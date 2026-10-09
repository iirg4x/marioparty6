/* Shared Garden Grab state types and callback declarations. */
#ifndef _REL_M635DLL_H
#define _REL_M635DLL_H

#include "game/main.h"
#include "game/object.h"
#include "game/mg/seqman.h"

#define M635_SFX_SCORE_TEAM_0 1859
#define M635_SFX_SCORE_TEAM_1 1860
#define M635_SFX_TEAM_0_STAGE_CUE 1861
#define M635_SFX_TEAM_1_STAGE_CUE 1862
#define M635_SFX_RACE_TEAM_0 1863
#define M635_SFX_RACE_TEAM_1 1864
#define M635_SFX_DAY_INTRO 1865
#define M635_SFX_NIGHT_INTRO 1866

typedef struct M635Model {
    s16 model; /* Engine model handle for one animated stage object. */
    s16 time; /* Current animation frame for this model. */
} M635Model;

typedef struct M635Team {
    u16 buttonIndex[2]; /* Current button-table index for each member. */
    s16 pressCount[2]; /* Rapid-press total for each member. */
    s16 alternatingPresses; /* Accepted presses before rapid-press scoring begins. */
    s16 nextMember; /* Member slot, 0 or 1, whose turn it is in the alternating phase. */
    s16 inputPhase; /* 0 alternates members; 1 counts rapid presses. */
    s16 scoreStep; /* Current team score animation step. */
    M635Model teamProgressModel; /* Team progress prop and its current animation frame. */
    M635Model teamScoreModel; /* Shared team score animation and its current frame. */
    M635Model memberOneScoreModel; /* Score animation associated with member slot 1. */
    M635Model memberZeroScoreModel; /* Score animation associated with member slot 0. */
    s16 lateStageModel; /* Engine handle for the late-stage team animation. */
    s16 teamCueModel; /* Engine handle for the recurring team progress cue. */
    s16 teamFinishModel; /* Engine handle for the team animation started at 20 steps. */
} M635Team;

typedef struct M635Work {
    M635Team team[2]; /* Input, score, and stage-animation state for each team. */
    s16 light; /* Engine handle for the scene's directional light. */
    s16 shadowedGardenModel; /* Engine handle for the shared shadow-mapped garden model. */
    s16 gardenStageModel; /* Engine handle for the shared garden stage model. */
    s16 winner; /* Winning team index, or -1 when the result is a tie or unset. */
    s16 nightF; /* Night-scene selector copied from the board minigame settings. */
    s16 state; /* Shared state counter for the intro and result transitions. */
    s32 music; /* Audio stream handle returned when the minigame music starts. */
} M635Work;

typedef struct M635Player {
    s16 model; /* Engine model handle for this player's character. */
    s16 motions[6]; /* Engine handles for the six character motions used here. */
    s16 playerNo; /* Global player slot, 0 through 3. */
    s16 charNo; /* Character selection used to load this player's model and motions. */
    s16 padNo; /* Controller slot assigned to this player. */
    s16 teamNo; /* Team index, 0 or 1. */
    s16 memberNo; /* Team member slot, 0 or 1. */
    s16 comF; /* Player type copied from the player configuration; nonzero selects CPU input. */
    s16 difficulty; /* CPU difficulty index used to set button timing. */
    float z; /* Current character depth in stage coordinates. */
    s16 timer; /* Frames remaining before the next simulated CPU press. */
    s16 state; /* Scripted character-position step index used by the position table. */
} M635Player;

typedef struct M635MovingModel {
    s16 model; /* Engine model handle for the moving garden prop. */
    s16 timer; /* Frames to wait at a stage edge before moving again. */
    float x; /* Current horizontal stage position. */
    s16 direction; /* Travel direction: 0 moves left; nonzero moves right. */
} M635MovingModel;

typedef struct M635PositionStep {
    float z[2]; /* Target depth in stage units for member slots 0 and 1. */
    s16 frames; /* Frames used to approach this step's target depth. */
} M635PositionStep;

typedef struct M635Motion {
    u32 dataNum; /* Data number for the character motion resource. */
    u32 attr; /* Motion attributes passed to the character animation manager. */
} M635Motion;

typedef struct M635Sprite {
    s16 group; /* Sprite-group handle for one player's button prompt. */
    s16 state; /* Prompt animation state, including 5 for hidden/idle. */
    s16 timer; /* Frames used by the prompt hide or blink animation. */
    s16 member; /* Sprite member index currently shown for the prompt. */
    float scale; /* Horizontal and vertical scale used during prompt animation. */
    /* Four bytes that the prompt code does not read or write. */
    u8 unaccessedBytes[4];
} M635Sprite;

extern M635Work lbl_1_bss_4;
extern OMOBJMAN *lbl_1_bss_64;
extern M635MovingModel lbl_1_bss_68;
extern M635Sprite lbl_1_bss_74[4];
extern s16 lbl_1_bss_B4[2][2];
extern M635Player lbl_1_bss_BC[4];
extern MGSEQ_PARAM lbl_1_data_78;
extern M635PositionStep lbl_1_data_2E8[21];
extern HuVecF lbl_1_data_F8[4];
extern M635Motion lbl_1_data_158[6];

void fn_1_A0(void);
void fn_1_F0(s16 mode, s16 frameNo);
void fn_1_134(s16 mode, s16 frameNo);
void fn_1_164(s16 mode, s16 frameNo);
void fn_1_1AC(s16 mode, s16 frameNo);
void fn_1_224(s16 mode, s16 frameNo);
void fn_1_2D0(s16 mode, s16 frameNo);
void fn_1_3D8(s16 mode, s16 frameNo);
void fn_1_3DC(s16 mode, s16 frameNo);
void fn_1_3E0(s16 mode, s16 frameNo);
void fn_1_408(s16 frameNo);
void fn_1_638(void);
void fn_1_8B0(void);
void fn_1_954(s16 frameNo);
void fn_1_9B8(s16 team);
s16 fn_1_D84(void);
u16 fn_1_EC0(u16 previous, s16 phase);
void fn_1_F44(s16 team);
void fn_1_10A0(void);
u16 fn_1_1168(s16 team, s16 member);
s16 fn_1_1308(s16 difficulty);
s16 fn_1_13F8(s16 difficulty);
void fn_1_1774(OMOBJMAN *objman);
void fn_1_195C(void);
void fn_1_2014(void);
void fn_1_2018(void);
void fn_1_2268(s16 team, s16 player);
void fn_1_2330(void);
void fn_1_25EC(s16 team, s16 player, s16 state);
s16 fn_1_27E4(s16 team, s16 player);
void fn_1_280C(s16 team, s16 player, s16 member);
void fn_1_28A8(s16 team, s16 player);
void fn_1_2904(void);
void fn_1_2954(void);
void fn_1_2CE8(void);
void fn_1_2F08(s16 player, s16 state);
void fn_1_2F40(s16 player);
BOOL fn_1_30C8(s16 player, s16 charNo);
void fn_1_31BC(s16 charNo);
void fn_1_3218(s16 player, s16 motion, float time);
s16 fn_1_32E0(s16 team, s16 member);
void fn_1_3340(void);
void fn_1_33F8(void);
void fn_1_33FC(s16 team);
void fn_1_3738(s16 team, s16 value);
void fn_1_3AC4(s16 team);
void fn_1_3B7C(s16 team, s16 member, s16 player);
void fn_1_3C34(void);
void fn_1_4114(void);
void fn_1_42A0(void);
void fn_1_448C(void);

#endif
