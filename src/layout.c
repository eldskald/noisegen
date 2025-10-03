#include "layout.h"
#include "defs.h"
#include "opensimplex.h"
#include "raylib.h"
#include <raygui.h>

void _layout_draw() {
    Texture2D opensimplex = _opensimplex_get();

    BeginDrawing();
    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    DrawTexturePro(
        opensimplex,
        (Rectangle){0, 0, (float)opensimplex.width, -(float)opensimplex.height},
        (Rectangle){TEX_BOX_X, TEX_BOX_Y, TEX_BOX_W, TEX_BOX_H},
        (Vector2){0, 0},
        0.0f,
        WHITE);

    EndDrawing();
}
