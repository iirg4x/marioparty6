/* Shared state and interface for the Lunar-tics minigame REL. */
#ifndef _REL_M657DLL_H
#define _REL_M657DLL_H

#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/mg/seqman.h"
#include "game/mg/score.h"

typedef struct {
    /* Object manager that owns the minigame's update objects. */
    OMOBJMAN *objman;
    /* Camera objects for the two participating teams. */
    OMOBJ *cameraObj[2];
    /* Shared scene light. */
    HU3D_LIGHTID light;
    /* Minigame result-transition state. */
    s32 state;
    /* Active music stream handle, or -1 when none is active. */
    s32 music;
    /* Frame counter used by the result sequence. */
    s32 timer;
    /* Team indices recorded as winners; -1 means no winner in that slot. */
    s16 winners[2];
    /* Number of winning teams: zero, one, or two. */
    s16 winnerCount;
    /* Losing team whose character continues into the result fly-by. */
    s16 losingTeam;
} M657Work;

typedef struct {
    /* Eight bytes in the allocated camera record whose meaning is unknown. */
    u8 unknownPrefix[8];
    /* Camera bit mask used by the 3D camera APIs. */
    s32 cameraMask;
    /* Player object followed by this camera. */
    OMOBJ *target;
    /* Camera position in world units. */
    HuVecF pos;
} M657Camera;

/* Player identity, presentation state, and world-space motion. */
typedef struct {
    /* Camera bit mask that renders this player. */
    s16 cameraMask;
    /* Global player slot, from the game player configuration. */
    s16 playerNo;
    /* Minigame team index, 0 or 1. */
    s16 team;
    /* Character model index. */
    s16 charNo;
    /* Currently selected character motion-table index. */
    s16 motionIndex;
    /* Player position in world units. */
    HuVecF pos;
    /* Player model rotation in degrees. */
    HuVecF rot;
    /* Normalized direction used by the winner fly-by. */
    HuVecF vel;
    /* Set while the player is falling into the arena. */
    BOOL falling;
    /* Four allocated bytes with no demonstrated use. */
    u8 unaccessedStateBytes[4];
    /* Per-player minigame state. */
    s32 state;
    /* Four allocated bytes with no demonstrated use. */
    u8 unaccessedWinnerBytes[4];
    /* Whether this team is a winner. */
    BOOL winner;
    /* Normalized descent progress, calculated from the 2573-unit start height. */
    float progress;
    /* Set once the player reaches the starting position. */
    BOOL ready;
} M657PlayerView;

/* Per-player input and animation state allocated alongside M657PlayerView. */
typedef struct {
    /* Shared player identity and movement state. */
    M657PlayerView player;
    /* Active sound handle, or -1 when no player sound is playing. */
    s32 sound;
    /* Controller slot assigned to this player. */
    s16 padNo;
    /* Horizontal analog stick axis. */
    float stickX;
    /* Vertical analog stick axis. */
    float stickY;
    /* Buttons sampled for the current player update. */
    u32 buttons;
    /* Buttons sampled during the preceding player update. */
    u32 prevButtons;
    /* Twenty allocated bytes with no demonstrated use. */
    u8 unaccessedInputBytes[20];
    /* Set when the exit animation has finished. */
    BOOL exitFinished;
    /* Whether this player is computer-controlled. */
    BOOL computer;
    /* Computer difficulty index, or -1 for a human player. */
    s16 difficulty;
    /* Set while the computer input sequence is active. */
    BOOL inputActive;
    /* Step in the computer input timing sequence. */
    s32 inputState;
    /* Frames elapsed in the current computer input timing step. */
    s32 inputTimer;
    /* Frames in the computer's A-button hold phase before it advances to the inactive phase. */
    s32 inputDelay;
    /* Frames in the inactive phase after the A-button interval and before input resets. */
    s32 inputDuration;
} M657Player;

typedef struct {
    /* Player object controlled by this computer-input record. */
    OMOBJ *obj;
    /* Whether this player is computer-controlled. */
    BOOL enabled;
} M657ComPlayer;

typedef struct {
    /* Computer-input records for the two participating teams. */
    M657ComPlayer *players[2];
    /* State of the computer-input controller. */
    s16 state;
    /* Reset to zero when the computer controller changes game modes. */
    s32 timer;
    /* Values returned by public accessors; their writers are outside this source. */
    s32 unknownValueA;
    s32 unknownValueB;
} M657ComWork;

typedef struct {
    /* Vertical viewport origin in pixels. */
    float y;
    /* Horizontal viewport origin in pixels. */
    float x;
    /* Viewport width in pixels. */
    float width;
    /* Viewport height in pixels. */
    float height;
} M657Viewport;

typedef struct {
    /* HUD sprite handles for the arena gauge. */
    s16 sprites[9];
    /* HUD marker sprite for each team. */
    s16 playerSprites[2];
} M657SpriteWork;

typedef struct {
    /* HUD group drawing the team's score panel. */
    HUSPR_GROUPID box;
    /* Score display for the remaining whole seconds. */
    MGSCORE *seconds;
    /* Score display for the remaining fractional second. */
    MGSCORE *hundredths;
    /* Sprite group containing the decimal separator. */
    HUSPR_GROUPID separator;
    /* Six allocated bytes with no demonstrated use. */
    u8 unaccessedScoreBytes[6];
    /* Remaining time in frames; negative values show time after the shared countdown. */
    s32 frames;
    /* Whether this team's score display is still being updated. */
    BOOL running;
} M657ScoreTeam;

typedef struct {
    /* Shared score-display state machine. */
    s32 state;
    /* Score and HUD data for each team. */
    M657ScoreTeam teams[2];
    /* Shared remaining-time countdown in frames, at 60 frames per second. */
    s32 frames;
} M657ScoreWork;

typedef struct {
    /* Item position in world units. */
    HuVecF pos;
    /* Unknown value initialized to zero; no meaning is demonstrated here. */
    s32 unknownValue;
} M657ArenaItem;

typedef struct {
    /* Arena item position and associated value. */
    M657ArenaItem item;
    /* Twelve allocated bytes with no demonstrated use. */
    u8 unaccessedArenaBytes[12];
    /* Arena object state machine. */
    s16 state;
} M657ArenaWork;

extern M657Work lbl_1_bss_0;
extern M657Work *lbl_1_data_0;
extern M657Viewport lbl_1_data_4[2];
extern s32 lbl_1_data_24[2];
extern MGSEQ_PARAM lbl_1_data_2C;
extern OMOBJ *lbl_1_bss_30[2];
extern OMOBJ *lbl_1_bss_28;
extern OMOBJ *lbl_1_bss_38;
extern OMOBJ *lbl_1_bss_40;
extern OMOBJ *lbl_1_bss_48;

void fn_1_0(void);
void fn_1_6C(void);
void fn_1_70(void);
void fn_1_BC(OMOBJMAN *objman);
void fn_1_258(void);
void fn_1_3EC(void);
void fn_1_3F0(s16 team, OMOBJ *target);
void fn_1_428(OMOBJ *obj);
void fn_1_438(OMOBJ *obj);
void fn_1_5C4(OMOBJ *obj);
void fn_1_5C8(void);
void fn_1_6E0(void);
void fn_1_784(void);
void fn_1_B10(s16 mode, s16 frameNo);
void fn_1_BD4(s16 mode, s16 frameNo);
void fn_1_C74(s16 mode, s16 frameNo);
void fn_1_CA0(s16 mode, s16 frameNo);
void fn_1_D28(s16 mode, s16 frameNo);
void fn_1_1090(s16 mode, s16 frameNo);
void fn_1_1564(s16 mode, s16 frameNo);
void fn_1_15B8(s16 mode, s16 frameNo);
void fn_1_15BC(s16 mode, s16 frameNo);
void fn_1_15C0(OMOBJMAN *objman);
void fn_1_1658(void);
void fn_1_165C(OMOBJ *obj);
void fn_1_1720(void);
void fn_1_1884(OMOBJ *obj);
void fn_1_19B4(OMOBJ *obj);
void fn_1_1AD4(void);
s32 fn_1_1B20(void);
void fn_1_1B84(OMOBJ *obj);
void fn_1_1F6C(OMOBJ *obj);
void fn_1_2018(OMOBJMAN *objman);
void fn_1_21A8(void);
void fn_1_21CC(OMOBJ *obj);
void fn_1_23F4(OMOBJ *obj);
void fn_1_2694(OMOBJ *obj, BOOL visible);
void fn_1_2748(OMOBJ *obj);
void fn_1_2840(OMOBJ *obj, s16 motionIndex);
void fn_1_294C(OMOBJ *obj);
void fn_1_2A50(OMOBJ *obj);
s32 fn_1_2AF0(OMOBJ *obj);
s32 fn_1_2B64(s16 team);
void fn_1_2B98(OMOBJ *obj);
void fn_1_2BE4(s16 team, s32 state);
void fn_1_2C20(s32 state);
s32 fn_1_2E78(s16 team);
s32 fn_1_2EEC(s16 team);
float *fn_1_2F48(s16 team);
void fn_1_2F90(s16 team, u16 cameraMask);
void fn_1_306C(s16 team, HuVecF pos);
void fn_1_30FC(s16 team);
s32 fn_1_3154(s32 team);
s32 fn_1_319C(OMOBJ *obj);
void fn_1_31CC(OMOBJ *obj);
void fn_1_32F4(OMOBJ *obj);
void fn_1_3468(OMOBJ *obj);
void fn_1_3B50(OMOBJ *obj);
void fn_1_3E1C(OMOBJMAN *objman);
void fn_1_3EA4(void);
void fn_1_3EA8(s16 team);
s32 fn_1_3EEC(s32 team);
s32 fn_1_3F2C(void);
void fn_1_3F54(void);
void fn_1_4004(OMOBJ *obj);
void fn_1_42F4(OMOBJ *obj);
void fn_1_4570(OMOBJMAN *objman);
void fn_1_4608(OMOBJ *obj);
void fn_1_463C(OMOBJ *obj);
void fn_1_47A8(OMOBJ *obj);
void fn_1_47D0(OMOBJ *obj);
void fn_1_47EC(OMOBJ *obj);
s32 fn_1_48C0(void);
s32 fn_1_48E8(void);
s32 fn_1_4910(M657ComPlayer *com);
void fn_1_4964(M657ComPlayer *com);
void fn_1_4A48(M657ComPlayer *com);
void fn_1_4A80(M657ComPlayer *com);
void fn_1_4B10(M657ComPlayer *com);
void fn_1_4BA4(M657ComPlayer *com);
void fn_1_4C78(M657ComPlayer *com);
void fn_1_4D4C(M657ComPlayer *com);
void fn_1_4E58(OMOBJMAN *objman);
void fn_1_4EE4(s16 team);
void fn_1_5020(void);
void fn_1_5094(void);
void fn_1_5248(OMOBJ *obj);
void fn_1_5414(OMOBJ *obj);

#endif
