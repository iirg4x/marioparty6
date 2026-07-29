#include <dolphin/gx/GXStruct.h>
#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;
typedef s16 HU3D_LIGHTID;

typedef struct StaffLightVectors {
    HuVecF entries[2];
} STAFF_LIGHT_VECTORS;

extern const STAFF_LIGHT_VECTORS lbl_1_rodata_C8;
extern const STAFF_LIGHT_VECTORS lbl_1_rodata_E0;
extern const GXColor lbl_1_rodata_F8;
extern HU3D_LIGHTID lbl_1_bss_82C[2];

HU3D_LIGHTID Hu3DGLightCreateV(
    HuVecF *position, HuVecF *direction, GXColor *color);
void Hu3DGLightInfinitytSet(HU3D_LIGHTID lightId);
void Hu3DGLightStaticSet(HU3D_LIGHTID lightId, BOOL isStatic);

void fn_1_F68(void)
{
    STAFF_LIGHT_VECTORS positions = lbl_1_rodata_C8;
    STAFF_LIGHT_VECTORS directions = lbl_1_rodata_E0;
    GXColor color = lbl_1_rodata_F8;
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_82C[i] = Hu3DGLightCreateV(
            &positions.entries[i], &directions.entries[i], &color);
        Hu3DGLightInfinitytSet(lbl_1_bss_82C[i]);
        Hu3DGLightStaticSet(lbl_1_bss_82C[i], TRUE);
    }
}
