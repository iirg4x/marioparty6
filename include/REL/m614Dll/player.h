#ifndef REL_M614DLL_PLAYER_H
#define REL_M614DLL_PLAYER_H

#include "game/main.h"

typedef struct M614PlayerEffect {
    s32 state;
    f32 x;
    f32 y;
    f32 z;
    f32 scale;
} M614PlayerEffect;

typedef struct M614MovingEffect {
    s32 state;
    f32 x;
    f32 y;
    f32 z;
    f32 progress;
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 accelerationY;
} M614MovingEffect;

typedef struct M614PlayerCallbackWork {
    s32 state;
    s32 playerNumber;
    s32 layoutIndex;
    s32 character;
    s32 padIndex;
    s32 difficulty;
    s32 lightId;
    /* Eight bytes of player work whose contents are not yet known. */
    u8 unk_1C[8]; /* No code names this field. */
    s32 actionFrames;
    s32 elapsedFrames;
    M614MovingEffect movingEffects[6];
    M614PlayerEffect effects[6];
    s32 playerCount;
    s32 spriteFrame[5];
    s32 hookResetPending;
    s32 effectDirection;
    s32 characterState;
    s32 characterStateFrame;
    f32 positionX;
    f32 positionY;
    f32 positionZ;
    f32 movementX;
    f32 movementY;
    f32 turnSpeed;
    f32 headingForward;
    f32 headingSide;
    f32 rotationX;
    f32 headingYaw;
    f32 rotationZ;
    f32 headingOffset;
    char *hookName;
    s32 targetPlayer;
    s32 movementMode;
    s32 modeFrameA;
    s32 modeFrameB;
    f32 targetX;
    f32 targetZ;
    f32 targetAngle;
    f32 rotationAngle;
    f32 rotationSpeed;
    s32 movementSoundState;
} M614PlayerCallbackWork;
void fn_1_2DC(int *x, int *y, float limit);
void fn_1_5CC(void);

#endif
