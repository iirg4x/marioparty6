#define _MATH_H
#include "game/hsfex.h"
#include "math.h"
#include "REL/mdminidll/readonly_scalars.h"
#include "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"

/* Target-backed scalar owner views not yet shared by readonly_scalars.h. */
extern const f64 lbl_1_rodata_78;
extern const f64 lbl_1_rodata_88;
extern const f32 lbl_1_rodata_98;
extern const f32 lbl_1_rodata_9C;

void fn_1_1A58(s16 arg0, Point3d *arg1, Point3d *arg2, f32 farg0, f32 farg1, f32 farg2)
{
    Point3d sp54;
    Point3d sp48;
    Point3d sp3C;
    Point3d sp30;
    Point3d sp24;
    Point3d sp18;

    if (farg0 <= farg1) {
        f32 var_f29;
        f32 var_f28;
        f32 var_f27;
        f32 var_f26;
        f32 var_f25;
        f32 var_f24;
        f32 var_f23;
        f32 var_f21;
        f32 var_f20;
        f32 var_f19;
        f32 var_f18;

        Hu3DModelPosGet(arg0, &sp30);
        Hu3DModelRotGet(arg0, &sp3C);
        var_f21 = arg2->x;
        var_f28 = arg1->x;
        if (farg0 <= lbl_1_rodata_74) {
            var_f29 = var_f28;
        } else if (farg0 >= farg1) {
            var_f29 = var_f21;
        } else {
            var_f29 = var_f28 + ((farg0 / farg1) * (var_f21 - var_f28));
        }
        sp48.x = var_f29;
        var_f20 = arg2->y;
        var_f26 = arg1->y;
        if (farg0 <= lbl_1_rodata_74) {
            var_f27 = var_f26;
        } else if (farg0 >= farg1) {
            var_f27 = var_f20;
        } else {
            var_f27 = var_f26 + ((farg0 / farg1) * (var_f20 - var_f26));
        }
        sp48.y = var_f27;
        var_f19 = arg2->z;
        var_f24 = arg1->z;
        if (farg0 <= lbl_1_rodata_74) {
            var_f25 = var_f24;
        } else if (farg0 >= farg1) {
            var_f25 = var_f19;
        } else {
            var_f25 = var_f24 + ((farg0 / farg1) * (var_f19 - var_f24));
        }
        sp48.z = var_f25;
        sp30.x -= sp48.x;
        sp30.z -= sp48.z;
        sp54.y = (f32) -(lbl_1_rodata_88 * (atan2((f64) sp30.x, -sp30.z) / lbl_1_rodata_78));
        if ((sp3C.y - sp54.y) > lbl_1_rodata_90) {
            sp3C.y -= lbl_1_rodata_94;
        } else if ((sp3C.y - sp54.y) < lbl_1_rodata_98) {
            sp3C.y += lbl_1_rodata_94;
        }
        sp54.x = sp3C.x;
        var_f23 = sp54.y;
        {
            f32 sp14;

            sp14 = sp3C.y;
            if (sp14 == var_f23) {
                var_f18 = var_f23;
            } else {
                var_f18 = (var_f23 + (sp14 * lbl_1_rodata_9C)) / lbl_1_rodata_A0;
            }
        }
        sp54.y = var_f18;
        sp54.z = sp3C.z;
        Hu3DModelPosSetV(arg0, &sp48);
        Hu3DModelRotSetV(arg0, &sp54);
        return;
    }
    {
        f32 sp10;
        f32 spC;
        f32 var_f22;

        Hu3DModelRotGet(arg0, &sp18);
        sp24.y = farg2;
        if ((sp18.y - sp24.y) > lbl_1_rodata_90) {
            sp18.y -= lbl_1_rodata_94;
        } else if ((sp18.y - sp24.y) < lbl_1_rodata_98) {
            sp18.y += lbl_1_rodata_94;
        }
        sp24.x = sp18.x;
        var_f22 = sp24.y;
        spC = sp18.y;
        if (spC == var_f22) {
            sp10 = var_f22;
        } else {
            sp10 = (var_f22 + (spC * lbl_1_rodata_9C)) / lbl_1_rodata_A0;
        }
        sp24.y = sp10;
        sp24.z = sp18.z;
        Hu3DModelRotSetV(arg0, &sp24);
    }
}
