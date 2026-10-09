// Snowflake Lake scenery, Chain Chomp rides, and coin-collecting board events.
#define W04_LINK_PRIORITY_MASK 0x40
#define W04_RIDE_REGION_MASK 0x380
#define W04_SCENERY_TRIGGER_MASK 0x400
#define W04_MOLE_TRIGGER_MASK 0x3800
#define W04_PENGUIN_REGION_MASK 0x18000
#define W04_NIGHT_LINK_MASK 0x10
#define W04_NIGHT_BRANCH_MASK 0x20
#define W04_ROLLING_ROUTE_MASK 0x4
#define W04_EVENT_LINK_MASK 0x2000
#define W04_RIDE_CAMERA_KEEP_MASK 0x20000
#define W04_COLLISION_PASS_LIMIT 50
#define W04_DAY_BYTE_OFFSET 16
#define W04_PROJECTILE_BYTES 48
#define W04_COIN_BYTES 60
#define W04_ANIMATED_SPACE_BYTES 12
#define W04_SNOWBALL_HIT_SOUND 1497
#define W04_PLAYER_COLLISION_SOUND 1496
#define W04_RIDE_END_SOUND 1498
#define W04_MOLE_SOUND 1499
#define W04_ATTACHMENT_LAUNCH_SOUND 1500
#define W04_RIDE_ATTACK_SOUND 1495
#define W04_ROLLING_START_SOUND 1492
#define W04_ROLLING_SOUND 1493
#include "math.h"
#include "game/board/main.h"
#include "game/board/object.h"
#include "game/board/camera.h"
#include "game/board/audio.h"
#include "game/process.h"
#include "REL/w01Dll_world01.h"
#include "humath.h"
#include "game/hu3d.h"
#include "game/board/coin.h"
#include "game/board/player.h"
#include "game/frand.h"
#include "game/pad.h"
#include "game/board/masu.h"
#include "game/board/window.h"
#include "messdir_enum.h"
#include "datadir_enum.h"
#include "game/board/branch.h"
#include "game/board/opening.h"
#include "game/audio.h"
#include "game/data.h"
#include "string.h"
#include "game/object.h"
#include "game/board/status.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/sprite.h"
#include "dolphin/gx.h"
#include "dolphin/os.h"
#include "game/memory.h"
#include "game/gamework.h"
#include "game/board/effect.h"
#include "game/board/comchoice.h"
#include "msm_se.h"
#include "game/wipe.h"
#include "game/main.h"
#include "game/hsfex.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/mg/actman.h"
#include "game/board/tutorial.h"
#include "dolphin/types.h"
#include <dolphin/types.h>

typedef struct W04PairedBoardObject {
    MBMODELID mainModelId; // Animated model that releases the attached model.
    MBMODELID movingModelId; // Model carried by the y_h1 hook before its flight.
    u32 unusedStorage; // Not read or written by this board's object events.
    float launchDegrees; // Y rotation and horizontal flight direction, in degrees.
} W04PairedBoardObject;

typedef struct W04GridCell {
    int distance; // One-based route length; zero means unreached.
    float cost; // Route penalty; negative cells cannot be entered.
} W04GridCell;

typedef struct W04GridPoint {
    int x; // Column in the 12 by 12 movement grid.
    int y; // Row in the 12 by 12 movement grid, corresponding to world Z.
} W04GridPoint;

typedef struct W04CoinPlayer {
    BOOL knockedBack; // Set until the collision reaction and protection period end.
    HuVecF position; // Player position in world units.
    HuVecF velocity; // Movement velocity in world units per second.
    s16 state; // Move, knockback, stand-up wait, or stand-up motion (0 through 3).
    s16 unusedStorage; // Not accessed by the falling-coin event.
    float facingDegrees; // Player's Y rotation in degrees.
    s16 elapsedFrames; // Time spent in the current stand-up wait.
    s16 recoveryFrames; // Length of the stand-up wait in frames.
    s16 protectionFrames; // Remaining post-collision frames; also controls flashing.
    s16 motions[3]; // Knockback, standing up, and skating motions.
    s16 attachmentState; // Not accessed by the falling-coin event.
    s16 eventModels[2]; // Models attached to the player's item hooks during the event.
    u8 eventState[2]; // Not accessed by the falling-coin event.
    W04GridPoint cell; // Current movement-grid location used by computer players.
    int targetPlayerNo; // Not accessed by the falling-coin event.
    HuVecF targetOffset; // Not accessed by the falling-coin event.
    int targetFrames; // Not accessed by the falling-coin event.
} W04CoinPlayer;

typedef struct W04CoinState {
    BOOL active; // Whether the coin can be updated or collected.
    HuVecF position; // Coin position in world units.
    HuVecF velocity; // Coin velocity in world units per second.
    HuVecF rotation; // Coin rotation in degrees.
    HuVecF rotationVelocity; // Rotation added on each event frame, in degrees.
    s16 coinObjectId; // Coin renderer's object ID.
    s16 groundMarkerId; // Board model drawn below the coin.
    s16 expirationFrames; // -1 until landing, then remaining lifetime; also controls flashing.
} W04CoinState;

typedef struct W04EventPlayer {
    HuVecF position; // Player position in world units.
    HuVecF velocity; // Movement velocity in world units per second.
    s16 state; // Move, throw, knockback, stand-up wait, or stand-up motion (0 through 4).
    s16 unusedStorage; // Not accessed by the snowball event.
    float facingDegrees; // Player's Y rotation in degrees.
    s16 elapsedFrames; // Time spent throwing or waiting to recover.
    s16 recoveryFrames; // Frame at which throwing or the stand-up wait ends.
    s16 hitProtection; // Remaining protected frames; also controls flashing.
    s16 motions[3]; // Knockback, standing up, and throwing motions.
    int eventState; // Not accessed by the snowball event.
    W04GridPoint cell; // Current movement-grid location used by computer players.
    int targetPlayerNo; // Opponent chosen by a computer player, or -1.
    HuVecF targetOffset; // World-space offset added to the selected opponent's position.
    int targetFrames; // Frames before a computer player chooses another destination.
} W04EventPlayer;

typedef struct W04BoardModels {
    MBMODELID models[65]; // Board model in slot zero, followed by other board models.
    MBMODELID shadowModelId; // Board model added to the shadow map during board event scenes.
    GXColor lightColor; // Saved global-light color restored at the end of a turn.
} W04BoardModels;

typedef struct W04BoardWork {
    u32 unusedStorage; // Cleared on a new board; otherwise not accessed here.
    u8 linkedSpaceMessageShown : 1; // Night-route introduction has already been shown.
} W04BoardWork;

typedef struct W04Projectile {
    BOOL active; // Whether this projectile slot is in use.
    int phase; // Zero while flying; one while its impact motion plays.
    MBMODELID movingModelId; // Model displayed during flight.
    MBMODELID impactModelId; // Model displayed after landing or hitting a player.
    HuVecF position; // Projectile position in world units.
    HuVecF velocity; // Horizontal velocity in world units per second.
    int ownerPlayerNo; // Throwing player, excluded from the collision check.
    float startHeight; // Height from which the flight arc starts, in world units.
    s16 elapsedFrames; // Frames since the projectile was thrown.
    s16 travelFrames; // Number of frames in the flight arc.
} W04Projectile;

typedef struct W04FallingParticle {
    HuVecF position; // World-space position, wrapped around the board camera.
    HuVecF velocity; // Fall and drift velocity in world units per second.
    float directionDegrees; // Heading used to compute horizontal drifting motion.
    float directionSine; // Sine of the drift heading.
    float directionCosine; // Cosine of the drift heading.
    float rotationRadians; // Current phase of the particle's oscillation.
    float motionFactor; // Angular velocity of the drift oscillation, in radians per second.
    float scale; // Multiplier applied to the particle quad's X and Y dimensions.
} W04FallingParticle;

typedef struct W04SpaceModels {
    u8 jumping : 1; // Current route segment requires a jump or climb.
    u8 landed : 1; // Requests the ride model's particle effect.
    u8 attacking : 1; // A star-stealing encounter is in progress.
    u8 state : 5; // Not accessed by the ride event.
    u8 unusedStorage; // Not accessed by the ride event.
    MBMODELID actorModelId; // Mount model used for this linked-space ride.
    MBMODELID baseModelId; // Stationary model at the linked destination.
    MBMODELID effectModelId; // Additional model displayed for the ride event.
    MBMODELID capsuleModelId; // Model displayed when a player repels the mount with a capsule.
    s16 eventSpaceId; // Space that offers the ride.
    s16 linkedSpaceId; // Linked boarding and destination space.
    s16 motion; // Not accessed by the ride event.
    float directionDegrees; // Mount's travel heading in degrees.
    int modelIndex; // Index assigned when the linked-space models are created.
    int playerNo; // Player currently taking this ride.
    int starCount; // Number of stars obtained during the ride.
    int soundId; // Playing ride sound's handle, stopped when travel ends.
    OMOBJ *diceNumberObject; // Die result displayed above the mount.
} W04SpaceModels;

typedef struct W04MovingModels {
    MBMODELID primaryModelId; // Rolling model that carries players along the board path.
    MBMODELID secondaryModelId; // Model used for the event's ending animation.
    HuVecF position; // Rolling model's current world position.
    int state; // Rolling-event phase shared with the carried-player process.
} W04MovingModels;

typedef struct W04RollingPlayer {
    int state; // Carried, launched, or finished player movement phase.
    BOOL active; // Whether this player was on the rolling route.
    HuVecF position; // Player position during carriage and launch.
    Mtx rotation; // Rotation applied while the player is carried by the rolling model.
    s16 elapsedFrames; // Frames since the launch started.
    s16 durationFrames; // Duration of the launch arc in frames.
} W04RollingPlayer;

typedef struct W04ProtectedPlayer {
    int playerNo; // Player whose capsule can repel the mount.
    int capsuleIndex; // Capsule slot selected for the encounter.
} W04ProtectedPlayer;

typedef struct W04SpaceObject {
    BOOL running; // Model's one-shot motion is currently playing.
    MBMODELID modelId; // Hidden model shown when its marked space is triggered.
    s16 spaceId; // Space carrying the model's trigger bit.
} W04SpaceObject;

typedef struct W04AnimatedObject {
    BOOL running; // Alternate motion is playing before the idle loop resumes.
    MBMODELID modelId; // Board scenery model.
} W04AnimatedObject;

typedef struct W04AnimatedSpaceObject {
    BOOL running; // Space-triggered reaction is playing before the idle loop resumes.
    MBMODELID modelId; // Hooked penguin model.
    int spaceIndex; // Encoded space group that triggers this penguin.
} W04AnimatedSpaceObject;

typedef struct W04EffectRequest {
    u8 suppressed : 1; // Prevents the usual motion-frame particle burst.
    u8 requested : 1; // Requests a particle burst regardless of the motion frame.
    u8 : 6;
    u8 unusedStorage; // Not accessed by the particle-burst process.
    MBMODELID modelId; // Model whose motion and position control the burst.
} W04EffectRequest;

typedef struct W04EffectActivity {
    s16 active; // Nonzero while the particle ring is running.
} W04EffectActivity;

typedef struct W04TimeChange {
    BOOL skipAllowed; // Human input can end the current time-change presentation.
    BOOL finished; // The time-change sequence has ended.
    ANIMDATA *animation; // Sprite animation used by the time-change particle effect.
    HU3D_MODELID particleModelId; // Particle model active during the presentation.
} W04TimeChange;

typedef struct W04LinkedObject {
    s16 modelId; // Looping model displayed at night.
    s16 linkedSpaceId; // Linked destination associated with this model.
} W04LinkedObject;

void mbWipeSpecialFadeInCreate(int mode, int frames);
void mbWipeSpecialFadeOutCreate(int mode, int frames);
void mbWipeDissolveFadeOut(void);
void mbWipeDissolveFadeIn(void);
void mbMapSprAdd(int type, s16 id);
extern BOOL mbSaveNewF;
void mbev_ShopBackMotCreate(int dataNum, int motionDataNum, int motionNo,
    BOOL link, char *hookName);
void mbScrollHookSet(void (*hook)(BOOL));
void mbCoinAddAllProcExecV(int *coins, BOOL *display, BOOL wait);
s16 mbCoinDispCreate(HuVecF *position, int count, int sign, BOOL sound);
int mbCapObjCreate(int capsuleNo, BOOL link);
void mbev_Scroll(int playerNo, BOOL mapOpen);
int mbDiceProcExec(int playerNo, int diceType, int diceNo, int diceValue,
    BOOL display, BOOL wait, int hook, BOOL npc);
void mbDiceNumKill(int playerNo);
OMOBJ *mbDiceSNpcNumObjCreate(HuVecF *position, HuVecF *offset,
    int number, BOOL screen, BOOL display);
void mbDiceSNpcNumKill(OMOBJ *object);
int sprintf();
void HuSprTexLoad(ANIMDATA *animation, s16 bitmapNo, s16 textureMap,
    GXTexWrapMode wrapS, GXTexWrapMode wrapT, GXTexFilter filter);
void mtxTransCat(Mtx matrix, float x, float y, float z);
s8 mbPadStkXGet(int padNo);
s8 mbPadStkYGet(int padNo);
float mbAngleEaseOut(float angle, float target, float factor);
float mbBezierCalc(float pointA, float pointB, float pointC, float parameter);
void mbDiceSNpcNumOfsGet(OMOBJ *object, HuVecF *offset);
void mbDiceSNpcNumOfsSet(OMOBJ *object, HuVecF *offset);
void mbDiceSNpcNumSet(OMOBJ *object, int number);
void mbDiceSNpcNumDispSet(OMOBJ *object, BOOL display);
int mbStarAddProcExec(int playerNo, int count, BOOL display, BOOL wait);
float mbAngleEaseOut(float angle, float target, float progress);
void mbObjFadeCreate(MBMODELID modelId, HuVecF *pos);
void mbObjFadeKill(MBMODELID modelId);
void mbObjFadeTexColorSet(MBMODELID modelId, u8 r, u8 g, u8 b, float a);
void mbObjFadeTexRotSet(MBMODELID modelId, HuVecF *pos, HuVecF *rot);
void mbDiceSNpcNumPosSet(OMOBJ *object, HuVecF *position);
void mbStarAddAllProcExecV(int *stars, int *work, BOOL wait);
void mbCapEffUseWanWanCreate(int playerNo, int capsuleNo, HuVecF *position);
typedef float (*W04CurveEval)();

float *lbl_1_bss_7310;
OSTick lbl_1_bss_730C;
W04BoardWork *lbl_1_bss_7308;
W04BoardModels lbl_1_bss_7280;
W04TimeChange lbl_1_bss_7270;
void *lbl_1_bss_726C;
int lbl_1_bss_7268;
ANIMDATA *lbl_1_bss_7264;
BOOL lbl_1_bss_7260;
int lbl_1_bss_725C;
HU3D_MODELID lbl_1_bss_724C[8];
W04LinkedObject *lbl_1_bss_7248;
int lbl_1_bss_7244;
W04FallingParticle lbl_1_bss_1244[512];
HU3D_MODELID lbl_1_bss_1240;
void *lbl_1_bss_123C;
u32 lbl_1_bss_1238;
ANIMDATA *lbl_1_bss_1234;
float lbl_1_bss_1230;
BOOL lbl_1_bss_122C;
W04MovingModels lbl_1_bss_1218;
s16 lbl_1_bss_1214;
W04GridCell lbl_1_bss_D94[12][12];
BOOL lbl_1_bss_D90;
int lbl_1_bss_D80[4];
W04EventPlayer lbl_1_bss_C50[4];
W04Projectile lbl_1_bss_950[16];
W04CoinState lbl_1_bss_590[16];
BOOL lbl_1_bss_580[4];
int lbl_1_bss_570[4];
W04CoinPlayer lbl_1_bss_420[4];
W04CoinState lbl_1_bss_60[16];
MBMODELID lbl_1_bss_58[4];
W04SpaceObject lbl_1_bss_50;
W04PairedBoardObject lbl_1_bss_44;
MBMODELID lbl_1_bss_38[6];
W04AnimatedSpaceObject lbl_1_bss_8[4];
W04AnimatedObject lbl_1_bss_0[1];
BOOL mbObjMotionEndCheck(s16 modelId);
void mbObjDispSet(s16 modelId, BOOL dispF);

extern HuVecF lbl_1_data_6A4[4];
extern int lbl_1_data_6D4[];
extern int lbl_1_data_6DC[];
extern HuVecF lbl_1_data_5E8;
extern W04GridPoint lbl_1_data_5A8[8];
extern float lbl_1_data_63C[4];
extern char *lbl_1_data_36C[20];
extern char *lbl_1_data_3F0[5];
extern char *lbl_1_data_438[5];
extern char *lbl_1_data_290[2];
extern char *lbl_1_data_45C[2];
extern HuVecF lbl_1_data_464;
extern HuVecF lbl_1_data_470;
extern HuVecF lbl_1_data_270;
extern HuVecF lbl_1_data_658;
extern HuVecF lbl_1_data_64C;
extern HuVecF lbl_1_data_680;
extern HuVecF lbl_1_data_68C;
extern HuVecF lbl_1_data_698;
extern int lbl_1_data_664[3];
extern BOOL lbl_1_data_670[4];
extern HuVecF lbl_1_data_600;
extern HuVecF lbl_1_data_5F4;
extern int lbl_1_data_60C[3];
extern HuVecF lbl_1_data_618;
extern HuVecF lbl_1_data_624;
extern HuVecF lbl_1_data_630;
extern int lbl_1_data_520[3];
extern int lbl_1_data_52C[3];
extern int lbl_1_data_538[3];
extern int lbl_1_data_544[3];
extern int lbl_1_data_550[3];
extern HuVecF lbl_1_data_F0[32];
extern int lbl_1_data_490[4];
extern HuVecF lbl_1_data_560;
extern int lbl_1_data_7A8[];
extern int lbl_1_data_7B0[];
extern char *lbl_1_data_728[6];
extern int lbl_1_data_778[];
extern int lbl_1_data_788[];
extern char *lbl_1_data_768[4];
extern int lbl_1_data_798[4];
extern HuVecF lbl_1_data_4A0[3][2];
extern float lbl_1_data_4E8[3][2];

void fn_1_A0(void);
void fn_1_F4(void);
void fn_1_9DC(void);
void fn_1_A04(void);
void fn_1_B5C(OMOBJ *object);
int fn_1_BA8(int playerNo, s16 spaceId);
int fn_1_C04(int playerNo, s16 spaceId);
int fn_1_D3C(int playerNo, s16 spaceId);
void fn_1_DF4(int playerNo);
void fn_1_E28(int turnPlayerNo);
void fn_1_EC4(void);
void fn_1_1010(void);
void fn_1_1014(int modelId, int shopNo);
int fn_1_1140(int playerNo, int count, s16 *spaces, BOOL forceMarked);
void fn_1_1338(int scrollOpen);
void fn_1_1388(BOOL mapOpen);
void fn_1_143C(void);
void fn_1_1738(void);
void fn_1_19EC(HU3D_MODEL *model, MBPARTICLE *effect, Mtx matrix);
void fn_1_1D74(void);
void fn_1_215C(void);
int fn_1_21EC(int playerNo, s16 spaceId, int modelIndex);
int fn_1_2748(int playerNo, s16 startSpace);
s16 fn_1_2810(s16 spaceId);
s16 fn_1_28EC(s16 spaceId, int steps);
void fn_1_29F4(W04SpaceModels *models, int moveCount);
void fn_1_3618(W04SpaceModels *models, s16 startSpace, s16 endSpace);
void fn_1_438C(void);
void fn_1_44B8(HU3D_MODEL *model, MBPARTICLE *effect, Mtx matrix);
void fn_1_47BC(void);
void fn_1_495C(void);
void fn_1_4ACC(void);
s16 fn_1_4B28(int index);
void fn_1_4B54(int playerNo, s16 spaceId);
void fn_1_4C48(void);
void fn_1_4EA8(void);
void fn_1_4F6C(void);
void fn_1_53E4(HU3D_MODEL *model, Mtx *matrix);
u32 fn_1_57D8(void **displayList);
BOOL fn_1_5A20(HuVecF *position);
void fn_1_5AF4(float opacity);
void fn_1_5B04(BOOL visible);
void fn_1_5B14(void);
void fn_1_5C64(int playerNo, s16 spaceId);
void fn_1_6064(void);
void fn_1_67D8(void);
void fn_1_6980(W04GridPoint *start);
BOOL fn_1_6BB0(W04GridPoint *goal);
void fn_1_6DC0(W04GridPoint *start, W04GridPoint *goal, W04GridPoint *nextStep);
void fn_1_707C(void);
void fn_1_709C(int playerNo, s16 spaceId);
void fn_1_8004(int playerNo);
void fn_1_89EC(s32 playerNo, HuVecF *direction);
u16 fn_1_8ACC(int playerNo, HuVecF *stick);
void fn_1_91AC(s32 playerNo);
void fn_1_9578(void);
s32 fn_1_95BC(void);
void fn_1_9878(void);
void fn_1_99BC(void);
void fn_1_9C18(s32 projectileIndex);
s32 fn_1_9CA0(s32 playerNo);
void fn_1_9DFC(void);
void fn_1_A170(void);
s32 fn_1_A660(s32 playerNo);
s32 fn_1_A80C(s32 playerNo, s32 count);
void fn_1_A9E0(s32 index);
void fn_1_AA44(void);
void fn_1_AC0C(void);
void fn_1_AC10(int activePlayerNo, s16 eventSpaceId);
void fn_1_B7CC(int playerNo);
void fn_1_BF98(s32 playerNo, HuVecF *direction);
void fn_1_C080(void);
s32 fn_1_C0C4(void);
void fn_1_C52C(void);
void fn_1_C810(void);
void fn_1_C99C(s32 index);
void fn_1_CA00(void);
BOOL fn_1_CB6C(int playerNo, HuVecF *stick);
void fn_1_D020(s32 playerNo);
void fn_1_D3EC(void);
void fn_1_D4F8(void);
void fn_1_D5B0(void);
void fn_1_D618(void);
void fn_1_D674(void);
void fn_1_D7C8(void);
void fn_1_DA4C(void);
void fn_1_DB18(s32 modelIndex);
void fn_1_DB84(void);
void fn_1_DC8C(int spaceIndex);
void fn_1_DD1C(void);
void fn_1_DDB8(void);
void fn_1_DE74(s32 objectIndex);
void fn_1_DEDC(void);
float fn_1_DF78(W04CurveEval speedAtParameter, HuVecF *curvePointA,
    HuVecF *curvePointB, HuVecF *curvePointC, HuVecF *curvePointD,
    float targetParameter);
float fn_1_E120(W04CurveEval speedAtParameter, HuVecF *curvePointA,
    HuVecF *curvePointB, HuVecF *curvePointC, HuVecF *curvePointD,
    float targetParameter);
float fn_1_E330(W04CurveEval speedAtParameter, HuVecF *curvePointA,
    HuVecF *curvePointB, HuVecF *curvePointC, HuVecF *curvePointD,
    float t, float targetDistance, int maxStep);
float fn_1_E64C(W04CurveEval speedAtParameter, HuVecF *curvePointA,
    HuVecF *curvePointB, HuVecF *curvePointC, HuVecF *curvePointD,
    float t, float targetDistance, int maxStep);
f32 fn_1_E900(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC, f32 t);
f32 fn_1_E9A4(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC,
    HuVecF *pointD, f32 t);
f32 fn_1_EA60(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC,
    HuVecF *pointD, f32 t);
f32 fn_1_EB1C(f32 pointA, f32 pointB, f32 pointC, f32 pointD, f32 t);
f32 fn_1_EBA4(f32 pointA, f32 pointB, f32 pointC, f32 pointD, f32 t);
void fn_1_EC7C(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC,
    HuVecF *pointD, HuVecF *position, float t);
void fn_1_EF3C(HuVecF *pointA, HuVecF *pointB, HuVecF *pointC,
    HuVecF *pointD, HuVecF *slope, float t);
void fn_1_F2EC(HuVecF *points, int count, HuVecF *position, float t);
float fn_1_F45C(int basisIndex, int degree, float t);
void fn_1_F9D0(HuVecF *points, int count, HuVecF *outPosition, float t);
void fn_1_FC8C(HuVecF *points, int index, int count, HuVecF *a, HuVecF *b,
    HuVecF *c, HuVecF *d);
void fn_1_FE68(HuVecF *points, s32 index, s32 count, HuVecF *pointA,
    HuVecF *pointB, HuVecF *tangentA, HuVecF *tangentB);
void fn_1_FFB8(HuVecF *startPosition, HuVecF *endPosition,
    HuVecF *position, f32 t);
void fn_1_1001C(int playerNo, HuVecF *dstPos, float dstRotY,
    float jumpHeight, int maxTime);
void fn_1_102BC(void);
OSTick fn_1_102E8(void);

static inline BOOL W04TimeDayGet(void)
{
    return GwSystem.curTime == 0;
}

// Fill the reachable grid distances from a player's current cell before choosing a route.
static inline void W04GridDistancesFill(W04GridPoint *start)
{
    W04GridPoint queue[100];
    W04GridPoint next;
    int row;
    int column;
    s16 readIndex;
    s16 writeIndex;
    int nextDistance;

    for (row = 0; row < 12; row++) {
        for (column = 0; column < 12; column++) {
            lbl_1_bss_D94[row][column].distance = 0;
        }
    }
    queue[0] = *start;
    lbl_1_bss_D94[start->y][start->x].distance = 1;
    readIndex = 0;
    writeIndex = 1;
    do {
        nextDistance = lbl_1_bss_D94[queue[readIndex].y][queue[readIndex].x].distance + 1;
        for (row = 0; row < 8; row++) {
            next.x = queue[readIndex].x + lbl_1_data_5A8[row].x;
            next.y = queue[readIndex].y + lbl_1_data_5A8[row].y;
            if (lbl_1_bss_D94[next.y][next.x].distance == 0 &&
                lbl_1_bss_D94[next.y][next.x].cost >= 0.0f) {
                lbl_1_bss_D94[next.y][next.x].distance = nextDistance;
                queue[writeIndex++] = next;
            }
        }
        readIndex++;
    } while (readIndex != writeIndex);
}

// Start a collision reaction and push the player along the supplied separation direction.
static inline void W04CoinPlayerKnockback(int playerNo, HuVecF *direction)
{
    HuVecF velocity;
    W04CoinPlayer *player = &lbl_1_bss_420[playerNo];

    player->knockedBack = TRUE;
    player->state = 1;
    mbPlayerMotionShiftSet(playerNo, player->motions[0], 0.0f, 8.0f, 0);
    mbPlayerMotionSpeedSet(playerNo, 2.0f);
    PSVECNormalize(direction, &velocity);
    PSVECScale(&velocity, &velocity, 500.0f);
    player->velocity = velocity;
    mbAudFXPlay(W04_PLAYER_COLLISION_SOUND);
    omVibrate(playerNo, 20, 7, 3);
}

// Hide a collected falling coin and its marker when the collection check selects it.
static inline void W04CollectedCoinRemove(s32 index)
{
    W04CoinState *coin = &lbl_1_bss_60[index];

    mbCoinEffCreate(&coin->position);
    mbCoinObjDispSet(coin->coinObjectId, FALSE);
    mbObjDispSet(coin->groundMarkerId, FALSE);
    coin->active = FALSE;
}

// Hide a collected scattered coin and its marker during the projectile event.
static inline void W04ScatteredCoinRemove(s32 index)
{
    W04CoinState *coin = &lbl_1_bss_590[index];

    mbCoinEffCreate(&coin->position);
    mbCoinObjDispSet(coin->coinObjectId, FALSE);
    mbObjDispSet(coin->groundMarkerId, FALSE);
    coin->active = FALSE;
}

// Switch a projectile to its impact model when it lands or strikes a player.
static inline void W04ProjectileImpact(s32 index)
{
    HuVecF position;
    W04Projectile *projectile = &lbl_1_bss_950[index];

    mbObjDispSet(projectile->movingModelId, FALSE);
    mbObjPosGet(projectile->movingModelId, &position);
    mbObjDispSet(projectile->impactModelId, TRUE);
    mbObjPosSetV(projectile->impactModelId, &position);
    mbObjMotionTimeSet(projectile->impactModelId, 0.0f);
    projectile->phase = 1;
}

// Start knockback along the impact direction when a projectile hits an unprotected player.
static inline void W04ProjectilePlayerHit(s32 playerNo, HuVecF *direction)
{
    HuVecF velocity;
    W04EventPlayer *player = &lbl_1_bss_C50[playerNo];

    player->state = 2;
    mbPlayerMotionShiftSet(playerNo, player->motions[0], 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    mbPlayerMotionSpeedSet(playerNo, 2.0f);
    VECNormalize(direction, &velocity);
    VECScale(&velocity, &velocity, 1000.0f);
    player->velocity = velocity;
    mbAudFXPlay(W04_SNOWBALL_HIT_SOUND);
    omVibrate(playerNo, 20, 7, 3);
}

static inline int W04PlayerTeamGet(s32 playerNo)
{
    return GwPlayer[playerNo].team;
}

// Prefer the marked available link, falling back to the first permitted destination.
static inline s16 W04LinkNext(s16 spaceId)
{
    s16 links[MASU_LINK_MAX];
    s16 selectedSpace = -1;
    int linkCount;
    int linkIndex;
    u32 spaceAttr;
    u32 modelAttr;

    linkCount = mbMasuLinkTblGet(spaceId, links);
    for (linkIndex = 0; linkIndex < linkCount; linkIndex++) {
        spaceAttr = mbMasuAttrGet(links[linkIndex]);
        modelAttr = mbMasuMAttrGet(links[linkIndex]);
        if ((spaceAttr & mbBranchAttrGet()) == 0 &&
            (modelAttr & mbBranchMAttrGet()) == 0) {
            if (modelAttr & 0x40) {
                return links[linkIndex];
            }
            if (selectedSpace < 0) {
                selectedSpace = links[linkIndex];
            }
        }
    }
    return selectedSpace;
}

// Find where a move ends, counting only visible spaces on the route.
static inline s16 W04RouteAdvance(s16 spaceId, int steps)
{
    while (steps != 0) {
        spaceId = W04LinkNext(spaceId);
        if (mbMasuDispCheck(spaceId)) {
            steps--;
        }
    }
    return spaceId;
}

HuVecF lbl_1_data_0[5] = {
    { -1200.0f, 130.0f, 1650.0f },
    { -1200.0f, 130.0f, 1850.0f },
    { -1200.0f, 130.0f, 2050.0f },
    { -1200.0f, 102.0f, 2250.0f },
    { -1200.0f, 74.0f, 2450.0f }
};

HuVecF lbl_1_data_3C[5] = {
    { -2150.0f, 1230.0f, -2950.0f },
    { -2150.0f, 1230.0f, -2750.0f },
    { -2150.0f, 1230.0f, -2550.0f },
    { -2150.0f, 1202.0f, -2350.0f },
    { -2150.0f, 1174.0f, -2150.0f }
};

HuVecF lbl_1_data_78[5] = {
    { 1835.0f, 1680.0f, -3050.0f },
    { 1835.0f, 1680.0f, -2850.0f },
    { 1835.0f, 1680.0f, -2650.0f },
    { 1835.0f, 1652.0f, -2450.0f },
    { 1835.0f, 1624.0f, -2250.0f }
};

HuVecF lbl_1_data_B4[5] = {
    { 3214.0f, 630.0f, 980.0f },
    { 3085.0f, 630.0f, 1305.0f },
    { 2956.0f, 630.0f, 1630.0f },
    { 2690.0f, 602.0f, 1630.0f },
    { 2424.0f, 574.0f, 1630.0f }
};

HuVecF lbl_1_data_F0[32] = {
    { -2485.0f, 1675.0f, -2175.0f },
    { -2297.88501f, 1675.0f, -2322.91992f },
    { -2081.14404f, 1675.0f, -2418.21997f },
    { -1843.52002f, 1675.0f, -2417.87012f },
    { -1608.255f, 1675.0f, -2377.92993f },
    { -1371.08105f, 1675.0f, -2354.73999f },
    { -1134.26099f, 1675.0f, -2382.09009f },
    { -896.836975f, 1675.0f, -2400.25f },
    { -660.440979f, 1675.0f, -2368.44995f },
    { -423.039001f, 1675.0f, -2346.79004f },
    { -184.460007f, 1675.0f, -2351.94995f },
    { 54.1570015f, 1676.50598f, -2354.62012f },
    { 290.462006f, 1703.14905f, -2351.19995f },
    { 506.480011f, 1775.46704f, -2376.08008f },
    { 718.22699f, 1859.29395f, -2399.98999f },
    { 956.314026f, 1861.17102f, -2400.0f },
    { 1167.29504f, 1939.83496f, -2397.43994f },
    { 1392.98096f, 1982.08398f, -2391.88989f },
    { 1618.03601f, 1975.87903f, -2468.55005f },
    { 1840.724f, 1975.00403f, -2553.73999f },
    { 2076.24805f, 1975.08801f, -2587.77002f },
    { 2308.33398f, 1976.23596f, -2544.29004f },
    { 2373.39697f, 1966.68005f, -2334.17993f },
    { 2295.80298f, 1880.79199f, -2128.37988f },
    { 2123.34595f, 1546.49695f, -1837.03003f },
    { 1967.82202f, 1168.79004f, -1590.31995f },
    { 1819.745f, 789.411011f, -1341.52002f },
    { 1669.96301f, 467.988007f, -1026.51001f },
    { 1578.40405f, 315.186005f, -587.080017f },
    { 1556.06299f, 285.210999f, -113.110001f },
    { 1560.25195f, 324.709015f, 361.850006f },
    { 1500.0f, 500.0f, 800.0f }
};

HuVecF lbl_1_data_270 = { 0.0f, 0.0f, -120.0f };

char *lbl_1_data_290[2] = { "flower_h1", "flower_h2" };

char *lbl_1_data_36C[20] = {
    "tree_h_a1", "tree_h_a2", "tree_h_a3", "tree_h_a4",
    "tree_h_a5", "tree_h_a6", "tree_h_a7", "tree_h_a8",
    "tree_h_a9", "tree_h_a10", "tree_h_a11", "tree_h_a12",
    "tree_h_a13", "tree_h_a14", "tree_h_a15", "tree_h_a16",
    "tree_h_a17", "tree_h_a18", "tree_h_a19", "tree_h_a20"
};

char *lbl_1_data_3F0[5] = { "tree_h_b1", "tree_h_b2", "tree_h_b3", "tree_h_b4", "tree_h_b5" };

char *lbl_1_data_438[5] = { "tree_h_c1", "tree_h_c2", "tree_h_c3", "tree_h_c4", "tree_h_c5" };

char *lbl_1_data_45C[2] = { "sori_h1", "sori_h2" };

HuVecF lbl_1_data_464 = { -28.0f, 0.0f, 0.0f };

HuVecF lbl_1_data_470 = { 0.0f, 500.0f, 0.0f };

#include "REL/w04Dll/common.h"

// Enter party mode and register this board's creation and closing hooks.
void fn_1_A0(void)
{
    GWPartySet(TRUE);
    mbObjectSetup(3, fn_1_F4, fn_1_9DC);
}

// Create the time-of-day scenery, attach animated models, and install board behavior.
void fn_1_F4(void)
{
    W04BoardModels *models = &lbl_1_bss_7280;
    // This setup queries the board number even though it does not use the result.
    int boardNo = MBBoardNoGet();
    int modelIndex;
    u32 hookIndex;

    HuAudSndGrpSetSet(25);
    lbl_1_bss_7308 = (W04BoardWork *)GwSystem.boardWork;
    mbObjDirSet(DATA_w04, DATA_w04n);
    mbMasuInit(W04TimeDayGet() ? DATA_w04 : DATA_w04n);
    mbCapThrowColCreate(DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 2));
    models->models[0] = mbObjCreate(
        DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 3), NULL, FALSE);
    mbObjAttrSet(models->models[0], HU3D_MOTATTR_LOOP);
    modelIndex = 0;
    (&models->models[1])[modelIndex] = mbObjCreate(
        DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 4), NULL, FALSE);
    mbObjLayerSet((&models->models[1])[modelIndex], 1);
    modelIndex++;
    (&models->models[1])[modelIndex] = models->shadowModelId = mbObjCreate(
        DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 20), NULL, FALSE);
    modelIndex++;
    (&models->models[1])[modelIndex] = mbObjCreate(
        DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 9), NULL, FALSE);
    mbObjAttrSet((&models->models[1])[modelIndex], HU3D_MOTATTR_LOOP);
    modelIndex++;
    (&models->models[1])[modelIndex] = mbObjCreate(
        DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 10), NULL, FALSE);
    mbObjAttrSet((&models->models[1])[modelIndex], HU3D_MOTATTR_LOOP);
    modelIndex++;
    for (hookIndex = 0; hookIndex < 20; modelIndex++, hookIndex++) {
        (&models->models[1])[modelIndex] = mbObjCreate(
            DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 23), NULL, TRUE);
        mbObjAttrSet((&models->models[1])[modelIndex], HU3D_MOTATTR_LOOP);
        mbObjHookSet(models->models[0], lbl_1_data_36C[hookIndex],
            (&models->models[1])[modelIndex]);
    }
    for (hookIndex = 0; hookIndex < 5; modelIndex++, hookIndex++) {
        (&models->models[1])[modelIndex] = mbObjCreate(
            DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 24), NULL, TRUE);
        mbObjAttrSet((&models->models[1])[modelIndex], HU3D_MOTATTR_LOOP);
        mbObjHookSet(models->models[0], lbl_1_data_3F0[hookIndex],
            (&models->models[1])[modelIndex]);
    }
    for (hookIndex = 0; hookIndex < 5; modelIndex++, hookIndex++) {
        (&models->models[1])[modelIndex] = mbObjCreate(
            DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 25), NULL, TRUE);
        mbObjAttrSet((&models->models[1])[modelIndex], HU3D_MOTATTR_LOOP);
        mbObjHookSet(models->models[0], lbl_1_data_438[hookIndex],
            (&models->models[1])[modelIndex]);
    }
    for (hookIndex = 0; hookIndex < 2; modelIndex++, hookIndex++) {
        (&models->models[1])[modelIndex] = mbObjCreate(
            DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 26), NULL, TRUE);
        mbObjAttrSet((&models->models[1])[modelIndex], HU3D_MOTATTR_LOOP);
        mbObjHookSet(models->models[0], lbl_1_data_290[hookIndex],
            (&models->models[1])[modelIndex]);
    }
    for (hookIndex = 0; hookIndex < 2; modelIndex++, hookIndex++) {
        (&models->models[1])[modelIndex] = mbObjCreate(
            DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 27), NULL, TRUE);
        mbObjHookSet(models->models[0], lbl_1_data_45C[hookIndex],
            (&models->models[1])[modelIndex]);
    }
    mbScrollInit(DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 1));
    mbLightFuncSet(fn_1_EC4, fn_1_1010);
    if (mbSaveNewF) {
        memset(lbl_1_bss_7308, 0, sizeof(W04BoardWork));
    }
    mbPlayerTurnInitHookSet(fn_1_DF4);
    mbPlayerTurnCloseHookSet(fn_1_E28);
    mbev_ShopExInit(DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 5), fn_1_1014);
    mbev_ShopBackMotCreate(DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 7),
        -1, 1, TRUE, "kanban_h");
    mbev_MasuMoveEndSet(fn_1_C04);
    mbev_MasuMoveStartSet(fn_1_BA8);
    mbOpeningInstHookSet(fn_1_A04);
    mbOpeningStarInstHookSet(NULL);
    mbOpeningViewSet(&lbl_1_data_464, &lbl_1_data_470, 6500.0f);
    mbev_MasuHatenaSet(fn_1_D3C);
    mbBranchComStarHookSet(fn_1_1140);
    mbScrollHookSet(fn_1_1338);
    mbMapCameraSet(NULL, &lbl_1_data_270, 14600.0f);
    mbMapHookSet(fn_1_1388);
    mbev_NextTimeSet(fn_1_143C);
    omAddObjEx(mbObjMan, 8204, 0, 0, -1, fn_1_B5C);
    fn_1_1D74();
    fn_1_495C();
    fn_1_4C48();
    fn_1_5B14();
    fn_1_707C();
    fn_1_AC0C();
    fn_1_D3EC();
    fn_1_D4F8();
    fn_1_D674();
    fn_1_DA4C();
    fn_1_DB84();
    fn_1_DDB8();
    HuDataDirClose(W04TimeDayGet() ? DATA_w04 : DATA_w04n);
}

// Release event and weather resources when the board's closing hook runs.
void fn_1_9DC(void)
{
    fn_1_215C();
    fn_1_4ACC();
    fn_1_4EA8();
}

// Present the guide's introduction, show the marked region, and restore the prior camera.
void fn_1_A04(void)
{
    static const HuVecF lbl_1_rodata_80 = { -30.0f, 0.0f, 0.0f };
    s16 windowId;
    s16 markedSpace;
    s16 linkedSpace;

    mbCameraStackPush();
    windowId = mbWinCreate(MBWIN_TYPE_EVENT,
        MESSNUM(MESS_BOARD_W04, 21), mbGuideSpeakerNoGet());
    mbWinWait(windowId);
    windowId = mbWinCreate(MBWIN_TYPE_EVENT,
        MESSNUM(MESS_BOARD_W04, 22), mbGuideSpeakerNoGet());
    mbWinWait(windowId);
    mbWipeDissolveFadeOut();
    markedSpace = mbMasuFind_MAttrIdGet(-1, W04_RIDE_REGION_MASK);
    linkedSpace = mbMasuAttrFindLink(markedSpace, W04_EVENT_LINK_MASK);
    mbCameraMoveMasu(linkedSpace, (HuVecF *)&lbl_1_rodata_80, NULL,
        3700.0f, -1.0f, -1);
    mbCameraMoveWait();
    mbWipeDissolveFadeIn();
    windowId = mbWinCreate(MBWIN_TYPE_EVENT,
        MESSNUM(MESS_BOARD_W04, 23), mbGuideSpeakerNoGet());
    mbWinWait(windowId);
    windowId = mbWinCreate(MBWIN_TYPE_EVENT,
        MESSNUM(MESS_BOARD_W04, 24), mbGuideSpeakerNoGet());
    mbWinWait(windowId);
    windowId = mbWinCreate(MBWIN_TYPE_EVENT,
        MESSNUM(MESS_BOARD_W04, 25), mbGuideSpeakerNoGet());
    mbWinWait(windowId);
    mbWipeDissolveFadeOut();
    mbCameraStackPop(-1);
    mbWipeDissolveFadeIn();
}

// Update effects and object motions each frame, or remove the updater during exit.
void fn_1_B5C(OMOBJ *object)
{
    if (mbExitCheck()) {
        omDelObjEx(HuPrcCurrentGet(), object);
    } else {
        fn_1_4F6C();
        fn_1_D618();
        fn_1_DEDC();
        fn_1_DD1C();
    }
}

// Start marked scenery animations when a player begins moving from a board space.
int fn_1_BA8(int playerNo, s16 spaceId) {
    u32 attr = mbMasuMAttrGet(spaceId);
    if (attr & 0x00000400U) {
        fn_1_D5B0();
    }
    if (attr & 0x00004000U) {
        fn_1_DE74(0);
    }
    return 0;
}

// Run region events and trigger the night-only introduction once for a human player.
int fn_1_C04(int playerNo, s16 spaceId)
{
    u32 attr = mbMasuMAttrGet(spaceId);

    if (attr & W04_RIDE_REGION_MASK) {
        return fn_1_21EC(playerNo, spaceId, mbev_MasuBitGet(attr, W04_RIDE_REGION_MASK) - 1);
    }
    if (attr & W04_MOLE_TRIGGER_MASK) {
        fn_1_DB18(mbev_MasuBitGet(attr, W04_MOLE_TRIGGER_MASK) - 1);
    }
    if ((attr & 0x10) && GwSystem.curTime != 0 && GwPlayer[playerNo].comF == 0 &&
        lbl_1_bss_7308->linkedSpaceMessageShown == 0) {
        fn_1_4B54(playerNo, spaceId);
    }
    if (attr & W04_PENGUIN_REGION_MASK) {
        fn_1_DC8C(mbev_MasuBitGet(attr, W04_PENGUIN_REGION_MASK) - 1);
    }
    return 0;
}

// Handle a happening space with a rolling event, or the day/night coin event, when landed on.
int fn_1_D3C(int playerNo, s16 spaceId) {
    s32 spaceAttr;

    spaceAttr = mbMasuMAttrGet(spaceId);
    if ((u32) (spaceAttr & 2) != 0) {
        fn_1_5C64(playerNo, spaceId);
        return 0;
    }
    if ((u32) (spaceAttr & 8) != 0) {
        if ((u8) (((*(u8 *)((s8 *)(&GwSystem) + (W04_DAY_BYTE_OFFSET))) >> 6U) & 1) != 0) {
            fn_1_709C(playerNo, spaceId);
        } else {
            fn_1_AC10(playerNo, spaceId);
        }
        return 0;
    }
    return 1;
}

// Restore full weather opacity and show its particles when a player's turn begins.
void fn_1_DF4(int playerNo)
{
    fn_1_5AF4(1.0f);
    fn_1_5B04(TRUE);
}

// Restore player shadows and board lighting after a scene closes.
void fn_1_E28(int turnPlayerNo)
{
    W04BoardModels *models = &lbl_1_bss_7280;
    HU3D_LIGHT *light = Hu3DGlobalLight;
    int playerNo;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        mbObjShadowReset(mbPlayerObjIDGet(playerNo));
    }
    light->color.r = models->lightColor.r;
    light->color.g = models->lightColor.g;
    light->color.b = models->lightColor.b;
    mbObjShadowMapReset(lbl_1_bss_7280.shadowModelId);
    fn_1_5B04(TRUE);
    mbMasuPlayerFadeSet(FALSE);
}

// Give starting stars on a new board and save the light color for later restoration.
void fn_1_EC4(void)
{
    W04BoardModels *models = &lbl_1_bss_7280;
    HU3D_LIGHT *light = Hu3DGlobalLight;
    int playerNo;

    if (mbSaveNewF) {
        if ((BOOL)GwSystem.tagF == FALSE) {
            for (playerNo = 0; playerNo < 4; playerNo++) {
                mbPlayerStarSet(playerNo, mbPlayerHandicapGet(playerNo) + 5);
            }
        } else {
            for (playerNo = 0; playerNo < 2; playerNo++) {
                int teamPlayer = mbPlayerTeamFindPlayer(playerNo, 0);
                mbPlayerStarSet(teamPlayer, mbPlayerHandicapGet(teamPlayer) + 10);
            }
        }
    }
    Hu3DBGColorSet(255, 255, 255);
    Hu3DModelLightInfoSet(mbObjModelIDGet(models->models[0]), TRUE);
    Hu3DFogSet(5000.0f, 70000.0f, 200, 200, 200);
    models->lightColor.r = light->color.r;
    models->lightColor.g = light->color.g;
    models->lightColor.b = light->color.b;
}

void fn_1_1010(void)
{
}

// Copy the shop model's transform to one model, then attach a looping model to its smoke_f1 hook.
void fn_1_1014(int modelId, int shopNo)
{
    HuVecF position;
    HuVecF rotation;
    MBMODELID smokeModelId;

    smokeModelId = mbObjCreate(DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 6),
        NULL, TRUE);
    mbObjPosGet(modelId, &position);
    mbObjRotGet(modelId, &rotation);
    mbObjPosSetV(smokeModelId, &position);
    mbObjRotSetV(smokeModelId, &rotation);
    smokeModelId = mbObjCreate(DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 8),
        NULL, TRUE);
    mbObjAttrSet(smokeModelId, HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP);
    mbObjHookSet(modelId, "smoke_f1", smokeModelId);
    mbObjLayerSet(modelId, 3);
    mbObjCullRadiusSet(modelId, 400.0f);
}

int lbl_1_data_490[4] = { 7, 9, 11, 13 };

// Select a branch using marked spaces and how close other players are to the route's destination.
int fn_1_1140(int playerNo, int count, s16 *spaces, BOOL forceMarked)
{
    int nearestDistance = 9999;
    s16 markedIndex;
    u32 attr;
    int index;
    int distance;
    s16 startSpace;
    int stepsToRegion;
    s16 endSpace;
    s16 otherSpace;

    if (forceMarked) {
        for (index = 0; index < count; index++) {
            attr = mbMasuMAttrGet(spaces[index]);
            if (attr & 0x40) {
                return index;
            }
        }
        return 0;
    }
    markedIndex = -1;
    for (index = 0; index < count; index++) {
        attr = mbMasuMAttrGet(spaces[index]);
        if (attr & 0x40) {
            markedIndex = index;
        }
    }
    if (markedIndex < 0) {
        return mbRandMod(count);
    }
    startSpace = GwPlayer[playerNo].masuId;
    endSpace = fn_1_28EC(startSpace, GwPlayer[playerNo].moveNum);
    stepsToRegion = mbMasuFind_MAttrStepGet(startSpace, W04_RIDE_REGION_MASK);
    if (stepsToRegion > GwPlayer[playerNo].moveNum) {
        return markedIndex;
    }
    for (index = 0; index < 4; index++) {
        if (index == playerNo) {
            continue;
        }
        otherSpace = GwPlayer[index].masuId;
        distance = mbMasuFind_IdStepGet(otherSpace, endSpace);
        if (distance < nearestDistance) {
            nearestDistance = distance;
        }
    }
    if (nearestDistance < lbl_1_data_490[GwPlayer[playerNo].comDif]) {
        for (index = 0; index < count; index++) {
            if (markedIndex != index) {
                return index;
            }
        }
    }
    return markedIndex;
}

// Halve weather opacity while the board scroll is open, then restore it when closed.
void fn_1_1338(int scrollOpen) {
    if (scrollOpen != 0) {
        fn_1_5AF4(0.5f);
    } else {
        fn_1_5AF4(1.0f);
    }
}

// Hide fog and weather while adding marked spaces to the open board map.
void fn_1_1388(BOOL mapOpen)
{
    s16 spaces[16];
    int count;
    int index;

    if (mapOpen) {
        fn_1_5B04(FALSE);
        Hu3DFogClear();
        count = mbMasuMAttrListGet(W04_RIDE_REGION_MASK, spaces);
        for (index = 0; index < count; index++) {
            mbMapSprAdd(20, spaces[index]);
        }
    } else {
        fn_1_5B04(TRUE);
        Hu3DFogSet(5000.0f, 70000.0f, 200, 200, 200);
    }
}

// Hide players during the time-change presentation and let a human skip its model sequence.
void fn_1_143C(void)
{
    HuVecF position;
    W04TimeChange *sequence = &lbl_1_bss_7270;
    int playerNo;
    MBMODELID modelId;
    s16 windowId;
    HUPROCESS *sequenceProcess;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        mbPlayerDispSet(playerNo, FALSE);
    }
    memset(sequence, 0, sizeof(W04TimeChange));
    sequenceProcess = HuPrcChildCreate(fn_1_1738, 8199, 32768, 0, mbMainProc);
    sequence->animation = HuSprAnimRead(HuDataSelHeapReadNum(
        mbBoardDataNumGet(DATANUM(DATA_board, 94)), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprAnimLock(sequence->animation);
    sequence->particleModelId = mbParticleCreate(sequence->animation, 30);
    Hu3DModelLayerSet(sequence->particleModelId, 5);
    mbParticleAttrSet(sequence->particleModelId, MB_PARTICLE_ATTR_UPAUSE);
    mbParticleBlendModeSet((s16)sequence->particleModelId, MB_PARTICLE_BLEND_ADDCOL);
    mbParticleHookSet(sequence->particleModelId, fn_1_19EC);
    Hu3DModelAttrSet(sequence->particleModelId, HU3D_ATTR_DISPOFF);
    windowId = -1;
    if (GwSystem.curTime == 0) {
        for (playerNo = 0; playerNo < 3; playerNo++) {
            modelId = fn_1_4B28(playerNo);
            mbObjPosGet(modelId, &position);
            mbObjFadeCreate((s16)modelId, &position);
            mbObjFadeTexColorSet((s16)modelId, 128, 128, 128, 1.0f);
            mbObjDispSet(modelId, TRUE);
        }
    }
    do {
        if (sequence->skipAllowed) {
            if (windowId < 0) {
                windowId = mbWinCreateHelp(MESSNUM(MESS_BOARD_OPE, 12));
                mbWinPosSet(windowId, 228, 408);
            }
            for (playerNo = 0; playerNo < 4; playerNo++) {
                if (GwPlayer[playerNo].comF == 0 &&
                    (HuPadBtnDown[GwPlayer[playerNo].padNo] & PAD_BUTTON_START)) {
                    HuPrcKill(sequenceProcess);
                    sequence->finished = TRUE;
                    break;
                }
            }
        }
        HuPrcVSleep();
    } while (!sequence->finished);
    mbWipeWait();
    if (!WipeCheckIn()) {
        mbWipeFadeOut();
    }
    if (GwSystem.curTime == 0) {
        for (playerNo = 0; playerNo < 3; playerNo++) {
            modelId = fn_1_4B28(playerNo);
            mbObjFadeKill((s16)modelId);
            mbObjDispSet(modelId, FALSE);
        }
    }
    mbWinKill(windowId);
    mbParticleKill(sequence->particleModelId);
    HuSprAnimKill(sequence->animation);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        mbPlayerDispSet(playerNo, TRUE);
    }
}

HuVecF lbl_1_data_4A0[3][2] = {
    { { -50.0f, 15.0f, 0.0f }, { -20.0f, 0.0f, 0.0f } },
    { { -30.0f, -10.0f, 0.0f }, { -20.0f, 0.0f, 0.0f } },
    { { -60.0f, 15.0f, 0.0f }, { -20.0f, 0.0f, 0.0f } }
};

float lbl_1_data_4E8[3][2] = {
    { 5000.0f, 2000.0f },
    { 3000.0f, 2000.0f },
    { 2500.0f, 2000.0f }
};
HuVecF *lbl_1_data_500[4] = { lbl_1_data_0, lbl_1_data_3C, lbl_1_data_78, lbl_1_data_B4 };
int lbl_1_data_510[4] = { 5, 5, 5, 5 };

int lbl_1_data_520[3] = { DATANUM(DATA_w04, 33), DATANUM(DATA_w04, 34), -1 };

int lbl_1_data_52C[3] = { DATANUM(DATA_w04n, 33), DATANUM(DATA_w04n, 34), -1 };

// Present the time-change title and visit each model before the board resumes.
void fn_1_1738(void)
{
    HuVecF cameraPosition;
    HuVecF modelPosition;
    W04TimeChange *sequence = &lbl_1_bss_7270;
    MBMODELID modelId;
    MBPARTICLE *particle;
    int modelIndex;
    int frame;
    float progress;

    particle = Hu3DData[sequence->particleModelId].hookData;
    cameraPosition.x = 0.0f;
    cameraPosition.y = 300.0f;
    cameraPosition.z = -250.0f;
    mbCameraMovePos(&cameraPosition, NULL, NULL, 5000.0f, -1.0f, 1);
    mbCameraMoveWait();
    mbWipeFadeIn();
    mbTelopTimeChangeCreate();
    while (mbTelopTimeChangeCheck()) {
        HuPrcVSleep();
    }
    mbWipeFadeOut();
    sequence->skipAllowed = TRUE;
    for (modelIndex = 0; modelIndex < 3; modelIndex++) {
        modelId = fn_1_4B28(modelIndex);
        mbObjPosGet(modelId, &modelPosition);
        cameraPosition.x = modelPosition.x;
        cameraPosition.y = 1000.0f + modelPosition.y;
        cameraPosition.z = modelPosition.z;
        mbCameraMovePos(&cameraPosition, &lbl_1_data_4A0[modelIndex][0], NULL,
            lbl_1_data_4E8[modelIndex][0], -1.0f, 1);
        HuPrcVSleep();
        cameraPosition.y -= 1000.0f;
        mbCameraMovePos(&cameraPosition, &lbl_1_data_4A0[modelIndex][1], NULL,
            lbl_1_data_4E8[modelIndex][1], -1.0f, 240);
        mbCameraCurveTypeSet(1);
        mbWipeFadeIn();
        if (GwSystem.curTime == 0) {
            Hu3DModelPosSetV(sequence->particleModelId, &modelPosition);
            Hu3DModelAttrReset(sequence->particleModelId, HU3D_ATTR_DISPOFF);
            particle->mode = 0;
            for (frame = 0; frame <= 180U; frame++) {
                progress = frame / 180.0f;
                mbObjPosSet(modelId, modelPosition.x,
                    modelPosition.y - 300.0f * progress, modelPosition.z);
                HuPrcVSleep();
            }
        } else {
            HuPrcSleep(180);
        }
        mbWipeFadeOut();
    }
    sequence->finished = TRUE;
    HuPrcEnd();
}

// Delay the time-change particles, then emit and restart them until the presentation ends.
void fn_1_19EC(HU3D_MODEL *model, MBPARTICLE *effect, Mtx matrix)
{
    GXColor color = {128, 128, 128, 128};
    MBPARTICLEDATA *particle;
    int index;
    int activeCount;

    if (effect->mode == 0) {
        particle = effect->data;
        for (index = 0; index < effect->num; index++, particle++) {
            particle->pos.x = 150.0f * frandf() - 75.0f;
            particle->pos.y = 0.0f;
            particle->pos.z = 150.0f * frandf() - 75.0f;
            particle->vel.x = 0.0f;
            particle->vel.y = 400.0f + 400.0f * frandf();
            particle->vel.z = 0.0f;
            particle->scale = 100.0f + 50.0f * frandf();
            particle->animTime = 0.0f;
            particle->animSpeed = 0.5f;
            particle->animNo = 0;
            particle->dispF = FALSE;
            particle->pauseF = TRUE;
            particle->time = 1.0f + 60.0f * frandf();
            particle->color = color;
        }
        effect->mode = 1;
        effect->count = 0;
    }
    activeCount = 0;
    particle = effect->data;
    for (index = 0; index < effect->num; index++, particle++) {
        if (particle->time != 0) {
            if (--particle->time == 0) {
                particle->dispF = TRUE;
                particle->pauseF = FALSE;
            }
            activeCount++;
        } else {
            if (particle->pauseF) {
                if (effect->count > 168) {
                    continue;
                }
                particle->pos.x = 150.0f * frandf() - 75.0f;
                particle->pos.y = 0.0f;
                particle->pos.z = 150.0f * frandf() - 75.0f;
                particle->animTime = 0.0f;
                particle->animSpeed = 0.5f;
                particle->animNo = 0;
                particle->dispF = TRUE;
                particle->pauseF = FALSE;
            }
            particle->pos.x += (1.0f / 60.0f) * particle->vel.x;
            particle->pos.y += (1.0f / 60.0f) * particle->vel.y;
            particle->pos.z += (1.0f / 60.0f) * particle->vel.z;
            activeCount++;
        }
    }
    if (activeCount == 0) {
        Hu3DModelAttrSet(effect->modelId, HU3D_ATTR_DISPOFF);
    }
}

// Create the models at each linked event space and the particle rings used by its events.
void fn_1_1D74(void)
{
    HuVecF eventPosition;
    HuVecF linkedPosition;
    HuVecF direction;
    W04SpaceModels *models;
    int index;
    s32 allocationSize;
    u32 spaceAttribute;

    lbl_1_bss_7268 = mbMasuMAttrListGet(W04_RIDE_REGION_MASK, NULL);
    allocationSize = lbl_1_bss_7268 * sizeof(W04SpaceModels);
    models = mbMalloc(allocationSize);
    lbl_1_bss_726C = models;
    for (index = 0; index < lbl_1_bss_7268; index++, models++) {
        if (GwSystem.curTime == 0) {
            models->actorModelId = mbObjCreate(
                DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 32), lbl_1_data_520, FALSE);
        } else {
            models->actorModelId = mbObjCreate(
                DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 32), lbl_1_data_52C, FALSE);
        }
        mbObjDispSet(models->actorModelId, FALSE);
        mbObjMotionSet(models->actorModelId, 1, HU3D_MOTATTR_LOOP);
        models->baseModelId = mbObjCreate(
            DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 12), NULL, TRUE);
        models->effectModelId = mbObjCreate(
            DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 39), NULL, TRUE);
        mbObjDispSet(models->effectModelId, FALSE);
        models->capsuleModelId = mbCapObjCreate(30, FALSE);
        mbObjDispSet(models->capsuleModelId, FALSE);
        spaceAttribute = mbev_MasuAttrGet(index + 1, W04_RIDE_REGION_MASK);
        models->eventSpaceId = mbMasuFind_MAttrMatchIdGet(-1, spaceAttribute, W04_RIDE_REGION_MASK);
        mbMasuPosGet(models->eventSpaceId, &eventPosition);
        models->linkedSpaceId = mbMasuAttrFindLink(models->eventSpaceId, W04_EVENT_LINK_MASK);
        mbMasuPosGet(models->linkedSpaceId, &linkedPosition);
        mbObjPosSet(models->actorModelId, linkedPosition.x,
            30.000002f + linkedPosition.y, linkedPosition.z);
        mbObjPosSetV(models->baseModelId, &linkedPosition);
        PSVECSubtract(&eventPosition, &linkedPosition, &direction);
        models->directionDegrees = 180.0 * (atan2(direction.x, direction.z) / M_PI);
        mbObjRotYSet(models->actorModelId, models->directionDegrees);
        mbObjRotYSet(models->baseModelId, models->directionDegrees);
        models->modelIndex = index;
        models->playerNo = -1;
        models->soundId = -1;
    }
    lbl_1_bss_7264 = HuSprAnimRead(HuDataSelHeapReadNum(
        mbBoardDataNumGet(DATANUM(DATA_board, 94)), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprAnimLock(lbl_1_bss_7264);
    for (index = 0; index < 8; index++) {
        lbl_1_bss_724C[index] = mbParticleCreate(lbl_1_bss_7264, 20);
        Hu3DModelLayerSet(lbl_1_bss_724C[index], 5);
        mbParticleAttrSet(lbl_1_bss_724C[index], MB_PARTICLE_ATTR_UPAUSE);
        mbParticleHookSet(lbl_1_bss_724C[index], fn_1_44B8);
        Hu3DModelAttrSet(lbl_1_bss_724C[index], HU3D_ATTR_DISPOFF);
    }
}

// Release the effect's working memory and sprite animation when it closes.
void fn_1_215C(void)
{
    if (lbl_1_bss_726C != 0) {
        void *work = lbl_1_bss_726C;

        HuMemDirectFree(work);
        lbl_1_bss_726C = 0;
    }
    if (lbl_1_bss_7264 != 0) {
        HuSprAnimKill(lbl_1_bss_7264);
        lbl_1_bss_7264 = 0;
    }
}

int lbl_1_data_538[3] = { 15, 19, 20 };

int lbl_1_data_544[3] = { 10, 20, 30 };

int lbl_1_data_550[3] = { 7, 14, 21 };

// Offer a paid die at a linked space, then run its movement event for the player.
int fn_1_21EC(int playerNo, s16 spaceId, int modelIndex)
{
    char numberText[16];
    HuVecF playerPosition;
    HuVec2f windowCenter;
    MBPLAYERWORK *playerWork;
    s16 window;
    int choice;
    int index;
    W04SpaceModels *models;
    int highestChoice;
    int moveCount;
    int diceType;
    int price;
    int pathLength;

    models = &((W04SpaceModels *)lbl_1_bss_726C)[modelIndex];
    playerWork = mbPlayerWorkGet(playerNo);
    {
        HuVecF numberOffset = {0.0f, 300.0f, 0.0f};
        mbPlayerRotateStart(playerNo, 180.0f + models->directionDegrees, 15);
        while (!mbPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
        if (GwSystem.curTime == 0) {
            if (mbPlayerCoinGet(playerNo) < 20) {
                window = mbWinCreate(2, MESSNUM(MESS_BOARD_W04, 1), -1);
                mbWinWait(window);
                return FALSE;
            }
        } else {
            if (mbPlayerCoinGet(playerNo) < 10) {
                window = mbWinCreate(2, MESSNUM(MESS_BOARD_W04, 2), -1);
                mbWinWait(window);
                return FALSE;
            }
        }
        if (GwSystem.curTime == 0) {
            highestChoice = 0;
        } else {
            highestChoice = 0;
            for (index = 0; index < 3; index++) {
                if (lbl_1_data_544[index] > mbPlayerCoinGet(playerNo)) {
                    break;
                }
                highestChoice = index;
            }
        }
        do {
            window = mbWinCreateChoice(2, MESSNUM(MESS_BOARD_W04, 0), 22, 0);
            mbWinCenterGet(window, &windowCenter);
            mbWinChoiceGet(window);
            mbWinPosSet(window, windowCenter.x, 288);
            mbAudFXPlay(MSM_SE_GUIDE_77);
            if (GwPlayer[playerNo].comF) {
                pathLength = fn_1_2748(playerNo, spaceId);
                if (pathLength >= 0 && pathLength <= lbl_1_data_550[highestChoice]) {
                    mbComChoiceUpSet();
                } else {
                    mbComChoiceDownSet();
                }
            }
            mbWinWait(window);
            choice = mbWinChoiceGet(window);
            if (choice == 2) {
                mbMoveNumDispSet(playerNo, FALSE);
                mbev_Scroll(playerNo, FALSE);
                mbMoveNumDispSet(playerNo, TRUE);
            }
        } while (choice == 2);
        if (choice < 0 || choice == 1) {
            return FALSE;
        }
        mbMoveNumDispSet(playerNo, FALSE);
        if (GwSystem.curTime == 0) {
            window = mbWinCreateChoice(1, MESSNUM(MESS_BOARD_W04, 5), 22, 0);
        } else {
            window = mbWinCreateChoice(1, MESSNUM(MESS_BOARD_W04, 6), 22, 0);
            for (index = highestChoice + 1; index < 3; index++) {
                mbWinChoiceDisable(window, index);
            }
        }
        mbWinChoiceGet(window);
        if (GwPlayer[playerNo].comF) {
            mbComChoiceListDownSet(highestChoice);
        }
        mbWinWait(window);
        choice = mbWinChoiceGet(window);
        if (GwSystem.curTime == 0) {
            if (choice < 0 || choice == 1) {
                mbMoveNumDispSet(playerNo, TRUE);
                return FALSE;
            }
            diceType = 15;
            price = 20;
        } else {
            if (choice < 0 || choice == 3) {
                mbMoveNumDispSet(playerNo, TRUE);
                return FALSE;
            }
            diceType = lbl_1_data_538[choice];
            price = lbl_1_data_544[choice];
        }
        mbCoinAddProcExec(playerNo, -price, -1, TRUE);
        mbCameraPlayerViewSet(playerNo, MB_CAMERA_VIEW_ZOOMIN);
        mbPlayerRotateStart(playerNo, 0, 18);
        mbCameraMoveWait();
        while (!mbPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
        moveCount = mbDiceProcExec(playerNo, diceType, 0, 0, TRUE, TRUE, 0, TRUE);
        mbDiceNumKill(playerNo);
        mbPlayerPosGet(playerNo, &playerPosition);
        models->diceNumberObject = mbDiceSNpcNumObjCreate(
            &playerPosition, &numberOffset, moveCount, FALSE, TRUE);
        window = mbWinCreate(2, MESSNUM(MESS_BOARD_W04, 7), 22);
        sprintf(numberText, "%d", moveCount);
        mbWinInsertMesSet(window, (u32)numberText, 0);
        mbAudFXPlay(MSM_SE_GUIDE_77);
        mbWinWait(window);
        mbCameraPlayerViewSet(playerNo, MB_CAMERA_VIEW_WALK);
        mbCameraMoveWait();
        models->playerNo = playerNo;
        fn_1_29F4(models, moveCount);
        mbDiceSNpcNumKill(models->diceNumberObject);
        GwPlayer[playerNo].moveNum = 0;
        return TRUE;
    }
}

// Count visible spaces until reaching another player, or return -1 after a full loop.
int fn_1_2748(int playerNo, s16 startSpace)
{
    int visibleCount = 0;
    int otherPlayer;
    s16 currentSpace = startSpace;

    do {
        currentSpace = fn_1_2810(currentSpace);
        if (mbMasuDispCheck(currentSpace)) {
            visibleCount++;
        }
        for (otherPlayer = 0; otherPlayer < 4; otherPlayer++) {
            if (otherPlayer != playerNo
                && currentSpace == GwPlayer[otherPlayer].masuId) {
                return visibleCount;
            }
        }
    } while (currentSpace != startSpace);
    return -1;
}

// Choose a permitted outgoing link, preferring one with the board's priority bit.
s16 fn_1_2810(s16 spaceId)
{
    s16 links[MASU_LINK_MAX];
    s16 selectedSpace = -1;
    int linkCount;
    int linkIndex;
    u32 spaceAttr;
    u32 modelAttr;

    linkCount = mbMasuLinkTblGet(spaceId, links);
    for (linkIndex = 0; linkIndex < linkCount; linkIndex++) {
        spaceAttr = mbMasuAttrGet(links[linkIndex]);
        modelAttr = mbMasuMAttrGet(links[linkIndex]);
        if ((spaceAttr & mbBranchAttrGet()) != 0
            || (modelAttr & mbBranchMAttrGet()) != 0) {
            continue;
        }
        if (modelAttr & W04_LINK_PRIORITY_MASK) {
            return links[linkIndex];
        }
        if (selectedSpace < 0) {
            selectedSpace = links[linkIndex];
        }
    }
    return selectedSpace;
}

// Advance the requested number of visible spaces along permitted board links.
s16 fn_1_28EC(s16 spaceId, int steps)
{
    while (steps != 0) {
        spaceId = fn_1_2810(spaceId);
        if (mbMasuDispCheck(spaceId)) {
            steps--;
        }
    }
    return spaceId;
}

HuVecF lbl_1_data_560 = { -35.0f, 0.0f, 0.0f };

// Board the mount, ride the requested visible spaces, and return the player to the board.
void fn_1_29F4(W04SpaceModels *models, int moveCount)
{
    HuVecF position;
    HuVecF playerPosition;
    HuVecF hookPosition;
    HuVecF modelPosition;
    HuVecF startPosition;
    HuVecF entrancePosition;
    HuVecF numberOffset;
    HuVecF currentNumberOffset;
    HuVecF exitRotation;
    char numberText[16];
    s16 mountMotion;
    s16 nextSpace;
    int playerNo;
    HUPROCESS *currentProcess;
    s16 windowId;
    HUPROCESS *mountProcess;
    HUPROCESS *reactionProcess;
    s16 currentSpace;
    int frame;
    float progress;
    float originalAngle;
    float height;

    playerNo = models->playerNo;
    currentProcess = HuPrcCurrentGet();
    GwPlayer[playerNo].masuIdNext = W04RouteAdvance(models->eventSpaceId, moveCount);
    HuPrcVSleep();
    while (!mbPlayerColCheck()) {
        HuPrcVSleep();
    }
    mountMotion = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 66));
    mbMasuPosGet(models->eventSpaceId, &startPosition);
    mbMasuPosGet(models->linkedSpaceId, &entrancePosition);
    mbObjPosSet(models->actorModelId, entrancePosition.x,
        30.000002f + entrancePosition.y, entrancePosition.z);
    Hu3DModelObjPosGet(mbObjModelIDGet(models->actorModelId), "itemhook_C", &hookPosition);
    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    mbObjMotionSet(models->actorModelId, 2, HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP);
    mbPlayerPosGet(playerNo, &playerPosition);
    originalAngle = mbPlayerRotYGet(playerNo);
    mbDiceSNpcNumOfsGet(models->diceNumberObject, &numberOffset);
    mbObjDispSet(models->actorModelId, TRUE);
    mountProcess = HuPrcChildCreate(fn_1_438C, currentProcess->prio, 16384, 0, mbMainProc);
    mountProcess->property = models;
    for (frame = 0; frame <= 30U; frame++) {
        progress = (frame + 5) / 30.0f;
        if (progress > 1.0f) {
            progress = 1.0f;
        }
        if (frame == 24U) {
            mbPlayerMotionShiftSet(playerNo, mountMotion, 0.0f, 8.0f,
                HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP);
        }
        height = 30.000002f * mbCosDeg(90.0f * progress);
        position.x = entrancePosition.x + progress * (startPosition.x - entrancePosition.x);
        position.y =
            height + (entrancePosition.y + progress * (startPosition.y - entrancePosition.y));
        position.z = entrancePosition.z + progress * (startPosition.z - entrancePosition.z);
        mbObjPosSet(models->actorModelId, position.x, position.y, position.z);
        progress = frame / 30.0f;
        position.y = playerPosition.y + progress * (hookPosition.y - playerPosition.y) +
            100.0f * (3.0f * mbSinDeg(180.0f * progress));
        mbPlayerPosSet(playerNo, playerPosition.x, position.y, playerPosition.z);
        mbPlayerRotYSet(playerNo, mbAngleLerp(originalAngle, models->directionDegrees, progress));
        currentNumberOffset.x = numberOffset.x;
        currentNumberOffset.y = numberOffset.y + 100.0f * (2.0f * progress);
        currentNumberOffset.z = numberOffset.z;
        mbDiceSNpcNumOfsSet(models->diceNumberObject, &currentNumberOffset);
        HuPrcVSleep();
    }
    mbCameraFocusObjSet(models->actorModelId);
    mbCameraOffsetSet(0.0f, 150.0f, 0.0f);
    mbPlayerPosSet(playerNo, 0.0f, 0.0f, 0.0f);
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    mbPlayerScaleSet(playerNo, 1.0f, 1.0f, 1.0f);
    mbObjHookSet(models->actorModelId, "itemhook_C", mbPlayerObjIDGet(playerNo));
    reactionProcess = HuPrcChildCreate(fn_1_47BC, currentProcess->prio, 16384, 0, mbMainProc);
    reactionProcess->property = models;
    models->starCount = 0;
    lbl_1_bss_7260 = FALSE;
    lbl_1_bss_725C = 0;
    currentSpace = models->eventSpaceId;
    while (moveCount != 0) {
        nextSpace = W04LinkNext(currentSpace);
        fn_1_3618(models, currentSpace, nextSpace);
        currentSpace = nextSpace;
        if (mbMasuDispCheck(currentSpace)) {
            moveCount--;
        }
        mbDiceSNpcNumSet(models->diceNumberObject, moveCount);
    }
    mbDiceSNpcNumDispSet(models->diceNumberObject, FALSE);
    mbObjMotionSet(models->actorModelId, 1, HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP);
    originalAngle = mbObjRotYGet(models->actorModelId);
    for (frame = 0; frame <= 12U; frame++) {
        progress = frame / 12.0f;
        mbObjRotYSet(models->actorModelId, mbAngleEaseOut(originalAngle, 0.0f, progress));
        HuPrcVSleep();
    }
    mbObjHookReset(models->actorModelId);
    Hu3DModelObjPosGet(mbObjModelIDGet(models->actorModelId), "itemhook_C", &position);
    mbPlayerPosSetV(playerNo, &position);
    mbPlayerRotYSet(playerNo, mbObjRotYGet(models->actorModelId));
    if (models->starCount == 0) {
        windowId = mbWinCreate(2, MESSNUM(MESS_BOARD_W04, 4), 22);
        mbAudFXPlay(MSM_SE_GUIDE_77);
    } else {
        windowId = mbWinCreate(2, MESSNUM(MESS_BOARD_W04, 3), 22);
        sprintf(numberText, "%d", models->starCount);
        mbWinInsertMesSet(windowId, (u32)numberText, 0);
        mbAudFXPlay(MSM_SE_GUIDE_77);
    }
    mbWinWait(windowId);
    mbCameraMovePlayer(playerNo, &lbl_1_data_560, NULL, 1600.0f, -1.0f, 30);
    mbCameraMoveWait();
    if (models->starCount != 0) {
        mbPlayerMotionShiftSet(playerNo, 7, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
        CharFXPlay(GwPlayer[playerNo].charNo, CHARVOICEID(0));
    } else {
        mbPlayerMotionShiftSet(playerNo, 8, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
        CharFXPlay(GwPlayer[playerNo].charNo, CHARVOICEID(12));
    }
    if (models->starCount == 0) {
        mbAudFXPlay(W04_RIDE_END_SOUND);
    }
    mbStarAddProcExec(playerNo, models->starCount, TRUE, TRUE);
    while (!mbPlayerMotionEndCheck(playerNo)) {
        HuPrcVSleep();
    }
    HuPrcSleep(12);
    mbCameraFocusObjSet(-1);
    mbObjPosGet(models->actorModelId, &modelPosition);
    mbPlayerPosGet(playerNo, &playerPosition);
    mbMasuPosGet(currentSpace, &startPosition);
    mbMasuRotGet(currentSpace, &exitRotation);
    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    lbl_1_bss_7260 = TRUE;
    mbObjFadeCreate((MBMODELID)models->actorModelId, &modelPosition);
    mbObjFadeTexRotSet((MBMODELID)models->actorModelId, &modelPosition, &exitRotation);
    mbObjFadeTexColorSet((MBMODELID)models->actorModelId, 0, 0, 0, 1.0f);
    models->soundId = mbAudFXPlay(MSM_SE_BRD00_81);
    for (frame = 0; frame < 30U; frame++) {
        progress = frame / 30.0f;
        position.y = modelPosition.y + 100.0f * (-2.0f * progress);
        mbObjPosSet(models->actorModelId, modelPosition.x, position.y, modelPosition.z);
        mbObjRotYSet(models->actorModelId, 720.0f * progress);
        // This jump finishes before the frame-54 motion change can run.
        if (frame == 54U) {
            mbPlayerMotionShiftSet(playerNo, 5, 0.0f, 2.0f, HU3D_MOTATTR_NONE);
        }
        position.x = playerPosition.x + progress * (startPosition.x - playerPosition.x);
        position.y = playerPosition.y + progress * (startPosition.y - playerPosition.y) +
            100.0f * mbSinDeg(180.0f * progress);
        position.z = playerPosition.z + progress * (startPosition.z - playerPosition.z);
        mbPlayerPosSetV(playerNo, &position);
        HuPrcVSleep();
    }
    mbAudFXStop(models->soundId);
    models->soundId = -1;
    HuPrcKill(reactionProcess);
    HuPrcKill(mountProcess);
    GwPlayer[playerNo].masuId = currentSpace;
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP);
    mbObjDispSet(models->actorModelId, FALSE);
    mbObjFadeKill((MBMODELID)models->actorModelId);
    mbObjRotYSet(models->actorModelId, models->directionDegrees);
    mbPlayerMotionKill(playerNo, mountMotion);
}

// Move between spaces, take a star from encountered players, or use their capsule to repel the
// mount.
void fn_1_3618(W04SpaceModels *models, s16 startSpace, s16 endSpace)
{
    W04ProtectedPlayer protectedPlayers[4];
    int encounteredPlayers[4];
    int starChanges[4] = {0, 0, 0, 0};
    HuVecF position;
    HuVecF startPosition;
    HuVecF endPosition;
    HuVecF defenderPosition;
    HuVecF direction;
    HuVecF effectRotation;
    HuVecF translation;
    W04ProtectedPlayer *defender;
    int capsuleIndex;
    int frame;
    int encounteredCount;
    int protectedCount;
    s16 nextSpace;
    float progress;
    float turnProgress;
    float directionAngle;
    float originalAngle;

    encounteredCount = 0;
    for (frame = 0; frame < 4; frame++) {
        if (frame != models->playerNo && (lbl_1_bss_725C & (1 << frame)) == 0 &&
            endSpace == GwPlayer[frame].masuId) {
            encounteredPlayers[encounteredCount++] = frame;
        }
    }
    mbObjPosGet(models->actorModelId, &startPosition);
    mbMasuPosGet(endSpace, &endPosition);
    PSVECSubtract(&endPosition, &startPosition, &direction);
    originalAngle = mbObjRotYGet(models->actorModelId);
    directionAngle = 180.0 * (atan2(direction.x, direction.z) / M_PI);
    if (encounteredCount == 0) {
        models->jumping = (mbMasuAttrGet(startSpace) & 0xA) != 0;
        for (frame = 1; frame <= 24U; frame++) {
            progress = frame / 24.0f;
            if (!models->jumping) {
                PSVECSubtract(&endPosition, &startPosition, &translation);
                PSVECScale(&translation, &translation, progress);
                PSVECAdd(&startPosition, &translation, &position);
            } else {
                position.x = startPosition.x + progress * (endPosition.x - startPosition.x);
                position.y = startPosition.y + progress * (endPosition.y - startPosition.y) +
                    100.0f * (2.0f * mbSinDeg(180.0f * progress));
                position.z = startPosition.z + progress * (endPosition.z - startPosition.z);
            }
            turnProgress = frame / 24.0f;
            if (turnProgress > 1.0f) {
                turnProgress = 1.0f;
            }
            mbObjRotYSet(models->actorModelId,
                mbAngleEaseOut(originalAngle, directionAngle, turnProgress));
            mbObjPosSetV(models->actorModelId, &position);
            mbDiceSNpcNumPosSet(models->diceNumberObject, &position);
            if ((frame - 1) % 30 == 0) {
                omVibrate(models->playerNo, 20, 4, 4);
            }
            HuPrcVSleep();
        }
        if (models->jumping) {
            models->landed = TRUE;
            models->jumping = FALSE;
        }
    } else {
        models->attacking = TRUE;
        protectedCount = 0;
        for (frame = 0; frame < encounteredCount; frame++) {
            if ((capsuleIndex = mbPlayerCapsuleFind(encounteredPlayers[frame], 30)) >= 0) {
                protectedPlayers[protectedCount].playerNo = encounteredPlayers[frame];
                protectedPlayers[protectedCount].capsuleIndex = capsuleIndex;
                protectedCount++;
            }
        }
        if (protectedCount == 0) {
            HuPrcSleep(6);
            models->jumping = TRUE;
            for (frame = 1; frame <= 12U; frame++) {
                progress = frame / 12.0f;
                position.x = startPosition.x + progress * (endPosition.x - startPosition.x);
                position.y = startPosition.y + progress * (endPosition.y - startPosition.y) +
                    100.0f * (3.0f * mbSinDeg(90.0f * progress));
                position.z = startPosition.z + progress * (endPosition.z - startPosition.z);
                mbObjRotYSet(models->actorModelId,
                    mbAngleEaseOut(originalAngle, directionAngle, progress));
                mbObjPosSetV(models->actorModelId, &position);
                mbDiceSNpcNumPosSet(models->diceNumberObject, &position);
                HuPrcVSleep();
            }
            HuPrcSleep(30);
            for (frame = 0; frame < encounteredCount; frame++) {
                if (mbPlayerStarGet(encounteredPlayers[frame]) > 0) {
                    starChanges[encounteredPlayers[frame]] = -1;
                    models->starCount++;
                }
                lbl_1_bss_725C |= 1 << encounteredPlayers[frame];
            }
            for (frame = 1; frame <= 6U; frame++) {
                progress = frame / 6.0f;
                position.y = endPosition.y + 100.0f *
                    (3.0f * mbCosDeg(90.0f * progress));
                mbObjPosSetV(models->actorModelId, &position);
                mbDiceSNpcNumPosSet(models->diceNumberObject, &position);
                HuPrcVSleep();
            }
            mbAudFXPlay(W04_RIDE_ATTACK_SOUND);
            omVibrate(models->playerNo, 20, 7, 3);
            for (frame = 0; frame < encounteredCount; frame++) {
                omVibrate(encounteredPlayers[frame], 20, 7, 3);
            }
            models->landed = TRUE;
            mbAudFXPlay(MSM_SE_BRD00_83);
            mbStarAddAllProcExecV(starChanges, starChanges, TRUE);
            models->jumping = FALSE;
            models->attacking = FALSE;
            return;
        }
        defender = &protectedPlayers[mbRandMod(protectedCount)];
        mbPlayerPosGet(defender->playerNo, &defenderPosition);
        PSVECSubtract(&startPosition, &defenderPosition, &direction);
        mbObjMotionShiftSet(models->actorModelId, 1, 0.0f, 8.0f,
            HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP);
        mbPlayerRotateStart(defender->playerNo,
            180.0 * (atan2(direction.x, direction.z) / M_PI), 12);
        while (!mbPlayerRotateCheck(defender->playerNo)) {
            HuPrcVSleep();
        }
        mbObjDispSet(models->capsuleModelId, TRUE);
        mbObjPosSet(models->capsuleModelId, defenderPosition.x,
            250.0f + defenderPosition.y, defenderPosition.z);
        for (frame = 0; frame <= 30U; frame++) {
            progress = frame / 30.0f;
            mbObjScaleSet(models->capsuleModelId, progress, progress, progress);
            HuPrcVSleep();
        }
        position.x = defenderPosition.x;
        position.y = 250.0f + defenderPosition.y;
        position.z = defenderPosition.z;
        mbCapEffUseWanWanCreate(defender->playerNo, 30, &position);
        omVibrate(defender->playerNo, 20, 7, 3);
        GwPlayer[defender->playerNo].capsuleUseNum++;
        HuPrcSleep(30);
        for (frame = 0; frame <= 30U; frame++) {
            progress = 1.0f - frame / 30.0f;
            mbObjScaleSet(models->capsuleModelId, progress, progress, progress);
            HuPrcVSleep();
        }
        mbObjDispSet(models->capsuleModelId, FALSE);
        mbPlayerMotionShiftSet(defender->playerNo, 4, 0.0f, 4.0f, HU3D_MOTATTR_NONE);
        mbAudFXPlay(MSM_SE_BRD00_84);
        HuPrcSleep(12);
        nextSpace = W04LinkNext(endSpace);
        if (!(mbMasuMAttrGet(nextSpace) & W04_RIDE_CAMERA_KEEP_MASK)) {
            mbMasuPosGet(nextSpace, &position);
            PSVECSubtract(&position, &endPosition, &direction);
            PSVECScale(&direction, &direction, 0.5f);
            PSVECAdd(&endPosition, &direction, &endPosition);
        } else {
            mbMasuPosGet(nextSpace, &endPosition);
        }
        mbObjDispSet(models->effectModelId, TRUE);
        mbObjRotSet(models->effectModelId, 0.0f, 0.0f, 0.0f);
        mbPlayerCapsuleRemove(defender->playerNo, defender->capsuleIndex);
        mbObjMotionShiftSet(models->actorModelId, 2, 0.0f, 8.0f,
            HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP);
        models->jumping = TRUE;
        for (frame = 0; frame <= 30U; frame++) {
            progress = frame / 30.0f;
            if (frame == 24U) {
                mbObjDispSet(models->effectModelId, FALSE);
            }
            position.y = 150.0f + defenderPosition.y +
                100.0f * (2.0f * mbSinDeg(180.0f * progress));
            mbObjPosSet(models->effectModelId, defenderPosition.x, position.y, defenderPosition.z);
            mbObjRotGet(models->effectModelId, &effectRotation);
            effectRotation.x += 5.0f;
            effectRotation.y += 10.0f;
            mbObjRotSetV(models->effectModelId, &effectRotation);
            mbObjRotYSet(models->actorModelId,
                mbAngleEaseOut(originalAngle, directionAngle, progress));
            position.x = startPosition.x + progress * (endPosition.x - startPosition.x);
            position.y = startPosition.y + progress * (endPosition.y - startPosition.y) +
                100.0f * (3.0f * mbSinDeg(180.0f * progress));
            position.z = startPosition.z + progress * (endPosition.z - startPosition.z);
            mbObjPosSetV(models->actorModelId, &position);
            mbDiceSNpcNumPosSet(models->diceNumberObject, &position);
            HuPrcVSleep();
        }
        mbPlayerMotionShiftSet(defender->playerNo, 1, 0.0f, 4.0f,
            HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP);
        models->jumping = FALSE;
        models->landed = TRUE;
        models->attacking = FALSE;
    }
}

// Start an idle effect on motion frame 12 or when the event explicitly requests one.
void fn_1_438C(void)
{
    HuVecF position;
    W04EffectRequest *request = HuPrcCurrentGet()->property;
    W04EffectActivity *effect;
    int index;

    while (TRUE) {
        if ((mbObjMotionGet(request->modelId) == 2 &&
             12.0f == mbObjMotionTimeGet(request->modelId) &&
             request->suppressed == 0) || request->requested != 0) {
            for (index = 0; index < 8; index++) {
                effect = Hu3DData[lbl_1_bss_724C[index]].hookData;
                if (effect->active == 0) {
                    break;
                }
            }
            if (index < 8) {
                Hu3DModelAttrReset(lbl_1_bss_724C[index], HU3D_ATTR_DISPOFF);
                mbObjPosGet(request->modelId, &position);
                Hu3DModelPosSetV(lbl_1_bss_724C[index], &position);
            }
            request->requested = 0;
            mbAudFXPlay(MSM_SE_BRD00_80);
        }
        HuPrcVSleep();
    }
}

// Emit a spreading particle ring and slow its motion until every animation has stopped.
void fn_1_44B8(HU3D_MODEL *model, MBPARTICLE *effect, Mtx matrix)
{
    GXColor color = {255, 255, 192, 192};
    MBPARTICLEDATA *particle;
    int index;
    int activeCount;
    float angle;
    float radius;

    if (effect->mode == 0) {
        particle = effect->data;
        for (index = 0; index < effect->num; index++, particle++) {
            angle = 360.0f * frandf();
            radius = 100.0f * (0.5f * frandf());
            particle->pos.x = radius * mbCosDeg(angle);
            particle->pos.y = 0.0f;
            particle->pos.z = radius * mbSinDeg(angle);
            particle->vel.x = 100.0f * (10.0f * mbCosDeg(angle));
            particle->vel.y = 0.0f;
            particle->vel.z = 100.0f * (10.0f * mbSinDeg(angle));
            particle->scale = 200.0f + 20.0f * frandf();
            particle->animTime = 0.0f;
            particle->animSpeed = 0.5f;
            particle->animNo = 0;
            particle->dispF = TRUE;
            particle->pauseF = FALSE;
            particle->color = color;
        }
        effect->mode = 1;
    }
    activeCount = 0;
    particle = effect->data;
    for (index = 0; index < effect->num; index++, particle++) {
        if (!particle->pauseF) {
            particle->pos.x += (1.0f / 60.0f) * particle->vel.x;
            particle->pos.y += (1.0f / 60.0f) * particle->vel.y;
            particle->pos.z += (1.0f / 60.0f) * particle->vel.z;
            particle->vel.x *= 0.9f;
            particle->vel.y *= 0.9f;
            particle->vel.z *= 0.9f;
            activeCount++;
        }
    }
    if (activeCount == 0) {
        effect->mode = 0;
        Hu3DModelAttrSet(effect->modelId, HU3D_ATTR_DISPOFF);
    }
}

// Wait for each selected player to complete the held reaction animation.
void fn_1_47BC(void)
{
    int state[4] = {0, 0, 0, 0};
    void *work;
    int playerNo;

    work = HuPrcCurrentGet()->property;
    while (TRUE) {
        for (playerNo = 0; playerNo < 4; playerNo++) {
            switch (state[playerNo]) {
            case 0:
                if ((lbl_1_bss_725C & (1 << playerNo)) && state[playerNo] == 0) {
                    mbPlayerMotionSet(playerNo, 10, HU3D_MOTATTR_NONE);
                    mbPlayerMotionStartEndSet(playerNo, -1.0f, 10.0f);
                    state[playerNo] = 1;
                }
                break;
            case 1:
                if (lbl_1_bss_7260) {
                    mbPlayerMotionStartEndSet(playerNo, -1.0f,
                        mbPlayerMotionMaxTimeGet(playerNo));
                    mbPlayerMotionTimeSet(playerNo, 50.0f);
                    state[playerNo]++;
                }
                break;
            case 2:
                if (mbPlayerMotionEndCheck(playerNo)) {
                    mbPlayerMotionSet(playerNo, 1, HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP);
                    state[playerNo]++;
                }
                break;
            }
        }
        HuPrcVSleep();
    }
}

// Create a looping model at each linked space and hide these models during the day.
void fn_1_495C(void)
{
    s16 spaces[16];
    HuVecF linkedPosition;
    HuVecF position;
    W04LinkedObject *object;
    int allocationSize;
    int index;
    s16 spaceId;
    BOOL day;

    lbl_1_bss_7244 = mbMasuMAttrListGet(W04_NIGHT_LINK_MASK, spaces);
    allocationSize = lbl_1_bss_7244 * sizeof(W04LinkedObject);
    object = mbMalloc(allocationSize);
    lbl_1_bss_7248 = object;
    for (index = 0; index < lbl_1_bss_7244; index++, object++) {
        day = GwSystem.curTime == 0;
        object->modelId = mbObjCreate(
            DATANUM(day ? DATA_w04 : DATA_w04n, 31), NULL, TRUE);
        mbObjAttrSet(object->modelId, HU3D_MOTATTR_LOOP);
        if (GwSystem.curTime == 0) {
            mbObjDispSet(object->modelId, FALSE);
        }
        spaceId = spaces[index];
        mbMasuPosGet(spaceId, &position);
        object->linkedSpaceId = mbMasuMAttrFindLink(spaceId, W04_NIGHT_BRANCH_MASK);
        mbMasuPosGet(object->linkedSpaceId, &linkedPosition);
        mbObjPosSetV(object->modelId, &linkedPosition);
    }
    if (GwSystem.curTime != 0) {
        mbBranchMAttrSet(W04_NIGHT_BRANCH_MASK);
    }
}

// Free the linked-space model list when the board closes.
void fn_1_4ACC(void)
{
    if (lbl_1_bss_7248 != 0) {
        void *data = lbl_1_bss_7248;
        HuMemDirectFree(data);
        // Clear the ride-model pointer; the linked-space list pointer is left as it was.
        lbl_1_bss_726C = 0;
    }
}

// Get the model used to mark the selected linked board space.
s16 fn_1_4B28(int index)
{
    W04LinkedObject *object = &lbl_1_bss_7248[index];

    return object->modelId;
}

// Turn the player toward the linked space, show its message, and remember it was shown.
void fn_1_4B54(int playerNo, s16 spaceId)
{
    HuVecF playerPosition;
    HuVecF linkedPosition;
    HuVecF direction;
    s16 windowId;
    s16 linkedSpace;

    mbPlayerPosGet(playerNo, &playerPosition);
    linkedSpace = mbMasuMAttrFindLink(spaceId, W04_NIGHT_BRANCH_MASK);
    mbMasuPosGet(linkedSpace, &linkedPosition);
    VECSubtract(&linkedPosition, &playerPosition, &direction);
    mbPlayerRotateStart(playerNo,
        (s16)(180.0 * (atan2(direction.x, direction.z) / M_PI)), 15);
    windowId = mbWinCreate(MBWIN_TYPE_EVENT,
        MESSNUM(MESS_BOARD_W04, 20), -1);
    mbWinWait(windowId);
    lbl_1_bss_7308->linkedSpaceMessageShown = TRUE;
}

// Initialize the drifting particles and install their textured drawing hook.
void fn_1_4C48(void)
{
    W04FallingParticle *particle = lbl_1_bss_1244;
    int index;

    memset(lbl_1_bss_1244, 0, sizeof(lbl_1_bss_1244));
    for (index = 0; index < 512; index++, particle++) {
        particle->position.x = -3000.0f + 6000.0f * frandf();
        particle->position.y = 3000.0f * frandf();
        particle->position.z = -3000.0f + 6000.0f * frandf();
        particle->velocity.x = particle->velocity.y = particle->velocity.z = 0.0f;
        particle->directionDegrees = 360.0f * frandf();
        particle->directionSine = mbSinDeg(particle->directionDegrees);
        particle->directionCosine = mbCosDeg(particle->directionDegrees);
        particle->rotationRadians = 3.1415927f * (frandf() - 0.5f);
        particle->motionFactor = 0.0f;
        particle->scale = 1.0f + 0.5f * frandf();
    }
    lbl_1_bss_1240 = Hu3DHookFuncCreate(fn_1_53E4);
    Hu3DModelCameraSet(lbl_1_bss_1240, HU3D_CAM0);
    Hu3DModelLayerSet(lbl_1_bss_1240, 5);
    lbl_1_bss_1238 = fn_1_57D8(&lbl_1_bss_123C);
    lbl_1_bss_1234 = HuSprAnimRead(HuDataSelHeapReadNum(
        DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 40), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprAnimLock(lbl_1_bss_1234);
    lbl_1_bss_1230 = 1.0f;
    lbl_1_bss_122C = TRUE;
}

// Release the effect's particle storage, sprite animation, and model.
void fn_1_4EA8(void)
{
    if (lbl_1_bss_123C != 0) {
        void *particles = lbl_1_bss_123C;

        HuMemDirectFree(particles);
        lbl_1_bss_123C = 0;
    }
    if (lbl_1_bss_1234 != 0) {
        HuSprAnimKill(lbl_1_bss_1234);
        lbl_1_bss_1234 = 0;
    }
    if (lbl_1_bss_1240 >= 0) {
        Hu3DModelKill(lbl_1_bss_1240);
        lbl_1_bss_1240 = -1;
    }
}

// Advance the drifting particles, recycle low ones, and wrap their positions around the camera.
void fn_1_4F6C(void)
{
    HuVecF targetVelocity;
    HuVecF displacement;
    MBCAMERA *camera;
    W04FallingParticle *particle;
    int index;
    float angleDegrees;
    float sine;
    float acceleration;
    float speed;

    camera = mbCameraGet();
    particle = lbl_1_bss_1244;
    if (!camera->dispOn) {
        return;
    }
    for (index = 0; index < 512; index++, particle++) {
        if (particle->position.y < 0.0f || fn_1_5A20(&particle->position)) {
            particle->position.y = 3000.0f;
            particle->velocity.x = particle->velocity.y = particle->velocity.z = 0.0f;
            particle->rotationRadians = 3.1415927f * (2.0f * frandf() - 1.0f);
        }
        if (1500.0f + camera->eye.x < particle->position.x) {
            particle->position.x = (-1500.0f + camera->eye.x) +
                fmod(particle->position.x - (1500.0f + camera->eye.x), 3000.0);
        } else if (-1500.0f + camera->eye.x > particle->position.x) {
            particle->position.x = (1500.0f + camera->eye.x) +
                fmod(particle->position.x - (-1500.0f + camera->eye.x), 3000.0);
        }
        if (camera->eye.z < particle->position.z) {
            particle->position.z = (-3000.0f + camera->eye.z) +
                fmod(particle->position.z - camera->eye.z, 3000.0);
        } else if (-3000.0f + camera->eye.z > particle->position.z) {
            particle->position.z = camera->eye.z +
                fmod(particle->position.z - (-3000.0f + camera->eye.z), 3000.0);
        }
        acceleration = 0.01f * (-980.0f * mbSinRad(particle->rotationRadians));
        particle->motionFactor += (1.0f / 60.0f) * acceleration;
        particle->rotationRadians += (1.0f / 60.0f) * particle->motionFactor;
        angleDegrees = 57.29578f * particle->rotationRadians;
        sine = mbSinDeg(angleDegrees);
        targetVelocity.x = particle->directionSine * sine;
        targetVelocity.y = -mbCosDeg(angleDegrees);
        targetVelocity.z = particle->directionCosine * sine;
        speed = 100.0f * (particle->motionFactor * particle->motionFactor);
        PSVECScale(&targetVelocity, &targetVelocity, speed);
        particle->velocity.x += (1.0f / 60.0f) * (targetVelocity.x - particle->velocity.x);
        particle->velocity.y += (1.0f / 60.0f) *
            ((-980.0f + targetVelocity.y) - particle->velocity.y);
        particle->velocity.z += (1.0f / 60.0f) * (targetVelocity.z - particle->velocity.z);
        PSVECScale(&particle->velocity, &displacement, (1.0f / 60.0f));
        PSVECAdd(&particle->position, &displacement, &particle->position);
        particle->motionFactor *= 0.998f;
        PSVECScale(&particle->velocity, &particle->velocity, 0.95f);
    }
}

// Draw drifting particles as camera-facing textured quads, fading those near the camera.
void fn_1_53E4(HU3D_MODEL *model, Mtx *matrix)
{
    Mtx particleMatrix;
    HuVecF cameraPosition;
    GXColor color = {255, 255, 255, 255};
    W04FallingParticle *particle = lbl_1_bss_1244;
    int index;

    if (!lbl_1_bss_122C) {
        return;
    }
    HuSprTexLoad(lbl_1_bss_1234, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
        GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX,
        GX_LIGHT0, GX_DF_CLAMP, GX_AF_SPOT);
    GXSetZCompLoc(GX_FALSE);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetCullMode(GX_CULL_BACK);
    GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    for (index = 0; index < 512; index++, particle++) {
        PSMTXMultVec(*matrix, &particle->position, &cameraPosition);
        PSMTXScale(particleMatrix, particle->scale, particle->scale, 1.0f);
        mtxTransCat(particleMatrix, cameraPosition.x, cameraPosition.y, cameraPosition.z);
        GXLoadPosMtxImm(particleMatrix, GX_PNMTX0);
        if (cameraPosition.z <= 0.0f && cameraPosition.z > -1500.0f) {
            color.a = (255.0f * (cameraPosition.z / -1500.0f)) * lbl_1_bss_1230;
        } else {
            color.a = 255.0f * lbl_1_bss_1230;
        }
        GXSetTevColor(GX_TEVREG0, color);
        GXCallDisplayList(lbl_1_bss_123C, lbl_1_bss_1238);
    }
}

// Build the particle quad once and retain a display list sized to its commands.
u32 fn_1_57D8(void **displayList)
{
    void *buffer;
    void *listBuffer;
    void *list;
    u32 size;

    buffer = HuMemDirectMallocNum(HEAP_HEAP, 8192, HU_MEMNUM_OVL);
    listBuffer = buffer;
    DCInvalidateRange(listBuffer, 8192);
    GXBeginDisplayList(listBuffer, 8192);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-12.0f, 12.0f, 0.0f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(12.0f, 12.0f, 0.0f);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition3f32(12.0f, -12.0f, 0.0f);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(-12.0f, -12.0f, 0.0f);
    GXTexCoord2f32(0.0f, 1.0f);
    size = GXEndDisplayList();
    list = HuMemDirectMallocNum(HEAP_HEAP, size, HU_MEMNUM_OVL);
    *displayList = list;
    memcpy(*displayList, listBuffer, size);
    DCFlushRange(*displayList, size);
    HuMemDirectFree(listBuffer);
    return size;
}

HuVecF lbl_1_data_578[3] = {
    { 0.0f, 350.0f, 0.0f },
    { 1300.0f, 250.0f, 1450.0f },
    { 3550.0f, 650.0f, 300.0f }
};

float lbl_1_data_59C[3] = { 490000.0f, 160000.0f, 90000.0f };

W04GridPoint lbl_1_data_5A8[8] = { { -1, -1 }, { 0, -1 }, { 1, -1 }, { 1, 0 },
                                   { 1, 1 },   { 0, 1 },  { -1, 1 }, { -1, 0 } };

HuVecF lbl_1_data_5E8 = { 0.0f, 350.0f, 0.0f };

HuVecF lbl_1_data_5F4 = { 0.0f, -100.0f, -50.0f };

HuVecF lbl_1_data_600 = { -45.0f, 0.0f, 0.0f };

int lbl_1_data_60C[3] = {
    DATANUM(DATA_mariomot, 16), DATANUM(DATA_mario, 21), DATANUM(DATA_mariomot, 94)
};

HuVecF lbl_1_data_618 = { 0.0f, 3300.0f, 10.0f };

HuVecF lbl_1_data_624 = { 0.0f, 1.0f, 0.0f };

HuVecF lbl_1_data_630 = { 0.0f, 0.0f, 0.0f };

float lbl_1_data_63C[4] = { 300.0f, 250.0f, 200.0f, 150.0f };

HuVecF lbl_1_data_64C = { 0.0f, -100.0f, -50.0f };

HuVecF lbl_1_data_658 = { -45.0f, 0.0f, 0.0f };

int lbl_1_data_664[3] = {
    DATANUM(DATA_mariomot, 16), DATANUM(DATA_mario, 21), DATANUM(DATA_mariomot, 74)
};

BOOL lbl_1_data_670[4] = { 1, 1, 1, 1 };

HuVecF lbl_1_data_680 = { 0.0f, 3300.0f, 10.0f };

HuVecF lbl_1_data_68C = { 0.0f, 1.0f, 0.0f };

HuVecF lbl_1_data_698 = { 0.0f, 0.0f, 0.0f };

HuVecF lbl_1_data_6A4[4] = {
    { -2128.0f, 905.0f, -188.0f },
    { -1488.0f, 1070.0f, -509.0f },
    { 859.0f, 1507.0f, -1160.0f },
    { 3693.0f, 1113.0f, -511.0f }
};

int lbl_1_data_6D4[2] = { DATANUM(DATA_w04, 15), -1 };

int lbl_1_data_6DC[2] = { DATANUM(DATA_w04n, 15), -1 };

// Return whether the position is below a region's ceiling and inside its radius.
BOOL fn_1_5A20(HuVecF *position)
{
    int region;
    float deltaX;
    float deltaZ;

    for (region = 0; region < 3; region++) {
        if (position->y > lbl_1_data_578[region].y) {
            continue;
        }
        deltaX = position->x - lbl_1_data_578[region].x;
        deltaZ = position->z - lbl_1_data_578[region].z;
        if (deltaX * deltaX + deltaZ * deltaZ < lbl_1_data_59C[region]) {
            return TRUE;
        }
    }
    return FALSE;
}

// Set the weather's opacity multiplier for the turn and board-scroll hooks.
void fn_1_5AF4(float opacity)
{
    lbl_1_bss_1230 = opacity;
}

// Show or hide the weather during turns, board-map display, and event scenes.
void fn_1_5B04(BOOL visible)
{
    lbl_1_bss_122C = visible;
}

// Create two hidden models, then select the marked space with the smallest x coordinate.
void fn_1_5B14(void)
{
    s16 spaceIds[32];
    HuVecF position;
    W04MovingModels *models = &lbl_1_bss_1218;
    int spaceCount;
    int index;
    float minimumX;

    models->primaryModelId = mbObjCreate(
        DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 16), NULL, TRUE);
    mbObjDispSet(models->primaryModelId, FALSE);
    models->secondaryModelId = mbObjCreate(
        DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 17), NULL, TRUE);
    mbObjDispSet(models->secondaryModelId, FALSE);
    minimumX = 9999.0f;
    spaceCount = mbMasuMAttrListGet(W04_ROLLING_ROUTE_MASK, spaceIds);
    for (index = 0; index < spaceCount; index++) {
        mbMasuPosGet(spaceIds[index], &position);
        if (minimumX > position.x) {
            lbl_1_bss_1214 = spaceIds[index];
            minimumX = position.x;
        }
    }
}

// Move the rolling model through the board path, then show its ending animation.
void fn_1_5C64(int playerNo, s16 spaceId)
{
    Mtx modelMatrix;
    Mtx rotationMatrix;
    HuVecF nextPosition;
    HuVecF rotationAxis;
    W04MovingModels *models = &lbl_1_bss_1218;
    int frame;
    HUPROCESS *sequenceProcess;
    HUPROCESS *currentProcess;
    int rollingSound;
    float progress;
    float distance;
    float negativeX;
    float rotationRadians;

    currentProcess = HuPrcCurrentGet();
    fn_1_D7C8();
    sequenceProcess = HuPrcChildCreate(fn_1_6064, currentProcess->prio,
        24576, 0, mbMainProc);
    sequenceProcess->property = models;
    mbCameraMoveMasu(lbl_1_bss_1214, NULL, NULL, 3000.0f, -1.0f, -1);
    mbObjDispSet(models->primaryModelId, TRUE);
    models->state = 0;
    mbMasuPlayerFadeSet(TRUE);
    mbWipeDissolveFadeIn();
    for (frame = 0; frame < 24U; frame++) {
        progress = frame / 24.0f;
        models->position.x = (lbl_1_data_F0[0].x - 500.0f) + 100.0f * (5.0f * progress);
        models->position.y = lbl_1_data_F0[0].y + 800.0f * mbCosDeg(90.0f * progress);
        models->position.z = lbl_1_data_F0[0].z;
        mbObjPosSetV(models->primaryModelId, &models->position);
        HuPrcVSleep();
    }
    models->state = 1;
    mbAudFXPlay(W04_ROLLING_START_SOUND);
    rollingSound = mbAudFXPlay(W04_ROLLING_SOUND);
    mbCameraShakeSet(12, 400.0f);
    mbCameraMoveObj(models->primaryModelId, NULL, NULL, -1.0f, -1.0f, 60);
    for (frame = 1; frame <= 480U; frame++) {
        progress = frame / 480.0f;
        fn_1_F9D0(lbl_1_data_F0, 32, &nextPosition, progress);
        PSVECSubtract(&nextPosition, &models->position, &rotationAxis);
        distance = PSVECMag(&rotationAxis);
        if (distance > 0.0f) {
            rotationRadians = distance / 175.0f;
            negativeX = -rotationAxis.x;
            rotationAxis.x = rotationAxis.z;
            rotationAxis.z = negativeX;
            rotationAxis.y = 0.0f;
            PSMTXRotAxisRad(rotationMatrix, &rotationAxis, rotationRadians);
            mbObjMtxGet(models->primaryModelId, &modelMatrix);
            PSMTXConcat(rotationMatrix, modelMatrix, modelMatrix);
            mbObjMtxSet(models->primaryModelId, &modelMatrix);
        }
        models->position = nextPosition;
        mbObjPosSetV(models->primaryModelId, &models->position);
        HuPrcVSleep();
    }
    mbAudFXStop(rollingSound);
    models->state = 2;
    mbAudFXPlay(W04_ROLLING_START_SOUND);
    mbCameraShakeSet(12, 400.0f);
    mbObjDispSet(models->primaryModelId, FALSE);
    mbObjMotionTimeSet(models->secondaryModelId, 0.0f);
    mbObjPosGet(models->primaryModelId, &nextPosition);
    mbObjPosSetV(models->secondaryModelId, &nextPosition);
    mbObjDispSet(models->secondaryModelId, TRUE);
    mbCameraPlayerViewSet(playerNo, MB_CAMERA_VIEW_WALK);
    HuPrcSleep(120);
    HuPrcKill(sequenceProcess);
    mbObjDispSet(models->secondaryModelId, FALSE);
}

// Attach affected players to the rolling model, then launch them toward its exit space.
void fn_1_6064(void)
{
    W04RollingPlayer players[4];
    HuVecF exitPositions[4];
    Mtx modelMatrix;
    Mtx inverseMatrix;
    Mtx playerMatrix;
    int playerIds[4];
    HuVecF worldPosition;
    HuVecF playerPosition;
    HuVecF modelPosition;
    HuVecF separation;
    int playerCount;
    W04RollingPlayer *player;
    W04MovingModels *models;
    int playerNo;
    int frame = 0;
    s16 exitSpace;
    float progress;
    float distance;

    models = HuPrcCurrentGet()->property;
    memset(players, 0, sizeof(players));
    exitSpace = mbMasuFind_AttrIdGet(-1, MASU_FLAG_START);
    PSMTXRotRad(modelMatrix, 'y', -1.5707964f);
    player = players;
    playerCount = 0;
    for (playerNo = 0; playerNo < 4; playerNo++, player++) {
        if ((mbMasuMAttrGet(GwPlayer[playerNo].masuId) & 0x4) != 0) {
            player->active = TRUE;
            mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
            mbPlayerMtxSet(playerNo, &modelMatrix);
            mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
            mbPlayerColSnapPlayerSet(playerNo, FALSE);
            playerIds[playerNo] = playerNo;
        } else {
            player->active = FALSE;
            playerIds[playerNo] = -1;
        }
    }
    mbev_PlayerColBall(exitSpace, playerIds, exitPositions);
    for (;;) {
        player = players;
        for (playerNo = 0; playerNo < 4; playerNo++, player++) {
            if (player->active) {
                switch (player->state) {
                case 0:
                    mbPlayerPosGet(playerNo, &playerPosition);
                    PSVECSubtract(&models->position, &playerPosition, &separation);
                    separation.y = 0.0f;
                    distance = PSVECMag(&separation);
                    if (distance < 800.0f) {
                        player->state = 1;
                        mbPlayerMotionShiftSet(playerNo, 9, 0.0f, 4.0f, 0);
                    }
                    break;
                case 1:
                    if (mbPlayerMotionGet(playerNo) == 9 && mbPlayerMotionEndCheck(playerNo)) {
                        mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
                    }
                    if (models->state == 1) {
                        mbPlayerPosGet(playerNo, &playerPosition);
                        PSVECSubtract(&models->position, &playerPosition, &separation);
                        separation.y = 0.0f;
                        distance = PSVECMag(&separation);
                        if (distance < 150.0f) {
                            player->state = 2;
                            mbObjPosGet(models->primaryModelId, &modelPosition);
                            mbObjMtxGet(models->primaryModelId, &modelMatrix);
                            PSVECSubtract(&playerPosition, &modelPosition, &worldPosition);
                            PSMTXInverse(modelMatrix, inverseMatrix);
                            PSMTXMultVec(inverseMatrix, &worldPosition, &separation);
                            distance = PSVECMag(&separation);
                            PSVECNormalize(&separation, &separation);
                            player->position.x = 195.0f * separation.x;
                            player->position.y = 195.0f * separation.y;
                            player->position.z = 195.0f * separation.z;
                            mbObjMtxGet(mbPlayerObjIDGet(playerNo), &playerMatrix);
                            PSMTXConcat(inverseMatrix, playerMatrix, inverseMatrix);
                            PSMTXCopy(inverseMatrix, player->rotation);
                            CharModelVoiceFlagSet(GwPlayer[playerNo].charNo, FALSE);
                            CharMotionVoiceOnSet(GwPlayer[playerNo].charNo, 2, FALSE);
                            mbPlayerMotionShiftSet(playerNo, 3, 0.0f, 4.0f, HU3D_MOTATTR_LOOP);
                            CharFXPlay(GwPlayer[playerNo].charNo, 588);
                        }
                    }
                    break;
                case 2:
                    mbObjMtxGet(models->primaryModelId, &modelMatrix);
                    PSMTXMultVec(modelMatrix, &player->position, &separation);
                    mbObjPosGet(models->primaryModelId, &modelPosition);
                    PSVECAdd(&modelPosition, &separation, &worldPosition);
                    mbPlayerPosSetV(playerNo, &worldPosition);
                    PSMTXConcat(modelMatrix, player->rotation, playerMatrix);
                    mbPlayerMtxSet(playerNo, &playerMatrix);
                    if ((frame & 0x1E) == 0) {
                        omVibrate(playerNo, 20, 7, 3);
                    }
                    if (models->state == 2) {
                        player->state = 3;
                        omVibrate(playerNo, 20, 20, 0);
                        GwPlayer[playerNo].masuIdNext = exitSpace;
                        PSMTXIdentity(modelMatrix);
                        mbPlayerMtxSet(playerNo, &modelMatrix);
                        mbPlayerPosGet(playerNo, &player->position);
                        player->elapsedFrames = 0;
                        player->durationFrames = 54;
                        mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
                    }
                    break;
                case 3:
                    progress = (float)player->elapsedFrames++ / player->durationFrames;
                    worldPosition.x = player->position.x +
                        progress * (exitPositions[playerNo].x - player->position.x);
                    worldPosition.y = player->position.y +
                        progress * (exitPositions[playerNo].y - player->position.y) +
                        700.0f * mbSinDeg(180.0f * progress);
                    worldPosition.z = player->position.z +
                        progress * (exitPositions[playerNo].z - player->position.z);
                    mbPlayerPosSetV(playerNo, &worldPosition);
                    mbPlayerRotSet(playerNo, 180.0f * progress, 0.0f, 0.0f);
                    PSMTXRotRad(modelMatrix, 'y', 0.017453292f * (1440.0f * progress));
                    mbPlayerMtxSet(playerNo, &modelMatrix);
                    if (player->elapsedFrames > player->durationFrames) {
                        player->state = -1;
                        mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
                        PSMTXIdentity(modelMatrix);
                        mbPlayerMtxSet(playerNo, &modelMatrix);
                        mbPlayerMotionSet(playerNo, 6, HU3D_MOTATTR_LOOP);
                        GwPlayer[playerNo].masuId = exitSpace;
                        CharModelVoiceFlagSet(GwPlayer[playerNo].charNo, TRUE);
                        CharMotionVoiceOnSet(GwPlayer[playerNo].charNo, 2, TRUE);
                    }
                    break;
                }
            }
        }
        frame++;
        HuPrcVSleep();
    }
}

// Mark grid cells inside the event boundary before searching player movement routes.
void fn_1_67D8(void)
{
    HuVecF position;
    int row;
    int column;
    float distance;

    position.z = 0.0f;
    for (row = 0; row < 12; row++) {
        for (column = 0; column < 12; column++) {
            position.x = 65.0 + (-650.0 + 130.0 * (column - 1));
            position.y = 65.0 + (-650.0 + 130.0 * (row - 1));
            distance = PSVECMag(&position);
            if (distance < 650.0) {
                lbl_1_bss_D94[row][column].cost = 0.0f;
            } else {
                lbl_1_bss_D94[row][column].cost = -1.0f;
            }
        }
    }
}

// Fill the reachable cells' step distances from the supplied board-grid position.
void fn_1_6980(W04GridPoint *start)
{
    W04GridPoint queue[100];
    W04GridPoint next;
    int row;
    int column;
    s16 readIndex;
    s16 writeIndex;
    int nextDistance;

    for (row = 0; row < 12; row++) {
        for (column = 0; column < 12; column++) {
            lbl_1_bss_D94[row][column].distance = 0;
        }
    }
    queue[0] = *start;
    lbl_1_bss_D94[start->y][start->x].distance = 1;
    readIndex = 0;
    writeIndex = 1;
    do {
        nextDistance = lbl_1_bss_D94[queue[readIndex].y][queue[readIndex].x].distance + 1;
        for (row = 0; row < 8; row++) {
            next.x = queue[readIndex].x + lbl_1_data_5A8[row].x;
            next.y = queue[readIndex].y + lbl_1_data_5A8[row].y;
            if (lbl_1_bss_D94[next.y][next.x].distance == 0 &&
                lbl_1_bss_D94[next.y][next.x].cost >= 0.0f) {
                lbl_1_bss_D94[next.y][next.x].distance = nextDistance;
                queue[writeIndex++] = next;
            }
        }
        readIndex++;
    } while (readIndex != writeIndex);
}

// Choose the cheapest nearby reachable cell and discard routes at its distance or farther.
BOOL fn_1_6BB0(W04GridPoint *goal)
{
    W04GridPoint selected;
    int row;
    int column;
    int selectedDistance;
    float minimumCost = 9999.0f;

    for (row = 1; row < 11; row++) {
        for (column = 1; column < 11; column++) {
            if (lbl_1_bss_D94[row][column].distance != 0 &&
                lbl_1_bss_D94[row][column].distance < 5 &&
                lbl_1_bss_D94[row][column].cost >= 0.0f &&
                lbl_1_bss_D94[row][column].cost < minimumCost) {
                minimumCost = lbl_1_bss_D94[row][column].cost;
                selected.x = column;
                selected.y = row;
            }
        }
    }
    if (minimumCost == 9999.0f) {
        return FALSE;
    }
    selectedDistance = lbl_1_bss_D94[selected.y][selected.x].distance;
    for (row = 1; row < 11; row++) {
        for (column = 1; column < 11; column++) {
            if (lbl_1_bss_D94[row][column].distance >= selectedDistance) {
                lbl_1_bss_D94[row][column].distance = 0;
            }
        }
    }
    lbl_1_bss_D94[selected.y][selected.x].distance = selectedDistance;
    *goal = selected;
    return TRUE;
}

// Choose the first step of a shortest route that minimizes the largest cell cost.
void fn_1_6DC0(W04GridPoint *start, W04GridPoint *goal, W04GridPoint *nextStep)
{
    W04GridPoint pending[100];
    W04GridPoint path[10];
    W04GridPoint bestPath[10];
    W04GridPoint next;
    W04GridPoint current;
    int pendingCount;
    int goalDistance;
    int index;
    float bestCost;
    float maximumCost;

    pending[0] = *start;
    pendingCount = 1;
    bestCost = 1.0f;
    goalDistance = lbl_1_bss_D94[goal->y][goal->x].distance;
    do {
        current = pending[--pendingCount];
        path[lbl_1_bss_D94[current.y][current.x].distance - 1] = current;
        if (current.x == goal->x && current.y == goal->y) {
            maximumCost = 0.0f;
            for (index = 1; index < goalDistance; index++) {
                if (lbl_1_bss_D94[path[index].y][path[index].x].cost > maximumCost) {
                    maximumCost = lbl_1_bss_D94[path[index].y][path[index].x].cost;
                }
            }
            if (maximumCost < bestCost) {
                bestCost = maximumCost;
                memcpy(bestPath, path, goalDistance * sizeof(W04GridPoint));
            }
        } else {
            for (index = 0; index < 8; index++) {
                next.x = current.x + lbl_1_data_5A8[index].x;
                next.y = current.y + lbl_1_data_5A8[index].y;
                if (lbl_1_bss_D94[current.y][current.x].distance + 1 ==
                    lbl_1_bss_D94[next.y][next.x].distance) {
                    pending[pendingCount++] = next;
                }
            }
        }
    } while (pendingCount != 0);
    nextStep->x = bestPath[1].x;
    nextStep->y = bestPath[1].y;
}

void fn_1_707C(void)
{
    fn_1_9878();
}

// Start the timed event and settle each player's or team's coin change when play ends.
void fn_1_709C(int playerNo, s16 spaceId)
{
    HuVecF awardPosition;
    HuVecF gridPosition;
    int teamCounts[2];
    int displayIds[2];
    HUPROCESS *process;
    HU3D_LIGHT *light;
    W04EventPlayer *player;
    W04CoinState *coin;
    W04Projectile *projectile;
    int index;
    int motionIndex;
    int column;
    int row;
    int remainingFrames;
    int teamNo;
    int pendingDisplays;
    s16 windowId;
    s16 helpWindowId;
    GAMEMESID timerId;
    float distance;

    process = HuPrcCurrentGet();
    mbWipeSpecialFadeInCreate(1, 1);
    light = Hu3DGlobalLight;
    light->color.r = light->color.g = light->color.b = 255;
    lbl_1_bss_1230 = 0.5f;
    mbStatusDispForceSetAll(FALSE);
    mbCameraMovePos(&lbl_1_data_5E8, &lbl_1_data_600, &lbl_1_data_5F4,
        2800.0f, -1.0f, -1);
    memset(lbl_1_bss_C50, 0, sizeof(lbl_1_bss_C50));
    Hu3DShadowCreate(30.0f, 20.0f, 5000.0f);
    Hu3DShadowTPLvlSet(0.5f);
    Hu3DShadowPosSet(&lbl_1_data_618, &lbl_1_data_624, &lbl_1_data_630);
    Hu3DShadowColSet(16, 0, 32);
    mbObjShadowMapSet(lbl_1_bss_7280.shadowModelId);
    player = lbl_1_bss_C50;
    for (index = 0; index < 4; index++, player++) {
        mbPlayerColSnapPlayerSet(index, FALSE);
        player->facingDegrees = 0.0f;
        mbPlayerRotYSet(index, player->facingDegrees);
        player->position.x = lbl_1_data_5E8.x - 300.0f + 600.0f * (index % 2);
        player->position.y = lbl_1_data_5E8.y;
        player->position.z = lbl_1_data_5E8.z - 300.0f + 600.0f * (index / 2);
        mbPlayerPosSetV(index, &player->position);
        player->targetPlayerNo = -1;
        for (motionIndex = 0; motionIndex < 3U; motionIndex++) {
            player->motions[motionIndex] = mbPlayerMotionCreate(index,
                lbl_1_data_60C[motionIndex]);
        }
        mbObjShadowSet(mbPlayerObjIDGet(index));
    }
    if (!GWTeamFGet()) {
        for (index = 0; index < 4; index++) {
            lbl_1_bss_D80[index] = mbPlayerCoinGet(index);
        }
    } else {
        for (index = 0; index < 2; index++) {
            lbl_1_bss_D80[index] = mbPlayerTeamCoinGet(index);
        }
    }
    projectile = lbl_1_bss_950;
    for (index = 0; index < 16; index++, projectile++) {
        projectile->active = FALSE;
        mbObjShadowSet(projectile->movingModelId);
    }
    memset(lbl_1_bss_590, 0, sizeof(lbl_1_bss_590));
    coin = lbl_1_bss_590;
    for (index = 0; index < 16; index++, coin++) {
        coin->active = FALSE;
        coin->coinObjectId = mbCoinCreate2();
        coin->groundMarkerId = mbObjCreate(
            DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 21), NULL, TRUE);
        mbObjLayerSet(coin->groundMarkerId, 3);
        mbCoinObjDispSet(coin->coinObjectId, FALSE);
        mbObjDispSet(coin->groundMarkerId, FALSE);
    }
    gridPosition.z = 0.0f;
    for (row = 0; row < 12; row++) {
        for (column = 0; column < 12; column++) {
            gridPosition.x = 65.0 + (-650.0 + 130.0 * (column - 1));
            gridPosition.y = 65.0 + (-650.0 + 130.0 * (row - 1));
            distance = PSVECMag(&gridPosition);
            if (distance < 650.0) {
                lbl_1_bss_D94[row][column].cost = 0.0f;
            } else {
                lbl_1_bss_D94[row][column].cost = -1.0f;
            }
        }
    }
    mbWipeSpecialFadeOutCreate(1, 30);
    windowId = mbWinCreate(2, MESSNUM(MESS_BOARD_W04, 11), 7);
    mbWinPlayerDisable(windowId, -1);
    mbAudGuidePlay(MSM_SE_GUIDE_25);
    mbWinWait(windowId);
    windowId = mbWinCreate(2, MESSNUM(MESS_BOARD_W04, 12), 7);
    mbWinPlayerDisable(windowId, -1);
    mbAudGuidePlay(MSM_SE_GUIDE_26);
    mbWinWait(windowId);
    windowId = mbWinCreate(2, MESSNUM(MESS_BOARD_W04, 13), 7);
    mbAudGuidePlay(MSM_SE_GUIDE_25);
    mbWinPause(windowId);
    HuPrcSleep(90);
    mbWinKill(windowId);
    helpWindowId = mbWinCreateHelp(MESSNUM(MESS_BOARD_W04, 14));
    mbWinPosSet(helpWindowId, 192, 376);
    lbl_1_bss_D90 = FALSE;
    timerId = GameMesCreate(1, 15, -1, -1);
    HuSprGrpDrawNoSet(GameMesGet(timerId)->grpId[0], 32);
    remainingFrames = 900;
    for (index = 0; index < 1080U; index++) {
        for (motionIndex = 0; motionIndex < 4; motionIndex++) {
            fn_1_8004(motionIndex);
        }
        fn_1_99BC();
        fn_1_9DFC();
        fn_1_9578();
        fn_1_A170();
        fn_1_AA44();
        player = lbl_1_bss_C50;
        for (motionIndex = 0; motionIndex < 4; motionIndex++, player++) {
            mbPlayerPosSetV(motionIndex, &player->position);
            mbPlayerRotYSet(motionIndex, player->facingDegrees);
        }
        if (remainingFrames >= 0) {
            remainingFrames--;
            if (remainingFrames >= 0) {
                GameMesDispSet(timerId, 1, (remainingFrames + 59) / 60);
            } else {
                lbl_1_bss_D90 = TRUE;
                GameMesDispSet(timerId, 2, -1);
            }
        }
        HuPrcVSleep();
    }
    mbWipeDissolveFadeOut();
    mbWinKill(helpWindowId);
    player = lbl_1_bss_C50;
    teamCounts[0] = teamCounts[1] = 0;
    for (index = 0; index < 4; index++, player++) {
        mbPlayerRotYSet(index, 0.0f);
        if (!GWTeamFGet()) {
            player->position.x = lbl_1_data_5E8.x - 300.0f + 600.0f * (index % 2);
            player->position.y = lbl_1_data_5E8.y;
            player->position.z = lbl_1_data_5E8.z - 300.0f + 600.0f * (index / 2);
        } else {
            teamNo = mbPlayerGrpGet(index);
            if (teamNo == 0) {
                player->position.x = lbl_1_data_5E8.x - 400.0f + 200.0f * teamCounts[teamNo];
            } else {
                player->position.x = 200.0f + lbl_1_data_5E8.x + 200.0f * teamCounts[teamNo];
            }
            player->position.y = lbl_1_data_5E8.y;
            player->position.z = lbl_1_data_5E8.z;
            teamCounts[teamNo]++;
        }
        mbPlayerPosSetV(index, &player->position);
        mbPlayerMotionSet(index, 1, HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP);
        mbPlayerDispSet(index, TRUE);
    }
    projectile = lbl_1_bss_950;
    for (index = 0; index < 16; index++, projectile++) {
        mbObjShadowReset(projectile->movingModelId);
    }
    coin = lbl_1_bss_590;
    for (index = 0; index < 16; index++, coin++) {
        mbCoinObjNumDec(coin->coinObjectId);
        mbObjKill(coin->groundMarkerId);
    }
    mbWipeDissolveFadeIn();
    HuPrcSleep(30);
    if (!GWTeamFGet()) {
        for (index = 0; index < 4; index++) {
            lbl_1_bss_570[index] = lbl_1_bss_D80[index] - mbPlayerCoinGet(index);
            if (lbl_1_bss_570[index] >= 0) {
                mbPlayerMotionShiftSet(index, 7, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
                lbl_1_bss_580[index] = 1;
                CharFXPlay(GwPlayer[index].charNo, CHARVOICEID(2));
            } else {
                mbPlayerMotionShiftSet(index, 8, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
                lbl_1_bss_580[index] = -1;
                CharFXPlay(GwPlayer[index].charNo, CHARVOICEID(12));
            }
        }
        mbCoinAddAllProcExecV(lbl_1_bss_570, lbl_1_bss_580, TRUE);
    } else {
        mbAudFXPlay(MSM_SE_CMN_16);
        for (index = 0; index < 2; index++) {
            lbl_1_bss_570[index] = lbl_1_bss_D80[index] - mbPlayerTeamCoinGet(index);
            if (index == 0) {
                awardPosition.x = lbl_1_data_5E8.x - 300.0f;
            } else {
                awardPosition.x = 300.0f + lbl_1_data_5E8.x;
            }
            awardPosition.y = 250.0f + lbl_1_data_5E8.y;
            awardPosition.z = lbl_1_data_5E8.z;
            for (motionIndex = 0; motionIndex < 2; motionIndex++) {
                playerNo = mbPlayerTeamFindPlayer(index, motionIndex);
                if (lbl_1_bss_570[index] >= 0) {
                    mbPlayerMotionShiftSet(playerNo, 7, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
                    CharFXPlay(GwPlayer[playerNo].charNo, CHARVOICEID(2));
                } else {
                    mbPlayerMotionShiftSet(playerNo, 8, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
                    CharFXPlay(GwPlayer[playerNo].charNo, CHARVOICEID(12));
                }
            }
            if (lbl_1_bss_570[index] >= 0) {
                displayIds[index] = mbCoinDispCreate(&awardPosition, lbl_1_bss_570[index], 1, TRUE);
            } else {
                displayIds[index] =
                    mbCoinDispCreate(&awardPosition, lbl_1_bss_570[index], -1, TRUE);
            }
            mbPlayerTeamCoinSet(index, lbl_1_bss_570[index] + mbPlayerTeamCoinGet(index));
        }
        do {
            pendingDisplays = 0;
            for (index = 0; index < 2; index++) {
                if (!mbCoinDispKillCheck(displayIds[index])) {
                    pendingDisplays++;
                }
            }
            HuPrcVSleep();
        } while (pendingDisplays != 0);
    }
    HuPrcSleep(60);
    player = lbl_1_bss_C50;
    for (index = 0; index < 4; index++, player++) {
        for (motionIndex = 0; motionIndex < 3U; motionIndex++) {
            mbPlayerMotionKill(index, player->motions[motionIndex]);
        }
    }
    HuDataDirClose(W04TimeDayGet() ? DATA_w04 : DATA_w04n);
}

// Move and turn projectile-event players, time their throws, and finish hit reactions.
void fn_1_8004(int playerNo)
{
    HuVecF stick;
    W04EventPlayer *player = &lbl_1_bss_C50[playerNo];
    s8 padNo = GwPlayer[playerNo].padNo;
    s16 motion = -1;
    u16 buttons;
    u32 motionAttribute;
    float stickMagnitude;
    float directionX;
    float directionZ;
    float acceleration;
    float speed;
    float velocitySquared;

    switch (player->state) {
    case 0:
        if (!lbl_1_bss_D90) {
            if (!GwPlayer[playerNo].comF) {
                stick.x = mbPadStkXGet(padNo);
                stick.y = mbPadStkYGet(padNo);
                buttons = HuPadBtnDown[padNo];
            } else {
                buttons = fn_1_8ACC(playerNo, &stick);
            }
        } else {
            stick.x = stick.y = 0.0f;
            buttons = 0;
        }
        directionX = stick.x;
        directionZ = -stick.y;
        stickMagnitude = sqrtf(directionX * directionX + directionZ * directionZ);
        if (stickMagnitude > 0.0f) {
            directionX /= stickMagnitude;
            directionZ /= stickMagnitude;
            if (stickMagnitude > 72.0f) {
                stickMagnitude = 72.0f;
            }
            acceleration = (10000.0f * stickMagnitude) / 72.0f;
            player->velocity.x += (1.0f / 60.0f) * (directionX * acceleration);
            player->velocity.z += (1.0f / 60.0f) * (directionZ * acceleration);
            velocitySquared = player->velocity.x * player->velocity.x +
                player->velocity.z * player->velocity.z;
            speed = sqrtf(velocitySquared);
            if (0.0f != speed) {
                player->velocity.x /= speed;
                player->velocity.z /= speed;
                if (speed > 600.0f) {
                    speed = 600.0f;
                }
                player->velocity.x *= speed;
                player->velocity.z *= speed;
            }
            player->facingDegrees = mbAngleEaseOut(player->facingDegrees,
                180.0 * (atan2(player->velocity.x, player->velocity.z) / M_PI), 0.2f);
            if (speed < 450.0f) {
                motion = 2;
            } else {
                motion = 3;
            }
            motionAttribute = HU3D_MOTATTR_LOOP;
        } else {
            player->velocity.x = player->velocity.z = 0.0f;
            motion = 1;
            motionAttribute = HU3D_MOTATTR_LOOP;
        }
        player->position.x += (1.0f / 60.0f) * player->velocity.x;
        player->position.z += (1.0f / 60.0f) * player->velocity.z;
        player->velocity.x *= 0.95f;
        player->velocity.z *= 0.95f;
        if (buttons & PAD_BUTTON_A) {
            player->state = 1;
            player->elapsedFrames = 0;
            player->recoveryFrames = 30;
            mbPlayerMotionShiftSet(playerNo, player->motions[2], 0.0f, 4.0f, 0);
            motion = -1;
        }
        if (motion >= 0 && mbObjMotionShiftIDGet(mbPlayerObjIDGet(playerNo)) < 0) {
            mbPlayerMotionShiftSet(playerNo, motion, 0.0f, 8.0f, motionAttribute);
        }
        break;
    case 1:
        player->elapsedFrames++;
        if (player->elapsedFrames <= 11U) {
            if (player->elapsedFrames == 11U) {
                fn_1_9CA0(playerNo);
            } else {
                if (!GwPlayer[playerNo].comF) {
                    stick.x = mbPadStkXGet(padNo);
                    stick.y = mbPadStkYGet(padNo);
                    buttons = HuPadBtnDown[padNo];
                } else {
                    buttons = fn_1_8ACC(playerNo, &stick);
                }
                if (0.0f != stick.x || 0.0f != stick.y) {
                    player->facingDegrees = mbAngleEaseOut(player->facingDegrees,
                        180.0 * (atan2(stick.x, -stick.y) / M_PI), 0.2f);
                }
            }
        }
        if (player->elapsedFrames > player->recoveryFrames) {
            player->state = 0;
            mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 4.0f, HU3D_MOTATTR_LOOP);
        }
        break;
    case 2:
        player->position.x += (1.0f / 60.0f) * player->velocity.x;
        player->position.z += (1.0f / 60.0f) * player->velocity.z;
        player->velocity.x *= 0.99f;
        player->velocity.z *= 0.99f;
        if (mbPlayerMotionEndCheck(playerNo)) {
            player->state = 3;
            player->elapsedFrames = 0;
            player->recoveryFrames = 30;
            mbPlayerMotionShiftSet(playerNo, 6, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbPlayerMotionSpeedSet(playerNo, 1.0f);
        }
        break;
    case 3:
        if (++player->elapsedFrames > player->recoveryFrames) {
            player->state = 4;
            mbPlayerMotionShiftSet(playerNo, player->motions[1], 0.0f, 8.0f, 0);
        }
        break;
    case 4:
        if (mbPlayerMotionEndCheck(playerNo)) {
            player->state = 0;
            player->hitProtection = 90;
        }
        break;
    }
    if (player->hitProtection != 0) {
        if (--player->hitProtection == 0) {
            mbPlayerDispSet(playerNo, TRUE);
            return;
        }
        mbPlayerDispSet(playerNo, (player->hitProtection & 0x4) != 0);
    }
}

// Apply a projectile hit to a player during the board's coin-loss event.
void fn_1_89EC(s32 playerNo, HuVecF *direction)
{
    HuVecF velocity;
    W04EventPlayer *player;

    player = &lbl_1_bss_C50[playerNo];
    player->state = 2;
    mbPlayerMotionShiftSet(playerNo, player->motions[0], 0.0f, 8.0f,
        HU3D_MOTATTR_NONE);
    mbPlayerMotionSpeedSet(playerNo, 2.0f);
    PSVECNormalize(direction, &velocity);
    PSVECScale(&velocity, &velocity, 1000.0f);
    player->velocity = velocity;
    mbAudFXPlay(W04_SNOWBALL_HIT_SOUND);
    omVibrate(playerNo, 20, 7, 3);
}

// Move a computer player toward coins, periodically lining up a throw at an opponent.
u16 fn_1_8ACC(int playerNo, HuVecF *stick)
{
    int candidates[4];
    HuVecF goalPosition;
    HuVecF playerPosition;
    HuVecF direction;
    W04GridPoint goal;
    W04GridPoint nextStep;
    W04EventPlayer *player = &lbl_1_bss_C50[playerNo];
    int index;
    u32 candidateCount;
    float angle;
    float range;

    if (player->targetPlayerNo >= 0) {
        mbPlayerPosGet(playerNo, &playerPosition);
        mbPlayerPosGet(player->targetPlayerNo, &goalPosition);
        PSVECAdd(&goalPosition, &player->targetOffset, &goalPosition);
        PSVECSubtract(&goalPosition, &playerPosition, &direction);
        if (PSVECMag(&direction) >= 10.0f) {
            angle = 180.0 * (atan2(direction.x, direction.z) / M_PI);
            stick->x = 70.0f * mbSinDeg(angle);
            stick->y = 70.0f * -mbCosDeg(angle);
        } else {
            stick->x = stick->y = 0.0f;
        }
        if (--player->targetFrames == 0) {
            player->targetPlayerNo = -1;
            return PAD_BUTTON_A;
        }
        return 0;
    }
    player->cell.x = (int)((650.0 + player->position.x) / 130.0) + 1;
    player->cell.y = (int)((650.0 + player->position.z) / 130.0) + 1;
    fn_1_91AC(playerNo);
    W04GridDistancesFill(&player->cell);
    if (fn_1_6BB0(&goal)) {
        fn_1_6DC0(&player->cell, &goal, &nextStep);
        goalPosition.x = 65.0 + (-650.0 + 130.0 * (nextStep.x - 1));
        goalPosition.y = 0.0f;
        goalPosition.z = 65.0 + (-650.0 + 130.0 * (nextStep.y - 1));
        mbPlayerPosGet(playerNo, &playerPosition);
        PSVECSubtract(&goalPosition, &playerPosition, &direction);
        direction.y = 0.0f;
        if (player->cell.x != goal.x || player->cell.y != goal.y) {
            angle = 180.0 * (atan2(direction.x, direction.z) / M_PI);
            stick->x = 70.0f * mbSinDeg(angle);
            stick->y = 70.0f * -mbCosDeg(angle);
        } else {
            stick->x = stick->y = 0.0f;
        }
    } else {
        stick->x = stick->y = 0.0f;
    }
    if (mbRandMod(100) < 4U) {
        candidateCount = 0;
        for (index = 0; index < 4; index++) {
            if (index != playerNo) {
                candidates[candidateCount++] = index;
            }
        }
        player->targetPlayerNo = candidates[mbRandMod(candidateCount)];
        range = lbl_1_data_63C[GwPlayer[playerNo].comDif];
        player->targetOffset.x = range * frandf() + -range * frandf();
        player->targetOffset.y = 0.0f;
        player->targetOffset.z = range * frandf() + -range * frandf();
        player->targetFrames = (3 - GwPlayer[playerNo].comDif) * 6 + 24;
    }
    return 0;
}

// Penalize cells near opponents and favor cells containing a scattered coin.
void fn_1_91AC(s32 playerNo)
{
    HuVecF cellPosition;
    HuVecF separation;
    W04GridPoint coinCell;
    W04EventPlayer *player = &lbl_1_bss_C50[playerNo];
    int row;
    int column;
    W04CoinState *coin;
    int index;
    float distance;
    float cost;

    separation.z = 0.0f;
    for (row = 1; row < 11; row++) {
        for (column = 1; column < 11; column++) {
            if (lbl_1_bss_D94[row][column].cost < 0.0f) {
                continue;
            }
            lbl_1_bss_D94[row][column].cost = 0.0f;
            player = lbl_1_bss_C50;
            for (index = 0; index < 4; index++, player++) {
                if (index != playerNo) {
                    cellPosition.x = 65.0 + (-650.0 + 130.0 * column);
                    cellPosition.y = 65.0 + (-650.0 + 130.0 * row);
                    separation.x = cellPosition.x - (player->position.x - lbl_1_data_5E8.x);
                    separation.y = cellPosition.y - (player->position.z - lbl_1_data_5E8.z);
                    distance = VECMag(&separation);
                    if (distance < 1300.0) {
                        cost = mbCosDeg(90.0 * (distance / 1300.0));
                        cost *= cost;
                        if (cost > lbl_1_bss_D94[row][column].cost) {
                            lbl_1_bss_D94[row][column].cost = cost;
                        }
                    }
                }
            }
        }
    }
    coin = lbl_1_bss_590;
    for (row = 0; row < 16; row++, coin++) {
        if (coin->active) {
            coinCell.x = (int)((650.0 + coin->position.x) / 130.0) + 1;
            coinCell.y = (int)((650.0 + coin->position.z) / 130.0) + 1;
            cost = lbl_1_bss_D94[coinCell.y][coinCell.x].cost;
            if (cost >= 0.0f) {
                cost -= 0.5f;
                if (cost < 0.0f) {
                    cost = 0.0f;
                }
                lbl_1_bss_D94[coinCell.y][coinCell.x].cost = cost;
            }
        }
    }
}

// Resolve player overlaps repeatedly after each projectile-event movement update.
void fn_1_9578(void) {
    s32 overlapPass;

    overlapPass = 0;
    // The separation check also runs once when the pass limit has been reached.
    while ((fn_1_95BC() != 0) && (overlapPass < W04_COLLISION_PASS_LIMIT)) {
        overlapPass += 1;
    }
}

// Separate overlapping players and keep their positions inside the event's circular area.
s32 fn_1_95BC(void)
{
    HuVecF corrections[4];
    HuVecF separation;
    W04EventPlayer *player;
    W04EventPlayer *otherPlayer;
    int playerNo;
    int otherPlayerNo;
    int overlapCount = 0;
    float correctionDistance;
    float minimumCorrection = 0.0001f;
    float distance;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        corrections[playerNo].x = corrections[playerNo].y = corrections[playerNo].z = 0.0f;
    }
    for (playerNo = 0; playerNo < 3; playerNo++) {
        player = &lbl_1_bss_C50[playerNo];
        for (otherPlayerNo = playerNo + 1; otherPlayerNo < 4; otherPlayerNo++) {
            otherPlayer = &lbl_1_bss_C50[otherPlayerNo];
            VECSubtract(&player->position, &otherPlayer->position, &separation);
            distance = VECMag(&separation);
            if (distance < 100.0) {
                VECNormalize(&separation, &separation);
                correctionDistance = 0.5 * (100.0 - distance) + minimumCorrection;
                VECScale(&separation, &separation, correctionDistance);
                VECAdd(&corrections[playerNo], &separation, &corrections[playerNo]);
                VECSubtract(&corrections[otherPlayerNo], &separation, &corrections[otherPlayerNo]);
                overlapCount++;
            }
        }
    }
    for (playerNo = 0; playerNo < 4; playerNo++) {
        player = &lbl_1_bss_C50[playerNo];
        VECAdd(&player->position, &corrections[playerNo], &player->position);
        separation.x = player->position.x - lbl_1_data_5E8.x;
        separation.y = 0.0f;
        separation.z = player->position.z - lbl_1_data_5E8.z;
        distance = VECMag(&separation);
        if (distance > 600.0) {
            VECNormalize(&separation, &separation);
            player->position.x = lbl_1_data_5E8.x + 600.0 * separation.x;
            player->position.z = lbl_1_data_5E8.z + 600.0 * separation.z;
        }
    }
    return overlapCount;
}

// Allocate and hide both models for each projectile before the event starts.
void fn_1_9878(void)
{
    W04Projectile *projectile = lbl_1_bss_950;
    int index;

    for (index = 0; index < 16; index++, projectile++) {
        projectile->movingModelId = mbObjCreate(
            DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 18), NULL, TRUE);
        mbObjScaleSet(projectile->movingModelId, 0.3f, 0.3f, 0.3f);
        mbObjDispSet(projectile->movingModelId, FALSE);
        projectile->impactModelId = mbObjCreate(
            DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 19), NULL, TRUE);
        mbObjScaleSet(projectile->impactModelId, 0.3f, 0.3f, 0.3f);
        mbObjDispSet(projectile->impactModelId, FALSE);
    }
}

// Advance each thrown projectile along its arc and finish the impact animation.
void fn_1_99BC(void)
{
    W04Projectile *projectile = lbl_1_bss_950;
    HuVecF displacement;
    int index;
    float progress;
    float distance;

    for (index = 0; index < 16; index++, projectile++) {
        if (projectile->active) {
            if (projectile->phase == 0) {
                progress = projectile->elapsedFrames++ / (float)projectile->travelFrames;
                displacement = projectile->velocity;
                VECScale(&displacement, &displacement, 1.0f / 60.0f);
                VECAdd(&projectile->position, &displacement, &projectile->position);
                projectile->position.y = mbBezierCalc(projectile->startHeight,
                    100.0f + projectile->startHeight, lbl_1_data_5E8.y, progress);
                if (projectile->elapsedFrames > projectile->travelFrames) {
                    fn_1_9C18(index);
                } else {
                    VECSubtract(&projectile->position, &lbl_1_data_5E8, &displacement);
                    distance = VECMag(&displacement);
                    if (distance > 700.0 && projectile->position.z < lbl_1_data_5E8.z) {
                        fn_1_9C18(index);
                        VECNormalize(&displacement, &displacement);
                        projectile->position.x = lbl_1_data_5E8.x + distance * displacement.x;
                        projectile->position.y = lbl_1_data_5E8.y + distance * displacement.y;
                        projectile->position.z = lbl_1_data_5E8.z + distance * displacement.z;
                    }
                }
                mbObjPosSetV(projectile->movingModelId, &projectile->position);
            } else if (mbObjMotionEndCheck(projectile->impactModelId)) {
                projectile->active = FALSE;
                mbObjDispSet(projectile->impactModelId, FALSE);
            }
        }
    }
}

// Show the impact model at the projectile's last position when its flight ends.
void fn_1_9C18(s32 projectileIndex) {
    Point3d impactPosition;
    s32 unused;
    void * projectileRecord;
    projectileRecord = (s32 *)((u8 *) & lbl_1_bss_950 + (projectileIndex * W04_PROJECTILE_BYTES));
    mbObjDispSet((*(s16 *)((s8 *)(projectileRecord) + (8))), 0);
    mbObjPosGet((*(s16 *)((s8 *)(projectileRecord) + (8))), & impactPosition);
    mbObjDispSet((*(s16 *)((s8 *)(projectileRecord) + (10))), 1);
    mbObjPosSetV((*(s16 *)((s8 *)(projectileRecord) + (10))), & impactPosition);
    mbObjMotionTimeSet((*(s16 *)((s8 *)(projectileRecord) + (10))), 0.0f);
    (*(s32 *)((s8 *)(projectileRecord) + (4))) = 1;
}

// Launch the first free projectile from the selected player's held item.
s32 fn_1_9CA0(s32 playerNo)
{
    W04Projectile *projectile = lbl_1_bss_950;
    int index;
    char *hook;
    float angle;

    for (index = 0; index < 16; index++, projectile++) {
        if (!projectile->active) {
            break;
        }
    }
    if (index >= 16) {
        return -1;
    }
    projectile->active = TRUE;
    projectile->phase = 0;
    projectile->ownerPlayerNo = playerNo;
    hook = CharModelItemHookGet(GwPlayer[playerNo].charNo, 4, 0);
    Hu3DModelObjPosGet(mbPlayerModelIDGet(playerNo), hook, &projectile->position);
    projectile->startHeight = projectile->position.y;
    angle = mbPlayerRotYGet(playerNo);
    projectile->velocity.x = 1500.0f * mbSinDeg(angle);
    projectile->velocity.y = 0.0f;
    projectile->velocity.z = 1500.0f * mbCosDeg(angle);
    projectile->elapsedFrames = 0;
    projectile->travelFrames = 30;
    mbObjDispSet(projectile->movingModelId, TRUE);
    return index;
}

// Resolve a thrown projectile's hit and scatter coins from an unprotected player.
void fn_1_9DFC(void)
{
    int candidates[4];
    HuVecF separation;
    int coinCount;
    W04Projectile *projectile = lbl_1_bss_950;
    W04EventPlayer *player;
    int projectileIndex;
    int playerNo;
    int candidateCount;
    int team;
    float distance;

    for (projectileIndex = 0; projectileIndex < 16; projectileIndex++, projectile++) {
        if (projectile->active && projectile->phase == 0) {
            candidateCount = 0;
            player = lbl_1_bss_C50;
            for (playerNo = 0; playerNo < 4; playerNo++, player++) {
                if (player->state < 2 && projectile->ownerPlayerNo != playerNo) {
                    VECSubtract(&projectile->position, &player->position, &separation);
                    separation.y = 0.0f;
                    distance = VECMag(&separation);
                    if (distance < 100.0) {
                        candidates[candidateCount++] = playerNo;
                    }
                }
            }
            if (candidateCount != 0) {
                playerNo = candidates[mbRandMod(candidateCount)];
                W04ProjectileImpact(projectileIndex);
                player = &lbl_1_bss_C50[playerNo];
                if (player->hitProtection == 0 && lbl_1_bss_D90 == 0) {
                    coinCount = mbRandMod(2) + 3;
                    if (!GWTeamFGet()) {
                        if (lbl_1_bss_D80[playerNo] >= coinCount) {
                            coinCount = fn_1_A80C(playerNo, coinCount);
                        } else {
                            coinCount = fn_1_A80C(playerNo, lbl_1_bss_D80[playerNo]);
                        }
                        lbl_1_bss_D80[playerNo] -= coinCount;
                    } else {
                        team = W04PlayerTeamGet(playerNo);
                        if (lbl_1_bss_D80[team] >= coinCount) {
                            coinCount = fn_1_A80C(playerNo, coinCount);
                        } else {
                            coinCount = fn_1_A80C(playerNo, lbl_1_bss_D80[team]);
                        }
                        lbl_1_bss_D80[team] -= coinCount;
                    }
                    W04ProjectilePlayerHit(playerNo, &projectile->velocity);
                }
            }
        }
    }
}

// Bounce scattered coins inside the event boundary and fade their markers with height.
void fn_1_A170(void)
{
    HuVecF areaOffset;
    HuVecF boundaryNormal;
    W04CoinState *coin = lbl_1_bss_590;
    int index;
    float markerScale;
    float reflection;
    float distance;

    for (index = 0; index < 16; index++, coin++) {
        if (coin->active) {
            coin->velocity.y += -49.000004f;
            coin->position.x += (1.0f / 60.0f) * coin->velocity.x;
            coin->position.y += (1.0f / 60.0f) * coin->velocity.y;
            coin->position.z += (1.0f / 60.0f) * coin->velocity.z;
            coin->rotation.y += coin->rotationVelocity.y;
            if (coin->position.y < 50.0f + lbl_1_data_5E8.y && coin->velocity.y < 0.0f) {
                coin->velocity.y = 0.3f * -coin->velocity.y;
                coin->position.y = 50.0f + lbl_1_data_5E8.y;
                if (coin->expirationFrames < 0) {
                    coin->expirationFrames = 180;
                    mbAudFXPlay(MSM_SE_CMN_19);
                }
            }
            areaOffset.x = coin->position.x - lbl_1_data_5E8.x;
            areaOffset.y = 0.0f;
            areaOffset.z = coin->position.z - lbl_1_data_5E8.z;
            distance = PSVECMag(&areaOffset);
            if (distance > 600.0) {
                PSVECNormalize(&areaOffset, &areaOffset);
                coin->position.x = lbl_1_data_5E8.x + 600.0 * areaOffset.x;
                coin->position.z = lbl_1_data_5E8.z + 600.0 * areaOffset.z;
                boundaryNormal.x = coin->position.x - lbl_1_data_5E8.x;
                boundaryNormal.y = 0.0f;
                boundaryNormal.z = coin->position.z - lbl_1_data_5E8.z;
                PSVECNormalize(&boundaryNormal, &boundaryNormal);
                reflection = -coin->velocity.x * boundaryNormal.x +
                    -coin->velocity.z * boundaryNormal.z;
                coin->velocity.x += (2.0f * boundaryNormal.x) * reflection;
                coin->velocity.z += (2.0f * boundaryNormal.z) * reflection;
                coin->velocity.x *= 0.5f;
                coin->velocity.z *= 0.5f;
            }
            if (coin->expirationFrames > 0) {
                if (coin->expirationFrames < 60U) {
                    mbCoinObjDispSet(coin->coinObjectId, (coin->expirationFrames & 0x4) != 0);
                }
                if (--coin->expirationFrames == 0) {
                    coin->active = FALSE;
                    mbCoinObjDispSet(coin->coinObjectId, FALSE);
                    mbObjDispSet(coin->groundMarkerId, FALSE);
                    continue;
                }
                coin->velocity.x *= 0.95f;
                coin->velocity.z *= 0.95f;
            }
            mbCoinObjPosSetV(coin->coinObjectId, &coin->position);
            mbCoinObjRotSetV(coin->coinObjectId, &coin->rotation);
            mbObjPosSet(coin->groundMarkerId, coin->position.x,
                20.0f + lbl_1_data_5E8.y, coin->position.z);
            mbObjRotSet(coin->groundMarkerId, 0.0f, coin->rotation.y, 0.0f);
            markerScale = 1.0f - (coin->position.y - lbl_1_data_5E8.y) / 1500.0f;
            if (markerScale < 0.0f) {
                markerScale = 0.0f;
            } else if (markerScale > 1.0f) {
                markerScale = 1.0f;
            }
            mbObjScaleSet(coin->groundMarkerId, markerScale, 1.0f, markerScale);
        }
    }
}

// Throw a coin from the selected player's position during the coin-loss event.
s32 fn_1_A660(s32 playerNo)
{
    W04CoinState *coin = lbl_1_bss_590;
    W04EventPlayer *player = &lbl_1_bss_C50[playerNo];
    int index;
    float direction;
    float speed;

    for (index = 0; index < 16; index++, coin++) {
        if (!coin->active) {
            break;
        }
    }
    if (index >= 16) {
        return -1;
    }
    coin->active = TRUE;
    direction = 360.0f * frandf();
    speed = 200.0f + 200.0f * frandf();
    coin->position = player->position;
    coin->velocity.x = speed * mbCosDeg(direction);
    coin->velocity.y = 1200.0f + 400.0f * frandf();
    coin->velocity.z = speed * mbSinDeg(direction);
    coin->rotation.x = coin->rotation.y = coin->rotation.z = 0.0f;
    coin->rotationVelocity.y = 8.0f * (frandf() - 0.5f);
    coin->expirationFrames = -1;
    mbCoinObjDispSet(coin->coinObjectId, TRUE);
    mbObjDispSet(coin->groundMarkerId, TRUE);
    return index;
}

// Scatter as many coins as the available model slots can hold.
s32 fn_1_A80C(s32 playerNo, s32 count)
{
    int index;
    int spawnedCount = 0;

    for (index = 0; index < count; index++) {
        if (fn_1_A660(playerNo) >= 0) {
            spawnedCount++;
        }
    }
    return spawnedCount;
}

// Remove a scattered coin and show its collection effect after a player reaches it.
void fn_1_A9E0(s32 index) {
    W04CoinState *coin = &lbl_1_bss_590[index];
    mbCoinEffCreate(&coin->position);
    mbCoinObjDispSet(coin->coinObjectId, 0);
    mbObjDispSet(coin->groundMarkerId, 0);
    coin->active = 0;
}

// Award each scattered coin to a nearby player, or to that player's team.
void fn_1_AA44(void)
{
    int candidates[4];
    HuVecF separation;
    W04CoinState *coin = lbl_1_bss_590;
    W04EventPlayer *player;
    int coinIndex;
    int playerNo;
    int candidateCount;
    int team;
    float distance;

    for (coinIndex = 0; coinIndex < 16; coinIndex++, coin++) {
        if (coin->active) {
            candidateCount = 0;
            player = lbl_1_bss_C50;
            for (playerNo = 0; playerNo < 4; playerNo++, player++) {
                if (player->state == 0) {
                    VECSubtract(&coin->position, &player->position, &separation);
                    if (!(separation.y >= 200.0f)) {
                        separation.y = 0.0f;
                        distance = VECMag(&separation);
                        if (distance < 100.0) {
                            candidates[candidateCount++] = playerNo;
                        }
                    }
                }
            }
            if (candidateCount != 0) {
                playerNo = candidates[mbRandMod(candidateCount)];
                W04ScatteredCoinRemove(coinIndex);
                if (!GWTeamFGet()) {
                    lbl_1_bss_D80[playerNo]++;
                } else {
                    team = GwPlayer[playerNo].team;
                    lbl_1_bss_D80[team]++;
                }
            }
        }
    }
}

void fn_1_AC0C(void)
{
}

// Run the falling-coin game, then restore the board and give each player their collected coins.
void fn_1_AC10(int activePlayerNo, s16 eventSpaceId)
{
    int spawnFrames[30];
    HuVecF gridPosition;
    HUPROCESS *process;
    W04CoinPlayer *player;
    W04CoinState *coin;
    int index;
    int playerNo;
    int column;
    int row;
    int spawnIndex;
    int remainingFrames;
    s16 windowId;
    s16 helpWindowId;
    GAMEMESID timerId;
    char *hook;
    float distance;

    process = HuPrcCurrentGet();
    mbWipeSpecialFadeInCreate(1, 1);
    lbl_1_bss_122C = 0;
    mbStatusDispForceSetAll(FALSE);
    mbCameraMovePos(&lbl_1_data_5E8, &lbl_1_data_658, &lbl_1_data_64C,
        2800.0f, -1.0f, -1);
    memset(lbl_1_bss_420, 0, sizeof(lbl_1_bss_420));
    player = lbl_1_bss_420;
    Hu3DShadowCreate(30.0f, 20.0f, 5000.0f);
    Hu3DShadowTPLvlSet(0.6f);
    Hu3DShadowPosSet(&lbl_1_data_680, &lbl_1_data_68C, &lbl_1_data_698);
    Hu3DShadowColSet(32, 0, 64);
    mbObjShadowMapSet(lbl_1_bss_7280.shadowModelId);
    for (index = 0; index < 4; index++, player++) {
        mbPlayerColSnapPlayerSet(index, FALSE);
        player->facingDegrees = 0.0f;
        mbPlayerRotYSet(index, player->facingDegrees);
        player->position.x = lbl_1_data_5E8.x - 300.0f + 600.0f * (index % 2);
        player->position.y = 10.0f + lbl_1_data_5E8.y;
        player->position.z = lbl_1_data_5E8.z - 300.0f + 600.0f * (index / 2);
        mbPlayerPosSetV(index, &player->position);
        player->targetPlayerNo = -1;
        for (playerNo = 0; playerNo < 3U; playerNo++) {
            player->motions[playerNo] = mbPlayerMotionCreate(index, lbl_1_data_664[playerNo]);
        }
        player->eventModels[0] = mbObjCreate(
            DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 22), NULL, FALSE);
        hook = CharModelItemHookGet(GwPlayer[index].charNo, 4, 2);
        mbObjHookSet(mbPlayerObjIDGet(index), hook, player->eventModels[0]);
        player->eventModels[1] = mbObjCreate(
            DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 22), NULL, FALSE);
        hook = CharModelItemHookGet(GwPlayer[index].charNo, 4, 3);
        mbObjHookSet(mbPlayerObjIDGet(index), hook, player->eventModels[1]);
        mbObjShadowSet(mbPlayerObjIDGet(index));
    }
    memset(lbl_1_bss_D80, 0, sizeof(lbl_1_bss_D80));
    memset(lbl_1_bss_60, 0, sizeof(lbl_1_bss_60));
    coin = lbl_1_bss_60;
    for (index = 0; index < 16; index++, coin++) {
        coin->active = FALSE;
        coin->coinObjectId = mbCoinCreate2();
        coin->groundMarkerId = mbObjCreate(
            DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 21), NULL, TRUE);
        mbObjLayerSet(coin->groundMarkerId, 3);
        mbCoinObjDispSet(coin->coinObjectId, FALSE);
        mbObjDispSet(coin->groundMarkerId, FALSE);
    }
    spawnIndex = 0;
    for (index = 0; index < 30; index++) {
        spawnFrames[index] = index * 780 / (sizeof(spawnFrames) / sizeof(spawnFrames[0]));
    }
    gridPosition.z = 0.0f;
    for (row = 0; row < 12; row++) {
        for (column = 0; column < 12; column++) {
            gridPosition.x = 65.0 + (-650.0 + 130.0 * (column - 1));
            gridPosition.y = 65.0 + (-650.0 + 130.0 * (row - 1));
            distance = PSVECMag(&gridPosition);
            if (distance < 650.0) {
                lbl_1_bss_D94[row][column].cost = 0.0f;
            } else {
                lbl_1_bss_D94[row][column].cost = -1.0f;
            }
        }
    }
    mbWipeSpecialFadeOutCreate(1, 30);
    windowId = mbWinCreate(2, MESSNUM(MESS_BOARD_W04, 15), 6);
    mbWinPlayerDisable(windowId, -1);
    mbAudGuidePlay(MSM_SE_GUIDE_25);
    mbWinWait(windowId);
    windowId = mbWinCreate(2, MESSNUM(MESS_BOARD_W04, 16), 6);
    mbWinPlayerDisable(windowId, -1);
    mbAudGuidePlay(MSM_SE_GUIDE_26);
    mbWinWait(windowId);
    windowId = mbWinCreate(2, MESSNUM(MESS_BOARD_W04, 10), 6);
    mbAudGuidePlay(MSM_SE_GUIDE_25);
    mbWinPause(windowId);
    HuPrcSleep(90);
    mbWinKill(windowId);
    helpWindowId = mbWinCreateHelp(MESSNUM(MESS_BOARD_W04, 19));
    mbWinPosSet(helpWindowId, 240, 400);
    lbl_1_bss_D90 = FALSE;
    timerId = GameMesCreate(1, 15, -1, -1);
    HuSprGrpDrawNoSet(GameMesGet(timerId)->grpId[0], 32);
    remainingFrames = 900;
    for (index = 0; index < 1080U; index++) {
        for (playerNo = 0; playerNo < 4; playerNo++) {
            fn_1_B7CC(playerNo);
        }
        if (spawnIndex < 30 && index == spawnFrames[spawnIndex]) {
            fn_1_C810();
            spawnIndex++;
        }
        fn_1_C52C();
        fn_1_C080();
        fn_1_CA00();
        player = lbl_1_bss_420;
        for (playerNo = 0; playerNo < 4; playerNo++, player++) {
            mbPlayerPosSetV(playerNo, &player->position);
            mbPlayerRotYSet(playerNo, player->facingDegrees);
        }
        if (remainingFrames >= 0) {
            remainingFrames--;
            if (remainingFrames >= 0) {
                GameMesDispSet(timerId, 1, (remainingFrames + 59) / 60);
            } else {
                lbl_1_bss_D90 = TRUE;
                GameMesDispSet(timerId, 2, -1);
            }
        }
        HuPrcVSleep();
    }
    mbWipeDissolveFadeOut();
    mbWinKill(helpWindowId);
    player = lbl_1_bss_420;
    for (index = 0; index < 4; index++, player++) {
        mbPlayerRotYSet(index, 0.0f);
        player->position.x = lbl_1_data_5E8.x - 300.0f + 600.0f * (index % 2);
        player->position.y = lbl_1_data_5E8.y;
        player->position.z = lbl_1_data_5E8.z - 300.0f + 600.0f * (index / 2);
        mbPlayerPosSetV(index, &player->position);
        mbPlayerMotionSet(index, 1, HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP);
        mbPlayerDispSet(index, TRUE);
        mbObjHookReset(mbPlayerObjIDGet(index));
        for (playerNo = 0; playerNo < 2; playerNo++) {
            if (player->eventModels[playerNo]) {
                mbObjKill(player->eventModels[playerNo]);
                player->eventModels[playerNo] = 0;
            }
        }
    }
    coin = lbl_1_bss_60;
    for (index = 0; index < 16; index++, coin++) {
        mbCoinObjNumDec(coin->coinObjectId);
        mbObjKill(coin->groundMarkerId);
    }
    mbWipeDissolveFadeIn();
    HuPrcSleep(30);
    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_D80[index] > 0) {
            mbPlayerMotionShiftSet(index, 7, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
            CharFXPlay(GwPlayer[index].charNo, CHARVOICEID(2));
        } else {
            mbPlayerMotionShiftSet(index, 8, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
            CharFXPlay(GwPlayer[index].charNo, CHARVOICEID(12));
        }
    }
    mbCoinAddAllProcExecV(lbl_1_bss_D80, lbl_1_data_670, TRUE);
    HuPrcSleep(60);
    player = lbl_1_bss_420;
    for (index = 0; index < 4; index++, player++) {
        for (playerNo = 0; playerNo < 3U; playerNo++) {
            mbPlayerMotionKill(index, player->motions[playerNo]);
        }
    }
    HuDataDirClose(W04TimeDayGet() ? DATA_w04 : DATA_w04n);
}

// Update a player's movement and standing up after knockback during the falling-coin event.
void fn_1_B7CC(int playerNo)
{
    HuVecF stick;
    W04CoinPlayer *player = &lbl_1_bss_420[playerNo];
    s8 padNo = GwPlayer[playerNo].padNo;
    s16 motion = -1;
    u32 buttons;
    u32 motionAttribute;
    float stickMagnitude;
    float directionX;
    float directionZ;
    float acceleration;
    float speed;
    float velocitySquared;

    switch (player->state) {
    case 0:
        if (!lbl_1_bss_D90) {
            if (!GwPlayer[playerNo].comF) {
                stick.x = mbPadStkXGet(padNo);
                stick.y = mbPadStkYGet(padNo);
                buttons = HuPadBtnDown[padNo];
            } else {
                buttons = fn_1_CB6C(playerNo, &stick);
            }
        } else {
            stick.x = stick.y = 0.0f;
            buttons = 0;
        }
        directionX = stick.x;
        directionZ = -stick.y;
        stickMagnitude = sqrtf(directionX * directionX + directionZ * directionZ);
        if (stickMagnitude > 0.0f) {
            directionX /= stickMagnitude;
            directionZ /= stickMagnitude;
            if (stickMagnitude > 72.0f) {
                stickMagnitude = 72.0f;
            }
            acceleration = (2000.0f * stickMagnitude) / 72.0f;
            player->velocity.x += (1.0f / 60.0f) * (directionX * acceleration);
            player->velocity.z += (1.0f / 60.0f) * (directionZ * acceleration);
            velocitySquared = player->velocity.x * player->velocity.x +
                player->velocity.z * player->velocity.z;
            speed = sqrtf(velocitySquared);
            if (0.0f != speed) {
                player->velocity.x /= speed;
                player->velocity.z /= speed;
                if (speed > 1000.0f) {
                    speed = 1000.0f;
                }
                player->velocity.x *= speed;
                player->velocity.z *= speed;
            }
            player->facingDegrees = mbAngleEaseOut(player->facingDegrees,
                180.0 * (atan2(directionX, directionZ) / M_PI), 0.2f);
            motion = player->motions[2];
            motionAttribute = HU3D_MOTATTR_LOOP;
        } else {
            motion = 1;
            motionAttribute = HU3D_MOTATTR_LOOP;
        }
        player->position.x += (1.0f / 60.0f) * player->velocity.x;
        player->position.z += (1.0f / 60.0f) * player->velocity.z;
        player->velocity.x *= 0.99f;
        player->velocity.z *= 0.99f;
        if (motion >= 0 && mbObjMotionShiftIDGet(mbPlayerObjIDGet(playerNo)) < 0) {
            mbPlayerMotionShiftSet(playerNo, motion, 0.0f, 8.0f, motionAttribute);
        }
        break;
    case 1:
        player->position.x += (1.0f / 60.0f) * player->velocity.x;
        player->position.z += (1.0f / 60.0f) * player->velocity.z;
        player->velocity.x *= 0.99f;
        player->velocity.z *= 0.99f;
        if (mbPlayerMotionEndCheck(playerNo)) {
            player->state = 2;
            player->elapsedFrames = 0;
            player->recoveryFrames = 30;
            mbPlayerMotionShiftSet(playerNo, 6, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbPlayerMotionSpeedSet(playerNo, 1.0f);
            player->velocity.x = player->velocity.z = 0.0f;
        }
        break;
    case 2:
        if (++player->elapsedFrames > player->recoveryFrames) {
            player->state = 3;
            mbPlayerMotionShiftSet(playerNo, player->motions[1], 0.0f, 8.0f, 0);
        }
        break;
    case 3:
        if (mbPlayerMotionEndCheck(playerNo)) {
            player->state = 0;
            player->protectionFrames = 90;
        }
        break;
    }
    if (player->protectionFrames != 0) {
        if (--player->protectionFrames == 0) {
            mbPlayerDispSet(playerNo, TRUE);
            player->knockedBack = FALSE;
            return;
        }
        mbPlayerDispSet(playerNo, (player->protectionFrames & 0x4) != 0);
    }
}

// Start the player's knockback motion when hit during the falling-coin event.
void fn_1_BF98(s32 playerNo, HuVecF *direction)
{
    HuVecF velocity;
    W04CoinPlayer *player;

    player = &lbl_1_bss_420[playerNo];
    player->knockedBack = TRUE;
    player->state = 1;
    mbPlayerMotionShiftSet(playerNo, player->motions[0], 0.0f, 8.0f,
        HU3D_MOTATTR_NONE);
    mbPlayerMotionSpeedSet(playerNo, 2.0f);
    PSVECNormalize(direction, &velocity);
    PSVECScale(&velocity, &velocity, 500.0f);
    player->velocity = velocity;
    mbAudFXPlay(W04_PLAYER_COLLISION_SOUND);
    omVibrate(playerNo, 20, 7, 3);
}

// Resolve player overlaps repeatedly after each falling-coin movement update.
void fn_1_C080(void) {
    s32 overlapPass;

    overlapPass = 0;
    // The separation check also runs once when the pass limit has been reached.
    while ((fn_1_C0C4() != 0) && (overlapPass < W04_COLLISION_PASS_LIMIT)) {
        overlapPass += 1;
    }
}

// Separate coin-event players and start knockback reactions while the event is active.
s32 fn_1_C0C4(void)
{
    HuVecF corrections[4];
    HuVecF separation;
    W04CoinPlayer *player;
    int playerNo;
    int otherPlayerNo;
    int overlapCount = 0;
    W04CoinPlayer *otherPlayer;
    float correctionDistance;
    float minimumCorrection = 0.0001f;
    float distance;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        corrections[playerNo].x = corrections[playerNo].y = corrections[playerNo].z = 0.0f;
    }
    for (playerNo = 0; playerNo < 3; playerNo++) {
        player = &lbl_1_bss_420[playerNo];
        for (otherPlayerNo = playerNo + 1; otherPlayerNo < 4; otherPlayerNo++) {
            otherPlayer = &lbl_1_bss_420[otherPlayerNo];
            PSVECSubtract(&player->position, &otherPlayer->position, &separation);
            distance = PSVECMag(&separation);
            if (distance < 100.0) {
                PSVECNormalize(&separation, &separation);
                correctionDistance = 0.5 * (100.0 - distance) + minimumCorrection;
                PSVECScale(&separation, &separation, correctionDistance);
                PSVECAdd(&corrections[playerNo], &separation, &corrections[playerNo]);
                PSVECSubtract(&corrections[otherPlayerNo], &separation,
                    &corrections[otherPlayerNo]);
                if (!lbl_1_bss_D90) {
                    if (!player->knockedBack) {
                        W04CoinPlayerKnockback(playerNo, &separation);
                    }
                    if (!otherPlayer->knockedBack) {
                        PSVECScale(&separation, &separation, -1.0f);
                        W04CoinPlayerKnockback(otherPlayerNo, &separation);
                    }
                }
                overlapCount++;
            }
        }
    }
    for (playerNo = 0; playerNo < 4; playerNo++) {
        player = &lbl_1_bss_420[playerNo];
        PSVECAdd(&player->position, &corrections[playerNo], &player->position);
        separation.x = player->position.x - lbl_1_data_5E8.x;
        separation.y = 0.0f;
        separation.z = player->position.z - lbl_1_data_5E8.z;
        distance = PSVECMag(&separation);
        if (distance > 600.0) {
            PSVECNormalize(&separation, &separation);
            player->position.x = lbl_1_data_5E8.x + 600.0 * separation.x;
            player->position.z = lbl_1_data_5E8.z + 600.0 * separation.z;
        }
    }
    return overlapCount;
}

// Apply gravity, bouncing, expiration flashes, and ground-marker scaling each frame.
void fn_1_C52C(void)
{
    W04CoinState *coin = lbl_1_bss_60;
    int index;
    float markerScale;

    for (index = 0; index < 16; index++, coin++) {
        if (coin->active) {
            coin->velocity.y += -980.0f / 60.0f;
            coin->position.x += (1.0f / 60.0f) * coin->velocity.x;
            coin->position.y += (1.0f / 60.0f) * coin->velocity.y;
            coin->position.z += (1.0f / 60.0f) * coin->velocity.z;
            coin->rotation.y += coin->rotationVelocity.y;
            if (coin->position.y < 50.0f + lbl_1_data_5E8.y && coin->velocity.y < 0.0f) {
                coin->velocity.y = 0.3f * -coin->velocity.y;
                coin->position.y = 50.0f + lbl_1_data_5E8.y;
                if (coin->expirationFrames < 0) {
                    coin->expirationFrames = 180;
                    mbAudFXPlay(MSM_SE_CMN_19);
                }
            }
            if (coin->expirationFrames > 0) {
                if (coin->expirationFrames < 60U) {
                    mbCoinObjDispSet(coin->coinObjectId, (coin->expirationFrames & 4) != 0);
                }
                if (--coin->expirationFrames == 0) {
                    coin->active = FALSE;
                    mbCoinObjDispSet(coin->coinObjectId, FALSE);
                    mbObjDispSet(coin->groundMarkerId, FALSE);
                    continue;
                }
            }
            mbCoinObjPosSetV(coin->coinObjectId, &coin->position);
            mbCoinObjRotSetV(coin->coinObjectId, &coin->rotation);
            mbObjPosSet(coin->groundMarkerId, coin->position.x, lbl_1_data_5E8.y, coin->position.z);
            mbObjRotSet(coin->groundMarkerId, 0.0f, coin->rotation.y, 0.0f);
            markerScale = 1.0f - (coin->position.y - lbl_1_data_5E8.y) / 1500.0f;
            if (markerScale < 0.0f) {
                markerScale = 0.0f;
            } else if (markerScale > 1.0f) {
                markerScale = 1.0f;
            }
            mbObjScaleSet(coin->groundMarkerId, markerScale, 1.0f, markerScale);
        }
    }
}

// Activate the first free falling coin at a random point around the event center.
void fn_1_C810(void)
{
    W04CoinState *coin = lbl_1_bss_60;
    int index;
    float direction;
    float distance;

    for (index = 0; index < 16; index++, coin++) {
        if (!coin->active) {
            break;
        }
    }
    if (index >= 16) {
        return;
    }
    coin->active = TRUE;
    direction = 360.0f * frandf();
    distance = 650.0 * frandf();
    coin->position.x = lbl_1_data_5E8.x + distance * mbCosDeg(direction);
    coin->position.y = 1500.0f + lbl_1_data_5E8.y;
    coin->position.z = lbl_1_data_5E8.z + distance * mbSinDeg(direction);
    coin->velocity.x = coin->velocity.y = coin->velocity.z = 0.0f;
    coin->rotation.x = coin->rotation.y = coin->rotation.z = 0.0f;
    coin->rotationVelocity.y = 8.0f * (frandf() - 0.5f);
    coin->expirationFrames = -1;
    mbCoinObjDispSet(coin->coinObjectId, TRUE);
    mbObjDispSet(coin->groundMarkerId, TRUE);
}

// Remove a collected coin and its ground marker when the collection check selects it.
void fn_1_C99C(s32 index)
{
    W04CoinState *state = &lbl_1_bss_60[index];

    mbCoinEffCreate(&state->position);
    mbCoinObjDispSet(state->coinObjectId, 0);
    mbObjDispSet(state->groundMarkerId, 0);
    state->active = 0;
}

// Award each nearby falling coin to one randomly selected eligible player.
void fn_1_CA00(void)
{
    int candidates[4];
    HuVecF separation;
    W04CoinState *coin = lbl_1_bss_60;
    W04CoinPlayer *player;
    int coinIndex;
    int playerNo;
    int candidateCount;
    float distance;

    for (coinIndex = 0; coinIndex < 16; coinIndex++, coin++) {
        if (coin->active) {
            candidateCount = 0;
            player = lbl_1_bss_420;
            for (playerNo = 0; playerNo < 4; playerNo++, player++) {
                VECSubtract(&coin->position, &player->position, &separation);
                if (!(separation.y >= 200.0f)) {
                    separation.y = 0.0f;
                    distance = VECMag(&separation);
                    if (distance < 100.0) {
                        candidates[candidateCount++] = playerNo;
                    }
                }
            }
            if (candidateCount != 0) {
                playerNo = candidates[mbRandMod(candidateCount)];
                W04CollectedCoinRemove(coinIndex);
                lbl_1_bss_D80[playerNo]++;
            }
        }
    }
}

// Steer a computer player toward a reachable low-cost cell during the falling-coin event.
BOOL fn_1_CB6C(int playerNo, HuVecF *stick)
{
    HuVecF goalPosition;
    HuVecF playerPosition;
    HuVecF direction;
    W04GridPoint goal;
    W04GridPoint nextStep;
    W04CoinPlayer *player = &lbl_1_bss_420[playerNo];
    float angle;

    player->cell.x = (int)((650.0 + player->position.x) / 130.0) + 1;
    player->cell.y = (int)((650.0 + player->position.z) / 130.0) + 1;
    fn_1_D020(playerNo);
    W04GridDistancesFill(&player->cell);
    if (fn_1_6BB0(&goal)) {
        fn_1_6DC0(&player->cell, &goal, &nextStep);
        goalPosition.x = 65.0 + (-650.0 + 130.0 * (nextStep.x - 1));
        goalPosition.y = 0.0f;
        goalPosition.z = 65.0 + (-650.0 + 130.0 * (nextStep.y - 1));
        mbPlayerPosGet(playerNo, &playerPosition);
        PSVECSubtract(&goalPosition, &playerPosition, &direction);
        direction.y = 0.0f;
        if (player->cell.x != goal.x || player->cell.y != goal.y) {
            angle = 180.0 * (atan2(direction.x, direction.z) / M_PI);
            stick->x = 70.0f * mbSinDeg(angle);
            stick->y = 70.0f * -mbCosDeg(angle);
        } else {
            stick->x = stick->y = 0.0f;
        }
    } else {
        stick->x = stick->y = 0.0f;
    }
    return FALSE;
}

// Penalize cells near opponents and favor cells containing a falling coin.
void fn_1_D020(s32 playerNo)
{
    HuVecF cellPosition;
    HuVecF separation;
    W04GridPoint coinCell;
    W04CoinPlayer *player = &lbl_1_bss_420[playerNo];
    int row;
    int column;
    W04CoinState *coin;
    int index;
    float distance;
    float cost;

    separation.z = 0.0f;
    for (row = 1; row < 11; row++) {
        for (column = 1; column < 11; column++) {
            if (lbl_1_bss_D94[row][column].cost < 0.0f) {
                continue;
            }
            lbl_1_bss_D94[row][column].cost = 0.0f;
            player = lbl_1_bss_420;
            for (index = 0; index < 4; index++, player++) {
                if (index != playerNo) {
                    cellPosition.x = 65.0 + (-650.0 + 130.0 * column);
                    cellPosition.y = 65.0 + (-650.0 + 130.0 * row);
                    separation.x = cellPosition.x - (player->position.x - lbl_1_data_5E8.x);
                    separation.y = cellPosition.y - (player->position.z - lbl_1_data_5E8.z);
                    distance = VECMag(&separation);
                    if (distance < 1300.0) {
                        cost = mbCosDeg(90.0 * (distance / 1300.0));
                        cost *= cost;
                        if (cost > lbl_1_bss_D94[row][column].cost) {
                            lbl_1_bss_D94[row][column].cost = cost;
                        }
                    }
                }
            }
        }
    }
    coin = lbl_1_bss_60;
    for (row = 0; row < 16; row++, coin++) {
        if (coin->active) {
            coinCell.x = (int)((650.0 + coin->position.x) / 130.0) + 1;
            coinCell.y = (int)((650.0 + coin->position.z) / 130.0) + 1;
            cost = lbl_1_bss_D94[coinCell.y][coinCell.x].cost;
            if (cost >= 0.0f) {
                cost -= 0.5f;
                if (cost < 0.0f) {
                    cost = 0.0f;
                }
                lbl_1_bss_D94[coinCell.y][coinCell.x].cost = cost;
            }
        }
    }
}

// Create the four looping smoke models at their fixed board positions.
void fn_1_D3EC(void)
{
    MBMODELID *model = lbl_1_bss_58;
    int index;

    for (index = 0; index < 4; index++, model++) {
        *model = mbObjCreate(DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 8),
            NULL, FALSE);
        mbObjLayerSet(*model, 3);
        mbObjAttrSet(*model, HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP);
        mbObjPosSetV(*model, &lbl_1_data_6A4[index]);
        mbObjMotionTimeSet(*model, mbRandMod((u32)mbObjMotionMaxTimeGet(*model)));
    }
}

// Create a hidden space-triggered model during board setup and pause its motion.
void fn_1_D4F8(void)
{
    W04SpaceObject *object = &lbl_1_bss_50;
    BOOL day = GwSystem.curTime == 0;

    object->modelId = mbObjCreate(
        DATANUM(day ? DATA_w04 : DATA_w04n, 11), NULL, TRUE);
    object->spaceId = mbMasuFind_MAttrIdGet(-1, W04_SCENERY_TRIGGER_MASK);
    object->running = FALSE;
    mbObjMotionSpeedSet(object->modelId, 0.0f);
    mbObjDispSet(object->modelId, FALSE);
}

// Show and restart the space-triggered model when a player begins moving from its space.
void fn_1_D5B0(void)
{
    W04SpaceObject *state = &lbl_1_bss_50;

    mbObjDispSet(state->modelId, 1);
    mbObjMotionTimeSet(state->modelId, 0.0f);
    mbObjMotionSpeedSet(state->modelId, 1.0f);
    state->running = 1;
}

// Hide the space-triggered model after its motion ends, from the board's frame updater.
void fn_1_D618(void) {
    W04SpaceObject *object = &lbl_1_bss_50;
    if (object->running != 0 && mbObjMotionEndCheck(object->modelId) != 0) {
        object->running = 0;
        mbObjDispSet(object->modelId, 0);
    }
}

// Create the animated board model and its hidden moving attachment.
void fn_1_D674(void)
{
    W04PairedBoardObject *object = &lbl_1_bss_44;

    if (GwSystem.curTime == 0) {
        object->mainModelId = mbObjCreate(DATANUM(DATA_w04, 14), lbl_1_data_6D4, TRUE);
    } else {
        object->mainModelId = mbObjCreate(DATANUM(DATA_w04n, 14), lbl_1_data_6DC, TRUE);
    }
    mbObjAttrSet(object->mainModelId, HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP);
    mbObjPosSet(object->mainModelId, -3200.0f, 1100.0f, -1950.0f);
    object->launchDegrees = 65.0f;
    mbObjRotSet(object->mainModelId, 0.0f, object->launchDegrees, 0.0f);
    object->movingModelId = mbObjCreate(
        DATANUM(W04TimeDayGet() ? DATA_w04 : DATA_w04n, 16), NULL, TRUE);
    mbObjDispSet(object->movingModelId, FALSE);
}

// Show the board model releasing its attachment, then move it through a short arc.
void fn_1_D7C8(void)
{
    W04PairedBoardObject *object = &lbl_1_bss_44;
    HuVecF position;
    HuVecF hookPosition;
    int frame;
    float progress;

    mbWipeSpecialFadeInCreate(1, 1);
    mbObjMotionSet(object->mainModelId, 1, HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP);
    mbObjMotionSpeedSet(object->mainModelId, 0.0f);
    mbObjPosGet(object->mainModelId, &position);
    mbCameraMovePos(&position, NULL, NULL, 2800.0f, -1.0f, -1);
    mbObjDispSet(object->movingModelId, TRUE);
    mbObjHookSet(object->mainModelId, "y_h1", object->movingModelId);
    mbObjPosSet(object->movingModelId, 0.0f, 0.0f, 0.0f);
    mbWipeSpecialFadeOutCreate(1, 30);
    mbObjMotionSpeedSet(object->mainModelId, 1.0f);
    mbAudFXDelaySet(18);
    mbAudFXPlay(W04_ATTACHMENT_LAUNCH_SOUND);
    HuPrcSleep(31);
    Hu3DModelObjPosGet(mbObjModelIDGet(object->mainModelId),
        "y_h1", &hookPosition);
    mbObjHookReset(object->mainModelId);
    for (frame = 0; frame < 60U; frame++) {
        progress = frame / 60.0f;
        position.x =
            hookPosition.x + 100.0f * (30.0f * (progress * mbSinDeg(object->launchDegrees)));
        position.y = hookPosition.y + 100.0f * (40.0f * mbSinDeg(90.0 * progress));
        position.z =
            hookPosition.z + 100.0f * (30.0f * (progress * mbCosDeg(object->launchDegrees)));
        mbObjPosSetV(object->movingModelId, &position);
        HuPrcVSleep();
    }
    mbWipeDissolveFadeOut();
    mbObjMotionSet(object->mainModelId, 0, HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP);
    mbObjDispSet(object->movingModelId, FALSE);
}

char *lbl_1_data_728[6] = {
    "mogura_h1", "mogura_h2", "mogura_h3", "mogura_h4", "mogura_h5", "mogura_h6"
};

// Attach six motion-stopped models to the named hooks on the board model.
void fn_1_DA4C(void)
{
    MBMODELID *model = lbl_1_bss_38;
    MBMODELID *boardModel = lbl_1_bss_7280.models;
    int index;
    BOOL day;

    for (index = 0; index < 6; index++, model++) {
        day = GwSystem.curTime == 0;
        *model = mbObjCreate(DATANUM(day ? DATA_w04 : DATA_w04n, 28), NULL, TRUE);
        mbObjHookSet(*boardModel, lbl_1_data_728[index], *model);
        mbObjMotionSpeedSet(*model, 0.0f);
    }
}

// Restart the selected hooked mole's motion when a player ends movement on its marked space.
void fn_1_DB18(s32 modelIndex) {
    s32 unused;
    s16 * modelId;
    modelId = (s16 *)((u8 *) & lbl_1_bss_38 + (modelIndex * 2));
    mbObjMotionTimeSet(* modelId, 0.0f);
    mbObjMotionSpeedSet(* modelId, 1.0f);
    mbAudFXPlay(W04_MOLE_SOUND);
}

char *lbl_1_data_768[4] = { "pengin_h1", "pengin_h2", "pengin_h3", "pengin_h4" };

int lbl_1_data_778[4] = { DATANUM(DATA_w04, 36), DATANUM(DATA_w04, 37), DATANUM(DATA_w04, 38), -1 };

int lbl_1_data_788[4] = {
    DATANUM(DATA_w04n, 36), DATANUM(DATA_w04n, 37), DATANUM(DATA_w04n, 38), -1
};

int lbl_1_data_798[4] = { 0, 0, 1, 1 };

// Create four looping models on board hooks and associate them with board space indices.
void fn_1_DB84(void)
{
    W04AnimatedSpaceObject *object = lbl_1_bss_8;
    MBMODELID *boardModel = lbl_1_bss_7280.models;
    int index;

    for (index = 0; index < 4; index++, object++) {
        if (GwSystem.curTime == 0) {
            object->modelId = mbObjCreate(
                DATANUM(DATA_w04, 35), lbl_1_data_778, TRUE);
        } else {
            object->modelId = mbObjCreate(
                DATANUM(DATA_w04n, 35), lbl_1_data_788, TRUE);
        }
        mbObjHookSet(*boardModel, lbl_1_data_768[index], object->modelId);
        mbObjMotionSet(object->modelId, 1, HU3D_MOTATTR_LOOP);
        object->running = FALSE;
        object->spaceIndex = lbl_1_data_798[index];
    }
}

// Start the second motion on every object associated with this board space.
void fn_1_DC8C(int spaceIndex)
{
    W04AnimatedSpaceObject *object = lbl_1_bss_8;
    int index;

    for (index = 0; index < 4; index++, object++) {
        if (object->spaceIndex == spaceIndex) {
            object->running = TRUE;
            mbObjMotionShiftSet(object->modelId, 2, 0.0f, 8.0f,
                HU3D_MOTATTR_NONE);
        }
    }
}

// Return triggered penguins to their looping motion each frame after their reaction finishes.
void fn_1_DD1C(void) {
    W04AnimatedSpaceObject *object;
    s32 objectIndex;

    object = lbl_1_bss_8;
    objectIndex = 0;
    while (objectIndex < 4) {
        if (object->running != 0 && mbObjMotionEndCheck(object->modelId) != 0) {
            mbObjMotionShiftSet(object->modelId, 1, 0.0f, 8.0f,
                                HU3D_MOTATTR_LOOP);
            object->running = 0;
        }
        objectIndex += 1;
        object++;
    }
}

int lbl_1_data_7A8[2] = { DATANUM(DATA_w04, 30), -1 };

int lbl_1_data_7B0[2] = { DATANUM(DATA_w04n, 30), -1 };

// Create the day or night version of the board's looping animated object.
void fn_1_DDB8(void)
{
    W04AnimatedObject *object = lbl_1_bss_0;
    int index;

    for (index = 0; index < 1; index++, object++) {
        if (GwSystem.curTime == 0) {
            object->modelId = mbObjCreate(
                DATANUM(DATA_w04, 29), lbl_1_data_7A8, TRUE);
        } else {
            object->modelId = mbObjCreate(
                DATANUM(DATA_w04n, 29), lbl_1_data_7B0, TRUE);
        }
        mbObjAttrSet(object->modelId, HU3D_MOTATTR_LOOP);
        object->running = FALSE;
    }
}

// Start the board object's alternate motion when a player leaves its marked space.
void fn_1_DE74(s32 objectIndex) {
    s32 unused;
    void * objectRecord;
    objectRecord = (s32 *)((u8 *) & lbl_1_bss_0 + (objectIndex * 8));
    mbObjMotionSet((*(s16 *)((s8 *)(objectRecord) + (4))), 1, 0U);
    mbObjMotionTimeSet((*(s16 *)((s8 *)(objectRecord) + (4))), 0.0f);
    (*(s32 *)((s8 *)(objectRecord) + (0))) = 1;
}

// Restore the board object's idle loop after its alternate motion, from the frame updater.
void fn_1_DEDC(void) {
    W04AnimatedObject *object;
    s32 objectIndex;

    object = lbl_1_bss_0;
    objectIndex = 0;
    while (objectIndex < 1) {
        if (object->running != 0 && mbObjMotionEndCheck(object->modelId) != 0) {
            mbObjMotionShiftSet(object->modelId, 0, 0.0f, 8.0f,
                                HU3D_MOTATTR_LOOP);
            object->running = 0;
        }
        objectIndex += 1;
        object++;
    }
}
