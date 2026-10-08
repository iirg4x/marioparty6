// Board music and sound-effect playback helpers.
#define _MATH_H

#include "game/board/audio.h"
#include "game/board/main.h"
#include "game/charman.h"
#include "game/disp.h"
#include "game/memory.h"

#include <string.h>

typedef struct MBAudFXObj_s {
    int seId; // Sound-effect resource ID.
    int seNo; // Active sound-effect playback handle, or a sentinel when inactive.
    int vol; // Requested sound-effect volume.
    BOOL emitterF; // Whether playback uses a world-position emitter.
    BOOL pauseF; // Whether the tracked sound is paused.
    BOOL lockF; // Whether this entry is retained until its owner releases it.
    Vec pos; // Emitter position in world coordinates.
    int *fxRef; // Optional caller-owned slot updated when deferred playback starts.
} MBAUDFXOBJ;

typedef struct MBAudFXData_s {
    int seId; // Sound-effect resource ID queued for delayed playback.
    int delay; // Remaining process frames before playback.
    int type; // MB_AUD_FX_TYPE_* playback mode.
    s8 pan; // Stereo pan for MB_AUD_FX_TYPE_PAN playback.
    Vec pos; // World position for MB_AUD_FX_TYPE_EMITTER playback.
} MBAUDFXDATA;

typedef struct MBMusData_s {
    int streamNo; // Audio stream handle, or MSM_STREAMNO_NONE when unused.
    s16 id; // Music resource ID associated with streamNo.
    s16 vol; // Current requested stream volume.
    s16 closeTime; // Frames to retain a stream after the engine reports completion.
    s16 stopTime; // Frames remaining before a requested stop is finalized.
    s16 fadeTime; // Frames remaining in a pause/fade operation.
    s16 pauseF; // TRUE while this channel is marked as paused.
    s16 stopF; // TRUE while a stop countdown is active.
    HUPROCESS *proc; // Process that monitors this channel's stream.
} MBMUSDATA;

typedef struct BoardMusData_s {
    u32 boardNo; // Board index; 0xFFFFFFFF terminates the table.
    s32 dayMusId; // Stream resource used during daytime.
    s32 nightMusId; // Stream resource used during nighttime.
    BOOL timeF; // TRUE when the board has separate day and night music.
} BOARDMUSDATA;

typedef struct MusBoardFadeData_s {
    int chan; // Channel whose current stream is fading or pausing.
    int nextChan; // Channel used for the replacement stream.
    int speed; // Fade or pause-fade speed passed to the audio engine.
    int fadeSpeed; // Fade-in speed for the replacement stream.
    int musId; // Replacement music ID; MSM_STREAM_NONE selects board music.
    BOOL pauseF; // TRUE requests pause-fade behavior for the outgoing stream.
} MUSBOARDFADEDATA;

enum {
    MB_AUD_FX_TYPE_NORMAL,
    MB_AUD_FX_TYPE_PAN,
    MB_AUD_FX_TYPE_EMITTER
};

static BOARDMUSDATA boardMusData[] = {
    { 0, 13, 14, TRUE },
    { 1, 15, 16, TRUE },
    { 2, 17, 18, TRUE },
    { 3, 19, 20, TRUE },
    { 4, 21, 22, TRUE },
    { 5, 23, 24, TRUE },
    { 6, 45, 45, FALSE },
    { 7, 46, 46, FALSE },
    { 8, 47, 47, FALSE },
    { 9, 37, 38, TRUE },
    { 10, 44, 44, FALSE },
    { -1, 0, 0, FALSE }
};

static u32 guideFxTbl[][2] = {
    { 948, 940 },
    { 949, 941 },
    { 950, 942 },
    { 951, 943 },
    { 952, 944 },
    { 953, 945 },
    { 954, 946 },
    { 955, 947 },
    { -1, -1 }
};

static MBAUDFXOBJ audFXObjData[MB_AUD_FX_OBJ_MAX];
static MBAUDFXDATA audFXData[MB_AUD_FX_DELAY_MAX];
static MBMUSDATA musData[MB_MUS_CHAN_MAX];

static int dummyId = -1;

static int musChanId[MB_MUS_CHAN_MAX];
static int audFXDelay;
static int audFXObjOnTimer;
static BOOL audFXObjOnF;
static HUPROCESS *audFXProc;
static HUPROCESS *musBoardFadeProc;

extern HUPROCESS *mbMainProc;
extern void mbPlayerPosGet(int playerNo, Vec *pos);
extern void Hu3D3Dto2D(Vec *src, s16 cameraBit, Vec *dst);

static inline int BoardNoGet(void)
{
    return GwSystem.boardNo;
}

static void MusPlay(void);
static void MusPlayKill(void);
static void MusBoardFade(void);
static void AudFXMain(void);
static void AudFXMainDestroy(void);

// Pauses or resumes a tracked sound effect; repeated requests in the current state are ignored.
static inline void AudFXObjPauseSet(int seNo, BOOL pauseF)
{
    MBAUDFXOBJ *audFx = &audFXObjData[0];
    int i;

    for (i = 0, audFx = &audFXObjData[0]; i < MB_AUD_FX_OBJ_MAX; i++, audFx++) {
        if (audFx->seNo == seNo) {
            break;
        }
    }
    if (i >= MB_AUD_FX_OBJ_MAX) {
        return;
    }
    if (pauseF == FALSE) {
        if (audFx->pauseF != FALSE) {
            (void)i;
        } else {
            (void)i;
            return;
        }
        audFx->pauseF = FALSE;
        msmSePause(audFx->seNo, FALSE, 1000);
    } else {
        if (audFx->pauseF != FALSE) {
            (void)audFx;
        } else {
            audFx->pauseF = TRUE;
            msmSePause(audFx->seNo, TRUE, 1000);
            return;
        }
    }
}

// Stores a delayed emitter position when a queued sound has one.
static inline void AudFXPosSet(MBAUDFXDATA *audFx, Vec *pos)
{
    if (pos != NULL) {
        audFx->pos = *pos;
    }
}

// Initializes board music and sound-effect tracking when board mode starts.
void mbAudInit(void)
{
    MBAUDFXOBJ *audFx = &audFXObjData[0];
    MBAUDFXDATA *delayFx = &audFXData[0];
    int i;

    for (i = 0; i < MB_MUS_CHAN_MAX; i++) {
        musData[i].streamNo = MSM_STREAMNO_NONE;
        musData[i].id = MSM_STREAM_NONE;
        musData[i].vol = 0;
        musData[i].closeTime = 0;
        musData[i].stopTime = 0;
        musData[i].stopF = FALSE;
        musData[i].fadeTime = 0;
        musData[i].pauseF = FALSE;
        musData[i].proc = NULL;
    }
    musBoardFadeProc = NULL;
    dummyId = -1;

    memset(&audFXObjData[0], 0, sizeof(audFXObjData));
    for (i = 0, audFx = &audFXObjData[0]; i < MB_AUD_FX_OBJ_MAX; i++, audFx++) {
        audFx->seId = MSM_SE_NONE;
        audFx->seNo = MSM_SENO_NONE;
    }

    audFXProc = HuPrcChildCreate(AudFXMain, 0x2001, 0x4000, 0, mbMainProc);
    HuPrcDestructorSet2(audFXProc, AudFXMainDestroy);
    if (_CheckFlag(FLAG_BOARD_STAR_RESET)) {
        audFXObjOnF = TRUE;
        audFXObjOnTimer = 1;
    } else {
        audFXObjOnF = FALSE;
        audFXObjOnTimer = 0;
    }

    audFXDelay = 0;
    memset(&audFXData[0], 0, sizeof(audFXData));
    for (i = 0; i < MB_AUD_FX_DELAY_MAX; i++, delayFx++) {
        delayFx->seId = MSM_SE_NONE;
    }
    mbAudFXPlay(MSM_SE_BRD00_102);
}

// Selects the current board's day/night stream and starts it on the background channel.
void mbMusBoardPlay(void)
{
    int i;
    int musId = boardMusData[0].dayMusId;

    for (i = 0; boardMusData[i].boardNo != 0xFFFFFFFF; i++) {
        if (boardMusData[i].boardNo == BoardNoGet()) {
            if (boardMusData[i].timeF) {
                if (GwSystem.curTime == FALSE) {
                    musId = boardMusData[i].dayMusId;
                } else {
                    musId = boardMusData[i].nightMusId;
                }
            } else {
                musId = boardMusData[i].dayMusId;
            }
            break;
        }
    }
    mbMusPlay(MB_MUS_CHAN_BG, musId, MSM_VOL_MAX, 0);
}

// Starts a stream on a board music channel; called by board events and fade transitions.
// The parameter block is filled, but the start call takes only ID and channel, so volume/fade are
// not applied here.
void mbMusPlay(int chan, int musicId, s8 volume, u16 fadeSpeed)
{
    MBMUSDATA *musP = &musData[chan];
    int streamHandle;
    int chanOther;
    MSM_STREAMPARAM streamParam;

    if (musicOffF || omSysExitReq) {
        return;
    }

    streamParam.flag = MSM_STREAMPARAM_CHAN | MSM_STREAMPARAM_VOL;
    // This opposite-channel value is computed but not used by the remaining code.
    chanOther = chan ^ 1;
    if (musData[chan].streamNo != MSM_STREAMNO_NONE && musicId == musData[chan].id) {
        if (mbMusStatGet(chan) == MSM_STREAM_PLAY || mbMusStatGet(chan) == MSM_STREAM_PAUSEOUT) {
            return;
        }
    }
    if (musData[chan].streamNo != MSM_STREAMNO_NONE && musicId == musData[chan].id &&
        mbMusStopCheck(chan)) {
        mbMusPauseFadeOut(chan, FALSE, 0);
        return;
    }
    if (mbMusCheck(chan)) {
        if (mbMusEndCheck(chan) == FALSE) {
            mbMusFadeOut(chan);
        }
        while (mbMusCheck(chan)) {
            HuPrcVSleep();
        }
    }
    mbMusStop(chan, FALSE);

    if (fadeSpeed) {
        streamParam.flag |= MSM_STREAMPARAM_FADESPEED;
        streamParam.fadeSpeed = fadeSpeed;
    }
    streamParam.chan = chan * 2;
    streamParam.vol = volume;
    streamHandle = HuAudSStreamChanPlay(musicId, chan * 2);
    if (streamHandle >= 0) {
        int *chanP;

        musP->streamNo = streamHandle;
        musP->id = musicId;
        musP->vol = volume;
        musP->closeTime = 3;
        musP->stopTime = 60;
        musP->stopF = FALSE;
        musP->proc = HuPrcChildCreate(MusPlay, 0x2001, 0x2000, 0, mbMainProc);
        HuPrcSetStat(musP->proc, HU_PRC_STAT_PAUSE_ON | HU_PRC_STAT_UPAUSE_ON);
        musP->proc->property = chanP = &musChanId[chan];
        *chanP = chan;
        HuPrcDestructorSet2(musP->proc, MusPlayKill);
    }
}

// Fades the selected channel using the default one-second fade.
void mbMusFadeOut(int chan)
{
    MBMUSDATA *musP = &musData[chan];

    mbMusFadeOutSpeed(chan, 1000);
}

// Stops the selected channel immediately and clears its tracked stream state.
void mbMusStop(int chan, BOOL unused)
{
    MBMUSDATA *musP = &musData[chan];

    HuAudSStreamStop(musP->streamNo);
    musP->streamNo = MSM_STREAMNO_NONE;
    musP->id = MSM_STREAM_NONE;
    musP->closeTime = 0;
    musP->stopTime = 0;
    musP->stopF = FALSE;
    musP->fadeTime = 0;
    musP->pauseF = FALSE;
}

// Fades a live stream and records a timeout for the channel monitor process.
void mbMusFadeOutSpeed(int chan, u16 speed)
{
    MBMUSDATA *musP = &musData[chan];

    if (musP->streamNo == MSM_STREAMNO_NONE) {
        return;
    }
    if (mbMusEndCheck(chan)) {
        return;
    }
    HuAudSStreamFadeOut(musP->streamNo, speed);
    musP->stopTime = (s16)((speed * 60) / 1000) + 5;
    musP->stopF = TRUE;
}

void mbMusStub(void)
{
}

// Updates a live stream's volume (negative values select MSM_VOL_MAX) and optional fade speed.
void mbMusParamSet(int chan, s8 vol, u16 fadeSpeed)
{
    MBMUSDATA *musP = &musData[chan];
    MSM_STREAMPARAM streamParam;

    if (musP->streamNo == MSM_STREAMNO_NONE) {
        return;
    }
    streamParam.flag = MSM_STREAMPARAM_CHAN | MSM_STREAMPARAM_VOL;
    if (fadeSpeed) {
        streamParam.flag |= MSM_STREAMPARAM_FADESPEED;
        streamParam.fadeSpeed = fadeSpeed;
    }
    if (vol < 0) {
        vol = MSM_VOL_MAX;
    }
    streamParam.vol = vol;
    musP->vol = vol;
    // The parameter block is not passed; the helper receives volume and fade speed directly.
    HuAudSStreamParamSet(musP->streamNo, vol, fadeSpeed);
}

// Starts or reverses the channel's pause fade when its current state allows it; nonpositive speeds
// use a 1000 ms engine fade and a 62-frame tracked countdown.
void mbMusPauseFadeOut(int chan, BOOL pauseF, int speed)
{
    MBMUSDATA *musP = &musData[chan];

    if (musP->streamNo == MSM_STREAMNO_NONE) {
        return;
    }
    if (pauseF) {
        if (mbMusStopCheck(chan)) {
            return;
        }
    } else if (mbMusStopCheck(chan) == FALSE) {
        return;
    }

    musData[chan].pauseF = pauseF;
    if (speed > 0) {
        HuAudSStreamPauseFadeOut(musP->streamNo, pauseF, speed);
        musData[chan].fadeTime = 2.0f + ((speed * 60) / 1000.0f);
    } else {
        HuAudSStreamPauseFadeOut(musP->streamNo, pauseF, 1000);
        musData[chan].fadeTime = 62;
    }
}

// Reports the tracked channel state, prioritizing stopped and paused flags.
s32 mbMusStatGet(int chan)
{
    MBMUSDATA *musP = &musData[chan];

    if (musP->streamNo == MSM_STREAMNO_NONE) {
        return MSM_STREAM_DONE;
    }
    if (musP->stopF) {
        return MSM_STREAM_STOP;
    }
    if (musP->pauseF) {
        return MSM_STREAM_PAUSEIN;
    }
    return MSM_STREAM_PLAY;
}

// Fades all tracked music channels and closes tracked board sound effects.
void mbAudClose(void)
{
    int i;

    for (i = 0; i < MB_MUS_CHAN_MAX; i++) {
        MBMUSDATA *musP = &musData[i];

        mbMusFadeOutSpeed(i, 1000);
    }
    mbAudFXObjClose();
}

// Returns whether a stream handle is assigned to this channel.
BOOL mbMusCheck(int chan)
{
    if (musData[chan].streamNo != MSM_STREAMNO_NONE) {
        return TRUE;
    }
    return FALSE;
}

// Returns whether the tracked channel is in the pause-in state.
BOOL mbMusStopCheck(int chan)
{
    if (mbMusCheck(chan) == FALSE) {
        return FALSE;
    }
    if (mbMusStatGet(chan) == MSM_STREAM_PAUSEIN) {
        return TRUE;
    }
    return FALSE;
}

// Returns whether a pause/fade countdown remains for this channel.
BOOL mbMusFadeCheck(int chan)
{
    if (mbMusCheck(chan) == FALSE) {
        return FALSE;
    }
    if (musData[chan].fadeTime > 0) {
        return TRUE;
    }
    return FALSE;
}

// Returns whether this channel has no stream or is stopped/done.
BOOL mbMusEndCheck(int chan)
{
    if (mbMusCheck(chan) == FALSE) {
        return TRUE;
    }
    if (mbMusStatGet(chan) == MSM_STREAM_STOP || mbMusStatGet(chan) == MSM_STREAM_DONE) {
        return TRUE;
    }
    return FALSE;
}

// Starts a foreground jingle stream and returns its audio handle.
int mbMusJinglePlay(s16 id)
{
    int streamNo = HuAudSStreamChanPlay(id, MB_MUS_CHAN_FG * 2);

    return streamNo;
}

// Waits up to 600 process frames for a jingle, then fades it out if still active.
void mbMusJingleWait(int streamNo)
{
    int time;

    if (streamNo < 0) {
        return;
    }
    for (time = 0; time < 600; time++) {
        if (mbMusJingleStatGet(streamNo) == MSM_STREAM_DONE) {
            break;
        }
        HuPrcVSleep();
    }
    if (time >= 600) {
        HuAudSStreamFadeOut(streamNo, 10);
        HuPrcSleep(11);
    }
}

// Returns the audio engine state for a jingle handle; negative handles are done.
s32 mbMusJingleStatGet(int streamNo)
{
    if (streamNo < 0) {
        return MSM_STREAM_DONE;
    }
    return HuAudSStreamStatGet(streamNo);
}

// Monitors a music channel and clears it on board exit, stop-timeout expiry, or stream completion;
// completed streams are retained for up to three frames, while DVD errors clear immediately.
// mbMusPlay starts this child process per channel.
static void MusPlay(void)
{
    MBMUSDATA *musP;

    do {
        HUPROCESS *proc = HuPrcCurrentGet();
        int *chanP = proc->property;

        musP = &musData[*chanP];
        if (musP->fadeTime > 0) {
            musP->fadeTime--;
        }
        if (mbExitCheck()) {
            if (mbMusEndCheck(*chanP) == FALSE) {
                mbMusStop(*chanP, FALSE);
            }
            musP->streamNo = MSM_STREAMNO_NONE;
            musP->id = MSM_STREAM_NONE;
            musP->closeTime = 0;
            musP->stopTime = 0;
            musP->stopF = FALSE;
            musP->fadeTime = 0;
            musP->pauseF = FALSE;
            musP->proc = NULL;
            break;
        }
        if (musP->stopF) {
            if (--musP->stopTime < 0) {
                mbMusStop(*chanP, FALSE);
                musP->streamNo = MSM_STREAMNO_NONE;
                musP->id = MSM_STREAM_NONE;
                musP->closeTime = 0;
                musP->stopTime = 0;
                musP->stopF = FALSE;
                musP->fadeTime = 0;
                musP->pauseF = FALSE;
                musP->proc = NULL;
                break;
            }
        }
        if (musP->streamNo != MSM_STREAMNO_NONE) {
            switch (HuAudSStreamStatGet(musP->streamNo)) {
                case MSM_STREAM_DVDERROR:
                    mbMusStop(*chanP, FALSE);
                    musP->closeTime = 0;
                case MSM_STREAM_DONE:
                    if (--musP->closeTime <= 0) {
                        musP->streamNo = MSM_STREAMNO_NONE;
                        musP->id = MSM_STREAM_NONE;
                        musP->closeTime = 0;
                        musP->stopTime = 0;
                        musP->stopF = FALSE;
                        musP->fadeTime = 0;
                        musP->pauseF = FALSE;
                        musP->proc = NULL;
                    }
                    break;
            }
            HuPrcVSleep();
        }
        if (musP == NULL) {
            break;
        }
    } while (musP->streamNo != MSM_STREAMNO_NONE);

    if (musP) {
        musP->proc = NULL;
    }
    HuPrcEnd();
}

// Clears the music monitor process property when its destructor runs.
static void MusPlayKill(void)
{
    HUPROCESS *proc = HuPrcCurrentGet();

    if (proc->property) {
        proc->property = NULL;
    }
}

static inline void *MusBoardFadeWorkAlloc(void)
{
    return HuMemDirectMallocNum(HEAP_HEAP, sizeof(MUSBOARDFADEDATA), HU_MEMNUM_OVL);
}

// Starts a board music transition requested by board events; only one fade process can run at a
// time.
BOOL mbMusBoardFadeOut(int chan, int nextChan, int speed, int fadeSpeed, int musId, BOOL pauseF)
{
    MUSBOARDFADEDATA *work;

    if (musBoardFadeProc) {
        return FALSE;
    }
    musBoardFadeProc = HuPrcChildCreate(MusBoardFade, 0x2001, 0x2000, 0, mbMainProc);
    work = MusBoardFadeWorkAlloc();
    musBoardFadeProc->property = work;
    memset(work, 0, sizeof(MUSBOARDFADEDATA));
    work->chan = chan;
    work->nextChan = nextChan;
    work->speed = speed;
    work->fadeSpeed = fadeSpeed;
    work->musId = musId;
    work->pauseF = pauseF;

    if (work->pauseF) {
        mbMusPauseFadeOut(chan, TRUE, work->speed);
    } else {
        mbMusFadeOutSpeed(chan, (u16)work->speed);
    }
    return TRUE;
}

// Lets board events check whether the asynchronous board music transition is still active.
BOOL mbMusBoardFadeCheck(void)
{
    if (musBoardFadeProc == NULL) {
        return FALSE;
    }
    return TRUE;
}

// Waits for the outgoing stream fade, starts replacement or board music, then frees transition
// state.
static void MusBoardFade(void)
{
    MUSBOARDFADEDATA *work = HuPrcCurrentGet()->property;
    int waitTime;
    int time;

    waitTime = ((work->speed * 60) / 1000) + 10;
    for (time = 0; time < waitTime; time++) {
        HuPrcVSleep();
    }
    HuPrcSleep(5);

    if (work->musId == MSM_STREAM_NONE) {
        int i;
        int musId = boardMusData[0].dayMusId;

        for (i = 0; boardMusData[i].boardNo != 0xFFFFFFFF; i++) {
            if (boardMusData[i].boardNo == BoardNoGet()) {
                if (boardMusData[i].timeF) {
                    if (GwSystem.curTime == FALSE) {
                        musId = boardMusData[i].dayMusId;
                    } else {
                        musId = boardMusData[i].nightMusId;
                    }
                } else {
                    musId = boardMusData[i].dayMusId;
                }
                break;
            }
        }
        mbMusPlay(MB_MUS_CHAN_BG, musId, MSM_VOL_MAX, 0);
    } else {
        mbMusPlay(work->nextChan, work->musId, MSM_VOL_MAX, (u16)work->fadeSpeed);
    }
    HuMemDirectFree(work);
    musBoardFadeProc = NULL;
    HuPrcEnd();
}

// Pauses or resumes tracked board streams and engine stream 2 directly; these calls do not update
// the per-channel pause flags.
void mbMusPauseSet(BOOL pauseF)
{
    s32 pauseStatus;
    s32 unpauseStatus;

    if (pauseF) {
        if (musData[MB_MUS_CHAN_BG].streamNo != MSM_STREAMNO_NONE) {
            HuAudSStreamPauseFadeOut(musData[MB_MUS_CHAN_BG].streamNo, TRUE, 5);
        }
        if (musData[MB_MUS_CHAN_FG].streamNo != MSM_STREAMNO_NONE) {
            HuAudSStreamPauseFadeOut(musData[MB_MUS_CHAN_FG].streamNo, TRUE, 5);
        }
        pauseStatus = HuAudSStreamStatGet(2);
        if (pauseStatus == MSM_STREAM_PLAY) {
            HuAudSStreamPauseFadeOut(2, TRUE, 5);
        }
    } else {
        if (musData[MB_MUS_CHAN_BG].streamNo != MSM_STREAMNO_NONE &&
            musData[MB_MUS_CHAN_BG].pauseF == FALSE) {
            HuAudSStreamPauseFadeOut(musData[MB_MUS_CHAN_BG].streamNo, FALSE, 5);
        }
        // The foreground resume is gated by the background channel's pause flag as well.
        if (musData[MB_MUS_CHAN_FG].streamNo != MSM_STREAMNO_NONE &&
            musData[MB_MUS_CHAN_BG].pauseF == FALSE) {
            HuAudSStreamPauseFadeOut(musData[MB_MUS_CHAN_FG].streamNo, FALSE, 5);
        }
        unpauseStatus = HuAudSStreamStatGet(2);
        if (unpauseStatus == MSM_STREAM_PAUSEIN) {
            HuAudSStreamPauseFadeOut(2, FALSE, 5);
        }
    }
}

void mbAudStub1(void)
{
}

void mbAudStub2(void)
{
}

int mbAudFXObjSet(int seId)
{
    return mbAudFXObjCreate(seId, TRUE);
}

// Tracks a board sound effect for later volume, pause, position, or cleanup requests.
int mbAudFXObjCreate(int seId, BOOL multiF)
{
    MBAUDFXOBJ *audFx = &audFXObjData[0];
    int i;

    if (multiF == FALSE) {
        for (i = 0, audFx = &audFXObjData[0]; i < MB_AUD_FX_OBJ_MAX; i++, audFx++) {
            if (audFx->seId == seId) {
                return MSM_SENO_NONE;
            }
        }
    }
    for (i = 0, audFx = &audFXObjData[0]; i < MB_AUD_FX_OBJ_MAX; i++, audFx++) {
        if (audFx->seId == MSM_SE_NONE) {
            break;
        }
    }
    if (i >= MB_AUD_FX_OBJ_MAX) {
        return MSM_SENO_NONE;
    }
    if (audFXObjOnF) {
        audFx->seNo = MB_AUD_FXNO_OFF;
        audFx->seId = seId;
        audFx->vol = MSM_VOL_MAX;
        audFx->pauseF = FALSE;
        audFx->emitterF = FALSE;
        audFx->lockF = TRUE;
        audFx->fxRef = NULL;
        return MB_AUD_FXNO_OFF;
    }

    audFx->seNo = HuAudFXPlay(seId);
    if (audFx->seNo < 0) {
        audFx->seNo = audFx->seId = MSM_SE_NONE;
        return MSM_SENO_NONE;
    }
    audFx->seId = seId;
    audFx->vol = MSM_VOL_MAX;
    audFx->pauseF = FALSE;
    audFx->emitterF = FALSE;
    audFx->lockF = FALSE;
    audFx->fxRef = NULL;
    return audFx->seNo;
}

// Changes the requested volume of a tracked board sound effect.
void mbAudFXObjVolSet(int seNo, s16 vol)
{
    MBAUDFXOBJ *audFx = &audFXObjData[0];
    int i;

    for (i = 0, audFx = &audFXObjData[0]; i < MB_AUD_FX_OBJ_MAX; i++, audFx++) {
        if (audFx->seNo == seNo) {
            break;
        }
    }
    if (i >= MB_AUD_FX_OBJ_MAX) {
        return;
    }
    audFx->vol = vol;
    HuAudFXVolSet(audFx->seNo, audFx->vol);
}

// Releases a tracked sound-effect slot and fades its playback out over one second.
void mbAudFXObjKill(int seNo)
{
    MBAUDFXOBJ *audFx = &audFXObjData[0];
    int i;

    for (i = 0, audFx = &audFXObjData[0]; i < MB_AUD_FX_OBJ_MAX; i++, audFx++) {
        if (audFx->seNo == seNo) {
            break;
        }
    }
    if (i >= MB_AUD_FX_OBJ_MAX) {
        return;
    }
    audFx->seId = MSM_SE_NONE;
    audFx->seNo = MSM_SENO_NONE;
    HuAudFXFadeOut(seNo, 1000);
}

// Fades out every currently tracked board sound effect during audio shutdown.
void mbAudFXObjClose(void)
{
    MBAUDFXOBJ *audFx = &audFXObjData[0];
    int i;

    for (i = 0, audFx = &audFXObjData[0]; i < MB_AUD_FX_OBJ_MAX; i++, audFx++) {
        if (audFx->seNo != MSM_SENO_NONE) {
            mbAudFXObjKill(audFx->seNo);
        }
    }
}

int mbAudFXObjEmitterSet(int seId, Vec *pos)
{
    return mbAudFXObjEmitterCreate(seId, pos, TRUE);
}

// Tracks a sound effect attached to a world position for later position updates or cleanup.
int mbAudFXObjEmitterCreate(int seId, Vec *pos, BOOL multiF)
{
    MBAUDFXOBJ *audFx = &audFXObjData[0];
    int i;

    if (multiF == FALSE) {
        for (i = 0, audFx = &audFXObjData[0]; i < MB_AUD_FX_OBJ_MAX; i++, audFx++) {
            if (audFx->seId == seId) {
                return MSM_SENO_NONE;
            }
        }
    }
    for (i = 0, audFx = &audFXObjData[0]; i < MB_AUD_FX_OBJ_MAX; i++, audFx++) {
        if (audFx->seId == MSM_SE_NONE) {
            break;
        }
    }
    if (i >= MB_AUD_FX_OBJ_MAX) {
        return MSM_SENO_NONE;
    }
    if (audFXObjOnF) {
        audFx->seNo = MB_AUD_FXNO_OFF;
        audFx->seId = seId;
        audFx->vol = MSM_VOL_MAX;
        audFx->pauseF = FALSE;
        audFx->emitterF = TRUE;
        audFx->pos = *pos;
        audFx->lockF = TRUE;
        audFx->fxRef = NULL;
        return MB_AUD_FXNO_OFF;
    }

    audFx->seNo = HuAudFXEmiterPlay(seId, pos);
    if (audFx->seNo < 0) {
        audFx->seNo = MSM_SENO_NONE;
        return MSM_SENO_NONE;
    }
    audFx->seId = seId;
    audFx->vol = MSM_VOL_MAX;
    audFx->pauseF = FALSE;
    audFx->emitterF = TRUE;
    audFx->pos = *pos;
    audFx->lockF = FALSE;
    audFx->fxRef = NULL;
    return audFx->seNo;
}

// Stores the emitter position and updates live playback unless star-reset audio is holding effects.
void mbAudFXObjEmitterUpdate(int seNo, Vec *pos)
{
    MBAUDFXOBJ *audFx = &audFXObjData[0];
    int i;

    for (i = 0, audFx = &audFXObjData[0]; i < MB_AUD_FX_OBJ_MAX; i++, audFx++) {
        if (audFx->seNo == seNo && audFx->emitterF) {
            break;
        }
    }
    if (i >= MB_AUD_FX_OBJ_MAX) {
        return;
    }
    audFx->pos = *pos;
    if (audFXObjOnF == FALSE) {
        HuAudFXEmiterUpDate(audFx->seNo, &audFx->pos);
    }
}

// Stores a new position for a locked emitter that is waiting for deferred playback.
void mbAudFXObjEmiterPosSet(int seId, Vec *pos)
{
    MBAUDFXOBJ *audFx = &audFXObjData[0];
    int i;

    for (i = 0, audFx = &audFXObjData[0]; i < MB_AUD_FX_OBJ_MAX; i++, audFx++) {
        if (audFx->seId == seId && audFx->emitterF && audFx->lockF) {
            break;
        }
    }
    if (i >= MB_AUD_FX_OBJ_MAX) {
        return;
    }
    audFx->pos = *pos;
}

// Registers a caller-owned handle slot to fill when a locked sound effect begins playback.
void mbAudFXObjRefSet(int seId, int *fxRef)
{
    MBAUDFXOBJ *audFx = &audFXObjData[0];
    int i;

    for (i = 0, audFx = &audFXObjData[0]; i < MB_AUD_FX_OBJ_MAX; i++, audFx++) {
        if (audFx->seId == seId && audFx->lockF) {
            break;
        }
    }
    if (i >= MB_AUD_FX_OBJ_MAX) {
        return;
    }
    audFx->fxRef = fxRef;
}

// Board child process: starts queued sounds, retires finished handles, tracks star-reset audio
// state, and updates each player's voice pan every frame.
static void AudFXMain(void)
{
    MBAUDFXOBJ *audFx = &audFXObjData[0];
    MBAUDFXDATA *delayFx = &audFXData[0];
    int i;

    do {
        delayFx = &audFXData[0];
        for (i = 0; i < MB_AUD_FX_DELAY_MAX; i++, delayFx++) {
            if (delayFx->seId >= 0 && --delayFx->delay <= 0) {
                switch (delayFx->type) {
                    case MB_AUD_FX_TYPE_NORMAL:
                        HuAudFXPlay(delayFx->seId);
                        break;
                    case MB_AUD_FX_TYPE_PAN:
                        HuAudFXPlayPan(delayFx->seId, delayFx->pan);
                        break;
                    case MB_AUD_FX_TYPE_EMITTER:
                        HuAudFXEmiterPlay(delayFx->seId, &delayFx->pos);
                        break;
                }
                delayFx->seId = MSM_SE_NONE;
                delayFx->delay = -1;
                delayFx->type = MB_AUD_FX_TYPE_NORMAL;
            }
        }

        for (i = 0, audFx = &audFXObjData[0]; i < MB_AUD_FX_OBJ_MAX; i++, audFx++) {
            if (audFx->seId != MSM_SE_NONE && audFx->seNo != MSM_SENO_NONE && audFx->lockF == FALSE
                && HuAudFXStatusGet(audFx->seNo) == MSM_SE_DONE) {
                audFx->seId = MSM_SE_NONE;
                audFx->seNo = MSM_SENO_NONE;
            }
        }

        if (_CheckFlag(FLAG_BOARD_STAR_RESET) == FALSE) {
            if (audFXObjOnF) {
                if (--audFXObjOnTimer <= 0) {
                    for (i = 0, audFx = &audFXObjData[0]; i < MB_AUD_FX_OBJ_MAX; i++, audFx++) {
                        if (audFx->lockF && audFx->seNo == MB_AUD_FXNO_OFF) {
                            if (audFx->emitterF) {
                                audFx->seNo = HuAudFXEmiterPlay(audFx->seId, &audFx->pos);
                                if (audFx->seNo < 0) {
                                    audFx->seNo = MSM_SENO_NONE;
                                    audFx->seId = MSM_SE_NONE;
                                }
                            } else {
                                audFx->seNo = HuAudFXPlay(audFx->seId);
                                if (audFx->seNo < 0) {
                                    audFx->seNo = MSM_SENO_NONE;
                                    audFx->seId = MSM_SE_NONE;
                                }
                            }
                            if (audFx->fxRef != NULL) {
                                *audFx->fxRef = audFx->seNo;
                            }
                            audFx->lockF = FALSE;
                        }
                        if (audFx->seId != MSM_SE_NONE && audFx->seNo != MSM_SENO_NONE) {
                            AudFXObjPauseSet(audFx->seNo, FALSE);
                        }
                    }
                }
            }
            if (audFXObjOnTimer <= 0) {
                audFXObjOnF = FALSE;
            }
        } else {
            if (audFXObjOnF == FALSE) {
                for (i = 0, audFx = &audFXObjData[0]; i < MB_AUD_FX_OBJ_MAX; i++, audFx++) {
                    if (audFx->seId != MSM_SE_NONE && audFx->seNo != MSM_SENO_NONE) {
                        AudFXObjPauseSet(audFx->seNo, TRUE);
                    }
                }
            }
            audFXObjOnF = TRUE;
            audFXObjOnTimer = 1;
        }

        for (i = 0; i < GW_PLAYER_MAX; i++) {
            Vec pos;
            int pan;

            mbPlayerPosGet(i, &pos);
            pan = mbAudFXPosPanGet(&pos);
            CharModelVoicePanSet(GwPlayer[i].charNo, pan);
        }
        HuPrcVSleep();
    } while (mbExitCheck() == FALSE);

    HuPrcEnd();
}

// Closes tracked board sound effects and clears the audio worker process pointer.
static void AudFXMainDestroy(void)
{
    mbAudFXObjClose();
    audFXProc = NULL;
}

// Plays a board sound effect now or queues it for the configured delay; a full delayed queue drops
// the request, and the delay setting is consumed either way. Returns the immediate handle (or
// MSM_SENO_NONE).
int mbAudFXPlay(s16 seId)
{
    int seNo = MSM_SENO_NONE;

    if (audFXDelay > 0) {
        int delay = audFXDelay;
        MBAUDFXDATA *audFx = &audFXData[0];
        int i;

        for (i = 0; i < MB_AUD_FX_DELAY_MAX; i++, audFx++) {
            if (audFx->seId < 0) {
                break;
            }
        }
        if (i < MB_AUD_FX_DELAY_MAX) {
            audFx->seId = seId;
            audFx->delay = delay;
            audFx->type = MB_AUD_FX_TYPE_NORMAL;
            audFx->pan = 0;
            AudFXPosSet(audFx, NULL);
        }
    } else {
        seNo = HuAudFXPlay(seId);
    }
    audFXDelay = 0;
    return seNo;
}

void mbAudFXStop(int seNo)
{
    HuAudFXStop(seNo);
}

// Stops all engine sound effects, using a 1000 ms fade when speed <= 0, and clears the board's
// tracked and delayed sound-effect entries.
void mbAudFXStopAll(int speed)
{
    MBAUDFXOBJ *audFx = &audFXObjData[0];
    MBAUDFXDATA *delayFx = &audFXData[0];
    int i;

    if (speed <= 0) {
        speed = 1000;
    }
    msmSeStopAll(FALSE, speed);

    memset(&audFXObjData[0], 0, sizeof(audFXObjData));
    for (i = 0, audFx = &audFXObjData[0]; i < MB_AUD_FX_OBJ_MAX; i++, audFx++) {
        audFx->seId = MSM_SE_NONE;
        audFx->seNo = MSM_SENO_NONE;
    }

    memset(&audFXData[0], 0, sizeof(audFXData));
    for (i = 0; i < MB_AUD_FX_DELAY_MAX; i++, delayFx++) {
        delayFx->seId = MSM_SE_NONE;
    }
}

// Plays a sound with pan derived from its world position, or queues that request for delayed
// playback; a full delayed queue drops it and still consumes the delay setting.
int mbAudFXPosPlay(s16 seId, Vec *pos)
{
    int pan;
    int seNo = MSM_SENO_NONE;

    pan = mbAudFXPosPanGet(pos);

    if (audFXDelay > 0) {
        int delay = audFXDelay;
        MBAUDFXDATA *audFx = &audFXData[0];
        int i;

        for (i = 0; i < MB_AUD_FX_DELAY_MAX; i++, audFx++) {
            if (audFx->seId < 0) {
                break;
            }
        }
        if (i < MB_AUD_FX_DELAY_MAX) {
            audFx->seId = seId;
            audFx->delay = delay;
            audFx->type = MB_AUD_FX_TYPE_PAN;
            audFx->pan = pan;
            AudFXPosSet(audFx, NULL);
        }
    } else {
        seNo = HuAudFXPlayPan(seId, pan);
    }
    audFXDelay = 0;
    return seNo;
}

// Projects a world position to the screen and returns its clamped left-to-right sound pan.
u8 mbAudFXPosPanGet(Vec *pos)
{
    Vec pos2D;
    int pan;

    Hu3D3Dto2D(pos, HU3D_CAM0, &pos2D);
    pan = (int)((pos2D.x * (1.0f / HU_DISP_WIDTH)) * MSM_PAN_WIDTH) + MSM_PAN_LEFT;
    if (pan < MSM_PAN_LEFT) {
        pan = MSM_PAN_LEFT;
    } else if (pan > MSM_PAN_RIGHT) {
        pan = MSM_PAN_RIGHT;
    }
    return pan;
}

// Plays a sound from a world-position emitter, or stores the request for delayed playback; a full
// delayed queue drops it and still consumes the delay setting.
int mbAudFXEmitterPlay(int seId, Vec *pos)
{
    int seNo = MSM_SENO_NONE;

    if (audFXDelay > 0) {
        int delay = audFXDelay;
        MBAUDFXDATA *audFx = &audFXData[0];
        int i;

        for (i = 0; i < MB_AUD_FX_DELAY_MAX; i++, audFx++) {
            if (audFx->seId < 0) {
                break;
            }
        }
        if (i < MB_AUD_FX_DELAY_MAX) {
            audFx->seId = (s16)seId;
            audFx->delay = delay;
            audFx->type = MB_AUD_FX_TYPE_EMITTER;
            audFx->pan = 0;
            AudFXPosSet(audFx, pos);
        }
    } else {
        seNo = HuAudFXEmiterPlay(seId, pos);
    }
    audFXDelay = 0;
    return seNo;
}

void mbAudFXVolSet(int seNo, s16 vol)
{
    HuAudFXVolSet(seNo, vol);
}

void mbAudFXPanning(int seNo, s16 pan)
{
    HuAudFXPanning(seNo, pan);
}

// Recalculates a playing sound's pan from its current world position.
void mbAudFXPosPanning(int seNo, Vec *pos)
{
    s16 pan = mbAudFXPosPanGet(pos);

    mbAudFXPanning(seNo, pan);
}

void mbAudFXDelaySet(int delay)
{
    audFXDelay = delay;
}

// Selects the time-appropriate guide voice for a paired cue and plays or delays the selected sound;
// a full delayed queue drops the request and still consumes the delay setting.
int mbAudGuidePlay(s16 seId)
{
    int delay1;
    int delay2;
    BOOL partyF;
    int timeNo;
    int i;

    if (GwSystem.curTime) {
        partyF = GwSystem.partyF;

        if (partyF != FALSE) {
            goto time_one;
        }
    }
    timeNo = 0;
    goto time_set;
time_one:
    timeNo = 1;
time_set:

    for (i = 0; guideFxTbl[i][0] != -1; i++) {
        if (seId == guideFxTbl[i][0] || seId == guideFxTbl[i][1]) {
            break;
        }
    }
    if (guideFxTbl[i][timeNo] != -1) {
        int seNo;
        s16 guideSeId = guideFxTbl[i][timeNo];

        seNo = MSM_SENO_NONE;

        if (audFXDelay > 0) {
            MBAUDFXDATA *audFx;
            int j;

            delay1 = audFXDelay;
            audFx = &audFXData[0];
            for (j = 0; j < MB_AUD_FX_DELAY_MAX; j++, audFx++) {
                if (audFx->seId < 0) {
                    break;
                }
            }
            if (j < MB_AUD_FX_DELAY_MAX) {
                audFx->seId = guideSeId;
                audFx->delay = delay1;
                audFx->type = MB_AUD_FX_TYPE_NORMAL;
                audFx->pan = 0;
                AudFXPosSet(audFx, NULL);
            }
        } else {
            seNo = HuAudFXPlay(guideSeId);
        }
        audFXDelay = 0;
        return seNo;
    } else {
        int seNo = MSM_SENO_NONE;

        if (audFXDelay > 0) {
            MBAUDFXDATA *audFx;
            int j;

            delay2 = audFXDelay;
            audFx = &audFXData[0];
            for (j = 0; j < MB_AUD_FX_DELAY_MAX; j++, audFx++) {
                if (audFx->seId < 0) {
                    break;
                }
            }
            if (j < MB_AUD_FX_DELAY_MAX) {
                audFx->seId = seId;
                audFx->delay = delay2;
                audFx->type = MB_AUD_FX_TYPE_NORMAL;
                audFx->pan = 0;
                AudFXPosSet(audFx, NULL);
            }
        } else {
            seNo = HuAudFXPlay(seId);
        }
        audFXDelay = 0;
        return seNo;
    }
}
