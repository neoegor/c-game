#include <raylib.h>
#include <raygui.h>
#include <raymath.h>

#include "common.h"
#include "scenes/play_scene.h"
#include "world/play_world.h"

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

static void draw_hover_tower(PlayScene* scene) {
    Vector2 mouse;
    TowerType type;

    if (scene->state == SCENE_NORMAL) {
        // if (scene->world.inventory.selected_slot == -1) return;
        if (!scene->dragging) return;
        mouse = mouse_to_grid(scene->layout);
        // TODO helper here
        type = inventory_get_tower_type(&scene->world.inventory, scene->dragging_slot_index);
    } else if (scene->state == SCENE_OPERAND_PROMPT) {
        mouse = scene->pending.position;
        type = scene->pending.type;
    } else return;

    Color hover_color = GRAY;
    if (!tower_placement_is_allowed(&scene->world, mouse, type)) {
        hover_color = MAROON;
    }

    DrawRectangleRec(
        (Rectangle){
            mouse.x,
            mouse.y,
            1,
            1,
        },
        hover_color
    );
    DrawCircleLinesV(
        (Vector2){
            mouse.x + 0.5f,
            mouse.y + 0.5f
        },
        tower_get_definition(type)->attack_radius,
        hover_color 
    );
}

static void draw_towers(PlayScene* scene) {
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

static void draw_path(PlayScene* scene) {
    Vector2 prev = scene->world.path.points[0];
    for (int i = 1; i < scene->world.path.count; i++) {
        Vector2 current = scene->world.path.points[i];
        if (prev.x < current.x) {
            DrawRectangleRec(
                (Rectangle) {
                    prev.x,
                    prev.y,
                    1 + (current.x - prev.x),
                    1 + (current.y - prev.y),
                },
                LIGHTGRAY
            );
        } else {
            DrawRectangleRec(
                (Rectangle){
                    current.x,
                    prev.y,
                    1 + (prev.x - current.x),
                    1 + (current.y - prev.y),
                },
                LIGHTGRAY
            );
        }
        prev = current;
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

static void draw_hud(PlayScene* scene) {
    DrawText(TextFormat("Health: %d", scene->world.health), 5, 20, 20, BLACK);
    DrawText(TextFormat("Currency: %d", scene->world.currency), 5, 45, 20, BLACK);
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

static void begin_tower_placement(PlayScene* scene, Vector2 mouse) {
    if (scene->dragging_slot_index == -1) return;

    TowerType type = inventory_get_tower_type(&scene->world.inventory, scene->dragging_slot_index);

    if (tower_placement_is_allowed(&scene->world, mouse, type)) {
        scene->state = SCENE_OPERAND_PROMPT;
        scene->pending.position = mouse;
        scene->pending.type = type;
        scene->pending.operand = tower_get_definition(type)->default_operand;
        scene->pending.operand_edit_mode = false;

        if (!tower_get_definition(type)->operand_required) {
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
    }
}

static void draw_inventory_ui(PlayScene* scene) {
    GuiPanel(scene->layout.inventory_area, NULL);

    Inventory* inventory = &scene->world.inventory;

    for (int i = 0; i < inventory->slot_count; i++) {
        Rectangle slot = inventory_slot_rect(inventory, i);
        TowerType type = inventory->slots[i].type;

        const char *text = TextFormat(
            tower_get_definition(type)->display_name
        );

        if (GuiButton(slot, text)) {
            // inventory_select_slot(inventory, i);
        }

        if (inventory->selected_slot == i) {
            DrawRectangleLinesEx(slot, 3.0f, BLACK);
        }
    }
}

void draw_operand_prompt_ui(PlayScene* scene) {
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
        // TODO repetition here
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

    if (GuiButton(
        (Rectangle){ popup.x + 160, popup.y + 135, 90, 35 },
        "Cancel"
    )) {
        scene->pending.operand_edit_mode = false;
        inventory_select_slot(&scene->world.inventory, -1);
        scene->state = SCENE_NORMAL;
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

    play_world_init(&scene->world);

    scene->dragging = false;
    scene->dragging_slot_index = -1;

    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_MIDDLE);
}

void play_reset(PlayScene* scene) {
    play_world_init(&scene->world);
    scene->state = SCENE_NORMAL;
    scene->dragging = false;
    scene->dragging_slot_index = -1;
}

SceneRequest play_update(PlayScene* scene, float dt) {
    SceneRequest request = {.type = REQUEST_NONE};

    if (scene->world.state == WORLD_PLAYING && scene->state != SCENE_OPERAND_PROMPT) {
        // if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        //     Vector2 mouse = GetMousePosition();
        //     Inventory* inventory = &scene->world.inventory;
        //     if (inventory_slot_at(inventory, mouse) == -1) {
        //         begin_tower_placement(scene, mouse_to_grid(scene->layout));
        //     }
        // } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        //     Inventory* inventory = &scene->world.inventory;
        //     inventory_select_slot(inventory, -1);
        // }

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
                }
            }
            scene->dragging = false;
        }
    }

    if (scene->state == SCENE_NORMAL) {
        play_world_update(&scene->world, dt);
    }

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
    draw_enemies_text(scene);
    BeginMode2D(scene->layout.world_camera);

    draw_projectiles(scene);
    draw_hover_tower(scene);
    
    EndMode2D();

    draw_hud(scene);

    EndScissorMode();
}

SceneRequest play_draw_ui(PlayScene* scene) {
    SceneRequest request = {.type = REQUEST_NONE};

    draw_inventory_ui(scene);
    
    if (scene->state == SCENE_OPERAND_PROMPT) {
        draw_operand_prompt_ui(scene);
    }

    draw_game_over_ui(scene, &request);

    return request;
}

void play_free(PlayScene* scene) {

}
