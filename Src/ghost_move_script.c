
#include "ghost.h"
#include "pacman_obj.h"
#include "map.h"
/* Shared variables */
#define GO_OUT_TIME 256
extern uint32_t GAME_TICK_CD;
extern uint32_t GAME_TICK;
extern ALLEGRO_TIMER* game_tick_timer;

/* Declare static function prototypes */
// static function reference: https://stackoverflow.com/questions/558122/what-is-a-static-function-in-c
static void ghost_move_script_FREEDOM_random(Ghost* ghost, Map* M);
static void ghost_move_script_FREEDOM_shortest_path_Blinky(Ghost* ghost, Map* M, Pacman* pman);
static void ghost_move_script_FREEDOM_shortest_path_Inky(Ghost* ghost, Map* M, Pacman* pman);
static void ghost_move_script_FREEDOM_shortest_path_Pinky(Ghost* ghost, Map* M, Pacman* pman);
static void ghost_move_script_FREEDOM_shortest_path_Clyde(Ghost* ghost, Map* M, Pacman* pman);
static void ghost_move_script_FREEDOM_shortest_path(Ghost* ghost, Map* M, Pacman* pman);
static void ghost_move_script_BLOCKED(Ghost* ghost, Map* M);
static void ghost_move_script_GO_IN(Ghost* ghost, Map* M);
static void ghost_move_script_GO_OUT(Ghost* ghost, Map* M);
static void ghost_move_script_FLEE(Ghost* ghost, Map* M, const Pacman* const pacman);

static void ghost_move_script_FREEDOM_random(Ghost* ghost, Map* M) {
    Directions counter;
    switch (ghost->objData.preMove){
        case RIGHT:
            counter = LEFT;
            break;
        case LEFT:
            counter = RIGHT;
            break;
        case UP:
            counter = DOWN;
            break;
        case DOWN:
            counter = UP;
            break;
        default:
            break;
    }
	static Directions proba[4]; // possible movement
	int cnt = 0;
	for (Directions i = 1; i <= 4; i++)
		if (i != counter && ghost_movable(ghost, M, i, 0)) 	proba[cnt++] = i;
    if (cnt)
        ghost_NextMove(ghost, proba[generateRandomNumber(0, cnt)]);
    else
        ghost_NextMove(ghost, counter);

}

static void ghost_move_script_FREEDOM_shortest_path_Blinky(Ghost* ghost, Map* M, Pacman* pman)
{
    ghost_move_script_FREEDOM_shortest_path(ghost, M, pman);
}

static void ghost_move_script_FREEDOM_shortest_path_Inky(Ghost* ghost, Map* M, Pacman* pman)
{
    ghost_move_script_FREEDOM_random(ghost, M);
}

static void ghost_move_script_FREEDOM_shortest_path_Pinky(Ghost* ghost, Map* M, Pacman* pman)
{
    if (ghost->drawn)
        ghost_move_script_FREEDOM_shortest_path(ghost, M, pman);
    else
        ghost_move_script_FREEDOM_random(ghost, M);
}

static void ghost_move_script_FREEDOM_shortest_path_Clyde(Ghost* ghost, Map* M, Pacman* pman)
{
    int facingx, facingy;
    switch (pman->objData.facing){
    case LEFT:
        facingx = -2;
        facingy = 0;
        break;
    case RIGHT:
        facingx = 2;
        facingy = 0;
        break;
    case UP:
        facingx = 0;
        facingy = -2;
        break;
    case DOWN:
        facingx = 0;
        facingy = 2;
        break;
    default:
        facingx = facingy = 0;
    }
    Directions shortestDirection = shortest_path_direc(M, ghost->objData.Coord.x, ghost->objData.Coord.y, pman->objData.Coord.x + facingx, pman->objData.Coord.y + facingy);
    if (ghost_movable(ghost, M, shortestDirection, 0))
        ghost->objData.nextTryMove = shortestDirection;
    else
        ghost_move_script_FREEDOM_shortest_path(ghost, M, pman);
}

static void ghost_move_script_FREEDOM_shortest_path(Ghost* ghost, Map* M, Pacman* pman)
{
	ghost->objData.nextTryMove = shortest_path_direc(M, ghost->objData.Coord.x, ghost->objData.Coord.y, pman->objData.Coord.x, pman->objData.Coord.y);
}

static void ghost_move_script_BLOCKED(Ghost* ghost, Map* M) {

	switch (ghost->objData.preMove)
	{
	case UP:
		if (ghost->objData.Coord.y == 10)
			ghost_NextMove(ghost, DOWN);
		else
			ghost_NextMove(ghost, UP);
		break;
	case DOWN:
		if (ghost->objData.Coord.y == 12)
			ghost_NextMove(ghost, UP);
		else
			ghost_NextMove(ghost, DOWN);
		break;
	default:
		ghost_NextMove(ghost, UP);
		break;
	}
}

static void ghost_move_script_GO_IN(Ghost* ghost, Map* M) {
	// Description
	// `shortest_path_direc` is a function that returns the direction of shortest path.
	// Check `map.c` for its detail usage.
	// For GO_IN state.
	ghost->objData.nextTryMove = shortest_path_direc(M, ghost->objData.Coord.x, ghost->objData.Coord.y, M->cage_grid.x, M->cage_grid.y);
}

static void ghost_move_script_GO_OUT(Ghost* ghost, Map* M) {
	// Description
	// Here we always assume the room of ghosts opens upward.
	// And used a greedy method to drag ghosts out of room.
	// You should modify here if you have different implementation/design of room.
	if(M->map[ghost->objData.Coord.y][ghost->objData.Coord.x] == 'B') 
		ghost_NextMove(ghost, UP);
	else
		ghost->status = FREEDOM;
}

static void ghost_move_script_FLEE(Ghost* ghost, Map* M, const Pacman * const pacman) {
	Directions shortestDirection = shortest_path_direc(M, ghost->objData.Coord.x, ghost->objData.Coord.y, pacman->objData.Coord.x, pacman->objData.Coord.y);
    switch (shortestDirection){
    case LEFT:
        shortestDirection = RIGHT;
        break;
    case RIGHT:
        shortestDirection = LEFT;
        break;
    case UP:
        shortestDirection = DOWN;
        break;
    case DOWN:
        shortestDirection = UP;
        break;
    default:
        break;
    }
    ghost->objData.nextTryMove = shortestDirection;

}

void ghost_move_script_random(Ghost* ghost, Map* M, Pacman* pacman) {
	if (!movetime(ghost->speed))
		return;
    switch (ghost->status)
    {
		case BLOCKED:
			ghost_move_script_BLOCKED(ghost, M);
			if (al_get_timer_count(game_tick_timer) > GO_OUT_TIME)
				ghost->status = GO_OUT;
			break;
		case FREEDOM:
			ghost_move_script_FREEDOM_random(ghost, M);
			break;
		case GO_OUT:
			ghost_move_script_GO_OUT(ghost, M);
			break;
		case GO_IN:
			ghost_move_script_GO_IN(ghost, M);
			if (M->map[ghost->objData.Coord.y][ghost->objData.Coord.x] == 'B') {
				ghost->status = GO_OUT;
				ghost->speed = 2; // reset the speed after back to the cage.
			}
			break;
		case FLEE:
			ghost_move_script_FLEE(ghost, M, pacman);
			break;
		default:
			break;
    }

    if(ghost_movable(ghost, M, ghost->objData.nextTryMove, false)){
        ghost->objData.preMove = ghost->objData.nextTryMove;
        ghost->objData.nextTryMove = NONE;
    }
    else if (!ghost_movable(ghost, M, ghost->objData.preMove, false))
        return;

    switch (ghost->objData.preMove) {
    case RIGHT:
        ghost->objData.Coord.x += 1;
        break;
    case LEFT:
        ghost->objData.Coord.x -= 1;
        break;
    case UP:
        ghost->objData.Coord.y -= 1;
        break;
    case DOWN:
        ghost->objData.Coord.y += 1;
        break;
    default:
        break;
    }
    ghost->objData.facing = ghost->objData.preMove;
    ghost->objData.moveCD = GAME_TICK_CD;
}

void ghost_move_script_shortest_path(Ghost* ghost, Map* M, Pacman* pacman) {
	// TODO-GC-movement: do a little modification on shortest path move script
	// Since always shortest path strategy is too strong, player have no chance to win this.
	// hint: Do shortest path sometime and do random move sometime.
	if (!movetime(ghost->speed))
		return;
    switch (ghost->status)
    {
    case BLOCKED:
        ghost_move_script_BLOCKED(ghost, M);
        if (al_get_timer_count(game_tick_timer) - ghost->go_in_time > GO_OUT_TIME)
            ghost->status = GO_OUT;
        break;
    case FREEDOM:
        switch (ghost->typeFlag){
            case Blinky:
                ghost_move_script_FREEDOM_shortest_path_Blinky(ghost, M, pacman);
                break;
            case Inky:
                ghost_move_script_FREEDOM_shortest_path_Inky(ghost, M, pacman);
                break;
            case Pinky:
                ghost_move_script_FREEDOM_shortest_path_Pinky(ghost, M, pacman);
                break;
            case Clyde:
                ghost_move_script_FREEDOM_shortest_path_Clyde(ghost, M, pacman);
                break;
        }
        break;
    case GO_OUT:
        ghost_move_script_GO_OUT(ghost, M);
        break;
    case GO_IN:
        ghost_move_script_GO_IN(ghost, M);
        if (M->map[ghost->objData.Coord.y][ghost->objData.Coord.x] == 'B') {
            ghost->status = BLOCKED;
            ghost->speed = 2; // reset the speed after back to the cage.
            ghost->go_in_time = al_get_timer_count(game_tick_timer); 
        }
        break;
    case FLEE:
        ghost_move_script_FLEE(ghost, M, pacman);
        break;
    default:
        break;
    }

    if(ghost_movable(ghost, M, ghost->objData.nextTryMove, false)){
        ghost->objData.preMove = ghost->objData.nextTryMove;
        ghost->objData.nextTryMove = NONE;
    }
    else if (!ghost_movable(ghost, M, ghost->objData.preMove, false))
        return;

    switch (ghost->objData.preMove) {
    case RIGHT:
        ghost->objData.Coord.x += 1;
        break;
    case LEFT:
        ghost->objData.Coord.x -= 1;
        break;
    case UP:
        ghost->objData.Coord.y -= 1;
        break;
    case DOWN:
        ghost->objData.Coord.y += 1;
        break;
    default:
        break;
    }
    ghost->objData.facing = ghost->objData.preMove;
    ghost->objData.moveCD = GAME_TICK_CD;
}
