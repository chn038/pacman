#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <string.h>
#include "allegro5/events.h"
#include "game.h"
#include "shared.h"
#include "utility.h"
#include "scene_game.h"
#include "scene_menu.h"
#include "pacman_obj.h"
#include "ghost.h"
#include "map.h"


extern const uint32_t GAME_TICK_CD;
extern uint32_t GAME_TICK;
extern ALLEGRO_TIMER* game_tick_timer;
int MAX_GHOST_NUM = 5; 
Pair_IntInt vision = {8, 3};
Pair_IntInt range = {16, 16};
int game_main_Score = 0;
int power_counter = 0;
bool game_over = false;

static ALLEGRO_TIMER* power_up_timer;
static const int power_up_duration = 10;
static Pacman* pman;
static Map* basic_map;
static Submap* view_map;
static Submap* submap;
static Ghost** ghosts;
static uint32_t ghost_count;
static char warning[50];
static Pair_IntInt center;
bool debug_mode = false;
bool cheat_mode = false;

static void init(void);
static void step(void);
static void checkItem(void);
static void status_update(void);
static void update(void);
static void draw(void);
static void printinfo(void);
static void destroy(void);
static void on_key_down(int key_code);
static void on_mouse_down(int btn, int x, int y, int dz);
static void render_init_screen(void);
static void draw_hitboxes(void);
static void power_up(void);

static void init(void) {
	game_over = false;
	game_main_Score = 0;
    power_counter = 0;
    ghost_count = 0;

	basic_map = create_map("Assets/Map/map_nthu.txt");
	if (!basic_map) {
        game_log("Error loading map, use default one.");
        basic_map = create_map(0);
        if (!basic_map)
            game_abort("error on creating map");
	}	
    view_map = create_submap(basic_map, vision);
    if (!view_map){
        game_abort("Error creating view_map");
    }
    submap = create_submap(basic_map, range);
    if (!submap){
        game_abort("Error creating submap");
    }
    center = make_pair(basic_map->col_num / 2, basic_map->row_num / 2.0);

	pman = pacman_create(basic_map->start_grid);
	if (!pman) {
		game_abort("error on creating pacamn\n");
	}
	
	ghosts = (Ghost**)malloc(sizeof(Ghost*) * MAX_GHOST_NUM);
	if(!ghosts){
		game_log("We haven't create any ghosts!\n");
	}
	else {
        ghosts[ghost_count] = ghost_create(ghost_count, basic_map->cage_grid);  
        if (!ghosts[ghost_count])
            game_abort("error creating ghost\n");
        ghost_count += 1;
	}
	GAME_TICK = 0;
    update_submap(view_map, make_pair(pman->objData.Coord.x + 3, pman->objData.Coord.y), vision, true);
    update_submap(submap, make_pair(pman->objData.Coord.x, pman->objData.Coord.y), range, false);
	render_init_screen();
	power_up_timer = al_create_timer(1.0f); // 1 tick per second
	if (!power_up_timer)
		game_abort("Error on create timer\n");
	return;
}

static void step(void) {
	if (pman->objData.moveCD > 0)
		pman->objData.moveCD -= pman->speed;
	for (int i = 0; i < ghost_count; i++) {
		if (ghosts[i]->objData.moveCD > 0)
			ghosts[i]->objData.moveCD -= ghosts[i]->speed;
	}
}
static void checkItem(void) {
	int Grid_x = pman->objData.Coord.x, Grid_y = pman->objData.Coord.y;
	if (Grid_y >= basic_map->row_num - 1 || Grid_y <= 0 || Grid_x >= basic_map->col_num - 1 || Grid_x <= 0)
		return;
	switch (basic_map->map[Grid_y][Grid_x])
	{
	case '.':
		pacman_eatItem(pman, '.');
        basic_map->beansNum -= 1;
        game_main_Score += 10;
        break;
	case 'P':
        pacman_eatItem(pman, 'P');
        basic_map->beansNum -= 1;
        game_main_Score += 20;
        power_counter++;
		break;
	default:
		break;
	}
    if(!(is_room_block(basic_map, Grid_x, Grid_y)) || !(is_wall_block(basic_map, Grid_x, Grid_y))){
        basic_map->map[Grid_y][Grid_x] = ' ';
    }
    
}

static void power_up(void){
    if (!power_counter) return;
    al_stop_timer(power_up_timer);
    al_set_timer_count(power_up_timer, 0);
    al_start_timer(power_up_timer);
    for (int i = 0; i < ghost_count; ++i)
            ghost_toggle_FLEE(ghosts[i], true);
    pman->powerUp = true;
    power_counter--;
    return;
}

static void status_update(void) {
	if (pman->powerUp)
	{
        if (al_get_timer_count(power_up_timer) == power_up_duration) {
            for (int i = 0; i < ghost_count; ++i){
                ghost_toggle_FLEE(ghosts[i], false);
            }
            al_stop_timer(power_up_timer);
            al_set_timer_count(power_up_timer, 0);
            pman->powerUp = false;
        }
	}

    if (game_over) return;

	for (int i = 0; i < ghost_count; i++) {
        if (game_over) break;
        RecArea pmanHB = getDrawArea((object *)pman, GAME_TICK_CD);
        RecArea ghostHB = getDrawArea((object*)(ghosts[i]), GAME_TICK_CD);
        if (RecAreaOverlap(&pmanHB, &ghostHB)){
            switch (ghosts[i]->status){
                case FREEDOM:
                    if (cheat_mode){
                        game_log("Cheat mode enabled, nothing happened.");
                        break;
                    }
                    game_log("collide with ghost %d", i);
                    al_rest(1.0);
                    pacman_die();
                    game_over = true;
                    al_stop_timer(pman->death_anim_counter);
                    al_start_timer(pman->death_anim_counter);
                    break;
                case FLEE:
                    game_log("Power mode is active.");
                    ghost_collided(ghosts[i]);
                    game_main_Score += 100;
                    break;
                case GO_IN:
                case GO_OUT:
                case BLOCKED:
                    continue;
            }
        }
	}

    if (ghost_count == MAX_GHOST_NUM) return;
    float diff = MAX_GHOST_NUM * 0.1;
    GhostType newGhost;
    if (ghost_count >= diff * 8)
        newGhost = Blinky;
    else if (ghost_count >= diff * 5.5)
        newGhost = Clyde;
    else if (ghost_count >= diff * 3)
        newGhost = Pinky;
    else
        newGhost = Inky;

    if (basic_map->beansCount - basic_map->beansNum == basic_map->beansCount / MAX_GHOST_NUM * ghost_count){
        ghosts[ghost_count] = ghost_create(newGhost, basic_map->cage_grid);
        if (!ghosts[ghost_count]){
            game_log("Failed to create ghost %d", ghost_count);
        }
        ghost_count++;
    }
}

static void update(void) {

	if (game_over) {
        if (al_get_timer_count(pman->death_anim_counter) == 12)
            game_change_scene(scene_menu_create());
		return;
	}

	step();
	checkItem();
	status_update();
	pacman_move(pman, basic_map);
	for (int i = 0; i < ghost_count; i++) 
		ghosts[i]->move_script(ghosts[i], basic_map, pman);
    Pair_IntInt offset;
    Pair_IntInt dire;
    switch (pman->objData.facing){
    case LEFT:
        dire.x = vision.x;
        dire.y = vision.y;
        offset.x = -(dire.x / 2) + 2;
        offset.y = 0;
        break;
    case RIGHT:
        dire.x = vision.x;
        dire.y = vision.y;
        offset.x = (dire.x / 2) - 1;
        offset.y = 0;
        break;
    case UP:
        dire.x = vision.y;
        dire.y = vision.x;
        offset.x = 0;
        offset.y = -(dire.y / 2) + 2;
        break;
    case DOWN:
        dire.x = vision.y;
        dire.y = vision.x;
        offset.x = 0;
        offset.y = (dire.y / 2) - 1;
        break;
    default:
        offset.x = 0;
        offset.y = 0;
        dire.x = vision.x;
        dire.y = vision.y;
        break;
    }
    update_submap(view_map, make_pair(pman->objData.Coord.x + offset.x, pman->objData.Coord.y + offset.y), dire, true);
    update_submap(submap, make_pair(pman->objData.Coord.x, pman->objData.Coord.y), range, false);
}

static void draw(void) {

	al_clear_to_color(al_map_rgb(0, 0, 0));

	static char scoreboard[50];
    static char progress_indicator[50];
    static char powerup[50];

    sprintf(scoreboard, "Score: %d", game_main_Score);
    sprintf(progress_indicator, "LEFT: %d", basic_map->beansNum);
    sprintf(powerup, "(E)Power Up: %d", power_counter);

    al_draw_text(menuFont, al_map_rgb(255, 255, 255), (double)block_width*submap->col_num/2, block_height*submap->row_num + map_offset_y, ALLEGRO_ALIGN_CENTER, scoreboard); 
    al_draw_text(menuFont, al_map_rgb(255, 255, 255), (double)block_width*submap->col_num/2, block_height*submap->row_num + map_offset_y + fontSize, ALLEGRO_ALIGN_CENTER, progress_indicator); 
        al_draw_text(menuFont, al_map_rgb(255, 255, 255), block_width*submap->col_num - (fontSize * 5), block_height*submap->row_num + map_offset_y , ALLEGRO_ALIGN_CENTER, powerup); 
    if (cheat_mode)
        al_draw_text(menuFont, al_map_rgb(255, 255, 255), fontSize * 5, block_height*submap->row_num + map_offset_y, ALLEGRO_ALIGN_CENTER, "Cheat Mode"); 
    if (debug_mode)
        al_draw_text(menuFont, al_map_rgb(255, 255, 255), fontSize * 5, block_height*submap->row_num + map_offset_y + fontSize, ALLEGRO_ALIGN_CENTER, "Debug Mode"); 
    

    draw_submap(view_map, submap->offset);

	pacman_draw(pman, view_map, submap);
	if (game_over)
		return;
	// no drawing below when game over
	for (int i = 0; i < ghost_count; i++)
		ghost_draw(ghosts[i], view_map, submap);
	
	//debugging mode
	if (debug_mode) {
		draw_hitboxes();
	}

}

static void draw_hitboxes(void) {
    object fixed;
    fixed.Size = pman->objData.Size;
    fixed.moveCD = pman->objData.moveCD;
    fixed.preMove = pman->objData.preMove;
    fixed.Coord.x = pman->objData.Coord.x - submap->offset.x;
    fixed.Coord.y = pman->objData.Coord.y - submap->offset.y;
	RecArea pmanHB = getDrawArea(&fixed, GAME_TICK_CD);
	al_draw_rectangle(
		pmanHB.x, pmanHB.y,
		pmanHB.x + pmanHB.w, pmanHB.y + pmanHB.h,
		al_map_rgb_f(1.0, 0.0, 0.0), 2
	);

	for (int i = 0; i < ghost_count; i++) {
        if (!ghosts[i]->drawn) continue;
        fixed.Size = ghosts[i]->objData.Size;
        fixed.moveCD = ghosts[i]->objData.moveCD;
        fixed.preMove = ghosts[i]->objData.preMove;
        fixed.Coord.x = ghosts[i]->objData.Coord.x - submap->offset.x;
        fixed.Coord.y = ghosts[i]->objData.Coord.y - submap->offset.y;
		RecArea ghostHB = getDrawArea(&fixed, GAME_TICK_CD);
		al_draw_rectangle(
			ghostHB.x, ghostHB.y,
			ghostHB.x + ghostHB.w, ghostHB.y + ghostHB.h,
			al_map_rgb_f(1.0, 0.0, 0.0), 2
		);
	}

}

static void printinfo(void) {
	game_log("pacman:\n");
	game_log("coord: %d, %d\n", pman->objData.Coord.x, pman->objData.Coord.y);
	game_log("PreMove: %d\n", pman->objData.preMove);
	game_log("NextTryMove: %d\n", pman->objData.nextTryMove);
	game_log("Speed: %f\n", pman->speed);
}


static void destroy(void) {
    for (int i = 0; i < ghost_count; ++i)
        ghost_destroy(ghosts[i]);
    pacman_destroy(pman);
    free(ghosts);
    delete_map(basic_map);
    delete_submap(view_map);
    delete_submap(submap);
}

static void on_key_down(int key_code) {
	switch (key_code)
	{
    case ALLEGRO_KEY_W:
        pacman_NextMove(pman, UP);
        pacman_turn_head(pman, UP);
        break;
    case ALLEGRO_KEY_A:
        pacman_NextMove(pman, LEFT);
        pacman_turn_head(pman, LEFT);
        break;
    case ALLEGRO_KEY_S:
        pacman_NextMove(pman, DOWN);
        pacman_turn_head(pman, DOWN);
        break;
    case ALLEGRO_KEY_D:
        pacman_NextMove(pman, RIGHT);
        pacman_turn_head(pman, RIGHT);
        break;
    case ALLEGRO_KEY_I:
        pacman_turn_head(pman, UP);
        break;
    case ALLEGRO_KEY_J:
        pacman_turn_head(pman, LEFT);
        break;
    case ALLEGRO_KEY_K:
        pacman_turn_head(pman, DOWN);
        break;
    case ALLEGRO_KEY_L:
        pacman_turn_head(pman, RIGHT);
        break;
    case ALLEGRO_KEY_C:
        cheat_mode = !cheat_mode;
        if (cheat_mode)
            printf("cheat mode on\n");
        else 
            printf("cheat mode off\n");
        break;
    case ALLEGRO_KEY_G:
        debug_mode = !debug_mode;
        if (debug_mode)
            printf("debug mode on\n");
        else 
            printf("debug mode off\n");
        break;
    case ALLEGRO_KEY_E:
        power_up();
        break;
    case ALLEGRO_KEY_Q:
        pacman_NextMove(pman, NONE);
        break;
	default:
		break;
	}

}

static void on_mouse_down(int btn, int x, int y, int dz) {

	// nothing here

}

static void render_init_screen(void) {
	al_clear_to_color(al_map_rgb(0, 0, 0));

	draw_submap(view_map, submap->offset);
	pacman_draw(pman, view_map, submap);
	for (int i = 0; i < ghost_count; i++) {
		ghost_draw(ghosts[i], view_map, submap);
	}

	al_draw_text(
		menuFont,
		al_map_rgb(255, 255, 0),
		400 , 400,
		ALLEGRO_ALIGN_CENTER,
		"READY!"
	);

	al_flip_display();
	al_rest(2.0);
    game_log("Game initialization finished.\n");

}
// Functions without 'static', 'extern' prefixes is just a normal
// function, they can be accessed by other files using 'extern'.
// Define your normal function prototypes below.

// The only function that is shared across files.
Scene scene_main_create(void) {
	Scene scene;
	memset(&scene, 0, sizeof(Scene));
	scene.name = "Start";
	scene.initialize = &init;
	scene.update = &update;
	scene.draw = &draw;
	scene.destroy = &destroy;
	scene.on_key_down = &on_key_down;
	scene.on_mouse_down = &on_mouse_down;
	// TODO-IF: Register more event callback functions such as keyboard, mouse, ...
	game_log("Start scene created");
	return scene;
}

int64_t get_power_up_timer_tick(){
    return al_get_timer_count(power_up_timer);
}

int64_t get_power_up_duration(){
    return power_up_duration;
}
