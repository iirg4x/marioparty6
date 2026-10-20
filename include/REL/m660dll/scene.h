/* Shared player, model, motion, camera and stage state for Boo'd Off the Stage. */
#ifndef REL_M660_SCENE_H
#define REL_M660_SCENE_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "dolphin/gx.h"
#include "game/object.h"
extern s16 lbl_1_data_F6[5];

typedef struct M660ManagerState {
    OMOBJMAN *objects;
    u32 unknown;
} M660ManagerState;
extern M660ManagerState lbl_1_bss_2678;

typedef struct M660PadInput {
    u32 buttons;
    u32 pressedButtons;
    u16 repeatedButtons;
    s8 stickX;
    s8 stickY;
    s8 substickX;
    s8 substickY;
    u8 triggerL;
    u8 triggerR;
    u8 directionalPad;
    u8 repeatedDirectionalPad;
    u8 alignmentPadding[2]; /* Aligns the end of the sampled input record. */
} M660PadInput;

typedef struct M660Player {
    M660PadInput input;
    s8 selectedPlayer;
    s8 playerIndex;
    s8 character;
    s8 pad;
    s8 group;
    s8 type;
    s8 difficulty;
    s8 inputDisabled; /* Nonzero suppresses sampled input and sends zero pad input to the actor. */
    s8 mode;
    s32 actionState;
    u8 unknown24[12];
    s8 setupState;
    s8 collisionActive;
    u8 positionPadding[2]; /* Aligns the position vector that follows. */
    Vec position;
    Vec previousPosition;
    Vec rotation;
    s8 state;
    u8 meshPadding; /* Aligns the stored mesh indices. */
    s16 storedMesh;
    s16 previousMesh;
    s16 partnerIndex;
    s16 previousPartner;
    s16 collisionCount;
    Vec actionOrigin;
    s16 actionSlot;
    s16 unknown72;
    s32 buttonsWork;
    s32 pressedButtonsWork;
    s32 stickXWork;
    s32 stickYWork;
    s16 followState;
    s16 transitionState;
    s16 movementState;
    s16 routeState;
    s16 movementTimer;
    s16 unknown8E;
    s16 movementCounter;
    s16 chaseState;
    Vec currentPosition;
    Vec previousChasePosition;
    f32 forwardSpeed;
    f32 arrivalRadius;
    Vec movementOrigin;
    u8 unknownC0[4];
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
    s16 unknownF6;
    Vec contactNormals[2];
    s16 selectedRoute;
    s16 selectedPath;
    s16 selectedPoint;
    u8 unknown116[18];
    s16 waitTimer;
    s16 destinationPointIndex;
    s16 routeActive;
    s16 routeRandomRange;
    s16 routeChance;
    s16 routeLifetime;
} M660Player;

typedef struct M660Model {
    s16 modelId;
    u8 unknown02[6]; /* Six bytes before the position vector; their role is unknown. */
    Vec position;
    Vec attachment;
    Vec rotation;
    Vec bounds;
    Vec normals[4];
} M660Model;

typedef struct M660Motion {
    s16 motionIds[7];
    s16 mode;
    s16 previousMode;
    f32 speed;
    f32 previousSpeed;
    f32 time;
    f32 previousTime;
    f32 maximumTime;
    u32 attributes;
    u32 previousAttributes;
    u32 attributesToReset;
    s32 motionEnded;
    s16 resetRequested;
    s16 resetMode;
    f32 transition;
    u32 resetAttributes;
    u32 resetAttributesToClear;
    s16 shiftState;
    s16 shiftMotion;
    u32 shiftFlags;
    f32 shiftStart;
    f32 shiftEnd;
    f32 shiftTime;
    f32 shiftMaximumTime;
    u8 padding60[4]; /* Four unobserved tail bytes preserve the 100-byte record stride;
                     * their role is unknown. */
} M660Motion;

typedef struct M660StageInput {
    u8 unknown00[8];
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
    u8 unknown6C[4];
    s16 night;
    s16 selectedObject;
    u8 unknown74[4];
    f32 angle;
    f32 interval;
    f32 speedRandomRange;
    s16 count;
    s16 historyCount;
    s16 slotHistory[10];
    s16 selectedSlot;
    s16 effectIndex;
    s16 soundPlayed;
    u8 streamIdsPadding[2]; /* Aligns the streamed-audio handles. */
    s32 firstStreamId;
    s32 backgroundTrackId;
    s32 secondStreamId;
    s32 jingleId;
    s32 unknownB4;
    s32 soundEffectId[4];
    s16 soundStatus[4];
    s16 minimumX;
    s16 maximumX;
    f32 leftEdge;
    f32 rightEdge;
    s16 minimumY;
    s16 maximumY;
    f32 topEdge;
    f32 bottomEdge;
} M660StageInput;

typedef struct M660CameraRecord {
    s32 cameraIndex;
    u8 unused04[12];
    Vec rotation;
    Vec position;
    u8 unused28[12];
    f32 zoom;
    f32 viewport[6];
} M660CameraRecord;

extern M660Player lbl_1_bss_2DC[2];
extern M660Model lbl_1_bss_60C[75];
extern M660Motion lbl_1_bss_544[2];
extern M660StageInput lbl_1_bss_1F4;
typedef struct M660CameraState {
    M660CameraRecord records[4];
    u8 unknown140[8];
} M660CameraState;

extern M660CameraState lbl_1_bss_2530;
typedef struct M660Slot {
    s16 active;
    s16 state;
    s16 delay;
    s16 pathIndex;
    s16 effectIndex;
    s16 modelA;
    s16 modelB;
    u8 unknown0E;
    s8 playerIndex;
    Vec velocity;
    f32 travelSpeed;
    f32 unused20;
    f32 phase;
} M660Slot;

typedef struct M660SceneLight {
    u8 unused00[4];
    s16 lightSlot;
    s16 modelId;
    s16 unused08;
    s16 lightId;
    Vec position;
    u8 unused18[12];
    f32 unused24;
    f32 unused28;
    f32 unused2C;
    Vec direction;
    GXColor color;
    f32 referenceDistance;
    f32 referenceBrightness;
} M660SceneLight;

typedef struct M660StageConnection { s8 directions[4]; } M660StageConnection;

extern M660Slot lbl_1_bss_80[9];
extern M660SceneLight lbl_1_bss_38;
extern M660StageConnection lbl_1_bss_0[14];
#endif
