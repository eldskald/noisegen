#include "manager.h"
#include "defs.h"
#include "layout.h"
#include "opensimplex.h"
#include <raygui.h>
#include <raylib.h>

void _manager_init() {
#ifndef DEV
    SetTraceLogLevel(LOG_NONE);
#endif
    InitWindow(WINDOW_SIZE_X, WINDOW_SIZE_Y, APP_NAME " " APP_VERSION);

    _layout_init();
    _opensimplex_init();

    SetExitKey(KEY_NULL);
    GuiLoadStyle("res/style_dark.rgs");
#ifndef WEB
    SetTargetFPS(WINDOW_FPS);
#endif
}

void _manager_stop() {
    _opensimplex_stop();
    CloseWindow();
}

void _manager_tick() {
    _layout_draw();
}
