#include "cellular.h"
#include "defs.h"
#include <raylib.h>
#include <rlgl.h>
#include <stdbool.h>
#include <stdlib.h>

static Shader noise_shader = {0};
static Shader reduce_shader = {0};
static Shader norm_shader = {0};

static RenderTexture2D prop = {0};
static RenderTexture2D noise = {0};
static RenderTexture2D final = {0};

static RenderTexture2D *mip_chain = NULL;
static size_t mip_count = 0;

static int seed = START_SEED;
static int res_x = START_RES_X;
static int res_y = START_RES_Y;
static int d_mode = START_MODE;
static int squared = START_SQUARED;
static float jitter = START_JITTER;
static int seamless = START_SEAMLESS;
static float freq = START_FREQ;
static float range_min = START_RANGE_MIN;
static float range_max = START_RANGE_MAX;
static float power = START_POWER;
static int invert = START_INVERT;
static int octaves = START_OCTAVES;
static float persistence = START_PERSISTENCE;
static float lacunarity = START_LACUNARITY;

static void __set_shader_property(Shader shader,
                                  const char *property,
                                  const void *value,
                                  int uniform_type) {
    int uniform_loc = GetShaderLocation(shader, property);
    SetShaderValue(shader, uniform_loc, value, uniform_type);
}

static void __load_mip_chain() {
    int w = res_x;
    int h = res_y;
    size_t count = 0;
    while (w > 1 || h > 1) {
        w = (w + 1) / 2;
        h = (h + 1) / 2;
        count++;
    }
    if (count == 0) count = 1;

    mip_chain = calloc(count, sizeof(RenderTexture2D));
    mip_count = count;

    w = res_x;
    h = res_y;
    for (int i = 0; i < count; i++) {
        w = (w + 1) / 2;
        h = (h + 1) / 2;
        if (w < 1) w = 1;
        if (h < 1) h = 1;
        mip_chain[i] = LoadRenderTexture(w, h);
        SetTextureFilter(mip_chain[i].texture, TEXTURE_FILTER_POINT);
    }
}

static void __free_mip_chain() {
    if (mip_chain != NULL) {
        for (int i = 0; i < mip_count; i++) {
            UnloadRenderTexture(mip_chain[i]);
        }
        free(mip_chain);
        mip_chain = NULL;
    }
    mip_count = 0;
}

static void __draw_fullscreen(Texture2D src, int dst_w, int dst_h) {
    Rectangle src_rec = {0, 0, (float)src.width, (float)src.height};
    Rectangle dst_rec = {0, 0, (float)dst_w, (float)dst_h};
    DrawTexturePro(src, src_rec, dst_rec, (Vector2){0, 0}, 0.0f, WHITE);
}

static void __generate_ce() {
    // We are using the alpha channel to hold actual data, each texture will
    // sample against a BLANK cleared background, there is nothing to blend.
    // Without it, the data on the alpha channel will corrupt the data on the
    // other channels.
    rlDrawRenderBatchActive();
    rlDisableColorBlend();

    __set_shader_property(noise_shader, "u_seed", &seed, SHADER_UNIFORM_INT);
    __set_shader_property(
        noise_shader, "u_frequency", &freq, SHADER_UNIFORM_FLOAT);
    __set_shader_property(
        noise_shader, "u_jitter", &jitter, SHADER_UNIFORM_FLOAT);
    __set_shader_property(
        noise_shader, "u_seamless", &seamless, SHADER_UNIFORM_INT);
    __set_shader_property(
        noise_shader, "u_combination", &d_mode, SHADER_UNIFORM_INT);
    __set_shader_property(
        noise_shader, "u_squaredDistance", &squared, SHADER_UNIFORM_INT);
    __set_shader_property(
        noise_shader, "u_octaves", &octaves, SHADER_UNIFORM_INT);
    __set_shader_property(
        noise_shader, "u_persistence", &persistence, SHADER_UNIFORM_FLOAT);
    __set_shader_property(
        noise_shader, "u_lacunarity", &lacunarity, SHADER_UNIFORM_FLOAT);

    // This texture is just to cause the shader to run on all the entire target
    // texture.
    BeginTextureMode(prop);
    ClearBackground(BLACK);
    EndTextureMode();

    // Base noise texture. On a R32G32B32A32 texture, each pixel containing a
    // raw noise float value.
    BeginTextureMode(noise);
    ClearBackground(BLACK);
    BeginShaderMode(noise_shader);
    __draw_fullscreen(prop.texture, noise.texture.width, noise.texture.height);
    EndShaderMode();
    EndTextureMode();

    // Chain mip textures are for minmax reduction. At the end of the process,
    // the last mip texture should be a single pixel containing the max value in
    // the noise pixel on R and the min value on G.
    for (int i = 0; i < mip_count; i++) {
        Texture2D src_tex = (i == 0) ? noise.texture : mip_chain[i - 1].texture;
        Vector2 texel_size = {1.0f / (float)src_tex.width,
                              1.0f / (float)src_tex.height};
        __set_shader_property(
            reduce_shader, "u_srcTexelSize", &texel_size, SHADER_UNIFORM_VEC2);

        BeginTextureMode(mip_chain[i]);
        ClearBackground(BLACK);
        BeginShaderMode(reduce_shader);
        SetShaderValueTexture(reduce_shader,
                              GetShaderLocation(reduce_shader, "u_texture"),
                              src_tex);
        __draw_fullscreen(
            src_tex, mip_chain[i].texture.width, mip_chain[i].texture.height);
        EndShaderMode();
        EndTextureMode();
    }

    // Normalization step. With texture with raw noise values and their min and
    // max values, we can normalize them in a normal R8G8B8A8 texture, as well
    // as apply range, power and inverted properties.
    __set_shader_property(
        norm_shader, "u_rangeMin", &range_min, SHADER_UNIFORM_FLOAT);
    __set_shader_property(
        norm_shader, "u_rangeMax", &range_max, SHADER_UNIFORM_FLOAT);
    __set_shader_property(norm_shader, "u_power", &power, SHADER_UNIFORM_FLOAT);
    __set_shader_property(
        norm_shader, "u_inverted", &invert, SHADER_UNIFORM_INT);
    BeginTextureMode(final);
    ClearBackground(BLACK);
    BeginShaderMode(norm_shader);
    SetShaderValueTexture(norm_shader,
                          GetShaderLocation(norm_shader, "u_rawNoise"),
                          noise.texture);
    SetShaderValueTexture(norm_shader,
                          GetShaderLocation(norm_shader, "u_minMaxTex"),
                          mip_chain[mip_count - 1].texture);
    __draw_fullscreen(noise.texture, final.texture.width, final.texture.height);
    EndShaderMode();
    EndTextureMode();

    // Restore color blending before rendering the rest of the app.
    rlDrawRenderBatchActive();
    rlEnableColorBlend();
}

static void __resize_ce() {
    UnloadRenderTexture(prop);
    prop = LoadRenderTexture(res_x, res_y);
    SetTextureFilter(prop.texture, TEXTURE_FILTER_BILINEAR);

    UnloadRenderTexture(noise);
    noise = LoadRenderTexture(res_x, res_y);
    SetTextureFilter(noise.texture, TEXTURE_FILTER_POINT);

    UnloadRenderTexture(final);
    final = LoadRenderTexture(res_x, res_y);
    SetTextureFilter(final.texture, TEXTURE_FILTER_BILINEAR);

    __free_mip_chain();
    __load_mip_chain();
}

void _cellular_init() {
    prop = LoadRenderTexture(res_x, res_y);
    SetTextureFilter(prop.texture, TEXTURE_FILTER_BILINEAR);

    noise = LoadRenderTexture(res_x, res_y);
    SetTextureFilter(noise.texture, TEXTURE_FILTER_POINT);

    final = LoadRenderTexture(res_x, res_y);
    SetTextureFilter(final.texture, TEXTURE_FILTER_BILINEAR);

    __load_mip_chain();

    char vert[] = {
#embed "shaders/base.vert" suffix(, '\0')
    };

    char noise_code[] = {
#embed "shaders/cellular.frag" suffix(, '\0')
    };

    char reduce_code[] = {
#embed "shaders/minmax_reduce.frag" suffix(, '\0')
    };

    char norm_code[] = {
#embed "shaders/normalize.frag" suffix(, '\0')
    };

    noise_shader = LoadShaderFromMemory(vert, noise_code);
    reduce_shader = LoadShaderFromMemory(vert, reduce_code);
    norm_shader = LoadShaderFromMemory(vert, norm_code);

    __generate_ce();
}

void _cellular_stop() {
    UnloadRenderTexture(prop);
    UnloadRenderTexture(noise);
    UnloadRenderTexture(final);
    __free_mip_chain();

    UnloadShader(noise_shader);
    UnloadShader(reduce_shader);
    UnloadShader(norm_shader);
}

void _cellular_set_seed(int new_val) {
    seed = new_val;
    __generate_ce();
}

void _cellular_set_mode(int new_val) {
    d_mode = new_val;
    __generate_ce();
}

void _cellular_set_squared(bool new_val) {
    squared = new_val;
    __generate_ce();
}

void _cellular_set_res(int new_x, int new_y) {
    res_x = new_x;
    res_y = new_y;
    __resize_ce();
    __generate_ce();
}

void _cellular_set_jitter(float new_val) {
    jitter = new_val;
    __generate_ce();
}

void _cellular_set_seamless(bool new_val) {
    seamless = new_val;
    __generate_ce();
}

void _cellular_set_freq(float new_val) {
    freq = new_val;
    __generate_ce();
}

void _cellular_set_range_min(float new_val) {
    range_min = new_val;
    __generate_ce();
}

void _cellular_set_range_max(float new_val) {
    range_max = new_val;
    __generate_ce();
}

void _cellular_set_power(float new_val) {
    power = new_val;
    __generate_ce();
}

void _cellular_set_invert(bool new_val) {
    invert = new_val;
    __generate_ce();
}

void _cellular_set_octaves(int new_val) {
    octaves = new_val;
    __generate_ce();
}

void _cellular_set_persistence(float new_val) {
    persistence = new_val;
    __generate_ce();
}

void _cellular_set_lacunarity(float new_val) {
    lacunarity = new_val;
    __generate_ce();
}

Texture2D _cellular_get() {
    return final.texture;
}
