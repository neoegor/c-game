#include "common.h"
#include "entities/inventory.h"

void inventory_init(Inventory* inventory) {
    inventory->slot_count = 0;
    inventory->selected_slot = -1;
}

void inventory_add_tower(Inventory* inventory, TowerType tower_type) {
    inventory->slots[inventory->slot_count].type = tower_type;
    inventory->slot_count++;
}

TowerType inventory_get_tower_type(Inventory* inventory, int index) {
    return inventory->slots[index].type;
}

void inventory_select_slot(Inventory* inventory, int index) {
    if ((index < inventory->slot_count && index >= 0) || index == -1) {
        inventory->selected_slot = index;
    }
}
