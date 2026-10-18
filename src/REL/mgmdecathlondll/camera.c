/* Decathlon Park camera setup, movement and interpolation. */
#include "REL/mgmdecathlondll/module_types.h"

#include <math.h>
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

s32 lbl_1_data_0[4] = {
    DATANUM(DATA_mgmdecathlon, 31),
    DATANUM(DATA_mgmdecathlon, 33),
    DATANUM(DATA_mgmdecathlon, 32),
    DATANUM(DATA_mgmdecathlon, 34),
};

/* Overlay setup creates the shared camera and loads the four mode-view motions. */
void fn_1_0(void) {
    s16 motionIndex;

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 20.0f, 10.0f, 5000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    motionIndex = 0;
    while (motionIndex < 4) {
        lbl_1_bss_A[motionIndex] = Hu3DMotionCreate(HuDataSelHeapReadNum(
            lbl_1_data_0[motionIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_2[motionIndex] =
            Hu3DModelCameraCreate(lbl_1_bss_A[motionIndex], 1U);
        Hu3DCameraMotionOff(lbl_1_bss_2[motionIndex]);
        motionIndex += 1;
    }
    lbl_1_bss_0 = 0;
    Hu3DCameraMotionOn(lbl_1_bss_2[0], 1U);
}

void fn_1_184(void)
{
}

void Hu3DCameraMotionOff(s16 cameraId);
void HuPrcVSleep();
void fn_1_264(s16 modelId, s16 cameraMask, f32 durationFrames, s16 easingMode);
void fn_1_310(s16 modelId, s16 cameraMask, Vec *initialPosition, Vec *initialUp,
              Vec *initialTarget, f32 durationFrames, s16 easingMode);
void fn_1_1AC8(s16 modelId, const u16 *trackWords, f32 channelValue, CAM_MOTION_WORK *output);

/* The mode sequence changes between its four camera presets, waiting for the view to move. */
void fn_1_188(s16 presetNo) {
    s16 cameraNo;

    if ((presetNo < 0) || (presetNo >= 4)) {
        return;
    }
    if (lbl_1_bss_0 != presetNo) {
        cameraNo = 0;
        while (cameraNo < 4) {
            Hu3DCameraMotionOff(lbl_1_bss_2[cameraNo]);
            cameraNo++;
        }
        fn_1_264(lbl_1_bss_2[presetNo], 1, 30.0f, 0);
        lbl_1_bss_0 = presetNo;
        HuPrcVSleep();
    }
}

/* Camera selection moves the first camera in the mask along the selected model's motion path. */
void fn_1_264(s16 modelId, s16 cameraMask, f32 durationFrames, s16 easingMode)
{
    s16 cameraNo = 0;
    HU3D_CAMERA *camera;

    while (cameraNo < 16) {
        if (cameraMask & (1 << cameraNo)) {
            break;
        }
        cameraNo++;
    }
    camera = &Hu3DCamera[cameraNo];
    fn_1_310(modelId, cameraMask, &camera->pos, &camera->up, &camera->target,
             durationFrames, easingMode);
}

#define CAM_PATH_LINEAR 0
#define CAM_PATH_EASE_OUT 1
#define CAM_PATH_EASE_IN 2
#define CAMERA_FLOAT_EXPONENT_MASK 0x7F800000
#define CAMERA_FLOAT_FRACTION_MASK 0x007FFFFF
#define CAMERA_VALUE_NAN 1
#define CAMERA_VALUE_INFINITY 2
#define CAMERA_VALUE_ZERO 3
#define CAMERA_VALUE_NORMAL 4
#define CAMERA_VALUE_SUBNORMAL 5

/* Camera-path interpolation uses this to distinguish finite values from NaN and infinity. */
static inline s32 DecathlonCameraValueClass(const f32 *value)
{
    union {
        f32 value; /* camera-channel value to classify */
        s32 bits; /* same value's floating-point representation */
    } representation;

    representation.value = *value;
    switch (representation.bits & CAMERA_FLOAT_EXPONENT_MASK) {
    case CAMERA_FLOAT_EXPONENT_MASK:
        if ((representation.bits & CAMERA_FLOAT_FRACTION_MASK) != 0) {
            return CAMERA_VALUE_NAN;
        }
        return CAMERA_VALUE_INFINITY;
    case 0:
        if ((representation.bits & CAMERA_FLOAT_FRACTION_MASK) != 0) {
            return CAMERA_VALUE_SUBNORMAL;
        }
        return CAMERA_VALUE_ZERO;
    default:
        return CAMERA_VALUE_NORMAL;
    }
}

/* Camera-path interpolation calls this to replace NaN and infinity with a neighboring sample. */
static inline void DecathlonCameraFiniteValue(f32 *value, const f32 *fallback)
{
    if (DecathlonCameraValueClass(value) == CAMERA_VALUE_NAN) {
        *value = *fallback;
    }
    if (DecathlonCameraValueClass(value) == CAMERA_VALUE_INFINITY) {
        *value = *fallback;
    }
}

/* fn_1_264 calls this to sample and retime the motion's camera tracks over the requested
 * duration. */
void fn_1_310(s16 modelId, s16 cameraMask, Vec *initialPosition, Vec *initialUp,
              Vec *initialTarget, f32 durationFrames, s16 easingMode)
{
    HU3D_MODEL *model = &Hu3DData[modelId];
    HU3D_MOTION *motion = &Hu3DMotion[model->motId];
    HSF_DATA *modelData = model->hsf;
    HSF_DATA  *motionData = motion->hsf;
    HSF_MOTION *motionTracks = motionData->motion;
    s16 intervalIndex;
    s16 sampleCount;

    HSF_TRACK *track;
    HSF_TRACK *tracksEnd;
    s16 cameraIndex;
    HU3D_CAMERA *camera;
    float frame;
    float easedFrame;
    float motionEndFrame;
    float pathDistance;
    float pathLength;
    HuVecF segmentOffset;
    float channelSampleValues[4];
    float surroundingSampleTimes[4];
    s16 sampleCapacity;
    CAM_MOTION_WORK *samples;
    CAM_MOTION_WORK *sample;
    HSF_OBJECT *cameraObject;
    for(cameraIndex=0; cameraIndex<HU3D_CAM_MAX; cameraIndex++) {
        if(cameraMask & (1 << cameraIndex)) {
            break;
        }
    }
    camera = &Hu3DCamera[cameraIndex];
    motionEndFrame = motionTracks->maxTime;
    sampleCapacity = 1+(motionEndFrame/6.0)+1;

    sample = samples = HuMemDirectMallocNum(
        HEAP_HEAP, (sampleCapacity + 1) * sizeof(CAM_MOTION_WORK), HU_MEMNUM_OVL);
    sample->time = 0;
    sample->pos = *initialPosition;
    sample->target = *initialTarget;
    sample->up = *initialUp;
    sample++;
    for(sampleCount=1, frame=0; frame<=motionEndFrame; frame += 6, sample++, sampleCount++) {
        sample->time = frame;
        track = motionTracks->track;
        tracksEnd = &track[motionTracks->numTracks];
        if(!(model->attr & HU3D_ATTR_CAMERA)) {
            for(; track<tracksEnd; track++) {
                if(track->type == HSF_TRACK_TRANSFORM) {
                    cameraObject = &modelData->object[track->target];
                    if(cameraObject->type == HSF_OBJ_CAMERA) {
                        fn_1_1AC8(modelId, (const u16 *)track, GetCurve(track, frame), sample);
                    }
                }
            }
        } else {
            for(; track<tracksEnd; track++) {
                if(track->type == HSF_TRACK_TRANSFORM) {
                   if(track->index ==  HSF_OBJ_CAMERA) {
                       fn_1_1AC8(modelId, (const u16 *)track, GetCurve(track, frame), sample);
                   }
                }
            }
        }
    }
    /* The loop has advanced past its last sample, so append the endpoint even when the
     * six-frame sampling already included it. */
    if(frame != motionEndFrame) {
        sample->time = motionEndFrame;
        track = motionTracks->track;
        tracksEnd = &track[motionTracks->numTracks];
        if(!(model->attr & HU3D_ATTR_CAMERA)) {
            for(; track<tracksEnd; track++) {
                if(track->type == HSF_TRACK_TRANSFORM) {
                    cameraObject = &modelData->object[track->target];
                    if(cameraObject->type == HSF_OBJ_CAMERA) {
                        fn_1_1AC8(modelId, (const u16 *) track, GetCurve(track, motionEndFrame),
                                  sample);
                    }
                }
            }
        } else {
            for(; track<tracksEnd; track++) {
                if(track->type == HSF_TRACK_TRANSFORM) {
                   if(track->index ==  HSF_OBJ_CAMERA) {
                       fn_1_1AC8(modelId, (const u16 *) track, GetCurve(track, motionEndFrame),
                                 sample);
                   }
                }
            }
        }
        sampleCount++;
    }
    // The supplied starting view has no FOV parameter, so use the track's first sampled FOV.
    samples[0].fov = samples[1].fov;
    sample = samples;
    sample[0].dist = 0;
    for(frame=pathLength=0; frame<sampleCount-1; frame++, sample++) {
        VECSubtract(&sample[1].pos, &sample[0].pos, &segmentOffset);
        sample[1].dist = VECMag(&segmentOffset);
        pathLength += sample[1].dist;
    }
    sample = samples;
    // Retiming assumes the sampled camera position travels a nonzero distance.
    for(frame=pathDistance=0; frame<sampleCount; frame++, sample++) {
        pathDistance += sample->dist;
        sample->time = durationFrames*(pathDistance/pathLength);
    }
    for(frame=0; frame<=durationFrames; frame++) {
        switch(easingMode) {
            case CAM_PATH_LINEAR:
                easedFrame = frame;
                break;

            case CAM_PATH_EASE_OUT:
                easedFrame = durationFrames*HuSin((frame/durationFrames)*90);
                break;

            case CAM_PATH_EASE_IN:
                easedFrame = durationFrames*(1-HuCos((frame/durationFrames)*90));
                break;
        }
        for(sample=samples, intervalIndex=0; intervalIndex<sampleCount; intervalIndex++, sample++) {
            if(sample->time <= easedFrame && sample[1].time > easedFrame) {
                break;
            }

        }
        if(intervalIndex == sampleCount) {
            break;
        }
        if(intervalIndex == 0) {
            surroundingSampleTimes[0] = -1;
        } else {
            surroundingSampleTimes[0] = sample[-1].time;
        }
        surroundingSampleTimes[1] = sample[0].time;
        if(intervalIndex >= sampleCount-1) {
            surroundingSampleTimes[2] = 1+sample[0].time;
        }
        // This next-sample time overwrites the fallback assigned above.
        surroundingSampleTimes[2] = sample[1].time;
        if(intervalIndex >= sampleCount-2) {
            surroundingSampleTimes[3] = 1+surroundingSampleTimes[2];
        } else {
            surroundingSampleTimes[3] = sample[2].time;
        }
        if(intervalIndex == 0) {
            channelSampleValues[0] = sample[0].pos.x;
        } else {
            channelSampleValues[0] = sample[-1].pos.x;
        }
        channelSampleValues[1] = sample[0].pos.x;
        if(intervalIndex >= sampleCount-1) {
            channelSampleValues[2] = channelSampleValues[1];
        } else {
            channelSampleValues[2] = sample[1].pos.x;
        }
        if(intervalIndex >= sampleCount-2) {
            channelSampleValues[3] = channelSampleValues[2];
        } else {
            channelSampleValues[3] = sample[2].pos.x;
        }
        camera->pos.x = InterpolateBMLine(channelSampleValues, surroundingSampleTimes, easedFrame);
        DecathlonCameraFiniteValue(&camera->pos.x, &channelSampleValues[2]);
        if(intervalIndex == 0) {
            channelSampleValues[0] = sample[0].pos.y;
        } else {
            channelSampleValues[0] = sample[-1].pos.y;
        }
        channelSampleValues[1] = sample[0].pos.y;
        if(intervalIndex >= sampleCount-1) {
            channelSampleValues[2] = channelSampleValues[1];
        } else {
            channelSampleValues[2] = sample[1].pos.y;
        }
        if(intervalIndex >= sampleCount-2) {
            channelSampleValues[3] = channelSampleValues[2];
        } else {
            channelSampleValues[3] = sample[2].pos.y;
        }
        camera->pos.y = InterpolateBMLine(channelSampleValues, surroundingSampleTimes, easedFrame);
        DecathlonCameraFiniteValue(&camera->pos.y, &channelSampleValues[2]);
        if(intervalIndex == 0) {
            channelSampleValues[0] = sample[0].pos.z;
        } else {
            channelSampleValues[0] = sample[-1].pos.z;
        }
        channelSampleValues[1] = sample[0].pos.z;
        if(intervalIndex >= sampleCount-1) {
            channelSampleValues[2] = channelSampleValues[1];
        } else {
            channelSampleValues[2] = sample[1].pos.z;
        }
        if(intervalIndex >= sampleCount-2) {
            channelSampleValues[3] = channelSampleValues[2];
        } else {
            channelSampleValues[3] = sample[2].pos.z;
        }
        camera->pos.z = InterpolateBMLine(channelSampleValues, surroundingSampleTimes, easedFrame);
        DecathlonCameraFiniteValue(&camera->pos.z, &channelSampleValues[2]);
        if(intervalIndex == 0) {
            channelSampleValues[0] = sample[0].target.x;
        } else {
            channelSampleValues[0] = sample[-1].target.x;
        }
        channelSampleValues[1] = sample[0].target.x;
        if(intervalIndex >= sampleCount-1) {
            channelSampleValues[2] = channelSampleValues[1];
        } else {
            channelSampleValues[2] = sample[1].target.x;
        }
        if(intervalIndex >= sampleCount-2) {
            channelSampleValues[3] = channelSampleValues[2];
        } else {
            channelSampleValues[3] = sample[2].target.x;
        }
        camera->target.x =
            InterpolateBMLine(channelSampleValues, surroundingSampleTimes, easedFrame);
        DecathlonCameraFiniteValue(&camera->target.x, &channelSampleValues[2]);
        if(intervalIndex == 0) {
            channelSampleValues[0] = sample[0].target.y;
        } else {
            channelSampleValues[0] = sample[-1].target.y;
        }
        channelSampleValues[1] = sample[0].target.y;
        if(intervalIndex >= sampleCount-1) {
            channelSampleValues[2] = channelSampleValues[1];
        } else {
            channelSampleValues[2] = sample[1].target.y;
        }
        if(intervalIndex >= sampleCount-2) {
            channelSampleValues[3] = channelSampleValues[2];
        } else {
            channelSampleValues[3] = sample[2].target.y;
        }
        camera->target.y =
            InterpolateBMLine(channelSampleValues, surroundingSampleTimes, easedFrame);
        DecathlonCameraFiniteValue(&camera->target.y, &channelSampleValues[2]);
        if(intervalIndex == 0) {
            channelSampleValues[0] = sample[0].target.z;
        } else {
            channelSampleValues[0] = sample[-1].target.z;
        }
        channelSampleValues[1] = sample[0].target.z;
        if(intervalIndex >= sampleCount-1) {
            channelSampleValues[2] = channelSampleValues[1];
        } else {
            channelSampleValues[2] = sample[1].target.z;
        }
        if(intervalIndex >= sampleCount-2) {
            channelSampleValues[3] = channelSampleValues[2];
        } else {
            channelSampleValues[3] = sample[2].target.z;
        }
        camera->target.z =
            InterpolateBMLine(channelSampleValues, surroundingSampleTimes, easedFrame);
        DecathlonCameraFiniteValue(&camera->target.z, &channelSampleValues[2]);
        if(intervalIndex == 0) {
            channelSampleValues[0] = sample[0].up.x;
        } else {
            channelSampleValues[0] = sample[-1].up.x;
        }
        channelSampleValues[1] = sample[0].up.x;
        if(intervalIndex >= sampleCount-1) {
            channelSampleValues[2] = channelSampleValues[1];
        } else {
            channelSampleValues[2] = sample[1].up.x;
        }
        if(intervalIndex >= sampleCount-2) {
            channelSampleValues[3] = channelSampleValues[2];
        } else {
            channelSampleValues[3] = sample[2].up.x;
        }
        camera->up.x = InterpolateBMLine(channelSampleValues, surroundingSampleTimes, easedFrame);
        DecathlonCameraFiniteValue(&camera->up.x, &channelSampleValues[2]);
        if(intervalIndex == 0) {
            channelSampleValues[0] = sample[0].up.y;
        } else {
            channelSampleValues[0] = sample[-1].up.y;
        }
        channelSampleValues[1] = sample[0].up.y;
        if(intervalIndex >= sampleCount-1) {
            channelSampleValues[2] = channelSampleValues[1];
        } else {
            channelSampleValues[2] = sample[1].up.y;
        }
        if(intervalIndex >= sampleCount-2) {
            channelSampleValues[3] = channelSampleValues[2];
        } else {
            channelSampleValues[3] = sample[2].up.y;
        }
        camera->up.y = InterpolateBMLine(channelSampleValues, surroundingSampleTimes, easedFrame);
        DecathlonCameraFiniteValue(&camera->up.y, &channelSampleValues[2]);
        if(intervalIndex == 0) {
            channelSampleValues[0] = sample[0].up.z;
        } else {
            channelSampleValues[0] = sample[-1].up.z;
        }
        channelSampleValues[1] = sample[0].up.z;
        if(intervalIndex >= sampleCount-1) {
            channelSampleValues[2] = channelSampleValues[1];
        } else {
            channelSampleValues[2] = sample[1].up.z;
        }
        if(intervalIndex >= sampleCount-2) {
            channelSampleValues[3] = channelSampleValues[2];
        } else {
            channelSampleValues[3] = sample[2].up.z;
        }
        camera->up.z = InterpolateBMLine(channelSampleValues, surroundingSampleTimes, easedFrame);
        DecathlonCameraFiniteValue(&camera->up.z, &channelSampleValues[2]);
        if(intervalIndex == 0) {
            channelSampleValues[0] = sample[0].fov;
        } else {
            channelSampleValues[0] = sample[-1].fov;
        }
        channelSampleValues[1] = sample[0].fov;
        if(intervalIndex >= sampleCount-1) {
            channelSampleValues[2] = channelSampleValues[1];
        } else {
            channelSampleValues[2] = sample[1].fov;
        }
        if(intervalIndex >= sampleCount-2) {
            channelSampleValues[3] = channelSampleValues[2];
        } else {
            channelSampleValues[3] = sample[2].fov;
        }
        camera->fov = InterpolateBMLine(channelSampleValues, surroundingSampleTimes, easedFrame);
        DecathlonCameraFiniteValue(&camera->fov, &channelSampleValues[2]);
        HuPrcVSleep();
    }
    HuMemDirectFree(samples);
}

/* Path sampling applies one evaluated track channel to its camera sample, including camera roll. */
void fn_1_1AC8(s16 modelId, const u16 *trackWords, f32 channelValue,
               CAM_MOTION_WORK *output) {
    HuVecF rolledUp;
    HuVecF viewDirection;
    HU3D_MODEL *modelData;

    modelData = &Hu3DData[modelId];
    switch (trackWords[3]) {
    case HSF_CHANNEL_POSX:
        /* Position and target channels scale both the sampled value and the model's offset. */
        output->pos.x = modelData->scale.x * (channelValue + modelData->pos.x);
        break;
    case HSF_CHANNEL_POSY:
        output->pos.y = modelData->scale.y * (channelValue + modelData->pos.y);
        break;
    case HSF_CHANNEL_POSZ:
        output->pos.z = modelData->scale.z * (channelValue + modelData->pos.z);
        break;
    case HSF_CHANNEL_TARGETX:
        output->target.x = modelData->scale.x * (channelValue + modelData->pos.x);
        break;
    case HSF_CHANNEL_TARGETY:
        output->target.y = modelData->scale.y * (channelValue + modelData->pos.y);
        break;
    case HSF_CHANNEL_TARGETZ:
        output->target.z = modelData->scale.z * (channelValue + modelData->pos.z);
        break;
    case HSF_CHANNEL_FOV:
        output->fov = channelValue;
        break;
    case HSF_CHANNEL_UPROT:
        PSVECSubtract(&output->pos, &output->target, &viewDirection);
        PSVECNormalize(&viewDirection, &viewDirection);
        rolledUp.x = (viewDirection.x * viewDirection.y) *
                         (1.0 - cos((3.141592653589793 * channelValue) / 180.0)) -
                     (viewDirection.z * sin((3.141592653589793 * channelValue) / 180.0));
        rolledUp.y = (viewDirection.y * viewDirection.y) +
                            (1.0f - (viewDirection.y * viewDirection.y)) *
                            cos((3.141592653589793 * channelValue) / 180.0);
        rolledUp.z = (viewDirection.y * viewDirection.z) *
                         (1.0 - cos((3.141592653589793 * channelValue) / 180.0)) +
                     (sin((3.141592653589793 * channelValue) / 180.0) * viewDirection.x);
        PSVECNormalize(&rolledUp, &output->up);
        break;
    }
}

s16 lbl_1_bss_A[4];
s16 lbl_1_bss_2[4];
s16 lbl_1_bss_0;
