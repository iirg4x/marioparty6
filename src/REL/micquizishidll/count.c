/* Counting round: objects moving on a grid, the count answer and its check. */
#include "msm_stream.h"
#include "REL/micquizishidll/related_types.h"

#include "REL/micquizishidll/module_base.h"
#include "REL/micquizishidll/object_work.h"

#include "REL/micquizishidll/process_manager.h"

#include "dolphin/types.h"

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

#include "REL/micquizishidll/micquizishidll_callback_views_v1.h"

#include "REL/micquizishidll/micquizishidll_record_views_v1.h"

#include "dolphin/os.h"
#include "REL/micquizishidll/micquizishidll_message_setup_views_v2.h"

void MicQuizWinSoundCallback(HUWINID work, u32 answerId, s16 soundIndex);

struct QuizSelectionEntry {
    s32 count;
    s32 minCount;
    s32 maxCountExclusive;
};

struct QuizCardEntries {
    s32 marker;
    s32 objectIndex;
    s32 placementState;
};

extern s32 lbl_1_bss_20;

extern s32 lbl_1_bss_FC;

extern s16 micQuizGameMes;

extern s32 lbl_1_bss_100;

extern f32 lbl_1_bss_104;

extern s32 lbl_1_bss_110;

extern OMOBJ *lbl_1_bss_A8;

extern s16 lbl_1_bss_A0;

extern s16 lbl_1_bss_A2;

extern s16 lbl_1_bss_A4;

extern s16 lbl_1_bss_9C[2];

extern s16 lbl_1_bss_3DC[1];

extern s16 lbl_1_bss_3D8[2];

extern OMOBJ *lbl_1_bss_AC[0x14];

extern s32 lbl_1_bss_24[0x14];

extern s32 lbl_1_bss_98;

extern s32 lbl_1_bss_90;

extern f32 lbl_1_bss_880[8];

extern s32 lbl_1_bss_860[8];

extern MicQuizRecordGroup lbl_1_bss_6E0[8];

extern struct QuizCardEntries lbl_1_bss_3E0[8][8];

extern struct QuizSelectionEntry lbl_1_data_41C[];

extern f32 lbl_1_data_66C[5];

extern OM_CAMERA_VIEW lbl_1_data_388;

extern Point3d lbl_1_data_470;

extern Point3d lbl_1_data_47C;

extern u8 lbl_1_data_488[4];

extern Point3d lbl_1_data_648;

extern Point3d lbl_1_data_654;

extern Point3d lbl_1_data_660;

u8 MicQuizQuestionLevelGet(void);

s32 MicQuizRandRange(s32, s32);

void fn_1_2800(void);

void fn_1_15F94(OMOBJ *);

s32 fn_1_15714(void);

void fn_1_15828(void);

s32 MicQuizCountdownInit(void);

s32 MicQuizCurrentPlayerGet(void);

void MicQuizPortraitShow(s32 baseIndex);

s32 MicQuizPauseInit(HUPROCESS *);

void MicQuizMicContextSet(s32);

void fn_1_15968(void);

u8 lbl_1_data_5F0[0x1C] = { 0x00, 0xA4, 0x00, 0x7A, 0x00, 0xA4, 0x00, 0x7B, 0x00, 0xA4, 0x00, 0x7C,
        0x00, 0xA4, 0x00, 0x7D, 0x00, 0xA4, 0x00, 0x7E, 0x00, 0xA4, 0x00, 0x7F, 0x00, 0xA4, 0x00, 0x80 };
s32 lbl_1_data_60C[5][3] = { { 99, -1, -1 }, { 99, -1, -1 }, { 49, 99, -1 }, { 39, 69, 99 },
                            { 32, 65, 99 } };
Point3d lbl_1_data_648 = { -100.0f, 8000.0f, -400.0f };
Point3d lbl_1_data_654 = { 0.0f, 1.0f, 0.0f };
Point3d lbl_1_data_660 = { 0.0f, 0.0f, -400.0f };
f32 lbl_1_data_66C[5] = { 400.0f, 325.0f, 250.0f, 175.0f, 100.0f };

/* Builds the quiz camera and models, then creates the card workers. */
void fn_1_14EE0(void) {
    s32 index;
    s32 column;

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 10.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f,
                          0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 0x280, 0x1E0);
    fn_8003E488(1, &lbl_1_data_388);
    lbl_1_bss_A8 = fn_8003AB38(*lbl_1_bss_114, 0x7FDA, 0, 0, -1,
                              fn_8003DAA4);
    lbl_1_bss_A8->work[0] = 1;
    fn_800285BC();
    lbl_1_bss_110 = Hu3DGLightCreate(
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0);
    Hu3DGLightPosAimSetV(lbl_1_bss_110, &lbl_1_data_470,
                         &lbl_1_data_47C);
    Hu3DGLightStaticSet(lbl_1_bss_110, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_110);
    Hu3DGLightColorSet(lbl_1_bss_110, lbl_1_data_488[0],
                       lbl_1_data_488[1], lbl_1_data_488[2],
                       lbl_1_data_488[3]);
    Hu3DShadowCreate(20.0f, 10.0f, 8000.0f);
    fn_8002AAB4(0x10, 0x10, 0x10);
    Hu3DShadowPosSet(&lbl_1_data_648, &lbl_1_data_654, &lbl_1_data_660);
    Hu3DShadowTPLvlSet(0.3f);

    lbl_1_bss_FC = 0;
    micQuizGameMes = -1;
    lbl_1_bss_100 = (s32)MicQuizQuestionLevelGet();
    lbl_1_bss_104 = 0.0f;
    lbl_1_bss_A4 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 34), 0x10000000, 2));
    lbl_1_bss_A2 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 37), 0x10000000, 2));
    fn_80026FFC(lbl_1_bss_A4);
    lbl_1_bss_A0 = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 43), 0x10000000, 2));
    lbl_1_bss_9C[0] = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 44), 0x10000000, 2));
    lbl_1_bss_9C[1] = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 45), 0x10000000, 2));
    lbl_1_bss_3DC[0] = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 38), 0x10000000, 2));
    lbl_1_bss_3D8[0] = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 39), 0x10000000, 2));
    lbl_1_bss_3D8[1] = fn_80025A04(
        fn_80008DD0(DATANUM(0xa40000, 40), 0x10000000, 2));

    fn_80026760(lbl_1_bss_3D8[0], -lbl_1_data_66C[lbl_1_bss_100],
                    0.0f, -110.0f);
    fn_80026760(lbl_1_bss_3D8[1], lbl_1_data_66C[lbl_1_bss_100],
                    0.0f, -110.0f);

    for (index = 0; index < 0x14; index++) {
        lbl_1_bss_AC[index] = 0;
    }
    for (index = 0; index < 8; index++) {
        for (column = 0; column < 4; column++) {
            lbl_1_bss_6E0[index].entries[column].word00 = -1;
            lbl_1_bss_6E0[index].entries[column].word04 = -1;
            lbl_1_bss_6E0[index].entries[column].word08 = -1;
        }
    }
    for (index = 0; index < 8; index++) {
        for (column = 0; column < 8; column++) {
            lbl_1_bss_3E0[index][column].marker = -1;
            lbl_1_bss_3E0[index][column].objectIndex = -1;
            lbl_1_bss_3E0[index][column].placementState = -1;
        }
    }
    for (index = 0; index < 8; index++) {
        lbl_1_bss_880[index] =
            -(3.3f + 0.012941177f *
              (f32)(u8)frand());
    }
    for (index = 0; index < 8; index++) {
        lbl_1_bss_860[index] = frandmod(0x1E);
    }

    lbl_1_bss_90 = MicQuizRandRange(
        lbl_1_data_41C[lbl_1_bss_100].minCount,
        lbl_1_data_41C[lbl_1_bss_100].maxCountExclusive);
    fn_1_2800();
    for (index = 0; index < lbl_1_bss_90; index++) {
        lbl_1_bss_AC[index] = fn_8003AB38(
            *lbl_1_bss_114, 0x64, 1, 3, -1, fn_1_15F94);
        lbl_1_bss_AC[index]->data =
            HuMemDirectMallocNum(HEAP_HEAP, 0x58, 0x10000000U);
        lbl_1_bss_AC[index]->work[0] = (u32)lbl_1_bss_24[index];
        lbl_1_bss_AC[index]->work[1] = (u32)index;
    }

    lbl_1_bss_98 = fn_1_15714();
    fn_1_15828();
    lbl_1_bss_100 = (s32)MicQuizQuestionLevelGet();
    MicQuizCountdownInit();
    MicQuizPortraitShow(MicQuizCurrentPlayerGet());
    MicQuizPauseInit(*lbl_1_bss_114);
    MicQuizMicContextSet(0);
    fn_8000DAC0(fn_1_15968, 0x64, 0x1800, 0, *lbl_1_bss_114);
    lbl_1_bss_20 = HuAudSStreamPlay(MSM_STREAM_QUESTION_MUSIC_SECOND);
}

/* Rolls the round's probability table and returns which answer set (A, B or C) to use. */
s32 fn_1_15714(void) {
    s32 roll;

    roll = frandmod(100);
    OSReport("PLOB RAND%d\n", roll);
    if (lbl_1_data_60C[lbl_1_bss_100][0] >= roll) {
        OSReport("PROB_A\n");
        return 0;
    }
    if (lbl_1_data_60C[lbl_1_bss_100][1] >= roll) {
        OSReport("PROB_B\n");
        return 1;
    }
    if (lbl_1_data_60C[lbl_1_bss_100][2] >= roll) {
        OSReport("PROB_C\n");
        return 2;
    }
    OSReport("Q Sel Error!!!!!!!!\n");
}

s32 MicQuizExpectedAnswerSet(s32);

extern s32 lbl_1_bss_74;

extern s32 lbl_1_bss_94;

/* Counts the objects that make up the answer, reports it and records it. */
void fn_1_15828(void) {
    s32 count;
    s32 index;
    count = 0;
    lbl_1_bss_94 = lbl_1_bss_74;
    switch (lbl_1_bss_98)
    {
    case 0 :
        index = 0;
        while (index < lbl_1_bss_90)
        {
            if (lbl_1_bss_94 == lbl_1_bss_24[index])
            {
                count += 1;
            }
            index += 1;
        }
        break;
    case 1 :
        count = lbl_1_bss_90;
        break;
    case 2 :
        index = 0;
        while (index < lbl_1_bss_90)
        {
            if (lbl_1_bss_94 != lbl_1_bss_24[index])
            {
                count += 1;
            }
            index += 1;
        }
        break;
    }
    MicQuizExpectedAnswerSet(count);
    OSReport("----------------------------ANSER:%d\n", count);
}

void fn_1_15968(void) {
    MicQuizResponseRun(0);
}

extern f32 lbl_1_bss_8C;

extern u32 micQuizNpcAnswerMes[0x11];

void MicQuizCountdownUpdate(void);

void fn_1_161F8(void);

void fn_1_16874(void);

s32 fn_1_168F4(void);

void fn_1_16960(void);

void fn_1_16D74(void);

void fn_1_16E38(void);

s32 fn_1_16EFC(s32 value);

s32 MicQuizResponseStateGet(void);

void fn_1_83C(void);

void MicQuizQuestionReadySet(void);

void MicQuizListenStart(void);

u64 HuMCClose(void);

void HuMCSelWinContextKill(void);

/* Runs the counting round's mode steps: the question message, the objects' moves, the answer and
   the closing wipe. */
void fn_1_1598C(void) {
    MicQuizCountdownUpdate();
    switch (lbl_1_bss_FC) {
    case 0:
        micQuizGameMes = GameMesCreate(2, 0);
        lbl_1_bss_FC = 1;
        return;
    case 1:
        if (GameMesStatGet(micQuizGameMes) != 0) {
            return;
        }
        lbl_1_bss_FC = 2;
        lbl_1_bss_8C = 11.111111f;
        lbl_1_bss_104 = 0.0f;
        fn_1_83C();
        return;
    case 2:
        lbl_1_bss_104 += 1.0f;
        if (lbl_1_bss_104 >= 45.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 3;
            fn_1_16874();
            return;
        }
        fn_1_16D74();
        return;
    case 3:
        if (fn_1_168F4() != 0) {
            lbl_1_bss_FC = 4;
            lbl_1_bss_8C = 22.222221f;
            return;
        }
        break;
    case 4:
        if (fn_1_16EFC((s32)lbl_1_bss_8C) != 0) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 5;
            MicQuizQuestionReadySet();
            return;
        }
        break;
    case 5:
        if (MicQuizResponseStateGet() == 3 || MicQuizResponseStateGet() == 4) {
            lbl_1_bss_FC = 0xA;
            lbl_1_bss_8C = 16.666666f;
            fn_1_16960();
            fn_800269E8(lbl_1_bss_3DC[0], 1U);
            fn_800269E8(lbl_1_bss_3D8[0], 1U);
            fn_800269E8(lbl_1_bss_3D8[1], 1U);
            return;
        }
        if (MicQuizResponseStateGet() == 2) {
            lbl_1_bss_FC = 6;
            return;
        }
        if (MicQuizResponseStateGet() == 0) {
            if (lbl_1_bss_98 == 0) {
                fn_1_A38(micQuizNpcAnswerMes[lbl_1_bss_94]);
                return;
            }
            if (lbl_1_bss_98 == 1) {
                fn_1_C78();
                return;
            }
            if (lbl_1_bss_98 == 2) {
                fn_1_E74(micQuizNpcAnswerMes[lbl_1_bss_94]);
                return;
            }
        }
        break;
    case 6:
        fn_1_161F8();
        lbl_1_bss_FC = 7;
        lbl_1_bss_8C = 11.111111f;
        lbl_1_bss_104 = 0.0f;
        return;
    case 7:
        lbl_1_bss_104 += 1.0f;
        if (lbl_1_bss_104 >= 45.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 8;
            fn_1_16874();
            return;
        }
        fn_1_16D74();
        return;
    case 8:
        if (fn_1_168F4() != 0) {
            lbl_1_bss_FC = 9;
            lbl_1_bss_8C = 22.222221f;
            return;
        }
        break;
    case 9:
        if (fn_1_16EFC((s32)lbl_1_bss_8C) != 0) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 5;
            MicQuizListenStart();
            return;
        }
        break;
    case 10:
        if (lbl_1_bss_104 >= 30.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 0xB;
            lbl_1_bss_8C = 11.111111f;
            return;
        }
        lbl_1_bss_104 += 1.0f;
        fn_1_16E38();
        return;
    case 11:
        if (lbl_1_bss_104 >= 180.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 0xC;
            WipeCreate(2, 0, 0x14);
            HuAudSStreamFadeOut(lbl_1_bss_20, 0x1F4);
            return;
        }
        lbl_1_bss_104 += 1.0f;
        return;
    case 12:
        if (WipeCheck() == 0) {
            HuMCSelWinContextKill();
            OSReport("                                    HuMCClose()\n", (u32)HuMCClose());
            OSReport(" COUNT END==========================\n");
            fn_8003A5E4(1, 1);
        }
        break;
    }
}

extern u32 lbl_1_data_3A4[6][4];

extern s32 lbl_1_data_404[6];

void fn_1_162F8(OMOBJ *object);

s32 fn_1_166D4(HuVecF *position, s32 objectType, s32 motionData);

/* Creates one counted object: its model, motions and a free place, then hands it to its update
   step. */
void fn_1_15F94(OMOBJ *object) {
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
    work->rotation.y = -90.0f;
    work->state = 0;

    fn_80026E44(work->modelId);
    fn_800269E8(work->modelId, ((1 << 30) | (1 << 0)));
    fn_800269E8(work->modelId, 1);
    positionResult = fn_1_166D4(&work->position, object->work[0], work->objectIndex);
    work->animationSpeed = lbl_1_bss_880[positionResult];
    work->remainingFrames = lbl_1_bss_860[positionResult];
    if (work->motionType == 4) {
        fn_800268B0(work->modelId, 0.5f, 0.5f, 0.5f);
        work->rotation.y = 0.0f;
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
    object->objFunc = fn_1_162F8;
}

extern OMOBJ *lbl_1_bss_AC[];

/* Puts every counted object back at its saved place with its starting frame count. */
void fn_1_161F8(void) {
    s32 index;
    MicquizObjectWork *work;

    for (index = 0; index < lbl_1_bss_90; index++) {
        work = lbl_1_bss_AC[index]->data;
        work->position = work->savedPosition;
        work->remainingFrames = work->initialFrames;
    }
}

/* Claims a free slot of a group for an object and returns 1, or returns 0 when the slot is
   taken. */
s32 fn_1_1626C(s32 groupIndex, s32 entryIndex, s32 word00, s32 word04) {
    if (lbl_1_bss_6E0[groupIndex].entries[entryIndex].word00 == -1) {
        lbl_1_bss_6E0[groupIndex].entries[entryIndex].word00 = word00;
        lbl_1_bss_6E0[groupIndex].entries[entryIndex].word04 = word04;
        lbl_1_bss_6E0[groupIndex].entries[entryIndex].word08 = 1;
        return 1;
    }
    return 0;
}

extern void fn_1_164B8(s16 modelId, f32 animationSpeed, s32 motionType);

extern s32 fn_1_16628(f32 angle);

extern void fn_1_2788(OMOBJ *object, s32 motion, f32 scale, u32 attribute);

/* Updates a counted object each frame by its state: its motion, place and turn. */
void fn_1_162F8(OMOBJ *object) {
    MicquizObjectWork *work;

    work = object->data;
    switch (work->state) {
    case 0:
        break;
    case 1:
        if (work->remainingFrames <= 0) {
            work->state = 2;
            fn_1_164B8(work->modelId, work->animationSpeed,
                       work->motionType);
            if (work->animationSpeed > 4.95f) {
                fn_1_2788(object, 2, 8.0f, ((1 << 30) | (1 << 0)));
            } else {
                fn_1_2788(object, 1, 8.0f, ((1 << 30) | (1 << 0)));
            }
        } else {
            work->remainingFrames--;
        }
        break;
    case 2:
        if (fn_1_16628(work->position.x) != 0) {
            fn_80026A44(work->modelId, 1);
        } else {
            fn_800269E8(work->modelId, 1);
        }
        work->previousPosition = work->position;
        work->position.x += work->animationSpeed;
        if (work->position.x < -600.0f) {
            work->state = 3;
            fn_8002D86C(work->modelId, 1.0f);
            fn_1_2788(object, 0, 8.0f, ((1 << 30) | (1 << 0)));
        }
        break;
    case 3:
        break;
    }
    fn_80026760(work->modelId, work->position.x,
                    work->position.y, work->position.z);
    fn_80026808(work->modelId, work->rotation.x, work->rotation.y,
                    work->rotation.z);
}

/* Sets a model's motion speed from the given speed, by the motion type. */
void fn_1_164B8(s16 modelId, f32 animationSpeed, s32 motionType) {
    f32 magnitude;
    f32 comparisonSpeed;

    if (animationSpeed < 0.0f) {
        magnitude = -animationSpeed;
    } else {
        magnitude = animationSpeed;
    }
    comparisonSpeed = magnitude;
    if (comparisonSpeed > 5.5f) {
        if (motionType != 4) {
            fn_8002D86C(modelId, 2.0f);
        } else {
            fn_8002D86C(modelId, 4.0f);
        }
        return;
    }
    if (comparisonSpeed > 4.4f) {
        if (motionType != 4) {
            fn_8002D86C(modelId, 1.5f);
        } else {
            fn_8002D86C(modelId, 3.0f);
        }
        return;
    }
    if (comparisonSpeed > 3.3f) {
        if (motionType != 4) {
            fn_8002D86C(modelId, 1.0f);
        } else {
            fn_8002D86C(modelId, 2.0f);
        }
        return;
    }
}

/* Returns 1 when the angle is inside the round's allowed range. */
s32 fn_1_16628(f32 angle) {
    f32 lowerAngle;
    f32 upperAngle;

    lowerAngle = -lbl_1_data_66C[lbl_1_bss_100] - 100.0f;
    upperAngle = 100.0f + lbl_1_data_66C[lbl_1_bss_100];
    if ((upperAngle > angle) && (lowerAngle < angle)) {
        return 1;
    }
    return 0;
}

/* Finds a random free place on the grid for an object and returns its position. */
s32 fn_1_166D4(HuVecF *position, s32 firstValue, s32 secondValue) {
    s32 row;
    s32 column;
    s32 result;

    do {
        row = frandmod(8);
        column = frandmod(4);
        if (lbl_1_bss_6E0[row].entries[column].word00 == -1) {
            lbl_1_bss_6E0[row].entries[column].word00 = firstValue;
            lbl_1_bss_6E0[row].entries[column].word04 = secondValue;
            lbl_1_bss_6E0[row].entries[column].word08 = 1;
            result = 1;
        } else {
            result = 0;
        }
    } while (result == 0);
    position->x = 500.0f + 200.0f * (f32)column;
    position->y = 0.0f;
    position->z = -(118.75f * (f32)row) - 200.0f;
    return row;
}

/* Starts every counted object moving (state 1). */
void fn_1_16874(void) {
    s32 index;
    MicquizObjectWork *work;

    for (index = 0; index < lbl_1_bss_90; index++) {
        work = lbl_1_bss_AC[index]->data;
        work->state = 1;
    }
            OSReport("IDX2:%d\n", index);
}

/* Returns 1 once every counted object has reached state 3. */
s32 fn_1_168F4(void) {
    s32 index;
    MicquizObjectWork *work;

    for (index = 0; index < lbl_1_bss_90; index++) {
        work = lbl_1_bss_AC[index]->data;
        if (work->state != 3) {
            return 0;
        }
    }
    return 1;
}

s32 fn_1_16BA8(s32, s32, s32);

/* Places the counted objects on random free grid cells, shows them and starts their motions. */
void fn_1_16960(void) {
    s32 index;
    s32 row;
    s32 column;
    MicquizObjectWork *work;
    HuVecF position;

    for (index = 0; index < lbl_1_bss_90; index++) {
        work = lbl_1_bss_AC[index]->data;
        do {
            row = frandmod(8);
            column = frandmod(8);
        } while (fn_1_16BA8(row, column, work->objectIndex) == 0);

        position.x = (100.0f * (f32)column) - 400.0f;
        position.y = 0.0f;
        position.z = (100.0f * (f32)row) - 1000.0f;
        work->position = position;
        work->rotation.y = 0.0f;

        switch (lbl_1_bss_98) {
        case 0:
            if (work->motionType != lbl_1_bss_94) {
                fn_800269E8(work->modelId, 1);
            } else {
                fn_80026A44(work->modelId, 1);
            }
            break;
        case 1:
            fn_80026A44(work->modelId, 1);
            break;
        case 2:
            if (work->motionType == lbl_1_bss_94) {
                fn_800269E8(work->modelId, 1);
            } else {
                fn_80026A44(work->modelId, 1);
            }
            break;
        }

        fn_1_2788(lbl_1_bss_AC[index], 3, 8.0f, ((1 << 30) | (1 << 0)));
    }
}

/* Returns 1 when the grid cell and its neighbouring cells do not hold the value. */
s32 fn_1_16BA8(s32 row, s32 column, s32 value) {
    s32 nextRow;
    s32 nextColumn;
    s32 previousRow;
    s32 previousColumn;
    previousRow = row - 1;
    if (previousRow < 0)
    {
        previousRow = row;
    }
    nextRow = row + 1;
    if (previousRow >= 8)
    {
        nextRow = row;
    }
    previousColumn = column - 1;
    if (previousColumn < 0)
    {
        previousColumn = column;
    }
    nextColumn = column + 1;
    if (previousColumn >= 8)
    {
        nextColumn = column;
    }
    if ((lbl_1_bss_90 < 8) && ((row < 1) || (row > 6) || (column < 1) || (6 < column)))
    {
        return 0;
    }
    if (lbl_1_bss_3E0[row][column].objectIndex == -1)
    {
        if ((lbl_1_bss_3E0[previousRow][column].objectIndex == -1) &&
            (lbl_1_bss_3E0[nextRow][column].objectIndex == -1) &&
            (lbl_1_bss_3E0[row][previousColumn].objectIndex == -1) &&
            (lbl_1_bss_3E0[row][nextColumn].objectIndex == -1))
        {
            lbl_1_bss_3E0[row][column].marker = -1;
            lbl_1_bss_3E0[row][column].objectIndex = value;
            lbl_1_bss_3E0[row][column].placementState = 1;
            return 1;
        }
        return 0;
    }
    return 0;
}

void fn_1_16D74(void)
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

void fn_1_16E38(void)
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

/* Slides the two side models toward the middle by the given step (0 puts them there at once) and
   returns 1 once they meet. */
s32 fn_1_16EFC(s32 distance) {
    Point3d positions[2];
    s32 reachedLimit;
    s32 index;

    reachedLimit = 0;
    for (index = 0; index < 2; index++) {
        fn_800267CC(lbl_1_bss_9C[index], &positions[index]);
    }
    if (distance == 0) {
        positions[0].x = 0.0f;
        positions[1].x = 0.0f;
    } else {
        positions[0].x += (f32)distance;
        positions[1].x -= (f32)distance;
        if (positions[0].x >= 0.0f) {
            positions[0].x = positions[1].x = 0.0f;
            reachedLimit = 1;
        }
    }
    for (index = 0; index < 2; index++) {
        fn_80026790(lbl_1_bss_9C[index], &positions[index]);
    }
    return reachedLimit;
}

f32 lbl_1_bss_880[8];
s32 lbl_1_bss_860[8];
MicQuizRecordGroup lbl_1_bss_6E0[8];
struct QuizCardEntries lbl_1_bss_3E0[8][8];
s16 lbl_1_bss_3DC[1];
s16 lbl_1_bss_3D8[2];
