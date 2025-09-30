#include "manager.h"
#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#ifdef WEB
#include <emscripten/emscripten.h>
#endif

int main() {
    _manager_init();

#ifdef WEB
    emscripten_set_main_loop(_manager_tick, 0, 1);
#else
    while (!WindowShouldClose()) {
        _manager_tick();
    }
#endif

    _manager_stop();

    return 0;
}
