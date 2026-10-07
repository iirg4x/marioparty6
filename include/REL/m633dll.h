#ifndef M633DLL_H
#define M633DLL_H

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

#define M633_SEGMENT_SPAWN_SE_ID 1849 /* Effect played when a segment is created at the arena center. */
#define M633_SEGMENT_WALL_REFLECTION_SE_ID 1850 /* Effect played after a segment reflects from the arena wall. */
#define M633_PLAYER_ELIMINATION_SE_ID 1851 /* Effect played when a moving segment eliminates a player. */
#define M633_ROTATION_CONTROL_SE_ID 1852 /* Effect held while either rotation control is pressed. */


/* Shared arena, player, camera, and sequence state used by the minigame callbacks. */
typedef struct M633Segment {
    s32 endpointModelIndex; /* Model index for the segment's displayed effect. */
    s16 reflected; /* 0 while advancing; 1 after a wall reflection. */
    unsigned char unknown06[2];
    Point3d startPosition; /* Starting point used for segment collision checks. */
    Point3d direction; /* Direction used while extending the segment. */
    Point3d endPosition; /* Current moving endpoint in world coordinates. */
} M633Segment;

typedef struct M633AI {
    s32 decisionCounter; /* Computer-player callback counter; negative values have callback-specific behavior. */
    s32 unknown04; /* Unused by the current computer-player behavior. */
    s32 decisionMode; /* Decision mode: fn_1_38C0 selects a target or random heading; fn_1_3FFC selects free movement or arena routing. */
    float targetHeadingDegrees; /* Target heading in degrees. */
    float turnDirection; /* Turn direction, initialized to -1 or 1. */
    s32 targetPlayerNo; /* Player slot selected as an attack target. */
    Point3d destination; /* Current world-space destination for computer movement. */
} M633AI;

typedef struct M633Work {
    HUPROCESS *workProcess;
    unsigned char unknown004[12];
    /* Camera motion IDs and the currently active motion are kept in this group. */
    s16 cameraMotionIds[4];
    s16 cameraModelIds[4];
    s16 activeCameraModelId;
    unsigned char unknown022[2];
    float cameraMotionTime;
    float cameraMotionMaxTime;
    MGTIMER *timer;
    OMOBJ *object;
    /* Handles for the four arena models switched by phase transitions. */
    s16 arenaPhaseModelIds[4];
    s16 arenaModelId;
    s16 placementModelId;
    MGPLAYER *players[4];
    s32 characterNumbers[4];
    s32 padNumbers[4]; /* Controller pad number copied from each player configuration. */
    s32 outsideGroupZero[4]; /* 0 for group 0 and 1 for other groups. */
    s32 playerRemoved[4]; /* Set when that player has been hit or removed. */
    Point3d hitDirections[4]; /* Outward direction used by the hit-reaction animation. */
    s32 hitUpdateCounts[4]; /* Hit-reaction update count. */
    float hitVerticalOffsets[4]; /* Vertical model offset during the hit reaction, in world units. */
    s32 activePlayerCount; /* Number of players still active in the round. */
    OMOBJ *playerObjects[4];
    u16 previousDirectionButtons; /* Previous direction-button mask used to detect a new press. */
    unsigned char unknown0F6[2];
    s32 winnerFlags[4]; /* Winner flags consumed by the results motion callback. */
    s32 segmentCount; /* Number of live arena segments. */
    s32 arenaPhase; /* Arena animation phase selected by fn_1_70A8. */
    s16 rotatingStageModelId;
    s16 secondRotatingModelId;
    s16 stageIdleMotionId;
    s16 stageAuxiliaryIds[2]; /* Rotation motion at index 0; model handle at index 1. */
    s16 segmentBaseModelId;
    s16 segmentEndpointModelIds[50];
    s16 nozzleModelIds[2];
    s16 firstNozzleMotionIds[4];
    s16 secondNozzleMotionIds[4];
    float arenaYawDegrees; /* Arena rotation angle in degrees, wrapped to one turn. */
    s32 segmentUpdateCountdown; /* Shared segment animation countdown in updates. */
    M633Segment segments[50]; /* Storage for active and reusable arena segments. */
    float nozzleCollisionRadius; /* Current arena collision radius in world units, up to 90. */
    s16 hiddenLoopingModelIds[4]; /* Model IDs initialized hidden with looping motion. */
    M633AI aiStates[4]; /* Per-slot computer-player decisions and targets. */
    u16 aiButtons; /* Button mask supplied to computer-player callbacks. */
    u16 aiPressedButtons; /* Newly pressed button mask supplied to computer-player callbacks. */
    s32 segmentReflectionSoundPending; /* Set when the current update should play the segment sound. */
    s32 aiTurning; /* Enables left/right input until the target heading is nearly aligned. */
    s32 preventArenaRestart; /* Prevents final segment removal from returning the arena motion to phase 0. */
    s32 bgmHandle; /* Active BGM handle, or -1 when no stream is playing. */
    s32 rotationSoundHandle; /* Active sound effect handle, or -1 when none is playing. */
} M633Work;
extern M633Work lbl_1_bss_0;

extern unsigned int lbl_1_data_1E0[16];
extern unsigned int lbl_1_data_220[16];

void fn_1_26B8(OMOBJ *obj);
void fn_1_28A4(OMOBJ *obj);
void fn_1_32C0(OMOBJ *obj);
void fn_1_34B0(OMOBJ *obj);
void fn_1_36BC(OMOBJ *obj);
void fn_1_24EC(s32 soundId, Point3d *pos);
void fn_1_45EC(MGPLAYER *player, M633AI *ai, M633Segment *segment);
void fn_1_4A00(MGPLAYER *player, M633AI *ai, M633Segment *first, M633Segment *second);
void fn_1_51D4(MGPLAYER *player, M633AI *ai);
void fn_1_55D0(MGPLAYER *player, M633AI *ai);
void fn_1_5B20(MGPLAYER *player, Point3d *first, Point3d *second, Point3d *third);
int fn_1_5ED0(Point3d *pos, Point3d *dir);
void fn_1_70A8(s32 phase);
int fn_1_7F40(void);

s32 fn_1_A0(s32 streamHandle, s32 bgmId);
void fn_1_104(s32 streamHandle);
void fn_1_140(void);
void fn_1_224(s16 mode, s16 frameNo);
void fn_1_310(s16 mode, s16 frameNo);
void fn_1_55C(s16 mode, s16 frameNo);
void fn_1_690(s16 mode, s16 frameNo);
void fn_1_AB0(s16 mode, s16 frameNo);
void fn_1_C78(s16 mode, s16 frameNo);
void fn_1_118C(s16 mode, s16 frameNo);
void fn_1_1284(s16 mode, s16 frameNo);
void fn_1_1288(s16 mode, s16 frameNo);
void fn_1_128C(void);
void fn_1_1598(OMOBJ *obj);
void fn_1_258C(void);
void fn_1_26B4(OMOBJ *obj);
void fn_1_34A8(void);
void fn_1_34AC(OMOBJ *obj);
void fn_1_3828(OMOBJ *obj);
void fn_1_382C(void);
void fn_1_3830(s32 playerNo);
void fn_1_38C0(s32 playerNo);
void fn_1_3FFC(s32 playerNo);
s32 fn_1_4260(Point3d *start, Point3d *end, Point3d *pos, Point3d *nearest, float *distance);
s32 fn_1_43D4(M633Segment *segment, Point3d *pos, Point3d *nearest, float *distance);
void fn_1_4598(M633Segment *segment, Point3d *pos);
void fn_1_60F4(OMOBJ *obj);
void fn_1_6DE8(void);
void fn_1_73D8(void);
void fn_1_7E70(s32 motionIndex);
extern MGSEQ_PARAM lbl_1_data_0;
extern char lbl_1_data_28[25];
extern char lbl_1_data_41[25];
extern char lbl_1_data_5A[25];
extern char lbl_1_data_73[25];
extern char lbl_1_data_8C[25];
extern char lbl_1_data_A5[27];
extern char lbl_1_data_C0[];
extern char lbl_1_data_C1[];
extern char lbl_1_data_D2[];
extern char lbl_1_data_E3[];
extern char *lbl_1_data_F4[4];
extern char lbl_1_data_104[27];
extern char lbl_1_data_11F[23];
extern char lbl_1_data_136[25];
extern char lbl_1_data_14F[17];
extern char lbl_1_data_160[23];
extern char lbl_1_data_177[17];
extern char lbl_1_data_188[24];
extern char lbl_1_data_1A0[24];
extern s32 lbl_1_data_1B8[6];
extern s32 lbl_1_data_1D0[4];
extern u32 lbl_1_data_260[4];
extern u32 lbl_1_data_270[4];
extern u32 lbl_1_data_280[4];

#endif /* M633DLL_H */
