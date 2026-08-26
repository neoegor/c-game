#ifndef PLAY_SCENE_H
#define PLAY_SCENE_H

#include <raylib.h>

#include "common.h"
#include "scenes/scene.h"
#include "world/play_world.h"

#define CELL 20.0f
#define INVENTORY_SLOT_SIZE 90.0f
#define INVENTORY_GAP 8.0f
#define INVENTORY_BOTTOM_MARGIN 12.0f

typedef enum {
    SCENE_NORMAL,
    SCENE_OPERAND_PROMPT
} SceneState;

typedef struct {
    Vector2 position;
    TowerType type;
    int operand;
    bool operand_edit_mode;
} PendingPlacement;

typedef struct {
    Scene scene;
    SceneState state;
    PlayWorld world;
    PendingPlacement pending;
} PlayScene;

void play_init(PlayScene* scene);
void play_reset(PlayScene* scene);
SceneRequest play_update(PlayScene* scene, float dt);
void play_draw_world(PlayScene* scene);
SceneRequest play_draw_ui(PlayScene* scene);
void play_free(PlayScene* scene);

#endif
