/* E. Gadd's Garage board events, routes, capsule stations, and model effects. */
#include "math.h"
#include "game/gamework.h"
#include "game/board/main.h"
#include "game/board/object.h"
#include "game/board/masu.h"
#include "game/board/player.h"
#include "game/board/branch.h"
#include "game/board/capsule.h"
#include "game/board/opening.h"
#include "game/sprite.h"
#include "game/audio.h"
#include "msm_se.h"
#include "game/data.h"
#include "game/memory.h"
#include "string.h"
#include "game/object.h"
#include "game/board/audio.h"
#include "game/board/window.h"
#include "game/board/comchoice.h"
#include "game/board/camera.h"
#include "game/board/coin.h"
#include "dolphin/types.h"
#include "game/process.h"
#include "humath.h"
#include "game/pad.h"
#include "game/charman.h"
#include "game/hu3d.h"
#include "dolphin/ppcarch.h"
#include "game/main.h"
#include "game/frand.h"
#include "game/wipe.h"
#include "dolphin/gx.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/mg/actman.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "types.h"
#include <types.h>

#define W02_RANDOM_MODULUS 268435456
#define W02_RANDOM_MODULUS_U 268435456U
#define W02_RANDOM_FRACTION_SCALE 3.7252903e-09f
#define W02_STAR_SPACE_GROUP_MASK 0x1C0000
#define W02_STAR_SPACE_GROUP_SHIFT 18U
#define W02_BRANCH_MATTR_MASK 0x502
#define W02_MECHANISM_SOUND_ID 1435
#define W02_PATH_TRANSITION_SOUND_ID 1443
#define W02_STAR_SPACE_SOUND_ID 1455
#define W02_BOARD_ROTATION_SOUND_ID 1436
#define W02_WIPE_READY_FLAG_OFFSET 4988
#define W02_WIPE_ACTIVE_FLAG_OFFSET 4992
#define W02_ROTATION_ACTIVE_FLAG_OFFSET 12
#define W02_ROTATION_PROCESS_PRIORITY 8195
#define W02_ROTATION_PROCESS_STACK_SIZE 16384
#define W02_ROUTE_SPACE_ATTR_6_FLAG (1 << 6)
#define W02_ROUTE_SPACE_ATTR_7_FLAG (1 << 7)
#define W02_ROUTE_SPACE_ATTR_8_FLAG (1 << 8)
#define W02_ROUTE_SPACE_ATTR_9_FLAG (1 << 9)
#define W02_ROUTE_SPACE_ATTR_10_FLAG (1 << 10)
#define W02_PATH_TRAVEL_SOUND_ID 1437
#define W02_PATH_ARRIVAL_SOUND_ID 1438
#define W02_PATH_DEPARTURE_SOUND_ID 1445
#define W02_PATH_TRANSFER_SOUND_ID 1446
#define W02_PATH_FLIGHT_SOUND_ID 1447
#define W02_EVENT_PROCESS_STATE_OFFSET 4508
#define W02_PATH_PARTICLE_ALPHA_MAX 255
#define W02_STATION_CHOICE_SOUND_ID 1459
#define W02_STATION_SELECTION_SOUND_ID 1460
#define W02_STATION_EXIT_SOUND_ID 1468
#define W02_STATION_ACCEPT_SOUND_ID 1467
#define W02_CAPSULE_CONFIRM_SOUND_ID 1461
#define W02_ATTACHMENT_START_SOUND_ID 1453
#define W02_ATTACHMENT_END_SOUND_ID 1454
#define W02_PLAYER_EVENT_ATTR_11_FLAG (1 << 11)
#define W02_MATERIAL_ATTR_4_FLAG (1 << 4)
#define W02_MATERIAL_FLAGS_MASK 0x1202
#define W02_ROUTE_ENTRY_DATA_OFFSET 4512
#define W02_ROUTE_ENTRY_COUNT_OFFSET 4548
#define W02_ROUTE_ENTRY_SIZE 12
#define W02_STATION_LINK_ATTR_FLAG (1 << 13)
#define W02_LIFT_SOUND_ID 1458
#define W02_CAPSULE_DROP_SOUND_ID 1439
#define W02_MACHINE_LOOP_SOUND_ID 1440
#define W02_CAPSULE_LAUNCH_SOUND_ID 1441
#define W02_CAPSULE_REWARD_SOUND_ID 1442
#define W02_LIFT_MOVE_SOUND_ID 1457
#define W02_COIN_REWARD_SOUND_ID 18
#define W02_MACHINE_SPIN_SOUND_ID 1464
#define W02_MACHINE_START_SOUND_ID 1463
#define W02_MACHINE_STOP_SOUND_ID 1444
#define W02_PLAYER_MANAGER_ALLOCATION_OFFSET 4632
#define W02_PLAYER_MANAGER_COUNT_OFFSET 4628
#define W02_CAPSULE_WIN_MOTION_NO 12
#define W02_CAPSULE_WIN_EFFECT_ID 579
#define W02_COIN_LOSS_EFFECT_ID 581
#define W02_PLAYER_FLIGHT_PLAYER_OFFSET 4676
#define W02_PLAYER_FLIGHT_STATE_OFFSET 4680
#define W02_PLAYER_FLIGHT_TARGET_OFFSET 4684
#define W02_COIN_MODEL_LIST_OFFSET 4720
#define W02_COIN_MODEL_COUNT_OFFSET 4716
#define W02_COIN_SPRITE_MODEL_OFFSET 4764
#define W02_COIN_SPRITE_IMAGE_OFFSET 4756
#define W02_COIN_SPRITE_STATE_OFFSET 4752
#define W02_COIN_EFFECT_RECORDS_OFFSET 4732
#define W02_COIN_EFFECT_ACTIVE_OFFSET 24
#define W02_COIN_EFFECT_OBJECT_OFFSET 4728
#define W02_CANNON_UNAVAILABLE_SOUND_ID 1466
#define W02_CANNON_CANCEL_SOUND_ID 1470
#define W02_CANNON_READY_SOUND_ID 1448
#define W02_CANNON_LOAD_SOUND_ID 1449
#define W02_CANNON_TURN_SOUND_ID 1450
#define W02_CANNON_TURN_END_SOUND_ID 1451
#define W02_CANNON_FIRE_SOUND_ID 1452
#define W02_CANNON_FINISH_SOUND_ID 1469
#define W02_BOARD_EVENT_STATE_OFFSET 14
#define W02_HOOK_UV_INITIALIZED_FLAG (1 << 4)
#define W02_MODEL_HOOK_DATA_OFFSET 288
#define W02_SPRITE_HOOK_TILE_COORDINATES_OFFSET 80
#define W02_SPRITE_HOOK_CORNERS_OFFSET 84
#define W02_SPRITE_HOOK_ANIMATION_OFFSET 56
#define W02_SPRITE_OBJECT_DATA_OFFSET 92

/* Initializes the mechanical board's models and registers its board event hooks. */

typedef struct ModelEntry {
    int modelId;
    int entryIndex;
    int flightStage;
    int age;
    int travelFrames;
    float jumpHeight;
    float rotationSpeed;
    HuVecF start;
    HuVecF end;
    HuVecF bend;
    HuVecF rotation;
} ModelEntry;

typedef struct ModelTransform {
    Mtx matrix;
    HuVecF position;
} ModelTransform;

typedef struct SpaceBinding {
    s16 spaceId;
    s16 transformIndex;
} SpaceBinding;

typedef struct MapMarker {
    int modelId;
    int spaceId;
    int linkedSpace;
} MapMarker;

typedef struct SpriteEffect {
    int objectId;
    int flightSlot;
    u8 stateData[16];
    BOOL active;
    u8 trajectoryData[140];
} SpriteEffect;

typedef struct FlyingEffect {
    int objectId;
    int state;
    int age;
    int travelFrames;
    int remainingFrames;
    int mode;
    BOOL active;
    HuVecF position;
    HuVecF previousPosition;
    HuVecF rotation;
    HuVecF positionHistory[4];
    HuVecF start;
    HuVecF control;
    HuVecF end;
    float spinAngle;
    float spinSpeedSlope;
    float startDrift;
    float spiralRadius;
    float spinRateScale;
} FlyingEffect;

typedef struct SpriteParticle {
    s16 index;
    s16 state;
    u8 textureData[4];
    HuVecF velocity;
    u8 movementData[8];
    float rotationSpeed;
    u8 animationData[12];
    float animationTime;
    float animationSpeed;
    float activityValue;
    HuVecF position;
    HuVec2f rendering;
    float rotation;
    GXColor color;
    int timer;
} SpriteParticle;

typedef struct SpriteObject {
    int modelId;
    int state;
    int timer;
    ANIMDATA *animation;
} SpriteObject;

typedef struct SpriteHook {
    s16 textureFrame;
    s16 textureIndex;
    u8 renderingData[28];
    s8 index;
    u8 animationData;
    u8 flags;
    u8 hookFlags[3];
    s16 particleCount;
    u32 frameCounter;
    u32 lastUpdate;
    u32 duration;
    u32 displayListSize;
    ANIMDATA *animation;
    SpriteParticle *particles;
    HuVecF *positions;
    HuVec2f *textureCoordinates;
    void *displayList;
    u32 callbackAddress;
    HuVec2f *tileCoordinates;
    void *corners;
    u32 linkedModelAddress;
} SpriteHook;

typedef struct PathSpaces {
    int boardModelId;
    int modelId;
    int spaceCount;
    int modelSpaceCount;
    float length;
    float travelDistance;
    HuVecF end;
    HuVecF start;
    HuVecF markerStart;
    HuVecF positions[32];
    int spaceIds[32];
    int spaceTypes[32];
} PathSpaces;

typedef struct ThrowPath {
    BOOL ready;
    int state;
    int age;
    int travelFrames;
    HuVecF start;
    HuVecF bend;
    HuVecF end;
    HuVecF trailStart;
    HuVecF trailEnd;
} ThrowPath;

typedef struct PositionModel {
    int primaryModel;
    int secondaryModel;
    int spaceCount;
    int spaceState;
    ModelTransform transforms[14];
    SpaceBinding bindings[32];
    PathSpaces paths[5];
    u8 earlierState[4];
    int pathEffectModel;
    int particleModels[4];
    int coinRewardCount;
    MapMarker markers[3];
    int markerCount;
    int markerEffectModels[5];
    int models[3];
    int attachedModel;
    int restorePlayer;
    HSF_MATERIAL *materialBackup;
    HSF_PALETTE *paletteBackup;
    int *objectData;
    int linkedSpaceBaseModel;
    int linkedSpaceBodyModel;
    int eyeModel;
    int attachmentModel;
    int subAttachmentModel;
    int transitionModel;
    OMOBJ *entryObject;
    ModelEntry *entryData;
    int stationLiftBaseModel;
    int motionReferenceModel;
    int modelId;
    int hookedModel;
    int displayModels[4];
    int hiddenModel;
    int emissionFrame;
    int eventPlayerNo;
    int eventTimer;
    int opponentPlayerNo;
    int eventStage;
    HuVecF flightOrigin;
    HuVecF flightDestination;
    OMOBJ *modelObject;
    FlyingEffect *flyingModels;
    BOOL modelsEnabled;
    OMOBJ *coinObject;
    FlyingEffect *flyingCoins;
    BOOL coinsEnabled;
    int leverObjectId;
    int liftObjectId;
    int characterObjectId;
    OMOBJ *effectObject;
    SpriteEffect *effects;
    ANIMDATA *effectAnimation;
    int effectModel;
    float effectScale;
    int parentModel;
    int pairedModel;
    int throwPathIndex;
    int throwPathCount;
    ThrowPath *throwPaths;
    BOOL throwStopped;
    int spaceModels[3];
    int eventModel;
    int mapDisplayModel;
    int starModel;
    int mapModels[5];
    float orbitAngles[5];
    float orbitRadii[5];
    float bobAngles[5];
    float orbitSpeed;
    HuVecF particleOrigins[4];
    int emissionDelays[4];
    int burstFrames[4];
    BOOL particlesDisabled;
    BOOL mapView;
    BOOL transitionActive;
} PositionModel;

extern BOOL mbSaveNewF;

extern HuVecF lbl_1_data_0[3];

extern HuVecF lbl_1_data_24[3];

extern HuVecF lbl_1_data_78[6];

extern HuVecF lbl_1_data_C0[6];

extern char *lbl_1_data_1C0[16];

extern char *lbl_1_data_234[5];

extern char *lbl_1_data_284[6];

extern char *lbl_1_data_2F0[9];

extern char *lbl_1_data_32C[2];

extern char *lbl_1_data_348[2];

extern char *lbl_1_data_418[19];

extern char *lbl_1_data_490[4];

extern char *lbl_1_data_4B8[2];

extern char *lbl_1_data_4D4[2];

extern HuVecF lbl_1_data_4DC;

extern HuVecF lbl_1_data_4E8;

extern float lbl_1_data_4F4;

extern HuVecF lbl_1_data_504;

extern float lbl_1_data_510;

void mbScrollInit(int dataNum);

int mbCapThrowColCreate(int dataNum);

void mbev_ShopExInit(int dataNum, void (*hook)(int modelId, int shopNo));

void mbev_NextTimeSet(void (*hook)(void));

void mbLightFuncSet(void (*setHook)(void), void (*resetHook)(void));

void mbMapHookSet(void (*hook)(BOOL enter));

void mbMapCameraSet(const HuVecF *rotation, const HuVecF *position, float zoom);

void mbStarMoveHookSet(void (*hook)(void));

/* Runs the station's paid capsule launches toward eligible board spaces. */

typedef struct CapsuleShot {
    BOOL ready;
    int playerNo;
    int destinationSpace;
    int travelFrames;
    HuVecF start;
    HuVecF bend;
    HuVecF end;
    HuVecF trailStart;
    HuVecF trailEnd;
} CapsuleShot;

typedef struct CapsuleStation {
    u8 boardState[4584];
    int stationModel;
    u8 attachmentState[184];
    int launchBaseModel;
    int cannonModel;
    int shotIndex;
    int shotCount;
    CapsuleShot *shots;
    BOOL shotsStopped;
    int spaceModels[3];
    int eventModel;
    int mapDisplayModel;
    u8 mapState[168];
    BOOL mapEffectsHidden;
} CapsuleStation;

extern int lbl_1_data_C64[4][5];

extern HuVecF lbl_1_data_CB4;

extern HuVecF lbl_1_data_CC0;

extern HuVecF lbl_1_data_CCC;

int mbBGRead(int dataDir);

void mbBGReadWait(int readStatus);

void mbev_CapVecChase(float progress, HuVecF *start, HuVecF *end, HuVecF *position);

BOOL mbev_CapPlayerCheck(int capsulePlayer, int playerNo);

float mbev_CapAngleWrap(float current, float target);

float mbev_CapAngleSumLerp(float progress, float current, float target);

void mbev_CapVecRotGet(HuVecF *direction, HuVecF *rotation);

void mbev_Scroll(int playerNo, BOOL mapF);

void mbWipeSpecialFadeInCreate(int type, int durationFrames);

void mbWipeSpecialFadeOutCreate(int type, int durationFrames);

void mbWipeDissolveFadeOutTime(int durationFrames);

void mbWipeDissolveFadeInTime(int durationFrames);

int sprintf();

/* Shared state for Garage model flights, route markers, and particle effects. */

typedef struct EventModelEntry {
    s32 modelId;
    s32 entryIndex;
    s32 flightStage;
    s32 age;
    s32 travelFrames;
    float jumpHeight;
    float rotationSpeed;
    HuVecF start;
    HuVecF end;
    HuVecF bend;
    HuVecF rotation;
} EventModelEntry;

typedef struct EventMapMarker {
    s32 modelId;
    s32 spaceId;
    s32 linkedSpace;
} EventMapMarker;

typedef struct EventSpriteEffect {
    s32 objectId;
    s32 flightSlot;
    u8 stateData[16];
    BOOL active;
    u8 trajectoryData[140];
} EventSpriteEffect;

typedef struct EventFlyingEffect {
    s32 objectId;
    s32 state;
    s32 age;
    s32 travelFrames;
    s32 remainingFrames;
    s32 mode;
    BOOL active;
    HuVecF position;
    HuVecF previousPosition;
    HuVecF rotation;
    HuVecF positionHistory[4];
    HuVecF start;
    HuVecF control;
    HuVecF end;
    float spinAngle;
    float spinSpeedSlope;
    float startDrift;
    float spiralRadius;
    float spinRateScale;
} EventFlyingEffect;

typedef struct EventSpriteParticle {
    s16 frame;
    s16 state;
    u8 textureData[4];
    HuVecF velocity;
    u8 movementData[8];
    float rotationSpeed;
    u8 animationData[12];
    float animationTime;
    float animationSpeed;
    float activityValue;
    HuVecF position;
    HuVec2f rendering;
    float rotation;
    GXColor color;
    s32 timer;
} EventSpriteParticle;

typedef struct EventSpriteObject {
    s32 modelId;
    s32 state;
    s32 timer;
    ANIMDATA *animation;
} EventSpriteObject;

typedef struct EventSpriteHook {
    s16 textureFrame;
    s16 textureIndex;
    u8 renderingData[28];
    s8 blendMode;
    u8 animationData;
    u8 flags;
    u8 hookFlags[3];
    s16 particleCount;
    u32 frameCounter;
    u32 lastUpdate;
    u32 duration;
    u32 displayListSize;
    ANIMDATA *animation;
    EventSpriteParticle *particles;
    HuVecF *positions;
    HuVec2f *textureCoordinates;
    void *displayList;
    u32 callbackAddress;
    HuVec2f *tileCoordinates;
    void *corners;
    u32 linkedModelAddress;
} EventSpriteHook;

typedef struct EventPathSpaces {
    s32 state;
    s32 modelId;
    s32 spaceCount;
    s32 modelSpaceCount;
    float length;
    float travelDistance;
    HuVecF end;
    HuVecF start;
    HuVecF markerStart;
    HuVecF positions[32];
    s32 spaceIds[32];
    s32 spaceTypes[32];
} EventPathSpaces;

typedef struct EventThrowPath {
    BOOL ready;
    s32 state;
    s32 age;
    s32 travelFrames;
    HuVecF start;
    HuVecF bend;
    HuVecF end;
    HuVecF trailStart;
    HuVecF trailEnd;
} EventThrowPath;

typedef struct BoardModels {
    s32 primaryModel;
    s32 secondaryModel;
    s32 spaceCount;
    s32 spaceState;
    ModelTransform transforms[14];
    SpaceBinding bindings[32];
    EventPathSpaces paths[5];
    u8 earlierState[8];
    s32 particleModels[4];
    s32 coinRewardCount;
    EventMapMarker markers[3];
    s32 markerCount;
    s32 markerEffectModels[5];
    s32 models[3];
    s32 attachedModel;
    s32 restorePlayer;
    HSF_MATERIAL *materialBackup;
    HSF_PALETTE *paletteBackup;
    s32 *objectData;
    s32 linkedSpaceBaseModel;
    s32 linkedSpaceBodyModel;
    s32 eyeModel;
    s32 attachmentModel;
    s32 subAttachmentModel;
    u8 laterState[4];
    OMOBJ *entryObject;
    EventModelEntry *entryData;
    u8 positionState[8];
    s32 positionModel;
    s32 hookedModel;
    s32 displayModels[4];
    s32 hiddenModel;
    u8 spaceData[20];
    HuVecF flightOrigin;
    HuVecF flightDestination;
    OMOBJ *modelObject;
    EventFlyingEffect *flyingModels;
    BOOL modelsEnabled;
    OMOBJ *coinObject;
    EventFlyingEffect *flyingCoins;
    BOOL coinsEnabled;
    s32 leverObjectId;
    s32 liftObjectId;
    s32 characterObjectId;
    OMOBJ *effectObject;
    EventSpriteEffect *effects;
    ANIMDATA *effectAnimation;
    s32 effectModel;
    float effectScale;
    s32 parentModel;
    s32 pairedModel;
    s32 throwPathIndex;
    s32 throwPathCount;
    EventThrowPath *throwPaths;
    BOOL throwStopped;
    s32 spaceModels[3];
    s32 eventModel;
    s32 mapDisplayModel;
    s32 starModel;
    s32 mapModels[5];
    u8 laterEventState[144];
    BOOL mapView;
} BoardModels;

typedef struct BoardConfiguration {
    u8 configuration;
} BoardConfiguration;

extern HuVecF lbl_1_data_888[5];

extern HuVecF lbl_1_data_8C4[5];

extern HuVecF lbl_1_data_900[4];

extern HuVec2f lbl_1_data_7F4[16];

extern HuVecF lbl_1_data_A9C[3];

extern HuVecF lbl_1_data_AC0[3];

extern char *lbl_1_data_6C0[14];

extern HuVecF lbl_1_data_C10[4];

extern HuVecF lbl_1_data_B94;

float mbev_CapAngleLerp(float target, float current, float step);

float mbSinDeg(float deg);

int mbCapSellCostGet(s16 capsuleNo, s16 playerNo);

void mbMapSprAdd(int type, s16 spaceId);

typedef struct PlayerMove {
    s32 playerNo;
    s32 spaceId;
    s32 state;
    s32 timer;
    s32 returnFrames;
    float moveDistance;
    HuVecF start;
    HuVecF destination;
    s32 motions[3];
} PlayerMove;

typedef struct PlayerTravel {
    s32 playerNo;
    s32 destinationSpace;
    s32 elapsedFrames;
    s32 travelFrames;
    float facingAngle;
    float jumpHeight;
    BOOL upright;
    HuVecF start;
    HuVecF destination;
} PlayerTravel;

typedef struct EffectElement {
    u8 earlierData[52];
    float value;
    HuVecF position;
    u8 elementData[8];
    float rotation;
    GXColor color;
    s32 type;
} EffectElement;

typedef struct EffectData {
    u8 earlierData[34];
    u8 flags;
    u8 animationState[3];
    s16 elementCount;
    u8 laterData[20];
    EffectElement *elements;
} EffectData;

/* Updates the flying board models and coins while their event objects are active. */

typedef struct BoardEffects {
    u8 earlierState[4716];
    OMOBJ *modelObject;
    EventFlyingEffect *models;
    BOOL modelsEnabled;
    OMOBJ *coinObject;
    EventFlyingEffect *coins;
    BOOL coinsEnabled;
} BoardEffects;

extern s32 lbl_1_data_BE0[12];

void mbWipeDissolveFadeIn(void);

s16 mbCoinDispCreate(HuVecF *position, int coinCount, int sign, BOOL playSe);

int mbev_CapCoinDisp(int playerNo, int coinCount, BOOL playResultMotion, BOOL waitF);

typedef struct TransferWork {
    u8 earlierState[4512];
    EventMapMarker markers[3];
    s32 markerCount;
    s32 markerEffectModels[5];
    s32 models[3];
    s32 attachedModel;
} TransferWork;

extern s32 lbl_1_data_A84[3];

extern s32 lbl_1_data_A90[3];

extern HuVecF lbl_1_data_9E0[9];

extern float lbl_1_data_A4C[];

/* Updates space-link destinations and the visibility of their capsule markers. */

typedef struct SpaceGroup {
    s32 state;
    s32 modelId;
    s32 spaceCount;
    u8 displayData[432];
    s32 spaceIds[64];
} SpaceGroup;

typedef struct SpaceEffectWork {
    u8 earlierState[984];
    SpaceGroup groups[5];
} SpaceEffectWork;

typedef struct MarkerParticle {
    u8 renderingData[83];
    u8 alpha;
    u8 colorData[4];
} MarkerParticle;

typedef struct MarkerHook {
    u8 renderingData[60];
    MarkerParticle *particles;
} MarkerHook;

extern s32 lbl_1_data_704[][4];

void mbev_CapPlayerIdleWait(void);

typedef struct SpacePathSpaces {
    s32 state;
    s32 modelId;
    s32 spaceCount;
    s32 modelSpaceCount;
    u8 pathData[428];
    s32 spaceIds[32];
    s32 spaceTypes[32];
} SpacePathSpaces;

typedef struct BoardPaths {
    u8 earlierData[984];
    SpacePathSpaces paths[5];
} BoardPaths;

typedef struct SpaceElement {
    u8 earlierData[52];
    float value;
    HuVecF position;
    u8 elementData[12];
    GXColor color;
    s32 type;
} SpaceElement;

typedef struct SpaceEffect {
    u8 earlierData[38];
    s16 elementCount;
    u8 laterData[20];
    SpaceElement *elements;
} SpaceEffect;

/* Creates the board's sprite models and their update objects. */

typedef struct spriteinitializeSpriteParticle {
    s16 frame;
    s16 state;
    u8 textureData[4];
    HuVecF velocity;
    u8 movementData[8];
    float rotationSpeed;
    u8 animationData[12];
    float animationTime;
    float animationSpeed;
    float activityValue;
    HuVecF position;
    u8 renderingData[8];
    float rotation;
    GXColor color;
    s32 animationFrame;
} spriteinitializeSpriteParticle;

typedef struct spriteinitializeSpriteHook {
    u8 renderingData[32];
    s8 blendMode;
    u8 animationData;
    u8 flags;
    u8 updateFlags;
    u8 hookState[2];
    s16 particleCount;
    u8 renderParameters[20];
    spriteinitializeSpriteParticle *particles;
} spriteinitializeSpriteHook;

typedef struct BoardSpriteModels {
    u8 earlierState[4492];
    s32 models[4];
} BoardSpriteModels;

extern HuVecF lbl_1_data_930[4];

extern float lbl_1_data_960[16][2];

/* Draws the board's sprite particles, building vertices and texture coordinates for their display
 * list. */

typedef struct spriterenderSpriteParticle {
    s16 frame;
    s16 state;
    u8 renderingData[48];
    float size;
    HuVecF position;
    HuVecF rotation;
    GXColor color;
    s32 textureFrame;
} spriterenderSpriteParticle;

typedef struct SpriteRenderHook SpriteRenderHook;

struct SpriteRenderHook {
    s16 textureFrame;
    s16 textureIndex;
    u8 renderingData[28];
    u8 blendMode;
    u8 animationFlags;
    u8 flags;
    u8 hookFlags[3];
    s16 particleCount;
    u32 frame;
    u32 lastDraw;
    u32 duration;
    u32 displayListSize;
    ANIMDATA *animation;
    spriterenderSpriteParticle *particles;
    HuVecF *positions;
    HuVec2f *textureCoordinates;
    void *displayList;
    void (*update)(HU3D_MODEL *, SpriteRenderHook *, Mtx *);
    HuVec2f *tileCoordinates;
    HuVecF *corners;
    HU3D_MODEL *linkedModel;
};

typedef struct StarOrder {
    u8 configuration;
    u8 priorState[4];
    u8 spaceCount;
    u8 order[8];
    u8 position;
} StarOrder;

typedef struct StarDisplayWork {
    u8 earlierState[4816];
    s32 starModel;
} StarDisplayWork;

void mbStarMasuNextSet(int spaceId);

/* Animates the board station's capsule exchange, particles, and coin rewards. */

typedef struct CapsuleList {
    s8 id;
    s8 cost[3];
    s8 weight[12];
} CapsuleList;

typedef struct StationWork {
    u8 boardState[4584];
    int stationModel;
    u8 restoreState[16];
    int machineModels[3];
    int capsuleGateModel;
    u8 cameraState[16];
    int liftBaseModel;
    int liftBodyModel;
} StationWork;

extern HuVecF lbl_1_data_B08;

extern HuVecF lbl_1_data_B14;

extern char *lbl_1_data_B34[2];

extern float lbl_1_data_B3C[2];

int mbCapObjColorCreate(int capsuleNo, BOOL createF);

void mbCapObjColorScaleSet(int id, float x, float y, float z);

void mbCapObjColorPosSetV(int id, HuVecF *position);

void mbCapObjColorKill(int id);

int mbCapListCopy(CapsuleList *list);

void mbCapCapsuleGet(int playerNo, int capsuleNo);

int mbCapUseMesGet(int capsuleNo);

typedef struct textSpriteParticle {
    s16 maxAlpha;
    s16 alphaStep;
    u8 renderingData[48];
    float value;
    HuVecF position;
    HuVecF rotation;
    GXColor color;
    s32 type;
} textSpriteParticle;

typedef struct textSpriteHook {
    u8 renderingData[60];
    textSpriteParticle *particles;
} textSpriteHook;

typedef struct BoardSpriteWork {
    u8 earlierState[4692];
    HuVecF origin;
    HuVecF destination;
    OMOBJ *modelObject;
    EventFlyingEffect *models;
    BOOL modelsEnabled;
    OMOBJ *coinObject;
    EventFlyingEffect *coins;
    BOOL coinsEnabled;
    s32 leverObjectId;
    s32 liftObjectId;
    s32 characterObjectId;
    OMOBJ *effectObject;
    EventFlyingEffect *effects;
    ANIMDATA *effectAnimation;
    s32 effectModel;
    float effectScale;
} BoardSpriteWork;

float mbCosDeg(float angle);

extern GXColor lbl_1_data_648;

extern GXColor lbl_1_data_64C;

typedef struct PathPathSpaces {
    s32 boardModelId;
    s32 modelId;
    s32 spaceCount;
    s32 modelSpaceCount;
    float length;
    float travelDistance;
    HuVecF end;
    HuVecF start;
    HuVecF markerStart;
    HuVecF positions[32];
    s32 spaceIds[32];
    s32 spaceTypes[32];
} PathPathSpaces;

typedef struct PathBoardModels {
    s32 primaryModel;
    s32 secondaryModel;
    s32 spaceCount;
    s32 spaceState;
    ModelTransform transforms[14];
    SpaceBinding bindings[32];
    PathPathSpaces paths[5];
    u8 earlierState[4];
    s32 pathEffectModel;
    s32 particleModels[4];
    s32 coinRewardCount;
    EventMapMarker markers[3];
    s32 markerCount;
    s32 markerEffectModels[5];
    s32 models[3];
    s32 attachedModel;
    s32 restorePlayer;
    HSF_MATERIAL *materialBackup;
    HSF_PALETTE *paletteBackup;
    s32 *objectData;
    s32 linkedSpaceBaseModel;
    s32 linkedSpaceBodyModel;
    s32 eyeModel;
    s32 attachmentModel;
    s32 subAttachmentModel;
    s32 transitionModel;
    OMOBJ *entryObject;
    EventModelEntry *entryData;
    u8 positionState[8];
    s32 positionModel;
    s32 hookedModel;
    s32 displayModels[4];
    s32 hiddenModel;
    s32 emissionFrame;
    u8 spaceData[12];
    s32 eventStage;
    HuVecF flightOrigin;
    HuVecF flightDestination;
    OMOBJ *modelObject;
    EventFlyingEffect *flyingModels;
    BOOL modelsEnabled;
    OMOBJ *coinObject;
    EventFlyingEffect *flyingCoins;
    BOOL coinsEnabled;
    s32 leverObjectId;
    s32 liftObjectId;
    s32 characterObjectId;
    OMOBJ *effectObject;
    EventSpriteEffect *effects;
    ANIMDATA *effectAnimation;
    s32 effectModel;
    float effectScale;
    s32 parentModel;
    s32 pairedModel;
    s32 throwPathIndex;
    s32 throwPathCount;
    EventThrowPath *throwPaths;
    BOOL throwStopped;
    s32 spaceModels[3];
    s32 eventModel;
    s32 mapDisplayModel;
    s32 starModel;
    s32 mapModels[5];
    float orbitAngles[5];
    float orbitRadii[5];
    float bobAngles[5];
    float orbitSpeed;
    HuVecF particleOrigins[4];
    s32 emissionDelays[4];
    s32 burstFrames[4];
    BOOL particlesDisabled;
    BOOL mapView;
    BOOL transitionActive;
} PathBoardModels;

extern HUPROCESS *mbMainProc;

extern HuVecF lbl_1_data_554[4];

extern s32 lbl_1_data_584[4];

extern HuVecF lbl_1_data_594;

extern HuVecF lbl_1_data_5BC;

extern HuVecF lbl_1_data_5C8;

extern float lbl_1_data_5D4;

extern HuVecF lbl_1_data_5D8;

extern HuVecF lbl_1_data_5E4;

extern float lbl_1_data_5F0;

extern HuVecF lbl_1_data_5F4;

extern HuVecF lbl_1_data_600;

extern float lbl_1_data_60C;

extern HuVecF lbl_1_data_610;

extern HuVecF lbl_1_data_61C;

extern float lbl_1_data_628;

extern HuVecF lbl_1_data_62C;

extern HuVecF lbl_1_data_638;

extern float lbl_1_data_644;

void mbTelopTimeChangeCreate(void);

BOOL mbTelopTimeChangeCheck(void);

s16 mbev_CapMasuLinkNextGet(s16 spaceId, BOOL positionAddress);

s16 mbCoinDispCapsuleCreate(HuVecF *position, int coinCount);

typedef struct BoardEntry {
    BOOL active;
    s32 objectId;
    s32 state;
    s32 counter;
    s32 duration;
    s32 playerNo;
    float spinSpeed;
    HuVecF position;
    HuVecF rotation;
    HuVecF velocity;
    HuVecF start;
    HuVecF end;
} BoardEntry;

typedef struct BoardFlightWork {
    u8 earlierState[4692];
    HuVecF origin;
    HuVecF destination;
} BoardFlightWork;

void mbev_CapBezierGetV(float time, HuVecF *start, HuVecF *control,
                      HuVecF *end, HuVecF *position);

/* Builds throw paths one at a time for the board's pending capsule flights. */

typedef struct throwpathsThrowPath {
    BOOL ready;
    s32 state;
    s32 age;
    s32 travelFrames;
    HuVecF start;
    HuVecF bend;
    HuVecF end;
    u8 flightData[24];
} throwpathsThrowPath;

typedef struct ThrowPathWork {
    u8 earlierState[4780];
    s32 pathIndex;
    s32 pathCount;
    throwpathsThrowPath *paths;
} ThrowPathWork;

void mbCapEffThrowCreate(HuVecF *start, HuVecF *end);

BOOL mbCapEffThrowCheck(HuVecF *bend, int *travelFrames);

extern HUPROCESS *mbObjMan;

extern HUPROCESS *mbObjMan;

extern void (*lbl_1_data_874[5])(void);

extern HUPROCESS *mbMainProc;

s32 mbGuideSpeakerNoGet();

extern HuVecF lbl_1_data_BA8;

extern HuVecF lbl_1_data_D44[4];

extern HuVec2f lbl_1_data_D74[4];

typedef struct TextureGridHook {
    u8 renderingData[32];
    s8 blendMode;
    u8 animationData;
    u8 flags;
    u8 hookFlags[3];
    s16 particleCount;
    u8 renderParameters[20];
    SpriteParticle *particles;
    HuVecF *positions;
    HuVec2f *textureCoordinates;
    void *displayList;
    u32 callbackAddress;
    HuVec2f *tileCoordinates;
} TextureGridHook;

s16 lbl_1_bss_5C;

OMOBJ *lbl_1_bss_4C[4];

s32 lbl_1_bss_3C[4];

s32 lbl_1_bss_2C[4];

HUPROCESS *lbl_1_bss_18[5];

s16 lbl_1_bss_14;

OMOBJ *lbl_1_bss_10;

OMOBJ *lbl_1_bss_C;

void *lbl_1_bss_8;

StarOrder *lbl_1_bss_4;

OMOBJ *lbl_1_bss_0;

void fn_1_F4(void);

void fn_1_13708(int playerNo, int spaceId);

s16 fn_1_15E30(ANIMDATA *animation, s16 capacity);

s32 fn_1_D4D0(HuVecF *start, HuVecF *end);

void fn_1_10D20(EventFlyingEffect *effect, HuVecF *position, float scale, float spread);

void fn_1_150F0(OMOBJ *object);

void fn_1_6E28(s32 pathIndex, s32 *spaceIds, BOOL createMarkers);

void fn_1_3CC8(void);

void fn_1_4494(void);

s32 fn_1_39F4(s32 player, s16 spaceId);

void fn_1_A554(s32 playerNo);

void fn_1_A7BC(s32 playerNo);

void fn_1_7FD4(OMOBJ *object);

void fn_1_89A8(void);

void fn_1_AD84(void);

void fn_1_DD5C(void);

void fn_1_B11C(s32 playerNo);

void fn_1_B334(s32 playerNo);

void fn_1_A914(void);

void fn_1_AB78(s32 index);

void fn_1_AC34(s32 index);

void fn_1_ACF4(s32 index);

void fn_1_D410(void);

void fn_1_D8EC(OMOBJ *object);

void fn_1_774C(OMOBJ *object);

void fn_1_FF6C(HuVecF *position);

void fn_1_11904(void);

void fn_1_135B0(void);

OMOBJ *fn_1_12AC8(s32 playerNo, HuVecF *eventPosition);

s32 fn_1_126AC(HuVecF *position, s32 shortFlight);

void fn_1_AA04(s32 index);

s32 fn_1_176E4(HuVecF *position, HuVecF *velocity, float activityValue,
    float rotationSpeed, float animationSpeed, GXColor *color);

void fn_1_3B44(s32 mapView);

void fn_1_10954(void);

s32 fn_1_10AC0(HuVecF *position, s32 shortFlight);

void fn_1_1119C(OMOBJ *object);

void fn_1_128A8(OMOBJ *object);

void fn_1_E074(s32 playerNo);

void fn_1_FEB4(HuVecF *firstPosition, HuVecF *secondPosition);

void fn_1_8C9C(int playerNo, int spaceId);

void fn_1_7A4C(s32 playerNo, s32 destinationSpace);

s32 fn_1_387C(s32 playerNo, s16 spaceId);

void fn_1_45A4(void);

void fn_1_7418(OMOBJ *object);

void fn_1_71B4(s32 pathIndex);

void fn_1_7280(s32 pathIndex);

void fn_1_7548(s32 pathIndex);

void fn_1_1735C(void);

void fn_1_17474(OMOBJ *object);

void fn_1_762C(void);

void fn_1_16300(HU3D_MODEL *model, Mtx *matrix);

void fn_1_1557C(void);

void fn_1_1560C(void);

void fn_1_15794(void);

void fn_1_159FC(s32 previousSpace);

void fn_1_B570(s32 playerNo, s32 action, s32 rewardCount);

void fn_1_11A54(OMOBJ *object);

void fn_1_12030(void);

void fn_1_120B4(HuVecF *position, s32 shortFlight);

void fn_1_172E8(s16 modelId);

void fn_1_17894(HuVecF *origin, HuVecF *angles, s32 count);

void fn_1_3754(void);

void fn_1_37C0(void);

void fn_1_3878(void);

s32 fn_1_39E4(void);

s32 fn_1_39EC(void);

s32 fn_1_3AF4(void);

void fn_1_3AFC(void);

void fn_1_1BD8(OMOBJ *object);

void fn_1_31D8(void);

void fn_1_3FB8(void);

void fn_1_4020(void);

void fn_1_471C(void);

void fn_1_4FB0(void);

void fn_1_544C(void);

void fn_1_58E8(void);

void fn_1_5D6C(void);

void fn_1_6378(void);

void fn_1_100A8(s32 playerNo, s32 emissionState, s32 targetPlayer);

void fn_1_12CB8(OMOBJ *object);

void fn_1_112B4(EventFlyingEffect *effect);

void fn_1_16E84(s16 modelId, s32 columns, s32 rows, s32 alternate);

s32 fn_1_8338(void);

void fn_1_838C(void);

void fn_1_8440(OMOBJ *object);

void fn_1_A0(void);

void fn_1_14F9C(OMOBJ *object);

s32 fn_1_1769C(void);

void fn_1_16E44(s16 modelId, void *corners, void *tileCoordinates);

s32 fn_1_15DE4(s32 spaceId);

void fn_1_154E8(void);

s32 fn_1_129C0(void);

void fn_1_12A2C(void);

void fn_1_125C0(void);

s32 fn_1_117E8(void);

void fn_1_11868(void);

void fn_1_10034(OMOBJ *obj);

s32 fn_1_DC70(void);

void fn_1_DCF0(void);

s32 fn_1_A4B4(s32 needle);

s32 fn_1_A51C(s32 index);

s32 fn_1_874C(void);

void fn_1_87D0(HuVecF *start, HuVecF *velocity, s32 playerNo, s32 duration);

void fn_1_7814(void);

s32 fn_1_786C(s32 *playerList, s32 attributeMask);

s32 fn_1_6DCC(void);

void fn_1_6CA0(void);

void fn_1_6CFC(s32 processSlot);

void fn_1_6D68(void);

s32 fn_1_46D0(s32 spaceId);

s32 fn_1_4470(void);

void fn_1_30F4(void);

void fn_1_312C(s32 parentModel);

void fn_1_1B70(void);

void fn_1_17688(void);

void fn_1_8738(void);

/* Selects the day or night archive while retaining the asset's file number. */
static inline int BoardResourceGet(int dataNum) {
    dataNum = FILENUM(dataNum);
    if (!GwSystem.curTime) {
        dataNum |= DATA_w02;
    } else {
        dataNum |= DATA_w02n;
    }
    return dataNum;
}

/* Reserves the board's work record before its models are initialized. */
static inline void *BoardWorkAllocate(void) {
    return HuMemDirectMallocNum(HEAP_HEAP, sizeof(PositionModel), HU_MEMNUM_OVL);
}

/* Selects the Garage day or night archive for model data requested by board setup. */
static inline int BoardModelDataNum(int dataNum) {
    dataNum = FILENUM(dataNum);
    if (!GwSystem.curTime) {
        dataNum |= DATA_w02;
    } else {
        dataNum |= DATA_w02n;
    }
    return dataNum;
}

static inline void *SpriteAllocate(s32 size, u32 allocationNo) {
    return HuMemDirectMallocNum(HEAP_MODEL, size, allocationNo);
}

/* Copies a sprite particle's position or rotation vector while advancing its trail. */
static inline void CopySpriteVector(HuVecF *destination, HuVecF *source) {
    float x;
    float y;
    float z;
    x = source->x;
    y = source->y;
    z = source->z;
    destination->x = x;
    destination->y = y;
    destination->z = z;
}

/* Initializes a free particle with its position, velocity, color, and animation speeds. */
static inline s32 EmitSpriteParticle(HuVecF *position, HuVecF *velocity, float activityValue,
    float rotationSpeed, float animationSpeed, GXColor *color) {
    EventSpriteParticle *particle;
    EventSpriteHook *hook;
    EventSpriteObject *work;
    s32 index;
    OMOBJ *object;
    HU3D_MODEL *model;

    object = lbl_1_bss_C;
    work = object->data;
    model = &Hu3DData[work->modelId];
    hook = model->hookData;
    particle = hook->particles;
    for (index = 0; index < hook->particleCount; index++, particle++) {
        if (particle->activityValue <= 0.0f) {
            break;
        }
    }
    if (index >= hook->particleCount) {
        return -1;
    }
    particle->frame = particle->state = 0;
    particle->position.x = position->x;
    particle->position.y = position->y;
    particle->position.z = position->z;
    particle->velocity.x = velocity->x;
    particle->velocity.y = velocity->y;
    particle->velocity.z = velocity->z;
    particle->rotationSpeed = rotationSpeed;
    particle->activityValue = activityValue;
    particle->color = *color;
    particle->rotation = 360.0f * (W02_RANDOM_FRACTION_SCALE * mbRandMod(W02_RANDOM_MODULUS));
    particle->timer = 0;
    particle->frame = 0;
    particle->animationTime = 0.0f;
    particle->animationSpeed = animationSpeed;
    work->state++;
    return index;
}

static inline float PlayerSinDeg(float angle) {
    return mbSinDeg(angle);
}

static inline float FlightSinDeg(float angle) {
    return mbSinDeg(angle);
}

static inline float FlightCosDeg(float angle) {
    return mbCosDeg(angle);
}

/* Copies a recent flight position while updating a sprite effect's trail. */
static inline void CopyFlightPosition(HuVecF *destination, const HuVecF *source) {
    float x;
    float y;
    float z;
    x = source->x;
    y = source->y;
    z = source->z;
    destination->x = x;
    destination->y = y;
    destination->z = z;
}
HuVecF lbl_1_data_0[3] = { { 1790.0f, 0.0f, -1960.0f },
                           { 1455.0f, 0.0f, -2145.0f },
                           { 1275.0f, 0.0f, -2480.0f } };

HuVecF lbl_1_data_24[3] = { { 0.0f, -12.0f, 0.0f },
                            { 0.0f, -45.0f, 0.0f },
                            { 0.0f, -78.0f, 0.0f } };

HuVecF lbl_1_data_48[4] = { { -1010.0f, -395.0f, 382.0f },
                            { -2825.0f, -395.0f, 285.0f },
                            { 0.0f, 60.0f, 0.0f },
                            { 0.0f, 160.0f, 0.0f } };

HuVecF lbl_1_data_78[6] = { { -3840.0f, 470.0f, -4105.0f },  { -3840.0f, 1690.0f, -4535.0f },
                            { -3840.0f, 1690.0f, -4535.0f }, { -2735.0f, 1690.0f, -5795.0f },
                            { -2735.0f, 1690.0f, -5795.0f }, { -1825.0f, 470.0f, -5795.0f } };

HuVecF lbl_1_data_C0[6] = {
    { 0.0f, 0.0f, 0.0f },    { 90.0f, 180.0f, 0.0f }, { 90.0f, 0.0f, 0.0f },
    { 90.0f, -90.0f, 0.0f }, { 90.0f, 90.0f, 0.0f },  { 0.0f, 90.0f, 0.0f }
};

char * lbl_1_data_1C0[16] = {
    "arrowhook1", "arrowhook2", "arrowhook3", "arrowhook4",
    "arrowhook5", "arrowhook6", "arrowhook7", "arrowhook8",
    "arrowhook9", "arrowhook10", "arrowhook11", "arrowhook12",
    "arrowhook13", "arrowhook14", "arrowhook15", "arrowhook16"
};

char * lbl_1_data_234[5] = { "sakuhook1", "sakuhook2", "sakuhook3", "sakuhook4", "sakuhook5" };

char * lbl_1_data_284[6] = {
    "belthook1", "belthook2", "belthook3", "belthook4", "belthook5", "belthook6"
};

char * lbl_1_data_2F0[9] = {
    "stfhook1", "stfhook2", "stfhook3", "stfhook4", "stfhook5", "stfhook6", "stfhook7", "stfhook8",
    "stfhook9"
};

char * lbl_1_data_32C[2] = { "planthook1", "planthook2" };

char * lbl_1_data_348[2] = { "tankhook1", "tankhook2" };

char * lbl_1_data_418[19] = {
    "pipehook1", "pipehook2", "pipehook3", "pipehook4",
    "pipehook5", "pipehook6", "pipehook7", "pipehook8",
    "pipehook9", "pipehook10", "pipehook11", "pipehook12",
    "pipehook13", "pipehook14", "pipehook15", "pipehook16",
    "pipehook17", "pipehook18", "pipehook19"
};

char * lbl_1_data_490[4] = { "steamhook1", "steamhook2", "steamhook3", "steamhook4" };

char * lbl_1_data_4B8[2] = { "flifthook1", "flifthook2" };

char * lbl_1_data_4D4[2] = { "machook1", "machook2" };

HuVecF lbl_1_data_4DC = { -53.0f, 0.0f, 0.0f };

HuVecF lbl_1_data_4E8 = { 28.0f, -160.0f, -3094.0f };

float lbl_1_data_4F4 = 13139.0f;

HuVecF lbl_1_data_4F8 = { 282.0f, 0.0f, 0.0f };

HuVecF lbl_1_data_504 = { 0.0f, 544.0f, -2233.0f };

float lbl_1_data_510 = 14778.0f;

void fn_1_A0(void) {
    GWPartySet(TRUE);
    mbObjectSetup(1, fn_1_F4, fn_1_1B70);
}

/* Builds the board scenery, mechanical attachments and animated map decorations at board setup. */
void fn_1_F4(void) {
    Mtx hookMatrix;
    int boardNo;
    s16 backgroundModel;
    PositionModel *work;
    int index;
    int modelId;
    float scale;

    /* The board number is fetched but is not used by this setup routine. */
    boardNo = MBBoardNoGet();
    HuAudSndGrpSetSet(23);
    mbMasuInit(BoardResourceGet(DATANUM(DATA_w02, 0)));
    lbl_1_bss_4 = (StarOrder *) GwSystem.boardWork;
    if (mbSaveNewF) {
        lbl_1_bss_4->configuration = 0;
        lbl_1_bss_4->priorState[0] = lbl_1_bss_4->priorState[1] = lbl_1_bss_4->priorState[2] = 0;
        lbl_1_bss_4->priorState[3] = 0;
        fn_1_154E8();
        fn_1_159FC(-1);
    }
    work = lbl_1_bss_8 = BoardWorkAllocate();
    memset(work, 0, sizeof(PositionModel));
    for (index = 0; index < 4; index++) {
        lbl_1_bss_3C[index] = -1;
    }
    for (index = 0; index < 4; index++) {
        lbl_1_bss_2C[index] = -1;
    }
    work->restorePlayer = -1;
    work->materialBackup = NULL;
    work->paletteBackup = NULL;
    work->objectData = NULL;
    work->effectObject = NULL;
    work->effects = NULL;
    work->effectModel = -1;
    work->effectAnimation = HuSprAnimRead(HuDataSelHeapReadNum(
        BoardResourceGet(DATANUM(DATA_w02, 57)), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_5C = mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 3)), NULL, FALSE);
    mbObjPosSet(lbl_1_bss_5C, 0.0f, 0.0f, 0.0f);
    backgroundModel = lbl_1_bss_5C;
    mbObjAttrSet(backgroundModel, HU3D_MOTATTR_LOOP);
    mbObjCullRadiusSet(lbl_1_bss_5C, -1.0f);
    mbScrollInit(BoardResourceGet(DATANUM(DATA_w02, 1)));
    mbCapThrowColCreate(BoardResourceGet(DATANUM(DATA_w02, 2)));
    fn_1_3CC8();
    fn_1_89A8();
    fn_1_DD5C();
    fn_1_AD84();
    fn_1_471C();
    fn_1_135B0();
    mbev_ShopExInit(BoardResourceGet(DATANUM(DATA_w02, 4)),
        (void (*)(int, int)) fn_1_312C);
    mbev_NextTimeSet(fn_1_31D8);
    work->mapView = FALSE;
    work->transitionActive = FALSE;
    mbLightFuncSet(fn_1_37C0, fn_1_3878);
    modelId = mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 5)), NULL, FALSE);
    for (index = 0; index < 16; index++) {
        mbObjHookSet(lbl_1_bss_5C, lbl_1_data_1C0[index], modelId);
    }
    work->eventModel = modelId = mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 6)), NULL, FALSE);
    mbObjMotionSpeedSet(modelId, 0.0f);
    modelId = mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 9)), NULL, FALSE);
    for (index = 0; index < 5; index++) {
        mbObjHookSet(lbl_1_bss_5C, lbl_1_data_234[index], modelId);
    }
    modelId = mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 11)), NULL, FALSE);
    for (index = 0; index < 6; index++) {
        mbObjHookSet(lbl_1_bss_5C, lbl_1_data_284[index], modelId);
    }
    modelId = mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 12)), NULL, FALSE);
    for (index = 0; index < 9; index++) {
        mbObjHookSet(lbl_1_bss_5C, lbl_1_data_2F0[index], modelId);
    }
    modelId = mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 13)), NULL, FALSE);
    for (index = 0; index < 2; index++) {
        mbObjHookSet(lbl_1_bss_5C, lbl_1_data_32C[index], modelId);
    }
    modelId = mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 14)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    for (index = 0; index < 2; index++) {
        mbObjHookSet(lbl_1_bss_5C, lbl_1_data_348[index], modelId);
    }
    modelId = mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 15)), NULL, FALSE);
    for (index = 0; index < 19; index++) {
        mbObjHookSet(lbl_1_bss_5C, lbl_1_data_418[index], modelId);
    }
    for (index = 0; index < 3; index++) {
        work->spaceModels[index] = modelId =
            mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 8)), NULL, TRUE);
        mbObjMotionSpeedSet(modelId, 0.0f);
        mbObjPosSetV(modelId, &lbl_1_data_0[index]);
        mbObjRotSetV(modelId, &lbl_1_data_24[index]);
    }
    modelId = mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 10)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    for (index = 0; index < 2; index++) {
        mbObjHookSet(lbl_1_bss_5C, lbl_1_data_4B8[index], modelId);
    }
    modelId = mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 18)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    for (index = 0; index < 2; index++) {
        mbObjHookSet(lbl_1_bss_5C, lbl_1_data_4D4[index], modelId);
    }
    for (index = 0; index < 6; index++) {
        modelId = mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 7)), NULL, TRUE);
        mbObjPosSetV(modelId, &lbl_1_data_78[index]);
        mbObjRotSetV(modelId, &lbl_1_data_C0[index]);
    }
    modelId = mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 16)), NULL, FALSE);
    mbObjLayerSet(modelId, 1);
    work->mapDisplayModel = modelId =
        mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 48)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjDispSet(modelId, FALSE);
    modelId = mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 49)), NULL, FALSE);
    mbObjLayerSet(modelId, 3);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjHookSet(modelId, "dronehook1", work->mapDisplayModel);
    mbObjHookSet(modelId, "dronehook2", work->mapDisplayModel);
    mbObjHookSet(modelId, "dronehook3", work->mapDisplayModel);
    work->starModel = modelId = mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 19)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjDispSet(modelId, FALSE);
    for (index = 0; index < 5; index++) {
        work->mapModels[index] = modelId =
            mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 22)), NULL, TRUE);
        mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
        mbObjMotionTimeSet(modelId,
            mbObjMotionMaxTimeGet(modelId) * MBCapsuleEffRandF());
        mbObjMotionSpeedSet(modelId, 0.8f + (4.0f * (-0.5f + MBCapsuleEffRandF())));
        if (index == 0) {
            mbObjScaleSet(modelId, 1.0f, 1.0f, 1.0f);
            work->orbitAngles[index] = 360.0f * MBCapsuleEffRandF();
        } else {
            scale = 0.5f + (0.2f * MBCapsuleEffRandF());
            mbObjScaleSet(modelId, scale, scale, scale);
            work->orbitAngles[index] = 20.0f + work->orbitAngles[index - 1] +
                (5.0f * MBCapsuleEffRandF());
        }
        if (GwSystem.curTime) {
            scale = (0.1f * (float) index) +
                (0.3f + (0.05f * MBCapsuleEffRandF()));
            mbObjScaleSet(modelId, scale, scale, scale);
        }
        work->orbitRadii[index] = 650.0f + (50.0f * (-0.5f + MBCapsuleEffRandF()));
        work->bobAngles[index] = 0.0f;
    }
    work->orbitSpeed = -0.3f;
    Hu3DMotionCalc(mbObjModelIDGet(lbl_1_bss_5C));
    for (index = 0; index < 4; index++) {
        Hu3DModelObjMtxGet(mbObjModelIDGet(lbl_1_bss_5C), lbl_1_data_490[index], hookMatrix);
        work->particleOrigins[index].x = hookMatrix[0][3];
        work->particleOrigins[index].y = hookMatrix[1][3];
        work->particleOrigins[index].z = hookMatrix[2][3];
        if (!GwSystem.curTime) {
            work->emissionDelays[index] = 60.0f * (1.0f + (0.5f * MBCapsuleEffRandF()));
        } else {
            work->emissionDelays[index] = 60.0f * (2.0f + MBCapsuleEffRandF());
        }
        if (!GwSystem.curTime) {
            work->burstFrames[index] = 60.0f * (0.5f + (0.2f * MBCapsuleEffRandF()));
        } else {
            work->burstFrames[index] = 60.0f * (0.2f + (0.1f * MBCapsuleEffRandF()));
        }
    }
    work->particlesDisabled = TRUE;
    fn_1_A914();
    work->transitionModel = modelId =
        mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 40)), NULL, FALSE);
    if (!GwSystem.curTime) {
        mbObjMotionTimeSet(modelId, 0.0f);
    } else {
        mbObjMotionTimeSet(modelId, mbObjMotionMaxTimeGet(modelId) / 2.0f);
    }
    mbObjMotionSpeedSet(modelId, 0.0f);
    work->stationLiftBaseModel = modelId =
        mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 50)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjLayerSet(modelId, 3);
    mbObjDispSet(modelId, FALSE);
    work->motionReferenceModel = modelId =
        mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 51)), NULL, FALSE);
    mbObjHookSet(work->stationLiftBaseModel, "leverhook", modelId);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjLayerSet(modelId, 3);
    work->leverObjectId = modelId =
        mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 52)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjLayerSet(modelId, 3);
    mbObjDispSet(modelId, FALSE);
    work->liftObjectId = modelId =
        mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 53)), NULL, FALSE);
    mbObjHookSet(work->leverObjectId, "lever2hook", modelId);
    mbObjLayerSet(modelId, 3);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    work->characterObjectId = modelId =
        mbObjCreate(BoardResourceGet(DATANUM(DATA_w02, 54)), NULL, FALSE);
    mbObjHookSet(work->liftObjectId, "lifthook", modelId);
    mbObjLayerSet(modelId, 3);
    mbObjMotionSpeedSet(modelId, 0.0f);
    fn_1_1735C();
    mbBranchMAttrSet(W02_BRANCH_MATTR_MASK);
    HuDataDirClose(BoardResourceGet(DATANUM(DATA_w02, 0)));
    mbev_MasuMoveStartSet((MASUEVENTHOOK) fn_1_39EC);
    mbev_MasuMoveEndSet((MASUEVENTHOOK) fn_1_39F4);
    mbev_MasuHatenaSet((MASUEVENTHOOK) fn_1_387C);
    mbev_MasuLinkTblHookSet((MASUPATHCHECKHOOK) fn_1_3AF4);
    mbPlayerTurnInitHookSet((void (*)(int)) fn_1_3AFC);
    mbMapHookSet((void (*)(BOOL)) fn_1_3B44);
    mbOpeningInstHookSet(fn_1_30F4);
    mbOpeningStarInstHookSet(NULL);
    mbStarMoveHookSet(fn_1_15794);
    fn_1_1557C();
    lbl_1_bss_0 = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_1BD8);
    mbOpeningViewSet(&lbl_1_data_4DC, &lbl_1_data_4E8, lbl_1_data_4F4);
    mbMapCameraSet(NULL, &lbl_1_data_504, lbl_1_data_510);
}

/* Releases the board background model and capsule-throw collision data during board shutdown. */
void fn_1_1B70(void) {
    if (lbl_1_bss_5C >= 0) {
        mbObjKill(lbl_1_bss_5C);
        lbl_1_bss_5C = -1;
    }
    mbCapThrowColCreate(-1);
    lbl_1_bss_0 = 0;
}
HuVecF lbl_1_data_554[4] = { { -50.0f, 0.0f, 100.0f },
                             { -40.0f, 100.0f, 10.0f },
                             { 0.0f, 100.0f, 0.0f },
                             { 0.0f, 0.0f, 100.0f } };

s32 lbl_1_data_584[4] = { 5, 3, 2, 2 };

HuVecF lbl_1_data_594 = { 0.0f, -70.0f, -2950.0f };

/* Updates piston spaces, timed steam emitters, orbiting models, and transition input. */
void fn_1_1BD8(OMOBJ *object) {
    Mtx motionMatrix;
    Mtx spaceMatrix;
    HuVecF position;
    HuVecF velocity;
    HuVecF pistonPosition;
    HuVecF pistonVelocity;
    HuVecF emitterPosition;
    HuVecF emitterVelocity;
    GXColor color;
    GXColor pistonColor;
    GXColor emitterColor;
    PathBoardModels *work;
    s32 index;
    s32 particleIndex;
    u32 group;
    GXColor *pistonColorPtr;
    HuVecF *pistonVelocityPtr;
    HuVecF *pistonPositionPtr;
    GXColor *emitterColorPtr;
    HuVecF *emitterVelocityPtr;
    HuVecF *emitterPositionPtr;
    float brightness;

    work = lbl_1_bss_8;
    if (mbExitCheck() || lbl_1_bss_0 == NULL) {
        omDelObjEx(mbObjMan, object);
        lbl_1_bss_0 = NULL;
        return;
    }
    for (index = 1; index < mbMasuNumGet(); index++) {
        if (mbMasuMAttrGet(index) & 0xC000) {
            group = (mbMasuMAttrGet(index) & 0xC000) >> 14;
            Hu3DModelObjMtxGet(mbObjModelIDGet(work->spaceModels[group - 1]),
                "pistonhook", motionMatrix);
            motionMatrix[1][3] += 10.0f;
            PSMTXIdentity(spaceMatrix);
            spaceMatrix[0][3] = motionMatrix[0][3];
            spaceMatrix[1][3] = motionMatrix[1][3];
            spaceMatrix[2][3] = motionMatrix[2][3];
            mbMasuMtxSet(index, spaceMatrix);
            if (mbObjMotionSpeedGet(work->spaceModels[group - 1]) > 0.01f &&
                mbObjMotionTimeGet(work->spaceModels[group - 1]) < 20.0f) {
                if (1.0f == mbObjMotionTimeGet(work->spaceModels[group - 1])) {
                    mbAudFXPlay(W02_MECHANISM_SOUND_ID);
                }
                Hu3DModelObjMtxGet(mbObjModelIDGet(work->spaceModels[group - 1]),
                    "steamhook", motionMatrix);
                for (particleIndex = 0; particleIndex < 2; particleIndex++) {
                    position.x = motionMatrix[0][3];
                    position.y = motionMatrix[1][3];
                    position.z = motionMatrix[2][3];
                    velocity.x = 100.0f * (0.01f + 0.05f * (-0.5f +
                        W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                    velocity.y = 100.0f * (0.2f + 0.2f *
                        (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                    velocity.z = 100.0f * (0.01f + 0.05f * (-0.5f +
                        W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                    brightness = W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS);
                    color.r = 192.0f + 32.0f * brightness;
                    color.g = 192.0f + 32.0f * brightness;
                    color.b = 192.0f + 32.0f * brightness;
                    color.a = 127.0f + 63.0f *
                        (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
                    pistonColor = color;
                    pistonColorPtr = &pistonColor;
                    pistonVelocity = velocity;
                    pistonVelocityPtr = &pistonVelocity;
                    pistonPosition = position;
                    pistonPositionPtr = &pistonPosition;
                    fn_1_176E4(pistonPositionPtr, pistonVelocityPtr,
                               100.0f * (1.0f + 0.5f * (W02_RANDOM_FRACTION_SCALE *
                                                        (float) mbRandMod(W02_RANDOM_MODULUS))),
                               -0.5f + W02_RANDOM_FRACTION_SCALE *
                                           (float) mbRandMod(W02_RANDOM_MODULUS),
                               0.33f + 0.66f * (W02_RANDOM_FRACTION_SCALE *
                                                (float) mbRandMod(W02_RANDOM_MODULUS)),
                               pistonColorPtr);
                }
            }
        }
        /* The additional space-attribute mask is evaluated without using its result. */
        mbMasuMAttrGet(index) & (1 << 16);
    }
    for (index = 0; index < 4; index++) {
        if (!work->particlesDisabled) {
            if (--work->emissionDelays[index] <= 0) {
                for (particleIndex = 0; particleIndex < lbl_1_data_584[index]; particleIndex++) {
                    position = work->particleOrigins[index];
                    switch (index) {
                    case 0:
                        velocity.x = lbl_1_data_554[index].x * (0.2f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                        velocity.y = lbl_1_data_554[index].y * (0.2f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                        velocity.z = lbl_1_data_554[index].z * (0.2f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                        break;
                    case 1:
                        velocity.x = lbl_1_data_554[index].x * (0.2f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                        velocity.y = lbl_1_data_554[index].y * (0.2f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                        velocity.z = lbl_1_data_554[index].z * (0.2f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                        break;
                    case 2:
                        velocity.x = lbl_1_data_554[index].x * (0.2f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                        velocity.y = lbl_1_data_554[index].y * (0.2f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                        velocity.z = lbl_1_data_554[index].z * (0.2f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                        /* This emitter also calculates the next case, replacing this velocity. */
                    case 3:
                        velocity.x = lbl_1_data_554[index].x * (0.2f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                        velocity.y = lbl_1_data_554[index].y * (0.2f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                        velocity.z = lbl_1_data_554[index].z * (0.2f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                        break;
                    }
                    if (GwSystem.curTime) {
                        velocity.x *= 0.5f + 0.1f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
                        velocity.y *= 0.5f + 0.1f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
                        velocity.z *= 0.5f + 0.1f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
                    }
                    brightness = W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS);
                    color.r = 192.0f + 32.0f * brightness;
                    color.g = 192.0f + 32.0f * brightness;
                    color.b = 192.0f + 32.0f * brightness;
                    color.a = 127.0f + 63.0f *
                        (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
                    emitterColor = color;
                    emitterColorPtr = &emitterColor;
                    emitterVelocity = velocity;
                    emitterVelocityPtr = &emitterVelocity;
                    emitterPosition = position;
                    emitterPositionPtr = &emitterPosition;
                    fn_1_176E4(emitterPositionPtr, emitterVelocityPtr,
                               100.0f * (1.0f + 0.5f * (W02_RANDOM_FRACTION_SCALE *
                                                        (float) mbRandMod(W02_RANDOM_MODULUS))),
                               0.5f * (-0.5f + W02_RANDOM_FRACTION_SCALE *
                                                   (float) mbRandMod(W02_RANDOM_MODULUS)),
                               0.33f + 0.66f * (W02_RANDOM_FRACTION_SCALE *
                                                (float) mbRandMod(W02_RANDOM_MODULUS)),
                               emitterColorPtr);
                }
                if (--work->burstFrames[index] <= 0) {
                    if (!GwSystem.curTime) {
                        work->emissionDelays[index] = 60.0f * (1.0f + 0.5f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                    } else {
                        work->emissionDelays[index] = 60.0f * (2.0f +
                            W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
                    }
                    if (!GwSystem.curTime) {
                        work->burstFrames[index] = 60.0f * (0.5f + 0.2f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                    } else {
                        work->burstFrames[index] = 60.0f * (0.2f + 0.1f *
                            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                    }
                }
            }
        }
    }
    for (index = 0; index < 5; index++) {
        work->orbitAngles[index] += work->orbitSpeed;
        if (work->orbitAngles[index] < 0.0f) {
            work->orbitAngles[index] += 360.0f;
        }
        if (work->orbitAngles[index] > 360.0f) {
            work->orbitAngles[index] -= 360.0f;
        }
        work->bobAngles[index] += 3.0f *
            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
        position.x = lbl_1_data_594.x + work->orbitRadii[index] *
            sin((M_PI * work->orbitAngles[index]) / 180.0);
        position.z = lbl_1_data_594.z + work->orbitRadii[index] *
            cos((M_PI * work->orbitAngles[index]) / 180.0);
        position.y = lbl_1_data_594.y + 0.2f * (100.0 *
            (sin((M_PI * work->bobAngles[index]) / 180.0) - 0.5));
        mbObjPosSetV(work->mapModels[index], &position);
        mbObjRotSet(work->mapModels[index], 0.0f, work->orbitAngles[index] - 90.0f, 0.0f);
    }
    if (work->transitionActive && !work->mapView) {
        for (index = 0; index < 4; index++) {
            if (HuPadBtnDown[index] & PAD_BUTTON_START) {
                work->mapView = TRUE;
            }
        }
    }
}

/* extern */

void fn_1_30F4(void) {
    mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 26), mbGuideSpeakerNoGet());
    mbWinTopWait();
}

/* Adds the Garage character attachment to its parent model during event setup. */
void fn_1_312C(s32 parentModel) {
    int dataNum;
    int model;
    int resource;

    dataNum = DATANUM(DATA_w02, 17);
    dataNum = FILENUM(dataNum);
    if (!GwSystem.curTime) {
        dataNum |= DATA_w02;
    } else {
        dataNum |= DATA_w02n;
    }
    resource = dataNum;
    model = mbObjCreate(resource, NULL, TRUE);
    mbObjAttrSet(model, HU3D_MOTATTR_LOOP);
    mbObjHookSet(parentModel, "plhook", model);
}
HuVecF lbl_1_data_5BC = { 0.0f, 1652.0f, -2010.0f };

HuVecF lbl_1_data_5C8 = { 315.0f, 0.0f, 0.0f };

float lbl_1_data_5D4 = 5000.0f;

HuVecF lbl_1_data_5D8 = { 0.0f, 1200.0f, -2029.0f };

HuVecF lbl_1_data_5E4 = { 320.0f, 0.0f, 0.0f };

float lbl_1_data_5F0 = 1500.0f;

HuVecF lbl_1_data_5F4 = { 2863.0f, 1289.0f, -2029.0f };

HuVecF lbl_1_data_600 = { 341.0f, -14.0f, 0.0f };

float lbl_1_data_60C = 875.0f;

HuVecF lbl_1_data_610 = { 3663.0f, 789.0f, -2029.0f };

HuVecF lbl_1_data_61C = { 341.0f, 30.0f, 0.0f };

float lbl_1_data_628 = 575.0f;

HuVecF lbl_1_data_62C = { -80.0f, 1285.0f, -1707.0f };

HuVecF lbl_1_data_638 = { 315.0f, 0.0f, 0.0f };

float lbl_1_data_644 = 5975.0f;

GXColor lbl_1_data_648 = { 192, 192, 255, 255 };

GXColor lbl_1_data_64C = { 64, 64, 64, 255 };

char * lbl_1_data_6C0[14] = {
    "ghook00", "ghook01", "ghook02", "ghook03", "ghook04", "ghook05", "ghook06", "ghook07",
    "ghook08", "ghook09", "ghook10", "ghook11", "ghook12", "ghook13"
};

HuVecF lbl_1_data_6F8 = { 0.0f, 0.0f, -2950.0f };

s32 lbl_1_data_704[15][4] = { { -1, -1, -1, -1 }, { 5, 14, 10, 9 },   { -1, -1, -1, -1 },
                              { 10, 9, 5, 14 },   { -1, -1, -1, -1 }, { 6, 2, 6, 4 },
                              { 7, 5, 7, 5 },     { 8, 6, 8, 6 },     { 9, 7, 9, 7 },
                              { 2, 8, 4, 8 },     { 11, 4, 11, 2 },   { 12, 10, 12, 10 },
                              { 13, 11, 13, 11 }, { 14, 12, 14, 12 }, { 4, 13, 2, 13 } };

HuVec2f lbl_1_data_7F4[16] = { { 0.0f, 0.0f },
                               { 0.25f, 0.0f },
                               { 0.25f, 0.33000001311302185f },
                               { 0.0f, 0.33000001311302185f },
                               { 0.25f, 0.0f },
                               { 0.5f, 0.0f },
                               { 0.5f, 0.33000001311302185f },
                               { 0.25f, 0.33000001311302185f },
                               { 0.5f, 0.0f },
                               { 0.75f, 0.0f },
                               { 0.75f, 0.33000001311302185f },
                               { 0.5f, 0.33000001311302185f },
                               { 0.75f, 0.0f },
                               { 1.0f, 0.0f },
                               { 1.0f, 0.33000001311302185f },
                               { 0.75f, 0.33000001311302185f } };

void (*lbl_1_data_874[5])(void) = { fn_1_4FB0, fn_1_544C, fn_1_58E8, fn_1_5D6C, fn_1_6378 };

HuVecF lbl_1_data_888[5] = { { -300.0f, 0.0f, 0.0f },
                             { 0.0f, 0.0f, -300.0f },
                             { 300.0f, 0.0f, 0.0f },
                             { 0.0f, 0.0f, 0.0f },
                             { 0.0f, 0.0f, 0.0f } };

HuVecF lbl_1_data_8C4[5] = { { 150.0f, 0.0f, 0.0f },
                             { 0.0f, 0.0f, 100.0f },
                             { -100.0f, 0.0f, 0.0f },
                             { 0.0f, 0.0f, 100.0f },
                             { 0.0f, 0.0f, 100.0f } };

HuVecF lbl_1_data_900[4] = {
    { -0.5f, 0.0f, -0.5f }, { 0.5f, 0.0f, -0.5f }, { 0.5f, 0.0f, 0.5f }, { -0.5f, 0.0f, 0.5f }
};

HuVecF lbl_1_data_930[4] = {
    { -0.5f, 0.0f, -0.5f }, { 0.5f, 0.0f, -0.5f }, { 0.5f, 0.0f, 0.5f }, { -0.5f, 0.0f, 0.5f }
};

float lbl_1_data_960[16][2] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f },
                                { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f },
                                { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f },
                                { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } };

HuVecF lbl_1_data_9E0[9] = { { 0.0f, 0.0f, 0.0f },    { -80.0f, 0.0f, -80.0f },
                             { 80.0f, 0.0f, -80.0f }, { -80.0f, 0.0f, 80.0f },
                             { 80.0f, 0.0f, 80.0f },  { -100.0f, 0.0f, 0.0f },
                             { 100.0f, 0.0f, 0.0f },  { 0.0f, 0.0f, -100.0f },
                             { 0.0f, 0.0f, 100.0f } };

float lbl_1_data_A4C[14] = {
    1.0f, 1.100000023841858f, 1.2999999523162842f, 1.0f,
    0.800000011920929f, 1.2000000476837158f, 1.2000000476837158f, 1.2000000476837158f,
    1.2999999523162842f, 0.8999999761581421f, 1.2999999523162842f, 0.8999999761581421f,
    0.8999999761581421f, 0.8999999761581421f
};

s32 lbl_1_data_A84[3] = { 2, 0, 1 };

s32 lbl_1_data_A90[3] = { 0, 2, 1 };

HuVecF lbl_1_data_A9C[3] = { { 1000.0f, 170.0f, -1600.0f },
                             { -3300.0f, 720.0f, -3400.0f },
                             { 2050.0f, 950.0f, -4550.0f } };

HuVecF lbl_1_data_AC0[3] = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };

/* Shows the time-change title, rotates the board, and plays the day/night transition. */
void fn_1_31D8(void) {
    HuVec2f windowPosition;
    PathBoardModels *work;
    s32 index;
    s32 window;

    work = lbl_1_bss_8;
    for (index = 0; index < 4; index++) {
        mbPlayerDispSet(index, FALSE);
    }
    mbCameraCenterSetV(&lbl_1_data_62C);
    mbCameraRotSetV(&lbl_1_data_638);
    mbCameraZoomSet(lbl_1_data_644);
    mbWipeFadeIn();
    mbTelopTimeChangeCreate();
    while (mbTelopTimeChangeCheck()) {
        HuPrcVSleep();
    }
    mbWipeFadeOut();
    HuPrcVSleep();
    mbCameraCenterSetV(&lbl_1_data_5BC);
    mbCameraRotSetV(&lbl_1_data_5C8);
    mbCameraZoomSet(lbl_1_data_5D4);
    if (!work->mapView) {
        mbCameraMovePos(&lbl_1_data_5D8, &lbl_1_data_5E4, NULL, lbl_1_data_5F0, -1.0f, 250);
    }
    for (index = 0; index < 4; index++) {
        mbPlayerDispSet(index, TRUE);
    }
    mbWipeFadeIn();
    window = mbWinCreateHelp(MESSNUM(MESS_BOARD_OPE, 12));
    mbWinPosGet(window, &windowPosition);
    windowPosition.y = 408.0f;
    mbWinPosSet(window, windowPosition.x, windowPosition.y);
    work->transitionActive = TRUE;
    fn_1_3FB8();
    while (fn_1_4470() && !work->mapView) {
        fn_1_3754();
    }
    for (index = 0; (float) index < 60.0f; index++) {
        fn_1_3754();
    }
    if (!work->mapView) {
        mbWipeFadeOut();
    }
    mbWipeWait();
    for (index = 0; index < 4; index++) {
        mbPlayerRotSet(index, 0.0f, 0.0f, 0.0f);
    }
    mbCameraCenterSetV(&lbl_1_data_5F4);
    mbCameraRotSetV(&lbl_1_data_600);
    mbCameraZoomSet(lbl_1_data_60C);
    if (!GwSystem.curTime) {
        mbObjMotionTimeSet(work->transitionModel,
            mbObjMotionMaxTimeGet(work->transitionModel) / 2.0f);
    } else {
        mbObjMotionTimeSet(work->transitionModel, 0.0f);
    }
    if (!work->mapView) {
        mbCameraMovePos(&lbl_1_data_610, &lbl_1_data_61C, NULL, lbl_1_data_628, -1.0f, 250);
    }
    if (!work->mapView) {
        mbWipeFadeIn();
    }
    if (!work->mapView) {
        mbAudFXPlay(W02_PATH_TRANSITION_SOUND_ID);
    }
    mbObjMotionSpeedSet(work->transitionModel, 1.0f);
    if (!GwSystem.curTime) {
        while (mbObjMotionTimeGet(work->transitionModel) <
               mbObjMotionMaxTimeGet(work->transitionModel) && !work->mapView) {
            fn_1_3754();
        }
    } else {
        while (mbObjMotionTimeGet(work->transitionModel) <
               mbObjMotionMaxTimeGet(work->transitionModel) / 2.0f && !work->mapView) {
            fn_1_3754();
        }
    }
    mbObjMotionSpeedSet(work->transitionModel, 0.0f);
    for (index = 0; (float) index < 60.0f; index++) {
        fn_1_3754();
    }
    if (!work->mapView) {
        mbWipeFadeOut();
    }
    fn_1_3754();
    mbWipeWait();
    for (index = 0; index < 4; index++) {
        mbPlayerDispSet(index, FALSE);
    }
    if (!GwSystem.curTime) {
        mbObjMotionTimeSet(work->transitionModel, 0.0f);
    } else {
        mbObjMotionTimeSet(work->transitionModel,
            mbObjMotionMaxTimeGet(work->transitionModel) / 2.0f);
    }
    while (fn_1_4470()) {
        HuPrcVSleep();
    }
    for (index = 0; index < 4; index++) {
        mbPlayerRotSet(index, 0.0f, 0.0f, 0.0f);
    }
    work->transitionActive = FALSE;
    mbWinKill(window);
    for (index = 0; index < 4; index++) {
        mbPlayerDispSet(index, TRUE);
    }
}

/* During the day/night sequence, yields once unless both transition flags are set.
 * With both set, runs a normal fade-out only when the wipe checks are clear; otherwise returns
 * without yielding. */
void fn_1_3754(void) {
    PositionModel *work = lbl_1_bss_8;

    if (work->mapView != 0 && work->transitionActive != 0) {
        if (mbWipeSpecialCheck() == 0 && mbWipeSpecialStatGet() == 0) {
            mbWipeFadeOut();
        }
    } else {
        HuPrcVSleep();
    }
}

/* Loads the board model lights, then sets the colors used by player metal effects. */
void fn_1_37C0(void) {
    GXColor shadowColor;
    GXColor hiliteColor;
    const GXColor *hilite;
    const GXColor *shadow;

    Hu3DBGColorSet(0, 0, 0);
    Hu3DModelLightInfoSet(mbObjModelIDGet(lbl_1_bss_5C), 1);
    hiliteColor = lbl_1_data_64C;
    hilite = &hiliteColor;
    shadowColor = lbl_1_data_648;
    shadow = &shadowColor;
    mbPlayerMetalColorSet(shadow, hilite);
}

void fn_1_3878(void) {
}

/* Dispatches a player's space event, returning handled even when no event is marked. */
s32 fn_1_387C(s32 playerNo, s16 spaceId) {
    if (mbMasuMAttrGet(spaceId) & 0x40) {
        fn_1_6CA0();
        fn_1_6CFC(0);
        fn_1_6D68();
        mbev_CapPlayerIdleWait();
    } else if (mbMasuMAttrGet(spaceId) & 0x80) {
        fn_1_6CA0();
        fn_1_6CFC(1);
        fn_1_6D68();
        mbev_CapPlayerIdleWait();
    } else if (mbMasuMAttrGet(spaceId) & 0x200) {
        fn_1_6CA0();
        fn_1_6CFC(2);
        fn_1_6D68();
        mbev_CapPlayerIdleWait();
    } else if (mbMasuMAttrGet(spaceId) & 1) {
        fn_1_8C9C(playerNo, spaceId);
    } else if (mbMasuMAttrGet(spaceId) & 0x800) {
        if (!GwSystem.curTime) {
            fn_1_B11C(playerNo);
        } else {
            fn_1_B334(playerNo);
        }
    } else if (mbMasuMAttrGet(spaceId) & 0x1000) {
        fn_1_E074(playerNo);
    } else if (mbMasuMAttrGet(spaceId) & (1 << 17)) {
        fn_1_13708(playerNo, spaceId);
    }
    return 1;
}

s32 fn_1_39E4(void) { return 0; }

s32 fn_1_39EC(void) { return 0; }

/* Runs after a player finishes moving, restarting the animation for the space's active event. */
s32 fn_1_39F4(s32 player, s16 spaceId) {
    BoardModels *work;
    u32 modelNo;

    work = lbl_1_bss_8;
    if (mbMasuMAttrGet(spaceId) & 0xC000) {
        modelNo = (mbMasuMAttrGet(spaceId) & 0xC000) >> 14;
        mbObjMotionTimeSet(work->spaceModels[modelNo - 1], 0.0f);
        mbObjMotionSpeedSet(work->spaceModels[modelNo - 1], 1.0f);
    }
    if (mbMasuMAttrGet(spaceId) & (1 << 16)) {
        mbAudFXPlay(W02_STAR_SPACE_SOUND_ID);
        mbObjMotionSpeedSet(work->eventModel, 1.0f);
        mbObjMotionTimeSet(work->eventModel, 0.0f);
    }
    return 0;
}

s32 fn_1_3AF4(void) { return 0; }

/* At turn start, reveals the model on the drone hooks and enables the steam emitters. */
void fn_1_3AFC(void) {
    void *work;

    work = lbl_1_bss_8;
    mbObjDispSet((s16) (*(s32 *)((s8 *)(work) + 4812)), 1);
    (*(s32 *)((s8 *)(work) + 4984)) = 0;
}

/* Switches event models to map sprites while the map view is open. */
void fn_1_3B44(s32 mapView) {
    s32 spaceId;
    BoardModels *work;
    EventMapMarker *marker;
    s32 linkedSpace;

    work = lbl_1_bss_8;
    marker = work->markers;
    if (mapView) {
        for (spaceId = 1; spaceId < mbMasuNumGet(); spaceId++) {
            if (mbMasuMAttrGet(spaceId) & 1) {
                linkedSpace = mbMasuMAttrFindLink(spaceId, 2);
                mbMapSprAdd(19, linkedSpace);
                mbObjDispSet(marker->modelId, FALSE);
                marker++;
            }
        }
        mbObjDispSet(work->mapDisplayModel, FALSE);
        for (spaceId = 0; spaceId < 5; spaceId++) {
            mbObjDispSet(work->mapModels[spaceId], FALSE);
        }
        work->mapView = TRUE;
    } else {
        for (spaceId = 1; spaceId < mbMasuNumGet(); spaceId++) {
            if (mbMasuMAttrGet(spaceId) & 1) {
                mbObjDispSet(marker->modelId, TRUE);
                marker++;
            }
        }
        mbObjDispSet(work->mapDisplayModel, TRUE);
        for (spaceId = 0; spaceId < 5; spaceId++) {
            mbObjDispSet(work->mapModels[spaceId], TRUE);
        }
        work->mapView = FALSE;
    }
}

/* Loads the moving board geometry and associates marked spaces with nearby model transforms. */
void fn_1_3CC8(void) {
    ModelTransform *transform;
    BoardModels *work;
    s32 spaceId;
    SpaceBinding *binding;
    s32 modelId;
    s32 transformIndex;
    HuVecF position;
    HuVecF delta;

    work = lbl_1_bss_8;
    work->primaryModel = modelId = mbObjCreate(
        BoardModelDataNum(DATANUM(DATA_w02, 20)), NULL, FALSE);
    mbObjMotionSpeedSet(modelId, 0.0f);
    Hu3DMotionCalc(mbObjModelIDGet(modelId));
    work->spaceState = 0;
    work->secondaryModel = mbObjCreate(BoardModelDataNum(DATANUM(DATA_w02, 21)), NULL, FALSE);
    mbObjMotionSpeedSet(work->secondaryModel, 0.0f);
    mbObjLayerSet(work->secondaryModel, 3);
    /* This pass reads marked-space attributes without changing any board state. */
    for (spaceId = 1; spaceId < mbMasuNumGet(); spaceId++) {
        mbMasuMAttrGet(spaceId) & 0x3C;
    }
    transform = work->transforms;
    for (spaceId = 0; spaceId < 14; spaceId++, transform++) {
        Hu3DModelObjMtxGet(mbObjModelIDGet(modelId), lbl_1_data_6C0[spaceId], transform->matrix);
        transform->position.x = transform->matrix[0][3];
        transform->position.y = transform->matrix[1][3];
        transform->position.z = transform->matrix[2][3];
    }
    binding = work->bindings;
    work->spaceCount = 0;
    for (spaceId = 1; spaceId < mbMasuNumGet(); spaceId++) {
        if ((mbMasuMAttrGet(spaceId) & 0x3C) && fn_1_46D0(spaceId) >= 5) {
            binding->spaceId = spaceId;
            mbMasuPosGet(spaceId, &position);
            transform = work->transforms;
            for (transformIndex = 0; transformIndex < 14; transformIndex++, transform++) {
                PSVECSubtract(&position, &transform->position, &delta);
                delta.y = 0.0f;
                if (PSVECMag(&delta) <= 100.0f) {
                    break;
                }
            }
            /* The failed-search check is discarded, so index 14 is stored when no transform
             * matches. */
            transformIndex >= 14;
            binding->transformIndex = transformIndex;
            binding++;
            work->spaceCount++;
        }
    }
    mbObjMotionTimeSet(modelId, lbl_1_bss_4->configuration *
        (0.25f * mbObjMotionMaxTimeGet(modelId)));
    Hu3DMotionCalc(mbObjModelIDGet(modelId));
    fn_1_4494();
    fn_1_45A4();
}

/* Starts the board rotation process once no prior rotation process is active. */
void fn_1_3FB8(void) {
    PositionModel *work = lbl_1_bss_8;

    if (fn_1_4470() == 0) {
        HuPrcChildCreate(fn_1_4020, W02_ROTATION_PROCESS_PRIORITY,
                         W02_ROTATION_PROCESS_STACK_SIZE, 0, mbMainProc);
        work->spaceState = 1;
    }
}

/* Rotates the board models and carries players with their current spaces. */
void fn_1_4020(void) {
    Mtx rotation;
    HuVecF playerOffset[4];
    HuVecF playerRotation[4];
    HuVecF position;
    HuVecF spacePosition;
    PathBoardModels *work;
    s32 playerNo;
    s32 frame;
    s32 sound;
    float maxTime;
    float quarterTime;
    float startTime;
    float endTime;
    float progress;
    float motionTime;

    work = lbl_1_bss_8;
    for (playerNo = 0; playerNo < 4; playerNo++) {
        mbMasuPosGet(GwPlayer[playerNo].masuId, &position);
        mbPlayerPosGet(playerNo, &playerOffset[playerNo]);
        PSVECSubtract(&playerOffset[playerNo], &position, &playerOffset[playerNo]);
        mbPlayerRotGet(playerNo, &playerRotation[playerNo]);
    }
    maxTime = mbObjMotionMaxTimeGet(work->primaryModel);
    quarterTime = 0.25f * maxTime;
    startTime = quarterTime * (float) lbl_1_bss_4->configuration;
    endTime = startTime + quarterTime;
    if (endTime > maxTime) {
        endTime = maxTime;
    }
    if (lbl_1_bss_4->configuration == 0) {
        startTime = 0.0f;
        endTime = quarterTime;
    }
    sound = mbAudFXPlay(W02_BOARD_ROTATION_SOUND_ID);
    for (frame = 0; (float) frame <= 120.0f; frame++) {
        if (work->mapView && mbWipeSpecialStatGet() && !mbWipeSpecialCheck()) {
            /* Once the skipped transition's wipe is ready, jump directly to the final rotation
             * frame. */
            frame = 120;
        }
        progress = (float) frame / 120.0f;
        progress = (float) sin((3.141592653589793 * (90.0f * progress)) / 180.0);
        motionTime = startTime + progress * (endTime - startTime);
        mbObjMotionTimeSet(work->primaryModel, motionTime);
        fn_1_4494();
        mbObjMotionTimeSet(work->secondaryModel,
            progress * mbObjMotionMaxTimeGet(work->secondaryModel));
        mtxRot(rotation, 0.0f, 90.0f * -progress, 0.0f);
        for (playerNo = 0; playerNo < 4; playerNo++) {
            if (fn_1_46D0(GwPlayer[playerNo].masuId) > 4) {
                mbMasuPosGet(GwPlayer[playerNo].masuId, &spacePosition);
                PSMTXMultVec(rotation, &playerOffset[playerNo], &position);
                PSVECAdd(&spacePosition, &position, &position);
                mbPlayerPosSetV(playerNo, &position);
                mbPlayerRotSet(playerNo, playerRotation[playerNo].x,
                    playerRotation[playerNo].y - 90.0f * progress, playerRotation[playerNo].z);
            }
        }
        HuPrcVSleep();
    }
    mbObjMotionTimeSet(work->primaryModel, endTime);
    fn_1_4494();
    if (sound != -1) {
        mbAudFXStop(sound);
    }
    if (++lbl_1_bss_4->configuration >= 4) {
        lbl_1_bss_4->configuration = 0;
    }
    fn_1_45A4();
    fn_1_1560C();
    work->spaceState = 0;
    HuPrcEnd();
}

s32 fn_1_4470(void) {
    PositionModel *state = lbl_1_bss_8;
    return state->spaceState;
}

/* Updates moving-space positions from the Garage model and applies them to the board map. */
void fn_1_4494(void) {
    Mtx matrix;
    ModelTransform *transform;
    s32 index;
    BoardModels *work;
    SpaceBinding *binding;
    s32 modelId;

    work = lbl_1_bss_8;
    modelId = work->primaryModel;
    Hu3DMotionCalc(mbObjModelIDGet(modelId));
    transform = work->transforms;
    for (index = 0; index < 14; index++, transform++) {
        Hu3DModelObjMtxGet(mbObjModelIDGet(modelId), lbl_1_data_6C0[index], transform->matrix);
        transform->position.x = transform->matrix[0][3];
        transform->position.y = transform->matrix[1][3] += 3.0f;
        transform->position.z = transform->matrix[2][3];
    }
    binding = work->bindings;
    for (index = 0; index < work->spaceCount; index++, binding++) {
        transform = &work->transforms[binding->transformIndex];
        PSMTXIdentity(matrix);
        matrix[0][3] = transform->matrix[0][3];
        matrix[1][3] = transform->matrix[1][3];
        matrix[2][3] = transform->matrix[2][3];
        mbMasuMtxSet(binding->spaceId, matrix);
    }
}

/* Reconnects marked spaces according to the current board configuration. */
void fn_1_45A4(void) {
    void *work[1];
    s32 destinationSpace;
    s32 sourceSpace;
    s32 sourceGroup;
    MASU *space;
    s32 destinationGroup;
    s32 configuration;

    /* The shared work pointer is fetched but is not used to reconnect the spaces. */
    work[0] = lbl_1_bss_8;
    configuration = lbl_1_bss_4->configuration;
    for (sourceSpace = 1; sourceSpace < mbMasuNumGet(); sourceSpace++) {
        if (mbMasuMAttrGet(sourceSpace) & 0x3C) {
            sourceGroup = fn_1_46D0(sourceSpace);
            /* The range checks are evaluated without enforcing their results. */
            if (sourceGroup > 0) {
                sourceGroup > 14;
            }
            destinationGroup = lbl_1_data_704[sourceGroup][configuration];
            if (destinationGroup > 0) {
                for (destinationSpace = 1; destinationSpace < mbMasuNumGet(); destinationSpace++) {
                    if ((mbMasuMAttrGet(destinationSpace) & 0x3C)
                        && destinationGroup == fn_1_46D0(destinationSpace)) {
                        break;
                    }
                }
                destinationSpace >= mbMasuNumGet();
                space = mbMasuGet(sourceSpace);
                space->linkTbl[0] = destinationSpace;
            }
        }
    }
}

/* Decodes the rotating-board group used to bind and reconnect moving spaces. */
s32 fn_1_46D0(s32 spaceId) {
    s32 unusedStackValue;
    u32 spaceAttributes;
    spaceAttributes = mbMasuMAttrGet((s16) spaceId) & 0x3C;
    if (spaceAttributes == 0)
    {
        return - 1;
    }
    return (spaceAttributes >> 2U) & 0xF;
}

/* Loads the moving path models and builds their marked-space ranges and display updates. */
void fn_1_471C(void) {
    s32 spaceIds[32];
    HuVecF position;
    PathPathSpaces *path;
    PathBoardModels *work;
    s32 spaceId;
    s32 spaceCount;
    s32 modelId;
    OMOBJ *object;

    work = lbl_1_bss_8;
    work->paths[0].boardModelId = modelId =
        mbObjCreate(BoardModelDataNum(DATANUM(DATA_w02, 42)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjMotionSpeedSet(modelId, 0.0f);
    work->paths[1].boardModelId = modelId =
        mbObjCreate(BoardModelDataNum(DATANUM(DATA_w02, 43)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjMotionSpeedSet(modelId, 0.0f);
    work->paths[2].boardModelId = modelId =
        mbObjCreate(BoardModelDataNum(DATANUM(DATA_w02, 44)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjMotionSpeedSet(modelId, 0.0f);
    work->paths[3].boardModelId = modelId =
        mbObjCreate(BoardModelDataNum(DATANUM(DATA_w02, 45)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjMotionSpeedSet(modelId, 0.0f);
    work->paths[4].boardModelId = modelId =
        mbObjCreate(BoardModelDataNum(DATANUM(DATA_w02, 47)), NULL, FALSE);
    mbObjMotionSpeedSet(modelId, 0.0f);
    work->pathEffectModel = modelId =
        mbObjCreate(BoardModelDataNum(DATANUM(DATA_w02, 46)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjMotionSpeedSet(modelId, 3.0f);

    path = &work->paths[0];
    spaceId = 1;
    spaceCount = 0;
    for (; spaceId < mbMasuNumGet(); spaceId++) {
        if (mbMasuMAttrGet(spaceId) & 0x40) {
            if (spaceCount == 0) {
                mbMasuPosGet(spaceId, &path->end);
                mbMasuPosGet(spaceId, &path->start);
            } else {
                mbMasuPosGet(spaceId, &position);
                if (position.x > path->start.x) {
                    path->start = position;
                }
                if (position.x < path->end.x) {
                    path->end = position;
                }
            }
            spaceIds[spaceCount] = spaceId;
            spaceCount++;
        }
    }
    path->spaceCount = spaceCount;
    path->modelSpaceCount = spaceCount * 2;
    fn_1_6E28(0, spaceIds, TRUE);
    fn_1_7280(0);
    object = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_7418);
    object->work[0] = 0;

    path = &work->paths[1];
    spaceId = 1;
    spaceCount = 0;
    for (; spaceId < mbMasuNumGet(); spaceId++) {
        if (mbMasuMAttrGet(spaceId) & 0x80) {
            if (spaceCount == 0) {
                mbMasuPosGet(spaceId, &path->end);
                mbMasuPosGet(spaceId, &path->start);
            } else {
                mbMasuPosGet(spaceId, &position);
                if (position.z > path->start.z) {
                    path->start = position;
                }
                if (position.z < path->end.z) {
                    path->end = position;
                }
            }
            spaceIds[spaceCount] = spaceId;
            spaceCount++;
        }
    }
    path->spaceCount = spaceCount;
    path->modelSpaceCount = spaceCount * 2;
    fn_1_6E28(1, spaceIds, TRUE);
    fn_1_7280(1);
    object = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_7418);
    object->work[0] = 1;

    path = &work->paths[2];
    spaceId = 1;
    spaceCount = 0;
    for (; spaceId < mbMasuNumGet(); spaceId++) {
        if (mbMasuMAttrGet(spaceId) & 0x200) {
            if (spaceCount == 0) {
                mbMasuPosGet(spaceId, &path->end);
                mbMasuPosGet(spaceId, &path->start);
            } else {
                mbMasuPosGet(spaceId, &position);
                if (position.x < path->start.x) {
                    path->start = position;
                }
                if (position.x > path->end.x) {
                    path->end = position;
                }
            }
            spaceIds[spaceCount] = spaceId;
            spaceCount++;
        }
    }
    path->spaceCount = spaceCount;
    path->modelSpaceCount = spaceCount * 2;
    fn_1_6E28(2, spaceIds, TRUE);
    fn_1_7280(2);
    object = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_7418);
    object->work[0] = 2;

    path = &work->paths[3];
    spaceId = 1;
    spaceCount = 0;
    for (; spaceId < mbMasuNumGet(); spaceId++) {
        if (mbMasuMAttrGet(spaceId) & 0x100) {
            if (spaceCount == 0) {
                mbMasuPosGet(spaceId, &path->end);
                mbMasuPosGet(spaceId, &path->start);
            } else {
                mbMasuPosGet(spaceId, &position);
                if (position.z > path->start.z) {
                    path->start = position;
                }
                if (position.z < path->end.z) {
                    path->end = position;
                }
            }
            spaceIds[spaceCount] = spaceId;
            spaceCount++;
        }
    }
    path->spaceCount = spaceCount;
    path->modelSpaceCount = spaceCount * 2;
    fn_1_6E28(3, spaceIds, FALSE);

    path = &work->paths[4];
    spaceId = 1;
    spaceCount = 0;
    for (; spaceId < mbMasuNumGet(); spaceId++) {
        if (mbMasuMAttrGet(spaceId) & 0x400) {
            if (spaceCount == 0) {
                mbMasuPosGet(spaceId, &path->end);
                mbMasuPosGet(spaceId, &path->start);
            } else {
                mbMasuPosGet(spaceId, &position);
                if (position.z > path->start.z) {
                    path->start = position;
                }
                if (position.z < path->end.z) {
                    path->end = position;
                }
            }
            spaceIds[spaceCount] = spaceId;
            spaceCount++;
        }
    }
    path->spaceCount = spaceCount;
    path->modelSpaceCount = spaceCount * 2;
    fn_1_6E28(4, spaceIds, FALSE);
    fn_1_7548(0);
    fn_1_7548(1);
    fn_1_7548(2);
    if (lbl_1_bss_4->priorState[0]) {
        fn_1_71B4(0);
        fn_1_7280(0);
    }
    if (lbl_1_bss_4->priorState[1]) {
        fn_1_71B4(1);
        fn_1_7280(1);
    }
    if (lbl_1_bss_4->priorState[2]) {
        fn_1_71B4(2);
        fn_1_7280(2);
    }
    fn_1_762C();
    HuDataDirClose(DATA_bmasu);
    fn_1_6CA0();
}

/* Slides the first path right, transferring players when they reach its destination. */
void fn_1_4FB0(void) {
    HuVecF position;
    s32 players[4];
    PathBoardModels *work;
    PathPathSpaces *path;
    s32 sound;
    s32 finished;
    s32 eventStarted;
    s32 playerCount;
    s32 destinationSpace;
    s32 frame;
    s32 index;
    EffectElement *element;
    EffectData *effect;
    HU3D_MODEL *model;

    work = lbl_1_bss_8;
    path = &work->paths[0];
    finished = 0;
    eventStarted = 0;
    fn_1_7814();
    playerCount = fn_1_786C(players, W02_ROUTE_SPACE_ATTR_6_FLAG);
    destinationSpace = mbMasuMAttrFindLink(path->spaceIds[0], W02_ROUTE_SPACE_ATTR_8_FLAG);
    mbMasuNextDispSet(0);
    work->coinRewardCount += mbRandMod(3) + 4;
    sound = mbAudFXPlay(W02_PATH_TRAVEL_SOUND_ID);
    mbObjMotionSpeedSet(path->boardModelId, 2.72f);
    frame = 0;
    while (!finished) {
        path->travelDistance += 10.0f;
        if (path->travelDistance >= path->length) {
            finished = 1;
        }
        frame++;
        for (index = 0; index < playerCount; index++) {
            if (players[index] != -1) {
                mbPlayerPosGet(players[index], &position);
                position.x += 10.0f;
                mbPlayerPosSetV(players[index], &position);
                if (frame % 20 == 1) {
                    omVibrate(players[index], 20, 4, 4);
                }
                if (position.x >= path->markerStart.x) {
                    fn_1_7A4C(players[index], destinationSpace);
                    players[index] = -1;
                }
            }
        }
        for (index = 0; index < playerCount; index++) {
            if (players[index] != -1) {
                break;
            }
        }
        if (index >= playerCount && fn_1_8338() && !eventStarted) {
            fn_1_6CFC(3);
            eventStarted = 1;
        }
        model = &Hu3DData[path->modelId];
        effect = model->hookData;
        element = effect->elements;
        for (index = 0; index < path->modelSpaceCount; index++, element++) {
            element->position = path->positions[index];
            element->position.x += path->travelDistance;
            element->position.y += 3.0f;
            if (element->position.x > path->start.x || element->position.x < path->end.x) {
                if (element->color.a > 20) {
                    element->color.a -= 20;
                } else {
                    element->color.a = 0;
                }
            } else if (element->position.x <= path->start.x &&
                       element->position.x >= path->end.x) {
                if (element->color.a < 235) {
                    element->color.a += 20;
                } else {
                    element->color.a = 255;
                }
            }
        }
        fn_1_7548(0);
        HuPrcVSleep();
    }
    mbObjMotionSpeedSet(path->boardModelId, 0.0f);
    if (sound != -1) {
        mbAudFXStop(sound);
    }
    while (!fn_1_8338()) {
        HuPrcVSleep();
    }
    if (!eventStarted) {
        fn_1_6CFC(3);
        eventStarted = 1;
    }
    fn_1_71B4(0);
    fn_1_7280(0);
    lbl_1_bss_4->priorState[0] ^= 1;
    path->travelDistance = 0.0f;
    model = &Hu3DData[path->modelId];
    effect = model->hookData;
    element = effect->elements;
    for (index = 0; index < path->modelSpaceCount; index++, element++) {
        element->position = path->positions[index];
        element->position.y += 3.0f;
        if (index <= path->spaceCount) {
            element->color.a = 255;
        } else {
            element->color.a = 0;
        }
    }
    fn_1_7548(0);
    mbMasuNextDispSet(1);
    lbl_1_bss_18[0] = NULL;
    HuPrcEnd();
}

/* Slides the second path along Z, transferring players when they reach its destination. */
void fn_1_544C(void) {
    HuVecF position;
    s32 players[4];
    s16 parentSpaces[MASU_LINK_MAX];
    PathBoardModels *work;
    PathPathSpaces *path;
    s32 sound;
    s32 finished;
    s32 eventStarted;
    s32 playerCount;
    s32 destinationSpace;
    s32 frame;
    s32 index;
    EffectElement *element;
    EffectData *effect;
    HU3D_MODEL *model;

    work = lbl_1_bss_8;
    path = &work->paths[1];
    finished = 0;
    eventStarted = 0;
    fn_1_7814();
    playerCount = fn_1_786C(players, W02_ROUTE_SPACE_ATTR_7_FLAG);
    mbMasuLinkParentGet(path->spaceIds[0], parentSpaces);
    destinationSpace = parentSpaces[0];
    mbMasuNextDispSet(0);
    work->coinRewardCount += mbRandMod(2) + 3;
    sound = mbAudFXPlay(W02_PATH_TRAVEL_SOUND_ID);
    mbObjMotionSpeedSet(path->boardModelId, 2.72f);
    frame = 0;
    while (!finished) {
        path->travelDistance += 10.0f;
        if (path->travelDistance >= path->length) {
            finished = 1;
        }
        frame++;
        for (index = 0; index < playerCount; index++) {
            if (players[index] != -1) {
                mbPlayerPosGet(players[index], &position);
                position.z += 10.0f;
                mbPlayerPosSetV(players[index], &position);
                if (frame % 20 == 1) {
                    omVibrate(players[index], 20, 4, 4);
                }
                if (position.z >= path->markerStart.z) {
                    fn_1_7A4C(players[index], destinationSpace);
                    players[index] = -1;
                }
            }
        }
        for (index = 0; index < playerCount; index++) {
            if (players[index] != -1) {
                break;
            }
        }
        if (index >= playerCount && fn_1_8338() && !eventStarted) {
            fn_1_6CFC(0);
            eventStarted = 1;
        }
        model = &Hu3DData[path->modelId];
        effect = model->hookData;
        element = effect->elements;
        for (index = 0; index < path->modelSpaceCount; index++, element++) {
            element->position = path->positions[index];
            element->position.z += path->travelDistance;
            element->position.y += 3.0f;
            if (element->position.z > path->start.z || element->position.z < path->end.z) {
                if (element->color.a > 20) {
                    element->color.a -= 20;
                } else {
                    element->color.a = 0;
                }
            } else if (element->position.z <= path->start.z &&
                       element->position.z >= path->end.z) {
                if (element->color.a < 235) {
                    element->color.a += 20;
                } else {
                    element->color.a = 255;
                }
            }
        }
        fn_1_7548(1);
        HuPrcVSleep();
    }
    mbObjMotionSpeedSet(path->boardModelId, 0.0f);
    if (sound != -1) {
        mbAudFXStop(sound);
    }
    while (!fn_1_8338()) {
        HuPrcVSleep();
    }
    if (!eventStarted) {
        fn_1_6CFC(0);
        eventStarted = 1;
    }
    fn_1_71B4(1);
    fn_1_7280(1);
    lbl_1_bss_4->priorState[1] ^= 1;
    path->travelDistance = 0.0f;
    model = &Hu3DData[path->modelId];
    effect = model->hookData;
    element = effect->elements;
    for (index = 0; index < path->modelSpaceCount; index++, element++) {
        element->position = path->positions[index];
        element->position.y += 3.0f;
        if (index <= path->spaceCount) {
            element->color.a = 255;
        } else {
            element->color.a = 0;
        }
    }
    fn_1_7548(1);
    mbMasuNextDispSet(1);
    lbl_1_bss_18[1] = NULL;
    HuPrcEnd();
}

/* Slides the third path left, transferring players when they reach its destination. */
void fn_1_58E8(void) {
    HuVecF position;
    s32 players[4];
    PathBoardModels *work;
    PathPathSpaces *path;
    s32 sound;
    s32 finished;
    s32 eventStarted;
    s32 playerCount;
    s32 destinationSpace;
    s32 frame;
    s32 index;
    EffectElement *element;
    EffectData *effect;
    HU3D_MODEL *model;

    work = lbl_1_bss_8;
    path = &work->paths[2];
    finished = 0;
    eventStarted = 0;
    fn_1_7814();
    playerCount = fn_1_786C(players, W02_ROUTE_SPACE_ATTR_9_FLAG);
    destinationSpace = mbMasuMAttrFindLink(path->spaceIds[0], W02_ROUTE_SPACE_ATTR_10_FLAG);
    mbMasuNextDispSet(0);
    sound = mbAudFXPlay(W02_PATH_TRAVEL_SOUND_ID);
    mbObjMotionSpeedSet(path->boardModelId, 2.72f);
    frame = 0;
    while (!finished) {
        path->travelDistance += 10.0f;
        if (path->travelDistance >= path->length) {
            finished = 1;
        }
        frame++;
        for (index = 0; index < playerCount; index++) {
            if (players[index] != -1) {
                mbPlayerPosGet(players[index], &position);
                position.x -= 10.0f;
                mbPlayerPosSetV(players[index], &position);
                if (frame % 20 == 1) {
                    omVibrate(players[index], 20, 4, 4);
                }
                if (position.x <= path->markerStart.x) {
                    fn_1_7A4C(players[index], destinationSpace);
                    players[index] = -1;
                }
            }
        }
        for (index = 0; index < playerCount; index++) {
            if (players[index] != -1) {
                break;
            }
        }
        if (index >= playerCount && fn_1_8338() && !eventStarted) {
            fn_1_6CFC(4);
            eventStarted = 1;
        }
        model = &Hu3DData[path->modelId];
        effect = model->hookData;
        element = effect->elements;
        for (index = 0; index < path->modelSpaceCount; index++, element++) {
            element->position = path->positions[index];
            element->position.x -= path->travelDistance;
            element->position.y += 3.0f;
            if (element->position.x < path->start.x || element->position.x > path->end.x) {
                if (element->color.a > 20) {
                    element->color.a -= 20;
                } else {
                    element->color.a = 0;
                }
            } else if (element->position.x >= path->start.x &&
                       element->position.x <= path->end.x) {
                if (element->color.a < 235) {
                    element->color.a += 20;
                } else {
                    element->color.a = 255;
                }
            }
        }
        fn_1_7548(2);
        HuPrcVSleep();
    }
    mbObjMotionSpeedSet(path->boardModelId, 0.0f);
    if (sound != -1) {
        mbAudFXStop(sound);
    }
    while (!fn_1_8338()) {
        HuPrcVSleep();
    }
    if (!eventStarted) {
        fn_1_6CFC(4);
        eventStarted = 1;
    }
    fn_1_71B4(2);
    fn_1_7280(2);
    lbl_1_bss_4->priorState[2] ^= 1;
    path->travelDistance = 0.0f;
    model = &Hu3DData[path->modelId];
    effect = model->hookData;
    element = effect->elements;
    for (index = 0; index < path->modelSpaceCount; index++, element++) {
        element->position = path->positions[index];
        element->position.y += 3.0f;
        if (index <= path->spaceCount) {
            element->color.a = 255;
        } else {
            element->color.a = 0;
        }
    }
    fn_1_7548(2);
    mbMasuNextDispSet(1);
    lbl_1_bss_18[2] = NULL;
    HuPrcEnd();
}

/* Carries players off the fourth path, awards flying coins, and displays each player's total. */
void fn_1_5D6C(void) {
    HuVecF position;
    HuVecF velocity;
    HuVecF anchor;
    s32 players[4];
    s32 participants[4];
    s32 coinCounts[4];
    s32 displayIds[4];
    PathBoardModels *work;
    PathPathSpaces *path;
    u32 participantIndex;
    s32 playerNo;
    s32 sound;
    s32 finished;
    s32 cameraMoved;
    s32 playerCount;
    s32 destinationSpace;
    s32 frame;
    s32 index;

    work = lbl_1_bss_8;
    path = &work->paths[3];
    finished = 0;
    cameraMoved = 0;
    fn_1_7814();
    playerCount = fn_1_786C(players, W02_ROUTE_SPACE_ATTR_8_FLAG);
    for (index = 0; index < playerCount; index++) {
        participants[index] = players[index];
    }
    for (index = 0; index < playerCount; index++) {
        coinCounts[index] = 0;
    }
    destinationSpace = mbev_CapMasuLinkNextGet(path->spaceIds[0], FALSE);
    mbev_PlayerColMasuSet(GwSystem.turnPlayerNo, destinationSpace, FALSE);
    mbMasuNextDispSet(FALSE);
    fn_1_838C();
    sound = mbAudFXPlay(W02_PATH_TRAVEL_SOUND_ID);
    mbObjMotionSpeedSet(path->boardModelId, 4.1f);
    frame = 0;
    while (!finished) {
        path->travelDistance += 15.000001f;
        if (path->travelDistance >= path->length) {
            finished = 1;
        }
        frame++;
        for (index = 0; index < playerCount; index++) {
            if (players[index] != -1) {
                mbPlayerPosGet(players[index], &position);
                position.z += 15.000001f;
                mbPlayerPosSetV(players[index], &position);
                anchor = position;
                if (frame % 20 == 1) {
                    omVibrate(players[index], 20, 4, 4);
                }
                if (position.z >= path->markerStart.z) {
                    fn_1_7A4C(players[index], destinationSpace);
                    players[index] = -1;
                    if (!cameraMoved) {
                        position.x = position.z = 0.0f;
                        position.y = 100.0f;
                        mbCameraMoveMasu(destinationSpace, NULL, &position, 1600.0f, -1.0f, 30);
                        cameraMoved = 1;
                    }
                }
            }
        }
        HuPrcVSleep();
    }
    for (index = 0; index < playerCount; index++) {
        if (players[index] != -1) {
            fn_1_7A4C(players[index], destinationSpace);
            players[index] = -1;
        }
    }
    while (!fn_1_8338()) {
        HuPrcVSleep();
    }
    for (index = 0; index < work->coinRewardCount; index++) {
        participantIndex = mbRandMod(playerCount);
        playerNo = participants[participantIndex];
        coinCounts[participantIndex]++;
        frame = 60.0f * (0.2f + 0.1f *
            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
        mbPlayerPosGet(playerNo, &position);
        position.x += 0.5f * (100.0f * (-0.5f +
            W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
        position.y = 50.0f + anchor.y;
        position.z = path->markerStart.z - 15.000001f * (float) frame;
        velocity.x = velocity.y = 0.0f;
        velocity.z = 15.000001f;
        fn_1_87D0(&position, &velocity, playerNo, frame);
        HuPrcSleep(mbRandMod(3) + 3);
    }
    while (fn_1_874C()) {
        HuPrcVSleep();
    }
    mbAudFXPlay(15);
    mbObjMotionSpeedSet(path->boardModelId, 0.0f);
    if (sound != -1) {
        mbAudFXStop(sound);
    }
    for (index = 0; index < playerCount; index++) {
        mbPlayerPosGet(participants[index], &position);
        position.y += 250.0f;
        displayIds[index] = mbCoinDispCapsuleCreate(&position, coinCounts[index]);
    }
    do {
        for (index = 0; index < playerCount; index++) {
            if (displayIds[index] != -1 && !mbCoinDispKillCheck(displayIds[index])) {
                break;
            }
        }
        HuPrcVSleep();
    } while (index < playerCount);
    path->travelDistance = 0.0f;
    mbMasuNextDispSet(TRUE);
    fn_1_8738();
    lbl_1_bss_18[3] = NULL;
    HuPrcEnd();
}

/* Moves players along the fifth path and bursts particles for unfinished transfers. */
void fn_1_6378(void) {
    HuVecF position;
    HuVecF step;
    HuVecF nextSpace;
    HuVecF firstSpace;
    HuVecF spawn;
    HuVecF velocity;
    s32 players[4];
    HuVecF particlePosition;
    HuVecF particleVelocity;
    GXColor color;
    GXColor particleColor;
    PathBoardModels *work;
    PathPathSpaces *path;
    s32 destinationSpace;
    s32 sound;
    s32 finished;
    s32 playerCount;
    s32 eventStarted;
    s32 particlesEmitted;
    s32 particleIndex;
    s32 frame;
    s32 index;
    GXColor *colorPtr;
    HuVecF *velocityPtr;
    HuVecF *positionPtr;
    float brightness;

    work = lbl_1_bss_8;
    path = &work->paths[4];
    finished = 0;
    eventStarted = 0;
    particlesEmitted = 0;
    fn_1_7814();
    playerCount = fn_1_786C(players, W02_ROUTE_SPACE_ATTR_10_FLAG);
    destinationSpace = mbMasuMAttrFindLink(path->spaceIds[0], W02_ROUTE_SPACE_ATTR_8_FLAG);
    mbMasuPosGet(path->spaceIds[1], &nextSpace);
    mbMasuPosGet(path->spaceIds[0], &firstSpace);
    PSVECSubtract(&firstSpace, &nextSpace, &step);
    PSVECScale(&step, &step, 1.0f / (path->length / 20.0f));
    mbMasuNextDispSet(FALSE);
    work->coinRewardCount += mbRandMod(5) + 8;
    mbObjMotionTimeSet(path->boardModelId, 0.0f);
    mbObjMotionSpeedSet(path->boardModelId, 0.2f);
    sound = mbAudFXPlay(W02_PATH_ARRIVAL_SOUND_ID);
    frame = 0;
    while (!finished) {
        path->travelDistance += 20.0f;
        if (path->travelDistance >= path->length) {
            finished = 1;
        }
        frame++;
        for (index = 0; index < playerCount; index++) {
            if (players[index] != -1) {
                mbPlayerPosGet(players[index], &position);
                PSVECAdd(&position, &step, &position);
                mbPlayerPosSetV(players[index], &position);
                if (frame % 20 == 1) {
                    omVibrate(players[index], 20, 4, 4);
                }
                if (position.z >= path->markerStart.z) {
                    mbPlayerDispSet(players[index], TRUE);
                    fn_1_7A4C(players[index], destinationSpace);
                    players[index] = -1;
                }
            }
        }
        for (index = 0; index < playerCount; index++) {
            if (players[index] != -1) {
                break;
            }
        }
        if (index >= playerCount && fn_1_8338() && !eventStarted) {
            fn_1_6CFC(3);
            eventStarted = 1;
        }
        HuPrcVSleep();
    }
    if (sound != -1) {
        mbAudFXStop(sound);
    }
    for (index = 0; index < playerCount; index++) {
        if (players[index] != -1) {
            mbPlayerDispSet(players[index], TRUE);
            fn_1_7A4C(players[index], destinationSpace);
            mbPlayerPosGet(players[index], &position);
            players[index] = -1;
            for (particleIndex = 0; particleIndex < 32; particleIndex++) {
                spawn.x = position.x + 100.0f * (0.5f * (-0.5f +
                    W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                spawn.y = position.y + 100.0f * (0.5f * (-0.5f +
                    W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                spawn.z = position.z + 100.0f * (0.5f * (-0.5f +
                    W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                velocity.x = 100.0f * (0.1f * (-0.5f +
                    W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                velocity.y = 100.0f * (0.1f * -(W02_RANDOM_FRACTION_SCALE *
                    (float) mbRandMod(W02_RANDOM_MODULUS)));
                velocity.z = 100.0f * (0.1f * (-0.5f +
                    W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                brightness = W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS);
                color.r = 192.0f + 32.0f * brightness;
                color.g = 192.0f + 32.0f * brightness;
                color.b = 192.0f + 32.0f * brightness;
                color.a = 127.0f + 63.0f *
                    (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
                particleColor = color;
                colorPtr = &particleColor;
                particleVelocity = velocity;
                velocityPtr = &particleVelocity;
                particlePosition = spawn;
                positionPtr = &particlePosition;
                fn_1_176E4(positionPtr, velocityPtr,
                           100.0f * (1.0f + 0.5f * (W02_RANDOM_FRACTION_SCALE *
                                                    (float) mbRandMod(W02_RANDOM_MODULUS))),
                           0.5f * (-0.5f + W02_RANDOM_FRACTION_SCALE *
                                               (float) mbRandMod(W02_RANDOM_MODULUS)),
                           0.33f + 0.66f * (W02_RANDOM_FRACTION_SCALE *
                                            (float) mbRandMod(W02_RANDOM_MODULUS)),
                           colorPtr);
                /* The flag is set for each emitted particle; completion does not read it. */
                particlesEmitted = 1;
            }
        }
    }
    while (!fn_1_8338()) {
        HuPrcVSleep();
    }
    if (!eventStarted) {
        fn_1_6CFC(3);
        eventStarted = 1;
    }
    path->travelDistance = 0.0f;
    mbMasuNextDispSet(TRUE);
    lbl_1_bss_18[4] = NULL;
    HuPrcEnd();
}

/* Clears Garage event-process slots and resets event state before dispatch. */
void fn_1_6CA0(void) {
    PositionModel *work = lbl_1_bss_8;
    s32 index;

    for (index = 0; index < 5; index++) {
        lbl_1_bss_18[index] = NULL;
    }
    work->coinRewardCount = 0;
}

/* Starts the Garage event routine assigned to a slot when its dispatcher requests it. */
void fn_1_6CFC(s32 processSlot) {
    *(HUPROCESS **) ((u8 *) &lbl_1_bss_18 + (processSlot * 4)) =
        HuPrcChildCreate(*(void (**)(void))((u8 *) &lbl_1_data_874 + (processSlot * 4)), 8196U,
                         24576U, 0, mbMainProc);
}

/* Yields until every Garage event child process has cleared its slot. */
void fn_1_6D68(void) {
    s32 index;

    do {
        for (index = 0; index < 5; index++) {
            if (*(void **)((u8 *)&lbl_1_bss_18 + index * 4) != NULL) {
                break;
            }
        }
        HuPrcVSleep();
    } while (index < 5);
}

/* Reports whether all Garage event child-process slots are empty for the next dispatch. */
s32 fn_1_6DCC(void) {
    s32 index;

    for (index = 0; index < 5; index++) {
        if (lbl_1_bss_18[index] != NULL) {
            break;
        }
    }
    if (index < 5) {
        return 0;
    }
    return 1;
}

/* Places path markers along the route and associates each with its nearby space. */
void fn_1_6E28(s32 pathIndex, s32 *spaceIds, BOOL createMarkers) {
    HuVecF direction;
    HuVecF position;
    EventPathSpaces *path;
    s32 markerIndex;
    s32 spaceIndex;
    EffectElement *element;
    EffectData *effect;
    HU3D_MODEL *model;
    BoardModels *work;
    float spacing;

    work = lbl_1_bss_8;
    path = &work->paths[pathIndex];
    PSVECSubtract(&path->end, &path->start, &direction);
    spacing = PSVECMag(&direction);
    if (spacing) {
        PSVECNormalize(&direction, &direction);
    }
    spacing /= (float) (path->spaceCount - 1);
    for (markerIndex = 0; markerIndex < path->modelSpaceCount; markerIndex++) {
        PSVECScale(&direction, &position, spacing * (float) markerIndex);
        PSVECAdd(&path->start, &position, &position);
        path->positions[markerIndex] = position;
        path->spaceIds[markerIndex] = -1;
        for (spaceIndex = 0; spaceIndex < path->spaceCount &&
             path->spaceIds[markerIndex] == -1; spaceIndex++) {
            mbMasuPosGet(spaceIds[spaceIndex], &position);
            PSVECSubtract(&position, &path->positions[markerIndex], &position);
            if (PSVECMag(&position) < 50.0f) {
                path->spaceIds[markerIndex] = spaceIds[spaceIndex];
            }
        }
    }
    path->length = spacing * (float) path->spaceCount;
    path->travelDistance = 0.0f;
    PSVECAdd(&path->end, &lbl_1_data_888[pathIndex], &path->end);
    PSVECAdd(&path->start, &lbl_1_data_8C4[pathIndex], &path->markerStart);
    if (!createMarkers) {
        path->modelId = -1;
        return;
    }
    path->modelId = fn_1_15E30(HuSprAnimRead(HuDataSelHeapReadNum(DATA_bmasu,
                              HU_MEMNUM_OVL, HEAP_MODEL)), path->modelSpaceCount);
    fn_1_16E44(path->modelId, lbl_1_data_900, lbl_1_data_7F4);
    model = &Hu3DData[path->modelId];
    effect = model->hookData;
    effect->flags |= 0xC;
    element = effect->elements;
    for (markerIndex = 0; markerIndex < path->modelSpaceCount; markerIndex++, element++) {
        element->type = 3;
        element->value = 200.0f;
        element->rotation = 0.0f;
        element->position = path->positions[markerIndex];
        element->position.y += 3.0f;
        if (markerIndex < path->spaceCount + 1) {
            element->color.a = 255;
        } else {
            element->color.a = 0;
        }
    }
}

/* Swaps blue and red route spaces after movement or while restoring a saved route state. */
void fn_1_71B4(s32 pathIndex) {
    SpacePathSpaces *path;
    s32 spaceIndex;
    BoardPaths *work;

    work = lbl_1_bss_8;
    path = &work->paths[pathIndex];
    for (spaceIndex = 0; spaceIndex < path->spaceCount; spaceIndex++) {
        switch (mbMasuTypeGet(path->spaceIds[spaceIndex])) {
            case 1:
                mbMasuTypeSet(path->spaceIds[spaceIndex], 2);
                break;
            case 2:
                mbMasuTypeSet(path->spaceIds[spaceIndex], 1);
                break;
        }
    }
}

/* Prepares a route's colored space markers for display using the current board-space types. */
void fn_1_7280(s32 pathIndex) {
    BoardPaths *work;
    SpacePathSpaces *path;
    s32 spaceIndex;
    SpaceEffect *effect;
    HU3D_MODEL *model;
    SpaceElement *element;

    work = lbl_1_bss_8;
    path = &work->paths[pathIndex];
    model = &Hu3DData[path->modelId];
    effect = model->hookData;
    element = effect->elements;
    for (spaceIndex = 0; spaceIndex < path->modelSpaceCount; spaceIndex++, element++) {
        if (spaceIndex < path->spaceCount) {
            path->spaceTypes[spaceIndex] = mbMasuTypeGet(path->spaceIds[spaceIndex]);
        } else {
            /* The second half previews the swapped route: blue and red exchange types; other types
             * repeat. */
            switch (path->spaceTypes[spaceIndex % path->spaceCount]) {
                case 1:
                    path->spaceTypes[spaceIndex] = 2;
                    break;
                case 2:
                    path->spaceTypes[spaceIndex] = 1;
                    break;
                default:
                    path->spaceTypes[spaceIndex]
                        = path->spaceTypes[spaceIndex % path->spaceCount];
                    break;
            }
        }
        switch (path->spaceTypes[spaceIndex]) {
            case 1:
                element->type = 0;
                break;
            case 2:
                element->type = 1;
                break;
            case 3:
                element->type = 2;
                break;
            default:
                element->type = 3;
                break;
        }
    }
}

/* Refreshes marker opacity while all five route-event process slots are empty. */
void fn_1_7418(OMOBJ *object) {
    SpaceGroup *group;
    MarkerParticle *particle;
    s32 eventIndex;
    s32 particleIndex;
    MarkerHook *hook;
    HU3D_MODEL *model;
    BOOL refresh;
    SpaceEffectWork *work;

    work = lbl_1_bss_8;
    if (mbExitCheck()) {
        omDelObjEx(mbObjMan, object);
        return;
    }
    group = &work->groups[object->work[0]];
    for (eventIndex = 0; eventIndex < 5; eventIndex++) {
        if (lbl_1_bss_18[eventIndex] != NULL) {
            break;
        }
    }
    if (eventIndex < 5) {
        refresh = FALSE;
    } else {
        refresh = TRUE;
    }
    if (refresh) {
        model = &Hu3DData[group->modelId];
        hook = model->hookData;
        particle = hook->particles;
        for (particleIndex = 0; particleIndex < group->spaceCount; particleIndex++, particle++) {
            if (mbCapMasuDispTypeGet(group->spaceIds[particleIndex]) == 1) {
                particle->alpha = 0;
            } else {
                particle->alpha = W02_PATH_PARTICLE_ALPHA_MAX;
            }
        }
    }
}

/* Updates moving-route space transforms and removes capsules from spaces whose model elements
 * have alpha below 255. */
void fn_1_7548(s32 pathIndex) {
    Mtx matrix;
    SpaceElement *element;
    SpacePathSpaces *path;
    s32 spaceIndex;
    SpaceEffect *effect;
    HU3D_MODEL *model;
    BoardPaths *work;

    work = lbl_1_bss_8;
    path = &work->paths[pathIndex];
    PSMTXIdentity(matrix);
    model = &Hu3DData[path->modelId];
    effect = model->hookData;
    element = effect->elements;
    for (spaceIndex = 0; spaceIndex < path->spaceCount; spaceIndex++, element++) {
        matrix[0][3] = element->position.x;
        matrix[1][3] = element->position.y;
        matrix[2][3] = element->position.z;
        mbMasuMtxSet(path->spaceIds[spaceIndex], matrix);
        if (element->color.a < 255) {
            mbMasuCapsuleSet(path->spaceIds[spaceIndex], -1);
        }
    }
}

/* Creates four model-heap sprite effects and installs their board update object. */
void fn_1_762C(void) {
    BoardSpriteModels *work;
    s32 index;
    spriteinitializeSpriteHook *hook;
    HU3D_MODEL *model;

    work = lbl_1_bss_8;
    for (index = 0; index < 4; index++) {
        work->models[index] = fn_1_15E30(
            HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_bmasu, 0), HU_MEMNUM_OVL, HEAP_MODEL)),
            16);
        fn_1_16E44(work->models[index], lbl_1_data_930, lbl_1_data_960);
        Hu3DModelLayerSet(work->models[index], 2);
        model = &Hu3DData[work->models[index]];
        hook = model->hookData;
        hook->flags |= 0xC;
    }
    omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_774C);
}

/* Clears every particle's render size in the four path sprite models until board exit. */
void fn_1_774C(OMOBJ *object) {
    EffectData *effect;
    EffectElement *element;
    s32 modelIndex;
    s32 elementIndex;
    BoardModels *work;
    HU3D_MODEL *model;

    work = lbl_1_bss_8;
    if (mbExitCheck()) {
        omDelObjEx(mbObjMan, object);
        return;
    }
    for (modelIndex = 0; modelIndex < 4; modelIndex++) {
        model = &Hu3DData[work->particleModels[modelIndex]];
        effect = model->hookData;
        element = effect->elements;
        for (elementIndex = 0; elementIndex < effect->elementCount; elementIndex++, element++) {
            element->value = 0.0f;
        }
    }
}

/* Clears the four player object and motion slots before a path-transfer event collects riders. */
void fn_1_7814(void) {
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 4) {
        *(s32 *) ((u8 *) &lbl_1_bss_4C + (playerIndex * 4)) = 0;
        *(s32 *) ((u8 *) &lbl_1_bss_3C + (playerIndex * 4)) = -1;
        playerIndex += 1;
    }
}

/* Finds riders whose spaces match an event mask and starts their motions before route transfer. */
s32 fn_1_786C(s32 *playerList, s32 attributeMask) {
    s32 playerCount;
    s32 playerIndex;
    HU3D_MODEL *model;

    playerIndex = 0;
    playerCount = 0;
    while (playerIndex < 4) {
        if ((u32)(attributeMask & mbMasuMAttrGet(GwPlayer[playerIndex].masuId)) != 0) {
            mbPlayerColSnapPlayerSet(playerIndex, 0);
            playerList[playerCount] = playerIndex;
            playerCount++;
            if (mbPlayerMotionGet(playerIndex) != 6) {
                if (playerIndex != (s8)GwSystem.turnPlayerNo ||
                    ((u32)(mbMasuMAttrGet(GwPlayer[playerIndex].masuId) & 0x40) != 0)) {
                    lbl_1_bss_3C[playerIndex] =
                        mbPlayerMotionCreate(playerIndex, DATANUM(DATA_mariomot, 34));
                    mbPlayerMotionShiftSet(playerIndex, lbl_1_bss_3C[playerIndex],
                                           0.0f, 8.0f,
                                           HU3D_MOTATTR_LOOP);
                } else {
                    lbl_1_bss_3C[playerIndex] =
                        mbPlayerMotionCreate(playerIndex, DATANUM(DATA_mariomot, 18));
                    mbPlayerMotionShiftSet(playerIndex, lbl_1_bss_3C[playerIndex],
                                           0.0f, 8.0f, 0);
                }
                model = &Hu3DData[mbObjModelIDGet(mbPlayerObjIDGet(playerIndex))];
                model->motShiftWork.speed = 2.0f;
            }
        }
        playerIndex++;
    }
    return playerCount;
}

/* Searches for a free landing offset and starts the player transfer.
 * Riders from the third path keep the destination center instead of the selected offset. */
void fn_1_7A4C(s32 playerNo, s32 destinationSpace) {
    HuVecF otherPosition;
    HuVecF direction;
    PlayerTravel *travel;
    OMOBJ *object;
    s32 offsetIndex;
    s32 otherPlayer;
    float distance;

    object = lbl_1_bss_4C[playerNo] = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_7FD4);
    object->data = travel = HuMemDirectMallocNum(HEAP_HEAP, sizeof(PlayerTravel), HU_MEMNUM_OVL);
    memset(travel, 0, sizeof(PlayerTravel));
    travel->playerNo = playerNo;
    travel->destinationSpace = destinationSpace;
    travel->elapsedFrames = 0;
    travel->travelFrames = 0;
    travel->jumpHeight = 150.0f;
    if (mbMasuMAttrGet(GwPlayer[playerNo].masuId) & 0x400) {
        travel->jumpHeight = 0.0f;
    } else if (mbMasuMAttrGet(GwPlayer[playerNo].masuId) & 0x200) {
        travel->jumpHeight *= 2.0f;
    }
    mbPlayerPosGet(playerNo, &travel->start);
    mbMasuPosGet(destinationSpace, &travel->destination);
    lbl_1_bss_2C[playerNo] = -1;
    for (offsetIndex = 0; offsetIndex < 9; offsetIndex++) {
        for (otherPlayer = 0; otherPlayer < 4; otherPlayer++) {
            if (otherPlayer == playerNo ||
                GwPlayer[otherPlayer].masuId != GwPlayer[playerNo].masuId ||
                offsetIndex != lbl_1_bss_2C[otherPlayer]) {
                continue;
            }
            break;
        }
        if (otherPlayer < 4) {
            continue;
        }
        for (otherPlayer = 0; otherPlayer < 4; otherPlayer++) {
            if (otherPlayer == playerNo || destinationSpace != GwPlayer[otherPlayer].masuId) {
                continue;
            }
            mbPlayerPosGet(otherPlayer, &otherPosition);
            PSVECAdd(&travel->destination, &lbl_1_data_9E0[offsetIndex], &direction);
            PSVECSubtract(&otherPosition, &direction, &direction);
            if (PSVECMag(&direction) < 75.0f) {
                break;
            }
        }
        if (otherPlayer < 4) {
            continue;
        }
        break;
    }
    /* For other paths, a failed search still indexes the nine-entry offset table at 9. */
    if (!(mbMasuMAttrGet(GwPlayer[playerNo].masuId) & 0x200)) {
        lbl_1_bss_2C[playerNo] = offsetIndex;
        PSVECAdd(&travel->destination, &lbl_1_data_9E0[offsetIndex], &travel->destination);
    }
    if (fabs(travel->start.y - travel->destination.y) < 100.0 ||
        (mbMasuMAttrGet(GwPlayer[playerNo].masuId) & 0x400) ||
        (mbMasuMAttrGet(GwPlayer[playerNo].masuId) & 0x200)) {
        travel->upright = TRUE;
    } else {
        travel->upright = FALSE;
    }
    PSVECSubtract(&travel->start, &travel->destination, &direction);
    distance = PSVECMag(&direction);
    if (distance > 0.0f) {
        travel->travelFrames = distance / 20.0f;
    }
    /* Keep the base trip at least 24 frames before adding the character's timing adjustment. */
    if ((float) travel->travelFrames < 24.0f) {
        travel->travelFrames = 24;
    }
    travel->travelFrames = (float) travel->travelFrames +
        6.0f * lbl_1_data_A4C[GwPlayer[playerNo].charNo];
    if (mbMasuMAttrGet(destinationSpace) & 0x400) {
        CharMotionUpdateSet(GwPlayer[playerNo].charNo, DATANUM(DATA_mariomot, 22), FALSE);
    } else {
        CharMotionUpdateSet(GwPlayer[playerNo].charNo, DATANUM(DATA_mariomot, 22), TRUE);
    }
    mbPlayerMotionShiftSet(playerNo, 6, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    PSVECSubtract(&travel->destination, &travel->start, &direction);
    travel->facingAngle = 180.0 * (atan2(direction.x, direction.z) / M_PI);
}

/* Carries a player between spaces with a jump arc. Arrival restores player snapping, frees the
 * temporary motion, and hides the player at fifth-path destinations. */
void fn_1_7FD4(OMOBJ *object) {
    HuVecF position;
    HuVecF rotation;
    HuVecF delta;
    float progress;
    float distance;
    PlayerTravel *travel;

    travel = object->data;
    if (mbExitCheck() || lbl_1_bss_4C[travel->playerNo] == NULL) {
        omDelObjEx(mbObjMan, object);
        return;
    }
    progress = (float) ++travel->elapsedFrames / (float) travel->travelFrames;
    PSVECSubtract(&travel->destination, &travel->start, &delta);
    distance = PSVECMag(&delta);
    if (distance > 0.0f) {
        PSVECNormalize(&delta, &delta);
    }
    PSVECScale(&delta, &delta, distance * progress);
    PSVECAdd(&travel->start, &delta, &position);
    position.y += travel->jumpHeight *
        sin((3.141592653589793 * (180.0f * progress)) / 180.0);
    mbPlayerPosSetV(travel->playerNo, &position);
    mbPlayerRotGet(travel->playerNo, &rotation);
    if (!travel->upright) {
        rotation.x = 360.0f * progress * progress;
    } else {
        rotation.x = 0.0f;
    }
    if (!travel->upright) {
        rotation.y = mbev_CapAngleLerp(travel->facingAngle, rotation.y, 5.0f);
    } else {
        rotation.y = mbev_CapAngleLerp(travel->facingAngle, rotation.y, 1.0f);
    }
    rotation.z = 0.0f;
    mbPlayerRotSetV(travel->playerNo, &rotation);
    if (progress >= 1.0f) {
        GwPlayer[travel->playerNo].masuId = travel->destinationSpace;
        mbPlayerColSnapPlayerSet(travel->playerNo, TRUE);
        if (mbMasuMAttrGet(travel->destinationSpace) & 0x400) {
            mbPlayerDispSet(travel->playerNo, FALSE);
        }
        if (lbl_1_bss_3C[travel->playerNo] != -1) {
            mbPlayerMotionKill(travel->playerNo, lbl_1_bss_3C[travel->playerNo]);
        }
        lbl_1_bss_3C[travel->playerNo] = -1;
        lbl_1_bss_2C[travel->playerNo] = -1;
        lbl_1_bss_4C[travel->playerNo] = NULL;
        omDelObjEx(mbObjMan, object);
    }
}

/* Reports whether all four player-transfer object slots are empty. */
s32 fn_1_8338(void) {
    s32 index;

    for (index = 0; index < 4; index++) {
        if (*(void **)((u8 *)&lbl_1_bss_4C + index * 4) != NULL) {
            return 0;
        }
    }
    return 1;
}

/* Creates the Garage coin-effect manager and initializes its 64 coin slots before awards begin. */
void fn_1_838C(void) {
    BoardEntry *entry;
    s32 index;
    OMOBJ *object;

    object = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_8440);
    lbl_1_bss_10 = object;
    entry = object->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(BoardEntry) * 64, HU_MEMNUM_OVL);
    memset(entry, 0, sizeof(BoardEntry) * 64);
    for (index = 0; index < 64; index++, entry++) {
        entry->objectId = -1;
    }
}

/* Moves emitted coins toward their player, then awards each coin when its chase ends. */
void fn_1_8440(OMOBJ *object) {
    BoardEntry *entry;
    s32 index;
    HuVecF effectPosition;
    float progress;

    entry = object->data;
    if (mbExitCheck() || lbl_1_bss_10 == NULL) {
        for (index = 0; index < 64; index++, entry++) {
            if (entry->objectId != -1) {
                mbCoinObjNumDec(entry->objectId);
            }
        }
        omDelObjEx(mbObjMan, object);
        return;
    }
    for (index = 0; index < 64; index++, entry++) {
        if (entry->active) {
            entry->rotation.y += entry->spinSpeed;
            switch (entry->state) {
            case 0:
                PSVECAdd(&entry->position, &entry->velocity, &entry->position);
                mbCoinObjPosSetV(entry->objectId, &entry->position);
                mbCoinObjRotSetV(entry->objectId, &entry->rotation);
                if (++entry->counter >= entry->duration) {
                    entry->counter = 0;
                    entry->duration = 21;
                    entry->state++;
                    entry->start = entry->position;
                    mbPlayerPosGet(entry->playerNo, &entry->end);
                    entry->end.y += 100.0f;
                }
                break;
            case 1:
                progress = (float) ++entry->counter / (float) entry->duration;
                mbev_CapVecChase(progress, &entry->start, &entry->end, &entry->position);
                /* Replace the chase's linear Y movement with a sine curve toward the player's
                 * target height. */
                entry->position.y = entry->start.y + (entry->end.y - entry->start.y) *
                    sin((3.141592653589793 * (90.0f * progress)) / 180.0);
                mbCoinObjPosSetV(entry->objectId, &entry->position);
                mbCoinObjRotSetV(entry->objectId, &entry->rotation);
                if (progress >= 1.0f) {
                    effectPosition = entry->position;
                    mbCoinEffCreate(&effectPosition);
                    mbPlayerCoinAdd(entry->playerNo, 1);
                    mbAudFXPlay(7);
                    entry->active = FALSE;
                    mbCoinObjNumDec(entry->objectId);
                    entry->objectId = -1;
                }
                break;
            }
        }
    }
}

void fn_1_8738(void) {
    lbl_1_bss_10 = 0;
}

/* Counts occupied path-effect entries; the board event waits for this count to clear. */
s32 fn_1_874C(void) {
    s32 *entryData;
    s32 entryIndex;
    s32 activeCount;
    if ((void *) lbl_1_bss_10 == NULL)
    {
        return 0;
    }
    entryData = (*(s32 * *)((s8 *)(lbl_1_bss_10) + (92)));
    entryIndex = 0;
    activeCount = 0;
    while (entryIndex < 64)
    {
        if ((s32) * entryData != 0)
        {
            activeCount += 1;
        }
        entryIndex += 1;
        entryData += 22;
    }
    return activeCount;
}

/* Queues a coin at its starting position with an initial velocity when a path event awards
 * coins. */
void fn_1_87D0(HuVecF *start, HuVecF *velocity, s32 playerNo, s32 duration) {
    BoardEntry *effect;
    s32 index;

    if (lbl_1_bss_10 == NULL) {
        return;
    }
    effect = lbl_1_bss_10->data;
    for (index = 0; index < 64; index++, effect++) {
        if (!effect->active) {
            break;
        }
    }
    if (index >= 64) {
        return;
    }
    effect->active = TRUE;
    effect->objectId = (s16) mbCoinCreate();
    effect->state = 0;
    effect->counter = 0;
    effect->duration = duration;
    effect->playerNo = playerNo;
    effect->spinSpeed = 10.0f *
        (-0.5f + W02_RANDOM_FRACTION_SCALE * (f32) (u32) mbRandMod(W02_RANDOM_MODULUS));
    effect->position = *start;
    effect->velocity = *velocity;
    effect->rotation.x = effect->rotation.z = 0.0f;
    effect->rotation.y = 360.0f *
        (W02_RANDOM_FRACTION_SCALE * (f32) (u32) mbRandMod(W02_RANDOM_MODULUS));
    /* This initial target is replaced with the player's position when the coin starts its chase. */
    mbPlayerPosGet(playerNo, &effect->end);
    mbCoinObjPosSetV((s16) effect->objectId, &effect->position);
    mbCoinObjRotSetV((s16) effect->objectId, &effect->rotation);
}

/* Builds markers for linked spaces and keeps the event's five supporting models hidden. */
void fn_1_89A8(void) {
    s32 modelId;
    BoardModels *work;
    EventMapMarker *marker;
    s32 spaceId;
    s32 linkedSpace;
    s32 markerCount;
    HuVecF position;

    work = lbl_1_bss_8;
    marker = work->markers;
    spaceId = 1;
    markerCount = 0;
    for (; spaceId < mbMasuNumGet(); spaceId++) {
        if (mbMasuMAttrGet(spaceId) & 2) {
            marker->modelId = modelId =
                mbObjCreate(BoardModelDataNum(DATANUM(DATA_w02, 23)), NULL, TRUE);
            mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
            mbMasuPosGet(spaceId, &position);
            mbObjPosSetV(modelId, &position);
            marker->spaceId = spaceId;
            for (linkedSpace = 1; linkedSpace < mbMasuNumGet(); linkedSpace++) {
                if (spaceId == mbMasuMAttrFindLink(linkedSpace, 2)) {
                    break;
                }
            }
            marker->linkedSpace = linkedSpace;
            markerCount++;
            marker++;
        }
    }
    work->markerCount = markerCount;
    work->markerEffectModels[0] = modelId =
        mbObjCreate(BoardModelDataNum(DATANUM(DATA_w02, 24)), NULL, FALSE);
    mbObjDispSet(modelId, FALSE);
    work->markerEffectModels[1] = modelId =
        mbObjCreate(BoardModelDataNum(DATANUM(DATA_w02, 25)), NULL, FALSE);
    mbObjDispSet(modelId, FALSE);
    work->markerEffectModels[2] = modelId =
        mbObjCreate(BoardModelDataNum(DATANUM(DATA_w02, 26)), NULL, FALSE);
    mbObjDispSet(modelId, FALSE);
    work->markerEffectModels[3] = modelId =
        mbObjCreate(BoardModelDataNum(DATANUM(DATA_w02, 27)), NULL, FALSE);
    mbObjDispSet(modelId, FALSE);
    work->markerEffectModels[4] = modelId =
        mbObjCreate(BoardModelDataNum(DATANUM(DATA_w02, 28)), NULL, FALSE);
    mbObjDispSet(modelId, FALSE);
}

/* Offers the station trip and animates the player and transfer effects after acceptance. */
void fn_1_8C9C(int playerNo, int spaceId) {
    HuVecF start;
    HuVecF end;
    HuVecF position;
    int travelMotion[1];
    int departureMotion[1];
    s16 departureShell[1];
    s16 departureBeam[1];
    s16 outerTrail[1];
    s16 innerTrail[1];
    s16 arrivalBeam[1];
    s16 arrivalShell[1];
    TransferWork *work;
    EventMapMarker *marker;
    int sourceSpace;
    int linkedStarDistance;
    s16 effectModel;
    int stationIndex;
    int markerIndex;
    int choice;
    int destinationSpace;
    int currentStarDistance;
    int frame;
    int travelSound;
    BOOL effectMoved;
    float progress;
    float motionDuration;

    work = lbl_1_bss_8;
    marker = &work->markers[0];
    sourceSpace = mbMasuMAttrFindLink(spaceId, 2);
    stationIndex = fn_1_A4B4(sourceSpace);
    markerIndex = lbl_1_data_A84[stationIndex];
    destinationSpace = fn_1_A51C(markerIndex);
    mbPlayerRotateStart(playerNo, 180, 15);
    while (!mbPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    fn_1_AA04(lbl_1_data_A90[stationIndex]);
    mbObjMotionShiftSet(work->attachedModel, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    do {
        if (GwPlayer[playerNo].charNo == 1 && lbl_1_bss_4->priorState[3] == 0) {
            mbAudFXPlay(W02_STATION_CHOICE_SOUND_ID);
            mbWinCreateChoice(1, MESSNUM(MESS_BOARD_W02, 1), 14, 0);
            lbl_1_bss_4->priorState[3] = 1;
        } else {
            mbAudFXPlay(W02_STATION_SELECTION_SOUND_ID);
            mbWinCreateChoice(1, MESSNUM(MESS_BOARD_W02, 0), 14, 0);
        }
        mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo), 0);
        if (GwPlayer[playerNo].comF) {
            if (GwPlayer[playerNo].comDif < 2) {
                currentStarDistance = mbMasuFind_TypeStepGet(spaceId, 7);
                if (currentStarDistance > 10 &&
                    W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS) > 0.3f) {
                    mbComChoiceUpSet();
                } else {
                    mbComChoiceDownSet();
                }
            } else {
                marker = &work->markers[markerIndex];
                currentStarDistance = mbMasuFind_TypeStepGet(spaceId, 7);
                linkedStarDistance = mbMasuFind_TypeStepGet(marker->linkedSpace, 7);
                if (currentStarDistance > linkedStarDistance) {
                    mbComChoiceUpSet();
                } else {
                    mbComChoiceDownSet();
                }
            }
        }
        mbWinTopWait();
        if ((choice = mbWinTopChoiceGet()) == 2) {
            mbev_Scroll(playerNo, 0);
        }
    } while (choice == 2);
    if (choice == 1 || choice == -1) {
        mbAudFXPlay(W02_STATION_EXIT_SOUND_ID);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 3), 14);
        mbWinTopWait();
        mbObjMotionShiftSet(work->attachedModel, 4, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_AC34(lbl_1_data_A90[stationIndex]);
        mbPlayerRotateStart(playerNo, 0, 15);
        while (!mbPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
        return;
    }
    mbAudFXPlay(W02_STATION_ACCEPT_SOUND_ID);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 2), 14);
    mbWinTopWait();
    mbObjMotionShiftSet(work->attachedModel, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbPlayerPosGet(playerNo, &start);
    mbMasuPosGet(sourceSpace, &end);
    PSVECSubtract(&end, &start, &position);
    mbPlayerRotateStart(playerNo, 180.0 * (atan2(position.x, position.z) / M_PI), 15);
    while (!mbPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    mbPlayerPosGet(playerNo, &start);
    mbMasuPosGet(sourceSpace, &end);
    mbPlayerColSnapPlayerSet(playerNo, 0);
    mbPlayerMotionShiftSet(playerNo, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    for (frame = 0; frame <= 30.0f; frame++) {
        progress = (float) frame / 30.0f;
        position.x = start.x + progress * (end.x - start.x);
        position.y = start.y + progress * (end.y - start.y);
        position.z = start.z + progress * (end.z - start.z);
        mbPlayerPosSetV(playerNo, &position);
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbPlayerRotateStart(playerNo, 0, 15);
    while (!mbPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    HuPrcSleep(2);
    departureMotion[0] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mario, 33));
    HuPrcSleep(2);
    travelMotion[0] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 26));
    HuPrcSleep(2);
    fn_1_AB78(lbl_1_data_A90[stationIndex]);
    mbMasuPosGet(sourceSpace, &start);
    mbMasuPosGet(destinationSpace, &end);
    mbAudFXPlay(W02_PATH_DEPARTURE_SOUND_ID);
    mbObjPosSetV(work->markerEffectModels[0], &start);
    mbObjMotionTimeSet(work->markerEffectModels[0], 0.0f);
    effectModel = work->markerEffectModels[0];
    mbObjAttrSet(effectModel, HU3D_MOTATTR_LOOP);
    mbObjLayerSet(work->markerEffectModels[0], 4);
    mbObjDispSet(work->markerEffectModels[0], 1);
    for (frame = 1; frame < 30.0f; frame++) {
        progress = (float) frame / 30.0f;
        mbObjScaleSet(work->markerEffectModels[0], 1.0f, progress, 1.0f);
        HuPrcVSleep();
    }
    mbAudFXPlay(W02_PATH_TRANSFER_SOUND_ID);
    omVibrate(playerNo, 20, 7, 3);
    mbObjPosSetV(work->markerEffectModels[1], &start);
    mbObjMotionTimeSet(work->markerEffectModels[1], 0.0f);
    departureShell[0] = work->markerEffectModels[1];
    mbObjAttrSet(departureShell[0], HU3D_MOTATTR_LOOP);
    mbObjLayerSet(work->markerEffectModels[1], 3);
    mbObjDispSet(work->markerEffectModels[1], 1);
    for (frame = 1; frame < 30.0f; frame++) {
        progress = (float) frame / 30.0f;
        mbObjScaleSet(work->markerEffectModels[1], 1.0f, progress, 1.0f);
        HuPrcVSleep();
    }
    for (frame = 0; frame < 30.0f; frame++) {
        progress = 1.0f - (float) frame / 30.0f;
        mbPlayerScaleSet(playerNo, progress, 1.0f, progress);
        mbObjScaleSet(work->markerEffectModels[1], progress, 1.0f, progress);
        HuPrcVSleep();
    }
    mbObjDispSet(work->markerEffectModels[1], 0);
    mbPlayerDispSet(playerNo, 0);
    mbObjPosSetV(work->markerEffectModels[2], &start);
    mbObjMotionTimeSet(work->markerEffectModels[2], 0.0f);
    departureBeam[0] = work->markerEffectModels[2];
    mbObjAttrReset(departureBeam[0], HU3D_MOTATTR_REV);
    mbObjLayerSet(work->markerEffectModels[2], 5);
    mbObjMotionSpeedSet(work->markerEffectModels[2], 2.0f);
    mbObjDispSet(work->markerEffectModels[2], 1);
    fn_1_A554(playerNo);
    for (frame = 0; frame < 30.0f; frame++) {
        progress = (float) frame / 30.0f;
        position.x = start.x;
        position.y = start.y + 800.0f * progress;
        position.z = start.z;
        mbPlayerPosSetV(playerNo, &position);
        HuPrcVSleep();
    }
    mbPlayerPosGet(playerNo, &position);
    mbObjPosSetV(work->markerEffectModels[3], &position);
    mbObjScaleSet(work->markerEffectModels[3], 2.0f, 2.0f, 2.0f);
    mbObjMotionTimeSet(work->markerEffectModels[3], 0.0f);
    outerTrail[0] = work->markerEffectModels[3];
    mbObjAttrSet(outerTrail[0], HU3D_MOTATTR_LOOP);
    mbObjLayerSet(work->markerEffectModels[3], 4);
    mbObjDispSet(work->markerEffectModels[3], 1);
    if (GwPlayer[playerNo].charNo != 5) {
        mbObjAlphaSet(work->markerEffectModels[3], 127);
    } else {
        mbObjAlphaSet(work->markerEffectModels[3], 96);
    }
    mbPlayerPosGet(playerNo, &position);
    mbObjPosSetV(work->markerEffectModels[4], &position);
    mbObjScaleSet(work->markerEffectModels[4], 2.0f, 2.0f, 2.0f);
    mbObjMotionTimeSet(work->markerEffectModels[4], 0.0f);
    innerTrail[0] = work->markerEffectModels[4];
    mbObjAttrSet(innerTrail[0], HU3D_MOTATTR_LOOP);
    mbObjLayerSet(work->markerEffectModels[4], 3);
    mbObjDispSet(work->markerEffectModels[4], 1);
    mbPlayerMotionSet(playerNo, departureMotion[0], HU3D_MOTATTR_LOOP);
    PSVECSubtract(&end, &start, &position);
    mbPlayerRotSet(playerNo, 0.0f, 180.0 * (atan2(position.x, position.z) / M_PI), 0.0f);
    mbPlayerDispSet(playerNo, 1);
    while (!mbObjMotionEndCheck(work->markerEffectModels[2])) {
        motionDuration = mbObjMotionMaxTimeGet(work->markerEffectModels[2]);
        progress = mbObjMotionTimeGet(work->markerEffectModels[2]) / motionDuration;
        /* Keep the player and both trail models at least one tenth of their full departure
         * scale. */
        if (progress <= 0.1f) {
            progress = 0.1f;
        }
        mbPlayerScaleSet(playerNo, progress, progress, progress);
        mbObjScaleSet(work->markerEffectModels[3], 2.0f * progress,
                      2.0f * progress, 2.0f * progress);
        mbObjScaleSet(work->markerEffectModels[4], 2.0f * progress,
                      2.0f * progress, 2.0f * progress);
        HuPrcVSleep();
    }
    mbObjDispSet(work->markerEffectModels[2], 0);
    mbPlayerScaleSet(playerNo, 1.0f, 1.0f, 1.0f);
    mbObjScaleSet(work->markerEffectModels[3], 2.0f, 2.0f, 2.0f);
    mbObjScaleSet(work->markerEffectModels[4], 2.0f, 2.0f, 2.0f);
    mbPlayerMotionShiftSet(playerNo, travelMotion[0], 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbPlayerMotionSpeedSet(playerNo, 2.0f);
    travelSound = mbAudFXPlay(W02_PATH_FLIGHT_SOUND_ID);
    frame = 0;
    effectMoved = 0;
    for (; frame < 180.0f; frame++) {
        progress = (float) frame / 180.0f;
        position.x = start.x + progress * (end.x - start.x);
        position.y = 800.0f + (start.y + progress * (end.y - start.y));
        position.z = start.z + progress * (end.z - start.z);
        mbPlayerPosSetV(playerNo, &position);
        mbObjPosSetV(work->markerEffectModels[3], &position);
        mbObjPosSetV(work->markerEffectModels[4], &position);
        if (effectMoved == 0 && progress >= 0.5f) {
            mbObjPosSetV(work->markerEffectModels[0], &end);
            effectMoved = 1;
        }
        HuPrcVSleep();
    }
    if (travelSound != -1) {
        mbAudFXStop(travelSound);
    }
    mbObjPosSetV(work->markerEffectModels[2], &end);
    mbObjMotionTimeSet(work->markerEffectModels[2],
                      mbObjMotionMaxTimeGet(work->markerEffectModels[2]));
    mbObjLayerSet(work->markerEffectModels[2], 5);
    arrivalBeam[0] = work->markerEffectModels[2];
    mbObjAttrSet(arrivalBeam[0], HU3D_MOTATTR_REV);
    mbObjMotionSpeedSet(work->markerEffectModels[2], 2.0f);
    mbObjDispSet(work->markerEffectModels[2], 1);
    for (frame = 0; frame < 15.0; frame++) {
        progress = 1.0 - (float) frame / 15.0;
        mbPlayerScaleSet(playerNo, progress, progress, progress);
        mbObjScaleSet(work->markerEffectModels[3], 2.0f * progress,
                      2.0f * progress, 2.0f * progress);
        mbObjScaleSet(work->markerEffectModels[4], 2.0f * progress,
                      2.0f * progress, 2.0f * progress);
        HuPrcVSleep();
    }
    mbPlayerDispSet(playerNo, 0);
    mbObjDispSet(work->markerEffectModels[3], 0);
    mbObjDispSet(work->markerEffectModels[4], 0);
    mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
    mbPlayerMotionKill(playerNo, departureMotion[0]);
    mbPlayerMotionKill(playerNo, travelMotion[0]);
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    for (frame = 0; frame < 30.0f; frame++) {
        progress = (float) frame / 30.0f;
        position.x = end.x;
        position.y = end.y + 800.0f * (1.0f - progress);
        position.z = end.z;
        mbPlayerPosSetV(playerNo, &position);
        HuPrcVSleep();
    }
    mbCameraMoveWait();
    mbObjDispSet(work->markerEffectModels[2], 0);
    mbPlayerDispSet(playerNo, 1);
    fn_1_A7BC(playerNo);
    mbAudFXPlay(W02_PATH_TRANSFER_SOUND_ID);
    omVibrate(playerNo, 20, 7, 3);
    mbObjPosSetV(work->markerEffectModels[1], &end);
    mbObjMotionTimeSet(work->markerEffectModels[1], 0.0f);
    arrivalShell[0] = work->markerEffectModels[1];
    mbObjAttrSet(arrivalShell[0], HU3D_MOTATTR_LOOP);
    mbObjLayerSet(work->markerEffectModels[1], 3);
    mbObjDispSet(work->markerEffectModels[1], 1);
    for (frame = 1; frame < 30.0f; frame++) {
        progress = (float) frame / 30.0f;
        mbPlayerScaleSet(playerNo, progress, 1.0f, progress);
        mbObjScaleSet(work->markerEffectModels[1], progress, 1.0f, progress);
        HuPrcVSleep();
    }
    for (frame = 0; frame < 30.0f; frame++) {
        progress = 1.0f - (float) frame / 30.0f;
        mbObjScaleSet(work->markerEffectModels[1], 1.0f, progress, 1.0f);
        HuPrcVSleep();
    }
    mbObjDispSet(work->markerEffectModels[1], 0);
    for (frame = 0; frame < 30.0f; frame++) {
        progress = 1.0f - (float) frame / 30.0f;
        mbObjScaleSet(work->markerEffectModels[0], 1.0f, progress, 1.0f);
        HuPrcVSleep();
    }
    mbObjDispSet(work->markerEffectModels[0], 0);
    mbObjDispSet(work->markerEffectModels[1], 0);
    mbObjDispSet(work->markerEffectModels[2], 0);
    mbObjDispSet(work->markerEffectModels[3], 0);
    fn_1_ACF4(lbl_1_data_A90[stationIndex]);
    marker = &work->markers[markerIndex];
    mbMasuPosGet(destinationSpace, &start);
    mbMasuPosGet(marker->linkedSpace, &end);
    GwPlayer[playerNo].masuIdNext = marker->linkedSpace;
    PSVECSubtract(&end, &start, &position);
    mbPlayerRotSet(playerNo, 0.0f, 180.0 * (atan2(position.x, position.z) / M_PI), 0.0f);
    mbPlayerMotionShiftSet(playerNo, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    for (frame = 0; frame < 30.0f; frame++) {
        progress = (float) frame / 30.0f;
        position.x = start.x + progress * (end.x - start.x);
        position.y = start.y + progress * (end.y - start.y);
        position.z = start.z + progress * (end.z - start.z);
        mbPlayerPosSetV(playerNo, &position);
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbPlayerRotateStart(playerNo, 0, 15);
    while (!mbPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    /* This transfer completes without re-enabling the player's collision snapping disabled
     * earlier. */
    GwPlayer[playerNo].masuId = marker->linkedSpace;
}

/* Finds the index of a stored Garage space entry by its space ID, or returns -1 when absent. */
s32 fn_1_A4B4(s32 needle) {
    PositionModel *base = lbl_1_bss_8;
    MapMarker *entry = base->markers;
    s32 index = 0;

    while (index < base->markerCount) {
        if (entry->spaceId == needle) {
            return index;
        }
        index++;
        entry++;
    }
    return -1;
}

/* Returns the space ID stored at the requested Garage route-entry index. */
s32 fn_1_A51C(s32 index) {
    PositionModel *work = lbl_1_bss_8;
    MapMarker *entry = &work->markers[index];

    return entry->spaceId;
}

/* Saves a player's material, palette and object state before making its model translucent white. */
void fn_1_A554(s32 playerNo) {
    HSF_DATA *modelData;
    HSF_MATERIAL *material;
    BoardModels *work;
    s16 objectIndex;
    HSF_OBJECT *object;
    MBMODELID modelObject;
    HSF_OBJECT *cursor;
    HU3D_MODEL *model;
    s32 *constData;
    int materialSize;
    int paletteSize;
    int objectSize;
    void *materialAllocation;
    HSF_MATERIAL *materialBackup;
    HSF_BUFFER *colorBuffer;
    HSF_PALETTE *palette;
    void *paletteAllocation;
    HSF_PALETTE *paletteBackup;
    void *objectAllocation;
    s32 *objectBackup;

    work = lbl_1_bss_8;
    modelObject = mbPlayerObjIDGet(playerNo);
    model = &Hu3DData[mbObjModelIDGet(modelObject)];
    modelData = model->hsf;
    material = modelData->material;
    /* The color-buffer pointer is fetched but is not used by this material effect. */
    colorBuffer = modelData->color;
    palette = modelData->palette;
    work->restorePlayer = playerNo;
    if (modelData->materialNum != 0) {
        materialSize = modelData->materialNum * sizeof(HSF_MATERIAL);
        materialAllocation = HuMemDirectMallocNum(HEAP_HEAP, materialSize, HU_MEMNUM_OVL);
        materialBackup = materialAllocation;
        work->materialBackup = materialBackup;
        memcpy(work->materialBackup, modelData->material,
               modelData->materialNum * sizeof(HSF_MATERIAL));
    }
    if (modelData->paletteNum != 0) {
        paletteSize = modelData->paletteNum * sizeof(HSF_PALETTE);
        paletteAllocation = HuMemDirectMallocNum(HEAP_HEAP, paletteSize, HU_MEMNUM_OVL);
        paletteBackup = paletteAllocation;
        work->paletteBackup = paletteBackup;
        memcpy(work->paletteBackup, modelData->palette,
               modelData->paletteNum * sizeof(HSF_PALETTE));
    }
    if (modelData->objectNum != 0) {
        objectSize = modelData->objectNum * sizeof(s32);
        objectAllocation = HuMemDirectMallocNum(HEAP_HEAP, objectSize, HU_MEMNUM_OVL);
        objectBackup = objectAllocation;
        work->objectData = objectBackup;
        cursor = modelData->object;
        for (objectIndex = 0; objectIndex < modelData->objectNum;
             cursor++, objectIndex++) {
            object = cursor;
            if (object->type == 2) {
                constData = object->constData;
                work->objectData[objectIndex] = *constData;
            }
        }
    }
    if (GwPlayer[playerNo].charNo != 5) {
        mbObjAlphaSet(modelObject, 127);
    } else {
        mbObjAlphaSet(modelObject, 192);
    }
    mbPlayerAttrSet(playerNo, W02_PLAYER_EVENT_ATTR_11_FLAG);
    for (objectIndex = 0; objectIndex < modelData->materialNum; objectIndex++, material++) {
        material->litColor[0] = 255;
        material->litColor[1] = 255;
        material->litColor[2] = 255;
        material->color[0] = 255;
        material->color[1] = 255;
        material->color[2] = 255;
        material->shadowColor[0] = 255;
        material->shadowColor[1] = 255;
        material->shadowColor[2] = 255;
        material->flags |= W02_MATERIAL_ATTR_4_FLAG;
        material->flags &= ~W02_MATERIAL_FLAGS_MASK;
    }
}

/* Restores a player's saved model materials, palette and object state after the Garage effect. */
void fn_1_A7BC(s32 playerNo) {
    HSF_DATA *modelData;
    BoardModels *work;
    s16 objectIndex;
    HSF_OBJECT *object;
    s16 modelObject;
    HSF_OBJECT *cursor;
    HU3D_MODEL *model;
    void *constData;
    HSF_MATERIAL *materialBackup;
    HSF_PALETTE *paletteBackup;
    s32 *objectData;

    work = lbl_1_bss_8;
    modelObject = mbPlayerObjIDGet(playerNo);
    model = &Hu3DData[mbObjModelIDGet(modelObject)];
    modelData = model->hsf;
    if (work->restorePlayer == playerNo) {
        if (modelData->materialNum != 0) {
            memcpy(modelData->material, work->materialBackup,
                   modelData->materialNum * sizeof(*modelData->material));
            materialBackup = work->materialBackup;
            HuMemDirectFree(materialBackup);
        }
        if (modelData->paletteNum != 0) {
            memcpy(modelData->palette, work->paletteBackup,
                   modelData->paletteNum * sizeof(*modelData->palette));
            paletteBackup = work->paletteBackup;
            HuMemDirectFree(paletteBackup);
        }
        if (modelData->objectNum != 0) {
            cursor = modelData->object;
            for (objectIndex = 0; objectIndex < modelData->objectNum;
                 cursor++, objectIndex++) {
                object = cursor;
                if (object->type == 2) {
                    constData = object->constData;
                    *(s32 *) constData = work->objectData[objectIndex];
                }
            }
            objectData = work->objectData;
            HuMemDirectFree(objectData);
        }
        mbObjAttrSet(modelObject, 2);
        mbObjAttrReset(modelObject, HU3D_ATTR_TPLVL_SET | HU3D_ATTR_MOT_EXEC);
        work->restorePlayer = -1;
    }
}

/* Creates the three stationary Garage effect models used around the moving attachment. */
void fn_1_A914(void) {
    int dataNum;
    s32 index;
    BoardModels *work;
    int model;
    int resource;

    work = lbl_1_bss_8;
    for (index = 0; index < 3; index++) {
        dataNum = DATANUM(DATA_w02, 29);
        dataNum = FILENUM(dataNum);
        if (!GwSystem.curTime) {
            dataNum |= DATA_w02;
        } else {
            dataNum |= DATA_w02n;
        }
        resource = dataNum;
        model = mbObjCreate(resource, NULL, TRUE);
        work->models[index] = model;
        mbObjPosSetV(model, &lbl_1_data_A9C[index]);
        mbObjRotSetV(model, &lbl_1_data_AC0[index]);
        mbObjMotionSpeedSet(model, 0.0f);
    }
    work->attachedModel = -1;
}

/* Loads the hooked event character, plays its sound cues, and freezes the parent at frame 86. */
void fn_1_AA04(s32 index) {
    int motionData[16];
    BoardModels *work;
    s32 modelId;
    s32 readStatus;

    work = lbl_1_bss_8;
    modelId = work->models[index];
    readStatus = mbBGRead(DATA_capsulechar3);
    if (readStatus != -1) {
        mbBGReadWait(readStatus);
    }
    motionData[0] = DATANUM(DATA_capsulechar3, 1);
    motionData[1] = DATANUM(DATA_capsulechar3, 2);
    motionData[2] = DATANUM(DATA_capsulechar3, 4);
    motionData[3] = DATANUM(DATA_capsulechar3, 5);
    motionData[4] = -1;
    work->attachedModel = mbObjCreate(DATANUM(DATA_capsulechar3, 0), motionData, FALSE);
    mbObjMotionSet(work->attachedModel, 1, HU3D_MOTATTR_LOOP);
    mbObjLayerSet(work->attachedModel, 3);
    HuDataDirClose(DATA_capsulechar3);
    mbObjHookSet(modelId, "oyhook", work->attachedModel);
    mbObjMotionTimeSet(modelId, 0.0f);
    mbObjMotionSpeedSet(modelId, 1.0f);
    mbAudFXPlay(W02_ATTACHMENT_START_SOUND_ID);
    do {
        HuPrcVSleep();
    } while (mbObjMotionTimeGet(modelId) < 86.0f);
    mbAudFXPlay(W02_ATTACHMENT_END_SOUND_ID);
    mbObjMotionSpeedSet(modelId, 0.0f);
}

/* Plays the shared attachment action, resumes the indexed parent model, then blends the
 * attachment back to its looping idle motion. */
void fn_1_AB78(s32 index) {
    BoardModels *work;
    s32 model;

    work = lbl_1_bss_8;
    model = work->models[index];
    mbObjMotionSet(work->attachedModel, 2, 0);
    mbObjMotionSpeedSet(model, 1.0f);
    do {
        HuPrcVSleep();
    } while (!mbObjMotionEndCheck(work->attachedModel));
    mbObjMotionShiftSet(work->attachedModel, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
}

/* Reverses the indexed Garage effect model, then removes its hooked attachment when it reaches the
 * start. */
void fn_1_AC34(s32 index) {
    BoardModels *work;
    s32 model;

    work = lbl_1_bss_8;
    model = work->models[index];
    mbObjMotionTimeSet(model, 85.0f);
    mbObjMotionSpeedSet(model, -2.0f);
    mbAudFXPlay(W02_ATTACHMENT_START_SOUND_ID);
    do {
        HuPrcVSleep();
    } while (mbObjMotionTimeGet(model) > 0.0f);
    mbAudFXPlay(W02_ATTACHMENT_END_SOUND_ID);
    mbObjHookReset(model);
    mbObjKill(work->attachedModel);
    work->attachedModel = -1;
}

/* Stops the indexed Garage effect model and removes its temporary hooked attachment. */
void fn_1_ACF4(s32 index) {
    BoardModels *work;
    s32 model;

    work = lbl_1_bss_8;
    model = work->models[index];
    mbObjMotionTimeSet(model, 0.0f);
    mbObjMotionSpeedSet(model, 0.0f);
    mbObjHookReset(model);
    mbObjKill(work->attachedModel);
    work->attachedModel = -1;
}

/* Creates the linked-space model pair, freezes its eyes and main attachment, and adds the
 * looping child on tosakahook. */
void fn_1_AD84(void) {
    s32 modelId;
    BoardModels *work;
    s32 spaceId;
    s32 firstSpace;
    s32 bodyModel;
    s32 secondSpace;
    HuVecF secondPosition;
    HuVecF firstPosition;
    HuVecF delta;

    work = lbl_1_bss_8;
    for (spaceId = 1; spaceId < mbMasuNumGet(); spaceId++) {
        if (mbMasuMAttrGet(spaceId) & 0x800) {
            break;
        }
    }
    firstSpace = mbMasuAttrFindLink(spaceId, (1 << 13));
    secondSpace = mbMasuAttrFindLink(firstSpace, (1 << 13));
    mbMasuPosGet(firstSpace, &firstPosition);
    mbMasuPosGet(secondSpace, &secondPosition);
    PSVECSubtract(&firstPosition, &secondPosition, &delta);
    work->linkedSpaceBaseModel = modelId = mbObjCreate(
        BoardModelDataNum(DATANUM(DATA_w02, 36)), NULL, FALSE);
    mbObjPosSetV(modelId, &secondPosition);
    mbObjRotSet(modelId, 0.0f,
                180.0 * (atan2(delta.x, delta.z) / 3.141592653589793),
                0.0f);
    mbObjMotionSpeedSet(modelId, 0.0f);
    modelId = bodyModel = mbObjCreate(BoardModelDataNum(DATANUM(DATA_w02, 37)), NULL, FALSE);
    work->linkedSpaceBodyModel = modelId;
    mbObjPosSetV(modelId, &secondPosition);
    mbObjRotSet(modelId, 0.0f,
                180.0 * (atan2(delta.x, delta.z) / 3.141592653589793),
                0.0f);
    mbObjMotionSpeedSet(modelId, 0.0f);
    work->eyeModel = modelId = mbObjCreate(
        BoardModelDataNum(DATANUM(DATA_w02, 38)), NULL, FALSE);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjHookSet(bodyModel, "OYeyehook", modelId);
    work->attachmentModel = modelId = mbObjCreate(
        BoardModelDataNum(DATANUM(DATA_w02, 39)), NULL, FALSE);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjHookSet(bodyModel, "OYhook", modelId);
    work->subAttachmentModel = modelId = mbObjCreate(
        BoardModelDataNum(DATANUM(DATA_w02, 41)), NULL, FALSE);
            mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjHookSet(work->attachmentModel, "tosakahook", modelId);
}

/* Offers the capsule exchange, reopening the choice after the player views the board map. */
void fn_1_B11C(s32 playerNo) {
    BoardModels *work;
    s32 choice;
    s32 capsuleCount;

    work = lbl_1_bss_8;
    capsuleCount = mbPlayerCapsuleNumGet(playerNo);
    fn_1_B570(playerNo, 0, 0);
    if (capsuleCount <= 0) {
        mbAudFXPlay(W02_STATION_EXIT_SOUND_ID);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 8), 14);
        mbWinTopWait();
        fn_1_B570(playerNo, 16, 0);
    } else {
        fn_1_B570(playerNo, 1, 0);
        do {
            mbAudFXPlay(W02_STATION_CHOICE_SOUND_ID);
            mbWinCreateChoice(1, MESSNUM(MESS_BOARD_W02, 4), 14, 0);
            if (GwPlayer[playerNo].comF) {
                mbComChoiceLeftSet();
            }
            mbWinTopWait();
            choice = mbWinTopChoiceGet();
            if (choice == 2) {
                mbev_Scroll(playerNo, 0);
            }
        } while (choice == 2);
        if (choice == 1 || choice == -1) {
            mbAudFXPlay(W02_CAPSULE_CONFIRM_SOUND_ID);
            mbObjMotionShiftSet(work->attachedModel, 3, 0.0f,
                                8.0f, HU3D_MOTATTR_LOOP);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 7), 14);
            mbWinTopWait();
            fn_1_B570(playerNo, 16, 0);
        } else {
            fn_1_B570(playerNo, 2, 0);
            fn_1_B570(playerNo, 3, capsuleCount);
            mbAudFXPlay(W02_STATION_SELECTION_SOUND_ID);
            mbObjMotionShiftSet(work->attachedModel, 3, 0.0f,
                                8.0f, HU3D_MOTATTR_LOOP);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 6), 14);
            mbWinTopWait();
            fn_1_B570(playerNo, 16, 0);
        }
    }
}

/* Offers to sell the carried capsules for their combined rank-dependent price. */
void fn_1_B334(s32 playerNo) {
    BoardModels *work;
    s32 index;
    s32 choice;
    s32 totalPrice;
    s32 capsuleCount;

    work = lbl_1_bss_8;
    capsuleCount = mbPlayerCapsuleNumGet(playerNo);
    index = 0;
    totalPrice = 0;
    for (; index < capsuleCount; index++) {
        totalPrice += mbCapSellCostGet(mbPlayerCapsuleGet(playerNo, index), playerNo);
    }
    fn_1_B570(playerNo, 0, 0);
    if (capsuleCount <= 0) {
        mbAudFXPlay(W02_STATION_EXIT_SOUND_ID);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 13), 14);
        mbWinTopWait();
        fn_1_B570(playerNo, 16, 0);
    } else {
        fn_1_B570(playerNo, 1, 0);
        do {
            mbAudFXPlay(W02_STATION_CHOICE_SOUND_ID);
        mbWinCreateChoice(1, MESSNUM(MESS_BOARD_W02, 9), 14, 0);
            if (GwPlayer[playerNo].comF) {
                mbComChoiceLeftSet();
            }
            mbWinTopWait();
            choice = mbWinTopChoiceGet();
            if (choice == 2) {
                mbev_Scroll(playerNo, 0);
            }
        } while (choice == 2);
        if (choice == 1 || choice == -1) {
            mbAudFXPlay(W02_CAPSULE_CONFIRM_SOUND_ID);
            mbObjMotionShiftSet(work->attachedModel, 3, 0.0f,
                            8.0f, HU3D_MOTATTR_LOOP);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 12), 14);
            mbWinTopWait();
            fn_1_B570(playerNo, 16, 0);
        } else {
            fn_1_B570(playerNo, 2, 0);
            fn_1_B570(playerNo, 4, totalPrice);
            mbAudFXPlay(W02_STATION_SELECTION_SOUND_ID);
            mbObjMotionShiftSet(work->attachedModel, 3, 0.0f,
                            8.0f, HU3D_MOTATTR_LOOP);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 11), 14);
            mbWinTopWait();
            fn_1_B570(playerNo, 16, 0);
        }
    }
}
HuVecF lbl_1_data_B08 = { 0.0f, 500.0f, 0.0f };

HuVecF lbl_1_data_B14 = { 400.0f, 200.0f, 300.0f };

char * lbl_1_data_B34[2] = { "yearrhook", "yearlhook" };

float lbl_1_data_B3C[2] = { 210.0f, 30.0f };

/* Runs one station phase selected by the exchange event, yielding during animations. */
void fn_1_B570(s32 playerNo, s32 action, s32 rewardCount) {
    Mtx matrix;
    int capsules[16];
    int motionData[16];
    HuVecF stationPosition;
    HuVecF exitPosition;
    HuVecF direction;
    HuVecF bend;
    HuVecF position;
    HuVecF playerStart;
    HuVecF effectPosition;
    HuVecF effectVelocity;
    int motionLoops[3];
    HuVecF firstPosition;
    HuVecF firstVelocity;
    HuVecF secondPosition;
    HuVecF secondVelocity;
    GXColor tint;
    GXColor firstTint;
    GXColor secondTint;
    GXColor *firstTintCapture;
    HuVecF *firstVelocityCapture;
    HuVecF *firstPositionCapture;
    GXColor *secondTintCapture;
    HuVecF *secondVelocityCapture;
    HuVecF *secondPositionCapture;
    CapsuleList *list;
    StationWork *work;
    int index;
    int step;
    int model;
    int playerSpace;
    int stationSpace;
    int exitSpace;
    int capsuleCount;
    int capsuleModel;
    int sound;
    void *allocation;
    float progress;

    work = lbl_1_bss_8;
    list = NULL;
    playerSpace = GwPlayer[playerNo].masuId;
    stationSpace = mbMasuAttrFindLink((s16) playerSpace, W02_STATION_LINK_ATTR_FLAG);
    exitSpace = mbMasuAttrFindLink(stationSpace, W02_STATION_LINK_ATTR_FLAG);
    mbMasuPosGet((s16) playerSpace, &playerStart);
    mbMasuPosGet((s16) stationSpace, &stationPosition);
    if (exitSpace != -1) {
        mbMasuPosGet((s16) exitSpace, &exitPosition);
    } else {
        exitPosition.x = 200.0f + stationPosition.x;
        exitPosition.y = stationPosition.y;
        exitPosition.z = stationPosition.z - 100.0f;
    }
    PSVECSubtract(&exitPosition, &stationPosition, &direction);
    switch (action) {
    case 0:
        mbWipeSpecialFadeInCreate(1, 1);
        for (index = 0; index < 4; index++) {
            if (index != playerNo) {
                mbPlayerDispSet(index, 0);
            }
        }
        mbPlayerColSnapPlayerSet(playerNo, 0);
        mbPlayerPosSetV(playerNo, &stationPosition);
        mbPlayerRotSet(playerNo, 0.0f,
                       (f32) (180.0 * (atan2((f64) direction.x, (f64) direction.z) / M_PI)), 0.0f);
        position.x = -30.0f;
        position.y = (f32) (180.0 + (180.0 * (atan2((f64) direction.x, (f64) direction.z) / M_PI)));
        if (position.y > 180.0f) {
            position.y -= 360.0f;
        }
        position.z = 0.0f;
        mbCameraMovePlayer(playerNo, &position, &lbl_1_data_B08, 1900.0f, -1.0f, -1);
        mbCameraMoveWait();
        motionData[0] = DATANUM(DATA_capsulechar3, 1);
        motionData[1] = DATANUM(DATA_capsulechar3, 3);
        motionData[2] = DATANUM(DATA_capsulechar3, 4);
        motionData[3] = DATANUM(DATA_capsulechar3, 5);
        motionData[4] = -1;
        work->stationModel = mbObjCreate(DATANUM(DATA_capsulechar3, 0), motionData, 0);
        mbObjMotionSet((s16) work->stationModel, 2, HU3D_MOTATTR_LOOP);
        mbObjLayerSet((s16) work->stationModel, 3U);
        HuDataDirClose(DATA_capsulechar3);
        mbObjHookSet((s16) work->liftBodyModel, "oyamahook", (s16) work->stationModel);
        mbObjMotionTimeSet((s16) work->liftBodyModel, 0.0f);
        mbObjDispSet((s16) work->liftBaseModel, 1);
        mbPlayerPosGet(playerNo, &position);
        PSVECAdd(&position, &lbl_1_data_B14, &position);
        position.y += 900.0f;
        mbObjPosSetV((s16) work->liftBaseModel, &position);
        mbObjRotSet((s16) work->liftBaseModel, 0.0f, -90.0f, 0.0f);
        mbWipeSpecialFadeOutCreate(1, 60);
        mbAudFXPlay(W02_LIFT_SOUND_ID);
        for (index = 1; (f32) index <= 60.0f; index++) {
            progress = (f32) index / 60.0f;
            mbPlayerPosGet(playerNo, &position);
            PSVECAdd(&position, &lbl_1_data_B14, &position);
            position.y = (f32) ((f64) position.y +
                                (900.0 - (900.0 * sin((M_PI * (f64) (90.0f * progress)) / 180.0))));
            mbObjPosSetV((s16) work->liftBaseModel, &position);
            HuPrcVSleep();
        }
        mbObjMotionSpeedSet((s16) work->liftBodyModel, 0.0f);
        mbObjMotionShiftSet((s16) work->stationModel, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        return;
    case 1:
        model = work->machineModels[0];
        mbObjMotionTimeSet((s16) model, 0.0f);
        mbObjMotionSpeedSet((s16) model, 2.0f);
        model = work->machineModels[2];
        mbObjMotionTimeSet((s16) model, 0.0f);
        mbObjMotionSpeedSet((s16) model, 1.0f);
        return;
    case 2:
        mbObjMotionShiftSet((s16) work->stationModel, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        model = work->capsuleGateModel;
        mbObjMotionTimeSet((s16) model, 0.0f);
        mbObjMotionSpeedSet((s16) model, 1.0f);
        while (mbObjMotionTimeGet(model) < 0.5f * mbObjMotionMaxTimeGet(model)) {
            HuPrcVSleep();
        }
        mbObjMotionSpeedSet((s16) model, 0.0f);
        stationPosition.y += 100.0f;
        exitPosition.y += 400.0f;
        capsuleCount = mbPlayerCapsuleNumGet(playerNo);
        for (index = 0; index < capsuleCount; index++) {
            capsuleModel = mbCapObjColorCreate(mbPlayerCapsuleGet(playerNo, 0), 1);
            mbCapObjColorScaleSet(capsuleModel, 0.5f, 0.5f, 0.5f);
            mbPlayerCapsuleRemove(playerNo, 0);
            for (step = 0; (f32) step < 30.0f; step++) {
                progress = (f32) step / 30.0f;
                position.x = stationPosition.x + (progress * (exitPosition.x - stationPosition.x));
                position.z = stationPosition.z + (progress * (exitPosition.z - stationPosition.z));
                position.y =
                    (f32) ((f64) (stationPosition.y +
                                  (progress * (exitPosition.y - stationPosition.y))) +
                           4.0 * (100.0 * sin((M_PI * (f64) (180.0f * progress)) / 180.0)));
                mbCapObjColorPosSetV(capsuleModel, &position);
                HuPrcVSleep();
            }
            mbCapObjColorKill(capsuleModel);
            mbAudFXPlay(W02_CAPSULE_DROP_SOUND_ID);
            omVibrate((s16) playerNo, 20, 4, 4);
        }
        model = work->capsuleGateModel;
        mbObjMotionSpeedSet((s16) model, 1.0f);
        while (mbObjMotionTimeGet(model) < mbObjMotionMaxTimeGet(model)) {
            HuPrcVSleep();
        }
        mbObjMotionSpeedSet((s16) model, 0.0f);
        mbAudFXPlay(W02_MECHANISM_SOUND_ID);
        for (index = 0; (f32) index < 60.0f; index++) {
            for (step = 0; step < 2; step++) {
                Hu3DModelObjMtxGet(mbObjModelIDGet((s16) work->machineModels[1]),
                                   lbl_1_data_B34[step], matrix);
                effectPosition.x = matrix[0][3];
                effectPosition.y = matrix[1][3];
                effectPosition.z = matrix[2][3];
                effectVelocity.x =
                    (f32) ((100.0 * (0.1f * sin((M_PI * (f64) lbl_1_data_B3C[step]) / 180.0))) +
                           (0.01f + (0.05f * (-0.5f + (3.7252903e-9f *
                                                       (f32) mbRandMod(W02_RANDOM_MODULUS_U))))));
                effectVelocity.z =
                    (f32) ((100.0 * (0.1f * cos((M_PI * (f64) lbl_1_data_B3C[step]) / 180.0))) +
                           (0.01f + (0.05f * (-0.5f + (3.7252903e-9f *
                                                       (f32) mbRandMod(W02_RANDOM_MODULUS_U))))));
                effectVelocity.y =
                    100.0f *
                    (0.01f + (0.01f * (3.7252903e-9f * (f32) mbRandMod(W02_RANDOM_MODULUS_U))));
                progress = 3.7252903e-9f * (f32) mbRandMod(W02_RANDOM_MODULUS_U);
                tint.r = (u8) (192.0f + (32.0f * progress));
                tint.g = (u8) (192.0f + (32.0f * progress));
                tint.b = (u8) (192.0f + (32.0f * progress));
                tint.a = (u8) (127.0f +
                               (63.0f * (3.7252903e-9f * (f32) mbRandMod(W02_RANDOM_MODULUS_U))));
                firstTint.r = tint.r;
                firstTint.g = tint.g;
                firstTint.b = tint.b;
                firstTint.a = tint.a;
                firstTintCapture = &firstTint;
                firstVelocity = effectVelocity;
                firstVelocityCapture = &firstVelocity;
                firstPosition = effectPosition;
                firstPositionCapture = &firstPosition;
                fn_1_176E4(
                    firstPositionCapture, firstVelocityCapture,
                    100.0f *
                        (1.0f + (0.5f * (3.7252903e-9f * (f32) mbRandMod(W02_RANDOM_MODULUS_U)))),
                    -0.5f + (3.7252903e-9f * (f32) mbRandMod(W02_RANDOM_MODULUS_U)),
                    0.33f + (0.66f * (3.7252903e-9f * (f32) mbRandMod(W02_RANDOM_MODULUS_U))),
                    firstTintCapture);
            }
            HuPrcVSleep();
        }
        mbObjMotionShiftSet((s16) work->stationModel, 2,
                            mbObjMotionTimeGet((s16) work->liftBodyModel), 8.0f, HU3D_MOTATTR_LOOP);
        while (mbObjMotionShiftIDGet((s16) work->stationModel) != -1) {
            HuPrcVSleep();
        }
        mbObjMotionSpeedSet((s16) work->liftBodyModel, 1.0f);
        mbObjMotionTimeSet((s16) work->liftBodyModel, mbObjMotionTimeGet((s16) work->stationModel));
        sound = mbAudFXPlay(W02_MACHINE_LOOP_SOUND_ID);
        model = work->machineModels[0];
        mbObjMotionTimeSet((s16) model, 0.0f);
        mbObjMotionSpeedSet((s16) model, 1.0f);
        model = work->machineModels[1];
        mbObjMotionTimeSet((s16) model, 0.0f);
        mbObjMotionSpeedSet((s16) model, 2.0f);
        model = work->machineModels[2];
        mbObjMotionTimeSet((s16) model, 0.0f);
        mbObjMotionSpeedSet((s16) model, 2.0f);
        motionLoops[0] = 2;
        motionLoops[1] = 1;
        motionLoops[2] = 8;
        omVibrate((s16) playerNo, 300, 4, 4);
        do {
            for (index = 0; index < 3; index++) {
                model = work->machineModels[index];
                if ((s32) motionLoops[index] > 0) {
                    if (mbObjMotionTimeGet((s16) model) >= mbObjMotionMaxTimeGet((s16) model)) {
                        mbObjMotionTimeSet((s16) model, 0.0f);
                        motionLoops[index] -= 1;
                    }
                }
            }
            HuPrcVSleep();
        } while (motionLoops[0] > 0 || motionLoops[1] > 0 || motionLoops[2] > 0);
        do {
            for (index = 0; index < 3; index++) {
                model = work->machineModels[index];
                if (mbObjMotionTimeGet(model) < mbObjMotionMaxTimeGet(model)) {
                    break;
                }
            }
            HuPrcVSleep();
        } while (index < 3);
        if (sound != -1) {
            mbAudFXStop(sound);
        }
        mbObjMotionSpeedSet((s16) work->liftBodyModel, 0.0f);
        mbObjMotionShiftSet((s16) work->stationModel, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbAudFXPlay(W02_MECHANISM_SOUND_ID);
        for (index = 0; (f32) index < 60.0f; index++) {
            for (step = 0; step < 2; step++) {
                Hu3DModelObjMtxGet(mbObjModelIDGet((s16) work->machineModels[1]),
                                   lbl_1_data_B34[step], matrix);
                effectPosition.x = matrix[0][3];
                effectPosition.y = matrix[1][3];
                effectPosition.z = matrix[2][3];
                effectVelocity.x =
                    (f32) ((100.0 * (0.1f * sin((M_PI * (f64) lbl_1_data_B3C[step]) / 180.0))) +
                           (0.01f + (0.05f * (-0.5f + (3.7252903e-9f *
                                                       (f32) mbRandMod(W02_RANDOM_MODULUS_U))))));
                effectVelocity.z =
                    (f32) ((100.0 * (0.1f * cos((M_PI * (f64) lbl_1_data_B3C[step]) / 180.0))) +
                           (0.01f + (0.05f * (-0.5f + (3.7252903e-9f *
                                                       (f32) mbRandMod(W02_RANDOM_MODULUS_U))))));
                effectVelocity.y =
                    100.0f *
                    (0.01f + (0.01f * (3.7252903e-9f * (f32) mbRandMod(W02_RANDOM_MODULUS_U))));
                progress = 3.7252903e-9f * (f32) mbRandMod(W02_RANDOM_MODULUS_U);
                tint.r = (u8) (192.0f + (32.0f * progress));
                tint.g = (u8) (192.0f + (32.0f * progress));
                tint.b = (u8) (192.0f + (32.0f * progress));
                tint.a = (u8) (127.0f +
                               (63.0f * (3.7252903e-9f * (f32) mbRandMod(W02_RANDOM_MODULUS_U))));
                secondTint.r = tint.r;
                secondTint.g = tint.g;
                secondTint.b = tint.b;
                secondTint.a = tint.a;
                secondTintCapture = &secondTint;
                secondVelocity = effectVelocity;
                secondVelocityCapture = &secondVelocity;
                secondPosition = effectPosition;
                secondPositionCapture = &secondPosition;
                fn_1_176E4(
                    secondPositionCapture, secondVelocityCapture,
                    100.0f *
                        (1.0f + (0.5f * (3.7252903e-9f * (f32) mbRandMod(W02_RANDOM_MODULUS_U)))),
                    -0.5f + (3.7252903e-9f * (f32) mbRandMod(W02_RANDOM_MODULUS_U)),
                    0.33f + (0.66f * (3.7252903e-9f * (f32) mbRandMod(W02_RANDOM_MODULUS_U))),
                    secondTintCapture);
            }
            HuPrcVSleep();
        }
        return;
    case 3:
        exitPosition.y += 50.0f;
        allocation = HuMemDirectMallocNum(HEAP_HEAP, 528, HU_MEMNUM_OVL);
        list = allocation;
        capsuleCount = mbCapListCopy(list);
        for (index = 0; index < rewardCount; index++) {
            capsules[index] = list[(frand() & 0x7FFF) % capsuleCount].id;
            capsuleModel = mbCapObjColorCreate(capsules[index], 1);
            mbCapObjColorScaleSet(capsuleModel, 0.5f, 0.5f, 0.5f);
            bend.x = stationPosition.x + (0.5f * direction.x) +
                     (0.25f *
                      (100.0f * (-0.5f + (3.7252903e-9f * (f32) mbRandMod(W02_RANDOM_MODULUS_U)))));
            bend.y = stationPosition.y;
            bend.z = stationPosition.z + (0.5f * direction.z) +
                     (0.25f *
                      (100.0f * (-0.5f + (3.7252903e-9f * (f32) mbRandMod(W02_RANDOM_MODULUS_U)))));
            mbAudFXPlay(W02_CAPSULE_LAUNCH_SOUND_ID);
            for (step = 0; (f32) step < 30.0f; step++) {
                progress = (f32) step / 30.0f;
                position.x = exitPosition.x + (progress * (bend.x - exitPosition.x));
                position.y =
                    (f32) ((f64) (exitPosition.y + (progress * (bend.y - exitPosition.y))) +
                           1.5 * (100.0 * sin((M_PI * (f64) (180.0f * progress)) / 180.0)));
                position.z = exitPosition.z + (progress * (bend.z - exitPosition.z));
                mbCapObjColorPosSetV(capsuleModel, &position);
                HuPrcVSleep();
            }
            for (step = 0; (f32) step < 30.0f; step++) {
                progress = (f32) step / 30.0f;
                position.x = bend.x + (progress * (stationPosition.x - bend.x));
                position.y = (f32) ((f64) (bend.y + (progress * (stationPosition.y - bend.y))) +
                                    100.0 * sin((M_PI * (f64) (180.0f * progress)) / 180.0));
                position.z = bend.z + (progress * (stationPosition.z - bend.z));
                mbCapObjColorPosSetV(capsuleModel, &position);
                HuPrcVSleep();
            }
            mbCapObjColorKill(capsuleModel);
        }
        HuMemDirectFree(list);
        mbPlayerRotateStart(
            playerNo,
            (s16) (180.0 + (180.0 * (atan2((f64) direction.x, (f64) direction.z) / M_PI))), 15);
        while (mbPlayerRotateCheck(playerNo) == 0) {
            HuPrcVSleep();
        }
        for (index = 0; index < rewardCount; index++) {
            mbCapCapsuleGet(playerNo, capsules[index]);
            mbPlayerCapsuleAdd(playerNo, capsules[index]);
            mbPlayerWinLoseVoicePlay(playerNo, W02_CAPSULE_WIN_MOTION_NO,
                                     W02_CAPSULE_WIN_EFFECT_ID);
            mbPlayerMotionShiftSet(playerNo, 12, 0.0f, 8.0f, 0U);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 5), -1);
            mbWinTopInsertMesSet(mbCapUseMesGet(capsules[index]), 0);
            mbWinTopWait();
            mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        }
        return;
    case 4:
        fn_1_D410();
        exitPosition.y += 30.000002f;
        for (index = 0; index < rewardCount; index++) {
            mbAudFXPlay(W02_CAPSULE_REWARD_SOUND_ID);
            fn_1_D4D0(&exitPosition, &stationPosition);
            HuPrcVSleep();
        }
        while (fn_1_DC70() > 0) {
            HuPrcVSleep();
        }
        fn_1_DCF0();
        mbPlayerRotateStart(
            playerNo,
            (s16) (180.0 + (180.0 * (atan2((f64) direction.x, (f64) direction.z) / M_PI))), 15);
        while (mbPlayerRotateCheck(playerNo) == 0) {
            HuPrcVSleep();
        }
        mbCoinAddDispExec(playerNo, rewardCount, 0, 1);
        mbev_CapCoinDisp(playerNo, rewardCount, 1, 1);
        return;
    case 16:
        mbObjMotionShiftSet((s16) work->stationModel, 4, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbAudFXPlay(W02_LIFT_MOVE_SOUND_ID);
        for (index = 1; (f32) index <= 60.0f; index++) {
            progress = (f32) index / 60.0f;
            mbPlayerPosGet(playerNo, &position);
            PSVECAdd(&position, &lbl_1_data_B14, &position);
            position.y += 900.0f * (progress * progress);
            mbObjPosSetV((s16) work->liftBaseModel, &position);
            HuPrcVSleep();
        }
        mbWipeSpecialFadeInCreate(1, 1);
        for (index = 0; index < 4; index++) {
            mbPlayerDispSet(index, 1);
        }
        mbPlayerColSnapPlayerSet(playerNo, 1);
        mbPlayerPosSetV(playerNo, &playerStart);
        mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
        mbCameraPlayerViewSetFast(playerNo, 0);
        mbCameraMoveWait();
        mbObjHookReset((s16) work->liftBodyModel);
        mbObjKill((s16) work->stationModel);
        work->stationModel = -1;
        mbObjDispSet((s16) work->liftBaseModel, 0);
        mbWipeSpecialFadeOutCreate(1, 60);
        /* fallthrough */
    default:
        return;
    }
}

/* Allocates the Garage flying-model table and marks all 64 model slots unused before effects
 * run. */
void fn_1_D410(void) {
    BoardModels *work;
    EventModelEntry *entry;
    s32 index;
    EventModelEntry *allocation;

    work = lbl_1_bss_8;
    work->entryObject = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_D8EC);
    allocation = HuMemDirectMallocNum(HEAP_HEAP, sizeof(EventModelEntry) * 64, HU_MEMNUM_OVL);
    work->entryData = allocation;
    entry = allocation;
    memset(work->entryData, 0, sizeof(EventModelEntry) * 64);
    for (index = 0; index < 64; index++, entry++) {
        entry->modelId = -1;
    }
}

/* Starts a two-arc model flight between two positions, returning its pool index. */
s32 fn_1_D4D0(HuVecF *start, HuVecF *end) {
    EventModelEntry *entry;
    s32 index;
    BoardModels *work;
    MBMODELID positionModel;
    MBMODELID rotationModel;
    MBMODELID scaleModel;
    MBMODELID modelId;

    work = lbl_1_bss_8;
    entry = work->entryData;
    for (index = 0; index < 64; index++, entry++) {
        if (entry->modelId == -1) {
            break;
        }
    }
    if (index >= 64) {
        return -1;
    }
    modelId = mbObjCreate(mbBoardDataNumGet(DATANUM(DATA_board, 4)), NULL, TRUE);
    entry->modelId = modelId;
    entry->entryIndex = index;
    entry->flightStage = 0;
    entry->age = 0;
    entry->travelFrames = 60.0f * (0.4f +
        0.3f * (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
    entry->jumpHeight = 100.0f * (1.2f +
        0.6f * (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
    entry->rotationSpeed = 20.0f * (-0.5f +
        W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
    entry->start = *start;
    entry->end = *end;
    entry->rotation.x = 0.0f;
    entry->rotation.y = 360.0f *
        (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
    entry->rotation.z = 0.0f;
    entry->bend.x = entry->start.x + 0.5f * (entry->end.x - entry->start.x) +
        0.5f * (100.0f * (-0.5f +
            W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
    entry->bend.z = entry->start.z + 0.5f * (entry->end.z - entry->start.z) +
        0.5f * (100.0f * (-0.5f +
            W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
    entry->bend.y = entry->start.y;
    positionModel = entry->modelId;
    mbObjPosSetV(positionModel, &entry->start);
    rotationModel = entry->modelId;
    mbObjRotSetV(rotationModel, &entry->rotation);
    scaleModel = entry->modelId;
    mbObjScaleSet(scaleModel, 0.5f, 0.5f, 0.5f);
    return index;
}

/* Moves active models through two flight arcs and removes them after landing. */
void fn_1_D8EC(OMOBJ *object) {
    HuVecF displacement;
    HuVecF position;
    BoardModels *work;
    EventModelEntry *entry;
    s32 index;
    float progress;

    work = lbl_1_bss_8;
    entry = work->entryData;
    if (mbExitCheck() || work->entryObject == NULL) {
        omDelObjEx(mbObjMan, object);
        return;
    }
    for (index = 0; index < 64; index++, entry++) {
        if (entry->modelId != -1) {
            progress = (float) ++entry->age / (float) entry->travelFrames;
            if (entry->flightStage == 0) {
                float arc;

                PSVECSubtract(&entry->bend, &entry->start, &displacement);
                position.x = entry->start.x + progress * displacement.x;
                arc = mbSinDeg(180.0f * progress);
                position.y = entry->start.y + progress * displacement.y +
                    entry->jumpHeight * arc;
                position.z = entry->start.z + progress * displacement.z;
            } else {
                float arc;

                PSVECSubtract(&entry->end, &entry->bend, &displacement);
                position.x = entry->bend.x + progress * displacement.x;
                arc = mbSinDeg(180.0f * progress);
                position.y = entry->bend.y + progress * displacement.y +
                    entry->jumpHeight * arc;
                position.z = entry->bend.z + progress * displacement.z;
            }
            {
                MBMODELID modelId = entry->modelId;
                mbObjPosSetV(modelId, &position);
            }
            {
                MBMODELID modelId = entry->modelId;
                mbObjRotSetV(modelId, &entry->rotation);
            }
            entry->rotation.y += entry->rotationSpeed;
            if (entry->age >= entry->travelFrames) {
                if (entry->flightStage >= 1) {
                    MBMODELID modelId = entry->modelId;
                    mbObjKill(modelId);
                    entry->modelId = -1;
                } else {
                    entry->age = 0;
                    entry->travelFrames = 60.0f * (0.4f +
                        0.3f * (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                    entry->jumpHeight = 100.0f * (1.2f +
                        0.6f * (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                    entry->flightStage++;
                    mbAudFXPlay(W02_COIN_REWARD_SOUND_ID);
                }
            }
        }
    }
}

/* Counts active two-arc flights; the capsule sale waits for them before crediting coins. */
s32 fn_1_DC70(void) {
    BoardModels *work;
    EventModelEntry *effectData;
    s32 effectIndex;
    s32 activeCount;
    work = lbl_1_bss_8;
    effectData = work->entryData;
    if (effectData == NULL) {
        return 0;
    }
    effectIndex = 0;
    activeCount = 0;
    while (effectIndex < 64) {
        if (effectData->modelId != -1) {
            activeCount += 1;
        }
        effectIndex += 1;
        effectData++;
    }
    return activeCount;
}

/* Frees the Garage flying-effect table and clears its data and update-object pointers after the
 * sequence. */
void fn_1_DCF0(void) {
    BoardModels *manager;
    void *allocation;
    void *allocationToFree;

    manager = lbl_1_bss_8;
    /* This copy of the table pointer is unused; the free below fetches the pointer again. */
    allocation = manager->entryData;
    if (manager->entryData != NULL) {
        allocationToFree = manager->entryData;
        HuMemDirectFree(allocationToFree);
    }
    manager->entryData = NULL;
    manager->entryObject = NULL;
}
HuVecF lbl_1_data_B94 = { -2690.0f, 1000.0f, -4900.0f };

/* Creates the event's positioned base, hooked fan, and initially hidden display models. */
void fn_1_DD5C(void) {
    s32 modelId;
    BoardModels *work;
    s32 primaryModel;

    work = lbl_1_bss_8;
    primaryModel = mbObjCreate(BoardModelDataNum(DATANUM(DATA_w02, 30)), NULL, FALSE);
    work->positionModel = primaryModel;
    modelId = primaryModel;
            mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjPosSetV(modelId, &lbl_1_data_B94);
    work->hookedModel = modelId = mbObjCreate(
        BoardModelDataNum(DATANUM(DATA_w02, 31)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjMotionSpeedSet(modelId, 0.5f);
    mbObjHookSet(primaryModel, "fanhook", modelId);
    work->displayModels[0] = modelId = mbObjCreate(
        BoardModelDataNum(DATANUM(DATA_w02, 32)), NULL, TRUE);
    mbObjLayerSet(modelId, 3);
    mbObjDispSet(modelId, FALSE);
    work->displayModels[1] = modelId = mbObjCreate(
        BoardModelDataNum(DATANUM(DATA_w02, 33)), NULL, TRUE);
    mbObjLayerSet(modelId, 3);
    mbObjDispSet(modelId, FALSE);
    work->displayModels[2] = modelId = mbObjCreate(
        BoardModelDataNum(DATANUM(DATA_w02, 34)), NULL, TRUE);
    mbObjLayerSet(modelId, 3);
    mbObjDispSet(modelId, FALSE);
    work->displayModels[3] = modelId = mbObjCreate(
        BoardModelDataNum(DATANUM(DATA_w02, 35)), NULL, TRUE);
    mbObjLayerSet(modelId, 3);
    mbObjDispSet(modelId, FALSE);
    {
        s16 commonModel[1];

        commonModel[0] = mbObjCreate(mbBoardDataNumGet(DATANUM(DATA_board, 4)), NULL, TRUE);
        work->hiddenModel = modelId = commonModel[0];
        mbObjDispSet(modelId, FALSE);
    }
}
HuVecF lbl_1_data_BA8 = { 0.0f, 200.0f, -400.0f };

/* Handles a machine space: offers the ride, moves the player through its hooks, and lets opponents
 * resist the coin drain. */
void fn_1_E074(s32 playerNo) {
    int motionData[16];
    HuVecF firstHook;
    HuVecF secondHook;
    HuVecF start;
    HuVecF position;
    HuVecF destination;
    HuVecF direction;
    HuVecF approach;
    HuVecF cameraRotation;
    int lostCoins[4];
    int modelId;
    int helpWindow;
    int readStatus;
    s16 activeModel;
    s16 activeHookedModel;
    s16 endingModel;
    PositionModel *work;
    int motionId;
    int cameraModel;
    int choice;
    int coinDisplay;
    int machineSound;
    int index;
    int totalCoins;
    int remainingAttempts;
    int elapsed;
    int emptySoundPlayed;
    int resisted;
    int timer;
    int finished;
    float progress;

    work = lbl_1_bss_8;
    finished = 0;
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_10954();
    HuPrcSleep(2);
    fn_1_11904();
    HuPrcSleep(2);
    fn_1_125C0();
    HuPrcSleep(2);
    mbPlayerColSnapPlayerSet(playerNo, 0);
    /* This model lookup is retained even though its stored result is never read. */
    modelId = (int) mbObjModelIDGet((s16) work->modelId);
    motionId = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mario, 217));
    HuPrcSleep(2);
    cameraModel = mbObjCreate(DATANUM(DATA_capsule, 33), NULL, 0);
    mbObjDispSet(cameraModel, 0);
    mbPlayerPosGet(playerNo, &start);
    start.x -= 400.0f;
    start.y += 500.0f;
    mbObjPosSetV((s16) cameraModel, &start);
    HuPrcSleep(2);
    mbPlayerRotateStart(playerNo, 180, 15);
    while (mbPlayerRotateCheck(playerNo) == 0) {
        HuPrcVSleep();
    }
    if ((readStatus = mbBGRead(DATA_capsulechar3)) != -1) {
        mbBGReadWait(readStatus);
    }
    HuPrcSleep(2);
    motionData[0] = DATANUM(DATA_capsulechar3, 1);
    motionData[1] = DATANUM(DATA_capsulechar3, 3);
    motionData[2] = DATANUM(DATA_capsulechar3, 4);
    motionData[3] = DATANUM(DATA_capsulechar3, 5);
    motionData[4] = -1;
    work->attachedModel = (int) mbObjCreate(DATANUM(DATA_capsulechar3, 0), NULL, 0);
    mbObjLayerSet((s16) work->attachedModel, 3U);
    mbObjDispSet((s16) work->attachedModel, 0);
    HuPrcSleep(2);
    for (index = 0; (int) motionData[index] >= 0; index++) {
        mbObjMotionCreate((s16) work->attachedModel, motionData[index]);
        HuPrcSleep(2);
    }
    mbObjDispSet((s16) work->attachedModel, 1);
    mbObjMotionSet((s16) work->attachedModel, 2, HU3D_MOTATTR_LOOP);
    HuDataDirClose(DATA_capsulechar3);
    mbObjHookSet((s16) work->liftObjectId, "oyama2hook", (s16) work->attachedModel);
    mbObjMotionTimeSet((s16) work->liftObjectId, 0.0f);
    mbObjDispSet((s16) work->leverObjectId, 1);
    mbPlayerPosGet(playerNo, &position);
    PSVECAdd(&position, &lbl_1_data_BA8, &position);
    position.y += 600.0f;
    mbObjPosSetV((s16) work->leverObjectId, &position);
    mbObjRotSet((s16) work->leverObjectId, 0.0f, 0.0f, 0.0f);
    mbAudFXPlay(W02_LIFT_SOUND_ID);
    for (index = 1; (f32) index <= 60.0f; index++) {
        progress = index / 60.0f;
        mbPlayerPosGet(playerNo, &position);
        PSVECAdd(&position, &lbl_1_data_BA8, &position);
        position.y += (600.0 - (600.0 * HuSin(90.0f * progress)));
        mbObjPosSetV((s16) work->leverObjectId, &position);
        HuPrcVSleep();
    }
    mbObjMotionSpeedSet((s16) work->liftObjectId, 0.0f);
    mbObjMotionShiftSet((s16) work->attachedModel, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    do {
        mbAudFXPlay(W02_STATION_CHOICE_SOUND_ID);
        mbWinCreateChoice(1, MESSNUM(MESS_BOARD_W02, 15), 14, 0);
        if (GwPlayer[playerNo].comF != 0) {
            mbComChoiceRightSet();
        }
        mbWinTopWait();
        choice = mbWinTopChoiceGet();
        switch (choice) {
        case 0:
        case 1:
            break;
        case 2:
            mbev_Scroll(playerNo, 0);
            break;
        }
    } while (choice == 2);
    if (choice) {
        mbAudFXPlay(W02_MACHINE_SPIN_SOUND_ID);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 17), 14);
        mbWinTopWait();
    } else {
        cameraRotation.x = -33.0f;
        cameraRotation.y = 0.0f;
        cameraRotation.z = 0.0f;
        position.x = 0.0f;
        position.y = 0.0f;
        position.z = 0.0f;
        mbCameraMoveObj(cameraModel, &cameraRotation, &position, 2700.0f, -1.0f, 21);
        mbObjMotionShiftSet((s16) work->attachedModel, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbPlayerPosGet(playerNo, &start);
        mbObjMotionSpeedSet((s16) work->characterObjectId, 1.0f);
        while (mbObjMotionEndCheck((s16) work->characterObjectId) == 0) {
            HuPrcVSleep();
        }
        fn_1_FF6C(&destination);
        PSVECSubtract(&destination, &start, &direction);
        mbPlayerMotionSet(playerNo, 4, 0U);
        mbPlayerRotSet(playerNo, 0.0f, HuAtan(direction.x, direction.z), 0.0f);
        for (index = 1; (f32) index <= 30.0f; index++) {
            progress = index / 30.0f;
            fn_1_FF6C(&destination);
            position.x = start.x + (progress * (destination.x - start.x));
            position.y = (f32) ((f64) (start.y + (progress * (destination.y - start.y))) +
                                (2.0 * (100.0 * HuSin(180.0f * progress))));
            position.z = start.z + (progress * (destination.z - start.z));
            mbPlayerPosSetV(playerNo, &position);
            HuPrcVSleep();
        }
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbObjMotionShiftSet((s16) work->attachedModel, 2,
                            mbObjMotionTimeGet((s16) work->motionReferenceModel), 8.0f,
                            HU3D_MOTATTR_LOOP);
        while (mbObjMotionShiftIDGet((s16) work->attachedModel) != -1) {
            HuPrcVSleep();
        }
        mbObjMotionSpeedSet((s16) work->liftObjectId, 1.0f);
        mbObjMotionTimeSet((s16) work->liftObjectId, mbObjMotionTimeGet((s16) work->attachedModel));
        mbAudFXPlay(W02_LIFT_MOVE_SOUND_ID);
        for (index = 1; (f32) index <= 60.0f; index++) {
            progress = index / 60.0f;
            mbMasuPosGet(GwPlayer[playerNo].masuId, &position);
            PSVECAdd(&position, &lbl_1_data_BA8, &position);
            position.y += (600.0 * HuSin(90.0f * (progress * progress)));
            mbObjPosSetV((s16) work->leverObjectId, &position);
            fn_1_FF6C(&position);
            mbPlayerPosSetV(playerNo, &position);
            HuPrcVSleep();
        }
        mbPlayerPosGet(playerNo, &start);
        fn_1_FEB4(&firstHook, &secondHook);
        PSVECSubtract(&secondHook, &firstHook, &direction);
        PSVECSubtract(&firstHook, &start, &approach);
        mbPlayerRotSet(playerNo, 0.0f, HuAtan(approach.x, approach.z), 0.0f);
        mbPlayerMotionSet(playerNo, 4, 0U);
        for (index = 1; (f32) index <= 30.0f; index++) {
            progress = index / 30.0f;
            position.x = start.x + (progress * (firstHook.x - start.x));
            position.y = (f32) ((f64) (start.y + (progress * (firstHook.y - start.y))) +
                                (2.0 * (100.0 * HuSin(180.0f * progress))));
            position.z = start.z + (progress * (firstHook.z - start.z));
            mbPlayerPosSetV(playerNo, &position);
            HuPrcVSleep();
        }
        mbPlayerRotateStart(playerNo, (s16) (HuAtan(direction.x, direction.z)), 15);
        while (mbPlayerRotateCheck(playerNo) == 0) {
            HuPrcVSleep();
        }
        mbPlayerMotionShiftSet(playerNo, motionId, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbAudFXPlay(W02_LIFT_MOVE_SOUND_ID);
        for (index = 1; (f32) index <= 30.0f; index++) {
            progress = index / 30.0f;
            mbMasuPosGet(GwPlayer[playerNo].masuId, &position);
            PSVECAdd(&position, &lbl_1_data_BA8, &position);
            position.y += (600.0 + (600.0 * HuSin(90.0f * (progress * progress))));
            mbObjPosSetV((s16) work->leverObjectId, &position);
            HuPrcVSleep();
        }
        mbObjDispSet((s16) work->leverObjectId, 0);
        work->particlesDisabled = 1;
        mbAudFXPlay(W02_MACHINE_START_SOUND_ID);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 16), 14);
        mbWinTopWait();
        machineSound = mbAudFXPlay(W02_MACHINE_STOP_SOUND_ID);
        mbObjMotionSpeedSet((s16) work->modelId, 1.0f);
        activeModel = (s16) work->modelId;
        mbObjAttrSet(activeModel, HU3D_MOTATTR_LOOP);
        mbObjMotionSpeedSet((s16) work->hookedModel, 2.0f);
        activeHookedModel = (s16) work->hookedModel;
        mbObjAttrSet(activeHookedModel, HU3D_MOTATTR_LOOP);
        work->eventPlayerNo = playerNo;
        work->emissionFrame = 0;
        work->eventTimer = 1;
        work->opponentPlayerNo = -1;
        work->eventStage = 0;
        omAddObjEx(mbObjMan, -32768, 0U, 0U, -1, fn_1_10034);
        for (index = 0; index < 4; index++) {
            if (index != playerNo) {
                fn_1_12AC8(index, &firstHook);
                HuPrcVSleep();
            }
        }
        omVibrate((s16) playerNo, 120, 4, 4);
        HuPrcSleep(120);
        omVibrate((s16) playerNo, 180, 7, 3);
        HuPrcSleep(60);
        mbWipeDissolveFadeOutTime(1);
        for (index = 0; index < 4; index++) {
            lostCoins[index] = 0;
        }
        totalCoins = 0;
    helpWindow = mbWinCreateHelp(MESSNUM(MESS_BOARD_W02, 14));
        for (index = 0; index < 4; index++) {
            if (index != playerNo) {
                mbCameraPlayerViewSetFast(index, 1);
                mbCameraMoveWait();
                work->opponentPlayerNo = index;
                /* Later turn intervals increase the number of coin attempts. */
                if (GwSystem.turnNo <= 10U) {
                    remainingAttempts = mbRandMod(6U) + 5;
                } else if (GwSystem.turnNo <= 20U) {
                    remainingAttempts = mbRandMod(6U) + 10;
                } else if (GwSystem.turnNo <= 30U) {
                    remainingAttempts = mbRandMod(6U) + 15;
                } else if (GwSystem.turnNo <= 40U) {
                    remainingAttempts = mbRandMod(6U) + 20;
                } else {
                    remainingAttempts = mbRandMod(6U) + 25;
                }
                if (remainingAttempts > mbPlayerCoinGet(index)) {
                    remainingAttempts = mbPlayerCoinGet(index);
                }
                mbWipeDissolveFadeIn();
                omVibrate((s16) playerNo, 360, 7, 3);
                omVibrate((s16) index, 360, 7, 3);
                HuPrcSleep(30);
                CharFXPlay((s16) GwPlayer[index].charNo, W02_COIN_LOSS_EFFECT_ID);
                emptySoundPlayed = 0;
                for (elapsed = 0; (f32) elapsed < 180.0f || remainingAttempts > 0; elapsed++) {
                    if (((elapsed & 3) == 0) && (remainingAttempts > 0)) {
                        resisted = 0;
                        if (!GwPlayer[index].comF) {
                            if (HuPadBtnDown[GwPlayer[index].padNo] & 0x100) {
                                resisted = 1;
                            } else {
                                resisted = 0;
                            }
                        } else {
                            if ((100.0f * MBCapsuleEffRandF()) <
                                (10.0f * (f32) ((GwPlayer[index].comDif * 2) + 1))) {
                                resisted = 1;
                            } else {
                                resisted = 0;
                            }
                        }
                        if (resisted == 0) {
                            mbPlayerPosGet(index, &position);
                            position.y += 150.0f;
                            position.x += (0.5f * (100.0f * (-0.5f + MBCapsuleEffRandF())));
                            position.y += (0.5f * (100.0f * (-0.5f + MBCapsuleEffRandF())));
                            position.z += 0.5f * (100.0f * (-0.5f + MBCapsuleEffRandF()));
                            fn_1_126AC(&position, 1);
                            mbPlayerCoinAdd(index, -1);
                            mbAudFXPlay(MSM_SE_CMN_15);
                            lostCoins[index]++;
                        }
                        /* A successful defense still consumes one attempt. */
                        remainingAttempts--;
                    }
                    if ((emptySoundPlayed == 0) && (remainingAttempts <= 0)) {
                        mbAudFXPlay(MSM_SE_CMN_16);
                        emptySoundPlayed = 1;
                    }
                    HuPrcVSleep();
                }
                if (emptySoundPlayed == 0) {
                    mbAudFXPlay(MSM_SE_CMN_16);
                    emptySoundPlayed = 1;
                }
                mbPlayerPosGet(index, &position);
                position.y += 250.0f;
                coinDisplay = mbCoinDispCreate(&position, -lostCoins[index], -1, 0);
                if (coinDisplay != -1) {
                    while (mbCoinDispKillCheck((s16) coinDisplay) == 0) {
                        HuPrcVSleep();
                    }
                }
                mbWipeDissolveFadeOutTime(1);
                if (!GwPlayer[index].comF) {
                    HuPadRumbleStop(GwPlayer[index].padNo);
                }
                work->opponentPlayerNo = -1;
            }
        }
        for (index = 0; index < 4; index++) {
            totalCoins += lostCoins[index];
        }
        mbWinKill((s16) helpWindow);
        cameraRotation.x = -33.0f;
        cameraRotation.y = 0.0f;
        cameraRotation.z = 0.0f;
        position.x = 0.0f;
        position.y = 0.0f;
        position.z = 0.0f;
        mbCameraMoveObj(cameraModel, &cameraRotation, &position, 2700.0f, -1.0f, -1);
        mbCameraMoveWait();
        for (index = 0; (index < 10) && (index < totalCoins); index++) {
            mbPlayerPosGet(playerNo, &position);
            position.x += ((15.0f * (100.0f * MBCapsuleEffRandF())) - 500.0f);
            position.y -= 5.0f * (100.0f * MBCapsuleEffRandF());
            position.z += 1000.0f + (20.0f * (100.0f * MBCapsuleEffRandF()));
            fn_1_126AC(&position, 0);
        }
        mbWipeDissolveFadeIn();
        omVibrate((s16) playerNo, 120, 7, 3);
        timer = 120;
        do {
            work->eventTimer = timer;
            if (timer == 0) {
                omVibrate((s16) playerNo, 180, 4, 4);
            }
            if (timer < 0) {
                work->eventStage = 1;
                if (work->effectScale > 0.0f) {
                    work->effectScale -= 0.002f;
                } else {
                    work->effectScale = 0.0f;
                }
            }
            timer--;
            if ((timer <= 0) && (fn_1_117E8() <= 0) && (fn_1_129C0() <= 0)) {
                endingModel = (s16) work->modelId;
                mbObjAttrReset(endingModel, HU3D_MOTATTR_LOOP);
                mbObjMotionSpeedSet((s16) work->hookedModel, 1.0f);
                if (mbObjMotionEndCheck((s16) work->modelId) != 0) {
                    mbObjMotionSpeedSet((s16) work->modelId, 0.0f);
                    mbObjMotionSpeedSet((s16) work->hookedModel, 0.5f);
                    finished = 1;
                }
            }
            HuPrcVSleep();
        } while (finished == 0);
        if (machineSound != -1) {
            mbAudFXStop(machineSound);
        }
        work->eventStage = 2;
        work->eventPlayerNo = -1;
        mbMasuPosGet(GwPlayer[playerNo].masuId, &start);
        fn_1_FEB4(&firstHook, &secondHook);
        /* The direction between the hooks is computed but is not used by the return sequence. */
        PSVECSubtract(&secondHook, &firstHook, &direction);
        PSVECSubtract(&firstHook, &start, &approach);
        mbPlayerMotionSet(playerNo, 4, 0U);
        mbPlayerRotSet(playerNo, 0.0f, (f32) (HuAtan(-approach.x, -approach.z)), 0.0f);
        for (index = 1; (f32) index <= 30.0f; index++) {
            progress = index / 30.0f;
            position.x = firstHook.x + (progress * (start.x - firstHook.x));
            position.y = (f32) ((f64) (firstHook.y + (progress * (start.y - firstHook.y))) +
                                (4.5 * (100.0 * HuSin(180.0f * progress))));
            position.z = firstHook.z + (progress * (start.z - firstHook.z));
            /* The player rotation is fetched here, but the returned vector is not used. */
            mbPlayerRotGet(playerNo, &cameraRotation);
            mbPlayerPosSetV(playerNo, &position);
            HuPrcVSleep();
        }
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbPlayerRotateStart(playerNo, 0, 15);
        while (mbPlayerRotateCheck(playerNo) == 0) {
            HuPrcVSleep();
        }
        mbCameraPlayerViewSet(playerNo, 1);
        mbCameraMoveWait();
        mbCoinAddDispExec(playerNo, totalCoins, 0, 1);
        mbev_CapCoinDisp(playerNo, totalCoins, 1, 1);
        mbObjMotionTimeSet((s16) work->characterObjectId, 0.0f);
        mbObjMotionSpeedSet((s16) work->characterObjectId, 0.0f);
        mbObjDispSet((s16) work->leverObjectId, 1);
        work->particlesDisabled = 0;
        mbAudFXPlay(W02_LIFT_SOUND_ID);
        for (index = 1; (f32) index <= 60.0f; index++) {
            progress = index / 60.0f;
            mbMasuPosGet(GwPlayer[playerNo].masuId, &position);
            PSVECAdd(&position, &lbl_1_data_BA8, &position);
            position.y += (600.0 - (600.0 * HuSin(90.0f * progress)));
            mbObjPosSetV((s16) work->leverObjectId, &position);
            HuPrcVSleep();
        }
        mbObjMotionSpeedSet((s16) work->liftObjectId, 0.0f);
        mbObjMotionShiftSet((s16) work->attachedModel, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbAudFXPlay(W02_STATION_SELECTION_SOUND_ID);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 18), 14);
        mbWinTopWait();
    }
    mbObjMotionShiftSet((s16) work->attachedModel, 4, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbAudFXPlay(W02_LIFT_MOVE_SOUND_ID);
    for (index = 1; (f32) index <= 60.0f; index++) {
        progress = index / 60.0f;
        mbPlayerPosGet(playerNo, &position);
        PSVECAdd(&position, &lbl_1_data_BA8, &position);
        position.y += 600.0f * (progress * progress);
        mbObjPosSetV((s16) work->leverObjectId, &position);
        HuPrcVSleep();
    }
    mbObjHookObjReset((s16) work->liftObjectId, "oyama2hook");
    mbObjKill((s16) work->attachedModel);
    work->attachedModel = -1;
    mbObjDispSet((s16) work->leverObjectId, 0);
    fn_1_11868();
    fn_1_12030();
    fn_1_12A2C();
    mbObjKill((s16) cameraModel);
    mbPlayerMotionKill(playerNo, motionId);
    mbPlayerColSnapPlayerSet(playerNo, 1);
}

/* Reads the two character-hook positions for the Garage event's paired movement sequence. */
void fn_1_FEB4(HuVecF *firstPosition, HuVecF *secondPosition) {
    Mtx firstMatrix;
    Mtx secondMatrix;
    PositionModel *work;
    int model;

    work = lbl_1_bss_8;
    model = mbObjModelIDGet(work->modelId);
    Hu3DModelObjMtxGet(model, "char1hook", firstMatrix);
    Hu3DModelObjMtxGet(model, "char2hook", secondMatrix);
    firstPosition->x = firstMatrix[0][3];
    firstPosition->y = firstMatrix[1][3];
    firstPosition->z = firstMatrix[2][3];
    secondPosition->x = secondMatrix[0][3];
    secondPosition->y = secondMatrix[1][3];
    secondPosition->z = secondMatrix[2][3];
}

/* Combines the moving lever, lift and character hooks to locate the character. */
void fn_1_FF6C(HuVecF *position) {
    Mtx leverMatrix;
    Mtx liftMatrix;
    Mtx characterMatrix;
    BoardModels *work;

    work = lbl_1_bss_8;
    Hu3DModelObjMtxGet(mbObjModelIDGet(work->leverObjectId), "lever2hook", leverMatrix);
    Hu3DModelObjMtxGet(mbObjModelIDGet(work->liftObjectId), "lifthook", liftMatrix);
    Hu3DModelObjMtxGet(mbObjModelIDGet(work->characterObjectId), "fanchrhook", characterMatrix);
    PSMTXConcat(leverMatrix, liftMatrix, leverMatrix);
    PSMTXConcat(leverMatrix, characterMatrix, leverMatrix);
    position->x = leverMatrix[0][3];
    position->y = leverMatrix[1][3];
    position->z = leverMatrix[2][3];
}

/* Updates the Garage player-flight effect until its target is cleared or the board exits. */
void fn_1_10034(OMOBJ *obj)
{
    PositionModel *work = lbl_1_bss_8;
    if (mbExitCheck() != 0 || work->eventPlayerNo < 0) {
        omDelObjEx(mbObjMan, obj);
    } else {
        fn_1_100A8(
            work->eventPlayerNo,
            work->eventTimer,
            work->opponentPlayerNo);
    }
}

/* Aims the player along two model hooks and emits the event's incoming flight effects. */
void fn_1_100A8(s32 playerNo, s32 emissionState, s32 targetPlayer) {
    Mtx endMatrix;
    Mtx startMatrix;
    HuVecF start;
    HuVecF end;
    HuVecF direction;
    HuVecF position;
    PathBoardModels *work;
    s32 frame;
    s32 modelId;
    PathBoardModels *modelWork;
    s32 index;
    float horizontalLength;

    work = lbl_1_bss_8;
    frame = work->emissionFrame;
    modelWork = lbl_1_bss_8;
    modelId = mbObjModelIDGet(modelWork->positionModel);
    Hu3DModelObjMtxGet(modelId, "char1hook", startMatrix);
    Hu3DModelObjMtxGet(modelId, "char2hook", endMatrix);
    start.x = startMatrix[0][3];
    start.y = startMatrix[1][3];
    start.z = startMatrix[2][3];
    end.x = endMatrix[0][3];
    end.y = endMatrix[1][3];
    end.z = endMatrix[2][3];
    PSVECSubtract(&end, &start, &direction);
    horizontalLength = sqrtf(direction.x * direction.x + direction.z * direction.z);
    mbPlayerPosSetV(playerNo, &start);
    mbPlayerRotSet(playerNo, 180.0 * (atan2(-direction.y, horizontalLength) / M_PI),
        180.0 * (atan2(direction.x, direction.z) / M_PI), 0.0f);
    work->flightOrigin = start;
    work->flightOrigin.y -= 300.0f;
    work->flightDestination = end;
    work->flightDestination.y -= 300.0f;
    if (emissionState > 0 && (frame & 7) == 0) {
        if (targetPlayer == -1) {
            position.x = 60.0f * (100.0f * (-0.5f +
                W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
            position.y = 500.0f;
            position.z = -5000.0f + 60.0f * (100.0f *
                (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
            fn_1_10AC0(&position, 0);
        } else {
            mbPlayerPosGet(targetPlayer, &position);
            position.x += 10.0f * (100.0f * (-0.5f +
                W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
            position.z += 10.0f * (100.0f * (-0.5f +
                W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
            fn_1_10AC0(&position, 1);
        }
    }
    if (emissionState > 0) {
        for (index = 0; index < 2; index++) {
            if (targetPlayer == -1) {
                position.x = 60.0f * (100.0f * (-0.5f +
                    W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                position.y = 500.0f;
                position.z = -5000.0f + 60.0f * (100.0f *
                    (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                fn_1_120B4(&position, 0);
            } else {
                mbPlayerPosGet(targetPlayer, &position);
                position.x += 15.0f * (100.0f * (-0.5f +
                    W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                position.y += 5.0f * (100.0f *
                    (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                position.z += 5.0f * (100.0f *
                    (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
                fn_1_120B4(&position, 1);
            }
        }
    } else if (emissionState <= 0 && (float) emissionState > -60.0f && (frame & 3) == 0) {
        position.x = 60.0f * (100.0f * (-0.5f +
            W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
        position.y = 500.0f;
        position.z = -5000.0f + 60.0f * (100.0f *
            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
        fn_1_120B4(&position, 0);
    }
    work->emissionFrame++;
}
s32 lbl_1_data_BE0[12] = {
    DATANUM(DATA_w02, 35), DATANUM(DATA_w02, 32), DATANUM(DATA_w02, 35),
    DATANUM(DATA_w02, 32), DATANUM(DATA_w02, 35), DATANUM(DATA_w02, 33),
    DATANUM(DATA_w02, 35), DATANUM(DATA_w02, 34), DATANUM(DATA_w02, 35),
    DATANUM(DATA_w02, 35), DATANUM(DATA_w02, 32), DATANUM(DATA_w02, 33)
};

/* Builds 32 hidden flight models, yielding after each pair before enabling updates. */
void fn_1_10954(void) {
    BoardEffects *work;
    EventFlyingEffect *effect;
    int dataNum;
    s32 index;
    int resource;
    EventFlyingEffect *models;

    work = lbl_1_bss_8;
    work->modelObject = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_1119C);
    work->models = models = HuMemDirectMallocNum(HEAP_HEAP,
                                                sizeof(EventFlyingEffect) * 32, HU_MEMNUM_OVL);
    effect = models;
    memset(work->models, 0, sizeof(EventFlyingEffect) * 32);
    work->modelsEnabled = FALSE;
    for (index = 0; index < 32; index++, effect++) {
        dataNum = lbl_1_data_BE0[index % 12];
        dataNum = FILENUM(dataNum);
        if (!GwSystem.curTime) {
            dataNum |= DATA_w02;
        } else {
            dataNum |= DATA_w02n;
        }
        resource = dataNum;
        effect->objectId = mbObjCreate(resource, NULL, TRUE);
        mbObjLayerSet(effect->objectId, 3);
        mbObjDispSet(effect->objectId, FALSE);
        effect->active = FALSE;
        if (index & 1) {
            HuPrcVSleep();
        }
    }
    work->modelsEnabled = TRUE;
}

/* Starts an available flight model, searching from a random slot before wrapping to zero. */
s32 fn_1_10AC0(HuVecF *position, s32 shortFlight) {
    EventFlyingEffect *effect;
    s32 index;
    BoardEffects *work;
    MBMODELID modelId;

    work = lbl_1_bss_8;
    effect = work->models;
    index = mbRandMod(32);
    effect = &work->models[index];
    for (; index < 32; index++, effect++) {
        if (!effect->active) {
            break;
        }
    }
    if (index >= 32) {
        index = 0;
        effect = work->models;
        for (; index < 32; index++, effect++) {
            if (!effect->active) {
                break;
            }
        }
    }
    if (index >= 32) {
        return -1;
    }
    fn_1_10D20(effect, position, 1.0f, 0.75f);
    effect->spinRateScale = 0.5f + 0.5f *
        (W02_RANDOM_FRACTION_SCALE * (f32) (u32) mbRandMod(W02_RANDOM_MODULUS));
    if (shortFlight) {
        effect->remainingFrames = 20;
    }
    mbObjDispSet(effect->objectId, TRUE);
    effect->active = TRUE;
    modelId = effect->objectId;
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    effect->state = index;
    effect->rotation.x = 180.0f *
        (W02_RANDOM_FRACTION_SCALE * (f32) (u32) mbRandMod(W02_RANDOM_MODULUS));
    effect->rotation.y = 180.0f *
        (W02_RANDOM_FRACTION_SCALE * (f32) (u32) mbRandMod(W02_RANDOM_MODULUS));
    effect->rotation.z = 0.0f;
    mbObjPosSetV(effect->objectId, &effect->start);
    mbObjRotSetV(effect->objectId, &effect->rotation);
    return index;
}

/* Prepares a model's curved flight, spiral motion and travel duration. */
void fn_1_10D20(EventFlyingEffect *effect, HuVecF *position, float scale, float spread) {
    HuVecF origin;
    HuVecF destination;
    HuVecF displacement;
    BoardModels *work;
    s32 index;
    float distance;

    work = lbl_1_bss_8;
    /* The shared flight origin is fetched but is not used to prepare this flight. */
    origin = work->flightOrigin;
    destination = work->flightDestination;
    effect->age = 0;
    effect->travelFrames = 0;
    effect->remainingFrames = -1;
    effect->mode = 0;
    effect->previousPosition = *position;
    effect->position = effect->previousPosition;
    for (index = 0; index < 4; index++) {
        effect->positionHistory[index] = effect->position;
    }
    effect->start = *position;
    effect->end = destination;
    PSVECSubtract(&effect->end, &effect->start, &displacement);
    distance = PSVECMag(&displacement);
    PSVECScale(&displacement, &displacement, spread);
    effect->control.x = displacement.x + 0.2f *
        (distance * (-0.5f + W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
    effect->control.y = displacement.y + 0.2f *
        (distance * (-0.5f + W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
    effect->control.z = displacement.z + 0.2f *
        (distance * (-0.5f + W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
    PSVECAdd(&effect->control, &effect->start, &effect->control);
    PSVECSubtract(&effect->end, &effect->start, &displacement);
    effect->spinAngle = 360.0f *
        (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
    effect->spinSpeedSlope = 10.0f + 5.0f *
        (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
    effect->startDrift = PSVECMag(&displacement);
    effect->spiralRadius = 0.15f * effect->startDrift + 100.0f *
        (1.0f + W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
    effect->spinRateScale = 1.0f;
    /* Replace a nonpositive speed scale with 0.01 and keep the resulting trip at least one frame
     * long. */
    if (scale <= 0.0f) {
        scale = 0.01f;
    }
    effect->travelFrames = effect->startDrift / (50.0f * scale);
    if (effect->travelFrames < 1) {
        effect->travelFrames = 1;
    }
}

/* Moves each live model and retires it when its flight or timeout ends. */
void fn_1_1119C(OMOBJ *object) {
    EventFlyingEffect *effect;
    BoardEffects *work;
    s32 index;

    work = lbl_1_bss_8;
    effect = work->models;
    if (mbExitCheck() || work->modelObject == NULL) {
        omDelObjEx(mbObjMan, object);
        return;
    }
    if (work->modelsEnabled) {
        for (index = 0; index < 32; index++, effect++) {
            if (effect->active) {
                fn_1_112B4(effect);
                effect->rotation.z = effect->spinAngle;
                mbObjPosSetV(effect->objectId, &effect->position);
                mbObjRotSetV(effect->objectId, &effect->rotation);
                if (effect->age >= effect->travelFrames || --effect->remainingFrames == 0) {
                    mbObjDispSet(effect->objectId, FALSE);
                    effect->active = FALSE;
                }
            }
        }
    }
}

/* Blends curved flight with a shrinking spiral, retaining recent positions for trails. */
void fn_1_112B4(EventFlyingEffect *effect) {
    Mtx rotation;
    HuVecF origin;
    HuVecF destination;
    HuVecF direction;
    HuVecF offset;
    HuVecF position;
    HuVecF curvePosition;
    HuVecF spiralPosition;
    BoardFlightWork *work;
    float progress;
    float blend;

    work = lbl_1_bss_8;
    origin = work->origin;
    destination = work->destination;
    progress = (float) ++effect->age / (float) effect->travelFrames;
    /* Follow the machine's current destination instead of keeping the endpoint saved at launch. */
    effect->end = destination;
    mbev_CapBezierGetV(progress, &effect->start, &effect->control,
                      &effect->end, &curvePosition);
    PSVECSubtract(&destination, &origin, &direction);
    if (PSVECMag(&direction) > 0.0f) {
        PSVECNormalize(&direction, &direction);
    }
    PSVECScale(&direction, &direction, effect->startDrift *
               (0.3f * FlightCosDeg(90.0f * progress)));
    PSVECAdd(&origin, &direction, &spiralPosition);
    if ((effect->spinAngle += effect->spinRateScale *
        (5.0f + progress * effect->spinSpeedSlope)) > 360.0f) {
        effect->spinAngle -= 360.0f;
    }
    PSVECSubtract(&destination, &origin, &direction);
    mtxRot(rotation, 0.0f,
           180.0 * (atan2(direction.x, direction.z) / 3.141592653589793),
           0.0f);
    offset.x = (1.0f - 0.9f * progress) *
        (effect->spiralRadius * FlightSinDeg(effect->spinAngle));
    offset.y = (1.0f - 0.9f * progress) *
        (effect->spiralRadius * FlightCosDeg(effect->spinAngle));
    offset.z = 0.0f;
    PSMTXMultVec(rotation, &offset, &offset);
    PSVECAdd(&spiralPosition, &offset, &spiralPosition);
    if (progress < 0.75f) {
        blend = 1.333333f * progress;
    } else {
        blend = 1.0f;
    }
    blend = FlightSinDeg(90.0f * blend);
    blend *= blend;
    position.x = curvePosition.x + blend * (spiralPosition.x - curvePosition.x);
    position.y = curvePosition.y + blend * (spiralPosition.y - curvePosition.y);
    position.z = curvePosition.z + blend * (spiralPosition.z - curvePosition.z);
    effect->previousPosition = effect->position;
    effect->position = position;
    if (effect->mode) {
        CopyFlightPosition(&effect->positionHistory[3], &effect->positionHistory[2]);
        CopyFlightPosition(&effect->positionHistory[2], &effect->positionHistory[1]);
        CopyFlightPosition(&effect->positionHistory[1], &effect->positionHistory[0]);
        CopyFlightPosition(&effect->positionHistory[0], &effect->position);
    }
}

/* Counts active Garage flight models; the machine sequence waits for them to finish. */
s32 fn_1_117E8(void) {
    BoardEffects *work;
    EventFlyingEffect *modelEntries;
    s32 modelIndex;
    s32 activeModelCount;
    work = lbl_1_bss_8;
    modelEntries = work->models;
    if (modelEntries == NULL) {
        return 0;
    }
    modelIndex = 0;
    activeModelCount = 0;
    while (modelIndex < 32) {
        if (modelEntries->active != 0) {
            activeModelCount += 1;
        }
        modelIndex += 1;
        modelEntries++;
    }
    return activeModelCount;
}

/* Kills all 32 Garage flight models and frees their table when the machine effects are released. */
void fn_1_11868(void) {
    BoardEffects *manager;
    EventFlyingEffect *objectList;
    s32 index;
    void *allocationToFree;

    manager = lbl_1_bss_8;
    objectList = manager->models;
    if (manager->models != NULL) {
        index = 0;
        while (index < 32) {
            mbObjKill((s16)objectList->objectId);
            index += 1;
            objectList++;
        }
        allocationToFree = manager->models;
        HuMemDirectFree(allocationToFree);
    }
    manager->models = NULL;
    manager->modelObject = NULL;
}
HuVecF lbl_1_data_C10[4] = {
    { -0.5f, 0.0f, 0.5f }, { 0.5f, 0.0f, 0.5f }, { 0.5f, 0.0f, -0.5f }, { -0.5f, 0.0f, -0.5f }
};

/* Allocates 256 inactive effects and creates their shared layer-five sprite model. */
void fn_1_11904(void) {
    BoardModels *work;
    EventSpriteEffect *effect;
    s32 index;
    HU3D_MODEL *model;
    EventSpriteHook *hook;
    void *data;

    work = lbl_1_bss_8;
    work->effectObject = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_11A54);
    data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(EventSpriteEffect) * 256, HU_MEMNUM_OVL);
    work->effects = data;
    effect = data;
    memset(work->effects, 0, sizeof(EventSpriteEffect) * 256);
    for (index = 0; index < 256; index++, effect++) {
        effect->flightSlot = -1;
        effect->active = FALSE;
    }
    work->effectScale = 1.0f;
    work->effectModel = fn_1_15E30(work->effectAnimation, 768);
    fn_1_16E44(work->effectModel, lbl_1_data_C10, NULL);
    fn_1_16E84(work->effectModel, 3, 3, 1);
    Hu3DModelLayerSet(work->effectModel, 5);
    model = &Hu3DData[work->effectModel];
    hook = model->hookData;
    hook->flags |= 0x2C;
}

/* Updates each flight and shifts its former particle transforms down the three-part trail. */
void fn_1_11A54(OMOBJ *object) {
    HuVecF direction;
    HuVecF previousPosition;
    HuVecF savedPosition;
    HuVecF previousRotation;
    HuVecF savedRotation;
    BoardSpriteWork *work;
    EventFlyingEffect *flight;
    textSpriteParticle *particle;
    HU3D_MODEL *model;
    textSpriteHook *hook;
    int effectIndex;
    int particleIndex;
    u8 previousAlpha;
    u8 savedAlpha;
    float previousValue;
    float savedValue;
    float length;

    work = lbl_1_bss_8;
    if (mbExitCheck() || work->effectObject == NULL) {
        omDelObjEx(mbObjMan, object);
        return;
    }
    flight = work->effects;
    model = &Hu3DData[work->effectModel];
    hook = model->hookData;
    particle = hook->particles;
    previousPosition.x = previousPosition.y = previousPosition.z = 0.0f;
    savedPosition = savedRotation = previousRotation = previousPosition;
    for (effectIndex = 0; effectIndex < 256; effectIndex++, flight++) {
        if (!flight->active) {
            particle += 3;
        } else {
            fn_1_112B4(flight);
            if (flight->age >= flight->travelFrames || --flight->remainingFrames == 0) {
                flight->state = -1;
                flight->active = FALSE;
                for (particleIndex = 0; particleIndex < 3; particleIndex++, particle++) {
                    particle->value = 0.0f;
                }
            } else {
                u8 alpha;
                alpha = particle->color.a += particle->alphaStep;
                if (alpha > particle->maxAlpha) {
                    alpha = particle->maxAlpha;
                }
                for (particleIndex = 0; particleIndex < 3; particle++, particleIndex++) {
                    CopySpriteVector(&savedPosition, &particle->position);
                    CopySpriteVector(&savedRotation, &particle->rotation);
                    savedValue = particle->value;
                    savedAlpha = particle->color.a;
                    if (particleIndex == 0) {
                        PSVECSubtract(&flight->positionHistory[particleIndex],
                            &flight->positionHistory[particleIndex + 1], &direction);
                        length = PSVECMag(&direction);
                        PSVECScale(&direction, &direction, 0.5f);
                        PSVECAdd(&flight->positionHistory[particleIndex + 1], &direction,
                            &particle->position);
                        particle->value = length;
                        length = sqrtf(direction.x * direction.x + direction.z * direction.z);
                        particle->rotation.x = 180.0 * (atan2(-direction.y, length) / M_PI);
                        particle->rotation.y = 180.0 * (atan2(direction.x, direction.z) / M_PI);
                        particle->rotation.z = 0.0f;
                        particle->color.a = (float) alpha * work->effectScale;
                    } else {
                        CopySpriteVector(&particle->position, &previousPosition);
                        CopySpriteVector(&particle->rotation, &previousRotation);
                        particle->value = previousValue;
                        particle->color.a = (float) previousAlpha * work->effectScale;
                    }
                    CopySpriteVector(&previousPosition, &savedPosition);
                    CopySpriteVector(&previousRotation, &savedRotation);
                    previousValue = savedValue;
                    previousAlpha = savedAlpha;
                }
            }
        }
    }
}

/* Releases the Garage sprite model and its effect table when its display effect ends. */
void fn_1_12030(void) {
    BoardModels *work = lbl_1_bss_8;
    if (work->effectModel >= 0) {
        Hu3DModelKill((s16)work->effectModel);
    }
    work->effectModel = -1;
    if (work->effects != NULL) {
        void *effectsToFree = work->effects;
        HuMemDirectFree(effectsToFree);
    }
    work->effects = NULL;
    work->effectObject = NULL;
}

/* Starts one free flight effect and initializes its three linked sprite particles. */
void fn_1_120B4(HuVecF *position, s32 shortFlight) {
    HuVecF direction;
    EventFlyingEffect *flight;
    BoardSpriteWork *work;
    textSpriteParticle *particle;
    textSpriteParticle *copy;
    HU3D_MODEL *model;
    textSpriteHook *hook;
    s32 effectIndex;
    s32 copyIndex;
    float horizontalLength;

    work = lbl_1_bss_8;
    flight = work->effects;
    model = &Hu3DData[work->effectModel];
    hook = model->hookData;
    particle = hook->particles;
    for (effectIndex = 0; effectIndex < 256; effectIndex++, flight++, particle += 3) {
        if (flight->state < 0) {
            break;
        }
    }
    if (effectIndex < 256) {
        fn_1_10D20(flight, position,
            1.0f + W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS), 0.75f);
        flight->state = effectIndex;
        flight->spiralRadius *= 1.0f + 0.2f *
            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
        flight->mode = 1;
        if (shortFlight) {
            flight->remainingFrames = 20;
        }
        flight->active = TRUE;
        particle->value = 100.0f * (1.0f + 0.5f *
            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
        particle->position = *position;
        particle->type = 3 * ((frand() & 0x7FFF) % 3);
        particle->color.a = 0;
        particle->maxAlpha = 64.0f + 64.0f *
            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
        particle->alphaStep = 8;
        PSVECSubtract(&work->origin, position, &direction);
        horizontalLength = sqrtf(direction.x * direction.x + direction.z * direction.z);
        particle->rotation.x = 180.0 * (atan2(-direction.y, horizontalLength) / M_PI);
        particle->rotation.y = 180.0 * (atan2(direction.x, direction.z) / M_PI);
        particle->rotation.z = 0.0f;
        copy = particle + 1;
        for (copyIndex = 1; copyIndex < 3; copyIndex++, copy++) {
            copy->value = particle->value;
            CopySpriteVector(&copy->position, &particle->position);
            CopySpriteVector(&copy->rotation, &particle->rotation);
            copy->type = particle->type + copyIndex;
            copy->color.a = particle->color.a;
        }
    }
}

/* Creates and hides the 32 coin models used by Garage effects when the coin-event sequence prepares
 * them. */
void fn_1_125C0(void) {
    s32 coinIndex;
    BoardEffects *work;
    EventFlyingEffect *coinAllocation;
    EventFlyingEffect *coinEntry;

    work = lbl_1_bss_8;
    work->coinObject =
        omAddObjEx(mbObjMan, -32768, 0U, 0U, -1, fn_1_128A8);
    coinAllocation = HuMemDirectMallocNum(HEAP_HEAP, sizeof(EventFlyingEffect) * 32, HU_MEMNUM_OVL);
    work->coins = coinAllocation;
    coinEntry = coinAllocation;
    memset(work->coins, 0, sizeof(EventFlyingEffect) * 32);
    work->coinsEnabled = 0;
    coinIndex = 0;
    while (coinIndex < 32) {
        coinEntry->objectId = (s32) mbCoinCreate2();
        mbCoinObjDispSet((s16) coinEntry->objectId, 0);
        coinEntry->active = 0;
        coinIndex += 1;
        coinEntry++;
    }
    work->coinsEnabled = 1;
}

/* Starts an available flying coin, with an optional fifteen-frame timeout. */
s32 fn_1_126AC(HuVecF *position, s32 shortFlight) {
    EventFlyingEffect *effect;
    s32 index;
    BoardModels *work;

    work = lbl_1_bss_8;
    effect = work->flyingCoins;
    for (index = 0; index < 32; index++, effect++) {
        if (!effect->active) {
            break;
        }
    }
    if (index >= 32) {
        return -1;
    }
    fn_1_10D20(effect, position, 1.0f, 0.5f);
    effect->spinRateScale =
        0.5f + 0.5f * (W02_RANDOM_FRACTION_SCALE * mbRandMod(W02_RANDOM_MODULUS));
    if (shortFlight) {
        effect->remainingFrames = 15;
    }
    mbCoinObjDispSet(effect->objectId, TRUE);
    effect->active = TRUE;
    effect->state = index;
    effect->rotation.x = 180.0f * (W02_RANDOM_FRACTION_SCALE * mbRandMod(W02_RANDOM_MODULUS));
    effect->rotation.y = 180.0f * (W02_RANDOM_FRACTION_SCALE * mbRandMod(W02_RANDOM_MODULUS));
    effect->rotation.z = 0.0f;
    mbCoinObjPosSetV(effect->objectId, &effect->start);
    mbCoinObjRotSetV(effect->objectId, &effect->rotation);
    return index;
}

/* Applies the same flight update to coin objects owned by the board event. */
void fn_1_128A8(OMOBJ *object) {
    EventFlyingEffect *effect;
    BoardEffects *work;
    s32 index;

    work = lbl_1_bss_8;
    effect = work->coins;
    if (mbExitCheck() || work->coinObject == NULL) {
        omDelObjEx(mbObjMan, object);
        return;
    }
    if (work->coinsEnabled) {
        for (index = 0; index < 32; index++, effect++) {
            if (effect->active) {
                fn_1_112B4(effect);
                effect->rotation.z = effect->spinAngle;
                mbCoinObjPosSetV(effect->objectId, &effect->position);
                mbCoinObjRotSetV(effect->objectId, &effect->rotation);
                if (effect->age >= effect->travelFrames || --effect->remainingFrames == 0) {
                    mbCoinObjDispSet(effect->objectId, FALSE);
                    effect->active = FALSE;
                }
            }
        }
    }
}

/* Counts Garage coin slots whose effect state is still active while the sequence waits. */
s32 fn_1_129C0(void) {
    BoardEffects *manager;
    EventFlyingEffect *record;
    s32 coinIndex;
    s32 active_count;

    manager = lbl_1_bss_8;
    record = manager->coins;
    coinIndex = 0;
    active_count = 0;
    while (coinIndex < 32) {
        if (record->active != 0) {
            active_count += 1;
        }
        coinIndex += 1;
        record++;
    }
    return active_count;
}

/* Releases all 32 Garage coin objects and frees their table after the machine effect. */
void fn_1_12A2C(void) {
    BoardEffects *manager;
    EventFlyingEffect *coinObjectList;
    s32 index;
    void *allocationToFree;

    manager = lbl_1_bss_8;
    coinObjectList = manager->coins;
    if (manager->coins != NULL) {
        index = 0;
        while (index < 32) {
            mbCoinObjNumDec((s16)coinObjectList->objectId);
            index += 1;
            coinObjectList++;
        }
        allocationToFree = manager->coins;
        HuMemDirectFree(allocationToFree);
    }
    manager->coins = NULL;
    manager->coinObject = NULL;
}

/* Prepares a player's short move toward an event position and its three motions. */
OMOBJ *fn_1_12AC8(s32 playerNo, HuVecF *eventPosition) {
    HuVecF direction;
    PlayerMove *work;
    OMOBJ *object;

    object = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_12CB8);
    work = object->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(PlayerMove), HU_MEMNUM_OVL);
    memset(work, 0, sizeof(PlayerMove));
    work->playerNo = playerNo;
    /* The original space is saved here, but this movement's updater never reads it. */
    work->spaceId = GwPlayer[playerNo].masuId;
    work->state = 0;
    work->timer = 0;
    work->returnFrames = 0;
    work->moveDistance = 100.0f * (0.3f + 0.2f *
        (W02_RANDOM_FRACTION_SCALE * mbRandMod(W02_RANDOM_MODULUS)));
    mbPlayerPosGet(playerNo, &work->start);
    PSVECSubtract(eventPosition, &work->start, &direction);
    direction.y = 0.0f;
    if (PSVECMag(&direction) > 0.0f) {
        PSVECNormalize(&direction, &direction);
    }
    PSVECScale(&direction, &direction, work->moveDistance);
    PSVECAdd(&work->start, &direction, &work->destination);
    work->motions[0] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 35));
    work->motions[1] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 18));
    work->motions[2] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 19));
    mbPlayerColSnapPlayerSet(work->playerNo, 0);
    return object;
}

/* Updates opponents' sway and return each frame during the coin-draining machine event. */
void fn_1_12CB8(OMOBJ *object) {
    HuVecF position;
    HuVecF rotation;
    PathBoardModels *work;
    PlayerMove *move;
    float progress;

    work = lbl_1_bss_8;
    move = object->data;
    if (mbExitCheck()) {
        omDelObjEx(mbObjMan, object);
        return;
    }
    /* The timer measures degrees while swaying and frames while returning or waiting. */
    switch (move->state) {
    case 0:
        move->timer += 1.0f + 2.0f *
            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
        progress = fabs(PlayerSinDeg(move->timer));
        position.x = move->start.x + progress * (move->destination.x - move->start.x);
        position.y = move->start.y + progress * (move->destination.y - move->start.y);
        position.z = move->start.z + progress * (move->destination.z - move->start.z);
        mbPlayerPosSetV(move->playerNo, &position);
        if (move->timer >= 90) {
            mbPlayerMotionShiftSet(move->playerNo, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            move->state++;
            move->returnFrames = 0;
        }
        break;
    case 1:
        move->timer += 1.0f + 2.0f *
            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
        progress = fabs(PlayerSinDeg(move->timer));
        position.x = move->start.x + progress * (move->destination.x - move->start.x);
        position.y = move->start.y + progress * (move->destination.y - move->start.y);
        position.z = move->start.z + progress * (move->destination.z - move->start.z);
        mbPlayerPosSetV(move->playerNo, &position);
        mbPlayerRotGet(move->playerNo, &rotation);
        PSVECSubtract(&move->start, &move->destination, &position);
        rotation.y = mbev_CapAngleLerp(180.0 * (atan2(position.x, position.z) / M_PI),
            rotation.y, 5.0f);
        mbPlayerRotSetV(move->playerNo, &rotation);
        if (move->timer >= 180) {
            mbPlayerMotionShiftSet(move->playerNo, move->motions[0], 0.0f, 8.0f,
                HU3D_MOTATTR_LOOP);
            move->state++;
            move->returnFrames = 0;
        }
        break;
    case 2:
        move->timer += 1.0f + 2.0f *
            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
        progress = fabs(PlayerSinDeg(move->timer));
        position.x = move->start.x + progress * (move->destination.x - move->start.x);
        position.y = move->start.y + progress * (move->destination.y - move->start.y);
        position.z = move->start.z + progress * (move->destination.z - move->start.z);
        mbPlayerPosSetV(move->playerNo, &position);
        if (work->eventStage >= 1) {
            mbPlayerMotionShiftSet(move->playerNo, move->motions[1], 0.0f, 8.0f, 0);
            mbPlayerPosGet(move->playerNo, &move->destination);
            move->state++;
            move->timer = 0;
            move->returnFrames = 8;
        }
        break;
    case 3:
        progress = 1.0f - (float) ++move->timer / (float) move->returnFrames;
        if (progress > 1.0f) {
            progress = 1.0f;
        } else if (progress < 0.0f) {
            progress = 0.0f;
        }
        position.x = move->start.x + progress * (move->destination.x - move->start.x);
        position.y = move->start.y + progress * (move->destination.y - move->start.y);
        position.z = move->start.z + progress * (move->destination.z - move->start.z);
        mbPlayerPosSetV(move->playerNo, &position);
        if (progress <= 0.0f && mbPlayerMotionEndCheck(move->playerNo)) {
            move->state++;
            move->timer = 0;
            move->returnFrames = 0;
        }
        break;
    case 4:
        if (work->eventStage >= 2) {
            mbPlayerMotionSet(move->playerNo, move->motions[2], 0);
            move->state++;
            move->timer = 0;
            move->returnFrames = 0;
        }
        break;
    case 5:
        if (mbPlayerMotionEndCheck(move->playerNo)) {
            mbPlayerRotateStart(move->playerNo, 0, 15);
            mbPlayerMotionShiftSet(move->playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            move->state++;
            move->timer = 0;
            move->returnFrames = 0;
        }
        break;
    case 6:
        if (++move->timer > 20) {
            mbPlayerMotionKill(move->playerNo, move->motions[0]);
            mbPlayerMotionKill(move->playerNo, move->motions[1]);
            mbPlayerMotionKill(move->playerNo, move->motions[2]);
            /* This completion does not re-enable the opponent's collision snapping disabled during
             * setup. */
            omDelObjEx(mbObjMan, object);
            return;
        }
        break;
    default:
        break;
    }
}

/* Creates the parent and its initially hidden attachment with frozen motion. */
void fn_1_135B0(void) {
    s32 modelId;
    BoardModels *work;

    work = lbl_1_bss_8;
    modelId = mbObjCreate(BoardModelDataNum(DATANUM(DATA_w02, 55)), NULL, FALSE);
    work->parentModel = modelId;
    mbObjLayerSet(modelId, 3);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjLayerSet(modelId, 3);
    modelId = mbObjCreate(BoardModelDataNum(DATANUM(DATA_w02, 56)), NULL, FALSE);
    work->pairedModel = modelId;
    mbObjLayerSet(modelId, 3);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjHookSet(work->parentModel, "cpmac", modelId);
    mbObjDispSet(modelId, FALSE);
}
int lbl_1_data_C64[4][5] = {
    { 10, 30, 60, 100, 150 }, { 10, 25, 50, 80, 120 }, { 10, 20, 40, 60, 90 }, { 5, 15, 30, 50, 75 }
};

HuVecF lbl_1_data_CB4 = { 3055.0f, 682.0f, -469.0f };

HuVecF lbl_1_data_CC0 = { 325.0f, 0.0f, 0.0f };

HuVecF lbl_1_data_CCC = { 2998.0f, 1111.0f, 22.0f };

/* Offers up to five launches, aims the cannon, and restores the player after the station event. */
void fn_1_13708(int playerNo, int spaceId) {
    int capsules[33];
    Mtx parentMatrix;
    Mtx barrelStartMatrix;
    Mtx barrelEndMatrix;
    char costText[5][16];
    int motionData[16];
    HuVecF playerStart;
    HuVecF stationPosition;
    HuVecF direction;
    HuVecF position;
    HuVecF rotation;
    char numberText[16];
    HuVecF effectOrigin;
    HuVecF effectAngles;
    int readStatus;
    int availableChoices;
    int cameraModel;
    struct { int coins; } budget;
    int capsuleCount;
    CapsuleShot *releasedShots;
    CapsuleShot *allocatedShots;
    s16 *allocatedDestinations;
    s16 *allocatedOccupiedSpaces;
    s16 *destinations;
    s16 *occupiedSpaces;
    HuVecF *angles;
    HuVecF *origin;
    s16 shuffledSpace;
    s16 originalSpace;
    s16 nextSpace;
    s16 exitSpace;
    int choice;
    s16 turnFrames;
    s16 cameraStack;
    CapsuleStation *work;
    int *costs;
    CapsuleShot *shot;
    OMOBJ *object;
    int index;
    int count;
    int firstIndex;
    int randomIndex;
    int selectionValue;
    int capsuleIndex;
    s16 occupiedCount;
    float progress;
    float currentAngle;
    float targetAngle;
    float throwProgress;
    float turnAngle;

    work = lbl_1_bss_8;
    allocatedShots = HuMemDirectMallocNum(HEAP_HEAP, 380, HU_MEMNUM_OVL);
    shot = work->shots = allocatedShots;
    memset(shot, 0, 380);
    work->shotIndex = work->shotCount = 0;
    costs = lbl_1_data_C64[GwPlayer[playerNo].rank];
    originalSpace = spaceId;
    nextSpace = mbMasuAttrFindLink(originalSpace, W02_STATION_LINK_ATTR_FLAG);
    exitSpace = mbMasuAttrFindLink(nextSpace, W02_STATION_LINK_ATTR_FLAG);
    mbMasuPosGet(originalSpace, &playerStart);
    mbMasuPosGet(nextSpace, &stationPosition);
    if ((readStatus = mbBGRead(DATA_capsulechar3)) != -1) {
        mbBGReadWait(readStatus);
    }
    motionData[0] = DATANUM(DATA_capsulechar3, 1);
    motionData[1] = DATANUM(DATA_capsulechar3, 3);
    motionData[2] = DATANUM(DATA_capsulechar3, 4);
    motionData[3] = DATANUM(DATA_capsulechar3, 5);
    motionData[4] = -1;
    work->stationModel = mbObjCreate(DATANUM(DATA_capsulechar3, 0), motionData, 0);
    mbObjLayerSet(work->stationModel, 3);
    HuPrcVSleep();
    mbObjMotionSet(work->stationModel, 1, HU3D_MOTATTR_LOOP);
    mbMasuPosGet(exitSpace, &position);
    mbObjPosSetV(work->stationModel, &position);
    mbObjRotSet(work->stationModel, 0.0f, -180.0f, 0.0f);
    HuDataDirClose(DATA_capsulechar3);
    PSVECSubtract(&stationPosition, &playerStart, &direction);
    mbPlayerMotionSet(playerNo, 4, 0);
    mbPlayerRotSet(playerNo, 0.0f,
        180.0 * (atan2(direction.x, direction.z) / M_PI), 0.0f);
    for (index = 1; (float) index <= 30.0f; index++) {
        progress = (float) index / 30.0f;
        mbev_CapVecChase(progress, &playerStart, &stationPosition, &position);
        position.y += 2.0 * (100.0 * sin((M_PI * (180.0f * progress)) / 180.0));
        mbPlayerPosSetV(playerNo, &position);
        HuPrcVSleep();
    }
    mbPlayerPosSetV(playerNo, &stationPosition);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    position.x = 0.0f;
    position.y = 100.0f;
    position.z = -300.0f;
    mbCameraMoveObj(work->stationModel, NULL, &position, -1.0f, -1.0f, 45);
    mbCameraMoveWait();
    mbAudFXPlay(W02_STATION_CHOICE_SOUND_ID);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 19), 14);
    mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo), 0);
    mbWinTopWait();
    for (index = 1, count = 0; index < mbMasuNumGet(); index++) {
        if (mbCapThrowMasuCheck(index)) {
            count++;
        }
    }
    if (mbPlayerCoinGet(playerNo) < costs[0]) {
        mbAudFXPlay(W02_CANNON_UNAVAILABLE_SOUND_ID);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 22), 14);
        mbWinTopWait();
    } else if (count <= 0) {
        mbAudFXPlay(W02_CANNON_UNAVAILABLE_SOUND_ID);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 21), 14);
        mbWinTopWait();
    } else {
        do {
            mbAudFXPlay(W02_STATION_ACCEPT_SOUND_ID);
            mbWinCreateChoice(1, MESSNUM(MESS_BOARD_W02, 20), 14, 0);
            for (index = 0, availableChoices = 0; index < 5; index++) {
                if (mbPlayerCoinGet(playerNo) < costs[index] || count <= index) {
                    mbWinTopChoiceDisable(index);
                } else {
                    availableChoices++;
                }
            }
            sprintf(numberText, "%d", availableChoices);
            mbWinTopInsertMesSet((u32) numberText, 0);
            if (costs[4] >= 100) {
                sprintf(costText[0], "%3d", costs[0]);
                mbWinTopInsertMesSet((u32) costText[0], 1);
                sprintf(costText[1], "%3d", costs[1]);
                mbWinTopInsertMesSet((u32) costText[1], 2);
                sprintf(costText[2], "%3d", costs[2]);
                mbWinTopInsertMesSet((u32) costText[2], 3);
                sprintf(costText[3], "%3d", costs[3]);
                mbWinTopInsertMesSet((u32) costText[3], 4);
                sprintf(costText[4], "%3d", costs[4]);
                mbWinTopInsertMesSet((u32) costText[4], 5);
            } else {
                sprintf(costText[0], "%d", costs[0]);
                mbWinTopInsertMesSet((u32) costText[0], 1);
                sprintf(costText[1], "%2d", costs[1]);
                mbWinTopInsertMesSet((u32) costText[1], 2);
                sprintf(costText[2], "%2d", costs[2]);
                mbWinTopInsertMesSet((u32) costText[2], 3);
                sprintf(costText[3], "%2d", costs[3]);
                mbWinTopInsertMesSet((u32) costText[3], 4);
                sprintf(costText[4], "%2d", costs[4]);
                mbWinTopInsertMesSet((u32) costText[4], 5);
            }
            if (GwPlayer[playerNo].comF) {
                budget.coins = mbPlayerCoinGet(playerNo);
                if (mbMasuFind_TypeStepGet(spaceId, 7) <= 10) {
                    budget.coins -= 20;
                }
                if (budget.coins > costs[2]) {
                    budget.coins -= mbRandMod(10);
                }
                for (index = 0, selectionValue = -1; index < 5 && index < count; index++) {
                    if (budget.coins <= costs[index]) {
                        break;
                    }
                    selectionValue = index;
                }
                if (selectionValue >= 0) {
                    mbComChoiceListDownSet(selectionValue);
                } else {
                    mbComChoiceListDownSet(-1);
                }
            }
            mbWinTopWait();
            choice = mbWinTopChoiceGet();
            if (choice >= 0 && choice <= 4) {
                work->shotCount = choice + 1;
            } else if (choice == 5) {
                mbev_Scroll(playerNo, 0);
            }
        } while (choice == 5);
        if (work->shotCount == 0) {
            mbAudFXPlay(W02_CANNON_CANCEL_SOUND_ID);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 25), 14);
            mbWinTopWait();
        } else {
            mbCoinAddDispExec(playerNo, -costs[choice], 1, 1);
            allocatedDestinations = HuMemDirectMallocNum(HEAP_HEAP, 512, HU_MEMNUM_OVL);
            destinations = allocatedDestinations;
            allocatedOccupiedSpaces = HuMemDirectMallocNum(HEAP_HEAP, 512, HU_MEMNUM_OVL);
            occupiedSpaces = allocatedOccupiedSpaces;
            memset(destinations, 0, 512);
            memset(occupiedSpaces, 0, 512);
            for (index = 1, count = 0; index < mbMasuNumGet(); index++) {
                if (mbCapThrowMasuCheck(index) && mbCapMasuDispTypeGet(index) == 0) {
                    destinations[count] = index;
                    count++;
                }
            }
            for (index = 0; index < count; index++) {
                firstIndex = index;
                randomIndex = mbRandMod(count);
                if (firstIndex != randomIndex) {
                    shuffledSpace = destinations[firstIndex];
                    destinations[firstIndex] = destinations[randomIndex];
                    destinations[randomIndex] = shuffledSpace;
                }
            }
            if (count < 5) {
                for (index = 1, occupiedCount = 0; index < mbMasuNumGet(); index++) {
                    if (mbCapThrowMasuCheck(index) && mbCapMasuDispTypeGet(index) != 0) {
                        if (!mbev_CapPlayerCheck(mbCapMasuPlayerGet(index), playerNo)) {
                            occupiedSpaces[occupiedCount] = index;
                            occupiedCount++;
                        }
                    }
                }
                if (occupiedCount > 1) {
                    for (index = 0; index < occupiedCount; index++) {
                        firstIndex = index;
                        randomIndex = mbRandMod(occupiedCount);
                        if (firstIndex != randomIndex) {
                            shuffledSpace = occupiedSpaces[firstIndex];
                            occupiedSpaces[firstIndex] = occupiedSpaces[randomIndex];
                            occupiedSpaces[randomIndex] = shuffledSpace;
                        }
                    }
                }
                for (index = 0; index < occupiedCount; index++) {
                    destinations[count] = occupiedSpaces[index];
                    count++;
                }
            }
            if (count < 5) {
                for (index = 1, occupiedCount = 0; index < mbMasuNumGet(); index++) {
                    if (mbCapThrowMasuCheck(index) && mbCapMasuDispTypeGet(index) != 0 &&
                        mbev_CapPlayerCheck(mbCapMasuPlayerGet(index), playerNo)) {
                        occupiedSpaces[occupiedCount] = index;
                            occupiedCount++;
                    }
                }
                if (occupiedCount > 1) {
                    for (index = 0; index < occupiedCount; index++) {
                        firstIndex = index;
                        randomIndex = mbRandMod(occupiedCount);
                        if (firstIndex != randomIndex) {
                            shuffledSpace = occupiedSpaces[firstIndex];
                            occupiedSpaces[firstIndex] = occupiedSpaces[randomIndex];
                            occupiedSpaces[randomIndex] = shuffledSpace;
                        }
                    }
                }
                for (index = 0; index < occupiedCount; index++) {
                    destinations[count] = occupiedSpaces[index];
                    count++;
                }
            }
            for (index = 0, shot = work->shots;
                index < work->shotCount && index < count; index++, shot++) {
                shot->ready = FALSE;
                shot->playerNo = playerNo;
                shot->destinationSpace = destinations[index];
                shot->travelFrames = 0;
                shot->start = lbl_1_data_CCC;
                shot->bend = shot->start;
                mbMasuPosGet(shot->destinationSpace, &shot->end);
            }
            HuMemDirectFree(destinations);
            HuMemDirectFree(occupiedSpaces);
            capsuleCount = mbCapRandomListGet(capsules, 33);
            for (index = 0; index < capsuleCount; index++) {
                firstIndex = index;
                randomIndex = mbRandMod(capsuleCount);
                selectionValue = capsules[firstIndex];
                capsules[firstIndex] = capsules[randomIndex];
                capsules[randomIndex] = selectionValue;
            }
            object = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_14F9C);
            object->work[0] = object->work[1] = object->work[2] = object->work[3] = 0;
            mbAudFXPlay(W02_STATION_EXIT_SOUND_ID);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 23), 14);
            sprintf(numberText, "%d", work->shotCount);
            mbWinTopInsertMesSet((u32) numberText, 0);
            mbWinTopWait();
            mbWipeSpecialFadeInCreate(1, 1);
            mbObjDispSet(work->cannonModel, 1);
            mbObjRotSet(work->cannonModel, 0.0f, 0.0f, 0.0f);
            mbObjMotionTimeSet(work->cannonModel, 0.0f);
            mbObjMotionSpeedSet(work->cannonModel, 1.0f);
            mbObjHookSet(work->cannonModel, "cpoyhook", work->stationModel);
            mbObjMotionTimeSet(work->cannonModel, 0.0f);
            mbObjMotionSet(work->stationModel, 2, HU3D_MOTATTR_LOOP);
            mbObjMotionTimeSet(work->stationModel, 0.0f);
            mbObjPosSet(work->stationModel, 0.0f, 0.0f, 0.0f);
            mbObjRotSet(work->stationModel, 0.0f, 0.0f, 0.0f);
            /* The camera is saved on the stack, but the returned stack index is not used here. */
            cameraStack = mbCameraStackPush();
    cameraModel = mbObjCreate(DATANUM(DATA_capsule, 68), NULL, 0);
            mbObjDispSet(cameraModel, 0);
            mbObjPosSetV(cameraModel, &lbl_1_data_CB4);
            mbCameraMoveObj(cameraModel, &lbl_1_data_CC0, NULL, 3145.0f, -1.0f, -1);
            mbCameraMoveWait();
            mbWipeSpecialFadeOutCreate(1, 60);
            omVibrate(playerNo, 90, 4, 4);
            mbAudFXPlay(W02_CANNON_READY_SOUND_ID);
            mbObjMotionTimeSet(work->launchBaseModel, 0.0f);
            mbObjMotionSpeedSet(work->launchBaseModel, 1.0f);
            HuPrcSleep(50);
            mbAudFXPlay(W02_CANNON_LOAD_SOUND_ID);
            mbAudFXPlay(W02_CANNON_TURN_SOUND_ID);
            while (!mbObjMotionEndCheck(work->launchBaseModel)) {
                HuPrcVSleep();
            }
            mbAudFXPlay(W02_CANNON_TURN_END_SOUND_ID);
            currentAngle = 0.0f;
            work->shotsStopped = FALSE;
            for (index = 0, shot = work->shots; index < work->shotCount; index++, shot++) {
                PSVECSubtract(&shot->end, &shot->start, &position);
                targetAngle = 180.0 * (atan2(position.x, position.z) / M_PI);
                turnAngle = fabs(mbev_CapAngleWrap(currentAngle, targetAngle));
                turnFrames = turnAngle / 2.0f;
                /* A cannon turn lasts at least 20 frames, even when the target angle is
                 * unchanged. */
                if (turnFrames < 20) {
                    turnFrames = 20;
                }
                if (turnFrames > 1) {
                    mbAudFXPlay(W02_CANNON_TURN_SOUND_ID);
                }
                for (selectionValue = 1; selectionValue <= turnFrames; selectionValue++) {
                    progress = (float) selectionValue / (float) turnFrames;
                    progress = (float) sin((M_PI * (90.0f * progress)) / 180.0);
                    turnAngle = mbev_CapAngleSumLerp(progress, currentAngle, targetAngle);
                    mbObjRotSet(work->cannonModel, 0.0f, turnAngle, 0.0f);
                    HuPrcVSleep();
                }
                currentAngle = targetAngle;
                mbObjRotSet(work->cannonModel, 0.0f, currentAngle, 0.0f);
                if (turnFrames > 1) {
                    mbAudFXPlay(W02_CANNON_TURN_END_SOUND_ID);
                }
                Hu3DModelObjMtxGet(mbObjModelIDGet(work->launchBaseModel), "cpmac",
                    parentMatrix);
                Hu3DModelObjMtxGet(mbObjModelIDGet(work->cannonModel), "cpcphook",
                    barrelStartMatrix);
                Hu3DModelObjMtxGet(mbObjModelIDGet(work->cannonModel), "cp2hook",
                    barrelEndMatrix);
                PSMTXConcat(parentMatrix, barrelStartMatrix, barrelStartMatrix);
                PSMTXConcat(parentMatrix, barrelEndMatrix, barrelEndMatrix);
                position.x = barrelEndMatrix[0][3];
                position.y = barrelEndMatrix[1][3];
                position.z = barrelEndMatrix[2][3];
                rotation.x = barrelEndMatrix[0][3] - barrelStartMatrix[0][3];
                rotation.y = barrelEndMatrix[1][3] - barrelStartMatrix[1][3];
                rotation.z = barrelEndMatrix[2][3] - barrelStartMatrix[2][3];
                if (PSVECMag(&rotation) > 0.0f) {
                    PSVECNormalize(&rotation, &rotation);
                }
                direction = rotation;
                mbev_CapVecRotGet(&rotation, &rotation);
                rotation.x += 90.0f;
                effectAngles = rotation;
                angles = &effectAngles;
                effectOrigin = position;
                origin = &effectOrigin;
                fn_1_17894(origin, angles, 16);
                mbCameraShakeSet(30, 100.0f);
                mbAudFXPlay(W02_CANNON_FIRE_SOUND_ID);
                omVibrate(playerNo, 20, 7, 3);
                shot->trailStart = position;
                shot->trailEnd = position;
                PSVECScale(&direction, &direction, 1000.0f);
                PSVECAdd(&shot->trailEnd, &direction, &shot->trailEnd);
                object = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_150F0);
                object->work[0] = object->work[2] = object->work[3] = 0;
                object->work[1] = index;
                HuPrcSleep(30);
            }
            mbWipeSpecialFadeInCreate(1, 1);
            work->shotsStopped = TRUE;
            work->mapEffectsHidden = TRUE;
            mbObjDispSet(work->mapDisplayModel, 0);
            for (index = 0, capsuleIndex = 0, shot = work->shots;
                index < work->shotCount; index++, shot++) {
                while (mbCapUseModeGet(capsules[capsuleIndex]) != 1 &&
                    mbCapUseModeGet(capsules[capsuleIndex]) != 2) {
                    if (++capsuleIndex >= capsuleCount) {
                        capsuleIndex = 0;
                    }
                }
                while (!shot->ready) {
                    HuPrcVSleep();
                }
                if ((float) shot->travelFrames < 18.0f) {
                    throwProgress = 0.0f;
                } else if ((float) shot->travelFrames < 60.0f) {
                    throwProgress = 0.4f;
                } else if ((float) shot->travelFrames < 120.0f) {
                    throwProgress = 0.5f;
                } else if ((float) shot->travelFrames < 180.0f) {
                    throwProgress = 0.6f;
                } else {
                    throwProgress = 0.7f;
                }
                mbCapAutoThrow(&shot->start, &shot->bend, &shot->end, playerNo,
                    shot->destinationSpace, capsules[capsuleIndex], shot->travelFrames,
                    throwProgress);
                if (++capsuleIndex >= capsuleCount) {
                    capsuleIndex = 0;
                }
                HuPrcSleep(2);
            }
            work->mapEffectsHidden = FALSE;
            mbObjDispSet(work->mapDisplayModel, 1);
            mbObjMotionTimeSet(work->launchBaseModel, 0.0f);
            mbObjMotionSpeedSet(work->launchBaseModel, 0.0f);
            mbObjHookReset(work->cannonModel);
            mbObjRotSet(work->cannonModel, 0.0f, 0.0f, 0.0f);
            mbMasuPosGet(exitSpace, &position);
            mbObjPosSetV(work->stationModel, &position);
            mbObjRotSet(work->stationModel, 0.0f, -180.0f, 0.0f);
            mbObjMotionSet(work->stationModel, 3, HU3D_MOTATTR_LOOP);
            position.x = 0.0f;
            position.y = 100.0f;
            position.z = -300.0f;
            mbCameraMoveObj(work->stationModel, NULL, &position, -1.0f, -1.0f, -1);
            mbCameraMoveWait();
            mbCameraStackPop(1);
            mbObjKill(cameraModel);
            mbWipeSpecialFadeOutCreate(1, 60);
            mbAudFXPlay(W02_CANNON_FINISH_SOUND_ID);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W02, 24), 14);
            sprintf(numberText, "%d", work->shotCount);
            mbWinTopInsertMesSet((u32) numberText, 0);
            mbWinTopWait();
        }
    }
    mbWipeDissolveFadeOutTime(1);
    mbMasuPosGet(originalSpace, &position);
    mbPlayerPosSetV(playerNo, &position);
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    mbPlayerColSnapPlayerSet(playerNo, 1);
    mbCameraPlayerViewSetFast(playerNo, 1);
    mbCameraMoveWait();
    mbObjHookReset(work->cannonModel);
    mbObjKill(work->stationModel);
    work->stationModel = -1;
    mbObjDispSet(work->cannonModel, 0);
    releasedShots = work->shots;
    HuMemDirectFree(releasedShots);
    work->shots = NULL;
    mbWipeDissolveFadeInTime(60);
}

/* Starts each path search, saves its result, and stops after the final path. */
void fn_1_14F9C(OMOBJ *object) {
    HuVecF start;
    HuVecF end;
    int travelFrames;
    throwpathsThrowPath *path;
    ThrowPathWork *work;

    work = lbl_1_bss_8;
    if (mbExitCheck() || object->work[3] != 0U) {
        omDelObjEx(mbObjMan, object);
        return;
    }
    path = &work->paths[work->pathIndex];
    switch (object->work[0]) {
    case 0:
        start = path->start;
        end = path->end;
        mbCapEffThrowCreate(&start, &end);
        object->work[0]++;
        break;
    case 1:
        if (mbCapEffThrowCheck(&start, &travelFrames)) {
            path->ready = TRUE;
            path->travelFrames = travelFrames;
            path->bend = start;
            if (++work->pathIndex >= work->pathCount) {
                object->work[3] = 1;
                return;
            }
            object->work[0] = 0;
        }
        break;
    }
}

/* Emits one pale trail particle per update, then checks whether thirty emissions have passed
* or shots have stopped. */
void fn_1_150F0(OMOBJ *object) {
    HuVecF position;
    HuVecF velocity;
    HuVecF particlePosition;
    HuVecF particleVelocity;
    GXColor color;
    GXColor particleColor;
    GXColor *colorPtr;
    HuVecF *velocityPtr;
    HuVecF *positionPtr;
    EventThrowPath *path;
    BoardModels *work;
    float brightness;
    float progress;

    work = lbl_1_bss_8;
    if (mbExitCheck() || object->work[3] != 0U) {
        omDelObjEx(mbObjMan, object);
        return;
    }
    path = &work->throwPaths[object->work[1]];
    switch ((s32) object->work[0]) {
    case 0:
    progress = (float) ++object->work[2] / 30.0f;
    mbev_CapVecChase(progress, &path->trailStart, &path->trailEnd, &position);
    velocity.x = velocity.y = velocity.z = 0.0f;
    brightness = 63.0f * (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
    color.r = 192.0f + brightness;
    color.g = 192.0f + brightness;
    color.b = 192.0f + brightness;
    color.a = 192.0f + 63.0f *
        (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
    particleColor = color;
    colorPtr = &particleColor;
    particleVelocity = velocity;
    velocityPtr = &particleVelocity;
    particlePosition = position;
    positionPtr = &particlePosition;
    fn_1_176E4(positionPtr, velocityPtr,
        100.0f * (1.0f + 0.5f *
            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS))),
        -0.5f + W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS),
        0.33f + 0.66f *
            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)), colorPtr);
    if (progress >= 1.0f || work->throwStopped) {
        object->work[0]++;
        object->work[3] = 1;
    }
    break;
    }
}

/* Initializes the ordering of spaces that can hold the Garage star. */
void fn_1_154E8(void) {
    s32 index;

    *((s8 *)(u8 *)lbl_1_bss_4 + W02_BOARD_EVENT_STATE_OFFSET) = 0;
    *((u8 *)(u8 *)lbl_1_bss_4 + 5) = mbMasuMAttrListGet(W02_STAR_SPACE_GROUP_MASK, NULL);
    index = 0;
    while (index < *((u8 *)(u8 *)lbl_1_bss_4 + 5)) {
        *((u8 *)(u8 *)lbl_1_bss_4 + index + 6) = (u8)index;
        index++;
    }
}

/* Moves the Garage star to the space selected by the current randomized order. */
void fn_1_1557C(void) {
    s16 spaceIds[8];
    int spaceCount;

    /* The space-ID list is used below; the returned count is stored but never read. */
    spaceCount = mbMasuMAttrListGet(W02_STAR_SPACE_GROUP_MASK, spaceIds);
    lbl_1_bss_14 = spaceIds[lbl_1_bss_4->order[lbl_1_bss_4->position]];
    mbMasuTypeSet(lbl_1_bss_14, 7);
    mbStarMasuNextSet(lbl_1_bss_14);
    fn_1_1560C();
}

/* Reattaches the visible star to the hook selected by its current space group. */
void fn_1_1560C(void) {
    char hookName[32];
    s32 hookIndex;
    StarDisplayWork *work;

    work = lbl_1_bss_8;
    mbObjHookObjReset(lbl_1_bss_5C, "starhook1");
    mbObjHookObjReset(lbl_1_bss_5C, "starhook2");
    mbObjHookObjReset(lbl_1_bss_5C, "starhook3");
    mbObjHookObjReset(lbl_1_bss_5C, "starhook4");
    mbObjHookObjReset(lbl_1_bss_5C, "starhook5");
    mbObjHookObjReset(lbl_1_bss_5C, "starhook6");
    hookIndex = fn_1_15DE4(lbl_1_bss_14);
    if (hookIndex == 5) {
        if (lbl_1_bss_4->configuration == 0) {
            hookIndex = 5;
        } else if (lbl_1_bss_4->configuration == 1) {
            hookIndex = 5;
        } else if (lbl_1_bss_4->configuration == 2) {
            hookIndex = 6;
        } else {
            hookIndex = 6;
        }
    }
    sprintf(hookName, "starhook%d", hookIndex);
    mbObjHookSet(lbl_1_bss_5C, hookName, work->starModel);
    mbObjDispSet(work->starModel, TRUE);
}

/* Restores the old space to blue, wraps the star order, and attaches the next star model. */
void fn_1_15794(void) {
    char hookName[32];
    HuVecF previousPosition;
    s16 spaceIds[8];
    int spaceCount;
    s32 hookIndex;
    StarDisplayWork *work;

    /* The previous star position is fetched but is not used to select or place the next star. */
    mbMasuPosGet(lbl_1_bss_14, &previousPosition);
    mbMasuTypeSet(lbl_1_bss_14, 1);
    if (++lbl_1_bss_4->position >= lbl_1_bss_4->spaceCount) {
        lbl_1_bss_4->position = 0;
    }
    /* The space-ID list selects the next star; the returned count is stored but never read. */
    spaceCount = mbMasuMAttrListGet(W02_STAR_SPACE_GROUP_MASK, spaceIds);
    lbl_1_bss_14 = spaceIds[lbl_1_bss_4->order[lbl_1_bss_4->position]];
    mbMasuTypeSet(lbl_1_bss_14, 7);
    mbStarMasuNextSet(lbl_1_bss_14);
    work = lbl_1_bss_8;
    mbObjHookObjReset(lbl_1_bss_5C, "starhook1");
    mbObjHookObjReset(lbl_1_bss_5C, "starhook2");
    mbObjHookObjReset(lbl_1_bss_5C, "starhook3");
    mbObjHookObjReset(lbl_1_bss_5C, "starhook4");
    mbObjHookObjReset(lbl_1_bss_5C, "starhook5");
    mbObjHookObjReset(lbl_1_bss_5C, "starhook6");
    hookIndex = fn_1_15DE4(lbl_1_bss_14);
    if (hookIndex == 5) {
        if (lbl_1_bss_4->configuration == 0) {
            hookIndex = 5;
        } else if (lbl_1_bss_4->configuration == 1) {
            hookIndex = 5;
        } else if (lbl_1_bss_4->configuration == 2) {
            hookIndex = 6;
        } else {
            hookIndex = 6;
        }
    }
    sprintf(hookName, "starhook%d", hookIndex);
    mbObjHookSet(lbl_1_bss_5C, hookName, work->starModel);
    mbObjDispSet(work->starModel, TRUE);
}

/* Shuffles star spaces, then retries group transitions and avoids the previous opening space. */
void fn_1_159FC(s32 previousSpace) {
    s32 groups[8];
    s16 spaceIds[8];
    int spaceCount;
    s32 index;
    s32 secondIndex;
    s32 firstIndex;
    u8 savedOrder;
    BOOL changed;
    s32 pass;
    s32 firstOrder;
    s32 secondOrder;

    /* The returned count is unused; the shuffle uses the count already stored in the star order. */
    spaceCount = mbMasuMAttrListGet(W02_STAR_SPACE_GROUP_MASK, spaceIds);
    for (index = 0; index < lbl_1_bss_4->spaceCount; index++) {
        groups[index] = fn_1_15DE4(spaceIds[index]);
    }
    for (index = 0; index < 100; index++) {
        firstIndex = mbRandMod(lbl_1_bss_4->spaceCount);
        secondIndex = mbRandMod(lbl_1_bss_4->spaceCount);
        savedOrder = lbl_1_bss_4->order[firstIndex];
        lbl_1_bss_4->order[firstIndex] = lbl_1_bss_4->order[secondIndex];
        lbl_1_bss_4->order[secondIndex] = savedOrder;
    }
    for (pass = 0; pass < 100; pass++) {
        changed = FALSE;
        for (index = 0; index < lbl_1_bss_4->spaceCount; index++) {
            firstIndex = index % lbl_1_bss_4->spaceCount;
            secondIndex = mbRandMod(lbl_1_bss_4->spaceCount);
            savedOrder = lbl_1_bss_4->order[firstIndex];
            lbl_1_bss_4->order[firstIndex] = lbl_1_bss_4->order[secondIndex];
            lbl_1_bss_4->order[secondIndex] = savedOrder;
        }
        for (index = 0; index < lbl_1_bss_4->spaceCount; index++) {
            firstIndex = index;
            secondIndex = index + 1;
            if (secondIndex >= lbl_1_bss_4->spaceCount) {
                secondIndex = 0;
            }
            firstOrder = lbl_1_bss_4->order[firstIndex];
            secondOrder = lbl_1_bss_4->order[secondIndex];
            if (groups[firstOrder] == 1 && groups[secondOrder] == 2) {
                changed = TRUE;
            }
            if (groups[firstOrder] == 2 && groups[secondOrder] == 3) {
                changed = TRUE;
            }
            if (groups[firstOrder] == 3 && groups[secondOrder] == 4) {
                changed = TRUE;
            }
            /* Once set, the flag also swaps every later pair in this pass. */
            if (changed) {
                savedOrder = lbl_1_bss_4->order[firstIndex];
                lbl_1_bss_4->order[firstIndex] = lbl_1_bss_4->order[secondIndex];
                lbl_1_bss_4->order[secondIndex] = savedOrder;
            }
        }
        if (previousSpace == spaceIds[lbl_1_bss_4->order[0]]) {
            firstIndex = 0;
            secondIndex = mbRandMod(lbl_1_bss_4->spaceCount - 1) + 1;
            savedOrder = lbl_1_bss_4->order[firstIndex];
            lbl_1_bss_4->order[firstIndex] = lbl_1_bss_4->order[secondIndex];
            lbl_1_bss_4->order[secondIndex] = savedOrder;
            changed = TRUE;
        }
        if (!changed) {
            break;
        }
    }
    /* The final traversal changes only the local index. */
    for (index = 0; index < lbl_1_bss_4->spaceCount; index++) {
    }
}

/* extern */

/* Returns the hook group for a star space, or -1 when it has no group; star setup calls it. */
s32 fn_1_15DE4(s32 spaceId) {
    s32 unused;
    u32 groupAttributes;
    groupAttributes = mbMasuMAttrGet((s16) spaceId) & W02_STAR_SPACE_GROUP_MASK;
    if (groupAttributes == 0)
    {
        return - 1;
    }
    return (groupAttributes >> W02_STAR_SPACE_GROUP_SHIFT) & 7;
}

/* Creates an initially inactive quad sprite pool and its indexed display list. */
s16 fn_1_15E30(ANIMDATA *animation, s16 capacity) {
    HU3D_MODELID modelId = Hu3DHookFuncCreate(fn_1_16300);
    HuVecF *vertex;
    EventSpriteHook *hook;
    EventSpriteParticle *particle;
    s16 index;
    HuVec2f *coordinate;
    HU3D_MODEL *model;
    u8 *buffer;
    u8 *displayList;

    Hu3DModelCameraSet(modelId, HU3D_CAM0);
    model = &Hu3DData[modelId];
    hook = SpriteAllocate(sizeof(EventSpriteHook), model->mallocNo);
    model->hookData = hook;
    hook->animation = animation;
    hook->particleCount = capacity;
    hook->blendMode = 0;
    hook->flags = 0;
    hook->callbackAddress = 0;
    hook->linkedModelAddress = 0;
    hook->frameCounter = 0;
    hook->animationData = 0;
    hook->hookFlags[0] = 0;
    hook->duration = 0;
    hook->textureFrame = hook->textureIndex = 0;
    hook->tileCoordinates = NULL;
    hook->corners = NULL;
    particle = SpriteAllocate(sizeof(EventSpriteParticle) * capacity, model->mallocNo);
    hook->particles = particle;
    memset(particle, 0, sizeof(EventSpriteParticle) * capacity);
    for (index = 0; index < capacity; index++, particle++) {
        particle->activityValue = 0.0f;
        particle->rendering.x = particle->rendering.y = particle->rotation = 0.0f;
        particle->animationTime = 0.0f;
        particle->animationSpeed = 1.0f;
        particle->position.x = 0.0f;
        particle->position.y = 0.0f;
        particle->position.z = 0.0f;
        particle->color.r = particle->color.g = particle->color.b = particle->color.a = 255;
        particle->timer = 0;
    }
    hook->positions = vertex = SpriteAllocate((sizeof(HuVecF) * capacity) * 4, model->mallocNo);
    for (index = 0; index < capacity * 4; index++, vertex++) {
        vertex->x = vertex->y = vertex->z = 0.0f;
    }
    coordinate = SpriteAllocate(sizeof(float) * (capacity * 8), model->mallocNo);
    hook->textureCoordinates = coordinate;
    for (index = 0; index < capacity; index++) {
        coordinate->x = 0.0f;
        coordinate->y = 0.0f;
        coordinate++;
        coordinate->x = 1.0f;
        coordinate->y = 0.0f;
        coordinate++;
        coordinate->x = 1.0f;
        coordinate->y = 1.0f;
        coordinate++;
        coordinate->x = 0.0f;
        coordinate->y = 1.0f;
        coordinate++;
    }
    displayList = buffer = SpriteAllocate(65536, model->mallocNo);
    DCFlushRange(buffer, 65536);
    GXBeginDisplayList(displayList, 65536);
    GXBegin(GX_QUADS, GX_VTXFMT0, capacity * 4);
    for (index = 0; index < capacity; index++) {
        GXPosition1x16(index * 4);
        GXColor1x16(index);
        GXTexCoord1x16(index * 4);
        GXPosition1x16(index * 4 + 1);
        GXColor1x16(index);
        GXTexCoord1x16(index * 4 + 1);
        GXPosition1x16(index * 4 + 2);
        GXColor1x16(index);
        GXTexCoord1x16(index * 4 + 2);
        GXPosition1x16(index * 4 + 3);
        GXColor1x16(index);
        GXTexCoord1x16(index * 4 + 3);
    }
    hook->displayListSize = GXEndDisplayList();
    hook->displayList = SpriteAllocate(hook->displayListSize, model->mallocNo);
    memcpy(hook->displayList, buffer, hook->displayListSize);
    DCFlushRange(hook->displayList, hook->displayListSize);
    HuMemDirectFree(buffer);
    return modelId;
}
HuVecF lbl_1_data_D44[4] = {
    { -0.5f, 0.5f, 0.0f }, { 0.5f, 0.5f, 0.0f }, { 0.5f, -0.5f, 0.0f }, { -0.5f, -0.5f, 0.0f }
};

HuVec2f lbl_1_data_D74[4] = { { 0.0f, 0.0f }, { 0.25f, 0.0f }, { 0.25f, 0.25f }, { 0.0f, 0.25f } };

/* Builds quads for the first normal draw of a frame and every shadow draw, including optional
 * linked models. */
void fn_1_16300(HU3D_MODEL *model, Mtx *matrix) {
    Mtx inverse;
    Mtx transform;
    Mtx rotation;
    HuVecF scaled[4];
    HuVecF rotated[4];
    HuVecF cached[4];
    ROMtx reordered;
    HuVecF *corners;
    SpriteRenderHook *hook;
    spriterenderSpriteParticle *particle;
    HuVecF *vertex;
    HuVec2f *texture;
    HuVecF *cachedPtr;
    HuVecF *scaledPtr;
    s16 particleIndex;
    s16 cornerIndex;
    s16 bitmapFormat;
    s16 tileX;
    s16 tileY;
    void (*callback)(HU3D_MODEL *, SpriteRenderHook *, Mtx *);

    hook = model->hookData;
    if (hook->lastDraw != GlobalCounter || shadowModelDrawF) {
        if (hook->linkedModel && hook->linkedModel != model) {
            fn_1_16300(hook->linkedModel, matrix);
        }
        GXLoadPosMtxImm(*matrix, GX_PNMTX0);
        GXSetNumTevStages(1);
        GXSetNumTexGens(1);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, FALSE,
            GX_PTIDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        if (shadowModelDrawF) {
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ONE, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
            GXSetZMode(FALSE, GX_LEQUAL, FALSE);
        } else {
            bitmapFormat = hook->animation->bmp->dataFmt & ANIM_BMP_FMTMASK;
            if (bitmapFormat == ANIM_BMP_I8 || bitmapFormat == ANIM_BMP_I4) {
                GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ONE, GX_CC_RASC, GX_CC_ZERO);
            } else {
                GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
            }
            if (hook->flags & 1) {
                GXSetZMode(FALSE, GX_LEQUAL, FALSE);
            } else if (model->attr & HU3D_ATTR_ZWRITE_OFF) {
                /* This renderer enables depth writes when the model carries this flag. */
                GXSetZMode(TRUE, GX_LEQUAL, TRUE);
            } else {
                GXSetZMode(TRUE, GX_LEQUAL, FALSE);
            }
        }
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, TRUE, GX_TEVPREV);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL,
            GX_DF_CLAMP, GX_AF_NONE);
        HuSprTexLoad(hook->animation, 0, GX_TEXMAP0, GX_REPEAT, GX_REPEAT, GX_LINEAR);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetZCompLoc(FALSE);
        switch (hook->blendMode) {
        case 0:
            GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
            break;
        case 1:
            GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
            break;
        case 2:
            GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_INVSRCALPHA, GX_LO_NOOP);
            break;
        }
        if (hook->flags & 0x20) {
            GXSetCullMode(GX_CULL_NONE);
        } else {
            GXSetCullMode(GX_CULL_BACK);
        }
        if (!HmfInverseMtxF3X3(*matrix, inverse)) {
            PSMTXIdentity(inverse);
        }
        PSMTXReorder(inverse, reordered);
        if (hook->update) {
            callback = hook->update;
            callback(model, hook, matrix);
        }
        if (hook->corners == NULL) {
            corners = lbl_1_data_D44;
        } else {
            corners = hook->corners;
        }
        particle = hook->particles;
        vertex = hook->positions;
        texture = hook->textureCoordinates;
        if (hook->flags & 4) {
            PSMTXIdentity(inverse);
            PSMTXIdentity((float (*)[4]) reordered);
            cached[0] = corners[0];
            cached[1] = corners[1];
            cached[2] = corners[2];
            cached[3] = corners[3];
        } else {
            /* This branch interprets the corner pointer and adjacent local storage as four input
             * vectors. */
            PSMTXROMultVecArray(reordered, (HuVecF *) &corners, cached, 4);
        }
        for (particleIndex = 0; particleIndex < hook->particleCount; particleIndex++, particle++) {
            if (!particle->size) {
                vertex->x = vertex->y = vertex->z = 0.0f;
                vertex++;
                vertex->x = vertex->y = vertex->z = 0.0f;
                vertex++;
                vertex->x = vertex->y = vertex->z = 0.0f;
                vertex++;
                vertex->x = vertex->y = vertex->z = 0.0f;
                vertex++;
            } else if (hook->flags & 8) {
                PSVECScale(&corners[0], &scaled[0], particle->size);
                PSVECScale(&corners[1], &scaled[1], particle->size);
                PSVECScale(&corners[2], &scaled[2], particle->size);
                PSVECScale(&corners[3], &scaled[3], particle->size);
                mtxRot(transform, particle->rotation.x, particle->rotation.y, particle->rotation.z);
                PSMTXMultVecArray(transform, scaled, rotated, 4);
                PSVECAdd(&rotated[0], &particle->position, vertex++);
                PSVECAdd(&rotated[1], &particle->position, vertex++);
                PSVECAdd(&rotated[2], &particle->position, vertex++);
                PSVECAdd(&rotated[3], &particle->position, vertex++);
            } else if (!particle->rotation.z) {
                scaledPtr = scaled;
                cachedPtr = cached;
                PSVECScale(cachedPtr++, scaledPtr, particle->size);
                PSVECAdd(scaledPtr++, &particle->position, vertex++);
                PSVECScale(cachedPtr++, scaledPtr, particle->size);
                PSVECAdd(scaledPtr++, &particle->position, vertex++);
                PSVECScale(cachedPtr++, scaledPtr, particle->size);
                PSVECAdd(scaledPtr++, &particle->position, vertex++);
                PSVECScale(cachedPtr++, scaledPtr, particle->size);
                PSVECAdd(scaledPtr++, &particle->position, vertex++);
            } else {
                PSVECScale(&corners[0], &scaled[0], particle->size);
                PSVECScale(&corners[1], &scaled[1], particle->size);
                PSVECScale(&corners[2], &scaled[2], particle->size);
                PSVECScale(&corners[3], &scaled[3], particle->size);
                /* The stored angle is wrapped at 360, but this call consumes it directly as
                 * radians. */
                PSMTXRotRad(rotation, 'Z', particle->rotation.z);
                PSMTXConcat(inverse, rotation, transform);
                PSMTXMultVecArray(transform, scaled, rotated, 4);
                PSVECAdd(&rotated[0], &particle->position, vertex++);
                PSVECAdd(&rotated[1], &particle->position, vertex++);
                PSVECAdd(&rotated[2], &particle->position, vertex++);
                PSVECAdd(&rotated[3], &particle->position, vertex++);
            }
        }
        particle = hook->particles;
        texture = hook->textureCoordinates;
        if (!(hook->flags & 2)) {
            if (hook->tileCoordinates == NULL) {
                for (particleIndex = 0; particleIndex < hook->particleCount;
                     particleIndex++, particle++) {
                    tileX = particle->textureFrame & 3;
                    tileY = (particle->textureFrame >> 2) & 3;
                    for (cornerIndex = 0; cornerIndex < 4; cornerIndex++, texture++) {
                        texture->x = 0.25f * (float) tileX +
                            lbl_1_data_D74[cornerIndex].x;
                        texture->y = 0.25f * (float) tileY +
                            lbl_1_data_D74[cornerIndex].y;
                    }
                }
            } else {
                for (particleIndex = 0; particleIndex < hook->particleCount;
                     particleIndex++, particle++) {
                    for (cornerIndex = 0; cornerIndex < 4; cornerIndex++, texture++) {
                        texture->x =
                            hook->tileCoordinates[cornerIndex + particle->textureFrame * 4].x;
                        texture->y =
                            hook->tileCoordinates[cornerIndex + particle->textureFrame * 4].y;
                    }
                }
            }
            hook->flags &= ~W02_HOOK_UV_INITIALIZED_FLAG;
        } else if (!(hook->flags & W02_HOOK_UV_INITIALIZED_FLAG)) {
            for (particleIndex = 0; particleIndex < hook->particleCount;
                 particleIndex++, particle++) {
                for (cornerIndex = 0; cornerIndex < 4; cornerIndex++, texture++) {
                    texture->x = 4.0f * lbl_1_data_D74[cornerIndex].x;
                    texture->y = 4.0f * lbl_1_data_D74[cornerIndex].y;
                }
            }
            hook->flags |= W02_HOOK_UV_INITIALIZED_FLAG;
        }
        DCFlushRangeNoSync(hook->positions, hook->particleCount * sizeof(HuVecF) * 4);
        DCFlushRangeNoSync(hook->textureCoordinates, hook->particleCount * sizeof(HuVec2f) * 4);
        DCFlushRangeNoSync(hook->particles,
                           hook->particleCount * sizeof(spriterenderSpriteParticle));
        PPCSync();
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXSetArray(GX_VA_POS, hook->positions, sizeof(HuVecF));
        GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GXSetArray(GX_VA_CLR0, &hook->particles->color, sizeof(spriterenderSpriteParticle));
        GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GXSetArray(GX_VA_TEX0, hook->textureCoordinates, sizeof(HuVec2f));
        GXCallDisplayList(hook->displayList, hook->displayListSize);
        if (!shadowModelDrawF) {
            if (!(hook->animationFlags & 2)) {
                hook->frame++;
            }
            if (hook->duration && hook->duration <= hook->frame) {
                if (hook->animationFlags & 1) {
                    hook->frame = 0;
                }
                /* The duration assignment follows even when the frame was just cleared. */
                hook->frame = hook->duration;
            }
            hook->lastDraw = GlobalCounter;
        }
    }
}

/* Sets the quad corners and optional tile coordinates used by this sprite model. */
void fn_1_16E44(s16 modelId, void *corners, void *tileCoordinates) {
    HU3D_MODEL *model = &Hu3DData[modelId];
    SpriteRenderHook *hookData = model->hookData;
    hookData->corners = corners;
    hookData->tileCoordinates = tileCoordinates;
}

/* Builds four UV corners per sprite tile, replacing dimensions below one with one.
 * The alternate mode swaps grid indices. */
void fn_1_16E84(s16 modelId, s32 columns, s32 rows, s32 alternate) {
    HuVec2f *coordinate;
    s32 row;
    s32 column;
    HU3D_MODEL *model;
    TextureGridHook *hook;
    s32 tileCount;
    float width;
    float height;

    if (columns < 1) {
        columns = 1;
    }
    if (rows < 1) {
        rows = 1;
    }
    tileCount = columns * rows;
    width = 1.0f / (float) columns;
    height = 1.0f / (float) rows;
    model = &Hu3DData[modelId];
    hook = model->hookData;
    coordinate = HuMemDirectMallocNum(HEAP_MODEL, sizeof(float) * (tileCount * 8),
                                      model->mallocNo);
    hook->tileCoordinates = coordinate;
    memset(coordinate, 0, sizeof(float) * (tileCount * 8));
    for (row = 0; row < rows; row++) {
        for (column = 0; column < columns; column++) {
            if (alternate) {
                coordinate->x = (float) row * width;
                coordinate->y = (float) column * height;
                coordinate++;
                coordinate->x = (float) (row + 1) * width;
                coordinate->y = (float) column * height;
                coordinate++;
                coordinate->x = (float) (row + 1) * width;
                coordinate->y = (float) (column + 1) * height;
                coordinate++;
                coordinate->x = (float) row * width;
                coordinate->y = (float) (column + 1) * height;
                coordinate++;
            } else {
                coordinate->x = (float) column * width;
                coordinate->y = (float) row * height;
                coordinate++;
                coordinate->x = (float) (column + 1) * width;
                coordinate->y = (float) row * height;
                coordinate++;
                coordinate->x = (float) (column + 1) * width;
                coordinate->y = (float) (row + 1) * height;
                coordinate++;
                coordinate->x = (float) column * width;
                coordinate->y = (float) (row + 1) * height;
                coordinate++;
            }
        }
    }
}

/* Releases a sprite model's animation before removing the model itself. */
void fn_1_172E8(s16 modelId) {
    SpriteRenderHook *hookData;
    HU3D_MODEL *model;

    model = &Hu3DData[modelId];
    hookData = model->hookData;
    if (hookData->animation != NULL) {
        HuSprAnimKill(hookData->animation);
    }
    Hu3DModelKill(modelId);
}

/* Creates the layer-five sprite object with a four-by-four animation grid. */
void fn_1_1735C(void) {
    s32 modelId[1];
    OMOBJ *object;
    EventSpriteObject *work;
    spriteinitializeSpriteHook *hook;
    HU3D_MODEL *model;
    ANIMDATA *animation;
    void *data;

    object = lbl_1_bss_C = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_17474);
    data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(EventSpriteObject), HU_MEMNUM_OVL);
    object->data = data;
    work = data;
    memset(work, 0, sizeof(EventSpriteObject));
    animation = HuSprAnimRead(HuDataReadNum(DATANUM(DATA_capsule, 40), HU_MEMNUM_OVL));
    work->animation = animation;
    work->modelId = modelId[0] = fn_1_15E30(animation, 512);
    fn_1_16E84(work->modelId, 4, 4, 0);
    Hu3DModelLayerSet(work->modelId, 5);
    work->state = 0;
    model = &Hu3DData[work->modelId];
    hook = model->hookData;
    hook->blendMode = 0;
}

/* Advances live particles each frame and releases the sprite model when the board exits. */
void fn_1_17474(OMOBJ *object) {
    spriteinitializeSpriteParticle *particle;
    EventSpriteObject *work;
    spriteinitializeSpriteHook *hook;
    s32 index;
    HU3D_MODEL *model;

    work = object->data;
    if (mbExitCheck() || lbl_1_bss_C == NULL) {
        Hu3DModelKill(work->modelId);
        work->modelId = -1;
        HuSprAnimKill(work->animation);
        work->animation = NULL;
        lbl_1_bss_C = NULL;
        omDelObjEx(mbObjMan, object);
        return;
    }
    if (work->state <= 0) {
        Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF);
        return;
    }
    Hu3DModelAttrReset(work->modelId, HU3D_ATTR_DISPOFF);
    model = &Hu3DData[work->modelId];
    hook = model->hookData;
    particle = hook->particles;
    hook->updateFlags = 0;
    for (index = 0; index < hook->particleCount; index++, particle++) {
        if (particle->activityValue <= 0.0f) {
            continue;
        }
        particle->position.x += particle->velocity.x;
        particle->position.y += particle->velocity.y;
        particle->position.z += particle->velocity.z;
        particle->rotation += particle->rotationSpeed;
        if (particle->rotation >= 360.0f) {
            particle->rotation -= 360.0f;
        }
        particle->animationFrame = particle->animationTime;
        particle->animationTime += particle->animationSpeed;
        if (particle->animationTime >= 16.0f) {
            particle->animationFrame = 0;
            particle->frame = 0;
            particle->activityValue = 0.0f;
            work->state--;
        }
    }
}

/* Clears the shared sprite object's reference so its updater releases the model on its next
 * update. */
void fn_1_17688(void) {
    lbl_1_bss_C = 0;
}

/* Returns the number of active particles in the board's shared sprite object. */
s32 fn_1_1769C(void)
{
    EventSpriteObject *objectData;

    if (lbl_1_bss_C == (void *)0) {
        return 0;
    }
    objectData = lbl_1_bss_C->data;
    return (s32)objectData->state;
}

/* Starts one inactive particle when a board effect emits a sprite. */
s32 fn_1_176E4(HuVecF *position, HuVecF *velocity, float activityValue,
    float rotationSpeed, float animationSpeed, GXColor *color) {
    EventSpriteParticle *particle;
    EventSpriteHook *hook;
    s32 index;
    EventSpriteObject *work;
    OMOBJ *object;
    HU3D_MODEL *model;

    object = lbl_1_bss_C;
    work = object->data;
    model = &Hu3DData[work->modelId];
    hook = model->hookData;
    particle = hook->particles;
    for (index = 0; index < hook->particleCount; index++, particle++) {
        if (particle->activityValue <= 0.0f) {
            break;
        }
    }
    if (index >= hook->particleCount) {
        return -1;
    }
    particle->frame = particle->state = 0;
    particle->position.x = position->x;
    particle->position.y = position->y;
    particle->position.z = position->z;
    particle->velocity.x = velocity->x;
    particle->velocity.y = velocity->y;
    particle->velocity.z = velocity->z;
    particle->rotationSpeed = rotationSpeed;
    particle->activityValue = activityValue;
    particle->color = *color;
    particle->rotation = 360.0f * (W02_RANDOM_FRACTION_SCALE * mbRandMod(W02_RANDOM_MODULUS));
    particle->timer = 0;
    particle->frame = 0;
    particle->animationTime = 0.0f;
    particle->animationSpeed = animationSpeed;
    work->state++;
    return index;
}

/* Emits a ring of particles after rotating their outward velocities. */
void fn_1_17894(HuVecF *origin, HuVecF *angles, s32 count) {
    Mtx matrix;
    HuVecF position;
    HuVecF velocity;
    HuVecF particlePosition;
    HuVecF particleVelocity;
    GXColor color;
    GXColor particleColor;
    s32 particleIndex;
    s32 angle;
    s32 brightness;
    float rotationSpeed;
    float sine;
    float cosine;

    mtxRot(matrix, angles->x, angles->y, angles->z);
    for (particleIndex = 0; particleIndex < count; particleIndex++) {
        position.x = origin->x;
        position.y = origin->y;
        position.z = origin->z;
        angle = (360.0f / (float) count) * (float) particleIndex;
        sine = mbSinDeg(angle);
        velocity.x = 0.075f * (100.0f * sine) * (0.9f + 0.1f *
            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
        velocity.y = 0.0f;
        cosine = mbCosDeg(angle);
        velocity.z = 0.075f * (100.0f * cosine) * (0.9f + 0.1f *
            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS)));
        PSMTXMultVec(matrix, &velocity, &velocity);
        brightness = 63.0f * (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
        color.r = (u8) (192 + brightness);
        color.g = (u8) (192 + brightness);
        color.b = (u8) (192 + brightness);
        color.a = 192.0f + 63.0f *
            (W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
        particleColor = color;
        rotationSpeed = 10.0f * (-0.5f +
            W02_RANDOM_FRACTION_SCALE * (float) mbRandMod(W02_RANDOM_MODULUS));
        particleVelocity = velocity;
        particlePosition = position;
        EmitSpriteParticle(&particlePosition, &particleVelocity, 200.0f,
            rotationSpeed, 0.33f, &particleColor);
    }
}
