#ifndef PATH_H
#define PATH_H

#include <raylib.h>

#include "common.h"

typedef struct {
    Vector2 points[128];
    int count;
} Path;

Vector2 path_get_next_direction(Path* path, int i);

#endif
