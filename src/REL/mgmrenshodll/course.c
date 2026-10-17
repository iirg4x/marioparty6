/* Builds and animates the Endurance Alley course scenery and event effects. */
#include "REL/mgmrenshodll/module_types.h"
#include "dolphin/math.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "dolphin.h"
#include "dolphin/types.h"
#include "dolphin/os.h"
#include "dolphin/mtx.h"
#include "string.h"
#include "humath.h"
#include "messdir_enum.h"
#include "game/hu3d.h"
#include "game/window.h"
#include "game/process.h"
#include "game/flag.h"
#include "game/armem.h"
#include "game/mgdata.h"
#include "game/printfunc.h"
#include "game/saveload.h"
#include "game/esprite.h"
#include "game/thpmain.h"
#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/masu.h"
#include "game/board/player.h"
#define MSM_SE_RENSHO_1254 1254
#define MSM_SE_RENSHO_1255 1255
#define MSM_SE_RENSHO_1256 1256
#define MSM_SE_RENSHO_1258 1258
#define MSM_SE_RENSHO_1259 1259
#define MSM_SE_RENSHO_1260 1260
#define MSM_SE_RENSHO_1257 1257
/* Course scenery loops both its main and shape tracks; event props pause every motion layer. */
#define HU3D_MOTATTR_RENSHO_LOOP (HU3D_MOTATTR_LOOP | HU3D_MOTATTR_SHAPE_LOOP)
#define HU3D_MOTATTR_RENSHO_PAUSE                                                                  \
    (HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE | HU3D_MOTATTR_SHAPE_PAUSE |                    \
     HU3D_MOTATTR_OVL_PAUSE)

s16 fn_1_A00C(void);
s16 fn_1_A05C(s16);
extern u8 MgModePlayNum;
f32 fn_1_4ADC(void);
s16 fn_1_A00C();
extern OMOBJMAN *lbl_1_bss_158;
void fn_1_4F1C(OMOBJ *object);
void fn_1_3C14(void);

typedef struct {
    s32 enabled; /* Stored course-fog enable setting; this module does not apply these settings. */
    f32 start; /* Fog start distance in world units. */
    f32 end; /* Fog end distance in world units. */
    u8 red; /* Current fog red channel from 0 to 255. */
    u8 green; /* Current fog green channel from 0 to 255. */
    u8 blue; /* Current fog blue channel from 0 to 255. */
} MGRenshoFogSettings;

MGRenshoFogSettings lbl_1_bss_104; /* Fog state selected for the current race segment. */
f32 lbl_1_bss_100; /* Pending course-section depth shift in world units. */
OMOBJ *lbl_1_bss_FC; /* Course scenery object updated once per frame. */
/* Section event flags: 0 idle, 1 pending or running, 2 acknowledged; some motions continue in state
 * 2. */
s16 lbl_1_bss_CE[23];
s16 lbl_1_bss_CC; /* Countdown for the nineteenth-section finish event. */
s16 lbl_1_bss_CA; /* Remaining countdown applied during the seventh/eighth-section event. */
s16 lbl_1_bss_C8; /* Delay before the seventh/eighth-section impact cue. */

/* Archive entries for the course sections, including repeated scenery pieces. */
s32 lbl_1_data_1C8[23] = {
    DATANUM(DATA_mgmrensho, 0), DATANUM(DATA_mgmrensho, 1),
    DATANUM(DATA_mgmrensho, 2), DATANUM(DATA_mgmrensho, 3),
    DATANUM(DATA_mgmrensho, 4), DATANUM(DATA_mgmrensho, 5),
    DATANUM(DATA_mgmrensho, 6), DATANUM(DATA_mgmrensho, 6),
    DATANUM(DATA_mgmrensho, 7), DATANUM(DATA_mgmrensho, 8),
    DATANUM(DATA_mgmrensho, 9), DATANUM(DATA_mgmrensho, 10),
    DATANUM(DATA_mgmrensho, 11), DATANUM(DATA_mgmrensho, 12),
    DATANUM(DATA_mgmrensho, 13), DATANUM(DATA_mgmrensho, 14),
    DATANUM(DATA_mgmrensho, 15), DATANUM(DATA_mgmrensho, 16),
    DATANUM(DATA_mgmrensho, 17), DATANUM(DATA_mgmrensho, 18),
    DATANUM(DATA_mgmrensho, 19), DATANUM(DATA_mgmrensho, 20),
    DATANUM(DATA_mgmrensho, 21)
};
/* HSF object names that receive shadow maps on course section four. */
char lbl_1_data_224[] = "f-inst16";
char lbl_1_data_22D[] = "f-grid352";
char lbl_1_data_237[] = "f-grid354";
char lbl_1_data_241[] = "f-grid355";
char lbl_1_data_24B[] = "f-grid356";
char lbl_1_data_255[] = "f-grid357";
char lbl_1_data_25F[] = "f-grid358";
char lbl_1_data_269[] = "f-grid359";
char lbl_1_data_273[] = "f-grid360";
char lbl_1_data_27D[] = "f-grid361";
char lbl_1_data_287[] = "f-grid369";

/* Called during course setup to load a section model at its initial depth. */
void fn_1_2C88(s16 modelIndex, f32 initialDepth, s32 useShadowMap)
{
    lbl_1_bss_FC->mdlId[modelIndex] =
        Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_1C8[modelIndex],
                                             HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_FC->mdlId[modelIndex], 1U);
    Hu3DModelLayerSet(lbl_1_bss_FC->mdlId[modelIndex], 2);
    Hu3DModelPosSet(lbl_1_bss_FC->mdlId[modelIndex], 0.0f,
                    0.0f, initialDepth);
    Hu3DModelAttrSet(lbl_1_bss_FC->mdlId[modelIndex], HU3D_MOTATTR_RENSHO_LOOP);
    if (useShadowMap != 0) {
        if (modelIndex == 3) {
            Hu3DModelShadowMapObjSet(lbl_1_bss_FC->mdlId[modelIndex], lbl_1_data_224);
            Hu3DModelShadowMapObjSet(lbl_1_bss_FC->mdlId[modelIndex], lbl_1_data_22D);
            Hu3DModelShadowMapObjSet(lbl_1_bss_FC->mdlId[modelIndex], lbl_1_data_237);
            Hu3DModelShadowMapObjSet(lbl_1_bss_FC->mdlId[modelIndex], lbl_1_data_241);
            Hu3DModelShadowMapObjSet(lbl_1_bss_FC->mdlId[modelIndex], lbl_1_data_24B);
            Hu3DModelShadowMapObjSet(lbl_1_bss_FC->mdlId[modelIndex], lbl_1_data_255);
            Hu3DModelShadowMapObjSet(lbl_1_bss_FC->mdlId[modelIndex], lbl_1_data_25F);
            Hu3DModelShadowMapObjSet(lbl_1_bss_FC->mdlId[modelIndex], lbl_1_data_269);
            Hu3DModelShadowMapObjSet(lbl_1_bss_FC->mdlId[modelIndex], lbl_1_data_273);
            Hu3DModelShadowMapObjSet(lbl_1_bss_FC->mdlId[modelIndex], lbl_1_data_27D);
            Hu3DModelShadowMapObjSet(lbl_1_bss_FC->mdlId[modelIndex], lbl_1_data_287);
            return;
        }
        Hu3DModelShadowMapSet(lbl_1_bss_FC->mdlId[modelIndex]);
    }
}

/* Called by the overlay setup to create course scenery for the selected race. */
void fn_1_2F9C(void)
{
    OMOBJ *object;
    s32 index;

    lbl_1_bss_104.enabled = 0;
    lbl_1_bss_100 = 0.0f;
    index = 0;
    while (index < 23) {
        lbl_1_bss_CE[index] = 0;
        index++;
    }
    object = omAddObjEx(lbl_1_bss_158, 4096, 23U, 0U, -1, fn_1_4F1C);
    lbl_1_bss_FC = object;
    object->work[0] = object->work[1] = object->work[2] = object->work[3] = 0;
    object->stat |= OM_STAT_MODELPAUSE;
    index = 0;
    while (index < 23) {
        object->mdlId[index] = -1;
        index++;
    }

    if (MgModePlayNum == 0) {
        fn_1_2C88(0, 0.0f, 0);
        fn_1_2C88(1, 0.0f, 1);
        lbl_1_bss_104.enabled = 1;
        lbl_1_bss_104.start = 3000.0f;
        lbl_1_bss_104.end = 15000.0f;
        lbl_1_bss_104.red = lbl_1_bss_104.green = lbl_1_bss_104.blue = 0;
    } else if (MgModePlayNum <= 10) {
        fn_1_2C88(1, 0.0f, 1);
        if (MgModePlayNum > 6) {
            fn_1_2C88(2, -30000.0f, 1);
        }
        lbl_1_bss_104.enabled = 1;
        lbl_1_bss_104.start = 3000.0f;
        lbl_1_bss_104.end = 15000.0f;
        lbl_1_bss_104.red = lbl_1_bss_104.green = lbl_1_bss_104.blue = 0;
    } else if (MgModePlayNum <= 20) {
        if (MgModePlayNum < 15) {
            fn_1_2C88(1, 30000.0f, 1);
        }
        fn_1_2C88(2, 0.0f, 1);
        if (MgModePlayNum > 16) {
            fn_1_2C88(3, -30000.0f, 1);
        }
        lbl_1_bss_104.enabled = 1;
        lbl_1_bss_104.start = 2000.0f;
        lbl_1_bss_104.end = 25000.0f;
        lbl_1_bss_104.red = lbl_1_bss_104.green = lbl_1_bss_104.blue = 0;
    } else if (MgModePlayNum <= 30) {
        if (MgModePlayNum < 25) {
            fn_1_2C88(2, 30000.0f, 1);
        }
        fn_1_2C88(3, 0.0f, 1);
        if (MgModePlayNum > 26) {
            fn_1_2C88(4, -30000.0f, 1);
        }
        if (MgModePlayNum == 30) {
            fn_1_2C88(19, 0.0f, 0);
        }
        lbl_1_bss_104.enabled = 1;
        lbl_1_bss_104.start = 3000.0f;
        lbl_1_bss_104.end = 20000.0f;
        lbl_1_bss_104.red = lbl_1_bss_104.green = lbl_1_bss_104.blue = 0;
    } else if (MgModePlayNum <= 40) {
        if (MgModePlayNum < 35) {
            fn_1_2C88(3, 30000.0f, 1);
        }
        fn_1_2C88(4, 0.0f, 1);
        if (MgModePlayNum > 36) {
            fn_1_2C88(5, -30000.0f, 1);
        }
        if (MgModePlayNum != 40) {
            fn_1_2C88(19, 0.0f, 0);
        } else {
            fn_1_2C88(20, 0.0f, 0);
        }
        lbl_1_bss_104.enabled = 1;
        lbl_1_bss_104.start = 3000.0f;
        lbl_1_bss_104.end = 30000.0f;
        lbl_1_bss_104.red = 160;
        lbl_1_bss_104.green = 64;
        lbl_1_bss_104.blue = 0;
    } else if (MgModePlayNum <= 50) {
        if (MgModePlayNum < 43) {
            fn_1_2C88(4, 30000.0f, 1);
        }
        fn_1_2C88(5, 0.0f, 1);
        fn_1_2C88(6, 0.0f, 1);
        fn_1_2C88(7, -2650.0f, 1);
        Hu3DModelAttrSet(object->mdlId[6], HU3D_MOTATTR_RENSHO_PAUSE);
        Hu3DMotionTimeSet(object->mdlId[6], 0.0f);
        Hu3DModelAttrSet(object->mdlId[7], HU3D_MOTATTR_RENSHO_PAUSE);
        Hu3DMotionTimeSet(object->mdlId[7], 0.0f);
        if (MgModePlayNum > 44) {
            fn_1_2C88(8, -30000.0f, 1);
            fn_1_2C88(9, -30000.0f, 0);
        }
        fn_1_2C88(20, 0.0f, 0);
    } else if (MgModePlayNum <= 60) {
        if (MgModePlayNum < 55) {
            fn_1_2C88(5, 30000.0f, 1);
        }
        fn_1_2C88(8, 0.0f, 1);
        fn_1_2C88(9, 0.0f, 0);
        if (MgModePlayNum > 56) {
            fn_1_2C88(10, -30000.0f, 1);
            fn_1_2C88(11, -30000.0f, 0);
        }
        fn_1_2C88(20, 0.0f, 0);
    } else if (MgModePlayNum <= 70) {
        if (MgModePlayNum < 65) {
            fn_1_2C88(8, 30000.0f, 1);
            fn_1_2C88(9, 30000.0f, 0);
        }
        fn_1_2C88(10, 0.0f, 1);
        fn_1_2C88(11, 0.0f, 0);
        fn_1_2C88(12, 0.0f, 0);
        if (MgModePlayNum == 70) {
            lbl_1_bss_CE[12] = 2;
        }
        if (MgModePlayNum > 66) {
            fn_1_2C88(13, -30000.0f, 0);
            fn_1_2C88(15, -30000.0f, 1);
        }
        if (MgModePlayNum != 70) {
            fn_1_2C88(20, 0.0f, 0);
        }
        if (MgModePlayNum > 61) {
            lbl_1_bss_CE[10] = 2;
        }
    } else if (MgModePlayNum <= 80) {
        if (MgModePlayNum < 75) {
            fn_1_2C88(10, 30000.0f, 1);
            fn_1_2C88(11, 30000.0f, 0);
            fn_1_2C88(12, 30000.0f, 0);
            lbl_1_bss_CE[12] = 2;
        }
        fn_1_2C88(13, 0.0f, 1);
        fn_1_2C88(14, 0.0f, 0);
        fn_1_2C88(15, 0.0f, 1);
        if (MgModePlayNum > 76) {
            fn_1_2C88(16, -30000.0f, 1);
            lbl_1_bss_CE[16] = 0;
        }
        if (MgModePlayNum == 80) {
            fn_1_2C88(21, 0.0f, 0);
        }
        if (MgModePlayNum > 74) {
            lbl_1_bss_CE[14] = 2;
        }
    } else if (MgModePlayNum <= 90) {
        if (MgModePlayNum < 85) {
            fn_1_2C88(15, 30000.0f, 1);
        }
        fn_1_2C88(16, 0.0f, 1);
        if (MgModePlayNum > 82) {
            lbl_1_bss_CE[16] = 2;
        }
        if (MgModePlayNum > 86) {
            fn_1_2C88(17, -30000.0f, 0);
            fn_1_2C88(18, -30000.0f, 1);
        }
        fn_1_2C88(21, 0.0f, 0);
    } else if (MgModePlayNum <= 100) {
        if (MgModePlayNum < 95) {
            fn_1_2C88(16, 30000.0f, 1);
            fn_1_2C88(18, 0.0f, 1);
        }
        fn_1_2C88(17, 0.0f, 1);
        if (MgModePlayNum > 90) {
            lbl_1_bss_CE[18] = 2;
        }
        fn_1_2C88(21, 0.0f, 0);
        lbl_1_bss_CE[21] = 2;
        fn_1_2C88(22, 0.0f, 0);
        Hu3DModelLayerSet(object->mdlId[22], 4);
        Hu3DModelPosSet(object->mdlId[22], 0.0f,
                        100.0f, 0.0f);
    }
    fn_1_3C14();
}

/* Called after course setup to place the loaded sections around the selected race. */
void fn_1_3C14(void)
{
    OMOBJ *object;
    s16 column;
    f32 positionOffset;

    object = lbl_1_bss_FC;
    column = (MgModePlayNum - 1) % 10;
    if (column < 0) {
        column = 0;
    }
    positionOffset = 3000.0f * (f32)column;
    if (MgModePlayNum != 0) {
        if (MgModePlayNum <= 10) {
            if (object->mdlId[1] >= 0) {
                Hu3DModelPosSet(object->mdlId[1], 0.0f, 0.0f,
                                positionOffset);
            }
            if (object->mdlId[2] >= 0) {
                Hu3DModelPosSet(object->mdlId[2], 0.0f, 0.0f,
                                -30000.0f + positionOffset);
            }
        } else if (MgModePlayNum <= 20) {
            if (object->mdlId[1] >= 0) {
                Hu3DModelPosSet(object->mdlId[1], 0.0f, 0.0f,
                                30000.0f + positionOffset);
            }
            if (object->mdlId[2] >= 0) {
                Hu3DModelPosSet(object->mdlId[2], 0.0f, 0.0f,
                                positionOffset);
            }
            if (object->mdlId[3] >= 0) {
                Hu3DModelPosSet(object->mdlId[3], 0.0f, 0.0f,
                                -30000.0f + positionOffset);
            }
        } else if (MgModePlayNum <= 30) {
            if (object->mdlId[2] >= 0) {
                Hu3DModelPosSet(object->mdlId[2], 0.0f, 0.0f,
                                30000.0f + positionOffset);
            }
            if (object->mdlId[3] >= 0) {
                Hu3DModelPosSet(object->mdlId[3], 0.0f, 0.0f,
                                positionOffset);
            }
            if (object->mdlId[4] >= 0) {
                Hu3DModelPosSet(object->mdlId[4], 0.0f, 0.0f,
                                -30000.0f + positionOffset);
            }
        } else if (MgModePlayNum <= 40) {
            if (object->mdlId[3] >= 0) {
                Hu3DModelPosSet(object->mdlId[3], 0.0f, 0.0f,
                                30000.0f + positionOffset);
            }
            if (object->mdlId[4] >= 0) {
                Hu3DModelPosSet(object->mdlId[4], 0.0f, 0.0f,
                                positionOffset);
            }
            if (object->mdlId[5] >= 0) {
                Hu3DModelPosSet(object->mdlId[5], 0.0f, 0.0f,
                                -30000.0f + positionOffset);
            }
        } else if (MgModePlayNum <= 50) {
            if (object->mdlId[4] >= 0) {
                Hu3DModelPosSet(object->mdlId[4], 0.0f, 0.0f,
                                30000.0f + positionOffset);
            }
            if (object->mdlId[5] >= 0) {
                Hu3DModelPosSet(object->mdlId[5], 0.0f, 0.0f,
                                positionOffset);
            }
            if (object->mdlId[6] >= 0) {
                Hu3DModelPosSet(object->mdlId[6], 0.0f, 0.0f,
                                positionOffset);
            }
            if (object->mdlId[7] >= 0) {
                Hu3DModelPosSet(object->mdlId[7], 0.0f, 0.0f,
                                -2650.0f + positionOffset);
            }
            if (object->mdlId[8] >= 0) {
                Hu3DModelPosSet(object->mdlId[8], 0.0f, 0.0f,
                                -30000.0f + positionOffset);
            }
            if (object->mdlId[9] >= 0) {
                Hu3DModelPosSet(object->mdlId[9], 0.0f, 0.0f,
                                -30000.0f + positionOffset);
            }
        } else if (MgModePlayNum <= 60) {
            if (object->mdlId[5] >= 0) {
                Hu3DModelPosSet(object->mdlId[5], 0.0f, 0.0f,
                                30000.0f + positionOffset);
            }
            if (object->mdlId[8] >= 0) {
                Hu3DModelPosSet(object->mdlId[8], 0.0f, 0.0f,
                                positionOffset);
            }
            if (object->mdlId[9] >= 0) {
                Hu3DModelPosSet(object->mdlId[9], 0.0f, 0.0f,
                                positionOffset);
            }
            if (object->mdlId[10] >= 0) {
                Hu3DModelPosSet(object->mdlId[10], 0.0f, 0.0f,
                                -30000.0f + positionOffset);
            }
            if (object->mdlId[11] >= 0) {
                Hu3DModelPosSet(object->mdlId[11], 0.0f, 0.0f,
                                -30000.0f + positionOffset);
            }
            if (object->mdlId[12] >= 0) {
                Hu3DModelPosSet(object->mdlId[12], 0.0f, 0.0f,
                                -30000.0f + positionOffset);
            }
        } else if (MgModePlayNum <= 70) {
            if (object->mdlId[8] >= 0) {
                Hu3DModelPosSet(object->mdlId[8], 0.0f, 0.0f,
                                30000.0f + positionOffset);
            }
            if (object->mdlId[9] >= 0) {
                Hu3DModelPosSet(object->mdlId[9], 0.0f, 0.0f,
                                30000.0f + positionOffset);
            }
            if (object->mdlId[10] >= 0) {
                Hu3DModelPosSet(object->mdlId[10], 0.0f, 0.0f,
                                positionOffset);
            }
            if (object->mdlId[11] >= 0) {
                Hu3DModelPosSet(object->mdlId[11], 0.0f, 0.0f,
                                positionOffset);
            }
            if (object->mdlId[12] >= 0) {
                Hu3DModelPosSet(object->mdlId[12], 0.0f, 0.0f,
                                positionOffset);
            }
            if (object->mdlId[13] >= 0) {
                Hu3DModelPosSet(object->mdlId[13], 0.0f, 0.0f,
                                -30000.0f + positionOffset);
            }
            if (object->mdlId[14] >= 0) {
                Hu3DModelPosSet(object->mdlId[14], 0.0f, 0.0f,
                                -30000.0f + positionOffset);
            }
            if (object->mdlId[15] >= 0) {
                Hu3DModelPosSet(object->mdlId[15], 0.0f, 0.0f,
                                -30000.0f + positionOffset);
            }
        } else if (MgModePlayNum <= 80) {
            if (object->mdlId[10] >= 0) {
                Hu3DModelPosSet(object->mdlId[10], 0.0f, 0.0f,
                                30000.0f + positionOffset);
            }
            if (object->mdlId[11] >= 0) {
                Hu3DModelPosSet(object->mdlId[11], 0.0f, 0.0f,
                                30000.0f + positionOffset);
            }
            if (object->mdlId[12] >= 0) {
                Hu3DModelPosSet(object->mdlId[12], 0.0f, 0.0f,
                                30000.0f + positionOffset);
            }
            if (object->mdlId[13] >= 0) {
                Hu3DModelPosSet(object->mdlId[13], 0.0f, 0.0f,
                                positionOffset);
            }
            if (object->mdlId[14] >= 0) {
                Hu3DModelPosSet(object->mdlId[14], 0.0f, 0.0f,
                                positionOffset);
            }
            if (object->mdlId[15] >= 0) {
                Hu3DModelPosSet(object->mdlId[15], 0.0f, 0.0f,
                                positionOffset);
            }
            if (object->mdlId[16] >= 0) {
                Hu3DModelPosSet(object->mdlId[16], 0.0f, 0.0f,
                                -30000.0f + positionOffset);
            }
        } else if (MgModePlayNum <= 90) {
            if (object->mdlId[13] >= 0) {
                Hu3DModelPosSet(object->mdlId[13], 0.0f, 0.0f,
                                30000.0f + positionOffset);
            }
            if (object->mdlId[14] >= 0) {
                Hu3DModelPosSet(object->mdlId[14], 0.0f, 0.0f,
                                30000.0f + positionOffset);
            }
            if (object->mdlId[15] >= 0) {
                Hu3DModelPosSet(object->mdlId[15], 0.0f, 0.0f,
                                30000.0f + positionOffset);
            }
            if (object->mdlId[16] >= 0) {
                Hu3DModelPosSet(object->mdlId[16], 0.0f, 0.0f,
                                positionOffset);
            }
            if (object->mdlId[17] >= 0) {
                Hu3DModelPosSet(object->mdlId[17], 0.0f, 0.0f,
                                -30000.0f + positionOffset);
            }
            if (object->mdlId[18] >= 0) {
                Hu3DModelPosSet(object->mdlId[18], 0.0f, 0.0f,
                                -30000.0f + positionOffset);
            }
        } else if (MgModePlayNum <= 100) {
            if (object->mdlId[16] >= 0) {
                Hu3DModelPosSet(object->mdlId[16], 0.0f, 0.0f,
                                30000.0f + positionOffset);
            }
            if (object->mdlId[17] >= 0) {
                Hu3DModelPosSet(object->mdlId[17], 0.0f, 0.0f,
                                positionOffset);
            }
            if (object->mdlId[18] >= 0) {
                Hu3DModelPosSet(object->mdlId[18], 0.0f, 0.0f,
                                positionOffset);
            }
        }
    }
}

/* Called by the scenery update for each active section to apply its pending depth shift. */
void fn_1_4984(s16 modelIndex, f32 zOffset)
{
    Point3d position;

    if (lbl_1_bss_FC->mdlId[modelIndex] >= 0) {
        Hu3DModelPosGet(lbl_1_bss_FC->mdlId[modelIndex], &position);
        position.z += zOffset;
        Hu3DModelPosSetV(lbl_1_bss_FC->mdlId[modelIndex], &position);
    }
}

/* Used by the runner and scenery code to get this race's depth in its ten-race group. */
f32 fn_1_4A2C(void)
{
    s16 playCount;

    playCount = MgModePlayNum % 10;
    if (playCount < 0) {
        playCount = 0;
    }
    if (playCount == 0) {
        playCount = 10;
    }
    if (MgModePlayNum == 0) {
        playCount = 0;
    }
    return 3000.0f * (f32)playCount;
}

/* Used by course and runner updates to read the loaded section's current depth. */
f32 fn_1_4ADC(void)
{
    s16 modelIndex;
    Point3d position;

    if (MgModePlayNum <= 10) {
        modelIndex = 1;
    } else if (MgModePlayNum <= 20) {
        modelIndex = 2;
    } else if (MgModePlayNum <= 30) {
        modelIndex = 3;
    } else if (MgModePlayNum <= 40) {
        modelIndex = 4;
    } else if (MgModePlayNum <= 50) {
        modelIndex = 5;
    } else if (MgModePlayNum <= 60) {
        modelIndex = 8;
    } else if (MgModePlayNum <= 70) {
        modelIndex = 10;
    } else if (MgModePlayNum <= 80) {
        modelIndex = 13;
    } else if (MgModePlayNum <= 90) {
        modelIndex = 16;
    } else if (MgModePlayNum <= 100) {
        modelIndex = 17;
    }
    Hu3DModelPosGet(lbl_1_bss_FC->mdlId[modelIndex], &position);
    return position.z;
}

/* Supplies the fixed depth of one race segment. */
f32 fn_1_4C3C(void) {
    return 3000.0f;
}

/* Called by the turn-end script at race 46 to start section seven's finish event. */
void fn_1_4C4C(void) {
    lbl_1_bss_CE[6] = 1;
    lbl_1_bss_C8 = 50;
}

/* Called with section seven's event to set the shared 90-frame countdown. */
void fn_1_4C70(void) {
    lbl_1_bss_CA = 90;
}

/* Called by the turn-end script at race 47 to start section eight's finish event. */
void fn_1_4C84(void) {
    lbl_1_bss_CE[7] = 1;
    lbl_1_bss_C8 = 40;
}

/* Called with section eight's event to restart the shared 90-frame countdown. */
void fn_1_4CA8(void) {
    lbl_1_bss_CA = 90;
}

/* Called by the turn-end script at race 61 to start section eleven's event. */
void fn_1_4CBC(void) {
    lbl_1_bss_CE[10] = 1;
    HuAudFXPlay(MSM_SE_RENSHO_1254);
}

/* Called by the turn-end script at race 74 to start section fifteen's event. */
void fn_1_4CF0(void) {
    lbl_1_bss_CE[14] = 1;
    HuAudFXPlay(MSM_SE_RENSHO_1255);
}

/* Called by the turn-end script at race 69 to start section thirteen's event. */
void fn_1_4D24(void) {
    lbl_1_bss_CE[12] = 1;
    HuAudFXPlay(MSM_SE_RENSHO_1255);
}

/* Called by the turn-end script at race 82 to start section seventeen's event. */
void fn_1_4D58(void) {
    lbl_1_bss_CE[16] = 1;
    HuAudFXPlay(MSM_SE_RENSHO_1258);
}

/* Called by the turn-end script at race 90 to start section nineteen's countdown. */
void fn_1_4D8C(void) {
    lbl_1_bss_CE[18] = 1;
    lbl_1_bss_CC = 50;
}

/* Called when section nineteen ends to start section twenty-two's transition at motion frame
 * 301. */
void fn_1_4DB0(void) {
    lbl_1_bss_CE[21] = 1;
    Hu3DMotionTimeSet(lbl_1_bss_FC->mdlId[21], 301.0f);
}

/* Polled after round 90 until section twenty-two switches from its transition to its closing
 * loop. */
s16 fn_1_4E00(void) {
    if (lbl_1_bss_CE[21] >= 2) {
        return 1;
    }
    return 0;
}

/* Called during the final race sequence to start the last course event. */
void fn_1_4E24(void) {
    lbl_1_bss_CE[22] = 1;
    HuAudFXPlay(MSM_SE_RENSHO_1259);
}

/* Turn-end and completion scripts poll for the absence of event flag 1;
 * motions in state 2 may continue. */
s16 fn_1_4E58(void) {
    s16 eventIndex = 0;
    while (eventIndex < 23) {
        if (lbl_1_bss_CE[eventIndex] == 1) {
            return 0;
        }
        eventIndex++;
    }
    return 1;
}

/* Called at hopping-scenery phase frame 50 to start section sixteen's event,
 * play its cue, and rumble a human player. */
void fn_1_4EB0(void) {
    lbl_1_bss_CE[15] = 1;
    HuAudFXPlay(MSM_SE_RENSHO_1256);
    if (fn_1_A05C(fn_1_A00C()) == 0) {
        omVibrate(fn_1_A00C(), 20, 4, 4);
    }
}

/* Called by runner movement updates to queue a depth shift for the next scenery update. */
void fn_1_4F0C(float zOffset)
{
    lbl_1_bss_100 = zOffset;
}

/* Registered as the course object's callback; advances animations and race events each frame. */
void fn_1_4F1C(OMOBJ *object) {
    f32 frameOrCourseProgress;
    s16 playerNo;
    if (object->mdlId[6] >= 0)
    {
        if (lbl_1_bss_CE[6] == 0)
        {
            if ((Hu3DData[object->mdlId[6]].motWork.speed + Hu3DMotionTimeGet(object->mdlId[6])) >=
                Hu3DMotionMaxTimeGet(object->mdlId[6])) {
                Hu3DModelAttrSet(object->mdlId[6], HU3D_MOTATTR_RENSHO_PAUSE);
                Hu3DMotionTimeSet(object->mdlId[6], 0.0f);
            }
        }
        else
        {
            lbl_1_bss_C8 -= 1;
            if (lbl_1_bss_C8 <= 0)
            {
                if (lbl_1_bss_C8 == 0)
                {
                    HuAudFXPlay(MSM_SE_RENSHO_1260);
                    playerNo = fn_1_A00C();
                    if (fn_1_A05C(playerNo) == 0)
                    {
                        omVibrate(playerNo, lbl_1_bss_CA, 4, 4);
                    }
                }
                lbl_1_bss_C8 = 0;
                Hu3DModelAttrReset(object->mdlId[6], HU3D_MOTATTR_RENSHO_PAUSE);
                lbl_1_bss_CE[6] = 2;
                lbl_1_bss_CA -= 1;
                if (lbl_1_bss_CA <= 0)
                {
                    lbl_1_bss_CA = 0;
                    lbl_1_bss_CE[6] = 0;
                }
            }
        }
    }
    if (object->mdlId[7] >= 0)
    {
        if (! (lbl_1_bss_CE[7]))
        {
            if ((Hu3DMotionTimeGet(object->mdlId[7]) + Hu3DData[object->mdlId[7]].motWork.speed) >=
                Hu3DMotionMaxTimeGet(object->mdlId[7])) {
                Hu3DModelAttrSet(object->mdlId[7], HU3D_MOTATTR_RENSHO_PAUSE);
                Hu3DMotionTimeSet(object->mdlId[7], 0.0f);
            }
        }
        else
        {
            lbl_1_bss_C8 -= 1;
            if (lbl_1_bss_C8 <= 0)
            {
                if (lbl_1_bss_C8 == 0)
                {
                    HuAudFXPlay(MSM_SE_RENSHO_1260);
                    playerNo = fn_1_A00C();
                    if (fn_1_A05C(playerNo) == 0)
                    {
                        omVibrate(playerNo, lbl_1_bss_CA, 4, 4);
                    }
                }
                lbl_1_bss_C8 = 0;
                Hu3DModelAttrReset(object->mdlId[7], HU3D_MOTATTR_RENSHO_PAUSE);
                lbl_1_bss_CE[7] = 2;
                lbl_1_bss_CA -= 1;
                if (lbl_1_bss_CA <= 0)
                {
                    lbl_1_bss_CA = 0;
                    lbl_1_bss_CE[7] = 0;
                }
            }
        }
    }
    if (object->mdlId[10] >= 0)
    {
        if (lbl_1_bss_CE[10] == 0)
        {
            Hu3DModelAttrSet(object->mdlId[10], HU3D_MOTATTR_RENSHO_PAUSE);
            Hu3DMotionTimeSet(object->mdlId[10], 0.0f);
        }
        else
        {
            if (lbl_1_bss_CE[10] == 1)
            {
                Hu3DModelAttrReset(object->mdlId[10], HU3D_MOTATTR_RENSHO_PAUSE);
                if ((Hu3DMotionTimeGet(object->mdlId[10]) +
                     Hu3DData[object->mdlId[10]].motWork.speed) >=
                    Hu3DMotionMaxTimeGet(object->mdlId[10])) {
                    lbl_1_bss_CE[10] = 2;
                    Hu3DModelAttrSet(object->mdlId[10], HU3D_MOTATTR_RENSHO_PAUSE);
                    Hu3DMotionTimeSet(object->mdlId[10], Hu3DMotionMaxTimeGet(object->mdlId[10]));
                }
            }
            else
            {
                Hu3DModelAttrSet(object->mdlId[10], HU3D_MOTATTR_RENSHO_PAUSE);
                Hu3DMotionTimeSet(object->mdlId[10], Hu3DMotionMaxTimeGet(object->mdlId[10]));
            }
        }
    }
    if (object->mdlId[12] >= 0)
    {
        if (lbl_1_bss_CE[12] == 0)
        {
            Hu3DModelAttrSet(object->mdlId[12], HU3D_MOTATTR_RENSHO_PAUSE);
            Hu3DMotionTimeSet(object->mdlId[12], 0.0f);
        }
        else
        {
            if (lbl_1_bss_CE[12] == 1)
            {
                Hu3DModelAttrReset(object->mdlId[12], HU3D_MOTATTR_RENSHO_PAUSE);
                if ((Hu3DMotionTimeGet(object->mdlId[12]) +
                     Hu3DData[object->mdlId[12]].motWork.speed) >=
                    Hu3DMotionMaxTimeGet(object->mdlId[12])) {
                    lbl_1_bss_CE[12] = 2;
                    Hu3DModelAttrSet(object->mdlId[12], HU3D_MOTATTR_RENSHO_PAUSE);
                    Hu3DMotionTimeSet(object->mdlId[12], Hu3DMotionMaxTimeGet(object->mdlId[12]));
                    playerNo = fn_1_A00C();
                    if (fn_1_A05C(playerNo) == 0)
                    {
                        omVibrate(playerNo, 20, 20, 0);
                    }
                }
            }
            else
            {
                Hu3DModelAttrSet(object->mdlId[12], HU3D_MOTATTR_RENSHO_PAUSE);
                Hu3DMotionTimeSet(object->mdlId[12], Hu3DMotionMaxTimeGet(object->mdlId[12]));
                Hu3DModelShadowMapSet(lbl_1_bss_FC->mdlId[12]);
            }
        }
    }
    if (object->mdlId[14] >= 0)
    {
        if (lbl_1_bss_CE[14] == 0)
        {
            Hu3DModelAttrSet(object->mdlId[14], HU3D_MOTATTR_RENSHO_PAUSE);
            Hu3DMotionTimeSet(object->mdlId[14], 0.0f);
        }
        else
        {
            if (lbl_1_bss_CE[14] == 1)
            {
                Hu3DModelAttrReset(object->mdlId[14], HU3D_MOTATTR_RENSHO_PAUSE);
                if ((Hu3DMotionTimeGet(object->mdlId[14]) +
                     Hu3DData[object->mdlId[14]].motWork.speed) >=
                    Hu3DMotionMaxTimeGet(object->mdlId[14])) {
                    lbl_1_bss_CE[14] = 2;
                    Hu3DModelAttrSet(object->mdlId[14], HU3D_MOTATTR_RENSHO_PAUSE);
                    Hu3DMotionTimeSet(object->mdlId[14], Hu3DMotionMaxTimeGet(object->mdlId[14]));
                    playerNo = fn_1_A00C();
                    if (fn_1_A05C(playerNo) == 0)
                    {
                        omVibrate(playerNo, 20, 20, 0);
                    }
                }
            }
            else
            {
                Hu3DModelAttrSet(object->mdlId[14], HU3D_MOTATTR_RENSHO_PAUSE);
                Hu3DMotionTimeSet(object->mdlId[14], Hu3DMotionMaxTimeGet(object->mdlId[14]));
                Hu3DModelShadowMapSet(lbl_1_bss_FC->mdlId[14]);
            }
        }
    }
    if (object->mdlId[15] >= 0)
    {
        if (lbl_1_bss_CE[15] == 0)
        {
            Hu3DModelAttrSet(object->mdlId[15], HU3D_MOTATTR_RENSHO_PAUSE);
            Hu3DMotionTimeSet(object->mdlId[15], 0.0f);
        }
        else
        {
            lbl_1_bss_CE[15] = 2;
            Hu3DModelAttrReset(object->mdlId[15], HU3D_MOTATTR_RENSHO_PAUSE);
            if ((Hu3DMotionTimeGet(object->mdlId[15]) +
                 Hu3DData[object->mdlId[15]].motWork.speed) >=
                Hu3DMotionMaxTimeGet(object->mdlId[15])) {
                lbl_1_bss_CE[15] = 0;
                Hu3DModelAttrSet(object->mdlId[15], HU3D_MOTATTR_RENSHO_PAUSE);
                Hu3DMotionTimeSet(object->mdlId[15], 0.0f);
            }
        }
    }
    if (object->mdlId[16] >= 0)
    {
        if (lbl_1_bss_CE[16] == 0)
        {
            Hu3DModelAttrSet(object->mdlId[16], HU3D_MOTATTR_RENSHO_PAUSE);
            Hu3DMotionTimeSet(object->mdlId[16], 0.0f);
        }
        else
        {
            if (lbl_1_bss_CE[16] == 1)
            {
                Hu3DModelAttrReset(object->mdlId[16], HU3D_MOTATTR_RENSHO_PAUSE);
                if ((Hu3DMotionTimeGet(object->mdlId[16]) +
                     Hu3DData[object->mdlId[16]].motWork.speed) >=
                    Hu3DMotionMaxTimeGet(object->mdlId[16])) {
                    lbl_1_bss_CE[16] = 2;
                    Hu3DModelAttrSet(object->mdlId[16], HU3D_MOTATTR_RENSHO_PAUSE);
                    Hu3DMotionTimeSet(object->mdlId[16], Hu3DMotionMaxTimeGet(object->mdlId[16]));
                }
            }
            else
            {
                Hu3DModelAttrSet(object->mdlId[16], HU3D_MOTATTR_RENSHO_PAUSE);
                Hu3DMotionTimeSet(object->mdlId[16], Hu3DMotionMaxTimeGet(object->mdlId[16]));
            }
        }
    }
    if (object->mdlId[18] >= 0)
    {
        if (lbl_1_bss_CE[18] == 0)
        {
            Hu3DModelAttrSet(object->mdlId[18], HU3D_MOTATTR_RENSHO_PAUSE);
            Hu3DMotionTimeSet(object->mdlId[18], 0.0f);
        }
        else
        {
            if (lbl_1_bss_CE[18] == 1)
            {
                lbl_1_bss_CC -= 1;
                if (lbl_1_bss_CC <= 0)
                {
                    if (lbl_1_bss_CC == 0)
                    {
                        HuAudFXPlay(MSM_SE_RENSHO_1257);
                    }
                    lbl_1_bss_CC = - 1;
                    Hu3DModelAttrReset(object->mdlId[18], HU3D_MOTATTR_RENSHO_PAUSE);
                    if ((Hu3DMotionTimeGet(object->mdlId[18]) +
                         Hu3DData[object->mdlId[18]].motWork.speed) >=
                        Hu3DMotionMaxTimeGet(object->mdlId[18])) {
                        lbl_1_bss_CE[18] = 2;
                        Hu3DModelAttrSet(object->mdlId[18], HU3D_MOTATTR_RENSHO_PAUSE);
                        Hu3DMotionTimeSet(object->mdlId[18],
                                          Hu3DMotionMaxTimeGet(object->mdlId[18]));
                        fn_1_4DB0();
                    }
                    frameOrCourseProgress = Hu3DMotionTimeGet(object->mdlId[18]);
                    if ((100.0f <= frameOrCourseProgress) && (frameOrCourseProgress < 101.0f))
                    {
                        playerNo = fn_1_A00C();
                        if (fn_1_A05C(playerNo) == 0)
                        {
                            omVibrate(playerNo, 20, 20, 0);
                        }
                    }
                }
            }
            else
            {
                Hu3DModelAttrSet(object->mdlId[18], HU3D_MOTATTR_RENSHO_PAUSE);
                Hu3DMotionTimeSet(object->mdlId[18], Hu3DMotionMaxTimeGet(object->mdlId[18]));
            }
        }
    }
    if (object->mdlId[21] >= 0)
    {
        if (lbl_1_bss_CE[21] == 0)
        {
            if ((Hu3DMotionTimeGet(object->mdlId[21]) +
                 Hu3DData[object->mdlId[21]].motWork.speed) >= 300.0f) {
                Hu3DMotionTimeSet(object->mdlId[21], 0.0f);
            }
        }
        else
        {
            if (lbl_1_bss_CE[21] == 1)
            {
                if ((Hu3DMotionTimeGet(object->mdlId[21]) +
                     Hu3DData[object->mdlId[21]].motWork.speed) >= 600.0f) {
                    lbl_1_bss_CE[21] = 2;
                }
            }
            else
            {
                if ((Hu3DMotionTimeGet(object->mdlId[21]) +
                     Hu3DData[object->mdlId[21]].motWork.speed) >=
                    Hu3DMotionMaxTimeGet(object->mdlId[21])) {
                    Hu3DMotionTimeSet(object->mdlId[21], 601.0f);
                }
                if (Hu3DMotionTimeGet(object->mdlId[21]) < 601.0f)
                {
                    Hu3DMotionTimeSet(object->mdlId[21], 601.0f);
                }
            }
        }
    }
    if (object->mdlId[22] >= 0)
    {
        if (lbl_1_bss_CE[22] == 0)
        {
            Hu3DModelAttrSet(object->mdlId[22], 1U);
            Hu3DModelAttrSet(object->mdlId[22], HU3D_MOTATTR_RENSHO_PAUSE);
            Hu3DMotionTimeSet(object->mdlId[22], 0.0f);
        }
        else
        {
            if (lbl_1_bss_CE[22] == 1)
            {
                Hu3DModelAttrReset(object->mdlId[22], 1U);
                Hu3DModelAttrReset(object->mdlId[22], HU3D_MOTATTR_RENSHO_PAUSE);
                if ((Hu3DMotionTimeGet(object->mdlId[22]) +
                     Hu3DData[object->mdlId[22]].motWork.speed) >= 280.0f) {
                    lbl_1_bss_CE[22] = 2;
                }
                frameOrCourseProgress = Hu3DMotionTimeGet(object->mdlId[22]);
                if ((80.0f <= frameOrCourseProgress) && (frameOrCourseProgress < 275.0f))
                {
                    playerNo = fn_1_A00C();
                    if (fn_1_A05C(playerNo) == 0)
                    {
                        omVibrate(playerNo, 20, 4, 4);
                    }
                }
            }
            else
            {
                Hu3DModelAttrReset(object->mdlId[22], 1U);
                Hu3DModelAttrReset(object->mdlId[22], HU3D_MOTATTR_RENSHO_PAUSE);
                if ((Hu3DMotionTimeGet(object->mdlId[22]) +
                     Hu3DData[object->mdlId[22]].motWork.speed) >=
                    Hu3DMotionMaxTimeGet(object->mdlId[22])) {
                    Hu3DMotionTimeSet(object->mdlId[22], 281.0f);
                }
            }
        }
    }
    if (object->mdlId[0] >= 0)
    {
        if ((Hu3DMotionTimeGet(object->mdlId[0]) + Hu3DData[object->mdlId[0]].motWork.speed) >=
            Hu3DMotionMaxTimeGet(object->mdlId[0])) {
            Hu3DModelAttrSet(object->mdlId[0], HU3D_MOTATTR_RENSHO_PAUSE);
        }
    }
    if (MgModePlayNum <= 10U)
    {
        fn_1_4984(1, lbl_1_bss_100);
        fn_1_4984(2, lbl_1_bss_100);
    }
    else if (MgModePlayNum <= 20U)
    {
        fn_1_4984(1, lbl_1_bss_100);
        fn_1_4984(2, lbl_1_bss_100);
        fn_1_4984(3, lbl_1_bss_100);
    }
    else if (MgModePlayNum <= 30U)
    {
        fn_1_4984(2, lbl_1_bss_100);
        fn_1_4984(3, lbl_1_bss_100);
        fn_1_4984(4, lbl_1_bss_100);
    }
    else if (MgModePlayNum <= 40U)
    {
        fn_1_4984(3, lbl_1_bss_100);
        fn_1_4984(4, lbl_1_bss_100);
        fn_1_4984(5, lbl_1_bss_100);
    }
    else if (MgModePlayNum <= 50U)
    {
        fn_1_4984(4, lbl_1_bss_100);
        fn_1_4984(5, lbl_1_bss_100);
        fn_1_4984(6, lbl_1_bss_100);
        fn_1_4984(7, lbl_1_bss_100);
        fn_1_4984(8, lbl_1_bss_100);
        fn_1_4984(9, lbl_1_bss_100);
    }
    else if (MgModePlayNum <= 60U)
    {
        fn_1_4984(5, lbl_1_bss_100);
        fn_1_4984(8, lbl_1_bss_100);
        fn_1_4984(9, lbl_1_bss_100);
        fn_1_4984(10, lbl_1_bss_100);
        fn_1_4984(11, lbl_1_bss_100);
        fn_1_4984(12, lbl_1_bss_100);
    }
    else if (MgModePlayNum <= 70U)
    {
        fn_1_4984(8, lbl_1_bss_100);
        fn_1_4984(9, lbl_1_bss_100);
        fn_1_4984(10, lbl_1_bss_100);
        fn_1_4984(11, lbl_1_bss_100);
        fn_1_4984(12, lbl_1_bss_100);
        fn_1_4984(13, lbl_1_bss_100);
        fn_1_4984(14, lbl_1_bss_100);
        fn_1_4984(15, lbl_1_bss_100);
    }
    else if (MgModePlayNum <= 80U)
    {
        fn_1_4984(10, lbl_1_bss_100);
        fn_1_4984(11, lbl_1_bss_100);
        fn_1_4984(12, lbl_1_bss_100);
        fn_1_4984(13, lbl_1_bss_100);
        fn_1_4984(14, lbl_1_bss_100);
        fn_1_4984(15, lbl_1_bss_100);
        fn_1_4984(16, lbl_1_bss_100);
    }
    else if (MgModePlayNum <= 90U)
    {
        fn_1_4984(13, lbl_1_bss_100);
        fn_1_4984(14, lbl_1_bss_100);
        fn_1_4984(15, lbl_1_bss_100);
        fn_1_4984(16, lbl_1_bss_100);
        fn_1_4984(17, lbl_1_bss_100);
        fn_1_4984(18, lbl_1_bss_100);
    }
    else if (MgModePlayNum <= 100U)
    {
        fn_1_4984(16, lbl_1_bss_100);
        fn_1_4984(17, lbl_1_bss_100);
        fn_1_4984(18, lbl_1_bss_100);
    }
    lbl_1_bss_100 = 0.0f;
    if (MgModePlayNum == 30)
    {
        frameOrCourseProgress = fn_1_4ADC() - (fn_1_4A2C() - fn_1_4C3C());
        frameOrCourseProgress /= fn_1_4C3C();
        lbl_1_bss_104.red = (u8)(160.0f * frameOrCourseProgress);
        lbl_1_bss_104.green = (u8)(64.0f * frameOrCourseProgress);
        lbl_1_bss_104.blue = 0;
    }
    if (MgModePlayNum == 39)
    {
        frameOrCourseProgress = fn_1_4ADC() - (fn_1_4A2C() - fn_1_4C3C());
        frameOrCourseProgress /= fn_1_4C3C();
        lbl_1_bss_104.red = (u8)((s32)(90.0f * frameOrCourseProgress) + 160);
        lbl_1_bss_104.green = (u8)((s32)(186.0f * frameOrCourseProgress) + 64);
        lbl_1_bss_104.blue = (u8)(250.0f * frameOrCourseProgress);
    }
    if (MgModePlayNum == 40)
    {
        lbl_1_bss_104.enabled = 0;
        Hu3DFogClear();
    }
}
