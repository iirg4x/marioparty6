/* Creates and animates Endurance Alley's sign and course scenery models. */
#include "REL/mgmrenshodll/module_types.h"
#include "dolphin/math.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/frand.h"
#include "msm_se.h"

extern OMOBJMAN *lbl_1_bss_158;

void fn_1_7E84(OMOBJ *sceneryObject);
void fn_1_8480(Vec *output, const Vec *sample0, const Vec *sample1, const Vec *sample2,
               const Vec *sample3, f32 duration, f32 frame);
extern f32 fn_1_4A2C(void);
extern f32 fn_1_4C3C(void);
extern f32 fn_1_4ADC(void);
extern u8 MgModePlayNum;
void fn_1_8B58(OMOBJ *arenaObject);
extern f32 InterpolateBMLine(f32 *samples, f32 *sampleTimes, f32 frame);
extern void fn_1_4EB0(void);

Vec lbl_1_data_298[5] = {
    {0.0f, 100.0f, -150.0f},
    {0.0f, 300.0f, -150.0f},
    {250.0f, 100.0f, -150.0f},
    {-250.0f, 100.0f, -150.0f},
    {450.0f, 300.0f, -1500.0f}
};
char lbl_1_data_2D4[] = "f-itemhook_R";
Vec lbl_1_data_2E4[7] = {
    {-580.0f, 40.0f, -18950.0f},
    {-580.0f, 40.0f, -21950.0f},
    {-580.0f, 40.0f, -23170.0f},
    {-750.0f, 0.0f, -22700.0f},
    {-710.0f, 0.0f, -28160.0f},
    {-820.0f, 0.0f, -28470.0f},
    {-730.0f, 0.0f, -28780.0f}
};
u32 lbl_1_bss_138;
Vec lbl_1_bss_12C;
s16 lbl_1_bss_128;
OMOBJ *lbl_1_bss_124;
s16 lbl_1_bss_11C[4];
OMOBJ *lbl_1_bss_118;

/* Called during scene setup to create the moving sign and its model motions. */
void fn_1_78FC(void)
{
    lbl_1_bss_124 = omAddObjEx(lbl_1_bss_158, 4608, 2U, 3U, -1, fn_1_7E84);
    lbl_1_bss_124->data = NULL;
    lbl_1_bss_124->work[0] = lbl_1_bss_124->work[1] = 0;
    lbl_1_bss_124->work[2] = 0;
    lbl_1_bss_124->work[3] = 1;
    lbl_1_bss_138 = 1;
    lbl_1_bss_128 = 0;
    lbl_1_bss_124->stat |= 0x100;
    *lbl_1_bss_124->mdlId = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mgmrensho, 26), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(*lbl_1_bss_124->mdlId, 1U);
    Hu3DModelLayerSet(*lbl_1_bss_124->mdlId, 3);
    Hu3DModelPosSetV(*lbl_1_bss_124->mdlId, &lbl_1_data_298[lbl_1_bss_124->work[2]]);
    lbl_1_bss_12C = lbl_1_data_298[lbl_1_bss_124->work[2]];
    Hu3DModelAttrSet(*lbl_1_bss_124->mdlId, HU3D_MOTATTR_LOOP);
    Hu3DModelScaleSet(*lbl_1_bss_124->mdlId, 1.2f, 1.2f, 1.2f);
    Hu3DModelShadowSet(*lbl_1_bss_124->mdlId);
    Hu3DLLightCreate(*lbl_1_bss_124->mdlId, 0.0f, 1500.0f, 1500.0f,
                     0.0f, -0.1f, -1.0f, 204U, 204U, 204U);
    lbl_1_bss_124->mdlId[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mgmrensho, 30), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelHookSet(*lbl_1_bss_124->mdlId, lbl_1_data_2D4, lbl_1_bss_124->mdlId[1]);
    *lbl_1_bss_124->mtnId = Hu3DJointMotion(*lbl_1_bss_124->mdlId,
                                            HuDataSelHeapReadNum(
                                                DATANUM(DATA_mgmrensho, 27), HU_MEMNUM_OVL,
                                                HEAP_MODEL));
    Hu3DMotionSet(*lbl_1_bss_124->mdlId, *lbl_1_bss_124->mtnId);
    lbl_1_bss_124->mtnId[1] = Hu3DJointMotion(
        *lbl_1_bss_124->mdlId, HuDataSelHeapReadNum(
            DATANUM(DATA_mgmrensho, 28), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_124->mtnId[2] = Hu3DJointMotion(
        *lbl_1_bss_124->mdlId, HuDataSelHeapReadNum(
            DATANUM(DATA_mgmrensho, 29), HU_MEMNUM_OVL, HEAP_MODEL));
}

/* Mode scripts start and end the sign's response animation. */
void fn_1_7D08(u16 responseActive) {
    if ((lbl_1_bss_124->work[1] == 0U) && (responseActive != 0U)) {
        lbl_1_bss_124->work[1] = 1U;
    }
    if ((lbl_1_bss_124->work[1] != 0U) && (responseActive == 0U)) {
        lbl_1_bss_124->work[1] = 2U;
    }
}

/* Requests the sign's speaking motion on its next update. */
void fn_1_7D7C(void) {
    lbl_1_bss_124->work[0] = 1;
}

/* Changes the sign's path segment and frame span when the scene advances. */
void fn_1_7D94(s16 pathIndex, s16 segmentFrames)
{
    if (lbl_1_bss_124->work[2] != pathIndex) {
        lbl_1_bss_124->work[2] = pathIndex;
        lbl_1_bss_124->work[3] = segmentFrames;
        lbl_1_bss_138 = 0;
        Hu3DModelPosGet(lbl_1_bss_124->mdlId[0], &lbl_1_bss_12C);
    }
}

/* Enables the sign's course-depth correction during the turn-end travel sequence. */
void fn_1_7E34(void) {
    if (lbl_1_bss_128 == 0) {
        lbl_1_bss_128 = 1;
    }
}

/* Disables the sign's course-depth correction after the turn-end travel sequence. */
void fn_1_7E5C(void) {
    if (lbl_1_bss_128 != 0) {
        lbl_1_bss_128 = 0;
    }
}

/* Runs each frame to move the sign along its path and update its animation. */
void fn_1_7E84(OMOBJ *sceneryObject)
{
    Vec position;
    Vec currentPath;
    Vec previousPath;
    Vec nextPath;
    Vec followingPath;
    f32 depthOffset;

    Hu3DModelPosGet(sceneryObject->mdlId[0], (Point3d *)&position);
    previousPath = lbl_1_bss_12C;
    nextPath = lbl_1_data_298[sceneryObject->work[2]];
    currentPath = previousPath;
    followingPath = nextPath;
    fn_1_8480(&position, &currentPath, &previousPath, &nextPath, &followingPath,
              (f32)sceneryObject->work[3], (f32)lbl_1_bss_138);
    Hu3DModelPosSetV(sceneryObject->mdlId[0], (Point3d *)&position);
    lbl_1_bss_138++;
    if (lbl_1_bss_138 >= sceneryObject->work[3]) {
        lbl_1_bss_138 = 1;
        sceneryObject->work[3] = 1;
    }
    if (lbl_1_bss_128 > 0) {
        depthOffset = fn_1_4A2C() - fn_1_4C3C();
        depthOffset += (f32)lbl_1_bss_128 * (fn_1_4C3C() / 120.0f);
        depthOffset -= fn_1_4ADC();
        depthOffset *= -1.0f;
        position.z += depthOffset;
        Hu3DModelPosSetV(sceneryObject->mdlId[0], (Point3d *)&position);
        lbl_1_bss_128++;
        if (lbl_1_bss_128 > 120) {
            lbl_1_bss_128 = 120;
        }
    }
    if (sceneryObject->work[1] != 0) {
        if (sceneryObject->work[1] == 1) {
            if ((Hu3DMotionShiftIDGet(sceneryObject->mdlId[0]) < 0) &&
                (sceneryObject->mtnId[2] != Hu3DMotionIDGet(sceneryObject->mdlId[0]))) {
                Hu3DMotionShiftSet(sceneryObject->mdlId[0], sceneryObject->mtnId[2], 0.0f,
                                   8.0f, HU3D_MOTATTR_LOOP);
                Hu3DMotionSpeedSet(sceneryObject->mdlId[0], 1.0f);
                sceneryObject->work[0] = 0;
            }
            if ((Hu3DMotionShiftIDGet(sceneryObject->mdlId[0]) < 0) &&
                (sceneryObject->mtnId[2] == Hu3DMotionIDGet(sceneryObject->mdlId[0])) &&
                ((6.0f == Hu3DMotionTimeGet(sceneryObject->mdlId[0])) ||
                 (22.0f == Hu3DMotionTimeGet(sceneryObject->mdlId[0])) ||
                 (38.0f == Hu3DMotionTimeGet(sceneryObject->mdlId[0])) ||
                 (54.0f == Hu3DMotionTimeGet(sceneryObject->mdlId[0])) ||
                 (70.0f == Hu3DMotionTimeGet(sceneryObject->mdlId[0])))) {
                HuAudFXPlay(MSM_SE_GUIDE_16);
            }
        } else if (Hu3DMotionShiftIDGet(sceneryObject->mdlId[0]) < 0) {
            Hu3DModelAttrSet(sceneryObject->mdlId[0], HU3D_MOTATTR_PAUSE);
            Hu3DMotionShiftSet(sceneryObject->mdlId[0], sceneryObject->mtnId[0], 0.0f,
                               8.0f, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(sceneryObject->mdlId[0], 1.0f);
            sceneryObject->work[1] = 0;
            sceneryObject->work[0] = 0;
        }
    } else if (sceneryObject->work[0] != 0) {
        if ((Hu3DMotionShiftIDGet(sceneryObject->mdlId[0]) < 0) &&
            (sceneryObject->mtnId[1] != Hu3DMotionIDGet(sceneryObject->mdlId[0]))) {
            Hu3DMotionShiftSet(sceneryObject->mdlId[0], sceneryObject->mtnId[1], 0.0f,
                               8.0f, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(sceneryObject->mdlId[0], 2.0f);
            return;
        }
        if ((Hu3DMotionShiftIDGet(sceneryObject->mdlId[0]) < 0) &&
            (sceneryObject->mtnId[1] == Hu3DMotionIDGet(sceneryObject->mdlId[0]))) {
            if ((Hu3DData[sceneryObject->mdlId[0]].motWork.speed +
                 Hu3DMotionTimeGet(sceneryObject->mdlId[0])) >=
                Hu3DMotionMaxTimeGet(sceneryObject->mdlId[0])) {
                Hu3DMotionShiftSet(sceneryObject->mdlId[0], sceneryObject->mtnId[0], 0.0f,
                                   8.0f, HU3D_MOTATTR_LOOP);
                Hu3DMotionSpeedSet(sceneryObject->mdlId[0], 1.0f);
                sceneryObject->work[0] = 0;
            }
        }
    }
}

/* Interpolates the sign position between four path samples over one segment. */
void fn_1_8480(
    Vec *output,
    const Vec *sample0,
    const Vec *sample1,
    const Vec *sample2,
    const Vec *sample3,
    f32 duration,
    f32 frame)
{
    f32 sampleTimes[4];
    f32 samples[4];

    sampleTimes[0] = -1.0f * duration;
    sampleTimes[1] = 0.0f;
    sampleTimes[2] = duration;
    sampleTimes[3] = 2.0f * duration;
    /* Apply quarter-sine easing to the sample time before interpolating the position. */
    frame = (f32)((f64)duration *
        sin((3.141592653589793 *
            (f64)(90.0f * (frame / duration))) /
            180.0));

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

/* Called during scene setup to create course scenery models selected by the current play count. */
void fn_1_8600(void)
{
    OMOBJ *arenaObject;
    s16 modelIndex;

    arenaObject = omAddObjEx(lbl_1_bss_158, 4864, 7U, 14U, -1, fn_1_8B58);
    lbl_1_bss_118 = arenaObject;
    arenaObject->stat |= 0x100;
    arenaObject->data = NULL;
    memset(arenaObject->work, 0, 16U);
    modelIndex = 0;
    while (modelIndex < 7) {
        arenaObject->mdlId[modelIndex] = -1;
        modelIndex++;
    }
    modelIndex = 0;
    while (modelIndex < 14) {
        arenaObject->mtnId[modelIndex] = -1;
        modelIndex++;
    }
    if ((MgModePlayNum >= 60U) && (MgModePlayNum < 70U)) {
        arenaObject->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mgmrensho, 31), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(arenaObject->mdlId[0], 1U);
        Hu3DModelLayerSet(arenaObject->mdlId[0], 1);
        arenaObject->mtnId[0] = Hu3DJointMotion(arenaObject->mdlId[0], HuDataSelHeapReadNum(
            DATANUM(DATA_mgmrensho, 32), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DMotionSet(arenaObject->mdlId[0], arenaObject->mtnId[0]);
        Hu3DModelAttrSet(arenaObject->mdlId[0], HU3D_MOTATTR_LOOP);

        arenaObject->mdlId[1] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mgmrensho, 33), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(arenaObject->mdlId[1], 1U);
        Hu3DModelLayerSet(arenaObject->mdlId[1], 1);
        arenaObject->mtnId[2] = Hu3DJointMotion(arenaObject->mdlId[1], HuDataSelHeapReadNum(
            DATANUM(DATA_mgmrensho, 34), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DMotionSet(arenaObject->mdlId[1], arenaObject->mtnId[2]);
        Hu3DModelAttrSet(arenaObject->mdlId[1], HU3D_MOTATTR_LOOP);

        arenaObject->mdlId[2] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mgmrensho, 35), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(arenaObject->mdlId[2], 1U);
        Hu3DModelLayerSet(arenaObject->mdlId[2], 1);
        arenaObject->mtnId[4] = Hu3DJointMotion(arenaObject->mdlId[2], HuDataSelHeapReadNum(
            DATANUM(DATA_mgmrensho, 36), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DMotionSet(arenaObject->mdlId[2], arenaObject->mtnId[4]);
        Hu3DModelAttrSet(arenaObject->mdlId[2], HU3D_MOTATTR_LOOP);
    }
    if ((MgModePlayNum > 76U) && (MgModePlayNum <= 80U)) {
        arenaObject->work[0] = 0;
        for (modelIndex = 0; modelIndex < 4; modelIndex++) {
            lbl_1_bss_11C[modelIndex] = frandmod(30) - 31;
            if (((MgModePlayNum == 77U) || (MgModePlayNum == 78U)) &&
                (modelIndex != 0)) {
                continue;
            }
            if (((MgModePlayNum == 79U) || (MgModePlayNum == 80U)) &&
                ((modelIndex != 1) && (modelIndex != 2) && (modelIndex != 3))) {
                continue;
            }
            if ((MgModePlayNum != 80U) && (modelIndex == 2)) {
                continue;
            }
            arenaObject->mdlId[modelIndex + 3] = Hu3DModelCreate(HuDataSelHeapReadNum(
                DATANUM(DATA_mgmrensho, 37), HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelCameraSet(arenaObject->mdlId[modelIndex + 3], 1U);
            Hu3DModelLayerSet(arenaObject->mdlId[modelIndex + 3], 1);
            arenaObject->mtnId[(modelIndex * 2) + 6] = Hu3DJointMotion(
                arenaObject->mdlId[modelIndex + 3], HuDataSelHeapReadNum(
                    DATANUM(DATA_mgmrensho, 38), HU_MEMNUM_OVL, HEAP_MODEL));
            arenaObject->mtnId[(modelIndex * 2) + 7] = Hu3DJointMotion(
                arenaObject->mdlId[modelIndex + 3], HuDataSelHeapReadNum(
                    DATANUM(DATA_mgmrensho, 39), HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DMotionSet(arenaObject->mdlId[modelIndex + 3],
                          arenaObject->mtnId[(modelIndex * 2) + 6]);
            Hu3DModelAttrSet(arenaObject->mdlId[modelIndex + 3], HU3D_MOTATTR_LOOP);
        }
    }
}

/* Allows the hopping phase timers to advance; the third model's timer advances either way. */
void fn_1_8B1C(s16 advancePhaseTimers) {
    if (advancePhaseTimers != 0) {
        lbl_1_bss_118->work[0] = 1;
        return;
    }
    lbl_1_bss_118->work[0] = 0;
}

/* Runs each frame to place the arena models and advance the hopping characters. */
void fn_1_8B58(OMOBJ *arenaObject)
{
    Point3d position;
    s16 index;

    if (arenaObject->mdlId[0] >= 0) {
        position = lbl_1_data_2E4[0];
        position.z += fn_1_4ADC();
        Hu3DModelPosSetV(arenaObject->mdlId[0], &position);
        Hu3DModelRotSet(arenaObject->mdlId[0], 0.0f, 60.0f, 0.0f);
    }
    if (arenaObject->mdlId[1] >= 0) {
        position = lbl_1_data_2E4[1];
        position.z += fn_1_4ADC();
        Hu3DModelPosSetV(arenaObject->mdlId[1], &position);
        Hu3DModelRotSet(arenaObject->mdlId[1], 0.0f, 60.0f, 0.0f);
    }
    if (arenaObject->mdlId[2] >= 0) {
        position = lbl_1_data_2E4[2];
        position.z += fn_1_4ADC();
        Hu3DModelPosSetV(arenaObject->mdlId[2], &position);
        Hu3DModelRotSet(arenaObject->mdlId[2], 0.0f, 60.0f, 0.0f);
    }

    index = 0;
    while (index < 4) {
        if (arenaObject->mdlId[index + 3] >= 0) {
            if (index == 2) {
                lbl_1_bss_11C[index] += 1;
                if (lbl_1_bss_11C[index] > 0) {
                    lbl_1_bss_11C[index] = frandmod(80) - 150;
                }
                Hu3DModelRotSet(arenaObject->mdlId[index + 3], 0.0f, 90.0f,
                                0.0f);
                position = lbl_1_data_2E4[index + 3];
                position.z += fn_1_4ADC();
                Hu3DModelPosSetV(arenaObject->mdlId[index + 3], &position);
            } else {
                if (arenaObject->work[0] == 0) {
                    lbl_1_bss_11C[index] -= 1;
                }
                lbl_1_bss_11C[index] += 1;
                if (lbl_1_bss_11C[index] > 150) {
                    lbl_1_bss_11C[index] = frandmod(50) - 100;
                }
                if (lbl_1_bss_11C[index] >= 150) {
                    Hu3DMotionShiftSet(arenaObject->mdlId[index + 3],
                                       arenaObject->mtnId[(index * 2) + 6], 0.0f,
                                       8.0f, HU3D_MOTATTR_LOOP);
                }
                Hu3DModelRotSet(arenaObject->mdlId[index + 3], 0.0f, 90.0f,
                                0.0f);
                position = lbl_1_data_2E4[index + 3];
                position.z += fn_1_4ADC();
                if (lbl_1_bss_11C[index] >= 0) {
                    if (lbl_1_bss_11C[index] < 50) {
                        position.x += 2.0f * (f32)lbl_1_bss_11C[index];
                        if (lbl_1_bss_11C[index] == 0) {
                            Hu3DMotionShiftSet(arenaObject->mdlId[index + 3],
                                               arenaObject->mtnId[(index * 2) + 7], 0.0f,
                                               8.0f, HU3D_MOTATTR_LOOP);
                            HuAudFXPlay(MSM_SE_GUIDE_77);
                        }
                    } else if (lbl_1_bss_11C[index] < 100) {
                        position.x += 100.0f;
                        if (lbl_1_bss_11C[index] == 50) {
                            fn_1_4EB0();
                            Hu3DMotionShiftSet(arenaObject->mdlId[index + 3],
                                               arenaObject->mtnId[(index * 2) + 6], 0.0f,
                                               8.0f, HU3D_MOTATTR_LOOP);
                        }
                    } else {
                        position.x += 2.0f *
                                      (f32)(50 - (lbl_1_bss_11C[index] - 100));
                        if (lbl_1_bss_11C[index] == 100) {
                            Hu3DMotionShiftSet(arenaObject->mdlId[index + 3],
                                               arenaObject->mtnId[(index * 2) + 7], 0.0f,
                                               8.0f, HU3D_MOTATTR_LOOP);
                        }
                    }
                }
                Hu3DModelPosSetV(arenaObject->mdlId[index + 3], &position);
            }
        }
        index++;
    }
}
