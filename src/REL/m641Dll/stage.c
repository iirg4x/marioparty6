/* Camera, board-space, and effect setup for the Mole-it! minigame. */
#include "math.h"

#include "dolphin/gx.h"

#include "dolphin/types.h"

#include "dolphin/os.h"

#include "dolphin/mtx.h"

#include "string.h"

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

#include "game/sprite.h"

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

#include "game/hu3d.h"

#include "REL/m641Dll/m641Dll.h"

#include "dolphin.h"

#include "humath.h"

#include "messdir_enum.h"

#include "game/window.h"

#include "game/process.h"

#include "game/flag.h"

#include "game/armem.h"

#include "game/mgdata.h"

#include "game/printfunc.h"

#include "game/saveload.h"

#include "game/esprite.h"

#include "game/thpmain.h"

#include "game/board/main.h"

#include "game/board/audio.h"

#include "game/board/masu.h"

#include "game/board/player.h"

extern s16 lbl_1_bss_7DC;

typedef struct M641CameraConfiguration {
    s32 cameraBit;
    f32 fov;
    f32 nearPlane;
    f32 farPlane;
    f32 aspectRatio;
    f32 viewportX;
    f32 viewportY;
    f32 viewportWidth;
    f32 viewportHeight;
    f32 viewportNearZ;
    f32 viewportFarZ;
    u32 scissorX;
    u32 scissorY;
    u32 scissorWidth;
    u32 scissorHeight;
    Point3d position;
    Point3d up;
    Point3d target;
} M641CameraConfiguration;

#define M641_HU3D_DISPOFF_FLAGS 0x40000001
#define M641_HU3D_ZWRITE_OFF_FLAGS 0x40000002
#define M641_HU3D_DISPOFF_ZWRITE_OFF_FLAGS 0x40000003
#define M641_HU3D_SHADOW_FLAGS 0x40000004
#define M641_HU3D_MOTATTR_FLAG 0x40000000

#define M641_SCHEDULE_ENTRY_COUNT 60

#define M641_MOTION_INFO_COUNT 14

#define M641_SCHEDULE_GROUP_DURATION_FRAMES 600
#define M641_SCHEDULE_RANDOM_OFFSET_FRAMES 60
#define M641_SCHEDULE_SPLIT_FRAME 900
#define M641_SCHEDULE_MAX_FRAME 1770
#define M641_SCHEDULE_LATE_BASE_FRAME 1680
#define M641_SCHEDULE_SECOND_LATE_BASE_FRAME 1740
#define M641_SCHEDULE_FINAL_STEP_FRAMES 10
#define M641_SCHEDULE_FINAL_RANDOM_OFFSET_COUNT 10
#define M641_SCHEDULE_END_FRAME 1800
#define M641_SPACE_FADE_TIMER_FRAMES 120
#define M641_SHORT_SPACE_FADE_TIMER_FRAMES 90

M641CameraConfiguration lbl_1_data_0[2][2] = {
    {
        {1, 15.0f, 10.0f, 15000.0f, 2.2857143878936768f, 0.0f, 0.0f, 640.0f, 280.0f, 0.0f, 1.0f,
         0, 0, 640, 238, {0.0f, 210.0f, 2400.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 210.0f, 0.0f}},
        {2, 15.0f, 10.0f, 15000.0f, 2.2857143878936768f, 0.0f, 200.0f, 640.0f, 280.0f, 0.0f, 1.0f,
         0, 242, 640, 238, {0.0f, 220.0f, 2400.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 220.0f, 0.0f}}
    },
    {
        {1, 15.0f, 10.0f, 15000.0f, 2.2857143878936768f, 0.0f, 0.0f, 640.0f, 280.0f, 0.0f, 1.0f,
         0, 0, 640, 238, {0.0f, 207.0f, 2297.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 207.0f, 0.0f}},
        {2, 15.0f, 10.0f, 15000.0f, 2.2857143878936768f, 0.0f, 200.0f, 640.0f, 280.0f, 0.0f, 1.0f,
         0, 242, 640, 238, {0.0f, 207.0f, 2297.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 207.0f, 0.0f}}
    }
};

Point3d lbl_1_data_180[14] = {
    {-420.0f, 350.0f, 0.0f}, {-280.0f, 350.0f, 0.0f}, {-140.0f, 350.0f, 0.0f},
    {0.0f, 350.0f, 0.0f}, {140.0f, 350.0f, 0.0f}, {280.0f, 350.0f, 0.0f},
    {420.0f, 350.0f, 0.0f}, {-420.0f, 100.0f, 0.0f}, {-280.0f, 100.0f, 0.0f},
    {-140.0f, 100.0f, 0.0f}, {0.0f, 100.0f, 0.0f}, {140.0f, 100.0f, 0.0f},
    {280.0f, 100.0f, 0.0f}, {420.0f, 100.0f, 0.0f},
};

s16 lbl_1_data_228[6][2] = {
    {80, 90},
    {110, 120},
    {0, 0},
    {80, 90},
    {110, 120},
    {80, 90},
};

s32 lbl_1_data_240 = -1;
s32 lbl_1_data_244 = -1;
s32 lbl_1_data_248 = -1;

typedef struct M641LightConfiguration {
    HU3D_LIGHTID primaryLight;
    s16 padding; /* Unused two-byte alignment padding before primaryPosition. */
    HuVecF primaryPosition;
    HuVecF primaryDirection;
    GXColor primaryColor;
    HU3D_LIGHTID alternateLight;
    s16 alternatePadding; /* Unused two-byte alignment padding before alternatePosition. */
    HuVecF alternatePosition;
    HuVecF alternateDirection;
    GXColor alternateColor;
} M641LightConfiguration;

M641LightConfiguration lbl_1_data_24C = {
    0, 0,
    {0.0f, 3000.0f, 3000.0f},
    {0.0f, -1.0f, -1.0f},
    {255, 255, 255, 255},
    0, 0,
    {0.0f, 3000.0f, 3000.0f},
    {0.0f, -1.0f, -1.0f},
    {64, 64, 64, 64},
};

char lbl_1_data_28C[28] = "m641D_field-grid1";

/* The init hook creates the two cameras for the active board layout during scene setup. */
#define M641_CAMERA_CONFIGURATION(layout, camera) \
    (&lbl_1_data_0[(layout)][(camera)])

/* The init hook creates the two cameras for the active board layout during scene setup. */
void fn_1_126C(void) {
    s16 layoutIndex;
    s16 cameraIndex;

    if (lbl_1_bss_7DC == 0) {
        layoutIndex = 0;
    } else {
        layoutIndex = 1;
    }
    cameraIndex = 0;
    while (cameraIndex < 2) {
        Hu3DCameraCreate(M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->cameraBit);
        Hu3DCameraPerspectiveSet(
            M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->cameraBit,
            M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->fov,
            M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->nearPlane,
            M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->farPlane,
            M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->aspectRatio);
        Hu3DCameraViewportSet(
            M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->cameraBit,
            M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->viewportX,
            M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->viewportY,
            M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->viewportWidth,
            M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->viewportHeight,
            M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->viewportNearZ,
            M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->viewportFarZ);
        Hu3DCameraScissorSet(
            M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->cameraBit,
            M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->scissorX,
            M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->scissorY,
            M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->scissorWidth,
            M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->scissorHeight);
        Hu3DCameraPosSetV(M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->cameraBit,
                          &M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->position,
                          &M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->up,
                          &M641_CAMERA_CONFIGURATION(layoutIndex, cameraIndex)->target);
        cameraIndex += 1;
    }
}

#undef M641_CAMERA_CONFIGURATION

extern s16 lbl_1_bss_18;

extern s16 lbl_1_bss_1A;

extern s16 lbl_1_bss_1C;

/* Ages active board sound handles and clears each one after six update frames. */
void fn_1_15FC(void) {
    if ((s32) lbl_1_data_240 >= 0) {
        lbl_1_bss_18 += 1;
        if (lbl_1_bss_18 >= 6) {
            lbl_1_data_240 = -1;
            lbl_1_bss_18 = 0;
        }
    } else {
        lbl_1_data_240 = -1;
        lbl_1_bss_18 = 0;
    }
    if ((s32) lbl_1_data_244 >= 0) {
        lbl_1_bss_1A += 1;
        if (lbl_1_bss_1A >= 6) {
            lbl_1_data_244 = -1;
            lbl_1_bss_1A = 0;
        }
    } else {
        lbl_1_data_244 = -1;
        lbl_1_bss_1A = 0;
    }
    if ((s32) lbl_1_data_248 >= 0) {
        lbl_1_bss_1C += 1;
        if (lbl_1_bss_1C >= 6) {
            lbl_1_data_248 = -1;
            lbl_1_bss_1C = 0;
        }
    } else {
        lbl_1_data_248 = -1;
        lbl_1_bss_1C = 0;
    }
}

extern Point3d lbl_1_bss_C;

extern s32 fn_1_E10(const Point3d *firstPosition, const Point3d *firstDisplacement,
                    f32 firstRadius, const Point3d *secondPosition,
                    const Point3d *secondDisplacement, f32 secondRadius,
                    f32 *result);

extern void fn_1_146B8(Point3d *source, Point3d *destination);

s32 fn_1_8C(Point3d *first, Point3d *firstDisplacement, f32 firstRadius, Point3d *second,
            Point3d *secondDisplacement, f32 secondRadius);

s32 fn_1_540(Point3d *first, f32 firstRadius, f32 firstExtent, Point3d *firstDisplacement,
             Point3d *second, f32 secondRadius, f32 secondExtent, Point3d *secondDisplacement);

s32 fn_1_8C8(Point3d *first, f32 firstRadius, f32 firstExtent, Point3d *second, f32 secondRadius,
             f32 secondExtent);

extern f32 lbl_1_bss_20;

extern s16 lbl_1_bss_24[6];

extern f32 lbl_1_bss_558[2];

const Point3d lbl_1_rodata_78 = {0.0f, 5000.0f, 1.0f};

const Point3d lbl_1_rodata_84 = {0.0f, 1.0f, 0.0f};

const Point3d lbl_1_rodata_90 = {0.0f, 0.0f, 0.0f};

extern void fn_1_1DD4(void);

/* The init hook initializes board shadows, lights, and background models before play begins. */
void fn_1_1798(void) {
    Point3d shadowPosition;
    Point3d shadowUp;
    Point3d shadowTarget;

    if (lbl_1_bss_7DC == 0) {
        Hu3DZClearLayerSet(5);
    }
    shadowPosition = lbl_1_rodata_78;
    shadowUp = lbl_1_rodata_84;
    shadowTarget = lbl_1_rodata_90;
    Hu3DShadowMultiCreate(30.0f, 20.0f, 5000.0f, 3);
    Hu3DShadowMultiTPLvlSet(0.5f, 3);
    Hu3DShadowMultiPosSet(&shadowPosition, &shadowUp, &shadowTarget, 3);

    if (lbl_1_bss_7DC == 0) {
        lbl_1_data_24C.primaryLight = Hu3DGLightCreateV(&lbl_1_data_24C.primaryPosition,
                                                       &lbl_1_data_24C.primaryDirection,
                                                       &lbl_1_data_24C.primaryColor);
        Hu3DGLightStaticSet(lbl_1_data_24C.primaryLight, 1);
        Hu3DGLightInfinitytSet(lbl_1_data_24C.primaryLight);
        lbl_1_data_24C.alternateLight = lbl_1_data_24C.primaryLight;
    } else {
        lbl_1_data_24C.alternateLight = Hu3DGLightCreateV(&lbl_1_data_24C.alternatePosition,
                                                         &lbl_1_data_24C.alternateDirection,
                                                         &lbl_1_data_24C.alternateColor);
        Hu3DGLightStaticSet(lbl_1_data_24C.alternateLight, 1);
        Hu3DGLightInfinitytSet(lbl_1_data_24C.alternateLight);
        lbl_1_data_24C.primaryLight = lbl_1_data_24C.alternateLight;
    }

    if (lbl_1_bss_7DC == 0) {
        lbl_1_bss_24[0] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m641, 4), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_24[1] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m641, 6), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_24[2] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m641, 8), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_24[3] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m641, 0), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_24[4] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m641, 2), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_20 = 0.0f;
    } else {
        lbl_1_bss_24[0] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m641, 5), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_24[1] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m641, 7), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_24[2] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m641, 9), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_24[3] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m641, 1), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_24[4] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m641, 3), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_20 = 4.0f + lbl_1_data_180[7].y;
        lbl_1_bss_24[5] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m641, 11), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_24[5], 3);
        Hu3DModelLayerSet(lbl_1_bss_24[5], 5);
    }
    Hu3DModelAttrSet(lbl_1_bss_24[0], M641_HU3D_DISPOFF_FLAGS);
    Hu3DModelAttrSet(lbl_1_bss_24[1], M641_HU3D_DISPOFF_FLAGS);
    Hu3DModelAttrSet(lbl_1_bss_24[2], M641_HU3D_DISPOFF_FLAGS);
    Hu3DModelAttrSet(lbl_1_bss_24[3], M641_HU3D_DISPOFF_FLAGS);
    Hu3DModelAttrSet(lbl_1_bss_24[4], M641_HU3D_DISPOFF_FLAGS);
    Hu3DModelCameraSet(lbl_1_bss_24[0], 3);
    Hu3DModelCameraSet(lbl_1_bss_24[1], 1);
    Hu3DModelCameraSet(lbl_1_bss_24[2], 2);
    Hu3DModelCameraSet(lbl_1_bss_24[3], 1);
    Hu3DModelCameraSet(lbl_1_bss_24[4], 2);
    Hu3DModelLayerSet(lbl_1_bss_24[0], 0);
    Hu3DModelLayerSet(lbl_1_bss_24[1], 0);
    Hu3DModelLayerSet(lbl_1_bss_24[2], 0);
    if (lbl_1_bss_7DC == 0) {
        Hu3DModelLayerSet(lbl_1_bss_24[3], 5);
        Hu3DModelLayerSet(lbl_1_bss_24[4], 5);
    } else {
        Hu3DModelLayerSet(lbl_1_bss_24[3], 5);
        Hu3DModelLayerSet(lbl_1_bss_24[4], 5);
    }
    if (lbl_1_bss_7DC == 0) {
        Hu3DModelShadowMapObjSet(lbl_1_bss_24[0], lbl_1_data_28C);
    }
    if (lbl_1_bss_7DC == 0) {
        lbl_1_bss_558[0] = lbl_1_data_180[0].y;
    } else {
        lbl_1_bss_558[0] = lbl_1_data_180[7].y;
    }
    fn_1_1DD4();
}

void fn_1_1D7C(void) {
}

/* Fade-in, finish, main, pre-winner, and start callbacks query the active layout's vertical board
 * position here. */
f32 fn_1_1D80(void) {
    if (lbl_1_bss_7DC == 0) {
        return lbl_1_data_180[0].y;
    }
    return 1500.0f;
}

f32 fn_1_1DB4(void) {
    return -600.0f;
}

f32 fn_1_1DC4(void) {
    return 600.0f;
}

extern Point3d lbl_1_bss_30[7];

extern void fn_1_2DD8(void);

/* The init hook builds board space models and lights after fn_1_1798 initializes the scene. */
void fn_1_1DD4(void) {
    Mtx modelMatrix;
    Point3d lightPosition;
    Point3d lightDirection;
    GXColor lightColor;
    s16 entryIndex;
    s16 spaceIndex;
    s16 participantIndex;
    s16 lightId;

    for (entryIndex = 0; entryIndex < 2; entryIndex++) {
        for (spaceIndex = 0; spaceIndex < 7; spaceIndex++) {
            lbl_1_bss_84.boards[entryIndex].spaceModel[spaceIndex] =
                Hu3DModelCreate(
                    HuDataSelHeapReadNum(DATANUM(DATA_m641, 12), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_84.boards[entryIndex].spaceMotion[spaceIndex][0] =
                Hu3DJointMotion(
                    lbl_1_bss_84.boards[entryIndex].spaceModel[spaceIndex],
                    HuDataSelHeapReadNum(DATANUM(DATA_m641, 13), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_84.boards[entryIndex].spaceMotion[spaceIndex][1] =
                Hu3DJointMotion(
                    lbl_1_bss_84.boards[entryIndex].spaceModel[spaceIndex],
                    HuDataSelHeapReadNum(DATANUM(DATA_m641, 14), HU_MEMNUM_OVL, HEAP_MODEL));

            lbl_1_bss_84.boards[entryIndex].itemModel[spaceIndex] =
                Hu3DModelCreate(
                    HuDataSelHeapReadNum(DATANUM(DATA_m641, 17), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_84.boards[entryIndex].itemMotionIds[spaceIndex] =
                Hu3DJointMotion(
                    lbl_1_bss_84.boards[entryIndex].itemModel[spaceIndex],
                    HuDataSelHeapReadNum(DATANUM(DATA_m641, 18), HU_MEMNUM_OVL, HEAP_MODEL));

            Hu3DModelMtxGet(
                lbl_1_bss_84.boards[entryIndex].itemModel[spaceIndex],
                &modelMatrix);
            PSMTXTrans(modelMatrix, 0.0f, 200.0f, 0.0f);
            Hu3DModelMtxSet(
                lbl_1_bss_84.boards[entryIndex].itemModel[spaceIndex],
                &modelMatrix);

            Hu3DModelAttrSet(lbl_1_bss_84.boards[entryIndex].spaceModel[spaceIndex],
                             HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_84.boards[entryIndex].itemModel[spaceIndex],
                             HU3D_ATTR_DISPOFF);
            Hu3DMotionSet(
                lbl_1_bss_84.boards[entryIndex].spaceModel[spaceIndex],
                lbl_1_bss_84.boards[entryIndex].spaceMotion[spaceIndex][1]);
            Hu3DMotionSet(
                lbl_1_bss_84.boards[entryIndex].itemModel[spaceIndex],
                lbl_1_bss_84.boards[entryIndex].itemMotionIds[spaceIndex]);
            Hu3DModelAttrSet(lbl_1_bss_84.boards[entryIndex].spaceModel[spaceIndex],
                             M641_HU3D_DISPOFF_FLAGS);
            Hu3DModelAttrSet(lbl_1_bss_84.boards[entryIndex].itemModel[spaceIndex],
                             M641_HU3D_DISPOFF_FLAGS);

            if (entryIndex == 0) {
                Hu3DModelCameraSet(
                    lbl_1_bss_84.boards[entryIndex].spaceModel[spaceIndex], 1);
                Hu3DModelCameraSet(
                    lbl_1_bss_84.boards[entryIndex].itemModel[spaceIndex], 1);
            } else {
                Hu3DModelCameraSet(
                    lbl_1_bss_84.boards[entryIndex].spaceModel[spaceIndex], 2);
                Hu3DModelCameraSet(
                    lbl_1_bss_84.boards[entryIndex].itemModel[spaceIndex], 2);
            }

            if (lbl_1_bss_7DC == 0) {
                Hu3DModelLayerSet(
                    lbl_1_bss_84.boards[entryIndex].spaceModel[spaceIndex], 5);
                Hu3DModelLayerSet(
                    lbl_1_bss_84.boards[entryIndex].itemModel[spaceIndex], 5);
                Hu3DModelRotSet(
                    lbl_1_bss_84.boards[entryIndex].spaceModel[spaceIndex], 0.0f,
                    0.0f, 180.0f);
                Hu3DModelRotSet(
                    lbl_1_bss_84.boards[entryIndex].itemModel[spaceIndex], 0.0f, 0.0f, 180.0f);
            } else {
                Hu3DModelLayerSet(
                    lbl_1_bss_84.boards[entryIndex].spaceModel[spaceIndex], 5);
                Hu3DModelLayerSet(
                    lbl_1_bss_84.boards[entryIndex].itemModel[spaceIndex],
                    5);
            }

            if (lbl_1_bss_7DC == 0) {
                lightPosition.x = 0.0f;
            lightPosition.y = -1000.0f;
                lightPosition.z = 200.0f;
                lightDirection.x = 0.0f;
                lightDirection.y = 1.0f;
                lightDirection.z = -0.2f;
                lightColor.r = lightColor.g = lightColor.b = lightColor.a = 255;
                lightId = Hu3DLLightCreateV(
                    lbl_1_bss_84.boards[entryIndex].spaceModel[spaceIndex],
                    &lightPosition, &lightDirection, &lightColor);
                Hu3DLLightInfinitytSet(
                    lbl_1_bss_84.boards[entryIndex].spaceModel[spaceIndex],
                    lightId);
                lightId = Hu3DLLightCreateV(
                    lbl_1_bss_84.boards[entryIndex].itemModel[spaceIndex],
                    &lightPosition, &lightDirection, &lightColor);
                Hu3DLLightInfinitytSet(
                    lbl_1_bss_84.boards[entryIndex].itemModel[spaceIndex],
                    lightId);
            } else {
                lightPosition.x = 0.0f;
                lightPosition.y = -10000.0f;
                lightPosition.z = 0.0f;
                lightDirection.x = 0.0f;
                lightDirection.y = 1.0f;
                lightDirection.z = 0.0f;
                lightColor.r = lightColor.g = 224;
                lightColor.b = 0;
                lightColor.a = 255;
                lightId = Hu3DLLightCreateV(
                    lbl_1_bss_84.boards[entryIndex].spaceModel[spaceIndex],
                    &lightPosition, &lightDirection, &lightColor);
                lightId = Hu3DLLightCreateV(
                    lbl_1_bss_84.boards[entryIndex].itemModel[spaceIndex],
                    &lightPosition, &lightDirection, &lightColor);
            }

            lbl_1_bss_84.boards[entryIndex].spaceState[spaceIndex] = 0;
            lbl_1_bss_84.boards[entryIndex].itemState[spaceIndex] = 0;
            lbl_1_bss_84.boards[entryIndex].spaceTimer[spaceIndex] = 0;
            lbl_1_bss_84.boards[entryIndex].itemTimer[spaceIndex] = 0;
            lbl_1_bss_84.boards[entryIndex].spaceFlags[spaceIndex] = 0;
            for (participantIndex = 0; participantIndex < 4; participantIndex++) {
                lbl_1_bss_84.boards[entryIndex].itemFlags[spaceIndex][participantIndex] = 0;
            }
            lbl_1_bss_84.boards[entryIndex].itemSound[spaceIndex] = -1;
        }

        lbl_1_bss_84.boards[entryIndex].specialModel = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m641, 15), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_84.boards[entryIndex].specialMotion[0] = Hu3DJointMotion(
            lbl_1_bss_84.boards[entryIndex].specialModel,
                HuDataSelHeapReadNum(DATANUM(DATA_m641, 13), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_84.boards[entryIndex].specialMotion[1] = Hu3DJointMotion(
            lbl_1_bss_84.boards[entryIndex].specialModel,
                HuDataSelHeapReadNum(DATANUM(DATA_m641, 14), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_84.boards[entryIndex].specialSecondModel = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m641, 16), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_84.boards[entryIndex].specialModel, 1);
        Hu3DModelAttrSet(lbl_1_bss_84.boards[entryIndex].specialSecondModel, 1);
        Hu3DMotionSet(lbl_1_bss_84.boards[entryIndex].specialModel,
                      lbl_1_bss_84.boards[entryIndex].specialMotion[1]);
        Hu3DModelAttrSet(lbl_1_bss_84.boards[entryIndex].specialSecondModel,
                         M641_HU3D_DISPOFF_FLAGS);

        if (entryIndex == 0) {
            Hu3DModelCameraSet(lbl_1_bss_84.boards[entryIndex].specialModel, 1);
            Hu3DModelCameraSet(lbl_1_bss_84.boards[entryIndex].specialSecondModel, 1);
        } else {
            Hu3DModelCameraSet(lbl_1_bss_84.boards[entryIndex].specialModel, 2);
            Hu3DModelCameraSet(lbl_1_bss_84.boards[entryIndex].specialSecondModel, 2);
        }

        if (lbl_1_bss_7DC == 0) {
            Hu3DModelLayerSet(lbl_1_bss_84.boards[entryIndex].specialModel, 5);
            Hu3DModelLayerSet(lbl_1_bss_84.boards[entryIndex].specialSecondModel, 5);
            Hu3DModelRotSet(lbl_1_bss_84.boards[entryIndex].specialModel, 0.0f,
                            0.0f, 180.0f);
            Hu3DModelRotSet(lbl_1_bss_84.boards[entryIndex].specialSecondModel, 0.0f,
                            0.0f, 180.0f);
        } else {
            Hu3DModelLayerSet(lbl_1_bss_84.boards[entryIndex].specialModel, 5);
            Hu3DModelLayerSet(lbl_1_bss_84.boards[entryIndex].specialSecondModel, 5);
        }

        if (lbl_1_bss_7DC == 0) {
            lightPosition.x = 0.0f;
            lightPosition.y = -10000.0f;
            lightPosition.z = 0.0f;
            lightDirection.x = 0.0f;
            lightDirection.y = 1.0f;
            lightDirection.z = 0.0f;
    lightColor.r = lightColor.g = lightColor.b = lightColor.a = 255;
            lightId = Hu3DLLightCreateV(lbl_1_bss_84.boards[entryIndex].specialModel,
                              &lightPosition, &lightDirection, &lightColor);
        } else {
            lightPosition.x = 0.0f;
            lightPosition.y = -10000.0f;
            lightPosition.z = 0.0f;
            lightDirection.x = 0.0f;
            lightDirection.y = 1.0f;
            lightDirection.z = 0.0f;
            lightColor.r = lightColor.g = 224;
            lightColor.b = 0;
            lightColor.a = 255;
            lightId = Hu3DLLightCreateV(lbl_1_bss_84.boards[entryIndex].specialModel,
                              &lightPosition, &lightDirection, &lightColor);
        }
        lbl_1_bss_84.boards[entryIndex].specialState = 0;
        lbl_1_bss_84.boards[entryIndex].specialTimer = 0;
        lbl_1_bss_84.boards[entryIndex].specialSpace = 0;
        lbl_1_bss_84.boards[entryIndex].specialUsed = 0;
    }

    if (lbl_1_bss_7DC == 0) {
        for (entryIndex = 0; entryIndex < 7; entryIndex++) {
            lbl_1_bss_30[entryIndex] = ((Point3d (*)[7])lbl_1_data_180)[0][entryIndex];
        }
    } else {
        for (entryIndex = 0; entryIndex < 7; entryIndex++) {
            lbl_1_bss_30[entryIndex] = ((Point3d (*)[7])lbl_1_data_180)[1][entryIndex];
        }
    }

    for (entryIndex = 0; entryIndex < 2; entryIndex++) {
        for (spaceIndex = 0; spaceIndex < 7; spaceIndex++) {

            Hu3DModelPosSetV(lbl_1_bss_84.boards[entryIndex].spaceModel[spaceIndex],
                             &lbl_1_bss_30[spaceIndex]);
            Hu3DModelPosSetV(
                lbl_1_bss_84.boards[entryIndex].itemModel[spaceIndex],
                &lbl_1_bss_30[spaceIndex]);
        }
    }
    fn_1_2DD8();
}

/* The init hook passes the position index as an integer to this debug hook; the hook discards
 * it. */
static inline void DebugValue(s32 value) { }

/* Randomizes an unused copy of the first action time; frandmod still advances the shared random
 * stream. */
static inline void RandomizeActionTimeCopy(s16 *actionTime) {
    if (*actionTime >= 60) {
        *actionTime = 10 - *actionTime + frandmod(10);
    } else if (*actionTime >= 30) {
        *actionTime = 5 - *actionTime + frandmod(10);
    } else {
        *actionTime = 40 - *actionTime + frandmod(10);
    }
}

/* The init hook builds and sorts board 0 action times and types, then copies the schedule to board
 * 1. */
void fn_1_2DD8(void) {
    s16 positionIndex;
    s16 groupIndex;
    s16 entryIndex;
    s16 boardIndex;
    s16 firstType;
    u8 countTable[18];

    if (lbl_1_bss_7DC == 0) {
        countTable[15] = 4; countTable[16] = 5; countTable[17] = 6;
        countTable[12] = 5; countTable[13] = 4; countTable[14] = 3;
        countTable[9] = 2; countTable[10] = 2; countTable[11] = 2;
        countTable[6] = 3; countTable[7] = 2; countTable[8] = 1;
        countTable[3] = 1; countTable[4] = 2; countTable[5] = 3;
        countTable[0] = 1; countTable[1] = 0; countTable[2] = 1;
    } else {
        countTable[15] = 4; countTable[16] = 5; countTable[17] = 6;
        countTable[12] = 5; countTable[13] = 4; countTable[14] = 3;
        countTable[9] = 2; countTable[10] = 2; countTable[11] = 2;
        countTable[6] = 3; countTable[7] = 2; countTable[8] = 1;
        countTable[3] = 1; countTable[4] = 2; countTable[5] = 3;
        countTable[0] = 1; countTable[1] = 0; countTable[2] = 1;
    }

    for (boardIndex = 0; boardIndex < 2; boardIndex++) {
        lbl_1_bss_84.boards[boardIndex].nextScheduled = 0;
        lbl_1_bss_84.boards[boardIndex].queueCount = 0;
        for (groupIndex = 0; groupIndex < M641_SCHEDULE_ENTRY_COUNT; groupIndex++) {
            lbl_1_bss_84.boards[boardIndex].scheduledTime[groupIndex] = -1;
            lbl_1_bss_84.boards[boardIndex].scheduledType[groupIndex] = -1;
            lbl_1_bss_84.boards[boardIndex].queuedType[groupIndex] = -1;
        }
    }

    boardIndex = 0;
    positionIndex = 0;
    for (groupIndex = 0; groupIndex < 3; groupIndex++) {
        for (entryIndex = 0; entryIndex < countTable[groupIndex]; entryIndex++, positionIndex++) {
            lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] =
                groupIndex * M641_SCHEDULE_GROUP_DURATION_FRAMES +
                frandmod(M641_SCHEDULE_RANDOM_OFFSET_FRAMES);
            if (lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] >
                M641_SCHEDULE_SPLIT_FRAME) {
                lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] +=
                    (M641_SCHEDULE_GROUP_DURATION_FRAMES / (countTable[groupIndex] + 1)) *
                    entryIndex;
            } else {
                lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] +=
                    (M641_SCHEDULE_GROUP_DURATION_FRAMES / (countTable[groupIndex] + 1)) *
                    (entryIndex + 1);
            }
            if (lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] < 0)
                lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] = 0;
            if (lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] >=
                M641_SCHEDULE_MAX_FRAME)
                lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] =
                    M641_SCHEDULE_MAX_FRAME;
            lbl_1_bss_84.boards[boardIndex].scheduledType[positionIndex] = 5;
        }
        for (entryIndex = 0; entryIndex < countTable[groupIndex + 15];
             entryIndex++, positionIndex++) {
            lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] =
                groupIndex * M641_SCHEDULE_GROUP_DURATION_FRAMES +
                (M641_SCHEDULE_GROUP_DURATION_FRAMES / (countTable[groupIndex + 15] + 1)) *
                    entryIndex +
                frandmod(M641_SCHEDULE_RANDOM_OFFSET_FRAMES);
            if (lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] < 0)
                lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] = 0;
            if (lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] >=
                M641_SCHEDULE_MAX_FRAME)
                lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] =
                    M641_SCHEDULE_MAX_FRAME;
            lbl_1_bss_84.boards[boardIndex].scheduledType[positionIndex] = 0;
        }
        for (entryIndex = 0; entryIndex < countTable[groupIndex + 12];
             entryIndex++, positionIndex++) {
            lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] =
                groupIndex * M641_SCHEDULE_GROUP_DURATION_FRAMES +
                (M641_SCHEDULE_GROUP_DURATION_FRAMES / (countTable[groupIndex + 12] + 1)) *
                    entryIndex +
                frandmod(M641_SCHEDULE_RANDOM_OFFSET_FRAMES);
            if (lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] < 0)
                lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] = 0;
            if (lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] >=
                M641_SCHEDULE_MAX_FRAME)
                lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] =
                    M641_SCHEDULE_MAX_FRAME;
            lbl_1_bss_84.boards[boardIndex].scheduledType[positionIndex] = 1;
        }
        for (entryIndex = 0; entryIndex < countTable[groupIndex + 9];
             entryIndex++, positionIndex++) {
            lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] =
                groupIndex * M641_SCHEDULE_GROUP_DURATION_FRAMES +
                (M641_SCHEDULE_GROUP_DURATION_FRAMES / (countTable[groupIndex + 9] + 1)) *
                    entryIndex +
                frandmod(M641_SCHEDULE_RANDOM_OFFSET_FRAMES);
            if (lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] < 0)
                lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] = 0;
            if (lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] >=
                M641_SCHEDULE_MAX_FRAME)
                lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] =
                    M641_SCHEDULE_MAX_FRAME;
            lbl_1_bss_84.boards[boardIndex].scheduledType[positionIndex] = 2;
        }
        for (entryIndex = 0; entryIndex < countTable[groupIndex + 6];
             entryIndex++, positionIndex++) {
            lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] =
                groupIndex * M641_SCHEDULE_GROUP_DURATION_FRAMES +
                (M641_SCHEDULE_GROUP_DURATION_FRAMES / (countTable[groupIndex + 6] + 1)) *
                    (entryIndex + 1) +
                frandmod(M641_SCHEDULE_RANDOM_OFFSET_FRAMES);
            if (lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] < 0)
                lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] = 0;
            if (lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] >=
                M641_SCHEDULE_MAX_FRAME)
                lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] =
                    M641_SCHEDULE_MAX_FRAME;
            lbl_1_bss_84.boards[boardIndex].scheduledType[positionIndex] = 3;
        }
        for (entryIndex = 0; entryIndex < countTable[groupIndex + 3];
             entryIndex++, positionIndex++) {
            lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] =
                groupIndex * M641_SCHEDULE_GROUP_DURATION_FRAMES +
                (M641_SCHEDULE_GROUP_DURATION_FRAMES / (countTable[groupIndex + 3] + 1)) *
                    entryIndex +
                frandmod(M641_SCHEDULE_RANDOM_OFFSET_FRAMES);
            if (lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] < 0)
                lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] = 0;
            if (lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] >=
                M641_SCHEDULE_MAX_FRAME)
                lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] =
                    M641_SCHEDULE_MAX_FRAME;
            lbl_1_bss_84.boards[boardIndex].scheduledType[positionIndex] = 4;
        }
    }

    lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] =
        frandmod(7) * 5 + M641_SCHEDULE_LATE_BASE_FRAME;
    lbl_1_bss_84.boards[boardIndex].scheduledType[positionIndex] = 1;
    positionIndex++;
    lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] =
        frandmod(3) * M641_SCHEDULE_FINAL_STEP_FRAMES +
        M641_SCHEDULE_SECOND_LATE_BASE_FRAME;
    lbl_1_bss_84.boards[boardIndex].scheduledType[positionIndex] = 3;
    positionIndex++;
    lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] =
        frandmod(M641_SCHEDULE_FINAL_RANDOM_OFFSET_COUNT) + M641_SCHEDULE_MAX_FRAME;
    lbl_1_bss_84.boards[boardIndex].scheduledType[positionIndex] = 0;
    positionIndex++;
    lbl_1_bss_84.boards[boardIndex].scheduledTime[positionIndex] =
        M641_SCHEDULE_END_FRAME - (frandmod(M641_SCHEDULE_FINAL_RANDOM_OFFSET_COUNT) + 5);
    lbl_1_bss_84.boards[boardIndex].scheduledType[positionIndex] = 2;

    positionIndex++;
    boardIndex = 0;
    for (groupIndex = 0; groupIndex < M641_SCHEDULE_ENTRY_COUNT; groupIndex++) {
        for (entryIndex = groupIndex + 1; entryIndex < M641_SCHEDULE_ENTRY_COUNT; entryIndex++) {
            if (lbl_1_bss_84.boards[boardIndex].scheduledTime[groupIndex] >= 0 &&
                lbl_1_bss_84.boards[boardIndex].scheduledTime[entryIndex] >= 0 &&
                lbl_1_bss_84.boards[boardIndex].scheduledTime[groupIndex] >
                    lbl_1_bss_84.boards[boardIndex].scheduledTime[entryIndex]) {
                positionIndex = lbl_1_bss_84.boards[boardIndex].scheduledTime[groupIndex];
                firstType = lbl_1_bss_84.boards[boardIndex].scheduledType[groupIndex];
                lbl_1_bss_84.boards[boardIndex].scheduledTime[groupIndex] =
                    lbl_1_bss_84.boards[boardIndex].scheduledTime[entryIndex];

                lbl_1_bss_84.boards[boardIndex].scheduledType[groupIndex] =
                    lbl_1_bss_84.boards[boardIndex].scheduledType[entryIndex];
                lbl_1_bss_84.boards[boardIndex].scheduledTime[entryIndex] = positionIndex;
                lbl_1_bss_84.boards[boardIndex].scheduledType[entryIndex] = firstType;
            }
        }
    }

    boardIndex = 0;
    positionIndex = lbl_1_bss_84.boards[boardIndex].scheduledTime[0];
    RandomizeActionTimeCopy(&positionIndex);

    for (groupIndex = 1; groupIndex < 2; groupIndex++) {
        for (entryIndex = 0; entryIndex < M641_SCHEDULE_ENTRY_COUNT; entryIndex++) {
            lbl_1_bss_84.boards[groupIndex].scheduledTime[entryIndex] =
                lbl_1_bss_84.boards[boardIndex].scheduledTime[entryIndex];
            DebugValue(positionIndex);
            lbl_1_bss_84.boards[groupIndex].scheduledType[entryIndex] =
                lbl_1_bss_84.boards[boardIndex].scheduledType[entryIndex];
        }
    }
}

/* Fade-in, fade-out, main, and winner hooks start the requested space, item, or special-space
 * animation for its duration. */
s32 fn_1_3DA0(s16 boardIndex, s16 spaceIndex, s16 action, s16 durationFrames) {
    if ((lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] == 0) &&
        (lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] == 0) &&
        ((lbl_1_bss_84.boards[boardIndex].specialState == 0) ||
         (spaceIndex != lbl_1_bss_84.boards[boardIndex].specialSpace))) {
        if (action == 0) {
            lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] = 1;
            lbl_1_bss_84.boards[boardIndex].spaceTimer[spaceIndex] = durationFrames;
            return 0;
        } else if (action == 1) {
            lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] = 1;
            lbl_1_bss_84.boards[boardIndex].itemTimer[spaceIndex] = durationFrames;
            return 0;
        } else if (action == 2) {
            lbl_1_bss_84.boards[boardIndex].specialSpace = spaceIndex;
            lbl_1_bss_84.boards[boardIndex].specialState = 1;
            lbl_1_bss_84.boards[boardIndex].specialTimer = durationFrames;
            return 0;
        }
    }
    return 1;
}

/* Fade-in, finish, main, pre-winner, and start hooks start touched-space return animations and pan
 * their sounds from the model positions. */
void fn_1_3F64(s16 boardIndex, s16 spaceIndex) {
    extern void Hu3DModelPosGet();
    extern void HuAudFXPanning();
    extern int HuAudFXPlay();
    Point3d position;
    f32 panning;

    if (boardIndex >= 0 && boardIndex < 2 && spaceIndex >= 0 && spaceIndex < 7) {
        if (lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] == 1 ||
            lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] == 2 ||
            lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] == 3) {
            lbl_1_bss_84.boards[boardIndex].spaceTimer[spaceIndex] = 0;
            lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] = 4;
            Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                            &position);
            panning = 1200.0f;
            panning = (position.x - -600.0f) / panning;
            panning = 32.0f + 64.0f * panning;
            HuAudFXPanning(HuAudFXPlay(0x794), (s32)panning);
        }
        if (lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] == 1 ||
            lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] == 2 ||
            lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] == 3) {
            lbl_1_bss_84.boards[boardIndex].itemTimer[spaceIndex] = 0;
            lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] = 4;
            Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex], &position);
            panning = 1200.0f;
            panning = (position.x - -600.0f) / panning;
            panning = 32.0f + 64.0f * panning;
            HuAudFXPanning(HuAudFXPlay(0x795), (s32)panning);
        }
        if (spaceIndex == lbl_1_bss_84.boards[boardIndex].specialSpace &&
            (lbl_1_bss_84.boards[boardIndex].specialState == 1 ||
             lbl_1_bss_84.boards[boardIndex].specialState == 2 ||
             lbl_1_bss_84.boards[boardIndex].specialState == 3)) {
            lbl_1_bss_84.boards[boardIndex].specialTimer = 0;
            lbl_1_bss_84.boards[boardIndex].specialState = 4;
            Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].specialModel, &position);
            panning = 1200.0f;
            panning = (position.x - -600.0f) / panning;
            panning = 32.0f + 64.0f * panning;
            HuAudFXPanning(HuAudFXPlay(0x794), (s32)panning);
        }
    }
}

/* Marks active space, item, and special-space effects for their return animation. */
void fn_1_43C8(s16 boardIndex, s16 spaceIndex) {
    if ((boardIndex >= 0) && (boardIndex < 2) &&
        (spaceIndex >= 0) && (spaceIndex < 7)) {
        if ((lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] == 1) ||
            (lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] == 2) ||
            (lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] == 3)) {
            lbl_1_bss_84.boards[boardIndex].spaceTimer[spaceIndex] = 0;
            lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] = 4;
        }
        if ((lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] == 1) ||
            (lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] == 2) ||
            (lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] == 3)) {
            lbl_1_bss_84.boards[boardIndex].itemTimer[spaceIndex] = 0;
            lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] = 4;
        }
        if ((spaceIndex == lbl_1_bss_84.boards[boardIndex].specialSpace) &&
            ((lbl_1_bss_84.boards[boardIndex].specialState == 1) ||
             (lbl_1_bss_84.boards[boardIndex].specialState == 2) ||
             (lbl_1_bss_84.boards[boardIndex].specialState == 3))) {
            lbl_1_bss_84.boards[boardIndex].specialTimer = 0;
            lbl_1_bss_84.boards[boardIndex].specialState = 4;
        }
    }
}

struct _struct_lbl_1_data_180_0xC {
    s32 unknownInteger; /* Read as lbl_1_data_180[1].x through the Point3d table view. */
    f32 unknownFloatA; /* Read as lbl_1_data_180[1].y through the Point3d table view. */
    f32 unknownFloatB; /* Read as lbl_1_data_180[1].z through the Point3d table view. */
};

void fn_1_4660(s16 boardIndex, s16 spaceIndex, s16 kind);

s16 fn_1_9ABC(s16);

s32 fn_1_9C24(s16, s16);

s16 fn_1_A204(s16);

extern Point3d lbl_1_bss_560[4];

/* Fade-in, fade-out, finish, main, pre-winner, and winner updates advance one board space's
 * selected model, motion, and sound here. */
void fn_1_4660(s16 boardIndex, s16 spaceIndex, s16 kind) {
    Point3d position;
    Point3d boundaryPosition;
    f32 direction;
    f32 panning;
    s16 flagIndex;

    if (lbl_1_bss_7DC == 0) {
        direction = -1;
    } else {
        direction = 1;
    }
    if (kind == 0) {
        switch (lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex]) {
        case 1:
            Hu3DModelAttrReset(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex], 1);
            Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex], &position);
        position.y = lbl_1_bss_558[0] - (100.0f * direction);
            Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex], &position);
            lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] = 2;
            Hu3DMotionSet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                          lbl_1_bss_84.boards[boardIndex].spaceMotion[spaceIndex][0]);
            Hu3DModelAttrReset(
                lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                Hu3DModelMotionAttrGet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex]) |
                                   M641_HU3D_MOTATTR_FLAG);
            Hu3DModelAttrSet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                             M641_HU3D_DISPOFF_FLAGS);
            lbl_1_bss_84.boards[boardIndex].spaceFlags[spaceIndex] = 0;
            if (lbl_1_data_240 < 0) {
                Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex], &position);
                /* This pan is calculated but unused; the sound starts with centered pan. */
                panning = 1200.0f;
                panning = (position.x - (-600.0f)) / panning;
                panning = 32.0f + 64.0f * panning;
                lbl_1_data_240 = HuAudFXPlay(0x790);
            }
            break;
        case 2:
            Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex], &position);
            position.y += 5.0f * direction;
            Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex], &position);
        boundaryPosition.y = lbl_1_bss_558[0] - (50.0f * direction);
            if (direction < 0.0f) {
                if (position.y <= boundaryPosition.y) {
                    boundaryPosition.x = position.x;
                    boundaryPosition.z = position.z;
                    Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                                     &boundaryPosition);
                    lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] = 3;
                    return;
                }
            } else if (position.y >= boundaryPosition.y) {
    boundaryPosition.x = position.x;
                    boundaryPosition.z = position.z;
                    Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                                     &boundaryPosition);
                    lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] = 3;
                    return;
            }
            break;
        case 3:
            lbl_1_bss_84.boards[boardIndex].spaceTimer[spaceIndex]--;
            if (lbl_1_bss_84.boards[boardIndex].spaceTimer[spaceIndex] <= 0) {
                lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] = 4;
            }
            if (Hu3DMotionShiftIDGet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex]) < 0) {
                if ((Hu3DMotionTimeGet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex]) +
                     Hu3DData[lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex]]
                         .motWork.speed) >=
                    Hu3DMotionMaxTimeGet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex])) {
                    Hu3DMotionSet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                                  lbl_1_bss_84.boards[boardIndex].spaceMotion[spaceIndex][1]);
                    return;
                }
            }
            break;
        case 4:
            Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex], &position);
            position.y -= 5.0f * direction;
            Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex], &position);
        boundaryPosition.y = lbl_1_bss_558[0] - (100.0f * direction);
            if (direction < 0.0f) {
                if (position.y >= boundaryPosition.y) {
    boundaryPosition.x = position.x;
                    boundaryPosition.z = position.z;
                    Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                                     &boundaryPosition);
                    lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] = 5;
                }
            } else if (position.y <= boundaryPosition.y) {
    boundaryPosition.x = position.x;
                    boundaryPosition.z = position.z;
                    Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                                     &boundaryPosition);
                    lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] = 5;
            }
            if ((lbl_1_bss_84.boards[boardIndex].spaceMotion[spaceIndex][0] ==
                 Hu3DMotionIDGet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex])) &&
                (Hu3DMotionTimeGet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex]) <=
                 9.0f)) {
                Hu3DModelAttrSet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                                 M641_HU3D_ZWRITE_OFF_FLAGS);
                return;
            }
            if ((lbl_1_bss_84.boards[boardIndex].spaceMotion[spaceIndex][0] ==
                 Hu3DMotionIDGet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex])) &&
                ((u32) (Hu3DModelMotionAttrGet(
                            lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex]) &
                        M641_HU3D_SHADOW_FLAGS) == 0)) {
                Hu3DModelAttrReset(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                                   M641_HU3D_ZWRITE_OFF_FLAGS);
                Hu3DModelAttrSet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                                 M641_HU3D_SHADOW_FLAGS);
                return;
            }
            if (lbl_1_bss_84.boards[boardIndex].spaceMotion[spaceIndex][1] ==
                Hu3DMotionIDGet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex])) {
                Hu3DModelAttrReset(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                                   M641_HU3D_ZWRITE_OFF_FLAGS);
                Hu3DMotionSet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                              lbl_1_bss_84.boards[boardIndex].spaceMotion[spaceIndex][0]);
                Hu3DMotionTimeSet(
                    lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                    Hu3DMotionMaxTimeGet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex]) -
                        2.0f);
                Hu3DModelAttrSet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                                 M641_HU3D_SHADOW_FLAGS);
                return;
            }
            break;
        case 5:
            Hu3DModelAttrSet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex], 1);
            Hu3DModelAttrReset(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                               M641_HU3D_DISPOFF_ZWRITE_OFF_FLAGS);
            Hu3DModelAttrSet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                             M641_HU3D_DISPOFF_FLAGS);
            lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] = 0;
            return;
        case 6:
            Hu3DModelAttrReset(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex], 1);
            Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex], &position);
        position.y = lbl_1_bss_558[0] - (50.0f * direction);
            Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex], &position);
            lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] = 3;
            Hu3DMotionSet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                          lbl_1_bss_84.boards[boardIndex].spaceMotion[spaceIndex][1]);
            Hu3DModelAttrReset(
                lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                Hu3DModelMotionAttrGet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex]) |
                                       M641_HU3D_MOTATTR_FLAG);
            Hu3DModelAttrSet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                             M641_HU3D_DISPOFF_FLAGS);
            return;
        }
    } else if (kind == 1) {
        switch (lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex]) {
        case 1:
            Hu3DModelAttrReset(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex], 1);
            Hu3DModelAttrSet(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex],
                             M641_HU3D_ZWRITE_OFF_FLAGS);
            Hu3DMotionTimeSet(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex], 0.0f);
            Hu3DModelRotGet(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex], &position);
            Hu3DModelRotSet(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex], position.x, 0.0f,
                            position.z);
            Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex], &position);
        position.y = lbl_1_bss_558[0] - (350.0f * direction);
            Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex], &position);
            lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] = 2;
            for (flagIndex = 0; flagIndex < 4; flagIndex++) {
                lbl_1_bss_84.boards[boardIndex].itemFlags[spaceIndex][flagIndex] = 0;
            }
            if (lbl_1_data_244 < 0) {
                Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex], &position);
                /* This pan is calculated but unused; the sound starts with centered pan. */
                panning = 1200.0f;
                panning = (position.x - (-600.0f)) / panning;
                panning = 32.0f + 64.0f * panning;
                lbl_1_data_244 = HuAudFXPlay(0x791);
                return;
            }
            break;
        case 2:
            Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex], &position);
            position.y += 9.583333f * direction;
            Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex], &position);
        boundaryPosition.y = lbl_1_bss_558[0] - (230.0f * direction);
            if (direction < 0.0f) {
                if (position.y <= boundaryPosition.y) {
    boundaryPosition.x = position.x;
                    boundaryPosition.z = position.z;
                    Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex],
                                     &boundaryPosition);
                    lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] = 3;
                    return;
                }
            } else if (position.y >= boundaryPosition.y) {
    boundaryPosition.x = position.x;
                    boundaryPosition.z = position.z;
                    Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex],
                                     &boundaryPosition);
                    lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] = 3;
                    return;
            }
            break;
        case 3:
            if ((s32) lbl_1_bss_84.boards[boardIndex].itemSound[spaceIndex] < 0) {
                Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex], &position);
                panning = 1200.0f;
                panning = (position.x - (-600.0f)) / panning;
                panning = 32.0f + 64.0f * panning;
                lbl_1_bss_84.boards[boardIndex].itemSound[spaceIndex] = HuAudFXPlay(0x792);
                HuAudFXPanning(lbl_1_bss_84.boards[boardIndex].itemSound[spaceIndex], panning);
            }
            Hu3DModelAttrReset(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex],
                               M641_HU3D_ZWRITE_OFF_FLAGS);
            lbl_1_bss_84.boards[boardIndex].itemTimer[spaceIndex]--;
            if (lbl_1_bss_84.boards[boardIndex].itemTimer[spaceIndex] <= 0) {
                lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] = 4;
                Hu3DModelAttrSet(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex],
                                 M641_HU3D_ZWRITE_OFF_FLAGS);
                return;
            }
            break;
        case 4:
            Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex], &position);
            position.y -= 9.583333f * direction;
            Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex], &position);
        boundaryPosition.y = lbl_1_bss_558[0] - (350.0f * direction);
            if (direction < 0.0f) {
                if (position.y >= boundaryPosition.y) {
    boundaryPosition.x = position.x;
                    boundaryPosition.z = position.z;
                    Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex],
                                     &boundaryPosition);
                    lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] = 5;
                }
            } else if (position.y <= boundaryPosition.y) {
    boundaryPosition.x = position.x;
                    boundaryPosition.z = position.z;
                    Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex],
                                     &boundaryPosition);
                    lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] = 5;
            }
            lbl_1_bss_84.boards[boardIndex].itemSound[spaceIndex] = -1;
            return;
        case 5:
            Hu3DModelAttrSet(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex], 1);
            lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] = 0;
            return;
        case 6:
            Hu3DModelAttrReset(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex], 1);
            Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex], &position);
        position.y = lbl_1_bss_558[0] - (230.0f * direction);
            Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex], &position);
            lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] = 3;
            return;
        }
    } else if (kind == 2) {
        switch (lbl_1_bss_84.boards[boardIndex].specialState) {
        case 1:
            Hu3DModelAttrReset(lbl_1_bss_84.boards[boardIndex].specialModel, 1);
            Hu3DModelAttrReset(lbl_1_bss_84.boards[boardIndex].specialSecondModel, 1);
            position = lbl_1_data_180[lbl_1_bss_84.boards[boardIndex].specialSpace];
        position.y = lbl_1_bss_558[0] - (100.0f * direction);
            Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].specialModel, &position);
            Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].specialSecondModel, &position);
            lbl_1_bss_84.boards[boardIndex].specialState = 2;
            Hu3DMotionSet(lbl_1_bss_84.boards[boardIndex].specialModel,
                          lbl_1_bss_84.boards[boardIndex].specialMotion[0]);
            Hu3DModelAttrReset(
                lbl_1_bss_84.boards[boardIndex].specialModel,
                Hu3DModelMotionAttrGet(lbl_1_bss_84.boards[boardIndex].specialModel) |
                    M641_HU3D_MOTATTR_FLAG);
            Hu3DModelAttrSet(lbl_1_bss_84.boards[boardIndex].specialModel, M641_HU3D_DISPOFF_FLAGS);
            lbl_1_bss_84.boards[boardIndex].specialUsed = 0;
            if (lbl_1_data_248 < 0) {
                Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].specialModel, &position);
                /* This pan is calculated but unused; the sound starts with centered pan. */
                panning = 1200.0f;
                panning = (position.x - (-600.0f)) / panning;
                panning = 32.0f + 64.0f * panning;
                lbl_1_data_248 = HuAudFXPlay(0x793);
                return;
            }
            break;
        case 2:
            Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].specialModel, &position);
            position.y += 5.0f * direction;
            Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].specialModel, &position);
            Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].specialSecondModel, &position);
        boundaryPosition.y = lbl_1_bss_558[0] - (50.0f * direction);
            if (direction < 0.0f) {
                if (position.y <= boundaryPosition.y) {
    boundaryPosition.x = position.x;
                    boundaryPosition.z = position.z;
                    Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].specialModel,
                                     &boundaryPosition);
                    Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].specialSecondModel,
                                     &boundaryPosition);
                    lbl_1_bss_84.boards[boardIndex].specialState = 3;
                    return;
                }
            } else if (position.y >= boundaryPosition.y) {
    boundaryPosition.x = position.x;
                    boundaryPosition.z = position.z;
                Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].specialModel, &boundaryPosition);
                Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].specialSecondModel,
                                 &boundaryPosition);
                lbl_1_bss_84.boards[boardIndex].specialState = 3;
                return;
            }
            break;
        case 3:
            lbl_1_bss_84.boards[boardIndex].specialTimer--;
            if (lbl_1_bss_84.boards[boardIndex].specialTimer <= 0) {
                lbl_1_bss_84.boards[boardIndex].specialState = 4;
            }
            if (Hu3DMotionShiftIDGet(lbl_1_bss_84.boards[boardIndex].specialModel) < 0) {
                if ((Hu3DData[lbl_1_bss_84.boards[boardIndex].specialModel].motWork.speed +
                     Hu3DMotionTimeGet(lbl_1_bss_84.boards[boardIndex].specialModel)) >=
                    Hu3DMotionMaxTimeGet(lbl_1_bss_84.boards[boardIndex].specialModel)) {
                    Hu3DMotionSet(lbl_1_bss_84.boards[boardIndex].specialModel,
                                  lbl_1_bss_84.boards[boardIndex].specialMotion[1]);
                    return;
                }
            }
            break;
        case 4:
            Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].specialModel, &position);
            position.y -= 5.0f * direction;
            Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].specialModel, &position);
            Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].specialSecondModel, &position);
        boundaryPosition.y = lbl_1_bss_558[0] - (100.0f * direction);
            if (direction < 0.0f) {
                if (position.y >= boundaryPosition.y) {
    boundaryPosition.x = position.x;
                    boundaryPosition.z = position.z;
                    Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].specialModel,
                                     &boundaryPosition);
                    Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].specialSecondModel,
                                     &boundaryPosition);
                    lbl_1_bss_84.boards[boardIndex].specialState = 5;
                }
            } else if (position.y <= boundaryPosition.y) {
    boundaryPosition.x = position.x;
                    boundaryPosition.z = position.z;
                Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].specialModel, &boundaryPosition);
                Hu3DModelPosSetV(lbl_1_bss_84.boards[boardIndex].specialSecondModel,
                                 &boundaryPosition);
                lbl_1_bss_84.boards[boardIndex].specialState = 5;
            }
            if ((lbl_1_bss_84.boards[boardIndex].specialMotion[0] ==
                 Hu3DMotionIDGet(lbl_1_bss_84.boards[boardIndex].specialModel)) &&
                (Hu3DMotionTimeGet(lbl_1_bss_84.boards[boardIndex].specialModel) <= 9.0f)) {
                Hu3DModelAttrSet(lbl_1_bss_84.boards[boardIndex].specialModel,
                                 M641_HU3D_ZWRITE_OFF_FLAGS);
                return;
            }
            if ((lbl_1_bss_84.boards[boardIndex].specialMotion[0] ==
                 Hu3DMotionIDGet(lbl_1_bss_84.boards[boardIndex].specialModel)) &&
                ((u32) (Hu3DModelMotionAttrGet(lbl_1_bss_84.boards[boardIndex].specialModel) &
                        M641_HU3D_SHADOW_FLAGS) == 0)) {
                Hu3DModelAttrReset(lbl_1_bss_84.boards[boardIndex].specialModel,
                                   M641_HU3D_ZWRITE_OFF_FLAGS);
                Hu3DModelAttrSet(lbl_1_bss_84.boards[boardIndex].specialModel,
                                 M641_HU3D_SHADOW_FLAGS);
                return;
            }
            if (lbl_1_bss_84.boards[boardIndex].specialMotion[1] ==
                Hu3DMotionIDGet(lbl_1_bss_84.boards[boardIndex].specialModel)) {
                Hu3DModelAttrReset(lbl_1_bss_84.boards[boardIndex].specialModel,
                                   M641_HU3D_ZWRITE_OFF_FLAGS);
                Hu3DMotionSet(lbl_1_bss_84.boards[boardIndex].specialModel,
                              lbl_1_bss_84.boards[boardIndex].specialMotion[0]);
                Hu3DMotionTimeSet(
                    lbl_1_bss_84.boards[boardIndex].specialModel,
                    Hu3DMotionMaxTimeGet(lbl_1_bss_84.boards[boardIndex].specialModel) - 2.0f);
                Hu3DModelAttrSet(lbl_1_bss_84.boards[boardIndex].specialModel,
                                 M641_HU3D_SHADOW_FLAGS);
                return;
            }
            break;
        case 5:
            Hu3DModelAttrSet(lbl_1_bss_84.boards[boardIndex].specialModel, 1);
            Hu3DModelAttrSet(lbl_1_bss_84.boards[boardIndex].specialSecondModel, 1);
            Hu3DModelAttrReset(lbl_1_bss_84.boards[boardIndex].specialModel,
                               M641_HU3D_DISPOFF_ZWRITE_OFF_FLAGS);
            Hu3DModelAttrSet(lbl_1_bss_84.boards[boardIndex].specialModel, M641_HU3D_DISPOFF_FLAGS);
            lbl_1_bss_84.boards[boardIndex].specialState = 0;
            return;
        case 6:
            lbl_1_bss_84.boards[boardIndex].specialState = 5;
            break;
        }
    }
}

/* Fade-in calls with frame -1 to advance space animations; main-phase frames start scheduled
 * actions and advance the space animations. */
void fn_1_682C(s16 frame) {
    Point3d position;
    s16 availability[8];
    s16 boardIndex;
    s16 action;
    s16 scanIndex;
    s16 queueIndex;
    s16 chosenSpace;
    s16 availableCount;
    s16 delay;

    for (boardIndex = 0; boardIndex < 2; boardIndex++) {
        if (frame >= 0) {
            while (frame == lbl_1_bss_84.boards[boardIndex]
                                .scheduledTime[lbl_1_bss_84.boards[boardIndex].nextScheduled]) {
                action = lbl_1_bss_84.boards[boardIndex]
                             .scheduledType[lbl_1_bss_84.boards[boardIndex].nextScheduled];
                if (action == 0 || action == 1 || action == 2 || action == 5 || action == 3 ||
                    action == 4) {
                    chosenSpace = 0;
                    queueIndex = 0;
                    availableCount = 0;
                    for (; queueIndex < 7; queueIndex++) {
                        availability[queueIndex] = fn_1_9C24(boardIndex, queueIndex);
                        if (availability[queueIndex] == 0) {
                            availableCount++;
                        }
                    }
                    if (availableCount > 0) {
                        availableCount = frandmod(availableCount) + 1;
                        for (queueIndex = 0; queueIndex < 7; queueIndex++) {
                            if (availability[queueIndex] == 0) {
                                availableCount--;
                            }
                            if (availableCount == 0) {
                                chosenSpace = queueIndex;
                                break;
                            }
                        }
                        if (action == 3 || action == 4) {
                            if (chosenSpace > 0 &&
                                lbl_1_bss_84.boards[boardIndex].itemState[chosenSpace - 1] != 0) {
                                action = -1;
                                availableCount = -1;
                            }
                            if (chosenSpace < 6 &&
                                lbl_1_bss_84.boards[boardIndex].itemState[chosenSpace + 1] != 0) {
                                action = -1;
                                availableCount = -1;
                            }
                        }
                    } else {
                        lbl_1_bss_84.boards[boardIndex]
                            .queuedType[lbl_1_bss_84.boards[boardIndex].queueCount] = action;
                        lbl_1_bss_84.boards[boardIndex].queueCount++;
                        action = -1;
                        availableCount = -1;
                    }
                    if (lbl_1_bss_7DC == 1) {
                        position = ((Point3d (*)[7])lbl_1_data_180)[1][chosenSpace];
                        for (queueIndex = 0; queueIndex < 4; queueIndex++) {
                            if (boardIndex == fn_1_A204(queueIndex) && action != -1 &&
                                (((position.x - 70.0f <= lbl_1_bss_560[queueIndex].x - 40.0f) &&
                                  (lbl_1_bss_560[queueIndex].x - 40.0f <= 70.0f + position.x)) ||
                                 ((position.x - 70.0f <= 40.0f + lbl_1_bss_560[queueIndex].x) &&
                                  (40.0f + lbl_1_bss_560[queueIndex].x <= 70.0f + position.x)))) {
                                lbl_1_bss_84.boards[boardIndex]
                                    .queuedType[lbl_1_bss_84.boards[boardIndex].queueCount] =
                                    action;
                                lbl_1_bss_84.boards[boardIndex].queueCount++;
                                action = -1;
                                break;
                            }
                        }
                    }
                    delay = fn_1_9ABC(action);
                    if (action == 0 || action == 1 || action == 2) {
                        action = 0;
                        lbl_1_bss_84.boards[boardIndex]
                            .scheduledType[lbl_1_bss_84.boards[boardIndex].nextScheduled] = -1;
                    } else if (action == 5) {
                        action = 2;
                        lbl_1_bss_84.boards[boardIndex]
                            .scheduledType[lbl_1_bss_84.boards[boardIndex].nextScheduled] = -1;
                    } else if (action == 3 || action == 4) {
                        action = 1;
                        lbl_1_bss_84.boards[boardIndex]
                            .scheduledType[lbl_1_bss_84.boards[boardIndex].nextScheduled] = -1;
                    }
                    fn_1_3DA0(boardIndex, chosenSpace, action, delay);
                }
                lbl_1_bss_84.boards[boardIndex].nextScheduled++;
            }
            for (queueIndex = 0; queueIndex < lbl_1_bss_84.boards[boardIndex].queueCount;
                 queueIndex++) {
                action = lbl_1_bss_84.boards[boardIndex].queuedType[queueIndex];
                if (action == 5) {
                    scanIndex = 0;
                    availableCount = 0;
                    for (; scanIndex < 7; scanIndex++) {
                        availability[scanIndex] = fn_1_9C24(boardIndex, scanIndex);
                        if (availability[scanIndex] == 0) {
                            availableCount++;
                        }
                    }
                    chosenSpace = 0;
                    if (availableCount > 0) {
                        availableCount = frandmod(availableCount) + 1;
                        for (scanIndex = 0; scanIndex < 7; scanIndex++) {
                            if (availability[scanIndex] == 0) {
                                availableCount--;
                            }
                            if (availableCount == 0) {
                                chosenSpace = scanIndex;
                                break;
                            }
                        }
                    } else {
                        action = -1;
                        availableCount = -1;
                    }
                    if (lbl_1_bss_7DC == 1) {
                        position = ((Point3d (*)[7])lbl_1_data_180)[1][chosenSpace];
                        for (scanIndex = 0; scanIndex < 4; scanIndex++) {
                            if (boardIndex == fn_1_A204(scanIndex) && action != -1 &&
                                (((position.x - 70.0f <= lbl_1_bss_560[scanIndex].x - 40.0f) &&
                                  (lbl_1_bss_560[scanIndex].x - 40.0f <= 70.0f + position.x)) ||
                                 ((position.x - 70.0f <= 40.0f + lbl_1_bss_560[scanIndex].x) &&
                                  (40.0f + lbl_1_bss_560[scanIndex].x <= 70.0f + position.x)))) {
                                action = -1;
                                break;
                            }
                        }
                    }
                    delay = fn_1_9ABC(action);
                    if (action == 0 || action == 1 || action == 2) {
                        action = 0;
                        lbl_1_bss_84.boards[boardIndex].queuedType[queueIndex] = -1;
                    } else if (action == 5) {
                        action = 2;
                        lbl_1_bss_84.boards[boardIndex].queuedType[queueIndex] = -1;
                    } else if (action == 3 || action == 4) {
                        action = 1;
                        lbl_1_bss_84.boards[boardIndex].queuedType[queueIndex] = -1;
                    }
                    fn_1_3DA0(boardIndex, chosenSpace, action, delay);
                }
            }
            for (queueIndex = 0; queueIndex < lbl_1_bss_84.boards[boardIndex].queueCount;
                 queueIndex++) {
                action = lbl_1_bss_84.boards[boardIndex].queuedType[queueIndex];
                if (action == 0 || action == 1 || action == 2) {
                    scanIndex = 0;
                    availableCount = 0;
                    for (; scanIndex < 7; scanIndex++) {
                        availability[scanIndex] = fn_1_9C24(boardIndex, scanIndex);
                        if (availability[scanIndex] == 0) {
                            availableCount++;
                        }
                    }
                    chosenSpace = 0;
                    if (availableCount > 0) {
                        availableCount = frandmod(availableCount) + 1;
                        for (scanIndex = 0; scanIndex < 7; scanIndex++) {
                            if (availability[scanIndex] == 0) {
                                availableCount--;
                            }
                            if (availableCount == 0) {
                                chosenSpace = scanIndex;
                                break;
                            }
                        }
                    } else {
                        action = -1;
                        availableCount = -1;
                    }
                    if (lbl_1_bss_7DC == 1) {
                        position = ((Point3d (*)[7])lbl_1_data_180)[1][chosenSpace];
                        for (scanIndex = 0; scanIndex < 4; scanIndex++) {
                            if (boardIndex == fn_1_A204(scanIndex) && action != -1 &&
                                (((position.x - 70.0f <= lbl_1_bss_560[scanIndex].x - 40.0f) &&
                                  (lbl_1_bss_560[scanIndex].x - 40.0f <= 70.0f + position.x)) ||
                                 ((position.x - 70.0f <= 40.0f + lbl_1_bss_560[scanIndex].x) &&
                                  (40.0f + lbl_1_bss_560[scanIndex].x <= 70.0f + position.x)))) {
                                action = -1;
                                break;
                            }
                        }
                    }
                    delay = fn_1_9ABC(action);
                    if (action == 0 || action == 1 || action == 2) {
                        action = 0;
                        lbl_1_bss_84.boards[boardIndex].queuedType[queueIndex] = -1;
                    } else if (action == 5) {
                        action = 2;
                        lbl_1_bss_84.boards[boardIndex].queuedType[queueIndex] = -1;
                    } else if (action == 3 || action == 4) {
                        action = 1;
                        lbl_1_bss_84.boards[boardIndex].queuedType[queueIndex] = -1;
                    }
                    fn_1_3DA0(boardIndex, chosenSpace, action, delay);
                }
            }
            for (queueIndex = 0; queueIndex < lbl_1_bss_84.boards[boardIndex].queueCount;
                 queueIndex++) {
                action = lbl_1_bss_84.boards[boardIndex].queuedType[queueIndex];
                if (action == 3 || action == 4) {
                    scanIndex = 0;
                    availableCount = 0;
                    for (; scanIndex < 7; scanIndex++) {
                        availability[scanIndex] = fn_1_9C24(boardIndex, scanIndex);
                        if (availability[scanIndex] == 0) {
                            availableCount++;
                        }
                    }
                    chosenSpace = 0;
                    if (availableCount > 0) {
                        availableCount = frandmod(availableCount) + 1;
                        for (scanIndex = 0; scanIndex < 7; scanIndex++) {
                            if (availability[scanIndex] == 0) {
                                availableCount--;
                            }
                            if (availableCount == 0) {
                                chosenSpace = scanIndex;
                                break;
                            }
                        }
                    } else {
                        action = -1;
                        availableCount = -1;
                    }
                    if (lbl_1_bss_7DC == 1) {
                        position = ((Point3d (*)[7])lbl_1_data_180)[1][chosenSpace];
                        for (scanIndex = 0; scanIndex < 4; scanIndex++) {
                            if (boardIndex == fn_1_A204(scanIndex) && action != -1 &&
                                (((position.x - 70.0f <= lbl_1_bss_560[scanIndex].x - 40.0f) &&
                                  (lbl_1_bss_560[scanIndex].x - 40.0f <= 70.0f + position.x)) ||
                                 ((position.x - 70.0f <= 40.0f + lbl_1_bss_560[scanIndex].x) &&
                                  (40.0f + lbl_1_bss_560[scanIndex].x <= 70.0f + position.x)))) {
                                action = -1;
                                break;
                            }
                        }
                    }
                    delay = fn_1_9ABC(action);
                    if (action == 0 || action == 1 || action == 2) {
                        action = 0;
                        lbl_1_bss_84.boards[boardIndex].queuedType[queueIndex] = -1;
                    } else if (action == 5) {
                        action = 2;
                        lbl_1_bss_84.boards[boardIndex].queuedType[queueIndex] = -1;
                    } else if (action == 3 || action == 4) {
                        action = 1;
                        lbl_1_bss_84.boards[boardIndex].queuedType[queueIndex] = -1;
                    }
                    fn_1_3DA0(boardIndex, chosenSpace, action, delay);
                }
            }
        }
        for (queueIndex = 0; queueIndex < 7; queueIndex++) {
            fn_1_4660(boardIndex, queueIndex, 0);
            fn_1_4660(boardIndex, queueIndex, 1);
        }
        fn_1_4660(boardIndex, 0, 2);
    }
    if ((s32) lbl_1_data_240 >= 0) {
        lbl_1_bss_18 += 1;
        if (lbl_1_bss_18 >= 6) {
            lbl_1_data_240 = -1;
            lbl_1_bss_18 = 0;
        }
    } else {
        lbl_1_data_240 = -1;
        lbl_1_bss_18 = 0;
    }
    if ((s32) lbl_1_data_244 >= 0) {
        lbl_1_bss_1A += 1;
        if (lbl_1_bss_1A >= 6) {
            lbl_1_data_244 = -1;
            lbl_1_bss_1A = 0;
        }
    } else {
        lbl_1_data_244 = -1;
        lbl_1_bss_1A = 0;
    }
    if ((s32) lbl_1_data_248 >= 0) {
        lbl_1_bss_1C += 1;
        if (lbl_1_bss_1C >= 6) {
            lbl_1_data_248 = -1;
            lbl_1_bss_1C = 0;
        }
    } else {
        lbl_1_data_248 = -1;
        lbl_1_bss_1C = 0;
    }
}

extern void fn_1_682C(s16 choice);

extern void fn_1_3F64(s16 boardIndex, s16 spaceIndex);

extern s16 lbl_1_bss_1E;

/* Initializes fade-in space states on frame zero and returns when all ordinary spaces are idle. */
u8 fn_1_820C(s16 frameNo) {
    s16 boardIndex;
    s16 spaceIndex;
    u8 hasPendingAction;

    if (lbl_1_bss_7DC == 0) {
        if (frameNo == 0) {
            for (boardIndex = 0; boardIndex < 2; boardIndex++) {
                for (spaceIndex = 0; spaceIndex < 7; spaceIndex++) {
                    lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] = 6;
                    lbl_1_bss_84.boards[boardIndex].spaceTimer[spaceIndex] =
                        M641_SPACE_FADE_TIMER_FRAMES;
                }
            }
        }
    } else if (frameNo == 0) {
        for (boardIndex = 0; boardIndex < 2; boardIndex++) {
            lbl_1_bss_84.boards[boardIndex].spaceState[2] = 6;
            lbl_1_bss_84.boards[boardIndex].spaceTimer[2] =
                M641_SHORT_SPACE_FADE_TIMER_FRAMES;
            lbl_1_bss_84.boards[boardIndex].spaceState[4] = 6;
            lbl_1_bss_84.boards[boardIndex].spaceTimer[4] =
                M641_SHORT_SPACE_FADE_TIMER_FRAMES;
        }
    }
    fn_1_682C(-1);
    hasPendingAction = 0;
    for (boardIndex = 0; boardIndex < 2; boardIndex++) {
        for (spaceIndex = 0; spaceIndex < 7; spaceIndex++) {
            if (lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] != 0) {
                hasPendingAction = 1;
            }
        }
    }
    return (hasPendingAction == 0);
}

/* Tests each ordinary space followed by its item, then the special space. Adjusts movement,
 * records first-hit flags and contact position, and returns 0 for ordinary, 1 for item,
 * 2 for special, or -1 for no hit. */
s16 fn_1_8400(s16 boardIndex, s16 participant, Point3d *targetPosition,
              Point3d *targetDisplacement, f32 targetRadius, f32 targetHeight,
              s16 *collisionKind, Point3d *collisionPosition) {
    s16 spaceIndex;
    Point3d modelPosition;
    Point3d zeroExtent;
    Point3d boardTarget;
    Point3d boardCandidate;
    Point3d *boardCandidatePointer;
    Point3d *boardTargetPointer;
    Point3d itemTarget;
    Point3d itemCandidate;
    Point3d *itemCandidatePointer;
    Point3d *itemTargetPointer;
    Point3d specialTarget;
    Point3d specialCandidate;
    Point3d *specialCandidatePointer;
    Point3d *specialTargetPointer;
    f32 candidateRadius;
    f32 candidateExtent;

    *collisionKind = 0;
    for (spaceIndex = 0; spaceIndex < 7; spaceIndex++) {
        if ((lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] == 2 ||
             lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] == 3 ||
             lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] == 4) &&
            lbl_1_bss_84.boards[boardIndex].spaceFlags[spaceIndex] == 0) {
            Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].spaceModel[spaceIndex],
                            &modelPosition);
            zeroExtent.x = zeroExtent.y = zeroExtent.z = 0.0f;
            candidateRadius = 50.0f;
            candidateExtent = 100.0f;
            if (lbl_1_bss_7DC == 0) {
                modelPosition.y -= candidateExtent;
            }
            boardCandidate = modelPosition;
            boardCandidatePointer = &boardCandidate;
            boardTarget = *targetPosition;
            boardTargetPointer = &boardTarget;
            if (fn_1_540(boardTargetPointer, targetRadius, targetHeight, targetDisplacement,
                         boardCandidatePointer, candidateRadius, candidateExtent,
                         &zeroExtent) != 0) {
                fn_1_3F64(boardIndex, spaceIndex);
                lbl_1_bss_84.boards[boardIndex].spaceFlags[spaceIndex] = 1;
                *collisionKind = 1;
                *collisionPosition = lbl_1_bss_C;
                return 0;
            }
        }
        if ((lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] == 2 ||
             lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] == 3 ||
             lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] == 4) &&
            lbl_1_bss_84.boards[boardIndex].itemFlags[spaceIndex][participant] == 0) {
            Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].itemModel[spaceIndex],
                            &modelPosition);
            zeroExtent.x = zeroExtent.y = zeroExtent.z = 0.0f;
            candidateRadius = 45.0f;
            candidateExtent = 350.0f;
            if (lbl_1_bss_7DC == 0) {
                candidateExtent = 300.0f;
                modelPosition.y -= candidateExtent;
            }
            itemCandidate = modelPosition;
            itemCandidatePointer = &itemCandidate;
            itemTarget = *targetPosition;
            itemTargetPointer = &itemTarget;
            if (fn_1_540(itemTargetPointer, targetRadius, targetHeight, targetDisplacement,
                         itemCandidatePointer, candidateRadius, candidateExtent,
                         &zeroExtent) != 0) {
                fn_1_3F64(boardIndex, spaceIndex);
                lbl_1_bss_84.boards[boardIndex].itemFlags[spaceIndex][participant] = 1;
                *collisionKind = 0;
                *collisionPosition = lbl_1_bss_C;
                return 1;
            }
        }
    }

    if ((lbl_1_bss_84.boards[boardIndex].specialState == 2 ||
         lbl_1_bss_84.boards[boardIndex].specialState == 3 ||
         lbl_1_bss_84.boards[boardIndex].specialState == 4) &&
        lbl_1_bss_84.boards[boardIndex].specialUsed == 0) {
        Hu3DModelPosGet(lbl_1_bss_84.boards[boardIndex].specialModel, &modelPosition);
        zeroExtent.x = zeroExtent.y = zeroExtent.z = 0.0f;
        candidateRadius = 50.0f;
        candidateExtent = 100.0f;
        if (lbl_1_bss_7DC == 0) {
            modelPosition.y -= candidateExtent;
        }
        specialCandidate = modelPosition;
        specialCandidatePointer = &specialCandidate;
        specialTarget = *targetPosition;
        specialTargetPointer = &specialTarget;
        if (fn_1_540(specialTargetPointer, targetRadius, targetHeight, targetDisplacement,
                     specialCandidatePointer, candidateRadius, candidateExtent,
                     &zeroExtent) != 0) {
            fn_1_3F64(boardIndex, lbl_1_bss_84.boards[boardIndex].specialSpace);
            lbl_1_bss_84.boards[boardIndex].specialUsed = 1;
            *collisionKind = 3;
            *collisionPosition = lbl_1_bss_C;
            return 2;
        }
    }
    return -1;
}

/* During finish and pre-winner, starts active animations returning and advances their models. */
void fn_1_8A7C(void) {
    s16 boardIndex;
    s16 spaceIndex;

    for (boardIndex = 0; boardIndex < 2; boardIndex++) {
        for (spaceIndex = 0; spaceIndex < 7; spaceIndex++) {
            if (boardIndex >= 0 && boardIndex < 2 && spaceIndex >= 0 && spaceIndex < 7) {
                if (lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] == 1 ||
                    lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] == 2 ||
                    lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] == 3) {
                    lbl_1_bss_84.boards[boardIndex].spaceTimer[spaceIndex] = 0;
                    lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] = 4;
                }
                if (lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] == 1 ||
                    lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] == 2 ||
                    lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] == 3) {
                    lbl_1_bss_84.boards[boardIndex].itemTimer[spaceIndex] = 0;
                    lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] = 4;
                }
                if (spaceIndex == lbl_1_bss_84.boards[boardIndex].specialSpace &&
                    (lbl_1_bss_84.boards[boardIndex].specialState == 1 ||
                     lbl_1_bss_84.boards[boardIndex].specialState == 2 ||
                     lbl_1_bss_84.boards[boardIndex].specialState == 3)) {
                    lbl_1_bss_84.boards[boardIndex].specialTimer = 0;
                    lbl_1_bss_84.boards[boardIndex].specialState = 4;
                }
            }
            fn_1_4660(boardIndex, spaceIndex, 0);
            fn_1_4660(boardIndex, spaceIndex, 1);
        }
        fn_1_4660(boardIndex, 0, 2);
    }
}

/* Winner and fade-out hooks schedule result appearances using the saved winning group and a local
 * frame count. */
void fn_1_8D9C(s16 winningGroup) {
    s16 spaceIndex;
    s16 activeSpace;
    s16 boardIndex;
    if (lbl_1_bss_7DC == 0)
    {
        if (winningGroup == - 1)
        {
            activeSpace = lbl_1_bss_1E % 30;
            if (activeSpace == 0 && lbl_1_bss_1E < 210)
            {
                activeSpace = lbl_1_bss_1E / 30;
                activeSpace %= 7;
                fn_1_3DA0(0, activeSpace, 0, 20);
                activeSpace = 6 - activeSpace;
                fn_1_3DA0(1, activeSpace, 0, 20);
            }
        }
        else
        {
            if ((activeSpace = lbl_1_bss_1E % 30) == 0 && lbl_1_bss_1E < 210)
            {
                activeSpace = lbl_1_bss_1E / 30;
                activeSpace %= 7;
                fn_1_3DA0(winningGroup, activeSpace, 0, 300);
            }
            if (winningGroup == 0)
            {
                boardIndex = 1;
            }
            else
            {
                boardIndex = 0;
            }
            for (spaceIndex = 0; spaceIndex < 7 && lbl_1_bss_1E == 0; spaceIndex++)
            {
                fn_1_3DA0(boardIndex, spaceIndex, 1, 300);
            }
        }
    }
    else
    {
        if (winningGroup == - 1)
        {
            if ((activeSpace = lbl_1_bss_1E % 30) == 0 && lbl_1_bss_1E < 90)
            {
                activeSpace = lbl_1_bss_1E / 30;
                activeSpace = activeSpace * 2 + 1;
                activeSpace %= 6;
                fn_1_3DA0(0, activeSpace, 0, 20);
                activeSpace = 6 - activeSpace;
                fn_1_3DA0(1, activeSpace, 0, 20);
            }
        }
        else
        {
            if ((activeSpace = lbl_1_bss_1E % 30) == 0 && lbl_1_bss_1E < 90)
            {
                activeSpace = lbl_1_bss_1E / 30;
                activeSpace = activeSpace * 2 + 1;
                activeSpace %= 6;
                fn_1_3DA0(winningGroup, activeSpace, 0, 300);
            }
            if (winningGroup == 0)
            {
                boardIndex = 1;
            }
            else
            {
                boardIndex = 0;
            }
            if (lbl_1_bss_1E == 0)
            {
                fn_1_3DA0(boardIndex, 1, 1, 300);
                fn_1_3DA0(boardIndex, 3, 1, 300);
                fn_1_3DA0(boardIndex, 5, 1, 300);
            }
        }
    }
    for (boardIndex = 0; boardIndex < 2; ++boardIndex)
    {
        for (spaceIndex = 0; spaceIndex < 7; spaceIndex++)
        {
            fn_1_4660(boardIndex, spaceIndex, 0);
            fn_1_4660(boardIndex, spaceIndex, 1);
        }
        fn_1_4660(boardIndex, 0, 2);
    }
    lbl_1_bss_1E++;
    if ((s32) lbl_1_data_240 >= 0) {
        lbl_1_bss_18 += 1;
        if (lbl_1_bss_18 >= 6) {
            lbl_1_data_240 = -1;
            lbl_1_bss_18 = 0;
        }
    } else {
        lbl_1_data_240 = -1;
        lbl_1_bss_18 = 0;
    }
    if ((s32) lbl_1_data_244 >= 0) {
        lbl_1_bss_1A += 1;
        if (lbl_1_bss_1A >= 6) {
            lbl_1_data_244 = -1;
            lbl_1_bss_1A = 0;
        }
    } else {
        lbl_1_data_244 = -1;
        lbl_1_bss_1A = 0;
    }
    if ((s32) lbl_1_data_248 >= 0) {
        lbl_1_bss_1C += 1;
        if (lbl_1_bss_1C >= 6) {
            lbl_1_data_248 = -1;
            lbl_1_bss_1C = 0;
        }
    } else {
        lbl_1_data_248 = -1;
        lbl_1_bss_1C = 0;
    }

}

typedef struct M641MotionInfo {
    s16 motionId;
    s16 padding;
    f32 blendTime;
    u32 attributes;
    s32 voiceEnabled;
} M641MotionInfo;

extern M641MotionInfo lbl_1_data_344[14];

/* Fade-in and main hooks map an action category to its delay for the selected board layout. */
s16 fn_1_9ABC(s16 delayIndex) {
    s16 layoutIndex = 0;

    if (delayIndex < 0) {
        delayIndex = 0;
    }
    if (delayIndex > 5) {
        delayIndex = 5;
    }
    if (lbl_1_bss_7DC == 1) {
        layoutIndex = 1;
    }
    return lbl_1_data_228[delayIndex][layoutIndex];
}

void fn_1_9F7C(s16 playerIndex, s16 motionIndex);

/* The main hook counts scheduled and queued ordinary space actions before the board advances. */
s32 fn_1_9B30(s16 boardIndex) {
    s16 spaceIndex;
    s32 count;

    for (spaceIndex = 0, count = 0; spaceIndex < M641_SCHEDULE_ENTRY_COUNT; spaceIndex++) {
        if ((lbl_1_bss_84.boards[boardIndex].scheduledType[spaceIndex] == 0) ||
            (lbl_1_bss_84.boards[boardIndex].scheduledType[spaceIndex] == 1)) {
            count++;
        }
        if ((lbl_1_bss_84.boards[boardIndex].queuedType[spaceIndex] == 0) ||
            (lbl_1_bss_84.boards[boardIndex].queuedType[spaceIndex] == 1)) {
            count++;
        }
    }
    return count;
}

/* Fade-in and main hooks query whether an ordinary space, item, or special space already has an
 * active action. */
s32 fn_1_9C24(s16 boardIndex, s16 spaceIndex) {
    if ((lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] == 0) &&
        (lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] == 0) &&
        ((lbl_1_bss_84.boards[boardIndex].specialState == 0) ||
         (spaceIndex != lbl_1_bss_84.boards[boardIndex].specialSpace))) {
        return 0;
    }
    return 1;
}

/* The main hook gets a space position and reports the corresponding ordinary (0), item (1), or
 * special (2) space state, or -1 when none applies. */
void fn_1_9CD0(s16 boardIndex, s16 spaceIndex, Point3d *position, s16 *result) {
    s16 tableMode;

    if (lbl_1_bss_7DC == 0) {
        tableMode = 0;
    } else {
        tableMode = 1;
    }
    *position = ((Point3d (*)[7])lbl_1_data_180)[tableMode][spaceIndex];
    if ((lbl_1_bss_84.boards[boardIndex].spaceFlags[spaceIndex] == 0) &&
        ((lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] == 3) ||
         (lbl_1_bss_84.boards[boardIndex].spaceState[spaceIndex] == 4))) {
        *result = 0;
        return;
    }
    if ((lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] == 3) ||
        (lbl_1_bss_84.boards[boardIndex].itemState[spaceIndex] == 4)) {
        *result = 1;
        return;
    }
    if ((spaceIndex == lbl_1_bss_84.boards[boardIndex].specialSpace) &&
        (lbl_1_bss_84.boards[boardIndex].specialUsed == 0) &&
        ((lbl_1_bss_84.boards[boardIndex].specialState == 3) ||
         (lbl_1_bss_84.boards[boardIndex].specialState == 4))) {
        *result = 2;
        return;
    }
    *result = -1;
}

void fn_1_FF1C(s16 playerIndex);

/* Fade-in, finish, main, pre-winner, and start hooks emit the impact burst beside a space, offset
 * toward the active board side. */
void fn_1_9EC8(s16 effectId, Point3d *position) {
    s16 effectType;
    Point3d effectOffset;

    if (effectId == 0) {
        effectType = 1;
    } else {
        effectType = 2;
    }
    if (lbl_1_bss_7DC == 0) {
        effectOffset.x = -80.0f;
        effectOffset.y = effectOffset.z = 0.0f;
    } else {
        effectOffset.x = 80.0f;
        effectOffset.y = effectOffset.z = 0.0f;
    }
    CharEffectHitCreate(effectType, position, &effectOffset);
}

extern void fn_1_B108(void);

extern void fn_1_11AD8(void);

extern s32 lbl_1_data_2A8;

extern s32 lbl_1_data_2DC;

/* Unused motion helper: passes the pad number to character APIs and records the selected motion
 * index. */
static inline void fn_1_9F7C(s16 playerIndex, s16 motionIndex) {
    s16 motionId;

    if ((playerIndex < 0) || (playerIndex >= 4) || (motionIndex < 0) ||
        (motionIndex >= M641_MOTION_INFO_COUNT)) {
        return;
    }
    if (motionIndex != lbl_1_bss_590[playerIndex].currentMotion) {
        motionId = lbl_1_data_344[motionIndex].motionId;
        if (motionId != -1) {
            if (lbl_1_data_344[motionIndex].blendTime <= 1.0f) {
                CharMotionSet(lbl_1_bss_590[playerIndex].padNo,
                              lbl_1_bss_590[playerIndex].motionIds[motionId]);
                Hu3DModelAttrReset(lbl_1_bss_590[playerIndex].modelId,
                                   Hu3DModelMotionAttrGet(lbl_1_bss_590[playerIndex].modelId) |
                                       HU3D_MOTATTR);
                CharModelAttrSet(lbl_1_bss_590[playerIndex].padNo,
                                 lbl_1_data_344[motionIndex].attributes);
            } else {
                Hu3DModelAttrSet(lbl_1_bss_590[playerIndex].modelId,
                                 M641_HU3D_ZWRITE_OFF_FLAGS);
                CharMotionShiftSet(lbl_1_bss_590[playerIndex].padNo,
                                   lbl_1_bss_590[playerIndex].motionIds[motionId],
                                   0.0f,
                                    lbl_1_data_344[motionIndex].blendTime,
                                    lbl_1_data_344[motionIndex].attributes);
            }
        CharModelVoiceFlagSet(lbl_1_bss_590[playerIndex].padNo,
                              lbl_1_data_344[motionIndex].voiceEnabled);
        }
        lbl_1_bss_590[playerIndex].currentMotion = motionIndex;
    }
}

void fn_1_A220(void);

 s32 fn_1_ACE0(s16 frame);

f32 lbl_1_bss_558[2];
M641RouteState lbl_1_bss_84;
Point3d lbl_1_bss_30[7];
s16 lbl_1_bss_24[6];
f32 lbl_1_bss_20;
s16 lbl_1_bss_1E;
s16 lbl_1_bss_1C;
s16 lbl_1_bss_1A;
s16 lbl_1_bss_18;
Point3d lbl_1_bss_C;
Point3d lbl_1_bss_0;
