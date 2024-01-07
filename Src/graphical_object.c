#define _CRT_SECURE_NO_WARNINGS
#define board_color al_map_rgb(255, 255, 255)

#include "graphical_object.h"
#include "utility.h"
#include <allegro5/allegro_primitives.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const int board = 3;
const int padding = 10;

Button button_create(float x, float y, float w, float h, const char* default_image_path, const char* hovered_image_path) {

	Button button;
	memset(&button, 0, sizeof(Button));

	button.default_img = load_bitmap(default_image_path);
	if (!button.default_img) {
		game_log("failed loading button image!");
	}
	if (hovered_image_path) {
		button.hovered_img = load_bitmap(hovered_image_path);
        if (!button.hovered_img) {
            game_log("failed loading button image!");
        }
	}


	button.body.x = x;
	button.body.y = y;
	button.body.w = w;
	button.body.h = h;

	return button;
}

void drawButton(Button button) {
	ALLEGRO_BITMAP* _img = button.hovered_img ? 
												button.hovered ? 
													button.hovered_img : 
													button.default_img : 
												button.default_img;
	al_draw_scaled_bitmap(
		_img,
		0, 0,
		al_get_bitmap_width(_img), al_get_bitmap_height(_img),
		button.body.x, button.body.y,
		button.body.w, button.body.h, 0
	);
}

bool buttonHover(Button button, int mouse_x, int mouse_y) {
	return pnt_in_rect(mouse_x, mouse_y, button.body);
}

SettingBar settingbar_create(float x, float y, float w, float h, ALLEGRO_COLOR lit_color, ALLEGRO_COLOR default_color, float percent){
    SettingBar bar;
    memset(&bar, 0, sizeof(SettingBar));

    bar.hovered = false;
    bar.percent = percent;
    bar.lit_color = lit_color;
    bar.default_color = default_color;
    bar.body.x = x, bar.body.y = y, bar.body.w = w, bar.body.h = h;
    bar.mouse = x + w * percent;
    return bar;
}

void update_settingbar(SettingBar* bar){
    if (!bar->hovered) return;
    float work = bar->mouse - bar->body.x;
    if (work > bar->body.w) work = bar->body.w;
    bar->percent = work / bar->body.w;
    return;
}

void drawSettingBar(SettingBar bar){
    al_draw_filled_rectangle(bar.body.x, bar.body.y, bar.body.x + bar.body.w * bar.percent, bar.body.y + bar.body.h, board_color);
    al_draw_filled_rectangle(bar.body.x + bar.mouse, bar.body.y, bar.body.x + bar.body.w, bar.body.y + bar.body.h, bar.default_color);
    al_draw_rectangle(bar.body.x, bar.body.y, bar.body.x + bar.body.w, bar.body.y + bar.body.h, bar.lit_color, board);
    return;
}

CheckBox checkbox_create(float x, float y, float w, float h, ALLEGRO_COLOR checked_color, ALLEGRO_COLOR default_color, bool checked){
    CheckBox box;
    memset(&box, 0, sizeof(CheckBox));
    box.body.x = x;
    box.body.y = y;
    box.body.w = w;
    box.body.h = h;
    box.checked = checked;
    box.hovered = false;
    box.default_color = default_color;
    box.checked_color = checked_color;
    return box;
}

void drawCheckBox(CheckBox box){
    ALLEGRO_COLOR color;
    if (box.checked)
        color = box.checked_color;
    else 
        color = box.default_color;
    al_draw_filled_rectangle(box.body.x + padding, box.body.y + padding, box.body.x + box.body.w - padding, box.body.y + box.body.h - padding, color);
    al_draw_rectangle(box.body.x, box.body.y, box.body.x + box.body.w, box.body.y + box.body.h, board_color, board);
    return;
}

void update_checkbox(CheckBox* box){
    if (!box->hovered) return;
    box->checked = !box->checked;
    return;
}

#undef board_color
