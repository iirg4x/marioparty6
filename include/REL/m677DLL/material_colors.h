// Fades model materials toward black except for the named bitmaps.

#ifndef REL_SEER_TERROR_MATERIAL_COLORS_H
#define REL_SEER_TERROR_MATERIAL_COLORS_H
#include "dolphin/types.h"
#include <vector>
#include "REL/framework/model.h"

extern "C" {
#include "game/hsfformat.h"
}

// Stores the color information for one material in the model.
struct MaterialColor {
    u8 red;
    u8 green;
    u8 blue;
};

// Updates the model's material colors during a transition.
struct MaterialColors {
    Model *model;
    std::vector<const char *> names;
    std::vector<MaterialColor> colors;
    f32 unk1C; // The game's code never reads this field.

    explicit MaterialColors(Model *owner);
    ~MaterialColors();
    void addTextureFilter(const char *name);
    void applyMaterialColorScale(f32 value);
};

#endif
