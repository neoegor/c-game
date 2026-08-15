CC = gcc

RAYLIB = $(shell brew --prefix raylib)

CFLAGS = -Wall -Wextra -std=c11 \
	-I$(RAYLIB)/include \
	-Isrc \
	-Iexternal/raygui 

LDFLAGS = -L$(RAYLIB)/lib -lraylib \
	-framework Cocoa \
	-framework OpenGL \
	-framework IOKit \
	-framework CoreVideo

SRC = src/main.c \
	  src/game.c \
	  src/scenes/scene.c \
	  src/scenes/menu_scene.c \
	  src/scenes/play_scene.c \
	  src/world/play_world.c \
	  src/entities/tower.c \
	  src/entities/path.c \
	  src/entities/enemy.c \
	  src/entities/wave.c \
	  external/raygui/raygui_impl.c
	

game: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o game $(LDFLAGS)

run: game
	./game

clean:
	rm -f game
