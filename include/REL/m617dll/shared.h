/* Shared state and records used by Light Up My Night module files. */
#ifndef M617DLL_SHARED_H
#define M617DLL_SHARED_H
#include "REL/minigame_particle_models.h"
#include "REL/minigame_course_camera.h"
#include "REL/minigame_participant_motion.h"
#include "REL/minigame_shadow_camera.h"

#include "REL/minigame_light.h"
#include "REL/minigame_pad.h"
#include "dolphin/types.h"
#include "game/hu3d.h"
#include "game/mg/colman.h"
#include "game/mg/actman.h"

typedef struct M617Vector2 {
    f32 x;
    f32 y;
} M617Vector2;

typedef struct M617ScoreBoxEntry {
    s16 spriteId; /* Sprite ID, or score-box ID for the two character panels. */
    s16 groupId; /* Sprite group containing this marker. */
    s16 memberIndex; /* Member slot in that group. */
    u8 unk06[6]; /* No code names this field. */
    M617Vector2 position; /* Screen position for a panel, or group-local marker position. */
    s16 width; /* Score-box width supplied at creation. */
    s16 height; /* Score-box height supplied at creation. */
    f32 unk18; /* The game's code never reads this field. */
    u8 red;
    u8 green;
    u8 blue;
    u8 unk1F; /* The game's code never reads this field. */
    f32 tpLevel; /* Initial opacity sent to the score-box controls. */
    s16 drawPriority; /* Sprite draw priority passed at creation. */
} M617ScoreBoxEntry;

typedef struct M617SpriteGroup {
    s16 groupId; /* Group ID for one player's ten progress-marker sprites. */
    M617Vector2 position; /* Position of the group on screen. */
    M617Vector2 scale; /* Horizontal and vertical scale multipliers. */
} M617SpriteGroup;

typedef struct M617BoardSelection {
    s16 unk00; /* The game's code never reads this field. */
    s16 unk02; /* The game's code never reads this field. */
} M617BoardSelection;

typedef struct M617BoxEdgeLinks {
    s8 links[2][4];
} M617BoxEdgeLinks;

/* State shared by the sequence, controller snapshots, board setup and audio. */
typedef struct M617State {
    u8 unknown00[8]; /* No code names this field. */
    s16 transitionPhase;
    s16 transitionFrame;
    s16 sequencePhase;
    s16 sequenceSelector;
    s16 sequenceFrame;
    SystemPadInput padSnapshots[4];
    struct MgTimer_s *timer;
    s32 timerFrames;
    u8 unk6C[2]; /* No code names this field. */
    s16 unk6E; /* The game's code never reads this field. */
    s16 selection;
    f32 limit;
    s16 layout;
    f32 rotation;
    M617BoardSelection selected[9];
    u8 unkA4[4]; /* No code names this field. */
    s32 firstStream;
    s32 bgmId;
    s32 secondStream;
    s32 jingleId;
    u8 unkB8[4]; /* No code names this field. */
    s32 soundId[3];
    s16 soundActive[3];
    s16 panMinimum;
    s16 panMaximum;
    f32 panPositionMinimum;
    f32 panPositionMaximum;
    s16 volumeMinimum;
    s16 volumeMaximum;
    f32 volumePositionMinimum;
    f32 volumePositionMaximum;
} M617State;

extern M617State lbl_1_bss_9C8;

typedef struct M617RouteEntry {
    s16 state;
    s16 pointCount;
    Vec directions[2];
    Vec positions[2];
} M617RouteEntry;

typedef struct M617RouteGroup {
    f32 value;
    M617RouteEntry entries[4];
} M617RouteGroup;

extern M617RouteGroup lbl_1_bss_38F0[24];

/* Per-player input, motion, route selection and computer-player state. */
typedef struct M617PlayerStateData {
    SystemPadInput input;
    s8 playerIndex;
    s8 participantIndex;
    s8 character;
    s8 controllerPort;
    s8 unk18; /* The game's code never reads this field. */
    s8 computerControlled;
    s8 difficulty;
    s8 motionState;
    s8 resultState;
    s32 completionState;
    u8 unk24[4]; /* No code names this field. */
    s32 actionFrame;
    u8 unk2C[4]; /* No code names this field. */
    s8 unk30; /* The game's code never reads this field. */
    Vec position;
    Vec target;
    Vec rotation;
    s8 transformState;
    u8 unk59[1]; /* No code names this field. */
    s16 collisionMeshIndex;
    s16 unk5C; /* The game's code never reads this field. */
    s16 linkedPlayer;
    s16 previousLinkedPlayer;
    s16 orbitCounter;
    u8 unk64[4]; /* No code names this field. */
    char *itemHook;
    Vec previousPosition;
    Vec snapshotPosition;
    Vec movement;
    s16 rangeIndex;
    f32 upperIntensityLimit[3];
    f32 lowerIntensityLimit[3];
    f32 upperScaleLimit[3];
    f32 lowerScaleLimit[3];
    f32 ambient;
    s16 progress;
    f32 lightRadius;
    f32 lightAngle;
    f32 lightHeight;
    s32 computerButtonsHeld;
    s32 computerButtonsDown;
    s32 computerStick[2];
    s16 controlPhase;
    s16 selectionPhase;
    s16 actionPhase;
    s16 routeMode;
    s16 orbitActive;
    s16 routeStateFrame;
    s16 orbitDelay;
    s16 selectionTimer;
    s16 orbitFrame;
    s16 chasePhase;
    Vec targetPosition;
    Vec targetStep; /* Step along the ground plane, applied to the target's X and Z. */
    f32 chaseSpeed;
    f32 chaseRadius;
    s16 targetObstructed;
    s16 selectedModel;
    s16 collisionModels[2];
    s16 previousCollisionModels[2];
    s16 collisionEdges[2];
    s16 previousCollisionEdges[2];
    Vec collisionDirections[2];
    s16 routeIndex;
    s16 routePoint;
    s16 routeElement;
    f32 orbitAngleStep;
    Vec orbitOrigin;
    s16 stageFrame;
    s16 routeFrame;
    s16 obstacle;
    s16 previousObstacle;
    s16 selectionFrames;
    s16 selectionThreshold;
    s16 selectionDuration;
    s16 retryLimit;
    s16 selectionChanged;
    s16 targetSelectionChance;
    s16 chaseSpeedChance;
    f32 rangeLimits[3];
    s16 selectionDelayFrames;
    s16 elapsedFrames;
} M617PlayerStateData;

extern M617PlayerStateData lbl_1_bss_AB0[2];

typedef struct M617PlayerBindings {
    s16 collisionModel;
    struct MgPlayer_s *players[2];
} M617PlayerBindings;

extern M617PlayerBindings lbl_1_bss_9BC;

typedef struct M617ModelSetupRecord {
    s16 modelId;
    s16 motionState;
    u8 unk04[4]; /* No code names this field. */
    Vec position;
    Vec secondaryPosition;
    Vec rotation;
    Vec dimensions;
    Vec edgeDirections[4];
    f32 scaleStep;
} M617ModelSetupRecord;

typedef struct M617LightSource {
    u8 unknown00[4]; /* No code names this field. */
    s16 lightId;
    s16 modelId;
    s16 modelIndex;
    s16 localLightPoolId;
    Vec position;
    Vec referencePosition;
    Vec rotation;
    Vec direction;
    GXColor color;
    f32 distance;
    f32 intensity;
    f32 intensityStep;
    s16 enabled;
    s16 frame;
    Vec objectPosition;
} M617LightSource;

extern M617LightSource lbl_1_bss_458[15];
extern M617ScoreBoxEntry lbl_1_bss_0[22];
extern M617SpriteGroup lbl_1_bss_370[2];
extern M617ModelSetupRecord lbl_1_bss_E88[96];
extern MinigameParticipantMotionState lbl_1_bss_DC0[2];
extern M617BoxEdgeLinks lbl_1_bss_398[24];
extern struct Process_s *lbl_1_bss_38E4;
extern struct MgSeqParam_s lbl_1_data_0;
typedef struct M617CameraStorage {
    MinigameCourseCamera cameras[4];
    u8 unk140[8]; /* No code names this field. */
} M617CameraStorage;

extern M617CameraStorage lbl_1_bss_379C;
extern LightInitState lbl_1_bss_3760;
extern MinigameShadowCamera lbl_1_bss_3710;
extern MinigameParticleModels lbl_1_bss_3708;
extern s16 lbl_1_data_F6[4];
extern u32 lbl_1_data_118[51];
void ObjectSetup(void);
void fn_1_3E4(void);
void fn_1_730(void);
void fn_1_ECC(void);
void fn_1_24D8(void);
void fn_1_2B48(void);
s32 fn_1_2E94(f32 *value, f32 duration, f32 start, f32 end);
s16 fn_1_3BB8(HU3D_MODELID modelId, HU3D_LIGHTID lightId);
s32 fn_1_3CD0(HU3D_MODELID modelId, HU3D_LIGHTID lightId);
void fn_1_3DE0(void);
void fn_1_4DB0(M617PlayerStateData *work, s16 voiceMode);
void fn_1_535C(void);
void fn_1_54B8(s16 slot, s16 mode, Vec *position);
void fn_1_58DC(s16 slotIndex, s16 pan);
void fn_1_596C(s16 slotIndex, s16 pitch);
void fn_1_59FC(s16 slotIndex, s16 volume);
void fn_1_5C1C(void);
void fn_1_5DB4(void);
void fn_1_7048(void);
void fn_1_78C0(void);
s32 fn_1_93D0(void);
void fn_1_E1F0(void);
void fn_1_16898(void);
void fn_1_1817C(M617PlayerStateData *work);
void fn_1_187C8(void);
void fn_1_18BD4(void);

struct ColNarrowParam_s;
struct MgActor_s;
struct MgTimer_s *fn_1_2FB4(int type, int duration, int end, f32 x, f32 y);
void fn_1_3028(struct MgTimer_s *timer);
void fn_1_307C(struct MgTimer_s *timer);
s16 fn_1_4070(const Vec *position);
s16 fn_1_435C(s16 playerIndex, const Vec *position);
int fn_1_780C(struct ColNarrowParam_s *first, struct ColNarrowParam_s *second);
void fn_1_788C(struct MgActor_s *task);

BOOL fn_1_3054(struct MgTimer_s *timer);

typedef struct {
    f32 height;
    f32 radius;
    s32 param;
    s32 type;
    COL_NARROW_HOOK narrowHook;
    u32 attr;
    s32 correctHookParam;
    MGACTOR_CORRECT_HOOK correctHook;
    MGACTOR_CORRECT_HOOK correctionCallbacks[16];
} M617PlayerSetupParam;

#endif
