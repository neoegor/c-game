#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>

#include "common.h"
#include "entities/path.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

bool path_generate(Path* path, int grid_width, int grid_height, int vertical_segments) {
    if (vertical_segments < 0) return false;
    if (grid_height - 2 * MARGIN < 1) return false;
    if (grid_height - 2 * MARGIN_ENDPOINT < 1) return false;

    int x_component;
    int y_component = GetRandomValue(MARGIN_ENDPOINT, grid_height-1-MARGIN_ENDPOINT);
    int partition_x_length;

    if (vertical_segments == 0) {
        partition_x_length = grid_width;
    } else {
        partition_x_length = grid_width / (vertical_segments);
    }

    if (partition_x_length - 2 * MARGIN < 1) return false;

    path->count = 0;
    path->points[path->count++] = (Vector2){-1, y_component};

    for (int i = 0; i < vertical_segments; i++) {
        x_component = GetRandomValue(
            partition_x_length * i + MARGIN,
            MIN(grid_width-1, partition_x_length * i + partition_x_length) - MARGIN
        );
        path->points[path->count++] = (Vector2){x_component, y_component};

        int old_y_component = y_component;
        
        do {
            if (i == vertical_segments - 1) {
                y_component = GetRandomValue(MARGIN_ENDPOINT, grid_height-1-MARGIN_ENDPOINT);
            } else {
                y_component = GetRandomValue(MARGIN, grid_height-1-MARGIN);
            }
        } while (abs(old_y_component - y_component) < MIN_VERTICAL_LENGTH);

        path->points[path->count++] = (Vector2){x_component, y_component};
    }

    path->points[path->count++] = (Vector2){grid_width, y_component};

    return true;
}

Vector2 path_get_next_direction(Path* path, int i) {
    // may need guards here
    Vector2 direction = Vector2Normalize(Vector2Subtract(path->points[i+1], path->points[i]));

    return direction;
}

