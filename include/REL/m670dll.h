/* Shared state for Fruit Talktail's microphone round, pillars, and player callbacks. */
#ifndef M670DLL_H
#define M670DLL_H

#include "dolphin.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/mg/actman.h"
#include "game/mg/timer.h"

typedef struct M670Cpu_s {
    int playerNo; /* Player slot controlled by this CPU. */
    MGPLAYER *player; /* Actor and controller state for that player. */
    unsigned int state; /* Current CPU decision or movement state. */
    int timer; /* Frames spent waiting or moving. */
    int pillarNo; /* Target pillar index while moving. */
    HuVecF targetPos; /* World-space destination for the current move. */
    int buttonPressFlag; /* Set to 30 when the nearest pillar's height is not 100. If movement
                          * passes the distance and timeout checks, MgPlayerPadSet sends A;
                          * otherwise the flag is cleared without a pad update. */
} M670CPU;

typedef struct M670Work_s {
    OMOBJMAN *objman; /* Object manager for the playfield and player callbacks. */
    /* The round logic does not access these words; their role is unknown. */
    int reservedWordA;
    int reservedWordB;
    HU3D_MODELID cameraModel[3]; /* Camera models paired with the three camera motions. */
    HU3D_MOTIONID cameraMotion[3]; /* Opening, solo-player, and group camera motions. */
    HU3D_MODELID models[3]; /* Shared scene models, including the solo result platform. */
    HU3D_MODELID pillarModel[24]; /* Visible model at each of the 24 pillar positions. */
    HU3D_MOTIONID pillarMotion[24][4]; /* Four motions for each position's pillar type. */
    HU3D_MODELID collisionModel[24]; /* Hidden collision model paired with each pillar. */
    HuVecF pillarPos[24]; /* World-space base position of each pillar. */
    int collisionCount; /* Number of collision models registered with the playfield. */
    HU3D_MODELID collisionModels[24]; /* Collision models passed to the playfield collision map. */
    /* The round logic does not access this halfword; its role is unknown. */
    s16 reservedHalfword;
    HU3D_MODELID positionModel; /* Model whose named points define pillar and spawn positions. */
    HU3D_MODELID winnerModel; /* Model containing named positions for the winners. */
    s16 characterNo[4]; /* Character IDs for the four player slots. */
    s16 padNo[4]; /* Controller IDs for the four player slots. */
    MGPLAYER *players[4]; /* Player actor state for each slot. */
    int group[4]; /* Zero for the solo player, one for the opposing group. */
    int selectedWord; /* Selected pillar word type, or -1 when none is selected. */
    int speedLevel; /* Pillar cycle speed level, advanced up to 10. */
    s16 soloPlayer; /* Player slot that plays the microphone prompt. */
    s16 soloPad; /* Controller ID used by the microphone selection window. */
    s16 soloCharacter; /* Character ID of the solo player. */
    int state; /* Current shared pillar-cycle state. */
    int playerState[4]; /* Motion/retirement state for each player slot. */
    OMOBJ *playerObjects[4]; /* Per-player callback objects. */
    OMOBJ *pillarObject; /* Callback object that updates the pillar cycle. */
    int remainingPlayers; /* Bit mask of opposing player slots still in the round. */
    M670CPU cpu[4]; /* Decision and movement state for computer-controlled players. */
    u16 micContext; /* Microphone recognition context loaded for this game. */
    MGTIMER *timer; /* Round countdown timer. */
    int pattern; /* Selected layout index into the five pillar-word layouts. */
    int music; /* Background music handle, or -1 before it starts. */
} M670WORK;

extern M670WORK lbl_1_bss_10;
extern float lbl_1_bss_3B8[24];
extern int lbl_1_bss_418[25];
extern float lbl_1_bss_47C, lbl_1_bss_480;
extern s8 *lbl_1_data_250[6];

BOOL fn_1_1460(HuVecF *src, HuVecF *dst);
void fn_1_15B8(int soundId, HuVecF *pos);
void fn_1_1658(void);
void fn_1_22F0(int pillarNo, float height);
void fn_1_2384(MGACTOR *actor, int playerNo);
void fn_1_2460(int playerNo, int state);
void fn_1_2690(OMOBJ *obj);
void fn_1_28E8(OMOBJ *obj);
void fn_1_2A24(unsigned int state);
int fn_1_2DE8(HuVecF *pos);
float fn_1_2EF4(int pillarNo);
int fn_1_2F44(HuVecF *pos);
void fn_1_3098(OMOBJ *obj);
void fn_1_3B30(void);
void fn_1_3BDC(void);

#endif
