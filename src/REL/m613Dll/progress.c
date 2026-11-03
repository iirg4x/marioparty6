/* Displays player progress and animates the particles released by drawing. */
#include "REL/m613Dll/types.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/frand.h"
#include "game/mg/seqman.h"
#include "game/charman.h"

#define M613_MODEL_DATA_GROUP_FLAG 0x10000000
#define MSM_SE_M613_PROGRESS 1711

extern s32 lbl_1_bss_8;
extern s16 lbl_1_bss_0;

extern s16 lbl_1_bss_125E;

extern s32 fn_1_10C5C(Point3d *, Point3d *);
extern s32 fn_1_10DB4(f32, f32, f32 *);

void fn_1_F3C4(void);
void fn_1_F568(void);
void fn_1_F620(s16 playerIndex, s32 delta);
void fn_1_F6E0(s16 playerIndex, Point3d *position);
void fn_1_F84C(void);
void fn_1_F850(void);
s16 fn_1_FA48(void);
void fn_1_FC40(void);
void fn_1_FE58(void);
void fn_1_10104(HU3D_MODEL *unusedObject, HU3D_PARTICLE *manager, Mtx unusedMatrix);
void fn_1_1054C(const Point3d *sourcePosition, const Point3d *targetPosition,
                s16 playerIndex, s32 progressAmount);
void fn_1_106C0(void);

Point3d lbl_1_data_210[4] = { { -675.0f, 0.0f, -300.0f },
                              { 675.0f, 0.0f, -300.0f },
                              { -675.0f, 0.0f, 300.0f },
                              { 675.0f, 0.0f, 300.0f } };
Point3d lbl_1_data_240[4] = { { -300.0f, 0.0f, -300.0f },
                              { -100.0f, 0.0f, -300.0f },
                              { 100.0f, 0.0f, -300.0f },
                              { 300.0f, 0.0f, -300.0f } };

/* Called during the opening phase to create each player's progress model and particle effects. */
void fn_1_F3C4(void) {
    s16 night;
    s16 nightMode;
    s16 player;

    nightMode = lbl_802C0278;
    night = nightMode;
    player = 0;
    while (player < 4) {
        if (night == 0) {
            lbl_1_bss_1218[player].model = fn_80025A04(fn_80008DD0(
                DATANUM(0x480000, 13), M613_MODEL_DATA_GROUP_FLAG, 2));
        } else {
            lbl_1_bss_1218[player].model = fn_80025A04(fn_80008DD0(
                DATANUM(0x480000, 14), M613_MODEL_DATA_GROUP_FLAG, 2));
        }
        fn_80026B64(lbl_1_bss_1218[player].model, 1);
        fn_80026B8C(lbl_1_bss_1218[player].model, 5);
        fn_800269E8(lbl_1_bss_1218[player].model, ((1 << 30) | (1 << 0)));
        fn_80026790(lbl_1_bss_1218[player].model, &lbl_1_data_210[player]);
        lbl_1_bss_1218[player].verticalScale = 500.0f;
        lbl_1_bss_1218[player].progress = 0;
        player++;
    }
    fn_1_FE58();
}

/* Copies the leading progress value to every player's bar and previous-progress baseline. */
void fn_1_F568(void) {
    s16 playerIndex;
    s32 greatestProgress;
    greatestProgress = - 1;
    playerIndex = 0;
    while (playerIndex < 4)
    {
        if (greatestProgress < (s32) lbl_1_bss_1218[playerIndex].progress)
        {
            greatestProgress = lbl_1_bss_1218[playerIndex].progress;
        }
        playerIndex += 1;
    }
    playerIndex = 0;
    while (playerIndex < 4)
    {
        lbl_1_bss_1218[playerIndex].progress = greatestProgress;
        lbl_1_bss_1218[playerIndex].previousProgress = greatestProgress;
        playerIndex += 1;
    }
}

/* Adds a player's earned progress, clamped to 0 through 32768. */
void fn_1_F620(s16 playerIndex, s32 delta)
{
    if ((playerIndex >= 0) && (playerIndex < 4)) {
        (lbl_1_bss_1218[playerIndex].progress) += delta;
        if (lbl_1_bss_1218[playerIndex].progress > 32768) {
            lbl_1_bss_1218[playerIndex].progress = 32768;
        }
        if (lbl_1_bss_1218[playerIndex].progress < 0) {
            lbl_1_bss_1218[playerIndex].progress = 0;
        }
    }
}

/* Clamps playerIndex to 0 through 3, then returns a randomized position above that player's bar. */
void fn_1_F6E0(s16 playerIndex, Point3d *position)
{
    if (playerIndex < 0) {
        playerIndex = 0;
    }
    if (playerIndex >= 4) {
        playerIndex = 3;
    }

    *position = lbl_1_data_210[playerIndex];
    position->x += (f32) (frandmod(50) - 25);
    position->z += (f32) (frandmod(50) - 25);
    position->y = (f32) lbl_1_bss_1218[playerIndex].progress / 32768.0f;
    position->y *= lbl_1_bss_1218[playerIndex].verticalScale;
}

/* This progress callback has no work to perform. */
void fn_1_F84C(void) {
}

/* Called by sequence callbacks to loop or pause each player's progress animation at its mark. */
void fn_1_F850(void) {
    s16 player = 0;

    while (player < 4) {
        f32 progressRatio;
        s32 clipStart;
        s32 motionFrame;

        progressRatio = (f32)lbl_1_bss_1218[player].progress / 32768.0f;
        clipStart = (s32)(20.0f * progressRatio);
        if (clipStart >= 20) {
            clipStart = 20;
        }
        clipStart *= 60;
        motionFrame = (s32)fn_8002D4E0(lbl_1_bss_1218[player].model);

        if (motionFrame < clipStart + 59) {
            fn_80026A44(lbl_1_bss_1218[player].model, ((1 << 30) | (1 << 1)));
            fn_800269E8(lbl_1_bss_1218[player].model, ((1 << 30) | (1 << 0)));
            if ((motionFrame + 1 == clipStart) && (lbl_1_bss_125E == 3)) {
                if ((player == 0) || (player == 2)) {
                    HuAudFXPanning(HuAudFXPlay(MSM_SE_M613_PROGRESS), 32);
                } else {
                    HuAudFXPanning(HuAudFXPlay(MSM_SE_M613_PROGRESS), 96);
                }
            }
        } else {
            fn_80026A44(lbl_1_bss_1218[player].model, ((1 << 30) | (1 << 0)));
            fn_800269E8(lbl_1_bss_1218[player].model, ((1 << 30) | (1 << 1)));
        }
        player++;
    }
}

/* Divides current progress and the stored baseline by 655.36, overwriting the baseline, then
 * returns the mask of tied leaders whose converted progress exceeds that baseline. */
s16 fn_1_FA48(void) {
    s32 playerProgress[4];
    s16 player;
    s32 greatestProgress;
    u8 playerMask;

    playerMask = 0;
    greatestProgress = -1;
    player = 0;
    while (player < 4) {
        playerProgress[player] = (s32)((f32)lbl_1_bss_1218[player].progress / 655.36f);
        lbl_1_bss_1218[player].previousProgress =
            (s32)((f32)lbl_1_bss_1218[player].previousProgress / 655.36f);
        if (greatestProgress < playerProgress[player]) {
            greatestProgress = playerProgress[player];
        }
        player++;
    }

    player = 0;
    while (player < 4) {
        if ((greatestProgress <= playerProgress[player]) &&
            (playerProgress[player] > lbl_1_bss_1218[player].previousProgress)) {
            if (player == 0) {
                playerMask |= 1;
            }
            if (player == 1) {
                playerMask |= 2;
            }
            if (player == 2) {
                playerMask |= 4;
            }
            if (player == 3) {
                playerMask |= 8;
            }
        }
        player++;
    }
    return playerMask;
}

/* Creates each player's result effect in the result layout with a progress-based height, then
* hides the gameplay progress model. */
void fn_1_FC40(void) {
    s16 night;
    s16 nightMode;
    s16 player;

    nightMode = lbl_802C0278;
    night = nightMode;
    player = 0;
    while (player < 4) {
        if (night == 0) {
            lbl_1_bss_1218[player].effectModel = fn_80025A04(fn_80008DD0(
                DATANUM(0x480000, 15), M613_MODEL_DATA_GROUP_FLAG, 2));
        } else {
            lbl_1_bss_1218[player].effectModel = fn_80025A04(fn_80008DD0(
                DATANUM(0x480000, 16), M613_MODEL_DATA_GROUP_FLAG, 2));
        }
        fn_80026B64(lbl_1_bss_1218[player].effectModel, 1);
        fn_80026B8C(lbl_1_bss_1218[player].effectModel, 5);

        lbl_1_data_240[player].y = (f32)lbl_1_bss_1218[player].progress / 32768.0f;
        lbl_1_data_240[player].y *= 500.0f;
        lbl_1_data_240[player].y -= 500.0f;
        fn_80026790(lbl_1_bss_1218[player].effectModel, &lbl_1_data_240[player]);
        fn_800269E8(lbl_1_bss_1218[player].model, 1);
        player++;
    }
}

/* Called during progress setup to create the three particle pools and initialize their records. */
void fn_1_FE58(void) {
    s16 night;
    s16 nightMode;
    s16 particleIndex;
    ANIMDATA *animation;
    HU3D_PARTICLE *pool;
    HU3D_PARTICLE_DATA *record;

    nightMode = lbl_802C0278;
    night = nightMode;

    animation = HuSprAnimRead(
        fn_80008DD0(DATANUM(0x480000, 17), M613_MODEL_DATA_GROUP_FLAG, 2));
    lbl_1_bss_1210[0] = fn_8003216C(animation, 200);
    animation = HuSprAnimRead(
        fn_80008DD0(DATANUM(0x480000, 18), M613_MODEL_DATA_GROUP_FLAG, 2));
    lbl_1_bss_1210[1] = fn_8003216C(animation, 200);
    animation = HuSprAnimRead(
        fn_80008DD0(DATANUM(0x480000, 19), M613_MODEL_DATA_GROUP_FLAG, 2));
    lbl_1_bss_1210[2] = fn_8003216C(animation, 200);

    particleIndex = 0;
    while (particleIndex < 3) {
        fn_80026760(lbl_1_bss_1210[particleIndex], 0.0f,
                        0.0f, 0.0f);
        fn_800268B0(lbl_1_bss_1210[particleIndex], 1.0f,
                          1.0f, 1.0f);
        fn_80026B8C(lbl_1_bss_1210[particleIndex], 6);
        if (night == 0) {
            fn_800325B0(lbl_1_bss_1210[particleIndex], 255, 255, 255);
        } else {
            fn_800325B0(lbl_1_bss_1210[particleIndex], 128, 128, 128);
        }
        fn_800324E0(lbl_1_bss_1210[particleIndex], 1.0f);
        fn_800326D0(lbl_1_bss_1210[particleIndex], fn_1_10104);

        pool = lbl_802C01C8[lbl_1_bss_1210[particleIndex]].hookData;
        record = pool->data;
        {
            s16 recordIndex = 0;
            while (recordIndex < pool->maxCnt) {
                record->scale = 0.0f;
                record->time = 0;
                record->parManId = particleIndex;
                recordIndex++;
                record++;
            }
        }
        particleIndex++;
    }
}

/* Called by the particle draw hook when it runs to move marks and award progress on impact. */
void fn_1_10104(HU3D_MODEL *unusedObject, HU3D_PARTICLE *manager, Mtx unusedMatrix) {
    HU3D_PARTICLE_DATA *piece = manager->data;
    s16 pieceIndex = 0;

    while (pieceIndex < manager->maxCnt) {
            if (piece->scale) {
                Point3d movement;
                s32 progress;
                f32 distancePerDegreeOrAngle;
                s16 hitPlayer;

                movement.x = piece->accel.x - piece->vel.x;
                movement.y = piece->accel.y - piece->vel.y;
                movement.z = piece->accel.z - piece->vel.z;
                distancePerDegreeOrAngle = PSVECMag(&movement) / 180.0f;
                if (fn_1_10C5C(&movement, &movement) == 0) {
                    piece->scale = 0.0f;
                } else {
                    movement.x = 6.0f * movement.x * piece->time;
                    movement.y = 6.0f * movement.y * piece->time;
                    movement.z = 6.0f * movement.z * piece->time;
                    if (fn_1_10DB4(PSVECMag(&movement), distancePerDegreeOrAngle,
                                   &distancePerDegreeOrAngle) == 0) {
                        distancePerDegreeOrAngle = 0.0f;
                    }
                    movement.y =
                        (f32) (movement.y +
                               500.0 * sin((3.141592653589793 * (f64) distancePerDegreeOrAngle) /
                                           180.0));
                    if ((piece->accel.x - piece->vel.x) > 0.0) {
                        piece->zRot = (f32)((3.141592653589793 *
                                                 (f64)(-1.0f * distancePerDegreeOrAngle)) /
                                                180.0);
                    } else {
                        piece->zRot = (f32)((3.141592653589793 * (f64)distancePerDegreeOrAngle) /
                                                180.0);
                    }
                    PSVECAdd(&piece->vel, &movement, &piece->pos);
                    if (fn_1_0(piece->pos, 1.0f, piece->accel,
                               50.0f) != 0) {
                        piece->scale = 0.0f;
                        piece->time = 0;
                        progress =
                            lbl_1_bss_400[piece->parManId][pieceIndex];
                        hitPlayer = lbl_1_bss_D60[piece->parManId][pieceIndex];
                        fn_1_F620(hitPlayer, progress);
                    }
                    /* The increment below leaves time at one even after a hit resets it to zero. */
                    piece->time++;
                }
            }
            pieceIndex++;
            piece++;
    }
}

/* Chooses a random particle pool and launches a mark from source to target in its first free slot;
 * a full pool drops the mark. */
void fn_1_1054C(const Point3d *sourcePosition, const Point3d *targetPosition,
                s16 playerIndex, s32 progressAmount)
{
    s16 recordIndex;
    HU3D_PARTICLE *hook;
    HU3D_PARTICLE_DATA *record;

    recordIndex = (s16) frandmod(3);
    hook = lbl_802C01C8[lbl_1_bss_1210[recordIndex]].hookData;
    record = hook->data;
    recordIndex = 0;

    while (recordIndex < hook->maxCnt) {
        if (!record->scale) {
            break;
        }
        recordIndex++;
        record++;
    }

    if (recordIndex < hook->maxCnt) {
        /* The particle fields vel and accel hold fixed source and target positions for this
         * flight. */
        record->pos = *sourcePosition;
        record->vel = *sourcePosition;
        record->accel = *targetPosition;
        record->scale = 40.0f;
        record->time = 0;
        lbl_1_bss_D60[record->parManId][recordIndex] = playerIndex;
        lbl_1_bss_400[record->parManId][recordIndex] = progressAmount;
    }
}

/* Called during result-scene setup after the outgoing wipe to hide and reinitialize all
* particle records. */
void fn_1_106C0(void)
{
    s16 modelIndex;
    s16 dataIndex;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    for (modelIndex = 0; modelIndex < 3; modelIndex++) {
        particle = lbl_802C01C8[lbl_1_bss_1210[modelIndex]].hookData;
        data = particle->data;
        for (dataIndex = 0;
             dataIndex < particle->maxCnt; dataIndex++, data++) {
            data->scale = 0.0f;
            data->time = 0;
            data->parManId = modelIndex;
        }
    }
}

M613ProgressState lbl_1_bss_1218[4];
s16 lbl_1_bss_1210[4];
s16 lbl_1_bss_D60[3][200];
s32 lbl_1_bss_400[3][200];
