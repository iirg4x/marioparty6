#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;

extern const f32 lbl_1_rodata_10;
extern const f32 lbl_1_rodata_40;
extern const f32 lbl_1_rodata_64;
extern const f32 lbl_1_rodata_80;
extern const f32 lbl_1_rodata_98;
extern const f32 lbl_1_rodata_B0;
extern const f32 lbl_1_rodata_B4;
extern const f32 lbl_1_rodata_B8;
extern const f32 lbl_1_rodata_BC;
extern const f32 lbl_1_rodata_C0;
extern const f32 lbl_1_rodata_C4;

void Hu3DCameraCreate(u32 cameraBit);
void Hu3DCameraPerspectiveSet(u32 cameraBit, float fov, float near,
    float far, float aspect);
void Hu3DCameraViewportSet(u32 cameraBit, float x, float y, float width,
    float height, float near, float far);
void Hu3DCameraPosSet(u32 cameraBit, float posX, float posY, float posZ,
    float upX, float upY, float upZ, float targetX, float targetY,
    float targetZ);

void fn_1_E20(void)
{
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, lbl_1_rodata_B0, lbl_1_rodata_B4,
        lbl_1_rodata_B8, lbl_1_rodata_BC);
    Hu3DCameraViewportSet(1, lbl_1_rodata_10, lbl_1_rodata_10,
        lbl_1_rodata_C0, lbl_1_rodata_C4, lbl_1_rodata_10, lbl_1_rodata_40);
    Hu3DCameraPosSet(1, lbl_1_rodata_10, lbl_1_rodata_64,
        lbl_1_rodata_80, lbl_1_rodata_10, lbl_1_rodata_40,
        lbl_1_rodata_10, lbl_1_rodata_10, lbl_1_rodata_64,
        lbl_1_rodata_98);
}
