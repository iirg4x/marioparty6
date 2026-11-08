/* Round result: the per-frame steps that close a round and start each player's
 * result motion. */

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

#include "game/hu3d.h"
#include "REL/m661dll/player_state.h"
#include "REL/m661dll/system_state.h"
#include "REL/m661dll/score_display.h"
#include "REL/m661dll/particle.h"
#include "REL/m661dll/sound.h"
#include "REL/m661dll/motion.h"
#include "REL/m661dll.h"


typedef struct ModelSetupRecord {
    HU3D_MODELID modelId;
    s16 state;
    HuVecF position;
    HuVecF shadowPosition;
    HuVecF rotation;
} ModelSetupRecord;

typedef struct PlayerAnimationWork {
    s16 motionIds[5];
    s16 motionId, appliedMotionId;
    f32 motionSpeed, appliedMotionSpeed;
    f32 motionTime, appliedMotionTime, motionDuration;
    u32 modelAttributes, appliedModelAttributes, attributesToReset;
    s32 motionEnded;
    s16 repeatMotion, nextMotionId;
    f32 nextMotionTime;
    u32 nextModelAttributes, transitionState;
    s16 shiftState, shiftedMotionId;
    u32 shiftAttributes;
    f32 shiftTime, shiftDuration, shiftElapsed, shiftMaxTime;
    /* The array's 0x60-byte element stride leaves four unidentified bytes here. */
    u8 unknown5C[4]; /* No code names this field. */
} PlayerAnimationWork;


extern PlayerAnimationWork lbl_1_bss_1D7C[2];
extern ModelSetupRecord lbl_1_bss_1E3C[113];

void fn_1_C5C4(void);




s32 fn_1_BA38(void)
{
    s16 result = 0;

    lbl_1_bss_1C04[0].active = 1;
    lbl_1_bss_1C04[1].active = 1;
    fn_1_4F14(1, 0x64);
    fn_1_58DC();
    fn_1_2B58();
    fn_1_695C();
    fn_1_24E8();
    return result;
}




s32 fn_1_BD6C(void);

/* Runs the round's result step, then the shared object updates; returns the step's result. */
s16 fn_1_BAA0() {
    s16 result = 0;
    result = fn_1_BD6C();
    fn_1_58DC();
    fn_1_695C();
    fn_1_24E8();
    return result;
}




/* On entering the result mode, starts each player's result motion (motion 2 for a player with a
 * result character, motion 3 otherwise, shifted in over 5 frames), then runs the shared
 * updates and the result display. */
s32 fn_1_BAE0(void) {
    s16 playerIndex;

    switch (lbl_1_bss_196C.roundMode) { /* irregular */
    case 1:
        break;
    case 0:
        playerIndex = 0;
        while (playerIndex < 2) {
            fn_1_4918(&lbl_1_bss_1C04[playerIndex], 6);
            if (lbl_1_bss_1C04[playerIndex].resultCharacter >= 0) {
                lbl_1_bss_1D7C[playerIndex].shiftState = 1;
                lbl_1_bss_1D7C[playerIndex].shiftedMotionId = 2;
                lbl_1_bss_1D7C[playerIndex].shiftTime = 0.0f;
                lbl_1_bss_1D7C[playerIndex].shiftDuration = 5.0f;
                lbl_1_bss_1D7C[playerIndex].shiftAttributes = 0;
                lbl_1_bss_1D7C[playerIndex].attributesToReset = 0x40000001;
            } else {
                lbl_1_bss_1D7C[playerIndex].shiftState = 1;
                lbl_1_bss_1D7C[playerIndex].shiftedMotionId = 3;
                lbl_1_bss_1D7C[playerIndex].shiftTime = 0.0f;
                lbl_1_bss_1D7C[playerIndex].shiftDuration = 5.0f;
                lbl_1_bss_1D7C[playerIndex].shiftAttributes = 0;
                lbl_1_bss_1D7C[playerIndex].attributesToReset = 0x40000001;
            }
            playerIndex += 1;
        }
        lbl_1_bss_196C.roundMode += 1;
        break;
    }
    fn_1_71C();
    fn_1_24E8();
    fn_1_C5C4();
    return 0;
}






/* static */

s32 fn_1_BD20(void) {
    fn_1_24E8();
    fn_1_C5C4();
    return 0;
}

/* static */

s32 fn_1_BD48(void) {
    fn_1_5784();
    return 0;
}






extern f32 lbl_802C028C;

extern void fn_1_308C(MGTIMER *);


s32 fn_1_BD6C(void)
{
    s32 result = 0;
    s16 index;

    switch (lbl_1_bss_196C.phase) {
    case 0:
        if (lbl_1_bss_196C.timer++ > 0x1E) {
            WipeCreate(2, 6, 0x3C);
            lbl_1_bss_196C.timer = 0;
            lbl_1_bss_196C.phase++;
        }
        break;
    case 1:
        if (WipeCheck() == 0) {
            lbl_1_bss_1C04[0].x = -125.0f;
            lbl_1_bss_1C04[0].y = 0.0f;
            lbl_1_bss_1C04[0].z = -478.0f;
            lbl_1_bss_1C04[1].x = 125.0f;
            lbl_1_bss_1C04[1].y = 0.0f;
            lbl_1_bss_1C04[1].z = -478.0f;

            for (index = 0; index < 2; index++) {
                    lbl_1_bss_1C04[index].playerStatus[0] = 0;
                    lbl_1_bss_1C04[index].active = 1;
                    lbl_1_bss_1C04[index].rotationX = 0.0f;
                    lbl_1_bss_1C04[index].rotationY = 0.0f;
                    lbl_1_bss_1C04[index].rotationZ = 0.0f;
                    lbl_1_bss_1D7C[index].appliedModelAttributes =
                        (lbl_1_bss_1D7C[index].appliedMotionId = -1);
                    lbl_1_bss_1D7C[index].motionId = 0;
                    lbl_1_bss_1D7C[index].motionSpeed = 0.0f;
                    lbl_1_bss_1D7C[index].motionTime = 1.0f;
                    lbl_1_bss_1D7C[index].modelAttributes = 0x40000001;
                    lbl_1_bss_1D7C[index].attributesToReset = 0;
                    MgScoreBoxTPLvlSet(lbl_1_bss_0[index].characterBox,
                                       0.0f);
                    MgScoreDispOff(lbl_1_bss_0[index].score);
            }

            if ((s32)(lbl_1_bss_1C04[0].score +
                      lbl_1_bss_1C04[1].score) < 0x40) {
                fn_1_308C(lbl_1_bss_196C.action);
            }
            Hu3DCameraPerspectiveSet(1, 27.027f, 20.0f,
                                     40000.0f, 1.33f);
            lbl_80266A4C.x = -0.368f;
            lbl_80266A4C.y = 218.751f;
            lbl_80266A4C.z = -264.73f;
            lbl_80266A40.x = -9.81f;
            lbl_80266A40.y = 0.0f;
            lbl_80266A40.z = 0.0f;
            lbl_802C028C = 800.0f;

            {
                for (index = 0x23; index < 0x71; index++) {
                    fn_800269E8(lbl_1_bss_1E3C[index].modelId, 1);
                }
            }
            fn_800269E8(lbl_1_bss_1E3C[0x62].modelId, 1);
            fn_800269E8(lbl_1_bss_1E3C[0x4].modelId, 1);
            fn_800269E8(lbl_1_bss_1E3C[0xA].modelId, 1);
            fn_800269E8(lbl_1_bss_1E3C[0xB].modelId, 1);
            fn_800269E8(lbl_1_bss_1E3C[0xE].modelId, 1);
            fn_800269E8(lbl_1_bss_1E3C[0xF].modelId, 1);
            fn_800269E8(lbl_1_bss_1E3C[0x10].modelId, 1);
            fn_800269E8(lbl_1_bss_1E3C[0x11].modelId, 1);

            {
                for (index = 0x1A; index <= 0x22; index++) {
                    fn_80026A44(lbl_1_bss_1E3C[index].modelId, 1);
                    if (index == 0x1D || index == 0x20 || index == 0x22) {
                        fn_800269E8(
                            lbl_1_bss_1E3C[index].modelId, 0x40000001);
                        fn_8002D86C(
                            lbl_1_bss_1E3C[index].modelId,
                            1.0f);
                    }
                }
            }

            lbl_1_bss_196C.result = 0;
            if (lbl_1_bss_1C04[0].score == lbl_1_bss_1C04[1].score) {
                lbl_1_bss_196C.result = 1;
                fn_80071C18(-1, -1, -1, -1);
            } else if (lbl_1_bss_1C04[0].score > lbl_1_bss_1C04[1].score) {
                s32 winnerController;

                lbl_1_bss_1C04[0].resultCharacter =
                    lbl_1_bss_1C04[0].character;
                winnerController = lbl_1_bss_1C04[0].controller;
                if (fn_8006A124(0x1000F) == 0) {
                    lbl_80265750[winnerController].mgCoinBonus = 0xA;
                }
                fn_80071C18((s16)lbl_1_bss_1C04[0].resultCharacter,
                               -1, -1, -1);
            } else if (lbl_1_bss_1C04[0].score < lbl_1_bss_1C04[1].score) {
                s32 winnerController;

                lbl_1_bss_1C04[1].resultCharacter =
                    lbl_1_bss_1C04[1].character;
                winnerController = lbl_1_bss_1C04[1].controller;
                if (fn_8006A124(0x1000F) == 0) {
                    lbl_80265750[winnerController].mgCoinBonus = 0xA;
                }
                fn_80071C18((s16)lbl_1_bss_1C04[1].resultCharacter,
                               -1, -1, -1);
            }
            lbl_1_bss_196C.phase++;
        }
        break;
    case 2:
        if (lbl_1_bss_196C.timer++ > 0x1E) {
            WipeCreate(1, 6, 0x3C);
            lbl_1_bss_196C.timer = 0;
            lbl_1_bss_196C.phase++;
        }
        break;
    case 3:
        if (WipeCheck() == 0) {
            fn_1_5020(0, 5, 0);
            lbl_1_bss_196C.phase++;
        }
        break;
    case 4:
        if (lbl_1_bss_196C.timer++ > 0xA) {
            result = 1;
        }
        break;
    }
    if (lbl_1_bss_196C.phase >= 2) {
        fn_1_C5C4();
    }
    return result;
}




extern MotionControl lbl_1_bss_10[];


void fn_1_C5C4(void)
{
    s16 maximum;
    s16 activeCountLimit;
    f32 progressAsFloat;
    MotionControl *settings;
    s16 effectIndex;
    s16 settingsIndex;
    ModelSetupRecord *effect;

    if (lbl_1_bss_1C04[0].score < lbl_1_bss_1C04[1].score) {
        maximum = lbl_1_bss_1C04[1].score;
    } else {
        maximum = lbl_1_bss_1C04[0].score;
    }
    activeCountLimit = maximum;

    if (lbl_1_bss_196C.waitTimer++ > 0x14 &&
        lbl_1_bss_196C.activeCount < activeCountLimit) {
        progressAsFloat = (f32)(lbl_1_bss_196C.activeCount % 2);
        for (effectIndex = 0x62, settingsIndex = 0x3F;
             effectIndex <= 0x70;
             effectIndex++, settingsIndex++) {
            effect = &lbl_1_bss_1E3C[effectIndex];
            settings = &lbl_1_bss_10[settingsIndex];
            if (effect->state == 0) {
                settings->active = 0;
                settings->input[0].x = -5.0f;
                settings->input[0].z = 0.0f;
                fn_8002C95C(effect->modelId, settings->motionIds[1]);
                fn_8002D86C(effect->modelId, 1.0f);
                fn_8002D414(effect->modelId, fn_1_3D04(50.0f));
                fn_80026A44(effect->modelId, 1);
                fn_800269E8(effect->modelId, 0x40000001);
                effect->position.x = 700.0f;
                effect->position.y = 50.0f;
                effect->position.z = -700.0f + 50.0f * progressAsFloat;
                effect->rotation.x = 0.0f;
                effect->rotation.y = -90.0f;
                effect->rotation.z = 0.0f;
                effect->state = 1;
                lbl_1_bss_196C.activeCount++;
                break;
            }
        }
        lbl_1_bss_196C.waitTimer = 0;
    }

    for (effectIndex = 0x62, settingsIndex = 0x3F;
         effectIndex <= 0x70;
         effectIndex++, settingsIndex++) {
        effect = &lbl_1_bss_1E3C[effectIndex];
        settings = &lbl_1_bss_10[settingsIndex];

        switch (effect->state) {
        case 0:
            break;
        case 1:
            effect->position.x += settings->input[0].x;
            if (effect->position.x <= -700.0f) {
                fn_800269E8(effect->modelId, 1);
                effect->state = 0;
            }
            break;
        }
    }
}
