#ifndef SCENE_H
#define SCENE_H

#include "common.h"

typedef enum {
    MENU_SCENE,
    PLAY_SCENE,
} SceneType;

typedef struct {
    SceneType type;
} Scene;

typedef enum {
    REQUEST_NONE,
    REQUEST_SWITCH,
    REQUEST_QUIT,
} RequestType;

typedef struct {
    RequestType type;
    SceneType target;
} SceneRequest;

#endif
