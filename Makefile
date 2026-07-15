CC = gcc

RAYLIB = $(shell brew --prefix raylib)

CFLAGS = -Wall -Wextra -std=c11 -I$(RAYLIB)/include -Isrc
LDFLAGS = -L$(RAYLIB)/lib -lraylib \
	-framework Cocoa \
	-framework OpenGL \
	-framework IOKit \
	-framework CoreVideo

SRC = src/main.c src/game.c src/scenes/scene.c src/scenes/menu_scene.c src/scenes/play_scene.c src/ui/ui.c 
	

game: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o game $(LDFLAGS)

run: game
	./game

clean:
	rm -f game
