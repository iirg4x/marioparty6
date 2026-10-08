/* Defines the Body Builder scene, player, team, and piece data shared by the REL code. */
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
#ifndef M640_H
#define M640_H

typedef struct M640Scene {
    s16 state; /* Current opening or result state-machine step. */
    s16 frame; /* Frame counter used by the active sequence callback. */
    float cameraProgress; /* Split-camera transition progress; the opening finishes above 240. */
    s16 finishCount; /* Number of sides counted by the nonzero fourth-bucket check; piece-record
                      * index 0 is missed. */
    s16 winnerSide; /* Side selected for the result when one side finishes: 0 or 1. */
    s16 uniformPartSet; /* Set when all four winning pieces use the same part group. */
    s32 stream; /* Stream handle faded on result entry; this scene leaves the zero-initialized value
                 * unchanged. */
} M640Scene;
typedef struct M640Player {
    s16 characterModel; /* Character model shown in this player's camera view. */
    s16 characterMotions[5]; /* Character-motion handles from lbl_1_data_44: idle (0), winning
                              * sequence (1-2), no-winner result (3), and selection gesture (4). */
    s16 playerNo; /* Game-wide player number, 0 through 3. */
    s16 charNo; /* Character roster number used by the character-motion system. */
    s16 side; /* Side assigned to this player: 0 or 1. */
    s16 slot; /* Player position on the side: 0 or 1. */
    s16 padNo; /* Controller index used for human input. */
    s16 comDif; /* CPU difficulty, or -1 for a human player. */
    s16 delay; /* Remaining frames before the CPU chooses a bucket. */
    s16 error; /* Nonzero when the CPU turn is scheduled to miss. */
    s16 selectionFrameCount; /* Frames counted after selection input while waiting for the character
                              * timing hook; this count is not otherwise read. */
    s16 turnActionModel; /* Model playing the turn and selection action. */
    s16 smokeEffectModel; /* Smoke effect attached beside this player's slot marker. */
    s32 selectionSoundHandle; /* Sound handle for the active selection cue, or -1. */
    s32 turnLoopSoundHandle; /* Sound handle for the looping turn cue, or -1. */
    s16 spinnerModel; /* Rotating slot marker used to show the bucket selection. */
    float speed; /* Spinner rotation speed in degrees per frame. */
    s16 state; /* Player turn step: 0 idle, 1 ready, 2 spinning, 3 waiting for selection, 4 waiting
                * for the character timing hook, 5 playing the selection action, 6 aligning the
                * marker and waiting for the piece. */
    s16 unreadPlayerData; /* Stored player value with no use in this game flow. */
    s16 selectedBucket; /* Bucket index chosen by this player, 0 through 3. */
    s16 targetAngle; /* Target slot-marker angle in degrees, snapped to 30-degree steps. */
} M640Player;
typedef struct M640Team {
    s16 boardHookAnchorModel; /* Board model that holds the player and effect hooks. */
    s16 sidePlatformModel; /* Side-specific platform model. */
    s16 sideBackdropModel; /* Side-specific backdrop model. */
    s16 introAnimationModel; /* Side model animated during the opening demonstration. */
    s16 pieceHookModel; /* Side model that carries the falling-piece hooks. */
    s16 sideAccentModel; /* Decorative side model. */
    s16 hookEffectModels[4]; /* Effects attached to the four piece hooks, indexed by part. */
    s16 bucketEffectModels[4]; /* Landing effects at the four bucket heights. */
    s16 missedPieceEffectModel; /* Effect model played when a piece misses its target bucket. */
    M640Player *players[2]; /* Players assigned to this side, in turn order. */
    s16 bucketPieces[4]; /* Piece-record index placed in each bucket, in fill order. */
    s16 activePieceIndex; /* Piece record being handled for this side, or -1 when none is active. */
    s16 nextBucket; /* Next bucket position to fill, from 0 through 3. */
    u8 unreadTeamTail[4]; /* Stored team bytes not read or written by this game flow. */
} M640Team;
typedef struct M640Piece {
    u8 unreadPieceStorage[12]; /* Preserved bytes with no observed use in this game flow. */
    float speed; /* Signed vertical velocity in model units per frame. */
    s16 pieceModel; /* Model displaying this piece. */
    s16 resultMotions[2]; /* Mixed-set and matching-part celebration motions, respectively. */
    s16 placementMotions[4]; /* Motions used when placing into buckets 0 through 3. */
    s16 part; /* Piece part group, 0 through 3, also selecting its hook and landing height. */
    s16 state; /* Piece step: 0 idle, 1-3 bucket drop, 4 placement motion, 5-7 intro drop and
                * hook. */
    s16 variant; /* Visual copy selected from the three copies of this part group. */
    s16 frame; /* Frames elapsed in the bucket-drop delay. */
} M640Piece;
typedef struct M640MotionEntry {
	s32 motionFile; /* Character motion data file for this sequence slot. */
	u32 motionAttributes; /* Motion playback attributes, including looping. */
} M640MotionEntry;
extern M640MotionEntry lbl_1_data_44[5];
extern M640Piece lbl_1_bss_C[12][2];
extern M640Player lbl_1_bss_3F8[4];
extern M640Team lbl_1_bss_4FC[2];
extern u32 lbl_1_data_84[12][2];
extern u32 lbl_1_data_E4[12][4];
extern s32 lbl_1_data_7C[2];
extern s16 lbl_1_bss_3E4[5];
extern s16 lbl_1_bss_3CC[3][4];
extern OMOBJ *lbl_1_bss_3F0;
extern OMOBJ *lbl_1_bss_3F4;
extern void *lbl_1_bss_8;
extern M640Scene lbl_1_bss_4E8;
extern s32 lbl_1_bss_0;
extern HUPROCESS *lbl_1_bss_4;
extern MGSEQ_PARAM lbl_1_data_0;
void fn_1_270(s16 frame);
void fn_1_60C(void);
void fn_1_868(s16 frame);
void fn_1_86C(void);
void fn_1_351C(s16 side, s16 slot);
void fn_1_4CD8(void);
void fn_1_3090(void);
void fn_1_355C(void);
s16 fn_1_112C(s16 frame);
s32 fn_1_1C48(s32 frame);
void fn_1_8D4(void);
void fn_1_15C8(void);
void fn_1_2820(void);
void fn_1_5A28(void);
void fn_1_5DF0(void);
s32 fn_1_BEC(void);
s16 fn_1_EC8(void);
void fn_1_26B8(s16 playerNo, s16 motionSlot);
void fn_1_3C24(s16 side, s16 slot);
void fn_1_52D0(OMOBJ *obj);
void fn_1_60AC(OMOBJ *obj);
void fn_1_36D8(M640Player *player);
s16 fn_1_37F4(s16 side, M640Player *player);
u16 fn_1_39F0(s16 side, M640Player *player);
s16 fn_1_33D0(float rotationDegrees);

#endif
