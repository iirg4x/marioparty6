#include "game/hu3d.h"
#include "humath.h"

extern const float lbl_1_rodata_64;
extern const double lbl_1_rodata_68;
extern const double lbl_1_rodata_78;
extern const float lbl_1_rodata_80;
extern const float lbl_1_rodata_84;
extern const float lbl_1_rodata_88;
extern const float lbl_1_rodata_8C;
extern const float lbl_1_rodata_90;

static inline float blend_rotation(float current, float target)
{
    if (current == target) {
        return target;
    }
    return (target + (current * lbl_1_rodata_8C)) / lbl_1_rodata_90;
}

static inline float linear_value(
    float start, float end, float time, float duration)
{
    if (time <= lbl_1_rodata_64) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + ((time / duration) * (end - start));
}

static inline void move_model(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration)
{
    HuVecF modelPos;
    HuVecF modelRot;
    HuVecF pos;
    HuVecF rot;

    Hu3DModelPosGet(modelId, &modelPos);
    Hu3DModelRotGet(modelId, &modelRot);
    pos.x = linear_value(start->x, end->x, time, duration);
    pos.y = linear_value(start->y, end->y, time, duration);
    pos.z = linear_value(start->z, end->z, time, duration);
    modelPos.x -= pos.x;
    modelPos.z -= pos.z;
    rot.y = -(lbl_1_rodata_78
        * (atan2(modelPos.x, -modelPos.z) / lbl_1_rodata_68));
    if (modelRot.y - rot.y > lbl_1_rodata_80) {
        modelRot.y -= lbl_1_rodata_84;
    } else if (modelRot.y - rot.y < lbl_1_rodata_88) {
        modelRot.y += lbl_1_rodata_84;
    }
    rot.x = modelRot.x;
    rot.y = blend_rotation(modelRot.y, rot.y);
    rot.z = modelRot.z;
    Hu3DModelPosSetV(modelId, &pos);
    Hu3DModelRotSetV(modelId, &rot);
}

void fn_1_1A9C(HU3D_MODELID modelId, float rotY)
{
    HuVecF modelRot;
    HuVecF rot;

    Hu3DModelRotGet(modelId, &modelRot);
    rot.y = rotY;
    if (modelRot.y - rot.y > lbl_1_rodata_80) {
        modelRot.y -= lbl_1_rodata_84;
    } else if (modelRot.y - rot.y < lbl_1_rodata_88) {
        modelRot.y += lbl_1_rodata_84;
    }
    rot.x = modelRot.x;
    rot.y = blend_rotation(modelRot.y, rot.y);
    rot.z = modelRot.z;
    Hu3DModelRotSetV(modelId, &rot);
}

inline void fn_1_1A9C(HU3D_MODELID modelId, float rotY);

void fn_1_1BDC(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration, float rotY)
{
    if (time <= duration) {
        move_model(modelId, start, end, time, duration);
    } else {
        fn_1_1A9C(modelId, rotY);
    }
}
