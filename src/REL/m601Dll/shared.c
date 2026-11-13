/* Smashdance player control, stage objects, score display, and minigame setup. */
#include "math.h"

#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"

#include "dolphin/types.h"

#include "dolphin/os.h"

#include "dolphin/gx.h"

#include "dolphin/mtx.h"

#include <dolphin/pad.h>

#include "string.h"

#include "game/hu3d.h"

#include "game/object.h"

#include "game/pad.h"

#include "game/printfunc.h"

#include "game/mg/actman.h"

#include "game/charman.h"

#include "game/main.h"

#include "game/mg/seqman.h"

#include "game/mg/score.h"

#include "REL/m601Dll/sound_entry.h"

#include "dolphin.h"

#include "humath.h"

#include "datadir_enum.h"

#include "messdir_enum.h"

#include "game/gamework.h"

#include "game/audio.h"

#include "game/sprite.h"

#include "game/memory.h"

#include "game/data.h"

#include "game/window.h"

#include "game/wipe.h"

#include "game/frand.h"

#include "game/process.h"

#include "game/hsfex.h"

#include "game/gamemes.h"

#include "game/flag.h"

#include "game/armem.h"

#include "game/mgdata.h"

#include "game/saveload.h"

#include "game/esprite.h"

#include "game/thpmain.h"

#include "game/mg/timer.h"

#include "game/board/object.h"

#include "game/board/main.h"

#include "game/board/audio.h"

#include "game/board/tutorial.h"

#include "game/board/masu.h"

#include "game/board/player.h"

#include "game/init.h"

#include "REL/m601Dll.h"

typedef void (*M601VoidFunc)(void);

typedef struct M601CallbackStorage {
    void (*callback)(MGPLAYER *);
    u8 unknown04[0x8]; /* No code names this field. */
} M601CallbackStorage;

typedef struct M601PlayerControlWork {
    s32 playerNo;
    s32 configNo;
    s32 modelNo;
    s32 characterNo;
    s32 padNo;
    s32 difficulty;
    s32 firstLight;
    s32 secondLight;
    s32 state20[4];
    s32 entryMode;
    s32 state34[5];
    float scale;
    s32 score;
    float scoreScale;
    s32 resultPlacement;
    s32 motionState;
    float startX;
    float startZ;
    float targetX;
    float targetZ;
    s32 collisionPriority;
    s32 collisionFrames;
    s32 controlMode;
    s32 routeTime;
    s32 targetIndex;
    s32 controlTime;
    s32 directionState;
} M601PlayerControlWork;

typedef struct M601StageUpdateObject {
    s32 state;
    s32 animationTime;
    s32 routeTime;
    s32 eventState;
    Point3d position;
    s32 activationTime;
} M601StageUpdateObject;

typedef struct M601StagePlayerState {
    s32 playerNo;
    s32 configNo;
    s32 modelNo;
    s32 characterNo;
    s32 padNo;
    s32 difficulty;
    s32 firstLight;
    s32 secondLight;
    s32 clearedValues[4];
    s32 entryMode;
    s32 unk_34[5]; /* No code names this field. */
    float scale;
    s32 score;
    float scoreScale;
    s32 resultPlacement;
    s32 motionState;
    float startX;
    float startZ;
    float targetX;
    float targetZ;
} M601StagePlayerState;

typedef struct M601StagePosition {
    float x;
    float z;
} M601StagePosition;

typedef struct M601PlayerInitWork {
    s32 playerNo;
    s32 configNo;
    s32 modelNo;
    s32 characterNo;
    s32 padNo;
    s32 difficulty;
    s32 firstLight;
    s32 secondLight;
    s32 state20[4];
    s32 mode;
    s32 state34[5];
    float scale;
    s32 state4C[11];
    s32 randomX;
    s32 state7C;
    s32 randomZ;
    s32 state84[2];
} M601PlayerInitWork;


/* The initialized interval contains one float and 0x2C unidentified bytes. */
typedef struct M601UnknownRodata43C {
    float value00; /* the game's code never reads this field */
    u8 unknown04[0x2C]; /* No code names this field. */
} M601UnknownRodata43C;

/* BSS intervals with unidentified bytes. */
typedef struct M601UnknownBss4 {
    s32 winnerCount;
    u8 unknown04[0x4]; /* No code names this field. */
} M601UnknownBss4;

typedef struct M601UnknownBss124 {
    u32 flushSize;
    u8 unknown04[0x24]; /* No code names this field. */
} M601UnknownBss124;

typedef struct M601UnknownBss60 {
    Mtx positionMatrix;
    u8 unknown30[0x80]; /* No code names this field. */
} M601UnknownBss60;

extern char lbl_1_data_E8[9];

extern char lbl_1_data_F1[3];

float fn_1_0(float x, float y)
{
    return sqrtf(x * x + y * y);
}

void fn_1_134(float *x, float *y, float maximum)
{
    float length = fn_1_0(*x, *y);
    if (length > maximum) {
        *x = maximum * (*x / length);
        *y = maximum * (*y / length);
    }
}

void fn_1_2DC(int *x, int *y, float maximum)
{
    float length = fn_1_0(*x, *y);
    if (length > maximum) {
        *x = maximum * (*x / length);
        *y = maximum * (*y / length);
    }
}

extern const M601VoidFunc _ctors[];

extern const M601VoidFunc _dtors[];

void ObjectSetup(void);

int _prolog(void)
{
    const M601VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    ObjectSetup();
    return 0;
}

void _epilog(void)
{
    const M601VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

void fn_1_5CC(void)
{
    Point3d eye, center, up;
    float rx, ry, rz;

    lbl_80266A4C.x += 25.0 * (((lbl_802C0010[0] & 2) != 0) - ((lbl_802C0010[0] & 1) != 0));
    lbl_80266A4C.y += 25.0 * (((lbl_802C0010[0] & 8) != 0) - ((lbl_802C0010[0] & 4) != 0));
    lbl_802C028C += 25.0 * (((lbl_802C0010[0] & PAD_BUTTON_X) != 0) - ((lbl_802C0010[0] & PAD_BUTTON_Y) != 0));
    lbl_80266A40.y += lbl_802BFFF4[0] / 32.0;
    lbl_80266A40.x += lbl_802BFFF0[0] / -32.0;
    fontcolor = 15;
    print8(64, 392, 2.0f, lbl_1_data_E8, lbl_80266A4C.x, lbl_80266A4C.y, lbl_80266A4C.z);
    print8(64, 408, 2.0f, lbl_1_data_E8, lbl_80266A40.x, lbl_80266A40.y, lbl_80266A40.z);
    print8(64, 424, 2.0f, lbl_1_data_F1, lbl_802C028C);
    rx = lbl_80266A40.x;
    ry = lbl_80266A40.y;
    rz = lbl_80266A40.z;
    eye.x = lbl_80266A4C.x + lbl_802C028C * (sin(M_PI * ry / 180.0) * cos(M_PI * rx / 180.0));
    eye.y = lbl_80266A4C.y + lbl_802C028C * -sin(M_PI * rx / 180.0);
    eye.z = lbl_80266A4C.z + lbl_802C028C * (cos(M_PI * ry / 180.0) * cos(M_PI * rx / 180.0));
    center.x = lbl_80266A4C.x;
    center.y = lbl_80266A4C.y;
    center.z = lbl_80266A4C.z;
    up.x = sin(M_PI * ry / 180.0) * sin(M_PI * rx / 180.0);
    up.y = cos(M_PI * rx / 180.0);
    up.z = cos(M_PI * ry / 180.0) * sin(M_PI * rx / 180.0);
}

void fn_1_2A3C(OMOBJ *obj);

/* Sets the sound effect's pan and clamps its distance-scaled volume. */
void fn_1_BC4(s32 handle, float horizontalPosition, float distance)
{
    s16 volume;
    s16 minimumVolume;
    s16 maximumVolume;

    HuAudFXPanning(handle, (s16)(0.5 + (double)(64.0f
        + (63.0f * (0.25f
            * (horizontalPosition / 650.0f))))));
    volume = (s16)(0.5 + (double)(104.0f
        + (23.0f * (1.0f
            + (distance / 550.0f)))));
    if (volume < 0x68) {
        minimumVolume = 0x68;
    } else {
        if (volume > 0x7F) {
            maximumVolume = 0x7F;
        } else {
            maximumVolume = (s16)volume;
        }
        minimumVolume = maximumVolume;
    }
    volume = (s16)minimumVolume;
    HuAudFXVolSet(handle, volume);
}

extern M601CallbackStorage lbl_1_bss_114;

/* 0.7 */

void fn_1_D14(MGPLAYER *playerP)
{
    int previousMode;

    previousMode = playerP->subMode;
    lbl_1_bss_114.callback(playerP);
    if (previousMode == 2 && playerP->subMode == 3) {
        playerP->work[2] = 1;
    }
}

void fn_1_D7C(MGPLAYER *playerP)
{
    if (playerP->motNo != 10) {
        fn_8005C2C8(playerP->charNo, playerP->omObj->mtnId[10],
            0.0f, 5.0f, 0U);
        playerP->motNo = 10;
    }
    playerP->actor->rotY *= 0.7;
}

void fn_1_E08(MGPLAYER *playerP)
{
    if (playerP->motNo != 11) {
        fn_8005C2C8(playerP->charNo, playerP->omObj->mtnId[11],
            0.0f, 1.0f, ((1 << 30) | (1 << 0)));
        playerP->motNo = 11;
    }
}

void fn_1_E78(MGPLAYER *playerP)
{
    if (playerP->motNo != 12) {
        fn_8005C2C8(playerP->charNo, playerP->omObj->mtnId[12],
            0.0f, 1.0f, 0U);
        playerP->motNo = 12;
    }
}

extern MGPLAYER *lbl_1_bss_158[4];

extern OMOBJ *lbl_1_bss_16C[5];

extern s32 lbl_1_bss_154;

extern s32 lbl_1_bss_0;

extern s32 lbl_1_bss_10;

extern s32 lbl_1_bss_C;

extern s32 lbl_1_data_4;

extern M601StageUpdateObject lbl_1_bss_244[16];

extern MGSCORE *lbl_1_data_20[4];

/* Reconstruction of removed debug code. */
static inline void DebugCheckPlayerInput(M601PlayerControlWork *work, const HuVecF *position, s32 enabled)
{
    s32 playerNo;
    s32 targetIndex;
    float x;
    float z;
    if (enabled) {
        playerNo = work->playerNo;
        targetIndex = work->targetIndex;
        if (playerNo < 0 || playerNo >= 4 || targetIndex < 0 || targetIndex >= 0x10) {
            return;
        }
        if (lbl_1_bss_158[playerNo] == 0 || lbl_1_bss_16C[playerNo] == 0) {
            return;
        }
        if (((M601PlayerControlWork *)lbl_1_bss_16C[playerNo]->data)->targetIndex != targetIndex) {
            return;
        }
        if (lbl_1_bss_158[playerNo]->mode != 0) {
            return;
        }
        if (lbl_1_bss_244[targetIndex].state != 1) {
            return;
        }
        x = position->x - lbl_1_bss_244[targetIndex].position.x;
        z = position->z - lbl_1_bss_244[targetIndex].position.y;
        if (x != x || z != z) {
            return;
        }
        x *= 5.6f;
        z *= -5.6f;
        if (x < -56.0f) {
            x = -56.0f;
        }
        if (z < -56.0f) {
            z = -56.0f;
        }
        if (x > 56.0f) {
            x = 56.0f;
        }
        if (z > 56.0f) {
            z = 56.0f;
        }
        if (x * x + z * z > 56.0f * 56.0f) {
            x *= 0.5f;
            z *= 0.5f;
        }
        if (x == 0.0f && z == 0.0f) {
            return;
        }
        fn_80080520(lbl_1_bss_158[work->playerNo], (s32)x, (s32)z, 0, 0);
    }
}

void fn_1_EE4(OMOBJ *obj)
{
    M601PlayerControlWork *work = obj->data;
    MGPLAYER *player = lbl_1_bss_158[work->playerNo];
    s32 i;
    int padX;
    int padZ;
    s32 buttonDown;
    s32 button;
    HU3D_MODEL *model;
    HuVecF playerPositions[4];
    void *playerData[4];
    HuVecF playerPosition;
    s32 unusedTargetIndex;
    float unknownFloat = 0.0f;
    s32 unknownA = 0;
    s32 unknownB = 0;
    s32 unknownC = 0;
    s32 unknownD = 0;

    for (i = 0; i < 4; i++) {
        playerData[i] = lbl_1_bss_16C[i]->data;
        fn_80086BC8(lbl_1_bss_158[i]->actor, &playerPositions[i]);
    }
    fn_80086BC8(player->actor, &playerPosition);
    if (work->playerNo <= 0 || work->entryMode != 3) {
        model = &lbl_802C01C8[*obj->mdlId];
        obj->trans.x = playerPosition.x;
        obj->trans.y = playerPosition.y;
        obj->trans.z = playerPosition.z;
        obj->rot.y = player->actor->rotY;
        obj->scale.x = model->scale.x;
        obj->scale.y = model->scale.y;
        obj->scale.z = model->scale.z;
    }

    lbl_1_bss_154 = fn_80071A8C();
    lbl_1_bss_0 = fn_80071DE4();
    if (lbl_1_bss_154 == 2 && lbl_1_bss_0 == 1) {
        fn_8005C2C8(player->charNo, player->omObj->mtnId[0xB],
            6.0f, 0.0f, 0x40000001U);
    } else if (lbl_1_bss_154 == 2 && lbl_1_bss_0 == 0x96) {
        fn_8005C2C8(player->charNo, player->omObj->mtnId[0xA],
            6.0f, 0.0f, 0U);
    } else if (lbl_1_bss_154 == 2 && lbl_1_bss_0 == 0xBE) {
        fn_8005C2C8(player->charNo, *player->omObj->mtnId,
            12.0f, 0.0f, 0x40000001U);
    }
    if (lbl_1_bss_10 != 0) {
        fn_80028E48(*obj->mdlId, (HU3D_LLIGHTID)work->secondLight,
            0x54, 0x54, 0x6C, 0xFF);
        fn_800273EC(*obj->mdlId, 0.325f, 0.325f,
            0.325f);
    }
    if (lbl_1_bss_154 == 5) {
        if (work->padNo == -1) {
        s32 playerModeAttributes;
        s32 targetTimes[4] = {65, 52, 25, 5};
        s32 animationTimes[4] = {65, 57, 35, 10};
        s32 movementTimes[4] = {30, 25, 15, 5};
        s32 routeLimits[4] = {50, 45, 30, 15};
        float movementSpeeds[4] = {20.0f, 50.0f, 150.0f, 180.0f};
        padX = 0;
        padZ = 0;
        button = 0;
        buttonDown = 0;
        playerModeAttributes = fn_80087148(player, 0x13U);
        switch (work->controlMode) {
        case 1:
            break;
        case 2:
            work->controlTime++;
            if (lbl_1_bss_244[work->targetIndex].state != 1) {
                work->routeTime++;
                if (work->routeTime >= routeLimits[work->difficulty] ||
                    work->directionState >= 0xF) {
                    work->controlMode = 0;
                    work->controlTime = (s32)((((float)animationTimes[work->difficulty] *
                        ((float)rand8() / 255.0f)) / 5.0f) -
                        (float)((animationTimes[work->difficulty] / 5) / 2));
                    work->directionState = 0;
                    work->routeTime = (s32)((((float)targetTimes[work->difficulty] *
                        ((float)rand8() / 255.0f)) / 5.0f) -
                        (float)((targetTimes[work->difficulty] / 5) / 2));
                }
            }
            if (work->controlMode == 2 &&
                work->controlTime >= movementTimes[work->difficulty]) {
                float targetX;
                float targetZ;
                targetX = lbl_1_bss_244[work->targetIndex].position.x;
                targetZ = lbl_1_bss_244[work->targetIndex].position.y;
                targetX = playerPosition.x < targetX -
                    lbl_1_bss_244[i].position.z / 3.0f ?
                    targetX - lbl_1_bss_244[i].position.z / 3.0f :
                    (playerPosition.x > targetX + lbl_1_bss_244[i].position.z / 3.0f ?
                     targetX + lbl_1_bss_244[i].position.z / 3.0f : playerPosition.x);
                targetZ = playerPosition.z < targetZ -
                    lbl_1_bss_244[i].position.z / 3.0f ?
                    targetZ - lbl_1_bss_244[i].position.z / 3.0f :
                    (playerPosition.z > targetZ + lbl_1_bss_244[i].position.z / 3.0f ?
                     targetZ + lbl_1_bss_244[i].position.z / 3.0f : playerPosition.z);
                padX = (s32)((targetX - playerPosition.x) * 5.6f);
                padZ = -(s32)((targetZ - playerPosition.z) * 5.6f);
                fn_1_2DC(&padX, &padZ, 56.0f);
                if ((playerModeAttributes & 0x10) != 0) {
                    work->directionState += work->directionState != 8 ||
                        fn_1_0(targetX - playerPosition.x,
                            targetZ - playerPosition.z) < 5.0f;
                    if (work->directionState >= 0 && work->directionState <= 8) {
                        button = PAD_BUTTON_A;
                    } else if (work->directionState == 9) {
                        button = 0;
                    } else if (work->directionState == 10) {
                        button = buttonDown = PAD_BUTTON_A;
                    }
                } else {
                    if (fn_1_0(targetX - playerPosition.x,
                        targetZ - playerPosition.z) < movementSpeeds[work->difficulty]) {
                        buttonDown = button = PAD_BUTTON_A;
                    }
                    work->directionState = 0;
                }
            }
            break;
        case 0:
            work->controlTime++;
            if (work->controlTime >= animationTimes[work->difficulty] &&
                player->mode == 0) {
                work->routeTime++;
                if (work->routeTime >= targetTimes[work->difficulty]) {
                    float nearestDistance = 10000.0f;
                    unusedTargetIndex = -1;
                    for (i = 0; i < 0x10; i++) {
                        if (lbl_1_bss_244[i].state == 1) {
                            float candidateDistance;
                            candidateDistance = fn_1_0(playerPosition.x -
                                lbl_1_bss_244[i].position.x, playerPosition.z -
                                lbl_1_bss_244[i].position.y);
                            if (work->controlMode == 1 && i == work->targetIndex) {
                                candidateDistance -= 300.0f;
                            }
                            if (nearestDistance > candidateDistance) {
                                work->targetIndex = i;
                                work->controlMode = 2;
                                work->routeTime = 0;
                                work->controlTime = 0;
                                nearestDistance = candidateDistance;
                            }
                        }
                    }
                }
            }
            break;
        }
        fn_80080520(player, padX, padZ,
            buttonDown | (lbl_802C0008[player->padNo] & ~PAD_BUTTON_A),
            button | (lbl_802C0010[player->padNo] & ~PAD_BUTTON_A));
        }
    } else if ((lbl_1_bss_154 == 6 || lbl_1_bss_154 == 7) && lbl_1_bss_C != 0) {
        if (player->mode == 0) {
            player->actor->rotY *= 0.7;
            work->motionState = 1;
        }
        fn_80080520(player, 0, 0,
            lbl_802C0008[player->padNo] & ~PAD_BUTTON_A,
            lbl_802C0010[player->padNo] & ~PAD_BUTTON_A);
    } else if (lbl_1_bss_154 == 7 && lbl_1_data_4 != -1) {
        fn_80080520(player, 0, 0,
            lbl_802C0008[player->padNo] & ~PAD_BUTTON_A,
            lbl_802C0010[player->padNo] & ~PAD_BUTTON_A);
        if (lbl_1_bss_0 >= 0) {
            int resultPadX = 0;
            int resultPadZ = 0;
            float nearestDistance;

            nearestDistance = ((work->targetX - playerPosition.x) *
                (work->targetX - playerPosition.x)) +
                ((work->targetZ - playerPosition.z) *
                (work->targetZ - playerPosition.z));
            resultPadX = (s32)(5.6f * (work->startX - playerPosition.x));
            resultPadZ = -(s32)(5.6f * (work->startZ - playerPosition.z));
            fn_1_2DC(&resultPadX, &resultPadZ, 56.0f);
            fn_80080520(player, resultPadX, resultPadZ,
                lbl_802C0008[player->padNo] & ~PAD_BUTTON_A,
                lbl_802C0010[player->padNo] & ~PAD_BUTTON_A);
            i = work->motionState;
            work->motionState = ((resultPadX == 0 && resultPadZ == 0) &&
                nearestDistance <= 4.0f) && playerPosition.y <= 5.0f;
        }
    } else {
        fn_80080520(player, 0, 0,
            lbl_802C0008[player->padNo] & ~PAD_BUTTON_A,
            lbl_802C0010[player->padNo] & ~PAD_BUTTON_A);
    }
    if (work->motionState == 1) {

        if (lbl_1_bss_C == 0) {
            player->actor->rotY = 0.0f;
        }
        if (lbl_1_bss_154 == 8) {
            work->motionState = 2;
            player->actor->rotY = 0.0f;
            fn_8005C2C8(player->charNo,
                player->omObj->mtnId[(work->resultPlacement < 0 || lbl_1_data_4 == 4) + 5], 0.0f,
                0.0f, 0U);
        }
    } else if (work->motionState == 2) {
        player->actor->rotY = 0.0f;
    }
    if (work->motionState == 0 && lbl_1_bss_154 == 8) {
        work->motionState = 1;
    }
    work->scoreScale -= (work->scoreScale > 0.0f) ? 0.125f : work->scoreScale;
    if (lbl_1_data_20[work->playerNo] != 0) {
        MgScoreScaleSet(lbl_1_data_20[work->playerNo], 1.0f +
            (0.25f * work->scoreScale), 1.0f +
            (0.25f * work->scoreScale));
    }
    DebugCheckPlayerInput(work, &playerPosition, 0);
}

/* Returns the normalized animation envelope at the requested frame. */
float fn_1_2878(float frame, float rise, float hold, float fall)
{
    float area = fall / 2.0f + (rise / 2.0f + hold);

    return frame < rise
        ? ((1.0f + frame) / 2.0f * frame) / rise / area
        : frame < rise + hold
            ? ((1.0f + rise) / 2.0f * rise) / rise / area
                + (frame - rise) / area
            : frame < fall + (rise + hold) - 1.0f
                ? 1.0f - ((1.0f + (fall + (rise + hold) - frame - 1.0f))
                    / 2.0f * (fall + (rise + hold) - 1.0f - frame - 1.0f))
                    / (fall - 1.0f) / area
                : 1.0f;
}

extern M601SoundEntry lbl_1_bss_184[6];

extern M601UnknownBss4 lbl_1_bss_4;

extern s32 lbl_1_bss_14;

extern s32 lbl_1_bss_18;

extern s32 lbl_1_bss_20;

extern s32 lbl_1_bss_1C;

extern s32 lbl_1_bss_24;

extern s32 lbl_1_bss_28;

extern s32 lbl_1_bss_110;

extern s32 lbl_1_bss_2C;

extern s32 lbl_1_data_C;

extern s32 lbl_1_data_F4;

extern s32 lbl_1_data_F8[12];

extern s32 lbl_1_data_128[12];

extern s32 lbl_1_data_158;

extern Point3d lbl_1_data_58[12];

void fn_1_2A3C(OMOBJ *obj)
{
    M601StagePlayerState *playerData[4];
    s32 i;
    s32 j;
    s32 firstHalf;
    s32 secondHalf;
    s32 thirdHalf;
    s32 timerStage;
    s32 attempts;

    for (i = 0; i < 4; i++) {
        playerData[i] = lbl_1_bss_16C[i]->data;
    }
    lbl_1_bss_154 = fn_80071A8C();
    lbl_1_bss_0 = fn_80071DE4();
    lbl_1_data_C++;

    if (lbl_1_bss_154 <= 3 ||
        (lbl_1_bss_154 >= 8 && lbl_1_bss_C == 0 && lbl_1_bss_154 < 0xB)) {
        for (i = 0; i < 16; i++) {
            j = 0;
            if ((lbl_1_bss_154 == 2 && lbl_1_bss_0 > 0x69) ||
                (lbl_1_bss_154 == 3 && lbl_1_bss_0 <= 0x2D)) {
                if (lbl_1_bss_20 % 60 == 0) {
                    firstHalf = 1;
                    secondHalf = 1;
                    thirdHalf = 1;
                    if (i != 5 && i != 6) {
                        thirdHalf = 0;
                    }
                    if (thirdHalf == 0 && i != 9) {
                        secondHalf = 0;
                    }
                    if (secondHalf == 0 && i != 0xA) {
                        firstHalf = 0;
                    }
                    if ((lbl_1_bss_20 % 120) / 60 == firstHalf) {
                        j = 0x3C;
                    }
                }
            } else if (lbl_1_bss_154 <= 3) {
                lbl_1_bss_244[i].activationTime = 0;
            } else if (lbl_1_bss_4.winnerCount == 1) {
                if (lbl_1_bss_20 % 60 == 0) {
                    if ((lbl_1_bss_20 % 120) / 60 ==
                        (i + (i >= 8) + (i % 8 >= 6) + (i % 8 >= 2)) % 2) {
                        j = 0x3C;
                    }
                }
            } else if (lbl_1_bss_4.winnerCount == 2) {
                j = i < 8 ? i : 0xF - i;
                j = j >= 4 ? 0xB - j : j;
                if (lbl_1_bss_20 % 30 == 0 &&
                    j == (lbl_1_bss_20 % 270) / 30) {
                    j = 0x1E;
                } else {
                    j = 0;
                }
            } else if (lbl_1_bss_4.winnerCount == 3) {
                if (rand8() == 0 && lbl_1_bss_14 < 4) {
                    j = 0x3C;
                }
            } else if (lbl_1_data_4 != 4 && lbl_1_bss_4.winnerCount == 4) {
                j = 1;
            }

            if (j > 0) {
                if (lbl_1_bss_244[i].state == 3) {
                    lbl_1_bss_1C--;
                }
                if (lbl_1_bss_244[i].state == 1 ||
                    lbl_1_bss_244[i].state == 2) {
                    lbl_1_bss_14--;
                }
                lbl_1_bss_244[i].animationTime = lbl_1_bss_244[i].routeTime = j + 1;
                lbl_1_bss_244[i].state = 1;
                lbl_1_bss_244[i].eventState = 0;
                if (lbl_1_data_4 != 4 && lbl_1_bss_4.winnerCount == 4) {
                    lbl_1_bss_244[i].routeTime = 0x5A;
                    lbl_1_bss_244[i].animationTime =
                        (lbl_1_bss_20 % 120) + 1;
                    lbl_1_bss_244[i].animationTime =
                        lbl_1_bss_244[i].animationTime > lbl_1_bss_244[i].routeTime ?
                        lbl_1_bss_244[i].routeTime : lbl_1_bss_244[i].animationTime;
                }
                if (lbl_1_bss_184[2].count < 3) {
                    lbl_1_bss_184[2].x[lbl_1_bss_184[2].count] =
                    lbl_1_bss_244[i].position.x;
                    lbl_1_bss_184[2].z[lbl_1_bss_184[2].count] =
                    lbl_1_bss_244[i].position.y;
                    lbl_1_bss_184[2].count++;
                }
                fn_8002D414(obj->mdlId[i + 0x3A], 0.0f);
                fn_80026A44(obj->mdlId[i + 0x3A], (1 << 0));
                fn_8002D414(obj->mdlId[i + 0x4A], 0.0f);
                fn_80026A44(obj->mdlId[i + 0x4A], (1 << 0));
                fn_800269E8(obj->mdlId[i + 0x5A], (1 << 0));
                lbl_1_bss_14++;
            }
        }
        lbl_1_bss_20++;
    } else if (lbl_1_bss_154 == 5 && lbl_1_bss_18 <= 1) {
        timerStage = 2 - ((fn_80071B58() / 60U) / 10U);
        timerStage = timerStage < 0 ? 0 : timerStage > 2 ? 2 : timerStage;
        if (lbl_1_bss_14 < timerStage + 4) {
            attempts = 0;
            {
            s32 quadrant[4] = {0, 2, 10, 8};
            if (lbl_1_bss_24 == 0) {
                lbl_1_bss_24++;
                lbl_1_bss_110 = rand8() % 4;
                i = ((rand8() % 2) * 4) + ((rand8() % 2) + quadrant[lbl_1_bss_110]);
            } else if (lbl_1_bss_24 == 1) {
                lbl_1_bss_24++;
                i = ((rand8() % 2) * 4) + ((rand8() % 2) + quadrant[(lbl_1_bss_110 + 2) % 4]);
            } else {
                while (attempts < 0x78) {
                    i = rand8() % 16;
                    if (lbl_1_bss_244[i].state == 0 ||
                        lbl_1_bss_244[i].state == 3) {
                        if (lbl_1_bss_244[i].activationTime >
                            (attempts < 0x1E ? 0x78 - attempts * 4 : 0)) {
                            break;
                        }
                    }
                    attempts++;
                }
            }
            }
            if (attempts < 0x78) {
                if (lbl_1_bss_244[i].state == 3) {
                    lbl_1_bss_1C--;
                }
                lbl_1_bss_244[i].animationTime = lbl_1_bss_244[i].routeTime =
                    timerStage == 0 ? 0xB4 : timerStage == 1 ? 0xB4 : 0xB4;
                lbl_1_bss_244[i].state = 1;
                lbl_1_bss_244[i].eventState = 0;
                if (lbl_1_bss_184[2].count < 3) {
                    lbl_1_bss_184[2].x[lbl_1_bss_184[2].count] =
                        lbl_1_bss_244[i].position.x;
                    lbl_1_bss_184[2].z[lbl_1_bss_184[2].count] =
                        lbl_1_bss_244[i].position.y;
                    lbl_1_bss_184[2].count++;
                }
                fn_8002D414(obj->mdlId[i + 0x3A], 0.0f);
                fn_80026A44(obj->mdlId[i + 0x3A], (1 << 0));
                fn_8002D414(obj->mdlId[i + 0x4A], 0.0f);
                fn_80026A44(obj->mdlId[i + 0x4A], (1 << 0));
                fn_800269E8(obj->mdlId[i + 0x5A], (1 << 0));
                lbl_1_bss_14++;
                lbl_1_bss_18 = (rand8() % (0x32 - timerStage * 0xF)) + 0xA;
                if (lbl_1_bss_28 == 0) {
                    lbl_1_bss_28 = 1;
                    lbl_1_bss_18 = rand8() % 8;
                }
            }
            lbl_1_bss_20 = 0;
        } else {
            lbl_1_bss_18 += (rand8() % 8) + 0xA;
        }
    }
    lbl_1_bss_18 -= lbl_1_bss_18 > 0;
    lbl_1_data_F4 -= lbl_1_data_F4 > 0;

    for (j = 0; j < 4; j++) {
        MGPLAYER *player = lbl_1_bss_158[j];
        if (((u16 *)player->work)[2] != 0U) {
            HuVecF playerPosition;

            fn_80086BC8(player->actor, &playerPosition);
            player->work[2] = 0;
            for (i = 0; i < 16; i++) {
                if (lbl_1_bss_244[i].state != 0 && lbl_1_bss_244[i].state != 3 &&
                    lbl_1_bss_154 == 5 &&
                    lbl_1_bss_244[i].position.x - (lbl_1_bss_244[i].position.z / 2.0f) - 16.666666f <= playerPosition.x &&
                    playerPosition.x <= 16.666666f +
                        (lbl_1_bss_244[i].position.x + (lbl_1_bss_244[i].position.z / 2.0f)) &&
                    lbl_1_bss_244[i].position.y - (lbl_1_bss_244[i].position.z / 2.0f) - 16.666666f <= playerPosition.z &&
                    playerPosition.z <= 16.666666f +
                        (lbl_1_bss_244[i].position.y + (lbl_1_bss_244[i].position.z / 2.0f))) {
                    lbl_1_bss_244[i].eventState = 1;
                    playerData[j]->score +=
                        playerData[j]->score < 0x63;
                    if (lbl_1_data_20[j] != 0) {
                        MgScoreValueSet(lbl_1_data_20[j],
                            playerData[j]->score);
                    }
                    playerData[j]->scoreScale = 1.0f;
                }
            }
        }
    }
    for (j = 0; j < 4; j++) {
        playerData[j]->clearedValues[0] =
            playerData[j]->clearedValues[1] =
                playerData[j]->clearedValues[2] = 0;
    }
    for (i = 0; i < 16; i++) {
        s32 horizontalSpeed = 0;
        s32 depthSpeed = 0;
        s32 heightSpeed = 0;

        lbl_1_bss_244[i].activationTime += lbl_1_bss_244[i].activationTime < 0x8000;
        if (lbl_1_bss_244[i].eventState != 0 && lbl_1_bss_244[i].state == 1) {
            lbl_1_bss_244[i].animationTime = lbl_1_bss_244[i].routeTime = 0x1E;
            lbl_1_bss_244[i].state = 2;
            lbl_1_bss_244[i].eventState = 0;
            fn_8002D414(obj->mdlId[i + 0x5A], 0.0f);
            fn_80026A44(obj->mdlId[i + 0x5A], (1 << 0));
            fn_8002D414(obj->mdlId[i + 0x4A], 0.0f);
            fn_80026A44(obj->mdlId[i + 0x4A], (1 << 0));
            if (lbl_1_bss_184[3].count < 3) {
                lbl_1_bss_184[3].x[lbl_1_bss_184[3].count] =
                    lbl_1_bss_244[i].position.x;
                lbl_1_bss_184[3].z[lbl_1_bss_184[3].count] =
                    lbl_1_bss_244[i].position.y;
                lbl_1_bss_184[3].count++;
            }
        }
        if ((lbl_1_bss_244[i].state == 1 || lbl_1_bss_244[i].state == 2) &&
            (lbl_1_bss_244[i].animationTime <= 1 ||
                (lbl_1_bss_244[i].state == 1 && lbl_1_bss_154 == 6))) {
            lbl_1_bss_244[i].animationTime = lbl_1_bss_244[i].routeTime =
                lbl_1_bss_244[i].state = 0;
            lbl_1_bss_244[i].activationTime = 0;
            fn_8002D414(obj->mdlId[i + 0x3A], 0.0f);
            fn_800269E8(obj->mdlId[i + 0x3A], (1 << 0));
            fn_800269E8(obj->mdlId[i + 0x4A], (1 << 0));
            fn_800269E8(obj->mdlId[i + 0x5A], (1 << 0));
            lbl_1_bss_14--;
        } else if (lbl_1_bss_244[i].state == 3 && lbl_1_bss_244[i].animationTime <= 0) {
            lbl_1_bss_244[i].animationTime = lbl_1_bss_244[i].routeTime =
                lbl_1_bss_244[i].state = 0;
            lbl_1_bss_1C--;
        } else if (lbl_1_bss_244[i].state == 3) {
            lbl_1_bss_244[i].animationTime -= lbl_1_bss_244[i].animationTime > 0;
        } else if (lbl_1_bss_244[i].state == 1) {
            double progress = (float)lbl_1_bss_244[i].animationTime /
                (float)lbl_1_bss_244[i].routeTime;
            fn_8002D414(obj->mdlId[i + 0x3A],
                (1.0f - ((float)lbl_1_bss_244[i].animationTime / (float)lbl_1_bss_244[i].routeTime)) * fn_8002D530(
                    obj->mdlId[i + 0x3A]));
            fn_8002D414(obj->mdlId[i + 0x4A],
                (0.25f + 0.4f * (1.0f - ((float)lbl_1_bss_244[i].animationTime / (float)lbl_1_bss_244[i].routeTime))) *
                    fn_8002D530(obj->mdlId[i + 0x4A]));
            lbl_1_bss_244[i].animationTime -= lbl_1_bss_244[i].animationTime > 0;
            if (progress >= 0.25) {
                horizontalSpeed = (s32)(192.0 * ((0.75 +
                    (0.25 * (progress - 0.25)) / 0.75) * (((i + i / 4) % 2) ?
                    0.8 : 1.0)));
                depthSpeed = (s32)(192.0 * ((0.75 +
                    (0.25 * (progress - 0.25)) / 0.75) * (((i + i / 4) % 2) ?
                    0.8 : 0.1)));
                heightSpeed = 0;
            } else {
                horizontalSpeed = (s32)(192.0 * (((0.75 * progress) / 0.25) * (((i + i / 4) % 2) ?
                    0.8 : 1.0)));
                depthSpeed = (s32)(192.0 * (((0.75 * progress) / 0.25) * (((i + i / 4) % 2) ?
                    0.8 : 0.1)));
                heightSpeed = (s32)(192.0 *
                    (0.5 - 0.5 * progress / 0.25));
            }
        } else if (lbl_1_bss_244[i].state == 2) {
            double hitProgress = (float)lbl_1_bss_244[i].animationTime /
                (float)lbl_1_bss_244[i].routeTime;
            fn_8002D414(obj->mdlId[i + 0x3A], 0.0f);
            fn_8002D414(obj->mdlId[i + 0x4A],
                (1.0f - ((float)lbl_1_bss_244[i].animationTime / (float)lbl_1_bss_244[i].routeTime)) * fn_8002D530(
                    obj->mdlId[i + 0x4A]));
            lbl_1_bss_244[i].animationTime -= lbl_1_bss_244[i].animationTime > 0;
            horizontalSpeed = (s32)(192.0 * (hitProgress *
                (((i + i / 4) % 2) ? 0.8 : 1.0)));
            depthSpeed = (s32)(192.0 * (hitProgress *
                (((i + i / 4) % 2) ? 0.8 : 0.1)));
            heightSpeed = (s32)(192.0 * (0.25 * hitProgress));
        }
        for (j = 0; j < 4; j++) {
            float nearestX;
            float nearestZ;
            float distance;
            nearestX = lbl_1_bss_16C[j]->trans.x < lbl_1_bss_244[i].position.x - lbl_1_bss_244[i].position.z / 4.0f ?
                lbl_1_bss_244[i].position.x - lbl_1_bss_244[i].position.z / 4.0f :
                (lbl_1_bss_16C[j]->trans.x > lbl_1_bss_244[i].position.x + lbl_1_bss_244[i].position.z / 4.0f ?
                 lbl_1_bss_244[i].position.x + lbl_1_bss_244[i].position.z / 4.0f : lbl_1_bss_16C[j]->trans.x);
            distance = (lbl_1_bss_16C[j]->trans.x - nearestX) * (lbl_1_bss_16C[j]->trans.x - nearestX);
            nearestZ = lbl_1_bss_16C[j]->trans.z < lbl_1_bss_244[i].position.y - lbl_1_bss_244[i].position.z / 4.0f ?
                lbl_1_bss_244[i].position.y - lbl_1_bss_244[i].position.z / 4.0f :
                (lbl_1_bss_16C[j]->trans.z > lbl_1_bss_244[i].position.y + lbl_1_bss_244[i].position.z / 4.0f ?
                 lbl_1_bss_244[i].position.y + lbl_1_bss_244[i].position.z / 4.0f : lbl_1_bss_16C[j]->trans.z);
            distance += (lbl_1_bss_16C[j]->trans.z - nearestZ) * (lbl_1_bss_16C[j]->trans.z - nearestZ);
            distance = sqrtf(distance);
            distance = distance < 0.0f ? 1.0f : distance > 150.0f ? 0.0f :
                1.0f - distance / 150.0f;
            playerData[j]->clearedValues[0] += (float)horizontalSpeed * distance;
            playerData[j]->clearedValues[1] += (float)depthSpeed * distance;
            playerData[j]->clearedValues[2] += (float)heightSpeed * distance;
        }
    }
    for (j = 0; j < 4; j++) {
        playerData[j]->clearedValues[0] = playerData[j]->clearedValues[0] > 0xFF ? 0xFF : playerData[j]->clearedValues[0];
        playerData[j]->clearedValues[1] = playerData[j]->clearedValues[1] > 0xFF ? 0xFF : playerData[j]->clearedValues[1];
        playerData[j]->clearedValues[2] = playerData[j]->clearedValues[2] > 0xFF ? 0xFF : playerData[j]->clearedValues[2];
        fn_80028E48(*lbl_1_bss_16C[j]->mdlId, playerData[j]->firstLight,
            (u8)playerData[j]->clearedValues[0], (u8)playerData[j]->clearedValues[1],
            (u8)playerData[j]->clearedValues[2], 0xFF);
    }
    if (lbl_1_bss_10 != 0) {
        if (lbl_1_data_F8[0] == -1) {
        for (i = 0; i < 12; i++) {
            lbl_1_data_F8[i] = (s32)(62.0f +
                    170.0f * ((float)rand8() / 255.0f));
            }
        }
        for (i = 0; i < 12; i++) {
            if (lbl_1_bss_154 > 2 ||
                (lbl_1_bss_154 == 2 && lbl_1_bss_0 >= 0x69)) {
                if (lbl_1_data_F8[i] == 0x3D) {
                    fn_80026A44(obj->mdlId[i + 0x11], ((1 << 30) | (1 << 1)));
                    if (lbl_1_bss_184[5].count < 3) {
                        lbl_1_bss_184[5].x[lbl_1_bss_184[5].count] =
                            lbl_1_data_58[i].x;
                        lbl_1_bss_184[5].z[lbl_1_bss_184[5].count] =
                            lbl_1_data_58[i].y;
                        lbl_1_bss_184[5].count++;
                    }
                } else if (lbl_1_data_F8[i] == 1) {
                    fn_8002C95C(obj->mdlId[i + 0x11], obj->mtnId[0]);
                    fn_800269E8(obj->mdlId[i + 0x11], ((1 << 30) | (1 << 1)));
                }
                lbl_1_data_F8[i] -= lbl_1_data_F8[i] > 0;
            }
        }
    }
    if (lbl_1_bss_154 == 5 ||
        (lbl_1_bss_154 >= 8 && lbl_1_bss_C == 0)) {
        if (lbl_1_data_158 <= 0 &&
            (lbl_1_bss_2C < 3 ||
                (lbl_1_bss_154 >= 8 && lbl_1_bss_C == 0))) {
            for (j = -0x12C; j <= 0; j += 0xA) {
                i = rand8() % 12;
                if (lbl_1_data_128[i] <= j) {
                    lbl_1_data_128[i] = 0x1E;
                    fn_8002D414(obj->mdlId[i + 0x11], 0.0f);
                    fn_80026A44(obj->mdlId[i + 0x11],
                        ((1 << 30) | (1 << 1)));
                    if (lbl_1_bss_154 >= 8 && lbl_1_bss_C == 0) {
                        fn_800269E8(obj->mdlId[i + 0x11],
                            ((1 << 30) | (1 << 0)));
                        lbl_1_data_128[i] = 0x7530;
                    }
                    lbl_1_bss_2C++;
                    break;
                }
            }
            lbl_1_data_158 = (s32)((float)lbl_1_data_158 +
                (10.0f + 50.0f * ((float)rand8() / 255.0f)));
            if (lbl_1_bss_154 >= 8 && lbl_1_bss_C == 0) {
                lbl_1_data_158 = 0;
            }
        }
        lbl_1_data_158 -= lbl_1_data_158 > 0;
        for (i = 0; i < 12; i++) {
            lbl_1_data_128[i] -= lbl_1_data_128[i] > -0x4000;
            if (lbl_1_data_128[i] == 0) {
                fn_8002D414(obj->mdlId[i + 0x11], 0.0f);
                fn_800269E8(obj->mdlId[i + 0x11], ((1 << 30) | (1 << 1)));
                lbl_1_bss_2C--;
            }
        }
    }
    for (i = 0; i < 12; i++) {
        if (fn_8002D4E0(obj->mdlId[i + 0x11]) == 26.0f &&
            lbl_1_bss_184[4].count < 3) {
            lbl_1_bss_184[4].x[lbl_1_bss_184[4].count] =
                lbl_1_data_58[i].x;
            lbl_1_bss_184[4].z[lbl_1_bss_184[4].count] =
                lbl_1_data_58[i].y;
            lbl_1_bss_184[4].count++;
        }
    }
}

void fn_1_5610(s16 mode, s16 frameNo)
{
}

void fn_1_5614(s16 mode, s16 frameNo)
{
}

void fn_1_5618(s16 mode, s16 frameNo)
{
}

void fn_1_561C(s16 mode, s16 frameNo)
{
}

void fn_1_5620(s16 mode, s16 frameNo)
{
}

void fn_1_5624(s16 mode, s16 frameNo)
{
}

void fn_1_5628(s16 mode, s16 frameNo)
{
    if (frameNo >= 60) {
        fn_80071A9C();
    }
}

extern s32 lbl_1_data_10;

extern s16 lbl_1_data_18[4];

extern s32 lbl_1_data_8;

/* Reconstruction of removed debug code. */
static inline void DebugReportPlayerPositions(int frameNo, int enabled, float *radiusSquared)
{
    float x, z;
    float targetX, targetZ;
    float targetRadiusSquared;
    float dx, dz;
    int player, other;
    OMOBJ *playerObject;
    M601PlayerControlWork *playerWork;
    int frame;

    if (enabled) {
        frame = frameNo;
        for (player = 0; player < 4; player++) {
            playerObject = lbl_1_bss_16C[player];
            playerWork = lbl_1_bss_16C[player]->data;
            targetX = playerWork->targetX;
            targetZ = playerWork->targetZ;

            for (other = player + 1; other < 4; other++) {
                x = playerObject->trans.x;
                z = playerObject->trans.z;
                dx = x - lbl_1_bss_16C[other]->trans.x;
                dz = z - lbl_1_bss_16C[other]->trans.z;
                *radiusSquared = x * x + z * z;
                targetRadiusSquared = targetX * targetX + targetZ * targetZ;
                OSReport(
                    "[%d] player %d peer %d object %p position (%f,%f) target (%f,%f) delta (%f,%f)\n"
                    "[%d] player %d peer %d radius2 %f -> %f; change %f; ratio %f\n"
                    "[%d] player %d peer %d movement (%f,%f) length2 %f projection %f cross %f\n"
                    "[%d] peer %d position2 %f target2 %f dot %f cross %f\n"
                    "[%d] peer %d midpoint (%f,%f), offset (%f,%f)\n",
                    frame, player, other, playerObject, x, z, targetX, targetZ, dx, dz,
                    frame, player, other, *radiusSquared, targetRadiusSquared,
                    targetRadiusSquared - *radiusSquared,
                    targetRadiusSquared / *radiusSquared,
                    frame, player, other, targetX - x, targetZ - z,
                    (targetX - x) * (targetX - x) + (targetZ - z) * (targetZ - z),
                    (targetX - x) * dx + (targetZ - z) * dz,
                    (targetX - x) * dz - (targetZ - z) * dx,
                    frame, other, x * x + z * z, targetX * targetX + targetZ * targetZ,
                    x * targetX + z * targetZ, x * targetZ - z * targetX,
                    frame, other, (x + targetX) * 0.5f, (z + targetZ) * 0.5f,
                    x - dx * 0.5f, z - dz * 0.5f);
                x = (targetX - dx) * (targetX - dx);
                z = (targetZ - dz) * (targetZ - dz);

                *radiusSquared /= 125.0f * 125.0f;
                *radiusSquared -= 1.0f;
                *radiusSquared *= *radiusSquared;
                *radiusSquared *= 100.0f;
                targetRadiusSquared /= 125.0f * 125.0f;
                targetRadiusSquared -= 1.0f;
                targetRadiusSquared *= targetRadiusSquared;
                targetRadiusSquared *= 100.0f;
                OSReport(
                    "translated peer components2 (%f,%f) total2 %f\n"
                    "squared relative distance errors (%%) %f -> %f\n",
                    x, z, x + z,
                    *radiusSquared, targetRadiusSquared);
            }
        }
    }
}

u8 m601UnknownData00[4] = { 255, 255, 0, 0 };
s32 lbl_1_data_4 = -1;
s32 lbl_1_data_8 = -1;
s32 lbl_1_data_C = -1;
s32 lbl_1_data_10 = -1;
s32 lbl_1_data_14 = 1;
s16 lbl_1_data_18[4] = { 0, 0, 0, 0 };
MGSCORE *lbl_1_data_20[4] = { 0, 0, 0, 0 };
MGSEQ_PARAM lbl_1_data_30 = {30, 0, fn_1_A2AC, fn_1_5610, fn_1_5614, fn_1_5618, fn_1_561C, fn_1_5620, fn_1_5624, fn_1_5628, fn_1_A3B0};
Point3d lbl_1_data_58[12] = { { -530.0f, -830.0f, 0.0f }, { -305.0f, -830.0f, 0.0f }, { 305.0f, -830.0f, 0.0f }, { 530.0f, -830.0f, 0.0f }, { 830.0f, -530.0f, -90.0f }, { 830.0f, -305.0f, -90.0f }, { 830.0f, 305.0f, -90.0f }, { 830.0f, 530.0f, -90.0f }, { -830.0f, -530.0f, 90.0f }, { -830.0f, -305.0f, 90.0f }, { -830.0f, 305.0f, 90.0f }, { -830.0f, 530.0f, 90.0f } };
char lbl_1_data_E8[9] = "%f,%f,%f";
char lbl_1_data_F1[3] = "%f";
s32 lbl_1_data_F4 = 60;
s32 lbl_1_data_F8[12] = { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
s32 lbl_1_data_128[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
s32 lbl_1_data_158 = 100;

/* Updates player positions, results, and score displays for the current sequence. */
void fn_1_5658(OMOBJ *obj) {
    float debugSeparationSquared;
    s32 layoutIndex;
    s32 resultLayoutIndex;
    s32 frameNo = fn_80071DE4();
    s32 positionsB[4][4] = {
        -300, -100, 100, 300, 0, -300, 0, 300,
        -150, 150, -150, 150, -300, 0, 300, 0
    };
    s32 positionsA[4][4] = {
        0, 0, 0, 0, 300, 0, 0, 0,
        150, 150, -150, -150, 150, 150, 150, -150
    };
    /* The 0x1C bytes after the position table have no known use. */
    static const u8 lbl_1_rodata_1F0[0x1C] = { 0 };
    Point3d playerPositions[4];
    Point3d resolvedPositions[4];
    Mtx motionMatrix;
    M601PlayerControlWork *playerObjects[4];
    f32 playerDistance;
    f32 collisionDistance;
    s32 soundHandle;
    s32 placementIndex;
    HU3D_MODEL *playerModel;
    f32 separationX;
    f32 separationZ;
    s32 soundValue;
    int movementX;
    int movementZ;
    s32 otherPlayer;
    s32 playerIndex;
    s32 resultPlayer;
    M601SoundEntry *soundEntry;

    lbl_1_bss_154 = fn_80071A8C();
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        playerObjects[playerIndex] = lbl_1_bss_16C[playerIndex]->data;
    }
    fn_800856A8();
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        playerModel = &lbl_802C01C8[lbl_1_bss_16C[playerIndex]->mdlId[0]];
        fn_80086BC8(lbl_1_bss_158[playerIndex]->actor, &playerPositions[playerIndex]);
        lbl_1_bss_16C[playerIndex]->trans.x = playerPositions[playerIndex].x;
        lbl_1_bss_16C[playerIndex]->trans.y = playerPositions[playerIndex].y;
        lbl_1_bss_16C[playerIndex]->trans.z = playerPositions[playerIndex].z;
        lbl_1_bss_16C[playerIndex]->rot.y = lbl_1_bss_158[playerIndex]->actor->rotY;
        lbl_1_bss_16C[playerIndex]->scale.x = playerModel->scale.x;
        lbl_1_bss_16C[playerIndex]->scale.y = playerModel->scale.y;
        lbl_1_bss_16C[playerIndex]->scale.z = playerModel->scale.z;
        (playerObjects[playerIndex])->startX = playerPositions[playerIndex].x;
        (playerObjects[playerIndex])->startZ = playerPositions[playerIndex].z;
    }
    if ((lbl_1_bss_154 == 7) && (lbl_1_data_4 != -1)) {
        s32 collisionFlags[4] = {0, 0, 0, 0};
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            static const Point3d zeroMotionVector = { 0.0f, 0.0f, 0.0f };
            Point3d motionVector;
            movementX = 0;
            movementZ = 0;
            layoutIndex = (lbl_1_bss_4.winnerCount == 0 || lbl_1_bss_4.winnerCount == 4) ? 0 : lbl_1_bss_4.winnerCount;
            placementIndex = playerObjects[playerIndex]->resultPlacement >= 0 ?
                playerObjects[playerIndex]->resultPlacement :
                layoutIndex + (-playerObjects[playerIndex]->resultPlacement - 1);
            playerObjects[playerIndex]->collisionPriority = (3 - playerIndex) +
                (playerObjects[playerIndex]->motionState == 0 ? 0xA : 0);
            (playerObjects[playerIndex])->targetX = (f32)positionsB[layoutIndex][placementIndex];
            (playerObjects[playerIndex])->targetZ = (f32)positionsA[layoutIndex][placementIndex];
            movementX = (s32)(5.6f * ((f32)positionsB[layoutIndex][placementIndex] - playerPositions[playerIndex].x));
            movementZ = -(s32)(5.6f * ((f32)positionsA[layoutIndex][placementIndex] - playerPositions[playerIndex].z));
            fn_1_2DC(&movementX, &movementZ, 56.0f);
            motionVector = zeroMotionVector;
            motionVector.x = (f32) movementX;
            motionVector.z = (f32) movementZ;
            PSMTXRotRad(motionMatrix, 'y', (f32) MTXDegToRad((f64) ((playerIndex * 3) + 0x1E) * sin(M_PI*((f64) (1.25f * (f32) (playerObjects[playerIndex])->collisionFrames))/180.0)));
            PSMTXMultVec(motionMatrix, &motionVector, &motionVector);
            movementX = (s32) motionVector.x;
            movementZ = (s32) motionVector.z;
            resolvedPositions[playerIndex].x = ((f32)movementX / 5.6f) + playerPositions[playerIndex].x;
            resolvedPositions[playerIndex].y = 0.0f;
            resolvedPositions[playerIndex].z = playerPositions[playerIndex].z - ((f32)movementZ / 5.6f);
        }
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            for (otherPlayer = 0; otherPlayer < 4; otherPlayer++) {
                collisionDistance = 125.0f;
                if (playerIndex != otherPlayer) {
                separationX = resolvedPositions[playerIndex].x - resolvedPositions[otherPlayer].x;
                separationZ = resolvedPositions[playerIndex].z - resolvedPositions[otherPlayer].z;
                    playerDistance = sqrtf(separationX * separationX + separationZ * separationZ);
                    if (playerDistance < collisionDistance) {
                        if ((s32) (playerObjects[playerIndex])->collisionPriority < (s32) (playerObjects[otherPlayer])->collisionPriority) {
                            resolvedPositions[otherPlayer].x = (collisionDistance * (-separationX / playerDistance)) + resolvedPositions[playerIndex].x;
                            resolvedPositions[otherPlayer].z = (collisionDistance * (-separationZ / playerDistance)) + resolvedPositions[playerIndex].z;
                            collisionFlags[playerIndex] = collisionFlags[otherPlayer] = 1;
                        } else {
                            resolvedPositions[playerIndex].x = (collisionDistance * (separationX / playerDistance)) + resolvedPositions[otherPlayer].x;
                            resolvedPositions[playerIndex].z = (collisionDistance * (separationZ / playerDistance)) + resolvedPositions[otherPlayer].z;
                            collisionFlags[playerIndex] = collisionFlags[otherPlayer] = 1;
                        }
                    }
                }
            }
        }
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            (playerObjects[playerIndex])->startX = resolvedPositions[playerIndex].x;
            (playerObjects[playerIndex])->startZ = resolvedPositions[playerIndex].z;
            (playerObjects[playerIndex])->collisionFrames = collisionFlags[playerIndex] != 0 ? playerObjects[playerIndex]->collisionFrames + 1 : 0;
        }
    }
    for (playerIndex = 0; playerIndex < 6; playerIndex++) {
        s32 soundTable[6] = {0x61B, 0x61C, 0x61D, 0x61E, 0x61F, 0x620};
        if ((s32) lbl_1_bss_184[playerIndex].count > 3) {
            lbl_1_bss_184[playerIndex].count = 3;
        }
        if ((playerIndex == 2) && ((s32) lbl_1_bss_184[playerIndex].count > 1)) {
            lbl_1_bss_184[playerIndex].count = 1;
        }
        if ((s32) lbl_1_bss_184[playerIndex].count > 0) {
            if ((s32) soundTable[playerIndex] != -1) {
                soundHandle = HuAudFXPlay(soundTable[playerIndex]);
                lbl_1_bss_184[playerIndex].handle = soundHandle;
                HuAudFXPanning(soundHandle, (s16) (0.5 + (f64) (64.0f + (63.0f * (0.25f * (lbl_1_bss_184[playerIndex].x[lbl_1_bss_184[playerIndex].count - 1] / 440.0f))))));
                soundValue = (s32) (0.5 + (f64) (104.0f + (23.0f * (1.0f + (lbl_1_bss_184[playerIndex].z[lbl_1_bss_184[playerIndex].count - 1] / 550.0f)))));
                soundValue = soundValue < 0 ? 0 : (soundValue >= 0x7F ? 0x7F : soundValue);
            }
            lbl_1_bss_184[playerIndex].count--;
        }
    }
    switch (lbl_1_bss_154) {
    case 1:
        if (WipeCheck() == 0) {
            fn_80071A9C();
        }
    case 0:
    case 4:
    case 8:
    case 9:
    case 10:
    case 11:
    default:
        break;
    case 2:
        if (frameNo == 1) {
            HuAudFXPlay(lbl_1_bss_10 == 0 ? 0x61B : 0x61C);
            return;
        }
        break;
    case 3:
        if ((lbl_1_data_10 == -1) && ((s32) (GameMesStatGet(fn_80071DF4()) & 0x10) != 0)) {
            lbl_1_data_10 = HuAudBGMPlay(MSM_STREAM_JAZZY);
            return;
        }
        break;
    case 5:
        if (frameNo == 1) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                /* Copied from .rodata+0x240; no later read is known. */
                s32 lbl_1_rodata_240[11] = { 0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 8 };
                s32 scorePositions[4][2] = {60, 59, 176, 59, 400, 59, 516, 59};
                lbl_1_data_18[playerIndex] = MgScoreBoxCreateChar(0x60, 0x30, (s16) (playerObjects[playerIndex])->characterNo);
                MgScoreBoxPosSet(lbl_1_data_18[playerIndex], (f32) scorePositions[playerIndex][0], (f32) scorePositions[playerIndex][1]);
                lbl_1_data_20[playerIndex] = MgScoreCreate(DATANUM(0x9b0000, 49), -1, 1);
                MgScoreUnitBankSet(lbl_1_data_20[playerIndex], 0x10);
                MgScoreMaxDigitSet(lbl_1_data_20[playerIndex], 2);
                MgScorePosSet(lbl_1_data_20[playerIndex], (f32) (scorePositions[playerIndex][0] + 8), (f32) (scorePositions[playerIndex][1] + 1));
                MgScoreColorSet(lbl_1_data_20[playerIndex], 0xFFU, 0xD8U, 0x15U);
            }
            return;
        }
        break;
    case 6:
        if ((lbl_1_data_10 != -1) && ((s32) (GameMesStatGet(fn_80071DF4()) & 0x10) != 0)) {
            HuAudSStreamFadeOut(lbl_1_data_10, 0x64);
            lbl_1_data_10 = -1;
        }
        if (lbl_1_data_4 == -1) {
            M601PlayerControlWork *resultWork[4];
            s32 loserCount = 0;
            s32 resultPosition;
            s32 playerOrder[4] = {-1, -1, -1, -1};
            f32 distanceSquared[4];
            lbl_1_bss_4.winnerCount = lbl_1_data_8 = 0;
            for (resultPlayer = 0; resultPlayer < 4; resultPlayer++) {
                (&resultWork[0])[resultPlayer] = lbl_1_bss_16C[resultPlayer]->data;
                if ((s32) ((&resultWork[0])[resultPlayer])->score > lbl_1_data_8) {
                    lbl_1_data_8 = ((&resultWork[0])[resultPlayer])->score;
                }
                ((&resultWork[0])[resultPlayer])->resultPlacement = -1;
            }
            for (resultPlayer = 0; resultPlayer < 4; resultPlayer++) {
                if (lbl_1_data_8 == (s32) ((&resultWork[0])[resultPlayer])->score) {
                    ((&resultWork[0])[resultPlayer])->resultPlacement = lbl_1_bss_4.winnerCount;
                    playerOrder[lbl_1_bss_4.winnerCount] = resultPlayer;
                    lbl_1_bss_4.winnerCount += 1;
                } else {
                    loserCount += 1;
                    ((&resultWork[0])[resultPlayer])->resultPlacement = (s32) -loserCount;
                }
            }
            if (lbl_1_data_8 == 0) {
                lbl_1_data_4 = 4;
            } else {
                lbl_1_data_4 = 0;
            }
            if (lbl_1_data_4 != 4) {
                fn_80071C18((s16)(playerOrder[0] == -1 ? -1 : ((&resultWork[0])[playerOrder[0]])->characterNo), (s16)(playerOrder[1] == -1 ? -1 : ((&resultWork[0])[playerOrder[1]])->characterNo), (s16)(playerOrder[2] == -1 ? -1 : ((&resultWork[0])[playerOrder[2]])->characterNo), (s16)(playerOrder[3] == -1 ? -1 : ((&resultWork[0])[playerOrder[3]])->characterNo));
            } else {
                fn_80071C18(-1, -1, -1, -1);
            }
            if (lbl_1_data_4 != 4) {
                for (resultPlayer = 0; resultPlayer < 4; resultPlayer++) {
                    if (playerOrder[resultPlayer] != -1) {
                        GWMgCoinBonusSet(resultWork[playerOrder[resultPlayer]]->configNo, 0xA);
                    }
                }
            }
            for (resultPlayer = 0; resultPlayer < 4; resultPlayer++) {
                resultLayoutIndex = (lbl_1_bss_4.winnerCount == 0 || lbl_1_bss_4.winnerCount == 4) ? 0 : lbl_1_bss_4.winnerCount;
                resultPosition = resultWork[resultPlayer]->resultPlacement >= 0 ?
                    resultWork[resultPlayer]->resultPlacement :
                    resultLayoutIndex + (-resultWork[resultPlayer]->resultPlacement - 1);
                (&distanceSquared[0])[resultPlayer] = ((f32)positionsB[resultLayoutIndex][resultPosition] - lbl_1_bss_16C[resultPlayer]->trans.x) * ((f32)positionsB[resultLayoutIndex][resultPosition] - lbl_1_bss_16C[resultPlayer]->trans.x);
                (&distanceSquared[0])[resultPlayer] += ((f32)positionsA[resultLayoutIndex][resultPosition] - lbl_1_bss_16C[resultPlayer]->trans.z) * ((f32)positionsA[resultLayoutIndex][resultPosition] - lbl_1_bss_16C[resultPlayer]->trans.z);
                ((&resultWork[0])[resultPlayer])->collisionPriority = resultPlayer;
            }
            for (resultPlayer = 0; resultPlayer < 4; resultPlayer++) {
                    for (otherPlayer = 0; otherPlayer < 4; otherPlayer++) {
                }
            }
            return;
        }
        break;
    case 7:
        fn_80071B08();
        if (((frameNo >= 0x46) && ((s32) (playerObjects[0])->motionState != 0) && ((s32) (playerObjects[1])->motionState != 0) && ((s32) (playerObjects[2])->motionState != 0) && ((s32) (playerObjects[3])->motionState != 0)) || (frameNo >= 0x168)) {
            fn_80071B30();
            fn_80071DEC(1U);
            if (lbl_1_bss_C != 0) {
                fn_80071AC4(9);
                for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                    GWMgScoreSet(playerObjects[playerIndex]->configNo, playerObjects[playerIndex]->score);
                }
                return;
            }
            fn_80071A9C();
        }
        break;
    }
DebugReportPlayerPositions(frameNo, 0, &debugSeparationSquared);
}

/* Draws the six-column, 64-row checker pattern used by the grid display. */

unsigned int lbl_1_data_18C[16] = { 0x930000, 0x930001, 0x930002, 0x930003, 0x930004, 0x930006, 0x930028, 0x930008, 0x93000A, 0x930009, 0x930018, 0x930017, 0x930011, 0x930012, 0x930016, 0 };

void fn_1_71E8(HU3D_MODEL *model, Mtx *matrix)
{
    Mtx identity;
    Mtx projection;
    s32 row;
    s32 column;
    s32 drawColumn;
    s32 drawRow;
    s32 selectedAlpha;
    s32 alpha = 0;

    GXLoadPosMtxImm(*matrix, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetZMode(0, GX_ALWAYS, 0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    GXSetZMode(0, GX_ALWAYS, 0);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetZCompLoc(0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0);
    C_MTXOrtho(projection, 0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, -1.0f);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    PSMTXIdentity(identity);
    GXLoadPosMtxImm(identity, GX_PNMTX0);
    for (row = 0; row < 0x40; row++) {
        for (column = 0; column < 6; column++) {
            s32 rowWidths[6] = { 1, 2, 4, 8, 16, 32 };
            drawColumn = column;
            drawRow = row;
            if ((row % rowWidths[column] * 2) / rowWidths[column] != 0) {
                selectedAlpha = 0;
            } else {
                selectedAlpha = 0xFF;
            }
            alpha = selectedAlpha;
            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            GXPosition3f32((float)drawColumn / 160.0f,
                (float)drawRow / 480.0f, 0.0f);
            GXColor4u8(alpha, alpha, alpha, 0xFF);
            GXTexCoord2f32(0.0f, 0.0f);
            GXPosition3f32((float)(drawColumn + 1) / 160.0f,
                (float)drawRow / 480.0f, 0.0f);
            GXColor4u8(alpha, alpha, alpha, 0xFF);
            GXTexCoord2f32(1.0f, 0.0f);
            GXPosition3f32((float)(drawColumn + 1) / 160.0f,
                (float)(drawRow + 1) / 480.0f, 0.0f);
            GXColor4u8(alpha, alpha, alpha, 0xFF);
            GXTexCoord2f32(1.0f, 1.0f);
            GXPosition3f32((float)drawColumn / 160.0f,
                (float)(drawRow + 1) / 480.0f, 0.0f);
            GXColor4u8(alpha, alpha, alpha, 0xFF);
            GXTexCoord2f32(0.0f, 1.0f);
        }
    }
}

/* Draws the two full-screen blend quads used by the minigame overlay. */
void fn_1_7888(void *unused, Mtx *matrix)
{
    Mtx identity;
    Mtx projection;
    u32 color = 0;

    GXLoadPosMtxImm(*matrix, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetZMode(0, GX_ALWAYS, 0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_DSTCLR, GX_BL_ONE, GX_LO_NOOP);
    GXSetZMode(0, GX_ALWAYS, 0);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetZCompLoc(0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0);
    C_MTXOrtho(projection, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    PSMTXIdentity(identity);
    GXLoadPosMtxImm(identity, GX_PNMTX0);

    color = 0xFF;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(0.0f, 0.0f, 0.0f);
    GXColor4u8(color, color, color, 0xFF);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(1.0f, 0.0f, 0.0f);
    GXColor4u8(color, color, color, 0xFF);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition3f32(1.0f, 1.0f, 0.0f);
    GXColor4u8(color, color, color, 0xFF);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(0.0f, 1.0f, 0.0f);
    GXColor4u8(color, color, color, 0xFF);
    GXTexCoord2f32(0.0f, 1.0f);

    color = 0x80;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(0.0f, 0.0f, 0.0f);
    GXColor4u8(color, color, color, 0xFF);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(1.0f, 0.0f, 0.0f);
    GXColor4u8(color, color, color, 0xFF);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition3f32(1.0f, 1.0f, 0.0f);
    GXColor4u8(color, color, color, 0xFF);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(0.0f, 1.0f, 0.0f);
    GXColor4u8(color, color, color, 0xFF);
    GXTexCoord2f32(0.0f, 1.0f);
}

extern M601UnknownBss124 lbl_1_bss_124;

extern void *lbl_1_bss_120;

extern Mtx lbl_1_bss_30;

extern M601UnknownBss60 lbl_1_bss_60;

/* Captures the animated green strip into the minigame's texture buffer. */
void fn_1_7F8C(void)
{
    s32 frame;
    s32 green;
    s32 red;

    if (lbl_1_bss_124.flushSize != 0) {
        C_MTXOrtho(lbl_1_bss_30, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, -1.0f);
        GXSetProjection(lbl_1_bss_30, GX_ORTHOGRAPHIC);
        PSMTXIdentity(lbl_1_bss_60.positionMatrix);
        GXLoadPosMtxImm(lbl_1_bss_60.positionMatrix, GX_PNMTX0);
        GXSetNumTevStages(1);
        GXSetNumTexGens(1);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0, GX_PTIDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
        GXSetZMode(0, GX_ALWAYS, 0);
        GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 0x42);
        for (frame = 0; frame < 0x20; frame++) {
            green = (s32)(128.0 + 127.0
                * cos((M_PI * (double)(360.0f
                    * ((float)frame / 32.0f))) / 180.0));
            red = (s32)(128.0 + 127.0
                * cos((M_PI * (double)(2.0f
                    * (360.0f * ((float)frame / 32.0f))))
                    / 180.0));
            GXPosition3f32(0.0f, (float)frame / 480.0f,
                0.0f);
            GXColor4u8(0, red, green, 0xFF);
            GXPosition3f32(0.05f, (float)frame / 480.0f,
                0.0f);
            GXColor4u8(0, red, green, 0xFF);
        }
        GXPosition3f32(0.0f, (float)frame / 480.0f,
            0.0f);
        GXColor4u8(0, red, green, 0xFF);
        GXPosition3f32(0.05f, (float)frame / 480.0f,
            0.0f);
        GXColor4u8(0, red, green, 0xFF);
        GXSetCopyFilter(FALSE, RenderMode->sample_pattern, FALSE, RenderMode->vfilter);
        GXSetTexCopySrc(0, 0, 0x20, 0x20);
        GXSetTexCopyDst(0x20, 0x20, GX_CTF_GB8, FALSE);
        GXCopyTex(lbl_1_bss_120, FALSE);
        DCFlushRange(lbl_1_bss_120, lbl_1_bss_124.flushSize);
        GXPixModeSync();
        GXSetCopyFilter(RenderMode->aa, RenderMode->sample_pattern, TRUE, RenderMode->vfilter);
        lbl_1_bss_124.flushSize = 0;
    }
}

const HuVecF m601LightPosition = { 0.0f, 5000.0f, 0.0f };
const HuVecF m601LightDirectionA = { 0.1f, -1.0f, -0.4f };
const HuVecF m601LightDirectionB = { 0.0f, -1.0f, -1.0f };
const GXColor m601LightColorA = { 255, 255, 255, 255 };
const GXColor m601LightColorB = { 112, 112, 144, 255 };

void fn_1_8634(s32 modelId)
{
    s32 lightId;
    HuVecF position = m601LightPosition;
    HuVecF directionA = m601LightDirectionA;
    HuVecF directionB = m601LightDirectionB;
    GXColor colorA = m601LightColorA;
    GXColor colorB = m601LightColorB;
    GXColor *color;
    HuVecF *direction;

    if (lbl_1_bss_10 != 0) {
        color = &colorB;
    } else {
        color = &colorA;
    }
    if (lbl_1_bss_10 != 0) {
        direction = &directionB;
    } else {
        direction = &directionA;
    }
    lightId = fn_800288B4(modelId, &position, direction, color);
    fn_80028B58(modelId, lightId);
    fn_800296E0(modelId, lightId, 1);
}

static inline void m601ModelLightSetup(s32 modelId)
{
    s32 lightId;
    HuVecF position = m601LightPosition;
    HuVecF directionA = m601LightDirectionA;
    HuVecF directionB = m601LightDirectionB;
    GXColor colorA = m601LightColorA;
    GXColor colorB = m601LightColorB;

    lightId = fn_800288B4(modelId, &position,
        lbl_1_bss_10 ? &directionB : &directionA,
        lbl_1_bss_10 ? &colorB : &colorA);
    fn_80028B58(modelId, lightId);
    fn_800296E0(modelId, lightId, 1);
}

/* Creates the stage models and places their lights, markers and moving pieces. */
void fn_1_8798(OMOBJ *obj)
{
    HU3D_MODELID collisionModels[4];
    void *playerData[4];
    s32 i;
    s32 modelNo;
    s32 positionSet;
    float motionTime;

    for (i = 0; i < 4; i++) {
        playerData[i] = lbl_1_bss_16C[i]->data;
    }
    modelNo = 0;
    obj->mdlId[modelNo] = fn_80025A04(fn_80008DD0(
        lbl_1_bss_10 ? DATANUM(0x3c0000, 3) : DATANUM(0x3c0000, 2), 0x10000000, 2));
    fn_80026B8C(obj->mdlId[modelNo], 2);
    fn_80026FFC(obj->mdlId[modelNo]);
    m601ModelLightSetup(obj->mdlId[modelNo]);

    modelNo = 2;
    obj->mdlId[modelNo] = fn_80025A04(fn_80008DD0(DATANUM(0x3c0000, 0), 0x10000000, 2));
    fn_800269E8(obj->mdlId[modelNo], 1);
    collisionModels[0] = obj->mdlId[modelNo];
    modelNo = 3;
    obj->mdlId[modelNo] = fn_80025A04(fn_80008DD0(DATANUM(0x3c0000, 1), 0x10000000, 2));
    fn_800269E8(obj->mdlId[modelNo], 1);
    collisionModels[1] = obj->mdlId[modelNo];
    fn_800804E8(collisionModels, 2, 0x80);

    modelNo = 1;
    obj->mdlId[modelNo] = fn_80025A04(fn_80008DD0(
        lbl_1_bss_10 ? DATANUM(0x3c0000, 12) : DATANUM(0x3c0000, 11), 0x10000000, 2));
    fn_80026B8C(obj->mdlId[modelNo], 7);
    m601ModelLightSetup(obj->mdlId[modelNo]);
    fn_8002803C(obj->mdlId[modelNo], 1);

    if (lbl_1_bss_10 != 0) {
        for (i = 0; i < 6; i++) {
            M601StagePosition stagePositions[6] = {
                { -145.0f, -750.0f }, { 145.0f, -750.0f },
                { 750.0f, -145.0f }, { 750.0f, 145.0f },
                { -750.0f, -145.0f }, { -750.0f, 145.0f }
            };
            motionTime = rand8() / 256.0f;
            modelNo = i + 0xB;
            obj->mdlId[modelNo] = fn_80025A04(
                fn_80008DD0(DATANUM(0x3c0000, 9), 0x10000000, 2));
            fn_80026760(obj->mdlId[modelNo], stagePositions[i].x,
                0.0f, stagePositions[i].z);
            fn_80026B8C(obj->mdlId[modelNo], 2);
            fn_800269E8(obj->mdlId[modelNo], ((1 << 30) | (1 << 0)));
            fn_8002D414(obj->mdlId[modelNo],
                motionTime * fn_8002D530(obj->mdlId[modelNo]));
            modelNo = i + 5;
            if (i == 0) {
                obj->mdlId[modelNo] = fn_80025A04(
                    fn_80008DD0(DATANUM(0x3c0000, 10), 0x10000000, 2));
            } else {
                obj->mdlId[modelNo] = fn_80025D4C(obj->mdlId[5]);
            }
            fn_80026B8C(obj->mdlId[modelNo], 2);
            fn_800269E8(obj->mdlId[modelNo], ((1 << 30) | (1 << 0)));
            fn_80026760(obj->mdlId[modelNo], stagePositions[i].x,
                0.0f, stagePositions[i].z);
            fn_8002D414(obj->mdlId[modelNo],
                motionTime * fn_8002D530(obj->mdlId[modelNo]));
        }
    }

    memset(lbl_1_bss_244, 0, sizeof(lbl_1_bss_244));
    for (i = 0; i < 16; i++) {
        positionSet = i % 2;
        positionSet = (i / 4) % 2 == 0 ? positionSet : 1 - positionSet;
        lbl_1_bss_244[i].position.x = (i % 4) * 0x12C - 450.0;
        lbl_1_bss_244[i].position.y = (i / 4) * 0x12C - 450.0;
        lbl_1_bss_244[i].position.z = 180.0f;
        modelNo = i + 0x3A;
        obj->mdlId[modelNo] = fn_80025A04(fn_80008DD0(
            positionSet == 0 ? DATANUM(0x3c0000, 13) : DATANUM(0x3c0000, 14), 0x10000000, 2));
        fn_80026760(obj->mdlId[modelNo], lbl_1_bss_244[i].position.x,
            0.0f, lbl_1_bss_244[i].position.y);
        fn_800269E8(obj->mdlId[modelNo], 1);
        fn_800269E8(obj->mdlId[modelNo], ((1 << 30) | (1 << 1)));
        fn_80026B8C(obj->mdlId[modelNo], 5);

        modelNo = i + 0x4A;
        obj->mdlId[modelNo] = fn_80025A04(fn_80008DD0(
            positionSet == 0 ? DATANUM(0x3c0000, 15) : DATANUM(0x3c0000, 16), 0x10000000, 2));
        fn_80026760(obj->mdlId[modelNo], lbl_1_bss_244[i].position.x,
            0.0f, lbl_1_bss_244[i].position.y);
        fn_800269E8(obj->mdlId[modelNo], 1);
        fn_80026B8C(obj->mdlId[modelNo], 5);

        modelNo = i + 0x5A;
        obj->mdlId[modelNo] = fn_80025A04(fn_80008DD0(
            lbl_1_bss_10 ? (positionSet == 0 ? DATANUM(0x3c0000, 18) : DATANUM(0x3c0000, 19)) : DATANUM(0x3c0000, 17),
            0x10000000, 2));
        fn_80026760(obj->mdlId[modelNo], lbl_1_bss_244[i].position.x,
            0.0f, lbl_1_bss_244[i].position.y);
        fn_800269E8(obj->mdlId[modelNo], 1);
        fn_80026B8C(obj->mdlId[modelNo], 5);
    }

    for (i = 0; i < 12; i++) {
        HuVecF modelPositions[12] = {
            { -530.0f, -830.0f, 0.0f }, { -305.0f, -830.0f, 0.0f },
            { 305.0f, -830.0f, 0.0f }, { 530.0f, -830.0f, 0.0f },
            { 830.0f, -530.0f, -90.0f }, { 830.0f, -305.0f, -90.0f },
            { 830.0f, 305.0f, -90.0f }, { 830.0f, 530.0f, -90.0f },
            { -830.0f, -530.0f, 90.0f }, { -830.0f, -305.0f, 90.0f },
            { -830.0f, 305.0f, 90.0f }, { -830.0f, 530.0f, 90.0f }
        };
        modelNo = i + 0x11;
        if (i == 0) {
            obj->mdlId[modelNo] = fn_80025A04(fn_80008DD0(
                lbl_1_bss_10 ? DATANUM(0x3c0000, 5) : DATANUM(0x3c0000, 4), 0x10000000, 2));
        } else {
            obj->mdlId[modelNo] = fn_80025D4C(obj->mdlId[0x11]);
        }
        fn_80026760(obj->mdlId[modelNo], modelPositions[i].x,
            0.0f, modelPositions[i].y);
        fn_80026808(obj->mdlId[modelNo], 0.0f,
            modelPositions[i].z, 0.0f);
        fn_80026B8C(obj->mdlId[modelNo], 2);
        m601ModelLightSetup(obj->mdlId[modelNo]);
        if (i == 0) {
            if (lbl_1_bss_10 != 0) {
                obj->mtnId[1] = fn_80030458(obj->mdlId[0x11],
                    fn_80008DD0(DATANUM(0x3c0000, 8), 0x10000000, 2));
            }
            obj->mtnId[0] = fn_80030458(obj->mdlId[0x11], fn_80008DD0(
                lbl_1_bss_10 ? DATANUM(0x3c0000, 7) : DATANUM(0x3c0000, 6), 0x10000000, 2));
        }
        fn_8002C95C(obj->mdlId[modelNo], obj->mtnId[lbl_1_bss_10]);
        fn_800269E8(obj->mdlId[modelNo], ((1 << 30) | (1 << 1)));
    }
    obj->objFunc = fn_1_2A3C;
}

extern void fn_1_D14(MGPLAYER *playerP);

/* Creates and configures the player character for this object's assigned player slot. */
void fn_1_9574(OMOBJ *obj)
{
    MGACTOR_PARAM actorParam;
    MGACTOR_COLMAP_POLY collision;
    s32 playerOrder[4] = { 0, 1, 2, 3 };
    s32 i;
    s32 j;
    s32 playerCount;
    M601PlayerInitWork *work;
    s32 lightId;

    playerCount = 0;
    work = obj->data;
    memset(work, 0, sizeof(M601PlayerInitWork));
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (i == lbl_80265728[j].grpNo) {
                playerOrder[playerCount] = j;
                playerCount++;
            }
        }
    }
    work->playerNo = obj->work[0];
    work->configNo = playerOrder[work->playerNo];
    if (work->configNo == 4) {
        work->configNo = work->playerNo;
    }
    work->characterNo = lbl_80265728[work->configNo].charNo;
    work->modelNo = work->characterNo >= 11 ? 9 : work->characterNo;
    work->padNo = lbl_80265728[work->configNo].type != 0 ? -1 : lbl_80265728[work->configNo].padNo;
    work->difficulty = work->padNo != -1 ? 0 : lbl_80265728[work->configNo].comDif;
    work->mode = 2;
    work->scale = 1.0f;
    work->randomX = 40.0f * ((float)rand8() / 255.0f) - 20.0f
        + (work->difficulty == 0 ? rand8() % 20 + 0x32
        : work->difficulty == 1 ? 0x3C
        : work->difficulty == 2 ? 0x14
        : 0);
    work->randomZ = 40.0f * ((float)rand8() / 255.0f) - 20.0f
        + (work->difficulty == 0 ? rand8() % 20 + 0x32
        : work->difficulty == 1 ? 0x3C
        : work->difficulty == 2 ? 0x14
        : 0);

    actorParam.height = 150.0f;
    actorParam.radius = 50.0f;
    actorParam.param = work->playerNo;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.correctHookParam = 0;
    actorParam.narrowHook = NULL;
    actorParam.correctHook = NULL;
    lbl_1_bss_158[work->playerNo] = fn_80080728(
        (s16)work->configNo, &actorParam, 4, 1, -0xD, lbl_1_data_18C);
    *obj->mdlId = lbl_1_bss_158[work->playerNo]->actor->mdlId;
    fn_8005DDEC(4);

    {
    s32 xValues[4] = { -300, 300, -300, 300 };
    s32 zValues[4] = { -300, -300, 300, 300 };
    HuVecF position;
    lbl_1_bss_158[work->playerNo]->actor->pos.x = (float)xValues[work->playerNo];
    lbl_1_bss_158[work->playerNo]->actor->pos.y = 0.0f;
    lbl_1_bss_158[work->playerNo]->actor->pos.z = (float)zValues[work->playerNo];
    fn_8007ED60(lbl_1_bss_158[work->playerNo]);
    position = lbl_1_bss_158[work->playerNo]->actor->pos;
    position.y = -500.0f;
    if (fn_80086910(&lbl_1_bss_158[work->playerNo]->actor->pos,
        &position, 0xFFFF, &collision)) {
        collision.pos.y += 75.0f;
        fn_80086AA8(lbl_1_bss_158[work->playerNo]->actor, &collision.pos);
        lbl_1_bss_158[work->playerNo]->actor->pos.y = collision.pos.y;
    }
    fn_80026B8C(*obj->mdlId, 0);
    fn_80026E44(*obj->mdlId);
    fn_800273EC(*obj->mdlId, 0.5f, 0.5f,
        0.5f);
    if (lbl_1_bss_10 != 0) {
        lightId = Hu3DLLightCreate(*obj->mdlId, 0.0f,
            5000.0f, 0.0f, 0.0f,
            -1.0f, -0.4f, 0x54, 0x54, 0x6C);
        work->secondLight = lightId;
        fn_80028B58(*obj->mdlId, lightId);
        fn_800296E0(*obj->mdlId, lightId, 1);
        fn_800273EC(*obj->mdlId, 0.325f, 0.325f,
            0.325f);
        fn_800269E8(*obj->mdlId, 0x10000000);
    } else {
        m601ModelLightSetup(*obj->mdlId);
    }
    lightId = Hu3DLLightCreate(*obj->mdlId, 0.0f, 0.0f,
        500.0f, 0.0f, 1.0f,
        -0.4f, 0, 0, 0);
    work->firstLight = lightId;
    fn_80028B58(*obj->mdlId, lightId);
    fn_800296E0(*obj->mdlId, lightId, 1);
    lbl_1_bss_114.callback = lbl_1_bss_158[work->playerNo]->modeFunc[5];
    lbl_1_bss_158[work->playerNo]->modeFunc[5] = fn_1_D14;
    obj->objFunc = fn_1_EE4;
    }
}

void fn_1_9F6C(OMOBJ *obj)
{
    obj->objFunc = fn_1_5658;
}

extern s16 lbl_1_bss_14C;

extern OMOBJMAN *lbl_1_bss_150;

/* Sets up the minigame sequence, camera, shadow view, and initial object state. */
/* Reconstruction of removed debug code. */
static inline void DebugCheckLighting(int enabled)
{
    GXColor *lightColor;
    HuVecF *lightDirection;
    if (enabled) {
        HuVecF lightPosition = { 0.0f, 5000.0f, 0.0f };
        HuVecF lightDirectionDay = { 0.1f, -1.0f, -0.4f };
        HuVecF lightDirectionNight = { 0.0f, -1.0f, -1.0f };
        GXColor lightColorDay = { 255, 255, 255, 255 };
        GXColor lightColorNight = { 112, 112, 144, 255 };

        if (lbl_1_bss_10 != 0) {
            lightColor = &lightColorNight;
        } else {
            lightColor = &lightColorDay;
        }
        if (lbl_1_bss_10 != 0) {
            lightDirection = &lightDirectionNight;
        } else {
            lightDirection = &lightDirectionDay;
        }
        fn_800286D0(&lightPosition, lightDirection, lightColor);
    }
}

void ObjectSetup(void)
{
    s16 nightMode;
    HuVecF shadowPosition;
    HuVecF shadowUp;
    HuVecF shadowTarget;
    s32 player;
    OSReport("******* M601 ObjectSetup *********\n");
    lbl_1_bss_C = fn_8006A124(0x30002U);
    nightMode = lbl_802C0278;
    if (nightMode == 1)
    {
        lbl_1_bss_10 = 1;
    }
    for (player = 0; player < 6; player++)
    {
        lbl_1_bss_184[player].count = 0;
        lbl_1_bss_184[player].handle = - 1;
    }
    lbl_1_bss_150 = fn_80080418();
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 20.0f, 20.0f, 25000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 0x280, 0x1E0);
    lbl_802C028C = 3850.0f;
    lbl_80266A4C.x = 0.0f;
    lbl_80266A4C.y = 175.0f;
    lbl_80266A4C.z = 0.0f;
    lbl_80266A40.x = -33.625f;
    lbl_80266A40.y = 0.0f;
    lbl_80266A40.z = 0.0f;
    fn_8003D4FC(0);
    fn_800285BC();
    shadowPosition.x = 0.0f;
    shadowPosition.y = 10000.0f;
    shadowPosition.z = 0.0f;
    shadowUp.x = 0.0f;
    shadowUp.y = 1.0f;
    shadowUp.z = 0.0f;
    shadowTarget.x = 0.0f;
    shadowTarget.y = 0.0f;
    shadowTarget.z = -0.1f;
    Hu3DShadowCreate(8.5f, 5000.0f, 13000.0f);
    Hu3DShadowPosSet(& shadowPosition, & shadowUp, & shadowTarget);
    lbl_1_bss_14C = - 1;
    lbl_1_data_4 = - 1;
    fn_80070788(& lbl_1_data_30);
    DebugCheckLighting(0);
}

M601StageUpdateObject lbl_1_bss_244[16];
M601SoundEntry lbl_1_bss_184[6];
OMOBJ *lbl_1_bss_180;
OMOBJ *lbl_1_bss_16C[5];
OMOBJ *lbl_1_bss_168;
MGPLAYER *lbl_1_bss_158[4];
s32 lbl_1_bss_154;
OMOBJMAN *lbl_1_bss_150;
s16 lbl_1_bss_14C;
M601UnknownBss124 lbl_1_bss_124;
void *lbl_1_bss_120;
M601CallbackStorage lbl_1_bss_114;
s32 lbl_1_bss_110;
M601UnknownBss60 lbl_1_bss_60;
Mtx lbl_1_bss_30;
s32 lbl_1_bss_2C;
s32 lbl_1_bss_28;
s32 lbl_1_bss_24;
s32 lbl_1_bss_20;
s32 lbl_1_bss_1C;
s32 lbl_1_bss_18;
s32 lbl_1_bss_14;
s32 lbl_1_bss_10;
s32 lbl_1_bss_C;
M601UnknownBss4 lbl_1_bss_4;
s32 lbl_1_bss_0;
