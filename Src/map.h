#ifndef MAP_H
#define MAP_H
#include "utility.h"

#define MAX_WALL_NUM_W 100
#define MAX_WALL_NUM_H 100
extern const int block_width,  block_height;
extern const int map_offset_x, map_offset_y ;

typedef struct Map{
	int row_num, col_num;
	int wallnum;
	int beansNum;
	int beansCount;
	char** map;
} Map;
typedef struct Submap{
    int row_num, col_num;
    const Map *point;
    Pair_IntInt offset;
} Submap;

Map* create_map(const char * const filepath);
Submap* create_submap(const Map *map, Pair_IntInt distance);
void delete_submap(Submap*);
void delete_map(Map*);

void draw_map(const Map*);
void draw_submap(const Submap*);
void update_submap(Submap* submap, Pair_IntInt position, Pair_IntInt distance);
//void draw_block(Map*, RecArea);
Directions shortest_distance(Map* M, int startGridx, int startGridy, int endGridx, int endGridy);
Directions shortest_path_direc(Map* M, int startGridx, int startGridy, int endGridx, int endGridy);
bool is_wall_block(const Map* M, int index_x, int index_y);
bool is_room_block(const Map* M, int index_x, int index_y);
bool is_wall_point(const Map* M , float x, float y);
bool is_room_point(const Map* M , float x, float y);
#endif
