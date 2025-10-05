#include "layout.h"
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

static char filename[EXPORT_FILENAME_MAX_SIZE + 1];
static int seed = TEX_START_SEED;
static int res_x = TEX_START_RES_X;
static int res_y = TEX_START_RES_Y;
static float old_freq = TEX_START_FREQ;
static float new_freq = TEX_START_FREQ;
static bool old_seamless = TEX_START_SEAMLESS;
static bool new_seamless = TEX_START_SEAMLESS;
static float old_range_min = TEX_START_RANGE_MIN;
static float new_range_min = TEX_START_RANGE_MIN;
static float old_range_max = TEX_START_RANGE_MAX;
static float new_range_max = TEX_START_RANGE_MAX;
static float old_power = TEX_START_POWER;
static float new_power = TEX_START_POWER;
static bool old_invert = TEX_START_INVERT;
static bool new_invert = TEX_START_INVERT;
static float old_octaves = TEX_START_OCTAVES;
static float new_octaves = TEX_START_OCTAVES;
static float old_persistence = TEX_START_PERSISTENCE;
static float new_persistence = TEX_START_PERSISTENCE;
static float old_lacunarity = TEX_START_LACUNARITY;
static float new_lacunarity = TEX_START_LACUNARITY;

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
        (Rectangle){0, 0, (float)opensimplex.width, (float)opensimplex.height},
        (Rectangle){TEX_BOX_X, TEX_BOX_Y, (float)tex_w, (float)tex_h},
        (Vector2){0, 0},
        0.0f,
        WHITE);

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
        Image img = LoadImageFromTexture(_opensimplex_get());
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

    // Seed controls
    GuiLabel((Rectangle){SEED_SELECTOR_LABEL_X,
                         SEED_SELECTOR_LABEL_Y,
                         SEED_SELECTOR_LABEL_W,
                         SEED_SELECTOR_LABEL_H},
             SEED_SELECTOR_LABEL);
    if (GuiValueBox((Rectangle){SEED_SELECTOR_X,
                                SEED_SELECTOR_Y,
                                SEED_SELECTOR_W,
                                SEED_SELECTOR_H},
                    "",
                    &seed,
                    SEED_SELECTOR_MIN,
                    SEED_SELECTOR_MAX,
                    editing_seed)) {
        editing_seed = !editing_seed;
        _opensimplex_set_seed(seed);
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

    // Frequency controls
    GuiLabel((Rectangle){FREQ_SELECTOR_LABEL_X,
                         FREQ_SELECTOR_LABEL_Y,
                         FREQ_SELECTOR_LABEL_W,
                         FREQ_SELECTOR_LABEL_H},
             FREQ_SELECTOR_LABEL);
    GuiSlider(
        (Rectangle){
            FREQ_SELECTOR_X, FREQ_SELECTOR_Y, FREQ_SELECTOR_W, FREQ_SELECTOR_H},
        FREQ_SELECTOR_LABEL_LEFT,
        FREQ_SELECTOR_LABEL_RIGHT,
        &new_freq,
        FREQ_SELECTOR_MIN,
        FREQ_SELECTOR_MAX);
    if (new_freq != old_freq) _opensimplex_set_freq(new_freq);
    old_freq = new_freq;

    // Seamless controls
    GuiCheckBox((Rectangle){SEAMLESS_SELECTOR_X,
                            SEAMLESS_SELECTOR_Y,
                            SEAMLESS_SELECTOR_W,
                            SEAMLESS_SELECTOR_H},
                SEAMLESS_SELECTOR_LABEL,
                &new_seamless);
    if (new_seamless != old_seamless) _opensimplex_set_seamless(new_seamless);
    old_seamless = new_seamless;

    // Range controls
    GuiLabel((Rectangle){RANGE_SELECTOR_LABEL_X,
                         RANGE_SELECTOR_LABEL_Y,
                         RANGE_SELECTOR_LABEL_W,
                         RANGE_SELECTOR_LABEL_H},
             RANGE_SELECTOR_LABEL);
    GuiSlider((Rectangle){RANGE_SELECTOR_MIN_X,
                          RANGE_SELECTOR_MIN_Y,
                          RANGE_SELECTOR_MIN_W,
                          RANGE_SELECTOR_MIN_H},
              RANGE_SELECTOR_MIN_LABEL,
              TextFormat("%.2f", old_range_min),
              &new_range_min,
              RANGE_SELECTOR_MIN_MIN,
              RANGE_SELECTOR_MIN_MAX);
    GuiSlider((Rectangle){RANGE_SELECTOR_MAX_X,
                          RANGE_SELECTOR_MAX_Y,
                          RANGE_SELECTOR_MAX_W,
                          RANGE_SELECTOR_MAX_H},
              TextFormat("%.2f", old_range_max),
              RANGE_SELECTOR_MAX_LABEL,
              &new_range_max,
              RANGE_SELECTOR_MAX_MIN,
              RANGE_SELECTOR_MAX_MAX);
    if (new_range_min != old_range_min)
        _opensimplex_set_range_min(new_range_min);
    if (new_range_max != old_range_max)
        _opensimplex_set_range_max(new_range_max);
    old_range_min = new_range_min;
    old_range_max = new_range_max;

    // Power controls
    GuiLabel((Rectangle){POWER_SELECTOR_LABEL_X,
                         POWER_SELECTOR_LABEL_Y,
                         POWER_SELECTOR_LABEL_W,
                         POWER_SELECTOR_LABEL_H},
             POWER_SELECTOR_LABEL);
    GuiSlider((Rectangle){POWER_SELECTOR_X,
                          POWER_SELECTOR_Y,
                          POWER_SELECTOR_W,
                          POWER_SELECTOR_H},
              POWER_SELECTOR_LABEL_LEFT,
              POWER_SELECTOR_LABEL_RIGHT,
              &new_power,
              POWER_SELECTOR_MIN,
              POWER_SELECTOR_MAX);
    if (new_power != old_power) _opensimplex_set_power(new_power);

    // Invert controls
    GuiCheckBox((Rectangle){INVERT_SELECTOR_X,
                            INVERT_SELECTOR_Y,
                            INVERT_SELECTOR_W,
                            INVERT_SELECTOR_H},
                INVERT_SELECTOR_LABEL,
                &new_invert);
    if (new_invert != old_invert) _opensimplex_set_invert(new_invert);
    old_invert = new_invert;

    // Octaves controls
    GuiLabel((Rectangle){OCTAVES_SELECTOR_LABEL_X,
                         OCTAVES_SELECTOR_LABEL_Y,
                         OCTAVES_SELECTOR_LABEL_W,
                         OCTAVES_SELECTOR_LABEL_H},
             OCTAVES_SELECTOR_LABEL);
    GuiSlider((Rectangle){OCTAVES_SELECTOR_X,
                          OCTAVES_SELECTOR_Y,
                          OCTAVES_SELECTOR_W,
                          OCTAVES_SELECTOR_H},
              OCTAVES_SELECTOR_LABEL_LEFT,
              OCTAVES_SELECTOR_LABEL_RIGHT,
              &new_octaves,
              OCTAVES_SELECTOR_MIN,
              OCTAVES_SELECTOR_MAX);
    new_octaves = roundf(new_octaves);
    if (new_octaves != old_octaves) _opensimplex_set_octaves((int)new_octaves);
    old_octaves = new_octaves;

    // Persistence controls
    GuiLabel((Rectangle){PERSISTENCE_SELECTOR_LABEL_X,
                         PERSISTENCE_SELECTOR_LABEL_Y,
                         PERSISTENCE_SELECTOR_LABEL_W,
                         PERSISTENCE_SELECTOR_LABEL_H},
             PERSISTENCE_SELECTOR_LABEL);
    GuiSlider((Rectangle){PERSISTENCE_SELECTOR_X,
                          PERSISTENCE_SELECTOR_Y,
                          PERSISTENCE_SELECTOR_W,
                          PERSISTENCE_SELECTOR_H},
              PERSISTENCE_SELECTOR_LABEL_LEFT,
              PERSISTENCE_SELECTOR_LABEL_RIGHT,
              &new_persistence,
              PERSISTENCE_SELECTOR_MIN,
              PERSISTENCE_SELECTOR_MAX);
    if (new_persistence != old_persistence)
        _opensimplex_set_persistence(new_persistence);
    old_persistence = new_persistence;

    // Lacunarity controls
    GuiLabel((Rectangle){LACUNARITY_SELECTOR_LABEL_X,
                         LACUNARITY_SELECTOR_LABEL_Y,
                         LACUNARITY_SELECTOR_LABEL_W,
                         LACUNARITY_SELECTOR_LABEL_H},
             LACUNARITY_SELECTOR_LABEL);
    GuiSlider((Rectangle){LACUNARITY_SELECTOR_X,
                          LACUNARITY_SELECTOR_Y,
                          LACUNARITY_SELECTOR_W,
                          LACUNARITY_SELECTOR_H},
              LACUNARITY_SELECTOR_LABEL_LEFT,
              LACUNARITY_SELECTOR_LABEL_RIGHT,
              &new_lacunarity,
              LACUNARITY_SELECTOR_MIN,
              LACUNARITY_SELECTOR_MAX);
    if (new_lacunarity != old_lacunarity)
        _opensimplex_set_lacunarity(new_lacunarity);
    old_lacunarity = new_lacunarity;

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
