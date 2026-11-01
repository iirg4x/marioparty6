/* Runs the minigame from setup through play, results, and exit. */
#define _MATH_H

#include "game/mg/seqman.h"
#include "game/charman.h"
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
    s16 modeDelayFrames[12];     // Phase duration in frames; -1 waits for a change.
    u16 modeHookCount;                       // Number of slots appended by MgSeqModeHookAdd; reset
                                             // slots remain counted.
    MGSEQ_MODEHOOK modeHooks[64]; // Phase and callback registrations.
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

static s16 defModeDelayTbl[12] = {
    -1, -1, 240, -1, -1, -1, -1, 240, 210, 60, -1, -1
};

static MGSEQ_WORK seqWork;

static HUPROCESS *seqProc;
static u32 seqFrameNo;
static GAMEMESID seqGameMesId;

/* Starts a minigame sequence at the standard process priority. */
void fn_80070788(MGSEQ_PARAM *sequenceParam)
{
    fn_800707B4(sequenceParam, 100);
}

/* MgSeqCreate and minigame setup call this to initialize phases and create both child processes. */
void fn_800707B4(MGSEQ_PARAM *sequenceParam, s32 processPriority)
{
    s16 modeIndex;

    seqWork.param = *sequenceParam;
    seqWork.statusFlags = 0;
    seqWork.resetOnlyWord = 0;
    seqWork.currentMode = 0;
    seqWork.timer = NULL;
    seqWork.modeHookCount = 0;
    for (modeIndex = 0; modeIndex < 12; modeIndex++) {
        if (defModeDelayTbl[modeIndex] == -1) {
            seqWork.modeDelayFrames[modeIndex] = -1;
        } else {
            seqWork.modeDelayFrames[modeIndex] = (s32)(f64)defModeDelayTbl[modeIndex];
        }
    }
    seqProc = fn_8000DAC0(SeqExecProc, processPriority, MGSEQ_PROCESS_STACK_SIZE, 0,
                               fn_8000DD9C());
    fn_8000DAC0(SeqExit, processPriority, MGSEQ_EXIT_PROCESS_STACK_SIZE, 0, seqProc);
    seqGameMesId = GAMEMES_ID_NONE;
}

/* Requests the sequence process and its child process to stop. */
void fn_80070994(void)
{
    seqWork.currentMode = 0;
    HuPrcKill(seqProc);
}

/* The child created by MgSeqCreatePrio advances phases and calls per-phase and registered hooks. */
static void SeqExecProc(void)
{
    s32 phaseDuration;
    MGSEQ_FUNC phaseCallback;

    do {
        if (seqWork.currentMode == 0) {
            seqWork.currentMode = 1;
            phaseDuration = 0;
            if (seqWork.modeDelayFrames[1] == -1) {
                seqWork.statusFlags |= (1 << 1);
            } else {
                phaseDuration = seqWork.modeDelayFrames[1];
            }
            seqFrameNo = 0;
            while (seqFrameNo < phaseDuration ||
                   (seqWork.statusFlags & (1 << 1))) {
                if (seqWork.param.initHook) {
                    phaseCallback = seqWork.param.initHook;
                    phaseCallback(seqWork.currentMode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.currentMode, seqFrameNo);
                if (seqWork.statusFlags & (1 << 0)) {
                    seqWork.statusFlags &= ~(1 << 0);
                    break;
                }
                HuPrcVSleep();
                if (seqWork.statusFlags & (1 << 0)) {
                    seqWork.statusFlags &= ~(1 << 0);
                    break;
                }
                seqFrameNo++;
            }
        }
        seqWork.statusFlags &= ~(1 << 1);
        if (seqWork.currentMode == 1 || seqWork.currentMode == 2) {
            seqWork.currentMode = 2;
            if (!(seqWork.statusFlags & (1 << 5))) {
                WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREV, 60);
            }
            phaseDuration = 0;
            if (seqWork.modeDelayFrames[2] == -1) {
                seqWork.statusFlags |= (1 << 1);
            } else {
                phaseDuration = seqWork.modeDelayFrames[2];
            }
            seqFrameNo = 0;
            while (seqFrameNo < phaseDuration ||
                   (seqWork.statusFlags & (1 << 1))) {
                if (seqWork.param.fadeInHook) {
                    phaseCallback = seqWork.param.fadeInHook;
                    phaseCallback(seqWork.currentMode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.currentMode, seqFrameNo);
                if (seqWork.statusFlags & (1 << 0)) {
                    seqWork.statusFlags &= ~(1 << 0);
                    break;
                }
                HuPrcVSleep();
                seqFrameNo++;
            }
        }
        seqWork.statusFlags &= ~(1 << 1);
        if (seqWork.currentMode == 2 || seqWork.currentMode == 3) {
            seqWork.currentMode = 3;
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
                if (seqWork.statusFlags & (1 << 0)) {
                    seqWork.statusFlags &= ~(1 << 0);
                    break;
                }
                HuPrcVSleep();
                seqFrameNo++;
            }
            if (seqWork.timer) {
                MgTimerModeOnSet(seqWork.timer, 1);
            }
            seqFrameNo = 0;
            SeqExecModeHooks(4, seqFrameNo);
        }
        seqWork.statusFlags &= ~(1 << 1);
        if (seqWork.currentMode == 3 || seqWork.currentMode == 5) {
            seqWork.currentMode = 5;
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
                    (seqWork.statusFlags & (1 << 0))) {
                    HuPrcVSleep();
                    seqWork.statusFlags &= ~(1 << 0);
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
        seqWork.statusFlags &= ~(1 << 1);
        if (seqWork.currentMode == 5 || seqWork.currentMode == 6) {
            seqWork.currentMode = 6;
            seqGameMesId = GameMesCreate(GAMEMES_MES_MG, GAMEMES_MG_TYPE_FINISH);
            seqFrameNo = 0;
            while (GameMesStatGet(seqGameMesId)) {
                if (seqWork.param.finishHook) {
                    phaseCallback = seqWork.param.finishHook;
                    phaseCallback(seqWork.currentMode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.currentMode, seqFrameNo);
                if (seqWork.statusFlags & (1 << 0)) {
                    seqWork.statusFlags &= ~(1 << 0);
                    break;
                }
                HuPrcVSleep();
                seqFrameNo++;
            }
        }
        seqWork.statusFlags &= ~(1 << 1);
        if (seqWork.currentMode == 6 || seqWork.currentMode == 7) {
            /* Preserve a short FINISH phase's frame count into PREWIN; reset it after frame 90 or
             * when entering from another phase. */
            if (seqWork.currentMode != 6 || seqFrameNo >= 90) {
                seqFrameNo = 0;
            }
            seqWork.currentMode = 7;
            phaseDuration = 0;
            if (seqWork.modeDelayFrames[7] == -1) {
                seqWork.statusFlags |= (1 << 1);
            } else {
                phaseDuration = seqWork.modeDelayFrames[7];
            }
            while (seqFrameNo < phaseDuration ||
                   (seqWork.statusFlags & (1 << 1))) {
                if (seqWork.param.preWinnerHook) {
                    phaseCallback = seqWork.param.preWinnerHook;
                    phaseCallback(seqWork.currentMode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.currentMode, seqFrameNo);
                if ((seqWork.statusFlags & (1 << 2)) && seqFrameNo == 90) {
                    if (phaseDuration == (s32)(f64)defModeDelayTbl[7]) {
                        /* If duration is the default 210 frames, showing a record extends it to
                         * 270. */
                        phaseDuration = 270;
                    }
                    GameMesCreate(GAMEMES_MES_MG_RECORD, seqWork.recordValue);
                    HuAudJinglePlay(MGSEQ_JINGLE_RECORD);
                }
                if (seqWork.statusFlags & (1 << 0)) {
                    seqWork.statusFlags &= ~(1 << 0);
                    break;
                }
                HuPrcVSleep();
                seqFrameNo++;
            }
        }
        seqWork.statusFlags &= ~(1 << 1);
        if (seqWork.currentMode == 7 || seqWork.currentMode == 8) {
            seqWork.currentMode = 8;
            if (seqWork.statusFlags & (1 << 3)) {
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
            if (seqWork.modeDelayFrames[8] == -1) {
                seqWork.statusFlags |= (1 << 1);
            } else {
                phaseDuration = seqWork.modeDelayFrames[8];
            }
            seqFrameNo = 0;
            while (seqFrameNo < phaseDuration ||
                   (seqWork.statusFlags & (1 << 1))) {
                if (seqWork.param.winnerHook) {
                    phaseCallback = seqWork.param.winnerHook;
                    phaseCallback(seqWork.currentMode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.currentMode, seqFrameNo);
                if (seqWork.statusFlags & (1 << 0)) {
                    seqWork.statusFlags &= ~(1 << 0);
                    break;
                }
                HuPrcVSleep();
                seqFrameNo++;
            }
        }
        seqWork.statusFlags &= ~(1 << 1);
        if (seqWork.currentMode == 8 || seqWork.currentMode == 9) {
            seqWork.currentMode = 9;
            phaseDuration = 0;
            if (seqWork.modeDelayFrames[9] == -1) {
                seqWork.statusFlags |= (1 << 1);
            } else {
                phaseDuration = seqWork.modeDelayFrames[9];
            }
            if (!(seqWork.statusFlags & (1 << 6))) {
                if (!fn_8006A124(196610)) {
                    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, phaseDuration);
                } else {
                    /* Both flag states currently choose the same wipe mode, type, and duration. */
                    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, phaseDuration);
                }
                HuAudFadeOut(1000);
            }
            seqFrameNo = 0;
            while (seqFrameNo < phaseDuration ||
                   (seqWork.statusFlags & (1 << 1))) {
                if (seqWork.param.fadeOutHook) {
                    phaseCallback = seqWork.param.fadeOutHook;
                    phaseCallback(seqWork.currentMode, seqFrameNo);
                }
                SeqExecModeHooks(seqWork.currentMode, seqFrameNo);
                if (seqWork.statusFlags & (1 << 0)) {
                    seqWork.statusFlags &= ~(1 << 0);
                    break;
                }
                HuPrcVSleep();
                seqFrameNo++;
            }
        }
    } while (seqWork.currentMode != 9 && seqWork.currentMode != 10);
    seqWork.currentMode = 10;
    seqFrameNo = 0;
    if (seqWork.param.closeHook) {
        phaseCallback = seqWork.param.closeHook;
        phaseCallback(seqWork.currentMode, seqFrameNo);
    }
    SeqExecModeHooks(seqWork.currentMode, seqFrameNo);
    fn_8003A5E4(1, 1);
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
        exitCallback(11, 0);
    }
    SeqExecModeHooks(11, 0);
    fn_8003A5E4(1, 1);
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
u32 fn_80071A8C(void)
{
    return seqWork.currentMode;
}

/* Minigame callbacks call this to request phase advance and read the current phase. */
u32 fn_80071A9C(void)
{
    seqWork.statusFlags |= (1 << 0);
    return seqWork.currentMode;
}

/* Minigame callbacks set the next phase; values beyond close are forced to close. */
u32 fn_80071AC4(u32 requestedMode)
{
    if (requestedMode > 10) {
        requestedMode = 10;
    }
    seqWork.currentMode = requestedMode;
    seqWork.statusFlags |= (1 << 0);
    return seqWork.currentMode;
}

/* A minigame callback calls this to keep its phase active until it requests a mode change. */
u16 fn_80071B08(void)
{
    seqWork.statusFlags |= (1 << 1);
    return seqWork.statusFlags;
}

/* A minigame callback calls this to clear the phase hold set by MgSeqModeChangeOff. */
u16 fn_80071B30(void)
{
    seqWork.statusFlags &= ~(1 << 1);
    return seqWork.statusFlags;
}

/* Minigame callbacks call this for the live timer, or the configured limit in frames if absent. */
s32 fn_80071B58(void)
{
    if (!seqWork.timer) {
        return seqWork.param.maxTime * 60;
    } else {
        return MgTimerValueGet(seqWork.timer);
    }
}

/* The minigame result callback supplies a record for pre-winner display; practice mode ignores
 * it. */
void fn_80071BAC(int recordValue)
{
    if (!fn_8006A124(65551)) {
        seqWork.statusFlags |= (1 << 2);
        seqWork.recordValue = recordValue;
    }
}

/* Minigame callbacks call this to read the sequence status flags. */
u16 fn_80071C08(void)
{
    return seqWork.statusFlags;
}

/* The result callback stores winner character slots and enables the winner message. */
u16 fn_80071C18(s16 winnerChar1, s16 winnerChar2, s16 winnerChar3, s16 winnerChar4)
{
    seqWork.winnerCharNo[0] = winnerChar1;
    seqWork.winnerCharNo[1] = winnerChar2;
    seqWork.winnerCharNo[2] = winnerChar3;
    seqWork.winnerCharNo[3] = winnerChar4;
    seqWork.statusFlags |= (1 << 3);
    return seqWork.statusFlags;
}

/* The result callback marks all winner slots empty so the winner phase shows a draw. */
u16 fn_80071C70(void)
{
    seqWork.winnerCharNo[0] = seqWork.winnerCharNo[1] = seqWork.winnerCharNo[2] =
        seqWork.winnerCharNo[3] = -1;
    seqWork.statusFlags |= (1 << 3);
    return seqWork.statusFlags;
}

/* Sets the sequence status bit whose meaning is not established here. */
void fn_80071CCC(void)
{
    seqWork.statusFlags |= (1 << 4);
}

/* Minigame setup sets a phase duration in frames; -1 leaves it waiting for a change. */
void fn_80071CE8(s16 sequenceMode, s16 delayFrames)
{
    if (sequenceMode < 0 || sequenceMode >= 12) {
        return;
    }
    seqWork.modeDelayFrames[sequenceMode] = delayFrames;
}

/* Minigame setup registers a phase callback and gets its slot, or -1 if mode or capacity is
 * invalid. */
u32 fn_80071D20(s16 sequenceMode, MGSEQ_FUNC modeCallback)
{
    if (sequenceMode < 0 || sequenceMode >= 12 ||
        seqWork.modeHookCount >= 64) {
        return -1;
    }
    seqWork.modeHooks[seqWork.modeHookCount].mode = sequenceMode;
    seqWork.modeHooks[seqWork.modeHookCount].hook = modeCallback;
    seqWork.modeHookCount++;
    return seqWork.modeHookCount - 1;
}

/* Minigame setup marks the supplied hook slot unmatched; its callback pointer remains stored. */
void fn_80071DC0(s16 hookSlot)
{
    seqWork.modeHooks[hookSlot].mode = -1;
}

/* Minigame callbacks call this to read the frame counter used by the current phase. */
u32 fn_80071DE4(void)
{
    return seqFrameNo;
}

/* Minigame callbacks call this to replace the current phase frame counter. */
void fn_80071DEC(u32 frameNumber)
{
    seqFrameNo = frameNumber;
}

/* Minigame callbacks call this to access the start or finish announcement message slot. */
s16 fn_80071DF4(void)
{
    return seqGameMesId;
}

/* Minigame control kills the active timer, clears its pointer, and changes the configured time
 * limit. */
void fn_80071DFC(s16 newMaxTime)
{
    if (seqWork.timer) {
        MgTimerKill(seqWork.timer);
    }
    seqWork.timer = NULL;
    seqWork.param.maxTime = newMaxTime;
}

/* Minigame callbacks add the supplied bits to the sequence status flags. */
void fn_80071E60(u16 statusMask)
{
    seqWork.statusFlags |= statusMask;
}

/* Minigame callbacks clear the supplied bits from the sequence status flags. */
void fn_80071E78(u16 statusMask)
{
    seqWork.statusFlags &= ~statusMask;
}
