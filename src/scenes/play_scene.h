#ifndef PLAY_SCENE_H
#define PLAY_SCENE_H

#include <raylib.h>

#include "common.h"
#include "scenes/scene.h"
#include "world/play_world.h"
#include "ui/notifier.h"

#define CELL 20.0f
#define PROJECTILE_RADIUS 0.125f
#define ENEMY_TEXT_SIZE 20
#define INVENTORY_HEIGH 120.0f
#define INVENTORY_TEXT_SIZE 20
#define INVENTORY_SLOT_SIZE 90.0f
#define INVENTORY_GAP 10.0f
#define INVENTORY_BOTTOM_MARGIN 15.0f
#define WAVE_PROGRESS_WIDTH 500
#define NOTIFIER_TEXT_SIZE 20

typedef enum {
    SCENE_NORMAL,
    SCENE_OPERAND_PROMPT
} SceneState;

typedef struct {
    Rectangle world_area;
    Camera2D world_camera;
    Rectangle inventory_area;
} PlayLayout;

typedef struct {
    Vector2 position;
    TowerType type;
    int operand;
    bool operand_edit_mode;
    bool just_opened;
} PendingPlacement;

typedef struct {
    Scene scene;
    SceneState state;
    PlayLayout layout;
    PlayWorld world;
    PendingPlacement pending;
    bool dragging;
    int dragging_slot_index;
    bool range_reveal;
    Notifier notifier;
} PlayScene;

void play_init(PlayScene* scene);
void play_reset(PlayScene* scene);
SceneRequest play_update(PlayScene* scene, float dt);
void play_draw_world(PlayScene* scene);
SceneRequest play_draw_ui(PlayScene* scene);
void play_free(PlayScene* scene);

#endif
