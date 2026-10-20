/* Advances the finish, winner, and closing scenes after course play ends. */
#include "dolphin/math.h"
#include "REL/m609dll/state.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/mg/seqman.h"
#include "game/charman.h"
#include "game/mg/actman.h"
#include "game/frand.h"
#include "game/flag.h"
#include "game/data.h"
#include "game/memory.h"

typedef struct M609SequencePlayerSlots {
    u8 beforePlayerSlots[106];
    s8 playerSlots[246];
} M609SequencePlayerSlots;

extern void fn_1_11B80(void);
extern void fn_1_52B8(void);
extern void fn_1_6974(void);
extern void fn_1_2DF4(void);
extern void fn_1_8EE4(void);
extern void fn_1_2780(void);
extern void fn_1_46F8(s16, s16, void *);
extern s32 fn_1_118BC(void);
extern s32 fn_1_30D8(f32 *value, f32 step, f32 start, f32 end);
extern f32 fn_1_31F8(f32 value);
extern void fn_1_670(void);
extern void fn_1_4F98(void);
extern void fn_1_7150(M609Model *record);
extern s32 fn_1_89E0(f32 startZ);
extern void fn_1_7E10(void);
extern void fn_1_8670(void);
extern void fn_1_8C54(void);
extern double sin(double);
extern double cos(double);
extern double sqrt(double);
extern void fn_1_3FF0(M609Player *player, s16 mode);
extern s32 fn_1_126F0(M609Player *player, s16 advanceAfterChase);

extern double atan2(double, double);

typedef struct M609CollisionEdge {
    f32 startX;
    f32 endX;
    f32 startZ;
    f32 endZ;
} M609CollisionEdge;

typedef struct M609Placement {
    s16 status;
    s16 pathIndex;
    Vec position;
    Vec movementValues;
    Vec positionStep;
    Vec movementSelectors;
    Vec bounds;
    s16 record;
    s16 linkedRecord;
} M609Placement;

void fn_1_5618(s16,s16);
void fn_1_45EC(s32,s32);
s32 fn_1_111A8(void);
s32 fn_1_11658(void);
s32 fn_1_118BC(void);
void fn_1_11B80(void);
s32 fn_1_1115C(void);
s32 fn_1_1183C(void);
int fn_1_1188C(void);

/* Starts the results transition and refreshes its setup helpers from phase callback fn_1_2E0. */
s32 fn_1_1115C(void) {
    s32 result = 1;

    fn_1_45EC(1, 100);
    fn_1_670();
    fn_1_4F98();
    fn_1_2DF4();
    fn_1_8EE4();
    fn_1_2780();
    return result;
}

/* Awards non-practice coins and advances the winner and camera sequence each frame from
 * fn_1_30C. */
s32 fn_1_111A8(void) {
    s16 winners[4];
    f32 motionStep;
    s16 playerNo;
    s32 motionFrames;
    s32 playerId;
    s32 complete = 0;

    motionFrames = 0;
    switch (lbl_1_bss_30.modeStep) {
    case 0:
        if (lbl_1_bss_30.resultsReady != 0) {
            for (playerNo = 0; playerNo < 4; playerNo++) {
                if (lbl_1_bss_24C[playerNo].mode >= 0) {
                    winners[playerNo] = lbl_1_bss_24C[playerNo].characterNo;
                    playerId = lbl_1_bss_24C[playerNo].playerNo;
                    if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                        GwPlayer[playerId].mgCoinBonus = 10;
                    }
                } else {
                    winners[playerNo] = -1;
                }
            }
            MgSeqWinnerSet(winners[0], winners[1], winners[2], winners[3]);
            lbl_1_bss_30.modeStep++;
        }
        break;
    case 1:
        fn_1_118BC();
        if (lbl_1_bss_30.waitTimer++ > 120) {
            if (lbl_1_bss_30.setupFlag == 0 &&
                lbl_1_bss_30.night == 0) {
                Hu3DMotionSpeedSet(lbl_1_bss_8BC[7].model,
                                   (1.0f));
            }
            lbl_1_bss_30.waitTimer = 0;
            lbl_1_bss_30.modeStep++;
        }
        break;
    case 2:
        fn_1_118BC();
        if (lbl_1_bss_30.setupFlag == 0) {
            motionStep = (60.0f);
            motionFrames += fn_1_30D8(&Center.x, motionStep,
                                      (257.0f), (0.0f));
            motionFrames += fn_1_30D8(&Center.y, motionStep,
                                      (350.0f), (251.0f));
            motionFrames += fn_1_30D8(&Center.z, motionStep,
                                      (274.0f), (274.0f));
            motionFrames += fn_1_30D8(&CRot.x, motionStep,
                                      (-42.0f), (-26.0f));
            motionFrames += fn_1_30D8(&CRot.y, motionStep,
                                      (30.0f), (0.0f));
            motionFrames += fn_1_30D8(&CRot.z, motionStep,
                                      (0.0f), (0.0f));
            motionFrames += fn_1_30D8(&CZoom, motionStep,
                                      (1587.0f), (960.0f));
        } else if (lbl_1_bss_30.setupFlag == 1) {
            motionFrames = 1;
            if (lbl_1_bss_30.waitTimer++ > 120) {
                lbl_1_bss_30.waitTimer = 0;
                motionFrames = 0;
            }
        }
        if ((s16)motionFrames == 0) {
            lbl_1_bss_30.modeStep++;
        }
        break;
    case 3:
        if ((s16)fn_1_118BC() != 0 || lbl_1_bss_30.setupFlag == 1) {
            lbl_1_bss_30.modeStep = 0;
            complete = 1;
        }
        break;
    }

    if (lbl_1_bss_30.night == 0 &&
        Hu3DMotionTimeGet(lbl_1_bss_8BC[7].model) >= (118.9f)) {
        Hu3DModelAttrSet(lbl_1_bss_8BC[7].model, 1);
    }
    fn_1_670();
    fn_1_4F98();
    fn_1_2DF4();
    fn_1_8EE4();
    fn_1_2780();
    return complete;
}

/* Starts active players' result motions and updates the result scene each frame from fn_1_338. */
s32 fn_1_11658(void) {
    s32 complete = 0;
    s16 playerNo;

    switch (lbl_1_bss_30.modeStep) {
    case 0:
        if (lbl_1_bss_30.setupFlag != 1) {
            for (playerNo = 0; playerNo < 4; playerNo++) {
                if (lbl_1_bss_24C[playerNo].mode > 0) {
                    lbl_1_bss_70C[playerNo].shiftState = 1;
                    lbl_1_bss_70C[playerNo].shiftMotion = 2;
                    lbl_1_bss_70C[playerNo].shiftStart = (0.0f);
                    lbl_1_bss_70C[playerNo].blendDuration = (5.0f);
                    lbl_1_bss_70C[playerNo].shiftFlags = 0;
                    lbl_1_bss_70C[playerNo].attributesToReset = HU3D_MOTATTR_LOOP;
                }
            }
        }
        lbl_1_bss_30.modeStep++;
        break;
    case 1:
        break;
    }

    if (lbl_1_bss_30.night == 0 &&
        Hu3DMotionTimeGet(lbl_1_bss_8BC[7].model) >= (118.9f)) {
        Hu3DModelAttrSet(lbl_1_bss_8BC[7].model, 1);
    }

    fn_1_11B80();
    fn_1_52B8();
    fn_1_6974();
    fn_1_2DF4();
    fn_1_8EE4();
    fn_1_2780();

    return complete;
}

/* Starts the next result transition and refreshes its scene helpers each frame from fn_1_364. */
s32 fn_1_1183C(void) {
    s32 result;

    result = 0;
    fn_1_5618(7, 100);
    fn_1_11B80();
    fn_1_52B8();
    fn_1_6974();
    fn_1_2DF4();
    fn_1_8EE4();
    fn_1_2780();
    return result;
}

/* Stops all audio from the closing sequence callback and returns zero. */
int fn_1_1188C(void) {
    int result = 0;
    fn_1_4E90();
    return result;
}

/* Sequences final lineup walks from fn_1_111A8: starts the next player after chase
 * arrival, then waits for the last player to finish turning forward. */
s32 fn_1_118BC(void) {
    s32 complete = 0;
    s16 winnerIndex;
    s16 completedPlayers;

    switch (lbl_1_bss_30.sequenceStep) {
    case 0:
        if (lbl_1_bss_30.participantCount != 0) {
            winnerIndex = 0;
            completedPlayers = 0;
            while (winnerIndex < lbl_1_bss_30.participantCount) {
                if (lbl_1_bss_24C[lbl_1_bss_30.winners[winnerIndex]].state == 3) {
                    completedPlayers++;
                }
                winnerIndex++;
            }
            if (completedPlayers == lbl_1_bss_30.participantCount &&
                lbl_1_bss_8BC[50].stateOrTimer == 3) {
                lbl_1_bss_30.index = 0;
                lbl_1_bss_24C[
                    lbl_1_bss_30.winners[lbl_1_bss_30.index]].state++;
                lbl_1_bss_30.sequenceStep++;
            }
        }
        break;
    case 1:
        if (lbl_1_bss_24C[
                lbl_1_bss_30.winners[lbl_1_bss_30.index]].state >= 5) {
            lbl_1_bss_30.index++;
            if (lbl_1_bss_30.participantCount > lbl_1_bss_30.index) {
                lbl_1_bss_24C[
                    lbl_1_bss_30.winners[lbl_1_bss_30.index]].state++;
            } else {
                lbl_1_bss_30.sequenceStep++;
            }
        }
        break;
    case 2:
        /* index is the participant count. winners begins at byte 107, so byte 106 + index
         * selects the last announced winner, winners[index - 1]. */
        if (lbl_1_bss_24C[
                ((s8 *)&lbl_1_bss_30)[lbl_1_bss_30.index + 106]].state >= 6) {
            lbl_1_bss_30.sequenceStep++;
            lbl_1_bss_30.index = 0;
        }
        break;
    case 3:
        if (lbl_1_bss_30.index++ > 30) {
            complete = 1;
        }
        break;
    }

    return complete;
}

/* Moves the shared result platform along negative Z and reveals models 11 through 16
 * during fn_1_11658 and fn_1_1183C. */
void fn_1_11B80(void) {
    s16 modelNo;

    switch (lbl_1_bss_30.sequenceState) {
    case 0:
        lbl_1_bss_8BC[50].movementValues.z = (0.0f);
        lbl_1_bss_8BC[11].stateOrTimer = 1;
        lbl_1_bss_8BC[12].stateOrTimer = 11;
        lbl_1_bss_8BC[13].stateOrTimer = 21;
        lbl_1_bss_8BC[14].stateOrTimer = 31;
        lbl_1_bss_8BC[15].stateOrTimer = 41;
        lbl_1_bss_8BC[16].stateOrTimer = 51;
        if (lbl_1_bss_30.setupFlag == 0) {
            fn_1_46F8(6, 5, 0);
        }
        lbl_1_bss_30.sequenceState++;
        /* fall through */
    case 1:
        if (lbl_1_bss_30.setupFlag == 0) {
            if ((lbl_1_bss_8BC[50].movementValues.z -= (0.05f)) <= (-12.0f)) {
                lbl_1_bss_8BC[50].movementValues.z = (-12.0f);
            }
            lbl_1_bss_8BC[50].position.z += lbl_1_bss_8BC[50].movementValues.z;
            lbl_1_bss_6B6C.target.z = lbl_1_bss_8BC[50].position.z;
            for (modelNo = 11; modelNo <= 16; modelNo++) {
                if (lbl_1_bss_8BC[modelNo].stateOrTimer-- == 0) {
                    Hu3DModelAttrReset(lbl_1_bss_8BC[modelNo].model, 1);
                    Hu3DModelAttrSet(lbl_1_bss_8BC[modelNo].model, HU3D_MOTATTR_LOOP);
                    Hu3DMotionTimeSet(lbl_1_bss_8BC[modelNo].model, (0.0f));
                    Hu3DMotionSpeedSet(lbl_1_bss_8BC[modelNo].model, (1.0f));
                }
                lbl_1_bss_8BC[modelNo].position = lbl_1_bss_8BC[50].position;
            }
        }
        break;
    }
}
