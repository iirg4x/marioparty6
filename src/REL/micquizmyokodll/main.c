// Mic quiz minigame (micquizmyokodll): a parade of houses, each with npcs, items and
// sometimes a guest character, scrolls past the host; the player then answers by voice
// which character was at a house, or how many items or npcs of one type went by.
// This file holds the scene setup, the main object's state machine, the answer timer and the
// reveal.
#include "math.h"
#include "dolphin/gx.h"
#include "dolphin/types.h"
#include "dolphin/os.h"
#include "dolphin/mtx.h"
#include "game/window.h"
#include "REL/micquizmyokodll/module_types.h"
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
#include "messdir_enum.h"
#include "game/mic.h"
#include "humath.h"
#include "msm_se.h"
#include "msm_stream.h"
#include "dolphin.h"
#include "game/hu3d.h"
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
#include "REL/micquizmyokodll/shared_types.h"
#include "REL/micquizmyokodll/message_tables.h"
#include "game/init.h"
#include "REL/micquizmyokodll/response_table.h"

void ObjectSetup(void);

void fn_1_110(OMOBJ *obj);

void fn_1_220(OMOBJ *object);

#include <string.h>
#include "REL/micquiz/quiz_common.h"

void fn_1_148(OMOBJ *object);

void fn_1_1898(void);

void fn_1_1A4C(void);

void fn_1_1BAC(s32 messageSet);

void fn_1_1F04(void);

s32 fn_1_16B4(void);

s32 fn_1_17A8(void);

void fn_1_31CC(s16 playerIndex);

void fn_1_1F4(void);

void fn_1_2B14(s16 playerIndex);

void fn_1_3258(void);

typedef void (*VoidFunc)(void);
typedef MicquizPlayerWork MicquizPlayerModels;
extern s16 lbl_1_bss_20[0x14];
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern HUPROCESS *lbl_1_bss_2B8[11];
extern MicquizControlWork lbl_1_bss_10A;
extern s32 lbl_1_bss_1C;
extern MicquizAnimationState lbl_1_bss_54;
extern MicquizSequence lbl_1_bss_D4;
extern MicquizHostState lbl_1_bss_F4;
extern s16 lbl_1_data_3FC[6];
extern s16 lbl_1_bss_48[6];
extern HUPROCESS *lbl_1_bss_2A4[5];
extern MicquizAccessoryPoolEntry lbl_1_bss_98[6];

extern u16 lbl_1_rodata_33C[5];
extern u32 lbl_1_data_4B8[9];
extern MicquizPlayerWork lbl_1_bss_128[5];

extern s32 lbl_1_data_43C[6];
extern s16 lbl_1_bss_CE;
extern s16 lbl_1_bss_D2[4];
extern s16 lbl_1_bss_18A;
extern s16 lbl_1_bss_8C;
extern s16 lbl_1_bss_8E;
int _prolog(void);
void _epilog(void);
void fn_1_938(void);
void fn_1_1EB4(void);
s32 fn_1_45B0(s16 playerIndex);
s32 fn_1_4728(s16 playerIndex);
void fn_1_48C8(void);
void fn_1_5194(void);
void fn_1_5288(void);
void fn_1_7188(void);
void fn_1_71D0(void);
void fn_1_7754(void);
s32 fn_1_7FAC(void);
void fn_1_8200(void);
void fn_1_8280(s32 modelIndex);
void fn_1_829C(s16 layerNo);
void fn_1_834C(void);

extern Point3d lbl_1_data_408;
extern Point3d lbl_1_data_414;
extern GXColor lbl_1_data_420;
extern s32 lbl_1_data_47C[5];
extern s32 lbl_1_data_490[5];
extern s32 lbl_1_data_4A4[5];
extern u32 lbl_1_data_394[5][2];
extern u32 lbl_1_data_3BC[5];
extern u32 lbl_1_data_3D0[5];
extern u32 lbl_1_data_3E4[6];
extern s32 lbl_1_data_4DC[11];
extern s32 lbl_1_data_508[11];
extern s32 lbl_1_data_534[11];
extern char lbl_1_data_5F8[12];
extern s16 lbl_1_data_388[6];
extern s32 lbl_1_data_424[6];
extern s32 lbl_1_data_454[5];
extern s32 lbl_1_data_468[5];
extern f32 lbl_1_data_560[11];
s16 lbl_1_data_388[6] = { 0x0, 0x0, 0x0, 0x0, 0x50, 0x0 };

u32 lbl_1_data_394[5][2] = {
    { MESSNUM(MESS_MICQUIZ_Q, 19), MESSNUM(MESS_MICQUIZ_Q, 51) },
    { MESSNUM(MESS_MICQUIZ_Q, 20), MESSNUM(MESS_MICQUIZ_Q, 52) },
    { MESSNUM(MESS_MICQUIZ_Q, 21), MESSNUM(MESS_MICQUIZ_Q, 53) },
    { MESSNUM(MESS_MICQUIZ_Q, 22), MESSNUM(MESS_MICQUIZ_Q, 54) },
    { MESSNUM(MESS_MICQUIZ, 29), MESSNUM(MESS_MICQUIZ, 85) }
};
u32 lbl_1_data_3BC[5] = {
    MESSNUM(MESS_MICQUIZ_Q, 23), MESSNUM(MESS_MICQUIZ_Q, 24), MESSNUM(MESS_MICQUIZ_Q, 25),
    MESSNUM(MESS_MICQUIZ_Q, 26), MESSNUM(MESS_MICQUIZ_Q, 27)
};
u32 lbl_1_data_3D0[5] = {
    MESSNUM(MESS_MICQUIZ_Q, 28), MESSNUM(MESS_MICQUIZ_Q, 29), MESSNUM(MESS_MICQUIZ_Q, 31),
    MESSNUM(MESS_MICQUIZ_Q, 32), MESSNUM(MESS_MICQUIZ_Q, 33)
};
u32 lbl_1_data_3E4[6] = {
    MESSNUM(MESS_QUIZ_CHARA, 12), MESSNUM(MESS_QUIZ_CHARA, 13), MESSNUM(MESS_QUIZ_CHARA, 14),
    MESSNUM(MESS_QUIZ_CHARA, 17), MESSNUM(MESS_QUIZ_CHARA, 15), MESSNUM(MESS_QUIZ_CHARA, 16)
};
s16 lbl_1_data_3FC[6] = { 1, 2, 3, 4, 5, 0 };
Point3d lbl_1_data_408 = { -2000.0f, 10000.0f, -400.0f };
Point3d lbl_1_data_414 = { -0.4f, -0.8f, -0.8f };
GXColor lbl_1_data_420 = { 0xC0, 0xC0, 0xC0, 0xC0 };
s32 lbl_1_data_424[6] = {
    DATANUM(0xa50000, 0), DATANUM(0xa50000, 5), DATANUM(0xa50000, 10),
    DATANUM(0xa50000, 15), DATANUM(0xa50000, 20), DATANUM(0xa50000, 25)
};
s32 lbl_1_data_43C[6] = {
    960, 981, 986,
    999, 0, 939
};
s32 lbl_1_data_454[5] = {
    DATANUM(0xa50000, 50), DATANUM(0xa50000, 51), DATANUM(0xa50000, 52),
    DATANUM(0xa50000, 54), DATANUM(0xa50000, 56)
};
s32 lbl_1_data_468[5] = {
    DATANUM(0xa50000, 57), DATANUM(0xa50000, 58), DATANUM(0xa50000, 59),
    DATANUM(0xa50000, 60), DATANUM(0xa50000, 61)
};
s32 lbl_1_data_47C[5] = {
    DATANUM(0xa50000, 30), DATANUM(0xa50000, 31), DATANUM(0xa50000, 32),
    DATANUM(0xa50000, 33), DATANUM(0xa50000, 34)
};
s32 lbl_1_data_490[5] = {
    DATANUM(0xa50000, 35), DATANUM(0xa50000, 36), DATANUM(0xa50000, 37),
    DATANUM(0xa50000, 38), DATANUM(0xa50000, 39)
};
s32 lbl_1_data_4A4[5] = {
    DATANUM(0xa50000, 40), DATANUM(0xa50000, 41), DATANUM(0xa50000, 42),
    DATANUM(0xa50000, 43), DATANUM(0xa50000, 44)
};
u32 lbl_1_data_4B8[9] = {
    DATANUM(0x930000, 0), DATANUM(0x930000, 1), DATANUM(0x930000, 39),
    DATANUM(0x930000, 38), DATANUM(0x930000, 40), DATANUM(0x930000, 3),
    DATANUM(0x930000, 4), DATANUM(0x930000, 11), DATANUM(0x930000, 34)
};
s32 lbl_1_data_4DC[11] = {
    0x910000, 0x390000, 0xc30000, 0xf60000, 0xef0000,
    0x170000, 0xe90000, 0x330000, 0xd20000, 0xba0000,
    0x2d0000
};
s32 lbl_1_data_508[11] = {
    0x930000, 0x3b0000, 0xc50000, 0xf80000, 0xf10000,
    0x190000, 0xeb0000, 0x350000, 0xd40000, 0xbc0000,
    0x2f0000
};
s32 lbl_1_data_534[11] = {
    0x8e0000, 0x360000, 0xc00000, 0xf30000, 0xec0000,
    0x140000, 0xe60000, 0x300000, 0xcf0000, 0xa80000, 0x2a0000
};

f32 lbl_1_data_560[11] = {
    85.0f, 80.0f, 60.0f, 85.0f, 100.0f, 60.0f,
    50.0f, 140.0f, 105.0f, 140.0f, 140.0f
};

extern s16 lbl_1_bss_2D8;

// Module entry: runs every static constructor in the list, then sets up the minigame.
int _prolog(void) {
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    ObjectSetup();
    return 0;
}

// Module exit: runs every static destructor in the list.
void _epilog(void) {
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

// Creates the module's object-manager process and the main game object, whose first
// call (fn_1_110) builds the scene and switches it to the per-frame update.
void ObjectSetup(void) {
    OMOBJ *object;

    lbl_1_bss_2B8[0] = fn_8003A8BC(0xC8, 0x2000);
    fn_80069F1C(lbl_1_bss_2B8[0]);
    object = fn_8003AB38(lbl_1_bss_2B8[0], 0x2000, 0U, 0U, -1, fn_1_110);
}

void fn_1_110(OMOBJ *obj)
{
    fn_1_938();
    obj->objFunc = fn_1_220;
}

// Exit step of the main object: once the screen wipe has finished, frees the snapshot
// sprites, fades out the music, closes the message window and the microphone, and leaves the
// minigame.
void fn_1_148(OMOBJ *object) {
    s32 animationIndex;

    if (WipeCheck() == 0) {
        animationIndex = 0;
        while (animationIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]) {
            HuSprAnimKill(lbl_1_bss_54.spriteAnimations[animationIndex]);
            animationIndex += 1;
        }
        HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
        fn_1_1EB4();
        HuMCListenerKill();
        HuMCClose();
        fn_8003A5E4(1, 1);
    }
}

void fn_1_1F4(void) {
    MicQuizResponseRun(lbl_1_bss_D4.responseMode);
}

// Per-frame update of the main object: handles an exit request, ticks the answer timer down
// (time up at zero), and steps through the phases: intro messages, the houses scrolling past,
// the question, waiting for the spoken answer, the hint-board replay and the answer reveal.
void fn_1_220(OMOBJ *process) {
    s32 animationIndex;
    s32 soundValue;
    s32 soundBank;
    s32 soundSlot;

    if (omSysExitReq != 0) {
        for (animationIndex = 0;
             animationIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode];
             animationIndex++) {
            HuSprAnimKill(lbl_1_bss_54.spriteAnimations[animationIndex]);
        }
        HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
        WipeCreate(2, 0, 0x14);
        process->objFunc = fn_1_148;
    }

    if (micQuizCountdownState == 0) {
        micQuizCountdownFrame++;
        if (micQuizCountdownFrame >= micQuizCountdownStepFrames) {
            HuAudFXPlay(MSM_SE_MICQUIZ_COUNTDOWN_TICK);
            micQuizCountdownValue--;
            micQuizCountdownFrame = 0;
            soundValue = micQuizCountdownValue;
            if (soundValue >= 0) {
                soundBank = soundValue / 10;
                soundSlot = soundValue - (soundBank * 10);
                HuSprBankSet(micQuizCountdownGroup[0], 0, soundBank);
                HuSprBankSet(micQuizCountdownGroup[0], 1, soundSlot);
            }
            if (micQuizCountdownValue <= 0) {
                micQuizCountdownState = 2;
            }
        }
    }

    switch (lbl_1_bss_10A.phase) {
    case 0:
        WipeCreate(1, 0, 0x14);
        lbl_1_bss_10A.phase++;
        break;
    case 1:
        if (lbl_1_bss_10A.timer++ >= 0x1E) {
            lbl_1_bss_1C = HuAudBGMPlay(MSM_STREAM_QUESTION_MUSIC_THIRD);
            fn_1_1A4C();
            fn_1_1BAC(0);
            lbl_1_bss_10A.phase++;
            lbl_1_bss_10A.timer = 0;
        }
        break;
    case 2:
        if (lbl_1_bss_10A.timer++ == 0x2D) {
            micQuizGameMes = GameMesCreate(2, 0);
        } else if (lbl_1_bss_10A.timer > 0x2D &&
                   GameMesStatGet(micQuizGameMes) == 0) {
            fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[1],
                               0.0f, 0.0f,
                               ((1 << 30) | (1 << 0)));
            lbl_1_bss_10A.phase++;
            lbl_1_bss_10A.timer = 0;
        }
        break;
    case 3:
        if ((s16)fn_1_16B4() != 0) {
            fn_1_31CC(0);
            lbl_1_bss_10A.phase++;
        }
        break;
    case 4:
        if ((s16)fn_1_45B0(lbl_1_bss_10A.currentPlayer) != 0) {
            lbl_1_bss_10A.currentPlayer++;
        }
        if (lbl_1_bss_10A.currentPlayer >=
            lbl_1_data_3FC[lbl_1_bss_10A.mode]) {
            lbl_1_bss_10A.phase++;
            lbl_1_bss_10A.flag_80 = 0;
            lbl_1_bss_10A.flag_40 = 1;
        } else {
            fn_1_31CC(lbl_1_bss_10A.currentPlayer);
        }
        break;
    case 5: {
        Point3d modelPosition;
        fn_800267CC(lbl_1_bss_F4.modelId, &modelPosition);
        if (modelPosition.x > 600.0f) {
            fn_8005BE80(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[0]);
            lbl_1_bss_10A.phase++;
        }
        break;
    }
    case 6:
        if ((s16)fn_1_17A8() != 0) {
            lbl_1_bss_10A.flag_40 = 0;
            fn_1_1BAC(lbl_1_bss_D4.phase + 1);
            if (micQuizActiveWin != -1) {
                HuWinMesWait(micQuizActiveWin);
            }
            micQuizQuestionReady = 1;
            lbl_1_bss_10A.phase++;
        }
        break;
    case 7: {
        s32 firstResult = micQuizResponseState;
        if (firstResult == 3) {
            lbl_1_bss_10A.phase = 9;
            fn_1_48C8();
        } else {
            s32 secondResult = micQuizResponseState;
            if (secondResult == 4) {
                lbl_1_bss_10A.completionFlag = 1;
                lbl_1_bss_10A.phase = 9;
                fn_1_48C8();
            } else {
                s32 thirdResult = micQuizResponseState;
                if (thirdResult == 2) {
                    lbl_1_bss_10A.phase = 12;
                }
            }
        }
        break;
    }
    case 9:
        if ((s16)fn_1_16B4() != 0) {
            fn_1_5288();
        }
        break;
    case 12:
        Hu3DAmbColorSet(0.01f, 0.01f,
                        0.01f);
        lbl_1_bss_10A.phase = 13;
        break;
    case 13:
        if ((s16)fn_1_16B4() != 0) {
            lbl_1_bss_10A.phase = 14;
        }
        break;
    case 14:
        if ((s16)fn_1_7FAC() != 0) {
            lbl_1_bss_10A.phase = 15;
        }
        break;
    case 15:
        if ((s16)fn_1_17A8() != 0) {
            lbl_1_bss_10A.phase = 7;
            Hu3DAmbColorSet(1.0f, 1.0f,
                            1.0f);
            fn_1_8200();
            if (micQuizMicHudMotion[0] != 2) {
                micQuizMicHudMotion[0] = 2;
                fn_8000DAC0(MicQuizMicGlowFadeIn, 0x64, 0x1800, 0, micQuizParentProc);
            }
            micQuizListenReady = 1;
            micQuizResponseState = 1;
        }
        break;
    case 8:
    case 10:
    case 11:
        break;
    }

    fn_1_1898();
}

// Builds the scene: full-screen camera and a small 192x164 snapshot camera, light and shadow,
// scrolling background and stage models, the host with its nine motions, the timer, the random
// houses and question (fn_1_1F04), the microphone context and the helper processes.
void fn_1_938(void) {
    Point3d shadowPosition = {0.0f, 3000.0f, 0.0f};
    Point3d shadowDirection = {0.0f, 1.0f, 0.0f};
    Point3d shadowTarget = {0.0f, 0.0f, -1000.0f};
    f32 far, near, fov;
    s16 motionId;
    s32 cameraMask;
    HUPROCESS *parentProcess;
    s16 lightId;
    s16 modelId;
    s32 cameraIndex;
    s32 index;

    for (index = 0; index < 5; index++) lbl_1_bss_2A4[index] = 0;
    memset(&lbl_1_bss_10A, 0, 0x1E);
    Hu3DCameraCreate(2);
    Hu3DCameraPerspectiveSet(2, 30.0f, 10.0f, 5000.0f, 1.2f);
    Hu3DCameraViewportSet(2, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(2, 0, 0, 0x280, 0x1E0);
    Hu3DCameraPosSet(2, 0.0f, 543.0f, 1615.03f, 0.0f, 1.0f, 0.0f, 0.0f, 202.32f, -31.21f);
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 20.0f, 10.0f, 5000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 192.0f, 164.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 0xC0, 0xA4);
    Hu3DCameraPosSet(1, 0.0f, 543.0f, 1615.03f, 0.0f, 1.0f, 0.0f, 0.0f, 202.32f, -31.21f);
    MicQuizHudCreate();
    parentProcess = lbl_1_bss_2B8[0];
    micQuizPauseProc = fn_8000DAC0(MicQuizPauseWatch, 0xFDE8, 0x2000, 0, fn_8000DD9C());
    HuPrcSetStat(micQuizPauseProc, 0xC);
    micQuizModeWork->pauseAllowed = 0;
    for (cameraIndex = 0; cameraIndex < 0x10; cameraIndex++) {
        fn_80027D8C(1 << cameraIndex, &fov, &near, &far);
        if (fov != -1.0f) cameraMask = 1 << cameraIndex;
    }
    micQuizPauseCameraBit = cameraMask;
    micQuizParentProc = parentProcess;
    MicQuizCountdownInit();
    MicQuizPortraitShow(MicQuizCurrentPlayerGet());
    lbl_1_bss_10A.mode = MicQuizQuestionLevelGet();
    lightId = fn_800286D0(&lbl_1_data_408, &lbl_1_data_414, &lbl_1_data_420);
    Hu3DGLightStaticSet(lightId, 1);
    Hu3DGLightInfinitytSet(lightId);
    Hu3DShadowCreate(30.0f, 20.0f, 5000.0f);
    Hu3DShadowPosSet(&shadowPosition, &shadowDirection, &shadowTarget);
    for (index = 0; index < 2; index++) {
        modelId = fn_80025A04(fn_80008DD0(lbl_1_data_47C[lbl_1_bss_10A.mode], 0x10000000, 2));
        fn_80026760(modelId, -1300.0f + 2600.0f * index, 0.0f, 0.0f);
        lbl_1_bss_10A.sideModelIds[index] = modelId;
        modelId = fn_80025A04(fn_80008DD0(lbl_1_data_490[lbl_1_bss_10A.mode], 0x10000000, 2));
        fn_80026760(modelId, -1000.0f + 2000.0f * index, 0.0f, 0.0f);
        fn_80026FFC(modelId);
        lbl_1_bss_10A.shadowModelIds[index] = modelId;
    }
    modelId = fn_80025A04(fn_80008DD0(lbl_1_data_4A4[lbl_1_bss_10A.mode], 0x10000000, 2));
    lbl_1_bss_10A.modelIds[0] = modelId;
    modelId = fn_80025A04(fn_80008DD0(DATANUM(0xa50000, 45), 0x10000000, 2));
    fn_80026760(modelId, 0.0f, -150.0f, 50.0f);
    fn_80026B8C(modelId, 1);
    lbl_1_bss_10A.modelIds[1] = modelId;
    modelId = fn_80025A04(fn_80008DD0(DATANUM(0xa50000, 46), 0x10000000, 2));
    fn_80026B8C(modelId, 1);
    lbl_1_bss_10A.modelIds[2] = modelId;
    modelId = fn_80025A04(fn_80008DD0(DATANUM(0xa50000, 47), 0x10000000, 2));
    fn_80026B8C(modelId, 1);
    lbl_1_bss_10A.modelIds[3] = modelId;
    modelId = fn_80025A04(fn_80008DD0(DATANUM(0xa50000, 48), 0x10000000, 2));
    lbl_1_bss_10A.modelIds[4] = modelId;
    lbl_1_bss_F4.charNo = lbl_80265728[MicQuizCurrentPlayerGet()].charNo;
    modelId = fn_80057068(lbl_1_bss_F4.charNo, 8);
    fn_80026760(modelId, 0.0f, 15.0f, -350.0f);
    fn_80026808(modelId, 0.0f, 90.0f, 0.0f);
    fn_80026E44(modelId);
    lbl_1_bss_F4.modelId = modelId;
    for (index = 0; index < 9; index++) {
        motionId = fn_8005B0B8(lbl_1_bss_F4.charNo, lbl_1_data_4B8[index]);
        lbl_1_bss_F4.motionIds[index] = motionId;
    }
    fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[0], 0.0f, 0.0f, ((1 << 30) | (1 << 0)));
    lbl_1_bss_10A.flag_80 = 1;
    for (index = 0; index < 6; index++) lbl_1_bss_98[index].modelId = -1;
    for (index = 0; index < 5; index++) lbl_1_bss_48[index] = -1;
    fn_1_1F04();
    MicQuizMicContextSet(lbl_1_bss_D4.responseMode);
    fn_8000DAC0(fn_1_1F4, 0xA, 0x1800, 0, lbl_1_bss_2B8[0]);
}

// Slides stage models 2 and 3 apart (mirrored, 20 units per frame) until model 3 reaches
// x = 450; returns 1 once fully open.
s32 fn_1_16B4(void) {
    Point3d position;
    s32 i;

    i = 0;
    while (i < 2) {
        fn_800267CC(lbl_1_bss_10A.modelIds[3], &position);
        position.x += 10.0f;
        if (position.x >= 450.0f) {
            position.x = 450.0f;
        }
        fn_80026760(lbl_1_bss_10A.modelIds[3], position.x, position.y, position.z);
        fn_80026760(lbl_1_bss_10A.modelIds[2], -position.x, position.y, position.z);
        i += 1;
    }
    if (position.x >= 450.0f) {
        return 1;
    }
    return 0;
}

// Slides stage models 2 and 3 back together (20 units per frame) until they meet at x = 0;
// returns 1 once closed.
s32 fn_1_17A8(void) {
    Point3d position;
    s32 i;

    i = 0;
    while (i < 2) {
        fn_800267CC(lbl_1_bss_10A.modelIds[3], &position);
        position.x = position.x - 10.0f;
        if (position.x < 0.0f) {
            position.x = 0.0f;
        }
        fn_80026760(lbl_1_bss_10A.modelIds[3], position.x, position.y, position.z);
        fn_80026760(lbl_1_bss_10A.modelIds[2], -position.x, position.y, position.z);
        i += 1;
    }
    if (position.x <= 0.0f) {
        return 1;
    }
    return 0;
}

// Every frame: while the parade scrolls, moves both pairs of background models left at
// different speeds and wraps them around; otherwise, if the walk flag is set, walks the host right.
void fn_1_1898(void) {
    Point3d position;
    s32 i;

    if (lbl_1_bss_10A.flag_80 != 0) {
        i = 0;
        while (i < 2) {
            fn_800267CC(lbl_1_bss_10A.sideModelIds[i], &position);
            position.x -= 1.8f;
            if (position.x < -2600.0f) {
                position.x += 5200.0f;
            }
            fn_80026790(lbl_1_bss_10A.sideModelIds[i], &position);
            fn_800267CC(lbl_1_bss_10A.shadowModelIds[i], &position);
            position.x -= 3.6f;
            if (position.x < -2000.0f) {
                position.x += 4000.0f;
            }
            fn_80026790(lbl_1_bss_10A.shadowModelIds[i], &position);
            i += 1;
        }
        return;
    }
    if (lbl_1_bss_10A.flag_40 != 0) {
        fn_800267CC(lbl_1_bss_F4.modelId, &position);
        position.x += 3.6f;
        fn_80026790(lbl_1_bss_F4.modelId, &position);
    }
}

// Opens the host's message window: window 2 or 3 depending on the presenter layout.
void fn_1_1A4C(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(2);
    } else {
        MicQuizWinOpen(3);
    }
}

// Shows message set messageSet in the host's window (the layout picks window and table column);
// sets 1-3 also insert the name of the house, item type or npc type the question is about.
void fn_1_1BAC(s32 messageSet) {
    s32 windowMode;
    s32 dataSetIndex;
    u32 mainMessage;
    u32 firstInsertMessage;
    u32 secondInsertMessage;
    u32 thirdInsertMessage;

    if (micQuizModeWork->presenterLayout == 0) {
        windowMode = 2;
        dataSetIndex = 0;
    } else {
        windowMode = 3;
        dataSetIndex = 1;
    }

    mainMessage = lbl_1_data_394[messageSet][dataSetIndex];

    MicQuizWinMesSet(mainMessage, 1, windowMode);

    switch (messageSet) {
    case 1:
    {
        firstInsertMessage = lbl_1_data_3BC[lbl_1_bss_D4.playerIndex];
        MicQuizWinInsertMesSet(windowMode, firstInsertMessage, 0);
        break;
    }
    case 2:
    {
        secondInsertMessage = lbl_1_data_3D0[lbl_1_bss_D4.playerIndex];
        MicQuizWinInsertMesSet(windowMode, secondInsertMessage, 0);
        break;
    }
    case 3:
    {
        thirdInsertMessage = lbl_1_data_3E4[lbl_1_bss_D4.playerIndex];
        MicQuizWinInsertMesSet(windowMode, thirdInsertMessage, 0);
        break;
    }
    }
}

void fn_1_1EB4(void)
{
    if (micQuizActiveWin != -1) {
        HuWinExClose(micQuizActiveWin);
        micQuizActiveWin = -1;
    }
}

// Randomly generates the parade: per house a unique house type, maybe a guest character (at most
// 3),
// 1-3 npcs and 1-3 items of one type each; then picks the question and its answer, and creates
// the hint boards with their snapshot textures and the snapshot draw hook.
void fn_1_1F04(void) {
    s16 positionUsed[5];
    s16 firstModelRemaining[6];
    s16 secondModelRemaining[5];
    s16 playerTypeCounts[5] = {9, 8, 7, 6, 5};
    s16 motionUsed[11];
    s32 playerIndex;
    s32 motionIndex;
    s16 selectedPlayer;
    s32 listIndex;
    s32 animationIndex;
    s16 selectionValue;
    s16 questionType;
    s32 textureSize;

    lbl_1_bss_10A.characterModelsCreated = 0;

    for (playerIndex = 0; playerIndex < 5; playerIndex++) {
        positionUsed[playerIndex] = 0;
        firstModelRemaining[playerIndex] = 0xF;
        secondModelRemaining[playerIndex] = 0xF;
    }
    firstModelRemaining[5] = 0xF;

    selectionValue = frandmod(6);
    firstModelRemaining[selectionValue] = firstModelRemaining[(selectionValue + 2) % 6] =
        firstModelRemaining[(selectionValue + 4) % 6] = 0;
    selectionValue = frandmod(5);
    secondModelRemaining[selectionValue] = secondModelRemaining[(selectionValue + 3) % 5] = 0;

    for (playerIndex = 0; playerIndex < 10; playerIndex++) {
        motionUsed[playerIndex] = 0;
    }
    motionUsed[lbl_1_bss_F4.charNo] = 1;
    if (lbl_1_bss_F4.charNo == 0) {
        motionUsed[4] = 1;
    } else if (lbl_1_bss_F4.charNo == 4) {
        motionUsed[0] = 1;
    }
    motionUsed[8] = 1;

    for (playerIndex = 0; playerIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]; playerIndex++) {
        lbl_1_bss_128[playerIndex].motionStep = 0;
        do {
            selectionValue = frandmod(5);
        } while (positionUsed[selectionValue] != 0);
        positionUsed[selectionValue] = 1;
        lbl_1_bss_128[playerIndex].positionIndex = selectionValue;

        for (;;) {
            if ((u32)frandmod(0xA) < playerTypeCounts[lbl_1_bss_10A.mode] &&
                lbl_1_bss_10A.characterModelsCreated < 3) {
                selectionValue = frandmod(0xB);
                if (motionUsed[selectionValue] != 0) {
                    continue;
                }
                motionUsed[selectionValue] = 1;
                lbl_1_bss_10A.characterModelsCreated++;
            } else {
                selectionValue = -1;
            }
            break;
        }
        lbl_1_bss_128[playerIndex].characterId = selectionValue;

        if (selectionValue != -1) {
            lbl_1_bss_128[playerIndex].characterModelId = fn_80057068(selectionValue, 8);
            for (motionIndex = 0; motionIndex < 9; motionIndex++) {
                lbl_1_bss_128[playerIndex].characterMotions[motionIndex] =
                    fn_8005B0B8(selectionValue, lbl_1_data_4B8[motionIndex]);
            }
            fn_8005C2C8(lbl_1_bss_128[playerIndex].characterId,
                               lbl_1_bss_128[playerIndex].characterMotions[0],
                               0.0f, 0.0f, ((1 << 30) | (1 << 0)));
            fn_800269E8(lbl_1_bss_128[playerIndex].characterModelId, 1);
            fn_80026E44(lbl_1_bss_128[playerIndex].characterModelId);
            fn_800097F4(lbl_1_data_4DC[lbl_1_bss_128[playerIndex].characterId]);
            fn_800097F4(lbl_1_data_508[lbl_1_bss_128[playerIndex].characterId]);
            fn_800097F4(lbl_1_data_534[lbl_1_bss_128[playerIndex].characterId]);
        } else {
            lbl_1_bss_128[playerIndex].characterModelId = -1;
        }

        do {
            selectionValue = frandmod(6);
        } while (firstModelRemaining[selectionValue] == 0);
        lbl_1_bss_128[playerIndex].accessorySetIndex = selectionValue;
        do {
            selectionValue = frandmod(3) + 1;
        } while (firstModelRemaining[lbl_1_bss_128[playerIndex].accessorySetIndex] - selectionValue
                 < 0);
        firstModelRemaining[lbl_1_bss_128[playerIndex].accessorySetIndex] -= selectionValue;
        lbl_1_bss_128[playerIndex].accessoryCount = selectionValue;

        do {
            selectionValue = frandmod(5);
        } while (secondModelRemaining[selectionValue] <= 0);
        lbl_1_bss_128[playerIndex].secondarySetIndex = selectionValue;
        do {
            selectionValue = frandmod(3) + 1;
        } while (secondModelRemaining[lbl_1_bss_128[playerIndex].secondarySetIndex] - selectionValue
                 < 0);
        secondModelRemaining[lbl_1_bss_128[playerIndex].secondarySetIndex] -= selectionValue;
        lbl_1_bss_128[playerIndex].secondaryCount = selectionValue;
        OSReport("house:%d\n", lbl_1_bss_128[playerIndex].positionIndex);
        OSReport("player:%d\n", lbl_1_bss_128[playerIndex].characterId);
        OSReport("npc:%d %d\n", lbl_1_bss_128[playerIndex].accessorySetIndex,
                 lbl_1_bss_128[playerIndex].accessoryCount);
        OSReport("item:%d %d\n", lbl_1_bss_128[playerIndex].secondarySetIndex,
                 lbl_1_bss_128[playerIndex].secondaryCount);
    }

    memset(&lbl_1_bss_D4, 0, 0x20U);
    selectedPlayer = frandmod(lbl_1_data_3FC[lbl_1_bss_10A.mode]);
    do {
        questionType = frandmod(3);
        if (questionType == 0 && lbl_1_bss_128[selectedPlayer].characterId != -1 &&
            lbl_1_bss_128[selectedPlayer].characterId != 8) {
            break;
        }
    } while (questionType == 0);
    lbl_1_bss_D4.playerSlot = selectedPlayer;
    lbl_1_bss_D4.phase = questionType;
    switch (questionType) {
    case 0:
        lbl_1_bss_D4.playerIndex = lbl_1_bss_128[selectedPlayer].positionIndex;
        lbl_1_bss_D4.modelIndex = lbl_1_bss_128[selectedPlayer].characterId;
        lbl_1_bss_D4.responseMode = 3;
        MicQuizExpectedAnswerSet(lbl_1_bss_128[selectedPlayer].characterId);
        break;
    case 1:
        lbl_1_bss_D4.playerIndex = lbl_1_bss_128[selectedPlayer].secondarySetIndex;
        lbl_1_bss_D4.modelIndex = 0;
        for (listIndex = 0; listIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]; listIndex++) {
            if (lbl_1_bss_128[listIndex].secondarySetIndex
                == lbl_1_bss_128[selectedPlayer].secondarySetIndex) {
                lbl_1_bss_D4.modelIndex += lbl_1_bss_128[listIndex].secondaryCount;
            }
        }
        lbl_1_bss_D4.responseMode = 1;
        MicQuizExpectedAnswerSet(lbl_1_bss_D4.modelIndex);
        break;
    case 2:
        lbl_1_bss_D4.playerIndex = lbl_1_bss_128[selectedPlayer].accessorySetIndex;
        lbl_1_bss_D4.modelIndex = 0;
        for (listIndex = 0; listIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]; listIndex++) {
            if (lbl_1_bss_128[listIndex].accessorySetIndex
                == lbl_1_bss_128[selectedPlayer].accessorySetIndex) {
                lbl_1_bss_D4.modelIndex += lbl_1_bss_128[listIndex].accessoryCount;
            }
        }
        lbl_1_bss_D4.responseMode = 0;
        MicQuizExpectedAnswerSet(lbl_1_bss_D4.modelIndex);
        break;
    }

    textureSize = GXGetTexBufferSize(0xC0, 0xA4, 6, 0, 0);
    for (animationIndex = 0; animationIndex
         < lbl_1_data_3FC[lbl_1_bss_10A.mode]; animationIndex++) {
        s16 modelId = fn_80025A04(fn_80008DD0(DATANUM(0xa50000, 49), 0x10000000, 2));
        fn_80026760(modelId, 0.0f, 1000.0f, -30.0f);
        fn_800269E8(modelId, 1);
        lbl_1_bss_54.modelIds[animationIndex] = modelId;
        lbl_1_bss_54.states[animationIndex] = 0;
        fn_8000C200(lbl_1_bss_54.spriteAnimations[animationIndex] = HuSprAnimMake(0xC0, 0xA4, 0),
                          0x10000000);
        lbl_1_bss_54.spriteAnimations[animationIndex]->bmp->palData = NULL;
        lbl_1_bss_54.spriteAnimations[animationIndex]->bmp->palNum = 0;
        lbl_1_bss_54.spriteAnimations[animationIndex]->bmp->data =
            HuMemDirectMallocNum(2, textureSize, 0x10000000);
        lbl_1_bss_54.animationIds[animationIndex] =
            fn_80030B2C(lbl_1_bss_54.spriteAnimations[animationIndex], modelId, lbl_1_data_5F8);
    }
    fn_8002583C(2, 0, fn_1_829C);
    lbl_1_bss_54.currentModel = lbl_1_bss_54.frame = 0;
}

// Builds one house of the parade off-screen to the right: the house model (plus an extra model
// for house type 2 and a motion for type 3), its row of npcs and its row of items at random
// spacing.
void fn_1_2B14(s16 playerIndex) {
    Point3d modelPosition;
    MicquizPlayerWork *player;
    s32 modelIndex;
    s32 setIndex;
    s16 jointMotionId;
    s32 modelCount;
    s16 startX;
    s16 modelId;

    player = &lbl_1_bss_128[playerIndex];
    for (modelIndex = 0; modelIndex < 5; modelIndex++) {
        player->models_28[modelIndex] = -1;
        player->models_32[modelIndex] = -1;
    }

    modelId = fn_80025A04(
        fn_80008DD0(lbl_1_data_454[player->positionIndex],
                             0x10000000, 2));
    modelPosition.x = 800.0f;
    modelPosition.y = 15.0f +
                      lbl_1_data_388[player->positionIndex];
    modelPosition.z = -650.0f;
    fn_80026790(modelId, &modelPosition);
    fn_8002D86C(modelId, 0.0f);
    fn_80026E44(modelId);
    player->modelId = modelId;

    if (player->positionIndex == 2) {
        modelId = fn_80025A04(
            fn_80008DD0(DATANUM(0xa50000, 53), 0x10000000, 2));
        modelPosition.y += 200.0f;
        fn_80026790(modelId, &modelPosition);
        player->alternateModelId = modelId;
    } else {
        player->alternateModelId = -1;
    }

    if (player->positionIndex == 3) {
        jointMotionId = fn_80030458(
            player->modelId,
            fn_80008DD0(DATANUM(0xa50000, 55), 0x10000000, 2));
        fn_8002C95C(player->modelId, jointMotionId);
        fn_8002D86C(player->modelId, 0.0f);
    } else {
        player->motionId = -1;
    }

    modelCount = player->accessoryCount;
    setIndex = player->accessorySetIndex;
    startX = frandmod(0x64) + 0x3E8;
    for (modelIndex = 0; modelIndex < modelCount; modelIndex++) {
        s32 jointMotionIndex;

        if (lbl_1_bss_98[setIndex].modelId == -1) {
            modelId = fn_80025A04(
                fn_80008DD0(lbl_1_data_424[setIndex],
                                     0x10000000, 2));
            lbl_1_bss_98[setIndex].modelId = modelId;
            for (jointMotionIndex = 0; jointMotionIndex < 4;
                 jointMotionIndex++) {
                jointMotionId = fn_80030458(
                    modelId,
                    fn_80008DD0(
                        lbl_1_data_424[setIndex] + 1 + jointMotionIndex,
                        0x10000000, 2));
                lbl_1_bss_98[setIndex].jointMotions[jointMotionIndex] =
                    jointMotionId;
            }
        } else {
            modelId = fn_80025D4C(
                lbl_1_bss_98[setIndex].modelId);
        }

        if (setIndex == 5) {
            fn_800268B0(modelId, 0.5f,
                              0.5f, 0.5f);
        }
        player->models_28[modelIndex] = modelId;
        fn_8002C95C(modelId,
                      lbl_1_bss_98[setIndex].jointMotions[2]);
        fn_800269E8(modelId, ((1 << 30) | (1 << 0)));
        fn_80026760(
            modelId, (f32)startX +
                (80.0f * (f32)modelIndex) +
                    (f32)(u32)frandmod(0x32),
            15.0f,
            -200.0f - (f32)((modelIndex & 1) * 0xC8));
        fn_80026808(modelId, 0.0f,
                        270.0f, 0.0f);
        fn_80026E44(modelId);
    }

    modelCount = player->secondaryCount;
    setIndex = player->secondarySetIndex;
    if (player->positionIndex == 0) {
        startX = 900.0f + (u32)frandmod(0xC8);
    } else {
        startX = 850.0f + (u32)frandmod(0xFA);
    }
    for (modelIndex = 0; modelIndex < modelCount; modelIndex++) {
        s16 verticalPosition;
        s16 zPosition;

        if (setIndex < 2) {
            verticalPosition = 60.0f + (u32)frandmod(0x64);
        } else {
            verticalPosition = 0xF;
        }

        if (lbl_1_bss_48[setIndex] == -1) {
            modelId = fn_80025A04(
                fn_80008DD0(lbl_1_data_468[setIndex],
                                     0x10000000, 2));
            lbl_1_bss_48[setIndex] = modelId;
        } else {
            modelId = fn_80025D4C(
                lbl_1_bss_48[setIndex]);
        }

        player->models_32[modelIndex] = modelId;
        zPosition = (frandmod(5) * 0x14) - 0x258;
        fn_80026760(
            modelId, (f32)(startX + modelIndex * 0x64 +
                          (u32)frandmod(0xA)),
            (f32)verticalPosition, (f32)zPosition);
        fn_80026E44(modelId);
    }
}

// Starts a house scrolling if it is not running yet: builds its models and launches fn_1_3258.
void fn_1_31CC(s16 playerIndex) {
    if (lbl_1_bss_2A4[playerIndex] == 0) {
        fn_1_2B14(playerIndex);
        lbl_1_bss_2A4[playerIndex] = fn_8000DAC0(fn_1_3258, 0x64U, 0x1800U, 0, lbl_1_bss_2B8[0]);
    }
}char lbl_1_data_5F8[12] = "v75G_hint";

// Process for the passing house: every frame scrolls the house, items and npcs left, freezes the
// snapshot when the asked-about part reaches its mark, and plays the guest's routine for that house
// type.
void fn_1_3258(void) {
    Point3d position;
    Point3d effectPosition;
    Point3d secondPosition;
    MicquizPlayerWork *player;
    s32 modelIndex;
    f32 angle;

    player = &lbl_1_bss_128[lbl_1_bss_10A.currentPlayer];
    for (;;) {
        for (modelIndex = 0; modelIndex < 5; modelIndex++) {
            if (player->models_32[modelIndex] != -1) {
                fn_800267CC(player->models_32[modelIndex], &position);
                position.x -= 3.6f;
                fn_80026790(player->models_32[modelIndex], &position);
            }
        }
        for (modelIndex = 0; modelIndex < 5; modelIndex++) {
            if (player->models_28[modelIndex] != -1) {
                fn_800267CC(player->models_28[modelIndex], &position);
                position.x -= 7.2f;
                fn_80026790(player->models_28[modelIndex], &position);
            }
        }
        fn_800267CC(player->modelId, &position);
        position.x -= 3.6f;
        fn_80026790(player->modelId, &position);

        if (lbl_1_bss_54.playerStates[lbl_1_bss_10A.currentPlayer] == 0) {
            switch (lbl_1_bss_D4.phase) {
            case 0:
                if (lbl_1_bss_128[lbl_1_bss_10A.currentPlayer].positionIndex == 3) {
                    if (position.x <= 250.0f) {
                        fn_1_8280(lbl_1_bss_10A.currentPlayer);
                    }
                } else if (position.x <= -100.0f) {
                    fn_1_8280(lbl_1_bss_10A.currentPlayer);
                }
                break;
            case 1:
                fn_800267CC(player->models_32[player->secondaryCount / 2],
                                &secondPosition);
                if (secondPosition.x <= 200.0f) {
                    fn_1_8280(lbl_1_bss_10A.currentPlayer);
                }
                break;
            case 2:
                fn_800267CC(player->models_28[player->accessoryCount / 2],
                                &secondPosition);
                if (secondPosition.x <= 150.0f) {
                    fn_1_8280(lbl_1_bss_10A.currentPlayer);
                }
                break;
            }
        }
        if (player->characterModelId != -1) {
            switch (player->positionIndex) {
            case 0:
                position.z -= 50.0f;
                switch (player->motionStep) {
                case 0:
                    fn_80026A44(player->characterModelId, 1);
                    fn_8005C2C8(player->characterId, player->characterMotions[0],
                                       0.0f, 0.0f, ((1 << 30) | (1 << 0)));
                    player->motionStep++;
                    break;
                case 1:
                    if (position.x < 300.0f) {
                        fn_8002D86C(player->modelId, 1.0f);
                        HuAudFXPlay(0x524);
                        player->motionStep++;
                    }
                    break;
                case 2:
                    if (position.x < 150.0f) {
                        fn_8005C2C8(player->characterId, player->characterMotions[3],
                                           0.0f, 8.0f, 0);
                        player->motionStep++;
                    }
                    break;
                }
                break;
            case 1:
                position.y += player->motionValueA + player->motionValueD;
                position.z += -100.0f + player->motionValueB;
                switch (player->motionStep) {
                case 0:
                    fn_80026A44(player->characterModelId, 1);
                    if (position.x < 500.0f) {
                        player->motionStep++;
                    }
                    break;
                case 1:
                    player->motionValueA += 4.0f;
                    if (player->motionValueA >= lbl_1_data_560[player->characterId]) {
                        HuAudFXPlay(0x526);
                        player->motionStep++;
                    }
                    break;
                case 2:
                    player->motionValueC += 8.0f;
                    fn_80026808(player->characterModelId, 0.0f,
                                    (f32)(40.0 * sin(M_PI *
                                        (f64)player->motionValueC / 180.0)), 0.0f);
                    if (player->motionValueC >= 360.0f) {
                        player->motionStep++;
                        player->motionValueC = 0.0f;
                    }
                    break;
                case 3:
                    player->motionValueA -= 8.0f;
                    if (player->motionValueA <= 0.0f) {
                        fn_8005C2C8(player->characterId, player->characterMotions[5],
                                           0.0f, 8.0f, 0);
                        player->motionStep++;
                    }
                    break;
                case 4:
                    player->motionValueC += 1.0f;
                    player->motionValueA += 6.6666665f;
                    player->motionValueB += 3.3333333f;
                    player->motionValueD = (f32)(160.0 * sin(
                        M_PI * (f64)(6.0f * player->motionValueC) / 180.0));
                    if (player->motionValueC >= 30.0f) {
                        fn_8005C2C8(player->characterId, player->characterMotions[6],
                                           0.0f, 8.0f, 0);
                        player->motionStep++;
                    }
                    break;
                case 5:
                    if (fn_8005C1FC(player->characterId) <=
                        fn_8005C1B8(player->characterId)) {
                        fn_8005C2C8(player->characterId, player->characterMotions[0],
                                           0.0f, 8.0f, ((1 << 30) | (1 << 0)));
                        player->motionStep++;
                    }
                    break;
                }
                break;
            case 2:
                fn_80026760(player->alternateModelId, position.x,
                                330.0f + (position.y + player->motionValueA),
                                position.z);
                position.y += 100.0f;
                switch (player->motionStep) {
                case 0:
                    fn_80026A44(player->characterModelId, 1);
                    if (position.x < 200.0f) {
                        fn_8005C2C8(player->characterId, player->characterMotions[7],
                                           0.0f, 8.0f, 0);
                        player->motionStep++;
                    }
                    break;
                case 1:
                    if ((s16)fn_8005C750(player->characterId) != 0) {
                        HuAudFXPlay(0x522);
                        player->motionStep++;
                    }
                    break;
                case 2:
                    player->motionValueA += 4.0f;
                    if (player->motionValueA > 40.0f) {
                        player->motionStep++;
                    }
                    break;
                case 3:
                    player->motionValueA -= 4.0f;
                    if (player->motionValueA <= 0.0f) {
                        player->motionValueA = 0.0f;
                        player->motionValueB =
                            (f32)(u32)(frandmod(0x3C) + 0x3C);
                        fn_8005C2C8(player->characterId, player->characterMotions[0],
                                           0.0f, 8.0f, ((1 << 30) | (1 << 0)));
                        player->motionStep++;
                    }
                    break;
                case 4:
                    player->motionValueB -= 1.0f;
                    if (player->motionValueB <= 0.0f) {
                        fn_8005C2C8(player->characterId, player->characterMotions[7],
                                           0.0f, 8.0f, 0);
                        player->motionStep = 1;
                    }
                    break;
                }
                break;
            case 3:
                position.x += player->motionValueA;
                position.y += 130.0f + player->motionValueB;
                position.z -= 100.0f;
                switch (player->motionStep) {
                case 0:
                    if (position.x < 450.0f) {
                        fn_8002D86C(player->modelId, 8.0f);
                        player->motionStep++;
                    }
                    break;
                case 1:
                    if (fn_8002D4E0(player->modelId) >= 200.0f) {
                        fn_80026A44(player->characterModelId, 1);
                        fn_8005C2C8(player->characterId, player->characterMotions[8],
                                           0.0f, 0.0f, ((1 << 30) | (1 << 0)));
                        fn_80026808(player->characterModelId, 90.0f, 270.0f, 0.0f);
                        player->motionStep++;
                    }
                    break;
                case 2:
                    player->motionValueA -= 9.0f;
                    player->motionValueB += 2.3f;
                    if (player->motionValueA < -30.0f) {
                        fn_800267CC(player->modelId, &effectPosition);
                        effectPosition.x -= 175.0f;
                        effectPosition.y = 220.0f + effectPosition.y;
                        effectPosition.z = 50.0f + effectPosition.z;
                        fn_8005CF90(-1, &effectPosition, 2.0f);
                        HuAudFXPlay(0x523);
                        player->motionStep++;
                    }
                    break;
                case 3:
                    player->motionValueA -= 9.0f;
                    player->motionValueB += 2.3f;
                    if (position.x < -800.0f) {
                        player->motionStep++;
                    }
                    break;
                }
                break;
            case 4:
                switch (player->motionStep) {
                    case 0:
                        position.y = 660.0f;
                        position.z -= 30.0f;
                        if (position.x < 100.0f) {
                            fn_8005C2C8(player->characterId, player->characterMotions[5],
                                               0.0f, 0.0f, 0);
                            fn_80026A44(player->characterModelId, 1);
                            player->motionValueA = player->motionValueB =
                                player->motionValueC = player->motionValueD = 0.0f;
                            player->motionStep++;
                        }
                        break;
                    case 1:
                        player->motionValueB += 2.0f;
                        player->motionValueA += player->motionValueB;
                        if (player->motionValueA >= 470.0f) {
                            fn_8005C2C8(player->characterId, player->characterMotions[6],
                                               0.0f, 8.0f, 0);
                            HuAudFXPlay(0x525);
                            player->motionValueA = 470.0f;
                            player->motionStep++;
                        }
                        position.y = (710.0f - player->motionValueA) - 50.0f;
                        position.z -= 30.0f;
                        break;
                    case 2:
                        position.y = 190.0f;
                        position.z -= 30.0f;
                        if (fn_8005C1FC(player->characterId) <=
                            fn_8005C1B8(player->characterId)) {
                            fn_8005C2C8(player->characterId, player->characterMotions[0],
                                               0.0f, 8.0f, ((1 << 30) | (1 << 0)));
                            player->motionStep++;
                            player->motionValueC = 4.0f;
                            player->motionValueD = 0.0f;
                        }
                        break;
                    case 3:
                        position.y = (f32)((f64)position.y +
                            ((f64)player->motionValueC * sin(
                                M_PI * (f64)player->motionValueD / 180.0)));
                        fn_80026790(player->modelId, &position);
                        position.y += 80.0f;
                        position.z -= 30.0f;
                        player->motionValueD += 8.0f;
                        player->motionValueC -= 0.04f;
                        if (player->motionValueC < 0.0f) {
                            fn_8005C2C8(player->characterId, player->characterMotions[5],
                                               0.0f, 8.0f, 0);
                            player->motionValueA = 16.0f;
                            player->motionValueB = 0.0f;
                            player->motionValueC = 4.0f;
                            player->motionValueD = 180.0f;
                            player->motionStep++;
                        }
                        break;
                    case 4:
                        position.y = (f32)((f64)position.y +
                            ((f64)player->motionValueC * sin(
                                M_PI * (f64)player->motionValueD / 180.0)));
                        fn_80026790(player->modelId, &position);
                        player->motionValueD += 8.0f;
                        player->motionValueC -= 0.04f;
                        player->motionValueB += player->motionValueA;
                        player->motionValueA -= 1.0f;
                        if ((player->motionValueB <= 0.0f) &&
                            (player->motionValueA < 0.0f)) {
                            fn_8005C2C8(player->characterId, player->characterMotions[6],
                                               0.0f, 8.0f, 0);
                            player->motionValueB = 0.0f;
                            player->motionStep++;
                        }
                        position.y = 190.0f + player->motionValueB;
                        position.z -= 30.0f;
                        break;
                    case 5:
                        if (fn_8005C1FC(player->characterId) <=
                            fn_8005C1B8(player->characterId)) {
                            fn_8005C2C8(player->characterId, player->characterMotions[0],
                                               0.0f, 8.0f, ((1 << 30) | (1 << 0)));
                            player->motionStep++;
                        }
                        position.y = (f32)((f64)position.y +
                            ((f64)player->motionValueC * sin(
                                M_PI * (f64)player->motionValueD / 180.0)));
                        fn_80026790(player->modelId, &position);
                        player->motionValueD += 8.0f;
                        player->motionValueC -= 0.04f;
                        if (player->motionValueC < 0.0f) {
                            player->motionValueC = 0.0f;
                        }
                        position.y += 80.0f;
                        position.z -= 30.0f;
                        break;
                    case 6:
                        position.y = (f32)((f64)position.y +
                            ((f64)player->motionValueC * sin(
                                M_PI * (f64)player->motionValueD / 180.0)));
                        fn_80026790(player->modelId, &position);
                        player->motionValueD += 8.0f;
                        player->motionValueC -= 0.04f;
                        if (player->motionValueC < 0.0f) {
                            player->motionValueC = 0.0f;
                        }
                        position.y += 80.0f;
                        position.z -= 30.0f;
                        break;
                }
                break;
            }
            fn_80026790(player->characterModelId, &position);
        } else if (player->positionIndex == 2) {
            fn_800267CC(player->modelId, &position);
            position.y += 330.0f;
            fn_80026790(player->alternateModelId, &position);
        }
        HuPrcVSleep();
    }
}

// Returns 1 once a house and all its npcs and items have scrolled off the left edge, hiding the
// house and stopping its process; returns 0 while any of them is still on screen.
s32 fn_1_45B0(s16 playerIndex) {
    Point3d position;
    MicquizPlayerModels *player;
    s32 modelIndex;

    player = &lbl_1_bss_128[playerIndex];
    fn_800267CC(player->modelId, &position);
    if (position.x > -900.0f) {
        return 0;
    }
    fn_800269E8(player->modelId, 1);
    modelIndex = 0;
    while (modelIndex < 5) {
        if (player->models_32[modelIndex] != -1) {
            fn_800267CC(player->models_32[modelIndex], &position);
            if (position.x > -700.0f) {
                return 0;
            }
        }
        modelIndex += 1;
    }
    modelIndex = 0;
    while (modelIndex < 5) {
        if (player->models_28[modelIndex] != -1) {
            fn_800267CC(player->models_28[modelIndex], &position);
            if (position.x > -700.0f) {
                return 0;
            }
        }
        modelIndex += 1;
    }
    HuPrcKill(lbl_1_bss_2A4[playerIndex]);
    lbl_1_bss_2A4[playerIndex] = 0;
    return 1;
}

// Returns 1 once a house and all its npcs and items have scrolled off the left edge, hiding each
// of them and stopping the house's process; returns 0 while any is still on screen.
s32 fn_1_4728(s16 playerIndex) {
    Point3d position;
    MicquizPlayerModels *player;
    s32 modelIndex;

    player = &lbl_1_bss_128[playerIndex];
    fn_800267CC(player->modelId, &position);
    if (position.x > -900.0f) {
        return 0;
    }
    fn_800269E8(player->modelId, 1);
    modelIndex = 0;
    while (modelIndex < 5) {
        if (player->models_32[modelIndex] != -1) {
            fn_800267CC(player->models_32[modelIndex], &position);
            if (position.x > -700.0f) {
                return 0;
            }
            fn_800269E8(player->models_32[modelIndex], 1);
        }
        modelIndex += 1;
    }
    modelIndex = 0;
    while (modelIndex < 5) {
        if (player->models_28[modelIndex] != -1) {
            fn_800267CC(player->models_28[modelIndex], &position);
            if (position.x > -700.0f) {
                return 0;
            }
            fn_800269E8(player->models_28[modelIndex], 1);
        }
        modelIndex += 1;
    }
    HuPrcKill(lbl_1_bss_2A4[playerIndex]);
    lbl_1_bss_2A4[playerIndex] = 0;
    return 1;
}

// Sets up the answer reveal: brings the asked house back to the center with its guest, or moves
// the host off to the left and lays out every item of the asked type in rings, or every npc in
// rows.
void fn_1_48C8(void) {
    MicquizPlayerModels *player;
    Point3d position;
    s32 playerIndex;
    s32 modelIndex;
    s32 modelListCount;
    f32 radius;
    f32 angle;
    f32 angleStep;

    player = &lbl_1_bss_128[lbl_1_bss_D4.playerSlot];
    switch (lbl_1_bss_D4.phase) {
    case 0:
        position.x = 0.0f;
        position.y = 15.0f +
                     lbl_1_data_388[player->positionIndex];
        position.z = -650.0f;
        fn_80026790(player->modelId, &position);
        fn_8002D414(player->modelId, 0.0f);
        fn_8002D86C(player->modelId, 0.0f);
        fn_8005BE80(player->characterId, player->characterMotions[0]);
        switch (player->positionIndex) {
        case 0:
            position.z -= 50.0f;
            break;
        case 1:
            position.z -= 50.0f;
            break;
        case 2:
            fn_80026760(player->alternateModelId, position.x,
                            330.0f + position.y, position.z);
            position.y += 100.0f;
            break;
        case 3:
            position.y += 100.0f;
            position.z -= 100.0f;
            break;
        case 4:
            position.y += 80.0f;
            break;
        }
        fn_80026A44(player->modelId, 1);
        fn_80026790(player->characterModelId, &position);
        return;
    case 1:
        fn_80026760(lbl_1_bss_F4.modelId, -800.0f,
                        15.0f, -350.0f);
        fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[1],
                           0.0f, 0.0f,
                           0x40000001U);
        radius = 150.0f;
        angle = 0.0f;
        if (lbl_1_bss_D4.modelIndex < 3) {
            angleStep = 180.0f;
        } else if (lbl_1_bss_D4.modelIndex == 3) {
            angleStep = 120.0f;
        } else if (lbl_1_bss_D4.modelIndex == 4) {
            angleStep = 90.0f;
        } else if (lbl_1_bss_D4.modelIndex == 5) {
            angleStep = 72.0f;
        } else {
            angleStep = 60.0f;
        }
        for (playerIndex = 0; playerIndex < 20; playerIndex++) {
            lbl_1_bss_20[playerIndex] = -1;
        }
        playerIndex = 0;
        modelListCount = 0;
        for (;
             playerIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode];
             playerIndex++) {
            if (lbl_1_bss_D4.playerIndex ==
                lbl_1_bss_128[playerIndex].secondarySetIndex) {
                for (modelIndex = 0;
                     modelIndex < lbl_1_bss_128[playerIndex].secondaryCount;
                     modelIndex++) {
                    lbl_1_bss_20[modelListCount++] =
                        lbl_1_bss_128[playerIndex].models_32[modelIndex];
                    fn_800269E8(
                        lbl_1_bss_128[playerIndex].models_32[modelIndex], 1);
                    fn_800267CC(
                        lbl_1_bss_128[playerIndex].models_32[modelIndex],
                        &position);
                    position.x = (f32)(radius *
                        sin((M_PI *
                             (f64)(30.0f + angle)) /
                            180.0));
                    if (lbl_1_bss_D4.playerIndex <= 1) {
                        position.y = (f32)(80.0 -
                            (30.0 * cos((M_PI *
                                (f64)(30.0f + angle)) /
                                180.0)));
                    }
                    position.z = (f32)(-350.0 +
                        (radius * cos((M_PI *
                            (f64)(30.0f + angle)) /
                            180.0)));
                    fn_80026790(
                        lbl_1_bss_128[playerIndex].models_32[modelIndex],
                        &position);
                    angle += angleStep;
                    if (angle >= 360.0f) {
                        angle -= 330.0f;
                        radius += 100.0f;
                    }
                }
            }
        }
        return;
    case 2:
        fn_80026760(lbl_1_bss_F4.modelId, -800.0f,
                        15.0f, -350.0f);
        fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[1],
                           0.0f, 0.0f,
                           0x40000001U);
        for (playerIndex = 0; playerIndex < 20; playerIndex++) {
            lbl_1_bss_20[playerIndex] = -1;
        }
        position.x = 700.0f;
        position.z = -200.0f;
        angleStep = 1000.0f;
        playerIndex = 0;
        modelListCount = 0;
        for (;
             playerIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode];
             playerIndex++) {
            if (lbl_1_bss_D4.playerIndex ==
                lbl_1_bss_128[playerIndex].accessorySetIndex) {
                for (modelIndex = 0;
                     modelIndex < lbl_1_bss_128[playerIndex].accessoryCount;
                     modelIndex++) {
                    lbl_1_bss_20[modelListCount++] =
                        lbl_1_bss_128[playerIndex].models_28[modelIndex];
                    fn_80026760(
                        lbl_1_bss_128[playerIndex].models_28[modelIndex],
                        position.x, 15.0f, position.z);
                    fn_80026A44(
                        lbl_1_bss_128[playerIndex].models_28[modelIndex], 1);
                    position.x += 200.0f;
                    if (position.x > angleStep) {
                        position.x = 600.0f +
                                     (f32)(u32)frandmod(0x64);
                        if (position.z > -350.0f) {
                            position.z = -600.0f;
                        } else {
                            position.z -= 200.0f;
                        }
                        angleStep = 1200.0f;
                    }
                }
            }
        }
    }
}

// For a character question, places the asked house at the center far back (z = -900) with its
// motion stopped; does nothing for the count questions.
void fn_1_5194(void) {
    MicquizPlayerWork *playerWork;

    playerWork = &lbl_1_bss_128[lbl_1_bss_D4.playerSlot];
    switch (lbl_1_bss_D4.phase) {
    case 0:
        fn_80026760(playerWork->modelId, 0.0f,
                        30.0f + lbl_1_data_388[playerWork->positionIndex],
                        -900.0f);
        fn_8002D414(playerWork->modelId, 0.0f);
        fn_8002D86C(playerWork->modelId, 0.0f);
        break;
    case 1:
    case 2:
    default:
        break;
    }
}

// Plays the answer reveal each frame: the guest repeats its routine at the house, or the host walks
// in
// and each item appears with a puff of smoke, or the npcs come in and react; then the result motion
// plays and the game fades out and ends.
void fn_1_5288(void) {
    Point3d position;
    Point3d effectPosition;
    s16 counter;
    s32 index;
    s32 animationIndex;
    s32 secondAnimationIndex;
    s32 thirdAnimationIndex;
    s32 fourthAnimationIndex;
    s32 fifthAnimationIndex;
    s32 sixthAnimationIndex;
    s32 seventhAnimationIndex;
    MicquizPlayerWork *player;

    player = &lbl_1_bss_128[lbl_1_bss_D4.playerSlot];
    switch (lbl_1_bss_D4.phase) {                /* switch 1; irregular */
    case 0:                                         /* switch 1 */
        fn_800267CC(player->modelId, &position);
        switch (lbl_1_bss_D4.playerIndex) {            /* switch 2; irregular */
        case 0:                                     /* switch 2 */
            switch (lbl_1_bss_D4.motionStep) {        /* switch 3; irregular */
            case 0:                                 /* switch 3 */
                if (lbl_1_bss_D4.timer++ > 0x1E) {
                    fn_8002D86C(player->modelId, 1.0f);
                    HuAudFXPlay(0x524);
                    lbl_1_bss_D4.timer = 0;
                    lbl_1_bss_D4.motionStep += 1;
                    return;
                }
                break;
            case 1:                                 /* switch 3 */
                if (lbl_1_bss_D4.timer++ > 0x1E) {
                    micQuizModeWork->presentationBusy = 0;
                    if (lbl_1_bss_10A.completionFlag == 0) {
                        fn_8005C2C8(player->characterId, player->characterMotions[2], 0.0f, 8.0f,
                                    0U);
                    } else {
                        fn_8005C2C8(player->characterId, player->characterMotions[4], 0.0f, 8.0f,
                                    0U);
                    }
                    lbl_1_bss_D4.timer = 0;
                    lbl_1_bss_D4.motionStep += 1;
                    return;
                }
                break;
            case 2:                                 /* switch 3 */
                if (lbl_1_bss_D4.timer++ > 0x1E) {
                    if (fn_8005C1FC(player->characterId) <= fn_8005C1B8(player->characterId)) {
                        fn_1_834C();
                        lbl_1_bss_D4.motionStep += 1;
                        return;
                    }
                }
                break;
            case 3:                                 /* switch 3 */
                if (WipeCheck() == 0) {
for (animationIndex = 0; animationIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]; animationIndex++) {
                        HuSprAnimKill(lbl_1_bss_54.spriteAnimations[animationIndex]);
}
                    HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                    fn_1_1EB4();
                    HuMCListenerKill();
                    HuMCClose();
                    fn_8003A5E4(1, 1);
                    return;
                }
                break;
            }
            break;
        case 1:                                     /* switch 2 */
            switch (lbl_1_bss_D4.motionStep) {        /* switch 4; irregular */
            case 0:                                 /* switch 4 */
                if (lbl_1_bss_D4.timer++ > 0x1E) {
                    fn_8005C2C8(player->characterId, player->characterMotions[5], 0.0f, 8.0f, 0U);
                    lbl_1_bss_D4.motionStep += 1;
                    lbl_1_bss_D4.timer = 0;
                }
                break;
            case 1:                                 /* switch 4 */
                lbl_1_bss_D4.oscillationPhase += 1.0f;
                lbl_1_bss_D4.sequenceValue += 6.6666665f;
                lbl_1_bss_D4.sequenceOffset += 3.3333333f;
                lbl_1_bss_D4.verticalMotion = (f32) (200.0
                        * sin((M_PI * (f64) (6.0f * lbl_1_bss_D4.oscillationPhase)) / 180.0));
                if (lbl_1_bss_D4.oscillationPhase >= 30.0f) {
                    fn_8005C2C8(player->characterId, player->characterMotions[6], 0.0f, 8.0f, 0U);
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 2:                                 /* switch 4 */
                if (fn_8005C1FC(player->characterId) <= fn_8005C1B8(player->characterId)) {
                    micQuizModeWork->presentationBusy = 0;
                    fn_8005C2C8(player->characterId, player->characterMotions[0], 0.0f, 8.0f,
                                0x40000001U);
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 3:                                 /* switch 4 */
                if (lbl_1_bss_D4.timer++ > 0x3C) {
                    fn_1_834C();
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 4:                                 /* switch 4 */
                if (WipeCheck() == 0) {
for (secondAnimationIndex = 0; secondAnimationIndex
     < lbl_1_data_3FC[lbl_1_bss_10A.mode]; secondAnimationIndex++) {
                        HuSprAnimKill(lbl_1_bss_54.spriteAnimations[secondAnimationIndex]);
}
                    HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                    fn_1_1EB4();
                    HuMCListenerKill();
                    HuMCClose();
                    fn_8003A5E4(1, 1);
                }
                break;
            }
        fn_80026760(player->characterModelId, position.x,
                lbl_1_bss_D4.verticalMotion + (position.y + lbl_1_bss_D4.sequenceValue), (position.z
                        + lbl_1_bss_D4.sequenceOffset) - 100.0f);
            return;
        case 2:                                     /* switch 2 */
            switch (lbl_1_bss_D4.motionStep) {        /* switch 5; irregular */
            case 0:                                 /* switch 5 */
                if (lbl_1_bss_D4.timer++ > 0x1E) {
                    fn_8005C2C8(player->characterId, player->characterMotions[7], 0.0f, 8.0f, 0U);
                    lbl_1_bss_D4.timer = 0;
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 1:                                 /* switch 5 */
                if ((s16)fn_8005C750(player->characterId) != 0) {
                    HuAudFXPlay(0x522);
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 2:                                 /* switch 5 */
                lbl_1_bss_D4.sequenceValue += 4.0f;
                if (lbl_1_bss_D4.sequenceValue > 40.0f) {
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 3:                                 /* switch 5 */
                lbl_1_bss_D4.sequenceValue -= 4.0f;
                if (lbl_1_bss_D4.sequenceValue <= 0.0f) {
                    lbl_1_bss_D4.sequenceValue = 0.0f;
                    micQuizModeWork->presentationBusy = 0;
                    fn_8005C2C8(player->characterId, player->characterMotions[0], 0.0f, 8.0f,
                                0x40000001U);
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 4:                                 /* switch 5 */
                if (lbl_1_bss_D4.timer++ > 0x3C) {
                    fn_1_834C();
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 5:                                 /* switch 5 */
                if (WipeCheck() == 0) {
for (thirdAnimationIndex = 0; thirdAnimationIndex
     < lbl_1_data_3FC[lbl_1_bss_10A.mode]; thirdAnimationIndex++) {
                        HuSprAnimKill(lbl_1_bss_54.spriteAnimations[thirdAnimationIndex]);
}
                    HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                    fn_1_1EB4();
                    HuMCListenerKill();
                    HuMCClose();
                    fn_8003A5E4(1, 1);
                }
                break;
            }
        fn_80026760(player->alternateModelId, position.x,
                    330.0f + position.y + lbl_1_bss_D4.sequenceValue, position.z);
            return;
        case 3:                                     /* switch 2 */
            switch (lbl_1_bss_D4.motionStep) {        /* switch 6; irregular */
            case 0:                                 /* switch 6 */
                fn_8002D86C(player->modelId, 8.0f);
                lbl_1_bss_D4.motionStep += 1;
                break;
            case 1:                                 /* switch 6 */
                if (fn_8002D4E0(player->modelId) > 190.0f) {
    effectPosition.x = position.x - 200.0f;
                    effectPosition.y = 200.0f + position.y;
                    effectPosition.z = 10.0f + position.z;
                    fn_8005C2C8(player->characterId, player->characterMotions[8], 0.0f, 8.0f,
                                0x40000001U);
                    fn_8005CF90(-1, &effectPosition, 2.0f);
                    HuAudFXPlay(0x523);
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 2:                                 /* switch 6 */
                lbl_1_bss_D4.sequenceValue -= 9.0f;
                lbl_1_bss_D4.sequenceOffset += 2.5f;
                if (lbl_1_bss_D4.sequenceValue < -200.0f) {
                    micQuizModeWork->presentationBusy = 0;
                }
                if (lbl_1_bss_D4.sequenceValue < -800.0f) {
                    fn_1_834C();
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 3:                                 /* switch 6 */
                if (WipeCheck() == 0) {
for (fourthAnimationIndex = 0; fourthAnimationIndex
     < lbl_1_data_3FC[lbl_1_bss_10A.mode]; fourthAnimationIndex++) {
                        HuSprAnimKill(lbl_1_bss_54.spriteAnimations[fourthAnimationIndex]);
}
                    HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                    fn_1_1EB4();
                    HuMCListenerKill();
                    HuMCClose();
                    fn_8003A5E4(1, 1);
                }
                break;
            }
        fn_80026760(player->characterModelId, position.x + lbl_1_bss_D4.sequenceValue,
                    130.0f + position.y + lbl_1_bss_D4.sequenceOffset, position.z - 100.0f);
            return;
        case 4:                                     /* switch 2 */
            switch (lbl_1_bss_D4.motionStep) {        /* switch 7; irregular */
            case 0:                                 /* switch 7 */
                if (lbl_1_bss_D4.timer++ > 0) {
                    fn_8005C2C8(player->characterId, player->characterMotions[5], 0.0f, 8.0f, 0U);
                    lbl_1_bss_D4.timer = 0;
                    lbl_1_bss_D4.sequenceValue = 16.0f;
                    lbl_1_bss_D4.sequenceOffset = 0.0f;
                    lbl_1_bss_D4.oscillationPhase = 180.0f;
                    lbl_1_bss_D4.verticalMotion = 4.0f;
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 1:                                 /* switch 7 */
                lbl_1_bss_D4.oscillationPhase += 8.0f;
                position.y = (f32) ((f64) position.y
                        + ((f64) lbl_1_bss_D4.verticalMotion
                           * sin((M_PI * (f64) lbl_1_bss_D4.oscillationPhase) / 180.0)));
                lbl_1_bss_D4.verticalMotion -= 0.04f;
                lbl_1_bss_D4.sequenceOffset += lbl_1_bss_D4.sequenceValue;
                lbl_1_bss_D4.sequenceValue -= 1.0f;
                if ((lbl_1_bss_D4.sequenceOffset <= 0.0f) && (lbl_1_bss_D4.sequenceValue < 0.0f)) {
                    fn_8005C2C8(player->characterId, player->characterMotions[6], 0.0f, 8.0f, 0U);
                    HuAudFXPlay(0x525);
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 2:                                 /* switch 7 */
                lbl_1_bss_D4.sequenceOffset = (f32) (70.0f + (position.y - 180.0f));
                lbl_1_bss_D4.oscillationPhase += 8.0f;
                position.y = (f32) ((f64) position.y
                        + ((f64) lbl_1_bss_D4.verticalMotion
                           * sin((M_PI * (f64) lbl_1_bss_D4.oscillationPhase) / 180.0)));
                lbl_1_bss_D4.verticalMotion -= 0.04f;
                if (fn_8005C1FC(player->characterId) <= fn_8005C1B8(player->characterId)) {
                    micQuizModeWork->presentationBusy = 0;
                    fn_8005C2C8(player->characterId, player->characterMotions[0], 0.0f, 8.0f,
                                0x40000001U);
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 3:                                 /* switch 7 */
                lbl_1_bss_D4.sequenceOffset = (f32) (70.0f + (position.y - 180.0f));
                lbl_1_bss_D4.oscillationPhase += 8.0f;
                position.y = (f32) ((f64) position.y
                        + ((f64) lbl_1_bss_D4.verticalMotion
                           * sin((M_PI * (f64) lbl_1_bss_D4.oscillationPhase) / 180.0)));
                lbl_1_bss_D4.verticalMotion -= 0.04f;
                if (lbl_1_bss_D4.timer++ > 0x1E) {
                    fn_1_834C();
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 4:                                 /* switch 7 */
                lbl_1_bss_D4.sequenceOffset = (f32) (70.0f + (position.y - 180.0f));
                lbl_1_bss_D4.oscillationPhase += 8.0f;
                position.y = (f32) ((f64) position.y
                        + ((f64) lbl_1_bss_D4.verticalMotion
                           * sin((M_PI * (f64) lbl_1_bss_D4.oscillationPhase) / 180.0)));
                lbl_1_bss_D4.verticalMotion -= 0.04f;
                if (WipeCheck() == 0) {
for (fifthAnimationIndex = 0; fifthAnimationIndex
     < lbl_1_data_3FC[lbl_1_bss_10A.mode]; fifthAnimationIndex++) {
                        HuSprAnimKill(lbl_1_bss_54.spriteAnimations[fifthAnimationIndex]);
}
                    HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                    fn_1_1EB4();
                    HuMCListenerKill();
                    HuMCClose();
                    fn_8003A5E4(1, 1);
                }
                break;
            }
    fn_80026760(player->modelId, position.x, position.y, position.z);
             fn_80026760(player->characterModelId, position.x, 180.0f + lbl_1_bss_D4.sequenceOffset,
                         position.z - 50.0f);
            return;
        }
        break;
    case 1:                                         /* switch 1 */
        {
            switch (lbl_1_bss_D4.motionStep) {              /* switch 8 */
            case 0:                                 /* switch 8 */
                fn_800267CC(lbl_1_bss_F4.modelId, &position);
    position.x += 3.6f;
                if (position.x >= 0.0f) {
    position.x = 0.0f;
                    lbl_1_bss_D4.motionStep += 1;
                }
                fn_80026790(lbl_1_bss_F4.modelId, &position);
                return;
            case 1:                                 /* switch 8 */
                fn_80026874(lbl_1_bss_F4.modelId, &position);
                position.y -= 5.0f;
                if (position.y <= 0.0f) {
                    fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[0], 0.0f, 8.0f,
                                0x40000001U);
                    position.y = 0.0f;
                    lbl_1_bss_D4.motionStep += 1;
                }
                fn_80026838(lbl_1_bss_F4.modelId, &position);
                return;
            case 2:                                 /* switch 8 */
                lbl_1_bss_D4.sequenceValue += 1.0f;
                if (lbl_1_bss_D4.sequenceValue >= 10.0f) {
                    fn_800267CC(lbl_1_bss_20[lbl_1_bss_D4.timer], &position);
                    position.z += 50.0f;
                    position.y += 50.0f;
                    fn_8005CF90(2, &position, 1.0f);
                    HuAudFXPlay(0x527);
                    OSReport("smoke\n");
                    lbl_1_bss_D4.sequenceValue = 0.0f;
                    lbl_1_bss_D4.timer += 1;
                    if (lbl_1_bss_D4.timer >= lbl_1_bss_D4.modelIndex) {
                        lbl_1_bss_D4.motionStep += 1;
                    }
                }
                if ((lbl_1_bss_D4.sequenceValue == 5.0f) && (lbl_1_bss_D4.timer != 0)) {
                    fn_80026A44(lbl_1_bss_20[lbl_1_bss_D4.timer - 1], 1U);
                    return;
                }
                break;
            case 3:                                 /* switch 8 */
                lbl_1_bss_D4.sequenceValue += 1.0f;
                if (lbl_1_bss_D4.sequenceValue == 5.0f) {
                    if (lbl_1_bss_20[lbl_1_bss_D4.timer - 1] != -1) {
                        fn_80026A44(lbl_1_bss_20[lbl_1_bss_D4.timer - 1], 1U);
                    }
                    micQuizModeWork->presentationBusy = 0;
                    lbl_1_bss_D4.timer = 0;
                    lbl_1_bss_D4.motionStep += 1;
                    return;
                }
                break;
            case 4:                                 /* switch 8 */
                if (lbl_1_bss_D4.timer++ > 0x1E) {
                    if (lbl_1_bss_10A.completionFlag == 0) {
                        fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[2], 0.0f, 8.0f, 0U);
                    } else {
                        fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[4], 0.0f, 8.0f, 0U);
                    }
                    lbl_1_bss_D4.timer = 0;
                    lbl_1_bss_D4.motionStep += 1;
                    return;
                }
                break;
            case 5:                                 /* switch 8 */
                if (fn_8005C1FC(lbl_1_bss_F4.charNo) <= fn_8005C1B8(lbl_1_bss_F4.charNo)) {
                    fn_1_834C();
                    lbl_1_bss_D4.motionStep += 1;
                    return;
                }
                break;
            case 6:                                 /* switch 8 */
                if (WipeCheck() == 0) {
for (sixthAnimationIndex = 0; sixthAnimationIndex
     < lbl_1_data_3FC[lbl_1_bss_10A.mode]; sixthAnimationIndex++) {
                        HuSprAnimKill(lbl_1_bss_54.spriteAnimations[sixthAnimationIndex]);
}
                    HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                    fn_1_1EB4();
                    HuMCListenerKill();
                    HuMCClose();
                    fn_8003A5E4(1, 1);
                    return;
                }
                break;
            }
        }
        break;
    case 2:                                         /* switch 1 */
            switch (lbl_1_bss_D4.motionStep) {              /* switch 9 */
            case 0:                                 /* switch 9 */
                fn_800267CC(lbl_1_bss_F4.modelId, &position);
    position.x += 3.6f;
                if (position.x >= 0.0f) {
    position.x = 0.0f;
                    lbl_1_bss_D4.motionStep += 1;
                }
                fn_80026790(lbl_1_bss_F4.modelId, &position);
for (index = 0; index < lbl_1_bss_D4.modelIndex; index++) {
                    fn_800267CC(lbl_1_bss_20[index], &position);
    position.x -= 3.6f;
                    fn_80026790(lbl_1_bss_20[index], &position);
}
                return;
            case 1:                                 /* switch 9 */
                fn_80026874(lbl_1_bss_F4.modelId, &position);
                position.y -= 5.0f;
                if (position.y <= 0.0f) {
                    fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[0], 0.0f, 8.0f,
                                0x40000001U);
                    position.y = 0.0f;
for (index = 0; index < lbl_1_bss_D4.modelIndex; index++) {
                        fn_8002CCC0(lbl_1_bss_20[index],
                                lbl_1_bss_98[lbl_1_bss_D4.playerIndex].jointMotions[
                                        0], 0.0f, 8.0f, 0x40000001U);
}
                    lbl_1_bss_D4.motionStep += 1;
                }
                fn_80026838(lbl_1_bss_F4.modelId, &position);
for (index = 0; index < lbl_1_bss_D4.modelIndex; index++) {
                    fn_80026874(lbl_1_bss_20[index], &position);
                    position.y += 5.0f;
                    fn_80026838(lbl_1_bss_20[index], &position);
}
                return;
            case 2:                                 /* switch 9 */
                lbl_1_bss_D4.motionStep += 1;
                return;
            case 3:                                 /* switch 9 */
                micQuizModeWork->presentationBusy = 0;
                lbl_1_bss_D4.motionStep += 1;
                return;
            case 4:                                 /* switch 9 */
                if (lbl_1_bss_10A.completionFlag == 0) {
                    fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[2], 0.0f, 8.0f, 0U);
                } else {
                    fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[4], 0.0f, 8.0f, 0U);
                }
for (index = 0; index < lbl_1_bss_D4.modelIndex; index++) {
                    if (lbl_1_bss_10A.completionFlag == 0) {
                        fn_8002CCC0(lbl_1_bss_20[index],
                                lbl_1_bss_98[lbl_1_bss_D4.playerIndex].jointMotions[
                                        3], 0.0f, 8.0f, 0x40000001U);
                        if ((index == 0) && (lbl_1_bss_D4.playerIndex != 4)) {
                            HuAudFXPlay(lbl_1_data_43C[lbl_1_bss_D4.playerIndex]);
                        }
                    } else {
                        fn_8002CCC0(lbl_1_bss_20[index],
                                lbl_1_bss_98[lbl_1_bss_D4.playerIndex].jointMotions[
                                        0], 0.0f, 8.0f, 0x40000001U);
                    }
}
                lbl_1_bss_D4.motionStep += 1;
                return;
            case 5:                                 /* switch 9 */
                if (fn_8005C1FC(lbl_1_bss_F4.charNo) <= fn_8005C1B8(lbl_1_bss_F4.charNo)) {
                    fn_1_834C();
                    lbl_1_bss_D4.motionStep += 1;
                    return;
                }
                break;
            case 6:                                 /* switch 9 */
                if (WipeCheck() == 0) {
for (seventhAnimationIndex = 0; seventhAnimationIndex
     < lbl_1_data_3FC[lbl_1_bss_10A.mode]; seventhAnimationIndex++) {
                        HuSprAnimKill(lbl_1_bss_54.spriteAnimations[seventhAnimationIndex]);
}
                    HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                    fn_1_1EB4();
                    HuMCListenerKill();
                    HuMCClose();
                    fn_8003A5E4(1, 1);
                }
                break;
            }
        break;
    }
}

// Looks up the asked house and checks the question type, but takes no action for any type.
void fn_1_7188(void) {
    MicquizPlayerWork *playerWork;

    playerWork = &lbl_1_bss_128[lbl_1_bss_D4.playerSlot];
    switch (lbl_1_bss_D4.phase) {
    case 0:
    case 1:
    case 2:
        break;
    default:
        break;
    }
}

// Places the reveal scene by question type: the asked house at the center, or the host at the
// center facing front with every item of the asked type in rings, or the host off to the left.
void fn_1_71D0(void) {
    Point3d position;
    MicquizPlayerWork *currentPlayer;

    currentPlayer = &lbl_1_bss_128[lbl_1_bss_D4.playerSlot];

    switch (lbl_1_bss_D4.phase) {
    case 0:
        position.x = 0.0f;
        position.y = 15.0f + lbl_1_data_388[currentPlayer->positionIndex];
        position.z = -650.0f;
        fn_80026790(currentPlayer->modelId, &position);
        fn_8002D414(currentPlayer->modelId, 0.0f);
        fn_8002D86C(currentPlayer->modelId, 0.0f);
        if (currentPlayer->positionIndex == 2) {
            fn_80026760(currentPlayer->alternateModelId, position.x,
                            330.0f + position.y, position.z);
        }
        break;

    case 1: {
        f32 radius;
        f32 angle;
        f32 angleStep;
        s32 modelSetIndex;
        s32 modelIndex;
        s32 modelListCount;

        fn_80026760(lbl_1_bss_F4.modelId, 0.0f,
                        15.0f, -350.0f);
        fn_80026808(lbl_1_bss_F4.modelId, 0.0f,
                        0.0f, 0.0f);
        fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[0],
                           0.0f, 0.0f, 0x40000001U);

        radius = 150.0f;
        angle = 0.0f;
        if (lbl_1_bss_D4.modelIndex < 3) {
            angleStep = 180.0f;
        } else if (lbl_1_bss_D4.modelIndex == 3) {
            angleStep = 120.0f;
        } else if (lbl_1_bss_D4.modelIndex == 4) {
            angleStep = 90.0f;
        } else if (lbl_1_bss_D4.modelIndex == 5) {
            angleStep = 72.0f;
        } else {
            angleStep = 60.0f;
        }

        for (modelSetIndex = 0; modelSetIndex < 20; modelSetIndex++) {
            lbl_1_bss_20[modelSetIndex] = -1;
        }

        modelSetIndex = 0;
        modelListCount = 0;
        for (; modelSetIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode];
             modelSetIndex++) {
            if (lbl_1_bss_D4.playerIndex == lbl_1_bss_128[modelSetIndex].secondarySetIndex) {
                for (modelIndex = 0;
                     modelIndex < lbl_1_bss_128[modelSetIndex].secondaryCount;
                     modelIndex++) {
                    lbl_1_bss_20[modelListCount++] =
                        lbl_1_bss_128[modelSetIndex].models_32[modelIndex];
                    fn_800269E8(
                        lbl_1_bss_128[modelSetIndex].models_32[modelIndex], 1);
                    fn_800267CC(
                        lbl_1_bss_128[modelSetIndex].models_32[modelIndex],
                        &position);
                    position.x = (f32)(radius *
                                       sin((M_PI *
                                            (f64)(30.0f + angle)) /
                                           180.0));
                    position.z = (f32)(-350.0 +
                                       (radius *
                                        cos((M_PI *
                                             (f64)(30.0f + angle)) /
                                            180.0)));
                    fn_80026790(
                        lbl_1_bss_128[modelSetIndex].models_32[modelIndex],
                        &position);
                    angle += angleStep;
                    if (angle >= 360.0f) {
                        angle -= 330.0f;
                        radius += 100.0f;
                    }
                }
            }
        }
        return;
    }

    case 2:
        fn_80026760(lbl_1_bss_F4.modelId, -800.0f,
                        15.0f, -350.0f);
        fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[1],
                           0.0f, 0.0f, 0x40000001U);
        break;
    }
}

// Reveal by question type, then fade-out and exit: restarts the house motion; or the host plays
// motion 4 with its character's voice and the items appear one by one; or the host walks in, lands
// and plays motion 4.
void fn_1_7754(void) {
    MicquizPlayerWork *player;
    Point3d modelTransform;
    f32 motionTime;

    player = &lbl_1_bss_128[lbl_1_bss_D4.playerSlot];
    {
    s32 soundList[11] = { 585, 561, 705, 801, 753, 489, 777, 513, 729, 657, 537 };

    switch (lbl_1_bss_D4.phase) {
    case 0:
        switch (lbl_1_bss_D4.motionStep) {
        case 0:
            fn_8002D86C(player->modelId, 1.0f);
            lbl_1_bss_D4.motionStep++;
            break;
        case 1:
            fn_1_834C();
            lbl_1_bss_D4.motionStep++;
            break;
        case 2: {
            s32 animationIndex;
            if (WipeCheck() == 0) {
                animationIndex = 0;
                while (animationIndex <
                       lbl_1_data_3FC[lbl_1_bss_10A.mode]) {
                    HuSprAnimKill(
                        lbl_1_bss_54.spriteAnimations[animationIndex]);
                    animationIndex++;
                }
                HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                if (micQuizActiveWin != -1) {
                    HuWinExClose(micQuizActiveWin);
                    micQuizActiveWin = -1;
                }
                HuMCListenerKill();
                HuMCClose();
                fn_8003A5E4(1, 1);
            }
            break;
        }
        }
        break;
    case 1:
        switch (lbl_1_bss_D4.motionStep) {
        case 0:
            lbl_1_bss_D4.motionStep++;
            break;
        case 1:
            fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[4],
                               0.0f, 8.0f, 0);
            HuAudFXPlay(soundList[lbl_1_bss_F4.charNo]);
            lbl_1_bss_D4.motionStep++;
            break;
        case 2: {
            motionTime = fn_8005C1B8(lbl_1_bss_F4.charNo);
            if (fn_8005C1FC(lbl_1_bss_F4.charNo) <= motionTime) {
                lbl_1_bss_D4.motionStep++;
            }
            break;
        }
        case 3:
            lbl_1_bss_D4.sequenceValue += 1.0f;
            if (lbl_1_bss_D4.sequenceValue >= 10.0f) {
                fn_80026A44(
                    lbl_1_bss_20[(s16)lbl_1_bss_D4.sequenceOffset], 1);
                lbl_1_bss_D4.sequenceValue = 0.0f;
                lbl_1_bss_D4.sequenceOffset += 1.0f;
                if (lbl_1_bss_D4.sequenceOffset >=
                    (f32)lbl_1_bss_D4.modelIndex) {
                    lbl_1_bss_D4.motionStep++;
                }
            }
            break;
        case 4:
            lbl_1_bss_D4.motionStep++;
            break;
        case 5:
            if (lbl_1_bss_D4.timer++ > 0x1E) {
                fn_1_834C();
                lbl_1_bss_D4.motionStep++;
            }
            break;
        case 6: {
            s32 exitAnimationIndex;
            if (WipeCheck() == 0) {
                exitAnimationIndex = 0;
                while (exitAnimationIndex <
                       lbl_1_data_3FC[lbl_1_bss_10A.mode]) {
                    HuSprAnimKill(
                        lbl_1_bss_54.spriteAnimations[exitAnimationIndex]);
                    exitAnimationIndex++;
                }
                HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                if (micQuizActiveWin != -1) {
                    HuWinExClose(micQuizActiveWin);
                    micQuizActiveWin = -1;
                }
                HuMCListenerKill();
                HuMCClose();
                fn_8003A5E4(1, 1);
            }
            break;
        }
        }
        break;
    case 2:
        switch (lbl_1_bss_D4.motionStep) {
        case 0:
            lbl_1_bss_D4.motionStep++;
            break;
        case 1: {
            fn_800267CC(lbl_1_bss_F4.modelId, &modelTransform);
            modelTransform.x += 3.6f;
            if (modelTransform.x >= 0.0f) {
                modelTransform.x = 0.0f;
                lbl_1_bss_D4.motionStep++;
            }
            fn_80026790(lbl_1_bss_F4.modelId, &modelTransform);
            break;
        }
        case 2: {
            fn_80026874(lbl_1_bss_F4.modelId, &modelTransform);
            modelTransform.y -= 5.0f;
            if (modelTransform.y <= 0.0f) {
                fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[0],
                                   0.0f, 8.0f,
                                   0x40000001U);
                modelTransform.y = 0.0f;
                lbl_1_bss_D4.motionStep++;
            }
            fn_80026838(lbl_1_bss_F4.modelId, &modelTransform);
            break;
        }
        case 3:
            if (lbl_1_bss_D4.timer++ > 0x1E) {
                fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[4],
                                   0.0f, 8.0f, 0);
                lbl_1_bss_D4.motionStep++;
            }
            break;
        case 4: {
            motionTime = fn_8005C1B8(lbl_1_bss_F4.charNo);
            if (fn_8005C1FC(lbl_1_bss_F4.charNo) <= motionTime) {
                fn_1_834C();
                lbl_1_bss_D4.motionStep++;
            }
            break;
        }
        case 5: {
            s32 shutdownAnimationIndex;
            if (WipeCheck() == 0) {
                shutdownAnimationIndex = 0;
                while (shutdownAnimationIndex <
                       lbl_1_data_3FC[lbl_1_bss_10A.mode]) {
                    HuSprAnimKill(
                        lbl_1_bss_54.spriteAnimations[shutdownAnimationIndex]);
                    shutdownAnimationIndex++;
                }
                HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                if (micQuizActiveWin != -1) {
                    HuWinExClose(micQuizActiveWin);
                    micQuizActiveWin = -1;
                }
                HuMCListenerKill();
                HuMCClose();
                fn_8003A5E4(1, 1);
            }
            break;
        }
        }
        break;
    }
    }
}

// Drops the hint boards one after another (180 frames shared among the houses) until each lands
// at y = 30; returns 1 once every board has started falling.
s32 fn_1_7FAC(void) {
    Point3d position;
    s16 frameLimit;
    s32 index;

    frameLimit = (s16)(0xB4 / lbl_1_data_3FC[lbl_1_bss_10A.mode]);
    if (lbl_1_bss_54.frame++ == 0) {
        lbl_1_bss_54.states[lbl_1_bss_54.currentModel] = 1;
        fn_80026A44(lbl_1_bss_54.modelIds[lbl_1_bss_54.currentModel], 1);
    }
    if (lbl_1_bss_54.frame >= frameLimit) {
        lbl_1_bss_54.frame = 0;
        lbl_1_bss_54.currentModel += 1;
        if (lbl_1_bss_54.currentModel >=
            lbl_1_data_3FC[lbl_1_bss_10A.mode]) {
            return 1;
        }
    }
    for (index = 0; index < lbl_1_data_3FC[lbl_1_bss_10A.mode]; index++) {
        if (lbl_1_bss_54.states[index] == 1) {
            fn_800267CC(lbl_1_bss_54.modelIds[index], &position);
            position.y -= (f32)(lbl_1_bss_54.frame * 4);
            if (position.y <= 30.0f) {
                position.y = 30.0f;
                lbl_1_bss_54.states[index] = 2;
            }
            fn_80026790(lbl_1_bss_54.modelIds[index], &position);
        }
    }
    return 0;
}

// Hides all hint boards and calls fn_800258A8(2, 8).
void fn_1_8200(void) {
    s32 modelIndex;

    modelIndex = 0;
    while (modelIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]) {
        fn_800269E8(lbl_1_bss_54.modelIds[modelIndex], 1U);
        modelIndex += 1;
    }
    fn_800258A8(2, 8);
}

void fn_1_8280(s32 modelIndex) {
    lbl_1_bss_54.playerStates[modelIndex] = 1;
}

// Snapshot draw hook: copies the 192x164 frame into the texture of every house whose hint
// has not been frozen yet.
void fn_1_829C(s16 layerNo) {
    s32 modelIndex;

    modelIndex = 0;
    while (modelIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]) {
        if (lbl_1_bss_54.playerStates[modelIndex] == 0) {
            fn_8002BCDC(0, 0, 0xC0, 0xA4, GX_TF_RGBA8, 0,
                        lbl_1_bss_54.spriteAnimations[modelIndex]->bmp->data);
        }
        modelIndex += 1;
    }
    fn_8002BF48();
}

void fn_1_834C(void) {
    HuPrcSleep(0x3C);
    HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
    WipeCreate(2, 0, 0x1E);
}
HUPROCESS *lbl_1_bss_2B8[11];
HUPROCESS *lbl_1_bss_2A4[5];
MicquizPlayerWork lbl_1_bss_128[5];
MicquizControlWork lbl_1_bss_10A;
MicquizHostState lbl_1_bss_F4;
MicquizSequence lbl_1_bss_D4;
MicquizAccessoryPoolEntry lbl_1_bss_98[6];
MicquizAnimationState lbl_1_bss_54;
s16 lbl_1_bss_48[6];
s16 lbl_1_bss_20[0x14];
s32 lbl_1_bss_1C;
f32 micQuizPauseBgScroll;
ANIMDATA *micQuizPauseBgAnim;
HUPROCESS *micQuizMicPulseProc;
HUPROCESS *micQuizIndicatorSwingProc;
MicquizResponse *micQuizResponse;
s32 micQuizTimeoutHandled;
HUPROCESS *micQuizRingProc;
