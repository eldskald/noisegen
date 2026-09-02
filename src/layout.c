#include "layout.h"
#include "cellular.h"
#include "defs.h"
#include "opensimplex.h"
#include "raylib.h"
#include <math.h>
#include <raygui.h>
#include <stdbool.h>
#include <string.h>
#ifdef WEB
#include <emscripten/emscripten.h>
#endif

typedef enum { noise_type_opensimplex, noise_type_cellular } noise_type;

static char filename[EXPORT_FILENAME_MAX_SIZE + 1];
static int res_x = START_RES_X;
static int res_y = START_RES_Y;
static int noise = START_NOISE_TYPE;

static int os_seed = START_SEED;
static float os_old_freq = START_FREQ;
static float os_new_freq = START_FREQ;
static bool os_old_seamless = START_SEAMLESS;
static bool os_new_seamless = START_SEAMLESS;
static float os_old_range_min = START_RANGE_MIN;
static float os_new_range_min = START_RANGE_MIN;
static float os_old_range_max = START_RANGE_MAX;
static float os_new_range_max = START_RANGE_MAX;
static float os_old_power = START_POWER;
static float os_new_power = START_POWER;
static bool os_old_invert = START_INVERT;
static bool os_new_invert = START_INVERT;
static float os_old_octaves = START_OCTAVES;
static float os_new_octaves = START_OCTAVES;
static float os_old_persistence = START_PERSISTENCE;
static float os_new_persistence = START_PERSISTENCE;
static float os_old_lacunarity = START_LACUNARITY;
static float os_new_lacunarity = START_LACUNARITY;

static int ce_seed = START_SEED;
static int ce_old_mode = START_MODE;
static int ce_new_mode = START_MODE;
static bool ce_old_squared = START_SQUARED;
static bool ce_new_squared = START_SQUARED;
static float ce_old_freq = START_FREQ;
static float ce_new_freq = START_FREQ;
static bool ce_old_seamless = START_SEAMLESS;
static bool ce_new_seamless = START_SEAMLESS;
static float ce_old_jitter = START_JITTER;
static float ce_new_jitter = START_JITTER;
static float ce_old_range_min = START_RANGE_MIN;
static float ce_new_range_min = START_RANGE_MIN;
static float ce_old_range_max = START_RANGE_MAX;
static float ce_new_range_max = START_RANGE_MAX;
static float ce_old_power = START_POWER;
static float ce_new_power = START_POWER;
static bool ce_old_invert = START_INVERT;
static bool ce_new_invert = START_INVERT;
static float ce_old_octaves = START_OCTAVES;
static float ce_new_octaves = START_OCTAVES;
static float ce_old_persistence = START_PERSISTENCE;
static float ce_new_persistence = START_PERSISTENCE;
static float ce_old_lacunarity = START_LACUNARITY;
static float ce_new_lacunarity = START_LACUNARITY;

static bool editing_filename = false;
static bool editing_seed = false;
static bool editing_res_x = false;
static bool editing_res_y = false;

static bool msgbox_export_success = false;
static bool msgbox_export_failure = false;

void _layout_init() {
    int index = 0;
    TextAppend(filename, EXPORT_FILENAME_START_VAL, &index);
}

void _layout_draw() {
    Texture2D opensimplex = _opensimplex_get();
    Texture2D cellular = _cellular_get();

    BeginDrawing();
    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    // Lock/Unlock GUI if some popup is showing
    if (msgbox_export_success || msgbox_export_failure)
        GuiLock();
    else
        GuiUnlock();

    // App name and version at the top
    GuiLabel((Rectangle){APP_NAME_LABEL_X,
                         APP_NAME_LABEL_Y,
                         APP_NAME_LABEL_W,
                         APP_NAME_LABEL_H},
             APP_NAME " " APP_VERSION);

    // Export button
    if (GuiButton(
            (Rectangle){EXPORT_BTN_X, EXPORT_BTN_Y, EXPORT_BTN_W, EXPORT_BTN_H},
            EXPORT_BTN_LABEL)) {
        const int to_fit_dot_png = 4;
        char finalname[EXPORT_FILENAME_MAX_SIZE + to_fit_dot_png + 1];
        int index = 0;
        TextAppend(finalname, filename, &index);
        // Check for valid extension and make sure it is
        if ((GetFileExtension(filename) == NULL) ||
            !IsFileExtension(filename, ".png"))
            TextAppend(finalname, ".png", &index);

        // Exporting image
        Image img = {0};
        if (noise == noise_type_opensimplex)
            img = LoadImageFromTexture(_opensimplex_get());
        else
            img = LoadImageFromTexture(_cellular_get());
        if (ExportImage(img, finalname))
            msgbox_export_success = true;
        else
            msgbox_export_failure = true;
        UnloadImage(img);
#ifdef WEB
        // Downloading it if on web
        if (strchr(finalname, '\'') == NULL)
            emscripten_run_script(
                TextFormat("saveFileFromMEMFSToDisk('%s','%s')",
                           finalname,
                           GetFileName(finalname)));
#endif
    }
    GuiLabel((Rectangle){EXPORT_FILENAME_LABEL_X,
                         EXPORT_FILENAME_LABEL_Y,
                         EXPORT_FILENAME_LABEL_W,
                         EXPORT_FILENAME_LABEL_H},
             EXPORT_FILENAME_LABEL);
    if (GuiTextBox((Rectangle){EXPORT_FILENAME_X,
                               EXPORT_FILENAME_Y,
                               EXPORT_FILENAME_W,
                               EXPORT_FILENAME_H},
                   filename,
                   EXPORT_FILENAME_MAX_SIZE,
                   editing_filename)) {
        editing_filename = !editing_filename;
    }

    // Resolution controls
    GuiLabel((Rectangle){RES_SELECTOR_LABEL_X,
                         RES_SELECTOR_LABEL_Y,
                         RES_SELECTOR_LABEL_W,
                         RES_SELECTOR_LABEL_H},
             RES_SELECTOR_LABEL);
    if (GuiValueBox((Rectangle){RES_SELECTOR_X_X,
                                RES_SELECTOR_X_Y,
                                RES_SELECTOR_W,
                                RES_SELECTOR_H},
                    RES_SELECTOR_X_LABEL,
                    &res_x,
                    RES_SELECTOR_MIN,
                    RES_SELECTOR_MAX,
                    editing_res_x)) {
        editing_res_x = !editing_res_x;
        _opensimplex_set_res(res_x, res_y);
        _cellular_set_res(res_x, res_y);
    }
    if (GuiValueBox((Rectangle){RES_SELECTOR_Y_X,
                                RES_SELECTOR_Y_Y,
                                RES_SELECTOR_W,
                                RES_SELECTOR_H},
                    RES_SELECTOR_Y_LABEL,
                    &res_y,
                    RES_SELECTOR_MIN,
                    RES_SELECTOR_MAX,
                    editing_res_y)) {
        editing_res_y = !editing_res_y;
        _opensimplex_set_res(res_x, res_y);
    }

    // Noise type controls
    GuiLabel((Rectangle){NOISE_TYPE_LABEL_X,
                         NOISE_TYPE_LABEL_Y,
                         NOISE_TYPE_LABEL_W,
                         NOISE_TYPE_LABEL_H},
             NOISE_TYPE_LABEL);
    GuiToggleGroup(
        (Rectangle){
            NOISE_TYPE_X,
            NOISE_TYPE_Y,
            NOISE_TYPE_W,
            NOISE_TYPE_H,
        },
        NOISE_TYPE_OPTIONS,
        &noise);

    if (noise == noise_type_opensimplex) {
        // Texture at the right side
        int tex_w = 0;
        int tex_h = 0;
        if (opensimplex.width >= opensimplex.height) {
            tex_w = TEX_BOX_SIZE;
            tex_h = TEX_BOX_SIZE * opensimplex.height / opensimplex.width;
        } else {
            tex_h = TEX_BOX_SIZE;
            tex_w = TEX_BOX_SIZE * opensimplex.width / opensimplex.height;
        }
        DrawTexturePro(
            opensimplex,
            (Rectangle){
                0, 0, (float)opensimplex.width, (float)opensimplex.height},
            (Rectangle){TEX_BOX_X, TEX_BOX_Y, (float)tex_w, (float)tex_h},
            (Vector2){0, 0},
            0.0f,
            WHITE);

        // Seed controls
        GuiLabel((Rectangle){OS_SEED_SELECTOR_LABEL_X,
                             OS_SEED_SELECTOR_LABEL_Y,
                             OS_SEED_SELECTOR_LABEL_W,
                             OS_SEED_SELECTOR_LABEL_H},
                 OS_SEED_SELECTOR_LABEL);
        if (GuiValueBox((Rectangle){OS_SEED_SELECTOR_X,
                                    OS_SEED_SELECTOR_Y,
                                    OS_SEED_SELECTOR_W,
                                    OS_SEED_SELECTOR_H},
                        "",
                        &os_seed,
                        OS_SEED_SELECTOR_MIN,
                        OS_SEED_SELECTOR_MAX,
                        editing_seed)) {
            editing_seed = !editing_seed;
            _opensimplex_set_seed(os_seed);
        }

        // Frequency controls
        GuiLabel((Rectangle){OS_FREQ_SELECTOR_LABEL_X,
                             OS_FREQ_SELECTOR_LABEL_Y,
                             OS_FREQ_SELECTOR_LABEL_W,
                             OS_FREQ_SELECTOR_LABEL_H},
                 OS_FREQ_SELECTOR_LABEL);
        GuiSlider((Rectangle){OS_FREQ_SELECTOR_X,
                              OS_FREQ_SELECTOR_Y,
                              OS_FREQ_SELECTOR_W,
                              OS_FREQ_SELECTOR_H},
                  OS_FREQ_SELECTOR_LABEL_LEFT,
                  OS_FREQ_SELECTOR_LABEL_RIGHT,
                  &os_new_freq,
                  OS_FREQ_SELECTOR_MIN,
                  OS_FREQ_SELECTOR_MAX);
        GuiLabel((Rectangle){OS_FREQ_SELECTOR_VALUE_X,
                             OS_FREQ_SELECTOR_VALUE_Y,
                             OS_FREQ_SELECTOR_VALUE_W,
                             OS_FREQ_SELECTOR_VALUE_H},
                 TextFormat("%.2f", os_new_freq));
        if (os_new_freq != os_old_freq) _opensimplex_set_freq(os_new_freq);
        os_old_freq = os_new_freq;

        // Seamless controls
        GuiCheckBox((Rectangle){OS_SEAMLESS_SELECTOR_X,
                                OS_SEAMLESS_SELECTOR_Y,
                                OS_SEAMLESS_SELECTOR_W,
                                OS_SEAMLESS_SELECTOR_H},
                    OS_SEAMLESS_SELECTOR_LABEL,
                    &os_new_seamless);
        if (os_new_seamless != os_old_seamless)
            _opensimplex_set_seamless(os_new_seamless);
        os_old_seamless = os_new_seamless;

        // Range controls
        GuiLabel((Rectangle){OS_RANGE_SELECTOR_LABEL_X,
                             OS_RANGE_SELECTOR_LABEL_Y,
                             OS_RANGE_SELECTOR_LABEL_W,
                             OS_RANGE_SELECTOR_LABEL_H},
                 OS_RANGE_SELECTOR_LABEL);
        GuiSlider((Rectangle){OS_RANGE_SELECTOR_MIN_X,
                              OS_RANGE_SELECTOR_MIN_Y,
                              OS_RANGE_SELECTOR_MIN_W,
                              OS_RANGE_SELECTOR_MIN_H},
                  OS_RANGE_SELECTOR_MIN_LABEL,
                  TextFormat("%.2f", os_old_range_min),
                  &os_new_range_min,
                  OS_RANGE_SELECTOR_MIN_MIN,
                  OS_RANGE_SELECTOR_MIN_MAX);
        GuiSlider((Rectangle){OS_RANGE_SELECTOR_MAX_X,
                              OS_RANGE_SELECTOR_MAX_Y,
                              OS_RANGE_SELECTOR_MAX_W,
                              OS_RANGE_SELECTOR_MAX_H},
                  TextFormat("%.2f", os_old_range_max),
                  OS_RANGE_SELECTOR_MAX_LABEL,
                  &os_new_range_max,
                  OS_RANGE_SELECTOR_MAX_MIN,
                  OS_RANGE_SELECTOR_MAX_MAX);
        if (os_new_range_min != os_old_range_min)
            _opensimplex_set_range_min(os_new_range_min);
        if (os_new_range_max != os_old_range_max)
            _opensimplex_set_range_max(os_new_range_max);
        os_old_range_min = os_new_range_min;
        os_old_range_max = os_new_range_max;

        // Power controls
        GuiLabel((Rectangle){OS_POWER_SELECTOR_LABEL_X,
                             OS_POWER_SELECTOR_LABEL_Y,
                             OS_POWER_SELECTOR_LABEL_W,
                             OS_POWER_SELECTOR_LABEL_H},
                 OS_POWER_SELECTOR_LABEL);
        GuiSlider((Rectangle){OS_POWER_SELECTOR_X,
                              OS_POWER_SELECTOR_Y,
                              OS_POWER_SELECTOR_W,
                              OS_POWER_SELECTOR_H},
                  OS_POWER_SELECTOR_LABEL_LEFT,
                  OS_POWER_SELECTOR_LABEL_RIGHT,
                  &os_new_power,
                  OS_POWER_SELECTOR_MIN,
                  OS_POWER_SELECTOR_MAX);
        GuiLabel((Rectangle){OS_POWER_SELECTOR_VALUE_X,
                             OS_POWER_SELECTOR_VALUE_Y,
                             OS_POWER_SELECTOR_VALUE_W,
                             OS_POWER_SELECTOR_VALUE_H},
                 TextFormat("%.2f", os_new_power));
        if (os_new_power != os_old_power) _opensimplex_set_power(os_new_power);
        os_old_power = os_new_power;

        // Invert controls
        GuiCheckBox((Rectangle){OS_INVERT_SELECTOR_X,
                                OS_INVERT_SELECTOR_Y,
                                OS_INVERT_SELECTOR_W,
                                OS_INVERT_SELECTOR_H},
                    OS_INVERT_SELECTOR_LABEL,
                    &os_new_invert);
        if (os_new_invert != os_old_invert)
            _opensimplex_set_invert(os_new_invert);
        os_old_invert = os_new_invert;

        // Octaves controls
        GuiLabel((Rectangle){OS_OCTAVES_SELECTOR_LABEL_X,
                             OS_OCTAVES_SELECTOR_LABEL_Y,
                             OS_OCTAVES_SELECTOR_LABEL_W,
                             OS_OCTAVES_SELECTOR_LABEL_H},
                 OS_OCTAVES_SELECTOR_LABEL);
        GuiSlider((Rectangle){OS_OCTAVES_SELECTOR_X,
                              OS_OCTAVES_SELECTOR_Y,
                              OS_OCTAVES_SELECTOR_W,
                              OS_OCTAVES_SELECTOR_H},
                  OS_OCTAVES_SELECTOR_LABEL_LEFT,
                  OS_OCTAVES_SELECTOR_LABEL_RIGHT,
                  &os_new_octaves,
                  OS_OCTAVES_SELECTOR_MIN,
                  OS_OCTAVES_SELECTOR_MAX);
        GuiLabel((Rectangle){OS_OCTAVES_SELECTOR_VALUE_X,
                             OS_OCTAVES_SELECTOR_VALUE_Y,
                             OS_OCTAVES_SELECTOR_VALUE_W,
                             OS_OCTAVES_SELECTOR_VALUE_H},
                 TextFormat("%.0f", os_new_octaves));
        os_new_octaves = roundf(os_new_octaves);
        if (os_new_octaves != os_old_octaves)
            _opensimplex_set_octaves((int)os_new_octaves);
        os_old_octaves = os_new_octaves;

        // Persistence controls
        GuiLabel((Rectangle){OS_PERSISTENCE_SELECTOR_LABEL_X,
                             OS_PERSISTENCE_SELECTOR_LABEL_Y,
                             OS_PERSISTENCE_SELECTOR_LABEL_W,
                             OS_PERSISTENCE_SELECTOR_LABEL_H},
                 OS_PERSISTENCE_SELECTOR_LABEL);
        GuiSlider((Rectangle){OS_PERSISTENCE_SELECTOR_X,
                              OS_PERSISTENCE_SELECTOR_Y,
                              OS_PERSISTENCE_SELECTOR_W,
                              OS_PERSISTENCE_SELECTOR_H},
                  OS_PERSISTENCE_SELECTOR_LABEL_LEFT,
                  OS_PERSISTENCE_SELECTOR_LABEL_RIGHT,
                  &os_new_persistence,
                  OS_PERSISTENCE_SELECTOR_MIN,
                  OS_PERSISTENCE_SELECTOR_MAX);
        GuiLabel((Rectangle){OS_PERSISTENCE_SELECTOR_VALUE_X,
                             OS_PERSISTENCE_SELECTOR_VALUE_Y,
                             OS_PERSISTENCE_SELECTOR_VALUE_W,
                             OS_PERSISTENCE_SELECTOR_VALUE_H},
                 TextFormat("%.2f", os_new_persistence));
        if (os_new_persistence != os_old_persistence)
            _opensimplex_set_persistence(os_new_persistence);
        os_old_persistence = os_new_persistence;

        // Lacunarity controls
        GuiLabel((Rectangle){OS_LACUNARITY_SELECTOR_LABEL_X,
                             OS_LACUNARITY_SELECTOR_LABEL_Y,
                             OS_LACUNARITY_SELECTOR_LABEL_W,
                             OS_LACUNARITY_SELECTOR_LABEL_H},
                 OS_LACUNARITY_SELECTOR_LABEL);
        GuiSlider((Rectangle){OS_LACUNARITY_SELECTOR_X,
                              OS_LACUNARITY_SELECTOR_Y,
                              OS_LACUNARITY_SELECTOR_W,
                              OS_LACUNARITY_SELECTOR_H},
                  OS_LACUNARITY_SELECTOR_LABEL_LEFT,
                  OS_LACUNARITY_SELECTOR_LABEL_RIGHT,
                  &os_new_lacunarity,
                  OS_LACUNARITY_SELECTOR_MIN,
                  OS_LACUNARITY_SELECTOR_MAX);
        GuiLabel((Rectangle){OS_LACUNARITY_SELECTOR_VALUE_X,
                             OS_LACUNARITY_SELECTOR_VALUE_Y,
                             OS_LACUNARITY_SELECTOR_VALUE_W,
                             OS_LACUNARITY_SELECTOR_VALUE_H},
                 TextFormat("%.2f", os_new_lacunarity));
        if (os_new_lacunarity != os_old_lacunarity)
            _opensimplex_set_lacunarity(os_new_lacunarity);
        os_old_lacunarity = os_new_lacunarity;
    } else {
        // Texture at the right side
        int tex_w = 0;
        int tex_h = 0;
        if (cellular.width >= cellular.height) {
            tex_w = TEX_BOX_SIZE;
            tex_h = TEX_BOX_SIZE * cellular.height / cellular.width;
        } else {
            tex_h = TEX_BOX_SIZE;
            tex_w = TEX_BOX_SIZE * cellular.width / cellular.height;
        }
        DrawTexturePro(
            cellular,
            (Rectangle){0, 0, (float)cellular.width, (float)cellular.height},
            (Rectangle){TEX_BOX_X, TEX_BOX_Y, (float)tex_w, (float)tex_h},
            (Vector2){0, 0},
            0.0f,
            WHITE);

        // Seed controls
        GuiLabel((Rectangle){CE_SEED_SELECTOR_LABEL_X,
                             CE_SEED_SELECTOR_LABEL_Y,
                             CE_SEED_SELECTOR_LABEL_W,
                             CE_SEED_SELECTOR_LABEL_H},
                 CE_SEED_SELECTOR_LABEL);
        if (GuiValueBox((Rectangle){CE_SEED_SELECTOR_X,
                                    CE_SEED_SELECTOR_Y,
                                    CE_SEED_SELECTOR_W,
                                    CE_SEED_SELECTOR_H},
                        "",
                        &ce_seed,
                        CE_SEED_SELECTOR_MIN,
                        CE_SEED_SELECTOR_MAX,
                        editing_seed)) {
            editing_seed = !editing_seed;
            _cellular_set_seed(ce_seed);
        }

        // Mode controls
        GuiLabel((Rectangle){CE_MODE_LABEL_X,
                             CE_MODE_LABEL_Y,
                             CE_MODE_LABEL_W,
                             CE_MODE_LABEL_H},
                 CE_MODE_LABEL);
        GuiToggleGroup((Rectangle){CE_MODE_X, CE_MODE_Y, CE_MODE_W, CE_MODE_H},
                       CE_MODE_OPTIONS,
                       &ce_new_mode);
        if (ce_old_mode != ce_new_mode) {
            _cellular_set_mode(ce_new_mode);
        }
        ce_old_mode = ce_new_mode;

        // Squared controls
        GuiCheckBox(
            (Rectangle){CE_SQUARED_X, CE_SQUARED_Y, CE_SQUARED_W, CE_SQUARED_H},
            CE_SQUARED_LABEL,
            &ce_new_squared);
        if (ce_new_squared != ce_old_squared)
            _cellular_set_squared(ce_new_squared);
        ce_old_squared = ce_new_squared;

        // Frequency controls
        GuiLabel((Rectangle){CE_FREQ_SELECTOR_LABEL_X,
                             CE_FREQ_SELECTOR_LABEL_Y,
                             CE_FREQ_SELECTOR_LABEL_W,
                             CE_FREQ_SELECTOR_LABEL_H},
                 CE_FREQ_SELECTOR_LABEL);
        GuiSlider((Rectangle){CE_FREQ_SELECTOR_X,
                              CE_FREQ_SELECTOR_Y,
                              CE_FREQ_SELECTOR_W,
                              CE_FREQ_SELECTOR_H},
                  CE_FREQ_SELECTOR_LABEL_LEFT,
                  CE_FREQ_SELECTOR_LABEL_RIGHT,
                  &ce_new_freq,
                  CE_FREQ_SELECTOR_MIN,
                  CE_FREQ_SELECTOR_MAX);
        GuiLabel((Rectangle){CE_FREQ_SELECTOR_VALUE_X,
                             CE_FREQ_SELECTOR_VALUE_Y,
                             CE_FREQ_SELECTOR_VALUE_W,
                             CE_FREQ_SELECTOR_VALUE_H},
                 TextFormat("%.2f", ce_new_freq));
        if (ce_new_freq != ce_old_freq) _cellular_set_freq(ce_new_freq);
        ce_old_freq = ce_new_freq;

        // Seamless controls
        GuiCheckBox((Rectangle){CE_SEAMLESS_SELECTOR_X,
                                CE_SEAMLESS_SELECTOR_Y,
                                CE_SEAMLESS_SELECTOR_W,
                                CE_SEAMLESS_SELECTOR_H},
                    CE_SEAMLESS_SELECTOR_LABEL,
                    &ce_new_seamless);
        if (ce_new_seamless != ce_old_seamless)
            _cellular_set_seamless(ce_new_seamless);
        ce_old_seamless = ce_new_seamless;

        // Jitter controls
        GuiLabel((Rectangle){CE_JITTER_SELECTOR_LABEL_X,
                             CE_JITTER_SELECTOR_LABEL_Y,
                             CE_JITTER_SELECTOR_LABEL_W,
                             CE_JITTER_SELECTOR_LABEL_H},
                 CE_JITTER_SELECTOR_LABEL);
        GuiSlider((Rectangle){CE_JITTER_SELECTOR_X,
                              CE_JITTER_SELECTOR_Y,
                              CE_JITTER_SELECTOR_W,
                              CE_JITTER_SELECTOR_H},
                  CE_JITTER_SELECTOR_LABEL_LEFT,
                  CE_JITTER_SELECTOR_LABEL_RIGHT,
                  &ce_new_jitter,
                  CE_JITTER_SELECTOR_MIN,
                  CE_JITTER_SELECTOR_MAX);
        GuiLabel((Rectangle){CE_JITTER_SELECTOR_VALUE_X,
                             CE_JITTER_SELECTOR_VALUE_Y,
                             CE_JITTER_SELECTOR_VALUE_W,
                             CE_JITTER_SELECTOR_VALUE_H},
                 TextFormat("%.2f", ce_new_jitter));
        if (ce_new_jitter != ce_old_jitter) _cellular_set_jitter(ce_new_jitter);
        ce_old_jitter = ce_new_jitter;

        // Power controls
        GuiLabel((Rectangle){CE_POWER_SELECTOR_LABEL_X,
                             CE_POWER_SELECTOR_LABEL_Y,
                             CE_POWER_SELECTOR_LABEL_W,
                             CE_POWER_SELECTOR_LABEL_H},
                 CE_POWER_SELECTOR_LABEL);
        GuiSlider((Rectangle){CE_POWER_SELECTOR_X,
                              CE_POWER_SELECTOR_Y,
                              CE_POWER_SELECTOR_W,
                              CE_POWER_SELECTOR_H},
                  CE_POWER_SELECTOR_LABEL_LEFT,
                  CE_POWER_SELECTOR_LABEL_RIGHT,
                  &ce_new_power,
                  CE_POWER_SELECTOR_MIN,
                  CE_POWER_SELECTOR_MAX);
        GuiLabel((Rectangle){CE_POWER_SELECTOR_VALUE_X,
                             CE_POWER_SELECTOR_VALUE_Y,
                             CE_POWER_SELECTOR_VALUE_W,
                             CE_POWER_SELECTOR_VALUE_H},
                 TextFormat("%.2f", ce_new_power));
        if (ce_new_power != ce_old_power) _cellular_set_power(ce_new_power);
        ce_old_power = ce_new_power;

        // Invert controls
        GuiCheckBox((Rectangle){CE_INVERT_SELECTOR_X,
                                CE_INVERT_SELECTOR_Y,
                                CE_INVERT_SELECTOR_W,
                                CE_INVERT_SELECTOR_H},
                    CE_INVERT_SELECTOR_LABEL,
                    &ce_new_invert);
        if (ce_new_invert != ce_old_invert) _cellular_set_invert(ce_new_invert);
        ce_old_invert = ce_new_invert;

        // Range controls
        GuiLabel((Rectangle){CE_RANGE_SELECTOR_LABEL_X,
                             CE_RANGE_SELECTOR_LABEL_Y,
                             CE_RANGE_SELECTOR_LABEL_W,
                             CE_RANGE_SELECTOR_LABEL_H},
                 CE_RANGE_SELECTOR_LABEL);
        GuiSlider((Rectangle){CE_RANGE_SELECTOR_MIN_X,
                              CE_RANGE_SELECTOR_MIN_Y,
                              CE_RANGE_SELECTOR_MIN_W,
                              CE_RANGE_SELECTOR_MIN_H},
                  CE_RANGE_SELECTOR_MIN_LABEL,
                  TextFormat("%.2f", ce_old_range_min),
                  &ce_new_range_min,
                  CE_RANGE_SELECTOR_MIN_MIN,
                  CE_RANGE_SELECTOR_MIN_MAX);
        GuiSlider((Rectangle){CE_RANGE_SELECTOR_MAX_X,
                              CE_RANGE_SELECTOR_MAX_Y,
                              CE_RANGE_SELECTOR_MAX_W,
                              CE_RANGE_SELECTOR_MAX_H},
                  TextFormat("%.2f", ce_old_range_max),
                  CE_RANGE_SELECTOR_MAX_LABEL,
                  &ce_new_range_max,
                  CE_RANGE_SELECTOR_MAX_MIN,
                  CE_RANGE_SELECTOR_MAX_MAX);
        if (ce_new_range_min != ce_old_range_min)
            _cellular_set_range_min(ce_new_range_min);
        if (ce_new_range_max != ce_old_range_max)
            _cellular_set_range_max(ce_new_range_max);
        ce_old_range_min = ce_new_range_min;
        ce_old_range_max = ce_new_range_max;

        // Octaves controls
        GuiLabel((Rectangle){CE_OCTAVES_SELECTOR_LABEL_X,
                             CE_OCTAVES_SELECTOR_LABEL_Y,
                             CE_OCTAVES_SELECTOR_LABEL_W,
                             CE_OCTAVES_SELECTOR_LABEL_H},
                 CE_OCTAVES_SELECTOR_LABEL);
        GuiSlider((Rectangle){CE_OCTAVES_SELECTOR_X,
                              CE_OCTAVES_SELECTOR_Y,
                              CE_OCTAVES_SELECTOR_W,
                              CE_OCTAVES_SELECTOR_H},
                  CE_OCTAVES_SELECTOR_LABEL_LEFT,
                  CE_OCTAVES_SELECTOR_LABEL_RIGHT,
                  &ce_new_octaves,
                  CE_OCTAVES_SELECTOR_MIN,
                  CE_OCTAVES_SELECTOR_MAX);
        GuiLabel((Rectangle){CE_OCTAVES_SELECTOR_VALUE_X,
                             CE_OCTAVES_SELECTOR_VALUE_Y,
                             CE_OCTAVES_SELECTOR_VALUE_W,
                             CE_OCTAVES_SELECTOR_VALUE_H},
                 TextFormat("%.0f", ce_new_octaves));
        ce_new_octaves = roundf(ce_new_octaves);
        if (ce_new_octaves != ce_old_octaves)
            _cellular_set_octaves((int)ce_new_octaves);
        ce_old_octaves = ce_new_octaves;

        // Persistence controls
        GuiLabel((Rectangle){CE_PERSISTENCE_SELECTOR_LABEL_X,
                             CE_PERSISTENCE_SELECTOR_LABEL_Y,
                             CE_PERSISTENCE_SELECTOR_LABEL_W,
                             CE_PERSISTENCE_SELECTOR_LABEL_H},
                 CE_PERSISTENCE_SELECTOR_LABEL);
        GuiSlider((Rectangle){CE_PERSISTENCE_SELECTOR_X,
                              CE_PERSISTENCE_SELECTOR_Y,
                              CE_PERSISTENCE_SELECTOR_W,
                              CE_PERSISTENCE_SELECTOR_H},
                  CE_PERSISTENCE_SELECTOR_LABEL_LEFT,
                  CE_PERSISTENCE_SELECTOR_LABEL_RIGHT,
                  &ce_new_persistence,
                  CE_PERSISTENCE_SELECTOR_MIN,
                  CE_PERSISTENCE_SELECTOR_MAX);
        GuiLabel((Rectangle){CE_PERSISTENCE_SELECTOR_VALUE_X,
                             CE_PERSISTENCE_SELECTOR_VALUE_Y,
                             CE_PERSISTENCE_SELECTOR_VALUE_W,
                             CE_PERSISTENCE_SELECTOR_VALUE_H},
                 TextFormat("%.2f", ce_new_persistence));
        if (ce_new_persistence != ce_old_persistence)
            _cellular_set_persistence(ce_new_persistence);
        ce_old_persistence = ce_new_persistence;

        // Lacunarity controls
        GuiLabel((Rectangle){CE_LACUNARITY_SELECTOR_LABEL_X,
                             CE_LACUNARITY_SELECTOR_LABEL_Y,
                             CE_LACUNARITY_SELECTOR_LABEL_W,
                             CE_LACUNARITY_SELECTOR_LABEL_H},
                 CE_LACUNARITY_SELECTOR_LABEL);
        GuiSlider((Rectangle){CE_LACUNARITY_SELECTOR_X,
                              CE_LACUNARITY_SELECTOR_Y,
                              CE_LACUNARITY_SELECTOR_W,
                              CE_LACUNARITY_SELECTOR_H},
                  CE_LACUNARITY_SELECTOR_LABEL_LEFT,
                  CE_LACUNARITY_SELECTOR_LABEL_RIGHT,
                  &ce_new_lacunarity,
                  CE_LACUNARITY_SELECTOR_MIN,
                  CE_LACUNARITY_SELECTOR_MAX);
        GuiLabel((Rectangle){CE_LACUNARITY_SELECTOR_VALUE_X,
                             CE_LACUNARITY_SELECTOR_VALUE_Y,
                             CE_LACUNARITY_SELECTOR_VALUE_W,
                             CE_LACUNARITY_SELECTOR_VALUE_H},
                 TextFormat("%.2f", ce_new_lacunarity));
        if (ce_new_lacunarity != ce_old_lacunarity)
            _cellular_set_lacunarity(ce_new_lacunarity);
        ce_old_lacunarity = ce_new_lacunarity;
    }

    // Popups
    if (msgbox_export_success || msgbox_export_failure) {
        Color bg = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
        DrawRectangle(0,
                      0,
                      WINDOW_SIZE_X,
                      WINDOW_SIZE_Y,
                      (Color){bg.r, bg.g, bg.b, bg.a * 2 / 3});
    }
    if (msgbox_export_success) {
        GuiUnlock();
        int res = GuiMessageBox(
            (Rectangle){(WINDOW_SIZE_X - EXPORT_MSGBOX_SUCCESS_W) / 2,
                        (WINDOW_SIZE_Y - EXPORT_MSGBOX_SUCCESS_H) / 2,
                        EXPORT_MSGBOX_SUCCESS_W,
                        EXPORT_MSGBOX_SUCCESS_H},
            EXPORT_MSGBOX_SUCCESS_TITLE,
            EXPORT_MSGBOX_SUCCESS_MESSAGE,
            EXPORT_MSGBOX_SUCCESS_BUTTONS);
        GuiLock();
        if (res >= 0) msgbox_export_success = false;
    }
    if (msgbox_export_failure) {
        GuiUnlock();
        int res = GuiMessageBox(
            (Rectangle){(WINDOW_SIZE_X - EXPORT_MSGBOX_FAILURE_W) / 2,
                        (WINDOW_SIZE_Y - EXPORT_MSGBOX_FAILURE_H) / 2,
                        EXPORT_MSGBOX_FAILURE_W,
                        EXPORT_MSGBOX_FAILURE_H},
            EXPORT_MSGBOX_FAILURE_TITLE,
            EXPORT_MSGBOX_FAILURE_MESSAGE,
            EXPORT_MSGBOX_FAILURE_BUTTONS);
        GuiLock();
        if (res >= 0) msgbox_export_failure = false;
    }

    EndDrawing();
}
