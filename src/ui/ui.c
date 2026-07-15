#include <raylib.h>
#include <stdio.h>

#include "common.h"
#include "ui/ui.h"

void ui_init(UiContext* ui) {
    ui->hot = UI_ID_NONE;
    ui->active = UI_ID_NONE;
    ui->mouse_position = (Vector2){0, 0};
    ui->mouse_down = false;
    ui->mouse_pressed = false;
    ui->mouse_released = false;
}

void ui_update(UiContext* ui) {
    ui->hot = UI_ID_NONE;
    ui->mouse_position = GetMousePosition();
    ui->mouse_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    ui->mouse_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    ui->mouse_released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

bool ui_button(UiContext* ui, UiId id, const char* text, Rectangle rect) {
    bool clicked = false;
    bool hovered = CheckCollisionPointRec(ui->mouse_position, rect);

    if (hovered) ui->hot = id;
    
    if (ui->active == id && ui->mouse_released) {
        if (ui->hot == id) clicked = true;
        ui->active = UI_ID_NONE;
    } else if (ui->hot == id && ui->mouse_pressed) {
        ui->active = id;
    }

    Color color = RED;
    if (ui->active == id) {
        color = GRAY;
    } else if (ui->hot == id) {
        color = GREEN;
    }

    DrawRectangleRec(rect, color);
    DrawText(text, (int)rect.x, (int)rect.y, 20, BLACK);

    return clicked;
}
