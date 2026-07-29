#include <dolphin/mtx/GeoTypes.h>

#define HU3D_ATTR_DISPOFF (1 << 0)
#define MDRESULT_TRAIL_STATE_FADE_IN 1
#define MDRESULT_TRAIL_ALPHA_STEP 5
#define MDRESULT_TRAIL_ALPHA_MAX 255

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;

typedef struct GXColor {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} GXColor;

typedef struct MdResultTrailWork_s {
    HuVecF *points;
    HuVecF base;
    HuVecF velocity;
    s16 modelIndex;
    s16 state;
    s16 pointCount;
    s16 delay;
    GXColor color;
    s16 unk_28;
    s16 unk_2A;
} MDRESULT_TRAIL_WORK;

extern HU3D_MODELID lbl_1_bss_1480[8];
extern const float lbl_1_rodata_F80;

void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);

void fn_1_22CBC(MDRESULT_TRAIL_WORK *work)
{
    s16 i;

    for (i = work->pointCount - 1; i >= 1; i--) {
        work->points[i - 1].y -= lbl_1_rodata_F80;
        work->points[i].x = work->base.x + work->points[i - 1].x;
        work->points[i].y = work->base.y + work->points[i - 1].y;
        work->points[i].z = work->base.z + work->points[i - 1].z;
    }
    if (work->unk_28 == 0) {
        if (work->state == MDRESULT_TRAIL_STATE_FADE_IN) {
            work->color.a += MDRESULT_TRAIL_ALPHA_STEP;
            if (work->color.a >= MDRESULT_TRAIL_ALPHA_MAX) {
                work->color.a = MDRESULT_TRAIL_ALPHA_MAX;
            }
        } else {
            work->color.a -= MDRESULT_TRAIL_ALPHA_STEP;
            if (work->color.a == 0) {
                work->color.a = 0;
                Hu3DModelAttrSet(lbl_1_bss_1480[work->modelIndex],
                    HU3D_ATTR_DISPOFF);
            }
        }
    }
}
