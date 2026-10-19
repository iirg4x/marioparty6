/* Strawberry Shortfuse: shared round, board and effect records. */
#ifndef REL_M680DLL_WORK_H
#define REL_M680DLL_WORK_H

#include "humath.h"
#include "dolphin/gx.h"
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
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/player.h"
#include "game/board/window.h"
#include "game/window.h"
#include "game/printfunc.h"
#include "string.h"
#include "stdio.h"
#include "game/esprite.h"
#include "game/mic.h"
#include "game/thpmain.h"
#include "REL/m680Dll/character.h"
#include "REL/m680Dll/scheduler.h"

/* A path step records the grid coordinate and direction chosen by the search. */
typedef struct M680PathPoint { s32 column, row, direction; } M680PathPoint;

/* Each piece follows a path, then runs its result and attachment animations. */
typedef struct M680GridNode {
    struct M680GridNode *up, *down, *left, *right;
    Vec position, rotation, start, destination;
    s32 column, row, direction;
    M680PathPoint path[156];
    f32 targetRotation;
    s16 model;
    /* Idle, movement and result motions share the piece's model. */
    s16 motions[8];
    s16 itemModel, hookModel;
    /* Movement/result phase, frame timer, step count and direction run length. */
    s16 state, frames, steps, searchAttempts;
    s16 unknown7BE;
    s32 targetColumn, targetRow, pathIndex, pathLength;
    /* Completion flags and frame timers used by the round's animation callbacks. */
    s32 ready, result, winTimer, handTimer, loseTimer;
} M680GridNode;

typedef struct M680GridCell { Vec position; s32 state; } M680GridCell;
typedef struct M680PlayerNode {
    struct M680PlayerNode *previous, *next;
    M680CharacterState *player;
} M680PlayerNode;

typedef struct M680PadButtons { u16 current, pressed, previous; } M680PadButtons;

/* A callback advances one model's motion over a requested number of frames. */
typedef struct M680MotionDurationState {
    s16 motionId;
    u16 unknown2;
    s32 elapsedFrames, duration;
} M680MotionDurationState;

typedef struct M680MotionTimerState {
    s32 elapsedFrames, busy;
    s16 motionId, unknownA;
} M680MotionTimerState;

typedef struct M680PhaseState { s16 models[3], phase; } M680PhaseState;

typedef struct M680EffectState {
    f32 x, y, z;
    s16 model_a, model_b, model_c, unknown12;
    s32 progress, audio_handle;
} M680EffectState;

/* The staged attachment follows a fixed path while turning toward its next segment. */
typedef struct M680MotionWork {
    Vec position, rotation, origin, target;
    s16 model, endMotion, motion;
    u8 unknown36[10];
    f32 angle;
    s32 state, frame, step;
} M680MotionWork;

/* The round owns the players, pieces, board cells and the timed stage effects. */
typedef struct M680State {
    M680CharacterState *actors[4];
    /* Cached stage model and joint-motion handles, indexed by resource entry. */
    s16 models[63], motions[15];
    u8 unknownAC[4];
    M680GridNode grid[10];
    M680GridNode *first, *selected;
    /* Board positions and occupancy/search states: 0 free, 1 occupied, 2/3 visited. */
    M680GridCell cells[14][15];
    s32 nodeOrder[10];
    M680PlayerNode playerNodes[4];
    M680PlayerNode *players, *tail;
    u8 unknown5D20[32];
    M680MotionWork stage;
    u8 unknown5D90[12];
    s16 currentModel, motionIfNonzero, motionIfZero;
    u8 unknown5DA2[14];
    s16 attachedModel, attachedMotion;
    u8 unknown5DB4[12];
    s16 effectHookModel, effectHookMotion;
    M680EffectState effect;
    M680MotionDurationState transitions[3];
    MGTIMER *timer;
    Vec cursorPosition;
    s16 cursorModel, unknown5E16;
    M680PhaseState phaseEffect;
    M680PadButtons buttons;
    s16 unknown5E26;
    M680MotionTimerState gate;
    s16 state, openingPhase, musicStarted, winnerPhase, fadeState, finishPhase;
    s16 nightMode, unknown5E42;
    /* Sequence frame counter, completed sectors, player target and round number. */
    s32 frame, roundWins, playerCount, round;
    s32 eliminatedPlayers[4], eliminatedCount, modeTimer;
    s16 markers[5], gridIndex, repath, unknown5E7A;
    s32 transitionTimer, result, choiceFrames, choiceLimit, choiceCount;
} M680State;

typedef struct M680ObjectManagerState { OMOBJMAN *manager; u32 unknown4; } M680ObjectManagerState;

/* The stage object owns M680State; the stream handle and manager outlive a round. */
extern OMOBJ *lbl_1_bss_0;
extern s32 lbl_1_bss_4;
extern M680ObjectManagerState lbl_1_bss_8;

#endif
