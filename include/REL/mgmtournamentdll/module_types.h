/* Motion settings and object work data for Mount Duel tournament characters. */
#ifndef MG_TOURNAMENT_DLL_MODULE_TYPES_H
#define MG_TOURNAMENT_DLL_MODULE_TYPES_H

#include <math.h>
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

/* Current tournament layout shared by the scene translation units. */
s16 tournamentLayoutGet(void);
void tournamentCameraViewSet(s16 viewIndex, u32 durationFrames);
void tournamentCameraRestore(void);
void tournamentCameraOpeningPlay(void);
void tournamentCameraRoundViewSet(u32 durationFrames);
void fn_1_2E28(void);
void fn_1_3394(s16 modelIndex, s16 enabled);
void fn_1_34B0(s16 modelIndex);
void fn_1_34F0(void);
void fn_1_3E04(void);
s16 fn_1_3F00(void);
void fn_1_3F18(s16 side);
void fn_1_3F8C(void);
void fn_1_43BC(void);
void fn_1_4A1C(void);
void fn_1_4A94(s16 playerNo, s16 characterNo, s16 groupNo, Point3d worldPosition);
void fn_1_E9C(void);
void fn_1_1134(void);
void fn_1_64A8(void);
s32 fn_1_6A24(void);
void fn_1_6D78(void);
void fn_1_71E8(s16 *firstPlayer, s16 *secondPlayer);
void fn_1_74BC(void);
void fn_1_7924(void);
s16 fn_1_7BC0(s16 player);
s16 fn_1_7C00(s16 playerNo);
s16 fn_1_7C44(s16 playerNo);
void fn_1_7C78(Vec *center);
void fn_1_818C(void);
void fn_1_A6B4(void);
void fn_1_AB48(void);
void fn_1_AD50(void);
void fn_1_B014(void);
void fn_1_B2E0(void);
void fn_1_B57C(void);
void fn_1_B80C(void);
void fn_1_B850(void);
s16 fn_1_BAD4(void);
void fn_1_CF6C(void);
void fn_1_5154(void);
void fn_1_5590(void);
void fn_1_56B4(u16 enabled);
void fn_1_5728(void);
void fn_1_5740(s16 positionIndex, s16 duration);
void fn_1_57E0(s16 duration);
void fn_1_5A44(s16 duration);
void fn_1_DC28(void);
void fn_1_1A8C(void);
void fn_1_1BA0(void);
void fn_1_1D74(void);
void fn_1_1E80(void);
void fn_1_1F90(void);
s16 fn_1_2128(void);
void fn_1_2728(s16 player);
void fn_1_2948(void);
s16 fn_1_2BA0(void);

#endif
