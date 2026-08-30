#include <raylib.h>
#include <raygui.h>
#include <raymath.h>

#include "common.h"
#include "scenes/play_scene.h"
#include "world/play_world.h"
#include "ui/notifier.h"

static Vector2 mouse_to_grid(PlayLayout layout) {
    Vector2 mouse = GetMousePosition();

    Vector2 world_position = GetScreenToWorld2D(mouse, layout.world_camera);
    Vector2 grid_position = {
        floorf(world_position.x),
        floorf(world_position.y)
    };

    return grid_position;
}

static void layout_init(PlayLayout* layout) {
    int width = GetScreenWidth();
    int height = GetScreenHeight();

    layout->world_area = (Rectangle){
        0,
        0,
        width,
        height - INVENTORY_HEIGH
    };
    layout->world_camera = (Camera2D){
        .offset = {layout->world_area.x, layout->world_area.y},
        .target = {0, 0},
        .rotation = 0,
        .zoom = CELL
    };
    layout->inventory_area = (Rectangle){
        0,
        height - INVENTORY_HEIGH,
        width,
        INVENTORY_HEIGH
    };
}

static void draw_hover(PlayScene* scene) {
    if (scene->dragging || scene->state == SCENE_OPERAND_PROMPT) return;

    Vector2 mouse = mouse_to_grid(scene->layout);

    DrawRectangleLinesEx(
        (Rectangle){
            mouse.x,
            mouse.y,
            1,
            1,
        },
        0.1f,
        BLACK
    );
}

static void draw_dragging_tower(PlayScene* scene) {
    Vector2 mouse;
    TowerType type;

    if (scene->state == SCENE_NORMAL) {
        if (!scene->dragging) return;
        if (!CheckCollisionPointRec(GetMousePosition(), scene->layout.world_area)) return;

        mouse = mouse_to_grid(scene->layout);
        type = inventory_get_tower_type(&scene->world.inventory, scene->dragging_slot_index);
    } else if (scene->state == SCENE_OPERAND_PROMPT) {
        mouse = scene->pending.position;
        type = scene->pending.type;
    } else return;

    Color drag_color = GRAY;
    if (tower_placement_validate(&scene->world, mouse, type) != PLACEMENT_SUCCESS) {
        drag_color = MAROON;
    }

    DrawRectangleRec(
        (Rectangle){
            mouse.x,
            mouse.y,
            1,
            1,
        },
        drag_color
    );
    DrawCircleLinesV(
        (Vector2){
            mouse.x + 0.5f,
            mouse.y + 0.5f
        },
        tower_get_definition(type)->attack_radius,
        drag_color 
    );
}

static void draw_towers(PlayScene* scene) {
    Vector2 mouse = mouse_to_grid(scene->layout);

    for (int i = 0; i < scene->world.tower_count; i++) {
        Tower tower = scene->world.towers[i];
        DrawRectangleRec(
            (Rectangle){
                tower.position.x,
                tower.position.y,
                1,
                1
            }, 
            BLACK
        );

        if (scene->range_reveal || Vector2Equals(mouse, tower.position)) {
            DrawCircleLinesV(
                (Vector2){
                    tower.position.x + 0.5f,
                    tower.position.y + 0.5f,
                },
                tower.attack_radius,
                BLACK
            );
        }
    }
}

static void draw_towers_text(PlayScene* scene) {
    for (int i = 0; i < scene->world.tower_count; i++) {
        Tower tower = scene->world.towers[i];

        Vector2 screen_position = GetWorldToScreen2D(
            (Vector2){
                tower.position.x + 0.5f,
                tower.position.y + 0.5f
            },
            scene->layout.world_camera
        );

        const char *text = TextFormat("%s", tower_get_definition(tower.type)->display_name);
        int width = MeasureText(text, ENEMY_TEXT_SIZE);

        int text_x = (int)roundf(
            screen_position.x - (float)width / 2.0f
        );
        int text_y = (int)roundf(
            screen_position.y - (float)ENEMY_TEXT_SIZE / 2.0f
        );

        DrawText(
            text,
            text_x,
            text_y,
            ENEMY_TEXT_SIZE,
            LIGHTGRAY
        );
    }
}

static void draw_path(PlayScene* scene) {
    for (int i = 1; i < scene->world.path.count; i++) {
        Vector2 start = scene->world.path.points[i - 1];
        Vector2 end = scene->world.path.points[i];

        if (start.y == end.y) {
            DrawRectangleRec(
                (Rectangle){
                    fminf(start.x, end.x),
                    start.y,
                    fabsf(end.x - start.x) + 1.0f,
                    1.0f
                },
                LIGHTGRAY
            );
        } else if (start.x == end.x) {
            DrawRectangleRec(
                (Rectangle){
                    start.x,
                    fminf(start.y, end.y),
                    1.0f,
                    fabsf(end.y - start.y) + 1.0f
                },
                LIGHTGRAY
            );
        }
    }
}

static void draw_enemies(PlayScene* scene) {
    for (int i = 0; i < scene->world.enemy_count; i++) {
        Enemy enemy = scene->world.enemies[i];
        DrawRectangleRec(
            (Rectangle){
                enemy.position.x,
                enemy.position.y,
                1,
                1,
            },
            RED
        );
    }
}

static void draw_enemies_text(PlayScene* scene) {
    for (int i = 0; i < scene->world.enemy_count; i++) {
        Enemy enemy = scene->world.enemies[i];

        Vector2 screen_position = GetWorldToScreen2D(
            (Vector2){
                enemy.position.x + 0.5f,
                enemy.position.y + 0.5f
            },
            scene->layout.world_camera
        );

        const char *text = TextFormat("%d", enemy.value);
        int width = MeasureText(text, ENEMY_TEXT_SIZE);

        int text_x = (int)roundf(
            screen_position.x - (float)width / 2.0f
        );
        int text_y = (int)roundf(
            screen_position.y - (float)ENEMY_TEXT_SIZE / 2.0f
        );

        DrawText(
            text,
            text_x,
            text_y,
            ENEMY_TEXT_SIZE,
            BLACK
        );
    }
}

static void draw_projectiles(PlayScene* scene) {
    for (int i = 0; i < scene->world.tower_count; i++) {
        Tower tower = scene->world.towers[i];
        for (int j = 0; j < tower.projectile_count; j++) {
            DrawCircleV(
                (Vector2){
                    tower.projectiles[j].position.x + 0.5f,
                    tower.projectiles[j].position.y + 0.5f,
                },
                PROJECTILE_RADIUS,
                BLACK
            );
        }
    }
}

static void draw_wave_progress(PlayScene* scene) {
    int resolved_enemies = scene->world.wave.spawned_enemies - scene->world.enemy_count;
    int total_enemies = scene->world.wave.total_enemies;
    float progress;

    if (total_enemies == 0) {
        progress = 1.0f;
    } else {
        progress = (float)resolved_enemies / (float)total_enemies;
    }
    progress = Clamp(progress, 0.0f, 1.0f);

    Rectangle background = (Rectangle){
        (GetScreenWidth() - WAVE_PROGRESS_WIDTH) / 2,
        10,
        WAVE_PROGRESS_WIDTH,
        5
    };
    Rectangle infill = (Rectangle){
        background.x,
        background.y,
        background.width * progress,
        background.height
    };

    const char *text = TextFormat(
        "Wave 1 - %d %% - %d / %d cleared",
        (int)(progress * 100),
        resolved_enemies,
        total_enemies
    );
    int width = MeasureText(text, INVENTORY_TEXT_SIZE);
    int x = background.x + (background.width - width) / 2;
    int y = background.y + background.height + 5;   

    DrawText(text, x, y, 20, BLACK);
    DrawRectangleRec(background, GRAY);
    DrawRectangleRec(infill, BLACK);
}

static void draw_hud(PlayScene* scene) {
    draw_wave_progress(scene);
    DrawText(TextFormat("HP %d", scene->world.health), 5, 5, 20, BLACK);
    DrawText(TextFormat("$   %d", scene->world.currency), 5, 25, 20, BLACK);
}

static Rectangle inventory_slot_rect(
    Inventory* inventory,
    int index
) {
    float total_width =
        inventory->slot_count * INVENTORY_SLOT_SIZE +
        (inventory->slot_count - 1) * INVENTORY_GAP;

    float start_x = (GetScreenWidth() - total_width) / 2.0f;
    float x = start_x + index * (INVENTORY_SLOT_SIZE + INVENTORY_GAP);
    float y = GetScreenHeight() - INVENTORY_SLOT_SIZE - INVENTORY_BOTTOM_MARGIN;

    return (Rectangle){
        x,
        y,
        INVENTORY_SLOT_SIZE,
        INVENTORY_SLOT_SIZE
    };
}

static int inventory_slot_at(Inventory* inventory, Vector2 mouse) {
    for (int i = 0; i < inventory->slot_count; i++) {
        Rectangle slot = inventory_slot_rect(inventory, i);

        if (CheckCollisionPointRec(mouse, slot)) {
            return i;
        }
    }

    return -1;
}

static void pending_placement_begin(PlayScene* scene, Vector2 mouse, TowerType type) {
    scene->state = SCENE_OPERAND_PROMPT;
    scene->pending.position = mouse;
    scene->pending.type = type;
    scene->pending.operand = tower_get_definition(type)->default_operand;
    scene->pending.operand_edit_mode = false;
    scene->pending.just_opened = true;
}

static void pending_placement_place(PlayScene* scene) {
    scene->pending.operand_edit_mode = false;

    inventory_select_slot(&scene->world.inventory, -1);

    play_world_place_tower(
        &scene->world,
        scene->pending.position,
        scene->pending.type,
        scene->pending.operand
    );

    scene->state = SCENE_NORMAL;
}

static void pending_placement_cancel(PlayScene* scene) {
    scene->pending.operand_edit_mode = false;

    inventory_select_slot(&scene->world.inventory, -1);

    scene->state = SCENE_NORMAL;
}

static void begin_tower_placement(PlayScene* scene, Vector2 mouse) {
    if (scene->dragging_slot_index == -1) return;

    TowerType type = inventory_get_tower_type(&scene->world.inventory, scene->dragging_slot_index);

    TowerPlacementResult result = tower_placement_validate(&scene->world, mouse, type);

    if (result == PLACEMENT_SUCCESS) {
        pending_placement_begin(scene, mouse, type);

        if (!tower_get_definition(type)->operand_required) {
            pending_placement_place(scene);
        }
    } else {
        const char* message;

        switch (result) {
            case PLACEMENT_INSUFFICIENT_CURRENCY:
                message = "Insufficient currency";
                break;
            case PLACEMENT_OCCUPIED:
                message = "Position occupied";
                break;
            case PLACEMENT_ON_PATH:
                message = "Invalid tower placement";
                break;
            default:
                message = "";
                break;
        }

        notifier_show(&scene->notifier, NOTIFICATION_ERROR, message);
        inventory_select_slot(&scene->world.inventory, -1);
    }
}

static void draw_inventory_ui(PlayScene* scene) {
    GuiPanel(scene->layout.inventory_area, NULL);

    Inventory* inventory = &scene->world.inventory;

    for (int i = 0; i < inventory->slot_count; i++) {
        Rectangle slot = inventory_slot_rect(inventory, i);
        TowerType type = inventory->slots[i].type;
        int cost = tower_get_definition(type)->cost;

        const char *text = TextFormat(
            "%s\n$%d",
            tower_get_definition(type)->display_name,
            cost
        );
        int width = MeasureText(text, INVENTORY_TEXT_SIZE);
        int x = slot.x + (slot.width - width) / 2;
        int y = slot.y + (slot.height - INVENTORY_TEXT_SIZE*2) / 2;

        float progress = (float)scene->world.currency / (float)cost;
        progress = Clamp(progress, 0.0f, 1.0f);

        Rectangle fill = {
            slot.x,
            slot.y + slot.height * (1.0f - progress),
            slot.width,
            slot.height * progress
        };

        if (scene->world.currency >= cost) {
            DrawRectangleRoundedLinesEx(
                slot,
                0.2f,
                16,
                1.0f,
                (Color){75, 202, 62, 255}
            );
        } else {
            // DrawRectangleRec(fill, LIGHTGRAY);
            int clip_y = (int)floorf(fill.y);
            int clip_bottom = (int)ceilf(slot.y + slot.height);

            BeginScissorMode(
                (int)floorf(fill.x),
                clip_y,
                (int)ceilf(fill.width),
                clip_bottom - clip_y
            );
            DrawRectangleRounded(
                slot,
                0.2f,
                16,
                LIGHTGRAY
            );
            EndScissorMode();
            DrawRectangleRoundedLinesEx(
                slot,
                0.2f,
                16,
                1.0f,
                DARKGRAY
            );
        }

        DrawText(text, x, y, INVENTORY_TEXT_SIZE, BLACK);

        if (inventory->selected_slot == i) {
            DrawRectangleRoundedLinesEx(
                slot,
                0.2f,
                16,
                2.0f,
                BLACK
            );
        }
    }
}

static void draw_notifier(PlayScene* scene) {
    Notifier* notifier = &scene->notifier;
    if (notifier->type == NOTIFICATION_NONE) return;

    int width = MeasureText(notifier->message, NOTIFIER_TEXT_SIZE);
    int x = (GetScreenWidth() - width) / 2;
    int y = scene->layout.inventory_area.y - NOTIFIER_TEXT_SIZE - 5;   

    float alpha = Clamp(
        notifier->time_remaining / NOTIFICATION_FADE_DURATION,
        0.0f,
        1.0f
    );

    Color message_color = DARKGRAY;

    switch (notifier->type) {
        case NOTIFICATION_ERROR:
            message_color = MAROON;
            break;
        default:
            break;
    }

    DrawText(
        notifier->message,
        x,
        y,
        NOTIFIER_TEXT_SIZE,
        Fade(message_color, alpha)
    );
}

static void draw_operand_prompt_ui(PlayScene* scene) {
    if (scene->pending.just_opened) {
        scene->pending.just_opened = false;
        return;
    }

    Rectangle popup = {
        GetScreenWidth() / 2 - 150,
        GetScreenHeight() / 2 - 100,
        300,
        200
    };

    GuiPanel(popup, NULL);

    GuiLabel(
        (Rectangle){ popup.x + 40, popup.y + 30, 220, 30 },
        "Choose operand"
    );

    if (GuiValueBox(
        (Rectangle){ popup.x + 50, popup.y + 75, 200, 40 },
        NULL,
        &scene->pending.operand,
        tower_get_definition(scene->pending.type)->min_operand,
        tower_get_definition(scene->pending.type)->max_operand,
        scene->pending.operand_edit_mode
    )) {
        scene->pending.operand_edit_mode = !scene->pending.operand_edit_mode;
    }

    if (GuiButton(
            (Rectangle){ popup.x + 50, popup.y + 135, 90, 35 },
            "Place"
        )) {
        pending_placement_place(scene);
    }

    if (GuiButton(
        (Rectangle){ popup.x + 160, popup.y + 135, 90, 35 },
        "Cancel"
    )) {
        pending_placement_cancel(scene);
    }
}

void draw_game_over_ui(PlayScene* scene, SceneRequest* request) {
    if (scene->world.state != WORLD_PLAYING) {
        DrawRectangle(
            0, 0,
            GetScreenWidth(),
            GetScreenHeight(),
            Fade(BLACK, 0.5f)
        );

        Rectangle popup = {
            GetScreenWidth() / 2 - 150,
            GetScreenHeight() / 2 - 100,
            300,
            200
        };

        GuiPanel(popup, NULL);

        const char* text = scene->world.state == WORLD_WON 
            ? "You won!" : "You lost!";

        GuiLabel(
            (Rectangle){ popup.x + 80, popup.y + 35, 140, 30 },
            text
        );

        if (GuiButton(
            (Rectangle){ popup.x + 75, popup.y + 120, 150, 40 },
            "Go to menu"
        )) {
            request->type = REQUEST_SWITCH;
            request->target = MENU_SCENE;
        }
    }
}

void play_init(PlayScene* scene) {
    scene->scene.type = PLAY_SCENE;
    scene->state = SCENE_NORMAL;

    layout_init(&scene->layout);

    play_world_init(
        &scene->world,
        scene->layout.world_area.width / CELL,
        scene->layout.world_area.height / CELL
    );

    scene->dragging = false;
    scene->dragging_slot_index = -1;

    scene->range_reveal = false;

    notifier_init(&scene->notifier);

    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_MIDDLE);
}

void play_reset(PlayScene* scene) {
    play_world_init(
        &scene->world,
        scene->layout.world_area.width / CELL,
        scene->layout.world_area.height / CELL
    );
    scene->state = SCENE_NORMAL;
    scene->dragging = false;
    scene->dragging_slot_index = -1;
    scene->range_reveal = false;

    notifier_init(&scene->notifier);
}

SceneRequest play_update(PlayScene* scene, float dt) {
    SceneRequest request = {.type = REQUEST_NONE};

    if (IsKeyPressed(KEY_ESCAPE)) {
        request.type = REQUEST_SWITCH;
        request.target = MENU_SCENE;
    } else if (IsKeyPressed(KEY_R)) {
        scene->range_reveal = !scene->range_reveal;

        const char* message = scene->range_reveal
            ? "Tower ranges ON"
            : "Tower ranges OFF";

        notifier_show(&scene->notifier, NOTIFICATION_INFO, message);
    }

    if (scene->world.state == WORLD_PLAYING && scene->state != SCENE_OPERAND_PROMPT) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse = GetMousePosition();
            Inventory* inventory = &scene->world.inventory;
            int slot_index = inventory_slot_at(inventory, mouse);
            scene->dragging_slot_index = slot_index;
            if (slot_index != -1) {
                inventory_select_slot(inventory, slot_index);
                scene->dragging = true;
            }
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            if (scene->dragging == true) {
                if (CheckCollisionPointRec(GetMousePosition(), scene->layout.world_area)) {
                    begin_tower_placement(scene, mouse_to_grid(scene->layout));
                } else {
                    Inventory* inventory = &scene->world.inventory;
                    inventory_select_slot(inventory, -1);
                }
            }
            scene->dragging = false;
        }
    }

    if (scene->state == SCENE_NORMAL) {
        play_world_update(&scene->world, dt);
    }

    notifier_update(&scene->notifier, dt);

    return request;
}

void play_draw_world(PlayScene* scene) {
    ClearBackground(WHITE);

    BeginScissorMode(
        scene->layout.world_area.x,
        scene->layout.world_area.y,
        scene->layout.world_area.width,
        scene->layout.world_area.height
    );
    BeginMode2D(scene->layout.world_camera);
    
    draw_path(scene);
    draw_towers(scene);
    draw_enemies(scene);

    EndMode2D();
    draw_towers_text(scene);
    draw_enemies_text(scene);
    BeginMode2D(scene->layout.world_camera);

    draw_projectiles(scene);
    draw_dragging_tower(scene);
    draw_hover(scene);
    
    EndMode2D();

    draw_hud(scene);

    EndScissorMode();
}

SceneRequest play_draw_ui(PlayScene* scene) {
    SceneRequest request = {.type = REQUEST_NONE};

    draw_inventory_ui(scene);

    draw_notifier(scene);
    
    if (scene->state == SCENE_OPERAND_PROMPT) {
        draw_operand_prompt_ui(scene);
    }

    draw_game_over_ui(scene, &request);

    return request;
}

void play_free(PlayScene* scene) {

}
