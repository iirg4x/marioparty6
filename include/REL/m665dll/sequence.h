/* Sequence phase, team, scene, and music state for Verbal Assault. */
#ifndef REL_M665DLL_SEQUENCE_H
#define REL_M665DLL_SEQUENCE_H

#include "dolphin/types.h"
#include "game/process.h"

typedef struct M665SequenceState {
    s16 mainPlayer; /* Player-work index for the player on team zero. */
    s16 otherPlayers[3]; /* Player-work indices for the three opposing players. */
    s16 nightMode; /* Nonzero selects the night lighting and models. */
    s16 winningTeam; /* 0 for the main player, 1 for opponents, or -1 for a draw. */
    s16 phaseFrames; /* Frames elapsed in the current sequence phase. */
    s16 phase; /* Current fade-in or winner-transition phase. */
    s16 finished; /* Set after wipe-out to stop opponent gameplay updates during winner setup. */
    s16 backgroundModels[4]; /* Linked scrolling background model IDs. */
    s16 modelID; /* Main stage model ID. */
    s32 audioHandle; /* Streamed-music handle. */
    f32 scrollSpeed; /* Background movement in world units per update. */
} M665SequenceState;

extern M665SequenceState lbl_1_bss_250;

extern u16 lbl_1_bss_0;
extern s16 lbl_1_bss_2;
extern s16 lbl_1_bss_4;
extern HUPROCESS *lbl_1_bss_8;

struct omObj_s;
extern struct omObj_s *lbl_1_bss_274;
extern struct omObj_s *lbl_1_bss_278;

#endif
