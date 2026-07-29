#include <dolphin/mtx/GeoTypes.h>

extern float lbl_1_rodata_7C;
extern float lbl_1_rodata_C8;
extern float lbl_1_rodata_244;
extern float lbl_1_rodata_274;

void Hu3DCameraPosGet(int cameraBit, Vec *position, Vec *up, Vec *target);
void Hu3DCameraPosSetV(int cameraBit, Vec *position, Vec *up, Vec *target);

void fn_1_951C(void)
{
    Vec pos;
    Vec up;
    Vec target;

    Hu3DCameraPosGet(1, &pos, &up, &target);
    pos.x = lbl_1_rodata_C8;
    pos.y = lbl_1_rodata_274;
    pos.z = lbl_1_rodata_7C;
    target.x = lbl_1_rodata_C8;
    target.y = lbl_1_rodata_244;
    target.z = lbl_1_rodata_C8;
    Hu3DCameraPosSetV(1, &pos, &up, &target);
}
