#include "opensimplex.h"
#include "defs.h"
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>

static Shader os_noise_shader = {0};
static RenderTexture2D prop = {0};
static RenderTexture2D final = {0};
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

static void __set_shader_property_(const char *property,
                                   const void *value,
                                   int uniform_type) {
    int uniform_loc = GetShaderLocation(os_noise_shader, property);
    SetShaderValue(os_noise_shader, uniform_loc, value, uniform_type);
}

static float __compute_norm_factor(int octaves_in, float persistence_in) {
    float norm = 0.0f;
    for (int i = 0; i < octaves_in; i++) {
        norm += powf(persistence_in, (float)i);
    }
    return norm;
}

static void __generate_os() {
    __set_shader_property_("u_seed", &seed, SHADER_UNIFORM_INT);
    __set_shader_property_("u_frequency", &freq, SHADER_UNIFORM_FLOAT);
    __set_shader_property_("u_seamless", &seamless, SHADER_UNIFORM_INT);

    __set_shader_property_("u_power", &power, SHADER_UNIFORM_FLOAT);
    __set_shader_property_("u_rangeMin", &range_min, SHADER_UNIFORM_FLOAT);
    __set_shader_property_("u_rangeMax", &range_max, SHADER_UNIFORM_FLOAT);
    __set_shader_property_("u_inverted", &invert, SHADER_UNIFORM_INT);

    __set_shader_property_("u_octaves", &octaves, SHADER_UNIFORM_INT);
    __set_shader_property_("u_persistence", &persistence, SHADER_UNIFORM_FLOAT);
    __set_shader_property_("u_lacunarity", &lacunarity, SHADER_UNIFORM_FLOAT);

    float norm_factor = __compute_norm_factor(octaves, persistence);
    __set_shader_property_("u_normFactor", &norm_factor, SHADER_UNIFORM_FLOAT);

    BeginTextureMode(prop);
    ClearBackground(BLACK);
    EndTextureMode();

    BeginTextureMode(final);
    ClearBackground(BLACK);
    BeginShaderMode(os_noise_shader);
    DrawTexture(prop.texture, 0, 0, WHITE);
    EndShaderMode();
    EndTextureMode();
}

static void __resize_os() {
    UnloadRenderTexture(prop);
    prop = LoadRenderTexture(res_x, res_y);
    SetTextureFilter(prop.texture, TEXTURE_FILTER_BILINEAR);

    UnloadRenderTexture(final);
    final = LoadRenderTexture(res_x, res_y);
    SetTextureFilter(final.texture, TEXTURE_FILTER_BILINEAR);
}

void _opensimplex_init() {
    prop = LoadRenderTexture(res_x, res_y);
    SetTextureFilter(prop.texture, TEXTURE_FILTER_BILINEAR);

    final = LoadRenderTexture(res_x, res_y);
    SetTextureFilter(final.texture, TEXTURE_FILTER_BILINEAR);

    char vert[] = {
#embed "shaders/base.vert" suffix(, '\0')
    };

    char frag[] = {
#embed "shaders/opensimplex.frag" suffix(, '\0')
    };

    os_noise_shader = LoadShaderFromMemory(vert, frag);

    __generate_os();
}

void _opensimplex_stop() {
    UnloadRenderTexture(prop);
    UnloadRenderTexture(final);
    UnloadShader(os_noise_shader);
}

Texture2D _opensimplex_get() {
    return final.texture;
}

void _opensimplex_set_seed(int new_val) {
    seed = new_val;
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
