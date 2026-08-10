#ifndef PLAY_SCENE_H
#define PLAY_SCENE_H

#include <raylib.h>

#include "common.h"
#include "scenes/scene.h"

#define CELL 20.0f
#define PROJ_SPEED 10.0f
#define ENEMY_SPEED 5.0f
#define EPSILON 0.000001f

typedef struct {
    int target_enemy_id;
    Vector2 position;
    Vector2 velocity;
    Vector2 target_position;
} Projectile;

typedef struct {
    Vector2 position;
    float attack_interval;
    float time_since_attack;
    Projectile projectiles[128];
    int projectile_count;
} Tower;

typedef struct {
    Vector2 points[128];
    int count;
} Path;

typedef struct {
    int id;
    Vector2 position;
    Vector2 velocity;
    int target;
} Enemy;

typedef struct {
    Scene scene;

    Tower towers[128];
    int tower_count;

    int next_enemy_id;
    Enemy enemies[128];
    int enemy_count;

    Path path;
} PlayScene;

void play_init(PlayScene* scene);
SceneRequest play_update(PlayScene* scene, float dt);
void play_draw_world(PlayScene* scene);
SceneRequest play_draw_ui(PlayScene* scene);
void play_free(PlayScene* scene);

#endif
