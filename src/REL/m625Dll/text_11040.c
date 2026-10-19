/* Falling stage objects, their water and collision effects, and hit sounds. */
#include "dolphin/math.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/hu3d.h"
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
#include "REL/m625Dll/motion.h"

#define MSM_SE_M625_WATER_SPLASH_1 1797
#define MSM_SE_M625_WATER_SPLASH_2 1798
#define MSM_SE_M625_OBJECT_HIT 1799

extern s16 GwMgNightF;
void fn_1_1BB0(Point3d *);
void fn_1_EF54(void);
void fn_1_10E90(Point3d, f32, f32, f32);
s32 fn_1_13318(Point3d, f32, Point3d, f32);
void fn_1_12464(s16);

/* Called during stage setup to create the falling-object model pool. */
void fn_1_11040(void)
{
    s16 index;
    s16 nightFlag;
    s16 nightFlagForEffect;

    nightFlag = GwMgNightF;
    if (nightFlag == 0) {
        lbl_1_bss_7C4 =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m625, 9), HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        lbl_1_bss_7C4 = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m625, 11), HU_MEMNUM_OVL, HEAP_MODEL));
    }
    Hu3DModelAttrSet(lbl_1_bss_7C4, HU3D_ATTR_DISPOFF);
    Hu3DZClearLayerSet(4);

    for (index = 0; index < 13; index++) {
        lbl_1_bss_4B8[index].state = -1;
        lbl_1_bss_4B8[index].primaryModel = Hu3DModelLink(lbl_1_bss_7C4);
        nightFlagForEffect = GwMgNightF;
        /* Both lighting modes use the same secondary effect model. */
        if (nightFlagForEffect == 0) {
            lbl_1_bss_4B8[index].secondaryModel = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m625, 10), HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            lbl_1_bss_4B8[index].secondaryModel = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m625, 10), HU_MEMNUM_OVL, HEAP_MODEL));
        }
        lbl_1_bss_4B8[index].position.x =
            lbl_1_bss_4B8[index].position.y =
            lbl_1_bss_4B8[index].position.z = 0.0f;
        lbl_1_bss_4B8[index].velocity.x =
            lbl_1_bss_4B8[index].velocity.y =
            lbl_1_bss_4B8[index].velocity.z = 0.0f;
        lbl_1_bss_4B8[index].fallSpeed = 0.0f;
        lbl_1_bss_4B8[index].positionOffset.x =
            lbl_1_bss_4B8[index].positionOffset.z = 0.0f;
        lbl_1_bss_4B8[index].positionOffset.y = 25.0f;
        lbl_1_bss_4B8[index].collisionRadius = 25.0f;
        lbl_1_bss_4B8[index].effectState = 0;
        lbl_1_bss_4B8[index].effectTimer = 0;
        lbl_1_bss_4B8[index].motionTimer = 0;
        Hu3DModelPosSet(lbl_1_bss_4B8[index].primaryModel, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(lbl_1_bss_4B8[index].primaryModel, HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_4B8[index].primaryModel, 4);
        Hu3DModelPosSet(lbl_1_bss_4B8[index].secondaryModel, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(lbl_1_bss_4B8[index].secondaryModel, HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_4B8[index].secondaryModel, 4);
    }
}

/* Called by stage logic to activate the first free falling object at a position. */
void fn_1_11470(Point3d position)
{
    s16 index;

    for (index = 0; index < 13; index++) {
        if (lbl_1_bss_4B8[index].state == -1) {
            lbl_1_bss_4B8[index].state = 0;
            lbl_1_bss_4B8[index].position = position;
            lbl_1_bss_4B8[index].velocity.x =
                lbl_1_bss_4B8[index].velocity.y =
                lbl_1_bss_4B8[index].velocity.z = 0.0f;
            lbl_1_bss_4B8[index].fallSpeed = 8.0f;
            lbl_1_bss_4B8[index].effectTimer = 0;
            lbl_1_bss_4B8[index].motionTimer = 0;
            Hu3DModelPosSetV(lbl_1_bss_4B8[index].primaryModel,
                             &lbl_1_bss_4B8[index].position);
            Hu3DModelAttrReset(lbl_1_bss_4B8[index].primaryModel, HU3D_ATTR_DISPOFF);
            Hu3DModelPosSetV(lbl_1_bss_4B8[index].secondaryModel,
                             &lbl_1_bss_4B8[index].position);
            Hu3DModelAttrSet(lbl_1_bss_4B8[index].secondaryModel, HU3D_ATTR_DISPOFF);
            lbl_1_bss_4B8[index].effectState = 0;
            break;
        }
    }
}

/* Called after a hit to switch falling objects into their blinking state. */
void fn_1_11698(void) {
    s16 effectIndex;

    effectIndex = 0;
    while (effectIndex < 13) {
        if (lbl_1_bss_4B8[effectIndex].state == 1) {
            lbl_1_bss_4B8[effectIndex].state = 2;
        }
        effectIndex += 1;
    }
}

s16 lbl_1_data_3E8[4] = { 0, 0, 0, 0 };

/* Called from the stage update each frame to advance object and splash effects. */
void fn_1_11700(void)
{
    s16 effectIndex;
    s16 previousEffectState;
    s16 particleIndex;
    s16 particleCount;
    Point3d particlePosition;
    Point3d effectPosition;

    for (effectIndex = 0; effectIndex < 13; effectIndex++) {
        switch (lbl_1_bss_4B8[effectIndex].state) {
        case 0:
            lbl_1_bss_4B8[effectIndex].state = 1;
            /* fall through */
        case 1:
            previousEffectState = lbl_1_bss_4B8[effectIndex].effectState;
            if (lbl_1_bss_4B8[effectIndex].position.y < 0.0f) {
                lbl_1_bss_4B8[effectIndex].fallSpeed = 4.0f;
                lbl_1_bss_4B8[effectIndex].effectState = 1;
            }
            lbl_1_bss_4B8[effectIndex].velocity.y = -lbl_1_bss_4B8[effectIndex].fallSpeed;
            if (lbl_1_bss_4B8[effectIndex].position.y < -800.0f) {
                lbl_1_bss_4B8[effectIndex].state = 6;
            }
            if (previousEffectState == 0 &&
                lbl_1_bss_4B8[effectIndex].effectState == 1) {
                Point3d *position;
                f32 panning;
                effectPosition = lbl_1_bss_4B8[effectIndex].position;
                position = &effectPosition;
                fn_1_1BB0(position);
                fn_1_EF54();
                panning = (lbl_1_bss_4B8[effectIndex].position.x - -600.0f) /
                          1200.0f;
                panning = 32.0f + 64.0f * panning;
                HuAudFXPanning(HuAudFXPlay(MSM_SE_M625_WATER_SPLASH_1), (s16)panning);
                HuAudFXPanning(HuAudFXPlay(MSM_SE_M625_WATER_SPLASH_2), (s16)panning);
            }
            if (lbl_1_bss_4B8[effectIndex].effectTimer == 0 &&
                lbl_1_bss_4B8[effectIndex].position.y <= -10.0f) {
                lbl_1_bss_4B8[effectIndex].effectTimer = 60;
            }
            if (lbl_1_bss_4B8[effectIndex].effectTimer > 0) {
                particleCount = (s16)(0.083333336f *
                    (f32)(60 - lbl_1_bss_4B8[effectIndex].effectTimer));
                particleCount = 5 - particleCount;
                for (particleIndex = 0; particleIndex < particleCount; particleIndex++) {
                    particlePosition = lbl_1_bss_4B8[effectIndex].position;
                    particlePosition.x += (f32)(frandmod(50) - 25);
                    particlePosition.y += (f32)frandmod(25);
                    fn_1_10E90(particlePosition, 8.0f,
                        1.0f, -10.0f);
                }
                lbl_1_bss_4B8[effectIndex].effectTimer -= 1;
                if (lbl_1_bss_4B8[effectIndex].effectTimer <= 0) {
                    lbl_1_bss_4B8[effectIndex].effectTimer = -1;
                }
            }
            break;
        case 2:
            if ((u32) (Hu3DModelAttrGet(lbl_1_bss_4B8[effectIndex].primaryModel) &
                       HU3D_ATTR_DISPOFF) != 0) {
                Hu3DModelAttrReset(lbl_1_bss_4B8[effectIndex].primaryModel, HU3D_ATTR_DISPOFF);
            } else {
                Hu3DModelAttrSet(lbl_1_bss_4B8[effectIndex].primaryModel, HU3D_ATTR_DISPOFF);
            }
            if (lbl_1_bss_4B8[effectIndex].position.y < 0.0f) {
                lbl_1_bss_4B8[effectIndex].fallSpeed = 4.0f;
            }
            lbl_1_bss_4B8[effectIndex].velocity.y = -lbl_1_bss_4B8[effectIndex].fallSpeed;
            if (lbl_1_bss_4B8[effectIndex].position.y < -800.0f) {
                lbl_1_bss_4B8[effectIndex].state = 6;
            }
            break;
        case 3:
            lbl_1_bss_4B8[effectIndex].motionTimer += 1;
            if (lbl_1_bss_4B8[effectIndex].motionTimer >= 0) {
                Hu3DModelAttrSet(lbl_1_bss_4B8[effectIndex].primaryModel, HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(lbl_1_bss_4B8[effectIndex].secondaryModel, HU3D_ATTR_DISPOFF);
                Hu3DModelAttrSet(lbl_1_bss_4B8[effectIndex].secondaryModel, HU3D_MOTATTR_LOOP);
                Hu3DModelAttrReset(lbl_1_bss_4B8[effectIndex].secondaryModel, HU3D_MOTATTR_PAUSE);
                Hu3DMotionTimeSet(lbl_1_bss_4B8[effectIndex].secondaryModel, 0.0f);
                Hu3DModelPosSetV(lbl_1_bss_4B8[effectIndex].secondaryModel,
                                 &lbl_1_bss_4B8[effectIndex].position);
                lbl_1_bss_4B8[effectIndex].state = 4;
                /* fall through */
            case 4:
                if ((Hu3DMotionTimeGet(lbl_1_bss_4B8[effectIndex].secondaryModel) +
                     Hu3DData[lbl_1_bss_4B8[effectIndex].secondaryModel].motWork.speed) >= 5.0f) {
                    lbl_1_bss_4B8[effectIndex].state = 5;
                }
                break;
            }
            break;
        case 5: {
            f32 maximumTime;

            previousEffectState = lbl_1_bss_4B8[effectIndex].effectState;
            if ((Hu3DData[lbl_1_bss_4B8[effectIndex].secondaryModel].motWork.speed +
                     Hu3DMotionTimeGet(lbl_1_bss_4B8[effectIndex].secondaryModel)) >= 10.0f) {
                if ((u32) (Hu3DModelAttrGet(lbl_1_bss_4B8[effectIndex].secondaryModel) &
                           HU3D_ATTR_DISPOFF) != 0) {
                    Hu3DModelAttrReset(lbl_1_bss_4B8[effectIndex].secondaryModel,
                                       HU3D_ATTR_DISPOFF);
                } else {
                    Hu3DModelAttrSet(lbl_1_bss_4B8[effectIndex].secondaryModel, HU3D_ATTR_DISPOFF);
                }
            }
            if (lbl_1_bss_4B8[effectIndex].position.y < 0.0f) {
                lbl_1_bss_4B8[effectIndex].fallSpeed = 4.0f;
                lbl_1_bss_4B8[effectIndex].effectState = 1;
            }
            lbl_1_bss_4B8[effectIndex].velocity.y = -lbl_1_bss_4B8[effectIndex].fallSpeed;
            maximumTime = Hu3DMotionMaxTimeGet(lbl_1_bss_4B8[effectIndex].secondaryModel);
            if ((Hu3DData[lbl_1_bss_4B8[effectIndex].secondaryModel].motWork.speed +
                 Hu3DMotionTimeGet(lbl_1_bss_4B8[effectIndex].secondaryModel)) >= maximumTime) {
                Hu3DModelAttrReset(lbl_1_bss_4B8[effectIndex].secondaryModel, HU3D_MOTATTR_LOOP);
                Hu3DModelAttrSet(lbl_1_bss_4B8[effectIndex].secondaryModel, HU3D_MOTATTR_PAUSE);
                lbl_1_bss_4B8[effectIndex].state = 6;
            }
            break;
        }
        case 6:
            lbl_1_bss_4B8[effectIndex].state = -1;
            lbl_1_bss_4B8[effectIndex].velocity.x =
                lbl_1_bss_4B8[effectIndex].velocity.y =
                lbl_1_bss_4B8[effectIndex].velocity.z = 0.0f;
            lbl_1_bss_4B8[effectIndex].fallSpeed = 0.0f;
            Hu3DModelAttrSet(lbl_1_bss_4B8[effectIndex].primaryModel, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_4B8[effectIndex].secondaryModel, HU3D_ATTR_DISPOFF);
            break;
        }
    }
}

/* During play, detects hits on falling objects, zeroes each hit object's X/Y velocity, and
* starts its splash animation and hit sound. */
s32 fn_1_12254(Point3d position, Point3d *movement, f32 collisionRadius)
{
    Point3d predictedCenter;
    s16 hitCount;
    s16 index;

    hitCount = 0;
    if ((u32)MgSeqTimerValueGet() == 0) {
        return 0;
    }
    for (index = 0; index < 13; index++) {
        if (lbl_1_bss_4B8[index].state == 1) {
            position.x += movement->x;
            position.y += movement->y;
            position.z = 0.0f;
            predictedCenter = lbl_1_bss_4B8[index].position;
            predictedCenter.x += lbl_1_bss_4B8[index].velocity.x;
            predictedCenter.y += lbl_1_bss_4B8[index].velocity.y;
            predictedCenter.z = 0.0f;
            if (fn_1_13318(position, collisionRadius, predictedCenter,
                           lbl_1_bss_4B8[index].collisionRadius)) {
                hitCount++;
                lbl_1_bss_4B8[index].velocity.x = 0.0f;
                lbl_1_bss_4B8[index].velocity.y = 0.0f;
                fn_1_12464(index);
            }
        }
    }
    if (hitCount != 0) {
        return 1;
    }
    return 0;
}

/* Called after a collision to start the object's splash animation and hit sound. */
void fn_1_12464(s16 index)
{
    f32 pan;

    if (index >= 0 && index < 13 && lbl_1_bss_4B8[index].state == 1) {
        lbl_1_bss_4B8[index].state = 3;
        pan = (lbl_1_bss_4B8[index].position.x - -600.0f) / 1200.0f;
        pan = 32.0f + 64.0f * pan;
        HuAudFXPanning(HuAudFXPlay(MSM_SE_M625_OBJECT_HIT), (s16)pan);
    }
}

/* Called after physics to apply each object's accumulated movement to both models. */
void fn_1_12560(void)
{
    s16 index;

    for (index = 0; index < 13; index++) {
        lbl_1_bss_4B8[index].position.x += lbl_1_bss_4B8[index].velocity.x;
        lbl_1_bss_4B8[index].position.y += lbl_1_bss_4B8[index].velocity.y;
        lbl_1_bss_4B8[index].position.z += lbl_1_bss_4B8[index].velocity.z;
        lbl_1_bss_4B8[index].velocity.x =
            lbl_1_bss_4B8[index].velocity.y =
                lbl_1_bss_4B8[index].velocity.z = 0.0f;
        Hu3DModelPosSetV(lbl_1_bss_4B8[index].primaryModel, &lbl_1_bss_4B8[index].position);
        Hu3DModelPosSetV(lbl_1_bss_4B8[index].secondaryModel, &lbl_1_bss_4B8[index].position);
    }
}

/* Used by player selection logic to count objects still falling toward the water. */
s16 fn_1_12700(void) {
    s16 effectIndex;
    s16 activeCount;

    activeCount = 0;
    effectIndex = 0;
    while (effectIndex < 13) {
        if (lbl_1_bss_4B8[effectIndex].state == 1) {
            activeCount += 1;
        }
        effectIndex += 1;
    }
    return activeCount;
}

s16 fn_1_12760(void) {
    return 13;
}

/* Called by player logic to check whether an indexed falling object is active. */
s32 fn_1_12768(s16 effectIndex) {
    if ((effectIndex >= 0) && (effectIndex < 13) && (lbl_1_bss_4B8[effectIndex].state == 1)) {
        return 1;
    }
    return 0;
}

/* Called by player logic to read an object's center plus its collision offset. */
void fn_1_127B0(s16 effectIndex, Point3d *position) {
    *position = lbl_1_bss_4B8[effectIndex].position;
    position->x += lbl_1_bss_4B8[effectIndex].positionOffset.x;
    position->y += lbl_1_bss_4B8[effectIndex].positionOffset.y;
    position->z += lbl_1_bss_4B8[effectIndex].positionOffset.z;
}

HU3D_MODELID lbl_1_bss_7C4;
M625MotionState lbl_1_bss_4B8[13];
