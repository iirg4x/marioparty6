#ifndef REL_M625DLL_PLAYERS_H
#define REL_M625DLL_PLAYERS_H

#include "REL/m625Dll/wave.h"

typedef struct M625Player {
    s16 playerIndex;
    s16 characterNo;
    s16 controllerIndex;
    s16 team;
    s32 computerPlayer;
    s16 difficulty;
    s16 status;
    s16 actionState;
    HU3D_MODELID objectModel;
    s16 motionIds[12];
    s16 motionSet;
    Point3d position;
    Point3d rotation;
    Point3d scale;
    HU3D_LIGHTID light;
    Point3d velocity;
    f32 collisionRadius;
    f32 height;
    s8 stickX;
    s8 stickY;
    u16 buttons;
    u16 buttonsDown;
    HU3D_MODELID scaleModel;
    f32 modelScale;
    s16 timer;
    s16 particleTimer;
    Point3d targetRotation;
    f32 movedDistance;
    f32 moveSpeed;
    Point3d storedVelocity;
    u32 unknown;
    Point3d target;
    s32 hasTarget;
    s16 updateFrames;
    s16 motionCounter;
    s16 turnDirection;
} M625Player;

typedef struct M625MotionPreset {
    s16 motionId;
    s16 overlayMotionId;
    f32 blendTime;
    u32 modelAttributes;
} M625MotionPreset;

extern M625Player lbl_1_bss_1D8[4];
extern HU3D_MODELID lbl_1_bss_1C8;
extern s32 lbl_1_bss_1CC;
extern s32 lbl_1_bss_1D0;
extern s32 lbl_1_bss_1D4;
extern u32 lbl_1_data_1E8[12];
extern Point3d lbl_1_data_218[4];
extern Point3d lbl_1_data_248[4];
extern M625LightPreset lbl_1_data_278[2];
extern M625MotionPreset lbl_1_data_2B8[11];
extern s16 lbl_1_data_36C[4];
extern s32 lbl_1_data_3E4;
extern s16 lbl_1_data_3E8[4];

#endif
