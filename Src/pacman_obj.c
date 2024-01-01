#include <allegro5/allegro_primitives.h>
#include "allegro5/events.h"
#include "pacman_obj.h"
#include "map.h"
/* Static variables */
static const int start_grid_x = 25, start_grid_y = 25;		// where to put pacman at the beginning
static ALLEGRO_SAMPLE_ID PACMAN_MOVESOUND_ID;
// [ NOTE - speed ]
// If you want to implement something regarding speed.
// You may have to modify the basic_speed here.
// But before you modify this, make sure you are 
// totally understand the meaning of speed and function
// `step()` in `scene_game.c`, also the relationship between
// `speed`, `GAME_TICK`, `GAME_TICK_CD`, `objData->moveCD`.
static const int basic_speed = 2;

/* Shared variables */
extern ALLEGRO_SAMPLE* PACMAN_MOVESOUND;
extern ALLEGRO_SAMPLE* PACMAN_DEATH_SOUND;
extern uint32_t GAME_TICK;
extern uint32_t GAME_TICK_CD;
extern bool game_over;
extern float effect_volume;

/* Declare static function */
static bool pacman_movable(const Pacman* pacman, const Map* M, Directions targetDirec) {

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
        case NONE:
            dx = 0, dy = 0;
            break;
        default:
            return false;
    }
    if (!pacman->cheat && (is_wall_block(M, pacman->objData.Coord.x + dx, pacman->objData.Coord.y + dy) ||
        is_room_block(M, pacman->objData.Coord.x + dx, pacman->objData.Coord.y + dy)))
        return false;
	return true;
}

Pacman* pacman_create() {
	// Allocate dynamic memory for pman pointer;
	Pacman* pman = (Pacman*)malloc(sizeof(Pacman));
	if (!pman)
		return NULL;
	pman->objData.Coord.x = 24;
	pman->objData.Coord.y = 24;
	pman->objData.Size.x = block_width;
	pman->objData.Size.y = block_height;

	pman->objData.preMove = NONE;
	pman->objData.nextTryMove = NONE;
	pman->speed = basic_speed;

	pman->death_anim_counter = al_create_timer(1.0f / 8.0f);
	pman->powerUp = false;
    pman->cheat = false;
	/* load sprites */
	pman->move_sprite = load_bitmap("Assets/pacman_move.png");
	pman->die_sprite = load_bitmap("Assets/pacman_die.png");
	return pman;

}

void pacman_destroy(Pacman* pman) {	

    al_destroy_bitmap(pman->die_sprite);
    al_destroy_bitmap(pman->move_sprite);
    al_destroy_timer(pman->death_anim_counter);
    free(pman);
}


void pacman_draw(Pacman* pman, Submap *view, Submap *submap) {
	
    object fixed;
    fixed.Size = pman->objData.Size;
    fixed.moveCD = pman->objData.moveCD;
    fixed.preMove = pman->objData.preMove;
    fixed.Coord.x = pman->objData.Coord.x - submap->offset.x;
    fixed.Coord.y = pman->objData.Coord.y - submap->offset.y;

	RecArea drawArea = getDrawArea(&fixed, GAME_TICK_CD);

	//Draw default image
    
	int offset = 0;
    int facing = 0;
	if (!game_over) {
        if ((pman->objData.moveCD >> 4) & 1){
            offset = 16;
        } else {
            offset = 0;
        }
		switch(pman->objData.facing)
		{
        case DOWN:
            facing += 32;
        case UP:
            facing += 32;
		case LEFT:
            facing += 32;
		case RIGHT:
        default:
            break;
		}
        al_draw_scaled_bitmap(pman->move_sprite, facing + offset, 0,
            16, 16,
            drawArea.x, drawArea.y,
            block_width, block_height, 0
        );
	} else {
        int offset = al_get_timer_count(pman->death_anim_counter);
        al_draw_scaled_bitmap(pman->die_sprite, 16*offset, 0,
            16, 16,
            drawArea.x, drawArea.y,
            block_width, block_height, 0 
        );
	}
}
void pacman_turn_head(Pacman* pacman, Directions direction){
    pacman->objData.facing = direction;
}
void pacman_move(Pacman* pacman, Map* M) {
	if (!movetime(pacman->speed))
		return;
	if (game_over)
		return;

	int probe_x = pacman->objData.Coord.x, probe_y = pacman->objData.Coord.y;
	if (pacman_movable(pacman, M, pacman->objData.nextTryMove)) 
		pacman->objData.preMove = pacman->objData.nextTryMove;
	else if (!pacman_movable(pacman, M, pacman->objData.preMove)) 
		return;

	switch (pacman->objData.preMove)
	{
	case UP:
		pacman->objData.Coord.y -= 1;
		pacman->objData.nextTryMove = UP;
		break;
	case DOWN:
		pacman->objData.Coord.y += 1;
		pacman->objData.nextTryMove = DOWN;
		break;
	case LEFT:
		pacman->objData.Coord.x -= 1;
		pacman->objData.nextTryMove = LEFT;
		break;
	case RIGHT:
		pacman->objData.Coord.x += 1;
		pacman->objData.nextTryMove = RIGHT;
		break;
    case NONE:
        pacman->objData.nextTryMove = NONE;
	default:
		break;
	}
	pacman->objData.moveCD = GAME_TICK_CD;
}
void pacman_eatItem(Pacman* pacman, const char Item) {
	switch (Item)
	{
	case 'P':
        stop_bgm(PACMAN_MOVESOUND_ID);
        PACMAN_MOVESOUND_ID = play_audio(PACMAN_MOVESOUND, effect_volume);
		return;
	case '.':
		stop_bgm(PACMAN_MOVESOUND_ID);
		PACMAN_MOVESOUND_ID = play_audio(PACMAN_MOVESOUND, effect_volume);
		return;
	default:
		break;
	}
}

void pacman_NextMove(Pacman* pacman, Directions next) {
	pacman->objData.nextTryMove = next;
}

void pacman_die() {
	// TODO-GC-game_over: play sound of pacman's death! see shared.c
	// hint: check pacman_eatItem(...) above.
    stop_bgm(PACMAN_MOVESOUND_ID);
    play_audio(PACMAN_DEATH_SOUND, effect_volume);
}
