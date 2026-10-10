/* Collision checks, obstacle reactions, and obstacle drawing for Throw Me a Bone. */
#include "REL/m650/m650.h"

void fn_1_A0(void);
void fn_1_F0(s16 mode, s16 frame);
void fn_1_168(s16 mode, s16 frame);
void fn_1_188(s16 mode, s16 frame);
void fn_1_1C8(s16 mode, s16 frame);
void fn_1_200(s16 mode, s16 frame);
void fn_1_304(s16 mode, s16 frame);
void fn_1_3AC(s16 mode, s16 frame);
void fn_1_3B0(s16 mode, s16 frame);
void fn_1_3B4(s16 mode, s16 frame);
void fn_1_3B8(void);
void fn_1_484(void);
void fn_1_550(void);
void fn_1_8EC(void);
void fn_1_F90(OMOBJ *object);
void fn_1_1228(void);
void fn_1_14FC(void);
void fn_1_1830(void);
void fn_1_1AE4(void);
void fn_1_2010(s16 player);
void fn_1_21CC(OMOBJ *obj);
void fn_1_22A8(s16 player, s16 motion);
void fn_1_2320(s16 player, s16 motion);
void fn_1_23C4(s16 player, s16 motion, float blend);
void fn_1_2464(s16 player);
void fn_1_2558(void);
void fn_1_25B0(void);
void fn_1_26D0(void);
void fn_1_273C(s16 player);
void fn_1_289C(s16 player);
s16 fn_1_2CAC(s16 player);
void fn_1_2F1C(s16 player);
void fn_1_33F8(s16 player);
void fn_1_3F28(s16 player);
void fn_1_4430(s16 player);
void fn_1_473C(s16 player);
void fn_1_48C0(s16 player);
void fn_1_49C8(s16 player);
void fn_1_4BD4(s16 player);
s16 fn_1_4D58(s16 frame);
void fn_1_5258(void);
void fn_1_5308(void);
void fn_1_5354(void);
void fn_1_5394(void);
void fn_1_54D4(void);
void fn_1_5544(s16 player);
void fn_1_5600(s16 player, f32 value, f32 *out0, f32 *out1);
s16 fn_1_5728(void);
void fn_1_5A24(s16 player);
void fn_1_5E40(void);
void fn_1_62E0(void);
void fn_1_6478(HU3D_MODEL *model, Mtx *mtx);
s16 fn_1_69B0(s16 player);
s16 fn_1_6D88(s16 player, HuVecF *position);
s16 fn_1_6EE4(HuVecF *firstPosition, HuVecF *secondPosition, float radius);
void fn_1_6F54(void);
s16 fn_1_7000(s16 player, HuVecF *position, float angle, s16 obstacleIndex);

extern GXColor lbl_1_data_40[2];
extern Point3d lbl_1_data_28;
extern Point3d lbl_1_data_34;
extern s16 lbl_1_bss_4AC[4];
extern s16 lbl_1_bss_4B4;
extern s16 lbl_1_bss_4B6;
extern s32 lbl_1_bss_2A8;
extern s32 lbl_1_data_48[4];
extern s32 lbl_1_data_80[2];
extern s32 lbl_1_data_88[2];
extern s32 lbl_1_data_90[2];
extern s32 lbl_1_data_98[2];
extern s32 lbl_1_data_A0[2];

M650Point lbl_1_data_1F8[5] = {
    { { -550.0f, 0.0f, 1400.0f }, 0 },
    { { -350.0f, 0.0f, 1800.0f }, 1 },
    { { 0.0f, 0.0f, 2000.0f }, 0 },
    { { 350.0f, 0.0f, 1800.0f }, 1 },
    { { 550.0f, 0.0f, 1400.0f }, 0 },
};
M650Point lbl_1_data_248[32] = {
    { { -700.0f, 0.0f, 1000.0f }, 0 },
    { { 0.0f, 0.0f, 1000.0f }, 0 },
    { { 700.0f, 0.0f, 1000.0f }, 0 },
    { { -1050.0f, 0.0f, 1500.0f }, 0 },
    { { -350.0f, 0.0f, 1500.0f }, 0 },
    { { 350.0f, 0.0f, 1500.0f }, 0 },
    { { 1500.0f, 0.0f, 1500.0f }, 0 },
    { { -700.0f, 0.0f, 1950.0f }, 0 },
    { { 700.0f, 0.0f, 1950.0f }, 0 },
    { { -1000.0f, 0.0f, 2450.0f }, 0 },
    { { -600.0f, 0.0f, 2450.0f }, 0 },
    { { -200.0f, 0.0f, 2450.0f }, 0 },
    { { 200.0f, 0.0f, 2450.0f }, 0 },
    { { 600.0f, 0.0f, 2450.0f }, 0 },
    { { 1000.0f, 0.0f, 2450.0f }, 0 },
    { { -700.0f, 0.0f, 2900.0f }, 1 },
    { { 0.0f, 0.0f, 2900.0f }, 1 },
    { { 700.0f, 0.0f, 2900.0f }, 1 },
    { { -1050.0f, 0.0f, 3400.0f }, 1 },
    { { -350.0f, 0.0f, 3400.0f }, 1 },
    { { 350.0f, 0.0f, 3400.0f }, 1 },
    { { 1050.0f, 0.0f, 3400.0f }, 1 },
    { { -700.0f, 0.0f, 3850.0f }, 1 },
    { { 0.0f, 0.0f, 3850.0f }, 1 },
    { { 700.0f, 0.0f, 3850.0f }, 1 },
    { { -1050.0f, 0.0f, 4300.0f }, 1 },
    { { -350.0f, 0.0f, 4300.0f }, 1 },
    { { 350.0f, 0.0f, 4300.0f }, 1 },
    { { 1050.0f, 0.0f, 4300.0f }, 1 },
    { { -700.0f, 0.0f, 4750.0f }, 1 },
    { { 0.0f, 0.0f, 4750.0f }, 1 },
    { { 700.0f, 0.0f, 4750.0f }, 1 },
};
s32 lbl_1_data_448[2] = { DATANUM(DATA_m650, 2), DATANUM(DATA_m650, 8) };
s32 lbl_1_data_450[2] = { DATANUM(DATA_m650, 3), DATANUM(DATA_m650, 9) };

s16 lbl_1_bss_4B6;
s16 lbl_1_bss_4B4;
s16 lbl_1_bss_4AC[4];
M650Cell lbl_1_bss_2AC[32][4];
s32 lbl_1_bss_2A8;

/* Called from fn_1_F0 during round setup to load obstacle models and install camera draw hooks. */
void fn_1_62E0(void)
{
    u16 cameraMasks[4] = { 1, 2, 4, 8 };
    s32 cameraIndex;

    lbl_1_bss_2A8 = 0;
    memset(lbl_1_bss_2AC, 0, 512U);
    lbl_1_bss_4B6 = Hu3DModelCreate(
        HuDataSelHeapReadNum(lbl_1_data_448[lbl_1_bss_0.night], 1 << 28, HEAP_MODEL));
    lbl_1_bss_4B4 = Hu3DModelCreate(
        HuDataSelHeapReadNum(lbl_1_data_450[lbl_1_bss_0.night], 1 << 28, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_4B6, HU3D_CAM_ALL);
    Hu3DModelCameraSet(lbl_1_bss_4B4, HU3D_CAM_ALL);
    Hu3DModelAttrSet(lbl_1_bss_4B6, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_4B4, HU3D_ATTR_DISPOFF);
    cameraIndex = 0;
    while (cameraIndex < 4) {
        lbl_1_bss_4AC[cameraIndex] = Hu3DHookFuncCreate(fn_1_6478);
        Hu3DModelCameraSet(lbl_1_bss_4AC[cameraIndex], cameraMasks[cameraIndex]);
        cameraIndex += 1;
    }
}

/* Camera draw hook installed by fn_1_62E0; draws each obstacle and record prop in its current
 * state. */
void fn_1_6478(HU3D_MODEL *ignoredModel, Mtx *parentTransform)
{
    Mtx transform;
    HSF_DATA *sceneData;
    HU3D_MODEL *modelData;
    M650Cell *cell;
    s32 pointIndex;
    s32 row;
    s32 column;
    M650Player *currentPlayer;
    s16 motionModel;
    f32 recordZOffset;
    f32 motionFrame;
    f32 motionEndFrame;
    HSF_OBJECT *treeObject;
    HSF_OBJECT *rockObject;
    HSF_OBJECT *shadowObject;

    Hu3DModelObjDrawInit();
    modelData = &Hu3DData[lbl_1_bss_4B6];
    /* This scene pointer is fetched here but the draw decisions use named HSF objects. */
    sceneData = modelData->hsf;
    treeObject = Hu3DModelObjPtrGet(lbl_1_bss_4B6, "m650_03N");
    rockObject = Hu3DModelObjPtrGet(lbl_1_bss_4B4, "m650_04N");
    shadowObject = Hu3DModelObjPtrGet(lbl_1_bss_4B6, "shadow");
    pointIndex = 0;
    while (pointIndex < 32) {
        cell = &lbl_1_bss_2AC[pointIndex][lbl_1_bss_2A8];
        /* Broken props stay hidden from this camera pass. */
        if (cell->state != 2) {
                switch (cell->state) {
                case 2:
                    break;
                case 3:
                    /* This loop writes four rows even though the affine matrix has only three. */
                    row = 0;
                    while (row < 4) {
                        column = 0;
                        while (column < 4) {
                            if (row == column) {
                                transform[row][column] = 1.0f;
                            } else {
                                transform[row][column] = 0.0f;
                            }
                            column += 1;
                        }
                        row += 1;
                    }
                    if (lbl_1_bss_0.recordChanged != 0) {
                        recordZOffset = 350.0f;
                    } else {
                        recordZOffset = 0.0f;
                    }
                    mtxTransCat(transform, lbl_1_data_1F8[pointIndex].pos.x,
                                0.5f + lbl_1_data_1F8[pointIndex].pos.y,
                                recordZOffset + lbl_1_data_1F8[pointIndex].pos.z);
                    PSMTXConcat(*parentTransform, transform, transform);
                    if (lbl_1_data_1F8[pointIndex].isRock == 0) {
                        Hu3DModelObjPtrDraw(lbl_1_bss_4B6, treeObject, transform);
                    } else {
                        Hu3DModelObjPtrDraw(lbl_1_bss_4B4, rockObject, transform);
                    }
                    Hu3DModelObjPtrDraw(lbl_1_bss_4B6, shadowObject, transform);
                    break;
                default:
                    break;
                case 0:
                    /* This branch also writes four rows into a three-row affine matrix. */
                    row = 0;
                    while (row < 4) {
                        column = 0;
                        while (column < 4) {
                            if (row == column) {
                                transform[row][column] = 1.0f;
                            } else {
                                transform[row][column] = 0.0f;
                            }
                            column += 1;
                        }
                        row += 1;
                    }
                    mtxTransCat(transform, lbl_1_data_248[pointIndex].pos.x,
                                0.5f + lbl_1_data_248[pointIndex].pos.y,
                                lbl_1_data_248[pointIndex].pos.z);
                    PSMTXConcat(*parentTransform, transform, transform);
                    if (lbl_1_data_248[pointIndex].isRock == 0) {
                        Hu3DModelObjPtrDraw(lbl_1_bss_4B6, treeObject, transform);
                    } else {
                        Hu3DModelObjPtrDraw(lbl_1_bss_4B4, rockObject, transform);
                    }
                    Hu3DModelObjPtrDraw(lbl_1_bss_4B6, shadowObject, transform);
                    break;
                case 1:
                    currentPlayer = &lbl_1_bss_28[lbl_1_bss_2A8];
                    if (lbl_1_data_248[pointIndex].isRock == 0) {
                        motionModel = currentPlayer->model7A;
                    } else {
                        motionModel = currentPlayer->model7C;
                    }
                    motionFrame = Hu3DMotionTimeGet(motionModel);
                    motionEndFrame = Hu3DMotionMaxTimeGet(motionModel);
                    if (motionFrame >= motionEndFrame) {
                        if ((cell->timer % 4) == 0) {
                            Hu3DModelAttrSet(motionModel, HU3D_ATTR_DISPOFF);
                        } else if ((cell->timer % 4) == 2) {
                            Hu3DModelAttrReset(motionModel, HU3D_ATTR_DISPOFF);
                            /* The same model flag is reset a second time here. */
                            Hu3DModelAttrReset(motionModel, HU3D_ATTR_DISPOFF);
                        }
                        cell->timer += 1;
                        if (cell->timer >= 30) {
                            Hu3DModelAttrSet(motionModel, HU3D_ATTR_DISPOFF);
                            cell->state = 2;
                        }
                    }
                    break;
                }
        }
        pointIndex += 1;
    }
    /* Advance the player slot once per draw-hook call; the slot is not derived from camera
     * identity. */
    lbl_1_bss_2A8 = (lbl_1_bss_2A8 + 1) % 4;
}

/* Called after the runner moves toward the bone; detects prop or lane-wall collisions and stores
 * the runner's reflected rebound direction. */
s16 fn_1_69B0(s16 playerNo)
{
    M650Player *playerData = &lbl_1_bss_28[playerNo];
    HuVecF playerPosition;
    HuVecF reflectedDirection;
    HuVecF collisionNormal;
    s32 wallSounds[4] = { M650_EFFECT_2040, M650_EFFECT_2041, M650_EFFECT_2042, M650_EFFECT_2043 };
    s32 treeSounds[4] = { M650_EFFECT_2044, M650_EFFECT_2045, M650_EFFECT_2046, M650_EFFECT_2047 };
    s32 rockSounds[4] = { M650_EFFECT_2048, M650_EFFECT_2049, M650_EFFECT_2050, M650_EFFECT_2051 };
    int obstacleIndex;
    s16 obstacleEffectModel;

    Hu3DModelPosGet(playerData->model20, &playerPosition);
    for (obstacleIndex = 0; obstacleIndex < 32; obstacleIndex++) {
        if (lbl_1_bss_2AC[obstacleIndex][playerNo].state == 0 &&
            fn_1_6EE4(&playerPosition, &lbl_1_data_248[obstacleIndex].pos, 150.0f)) {
            if (lbl_1_data_248[obstacleIndex].isRock == 0) {
                obstacleEffectModel = playerData->model7A;
            } else {
                obstacleEffectModel = playerData->model7C;
            }
            Hu3DModelPosSetV(obstacleEffectModel, &lbl_1_data_248[obstacleIndex].pos);
            Hu3DMotionTimeSet(obstacleEffectModel, 0.0f);
            Hu3DMotionSpeedSet(obstacleEffectModel, 1.0f);
            Hu3DModelAttrReset(obstacleEffectModel, HU3D_ATTR_DISPOFF);
            Hu3DModelPosSetV(playerData->model78, &lbl_1_data_248[obstacleIndex].pos);
            Hu3DMotionTimeSet(playerData->model78, 0.0f);
            Hu3DMotionSpeedSet(playerData->model78, 1.0f);
            Hu3DModelAttrReset(playerData->model78, HU3D_ATTR_DISPOFF);
            lbl_1_bss_2AC[obstacleIndex][playerNo].state = 1;
            collisionNormal.x = playerPosition.x - lbl_1_data_248[obstacleIndex].pos.x;
            collisionNormal.y = 0.0f;
            collisionNormal.z = playerPosition.z - lbl_1_data_248[obstacleIndex].pos.z;
            C_VECReflect(&playerData->direction, &collisionNormal, &reflectedDirection);
            PSVECNormalize(&reflectedDirection, &playerData->reflectedDirection);
            if (lbl_1_data_248[obstacleIndex].isRock == 0) {
                HuAudFXPlay(treeSounds[playerNo]);
            } else {
                HuAudFXPlay(rockSounds[playerNo]);
            }
            return 1;
        }
    }
    if (playerPosition.x <= -1150.0f) {
        collisionNormal.x = 1.0f;
        collisionNormal.y = collisionNormal.z = 0.0f;
        C_VECReflect(&playerData->direction, &collisionNormal, &reflectedDirection);
        PSVECNormalize(&reflectedDirection, &playerData->reflectedDirection);
        HuAudFXPlay(wallSounds[playerNo]);
        return 1;
    }
    if (playerPosition.x >= 1150.0f) {
        collisionNormal.x = -1.0f;
        collisionNormal.y = collisionNormal.z = 0.0f;
        C_VECReflect(&playerData->direction, &collisionNormal, &reflectedDirection);
        PSVECNormalize(&reflectedDirection, &playerData->reflectedDirection);
        HuAudFXPlay(wallSounds[playerNo]);
        return 1;
    }
    return 0;
}

/* Called during per-frame throw movement to find the first available obstacle the item reaches. */
s16 fn_1_6D88(s16 playerNo, HuVecF *thrownPosition)
{
    HuVecF obstacleGroundPosition;
    M650Player *playerData;
    int obstacleIndex;

    playerData = &lbl_1_bss_28[playerNo];
    obstacleGroundPosition.x = thrownPosition->x;
    obstacleGroundPosition.y = 0.0f;
    obstacleGroundPosition.z = thrownPosition->z;
    for (obstacleIndex = 0; obstacleIndex < 32; obstacleIndex++) {
        if (lbl_1_bss_2AC[obstacleIndex][playerNo].state == 0 &&
            fn_1_6EE4(&obstacleGroundPosition, &lbl_1_data_248[obstacleIndex].pos, 90.0f)) {
            if (lbl_1_data_248[obstacleIndex].isRock == 1 && thrownPosition->y < 160.0f) {
                return obstacleIndex;
            }
            if (lbl_1_data_248[obstacleIndex].isRock == 0 && thrownPosition->y < 180.0f) {
                return obstacleIndex;
            }
        }
    }
    return -1;
}

/* Collision helper used by fn_1_69B0 and fn_1_6D88; compares 3D distance with a strict radius. */
s16 fn_1_6EE4(HuVecF *firstPoint, HuVecF *secondPoint, float radius)
{
    float distanceSquared;

    distanceSquared = PSVECSquareDistance(firstPoint, secondPoint);
    if (distanceSquared < radius * radius) {
        return 1;
    }
    return 0;
}

/* Called when results are prepared; hides the course obstacles and shows the five record props. */
void fn_1_6F54(void)
{
    int obstacleIndex;
    int playerNo;

    for (obstacleIndex = 0; obstacleIndex < 32; obstacleIndex++) {
        for (playerNo = 0; playerNo < 4; playerNo++) {
            lbl_1_bss_2AC[obstacleIndex][playerNo].state = 2;
        }
    }
    for (obstacleIndex = 0; obstacleIndex < 5; obstacleIndex++) {
        for (playerNo = 0; playerNo < 4; playerNo++) {
            lbl_1_bss_2AC[obstacleIndex][playerNo].state = 3;
        }
    }
}

/* CPU aiming calls this to test whether an obstacle lies within the aiming line's clearance.
 * Projection is not limited to the forward 800-unit segment. */
s16 fn_1_7000(s16 playerNo, HuVecF *throwStart, float angleDegrees, s16 obstacleIndex)
{
    HuVecF throwDirection;
    HuVecF obstacleDelta;
    HuVecF projection;
    HuVecF closestPoint;
    float remainingPathRatio;
    float projectionFactor;
    float threshold;
    float perpendicularDistance;

    if (obstacleIndex >= 32) {
        return 0;
    }
    throwDirection.x = 800.0 * sin((M_PI * angleDegrees) / 180.0);
    throwDirection.y = 0.0f;
    throwDirection.z = 800.0 * cos((M_PI * angleDegrees) / 180.0);
    obstacleDelta.x = lbl_1_data_248[obstacleIndex].pos.x - throwStart->x;
    obstacleDelta.y = 0.0f;
    obstacleDelta.z = lbl_1_data_248[obstacleIndex].pos.z - throwStart->z;
    projectionFactor =
        PSVECDotProduct(&throwDirection, &obstacleDelta) / PSVECSquareMag(&throwDirection);
    projection.x = projectionFactor * throwDirection.x;
    projection.y = 0.0f;
    projection.z = projectionFactor * throwDirection.z;
    remainingPathRatio = sqrtf(PSVECSquareMag(&projection));
    closestPoint.x = throwStart->x + projection.x;
    closestPoint.y = 0.0f;
    closestPoint.z = throwStart->z + projection.z;
    throwDirection.x = lbl_1_data_248[obstacleIndex].pos.x - closestPoint.x;
    throwDirection.y = 0.0f;
    throwDirection.z = lbl_1_data_248[obstacleIndex].pos.z - closestPoint.z;
    perpendicularDistance = sqrtf(PSVECSquareMag(&throwDirection));
    remainingPathRatio = (800.0f - remainingPathRatio) / 800.0f;
    if (remainingPathRatio < 0.0f) {
        remainingPathRatio = 0.0f;
    }
    if (throwDirection.x < 0.0f) {
        /* A negative-X offset from the projected point widens clearance as that point nears the
         * throw start. */
        threshold = 150.0f + (45.0f * remainingPathRatio);
    } else {
        threshold = 150.0f - (45.0f * remainingPathRatio);
    }
    if (perpendicularDistance <= threshold) {
        return 1;
    }
    return 0;
}
