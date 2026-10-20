/* Daft Rafts course motion, player contacts, effects, and selection results. */
#include "dolphin/math.h"
#include "REL/m609dll/state.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/mg/seqman.h"
#include "game/charman.h"
#include "game/mg/actman.h"
#include "game/frand.h"
#include "game/data.h"
#include "game/memory.h"

/* Participant and lineup-placement state carried by the sequence. */
typedef struct M609SequencePlayerSlots {
    u8 sequenceState[106]; /* State preceding the participant and placement data. */
    s8 participantPlacementState[246]; /* Participant count, winners, and lineup state bytes. */
} M609SequencePlayerSlots;

/* Local offset and model index of an auxiliary model attached to a segment. */
typedef struct M609CourseAnchor {
    Vec position; /* Offset from the linked collision model, in world units. */
    s16 modelIndex; /* Index into the course model array. */
    s16 state; /* Secondary entry value; every entry initializes it to zero. */
} M609CourseAnchor;
extern double sin(double);
extern double cos(double);
extern double sqrt(double);

extern double atan2(double, double);

/* XZ endpoints used when pushing a player out of a moving segment's bounds. */
typedef struct M609CollisionEdge {
    f32 startX; /* First endpoint X, in world units. */
    f32 endX; /* Second endpoint X, in world units. */
    f32 startZ; /* First endpoint Z, in world units. */
    f32 endZ; /* Second endpoint Z, in world units. */
} M609CollisionEdge;

/* Course segment transform, motion limits, and model links used during setup. */
typedef struct M609Placement {
    s16 status; /* Segment slot; -1 retires the segment. */
    s16 pathIndex; /* Path group used by the player route table. */
    Vec position; /* Initial world position. */
    Vec movementValues; /* Initial per-axis movement values. */
    Vec positionStep; /* X is the turning reference; Y starts the integrated Z displacement. */
    Vec movementSelectors; /* Values selecting lateral and longitudinal movement paths. */
    Vec bounds; /* Extents used for contacts with the course. */
    s16 record; /* Course model array index. */
    s16 linkedRecord; /* Associated collision model index, or -1. */
} M609Placement;

#include "game/audio.h"
#include "game/gamemes.h"
#include "game/pad.h"
#include "game/wipe.h"
#include "datadir_enum.h"
/* Player index and mode carried by an actor request. */
typedef struct M609LinkRequest {
    s32 opaqueState; /* Additional request state. */
    s32 playerNo; /* Player slot, from zero through three. */
    s32 mode; /* Requested player action mode. */
} M609LinkRequest;
/* Actor fields identifying the player and its current collision mesh. */
typedef struct M609CharacterAction {
    u8 actorIdentity[12]; /* Actor state before its player parameter. */
    s32 playerNo; /* Player slot, from zero through three. */
    u8 actorMotionState[100]; /* Actor motion and collision state. */
    u16 collisionMesh; /* Current course mesh index used for contact handling. */
} M609CharacterAction;
/* Number of initial collision updates that have already applied course scroll. */
s16 lbl_1_bss_6D84;
/* Selection-phase control and shadow initialization has run when nonzero. */
static long lbl_1_bss_6D80;
void fn_1_11B80(void);
void fn_1_2DF4(void);
void fn_1_2780(void);
void fn_1_46F8(s16,s16,void *);
s32 fn_1_118BC(void);
s32 fn_1_30D8(f32 *,f32,f32,f32);
f32 fn_1_31F8(f32);
void fn_1_670(void);
void fn_1_3FF0(M609Player *,s16);
s32 fn_1_126F0(M609Player *,s16);
s32 fn_1_125C4(void);
int fn_1_C9A8(COL_NARROW_PARAM *contact, COL_NARROW_PARAM *otherContact);
void fn_1_CA6C(MGACTOR *actor, int hookParameter);
void fn_1_52B8(void);
void fn_1_6974(void);
void fn_1_7150(M609Model *segment);
void fn_1_7E10(void);
void fn_1_8670(void);
s32 fn_1_89E0(f32 startZ);
void fn_1_8C54(void);
void fn_1_8EE4(void);
void fn_1_5618(s16 soundSlot, s16 fadeSpeed);
void fn_1_4DC8(s16 soundSlot);
void fn_1_4E90(void);
s32 fn_1_4EB0(void);
void fn_1_4B48(s16 soundSlot, s16 pan);
void fn_1_4BD8(s16 soundSlot, s16 pitch);
void fn_1_4C68(s16 soundSlot, s16 volume);
void fn_1_4F98(void);
void fn_1_4FC0(void);
void fn_1_52B4(MGPLAYER *player);

/* Unused pan helper: updates an enabled sound slot only when its reported status is 2. */
void fn_1_4B48(s16 soundSlot, s16 pan) {
    if (lbl_1_bss_30.soundSlotStates[soundSlot] == 1) {
        if (HuAudFXStatusGet(
                lbl_1_bss_30.soundHandles[soundSlot]) == 2) {
            HuAudFXPanning(
                lbl_1_bss_30.soundHandles[soundSlot], pan);
        }
    }
}

/* Unused pitch helper: updates an enabled sound slot only when its reported status is 2. */
void fn_1_4BD8(s16 soundSlot, s16 pitch) {
    if (lbl_1_bss_30.soundSlotStates[soundSlot] == 1) {
        if (HuAudFXStatusGet(
                lbl_1_bss_30.soundHandles[soundSlot]) == 2) {
            HuAudFXPitchSet(
                lbl_1_bss_30.soundHandles[soundSlot], pitch);
        }
    }
}

/* The opening camera sequence requests a volume ramp here; the update only runs
 * for an enabled sound slot reporting status 2. */
void fn_1_4C68(s16 soundSlot, s16 volume) {
    if (lbl_1_bss_30.soundSlotStates[soundSlot] == 1) {
        if (HuAudFXStatusGet(
                lbl_1_bss_30.soundHandles[soundSlot]) == 2) {
            HuAudFXVolSet(
                lbl_1_bss_30.soundHandles[soundSlot], volume);
        }
    }
}

/* During sequence transitions, fades an enabled sound slot only for reported status
 * 2 or 3; its slot is released immediately before the fade request. */
void fn_1_5618(s16 soundSlot, s16 fadeSpeed) {
    if (((lbl_1_bss_30.soundSlotStates[soundSlot]) == 1) &&
        ((HuAudFXStatusGet((lbl_1_bss_30.soundHandles[soundSlot])) == 2) ||
         (HuAudFXStatusGet((lbl_1_bss_30.soundHandles[soundSlot])) == 3))) {
        (lbl_1_bss_30.soundSlotStates[soundSlot]) = -1;
        HuAudFXFadeOut(
            (lbl_1_bss_30.soundHandles[soundSlot]),
            fadeSpeed);
    }
}

/* Unused stop helper: for reported status 2 or 3, releases the enabled sound slot
 * and immediately stops its handle. */
void fn_1_4DC8(s16 soundSlot) {
    if (((lbl_1_bss_30.soundSlotStates[soundSlot]) == 1) &&
        ((HuAudFXStatusGet((lbl_1_bss_30.soundHandles[soundSlot])) == 2) ||
         (HuAudFXStatusGet((lbl_1_bss_30.soundHandles[soundSlot])) == 3))) {
        (lbl_1_bss_30.soundSlotStates[soundSlot]) = -1;
        HuAudFXStop((lbl_1_bss_30.soundHandles[soundSlot]));
    }
}

/* Stops all minigame audio when the closing sequence reaches its final phase. */
void fn_1_4E90(void) {
    HuAudAllStop();
}

/* Selection-phase callback: initializes player control once, runs the course update, and reports
 * when selection is ready. */
s32 fn_1_4EB0(void) {
    s32 selectionReady;
    s32 playerIndex;

    selectionReady = 0;
    if ((s32) lbl_1_bss_6D80 == 0) {
        playerIndex = 0;
        while (playerIndex < 4) {
            (lbl_1_bss_24C[playerIndex].movementFlag) = 1;
            (lbl_1_bss_24C[playerIndex].motionFlag) = 1;
            playerIndex += 1;
        }
        lbl_1_bss_6B6C.target.z = -150.0f;
        lbl_1_bss_6D80 = 1;
    }
    fn_1_670();
    fn_1_4F98();
    fn_1_125C4();
    fn_1_2DF4();
    fn_1_8EE4();
    fn_1_2780();
    if (lbl_1_bss_30.resultsReady != 0) {
        selectionReady = 1;
    }
    return selectionReady;
}

/* Per-frame course helper used by sequence callbacks: advances player reactions, course models, and
 * the selection tally. */
void fn_1_4F98(void)
{
    fn_1_52B8();
    fn_1_6974();
    fn_1_4FC0();
}

/* Tallies accepted and eliminated players after the course update and saves lineup positions once
 * all four are decided. */
void fn_1_4FC0(void) {
    s16 playerOrCollisionModelIndex = 0;
    s16 selectedCount = 0;
    s16 eliminatedCount = 0;
    s16 undecidedPlayer;

    while (playerOrCollisionModelIndex < 4) {
        if (lbl_1_bss_24C[playerOrCollisionModelIndex].collisionMesh == 2) {
            lbl_1_bss_24C[playerOrCollisionModelIndex].disableInput = 1;
            lbl_1_bss_24C[playerOrCollisionModelIndex].mode = 1;
            lbl_1_bss_24C[playerOrCollisionModelIndex].boundaryMode = 1;
        }
        if (lbl_1_bss_24C[playerOrCollisionModelIndex].phase != 0) {
            lbl_1_bss_24C[playerOrCollisionModelIndex].mode = -1;
        }
        if (lbl_1_bss_24C[playerOrCollisionModelIndex].mode == 1) {
            selectedCount++;
        } else if (lbl_1_bss_24C[playerOrCollisionModelIndex].mode == -1) {
            eliminatedCount++;
        } else {
            undecidedPlayer = playerOrCollisionModelIndex;
        }
        playerOrCollisionModelIndex++;
    }

    if (selectedCount + eliminatedCount == 4) {
        if (eliminatedCount == 4) {
            lbl_1_bss_30.resultsReady = 1;
            lbl_1_bss_30.setupFlag = 1;
        } else {
            lbl_1_bss_30.participantCount = selectedCount;
            lbl_1_bss_30.resultsReady = 1;
            lbl_1_bss_30.setupFlag = 0;
            playerOrCollisionModelIndex = 0;
            {
                s16 placementIndex = 0;

                while (playerOrCollisionModelIndex < 4) {
                    if (lbl_1_bss_24C[playerOrCollisionModelIndex].mode == 1) {
                        lbl_1_bss_24C[playerOrCollisionModelIndex].savedPosition =
                            lbl_1_bss_30.winnerPositions[selectedCount - 1][placementIndex];
                        lbl_1_bss_30.winners[placementIndex] = playerOrCollisionModelIndex;
                        placementIndex++;
                    }
                    playerOrCollisionModelIndex++;
                }
            }
        }
    } else {
        playerOrCollisionModelIndex = lbl_1_bss_24C[undecidedPlayer].collisionMesh + 32;
        if (eliminatedCount == 3 &&
            lbl_1_bss_24C[undecidedPlayer].position.z <= (150.0f)) {
            lbl_1_bss_30.resultFlag = 1;
        }
    }
}

/* Empty action callback installed while a player leaves the playable area. */
void fn_1_52B4(MGPLAYER *player)
{
}

/* Joint names used to position the models attached to each course segment. */
char lbl_1_data_58[][8] = {
    "dri00", "dri01", "dri02", "dri03", "dri03s", "dri04", "dri04s", "dri05",
    "dri05s", "dri06", "dri06s", "dri07", "dri08", "rescue"
};
/* X-rotation samples for the attached collision models, spaced ten frames apart. */
f32 lbl_1_data_C8[] = {
    0.0f, 13.0f, 26.0f, 40.5f, 55.0f, 66.5f, 78.0f, 82.0f, 86.0f, 87.5f, 89.0f, 90.0f, 90.0f
};
/* Segment-slot to collision-model mapping; -1 means there is no collision model. */
s16 lbl_1_data_FC[] = {
    32, 33, 35, 36, 37, 40, 41, 42, 43, 32, 44, 45, 33, 40, 41, 42,
    43, 35, 36, 44, 45, 40, 41, 42, 43, -1, -1, -1, -1, 37, 38, 39,
    35, 34
};
/* Auxiliary-model offsets grouped by the segment that activates them. */
M609CourseAnchor lbl_1_data_140[] = {
    {{0.0f, 50.0f, 125.0f}, 144, 0},
    {{0.0f, 50.0f, 0.0f}, 145, 0},
    {{0.0f, 50.0f, -125.0f}, 146, 0},
    {{340.0f, 50.0f, 125.0f}, 147, 0},
    {{340.0f, 50.0f, 0.0f}, 148, 0},
    {{340.0f, 50.0f, -125.0f}, 149, 0},
    {{-340.0f, 50.0f, 125.0f}, 150, 0},
    {{-340.0f, 50.0f, 0.0f}, 151, 0},
    {{-340.0f, 50.0f, -125.0f}, 152, 0},
    {{100.0f, 50.0f, 125.0f}, 144, 0},
    {{0.0f, 50.0f, 0.0f}, 145, 0},
    {{-100.0f, 50.0f, -125.0f}, 146, 0},
    {{176.0f, 50.0f, 24.0f}, 147, 0},
    {{-176.0f, 50.0f, 24.0f}, 148, 0},
    {{0.0f, 50.0f, -50.0f}, 149, 0},
    {{300.0f, 50.0f, 0.0f}, 150, 0},
    {{-300.0f, 50.0f, 0.0f}, 151, 0},
    {{0.0f, 50.0f, 0.0f}, 152, 0},
    {{-300.0f, 50.0f, 0.0f}, 144, 0},
    {{300.0f, 50.0f, 0.0f}, 145, 0},
    {{0.0f, 50.0f, -100.0f}, 153, 0}
};
/* Suppress the six basic action callbacks without replacing the remaining modes. */
MGPLAYER_MODE_FUNC lbl_1_data_290[18] = {
    fn_1_52B4, fn_1_52B4, fn_1_52B4, fn_1_52B4, fn_1_52B4, fn_1_52B4
};

/* Called by course and sequence updates to advance player falls, knockback reactions, and
 * selected-player lineup movement. */
void fn_1_52B8(void)
{
    M609Player *player;
    s16 playerIndex;
    s16 slotOrPlayerIndex;
    s32 transitionResult = 0;
    s16 nearestSlot;
    f32 interpolationFrames;
    f32 turnStartAngle;
    f32 distanceSquared;
    f32 minimumDistance;
    f32 distance;
    Vec modelScale;

    Hu3DModelTPLvlSet(lbl_1_bss_8BC[46].model, 1.0f);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        player = &lbl_1_bss_24C[playerIndex];
        Hu3DModelScaleGet(lbl_1_bss_8BC[player->playerNo].model, &modelScale);
        if (modelScale.y < 0.98f) {
            player->scaledDown = 1;
        } else {
            player->scaledDown = 0;
        }
        fn_1_3FF0(player, 0);
        if (player->phase == 1) {
            /* Disable actor collision before moving a fallen player explicitly. */
            switch (player->state) {
            case 0:
                fn_1_46F8(3, 0, &player->position);
                MgActorKill(lbl_1_bss_4.players[playerIndex]->actor);
                player->movementFlag = 0;
                Hu3DModelLayerSet(lbl_1_bss_8BC[playerIndex].model, 1);
                Hu3DModelShadowDispOff(lbl_1_bss_8BC[playerIndex].model);
                Hu3DModelScaleSet(lbl_1_bss_8BC[player->playerNo].model,
                                  1.0f, 1.0f, 1.0f);
                Hu3DModelAttrReset(lbl_1_bss_8BC[player->playerNo].model, HU3D_ATTR_DISPOFF);
                lbl_1_bss_8BC[player->playerNo + 28].stateOrTimer = 1;
                if (player->position.z < 0.0f) {
                    lbl_1_bss_70C[playerIndex].shiftState = 1;
                    lbl_1_bss_70C[playerIndex].shiftMotion = 7;
                    lbl_1_bss_70C[playerIndex].shiftStart = 0.0f;
                    lbl_1_bss_70C[playerIndex].blendDuration = 10.0f;
                    lbl_1_bss_70C[playerIndex].shiftFlags = HU3D_MOTATTR_LOOP;
                    lbl_1_bss_70C[playerIndex].attributesToReset = 0;
                }
                omVibrate(player->playerNo, 30, 20, 0);
                player->savedPosition.z = player->position.z;
                player->velocity.z = 7.0f;
                player->velocity.y = -10.0f;
                player->disableInput = 1;
                player->state++;
                break;
            case 1:
                if (player->position.z < 0.0f) {
                    player->position.z += player->velocity.z;
                    player->position.y += player->velocity.y;
                    if (player->position.y < -150.0f &&
                        player->velocity.y < 0.0f &&
                        player->savedPosition.z < -100.0f) {
                        player->velocity.y = 5.0f;
                    } else if (player->position.y >= 0.0f) {
                        player->velocity.y = 0.0f;
                    }
                } else {
                    lbl_1_bss_70C[playerIndex].shiftState = 1;
                    lbl_1_bss_70C[playerIndex].shiftMotion = 8;
                    lbl_1_bss_70C[playerIndex].shiftStart = 0.0f;
                    lbl_1_bss_70C[playerIndex].blendDuration = 10.0f;
                    lbl_1_bss_70C[playerIndex].shiftFlags = HU3D_MOTATTR_LOOP;
                    lbl_1_bss_70C[playerIndex].attributesToReset = 0;
                    player->velocity.y = -10.0f;
                    player->velocity.z = 7.0f;
                    player->state++;
                }
                break;
            case 2:
                interpolationFrames = 10.0f;
                fn_1_30D8(&player->rotation.x, interpolationFrames, 0.0f, 50.0f);
                if ((player->position.y += player->velocity.y) < -170.0f) {
                    player->position.y = -170.0f;
                    player->velocity.y = 0.0f;
                }
                if ((player->position.z += player->velocity.z) > 250.0f) {
                    player->position.z = 250.0f;
                    player->velocity.y = -12.0f;
                    player->velocity.z = 0.0f;
                    player->motionFlag = 0;
                    player->state++;
                }
                break;
            case 3:
                interpolationFrames = 10.0f;
                fn_1_30D8(&player->rotation.x, interpolationFrames, 0.0f, 50.0f);
                player->velocity.y -= 1.0f;
                if ((player->position.y += player->velocity.y) < -1000.0f) {
                    Hu3DModelAttrSet(lbl_1_bss_8BC[playerIndex].model, HU3D_ATTR_DISPOFF);
                    Hu3DMotionTimeSet(lbl_1_bss_8BC[playerIndex].model, 0.0f);
                    Hu3DMotionSpeedSet(lbl_1_bss_8BC[playerIndex].model, 0.0f);
                    player->state++;
                }
                break;
            }
            interpolationFrames = 60.0f;
            turnStartAngle = player->rotation.y > 0.0f ? 180.0f : -180.0f;
            fn_1_30D8(&player->rotation.y, interpolationFrames, turnStartAngle, 0.0f);
        } else if (player->phase == 2) {
            switch (player->state) {
            case 0:
                MgActorKill(lbl_1_bss_4.players[playerIndex]->actor);
                player->movementFlag = 0;
                Hu3DModelScaleSet(lbl_1_bss_8BC[player->playerNo].model,
                                  1.0f, 1.0f, 1.0f);
                Hu3DModelAttrReset(lbl_1_bss_8BC[player->playerNo].model, HU3D_ATTR_DISPOFF);
                if (lbl_1_bss_30.night == 0) {
                    lbl_1_bss_70C[playerIndex].shiftState = 1;
                    lbl_1_bss_70C[playerIndex].shiftMotion = 9;
                    lbl_1_bss_70C[playerIndex].shiftStart = 0.0f;
                    lbl_1_bss_70C[playerIndex].blendDuration = 5.0f;
                    lbl_1_bss_70C[playerIndex].shiftFlags = HU3D_MOTATTR_LOOP;
                    lbl_1_bss_70C[playerIndex].attributesToReset = 0;
                } else {
                    lbl_1_bss_70C[playerIndex].shiftState = 1;
                    lbl_1_bss_70C[playerIndex].shiftMotion = 3;
                    lbl_1_bss_70C[playerIndex].shiftStart = 0.0f;
                    lbl_1_bss_70C[playerIndex].blendDuration = 5.0f;
                    lbl_1_bss_70C[playerIndex].shiftFlags = HU3D_MOTATTR_LOOP;
                    lbl_1_bss_70C[playerIndex].attributesToReset = 0;
                }
                omVibrate(player->playerNo, 30, 20, 0);
                player->motionFlag = 0;
                player->state++;
                break;
            case 1:
                player->spinSpeed += 0.7f;
                player->position.x += player->velocity.x;
                player->position.y += player->velocity.y;
                player->position.z += player->velocity.z;
                if (lbl_1_bss_30.night == 0) {
                    player->rotation.x -= player->spinSpeed;
                }
                if ((player->position.x > 1200.0f ||
                     player->position.x < -1200.0f ||
                     player->position.z > 1200.0f ||
                     player->position.z < -1200.0f) &&
                    player->position.y >= 1000.0f) {
                    Hu3DModelAttrSet(lbl_1_bss_8BC[playerIndex].model, HU3D_ATTR_DISPOFF);
                    Hu3DMotionTimeSet(lbl_1_bss_8BC[playerIndex].model, 0.0f);
                    Hu3DMotionSpeedSet(lbl_1_bss_8BC[playerIndex].model, 0.0f);
                    player->state++;
                }
                break;
            }
        } else if (player->phase == 4) {
            switch (player->state) {
            case 0:
                MgPlayerModeFuncSet(lbl_1_bss_4.players[playerIndex], lbl_1_data_290);
                player->movementFlag = 2;
                Hu3DModelLayerSet(lbl_1_bss_8BC[playerIndex].model, 1);
                Hu3DModelScaleSet(lbl_1_bss_8BC[player->playerNo].model,
                                  1.0f, 1.0f, 1.0f);
                Hu3DModelAttrReset(lbl_1_bss_8BC[player->playerNo].model, HU3D_ATTR_DISPOFF);
                lbl_1_bss_70C[playerIndex].shiftState = 1;
                lbl_1_bss_70C[playerIndex].shiftMotion = 8;
                lbl_1_bss_70C[playerIndex].shiftStart = 0.0f;
                lbl_1_bss_70C[playerIndex].blendDuration = 20.0f;
                lbl_1_bss_70C[playerIndex].shiftFlags = HU3D_MOTATTR_LOOP;
                lbl_1_bss_70C[playerIndex].attributesToReset = 0;
                omVibrate(player->playerNo, 30, 20, 0);
                player->velocity.z = 12.0f;
                player->motionFlag = 0;
                player->disableInput = 1;
                player->state++;
                break;
            case 1:
                if (player->position.y < -800.0f) {
                    MgActorKill(lbl_1_bss_4.players[playerIndex]->actor);
                    player->movementFlag = 0;
                    Hu3DModelAttrSet(lbl_1_bss_8BC[playerIndex].model, HU3D_ATTR_DISPOFF);
                    Hu3DMotionTimeSet(lbl_1_bss_8BC[playerIndex].model, 0.0f);
                    Hu3DMotionSpeedSet(lbl_1_bss_8BC[playerIndex].model, 0.0f);
                    player->state++;
                }
                break;
            }
        } else if (player->mode == 1) {
                switch (player->state) {
                case 0:
                    /* Claim the nearest free slot before following the moving lineup platform. */
                    for (slotOrPlayerIndex = 0, minimumDistance = 1000.0f;
                         slotOrPlayerIndex < 8; slotOrPlayerIndex++) {
                        if (lbl_1_bss_30.slotOccupied[slotOrPlayerIndex] == 0) {
                            distanceSquared =
                                (player->position.x - lbl_1_bss_30.levels[slotOrPlayerIndex]) *
                                (player->position.x - lbl_1_bss_30.levels[slotOrPlayerIndex]);
                            distance = sqrt(distanceSquared);
                            if (minimumDistance > distance) {
                                minimumDistance = distance;
                                nearestSlot = slotOrPlayerIndex;
                            }
                        }
                    }
                    lbl_1_bss_30.slotOccupied[nearestSlot] = 1;
                    player->slot = nearestSlot;
                    player->routePosition.x = lbl_1_bss_30.levels[nearestSlot];
                    player->routePosition.z = lbl_1_bss_8BC[50].position.z - 160.0f;
                    player->chaseRadius = 8.0f;
                    player->chaseSpeed = 7.0f;
                    Hu3DModelScaleSet(lbl_1_bss_8BC[player->playerNo].model,
                                      1.0f, 1.0f, 1.0f);
                    Hu3DModelAttrReset(lbl_1_bss_8BC[player->playerNo].model, HU3D_ATTR_DISPOFF);
                    player->state++;
                    break;
                case 1:
                    player->routePosition.z = lbl_1_bss_8BC[50].position.z - 160.0f;
                    transitionResult = fn_1_126F0(player, 0);
                    if ((s16)transitionResult != 0) {
                        lbl_1_bss_70C[playerIndex].mode = 1;
                        lbl_1_bss_70C[playerIndex].previousMode = -1;
                        lbl_1_bss_70C[playerIndex].time =
                            Hu3DMotionTimeGet(lbl_1_bss_8BC[player->playerNo].model);
                        lbl_1_bss_70C[playerIndex].previousTime = -1.0f;
                        player->turnYaw = lbl_1_bss_4.players[playerIndex]->actor->rotY;
                        player->state++;
                    } else if (player->linkCount > 240) {
                        /* Reset all four players' chase and reaction states and clear their
                         * recorded slots, including players outside the lineup. */
                        for (slotOrPlayerIndex = 0; slotOrPlayerIndex < 4; slotOrPlayerIndex++) {
                            MgPlayerComStkOff(lbl_1_bss_4.players[
                                lbl_1_bss_24C[slotOrPlayerIndex].playerNo]);
                            lbl_1_bss_24C[slotOrPlayerIndex].movementState = 0;
                            lbl_1_bss_24C[slotOrPlayerIndex].linkCount = 0;
                            lbl_1_bss_24C[slotOrPlayerIndex].state = 0;
                            lbl_1_bss_30.slotOccupied[lbl_1_bss_24C[slotOrPlayerIndex].slot] = 0;
                        }
                    }
                    break;
                case 2:
                    transitionResult = fn_1_30D8(&lbl_1_bss_4.players[playerIndex]->actor->rotY,
                                        30.0f, player->turnYaw, 0.0f);
                    if ((s16)transitionResult == 0) {
                        lbl_1_bss_70C[playerIndex].shiftState = 1;
                        lbl_1_bss_70C[playerIndex].shiftMotion = 0;
                        lbl_1_bss_70C[playerIndex].shiftStart = 0.0f;
                        lbl_1_bss_70C[playerIndex].blendDuration = 10.0f;
                        lbl_1_bss_70C[playerIndex].shiftFlags = 0;
                        lbl_1_bss_70C[playerIndex].attributesToReset = HU3D_MOTATTR_LOOP;
                        Hu3DModelScaleSet(lbl_1_bss_8BC[player->playerNo].model,
                                          1.0f, 1.0f, 1.0f);
                        Hu3DModelAttrReset(lbl_1_bss_8BC[player->playerNo].model,
                                           HU3D_ATTR_DISPOFF);
                        player->state++;
                    }
                    break;
                case 4:
                    player->routePosition = lbl_1_bss_24C[playerIndex].savedPosition;
                    transitionResult = fn_1_126F0(player, 0);
                    if ((s16)transitionResult != 0) {
                        lbl_1_bss_70C[playerIndex].mode = 1;
                        lbl_1_bss_70C[playerIndex].previousMode = -1;
                        lbl_1_bss_70C[playerIndex].time =
                            Hu3DMotionTimeGet(lbl_1_bss_8BC[player->playerNo].model);
                        lbl_1_bss_70C[playerIndex].previousTime = -1.0f;
                        player->position.x = player->routePosition.x;
                        player->position.z = player->routePosition.z;
                        player->turnYaw = lbl_1_bss_4.players[playerIndex]->actor->rotY;
                        player->state++;
                    }
                    break;
                case 5:
                    transitionResult = fn_1_30D8(&lbl_1_bss_4.players[playerIndex]->actor->rotY,
                                        10.0f, player->turnYaw, 0.0f);
                    if ((s16)transitionResult == 0) {
                        lbl_1_bss_70C[playerIndex].shiftState = 1;
                        lbl_1_bss_70C[playerIndex].shiftMotion = 0;
                        lbl_1_bss_70C[playerIndex].shiftStart = 0.0f;
                        lbl_1_bss_70C[playerIndex].blendDuration = 10.0f;
                        lbl_1_bss_70C[playerIndex].shiftFlags = 0;
                        lbl_1_bss_70C[playerIndex].attributesToReset = HU3D_MOTATTR_LOOP;
                        player->state++;
                    }
                    break;
                case 6:
                    player->position.x = player->routePosition.x;
                    player->position.z = player->routePosition.z;
                    break;
                }
        }
    }
}

/* Advances active segments and their linked collision models each course frame, then updates
 * attached effects and obstacles. */
void fn_1_6974(void) {
    s16 modelIndex;
    s16 motionFrameOrSampleIndex;

    lbl_1_bss_30.pushSpeed = 5.0f;
    for (modelIndex = 46; modelIndex <= 79; modelIndex++) {
        if (lbl_1_bss_8BC[modelIndex].status >= 0) {
            fn_1_7150(&lbl_1_bss_8BC[modelIndex]);
        }
    }
    for (modelIndex = 46; modelIndex <= 79; modelIndex++) {
        Hu3DModelPosSet(lbl_1_bss_8BC[modelIndex].model,
                        lbl_1_bss_8BC[modelIndex].position.x,
                        lbl_1_bss_8BC[modelIndex].position.y,
                        lbl_1_bss_8BC[modelIndex].position.z);
        Hu3DModelRotSet(
            lbl_1_bss_8BC[modelIndex].model,
            lbl_1_bss_8BC[modelIndex].rotation.x,
            lbl_1_bss_8BC[modelIndex].rotation.y,
            lbl_1_bss_8BC[modelIndex].rotation.z);
    }

    lbl_1_bss_8BC[6].position.x = lbl_1_bss_8BC[50].position.x;
    lbl_1_bss_8BC[6].position.y = lbl_1_bss_8BC[50].position.y;
    lbl_1_bss_8BC[6].position.z = lbl_1_bss_8BC[50].position.z - 100.0f;
    lbl_1_bss_8BC[9].position.x = lbl_1_bss_8BC[50].position.x;
    lbl_1_bss_8BC[9].position.y = lbl_1_bss_8BC[50].position.y;
    lbl_1_bss_8BC[9].position.z = lbl_1_bss_8BC[50].position.z;

    for (modelIndex = 32; modelIndex <= 45; modelIndex++) {
        if (lbl_1_bss_8BC[modelIndex].linkedRecord != -1) {
            Hu3DModelObjPosGet(
                lbl_1_bss_8BC[
                    lbl_1_bss_8BC[modelIndex].linkedRecord].model,
                lbl_1_data_58[lbl_1_bss_8BC[lbl_1_bss_8BC[modelIndex].linkedRecord].resourceIndex],
                &lbl_1_bss_8BC[lbl_1_bss_8BC[modelIndex].linkedRecord].jointPositionOrDriftOffset);
            lbl_1_bss_8BC[modelIndex].position =
                lbl_1_bss_8BC[lbl_1_bss_8BC[modelIndex].linkedRecord].jointPositionOrDriftOffset;

            if (lbl_1_bss_8BC[modelIndex].linkedRecord == 75) {
                lbl_1_bss_8BC[67].jointPositionOrDriftOffset =
                    lbl_1_bss_8BC[lbl_1_bss_8BC[modelIndex].linkedRecord]
                        .jointPositionOrDriftOffset;
                lbl_1_bss_8BC[67].jointPositionOrDriftOffset.x = lbl_1_bss_8BC[67].position.x;
            }

            {
                motionFrameOrSampleIndex = (s16)Hu3DMotionTimeGet(
                    lbl_1_bss_8BC[
                        lbl_1_bss_8BC[modelIndex].linkedRecord].model);
                if (motionFrameOrSampleIndex < 120 && motionFrameOrSampleIndex != 0 &&
                    lbl_1_bss_8BC[modelIndex].linkedRecord != 50) {
                /* The attachment rotation curve is sampled at ten-frame intervals. */
                if (lbl_1_bss_30.scroll) {
                    if (motionFrameOrSampleIndex % 10 == 0) {
                        lbl_1_bss_8BC[modelIndex].rotation.x =
                            lbl_1_data_C8[motionFrameOrSampleIndex / 10];
                    }
                }
                motionFrameOrSampleIndex /= 10;
                if (lbl_1_bss_30.scroll) {
                    fn_1_30D8(
                        &lbl_1_bss_8BC[modelIndex].rotation.x,
                        (10.0f),
                        lbl_1_data_C8[motionFrameOrSampleIndex],
                        lbl_1_data_C8[motionFrameOrSampleIndex + 1]);
                    lbl_1_bss_8BC[lbl_1_bss_8BC[modelIndex].linkedRecord].collisionRotation =
                        lbl_1_bss_8BC[modelIndex].rotation;
                }
                } else {
                    lbl_1_bss_8BC[modelIndex].rotation.x =
                        (0.0f);
                    lbl_1_bss_8BC[modelIndex].rotation.y =
                        (0.0f);
                    lbl_1_bss_8BC[modelIndex].rotation.z =
                        (0.0f);
                }
            }

            /* Collision models linked to segments 70, 71, 74 or 75 force X to zero,
             * overriding the joint's world X position copied above. */
            if (lbl_1_bss_8BC[modelIndex].linkedRecord == 70 ||
                lbl_1_bss_8BC[modelIndex].linkedRecord == 71 ||
                lbl_1_bss_8BC[modelIndex].linkedRecord == 74 ||
                lbl_1_bss_8BC[modelIndex].linkedRecord == 75) {
                lbl_1_bss_8BC[modelIndex].position.x = (0.0f);
            }
        } else {
            lbl_1_bss_8BC[modelIndex].position.y = (-10000.0f);
        }
    }

    for (modelIndex = 32; modelIndex <= 45; modelIndex++) {
        Hu3DModelPosSet(
            lbl_1_bss_8BC[modelIndex].model,
            lbl_1_bss_8BC[modelIndex].position.x,
            lbl_1_bss_8BC[modelIndex].position.y,
            lbl_1_bss_8BC[modelIndex].position.z);
        Hu3DModelRotSet(
            lbl_1_bss_8BC[modelIndex].model,
            lbl_1_bss_8BC[modelIndex].rotation.x,
            lbl_1_bss_8BC[modelIndex].rotation.y,
            lbl_1_bss_8BC[modelIndex].rotation.z);
    }
    fn_1_7E10();
    fn_1_8670();
    fn_1_8C54();
}

/* Advances one active segment's arrival, bobbing, animation, and departure; called by the course
 * model update. */
void fn_1_7150(M609Model *segment) {
    s16 attachmentIndex;
    s16 attachmentEnd = 0;
    s16 keepSegment = 1;
    s16 linkSlot;

    switch (segment->stateOrTimer) {
    case 0: {
        segment->position.z +=
            lbl_1_bss_30.scroll;
        if (segment->position.z >
            lbl_1_bss_30.startZ) {
            for (linkSlot = 0; linkSlot < 6; linkSlot++) {
                segment->linkedRecords[linkSlot] = -1;
            }

            if (lbl_1_bss_30.resultFlag != 0 &&
                segment->recordIndex != 50) {
                keepSegment = 0;

                if (lbl_1_bss_8BC[60].stateOrTimer != 0 &&
                    (segment->recordIndex == 64 ||
                     segment->recordIndex == 61 ||
                     segment->recordIndex == 65)) {
                    keepSegment = 1;
                }
                if (lbl_1_bss_8BC[68].stateOrTimer != 0 &&
                    (segment->recordIndex == 72 ||
                     segment->recordIndex == 69 ||
                     segment->recordIndex == 73)) {
                    keepSegment = 1;
                }
                if (lbl_1_bss_8BC[70].stateOrTimer != 0 &&
                    (segment->recordIndex == 74 ||
                     segment->recordIndex == 71 ||
                     segment->recordIndex == 75)) {
                    keepSegment = 1;
                }
                if (lbl_1_bss_8BC[62].stateOrTimer != 0 &&
                    (segment->recordIndex == 66 ||
                     segment->recordIndex == 63 ||
                     segment->recordIndex == 67)) {
                    keepSegment = 1;
                }
                if (lbl_1_bss_8BC[76].stateOrTimer != 0 &&
                    segment->recordIndex == 78) {
                    keepSegment = 1;
                }
                if (lbl_1_bss_8BC[77].stateOrTimer != 0 &&
                    segment->recordIndex == 79) {
                    keepSegment = 1;
                }
                if (lbl_1_bss_8BC[50].stateOrTimer == 0 &&
                    segment->recordIndex == 55) {
                    keepSegment = 1;
                }
                /* Keep segments alive while linked models still depend on them. */
                if (!keepSegment) {
                    segment->status = -1;
                    if (lbl_1_bss_8BC[50].stateOrTimer == 0) {
                        lbl_1_bss_30.skippedSegmentModel =
                            segment->recordIndex;
                        lbl_1_bss_8BC[50].position.z =
                            segment->position.z;
                    }
                }
            }
            if (keepSegment) {
                if (segment->recordIndex != 60 &&
                    segment->recordIndex != 64 &&
                    segment->recordIndex != 61 &&
                    segment->recordIndex != 68 &&
                    segment->recordIndex != 72 &&
                    segment->recordIndex != 69 &&
                    segment->recordIndex != 70 &&
                    segment->recordIndex != 74 &&
                    segment->recordIndex != 71 &&
                    segment->recordIndex != 62 &&
                    segment->recordIndex != 66 &&
                    segment->recordIndex != 63 &&
                    segment->recordIndex != 76 &&
                    segment->recordIndex != 77 &&
                    segment->recordIndex != 78 &&
                    segment->recordIndex != 79 &&
                    segment->recordIndex != 55) {
                    /* Roll before checking spawnDecisionMade, so a processed segment still consumes
                     * a random value on another arrival pass. Mark it processed even when no
                     * obstacle is created. */
                    if (frandmod(2) == 0 && segment->spawnDecisionMade == 0) {
                    if (segment->recordIndex == 65 ||
                        segment->recordIndex == 73 ||
                        segment->recordIndex == 75 ||
                        segment->recordIndex == 67) {
                        fn_1_89E0(segment->bounds.z +
                            (segment->position.z + segment->bounds.z));
                    } else {
                        fn_1_89E0((50.0f) +
                            (segment->position.z + segment->bounds.z));
                    }
                    }
                    segment->spawnDecisionMade = 1;
                }

                    Hu3DModelAttrReset(segment->model, HU3D_ATTR_DISPOFF);
                    if (segment->recordIndex >= 60 &&
                        segment->recordIndex <= 67) {
                        segment->rotation.x =
                            (6.0f) *
                            ((segment->position.z - (100.0f)) /
                             lbl_1_bss_30.scroll);
                    } else if (segment->recordIndex >= 68 &&
                               segment->recordIndex <= 75) {
                        segment->rotation.x =
                            (-6.0f) *
                            ((segment->position.z - (100.0f)) /
                             lbl_1_bss_30.scroll);
                    }
                    if (lbl_1_data_FC[segment->status] != -1) {
                        lbl_1_bss_8BC[lbl_1_data_FC[segment->status]].linkedRecord =
                            segment->recordIndex;
                    }

                        switch (segment->recordIndex) {
                        case 48:
                            attachmentIndex = 3;
                            attachmentEnd = 9;
                            break;
                        case 49:
                            attachmentIndex = 9;
                            attachmentEnd = 12;
                            break;
                        case 77:
                            attachmentIndex = 12;
                            attachmentEnd = 14;
                            break;
                        case 79:
                            attachmentIndex = 14;
                            attachmentEnd = 15;
                            break;
                        case 57:
                            attachmentIndex = 15;
                            attachmentEnd = 17;
                            break;
                        case 58:
                            attachmentIndex = 17;
                            attachmentEnd = 18;
                            break;
                        case 59:
                            attachmentIndex = 18;
                            attachmentEnd = 20;
                            break;
                        case 50:
                            Hu3DModelAttrReset(
                                lbl_1_bss_8BC[6].model, HU3D_ATTR_DISPOFF);
                            Hu3DModelAttrReset(
                                lbl_1_bss_8BC[9].model, HU3D_ATTR_DISPOFF);
                            attachmentIndex = 20;
                            attachmentEnd = 21;
                            break;
                        }

                    /* Place and activate this segment's linked auxiliary models. */
                    if (attachmentEnd != 0) {
                        for (linkSlot = 0; attachmentIndex < attachmentEnd;
                             attachmentIndex++, linkSlot++) {
                            Hu3DModelAttrReset(
                                lbl_1_bss_8BC[lbl_1_data_140[attachmentIndex].modelIndex].model,
                                HU3D_ATTR_DISPOFF);
                            lbl_1_bss_8BC[lbl_1_data_140[attachmentIndex].modelIndex]
                                .movementValues = lbl_1_data_140[attachmentIndex].position;
                            lbl_1_bss_8BC[lbl_1_data_140[attachmentIndex].modelIndex].linkedRecord =
                                lbl_1_data_FC[segment->status];
                            lbl_1_bss_8BC[lbl_1_data_140[attachmentIndex].modelIndex]
                                .stateOrTimer = 0;
                            segment->linkedRecords[linkSlot] =
                                lbl_1_data_140[attachmentIndex].modelIndex;
                        }
                    }
                    segment->stateOrTimer += 1;
            }
        }
        break;
    }
    case 1: {
        segment->position.z +=
            lbl_1_bss_30.scroll;
        if (segment->bobOffset >= 10.0f) {
            segment->bobSpeed = -0.3f;
        } else if (segment->bobOffset <= -10.0f) {
            segment->bobSpeed = 0.3f;
        }
        segment->bobOffset +=
            segment->bobSpeed;
        segment->position.y +=
            segment->bobSpeed;
        if (segment->motionVerticalPosition <= -10.0f) {
            segment->motionVerticalSpeed = 1.0f;
        } else if (segment->motionVerticalPosition >= 0.0f &&
                   segment->motionVerticalSpeed > 0.0f) {
            segment->motionVerticalSpeed = 0.0f;
        }
        segment->motionVerticalPosition +=
            segment->motionVerticalSpeed;
        segment->position.y +=
            segment->motionVerticalSpeed;

        if (segment->recordIndex >= 60 &&
            segment->recordIndex <= 67) {
            segment->rotation.x += 6.0f;
        } else if (segment->recordIndex >= 68 &&
                   segment->recordIndex <= 75) {
            segment->rotation.x -= 6.0f;
        }
        if (segment->position.z >= 100.0f) {
            Hu3DMotionSpeedSet(segment->model, 1.0f);
            segment->rotation.x = 0.0f;
            segment->stateOrTimer += 1;
        }
        if (segment->position.z >= 0.0f &&
            segment->recordIndex == 50) {
            segment->stateOrTimer = 3;
        }
        break;
    }
    case 2: {
        if (Hu3DMotionTimeGet(segment->model) > 119.0f) {
            Hu3DModelAttrSet(segment->model, HU3D_ATTR_DISPOFF);
            segment->status = -1;
        }
        if (!lbl_1_bss_30.scroll) {
            Hu3DMotionSpeedSet(segment->model, 0.0f);
        } else {
            Hu3DMotionSpeedSet(segment->model, 1.0f);
        }
        break;
    }
    case 3: {
        segment->bobOffset +=
            segment->bobSpeed;
        segment->position.y +=
            segment->bobSpeed;
        if (segment->bobOffset >= 10.0f) {
            segment->bobSpeed = -0.3f;
        } else if (segment->bobOffset <= -10.0f) {
            segment->bobSpeed = 0.3f;
        }
        if (segment->motionVerticalPosition <= -10.0f) {
            segment->motionVerticalSpeed = 1.0f;
        } else if (segment->motionVerticalPosition >= 0.0f &&
                   segment->motionVerticalSpeed > 0.0f) {
            segment->motionVerticalSpeed = 0.0f;
        }
        segment->motionVerticalPosition +=
            segment->motionVerticalSpeed;
        segment->position.y +=
            segment->motionVerticalSpeed;
        break;
    }
    }

    if (segment->movementSelectors.x) {
        if (segment->stateOrTimer >= 2 && segment->movementValues.x < 0.5f &&
            segment->movementValues.x > -0.5f) {
            segment->movementValues.y = 0.0f;
            segment->movementValues.x = 0.0f;
        }
        segment->movementValues.x += segment->movementValues.y;
        segment->position.x += segment->movementValues.x;
        switch (segment->movementPhase) {
        case 0:
            if (segment->position.x < segment->positionStep.x) {
            segment->movementValues.y *= -1.0f;
            segment->movementPhase += 1;
            }
            break;
        case 1:
            if (segment->position.x > segment->positionStep.x) {
            segment->movementValues.y *= -1.0f;
            segment->movementPhase = 0;
            }
            break;
        }
    } else if (segment->movementSelectors.z) {
        if (segment->stateOrTimer >= 2) {
            segment->movementValues.y = 0.0f;
            segment->movementValues.z = 0.0f;
        }
        segment->movementValues.z += segment->movementValues.y;
        segment->positionStep.y += segment->movementValues.z;
        segment->position.z += segment->movementValues.z;
        switch (segment->movementPhase) {
        case 0:
            if (segment->positionStep.y < 0.0f) {
            segment->movementValues.y *= -1.0f;
            segment->movementPhase += 1;
            }
            break;
        case 1:
            if (segment->positionStep.y > 0.0f) {
            segment->movementValues.y *= -1.0f;
            segment->movementPhase = 0;
            }
            break;
        }
    }
}

/* Updates models attached to course joints, including tilt-following, drift, and falling; called by
 * the course model update. */
void fn_1_7E10(void) {
    s16 modelIndex;
    M609Model *attachment;
    HuVecF sourcePosition;
    HuVecF sourceRotation;
    HuVecF separation;
    f32 distanceSquared;
    f32 releaseZOrDistance;

    for (modelIndex = 144; modelIndex < 153; modelIndex++) {
        if (lbl_1_bss_8BC[modelIndex].linkedRecord != -1) {
            attachment = &lbl_1_bss_8BC[modelIndex];
            sourcePosition =
                lbl_1_bss_8BC[attachment->linkedRecord].position;
            sourceRotation = lbl_1_bss_8BC[attachment->linkedRecord].rotation;

            switch (attachment->stateOrTimer) {
            case 0:
                attachment->position.x = sourcePosition.x +
                    attachment->movementValues.x;
                attachment->position.y = sourcePosition.y +
                    attachment->movementValues.y +
                    lbl_1_bss_30.raftOffset.y;
                attachment->position.z = sourcePosition.z +
                    attachment->movementValues.z;
                attachment->rotation = sourceRotation;
                if (attachment->rotation.x) {
                    attachment->jointPositionOrDriftOffset.x = (0.0f);
                    attachment->stateOrTimer++;
                }
                break;
            case 1:
                attachment->movementValues.z = (f32)(
                    attachment->movementValues.z +
                    (0.2) * sourceRotation.x);
                attachment->position.x = sourcePosition.x +
                    attachment->movementValues.x;
                attachment->position.y = sourcePosition.y +
                    attachment->movementValues.z *
                    sin(((3.141592653589793) * -sourceRotation.x) /
                        (180.0));
                attachment->position.z = sourcePosition.z +
                    attachment->movementValues.z *
                    cos(((3.141592653589793) * -sourceRotation.x) /
                        (180.0));
                attachment->position.y += ((50.0) +
                    lbl_1_bss_30.raftOffset.y) *
                    cos(((3.141592653589793) * sourceRotation.x) /
                        (180.0));
                attachment->position.z += ((50.0) +
                    lbl_1_bss_30.raftOffset.y) *
                    sin(((3.141592653589793) * sourceRotation.x) /
                        (180.0));
                if (lbl_1_bss_30.night == 0) {
                    attachment->rotation = sourceRotation;
                } else {
                    /* This auxiliary record uses jointPositionOrDriftOffset.x for forward drift,
                     * increased by one world unit per update. */
                    attachment->jointPositionOrDriftOffset.x += (1.0f);
                }
                switch (attachment->linkedRecord) {
                case 32:
                case 33:
                    releaseZOrDistance = sourcePosition.z + (260.0) *
                        cos(((3.141592653589793) * sourceRotation.x) / (180.0));
                    if (releaseZOrDistance < attachment->position.z) {
                        attachment->stateOrTimer++;
                    }
                    break;
                case 44:
                    separation.x = sourcePosition.x - attachment->position.x;
                    separation.y = sourcePosition.y - attachment->position.y +
                        (50.0f);
                    separation.z = sourcePosition.z - (200.0f) -
                        attachment->position.z;
                    distanceSquared = separation.z * separation.z +
                        (separation.x * separation.x +
                        separation.y * separation.y);
                    releaseZOrDistance = sqrt(distanceSquared);
                    if (releaseZOrDistance > (420.0f)) {
                        attachment->stateOrTimer++;
                    }
                    break;
                case 45:
                    releaseZOrDistance = sourcePosition.z + (250.0) *
                        cos(((3.141592653589793) * sourceRotation.x) / (180.0));
                    if (releaseZOrDistance < attachment->position.z) {
                        attachment->stateOrTimer++;
                    }
                    break;
                case 37:
                case 38:
                case 39:
                    releaseZOrDistance = sourcePosition.z + (170.0) *
                        cos(((3.141592653589793) * sourceRotation.x) / (180.0));
                    if (releaseZOrDistance < attachment->position.z) {
                        attachment->stateOrTimer++;
                    }
                    break;
                }

                attachment->position.z += attachment->jointPositionOrDriftOffset.x;
                break;
            case 2:
                attachment->position.y -= (20.0f);
                if (attachment->position.y < (-1000.0f)) {
                    Hu3DModelAttrSet(attachment->model, HU3D_ATTR_DISPOFF);
                }
                break;
            }

            switch (attachment->movementPhase) {
            case 1:
                attachment->status = 0;
                attachment->positionStep.x = (10.0f);
                (attachment->movementPhase)++;
                break;
            case 2:
                if (attachment->status % 2 != 0) {
                    attachment->positionStep.x *= (-0.9f);
                }
                attachment->position.x += attachment->positionStep.x;
                if ((attachment->status)++ >= 50) {
                    attachment->movementPhase = 0;
                }
                break;
            }
        }
    }

    /* Attachment 153 uses model 50's nokotachi joint instead of its stored local offsets.
     * Keep model 50's main motion looping at speed 1 and force the attachment's yaw to 180. */
    if (lbl_1_bss_8BC[153].linkedRecord != -1) {
        Hu3DModelAttrSet(lbl_1_bss_8BC[50].model,
                         HU3D_MOTATTR_LOOP);
        Hu3DMotionSpeedSet(lbl_1_bss_8BC[50].model,
                           (1.0f));
        Hu3DModelObjPosGet(lbl_1_bss_8BC[50].model,
                           "nokotachi", &sourcePosition);
        lbl_1_bss_8BC[153].position = sourcePosition;
        lbl_1_bss_8BC[153].rotation.y = (180.0f);
        lbl_1_bss_8BC[9].position =
            lbl_1_bss_8BC[50].position;
    }
}

/* Updates effects that follow course segments and starts splashes at the player's X/Z
 * with Y forced to zero, hiding each after its animation threshold. */
void fn_1_8670(void) {
    s16 effectModelIndex;
    s16 playerNo;

    effectModelIndex = 80;
    while (effectModelIndex <= 143) {
        if (lbl_1_bss_8BC[effectModelIndex].linkedRecord != -1 &&
            lbl_1_bss_8BC[effectModelIndex].stateOrTimer == 1) {
            if (Hu3DMotionTimeGet(lbl_1_bss_8BC[effectModelIndex].model) >=
                (22.9f)) {
                Hu3DModelAttrSet(lbl_1_bss_8BC[effectModelIndex].model, HU3D_ATTR_DISPOFF);
                Hu3DMotionSpeedSet(lbl_1_bss_8BC[effectModelIndex].model,
                                   (0.0f));
                lbl_1_bss_8BC[effectModelIndex].stateOrTimer = 0;
                lbl_1_bss_8BC[effectModelIndex].linkedRecord = -1;
            } else {
                lbl_1_bss_8BC[effectModelIndex].position =
                    lbl_1_bss_8BC[
                        lbl_1_bss_8BC[effectModelIndex].linkedRecord].position;
            }
        }
        effectModelIndex++;
    }

    effectModelIndex = 28;
    playerNo = 0;
    while (effectModelIndex <= 31) {
        switch (lbl_1_bss_8BC[effectModelIndex].stateOrTimer) {
        case 0:
            break;
        case 1:
            Hu3DModelAttrReset(lbl_1_bss_8BC[effectModelIndex].model, HU3D_ATTR_DISPOFF);
            Hu3DMotionSpeedSet(lbl_1_bss_8BC[effectModelIndex].model,
                               (1.0f));
            lbl_1_bss_8BC[effectModelIndex].position.x =
                lbl_1_bss_24C[playerNo].position.x;
            lbl_1_bss_8BC[effectModelIndex].position.y = (0.0f);
            lbl_1_bss_8BC[effectModelIndex].position.z =
                lbl_1_bss_24C[playerNo].position.z;
            lbl_1_bss_8BC[effectModelIndex].stateOrTimer++;
            break;
        case 2:
            if (Hu3DMotionTimeGet(lbl_1_bss_8BC[effectModelIndex].model) >=
                (38.9f)) {
                Hu3DModelAttrSet(lbl_1_bss_8BC[effectModelIndex].model, HU3D_ATTR_DISPOFF);
                Hu3DMotionSpeedSet(lbl_1_bss_8BC[effectModelIndex].model,
                                   (0.0f));
                lbl_1_bss_8BC[effectModelIndex].stateOrTimer = 0;
            }
            break;
        }
        effectModelIndex++;
        playerNo++;
    }
}

/* Activates an unused drifting obstacle when a course segment enters its active state.
 * spawnZOrScrollSteps begins as world Z and is reused as a signed scroll-update count. */
s32 fn_1_89E0(f32 spawnZOrScrollSteps) {
    s32 spawned = 0;
    s16 firstObstacle;
    s16 lastObstacle;
    s16 obstacleIndex;
    s16 groupRoll;
    s32 randomValue;
    f32 heightOffset;

    groupRoll = frandmod(10);
    if (groupRoll != 0) {
        firstObstacle = 17;
        lastObstacle = 21;
    } else {
        firstObstacle = 22;
        lastObstacle = 26;
    }

    obstacleIndex = firstObstacle;
    while (obstacleIndex <= lastObstacle) {
        if (lbl_1_bss_8BC[obstacleIndex].status == 0) {
            heightOffset = (f32)(obstacleIndex - 17);
            lbl_1_bss_8BC[obstacleIndex].status = 1;
            randomValue = frandmod(1100) - 550;
            lbl_1_bss_8BC[obstacleIndex].position.x =
                (f32)randomValue;
            lbl_1_bss_8BC[obstacleIndex].position.y =
                heightOffset / (10.0f);
            lbl_1_bss_8BC[obstacleIndex].position.z =
                spawnZOrScrollSteps;
            lbl_1_bss_8BC[obstacleIndex].rotation.y =
                (0.0f);
            randomValue = frandmod(3) + 1;
            lbl_1_bss_8BC[obstacleIndex].movementValues.y =
                (f32)randomValue;
            /* Lead the spin by the number of scroll updates before reaching the front. */
            spawnZOrScrollSteps -= (100.0f);
            spawnZOrScrollSteps /= lbl_1_bss_30.scroll;
            lbl_1_bss_8BC[obstacleIndex].rotation.y =
                spawnZOrScrollSteps *
                lbl_1_bss_8BC[obstacleIndex].movementValues.y;
            Hu3DModelAttrReset(lbl_1_bss_8BC[obstacleIndex].model,
                               HU3D_ATTR_DISPOFF);
            spawned = 1;
            break;
        }
        obstacleIndex++;
    }

    return spawned;
}

/* Moves spawned obstacles with the course scroll and retires them after their front-of-course
 * animation. */
void fn_1_8C54(void) {
    s16 obstacleIndex = 17;

    while (obstacleIndex <= 26) {
        if (lbl_1_bss_8BC[obstacleIndex].status != 0) {
            switch (lbl_1_bss_8BC[obstacleIndex].stateOrTimer) {
            case 0:
                lbl_1_bss_8BC[obstacleIndex].position.z +=
                    lbl_1_bss_30.scroll;
                lbl_1_bss_8BC[obstacleIndex].rotation.y +=
                    lbl_1_bss_8BC[obstacleIndex].movementValues.y;
                if (lbl_1_bss_8BC[obstacleIndex].position.z >=
                    (100.0f)) {
                    Hu3DMotionSpeedSet(lbl_1_bss_8BC[obstacleIndex].model,
                                       (1.0f));
                    Hu3DMotionTimeSet(lbl_1_bss_8BC[obstacleIndex].model,
                                      (0.0f));
                    lbl_1_bss_8BC[obstacleIndex].rotation.y =
                        (0.0f);
                    (lbl_1_bss_8BC[obstacleIndex].stateOrTimer)++;
                }
                break;
            case 1:
                if (Hu3DMotionTimeGet(lbl_1_bss_8BC[obstacleIndex].model) >
                    (118.9f)) {
                    Hu3DModelAttrSet(lbl_1_bss_8BC[obstacleIndex].model,
                                     HU3D_ATTR_DISPOFF);
                    Hu3DMotionSpeedSet(lbl_1_bss_8BC[obstacleIndex].model,
                                       (0.0f));
                    Hu3DMotionTimeSet(lbl_1_bss_8BC[obstacleIndex].model,
                                      (0.0f));
                    lbl_1_bss_8BC[obstacleIndex].status = 0;
                    lbl_1_bss_8BC[obstacleIndex].stateOrTimer = 0;
                }
                break;
            }
        }
        obstacleIndex++;
    }
}

/* Runs after actor movement to resolve course contacts, knockback, platform boundaries, and
 * off-course player phases. */
void fn_1_8EE4(void)
{
    M609Player *player;
    s16 modelIndex;
    s16 playerOrSegmentIndex;
    s16 edgeIndexOrContactGate;
    s16 positionCorrected;
    f32 xOffsetOrAngle;
    f32 zOffsetOrAngle;
    f32 angleDifferenceOrScale;
    f32 distanceOrScale;
    f32 contactRadiusOrDistance;
    /* Boundary projection loops reuse this for squared XZ edge length. */
    f32 cameraAngleOrEdgeLengthSquared;
    f32 hazardDistance;
    f32 boundaryRadius;
    f32 obstacleDistance;
    f32 directionAngleOrProjectionNumerator;
    f32 projectionScale;
    f32 normalAngle;
    f32 edgeAngle;
    f32 movementAngle;
    Vec cameraTargetOrContactVector;
    Vec cameraUpOrBoundaryVector;
    Vec delta;
    Vec cameraDeltaOrBoundaryPoint;
    Vec cameraPosition;
    Vec normals[4];
    M609CollisionEdge edges[4];

    /* Apply the initial scroll displacement only on the first two correction updates. */
    if (lbl_1_bss_6D84 <= 1) {
        for (playerOrSegmentIndex = 0; playerOrSegmentIndex < 4; playerOrSegmentIndex++) {
            lbl_1_bss_24C[playerOrSegmentIndex].position.z += lbl_1_bss_30.scroll;
            Hu3DModelPosSet(lbl_1_bss_8BC[playerOrSegmentIndex].model,
                lbl_1_bss_24C[playerOrSegmentIndex].position.x,
                lbl_1_bss_24C[playerOrSegmentIndex].position.y,
                lbl_1_bss_24C[playerOrSegmentIndex].position.z);
        }
        lbl_1_bss_6D84++;
    }
    normals[0].x = 0.0f;
    normals[0].y = 0.0f;
    normals[0].z = 1.0f;
    normals[1].x = 0.0f;
    normals[1].y = 0.0f;
    normals[1].z = -1.0f;
    normals[2].x = 1.0f;
    normals[2].y = 0.0f;
    normals[2].z = 0.0f;
    normals[3].x = -1.0f;
    normals[3].y = 0.0f;
    normals[3].z = 0.0f;
    lbl_1_bss_30.hazardRadius = 50.0f;
    lbl_1_bss_30.playerRadius = 60.0f;
    lbl_1_bss_30.obstacleRadius = 20.0f;
    boundaryRadius = 60.0f;
    hazardDistance = lbl_1_bss_30.playerRadius + lbl_1_bss_30.hazardRadius;
    obstacleDistance = lbl_1_bss_30.playerRadius + lbl_1_bss_30.obstacleRadius;
    for (playerOrSegmentIndex = 0; playerOrSegmentIndex < 4; playerOrSegmentIndex++) {
        player = &lbl_1_bss_24C[playerOrSegmentIndex];
        if (player->motionFlag != 0) {
            if (player->phase == 0) {
                if (player->direction != 0) {
                    player->position.z += lbl_1_bss_30.pushSpeed * player->direction;
                    player->direction = 0;
                }
                for (modelIndex = 144; modelIndex <= 152; modelIndex++) {
                    delta.x = player->position.x - lbl_1_bss_8BC[modelIndex].position.x;
                    delta.y = player->position.y - lbl_1_bss_8BC[modelIndex].position.y;
                    delta.z = player->position.z - lbl_1_bss_8BC[modelIndex].position.z;
                    distanceOrScale = sqrt(delta.z * delta.z +
                                 (delta.x * delta.x + delta.y * delta.y));
                    if (distanceOrScale < hazardDistance) {
                        if (lbl_1_bss_30.night == 0) {
                            fn_1_46F8(4, 0, &player->position);
                            lbl_1_bss_8BC[modelIndex].movementPhase = 1;
                        } else {
                            fn_1_46F8(5, 0, &player->position);
                        }
                        Hu3DCameraPosGet(1, &cameraPosition, &cameraUpOrBoundaryVector,
                                         &cameraTargetOrContactVector);
                        cameraDeltaOrBoundaryPoint.x = cameraPosition.x - player->position.x;
                        cameraDeltaOrBoundaryPoint.y = cameraPosition.y - player->position.y;
                        cameraDeltaOrBoundaryPoint.z = cameraPosition.z - player->position.z;
                        cameraAngleOrEdgeLengthSquared =
                            180.0 *
                            (atan2(cameraDeltaOrBoundaryPoint.x, cameraDeltaOrBoundaryPoint.z) /
                             3.141592653589793);
                        directionAngleOrProjectionNumerator =
                            180.0 * (atan2(delta.x, delta.z) / 3.141592653589793);
                        if ((cameraAngleOrEdgeLengthSquared > 90.0f &&
                             directionAngleOrProjectionNumerator < -90.0f) ||
                            (cameraAngleOrEdgeLengthSquared < -90.0f &&
                             directionAngleOrProjectionNumerator > 90.0f)) {
                            xOffsetOrAngle = cameraAngleOrEdgeLengthSquared < 0.0f
                                          ? 180.0f + cameraAngleOrEdgeLengthSquared
                                          : 180.0f - cameraAngleOrEdgeLengthSquared;
                            zOffsetOrAngle = directionAngleOrProjectionNumerator < 0.0f
                                          ? 180.0f + directionAngleOrProjectionNumerator
                                          : 180.0f - directionAngleOrProjectionNumerator;
                            angleDifferenceOrScale =
                                (xOffsetOrAngle + zOffsetOrAngle) > 0.0f
                                    ? xOffsetOrAngle + zOffsetOrAngle
                                    : -1.0f * (xOffsetOrAngle + zOffsetOrAngle);
                            xOffsetOrAngle = xOffsetOrAngle + zOffsetOrAngle;
                        } else {
                            angleDifferenceOrScale =
                                (cameraAngleOrEdgeLengthSquared -
                                 directionAngleOrProjectionNumerator) > 0.0f
                                    ? cameraAngleOrEdgeLengthSquared -
                                          directionAngleOrProjectionNumerator
                                    : -1.0f * (cameraAngleOrEdgeLengthSquared -
                                               directionAngleOrProjectionNumerator);
                            xOffsetOrAngle = cameraAngleOrEdgeLengthSquared -
                                      directionAngleOrProjectionNumerator;
                        }
                        /* Within twenty degrees of the player-to-camera direction,
                         * replace knockback with a direction six degrees to either side
                         * of that camera direction. */
                        if (angleDifferenceOrScale < 20.0f) {
                            xOffsetOrAngle = xOffsetOrAngle >= 0.0f ? 1 : -1;
                            cameraAngleOrEdgeLengthSquared += 6.0f * xOffsetOrAngle;
                            delta.x =
                                sin((3.141592653589793 * cameraAngleOrEdgeLengthSquared) / 180.0);
                            delta.z =
                                cos((3.141592653589793 * cameraAngleOrEdgeLengthSquared) / 180.0);
                        }
                        if (lbl_1_bss_30.night == 0) {
                            distanceOrScale = sqrt(delta.x * delta.x + delta.z * delta.z);
                            distanceOrScale = 40.0f / distanceOrScale;
                            delta.x *= distanceOrScale;
                            delta.z *= distanceOrScale;
                            player->velocity.x = delta.x;
                            player->velocity.y = 30.0f;
                            player->velocity.z = delta.z;
                            player->spinSpeed = 0.0f;
                        } else {
                            distanceOrScale = sqrt(delta.x * delta.x + delta.z * delta.z);
                            distanceOrScale = 20.0f / distanceOrScale;
                            delta.x *= distanceOrScale;
                            delta.z *= distanceOrScale;
                            player->velocity.x = delta.x;
                            player->velocity.y = 15.0f;
                            player->velocity.z = delta.z;
                            player->spinSpeed = 0.0f;
                        }
                        player->phase = 2;
                        break;
                    }
                }
                if (player->position.y < -10.0f && player->position.z < 200.0f &&
                    player->collisionMesh < 0) {
                    player->phase = 1;
                } else if (player->position.y < -50.0f && player->position.z < 300.0f &&
                           player->collisionMesh < 0) {
                    player->phase = 1;
                } else if (player->position.z > 400.0f) {
                    player->phase = 4;
                } else if (player->position.y < -50.0f) {
                    player->phase = 4;
                }
            } else if (player->phase == 1) {
                /* Separate players in the water before resolving drifting obstacles. */
                for (modelIndex = 0; modelIndex < 4; modelIndex++) {
                    if (playerOrSegmentIndex != modelIndex &&
                        lbl_1_bss_24C[modelIndex].phase == 1) {
                        delta.x = player->position.x - lbl_1_bss_24C[modelIndex].position.x;
                        delta.z = player->position.z - lbl_1_bss_24C[modelIndex].position.z;
                        /* Replace each X/Z difference within 0.1 of zero with +1,
                         * including small negative differences. */
                        if (delta.x <= 0.1f && delta.x >= -0.1f) {
                            delta.x = 1.0f;
                        }
                        if (delta.z <= 0.1f && delta.z >= -0.1f) {
                            delta.z = 1.0f;
                        }
                        distanceOrScale = sqrt(delta.x * delta.x + delta.z * delta.z);
                        if (distanceOrScale < 2.0f * lbl_1_bss_30.playerRadius) {
                            distanceOrScale = (2.0f * lbl_1_bss_30.playerRadius) / distanceOrScale;
                            delta.x = delta.x * distanceOrScale / 2.0f;
                            delta.z = delta.z * distanceOrScale / 2.0f;
                            distanceOrScale = sqrt(delta.x * delta.x + delta.z * delta.z);
                            if (distanceOrScale > 5.0f) {
                                angleDifferenceOrScale = 5.0f / distanceOrScale;
                                delta.x *= angleDifferenceOrScale;
                                delta.z *= angleDifferenceOrScale;
                            }
                            player->position.x += delta.x;
                            player->position.z += delta.z;
                            lbl_1_bss_24C[modelIndex].position.x -= delta.x;
                            lbl_1_bss_24C[modelIndex].position.z -= delta.z;
                        }
                    }
                }
                for (modelIndex = 17; modelIndex <= 26; modelIndex++) {
                    if (lbl_1_bss_8BC[modelIndex].status != 0) {
                        delta.x = player->position.x - lbl_1_bss_8BC[modelIndex].position.x;
                        delta.z = player->position.z - lbl_1_bss_8BC[modelIndex].position.z;
                        /* Replace tiny X differences with +1 and tiny Z differences with -1,
                         * regardless of their original signs. */
                        if (delta.x <= 0.1f && delta.x >= -0.1f) {
                            delta.x = 1.0f;
                        }
                        if (delta.z <= 0.1f && delta.z >= -0.1f) {
                            delta.z = -1.0f;
                        }
                        distanceOrScale = sqrt(delta.x * delta.x + delta.z * delta.z);
                        if (distanceOrScale < obstacleDistance) {
                            distanceOrScale = obstacleDistance / distanceOrScale;
                            delta.x = delta.x * distanceOrScale / 2.0f;
                            delta.z = delta.z * distanceOrScale / 2.0f;
                            distanceOrScale = sqrt(delta.x * delta.x + delta.z * delta.z);
                            if (distanceOrScale > 5.0f) {
                                angleDifferenceOrScale = 5.0f / distanceOrScale;
                                delta.x *= angleDifferenceOrScale;
                                delta.z *= angleDifferenceOrScale;
                            }
                            player->position.x += delta.x;
                            player->position.z += delta.z;
                            lbl_1_bss_8BC[modelIndex].position.x -= delta.x;
                            lbl_1_bss_8BC[modelIndex].position.z -= delta.z;
                        }
                    }
                }
                for (modelIndex = 46; modelIndex <= 79; modelIndex++) {
                    if (lbl_1_bss_8BC[modelIndex].status != -1 &&
                        lbl_1_bss_8BC[modelIndex].position.z > -1800.0f &&
                        lbl_1_bss_8BC[modelIndex].position.z <= 100.0f) {
                        if (modelIndex == 78 || modelIndex == 79) {
                            delta.x = player->position.x -
                                      lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.x;
                            delta.z =
                                player->position.z -
                                (200.0f + lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.z);
                            /* Give an exactly zero X separation a positive component,
                             * even when Z is nonzero. */
                            if (delta.x == 0.0f) {
                                delta.x = 1.0f;
                            }
                            distanceOrScale = sqrt(delta.x * delta.x + delta.z * delta.z);
                            contactRadiusOrDistance =
                                lbl_1_bss_30.playerRadius + lbl_1_bss_8BC[modelIndex].bounds.x;
                            edgeIndexOrContactGate = 1;
                            /* While model 76/77 has positive motion time, skip circular correction
                             * when player Z is at least model 78/79's joint/contact Z plus 200,
                             * respectively. */
                            if (modelIndex == 78) {
                                if (Hu3DMotionTimeGet(lbl_1_bss_8BC[76].model) > 0.0f &&
                                    delta.z >= 0.0f) {
                                    edgeIndexOrContactGate = 0;
                                }
                            } else if (modelIndex == 79 &&
                                       Hu3DMotionTimeGet(lbl_1_bss_8BC[77].model) > 0.0f &&
                                       delta.z >= 0.0f) {
                                edgeIndexOrContactGate = 0;
                            }
                            if (distanceOrScale < contactRadiusOrDistance &&
                                edgeIndexOrContactGate != 0) {
                                distanceOrScale = contactRadiusOrDistance / distanceOrScale;
                                delta.x *= distanceOrScale;
                                delta.z *= distanceOrScale;
                                player->position.x =
                                    delta.x +
                                    lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.x;
                                player->position.z =
                                    delta.z +
                                    (200.0f +
                                     lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.z);
                            }
                        } else if (lbl_1_bss_8BC[modelIndex].bounds.x >= 0.0f) {
                            edges[0].startX =
                                lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.x +
                                lbl_1_bss_8BC[modelIndex].bounds.x;
                            edges[0].endX = lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.x -
                                            lbl_1_bss_8BC[modelIndex].bounds.x;
                            edges[0].startZ =
                                lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.z +
                                lbl_1_bss_8BC[modelIndex].bounds.z;
                            edges[0].endZ = lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.z +
                                            lbl_1_bss_8BC[modelIndex].bounds.z;
                            edges[1].startX =
                                lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.x +
                                lbl_1_bss_8BC[modelIndex].bounds.x;
                            edges[1].endX = lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.x -
                                            lbl_1_bss_8BC[modelIndex].bounds.x;
                            edges[1].startZ =
                                lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.z -
                                lbl_1_bss_8BC[modelIndex].bounds.z;
                            edges[1].endZ = lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.z -
                                            lbl_1_bss_8BC[modelIndex].bounds.z;
                            edges[2].startX =
                                lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.x +
                                lbl_1_bss_8BC[modelIndex].bounds.x;
                            edges[2].endX = lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.x +
                                            lbl_1_bss_8BC[modelIndex].bounds.x;
                            edges[2].startZ =
                                lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.z +
                                lbl_1_bss_8BC[modelIndex].bounds.z;
                            edges[2].endZ = lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.z -
                                            lbl_1_bss_8BC[modelIndex].bounds.z;
                            edges[3].startX =
                                lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.x -
                                lbl_1_bss_8BC[modelIndex].bounds.x;
                            edges[3].endX = lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.x -
                                            lbl_1_bss_8BC[modelIndex].bounds.x;
                            edges[3].startZ =
                                lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.z +
                                lbl_1_bss_8BC[modelIndex].bounds.z;
                            edges[3].endZ = lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.z -
                                            lbl_1_bss_8BC[modelIndex].bounds.z;
                            if (modelIndex == 65 || modelIndex == 73 ||
                                modelIndex == 75 || modelIndex == 67) {
                                for (edgeIndexOrContactGate = 0; edgeIndexOrContactGate < 4;
                                     edgeIndexOrContactGate++) {
                                    edges[edgeIndexOrContactGate].startZ += 150.0f;
                                    edges[edgeIndexOrContactGate].endZ += 150.0f;
                                }
                            }
                            if (edges[0].startX + boundaryRadius > player->position.x &&
                                edges[0].endX - boundaryRadius < player->position.x &&
                                edges[2].startZ + boundaryRadius > player->position.z &&
                                edges[2].endZ - boundaryRadius < player->position.z) {
                                if (edges[0].startX > player->position.x &&
                                    edges[0].endX < player->position.x &&
                                    edges[2].startZ > player->position.z &&
                                    edges[2].endZ < player->position.z) {
                                    for (edgeIndexOrContactGate = 0, distanceOrScale = 10000.0f;
                                         edgeIndexOrContactGate < 4; edgeIndexOrContactGate++) {
                                        xOffsetOrAngle = edges[edgeIndexOrContactGate].startX -
                                                  edges[edgeIndexOrContactGate].endX;
                                        zOffsetOrAngle = edges[edgeIndexOrContactGate].startZ -
                                                  edges[edgeIndexOrContactGate].endZ;
                                        cameraAngleOrEdgeLengthSquared =
                                            xOffsetOrAngle * xOffsetOrAngle +
                                            zOffsetOrAngle * zOffsetOrAngle;
                                        directionAngleOrProjectionNumerator =
                                            -1.0f * (xOffsetOrAngle *
                                                         (edges[edgeIndexOrContactGate].startX -
                                                          player->position.x) +
                                                     zOffsetOrAngle *
                                                         (edges[edgeIndexOrContactGate].startZ -
                                                          player->position.z));
                                        projectionScale =
                                            cameraAngleOrEdgeLengthSquared
                                                ? directionAngleOrProjectionNumerator /
                                                      cameraAngleOrEdgeLengthSquared
                                                : 0.0f;
                                        cameraDeltaOrBoundaryPoint.x =
                                            xOffsetOrAngle * projectionScale +
                                            edges[edgeIndexOrContactGate].startX;
                                        cameraDeltaOrBoundaryPoint.z =
                                            zOffsetOrAngle * projectionScale +
                                            edges[edgeIndexOrContactGate].startZ;
                                        xOffsetOrAngle =
                                            player->position.x - cameraDeltaOrBoundaryPoint.x;
                                        zOffsetOrAngle =
                                            player->position.z - cameraDeltaOrBoundaryPoint.z;
                                        contactRadiusOrDistance =
                                            sqrt(xOffsetOrAngle * xOffsetOrAngle +
                                                 zOffsetOrAngle * zOffsetOrAngle);
                                        if (distanceOrScale > contactRadiusOrDistance) {
                                            distanceOrScale = contactRadiusOrDistance;
                                            cameraUpOrBoundaryVector = cameraDeltaOrBoundaryPoint;
                                            cameraTargetOrContactVector.x = xOffsetOrAngle;
                                            cameraTargetOrContactVector.z = zOffsetOrAngle;
                                        }
                                    }
                                    /* This uses the last edge's distance even when another
                                     * edge supplied cameraUpOrBoundaryVector and
                                     * cameraTargetOrContactVector. */
                                    angleDifferenceOrScale =
                                        contactRadiusOrDistance
                                            ? boundaryRadius / contactRadiusOrDistance
                                            : 0.0f;
                                    cameraTargetOrContactVector.x = -1.0f *
                                                                    cameraTargetOrContactVector.x *
                                                                    angleDifferenceOrScale;
                                    cameraTargetOrContactVector.z = -1.0f *
                                                                    cameraTargetOrContactVector.z *
                                                                    angleDifferenceOrScale;
                                    player->position.x =
                                        cameraUpOrBoundaryVector.x + cameraTargetOrContactVector.x;
                                    player->position.z =
                                        cameraUpOrBoundaryVector.z + cameraTargetOrContactVector.z;
                                }
                                /* Compare the edge normal with movement relative to the segment. */
                                for (edgeIndexOrContactGate = 0, positionCorrected = 0;
                                     edgeIndexOrContactGate < 4; edgeIndexOrContactGate++) {
                                    xOffsetOrAngle = edges[edgeIndexOrContactGate].startX -
                                              edges[edgeIndexOrContactGate].endX;
                                    zOffsetOrAngle = edges[edgeIndexOrContactGate].startZ -
                                              edges[edgeIndexOrContactGate].endZ;
                                    cameraAngleOrEdgeLengthSquared =
                                        xOffsetOrAngle * xOffsetOrAngle +
                                        zOffsetOrAngle * zOffsetOrAngle;
                                    directionAngleOrProjectionNumerator =
                                        -1.0f *
                                        (xOffsetOrAngle * (edges[edgeIndexOrContactGate].startX -
                                                           player->position.x) +
                                         zOffsetOrAngle * (edges[edgeIndexOrContactGate].startZ -
                                                           player->position.z));
                                    projectionScale = cameraAngleOrEdgeLengthSquared
                                                          ? directionAngleOrProjectionNumerator /
                                                                cameraAngleOrEdgeLengthSquared
                                                          : 0.0f;
                                    cameraDeltaOrBoundaryPoint.x =
                                        xOffsetOrAngle * projectionScale +
                                        edges[edgeIndexOrContactGate].startX;
                                    cameraDeltaOrBoundaryPoint.z =
                                        zOffsetOrAngle * projectionScale +
                                        edges[edgeIndexOrContactGate].startZ;
                                    xOffsetOrAngle =
                                        player->position.x - cameraDeltaOrBoundaryPoint.x;
                                    zOffsetOrAngle =
                                        player->position.z - cameraDeltaOrBoundaryPoint.z;
                                    contactRadiusOrDistance = sqrt(xOffsetOrAngle * xOffsetOrAngle +
                                                                   zOffsetOrAngle * zOffsetOrAngle);
                                    cameraUpOrBoundaryVector.x = xOffsetOrAngle;
                                    cameraUpOrBoundaryVector.y = 0.0f;
                                    cameraUpOrBoundaryVector.z = zOffsetOrAngle;
                                    if (cameraUpOrBoundaryVector.x == 0.0f &&
                                        cameraUpOrBoundaryVector.z == 0.0f) {
                                        normalAngle = 999.0f;
                                    } else {
                                        PSVECNormalize(&cameraUpOrBoundaryVector,
                                                       &cameraUpOrBoundaryVector);
                                        normalAngle = 180.0 * (atan2(cameraUpOrBoundaryVector.x,
                                                                     cameraUpOrBoundaryVector.z) /
                                                               3.141592653589793);
                                        edgeAngle =
                                            180.0 * (atan2(normals[edgeIndexOrContactGate].x,
                                                           normals[edgeIndexOrContactGate].z) /
                                                     3.141592653589793);
                                        if (edgeAngle == 180.0f && normalAngle < 0.0f) {
                                            edgeAngle = -180.0f;
                                        }
                                        normalAngle = normalAngle - edgeAngle > 0.0f
                                                          ? normalAngle - edgeAngle
                                                          : -1.0f * (normalAngle - edgeAngle);
                                    }
                                    cameraUpOrBoundaryVector.x =
                                        lbl_1_bss_8BC[modelIndex].previousPosition.x -
                                        lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.x;
                                    cameraUpOrBoundaryVector.y = 0.0f;
                                    cameraUpOrBoundaryVector.z =
                                        lbl_1_bss_8BC[modelIndex].previousPosition.z -
                                        lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.z;
                                    cameraTargetOrContactVector.x =
                                        (player->previousPosition.x - player->position.x) -
                                        cameraUpOrBoundaryVector.x;
                                    cameraTargetOrContactVector.y = 0.0f;
                                    cameraTargetOrContactVector.z =
                                        (player->previousPosition.z - player->position.z) -
                                        cameraUpOrBoundaryVector.z;
                                    if (cameraTargetOrContactVector.x == 0.0f &&
                                        cameraTargetOrContactVector.z == 0.0f) {
                                        movementAngle = 999.0f;
                                    } else {
                                        PSVECNormalize(&cameraTargetOrContactVector,
                                                       &cameraTargetOrContactVector);
                                        movementAngle =
                                            180.0 * (atan2(cameraTargetOrContactVector.x,
                                                           cameraTargetOrContactVector.z) /
                                                     3.141592653589793);
                                        edgeAngle =
                                            180.0 * (atan2(normals[edgeIndexOrContactGate].x,
                                                           normals[edgeIndexOrContactGate].z) /
                                                     3.141592653589793);
                                        if (edgeAngle == 180.0f && movementAngle < 0.0f) {
                                            edgeAngle = -180.0f;
                                        }
                                        movementAngle = movementAngle - edgeAngle > 0.0f
                                                            ? movementAngle - edgeAngle
                                                            : -1.0f * (movementAngle - edgeAngle);
                                    }
                                    if (contactRadiusOrDistance < boundaryRadius &&
                                        normalAngle < 10.0f && movementAngle < 90.0f) {
                                        angleDifferenceOrScale =
                                            contactRadiusOrDistance
                                                ? boundaryRadius / contactRadiusOrDistance
                                                : 0.0f;
                                        xOffsetOrAngle *= angleDifferenceOrScale;
                                        zOffsetOrAngle *= angleDifferenceOrScale;
                                        player->position.x =
                                            cameraDeltaOrBoundaryPoint.x + xOffsetOrAngle;
                                        player->position.z =
                                            cameraDeltaOrBoundaryPoint.z + zOffsetOrAngle;
                                        positionCorrected = 1;
                                    }
                                    if (positionCorrected != 0) {
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            } else if (player->phase == 3) {
                switch (player->fallState) {
                case 0:
                    MgPlayerDespawn(lbl_1_bss_4.players[player->playerNo]);
                    player->movementFlag = 0;
                    player->fallState++;
                    break;
                case 1:
                    player->position.y -= 10.0f;
                    if (player->position.y < -10.0f && player->position.z < 200.0f) {
                        player->phase = 1;
                    } else if (player->position.y < -50.0f && player->position.z < 300.0f) {
                        player->phase = 1;
                    } else if (player->position.z > 400.0f) {
                        player->phase = 4;
                    } else if (player->position.y < -50.0f) {
                        player->phase = 4;
                    }
                    if (player->phase != 3) {
                        MgPlayerSpawn(lbl_1_bss_4.players[player->playerNo], &player->position);
                    }
                    break;
                }
            }
            if (player->phase != 2) {
                if (player->position.x > 560.0f) {
                    player->position.x = 560.0f;
                }
                if (player->position.x < -560.0f) {
                    player->position.x = -560.0f;
                }
            }
            if (player->phase != 2 && player->mode != 1 && player->position.z < -1000.0f) {
                player->position.z = -1000.0f;
            }
        }
        /* Phase 4 adds forward velocity to Z and half that same value to Y. */
        if (player->phase == 4) {
            player->position.y += player->velocity.z / 2.0f;
            player->position.z += player->velocity.z;
        }
        switch (player->boundaryMode) {
        case 0:
            if (lbl_1_bss_8BC[50].position.z > player->position.z) {
                player->position.z = lbl_1_bss_8BC[50].position.z;
            }
            break;
        case 1:
            /* Lineup boundary mode also forces phase back to zero
             * after the hazard and fall checks above. */
            if (35.0f + (lbl_1_bss_8BC[50].position.z - lbl_1_bss_8BC[50].bounds.z) >
                player->position.z) {
                player->position.z =
                    35.0f + (lbl_1_bss_8BC[50].position.z - lbl_1_bss_8BC[50].bounds.z);
            } else if ((lbl_1_bss_8BC[50].position.z + lbl_1_bss_8BC[50].bounds.z) - 35.0f <
                       player->position.z) {
                player->position.z =
                    (lbl_1_bss_8BC[50].position.z + lbl_1_bss_8BC[50].bounds.z) - 35.0f;
            }
            if (35.0f + (lbl_1_bss_8BC[50].position.x - lbl_1_bss_8BC[50].bounds.x) >
                player->position.x) {
                player->position.x =
                    35.0f + (lbl_1_bss_8BC[50].position.x - lbl_1_bss_8BC[50].bounds.x);
            } else if ((lbl_1_bss_8BC[50].position.x + lbl_1_bss_8BC[50].bounds.x) - 35.0f <
                       player->position.x) {
                player->position.x =
                    (lbl_1_bss_8BC[50].position.x + lbl_1_bss_8BC[50].bounds.x) - 35.0f;
            }
            player->position.y = 1.0f + lbl_1_bss_8BC[50].position.y;
            player->phase = 0;
            break;
        }
    }
    /* Both moving-rim loops below test every player regardless of motionFlag or phase;
     * correction still requires proximity to the animated rim. */
    for (modelIndex = 46; modelIndex <= 59; modelIndex++) {
        if (Hu3DMotionTimeGet(lbl_1_bss_8BC[modelIndex].model) > 0.0f) {
            cameraDeltaOrBoundaryPoint.x = lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.x;
            cameraDeltaOrBoundaryPoint.y =
                lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.y +
                lbl_1_bss_8BC[modelIndex].bounds.z *
                    sin((3.141592653589793 * lbl_1_bss_8BC[modelIndex].collisionRotation.x) /
                        180.0);
            cameraDeltaOrBoundaryPoint.z =
                lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.z -
                lbl_1_bss_8BC[modelIndex].bounds.z *
                    cos((3.141592653589793 * lbl_1_bss_8BC[modelIndex].collisionRotation.x) /
                        180.0);
            for (playerOrSegmentIndex = 0; playerOrSegmentIndex < 4; playerOrSegmentIndex++) {
                player = &lbl_1_bss_24C[playerOrSegmentIndex];
                if (cameraDeltaOrBoundaryPoint.x + lbl_1_bss_8BC[modelIndex].bounds.x >=
                        player->position.x &&
                    cameraDeltaOrBoundaryPoint.x - lbl_1_bss_8BC[modelIndex].bounds.x <=
                        player->position.x &&
                    player->collisionMesh < 0) {
                    delta.y = cameraDeltaOrBoundaryPoint.y - player->position.y;
                    delta.z = cameraDeltaOrBoundaryPoint.z - player->position.z;
                    distanceOrScale = sqrt(delta.y * delta.y + delta.z * delta.z);
                    contactRadiusOrDistance = 50.0f;
                    if (distanceOrScale < contactRadiusOrDistance) {
                        angleDifferenceOrScale = contactRadiusOrDistance / distanceOrScale;
                        delta.y *= angleDifferenceOrScale;
                        delta.z *= angleDifferenceOrScale;
                        player->position.y = cameraDeltaOrBoundaryPoint.y - delta.y;
                        player->position.z = cameraDeltaOrBoundaryPoint.z - delta.z;
                    }
                }
            }
        }
    }
    /* Models 76/77 force the contact gate negative and can correct players on a mesh;
     * models 78/79 require the player's collisionMesh to be negative. */
    for (modelIndex = 76; modelIndex <= 79; modelIndex++) {
        if (Hu3DMotionTimeGet(lbl_1_bss_8BC[modelIndex].model) > 0.0f) {
            if (modelIndex == 76 || modelIndex == 77) {
                xOffsetOrAngle = 400.0f;
                zOffsetOrAngle = 200.0f;
                edgeIndexOrContactGate = -1;
            } else if (modelIndex == 78 || modelIndex == 79) {
                xOffsetOrAngle = 100.0f;
                zOffsetOrAngle = 180.0f;
            }
            cameraDeltaOrBoundaryPoint.x = lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.x;
            cameraDeltaOrBoundaryPoint.y =
                lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.y +
                zOffsetOrAngle *
                    sin((3.141592653589793 * lbl_1_bss_8BC[modelIndex].collisionRotation.x) /
                        180.0);
            cameraDeltaOrBoundaryPoint.z =
                lbl_1_bss_8BC[modelIndex].jointPositionOrDriftOffset.z -
                zOffsetOrAngle *
                    cos((3.141592653589793 * lbl_1_bss_8BC[modelIndex].collisionRotation.x) /
                        180.0);
            for (playerOrSegmentIndex = 0; playerOrSegmentIndex < 4; playerOrSegmentIndex++) {
                player = &lbl_1_bss_24C[playerOrSegmentIndex];
                if (modelIndex == 78 || modelIndex == 79) {
                    edgeIndexOrContactGate = player->collisionMesh;
                }
                if (cameraDeltaOrBoundaryPoint.x + xOffsetOrAngle >= player->position.x &&
                    cameraDeltaOrBoundaryPoint.x - xOffsetOrAngle <= player->position.x &&
                    edgeIndexOrContactGate < 0) {
                    delta.y = cameraDeltaOrBoundaryPoint.y - player->position.y;
                    delta.z = cameraDeltaOrBoundaryPoint.z - player->position.z;
                    distanceOrScale = sqrt(delta.y * delta.y + delta.z * delta.z);
                    contactRadiusOrDistance = 50.0f;
                    if (distanceOrScale < contactRadiusOrDistance) {
                        angleDifferenceOrScale = contactRadiusOrDistance / distanceOrScale;
                        delta.y *= angleDifferenceOrScale;
                        delta.z *= angleDifferenceOrScale;
                        player->position.y = cameraDeltaOrBoundaryPoint.y - delta.y;
                        player->position.z = cameraDeltaOrBoundaryPoint.z - delta.z;
                    }
                }
            }
        }
    }
    for (playerOrSegmentIndex = 0; playerOrSegmentIndex < 4; playerOrSegmentIndex++) {
        player = &lbl_1_bss_24C[playerOrSegmentIndex];
        if (player->movementFlag == 0) {
            player->previousPosition = player->position;
        }
    }
    for (playerOrSegmentIndex = 46; playerOrSegmentIndex <= 79; playerOrSegmentIndex++) {
        lbl_1_bss_8BC[playerOrSegmentIndex].previousPosition =
            lbl_1_bss_8BC[playerOrSegmentIndex].jointPositionOrDriftOffset;
    }
}

/* First-contact initialization flags indexed by player number. */
s16 lbl_1_data_37E[5] = {0,0,0,0,0};

/* Records player contact pairs. Each contact increments the invoking player's counter when
 * its saved linkState is nonnegative, even if the partner changed. The initial zero also
 * passes this test before any previous player contact has been recorded. */
int fn_1_C9A8(COL_NARROW_PARAM *contact, COL_NARROW_PARAM *otherContact)
{
    switch (otherContact->type) {
    case 0:
        lbl_1_bss_24C[contact->paramB].linkedPlayer = (s8)otherContact->paramB;
        lbl_1_bss_24C[otherContact->paramB].linkedPlayer = (s8)contact->paramB;
        if (lbl_1_bss_24C[contact->paramB].linkState >= 0) {
            lbl_1_bss_24C[contact->paramB].linkCount += 1;
        } else {
            lbl_1_bss_24C[contact->paramB].linkCount = 0;
        }
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        break;
    }
    return 0;
}

/* Actor correction hook: updates the contacted segment, redirects edge contacts, and starts effects
 * when the contact changes. */
void fn_1_CA6C(MGACTOR *actor, int hookParameter)
{
    s32 playerNo;
    s16 contactEffectIndex;
    s16 nextSegment;
    s16 contactRoute;
    s16 routeVariant;
    s16 contactSegment;
    s16 collisionModelIndex;

    playerNo = actor->param;
    contactRoute = 0;
    /* Seed the previous contact only on this player's first correction callback. */
    if (lbl_1_data_37E[playerNo] == 0) {
        lbl_1_bss_24C[playerNo].collisionMesh =
            lbl_1_bss_24C[playerNo].previousCollisionMesh = 0;
        lbl_1_data_37E[playerNo] = 1;
    }
    lbl_1_bss_24C[playerNo].collisionMesh = (s8)actor->colMesh;
    contactSegment = lbl_1_bss_8BC[actor->colMesh + 32].linkedRecord;
    collisionModelIndex = actor->colMesh + 32;
    if (actor->colMesh == 8 || actor->colMesh == 9 ||
        actor->colMesh == 10 || actor->colMesh == 11) {
        switch (contactSegment) {
        case 60:
        case 61:
        case 64:
        case 65:
            if (lbl_1_bss_24C[playerNo].position.x > 400.0f ||
                lbl_1_bss_24C[playerNo].position.x < -400.0f) {
                lbl_1_bss_24C[playerNo].phase = 3;
                contactRoute = 2;
            } else {
                lbl_1_bss_24C[playerNo].direction = 1;
            }
            break;
        case 68:
        case 69:
        case 72:
        case 73:
            if (lbl_1_bss_24C[playerNo].position.x > 400.0f ||
                lbl_1_bss_24C[playerNo].position.x < -400.0f) {
                lbl_1_bss_24C[playerNo].phase = 3;
                contactRoute = 2;
            } else {
                lbl_1_bss_24C[playerNo].direction = -1;
            }
            break;
        case 70:
        case 71:
        case 74:
        case 75:
            if (lbl_1_bss_24C[playerNo].position.x < 50.0f &&
                lbl_1_bss_24C[playerNo].position.x > -50.0f) {
                lbl_1_bss_24C[playerNo].phase = 3;
                contactRoute = 2;
            } else if (lbl_1_bss_24C[playerNo].position.x > 0.0f) {
                lbl_1_bss_24C[playerNo].direction = -1;
                nextSegment = contactSegment;
                switch (nextSegment) {
                case 70: routeVariant = 0; break;
                case 74: routeVariant = 1; break;
                case 71: routeVariant = 2; break;
                case 75: routeVariant = 3; break;
                }
                contactRoute = 1;
            } else {
                lbl_1_bss_24C[playerNo].direction = 1;
                switch (contactSegment) {
                case 70: nextSegment = 62; routeVariant = 0; break;
                case 74: nextSegment = 66; routeVariant = 1; break;
                case 71: nextSegment = 63; routeVariant = 2; break;
                case 75: nextSegment = 67; routeVariant = 3; break;
                }
                contactRoute = 1;
            }
            break;
        }
    }
    if (contactRoute == 0) {
        lbl_1_bss_24C[playerNo].segmentModel = contactSegment;
    } else if (contactRoute == 1) {
        lbl_1_bss_24C[playerNo].segmentModel = nextSegment;
    }
    if (lbl_1_bss_24C[playerNo].previousCollisionMesh !=
        lbl_1_bss_24C[playerNo].collisionMesh) {
        if (contactRoute == 0) {
            contactEffectIndex = lbl_1_bss_8BC[collisionModelIndex].playerModels[playerNo];
            lbl_1_bss_8BC[contactSegment].motionVerticalSpeed = -1.0f;
            if (lbl_1_bss_8BC[contactEffectIndex].stateOrTimer == 0 &&
                lbl_1_bss_8BC[collisionModelIndex].rotation.x <= 0.2f) {
                Hu3DModelAttrReset(lbl_1_bss_8BC[contactEffectIndex].model, HU3D_ATTR_DISPOFF);
                Hu3DMotionTimeSet(lbl_1_bss_8BC[contactEffectIndex].model, 0.0f);
                Hu3DMotionSpeedSet(lbl_1_bss_8BC[contactEffectIndex].model, 1.0f);
                lbl_1_bss_8BC[contactEffectIndex].stateOrTimer = 1;
                lbl_1_bss_8BC[contactEffectIndex].linkedRecord = contactSegment;
                lbl_1_bss_8BC[contactEffectIndex].position =
                    lbl_1_bss_8BC[collisionModelIndex].position;
            }
        } else if (contactRoute == 1) {
            contactEffectIndex = lbl_1_bss_30.models[routeVariant][playerNo];
            lbl_1_bss_8BC[nextSegment].motionVerticalSpeed = -1.0f;
            if (lbl_1_bss_8BC[contactEffectIndex].stateOrTimer == 0 &&
                Hu3DMotionTimeGet(lbl_1_bss_8BC[nextSegment].model) == 0.0f) {
                Hu3DModelAttrReset(lbl_1_bss_8BC[contactEffectIndex].model, HU3D_ATTR_DISPOFF);
                Hu3DMotionTimeSet(lbl_1_bss_8BC[contactEffectIndex].model, 0.0f);
                Hu3DMotionSpeedSet(lbl_1_bss_8BC[contactEffectIndex].model, 1.0f);
                lbl_1_bss_8BC[contactEffectIndex].stateOrTimer = 1;
                lbl_1_bss_8BC[contactEffectIndex].linkedRecord = nextSegment;
                lbl_1_bss_8BC[contactEffectIndex].position = lbl_1_bss_8BC[nextSegment].position;
            }
        }
    }
}
