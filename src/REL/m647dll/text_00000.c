/* Insectiride: minigame sequence callbacks, camera moves and results. */
#include "math.h"
#include "REL/m647dll/module_types.h"
#include "game/flag.h"

#define INSECTIRIDE_INTRO_SOUND 1997

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_A0(void);
void fn_1_B50(void);
void fn_1_1018(HUPROCESS *process);
void fn_1_2094(void);
void fn_1_14B0(void);
void fn_1_28C8(void);
void fn_1_6A2C(void);
void fn_1_66BC(void);
void fn_1_B234(void);
void fn_1_A74(OMOBJ *object);
void fn_1_440(s16 frame);
void fn_1_64C4(void);
void fn_1_1C9C(void);
void fn_1_2CD0(void);
void fn_1_AA64(void);
void fn_1_A8C4(void);
void fn_1_341C(s16 recordIndex);
f32 fn_1_12D8(void);
void fn_1_73C(void);
s32 fn_1_A740(void);
void fn_1_1FAC(void);
void fn_1_1814(void);
void fn_1_15F4(void);
void fn_1_1908(void);
void fn_1_AF18(s16 mode);
void fn_1_A534(void);
void fn_1_B38C(const Point3d *data, s32 mode);
void fn_1_381C(void);
void fn_1_353C(void);

void fn_1_F0(s16 mode, s16 frame);
void fn_1_170(s16 mode, s16 frameNo);
void fn_1_19C(s16 mode, s16 frameNo);
void fn_1_1C0(s16 mode, s16 frameNo);
void fn_1_218(s16 mode, s16 frame);
void fn_1_2EC(s16 mode, s16 frameNo);
void fn_1_368(s16 mode, s16 frameNo);
void fn_1_3EC(s16 mode, s16 frameNo);
void fn_1_420(s16 mode, s16 frameNo);

MGSEQ_PARAM lbl_1_data_0 = {
    300, 0, fn_1_F0, fn_1_170, fn_1_19C, fn_1_1C0, fn_1_218,
    fn_1_2EC, fn_1_368, fn_1_3EC, fn_1_420
};

/* Runs the module constructors and starts Insectiride when the overlay loads. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_A0();
    return 0;
}

/* Runs the module destructors when the overlay unloads. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Creates the object manager and registers the minigame sequence callbacks. */
void fn_1_A0(void)
{
    lbl_1_bss_4 = omInitObjMan(256, 8192);
    omGameSysInit(lbl_1_bss_4);
    MgSeqCreate(&lbl_1_data_0);
}

/* Initializes the scene and its objects during the sequence setup phase. */
void fn_1_F0(s16 mode, s16 frame) {
    fn_1_B50();
    fn_1_1018(lbl_1_bss_4);
    fn_1_2094();
    fn_1_14B0();
    fn_1_28C8();
    fn_1_6A2C();
    fn_1_66BC();
    fn_1_B234();
    lbl_1_bss_0 = omAddObjEx(lbl_1_bss_4, 100, 0, 0, -1, fn_1_A74);
    MgSeqModeNext();
}

/* Advances the introductory camera sequence during fade-in. */
void fn_1_170(s16 mode, s16 frameNo)
{
    fn_1_440(frameNo);
    fn_1_64C4();
}

/* Advances the opening camera while the start sequence runs. */
void fn_1_19C(s16 mode, s16 frameNo)
{
    fn_1_1C9C();
    fn_1_64C4();
}

/* Updates activity selection, mounting, launch and racing during the main phase; fades the
 * introductory sound on entry. */
void fn_1_1C0(s16 mode, s16 frameNo)
{
    if (frameNo == 0) {
        HuAudFXFadeOut(lbl_1_bss_174.soundEffectId, 1000);
    }
    lbl_1_bss_174.finishPhaseActive = 0;
    fn_1_2CD0();
    fn_1_64C4();
}

/* Updates players during the finish phase, fading music on entry. */
void fn_1_218(s16 mode, s16 frame) {
    s32 recordIndex;

    if (frame == 0) {
        fn_1_AA64();
        fn_1_A8C4();
        HuAudSStreamFadeOut(lbl_1_bss_174.musicHandle, 100);
    }
    lbl_1_bss_174.finishPhaseActive = 1;
    if (lbl_1_bss_18 != 0) {
        lbl_1_bss_174.sequencePhase = 0;
        lbl_1_bss_174.phaseFrame = 0;
        for (recordIndex = 0; recordIndex < 4; recordIndex++) {
            fn_1_341C(recordIndex);
        }
        fn_1_12D8();
    } else {
        for (recordIndex = 0; recordIndex < 4; recordIndex++) {
            fn_1_341C(recordIndex);
        }
    }
}

/* Runs the pre-winner sequence; with no finishers, waits for the animation and at least 201
 * frames. */
void fn_1_2EC(s16 mode, s16 frameNo) {
    if (lbl_1_bss_18 != 0) {
        fn_1_73C();
        return;
    }
    if (frameNo == 0) {
        MgSeqModeChangeOff();
        HuAudJinglePlay(MSM_STREAM_FILESEL);
    }
    if (((s16)fn_1_A740() != 0) && (frameNo > 200)) {
        MgSeqModeNext();
    }
}

/* Handles the winner phase; with no finishers, awards all four players three coins outside
 * practice. */
void fn_1_368(s16 mode, s16 frameNo) {
    s32 playerIndex;

    if (lbl_1_bss_18 != 0) {
        fn_1_1FAC();
        return;
    }
    playerIndex = 0;
    while (playerIndex < 4) {
        if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
            GwPlayer[playerIndex].mgCoinBonus = 3;
        }
        playerIndex += 1;
    }
    MgSeqModeNext();
}

/* Updates the results camera during fade-out when at least one player has finished. */
void fn_1_3EC(s16 mode, s16 frameNo) {
    if (lbl_1_bss_18 != 0) {
        fn_1_1FAC();
    }
}

/* Closes the scene through its sequence cleanup callback. */
void fn_1_420(s16 mode, s16 frameNo)
{
    fn_1_1814();
}

/* Advances the introduction through camera moves, animation cues and timed pauses. */
void fn_1_440(s16 frame) {
    OM_CAMERA_VIEW view;

    switch (lbl_1_bss_174.sequencePhase) {
        case 0:
            MgSeqModeChangeOff();
            lbl_1_bss_174.musicHandle = HuAudBGMPlay(MSM_STREAM_MGMUS_LAST_KOOPA);
            lbl_1_bss_174.soundEffectId = HuAudFXPlay(INSECTIRIDE_INTRO_SOUND);
            lbl_1_bss_174.sequencePhase++;
            break;
        case 1:
            if (frame == 120) {
                view.center.x = 0.0f;
                view.center.y = 300.0f;
                view.center.z = 430.0f;
                view.rot.x = -28.0f;
                view.rot.y = view.rot.z = 0.0f;
                view.zoom = 6400.0f;
                omCameraViewMoveSimpleMulti(2, &view, 180);
                Hu3DMotionSpeedSet(lbl_1_bss_174.modelId, 1.0f);
                fn_1_AF18(1);
                lbl_1_bss_174.sequencePhase++;
            }
            break;
        case 2:
            if (omCameraViewCheck(2) != 0) {
                lbl_1_bss_174.sequencePhase++;
            }
            /* The animation cue is independent of the camera completing this frame. */
            if (Hu3DMotionTimeGet(lbl_1_bss_174.modelId) == 32.0f) {
                Hu3DModelAttrSet(lbl_1_bss_174.modelId, HU3D_MOTATTR_LOOP);
                fn_1_1908();
                fn_1_15F4();
            }
            break;
        case 3:
            if (lbl_1_bss_174.phaseFrame++ >= 30) {
                lbl_1_bss_174.phaseFrame = 0;
                lbl_1_bss_174.sequencePhase++;
            }
            break;
        case 4:
            view.center.x = 0.0f;
            view.center.y = 150.0f;
            view.center.z = -1720.0f;
            view.rot.x = -16.0f;
            view.rot.y = view.rot.z = 0.0f;
            view.zoom = 5230.0f;
            omCameraViewMoveSimpleMulti(2, &view, 45);
            lbl_1_bss_174.sequencePhase++;
            break;
        case 5:
            if (omCameraViewCheck(2) != 0) {
                lbl_1_bss_174.sequencePhase++;
            }
            break;
        case 6:
            if (lbl_1_bss_174.phaseFrame++ >= 30) {
                lbl_1_bss_174.sequencePhase = 0;
                lbl_1_bss_174.phaseFrame = 0;
                MgSeqModeNext();
            }
            break;
        default:
            break;
    }
    fn_1_1C9C();
}

/* Runs the winner reveal through two wipes, then emits repeated celebration bursts. */
void fn_1_73C(void) {
    s32 recordIndex;
    switch (lbl_1_bss_174.sequencePhase) {
        case 0:
            MgSeqModeChangeOff();
            for (recordIndex = 0; recordIndex < 4; recordIndex++) {
                fn_1_341C(recordIndex);
            }
            lbl_1_bss_174.sequencePhase++;
            break;
        case 1:
            if ((s16)fn_1_A740() != 0) {
                lbl_1_bss_174.sequencePhase++;
            }
            for (recordIndex = 0; recordIndex < 4; recordIndex++) {
                fn_1_341C(recordIndex);
            }
            break;
        case 2:
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
            lbl_1_bss_174.sequencePhase++;
            break;
        case 3:
            if (WipeCheck() == 0) {
                CenterM[1].x = (0.0f);
                CenterM[1].y = (300.0f);
                CenterM[1].z = (-750.0f);
                CRotM[1].x = (-12.0f);
                CRotM[1].y = CRotM[1].z = (0.0f);
                CZoomM[1] = (3000.0f);
                Hu3DModelAttrSet(lbl_1_bss_174.openingModelId, HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(lbl_1_bss_174.gameplayModelId, HU3D_ATTR_DISPOFF);
                fn_1_381C();
                fn_1_A534();
                HuAudFXStop(lbl_1_bss_174.soundEffectId);
                lbl_1_bss_174.sequencePhase++;
            }
            break;
        case 4:
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREV, 60);
            lbl_1_bss_174.sequencePhase++;
            break;
        case 5:
            if (WipeCheck() == 0) {
                HuVecF burstOrigin = {0.0f, 1000.0f, -280.0f};
                fn_1_B38C(&burstOrigin, 40);
                lbl_1_bss_174.phaseFrame = 0;
                lbl_1_bss_174.sequencePhase++;
            }
            break;
        case 6:
            if (lbl_1_bss_174.phaseFrame++ > 45) {
                MgSeqModeNext();
            }
            if (lbl_1_bss_174.phaseFrame == 30) {
                HuAudJinglePlay(MSM_STREAM_MGMUS_17);
                fn_1_353C();
            }
            if ((lbl_1_bss_174.phaseFrame % 4) == 0) {
                HuVecF burstOrigin = {0.0f, 1000.0f, -280.0f};
                fn_1_B38C(&burstOrigin, 8);
            }
            break;
        default:
            break;
    }
    if (lbl_1_bss_174.sequencePhase >= 4) {
        fn_1_1FAC();
    }
    fn_1_64C4();
}

HUPROCESS *lbl_1_bss_4;
OMOBJ *lbl_1_bss_0;
