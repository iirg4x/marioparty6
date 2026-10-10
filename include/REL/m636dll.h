/* Shared scene, player, picture and score state for Pixel Perfect. */
#ifndef M636DLL_RECOVERED_H
#define M636DLL_RECOVERED_H

#include "dolphin/types.h"
#include "game/hu3d.h"
#include "game/audio.h"
#include "string.h"
#include "game/mg/actman.h"
#include "game/mg/seqman.h"
#include "game/process.h"
#include "game/gamework.h"
#include "game/mg/timer.h"
#include "game/frand.h"
#include "dolphin/os.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include <dolphin/os.h>
#include <dolphin/types.h>
#include <game/hu3d.h>
#include <game/frand.h>
#include <game/hsfex.h>
#include <game/audio.h>
#include <game/charman.h>
#include <game/mg/seqman.h>

s32 fn_1_A0(s32 musicHandle, s32 musicId);
void fn_1_104(s32 musicHandle);
void fn_1_140(s32 soundHandle, Point3d *pos);
void fn_1_1EC(void);
void fn_1_2D0(s16 mode, s16 frameNo);
void fn_1_2F4(s16 mode, s16 frameNo);
void fn_1_494(s16 mode, s16 frameNo);
void fn_1_5F0(s16 mode, s16 frameNo);
void fn_1_12E0(s16 mode, s16 frameNo);
void fn_1_151C(s16 mode, s16 frameNo);
void fn_1_1C74(s16 mode, s16 frameNo);
void fn_1_1F58(s16 mode, s16 frameNo);
void fn_1_1F78(s16 mode, s16 frameNo);
void ObjectSetup(void);
void fn_1_21E4(OMOBJ *obj);
void fn_1_2F10(MGACTOR *actor, int playerSlot);
void fn_1_31BC(void);
void fn_1_3460(s32 pictureIndex, s32 cellIndex);
BOOL fn_1_35E0(s32 pictureIndex, s32 cellIndex);
void fn_1_364C(s32 pictureIndex, s32 cellIndex);
s32 fn_1_37A8(s32 pictureIndex, s32 cellIndex);
void fn_1_3834(s32 pictureIndex, s32 cellIndex);
int fn_1_3984(s32 side);
s32 fn_1_39FC(s32 side);
s32 fn_1_3A84(s32 side, s32 cellIndex);
s32 fn_1_3AE0(s32 side, s32 column, s32 row);
void fn_1_3C94(HU3D_MODEL *model, Mtx *matrix);
void fn_1_3C98(s32 playerIndex);
s32 fn_1_3D28(s32 playerIndex);
s32 fn_1_3D80(s32 playerSlot);
s32 fn_1_3E30(s32 playerIndex);
void fn_1_40E0(s32 playerIndex);
void fn_1_48FC(s32 playerIndex);
void fn_1_50A8(s32 playerIndex);
void fn_1_582C(s32 playerIndex);
void fn_1_5F54(s32 playerIndex, Point3d *start, Point3d *target, Point3d *stickDirection,
               s32 gridCoordinates);
void fn_1_6360(OMOBJ *object);
void fn_1_6398(s32 motionIndex);
void fn_1_644C(void);
void fn_1_6478(void);
void fn_1_6790(BOOL visible);
void fn_1_67F4(void);

/* Byte spans used by the sequence, camera and CPU state. */
#define PIXEL_SEQUENCE_STATE_BYTES 16
#define PIXEL_CAMERA_TIMER_STATE_BYTES 52
#define PIXEL_CPU_STATE_BYTES 52
#define PIXEL_STEERING_SCORES_BYTES 60
#define PIXEL_PATTERN_STATE_BYTES 88
#define PIXEL_PLAYER_RECORDS_OFFSET 120
#define PIXEL_TIMER_RECORD_OFFSET 68

/* State shared by the scene, round sequence and CPU player routines. */
typedef struct M636WorkingData {
    u8 sequenceState[PIXEL_SEQUENCE_STATE_BYTES]; /* Object manager, round state, frame counter and
                                                   * camera object. */
    s16 cameraMotions[2]; /* Opening and play-view camera motion IDs. */
    u8 cameraAndTimerState[PIXEL_CAMERA_TIMER_STATE_BYTES]; /* Camera model/time values and
                                                             * countdown timer pointer. */
    s32 charNo[4]; /* Character IDs for the four player slots. */
    s32 padNo[4]; /* Controller ports for the four player slots. */
    s32 teamSide[4]; /* Team side for each player: 0 is left, 1 is right. */
    MGPLAYER *players[4]; /* Player records owned by the actor manager. */
    u8 aiState[PIXEL_CPU_STATE_BYTES]; /* Landing counters, CPU enable state, CPU modes and mode
                                        * timers. */
    s32 targetCell[4]; /* CPU target panel, 0-15; -1 means no panel selected. */
    u8 steeringAndScores[PIXEL_STEERING_SCORES_BYTES]; /* CPU direction vectors, team points and
                                                        * round index. */
    s32 mode; /* Round/result outcome: 1 left, 2 right, 3 timeout or draw. */
    u8 referenceModelPrefix[2]; /* Storage immediately before the reference display model. */
    s16 referenceModel; /* Reference display model hidden when a cell is shown. */
    u8 pictureModelPrefix[2]; /* Storage immediately before the picture model arrays. */
    s16 pictureIntroModels[3]; /* Opening animation model for each picture. */
    s16 pictureResultModels[3]; /* Result animation model for each picture. */
    s16 pictureCellModels[3][16]; /* Animated cell model for each of the three pictures. */
    s16 pictureFinishModels[3]; /* Picture model shown when play finishes. */
    s16 roundIndicatorModel; /* Model with idle, reset and team-result motions. */
    s16 roundIndicatorMotions[4]; /* Idle, reset, left-result and right-result motion IDs. */
    s16 panelBaseModel; /* Model shared by the two teams' floor panels. */
    s16 panelMotions[4]; /* Off, on, turn-on and turn-off panel motions. */
    s16 panelModels[2][16]; /* Floor panel models, indexed by team and cell. */
    u8 roundIndicatorState[4]; /* Marks whether the round indicator has returned to idle. */
    s16 model; /* Scene model with idle and team-result motions. */
    s16 motion[3]; /* Idle, left-result and right-result scene motion IDs. */
    s16 finishModel; /* Model animated when the final round finishes. */
    s16 finishMotions[2]; /* Initial and game-finish motion IDs. */
    u8 patternState[PIXEL_PATTERN_STATE_BYTES]; /* Picture selection, five cell choices and
                                                 * team/target pixel grids. */
    s16 referenceCellModels[16]; /* Reference display models for the sixteen picture cells. */
    HUSPR_GROUPID scoreBoxes[2]; /* Background sprite group for each team's score. */
    HUSPR_GROUPID scoreGroups[2]; /* Two earned markers and two empty markers per team. */
    ANIMDATA *earnedPointAnim; /* Animation data for an earned round point. */
    ANIMDATA *emptyPointAnim; /* Animation data for an unearned round point. */
    s32 musicHandle; /* Background music handle; -1 before playback starts. */
    s16 extraSceneModel; /* Additional scene model loaded during setup. */
    s16 extraSceneMotion; /* Motion attached to the additional scene model. */
} M636WorkingData;
extern M636WorkingData lbl_1_bss_0;

extern MGSEQ_PARAM lbl_1_data_0;
extern unsigned int lbl_1_data_E0[11];
extern u8 lbl_1_data_140[16];
extern float lbl_1_data_230[4];
extern Point3d lbl_1_data_240[4];
extern f32 lbl_1_data_270[2][2];
extern u32 lbl_1_data_280[2];
extern u32 lbl_1_data_288[3];
extern u32 lbl_1_data_294[3];
extern u32 lbl_1_data_2A0[3];
extern u32 lbl_1_data_2AC[3][16];
extern u32 lbl_1_data_36C[3];
extern u8 lbl_1_data_378[768];

#define BSS16(offset) (*(s16 *)((u8 *)&lbl_1_bss_0 + (offset)))
#define BSS32(offset) (*(s32 *)((u8 *)&lbl_1_bss_0 + (offset)))
#define BSS_ARRAY32(offset) ((s32 *)((u8 *)&lbl_1_bss_0 + (offset)))
#define PLAYER(index)                                                                              \
    (*(MGPLAYER **) ((u8 *) &lbl_1_bss_0 + (index) * sizeof(MGPLAYER *) +                          \
                     PIXEL_PLAYER_RECORDS_OFFSET))
#define TIMER (*(MGTIMER **)((u8 *)&lbl_1_bss_0 + PIXEL_TIMER_RECORD_OFFSET))
#define TICK (*(u32 *)((u8 *)&lbl_1_bss_0 + 8))
#define READ_MODEL(resource)                                                                       \
    Hu3DModelCreate(HuDataSelHeapReadNum((resource), HU_MEMNUM_OVL, HEAP_MODEL))
#define READ_MOTION(model, resource)                                                               \
    Hu3DJointMotion((model), HuDataSelHeapReadNum((resource), HU_MEMNUM_OVL, HEAP_MODEL))

#endif
