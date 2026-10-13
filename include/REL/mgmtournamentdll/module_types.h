/* Motion settings and object work data for Mount Duel tournament characters. */
#ifndef MG_TOURNAMENT_DLL_MODULE_TYPES_H
#define MG_TOURNAMENT_DLL_MODULE_TYPES_H

#include "dolphin/math.h"
#include "game/object.h"

typedef struct TournamentMotionData {
    s16 motionNo; /* Index of the character motion selected by this scene state. */
    f32 transitionTime; /* Character motion blend duration in frames. */
    u32 modelAttributes; /* Model and motion flags applied for this state. */
    u32 motionEffectsEnabled; /* Character-wide enable flag for automatic motion effects, including
                               * dust and some voices. */
    s32 motionAttributeTableFlag; /* Table flag checked by the selector; both branches set the same
                                   * motion attribute. */
} TournamentMotionData;

typedef struct TournamentPlayerAnimationWork {
    s16 playerNo; /* Player slot, from zero through three. */
    s16 characterNo; /* Character model slot used by the character manager. */
    s16 padNo; /* Controller slot copied from the player configuration. */
    s16 groupNo; /* Tournament side copied from the player configuration. */
    s32 playerType; /* Configured player type; zero permits controller rumble. */
    s16 difficulty; /* CPU difficulty used to weight the round-bonus odds. */
    s16 sceneState; /* Current step in the character tournament scene animation. */
    s16 motionMode; /* Current character motion-table entry. */
    Point3d position; /* Character position in world coordinates. */
    Point3d rotation; /* Character rotation in degrees while turning toward a destination. */
    f32 modelHeight; /* Height returned by the character manager during setup. */
    Point3d targetPosition; /* Destination position in world coordinates. */
    Point3d velocity; /* Per-update movement delta in world coordinates. */
    Point3d storedPosition; /* Position saved when a scene movement begins. */
} TournamentPlayerAnimationWork;

extern s16 lbl_1_bss_10C[4];

#endif
