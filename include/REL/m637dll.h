/* Shared declarations for Slot Trot's minigame sequence and play code. */
#ifndef M637DLL_H
#define M637DLL_H

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

#include "game/esprite.h"

/* One of the two target-symbol displays on the central machine. */
typedef struct M637Record118 {
    s16 symbolModels[8][2]; /* Model IDs for each symbol's two animated faces. */
    f32 symbolSetupHeight[8]; /* Set to 950.0 during setup; not read during play. */
    f32 incomingAngle, outgoingAngle; /* Face-turn angles in degrees. */
    s32 targetSymbol; /* Newly selected target, in the range 0-7. */
    s32 settledSymbol; /* Previous target until the face-turn animation finishes. */
    s32 openingTargetSymbol; /* Target chosen before the current round starts; excluded from later
                              * stop selections. */
    s32 lastChosenSymbol; /* Excluded from the next random selection. */
    s32 initialSymbol; /* Symbol shared by both teams at scene setup. */
} M637Record118;

/* Reel positions and animated models for one two-player team. */
typedef struct M637Record1D0 {
    s16 reelModels[2][2]; /* Two alternating model IDs for each player's reel. */
    f32 modelAngle[2]; /* Display angle in degrees, wrapped over 240 degrees. */
    f32 reelSpeed[2]; /* Signed degrees per update, clamped to -4 through 4. */
    s32 reelSymbol[2]; /* Symbol nearest each reel's center, in the range 0-7. */
    s32 alignmentState[2]; /* 0: off center; 1: centered; 2: match sound played. */
    s32 visibleReelModel[2]; /* Index 0 or 1 into each reel's model pair. */
    f32 symbolAngle[2]; /* Position in degrees over the eight-symbol, 480-degree cycle. */
    s16 spinEffectModel; /* Model whose animation pauses when both reels stop. */
    s16 teamAnimationModel; /* Model using the team's idle, spin, and score motions. */
    s16 idleMotion, spinMotion, scoreMotion; /* Motion IDs for teamAnimationModel. */
    s32 spinAnimationActive; /* Set while either reel is moving. */
    s16 reelAnimationModels[2]; /* Animated model ID accompanying each player's reel. */
    s16 reelAnimationMotions[2][2]; /* Idle and spinning motion IDs for each reel. */
    s16 matchIndicatorModels[2]; /* Shown when the corresponding reel matches its target. */
    s16 matchIndicatorMotions[2]; /* Looping motion IDs for the match indicators. */
    s16 teamMatchEffectModel, teamMatchEffectMotion; /* Model and motion shown for a round score. */
} M637Record1D0;

/* Character models, reel-speed poses, and round scores in team order. */
typedef struct M637Record2A0 {
    HU3D_MODELID model; /* Character model ID. */
    HU3D_MOTIONID motion[9]; /* Idle, speed, round-result, win, and draw motion IDs. */
    HU3D_MOTIONID selectedMotion; /* Last pose selected from the player's reel speed. */
    s32 idleInputFrames; /* Updates since the player last pressed A or B. */
    s32 scoredRound; /* Nonzero when this player's team matched both targets. */
    s32 teamScore; /* Round points, duplicated for teammates; three ends the game, and equal totals
                    * draw. */
} M637Record2A0;

/* One team's score panel and its three earned-point markers. */
typedef struct M637RecordB8 {
    s16 scoreBox, sizeX, sizeY; /* Score-box handle and requested width/height in pixels; creation
                                 * rounds the sizes up. */
    f32 positionX, positionY, visiblePositionX; /* Current center and on-screen X in pixels. */
    s16 unearnedPointSprites[3]; /* Dim marker sprite handles before a point is earned. */
    s16 earnedPointSprites[3]; /* Marker sprite handles shown for earned points. */
} M637RecordB8;

u32 MgSeqModeNext(void);

/* Input plan for one computer player, indexed in team order. */
typedef struct M637Record08 {
    s32 difficulty; /* GwPlayerConf computer difficulty index. */
    s32 inputPlanActive; /* Nonzero while a planned sequence of presses is being executed. */
    s32 pendingButtonPresses; /* Presses left in the current input plan. */
    s32 remainingDetours; /* Difficulty-dependent number of reversed input plans still to take. */
    unsigned char unusedBytes[4]; /* Not accessed by the minigame. */
    s32 targetReelIndex; /* Index 0 or 1 into the central target displays. */
    s32 teamIndex; /* Team index 0 or 1. */
    s32 teamReelIndex; /* Player's reel within the team, 0 or 1. */
    s32 buttonDirection; /* Planned direction: 0 selects A, 1 selects B; centered corrections can
                          * emit the opposite. */
    s32 inputDelayFrames; /* Updates to wait before another button press. */
    s32 correctionState; /* Plan type: -1 began centered on target; 0 began on another symbol; 1
                          * corrects alignment. */
} M637Record08;

/* Shared sequence state and game objects. */
extern s32 lbl_1_bss_0;
extern OMOBJMAN *lbl_1_bss_4;
extern M637Record08 lbl_1_bss_8[4];
extern M637RecordB8 lbl_1_bss_B8[2];
extern f32 lbl_1_bss_F8[2];
extern f32 lbl_1_bss_100;
extern s32 lbl_1_bss_104;
extern s16 lbl_1_bss_108;
extern s16 lbl_1_bss_10A;
extern s16 lbl_1_bss_10C;
extern s16 lbl_1_bss_10E;
extern s32 lbl_1_bss_110;
extern s32 lbl_1_bss_114;
extern M637Record118 lbl_1_bss_118[2];
extern M637Record1D0 lbl_1_bss_1D0[2];
extern s32 lbl_1_bss_290[4];
extern M637Record2A0 lbl_1_bss_2A0[4];
extern OM_CAMERA_VIEW lbl_1_bss_330;
extern MGTIMER *lbl_1_bss_34C;
extern s32 lbl_1_bss_350;
extern s32 lbl_1_bss_354;
extern s32 lbl_1_bss_358;
extern s32 lbl_1_bss_35C;
extern s32 lbl_1_bss_360;
extern s32 lbl_1_bss_364;
extern s32 lbl_1_bss_368;
extern s32 lbl_1_bss_36C;
extern s32 lbl_1_bss_370;
extern s32 lbl_1_bss_374;
extern MGSEQ_PARAM lbl_1_data_0;
extern u32 lbl_1_data_28[9];
extern s32 lbl_1_data_4C[4], lbl_1_data_5C[3];
extern char lbl_1_data_68[13];
void fn_1_A0(void);
void fn_1_F0(s16 mode, s16 frameNo);
void fn_1_334(s16 mode, s16 frameNo);
void fn_1_3B8(s16 mode, s16 frameNo);
void fn_1_3DC(s16 mode, s16 frameNo);
void fn_1_418(s16 mode, s16 frameNo);
void fn_1_488(s16 mode, s16 frameNo);
void fn_1_50C(s16 mode, s16 frameNo);
void fn_1_510(s16 mode, s16 frameNo);
void fn_1_514(s16 mode, s16 frameNo);
void fn_1_548(void);
s32 fn_1_65C(void);
void fn_1_A54(void);
void fn_1_B94(void);
void fn_1_E4C(void);
s32 fn_1_1110(s32 viewMode);
void fn_1_1464(s32 animationMode);
s32 fn_1_1814(void);
void fn_1_181C(void);
void fn_1_1CF0(void);
void fn_1_2D7C(void);
void fn_1_3460(void);
s32 fn_1_36E8(s32 acceptPlayerInput);
s32 fn_1_4C24(s32 selectionLimit);
s32 fn_1_5444(s32 reelIndex, s32 stopPhase);
void fn_1_55A4(s32 stopFrame);
void fn_1_5914(s32 setupMode);
void fn_1_5C98(s32 displayMode);
s32 fn_1_603C(void);
s32 fn_1_65A0(void);
void fn_1_67C4(void);
void fn_1_6BD4(void);

#endif
