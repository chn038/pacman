#include "scene_after_game.h"
#include "scene_menu.h"
#include "scene_game.h"
#include "shared.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static FILE *dashboard;
static int length;
static int output;
static char name[51];
static char board[11][100];
static int score[10];

static void init();
static void draw();
static void on_key_down(int keystroke);
static void destroy();
static void append_name();
static void insert(int keystroke);
static int compare(const void* a, const void* b);
bool written = false;
static bool read_only = false;

static void init(){
    length = 0;
    output = 0;
    written = read_only;
    memset(name, '\0', sizeof(name));
    if (read_only){
        strcpy(name, "Read only");
    }
    dashboard = fopen("Assets/Data/dashboard.txt", "r");
    if (!dashboard){
        game_log("Failed to load dashboard, create new one");
    } else {
        for (int i = 0; i < 10; ++i){
            if (feof(dashboard)) break;
            if (ferror(dashboard)) game_log("Something went wrong");
            fgets(board[output++], 100, dashboard);
        }
        fclose(dashboard);
    }
    if (!read_only){
        dashboard = fopen("Assets/Data/dashboard.txt", "w");
        if (!dashboard){
            game_log("Failed to open dashboard");
            game_change_scene(scene_menu_create());
        }
    }
}

static void append_name(){
    if (written) return;
    written = true;
    sprintf(board[output++], "%s:  %d\n", name, get_score());
    int position;
    qsort(board, output, sizeof(board[0]), compare);
    for (int i = 0; i < output; ++i){
        fwrite(board[i], sizeof(char), strlen(board[i]), dashboard);
        printf("%s", board[i]);
    }
    memset(name, '\0', sizeof(name));
    length = 0;
    strcpy(name, "Uploaded!");
}

static int compare(const void* a, const void* b){
    if (!strlen(a)) return -1;
    else if (!strlen(b)) return 1;
    int ia, ib;
    sscanf((const char*)a, "%*s %d", &ia);
    sscanf((const char*)b, "%*s %d", &ib);
    return ib - ia;
}

static void draw(){
    int offset = 0;
    al_draw_text(menuFont, 
                 al_map_rgb(255, 255, 255),
                 SCREEN_W*0.5, SCREEN_H*0.1, 
                 ALLEGRO_ALIGN_CENTER,
                 "Dashboard"
    );
    for (int i = 0; i < output; ++i){
        al_draw_text(menuFont,
            al_map_rgb(255, 255, 255),
            SCREEN_W*0.5,
            SCREEN_H*0.2 + offset,
            ALLEGRO_ALIGN_CENTER,
            board[i] 
        );
        offset += fontSize;
    }
    al_draw_text(menuFont,
            al_map_rgb(255, 255, 255),
            SCREEN_W*0.1, SCREEN_H*0.9,
            ALLEGRO_ALIGN_LEFT,
            "Name:"
    );
    al_draw_text(menuFont,
            al_map_rgb(255, 255, 255),
            SCREEN_W*0.5, SCREEN_H*0.9,
            ALLEGRO_ALIGN_CENTER,
            name
    );
    al_draw_text(menuFont,
            al_map_rgb(255, 255, 255),
            SCREEN_W*0.5, SCREEN_H*0.9 + fontSize,
            ALLEGRO_ALIGN_CENTER,
            "Press <Enter> to go back / record name"
    );
}

static void on_key_down(int keystroke){
    switch (keystroke){
    case ALLEGRO_KEY_ENTER:
        if (length)
            append_name();
        else
            game_change_scene(scene_menu_create());
        break;
    default:
        insert(keystroke);
        break;
    }
}

static void insert(int keystroke){
    if (written) return;
    if (length >= 50) return;
    char input;
    switch (keystroke){
    case ALLEGRO_KEY_0:
        input = '0';
        break;
    case ALLEGRO_KEY_1:
        input = '1';
        break;
    case ALLEGRO_KEY_2:
        input = '2';
        break;
    case ALLEGRO_KEY_3:
        input = '3';
        break;
    case ALLEGRO_KEY_4:
        input = '4';
        break;
    case ALLEGRO_KEY_5:
        input = '5';
        break;
    case ALLEGRO_KEY_6:
        input = '6';
        break;
    case ALLEGRO_KEY_7:
        input = '7';
        break;
    case ALLEGRO_KEY_8:
        input = '8';
        break;
    case ALLEGRO_KEY_9:
        input = '9';
        break;
    case ALLEGRO_KEY_A:
        input = 'A';
        break;
    case ALLEGRO_KEY_B:
        input = 'B';
        break;
    case ALLEGRO_KEY_C:
        input = 'C';
        break;
    case ALLEGRO_KEY_D:
        input = 'D';
        break;
    case ALLEGRO_KEY_E:
        input = 'E';
        break;
    case ALLEGRO_KEY_F:
        input = 'F';
        break;
    case ALLEGRO_KEY_G:
        input = 'G';
        break;
    case ALLEGRO_KEY_H:
        input = 'H';
        break;
    case ALLEGRO_KEY_I:
        input = 'I';
        break;
    case ALLEGRO_KEY_J:
        input = 'J';
        break;
    case ALLEGRO_KEY_K:
        input = 'K';
        break;
    case ALLEGRO_KEY_L:
        input = 'L';
        break;
    case ALLEGRO_KEY_M:
        input = 'M';
        break;
    case ALLEGRO_KEY_N:
        input = 'N';
        break;
    case ALLEGRO_KEY_O:
        input = 'O';
        break;
    case ALLEGRO_KEY_P:
        input = 'P';
        break;
    case ALLEGRO_KEY_Q:
        input = 'Q';
        break;
    case ALLEGRO_KEY_R:
        input = 'R';
        break;
    case ALLEGRO_KEY_S:
        input = 'S';
        break;
    case ALLEGRO_KEY_T:
        input = 'T';
        break;
    case ALLEGRO_KEY_U:
        input = 'U';
        break;
    case ALLEGRO_KEY_V:
        input = 'V';
        break;
    case ALLEGRO_KEY_W:
        input = 'W';
        break;
    case ALLEGRO_KEY_X:
        input = 'X';
        break;
    case ALLEGRO_KEY_Y:
        input = 'Y';
        break;
    case ALLEGRO_KEY_Z:
        input = 'Z';
        break;
    case ALLEGRO_KEY_BACKSPACE:
        if (length) {
            length--;
            name[length] = '\0';
        }
        return;
    default:
        return;
    }
    name[length] = input;
    length++;
    return;
}

static void destroy(){
    if (!read_only)
        fclose(dashboard);
}

Scene scene_after_game_create(bool append) {
	Scene scene;
	memset(&scene, 0, sizeof(Scene));
    if (append) read_only = false;
    else read_only = true;
	scene.name = "Dashboard";
    scene.initialize = &init;
	scene.draw = &draw;
	scene.on_key_down = &on_key_down;
    scene.destroy = &destroy;
	// TODO-IF: Register more event callback functions such as keyboard, mouse, ...
	game_log("Game over scene created");
	return scene;
}
