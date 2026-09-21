#define _MATH_H
#include "REL/m602Dll.h"

s16 lbl_1_bss_18;

u8 fn_1_D54(s16 arg0)
{
    Point3d pos;
    Point3d target;
    f32 temp_f31;

    if (arg0 < 60) {
        temp_f31 = arg0;
        if (arg0 == 1) {
            fn_1_48B4();
        }
        fn_1_4E48();
    } else if (arg0 < 240) {
        temp_f31 = (f32) (arg0 - 60);
        pos.x = lbl_1_data_9C[1].pos.x - lbl_1_data_9C->pos.x;
        pos.y = lbl_1_data_9C[1].pos.y - lbl_1_data_9C->pos.y;
        pos.z = lbl_1_data_9C[1].pos.z - lbl_1_data_9C->pos.z;
        pos.x = (pos.x / 180.0f) * temp_f31;
        pos.y = (pos.y / 180.0f) * temp_f31;
        pos.z = (pos.z / 180.0f) * temp_f31;
        pos.x += lbl_1_data_9C->pos.x;
        pos.y += lbl_1_data_9C->pos.y;
        pos.z += lbl_1_data_9C->pos.z;
        target.x = lbl_1_data_9C[1].target.x - lbl_1_data_9C->target.x;
        target.y = lbl_1_data_9C[1].target.y - lbl_1_data_9C->target.y;
        target.z = lbl_1_data_9C[1].target.z - lbl_1_data_9C->target.z;
        target.x = (target.x / 180.0f) * temp_f31;
        target.y = (target.y / 180.0f) * temp_f31;
        target.z = (target.z / 180.0f) * temp_f31;
        target.x += lbl_1_data_9C->target.x;
        target.y += lbl_1_data_9C->target.y;
        target.z += lbl_1_data_9C->target.z;
        lbl_1_data_28.pos = pos;
        lbl_1_data_28.target = target;
        Hu3DCameraPosSetV(lbl_1_data_28.cameraBit, &lbl_1_data_28.pos, &lbl_1_data_28.up, &lbl_1_data_28.target);
        if (temp_f31 > 90.0f) {
            fn_1_4EEC();
        }
    } else if (arg0 < 300) {
        temp_f31 = (f32) (arg0 - 240);
        pos.x = lbl_1_data_78.pos.x - lbl_1_data_9C[1].pos.x;
        pos.y = lbl_1_data_78.pos.y - lbl_1_data_9C[1].pos.y;
        pos.z = lbl_1_data_78.pos.z - lbl_1_data_9C[1].pos.z;
        pos.x = (pos.x / 60.0f) * temp_f31;
        pos.y = (pos.y / 60.0f) * temp_f31;
        pos.z = (pos.z / 60.0f) * temp_f31;
        pos.x += lbl_1_data_9C[1].pos.x;
        pos.y += lbl_1_data_9C[1].pos.y;
        pos.z += lbl_1_data_9C[1].pos.z;
        target.x = lbl_1_data_78.target.x - lbl_1_data_9C[1].target.x;
        target.y = lbl_1_data_78.target.y - lbl_1_data_9C[1].target.y;
        target.z = lbl_1_data_78.target.z - lbl_1_data_9C[1].target.z;
        target.x = (target.x / 60.0f) * temp_f31;
        target.y = (target.y / 60.0f) * temp_f31;
        target.z = (target.z / 60.0f) * temp_f31;
        target.x += lbl_1_data_9C[1].target.x;
        target.y += lbl_1_data_9C[1].target.y;
        target.z += lbl_1_data_9C[1].target.z;
        lbl_1_data_28.pos = pos;
        lbl_1_data_28.target = target;
        Hu3DCameraPosSetV(lbl_1_data_28.cameraBit, &lbl_1_data_28.pos, &lbl_1_data_28.up, &lbl_1_data_28.target);
        if ((s32) temp_f31 == 53) {
            fn_1_4D68();
            fn_1_4998();
        }
    } else {
    lbl_1_data_28.pos = lbl_1_data_78.pos;
    lbl_1_data_28.up = lbl_1_data_78.up;
    lbl_1_data_28.target = lbl_1_data_78.target;
    Hu3DCameraPosSetV(lbl_1_data_28.cameraBit, &lbl_1_data_28.pos, &lbl_1_data_28.up, &lbl_1_data_28.target);
    fn_1_4D68();
    return 1U;
    }
    return 0U;
}

void fn_1_1418(s16 arg0)
{
    lbl_1_bss_18 = arg0;
}

void fn_1_1428(void)
{
    Point3d sp20;
    Point3d sp14;
    Point3d jitter;
    s16 phase;

    if (lbl_1_bss_18 > 0) {
        phase = lbl_1_bss_18 % 2;
        if (phase == 0) {
            sp20 = lbl_1_data_78.pos;
            sp14 = lbl_1_data_78.target;
            jitter.x = ((u8)frand() % 20) - 10;
            jitter.y = ((u8)frand() % 40) - 20;
            sp20.x += jitter.x;
            sp20.y += jitter.y;
            sp14.x += jitter.x;
            sp14.y += jitter.y;
            Hu3DCameraPosSetV(lbl_1_data_28.cameraBit, &sp20, &lbl_1_data_28.up, &sp14);
        }
    } else {
        lbl_1_data_28.pos = lbl_1_data_78.pos;
        lbl_1_data_28.up = lbl_1_data_78.up;
        lbl_1_data_28.target = lbl_1_data_78.target;
        Hu3DCameraPosSetV(lbl_1_data_28.cameraBit, &lbl_1_data_28.pos, &lbl_1_data_28.up, &lbl_1_data_28.target);
    }
    lbl_1_bss_18 -= 1;
    if (lbl_1_bss_18 < 0) {
        lbl_1_bss_18 = 0;
        lbl_1_data_28.pos = lbl_1_data_78.pos;
        lbl_1_data_28.up = lbl_1_data_78.up;
        lbl_1_data_28.target = lbl_1_data_78.target;
        Hu3DCameraPosSetV(lbl_1_data_28.cameraBit, &lbl_1_data_28.pos, &lbl_1_data_28.up, &lbl_1_data_28.target);
    }
}
