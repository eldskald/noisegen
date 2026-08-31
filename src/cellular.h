#pragma once
#include <raylib.h>
#include <stdbool.h>

typedef enum { mode_one, mode_two } mode;

void _cellular_init();
void _cellular_stop();

void _cellular_set_seed(int new_val);
void _cellular_set_res(int new_x, int new_y);
void _cellular_set_mode(int new_val);
void _cellular_set_squared(bool new_val);
void _cellular_set_jitter(float new_val);
void _cellular_set_seamless(bool new_val);
void _cellular_set_freq(float new_val);
void _cellular_set_range_min(float new_val);
void _cellular_set_range_max(float new_val);
void _cellular_set_power(float new_val);
void _cellular_set_invert(bool new_val);
void _cellular_set_octaves(int new_val);
void _cellular_set_persistence(float new_val);
void _cellular_set_lacunarity(float new_val);

Texture2D _cellular_get();
