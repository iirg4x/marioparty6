#define _MATH_H
#include "game/hu3d.h"
#include "game/object.h"

/* Consumed 0x30-byte view; original record and member names are unknown.
 * The repeated vector/state layout is also observed in fn_1_5434. */
typedef struct MDMinTrajectory {
    s16 state;
    HuVecF current;
    HuVecF from;
    HuVecF to;
    f32 elapsed;
    f32 limit;
} MDMinTrajectory;

extern OMOBJ *lbl_1_bss_10;
extern MDMinTrajectory lbl_1_bss_1D0[2];
extern char lbl_1_data_83C[];
extern const f32 lbl_1_rodata_74;

void fn_1_23EC4(s16 index, HuVecF *position, s16 kind);
void fn_1_4790(OMOBJ *object);

void fn_1_4BE0(OMOBJ *arg0)
{
    /* Retail retains this null initialization and its guarded path. */
    OMOBJ *unusedObject = NULL;
    OMOBJ *object = lbl_1_bss_10;
    MDMinTrajectory *first = lbl_1_bss_1D0;
    MDMinTrajectory *second = &lbl_1_bss_1D0[1];

    if (unusedObject != NULL) {
        Hu3DModelObjPosGet(unusedObject->mdlId[0], lbl_1_data_83C,
            &first->current);
        fn_1_23EC4(0, &first->current, 1);
        first->elapsed = lbl_1_rodata_74;
    }
    if (object != NULL) {
        Hu3DModelObjPosGet(object->mdlId[0], lbl_1_data_83C,
            &second->current);
        fn_1_23EC4(1, &second->current, 1);
        second->elapsed = lbl_1_rodata_74;
    }
    if (++arg0->work[0] > 30U) {
        arg0->work[0] = 0;
        arg0->objFunc = fn_1_4790;
    }
}
