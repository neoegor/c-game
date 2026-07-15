#ifndef UI_H
#define UI_H

#include <raylib.h>

#include "common.h"

typedef unsigned int UiId;

enum {
    UI_ID_NONE,
};

typedef struct {
    UiId hot;
    UiId active;
    Vector2 mouse_position;
    bool mouse_down;
    bool mouse_pressed;
    bool mouse_released;
} UiContext;

void ui_init(UiContext* ui);
void ui_update(UiContext* ui);

bool ui_button(UiContext* ui, UiId id, const char* text, Rectangle rect);

#endif
