/* Memory round: objects that move and must be remembered, the answer and its check. */
#include "msm_stream.h"
#include "dolphin/types.h"

#include "dolphin/os.h"

#include "dolphin/gx.h"

#include "math.h"

#include "dolphin/mtx.h"

#include "string.h"

#include "dolphin.h"

#include "humath.h"

#include "datadir_enum.h"

#include "messdir_enum.h"

#include "game/hu3d.h"

#include "game/object.h"

#include "game/gamework.h"

#include "game/audio.h"

#include "game/sprite.h"

#include "game/memory.h"

#include "game/data.h"

#include "game/main.h"

#include "game/pad.h"

#include "game/window.h"

#include "game/wipe.h"

#include "game/frand.h"

#include "game/charman.h"

#include "game/process.h"

#include "game/hsfex.h"

#include "game/gamemes.h"

#include "game/flag.h"

#include "game/armem.h"

#include "game/mgdata.h"

#include "game/printfunc.h"

#include "game/saveload.h"

#include "game/esprite.h"

#include "game/thpmain.h"

#include "game/mg/seqman.h"

#include "game/mg/actman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/board/object.h"

#include "game/board/main.h"

#include "game/board/audio.h"

#include "game/board/tutorial.h"

#include "game/board/masu.h"

#include "game/board/player.h"
#include "REL/micquizishidll/micquizishidll_message_setup_views_v2.h"
#include "REL/micquizishidll/module_base.h"

void MicQuizResponseRun(s32 mode);

struct TargetPoints { Vec positions[8]; };

extern s32 lbl_1_bss_100;

extern s32 lbl_1_bss_110;

extern s32 lbl_1_bss_20;

extern HUPROCESS *lbl_1_bss_114[0xB];

extern f32 lbl_1_bss_8C;

extern s16 lbl_1_bss_9C[2];

extern s16 lbl_1_bss_A0;

extern OMOBJ *lbl_1_bss_A8;

struct MemoryHintState {
    s32 shown;
    u8 unknown04[4]; /* No code names this field. */
};

OMOBJ *lbl_1_bss_DFC[0xF];
s16 lbl_1_bss_DFA;
s16 lbl_1_bss_DE8[9];
s16 lbl_1_bss_DD8[8];
s32 lbl_1_bss_DA8[0xC];
s32 lbl_1_bss_D88[8];
s32 lbl_1_bss_D68[8];
s32 lbl_1_bss_D64;
s32 lbl_1_bss_D60;
s32 lbl_1_bss_D5C;
struct MemoryHintState lbl_1_bss_D54;
s32 lbl_1_bss_D50;

extern u32 lbl_1_bss_FC;

extern Point3d lbl_1_data_470;

extern Point3d lbl_1_data_47C;

extern u8 lbl_1_data_488[4];

u32 lbl_1_data_9E0[7] = {
    0xA4007A, 0xA4007B, 0xA4007C, 0xA4007D, 0xA4007E, 0xA4007F, 0xA40080
};

OM_CAMERA_VIEW lbl_1_data_9FC = {
    {0.0f, 230.0f, -225.0f}, {-27.2f, 0.0f, 0.0f}, 4214.0f
};

Point3d lbl_1_data_A18 = {-100.0f, 8000.0f, -400.0f};

Point3d lbl_1_data_A24 = {0.0f, 1.0f, 0.0f};

Point3d lbl_1_data_A30 = {0.0f, 0.0f, -400.0f};

s32 lbl_1_data_A3C[8] = {
    0xA4004C, 0xA4004D, 0xA4004E, 0xA4004F,
    0xA40050, 0xA40051, 0xA40052, 0xA40053
};

s32 lbl_1_data_A5C[5] = {2, 3, 4, 5, 6};

struct TargetPoints lbl_1_data_A70[5] = {
    {{{-300.0f, 0.0f, -600.0f}, {300.0f, 0.0f, -600.0f}}},
    {{{-550.0f, 0.0f, -600.0f}, {0.0f, 0.0f, -600.0f},
      {550.0f, 0.0f, -600.0f}}},
    {{{-550.0f, 0.0f, -300.0f}, {550.0f, 0.0f, -300.0f},
      {-550.0f, 0.0f, -1000.0f}, {550.0f, 0.0f, -1000.0f}}},
    {{{-500.0f, 0.0f, -300.0f}, {500.0f, 0.0f, -300.0f},
      {0.0f, 0.0f, -700.0f}, {-550.0f, 0.0f, -1000.0f},
      {550.0f, 0.0f, -1000.0f}}},
    {{{-550.0f, 0.0f, -300.0f}, {0.0f, 0.0f, -300.0f},
      {550.0f, 0.0f, -300.0f}, {-650.0f, 0.0f, -1000.0f},
      {0.0f, 0.0f, -1000.0f}, {650.0f, 0.0f, -1000.0f}}}
};

Vec lbl_1_data_C50[5][8] = {
    {{-75.0f, 0.0f, -150.0f}, {75.0f, 0.0f, -150.0f}},
    {{-150.0f, 0.0f, -150.0f}, {0.0f, 0.0f, -150.0f},
     {150.0f, 0.0f, -150.0f}},
    {{-225.0f, 0.0f, -150.0f}, {-75.0f, 0.0f, -150.0f},
     {75.0f, 0.0f, -150.0f}, {225.0f, 0.0f, -150.0f}},
    {{-305.0f, 0.0f, -150.0f}, {-150.0f, 0.0f, -150.0f},
     {0.0f, 0.0f, -150.0f}, {150.0f, 0.0f, -150.0f},
     {305.0f, 0.0f, -150.0f}},
    {{-375.0f, 0.0f, -150.0f}, {-225.0f, 0.0f, -150.0f},
     {-75.0f, 0.0f, -150.0f}, {75.0f, 0.0f, -150.0f},
     {225.0f, 0.0f, -150.0f}, {375.0f, 0.0f, -150.0f}}
};

s32 lbl_1_data_E30[14][4] = {
    {0xA30001, 0xA30002, 0xA30003, 0xA30004},
    {0xA30006, 0xA30007, 0xA30008, 0xA30009},
    {0xA3000B, 0xA3000C, 0xA3000D, 0xA3000E},
    {0xA30010, 0xA30011, 0xA30012, 0xA30013},
    {0xA30015, 0xA30016, 0xA30017, 0xA30018},
    {0xA3001A, 0xA3001B, 0xA3001C, 0xA3001D},
    {0xA3001F, 0xA30020, 0xA30021, 0xA30022},
    {0xA30024, 0xA30025, 0xA30026, 0xA30027},
    {0xA30029, 0xA3002A, 0xA3002B, 0xA3002C},
    {0xA3002E, 0xA3002F, 0xA30030, 0xA30031},
    {0xA30033, 0xA30034, 0xA30035, 0xA30036}
};

s32 lbl_1_data_F10[34] = {
    0xA30000, 0xA30005, 0xA3000A, 0xA3000F, 0xA30014, 0xA30019,
    0xA3001E, 0xA30023, 0xA30028, 0xA3002D, 0xA30032, 0, 0, 0,
    0xA30000, 0xA30000, 0xA30000, 0xA30000, 0xA30000,
    0xA30000, 0xA30000, 0xA30000, 0xA30000, 0xA30000,
    0xA30000, 0xA30000, 0xA30000, 0xA30000, 0xA30000,
    0xA30000, 0xA30000, 0xA30000, 0xA30000, 0xA30000
};

s32 lbl_1_data_F98[8] = {
    0x1E0012, 0x1E0010, 0x1E000B, 0x1E000D,
    0x1E000C, 0x1E000F, 0x1E000E, 0x1E0011
};

void MicQuizCountdownInit(void);

void MicQuizPortraitShow(s32 baseIndex);

void fn_1_19FB4(void);

void fn_1_1A4F8(OMOBJ *);

void fn_1_1BBF8(void);

void fn_1_1BD78(void);

void fn_1_1BEC8(void);

void fn_1_1C010(void);

s32 MicQuizPauseInit(HUPROCESS *);

u8 MicQuizQuestionLevelGet(void);

s32 MicQuizCurrentPlayerGet(void);

s32 MicQuizMicContextSet(s32);

/* Sets up the memory round: camera, lights, the objects and the models. */
void fn_1_197D4(void) {
    s32 index;

    lbl_1_bss_100 = (s32) MicQuizQuestionLevelGet();
    lbl_1_bss_FC = 0;
    lbl_1_bss_8C = 16.666666f;
    lbl_1_bss_D54.shown = 0;
    lbl_1_bss_D50 = lbl_1_data_A5C[lbl_1_bss_100];
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 20.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0U, 0U, 0x280U, 0x1E0U);
    fn_8003E488(1, &lbl_1_data_9FC);
    lbl_1_bss_A8 = fn_8003AB38(*lbl_1_bss_114, 0x7FDA, 0U, 0U, -1, fn_8003DAA4);
    lbl_1_bss_A8->work[0] = 1;
    fn_800285BC();
    lbl_1_bss_110 = Hu3DGLightCreate(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0U, 0U, 0U);
    Hu3DGLightPosAimSetV((s16) lbl_1_bss_110, &lbl_1_data_470, &lbl_1_data_47C);
    Hu3DGLightStaticSet((s16) lbl_1_bss_110, 1);
    Hu3DGLightInfinitytSet((s16) lbl_1_bss_110);
    Hu3DGLightColorSet((s16) lbl_1_bss_110, lbl_1_data_488[0], lbl_1_data_488[1], lbl_1_data_488[2],
                       lbl_1_data_488[3]);
    Hu3DShadowCreate(20.0f, 10.0f, 8000.0f);
    fn_8002AAB4(0x10U, 0x10U, 0x10U);
    Hu3DShadowPosSet(&lbl_1_data_A18, &lbl_1_data_A24, &lbl_1_data_A30);
    Hu3DShadowTPLvlSet(0.3f);
    lbl_1_bss_DFA = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x4A), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_DFA, ((1 << 30) | (1 << 0)));
    fn_80026B8C(lbl_1_bss_DFA, 0);
    lbl_1_bss_DFA = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x4B), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_DFA, ((1 << 30) | (1 << 0)));
    fn_80026FFC(lbl_1_bss_DFA);
    fn_80026B8C(lbl_1_bss_DFA, 0);
    index = 0;
    while (index < lbl_1_bss_D50) {
        lbl_1_bss_DD8[index] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x54), 0x10000000, 2));
        fn_80026760(lbl_1_bss_DD8[index], lbl_1_data_A70[lbl_1_bss_100].positions[index].x,
                lbl_1_data_A70[lbl_1_bss_100].positions[index].y, lbl_1_data_A70[
                        lbl_1_bss_100].positions[index].z);
        fn_80026B8C(lbl_1_bss_DD8[index], 1);
        index += 1;
    }
    fn_1_1BD78();
    fn_1_1BEC8();
    index = 0;
    while (index < lbl_1_bss_D50) {
        lbl_1_bss_DE8[index] = fn_80025A04(fn_80008DD0(lbl_1_data_A3C[lbl_1_bss_DA8[index]],
                                                       0x10000000, 2));
        fn_80026760(lbl_1_bss_DE8[index], lbl_1_data_A70[lbl_1_bss_100].positions[index].x,
                lbl_1_data_A70[lbl_1_bss_100].positions[index].y, lbl_1_data_A70[
                        lbl_1_bss_100].positions[index].z);
        fn_80026B8C(lbl_1_bss_DE8[index], 2);
        index += 1;
    }
    lbl_1_bss_A0 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x2B), 0x10000000, 2));
    fn_80026760(lbl_1_bss_A0, 0.0f, 200.0f, 0.0f);
    lbl_1_bss_9C[0] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x2C), 0x10000000, 2));
    lbl_1_bss_9C[1] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 0x2D), 0x10000000, 2));
    fn_80026B8C(lbl_1_bss_A0, 2);
    fn_80026B8C(lbl_1_bss_9C[0], 2);
    fn_80026B8C(lbl_1_bss_9C[1], 2);
    fn_800097F4(0xa20000);
    index = 0;
    while (index < lbl_1_bss_D50) {
        lbl_1_bss_DFC[index] = fn_8003AB38(*lbl_1_bss_114, 0x64, 1U, 4U, -1, fn_1_1A4F8);
        lbl_1_bss_DFC[index]->data = HuMemDirectMallocNum(HEAP_HEAP, 0x68, 0x10000000);
        lbl_1_bss_DFC[index]->work[0] = lbl_1_bss_D88[index];
        lbl_1_bss_DFC[index]->work[1] = (u32) index;
        index += 1;
    }
    fn_1_1BBF8();
    fn_1_1C010();
    MicQuizCountdownInit();
    MicQuizPortraitShow(MicQuizCurrentPlayerGet());
    MicQuizPauseInit(*lbl_1_bss_114);
    MicQuizMicContextSet(3);
    fn_8000DAC0(fn_1_19FB4, 0x64U, 0x1800U, 0, *lbl_1_bss_114);
    lbl_1_bss_20 = HuAudSStreamPlay(MSM_STREAM_QUESTION_MUSIC_THIRD);
}

void fn_1_19FB4(void) {
    MicQuizResponseRun(3);
}

extern f32 lbl_1_bss_104;

extern s16 micQuizGameMes;

typedef struct MicquizModelWork_s {
    Vec displacement;
    Vec movement;
    s32 timer;
    Vec *positions;
    s32 targetIndex;
    Vec position;
    Vec previousPosition;
    Vec rotation;
    s32 sourceSequence;
    s32 motionState;
    s32 state;
    f32 depthOffset;
    s32 objectIndex;
    HU3D_MODELID model;
    HU3D_MOTIONID motions[4];
} MicquizModelWork;

s32 MicQuizCountdownUpdate(void);

s32 fn_1_1A48C(void);

void fn_1_1BABC(void);

s32 fn_1_1BB8C(void);

void fn_1_1C14C(void);

void MicQuizQuestionReadySet(void);

s32 MicQuizResponseStateGet(void);

u64 HuMCClose(void);

void HuMCSelWinContextKill(void);

/* Runs the memory round's mode steps: the hint, the objects' moves, the answer, and the closing
   wipe. */
void fn_1_19FD8(void) {
    s32 objectIndex;
    s32 randomIndex;
    OMOBJ *object;
    MicquizModelWork *work;

    MicQuizCountdownUpdate();
    switch (lbl_1_bss_FC) {
    case 0:
        micQuizGameMes = GameMesCreate(2, 0);
        lbl_1_bss_FC = 1;
        lbl_1_bss_104 = 0.0f;
        fn_1_1D24();
        return;
    case 1:
        if (GameMesStatGet(micQuizGameMes) != 0) {
            return;
        }
        lbl_1_bss_104 = 0.0f;
        lbl_1_bss_FC = 3;
        return;
    case 3:
        if (lbl_1_bss_104 >= 45.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 2;
            return;
        }
        fn_1_1C14C();
        lbl_1_bss_104 += 1.0f;
        return;
    case 2:
        if (lbl_1_bss_104 >= 120.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 6;
            fn_1_1BABC();
            return;
        }
        lbl_1_bss_104 += 1.0f;
        return;
    case 6:
        if (fn_1_1BB8C() != 0) {
            lbl_1_bss_FC = 5;
            fn_1_1F20(lbl_1_data_F98[lbl_1_bss_DA8[lbl_1_bss_D64]]);
            MicQuizQuestionReadySet();
            return;
        }
        break;
    case 5:
        if (MicQuizResponseStateGet() == 3 || MicQuizResponseStateGet() == 4) {
            lbl_1_bss_FC = 9;
            for (objectIndex = 0; objectIndex < lbl_1_bss_D50; objectIndex++) {
                work = lbl_1_bss_DFC[objectIndex]->data;
                if (lbl_1_bss_D60 == objectIndex) {
                    work->state = 8;
                } else {
                    work->state = 7;
                }
            }
            return;
        }
        if (MicQuizResponseStateGet() == 2 && lbl_1_bss_D54.shown == 0) {
            lbl_1_bss_FC = 7;
            return;
        }
        break;
    case 7:
        do {
            randomIndex = frandmod(lbl_1_bss_D50);
        } while (randomIndex == lbl_1_bss_D60);
        OSReport("                              MEMORY HINT\n");
        OSReport("                              MemHintObjNum:%d\n", lbl_1_bss_D5C);
        if (lbl_1_bss_D54.shown == 0) {
            work = lbl_1_bss_DFC[randomIndex]->data;
            work->state = 0xF;
            lbl_1_bss_D54.shown = 1;
        }
        lbl_1_bss_FC = 8;
        return;
    case 9:
        if (fn_1_1A48C() != 0) {
            lbl_1_bss_FC = 0xA;
            return;
        }
        break;
    case 10:
        if (lbl_1_bss_104 >= 180.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 0xB;
            WipeCreate(2, 0, 0x14);
            HuAudSStreamFadeOut(lbl_1_bss_20, 0x1F4);
            return;
        }
        lbl_1_bss_104 += 1.0f;
        return;
    case 11:
        if (WipeCheck() == 0) {
            HuMCSelWinContextKill();
            OSReport(" MEMORY END==========================\n", (u32)HuMCClose());
            fn_8003A5E4(1, 1);
        }
        break;
    default:
        break;
    }
}

/* Returns 1 once every object has reached state 0xC. */
s32 fn_1_1A48C(void) {
    s32 index;
    MicquizModelWork *work;

    for (index = 0; index < lbl_1_bss_D50; index++) {
        work = lbl_1_bss_DFC[index]->data;
        if (work->state != 0xC) {
            return 0;
        }
    }
    return 1;
}

extern void fn_1_1A708(OMOBJ *obj);

/* Creates a memory object's model and motions at its place, then hands it to its update step. */
void fn_1_1A4F8(OMOBJ *obj) {
    MicquizModelWork *work;
    s32 motionIndex;

    work = obj->data;
    work->model = fn_80025A04(fn_80008DD0(lbl_1_data_F10[obj->work[0]], 0x10000000, 2));
    for (motionIndex = 0; motionIndex < 4; motionIndex++) {
        work->motions[motionIndex] = fn_80030458(work->model,
                fn_80008DD0(lbl_1_data_E30[obj->work[0]][motionIndex], 0x10000000, 2));
    }
    work->motionState = 0;
    work->sourceSequence = obj->work[0];
    work->objectIndex = obj->work[1];
    work->rotation.x = work->rotation.y = work->rotation.z = 0.0f;
    work->state = 0;
    work->timer = 0;
    if (frandmod(2) != 0U) {
        work->depthOffset = 200.0f;
    } else {
        work->depthOffset = -200.0f;
    }
    work->positions = &lbl_1_data_A70[lbl_1_bss_100].positions[work->targetIndex];
    fn_80026E44(work->model);
    fn_800269E8(work->model, ((1 << 30) | (1 << 0)));
    work->previousPosition = lbl_1_data_C50[lbl_1_bss_100][work->objectIndex];
    work->position = work->previousPosition;
    fn_8002C95C(work->model, work->motions[0]);
    fn_80026760(work->model, work->position.x, work->position.y, work->position.z);
    fn_80026808(work->model, work->rotation.x, work->rotation.y, work->rotation.z);
    obj->objFunc = fn_1_1A708;
}

s32 fn_1_1AE10(OMOBJ *, f32, f32, f32, f32, f32);

void fn_1_1B5FC(OMOBJ *, s32, f32, u32);

void MicQuizListenStart(void);

extern s16 lbl_1_data_458[0xC];

/* Updates a memory object each frame by its state, with its motions and sounds. */
void fn_1_1A708(OMOBJ *object) {
    MicquizModelWork *work;
    Vec target;

    work = object->data;
    work->previousPosition = work->position;
    target = lbl_1_data_A70[lbl_1_bss_100].positions[work->targetIndex];
    switch ((work->state)) {
    case 1:
        if (fn_8002D75C(work->model) != 0) {
            work->state = 2U;
            work->timer = 0;
            fn_1_1B5FC(object, 0, 32.0f, ((1 << 30) | (1 << 0)));
        }
        break;
    case 2:
        if (work->timer >= 0x3C) {
            work->timer = 0;
            work->state = 3U;
        } else {
            work->timer = work->timer + 1;
        }
        break;
    case 3:
        fn_1_1B5FC(object, 2, 8.0f, ((1 << 30) | (1 << 0)));
        if (fn_1_1AE10(object, target.x, target.y, target.z, 175.0f, 50.0f) != 0) {
            if (work->position.z < target.z) {
                work->state = 5U;
            } else {
                work->state = 4U;
            }
        }
        break;
    case 4:
        if (fn_1_1AE10(object, target.x + work->depthOffset, target.y, target.z, 5.0f, 5.0f) != 0) {
            work->state = 5U;
        }
        break;
    case 5:
        if (fn_1_1AE10(object, target.x + work->depthOffset, target.y, target.z - 150.0f, 5.0f,
                       5.0f) != 0) {
            work->state = 6U;
        }
        break;
    case 6:
        if (fn_1_1AE10(object, target.x, target.y, target.z - 150.0f, 5.0f, 5.0f) != 0) {
            work->state = 0xDU;
            work->position.z = (f32) (target.z - 100.0f);
            work->rotation.y = 0.0f;
            fn_1_1B5FC(object, 0, 8.0f, ((1 << 30) | (1 << 0)));
        }
        break;
    case 7:
        if (work->timer >= 0xF0) {
            work->timer = 0;
            work->state = 8U;
        } else {
            work->timer = work->timer + 1;
        }
        break;
    case 8:
        fn_1_1B5FC(object, 1, 8.0f, ((1 << 30) | (1 << 0)));
        if (fn_1_1AE10(object, target.x + work->depthOffset, target.y, target.z - 150.0f, 5.0f,
                       5.0f) != 0) {
            work->state = 9U;
        }
        break;
    case 9:
        if (fn_1_1AE10(object, target.x + work->depthOffset, target.y, target.z, 5.0f, 5.0f) != 0) {
            work->state = 0xAU;
        }
        break;
    case 10:
        if (fn_1_1AE10(object, target.x, target.y, 125.0f + target.z, 5.0f, 5.0f) != 0) {
            work->state = 0xBU;
            work->rotation.y = 0.0f;
            if (work->objectIndex == lbl_1_bss_D60) {
                HuAudFXPlay((s32) lbl_1_data_458[work->sourceSequence]);
                fn_1_1B5FC(object, 3, 8.0f, 0);
            } else {
                fn_1_1B5FC(object, 0, 8.0f, ((1 << 30) | (1 << 0)));
            }
        }
        break;
    case 11:
        work->rotation.y = fn_1_2AD8(work->rotation.y, 0.0f, 0.05f);
        if (work->objectIndex == lbl_1_bss_D60) {
            if (fn_8002D75C(work->model) != 0) {
                work->state = 0xCU;
            }
        } else {
            work->state = 0xCU;
        }
        break;
    case 15:
        fn_1_1B5FC(object, 1, 8.0f, ((1 << 30) | (1 << 0)));
        if (fn_1_1AE10(object, 200.0f + target.x, target.y, target.z - 150.0f, 5.0f, 5.0f) != 0) {
            work->state = 0x10U;
        }
        break;
    case 16:
        fn_1_1B5FC(object, 0, 8.0f, ((1 << 30) | (1 << 0)));
        work->rotation.y = fn_1_2AD8(work->rotation.y, 0.0f, 0.05f);
        if (work->timer >= 0x78) {
            work->timer = 0;
            work->state = 0x11U;
            fn_1_1B5FC(object, 1, 8.0f, ((1 << 30) | (1 << 0)));
        } else {
            work->timer = work->timer + 1;
        }
        break;
    case 17:
        work->rotation.y = fn_1_2AD8(work->rotation.y, 90.0f, 0.05f);
        if (fn_1_1AE10(object, target.x, target.y, target.z - 150.0f, 5.0f, 5.0f) != 0) {
            work->state = 0xDU;
            lbl_1_bss_FC = 5;
            work->position.z = (f32) (target.z - 100.0f);
            work->rotation.y = 0.0f;
            fn_1_1B5FC(object, 0, 8.0f, ((1 << 30) | (1 << 0)));
            MicQuizListenStart();
        }
        break;
    }
    fn_80026760(work->model, work->position.x, work->position.y, work->position.z);
    fn_80026808(work->model, work->rotation.x, work->rotation.y, work->rotation.z);
}

extern void fn_1_1B2A4(OMOBJ *object);

extern void fn_1_1AFC8(OMOBJ *object);

s32 fn_1_1AE10(OMOBJ *object, f32 targetX, f32 unusedHeight, f32 targetZ,
               f32 width, f32 height) {
    Vec targetPosition;
    MicquizModelWork *work;

    targetPosition.x = targetX;
    targetPosition.y = unusedHeight;
    targetPosition.z = targetZ;
    work = object->data;
    work->displacement.x = targetX - work->position.x;
    work->displacement.y = 0.0f;
    work->displacement.z = targetZ - work->position.z;
    PSVECNormalize(&work->displacement, &work->displacement);
    work->rotation.y = (f32)(180.0 * (atan2((f64)work->displacement.x,
                                      (f64)work->displacement.z) /
                                     M_PI));
    work->movement.x = (f32)(5.0 * sin((M_PI *
                                        (f64)work->rotation.y) /
                                       180.0));
    work->movement.y = 0.0f;
    work->movement.z = (f32)(5.0 * cos((M_PI *
                                        (f64)work->rotation.y) /
                                       180.0));
    PSVECAdd(&work->position, &work->movement, &work->position);
    fn_1_1B2A4(object);
    fn_1_1AFC8(object);
    if (fn_1_2A18(&targetPosition, width, &work->position, height) != 0) {
        return 1;
    }
    return 0;
}

/* Moves a memory object toward its next position. */
void fn_1_1AFC8(OMOBJ *object) {
    MicquizModelWork *otherWork;
    MicquizModelWork *work;
    Vec separation;
    Vec direction;
    Vec relativePosition;
    Vec otherPosition;
    f32 distanceFromOrigin;
    f32 otherDistanceFromOrigin;
    f32 strength;
    f32 blend;
    s32 index;

    work = object->data;
    PSVECSubtract(&work->position, &work->previousPosition, &separation);
    distanceFromOrigin = PSVECMag(&separation);
    if (distanceFromOrigin != 0.0f) {
        index = 0;
        while (index < lbl_1_bss_D50) {
            if (index != work->objectIndex) {
                otherWork = lbl_1_bss_DFC[index]->data;
                if (otherWork->state < 3 || otherWork->state > 6 ||
                    otherWork->state != 0xD) {
                    relativePosition.x = work->position.x;
                    relativePosition.y = work->position.y;
                    relativePosition.z = work->position.z;
                    relativePosition.y = 0.0f;
                    otherPosition.x = otherWork->position.x;
                    otherPosition.y = otherWork->position.y;
                    otherPosition.z = otherWork->position.z;
                    otherPosition.y = 0.0f;
                    PSVECSubtract(&relativePosition, &otherPosition,
                        &separation);
                    strength = PSVECMag(&separation);
                    if (!(strength >= 99.5f)) {
                        PSVECSubtract(&otherWork->position,
                            &otherWork->previousPosition, &separation);
                        otherDistanceFromOrigin = PSVECMag(&separation);
                        PSVECSubtract(&relativePosition, &otherPosition,
                            &direction);
                        PSVECNormalize(&direction, &direction);
                        strength = 100.0f - strength;
                        blend = ((0.6f * otherDistanceFromOrigin) +
                            (0.4f * distanceFromOrigin)) /
                            (distanceFromOrigin + otherDistanceFromOrigin);
                        work->position.x += blend * (strength * direction.x);
                        work->position.y += blend * (strength * direction.y);
                        work->position.z += blend * (strength * direction.z);
                        blend = ((0.6f * distanceFromOrigin) +
                            (0.4f * distanceFromOrigin)) /
                            (distanceFromOrigin + otherDistanceFromOrigin);
                        otherWork->position.x += blend * (strength * -direction.x);
                        otherWork->position.y += blend * (strength * -direction.y);
                        otherWork->position.z += blend * (strength * -direction.z);
                    }
                }
            }
            index += 1;
        }
    }
}

/* Turns a memory object toward the direction it moves. */
void fn_1_1B2A4(OMOBJ *object) {
    MicquizModelWork *work;
    Vec targetPosition;
    s32 index;
    f32 differenceX;
    f32 differenceZ;
    f32 distance;

    work = object->data;
    for (index = 0; index < lbl_1_bss_D50; index++) {
        if (work->targetIndex == index) {
            continue;
        }
        targetPosition.x = lbl_1_data_A70[lbl_1_bss_100].positions[index].x;
        targetPosition.y = lbl_1_data_A70[lbl_1_bss_100].positions[index].y;
        targetPosition.z = lbl_1_data_A70[lbl_1_bss_100].positions[index].z;
        differenceX = work->position.x -
            lbl_1_data_A70[lbl_1_bss_100].positions[index].x;
        differenceZ = work->position.z -
            lbl_1_data_A70[lbl_1_bss_100].positions[index].z;
        distance = sqrtf((differenceX * differenceX) +
                         (differenceZ * differenceZ));
        if (fn_1_2A18(&targetPosition, 125.0f, &work->position,
                      50.0f) == 0) {
            continue;
        }
        differenceX /= distance;
        differenceZ /= distance;
        work->position.x = lbl_1_data_A70[lbl_1_bss_100].positions[index].x +
            175.0f * differenceX;
        work->position.z = lbl_1_data_A70[lbl_1_bss_100].positions[index].z +
            175.0f * differenceZ;
    }
}

/* Starts a memory object's motion unless that motion is already playing. */
void fn_1_1B5FC(OMOBJ *obj, s32 motion, f32 blend, u32 attr) {
    MicquizModelWork *work = obj->data;
    if (work->motionState != motion) {
        work->motionState = motion;
        fn_8002CCC0(work->model, work->motions[motion],
                            0.0f, blend, attr);
    }
}

void fn_1_1B67C(OMOBJ *obj, s32 motion, f32 blend, u32 attr) {
    MicquizModelWork *work = obj->data;
    work->motionState = motion;
    fn_8002CCC0(work->model, work->motions[motion], 0.0f, blend, attr);
}

/* Debug view: prints the camera's center, rotation and zoom in a window. */
void fn_1_1B6F0(void) {
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

    if (lbl_802C0008[0] & 0x800) fn_8003E488(1, &lbl_1_data_9FC);

    printWin(0x10, 0x28, 0x180, 0x54, &windowColor);
    fontcolor = FONT_COLOR_YELLOW;
    print8(0x10, 0x28, 1.0f, "Center X:%.02f Y:%.02f Z:%.02f", lbl_80266B18[0].x, lbl_80266B18[0].y,
           lbl_80266B18[0].z);
    print8(0x10, 0x34, 1.0f, "CRot X:%.02f Y:%.02f Z:%.02f", lbl_80266A58[0].x, lbl_80266A58[0].y,
           lbl_80266A58[0].z);
    print8(0x10, 0x40, 1.0f, "CZoom:%.02f", lbl_80266BD8[0]);
}

/* Starts the objects' first motion and puts the first object into its next state. */
void fn_1_1BABC(void) {
    s32 index;
    MicquizModelWork *firstWork;
    OMOBJ *object;
    MicquizModelWork *work;

    for (index = 0; index < lbl_1_bss_D50; index++) {
        firstWork = lbl_1_bss_DFC[index]->data;
        firstWork->state = 1;
        object = lbl_1_bss_DFC[index];
        work = object->data;
        if (work->motionState != 3) {
            work->motionState = 3;
            fn_8002CCC0(work->model,
                work->motions[3], 0.0f,
                8.0f, 0);
        }
    }
}

/* Returns 1 once every object has reached state 0xD. */
s32 fn_1_1BB8C(void) {
    s32 index;
    MicquizModelWork *work;

    for (index = 0; index < lbl_1_bss_D50; index++) {
        work = lbl_1_bss_DFC[index]->data;
        if (work->state != 0xD) {
            return 0;
        }
    }
    return 1;
}

s32 fn_1_1BD00(s32 value);

/* Gives each object a random target place no other object took. */
void fn_1_1BBF8(void) {
    s32 index;
    MicquizModelWork *work;
    s32 selection;

    for (index = 0; index < 8; index++) {
        lbl_1_bss_D68[index] = -1;
    }
    for (index = 0; index < lbl_1_bss_D50; index++) {
        do {
            selection = frandmod(lbl_1_bss_D50);
        } while (fn_1_1BD00(selection) == 0);
        lbl_1_bss_D68[index] = selection;
    }
    for (index = 0; index < lbl_1_bss_D50; index++) {
        work = lbl_1_bss_DFC[index]->data;
        work->targetIndex = lbl_1_bss_D68[index];
    }
}

/* Returns 1 when no object has the given target yet. */
s32 fn_1_1BD00(s32 value) {
    s32 index;

    for (index = 0; index < lbl_1_bss_D50; index++) {
        if (lbl_1_bss_D68[index] != -1 && value == lbl_1_bss_D68[index]) {
            return 0;
        }
    }
    return 1;
}

s32 fn_1_1BE5C(s32 value);

/* Fills the player list with eight different random values and reports it. */
void fn_1_1BD78(void) {
    s32 index;
    s32 selection;

    for (index = 0; index < 8; index++) {
        lbl_1_bss_D88[index] = -1;
    }
    OSReport("PlList\n");

    for (index = 0; index < lbl_1_bss_D50; index++) {
        do {
            selection = frandmod(0xB);
        } while (selection == 8 || fn_1_1BE5C(selection) == 0);
        lbl_1_bss_D88[index] = selection;
        OSReport("%d,", selection);
    }
    OSReport("\n");
}

/* Returns 1 when the player list does not hold the value yet. */
s32 fn_1_1BE5C(s32 value) {
    s32 i;
    for (i = 0; i < 8; i++) {
        if (lbl_1_bss_D88[i] != -1 && value == lbl_1_bss_D88[i]) {
            return 0;
        }
    }
    return 1;
}

s32 fn_1_1BFA4(s32 value);

/* Fills the object list with eight different random values and reports it. */
void fn_1_1BEC8(void) {
    s32 index;
    s32 selection;

    for (index = 0; index < 8; index++) {
        lbl_1_bss_DA8[index] = -1;
    }
    OSReport("ObjList\n");

    for (index = 0; index < lbl_1_bss_D50; index++) {
        do {
            selection = frandmod(8);
        } while (fn_1_1BFA4(selection) == 0);
        lbl_1_bss_DA8[index] = selection;
        OSReport("%d,", selection);
    }
    OSReport("\n");
}

/* Returns 1 when the object list does not hold the value yet. */
s32 fn_1_1BFA4(s32 value) {
    s32 i;
    for (i = 0; i < 8; i++) {
        if (lbl_1_bss_DA8[i] != -1 && value == lbl_1_bss_DA8[i]) {
            return 0;
        }
    }
    return 1;
}

void MicQuizExpectedAnswerSet(s32 result);

/* Picks the question and the answer index at random, reports them and records the answer. */
void fn_1_1C010(void) {
    s32 randomSelection;
    s32 index;
    MicquizModelWork *work;
    s32 nextSelection;
    s32 result;

    randomSelection = frandmod(lbl_1_bss_D50);
    lbl_1_bss_D64 = randomSelection;
    for (index = 0; index < lbl_1_bss_D50; index++) {
        work = lbl_1_bss_DFC[index]->data;
        if (randomSelection == work->targetIndex) {
            result = lbl_1_bss_D88[index];
            lbl_1_bss_D60 = index;
            OSReport("=================MemAnserIdx:%d\n", lbl_1_bss_D60);
            OSReport("=================ndp->kind:%d\n", work->sourceSequence);
            OSReport("=================ques:%d\n", randomSelection);
            OSReport("=================ans:%d\n", result);
            break;
        }
    }
    MicQuizExpectedAnswerSet(result);
    do {
        nextSelection = frandmod(lbl_1_bss_D50);
    } while (nextSelection == randomSelection);
    lbl_1_bss_D5C = nextSelection;
}

void fn_1_1C14C(void)
{
    Vec positions[2];
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
