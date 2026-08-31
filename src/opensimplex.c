#include "opensimplex.h"
#include "OpenSimplex2F.h"
#include "defs.h"
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdlib.h>

#define FACTOR 72

static RenderTexture2D final = {0};
static struct OpenSimplex2F_context *ctx = NULL;
static int seed = START_SEED;
static int res_x = START_RES_X;
static int res_y = START_RES_Y;
static bool seamless = START_SEAMLESS;
static float freq = START_FREQ;
static float range_min = START_RANGE_MIN;
static float range_max = START_RANGE_MAX;
static float power = START_POWER;
static bool invert = START_INVERT;
static int octaves = START_OCTAVES;
static float persistence = START_PERSISTENCE;
static float lacunarity = START_LACUNARITY;

static Color __calc_color(double value) {
    // Apply range
    float val = Clamp((float)value, range_min, range_max);
    val = (val - range_min) / (range_max - range_min);

    // Apply power
    float k = powf(TWO, power - 1.0f);
    if (val <= HALF)
        val = k * powf(val, power);
    else
        val = 1.0f - k * powf(1.0f - val, power);

    // Invert
    if (invert) val = 1.0f - val;

    // Final color
    int col = (int)(val * RGBMAX);
    return (Color){col, col, col, RGBMAX};
}

static double __calc_value(int i, int j) {
    double sum = 0.0f;
    double amplitude = 1.0f;
    double frequency = freq;
    for (int k = 0; k < octaves; k++) {
        if (seamless) {
            sum += OpenSimplex2F_noise4_Classic(
                       ctx,
                       sin(TAU * (double)i / res_x) * frequency +
                           2 * freq * (double)k,
                       cos(TAU * (double)i / res_x) * frequency +
                           2 * freq * (double)k,
                       sin(TAU * (double)j / res_y) * frequency +
                           2 * freq * (double)k,
                       cos(TAU * (double)j / res_y) * frequency +
                           2 * freq * (double)k) *
                   amplitude;
        } else {
            sum += OpenSimplex2F_noise4_Classic(ctx,
                                                (double)i / FACTOR * frequency,
                                                (double)j / FACTOR * frequency,
                                                k,
                                                0) *
                   amplitude;
        }
        frequency *= lacunarity;
        amplitude *= persistence;
    }
    return sum;
}

static void __resize_os() {
    UnloadRenderTexture(final);
    final = LoadRenderTexture(res_x, res_y);
    SetTextureFilter(final.texture, TEXTURE_FILTER_BILINEAR);
}

static void __generate_os() {
    BeginTextureMode(final);
    for (int i = 0; i < res_x; i++) {
        for (int j = 0; j < res_y; j++) {
            DrawPixel(i, j, __calc_color(__calc_value(i, j)));
        }
    }
    EndTextureMode();
}

void _opensimplex_init() {
    final = LoadRenderTexture(res_x, res_y);
    SetTextureFilter(final.texture, TEXTURE_FILTER_BILINEAR);
    OpenSimplex2F(seed, &ctx);
    __generate_os();
}

void _opensimplex_stop() {
    UnloadRenderTexture(final);
    OpenSimplex2F_free(ctx);
    OpenSimplex2F_shutdown();
}

Texture2D _opensimplex_get() {
    return final.texture;
}

void _opensimplex_set_seed(int new_val) {
    seed = new_val;
    OpenSimplex2F_free(ctx);
    OpenSimplex2F(seed, &ctx);
    __generate_os();
}

void _opensimplex_set_res(int new_x, int new_y) {
    res_x = new_x;
    res_y = new_y;
    __resize_os();
    __generate_os();
}

void _opensimplex_set_seamless(bool new_val) {
    seamless = new_val;
    __generate_os();
}

void _opensimplex_set_freq(float new_val) {
    freq = new_val;
    __generate_os();
}

void _opensimplex_set_range_min(float new_val) {
    range_min = new_val;
    __generate_os();
}

void _opensimplex_set_range_max(float new_val) {
    range_max = new_val;
    __generate_os();
}

void _opensimplex_set_power(float new_val) {
    power = new_val;
    __generate_os();
}

void _opensimplex_set_invert(bool new_val) {
    invert = new_val;
    __generate_os();
}

void _opensimplex_set_octaves(int new_val) {
    octaves = new_val;
    __generate_os();
}

void _opensimplex_set_persistence(float new_val) {
    persistence = new_val;
    __generate_os();
}

void _opensimplex_set_lacunarity(float new_val) {
    lacunarity = new_val;
    __generate_os();
}
