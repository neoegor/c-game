#ifndef PATH_H
#define PATH_H

#include <raylib.h>

#include "common.h"

#define MARGIN 1
#define MARGIN_ENDPOINT 8
#define MIN_VERTICAL_LENGTH 2 // Coordinates difference

typedef struct {
    Vector2 points[128];
    int count;
} Path;

bool path_generate(Path* path, int world_width, int world_height, int vertical_segments);
Vector2 path_get_next_direction(Path* path, int i);

#endif
