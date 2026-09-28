#define _MATH_H
#include "game/object.h"
#include "game/data.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/sprite.h"
#include "REL/mdminidll/model_animation.h"
#include "string.h"

/* Consumed view of the existing four-pointer readonly initializer.
 * This view does not claim the original source type name or own storage. */
typedef struct MDMinidllAnimationNameTable {
    char *name[4];
} MDMinidllAnimationNameTable;

extern s32 lbl_1_data_474[21];
extern HUPROCESS *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_24;
extern ANIMDATA *lbl_1_bss_6C8[21];
extern const MDMinidllAnimationNameTable lbl_1_rodata_238;

void fn_1_B3E0(void)
{
    MDMinidllAnimationNameTable animationNames;
    s16 var_r31;
    s16 var_r30;

    animationNames = lbl_1_rodata_238;
    var_r31 = 0;
    while (var_r31 < 21) {
        lbl_1_bss_6C8[var_r31] =
            HuSprAnimRead(HuDataSelHeapReadNum(lbl_1_data_474[var_r31], 268435456, HEAP_MODEL));
        var_r31 += 1;
    }

    for (var_r31 = 0; var_r31 < 21; var_r31++) {
        memset(&lbl_1_bss_230[var_r31], 0, sizeof(lbl_1_bss_230[var_r31]));
        if (var_r31 == 0) {
            lbl_1_bss_230[var_r31].model =
                Hu3DModelCreate(HuDataSelHeapReadNum(9830425, 268435456, HEAP_MODEL));
        } else {
            lbl_1_bss_230[var_r31].model = Hu3DModelLink(lbl_1_bss_230[0].model);
        }

        var_r30 = 0;
        while (var_r30 < 4) {
            lbl_1_bss_230[var_r31].animation[var_r30] =
                Hu3DAnimCreate(lbl_1_bss_6C8[0], lbl_1_bss_230[var_r31].model,
                               animationNames.name[var_r30]);
            var_r30 += 1;
        }
        Hu3DModelLayerSet(lbl_1_bss_230[var_r31].model, 1);
        Hu3DModelAttrSet(lbl_1_bss_230[var_r31].model, 1U);
    }

    lbl_1_bss_24 = omAddObjEx(lbl_1_bss_4, 4096, 16U, 16U, -1, NULL);
}
