#ifndef REL_M623DLL_ITEM_EFFECTS_STATE_H
#define REL_M623DLL_ITEM_EFFECTS_STATE_H

#include "REL/framework/vector3.h"

// Tracks one item, its model, motion and visibility.
struct ItemEffectsState {
    unsigned char active;
    // Item kind: 1 for a coin, 2 for a dollar and 3 for a spiny.
    int mode;
    // The game's code never reads this field.
    int unk08;
    short model;
    Vector3 position;
    // Center of the item circle above the terrain.
    Vec basePosition;
    float rotationY;
    float rotationZ;
    // Motion state: 0 idle, 1 playing the hit motion and 2 hidden.
    int motionState;
    int frame;
    // Opacity multiplier for the circle under the item.
    float fade;
};

#endif
