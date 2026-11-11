/* Comparison round with moving objects on two sides, the answer and its effects. */
#include "msm_stream.h"
#include "math.h"

#include "dolphin/gx.h"

#include "dolphin/types.h"

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

#include "game/process.h"

#include "game/printfunc.h"

#include "game/window.h"

#include "game/hu3d.h"

#include "humath.h"

#include "msm_se.h"

#include "REL/micquizishidll/shared_types.h"

#include "REL/micquizishidll/micquizishidll_callback_views_v1.h"

#include <dolphin/mtx.h>

#include <string.h>

void fn_1_229E0(void);

void fn_1_239D4(OMOBJ *object);

void fn_1_23878(void);

void fn_1_245FC(OMOBJ *object, Vec *position, Vec *velocity, s32 side, f32 travel,
                f32 velocityScale);

void fn_1_24248(Vec *position, Vec *velocity, s32 side, f32 travel, f32 velocityScale);

void fn_1_23E44(OMOBJ *object);

void fn_1_24A68(void *source, HuVecF *position, s32 mode, s32 index);

/* The release debug hook evaluates its argument without retaining it. */
static inline void DebugValue(s32 value) { }

s32 fn_1_259E8(Vec *position0, f32 radius0, Vec *position1, f32 radius1);

void fn_1_25A90(void);

void fn_1_252CC(void);

void fn_1_25368(void);

void fn_1_24FA4(void);

s32 fn_1_262D4(s32 effectIndex, s32 blendMode);

void fn_1_26438(f32 positionX, f32 positionY, f32 positionZ, s32 modelIndex, f32 rotationY);

s16 fn_1_26168(ANIMDATA *animation, s32 blendMode, s32 alternatePosition);

void fn_1_26558(HU3D_MODEL *model, HU3D_PARTICLE *particleData, Mtx matrix);

typedef struct MicQuizModelPair {
    s16 firstModelId;
    s16 secondModelId;
} MicQuizModelPair;

extern s16 lbl_1_bss_18BC[2];

extern s16 lbl_1_bss_18C2[2];

extern s16 lbl_1_bss_18C6[2];

void MicQuizCountdownInit(void);

u8 MicQuizQuestionLevelGet(void);

extern s32 lbl_1_bss_100;

extern s32 lbl_1_bss_110;

extern s32 lbl_1_bss_20;

extern f32 lbl_1_bss_104;

extern f32 lbl_1_bss_8C;

extern u32 lbl_1_bss_FC;

extern HUPROCESS *lbl_1_bss_114[0xB];

extern s32 lbl_1_bss_1820[2];

extern ANIMDATA *lbl_1_bss_1830[2];

extern s32 lbl_1_bss_1840;

extern OMOBJ *lbl_1_bss_1844[2][12];

extern s16 lbl_1_bss_18A4;

extern s16 lbl_1_bss_18A6;

extern ANIMDATA *lbl_1_bss_18A8;

extern s32 lbl_1_bss_18AC[2];

extern s16 lbl_1_bss_18B4[2];

extern s16 lbl_1_bss_18B8[2];

extern s16 lbl_1_bss_18C0;

extern s16 lbl_1_bss_18CA;

extern s16 lbl_1_bss_9C[2];

extern s16 lbl_1_bss_A0;

extern s16 lbl_1_bss_A4;

extern OMOBJ *lbl_1_bss_A8;

extern OM_CAMERA_VIEW lbl_1_data_1714;

extern Point3d lbl_1_data_1730;

extern Point3d lbl_1_data_173C;

extern Point3d lbl_1_data_1748;

extern s32 lbl_1_data_17E4[2];

extern s32 lbl_1_data_17EC[5];

extern Point3d lbl_1_data_470;

extern Point3d lbl_1_data_47C;

extern u8 lbl_1_data_488[4];

void MicQuizExpectedAnswerSet(s32);

void MicQuizPauseInit(HUPROCESS *);

void fn_1_255E8(void);

u8 lbl_1_data_16F8[0x1C] = { 0x00, 0xA4, 0x00, 0x7A, 0x00, 0xA4, 0x00, 0x7B, 0x00, 0xA4, 0x00, 0x7C,
        0x00, 0xA4, 0x00, 0x7D, 0x00, 0xA4, 0x00, 0x7E, 0x00, 0xA4, 0x00, 0x7F, 0x00, 0xA4, 0x00, 0x80 };
OM_CAMERA_VIEW lbl_1_data_1714 = { { 0.0f, 360.0f, 0.0f }, { -5.05f, 0.0f, 0.0f }, 1442.0f };
Point3d lbl_1_data_1730 = { -100.0f, 8000.0f, -400.0f };
Point3d lbl_1_data_173C = { 0.0f, 1.0f, 0.0f };
Point3d lbl_1_data_1748 = { 0.0f, 0.0f, -400.0f };
HuVecF lbl_1_data_1754[12] = { { -130.0f, 150.0f, -200.0f }, { 130.0f, 150.0f, -200.0f },
        { 130.0f, 150.0f, 200.0f }, { -130.0f, 150.0f, 200.0f }, { 0.0f, 300.0f,
                -220.0f }, { 220.0f, 300.0f, 0.0f }, { 0.0f, 300.0f, -220.0f }, { -220.0f, 300.0f,
                        0.0f }, { -130.0f, 450.0f, -200.0f }, { 130.0f, 450.0f, -200.0f }, { 130.0f,
                                450.0f, 200.0f }, { -130.0f, 450.0f, 200.0f } };
s32 lbl_1_data_17E4[2] = { 0xA40068, 0xA40069 };
s32 lbl_1_data_17EC[5] = { 4, 6, 8, 10, 12 };
s32 lbl_1_data_1800[5] = { 1, 2, 3, 4, 5 };

/* Sets up the comparison round: picks the left and right answers, creates the camera, lights and
   models, and reports which side is correct. */
void fn_1_229E0(void) {
    s32 player;
    s32 slot;
    s32 animation;

    lbl_1_bss_104 = 0.0f;
    lbl_1_bss_1840 = 0;
    lbl_1_bss_100 = MicQuizQuestionLevelGet();
    MicQuizCountdownInit();
    lbl_1_bss_FC = 0;
    lbl_1_bss_8C = 13.333333f;
    MicQuizPortraitShow(MicQuizCurrentPlayerGet());

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 0x280, 0x1E0);
    fn_8003E488(1, &lbl_1_data_1714);
    lbl_1_bss_A8 = fn_8003AB38(*lbl_1_bss_114, 0x7FDA, 0, 0, -1, fn_8003DAA4);
    lbl_1_bss_A8->work[0] = 1;

    fn_800285BC();
    lbl_1_bss_110 = Hu3DGLightCreate(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0);
    Hu3DGLightPosAimSetV(lbl_1_bss_110, &lbl_1_data_470, &lbl_1_data_47C);
    Hu3DGLightStaticSet(lbl_1_bss_110, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_110);
    Hu3DGLightColorSet(lbl_1_bss_110, lbl_1_data_488[0], lbl_1_data_488[1],
                       lbl_1_data_488[2], lbl_1_data_488[3]);
    Hu3DShadowCreate(20.0f, 10.0f, 8000.0f);
    fn_8002AAB4(0x10, 0x10, 0x10);
    Hu3DShadowPosSet(&lbl_1_data_1730, &lbl_1_data_173C, &lbl_1_data_1748);
    Hu3DShadowTPLvlSet(0.15f);

    lbl_1_bss_A4 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 89), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_A4, 1);
    fn_80026B8C(lbl_1_bss_A4, 0);
    lbl_1_bss_A0 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 43), 0x10000000, 2));
    lbl_1_bss_9C[0] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 44), 0x10000000, 2));
    lbl_1_bss_9C[1] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 45), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_A0, 1);
    fn_80026B64(lbl_1_bss_9C[0], 1);
    fn_80026B64(lbl_1_bss_9C[1], 1);
    fn_80026760(lbl_1_bss_A0, 0.0f, 130.0f, 0.0f);
    fn_80026B8C(lbl_1_bss_A0, 3);
    fn_80026B8C(lbl_1_bss_9C[0], 3);
    fn_80026B8C(lbl_1_bss_9C[1], 3);

    lbl_1_bss_18C6[0] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 85), 0x10000000, 2));
    lbl_1_bss_18C6[1] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 86), 0x10000000, 2));
    fn_80026FFC(lbl_1_bss_18C6[0]);
    fn_80026FFC(lbl_1_bss_18C6[1]);
    lbl_1_bss_18C2[0] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 87), 0x10000000, 2));
    lbl_1_bss_18C2[1] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 88), 0x10000000, 2));
    fn_80026760(lbl_1_bss_18C6[0], -350.0f, 0.0f, -450.0f);
    fn_80026760(lbl_1_bss_18C6[1], 350.0f, 0.0f, -450.0f);
    fn_80026760(lbl_1_bss_18C2[0], -350.0f, 0.0f, -450.0f);
    fn_80026760(lbl_1_bss_18C2[1], 350.0f, 0.0f, -450.0f);
    lbl_1_bss_18C0 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 100), 0x10000000, 2));
    fn_80026B8C(lbl_1_bss_18C0, 2);
    lbl_1_bss_18BC[0] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 101), 0x10000000, 2));
    fn_80026B8C(lbl_1_bss_18BC[0], 2);
    fn_80026760(lbl_1_bss_18BC[0], -350.0f, 830.0f, -450.0f);
    lbl_1_bss_18BC[1] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 101), 0x10000000, 2));
    fn_80026B8C(lbl_1_bss_18BC[1], 2);
    fn_80026760(lbl_1_bss_18BC[1], 350.0f, 830.0f, -450.0f);

    lbl_1_bss_18AC[0] = lbl_1_data_17EC[lbl_1_bss_100];
    lbl_1_bss_18AC[1] = lbl_1_bss_18AC[0];
    if (frandmod(2) != 0U) {
        lbl_1_bss_18AC[1]--;
        lbl_1_bss_18CA = 0;
    } else {
        lbl_1_bss_18AC[0]--;
        lbl_1_bss_18CA = 1;
    }
    MicQuizExpectedAnswerSet(lbl_1_bss_18CA);

    for (player = 0; player < 2; ++player) {
        for (slot = 0; slot < 12; ++slot) {
            lbl_1_bss_1844[player][slot] = NULL;
        }
    }
    lbl_1_bss_1820[0] = frandmod(2);
    lbl_1_bss_1820[1] = frandmod(2);
    lbl_1_bss_1830[0] = HuSprAnimRead(fn_80008634(lbl_1_data_17E4[lbl_1_bss_1820[0]], 0x10000000));
    lbl_1_bss_1830[1] = HuSprAnimRead(fn_80008634(lbl_1_data_17E4[lbl_1_bss_1820[1]], 0x10000000));

    for (player = 0; player < 2; ++player) {
        if (lbl_1_bss_1820[player] == 0) {
            lbl_1_bss_18B8[player] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 102), 0x10000000,
                    2));
            lbl_1_bss_18B4[player] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 98), 0x10000000, 2));
        } else {
            lbl_1_bss_18B8[player] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 103), 0x10000000,
                    2));
            lbl_1_bss_18B4[player] = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 99), 0x10000000, 2));
        }
        if (player == 0) {
            fn_80026760(lbl_1_bss_18B8[player], -350.0f, 430.0f, -400.0f);
            fn_80026760(lbl_1_bss_18B4[player], -350.0f, 430.0f, -450.0f);
        } else {
            fn_80026760(lbl_1_bss_18B8[player], 350.0f, 430.0f, -400.0f);
            fn_80026760(lbl_1_bss_18B4[player], 350.0f, 430.0f, -450.0f);
        }
        fn_80026B8C(lbl_1_bss_18B8[player], 1);
        fn_800268B0(lbl_1_bss_18B8[player], 0.0f, 0.0f, 0.0f);
        fn_800268B0(lbl_1_bss_18B4[player], 0.0f, 0.0f, 0.0f);
        fn_800269E8(lbl_1_bss_18B8[player], 1);
        fn_800269E8(lbl_1_bss_18B8[player], ((1 << 30) | (1 << 0)));
        fn_800269E8(lbl_1_bss_18B4[player], 1);
        fn_800269E8(lbl_1_bss_18B4[player], ((1 << 30) | (1 << 1)));
        for (slot = 0; slot < lbl_1_bss_18AC[player]; ++slot) {
            lbl_1_bss_1844[player][slot] =
                fn_8003AB38(*lbl_1_bss_114, 0x64, 1, 3, -1, fn_1_239D4);
            lbl_1_bss_1844[player][slot]->data =
                HuMemDirectMallocNum(HEAP_HEAP, 108, 0x10000000);
            lbl_1_bss_1844[player][slot]->work[0] =
                lbl_1_bss_1820[player];
            lbl_1_bss_1844[player][slot]->work[1] = slot;
            lbl_1_bss_1844[player][slot]->work[2] = player;
        }
    }

    if (lbl_1_bss_18AC[0] > lbl_1_bss_18AC[1]) {
        OSReport("COMP ANSER:LEFT\n");
    } else {
        OSReport("COMP ANSER:RIGHT\n");
    }
    OSReport("COMP LEFT:%d RIGHT:%d\n", lbl_1_bss_18AC[0], lbl_1_bss_18AC[1]);
    lbl_1_bss_18A8 = HuSprAnimRead(fn_80008DD0(DATANUM(0xa40000, 95), 0x10000000, 2));
    lbl_1_bss_18A6 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 96), 0x10000000, 2));
    lbl_1_bss_18A4 = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 97), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_18A6, 1);
    fn_800269E8(lbl_1_bss_18A4, 1);
    MicQuizPauseInit(*lbl_1_bss_114);
    MicQuizMicContextSet(5);
    fn_8000DAC0(fn_1_255E8, 0x64, 0x1800, 0, *lbl_1_bss_114);
    fn_8000DAC0(fn_1_23878, 0x64, 0x1800, 0, *lbl_1_bss_114);
    lbl_1_bss_20 = HuAudSStreamPlay(MSM_STREAM_QUESTION_MUSIC_FOURTH);
}

/* Turns the stage models every frame, two pairs one way and one pair the other, half as fast. */
void fn_1_23878(void) {
    f32 firstAngle = 0.0f;
    f32 secondAngle = firstAngle;

    for (;;) {
        HuPrcVSleep();
        secondAngle += 1.0f;
        firstAngle += 0.5f;

        fn_80026808(lbl_1_bss_18C6[0], 0.0f, secondAngle, 0.0f);
        fn_80026808(lbl_1_bss_18C6[1], 0.0f, secondAngle, 0.0f);
        fn_80026808(lbl_1_bss_18BC[0], 0.0f, secondAngle, 0.0f);
        fn_80026808(lbl_1_bss_18BC[1], 0.0f, secondAngle, 0.0f);
        fn_80026808(lbl_1_bss_18C2[0], 0.0f, -firstAngle, 0.0f);
        fn_80026808(lbl_1_bss_18C2[1], 0.0f, -firstAngle, 0.0f);
    }
}

/* Creates one object of a side: its model, a random direction and speed, then hands it to its
   moving step. */
void fn_1_239D4(OMOBJ *object) {
    QuizParticleWork *work = object->data;
    f32 direction;
    f32 amount;
    f32 angle;

    if (object->work[0] == 0) {
        work->model = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 98), 0x10000000, 2));
    } else {
        work->model = fn_80025A04(fn_80008DD0(DATANUM(0xa40000, 99), 0x10000000, 2));
    }
    fn_80026E44(work->model);
    work->state = 4;
    work->modelVariant = object->work[0];
    work->positionIndex = object->work[1];
    work->side = object->work[2];
    work->position.x = work->position.y = work->position.z = 1e+06f;
    direction = 2.0f * frandf();
    amount = frandf();

    if (work->side == 0) {
        work->offset.x = (f32)((10.0 * ((f64)amount * sin(M_PI * (f64)direction))) - 350.0);
    } else {
        work->offset.x = (f32)(350.0 + (10.0 * ((f64)amount * sin(M_PI * (f64)direction))));
    }
    work->offset.y = (f32)(130.0 + (300.0 + (50.0 * ((f64)amount * cos(M_PI * (f64)direction)))));
    work->offset.z = (f32)((50.0 * ((f64)amount * cos(M_PI * (f64)direction))) - 450.0);
    work->active = 1;
    work->frame = 0;

    angle = 1.4117647f * (u8)frand();
    work->velocity.x = (f32)(sin((M_PI * (f64)angle) / 180.0)
                             * cos((M_PI * (f64)angle) / 180.0));
    work->velocity.z = (f32)(cos((M_PI * (f64)angle) / 180.0)
                             * cos((M_PI * (f64)angle) / 180.0));
    work->velocity.y = (f32)(cos((M_PI * (f64)angle) / 180.0)
                             * cos((M_PI * (f64)angle) / 180.0));
    work->velocityScale = 5.0f;
    PSVECNormalize(&work->velocity, &work->velocity);
    work->velocity.x *= work->velocityScale;
    work->velocity.y *= work->velocityScale;
    work->velocity.z *= work->velocityScale;
    fn_800269E8(work->model, 0x40000001U);
    fn_1_24A68(object, &work->position, work->side, work->positionIndex);
    work->scale.x = work->scale.y = work->scale.z = 0.5f;
    work->rotation.x = work->rotation.y = work->rotation.z = 0.0f;
    fn_800268B0(work->model, work->scale.x, work->scale.y, work->scale.z);
    fn_80026760(work->model, work->position.x, work->position.y, work->position.z);
    fn_80026808(work->model, work->rotation.x, work->rotation.y, work->rotation.z);
    object->objFunc = fn_1_23E44;
}

/* Moves an object each frame, reflecting it off its side's bounds, and updates its model. */
void fn_1_23E44(OMOBJ *object) {
    QuizParticleWork *work = object->data;

    if (work->state == 1) {
        if (work->frame >= 60) {
            work->frame = 0;
            work->state = 3;
        } else {
            work->rotation.y += 5.0f;
            work->scale.x -= 0.01f;
            work->scale.y -= 0.01f;
            work->scale.z -= 0.01f;
            work->frame++;
        }
    } else if (work->state == 2) {
        if (work->frame >= 60) {
            work->frame = 0;
            work->state = 4;
        } else {
            work->rotation.y += 5.0f;
            work->scale.x += 0.01f;
            work->scale.y += 0.01f;
            work->scale.z += 0.01f;
            work->frame++;
        }
    } else if (work->state == 3) {
        work->rotation.y += 5.0f;
    } else if (work->state == 5) {
        work->active = 0;
        PSVECSubtract(&work->offset, &work->position, &work->velocity);
        work->frame = 0;
        work->state = 6;
        work->angle = 0.0f;
        work->origin = work->position;
    } else if (work->state == 6) {
        work->position.x = (f32)((f64)work->origin.x + (f64)work->velocity.x
                                 * sin(M_PI * (f64)work->angle / 180.0));
        work->position.y = (f32)((f64)work->origin.y + (f64)work->velocity.y
                                 * sin(M_PI * (f64)work->angle / 180.0));
        work->position.z = (f32)((f64)work->origin.z + (f64)work->velocity.z
                                 * sin(M_PI * (f64)work->angle / 180.0));
        work->scale.x = work->scale.y = work->scale.z =
            (f32)(0.5 * sin(M_PI * (f64)(90.0f - work->angle) / 180.0));
        work->angle += 1.0f;
        work->frame++;
        if (work->frame > 90) {
            work->state = 7;
        }
    } else {
        DebugValue(work->state == 7);
    }

    if (work->active != 0) {
        PSVECAdd(&work->position, &work->velocity, &work->position);
        fn_1_24248(&work->position, &work->velocity, work->side, 50.0f, work->velocityScale);
        fn_1_245FC(object, &work->position, &work->velocity, work->side, 50.0f,
                   work->velocityScale);
    }
    fn_800268B0(work->model, work->scale.x, work->scale.y, work->scale.z);
    fn_80026760(work->model, work->position.x, work->position.y, work->position.z);
    fn_80026808(work->model, work->rotation.x, work->rotation.y, work->rotation.z);
}

/* Advances a position along its velocity and reflects the velocity off the side's bounds. */
void fn_1_24248(Vec *position, Vec *velocity, s32 side, f32 travel, f32 velocityScale) {
    Vec target;
    Vec normal;
    Vec current;
    Vec targetAtGround;
    f32 dx;
    f32 dz;
    f32 distance;

    if (side == 0) {
        target.x = -350.0f;
        target.y = 130.0f;
        target.z = -450.0f;
    } else {
        target.x = 350.0f;
        target.y = 130.0f;
        target.z = -450.0f;
    }
    targetAtGround = target;
    targetAtGround.y = 0.0f;

    dx = position->x - target.x;
    dz = position->z - target.z;
    distance = sqrtf(dx * dx + dz * dz);
    if (distance > 250.0f) {
        dx /= distance;
        dz /= distance;
        position->x = target.x + dx * (300.0f - travel);
        position->z = target.z + dz * (300.0f - travel);
        current.x = position->x;
        current.y = 0.0f;
        current.z = position->z;
        PSVECSubtract(&targetAtGround, &current, &normal);
        PSVECNormalize(&normal, &normal);
        C_VECReflect(velocity, &normal, velocity);
        velocity->x *= velocityScale;
        velocity->y *= velocityScale;
        velocity->z *= velocityScale;
    }

    if (position->y < 180.0f) {
        position->y = 180.0f;
        velocity->y *= (-1.0f);
    }
    if (position->y > 680.0f) {
        position->y = 680.0f;
        velocity->y *= (-1.0f);
    }
}

void fn_1_245FC(OMOBJ *object, Vec *, Vec *, s32 side,
                f32, f32 velocityScale) {
    QuizParticleWork *work = object->data;
    QuizParticleWork *otherWork;
    s32 groupNo;
    s32 objectNo;
    f32 distanceSquared;
    f32 distance;
    Vec target;

    if (side == 0) {
        target.x = -350.0f;
        target.y = 130.0f;
        target.z = -450.0f;
    } else {
        target.x = 350.0f;
        target.y = 130.0f;
        target.z = -450.0f;
    }

    for (groupNo = 0; groupNo < 2; groupNo++) {
        for (objectNo = 0; objectNo < lbl_1_bss_18AC[groupNo]; objectNo++) {
            if (lbl_1_bss_1844[groupNo][objectNo] != object
                && lbl_1_bss_1844[groupNo][objectNo] != NULL) {
                otherWork = lbl_1_bss_1844[groupNo][objectNo]->data;
                if (fn_1_259E8(&work->position, 50.0f,
                              &otherWork->position, 50.0f)) {
                    f32 separationX;
                    f32 separationY;
                    f32 separationZ;
                    Vec previousPosition;
                    Vec normal;

                    distanceSquared = ((work->position.z - otherWork->position.z)
                                       * (work->position.z - otherWork->position.z))
                        + (((work->position.x - otherWork->position.x)
                            * (work->position.x - otherWork->position.x))
                           + ((work->position.y - otherWork->position.y)
                              * (work->position.y - otherWork->position.y)));
                    distance = (f32)((100.0 - sqrt(distanceSquared))
                                     / (f64)velocityScale);
                    work->position.x -= distance * work->velocity.x;
                    work->position.y -= distance * work->velocity.y;
                    work->position.z -= distance * work->velocity.z;

                    separationX = work->position.x - otherWork->position.x;
                    separationY = work->position.y - otherWork->position.y;
                    separationZ = work->position.z - otherWork->position.z;
                    previousPosition.x = work->position.x - separationX;
                    previousPosition.y = work->position.y - separationY;
                    previousPosition.z = work->position.z - separationZ;
                    PSVECSubtract(&otherWork->position, &work->position, &normal);
                    PSVECNormalize(&normal, &normal);
                    C_VECReflect(&work->velocity, &normal, &work->velocity);
                    work->velocity.x *= work->velocityScale;
                    work->velocity.y *= work->velocityScale;
                    work->velocity.z *= work->velocityScale;
                }
            }
        }
    }
}

/* Gives the position of an object slot: the left or right side's base plus the slot's offset. */
void fn_1_24A68(void *source, HuVecF *position, s32 mode, s32 index) {
    HuVecF baseOffset;

    if (mode == 0) {
        baseOffset.x = -350.0f;
        baseOffset.y = 130.0f;
        baseOffset.z = -450.0f;
    } else {
        baseOffset.x = 350.0f;
        baseOffset.y = 130.0f;
        baseOffset.z = -450.0f;
    }
    position->x = baseOffset.x + lbl_1_data_1754[index].x;
    position->y = baseOffset.y + lbl_1_data_1754[index].y;
    position->z = baseOffset.z + lbl_1_data_1754[index].z;
}

void MicQuizQuestionReadySet(void);

void MicQuizListenStart(void);

s32 MicQuizCountdownUpdate(void);

void fn_1_235C(void);

extern s16 micQuizGameMes;

void fn_1_25524(void);

s32 MicQuizResponseStateGet(void);

/* Runs the comparison round's mode steps: the question message, the objects, the answer, and the
   closing wipe. */
void fn_1_24B40(void) {
    MicQuizCountdownUpdate();

    switch (lbl_1_bss_FC) {
    case 0:
        micQuizGameMes = GameMesCreate(2, 0);
        lbl_1_bss_FC = 1;
        return;
    case 1:
        if (GameMesStatGet(micQuizGameMes) == 0) {
            lbl_1_bss_FC = 3;
            lbl_1_bss_104 = 0.0f;
            fn_1_235C();
        }
        break;
    case 3:
        if (lbl_1_bss_104 >= 45.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 4;
            MicQuizQuestionReadySet();
        } else {
            lbl_1_bss_104 += 1.0f;
            fn_1_25524();
        }
        /* fall through */
    case 4:
        if (MicQuizResponseStateGet() == 3 || MicQuizResponseStateGet() == 4) {
            lbl_1_bss_FC = 8;
            return;
        }
        if (MicQuizResponseStateGet() == 2) {
            lbl_1_bss_FC = 5;
            return;
        }
        DebugValue(MicQuizResponseStateGet() == 0);
        break;
    case 5:
        lbl_1_bss_FC = 6;
        fn_1_252CC();
        return;
    case 6:
        if (lbl_1_bss_104 >= 300.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 7;
            fn_1_25368();
            return;
        }
        lbl_1_bss_104 += 1.0f;
        return;
    case 7:
        if (lbl_1_bss_104 >= 60.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 4;
            MicQuizListenStart();
            return;
        }
        lbl_1_bss_104 += 1.0f;
        return;
    case 8:
        lbl_1_bss_FC = 9;
        return;
    case 9:
        lbl_1_bss_FC = 10;
        fn_8000DAC0(fn_1_25A90, 0x64, 0x1800, 0,
                         fn_8000DD9C());
        return;
    case 10:
        if (lbl_1_bss_1840 != 0) {
            lbl_1_bss_FC = 11;
            return;
        }
        break;
    case 11:
        if (lbl_1_bss_104 >= 120.0f) {
            lbl_1_bss_104 = 0.0f;
            lbl_1_bss_FC = 12;
            WipeCreate(2, 0, 0x14);
            HuAudSStreamFadeOut(lbl_1_bss_20, 0x1F4);
            return;
        }
        lbl_1_bss_104 += 1.0f;
        return;
    case 12:
        if (WipeCheck() == 0) {
            HuMCSelWinContextKill();
            HuMCClose();
            OSReport(" COMP END==========================\n");
            fn_8003A5E4(1, 1);
        }
        break;
    }
}

/* Starts the result display process with two flags packed into its property. */
void fn_1_24F40(u8 firstFlag, u8 secondFlag) {
    HUPROCESS *process = 0;

    process = fn_8000D818(fn_1_24FA4, 0x64, 0x1800, 0);
    process->property = (void *)((firstFlag << 16) | (u16)secondFlag);
}

typedef struct QuizAnimationProcess {
    u8 unknown_000[0x12C]; /* No code names this field. */
    u32 animationSelection;
} QuizAnimationProcess;

extern char lbl_1_data_18A8[];

extern char lbl_1_data_18AE[];

extern char lbl_1_data_18A8[6];

extern char lbl_1_data_18AE[5];

char lbl_1_data_18A8[6] = "10fig";
char lbl_1_data_18AE[5] = "1fig";

/* Shows the result: switches the counter models' animation banks and swings them. */
void fn_1_24FA4(void) {
    QuizAnimationProcess *process;
    u32 selection;
    s16 model;
    u16 minorSelection;
    HU3D_ANIMID scaleAnimation;
    HU3D_ANIMID textureAnimation;
    s32 majorBank;
    s32 minorBank;
    s32 frame;
    u8 majorSelection;
    Vec position;
    f32 angle;
    f32 scaleX;
    f32 scaleY;
    f32 transparency;
    f32 verticalStep;

    process = (QuizAnimationProcess *)fn_8000DD9C();
    selection = process->animationSelection;
    majorSelection = (u8)(selection >> 16);
    minorSelection = (u16)selection;
    scaleX = scaleY = 0.0f;
    transparency = 1.0f;
    verticalStep = 2.2222223f;
    frame = 0;
    angle = 0.0f;

    if (majorSelection >= 10) {
        model = fn_80025D4C(lbl_1_bss_18A4);
        fn_80026A44(model, 1);
        scaleAnimation = fn_80030B2C(lbl_1_bss_18A8, model,
                                        lbl_1_data_18A8);
        textureAnimation = fn_80030B2C(lbl_1_bss_18A8, model,
                                          lbl_1_data_18AE);
        majorBank = majorSelection / 10;
        minorBank = majorSelection - (majorBank * 10);
        Hu3DAnimBankSet(scaleAnimation, majorBank);
        Hu3DAnimBankSet(textureAnimation, minorBank);
    } else {
        model = fn_80025D4C(lbl_1_bss_18A6);
        fn_80026A44(model, 1);
        textureAnimation = fn_80030B2C(lbl_1_bss_18A8, model,
                                          lbl_1_data_18AE);
        Hu3DAnimBankSet(textureAnimation, majorSelection);
    }

    if (minorSelection == 0) {
        position.x = -350.0f;
        position.y = 280.0f;
        position.z = -390.0f;
    } else {
        position.x = 350.0f;
        position.y = 280.0f;
        position.z = -390.0f;
    }
    fn_80026790(model, &position);
    fn_800268B0(model, scaleX, scaleY, 1.0f);
    fn_80026CBC(model, transparency);

    for (;;) {
        HuPrcVSleep();
        position.y += verticalStep;
        scaleX = scaleY =
            (f32)sin((M_PI * (f64)angle) / 180.0);
        transparency = (f32)sin((M_PI * (f64)angle) /
                               180.0);
        angle += 2.0f;
        fn_80026790(model, &position);
        fn_800268B0(model, scaleX, scaleY, 1.0f);
        fn_80026CBC(model, transparency);
        frame++;
        if ((f32)frame >= 90.0f) {
            break;
        }
    }
    fn_800262D0(model);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Puts every object of both sides into state 1. */
void fn_1_252CC(void) {
    s32 index;
    s32 group;
    QuizParticleWork *work;
    s32 count;

    count = lbl_1_data_1800[lbl_1_bss_100];

    for (group = 0; group < 2; group++) {
        for (index = 0; index < count; index++) {
            work = lbl_1_bss_1844[group][index]->data;
            work->state = 1;
        }
    }
}

/* Puts every object of both sides into state 2. */
void fn_1_25368(void) {
    s32 index;
    s32 group;
    QuizParticleWork *work;
    s32 count;

    count = lbl_1_data_1800[lbl_1_bss_100];

    for (group = 0; group < 2; group++) {
        for (index = 0; index < count; index++) {
            work = lbl_1_bss_1844[group][index]->data;
            work->state = 2;
        }
    }
}

/* Puts every object of both sides into state 5. */
void fn_1_25404(void) {
    s32 group;
    s32 index;

    for (group = 0; group < 2; group++) {
        for (index = 0; index < lbl_1_bss_18AC[group]; index++) {
            QuizParticleWork *work = lbl_1_bss_1844[group][index]->data;
            work->state = 5;
        }
    }
}

/* Returns 1 once every object of both sides has reached state 7. */
s32 fn_1_2548C(void) {
    s32 group;
    s32 index;

    for (group = 0; group < 2; group++) {
        for (index = 0; index < lbl_1_bss_18AC[group]; index++) {
            QuizParticleWork *work = lbl_1_bss_1844[group][index]->data;
            if (work->state != 7) {
                return 0;
            }
        }
    }
    return 1;
}

typedef signed short s16;

typedef float f32;

void fn_1_25524(void)
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

/* Recipient symbol bindings for a retained source (copy of micquizishidll fn_1_22434). */

void fn_1_255E8(void) {
    MicQuizResponseRun(5);
}

extern char lbl_1_data_18B3[];

extern char lbl_1_data_18D2[];

extern char lbl_1_data_18EF[];

extern char lbl_1_data_18B3[31];

extern char lbl_1_data_18D2[29];

extern char lbl_1_data_18EF[13];

char lbl_1_data_18B3[31] = "Center X:%.02f Y:%.02f Z:%.02f";
char lbl_1_data_18D2[29] = "CRot X:%.02f Y:%.02f Z:%.02f";
char lbl_1_data_18EF[13] = "CZoom:%.02f";

/* Debug view: prints the camera's center, rotation and zoom in a window. */
void fn_1_2560C(void) {
    GXColor windowColor;

    windowColor.r = 0;
    windowColor.g = 0;
    windowColor.b = 0x90;
    windowColor.a = 0xC0;

    lbl_80266BD8[0] += (f32)(lbl_802BFFEC[0] / 5);
    lbl_80266BD8[0] -= (f32)(lbl_802BFFE8[0] / 5);
    if (lbl_802C0010[0] & 0x4) lbl_80266B18[0].y -= 5.0f;
    if (lbl_802C0010[0] & 0x8) lbl_80266B18[0].y += 5.0f;
    if (lbl_802C0010[0] & 0x1) lbl_80266B18[0].x -= 5.0f;
    if (lbl_802C0010[0] & 0x2) lbl_80266B18[0].x += 5.0f;
    lbl_80266A58[0].x += 5.0f * ((f32)lbl_802BFFF0[0] / 44.0f);
    lbl_80266A58[0].y -= 5.0f * ((f32)lbl_802BFFF4[0] / 44.0f);

    if (lbl_802C0008[0] & 0x800) fn_8003E488(1, &lbl_1_data_1714);

    printWin(0x10, 0x28, 0xF0, 0x54, &windowColor);
    fontcolor = FONT_COLOR_YELLOW;
    print8(0x10, 0x28, 1.0f, lbl_1_data_18B3, lbl_80266B18[0].x, lbl_80266B18[0].y,
           lbl_80266B18[0].z);
    print8(0x10, 0x34, 1.0f, lbl_1_data_18D2, lbl_80266A58[0].x, lbl_80266A58[0].y,
           lbl_80266A58[0].z);
    print8(0x10, 0x40, 1.0f, lbl_1_data_18EF, lbl_80266BD8[0]);
}

/* Returns 1 when two spheres overlap. */
s32 fn_1_259E8(Vec *position0, f32 radius0, Vec *position1, f32 radius1) {
    f32 distanceSquared;
    f32 radiusSquared;

    distanceSquared = ((position1->z - position0->z) * (position1->z - position0->z))
        + (((position1->x - position0->x) * (position1->x - position0->x))
           + ((position1->y - position0->y) * (position1->y - position0->y)));
    radiusSquared = (radius0 + radius1) * (radius0 + radius1);
    if (distanceSquared < radiusSquared) {
        return 1;
    }
    return 0;
}

void fn_1_25DF4(void);

/* Moves the answer model in with a sine swing, starts its effect and ends this process. */
void fn_1_25A90(void) {
    s32 index;
    s32 group;
    s32 modelIndex;
    s32 effectsCreated;
    f32 scale;
    f32 angle;
    f32 opacity;
    QuizParticleWork *work;

    effectsCreated = 0;
    for (modelIndex = 0; modelIndex < 2; modelIndex++) {
        fn_80026A44(lbl_1_bss_18B8[modelIndex], 1);
    }
    angle = 0.0;
    scale = 0.0f;
    for (group = 0; group < 2; group++) {
        for (index = 0; index < lbl_1_bss_18AC[group]; index++) {
            work = lbl_1_bss_1844[group][index]->data;
            work->state = 5;
        }
    }

    for (;;) {
        HuPrcVSleep();
        scale = (f32)sin((M_PI * angle) / 180.0);
        angle += 1.5f;
        if (angle >= 90.0f) {
            break;
        }
        for (modelIndex = 0; modelIndex < 2; modelIndex++) {
            fn_800268B0(lbl_1_bss_18B8[modelIndex], scale, scale,
                              scale);
        }
    }

    angle = 0.0;
    scale = 1.0f;
    opacity = 1.0f;
    for (;;) {
        HuPrcVSleep();
        scale = (f32)(1.0 + sin((M_PI * angle) / 180.0));
        opacity = 1.0 - sin((M_PI * angle) / 180.0);
        angle += 9.0f;
        if (angle >= 90.0f) {
            break;
        }
        for (modelIndex = 0; modelIndex < 2; modelIndex++) {
            fn_800268B0(lbl_1_bss_18B8[modelIndex], scale, scale,
                              scale);
            fn_80026CBC(lbl_1_bss_18B8[modelIndex], opacity);
        }
        if (angle >= 45.0f && effectsCreated == 0) {
            effectsCreated = 1;
            fn_1_262D4(0, lbl_1_bss_1820[0]);
            fn_1_262D4(1, lbl_1_bss_1820[1]);
            fn_1_26438(-350.0f, 430.0f, -450.0f, 0, 0.0f);
            fn_1_26438(350.0f, 430.0f, -450.0f, 1, 0.0f);
            fn_8000D818(fn_1_25DF4, 0x64, 0x1800, 0);
        }
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

static inline void SetProcessProperty(HUPROCESS *process, u8 selection, u16 slot) {
    process->property = (void *)((selection << 16) | slot);
}

/* Shows a particle effect with a sound for a while, then ends this process. */
void fn_1_25DF4(void) {
    HUPROCESS *firstProcess;
    HUPROCESS *secondProcess;
    ANIMDATA *animation;
    s32 modelIndex;
    u8 firstSelection;
    u8 secondSelection;
    s16 particleManager[2];
    f32 angle;
    f32 firstScale;
    f32 secondScale;

    animation = HuSprAnimRead(fn_80008DD0(DATANUM(0x210000, 1), 0x10000000, 2));
    fn_800097F4(0x210000);
    particleManager[0] = fn_1_26168(animation, lbl_1_bss_1820[0], 0);
    particleManager[1] = fn_1_26168(animation, lbl_1_bss_1820[1], 1);
    Hu3DParManAttrSet(particleManager[0], 1);
    Hu3DParManAttrSet(particleManager[1], 1);

    for (modelIndex = 0; modelIndex < 2; modelIndex++) {
        fn_80026A44(lbl_1_bss_18B4[modelIndex], 1);
    }

    angle = 0.0f;
    firstScale = 0.0f;
    secondScale = 0.0f;
    if (lbl_1_bss_1820[0] == 0) {
        HuAudFXPlay(MSM_SE_BRD00_STAR_SPACE_ARRIVE);
    } else {
        HuAudFXPlay(MSM_SE_BRD00_STAR_GROW);
    }
    if (lbl_1_bss_1820[1] == 0) {
        HuAudFXPlay(MSM_SE_BRD00_STAR_SPACE_ARRIVE);
    } else {
        HuAudFXPlay(MSM_SE_BRD00_STAR_GROW);
    }

    while (1) {
        HuPrcVSleep();
        if (lbl_1_bss_18AC[0] > lbl_1_bss_18AC[1]) {
            firstScale = 2.0 * sin((M_PI * angle) / 180.0);
            secondScale = 1.5 * sin((M_PI * angle) / 180.0);
        } else {
            firstScale = 1.5 * sin((M_PI * angle) / 180.0);
            secondScale = 2.0 * sin((M_PI * angle) / 180.0);
        }
        angle += 3.0f;
        if (angle >= 90.0f) {
            break;
        }
        fn_800268B0(lbl_1_bss_18B4[0], firstScale, firstScale, firstScale);
        fn_800268B0(lbl_1_bss_18B4[1], secondScale, secondScale, secondScale);
    }

    Hu3DParManAttrReset(particleManager[0], 1);
    Hu3DParManAttrReset(particleManager[1], 1);
    firstSelection = (u8)lbl_1_bss_18AC[0];
    firstProcess = 0;
    firstProcess = fn_8000D818(fn_1_24FA4, 0x64, 0x1800, 0);
    SetProcessProperty(firstProcess, firstSelection, 0);
    secondSelection = (u8)lbl_1_bss_18AC[1];
    secondProcess = 0;
    secondProcess = fn_8000D818(fn_1_24FA4, 0x64, 0x1800, 0);
    SetProcessProperty(secondProcess, secondSelection, 1);
    HuAudFXPlay(0x52D);
    HuPrcSleep(0x78);
    lbl_1_bss_1840 = 1;
    HuPrcEnd();
    while (1) {
        HuPrcVSleep();
    }
}

extern HU3D_PARMAN_PARAM lbl_1_data_18FC[];

extern HU3D_PARMAN_PARAM lbl_1_data_194C[];

HU3D_PARMAN_PARAM lbl_1_data_18FC[1] = { { 30, 0, 3.3f, 120.0f, 7.0f, { 0.0f, -0.05f, 0.0f }, 2.0f,
        1.0f, 30.0f, 0.98f, 2, { { 255, 255, 255, 255 }, { 255, 255, 64, 255 }, { 0, 0, 0, 0 },
                { 0, 0, 0, 0 } }, { { 255, 128, 128, 0 }, { 255, 64, 32, 0 }, { 0, 0, 0, 0 },
                                   { 0, 0, 0, 0 } } } };

/* Creates a particle manager from an animation and places it at one of two positions. */
s16 fn_1_26168(ANIMDATA *animation, s32 blendMode, s32 alternatePosition) {
    Point3d position;
    s16 modelId;
    s16 particleManager;

    if (alternatePosition == 0) {
        position.x = -350.0f;
    } else {
        position.x = 350.0f;
    }
    position.y = 430.0f;
    position.z = -400.0f;
    if (blendMode == 0) {
        particleManager = fn_800334F4(animation, 0x64, lbl_1_data_18FC);
    } else {
        particleManager = fn_800334F4(animation, 0x64, lbl_1_data_194C);
    }
    Hu3DParManAttrSet(particleManager, 0x64);
    Hu3DParManRotSet(particleManager, 90.0f, 0.0f, 0.0f);
    modelId = Hu3DParManModelIDGet(particleManager);
    if (blendMode == 0) {
        fn_8003269C(modelId, 1);
    } else {
        fn_8003269C(modelId, 2);
    }
    fn_80026B8C(modelId, 2);
    fn_80026B64(modelId, 1);
    Hu3DParManPosSet(particleManager, position.x, position.y, position.z);
    return particleManager;
}
extern s16 lbl_1_bss_182A[2];

/* Creates the given effect model and starts it. */
s32 fn_1_262D4(s32 effectIndex, s32 blendMode) {
    HU3D_PARTICLE *particleWork;
    HU3D_MODELID modelId;
    s32 itemIndex;

    modelId = (lbl_1_bss_182A[effectIndex] =
                   fn_8003216C(lbl_1_bss_1830[effectIndex], 0xC8));
    particleWork = (HU3D_PARTICLE *)lbl_802C01C8[modelId].hookData;
    if (blendMode == 0) {
        fn_8003269C(modelId, 1);
    } else {
        fn_8003269C(modelId, 2);
    }
    fn_80026B8C(modelId, 2);
    fn_80026760(modelId, 0.0f, 0.0f, 0.0f);
    fn_80032704(modelId, 0x64);
    fn_800326D0(modelId, fn_1_26558);
    for (itemIndex = 0; itemIndex < particleWork->maxCnt; itemIndex++) {
        memset(particleWork->data + itemIndex, 0, sizeof(HU3D_PARTICLE_DATA));
        (particleWork->data + itemIndex)->time = -1;
        (particleWork->data + itemIndex)->cameraBit = 1;
    }
}

/* Places a model and turns it to the given angle. */
void fn_1_26438(f32 positionX, f32 positionY, f32 positionZ, s32 modelIndex, f32 rotationY) {
    HU3D_PARTICLE_DATA *entry;
    HU3D_PARTICLE *hook;
    s32 index;
    s16 modelId;
    HuVecF vector;

    modelId = lbl_1_bss_182A[modelIndex];
    fn_80026808(modelId, 0.0f, rotationY, 0.0f);
    fn_80026760(modelId, positionX, positionY, positionZ);
    hook = lbl_802C01C8[modelId].hookData;
    entry = hook->data;
    vector.x = 0.0f;
    vector.y = 0.0f;
    vector.z = 0.0f;
    index = 0;
    while (index < hook->maxCnt) {
        if (entry->time < 0) {
            entry->time = 0;
        }
        index++;
        entry++;
    }
}

/* Particle hook: starts new particles with a random direction, size and life, and moves the
   living ones. */
void fn_1_26558(HU3D_MODEL *model, HU3D_PARTICLE *particleData, Mtx matrix) {
    HU3D_PARTICLE_DATA *particle;
    s32 index;
    f32 angle;
    f32 magnitude;
    s32 randomValue;
    s32 alpha;

    particle = particleData->data;
    index = 0;
    while (index < particleData->maxCnt) {
        if (particle->time == 0) {
            angle = 2.0f * frandf();
            magnitude = frandf();
            randomValue = rand8() % 14;
            particle->time += 1;
            particle->parManId = rand8() % 32;
            particle->vel.x = (f32)(8.0 * ((f64)magnitude *
                sin(M_PI * (f64)angle)));
            particle->vel.y = (f32)(8.0 * ((f64)magnitude *
                cos(M_PI * (f64)angle)));
            particle->vel.z = 0.0f;
            particle->pos.x = (f32)((f64)particle->pos.x +
                20.0 * ((f64)magnitude * sin(M_PI * (f64)angle)));
            particle->pos.y = (f32)((f64)particle->pos.y +
                20.0 * ((f64)magnitude * cos(M_PI * (f64)angle)));
            particle->color.r = 0xFF;
            particle->color.g = 0xFF;
            particle->color.b = 0xFF;
            particle->color.a = 0xFF;
            particle->scale = rand8() % 40;
        }
        if (particle->time == 1) {
            alpha = particle->color.a - 6;
            particle->pos.y += particle->vel.y;
            particle->parManId += 1;
            particle->pos.x += particle->vel.x;
            if (alpha <= 0) {
                alpha = 0;
                particle->scale = 0.0f;
                particle->time = -1;
            }
            particle->color.a = alpha;
        }
        index += 1;
        particle += 1;
    }
}

extern u8 lbl_1_bss_1828[0x2];

s16 lbl_1_bss_18CA;
s16 lbl_1_bss_18C6[2];
s16 lbl_1_bss_18C2[2];
s16 lbl_1_bss_18C0;
s16 lbl_1_bss_18BC[2];
s16 lbl_1_bss_18B8[2];
s16 lbl_1_bss_18B4[2];
s32 lbl_1_bss_18AC[2];
ANIMDATA *lbl_1_bss_18A8;
s16 lbl_1_bss_18A6;
s16 lbl_1_bss_18A4;
OMOBJ *lbl_1_bss_1844[2][12];
s32 lbl_1_bss_1840;
u8 lbl_1_bss_1838[0x8];
ANIMDATA *lbl_1_bss_1830[2];
u8 lbl_1_bss_182E[0x2];
s16 lbl_1_bss_182A[2];
u8 lbl_1_bss_1828[0x2];
s32 lbl_1_bss_1820[2];
HU3D_PARMAN_PARAM lbl_1_data_194C[1] = { { 30, 0, 3.3f, 120.0f, 7.0f, { 0.0f, -0.05f, 0.0f }, 2.0f,
        1.0f, 30.0f, 0.98f, 2, { { 0, 0, 0, 255 }, { 0, 0, 0, 255 }, { 0, 0, 0, 0 },
                { 0, 0, 0, 0 } }, { { 64, 0, 64, 64 }, { 64, 0, 64, 64 }, { 0, 0, 0, 0 },
                                   { 0, 0, 0, 0 } } } };

u8 lbl_1_data_199C[0x4] = { 0x00, 0x00, 0x00, 0x00 };
