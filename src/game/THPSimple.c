// Opens THP movie files, buffers their frames, and feeds decoded video and audio to playback.
#include "game/THPSimple.h"
#include "game/THPDraw.h"

#define THP_ALIGNMENT_MASK 0x1F
#define ROUND_UP_32(val) (((val) + THP_ALIGNMENT_MASK) & ~THP_ALIGNMENT_MASK)
#define THP_WORK_AREA_SIZE 4096
#define AUDIO_MIX_SAMPLE_COUNT 160
#define THP_FILE_VERSION 65536
#define AI_DMA_CACHED_ADDRESS_BIT_PATTERN 0x80000000

static void __THPSimpleDVDCallback(s32 result, DVDFileInfo *fileInfo);
static s32 VideoDecode(void *frameComponentData);
static void THPAudioMixCallback(void);

UnkThpStruct10 SimpleControl;
static u8 SoundBuffer[2][640];
static u8 WorkBuffer[64] ATTRIBUTE_ALIGN(32);

static s32 SoundBufferIndex;
static void *LastAudioBuffer;
static void *CurAudioBuffer;
static s32 ReadResult;
static s32 CallbackStatus;
s32 bufIdx;
static s32 AudioSystem;
static void (*OldAIDCallback)();
static s32 Initialized;

// HuTHPSprCreateVol and HuTHP3DCreateVol call this to initialize the decoder for movie playback.
BOOL THPSimpleInit(s32 audioSystemMode)
{
    BOOL previousInterruptState;

    memset(&SimpleControl, 0, sizeof(SimpleControl));
    LCEnable();
    if(THPInit() == FALSE) {
        return 0;
    }
    previousInterruptState = OSDisableInterrupts();
    AudioSystem = audioSystemMode;
    SoundBufferIndex = 0;
    LastAudioBuffer = NULL;
    CurAudioBuffer = NULL;
    OldAIDCallback = AIRegisterDMACallback(THPAudioMixCallback);
    // A nonzero audio mode needs an existing DMA callback. Failure removes the new callback
    // but leaves locked cache enabled and the THP decoder initialized.
    if(OldAIDCallback == NULL && AudioSystem != 0) {
        AIRegisterDMACallback(NULL);
        OSRestoreInterrupts(previousInterruptState);
        return 0;
    }
    OSRestoreInterrupts(previousInterruptState);
    if(AudioSystem == 0) {
        memset(SoundBuffer, 0, sizeof(SoundBuffer));
        DCFlushRange(SoundBuffer, sizeof(SoundBuffer));
        AIInitDMA((u32)&SoundBuffer[SoundBufferIndex], sizeof(SoundBuffer[0]));
        AIStartDMA();
    }
    Initialized = 1;
    return 1;
}

// THPTestProc calls this after stopping playback to disable locked cache and restore a saved
// non-null audio DMA callback. With no saved callback, it leaves the callback unchanged;
// it does not stop DMA.
void THPSimpleQuit(void) {
    s32 interruptState;

    LCDisable();
    interruptState = OSDisableInterrupts();
    if(OldAIDCallback) {
        AIRegisterDMACallback(OldAIDCallback);
    }
    OSRestoreInterrupts(interruptState);
    Initialized = 0;
}

// THPTestProc calls this until the movie opens and its header and component records are readable.
s32 THPSimpleOpen(char *fileName) {
    s32 componentInfoOffset;
    s32 componentIndex;

    if(Initialized == 0) {
        return 0;
    }
    if(SimpleControl.movieOpen != 0) {
        return 0;
    }
    memset(&SimpleControl.videoInfo, 0, sizeof(SimpleControl.videoInfo));
    memset(SimpleControl.audioInfoData, 0, sizeof(SimpleControl.audioInfoData));
    if(DVDOpen(fileName, &SimpleControl.fileInfo) == 0) {
        return 0;
    }
    if(THPSimpleRead(&SimpleControl.fileInfo, &WorkBuffer, 64, 0) < 0) {
        DVDClose(&SimpleControl.fileInfo);
        return 0;
    }
    memcpy(&SimpleControl.header, &WorkBuffer, sizeof(SimpleControl.header));
    if(strcmp(SimpleControl.header.mMagic, "THP") != 0) {
        DVDClose(&SimpleControl.fileInfo);
        return 0;
    }
    if(SimpleControl.header.mVersion != THP_FILE_VERSION) {
        DVDClose(&SimpleControl.fileInfo);
        return 0;
    }
    componentInfoOffset = SimpleControl.header.mCompInfoDataOffsets;
    if(THPSimpleRead(&SimpleControl.fileInfo, &WorkBuffer, 32, componentInfoOffset) < 0) {
        DVDClose(&SimpleControl.fileInfo);
        return 0;
    }
    memcpy(&SimpleControl.componentInfo, &WorkBuffer, sizeof(SimpleControl.componentInfo));
    componentInfoOffset += sizeof(SimpleControl.componentInfo);
    SimpleControl.hasAudio = 0;
    for (componentIndex = 0; componentIndex < SimpleControl.componentInfo.mNumComponents;
         componentIndex++) {
        switch (SimpleControl.componentInfo.mFrameComp[componentIndex]) {
            case 0:
                if (THPSimpleRead(&SimpleControl.fileInfo, &WorkBuffer, 32, componentInfoOffset) <
                    0) {
                    DVDClose(&SimpleControl.fileInfo);
                    return 0;
                }
                memcpy(&SimpleControl.videoInfo, &WorkBuffer, sizeof(SimpleControl.videoInfo));
                componentInfoOffset += sizeof(SimpleControl.videoInfo);
                break;
            case 1:
                if (THPSimpleRead(&SimpleControl.fileInfo, &WorkBuffer, 32, componentInfoOffset) <
                    0) {
                    DVDClose(&SimpleControl.fileInfo);
                    return 0;
                }
                memcpy(SimpleControl.audioInfoData, &WorkBuffer,
                       sizeof(SimpleControl.audioInfoData));
                SimpleControl.hasAudio = 1;
                componentInfoOffset += sizeof(SimpleControl.audioInfoData);
                break;
            default:
                // An unsupported component type fails here without closing the opened DVD file.
                return 0;
        }
    }
    SimpleControl.nextFrameOffset = SimpleControl.header.mMovieDataOffsets;
    SimpleControl.nextFrameSize = SimpleControl.header.mFirstFrameSize;
    SimpleControl.readFrameSlot = 0;
    SimpleControl.nextFrameNumber = 0;
    SimpleControl.readError = 0;
    SimpleControl.videoBuffers[0].frameIndex = -1;
    SimpleControl.decodeFrameSlot = 0;
    SimpleControl.audioDecodeSlot = 0;
    SimpleControl.audioMixSlot = 0;
    SimpleControl.loadingActive = 0;
    SimpleControl.audioActive = 0;
    SimpleControl.loopMovie = 0;
    SimpleControl.movieOpen = 1;
    SimpleControl.targetVolume = SimpleControl.currentVolume;
    SimpleControl.volumeRampSamples = 0;
    return 1;
}

// DVDReadAsyncPrio calls this when a read completes; THPSimpleRead returns the saved status.
static void ReadCallback(s32 readResult, DVDFileInfo* fileInfo)
{
    // DVD completion is consumed by THPSimpleRead after the waiting process wakes.
    ReadResult = readResult;
    CallbackStatus = TRUE;
}

// THPSimpleOpen reads metadata and THPSimplePreLoad reads frames through this blocking helper.
s32 THPSimpleRead(DVDFileInfo *movieFile, void *readBuffer, s32 readLength, s32 fileOffset)
{
    BOOL readQueued;
    CallbackStatus = FALSE;
    readQueued = DVDReadAsyncPrio(movieFile, readBuffer, readLength, fileOffset, ReadCallback, 0);
    if(readQueued == FALSE) {
        return DVD_RESULT_FATAL_ERROR;
    }
    while(!CallbackStatus) {
        HuPrcVSleep();
    }
    return ReadResult;
}

// THPTestProc calls this during shutdown after stopping audio and frame loading.
s32 THPSimpleClose(void) {
    if(SimpleControl.movieOpen != 0 && SimpleControl.loadingActive == 0) {
        if(SimpleControl.hasAudio != 0) {
            if(SimpleControl.audioActive == 1) {
                return 0;
            }
        } else {
            SimpleControl.audioActive = 0;
        }
        if(SimpleControl.readPending == 0) {
            SimpleControl.movieOpen = 0;
            DVDClose(&SimpleControl.fileInfo);
            return 1;
        }
    }
    return 0;
}

// THPTestProc calls this after opening the movie to size its decoded planes and frame buffers.
s32 THPSimpleCalcNeedMemory(void) {
    s32 requiredBytes;

    if(SimpleControl.movieOpen != 0) {
        requiredBytes = ROUND_UP_32(SimpleControl.header.mBufferSize) * 10;
        requiredBytes +=
            ROUND_UP_32(SimpleControl.videoInfo.width * SimpleControl.videoInfo.height);
        requiredBytes +=
            ROUND_UP_32(SimpleControl.videoInfo.width * SimpleControl.videoInfo.height / 4);
        requiredBytes +=
            ROUND_UP_32(SimpleControl.videoInfo.width * SimpleControl.videoInfo.height / 4);
        requiredBytes +=
            ROUND_UP_32(SimpleControl.videoInfo.width * SimpleControl.videoInfo.height);
        requiredBytes +=
            ROUND_UP_32(SimpleControl.videoInfo.width * SimpleControl.videoInfo.height / 4);
        requiredBytes +=
            ROUND_UP_32(SimpleControl.videoInfo.width * SimpleControl.videoInfo.height / 4);
        if(SimpleControl.hasAudio != 0) {
            requiredBytes += ROUND_UP_32(SimpleControl.header.mAudioMaxSamples * 4) * 4;
        }
        // Reserve a fixed 4 KiB working area in addition to the frame buffers.
        requiredBytes += THP_WORK_AREA_SIZE;
        return requiredBytes;
    }
    return 0;
}

// THPTestProc calls this after allocating the size returned by THPSimpleCalcNeedMemory.
// Buffers are assigned only for an open movie with loading stopped; active audio returns zero.
// A closed movie or active loading leaves the buffers untouched but still returns one.
s32 THPSimpleSetBuffer(void *bufferMemory) {
    u8 *nextBuffer;
    u32 yPlaneBytes;
    u32 chromaPlaneBytes;
    u32 bufferIndex;

    if(SimpleControl.movieOpen != 0 && SimpleControl.loadingActive == 0) {
        if(SimpleControl.audioActive == 1) {
            return 0;
        }
        yPlaneBytes = ROUND_UP_32(SimpleControl.videoInfo.width * SimpleControl.videoInfo.height);
        chromaPlaneBytes =
            ROUND_UP_32(SimpleControl.videoInfo.width * SimpleControl.videoInfo.height / 4);
        nextBuffer = bufferMemory;
        for(bufferIndex = 0; bufferIndex < 2; bufferIndex++) {
            SimpleControl.videoBuffers[bufferIndex].yPlane = nextBuffer;
            DCInvalidateRange(nextBuffer, yPlaneBytes);
            nextBuffer += yPlaneBytes;
            SimpleControl.videoBuffers[bufferIndex].cbPlane = nextBuffer;
            DCInvalidateRange(nextBuffer, chromaPlaneBytes);
            nextBuffer += chromaPlaneBytes;
            SimpleControl.videoBuffers[bufferIndex].crPlane = nextBuffer;
            DCInvalidateRange(nextBuffer, chromaPlaneBytes);
            nextBuffer += chromaPlaneBytes;
        }
        for(bufferIndex = 0; bufferIndex < 10; bufferIndex++) {
            SimpleControl.readBuffers[bufferIndex].frameData = (s32*) nextBuffer;
            nextBuffer += ROUND_UP_32(SimpleControl.header.mBufferSize);
            SimpleControl.readBuffers[bufferIndex].frameReady = 0;
        }
        if(SimpleControl.hasAudio != 0) {
            for(bufferIndex = 0; bufferIndex < 4; bufferIndex++) {
                SimpleControl.audioBuffers[bufferIndex].sampleBuffer = nextBuffer;
                SimpleControl.audioBuffers[bufferIndex].sampleCursor = nextBuffer;
                SimpleControl.audioBuffers[bufferIndex].samplesRemaining = 0;
                nextBuffer += ROUND_UP_32(SimpleControl.header.mAudioMaxSamples * 4);
            }
        }
        SimpleControl.decodeWork = nextBuffer;
    }
    return 1;
}

// The DVD callback and THPSimpleDecode call this when the loader has a free frame-buffer slot.
static inline void THPSimpleInlineFunc(void) {
    if(SimpleControl.nextFrameNumber > SimpleControl.header.mNumFrames - 1) {
        if(SimpleControl.loopMovie != 1) {
            return;
        }
        SimpleControl.nextFrameNumber = 0;
        SimpleControl.nextFrameOffset = SimpleControl.header.mMovieDataOffsets;
        SimpleControl.nextFrameSize = SimpleControl.header.mFirstFrameSize;
    }
    // The frame slots wrap independently of the movie frame number.
    SimpleControl.readPending = 1;
    if (DVDReadAsyncPrio(&SimpleControl.fileInfo,
                         SimpleControl.readBuffers[SimpleControl.readFrameSlot].frameData,
                         SimpleControl.nextFrameSize, SimpleControl.nextFrameOffset,
                         __THPSimpleDVDCallback, 2) != 1) {
        SimpleControl.readPending = 0;
        SimpleControl.readError = 1;
    }
}

// Any status except fatal or canceled is treated as a completed read; the frame ring advances.
// Another read is queued only when loading is active, the next slot is free, and readError is
// clear. Fatal completion sets readError without clearing readPending; cancellation leaves
// both flags unchanged.
static void __THPSimpleDVDCallback(s32 readStatus, DVDFileInfo *fileInfo) {
    s32 nextReadSlot;

    if(readStatus == DVD_RESULT_FATAL_ERROR) {
        SimpleControl.readError = 1;
        return;
    }
    // A canceled read is being discarded while playback stops.
    if(readStatus != DVD_RESULT_CANCELED) {
        SimpleControl.readPending = 0;
        SimpleControl.readBuffers[SimpleControl.readFrameSlot].frameNumber =
            SimpleControl.nextFrameNumber;
        SimpleControl.nextFrameNumber++;
        SimpleControl.readBuffers[SimpleControl.readFrameSlot].frameReady = 1;
        SimpleControl.nextFrameOffset += SimpleControl.nextFrameSize;
        SimpleControl.nextFrameSize =
            *SimpleControl.readBuffers[SimpleControl.readFrameSlot].frameData;
        if(SimpleControl.readFrameSlot + 1 >= 10) {
            nextReadSlot = 0;
        } else {
            nextReadSlot = SimpleControl.readFrameSlot + 1;
        }
        SimpleControl.readFrameSlot = nextReadSlot;
        if (SimpleControl.readBuffers[SimpleControl.readFrameSlot].frameReady == 0 &&
            SimpleControl.readError == 0 && SimpleControl.loadingActive == 1) {
            THPSimpleInlineFunc();
        }
    }
}

// THPTestProc calls this before playback and after restart. The requested loop flag is stored
// only after preloading, so the initial preload does not wrap a short looping movie.
s32 THPSimplePreLoad(s32 loopMovie) {
    u32 preloadFrameCount;
    s32 frameIndex;

    if(SimpleControl.movieOpen != 0 && SimpleControl.loadingActive == 0) {
        preloadFrameCount = 10;
        if(loopMovie == 0 && SimpleControl.header.mNumFrames < 10) {
            preloadFrameCount = SimpleControl.header.mNumFrames;
        }
        for(frameIndex = 0; frameIndex < preloadFrameCount; frameIndex++) {
            if (THPSimpleRead(&SimpleControl.fileInfo,
                              SimpleControl.readBuffers[SimpleControl.readFrameSlot].frameData,
                              SimpleControl.nextFrameSize, SimpleControl.nextFrameOffset) < 0) {
                SimpleControl.readError = 1;
                return 0;
            }
            SimpleControl.nextFrameOffset += SimpleControl.nextFrameSize;
            SimpleControl.nextFrameSize =
                *SimpleControl.readBuffers[SimpleControl.readFrameSlot].frameData;
            SimpleControl.readBuffers[SimpleControl.readFrameSlot].frameReady = 1;
            SimpleControl.readBuffers[SimpleControl.readFrameSlot].frameNumber =
                SimpleControl.nextFrameNumber;
            SimpleControl.readFrameSlot =
                (SimpleControl.readFrameSlot + 1 >= 10) ? 0 : (SimpleControl.readFrameSlot + 1);
            SimpleControl.nextFrameNumber++;
            if (SimpleControl.nextFrameNumber > SimpleControl.header.mNumFrames - 1 &&
                SimpleControl.loopMovie == 1) {
                SimpleControl.nextFrameNumber = 0;
                SimpleControl.nextFrameOffset = SimpleControl.header.mMovieDataOffsets;
                SimpleControl.nextFrameSize = SimpleControl.header.mFirstFrameSize;
            }
        }
        SimpleControl.loopMovie = loopMovie;
        SimpleControl.loadingActive = 1;
        return 1;
    }
    return 0;
}

// THPTestProc enables movie audio when playback begins or resumes from a stop.
void THPSimpleAudioStart(void) {
    SimpleControl.audioActive = 1;
}

// THPTestProc disables movie audio before stopping or restarting frame loading.
void THPSimpleAudioStop(void) {
    SimpleControl.audioActive = 0;
}

// THPTestProc calls this after stopping audio to stop loading and reset movie state.
// A pending read is passed to DVDCancel; its result is ignored before readPending is cleared.
s32 THPSimpleLoadStop(void) {
    s32 bufferIndex;

    if(SimpleControl.movieOpen != 0 && SimpleControl.audioActive == 0) {
        SimpleControl.loadingActive = 0;
        if(SimpleControl.readPending != 0) {
            DVDCancel(&SimpleControl.fileInfo.cb);
            SimpleControl.readPending = 0;
        }
        for(bufferIndex = 0; bufferIndex < 10; bufferIndex++) {
            SimpleControl.readBuffers[bufferIndex].frameReady = 0;
        }
        for(bufferIndex = 0; bufferIndex < 4; bufferIndex++) {
            SimpleControl.audioBuffers[bufferIndex].samplesRemaining = 0;
        }
        SimpleControl.videoBuffers[0].frameIndex = -1;
        SimpleControl.nextFrameOffset = SimpleControl.header.mMovieDataOffsets;
        SimpleControl.nextFrameSize = SimpleControl.header.mFirstFrameSize;
        SimpleControl.readFrameSlot = 0;
        SimpleControl.nextFrameNumber = 0;
        SimpleControl.readError = 0;
        SimpleControl.decodeFrameSlot = 0;
        SimpleControl.audioDecodeSlot = 0;
        SimpleControl.audioMixSlot = 0;
        SimpleControl.currentVolume = SimpleControl.targetVolume;
        SimpleControl.volumeRampSamples = 0;
        return 1;
    }
    return 0;
}

// THPDecodeFunc calls this once per retrace to decode the next buffered frame and audio component.
s32 THPSimpleDecode(void) {
    u8 *componentData;
    s32 *componentSizes;
    s32 interruptState;
    s32 decodedAudioSamples;
    s32 nextInterruptState;
    s32 nextFrameSlot;
    s32 componentIndex;

    if(SimpleControl.readBuffers[SimpleControl.decodeFrameSlot].frameReady != 0) {
        componentSizes = SimpleControl.readBuffers[SimpleControl.decodeFrameSlot].frameData + 2;
        componentData = (u8 *) (&SimpleControl.readBuffers[SimpleControl.decodeFrameSlot]
                                     .frameData[SimpleControl.componentInfo.mNumComponents] +
                                2);
        if(SimpleControl.hasAudio != 0) {
            if(SimpleControl.audioBuffers[SimpleControl.audioDecodeSlot].samplesRemaining == 0) {
                for (componentIndex = 0;
                     componentIndex < SimpleControl.componentInfo.mNumComponents;
                     componentData += *componentSizes, componentSizes++, componentIndex++) {
                    switch (SimpleControl.componentInfo.mFrameComp[componentIndex]) {
                        case 0:
                            if(VideoDecode(componentData) == 0) {
                                return 1;
                            }
                            break;
                        case 1:
                            decodedAudioSamples = THPAudioDecode(
                                SimpleControl.audioBuffers[SimpleControl.audioDecodeSlot]
                                    .sampleBuffer,
                                componentData, 0);
                            interruptState = OSDisableInterrupts();
                            SimpleControl.audioBuffers[SimpleControl.audioDecodeSlot]
                                .samplesRemaining = decodedAudioSamples;
                            SimpleControl.audioBuffers[SimpleControl.audioDecodeSlot].sampleCursor =
                                SimpleControl.audioBuffers[SimpleControl.audioDecodeSlot]
                                    .sampleBuffer;
                            OSRestoreInterrupts(interruptState);
                            SimpleControl.audioDecodeSlot++;
                            if(SimpleControl.audioDecodeSlot >= 4) {
                                SimpleControl.audioDecodeSlot = 0;
                            }
                            break;
                    }
                }
            } else {
                return 3;
            }
        } else {
            for (componentIndex = 0; componentIndex < SimpleControl.componentInfo.mNumComponents;
                 componentData += *componentSizes, componentSizes++, componentIndex++) {
                switch(SimpleControl.componentInfo.mFrameComp[componentIndex]) {
                    case 0:
                        if(VideoDecode(componentData) == 0) {
                            return 1;
                        }
                        break;
                }
            }
        }
        SimpleControl.readBuffers[SimpleControl.decodeFrameSlot].frameReady = 0;
        if(SimpleControl.decodeFrameSlot + 1 >= 10) {
            nextFrameSlot = 0;
        } else {
            nextFrameSlot = SimpleControl.decodeFrameSlot + 1;
        }
        SimpleControl.decodeFrameSlot = nextFrameSlot;
        nextInterruptState = OSDisableInterrupts();
        if (SimpleControl.readBuffers[SimpleControl.readFrameSlot].frameReady == 0 &&
            SimpleControl.readPending == 0 && SimpleControl.readError == 0 &&
            SimpleControl.loadingActive == 1) {
            THPSimpleInlineFunc();
        }
        OSRestoreInterrupts(nextInterruptState);
        return 0;
    } else {
        return 2;
    }
}

// THPSimpleDecode calls this for each video component to decode into the alternate YUV planes.
// The display slot advances even on decode failure. Success writes videoBuffers[0].frameIndex,
// the shared tag for the last successful frame.
static s32 VideoDecode(void *frameComponentData) {
    s32 decodeStatus;
    s32 nextVideoBuffer = (bufIdx + 1) & 1;

    decodeStatus = THPVideoDecode(
        frameComponentData, SimpleControl.videoBuffers[nextVideoBuffer].yPlane,
        SimpleControl.videoBuffers[nextVideoBuffer].cbPlane,
        SimpleControl.videoBuffers[nextVideoBuffer].crPlane, SimpleControl.decodeWork);
    bufIdx = nextVideoBuffer;
    if(decodeStatus == 0) {
        SimpleControl.videoBuffers[0].frameIndex =
            SimpleControl.readBuffers[SimpleControl.decodeFrameSlot].frameNumber;
        return 1;
    }
    return 0;
}

// THPViewFunc and THPViewSprFunc call this to draw the selected video buffer.
// A failed decode can change that selection without updating the returned frame number.
s32 THPSimpleDrawCurrentFrame(GXRenderModeObj *renderMode, GXColor *drawColor, Mtx modelView,
                              s32 originX, s32 originY, s32 drawWidth, s32 drawHeight) {
    if(SimpleControl.videoBuffers[0].frameIndex >= 0) {
        THPGXYuv2RgbSetup(renderMode, drawColor, modelView);
        THPGXYuv2RgbDraw(
            SimpleControl.videoBuffers[bufIdx].yPlane, SimpleControl.videoBuffers[bufIdx].cbPlane,
            SimpleControl.videoBuffers[bufIdx].crPlane, originX, originY,
            SimpleControl.videoInfo.width, SimpleControl.videoInfo.height, drawWidth, drawHeight);
        THPGXRestore();
        return SimpleControl.videoBuffers[0].frameIndex;
    }
    return -1;
}

// Maps the 0-127 movie volume setting to a Q15 audio multiplier.
static u16 VolumeTable[] = {
    0, 2, 8, 18, 32, 50, 73, 99,
    130, 164, 203, 245, 292, 343, 398, 457,
    520, 587, 658, 733, 812, 895, 983, 1074,
    1170, 1269, 1373, 1481, 1592, 1708, 1828, 1952,
    2080, 2212, 2348, 2488, 2632, 2781, 2933, 3090,
    3250, 3415, 3583, 3756, 3933, 4114, 4298, 4487,
    4680, 4877, 5079, 5284, 5493, 5706, 5924, 6145,
    6371, 6600, 6834, 7072, 7313, 7559, 7809, 8063,
    8321, 8583, 8849, 9119, 9394, 9672, 9954, 10241,
    10531, 10826, 11125, 11427, 11734, 12045, 12360, 12679,
    13002, 13329, 13660, 13995, 14335, 14678, 15025, 15377,
    15732, 16092, 16456, 16823, 17195, 17571, 17951, 18335,
    18723, 19115, 19511, 19911, 20316, 20724, 21136, 21553,
    21974, 22398, 22827, 23260, 23696, 24137, 24582, 25031,
    25484, 25941, 26402, 26868, 27337, 27810, 28288, 28769,
    29255, 29744, 30238, 30736, 31238, 31744, 32254, 32768
};

// THPAudioMixCallback calls this to output queued stereo movie samples, optionally adding DMA
// audio. Unfilled output retains DMA audio when supplied, otherwise it is filled with silence.
static void MixAudio(void *outputSamples, void *sourceSamples, u32 sampleCount) {
    s32 mixedSample;
    u32 chunkSamples;
    u32 samplesRemaining;
    s32 sampleIndex;
    u16 volumeScale;
    s16 *sourceSample;
    s16 *outputSample;
    s16 *movieSample;

    if(sourceSamples) {
        if (SimpleControl.movieOpen != 0 && SimpleControl.audioActive == 1 &&
            SimpleControl.hasAudio != 0) {
            samplesRemaining = sampleCount;
            outputSample = outputSamples;
            sourceSample = sourceSamples;
            while(1) {
                if(SimpleControl.audioBuffers[SimpleControl.audioMixSlot].samplesRemaining == 0) {
                    break;
                }
                if (SimpleControl.audioBuffers[SimpleControl.audioMixSlot].samplesRemaining >=
                    samplesRemaining) {
                    chunkSamples = samplesRemaining;
                } else {
                    chunkSamples =
                        SimpleControl.audioBuffers[SimpleControl.audioMixSlot].samplesRemaining;
                }
                movieSample = SimpleControl.audioBuffers[SimpleControl.audioMixSlot].sampleCursor;
                for(sampleIndex = 0; sampleIndex < chunkSamples; sampleIndex++) {
                    if(SimpleControl.volumeRampSamples != 0) {
                        SimpleControl.volumeRampSamples--;
                        SimpleControl.currentVolume += SimpleControl.volumeStep;
                    } else {
                        SimpleControl.currentVolume = SimpleControl.targetVolume;
                    }
                    // The lookup applies the current volume ramp to both stereo channels.
                    volumeScale = VolumeTable[(s32) SimpleControl.currentVolume];
                    mixedSample = *sourceSample + ((volumeScale * *movieSample) >> 15);
                    if(mixedSample < -32768) {
                        mixedSample = -32768;
                    }
                    if(mixedSample > 32767) {
                        mixedSample = 32767;
                    }
                    *outputSample = mixedSample;
                    outputSample++;
                    sourceSample++;
                    movieSample++;
                    mixedSample = *sourceSample + ((volumeScale * *movieSample) >> 15);
                    if(mixedSample < -32768) {
                        mixedSample = -32768;
                    }
                    if(mixedSample > 32767) {
                        mixedSample = 32767;
                    }
                    *outputSample = mixedSample;
                    outputSample++;
                    sourceSample++;
                    movieSample++;
                }
                samplesRemaining -= chunkSamples;
                SimpleControl.audioBuffers[SimpleControl.audioMixSlot].samplesRemaining -=
                    chunkSamples;
                SimpleControl.audioBuffers[SimpleControl.audioMixSlot].sampleCursor = movieSample;
                if(SimpleControl.audioBuffers[SimpleControl.audioMixSlot].samplesRemaining == 0) {
                    SimpleControl.audioMixSlot++;
                    if(SimpleControl.audioMixSlot >= 4) {
                        SimpleControl.audioMixSlot = 0;
                    }
                }
                if(samplesRemaining == 0) {
                    return;
                }
            }
            memcpy(outputSample, sourceSample, samplesRemaining * 4);
        } else {
            memcpy(outputSamples, sourceSamples, sampleCount * 4);
        }
    } else if (SimpleControl.movieOpen != 0 && SimpleControl.audioActive == 1 &&
               SimpleControl.hasAudio != 0) {
        samplesRemaining = sampleCount;
        outputSample = outputSamples;
        while(1) {
            if(SimpleControl.audioBuffers[SimpleControl.audioMixSlot].samplesRemaining == 0) {
                break;
            }
            if (SimpleControl.audioBuffers[SimpleControl.audioMixSlot].samplesRemaining >=
                samplesRemaining) {
                chunkSamples = samplesRemaining;
            } else {
                chunkSamples =
                    SimpleControl.audioBuffers[SimpleControl.audioMixSlot].samplesRemaining;
            }
            movieSample = SimpleControl.audioBuffers[SimpleControl.audioMixSlot].sampleCursor;
            for(sampleIndex = 0; sampleIndex < chunkSamples; sampleIndex++) {
                if(SimpleControl.volumeRampSamples != 0) {
                    SimpleControl.volumeRampSamples--;
                    SimpleControl.currentVolume += SimpleControl.volumeStep;
                } else {
                    SimpleControl.currentVolume = SimpleControl.targetVolume;
                }
                volumeScale = VolumeTable[(s32) SimpleControl.currentVolume];
                mixedSample = (volumeScale * *movieSample) >> 15;
                if(mixedSample < -32768) {
                    mixedSample = -32768;
                }
                if(mixedSample > 32767) {
                    mixedSample = 32767;
                }
                *outputSample = mixedSample;
                outputSample++;
                movieSample++;
                mixedSample = (volumeScale * *movieSample) >> 15;
                if(mixedSample < -32768) {
                    mixedSample = -32768;
                }
                if(mixedSample > 32767) {
                    mixedSample = 32767;
                }
                *outputSample = mixedSample;
                outputSample++;
                movieSample++;
            }
            samplesRemaining -= chunkSamples;
            SimpleControl.audioBuffers[SimpleControl.audioMixSlot].samplesRemaining -= chunkSamples;
            SimpleControl.audioBuffers[SimpleControl.audioMixSlot].sampleCursor = movieSample;
            if(SimpleControl.audioBuffers[SimpleControl.audioMixSlot].samplesRemaining == 0) {
                SimpleControl.audioMixSlot++;
                if(SimpleControl.audioMixSlot >= 4) {
                    SimpleControl.audioMixSlot = 0;
                }
            }
            if(samplesRemaining == 0) {
                return;
            }
        }
        memset(outputSample, 0, samplesRemaining * 4);
    } else {
        memset(outputSamples, 0, sampleCount * 4);
    }
}

// THPTestProc reads the opened movie's decoded video dimensions before allocating its buffers.
s32 THPSimpleGetVideoInfo(UnkThpStruct04 *videoInfo) {
    if(SimpleControl.movieOpen != 0) {
        memcpy(videoInfo, &SimpleControl.videoInfo, sizeof(UnkThpStruct04));
        return 1;
    }
    return 0;
}

// Returns the opened THP header's frame rate, or zero before a movie is open.
float THPSimpleGetFrameRate(void)
{
    if(SimpleControl.movieOpen != 0) {
        return SimpleControl.header.mFrameRate;
    }
    return 0;
}

// HuTHPEndCheck and HuTHPTotalFrameGet read the opened movie's frame count through this helper.
s32 THPSimpleGetTotalFrame(void) {
    if(SimpleControl.movieOpen != 0) {
        return SimpleControl.header.mNumFrames;
    }
    return 0;
}

// AI calls this at DMA boundaries to mix movie audio into a 160-frame stereo output block.
// Mode 0 has no other audio source. Mode 1 uses the DMA buffer saved at the preceding callback
// (none on the first); other modes use the buffer selected by OldAIDCallback during this call.
static void THPAudioMixCallback(void) {
    s32 interruptState;

    if(AudioSystem == 0) {
        SoundBufferIndex ^= 1;
        AIInitDMA((u32) SoundBuffer[SoundBufferIndex], sizeof(SoundBuffer[0]));
        interruptState = OSEnableInterrupts();
        MixAudio(SoundBuffer[SoundBufferIndex], 0, AUDIO_MIX_SAMPLE_COUNT);
        DCFlushRange(SoundBuffer[SoundBufferIndex], sizeof(SoundBuffer[0]));
        OSRestoreInterrupts(interruptState);
    } else {
        if(AudioSystem == 1) {
            if(LastAudioBuffer) {
                CurAudioBuffer = LastAudioBuffer;
            }
            OldAIDCallback();
            LastAudioBuffer = (u8*) AIGetDMAStartAddr() + AI_DMA_CACHED_ADDRESS_BIT_PATTERN;
        } else {
            OldAIDCallback();
            CurAudioBuffer = (u8*) AIGetDMAStartAddr() + AI_DMA_CACHED_ADDRESS_BIT_PATTERN;
        }
        SoundBufferIndex ^= 1;
        AIInitDMA((u32) SoundBuffer[SoundBufferIndex], sizeof(SoundBuffer[0]));
        interruptState = OSEnableInterrupts();
        if(CurAudioBuffer) {
            DCInvalidateRange(CurAudioBuffer, sizeof(SoundBuffer[0]));
        }
        MixAudio(SoundBuffer[SoundBufferIndex], CurAudioBuffer, AUDIO_MIX_SAMPLE_COUNT);
        DCFlushRange(SoundBuffer[SoundBufferIndex], sizeof(SoundBuffer[0]));
        OSRestoreInterrupts(interruptState);
    }
}

// HuTHPSetVolume updates an open movie with audio, clamping the level to 0-127 and duration
// to 0-60000. Zero duration applies the level immediately; otherwise the ramp advances over
// duration times 32 mixed movie sample frames.
s32 THPSimpleSetVolume(s32 targetVolume, s32 rampDuration) {
    s32 interruptState;

    if(SimpleControl.movieOpen != 0 && SimpleControl.hasAudio != 0) {
        if(targetVolume > 127) {
            targetVolume = 127;
        }
        if(targetVolume < 0) {
            targetVolume = 0;
        }
        if(rampDuration > 60000) {
            rampDuration = 60000;
        }
        if(rampDuration < 0) {
            rampDuration = 0;
        }
        interruptState = OSDisableInterrupts();
        SimpleControl.targetVolume = targetVolume;
        if(rampDuration != 0) {
            SimpleControl.volumeRampSamples = (rampDuration << 5);
            SimpleControl.volumeStep = (SimpleControl.targetVolume - SimpleControl.currentVolume) /
                                       SimpleControl.volumeRampSamples;
        } else {
            SimpleControl.volumeRampSamples = 0;
            SimpleControl.currentVolume = SimpleControl.targetVolume;
        }
        OSRestoreInterrupts(interruptState);
        return 1;
    }
    return 0;
}
