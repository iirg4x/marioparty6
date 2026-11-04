/* Sets up both sides, advances player turns and piece placement, and runs the result animation. */
#include <math.h>
#include "REL/m640/m640.h"
#include "REL/m640dll.h"
#define M640_WIN_CELEBRATION_SFX 1917
#define M640_PIECE_PART_0_LANDING_SFX 1906
#define M640_PIECE_PART_1_LANDING_SFX 1907
#define M640_PIECE_MOTION_CUE_SFX 1918
#define M640_PLAYER_MISS_SFX_0 1928
#define M640_PLAYER_MISS_SFX_1 1929
#define M640_PLAYER_MISS_SFX_2 1930
#define M640_PLAYER_MISS_SFX_3 1931
#define M640_PLAYER_SPIN_SFX_0 1932
#define M640_PLAYER_SPIN_SFX_1 1933
#define M640_PLAYER_SPIN_SFX_2 1934
#define M640_PLAYER_SPIN_SFX_3 1935
#define M640_PLAYER_SELECT_SFX_0 1920
#define M640_PLAYER_SELECT_SFX_1 1921
#define M640_PLAYER_SELECT_SFX_2 1922
#define M640_PLAYER_SELECT_SFX_3 1923
#define M640_PLAYER_HIT_SFX_0 1924
#define M640_PLAYER_HIT_SFX_1 1925
#define M640_PLAYER_HIT_SFX_2 1926
#define M640_PLAYER_HIT_SFX_3 1927

void fn_1_A0(void);
void fn_1_86C(void);
void fn_1_8D4(void);
s32 fn_1_BEC(void);
s16 fn_1_EC8(void);
s16 fn_1_112C(s16 frame);
void fn_1_15C8(void);
s32 fn_1_1C48(s32 frame);
void fn_1_26B8(s16 playerNo, s16 motionSlot);
void fn_1_2820(void);
s16 fn_1_2FF8(void);
void fn_1_3090(void);
s16 fn_1_3274(M640Player *player, HuVecF *spinnerRotation);
s16 fn_1_33D0(float rotationDegrees);
void fn_1_345C(s16 side, s16 slot, s16 previewIndex);
void fn_1_351C(s16 side, s16 slot);
void fn_1_355C(void);
void fn_1_36D8(M640Player *player);
s16 fn_1_37F4(s16 side, M640Player *player);
u16 fn_1_39F0(s16 side, M640Player *player);
void fn_1_3C24(s16 side, s16 slot);
void fn_1_4C90(s16 side);
void fn_1_4CD8(void);
void fn_1_4DDC(s16 side, s16 part);
void fn_1_4FFC(s16 side, s16 part);
void fn_1_519C(s16 side, s16 part);
void fn_1_52D0(OMOBJ *obj);
void fn_1_5A28(void);
void fn_1_5DF0(void);
void fn_1_60AC(OMOBJ *obj);
void fn_1_623C(void);

Point3d lbl_1_data_28 = { 500.0f, 2000.0f, 400.0f };
Point3d lbl_1_data_34 = { -5.0f, -20.0f, -4.0f };
GXColor lbl_1_data_40 = { 224, 224, 224, 224 };
M640MotionEntry lbl_1_data_44[5] = {
    { 0x930000, ((1 << 30) | (1 << 0)) },
    { DATANUM(0x930000, 1), ((1 << 30) | (1 << 0)) },
    { DATANUM(0x930000, 6), 0 },
    { DATANUM(0x930000, 7), 0 },
    { DATANUM(0x930000, 11), 0 },
};
s32 lbl_1_data_6C[2] = { DATANUM(0x630000, 4), DATANUM(0x630000, 5) };
f32 lbl_1_data_74[2] = { -450.0f, 450.0f };
s32 lbl_1_data_7C[2] = { DATANUM(0x630000, 10), DATANUM(0x630000, 11) };
u32 lbl_1_data_84[12][2] = {
    { 0, 0 },
    { 0, 0 },
    { DATANUM(0x630000, 26), DATANUM(0x630000, 27) },
    { DATANUM(0x630000, 31), DATANUM(0x630000, 32) },
    { 0, 0 },
    { 0, 0 },
    { DATANUM(0x630000, 46), DATANUM(0x630000, 47) },
    { DATANUM(0x630000, 51), DATANUM(0x630000, 52) },
    { 0, 0 },
    { 0, 0 },
    { DATANUM(0x630000, 66), DATANUM(0x630000, 67) },
    { DATANUM(0x630000, 71), DATANUM(0x630000, 72) },
};
u32 lbl_1_data_E4[12][4] = {
    { 0, DATANUM(0x630000, 17), DATANUM(0x630000, 18), DATANUM(0x630000, 19) },
    { DATANUM(0x630000, 20), 0, DATANUM(0x630000, 21), DATANUM(0x630000, 22) },
    { DATANUM(0x630000, 23), DATANUM(0x630000, 24), 0, DATANUM(0x630000, 25) },
    { DATANUM(0x630000, 28), DATANUM(0x630000, 29), DATANUM(0x630000, 30), 0 },
    { 0, DATANUM(0x630000, 37), DATANUM(0x630000, 38), DATANUM(0x630000, 39) },
    { DATANUM(0x630000, 40), 0, DATANUM(0x630000, 41), DATANUM(0x630000, 42) },
    { DATANUM(0x630000, 43), DATANUM(0x630000, 44), 0, DATANUM(0x630000, 45) },
    { DATANUM(0x630000, 48), DATANUM(0x630000, 49), DATANUM(0x630000, 50), 0 },
    { 0, DATANUM(0x630000, 57), DATANUM(0x630000, 58), DATANUM(0x630000, 59) },
    { DATANUM(0x630000, 60), 0, DATANUM(0x630000, 61), DATANUM(0x630000, 62) },
    { DATANUM(0x630000, 63), DATANUM(0x630000, 64), 0, DATANUM(0x630000, 65) },
    { DATANUM(0x630000, 68), DATANUM(0x630000, 69), DATANUM(0x630000, 70), 0 },
};
s32 lbl_1_data_1A4[12] = {
    DATANUM(0x630000, 13),
    DATANUM(0x630000, 14),
    DATANUM(0x630000, 15),
    DATANUM(0x630000, 16),
    DATANUM(0x630000, 33),
    DATANUM(0x630000, 34),
    DATANUM(0x630000, 35),
    DATANUM(0x630000, 36),
    DATANUM(0x630000, 53),
    DATANUM(0x630000, 54),
    DATANUM(0x630000, 55),
    DATANUM(0x630000, 56),
};
s32 lbl_1_data_1D4[12] = {
    DATANUM(0x630000, 74),
    DATANUM(0x630000, 75),
    DATANUM(0x630000, 76),
    DATANUM(0x630000, 77),
    DATANUM(0x630000, 78),
    DATANUM(0x630000, 79),
    DATANUM(0x630000, 80),
    DATANUM(0x630000, 81),
    DATANUM(0x630000, 82),
    DATANUM(0x630000, 83),
    DATANUM(0x630000, 84),
    DATANUM(0x630000, 85),
};
M640Team lbl_1_bss_4FC[2];
M640Scene lbl_1_bss_4E8;
M640Player lbl_1_bss_3F8[4];
OMOBJ *lbl_1_bss_3F4;
OMOBJ *lbl_1_bss_3F0;
s16 lbl_1_bss_3E4[5];
s16 lbl_1_bss_3CC[3][4];
M640Piece lbl_1_bss_C[12][2];
void *lbl_1_bss_8;

/* Called at mode entry to clear the scene and team state, then create cameras, lights, players,
 * piece models, and backgrounds. */
void fn_1_86C(void)
{
    memset(&lbl_1_bss_4E8, 0, sizeof(lbl_1_bss_4E8));
    memset(lbl_1_bss_4FC, 0, sizeof(lbl_1_bss_4FC));
    lbl_1_bss_4E8.finishCount = 0;
    fn_1_8D4();
    fn_1_15C8();
    fn_1_2820();
    fn_1_5A28();
    fn_1_5DF0();
}

/* Creates the two stacked camera views and initializes the camera transforms used during the
 * opening split transition. */
void fn_1_8D4(void)
{
    int cameraIndex;
    OMOBJ *viewportObject;
    Hu3DCameraCreate(1);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(1, 20.0f, 60.0f, 25000.0f, 1.2f);
    Hu3DCameraScissorSet(1, 0, 0, 640, 480);
    Hu3DCameraCreate(2);
    Hu3DCameraViewportSet(2, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(2, 20.0f, 60.0f, 25000.0f, 1.2f);
    Hu3DCameraScissorSet(2, 0, 480, 640, 480);
    viewportObject = fn_8003AB38(lbl_1_bss_4, 32730, 0, 0, -1, fn_8003DAA4);
    viewportObject->work[0] = 2;
    for (cameraIndex = 0; cameraIndex < 2; cameraIndex++) {
        lbl_80266B18[cameraIndex].x = cameraIndex * 1300;
        lbl_80266B18[cameraIndex].y = 150.0f - cameraIndex * 75;
        lbl_80266B18[cameraIndex].z = 0.0f;
        lbl_80266A58[cameraIndex].x = -5.0f;
        lbl_80266A58[cameraIndex].y = 0.0f;
        lbl_80266A58[cameraIndex].z = 0.0f;
        lbl_80266BD8[cameraIndex] = 1400.0f + cameraIndex * 400;
    }
}

/* Once side zero's intro model reaches frame 90, reverses both sides' intro and piece-hook motions
 * from that frame, hides their hook effects and piece models, and reports intro completion. */
s32 fn_1_BEC(void)
{
    char *hooks[] = { "itemhook_fx1", "itemhook_fx2", "itemhook_fx3", NULL };
    int sideIndex, pieceRecordIndex;
    float introMotionTime;
    fn_8002D86C(lbl_1_bss_4FC[0].introAnimationModel, 1.5f);
    fn_8002D86C(lbl_1_bss_4FC[0].pieceHookModel, 1.5f);
    fn_8002D86C(lbl_1_bss_4FC[1].introAnimationModel, 1.5f);
    fn_8002D86C(lbl_1_bss_4FC[1].pieceHookModel, 1.5f);
    introMotionTime = fn_8002D4E0(lbl_1_bss_4FC[0].introAnimationModel);
    if (introMotionTime >= 90.0f) {
        for (sideIndex = 0; sideIndex < 2; sideIndex++) {
            fn_80027508(lbl_1_bss_4FC[sideIndex].pieceHookModel);
            fn_800269E8(lbl_1_bss_4FC[sideIndex].introAnimationModel, ((1 << 30) | (1 << 2)));
            fn_800269E8(lbl_1_bss_4FC[sideIndex].pieceHookModel, ((1 << 30) | (1 << 2)));
            fn_8002D414(lbl_1_bss_4FC[sideIndex].introAnimationModel, 90.0f);
            fn_8002D414(lbl_1_bss_4FC[sideIndex].pieceHookModel, 90.0f);
            lbl_1_bss_4FC[sideIndex].activePieceIndex = -1;
            for (pieceRecordIndex = 0; pieceRecordIndex < 4; pieceRecordIndex++) {
                fn_800269E8(lbl_1_bss_4FC[sideIndex].hookEffectModels[pieceRecordIndex],
                                 (1 << 0));
            }
        }
        for (sideIndex = 0; sideIndex < 2; sideIndex++) {
            for (pieceRecordIndex = 0; pieceRecordIndex < 12; pieceRecordIndex++) {
                fn_80026760(lbl_1_bss_C[pieceRecordIndex][sideIndex].pieceModel,
                                sideIndex * 1300, 400.0f, 200.0f);
                fn_800269E8(lbl_1_bss_C[pieceRecordIndex][sideIndex].pieceModel,
                                 (1 << 0));
                lbl_1_bss_C[pieceRecordIndex][sideIndex].state = 0;
            }
        }
        return 1;
    }
    return 0;
}

/* After the side-model transition, attaches four effect models to each side's hooks and reports
 * when they are ready. */
s16 fn_1_EC8(void)
{
    char *hooks[] = { "itemhook_fx0", "itemhook_fx1", "itemhook_fx2", "itemhook_fx3" };
    int effectIndex, sideIndex;
    float introMotionTime;
    fn_8002D86C(lbl_1_bss_4FC[0].introAnimationModel, 2.0f);
    fn_8002D86C(lbl_1_bss_4FC[0].pieceHookModel, 2.0f);
    fn_8002D86C(lbl_1_bss_4FC[1].introAnimationModel, 2.0f);
    fn_8002D86C(lbl_1_bss_4FC[1].pieceHookModel, 2.0f);
    introMotionTime = fn_8002D4E0(lbl_1_bss_4FC[0].introAnimationModel);
    if (introMotionTime <= 1.0f) {
        for (sideIndex = 0; sideIndex < 2; sideIndex++) {
            for (effectIndex = 0; effectIndex < 4; effectIndex++) {
                if (effectIndex == 0) {
                    lbl_1_bss_4FC[sideIndex].bucketEffectModels[effectIndex] = fn_80025A04(
                        fn_80008DD0(DATANUM(0x630000, 12), 0x10000000, 2));
                } else {
                    lbl_1_bss_4FC[sideIndex].bucketEffectModels[effectIndex] = fn_80025A04(
                        fn_80008DD0(DATANUM(0x630000, 9), 0x10000000, 2));
                }
                fn_8002D86C(lbl_1_bss_4FC[sideIndex].bucketEffectModels[effectIndex], 0.0f);
                fn_8002D414(lbl_1_bss_4FC[sideIndex].bucketEffectModels[effectIndex], 0.0f);
                fn_8002741C(lbl_1_bss_4FC[sideIndex].pieceHookModel, hooks[effectIndex],
                                 lbl_1_bss_4FC[sideIndex].bucketEffectModels[effectIndex]);
                fn_800269E8(lbl_1_bss_4FC[sideIndex].bucketEffectModels[effectIndex],
                                 (1 << 0));
            }
        }
        return 1;
    }
    return 0;
}

/* On each intro update, moves the two camera viewports toward their split positions and restores
 * normal model speed at frame 60. */
s16 fn_1_112C(s16 frame)
{
    int cameraIndex;
    float upperViewportHeight, viewportOverlap, cameraAspect;
    if ((float)frame == 60.0f) {
        fn_8002D86C(lbl_1_bss_4FC[0].introAnimationModel, 1.0f);
        fn_8002D86C(lbl_1_bss_4FC[1].introAnimationModel, 1.0f);
        fn_8002D86C(lbl_1_bss_4FC[0].pieceHookModel, 1.0f);
        fn_8002D86C(lbl_1_bss_4FC[1].pieceHookModel, 1.0f);
    }
    lbl_1_bss_4E8.cameraProgress += 4.0f;
    {
        u16 cameras[] = { 1, 2 };
        upperViewportHeight = 480.0f - 4.0f * frame;
        viewportOverlap = 40.0f * frame / 60.0f;
        for (cameraIndex = 0; cameraIndex < 2; cameraIndex++) {
            lbl_80266B18[cameraIndex].y -= 0.33333334f;
            lbl_80266B18[cameraIndex].z += 1.6666666f;
            lbl_80266A58[cameraIndex].x -= 0.016666668f;
            lbl_80266BD8[cameraIndex] += 3.3333333f;
        }
        for (cameraIndex = 0; cameraIndex < 2; cameraIndex++) {
            /* Set each camera's vertical origin and upperViewportHeight for the moving split. */
            float viewportBounds[2];
            switch (cameraIndex) {
            case 0:
                viewportBounds[0] = 0.0f;
                viewportBounds[1] = upperViewportHeight + viewportOverlap;
                Hu3DCameraScissorSet(cameras[cameraIndex], 0, 0, 640, upperViewportHeight - 2.0f);
                cameraAspect = 576.0 / (1.0f + upperViewportHeight);
                Hu3DCameraPerspectiveSet(cameras[cameraIndex], 20.0f, 60.0f, 25000.0f,
                                         cameraAspect);
                break;
            case 1:
                viewportBounds[0] = upperViewportHeight - viewportOverlap;
                viewportBounds[1] = (480.0f - upperViewportHeight) + viewportOverlap;
                Hu3DCameraScissorSet(cameras[cameraIndex], 0, 2.0f + upperViewportHeight, 640,
                                     (480.0f - upperViewportHeight) - 2.0f);
                cameraAspect = 2.4f;
                Hu3DCameraPerspectiveSet(cameras[cameraIndex], 20.0f, 60.0f, 25000.0f,
                                         cameraAspect);
                break;
            }
            Hu3DCameraViewportSet(cameras[cameraIndex], 0.0f, viewportBounds[0], 640.0f,
                                  viewportBounds[1], 0.0f, 1.0f);
        }
    }
    if (lbl_1_bss_4E8.cameraProgress > 240.0f) {
        return 1;
    }
    return 0;
}

/* Creates the scene light and shadow, then builds each side's board and attached scene models. */
void fn_1_15C8(void)
{
    HuVecF shadowPosition, shadowTarget, shadowUp;
    s16 modelHandle;
    int sideIndex, hookIndex;
    s16 lightId;
    u16 cameras[] = { 1, 2 };
    char *hooks[] = { "itemhook_fx0", "itemhook_fx1", "itemhook_fx2", "itemhook_fx3" };
    lightId = fn_800286D0(&lbl_1_data_28, &lbl_1_data_34, &lbl_1_data_40);
    Hu3DGLightStaticSet(lightId, 1);
    Hu3DGLightInfinitytSet(lightId);
    Hu3DShadowCreate(30.0f, 20.0f, 13000.0f);
    shadowPosition.x = 500.0f;
    shadowPosition.y = 8000.0f;
    shadowPosition.z = 2000.0f;
    shadowUp.y = 1.0f;
    shadowUp.x = shadowUp.z = 0.0f;
    shadowTarget.x = shadowTarget.y = shadowTarget.z = 0.0f;
    Hu3DShadowPosSet(&shadowPosition, &shadowUp, &shadowTarget);
    for (sideIndex = 0; sideIndex < 2; sideIndex++) {
        modelHandle =
            fn_80025A04(fn_80008DD0(DATANUM(0x630000, 1), 0x10000000, 2));
        fn_80026B64(modelHandle, cameras[sideIndex]);
        fn_80026FFC(modelHandle);
        fn_80026760(modelHandle, sideIndex * 1300, 0.0f, 0.0f);
        lbl_1_bss_4FC[sideIndex].boardHookAnchorModel = modelHandle;
        modelHandle =
            fn_80025A04(fn_80008DD0(DATANUM(0x630000, 2), 0x10000000, 2));
        fn_80026B64(modelHandle, cameras[sideIndex]);
        fn_80026760(modelHandle, sideIndex * 1300, 0.0f, 0.0f);
        lbl_1_bss_4FC[sideIndex].sidePlatformModel = modelHandle;
        modelHandle = fn_80025A04(
            fn_80008DD0(lbl_1_data_6C[sideIndex], 0x10000000, 2));
        fn_80026760(modelHandle, sideIndex * 1300, 0.0f, 0.0f);
        fn_80026B64(modelHandle, cameras[sideIndex]);
        lbl_1_bss_4FC[sideIndex].sideBackdropModel = modelHandle;
        modelHandle =
            fn_80025A04(fn_80008DD0(DATANUM(0x630000, 6), 0x10000000, 2));
        fn_80026760(modelHandle, sideIndex * 1300, 0.0f, 0.0f);
        fn_80026B64(modelHandle, cameras[sideIndex]);
        lbl_1_bss_4FC[sideIndex].introAnimationModel = modelHandle;
        fn_80026FFC(modelHandle);
        fn_8002D86C(modelHandle, 0.0f);
        modelHandle =
            fn_80025A04(fn_80008DD0(DATANUM(0x630000, 7), 0x10000000, 2));
        fn_80026760(modelHandle, sideIndex * 1300, 0.0f, 0.0f);
        fn_80026B64(modelHandle, cameras[sideIndex]);
        lbl_1_bss_4FC[sideIndex].pieceHookModel = modelHandle;
        fn_8002D86C(modelHandle, 0.0f);
        fn_80026E44(modelHandle);
        modelHandle =
            fn_80025A04(fn_80008DD0(DATANUM(0x630000, 8), 0x10000000, 2));
        fn_80026760(modelHandle, sideIndex * 1300, 0.0f, 0.0f);
        fn_80026B64(modelHandle, cameras[sideIndex]);
        fn_800269E8(modelHandle, ((1 << 30) | (1 << 0)));
        lbl_1_bss_4FC[sideIndex].sideAccentModel = modelHandle;
        for (hookIndex = 0; hookIndex < 4; hookIndex++) {
            if (hookIndex == 0) {
                modelHandle = fn_80025A04(
                    fn_80008DD0(DATANUM(0x630000, 12), 0x10000000, 2));
            } else {
                modelHandle = fn_80025A04(
                    fn_80008DD0(DATANUM(0x630000, 9), 0x10000000, 2));
            }
            fn_8002741C(lbl_1_bss_4FC[sideIndex].pieceHookModel, hooks[hookIndex],
                             modelHandle);
            fn_80026B64(modelHandle, cameras[sideIndex]);
            fn_800269E8(modelHandle, (1 << 0));
            fn_8002D86C(modelHandle, 0.0f);
            lbl_1_bss_4FC[sideIndex].hookEffectModels[hookIndex] = modelHandle;
        }
        /* Both sides use the second entry in the missed-piece model table. */
        modelHandle =
            fn_80025A04(fn_80008DD0(lbl_1_data_7C[1], 0x10000000, 2));
        fn_80026B64(modelHandle, cameras[sideIndex]);
        fn_80026760(modelHandle, sideIndex * 1300, 0.0f, 0.0f);
        fn_8002D86C(modelHandle, 0.0f);
        lbl_1_bss_4FC[sideIndex].missedPieceEffectModel = modelHandle;
    }
}

/* Advances the winning side's celebration by frame, including player motion, camera framing, and
 * piece animations. */
s32 fn_1_1C48(s32 frame)
{
    OM_CAMERA_VIEW cameraView;
    HuVecF characterRotation, characterPosition;
    int celebrationEntryIndex;
    M640Team *team = &lbl_1_bss_4FC[lbl_1_bss_4E8.winnerSide];
    M640Piece *piece;
    int cameraIndex;
    float winnerViewportHeight, viewportOverlap, remainingSplitHeight, cameraAspect;
    u16 cameras[] = { 1, 2 };

    if ((s16)frame == 0) {
        for (celebrationEntryIndex = 0; celebrationEntryIndex < 2; celebrationEntryIndex++) {
            fn_1_26B8(team->players[celebrationEntryIndex]->playerNo, 1);
        }
        cameraView.center.x = lbl_1_bss_4E8.winnerSide * 1300;
        cameraView.center.y = 100.0f;
        cameraView.center.z = -320.0f;
        cameraView.rot.x = -5.0f;
        cameraView.rot.y = cameraView.rot.z = 0.0f;
        cameraView.zoom = 2500.0f;
        fn_8003E8E4(cameras[lbl_1_bss_4E8.winnerSide], &cameraView, 80);
    }
    if ((s16)frame < 25) {
        for (celebrationEntryIndex = 0; celebrationEntryIndex < 2; celebrationEntryIndex++) {
            fn_80026874(team->players[celebrationEntryIndex]->characterModel,
                            &characterRotation);
            characterRotation.y += 3.0f - 6.0f * celebrationEntryIndex;
            fn_80026838(team->players[celebrationEntryIndex]->characterModel,
                             &characterRotation);
        }
    } else if ((s16)frame < 66) {
        for (celebrationEntryIndex = 0; celebrationEntryIndex < 2; celebrationEntryIndex++) {
            fn_800267CC(team->players[celebrationEntryIndex]->characterModel,
                            &characterPosition);
            characterPosition.x += 5.0f - 10.0f * celebrationEntryIndex;
            characterPosition.z += 2.0f;
            if ((s16)frame > 48 && characterPosition.y > 0.0f) {
                characterPosition.y -= 4.0f;
            }
            fn_80026790(team->players[celebrationEntryIndex]->characterModel,
                             &characterPosition);
        }
    } else if ((s16)frame < 96) {
        for (celebrationEntryIndex = 0; celebrationEntryIndex < 2; celebrationEntryIndex++) {
            fn_80026874(team->players[celebrationEntryIndex]->characterModel,
                            &characterRotation);
            characterRotation.y -= 3.0f - 6.0f * celebrationEntryIndex;
            fn_80026838(team->players[celebrationEntryIndex]->characterModel,
                             &characterRotation);
        }
    } else if ((s16)frame == 96) {
        for (celebrationEntryIndex = 0; celebrationEntryIndex < 2; celebrationEntryIndex++) {
            fn_1_26B8(team->players[celebrationEntryIndex]->playerNo, 0);
        }
    }
    if ((s16)frame < 60) {
        u16 cameraIds[] = { 1, 2 };
        remainingSplitHeight = 4.0f * (60 - (s16)frame);
        if (lbl_1_bss_4E8.winnerSide != 0) {
            winnerViewportHeight = remainingSplitHeight;
        } else {
            winnerViewportHeight = 480.0f - remainingSplitHeight;
        }
        viewportOverlap = 40.0f * (60 - (s16)frame) / 60.0f;
        for (cameraIndex = 0; cameraIndex < 2; cameraIndex++) {
            float viewportBounds[2];
            switch (cameraIndex) {
            case 0:
                viewportBounds[0] = 0.0f;
                viewportBounds[1] = winnerViewportHeight + viewportOverlap;
                Hu3DCameraScissorSet(cameraIds[cameraIndex], 0, 0, 640,
                                     winnerViewportHeight - 2.0f);
                cameraAspect = 576.0 / (1.0f + winnerViewportHeight);
                break;
            case 1:
                viewportBounds[0] = winnerViewportHeight - viewportOverlap;
                viewportBounds[1] = (480.0f - winnerViewportHeight) + viewportOverlap;
                Hu3DCameraScissorSet(cameraIds[cameraIndex], 0, 2.0f + winnerViewportHeight, 640,
                                     (480.0f - winnerViewportHeight) - 2.0f);
                cameraAspect = 576.0 / (1.0f + (480.0f - winnerViewportHeight));
                break;
            }
            if (cameraIndex == lbl_1_bss_4E8.winnerSide) {
                Hu3DCameraPerspectiveSet(cameraIds[cameraIndex], 20.0f, 60.0f, 25000.0f,
                                         cameraAspect);
            }
            Hu3DCameraViewportSet(cameraIds[cameraIndex], 0.0f, viewportBounds[0], 640.0f,
                                  viewportBounds[1], 0.0f, 1.0f);
        }
    }
    if (lbl_1_bss_4E8.uniformPartSet != 0) {
        switch (team->bucketPieces[0]) {
        case 0:
            if ((s16)frame == 67) {
                for (celebrationEntryIndex = 0; celebrationEntryIndex < 4;
                     celebrationEntryIndex++) {
                    piece = &lbl_1_bss_C[team->bucketPieces[celebrationEntryIndex]]
                                        [lbl_1_bss_4E8.winnerSide];
                    if (piece->resultMotions[1] != 0) {
                        fn_8002D730(piece->pieceModel, 27.0f, 240.0f);
                    }
                }
            }
            break;
        case 4:
            if ((s16)frame == 100) {
                for (celebrationEntryIndex = 0; celebrationEntryIndex < 4;
                     celebrationEntryIndex++) {
                    piece = &lbl_1_bss_C[team->bucketPieces[celebrationEntryIndex]]
                                        [lbl_1_bss_4E8.winnerSide];
                    if (piece->resultMotions[1] != 0) {
                        fn_8002D730(piece->pieceModel, 60.0f, 240.0f);
                    }
                }
            }
            break;
        case 8:
            if ((s16)frame == 90) {
                for (celebrationEntryIndex = 0; celebrationEntryIndex < 4;
                     celebrationEntryIndex++) {
                    piece = &lbl_1_bss_C[team->bucketPieces[celebrationEntryIndex]]
                                        [lbl_1_bss_4E8.winnerSide];
                    if (piece->resultMotions[1] != 0) {
                        fn_8002D730(piece->pieceModel, 50.0f, 240.0f);
                    }
                }
            }
            break;
        }
    }
    if ((s16)frame == 40) {
        HuAudFXPlay(M640_WIN_CELEBRATION_SFX);
        if ((s16)(team->bucketPieces[0] / 4) == (s16)(team->bucketPieces[1] / 4)
            && (s16)(team->bucketPieces[0] / 4) == (s16)(team->bucketPieces[2] / 4)
            && (s16)(team->bucketPieces[0] / 4) == (s16)(team->bucketPieces[3] / 4)) {
            lbl_1_bss_4E8.uniformPartSet = 1;
        }
        if (lbl_1_bss_4E8.uniformPartSet != 0) {
            for (celebrationEntryIndex = 0; celebrationEntryIndex < 4; celebrationEntryIndex++) {
                piece = &lbl_1_bss_C[team->bucketPieces[celebrationEntryIndex]]
                                    [lbl_1_bss_4E8.winnerSide];
                if (piece->resultMotions[1] != 0) {
                    fn_8002C95C(piece->pieceModel, piece->resultMotions[1]);
                    fn_8002D86C(piece->pieceModel, 1.0f);
                    fn_800269E8(piece->pieceModel, ((1 << 30) | (1 << 0)));
                }
            }
        } else {
            for (celebrationEntryIndex = 0; celebrationEntryIndex < 4; celebrationEntryIndex++) {
                piece = &lbl_1_bss_C[team->bucketPieces[celebrationEntryIndex]]
                                    [lbl_1_bss_4E8.winnerSide];
                if (piece->resultMotions[0] != 0) {
                    fn_8002C95C(piece->pieceModel, piece->resultMotions[0]);
                    fn_8002D86C(piece->pieceModel, 1.0f);
                    fn_800269E8(piece->pieceModel, ((1 << 30) | (1 << 0)));
                }
            }
        }
    }
    if ((s16)frame == 150) {
        for (celebrationEntryIndex = 0; celebrationEntryIndex < 2; celebrationEntryIndex++) {
            fn_1_26B8(team->players[celebrationEntryIndex]->playerNo, 2);
        }
        return 1;
    }
    return 0;
}

/* Applies one player's character motion; motion slot 4 runs at 1.5x speed, while other slots use an
 * 8-frame shift and their configured playback attributes. */
void fn_1_26B8(s16 playerNo, s16 motionSlot)
{
    if (motionSlot == 4) {
        fn_8005C390(lbl_1_bss_3F8[playerNo].charNo, 1.5f);
        fn_8005BE80(lbl_1_bss_3F8[playerNo].charNo,
                      lbl_1_bss_3F8[playerNo].characterMotions[motionSlot]);
        fn_80026A44(lbl_1_bss_3F8[playerNo].characterModel, ((1 << 30) | (1 << 0)));
        return;
    }
    fn_8005C390(lbl_1_bss_3F8[playerNo].charNo, 1.0f);
    fn_8005C2C8(lbl_1_bss_3F8[playerNo].charNo,
                       lbl_1_bss_3F8[playerNo].characterMotions[motionSlot], 0.0f, 8.0f,
                       lbl_1_data_44[motionSlot].motionAttributes);
}

/* Called by fn_1_86C during scene setup; assigns players to sides and slots, falls back to two
 * players per side for invalid team counts, creates their character and effect models, and sets the
 * character light. */
void fn_1_2820(void)
{
    HuVecF lightPos, lightDir;
    int playerIndex;
    s16 modelHandle;
    int slotIndex;
    s16 sideIndex, characterNo, hasInvalidTeamSetup;
    s16 motionHandle;
    float spinnerStartAngle;
    u16 cameras[] = { 1, 2 };
    char *slotHooks[] = { "R_slot_hook", "L_slot_hook" };
    char *smokeHooks[] = { "R_lump_hook", "L_lump_hook" };
    s16 sides[] = { 0, 0, 1, 1 };
    s16 playerCountBySide[2];
    GXColor color;
    hasInvalidTeamSetup = playerCountBySide[0] = playerCountBySide[1] = 0;
    memset(lbl_1_bss_3F8, 0, sizeof(lbl_1_bss_3F8));
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_80265728[playerIndex].grpNo >= 2) {
            hasInvalidTeamSetup = 1;
            break;
        }
        playerCountBySide[lbl_80265728[playerIndex].grpNo]++;
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        lbl_1_bss_3F8[playerIndex].playerNo = playerIndex;
        characterNo = lbl_80265728[playerIndex].charNo;
        lbl_1_bss_3F8[playerIndex].charNo = characterNo;
        lbl_1_bss_3F8[playerIndex].padNo = lbl_80265728[playerIndex].padNo;
        lbl_1_bss_3F8[playerIndex].turnLoopSoundHandle =
            lbl_1_bss_3F8[playerIndex].selectionSoundHandle = -1;
        if (hasInvalidTeamSetup || playerCountBySide[0] > 2 || playerCountBySide[1] > 2) {
            sideIndex = sides[playerIndex];
            lbl_1_bss_3F8[playerIndex].side = sideIndex;
        } else {
            sideIndex = lbl_80265728[playerIndex].grpNo;
            lbl_1_bss_3F8[playerIndex].side = sideIndex;
        }
        if (lbl_80265728[playerIndex].type == 1) {
            lbl_1_bss_3F8[playerIndex].comDif = lbl_80265728[playerIndex].comDif;
        } else {
            lbl_1_bss_3F8[playerIndex].comDif = -1;
        }
        modelHandle = fn_80057068(characterNo, 4);
        lbl_1_bss_3F8[playerIndex].characterModel = modelHandle;
        fn_80026E44(modelHandle);
        fn_80026B64(modelHandle, cameras[sideIndex]);
        for (slotIndex = 0; slotIndex < 5; slotIndex++) {
            motionHandle = fn_8005B0B8(characterNo, lbl_1_data_44[slotIndex].motionFile);
            lbl_1_bss_3F8[playerIndex].characterMotions[slotIndex] = motionHandle;
        }
        fn_8005C2C8(characterNo, lbl_1_bss_3F8[playerIndex].characterMotions[0], 0.0f, 0.0f,
                           lbl_1_data_44[0].motionAttributes);
        for (slotIndex = 0; slotIndex < 2; slotIndex++) {
            if (lbl_1_bss_4FC[sideIndex].players[slotIndex] == NULL) {
                lbl_1_bss_4FC[sideIndex].players[slotIndex] = &lbl_1_bss_3F8[playerIndex];
                break;
            }
        }
        lbl_1_bss_3F8[playerIndex].slot = slotIndex;
        fn_80026760(modelHandle, lbl_1_data_74[slotIndex] + sideIndex * 1300, 20.0f, 200.0f);
        fn_80026808(modelHandle, 0.0f, 10.0f - 20.0f * slotIndex, 0.0f);
        modelHandle =
            fn_80025A04(fn_80008DD0(DATANUM(0x630000, 0), 0x10000000, 2));
        fn_80026B64(modelHandle, cameras[sideIndex]);
        fn_8002741C(lbl_1_bss_4FC[sideIndex].boardHookAnchorModel, slotHooks[slotIndex],
                         modelHandle);
        fn_8002D86C(modelHandle, 0.0f);
        fn_8002D414(modelHandle, 45.0f);
        spinnerStartAngle = 30.0f * frandmod(4);
        fn_80026808(modelHandle, 0.0f, spinnerStartAngle, 0.0f);
        lbl_1_bss_3F8[playerIndex].spinnerModel = modelHandle;
        modelHandle = fn_80025A04(
            fn_80008DD0(DATANUM(0x630000, 73), 0x10000000, 2));
        fn_80026B64(modelHandle, cameras[sideIndex]);
        fn_8002741C(lbl_1_bss_4FC[sideIndex].boardHookAnchorModel, smokeHooks[slotIndex],
                         modelHandle);
        fn_800269E8(modelHandle, ((1 << 30) | (1 << 0)));
        fn_8002D86C(modelHandle, 0.0f);
        lbl_1_bss_3F8[playerIndex].smokeEffectModel = modelHandle;
        modelHandle =
            fn_80025A04(fn_80008DD0(DATANUM(0x630000, 3), 0x10000000, 2));
        fn_80026760(modelHandle, sideIndex * 1300, 0.0f, 0.0f);
        fn_80026B64(modelHandle, cameras[sideIndex]);
        fn_8002D86C(modelHandle, 0.0f);
        lbl_1_bss_3F8[playerIndex].turnActionModel = modelHandle;
        if (slotIndex != 0) {
            fn_800268B0(modelHandle, -1.0f, 1.0f, 1.0f);
            fn_800269E8(modelHandle, (1 << 23));
        }
    }
    lightPos.x = 0.0f;
    lightPos.y = 400.0f;
    lightPos.z = 1000.0f;
    lightDir.x = 0.0f;
    lightDir.y = -0.1f;
    lightDir.z = -1.0f;
    color.r = color.g = color.b = color.a = 192;
    fn_800614AC(&lightPos, &lightDir, &color);
    fn_80061814();
}

/* Called by fn_1_4CD8 after the turn updates; records sides whose fourth bucket check is nonzero
 * and remembers the last detected side. */
s16 fn_1_2FF8(void)
{
    int sideIndex;
    if (lbl_1_bss_4E8.finishCount) {
        return lbl_1_bss_4E8.finishCount;
    }
    for (sideIndex = 0; sideIndex < 2; sideIndex++) {
        /* This treats the stored piece-record index as a boolean, so index zero is not recognized
         * here. */
        if (lbl_1_bss_4FC[sideIndex].bucketPieces[3]) {
            lbl_1_bss_4E8.finishCount++;
            lbl_1_bss_4E8.winnerSide = sideIndex;
        }
    }
    return lbl_1_bss_4E8.finishCount;
}

/* On result entry, stops or rewinds each player's action and attached-effect animations to their
 * result poses. */
void fn_1_3090(void)
{
    M640Player *player;
    int playerIndex;
    float motionTime, motionEndTime;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        player = &lbl_1_bss_3F8[playerIndex];
        motionTime = fn_8002D4E0(player->turnActionModel);
        if (motionTime > 60.0f) {
            fn_800269E8(player->turnActionModel, ((1 << 30) | (1 << 2)));
            fn_80026A44(player->turnActionModel, ((1 << 30) | (1 << 0)));
            fn_8002D414(player->turnActionModel, 60.0f);
        } else if (motionTime) {
            fn_800269E8(player->turnActionModel, ((1 << 30) | (1 << 2)));
            fn_80026A44(player->turnActionModel, ((1 << 30) | (1 << 0)));
            fn_8002D414(player->turnActionModel, motionTime);
        }
        motionTime = fn_8002D4E0(player->smokeEffectModel);
        motionEndTime = fn_8002D530(player->smokeEffectModel);
        if (motionTime >= motionEndTime) {
        } else if (motionTime > 60.0f) {
            fn_8002D730(player->smokeEffectModel, 81.0f, 140.0f);
            fn_80026A44(player->smokeEffectModel, ((1 << 30) | (1 << 0)));
            fn_8002D414(player->smokeEffectModel, 81.0f);
        } else if (motionTime) {
            fn_800269E8(player->smokeEffectModel, ((1 << 30) | (1 << 2)));
            fn_80026A44(player->smokeEffectModel, ((1 << 30) | (1 << 0)));
            fn_8002D414(player->smokeEffectModel, 0.0f);
        }
        fn_8002D414(player->spinnerModel, 45.0f);
    }
}

/* Called by fn_1_3C24 during turn states 5 and 6; moves the slot marker toward its selected angle
 * and reports when it aligns. */
s16 fn_1_3274(M640Player *player, HuVecF *spinnerRotation)
{
    float targetAngle = player->targetAngle;
    float angleDifference;
    if (spinnerRotation->y == targetAngle) {
        /* This equality path returns no value even though the caller tests the result. */
        return;
    }
    angleDifference = fabs(spinnerRotation->y - targetAngle);
    if (angleDifference > 1.8f) {
        if (spinnerRotation->y < player->targetAngle) {
            spinnerRotation->y += 1.8f;
        } else {
            spinnerRotation->y -= 1.8f;
        }
    } else {
        spinnerRotation->y = player->targetAngle;
        return 1;
    }
    return 0;
}

/* Wraps a rotation angle into the 120-degree wheel range and converts it to one of four 30-degree
 * buckets. */
s16 fn_1_33D0(float rotationDegrees)
{
    rotationDegrees += 15.0f;
    while (rotationDegrees < 0.0f) {
        rotationDegrees += 120.0f;
    }
    while (rotationDegrees >= 120.0f) {
        rotationDegrees -= 120.0f;
    }
    return rotationDegrees / 30.0f;
}

/* Sets the selected player's slot-marker motion time for the requested bucket preview. */
void fn_1_345C(s16 side, s16 slot, s16 previewIndex)
{
    M640Player *player;
    player = lbl_1_bss_4FC[side].players[slot];
    {
        s16 motionTimes[] = { 5, 15, 25, 35, 45 };
        fn_8002D414(player->spinnerModel, motionTimes[previewIndex]);
    }
}

/* Marks a side's selected player as ready to take a turn. */
void fn_1_351C(s16 side, s16 slot)
{
    M640Player *player;
    player = lbl_1_bss_4FC[side].players[slot];
    player->state = 1;
}

/* On result-mode updates, closes out pending turn sounds and applies the remaining slot-marker
 * rotation. */
void fn_1_355C(void)
{
    HuVecF spinnerRotation;
    M640Player *player;
    int playerIndex;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        player = &lbl_1_bss_3F8[playerIndex];
        fn_80026874(player->spinnerModel, &spinnerRotation);
        if (spinnerRotation.y < 0.0f) {
            spinnerRotation.y += 360.0f;
        }
        if (player->state >= 1 && player->state <= 4) {
            if (player->speed > 0.1f) {
                player->speed -= 0.1f;
            } else {
                player->speed = 0.0f;
            }
        }
        if (player->turnLoopSoundHandle != -1) {
            HuAudFXStop(player->turnLoopSoundHandle);
            player->turnLoopSoundHandle = -1;
        }
        if (player->selectionSoundHandle != -1) {
            s16 missSoundByPlayer[] = { M640_PLAYER_MISS_SFX_0, M640_PLAYER_MISS_SFX_1,
                                        M640_PLAYER_MISS_SFX_2, M640_PLAYER_MISS_SFX_3 };
            HuAudFXPlay(missSoundByPlayer[player->slot + player->side * 2]);
            player->selectionSoundHandle = -1;
        }
        fn_80026808(player->spinnerModel, spinnerRotation.x, spinnerRotation.y - player->speed,
                        spinnerRotation.z);
    }
}

/* For a CPU player, chooses the next think delay and whether the next bucket selection will
 * miss. */
void fn_1_36D8(M640Player *player)
{
    s16 baseDelayByDifficulty[] = { 140, 100, 60, 10 };
    s16 randomDelayRangeByDifficulty[] = { 120, 40, 20, 40 };
    s16 missChanceByDifficulty[] = { 80, 60, 20, 5 };
    s16 missRoll;
    if (player->comDif != -1) {
        player->delay = baseDelayByDifficulty[player->comDif] +
                        frandmod(randomDelayRangeByDifficulty[player->comDif]);
        missRoll = frandmod(100);
        if (missRoll < missChanceByDifficulty[player->comDif]) {
            player->error = 1;
        } else {
            player->error = 0;
        }
    }
}

/* After the CPU delay, returns whether its bucket choice should miss the side's next target. The
 * delay table is initialized here but is not read. */
s16 fn_1_37F4(s16 side, M640Player *player)
{
    HuVecF spinnerRotation;
    s16 currentBucket;
    float rotationDegrees;
    s16 bucketByWheelPosition[] = { 0, 3, 2, 1 };
    s16 delayByDifficulty[] = { 8, 6, 2, 1 };
    if (player->delay != 0) {
        player->delay--;
        return 0;
    }
    fn_80026874(player->spinnerModel, &spinnerRotation);
    rotationDegrees = spinnerRotation.y - 34.2f;
    currentBucket = fn_1_33D0(rotationDegrees);
    currentBucket = bucketByWheelPosition[currentBucket];
    if (player->error != 0) {
        if (currentBucket == lbl_1_bss_4FC[side].nextBucket) {
            return 0;
        }
        return 1;
    }
    if (currentBucket == lbl_1_bss_4FC[side].nextBucket) {
        return 1;
    }
    return 0;
}

/* Returns the human player's selection button or asks the CPU turn logic whether to select now. */
u16 fn_1_39F0(s16 side, M640Player *player)
{
    if (player->comDif == -1) {
        return lbl_802C0010[player->padNo] & PAD_BUTTON_A;
    }
    if (fn_1_37F4(side, player) != 0) {
        return PAD_BUTTON_A;
    }
    return 0;
}

/* Updates one player's turn state, from starting the wheel through selecting a piece and settling
 * it in a bucket. */
void fn_1_3C24(s16 side, s16 slot)
{
    HuVecF spinnerRotation;
    M640Player *player = lbl_1_bss_4FC[side].players[slot];
    s16 bucketByWheelPosition[] = { 0, 3, 2, 1 };
    M640Team *team = &lbl_1_bss_4FC[side];
    M640Piece *piece;
    s16 actionMotionFrame;

    fn_80026874(player->spinnerModel, &spinnerRotation);
    if (spinnerRotation.y < 0.0f) {
        spinnerRotation.y += 360.0f;
    }
    switch (player->state) {
    case 1:
        {
        s16 spinSoundByPlayer[] = { M640_PLAYER_SPIN_SFX_0, M640_PLAYER_SPIN_SFX_1,
                                    M640_PLAYER_SPIN_SFX_2, M640_PLAYER_SPIN_SFX_3 };
        player->turnLoopSoundHandle = HuAudFXPlay(spinSoundByPlayer[slot + side * 2]);
        }
        player->state = 2;
        {
            s16 selectionSoundByPlayer[] = { M640_PLAYER_SELECT_SFX_0, M640_PLAYER_SELECT_SFX_1,
                                             M640_PLAYER_SELECT_SFX_2, M640_PLAYER_SELECT_SFX_3 };
            player->selectionSoundHandle = HuAudFXPlay(selectionSoundByPlayer[slot + side * 2]);
        }
        fn_8002D414(player->turnActionModel, 0.0f);
        fn_8002D730(player->turnActionModel, 0.0f,
                              fn_8002D530(player->turnActionModel));
        fn_80026A44(player->turnActionModel, ((1 << 30) | (1 << 2)));
        fn_8002D86C(player->turnActionModel, 1.0f);
        fn_8002D86C(player->smokeEffectModel, 1.0f);
        fn_8002D414(player->smokeEffectModel, 0.0f);
        fn_80026A44(player->smokeEffectModel, ((1 << 30) | (1 << 2)));
        fn_8002D730(player->smokeEffectModel, 0.0f, 80.0f);
        fn_1_345C(side, slot, lbl_1_bss_4FC[side].nextBucket);
        break;
    case 2:
        if (player->speed < 1.8f) {
            player->speed += 0.1;
        } else {
            player->speed = 1.8f;
        }
        actionMotionFrame = fn_8002D4E0(player->turnActionModel);
        if (actionMotionFrame == 60) {
            HuAudFXStop(player->turnLoopSoundHandle);
            player->turnLoopSoundHandle = -1;
            player->state = 3;
            fn_8002D730(player->turnActionModel, 61.0f, 80.0f);
            fn_8002D730(player->smokeEffectModel, 61.0f, 80.0f);
            fn_800269E8(player->turnActionModel, ((1 << 30) | (1 << 0)));
            fn_800269E8(player->smokeEffectModel, ((1 << 30) | (1 << 0)));
            fn_1_36D8(player);
        } else if (actionMotionFrame == 40) {
            fn_8006A030(player->playerNo, 20, 4, 4);
        }
        break;
    case 3:
        if (fn_1_39F0(side, player) != 0) {
            player->selectionFrameCount = 0;
            fn_1_26B8(player->playerNo, 4);
            player->state = 4;
        }
        break;
    case 4:
        player->selectionFrameCount++;
        if ((s16)fn_8005C750(player->charNo) != 0) {
            player->selectionSoundHandle = -1;
            player->speed = 0.0f;
            player->selectedBucket = fn_1_33D0(spinnerRotation.y);
            /* Convert the wheel sector into the bucket's physical order. */
            player->selectedBucket = bucketByWheelPosition[player->selectedBucket];
            player->targetAngle = (s32)((15.0f + spinnerRotation.y) / 30.0f) * 30;
            if (player->targetAngle == 0 && spinnerRotation.y >= 345.0f) {
                /* Keep the near-360-degree target on the high side of the wrap. */
                player->targetAngle = 360;
            }
            fn_80026A44(player->turnActionModel, ((1 << 30) | (1 << 0)));
            fn_8002D414(player->turnActionModel, 81.0f);
            fn_8002D730(player->turnActionModel, 81.0f, 110.0f);
            fn_80026A44(player->smokeEffectModel, ((1 << 30) | (1 << 0)));
            fn_8002D730(player->smokeEffectModel, 81.0f, 140.0f);
            player->state = 5;
            /* These hit and miss cues are played without keeping their returned handles. */
            if (player->selectedBucket == lbl_1_bss_4FC[side].nextBucket) {
                fn_1_4FFC(side, player->selectedBucket);
                {
                    s16 hitSoundByPlayer[] = { M640_PLAYER_HIT_SFX_0, M640_PLAYER_HIT_SFX_1,
                                               M640_PLAYER_HIT_SFX_2, M640_PLAYER_HIT_SFX_3 };
                    HuAudFXPlay(hitSoundByPlayer[slot + side * 2]);
                }
                player->selectionSoundHandle = -1;
            } else {
                fn_1_4DDC(side, player->selectedBucket);
                {
                    s16 missSoundByPlayer[] = { M640_PLAYER_MISS_SFX_0, M640_PLAYER_MISS_SFX_1,
                                                M640_PLAYER_MISS_SFX_2, M640_PLAYER_MISS_SFX_3 };
                    HuAudFXPlay(missSoundByPlayer[slot + side * 2]);
                }
                player->selectionSoundHandle = -1;
            }
        }
        break;
    case 5:
        if (fn_8005C1B8(player->charNo) >= 62.0f) {
            fn_1_26B8(player->playerNo, 0);
        }
        if (fn_8002D4E0(player->turnActionModel) >= 109.0f) {
            s16 spinSoundByPlayer[] = { M640_PLAYER_SPIN_SFX_0, M640_PLAYER_SPIN_SFX_1,
                                        M640_PLAYER_SPIN_SFX_2, M640_PLAYER_SPIN_SFX_3 };
            player->turnLoopSoundHandle = HuAudFXPlay(spinSoundByPlayer[slot + side * 2]);
            fn_8002D730(player->turnActionModel, 0.0f, 60.0f);
            fn_8002D414(player->turnActionModel, 60.0f);
            fn_800269E8(player->turnActionModel, ((1 << 30) | (1 << 2)));
            player->state = 6;
        }
        /* This turn state advances the marker but ignores the alignment result. */
        fn_1_3274(player, &spinnerRotation);
        break;
    case 6:
        if (fn_1_3274(player, &spinnerRotation) != 0) {
            fn_1_345C(side, slot, 4);
        }
        if (team->activePieceIndex != -1
            && ((piece = &lbl_1_bss_C[team->activePieceIndex][side], piece->state == 0)
                || piece->state == 3)) {
            HuAudFXStop(player->turnLoopSoundHandle);
            player->turnLoopSoundHandle = -1;
            if (team->nextBucket == player->selectedBucket) {
                team->bucketPieces[team->nextBucket] = team->activePieceIndex;
                team->nextBucket++;
            }
            team->activePieceIndex = -1;
            if (team->nextBucket < 4) {
                team->players[1 - slot]->state = 1;
            }
            player->state = 0;
            fn_1_345C(side, slot, 4);
            /* The loop sound was already stopped and its handle cleared above; this second stop
             * call still runs. */
            HuAudFXStop(player->turnLoopSoundHandle);
            player->turnLoopSoundHandle = -1;
        }
        break;
    }
    fn_80026808(player->spinnerModel, spinnerRotation.x, spinnerRotation.y - player->speed,
                    spinnerRotation.z);
}

/* Updates both player-turn state machines for one side. */
void fn_1_4C90(s16 side)
{
    int slotIndex;
    for (slotIndex = 0; slotIndex < 2; slotIndex++) {
        fn_1_3C24(side, slotIndex);
    }
}

/* Updates all four players each gameplay frame and advances the sequence when the fourth-bucket
 * check reports a completed side; piece-record index 0 is not detected. */
void fn_1_4CD8(void)
{
    int side, slot;
    for (side = 0; side < 2; side++) {
        for (slot = 0; slot < 2; slot++) {
            fn_1_3C24(side, slot);
        }
    }
    if (fn_1_2FF8()) {
        fn_80071A9C();
    }
}

/* Chooses an available piece variant and starts its placement animation when a player misses the
 * target bucket. */
void fn_1_4DDC(s16 side, s16 part)
{
    M640Piece *piece;
    s16 variant;
    M640Team *team;
    int variantSearchAttempt;
    team = &lbl_1_bss_4FC[side];
    variant = frandmod(3);
    for (variantSearchAttempt = 0; variantSearchAttempt < 3; variantSearchAttempt++) {
        if (lbl_1_bss_C[part + variant * 4][side].state == 0) {
            break;
        }
        variant = (variant + 1) % 3;
    }
    if (variantSearchAttempt == 3) {
        OSReport("no part is waiting\n");
        return;
    }
    fn_8002D86C(lbl_1_bss_4FC[side].missedPieceEffectModel, 1.0f);
    fn_8002D414(lbl_1_bss_4FC[side].missedPieceEffectModel, 0.0f);
    piece = &lbl_1_bss_C[part + variant * 4][side];
    if (piece->placementMotions[team->nextBucket] != 0) {
        fn_8002C95C(piece->pieceModel, piece->placementMotions[team->nextBucket]);
        fn_8002D86C(piece->pieceModel, 1.0f);
        piece->state = 4;
        piece->variant = variant;
        fn_80026A44(piece->pieceModel, (1 << 0));
        fn_80026760(piece->pieceModel, side * 1300, 0.0f, 0.0f);
        team->activePieceIndex = part + variant * 4;
    }
    piece->variant = variant;
    piece->part = part;
}

/* Chooses an available piece variant and starts its drop into the correct target bucket. */
void fn_1_4FFC(s16 side, s16 part)
{
    M640Piece *piece;
    s16 variant;
    int variantSearchAttempt;
    M640Team *team = &lbl_1_bss_4FC[side];
    variant = frandmod(3);
    for (variantSearchAttempt = 0; variantSearchAttempt < 3; variantSearchAttempt++) {
        if (lbl_1_bss_C[part + variant * 4][side].state == 0) {
            break;
        }
        variant = (variant + 1) % 3;
    }
    if (variantSearchAttempt == 3) {
        OSReport("no part is waiting\n");
        return;
    }
    piece = &lbl_1_bss_C[part + variant * 4][side];
    team->activePieceIndex = part + variant * 4;
    piece->state = 1;
    piece->variant = variant;
    piece->part = part;
    fn_80026A44(piece->pieceModel, (1 << 0));
    fn_80026760(piece->pieceModel, side * 1300, 400.0f, 200.0f);
    piece->frame = 0;
    piece->speed = 1.0f;
}

/* Starts a piece falling into a bucket during the timed opening demonstration. For nonzero parts,
 * the availability scan checks each variant's part-0 record before selecting that variant. */
void fn_1_519C(s16 side, s16 part)
{
    M640Piece *piece;
    int variantSearchIndex;
    s16 variant;
    M640Team *team = &lbl_1_bss_4FC[side];
    if (part == 0) {
        variant = frandmod(3);
    } else {
        for (variantSearchIndex = 0; variantSearchIndex < 3; variantSearchIndex++) {
            if (lbl_1_bss_C[variantSearchIndex * 4][side].state != 0) {
                break;
            }
        }
        if (variantSearchIndex < 3) {
            variant = (variantSearchIndex + part) % 3;
        } else {
            variant = 0;
        }
    }
    piece = &lbl_1_bss_C[part + variant * 4][side];
    piece->state = 5;
    piece->speed = 1.0f;
    piece->part = part;
    fn_80026A44(piece->pieceModel, (1 << 0));
}

/* Registered by fn_1_5A28 as an object-manager callback; each update handles drops, bounces,
 * placement effects, and completed-piece cleanup. */
void fn_1_52D0(OMOBJ *pieceUpdateObject)
{
    HuVecF piecePosition;
    M640Piece *piece;
    M640Team *team;
    int side, pieceRecordIndex;
    HSF_DATA *pieceModelData;
    float pieceMotionTime, pieceMotionEndTime;
    float bucketLandingHeight[] = { 40.0f, 100.0f, 160.0f, 220.0f };
    char *pieceBucketHookNames[] = { "itemhook1", "itemhook2", "itemhook3", "itemhook4" };
    float placementSoundCueTimes[4][4] = {
        { 0.0f, 40.0f, 47.0f, 40.0f },
        { 35.0f, 0.0f, 44.0f, 36.0f },
        { 35.0f, 33.0f, 0.0f, 33.0f },
        { 32.0f, 30.0f, 33.0f, 0.0f }
    };
    for (side = 0; side < 2; side++) {
        team = &lbl_1_bss_4FC[side];
        for (pieceRecordIndex = 0; pieceRecordIndex < 12; pieceRecordIndex++) {
            piece = &lbl_1_bss_C[pieceRecordIndex][side];
            fn_800267CC(piece->pieceModel, &piecePosition);
            if (piecePosition.y <= 300.0f) {
                fn_80026E44(piece->pieceModel);
            } else {
                fn_80026EF0(piece->pieceModel);
            }
            switch (piece->state) {
            case 1:
                if (piece->frame++ > 10) {
                    piecePosition.y -= piece->speed;
                    piece->speed += 1.0f;
                }
                if (piecePosition.y <= bucketLandingHeight[team->nextBucket]) {
                    piecePosition.y = bucketLandingHeight[team->nextBucket];
                    piece->speed *= -0.18f;
                    if (piece->part == 0) {
                        HuAudFXPlay(M640_PIECE_PART_0_LANDING_SFX);
                    } else {
                        HuAudFXPlay(M640_PIECE_PART_1_LANDING_SFX);
                    }
                    piece->state = 2;
                    fn_8002D86C(team->bucketEffectModels[team->nextBucket], 1.0f);
                    fn_80026A44(team->bucketEffectModels[team->nextBucket],
                                       (1 << 0));
                    fn_8002D414(team->bucketEffectModels[team->nextBucket], 0.0f);
                }
                break;
            case 2:
                piecePosition.y -= piece->speed;
                piece->speed += 1.0f;
                if (piecePosition.y <= bucketLandingHeight[team->nextBucket]) {
                    piecePosition.y = bucketLandingHeight[team->nextBucket];
                    piece->speed = 0.0f;
                    piece->state = 3;
                }
                break;
            case 4:
                pieceMotionEndTime = fn_8002D530(piece->pieceModel);
                pieceMotionTime = fn_8002D4E0(piece->pieceModel);
                if (pieceMotionTime == placementSoundCueTimes[team->nextBucket][piece->part]) {
                    HuAudFXPlay(M640_PIECE_MOTION_CUE_SFX);
                }
                if (pieceMotionTime >= pieceMotionEndTime) {
                    if (piece->part == 1) {
                        fn_8002C95C(piece->pieceModel, -1);
                    } else {
                        fn_8002D414(piece->pieceModel, 0.0f);
                        fn_8002C95C(piece->pieceModel, piece->resultMotions[0]);
                        fn_8002D86C(piece->pieceModel, 0.0f);
                    }
                    fn_80026760(piece->pieceModel, side * 1300, 400.0f, 200.0f);
                    piecePosition.x = side * 1300;
                    piecePosition.y = 400.0f;
                    piecePosition.z = 200.0f;
                    fn_800269E8(piece->pieceModel, (1 << 0));
                    piece->state = 0;
                    fn_80026CBC(piece->pieceModel, 1.0f);
                    if (pieceRecordIndex == 11) {
                        pieceModelData = lbl_802C01C8[piece->pieceModel].hsf;
                        memcpy(pieceModelData->material, lbl_1_bss_8,
                               pieceModelData->materialNum * sizeof(HSF_MATERIAL));
                    }
                    /* Returning here skips the remaining pieces and side for this object-manager
                     * update. */
                    return;
                }
                if (20.0f + pieceMotionTime > pieceMotionEndTime) {
                    fn_80026CBC(piece->pieceModel,
                                      (pieceMotionEndTime - pieceMotionTime) / 20.0f);
                }
                break;
            case 5:
                piecePosition.y -= piece->speed;
                piece->speed += 1.0f;
                if (piecePosition.y <= bucketLandingHeight[piece->part]) {
                    if (piece->part == 0) {
                        HuAudFXPlay(M640_PIECE_PART_0_LANDING_SFX);
                    } else {
                        HuAudFXPlay(M640_PIECE_PART_1_LANDING_SFX);
                    }
                    piecePosition.y = bucketLandingHeight[piece->part];
                    piece->speed *= -0.18f;
                    piece->state = 6;
                    fn_80026A44(team->hookEffectModels[piece->part], (1 << 0));
                    fn_8002D86C(team->hookEffectModels[piece->part], 1.0f);
                }
                break;
            case 6:
                piecePosition.y -= piece->speed;
                piece->speed += 1.0f;
                if (piecePosition.y <= bucketLandingHeight[piece->part]) {
                    piecePosition.y = bucketLandingHeight[piece->part];
                    piece->speed *= -0.18f;
                    if (fabsf(piece->speed) < 1.0f) {
                        piece->speed = 0.0f;
                        piece->state = 7;
                        fn_8002741C(team->pieceHookModel, pieceBucketHookNames[piece->part],
                                         piece->pieceModel);
                        /* The hooked piece sits at the hook origin. */
                        piecePosition.x = piecePosition.y = piecePosition.z = 0.0f;
                    }
                }
                break;
            }
            fn_80026760(piece->pieceModel, piecePosition.x, piecePosition.y, piecePosition.z);
        }
    }
}

/* Creates both sides' piece models and their joint motions, saves the material state, and registers
 * the piece callback. */
void fn_1_5A28(void)
{
    int pieceRecordIndex, sideIndex, motionSlot;
    HSF_DATA *pieceModelData;
    u16 cameras[] = { 1, 2 };
    memset(lbl_1_bss_C, 0, sizeof(lbl_1_bss_C));
    for (sideIndex = 0; sideIndex < 2; sideIndex++) {
        for (pieceRecordIndex = 0; pieceRecordIndex < 12; pieceRecordIndex++) {
            lbl_1_bss_C[pieceRecordIndex][sideIndex].pieceModel = fn_80025A04(
                fn_80008DD0(lbl_1_data_1A4[pieceRecordIndex], 0x10000000, 2));
            lbl_1_bss_C[pieceRecordIndex][sideIndex].part = pieceRecordIndex % 4;
            fn_80026760(lbl_1_bss_C[pieceRecordIndex][sideIndex].pieceModel, sideIndex * 1300,
                            400.0f, 200.0f);
            fn_80026B64(lbl_1_bss_C[pieceRecordIndex][sideIndex].pieceModel,
                               cameras[sideIndex]);
            fn_800269E8(lbl_1_bss_C[pieceRecordIndex][sideIndex].pieceModel,
                             (1 << 0));
            for (motionSlot = 0; motionSlot < 2; motionSlot++) {
                if (lbl_1_data_84[pieceRecordIndex][motionSlot] != 0) {
                    lbl_1_bss_C[pieceRecordIndex][sideIndex].resultMotions[motionSlot] =
                        fn_80030458(
                            lbl_1_bss_C[pieceRecordIndex][sideIndex].pieceModel,
                            fn_80008DD0(lbl_1_data_84[pieceRecordIndex][motionSlot],
                                                 0x10000000, 2));
                }
            }
            for (motionSlot = 0; motionSlot < 4; motionSlot++) {
                if (lbl_1_data_E4[pieceRecordIndex][motionSlot] != 0) {
                    lbl_1_bss_C[pieceRecordIndex][sideIndex].placementMotions[motionSlot] =
                        fn_80030458(
                            lbl_1_bss_C[pieceRecordIndex][sideIndex].pieceModel,
                            fn_80008DD0(lbl_1_data_E4[pieceRecordIndex][motionSlot],
                                                 0x10000000, 2));
                }
            }
            if (sideIndex == 0 && pieceRecordIndex == 11) {
                pieceModelData =
                    lbl_802C01C8[lbl_1_bss_C[pieceRecordIndex][sideIndex].pieceModel].hsf;
                OSReport("testtesttest");
                lbl_1_bss_8 = HuMemDirectMallocNum(
                    HEAP_HEAP, pieceModelData->materialNum * sizeof(HSF_MATERIAL), 0x10000000);
                memcpy(lbl_1_bss_8, pieceModelData->material,
                       pieceModelData->materialNum * sizeof(HSF_MATERIAL));
            }
        }
    }
    lbl_1_bss_3F0 = fn_8003AB38(lbl_1_bss_4, 50, 0, 0, -1, fn_1_52D0);
}

/* Creates the five upper and twelve lower background models and registers their scrolling
 * callback. */
void fn_1_5DF0(void)
{
    float backgroundLayerHeight[] = { 0.0f, 60.0f, 120.0f, 180.0f };
    int backgroundModelIndex, layerIndex;
    s16 modelVariant, variantEntry;
    for (backgroundModelIndex = 0; backgroundModelIndex < 5; backgroundModelIndex++) {
        modelVariant = frandmod(12);
        lbl_1_bss_3E4[backgroundModelIndex] = fn_80025A04(
            fn_80008DD0(lbl_1_data_1D4[modelVariant], 0x10000000, 2));
        fn_80026B64(lbl_1_bss_3E4[backgroundModelIndex], 3);
        fn_80026760(lbl_1_bss_3E4[backgroundModelIndex], -750.0f + backgroundModelIndex * 480,
                        75.0f, -300.0f);
    }
    for (backgroundModelIndex = 0; backgroundModelIndex < 3; backgroundModelIndex++) {
        modelVariant = frandmod(3);
        for (layerIndex = 0; layerIndex < 4; layerIndex++) {
            variantEntry = (modelVariant + layerIndex) % 3;
            lbl_1_bss_3CC[backgroundModelIndex][layerIndex] = fn_80025A04(fn_80008DD0(
                lbl_1_data_1D4[layerIndex + variantEntry * 4], 0x10000000, 2));
            fn_80026B64(lbl_1_bss_3CC[backgroundModelIndex][layerIndex], 3);
            fn_80026760(lbl_1_bss_3CC[backgroundModelIndex][layerIndex],
                            -750.0f + backgroundModelIndex * 800,
                            75.0f + backgroundLayerHeight[layerIndex], -900.0f);
        }
    }
    lbl_1_bss_3F4 = fn_8003AB38(lbl_1_bss_4, 80, 0, 0, -1, fn_1_60AC);
}

/* Moves the upper and lower background models horizontally each frame and wraps them at the scene
 * edges. */
void fn_1_60AC(OMOBJ *backgroundScrollObject)
{
    HuVecF backgroundPosition;
    int upperModelIndex, lowerModelIndex;
    for (upperModelIndex = 0; upperModelIndex < 5; upperModelIndex++) {
        fn_800267CC(lbl_1_bss_3E4[upperModelIndex], &backgroundPosition);
        backgroundPosition.x += 2.0f;
        if (backgroundPosition.x >= 1950.0f) {
            backgroundPosition.x -= 2700.0f;
        }
        fn_80026760(lbl_1_bss_3E4[upperModelIndex], backgroundPosition.x, backgroundPosition.y,
                        backgroundPosition.z);
    }
    for (upperModelIndex = 0; upperModelIndex < 3; upperModelIndex++) {
        for (lowerModelIndex = 0; lowerModelIndex < 4; lowerModelIndex++) {
            fn_800267CC(lbl_1_bss_3CC[upperModelIndex][lowerModelIndex], &backgroundPosition);
            backgroundPosition.x -= 2.0f;
            if (backgroundPosition.x <= -750.0f) {
                backgroundPosition.x += 2700.0f;
            }
            fn_80026760(lbl_1_bss_3CC[upperModelIndex][lowerModelIndex], backgroundPosition.x,
                            backgroundPosition.y, backgroundPosition.z);
        }
    }
}

/* This callback currently performs no game update. */
void fn_1_623C(void)
{
}
