/* Lift Leapers player, input, and lift state shared across its source files. */
#ifndef M618DLL_H
#define M618DLL_H

#include "game/main.h"
#include "game/object.h"
#include "game/mg/actman.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"

/* Per-player state for Lift Leapers. */
typedef struct M618Player {
    MGPLAYER *player; /* Engine player controlled by this minigame. */
    int moveDirection; /* Horizontal direction sign: 1 or -1. */
    int fallState; /* 0 active, 1 returning on the recovery lift, 2 in the scripted fall jump, 3
                    * traversing course sections. */
    int fallFrames; /* Frames spent falling before the player is reset. */
    int routeState; /* Stage-specific movement phase, advanced by the game. */
    int actionFrames; /* Frames elapsed in timed stage actions. */
    HU3D_MOTIONID motion[4]; /* Character motions used for stage and result poses. */
    HU3D_MODELID model; /* Recovery lift model used to carry this player back to the course. */
    HuVecF liftModelPosition; /* Current minigame-model position in course units. */
    float recoveryAngle; /* Degrees through the player's lift return animation, from 0 to 90. */
    int recoveryPositionValid; /* Set when a known collision mesh supplies a recovery position. */
    HuVecF recoveryPosition; /* Course position the player is returned toward. */
    int aiRouteChoice; /* AI-selected lift route: 0 for none, otherwise 1 through 4. */
    int aiWaitFrames; /* Frames remaining before the CPU's next lift-jump decision; movement
                       * continues during this wait. */
    unsigned char unidentifiedBytes[4]; /* Purpose is not established by the game logic. */
    HuVecF nearbyLiftPosition; /* Course position of the next lift used by the CPU. */
    int stageSection; /* Current course section, numbered 0 through 3. */
    int jumpFrames; /* Frames elapsed in the short scripted jump. */
    float jumpStartHeight; /* Player height in course units when the jump begins. */
} M618Player;

/* CPU decision state and the controller values supplied to a player. */
typedef struct M618Input {
    int cpuDifficulty; /* CPU difficulty copied from the player settings. */
    int decisionPending; /* AI decision transition flag. */
    int decisionIndex; /* Current option within the selected lift route. */
    int decisionCount; /* Number of options selected for this route. */
    int stickX; /* Horizontal stick value passed to the player controller. */
    int stickY; /* Vertical stick value passed to the player controller. */
    int buttonDown; /* Newly pressed button bits passed to the player controller. */
    int buttonHeld; /* Held button bits passed to the player controller. */
    int aiSpeed; /* Horizontal AI movement magnitude. */
} M618Input;

/* A course model and its matching collision model. */
typedef struct M618Model {
    HU3D_MODELID model; /* Visible course model. */
    HU3D_MODELID colModel; /* Collision model positioned at this course object's platform
                            * position. */
    float collisionHeight; /* Y coordinate of the synchronized collision model position. */
    unsigned char unidentifiedBytes[4]; /* Purpose is not established by the game logic. */
} M618Model;

extern int lbl_1_bss_0;
extern HUPROCESS *lbl_1_bss_4;
extern HuVecF lbl_1_bss_8[4];
extern int lbl_1_bss_38;
extern int lbl_1_bss_40;
extern int lbl_1_bss_44[4][4];
extern M618Input lbl_1_bss_84[4];
extern M618Player lbl_1_bss_114[4];
extern HU3D_MODELID lbl_1_bss_2B4[2][2];
extern HU3D_MODELID lbl_1_bss_2BC[4][2];
extern HU3D_MODELID lbl_1_bss_2CC[4][5];
extern HU3D_MODELID lbl_1_bss_2F4[4][3];
extern M618Model lbl_1_bss_30C[37];
extern float lbl_1_bss_4C8;
extern OM_CAMERA_VIEW lbl_1_bss_4CC[4];
extern int lbl_1_bss_53C;
extern HU3D_MODELID lbl_1_bss_540[2];
extern MGTIMER *lbl_1_bss_544;
extern int lbl_1_bss_548;
extern int lbl_1_bss_54C;
extern int lbl_1_bss_550;
extern int lbl_1_bss_554;
extern int lbl_1_bss_558;
extern MGSEQ_PARAM lbl_1_data_0;
extern int lbl_1_data_28[4];
extern unsigned int lbl_1_data_38[4];
extern HuVec2f lbl_1_data_48[4];

void fn_1_A0(void);
void fn_1_DC(s16 mode, s16 frameNo);
void fn_1_1EC(s16 mode, s16 frameNo);
void fn_1_294(s16 mode, s16 frameNo);
void fn_1_2B4(s16 mode, s16 frameNo);
void fn_1_2F0(s16 mode, s16 frameNo);
void fn_1_310(s16 mode, s16 frameNo);
void fn_1_33C(s16 mode, s16 frameNo);
void fn_1_340(s16 mode, s16 frameNo);
void fn_1_344(s16 mode, s16 frameNo);
void fn_1_378(void);
int fn_1_570(void);
void fn_1_644(void);
void fn_1_84C(void);
void fn_1_910(void);
int fn_1_DA8(void);
void fn_1_1B40(int cameraMode);
int fn_1_1CD8(void);
void fn_1_1CE0(void);
void fn_1_214C(void);
void fn_1_3294(int playerNo);
void fn_1_44CC(int playerNo);
void fn_1_4BE0(void);
void fn_1_52D0(void);
void fn_1_6298(void);
void fn_1_6704(int modelIndex);
void fn_1_6C0C(void);
void fn_1_796C(int winner);
void fn_1_8180(int playerNo);
void fn_1_C450(M618Input *input, int stickX, int stickY, int buttonDown, int buttonHeld);

u32 MgSeqModeNext(void);
u16 MgSeqModeChangeOff(void);
u16 MgSeqModeChangeOn(void);

#endif
