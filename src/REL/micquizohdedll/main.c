/* Microphone quiz object setup, camera configuration, and module exit. */
#include "msm_se.h"

#include "math.h"

#include "dolphin/gx.h"

#include "dolphin/types.h"

#include "dolphin/os.h"

#include "dolphin/mtx.h"

#include "string.h"

#include "game/object.h"

#include "REL/micquizohdedll.h"

#include "game/main.h"

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

#include "game/window.h"

#include "game/process.h"

#include "messdir_enum.h"

#include "game/init.h"

#include "game/mic.h"

#include "game/mgdata.h"

#include "game/board/player.h"

#include "dolphin.h"

#include "humath.h"

#include "game/hu3d.h"

#include "game/flag.h"

#include "game/armem.h"

#include "game/printfunc.h"

#include "game/saveload.h"

#include "game/esprite.h"

#include "game/thpmain.h"

#include "game/board/main.h"

#include "game/board/audio.h"

#include "game/board/masu.h"
#include "REL/micquiz/quiz_common.h"

void fn_1_548(OMOBJ *obj);
void fn_1_558(OMOBJ *obj);
void fn_1_6D8(void);

MicquizBss60Storage lbl_1_bss_60;
OMOBJ * lbl_1_bss_5C;
MicquizBss48Storage lbl_1_bss_48;
OMOBJ * lbl_1_bss_44;
OMOBJ * lbl_1_bss_40;
f32 lbl_1_bss_3C;
MicquizBss28Storage lbl_1_bss_28;
s16 lbl_1_bss_24;
MicquizResponseStorage lbl_1_bss_1C;
f32 micQuizPauseBgScroll;
ANIMDATA * micQuizPauseBgAnim;
HUPROCESS * micQuizMicPulseProc;
HUPROCESS * micQuizIndicatorSwingProc;
MicquizResponse *micQuizResponse;
s32 micQuizTimeoutHandled;
HUPROCESS * micQuizRingProc;

/* Camera view followed by initialized words with unknown meaning. */
typedef struct MicquizCameraData MicquizCameraData;
struct MicquizCameraData {
    OM_CAMERA_VIEW camera;
    u32 unknown_1C[9]; /* No code names this field. */
};

/* Initialized word with unknown meaning. */
u32 gap_04_00000384_data = 0xFFFFFFFF;

s32 lbl_1_data_388[0x36] = {
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x8e0000, 0x360000, 0xc00000, 0xf30000,
    0xec0000, 0x140000, 0xe60000, 0x300000, 0xcf0000, 0xa80000, 0x2a0000
};
Point3d lbl_1_data_460 = {0.0f, 10000.0f, 10000.0f};
Point3d lbl_1_data_46C = {0.0f, 0.0f, 0.0f};
/* Camera color. */
GXColor lbl_1_data_478 = { 0xFF, 0xFF, 0xFF, 0xFF };
MicquizCameraData lbl_1_data_47C = {
    { {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 700.0f },
    { 0xC2C80000, 0x45FA0000, 0x00000000, 0x00000000, 0x3F800000, 0x00000000, 0x00000000,
     0x00000000, 0x00000000 }
};
s32 lbl_1_data_4BC = -1;

/* Set up cameras, objects, lighting, and the quiz work area. */
void ObjectSetup(void) {
    OMOBJ *object;

    OSReport("******* Ohde ObjectSetup *********\n");
    lbl_1_bss_60.value = fn_8003A8BC(0xC8, 0x2000);
    fn_80069F1C(lbl_1_bss_60.value);
    Hu3DCameraCreate(3);
    Hu3DCameraPerspectiveSet(1, 45.0f, 10.0f,
                             20000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
                          640.0f, 480.0f,
                          0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 0x280, 0x1E0);
    Hu3DCameraPerspectiveSet(2, 45.0f, 10.0f,
                             20000.0f, 1.2f);
    Hu3DCameraViewportSet(2, 0.0f, 0.0f,
                          640.0f, 480.0f,
                          0.0f, 1.0f);
    Hu3DCameraScissorSet(2, 0, 0, 0x280, 0x1E0);
    fn_8003E488(1, &lbl_1_data_47C.camera);
    fn_8003E488(2, &lbl_1_data_47C.camera);
    lbl_1_bss_44 = fn_8003AB38(lbl_1_bss_60.value, 0x7FDA, 0, 0, -1, fn_8003DAA4);
    lbl_1_bss_44->work[0] = 2;
    lbl_1_bss_48.value = Hu3DGLightCreate(0.0f, 0.0f,
                                    0.0f, 0.0f,
                                    0.0f, 0.0f, 0, 0, 0);
    Hu3DGLightPosAimSetV((s16)lbl_1_bss_48.value, &lbl_1_data_460, &lbl_1_data_46C);
    Hu3DGLightStaticSet((s16)lbl_1_bss_48.value, 1);
    Hu3DGLightInfinitytSet((s16)lbl_1_bss_48.value);
    Hu3DGLightColorSet((s16)lbl_1_bss_48.value, lbl_1_data_478.r, lbl_1_data_478.g,
                       lbl_1_data_478.b, lbl_1_data_478.a);
    lbl_1_bss_3C = 0.0f;
    object = fn_8003AB38(lbl_1_bss_60.value, 0x65, 7, 0, -1, fn_1_548);
    lbl_1_bss_40 = object;
    lbl_1_bss_40->work[0] = 0;
    lbl_1_bss_5C = object;
    MicQuizHudCreate();
    MicQuizPauseInit(lbl_1_bss_60.value);
    if (micQuizModeWork != 0) {
        lbl_1_data_388[0] = micQuizModeWork->randomEnabled;
        lbl_1_bss_28.value = micQuizModeWork->mode0Option;
    }
}

void fn_1_548(OMOBJ *obj)
{
    obj->objFunc = fn_1_558;
}

/* Advance quiz startup and begin the exit wipe when requested. */
void fn_1_558(OMOBJ *obj) {
    if (omSysExitReq != 0) {
        if (lbl_1_data_4BC != -1) {
            HuAudSStreamFadeOut(lbl_1_data_4BC, 0x1F4);
            lbl_1_data_4BC = -1;
        }
        WipeCreate(2, 0, 0x14);
        obj->objFunc = (OMOBJ_FUNC)fn_1_6D8;
    }

    switch (obj->work[0]) {
    case 0:
        if (MicQuizQuestionColumnGet() == 0) {
            obj->work[0] = 2;
        } else {
            obj->work[0] = 2;
        }
        lbl_1_bss_3C = 0.0f;
        break;
    case 2:
        fn_1_12D08();
        WipeCreate(1, 0, 0x14);
        obj->work[0] = 3;
        break;
    case 3:
        if (WipeCheck() == 0) {
            fn_1_1D4AC(obj);
        }
        break;
    case 4:
        if (WipeCheck() == 0) {
            fn_8003A5E4(1, 1);
        }
        break;
    default:
        OSReport("*** main mode error(%d)!!\n", obj->work[0]);
        break;
    }
}

/* Close the microphone context after the exit wipe finishes. */
void fn_1_6D8(void) {
    if (WipeCheck() == 0) {
        HuMCSelWinContextKill();
        HuMCClose();
        fn_8003A5E4(1, 1);
    }
}

s32 fn_1_718(void)
{
    if ((s32)(lbl_802C0008[0] & 0x100) != 0) {
        return 1;
    }
    if ((s32)(lbl_802C0008[0] & 0x200) != 0) {
        return 2;
    }
    return 0;
}

s32 fn_1_760(s32 left, s32 right)
{
    if (left == right) {
        return 1;
    }
    return 2;
}

/* Initialize microphone input and create the selected context. */
s16 fn_1_778(char *contextId) {
    s16 context;

    HuMCInit(0);
    HuMCSelWinCreate(-10000.0f, 88.0f);
    context = HuMCContextCreate(contextId);
    return context;
}

/* Initialized words with unknown meaning. */
u32 micquizUnidentifiedWords500[0xB] = {
    0x00A4007A, 0x00A4007B, 0x00A4007C, 0x00A4007D, 0x00A4007E, 0x00A4007F, 0x00A40080, 0x00000800,
    0x00000200, 0x00000100, 0x00000400
};
s32 lbl_1_data_52C = 4;
