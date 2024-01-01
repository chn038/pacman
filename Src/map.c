#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <stdio.h>
#include <allegro5/allegro_primitives.h>
#include "game.h"
#include "map.h"
#define QUEUE_SIZE 3000

/*global variables*/
// [ NOTE ]
const int block_width = 21,  block_height = 21;			// the pixel size of a "block"
const int map_offset_x = 25, map_offset_y = 50;			// pixel offset of where to start draw map
const int four_probe[4][2] = {{ 1, 0 }, { 0, 1 }, { -1,0 }, { 0, -1 }};

/* Declare static function prototypes. */
static void draw_block_index(const Map* M, int row, int col, const Pair_IntInt position);
static void draw_bean(const Map* M, const Pair_IntInt position);
static void draw_power_bean(const Map* M, const Pair_IntInt position);

const char* nthu_map[] = {
	"#####################################",
	"#..................###.........#####",
	"#.####.###########.....#######.....#",
	"#.####.#...........###.....# #.###.#",
	"#.P......#########.....#.#.###.# #.#",
	"#.#####.###      ###.###.#.#P#.# #.#",
	"#.#   #...#        #.###.#.#.#.# #.#",
	"#.#   #.#.#        #.###.#...#.# #.#",
	"#.#####.#.##########.###.#####.###.#",
	"#..................................#",
	"#.#######.########.##BBB##.##.####.#",
	"#.###  ##.########.##BBB##.##.#P##.#",
	"#.#### ##....##....##BBB##.##.#.##.#",
	"#.## ####.##.##.##.#######.##.#.##.#",
	"#.##  ###.##.##.##.#######.##...##.#",
	"#.##   ##.##.##.##.##   ##.#######.#",
	"#.#######.##.##.##.#######.#######.#",
	"#..................................#",
	"#.######.###.##########.######.###.#",
	"#.#    #.###.#####   #.........# #.#",
	"#.######.........#####.###.###.# #.#",
	"#........#######.......# #.# #.# #.#",
	"#.######.#     #####.### #.# #.###.#",
	"#.#    #.###########.#####.###.....#",
	"#.######.#.....P.............#.###.#",
	"#..........#########.#######.#.# #.#",
	"#.#######.##########.#     #...# #.#",
	"#.#######............#######.#.###.#",
	"#.........##########.........#.....#",
	"####################################"
};

Map* create_map(const char * filepath) {

	Map* M = (Map*)malloc(sizeof(Map));
	FILE* pFile = NULL;
	if (!M) {
		game_abort("Error when creating Map");
		return NULL;
	}
	if (filepath == NULL) {
		M->row_num = 30;
		M->col_num = 36;
		game_log("Creating from default map. row = %d col = %d", M->row_num, M->col_num);
		
	}
	else {
		game_log("Tries to open %s\n", filepath);
		pFile = fopen(filepath, "r");
		if (!pFile) {
			game_abort("error to open map file\n");
			return NULL;
		}
		if(fscanf(pFile, "%d%d", &M->row_num, &M->col_num) != 2) {
			game_abort("Map format unmatched\n");
            fclose(pFile);
			return NULL;
		}
		while(getc(pFile) != '\n'){};
	}

	M->map = (char**)malloc(sizeof(char*) * M->row_num);
	if (!M->map) {
		game_abort("map char array malloc error\n");
		return NULL;
	}
	for (int i = 0; i < M->row_num; i++) {
		M->map[i] = (char*)malloc(sizeof(char) * (M->col_num));
		if(!M->map[i]){
			game_abort("map char array malloc error\n");
			return NULL;
		}
	}

	M->wallnum = M->beansCount = 0;
	for (int i = 0; i < M->row_num; i++) {
		for (int j = 0; j < M->col_num; j++) {
			if (filepath == NULL)
				M->map[i][j] = nthu_map[i][j];
			else
				fscanf(pFile, "%c", &M->map[i][j]);
			switch(M->map[i][j]) {
			case '#':
				M->wallnum++;
				break;
			case '.':
				M->beansCount++;
				break;
            case 'P':
                M->beansCount++;
                break;
			default:
				break;
			}
		}
		if(filepath != NULL) {
			while(getc(pFile) != '\n'){};
        }
	}
	M->beansNum = M->beansCount;
    if (pFile) fclose(pFile);
	return M;
}

Submap* create_submap(const Map* map, Pair_IntInt distance){
    if (distance.y > map->row_num || distance.x > map->col_num){
        game_abort("Too large for submap to create");
        return NULL;
    }
    Submap *submap = (Submap*)malloc(sizeof(Submap));
    if (!submap){
        game_abort("Error creating submap");
        return NULL;
    }
    submap->col_num = distance.x;
    submap->row_num = distance.y;
    submap->offset.x = submap->offset.y = 0;
    if (map)
        submap->point = map;
    else
        submap->point = NULL;
    return submap;
}

void update_submap(Submap *submap, Pair_IntInt position, Pair_IntInt vision){
    submap->row_num = vision.y, submap->col_num = vision.x;
    submap->offset.x = position.x - submap->col_num / 2;
    submap->offset.y = position.y - submap->row_num / 2;
    if (submap->offset.x < 0) {
        submap->col_num += submap->offset.x;
        submap->offset.x = 0;
    }
    if (submap->offset.y < 0) {
        submap->row_num += submap->offset.y;
        submap->offset.y = 0;
    }
    if (submap->offset.x + submap->col_num > submap->point->col_num){
        submap->col_num = submap->point->col_num - submap->offset.x;
    }
    if (submap->offset.y + submap->row_num > submap->point->row_num){
        submap->row_num = submap->point->row_num - submap->offset.y;
    }
    return;
}

void delete_submap(Submap *submap){
    free(submap);
}

void delete_map(Map* M) {
	if (!M)
		return;
	// TODO-GC-memory: free the dynamic allocated part of Map* M at here;
    if (M->map) {
        for (int i = 0; i < M->row_num; ++i){
            if (M->map[i]) free(M->map[i]);
        }
        free(M->map);
    }
	free(M);
}

void draw_submap(const Submap *submap, int centered){
    if (submap->point == NULL){
        game_abort("error map!\n");
        return;
    }
    if (centered){
        Pair_IntInt position;
        for (int row = submap->offset.y; row < submap->row_num + submap->offset.y; ++row){
            for (int col = submap->offset.x; col < submap->col_num + submap->offset.x; ++col) {
                position = make_pair(col - submap->offset.x, row - submap->offset.y);
                switch (submap->point->map[row][col])
                {
                    case '#':
                        draw_block_index(submap->point, row, col, position);
                        break;
                    // TODO-PB: draw the power bean
                    case 'P':
                        draw_power_bean(submap->point, position);
                        break;
                    case '.':
                        draw_bean(submap->point, position);
                        break;
                    default:
                        break;
                }
            }
        }
    } else {
        for (int row = submap->offset.y; row < submap->row_num + submap->offset.y; ++row){
            for (int col = submap->offset.x; col < submap->col_num + submap->offset.x; ++col) {
                switch (submap->point->map[row][col])
                {
                    case '#':
                        draw_block_index(submap->point, row, col, make_pair(row, col));
                        break;
                    // TODO-PB: draw the power bean
                    case 'P':
                        draw_power_bean(submap->point, make_pair(row, col));
                        break;
                    case '.':
                        draw_bean(submap->point, make_pair(row, col));
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void draw_map(Map const* M) {
	if (M == NULL) {
		game_abort("error map!\n");
		return;
	}
	// draw the map according to M->map
	for (int row = 0; row < M->row_num; row++) {
		for (int col = 0; col < M->col_num; col++) {
			switch (M->map[row][col])
			{
				case '#':
					draw_block_index(M, row, col, make_pair(row, col));
					break;
				// TODO-PB: draw the power bean
				case 'P':
					draw_power_bean(M, make_pair(row, col));
					break;
				case '.':
					draw_bean(M, make_pair(row, col));
					break;
				default:
					break;
			}
		}
	}
}

static void draw_block_index(const Map* M, const int row, const int col, const Pair_IntInt position) {
	bool U = is_wall_block(M, col, row - 1);
	bool UR = is_wall_block(M, col + 1, row - 1);
	bool UL = is_wall_block(M, col -1, row- 1);
	bool B = is_wall_block(M, col, row+ 1);
	bool BR = is_wall_block(M, col + 1, row + 1);
	bool BL = is_wall_block(M, col - 1, row + 1);
	bool R = is_wall_block(M, col + 1, row);
	bool L = is_wall_block(M, col - 1,row);
	if (!(U && UR && UL && B && BR && BL && R && L)) {
	int s_x, s_y, e_x, e_y, dw;
	int block_x = map_offset_x + block_width * position.x;
	int block_y = map_offset_y + block_height * position.y;
	dw = block_width / 3;
		s_x = block_x + dw;
		s_y = block_y+ dw;
		e_x = s_x + dw;
		e_y = s_y + dw;

		al_draw_filled_rectangle(s_x, s_y,
			e_x, e_y, al_map_rgb(25, 154, 25));
		if (row < M->row_num - 1 && B && !(BL && BR && R && L)) {
			s_x = block_x + dw;
			s_y = block_y + dw;
			e_x = s_x + dw;
			e_y = block_y + block_height;
			al_draw_filled_rectangle(s_x, s_y,
				e_x, e_y, al_map_rgb(25, 154, 25));
		}
		if (row > 0 && U && !(UL && UR && R && L)) {
			s_x = block_x + dw;
			s_y = block_y + (dw << 1);
			e_x = s_x + dw;
			e_y = block_y;
			al_draw_filled_rectangle(s_x, s_y,
				e_x, e_y, al_map_rgb(25, 154, 25));
		}
		if (col < M->col_num - 1 && R && !(UR && BR && U && B)) {
			s_x = block_x + dw;
			s_y = block_y + dw;
			e_x = block_x + block_width;
			e_y = s_y + dw;
			al_draw_filled_rectangle(s_x, s_y,
				e_x, e_y, al_map_rgb(25, 154, 25));

		}
		if (col > 0 && L && !(UL && BL && U && B)) {
			s_x = block_x;
			s_y = block_y + dw;
			e_x = s_x + (dw << 1);
			e_y = s_y + dw;
			al_draw_filled_rectangle(s_x, s_y,
				e_x, e_y, al_map_rgb(25, 154, 25));
		}
	}
}

static void draw_bean(const Map* M, const Pair_IntInt position) {
	al_draw_filled_circle(map_offset_x + position.x * block_width + block_width / 2.0, map_offset_y + position.y * block_height + block_height / 2.0, block_width/6.0,  al_map_rgb(234, 38, 38));
}

static void draw_power_bean(const Map* M, const Pair_IntInt position) {
	al_draw_filled_circle(map_offset_x + position.x * block_width + block_width / 2.0, map_offset_y + position.y * block_height + block_height / 2.0, block_width / 3.0, al_map_rgb(234, 178, 38));
}


bool is_wall_block(const Map* M, int index_x, int index_y) {
	if (index_x < 0 || index_x >= M->col_num || index_y < 0 || index_y >= M->row_num)
		return true;
	return M->map[index_y][index_x] == '#';
}
bool is_room_block(const Map* M, int index_x, int index_y) {
	if (index_x < 0 || index_x >= M->col_num || index_y < 0 || index_y >= M->row_num)
		return true;
	return M->map[index_y][index_x] == 'B';
}

Directions shortest_distance(Map* M, int startGridx, int startGridy, int endGridx, int endGridy) {
	// NOTODO
	// Here is a complete function return the next direction of the shortest path.
	// Given Map, start point and end point.
	// It will tell you where to go for the shortest path.
	// !NOTICE! if your map grow really large, the size of queue, may become not enough. 
	// Hint: You can alter this function and make it return direction and also the distance for your usage.


static int8_t queue_x[QUEUE_SIZE];
static int8_t queue_y[QUEUE_SIZE];
static	uint16_t front;
static	uint16_t end;

	static Directions steped[MAX_WALL_NUM_H][MAX_WALL_NUM_W];
	memset(steped, 0, sizeof(steped)); // set as NONE;

	front = end = 0;
	queue_x[end] = startGridx;
	queue_y[end] = startGridy;
	steped[startGridy][startGridx] = 1; /*	for dummy just means that startGrid have been visited.	*/ 

	end++;

	for (size_t i = 0; i < 4; i++) {
		int8_t x = queue_x[front] + four_probe[i][0];
		int8_t y = queue_y[front] + four_probe[i][1];
		if (is_wall_block(M, x, y) || steped[y][x])
			continue;
		queue_x[end] = x;
		queue_y[end] = y;
		switch (i) {
			case 0:
				steped[y][x] = RIGHT;
				break;
			case 1:
				steped[y][x] = DOWN;
				break;
			case 2:
				steped[y][x] = LEFT;
				break;
			case 3:
				steped[y][x] = UP;
				break;
			default:
				break;
		}
		end++;
	}
	front++;

	while (front != end && steped[endGridy][endGridx] == NONE) {

		for (size_t i = 0; i < 4; i++) {
			int8_t x = queue_x[front] + four_probe[i][0];
			int8_t y = queue_y[front] + four_probe[i][1];
			if (is_wall_block(M, x, y) || steped[y][x])
				continue;
			queue_x[end] = x;
			queue_y[end] = y;
			steped[y][x] = steped[queue_y[front]][queue_x[front]];
			end++;
		}
		front++;
	}
	return front;
}

Directions shortest_path_direc(Map* M, int startGridx, int startGridy, int endGridx, int endGridy) {
	// NOTODO
	// Here is a complete function return the next direction of the shortest path.
	// Given Map, start point and end point.
	// It will tell you where to go for the shortest path.
	// !NOTICE! if your map grow really large, the size of queue, may become not enough. 
	// Hint: You can alter this function and make it return direction and also the distance for your usage.


static int8_t queue_x[QUEUE_SIZE];
static int8_t queue_y[QUEUE_SIZE];
static	uint16_t front;
static	uint16_t end;

	static Directions steped[MAX_WALL_NUM_H][MAX_WALL_NUM_W];
	memset(steped, 0, sizeof(steped)); // set as NONE;

	front = end = 0;
	queue_x[end] = startGridx;
	queue_y[end] = startGridy;
	steped[startGridy][startGridx] = 1; /*	for dummy just means that startGrid have been visited.	*/ 

	end++;

	for (size_t i = 0; i < 4; i++) {
		int8_t x = queue_x[front] + four_probe[i][0];
		int8_t y = queue_y[front] + four_probe[i][1];
		if (is_wall_block(M, x, y) || steped[y][x])
			continue;
		queue_x[end] = x;
		queue_y[end] = y;
		switch (i) {
			case 0:
				steped[y][x] = RIGHT;
				break;
			case 1:
				steped[y][x] = DOWN;
				break;
			case 2:
				steped[y][x] = LEFT;
				break;
			case 3:
				steped[y][x] = UP;
				break;
			default:
				break;
		}
		end++;
	}
	front++;

	while (front != end && steped[endGridy][endGridx] == NONE) {

		for (size_t i = 0; i < 4; i++) {
			int8_t x = queue_x[front] + four_probe[i][0];
			int8_t y = queue_y[front] + four_probe[i][1];
			if (is_wall_block(M, x, y) || steped[y][x])
				continue;
			queue_x[end] = x;
			queue_y[end] = y;
			steped[y][x] = steped[queue_y[front]][queue_x[front]];
			end++;
		}
		front++;
	}
	return steped[endGridy][endGridx];
}
