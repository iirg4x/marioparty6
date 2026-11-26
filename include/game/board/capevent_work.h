#ifndef _BOARD_CAPEVENT_WORK_H
#define _BOARD_CAPEVENT_WORK_H

#include "game/object.h"
#include "game/sprite.h"

typedef struct EvCapWork EVCAPWORK;
typedef struct CapWork CAPWORK;

/* Sprite animation and model object used by the capsule particle effects. */
typedef struct CapEffWork {
    int modelId;
    int num;
    int objIdx;
    ANIMDATA *animP;
} CAPEFFWORK;

#define CAP_WORK_MAX 64

/* Resources and positions tracked for the lifetime of a capsule event. */
struct EvCapWork {
    int motId[CAP_WORK_MAX][4];
    int objId[CAP_WORK_MAX];
    int sprId[CAP_WORK_MAX];
    void *mem[CAP_WORK_MAX];
    int masuId[CAP_WORK_MAX];
    HuVecF objPos[CAP_WORK_MAX];
    int playerMasuId[4];
    HuVecF playerPos[4];
    int bgId;
    OMOBJ *obj;
};

/* Motion and award count for the coin and star characters. */
typedef struct CapAwardManWork {
    int objIdx;
    int activeF;
    int screenSpaceF;
    int modelId;
    int playerNo;
    int count;
    int time;
    int timeMax;
    float arcHeight;
    HuVecF pos;
    HuVecF targetPos;
} CAPAWARDMANWORK;

/* Flags used by capsule events and the minigames they launch. */
typedef struct CapWorkFlag {
    u8 _flag00 : 1;
    u8 kettouStopF : 1;
    u8 kettouMinigameReturnF : 1;
    u8 donkeyMinigameReturnF : 1;
    u8 koopaMinigameReturnF : 1;
    u8 koopaNoCoinF : 1;
    u8 _flag06 : 1; /* The game's code never reads this field. */
    u8 _flag07 : 1;
    u8 _flag08 : 1; /* The game's code never reads this field. */
    u8 _flag09 : 1; /* The game's code never reads this field. */
    u8 _flag10 : 1; /* The game's code never reads this field. */
    u8 _flag11 : 1; /* The game's code never reads this field. */
    u8 _flag12 : 1; /* The game's code never reads this field. */
    u8 _flag13 : 1; /* The game's code never reads this field. */
    u8 _flag14 : 1; /* The game's code never reads this field. */
    u8 _flag15 : 1; /* The game's code never reads this field. */
    u8 _flag16 : 1; /* The game's code never reads this field. */
    u8 _flag17 : 1; /* The game's code never reads this field. */
    u8 _flag18 : 1; /* The game's code never reads this field. */
    u8 _flag19 : 1; /* The game's code never reads this field. */
    u8 _flag20 : 1; /* The game's code never reads this field. */
    u8 _flag21 : 1; /* The game's code never reads this field. */
    u8 _flag22 : 1; /* The game's code never reads this field. */
    u8 _flag23 : 1; /* The game's code never reads this field. */
    u8 _flag24 : 1; /* The game's code never reads this field. */
    u8 _flag25 : 1; /* The game's code never reads this field. */
    u8 _flag26 : 1; /* The game's code never reads this field. */
    u8 _flag27 : 1; /* The game's code never reads this field. */
    u8 _flag28 : 1; /* The game's code never reads this field. */
    u8 _flag29 : 1; /* The game's code never reads this field. */
    u8 _flag30 : 1; /* The game's code never reads this field. */
    u8 _flag31 : 1; /* The game's code never reads this field. */
} CAPWORKFLAG;

/* Shared capsule event state, including resource and effect ownership. */
struct CapWork {
    int playerNo; /* Player who triggered the capsule. */
    int targetPlayerNo; /* Selected player affected by the event, when applicable. */
    int capsuleNo; /* Capsule being run. */
    int masuId; /* Space where the capsule event started. */
    int masuIdNext; /* Next space tracked by capsule movement. */
    int moveF; /* Whether this event was called during movement. */
    int stopF; /* Whether this event was called when movement stopped. */
    int trapF; /* Whether this event is a trap. */
    EVCAPWORK objWork;
    CAPWORKFLAG flags;
    int eventData[6]; /* Per-event object IDs, counts, or callback results. */
    u8 unkB84[40]; /* No code names this field. */
    OMOBJ *guideObj; /* Guide character object used by capsule scenes. */
    u8 unkBB0[28]; /* No code names this field. */
    int processNo; /* Process ID for the running capsule event. */
    OMOBJ *explodeObj; /* Explosion effect object, when the event uses one. */
    OMOBJ *boostObj; /* Boost effect object, when the event uses one. */
    OMOBJ *snowObj; /* Snow effect object, when the event uses one. */
    OMOBJ *glowObj; /* Glow effect object, when the event uses one. */
    OMOBJ *ringObj; /* Ring effect object, when the event uses one. */
    OMOBJ *coinObj; /* Coin effect object, when the event uses one. */
    OMOBJ *coinManObj; /* Coin character object, when the event uses one. */
    OMOBJ *starManObj; /* Star character object, when the event uses one. */
    OMOBJ *capLoseObj; /* Capsule-loss object, when the event uses one. */
};

#endif
