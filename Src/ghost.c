#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include "ghost.h"
#include "map.h"
#include "pacman_obj.h"
#include "scene_game.h" 

/* global variables*/
// [ NOTE ]
// if you change the map .txt to your own design.
// You have to modify cage_grid_{x,y} to corressponding value also.
// Or you can do some change while loading map (reading .txt file)
// Make the start position metadata stored with map.txt.
/* shared variables. */
extern uint32_t GAME_TICK;
extern uint32_t GAME_TICK_CD;
extern const int block_width,  block_height;
/* Internal variables */
static const int basic_speed = 2;

Ghost* ghost_create(int flag, Pair_IntInt position) {

	// NOTODO
	Ghost* ghost = (Ghost*)malloc(sizeof(Ghost));
	if (!ghost)
		return NULL;

    ghost->countdown = 0;
	ghost->go_in_time = GAME_TICK; 
	ghost->typeFlag = flag;
	ghost->objData.Size.x = block_width;
	ghost->objData.Size.y = block_height;

	ghost->objData.nextTryMove = NONE;
	ghost->speed = basic_speed;
	ghost->status = BLOCKED;
    ghost->drawn = false;
    ghost->stop = false;

	ghost->flee_sprite = load_bitmap("Assets/ghost_flee.png");
	ghost->dead_sprite = load_bitmap("Assets/ghost_dead.png");
    ghost->objData.Coord.x = position.x;
    ghost->objData.Coord.y = position.y;
	switch (ghost->typeFlag) {
	case Blinky:
		ghost->move_sprite = load_bitmap("Assets/ghost_move_red.png");
		ghost->move_script = &ghost_move_script_shortest_path;
		break;
	case Pinky:
		ghost->move_sprite = load_bitmap("Assets/ghost_move_pink.png");
		ghost->move_script = &ghost_move_script_shortest_path;
        break;
    case Inky:
		ghost->move_sprite = load_bitmap("Assets/ghost_move_orange.png");
		ghost->move_script = &ghost_move_script_shortest_path;
        break;
    case Clyde:
		ghost->move_sprite = load_bitmap("Assets/ghost_move_blue.png");
		ghost->move_script = &ghost_move_script_shortest_path;
        break;
	default:
		ghost->move_sprite = load_bitmap("Assets/ghost_move_red.png");
		ghost->move_script = &ghost_move_script_random;
    break;
	}
	return ghost;
}
void ghost_destroy(Ghost* ghost) {

    al_destroy_bitmap(ghost->move_sprite);
    al_destroy_bitmap(ghost->dead_sprite);
    al_destroy_bitmap(ghost->flee_sprite);

    free(ghost);
}
void ghost_draw(Ghost* ghost, Submap* view, Submap* submap) {
    object fixed;
    fixed.Size = ghost->objData.Size;
    fixed.moveCD = ghost->objData.moveCD;
    fixed.preMove = ghost->objData.preMove;
    fixed.Coord.x = ghost->objData.Coord.x - submap->offset.x;
    fixed.Coord.y = ghost->objData.Coord.y - submap->offset.y;
    if (fixed.Coord.x < 0 || fixed.Coord.y < 0 ||
        fixed.Coord.x >= submap->offset.x + submap->col_num ||
        fixed.Coord.y >= submap->offset.y + submap->row_num){
        ghost->drawn = false; 
        ghost->shown = false;
        return;
    }
	RecArea drawArea = getDrawArea(&fixed, GAME_TICK_CD);
    ghost->drawn = true;
    ghost->shown = true;

	//Draw default image
	int bitmap_x_offset = 0;
    int offset = 0;
    switch (ghost->status){
    case FLEE:
        if (get_power_up_timer_tick() >= 0.7*get_power_up_duration())
        {
            if (get_power_up_timer_tick() & 1) {
                bitmap_x_offset = 32 - bitmap_x_offset;
            }
        }
        al_draw_scaled_bitmap(ghost->flee_sprite, bitmap_x_offset, 0,
            16, 16,
            drawArea.x, drawArea.y,
            block_width, block_height, 0 
        );
        break;
    case GO_IN:
		switch (ghost->objData.facing)
		{
        case DOWN:
            bitmap_x_offset += 16;
        case UP:
            bitmap_x_offset += 16;
        case LEFT:
            bitmap_x_offset += 16;
        case RIGHT:
        default:
            break;
        }
        al_draw_scaled_bitmap(ghost->dead_sprite, bitmap_x_offset, 0,
            16, 16,
            drawArea.x, drawArea.y,
            block_width, block_height, 0 
        );
        break;
    default:
        if (ghost->speed < 4){
            if (ghost->objData.Coord.x < view->offset.x || 
                ghost->objData.Coord.x >= view->offset.x + view->col_num ||
                ghost->objData.Coord.y < view->offset.y ||
                ghost->objData.Coord.y >= view->offset.y + view->row_num){
                    ghost->shown = false;
                    return;
            }
        }
        if ((ghost->objData.moveCD >> 5) & 1) offset = 16;
        else offset = 0;
		switch (ghost->objData.facing)
		{
        case DOWN:
            bitmap_x_offset += 32;
        case UP:
            bitmap_x_offset += 32;
        case LEFT:
            bitmap_x_offset += 32;
        case RIGHT:
        default:
            break;
        }
        al_draw_scaled_bitmap(ghost->move_sprite, offset + bitmap_x_offset, 0,
            16, 16,
            drawArea.x, drawArea.y,
            block_width, block_height, 0 
        );
        break;
    }
}
void ghost_NextMove(Ghost* ghost, Directions next) {
	ghost->objData.nextTryMove = next;
}
void printGhostStatus(GhostStatus S) {

	switch(S){
	
	case BLOCKED: // stay inside the ghost room
		game_log("BLOCKED");
		break;
	case GO_OUT: // going out the ghost room
		game_log("GO_OUT");
		break;
	case FREEDOM: // free at the map
		game_log("FREEDOM");
		break;
	case GO_IN:
		game_log("GO_IN");
		break;
	case FLEE:
		game_log("FLEE");
		break;
	default:
		game_log("status error");
		break;
	}
}
bool ghost_movable(const Ghost* ghost, const Map* M, Directions targetDirec, bool room) {

    int dx, dy;
    switch (targetDirec){
        case UP:
            dx = 0, dy = -1;
            break;
        case DOWN:
            dx = 0, dy = 1;
            break;
        case LEFT:
            dx = -1, dy = 0;
            break;
        case RIGHT:
            dx = 1, dy = 0;
            break;
        default:
            return false;
    }
    if (is_wall_block(M, ghost->objData.Coord.x + dx, ghost->objData.Coord.y + dy))
        return false;
    if (room){
        if (is_room_block(M, ghost->objData.Coord.x + dx, ghost->objData.Coord.y + dy))
            return false;
    }
	return true;

}

void ghost_toggle_FLEE(Ghost* ghost, bool setFLEE) {
	if(setFLEE){
		// set FREEDOM ghost's status to FLEE and make them slow 
		if(ghost->status == FREEDOM){
            ghost->status = FLEE;
			ghost->speed = 1;
		}
	}else{
		// set FLEE ghost's status to FREESOME and them down
		if(ghost->status == FLEE){
            ghost->status = FREEDOM;
			ghost->speed = 2;
		}
	}
}

void ghost_collided(Ghost* ghost) {
	if (ghost->status == FLEE) {
		ghost->status = GO_IN;
		ghost->speed = 4; // Go back to cage faster
	}
}

