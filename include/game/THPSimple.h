// THP movie loading, decoding, drawing, and audio playback interfaces.
#ifndef _GAME_THPSIMPLE_H
#define _GAME_THPSIMPLE_H

#include "dolphin.h"
#include "dolphin/thp/THPFile.h"
#include "dolphin/thp/THPInfo.h"

// Compressed frame storage, its zero-based movie frame number, and a ready flag.
// THPSimpleDecode clears frameReady after processing all components; video failure leaves it set.
typedef struct {
    /* 0x00 */ s32 *frameData;
    /* 0x04 */ s32 frameNumber;
    /* 0x08 */ s32 frameReady;
} UnkThpStruct01; // Size 0xC

// One set of decoded YUV planes in the two-buffer display ring.
// Only videoBuffers[0].frameIndex holds the shared last-successful movie frame number;
// -1 means no frame is ready to draw. A failed decode can still change the selected planes.
typedef struct {
    /* 0x00 */ void *yPlane;
    /* 0x04 */ void *cbPlane;
    /* 0x08 */ void *crPlane;
    /* 0x0C */ s32 frameIndex;
} UnkThpStruct02; // Size 0x10

// Decoded interleaved stereo audio storage and the next pair of s16 samples to mix.
// samplesRemaining counts stereo frames; sampleCursor advances as those frames are consumed.
typedef struct {
    /* 0x00 */ void *sampleBuffer;
    /* 0x04 */ void *sampleCursor;
    /* 0x08 */ u32 samplesRemaining;
} UnkThpStruct03; // Size 0xC

typedef struct {
    /* 0x00 */ u32 width;
    /* 0x04 */ u32 height;
} UnkThpStruct04; // Size 8

// State for the single open movie and its read, video, and audio buffer rings.
// loopMovie holds the low byte of the preload request; only 1 enables frame wrapping.
// Volume fields use gain-table index units. volumeStep is the change per mixed stereo frame,
// and volumeRampSamples counts the remaining stereo frames in that ramp.
typedef struct {
    /* 0x000 */ DVDFileInfo fileInfo;
    /* 0x03C */ THPHeader header;
    /* 0x06C */ THPFrameCompInfo componentInfo;
    /* 0x080 */ UnkThpStruct04 videoInfo;
    /* 0x088 */ u8 audioInfoData[12];
    /* 0x094 */ void *decodeWork;
    /* 0x098 */ s32 movieOpen;
    /* 0x09C */ u8 loadingActive;
    /* 0x09D */ u8 audioActive;
    /* 0x09E */ u8 loopMovie;
    /* 0x09F */ u8 hasAudio;
    /* 0x0A0 */ s32 nextFrameOffset;
    /* 0x0A4 */ s32 readError;
    /* 0x0A8 */ u32 readPending;
    /* 0x0AC */ s32 decodeFrameSlot;
    /* 0x0B0 */ s32 readFrameSlot;
    /* 0x0B4 */ s32 nextFrameSize;
    /* 0x0B8 */ s32 nextFrameNumber;
    /* 0x0BC */ float currentVolume;
    /* 0x0C0 */ float targetVolume;
    /* 0x0C4 */ float volumeStep;
    /* 0x0C8 */ s32 volumeRampSamples;
    /* 0x0CC */ UnkThpStruct01 readBuffers[10];
    /* 0x144 */ UnkThpStruct02 videoBuffers[2];
    /* 0x164 */ UnkThpStruct03 audioBuffers[4];
    /* 0x194 */ s32 audioDecodeSlot;
    /* 0x198 */ s32 audioMixSlot;
// drawMode == 1 selects a model; other values select a sprite.
// drawObjectId is the corresponding model or sprite handle.
    /* 0x19C */ u8 drawMode;
    /* 0x19D */ char unk19D[1];
    /* 0x19E */ s16 drawObjectId;
} UnkThpStruct10; // Size 0x1A0

BOOL THPSimpleInit(s32 arg0);
void THPSimpleQuit(void);
s32 THPSimpleOpen(char *arg0);
s32 THPSimpleRead(DVDFileInfo *fileInfo, void *addr, s32 length, s32 offset);
s32 THPSimpleClose(void);
s32 THPSimpleCalcNeedMemory(void);
s32 THPSimpleSetBuffer(void *arg0);
s32 THPSimplePreLoad(s32 arg0);
void THPSimpleAudioStart(void);
void THPSimpleAudioStop(void);
s32 THPSimpleLoadStop(void);
s32 THPSimpleDecode(void);
s32 THPSimpleDrawCurrentFrame(GXRenderModeObj *arg0, GXColor *arg1, Mtx arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);
s32 THPSimpleGetVideoInfo(UnkThpStruct04 *arg0);
s32 THPSimpleGetTotalFrame(void);
float THPSimpleGetFrameRate(void);
s32 THPSimpleSetVolume(s32 targetVolume, s32 rampDuration);

extern UnkThpStruct10 SimpleControl;
extern s32 bufIdx;

#endif
