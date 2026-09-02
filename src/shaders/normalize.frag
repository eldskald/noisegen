#version 100

precision highp float;

varying vec2 uv;

uniform sampler2D u_rawNoise;
uniform sampler2D u_minMaxTex;

uniform float u_rangeMin;
uniform float u_rangeMax;
uniform float u_power;
uniform bool u_inverted;

#define NOISE_BOUND 8.0
#define EPSILON 1e-6

float unpackFloat(vec2 rg) {
    return dot(rg, vec2(1.0, 1.0 / 255.0));
}

void main() {
    // The raw noise texture is packed into 16-bits floats using two color channels to
    // do so. We do this to keep higher precision in a R8G8B8A8 texture, since we can't
    // use a R32G32B32A32 texture in WebGL.
    vec4 packedRaw = texture2D(u_rawNoise, uv);
    float encoded = unpackFloat(packedRaw.rg);
    float raw = encoded * (2.0 * NOISE_BOUND) - NOISE_BOUND;

    // packedMinMax is a 1x1 texture, RG is min, BA is max. We use unpackFloat()
    // to turn the 16-bits into a float of sorts that we packed on minmax_reduce.frag
    // Read minmax_reduce.frag to understand the method.
    vec4 packedMinMax = texture2D(u_minMaxTex, vec2(0.5, 0.5));
    float minVal = unpackFloat(packedMinMax.rg) * (2.0 * NOISE_BOUND) - NOISE_BOUND;
    float maxVal = unpackFloat(packedMinMax.ba) * (2.0 * NOISE_BOUND) - NOISE_BOUND;

    // Normalize with min/max
    float denom = maxVal - minVal;
    if (denom < EPSILON) denom = EPSILON; // guard against a flat/degenerate texture
    float t = clamp((raw - minVal) / denom, 0.0, 1.0);

    // Normalize with range
    float noise = (t - u_rangeMin) / (u_rangeMax - u_rangeMin);
    noise = clamp(noise, 0.0, 1.0);

    // Apply power
    float k = pow(2.0, u_power - 1.0);
    noise = (noise <= 0.5) ? (k * pow(noise, u_power)) : noise;
    noise = (noise >= 0.5) ? (1.0 - k * pow(1.0 - noise, u_power)) : noise;

    // Apply invertion
    if (u_inverted) {
        noise = 1.0 - noise;
    }

    gl_FragColor = vec4(vec3(noise), 1.0);
}
