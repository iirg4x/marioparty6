// Initializes and controls the game's music, sound effects, and streamed audio.
#define _MATH_H
#include "game/audio.h"
#include "game/memory.h"
#include "game/armem.h"
#include "game/wipe.h"
#include "game/object.h"
#include "game/process.h"
#include "game/memory.h"
#include "game/gamework.h"

#define HUMSMHEAP_SIZE 0xBFC00

#define HUAUD_STREAM_MAX 16

#define MSM_FILE_PATH "/sound/MP6_SND.msm"
#define PDT_FILE_PATH "/sound/MP6_Str.pdt"

static int HuSePlay(int seId, MSM_SEPARAM *param);
static BOOL HuSeExitCheck(void);

static BOOL CharSeLoadF[GW_CHARA_MAX];
static s32 sndFxBuf[64][2];

static s16 Hu3DAudVol;
static s16 sndGroupBak;
static int auxANoBak;
static int auxBNoBak;
static s8 HuAuxAVol;
static s8 HuAuxBVol;
float Snd3DBackSurDisOffset;
float Snd3DFrontSurDisOffset;
float Snd3DStartDisOffset;
float Snd3DSpeedOffset;
float Snd3DDistOffset;
BOOL musicOffF;
u8 fadeStat;

// Initializes the sound manager, selects mono or stereo output, resets audio defaults, and loops
// forever if initialization fails.
void HuAudInit(void)
{
    MSM_INIT msmConfig;
    MSM_ARAM aramConfig;
    s32 initResult;
    s16 index;
    msmConfig.heap = HuMemDirectMalloc(HEAP_SOUND, HUMSMHEAP_SIZE);
    msmConfig.heapSize = HUMSMHEAP_SIZE;
    msmConfig.msmPath = MSM_FILE_PATH;
    msmConfig.pdtPath = PDT_FILE_PATH;
    msmConfig.open = NULL;
    msmConfig.read = NULL;
    msmConfig.close = NULL;
    aramConfig.skipARInit = TRUE;
    aramConfig.aramEnd = HU_AMEM_BASE;
    initResult = msmSysInit(&msmConfig, &aramConfig);
    if(initResult < 0) {
        OSReport("MSM(Sound Manager) Error:Error Code %d\n", initResult);
        while(1);
    }
    if(OSGetSoundMode() == OS_SOUND_MODE_MONO) {
        msmSysSetOutputMode(SND_OUTPUTMODE_MONO);
    } else {
        msmSysSetOutputMode(SND_OUTPUTMODE_STEREO);
    }
    for(index=0; index<64; index++) {
        sndFxBuf[index][0] = -1;
    }
    for(index=0; index<GW_CHARA_MAX; index++) {
        CharSeLoadF[index] = FALSE;
    }
    sndGroupBak = MSM_GRP_NONE;
    auxANoBak = MSM_AUX_DEFAULT;
    auxBNoBak = MSM_AUX_DEFAULT;
    HuAuxAVol = HuAuxBVol = -1;
    fadeStat = FALSE;
    musicOffF = FALSE;
}

// The streaming-audio entry point is currently a stub and always returns zero.
s32 HuAudStreamPlay(char *name, BOOL flag)
{
    return 0;
}

// Sets both AI stream channel volumes and stores the requested value in Hu3DAudVol.
void HuAudStreamVolSet(s16 vol)
{
    AISetStreamVolLeft(vol);
    AISetStreamVolRight(vol);
    Hu3DAudVol = vol;
}

// Stops the AI stream while game audio is paused.
void HuAudStreamPauseOn(void)
{
    AISetStreamPlayState(AI_STREAM_STOP);
}

// Starts the AI stream again after a pause.
void HuAudStreamPauseOff(void)
{
    AISetStreamPlayState(AI_STREAM_START);
}

// Reserved entry point; it currently does not alter the AI stream.
void HuAudStreamFadeOut(s32 streamNo)
{
    
}

// Stops sequence music, sound effects, and streamed music together.
void HuAudAllStop(void)
{
    HuAudSeqAllStop();
    HuAudFXAllStop();
    HuAudSStreamAllStop();
}

// Stops sound effects immediately and fades sequence and streamed music at this rate.
void HuAudFadeOut(s32 speed)
{
    HuAudFXAllStop();
    HuAudSeqAllFadeOut(speed);
    HuAudSStreamAllFadeOut(speed);
}

// Converts the input to float; the converted local is not used afterward.
static void dummyfloat(unsigned int inputValue)
{
    // This value is not read after conversion.
    float convertedValue = inputValue;
}

// Plays a sound effect at default volume and pan unless shutdown or a late wipe blocks it.
int HuAudFXPlay(int seId)
{

    if(HuSeExitCheck()) {
        return 0;
    }
    return HuAudFXPlayVolPan(seId, MSM_VOL_MAX, MSM_PAN_CENTER);
}

// Plays a sound effect at the requested volume and centered pan unless shutdown is pending.
int HuAudFXPlayVol(int seId, s16 vol)
{
    if(omSysExitReq) {
        return 0;
    }
    return HuAudFXPlayVolPan(seId, vol, MSM_PAN_CENTER);
}

// Plays a sound effect at the requested pan and default volume unless shutdown is pending.
int HuAudFXPlayPan(int seId, s16 pan)
{
    MSM_SEPARAM seParam;
    if(omSysExitReq) {
        return 0;
    }
    seParam.flag = MSM_SEPARAM_PAN;
    seParam.pan = pan;
    return HuSePlay(seId, &seParam);
}

// Plays a sound effect with explicit volume and pan unless shutdown is pending.
int HuAudFXPlayVolPan(int seId, s16 vol, s16 pan)
{
    MSM_SEPARAM seParam;
    if(omSysExitReq) {
        return 0;
    }
    seParam.flag = MSM_SEPARAM_VOL|MSM_SEPARAM_PAN;
    seParam.vol = vol;
    seParam.pan = pan;
    return HuSePlay(seId, &seParam);
}

static void AudFXPlayDelay(void);

typedef struct HuAudFXDelay_s {
    int seId; // Sound effect ID to play after the wait.
    MSM_SEPARAM param; // Volume, pan, and other options for the sound effect.
    u16 delay; // Number of process frames to wait before playing.
} HUAUD_FX_DELAY;

// Schedules a default-volume, centered sound effect after the requested frame delay.
void HuAudFXPlayDelay(int seId, u16 delay)
{
    HuAudFXPlayDelayVolPan(seId, MSM_VOL_MAX, MSM_PAN_CENTER, delay);
}

// Schedules a centered sound effect at the requested volume after the frame delay.
void HuAudFXPlayDelayVol(int seId, u8 vol, u16 delay)
{
    HuAudFXPlayDelayVolPan(seId, vol, MSM_PAN_CENTER, delay);
}

// Schedules a default-volume sound effect at the requested pan after the frame delay.
void HuAudFXPlayDelayPan(int seId, u8 pan, u16 delay)
{
    HuAudFXPlayDelayVolPan(seId, MSM_VOL_MAX, pan, delay);
}

// Returns if shutdown is pending; otherwise plays immediately at zero delay or schedules playback
// after the requested frames.
void HuAudFXPlayDelayVolPan(int seId, u8 vol, u8 pan, u16 delay)
{
    if(omSysExitReq) {
        return;
    }
    if(delay == 0) {
        HuAudFXPlayVolPan(seId, vol, pan);
        return;
    } else {
        HUPROCESS *delayProcess = HuPrcCreate(AudFXPlayDelay, 1, 2304, 0);
        HUAUD_FX_DELAY *delayWork = HuMemDirectMalloc(HEAP_HEAP, sizeof(HUAUD_FX_DELAY));
        delayProcess->property = delayWork;
        delayWork->param.flag = MSM_SEPARAM_VOL|MSM_SEPARAM_PAN;
        delayWork->param.vol = vol;
        delayWork->param.pan = pan;
        delayWork->seId = seId;
        delayWork->delay = delay;
    }
}

// Sound-effect delay process callback; waits one frame at a time and skips playback on exit.
static void AudFXPlayDelay(void)
{
    HUPROCESS *process = HuPrcCurrentGet();
    HUAUD_FX_DELAY *delayWork = process->property;
    int waitedFrames;
    for(waitedFrames=0; waitedFrames<delayWork->delay; waitedFrames++) {
        if(omSysExitReq) {
            break;
        }
        HuPrcVSleep();
    }
    if(waitedFrames == delayWork->delay) {
        HuSePlay(delayWork->seId, &delayWork->param);
    }
    HuMemDirectFree(delayWork);
    HuPrcEnd();
    while(1) {
        HuPrcVSleep();
    }
}

// Stops one active sound-effect instance.
void HuAudFXStop(int seNo)
{
    msmSeStop(seNo, 0);
}

// Stops every active sound effect immediately.
void HuAudFXAllStop(void)
{
    msmSeStopAll(FALSE, 0);
}

// Stops one sound-effect instance using the sound manager's requested fade rate.
void HuAudFXFadeOut(int seNo, s32 speed)
{
    msmSeStop(seNo, speed);
}

// Changes one active sound effect's pan unless shutdown is pending.
void HuAudFXPanning(int seNo, s16 pan)
{
    MSM_SEPARAM param;
    if(omSysExitReq) {
        return;
    }
    param.flag = MSM_SEPARAM_PAN;
    param.pan = pan;
    msmSeSetParam(seNo, &param);
}

// Updates the 3D sound listener with global distance and speed offsets applied.
void HuAudFXListnerSet(Vec *pos, Vec *heading, float sndDist, float sndSpeed)
{
    if(omSysExitReq) {
      return;
    }
    // The extended setter adds distance, speed, start, and front/back offsets again after these
    // values are formed.
    HuAudFXListnerSetEX(pos, heading,
        sndDist + Snd3DDistOffset,
        sndSpeed + Snd3DSpeedOffset,
        Snd3DStartDisOffset,
        Snd3DFrontSurDisOffset + (0.25 * sndDist + Snd3DStartDisOffset),
        Snd3DBackSurDisOffset + (0.25 * sndDist + Snd3DStartDisOffset));
}

// Adds configured global distance, speed, and surround offsets before submitting the listener, then
// logs input and adjusted values.
void HuAudFXListnerSetEX(Vec *pos, Vec *heading, float sndDist, float sndSpeed, float startDis,
                         float frontSurDis, float backSurDis)
{
    MSM_SELISTENER listener;
    if(omSysExitReq) {
      return;
    }
    listener.flag = MSM_LISTENER_STARTDIS|MSM_LISTENER_FRONTSURDIS|MSM_LISTENER_BACKSURDIS;
    listener.startDis = startDis + Snd3DStartDisOffset;
    listener.frontSurDis = frontSurDis + Snd3DFrontSurDisOffset;
    listener.backSurDis = backSurDis + Snd3DBackSurDisOffset;
    msmSeSetListener(pos, heading, sndDist + Snd3DDistOffset, sndSpeed + Snd3DSpeedOffset,
                     &listener);
    OSReport("//////////////////////////////////\n");
    OSReport("sndDist %f\n", sndDist);
    OSReport("sndSpeed %f\n", sndSpeed);
    OSReport("startDis %f\n", listener.startDis);
    OSReport("frontSurDis %f\n", listener.frontSurDis);
    OSReport("backSurDis %f\n", listener.backSurDis);
    OSReport("//////////////////////////////////\n");
}

// Updates the current 3D listener's position and heading.
void HuAudFXListnerUpdate(Vec *pos, Vec *heading)
{
    if(omSysExitReq) {
      return;
    }
    msmSeUpdataListener(pos, heading);
}

// Starts a sound effect at a world position unless shutdown or a late wipe blocks it.
int HuAudFXEmiterPlay(int seId, Vec *pos)
{
    MSM_SEPARAM seParam;
    if(HuSeExitCheck()) {
      return 0;
    }
    seParam.flag = MSM_SEPARAM_POS;
    seParam.pos.x = pos->x;
    seParam.pos.y = pos->y;
    seParam.pos.z = pos->z;
    return HuSePlay(seId, &seParam);
}

// Moves an active positional sound effect to the supplied world position.
void HuAudFXEmiterUpDate(int seNo, Vec *pos)
{
    MSM_SEPARAM param;
    if(omSysExitReq) {
        return;
    }
    param.flag = MSM_SEPARAM_POS;
    param.pos.x = pos->x;
    param.pos.y = pos->y;
    param.pos.z = pos->z;
    msmSeSetParam(seNo, &param);
}

// Removes the 3D sound listener.
void HuAudFXListnerKill(void)
{
    msmSeDelListener();
}

// Pauses or resumes every sound effect with the sound manager's transition time.
void HuAudFXPauseAll(BOOL pauseF)
{
    msmSePauseAll(pauseF, 100);
}

s32 HuAudFXStatusGet(int seNo)
{
    return msmSeGetStatus(seNo);
}

// Changes one active sound effect's pitch unless shutdown is pending.
s32 HuAudFXPitchSet(int seNo, s16 pitch)
{
    MSM_SEPARAM param;

    if(omSysExitReq) {
        return 0;
    }
    param.flag = MSM_SEPARAM_PITCH;
    param.pitch = pitch;
    return msmSeSetParam(seNo, &param);
}

// Changes one active sound effect's volume unless shutdown is pending.
s32 HuAudFXVolSet(int seNo, s16 vol)
{
    MSM_SEPARAM param;

    if(omSysExitReq) {
        return 0;
    }
    param.flag = MSM_SEPARAM_VOL;
    param.vol = vol;
    return msmSeSetParam(seNo, &param);
}

// Starts a sequence track unless music has been disabled or shutdown is pending.
int HuAudSeqPlay(s16 musId)
{
    int musNo;
    if(musicOffF || omSysExitReq) {
        return 0;
    }
    musNo = msmMusPlay(musId, NULL);
    return musNo;
}

// Stops one sequence track immediately unless music has been disabled or shutdown is pending.
void HuAudSeqStop(int musNo)
{
    if(musicOffF || omSysExitReq) {
        return;
    }
    msmMusStop(musNo, 0);
}

// Fades one sequence track unless music has been disabled.
void HuAudSeqFadeOut(int musNo, s32 speed)
{
    if(musicOffF) {
        return;
    }
    msmMusStop(musNo, speed);
}

// Applies the requested fade to each currently playing sequence track.
void HuAudSeqAllFadeOut(s32 speed)
{
    s16 musicIndex;
    for(musicIndex=0; musicIndex<MSM_MUS_MAX; musicIndex++) {
        if(msmMusGetStatus(musicIndex) == MSM_MUS_PLAY) {
            msmMusStop(musicIndex, speed);
        }
    }
}

// Stops all sequence tracks immediately.
void HuAudSeqAllStop(void)
{
    msmMusStopAll(FALSE, 0);
}

// Pauses or resumes all sequence tracks with the sound manager's transition time.
void HuAudSeqPauseAll(BOOL pause)
{
    msmMusPauseAll(pause, 100);
}

// Reads one MIDI controller value unless music is disabled or shutdown is pending.
s32 HuAudSeqMidiCtrlGet(int musNo, s8 channel, s8 ctrl)
{
    if(musicOffF || omSysExitReq) {
        return 0;
    }
    return msmMusGetMidiCtrl(musNo, channel, ctrl);
}

static void SStreamPlay(void);

typedef struct SStreamWork_s {
    int channelNo; // Stream channel that must finish stopping before playback.
    int streamId; // Stream asset to start on that channel.
} SSTREAM_WORK;

static u8 streamVol[HUAUD_STREAM_MAX];

// Starts a stream on a channel, scheduling a process when that channel is still active.
int HuAudSStreamChanPlay(s16 streamId, s16 channelNo)
{
    if(musicOffF || omSysExitReq) {
        return MSM_STREAMNO_NONE;
    }
    if(msmStreamGetStatus(channelNo) != MSM_STREAM_DONE) {
        HUPROCESS *process;
        SSTREAM_WORK *streamWork;
        msmStreamStop(channelNo, 0);
        process = HuPrcCreate(SStreamPlay, 1, 2304, 0);
        process->property = streamWork = HuMemDirectMalloc(HEAP_HEAP, sizeof(SSTREAM_WORK));
        streamWork->channelNo = channelNo;
        streamWork->streamId = streamId;
        return channelNo;
    } else {
        MSM_STREAMPARAM param;
        int streamNo;
        param.flag = MSM_STREAMPARAM_CHAN;
        param.chan = channelNo;
        streamNo = msmStreamPlay(streamId, &param);
        streamVol[channelNo] = MSM_VOL_MAX;
        return streamNo;
    }
}

// Stream-start process callback; waits up to one second for the channel to stop, then starts it.
static void SStreamPlay(void)
{
    HUPROCESS *process = HuPrcCurrentGet();
    SSTREAM_WORK *streamWork = process->property;
    OSTick startTick = OSGetTick();
    MSM_STREAMPARAM param;
    while (msmStreamGetStatus(streamWork->channelNo) != MSM_STREAM_DONE &&
           OSTicksToMilliseconds(OSGetTick() - startTick) < 1000) {
        if(OSTicksToMilliseconds(OSGetTick()-startTick) > 800) {
             // Force a stop if the channel has not finished releasing by this point.
             msmStreamStop(streamWork->channelNo, 0);
        }
        HuPrcVSleep();
    }
    param.flag = MSM_STREAMPARAM_CHAN;
    param.chan = streamWork->channelNo;
    msmStreamPlay(streamWork->streamId, &param);
    streamVol[streamWork->channelNo] = MSM_VOL_MAX;
    HuMemDirectFree(streamWork);
    HuPrcEnd();
    while(1) {
        HuPrcVSleep();
    }
}

// Plays a stream on the default music channel.
int HuAudSStreamPlay(s16 streamId)
{
    return HuAudSStreamChanPlay(streamId, 0);
}

// Plays background music on the default music channel.
int HuAudBGMPlay(s16 streamId)
{
    return HuAudSStreamChanPlay(streamId, 0);
}

// Plays a jingle on channel two.
int HuAudJinglePlay(s16 streamId)
{
    return HuAudSStreamChanPlay(streamId, 2);
}

// Stops one streamed track immediately unless music is disabled.
void HuAudSStreamStop(int streamNo)
{
    if(musicOffF) {
        return;
    }
    msmStreamStop(streamNo, 0);
}

// Fades one streamed track unless music is disabled.
void HuAudSStreamFadeOut(int streamNo, s32 speed)
{
    if(musicOffF) {
        return;
    }
    msmStreamStop(streamNo, speed);
}

// Applies the requested fade to every streamed track.
void HuAudSStreamAllFadeOut(s32 speed)
{
    msmStreamStopAll(speed);
}

// Pauses or resumes one streamed track with a five millisecond transition.
void HuAudSStreamPause(s16 streamNo, BOOL pause)
{
    msmStreamPause(streamNo, (pause) ? TRUE : FALSE, 5);
}

// Pauses or resumes one streamed track using the requested transition duration.
void HuAudSStreamPauseFadeOut(s16 streamNo, BOOL pause, s32 speed)
{
    msmStreamPause(streamNo, (pause) ? TRUE : FALSE, speed);
}

// Pauses or resumes every streamed track with a five millisecond transition.
void HuAudSStreamPauseAll(BOOL pause)
{
    msmStreamPauseAll((pause) ? TRUE : FALSE, 5);
}

// Stops every streamed track immediately.
void HuAudSStreamAllStop(void)
{
    msmStreamStopAll(0);
}

s32 HuAudSStreamStatGet(int streamNo)
{
    return msmStreamGetStatus(streamNo);
}

typedef struct sStreamFadeWork_s {
    u8 volStart; // Volume at the start of the fade.
    u8 volEnd; // Target volume at the end of the fade.
    u32 speed; // Fade duration in milliseconds.
    int streamNo; // Stream channel whose volume is changed.
} SSTREAMFADEWORK;

static void SStreamFade(void);

// Sets stream volume immediately for short durations, otherwise creates a fade process.
void HuAudSStreamParamSet(int streamNo, u8 vol, u32 speed)
{
    if(musicOffF) {
        return;
    }
    if(speed <= 16) {
        MSM_STREAMPARAM param;
        param.flag = MSM_STREAMPARAM_VOL;
        param.vol = vol;
        msmStreamSetParam(streamNo, &param);
        streamVol[streamNo] = vol;
    } else {
        HUPROCESS *process = HuPrcCreate(SStreamFade, 1, 2304, 0);
        SSTREAMFADEWORK *fadeWork = HuMemDirectMalloc(HEAP_HEAP, sizeof(SSTREAMFADEWORK));
        process->property = fadeWork;
        fadeWork->speed =  speed;
        fadeWork->streamNo = streamNo;
        fadeWork->volStart = streamVol[streamNo];
        fadeWork->volEnd = vol;
    }
}

// Stream-volume fade process callback; interpolates volume over the requested duration.
static void SStreamFade(void)
{
    HUPROCESS *process = HuPrcCurrentGet();
    SSTREAMFADEWORK *fadeWork = process->property;
    MSM_STREAMPARAM param;
    float fadeSteps;
    s16 step;
    param.flag = MSM_STREAMPARAM_VOL;
    fadeSteps = fadeWork->speed/16.666668f;
    for(step=1; step<=fadeSteps; step++) {
        float volume;
        float progress;
        progress = step/fadeSteps;
        volume = (fadeWork->volEnd*progress)+(fadeWork->volStart*(1.0-progress));
        param.vol = volume;
        msmStreamSetParam(fadeWork->streamNo, &param);
        streamVol[fadeWork->streamNo] = volume;
        HuPrcVSleep();
    }
    param.vol = fadeWork->volEnd;
    msmStreamSetParam(fadeWork->streamNo, &param);
    streamVol[fadeWork->streamNo] = fadeWork->volEnd;
    HuMemDirectFree(fadeWork);
    HuPrcEnd();
    HuPrcSleep(10);
}

SNDGRPTBL sndGrpTable[] = {
    { DLL_m601dll, MSM_GRP_MG601, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m602dll, MSM_GRP_MG602, MSM_GRP_NONE, MSM_AUX_6, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m603dll, MSM_GRP_MG603, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m604dll, MSM_GRP_MG604, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m605dll, MSM_GRP_MG605, MSM_GRP_NONE, MSM_AUX_1, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m606dll, MSM_GRP_MG606, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m607dll, MSM_GRP_MG607, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m608dll, MSM_GRP_MG608, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m609dll, MSM_GRP_MG609, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m610dll, MSM_GRP_MG610, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m611dll, MSM_GRP_MG611, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m612dll, MSM_GRP_MG612, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m613dll, MSM_GRP_MG613, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m614dll, MSM_GRP_MG614, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m615dll, MSM_GRP_MG615, MSM_GRP_NONE, MSM_AUX_6, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m616dll, MSM_GRP_MG616, MSM_GRP_NONE, MSM_AUX_6, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m617dll, MSM_GRP_MG617, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m618dll, MSM_GRP_MG618, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m619dll, MSM_GRP_MG619, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m620dll, MSM_GRP_MG620, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m621dll, MSM_GRP_MG621, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m622dll, MSM_GRP_MG622, MSM_GRP_NONE, MSM_AUX_6, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m623dll, MSM_GRP_MG623, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m624dll, MSM_GRP_MG624, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m625dll, MSM_GRP_MG625, MSM_GRP_NONE, MSM_AUX_5, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m626dll, MSM_GRP_MG626, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m627dll, MSM_GRP_MG627, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m628dll, MSM_GRP_MG628, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m629dll, MSM_GRP_MG629, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m630dll, MSM_GRP_MG630, MSM_GRP_NONE, MSM_AUX_6, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m631dll, MSM_GRP_MG631, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m632dll, MSM_GRP_MG632, MSM_GRP_NONE, MSM_AUX_6, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m633dll, MSM_GRP_MG633, MSM_GRP_NONE, MSM_AUX_6, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m634dll, MSM_GRP_MG634, MSM_GRP_NONE, MSM_AUX_6, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m635dll, MSM_GRP_MG635, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m636dll, MSM_GRP_MG636, MSM_GRP_NONE, MSM_AUX_6, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m637dll, MSM_GRP_MG637, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m638dll, MSM_GRP_MG638, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m639dll, MSM_GRP_MG639, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m640dll, MSM_GRP_MG640, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m641dll, MSM_GRP_MG641, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m642dll, MSM_GRP_MG642, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m643dll, MSM_GRP_MG643, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m644dll, MSM_GRP_MG644, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m645dll, MSM_GRP_MG645, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m646dll, MSM_GRP_MG646, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m647dll, MSM_GRP_MG647, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m648dll, MSM_GRP_MG648, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m649dll, MSM_GRP_MG649, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m650dll, MSM_GRP_MG650, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m651dll, MSM_GRP_MG651, MSM_GRP_NONE, MSM_AUX_4, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m652dll, MSM_GRP_MG652, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m653dll, MSM_GRP_MG653, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m654dll, MSM_GRP_MG654, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m655dll, MSM_GRP_MG655, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m656dll, MSM_GRP_MG656, MSM_GRP_NONE, MSM_AUX_4, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m657dll, MSM_GRP_MG657, MSM_GRP_NONE, MSM_AUX_4, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m658dll, MSM_GRP_MG658, MSM_GRP_NONE, MSM_AUX_4, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m659dll, MSM_GRP_MG659, MSM_GRP_NONE, MSM_AUX_4, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m660dll, MSM_GRP_MG660, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m661dll, MSM_GRP_MG661, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m662dll, MSM_GRP_MG662, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m663dll, MSM_GRP_MG663, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m664dll, MSM_GRP_MG664, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m665dll, MSM_GRP_MG665, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m666dll, MSM_GRP_MG666, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m667dll, MSM_GRP_MG667, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m668dll, MSM_GRP_MG668, MSM_GRP_NONE, MSM_AUX_1, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m669dll, MSM_GRP_MG669, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m670dll, MSM_GRP_MG670, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m671dll, MSM_GRP_MG671, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m672dll, MSM_GRP_MG672, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m673dll, MSM_GRP_MG673, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m674dll, MSM_GRP_MG674, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m675dll, MSM_GRP_MG675, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m676dll, MSM_GRP_MG676, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m677dll, MSM_GRP_MG677, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m678dll, MSM_GRP_MG678, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m679dll, MSM_GRP_MG679, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m680dll, MSM_GRP_MG680, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m681dll, MSM_GRP_MG681, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_m699dll, MSM_GRP_MG699, MSM_GRP_NONE, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_bootdll, MSM_GRP_NONE, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_openingdll, MSM_GRP_MENU, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_fileseldll, MSM_GRP_FILESEL, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_mdseldll, MSM_GRP_MENU, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_mdminidll, MSM_GRP_MENU, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_mdmicdll, MSM_GRP_MENU, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_mdpartydll, MSM_GRP_MENU, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_mdsingdll, MSM_GRP_MENU, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_mdbankdll, MSM_GRP_BANK, MSM_GRP_FILESEL, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_miraclebookdll, MSM_GRP_BANK, MSM_GRP_FILESEL, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_mdpresultdll, MSM_GRP_MENU, MSM_GRP_PRESULT, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_optiondll, MSM_GRP_MENU, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_endingdll, MSM_GRP_BANK, MSM_GRP_ENDING, MSM_AUX_2, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_staffdll, MSM_GRP_NONE, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_mgmfreedll, MSM_GRP_MGM00, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_mgmdecathlondll, MSM_GRP_MGM00, MSM_GRP_MGM02, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_mgmbattledll, MSM_GRP_MGM00, MSM_GRP_MGM01, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_mgmtournamentdll, MSM_GRP_MGM00, MSM_GRP_MGM03, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_mgmbingodll, MSM_GRP_MGM00, MSM_GRP_MGM05, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_mgmrenshodll, MSM_GRP_MGM00, MSM_GRP_MGM04, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_micquizdll, MSM_GRP_MICQUIZ, MSM_GRP_CHARMIC, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_micquizishidll, MSM_GRP_MICQUIZ, MSM_GRP_CHARMIC, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_micquizmyokodll, MSM_GRP_MICQUIZ, MSM_GRP_CHARMIC, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_micquizohdedll, MSM_GRP_MICQUIZ, MSM_GRP_CHARMIC, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_mikeactdll, MSM_GRP_MIKEACT, MSM_GRP_CHARMIC, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_instdll, MSM_GRP_NONE, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_resultdll, MSM_GRP_NONE, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_w01dll, MSM_GRP_BRD01, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_w02dll, MSM_GRP_BRD02, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_w03dll, MSM_GRP_BRD03, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_w04dll, MSM_GRP_BRD04, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_w05dll, MSM_GRP_BRD05, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_w06dll, MSM_GRP_BRD06, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_w10dll, MSM_GRP_BRDTT, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_mdpresultdll, MSM_GRP_MENU, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_s01dll, MSM_GRP_SBRD, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_s02dll, MSM_GRP_SBRD, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_s03dll, MSM_GRP_SBRD, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_w11dll, MSM_GRP_BRDTT, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, -1, -1 },
    { DLL_NONE, MSM_GRP_NONE, MSM_GRP_NONE, MSM_AUX_DEFAULT, MSM_AUX_DEFAULT, 0, 0 }
};

// Selects the overlay sound groups and auxiliary levels; boot is ignored and unlisted overlays fall
// back to MG601.
void HuAudDllSndGrpSet(u16 overlayId)
{
    SNDGRPTBL *soundGroupEntry;
    s16 soundGroupId;
    if(overlayId == DLL_bootdll) {
        return;
    }
    soundGroupEntry=&sndGrpTable[0];
    while(1) {
        if(soundGroupEntry->ovl == overlayId) {
            soundGroupId = soundGroupEntry->grpSet;
            break;
        }
        if(soundGroupEntry->ovl == DLL_NONE) {
            soundGroupId = MSM_GRP_MG601;
            break;
        }
        soundGroupEntry++;
    }
    OSReport("SOUND ##########################\n");
    if(soundGroupId != MSM_GRP_NONE) {
        HuAudSndGrpSetSet(soundGroupId);
        if(soundGroupEntry->loadGrp != MSM_GRP_NONE) {
            HuAudSndGrpSet(soundGroupEntry->loadGrp);
        }
    }
    if(soundGroupEntry->auxANo != auxANoBak || soundGroupEntry->auxBNo != auxBNoBak) {
        msmSysSetAux(soundGroupEntry->auxANo, soundGroupEntry->auxBNo);
        OSReport("Change AUX %d,%d\n", soundGroupEntry->auxANo, soundGroupEntry->auxBNo);
        auxANoBak = soundGroupEntry->auxANo;
        auxBNoBak = soundGroupEntry->auxBNo;
        HuPrcVSleep();
    }
    HuAudAUXVolSet(soundGroupEntry->auxAVol, soundGroupEntry->auxBVol);
    OSReport("##########################\n");
}

#define SNDGRP_TIMEOUT 500

#define SNDGRP_WAIT(tickStart)                                                                     \
    while ((msmMusGetNumPlay(TRUE) != 0 || msmSeGetNumPlay(TRUE) != 0) &&                          \
           OSTicksToMilliseconds(OSGetTick() - (tickStart)) < SNDGRP_TIMEOUT)

// Called by HuAudDllSndGrpSet and overlay setup code when a sound group changes. It stops playback,
// waits up to 500 ms, clears replaceable groups, then loads the requested samples. If memory cannot
// hold the samples, the requested group remains selected and is retried only after another group is
// selected.
void HuAudSndGrpSetSet(s16 grpSet)
{
    u32 sampleSize;
    void *sampleBuffer;
    OSTick tickStart;
    s32 groupLoadResult;
    
    if(sndGroupBak != grpSet) {
        msmMusStopAll(TRUE, 0);
        msmSeStopAll(TRUE, 0);
        tickStart = OSGetTick();
        SNDGRP_WAIT(tickStart);
        if(OSTicksToMilliseconds(OSGetTick()-tickStart) >= SNDGRP_TIMEOUT) {
            OSReport("Timed Out! Mus %d:SE %d\n", msmMusGetNumPlay(TRUE), msmSeGetNumPlay(TRUE));
        }
        OSReport("GroupSet %d\n", grpSet);
        sndGroupBak = grpSet;
        groupLoadResult = msmSysDelGroupAll(); // Replaced by the following load result.
        sampleSize = msmSysGetSampSize(grpSet);
        if(HuMemMaxMemorySizeGet(HuMemHeapPtrGet(HEAP_MODEL)) > sampleSize) {
            sampleBuffer = HuMemDirectMalloc(HEAP_MODEL, sampleSize);
        } else if(HuMemMaxMemorySizeGet(HuMemHeapPtrGet(HEAP_HEAP)) > sampleSize) {
            sampleBuffer = HuMemDirectMalloc(HEAP_HEAP, sampleSize);
        } else if(HuMemMaxMemorySizeGet(HuMemHeapPtrGet(HEAP_DVD)) > sampleSize) {
            sampleBuffer = HuMemDirectMalloc(HEAP_DVD, sampleSize);
        } else {
            OSReport("Error: Sound GroupSet Error!!\n");
            return;
        }
        
        groupLoadResult = msmSysLoadGroup(grpSet, sampleBuffer, FALSE);
        if(groupLoadResult) {
            OSReport("***********GroupSet Error %d\n", groupLoadResult);
        }
        HuMemDirectFree(sampleBuffer);
    }
}

// Loads one sound group into temporary model-heap storage.
void HuAudSndGrpSet(s16 grp)
{
    void *sampleBuffer = HuMemDirectMalloc(HEAP_MODEL, msmSysGetSampSize(grp));
    msmSysLoadGroup(grp, sampleBuffer, FALSE);
    HuMemDirectFree(sampleBuffer);
}

// Clears character sound-load flags, stops playback, waits up to 500 ms, optionally deletes base
// group 0, then loads grp as the common base group.
void HuAudSndCommonGrpSet(s16 grp, BOOL delGrpF) 
{
    OSTick tickStart;
    s16 deleteResult;
    void *sampleBuffer;
    s16 characterIndex;
    
    for(characterIndex=0; characterIndex<GW_CHARA_MAX; characterIndex++) {
        CharSeLoadF[characterIndex] = 0;
    }
    msmMusStopAll(TRUE, 0);
    msmSeStopAll(TRUE, 0);
    tickStart = OSGetTick();
    SNDGRP_WAIT(tickStart);
    OSReport("CommonGrpSet %d\n", grp);
    if(delGrpF) {
        deleteResult = msmSysDelGroupBase(0);
        if(deleteResult < 0) {
            OSReport("Del Group Error %d\n", deleteResult);
        }
    }
    sampleBuffer = HuMemDirectMalloc(HEAP_MODEL, msmSysGetSampSize(grp));
    msmSysLoadGroupBase(grp, sampleBuffer);
    HuMemDirectFree(sampleBuffer);
    sndGroupBak = MSM_GRP_NONE;
}

// Selects auxiliary buses, translating the none value to the sound manager default.
void HuAudAUXSet(s32 auxA, s32 auxB)
{
    if(auxA == MSM_AUX_NONE) {
        auxA = MSM_AUX_DEFAULT;
    }
    if(auxB == MSM_AUX_NONE) {
        auxB = MSM_AUX_DEFAULT;
    }
    auxANoBak = auxA;
    auxBNoBak = auxB;
    msmSysSetAux(auxA, auxB);
}

// Stores the requested auxiliary bus volumes for later character sound playback.
void HuAudAUXVolSet(s8 volA, s8 volB)
{
    HuAuxAVol = volA;
    HuAuxBVol = volB;
}

// Plays a character sound using the character selected for this player.
s32 PlayerFXPlay(s16 playerNo, s16 seId)
{
    s16 characterNo = GwPlayerConf[playerNo].charNo;
    return CharFXPlay(characterNo, seId);
}

// Plays a character sound at a world position using this player's selected character.
s32 PlayerFXPlayPos(s16 playerNo, s16 seId, Vec *pos)
{
    s16 characterNo = GwPlayerConf[playerNo].charNo;
    return CharFXPlayPos(characterNo, seId, pos);
}

// Stops all active instances of this sound for the player's character when a REL requests it.
void PlayerFXStop(s16 playerNo, s16 seId)
{
    s16 characterNo = GwPlayerConf[playerNo].charNo;
    CharFXStop(characterNo, seId);
}

unsigned int CharSeTable[GW_CHARA_MAX] = {
    MSM_SE_CHAR_MARIO, //GW_CHARA_MARIO
    MSM_SE_CHAR_LUIGI, //GW_CHARA_LUIGI
    MSM_SE_CHAR_PEACH, //GW_CHARA_PEACH
    MSM_SE_CHAR_YOSHI, //GW_CHARA_YOSHI
    MSM_SE_CHAR_WARIO, //GW_CHARA_WARIO
    MSM_SE_CHAR_DAISY, //GW_CHARA_DAISY
    MSM_SE_CHAR_WALUIGI, //GW_CHARA_WALUIGI
    MSM_SE_CHAR_KINOPIO, //GW_CHARA_KINOPIO
    MSM_SE_CHAR_TERESA, //GW_CHARA_TERESA
    MSM_SE_CHAR_MINIKOOPAR, //GW_CHARA_MINIKOOPA
    MSM_SE_CHAR_KINOPIKO, //GW_CHARA_KINOPIKO
    MSM_SE_CHAR_MINIKOOPAR, //GW_CHARA_MINIKOOPAR
    MSM_SE_CHAR_MINIKOOPAG, //GW_CHARA_MINIKOOPAG
    MSM_SE_CHAR_MINIKOOPAB, //GW_CHARA_MINIKOOPAB
};

unsigned int CharVoiceSeTable[GW_CHARA_MAX] = {
    MSM_SE_CHARVOICE_MARIO, //GW_CHARA_MARIO
    MSM_SE_CHARVOICE_LUIGI, //GW_CHARA_LUIGI
    MSM_SE_CHARVOICE_PEACH, //GW_CHARA_PEACH
    MSM_SE_CHARVOICE_YOSHI, //GW_CHARA_YOSHI
    MSM_SE_CHARVOICE_WARIO, //GW_CHARA_WARIO
    MSM_SE_CHARVOICE_DAISY, //GW_CHARA_DAISY
    MSM_SE_CHARVOICE_WALUIGI, //GW_CHARA_WALUIGI
    MSM_SE_CHARVOICE_KINOPIO, //GW_CHARA_KINOPIO
    MSM_SE_CHARVOICE_TERESA, //GW_CHARA_TERESA
    MSM_SE_CHARVOICE_MINIKOOPAR, //GW_CHARA_MINIKOOPA
    MSM_SE_CHARVOICE_KINOPIKO, //GW_CHARA_KINOPIKO
    MSM_SE_CHARVOICE_MINIKOOPAR, //GW_CHARA_MINIKOOPAR
    MSM_SE_CHARVOICE_MINIKOOPAG, //GW_CHARA_MINIKOOPAG
    MSM_SE_CHARVOICE_MINIKOOPAB, //GW_CHARA_MINIKOOPAB
};

unsigned int CharMicSeTable[GW_CHARA_MAX] = {
    MSM_SE_CHARMIC_MARIO, //GW_CHARA_MARIO
    MSM_SE_CHARMIC_LUIGI, //GW_CHARA_LUIGI
    MSM_SE_CHARMIC_PEACH, //GW_CHARA_PEACH
    MSM_SE_CHARMIC_YOSHI, //GW_CHARA_YOSHI
    MSM_SE_CHARMIC_WARIO, //GW_CHARA_WARIO
    MSM_SE_CHARMIC_DAISY, //GW_CHARA_DAISY
    MSM_SE_CHARMIC_WALUIGI, //GW_CHARA_WALUIGI
    MSM_SE_CHARMIC_KINOPIO, //GW_CHARA_KINOPIO
    MSM_SE_CHARMIC_TERESA, //GW_CHARA_TERESA
    MSM_SE_CHARMIC_MINIKOOPAR, //GW_CHARA_MINIKOOPA
    MSM_SE_CHARMIC_KINOPIKO, //GW_CHARA_KINOPIKO
    MSM_SE_CHARMIC_MINIKOOPAR, //GW_CHARA_MINIKOOPAR
    MSM_SE_CHARMIC_MINIKOOPAG, //GW_CHARA_MINIKOOPAG
    MSM_SE_CHARMIC_MINIKOOPAB, //GW_CHARA_MINIKOOPAB
};

static unsigned int *CharSoundList[] = { CharSeTable, CharVoiceSeTable, CharMicSeTable, NULL };

// Maps a character-independent effect, voice, or mic ID to this character and plays with the
// requested volume, pan, and configured aux levels.
s32 CharFXPlayVolPan(s16 charNo, s16 seId, s16 vol, s16 pan)
{
    MSM_SEPARAM param;
    s16 soundTableIndex;
    unsigned int *characterSoundTable;
    if(HuSeExitCheck()) {
        return 0;
    }
    for(soundTableIndex=0; CharSoundList[soundTableIndex]; soundTableIndex++) {
        characterSoundTable = CharSoundList[soundTableIndex];
        if(seId < characterSoundTable[GW_CHARA_MARIO]) {
            break;
        }
    }
    if(soundTableIndex == 0) {
        return -1;
    }
    characterSoundTable = CharSoundList[soundTableIndex-1];
    seId -= characterSoundTable[GW_CHARA_MARIO];
    seId += characterSoundTable[charNo];
    param.flag = MSM_SEPARAM_NONE;
    if(HuAuxAVol != -1) {
        param.flag |= MSM_SEPARAM_AUXVOLA;
    }
    if(HuAuxBVol != -1) {
        param.flag |= MSM_SEPARAM_AUXVOLB;
    }
    param.auxAVol = HuAuxAVol;
    param.auxBVol = HuAuxBVol;
    param.flag |= MSM_SEPARAM_VOL|MSM_SEPARAM_PAN;
    param.vol = vol;
    param.pan = pan;
    return HuSePlay(seId, &param);
}

// Plays a character sound at default volume and centered pan.
s32 CharFXPlay(s16 charNo, s16 seId)
{
    return CharFXPlayVolPan(charNo, seId, MSM_VOL_MAX, MSM_PAN_CENTER);
}

// Plays a character sound at the requested volume and centered pan.
s32 CharFXPlayVol(s16 charNo, s16 seId, s16 vol)
{
    return CharFXPlayVolPan(charNo, seId, vol, MSM_PAN_CENTER);
}

// Plays a character sound at the requested pan and default volume.
s32 CharFXPlayPan(s16 charNo, s16 seId, s16 pan)
{
    return CharFXPlayVolPan(charNo, seId, MSM_VOL_MAX, pan);
}

// Maps a character voice/effect ID and aux levels; blocks requests during shutdown or after a
// wipe passes its midpoint. Scheduled playback rechecks shutdown only.
void CharFXPlayDelayVolPan(s16 charNo, s16 seId, u8 vol, u8 pan, u16 delay)
{
    MSM_SEPARAM param;
    if(HuSeExitCheck()) {
        return;
    }
    if(seId < MSM_SE_CHARVOICE_MARIO) {
        seId -= MSM_SE_CHAR_MARIO;
        seId += CharSeTable[charNo];
    } else {
        seId -= MSM_SE_CHARVOICE_MARIO;
        seId += CharVoiceSeTable[charNo];
    }
    param.flag = MSM_SEPARAM_NONE;
    if(HuAuxAVol != -1) {
        param.flag |= MSM_SEPARAM_AUXVOLA;
    }
    if(HuAuxBVol != -1) {
        param.flag |= MSM_SEPARAM_AUXVOLB;
    }
    param.auxAVol = HuAuxAVol;
    param.auxBVol = HuAuxBVol;
    param.flag |= MSM_SEPARAM_VOL|MSM_SEPARAM_PAN;
    param.vol = vol;
    param.pan = pan;
    if(delay == 0) {
        HuSePlay(seId, &param);
        return;
    } else {
        HUPROCESS *delayProcess = HuPrcCreate(AudFXPlayDelay, 1, 2304, 0);
        HUAUD_FX_DELAY *delayWork = HuMemDirectMalloc(HEAP_HEAP, sizeof(HUAUD_FX_DELAY));
        delayProcess->property = delayWork;
        delayWork->param = param;
        delayWork->seId = seId;
        delayWork->delay = delay;
    }
}

// Schedules a default-volume, centered character sound after the requested frame delay.
void CharFXPlayDelay(s16 charNo, s16 seId, u16 delay)
{
    CharFXPlayDelayVolPan(charNo, seId, MSM_VOL_MAX, MSM_PAN_CENTER, delay);
}

// Schedules a centered character sound at the requested volume after the frame delay.
void CharFXPlayDelayVol(s16 charNo, s16 seId, u8 vol, u16 delay)
{
    CharFXPlayDelayVolPan(charNo, seId, vol, MSM_PAN_CENTER, delay);
}

// Schedules a default-volume character sound at the requested pan after the frame delay.
void CharFXPlayDelayPan(s16 charNo, s16 seId, u8 pan, u16 delay)
{
    CharFXPlayDelayVolPan(charNo, seId, MSM_VOL_MAX, pan, delay);
}

// Maps a character effect ID through CharSeTable and plays it at a world position with configured
// aux bus levels.
s32 CharFXPlayPos(s16 charNo, s16 seId, Vec *pos)
{
    MSM_SEPARAM param;
    if(omSysExitReq) {
        return 0;
    }
    seId -= MSM_SE_CHAR_MARIO;
    seId += CharSeTable[charNo];
    param.flag = MSM_SEPARAM_POS;
    if(HuAuxAVol != -1) {
        param.flag |= MSM_SEPARAM_AUXVOLA;
    }
    if(HuAuxBVol != -1) {
        param.flag |= MSM_SEPARAM_AUXVOLB;
    }
    param.auxAVol = HuAuxAVol;
    param.auxBVol = HuAuxBVol;
    param.pos.x = pos->x;
    param.pos.y = pos->y;
    param.pos.z = pos->z;
    return HuSePlay(seId, &param);
}

// Finds and stops every active instance of this character's mapped sound effect.
void CharFXStop(s16 charNo, s16 seId)
{
    int activeSeNos[MSM_ENTRY_SENO_MAX];
    u16 activeSeCount;
    u16 entryIndex;
    seId -= MSM_SE_CHAR_MARIO;
    seId += CharSeTable[charNo];
    activeSeCount = msmSeGetEntryID(seId, activeSeNos);
    for(entryIndex=0; entryIndex<activeSeCount; entryIndex++) {
        msmSeStop(activeSeNos[entryIndex], 0);
    }
}

// Submits a sound effect to the sound manager and reports entry errors.
static int HuSePlay(int seId, MSM_SEPARAM *param)
{
    int playResult = msmSePlay(seId, param);
    if(playResult < 0) {
        OSReport("#########SE Entry Error<SE %d:ErrorNo %d>\n", seId, playResult);
    }
    return playResult;
}

// Blocks new effects during shutdown or after an outgoing screen wipe passes its midpoint.
static BOOL HuSeExitCheck(void)
{
    WIPEWORK *wipe = &wipeData;
    if(omSysExitReq || (wipeData.mode == WIPE_MODE_OUT && (wipe->time/wipe->maxTime > 0.5))) {
        return TRUE;
    } else {
        return FALSE;
    }
}
