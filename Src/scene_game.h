// Functions without 'static', 'extern' prefixes is just a normal
// function, they can be accessed by other files using 'extern'.
// Define your normal function prototypes below.

#ifndef SCENE_GAME_H
#define SCENE_GAME_H
#include "game.h"
#include "pacman_obj.h"
#include "ghost.h"
#include "map.h"
#include "scene_menu.h"

Scene scene_main_create(void);
int64_t get_power_up_timer_tick();
int64_t get_power_up_duration();
uint32_t get_score();

#endif
