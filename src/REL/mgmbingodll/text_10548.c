/* Controls the board-side model's movement and animation during Treetop Bingo. */
#include "REL/mgmbingodll/module_types.h"
#include "msm_se.h"

void fn_1_11138(Vec *result, const Vec *point0, const Vec *point1,
               const Vec *point2, const Vec *point3, f32 duration, f32 time);
void fn_1_10B04(OMOBJ *obj);
extern f32 InterpolateBMLine(f32 *samples, f32 *sampleTimes, f32 frame);

extern void fn_1_E9C(void);
extern void fn_1_1217C(void);

extern void (*_ctors[])(void);
typedef void (*VoidFunc)(void);
extern const VoidFunc _dtors[];

// World-space destinations selected by the round and final-results controllers.
Point3d lbl_1_data_A60[7] = { { 0.0f, 700.0f, 400.0f },    { 3.0f, 700.0f, 400.0f },
                              { 3.0f, 700.0f, 400.0f },    { 3.0f, 1000.0f, 400.0f },
                              { 3.0f, 1000.0f, 400.0f },   { 250.0f, 2250.0f, 280.0f },
                              { 250.0f, 2250.0f, -100.0f } };

// Creates the board's moving item model and loads its motion clips during mode setup.
void fn_1_10548(void)
{
    lbl_1_bss_AE4 = omAddObjEx(lbl_1_bss_B00, 4608, 2U, 4U, -1, fn_1_10B04);
    lbl_1_bss_AE4->data = NULL;
    // Work slots hold the one-shot request, loop state, destination, and travel frames.
    lbl_1_bss_AE4->work[0] = lbl_1_bss_AE4->work[1] = 0;
    lbl_1_bss_AE4->work[2] = 0;
    lbl_1_bss_AE4->work[3] = 1;
    lbl_1_bss_AF4 = 1;
    lbl_1_bss_AE4->stat |= 0x100;
    *lbl_1_bss_AE4->mdlId = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mgmbingo, 43), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(*lbl_1_bss_AE4->mdlId, 1U);
    Hu3DModelLayerSet(*lbl_1_bss_AE4->mdlId, 1);
    Hu3DModelPosSetV(*lbl_1_bss_AE4->mdlId, &lbl_1_data_A60[lbl_1_bss_AE4->work[2]]);
    lbl_1_bss_AE8 = lbl_1_data_A60[lbl_1_bss_AE4->work[2]];
    Hu3DModelAttrSet(*lbl_1_bss_AE4->mdlId, HU3D_MOTATTR_LOOP);
    Hu3DModelScaleSet(*lbl_1_bss_AE4->mdlId, (1.0f), (1.0f), (1.0f));
    Hu3DLLightCreate(*lbl_1_bss_AE4->mdlId, (0.0f), (1500.0f), (1500.0f), (0.0f), (-0.1f), (-1.0f),
                     204, 204, 204);
    lbl_1_bss_AE4->mdlId[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mgmbingo, 48), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelHookSet(*lbl_1_bss_AE4->mdlId, "f-itemhook_R", lbl_1_bss_AE4->mdlId[1]);
    *lbl_1_bss_AE4->mtnId =
        Hu3DJointMotion(*lbl_1_bss_AE4->mdlId, HuDataSelHeapReadNum(DATANUM(DATA_mgmbingo, 44),
                                                                    HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(*lbl_1_bss_AE4->mdlId, *lbl_1_bss_AE4->mtnId);
    lbl_1_bss_AE4->mtnId[1] = Hu3DJointMotion(
        *lbl_1_bss_AE4->mdlId,
        HuDataSelHeapReadNum(DATANUM(DATA_mgmbingo, 45), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_AE4->mtnId[2] = Hu3DJointMotion(
        *lbl_1_bss_AE4->mdlId,
        HuDataSelHeapReadNum(DATANUM(DATA_mgmbingo, 46), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_AE4->mtnId[3] = Hu3DJointMotion(
        *lbl_1_bss_AE4->mdlId,
        HuDataSelHeapReadNum(DATANUM(DATA_mgmbingo, 47), HU_MEMNUM_OVL, HEAP_MODEL));
}

// Requests the item model's movement animation when the board starts or stops it.
void fn_1_10970(u16 enabled) {
    if ((lbl_1_bss_AE4->work[1] == 0U) && (enabled != 0U)) {
        lbl_1_bss_AE4->work[1] = 1U;
    }
    if ((lbl_1_bss_AE4->work[1] != 0U) && (enabled == 0U)) {
        lbl_1_bss_AE4->work[1] = 2U;
    }
}

// Requests the board-side model's one-shot gesture, handled when its other motion modes allow it.
void fn_1_109E4(void) {
    lbl_1_bss_AE4->work[0] = 1;
}

// Starts or ends the special motion requested by the board's mode state.
void fn_1_109FC(s16 enable) {
    if ((lbl_1_bss_AE0 == 0) && (enable != 0)) {
        lbl_1_bss_AE0 = 1;
        return;
    }
    if ((lbl_1_bss_AE0 != 0) && (enable == 0)) {
        lbl_1_bss_AE0 = 3;
    }
}

// Selects a board position and transition length for the moving item model.
void fn_1_10A64(s16 positionIndex, s16 durationFrames) {
    if (lbl_1_bss_AE4->work[2] != (u32)positionIndex) {
        lbl_1_bss_AE4->work[2] = (u32)positionIndex;
        lbl_1_bss_AE4->work[3] = (s32)durationFrames;
        lbl_1_bss_AF4 = 0;
        Hu3DModelPosGet(lbl_1_bss_AE4->mdlId[0], &lbl_1_bss_AE8);
    }
}

// Updates the item's position and motion state once per object-manager frame.
void fn_1_10B04(OMOBJ *object)
{
    Vec position;
    Vec startPoint;
    Vec basePoint;
    Vec targetPoint;
    Vec endPoint;
    f32 motionMaxTime;

    Hu3DModelPosGet(object->mdlId[0], &position);
    basePoint = lbl_1_bss_AE8;
    targetPoint = lbl_1_data_A60[object->work[2]];
    startPoint = basePoint;
    endPoint = targetPoint;
    fn_1_11138(&position, &startPoint, &basePoint, &targetPoint, &endPoint,
               (f32)object->work[3], (f32)lbl_1_bss_AF4);
    Hu3DModelPosSetV(object->mdlId[0], &position);
    lbl_1_bss_AF4++;
    if ((u32)lbl_1_bss_AF4 >= object->work[3]) {
        lbl_1_bss_AF4 = 1;
        object->work[3] = 1;
    }
    if (lbl_1_bss_AE0 == 0) {
        if (object->work[1] != 0) {
            if (object->work[1] == 1) {
                if ((Hu3DMotionShiftIDGet(object->mdlId[0]) < 0) &&
                    (object->mtnId[2] != Hu3DMotionIDGet(object->mdlId[0]))) {
                    Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[2], (0.0f),
                                       (8.0f), HU3D_MOTATTR_LOOP);
                    Hu3DMotionSpeedSet(object->mdlId[0], (1.0f));
                    object->work[0] = 0;
                }
                if ((Hu3DMotionShiftIDGet(object->mdlId[0]) < 0) &&
                    (object->mtnId[2] == Hu3DMotionIDGet(object->mdlId[0])) &&
                    ((Hu3DMotionTimeGet(object->mdlId[0]) == (6.0f)) ||
                     (Hu3DMotionTimeGet(object->mdlId[0]) == (22.0f)) ||
                     (Hu3DMotionTimeGet(object->mdlId[0]) == (38.0f)) ||
                     (Hu3DMotionTimeGet(object->mdlId[0]) == (54.0f)) ||
                     (Hu3DMotionTimeGet(object->mdlId[0]) == (70.0f)))) {
                    HuAudFXPlay(MSM_SE_GUIDE_16);
                    return;
                }
            } else if (Hu3DMotionShiftIDGet(object->mdlId[0]) < 0) {
                Hu3DModelAttrSet(object->mdlId[0], HU3D_MOTATTR_PAUSE);
                Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0], (0.0f),
                                   (8.0f), HU3D_MOTATTR_LOOP);
                Hu3DMotionSpeedSet(object->mdlId[0], (1.0f));
                object->work[1] = 0;
                object->work[0] = 0;
                return;
            }
        } else if (object->work[0] != 0) {
            if ((Hu3DMotionShiftIDGet(object->mdlId[0]) < 0) &&
                (object->mtnId[1] != Hu3DMotionIDGet(object->mdlId[0]))) {
                Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[1], (0.0f),
                                   (8.0f), HU3D_MOTATTR_LOOP);
                Hu3DMotionSpeedSet(object->mdlId[0], (2.0f));
                return;
            }
            if ((Hu3DMotionShiftIDGet(object->mdlId[0]) < 0) &&
                (object->mtnId[1] == Hu3DMotionIDGet(object->mdlId[0]))) {
                motionMaxTime = Hu3DMotionMaxTimeGet(object->mdlId[0]);
                if ((Hu3DData[object->mdlId[0]].motWork.speed +
                     Hu3DMotionTimeGet(object->mdlId[0])) >= motionMaxTime) {
                    Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0], (0.0f),
                                       (8.0f), HU3D_MOTATTR_LOOP);
                    Hu3DMotionSpeedSet(object->mdlId[0], (1.0f));
                    object->work[0] = 0;
                }
            }
        }
    } else if (lbl_1_bss_AE0 == 1) {
        Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[3], (0.0f),
                           (8.0f), HU3D_MOTATTR_LOOP);
        Hu3DMotionSpeedSet(object->mdlId[0], (1.0f));
        Hu3DMotionAttrSet(object->mtnId[3], 1U);
        lbl_1_bss_AE0 = 2;
        return;
    } else if (lbl_1_bss_AE0 == 3) {
        Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0], (0.0f),
                           (8.0f), HU3D_MOTATTR_LOOP);
        Hu3DMotionSpeedSet(object->mdlId[0], (1.0f));
        Hu3DMotionAttrReset(object->mtnId[3], 1U);
        lbl_1_bss_AE0 = 0;
        object->work[1] = 0;
        object->work[0] = 0;
    }
}

// Interpolates the item's coordinates for its per-frame movement callback.
void fn_1_11138(
    HuVecF *output,
    const HuVecF *sample0,
    const HuVecF *sample1,
    const HuVecF *sample2,
    const HuVecF *sample3,
    f32 duration,
    f32 frame)
{
    f32 sampleTimes[4];
    f32 samples[4];

    sampleTimes[0] = (-1.0f) * duration;
    sampleTimes[1] = (0.0f);
    sampleTimes[2] = duration;
    sampleTimes[3] = (2.0f) * duration;
    // Ease the interpolation time toward the destination with a quarter sine wave.
    frame = (f32)((f64)duration *
        sin(((3.141592653589793) *
            (f64)((90.0f) * (frame / duration))) /
            (180.0)));

    samples[0] = sample0->x;
    samples[1] = sample1->x;
    samples[2] = sample2->x;
    samples[3] = sample3->x;
    output->x = InterpolateBMLine(samples, sampleTimes, frame);
    samples[0] = sample0->y;
    samples[1] = sample1->y;
    samples[2] = sample2->y;
    samples[3] = sample3->y;
    output->y = InterpolateBMLine(samples, sampleTimes, frame);
    samples[0] = sample0->z;
    samples[1] = sample1->z;
    samples[2] = sample2->z;
    samples[3] = sample3->z;
    output->z = InterpolateBMLine(samples, sampleTimes, frame);
}

// Initializes the board object manager and starts the Treetop Bingo control process.
void fn_1_112B8(void) {
    lbl_1_bss_B00 = omInitObjMan(100, 8192);
    *(s16 *)&MgModeSubMode = 0;
    fn_1_E9C();
    HuPrcChildCreate(fn_1_1217C, 100U, 12288U, 0, lbl_1_bss_B00);
}

// Creates Treetop Bingo's object manager and camera when the overlay loads.
s32 _prolog(void) {
    void (**constructor)(void) = _ctors;

    while (*constructor != 0) {
        (*constructor)();
        ++constructor;
    }
    lbl_1_bss_B00 = omInitObjMan(100, 8192);
    *(s16 *) &MgModeSubMode = 0;
    fn_1_E9C();
    HuPrcChildCreate(fn_1_1217C, 100U, 12288U, 0, lbl_1_bss_B00);
    return 0;
}

// Runs registered cleanup functions when the Treetop Bingo overlay unloads.
void _epilog(void) {
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

u32 lbl_1_bss_AF4;
Point3d lbl_1_bss_AE8;
OMOBJ *lbl_1_bss_AE4;
s16 lbl_1_bss_AE0;
