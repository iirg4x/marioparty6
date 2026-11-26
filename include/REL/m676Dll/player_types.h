/* Player collision registrations, movement and character animation work. */
#ifndef REL_M676DLL_PLAYER_TYPES_H
#define REL_M676DLL_PLAYER_TYPES_H
#include "REL/minigame_player_motion.h"

#include "REL/m676Dll/actor_types.h"

typedef struct M676PlayerConfig {
    /* Player movement and character-motion settings copied from the setup template. */
    f32 unk00;
    f32 unk04;
    f32 movementScale;
    f32 unk0C; /* No code names this field. */
    f32 gravity;
    s32 idleMotion;
    s32 moveMotion;
    f32 moveSpeed;
    s32 runMotion;
    f32 runSpeed;
    s32 jumpMotion;
    s32 landingMotion;
    s32 unk30; /* No code names this field. */
    s32 winMotion;
    s32 loseMotion;
} M676PlayerConfig;

typedef struct M676PlayerWork {
    M676ActorState actor;
    s32 phase;
    s32 timer;
    /* No code names these bytes. */
    u8 unkB4, unkB5, unkB6, unkB7;
    /* Desired horizontal heading in degrees and its world-space destination. */
    f32 targetAngle;
    Vec targetPosition;
    /* No code names these bytes. */
    u8 unkC8, unkC9, unkCA, unkCB;
    /* Frames elapsed before this player signals arrival; the sequence uses a 30-frame threshold. */
    s32 arrivalTimer;
    /* No code names these bytes. */
    u8 unkD0, unkD1, unkD2, unkD3, unkD4, unkD5;
    /* No code names these bytes. */
    u8 unkD6, unkD7, unkD8, unkD9, unkDA, unkDB;
    u8 unkDC, unkDD, unkDE, unkDF;
    s32 frameCounter;
    s32 playerType;
    s32 difficulty;
    s32 aiState;
    s32 aiWait;
    s32 elapsed;
    /* Frames remaining before the next turn decision. */
    s32 turnCooldown;
    s32 selectionTimer;
    s32 targetPropIndex;
    /* Current wandering heading and its per-update change, in degrees. */
    f32 heading;
    f32 turnSpeed;
    f32 movementSpeed;
    s32 motionActive;
    s32 motionTimer;
    s32 turnState;
    /* No code names these bytes. */
    u8 unk11C, unk11D, unk11E, unk11F, unk120, unk121;
    u8 unk122, unk123;
    void *ownerLink;
    s32 cooldown;
    s32 unk12C; /* The game's code never reads this field. */
    s32 unk130; /* The game's code never reads this field. */
    f32 motionScale;
    /* No code names these bytes. */
    u8 unk138, unk139, unk13A, unk13B;
    s32 landingState;
    s32 landingTimer;
    s32 selectedPropIndex;
    s32 order;
    /* No code names these bytes. */
    u8 unk14C, unk14D, unk14E, unk14F;
    s32 unk150; /* The game's code never reads this field. */
    f32 unk154; /* The game's code never reads this field. */
    Vec *unk158; /* The game's code never reads this field. */
    Vec positionCopy;
    M676PlayerConfig config;
    s32 playerIndex;
    s32 character;
    s32 modelId;
    s32 callbackState;
    /* No code names these bytes. */
    u8 unk1B4, unk1B5, unk1B6, unk1B7;
    s32 unk1B8; /* The game's code never reads this field. */
    s32 unk1BC; /* The game's code never reads this field. */
} M676PlayerWork;

extern M676PlayerWork *lbl_1_bss_43C[5];
extern s32 lbl_1_bss_458[4];
extern OMOBJ *lbl_1_bss_468[4];

typedef struct M676WanderEntry {
    f32 selectionTimingFactor;
    s32 preferredRank;
} M676WanderEntry;

extern u32 lbl_1_data_130[8];
extern MinigamePlayerMotionParams lbl_1_data_150[8];
extern Vec lbl_1_data_1B0[4];
extern f32 lbl_1_data_1E0[4];
extern Vec lbl_1_data_1F0[4];
extern f32 lbl_1_data_220[7];
extern M676PlayerConfig lbl_1_data_23C;
extern M676WanderEntry lbl_1_data_2A4[4];
extern Vec lbl_1_data_2C4;
extern s32 lbl_1_bss_438;
extern s32 lbl_1_bss_450;
extern s32 lbl_1_bss_454;
extern OMOBJ *lbl_1_bss_478;

#endif
