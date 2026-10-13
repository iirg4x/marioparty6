/* Animates the tournament menu character and moves it between bracket positions. */
#include "dolphin/math.h"
#include "dolphin.h"
#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/hsfex.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/process.h"
#include "game/memory.h"
#include "datadir_enum.h"
#include "msm_se.h"

#define HOST_MOTION_BLEND_FRAMES 8.0f

OMOBJ *lbl_1_bss_100;
u32 lbl_1_bss_FC;
Vec lbl_1_bss_F0;
extern OMOBJMAN *lbl_1_bss_12C;
void fn_1_5B84(OMOBJ *object);
s16 fn_1_A42C(void);
void fn_1_60F8(Vec *interpolatedPosition, const Vec *beforeStart,
               const Vec *startPosition, const Vec *destination,
               const Vec *pastDestination, f32 durationFrames, f32 elapsedFrames);

Vec lbl_1_data_440[16] = {
    { 0.0f, -1350.0f, 1500.0f },
    { 0.0f, -300.0f, 1500.0f },
    { 0.0f, -300.0f, 400.0f },
    { -400.0f, 0.0f, 400.0f },
    { -400.0f, -25.0f, -200.0f },
    { -400.0f, 150.0f, -200.0f },
    { 400.0f, -25.0f, -200.0f },
    { 400.0f, 150.0f, -200.0f },
    { 0.0f, -25.0f, -200.0f },
    { 0.0f, 0.0f, -200.0f },
    { 0.0f, 450.0f, -200.0f },
    { 0.0f, 450.0f, -650.0f },
    { 0.0f, 600.0f, -650.0f },
    { 350.0f, 1000.0f, -650.0f },
    { 350.0f, 1000.0f, -1200.0f },
    { 700.0f, 1000.0f, -1000.0f },
};

/* Scene setup creates the tournament host model and its held prop. */
void fn_1_5154(void) {
    lbl_1_bss_100 = omAddObjEx(lbl_1_bss_12C, 4352, 2, 3, -1, fn_1_5B84);
    lbl_1_bss_100->data = NULL;
    lbl_1_bss_100->work[0] = lbl_1_bss_100->work[1] = 0;
    lbl_1_bss_100->work[2] = 0;
    lbl_1_bss_100->work[3] = 1;
    lbl_1_bss_FC = 1;
    lbl_1_bss_100->stat |= OM_STAT_MODELPAUSE;
    *lbl_1_bss_100->mdlId = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_mgmtournament, 28), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(*lbl_1_bss_100->mdlId, 1);
    Hu3DModelLayerSet(*lbl_1_bss_100->mdlId, 2);
    Hu3DModelPosSetV(
        *lbl_1_bss_100->mdlId,
        &lbl_1_data_440[lbl_1_bss_100->work[2]]);
    lbl_1_bss_F0 = lbl_1_data_440[lbl_1_bss_100->work[2]];
    Hu3DModelAttrSet(*lbl_1_bss_100->mdlId, HU3D_MOTATTR_LOOP);
    Hu3DModelScaleSet(
        *lbl_1_bss_100->mdlId,
        1.2000000476837158f,
        1.2000000476837158f,
        1.2000000476837158f);
    Hu3DLLightCreate(
        *lbl_1_bss_100->mdlId,
        0.0f,
        1500.0f,
        1500.0f,
        0.0f,
        -0.10000000149011612f,
        -1.0f,
        204,
        204,
        204);
    lbl_1_bss_100->mdlId[1] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_mgmtournament, 32), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_100->mdlId[1], 1);
    Hu3DModelLayerSet(lbl_1_bss_100->mdlId[1], 2);
    Hu3DModelHookSet(
        *lbl_1_bss_100->mdlId, "f-itemhook_R", lbl_1_bss_100->mdlId[1]);
    Hu3DModelAttrSet(lbl_1_bss_100->mdlId[1], HU3D_MOTATTR_LOOP);
    *lbl_1_bss_100->mtnId = Hu3DJointMotion(
        *lbl_1_bss_100->mdlId,
        HuDataSelHeapReadNum(DATANUM(DATA_mgmtournament, 29), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(*lbl_1_bss_100->mdlId, *lbl_1_bss_100->mtnId);
    lbl_1_bss_100->mtnId[1] = Hu3DJointMotion(
        *lbl_1_bss_100->mdlId,
        HuDataSelHeapReadNum(DATANUM(DATA_mgmtournament, 30), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_100->mtnId[2] = Hu3DJointMotion(
        *lbl_1_bss_100->mdlId,
        HuDataSelHeapReadNum(DATANUM(DATA_mgmtournament, 31), HU_MEMNUM_OVL, HEAP_MODEL));
}

/* Scene setup places the host at the station selected for the current tournament mode. */
void fn_1_5590(void)
{
    s16 mode;

    mode = fn_1_A42C();
    if (mode == 0) {
        lbl_1_bss_100->work[2] = 4;
    } else if (mode == 1) {
        lbl_1_bss_100->work[2] = 6;
    } else if (mode == 2) {
        lbl_1_bss_100->work[2] = 8;
    } else {
        lbl_1_bss_100->work[2] = 11;
    }

    Hu3DModelPosSetV(lbl_1_bss_100->mdlId[0],
                     &lbl_1_data_440[lbl_1_bss_100->work[2]]);
    lbl_1_bss_F0 = lbl_1_data_440[lbl_1_bss_100->work[2]];
}

/* Result scenes request or end the host's looping reaction. */
void fn_1_56B4(u16 enabled) {
    if ((lbl_1_bss_100->work[1] == 0U) && (enabled != 0U)) {
        lbl_1_bss_100->work[1] = 1U;
    }
    if ((lbl_1_bss_100->work[1] != 0U) && (enabled == 0U)) {
        lbl_1_bss_100->work[1] = 2U;
    }
}

/* Window handlers request a brief host animation, which plays at double speed before returning
 * to the idle loop. */
void fn_1_5728(void) {
    lbl_1_bss_100->work[0] = 1;
}

/* The scene script sends the host toward a bracket point over the requested frame count. */
void fn_1_5740(s16 positionIndex, s16 duration)
{
    if (lbl_1_bss_100->work[2] != positionIndex) {
        lbl_1_bss_100->work[2] = positionIndex;
        lbl_1_bss_100->work[3] = duration;
        lbl_1_bss_FC = 0;
        Hu3DModelPosGet(lbl_1_bss_100->mdlId[0], &lbl_1_bss_F0);
    }
}

/* The scene script chooses a transition point for the active tournament mode. */
void fn_1_57E0(s16 duration)
{
    s16 mode;

    mode = fn_1_A42C();
    if ((mode == 0) && (lbl_1_bss_100->work[2] != 4)) {
        if (lbl_1_bss_100->work[2] == 3) {
            return;
        }
        lbl_1_bss_100->work[2] = 3;
        lbl_1_bss_100->work[3] = duration;
        lbl_1_bss_FC = 0;
        Hu3DModelPosGet(lbl_1_bss_100->mdlId[0], &lbl_1_bss_F0);
        return;
    } else if (mode == 1) {
        if (lbl_1_bss_100->work[2] != 6) {
            lbl_1_bss_100->work[2] = 6;
            lbl_1_bss_100->work[3] = duration;
            lbl_1_bss_FC = 0;
            Hu3DModelPosGet(lbl_1_bss_100->mdlId[0], &lbl_1_bss_F0);
        }
    } else if (mode == 2) {
        if (lbl_1_bss_100->work[2] != 8) {
            lbl_1_bss_100->work[2] = 8;
            lbl_1_bss_100->work[3] = duration;
            lbl_1_bss_FC = 0;
            Hu3DModelPosGet(lbl_1_bss_100->mdlId[0], &lbl_1_bss_F0);
        }
    } else if (mode == 3) {
        if ((lbl_1_bss_100->work[2] != 11) &&
            (lbl_1_bss_100->work[2] != 10)) {
            lbl_1_bss_100->work[2] = 10;
            lbl_1_bss_100->work[3] = duration;
            lbl_1_bss_FC = 0;
            Hu3DModelPosGet(lbl_1_bss_100->mdlId[0], &lbl_1_bss_F0);
        }
    }
}

/* The scene script moves the host to an opening or closing station, then sleeps for the move. */
void fn_1_5A44(s16 duration) {
    s16 mode;

    mode = fn_1_A42C();
    if (mode == 0) {
        if ((u32) lbl_1_bss_100->work[2] != 4U) {
            lbl_1_bss_100->work[2] = 4U;
            lbl_1_bss_100->work[3] = (s32) duration;
            lbl_1_bss_FC = 0;
            Hu3DModelPosGet(lbl_1_bss_100->mdlId[0], &lbl_1_bss_F0);
        }
        HuPrcSleep(duration);
    }
    if (mode == 3) {
        if ((u32) lbl_1_bss_100->work[2] != 11U) {
            lbl_1_bss_100->work[2] = 11U;
            lbl_1_bss_100->work[3] = (s32) duration;
            lbl_1_bss_FC = 0;
            Hu3DModelPosGet(lbl_1_bss_100->mdlId[0], &lbl_1_bss_F0);
        }
        HuPrcSleep(duration);
    }
}

/* The object manager calls this each update to move the host and advance its motions. */
void fn_1_5B84(OMOBJ *object)
{
    Vec position;
    Vec startPoint;
    Vec basePoint;
    Vec targetPoint;
    Vec endPoint;
    f32 motionMaxTime;

    Hu3DModelPosGet(object->mdlId[0], &position);
    basePoint = lbl_1_bss_F0;
    targetPoint = lbl_1_data_440[object->work[2]];
    startPoint = basePoint;
    endPoint = targetPoint;
    if (object->work[2] == 2) {
        /* The center stop follows a deeper curve before returning to its target. */
        startPoint.y -= 500.0f;
        startPoint.z -= 500.0f;
        endPoint.y -= 500.0f;
        endPoint.z += 500.0f;
    }
    fn_1_60F8(&position, &startPoint, &basePoint, &targetPoint, &endPoint,
              (f32)object->work[3], (f32)lbl_1_bss_FC);
    Hu3DModelPosSetV(object->mdlId[0], &position);
    lbl_1_bss_FC++;
    if ((u32)lbl_1_bss_FC >= object->work[3]) {
        lbl_1_bss_FC = 1;
        object->work[3] = 1;
    }
    if (object->work[1] != 0) {
        if (object->work[1] == 1) {
            if ((Hu3DMotionShiftIDGet(object->mdlId[0]) < 0) &&
                (object->mtnId[2] != Hu3DMotionIDGet(object->mdlId[0]))) {
                Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[2], 0.0f,
                                   HOST_MOTION_BLEND_FRAMES, HU3D_MOTATTR_LOOP);
                Hu3DMotionSpeedSet(object->mdlId[0], 1.0f);
                object->work[0] = 0;
            }
            if ((Hu3DMotionShiftIDGet(object->mdlId[0]) < 0) &&
                (object->mtnId[2] == Hu3DMotionIDGet(object->mdlId[0])) &&
                ((Hu3DMotionTimeGet(object->mdlId[0]) == 6.0f) ||
                 (Hu3DMotionTimeGet(object->mdlId[0]) == 22.0f) ||
                 (Hu3DMotionTimeGet(object->mdlId[0]) == 38.0f) ||
                 (Hu3DMotionTimeGet(object->mdlId[0]) == 54.0f) ||
                 (Hu3DMotionTimeGet(object->mdlId[0]) == 70.0f))) {
                /* The looping reaction repeats its cue at these five animation frames. */
                HuAudFXPlay(MSM_SE_GUIDE_16);
            }
        } else if (Hu3DMotionShiftIDGet(object->mdlId[0]) < 0) {
            Hu3DModelAttrSet(object->mdlId[0], HU3D_MOTATTR_PAUSE);
            Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0], 0.0f,
                               HOST_MOTION_BLEND_FRAMES, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(object->mdlId[0], 1.0f);
            object->work[1] = 0;
            object->work[0] = 0;
        }
    } else if (object->work[0] != 0) {
        if ((Hu3DMotionShiftIDGet(object->mdlId[0]) < 0) &&
            (object->mtnId[1] != Hu3DMotionIDGet(object->mdlId[0]))) {
            Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[1], 0.0f,
                               HOST_MOTION_BLEND_FRAMES, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(object->mdlId[0], 2.0f);
            return;
        }
        if ((Hu3DMotionShiftIDGet(object->mdlId[0]) < 0) &&
            (object->mtnId[1] == Hu3DMotionIDGet(object->mdlId[0]))) {
            motionMaxTime = Hu3DMotionMaxTimeGet(object->mdlId[0]);
            if ((Hu3DData[object->mdlId[0]].motWork.speed + Hu3DMotionTimeGet(object->mdlId[0])) >=
                motionMaxTime) {
                Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0], 0.0f,
                                   HOST_MOTION_BLEND_FRAMES, HU3D_MOTATTR_LOOP);
                Hu3DMotionSpeedSet(object->mdlId[0], 1.0f);
                object->work[0] = 0;
            }
        }
    }
}

/* The object callback evaluates the host's four-point movement path each frame. */
void fn_1_60F8(Vec *interpolatedPosition, const Vec *beforeStart,
               const Vec *startPosition, const Vec *destination,
               const Vec *pastDestination, f32 durationFrames, f32 elapsedFrames)
{
    f32 sampleTimes[4];
    f32 axisSamples[4];

    sampleTimes[0] = -1.0f * durationFrames;
    sampleTimes[1] = 0.0f;
    sampleTimes[2] = durationFrames;
    sampleTimes[3] = 2.0f * durationFrames;
    /* Ease the movement with a quarter sine wave so it slows near the destination. */
    elapsedFrames = (f32)((f64)durationFrames *
        sin((3.141592653589793 *
            (f64)(90.0f * (elapsedFrames / durationFrames))) /
            180.0));

    axisSamples[0] = beforeStart->x;
    axisSamples[1] = startPosition->x;
    axisSamples[2] = destination->x;
    axisSamples[3] = pastDestination->x;
    interpolatedPosition->x = InterpolateBMLine(axisSamples, sampleTimes, elapsedFrames);
    axisSamples[0] = beforeStart->y;
    axisSamples[1] = startPosition->y;
    axisSamples[2] = destination->y;
    axisSamples[3] = pastDestination->y;
    interpolatedPosition->y = InterpolateBMLine(axisSamples, sampleTimes, elapsedFrames);
    axisSamples[0] = beforeStart->z;
    axisSamples[1] = startPosition->z;
    axisSamples[2] = destination->z;
    axisSamples[3] = pastDestination->z;
    interpolatedPosition->z = InterpolateBMLine(axisSamples, sampleTimes, elapsedFrames);
}
