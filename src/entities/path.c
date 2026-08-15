#include <raylib.h>
#include <raymath.h>

#include "common.h"
#include "entities/path.h"

Vector2 path_get_next_direction(Path* path, int i) {
    // may need guards here
    Vector2 direction = Vector2Normalize(Vector2Subtract(path->points[i+1], path->points[i]));

    return direction;
}

