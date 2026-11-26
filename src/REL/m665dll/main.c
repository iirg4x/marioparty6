/* Microphone setup and sequence callbacks for Verbal Assault. */
#include "REL/m665dll/player.h"
#include "REL/m665dll/main.h"
#include "REL/m665dll/opponent.h"
#include "REL/m665dll/scene.h"
#include "dolphin/types.h"

#include "math.h"

#include "dolphin/gx.h"

#include "dolphin/os.h"

#include "game/mic.h"

#include "game/audio.h"

#include "include/game/mg/actman.h"

#include "include/game/audio.h"

#include "include/game/gamemes.h"

#include "game/main.h"

#include "game/object.h"

#include "game/charman.h"

#include "game/hu3d.h"

#include "game/gamework.h"

#include "game/wipe.h"

#include "game/mg/timer.h"

#include "game/mg/seqman.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/memory.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/sprite.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "REL/m665dll/stage.h"

#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm_stream.h"

#include "REL/m665dll/sequence.h"

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];

extern const VoidFunc _dtors[];

void ObjectSetup(void);

void fn_1_778(void);

/* The module loader runs constructors before setting up the minigame. */
/* The module loader runs constructors before setting up the minigame. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    ObjectSetup();
    return 0;
}

/* The module loader runs registered destructors when unloading the minigame. */
/* The module loader runs registered destructors when unloading the minigame. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

void fn_1_484(s16 mode, s16 frameNo);

MGSEQ_PARAM lbl_1_data_0 = {
    300, 2, fn_1_F0, (MGSEQ_FUNC)fn_1_1A4,
    (MGSEQ_FUNC)fn_1_33C, (MGSEQ_FUNC)fn_1_37C, (MGSEQ_FUNC)fn_1_400, (MGSEQ_FUNC)fn_1_464,
    fn_1_484, fn_1_488, fn_1_48C,
};

char lbl_1_data_28[20] = "/mic/ctx/m665_words";

/* Runs during module loading to initialize its global object manager and sequence. */
void ObjectSetup(void)
{
    lbl_1_bss_8 = fn_8003A8BC(256, 8192);
    fn_80069F1C(lbl_1_bss_8);
    fn_80070788(&lbl_1_data_0);
}

extern s16 lbl_802C0278;

/* The init callback opens microphone recognition and starts the sequence. */
void fn_1_F0(s16 mode, s16 frameNo) {
    s16 nightFlag;

    lbl_1_bss_2 = 0;
    HuMCInit(0);
    HuMCThresholdSet(100);
    HuMCSelWinCreate(-10000.0f, -10002.0f);
    nightFlag = lbl_802C0278;
    /* Day and night use the same recognition context. */
    if (nightFlag == 0) {
        lbl_1_bss_4 = HuMCContextCreate(lbl_1_data_28);
    } else {
        lbl_1_bss_4 = HuMCContextCreate(lbl_1_data_28);
    }
    fn_1_E3C();
    fn_80071A9C();
}

void fn_1_4D4(void);

/* The fade-in callback prepares four answer choices on its first frame. */
void fn_1_1A4(s16 unusedArgument, s16 frameNo)
{
    if (frameNo == 0) {
        HuMCSelWinItemSet(MESSNUM(MESS_MIC_MIN_665, 0), 4,
                          lbl_1_bss_10[lbl_1_bss_250.mainPlayer].controller);
    }
    fn_1_4D4();
}

/* The microphone listener callback maps the first recognized word to the result flag. */
void fn_1_200(response)
MCResponse_s *response;
{
    if ((response->status != 0) || (response->count == 0)) {
        return;
    }
    if (response->score < 3000) {
        lbl_1_bss_0 = 2;
        return;
    }
    switch (*response->result) {
    case 0:
        lbl_1_bss_0 = (1 << 8);
        break;
    case 1:
        lbl_1_bss_0 = (1 << 9);
        break;
    case 2:
        lbl_1_bss_0 = (1 << 10);
        break;
    case 3:
        lbl_1_bss_0 = (1 << 11);
        break;
    case 4:
        lbl_1_bss_0 = (1 << 14);
        break;
    case 5:
        lbl_1_bss_0 = 1;
        break;
    default:
        lbl_1_bss_0 = 0;
        break;
    }
    OSReport("Result %d\n", response->word);
}

/* The start callback starts the minigame's streamed music on its first frame. */
void fn_1_33C(s16 mode, s16 frameNo) {
    if (frameNo == 0) {
        lbl_1_bss_250.audioHandle = HuAudBGMPlay(MSM_STREAM_TENACIOUS);
    }
}

/* The main callback updates player effects and advances when a result is ready. */
void fn_1_37C(s16 unusedArgument, s16 frameNo)
{
    if (frameNo == 0) {
        fn_1_11DE4();
        fn_1_C6B8();
        HuMCListenerCreate(lbl_1_bss_4, fn_1_200,
                            (u8)lbl_1_bss_10[lbl_1_bss_250.mainPlayer].controller);
    }
    if (fn_1_C64() != 0) {
        fn_80071A9C();
    }
}

/* The finish callback stops microphone input, fades music, and handles a draw. */
void fn_1_400(s16 unusedArgument, s16 frameNo)
{
    if (frameNo == 0) {
        HuMCListenerKill();
        fn_1_11E4C();
        fn_1_BFD8();
        HuAudSStreamFadeOut(lbl_1_bss_250.audioHandle, 100);
        if (lbl_1_bss_250.winningTeam == -1) {
            fn_80071C70();
        }
    }
}

/* The pre-winner callback begins the winner transition. */
void fn_1_464(s16 mode, s16 frameNo)
{
    fn_1_778();
}

/* The winner phase has no additional work after the winner transition begins. */
void fn_1_484(s16 mode, s16 frameNo)
{
}

/* The fade-out phase leaves the microphone context open for the close callback. */
void fn_1_488(s16 mode, s16 frameNo)
{
}

/* The close callback consumes a pending microphone response and closes the context. */
void fn_1_48C(s16 mode, s16 frameNo) {
    if (lbl_1_bss_2 != 0) {
        HuMCResponseGet();
        lbl_1_bss_2 = 0;
    }
    HuMCClose();
}

/* The fade-in callback advances through prompt, sound, and camera setup phases. */
void fn_1_4D4(void) {
    switch (lbl_1_bss_250.phase) {
    case 0:
        fn_80071B08();
        if (lbl_1_bss_250.phaseFrames++ > 60) {
            lbl_1_bss_250.phaseFrames = 0;
            lbl_1_bss_250.phase++;
            break;
        }
        if (lbl_1_bss_250.phaseFrames == 10) {
            HuAudFXPlay(MSM_SE_MG665_START_CUE);
            lbl_1_bss_10[lbl_1_bss_250.mainPlayer].effect.main.soundHandle =
                HuAudFXPlay(MSM_SE_MG665_ENGINE_LOOP);
        }
        break;
    case 1:
        fn_1_C004();
        lbl_1_bss_250.phase++;
        break;
    case 2:
        if (lbl_1_bss_250.phaseFrames++ > 60) {
            lbl_1_bss_250.phaseFrames = 0;
            lbl_1_bss_250.phase++;
        }
        break;
    case 3: {
        OM_CAMERA_VIEW view;
        view.center.x = 0.0f;
        view.center.y = 250.0f;
        view.center.z = -1190.0f;
        view.rot.x = -30.0f;
        view.rot.y = 0.0f;
        view.rot.z = 0.0f;
        view.zoom = 6000.0f;
        fn_8003E7A0(&view, 120, 0);
        lbl_1_bss_250.phase++;
        break;
    }
    case 4:
        if (lbl_1_bss_250.phaseFrames++ > 60) {
            lbl_1_bss_250.phase++;
            lbl_1_bss_250.phaseFrames = 0;
        }
        if (lbl_1_bss_250.phaseFrames == 30) {
            fn_1_1FFC();
        }
        break;
    case 5:
        if ((s16) fn_1_11E50() != 0) {
            lbl_1_bss_250.phase++;
        }
        break;
    case 6:
        if (lbl_1_bss_250.phaseFrames++ > 30) {
            lbl_1_bss_250.phaseFrames = 0;
            lbl_1_bss_250.phase = 0;
            fn_80071A9C();
        }
        break;
    }
}

OMOBJMAN *lbl_1_bss_8;
s16 lbl_1_bss_4;
s16 lbl_1_bss_2;
u16 lbl_1_bss_0;

/* The pre-winner callback waits for wipes, stops effects, and sets the winning scene. */
void fn_1_778(void) {

    s32 index;

    switch (lbl_1_bss_250.phase) {
    case 0:
        fn_80071B08();
        WipeCreate(2, 0, 60);
        if (lbl_1_bss_10[lbl_1_bss_250.mainPlayer].effect.main.soundHandle != -1) {
            HuAudFXFadeOut(lbl_1_bss_10[lbl_1_bss_250.mainPlayer].effect.main.soundHandle, 60);
            lbl_1_bss_10[lbl_1_bss_250.mainPlayer].effect.main.soundHandle = -1;
        }
        lbl_1_bss_250.phase += 1;
        return;
    case 1:
        if (WipeCheck() != 0) {
            return;
        }
        for (index = 0; index < 3; index++) {
            if (lbl_1_bss_2E8.killers[index].soundHandle != -1) {
                HuAudFXStop(lbl_1_bss_2E8.killers[index].soundHandle);
                lbl_1_bss_2E8.killers[index].soundHandle = -1;
            }
        }
        lbl_1_bss_10[lbl_1_bss_250.mainPlayer].status.main.flags.bits.moving = 0;
        fn_1_C778();
        fn_80026A44(lbl_1_bss_250.modelID, 1U);
        for (index = 0; index < 4; index++) {
            fn_80026760(lbl_1_bss_250.backgroundModels[index],
                            0.0f, 0.0f,
                            -3500.0f -
                                (5000.0f * (f32) index));
        }
        fn_1_C0E0();
        lbl_1_bss_250.finished = 1;
        lbl_1_bss_2E8.fireEnabled = 0;
        lbl_80266A4C.x = 0.0f;
        lbl_80266A4C.y = 340.0f;
        lbl_80266A4C.z = 0.0f;
        lbl_80266A40.x = -4.0f;
        lbl_80266A40.y = 0.0f;
        lbl_80266A40.z = 0.0f;
        lbl_802C028C = 2900.0f;
        lbl_1_bss_250.scrollSpeed = 0.0f;
        lbl_1_bss_10[lbl_1_bss_250.mainPlayer].speed = 0.0f;
        lbl_1_bss_250.phase += 1;
        return;
    case 2:
        if (lbl_1_bss_250.winningTeam == 0) {
            fn_1_50DC();
            fn_1_1247C();
        } else if (lbl_1_bss_250.winningTeam == 1) {
            fn_1_51F8();
            fn_1_12228();
        } else {
            fn_1_53C0();
            fn_1_12720();
        }
        lbl_1_bss_250.phase += 1;
        return;
    case 3:
        WipeCreate(1, 5, 60);
        lbl_1_bss_250.phase += 1;
        return;
    case 4:
        if (WipeCheck() == 0) {
            if (lbl_1_bss_250.winningTeam == 0) {
                fn_1_2058(4, 8.0f);
                for (index = 0; index < 3; index++) {
                    fn_1_11D98(index, 5, 8.0f);
                }
            } else if (lbl_1_bss_250.winningTeam == 1) {
                fn_1_2058(5, 8.0f);
                for (index = 0; index < 3; index++) {
                    fn_1_11D98(index, 4, 8.0f);
                }
            } else {
                fn_1_2058(5, 8.0f);
                for (index = 0; index < 3; index++) {
                    fn_1_11D98(index, 5, 8.0f);
                }
            }
            fn_80071A9C();
        }
        break;
    }
}
