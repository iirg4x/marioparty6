/* Runs the minigame from setup through play, results, and exit. */
#define _MATH_H

#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/gamemes.h"
#include "game/process.h"
#include "game/audio.h"
#include "game/object.h"
#include "game/flag.h"
#include "game/wipe.h"

typedef struct MgSeqWork_s {
    MGSEQ_PARAM param;                       // Timing, timer placement, and phase callbacks.
    MGTIMER *timer;                          // Active minigame timer, or NULL when absent.
    u16 statusFlags;                         // MGSEQ_STAT_* state flags.
    u16 unusedWord;                          // Unused storage; purpose is not established.
    u16 currentMode;                         // Current MGSEQ_MODE_* phase.
    u16 resetOnlyWord;                       // Cleared during setup; purpose is not established.
    int recordValue;                         // Record value passed to the result message.
    s16 winnerCharNo[4];                     // Winning character slots; -1 means empty.
    s16 modeDelayFrames[MGSEQ_MODE_MAX];     // Phase duration in frames; -1 waits for a change.
    u16 modeHookCount;                       // Number of slots appended by MgSeqModeHookAdd; reset
                                             // slots remain counted.
    MGSEQ_MODEHOOK modeHooks[MGSEQ_MODEHOOK_MAX]; // Phase and callback registrations.
    s32 unusedTail;                          // Unused storage; purpose is not established.
} MGSEQ_WORK;

static void SeqExecProc(void);
static void SeqExit(void);
static void SeqExecModeHooks(s16 sequenceMode, s16 phaseFrame);
static void SeqSetTimerPos(s32 timerPosition, MGTIMER *sequenceTimer);

/* Result jingles selected by this sequence. */
#define MGSEQ_JINGLE_RECORD 66
#define MGSEQ_JINGLE_DRAW 69
#define MGSEQ_JINGLE_WINNER 67
#define MGSEQ_PROCESS_STACK_SIZE 20480
#define MGSEQ_EXIT_PROCESS_STACK_SIZE 12288

static s16 defModeDelayTbl[MGSEQ_MODE_MAX] = {
    -1, -1, 240, -1, -1, -1, -1, 240, 210, 60, -1, -1
};

static MGSEQ_WORK seqWork;

static HUPROCESS *seqProc;
static u32 seqFrameNo;
static GAMEMESID seqGameMesId;

/* Starts a minigame sequence at the standard process priority. */
void MgSeqCreate(MGSEQ_PARAM *sequenceParam)
{
    MgSeqCreatePrio(sequenceParam, 100);
}

/* MgSeqCreate and minigame setup call this to initialize phases and create both child processes. */
void MgSeqCreatePrio(MGSEQ_PARAM *sequenceParam, s32 processPriority)
{
    s16 modeIndex;

    seqWork.param = *sequenceParam;
    seqWork.statusFlags = 0;
    seqWork.resetOnlyWord = 0;
    seqWork.currentMode = MGSEQ_MODE_NONE;
    seqWork.timer = NULL;
    seqWork.modeHookCount = 0;
    for (modeIndex = 0; modeIndex < MGSEQ_MODE_MAX; modeIndex++) {
        if (defModeDelayTbl[modeIndex] == -1) {
            seqWork.modeDelayFrames[modeIndex] = -1;
        } else {
            seqWork.modeDelayFrames[modeIndex] = (s32)(f64)defModeDelayTbl[modeIndex];
        }
    }
    seqProc = HuPrcChildCreate(SeqExecProc, processPriority, MGSEQ_PROCESS_STACK_SIZE, 0,
                               HuPrcCurrentGet());
    HuPrcChildCreate(SeqExit, processPriority, MGSEQ_EXIT_PROCESS_STACK_SIZE, 0, seqProc);
    seqGameMesId = GAMEMES_ID_NONE;
}

/* Requests the sequence process and its child process to stop. */
void MgSeqKill(void)
{
    seqWork.currentMode = MGSEQ_MODE_NONE;
    HuPrcKill(seqProc);
}

/* The child created by MgSeqCreatePrio advances phases and calls per-phase and registered hooks. */
static void SeqExecProc(void)
{
    s32 phaseDuration;
    MGSEQ_FUNC phaseCallback;

    do {
        if (seqWork.currentMode == MGSEQ_MODE_NONE) {
            seqWork.currentMode = MGSEQ_MODE_INIT;
            phaseDuration = 0;
            if (seqWork.modeDelayFrames[MGSEQ_MODE_INIT] == -1) {
                seqWork.statusFlags |= MGSEQ_STAT_MODECHANGE_OFF;
            } else {
                phaseDuration = seqWork.modeDelayFrames[MGSEQ_MODE_INIT];
            }
            seqFrameNo = 0;
            while (seqFrameNo < phaseDuration ||
                   (seqWork.statusFlags & MGSEQ_STAT_MODECHANGE_OFF)) {
                if (seqWork.param.initHook) {
                    phaseCallback = seqWork.param.initHook;
                    phaseCallback(seqWork.currentMode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.currentMode, seqFrameNo);
                if (seqWork.statusFlags & MGSEQ_STAT_MODENEXT) {
                    seqWork.statusFlags &= ~MGSEQ_STAT_MODENEXT;
                    break;
                }
                HuPrcVSleep();
                if (seqWork.statusFlags & MGSEQ_STAT_MODENEXT) {
                    seqWork.statusFlags &= ~MGSEQ_STAT_MODENEXT;
                    break;
                }
                seqFrameNo++;
            }
        }
        seqWork.statusFlags &= ~MGSEQ_STAT_MODECHANGE_OFF;
        if (seqWork.currentMode == MGSEQ_MODE_INIT || seqWork.currentMode == MGSEQ_MODE_FADEIN) {
            seqWork.currentMode = MGSEQ_MODE_FADEIN;
            if (!(seqWork.statusFlags & MGSEQ_STAT_FADEIN_OFF)) {
                WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREV, 60);
            }
            phaseDuration = 0;
            if (seqWork.modeDelayFrames[MGSEQ_MODE_FADEIN] == -1) {
                seqWork.statusFlags |= MGSEQ_STAT_MODECHANGE_OFF;
            } else {
                phaseDuration = seqWork.modeDelayFrames[MGSEQ_MODE_FADEIN];
            }
            seqFrameNo = 0;
            while (seqFrameNo < phaseDuration ||
                   (seqWork.statusFlags & MGSEQ_STAT_MODECHANGE_OFF)) {
                if (seqWork.param.fadeInHook) {
                    phaseCallback = seqWork.param.fadeInHook;
                    phaseCallback(seqWork.currentMode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.currentMode, seqFrameNo);
                if (seqWork.statusFlags & MGSEQ_STAT_MODENEXT) {
                    seqWork.statusFlags &= ~MGSEQ_STAT_MODENEXT;
                    break;
                }
                HuPrcVSleep();
                seqFrameNo++;
            }
        }
        seqWork.statusFlags &= ~MGSEQ_STAT_MODECHANGE_OFF;
        if (seqWork.currentMode == MGSEQ_MODE_FADEIN || seqWork.currentMode == MGSEQ_MODE_START) {
            seqWork.currentMode = MGSEQ_MODE_START;
            seqGameMesId = GameMesCreate(GAMEMES_MES_MG, GAMEMES_MG_TYPE_START);
            if (seqWork.param.maxTime != 0 && seqWork.param.maxTime != 300) {
                seqWork.timer = MgTimerCreate(MGTIMER_POS_TOP);
                MgTimerParamSet(seqWork.timer, seqWork.param.maxTime * 60, 0, 0);
                SeqSetTimerPos(seqWork.param.timerPos, seqWork.timer);
            }
            seqFrameNo = 0;
            while (GameMesStatGet(seqGameMesId)) {
                if (seqWork.param.startHook) {
                    phaseCallback = seqWork.param.startHook;
                    phaseCallback(seqWork.currentMode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.currentMode, seqFrameNo);
                if (seqWork.statusFlags & MGSEQ_STAT_MODENEXT) {
                    seqWork.statusFlags &= ~MGSEQ_STAT_MODENEXT;
                    break;
                }
                HuPrcVSleep();
                seqFrameNo++;
            }
            if (seqWork.timer) {
                MgTimerModeOnSet(seqWork.timer, 1);
            }
            seqFrameNo = 0;
            SeqExecModeHooks(MGSEQ_MODE_PREMAIN, seqFrameNo);
        }
        seqWork.statusFlags &= ~MGSEQ_STAT_MODECHANGE_OFF;
        if (seqWork.currentMode == MGSEQ_MODE_START || seqWork.currentMode == MGSEQ_MODE_MAIN) {
            seqWork.currentMode = MGSEQ_MODE_MAIN;
            seqFrameNo = 0;
            while (1) {
                if (seqWork.param.mainHook) {
                    phaseCallback = seqWork.param.mainHook;
                    phaseCallback(seqWork.currentMode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.currentMode, seqFrameNo);
                if (seqWork.param.maxTime == 300 && seqFrameNo == 16200) {
                    /* The 300-second setting waits 270 seconds, then starts a 30-second timer. */
                    seqWork.timer = MgTimerCreate(MGTIMER_POS_TOP);
                    MgTimerParamSet(seqWork.timer, 1800, 0, 0);
                    SeqSetTimerPos(seqWork.param.timerPos, seqWork.timer);
                    MgTimerModeOnSet(seqWork.timer, 1);
                }
                if ((seqWork.timer && MgTimerDoneCheck(seqWork.timer)) ||
                    (seqWork.statusFlags & MGSEQ_STAT_MODENEXT)) {
                    HuPrcVSleep();
                    seqWork.statusFlags &= ~MGSEQ_STAT_MODENEXT;
                    break;
                }
                HuPrcVSleep();
                seqFrameNo++;
            }
        }
        if (seqWork.timer && !MgTimerDoneCheck(seqWork.timer)) {
            /* An early phase end stops the live timer and drops its sequence pointer. */
            if (seqWork.timer->stopF == 0) {
                seqWork.timer->stopF = 1;
                seqWork.timer->mode = 1;
            }
            seqWork.timer = NULL;
        }
        seqWork.statusFlags &= ~MGSEQ_STAT_MODECHANGE_OFF;
        if (seqWork.currentMode == MGSEQ_MODE_MAIN || seqWork.currentMode == MGSEQ_MODE_FINISH) {
            seqWork.currentMode = MGSEQ_MODE_FINISH;
            seqGameMesId = GameMesCreate(GAMEMES_MES_MG, GAMEMES_MG_TYPE_FINISH);
            seqFrameNo = 0;
            while (GameMesStatGet(seqGameMesId)) {
                if (seqWork.param.finishHook) {
                    phaseCallback = seqWork.param.finishHook;
                    phaseCallback(seqWork.currentMode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.currentMode, seqFrameNo);
                if (seqWork.statusFlags & MGSEQ_STAT_MODENEXT) {
                    seqWork.statusFlags &= ~MGSEQ_STAT_MODENEXT;
                    break;
                }
                HuPrcVSleep();
                seqFrameNo++;
            }
        }
        seqWork.statusFlags &= ~MGSEQ_STAT_MODECHANGE_OFF;
        if (seqWork.currentMode == MGSEQ_MODE_FINISH || seqWork.currentMode == MGSEQ_MODE_PREWIN) {
            /* Preserve a short FINISH phase's frame count into PREWIN; reset it after frame 90 or
             * when entering from another phase. */
            if (seqWork.currentMode != MGSEQ_MODE_FINISH || seqFrameNo >= 90) {
                seqFrameNo = 0;
            }
            seqWork.currentMode = MGSEQ_MODE_PREWIN;
            phaseDuration = 0;
            if (seqWork.modeDelayFrames[MGSEQ_MODE_PREWIN] == -1) {
                seqWork.statusFlags |= MGSEQ_STAT_MODECHANGE_OFF;
            } else {
                phaseDuration = seqWork.modeDelayFrames[MGSEQ_MODE_PREWIN];
            }
            while (seqFrameNo < phaseDuration ||
                   (seqWork.statusFlags & MGSEQ_STAT_MODECHANGE_OFF)) {
                if (seqWork.param.preWinnerHook) {
                    phaseCallback = seqWork.param.preWinnerHook;
                    phaseCallback(seqWork.currentMode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.currentMode, seqFrameNo);
                if ((seqWork.statusFlags & MGSEQ_STAT_RECORD) && seqFrameNo == 90) {
                    if (phaseDuration == (s32)(f64)defModeDelayTbl[MGSEQ_MODE_PREWIN]) {
                        /* If duration is the default 210 frames, showing a record extends it to
                         * 270. */
                        phaseDuration = 270;
                    }
                    GameMesCreate(GAMEMES_MES_MG_RECORD, seqWork.recordValue);
                    HuAudJinglePlay(MGSEQ_JINGLE_RECORD);
                }
                if (seqWork.statusFlags & MGSEQ_STAT_MODENEXT) {
                    seqWork.statusFlags &= ~MGSEQ_STAT_MODENEXT;
                    break;
                }
                HuPrcVSleep();
                seqFrameNo++;
            }
        }
        seqWork.statusFlags &= ~MGSEQ_STAT_MODECHANGE_OFF;
        if (seqWork.currentMode == MGSEQ_MODE_PREWIN || seqWork.currentMode == MGSEQ_MODE_WINNER) {
            seqWork.currentMode = MGSEQ_MODE_WINNER;
            if (seqWork.statusFlags & MGSEQ_STAT_WINNER) {
                if (seqWork.winnerCharNo[0] == -1 && seqWork.winnerCharNo[1] == -1 &&
                    seqWork.winnerCharNo[2] == -1 && seqWork.winnerCharNo[3] == -1) {
                    GameMesCreate(GAMEMES_MES_MG, GAMEMES_MG_TYPE_DRAW);
                    HuAudJinglePlay(MGSEQ_JINGLE_DRAW);
                } else {
                    GameMesCreate(GAMEMES_MES_MG_WINNER, GAMEMES_MG_TYPE_WIN,
                                  seqWork.winnerCharNo[0], seqWork.winnerCharNo[1],
                                  seqWork.winnerCharNo[2], seqWork.winnerCharNo[3]);
                    HuAudJinglePlay(MGSEQ_JINGLE_WINNER);
                }
            }
            phaseDuration = 0;
            if (seqWork.modeDelayFrames[MGSEQ_MODE_WINNER] == -1) {
                seqWork.statusFlags |= MGSEQ_STAT_MODECHANGE_OFF;
            } else {
                phaseDuration = seqWork.modeDelayFrames[MGSEQ_MODE_WINNER];
            }
            seqFrameNo = 0;
            while (seqFrameNo < phaseDuration ||
                   (seqWork.statusFlags & MGSEQ_STAT_MODECHANGE_OFF)) {
                if (seqWork.param.winnerHook) {
                    phaseCallback = seqWork.param.winnerHook;
                    phaseCallback(seqWork.currentMode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.currentMode, seqFrameNo);
                if (seqWork.statusFlags & MGSEQ_STAT_MODENEXT) {
                    seqWork.statusFlags &= ~MGSEQ_STAT_MODENEXT;
                    break;
                }
                HuPrcVSleep();
                seqFrameNo++;
            }
        }
        seqWork.statusFlags &= ~MGSEQ_STAT_MODECHANGE_OFF;
        if (seqWork.currentMode == MGSEQ_MODE_WINNER || seqWork.currentMode == MGSEQ_MODE_FADEOUT) {
            seqWork.currentMode = MGSEQ_MODE_FADEOUT;
            phaseDuration = 0;
            if (seqWork.modeDelayFrames[MGSEQ_MODE_FADEOUT] == -1) {
                seqWork.statusFlags |= MGSEQ_STAT_MODECHANGE_OFF;
            } else {
                phaseDuration = seqWork.modeDelayFrames[MGSEQ_MODE_FADEOUT];
            }
            if (!(seqWork.statusFlags & MGSEQ_STAT_FADEOUT_OFF)) {
                if (!_CheckFlag(FLAG_INST_DECA)) {
                    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, phaseDuration);
                } else {
                    /* Both flag states currently choose the same wipe mode, type, and duration. */
                    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, phaseDuration);
                }
                HuAudFadeOut(1000);
            }
            seqFrameNo = 0;
            while (seqFrameNo < phaseDuration ||
                   (seqWork.statusFlags & MGSEQ_STAT_MODECHANGE_OFF)) {
                if (seqWork.param.fadeOutHook) {
                    phaseCallback = seqWork.param.fadeOutHook;
                    phaseCallback(seqWork.currentMode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.currentMode, seqFrameNo);
                if (seqWork.statusFlags & MGSEQ_STAT_MODENEXT) {
                    seqWork.statusFlags &= ~MGSEQ_STAT_MODENEXT;
                    break;
                }
                HuPrcVSleep();
                seqFrameNo++;
            }
        }
    } while (seqWork.currentMode != MGSEQ_MODE_FADEOUT && seqWork.currentMode != MGSEQ_MODE_CLOSE);
    seqWork.currentMode = MGSEQ_MODE_CLOSE;
    seqFrameNo = 0;
    if (seqWork.param.closeHook) {
        phaseCallback = seqWork.param.closeHook;
        phaseCallback(seqWork.currentMode, seqFrameNo);
    }
    SeqExecModeHooks(seqWork.currentMode, seqFrameNo);
    omOvlReturnEx(1, 1);
    HuPrcEnd();
    while (1) {
        HuPrcVSleep();
    }
}

/* The exit child created by MgSeqCreatePrio waits for system exit, fades out, and calls exit
 * hooks. */
static void SeqExit(void)
{
    MGSEQ_FUNC exitCallback;

    while (omSysExitReq == 0) {
        HuPrcVSleep();
    }
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    HuAudFadeOut(1000);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    if (seqWork.param.closeHook) {
        exitCallback = seqWork.param.closeHook;
        exitCallback(MGSEQ_MODE_EXIT, 0);
    }
    SeqExecModeHooks(MGSEQ_MODE_EXIT, 0);
    omOvlReturnEx(1, 1);
    HuPrcKill(seqProc);
    while (1) {
        HuPrcVSleep();
    }
}

/* SeqExecProc and SeqExit call callbacks registered for this phase and frame. */
static void SeqExecModeHooks(s16 sequenceMode, s16 phaseFrame)
{
    s16 hookIndex;
    MGSEQ_FUNC modeCallback;

    for (hookIndex = 0; hookIndex < seqWork.modeHookCount; hookIndex++) {
        if (seqWork.modeHooks[hookIndex].mode == sequenceMode) {
            modeCallback = seqWork.modeHooks[hookIndex].hook;
            modeCallback(sequenceMode, phaseFrame);
        }
    }
}

/* SeqExecProc calls this to place a new timer at the configured bottom or right screen position. */
static void SeqSetTimerPos(s32 timerPosition, MGTIMER *sequenceTimer)
{
    switch (timerPosition) {
        case MGTIMER_POS_BOTTOM:
            MgTimerPosSet(sequenceTimer, 288.0f, 420.0f);
            break;
        case MGTIMER_POS_RIGHT:
            MgTimerPosSet(sequenceTimer, 525.0f, 64.0f);
            break;
    }
}

/* Minigame callbacks call this to query the current sequence phase. */
u32 MgSeqModeGet(void)
{
    return seqWork.currentMode;
}

/* Minigame callbacks call this to request phase advance and read the current phase. */
u32 MgSeqModeNext(void)
{
    seqWork.statusFlags |= MGSEQ_STAT_MODENEXT;
    return seqWork.currentMode;
}

/* Minigame callbacks set the next phase; values beyond close are forced to close. */
u32 MgSeqModeSet(u32 requestedMode)
{
    if (requestedMode > MGSEQ_MODE_CLOSE) {
        requestedMode = MGSEQ_MODE_CLOSE;
    }
    seqWork.currentMode = requestedMode;
    seqWork.statusFlags |= MGSEQ_STAT_MODENEXT;
    return seqWork.currentMode;
}

/* A minigame callback calls this to keep its phase active until it requests a mode change. */
u16 MgSeqModeChangeOff(void)
{
    seqWork.statusFlags |= MGSEQ_STAT_MODECHANGE_OFF;
    return seqWork.statusFlags;
}

/* A minigame callback calls this to clear the phase hold set by MgSeqModeChangeOff. */
u16 MgSeqModeChangeOn(void)
{
    seqWork.statusFlags &= ~MGSEQ_STAT_MODECHANGE_OFF;
    return seqWork.statusFlags;
}

/* Minigame callbacks call this for the live timer, or the configured limit in frames if absent. */
s32 MgSeqTimerValueGet(void)
{
    if (!seqWork.timer) {
        return seqWork.param.maxTime * 60;
    } else {
        return MgTimerValueGet(seqWork.timer);
    }
}

/* The minigame result callback supplies a record for pre-winner display; practice mode ignores
 * it. */
void MgSeqRecordSet(int recordValue)
{
    if (!_CheckFlag(FLAG_MG_PRACTICE)) {
        seqWork.statusFlags |= MGSEQ_STAT_RECORD;
        seqWork.recordValue = recordValue;
    }
}

/* Minigame callbacks call this to read the sequence status flags. */
u16 MgSeqStatGet(void)
{
    return seqWork.statusFlags;
}

/* The result callback stores winner character slots and enables the winner message. */
u16 MgSeqWinnerSet(s16 winnerChar1, s16 winnerChar2, s16 winnerChar3, s16 winnerChar4)
{
    seqWork.winnerCharNo[0] = winnerChar1;
    seqWork.winnerCharNo[1] = winnerChar2;
    seqWork.winnerCharNo[2] = winnerChar3;
    seqWork.winnerCharNo[3] = winnerChar4;
    seqWork.statusFlags |= MGSEQ_STAT_WINNER;
    return seqWork.statusFlags;
}

/* The result callback marks all winner slots empty so the winner phase shows a draw. */
u16 MgSeqDrawSet(void)
{
    seqWork.winnerCharNo[0] = seqWork.winnerCharNo[1] = seqWork.winnerCharNo[2] =
        seqWork.winnerCharNo[3] = -1;
    seqWork.statusFlags |= MGSEQ_STAT_WINNER;
    return seqWork.statusFlags;
}

/* Sets the sequence status bit whose meaning is not established here. */
void fn_80071CCC(void)
{
    seqWork.statusFlags |= MGSEQ_STAT_UNKBIT;
}

/* Minigame setup sets a phase duration in frames; -1 leaves it waiting for a change. */
void MgSeqModeDelaySet(s16 sequenceMode, s16 delayFrames)
{
    if (sequenceMode < 0 || sequenceMode >= MGSEQ_MODE_MAX) {
        return;
    }
    seqWork.modeDelayFrames[sequenceMode] = delayFrames;
}

/* Minigame setup registers a phase callback and gets its slot, or -1 if mode or capacity is
 * invalid. */
u32 MgSeqModeHookAdd(s16 sequenceMode, MGSEQ_FUNC modeCallback)
{
    if (sequenceMode < 0 || sequenceMode >= MGSEQ_MODE_MAX ||
        seqWork.modeHookCount >= MGSEQ_MODEHOOK_MAX) {
        return -1;
    }
    seqWork.modeHooks[seqWork.modeHookCount].mode = sequenceMode;
    seqWork.modeHooks[seqWork.modeHookCount].hook = modeCallback;
    seqWork.modeHookCount++;
    return seqWork.modeHookCount - 1;
}

/* Minigame setup marks the supplied hook slot unmatched; its callback pointer remains stored. */
void MgSeqModeHookReset(s16 hookSlot)
{
    seqWork.modeHooks[hookSlot].mode = MGSEQ_MODEHOOK_NULL;
}

/* Minigame callbacks call this to read the frame counter used by the current phase. */
u32 MgSeqFrameNoGet(void)
{
    return seqFrameNo;
}

/* Minigame callbacks call this to replace the current phase frame counter. */
void MgSeqFrameNoSet(u32 frameNumber)
{
    seqFrameNo = frameNumber;
}

/* Minigame callbacks call this to access the start or finish announcement message slot. */
s16 MgSeqGameMesIdGet(void)
{
    return seqGameMesId;
}

/* Minigame control kills the active timer, clears its pointer, and changes the configured time
 * limit. */
void MgSeqTimerKill(s16 newMaxTime)
{
    if (seqWork.timer) {
        MgTimerKill(seqWork.timer);
    }
    seqWork.timer = NULL;
    seqWork.param.maxTime = newMaxTime;
}

/* Minigame callbacks add the supplied bits to the sequence status flags. */
void MgSeqStatBitSet(u16 statusMask)
{
    seqWork.statusFlags |= statusMask;
}

/* Minigame callbacks clear the supplied bits from the sequence status flags. */
void MgSeqStatBitReset(u16 statusMask)
{
    seqWork.statusFlags &= ~statusMask;
}
