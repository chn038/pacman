// TODO-HACKATHON 3-9: Create scene_settings.h and scene_settings.c.
// No need to do anything for this part. We've already done it for
// you, so this 2 files is like the default scene template.
#include "scene_settings.h"
#include "scene_after_game.h"
#include "shared.h"
#include <allegro5/allegro_primitives.h>

// Variables and functions with 'static' prefix at the top level of a
// source file is only accessible in that file ("file scope", also
// known as "internal linkage"). If other files has the same variable
// name, they'll be different variables.

/* Define your static vars / function prototypes below. */
static float current_music = 0.5;
static float current_effect = 0.5;
static float max_music_volume = 2;
static float max_effect_volume = 2;
static bool music_muted = false;
static bool effect_muted = false;
static CheckBox music_mute;
static SettingBar music_volume_bar;
static CheckBox effect_mute;
static CheckBox large_map;
static SettingBar effect_volume_bar;
static void show_dashboard();
static bool dashboard = false;
static int output;
bool is_large_map = false;

static void init(void){
    dashboard = false;
    output = 0;
    music_volume_bar = settingbar_create(SCREEN_W * 0.4, SCREEN_H * 0.325, SCREEN_W * 0.3, SCREEN_H * 0.05, al_map_rgb(255, 255, 255), al_map_rgb(0, 0, 0), current_music / max_music_volume);
    music_mute = checkbox_create(SCREEN_W * 0.825, SCREEN_H * 0.325, SCREEN_W * 0.05, SCREEN_H * 0.05, al_map_rgb(255, 255, 255), al_map_rgb(0, 0, 0), music_muted);
    effect_volume_bar = settingbar_create(SCREEN_W * 0.4, SCREEN_H * 0.525, SCREEN_W * 0.3, SCREEN_H * 0.05, al_map_rgb(255, 255, 255), al_map_rgb(0, 0, 0), current_effect / max_effect_volume);
    effect_mute = checkbox_create(SCREEN_W * 0.825, SCREEN_H * 0.525, SCREEN_W * 0.05, SCREEN_H * 0.05, al_map_rgb(255, 255, 255), al_map_rgb(0, 0, 0), effect_muted);
    large_map = checkbox_create(SCREEN_W * 0.175, SCREEN_H * 0.725, SCREEN_W * 0.05, SCREEN_H * 0.05, al_map_rgb(255, 255, 255), al_map_rgb(0, 0, 0), is_large_map);
}

static void draw(void){

    static char music_indicator[50];
    static char effect_indicator[50];
	al_clear_to_color(al_map_rgb(0, 0, 0));

    sprintf(music_indicator, "Music: %.0f%%", current_music*100);
    sprintf(effect_indicator, "Effect: %.0f%%", current_effect*100);

    al_draw_text(
        menuFont, 
        al_map_rgb(255, 255, 255),
        SCREEN_W * 0.8,
        SCREEN_H * 0.2,
        ALLEGRO_ALIGN_LEFT,
        "Muted"
    );
    
    al_draw_text(
        menuFont, 
        al_map_rgb(255, 255, 255),
        SCREEN_W * 0.5,
        SCREEN_H * 0.1 - fontSize,
        ALLEGRO_ALIGN_CENTER,
        "Adjust by clicking the bar"
    );

    al_draw_text(
        menuFont, 
        al_map_rgb(255, 255, 255),
        SCREEN_W * 0.5,
        SCREEN_H * 0.1,
        ALLEGRO_ALIGN_CENTER,
        "You can also adjust with 'A' and 'D'"
    );

    al_draw_text(
        menuFont, 
        al_map_rgb(255, 255, 255),
        SCREEN_W * 0.1,
        SCREEN_H * 0.3375,
        ALLEGRO_ALIGN_LEFT,
        music_indicator
    );
    drawSettingBar(music_volume_bar);
    drawCheckBox(music_mute);

    al_draw_text(
        menuFont, 
        al_map_rgb(255, 255, 255),
        SCREEN_W * 0.1,
        SCREEN_H * 0.5375,
        ALLEGRO_ALIGN_LEFT,
        effect_indicator
    );
    drawSettingBar(effect_volume_bar);
    drawCheckBox(effect_mute);

    al_draw_text(
        menuFont,
        al_map_rgb(255, 255, 255),
        SCREEN_W * 0.3, SCREEN_H * 0.7375,
        ALLEGRO_ALIGN_LEFT,
        "A really large map"
    );
    drawCheckBox(large_map);

	al_draw_text(
		menuFont,
		al_map_rgb(255, 255, 255),
		SCREEN_W/2.0,
		SCREEN_H * 0.9 ,
		ALLEGRO_ALIGN_CENTER,
		"<ENTER> Back to menu"
	);
	al_draw_text(
		menuFont,
		al_map_rgb(255, 255, 255),
		SCREEN_W/2.0,
		SCREEN_H * 0.9 + fontSize,
		ALLEGRO_ALIGN_CENTER,
		"<S> to view Dashboard"
	);
    if (dashboard)
        show_dashboard();
}

static void update_setting(){
    current_music = max_music_volume * music_volume_bar.percent;
    current_effect = max_effect_volume * effect_volume_bar.percent;
    if (music_mute.checked) {
        music_muted = true;
        music_volume = 0;
    } else {
        music_muted = false;
        music_volume = current_music;
    }
    if (effect_mute.checked){
        effect_muted = true;
        effect_volume = 0;
    } else {
        effect_muted = false;
        effect_volume = current_effect;
    }
    if (large_map.checked){
        is_large_map = true;
    } else {
        is_large_map = false;
    }
}

static void show_dashboard(){
    static FILE *save;
    static char board[5][100];
    save = fopen("Assets/Data/dashboard.txt", "r");
    if (save) {
        if (!output)
            for (int i = 0; i < 5; ++i){
                if (feof(save)) break;
                fgets(board[i], 100, save);
                output++;
            }
        fclose(save);
    }
    al_draw_filled_rectangle(SCREEN_W*0.2, SCREEN_H*0.2, 
                SCREEN_W*0.8, SCREEN_H*0.8,
                al_map_rgb(0, 0, 0) 
    );
    al_draw_rectangle(SCREEN_W*0.2, SCREEN_H*0.2, 
                SCREEN_W*0.8, SCREEN_H*0.8,
                al_map_rgb(255, 255, 255), 3
    );
    al_draw_text(menuFont,
                 al_map_rgb(255, 255, 255),
                 SCREEN_W*0.5, SCREEN_H*0.2 + 10,
                 ALLEGRO_ALIGN_CENTER,
                 "Dashboard"
    );
    int offset = 0;
    static char index[12];
    for (int i = 0; i < output; ++i){
        sprintf(index, "%d", i + 1);
        al_draw_text(menuFont,
                     al_map_rgb(255, 255, 255),
                     SCREEN_W*0.3, SCREEN_H*0.3 + offset,
                     ALLEGRO_ALIGN_CENTER,
                     index
        );
        al_draw_text(menuFont,
                     al_map_rgb(255, 255, 255),
                     SCREEN_W*0.5, SCREEN_H*0.3 + offset,
                     ALLEGRO_ALIGN_CENTER,
                     board[i]
        );
        offset += fontSize;
    }
    al_draw_text(menuFont,
                 al_map_rgb(255, 255, 255),
                 SCREEN_W*0.5, SCREEN_H*0.8 - 10 - fontSize,
                 ALLEGRO_ALIGN_CENTER,
                 "Press <S> to go back"
    );
}

static void on_key_down(int keycode, int modifier) {
	switch (keycode) {
		case ALLEGRO_KEY_ENTER:
			game_change_scene(scene_menu_create());
			break;
        case ALLEGRO_KEY_A:
            if (music_volume_bar.hovered){
                music_volume_bar.mouse = music_volume_bar.body.x + music_volume_bar.body.w * music_volume_bar.percent;
                music_volume_bar.mouse -= 1;
                if (music_volume_bar.mouse < music_volume_bar.body.x)
                    music_volume_bar.mouse = music_volume_bar.body.x;
                update_settingbar(&music_volume_bar);
            }
            if (effect_volume_bar.hovered){
                effect_volume_bar.mouse = effect_volume_bar.body.x + effect_volume_bar.body.w * effect_volume_bar.percent;
                effect_volume_bar.mouse -= 1;
                if (effect_volume_bar.mouse < effect_volume_bar.body.x)
                    effect_volume_bar.mouse = effect_volume_bar.body.x;
                update_settingbar(&effect_volume_bar);
            }
            update_setting();
            break;
        case ALLEGRO_KEY_D:
            if (music_volume_bar.hovered){
                music_volume_bar.mouse = music_volume_bar.body.x + music_volume_bar.body.w * music_volume_bar.percent;
                music_volume_bar.mouse += 1;
                if (music_volume_bar.mouse > music_volume_bar.body.x + music_volume_bar.body.w)
                    music_volume_bar.mouse = music_volume_bar.body.x;
                update_settingbar(&music_volume_bar);
            }
            if (effect_volume_bar.hovered){
                effect_volume_bar.mouse = effect_volume_bar.body.x + effect_volume_bar.body.w * effect_volume_bar.percent;
                effect_volume_bar.mouse += 1;
                if (effect_volume_bar.mouse > effect_volume_bar.body.x + music_volume_bar.body.w)
                    effect_volume_bar.mouse = effect_volume_bar.body.x;
                update_settingbar(&effect_volume_bar);
            }
            update_setting();
            break;
        case ALLEGRO_KEY_S:
            if (dashboard)
                dashboard = false;
            else
                dashboard = true;
            break;
		default:
			break;
	}
}

static void on_mouse_move(int a, int mouse_x, int mouse_y, int f){
    if (dashboard) return;
    if (pnt_in_rect(mouse_x, mouse_y, music_volume_bar.body)) {
        music_volume_bar.hovered = true;
        music_volume_bar.mouse = mouse_x;
    } else {
        music_volume_bar.hovered = false;
    }
    if (pnt_in_rect(mouse_x, mouse_y, effect_volume_bar.body)) {
        effect_volume_bar.hovered = true;
        effect_volume_bar.mouse = mouse_x;
    } else {
        effect_volume_bar.hovered = false;
    }
    if (pnt_in_rect(mouse_x, mouse_y, music_mute.body))
        music_mute.hovered = true;
    else
        music_mute.hovered = false;
    if (pnt_in_rect(mouse_x, mouse_y, effect_mute.body))
        effect_mute.hovered = true;
    else
        effect_mute.hovered = false;
    if (pnt_in_rect(mouse_x, mouse_y, large_map.body))
        large_map.hovered = true;
    else 
        large_map.hovered = false;
}

static void on_mouse_down(){
    if (dashboard) {
        dashboard = false;
        return;
    }
    update_checkbox(&music_mute);
    update_checkbox(&effect_mute);
    update_settingbar(&music_volume_bar);
    update_settingbar(&effect_volume_bar);
    update_checkbox(&large_map);

    update_setting();
}

// The only function that is shared across files.
Scene scene_settings_create(void) {
	Scene scene;
	memset(&scene, 0, sizeof(Scene));
	scene.name = "Settings";
    scene.initialize = &init;
	scene.draw = &draw;
	scene.on_key_down = &on_key_down;
    scene.on_mouse_move = &on_mouse_move;
    scene.on_mouse_down = &on_mouse_down;
	// TODO-IF: Register more event callback functions such as keyboard, mouse, ...
	game_log("Settings scene created");
	return scene;
}
