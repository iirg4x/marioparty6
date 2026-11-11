/* Second counting round: objects under a fading light, the count answer and its check. */
#include "msm_stream.h"
#include "math.h"

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

#include "game/mic.h"

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

#include "dolphin/os.h"

#include "dolphin/types.h"

#include "game/hu3d.h"

#include "game/printfunc.h"
#include "REL/micquizishidll.h"

/* quiz.h leaves these out (other files call them without a prototype); this file's calls take
 * them with one. */
void MicQuizCountdownInit(void);
void MicQuizPauseInit(HUPROCESS *parentProcess);

typedef struct {
    s32 firstValue;
    s32 secondValue;
    s32 thirdValue;
} MicquizBoardEntry;

s16 lbl_1_bss_D4A;
s16 lbl_1_bss_D46[2];
s16 lbl_1_bss_D44;
s32 lbl_1_bss_D40;
u8 lbl_1_bss_D30[0x10];
MicQuizRecord lbl_1_bss_BB0[8][4];
MicquizBoardEntry lbl_1_bss_8B0[8][8];
s32 lbl_1_bss_8AC;
s32 lbl_1_bss_8A8;
s32 lbl_1_bss_8A4;
s32 lbl_1_bss_8A0;

u8 lbl_1_data_778[0x1C] = { 0x00, 0xA4, 0x00, 0x7A, 0x00, 0xA4, 0x00, 0x7B, 0x00, 0xA4, 0x00, 0x7C,
        0x00, 0xA4, 0x00, 0x7D, 0x00, 0xA4, 0x00, 0x7E, 0x00, 0xA4, 0x00, 0x7F, 0x00, 0xA4, 0x00, 0x80 };
s32 lbl_1_data_794[5][3] = { { 1, 3, 5 }, { 1, 5, 10 }, { 2, 10, 15 }, { 3, 10, 15 },
                            { 4, 15, 20 } };
s32 lbl_1_data_7D0[5][3] = { { 99, -1, -1 }, { 99, -1, -1 }, { 49, 99, -1 }, { 39, 69, 99 },
                            { 32, 65, 99 } };
Point3d lbl_1_data_80C = { -100.0f, 8000.0f, -400.0f };
Point3d lbl_1_data_818 = { 0.0f, 1.0f, 0.0f };
Point3d lbl_1_data_824 = { 0.0f, 0.0f, -400.0f };
OM_CAMERA_VIEW lbl_1_data_830 = { { 0.0f, 225.0f, 0.0f }, { -21.6f, 0.0f, 0.0f }, 6389.0f };
u8 lbl_1_data_84C[0x4] = { 0x43, 0xC8, 0x00, 0x00 };
Vec lbl_1_data_850 = { 0.0f, 1000.0f, 50.0f };
Vec lbl_1_data_85C = { 0.0f, 0.0f, -550.0f };
Point3d lbl_1_data_868 = { 0.0f, 300.0f, 500.0f };
Point3d lbl_1_data_874 = { 0.0f, 0.0f, -1000.0f };
u8 lbl_1_data_880[4] = { 0, 0, 0, 0 };
u8 lbl_1_data_884[0x14] = {
    0x44, 0x48, 0x00, 0x00, 0x44, 0x2F, 0x00, 0x00, 0x44, 0x16, 0x00, 0x00, 0x43, 0xFA, 0x00, 0x00,
    0x43, 0xC8, 0x00, 0x00
};
f32 lbl_1_data_898[6] = { 20.0f, 20.0f, 20.0f, 20.0f, 15.0f, 1.0f };

s32 lbl_1_data_8B0 = -1;

s32 fn_1_17974(void);
void fn_1_17A88(void);
void fn_1_17BC8(void);
void fn_1_1833C(OMOBJ *object);
void fn_1_18604(OMOBJ *object);
s32 fn_1_18670(Vec *position, s32 motionType, s32 objectIndex);
void fn_1_18858(void);
void fn_1_18998(void);
s32 fn_1_18A5C(void);
s32 fn_1_18D3C(void);
s32 fn_1_19280(void);
void fn_1_19560(void);

/* Sets up the second counting round: camera, spot light and shadow, the objects and their slots. */
void fn_1_1706C(void) {
    s32 index;
    s32 slot;

    lbl_1_bss_FC = 4;
    lbl_1_bss_8C = 11.363636f;
    micQuizGameMes = -1;
    lbl_1_bss_100 = (s32) MicQuizQuestionLevelGet();
    lbl_1_bss_104 = 0.0f;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 10.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0U, 0U, 0x280U, 0x1E0U);
    fn_8003E488(1, &lbl_1_data_830);
    lbl_1_bss_A8 = fn_8003AB38(*lbl_1_bss_114, 0x7FDA, 0U, 0U, -1, fn_8003DAA4);
    lbl_1_bss_A8->work[0] = 1;
    fn_800285BC();
    lbl_1_bss_D40 = (s16) Hu3DGLightCreate(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0U, 0U, 0U);
    Hu3DGLightPosAimSetV((s16) lbl_1_bss_D40, &lbl_1_data_850, &lbl_1_data_85C);
    Hu3DGLightColorSet((s16) lbl_1_bss_D40, lbl_1_data_880[0], lbl_1_data_880[1], lbl_1_data_880[2],
                       lbl_1_data_880[3]);
    lbl_1_bss_8AC = lbl_1_bss_8A8 = lbl_1_bss_8A4 = lbl_1_bss_8A0 = 0;
    Hu3DGLightSpotSet((s16) lbl_1_bss_D40, lbl_1_data_898[lbl_1_bss_100], 2);
    Hu3DGLightStaticSet((s16) lbl_1_bss_D40, 1);
    Hu3DShadowCreate(20.0f, 10.0f, 8000.0f);
    fn_8002AAB4(0x10U, 0x10U, 0x10U);
    Hu3DShadowPosSet(&lbl_1_data_80C, &lbl_1_data_818, &lbl_1_data_824);
    Hu3DShadowTPLvlSet(0.3f);
    lbl_1_bss_A4 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 34), 0x10000000, 2));
    lbl_1_bss_A2 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 36), 0x10000000, 2));
    fn_80026FFC(lbl_1_bss_A4);
    fn_800273EC(lbl_1_bss_A4, 0.0f, 0.0f, 0.0f);
    fn_800273EC(lbl_1_bss_A2, 0.0f, 0.0f, 0.0f);
    lbl_1_bss_D44 = fn_80030458(lbl_1_bss_A4, fn_80008DD0(DATANUM(0xa40000, 35), 0x10000000, 2));
    fn_8002C95C(lbl_1_bss_A4, lbl_1_bss_D44);
    lbl_1_bss_A0 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 43), 0x10000000, 2));
    lbl_1_bss_9C[0] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 44), 0x10000000, 2));
    lbl_1_bss_9C[1] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 45), 0x10000000, 2));
    fn_800273EC(lbl_1_bss_9C[0], 0.0f, 0.0f, 0.0f);
    fn_800273EC(lbl_1_bss_9C[1], 0.0f, 0.0f, 0.0f);
    lbl_1_bss_D4A = fn_80030458(lbl_1_bss_A0, fn_80008DD0(DATANUM(0xa40000, 46), 0x10000000, 2));
    lbl_1_bss_D46[0] = fn_80030458(lbl_1_bss_9C[0],
                                   fn_80008DD0(DATANUM(0xa40000, 47), 0x10000000, 2));
    lbl_1_bss_D46[1] = fn_80030458(lbl_1_bss_9C[2],
                                   fn_80008DD0(DATANUM(0xa40000, 48), 0x10000000, 2));
    fn_8002C95C(lbl_1_bss_A0, lbl_1_bss_D4A);
    fn_8002C95C(lbl_1_bss_9C[0], lbl_1_bss_D46[0]);
    fn_8002C95C(lbl_1_bss_9C[1], lbl_1_bss_D46[1]);

    index = 0;
    while (index < 0x14) {
        lbl_1_bss_AC[index] = 0;
        index += 1;
    }
    index = 0;
    while (index < 8) {
        slot = 0;
        while (slot < 4) {
            lbl_1_bss_BB0[index][slot].motionType = -1;
            lbl_1_bss_BB0[index][slot].objectIndex = -1;
            lbl_1_bss_BB0[index][slot].placed = -1;
            slot += 1;
        }
        index += 1;
    }
    index = 0;
    while (index < 8) {
        slot = 0;
        while (slot < 8) {
            lbl_1_bss_8B0[index][slot].firstValue = -1;
            lbl_1_bss_8B0[index][slot].secondValue = -1;
            lbl_1_bss_8B0[index][slot].thirdValue = -1;
            slot += 1;
        }
        index += 1;
    }
    lbl_1_bss_90 = MicQuizRandRange(lbl_1_data_794[lbl_1_bss_100][1],
                                    lbl_1_data_794[lbl_1_bss_100][2]);
    fn_1_2800();
    index = 0;
    while (index < lbl_1_bss_90) {
        lbl_1_bss_AC[index] = fn_8003AB38(*lbl_1_bss_114, 0x64, 1U, 3U, -1, fn_1_1833C);
        lbl_1_bss_AC[index]->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MicquizObjectWork),
                                                         0x10000000);
        lbl_1_bss_AC[index]->work[0] = lbl_1_bss_24[index];
        lbl_1_bss_AC[index]->work[1] = index;
        index += 1;
    }
    lbl_1_bss_98 = fn_1_17974();
    fn_1_17A88();
    MicQuizCountdownInit();
    MicQuizPortraitShow(MicQuizCurrentPlayerGet());
    MicQuizPauseInit(*lbl_1_bss_114);
    MicQuizMicContextSet(0);
    fn_8000DAC0(fn_1_17BC8, 0x64U, 0x1800U, 0, *lbl_1_bss_114);
    lbl_1_bss_20 = HuAudSStreamPlay(MSM_STREAM_QUESTION_MUSIC_SECOND);
}

/* Rolls the round's probability table and returns which answer set (A, B or C) to use. */
s32 fn_1_17974(void) {
    s32 roll;

    roll = frandmod(0x64);
    OSReport("PLOB RAND%d\n", roll);
    if (lbl_1_data_7D0[lbl_1_bss_100][0] >= roll) {
        OSReport("PROB_A\n");
        return 0;
    }
    if (lbl_1_data_7D0[lbl_1_bss_100][1] >= roll) {
        OSReport("PROB_B\n");
        return 1;
    }
    if (lbl_1_data_7D0[lbl_1_bss_100][2] >= roll) {
        OSReport("PROB_C\n");
        return 2;
    }
    OSReport("Q Sel Error!!!!!!!!\n");
}

/* Counts the objects that make up the answer, reports it and records it. */
void fn_1_17A88(void) {
    s32 answerCount;
    s32 index;
    answerCount = 0;
    lbl_1_bss_94 = lbl_1_bss_74[0];
    switch (lbl_1_bss_98)
    {
    case 0 :
        index = 0;
        while (index < lbl_1_bss_90)
        {
            if (lbl_1_bss_94 == lbl_1_bss_24[index])
            {
                answerCount += 1;
            }
            index += 1;
        }
        break;
    case 1 :
        answerCount = lbl_1_bss_90;
        break;
    case 2 :
        index = 0;
        while (index < lbl_1_bss_90)
        {
            if (lbl_1_bss_94 != lbl_1_bss_24[index])
            {
                answerCount += 1;
            }
            index += 1;
        }
        break;
    }
    MicQuizExpectedAnswerSet(answerCount);
    OSReport("----------------------------ANSER:%d\n", answerCount);
}

void fn_1_17BC8(void) {
    MicQuizResponseRun(0);
}

/* Runs the second counting round's mode steps: the question message, the light fade, the
   objects, the answer and the closing wipe. */
void fn_1_17BEC(OMOBJ *object) {
    s32 index;
    MicquizObjectWork *work;

    MicQuizCountdownUpdate();
    switch (lbl_1_bss_FC) {
    case 0:
        fn_80026A44(lbl_1_bss_A4, ((1 << 30) | (1 << 1)));
        fn_80026A44(lbl_1_bss_A2, ((1 << 30) | (1 << 1)));
        lbl_1_bss_FC = 4;
        lbl_1_bss_104 = 0.0f;
        return;
    case 4:
        lbl_1_bss_104 += 1.0f;
        if (lbl_1_bss_104 >= 40.0f) {
            lbl_1_bss_FC = 5;
            lbl_1_bss_104 = 0.0f;
            return;
        }
        break;
    case 5:
        lbl_1_bss_104 += 1.0f;
        if (lbl_1_bss_104 >= 44.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 6;
            return;
        }
        fn_1_18998();
        return;
    case 6:
        if (fn_1_18D3C() != 0) {
            lbl_1_bss_FC = 1;
            return;
        }
        break;
    case 1:
        micQuizGameMes = GameMesCreate(2, 0);
        lbl_1_bss_FC = 2;
        return;
    case 2:
        if (GameMesStatGet(micQuizGameMes) == 0) {
            lbl_1_bss_104 = 0.0f;
            fn_1_83C();
            lbl_1_bss_FC = 3;
            lbl_1_data_8B0 = 7;
            fn_8000DAC0(fn_1_19560, 0x64U, 0x1800U, 0, *lbl_1_bss_114);
            return;
        }
        break;
    case 7:
        if (fn_1_18A5C() != 0) {
            lbl_1_bss_FC = 8;
            MicQuizQuestionReadySet();
            return;
        }
        break;
    case 8:
        if ((MicQuizResponseStateGet() == 3) || (MicQuizResponseStateGet() == 4)) {
            lbl_1_bss_FC = 0xC;
            fn_800269E8(lbl_1_bss_A4, ((1 << 30) | (1 << 2)));
            fn_800269E8(lbl_1_bss_A2, ((1 << 30) | (1 << 2)));
            fn_800269E8(lbl_1_bss_A0, ((1 << 30) | (1 << 2)));
            fn_800269E8(lbl_1_bss_9C[0], ((1 << 30) | (1 << 2)));
            fn_800269E8(lbl_1_bss_9C[1], ((1 << 30) | (1 << 2)));
            lbl_1_bss_8AC = lbl_1_bss_8A8 = lbl_1_bss_8A4 = lbl_1_bss_8A0 = 0;
            Hu3DGLightPosAimSetV((s16) lbl_1_bss_D40, &lbl_1_data_868, &lbl_1_data_874);
            fn_800273EC(lbl_1_bss_A4, 1.0f, 1.0f, 1.0f);
            fn_800273EC(lbl_1_bss_A2, 1.0f, 1.0f, 1.0f);
            index = 0;
            while (index < lbl_1_bss_90) {
                work = lbl_1_bss_AC[index]->data;
                fn_800273EC(work->modelId, 1.0f, 1.0f, 1.0f);
                index += 1;
            }
            lbl_1_bss_8C = 20.0f;
            fn_1_18858();
            return;
        }
        if (MicQuizResponseStateGet() == 2) {
            lbl_1_bss_FC = 9;
            return;
        }
        if (MicQuizResponseStateGet() == 0) {
            if (lbl_1_bss_98 == 0) {
                fn_1_10B4(micQuizNpcAnswerMes[lbl_1_bss_94]);
                return;
            }
            if (lbl_1_bss_98 == 1) {
                fn_1_12F4();
                return;
            }
            if (lbl_1_bss_98 == 2) {
                fn_1_14F0(micQuizNpcAnswerMes[lbl_1_bss_94]);
                return;
            }
        }
        break;
    case 9:
        lbl_1_bss_FC = 0xA;
        lbl_1_bss_104 = 0.0f;
        return;
    case 10:
        if (fn_1_18D3C() != 0) {
            lbl_1_bss_FC = 3;
            lbl_1_data_8B0 = 0xB;
            fn_8000DAC0(fn_1_19560, 0x64U, 0x1800U, 0, *lbl_1_bss_114);
            return;
        }
        break;
    case 11:
        if (fn_1_18A5C() != 0) {
            lbl_1_bss_FC = 8;
            MicQuizListenStart();
            return;
        }
        break;
    case 12:
        if (fn_1_19280() != 0) {
            lbl_1_bss_FC = 0xD;
            fn_800269E8(lbl_1_bss_A4, ((1 << 30) | (1 << 1)));
            fn_800269E8(lbl_1_bss_A2, ((1 << 30) | (1 << 1)));
            fn_800269E8(lbl_1_bss_9C[0], ((1 << 30) | (1 << 1)));
            fn_800269E8(lbl_1_bss_9C[1], ((1 << 30) | (1 << 1)));
            return;
        }
        break;
    case 13:
        if (lbl_1_bss_104 >= 180.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 0xE;
            WipeCreate(2, 0, 0x14);
            HuAudSStreamFadeOut(lbl_1_bss_20, 0x64);
            return;
        }
        lbl_1_bss_104 += 1.0f;
        return;
    case 14:
        if (WipeCheck() == 0) {
            HuMCSelWinContextKill();
            HuMCClose();
            OSReport("                                    HuMCClose()\n");
            OSReport(" COUNT2 END==========================\n");
            fn_8003A5E4(1, 1);
        }
    }
}

/* Creates one counted object: its model, motions and a free place, then hands it to its update
   step. */
void fn_1_1833C(OMOBJ *object) {
    MicquizObjectWork *work = object->data;
    s32 positionResult;
    s32 motionIndex;

    work->modelId = fn_80025A04(
        fn_80008DD0(lbl_1_data_404[object->work[0]], 0x10000000, 2));
    for (motionIndex = 0; motionIndex < 4; motionIndex++) {
        if (lbl_1_data_3A4[object->work[0]][motionIndex] == 0) {
            work->motionIds[motionIndex] = -1;
        } else {
            work->motionIds[motionIndex] = fn_80030458(
                work->modelId,
                fn_80008DD0(lbl_1_data_3A4[object->work[0]][motionIndex],
                                     0x10000000, 2));
        }
    }

    work->currentMotion = 0;
    work->motionType = object->work[0];
    work->objectIndex = object->work[1];
    work->rotation.x = work->rotation.z = 0.0f;
    work->rotation.y = 0.0f;
    work->state = 0;

    fn_80026E44(work->modelId);
    fn_800269E8(work->modelId, ((1 << 30) | (1 << 0)));
    positionResult = fn_1_18670(&work->position, object->work[0], work->objectIndex);
    if (work->motionType == 4) {
        fn_800268B0(work->modelId, 0.5f, 0.5f, 0.5f);
    }

    work->savedPosition = work->position;
    work->initialFrames = work->remainingFrames;
    if (work->motionIds[0] != -1) {
        fn_8002C95C(work->modelId, work->motionIds[0]);
    }
    fn_80026760(work->modelId, work->position.x, work->position.y,
                    work->position.z);
    fn_80026808(work->modelId, work->rotation.x, work->rotation.y,
                    work->rotation.z);
    fn_800273EC(work->modelId, 0.0f, 0.0f, 0.0f);
    object->objFunc = fn_1_18604;
}

/* Claims a free slot of a group for an object and returns 1, or returns 0 when the slot is
   taken. */
s32 fn_1_18578(s32 groupIndex, s32 entryIndex, s32 motionType, s32 objectIndex) {
    if (lbl_1_bss_BB0[groupIndex][entryIndex].motionType == -1) {
        lbl_1_bss_BB0[groupIndex][entryIndex].motionType = motionType;
        lbl_1_bss_BB0[groupIndex][entryIndex].objectIndex = objectIndex;
        lbl_1_bss_BB0[groupIndex][entryIndex].placed = 1;
        return 1;
    }
    return 0;
}

/* Updates a counted object's model from its position and rotation each frame. */
void fn_1_18604(OMOBJ *object) {
    MicquizObjectWork *work;

    work = object->data;
    switch (work->state) {
    case 0:
        DebugValue(work->state);
        break;
    }
    fn_80026760(work->modelId, work->position.x, work->position.y,
        work->position.z);
    fn_80026808(work->modelId, work->rotation.x, work->rotation.y,
        work->rotation.z);
}

/* Finds a random free place on the grid for an object and returns its position. */
s32 fn_1_18670(Vec *position, s32 motionType, s32 objectIndex) {
    s32 row;
    s32 column;
    s32 slotClaimed;
    f32 rowOffset;

    do {
        row = frandmod(8);
        column = frandmod(4);
        if (lbl_1_bss_BB0[row][column].motionType == -1) {
            lbl_1_bss_BB0[row][column].motionType = motionType;
            lbl_1_bss_BB0[row][column].objectIndex = objectIndex;
            lbl_1_bss_BB0[row][column].placed = 1;
            slotClaimed = 1;
        } else {
            slotClaimed = 0;
        }
    } while (slotClaimed == 0);

    if (row % 2 == 0) {
        rowOffset = 50.0f;
    } else {
        rowOffset = -50.0f;
    }
    position->x = (200.0f * (f32)column - 300.0f) + rowOffset;
    position->y = 0.0f;
    position->z = -(118.75f * (f32)row) - 100.0f;
    return row;
}

/* Shows the counted objects and starts their motions. */
void fn_1_18858(void) {
    s32 index;
    MicquizObjectWork *work;

    for (index = 0; index < lbl_1_bss_90; index++) {
        work = lbl_1_bss_AC[index]->data;
        switch (lbl_1_bss_98) {
        case 0:
            if (work->motionType != lbl_1_bss_94) {
                fn_800269E8(work->modelId, (1 << 0));
            } else {
                fn_80026A44(work->modelId, (1 << 0));
            }
            break;
        case 1:
            fn_80026A44(work->modelId, (1 << 0));
            break;
        case 2:
            if (work->motionType == lbl_1_bss_94) {
                fn_800269E8(work->modelId, (1 << 0));
            } else {
                fn_80026A44(work->modelId, (1 << 0));
            }
            break;
        }
        fn_1_2788(lbl_1_bss_AC[index], 3, 8.0f, ((1 << 30) | (1 << 0)));
    }
}

void fn_1_18998(void)
{
    HuVecF positions[2];
    s32 i;

    i = 0;
    while (i < 2) {
        fn_800267CC(lbl_1_bss_9C[i], &positions[i]);
        i += 1;
    }
    positions[0].x -= lbl_1_bss_8C;
    positions[1].x += lbl_1_bss_8C;
    i = 0;
    while (i < 2) {
        fn_80026790(lbl_1_bss_9C[i], &positions[i]);
        i += 1;
    }
}

/* Steps the light color toward 0 on every channel, sets the light's color and returns 1 once
   every channel is 0. */
s32 fn_1_18A5C(void) {
    f32 decrement;
    union {
        u32 packed;
        GXColor rgba;
    } lightColor;
    HuVecF initialVector;

    lightColor.packed = 0;
    decrement = 6.0f;
    initialVector.z = 6.0f;
    initialVector.y = 6.0f;
    initialVector.x = 6.0f;

    lbl_1_bss_8AC = (s32)((f32)lbl_1_bss_8AC - decrement);
    if (lbl_1_bss_8AC < 0) {
        lbl_1_bss_8AC = 0;
    }
    lbl_1_bss_8A8 = (s32)((f32)lbl_1_bss_8A8 - decrement);
    if (lbl_1_bss_8A8 < 0) {
        lbl_1_bss_8A8 = 0;
    }
    lbl_1_bss_8A4 = (s32)((f32)lbl_1_bss_8A4 - decrement);
    if (lbl_1_bss_8A4 < 0) {
        lbl_1_bss_8A4 = 0;
    }
    lbl_1_bss_8A0 = (s32)((f32)lbl_1_bss_8A0 - decrement);
    if (lbl_1_bss_8A0 < 0) {
        lbl_1_bss_8A0 = 0;
    }
    Hu3DGLightColorSet((s16)lbl_1_bss_D40, (u8)lbl_1_bss_8AC,
                       (u8)lbl_1_bss_8A8, (u8)lbl_1_bss_8A4,
                       (u8)lbl_1_bss_8A0);
    if (lbl_1_bss_8AC == 0 && lbl_1_bss_8A8 == 0 &&
        lbl_1_bss_8A4 == 0 && lbl_1_bss_8A0 == 0) {
        return 1;
    }
    return 0;
}

/* Steps the light color toward 0xFF on every channel, sets the light's color and returns 1 once
   every channel is 0xFF. */
s32 fn_1_18D3C(void) {
    f32 increment;
    union {
        u32 packed;
        GXColor rgba;
    } lightColor;
    HuVecF initialVector;

    lightColor.packed = 0;
    increment = 6.0f;
    initialVector.z = 6.0f;
    initialVector.y = 6.0f;
    initialVector.x = 6.0f;

    lbl_1_bss_8AC = (s32)((f32)lbl_1_bss_8AC + increment);
    if (lbl_1_bss_8AC > 0xFF) {
        lbl_1_bss_8AC = 0xFF;
    }
    lbl_1_bss_8A8 = (s32)((f32)lbl_1_bss_8A8 + increment);
    if (lbl_1_bss_8A8 > 0xFF) {
        lbl_1_bss_8A8 = 0xFF;
    }
    lbl_1_bss_8A4 = (s32)((f32)lbl_1_bss_8A4 + increment);
    if (lbl_1_bss_8A4 > 0xFF) {
        lbl_1_bss_8A4 = 0xFF;
    }
    lbl_1_bss_8A0 = (s32)((f32)lbl_1_bss_8A0 + increment);
    if (lbl_1_bss_8A0 > 0xFF) {
        lbl_1_bss_8A0 = 0xFF;
    }
    Hu3DGLightColorSet((s16)lbl_1_bss_D40, (u8)lbl_1_bss_8AC,
                       (u8)lbl_1_bss_8A8, (u8)lbl_1_bss_8A4,
                       (u8)lbl_1_bss_8A0);
    if (lbl_1_bss_8AC == 0xFF && lbl_1_bss_8A8 == 0xFF &&
        lbl_1_bss_8A4 == 0xFF && lbl_1_bss_8A0 == 0xFF) {
        return 1;
    }
    return 0;
}

/* Debug control: moves the light with the second pad and prints its position. */
void fn_1_1901C(void) {
    lbl_1_data_85C.x += 2.0f * ((f32)(s8)lbl_802BFFF4[1] / 44.0f);
    lbl_1_data_85C.z -= 2.0f * ((f32)(s8)lbl_802BFFF0[1] / 44.0f);
    if (lbl_802C0010[1] & 4) {
        lbl_1_data_850.y -= 5.0f;
        if (lbl_1_data_850.y <= 0.0f) {
            lbl_1_data_850.y = 0.0f;
        }
    }
    if (lbl_802C0010[1] & 8) {
        lbl_1_data_850.y += 5.0f;
    }
    Hu3DGLightPosAimSetV((s16)lbl_1_bss_D40, &lbl_1_data_850,
        &lbl_1_data_85C);
    fontcolor = 0xE;
    print8(0x10, 0x58, 2.0f, "POS X:%.02f Y:%.02f Z:%.02f",
        lbl_1_data_850.x, lbl_1_data_850.y, lbl_1_data_850.z);
    print8(0x10, 0x68, 2.0f, "VEC X:%.02f Y:%.02f Z:%.02f",
        lbl_1_data_85C.x, lbl_1_data_85C.y, lbl_1_data_85C.z);
}

/* Steps the light color toward 0xFF on every channel, sets the light's color and returns 1 once
   every channel is 0xFF. */
s32 fn_1_19280(void) {
    f32 increment;
    union {
        u32 packed;
        GXColor rgba;
    } lightColor;
    HuVecF initialVector;

    lightColor.packed = 0;
    increment = 6.0f;
    initialVector.z = 6.0f;
    initialVector.y = 6.0f;
    initialVector.x = 6.0f;

    lbl_1_bss_8AC = (s32)((f32)lbl_1_bss_8AC + increment);
    if (lbl_1_bss_8AC > 0xFF) {
        lbl_1_bss_8AC = 0xFF;
    }
    lbl_1_bss_8A8 = (s32)((f32)lbl_1_bss_8A8 + increment);
    if (lbl_1_bss_8A8 > 0xFF) {
        lbl_1_bss_8A8 = 0xFF;
    }
    lbl_1_bss_8A4 = (s32)((f32)lbl_1_bss_8A4 + increment);
    if (lbl_1_bss_8A4 > 0xFF) {
        lbl_1_bss_8A4 = 0xFF;
    }
    lbl_1_bss_8A0 = (s32)((f32)lbl_1_bss_8A0 + increment);
    if (lbl_1_bss_8A0 > 0xFF) {
        lbl_1_bss_8A0 = 0xFF;
    }
    Hu3DGLightColorSet((s16)lbl_1_bss_D40, (u8)lbl_1_bss_8AC,
                       (u8)lbl_1_bss_8A8, (u8)lbl_1_bss_8A4,
                       (u8)lbl_1_bss_8A0);
    if (lbl_1_bss_8AC == 0xFF && lbl_1_bss_8A8 == 0xFF &&
        lbl_1_bss_8A4 == 0xFF && lbl_1_bss_8A0 == 0xFF) {
        return 1;
    }
    return 0;
}

/* Swings the light around the scene, returns it to its place and ends this process. */
void fn_1_19560(void) {
    Vec homePosition;
    Vec startingAim;
    Vec returnDelta;
    f32 radius;
    f32 angle;
    f32 elapsed;
    f32 initialAimX;
    f32 initialAimZ;
    f32 sineOffset;
    f32 cosineOffset;
    s32 frame;

    angle = 0.0f;
    radius = 0.0f;
    elapsed = 0.0f;
    initialAimX = lbl_1_data_85C.x;
    initialAimZ = lbl_1_data_85C.z;
    do {
        HuPrcVSleep();
        sineOffset = (f32)(sin((M_PI * angle) / 180.0) * radius);
        cosineOffset = (f32)(cos((M_PI * angle) / 180.0) * radius);
        lbl_1_data_85C.x = initialAimX + sineOffset;
        lbl_1_data_85C.z = initialAimZ + cosineOffset;
        radius += 2.0f;
        angle += 2.0f;
        elapsed += 1.0f;
        Hu3DGLightPosAimSetV((s16)lbl_1_bss_D40, &lbl_1_data_850,
            &lbl_1_data_85C);
    } while (!(elapsed >= 360.0f));

    frame = 0;
    startingAim = lbl_1_data_85C;
    homePosition.x = 0.0f;
    homePosition.z = -550.0f;
    PSVECSubtract(&homePosition, &startingAim, &returnDelta);
    for (;;) {
        HuPrcVSleep();
        lbl_1_data_85C.x += returnDelta.x / 60.0f;
        lbl_1_data_85C.z += returnDelta.z / 60.0f;
        frame += 1;
        if (frame >= 60) {
            break;
        }
        Hu3DGLightPosAimSetV((s16)lbl_1_bss_D40,
            &lbl_1_data_850, &lbl_1_data_85C);
    }

    lbl_1_bss_FC = lbl_1_data_8B0;
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}
