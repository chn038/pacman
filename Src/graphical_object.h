#ifndef MENU_OBJECT_H
#define MENU_OBJECT_H

#include "utility.h"
#include "shared.h"
#include "game.h"

typedef struct Button {
	RecArea body;
	ALLEGRO_BITMAP* default_img;
	ALLEGRO_BITMAP* hovered_img;
	bool hovered;
}Button;

typedef struct SettingBar {
    RecArea body;
    ALLEGRO_COLOR lit_color;
    ALLEGRO_COLOR default_color;
    float percent;
    float mouse;
    bool hovered;
} SettingBar;

typedef struct CheckBox {
    RecArea body;
    ALLEGRO_COLOR checked_color;
    ALLEGRO_COLOR default_color;
    bool checked;
    bool hovered;
} CheckBox;

Button button_create(float x, float y, float w, float h, const char* default_image_path, const char* hovered_image_path);
void drawButton(Button button);
bool buttonHover(Button, int, int);
SettingBar settingbar_create(float x, float y, float w, float h, ALLEGRO_COLOR lit_color, ALLEGRO_COLOR default_color, float percent);
void drawSettingBar(SettingBar bar);
void update_settingbar(SettingBar *bar);
CheckBox checkbox_create(float x, float y, float w, float h, ALLEGRO_COLOR checked_color, ALLEGRO_COLOR default_color, bool checked);
void drawCheckBox(CheckBox box);
void update_checkbox(CheckBox *box);

#endif
