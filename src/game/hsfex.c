// Camera paths, screen projection, and transform extraction used by game and menu scenes.
#include "game/hsfex.h"
#include "game/hu3d.h"
#include "game/disp.h"

#include "math.h"

#define DISP_HALF_W (HU_DISP_WIDTH/2.0f)
#define DISP_HALF_H (HU_DISP_HEIGHT/2.0f)

#define CAM_PATH_LINEAR 0
#define CAM_PATH_EASE_OUT 1
#define CAM_PATH_EASE_IN 2

// CamMotionEx and CamMotionExPathGet use this while sampling a camera transform track.
static void SetObjCamMotion(HU3D_MODELID modelId, HSF_TRACK *cameraTrack, float channelValue,
                            CAM_MOTION_WORK *sample);

// Camera-motion entry point: start from the current view of the first camera selected by the mask.
// CamMotionEx runs synchronously and yields while playing the model's camera track.
void CamMotionEx2(HU3D_MODELID modelId, s16 cameraMask, float durationFrames, s16 easingMode)
{
    s16 cameraIndex;
    HU3D_CAMERA *camera;
    for(cameraIndex=0; cameraIndex<HU3D_CAM_MAX; cameraIndex++) {
        if(cameraMask & (1 << cameraIndex)) {
            break;
        }
    }
    camera = &Hu3DCamera[cameraIndex];
    CamMotionEx(modelId, cameraMask, &camera->pos, &camera->up, &camera->target, durationFrames,
                easingMode);
}

// CamMotionEx2 and camera sequences use this to play a model's camera track over durationFrames.
// Seed the supplied view, sample every six motion frames, then append the exact endpoint before
// timing the path by traveled distance. This duplicates the endpoint when the sample loop already
// reached it.
void CamMotionEx(HU3D_MODELID modelId, s16 cameraMask, HuVecF *initialPosition, HuVecF *initialUp,
                 HuVecF *initialTarget, float durationFrames, s16 easingMode)
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
                        SetObjCamMotion(modelId, track, GetCurve(track, frame), sample);
                    }
                }
            }
        } else {
            for(; track<tracksEnd; track++) {
                if(track->type == HSF_TRACK_TRANSFORM) {
                   if(track->index ==  HSF_OBJ_CAMERA) {
                       SetObjCamMotion(modelId, track, GetCurve(track, frame), sample);
                   }
                }
            }
        }
    }
    if(frame != motionEndFrame) {
        sample->time = motionEndFrame;
        track = motionTracks->track;
        tracksEnd = &track[motionTracks->numTracks];
        if(!(model->attr & HU3D_ATTR_CAMERA)) {
            for(; track<tracksEnd; track++) {
                if(track->type == HSF_TRACK_TRANSFORM) {
                    cameraObject = &modelData->object[track->target];
                    if(cameraObject->type == HSF_OBJ_CAMERA) {
                        SetObjCamMotion(modelId, track, GetCurve(track, motionEndFrame), sample);
                    }
                }
            }
        } else {
            for(; track<tracksEnd; track++) {
                if(track->type == HSF_TRACK_TRANSFORM) {
                   if(track->index ==  HSF_OBJ_CAMERA) {
                       SetObjCamMotion(modelId, track, GetCurve(track, motionEndFrame), sample);
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
        HuPrcVSleep();
    }
    HuMemDirectFree(samples);
}

// CamMotionEx and CamMotionExPathGet use this while sampling a camera transform track.
static void SetObjCamMotion(HU3D_MODELID modelId, HSF_TRACK *cameraTrack, float channelValue,
                            CAM_MOTION_WORK *sample)
{
    HU3D_MODEL *model = &Hu3DData[modelId];
    Vec cameraUp;
    Vec viewDirection;

    // Position and target apply scale to the sum of the channel value and model translation.
    switch (cameraTrack->channel) {
        case HSF_CHANNEL_POSX:
            sample->pos.x = model->scale.x * (channelValue + model->pos.x);
            break;

        case HSF_CHANNEL_POSY:
            sample->pos.y = model->scale.y * (channelValue + model->pos.y);
            break;

        case HSF_CHANNEL_POSZ:
            sample->pos.z = model->scale.z * (channelValue + model->pos.z);
            break;

        case HSF_CHANNEL_TARGETX:
            sample->target.x = model->scale.x * (channelValue + model->pos.x);
            break;

        case HSF_CHANNEL_TARGETY:
            sample->target.y = model->scale.y * (channelValue + model->pos.y);
            break;

        case HSF_CHANNEL_TARGETZ:
            sample->target.z = model->scale.z * (channelValue + model->pos.z);
            break;

        case HSF_CHANNEL_FOV:
            sample->fov = channelValue;
            break;

        case HSF_CHANNEL_UPROT:
            // Rotate world-up around the direction from the target to the camera.
            VECSubtract(&sample->pos, &sample->target, &viewDirection);
            VECNormalize(&viewDirection, &viewDirection);
            cameraUp.x = viewDirection.x * viewDirection.y * (1.0 - HuCos(channelValue)) -
                         viewDirection.z * HuSin(channelValue);
            cameraUp.y = viewDirection.y * viewDirection.y +
                         (1.0f - viewDirection.y * viewDirection.y) * HuCos(channelValue);
            cameraUp.z = viewDirection.y * viewDirection.z * (1.0 - HuCos(channelValue)) +
                         viewDirection.x * HuSin(channelValue);
            VECNormalize(&cameraUp, &sample->up);
            break;
    }
}

// Prepare the model's camera samples for CamMotionExPath, retiming by position distance and easing.
// Return the sample count, or -1 when the caller's buffer capacity is too small.
int CamMotionExPathGet(HU3D_MODELID modelId, s16 sampleFrameStep, float durationFrames,
                       s16 easingMode, CAM_MOTION_WORK **sampleBuffer, int *sampleCapacity)
{
    HU3D_MODEL *model = &Hu3DData[modelId];
    HU3D_MOTION *motion = &Hu3DMotion[model->motId];
    HSF_DATA *modelData = model->hsf;
    HSF_DATA  *motionData = motion->hsf;
    HSF_MOTION *motionTracks = motionData->motion;

    HSF_TRACK *track;
    HSF_TRACK *tracksEnd;
    HSF_OBJECT *cameraObject;
    CAM_MOTION_WORK *sample;
    s16 sampleCount;

    float frame;
    float motionEndFrame;

    float *segmentLengths;
    float *segmentLength;
    float pathLength;
    float pathDistance;

    s16 requiredSampleCount;

    motionEndFrame = motionTracks->maxTime;
    requiredSampleCount = 1+(motionEndFrame/sampleFrameStep)+1;
    // A null buffer handle enters a branch that then writes through the same pointer.
    if(!sampleBuffer) {
        *sampleBuffer = HuMemDirectMallocNum(
            HEAP_HEAP, (requiredSampleCount + 1) * sizeof(CAM_MOTION_WORK), HU_MEMNUM_OVL);
        *sampleCapacity = requiredSampleCount+1;
    } else {
        if(*sampleCapacity < requiredSampleCount+1) {
            return -1;
        }
    }
    for (sample = *sampleBuffer, sampleCount = 0, frame = 0; frame <= motionEndFrame;
         frame += sampleFrameStep, sample++, sampleCount++) {
        sample->time = frame;
        track = motionTracks->track;
        tracksEnd = &track[motionTracks->numTracks];
        if(!(model->attr & HU3D_ATTR_CAMERA)) {
            for(; track<tracksEnd; track++) {
                if(track->type == HSF_TRACK_TRANSFORM) {
                    cameraObject = &modelData->object[track->target];
                    if(cameraObject->type == HSF_OBJ_CAMERA) {
                        SetObjCamMotion(modelId, track, GetCurve(track, frame), sample);
                    }
                }
            }
        } else {
            for(; track<tracksEnd; track++) {
                if(track->type == HSF_TRACK_TRANSFORM) {
                   if(track->index ==  HSF_OBJ_CAMERA) {
                       SetObjCamMotion(modelId, track, GetCurve(track, frame), sample);
                   }
                }
            }
        }
    }
    // Append the exact endpoint after the sample loop, even when the loop already sampled it.
    if(frame != motionEndFrame) {
        sample->time = motionEndFrame;
        track = motionTracks->track;
        tracksEnd = &track[motionTracks->numTracks];
        if(!(model->attr & HU3D_ATTR_CAMERA)) {
            for(; track<tracksEnd; track++) {
                if(track->type == HSF_TRACK_TRANSFORM) {
                    cameraObject = &modelData->object[track->target];
                    if(cameraObject->type == HSF_OBJ_CAMERA) {
                        SetObjCamMotion(modelId, track, GetCurve(track, motionEndFrame), sample);
                    }
                }
            }
        } else {
            for(; track<tracksEnd; track++) {
                if(track->type == HSF_TRACK_TRANSFORM) {
                   if(track->index ==  HSF_OBJ_CAMERA) {
                       SetObjCamMotion(modelId, track, GetCurve(track, motionEndFrame), sample);
                   }
                }
            }
        }
        sampleCount++;
    }
    segmentLengths = HuMemDirectMallocNum(HEAP_HEAP, sampleCount*4, HU_MEMNUM_OVL);
    sample = *sampleBuffer;
    segmentLength = segmentLengths;
    segmentLength[0] = 0;
    for(pathLength=0, frame=0; frame<sampleCount-1; frame++, sample++, segmentLength++) {
        HuVecF segmentOffset;
        VECSubtract(&sample[1].pos, &sample[0].pos, &segmentOffset);
        segmentLength[1] = VECMag(&segmentOffset);
        pathLength += segmentLength[1];
    }
    // A stationary path keeps its original motion times instead of using durationFrames.
    if(fabsf(pathLength) > 0.0001) {
        sample = *sampleBuffer;
        segmentLength = segmentLengths;
        for(pathDistance=0, frame=0; frame<sampleCount; frame++, sample++, segmentLength++) {
            pathDistance += segmentLength[0];
            sample->time = durationFrames*(pathDistance/pathLength);
            switch(easingMode) {
                case CAM_PATH_LINEAR:
                    break;

                case CAM_PATH_EASE_OUT:
                    sample->time = durationFrames*HuSin((sample->time/durationFrames)*90);
                    break;

                case CAM_PATH_EASE_IN:
                    sample->time = durationFrames*(1-HuCos((sample->time/durationFrames)*90));
                    break;
            }
        }
    }
    HuMemDirectFree(segmentLengths);
    return sampleCount;
}

// Camera sequences call this to play a prepared path into the first camera selected by the mask.
// Interpolate all ten view channels and yield once after each displayed path frame.
void CamMotionExPath(s16 cameraMask, CAM_MOTION_WORK *samples, int sampleCount)
{
    CAM_MOTION_WORK *sample;
    s16 selectedIndex, channelIndex;
    HU3D_CAMERA *camera;
    float frame;
    float pathEndFrame;

    enum { CAM_PATH_SAMPLE_FLOAT_STRIDE = sizeof(CAM_MOTION_WORK) / sizeof(float) };

    float *cameraChannels[10];
    float *sampleChannels[10];
    float channelSampleValues[4];
    float surroundingSampleTimes[4];

    for(selectedIndex=0; selectedIndex<HU3D_CAM_MAX; selectedIndex++) {
        if(cameraMask & (1 << selectedIndex)) {
            camera = &Hu3DCamera[selectedIndex];
            break;
        }
    }
    pathEndFrame = samples[sampleCount-1].time;
    cameraChannels[0] = &camera->fov;
    cameraChannels[1] = &camera->pos.x;
    cameraChannels[2] = &camera->pos.y;
    cameraChannels[3] = &camera->pos.z;
    cameraChannels[4] = &camera->up.x;
    cameraChannels[5] = &camera->up.y;
    cameraChannels[6] = &camera->up.z;
    cameraChannels[7] = &camera->target.x;
    cameraChannels[8] = &camera->target.y;
    cameraChannels[9] = &camera->target.z;
    for(frame=0; frame<=pathEndFrame; frame++) {
        sample = samples;
        for(selectedIndex=0; selectedIndex<sampleCount; selectedIndex++, sample++) {
            if(sample[0].time <= frame && sample[1].time > frame) {
                break;
            }
        }
        if(selectedIndex == sampleCount) {
            break;
        }
        sampleChannels[0] = &sample->fov;
        sampleChannels[1] = &sample->pos.x;
        sampleChannels[2] = &sample->pos.y;
        sampleChannels[3] = &sample->pos.z;
        sampleChannels[4] = &sample->up.x;
        sampleChannels[5] = &sample->up.y;
        sampleChannels[6] = &sample->up.z;
        sampleChannels[7] = &sample->target.x;
        sampleChannels[8] = &sample->target.y;
        sampleChannels[9] = &sample->target.z;
        if(selectedIndex == 0) {
            surroundingSampleTimes[0] = -1;
        } else {
            surroundingSampleTimes[0] = sample[-1].time;
        }
        surroundingSampleTimes[1] = sample[0].time;
        if(selectedIndex >= sampleCount-1) {
            surroundingSampleTimes[2] = 1+surroundingSampleTimes[1];
        } else {
            surroundingSampleTimes[2] = sample[1].time;
        }
        if(selectedIndex >= sampleCount-2) {
            surroundingSampleTimes[3] = 1+surroundingSampleTimes[2];
        } else {
            surroundingSampleTimes[3] = sample[2].time;
        }
        // Each float channel keeps its offset across the float-only camera sample records.
        for(channelIndex=10; channelIndex-- != 0;) {
            if(selectedIndex == 0) {
                channelSampleValues[0] = *sampleChannels[channelIndex];
            } else {
                channelSampleValues[0] =
                    sampleChannels[channelIndex][-CAM_PATH_SAMPLE_FLOAT_STRIDE];
            }
            channelSampleValues[1] = *sampleChannels[channelIndex];
            if(selectedIndex >= sampleCount-1) {
                channelSampleValues[2] = channelSampleValues[1];
            } else {
                channelSampleValues[2] =
                    sampleChannels[channelIndex][CAM_PATH_SAMPLE_FLOAT_STRIDE];
            }
            if(selectedIndex >= sampleCount-2) {
                channelSampleValues[3] = channelSampleValues[2];
            } else {
                channelSampleValues[3] =
                    sampleChannels[channelIndex][2 * CAM_PATH_SAMPLE_FLOAT_STRIDE];
            }
            *cameraChannels[channelIndex] =
                InterpolateBMLine(channelSampleValues, surroundingSampleTimes, frame);
        }
        HuPrcVSleep();
    }
}

// Camera path playback uses four surrounding samples to interpolate one channel at pathTime.
// Estimate endpoint slopes, then interpolate through the midpoint using two quadratic segments.
float InterpolateBMLine(float *channelSampleValues, float *sampleTimes, float pathTime)
{
    float timeDeltas[3];
    float sampleSlopes[2];
    float crossingTime;
    float intervalEndValue;
    float leftAdjustedSlope;
    float midpointTime;
    float halfInterval;
    float midpointValue;
    float midpointSlope;
    float intervalStartValue;
    float intervalStartSlope;
    float intervalEndTime;
    float intervalEndSlope;
    float intervalStartTime;
    s32 hasCrossing;
    s32 sampleIndex;

    if (channelSampleValues[0] == channelSampleValues[1] &&
        channelSampleValues[0] == channelSampleValues[2] &&
        channelSampleValues[0] == channelSampleValues[3]) {
        return channelSampleValues[0];
    }
    // Coincident sample times use a small interval when calculating the neighboring slopes.
    timeDeltas[0] = (sampleTimes[0] != sampleTimes[1]) ? (sampleTimes[1]-sampleTimes[0]) : 0.001;
    timeDeltas[1] = (sampleTimes[1] != sampleTimes[2]) ? (sampleTimes[2]-sampleTimes[1]) : 0.001;
    timeDeltas[2] = (sampleTimes[2] != sampleTimes[3]) ? (sampleTimes[3]-sampleTimes[2]) : 0.001;
    for(sampleIndex=1; sampleIndex<=2; sampleIndex++) {
        sampleSlopes[sampleIndex - 1] =
            0.5f * ((channelSampleValues[sampleIndex] - channelSampleValues[sampleIndex - 1]) /
                        timeDeltas[sampleIndex - 1] +
                    (channelSampleValues[sampleIndex + 1] - channelSampleValues[sampleIndex]) /
                        timeDeltas[sampleIndex]);
    }
    midpointTime = 0.5f * (sampleTimes[2] + sampleTimes[1]);
    hasCrossing = 0;
    if (sampleSlopes[1] - sampleSlopes[0] != 0.0f) {
        crossingTime = (sampleSlopes[1] * sampleTimes[2] - sampleSlopes[0] * sampleTimes[1] -
                        (channelSampleValues[2] - channelSampleValues[1])) /
                       (sampleSlopes[1] - sampleSlopes[0]);
        hasCrossing =
            (sampleTimes[1] <= crossingTime && crossingTime <= sampleTimes[2]) ? TRUE : FALSE;
    }
    // Crossing tangent lines within the interval select the curved midpoint estimate.
    if (hasCrossing == 1) {
        halfInterval = midpointTime - sampleTimes[1];
        leftAdjustedSlope = (channelSampleValues[2] - channelSampleValues[1]) / timeDeltas[1] -
                            (sampleSlopes[1] - sampleSlopes[0]) / 2;
        midpointValue =
            channelSampleValues[1] +
            ((halfInterval *
              (halfInterval * ((sampleSlopes[1] - sampleSlopes[0]) / (2.0f * timeDeltas[1])))) +
             (leftAdjustedSlope * halfInterval));
        midpointSlope = leftAdjustedSlope +
                        (halfInterval * ((sampleSlopes[1] - sampleSlopes[0]) / timeDeltas[1]));
    } else {
        halfInterval = midpointTime - sampleTimes[1];
        midpointValue =
            (channelSampleValues[2] + channelSampleValues[1]) * (halfInterval / timeDeltas[1]);
        midpointSlope =
            ((2.0f * (channelSampleValues[2] - channelSampleValues[1])) / timeDeltas[1]) -
            ((sampleSlopes[1] + sampleSlopes[0]) * (halfInterval / timeDeltas[1]));
    }
    if (pathTime < midpointTime) {
        intervalStartTime = sampleTimes[1];
        intervalStartValue = channelSampleValues[1];
        intervalStartSlope = sampleSlopes[0];
        intervalEndTime = midpointTime;
        intervalEndValue = midpointValue;
        intervalEndSlope = midpointSlope;
    } else {
        intervalStartTime = midpointTime;
        intervalStartValue = midpointValue;
        intervalStartSlope = midpointSlope;
        intervalEndTime = sampleTimes[2];
        intervalEndValue = channelSampleValues[2];
        intervalEndSlope = sampleSlopes[1];
    }
    return ((intervalEndSlope - intervalStartSlope) /
            (2.0f * (intervalEndTime - intervalStartTime))) *
               (pathTime - intervalStartTime) * (pathTime - intervalStartTime) +
           (pathTime - intervalStartTime) *
               ((intervalEndValue - intervalStartValue) / (intervalEndTime - intervalStartTime) -
                (intervalEndSlope - intervalStartSlope) / 2) +
           intervalStartValue;
}

// Game/menu placement code converts a screen pixel position and positive camera depth to world
// space.
// Use the first selected camera with the standard display size and aspect ratio.
void Hu3D2Dto3D(HuVecF *screenPosition, s16 cameraMask, HuVecF *worldPosition)
{
    HU3D_CAMERA *camera;
    float viewHeight;
    float halfFovTangent;
    float viewWidth;
    float screenFractionX;
    float screenFractionY;
    s16 cameraIndex;
    Mtx viewMatrix;

    for (cameraIndex = 0; cameraIndex < HU3D_CAM_MAX; cameraIndex++) {
        if (cameraMask & (1 << cameraIndex)) {
            break;
        }
    }
    camera = &Hu3DCamera[cameraIndex];
    halfFovTangent = HuSin(camera->fov/2)/HuCos(camera->fov/2);
    viewHeight = halfFovTangent * screenPosition->z * 2.0f;
    viewWidth = viewHeight * HU_DISP_ASPECT;
    screenFractionX = screenPosition->x / HU_DISP_WIDTH;
    screenFractionY = screenPosition->y / HU_DISP_HEIGHT;
    worldPosition->x = (screenFractionX-0.5)*viewWidth;
    worldPosition->y = -(screenFractionY-0.5)*viewHeight;
    worldPosition->z = -screenPosition->z;
    MTXLookAt(viewMatrix, &camera->pos, &camera->up, &camera->target);
    MTXInverse(viewMatrix, viewMatrix);
    MTXMultVec(viewMatrix, worldPosition, worldPosition);
}

// Game/menu labels, icons, and spatial effects project a world point through the first selected
// camera.
// Return standard display pixel coordinates; screenPosition->z is always cleared.
void Hu3D3Dto2D(HuVecF *worldPosition, s16 cameraMask, HuVecF *screenPosition)
{
    Vec viewPosition;
    HU3D_CAMERA *camera;
    float halfViewWidth;
    float halfViewHeight;
    s16 cameraIndex;
    Mtx viewMatrix;

    for (cameraIndex = 0; cameraIndex < HU3D_CAM_MAX; cameraIndex++) {
        if (cameraMask & (1 << cameraIndex)) {
            break;
        }
    }
    camera = &Hu3DCamera[cameraIndex];
    MTXLookAt(viewMatrix, &camera->pos, &camera->up, &camera->target);
    MTXMultVec(viewMatrix, worldPosition, &viewPosition);
    halfViewWidth = (HuSin(camera->fov/2) / HuCos(camera->fov/2)) * viewPosition.z * HU_DISP_ASPECT;
    halfViewHeight = (HuSin(camera->fov/2) / HuCos(camera->fov/2)) * viewPosition.z;
    screenPosition->x = DISP_HALF_W + viewPosition.x * (DISP_HALF_W / -halfViewWidth);
    screenPosition->y = DISP_HALF_H + viewPosition.y * (DISP_HALF_H / halfViewHeight);
    screenPosition->z = 0.0f;
}

// Model attachment code reads the world translation from the transform's final column.
void Hu3DMtxTransGet(Mtx transform, HuVecF *translation)
{
    translation->x = transform[0][3];
    translation->y = transform[1][3];
    translation->z = transform[2][3];
}

// Hu3DMtxRotGet uses atan2(x, y); when y is zero, force +/- pi/2, including +pi/2 for x == 0.
static inline float GetAngleXY(float x, float y) {
    if (y == 0.0f) {
        if (x >= 0.0f) {
            return M_PI / 2;
        } else {
            return -(M_PI / 2);
        }
    } else {
        return atan2f(x, y);
    }
}

// Model attachment code extracts Euler rotation in degrees after removing each basis vector's
// scale.
// A near-zero basis returns zero rotation; the Y-axis singular case assigns zero Z rotation.
void Hu3DMtxRotGet(Mtx transform, HuVecF *rotation)
{
    float cosineY;
    float scaleX;
    float scaleY;
    float scaleZ;
    float squaredScaleX;
    float squaredScaleY;
    float squaredScaleZ;
    float angleY;
    float sineY;

    squaredScaleX = transform[0][0] * transform[0][0] + transform[1][0] * transform[1][0] +
                    transform[2][0] * transform[2][0];
    scaleX = sqrtf(squaredScaleX);
    if (!(scaleX < 0.00000001f)) {
        squaredScaleY = transform[0][1] * transform[0][1] + transform[1][1] * transform[1][1] +
                        transform[2][1] * transform[2][1];
        scaleY = sqrtf(squaredScaleY);
        if (!(scaleY < 0.00000001f)) {
            squaredScaleZ = transform[0][2] * transform[0][2] + transform[1][2] * transform[1][2] +
                            transform[2][2] * transform[2][2];
            scaleZ = sqrtf(squaredScaleZ);
            if (!(scaleZ < 0.00000001f)) {
                sineY = -transform[2][0] / scaleX;
                if (sineY >= 1.0f) {
                    angleY = M_PI / 2;
                } else if (sineY <= -1.0f) {
                    angleY = -(M_PI / 2);
                } else {
                    angleY = asinf(sineY);
                }
                rotation->y = angleY;
                cosineY = cos(rotation->y);
                if (cosineY >= 0.00000001f) {
                    rotation->x = GetAngleXY(transform[2][1] / scaleY, transform[2][2] / scaleZ);
                    rotation->z = GetAngleXY(transform[1][0], transform[0][0]);
                } else {
                    // At the Y-axis singularity, combine the remaining rotation into X.
                    rotation->x = GetAngleXY(transform[0][1], transform[1][1]);
                    rotation->z = 0.0f;
                }
                rotation->x = MTXRadToDeg(rotation->x);
                rotation->y = MTXRadToDeg(rotation->y);
                rotation->z = MTXRadToDeg(rotation->z);
                return;
            }
        }
    }
    rotation->x = 0.0f;
    rotation->y = 0.0f;
    rotation->z = 0.0f;
}

// Model attachment and skinning code extract scale while removing shear between the basis vectors.
// A zero first basis returns unit scale; a reversed basis orientation negates all three scales.
void Hu3DMtxScaleGet(Mtx transform, HuVecF *scale)
{
    Vec shear;
    Vec basisX;
    Vec basisY;
    Vec basisZ;
    Vec projectedBasis;

    basisX.x = transform[0][0];
    basisX.y = transform[1][0];
    basisX.z = transform[2][0];
    scale->x = VECMag(&basisX);
    if(scale->x != 0.0) {
        VECNormalize(&basisX, &basisX);
    } else {
        scale->x = scale->y = scale->z = 1;
        return;
    }

    basisY.x = transform[0][1];
    basisY.y = transform[1][1];
    basisY.z = transform[2][1];
    // Remove X from Y, then remove Y and X from Z before measuring their scale.
    shear.x = VECDotProduct(&basisX, &basisY);
    VECScale(&basisX, &projectedBasis, shear.x);
    VECSubtract(&basisY, &projectedBasis, &basisY);
    scale->y = VECMag(&basisY);
    VECNormalize(&basisY, &basisY);
    shear.x /= scale->y;
    basisZ.x = transform[0][2];
    basisZ.y = transform[1][2];
    basisZ.z = transform[2][2];
    shear.z = VECDotProduct(&basisY, &basisZ);
    VECScale(&basisY, &projectedBasis, shear.z);
    VECSubtract(&basisZ, &projectedBasis, &basisZ);
    shear.y = VECDotProduct(&basisX, &basisZ);
    VECScale(&basisX, &projectedBasis, shear.y);
    VECSubtract(&basisZ, &projectedBasis, &basisZ);
    scale->z = VECMag(&basisZ);
    VECNormalize(&basisZ, &basisZ);
    VECCrossProduct(&basisY, &basisZ, &projectedBasis);
    // Preserve the sign of a transform whose basis reverses orientation.
    if (VECDotProduct(&basisX, &projectedBasis) < 0.0) {
        scale->x *= -1.0;
        scale->y *= -1.0;
        scale->z *= -1.0;
    }

}
