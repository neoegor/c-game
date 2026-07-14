CC = gcc

RAYLIB = $(shell brew --prefix raylib)

CFLAGS = -Wall -Wextra -std=c11 -I$(RAYLIB)/include
LDFLAGS = -L$(RAYLIB)/lib -lraylib \
	-framework Cocoa \
	-framework OpenGL \
	-framework IOKit \
	-framework CoreVideo

SRC = main.c game.c scene.c menu_scene.c play_scene.c 
	

game: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o game $(LDFLAGS)

run: game
	./game

clean:
	rm -f game
