#include "dolphin/mtx/GeoTypes.h"
#include "dolphin/types.h"
#include "game/object.h"

typedef struct Lbl1Bss1348_s LBL_1_BSS_1348;
typedef void (*LBL_1_BSS_1348_CALLBACK)(
    OMOBJ *obj, LBL_1_BSS_1348 *work);

struct Lbl1Bss1348_s {
    OMOBJ *obj;
    Vec center;
    Vec targetCenter;
    Vec rot;
    Vec targetRot;
    float zoom;
    float targetZoom;
    LBL_1_BSS_1348_CALLBACK callback;
};

extern const float lbl_1_rodata_5C;
extern LBL_1_BSS_1348 lbl_1_bss_1348;

static inline float blend_value(float current, float target, float weight)
{
    if (current == target) {
        return target;
    }
    return (target + (current * (weight - lbl_1_rodata_5C))) / weight;
}

static inline void blend_vector(Vec *current, const Vec *target, float weight)
{
    current->x = blend_value(current->x, target->x, weight);
    current->y = blend_value(current->y, target->y, weight);
    current->z = blend_value(current->z, target->z, weight);
}

void fn_1_279C(LBL_1_BSS_1348 *camera, float weight)
{
    blend_vector(&camera->center, &camera->targetCenter, weight);
    blend_vector(&camera->rot, &camera->targetRot, weight);
    camera->zoom = blend_value(camera->zoom, camera->targetZoom, weight);
}

void fn_1_2A50(LBL_1_BSS_1348_CALLBACK callback)
{
    lbl_1_bss_1348.callback = callback;
}
