/* Shared state and interfaces for the M632 minigame. */
#ifndef M632DLL_H
#define M632DLL_H

#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "datadir_enum.h"
#include "string.h"
#include "math.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"
#include <stddef.h>

#define M632_ARENA_COLLISION_SE_ID 1846 /* Sound played when arena collision checks report a hit. */
#define M632_PLAYER_ELIMINATION_SE_ID 1847 /* Sound played when an arena collision knocks out a
                                            * player. */
#define M632_OPENING_SEQUENCE_CUE_SE_ID 1848 /* Sound played at frame 98 of the opening sequence. */
#define M632_PLAY_MODE_BGM_STREAM_ID 73 /* Background music stream started by the sequence callback
                                         * when allowed. */
#define M632_INTRO_CHARACTER_SE_ID 581 /* Character effect played at setup and used for opening
                                        * voice selection. */
#define M632_GROUP1_INTRO_SE_ID 576 /* Character effect played for group 1 at opening frame 112. */

#define fn_1_7364 __cvt_fp2unsigned
#define fn_1_75B8 __div2u
#define fn_1_76A4 __div2i
#define fn_1_77DC __mod2u
#define fn_1_78C0 __mod2i
#define fn_1_79CC __shl2i
#define fn_1_79F0 __shr2u
#define fn_1_7A14 __shr2i
#define fn_1_7A3C __cvt_sll_dbl
#define fn_1_7AEC __cvt_ull_dbl
#define fn_1_7B88 __cvt_sll_flt
#define fn_1_7C3C __cvt_ull_flt
#define fn_1_7CDC __cvt_dbl_usll
/* State shared by the sequence callbacks and arena logic. */
typedef struct M632State {
    HUPROCESS *objectManager;
    s16 phase; /* Opening or result sequence phase, values 1 through 5. */
    s32 frame; /* Frame counter for the current sequence phase. */
    s16 cameraModels[5]; /* Camera models for the intro and round endings. */
    s16 cameraMotions[5]; /* Camera motions selected for each sequence scene. */
    s16 motions020[2];
    s16 playerModels[4];
    s16 modelId;
    s16 sceneModels[3];
    s16 attachmentModels[3];
    u8 unknown03A[2];
    s16 collisionModel;
    u8 unknown03E[22];
    MGACTOR *collisionActors[10]; /* Arena actors checked during collision and map updates. */
    s32 collisionCount; /* Number of active arena collision actors. */
    s16 linkedModels[10]; /* Models linked to the arena scene. */
    s32 linkedModelCount; /* Number of linked arena models. */
    f32 collisionRotY[10]; /* Target Y rotation for each collision actor, in degrees. */
    s16 hookModelId;
    MGPLAYER *players[4]; /* Player objects indexed by player slot. */
    s32 charNo[4]; /* Character number for each player slot. */
    s32 padNo[4]; /* Controller number assigned to each player slot. */
    s32 group[4]; /* Group assignment used by the round and result sequences. */
    s32 playerState[4]; /* Per-player animation and elimination state. */
    Point3d playerMotionVec[4]; /* Direction used while a defeated player moves through the air. */
    s32 playerTimer[4]; /* CPU decision countdown, updated once per frame. */
    s32 playerAIState[4]; /* CPU movement mode: 0 idle, 1 free move, 2 grid pursuit. */
    Point3d playerDirection[4]; /* X/Z direction selected by CPU movement. */
    f32 cpuStickX, cpuStickZ; /* CPU-generated X/Z stick values used by group 0. */
    s32 patternIndex; /* Selected arena layout, index 0 through 3. */
    Point3d positions[4]; /* Starting positions assigned during the intro. */
    f32 targetTiltX; /* Target tilt.x derived from the player's stick input. */
    u8 unknown1D4[4];
    f32 targetTiltZ; /* Target tilt.z derived from the player's stick input. */
    Point3d tilt; /* Current arena tilt in degrees; X/Z approach their targets each update. */
    MGTIMER *timer;
    s32 completion; /* 0 when all active group-1 players are eliminated; 1 when the timer
                     * expires. */
    int activeMask; /* One bit per player while that player is active in the current sequence
                     * phase. */
    s32 gridA[64]; /* Row-major 8-by-8 risk map after player proximity is added, values 0..255. */
    s32 gridB[64]; /* Row-major 8-by-8 obstacle risk map, values 0..255. */
    /* Stream handle (-1 means none), collision flags, and contact-sound cooldown in frames. */
    s32 stream, previousCollisionFlag, collisionFlag, soundCooldownFrames;
    s32 collisionPairs[10][2]; /* Previous and current ground-contact flags for each actor. */
    s32 collisionTimers[10]; /* Frames since each actor's last contact sound. */
} M632State;

/* A board cell ranked by its distance-weighted risk. */
typedef struct M632GridCandidate {
    s32 x, z; /* Cell coordinates from 0 through 7. */
    f32 distance; /* Euclidean distance in grid cells. */
    s32 cost; /* Cell risk multiplied by distance. */
} M632GridCandidate;

/* Sorting and floating-point support routines used by the minigame. */
void fn_1_6574(void *, size_t, size_t, int (*)(const void *, const void *));
u32 fn_1_7364(f64);

extern MGSEQ_PARAM lbl_1_data_0;
/* Character model hook names followed during the opening sequence. */
extern char *lbl_1_data_108[4];
/* Collision model data numbers for the four arena layouts. */
extern unsigned int lbl_1_data_118[4];
/* Four 3D points at Y=1200. */
extern Point3d lbl_1_data_128[4];
/* Character animation data numbers loaded during player setup. */
extern unsigned int lbl_1_data_158[16];
/* Group-1 CPU risk thresholds indexed by difficulty. */
extern s32 lbl_1_data_198[4];
/* Cell patterns used to build each arena layout's obstacle map. */
extern u8 *lbl_1_data_2A8[4];
extern M632State lbl_1_bss_0;
s32 fn_1_A0(s32 streamHandle, s32 streamId);
void fn_1_104(s32 streamHandle);
void fn_1_140(s32 playerIndex, s32 cameraId);
void fn_1_21C(s16 mode, s16 frameNo);
void fn_1_240(OMOBJ *obj);
void fn_1_3E0(OMOBJ *obj);
void fn_1_5A8(s16 mode, s16 frameNo);
void fn_1_173C(s16 mode, s16 frameNo);
void fn_1_17C0(s16 mode, s16 frameNo);
void fn_1_2368(s16 mode, s16 frameNo);
void fn_1_2574(s16 mode, s16 frameNo);
void fn_1_2E04(s16 mode, s16 frameNo);
void fn_1_2F88(s16 mode, s16 frameNo);
void fn_1_2F8C(s16 mode, s16 frameNo);
void fn_1_2F90(void);
void fn_1_3510(MGACTOR *actorP, int param);
int fn_1_353C(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b);
void fn_1_3934(void);
void fn_1_4C8C(HU3D_MODEL *modelP, Mtx *mtx);
void fn_1_4C90(void);
void fn_1_4D48(void);
void fn_1_50A0(s32 playerIndex);
void fn_1_535C(void);
void fn_1_5400(s32 playerIndex);
void fn_1_597C(s32 playerIndex);
void fn_1_5D34(s32 playerIndex);
int fn_1_5FE4(const void *candidateA, const void *candidateB);
s32 fn_1_604C(s32 playerIndex, s32 startX, s32 startZ, s32 *selectedX, s32 *selectedZ);

#endif
