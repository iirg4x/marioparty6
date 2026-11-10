/* Cannonball Fun: player and computer movement, hits and scoring, the stage and player models, the
 * minigame sequence callbacks, and a debug curve display. */
#include "math.h"

#include "string.h"

#include "game/main.h"

#include "game/object.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

#include "game/pad.h"

#include "game/audio.h"

#include "game/data.h"

#include "game/gamework.h"

#include "game/gamemes.h"

#include "game/mg/actman.h"

#include "game/printfunc.h"

#include "REL/m614Dll/player.h"
#include "REL/m614Dll/state.h"

#include "game/charman.h"

#include "game/sprite.h"

#include "game/esprite.h"

#include "game/hu3d.h"

#include "game/wipe.h"

#include "game/flag.h"

#include "game/memory.h"

#include "humath.h"

#include "dolphin/gx/GXVert.h"
#include "dolphin/gx.h"

/* The game's code never reads this value. */
u32 lbl_1_data_0 = 0xFFFF0000;
s32 lbl_1_data_4 = -1;
s32 lbl_1_data_8 = 255;
s32 lbl_1_data_C = -1;
s32 lbl_1_data_10 = -1;
s32 lbl_1_data_14 = 1;
MGSEQ_PARAM lbl_1_data_18 = {
    60, 0, fn_1_A38C, fn_1_6524, fn_1_6528, fn_1_652C, fn_1_6530,
    fn_1_6534, fn_1_6538, fn_1_653C, fn_1_A4D8
};
/* The game's code never reads these values. */
u8 lbl_1_data_40[14] = {0, 0, 24, 90, 30, 0, 18, 42, 96, 30, 42, 30, 30, 30};

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

void fn_1_2DC(int *x, int *y, float limit)
{
    float magnitude = fn_1_0(*x, *y);

    if (magnitude > limit) {
        *x = limit * (*x / magnitude);
        *y = limit * (*y / magnitude);
    }
}

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];

extern const VoidFunc _dtors[];

void ObjectSetup(void);

int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    ObjectSetup();
    return 0;
}

void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

const u32 lbl_1_rodata_30[4] = {1, 2, 4, 8};

void fn_1_5CC(void)
{
    Vec cameraPosition;
    Vec center;
    Vec up;
    f32 verticalOffset;
    f32 pitchCosine;
    f32 unusedRoll;
    f32 rotationZ;
    s16 cameraIndex;
    s32 zoomIn;

    cameraIndex = 0;
    while (cameraIndex < 4) {
        if (cameraIndex == 0) {
            lbl_1_bss_20 = (f32)((f64)lbl_802BFFF0[0] / (-32.0));
            lbl_1_bss_24 += (f64)lbl_802BFFF4[0] / 32.0;
            lbl_1_bss_28.verticalOffset = (f32)(5.0 * (f64)(
                ((lbl_802C0010[0] & 8) != 0) - ((lbl_802C0010[0] & 4) != 0)));
        }

        lbl_80266B18[cameraIndex].x += 5.0 * (f64)(
            ((lbl_802C0010[0] & 2) != 0) - ((lbl_802C0010[0] & 1) != 0));

        zoomIn = 0;
        if (((lbl_802C0010[0] & 0x800) != 0) && (lbl_80266BD8[cameraIndex] > 25.0f)) {
            zoomIn = 1;
        }
        lbl_80266BD8[cameraIndex] += 25.0 * (f64)(
            ((lbl_802C0010[0] & 0x400) != 0) - zoomIn);
        if (lbl_80266BD8[cameraIndex] <= 0.0f) {
            verticalOffset = 25.0f;
        } else {
            verticalOffset = lbl_80266BD8[cameraIndex];
        }
        lbl_80266BD8[cameraIndex] = verticalOffset;

        lbl_80266A58[cameraIndex].x += lbl_1_bss_20;
        lbl_80266A58[cameraIndex].y += lbl_1_bss_24;
        lbl_80266B18[cameraIndex].y += lbl_1_bss_28.verticalOffset;

        if (cameraIndex == -1) {
            fontcolor = 0xF;
            print8(0x40, 0x188, 2.0f, "%f,%f,%f",
                lbl_80266B18[cameraIndex].x, lbl_80266B18[cameraIndex].y,
                lbl_80266B18[cameraIndex].z);
            print8(0x40, 0x198, 2.0f, "%f,%f,%f",
                lbl_80266A58[cameraIndex].x, lbl_80266A58[cameraIndex].y,
                lbl_80266A58[cameraIndex].z);
            print8(0x40, 0x1A8, 2.0f, "%f",
                lbl_80266BD8[cameraIndex]);
        }

        pitchCosine = lbl_80266A58[cameraIndex].x;
        unusedRoll = lbl_80266A58[cameraIndex].y;
        rotationZ = lbl_80266A58[cameraIndex].z;
        cameraPosition.x = (f32)((f64)lbl_80266B18[cameraIndex].x +
            ((f64)lbl_80266BD8[cameraIndex] *
                (sin((M_PI * unusedRoll) / 180.0) *
                    cos((M_PI * pitchCosine) /
                        180.0))));
        cameraPosition.y = (f32)((f64)lbl_80266B18[cameraIndex].y +
            (lbl_80266BD8[cameraIndex] *
                -sin((M_PI * pitchCosine) / 180.0)));
        cameraPosition.z = (f32)((f64)lbl_80266B18[cameraIndex].z +
            ((f64)lbl_80266BD8[cameraIndex] *
                (cos((M_PI * unusedRoll) /
                    180.0) *
                    cos((M_PI * pitchCosine) /
                        180.0))));
        center.x = lbl_80266B18[cameraIndex].x;
        center.y = lbl_80266B18[cameraIndex].y;
        center.z = lbl_80266B18[cameraIndex].z;
        up.x = (f32)(sin((M_PI * unusedRoll) /
            180.0) * sin((M_PI * pitchCosine) /
                180.0));
        up.y = (f32)cos((M_PI * pitchCosine) /
            180.0);
        up.z = (f32)(cos((M_PI * unusedRoll) /
            180.0) * sin((M_PI * pitchCosine) /
                180.0));
        cameraIndex += 1;
    }
}

Vec lbl_1_data_5C = {0.0f, 5000.0f, 0.0f};
Vec lbl_1_data_68 = {0.2f, -1.0f, -0.5f};
Vec lbl_1_data_74 = {0.0f, -1.0f, -1.0f};
GXColor lbl_1_data_80[2] = {{255, 255, 255, 255}, {192, 192, 255, 255}};
GXColor lbl_1_data_88 = {192, 192, 192, 255};

/* 0.7 */

void fn_1_E14(MGPLAYER *playerP)
{
    int previousMode;

    previousMode = playerP->subMode;
    lbl_1_bss_A060.callback(playerP);
    if (previousMode == 2 && playerP->subMode == 3) {
        playerP->work[2] = 1;
    }
}

void fn_1_E7C(MGPLAYER *playerP)
{
    if (playerP->motNo != 10) {
        fn_8005C2C8(playerP->charNo, playerP->omObj->mtnId[10],
            0.0f, 5.0f, 0U);
        playerP->motNo = 10;
    }
    playerP->actor->rotY *= 0.7;
}

void fn_1_F08(MGPLAYER *playerP)
{
    if (playerP->motNo != 11) {
        fn_8005C2C8(playerP->charNo, playerP->omObj->mtnId[11],
            0.0f, 1.0f, 0x40000001U);
        playerP->motNo = 11;
    }
}

void fn_1_F78(MGPLAYER *playerP)
{
    if (playerP->motNo != 12) {
        fn_8005C2C8(playerP->charNo, playerP->omObj->mtnId[12],
            0.0f, 1.0f, 0U);
        playerP->motNo = 12;
    }
}

float fn_1_FE4(float current, float target, float lowerStep, float upperStep)
{
    float signedDifference;
    float limitedDifference;
    float lowerLimitedDifference;
    float resultDifference;
    s32 signedDirection;
    s32 movesAcrossThreshold;
    s32 crossesThreshold;
    s32 reverseCrossesThreshold;
    s32 thresholdDirection;
    s32 movementDirection;
    movesAcrossThreshold = 1;
    crossesThreshold = 0;
    if ((current >= 0.0f) && (target < 0.0f))
    {
        crossesThreshold = 1;
    }
    if (crossesThreshold == 0)
    {
        reverseCrossesThreshold = 0;
        if ((current < 0.0f) && (target >= 0.0f))
        {
            reverseCrossesThreshold = 1;
        }
        if (reverseCrossesThreshold == 0)
        {
            movesAcrossThreshold = 0;
        }
    }
    if (movesAcrossThreshold != 0)
    {
        thresholdDirection = - 1;
    }
    else
    {
        thresholdDirection = 1;
    }
    if (current < 0.0f)
    {
        movementDirection = - 1;
    }
    else
    {
        movementDirection = 1;
    }
    signedDirection = movementDirection * thresholdDirection;
    signedDifference = (float) signedDirection * (target - current);
    if (signedDifference < - upperStep)
    {
        limitedDifference = - upperStep;
    }
    else
    {
        if (signedDifference < 0.0f)
        {
            lowerLimitedDifference = signedDifference;
        }
        else
        {
            if (signedDifference > lowerStep)
            {
                resultDifference = lowerStep;
            }
            else
            {
                resultDifference = signedDifference;
            }
            lowerLimitedDifference = resultDifference;
        }
        limitedDifference = lowerLimitedDifference;
    }
    return current + ((float) signedDirection * limitedDifference);
}

extern f32 fn_1_0(f32 x, f32 y);

/* Debug hook: empty in the release build. */
static inline void DebugValue(s32 value) { }

/* Debug hook: empty in the release build. */
static inline void DebugFloat(f32 value) { }

void fn_1_11AC(OMOBJ *obj)
{
    M614PlayerCallbackWork * work = obj->data;
    s32 unusedA = 0;
    s32 unusedB = 0;
    s32 unusedC = 0;
    s32 unusedD = 0;
    s32 groupSnapshot;
    M614PlayerCallbackWork * playerWork[4];
    Vec hookPosition;
    Mtx movementMatrix;
    s32 i;
    s32 effectIndex;
    f32 value;
    f32 moveX;
    f32 moveZ;
    f32 sideInput;
    f32 moveAnalog;
    s32 actionButton;
    f32 forwardInput;
    f32 analogInput;
    f32 stickLength;
    extern f32 fn_1_B78C(f64 value);
    extern double atan2(double y, double x);
    extern double cos(double x);
    extern void HuAudFXStop(int seNo);
    extern void Hu3DMtxRotGet(Mtx src, HuVecF * dst);
    extern void espDispOn(s16 espId);
    extern void espTPLvlSet(s16 espId, f32 tpLvl);
    extern void espScaleSet(s16 espId, f32 scaleX, f32 scaleY);
    DebugValue(unusedA);
    DebugValue(unusedB);
    DebugValue(unusedC);
    DebugValue(unusedD);
    groupSnapshot = work->state;
    lbl_1_bss_A0E4 = fn_80071A8C();
    lbl_1_bss_0.frame = fn_80071DE4();
    for (i = 0; i < 4; i++)
    {
        playerWork[i] = lbl_1_bss_A0EC[i]->data;
    }
    forwardInput = 0.0f;
    sideInput = 0.0f;
    analogInput = 0.0f;
    moveX = 0.0f;
    moveZ = 0.0f;
    moveAnalog = 0.0f;
    actionButton = 0;
    work->rotationAngle += work->rotationSpeed;
    work->rotationAngle -= work->rotationAngle >= 360.0f ? 360 : 0;
    if (lbl_1_bss_14 != 0 && work->playerCount >= lbl_1_data_8)
    {
        s32 layoutOffsets[14] = { 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 15, 0, 0, 0 };
        fn_80026760(obj->mdlId[1], obj->trans.x, (f32)(182.0 + 5.0 * sin(M_PI * (f64) work->rotationAngle / 180.0)), obj->trans.z);
        Hu3DModelObjPosGet(obj->mdlId[1], "chara_hook", & hookPosition);
        fn_80026760(obj->mdlId[6], hookPosition.x, hookPosition.y + layoutOffsets[work->layoutIndex], hookPosition.z);
        if (lbl_1_bss_A0E4 > 7 && fn_8002D75C(obj->mdlId[6]))
        {
            fn_8002D414(obj->mdlId[6], 10.0f);
        }
    }
    if (work->state == 0 && lbl_1_bss_A0E4 <= 3)
    {
        if (lbl_1_bss_10 < 0x32)
        {
            moveZ = - 1.0f;
            moveX = 0.6f;
        }
        else
        {
            if (lbl_1_bss_10 < 0x5A)
            {
                moveZ = - (1.0f - (0.75f * (f32)(lbl_1_bss_10 - 0x32) / 40.0f));
                moveX = 0.6 + (0.4f * ((f32)(lbl_1_bss_10 - 0x32) / 40.0f));
            }
            else
            {
                if (lbl_1_bss_10 < 0x8C)
                {
                    moveZ = - 0.198f;
                    moveX = 1.0f;
                }
                else
                {
                    if (lbl_1_bss_10 < 0xB4)
                    {
                        f32 length;
                        f32 speed;
                        speed = length = fn_1_0(work->movementX, work->movementY);
                        if (length != 0.0f)
                        {
                            speed = (speed < 0.5925926f) ? 0.0f : speed - 0.5925926f;
                            work->movementX *= speed / length;
                            work->movementY *= speed / length;
                        }
                    }
                }
            }
        }
        if (moveX != 0.0f || moveZ != 0.0f)
        {
            f32 inputLength = fn_1_0(moveX, moveZ);
            if (inputLength != 0.0f)
            {
                moveX /= inputLength;
                moveZ /= inputLength;
            }
        }
        if (lbl_1_bss_10 == 0xD2)
        {
            work->headingYaw = 45.0f;
            work->positionX = (work->positionX + work->positionZ) / 2.0f;
            work->positionZ = work->positionX;
            playerWork[1]->positionX = - work->positionX;
            playerWork[1]->positionZ = work->positionZ;
            playerWork[2]->positionX = work->positionX;
            playerWork[2]->positionZ = - work->positionZ;
            playerWork[3]->positionX = - work->positionX;
            playerWork[3]->positionZ = - work->positionZ;
            fn_80026B64(* lbl_1_bss_A0E8->mdlId, 0xFFFFU);
        }
        if (lbl_1_bss_10 == 0xD2)
        {
            ASSERT(FALSE);
        }
        if (lbl_1_bss_10 == 0x10E && work->hookResetPending != 0)
        {
            fn_80027508(obj->mdlId[1]);
            fn_800269E8(obj->mdlId[1], (1 << 0));
            fn_800269E8(obj->mdlId[6], (1 << 0));
            fn_80026A44(obj->mdlId[0], (1 << 0));
            fn_80026A44(obj->mdlId[5], (1 << 0));
            fn_8002741C(obj->mdlId[0], "itemhook", obj->mdlId[2]);
            fn_8002741C(obj->mdlId[0], work->hookName, obj->mdlId[5]);
            work->hookResetPending = 0;
        }
        if (lbl_1_bss_10 == 0x10E)
        {
            for (i = 0; i < 4; i++)
            {
                fn_80026B64(* lbl_1_bss_A0EC[i]->mdlId, 0xFFFFU);
                fn_80026B64(lbl_1_bss_A0EC[i]->mdlId[5], 0xFFFFU);
                fn_80026B64(lbl_1_bss_A0EC[i]->mdlId[2], 0xFFFFU);
            }
        }
    }
    else
    {
        if (work->elapsedFrames >= 0x5A && work->characterState != 3 && lbl_1_bss_A0E4 == 5)
        {
            s32 modeIndex;
            if (work->padIndex != - 1)
            {
                moveX = lbl_802BFFFC[work->padIndex] / 55.0f;
                moveZ = -lbl_802BFFF8[work->padIndex] / 55.0f;
                moveAnalog = (f32)(lbl_802BFFEC[work->padIndex] - lbl_802BFFE8[work->padIndex]) / 150.0f;
                actionButton = (work->characterState == 1 ? lbl_802C0008[work->padIndex] : lbl_802C0008[work->padIndex]) & 0x100;
            }
            else
            {
                s32 blocked;
                s32 modeLimits[4] = { 180, 120, 10, 0 };
                s32 actionThresholds[4] = { 300, 210, 70, 30 };
                s32 targetDelay[4] = { 540, 400, 60, 0 };
                modeIndex = lbl_1_bss_1C == 0 && work->difficulty < 2 ? 2 : work->difficulty;
                work->modeFrameA += work->modeFrameA < 0xFFFF;
                work->modeFrameB += work->modeFrameB < 0xFFFF;
                switch (work->movementMode)
                {
                case 0 :
                    for (effectIndex = 0; effectIndex < 0x80; effectIndex++)
                    {
                        blocked = 0;
                        work->targetX = 1.5f * (1800.0f * (2.0f * ((f32)(rand8() + (rand8() << 8)) / 65536.0f) - 1.0f));
                        work->targetZ = 1.5f * (1800.0f * (2.0f * ((f32)(rand8() + (rand8() << 8)) / 65536.0f) - 1.0f));
                        if (work->targetX * work->targetX + work->targetZ * work->targetZ >= 8122500.0f)
                        {
                            continue;
                        }
                        for (i = 0; i < 5; i++)
                        {
                            if (lbl_1_bss_A1B8[i].state != 0)
                            {
                                if ((blocked = (work->targetX - lbl_1_bss_A1B8[i].x) * (work->targetX - lbl_1_bss_A1B8[i].x) + (work->targetZ - lbl_1_bss_A1B8[i].z) * (work->targetZ - lbl_1_bss_A1B8[i].z) <= (400.0f + lbl_1_bss_A1B8[i].radius) * (400.0f + lbl_1_bss_A1B8[i].radius)))
                                {
                                    break;
                                }
                            }
                        }
                        if (blocked)
                        {
                            continue;
                        }
                        if ((blocked = (work->targetX - work->positionX) * (work->targetX - work->positionX) + (work->targetZ - work->positionZ) * (work->targetZ - work->positionZ) <= 250000.0f))
                        {
                            break;
                        }
                        if (! blocked)
                        {
                            break;
                        }
                    }
                    if (effectIndex < 0x80)
                    {
                        work->movementMode = 1;
                        work->modeFrameA = 0;
                        work->targetPlayer = - 1;
                    }
                    break;
                case 1 :
                    if ((work->targetPlayer == - 1 && work->modeFrameA >= 0x12C) || ((work->targetX - work->positionX) * (work->targetX - work->positionX) + (work->targetZ - work->positionZ) * (work->targetZ - work->positionZ) < 40000.0f))
                    {
                        work->movementMode = 0;
                        work->modeFrameA = 0;
                    }
                    break;
                case 2 :
                    {
                        f32 turn;
                        turn = work->targetAngle - work->headingYaw;
                        turn += (f32)(turn < - 180.0f ? 360 : turn > 180.0f ? - 360 : 0);
                        turn += (f32)(turn < - 180.0f ? 360 : turn > 180.0f ? - 360 : 0);
                        if (fabs((f64) turn) < 8.0)
                        {
                            work->movementMode = 0;
                            work->modeFrameA = 0;
                        }
                        else
                        {
                            moveX = turn / - 2.0f + (fabs((f64) work->turnSpeed) > 0.0625 ? 7.0f * (work->turnSpeed / 2.0f) : 0.0f);
                        }
                        break;
                    }
                case 3 :
                    work->targetX = playerWork[work->targetPlayer]->positionX;
                    work->targetZ = playerWork[work->targetPlayer]->positionZ;
                    break;
                default :
                    break;
                }
                if (work->movementMode == 1 || work->movementMode == 3)
                {
                    f32 targetDistance;
                    double result2;
                    value = (f32)(180.0 * (atan2((f64)(work->targetX - work->positionX), (f64)(work->targetZ - work->positionZ)) / M_PI) - (f64) work->headingYaw);
                    value = value < - 180.0f || value >= 180.0f ? value - (s32)((value < 0.0f ? value - 180.0f : value + 180.0f) / 360.0f) * 360 : value;
                    targetDistance = fn_1_0(work->targetX - work->positionX, work->targetZ - work->positionZ);
                    moveX = value / - 2.0f + (fabs((f64) work->turnSpeed) > 0.0625 ? 7.0f * (work->turnSpeed / 2.0f) : 0.0f);
                    moveX = moveX < - 1.0f ? - 1.0f : moveX > 1.0f ? 1.0f : moveX;
                    moveZ = - 1.0 * (fabs((f64) value) < 45.0 ? 1.0 - fabs((f64) value) / 45.0 : 0.0) * (work->movementMode == 1 ? 1.0f : targetDistance < 400.0f ? - 0.5f : targetDistance < 800.0f ? 0.0f : targetDistance < 1300.0f ? (targetDistance - 500.0f) / 500.0f : 1.0f);
                    if (work->movementMode == 3)
                    {
                        if (fabs((f64) value) < (work->characterState != 0 ? 3 : 0x14))
                        {
                            if (targetDistance < (work->characterState != 0 ? 7500.0f : 2400.0f))
                            {
                                actionButton = actionThresholds[modeIndex] + rand8() % 12 <= work->actionFrames;
                            }
                        }
                    }
                }
                if (lbl_1_data_C % 4 == work->state && work->modeFrameA > 0xA && work->modeFrameB > targetDelay[modeIndex])
                {
                    s32 targetBlocked[4] = { - 1, - 1, - 1, - 1 };
                    s32 targetIndex = work->state;
                    f32 nearestDistanceSquared = 2147483600.0f;
                    for (i = 0; i < 4; i++)
                    {
                        f32 sine = (f32) sin((M_PI * (f64) work->headingYaw) / 180.0);
                        f32 cosine = (f32) cos((M_PI * (f64) work->headingYaw) / 180.0);
                        f32 targetDeltaX = playerWork[i]->positionX - work->positionX;
                        f32 targetDeltaZ = playerWork[i]->positionZ - work->positionZ;
                        f32 targetDistance = fn_1_0(targetDeltaX, targetDeltaZ);
                        if (i != work->state && playerWork[i]->elapsedFrames >= 0xD2 && ! ((sine * targetDeltaX + cosine * targetDeltaZ) < 0.0f))
                        {
                            targetBlocked[i] = 0;
                            for (effectIndex = 0; effectIndex < 5; effectIndex++)
                            {
                                f32 obstacleDeltaX = lbl_1_bss_A1B8[effectIndex].x - work->positionX;
                                f32 obstacleDeltaZ = lbl_1_bss_A1B8[effectIndex].z - work->positionZ;
                                f32 projection = (targetDeltaX * obstacleDeltaX + targetDeltaZ * obstacleDeltaZ) / (targetDistance * targetDistance);
                                f32 projectedX = projection * targetDeltaX;
                                f32 projectedZ = projection * targetDeltaZ;
                                if (! ((sine * obstacleDeltaX + cosine * obstacleDeltaZ) < 0.0f) && ! (targetDeltaX * targetDeltaX + targetDeltaZ * targetDeltaZ < obstacleDeltaX * obstacleDeltaX + obstacleDeltaZ * obstacleDeltaZ))
                                {
                                    projectedX += work->positionX;
                                    projectedZ += work->positionZ;
                                    if (fn_1_0(projectedX - lbl_1_bss_A1B8[effectIndex].x, projectedZ - lbl_1_bss_A1B8[effectIndex].z) <= lbl_1_bss_A1B8[effectIndex].radius)
                                    {
                                        targetBlocked[i]++;
                                    }
                                }
                            }
                            if (targetBlocked[i] == 0 && targetDeltaX * targetDeltaX + targetDeltaZ * targetDeltaZ < nearestDistanceSquared)
                            {
                                nearestDistanceSquared = targetDeltaX * targetDeltaX + targetDeltaZ * targetDeltaZ;
                                targetIndex = i;
                            }
                        }
                    }
                    if (targetIndex != work->state)
                    {
                        work->movementMode = 3;
                        work->modeFrameA = 0;
                        work->targetPlayer = targetIndex;
                    }
                    else
                    {
                        if (work->movementMode == 3 && targetBlocked[work->targetPlayer] != 0)
                        {
                            if (modeIndex <= 1)
                            {
                                work->movementMode = 0;
                                work->modeFrameA = 0;
                            }
                            else
                            {
                                work->movementMode = 2;
                                work->modeFrameA = 0;
                                if (playerWork[work->targetPlayer]->elapsedFrames < 0xD2)
                                {
                                    work->targetPlayer = - 1;
                                }
                            }
                        }
                    }
                }
                if (work->elapsedFrames < modeLimits[modeIndex])
                {
                    moveX = moveZ = moveAnalog = actionButton = 0;
                }
            }
        }
    }
    {
        f32 directionAngle;
        f32 inputLength;
        f32 sine;
        f32 cosine;
        f32 worldX;
        f32 worldZ;
        f32 directionLength;
        f32 value;
        f32 velocityDeltaX;
        f32 velocityDeltaZ;
        f32 velocityDistance;
        f32 maxVelocityStep;
        f32 boundaryLength;
        if (lbl_1_bss_14 == 0)
        {
            if (work->movementSoundState != 1 && moveZ * moveZ + moveAnalog * moveAnalog < 0.0025000002f)
            {
                work->movementSoundState = 1;
                if (lbl_1_bss_A104[work->state + 4].count < 1)
                {
                    lbl_1_bss_A104[work->state + 4].count++;
                }
                if (lbl_1_bss_A104[work->state].soundHandle != - 1)
                {
                    HuAudFXStop(lbl_1_bss_A104[work->state].soundHandle);
                }
            }
            else
            {
                if (work->movementSoundState != 2 && moveZ * moveZ + moveAnalog * moveAnalog > 0.010000001f)
                {
                    work->movementSoundState = 2;
                    if (lbl_1_bss_A104[work->state].count < 1)
                    {
                        lbl_1_bss_A104[work->state].count++;
                    }
                    if (lbl_1_bss_A104[work->state + 4].soundHandle != - 1)
                    {
                        HuAudFXStop(lbl_1_bss_A104[work->state + 4].soundHandle);
                    }
                }
                else
                {
                    directionAngle = (f32)(180.0 * (atan2((f64) moveX, (f64) moveZ) / M_PI));
                }
            }
        }
        directionAngle += directionAngle < 0.0f ? 360 : 0;
        stickLength = sqrtf(moveX * moveX + moveZ * moveZ);
        stickLength = stickLength > 1.0f ? 1.0f : stickLength;
        forwardInput = directionAngle > 180.0f ? directionAngle - 180.0f - 90.0f : directionAngle - 90.0f;
        forwardInput = stickLength * ((directionAngle > 180.0f ? 1 : - 1) * (1.0f - fn_1_B78C(forwardInput) / 90.0f));
        sideInput = directionAngle > 270.0f ? directionAngle - 360.0f : directionAngle > 90.0f ? directionAngle - 180.0f : directionAngle;
        sideInput = stickLength * ((directionAngle > 90.0f && directionAngle < 270.0f ? 1 : - 1) * (1.0f - fn_1_B78C(sideInput) / 90.0f));
        analogInput = moveAnalog < - 1.0f ? - 1.0f : moveAnalog > 1.0f ? 1.0f : moveAnalog;
        if (sideInput * sideInput + analogInput * analogInput > 1.0f)
        {
            inputLength = fn_1_0(sideInput, analogInput);
            if (inputLength != 0.0f)
            {
                sideInput /= inputLength;
                analogInput /= inputLength;
            }
        }
        work->turnSpeed = fn_1_FE4(work->turnSpeed, 2.0f * forwardInput, 0.125f, 0.0625f);
        work->headingYaw += work->turnSpeed;
        work->headingYaw += work->headingYaw < - 180.0f ? 360 : work->headingYaw >= 180.0f ? - 360 : 0;
        work->headingForward = fn_1_FE4(work->headingForward, 4.0f * sideInput, 1.0f, 1.0f);
        work->headingSide = fn_1_FE4(work->headingSide, 4.0f * analogInput, 1.0f, 1.0f);
        sine = (f32) sin((M_PI * (f64) work->headingYaw) / 180.0);
        cosine = (f32) cos((M_PI * (f64) work->headingYaw) / 180.0);
        worldX = sine * work->headingForward + cosine * work->headingSide;
        worldZ = cosine * work->headingForward - sine * work->headingSide;
        if (worldX != 0.0f || worldZ != 0.0f)
        {
            directionLength = worldX * worldX + worldZ * worldZ;
            if (directionLength > 1.0f)
            {
                value = directionLength = sqrtf(directionLength);
                directionLength = 1.0f;
                if (value != 0.0f)
                {
                    worldX *= directionLength / value;
                    worldZ *= directionLength / value;
                }
            }
        }
        velocityDeltaX = 26.666666f * worldX - work->movementX;
        velocityDeltaZ = 26.666666f * worldZ - work->movementY;
        if (velocityDeltaX != 0.0f || velocityDeltaZ != 0.0f)
        {
            velocityDistance = fn_1_0(velocityDeltaX, velocityDeltaZ);
            maxVelocityStep = worldX * work->movementX + worldZ * work->movementY < 0.0f || work->movementX * work->movementX + work->movementY * work->movementY < (26.666666f * worldX) * (26.666666f * worldX) + (26.666666f * worldZ) * (26.666666f * worldZ) ? 1.6666666f : 0.14814815f;
            maxVelocityStep = (velocityDistance > maxVelocityStep) ? maxVelocityStep : velocityDistance;
            if (velocityDistance != 0.0f)
            {
                work->movementX += maxVelocityStep * (velocityDeltaX / velocityDistance);
                work->movementY += maxVelocityStep * (velocityDeltaZ / velocityDistance);
            }
        }
        work->positionX += work->movementX;
        work->positionZ += work->movementY;
        if (work->positionX * work->positionX + work->positionZ * work->positionZ > 8122500.0f)
        {
            boundaryLength = fn_1_0(work->positionX, work->positionZ);
            if (boundaryLength != 0.0f)
            {
                work->positionX *= 2850.0f / boundaryLength;
                work->positionZ *= 2850.0f / boundaryLength;
            }
        }
    }
    for (i = 0; i < 4; i++)
    {
        if (i != work->state)
        {
            f32 deltaX;
            f32 deltaZ;
            f32 distance;
            f32 push;
            deltaX = work->positionX - playerWork[i]->positionX;
            deltaZ = work->positionZ - playerWork[i]->positionZ;
            distance = deltaX * deltaX + deltaZ * deltaZ;
            if (distance != 0.0f && distance < 90000.0f)
            {
                distance = sqrtf(distance);
                if (distance > 0.0f)
                {
                    push = fn_1_0(work->movementX, work->movementY) / 16.0f;
                    push = (push > 26.666666f) ? 26.666666f : push;
                    push = (push < 0.0f) ? 0.0f : push;
                    playerWork[i]->movementX -= push * (deltaX / distance);
                    playerWork[i]->movementY -= push * (deltaZ / distance);
                    push = fn_1_0(playerWork[i]->movementX, playerWork[i]->movementY);
                    if (push > 26.666666f)
                    {
                        playerWork[i]->movementX *= 26.666666f / push;
                        playerWork[i]->movementY *= 26.666666f / push;
                    }
                    distance = 300.0f / distance;
                    work->positionX = playerWork[i]->positionX + deltaX * distance;
                    work->positionZ = playerWork[i]->positionZ + deltaZ * distance;
                }
            }
        }
    }
    for (i = 0; i < 5; i++)
    {
        if (lbl_1_bss_A1B8[i].state == 0)
        {
            f32 deltaX;
            f32 deltaZ;
            f32 distance;
            deltaX = work->positionX - lbl_1_bss_A1B8[i].x;
            deltaZ = work->positionZ - lbl_1_bss_A1B8[i].z;
            distance = deltaX * deltaX + deltaZ * deltaZ;
            if (distance != 0.0f && distance < (150.0f + lbl_1_bss_A1B8[i].radius) * (150.0f + lbl_1_bss_A1B8[i].radius))
            {
                distance = sqrtf(distance);
                if (distance != 0.0f)
                {
                    distance = (150.0f + lbl_1_bss_A1B8[i].radius) / distance;
                    work->positionX = lbl_1_bss_A1B8[i].x + deltaX * distance;
                    work->positionZ = lbl_1_bss_A1B8[i].z + deltaZ * distance;
                }
            }
        }
    }
    if (lbl_1_bss_14 == 0 || lbl_1_data_4 == 5)
    {
        f32 value;
        s32 bobSign;
        bobSign = work->elapsedFrames % 2 * 2 - 1;
        omSetTra(obj, work->positionX, 225.0f + (work->elapsedFrames < 0x5A ? 20.0f * ((1.0f - (f32) work->elapsedFrames / 100.0f) * bobSign) : 0.0f) + 5.0 * sin(M_PI * work->rotationAngle / 180.0), work->positionZ);
        fn_80026760(obj->mdlId[1], obj->trans.x, obj->trans.y, obj->trans.z);
        fn_80026760(obj->mdlId[5], 0.0f, 0.0f, 0.0f);
        fn_80026760(obj->mdlId[3], obj->trans.x, 4.0f, obj->trans.z);
        fn_80026760(obj->mdlId[4], obj->trans.x, 4.0f, obj->trans.z);
        if (work->hookResetPending == 0 && lbl_1_bss_10 > 0xB4)
        {
            value = 1.0f;
            if (lbl_1_bss_14 == 0 && work->elapsedFrames >= 0x5A && work->elapsedFrames <= 0xD2)
            {
                if (work->elapsedFrames < 0xA5)
                {
                    value = 4.0f * ((f32)(work->elapsedFrames - 0x5A) / 15.0f);
                    value = 0.175f + 0.275f * (0.5 + cos(M_PI * (360.0f * value) / 180.0) / 2.0);
                }
                else
                {
                    value = 1.5f * ((f32)(work->elapsedFrames - 0xA5) / 15.0f);
                    value = 0.4f + 0.35f * (0.5 + cos(M_PI * (360.0f * value) / 180.0) / 2.0);
                }
            }
            if (value == 1.0f)
            {
                fn_80026CBC(obj->mdlId[0], value);
                fn_80026CBC(obj->mdlId[5], value);
                fn_80026CBC(obj->mdlId[2], value);
                fn_80026CBC(obj->mdlId[3], value);
                fn_80026A44(obj->mdlId[0], (1 << 0));
                fn_80026A44(obj->mdlId[5], (1 << 0));
                fn_80026A44(obj->mdlId[3], (1 << 0));
                fn_80026A44(obj->mdlId[4], (1 << 0));
            }
            else
            {
                fn_800269E8(obj->mdlId[0], (1 << 1));
                fn_800269E8(obj->mdlId[5], (1 << 1));
                fn_800269E8(obj->mdlId[2], (1 << 1));
                fn_80026CBC(obj->mdlId[0], value);
                fn_80026CBC(obj->mdlId[5], value);
                fn_80026CBC(obj->mdlId[2], value);
                fn_80026CBC(obj->mdlId[3], value);
            }
        }
        {
            Mtx headingMatrix;
            Vec rotationAxis;
            rotationAxis.x = work->movementY;
            rotationAxis.z = - work->movementX;
            rotationAxis.y = rotationAxis.x * rotationAxis.x + rotationAxis.z * rotationAxis.z == 0.0f;
            work->headingOffset = 15.0f * (work->turnSpeed / 2.0f);
            PSMTXRotAxisRad(movementMatrix, & rotationAxis, MTXDegToRad(5.0f * (fn_1_0(work->movementY, work->movementX) / 4.0f)));
            PSMTXRotRad(headingMatrix, 0x79, (f32)(M_PI * ((work->headingYaw + work->headingOffset) / 180.0f)));
            PSMTXConcat(movementMatrix, headingMatrix, movementMatrix);
            Hu3DMtxRotGet(movementMatrix, & rotationAxis);
            omSetRot(obj, rotationAxis.x, rotationAxis.y, rotationAxis.z);
            fn_80026808(obj->mdlId[1], obj->rot.x, obj->rot.y, obj->rot.z);
            fn_80026808(obj->mdlId[5], 0.0f, 0.0f, 0.0f);
            fn_80026808(obj->mdlId[3], 0.0f, work->headingYaw + work->headingOffset, 0.0f);
            fn_80026760(obj->mdlId[4], obj->trans.x - 80.0f * (work->movementX / 26.666666f), 4.0f, obj->trans.z - 80.0f * (work->movementY / 26.666666f));
        }
    }
    work->characterStateFrame += work->characterStateFrame < 0xFFFF;
    if (work->characterState == 0 && work->actionFrames >= 0x1E && actionButton != 0)
    {
        work->actionFrames = 0;
        for (i = 0; i < 1; i++)
        {
            for (effectIndex = 0; effectIndex < 6; effectIndex++)
            {
                if (work->movingEffects[effectIndex].state == 0)
                {
                    Vec firstOffset = {72.0f, 75.0f, 110.0f};
                    Vec secondOffset = {96.0f, 75.0f, 120.0f};
                    firstOffset.x *= (f32)(work->effectDirection != 0 ? 1 : - 1);
                    secondOffset.x *= (f32)(work->effectDirection != 0 ? 1 : - 1);
                    PSMTXMultVec(movementMatrix, & firstOffset, & firstOffset);
                    PSMTXMultVec(movementMatrix, & secondOffset, & secondOffset);
                    work->movingEffects[effectIndex].state = 1;
                    work->movingEffects[effectIndex].x = work->positionX + firstOffset.x;
                    work->movingEffects[effectIndex].y = work->positionY + firstOffset.y;
                    work->movingEffects[effectIndex].z = work->positionZ + firstOffset.z;
                    work->movingEffects[effectIndex].velocityX = (f32)(75.0 * sin(M_PI * (f64)(- 2.0f * ((f32) work->effectDirection - 0.5f) + (work->headingYaw + work->headingOffset)) / 180.0));
                    work->movingEffects[effectIndex].velocityY = 0.0f;
                    work->movingEffects[effectIndex].velocityZ = (f32)(75.0 * cos(M_PI * (f64)(- 2.0f * ((f32) work->effectDirection - 0.5f) + (work->headingYaw + work->headingOffset)) / 180.0));
                    work->movingEffects[effectIndex].accelerationY = work->movingEffects[effectIndex].state == 1 ? (work->movingEffects[effectIndex].y - 37.5f) / 528.0f : 0.0f;
                    fn_8002C95C(obj->mdlId[0], obj->mtnId[work->effectDirection == 0 ? 0 : 1]);
                    fn_80026A44(obj->mdlId[work->effectDirection + 7], (1 << 0));
                    fn_80026760(obj->mdlId[work->effectDirection + 7], work->positionX + secondOffset.x, work->positionY + secondOffset.y, work->positionZ + secondOffset.z);
                    fn_80026808(obj->mdlId[work->effectDirection + 7], obj->rot.x, obj->rot.y, obj->rot.z);
                    fn_8002D414(obj->mdlId[work->effectDirection + 7], 6.0f);
                    work->effectDirection = 1 - work->effectDirection;
                    if (lbl_1_bss_A104[work->state + 8].count < 1)
                    {
                        lbl_1_bss_A104[work->state + 8].count++;
                    }
                    break;
                }
            }
        }
    }
    work->actionFrames += work->actionFrames < 0xFFFF;
    for (i = 0; i < 6; i++)
    {
        if (work->movingEffects[i].state != 0)
        {
            f32 unusedExtent = work->movingEffects[i].state == 1 ? 330.0f : 330.0f;
            f32 unusedRadius = work->movingEffects[i].state == 1 ? 37.5f : 250.0f;
            DebugFloat(unusedExtent);
            DebugFloat(unusedRadius);
            work->movingEffects[i].velocityY -= work->movingEffects[i].accelerationY;
            work->movingEffects[i].x += work->movingEffects[i].velocityX;
            work->movingEffects[i].y += work->movingEffects[i].velocityY;
            work->movingEffects[i].z += work->movingEffects[i].velocityZ;
            if (work->movingEffects[i].state == 1)
            {
                fn_80026760(obj->mdlId[i + 9], work->movingEffects[i].x, work->movingEffects[i].y, work->movingEffects[i].z);
            }
        }
        if (work->movingEffects[i].state == 1)
        {
            fn_80026A44(obj->mdlId[i + 9], (1 << 0));
        }
        else
        {
            fn_800269E8(obj->mdlId[i + 9], (1 << 0));
        }
    }
    work->elapsedFrames += work->elapsedFrames < 0xFFFF;
    for (effectIndex = 0; effectIndex < 6; effectIndex++)
    {
        if (work->movingEffects[effectIndex].state != 0 && (work->movingEffects[effectIndex].state != 2 || ! (work->movingEffects[effectIndex].progress < 0.25f)))
        {
            f32 collisionRadius;
            f32 positionX;
            f32 positionY;
            f32 positionZ;
            collisionRadius = work->movingEffects[effectIndex].state == 1 ? 37.5f : 250.0f * work->movingEffects[effectIndex].progress;
            positionX = work->movingEffects[effectIndex].x;
            positionY = work->movingEffects[effectIndex].y;
            positionZ = work->movingEffects[effectIndex].z;
            if (work->movingEffects[effectIndex].y < collisionRadius && work->movingEffects[effectIndex].state == 1)
            {
                work->movingEffects[effectIndex].state = 0;
            }
            if (work->movingEffects[effectIndex].state != 0)
            {
                for (i = 0; i < 4; i++)
                {
                    if (i != work->state)
                    {
                        f32 deltaX;
                        f32 deltaY;
                        f32 deltaZ;
                        f32 distance;
                        deltaX = work->movingEffects[effectIndex].x - playerWork[i]->positionX;
                        deltaY = 0.5f * (work->movingEffects[effectIndex].y - 220.0f);
                        deltaZ = work->movingEffects[effectIndex].z - playerWork[i]->positionZ;
                        distance = deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
                        if (distance < (150.0f + collisionRadius) * (150.0f + collisionRadius))
                        {
                            if (deltaX == 0.0f && deltaZ == 0.0f)
                            {
                                deltaX = 1.0f;
                                deltaZ = 0.0f;
                            }
                            distance = fn_1_0(deltaX, deltaZ);
                            distance = distance == 0.0f ? 1.0f : distance;
                            if (playerWork[i]->elapsedFrames >= 0xD2)
                            {
                                playerWork[i]->movementX = 26.666666f * (- deltaX / distance);
                                playerWork[i]->movementY = 26.666666f * (- deltaZ / distance);
                                playerWork[i]->elapsedFrames = 0;
                                if (lbl_1_bss_A0E4 == 5)
                                {
                                    fn_8006A030((s16) playerWork[i]->playerNumber, 0x14, 7, 3);
                                    if (lbl_1_data_4 == - 1 && work->playerCount < 0xFF)
                                    {
                                        work->playerCount++;
                                        if (work->playerCount > 0 && work->playerCount <= 5)
                                        {
                                            work->spriteFrame[work->playerCount - 1] = 0;
                                        }
                                    }
                                }
                            }
                            work->movingEffects[effectIndex].state = 0;
                            if (lbl_1_bss_A104[i + 0x10].count < 1)
                            {
                                lbl_1_bss_A104[i + 0x10].count++;
                            }
                            if (work->characterState == 2)
                            {
                                work->characterState = 4;
                                work->characterStateFrame = 0;
                            }
                            break;
                        }
                    }
                }
            }
            if (work->movingEffects[effectIndex].state != 0)
            {
                for (i = 0; i < 5; i++)
                {
                    if (lbl_1_bss_A1B8[i].state == 0)
                    {
                        f32 deltaX;
                        f32 deltaZ;
                        f32 distance;
                        deltaX = work->movingEffects[effectIndex].x - lbl_1_bss_A1B8[i].x;
                        deltaZ = work->movingEffects[effectIndex].z - lbl_1_bss_A1B8[i].z;
                        distance = deltaX * deltaX + deltaZ * deltaZ;
                        if (distance != 0.0f && distance < (collisionRadius + lbl_1_bss_A1B8[i].radius) * (collisionRadius + lbl_1_bss_A1B8[i].radius))
                        {
                            distance = sqrtf(distance);
                            distance = distance == 0.0f ? 1.0f : distance;
                            distance = (collisionRadius + lbl_1_bss_A1B8[i].radius) / distance;
                            work->movingEffects[effectIndex].x = lbl_1_bss_A1B8[i].x + deltaX * distance;
                            work->movingEffects[effectIndex].z = lbl_1_bss_A1B8[i].z + deltaZ * distance;
                            if (work->movingEffects[effectIndex].state > 1)
                            {
                                lbl_1_bss_A1B8[i].state = 1;
                            }
                            else
                            {
                                work->movingEffects[effectIndex].state = 0;
                                positionX -= 0.9f * work->movingEffects[effectIndex].velocityX;
                                positionZ -= 0.9f * work->movingEffects[effectIndex].velocityZ;
                            }
                        }
                    }
                }
            }
            if (work->movingEffects[effectIndex].state != 0 && work->movingEffects[effectIndex].state != 2 && fn_1_0(work->movingEffects[effectIndex].x, work->movingEffects[effectIndex].z) >= 3000.0f - collisionRadius && work->movingEffects[i].state != 2)
            {
                work->movingEffects[effectIndex].state = 0;
                positionX -= 0.9f * work->movingEffects[effectIndex].velocityX;
                positionZ -= 0.9f * work->movingEffects[effectIndex].velocityZ;
            }
            if (work->movingEffects[effectIndex].state == 0)
            {
                for (i = 0; i < 6; i++)
                {
                    if (work->effects[i].state == 0)
                    {
                        work->effects[i].state = 1;
                        work->effects[i].x = positionX;
                        work->effects[i].y = positionY;
                        work->effects[i].z = positionZ;
                        work->effects[i].scale = 1.0f;
                        if (lbl_1_bss_A104[work->state + 12].count < 1)
                        {
                            lbl_1_bss_A104[work->state + 12].count++;
                        }
                        break;
                    }
                }
            }
        }
    }
    for (i = 0; i < 6; i++)
    {
        if (lbl_1_bss_14 == 0 && work->effects[i].state != 0)
        {
            fn_80026A44(obj->mdlId[i + 0xF], (1 << 0));
            if (work->effects[i].state == 1)
            {
                fn_8002D414(obj->mdlId[i + 0xF], 0.0f);
            }
            fn_80026760(obj->mdlId[i + 0xF], work->effects[i].x, work->effects[i].y, work->effects[i].z);
            fn_800268B0(obj->mdlId[i + 0xF], work->effects[i].scale, work->effects[i].scale, work->effects[i].scale);
            work->effects[i].state++;
            if (work->effects[i].state > 0xF)
            {
                work->effects[i].state = 0;
            }
        }
        else
        {
            fn_800269E8(obj->mdlId[i + 0xF], (1 << 0));
        }
    }
    if (lbl_1_bss_14 == 0)
    {
        for (i = 0; i < 2; i++)
        {
            HU3D_MODEL * model = & lbl_802C01C8[obj->mdlId[i + 7]];
            if ((model->attr & 1U) == 0)
            {
                if (fn_8002D4E0(obj->mdlId[i + 7]) >= fn_8002D530(obj->mdlId[i + 7]))
                {
                    fn_800269E8(obj->mdlId[i + 7], (1 << 0));
                }
            }
        }
    }
    if (lbl_1_data_4 == - 1 || lbl_1_data_4 == 5 || lbl_1_bss_14 == 0)
    {
        for (i = 0; i < 5; i++)
        {
            work->spriteFrame[i] += work->spriteFrame[i] < 0xFFFF;
            if (i < work->playerCount)
            {
                espDispOn(lbl_1_bss_A0A4[work->state][i]);
                if (work->spriteFrame[i] <= 30)
                {
                    f32 progress = (f32) work->spriteFrame[i] / 30.0f;
                    f32 scale = 1.0f + 2.5f * ((1.0f - progress) * (1.0f - progress));
                    espTPLvlSet(lbl_1_bss_A0A4[work->state][i], progress);
                    espScaleSet(lbl_1_bss_A0A4[work->state][i], 0.875f * scale, 0.875f * scale);
                }
            }
        }
    }
}

float fn_1_53DC(float frame, float rise, float hold, float fall)
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

void fn_1_55A0(OMOBJ *obj)
{
    M614PlayerCallbackWork * playerWork[4];
    s32 player;
    s32 camera;
    s32 stage;
    s32 model;
    s32 sprite;
    s32 spritePair;
    s32 frame;
    s32 itemId;
    s32 x;
    s32 y;
    s32 spriteId;
    s32 left;
    s32 top;
    f32 curveOutput;
    f32 viewportProgress;
    f32 opacity;
    f32 distance;
    f32 factor;
    f32 hold;
    f32 viewportWidth;
    f32 fall;
    f32 x0;
    f32 firstProgress;
    f32 dx;
    f32 dz;
    u32 resource;
    f32 x1;
    for (player = 0; player < 4; player++)
    {
        playerWork[player] = lbl_1_bss_A0EC[player]->data;
    }
    lbl_1_bss_A0E4 = fn_80071A8C();
    lbl_1_bss_0.frame = fn_80071DE4();
    lbl_1_data_C++;
    lbl_1_data_C %= 0xF0000000;
    lbl_1_bss_10 += lbl_1_bss_10 < 0xFFFF;
    for (player = 0; player < 4; player++)
    {
        if (lbl_1_data_4 == - 1 || lbl_1_data_4 == 5 || lbl_1_bss_14 == 0)
        {
            lbl_80266B18[player].x = playerWork[player]->positionX;
            lbl_80266B18[player].z = playerWork[player]->positionZ;
            lbl_80266A58[player].y = 180.0f + playerWork[player]->headingYaw;
        }
    }
    if (lbl_1_bss_10 < 0xB4)
    {
        lbl_80266A58[0].x = lbl_1_bss_10 < 0x78 ?
            (f32)(lbl_1_bss_10 * 0xF) / 120.0f + (-50.0f) :
            -35.0f;
        lbl_80266A58[0].y = playerWork[0]->headingYaw;
    }
    else
    {
        if (lbl_1_bss_10 >= 0xB4 && lbl_1_bss_10 <= 0x10E)
        {
            curveOutput = fn_1_53DC((f32)(lbl_1_bss_10 - 0xB4), 1.0f, 89.0f, 0.0f);
            firstProgress = 1.0f - curveOutput;
            lbl_80266A58[0].x = -35.0f * firstProgress + (- 19.0f) * curveOutput;
            lbl_80266A58[0].y = playerWork[0]->headingYaw + 180.0f * curveOutput;
            lbl_80266BD8[0] = 750.0f * firstProgress + 1000.0f * curveOutput;
            lbl_80266B18[0].y = 230.0f * firstProgress + 380.0f * curveOutput;
        }
    }
    if (lbl_1_bss_10 > 0xD2 && lbl_1_bss_10 <= 0x10E)
    {
        left = 0x10;
        top = 0x28;
        viewportProgress = (f32)(lbl_1_bss_10 - 0xD2) / 60.0f;
        hold = 1.0f - viewportProgress;
        viewportWidth = 640.0f * hold + viewportProgress * (f32)(left + 0x140);
        fall = viewportProgress * (f32)(left + 0x140);
        x0 = 480.0f * hold + viewportProgress * (f32)(top + 0xF0);
        x1 = viewportProgress * (f32)(top + 0xF0);
        for (player = 0; player < 4; player++)
        {
            x = player % 2;
            y = player / 2;
            Hu3DCameraPerspectiveSet(lbl_1_rodata_30[player], 45.0f, 20.0f, 25000.0f, (1.2f * (viewportWidth / x0)) / 1.3333334f);
            Hu3DCameraViewportSet(lbl_1_rodata_30[player], x == 0 ? 0.0f : 640.0f - fall, y == 0 ? 0.0f : 480.0f - x1, x == 0 ? viewportWidth : fall, y == 0 ? x0 : x1, 0.0f, 1.0f);
            Hu3DCameraScissorSet(lbl_1_rodata_30[player], (u32)(2.0f + (x == 0 ? 0.0f : 640.0f - 320.0f * viewportProgress)), (u32)(2.0f + (y == 0 ? 0.0f : 480.0f - 240.0f * viewportProgress)), (u32)((x == 0 ? 640.0f - 320.0f * viewportProgress : 320.0f * viewportProgress) - 4.0f), (u32)((y == 0 ? 480.0f - 240.0f * viewportProgress : 240.0f * viewportProgress) - 4.0f));
        }
    }
    if (lbl_1_bss_14 == 0)
    {
        for (player = 0; player < 5; player++)
        {
            lbl_1_bss_A1B8[player].state += lbl_1_bss_A1B8[player].state > 0 && lbl_1_bss_A1B8[player].state < 0xFFFF;
            for (camera = 0; camera < 4; camera++)
            {
                opacity = 1.0f;
                model = player + 0xB + camera * 5;
                if (lbl_1_bss_A1B8[player].state >= 0x3C)
                {
                    fn_800269E8(obj->mdlId[model], 1);
                }
                if (lbl_1_bss_A1B8[player].state >= 0)
                {
                    dz = lbl_1_bss_A1B8[player].z - playerWork[camera]->positionZ;
                    dx = lbl_1_bss_A1B8[player].x - playerWork[camera]->positionX;
                    distance = fn_1_0(dx, dz);
                    distance = distance == 0.0f ? 1.0f : distance;
                    if ((factor = (f32)(((lbl_1_bss_A1B8[player].x - playerWork[camera]->positionX) / distance) * sin((M_PI * (f64) playerWork[camera]->headingYaw) / 180.0) + (((lbl_1_bss_A1B8[player].z - playerWork[camera]->positionZ) / distance) * cos((M_PI * (f64) playerWork[camera]->headingYaw) / 180.0)))) < (- 0.25f))
                    {
                        opacity *= factor < (- 1.0f) ? 0.5f : 1.0f - (0.6f * (- factor - 0.25f)) / 0.75f;
                    }
                    if (lbl_1_bss_A1B8[player].state >= 0x1E)
                    {
                        fn_80026A44(obj->mdlId[model], 2);
                        fn_80026B8C(obj->mdlId[model], 4);
                        opacity *= 1.0f - (f32)(lbl_1_bss_A1B8[player].state - 0x1E) / 30.0f;
                    }
                    else
                    {
                        if (lbl_1_bss_A1B8[player].state >= 1)
                        {
                            fn_80026A44(obj->mdlId[model], 0x40000002U);
                        }
                    }
                    fn_80026CBC(obj->mdlId[model], opacity);
                }
            }
        }
    }
}

void fn_1_6524(s16 mode, s16 frameNo)
{
}

void fn_1_6528(s16 mode, s16 frameNo)
{
}

void fn_1_652C(s16 mode, s16 frameNo)
{
}

void fn_1_6530(s16 mode, s16 frameNo)
{
}

void fn_1_6534(s16 mode, s16 frameNo)
{
}

void fn_1_6538(s16 mode, s16 frameNo)
{
}

void fn_1_653C(s16 mode, s16 frameNo);

void fn_1_653C(s16 mode, s16 frameNo)
{
    if (frameNo >= 60) {
        fn_80071A9C();
    }
}

void fn_1_656C(OMOBJ *obj)
{
    M614PlayerCallbackWork *playerWork[4];
    s32 frame;
    s32 player;
    s32 index;
    s32 winnerCount;
    s32 winnerIndex;
    s32 otherCount;
    s32 otherIndex;
    s32 firstWinner;
    s32 secondWinner;
    s32 thirdWinner;
    s32 fourthWinner;
    s32 x;
    s32 y;
    f32 centerX;
    f32 centerZ;
    f32 rotation;
    f32 xPosition;
    f32 zPosition;

    frame = fn_80071DE4();
    {
        s32 layoutOffsets[16] = {
            -300, -100, 100, 300, 0, -300, 0, 300,
            -150, 150, -150, 150, -300, 0, 300, 0
        };
        s32 layoutPositions[16] = {
            0, 0, 0, 0, 300, 0, 0, 0,
            150, 150, -150, -150, 150, 150, 150, -150
        };
    }
    lbl_1_bss_A0E4 = fn_80071A8C();
    for (player = 0; player < 4; player++) {
        playerWork[player] = lbl_1_bss_A0EC[player]->data;
    }
    for (player = 0; player < 20; player++) {
        s32 soundEffectIds[20] = { MSM_SE_MG614_MOVE_LOOP_P1,       MSM_SE_MG614_MOVE_LOOP_P2,
                                   MSM_SE_MG614_MOVE_LOOP_P3,       MSM_SE_MG614_MOVE_LOOP_P4,
                                   MSM_SE_MG614_STAND_LOOP_P1,      MSM_SE_MG614_STAND_LOOP_P2,
                                   MSM_SE_MG614_STAND_LOOP_P3,      MSM_SE_MG614_STAND_LOOP_P4,
                                   MSM_SE_MG614_CANNONBALL_FIRE_P1, MSM_SE_MG614_CANNONBALL_FIRE_P2,
                                   MSM_SE_MG614_CANNONBALL_FIRE_P3, MSM_SE_MG614_CANNONBALL_FIRE_P4,
                                   MSM_SE_MG614_CANNONBALL_LAND_P1, MSM_SE_MG614_CANNONBALL_LAND_P2,
                                   MSM_SE_MG614_CANNONBALL_LAND_P3, MSM_SE_MG614_CANNONBALL_LAND_P4,
                                   MSM_SE_MG614_PLAYER_HIT_P1,      MSM_SE_MG614_PLAYER_HIT_P2,
                                   MSM_SE_MG614_PLAYER_HIT_P3,      MSM_SE_MG614_PLAYER_HIT_P4 };
        if (lbl_1_bss_A104[player].count > 3) {
            lbl_1_bss_A104[player].count = 3;
        }
        if (lbl_1_bss_A104[player].count > 0) {
            if (soundEffectIds[player] != -1) {
                s32 soundHandle;

                soundHandle = HuAudFXPlay(soundEffectIds[player]);
                lbl_1_bss_A104[player].soundHandle = soundHandle;
            }
            lbl_1_bss_A104[player].count--;
        }
    }

    frame = fn_80071DE4();
    lbl_1_bss_A0E4 = fn_80071A8C();
    lbl_1_bss_34 += lbl_1_bss_A0E4 == 6 ||
        lbl_1_bss_A0E4 == 7 || lbl_1_bss_A0E4 == 8;
    switch (lbl_1_bss_A0E4) {
    case 0:
    case 1:
    case 2:
        break;
    case 3:
        if (lbl_1_data_10 != -1) {
            return;
        }
        if ((GameMesStatGet(fn_80071DF4()) & 0x10) == 0) {
            return;
        }
        lbl_1_data_10 = HuAudBGMPlay(MSM_STREAM_FRANTIC);
        return;
    case 4:
        break;
    case 5:
        if (frame == 0) {
            for (player = 0; player < 4; player++) {
                for (index = 0; index < 5; index++) {
                    espDispOn(lbl_1_bss_A07C[player][index]);
                }
                MgScoreBoxDispSet(lbl_1_bss_A0CC[player], 1);
            }
        }
        if (lbl_1_data_4 == -1 &&
            (playerWork[0]->playerCount >= 5 ||
             playerWork[1]->playerCount >= 5 ||
             playerWork[2]->playerCount >= 5 ||
             playerWork[3]->playerCount >= 5)) {
            lbl_1_data_4 = 0;
            lbl_1_data_8 = 5;
            fn_80071A9C();
            return;
        }
        break;
    case 6:
        if (lbl_1_data_10 != -1 &&
            (GameMesStatGet(fn_80071DF4()) & 0x10) != 0) {
            HuAudSStreamFadeOut(lbl_1_data_10, 0x64);
            lbl_1_data_10 = -1;
        }
        /* Fall through into the winner sequence. */
    case 7:
        if (lbl_1_data_4 != 5) {
            if (lbl_1_bss_14 == 0) {
                if (lbl_1_data_4 == -1) {
                    player = 0;
                    lbl_1_data_8 = -1;
                    for (; player < 4; player++) {
                        lbl_1_data_8 =
                            playerWork[player]->playerCount > lbl_1_data_8
                                ? playerWork[player]->playerCount
                                : lbl_1_data_8;
                    }
                    lbl_1_data_4 = lbl_1_data_8 > 0 ? 0 : 4;
                    lbl_1_data_8 = lbl_1_data_8 > 0 ? lbl_1_data_8 : 1;
                }
                if (lbl_1_bss_34 == 1) {
                    fn_80071B08();
                }
                if (lbl_1_bss_34 == 0x78) {
                    WipeCreate(2, 6, 0x3C);
                    for (player = 0; player < 4; player++) {
                        HuAudFXFadeOut(lbl_1_bss_A104[player + 4].soundHandle, 0x384);
                        HuAudFXFadeOut(lbl_1_bss_A104[player].soundHandle, 0x384);
                    }
                }
                if (lbl_1_bss_34 >= 0xB4) {
                    lbl_1_bss_14 = 1;
                    lbl_1_bss_34 = 0;
                }
            }
            if (lbl_1_bss_14 != 0) {
                if (lbl_1_data_4 == 5) {
                    if (lbl_1_bss_34 == 0) {
                        fn_80071C18(-1, -1, -1, -1);
                    }
                } else {
                    if (lbl_1_bss_34 == 0) {
                        s32 winnerPlayers[4] = {-1, -1, -1, -1};
                        winnerIndex = 0;
                        WipeCreate(1, 6, 0x3C);
                        for (player = 0; player < 4; player++) {
                            Hu3DCameraPerspectiveSet(lbl_1_rodata_30[player],
                                52.0f, 20.0f, 25000.0f, 1.2f);
                            if (player == 0) {
                                Hu3DCameraViewportSet(lbl_1_rodata_30[player],
                                    0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
                                Hu3DCameraScissorSet(lbl_1_rodata_30[player],
                                    0, 0, 0x280, 0x1E0);
                                lbl_80266BD8[0] = 850.0f;
                                lbl_80266B18[0].x = 0.0f;
                                lbl_80266B18[0].y = lbl_1_data_4 != 4 ? 300.0f : 200.0f;
                                lbl_80266B18[0].z = -700.0f;
                                lbl_80266A58[0].x = -2.0f;
                                lbl_80266A58[0].y = 0.0f;
                                lbl_80266A58[0].z = 0.0f;
                            } else {
                                Hu3DCameraViewportSet(lbl_1_rodata_30[player],
                                    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
                                Hu3DCameraScissorSet(lbl_1_rodata_30[player],
                                    0, 0, 0, 0);
                            }
                            fn_80027508(*lbl_1_bss_A0EC[player]->mdlId);
                            fn_800269E8(*lbl_1_bss_A0EC[player]->mdlId, (1 << 0));
                            fn_800269E8(lbl_1_bss_A0EC[player]->mdlId[2], (1 << 0));
                            fn_800269E8(lbl_1_bss_A0EC[player]->mdlId[5], (1 << 0));
                            fn_80026A44(lbl_1_bss_A0EC[player]->mdlId[6], (1 << 0));
                            fn_800269E8(lbl_1_bss_A0EC[player]->mdlId[3], (1 << 0));
                            fn_800269E8(lbl_1_bss_A0EC[player]->mdlId[4], (1 << 0));
                            if (playerWork[player]->playerCount >= lbl_1_data_8) {
                                s32 playerNo;

                                winnerPlayers[winnerIndex] =
                                    playerWork[player]->character;
                                winnerIndex++;
                                playerNo = playerWork[player]->playerNumber;
                                if (fn_8006A124(65551) == 0) {
                                    lbl_80265750[playerNo].mgCoinBonus = 0xA;
                                }
                                fn_80026CBC(lbl_1_bss_A0EC[player]->mdlId[2], 1.0f);
                                fn_80026CBC(lbl_1_bss_A0EC[player]->mdlId[3], 1.0f);
                                fn_80026A44(lbl_1_bss_A0EC[player]->mdlId[1], (1 << 0));
                                fn_80026A44(lbl_1_bss_A0EC[player]->mdlId[2], (1 << 0));
                                fn_8002741C(lbl_1_bss_A0EC[player]->mdlId[1],
                                    "itemhook", lbl_1_bss_A0EC[player]->mdlId[2]);
                                fn_80026E44(lbl_1_bss_A0EC[player]->mdlId[1]);
                            } else {
                                fn_80026E44(lbl_1_bss_A0EC[player]->mdlId[6]);
                            }
                            fn_80026B64(lbl_1_bss_A0EC[player]->mdlId[6], 1U);
                            fn_80026B64(lbl_1_bss_A0EC[player]->mdlId[1], 1U);
                            fn_80026B64(lbl_1_bss_A0EC[player]->mdlId[2], 1U);
                            fn_800269E8(lbl_1_bss_A0EC[player]->mdlId[7], (1 << 0));
                            fn_800269E8(lbl_1_bss_A0EC[player]->mdlId[8], (1 << 0));
                            for (index = 0; index < 5; index++) {
                                fn_800269E8(lbl_1_bss_A0EC[player]->mdlId[index + 9], (1 << 0));
                            }
                            if (lbl_1_bss_A104[player + 4].soundHandle != -1) {
                                HuAudFXStop(lbl_1_bss_A104[player + 4].soundHandle);
                            }
                            if (lbl_1_bss_A104[player].soundHandle != -1) {
                                HuAudFXStop(lbl_1_bss_A104[player].soundHandle);
                            }
                        }
                        if (winnerIndex >= 1 && lbl_1_bss_A104[5].count < 1) {
                            lbl_1_bss_A104[5].count++;
                        }
                        if (winnerIndex >= 2 && lbl_1_bss_A104[4].count < 1) {
                            lbl_1_bss_A104[4].count++;
                        }
                        fn_80026B64(*lbl_1_bss_A0E8->mdlId, 1U);
                        omSetRot(lbl_1_bss_A0E8, 0.0f, 45.0f, 0.0f);
                        if (lbl_1_bss_18 != 0) {
                            fn_800268B0(lbl_1_bss_A0E8->mdlId[2], 0.0f, 45.0f, 0.0f);
                        }
                        fn_80026B64(lbl_1_bss_A0E8->mdlId[2], 1U);
                        fn_80026A44(lbl_1_bss_A0E8->mdlId[3], (1 << 0));
                        fn_80026A44(lbl_1_bss_A0E8->mdlId[3], ((1 << 30) | (1 << 1)));
                        fn_800269E8(lbl_1_bss_A0E8->mdlId[3], ((1 << 30) | (1 << 0)));
                        fn_80026A44(lbl_1_bss_A0E8->mdlId[1], (1 << 0));
                        fn_80026A44(lbl_1_bss_A0E8->mdlId[5], (1 << 0));
                        fn_800269E8(lbl_1_bss_A0E8->mdlId[4], (1 << 0));
                        fn_80026FFC(*lbl_1_bss_A0E8->mdlId);
                        for (player = 0; player < 20; player++) {
                            if (player < 5) {
                                centerX = lbl_1_bss_A1B8[player].x;
                                centerZ = lbl_1_bss_A1B8[player].z;
                                lbl_1_bss_A1B8[player].x = (f32)((f64)centerX * cos(M_PI * 45 / 180) - (f64)centerZ * sin(M_PI * 45 / 180));
                                lbl_1_bss_A1B8[player].z = (f32)((f64)centerX * sin(M_PI * 45 / 180) + (f64)centerZ * cos(M_PI * 45 / 180));
                            }
                            if (player % 5 == 0) {
                                fn_800269E8(lbl_1_bss_A0E8->mdlId[player + 0xB], (1 << 0));
                            } else {
                                fn_80026A44(lbl_1_bss_A0E8->mdlId[player + 0xB], (1 << 0));
                            }
                            fn_80026CBC(lbl_1_bss_A0E8->mdlId[player + 0xB], 1.0f);
                            fn_80026760(lbl_1_bss_A0E8->mdlId[player + 0xB],
                                lbl_1_bss_A1B8[player % 5].x, 0.0f,
                                lbl_1_bss_A1B8[player % 5].z);
                            fn_80026808(lbl_1_bss_A0E8->mdlId[player + 0xB], 0.0f, 45.0f, 0.0f);
                        }
                        fn_80071C18(winnerPlayers[0], winnerPlayers[1],
                            winnerPlayers[2], winnerPlayers[3]);
                        winnerCount = 0;
                        otherCount = 0;
                        firstWinner = 0;
                        otherIndex = 0;
                        for (player = 0; player < 4; player++) {
                            winnerCount += playerWork[player]->playerCount >= lbl_1_data_8;
                        }
                        otherCount = 4 - winnerCount;
                        lbl_80266BD8[0] = 850.0f + (f32)((winnerCount - 1) * 0x96);
                        lbl_80266B18[0].y = (f32)(winnerCount == 4 ? 0x145 : 0x12C);
                        lbl_80266B18[0].z = -700.0f + (f32)((winnerCount - 1) * -0x64);
                        if (otherCount == 4) {
                            lbl_80266B18[0].y = 175.0f;
                        }
                        for (player = 0; player < 4; player++) {
                            xPosition = 300.0f * ((f32)player - 1.5f);
                            zPosition = 0.0f;
                            if (playerWork[player]->playerCount >= lbl_1_data_8) {
                                xPosition = winnerCount == 4 ?
                                    330.0f * ((f32)firstWinner - 1.5f) :
                                    (f32)(((winnerCount - 1) * 0x4B + 0xFA) - firstWinner * 0x14A);
                                zPosition = (f32)(((winnerCount - 1) * -0x64) - 0x2BC);
                                rotation = (f32)(winnerCount == 4 ?
                                    (firstWinner < 2 ? 0x14 : -0xF) - firstWinner * 5 :
                                    firstWinner * 5 - 0x14);
                                playerWork[player]->rotationAngle = 360.0f * ((f32)rand8() / 255.0f);
                                omSetTra(lbl_1_bss_A0EC[player], xPosition, 182.0f, zPosition);
                                fn_80026760(lbl_1_bss_A0EC[player]->mdlId[1], xPosition, 182.0f, zPosition);
                                fn_80026760(lbl_1_bss_A0EC[player]->mdlId[4], xPosition, 7.0f, zPosition);
                                fn_8002D414(lbl_1_bss_A0EC[player]->mdlId[4],
                                    ((f32)rand8() / 255.0f) * fn_8002D530(lbl_1_bss_A0EC[player]->mdlId[4]));
                                fn_80026B64(lbl_1_bss_A0EC[player]->mdlId[4], 1U);
                                fn_80026A44(lbl_1_bss_A0EC[player]->mdlId[4], (1 << 0));
                                fn_80026808(lbl_1_bss_A0EC[player]->mdlId[1], 0.0f, rotation, 0.0f);
                                fn_80026808(lbl_1_bss_A0EC[player]->mdlId[6], 0.0f, rotation, 0.0f);
                                fn_8002C95C(lbl_1_bss_A0EC[player]->mdlId[6], lbl_1_bss_A0EC[player]->mtnId[3]);
                                fn_800269E8(lbl_1_bss_A0EC[player]->mdlId[6], ((1 << 30) | (1 << 1)));
                                firstWinner++;
                            } else {
                                xPosition = otherCount == 4 ?
                                    175.0f * ((f32)otherIndex - 1.5f) :
                                    (f32)((otherCount == 1 ? 0x32 : 0) + ((otherIndex * 0x96 * (otherCount == 2 ? 1 : 1)) + (((winnerCount - 1) * -0xAF) - 0x12C)));
                                zPosition = (f32)(otherCount == 4 ? 0 :
                                    -0xC8 - otherIndex * 0x32 * (otherCount == 2 ? 1 : 1));
                                zPosition += (f32)(((winnerCount - 1) * -0x64) - 0x2BC);
                                fn_80026760(lbl_1_bss_A0EC[player]->mdlId[6],
                                    xPosition, 0.0f, zPosition);
                                fn_80026808(lbl_1_bss_A0EC[player]->mdlId[6], 0.0f, 0.0f, 0.0f);
                                fn_8002C95C(lbl_1_bss_A0EC[player]->mdlId[6], lbl_1_bss_A0EC[player]->mtnId[2]);
                                otherIndex++;
                            }
                        }
                        for (player = 0; player < 4; player++) {
                            for (index = 0; index < 5; index++) {
                                espDispOff(lbl_1_bss_A0A4[player][index]);
                                espDispOff(lbl_1_bss_A07C[player][index]);
                            }
                            MgScoreBoxDispSet(lbl_1_bss_A0CC[player], 0);
                        }
                    }
                    lbl_80266A58[0].y = 0.0f;
                }
                if (lbl_1_bss_34 >= 0x3C) {
                    fn_80071B30();
                    fn_80071A9C();
                    return;
                }
            }
        }
        break;
    case 8:
        if (frame == 0) {
            for (player = 0; player < 4; player++) {
                if (playerWork[player]->playerCount >= lbl_1_data_8) {
                    fn_8002CCC0(lbl_1_bss_A0EC[player]->mdlId[6],
                        lbl_1_bss_A0EC[player]->mtnId[4], 0.0f, 9.0f, 0U);
                    CharFXPlay((s16) playerWork[player]->character,
                               MSM_SE_CHARVOICE_RESULT_WINNER_CHEER);
                } else {
                    fn_8002CCC0(lbl_1_bss_A0EC[player]->mdlId[6],
                        lbl_1_bss_A0EC[player]->mtnId[5], 0.0f, 5.0f, 0U);
                    fn_80026A44(lbl_1_bss_A0EC[player]->mdlId[6], ((1 << 30) | (1 << 0)));
                }
            }
        }
        break;
    case 9:
    case 10:
    case 11:
        break;
    }
}

extern void fn_1_11AC(OMOBJ *obj);

char lbl_1_data_D0[2][16] = {"chara_hook", "wario_hook"};

void fn_1_8180(OMOBJ *obj)
{
    M614PlayerCallbackWork * work;
    s32 order[4] = { 0, 1, 2, 3 };
    s32 player;
    s32 index;
    s32 orderIndex = 0;
    s32 modelIndex;
    s32 hookModel;
    work = obj->data;
    memset(work, 0, 0x200);
    for (index = 0; index < 4; index++)
    {
        for (player = 0; player < 4; player++)
        {
            if (index == lbl_80265728[player].grpNo)
            {
                order[orderIndex] = player;
                orderIndex++;
            }
        }
    }
    work->state = obj->work[0];
    work->playerNumber = order[work->state];
    if (work->playerNumber == 4)
    {
        work->playerNumber = work->state;
    }
    work->character = lbl_80265728[work->playerNumber].charNo;
    work->layoutIndex = work->character >= 0xB ? 9 : work->character;
    work->padIndex = lbl_80265728[work->playerNumber].type != 0 ? - 1 : lbl_80265728[work->playerNumber].padNo;
    lbl_1_bss_1C += work->padIndex != -1;
    work->difficulty = work->padIndex != -1 ? 0 : lbl_80265728[work->playerNumber].comDif;
    work->hookName = lbl_1_data_D0[work->character == 4 || work->character == 6];
    work->actionFrames = 0xFFFF;
    work->elapsedFrames = 0xFFFF;
    work->playerCount = 0;
    for (index = 0; index < 5; index++)
    {
        work->spriteFrame[index] = 0xFFFF;
    }
    work->hookResetPending = work->state == 0 ? 1 : 0;
    work->effectDirection = 0;
    work->characterState = 0;
    work->characterStateFrame = 0;
    work->targetPlayer = - 1;
    work->movementMode = 0;
    work->rotationAngle = 360.0f * ((f32) rand8() / 255.0f);
    work->rotationSpeed = 3.0f + 0.5f * ((f32) rand8() / 255.0f) - 0.25f;
    work->positionX = 1803.105f * (work->state % 2 != 0 ? 1 : - 1);
    work->positionY = 225.0f;
    work->positionZ = 1803.105f * (work->state / 2 != 0 ? 1 : - 1);
    work->movementX = work->movementY = work->turnSpeed = work->headingForward = 0.0f;
    work->rotationX = work->rotationZ = 0.0f;
    work->headingYaw = work->state == 0 ? -0x89 : work->state == 1 ? 0x13B : work->state == 2 ? 0x87 : 0xE1;
    if (work->state == 0)
    {
        work->positionX = 1.5 * (1750.0 * sin(M_PI * -45.0 / 180.0));
        work->positionZ = 1.5 * (1750.0 * cos(M_PI * -45.0 / 180.0));
        work->movementX = 26.666666f * sin((M_PI * (f64) work->headingYaw) / 180.0);
        work->movementY = 26.666666f * cos((M_PI * (f64) work->headingYaw) / 180.0);
        work->positionX += -103.0f;
        work->positionZ += -180.0f;
    }
    {
        s32 modelNumbersNight[2][4] = {{DATANUM(0x490000, 15), DATANUM(0x490000, 16), DATANUM(0x490000, 17), DATANUM(0x490000, 18)}, {DATANUM(0x490000, 19), DATANUM(0x490000, 20), DATANUM(0x490000, 21), DATANUM(0x490000, 22)}};
        obj->mdlId[0] = fn_80025A04(fn_80008DD0(modelNumbersNight[lbl_1_bss_18][work->state], 0x10000000, 2));
    }
    fn_80026B8C(obj->mdlId[0], 3);
    omSetTra(obj, work->positionX, work->positionY, work->positionZ);
    omSetRot(obj, work->rotationX, work->headingYaw, work->rotationZ);
    fn_800269E8(obj->mdlId[0], 1U);
    obj->mtnId[0] = fn_80030458(obj->mdlId[0], fn_80008DD0(DATANUM(0x490000, 23), 0x10000000, 2));
    obj->mtnId[1] = fn_80030458(obj->mdlId[0], fn_80008DD0(DATANUM(0x490000, 24), 0x10000000, 2));
    {
        s32 modelNumbersDay[2][4] = {{DATANUM(0x490000, 25), DATANUM(0x490000, 26), DATANUM(0x490000, 27), DATANUM(0x490000, 28)}, {DATANUM(0x490000, 29), DATANUM(0x490000, 30), DATANUM(0x490000, 31), DATANUM(0x490000, 32)}};
        obj->mdlId[1] = fn_80025A04(fn_80008DD0(modelNumbersDay[lbl_1_bss_18][work->state], 0x10000000, 2));
    }
    fn_80026B8C(obj->mdlId[1], 3);
    fn_800269E8(obj->mdlId[1], 0x40000001U);
    fn_80026B64(obj->mdlId[6], 1U);
    if (work->state != 0)
    {
        fn_800269E8(obj->mdlId[1], 1U);
    }
    obj->mdlId[2] = fn_80025A04(fn_80008DD0(lbl_1_bss_18 != 0 ? DATANUM(0x490000, 36) : DATANUM(0x490000, 35), 0x10000000, 2));
    fn_80026B8C(obj->mdlId[2], 3);
    fn_800269E8(obj->mdlId[2], 0x40000001U);
    fn_8002741C(obj->mdlId[work->hookResetPending ? 1 : 0], "itemhook", obj->mdlId[2]);
    {
        s32 characterModelNumbers[14] = { DATANUM(0x490000, 37), DATANUM(0x490000, 38), DATANUM(0x490000, 39), DATANUM(0x490000, 40), DATANUM(0x490000, 41), DATANUM(0x490000, 42), DATANUM(0x490000, 43), DATANUM(0x490000, 44), DATANUM(0x490000, 45), DATANUM(0x490000, 46), DATANUM(0x490000, 47), DATANUM(0x490000, 48), DATANUM(0x490000, 49), DATANUM(0x490000, 50) };
        obj->mdlId[5] = fn_80025A04(fn_80008DD0(characterModelNumbers[work->character], 0x10000000, 2));
    }
    fn_80026B8C(obj->mdlId[5], 3);
    if (work->hookResetPending != 0)
    {
        fn_80026760(obj->mdlId[5], 0.0f, -1000.0f, 0.0f);
    }
    else
    {
        fn_8002741C(obj->mdlId[0], work->hookName, obj->mdlId[5]);
        fn_800269E8(obj->mdlId[5], 1U);
    }
    if (lbl_1_bss_18 != 0)
    {
        fn_80029E44(obj->mdlId[5], (s16) lbl_1_bss_A0D8);
        work->lightId = fn_800288B4(obj->mdlId[5], &lbl_1_data_5C, lbl_1_bss_18 != 0 ? &lbl_1_data_68 : &lbl_1_data_68, &lbl_1_data_88);
        fn_80028B58(obj->mdlId[5], (s16) work->lightId);
        fn_800296E0(obj->mdlId[5], (s16) work->lightId, 1);
    }
    obj->mdlId[3] = fn_80025A04(fn_80008DD0(DATANUM(0x490000, 33), 0x10000000, 2));
    fn_80026B8C(obj->mdlId[3], 2);
    if (work->state != 0)
    {
        fn_800269E8(obj->mdlId[3], 1U);
    }
    obj->mdlId[4] = fn_80025A04(fn_80008DD0(DATANUM(0x490000, 34), 0x10000000, 2));
    fn_80026B8C(obj->mdlId[4], 3);
    fn_800269E8(obj->mdlId[4], 0x40000001U);
    if (work->state != 0)
    {
        fn_800269E8(obj->mdlId[4], 1U);
    }
    fn_8002D414(obj->mdlId[4], ((f32) rand8() / 255.0f) * fn_8002D530(obj->mdlId[4]));
    fn_80026B64(obj->mdlId[0], * (lbl_1_rodata_30 + work->state));
    fn_80026B64(obj->mdlId[5], * (lbl_1_rodata_30 + work->state));
    fn_80026B64(obj->mdlId[2], * (lbl_1_rodata_30 + work->state));
    if (work->state == 0)
    {
        obj->mdlId[7] = fn_80025A04(fn_80008DD0(DATANUM(0x490000, 12), 0x10000000, 2));
        obj->mdlId[8] = fn_80025D4C(obj->mdlId[7]);
    }
    else
    {
        obj->mdlId[7] = fn_80025D4C(lbl_1_bss_A0EC[0]->mdlId[7]);
        obj->mdlId[8] = fn_80025D4C(lbl_1_bss_A0EC[0]->mdlId[8]);
    }
    fn_80026B8C(obj->mdlId[7], 4);
    fn_80026B8C(obj->mdlId[8], 4);
    fn_800269E8(obj->mdlId[7], 1U);
    fn_800269E8(obj->mdlId[8], 1U);
    for (index = 0; index < 6; index++)
    {
        modelIndex = index + 9;
        if (index == 0) {
            obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(lbl_1_bss_18 != 0 ? DATANUM(0x490000, 11) : DATANUM(0x490000, 11), 0x10000000, 2));
        } else {
            obj->mdlId[modelIndex] = fn_80025D4C(obj->mdlId[9]);
        }
        fn_800269E8(obj->mdlId[modelIndex], 1U);
        fn_80026B8C(obj->mdlId[modelIndex], 3);
        fn_800268B0(obj->mdlId[modelIndex], 1.5f, 1.5f, 1.5f);
    }
    for (index = 0; index < 6; index++)
    {
        modelIndex = index + 0xF;
        if (index == 0) {
            obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(lbl_1_bss_18 != 0 ? DATANUM(0x490000, 10) : DATANUM(0x490000, 10), 0x10000000, 2));
        } else {
            obj->mdlId[modelIndex] = fn_80025D4C(obj->mdlId[0xF]);
        }
        fn_800269E8(obj->mdlId[modelIndex], 1U);
        fn_80026B8C(obj->mdlId[modelIndex], 4);
        fn_800268B0(obj->mdlId[modelIndex], 1.0f, 1.0f, 1.0f);
    }
    {
        unsigned int characterModelData = lbl_8022E718[work->character][1];
        obj->mdlId[6] = fn_80057068((s16) work->character, 2);
    }
    for (index = 2; index < 6; index++)
    {
        s32 motionNumbers[4] = { DATANUM(0x930000, 0), DATANUM(0x8e0000, 32), DATANUM(0x8e0000, 127), DATANUM(0x930000, 7) };
        obj->mtnId[index] = fn_8005B0B8((s16) work->character, motionNumbers[index - 2]);
    }
    if (work->hookResetPending)
    {
        fn_8002741C(obj->mdlId[1], work->hookName, obj->mdlId[6]);
        fn_8002C95C(obj->mdlId[6], obj->mtnId[3]);
    }
    else
    {
        fn_800269E8(obj->mdlId[6], 1U);
    }
    fn_800269E8(obj->mdlId[6], 0x40000001U);
    fn_80026B64(obj->mdlId[6], 1U);
    if (lbl_1_bss_18 != 0)
    {
        fn_80029E44(obj->mdlId[6], (s16) lbl_1_bss_A0D8);
        work->lightId = fn_800288B4(obj->mdlId[6], &lbl_1_data_5C, lbl_1_bss_18 != 0 ? &lbl_1_data_68 : &lbl_1_data_68, &lbl_1_data_88);
        fn_80028B58(obj->mdlId[6], (s16) work->lightId);
        fn_800296E0(obj->mdlId[6], (s16) work->lightId, 1);
    }
    obj->objFunc = fn_1_11AC;
}

/* Builds the night or day sequence models and score display for each player. */
extern void fn_1_A574(HU3D_MODEL *model, Mtx *matrix);

extern void fn_1_55A0(OMOBJ *obj);

void fn_1_91E8(OMOBJ *obj)
{
    void *objectData[4];
    s32 dataIndex;
    s32 playerIndex;
    s32 modelIndex;
    s32 screenX;
    s32 screenY;
    s32 espIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        objectData[playerIndex] = lbl_1_bss_A0EC[playerIndex]->data;
    }

    modelIndex = 0;
    {
        s32 resourceNumber;
        if (lbl_1_bss_18 != 0) {
            resourceNumber = DATANUM(0x490000, 4);
        } else {
            resourceNumber = DATANUM(0x490000, 0);
        }
        obj->mdlId[modelIndex] = fn_80025A04(
            fn_80008DD0(resourceNumber, 0x10000000, 2));
    }
    fn_80026B8C(obj->mdlId[modelIndex], 1);
    fn_80026760(obj->mdlId[modelIndex], 0.0f, 0.0f,
        0.0f);
    fn_800268B0(obj->mdlId[modelIndex], 1.5f, 1.5f,
        1.5f);
    omSetSca(obj, 1.5f, 1.5f, 1.5f);
    fn_800269E8(obj->mdlId[modelIndex], 0x40000002U);
    fn_80026B64(obj->mdlId[modelIndex], 1U);

    modelIndex = 1;
    {
        s32 resourceNumber;
        if (lbl_1_bss_18 != 0) {
            resourceNumber = DATANUM(0x490000, 5);
        } else {
            resourceNumber = DATANUM(0x490000, 1);
        }
        obj->mdlId[modelIndex] = fn_80025A04(
            fn_80008DD0(resourceNumber, 0x10000000, 2));
    }
    fn_80026B8C(obj->mdlId[modelIndex], 1);
    fn_800268B0(obj->mdlId[modelIndex], 1.5f, 1.5f,
        1.5f);
    fn_80026B64(obj->mdlId[modelIndex], 1U);
    fn_800269E8(obj->mdlId[modelIndex], 1U);

    if (lbl_1_bss_18 != 0) {
        modelIndex = 2;
        obj->mdlId[modelIndex] = fn_80025A04(
            fn_80008DD0(DATANUM(0x490000, 8), 0x10000000, 2));
        fn_80026B8C(obj->mdlId[modelIndex], 5);
        fn_800268B0(obj->mdlId[modelIndex], 1.5f,
            1.5f, 1.5f);
        modelIndex = 3;
        obj->mdlId[modelIndex] = fn_80025A04(
            fn_80008DD0(DATANUM(0x490000, 9), 0x10000000, 2));
        fn_80026B8C(obj->mdlId[modelIndex], 1);
        fn_800268B0(obj->mdlId[modelIndex], 1.5f,
            1.5f, 1.5f);
        fn_80026B64(obj->mdlId[modelIndex], 1U);
        fn_800269E8(obj->mdlId[modelIndex], 1U);
    }

    modelIndex = 5;
    {
        s32 resourceNumber;
        if (lbl_1_bss_18 != 0) {
            resourceNumber = DATANUM(0x490000, 6);
        } else {
            resourceNumber = DATANUM(0x490000, 2);
        }
        obj->mdlId[modelIndex] = fn_80025A04(
            fn_80008DD0(resourceNumber, 0x10000000, 2));
    }
    fn_80026B8C(obj->mdlId[modelIndex], 1);
    fn_800268B0(obj->mdlId[modelIndex], 1.5f, 1.5f,
        1.5f);
    fn_80026B64(obj->mdlId[modelIndex], 1U);
    fn_800269E8(obj->mdlId[modelIndex], 1U);

    modelIndex = 4;
    {
        s32 resourceNumber;
        if (lbl_1_bss_18 != 0) {
            resourceNumber = DATANUM(0x490000, 7);
        } else {
            resourceNumber = DATANUM(0x490000, 3);
        }
        obj->mdlId[modelIndex] = fn_80025A04(
            fn_80008DD0(resourceNumber, 0x10000000, 2));
    }
    fn_80026B8C(obj->mdlId[modelIndex], 1);
    fn_800268B0(obj->mdlId[modelIndex], 1.5f, 1.5f,
        1.5f);
    fn_80026B64(obj->mdlId[modelIndex], 3U);

    for (playerIndex = 0; playerIndex < 5; playerIndex++) {
        s32 playerXOffsets[5] = {0, -1250, 1250, 0, 0};
        s32 playerZOffsets[5] = {0, 0, 0, -1250, 1250};
        lbl_1_bss_A1B8[playerIndex].state = 0;
            lbl_1_bss_A1B8[playerIndex].x = 1.5f *
                (f32)playerXOffsets[playerIndex];
            lbl_1_bss_A1B8[playerIndex].z = 1.5f *
                (f32)playerZOffsets[playerIndex];
        lbl_1_bss_A1B8[playerIndex].radius = 300.0f;
    for (dataIndex = 0; dataIndex < 4; dataIndex++) {
            modelIndex = playerIndex + 0xB + dataIndex * 5;
            if (playerIndex == 0 && dataIndex == 0) {
                s32 resourceNumber;
                if (lbl_1_bss_18 != 0) {
                    resourceNumber = DATANUM(0x490000, 14);
                } else {
                    resourceNumber = DATANUM(0x490000, 13);
                }
                obj->mdlId[modelIndex] = fn_80025A04(
                    fn_80008DD0(resourceNumber, 0x10000000,
                        2));
            } else {
                obj->mdlId[modelIndex] = fn_80025D4C(obj->mdlId[0xB]);
            }
            fn_80026760(obj->mdlId[modelIndex],
                lbl_1_bss_A1B8[playerIndex].x, 0.0f,
                lbl_1_bss_A1B8[playerIndex].z);
            fn_800268B0(obj->mdlId[modelIndex], 1.5f,
                1.5f, 1.5f);
            fn_800269E8(obj->mdlId[modelIndex], 0x40000002U);
            fn_800269E8(obj->mdlId[modelIndex], 2U);
            fn_80026B8C(obj->mdlId[modelIndex], 4);
            fn_80026CBC(obj->mdlId[modelIndex], 1.0f);
            fn_80026B64(obj->mdlId[modelIndex],
                (u16)lbl_1_rodata_30[dataIndex]);
        }
    }

    lbl_1_bss_A0D4 = (s32)espEntry(DATANUM(0x490000, 51), 0, 0);
    espDispOff((s16)lbl_1_bss_A0D4);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        modelIndex = playerIndex + 7;
        obj->mdlId[modelIndex] = fn_800260F4(
            (void (*)(HU3D_MODEL *, Mtx *))fn_1_A574);
        fn_80026B8C(obj->mdlId[modelIndex], 2);
        fn_80026B64(obj->mdlId[modelIndex],
            (u16)lbl_1_rodata_30[playerIndex]);
    }

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        screenX = (playerIndex % 2) != 0 ? 0x1F6 : 0x4A;
        screenY = (playerIndex / 2) != 0 ? 0x1A6 : 0xDE;
        for (dataIndex = 0; dataIndex < 5; dataIndex++) {
            for (modelIndex = 0; modelIndex < 2; modelIndex++) {
                espIndex = espEntry(
                    modelIndex != 0 ? DATANUM(0x9b0000, 31) : DATANUM(0x9b0000, 30), 0, 0);
                espDispOff((s16)espIndex);
                espAttrSet((s16)espIndex, 1U);
                espBankSet((s16)espIndex, 0);
                espPosSet((s16)espIndex,
                    (f32)(screenX + (dataIndex - 2) * 0x14), (f32)screenY);
                espScaleSet((s16)espIndex, 0.875f,
                    0.875f);
                espPriSet((s16)espIndex, (s16)(0x60 - playerIndex));
                if (modelIndex != 0) {
                    lbl_1_bss_A0A4[playerIndex][dataIndex] = (s16)espIndex;
                } else {
                    lbl_1_bss_A07C[playerIndex][dataIndex] = (s16)espIndex;
                }
                if (modelIndex == 0) {
                    espTPLvlSet((s16)espIndex, 0.75f);
                }
            }
        }
        lbl_1_bss_A0CC[playerIndex] = MgScoreBoxCreate(0x76, 0x20);
        MgScoreBoxPosSet(lbl_1_bss_A0CC[playerIndex], (f32)screenX,
            (f32)screenY);
        MgScoreBoxDispSet(lbl_1_bss_A0CC[playerIndex], 0);
    }

    lbl_1_bss_A1A4.values[0] = 0;
    lbl_1_bss_A1A4.values[1] = 0;
    lbl_1_bss_A1A4.values[2] = (s32)(60.0f *
        (25.0f + (10.0f *
            ((f32)rand8() / 255.0f))));
    lbl_1_bss_A1A4.values[2] = -1;
    obj->objFunc = fn_1_55A0;
}


void fn_1_9D78(OMOBJ *obj)
{
    obj->objFunc = fn_1_656C;
}

void ObjectSetup(void)
{
    Vec lightPosition;
    Vec shadowPosition;
    Vec shadowUp;
    Vec shadowTarget;
    s32 i;
    s16 nightMode;
    s32 cameraColumn;
    s32 cameraRow;
    s32 viewportHeight;
    s32 viewportWidth;

    OSReport("******* M614 ObjectSetup *********\n");
    nightMode = lbl_802C0278;
    if (nightMode == 1) {
        lbl_1_bss_18 = 1;
    }
    lbl_1_bss_A0E0 = fn_80080418();
    for (i = 0; i < 0x14; i++) {
        lbl_1_bss_A104[i].count = 0;
        lbl_1_bss_A104[i].soundHandle = -1;
    }
    Hu3DCameraCreate(0xF);
    for (i = 0; i < 4; i++) {
        cameraColumn = i % 2;
        cameraRow = i / 2;
        viewportHeight = 0x10;
        viewportWidth = 0x28;
        Hu3DCameraPerspectiveSet(lbl_1_rodata_30[i], 45.0f,
            20.0f, 25000.0f, 1.2f);
        if (i == 0) {
            Hu3DCameraViewportSet(lbl_1_rodata_30[i], 0.0f,
                0.0f, 640.0f, 480.0f,
                0.0f, 1.0f);
            Hu3DCameraScissorSet(lbl_1_rodata_30[i], 0U, 0U, 0x280U,
                0x1E0U);
            lbl_80266BD8[i] = 750.0f;
            lbl_80266B18[i].x = 0.0f;
            lbl_80266B18[i].y = 230.0f;
            lbl_80266B18[i].z = 0.0f;
            lbl_80266A58[i].x = -50.0f;
            lbl_80266A58[i].y = 120.0f;
            lbl_80266A58[i].z = 0.0f;
        } else {
            Hu3DCameraViewportSet(lbl_1_rodata_30[i], 0.0f,
                0.0f, 0.0f, 0.0f,
                0.0f, 1.0f);
            Hu3DCameraScissorSet(lbl_1_rodata_30[i], 0U, 0U, 0U, 0U);
            lbl_80266BD8[i] = 1000.0f;
            lbl_80266B18[i].x = 0.0f;
            lbl_80266B18[i].y = 380.0f;
            lbl_80266B18[i].z = 0.0f;
            lbl_80266A58[i].x = -19.0f;
            lbl_80266A58[i].y = 0.0f;
            lbl_80266A58[i].z = 0.0f;
        }
    }
    fn_800285BC();
    lbl_1_bss_A0D8 = (s32)fn_800286D0(&lbl_1_data_5C,
        lbl_1_bss_18 != 0 ? &lbl_1_data_74 : &lbl_1_data_68,
        lbl_1_bss_18 != 0 ? &lbl_1_data_80[0] : &lbl_1_data_80[0]);
    Hu3DGLightStaticSet((s16)lbl_1_bss_A0D8, 1);
    Hu3DGLightInfinitytSet((s16)lbl_1_bss_A0D8);
    lightPosition.x = 0.0f;
    lightPosition.y = 10000.0f;
    lightPosition.z = -500.0f;
    shadowPosition.x = 0.0f;
    shadowPosition.y = 1.0f;
    shadowPosition.z = 0.0f;
    shadowUp.x = 0.0f;
    shadowUp.y = 0.0f;
    shadowUp.z = -700.0f;
    fn_8002A3B0(8.0f, 5000.0f,
        13000.0f, 1);
    fn_8002A7D8(0.5f, 1);
    fn_8002A674(&lightPosition, &shadowPosition, &shadowUp, 1);
    fn_8002AA3C(0U, 0U, 0U, 1);
    lbl_1_bss_A0DC = -1;
    lbl_1_data_4 = -1;
    fn_80070788(&lbl_1_data_18);
}




void fn_1_A38C(s16 mode, s16 frameNo)
{
    s32 i;
    OMOBJ *obj;

    lbl_1_bss_A0E8 = fn_8003AB38(lbl_1_bss_A0E0, 0x65, 0x20U, 2U, 1,
        fn_1_91E8);
    for (i = 0; i < 4; i++) {
        obj = lbl_1_bss_A0EC[i] = fn_8003AB38(lbl_1_bss_A0E0,
            0x64, 0x15U, 6U, 0, fn_1_8180);
        obj->data = HuMemDirectMallocNum(HEAP_HEAP, 0x200, 0x10000000);
        obj->work[0] = (u32)i;
    }
    lbl_1_bss_A0FC = fn_8003AB38(lbl_1_bss_A0E0, 0x7FDA, 0U, 0U, -1,
        fn_8003DAA4);
    lbl_1_bss_A0FC->work[0] = 4;
    lbl_1_bss_A100 = fn_8003AB38(lbl_1_bss_A0E0, 0x66, 0U, 0U, -1,
        fn_1_9D78);
    fn_80071A9C();
}

void fn_1_A4D8(s16 mode, s16 frameNo)
{
    void *work;
    s32 i;

    fn_80071A9C();
    i = 0;
    while (i < 4) {
        work = lbl_1_bss_A0EC[i]->data;
        i += 1;
    }
    if (lbl_1_bss_A0DC >= 0) {
        GameMesKill(lbl_1_bss_A0DC);
    }
    GameMesClose();
    HuAudAllStop();
    fn_8003A5E4((s16)lbl_1_data_14, 1);
}

typedef struct {
    s16 groupMemberIndex;
    s16 animationSlot;
} M614SpriteSlot;

typedef struct {
    u32 dataNumber;
    u16 referenceCount;
    u16 alignment;
    ANIMDATA *animation;
} M614AnimationSlot;

extern M614SpriteSlot esprite[HUSPR_MAX];

extern M614AnimationSlot espanim[HUSPR_MAX];

extern void HuSprTexLoad(ANIMDATA *animation, s16 image, s16 map, GXTexWrapMode wrapS, GXTexWrapMode wrapT, GXTexFilter filter);

void fn_1_A574(HU3D_MODEL *model, Mtx *matrix)
{
    s32 mode;
    M614PlayerCallbackWork *work;
    s32 drawIndex;
    s32 x;
    s32 yOffset;
    s32 y;
    s32 alpha;
    s32 spriteIndex;
    f32 halfSize;

    alpha = 0x60;
    mode = fn_80071A8C();
    GXLoadPosMtxImm(*matrix, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
        GX_CC_RASC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_RASA, GX_CA_ZERO, GX_CA_TEXA,
        GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        1, GX_TEVPREV);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP,
        GX_AF_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
        GX_LO_NOOP);
    GXSetZMode(1, GX_LEQUAL, 0);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    HuSprTexLoad(espanim[esprite[lbl_1_bss_A0D4].animationSlot].animation,
        0, 0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    if (lbl_1_bss_14 == 0) {
        for (spriteIndex = 0; spriteIndex < 4; spriteIndex++) {
            work = lbl_1_bss_A0EC[spriteIndex]->data;
            for (drawIndex = 0; drawIndex < 6; drawIndex++) {
                if (work->movingEffects[drawIndex].state == 1) {
                    halfSize = work->movingEffects[drawIndex].progress;
                    halfSize = 50.0f;
                    x = (s32)work->movingEffects[drawIndex].x;
                    yOffset = 0xC;
                    y = (s32)work->movingEffects[drawIndex].z;
                    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                    GXWGFifo.f32 = (f32)x - halfSize;
                    GXWGFifo.f32 = (f32)yOffset;
                    GXWGFifo.f32 = (f32)y - halfSize;
                    GXWGFifo.u8 = 0;
                    GXWGFifo.u8 = 0;
                    GXWGFifo.u8 = 0;
                    GXWGFifo.u8 = alpha;
                    GXWGFifo.f32 = 0.0f;
                    GXWGFifo.f32 = 0.0f;
                    GXWGFifo.f32 = (f32)x + halfSize;
                    GXWGFifo.f32 = (f32)yOffset;
                    GXWGFifo.f32 = (f32)y - halfSize;
                    GXWGFifo.u8 = 0;
                    GXWGFifo.u8 = 0;
                    GXWGFifo.u8 = 0;
                    GXWGFifo.u8 = alpha;
                    GXWGFifo.f32 = 1.0f;
                    GXWGFifo.f32 = 0.0f;
                    GXWGFifo.f32 = (f32)x + halfSize;
                    GXWGFifo.f32 = (f32)yOffset;
                    GXWGFifo.f32 = (f32)y + halfSize;
                    GXWGFifo.u8 = 0;
                    GXWGFifo.u8 = 0;
                    GXWGFifo.u8 = 0;
                    GXWGFifo.u8 = alpha;
                    GXWGFifo.f32 = 1.0f;
                    GXWGFifo.f32 = 1.0f;
                    GXWGFifo.f32 = (f32)x - halfSize;
                    GXWGFifo.f32 = (f32)yOffset;
                    GXWGFifo.f32 = (f32)y + halfSize;
                    GXWGFifo.u8 = 0;
                    GXWGFifo.u8 = 0;
                    GXWGFifo.u8 = 0;
                    GXWGFifo.u8 = alpha;
                    GXWGFifo.f32 = 0.0f;
                    GXWGFifo.f32 = 1.0f;
                }
            }
        }
    }
}

f32 lbl_1_data_114[3] = {0.0f, -1.0f, 0.0f};
f32 lbl_1_data_120[3] = {-2.0f, -1.0f, 0.0f};

void fn_1_ABF4(void *model, Mtx *matrix)
{
    Mtx matrices[2];
    f32 delta;
    f32 angle;
    f32 x;
    f32 y;
    f32 z;
    f32 screenX;
    f32 screenY;
    f32 length;
    s32 frame;
    s32 axis;
    s32 baseColor;
    s32 red;
    s32 green;
    s32 blue;
    s32 alpha;
    s8 stickX;
    s8 stickY;

    baseColor = 0;
    alpha = 0xFF;
    GXLoadPosMtxImm(*matrix, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,
        GX_PTIDENTITY);
    GXSetZMode(0, GX_ALWAYS, 0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP,
        GX_AF_NONE);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1,
        GX_TEVPREV);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
        GX_CC_RASC);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1,
        GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_RASA);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
        GX_LO_NOOP);
    GXSetZMode(0, GX_ALWAYS, 0);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetZCompLoc(0);
    GXSetScissor(0, 0, 0x280, 0x1E0);
    GXSetViewport(0.0f, 0.0f, 640.0f,
        480.0f, 0.0f, 1.0f);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    C_MTXOrtho(matrices[0], 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
        -1.0f);
    GXSetProjection(matrices[0], GX_ORTHOGRAPHIC);
    PSMTXIdentity(matrices[1]);
    GXLoadPosMtxImm(matrices[1], GX_PNMTX0);
    baseColor = 0;
    alpha = 0xFF;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXWGFifo.f32 = 0.0f;
    GXWGFifo.f32 = 0.0f;
    GXWGFifo.f32 = 0.0f;
    GXWGFifo.u8 = (u8)baseColor;
    GXWGFifo.u8 = (u8)baseColor;
    GXWGFifo.u8 = (u8)baseColor;
    GXWGFifo.u8 = (u8)alpha;
    GXWGFifo.f32 = 0.0f;
    GXWGFifo.f32 = 0.0f;
    GXWGFifo.f32 = 1.0f;
    GXWGFifo.f32 = 0.0f;
    GXWGFifo.f32 = 0.0f;
    GXWGFifo.u8 = (u8)baseColor;
    GXWGFifo.u8 = (u8)baseColor;
    GXWGFifo.u8 = (u8)baseColor;
    GXWGFifo.u8 = (u8)alpha;
    GXWGFifo.f32 = 1.0f;
    GXWGFifo.f32 = 0.0f;
    GXWGFifo.f32 = 1.0f;
    GXWGFifo.f32 = 1.0f;
    GXWGFifo.f32 = 0.0f;
    GXWGFifo.u8 = (u8)baseColor;
    GXWGFifo.u8 = (u8)baseColor;
    GXWGFifo.u8 = (u8)baseColor;
    GXWGFifo.u8 = (u8)alpha;
    GXWGFifo.f32 = 1.0f;
    GXWGFifo.f32 = 1.0f;
    GXWGFifo.f32 = 0.0f;
    GXWGFifo.f32 = 1.0f;
    GXWGFifo.f32 = 0.0f;
    GXWGFifo.u8 = (u8)baseColor;
    GXWGFifo.u8 = (u8)baseColor;
    GXWGFifo.u8 = (u8)baseColor;
    GXWGFifo.u8 = (u8)alpha;
    GXWGFifo.f32 = 0.0f;
    GXWGFifo.f32 = 1.0f;

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    for (frame = 0; frame < 3; frame++) {
        red = 0;
        green = 0;
        blue = 0;
        lbl_1_bss_38.frame++;
        lbl_1_bss_38.frame %= 0x3C;
        if (frame < 2) {
            if (frame == 0) {
                stickX = lbl_802BFFFC[0];
            } else {
                stickX = lbl_802BFFF4[0];
            }
            lbl_1_data_114[frame] +=
                ((f32)stickX / 64.0f) / 60.0f;
            if (frame == 0) {
                stickY = lbl_802BFFF8[0];
            } else {
                stickY = lbl_802BFFF0[0];
            }
            lbl_1_data_120[frame] -=
                ((f32)stickY / 64.0f) / 60.0f;
        } else {
            y = lbl_1_data_120[0];
            x = lbl_1_data_114[0];
            length = sqrtf(x * x + y * y);
            angle = length;
            angle = (lbl_1_data_114[0] * lbl_1_data_114[1] +
                lbl_1_data_120[0] * lbl_1_data_120[1]) /
                (angle * angle);
            lbl_1_data_114[frame] = angle * lbl_1_data_114[0];
            lbl_1_data_120[frame] = angle * lbl_1_data_120[0];
        }
        red = frame == 0 ? 0xFF : 0;
        green = frame == 2 ? 0xFF : 0;
        blue = frame == 1 ? 0xFF : 0;
        alpha = 0xC0;
        GXSetLineWidth((u8)(frame < 2 ? 0x1E : 0xC), GX_TO_ONE);
        GXBegin(GX_LINES, GX_VTXFMT0, 2);
        screenY = 0.5f + lbl_1_data_120[frame] / 8.0f;
        screenX = 0.5f +
            0.9f * (lbl_1_data_114[frame] / 8.0f);
        GXWGFifo.f32 = screenX;
        GXWGFifo.f32 = screenY;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u8 = (u8)red;
        GXWGFifo.u8 = (u8)green;
        GXWGFifo.u8 = (u8)blue;
        GXWGFifo.u8 = (u8)alpha;
        GXWGFifo.f32 = 1.0f;
        GXWGFifo.f32 = 1.0f;
        GXWGFifo.f32 = 0.5f;
        GXWGFifo.f32 = 0.5f;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u8 = (u8)red;
        GXWGFifo.u8 = (u8)green;
        GXWGFifo.u8 = (u8)blue;
        GXWGFifo.u8 = (u8)alpha;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.f32 = 1.0f;
        if (lbl_1_bss_38.frame == 0) {
            OSReport("%d: %f %f\n", frame, lbl_1_data_114[frame],
                lbl_1_data_120[frame]);
        }
    }
}

float fn_1_B78C(double value) {
    return fabs(value);
}

M614Obstacle lbl_1_bss_A1B8[5];
M614SequenceState lbl_1_bss_A1A4;
M614SoundQueueEntry lbl_1_bss_A104[20];
OMOBJ *lbl_1_bss_A100;
OMOBJ *lbl_1_bss_A0FC;
OMOBJ *lbl_1_bss_A0EC[4];
OMOBJ *lbl_1_bss_A0E8;
s32 lbl_1_bss_A0E4;
OMOBJMAN *lbl_1_bss_A0E0;
s16 lbl_1_bss_A0DC;
s32 lbl_1_bss_A0D8;
s32 lbl_1_bss_A0D4;
s16 lbl_1_bss_A0CC[4];
s16 lbl_1_bss_A0A4[4][5];
s16 lbl_1_bss_A07C[4][5];
CallbackStorage lbl_1_bss_A060;
M614DebugStorage lbl_1_bss_38;
s32 lbl_1_bss_34;
M614CameraOffsetStorage lbl_1_bss_28;
f32 lbl_1_bss_24;
f32 lbl_1_bss_20;
s32 lbl_1_bss_1C;
s32 lbl_1_bss_18;
s32 lbl_1_bss_14;
s32 lbl_1_bss_10;
M614FrameStorage lbl_1_bss_0;
