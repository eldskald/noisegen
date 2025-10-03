#pragma once
#include <raylib.h>
#include <stdbool.h>

void _opensimplex_init();
void _opensimplex_stop();

void _opensimplex_set_seed(int new_val);
void _opensimplex_set_res(int new_val);
void _opensimplex_set_seamless(bool new_val);
void _opensimplex_set_freq(float new_val);
void _opensimplex_set_range_min(float new_val);
void _opensimplex_set_range_max(float new_val);

Texture2D _opensimplex_get();
