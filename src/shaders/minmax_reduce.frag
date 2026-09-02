#version 100

precision highp float;

varying vec2 uv;

// Minmax reduce shader.
//
// The goal is to take a texture of raw values packed in 16-bit floats in its RG and
// BA channels (see packFloat and unpackFloat functions) and spit out a texture with
// half the resolution. Each pixel takes the 4 equivalent source pixels, unpack the
// values, measures min and max values, puts min in RG and max in BA. If we keep doing
// that successively, we eventually get a 1x1 texture with the original texture's min
// packed in its RG and its max packed in its BA.
//
// This is a GPU only way to find a texture's min and max values, and it takes O(log(n))
// where n is the texture's resolution calls, which is quite fast especially given they
// keep getting exponentially smaller.

uniform sampler2D u_texture;
uniform vec2 u_srcTexelSize;

vec2 packFloat(float v) {
    vec2 enc = vec2(1.0, 255.0) * v;
    enc = fract(enc);
    enc.x -= enc.y * (1.0 / 255.0);
    return enc;
}

float unpackFloat(vec2 rg) {
    return dot(rg, vec2(1.0, 1.0 / 255.0));
}

void main() {
    vec2 uv0 = uv - u_srcTexelSize * 0.5;

    vec4 a = texture2D(u_texture, uv0);
    vec4 b = texture2D(u_texture, uv0 + vec2(u_srcTexelSize.x, 0.0));
    vec4 c = texture2D(u_texture, uv0 + vec2(0.0, u_srcTexelSize.y));
    vec4 d = texture2D(u_texture, uv0 + u_srcTexelSize);

    float minA = unpackFloat(a.rg);
    float minB = unpackFloat(b.rg);
    float minC = unpackFloat(c.rg);
    float minD = unpackFloat(d.rg);

    float maxA = unpackFloat(a.ba);
    float maxB = unpackFloat(b.ba);
    float maxC = unpackFloat(c.ba);
    float maxD = unpackFloat(d.ba);

    float minVal = min(min(minA, minB), min(minC, minD));
    float maxVal = max(max(maxA, maxB), max(maxC, maxD));

    gl_FragColor = vec4(packFloat(minVal), packFloat(maxVal));
}
