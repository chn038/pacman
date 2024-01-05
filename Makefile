.PHONY: clean

CC=gcc
SRC=Src
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
	CCFLAGS=$$(pkg-config allegro-5 allegro_font-5 allegro_ttf-5 allegro_audio-5 allegro_image-5 allegro_primitives-5 allegro_acodec-5 --libs --cflags)
	INCLUDE=/usr/include/allegro5
endif
#REVISE INCLUDE path if you have installed different version or install path.

DBG = -g -Wall -Wextra 
objects= main.o game.o ghost.o ghost_move_script.o map.o pacman_obj.o scene_game.o scene_menu.o graphical_object.o scene_settings.o shared.o utility.o scene_after_game.o

all: pacman 

pacman: $(objects)
	$(CC) -lm $(CCFLAGS) $(DBG) -o pacman $^

%.o: $(SRC)/%.c $(SRC)/%.h
	$(CC) -I$(INCLUDE) -c $< -o $@

%.o: $(SRC)/%.c
	$(CC) -I$(INCLUDE) -c $< -o $@

clean:
	rm pacman *.o
