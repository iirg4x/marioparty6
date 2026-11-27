/* Shared player, model, motion, camera and stage state for Boo'd Off the Stage. */
#ifndef REL_M660_SCENE_H
#define REL_M660_SCENE_H
#include "REL/minigame_camera_preset.h"
#include "REL/minigame_participant_motion.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "dolphin/gx.h"
#include "game/object.h"
extern s16 lbl_1_data_F6[5];

/* Object manager state for the stage. */
typedef struct M660ManagerState {
    OMOBJMAN *objects;
    u32 unk04; /* No code names this field. */
} M660ManagerState;
extern M660ManagerState lbl_1_bss_2678;

typedef struct M660PadInput {
    u32 unk00; /* The game's code never reads this field. */
    u32 unk04; /* The game's code never reads this field. */
    u16 unk08; /* The game's code never reads this field. */
    s8 unk0A; /* The game's code never reads this field. */
    s8 unk0B; /* The game's code never reads this field. */
    s8 unk0C; /* The game's code never reads this field. */
    s8 unk0D; /* The game's code never reads this field. */
    u8 unk0E; /* The game's code never reads this field. */
    u8 unk0F; /* The game's code never reads this field. */
    u8 unk10; /* The game's code never reads this field. */
    u8 unk11; /* The game's code never reads this field. */
    u8 alignmentPadding[2]; /* Aligns the end of the sampled input record. */
} M660PadInput;

/* Controller input, action and collision state for one player. */
typedef struct M660Player {
    M660PadInput input;
    s8 selectedPlayer;
    s8 playerIndex;
    s8 character;
    s8 pad;
    s8 unk18; /* The game's code never reads this field. */
    s8 type;
    s8 difficulty;
    s8 inputDisabled; /* Nonzero suppresses sampled input and sends zero pad input to the actor. */
    s8 mode;
    s32 actionState;
    u8 unk24[12]; /* No code names this field. */
    s8 hitVoiceRequest;
    s8 collisionActive;
    u8 positionPadding[2]; /* Aligns the position vector that follows. */
    Vec position;
    Vec previousPosition;
    Vec rotation;
    s8 state;
    u8 meshPadding; /* Aligns the stored mesh indices. */
    s16 storedMesh;
    s16 unk5C; /* The game's code never reads this field. */
    s16 partnerIndex;
    s16 previousPartner;
    s16 collisionCount;
    Vec actionOrigin;
    s16 actionSlot;
    s16 unk72; /* No code names this field. */
    s32 buttonsWork;
    s32 pressedButtonsWork;
    s32 stickXWork;
    s32 stickYWork;
    s16 followState;
    s16 transitionState;
    s16 movementState;
    s16 routeState;
    s16 movementTimer;
    s16 unk8E; /* No code names this field. */
    s16 movementCounter;
    s16 chaseState;
    Vec currentPosition;
    Vec previousChasePosition;
    f32 forwardSpeed;
    f32 arrivalRadius;
    Vec movementOrigin;
    u8 unkC0[4]; /* No code names this field. */
    f32 angle;
    f32 steeringX;
    f32 steeringZ;
    f32 distance;
    Vec destination;
    s16 reached;
    s16 arrived;
    s16 external;
    s16 contacts[2];
    s16 oldContacts[2];
    s16 contactEdges[2];
    s16 oldContactEdges[2];
    s16 unkF6; /* No code names this field. */
    Vec unkF8[2]; /* The game's code never reads this field. */
    s16 selectedRoute;
    s16 selectedPath;
    s16 selectedPoint;
    u8 unk116[18]; /* No code names this field. */
    s16 waitTimer;
    s16 destinationPointIndex;
    s16 routeActive;
    s16 routeRandomRange;
    s16 routeChance;
    s16 routeLifetime;
} M660Player;

typedef struct M660Model {
    s16 modelId;
    u8 unk02[6]; /* No code names this field. */
    Vec position;
    Vec attachment;
    Vec rotation;
    Vec bounds;
    Vec normals[4];
} M660Model;

/* The record stride is 100 bytes. */
/* Stage sequence, controller samples and effect state. */
typedef struct M660StageState {
    u8 unk00[8]; /* No code names this field. */
    s16 openingState;
    s16 openingTimer;
    s16 phase;
    s16 resultState;
    s16 timer;
    s16 resultStarted; /* Nonzero after a winner or timeout stops new stage events. */
    s16 tie;
    s16 losingPlayerIndex;
    M660PadInput samples[4];
    struct MgTimer_s *gameTimer;
    u8 unk6C[4]; /* No code names this field. */
    s16 unk70; /* The game's code never reads this field. */
    s16 selectedObject;
    u8 unk74[4]; /* No code names this field. */
    f32 angle;
    f32 interval;
    f32 speedRandomRange;
    s16 count;
    s16 historyCount;
    s16 effectHistory[10];
    s16 selectedSlot;
    s16 effectIndex;
    s16 soundPlayed;
    u8 streamHandlePadding[2]; /* Aligns the streamed-audio handles. */
    s32 backgroundStreamHandle;
    s32 backgroundTrackId;
    s32 jingleStreamHandle;
    s32 jingleId;
    s32 unkB4; /* No code names this field. */
    s32 soundEffectHandle[4];
    s16 soundStatus[4];
    /* Stereo pan mapped from world X. */
    s16 panMinimum;
    s16 panMaximum;
    f32 xMinimum;
    f32 xMaximum;
    /* Effect volume mapped from world Z. */
    s16 volumeMinimum;
    s16 volumeMaximum;
    f32 zMinimum;
    f32 zMaximum;
} M660StageState;

extern M660Player lbl_1_bss_2DC[2];
extern M660Model lbl_1_bss_60C[75];
extern MinigameParticipantMotionState lbl_1_bss_544[2];
extern M660StageState lbl_1_bss_1F4;
typedef struct M660CameraState {
    MinigameCameraPreset records[4];
    u8 unk140[8]; /* No code names this field. */
} M660CameraState;

extern M660CameraState lbl_1_bss_2530;
/* Movement and path state for one moving slot. */
typedef struct M660Slot {
    s16 active;
    s16 unk02; /* The game's code never reads this field. */
    s16 delay;
    s16 pathIndex;
    s16 effectIndex;
    s16 modelA;
    s16 modelB;
    u8 unk0E; /* No code names this field. */
    s8 unk0F; /* No code names this field. */
    Vec velocity;
    f32 travelSpeed;
    f32 unk20; /* No code names this field. */
    f32 phase;
} M660Slot;

typedef struct M660SceneLight {
    u8 unk00[4]; /* No code names this field. */
    s16 lightSlot;
    s16 modelId;
    s16 unk08; /* The game's code never reads this field. */
    s16 lightId;
    Vec position;
    u8 unk18[12]; /* No code names this field. */
    f32 unk24; /* The game's code never reads this field. */
    f32 unk28; /* The game's code never reads this field. */
    f32 unk2C; /* The game's code never reads this field. */
    Vec direction;
    GXColor color;
    f32 referenceDistance;
    f32 referenceBrightness;
} M660SceneLight;

extern M660Slot lbl_1_bss_80[9];
extern M660SceneLight lbl_1_bss_38;
extern s8 lbl_1_bss_0[14][4];
#endif
