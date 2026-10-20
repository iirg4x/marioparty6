/* Mode setup, sequence hooks, and camera transitions for this minigame. */
#include "math.h"
#include "dolphin/types.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/gamework.h"
#include "game/wipe.h"
#include "game/mg/seqman.h"
#include "game/charman.h"
#include "game/mg/timer.h"
#include "datadir_enum.h"
#include "REL/m674Dll/scene.h"

/* Started with the stage music and faded by the results exit callback. */
#define M674_STAGE_SE 2220

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
/* Shared state for the minigame sequence, result values, and player slots. */
OMOBJMAN *lbl_1_bss_40;
s32 lbl_1_bss_3C;
s32 lbl_1_bss_38;
s32 lbl_1_bss_34;
s32 lbl_1_bss_30;
s32 lbl_1_bss_2C;
s32 lbl_1_bss_28;
s32 lbl_1_bss_24;
s16 lbl_1_bss_1C[4];
s16 lbl_1_bss_14[4];
s16 lbl_1_bss_C[4];
s16 lbl_1_bss_4[4];
s32 lbl_1_bss_0;

void fn_1_A0(void);
s32 fn_1_A68(s16 sequenceMode, s16 frameNo);
void fn_1_B08(OMOBJ *object, u32 camera, s32 transitionIndex);
s32 fn_1_E18(s32 resourceIndex);
extern s32 fn_1_8A44(void), fn_1_DF88(void), fn_1_DB40(void);
extern s32 fn_1_100F8(s32 player);
extern void fn_1_100A4(void), fn_1_8BCC(void), fn_1_53F8(void), fn_1_FAD0(void);
extern void fn_1_21A8(s32 camera, Vec *direction, Vec *target, f32 fov, f32 radius);
extern void fn_1_289C(u32 cameraMask);

void fn_1_2D4(s16 sequenceMode, s16 frameNo);
void fn_1_310(s16 sequenceMode, s16 frameNo);
void fn_1_4AC(s16 sequenceMode, s16 frameNo);
void fn_1_520(s16 sequenceMode, s16 frameNo);
void fn_1_598(s16 sequenceMode, s16 frameNo);
void fn_1_62C(s16 sequenceMode, s16 frameNo);
void fn_1_8A0(s16 sequenceMode, s16 frameNo);
void fn_1_9B8(s16 sequenceMode, s16 frameNo);
void fn_1_A3C(s16 sequenceMode, s16 frameNo);

OM_CAMERA_VIEW lbl_1_data_0 = {
    {0.0f, -400.0f, -1500.0f}, {-30.0f, 0.0f, 0.0f}, 2200.0f
};
Vec lbl_1_data_1C = {0.0f, 800.0f, 0.0f};
Vec lbl_1_data_28 = {0.0f, 0.0f, 0.0f};
GXColor lbl_1_data_34 = {255, 255, 255, 255};
M674AudioRange lbl_1_data_38[2] = {
    {65535, 32, 96, 32, 64, 110, 2300.0f, 1800.0f, 1250.0f},
    {0}
};
MGSEQ_PARAM lbl_1_data_80 = {
    15, 0, fn_1_2D4, fn_1_310, fn_1_4AC, fn_1_520, fn_1_598,
    fn_1_62C, fn_1_8A0, fn_1_9B8, fn_1_A3C
};

/* Run module constructors before creating the minigame's managers and sequence. */
int _prolog(void) {
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_A0();
    return 0;
}

/* Run module destructors when the overlay is unloaded. */
void _epilog(void) {
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Set up the board, camera, audio ranges, sequence callbacks, and gameplay objects. */
void fn_1_A0(void) {
    extern void fn_1_4E4(s16 sequenceMode, s16 frameNo);
    extern void Hu3DBGColorSet(u8, u8, u8);
    extern s32 HuDataSelHeapReadNum(s32, u32, s32);
    extern void fn_1_27E4(s32, s32);
    extern void fn_1_FA48(OMOBJMAN *);
    extern void fn_1_101B8(OMOBJMAN *);
    extern void fn_1_52FC(OMOBJMAN *);
    extern void fn_1_8ACC(OMOBJMAN *);
    extern void fn_1_E0C4(OMOBJMAN *);
    extern void HuPrcVSleep(void);
    s32 setupStep;
    s32 resourceNumber;

    lbl_1_bss_34 = -1;
    lbl_1_bss_30 = -1;
    lbl_1_bss_28 = 0;
    lbl_1_bss_2C = 0;
    lbl_1_bss_0 = 0;
    for (setupStep = 0; setupStep < 4; setupStep++) {
        lbl_1_bss_1C[setupStep] = 0;
    }
    lbl_1_bss_40 = omInitObjMan(120, 8192);
    omGameSysInit(lbl_1_bss_40);
    MgSeqCreate(&lbl_1_data_80);
    MgSeqModeHookAdd(4, fn_1_4E4);
    fn_1_F8C(lbl_1_bss_40);
    fn_1_1054(lbl_1_bss_40, &lbl_1_data_1C, &lbl_1_data_28, &lbl_1_data_34);
    Hu3DBGColorSet(0, 0, 0);
    fn_1_173C(lbl_1_bss_40, 1, 45.0f, 20.0f, 5000.0f,
              0.0f, &lbl_1_data_0);
    resourceNumber = fn_1_E18(0);
    fn_1_27E4(1, HuDataSelHeapReadNum(resourceNumber, HU_MEMNUM_OVL, HEAP_MODEL));
    fn_1_2398(fn_1_B08);
    fn_1_3308(lbl_1_bss_40, lbl_1_data_38);
    fn_1_FA48(lbl_1_bss_40);
    fn_1_101B8(lbl_1_bss_40);
    fn_1_52FC(lbl_1_bss_40);
    fn_1_8ACC(lbl_1_bss_40);
    fn_1_E0C4(lbl_1_bss_40);
    for (setupStep = 0; setupStep < 10; setupStep++) {
        HuPrcVSleep();
    }
}

/* Advance the INIT sequence immediately; the shared callback guard always returns zero. */
void fn_1_2D4(s16 sequenceMode, s16 frameNo) {
    if (!fn_1_A68(sequenceMode, frameNo)) MgSeqModeNext();
}

/* Start the play sequence after its opening wait and start the stage audio once. */
void fn_1_310(s16 sequenceMode, s16 frameNo)
{
    if (fn_1_A68(sequenceMode, frameNo) != 0) {
        return;
    }
    if (frameNo == 0) {
        lbl_1_bss_3C = 0;
    }
    if (lbl_1_bss_34 == -1 && frameNo >= 18) {
        lbl_1_bss_34 = HuAudBGMPlay(MSM_STREAM_MGMUS_30);
        lbl_1_bss_30 = HuAudFXPlay(M674_STAGE_SE);
    }
    switch (lbl_1_bss_3C) {
    case 0:
        lbl_1_bss_3C++;
        lbl_1_bss_38 = 0;
        MgSeqModeChangeOff();
        break;
    case 1:
        if (++lbl_1_bss_38 < 30 || fn_1_DB40() != 0) {
            break;
        }
        lbl_1_bss_3C++;
        lbl_1_bss_38 = 0;
        lbl_1_bss_0 = 1;
        /* Continue the opening wait after triggering the camera transition. */
    case 2:
        if (++lbl_1_bss_38 >= 60) {
            MgSeqModeNext();
        }
        break;
    }
}

/* Leave the sequence state unchanged when this callback's mode is active. */
void fn_1_4AC(s16 sequenceMode, s16 frameNo) {
    if (fn_1_A68(sequenceMode, frameNo) != 0) {
        return;
    }
}

/* Enable the score display when the sequence's phase-four hook runs. */
void fn_1_4E4(short sequenceMode, short frameNo)
{
    if (!fn_1_A68(sequenceMode, frameNo)) {
        fn_1_100A4();
    }
}

/* During MAIN, advance when no active players or no uncollected targets remain; after frame 720,
 * also set the transition flag. */
void fn_1_520(s16 sequenceMode, s16 frameNo)
{
    if (fn_1_A68(sequenceMode, frameNo) == 0) {
        if (frameNo > 720) {
            lbl_1_bss_24 = 1;
        }
        if ((fn_1_8A44() == 0) || (fn_1_DF88() == 0)) {
            MgSeqModeNext();
        }
    }
}

/* On the first FINISH frame, fade the stage music and set the module's transition flag. */
void fn_1_598(s16 sequenceMode, s16 frameNo) {
    if ((fn_1_A68(sequenceMode, frameNo) == 0) && (frameNo == 0)) {
        if (lbl_1_bss_34 != -1) {
            HuAudSStreamFadeOut(lbl_1_bss_34, 100);
            lbl_1_bss_34 = -1;
        }
        lbl_1_bss_24 = 1;
    }
}

/* Record which characters scored, wipe to the results scene, and advance the sequence. */
void fn_1_62C(s16 sequenceMode, s16 frameNo)
{
    s32 playerIndex;

    if (fn_1_A68(sequenceMode, frameNo) != 0) {
        return;
    }
    if (frameNo == 0) {
        lbl_1_bss_3C = 0;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (fn_1_100F8(playerIndex) != 0) {
                lbl_1_bss_14[playerIndex] = lbl_1_bss_4[playerIndex];
            } else {
                lbl_1_bss_14[playerIndex] = -1;
            }
        }
    }
    switch (lbl_1_bss_3C) {
    case 0:
        lbl_1_bss_3C++;
        lbl_1_bss_38 = 0;
        MgSeqModeChangeOff();
        /* Begin the outgoing results wipe. */
    case 1:
        lbl_1_bss_3C++;
        WipeCreate(2, 6, 60);
        /* Wait for the first results wipe to finish. */
    case 2:
        if (WipeCheck() == 0) {
            lbl_1_bss_3C++;
            lbl_1_bss_38 = 0;
            lbl_1_bss_2C = 1;
            return;
        }
        break;
    case 3:
        if (++lbl_1_bss_38 < 12) {
            break;
        }
        WipeCreate(1, 6, 60);
        lbl_1_bss_3C++;
        /* Wait for the return wipe to finish. */
    case 4:
        if (WipeCheck() != 0) {
            break;
        }
        lbl_1_bss_3C++;
        lbl_1_bss_38 = 0;
        /* Hold the results view for its final frames. */
    case 5:
        if (++lbl_1_bss_38 >= 6) {
            lbl_1_bss_3C++;
            MgSeqModeNext();
        }
        break;
    case 6:
        break;
    default:
        break;
    }
}

/* Add earned bonus coins outside practice mode, then choose the results jingle from the scores. */
void fn_1_8A0(s16 sequenceMode, s16 frameNo) {
    s32 anyPlayerScored;
    s32 playerIndex;

    if ((fn_1_A68(sequenceMode, frameNo) == 0) && (frameNo == 0)) {
        anyPlayerScored = 0;
        playerIndex = 0;
        goto check_players;
    player_loop:
        if (lbl_1_bss_14[playerIndex] >= 0) {
            s16 awardedBonus;
            s32 bonusBeforeAward;

            bonusBeforeAward = GwPlayer[playerIndex].mgCoinBonus;
            awardedBonus = bonusBeforeAward + fn_1_100F8(playerIndex);

            if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                GwPlayer[playerIndex].mgCoinBonus = awardedBonus;
            }
        }
        playerIndex++;
    check_players:
        if (playerIndex < 4) {
            goto player_loop;
        }

        playerIndex = 0;
        goto check_results;
    result_loop:
        if (fn_1_100F8(playerIndex) != 0) {
            anyPlayerScored = 1;
        }
        playerIndex++;
    check_results:
        if (playerIndex < 4) {
            goto result_loop;
        }

        if (anyPlayerScored != 0) {
            HuAudJinglePlay(MSM_STREAM_MGMUS_17);
            return;
        }
        HuAudJinglePlay(MSM_STREAM_FILESEL);
    }
}

/* Fade the stage sound when the results callback reaches its exit state. */
void fn_1_9B8(short sequenceMode, short frameNo)
{
    if (fn_1_A68(sequenceMode, frameNo) == 0 && frameNo == 0) {
        if (lbl_1_bss_30 != -1) {
            HuAudFXFadeOut(lbl_1_bss_30, 1000);
            lbl_1_bss_30 = -1;
        }
    }
}

/* Call prop shutdown and character cleanup, then call the empty score shutdown hook twice. */
void fn_1_A3C(s16 sequenceMode, s16 frameNo)
{
    fn_1_8BCC();
    fn_1_53F8();
    fn_1_FAD0();
    fn_1_FAD0();
}

s32 fn_1_A68(s16 sequenceMode, s16 frameNo) { return 0; }

/* Blend camera center, rotation, and zoom between two views for a transition. */
void fn_1_A70(OM_CAMERA_VIEW *start, OM_CAMERA_VIEW *end, OM_CAMERA_VIEW *result,
    float factor)
{
    fn_1_4F84(&start->center, &end->center, factor, &result->center);
    fn_1_4F84(&start->rot, &end->rot, factor, &result->rot);
    result->zoom = start->zoom + factor * (end->zoom - start->zoom);
}

/* Initialize scene shadows and animate this camera from the opening view to its target. */
void fn_1_B08(OMOBJ *object, u32 camera, s32 transitionIndex)
{
    M674CameraWork *work = object->data;
    M674CameraEntry *entry = &work->entries[transitionIndex];
    OM_CAMERA_VIEW *current = &entry->view;
    OM_CAMERA_VIEW *target = &entry->previousView;

    if (work->shadowInitialized == 0) {
        ++work->shadowInitialized;
        fn_1_21A8(work->cameras, &work->shadowDirection, &entry->target,
                  work->shadowFov, 1200.0f);
        Hu3DShadowColSet(0, 0, 0);
        Hu3DShadowMultiTPLvlSet(0.6f, (s16)camera);
    }

    switch (entry->phase) {
    case 0:
        fn_1_289C(1);
        ++entry->phase;
        entry->timer = 0;
        *current = lbl_1_data_0;
        /* Hold the opening view until the fade-in hook requests the camera transition. */
    case 1:
        if (lbl_1_bss_0 == 0) {
            return;
        }
        ++entry->phase;
        entry->timer = 0;
        /* Start blending toward the camera's target view. */
    case 2: {
        f32 blend = (f32)(0.016666666666666666 * (f64)(f32)entry->timer);

        blend = (f32)sin((3.141592653589793 * (90.0 * (f64)blend)) / 180.0);

        fn_1_4F84(&lbl_1_data_0.center, &target->center, blend, &current->center);
        fn_1_4F84(&lbl_1_data_0.rot, &target->rot, blend, &current->rot);
        current->zoom = lbl_1_data_0.zoom + blend * (target->zoom - lbl_1_data_0.zoom);
        if (++entry->timer < 60) {
            return;
        }
        ++entry->phase;
        entry->timer = 0;
        *current = *target;
        /* Keep the target view while the results transition is pending. */
    }
    case 3:
        if (lbl_1_bss_2C != 0) {
            ++entry->phase;
            entry->timer = 0;
            *current = lbl_1_data_0;
        }
        break;
    default:
        break;
    }
}

/* Add the minigame archive base to a resource-entry index before loading the resource. */
s32 fn_1_E18(s32 resourceIndex) { return resourceIndex + DATA_m674; }

/* Return whether the results wipe has reached the state used by the camera callback. */
s32 fn_1_E20(void) { return lbl_1_bss_2C; }

/* Query the active sequence timer and discard its returned value. */
void fn_1_E30(void) {
    MgSeqTimerValueGet();
}

/* Return the character ID saved for this player, or -1 when they did not score. */
s16 fn_1_E50(s32 playerIndex) {
    return lbl_1_bss_14[playerIndex];
}

/* Resolve a score/result-slot ID to player result status; preserve IDs with no player record. */
u32 fn_1_E68(u32 scoreIndex) {
    s32 playerIndex;
    u32 playerStatus;

    playerIndex = 0;
    goto check;
loop:
    if (scoreIndex == (u32) lbl_1_bss_C[playerIndex]) {
        if (lbl_1_bss_14[playerIndex] >= 0) {
            playerStatus = 1;
        } else {
            playerStatus = (u32) -1;
        }
        return playerStatus;
    }
    playerIndex++;
check:
    if (playerIndex < 4) {
        goto loop;
    }
    return scoreIndex;
}

/* Store the score/result-slot ID and character ID associated with one of four player slots. */
void fn_1_EE4(s32 slotIndex, s32 scoreIndex, s32 characterNo)
{
    slotIndex &= 3;
    lbl_1_bss_C[slotIndex] = (s16)scoreIndex;
    lbl_1_bss_4[slotIndex] = (s16)characterNo;
}

/* Mark one player slot as having reported completion. */
void fn_1_F1C(unsigned int playerIndex) {
    playerIndex&=3;
    lbl_1_bss_1C[playerIndex]=1;
}

/* Return whether all four player slots have registered their completion. */
int fn_1_F3C() {
    int playerIndex;
    for(playerIndex=0;playerIndex<4;playerIndex++) {
        if(lbl_1_bss_1C[playerIndex]==0) return 0;
    }
    return 1;
}
