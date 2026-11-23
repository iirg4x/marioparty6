// Fades model materials toward black except for the named bitmaps.

#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include <vector>
#include "REL/framework/model.h"
extern "C" {
#include "game/hu3d.h"
#include "game/hsfex.h"
#include "game/hsfformat.h"
#include "string.h"
}

#include "REL/m677DLL/material_colors.h"

// Saves the source colors of the model materials.
MaterialColors::MaterialColors(Model *owner) : model(owner), unk1C(0.0f)
{
    HSF_MATERIAL *materialEnd;
    HSF_DATA *hsf = lbl_802C01C8[*model].hsf;
    HSF_MATERIAL *material = hsf->material;
    materialEnd = material + hsf->materialNum;
    while (material != materialEnd) {
        MaterialColor color;
        color.red = (u8) material->color[0];
        color.green = (u8) material->color[1];
        color.blue = (u8) material->color[2];
        colors.push_back(color);
        ++material;
    }
}

// Releases the saved material colors and texture names.
MaterialColors::~MaterialColors()
{
}

// Adds a texture name to the material filter.
void MaterialColors::addTextureFilter(const char *name)
{
    names.push_back(name);
}

// Tests whether a material uses any texture in the filter.
static bool materialUsesFilteredTexture(HSF_DATA *hsf, const HSF_MATERIAL *material,
                                        const std::vector<const char *> &names)
{
    if (names.empty()) {
        return false;
    }
    for (u32 index = 0; index < material->attrNum; ++index) {
        HSF_ATTRIBUTE &attribute = hsf->attribute[material->attr[index]];
        const char *bitmapName = attribute.bitmap->name;
        std::vector<const char *>::const_iterator name = names.begin();
        std::vector<const char *>::const_iterator end = names.end();
        for (; name != end; ++name) {
            if (strcmp(*name, bitmapName) == 0) {
                return true;
            }
        }
    }
    return false;
}

// Skips filtered materials and scales the remaining saved RGB by clamp(1 - value, 0, 1).
void MaterialColors::applyMaterialColorScale(float value)
{
    float scale;
    double clamped;
    HSF_MATERIAL *material;
    HSF_DATA *hsf = lbl_802C01C8[*model].hsf;
    material = hsf->material;
    HSF_MATERIAL *materialEnd = material + hsf->materialNum;
    std::vector<MaterialColor>::iterator color = colors.begin();
    std::vector<MaterialColor>::iterator colorEnd = colors.end();
    if ((1.0 - value) < 0.0) {
        clamped = 0.0;
    } else if ((1.0 - value) > 1.0) {
        clamped = 1.0;
    } else {
        clamped = 1.0 - value;
    }
    scale = clamped;
    for (; material != materialEnd && color != colorEnd; ++material, ++color) {
        if (!materialUsesFilteredTexture(hsf, material, names)) {
            material->color[0] = (u8) (scale * color->red);
            material->color[1] = (u8) (scale * color->green);
            material->color[2] = (u8) (scale * color->blue);
        }
    }
    unk1C = value;
}
