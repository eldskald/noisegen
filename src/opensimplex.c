#include "opensimplex.h"
#include "OpenSimplex2F.h"
#include "defs.h"
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdlib.h>

#define TAU PI * 2.0f
#define RGBMAX 255
#define FACTOR 24

static RenderTexture2D final = {0};
static struct OpenSimplex2F_context *ctx = NULL;
static int seed = TEX_START_SEED;
static int res_x = TEX_START_RES_X;
static int res_y = TEX_START_RES_Y;
static bool seamless = TEX_START_SEAMLESS;
static float freq = TEX_START_FREQ;
static float range_min = TEX_START_RANGE_MIN;
static float range_max = TEX_START_RANGE_MAX;

Color __calc_color(double val) {
    float clamped = Clamp((float)val, range_min, range_max);
    int col =
        (int)((clamped - range_min) / (range_max - range_min) * (float)RGBMAX);
    return (Color){col, col, col, RGBMAX};
}

double __calc_value(int i, int j) {
    if (seamless) {
        return OpenSimplex2F_noise4_Classic(
            ctx, (double)i / FACTOR * freq, (double)j / FACTOR * freq, 0, 0);
    }
    return OpenSimplex2F_noise4_Classic(ctx,
                                        sin(TAU * (double)i) * freq / FACTOR,
                                        cos(TAU * (double)i) * freq / FACTOR,
                                        sin(TAU * (double)j) * freq / FACTOR,
                                        cos(TAU * (double)j) * freq / FACTOR);
}

void __generate() {
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
    OpenSimplex2F(seed, &ctx);
    __generate();
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
    __generate();
}

void _opensimplex_set_res(int new_val) {
    res_x = new_val;
    res_y = new_val;
    __generate();
}

void _opensimplex_set_seamless(bool new_val) {
    seamless = new_val;
    __generate();
}

void _opensimplex_set_freq(float new_val) {
    freq = new_val;
    __generate();
}

void _opensimplex_set_range_min(float new_val) {
    range_min = new_val;
    __generate();
}

void _opensimplex_set_range_max(float new_val) {
    range_max = new_val;
    __generate();
}
