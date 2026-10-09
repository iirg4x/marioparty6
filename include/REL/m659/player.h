/* Player, movement, and computer-control state for Asteroad Rage. */
#ifndef M659_PLAYER_H
#define M659_PLAYER_H
#include "REL/m659/collision.h"

typedef struct M659Motion {
    int state; /* 0: player control; nonzero: ship movement. */
    s16 column; /* Current lane, from 0 through 6. */
    s16 direction; /* Requested lane change: 0 left, 1 right, -1 none. */
    s16 previousDirection; /* Last requested direction, to detect a new press. */
    HuVecF start; /* Ship position in world units. */
    HuVecF rotation; /* Ship rotation in degrees. */
    HuVecF travelDirection; /* Normalized direction used to move the losing ship across the result
                             * scene. */
    s16 substate; /* Step within a movement or ending sequence. */
    int done; /* Set when the ship has collided. */
    HuVecF position; /* Collision position in world units. */
    HuVecF velocity; /* Collision velocity in world units per frame. */
    float phase; /* Phase of the idle vertical bob. */
} M659Motion;

typedef struct M659ComRoute {
    HuVecF position; /* Chosen lane position for this obstacle row. */
    s16 column; /* Chosen lane, from 0 through 6. */
    double distance; /* Distance from the preceding route point, in world units. */
    s16 delay; /* Frames before the computer player changes lanes. */
} M659ComRoute;

typedef struct M659ComRecord {
    int isCom; /* Nonzero when this player is controlled by the computer. */
    s16 difficulty; /* Computer difficulty, 0 through 3. */
    int active; /* Nonzero while a computer lane-change request is pending. */
    int state; /* Step in the computer player's lane-change request sequence. */
    unsigned char unreadComBytes[4]; /* Bytes with no established computer-control behavior in this
                                      * module. */
    s16 direction; /* Pending lane change: 0 left, 1 right, -1 none. */
    s16 routeIndex; /* Current obstacle row being followed. */
    M659ComRoute routes[22]; /* Planned lane for each obstacle row. */
} M659ComRecord;

typedef struct M659PlayerInfo {
    s16 cameraBit; /* Camera bit used to draw this player. */
    s16 playerNo; /* Game player number. */
    s16 side; /* Split-screen side and obstacle lane group. */
    s16 charNo; /* Selected character number. */
    s16 motionId; /* Current character animation index. */
    int initializationMarker; /* Set to one during character setup; no reader in this module tests
                               * it. */
    int state; /* Main player object state. */
    unsigned char unreadPlayerBytes[4]; /* Bytes with no established player-info behavior in this
                                         * module. */
    int result; /* Nonzero when this player has cleared the course. */
    HuVecF endingPosition; /* Initialized during winner ending setup; not read in this module. */
    HuVecF endingRotation; /* Initialized during winner ending setup; not read in this module. */
    int engineSoundHandle; /* Handle returned by the looping engine sound. */
} M659PlayerInfo;

typedef struct M659Player {
    M659PlayerInfo info; /* Character, result, and camera state for this player. */
    M659Motion motion; /* Ship lane, position, and movement state. */
    s16 padNo; /* Controller assigned to this player. */
    float stickX; /* Last horizontal stick input. */
    float stickY; /* Last vertical stick input. */
    unsigned char unreadPlayerDataBytes[8]; /* Bytes with no established player behavior in this
                                             * module. */
    M659ComRecord com; /* Computer route and lane-request state. */
    M659Collision collision; /* Collision data registered for the ship. */
} M659Player;

typedef struct M659ComSlot {
    OMOBJ *obj; /* Player object controlled by this slot. */
    int isCom; /* Nonzero when the slot belongs to a computer player. */
    int countdown; /* Frames remaining before its next lane request. */
} M659ComSlot;

extern OMOBJ *lbl_1_bss_2C[4];
int fn_1_1840(s16 side);
void fn_1_19DC(OMOBJ *obj, s16 motion);
void fn_1_2D0C(OMOBJ *obj);
void fn_1_2FE0(OMOBJ *obj);
void fn_1_5CB0(OMOBJ *obj);
int fn_1_5DB4(M659ComSlot *com);
void fn_1_5E08(M659ComSlot *com);
void fn_1_5E9C(M659ComSlot *com);
void fn_1_64B0(M659ComSlot *com);
void fn_1_6714(M659ComSlot *com, s16 direction);
#endif
