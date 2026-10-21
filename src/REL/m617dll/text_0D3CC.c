/* Result-scene setup and sequence updates for Light Up My Night. */
#include "dolphin/math.h"

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

#include "REL/m617dll/shared.h"

#include "game/hu3d.h"

s32 fn_1_24D8();

s32 fn_1_D6F8(void);

s32 fn_1_4DB0(void *, s32);

s32 fn_1_730();

extern void fn_1_2B48();

/* Entering the result sequence fades the music, blocks participant input, and synchronizes
 * their models and animations. */
s32 fn_1_D3CC(void)
{
    fn_1_53AC(1, 100);
    fn_1_5DB4();
    lbl_1_bss_AB0[0].motionState = 1;
    lbl_1_bss_AB0[1].motionState = 1;
    fn_1_2B48();
    fn_1_24D8();
    return 0;
}

/* Called during result presentation to advance the camera and refresh scores in the opening
 * phases. */
s16 fn_1_D424(void)
{
    s16 result = 0;

    result = fn_1_D6F8();
    if (lbl_1_bss_9C8.sequencePhase < 2) {
        fn_1_5DB4();
    }
    fn_1_24D8();
    return result;
}

/* Sets both players' result motions once, then synchronizes models, animations, and shadows. */
s32 fn_1_D474(void) {
    s16 playerIndex;

    switch (lbl_1_bss_9C8.sequenceSelector) {
    case 1:
        break;
    case 0:
        playerIndex = 0;
        while (playerIndex < 2) {
            fn_1_4DB0(&lbl_1_bss_AB0[playerIndex], 6);
            if (lbl_1_bss_AB0[playerIndex].resultState >= 0) {
                lbl_1_bss_DC0[playerIndex].shiftState = 1;
                lbl_1_bss_DC0[playerIndex].shiftMotion = 1;
                lbl_1_bss_DC0[playerIndex].shiftStart = 0.0f;
                lbl_1_bss_DC0[playerIndex].shiftEnd = 5.0f;
                lbl_1_bss_DC0[playerIndex].shiftFlags = 0;
                lbl_1_bss_DC0[playerIndex].attributesToReset = HU3D_MOTATTR_LOOP;
            } else {
                lbl_1_bss_DC0[playerIndex].shiftState = 1;
                lbl_1_bss_DC0[playerIndex].shiftMotion = 2;
                lbl_1_bss_DC0[playerIndex].shiftStart = 0.0f;
                lbl_1_bss_DC0[playerIndex].shiftEnd = 5.0f;
                lbl_1_bss_DC0[playerIndex].shiftFlags = 0;
                lbl_1_bss_DC0[playerIndex].attributesToReset = HU3D_MOTATTR_LOOP;
            }
            playerIndex += 1;
        }
        lbl_1_bss_9C8.sequenceSelector += 1;
        break;
    }
    fn_1_730();
    fn_1_24D8();
    return 0;
}

void fn_1_1817C(M617PlayerStateData *work);

typedef struct M617E1ConstantBlock {
    Vec directions[8];
} M617E1ConstantBlock;

extern f64 fn_1_18688(f64 value);

extern const M617E1ConstantBlock lbl_1_rodata_CE8;

extern const u16 lbl_1_rodata_D48[4];

extern const u16 lbl_1_rodata_D50[4];

extern const u16 lbl_1_rodata_D58[4];

extern const u16 lbl_1_rodata_D60[4];

extern const u16 lbl_1_rodata_D68[4];

extern const s32 lbl_1_rodata_D70[12];

extern const s16 lbl_1_rodata_DA0[8];

s32 fn_1_24D8(void);

s16 fn_1_5C1C(void);

typedef struct M617D6HeaderConstants {
    s32 word[9];
} M617D6HeaderConstants;

extern const M617D6HeaderConstants lbl_1_rodata_C88;

extern f32 CZoom;

extern void fn_1_307C(MGTIMER *timer);

extern s32 fn_1_3CD0(s16 modelId, s16 lightId);

extern s16 fn_1_3BB8(s16 modelId, s16 lightId);

/* Refreshes participant and scene-model transforms, participant animations, and shadow
 * placement. */
s32 fn_1_D6B0(void)
{
    fn_1_24D8();
    return 0;
}

/* Stops music and sound effects when the result sequence shuts audio down. */
s32 fn_1_D6D4(void)
{
    fn_1_5C1C();
    return 0;
}

/* The sequence calls this each frame to run the wipe, scene setup, and final delay phases. */
s32 fn_1_D6F8(void)
{
    s32 result = 0;
    Vec scenePositions[3] = {
        {-300.0f, 336.0f, -700.0f},
        {300.0f, 336.0f, -700.0f},
        {0.0f, 336.0f, -150.0f}
    };
    s16 modelIndex;
    s16 lightIndex;
    M617LightSource *light;

    switch (lbl_1_bss_9C8.sequencePhase) {
    case 0:
        if (lbl_1_bss_9C8.sequenceFrame++ > 30) {
            WipeCreate(2, 6, 60);
            lbl_1_bss_9C8.sequenceFrame = 0;
            lbl_1_bss_9C8.sequencePhase++;
        }
        break;
    case 1:
        if (WipeCheck() == 0) {
            lbl_1_bss_AB0[0].transformState = 0;
            lbl_1_bss_AB0[1].transformState = 0;
            lbl_1_bss_AB0[0].position.x = -125.0f;
            lbl_1_bss_AB0[0].position.y = 0.0f;
            lbl_1_bss_AB0[0].position.z = -478.0f;
            lbl_1_bss_AB0[0].rotation.x = 0.0f;
            lbl_1_bss_AB0[0].rotation.y = 0.0f;
            lbl_1_bss_AB0[0].rotation.z = 0.0f;
            lbl_1_bss_AB0[1].position.x = 125.0f;
            lbl_1_bss_AB0[1].position.y = 0.0f;
            lbl_1_bss_AB0[1].position.z = -478.0f;
            lbl_1_bss_AB0[1].rotation.x = 0.0f;
            lbl_1_bss_AB0[1].rotation.y = 0.0f;
            lbl_1_bss_AB0[1].rotation.z = 0.0f;
            Hu3DModelHookReset(lbl_1_bss_E88[0].modelId);
            Hu3DModelHookReset(lbl_1_bss_E88[1].modelId);
            Hu3DModelAmbSet(lbl_1_bss_E88[0].modelId, 1.0f, 1.0f, 1.0f);
            Hu3DModelAmbSet(lbl_1_bss_E88[1].modelId, 1.0f, 1.0f, 1.0f);
            lbl_1_bss_DC0[0].previousAttributes = (lbl_1_bss_DC0[0].previousMode = -1);
            /* Reset player zero's attribute cache again while resetting player one's motion
             * cache. */
            lbl_1_bss_DC0[0].previousAttributes = (lbl_1_bss_DC0[1].previousMode = -1);
            lbl_1_bss_DC0[0].mode = 0;
            lbl_1_bss_DC0[0].speed = 0.0f;
            lbl_1_bss_DC0[0].time = 1.0f;
            lbl_1_bss_DC0[0].attributes = HU3D_MOTATTR_LOOP;
            lbl_1_bss_DC0[0].attributesToReset = 0;
            lbl_1_bss_DC0[1].mode = 0;
            lbl_1_bss_DC0[1].speed = 0.0f;
            lbl_1_bss_DC0[1].time = 1.0f;
            lbl_1_bss_DC0[1].attributes = HU3D_MOTATTR_LOOP;
            lbl_1_bss_DC0[1].attributesToReset = 0;
            if (lbl_1_bss_AB0[0].resultState == -1 && lbl_1_bss_AB0[1].resultState == -1) {
                fn_1_307C(lbl_1_bss_9C8.timer);
            }
            Hu3DCameraPerspectiveSet(1, 27.027f, 20.0f, 40000.0f, 1.33f);
            Center.x = -0.368f;
            Center.y = 218.751f;
            Center.z = -264.73f;
            CRot.x = -9.81f;
            CRot.y = 0.0f;
            CRot.z = 0.0f;
            CZoom = 800.0f;
            for (modelIndex = 46; modelIndex <= 91; modelIndex++) {
                Hu3DModelAttrSet(lbl_1_bss_E88[modelIndex].modelId, HU3D_ATTR_DISPOFF);
            }
            for (modelIndex = 92; modelIndex <= 93; modelIndex++) {
                Hu3DModelAttrSet(lbl_1_bss_E88[modelIndex].modelId, HU3D_ATTR_DISPOFF);
            }
            Hu3DModelAttrSet(lbl_1_bss_E88[19].modelId, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_E88[25].modelId, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_E88[26].modelId, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_E88[29].modelId, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_E88[30].modelId, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_E88[31].modelId, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_E88[32].modelId, HU3D_ATTR_DISPOFF);
            for (modelIndex = 37; modelIndex <= 45; modelIndex++) {
                Hu3DModelAttrReset(lbl_1_bss_E88[modelIndex].modelId, HU3D_ATTR_DISPOFF);
                if (modelIndex == 40 || modelIndex == 43 || modelIndex == 45) {
                    Hu3DModelAttrSet(lbl_1_bss_E88[modelIndex].modelId, HU3D_MOTATTR_LOOP);
                    Hu3DMotionSpeedSet(lbl_1_bss_E88[modelIndex].modelId, 1.0f);
                }
            }
            lbl_1_bss_0[20].tpLevel = 0.0f;
            lbl_1_bss_0[21].tpLevel = 0.0f;
            MgScoreBoxTPLvlSet(lbl_1_bss_0[20].spriteId, lbl_1_bss_0[20].tpLevel);
            MgScoreBoxTPLvlSet(lbl_1_bss_0[21].spriteId, lbl_1_bss_0[21].tpLevel);
            lbl_1_bss_370[0].scaleY =
                lbl_1_bss_370[0].scaleX = 0.0f;
            lbl_1_bss_370[1].scaleY =
                lbl_1_bss_370[1].scaleX = 0.0f;
            HuSprGrpScaleSet(lbl_1_bss_370[0].groupId, lbl_1_bss_370[0].scaleX,
                             lbl_1_bss_370[0].scaleY);
            HuSprGrpScaleSet(lbl_1_bss_370[1].groupId, lbl_1_bss_370[1].scaleX,
                             lbl_1_bss_370[1].scaleY);
            for (modelIndex = 0; modelIndex <= 14; modelIndex++) {
                light = &lbl_1_bss_458[modelIndex];
                /* The light-removal sweep also visits slot 96, beyond the 96-entry model array. */
                for (lightIndex = 0; lightIndex <= 96; lightIndex++) {
                    if (light->modelId != lbl_1_bss_E88[lightIndex].modelId) {
                        fn_1_3CD0(lbl_1_bss_E88[lightIndex].modelId,
                                  light->localLightPoolId);
                    }
                }
            }
            light = &lbl_1_bss_458[0];
            light->position.x = 0.0f;
            light->position.y = 700.0f;
            light->position.z = 0.0f;
            light->distance = light->position.y;
            light->intensity = 1.0f;
            light->color.r = 153;
            light->color.g = 153;
            light->color.b = 128;
            light->color.a = 255;
            fn_1_3BB8(lbl_1_bss_E88[0].modelId, light->localLightPoolId);
            fn_1_3BB8(lbl_1_bss_E88[1].modelId, light->localLightPoolId);
            fn_1_3BB8(lbl_1_bss_E88[20].modelId, light->localLightPoolId);
            fn_1_3BB8(lbl_1_bss_E88[21].modelId, light->localLightPoolId);
            fn_1_3BB8(lbl_1_bss_E88[4].modelId, light->localLightPoolId);
            fn_1_3BB8(lbl_1_bss_E88[5].modelId, light->localLightPoolId);
            fn_1_3BB8(lbl_1_bss_E88[8].modelId, light->localLightPoolId);
            fn_1_3BB8(lbl_1_bss_E88[9].modelId, light->localLightPoolId);
            Hu3DLLightPosSetV(light->modelId, light->lightId,
                              (Point3d *)&light->position.x, &light->direction);
            Hu3DLLightPointSet(light->modelId, light->lightId,
                               light->distance, light->intensity, GX_DA_STEEP);
            Hu3DLLightColorSet(light->modelId, light->lightId,
                               light->color.r, light->color.g,
                               light->color.b, light->color.a);
            Hu3DModelAmbSet(lbl_1_bss_E88[0].modelId, 1.0f, 1.0f, 1.0f);
            Hu3DModelAmbSet(lbl_1_bss_E88[1].modelId, 1.0f, 1.0f, 1.0f);
            lbl_1_bss_3710.shadowOpacity = 0.2f;
            lbl_1_bss_9C8.sequencePhase++;
        }
        break;
    case 2:
        if (lbl_1_bss_9C8.sequenceFrame++ > 30) {
            WipeCreate(1, 6, 60);
            lbl_1_bss_9C8.sequenceFrame = 0;
            lbl_1_bss_9C8.sequencePhase++;
        }
        break;
    case 3:
        if (WipeCheck() == 0) {
            lbl_1_bss_9C8.sequencePhase++;
        }
        break;
    case 4:
        if (lbl_1_bss_9C8.sequenceFrame++ > 10) {
            result = 1;
        }
        break;
    }
    return result;
}
