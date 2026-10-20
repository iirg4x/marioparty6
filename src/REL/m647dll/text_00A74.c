/* Camera, player input and round progression for the minigame. */
#include "REL/m647dll/module_types.h"
#include "game/flag.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm_se.h"
#include "msm_stream.h"

#define M647_RESOURCE_GROUP 268435456
#define MSM_SE_M647_PLAYER_ENTRY 1992
#define MSM_SE_M647_OBSTACLE_START 1993
#define MSM_SE_M647_PLAYER_FALL 1994
#define MSM_SE_M647_MOUNT_START 1995
#define MSM_SE_M647_MOUNT_LAND 1996
#define MSM_SE_M647_LAUNCH_SEQUENCE 1997
#define MSM_SE_M647_LAUNCH_CUE 1998
#define MSM_SE_M647_RACE_START 2011

Point3d lbl_1_data_60 = { 500.0f, 2000.0f, 400.0f };
Point3d lbl_1_data_6C = { -5.0f, -20.0f, -4.0f };
GXColor lbl_1_data_78 = { 192, 192, 192, 192 };
f32 lbl_1_data_7C[3] = { 900.0f, 240.0f, -200.0f };
s32 lbl_1_data_88[2] = { DATANUM(DATA_m647, 0), DATANUM(DATA_m647, 1) };
s32 lbl_1_data_90[2] = { DATANUM(DATA_m647, 4), DATANUM(DATA_m647, 5) };
s32 lbl_1_data_98[2] = { DATANUM(DATA_m647, 6), DATANUM(DATA_m647, 7) };
s32 lbl_1_data_A0[2] = { DATANUM(DATA_m647, 8), DATANUM(DATA_m647, 9) };
s32 lbl_1_data_A8[2] = { DATANUM(DATA_m647, 2), DATANUM(DATA_m647, 3) };
s32 lbl_1_data_B0[2] = { DATANUM(DATA_m647, 11), DATANUM(DATA_m647, 10) };
s32 lbl_1_data_B8[2] = { DATANUM(DATA_m647, 13), DATANUM(DATA_m647, 12) };
u32 lbl_1_data_C0[] = {
    MESSNUM(MESS_MESS_647, 0), MESSNUM(MESS_MESS_647, 1),
    MESSNUM(MESS_MESS_647, 2), MESSNUM(MESS_MESS_647, 3)
};
u32 lbl_1_data_D0[9][2] = {
    DATANUM(DATA_mariomot, 0), 1, DATANUM(DATA_mariomot, 1), 1,
    DATANUM(DATA_mariomot, 2), 1, DATANUM(DATA_mariomot, 3), 0,
    DATANUM(DATA_mariomot, 4), 0, DATANUM(DATA_mariomot, 55), 1,
    DATANUM(DATA_mariomot, 39), 0, DATANUM(DATA_mariomot, 40), 0,
    DATANUM(DATA_mariomot, 36), 0
};
HuVecF lbl_1_data_118[2] = { -975.0f, 300.0f, 200.0f, 0.0f, 300.0f, -2200.0f };
HuVecF lbl_1_data_130[2] = { -31.0f, 53.0f, 0.0f, -10.0f, 0.0f, 0.0f };
f32 lbl_1_data_148[2] = { 2500.0f, 2700.0f };
f32 lbl_1_data_150[4] = { -450.0f, -150.0f, 150.0f, 450.0f };
Point3d lbl_1_data_160[] = {
    0.0f, 420.0f, -280.0f, -170.0f, 340.0f, -280.0f, 170.0f, 315.0f, -280.0f
};
Point3d lbl_1_data_184[] = {
    -400.0f, 200.0f, -280.0f, 400.0f, 200.0f, -280.0f, -170.0f, 200.0f,
    -100.0f, 170.0f, 200.0f, -100.0f
};
Point3d lbl_1_data_1B4[] = {
    -200.0f, 200.0f, -100.0f, 0.0f, 200.0f, -100.0f, 200.0f, 200.0f,
    -100.0f, 400.0f, 200.0f, -100.0f
};
s16 lbl_1_data_1E4[24][4] = {
    0, 1, 2, 3, 0, 1, 3, 2, 0, 2, 1, 3, 0, 2, 3, 1,
    0, 3, 1, 2, 0, 3, 2, 1, 1, 0, 2, 3, 1, 0, 3, 2,
    1, 2, 0, 3, 1, 2, 3, 0, 1, 3, 0, 2, 1, 3, 2, 0,
    2, 0, 1, 3, 2, 0, 3, 1, 2, 1, 0, 3, 2, 1, 3, 0,
    2, 3, 0, 1, 2, 3, 1, 0, 3, 0, 1, 2, 3, 0, 2, 1,
    3, 1, 0, 2, 3, 1, 2, 0, 3, 2, 0, 1, 3, 2, 1, 0
};
Point3d lbl_1_data_2A4[4] = {
    -1580.0f, 0.0f, 475.0f, -1580.0f, 0.0f, 825.0f, -1580.0f, 0.0f, 1175.0f, -1580.0f, 0.0f, 1525.0f
};
Point3d lbl_1_data_2D4[4] = {
    -1130.0f, 200.0f, 475.0f, -1220.0f, 220.0f, 825.0f, -1260.0f, 190.0f,
    1175.0f, -1110.0f, 210.0f, 1525.0f
};

typedef struct M647ActivityCallbacks_s {
    void (*entries[4])(s16 index); /* Activity update callback by activity index. */
} M647ActivityCallbacks;

typedef struct M647SoundTable_s {
    u32 soundIds[14]; /* Finish sound for each character ID. */
} M647SoundTable;

typedef struct M647TimerThresholds_s {
    s16 values[4]; /* CPU selection wait by difficulty. */
} M647TimerThresholds;

extern s16 GwMgNightF;

void fn_1_62FC(s32 selection);
void fn_1_6414(void);
void fn_1_A94(void);
void fn_1_DA8(void);
extern f32 fn_1_A790(void);
extern f32 fn_1_A854(void);
s16 fn_1_8648(void);
extern s16 fn_1_A518(s16 index);
void fn_1_15A4(s16 layerNo);
void fn_1_17C4(s16 layerNo);
s16 fn_1_62B8(void);
void fn_1_4204(void);
void fn_1_5430(void);
void fn_1_57C0(void);
void fn_1_5978(void);
void fn_1_5F08(void);
void fn_1_6484(s16 value);
void fn_1_64A4(void);
extern void fn_1_6ED0(s16 entryIndex, f32 speed);
extern void fn_1_AC40(s16 entryIndex);
void fn_1_B04(void);
void fn_1_B50(void);
void fn_1_C4C(void);
void fn_1_1018(HUPROCESS *process);
f32 fn_1_12D8(void);
void fn_1_185C(void);
void fn_1_1908(void);
void fn_1_1978(f32 horizontalPosition);
void fn_1_1C04(f32 activityX);
void fn_1_1C9C(void);
void fn_1_1EB8(void);
void fn_1_1FAC(void);
void fn_1_2B84(s16 recordIndex, s16 motionIndex);
void fn_1_2C2C(s16 playerIndex, Point3d *movement);
void fn_1_32F4(s16 recordIndex);
s32 fn_1_34A0(s16 activityIndex);
void fn_1_353C(void);
void fn_1_366C(s16 recordIndex);
void fn_1_370C(s16 recordIndex);
s16 fn_1_3AB4(s16 startIndex, s16 direction);
void fn_1_37C4(s16 recordIndex);
void fn_1_14B0(void);
void fn_1_15F4(void);
void fn_1_1814(void);
void fn_1_2094(void);
void fn_1_28C8(void);
s16 fn_1_2B30(s16 index);
s16 fn_1_2B4C(s16 index);
s16 fn_1_2B68(s16 index);
void fn_1_2CD0(void);
void fn_1_341C(s16 recordIndex);
s16 fn_1_3468(s16 index);
s16 fn_1_3484(s16 index);
void fn_1_381C(void);
s16 fn_1_3A44(void);
void fn_1_3B54(M647PlayerRecord *input, Point3d *position);
void fn_1_A74(OMOBJ *object);
void fn_1_B108(void);
void fn_1_8A40(s16 index);
void fn_1_98CC(s16 index);
void fn_1_7230(s16 playerIndex);
void fn_1_7D68(s16 playerIndex);

/* Updates celebration particles each frame through the object manager callback. */
void fn_1_A74(OMOBJ *object) { fn_1_B108(); }

/* Runs the request loop started by fn_1_B04 and fn_1_B50 for message windows. */
void fn_1_A94(void) {
    for (;;) {
        if (lbl_1_bss_14.mode == 1) {
            fn_1_62FC(lbl_1_bss_14.argument);
            lbl_1_bss_14.mode = 0;
        }
        if (lbl_1_bss_14.mode == 2) {
            fn_1_6414();
            lbl_1_bss_14.mode = 0;
        }
        HuPrcVSleep();
    }
}

/* Starts the background message-window request process. */
void fn_1_B04(void) {
    lbl_1_bss_14.mode = 0;
    HuPrcChildCreate(fn_1_A94, 100, 8192, 0, HuPrcCurrentGet());
}

/* Resets per-round state and starts the shared request process at game start. */
void fn_1_B50(void) {
    s32 index;
    s16 nightMode;

    lbl_1_bss_14.mode = 0;
    HuPrcChildCreate(fn_1_A94, 100, 8192, 0, HuPrcCurrentGet());
    memset(&lbl_1_bss_174, 0, 52);
    fn_1_DA8();
    for (index = 0; index < 4; index++) {
        memset(&lbl_1_bss_54[index], 0, 72);
        lbl_1_bss_54[index].cpuDifficulty = -1;
        lbl_1_bss_22[index] = -1;
    }
    nightMode = GwMgNightF;
    lbl_1_bss_174.nightMode = nightMode;
    lbl_1_bss_174.windowId = -1;
}

/* Chooses the players' activity pattern; pad 0 X, Y, L or R forces pattern 0, 6, 12 or 18,
 * in that priority order. */
void fn_1_C4C(void) {
    M647ActivityCallbacks activityCallbacks = {{fn_1_8A40, fn_1_98CC, fn_1_7230, fn_1_7D68}};
    s32 recordIndex;
    s16 patternIndex;

    patternIndex = (s16)frandmod(24);

    if (HuPadBtn[0] & PAD_BUTTON_X) {
        patternIndex = 0;
    } else if (HuPadBtn[0] & PAD_BUTTON_Y) {
        patternIndex = 6;
    } else if (HuPadBtn[0] & (1 << 14)) {
        patternIndex = 12;
    } else if (HuPadBtn[0] & (1 << 13)) {
        patternIndex = 18;
    }

    lbl_1_bss_174.activityPattern = patternIndex;
    for (recordIndex = 0; recordIndex < 4; recordIndex++) {
        s16 choice = lbl_1_data_1E4[patternIndex][recordIndex];
        lbl_1_bss_54[recordIndex].activityIndex = choice;
        lbl_1_bss_54[recordIndex].callback = activityCallbacks.entries[choice];
    }
}

/* Builds the player order from board ranking, or shuffles it in special modes. */
void fn_1_DA8(void) {
    s16 rank[4];
    s16 playerOrder[4];
    s32 playerIndex;
    s16 swappedValue;
    s32 randomIndex;
    s32 otherIndex;

    if (_CheckFlag(FLAG_INST_MG_MODE) != 0 || _CheckFlag(FLAG_BOARD_INIT) != 0) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            playerOrder[playerIndex] = playerIndex;
        }
        for (playerIndex = 0; playerIndex < 8; playerIndex++) {
            otherIndex = frandmod(4);
            randomIndex = (otherIndex + 1 + frandmod(3)) % 4;
            swappedValue = playerOrder[otherIndex];
            playerOrder[otherIndex] = playerOrder[randomIndex];
            playerOrder[randomIndex] = swappedValue;
        }
    } else {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            rank[playerIndex] = mbPlayerRankGet(playerIndex);
            playerOrder[playerIndex] = playerIndex;
        }
        for (playerIndex = 0; playerIndex < 3; playerIndex++) {
            for (otherIndex = playerIndex + 1; otherIndex < 4; otherIndex++) {
                if (playerIndex != otherIndex &&
                    (rank[playerIndex] < rank[otherIndex] ||
                     (rank[playerIndex] == rank[otherIndex] &&
                      playerOrder[playerIndex] > playerOrder[otherIndex]))) {
                    swappedValue = rank[playerIndex];
                    rank[playerIndex] = rank[otherIndex];
                    rank[otherIndex] = swappedValue;
                    swappedValue = playerOrder[playerIndex];
                    playerOrder[playerIndex] = playerOrder[otherIndex];
                    playerOrder[otherIndex] = swappedValue;
                    playerIndex = 0;
                }
            }
        }
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        lbl_1_bss_1A[playerIndex] = playerOrder[playerIndex];
    }
}

/* Configures the two Insectiride cameras and their initial views during setup. */
void fn_1_1018(HUPROCESS *process) {
    s32 cameraIndex;
    OMOBJ *viewportObject;

    Hu3DCameraCreate(1);
    Hu3DCameraViewportSet(1, (0.0f), (0.0f), (640.0f),
                          (480.0f), (0.0f), (1.0f));
    Hu3DCameraPerspectiveSet(1, (20.0f), (60.0f), (25000.0f),
                             (1.2f));
    Hu3DCameraScissorSet(1, 0, 0, 640, 480);
    Hu3DCameraCreate(2);
    Hu3DCameraViewportSet(2, (0.0f), (0.0f), (640.0f),
                          (480.0f), (0.0f), (1.0f));
    Hu3DCameraPerspectiveSet(2, (20.0f), (60.0f), (25000.0f),
                             (1.2f));
    viewportObject = omAddObjEx(process, 32730, 0, 0, -1, omOutViewMulti);
    viewportObject->work[0] = 2;
    for (cameraIndex = 0; cameraIndex < 2; cameraIndex++) {
        CenterM[cameraIndex].x = lbl_1_data_118[cameraIndex].x;
        CenterM[cameraIndex].y = lbl_1_data_118[cameraIndex].y;
        CenterM[cameraIndex].z = lbl_1_data_118[cameraIndex].z;
        CRotM[cameraIndex].x = lbl_1_data_130[cameraIndex].x;
        CRotM[cameraIndex].y = lbl_1_data_130[cameraIndex].y;
        CRotM[cameraIndex].z = lbl_1_data_130[cameraIndex].z;
        CZoomM[cameraIndex] = lbl_1_data_148[cameraIndex];
    }
}

/* Reframes camera 1 around the four activity entries' horizontal span. */
f32 fn_1_12D8(void) {
    f32 maximumX;
    f32 minimumX;

    maximumX = fn_1_A790();
    minimumX = fn_1_A854();
    lbl_1_bss_2C[0] = 100.0f +
        (minimumX + (maximumX - minimumX) / 2);
    if (lbl_1_bss_2C[0] < -800.0f) {
        lbl_1_bss_2C[0] = -800.0f;
    }
    lbl_1_bss_2C[1] = 4.5 * fabs(maximumX - minimumX);
    if (lbl_1_bss_2C[1] < 5500.0f) {
        lbl_1_bss_2C[1] = 5500.0f;
    }
    if (lbl_1_bss_2C[1] > 8500.0f) {
        lbl_1_bss_2C[1] = 8500.0f;
    }
    {
        OM_CAMERA_VIEW view;

        view.center.x = lbl_1_bss_2C[0];
        view.center.y = 150.0f;
        view.center.z = 535.0f;
        view.rot.x = -28.0f;
        view.rot.y = view.rot.z = 0.0f;
        view.zoom = lbl_1_bss_2C[1];
    omCameraViewMoveSimpleMulti(2, &view, 60);
    }
    return maximumX;
}

/* Allocates the full-screen capture texture and installs its camera-layer hook. */
void fn_1_14B0(void) {
    u32 textureSize;

    textureSize = GXGetTexBufferSize(640, 480, 6, 0, 0);
    lbl_1_bss_8 = HuSprAnimMake(640, 480, 0);
    lbl_1_bss_8->bmp->palData = NULL;
    lbl_1_bss_8->bmp->palNum = 0;
    lbl_1_bss_8->bmp->data =
        HuMemDirectMallocNum(HEAP_MODEL, (s32) textureSize, M647_RESOURCE_GROUP);
    lbl_1_bss_174.animId = Hu3DAnimCreate(lbl_1_bss_8, lbl_1_bss_174.modelId, "screen");
    Hu3DCameraLayerHookSet(1, 8, fn_1_15A4);
}

/* Copies the rendered scene for the full-screen overlay, then clears its depth. */
void fn_1_15A4(s16 layerNo) {
    Hu3DFbCopyExec(0, 0, 640, 480, GX_TF_RGBA8, 0, lbl_1_bss_8->bmp->data);
    Hu3DZClear();
}

/* Replaces the full-screen capture with a small view during the transition. */
void fn_1_15F4(void) {
    u32 textureSize;

    Hu3DAnimKill(lbl_1_bss_174.animId);
    Hu3DCameraLayerHookReset(1, 8);
    HuSprAnimKill(lbl_1_bss_8);
    lbl_1_bss_8 = NULL;
    textureSize = GXGetTexBufferSize(144, 96, 6, 0, 0);
    lbl_1_bss_8 = HuSprAnimMake(144, 96, 0);
    lbl_1_bss_8->bmp->palData = NULL;
    lbl_1_bss_8->bmp->palNum = 0;
    lbl_1_bss_8->bmp->data =
        HuMemDirectMallocNum(HEAP_MODEL, (s32) textureSize, M647_RESOURCE_GROUP);
    lbl_1_bss_174.animId = Hu3DAnimCreate(lbl_1_bss_8, lbl_1_bss_174.modelId, "screen");
    Hu3DCameraLayerHookSet(1, 8, fn_1_17C4);
    Hu3DCameraViewportSet(1, (0.0f), (0.0f), (144.0f), (96.0f), (0.0f), (1.0f));
    Hu3DCameraPerspectiveSet(1, (20.0f), (60.0f), (25000.0f), (1.2f));
    Hu3DCameraScissorSet(1, 0, 0, 144, 96);
}

/* Captures the small transition view before clearing its depth buffer. */
void fn_1_17C4(s16 layerNo) {
    Hu3DFbCopyExec(0, 0, 144, 96, GX_TF_RGBA8, 0, lbl_1_bss_8->bmp->data);
    Hu3DZClear();
}

/* Releases the transition capture resources after the camera effect ends. */
void fn_1_1814(void) {
    Hu3DAnimKill(lbl_1_bss_174.animId);
    HuSprAnimKill(lbl_1_bss_8);
    Hu3DCameraLayerHookReset(1, 8);
}

/* Starts the obstacle sound and motion range 41-47 during slot selection, hiding the shared
 * activity models. */
void fn_1_185C(void) {
    s32 modelIndex;

    lbl_1_bss_10 = HuAudFXPlay(MSM_SE_M647_OBSTACLE_START);
    Hu3DMotionTimeSet(lbl_1_bss_174.modelId, (41.0f));
    Hu3DMotionStartEndSet(lbl_1_bss_174.modelId, (41.0f), (47.0f));
    for (modelIndex = 0; modelIndex < 4; modelIndex++) {
        Hu3DModelAttrSet(lbl_1_bss_174.modelIds[modelIndex], 1U);
    }
}

/* Stops the obstacle sound and returns its model motion to the resting range. */
void fn_1_1908(void) {
    HuAudFXStop(lbl_1_bss_10);
    Hu3DMotionTimeSet(lbl_1_bss_174.modelId, (33.0f));
    Hu3DMotionStartEndSet(lbl_1_bss_174.modelId, (33.0f), (39.0f));
}

/* Updates shared activity model visibility by the player's X position and switches the obstacle
 * motion; starting that motion hides all four shared models for this update. */
void fn_1_1978(f32 horizontalPosition) {
    s32 index;

    for (index = 0; index < 4; index++) {
        if (horizontalPosition < (140.0f) + lbl_1_data_150[index] &&
            horizontalPosition > lbl_1_data_150[index] - (140.0f)) {
            Hu3DModelAttrReset(lbl_1_bss_174.modelIds[3 - index], 1);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_174.modelIds[3 - index], 1);
        }
    }
    for (index = 0; index < 4; index++) {
        if (horizontalPosition < (100.0f) + lbl_1_data_150[index] &&
            horizontalPosition > lbl_1_data_150[index] - (100.0f)) {
            break;
        }
    }
    if (index == 4) {
        if (lbl_1_bss_174.obstacleActive == 0) {
            fn_1_185C();
        }
        lbl_1_bss_174.obstacleActive = 1;
        return;
    }
    if (lbl_1_bss_174.obstacleActive == 1) {
        fn_1_1908();
    }
    lbl_1_bss_174.obstacleActive = 0;
}

/* Moves camera 0 to the supplied activity X coordinate during race updates. */
void fn_1_1C04(f32 activityX) {
    OM_CAMERA_VIEW view;

    view.center.x = activityX;
    view.center.y = (300.0f);
    view.center.z = (1000.0f);
    view.rot.x = (-40.0f);
    view.rot.y = (65.0f);
    view.rot.z = (0.0f);
    view.zoom = (3700.0f);
    omCameraViewMoveSimpleMulti(1, &view, 30);
}

/* Advances the opening camera through its staged views during the intro. */
void fn_1_1C9C(void) {
    switch (lbl_1_bss_34.phase) {
    case 0:
        if (lbl_1_bss_34.frame++ >= 0) {
            lbl_1_bss_34.frame = 0;
            lbl_1_bss_34.phase++;
        }
        break;
    case 1:
        lbl_1_bss_34.view.center.x = (-975.0f);
        lbl_1_bss_34.view.center.y = (300.0f);
        lbl_1_bss_34.view.center.z = (1575.0f);
        lbl_1_bss_34.view.rot.x = (-31.0f);
        lbl_1_bss_34.view.rot.y = (53.0f);
        lbl_1_bss_34.view.rot.z = (0.0f);
        lbl_1_bss_34.view.zoom = (2500.0f);
        omCameraViewMoveSimpleMulti(1, &lbl_1_bss_34.view, 240);
        lbl_1_bss_34.phase++;
        break;
    case 2:
        if (lbl_1_bss_174.sequencePhase < 2) {
            break;
        }
        lbl_1_bss_34.view.center.y = (200.0f);
        lbl_1_bss_34.view.center.z = (1000.0f);
        lbl_1_bss_34.view.rot.x = (-45.0f);
        lbl_1_bss_34.view.rot.y = (-45.0f);
        lbl_1_bss_34.view.zoom = (3000.0f);
        omCameraViewMoveSimpleMulti(1, &lbl_1_bss_34.view, 300);
        lbl_1_bss_34.phase++;
        break;
    }
}

/* Orbits camera 0 around the shared activity display once the first player reaches input phase
 * 5. */
void fn_1_1EB8(void) {
    if (lbl_1_bss_54[lbl_1_bss_1A[0]].inputPhase > 4) {
        CenterM[0].x = (7000.0f);
        CenterM[0].y = (80.0f);
        CenterM[0].z = (0.0f);
        CRotM[0].x = (-30.0f);
        CRotM[0].y = lbl_1_bss_34.view.rot.y;
        CRotM[0].z = (0.0f);
        CZoomM[0] = (1300.0f);
        lbl_1_bss_34.view.rot.y += (1.0f);
    }
}

/* Adjusts the results camera while at least one player has finished. */
void fn_1_1FAC(void) {
    if (lbl_1_bss_18 != 0) {
        CenterM->x = (1160.0f);
        CenterM->y = (150.0f);
        CenterM->z = (1000.0f);
        CRotM->x = (-40.0f);
        CRotM->y = lbl_1_bss_C;
        CRotM->z = (0.0f);
        CZoomM[0] = (1500.0f);
        lbl_1_bss_C += (0.5f);
    }
}

/* Sets up scene lighting and shadows and loads the stage models. */
void fn_1_2094(void) {
    Point3d shadowPosition;
    Point3d shadowTarget;
    Point3d shadowDirection;
    HU3D_MODELID modelId;
    s16 lightId;

    lightId = Hu3DGLightCreateV(&lbl_1_data_60, &lbl_1_data_6C, &lbl_1_data_78);
    Hu3DGLightStaticSet(lightId, 1);
    Hu3DGLightInfinitytSet(lightId);
    Hu3DShadowCreate(30.0f, 20.0f, 13000.0f);
    shadowPosition.x = -500.0f;
    shadowPosition.y = 4000.0f;
    shadowPosition.z = 4000.0f;
    shadowDirection.y = 1.0f;
    shadowDirection.x = shadowDirection.z = 0.0f;
    shadowTarget.x = shadowTarget.y = shadowTarget.z = 0.0f;
    Hu3DShadowPosSet(&shadowPosition, &shadowDirection, &shadowTarget);

    modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
        lbl_1_data_88[lbl_1_bss_174.nightMode],
        M647_RESOURCE_GROUP, HEAP_MODEL));
    Hu3DModelCameraSet(modelId, 3);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    Hu3DModelShadowMapSet(modelId);
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_LOOP);
    if (lbl_1_bss_174.nightMode == 0) {
        Hu3DModelShadowMapTPLvlSet(modelId, 0.7f);
    } else {
        Hu3DModelShadowMapTPLvlSet(modelId, 0.5f);
    }
    lbl_1_bss_174.backgroundModelId = modelId;

    modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
        lbl_1_data_98[lbl_1_bss_174.nightMode],
        M647_RESOURCE_GROUP, HEAP_MODEL));
    Hu3DModelCameraSet(modelId, 3);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    Hu3DModelShadowMapSet(modelId);
    lbl_1_bss_174.openingModelId = modelId;

    modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
        lbl_1_data_A0[lbl_1_bss_174.nightMode],
        M647_RESOURCE_GROUP, HEAP_MODEL));
    Hu3DModelCameraSet(modelId, 3);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    Hu3DMotionSpeedSet(modelId, 0.0f);
    lbl_1_bss_174.motionModelId = modelId;

    modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
        lbl_1_data_A8[lbl_1_bss_174.nightMode],
        M647_RESOURCE_GROUP, HEAP_MODEL));
    Hu3DModelCameraSet(modelId, 3);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    Hu3DMotionSpeedSet(modelId, 0.0f);
    lbl_1_bss_174.entranceModelId = modelId;

    modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
        lbl_1_data_B0[lbl_1_bss_174.nightMode],
        M647_RESOURCE_GROUP, HEAP_MODEL));
    Hu3DModelCameraSet(modelId, 3);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    lbl_1_bss_174.effectModelId = modelId;

    modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
        lbl_1_data_90[lbl_1_bss_174.nightMode],
        M647_RESOURCE_GROUP, HEAP_MODEL));
    Hu3DModelCameraSet(modelId, 3);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    Hu3DMotionSpeedSet(modelId, 0.0f);
    lbl_1_bss_174.modelId = modelId;

    modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
        lbl_1_data_B8[lbl_1_bss_174.nightMode],
        M647_RESOURCE_GROUP, HEAP_MODEL));
    Hu3DModelCameraSet(modelId, 3);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    Hu3DModelShadowMapSet(modelId);
    Hu3DModelAttrSet(modelId, HU3D_ATTR_DISPOFF);
    lbl_1_bss_174.gameplayModelId = modelId;

    modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_m647, 14), M647_RESOURCE_GROUP, HEAP_MODEL));
    Hu3DModelCameraSet(modelId, 0);
    Hu3DModelPosSet(modelId, 7000.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelShadowMapSet(modelId);
    Hu3DModelScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    lbl_1_bss_174.shadowModelId = modelId;
}

/* Creates the four player models and their motions before the round starts. */
void fn_1_28C8(void) {
    s32 playerIndex;
    s32 motionIndex;
    HU3D_MODELID modelId;
    s16 characterId;
    HU3D_MOTIONID motionId;
    s16 padNo;

    lbl_1_bss_18 = 0;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        lbl_1_bss_54[playerIndex].motionState = -1;
        lbl_1_bss_54[playerIndex].playerIndex = (s16)playerIndex;
        lbl_1_bss_54[playerIndex].finishPlace = 4;
        characterId = GwPlayerConf[playerIndex].charNo;
        padNo = GwPlayerConf[playerIndex].padNo;
        modelId = CharModelCreate(characterId, 8);
        Hu3DModelPosSetV(modelId, (HuVecF *)(void *)lbl_1_data_7C);
        Hu3DModelRotSet(modelId, (0.0f), (270.0f), (0.0f));
        for (motionIndex = 0; motionIndex < 9; motionIndex++) {
            motionId = CharMotionCreate(characterId, lbl_1_data_D0[motionIndex][0]);
            lbl_1_bss_54[playerIndex].motionIds[motionIndex] = motionId;
        }
        CharMotionShiftSet(characterId, lbl_1_bss_54[playerIndex].motionIds[0],
                           (1.0f), (0.0f), lbl_1_data_D0[0][1]);
        lbl_1_bss_54[playerIndex].modelId = modelId;
        lbl_1_bss_54[playerIndex].characterId = characterId;
        lbl_1_bss_54[playerIndex].padIndex = padNo;
        lbl_1_bss_54[playerIndex].positionX = lbl_1_data_7C[0];
        lbl_1_bss_54[playerIndex].rotationY = (270.0f);
        if (GwPlayerConf[playerIndex].type == 1) {
            lbl_1_bss_54[playerIndex].cpuDifficulty = GwPlayerConf[playerIndex].comDif;
        }
        Hu3DModelCameraSet(modelId, 2);
    }
}

/* Returns the configured CPU difficulty, or -1 for a human player, for activity input. */
s16 fn_1_2B30(s16 index) {
    return lbl_1_bss_54[index].cpuDifficulty;
}

/* Returns the player's controller port for activity input. */
s16 fn_1_2B4C(s16 index) {
    return lbl_1_bss_54[index].padIndex;
}

/* Returns the character ID stored for the player record. */
s16 fn_1_2B68(s16 index) {
    return lbl_1_bss_54[index].characterId;
}

/* Crossfades one player to a requested motion when the game changes their action. */
void fn_1_2B84(s16 recordIndex, s16 motionIndex) {
    M647PlayerRecord *record;

    record = &lbl_1_bss_54[recordIndex];
    if (record->motionState != motionIndex) {
        CharMotionShiftSet(record->characterId,
                           record->motionIds[motionIndex],
                           (0.0f), (8.0f),
                           lbl_1_data_D0[motionIndex][1]);
        record->motionState = motionIndex;
    }
}

/* Offsets a player's model position when an activity applies a movement vector. */
void fn_1_2C2C(s16 playerIndex, Point3d *movement) {
    Point3d modelPosition;

    Hu3DModelPosGet(lbl_1_bss_54[playerIndex].modelId, &modelPosition);
    modelPosition.x = modelPosition.x + movement->x;
    modelPosition.y += movement->y;
    modelPosition.z += movement->z;
    Hu3DModelPosSetV(lbl_1_bss_54[playerIndex].modelId, &modelPosition);
}

/* Advances the scene through the active minigame sequence phases. */
void fn_1_2CD0(void) {
    f32 maximumActivityX;
    s16 index;

    switch (lbl_1_bss_174.sequencePhase) {
    case 0:
        fn_1_4204();
        fn_1_1EB8();
        break;
    case 1:
        fn_1_5430();
        fn_1_1EB8();
        break;
    case 2:
        fn_1_57C0();
        fn_1_1EB8();
        break;
    case 3:
        fn_1_5978();
        return;
    case 4:
        fn_1_5F08();
        return;
    case 5:
        for (index = 0; index < 4; index++) {
            lbl_1_bss_54[index].callback(index);
        }
        maximumActivityX = fn_1_12D8();
        fn_1_1C04(maximumActivityX);
        if (fn_1_62B8() != 0) {
            MgSeqModeNext();
        }
        break;
    }
}

/* Records a player's finish order and plays the character's finish sound. */
void fn_1_32F4(s16 recordIndex) {
    M647SoundTable soundTable = { { 579, 555, 699, 795, 747, 483, 771, 507, 723, 651, 531, 675, 627,
                                    603 } };

    if (lbl_1_bss_54[recordIndex].finishPlace >= 4) {
        lbl_1_bss_54[recordIndex].finishPlace = lbl_1_bss_18;
        HuAudFXPlay(soundTable.soundIds[lbl_1_bss_54[recordIndex].characterId]);
        lbl_1_bss_18++;
    }
}

/* Runs one player's activity callback during finish and pre-winner updates. */
void fn_1_341C(s16 recordIndex) {
    lbl_1_bss_54[recordIndex].callback(recordIndex);
}

/* Returns a player's finish place when the result scene lays out the standings. */
s16 fn_1_3468(s16 index) {
    return lbl_1_bss_54[index].finishPlace;
}

/* Returns the activity slot assigned to the player record. */
s16 fn_1_3484(s16 index) {
    return lbl_1_bss_54[index].activityIndex;
}

/* Finds the player assigned to an activity, logging the match; returns player record 0 if none
 * matches. */
s32 fn_1_34A0(s16 activityIndex) {
    s32 recordIndex;

    for (recordIndex = 0; (s16)recordIndex < 4; recordIndex++) {
        if (activityIndex == lbl_1_bss_54[(s16)recordIndex].activityIndex) {
            OSReport("%d\n", lbl_1_bss_54[(s16)recordIndex].activityIndex);
            return recordIndex;
        }
    }
    return 0;
}

/* Gives placed players their result motions after all activities finish. */
void fn_1_353C(void) {
    s32 index;
    M647PlayerRecord *winnerRecord;
    M647PlayerRecord *nonPodiumRecord;

    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_54[index].finishPlace == 0) {
            winnerRecord = &lbl_1_bss_54[(s16)index];
            if (winnerRecord->motionState != 6) {
                CharMotionShiftSet(winnerRecord->characterId, winnerRecord->motionIds[6],
                                   (0.0f), (8.0f),
                                   lbl_1_data_D0[6][1]);
                winnerRecord->motionState = 6;
            }
        }
        if (lbl_1_bss_54[index].finishPlace >= 3) {
            nonPodiumRecord = &lbl_1_bss_54[(s16)index];
            if (nonPodiumRecord->motionState != 7) {
                CharMotionShiftSet(nonPodiumRecord->characterId, nonPodiumRecord->motionIds[7],
                                   (0.0f), (8.0f),
                                   lbl_1_data_D0[7][1]);
                nonPodiumRecord->motionState = 7;
            }
        }
    }
}

/* Hooks a player's model to the grid activity model's itemhook and zeroes its local position. */
void fn_1_366C(s16 recordIndex) {
    HU3D_MODELID hookModelId;

    hookModelId = fn_1_8648();
    Hu3DModelHookSet(hookModelId, "itemhook",
                     lbl_1_bss_54[recordIndex].modelId);
    Hu3DModelPosSet(lbl_1_bss_54[recordIndex].modelId,
                    (0.0f), (0.0f), (0.0f));
}

/* Hooks a player's model beneath the model for their selected activity. */
void fn_1_370C(s16 recordIndex) {
    HU3D_MODELID hookModelId;

    hookModelId = fn_1_A518(lbl_1_bss_54[recordIndex].activityIndex);
    Hu3DModelHookSet(hookModelId, "itemhook",
                     lbl_1_bss_54[recordIndex].modelId);
    Hu3DModelPosSet(lbl_1_bss_54[recordIndex].modelId,
                    (0.0f), (0.0f), (0.0f));
}

/* Removes a player's model from its activity model's item attachment point. */
void fn_1_37C4(s16 recordIndex) {
    HU3D_MODELID modelId;

    modelId = fn_1_A518(lbl_1_bss_54[recordIndex].activityIndex);
    Hu3DModelHookObjReset(modelId, "itemhook");
}

/* Places all four players for results. Outside practice, places 0-2 receive bonuses 0-2;
 * other players receive the finisher count, with a minimum bonus of one. */
void fn_1_381C(void) {
    s32 recordIndex = 0;
    s16 nonPodiumCount;
    M647PlayerRecord *record;
    M647PlayerRecord *motionRecord;
    HU3D_MODELID hookModelId;
    s32 playerIndexForPlacement;
    s16 finishPlace;
    s32 playerIndexForSingleBonus;
    s32 playerIndexForMultipleBonus;
    s16 coinBonus;

    nonPodiumCount = 0;
    for (; recordIndex < 4; recordIndex++) {
        record = &lbl_1_bss_54[recordIndex];
        hookModelId = fn_1_A518(lbl_1_bss_54[(s16)recordIndex].activityIndex);
        Hu3DModelHookObjReset(hookModelId, "itemhook");
        Hu3DModelRotSet(record->modelId,
                        (0.0f), (0.0f), (0.0f));
        Hu3DModelShadowSet(record->modelId);

        if (record->finishPlace < 3) {
            Hu3DModelPosSetV(record->modelId,
                             &lbl_1_data_160[record->finishPlace]);
            finishPlace = record->finishPlace;
            playerIndexForPlacement = record->playerIndex;
            if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                GwPlayer[playerIndexForPlacement].mgCoinBonus = finishPlace;
            }
        } else {
            if (lbl_1_bss_18 <= 1) {
                Hu3DModelPosSetV(record->modelId,
                                 &lbl_1_data_1B4[nonPodiumCount]);
                playerIndexForSingleBonus = record->playerIndex;
            if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                    GwPlayer[playerIndexForSingleBonus].mgCoinBonus = 1;
                }
            } else {
                Hu3DModelPosSetV(record->modelId,
                                 &lbl_1_data_184[nonPodiumCount]);
                coinBonus = lbl_1_bss_18;
                playerIndexForMultipleBonus = record->playerIndex;
        if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                    GwPlayer[playerIndexForMultipleBonus].mgCoinBonus = coinBonus;
                }
            }
            nonPodiumCount++;
        }

        motionRecord = &lbl_1_bss_54[(s16)recordIndex];
        if (motionRecord->motionState != 0) {
            CharMotionShiftSet(motionRecord->characterId,
                               motionRecord->motionIds[0],
                               (0.0f), (8.0f),
                               lbl_1_data_D0[0][1]);
            motionRecord->motionState = 0;
        }
    }
}

/* Returns the highest unclaimed activity slot at turn start; logs and waits forever if none
 * remains. */
s16 fn_1_3A44(void) {
    s32 entryIndex;

    for (entryIndex = 3; entryIndex >= 0; entryIndex--) {
        if (lbl_1_bss_22[entryIndex] == -1) {
            return (s16)entryIndex;
        }
    }
    OSReport("no pipe\n");
    for (;;) {
    }
}

/* Finds the nearest free slot to either side while a player changes selection. */
s16 fn_1_3AB4(s16 startIndex, s16 direction) {
    s32 index;

    if (direction < 0) {
        for (index = startIndex - 1; index >= 0; index--) {
            if (lbl_1_bss_22[index] == -1) {
                return (s16)index;
            }
        }
    } else {
        for (index = startIndex + 1; index < 4; index++) {
            if (lbl_1_bss_22[index] == -1) {
                return (s16)index;
            }
        }
    }
    return -1;
}

/* Handles human or CPU movement and slot selection during the player's turn. */
void fn_1_3B54(M647PlayerRecord *input, Point3d *position) {
    M647TimerThresholds timerThresholds = {{45, 40, 40, 40}};
    f32 destinationX;
    s16 selectedSlot;
    s16 randomValue;
    s32 searchIndex;

    if (input->cpuDifficulty == -1) {
        if (position->x == input->targetX && lbl_1_bss_14.mode == 0) {
            if (input->timer == 0 &&
                position->x == lbl_1_data_150[input->selectedSlot]) {
                fn_1_6484(input->selectedSlot);
            }
            if (input->timer > 20) {
                if ((s8)HuPadStkX[input->padIndex] > 20) {
                    if ((selectedSlot = fn_1_3AB4(input->selectedSlot, 1)) != -1) {
                        input->selectedSlot = selectedSlot;
                        fn_1_64A4();
                        input->timer = 0;
                    }
                } else if ((s8)HuPadStkX[input->padIndex] < -20) {
                    if ((selectedSlot = fn_1_3AB4(input->selectedSlot, -1)) != -1) {
                        input->selectedSlot = selectedSlot;
                        fn_1_64A4();
                        input->timer = 0;
                    }
                }
                if ((HuPadBtnDown[input->padIndex] & PAD_BUTTON_A) != 0) {
                    input->inputPhase = 6;
                    lbl_1_bss_22[input->selectedSlot] = input->occupancyMarker;
                    MgTimerKill(input->timerId);
                    input->timer = 0;
                    fn_1_64A4();
                    return;
                }
            }
        }
    } else if (position->x == input->targetX && lbl_1_bss_14.mode == 0) {
        if (input->timer == 0 &&
            position->x == lbl_1_data_150[input->selectedSlot]) {
            fn_1_6484(input->selectedSlot);
        }
        if (input->timer > timerThresholds.values[input->cpuDifficulty]) {
            input->timer = 0;
            if (input->selectedSlot != 0) {
                randomValue = (s16)frandmod(input->selectedSlot);
            } else {
                randomValue = 0;
            }
            if (randomValue != 0) {
                if ((selectedSlot = fn_1_3AB4(input->selectedSlot, -1)) != -1) {
                    input->selectedSlot = selectedSlot;
                    input->selectionCount++;
                    fn_1_64A4();
                } else {
                    input->inputPhase = 6;
                    lbl_1_bss_22[input->selectedSlot] = input->occupancyMarker;
                    input->targetX = lbl_1_data_150[input->selectedSlot];
                    fn_1_64A4();
                    MgTimerKill(input->timerId);
                    return;
                }
            } else {
                input->inputPhase = 6;
                lbl_1_bss_22[input->selectedSlot] = input->occupancyMarker;
                input->targetX = lbl_1_data_150[input->selectedSlot];
                fn_1_64A4();
                MgTimerKill(input->timerId);
                return;
            }
        }
    }

    if (position->x == lbl_1_data_150[input->selectedSlot]) {
        input->timer++;
    }
    destinationX = lbl_1_data_150[input->selectedSlot];
    if (input->targetX != destinationX) {
        fn_1_2B84(input->playerIndex, 2);
        input->targetX = destinationX;
    }
    if (position->x < input->targetX) {
        if (input->rotationY > 90.0f) {
            input->rotationY -= 10.0f;
        } else {
            position->x += 10.0f;
            if (position->x >= input->targetX) {
                position->x = input->targetX;
                fn_1_2B84(input->playerIndex, 0);
            }
        }
    } else if (position->x > input->targetX) {
        if (input->rotationY < 270.0f) {
            input->rotationY += 10.0f;
        } else {
            position->x -= 10.0f;
            if (position->x <= input->targetX) {
                position->x = input->targetX;
                fn_1_2B84(input->playerIndex, 0);
            }
        }
    }
    input->positionX = position->x;
    Hu3DModelPosSetV(input->modelId, position);
    Hu3DModelRotSet(input->modelId, 0.0f, input->rotationY, 0.0f);
}

/* Updates the current player's entrance, movement, and activity selection each frame. */
void fn_1_4204(void) {
    OM_CAMERA_VIEW view;
    Point3d position;
    M647PlayerRecord *record;
    s16 playerIndex;
    s16 selectedIndex;
    M647ActivityCallbacks callbackTable = {{fn_1_8A40, fn_1_98CC, fn_1_7230, fn_1_7D68}};

    if ((lbl_1_bss_174.playerTurn) >= 4) {
        lbl_1_bss_174.sequencePhase = 1;
        lbl_1_bss_174.phaseFrame = 0;
        return;
    }

    playerIndex = lbl_1_bss_1A[(lbl_1_bss_174.playerTurn)];
    record = &lbl_1_bss_54[playerIndex];
    Hu3DModelPosGet(record->modelId, &position);
    switch (record->inputPhase) {
    case 0:
        if ((lbl_1_bss_174.playerTurn) != 0) {
            fn_1_2B84(playerIndex, 2);
            selectedIndex = fn_1_3A44();
            HuAudFXPlayPan(MSM_SE_M647_PLAYER_ENTRY, 80);
            Hu3DMotionSpeedSet((lbl_1_bss_174.entranceModelId), 1.0f);
            Hu3DMotionTimeSet((lbl_1_bss_174.entranceModelId), 0.0f);
            record->inputPhase = 2;
            record->selectedSlot = selectedIndex;
            record->targetX = lbl_1_data_150[3];
            return;
        }
        {
            view.center.x = (800.0f);
            view.center.y = (150.0f);
            view.center.z = (-1720.0f);
            view.rot.x = (-16.0f);
            view.rot.y = view.rot.z = (0.0f);
            view.zoom = (5230.0f);
            omCameraViewMoveSimpleMulti(2, &view, 45);
            record->inputPhase = 1;
            return;
        }

    case 1:
        if (omCameraViewCheck(2) != 0) {
            fn_1_2B84(playerIndex, 2);
            selectedIndex = fn_1_3A44();
            Hu3DMotionSpeedSet((lbl_1_bss_174.entranceModelId), 1.0f);
            Hu3DMotionTimeSet((lbl_1_bss_174.entranceModelId), 0.0f);
            HuAudFXPlay(MSM_SE_M647_PLAYER_ENTRY);
            record->selectedSlot = selectedIndex;
            record->targetX = lbl_1_data_150[3];
            record->inputPhase++;
            return;
        }
        break;

    case 2:
        if (record->timer++ >= 15) {
            record->inputPhase = 3;
            record->timer = 0;
            return;
        }
        break;

    case 3:
        position.x -= (10.0f);
        if (position.x <= (100.0f) + lbl_1_data_150[3] &&
            record->cameraShifted == 0) {
            view.center.x = (0.0f);
            view.center.y = (150.0f);
            view.center.z = (-1720.0f);
            view.rot.x = (-16.0f);
            view.rot.y = view.rot.z = (0.0f);
            view.zoom = (5230.0f);
            omCameraViewMoveSimpleMulti(2, &view, 45);
            record->cameraShifted = 1;
        }
        if (position.x <= record->targetX) {
            position.x = record->targetX;
            record->inputPhase = 4;
            fn_1_185C();
            if ((lbl_1_bss_174.playerTurn) == 0) {
                fn_1_AE58(1);
                fn_1_AF18(0);
            }
            fn_1_2B84(playerIndex, 0);
        }
        if (position.x < 670.0f && position.y > 200.0f) {
            Hu3DModelShadowSet(record->modelId);
            position.y -= 10.0f;
        }
        record->positionX = position.x;
        Hu3DModelPosSetV(record->modelId, &position);
        return;

    case 4:
        if (record->timer++ > 10) {
            record->timer = 0;
            omVibrate(playerIndex, 20, 7, 3);
            record->inputPhase = 5;
            record->timerId = MgTimerCreate(0);
            MgTimerParamSet(record->timerId, 1800, 0, 0);
            MgTimerModeOnSet(record->timerId, 1);
            Hu3DModelAttrReset(lbl_1_bss_174.modelIds[0], 1);
            fn_1_1908();
            return;
        }
        break;

    case 5:
        if (omCameraViewCheck(2) != 0) {
            fn_1_3B54(record, &position);
            /* Manual confirmation can free the selection timer above; this check still uses its
             * pointer. */
            if (MgTimerDoneCheck(record->timerId) == 1) {
                record->targetX = lbl_1_data_150[record->selectedSlot];
                record->inputPhase = 6;
                position.x = record->targetX;
                lbl_1_bss_22[record->selectedSlot] = record->occupancyMarker;
                fn_1_64A4();
            }
            fn_1_1978(position.x);
        }
        break;

    case 6:
        record->activityIndex = 3 - record->selectedSlot;
        record->callback =
            callbackTable.entries[
                record->activityIndex];
        if (position.x == record->targetX) {
            fn_1_2B84(playerIndex, 1);
            if (record->rotationY < 180.0f) {
                record->rotationY += 10.0f;
            } else if (record->rotationY > 180.0f) {
                record->rotationY -= 10.0f;
            } else if (position.z > -400.0f) {
                position.z -= 10.0f;
            } else {
                position.z = -400.0f;
                record->inputPhase = 7;
            }
        } else if (position.x > record->targetX) {
            if (record->rotationY < 270.0f) {
                record->rotationY += 10.0f;
            } else {
                position.x -= 10.0f;
                if (position.x < record->targetX) {
                    position.x = record->targetX;
                }
            }
        } else if (position.x < record->targetX) {
            if (record->rotationY > 90.0f) {
                record->rotationY -= 10.0f;
            } else {
                position.x += 10.0f;
                if (position.x > record->targetX) {
                    position.x = record->targetX;
                }
            }
        }
        Hu3DModelPosSetV(record->modelId, &position);
        Hu3DModelRotSet(record->modelId, 0.0f, record->rotationY, 0.0f);
            fn_1_1978(position.x);
        break;

    case 7:
        if (record->rotationY > 0.0f) {
            record->rotationY -= 10.0f;
        } else {
            record->rotationY = 0.0f;
            record->inputPhase = 8;
            fn_1_2B84(playerIndex, 0);
            (lbl_1_bss_174.playerTurn)++;
        }
        Hu3DModelRotSet(record->modelId, 0.0f, record->rotationY, 0.0f);
        fn_1_1978(position.x);
        break;
    }
}

/* Moves the players out of the selection scene before they mount their activities. */
void fn_1_5430(void) {
    Point3d position;
    Point3d nextPosition;
    s32 index;
    M647PlayerRecord *initialRecord;
    M647PlayerRecord *fallingRecord;
    s16 delay;
    f32 travelStep;
    f32 landingHeight;

    if (lbl_1_bss_174.phaseFrame == 0) {
        for (index = 0; index < 4; index++) {
            initialRecord = &lbl_1_bss_54[(s16)index];
            if (initialRecord->motionState != 3) {
                CharMotionShiftSet(initialRecord->characterId,
                                   initialRecord->motionIds[3],
                                   (0.0f), (8.0f),
                                   lbl_1_data_D0[3][1]);
                initialRecord->motionState = 3;
            }
        }
    }

    if (++lbl_1_bss_174.phaseFrame >= 14) {
        delay = lbl_1_bss_174.phaseFrame - 14;
        for (index = 0; index < 4; index++) {
            fallingRecord = &lbl_1_bss_54[index];
            Hu3DModelPosGet(fallingRecord->modelId, &position);
            travelStep = (6.6666665f);
            position.z -= travelStep;
            landingHeight = (400.0f) -
                (((f32)delay * travelStep - (100.0f)) *
                 ((f32)delay * travelStep - (100.0f))) /
                    (100.0f);
            if (position.y > landingHeight && landingHeight < (300.0f)) {
                Hu3DModelShadowReset(fallingRecord->modelId);
            }
            position.y = landingHeight;
            if (delay > 2 &&
                fallingRecord->rotationY < (180.0f)) {
                fallingRecord->rotationY += (10.0f);
            }
            if (position.z < (-600.0f)) {
                position.z = (-600.0f);
            }
            Hu3DModelPosSetV(fallingRecord->modelId, &position);
            Hu3DModelRotSet(fallingRecord->modelId, (0.0f),
                            fallingRecord->rotationY,
                            (0.0f));
        }
        if (delay > 42) {
            HuAudFXPlay(MSM_SE_M647_PLAYER_FALL);
            lbl_1_bss_174.phaseFrame = 0;
            lbl_1_bss_174.sequencePhase = 2;
            for (index = 0; index < 4; index++) {
                position.x = position.z = (0.0f);
                position.y = (-30.0f);
                Hu3DModelPosGet(lbl_1_bss_54[(s16)index].modelId,
                                &nextPosition);
                nextPosition.x += position.x;
                nextPosition.y += position.y;
                nextPosition.z += position.z;
                Hu3DModelPosSetV(
                    lbl_1_bss_54[(s16)index].modelId,
                    &nextPosition);
            }
        }
    }
}

/* Frames the activity lineup after the players leave selection, then starts their mounting
 * sequence. */
void fn_1_57C0(void) {
    OM_CAMERA_VIEW view;
    M647PlayerRecord *shiftRecord;
    s32 recordIndex;
    M647PlayerRecord *record;

    if (lbl_1_bss_174.phaseFrame++ == 0) {
        view.center.x = (-1360.0f);
        view.center.y = (150.0f);
        view.center.z = (535.0f);
        view.rot.x = (-16.0f);
        view.rot.y = (22.0f);
        view.rot.z = (0.0f);
        view.zoom = (5230.0f);
        fn_1_AF18(1);
        omCameraViewMoveSimpleMulti(2, &view, 90);
        return;
    }
    if (omCameraViewCheck(2) != 0) {
        lbl_1_bss_174.phaseFrame = 0;
        lbl_1_bss_174.sequencePhase = 3;
        fn_1_AE58(0);
        lbl_1_bss_174.playerTurn = 0;
        for (recordIndex = 0; recordIndex < 4; recordIndex++) {
            record = &lbl_1_bss_54[recordIndex];
            shiftRecord = &lbl_1_bss_54[(s16)recordIndex];
            if (shiftRecord->motionState != 0) {
                CharMotionShiftSet(shiftRecord->characterId, shiftRecord->motionIds[0],
                                   (0.0f), (8.0f), lbl_1_data_D0[0][1]);
                shiftRecord->motionState = 0;
            }
            if (record->activityIndex == 0) {
                record->mountingPhase = 1;
            }
        }
    }
}

/* Moves players onto their assigned activities in activity order, then starts the launch
 * sequence. */
void fn_1_5978(void) {
    Point3d modelPosition;
    f32 horizontalDistance;
    f32 verticalProgress;
    f32 horizontalProgress;
    s16 oldFrame;
    s16 selectedIndex;
    s32 scanIndex;
    s32 recordIndex;
    M647PlayerRecord *player;
    M647PlayerRecord *initialMotionPlayer;
    M647PlayerRecord *landingMotionPlayer;

    for (scanIndex = 0; scanIndex < 4; scanIndex++) {
        for (recordIndex = 0; recordIndex < 4; recordIndex++) {
            if (scanIndex == lbl_1_bss_54[recordIndex].activityIndex) {
                selectedIndex = (s16)recordIndex;
                break;
            }
        }
        player = &lbl_1_bss_54[selectedIndex];
        if (player->mountingPhase == 1) {
            if (player->frame++ == 0) {
                Hu3DModelAttrReset(player->modelId, 1);
                Hu3DModelPosSetV(player->modelId, &lbl_1_data_2A4[scanIndex]);
                Hu3DModelRotSet(player->modelId, 0.0f, 90.0f, 0.0f);
                initialMotionPlayer = &lbl_1_bss_54[selectedIndex];
                if (initialMotionPlayer->motionState != 3) {
                    CharMotionShiftSet(initialMotionPlayer->characterId,
                                       initialMotionPlayer->motionIds[3],
                                       0.0f, 8.0f, lbl_1_data_D0[3][1]);
                    initialMotionPlayer->motionState = 3;
                }
                HuAudFXPlay(MSM_SE_M647_MOUNT_START);
            } else {
                if (player->frame < 40) {
                    Hu3DModelPosGet(player->modelId, &modelPosition);
                    horizontalDistance =
                        lbl_1_data_2D4[player->activityIndex].x -
                        lbl_1_data_2A4[player->activityIndex].x;
                    modelPosition.x += horizontalDistance / 40.0f;
                    verticalProgress =
                        (f32)player->frame *
                        ((lbl_1_data_2D4[player->activityIndex].y -
                          lbl_1_data_2A4[player->activityIndex].y) / 40.0f);
                    horizontalProgress =
                        horizontalDistance * (f32)player->frame / 40.0f;
                    modelPosition.y =
                        (verticalProgress +
                         (450.0f + lbl_1_data_2A4[player->activityIndex].y)) -
                        ((1800.0f / (horizontalDistance * horizontalDistance)) *
                         ((horizontalProgress - horizontalDistance / 2.0f) *
                          (horizontalProgress - horizontalDistance / 2.0f)));
                    Hu3DModelPosSetV(player->modelId, &modelPosition);
                }
                if (scanIndex < 3 && player->frame == 30) {
                    for (recordIndex = 0; recordIndex < 4; recordIndex++) {
                        if (scanIndex + 1 ==
                            lbl_1_bss_54[recordIndex].activityIndex) {
                            lbl_1_bss_54[recordIndex].mountingPhase = 1;
                        }
                    }
                }
                if (player->frame == 40) {
                    landingMotionPlayer = &lbl_1_bss_54[selectedIndex];
                    if (landingMotionPlayer->motionState != 4) {
                        CharMotionShiftSet(landingMotionPlayer->characterId,
                                           landingMotionPlayer->motionIds[4],
                                           0.0f, 8.0f, lbl_1_data_D0[4][1]);
                        landingMotionPlayer->motionState = 4;
                    }
                    Hu3DModelPosSetV(player->modelId,
                                     &lbl_1_data_2D4[player->activityIndex]);
                    HuAudFXPlay(MSM_SE_M647_MOUNT_LAND);
                } else if (player->frame == 65) {
                    fn_1_370C(selectedIndex);
                    fn_1_2B84(player->playerIndex, 5);
                    player->mountingPhase++;
                }
            }
        }
    }

    /* Checks whether every player has finished mounting their assigned activity. */
    for (scanIndex = 0; scanIndex < 4; scanIndex++) {
        if (lbl_1_bss_54[scanIndex].mountingPhase != 2) {
            break;
        }
    }
    if (scanIndex == 4) {
        lbl_1_bss_174.phaseFrame = 0;
        lbl_1_bss_174.playerTurn = 4;
        lbl_1_bss_174.sequencePhase = 4;
        HuAudSStreamFadeOut(lbl_1_bss_174.musicHandle, 1000);
    }
}

/* Plays the activity launch sequence, sets the race cameras and enables race updates. */
void fn_1_5F08(void) {
    OM_CAMERA_VIEW view;
    s16 maxMotionTime;
    s16 motionTimeCopy;
    s32 index;

    maxMotionTime = Hu3DMotionMaxTimeGet(
        lbl_1_bss_174.motionModelId);
    if (omCameraViewCheck(2) != 0) {
        if (lbl_1_bss_174.phaseFrame == 20) {
            for (index = 0; index < 4; index++) {
                fn_1_6ED0((s16)index, (1.0f));
            }
            lbl_1_bss_174.soundEffectId = HuAudFXPlay(MSM_SE_M647_LAUNCH_SEQUENCE);
        }
        for (index = 0; index < 4; index++) {
            if (lbl_1_bss_174.phaseFrame == index * 6 + 30) {
                fn_1_AC40((s16)index);
            }
        }
        if (lbl_1_bss_174.phaseFrame == 80) {
            Hu3DMotionSpeedSet(
                lbl_1_bss_174.motionModelId,
                (1.0f));
            view.center.x = lbl_1_bss_2C[0] = (-800.0f);
            view.center.y = (150.0f);
            view.center.z = (535.0f);
            view.rot.x = (-28.0f);
            view.rot.y = (0.0f);
            view.rot.z = (0.0f);
            view.zoom = lbl_1_bss_2C[1] = (5500.0f);
            omCameraViewMoveSimpleMulti(2, &view, 60);
            CenterM->x = (-1120.0f);
            CenterM->y = (300.0f);
            CenterM->z = (-620.0f);
            CRotM->x = (-5.0f);
            CRotM->y = (14.0f);
            CRotM->z = (0.0f);
            CZoomM[0] = (2500.0f);
        }
        if (lbl_1_bss_174.phaseFrame == 81) {
            HuAudFXPlay(MSM_SE_M647_LAUNCH_CUE);
        } else if (lbl_1_bss_174.phaseFrame == 141) {
            HuAudFXPlay(MSM_SE_M647_LAUNCH_CUE);
        } else if (lbl_1_bss_174.phaseFrame == 200) {
            lbl_1_bss_174.musicHandle = HuAudBGMPlay(MSM_STREAM_MGMUS_26);
        }
        if (maxMotionTime == (motionTimeCopy = Hu3DMotionTimeGet(
                lbl_1_bss_174.motionModelId))) {
            view.center.x = (-960.0f);
            view.center.y = (300.0f);
            view.center.z = (1000.0f);
            view.rot.x = (-40.0f);
            view.rot.y = (65.0f);
            view.rot.z = (0.0f);
            view.zoom = (3700.0f);
            omCameraViewMoveSimpleMulti(1, &view, 60);
            lbl_1_bss_174.phaseFrame = 0;
            lbl_1_bss_174.sequencePhase = 5;
            HuAudFXPlay(MSM_SE_M647_RACE_START);
            MgSeqModeDelaySet(5, 1);
        }
        lbl_1_bss_174.phaseFrame++;
    }
}

/* Ends race updates after the third player finishes and resets the local sequence state. */
s16 fn_1_62B8(void) {
    if (lbl_1_bss_18 >= 3) {
        lbl_1_bss_174.sequencePhase = 0;
        lbl_1_bss_174.phaseFrame = 0;
        return 1;
    }
    return 0;
}

/* Opens the selected slot's message when the request process asks for it. */
void fn_1_62FC(s32 selection) {
    HuVec2f textSize;

    if (lbl_1_bss_174.windowId == -1) {
        selection = (s16)(3 - selection);
        HuWinMesMaxSizeBetGet(&textSize, lbl_1_data_C0[(s16) selection],
                              lbl_1_data_C0[(s16) selection]);
        lbl_1_bss_174.windowId = HuWinExCreateFrame((-10000.0f), (380.0f),
                                                    (s16)textSize.x, (s16)textSize.y, -1, 1);
        HuWinExOpen(lbl_1_bss_174.windowId);
        HuWinMesSet(lbl_1_bss_174.windowId, lbl_1_data_C0[(s16)selection]);
        HuWinMesSpeedSet(lbl_1_bss_174.windowId, 0);
    }
}

/* Closes and releases the active message window when requested. */
void fn_1_6414(void) {
    if (lbl_1_bss_174.windowId != -1) {
        HuWinHomeClear(lbl_1_bss_174.windowId);
        HuWinExClose(lbl_1_bss_174.windowId);
        HuWinExKill(lbl_1_bss_174.windowId);
        lbl_1_bss_174.windowId = -1;
    }
}

/* Queues a message-window request for the selected activity slot. */
void fn_1_6484(s16 selectionIndex) {
    lbl_1_bss_14.mode = 1;
    lbl_1_bss_14.argument = selectionIndex;
}

/* Requests the message-window process to close its active window. */
void fn_1_64A4(void) {
    lbl_1_bss_14.mode = 2;
}

M647AnimState lbl_1_bss_174;
M647PlayerRecord lbl_1_bss_54[4];
M647CameraSequence lbl_1_bss_34;
f32 lbl_1_bss_2C[2];
s16 lbl_1_bss_22[5];
s16 lbl_1_bss_1A[4];
s16 lbl_1_bss_18;
M647ModeState lbl_1_bss_14;
s32 lbl_1_bss_10;
f32 lbl_1_bss_C;
ANIMDATA *lbl_1_bss_8;
