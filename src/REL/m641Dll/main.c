/* Mole-it! sequence setup, phase callbacks, and teardown. */
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

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];

extern const VoidFunc _dtors[];

void fn_1_142C0(void);

/* Module startup runs registered constructors, then initializes the minigame manager. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_142C0();
    return 0;
}

/* Run registered cleanup callbacks when the module exits. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

s32 lbl_1_bss_7E4;
s16 lbl_1_bss_7E0;
s16 lbl_1_bss_7DE;
s16 lbl_1_bss_7DC;
HUPROCESS *lbl_1_bss_7D8;

extern MGSEQ_PARAM lbl_1_data_4B0;

void fn_1_144B8(s16 mode, s16 frameNo);

/* _prolog calls this to configure the object manager and minigame sequence phases. */
void fn_1_142C0(void) {
    lbl_1_bss_7D8 = omInitObjMan(300, 4096);
    omGameSysInit(lbl_1_bss_7D8);
    MgSeqCreatePrio(&lbl_1_data_4B0, 2000);
    MgSeqModeHookAdd(4, fn_1_144B8);
    /* These phases wait for an explicit sequence advance. */
    MgSeqModeDelaySet(2, -1);
    MgSeqModeDelaySet(7, -1);
}

/* Init callback saves the night-mode setting and initializes the board and score UI. */
void fn_1_1433C(void) {
    s16 initialNightMode;
    s16 displayNightMode;

    lbl_1_bss_7E0 = 0;
    initialNightMode = GwMgNightF;
    lbl_1_bss_7DC = initialNightMode;
    fn_1_126C();
    fn_1_1798();
    fn_1_A220();
    fn_1_13D10();
    displayNightMode = GwMgNightF;
    /* The 3D clear remains black in either night-mode state. */
    if (displayNightMode == 0) {
        Hu3DBGColorSet(0, 0, 0);
    } else {
        Hu3DBGColorSet(0, 0, 0);
    }
    lbl_1_bss_7E4 = HuAudBGMPlay(73);
    MgSeqModeNext();
}

extern void fn_1_B0C0();

extern void fn_1_1D7C(void);

extern void fn_1_13FC4();

extern void fn_1_682C(s16);

extern void fn_1_B0E0(void);

extern void fn_1_14118(void);

extern void fn_1_8A7C(void);

extern void fn_1_10530(void);

extern s16 fn_1_141C0(void);

extern void fn_1_10550(s16);

extern u8 fn_1_10844(s16);

extern void fn_1_10D10();

extern void fn_1_8D9C(s16);

extern u8 fn_1_820C(s32);

extern u8 fn_1_ACE0(s32);

/* Fade-in callback waits for both mode checks before requesting sequence advancement. */
void fn_1_143F4(s32 unusedMode, s32 frameNo) {
    u8 secondCheck;
    u8 firstCheck;

    lbl_1_bss_7E0 = 1;
    firstCheck = fn_1_820C(frameNo);
    secondCheck = fn_1_ACE0(frameNo);
    if (secondCheck && firstCheck) {
        MgSeqModeNext();
    }
    fn_1_1D7C();
}

/* Start callback enters phase 2, resets the result choice, and starts its sequence action. */
void fn_1_14474(void) {
    lbl_1_bss_7E0 = 2;
    lbl_1_bss_7DE = 0;
    fn_1_B0C0();
    fn_1_1D7C();
}

/* The setup-registered mode hook switches scoring on and shows both score displays. */
void fn_1_144B8(s16 mode, s16 frameNo) {
    lbl_1_bss_7E0 = 3;
    lbl_1_bss_7DE = -1;
    fn_1_13FC4();
}

/* Main callback advances scheduled board actions for this frame and refreshes both group totals. */
void fn_1_144F8(s32 unusedMode, s16 frameNo) {
    lbl_1_bss_7E0 = 3;
    fn_1_682C(frameNo);
    fn_1_B0E0();
    fn_1_1D7C();
    fn_1_14118();
}

/* On the first finish frame, requests a stream fade; updates the score display each frame. */
void fn_1_1453C(s32 unusedMode, s16 frameNo) {
    lbl_1_bss_7E0 = 4;
    if (frameNo == 0) {
        HuAudSStreamFadeOut(lbl_1_bss_7E4, 100);
    }
    fn_1_8A7C();
    fn_1_10530();
    fn_1_14118();
}

/* On the first pre-winner frame, records and awards the winner before checking advancement. */
void fn_1_14598(s32 unusedMode, s16 frameNo) {
    u8 shouldAdvance;

    lbl_1_bss_7E0 = 5;
    if (frameNo == 0) {
        lbl_1_bss_7DE = fn_1_141C0();
        fn_1_10550(lbl_1_bss_7DE);
    }
    fn_1_8A7C();
    shouldAdvance = fn_1_10844(frameNo);
    if (shouldAdvance) {
        MgSeqModeNext();
    }
}

/* Winner callback updates board-space appearances using the saved result value. */
void fn_1_14624(void) {
    lbl_1_bss_7E0 = 6;
    fn_1_10D10();
    fn_1_8D9C(lbl_1_bss_7DE);
}

/* Fade-out callback updates board-space appearances using the saved result value. */
void fn_1_14664(void) {
    lbl_1_bss_7E0 = 7;
    fn_1_10D10();
    fn_1_8D9C(lbl_1_bss_7DE);
}

/* Close callback marks phase 8 as the terminal sequence state. */
void fn_1_146A4(void) {
    lbl_1_bss_7E0 = 8;
}

MGSEQ_PARAM lbl_1_data_4B0 = {
    30, MGSEQ_TIMER_RIGHT, (MGSEQ_FUNC)fn_1_1433C, (MGSEQ_FUNC)fn_1_143F4,
    (MGSEQ_FUNC)fn_1_14474, (MGSEQ_FUNC)fn_1_144F8, (MGSEQ_FUNC)fn_1_1453C, (MGSEQ_FUNC)fn_1_14598,
    (MGSEQ_FUNC)fn_1_14624, (MGSEQ_FUNC)fn_1_14664, (MGSEQ_FUNC)fn_1_146A4,
};
