#version 100

precision highp float;

varying vec2 uv;

uniform int u_seed;
uniform bool u_seamless;
uniform int u_combination;
uniform bool u_squaredDistance;

uniform float u_frequency;
uniform int u_octaves;
uniform float u_persistence;
uniform float u_lacunarity;
uniform float u_jitter;

uniform float u_normFactor;
uniform float u_rangeMin;
uniform float u_rangeMax;
uniform float u_power;
uniform bool u_inverted;

#define MAX_OCTAVES 8

#define K  0.142857142857
#define Ko 0.428571428571

#define TAU 6.28318530718

// -----------------------------------------------------------------------
// Cellular/Voronoi noise, ported from Justin Hawkin's cginc (as adapted
// in the original Unity shader). GLSL's built-in mod() is already
// x - y*floor(x/y), matching the original's custom mod(), so no need
// to redefine it here.
// -----------------------------------------------------------------------

vec3 permutation(vec3 x) {
    return mod((34.0 * x + 1.0) * x, 289.0);
}

vec2 cellularNoise(vec4 P, float jitter) {
    vec4 Pi = mod(floor(P), 289.0);
    vec4 Pf = fract(P);
    vec3 oi = vec3(-1.0, 0.0, 1.0);
    vec3 ofs = vec3(-0.5, 0.5, 1.5);

    vec3 px = permutation(Pi.x + oi);
    vec3 py = permutation(Pi.y + oi);
    vec3 pz = permutation(Pi.z + oi);

    vec3 p, ox, oy, oz, ow, dx, dy, dz, dw, d;
    vec2 F = vec2(1e6, 1e6);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                p = permutation(px[i] + py[j] + pz[k] + Pi.w + oi);

                ox = fract(p * K) - Ko;
                oy = mod(floor(p * K), 7.0) * K - Ko;

                p = permutation(p);

                oz = fract(p * K) - Ko;
                ow = mod(floor(p * K), 7.0) * K - Ko;

                dx = Pf.x - ofs[i] + jitter * ox;
                dy = Pf.y - ofs[j] + jitter * oy;
                dz = Pf.z - ofs[k] + jitter * oz;
                dw = Pf.w - ofs + jitter * ow;

                d = dx * dx + dy * dy + dz * dz + dw * dw;

                for (int n = 0; n < 3; n++) {
                    if (d[n] < F.x) {
                        F.y = F.x;
                        F.x = d[n];
                    } else if (d[n] < F.y) {
                        F.y = d[n];
                    }
                }
            }
        }
    }

    return F;
}

// Cheap hash used to turn a single seed value into a decorrelated
// offset per axis, so incrementing the seed reshuffles the noise
// field instead of just sliding it sideways.
float hash1(float n) {
    float x = fract(n * 0.1031);
    x *= x + 33.33;
    x *= x + x;
    return fract(x);
}

vec4 torusMapping(vec2 i) {
    vec4 o;
    o.x = sin(i.x * TAU);
    o.y = cos(i.x * TAU);
    o.z = sin(i.y * TAU);
    o.w = cos(i.y * TAU);
    return o;
}

void main() {
    vec4 coords;
    if (u_seamless) {
        coords = torusMapping(uv);
    } else {
        coords = vec4(uv * 5.0, 0.0, 0.0);
    }

    // Decorrelated per-axis offset derived from the seed. Each axis
    // uses a different multiplier before hashing so the four values
    // don't move together.
    float seedF = float(u_seed);
    vec4 seedOffset = vec4(
            hash1(seedF * 12.9898) * 289.0,
            hash1(seedF * 78.2330) * 289.0,
            hash1(seedF * 37.7190) * 289.0,
            hash1(seedF * 93.9890) * 289.0
        );

    float noise = 0.0;
    float freq = u_frequency;
    float amp = 0.5;

    for (int i = 0; i < MAX_OCTAVES; i++) {
        if (i >= u_octaves) break;

        vec4 p = coords * freq;

        if (u_seamless) {
            p += seedOffset + float(i) * 17.0 + freq * 5.0;
        } else {
            p += vec4(0.0, 0.0, 0.0, seedOffset.w + float(i));
        }

        vec2 F = cellularNoise(p, u_jitter);

        float contribution;
        if (u_combination == 0) {
            contribution = u_squaredDistance ? F.x : sqrt(F.x);
        } else {
            contribution = u_squaredDistance ? (F.y - F.x) : (sqrt(F.y) - sqrt(F.x));
        }
        noise += contribution * amp;

        freq *= u_lacunarity;
        amp *= u_persistence;
    }

    noise = noise / u_normFactor;
    noise = (noise - u_rangeMin) / (u_rangeMax - u_rangeMin);
    noise = clamp(noise, 0.0, 1.0);

    float k = pow(2.0, u_power - 1.0);
    noise = (noise <= 0.5) ? (k * pow(noise, u_power)) : noise;
    noise = (noise >= 0.5) ? (1.0 - k * pow(1.0 - noise, u_power)) : noise;

    if (u_inverted) {
        noise = 1.0 - noise;
    }

    gl_FragColor = vec4(noise, noise, noise, 1.0);
}
