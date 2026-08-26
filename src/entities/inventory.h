#ifndef INVENTORY_H
#define INVENTORY_H

#include "common.h"
#include "tower.h"

typedef struct {
    TowerType type;
} Slot;

typedef struct {
    Slot slots[128];
    int slot_count;
    int selected_slot;
} Inventory;

void inventory_init(Inventory* inventory);
void inventory_add_tower(Inventory* inventory, TowerType tower_type);
TowerType inventory_get_tower_type(Inventory* inventory);
void inventory_select_slot(Inventory* inventory, int index);

#endif
