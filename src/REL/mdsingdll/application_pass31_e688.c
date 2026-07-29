#include "dolphin/mtx/GeoTypes.h"
#include "dolphin/types.h"

typedef s16 HU3D_MODELID;
typedef s16 HU3D_ANIMID;

typedef struct MdsingModelEntry {
    HU3D_MODELID modelId;
    HU3D_ANIMID animId[4];
    s16 unk_A;
    Vec pos;
    Vec rot;
    Vec scale;
    s16 unk_30;
    u8 unk_32;
    u8 unk_33;
    u8 unk_34;
    u8 unk_35;
    u8 unk_36;
    u8 unk_37;
} MDSING_MODEL_ENTRY;

extern MDSING_MODEL_ENTRY lbl_1_bss_E74[16];
extern Vec lbl_1_data_7A8[];

extern const float lbl_1_rodata_1E4;
extern const float lbl_1_rodata_1F4;
extern const float lbl_1_rodata_284;
extern const float lbl_1_rodata_84;
extern const float lbl_1_rodata_B0;

void Hu3DModelPosGet(HU3D_MODELID modelId, Vec *pos);
void Hu3DModelPosSetV(HU3D_MODELID modelId, Vec *pos);
void Hu3DModelRotGet(HU3D_MODELID modelId, Vec *rot);
void Hu3DModelRotSetV(HU3D_MODELID modelId, Vec *rot);
void Hu3DModelScaleGet(HU3D_MODELID modelId, Vec *scale);
void Hu3DModelScaleSetV(HU3D_MODELID modelId, Vec *scale);
void Hu3DModelLayerSet(HU3D_MODELID modelId, s16 layerNo);

static inline float blend_value(float current, float target)
{
    if (current == target) {
        return target;
    }
    return (target + (current * lbl_1_rodata_1F4)) /
        lbl_1_rodata_B0;
}

static inline void blend_vector(Vec *current, const Vec *target)
{
    current->x = blend_value(current->x, target->x);
    current->y = blend_value(current->y, target->y);
    current->z = blend_value(current->z, target->z);
}

void fn_1_E688(s16 modelNo)
{
    Vec value;
    MDSING_MODEL_ENTRY *entry = &lbl_1_bss_E74[modelNo + 11];

    Hu3DModelPosGet(entry->modelId, &value);
    blend_vector(&value, &lbl_1_data_7A8[modelNo + 6]);
    Hu3DModelPosSetV(entry->modelId, &value);
    if (entry->unk_30++ > 30) {
        entry->unk_30 = 35;
        Hu3DModelRotGet(entry->modelId, &value);
        value.y += lbl_1_rodata_1E4;
        if (value.y >= lbl_1_rodata_84) {
            value.y -= lbl_1_rodata_84;
        }
        Hu3DModelRotSetV(entry->modelId, &value);
    }
    Hu3DModelScaleGet(entry->modelId, &value);
    value.y = value.x =
        blend_value(value.x, lbl_1_rodata_284);
    Hu3DModelScaleSetV(entry->modelId, &value);
    Hu3DModelLayerSet(entry->modelId, 2);
}
