#include "layout.h"
#include "defs.h"
#include "opensimplex.h"
#include "raylib.h"
#include <raygui.h>
#include <stdbool.h>

static int seed = TEX_START_SEED;
static int res_x = TEX_START_RES_X;
static int res_y = TEX_START_RES_Y;

static bool editing_seed = false;
static bool editing_res_x = false;
static bool editing_res_y = false;

void _layout_draw() {
    Texture2D opensimplex = _opensimplex_get();

    BeginDrawing();
    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    // App name and version at the top
    GuiLabel((Rectangle){APP_NAME_LABEL_X,
                         APP_NAME_LABEL_Y,
                         APP_NAME_LABEL_W,
                         APP_NAME_LABEL_H},
             APP_NAME " - " APP_VERSION);

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
        (Rectangle){0, 0, (float)opensimplex.width, -(float)opensimplex.height},
        (Rectangle){TEX_BOX_X, TEX_BOX_Y, (float)tex_w, (float)tex_h},
        (Vector2){0, 0},
        0.0f,
        WHITE);

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

    EndDrawing();
}
