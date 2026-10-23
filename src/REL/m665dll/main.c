/* Microphone setup and sequence callbacks for Verbal Assault. */
#include "REL/m665dll/player.h"
#include "dolphin/types.h"

#include "math.h"

#include "dolphin/gx.h"

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

#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm_stream.h"

#include "REL/m665dll/sequence.h"

typedef void (*MGSEQ_FUNC)(s16 mode, s16 frameNo);
typedef struct MgSeqParam_s {
    s16 maxTime;
    u8 timerPos;
    MGSEQ_FUNC initHook;
    MGSEQ_FUNC fadeInHook;
    MGSEQ_FUNC startHook;
    MGSEQ_FUNC mainHook;
    MGSEQ_FUNC finishHook;
    MGSEQ_FUNC preWinnerHook;
    MGSEQ_FUNC winnerHook;
    MGSEQ_FUNC fadeOutHook;
    MGSEQ_FUNC closeHook;
} MGSEQ_PARAM;

void MgSeqCreate(MGSEQ_PARAM *param);
u32 MgSeqModeNext(void);
u16 MgSeqDrawSet(void);
void MgSeqModeChangeOff(void);

/* ---- from text_00000.c ---- */

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];

extern const VoidFunc _dtors[];

void fn_1_A0(void);

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
    fn_1_A0();
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

void fn_1_F0(void);

void fn_1_168(s16 mode, s16 frame);

void fn_1_188(s16 mode, s16 frame);

void fn_1_1C8(s16 mode, s16 frame);

void fn_1_304(s16 mode, s16 frame);

void fn_1_3AC(s16 mode, s16 frame);

void fn_1_3B0(s16 mode, s16 frame);

void fn_1_3B4(s16 mode, s16 frame);

void fn_1_3B8(void);

void fn_1_484(void);

void fn_1_550(void);

void fn_1_8EC(void);

void fn_1_F90(OMOBJ *obj);

void fn_1_1228(void);

void fn_1_14FC(void);

void fn_1_1830(void);

void fn_1_1AE4(void);

void fn_1_2010(s16);

void fn_1_22A8(s16 player, s16 motion);

void fn_1_2320(s16, s16);

void fn_1_23C4(s16 player, s16 motion, float blend);

void fn_1_2464(s16 player);

void fn_1_2558(void);

void fn_1_25B0(void);

void fn_1_26D0(void);

void fn_1_273C(s16);

void fn_1_289C(s16 player);

s16 fn_1_2CAC(s16);

void fn_1_2F1C(s16);

void fn_1_33F8(s16 player);

void fn_1_3F28(s16 player);

void fn_1_4430(s16 player);

void fn_1_473C(s16);

void fn_1_48C0(s16);

void fn_1_49C8(s16);

void fn_1_4BD4(s16);

s16 fn_1_4D58(s16);

void fn_1_5258(void);

void fn_1_5308(void);

void fn_1_5354(void);

void fn_1_5394(void);

void fn_1_54D4(void);

void fn_1_5544(s16);

void fn_1_5600(s16 unusedPlayer, f32 value, f32 *out0, f32 *out1);

s16 fn_1_5728(void);

void fn_1_5A24(s16);

void fn_1_5E40(void);

void fn_1_62E0(void);

void fn_1_6478(HU3D_MODEL *modelP, Mtx *mtx);

s16 fn_1_69B0(s16 player);

s16 fn_1_6D88(s16 player, HuVecF *pos);

s16 fn_1_6EE4(HuVecF *a, HuVecF *b, float radius);

void fn_1_6F54(void);

s16 fn_1_7000(s16 unusedPlayer, HuVecF *pos, float angle, s16 candidate);

extern GXColor lbl_1_data_40[2];

extern Point3d lbl_1_data_34;

extern s16 lbl_1_bss_4AC[4];

extern s16 lbl_1_bss_4B4;

extern s16 lbl_1_bss_4B6;

extern s32 lbl_1_bss_2A8;

extern s32 lbl_1_data_48[4];

extern s32 lbl_1_data_80[2];

extern s32 lbl_1_data_88[2];

extern s32 lbl_1_data_90[2];

extern s32 lbl_1_data_98[2];

extern s32 lbl_1_data_A0[2];

MGSEQ_PARAM lbl_1_data_0;

char lbl_1_data_28[20];

const char lbl_1_data_3C[12];

extern u8 lbl_1_bss_20[8];

extern u32 lbl_1_data_68[][2];

/* Runs during module loading to initialize its global object manager and sequence. */
void fn_1_A0(void)
{
    lbl_1_bss_8 = omInitObjMan(256, 8192);
    omGameSysInit(lbl_1_bss_8);
    MgSeqCreate(&lbl_1_data_0);
}

void fn_1_170C(s16 player, s16 motion, f32 frame);

/* ---- from text_000F0.c ---- */

extern s16 GwMgNightF;

void fn_1_E3C(void);

/* The init callback opens microphone recognition and starts the sequence. */
void fn_1_F0(void) {
    s16 nightFlag;

    lbl_1_bss_2 = 0;
    HuMCInit(0);
    HuMCThresholdSet(100);
    HuMCSelWinCreate((-10000.0f), (-10002.0f));
    nightFlag = GwMgNightF;
    /* Day and night use the same recognition context. */
    if (nightFlag == 0) {
        lbl_1_bss_4 = HuMCContextCreate(lbl_1_data_28);
    } else {
        lbl_1_bss_4 = HuMCContextCreate(lbl_1_data_28);
    }
    fn_1_E3C();
    MgSeqModeNext();
}

/* ---- from fn_1_1A4.c ---- */

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

/* ---- from text_00000200.c ---- */

typedef struct SequenceUpdate {
    s16 status; /* Zero for a successful microphone response. */
    u16 score; /* Recognition score of the first candidate. */
    s16 count; /* Number of recognized words in the first candidate. */
    s16 *result; /* Word indices for the first candidate. */
    s32 word; /* First word index or one-based answer-choice value. */
} SequenceUpdate;

void OSReport(const char *format, ...);

/* The microphone listener callback maps the first recognized word to the result flag. */
void fn_1_200(response)
SequenceUpdate *response;
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
    OSReport(lbl_1_data_3C, response->word);
}

/* ---- from fn_1_33C.c ---- */

/* The start callback starts the minigame's streamed music on its first frame. */
void fn_1_33C(s16 mode, s16 frameNo) {
    if (frameNo == 0) {
        lbl_1_bss_250.audioHandle = HuAudBGMPlay(MSM_STREAM_MGMUS_19);
    }
}

/* ---- from fn_1_37C.c ---- */

void fn_1_11DE4(void);

void fn_1_C6B8(void);

s16 fn_1_C64(void);

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
        MgSeqModeNext();
    }
}

/* ---- from fn_1_400.c ---- */

void HuMCListenerKill(void);

void fn_1_11E4C(void);

void fn_1_BFD8(void);

/* The finish callback stops microphone input, fades music, and handles a draw. */
void fn_1_400(s16 unusedArgument, s16 frameNo)
{
    if (frameNo == 0) {
        HuMCListenerKill();
        fn_1_11E4C();
        fn_1_BFD8();
        HuAudSStreamFadeOut(lbl_1_bss_250.audioHandle, 100);
        if (lbl_1_bss_250.winningTeam == -1) {
            MgSeqDrawSet();
        }
    }
}

/* ---- from text_00464.c ---- */

void fn_1_19C(void);

void fn_1_1C0(s16 mode, s16 frameNo);

void fn_1_1F4(s16 mode, s16 frameNo);

void fn_1_1F8(s16 mode, s16 frameNo);

void fn_1_254(s16 mode, s16 frameNo);

void fn_1_280(s16 mode, s16 frameNo);

void fn_1_2F0(s16 mode, s16 frameNo);

void fn_1_31C(s16 mode, s16 frameNo);

void fn_1_320(s16 mode, s16 frameNo);

/* The pre-winner callback begins the winner transition. */
void fn_1_464(s16 mode, s16 frameNo)
{
    fn_1_778();
}

extern s32 lbl_1_data_F18;

extern float lbl_1_bss_1BC;

extern s8 lbl_1_bss_218;

extern s8 lbl_1_bss_219;

extern s8 lbl_1_data_2758;

s32 fn_1_367CC(s32 *player);

void fn_1_484(void)
{
}

void fn_1_488(void)
{
}

/* ---- from fn_1_48C.c ---- */

/* The close callback consumes a pending microphone response and closes the context. */
void fn_1_48C(void) {
    if (lbl_1_bss_2 != 0) {
        HuMCResponseGet();
        lbl_1_bss_2 = 0;
    }
    HuMCClose();
}

void fn_1_C004(void);

void fn_1_1FFC(void);

s32 fn_1_11E50(void);

MGSEQ_PARAM lbl_1_data_0 = {
    300, 2, (MGSEQ_FUNC)fn_1_F0, (MGSEQ_FUNC)fn_1_1A4,
    (MGSEQ_FUNC)fn_1_33C, (MGSEQ_FUNC)fn_1_37C, (MGSEQ_FUNC)fn_1_400, (MGSEQ_FUNC)fn_1_464,
    (MGSEQ_FUNC)fn_1_484, (MGSEQ_FUNC)fn_1_488, (MGSEQ_FUNC)fn_1_48C,
};

char lbl_1_data_28[20] = "/mic/ctx/m665_words";
const char lbl_1_data_3C[12] = "Result %d\n";

/* The fade-in callback advances through prompt, sound, and camera setup phases. */
void fn_1_4D4(void) {
    switch (lbl_1_bss_250.phase) {
    case 0:
        MgSeqModeChangeOff();
        if (lbl_1_bss_250.phaseFrames++ > 60) {
            lbl_1_bss_250.phaseFrames = 0;
            lbl_1_bss_250.phase++;
            break;
        }
        if (lbl_1_bss_250.phaseFrames == 10) {
            HuAudFXPlay(0x85F);
            lbl_1_bss_10[lbl_1_bss_250.mainPlayer].effect.main.soundHandle = HuAudFXPlay(0x863);
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
        view.center.z = (-1190.0f);
        view.rot.x = (-30.0f);
        view.rot.y = 0.0f;
        view.rot.z = 0.0f;
        view.zoom = 6000.0f;
        omCameraViewMove(&view, 120, 0);
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
            MgSeqModeNext();
        }
        break;
    }
}

OMOBJMAN *lbl_1_bss_8;
s16 lbl_1_bss_4;
s16 lbl_1_bss_2;
u16 lbl_1_bss_0;

/* ---- from text_00000778.c ---- */

#include "REL/m665dll/stage.h"

void fn_1_C778(void);

void fn_1_C0E0(void);

void fn_1_50DC(void);

void fn_1_1247C(void);

void fn_1_51F8(void);

void fn_1_12228(void);

void fn_1_53C0(void);

void fn_1_12720(void);

void fn_1_2058(s16 mode, f32 frame);

void fn_1_11D98(s16 playerIndex, s16 mode, f32 frame);

/* The pre-winner callback waits for wipes, stops effects, and sets the winning scene. */
void fn_1_778(void) {

    s32 index;

    switch (lbl_1_bss_250.phase) {
    case 0:
        MgSeqModeChangeOff();
        WipeCreate(2, 0, 60);
        if (lbl_1_bss_10[lbl_1_bss_250.mainPlayer].effect.main.soundHandle != -1) {
            HuAudFXFadeOut(lbl_1_bss_10[lbl_1_bss_250.mainPlayer].effect.main.soundHandle,
                           60);
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
        Hu3DModelAttrReset(lbl_1_bss_250.modelID, 1U);
        for (index = 0; index < 4; index++) {
            Hu3DModelPosSet(lbl_1_bss_250.backgroundModels[index],
                            0.0f, 0.0f,
                            (-3500.0f) -
                                (5000.0f * (f32) index));
        }
        fn_1_C0E0();
        lbl_1_bss_250.finished = 1;
        lbl_1_bss_2E8.fireEnabled = 0;
        Center.x = 0.0f;
        Center.y = 340.0f;
        Center.z = 0.0f;
        CRot.x = (-4.0f);
        CRot.y = 0.0f;
        CRot.z = 0.0f;
        CZoom = 2900.0f;
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
            MgSeqModeNext();
        }
        break;
    }
}
