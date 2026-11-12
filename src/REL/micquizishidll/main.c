/* Microphone quiz setup, the main object running its rounds, shared helpers and module exit. */
#include "msm_se.h"

#include "math.h"

#include "dolphin/gx.h"

#include "dolphin/types.h"

#include "dolphin/os.h"

#include "string.h"

#include "game/mgdata.h"

#include "game/object.h"

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

#include "game/hu3d.h"

#include "game/printfunc.h"

#include "game/process.h"

#include "game/init.h"

#include "game/mic.h"

#include "game/board/player.h"

#include "dolphin.h"

#include "humath.h"

#include "messdir_enum.h"

#include "game/flag.h"

#include "game/armem.h"

#include "game/saveload.h"

#include "game/esprite.h"

#include "game/thpmain.h"

#include "game/board/main.h"

#include "game/board/audio.h"

#include "game/board/masu.h"

#include <math.h>
#include <dolphin/mtx.h>
#include "REL/micquiz/quiz_common.h"
#include "REL/micquizishidll.h"

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];

HUPROCESS *lbl_1_bss_114[0xB];
s32 lbl_1_bss_110;
u8 lbl_1_bss_10C[0x4];
OMOBJ *lbl_1_bss_108;
f32 lbl_1_bss_104;
s32 lbl_1_bss_100;
s32 lbl_1_bss_FC;
OMOBJ *lbl_1_bss_AC[0x14];
OMOBJ *lbl_1_bss_A8;
u8 lbl_1_bss_A6[0x2];
s16 lbl_1_bss_A4;
s16 lbl_1_bss_A2;
s16 lbl_1_bss_A0;
s16 lbl_1_bss_9C[2];
s32 lbl_1_bss_98;
s32 lbl_1_bss_94;
s32 lbl_1_bss_90;
f32 lbl_1_bss_8C;
s32 lbl_1_bss_74[6];
s32 lbl_1_bss_24[0x14];
s32 lbl_1_bss_20;
s32 lbl_1_bss_1C;
f32 micQuizPauseBgScroll;
ANIMDATA *micQuizPauseBgAnim;
HUPROCESS *micQuizMicPulseProc;
HUPROCESS *micQuizIndicatorSwingProc;
MicquizResponse *micQuizResponse;
s32 micQuizTimeoutHandled;
HUPROCESS *micQuizRingProc;

OM_CAMERA_VIEW lbl_1_data_388 = { { 0.0f, 210.0f, 0.0f }, { -23.5f, 0.0f, 0.0f }, 6195.0f };
u32 lbl_1_data_3A4[6][4] = { { 0xA40001, 0xA40002, 0xA40003, 0xA40004 },
        { 0xA40006, 0xA40007, 0xA40008, 0xA40009 }, { 0xA4000B, 0xA4000C, 0xA4000D,
                0xA4000E }, { 0xA40015, 0xA40016, 0xA40017, 0xA40018 }, { 0xA4001B, 0xA4001C,
                        0xA4001D, 0xA4001E }, { 0xA40010, 0xA40011, 0xA40012, 0xA40013 } };
s32 lbl_1_data_404[6] = { 0xA40000, 0xA40005, 0xA4000A, 0xA40014, 0xA4001A, 0xA4000F };
struct QuizSelectionEntry lbl_1_data_41C[5] = { { 1, 3, 5 }, { 1, 5, 10 }, { 2, 10, 15 },
                                               { 3, 10, 15 }, { 4, 15, 20 } };
s16 lbl_1_data_458[12] = { 579, 555, 699, 795, 747, 483, 771, 507, 723, 651, 531, 0 };
Point3d lbl_1_data_470 = { 0.0f, 10000.0f, 10000.0f };
Point3d lbl_1_data_47C = { 0.0f, 0.0f, 0.0f };
u8 lbl_1_data_488[4] = { 255, 255, 255, 255 };
u8 lbl_1_data_48C[0x40] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x2F, 0x00, 0x00, 0xC2, 0xC8, 0x00, 0x00, 0x45, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

void ObjectSetup(void);
void fn_1_224(OMOBJ *obj);
void fn_1_234(OMOBJ *object);
s32 fn_1_29AC(s32 value);
void fn_1_7D8(OMOBJ *object);

/* Runs the REL's static constructors, then sets up the minigame. */
int _prolog(void) {
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    ObjectSetup();
    return 0;
}

extern const VoidFunc _dtors[];

/* Runs the REL's static destructors. */
void _epilog(void) {
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Sets up the minigame: its object manager and main object, and the scene the selected mode starts with. */
void ObjectSetup(void) {
    OMOBJ *object;

    OSReport("******* Ishi ObjectSetup *********\n");
    lbl_1_bss_114[0] = fn_8003A8BC(0xC8, 0x2000);
    OSReport("                ISHI:%p\n", lbl_1_bss_114[0]);
    fn_80069F1C(lbl_1_bss_114[0]);
    lbl_1_bss_104 = 0.0f;
    lbl_1_bss_108 = object = fn_8003AB38(lbl_1_bss_114[0], 0x65, 0, 0, -1, fn_1_224);
    lbl_1_bss_108->work[0] = 0;
    micQuizModeWork = MgModeWorkGet();
    if (micQuizModeWork->questionColumn == 6) {
        lbl_1_bss_1C = 1;
    }
    if (lbl_1_bss_1C != 0) {
        micQuizModeWork->questionColumn = 0;
    }
    if (micQuizModeWork->questionColumn == 7) {
        MicQuizCompactHudCreate();
    } else {
        MicQuizHudCreate();
    }
    if (lbl_1_bss_1C != 0) {
        micQuizModeWork->questionColumn = 6;
    }
}
void fn_1_224(OMOBJ *obj)
{
    obj->objFunc = fn_1_234;
}
/* The main object's step: picks the next quiz round, wipes into it, runs it, and leaves the
   minigame when it ends. */
void fn_1_234(OMOBJ *object) {
    s32 selectedValue;

    if (omSysExitReq != 0) {
        HuAudSStreamFadeOut(lbl_1_bss_20, 0x1F4);
        WipeCreate(2, 0, 0x14);
        object->objFunc = fn_1_7D8;
    }

    switch (object->work[0]) {
    case 0:
        switch (MicQuizQuestionColumnGet()) {
        case 0:
            if (micQuizModeWork->randomEnabled != 0) {
                selectedValue = frandmod(2);
            }
            if (selectedValue == 0) {
                object->work[0] = 2;
            } else {
                object->work[0] = 4;
            }
            break;
        case 2:
            object->work[0] = 6;
            break;
        case 1:
            if (micQuizModeWork->randomEnabled != 0) {
                selectedValue = frandmod(2);
            } else {
                selectedValue = micQuizModeWork->modeOption[0];
            }
            if (selectedValue == 0) {
                object->work[0] = 2;
            } else {
                object->work[0] = 4;
            }
            break;
        case 3:
            if (micQuizModeWork->randomEnabled != 0) {
                selectedValue = frandmod(2);
            } else {
                selectedValue = micQuizModeWork->modeOption[2];
            }
            if (selectedValue == 0) {
                object->work[0] = 14;
            } else {
                object->work[0] = 16;
            }
            break;
        case 4:
            if (micQuizModeWork->randomEnabled != 0) {
                selectedValue = frandmod(3);
            } else {
                selectedValue = micQuizModeWork->modeOption[3];
            }
            if (selectedValue == 0) {
                object->work[0] = 8;
            } else if (selectedValue == 1) {
                object->work[0] = 10;
            } else if (selectedValue == 2) {
                object->work[0] = 12;
            }
            break;
        case 5:
            object->work[0] = 18;
            break;
        case 6:
            object->work[0] = 20;
            break;
        case 7:
            object->work[0] = 22;
            break;
        }
        lbl_1_bss_104 = 0.0f;
        break;
    case 2:
        fn_1_14EE0();
        HuPrcSleep(10);
        WipeCreate(1, 0, 0x14);
        object->work[0] = 3;
        break;
    case 3:
        if (WipeCheck() == 0) {
            fn_1_1598C(object);
        }
        break;
    case 4:
        fn_1_1706C();
        HuPrcSleep(10);
        WipeCreate(1, 0, 0x14);
        object->work[0] = 5;
        break;
    case 5:
        if (WipeCheck() == 0) {
            fn_1_17BEC(object);
        }
        break;
    case 6:
        fn_1_197D4();
        HuPrcSleep(10);
        WipeCreate(1, 0, 0x14);
        object->work[0] = 7;
        break;
    case 7:
        if (WipeCheck() == 0) {
            fn_1_19FD8(object);
        }
        break;
    case 8:
        fn_1_2A164();
        HuPrcSleep(10);
        WipeCreate(1, 0, 0x14);
        object->work[0] = 9;
        break;
    case 9:
        if (WipeCheck() == 0) {
            fn_1_2B290(object);
        }
        break;
    case 10:
        fn_1_1C210();
        HuPrcSleep(10);
        WipeCreate(1, 0, 0x14);
        object->work[0] = 11;
        break;
    case 11:
        if (WipeCheck() == 0) {
            fn_1_1DBBC(object);
        }
        break;
    case 12:
        fn_1_26810();
        HuPrcSleep(10);
        WipeCreate(1, 0, 0x14);
        object->work[0] = 13;
        break;
    case 13:
        if (WipeCheck() == 0) {
            fn_1_27CA0(object);
        }
        break;
    case 14:
        fn_1_1FB04();
        HuPrcSleep(10);
        WipeCreate(1, 0, 0x14);
        object->work[0] = 15;
        break;
    case 15:
        if (WipeCheck() == 0) {
            fn_1_20804(object);
        }
        break;
    case 16:
        fn_1_229E0();
        HuPrcSleep(10);
        WipeCreate(1, 0, 0x14);
        object->work[0] = 17;
        break;
    case 17:
        if (WipeCheck() == 0) {
            fn_1_24B40(object);
        }
        break;
    case 18:
        fn_1_2C624();
        HuPrcSleep(10);
        WipeCreate(1, 0, 0x14);
        object->work[0] = 19;
        break;
    case 19:
        if (WipeCheck() == 0) {
            fn_1_2CEA4(object);
        }
        break;
    case 20:
        fn_1_2EDB4();
        HuPrcSleep(10);
        WipeCreate(1, 0, 0x14);
        object->work[0] = 21;
        break;
    case 21:
        if (WipeCheck() == 0) {
            fn_1_2F2B8(object);
        }
        break;
    case 22:
        fn_1_2F890();
        HuPrcSleep(10);
        WipeCreate(1, 0, 0x14);
        object->work[0] = 23;
        break;
    case 23:
        if (WipeCheck() == 0) {
            fn_1_2FFA4(object);
        }
        break;
    case 24:
        if (WipeCheck() == 0) {
            fn_8003A5E4(1, 1);
        }
        break;
    default:
        OSReport("*** main mode error(%d)!!\n", object->work[0]);
        break;
    }
}
/* Waits for the closing wipe, closes the mic selection window and the mic, and leaves the
   minigame. */
void fn_1_7D8(OMOBJ *object) {
    if (WipeCheck() == 0) {
        if (lbl_1_bss_1C == 0) {
            HuMCSelWinContextKill();
            HuMCClose();
        }
        fn_8003A5E4(1, 1);
    }
}
void fn_1_82C(HuVecF *out, f32 x, f32 y, f32 z)
{
    out->x = x;
    out->y = y;
    out->z = z;
}
/* Shows the opening message in the dialogue window of the current dialogue mode. */
void fn_1_83C(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinMesSet(0x1E0002, 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(0x1E002A, 1, 3);
    }
    MicQuizWinMesWait();
}
/* Shows the given message in the dialogue window of the current dialogue mode. */
void fn_1_A38(u32 message) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinInsertMesSet(0, message, 0);
        MicQuizWinMesSet(0x1E0003, 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3, message, 0);
        MicQuizWinMesSet(0x1E002B, 1, 3);
    }
    MicQuizWinMesWait();
}
void fn_1_C78(void)
{
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinMesSet(0x1E0004, 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(0x1E002C, 1, 3);
    }
    MicQuizWinMesWait();
}
/* Shows the given message in the dialogue window of the current dialogue mode. */
void fn_1_E74(u32 message) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinInsertMesSet(0, message, 0);
        MicQuizWinMesSet(0x1E0005, 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3, message, 0);
        MicQuizWinMesSet(0x1E002D, 1, 3);
    }
    MicQuizWinMesWait();
}
/* Shows the given message in the dialogue window of the current dialogue mode. */
void fn_1_10B4(u32 message) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinInsertMesSet(0, message, 0);
        MicQuizWinMesSet(0x1E0006, 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3, message, 0);
        MicQuizWinMesSet(0x1E002E, 1, 3);
    }
    MicQuizWinMesWait();
}
void fn_1_12F4(void)
{
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinMesSet(0x1E0007, 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(0x1E002F, 1, 3);
    }
    MicQuizWinMesWait();
}
/* Shows the given message in the dialogue window of the current dialogue mode. */
void fn_1_14F0(u32 message) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinInsertMesSet(0, message, 0);
        MicQuizWinMesSet(0x1E0008, 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3, message, 0);
        MicQuizWinMesSet(0x1E0030, 1, 3);
    }
    MicQuizWinMesWait();
}
void fn_1_1730(void)
{
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinMesSet(0x1E0025, 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(0x1E003A, 1, 3);
    }
    MicQuizWinMesWait();
}
void fn_1_192C(void)
{
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinMesSet(0x1E0026, 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(0x1E003B, 1, 3);
    }
    MicQuizWinMesWait();
}
void fn_1_1B28(void)
{
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinMesSet(0x1E0027, 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(0x1E003C, 1, 3);
    }
    MicQuizWinMesWait();
}
void fn_1_1D24(void)
{
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinMesSet(0x1E0009, 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(0x1E0031, 1, 3);
    }
    MicQuizWinMesWait();
}
/* Shows the given message in the dialogue window of the current dialogue mode. */
void fn_1_1F20(u32 message) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinInsertMesSet(0, message, 0);
        MicQuizWinMesSet(0x1E000A, 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3, message, 0);
        MicQuizWinMesSet(0x1E0032, 1, 3);
    }
    MicQuizWinMesWait();
}
void fn_1_2160(void)
{
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinMesSet(0x1E0022, 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(0x1E0037, 1, 3);
    }
    MicQuizWinMesWait();
}
void fn_1_235C(void)
{
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinMesSet(0x1E0023, 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(0x1E0037, 1, 3);
    }
    MicQuizWinMesWait();
}
void fn_1_2558(void)
{
    MicQuizWinOpen(3);
    MicQuizWinMesSet(0x1E003D, 1, 3);
    MicQuizWinMesWait();
}
void fn_1_2670(void)
{
    MicQuizWinOpen(3);
    MicQuizWinMesSet(0x1E004E, 1, 3);
    MicQuizWinMesWait();
}
/* Starts an object's motion unless that motion is already playing. */
void fn_1_2788(OMOBJ *object, s32 motion, f32 scale, u32 attribute) {
    MicquizObjectWork *work = (MicquizObjectWork *)object->data;

    if (work->currentMotion != motion) {
        work->currentMotion = motion;
        if (work->motionIds[motion] != -1) {
            fn_8002C95C(work->modelId, work->motionIds[motion]);
        }
    }
}
/* Picks six different random values for the round. */
void fn_1_2800(void) {
    s32 index;
    s32 candidate;

    for (index = 0; index < 6; index++) {
        lbl_1_bss_74[index] = -1;
    }
    for (index = 0; index < 0x14; index++) {
        lbl_1_bss_24[index] = -1;
    }
    for (index = 0; index < lbl_1_data_41C[lbl_1_bss_100].count; index++) {
        do {
            candidate = frandmod(6);
        } while (fn_1_29AC(candidate) == 0);
        lbl_1_bss_74[index] = candidate;
    }
    for (index = 0; index < lbl_1_data_41C[lbl_1_bss_100].count; index++) {
        lbl_1_bss_24[index] = lbl_1_bss_74[index];
    }
    for (; index < lbl_1_bss_90; index++) {
        lbl_1_bss_24[index] = lbl_1_bss_74[frandmod(lbl_1_data_41C[lbl_1_bss_100].count)];
    }
}
/* Returns 1 when the round's six values do not hold the value yet. */
s32 fn_1_29AC(s32 value) {
    s32 i;
    for (i = 0; i < 6; i++) {
        if (lbl_1_bss_74[i] != -1 && value == lbl_1_bss_74[i]) {
            return 0;
        }
    }
    return 1;
}
/* Returns 1 when two circles overlap in the XZ plane. */
s32 fn_1_2A18(Vec *position0, f32 radius0, Vec *position1, f32 radius1) {
    if ((((position1->x - position0->x) * (position1->x - position0->x)) +
         ((position1->z - position0->z) * (position1->z - position0->z))) <
        ((radius0 + radius1) * (radius0 + radius1))) {
        return 1;
    }
    return 0;
}
/* Returns 1 when two circles overlap in the XY plane. */
s32 fn_1_2A78(Vec *position0, Vec *position1, f32 radius0, f32 radius1) {
    if ((((position1->x - position0->x) * (position1->x - position0->x)) +
         ((position1->y - position0->y) * (position1->y - position0->y))) <
        ((radius0 + radius1) * (radius0 + radius1))) {
        return 1;
    }
    return 0;
}
/* Turns an angle part of the way toward another, the short way round. */
f32 fn_1_2AD8(f32 start, f32 end, f32 amount) {
    f32 delta;
    f32 result;

    delta = fmod(end - start, 360.0);
    if (0.0f > delta) {
        delta += 360.0f;
    }
    if (180.0f < delta) {
        delta -= 360.0f;
    }
    result = fmod(start + delta * amount, 360.0);
    if (0.0f > result) {
        result += 360.0f;
    }
    return result;
}
/* Turns an angle toward another over the given number of frames, the short way round. */
f32 fn_1_2BDC(f32 start, f32 end, s32 frameCount) {
    f32 delta;
    f32 result;
    f32 fraction;

    delta = fmod(end - start, 360.0);
    if (0.0f > delta) {
        delta += 360.0f;
    }
    if (180.0f < delta) {
        delta -= 360.0f;
    }
    fraction = delta / (f32)frameCount;
    result = fmod(start + delta * fraction, 360.0);
    if (0.0f > result) {
        result += 360.0f;
    }
    return result;
}
/* Gives the world position of a model's named object. */
void fn_1_2D18(HU3D_MODELID modelId, char *objName, Vec *position) {
    Mtx matrix;

    fn_80023398(modelId, objName, matrix);
    position->x = matrix[0][3];
    position->y = matrix[1][3];
    position->z = matrix[2][3];
}
/* Debug view: prints the camera's center, rotation and zoom in a window. */
void fn_1_2D70(void) {
    GXColor windowColor;

    windowColor.r = 0;
    windowColor.g = 0;
    windowColor.b = 0x90;
    windowColor.a = 0xC0;

    lbl_80266BD8[0] += (f32)(lbl_802BFFEC[0] / 5);
    lbl_80266BD8[0] -= (f32)(lbl_802BFFE8[0] / 5);
    if (lbl_802C0010[0] & 0x4) lbl_80266B18[0].y -= 5.0f;
    if (lbl_802C0010[0] & 0x8) lbl_80266B18[0].y += 5.0f;
    if (lbl_802C0010[0] & 0x1) lbl_80266B18[0].z -= 5.0f;
    if (lbl_802C0010[0] & 0x2) lbl_80266B18[0].z += 5.0f;
    lbl_80266A58[0].x += 5.0f * ((f32)lbl_802BFFF0[0] / 44.0f);
    lbl_80266A58[0].y -= 5.0f * ((f32)lbl_802BFFF4[0] / 44.0f);

    if (lbl_802C0008[0] & 0x800) fn_8003E488(1, &lbl_1_data_388);

    printWin(0x10, 0x28, 0x180, 0x54, &windowColor);
    fontcolor = FONT_COLOR_YELLOW;
    print8(0x10, 0x28, 1.0f, "Center X:%.02f Y:%.02f Z:%.02f", lbl_80266B18[0].x, lbl_80266B18[0].y,
           lbl_80266B18[0].z);
    print8(0x10, 0x34, 1.0f, "CRot X:%.02f Y:%.02f Z:%.02f", lbl_80266A58[0].x, lbl_80266A58[0].y,
           lbl_80266A58[0].z);
    print8(0x10, 0x40, 1.0f, "CZoom:%.02f", lbl_80266BD8[0]);
}
