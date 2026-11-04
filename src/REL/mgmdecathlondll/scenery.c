/* Decathlon Park scenery models and their animation sequences. */
#include "REL/mgmdecathlondll/module_types.h"
#include "datadir_enum.h"
#include <math.h>
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
#include "msm_se.h"
#include "string.h"

#define MSM_SE_DECA_EVENT_REVEAL 1239
#define MSM_SE_DECA_EVENT_READY 1240
#define DECATHLON_NO_AUDIO_HANDLE 4294967295U

/* Each leader marker's frame callback pauses its motion layers when the next main-motion
 * step would reach or pass the end. */
void fn_1_68C0(OMOBJ *obj) {
    f32 maxTime;

    maxTime = fn_8002D530(*obj->mdlId);
    if ((lbl_802C01C8[*obj->mdlId].motWork.speed + fn_8002D4E0(*obj->mdlId)) >= maxTime) {
        fn_800269E8(*obj->mdlId,
                         ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                         ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        obj->work[0] = 100;
    }
}

Vec lbl_1_data_358 = {0.0f, 0.0f, 185.0f};

/* Mode setup creates the foreground actor, its right-hand prop, and six motion choices. */
void fn_1_6960(void) {
    lbl_1_bss_A0 = fn_8003AB38(lbl_1_bss_C0, 4608, 2U, 6U, -1, fn_1_6E80);
    lbl_1_bss_A0->data = NULL;
    lbl_1_bss_A0->work[0] = 0;
    lbl_1_bss_A0->work[1] = 0;
    lbl_1_bss_A0->work[2] = 0;
    lbl_1_bss_A0->work[3] = 0;
    lbl_1_bss_A0->stat = lbl_1_bss_A0->stat | (1 << 8);
    *lbl_1_bss_A0->mdlId = fn_80025A04(fn_80008DD0(
        DATANUM(0x9e0000, 45), 0x10000000, 2));
    fn_80026B64(*lbl_1_bss_A0->mdlId, 1U);
    fn_80026B8C(*lbl_1_bss_A0->mdlId, 5);
    fn_80026790(*lbl_1_bss_A0->mdlId, &lbl_1_data_358);
    fn_800269E8(*lbl_1_bss_A0->mdlId, ((1 << 30) | (1 << 0)));
    fn_800268B0(*lbl_1_bss_A0->mdlId, 1.0f, 1.0f, 1.0f);
    fn_80026E44(*lbl_1_bss_A0->mdlId);
    Hu3DLLightCreate(*lbl_1_bss_A0->mdlId, 0.0f, 1500.0f, 1500.0f, 0.0f, -0.1f, -1.0f, 204U, 204U,
                     204U);
    lbl_1_bss_A0->mdlId[1] =
        fn_80025A04(fn_80008DD0(
            DATANUM(0x9e0000, 52), 0x10000000, 2));
    fn_8002741C(*lbl_1_bss_A0->mdlId, "f-itemhook_R", lbl_1_bss_A0->mdlId[1]);
    *lbl_1_bss_A0->mtnId = fn_80030458(*lbl_1_bss_A0->mdlId,
                                           fn_80008DD0(
                                               DATANUM(0x9e0000, 46),
                                               0x10000000, 2));
    fn_8002C95C(*lbl_1_bss_A0->mdlId, *lbl_1_bss_A0->mtnId);
    lbl_1_bss_A0->mtnId[1] = fn_80030458(
        *lbl_1_bss_A0->mdlId, fn_80008DD0(
            DATANUM(0x9e0000, 47), 0x10000000, 2));
    lbl_1_bss_A0->mtnId[2] = fn_80030458(
        *lbl_1_bss_A0->mdlId, fn_80008DD0(
            DATANUM(0x9e0000, 48), 0x10000000, 2));
    lbl_1_bss_A0->mtnId[3] = fn_80030458(
        *lbl_1_bss_A0->mdlId, fn_80008DD0(
            DATANUM(0x9e0000, 49), 0x10000000, 2));
    lbl_1_bss_A0->mtnId[4] = fn_80030458(
        *lbl_1_bss_A0->mdlId, fn_80008DD0(
            DATANUM(0x9e0000, 50), 0x10000000, 2));
    lbl_1_bss_A0->mtnId[5] = fn_80030458(
        *lbl_1_bss_A0->mdlId, fn_80008DD0(
            DATANUM(0x9e0000, 51), 0x10000000, 2));
}

/* The final-results sequence requests the actor's repeating reaction or a return to idle. */
void fn_1_6DC4(u16 enabled) {
    if ((lbl_1_bss_A0->work[1] == 0U) && (enabled != 0U)) {
        lbl_1_bss_A0->work[1] = 1U;
    }
    if ((lbl_1_bss_A0->work[1] != 0U) && (enabled == 0U)) {
        lbl_1_bss_A0->work[1] = 2U;
    }
}

/* Dialogue starts the foreground actor's short response animation. */
void fn_1_6E38(void) {
    lbl_1_bss_A0->work[0] = 1;
}

/* The next-event introduction starts a 25-frame rising entrance for the foreground actor. */
void decathlonActorEntranceStart(void) {
    lbl_1_bss_A0->work[2] = 25;
}

/* Before launching an event, the sequence requests the actor's turn-away motion. */
void decathlonActorTurnAway(void) {
    lbl_1_bss_A0->work[3] = 1;
}

/* The foreground actor's frame callback advances entrance, turn-away, reaction, or dialogue
 * motion. */
void fn_1_6E80(OMOBJ *object) {
    Vec position;
    f32 entranceRemainingFraction;
    f32 horizontalStepFraction;
    if (object->work[2] != 0) {
        /* The shrinking vertical step and growing horizontal step curve the entrance to the
         * right. */
        fn_800267CC(object->mdlId[0], &position);
        entranceRemainingFraction = object->work[2] / 25.0f;
        horizontalStepFraction = 1.0f - entranceRemainingFraction;
        position.x += 17.0f * horizontalStepFraction;
        position.y += 23.0f * entranceRemainingFraction;
        fn_80026790(object->mdlId[0], &position);
        object->work[2]--;
        if (object->work[2] == 0) {
            fn_800269E8(object->mdlId[0], ((1 << 30) | (1 << 1)));
            fn_8002CCC0(object->mdlId[0], object->mtnId[0], 0.0f,
                               4.0f, ((1 << 30) | (1 << 0)));
            fn_8002D86C(object->mdlId[0], 1.0f);
            object->work[0] = 0;
            object->work[1] = 0;
            object->work[2] = 0;
            return;
        }
        if (fn_8002D3EC(object->mdlId[0]) < 0 &&
            object->mtnId[3] != fn_8002D3C4(object->mdlId[0])) {
            fn_800269E8(object->mdlId[0], ((1 << 30) | (1 << 1)));
            fn_8002CCC0(object->mdlId[0], object->mtnId[3], 0.0f,
                               4.0f, ((1 << 30) | (1 << 0)));
            fn_8002D86C(object->mdlId[0], 1.0f);
            return;
        }
        if (fn_8002D3EC(object->mdlId[0]) < 0 &&
            object->mtnId[3] == fn_8002D3C4(object->mdlId[0])) {
            if (lbl_802C01C8[object->mdlId[0]].motWork.speed + fn_8002D4E0(object->mdlId[0]) >=
                fn_8002D530(object->mdlId[0])) {
                fn_800269E8(object->mdlId[0], ((1 << 30) | (1 << 1)));
            }
        }
    } else if (object->work[3] != 0) {
        /* Once requested, the turn-away state keeps the actor facing forty degrees left. */
        fn_80026874(object->mdlId[0], &position);
        position.y -= 1.0f;
        if (position.y <= -40.0f) {
            position.y = -40.0f;
        }
        fn_80026838(object->mdlId[0], &position);
        if (fn_8002D3EC(object->mdlId[0]) < 0 &&
            object->mtnId[5] != fn_8002D3C4(object->mdlId[0])) {
            fn_8002CCC0(object->mdlId[0], object->mtnId[5], 0.0f,
                               6.0f, ((1 << 30) | (1 << 0)));
            fn_8002D86C(object->mdlId[0], 1.0f);
        }
    } else if (object->work[1] != 0) {
        if (object->work[1] == 1) {
            if (fn_8002D3EC(object->mdlId[0]) < 0 &&
                object->mtnId[2] != fn_8002D3C4(object->mdlId[0])) {
                fn_8002CCC0(object->mdlId[0], object->mtnId[2], 0.0f,
                                   8.0f, ((1 << 30) | (1 << 0)));
                fn_8002D86C(object->mdlId[0], 1.0f);
                object->work[0] = 0;
            }
            if (fn_8002D3EC(object->mdlId[0]) < 0 &&
                object->mtnId[2] == fn_8002D3C4(object->mdlId[0]) &&
                (fn_8002D4E0(object->mdlId[0]) == 6.0f ||
                 fn_8002D4E0(object->mdlId[0]) == 22.0f ||
                 fn_8002D4E0(object->mdlId[0]) == 38.0f ||
                 fn_8002D4E0(object->mdlId[0]) == 54.0f ||
                 fn_8002D4E0(object->mdlId[0]) == 70.0f)) {
                HuAudFXPlay(940);
            }
        } else if (fn_8002D3EC(object->mdlId[0]) < 0) {
            fn_800269E8(object->mdlId[0], ((1 << 30) | (1 << 1)));
            fn_8002CCC0(object->mdlId[0], object->mtnId[0], 0.0f,
                               8.0f, ((1 << 30) | (1 << 0)));
            fn_8002D86C(object->mdlId[0], 1.0f);
            object->work[1] = 0;
            object->work[0] = 0;
        }
    } else if (object->work[0] != 0) {
        if (fn_8002D3EC(object->mdlId[0]) < 0 &&
            object->mtnId[1] != fn_8002D3C4(object->mdlId[0])) {
            fn_8002CCC0(object->mdlId[0], object->mtnId[1], 0.0f,
                               8.0f, ((1 << 30) | (1 << 0)));
            fn_8002D86C(object->mdlId[0], 2.0f);
            return;
        }
        if (fn_8002D3EC(object->mdlId[0]) < 0 &&
            object->mtnId[1] == fn_8002D3C4(object->mdlId[0])) {
            if (lbl_802C01C8[object->mdlId[0]].motWork.speed + fn_8002D4E0(object->mdlId[0]) >=
                fn_8002D530(object->mdlId[0])) {
                fn_8002CCC0(object->mdlId[0], object->mtnId[0], 0.0f,
                                   8.0f, ((1 << 30) | (1 << 0)));
                fn_8002D86C(object->mdlId[0], 1.0f);
                object->work[0] = 0;
            }
        }
    }
}

void fn_1_7A94(OMOBJ *obj);
s32 lbl_1_data_374[10] = {
    DATANUM(0x9e0000, 35), DATANUM(0x9e0000, 36),
    DATANUM(0x9e0000, 37), DATANUM(0x9e0000, 38),
    DATANUM(0x9e0000, 39), DATANUM(0x9e0000, 40),
    DATANUM(0x9e0000, 41), DATANUM(0x9e0000, 42),
    DATANUM(0x9e0000, 43), DATANUM(0x9e0000, 44)
};
Point3d lbl_1_data_39C = {-60.0f, -110.0f, 310.0f};

typedef struct MgmDecathlonAnimationData_s {
    ANIMDATA *animations[10]; /* one symbol animation per Decathlon event */
    s16 symbolAnimationId; /* active symbol-animation handle; -1 before the first entrance */
} MgmDecathlonAnimationData;

extern u8 MgModePlayNum;
extern s16 fn_1_9784(s16 playerNo);

/* Mode setup prepares the hidden announcement actor and the symbols for all ten events. */
void fn_1_75C4(void) {
    s32 animationIndex;
    MgmDecathlonAnimationData *animationData;

    lbl_1_bss_9C = fn_8003AB38(lbl_1_bss_C0, 5120, 2U, 1U, -1, fn_1_7A94);
    lbl_1_bss_9C->stat = lbl_1_bss_9C->stat | (1 << 8);
    lbl_1_bss_9C->work[0] = 0;
    lbl_1_bss_9C->data = HuMemDirectMallocNum(HEAP_HEAP, 44, 0x10000000);
    memset(lbl_1_bss_9C->data, 0, 44);
    animationData = lbl_1_bss_9C->data;
    animationData->symbolAnimationId = -1;
    animationIndex = 0;
    while (animationIndex < 10) {
        animationData->animations[animationIndex] =
            HuSprAnimRead(fn_80008DD0(
                lbl_1_data_374[animationIndex],
                0x10000000, 2));
        animationIndex += 1;
    }
    *lbl_1_bss_9C->mdlId = fn_80025A04(fn_80008DD0(
        DATANUM(0x9e0000, 53), 0x10000000, 2));
    fn_80026B64(*lbl_1_bss_9C->mdlId, 1U);
    fn_80026B8C(*lbl_1_bss_9C->mdlId, 1);
    fn_80026790(*lbl_1_bss_9C->mdlId, &lbl_1_data_39C);
    lbl_1_bss_9C->mdlId[1] =
        fn_80025A04(fn_80008DD0(
            DATANUM(0x9e0000, 3), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_9C->mdlId[1], 1U);
    fn_80026B8C(lbl_1_bss_9C->mdlId[1], 1);
    fn_8002741C(*lbl_1_bss_9C->mdlId, "f-itemhook_L", lbl_1_bss_9C->mdlId[1]);
    *lbl_1_bss_9C->mtnId = fn_80030458(*lbl_1_bss_9C->mdlId,
                                           fn_80008DD0(
                                               DATANUM(0x9e0000, 54),
                                               0x10000000, 2));
    fn_8002C95C(*lbl_1_bss_9C->mdlId, *lbl_1_bss_9C->mtnId);
    fn_800269E8(*lbl_1_bss_9C->mdlId, ((1 << 30) | (1 << 0)));
    fn_800269E8(*lbl_1_bss_9C->mdlId,
                     ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                     ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
    fn_800269E8(*lbl_1_bss_9C->mdlId, (1 << 0));
}

/* The next-event introduction selects its symbol, reveals the actor, and cues human players. */
void fn_1_78DC(void) {
    s16 playerNo;
    MgmDecathlonAnimationData *symbolData;

    if ((MgModePlayNum < 10) && (lbl_1_bss_9C->work[0] == 0)) {
        symbolData = lbl_1_bss_9C->data;
        if (symbolData->symbolAnimationId >= 0) {
            fn_80031010(symbolData->symbolAnimationId);
        }
        symbolData->symbolAnimationId = fn_80030B2C(symbolData->animations[MgModePlayNum],
                                                       lbl_1_bss_9C->mdlId[1], "symbol01");
        fn_80026790(lbl_1_bss_9C->mdlId[0], &lbl_1_data_39C);
        fn_80026A44(lbl_1_bss_9C->mdlId[0],
                           ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                           ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        fn_80026A44(lbl_1_bss_9C->mdlId[0], (1 << 0));
        fn_8002D414(lbl_1_bss_9C->mdlId[0], 0.0f);
        lbl_1_bss_9C->work[0] = 1;
        HuAudFXPlay(MSM_SE_DECA_EVENT_REVEAL);
        for (playerNo = 0; playerNo < 4; playerNo++) {
            if (fn_1_9784(playerNo) == 0) {
                fn_8006A030(playerNo, 20, 4, 4);
            }
        }
    }
}

/* The main sequence polls this until the event-announcement actor finishes its entrance. */
s32 fn_1_7A6C(void) {
    if (lbl_1_bss_9C->work[0] == 2) {
        return 1;
    }
    return 0;
}

/* The announcement actor's frame callback raises it into view, then pauses its motion layers
 * when the next main-motion step would reach or pass the end. */
void fn_1_7A94(OMOBJ *object) {
    HuVecF position;
    f32 motionMaxTime;

    switch (object->work[0]) {
    case 0:
        fn_800269E8(object->mdlId[0], (1 << 0));
        break;
    case 1:
        if (fn_8002D4E0(object->mdlId[0]) < 15.0f) {
            fn_800267CC(object->mdlId[0], &position);
            position.y += 6.6666665f;
            fn_80026790(object->mdlId[0], &position);
        }
        motionMaxTime = fn_8002D530(object->mdlId[0]);
        if ((lbl_802C01C8[object->mdlId[0]].motWork.speed +
             fn_8002D4E0(object->mdlId[0])) >= motionMaxTime) {
            object->work[0] = 2;
            fn_800269E8(object->mdlId[0],
                             ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                             ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
            HuAudFXPlay(MSM_SE_DECA_EVENT_READY);
        }
        break;
    case 2:
        fn_80026A44(object->mdlId[0], (1 << 0));
        fn_800269E8(object->mdlId[0],
                         ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                         ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        break;
    }
}

s32 lbl_1_data_3C0[8] = {
    DATANUM(0x9e0000, 55), DATANUM(0x9e0000, 58),
    DATANUM(0x9e0000, 61), DATANUM(0x9e0000, 64),
    DATANUM(0x9e0000, 74), DATANUM(0x9e0000, 67),
    DATANUM(0x9e0000, 80), DATANUM(0x9e0000, 77)
};
s32 lbl_1_data_3E0[16] = {
    DATANUM(0x9e0000, 56), DATANUM(0x9e0000, 57),
    DATANUM(0x9e0000, 59), DATANUM(0x9e0000, 60),
    DATANUM(0x9e0000, 62), DATANUM(0x9e0000, 63),
    DATANUM(0x9e0000, 65), DATANUM(0x9e0000, 66),
    DATANUM(0x9e0000, 75), DATANUM(0x9e0000, 76),
    DATANUM(0x9e0000, 68), DATANUM(0x9e0000, 69),
    DATANUM(0x9e0000, 81), DATANUM(0x9e0000, 82),
    DATANUM(0x9e0000, 78), DATANUM(0x9e0000, 79)
};
Vec lbl_1_data_420[8] = { { 500.0f, 0.0f, -600.0f },   { -500.0f, 0.0f, -600.0f },
                          { 600.0f, 0.0f, -600.0f },   { -630.0f, 0.0f, -600.0f },
                          { 550.0f, 0.0f, -900.0f },   { -520.0f, 0.0f, -650.0f },
                          { 500.0f, 300.0f, -600.0f }, { -520.0f, 400.0f, -800.0f } };
extern void fn_1_8818(OMOBJ *object);
extern void fn_1_8708(void);

/* Mode setup creates the spectators and shows the arrangement for the current event count. */
void fn_1_7BF8(void) {
    OMOBJ *object;
    s16 modelIndex;
    s16 motionIndex;
    Vec position;

    object = fn_8003AB38(lbl_1_bss_C0, 4288, 8, 16, -1,
                        fn_1_8818);
    lbl_1_bss_90 = object;
    object->stat |= (1 << 8);
    object->data = NULL;
    for (modelIndex = 0; modelIndex < 4; modelIndex++) {
        object->work[modelIndex] = 0;
    }
    for (modelIndex = 0; modelIndex < 8; modelIndex++) {
        object->mdlId[modelIndex] =
            fn_80025A04(fn_80008DD0(lbl_1_data_3C0[modelIndex],
                                                 0x10000000, 2));
        fn_80026B64(object->mdlId[modelIndex], 1);
        fn_80026B8C(object->mdlId[modelIndex], 1);
        fn_80026790(object->mdlId[modelIndex], &lbl_1_data_420[modelIndex]);
        for (motionIndex = 0; motionIndex < 2; motionIndex++) {
            object->mtnId[(modelIndex * 2) + motionIndex] =
                fn_80030458(object->mdlId[modelIndex],
                    fn_80008DD0(lbl_1_data_3E0[(modelIndex * 2) + motionIndex],
                                         0x10000000, 2));
        }
        fn_8002C95C(object->mdlId[modelIndex], object->mtnId[modelIndex * 2]);
        fn_800269E8(object->mdlId[modelIndex], ((1 << 30) | (1 << 0)));
        fn_800269E8(object->mdlId[modelIndex], (1 << 0));
    }

    lbl_1_bss_98 = fn_80025A04(
        fn_80008DD0(DATANUM(0x9e0000, 71),
                             0x10000000, 2));
    fn_80026B64(lbl_1_bss_98, 1);
    fn_80026B8C(lbl_1_bss_98, 1);
    lbl_1_bss_96 = fn_80030458(
        lbl_1_bss_98, fn_80008DD0(DATANUM(0x9e0000, 73),
                                          0x10000000, 2));
    fn_8002C95C(lbl_1_bss_98, lbl_1_bss_96);
    fn_800269E8(lbl_1_bss_98, ((1 << 30) | (1 << 0)));
    fn_800269E8(lbl_1_bss_98, (1 << 0));
    fn_800268B0(object->mdlId[4], 1.2f,
                      1.2f, 1.2f);
    fn_800268B0(lbl_1_bss_98, 1.2f,
                      1.2f, 1.2f);
    lbl_1_bss_94 = fn_80030458(
        object->mdlId[5], fn_80008DD0(DATANUM(0x9e0000, 70),
                                               0x10000000, 2));

    switch (MgModePlayNum) {
    case 0:
        fn_80026A44(object->mdlId[0], (1 << 0));
        fn_80026790(object->mdlId[0], &lbl_1_data_420[1]);
        break;
    case 1:
        fn_80026A44(object->mdlId[0], (1 << 0));
        fn_80026790(object->mdlId[0], &lbl_1_data_420[1]);
        fn_80026A44(object->mdlId[1], (1 << 0));
        fn_80026790(object->mdlId[1], &lbl_1_data_420[0]);
        break;
    case 2:
        fn_80026A44(object->mdlId[0], (1 << 0));
        fn_80026790(object->mdlId[0], &lbl_1_data_420[1]);
        fn_80026A44(object->mdlId[1], (1 << 0));
        fn_80026790(object->mdlId[1], &lbl_1_data_420[0]);
        fn_80026A44(object->mdlId[7], (1 << 0));
        fn_80026790(object->mdlId[7], &lbl_1_data_420[6]);
        break;
    case 3:
        fn_80026A44(object->mdlId[0], (1 << 0));
        fn_80026790(object->mdlId[0], &lbl_1_data_420[1]);
        fn_80026A44(object->mdlId[1], (1 << 0));
        fn_80026790(object->mdlId[1], &lbl_1_data_420[0]);
        fn_80026A44(object->mdlId[7], (1 << 0));
        fn_80026790(object->mdlId[7], &lbl_1_data_420[6]);
        fn_80026A44(object->mdlId[6], (1 << 0));
        fn_80026790(object->mdlId[6], &lbl_1_data_420[7]);
        break;
    case 4:
        fn_80026A44(object->mdlId[0], (1 << 0));
        fn_80026790(object->mdlId[0], &lbl_1_data_420[1]);
        fn_80026A44(object->mdlId[1], (1 << 0));
        fn_80026790(object->mdlId[1], &lbl_1_data_420[0]);
        fn_80026A44(object->mdlId[7], (1 << 0));
        fn_80026790(object->mdlId[7], &lbl_1_data_420[6]);
        fn_80026A44(object->mdlId[6], (1 << 0));
        fn_80026790(object->mdlId[6], &lbl_1_data_420[7]);
        fn_80026A44(object->mdlId[5], (1 << 0));
        fn_80026790(object->mdlId[5], &lbl_1_data_420[4]);
        break;
    case 5:
        fn_80026A44(object->mdlId[2], (1 << 0));
        fn_80026790(object->mdlId[2], &lbl_1_data_420[1]);
        fn_80026A44(object->mdlId[1], (1 << 0));
        fn_80026790(object->mdlId[1], &lbl_1_data_420[0]);
        fn_80026A44(object->mdlId[7], (1 << 0));
        fn_80026790(object->mdlId[7], &lbl_1_data_420[6]);
        fn_80026A44(object->mdlId[6], (1 << 0));
        fn_80026790(object->mdlId[6], &lbl_1_data_420[7]);
        fn_80026A44(object->mdlId[5], (1 << 0));
        fn_80026790(object->mdlId[5], &lbl_1_data_420[4]);
        break;
    case 6:
        fn_80026A44(object->mdlId[2], (1 << 0));
        fn_80026790(object->mdlId[2], &lbl_1_data_420[1]);
        fn_80026A44(object->mdlId[3], (1 << 0));
        fn_80026790(object->mdlId[3], &lbl_1_data_420[0]);
        fn_80026A44(object->mdlId[7], (1 << 0));
        fn_80026790(object->mdlId[7], &lbl_1_data_420[6]);
        fn_80026A44(object->mdlId[6], (1 << 0));
        fn_80026790(object->mdlId[6], &lbl_1_data_420[7]);
        fn_80026A44(object->mdlId[5], (1 << 0));
        fn_80026790(object->mdlId[5], &lbl_1_data_420[4]);
        break;
    case 7:
        object->work[1] = 1;
        fn_80026A44(object->mdlId[2], (1 << 0));
        fn_80026790(object->mdlId[2], &lbl_1_data_420[1]);
        fn_80026A44(object->mdlId[3], (1 << 0));
        fn_80026790(object->mdlId[3], &lbl_1_data_420[0]);
        fn_80026A44(object->mdlId[7], (1 << 0));
        fn_80026790(object->mdlId[7], &lbl_1_data_420[6]);
        fn_80026A44(object->mdlId[6], (1 << 0));
        fn_80026790(object->mdlId[6], &lbl_1_data_420[7]);
        fn_80026A44(object->mdlId[5], (1 << 0));
        fn_80026790(object->mdlId[5], &lbl_1_data_420[4]);
        break;
    case 8:
        fn_80026A44(object->mdlId[2], (1 << 0));
        fn_80026790(object->mdlId[2], &lbl_1_data_420[1]);
        fn_80026A44(object->mdlId[3], (1 << 0));
        fn_80026790(object->mdlId[3], &lbl_1_data_420[0]);
        fn_80026A44(object->mdlId[7], (1 << 0));
        fn_80026790(object->mdlId[7], &lbl_1_data_420[6]);
        fn_80026A44(object->mdlId[6], (1 << 0));
        fn_80026790(object->mdlId[6], &lbl_1_data_420[7]);
        fn_80026A44(object->mdlId[5], (1 << 0));
        fn_80026790(object->mdlId[5], &lbl_1_data_420[4]);
        fn_80026A44(object->mdlId[4], (1 << 0));
        position = lbl_1_data_420[5];
        position.y += 100.0f;
        fn_80026790(object->mdlId[4], &position);
        break;
    case 9:
    default:
        fn_80026A44(object->mdlId[2], (1 << 0));
        fn_80026790(object->mdlId[2], &lbl_1_data_420[1]);
        fn_80026A44(object->mdlId[3], (1 << 0));
        fn_80026790(object->mdlId[3], &lbl_1_data_420[0]);
        fn_80026A44(object->mdlId[7], (1 << 0));
        fn_80026790(object->mdlId[7], &lbl_1_data_420[6]);
        fn_80026A44(object->mdlId[6], (1 << 0));
        fn_80026790(object->mdlId[6], &lbl_1_data_420[7]);
        fn_80026A44(object->mdlId[5], (1 << 0));
        fn_80026790(object->mdlId[5], &lbl_1_data_420[4]);
        fn_80026A44(object->mdlId[4], (1 << 0));
        position = lbl_1_data_420[5];
        position.y += 100.0f;
        fn_80026790(object->mdlId[4], &position);
        fn_80026A44(object->mdlId[0], (1 << 0));
        fn_80026790(object->mdlId[0], &lbl_1_data_420[2]);
        fn_80026A44(object->mdlId[1], (1 << 0));
        fn_80026790(object->mdlId[1], &lbl_1_data_420[3]);
        break;
    }
    fn_1_8708();
}

/* Scene transitions start the spectator cue if its model is visible and no cue handle is active. */
void fn_1_8708(void) {
    if (((fn_80026A90(lbl_1_bss_90->mdlId[6]) & (1 << 0)) == 0) &&
        (lbl_1_data_C90 < 0)) {
        lbl_1_data_C90 = HuAudFXPlayVolPan(963, 100, 32);
    }
}

/* Result sequences request spectator reactions, or ask the crowd to blend back to idle. */
void fn_1_8778(u16 enabled) {
    if ((lbl_1_bss_90->work[0] == 0) && (enabled != 0)) {
        lbl_1_bss_90->work[0] = 1U;
    }
    if ((lbl_1_bss_90->work[0] != 0) && (enabled == 0)) {
        lbl_1_bss_90->work[0] = 2U;
    }
}

/* Before the eighth event launches, the sequence asks one spectator to turn and walk away. */
void fn_1_87EC(void) {
    if (MgModePlayNum == 7) {
        lbl_1_bss_90->work[2] = 1;
    }
}

extern void fn_1_9E50(Vec *center);
extern s32 fn_1_FB2C(Vec *direction, Vec *unitDirection);
extern f64 fn_1_FD44(Vec first, Vec second);

/* Accepts the spectator audio status without changing the scene. */
static inline void DebugValue(u32 audioStatusBits) { }

/* The spectator frame callback faces the player lineup and advances reactions and walking
 * motions. */
void fn_1_8818(OMOBJ *object) {
    Vec center;
    Vec modelPosition;
    Vec forwardDirection;
    f32 angle;
    s16 index;
    s16 anyMotionActive;

    fn_1_9E50(&center);
    forwardDirection.x = forwardDirection.y = 0.0f;
    forwardDirection.z = 1.0f;

    for (index = 0; index < 8; index++) {
        if ((index != 5) || (object->work[2] == 0)) {
            fn_800267CC(object->mdlId[index], &modelPosition);
            PSVECSubtract(&center, &modelPosition, &modelPosition);
            modelPosition.y = 0.0f;
            if (fn_1_FB2C(&modelPosition, &modelPosition) == 0) {
                modelPosition.x = modelPosition.y = 0.0f;
                modelPosition.z = 1.0f;
            }
            angle = (f32)fn_1_FD44(forwardDirection, modelPosition);
            if (modelPosition.x >= 0.0f) {
                modelPosition.x = modelPosition.z = 0.0f;
                modelPosition.y = 0.6f * angle;
            } else {
                modelPosition.x = modelPosition.z = 0.0f;
                modelPosition.y = -0.6f * angle;
            }
            fn_80026838(object->mdlId[index], &modelPosition);
        }
    }

    if ((u32)object->work[0] == 1) {
        for (index = 0; index < 8; index++) {
            if (((index != 5) || (object->work[2] == 0)) &&
                (fn_8002D3EC(object->mdlId[index]) < 0) &&
                (object->mtnId[(index * 2) + 1] != fn_8002D3C4(object->mdlId[index]))) {
                fn_8002CCC0(object->mdlId[index], object->mtnId[(index * 2) + 1],
                                   0.0f, 8.0f, ((1 << 30) | (1 << 0)));
                if ((fn_80026A90(object->mdlId[index]) & (1 << 0)) == 0) {
                    if (index == 0) {
                        HuAudFXPlayPan(981, 32);
                    }
                    if (index == 1) {
                        HuAudFXPlayPan(960, 96);
                    }
                    if (index == 2) {
                        HuAudFXPlayPan(929, 32);
                    }
                    if (index == 3) {
                        HuAudFXPlayPan(986, 96);
                    }
                    if (index == 4) {
                        HuAudFXPlayPan(956, 32);
                    }
                    if (index == 7) {
                        HuAudFXPlayPan(965, 96);
                    }
                }
            }
        }
        if (MgModePlayNum > 9) {
            DebugValue((u32)lbl_1_data_C8C >> 31);
        }
    } else if ((u32)object->work[0] != 0) {
        index = 0;
        anyMotionActive = 0;
        while (index < 8) {
            if ((index != 5) || (object->work[2] == 0)) {
                if ((fn_8002D3EC(object->mdlId[index]) < 0) &&
                    (object->mtnId[index * 2] != fn_8002D3C4(object->mdlId[index]))) {
                    fn_8002CCC0(object->mdlId[index], object->mtnId[index * 2],
                                       0.0f, 8.0f, ((1 << 30) | (1 << 0)));
                }
                if (object->mtnId[index * 2] != fn_8002D3C4(object->mdlId[index])) {
                    anyMotionActive = 1;
                }
            }
            index++;
        }
        if (anyMotionActive == 0) {
            object->work[0] = 0;
        }
        if ((s32)lbl_1_data_C8C >= 0) {
            HuAudFXStop((s32)lbl_1_data_C8C);
        }
        lbl_1_data_C8C = DECATHLON_NO_AUDIO_HANDLE;
    }

    if (object->work[1] == 1) {
        /* This extra spectator paces between the left and right edges of the scene. */
        fn_80026A44(lbl_1_bss_98, (1 << 0));
        fn_80026808(lbl_1_bss_98, 0.0f, 90.0f, 0.0f);
        fn_80026760(lbl_1_bss_98, -700.0f, 0.0f, -350.0f);
        object->work[1] = 2;
    } else if (object->work[1] == 2) {
        fn_80026A44(lbl_1_bss_98, (1 << 0));
        fn_80026808(lbl_1_bss_98, 0.0f, 90.0f, 0.0f);
        fn_800267CC(lbl_1_bss_98, &modelPosition);
        modelPosition.x += 1.5f;
        if (modelPosition.x > 700.0f) {
            object->work[1] = 3;
        }
        fn_80026790(lbl_1_bss_98, &modelPosition);
    } else if (object->work[1] == 3) {
        fn_80026A44(lbl_1_bss_98, (1 << 0));
        fn_80026808(lbl_1_bss_98, 0.0f, 270.0f, 0.0f);
        fn_80026760(lbl_1_bss_98, 700.0f, 0.0f, -350.0f);
        object->work[1] = 4;
    } else if (object->work[1] == 4) {
        fn_80026A44(lbl_1_bss_98, (1 << 0));
        fn_80026808(lbl_1_bss_98, 0.0f, 270.0f, 0.0f);
        fn_800267CC(lbl_1_bss_98, &modelPosition);
        modelPosition.x -= 1.5f;
        if (modelPosition.x < -700.0f) {
            object->work[1] = 1;
        }
        fn_80026790(lbl_1_bss_98, &modelPosition);
    } else {
        fn_800269E8(lbl_1_bss_98, (1 << 0));
        object->work[1] = 0;
    }

    if (object->work[2] == 1) {
        fn_8002CCC0(object->mdlId[5], lbl_1_bss_94,
                           0.0f, 4.0f, ((1 << 30) | (1 << 0)));
        HuAudFXPlayPan(1001, 96);
        object->work[2] = 2;
        return;
    }
    if (object->work[2] != 0) {
        fn_80026874(object->mdlId[5], &modelPosition);
        modelPosition.y -= 15.0f;
        if (modelPosition.y <= -90.0f) {
            modelPosition.y = -90.0f;
        }
        fn_80026838(object->mdlId[5], &modelPosition);
        if (modelPosition.y <= -85.0f) {
            fn_800267CC(object->mdlId[5], &modelPosition);
            modelPosition.x -= 6.0f;
            fn_80026790(object->mdlId[5], &modelPosition);
        }
    }
}

OMOBJ *lbl_1_bss_A0;
OMOBJ *lbl_1_bss_9C;
s16 lbl_1_bss_98;
s16 lbl_1_bss_96;
s16 lbl_1_bss_94;
OMOBJ *lbl_1_bss_90;
