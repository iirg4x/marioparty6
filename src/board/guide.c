/* Creates and animates the board guide, including its fade particle effects. */
#define _MATH_H
#define M_PI 3.141592653589793
double sin(double);
double cos(double);

#include "game/board/guide.h"
#include "game/board/audio.h"

#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/data.h"
#include "game/frand.h"
#include "game/gamework.h"

#include "dolphin/mtx.h"

#define DATA_board (5 << 16)
#define GUIDE_OBJECT_NAME 1296189252
#define GUIDE_FADE_PARTICLE_CAPACITY 200
#define GUIDE_EFFECT_ALPHA_VARIATION 70
#define GUIDE_EFFECT_ALPHA_BASE 150

#define GUIDE_DATA_SUN_MODEL DATANUM(DATA_capsulechar4, 0)
#define GUIDE_DATA_MOON_MODEL DATANUM(DATA_capsulechar4, 27)
#define GUIDE_DATA_ATTACHED_MODEL DATANUM(DATA_capsulechar4, 53)
#define GUIDE_SOUND_FADE_IN MSM_SE_BRD00_116
#define GUIDE_SOUND_FADE_OUT MSM_SE_BRD00_117
#define GUIDE_DATA_FILE_01 DATANUM(DATA_capsulechar4, 1)
#define GUIDE_DATA_FILE_02 DATANUM(DATA_capsulechar4, 2)
#define GUIDE_DATA_FILE_03 DATANUM(DATA_capsulechar4, 3)
#define GUIDE_DATA_FILE_04 DATANUM(DATA_capsulechar4, 4)
#define GUIDE_DATA_FILE_05 DATANUM(DATA_capsulechar4, 5)
#define GUIDE_DATA_FILE_06 DATANUM(DATA_capsulechar4, 6)
#define GUIDE_DATA_FILE_07 DATANUM(DATA_capsulechar4, 7)
#define GUIDE_DATA_FILE_08 DATANUM(DATA_capsulechar4, 8)
#define GUIDE_DATA_FILE_09 DATANUM(DATA_capsulechar4, 9)
#define GUIDE_DATA_FILE_0A DATANUM(DATA_capsulechar4, 10)
#define GUIDE_DATA_FILE_0B DATANUM(DATA_capsulechar4, 11)
#define GUIDE_DATA_FILE_0C DATANUM(DATA_capsulechar4, 12)
#define GUIDE_DATA_FILE_0D DATANUM(DATA_capsulechar4, 13)
#define GUIDE_DATA_FILE_0E DATANUM(DATA_capsulechar4, 14)
#define GUIDE_DATA_FILE_0F DATANUM(DATA_capsulechar4, 15)
#define GUIDE_DATA_FILE_10 DATANUM(DATA_capsulechar4, 16)
#define GUIDE_DATA_FILE_11 DATANUM(DATA_capsulechar4, 17)
#define GUIDE_DATA_FILE_12 DATANUM(DATA_capsulechar4, 18)
#define GUIDE_DATA_FILE_13 DATANUM(DATA_capsulechar4, 19)
#define GUIDE_DATA_FILE_14 DATANUM(DATA_capsulechar4, 20)
#define GUIDE_DATA_FILE_15 DATANUM(DATA_capsulechar4, 21)
#define GUIDE_DATA_FILE_16 DATANUM(DATA_capsulechar4, 22)
#define GUIDE_DATA_FILE_17 DATANUM(DATA_capsulechar4, 23)
#define GUIDE_DATA_FILE_1C DATANUM(DATA_capsulechar4, 28)
#define GUIDE_DATA_FILE_1D DATANUM(DATA_capsulechar4, 29)
#define GUIDE_DATA_FILE_1E DATANUM(DATA_capsulechar4, 30)
#define GUIDE_DATA_FILE_1F DATANUM(DATA_capsulechar4, 31)
#define GUIDE_DATA_FILE_20 DATANUM(DATA_capsulechar4, 32)
#define GUIDE_DATA_FILE_21 DATANUM(DATA_capsulechar4, 33)
#define GUIDE_DATA_FILE_22 DATANUM(DATA_capsulechar4, 34)
#define GUIDE_DATA_FILE_23 DATANUM(DATA_capsulechar4, 35)
#define GUIDE_DATA_FILE_24 DATANUM(DATA_capsulechar4, 36)
#define GUIDE_DATA_FILE_25 DATANUM(DATA_capsulechar4, 37)
#define GUIDE_DATA_FILE_26 DATANUM(DATA_capsulechar4, 38)
#define GUIDE_DATA_FILE_27 DATANUM(DATA_capsulechar4, 39)
#define GUIDE_DATA_FILE_28 DATANUM(DATA_capsulechar4, 40)
#define GUIDE_DATA_FILE_29 DATANUM(DATA_capsulechar4, 41)
#define GUIDE_DATA_FILE_2A DATANUM(DATA_capsulechar4, 42)
#define GUIDE_DATA_FILE_2B DATANUM(DATA_capsulechar4, 43)
#define GUIDE_DATA_FILE_2C DATANUM(DATA_capsulechar4, 44)
#define GUIDE_DATA_FILE_2D DATANUM(DATA_capsulechar4, 45)
#define GUIDE_DATA_FILE_2E DATANUM(DATA_capsulechar4, 46)
#define GUIDE_DATA_FILE_2F DATANUM(DATA_capsulechar4, 47)
#define GUIDE_DATA_FILE_30 DATANUM(DATA_capsulechar4, 48)
#define GUIDE_DATA_FILE_31 DATANUM(DATA_capsulechar4, 49)

static int guideSunMotTbl[24] = {
    GUIDE_DATA_FILE_01, GUIDE_DATA_FILE_01, GUIDE_DATA_FILE_02, GUIDE_DATA_FILE_03,
    GUIDE_DATA_FILE_04, GUIDE_DATA_FILE_05, GUIDE_DATA_FILE_06, GUIDE_DATA_FILE_07,
    GUIDE_DATA_FILE_08, GUIDE_DATA_FILE_09, GUIDE_DATA_FILE_0A, GUIDE_DATA_FILE_0B,
    GUIDE_DATA_FILE_0C, GUIDE_DATA_FILE_0D, GUIDE_DATA_FILE_0E, GUIDE_DATA_FILE_0F,
    GUIDE_DATA_FILE_10, GUIDE_DATA_FILE_11, GUIDE_DATA_FILE_12, GUIDE_DATA_FILE_13,
    GUIDE_DATA_FILE_14, GUIDE_DATA_FILE_15, GUIDE_DATA_FILE_16, GUIDE_DATA_FILE_17,
};

static int guideMoonMotTbl[24] = {
    GUIDE_DATA_FILE_1C, GUIDE_DATA_FILE_1C, GUIDE_DATA_FILE_1D, GUIDE_DATA_FILE_1E,
    GUIDE_DATA_FILE_1F, GUIDE_DATA_FILE_20, GUIDE_DATA_FILE_21, GUIDE_DATA_FILE_22,
    GUIDE_DATA_FILE_23, GUIDE_DATA_FILE_23, GUIDE_DATA_FILE_24, GUIDE_DATA_FILE_25,
    GUIDE_DATA_FILE_26, GUIDE_DATA_FILE_27, GUIDE_DATA_FILE_28, GUIDE_DATA_FILE_29,
    GUIDE_DATA_FILE_2A, GUIDE_DATA_FILE_2B, GUIDE_DATA_FILE_2C, GUIDE_DATA_FILE_2D,
    GUIDE_DATA_FILE_2E, GUIDE_DATA_FILE_2F, GUIDE_DATA_FILE_30, GUIDE_DATA_FILE_31,
};

static int guideMdlFileTbl[2] = { GUIDE_DATA_SUN_MODEL, GUIDE_DATA_MOON_MODEL };
static int *guideMotTbl[2] = { guideSunMotTbl, guideMoonMotTbl };
static s8 guideDefaultMotTbl[5] = { 1, 7, 4, 17, -1 };

static void GuideOMExec(OMOBJ *obj);
static BOOL GuideFadeInUpdate(OMOBJ *obj);
static BOOL GuideFadeOutUpdate(OMOBJ *obj);
static MBMODELID GuideFadeInEffectCreate(OMOBJ *obj);
static MBMODELID GuideFadeOutEffectCreate(OMOBJ *obj);
static void GuideFadeInEffectHook(HU3D_MODEL *modelP, MBPARTICLE *effP, Mtx matrix);
static void GuideFadeOutEffectHook(HU3D_MODEL *modelP, MBPARTICLE *effP, Mtx matrix);

void mbGuideInit(void)
{
}

/* Board event setup calls this to create and place the selected sun or moon guide. */
OMOBJ *mbGuideCreate(int guideNo, HuVecF *pos, HuVecF *rot, s8 *motTbl, float scale, u32 attr)
{
    OMOBJ *obj;
    GUIDE_WORK *guideWork;
    MBMODELID modelId;
    int motId;
    int i;
    int motNo;
    s8 *motionEntry;
    u8 motEnable[25];

    for (i = 0; i < sizeof(motEnable); i++) {
        motEnable[i] = 0;
    }
    for (motionEntry = guideDefaultMotTbl; *motionEntry >= 0; ) {
        motEnable[*motionEntry++] = 1;
    }
    if (motTbl != NULL) {
        for (motionEntry = motTbl; *motionEntry >= 0; ) {
            motEnable[*motionEntry++] = 1;
        }
    }
    obj = omAddObjEx(mbObjMan, 256, 4, 0, OM_GRP_NONE, GuideOMExec);
    for (i = 0; i < 4; i++) {
        obj->mdlId[i] = -1;
    }
    omSetStatBit(obj, 1 << 8);
    guideWork = (GUIDE_WORK *)&obj->work[0];
    guideWork->name = GUIDE_OBJECT_NAME;
    guideWork->dispF = 1;
    guideWork->killF = 0;
    guideWork->altMtxF = 0;
    guideWork->motionF = 0;
    guideWork->screenF = 0;
    guideWork->mode = 0;
    guideWork->nextMotion = 0;
    modelId = (int)mbObjCreate(guideMdlFileTbl[guideNo], 0, 1);
    obj->mdlId[0] = modelId;
    if (guideNo == 1) {
        obj->mdlId[1] = mbObjCreate(GUIDE_DATA_ATTACHED_MODEL, 0, 1);
        mbObjHookSet(modelId, "itemhook_R", obj->mdlId[1]);
    }
    for (i = 1; i < sizeof(motEnable); i++) {
        if (motEnable[i]) {
            mbObjMotionNoCreate(modelId, guideMotTbl[guideNo][i], i);
            if (i == 11 || i == 16) {
                motId = mbObjMotionIDGet(modelId, i);
                Hu3DMotionAttrSet(motId, 1);
            }
        }
    }
    if (mbPauseProcCheck()) {
        omSetStatBit(obj, (1 << 7) | (1 << 5));
        mbObjAttrSet(modelId, HU3D_ATTR_NOPAUSE);
    }
    if (attr & MB_GUIDE_ATTR_SCREEN) {
        guideWork->screenF = 1;
        mbObjCameraSet(modelId, 4);
        mbObjLayerSet(modelId, 3);
    } else {
        mbObjLayerSet(modelId, 3);
    }
    if (pos) {
        mbObjPosSetV(modelId, pos);
    }
    if (rot) {
        mbObjRotSetV(modelId, rot);
    }
    mbObjScaleSet(modelId, scale, scale, scale);
    if (attr & MB_GUIDE_ATTR_LAYER) {
        motNo = 1;
    } else {
        motNo = 4;
    }
    mbObjMotionSet(modelId, motNo, HU3D_MOTATTR_LOOP);
    HuDataDirClose(GUIDE_DATA_SUN_MODEL);
    if (attr & MB_GUIDE_ATTR_ALTMTX) {
        guideWork->mode = 1;
        guideWork->phase = 0;
        while (!mbGuideIdleCheck(obj)) {
            HuPrcVSleep();
        }
    }
    return obj;
}

/* Board event setup uses these options to create the current guide. */
OMOBJ *mbGuideCreateFlag(HuVecF *pos, s8 *motTbl, BOOL screenF, BOOL altMtxF, BOOL layerF)
{
    u32 attr = MB_GUIDE_ATTR_NONE;
    if (screenF) {
        attr |= MB_GUIDE_ATTR_SCREEN;
    }
    if (altMtxF) {
        attr |= MB_GUIDE_ATTR_ALTMTX;
    }
    if (layerF) {
        attr |= MB_GUIDE_ATTR_LAYER;
    }
    return mbGuideCreate(mbGuideNoGet(), pos, NULL, motTbl, 1.0f, attr);
}

/* Board event setup calls this to create the current guide with default options. */
OMOBJ *mbGuideCreateIn(void)
{
    return mbGuideCreate(mbGuideNoGet(), NULL, NULL, NULL, 1.0f, MB_GUIDE_ATTR_NONE);
}

/* Board event scripts call this to hide the guide and let GuideOMExec release it. */
void mbGuideKill(OMOBJ *obj)
{
    GUIDE_WORK *guideWork = (GUIDE_WORK *)&obj->work[0];
    guideWork->killF = 1;
    if (obj->mdlId[0] != 0) {
        mbObjDispSet(obj->mdlId[0], FALSE);
    }
}

/* Board event scripts run the exit fade and mark the guide for cleanup; endF does not change this
 * behavior. */
void mbGuideEnd(OMOBJ *obj, BOOL endF)
{
    GUIDE_WORK *guideWork = (GUIDE_WORK *)&obj->work[0];
    guideWork->mode = 3;
    guideWork->phase = 0;
    while (!mbGuideIdleCheck(obj)) {
        HuPrcVSleep();
    }
    guideWork->killF = 1;
    if (obj->mdlId[0] != 0) {
        mbObjDispSet(obj->mdlId[0], FALSE);
    }
}

/* Board event scripts call this to run the entrance fade and wait for GuideOMExec to finish. */
void mbGuideFadeIn(OMOBJ *obj)
{
    GUIDE_WORK *guideWork = (GUIDE_WORK *)&obj->work[0];
    guideWork->mode = 1;
    guideWork->phase = 0;
    while (!mbGuideIdleCheck(obj)) {
        HuPrcVSleep();
    }
}

/* Board event scripts call this to run the exit fade and hide the guide after GuideOMExec
 * finishes. */
void mbGuideFadeOut(OMOBJ *obj)
{
    GUIDE_WORK *guideWork = (GUIDE_WORK *)&obj->work[0];
    guideWork->mode = 3;
    guideWork->phase = 0;
    while (!mbGuideIdleCheck(obj)) {
        HuPrcVSleep();
    }
    if (obj->mdlId[0] != 0) {
        mbObjDispSet(obj->mdlId[0], FALSE);
    }
}

/* Board event scripts call this to get the main model ID unless cleanup has been requested. */
int mbGuideModelGet(OMOBJ *obj)
{
    GUIDE_WORK *guideWork = (GUIDE_WORK *)&obj->work[0];
    MBMODELID modelId = 0;
    if (!guideWork->killF) {
        modelId = obj->mdlId[0];
    }
    return modelId;
}

/* Board event scripts call this to switch cameras; world mode also resets the model matrix. */
void mbGuideScreenSet(OMOBJ *obj, BOOL screenF)
{
    GUIDE_WORK *guideWork = (GUIDE_WORK *)&obj->work[0];
    MBMODELID modelId = 0;
    Mtx mtx;
    if (!guideWork->killF) {
        modelId = obj->mdlId[0];
        if (screenF) {
            guideWork->screenF = 1;
            mbObjCameraSet(modelId, 4);
            mbObjLayerSet(modelId, 3);
        } else {
            guideWork->screenF = 0;
            mbObjCameraSet(modelId, 1);
            mbObjLayerSet(modelId, 3);
            PSMTXIdentity(mtx);
            mbObjMtxSet(modelId, &mtx);
        }
    }
}

/* Fade callers use this to wait until GuideOMExec returns the guide to idle mode. */
BOOL mbGuideIdleCheck(OMOBJ *obj)
{
    GUIDE_WORK *guideWork = (GUIDE_WORK *)&obj->work[0];
    return guideWork->mode == 0;
}

/* Board event scripts queue the motion GuideOMExec starts after the current motion ends while
 * completion tracking is enabled. */
void mbGuideMotionNextSet(OMOBJ *obj, s16 motNo)
{
    GUIDE_WORK *guideWork = (GUIDE_WORK *)&obj->work[0];
    guideWork->nextMotion = motNo;
}

/* Board event scripts set a guide motion and record which motion should follow it. */
void mbGuideMotionSet(OMOBJ *obj, s16 motNo, BOOL shiftF)
{
    GUIDE_WORK *guideWork = (GUIDE_WORK *)&obj->work[0];
    MBMODELID modelId = obj->mdlId[0];
    if (shiftF) {
        mbObjMotionShiftSet(modelId, motNo, 0.0f, 12.0f, TRUE);
    } else {
        mbObjMotionSet(modelId, motNo, 1);
    }
    guideWork->motionF = 0;
    guideWork->nextMotion = motNo;
}

/* Board event scripts set a guide motion and enable completion tracking; when it ends, GuideOMExec
 * starts the stored nextMotion. */
void mbGuideMotionShiftSet(OMOBJ *obj, s16 motNo, BOOL shiftF)
{
    GUIDE_WORK *guideWork = (GUIDE_WORK *)&obj->work[0];
    MBMODELID modelId = obj->mdlId[0];
    if (shiftF) {
        mbObjMotionShiftSet(modelId, motNo, 0.0f, 12.0f, FALSE);
    } else {
        mbObjMotionSet(modelId, motNo, 0);
    }
    guideWork->motionF = 1;
}

/* Board event scripts call this to stop GuideOMExec from waiting on a motion completion. */
void mbGuideMotionStop(OMOBJ *obj)
{
    GUIDE_WORK *guideWork = (GUIDE_WORK *)&obj->work[0];
    guideWork->motionF = 0;
}

/* Board event scripts use this to check motion-wait status; it returns FALSE while GuideOMExec
 * waits for completion and TRUE otherwise. */
BOOL mbGuideMotionCheck(OMOBJ *obj)
{
    GUIDE_WORK *guideWork = (GUIDE_WORK *)&obj->work[0];
    return guideWork->motionF != 1;
}

/* The object manager calls this each update to advance guide motion and fades, clean up killed
 * guides, and face screen-space guides toward their camera. */
static void GuideOMExec(OMOBJ *obj)
{
    GUIDE_WORK *work = (GUIDE_WORK *)&obj->work[0];
    int i;
    HU3D_CAMERA *camP;
    MBMODELID modelId;
    HuVecF pos;
    HuVecF scale;
    Mtx mtx;

    if (mbExitCheck() || work->killF) {
        if (obj->mdlId[0] > 0) {
            mbObjKill(obj->mdlId[0]);
        }
        if (obj->mdlId[1] > 0) {
            mbObjKill(obj->mdlId[1]);
        }
        obj->mdlId[0] = 0;
        obj->mdlId[1] = 0;
        if (obj->mdlId[2] >= 0) {
            mbParticleKill(obj->mdlId[2]);
        }
        if (obj->mdlId[3] >= 0) {
            mbParticleKill(obj->mdlId[3]);
        }
        obj->mdlId[2] = -1;
        obj->mdlId[3] = -1;
        work->name = 0;
        omDelObjEx(HuPrcCurrentGet(), obj);
        return;
    }
    switch (work->mode) {
        case 0:
            if (work->motionF) {
                if (mbObjMotionShiftIDGet(obj->mdlId[0]) < 0) {
                    if (mbObjMotionEndCheck(obj->mdlId[0])) {
                        mbObjMotionShiftSet(obj->mdlId[0], work->nextMotion, 0.0f, 12.0f, TRUE);
                        work->motionF = 0;
                    }
                }
            }
            break;

        case 1:
            if (GuideFadeInUpdate(obj)) {
                work->mode = 0;
            }
            break;

        case 2:
            break;

        case 3:
            if (GuideFadeOutUpdate(obj)) {
                work->mode = 0;
            }
            break;

        case 4:
            break;
    }
    if (work->screenF) {
        for (i = 0; i < 16; i++) {
            if ((1 << i) & 4) {
                break;
            }
        }
        camP = &Hu3DCamera[i];
        modelId = obj->mdlId[0];
        mbObjPosGet(modelId, &pos);
        mbObjScaleGet(modelId, &scale);
        pos.y = pos.y + 150.0f * scale.y;
        C_MTXLookAt(mtx, &camP->pos, &camP->up, &pos);
        PSMTXInverse(mtx, mtx);
        mbObjMtxSet(modelId, &mtx);
    }
}

/* GuideOMExec calls this each update to animate the entrance fade until the guide is visible. */
static BOOL GuideFadeInUpdate(OMOBJ *obj)
{
    GUIDE_WORK *work = (GUIDE_WORK *)&obj->work[0];
    HuVecF pos;
    HuVecF rot = { 0.0f, 0.0f, 0.0f };
    BOOL done = FALSE;
    MBMODELID modelId = obj->mdlId[0];
    MBPARTICLE *effP;
    MBMODELID particleId;
    MBPARTICLE *effP2;
    float weight;

    switch (work->phase) {
        case 0:
            mbObjDispSet(modelId, TRUE);
            mbObjPosGet(modelId, &pos);
            obj->trans.x = pos.x;
            obj->trans.y = pos.y;
            obj->trans.z = pos.z;
            pos.y += 200.0f;
            mbObjFadeCreate(modelId, &pos);
            mbObjFadeTexColorSet(modelId, 128, 128, 128, 0.5f);
            if (obj->mdlId[1] > 0) {
                mbObjFadeCreate((s16)obj->mdlId[1], &pos);
                mbObjFadeTexColorSet((s16)obj->mdlId[1], 128, 128, 128, 0.5f);
            }
            obj->mdlId[2] = GuideFadeInEffectCreate(obj);
            Hu3DModelPosSetV(obj->mdlId[2], &pos);
            work->time = 0;
            work->timeMax = 60;
            work->phase++;
            mbAudFXPlay(GUIDE_SOUND_FADE_IN);
            break;

        case 1:
            work->time = 0;
            work->timeMax = 60;
            work->phase++;
            break;

        case 2:
            work->time++;
            weight = (float)work->time / work->timeMax;
            pos.x = obj->trans.x;
            pos.y = (obj->trans.y - 50.0f) + 250.0f * (1.0f - weight);
            pos.z = obj->trans.z;
            mbObjFadeTexRotSet(modelId, &pos, &rot);
            if (obj->mdlId[1] > 0) {
                mbObjFadeTexRotSet((s16)obj->mdlId[1], &pos, &rot);
            }
            if (pos.y <= obj->trans.y) {
                particleId = obj->mdlId[2];
                effP2 = Hu3DData[particleId].hookData;
                effP = effP2;
                effP->stopF = 1;
            } else {
                Hu3DModelPosSetV(obj->mdlId[2], &pos);
            }
            if (work->time >= work->timeMax) {
                work->timeMax = 30;
                work->time = 0;
                work->phase++;
            }
            break;

        case 3:
            work->time++;
            if (work->time >= work->timeMax) {
                work->phase++;
            } else {
                break;
            }
            /* fallthrough */

        case 4:
            mbObjFadeKill(modelId);
            if (obj->mdlId[1] > 0) {
                mbObjFadeKill((s16)obj->mdlId[1]);
            }
            mbParticleKill(obj->mdlId[2]);
            obj->mdlId[2] = -1;
            done = TRUE;
            break;
    }
    return done;
}

/* GuideOMExec calls this each update to animate the exit fade until the guide is hidden. */
static BOOL GuideFadeOutUpdate(OMOBJ *obj)
{
    GUIDE_WORK *work = (GUIDE_WORK *)&obj->work[0];
    HuVecF pos;
    HuVecF rot = { 0.0f, 0.0f, 0.0f };
    BOOL done = FALSE;
    MBMODELID modelId = obj->mdlId[0];
    MBPARTICLE *effP;
    MBMODELID particleId;
    MBPARTICLE *effP2;
    float weight;

    switch (work->phase) {
        case 0:
            mbObjPosGet(modelId, &pos);
            obj->trans.x = pos.x;
            obj->trans.y = pos.y;
            obj->trans.z = pos.z;
            pos.y -= 50.0f;
            mbObjFadeCreate(modelId, &pos);
            mbObjFadeTexColorSet(modelId, 128, 128, 128, 0.5f);
            if (obj->mdlId[1] > 0) {
                mbObjFadeCreate((s16)obj->mdlId[1], &pos);
                mbObjFadeTexColorSet((s16)obj->mdlId[1], 128, 128, 128, 0.5f);
            }
            obj->mdlId[2] = GuideFadeOutEffectCreate(obj);
            Hu3DModelPosSetV(obj->mdlId[2], &obj->trans);
            work->time = 0;
            work->timeMax = 60;
            work->phase++;
            mbAudFXPlay(GUIDE_SOUND_FADE_OUT);
            break;

        case 1:
            work->time = 0;
            work->timeMax = 60;
            work->phase++;
            break;

        case 2:
            work->time++;
            weight = (float)work->time / work->timeMax;
            pos.x = obj->trans.x;
            pos.y = (obj->trans.y - 50.0f) + 250.0f * weight;
            pos.z = obj->trans.z;
            mbObjFadeTexRotSet(modelId, &pos, &rot);
            if (obj->mdlId[1] > 0) {
                mbObjFadeTexRotSet((s16)obj->mdlId[1], &pos, &rot);
            }
            if (pos.y >= obj->trans.y) {
                Hu3DModelPosSetV(obj->mdlId[2], &pos);
            }
            if (pos.y >= obj->trans.y + 160.0f) {
                particleId = obj->mdlId[2];
                effP2 = Hu3DData[particleId].hookData;
                effP = effP2;
                effP->stopF = 1;
            }
            if (work->time >= work->timeMax) {
                work->timeMax = 30;
                work->time = 0;
                work->phase++;
            }
            break;

        case 3:
            work->time++;
            if (work->time >= work->timeMax) {
                work->phase++;
            } else {
                break;
            }
            /* fallthrough */

        case 4:
            mbObjDispSet(obj->mdlId[0], FALSE);
            mbObjFadeKill(modelId);
            if (obj->mdlId[1] > 0) {
                mbObjDispSet(obj->mdlId[1], FALSE);
                mbObjFadeKill((s16)obj->mdlId[1]);
            }
            mbParticleKill(obj->mdlId[2]);
            obj->mdlId[2] = -1;
            done = TRUE;
            break;
    }
    return done;
}

/* Guide creation and speaker lookup call this to select the sun guide outside party mode or on day
 * one, and the moon guide otherwise. */
int mbGuideNoGet(void)
{
    int no = 0;
    int party = GwSystem.partyF;
    if (party == 0) {
        no = 0;
    } else if (GwSystem.curTime == 0) {
        no = 0;
    } else {
        no = 1;
    }
    return no;
}

/* Board message setup calls this to select the sun or moon guide's speaker slot. */
int mbGuideSpeakerNoGet(void)
{
    static int speakerTbl[2] = { 6, 7 };
    return speakerTbl[mbGuideNoGet()];
}

/* GuideFadeInUpdate calls this to create the entrance sparkles and attach their particle
 * callback. */
static MBMODELID GuideFadeInEffectCreate(OMOBJ *obj)
{
    int particleId;
    Mtx mtx;
    particleId = mbParticleCreate(HuSprAnimDataRead(mbBoardDataNumGet(DATANUM(DATA_board, 100))),
                                  GUIDE_FADE_PARTICLE_CAPACITY);
    mbParticleHookSet(particleId, GuideFadeInEffectHook);
    Hu3DModelCameraSet(particleId, mbObjGet(obj->mdlId[0])->cameraBit);
    Hu3DModelLayerSet(particleId, mbObjGet(obj->mdlId[0])->layer + 1);
    mbObjMtxGet(obj->mdlId[0], &mtx);
    Hu3DModelMtxSet(particleId, &mtx);
    if (mbPauseProcCheck()) {
        Hu3DModelAttrSet(particleId, HU3D_ATTR_NOPAUSE);
    }
    return particleId;
}

/* GuideFadeOutUpdate calls this to create the exit sparkles and attach their particle callback. */
static MBMODELID GuideFadeOutEffectCreate(OMOBJ *obj)
{
    int particleId;
    Mtx mtx;
    particleId = mbParticleCreate(HuSprAnimDataRead(mbBoardDataNumGet(DATANUM(DATA_board, 100))),
                                  GUIDE_FADE_PARTICLE_CAPACITY);
    mbParticleHookSet(particleId, GuideFadeOutEffectHook);
    Hu3DModelCameraSet(particleId, mbObjGet(obj->mdlId[0])->cameraBit);
    Hu3DModelLayerSet(particleId, mbObjGet(obj->mdlId[0])->layer + 1);
    mbObjMtxGet(obj->mdlId[0], &mtx);
    Hu3DModelMtxSet(particleId, &mtx);
    if (mbPauseProcCheck()) {
        Hu3DModelAttrSet(particleId, HU3D_ATTR_NOPAUSE);
    }
    return particleId;
}

/* The particle system calls this callback to emit and animate the entrance sparkles. */
static void GuideFadeInEffectHook(HU3D_MODEL *model, MBPARTICLE *particleSystemP, Mtx modelMtx)
{
    static u8 effNo[16] = { 0, 1, 2, 2, 3, 3, 3, 3, 0, 1, 2, 2, 3, 3, 3, 3 };
    static float effSize[4] = { 0.9f, 0.9f, 0.9f, 0.8f };
    static GXColor effColor[8] = {
        { 220, 64, 64, 0 }, { 64, 220, 64, 0 },
        { 220, 220, 64, 0 }, { 64, 64, 220, 0 },
        { 220, 64, 220, 0 }, { 64, 220, 220, 0 },
        { 220, 120, 64, 0 }, { 64, 120, 220, 0 },
    };
    MBPARTICLEDATA *particleData;
    int particleIndex;
    int particlesToSpawn;
    int particleShape;
    int colorIndex;
    float randomValue;
    float riseWeight;

    if (particleSystemP->initF == 0) {
        particleSystemP->initF = 1;
        particleSystemP->stopF = 0;
        particleSystemP->spawnOffsetY = 0.0f;
    }
    particlesToSpawn = 0;
    if (particleSystemP->stopF == 0) {
        particlesToSpawn = 6;
    }
    particleData = particleSystemP->data;
    for (particleIndex = 0; particleIndex < particleSystemP->num; particleIndex++, particleData++) {
        if (particlesToSpawn <= 0) {
            break;
        }
        if (particleData->time == 0) {
            particleShape = effNo[mbRandMod(16)];
            particleData->rndNo = particleShape;
            particleData->vel.x = 360.0f * frandf();
            particleData->vel.y = 80.0f * (0.3f + 0.7f * frandf());
            particleData->vel.z = 130.0f * (0.7f + 0.3f * frandf());
            particleData->scale = effSize[particleShape] * (35.0f * (0.5f + 0.5f * frandf()));
            particleData->guideScaleBase = particleData->scale;
            colorIndex = mbRandMod(8);
            randomValue = 0.3f * frandf();
            particleData->color.r =
                randomValue * (255.0f - effColor[colorIndex].r) + effColor[colorIndex].r;
            particleData->color.g =
                randomValue * (255.0f - effColor[colorIndex].g) + effColor[colorIndex].g;
            particleData->color.b =
                randomValue * (255.0f - effColor[colorIndex].b) + effColor[colorIndex].b;
            particleData->color.a =
                mbRandMod(GUIDE_EFFECT_ALPHA_VARIATION) + GUIDE_EFFECT_ALPHA_BASE;
            particleData->alphaF = particleData->color.a;
            particleData->color.a = 0;
            particleData->weight = 0.0f;
            if (particleData->rndNo < 3) {
                particleData->weight = 360.0f * frandf();
            }
            particleData->time = 30;
            particlesToSpawn--;
        }
    }
    particleData = particleSystemP->data;
    for (particleIndex = 0; particleIndex < particleSystemP->num; particleIndex++, particleData++) {
        if (particleData->time != 0) {
            randomValue = 0.033333335f * (float)particleData->time;
            particleData->vel.x += 5.0f;
            particleData->pos.x =
                randomValue * (particleData->vel.z * mbSinDeg(particleData->vel.x));
            particleData->pos.z =
                randomValue * (particleData->vel.z * mbCosDeg(particleData->vel.x));
            riseWeight = 1.6666666f * (randomValue - 0.4f);
            if (riseWeight < 0.0f) {
                riseWeight = 0.0f;
            }
            particleData->pos.y = particleData->vel.y * (randomValue * randomValue);
            particleData->scale -= 0.4f;
        }
    }
    particleData = particleSystemP->data;
    for (particleIndex = 0; particleIndex < particleSystemP->num; particleIndex++, particleData++) {
        if (particleData->time != 0) {
            particleData->time--;
            if (particleData->time < 10) {
                particleData->scale *= 0.95f;
                particleData->color.a *= 0.7f;
                if (particleData->time == 0) {
                    particleData->color.a = 0;
                    particleData->scale = 0.0f;
                }
            } else if (particleSystemP->stopF == 0) {
                randomValue = particleData->color.a;
                randomValue = 1.0f + (randomValue + 0.3f * (particleData->alphaF - randomValue));
                particleData->color.a = randomValue;
            }
            if (particleData->color.r < 250) {
                particleData->color.r += 5;
            }
            if (particleData->color.g < 250) {
                particleData->color.g += 5;
            }
            if (particleData->color.b < 250) {
                particleData->color.b += 5;
            }
            if (particleSystemP->stopF != 0) {
                particleData->color.a *= 0.8f;
            }
        }
    }
}

/* The particle system calls this callback to emit and animate the exit sparkles. */
static void GuideFadeOutEffectHook(HU3D_MODEL *model, MBPARTICLE *particleSystemP, Mtx modelMtx)
{
    static u8 effNo[16] = { 0, 1, 2, 2, 3, 3, 3, 3, 0, 1, 2, 2, 3, 3, 3, 3 };
    static float effSize[4] = { 0.9f, 0.9f, 0.9f, 0.8f };
    static GXColor effColor[8] = {
        { 220, 64, 64, 0 }, { 64, 220, 64, 0 },
        { 220, 220, 64, 0 }, { 64, 64, 220, 0 },
        { 220, 64, 220, 0 }, { 64, 220, 220, 0 },
        { 220, 120, 64, 0 }, { 64, 120, 220, 0 },
    };
    MBPARTICLEDATA *particleData;
    int particleIndex;
    int particlesToSpawn;
    int particleShape;
    int colorIndex;
    float randomValue;
    float velocityScale;
    HuVecF particleVector;
    Mtx particleRotation;

    if (particleSystemP->initF == 0) {
        particleSystemP->initF = 1;
        particleSystemP->stopF = 0;
        particleSystemP->spawnOffsetY = 0.0f;
    }
    particlesToSpawn = 0;
    if (particleSystemP->stopF == 0) {
        particlesToSpawn = 6;
    }
    particleData = particleSystemP->data;
    for (particleIndex = 0; particleIndex < particleSystemP->num; particleIndex++, particleData++) {
        if (particlesToSpawn <= 0) {
            break;
        }
        if (particleData->time == 0) {
            particleShape = effNo[mbRandMod(16)];
            particleData->rndNo = particleShape;
            randomValue = 360.0f * frandf();
            mbMtxRotAxisDeg(particleRotation, 'Y', randomValue);
            particleVector.x = particleVector.y = 0.0f;
            particleVector.z = 0.016666668f * (600.0f * (0.7f + 0.3f * frandf()));
            PSMTXMultVec(particleRotation, &particleVector, &particleData->vel);
            randomValue = 360.0f * frandf();
            velocityScale = 0.7f + 0.3f * frandf();
            particleVector.x = velocityScale * mbCosDeg(randomValue);
            particleVector.y = 0.2f + velocityScale * mbSinDeg(randomValue);
            particleVector.z = -0.8f;
            PSMTXMultVec(particleRotation, &particleVector, &particleData->guideAccel);
            PSVECScale(&particleData->guideAccel, &particleData->guideAccel, 0.55555564f);
            PSVECScale(&particleData->vel, &particleData->pos, 1.0f);
            particleData->pos.y += 5.0f;
            particleData->scale = effSize[particleShape] * (30.0f * (0.5f + 0.5f * frandf()));
            particleData->guideScaleBase = particleData->scale;
            colorIndex = mbRandMod(8);
            particleData->no = colorIndex;
            randomValue = 0.8f + 0.2f * frandf();
            particleData->color.r =
                randomValue * (255.0f - effColor[colorIndex].r) + effColor[colorIndex].r;
            particleData->color.g =
                randomValue * (255.0f - effColor[colorIndex].g) + effColor[colorIndex].g;
            particleData->color.b =
                randomValue * (255.0f - effColor[colorIndex].b) + effColor[colorIndex].b;
            particleData->color.a =
                mbRandMod(GUIDE_EFFECT_ALPHA_VARIATION) + GUIDE_EFFECT_ALPHA_BASE;
            particleData->alphaF = particleData->color.a;
            particleData->color.a = 0;
            particleData->weight = 0.0f;
            if (particleData->rndNo < 3) {
                particleData->weight = 360.0f * frandf();
            }
            particleData->time = 20;
            particlesToSpawn--;
        }
    }
    particleData = particleSystemP->data;
    for (particleIndex = 0; particleIndex < particleSystemP->num; particleIndex++, particleData++) {
        if (particleData->time != 0) {
            PSVECAdd(&particleData->pos, &particleData->vel, &particleData->pos);
            PSVECAdd(&particleData->vel, &particleData->guideAccel, &particleData->vel);
        }
    }
    particleData = particleSystemP->data;
    for (particleIndex = 0; particleIndex < particleSystemP->num; particleIndex++, particleData++) {
        if (particleData->time != 0) {
            particleData->time--;
            if (particleData->time < 10) {
                particleData->scale *= 0.95f;
                particleData->color.a *= 0.7f;
                if (particleData->time == 0) {
                    particleData->color.a = 0;
                    particleData->scale = 0.0f;
                }
            } else if (particleSystemP->stopF == 0) {
                randomValue = particleData->color.a;
                randomValue = 1.0f + (randomValue + 0.3f * (particleData->alphaF - randomValue));
                particleData->color.a = randomValue;
            }
            colorIndex = particleData->no;
            particleData->color.r = particleData->color.r +
                                    0.1f * ((float) effColor[colorIndex].r - particleData->color.r);
            particleData->color.g = particleData->color.g +
                                    0.1f * ((float) effColor[colorIndex].g - particleData->color.g);
            particleData->color.b = particleData->color.b +
                                    0.1f * ((float) effColor[colorIndex].b - particleData->color.b);
            if (particleSystemP->stopF != 0) {
                particleData->color.a *= 0.8f;
            }
        }
    }
}
