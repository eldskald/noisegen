#include "manager.h"
#include "defs.h"
#include <raygui.h>
#include <raylib.h>

void _manager_init() {
#ifndef DEV
    SetTraceLogLevel(LOG_NONE);
#endif

    InitWindow(WINDOW_SIZE_X, WINDOW_SIZE_Y, WINDOW_TITLE);
    SetExitKey(KEY_NULL);
    GuiLoadStyle("res/style_dark.rgs");
#ifndef WEB
    SetTargetFPS(WINDOW_FPS);
#endif
}

void _manager_stop() {
    CloseWindow();
}

void _manager_tick() {
    BeginDrawing();
    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
    GuiLabel((Rectangle){0, 0, WINDOW_SIZE_X, WINDOW_SIZE_Y}, "Hello world!");
    EndDrawing();
}
