// Board-specific models, events, and motion used by Towering Treetop.
#include "dolphin.h"
#include "humath.h"
#include "REL/w01Dll_world01.h"

#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/coin.h"
#include "game/board/comchoice.h"
#include "game/board/effect.h"
#include "game/board/main.h"
#include "game/board/object.h"
#include "game/board/masu.h"
#include "game/board/player.h"
#include "game/board/opening.h"
#include "game/board/status.h"
#include "game/board/window.h"
#include "game/data.h"
#include "game/audio.h"
#include "game/frand.h"
#include "game/gamemes.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "string.h"
#include "messdir_enum.h"
#include "datadir_enum.h"
#include "msm_grp.h"

void mbWipeDissolveFadeOut(void);
void mbWipeDissolveFadeIn(void);

#define MBNO_WORLD_1 0

#define W01_DATA_DAY DATA_w01
#define W01_DATA_NIGHT DATA_w01n
#define W01_FILE_MASU 4
#define W01_FILE_GATE 5
#define W01_FILE_SCROLL 2
#define W01_FILE_CAP_THROW_COLLISION 3
#define W01_FILE_SHOP 22
#define W01_FILE_SHOP_RETURN 26
#define W01_FILE_SHOP_MODEL 23
#define W01_FILE_PATH_MARKER 8
#define W01_FILE_PATH_MODEL 9
#define W01_FILE_SPRING_MODEL 10
#define W01_FILE_SPRING_MAIN_MODEL 11
#define W01_OBJ_ATTR_BOARD_MODEL HU3D_MOTATTR_LOOP
#define W01_OBJ_ATTR_EFFECT_PAUSED HU3D_MOTATTR_PAUSE
#define W01_FILE_EFFECT_ROOT 27
#define W01_FILE_EFFECT_CHILD 31
#define W01_MESSAGE_OPENING_1 MESSNUM(MESS_BOARD_W01, 22)
#define W01_MESSAGE_OPENING_2 MESSNUM(MESS_BOARD_W01, 23)
#define W01_MESSAGE_OPENING_3 MESSNUM(MESS_BOARD_W01, 24)
#define W01_MASU_ATTR_EVENT_GROUP_MASK 0x00001C00
#define W01_MASU_ATTR_EVENT_INDEX_MASK 0x00030000
#define W01_MASU_ATTR_MOVE_HOOK_DASH (1 << 8)
#define W01_MASU_ATTR_MOVE_HOOK_PATH (1 << 14)
#define W01_MASU_ATTR_MOVE_HOOK_SPRING_A (1 << 22)
#define W01_MASU_ATTR_MOVE_HOOK_SPRING_B (1 << 20)
#define W01_MASU_ATTR_MOVE_HOOK_FLOAT (1 << 18)
#define W01_MASU_ATTR_MOVE_HOOK_STAR (1 << 24)
#define W01_MASU_ATTR_HATENA_SHOP (1 << 3)
#define W01_MASU_ATTR_HATENA_STAR (1 << 29)
#define W01_MASU_ATTR_HATENA_EVENT (1 << 6)
#define W01_MASU_ATTR_HATENA_CAPSULE (1 << 15)
#define W01_MASU_ATTR_EVENT_TYPE_MASK 0xE0000000
#define W01_MASU_ATTR_LINK (1 << 13)

#define W01_DATA_DIR_LIGHT_EFFECT DATA_capsuleshop
#define W01_MODEL_LIGHT_DAY DATANUM(DATA_capsuleshop, 5)
#define W01_MODEL_LIGHT_NIGHT DATANUM(DATA_capsuleshop, 0)
#define W01_FILE_WH_EFFECT_STAR_ANIM 51
#define W01_MODEL_GATE_BASE DATANUM(DATA_capsuleshop, 8)
#define W01_MESSAGE_WH_INTRO_DAY MESSNUM(MESS_BOARD_W01, 4)
#define W01_MESSAGE_WH_INTRO_NIGHT MESSNUM(MESS_BOARD_W01, 9)
#define W01_MESSAGE_WH_CHOICE_DAY MESSNUM(MESS_BOARD_W01, 5)
#define W01_MESSAGE_WH_CHOICE_NIGHT MESSNUM(MESS_BOARD_W01, 10)
#define W01_MESSAGE_WH_PLAYER_MOVE_DAY MESSNUM(MESS_BOARD_W01, 6)
#define W01_MESSAGE_WH_PLAYER_MOVE_NIGHT MESSNUM(MESS_BOARD_W01, 11)
#define W01_MESSAGE_WH_CHOICE_HELP MESSNUM(MESS_BOARD_W01, 8)
#define W01_MESSAGE_WH_RESULT MESSNUM(MESS_BOARD_W01, 7)
#define W01_SOUND_WH_EVENT_DAY 987
#define W01_SOUND_WH_EVENT_NIGHT 961

// Board resource IDs selected by the current time of day.
#define W01_MODEL_NEXT_TURN_DAY DATANUM(DATA_w01, 37)
#define W01_MODEL_NEXT_TURN_NIGHT DATANUM(DATA_w01n, 40)
#define W01_MODEL_COIN_NIGHT DATANUM(DATA_w01n, 34)
#define W01_DATA_DIR_DAY W01_DATA_DAY
#define W01_DATA_DIR_NIGHT W01_DATA_NIGHT

// Player motions and messages used by the spring, next-turn, and star events.
#define W01_PLAYER_MOTION_SPRING_LAUNCH DATANUM(DATA_mariomot, 99)
#define W01_PLAYER_MOTION_STAR_COLLECT DATANUM(DATA_mariomot, 30)
#define W01_PLAYER_MOTION_STAR_REACTION DATANUM(DATA_mariomot, 25)
#define W01_PLAYER_MOTION_PATH_RIDE DATANUM(DATA_mariomot, 10)
#define W01_PLAYER_MOTION_BIRI_TOSS DATANUM(DATA_mariomot, 23)
#define W01_PLAYER_MOTION_BIRI_REACTION DATANUM(DATA_mariomot, 98)
#define W01_MESSAGE_NEXT_TURN_HELP MESSNUM(MESS_BOARD_OPE, 12)
#define W01_MESSAGE_STAR_DAY MESSNUM(MESS_BOARD_W01, 0)
#define W01_MESSAGE_STAR_NIGHT MESSNUM(MESS_BOARD_W01, 2)
#define W01_MESSAGE_STAR_RESULT_DAY MESSNUM(MESS_BOARD_W01, 1)
#define W01_MESSAGE_STAR_RESULT_NIGHT MESSNUM(MESS_BOARD_W01, 3)
#define W01_DATA_STAR_ANIM_DAY DATANUM(DATA_board, 99)
#define W01_DATA_STAR_ANIM_NIGHT DATANUM(DATA_board, 94)
#define W01_FILE_FLOATING_MASU 14
#define W01_FILE_MOTION_VARIANT_A 12
#define W01_FILE_MOTION_VARIANT_B 13
#define W01_FILE_MASU_MOTION_EFFECT 50
#define W01_FILE_COIN_FIELD_MAIN 32

#define W01_COLOR_TREE_BARK_BIT_PATTERN 0x685747FF
#define W01_COLOR_TREE_LIGHT_BIT_PATTERN 0xBEB095FF
#define W01_MODEL_BOARD_SCENERY_DAY_1 DATANUM(DATA_w01, 6)
#define W01_MODEL_BOARD_SCENERY_DAY_2 DATANUM(DATA_w01, 7)
#define W01_MODEL_BOARD_SCENERY_DAY_3 DATANUM(DATA_w01, 18)
#define W01_MODEL_BOARD_SCENERY_DAY_4 DATANUM(DATA_w01, 19)
#define W01_MODEL_BOARD_SCENERY_DAY_5 DATANUM(DATA_w01, 20)
#define W01_MODEL_BOARD_SCENERY_NIGHT_3 DATANUM(DATA_w01, 21)
#define W01_MODEL_COIN_FIELD_DAY DATANUM(DATA_w01, 33)
#define W01_MODEL_COIN_FIELD_NIGHT DATANUM(DATA_w01n, 33)
#define W01_MODEL_MASU_EFFECT_K_H1 DATANUM(DATA_w01, 15)
#define W01_MODEL_MASU_EFFECT_K_H2 DATANUM(DATA_w01, 16)
#define W01_MODEL_MASU_EFFECT_K_H3 DATANUM(DATA_w01, 17)
#define W01_MODEL_MASU_SF_DAY DATANUM(DATA_w01, 35)
#define W01_MODEL_MASU_MF_DAY DATANUM(DATA_w01, 38)
#define W01_MODEL_MASU_SF_NIGHT DATANUM(DATA_w01, 36)
#define W01_MODEL_MASU_MF_NIGHT DATANUM(DATA_w01, 39)
#define W01_MASU_ATTR_SPECIAL_A (1 << 22)
#define W01_MASU_ATTR_SPECIAL_A_LINK_0 (1 << 23)
#define W01_MASU_ATTR_SPECIAL_A_LINK_1 (1 << 26)
#define W01_MASU_ATTR_SPECIAL_B (1 << 20)
#define W01_MASU_ATTR_SPECIAL_B_LINK_0 (1 << 21)
#define W01_MASU_ATTR_SPECIAL_B_LINK_1 (1 << 25)
#define W01_MODEL_LIGHT_DETAIL_DAY_1 DATANUM(DATA_capsuleshop, 6)
#define W01_MODEL_LIGHT_DETAIL_DAY_2 DATANUM(DATA_capsuleshop, 7)
#define W01_MODEL_LIGHT_DETAIL_NIGHT_1 DATANUM(DATA_capsuleshop, 1)
#define W01_MODEL_LIGHT_DETAIL_NIGHT_2 DATANUM(DATA_capsuleshop, 4)
#define W01_MODEL_WH_EFFECT_DAY_1 DATANUM(DATA_w01, 28)
#define W01_MODEL_WH_EFFECT_DAY_2 DATANUM(DATA_w01, 29)
#define W01_MODEL_WH_EFFECT_DAY_3 DATANUM(DATA_w01, 30)

static inline BOOL MBTimeDayGet(void)
{
    return GwSystem.curTime == 0;
}

typedef void (*VoidFunc)(void);
typedef float (*W01CurveEval)();

typedef struct W01StarMasuWork {
    u8 unusedPrefix[4]; // Not accessed by the board's star-space code.
    u8 count; // Number of available star positions.
    u8 order[8]; // Order in which the star positions are visited.
    u8 index; // Current entry in the star-position order.
} W01_STAR_MASU_WORK;

typedef struct W01CoinEffect {
    s32 active;
    s16 modelId;
    s16 pad06;
    HuVecF pos;
    HuVecF vel;
    s16 time;
    s16 maxTime;
} W01_COIN_EFFECT;

typedef struct W01PathMarker {
    s16 modelId;
    s16 masuId;
} W01_PATH_MARKER;

typedef struct W01PathWork {
    s16 startMasuId;
    s16 endMasuId;
    HuVecF startPos;
    HuVecF endPos;
    HuVecF delta;
    float halfLength;
    float length;
    float angle;
    BOOL initF;
    W01_PATH_MARKER marker[6];
    s16 modelId[7];
    u8 unusedBytesBeforeEndpoints[18];
    HuVecF endPoint[2];
    s16 lastMasuId;
    u8 unusedBytesAtEnd[2];
} W01_PATH_WORK;

typedef struct W01MotionWork {
    s32 activePlayerNo; // Player attached to the moving path, or -1 when idle.
    MBMODELID modelId[3];
    float swingState[4]; // Two angles in radians, then their angular velocities.
    float firstMass; // First segment's relative mass in the swing calculation.
    float secondMass; // Second segment's relative mass in the swing calculation.
    float armLength[2]; // Segment lengths in board-space units.
    HuVecF anchorPos; // Board-space origin of the moving path assembly.
    HuVecF endpointPos[2]; // Board-space endpoints for the two segments.
} W01_MOTION_WORK;

typedef struct W01MasuMotionWork {
    BOOL active; // True while the space follows its playing model motion.
    MBMODELID modelId; // Animated scenery carrying the space.
    s16 masuId; // Board space positioned at the moving joint.
    HU3D_MODELID effectModelId[2]; // Reusable falling particle models.
    char objName[16]; // Model joint used to locate the space.
} W01_MASU_MOTION_WORK;

typedef struct W01MasuModelWork {
    BOOL active; // True when the route's space position should be refreshed.
    BOOL dayF; // True selects the long launch; false selects the shorter hop.
    MBMODELID modelId; // Spring route scenery model.
    s16 masuId; // Space at the spring model's moving joint.
    s16 linkMasuId[2]; // Destinations for the launch and hop routes, respectively.
    char objName[16]; // Model joint carrying the spring's board space.
} W01_MASU_MODEL_WORK;

typedef struct W01WhParticle {
    HuVecF pos; // Position in board space.
    HuVecF rot; // Particle orientation in degrees.
    HuVecF baseRot; // Orientation restored before presenting the rides.
    HuVecF velocity; // Translational velocity in board units per second.
    HuVecF acceleration; // Per-axis multiplier for the shared wind acceleration.
    s16 delay; // Frames before release; -1 keeps the particle attached.
    u8 unusedAfterDelay[2]; // Not accessed by the particle update.
    float phase; // Swing angle in radians after release.
    float phaseVelocity; // Change in swing angle per second.
} W01_WH_PARTICLE;

typedef struct W01WhEffectWork {
    MBMODELID modelId; // Ride model carrying the player.
    char objName[18]; // Model joint name loaded for this ride.
    W01_WH_PARTICLE *particle; // Particles surrounding the ride.
    s32 particleCount; // Number of entries in the particle array.
    s32 state; // Nonzero keeps all particles attached; zero allows timed releases.
    s16 fadeState; // One while fading out; zero leaves opacity unchanged.
    u8 unusedAfterFadeState[2]; // Not accessed by the ride update.
    s32 playerNo; // Rider's player number, or -1 when empty.
    HuVecF pos; // Target position; Y is 1000 units above the model's base.
    HuVecF bobVelocity; // Y is vertical velocity in board units per second; X and Z are only
                        // cleared.
    OMOBJ *object; // Object manager entry that updates this ride.
    s16 time; // Elapsed fade frames.
    s16 maxTime; // Fade duration in frames.
    u8 unusedAfterTimers[4]; // Not accessed by the ride update.
    s32 pathMotionF; // Nonzero follows the scripted path instead of vertical bobbing.
} W01_WH_EFFECT_WORK;

typedef struct W01WhEffectStorage {
    W01_WH_EFFECT_WORK work[3];
    s32 unusedTail; // Not accessed by the special-space event.
} W01_WH_EFFECT_STORAGE;

typedef struct W01SortEntry {
    HuVecF pos; // Particle position in camera space, used for depth sorting.
    s32 workIndex; // Ride index selecting the particle's glow color.
    float facing; // Camera-depth component of the local Y axis, used to shade the daytime glow.
} W01_SORT_ENTRY;

typedef struct W01BiriWork W01_BIRI_WORK;

typedef struct W01BiriEntry {
    MBMODELID modelId;
    HuVecF pos; // Current position in board space.
    HuVecF homePos; // Position used when the Biri returns to its starting point.
    HuVecF rot; // Current model rotation in degrees.
    s16 state; // Current movement state; zero is idle.
    s16 index; // Entry slot used to identify this Biri in the shared event state.
    float bobAmplitude; // World-space amplitude used by the idle bobbing motion.
    float spinAngle; // Current spin angle in degrees.
    float spinSpeed; // Spin angle change per update in degrees.
    float phaseAngle; // Initial angular offset used by this entry's board effect.
    float targetY; // Height in board space while circling a player.
    float orbitAngle; // Current angle around the player in degrees.
    float orbitDirection; // Direction of orbit, either 1.0f or -1.0f.
    float orbitRadius; // Current distance from the player in board units.
    float targetOrbitRadius; // Radius restored after the approach transition.
    float orbitRadiusTransition; // Radial velocity in board units per second while easing the orbit
                                 // radius.
    s16 delayTimer; // Frames remaining before the next idle movement begins.
    s16 timer; // Frames elapsed in the current movement state.
    s16 maxTime; // Frame limit for the current movement state.
    s16 idleDuration; // Idle interval in frames restored after a return movement.
    HuVecF startPos; // Starting point for the current travel segment.
    HuVecF targetPos; // Destination for the current travel segment.
    W01_BIRI_WORK *work; // Shared board effect state that owns this entry.
} W01_BIRI_ENTRY;

struct W01BiriWork {
    MBMODELID modelId; // Scenery model above the five Biri members.
    HuVecF pos; // Group model's fixed position in board space.
    HuVecF rot; // Group model's current orientation in degrees.
    HuVecF baseRot; // Orientation to which the group's swing is added.
    W01_BIRI_ENTRY entry[5]; // Independently moving Biri members.
    float phase; // Group swing angle in radians.
    float phaseVelocity; // Swing-angle change per second.
    s32 state; // Zero is ambient motion; one starts circling; two is the attack phase.
};

typedef struct W01ParticleWork {
    MBMODELID modelId[6];
    HU3D_MODELID particleModelId;
} W01_PARTICLE_WORK;

typedef struct W01GateWork {
    MBMODELID baseModelId;
    MBMODELID hookModelId;
    MBMODELID hookChildModelId;
    s16 masuId;
    HuVecF masuPos;
    HuVecF linkPos;
    HuVecF direction;
    float rotY;
} W01_GATE_WORK;

typedef struct W01SnowParticle {
    HuVecF pos;
    HuVecF vel;
    float angle;
    float angleSpeed;
    float scale;
    s32 textureNo;
} W01_SNOW_PARTICLE;

typedef struct W01NextTimeWork {
    BOOL active;
    BOOL forceEnd;
    MBMODELID modelId;
} W01_NEXT_TIME_WORK;

typedef struct W01SpringSegmentWork {
    MBMODELID modelId;
    HuVecF pos;
    HuVecF vel;
} W01_SPRING_SEGMENT_WORK;

typedef struct W01SpringWork {
    s16 startMasuId;
    s16 endMasuId;
    HuVecF startPos;
    HuVecF endPos;
    HuVecF dir;
    float angle;
    float length;
    s32 state;
    MBMODELID pathModelId[16];
    MBMODELID mainModelId;
    W01_SPRING_SEGMENT_WORK segment[4];
} W01_SPRING_WORK;

typedef struct W01FloatWork {
    s32 active;
    MBMODELID modelId;
    HuVecF pos;
    HuVecF velocity;
    s16 masuId;
    s16 time;
    s16 maxTime;
    float speed;
} W01_FLOAT_WORK;

typedef struct W01CoinEntry {
    BOOL active; // True while this falling object is being animated.
    MBMODELID modelId; // Day coin or night falling-object model.
    HuVecF vel; // Velocity in board units per second.
    s16 state; // Zero is the opening burst; one falls to a target; two bounces and blinks.
    s16 delay; // Frames until the falling object becomes visible.
    s16 blinkTime; // Frame counter alternating visibility after a night hit.
    s32 playerNo; // Player struck or awarded a coin, or -1 for an empty space.
    float targetY; // Board-space height that ends the current fall.
    HuVecF pos; // Current position in board space.
    HuVecF rot; // Model rotation in degrees.
    HU3D_MODELID particleModelId; // Landing effect for an unoccupied space.
} W01_COIN_ENTRY;

typedef struct W01Work13D8 {
    HuVecF pos; // Model joint from which the opening coin burst is launched.
    MBMODELID modelId; // Day/night scenery that triggers the falling objects.
    u8 unusedAfterModelId[2]; // Not accessed by the coin event.
    s32 count; // Falling-object capacity: 50 by day or 30 at night.
    W01_COIN_ENTRY *data; // Falling objects reused for each group of players.
    s16 playerMotion[4]; // Reaction motion loaded for each player during the event.
    float hitReactionAmount[4]; // Set to 0.5 when a night coin strikes each player.
    s16 coinCount[4]; // Available coins for loss effects, indexed by player or team.
} W01_WORK_13D8;

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern void ObjectSetup(void);
void mbCoinAddAllProcExecV(int *addNum, BOOL *dispF, BOOL fastF);

void MB1_Create(void);
void MB1_Kill(void);

void fn_1_13CC(void);
void fn_1_DD0(void);
void fn_1_644(void);
void fn_1_6D4(OMOBJ *obj);
s32 fn_1_6D98(void);
int fn_1_734(int playerNo, s16 id);
int fn_1_760(int playerNo, s16 id);
int MB1Ev_MasuHatena(int playerNo, s16 id);
void fn_1_914(int playerNo);
void fn_1_944(int playerNo);
void MB1_LightSet(void);
void MB1_LightReset(void);
void fn_1_9F4(int modelId, int shopNo);
void fn_1_AC4(int value);
void fn_1_B68(void);
void fn_1_10A0(void);
void fn_1_3214(float value);
void fn_1_2A08(void);
void fn_1_2A1C(void);
void fn_1_2D08(int playerNo);
void fn_1_4004(void);
void fn_1_44C0(void);
void fn_1_45A0(void);
void fn_1_4750(void);
void fn_1_47B0(void);
void fn_1_4C6C(s32 playerNo);
void fn_1_4CD0(void);
void fn_1_4D78(s16 masuId);
void fn_1_4D88(void);
void fn_1_4F94(int playerNo);
void fn_1_5560(s32 value);
void fn_1_5888(s32 index);
void fn_1_5C18(void);
void fn_1_5C7C(void);
void fn_1_5EF0(void);
void fn_1_5FB4(s32 playerNo, s16 id);
HU3D_MODELID fn_1_7388(HuVecF *pos);
void fn_1_7B20(void);
void fn_1_7B70(s32 value);
void fn_1_7C14(void);
void fn_1_7D14(W01_MASU_MOTION_WORK *work, HuVecF *pos);
void fn_1_7D84(HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx mtx);
void fn_1_80B4(HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx mtx);
void fn_1_7448(HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx mtx);
void fn_1_77AC(HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx mtx);
void fn_1_7944(void);
void fn_1_8474(void);
void fn_1_8798(void);
void fn_1_87E8(void);
void fn_1_8860(int playerNo);
void fn_1_9134(HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx mtx);
void fn_1_939C(HU3D_MODELID modelId, s32 particleNo, HuVecF *pos);
void fn_1_950C(BOOL dispF);
void fn_1_9574(void);
void fn_1_99E4(void);
void fn_1_9AEC(s32 playerNo, s16 id);
s32 fn_1_A82C(s32 playerNo);
void fn_1_ACDC(OMOBJ *object);
void fn_1_AFD4(s32 index, s32 state);
void fn_1_B01C(s32 index, s32 type);
void fn_1_B3F8(s32 index);
void fn_1_B7D8(s32 index);
void fn_1_B97C(s32 index);
void fn_1_B9A8(s32 index, s32 duration);
void fn_1_BFA0(HU3D_MODEL *model, Mtx *input);
s32 fn_1_B434(W01_WH_PARTICLE **particle);
void fn_1_BB44(W01_WH_EFFECT_WORK *work, const HuVecF *origin);
s32 fn_1_CCEC(const HuVecF *src, s32 triangleCount, float inset,
    HuVecF *dst);
s32 fn_1_D1D0(HuVecF *dst, float scale);
s32 fn_1_D320(HuVecF *src, s32 count, HuVecF *dst);
s32 fn_1_C7DC(void **displayList);
s32 fn_1_CAA4(void **displayList);
void fn_1_D50C(W01_SORT_ENTRY *entry, s32 first, s32 last);
void fn_1_E234(int playerNo);
void fn_1_DEA4(void);
void fn_1_ED00(void);
void fn_1_F0D0(void);
void fn_1_F1F4(s32 playerNo, s16 id);
void fn_1_F66C(W01_BIRI_ENTRY *entry);
void fn_1_10900(void);
void fn_1_10998(s32 count);
void fn_1_10C68(void);
void fn_1_10AA8(void);
void fn_1_10CB8(s32 playerNo, s16 id);
void fn_1_11C14(HuVecF *pos);
void fn_1_11C84(HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx mtx);
void fn_1_12090(void);
void fn_1_12454(void);
void fn_1_124C8(void);
void fn_1_12CF0(void);
void fn_1_12A4C(void);
void fn_1_126EC(s32 playerNo, s16 id);
void fn_1_12D58(HU3D_MODEL *modelP, Mtx *mtx);
u32 fn_1_130C4(void **displayList);
void fn_1_1330C(void);
void fn_1_139F8(void);
void fn_1_13B38(void);
void fn_1_13BC4(void);
void fn_1_139E8(s32 value);
void fn_1_13D04(void);
void fn_1_13D8C(const HuVecF *pos, s32 count);
void fn_1_13F50(void);
float fn_1_14A90(HuVecF *a, HuVecF *b, HuVecF *c, float t);
float fn_1_14BF0(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, float t);

extern s8 mbPadStkXGet(s32 playerNo);
extern int mbCoinAddProcExec(int playerNo, int coinNum, BOOL dispF,
    BOOL fastF);
void HuSprTexLoad(ANIMDATA *anim, s16 bmpNo, s16 texMapId,
    GXTexWrapMode wrapS, GXTexWrapMode wrapT, GXTexFilter filter);
float mbAngleEaseOut(float angleStart, float angleEnd, float weight);
BOOL mbAngleAdd(float *angle, float target, float speed);
void mbBezierCalcV(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *out, float t);
float mbCosRad(float angle);
void mbev_CapTeresaFadeCreate(MBMODELID modelId);
void mbev_CapTeresaFadeKill(MBMODELID modelId);
void mbev_CapTeresaFadeSet(float alpha);
void mbev_CapCallTeresa(s32 playerNo, s16 masuId);

extern W01_COIN_EFFECT lbl_1_bss_0[32];
extern ANIMDATA *lbl_1_bss_4A4[3];
extern HuVecF lbl_1_bss_480;
extern HuVecF lbl_1_bss_48C;
extern u32 lbl_1_bss_498;
extern void *lbl_1_bss_49C;
extern HU3D_MODELID lbl_1_bss_4A0;
extern W01_SNOW_PARTICLE lbl_1_bss_4B0[64];
extern s32 lbl_1_bss_EB0;
extern W01_GATE_WORK lbl_1_bss_EB4;
extern ANIMDATA *lbl_1_bss_EE4;
extern W01_PARTICLE_WORK lbl_1_bss_EE8;
extern s32 lbl_1_bss_EF8;
extern W01_BIRI_WORK lbl_1_bss_EFC;
extern float lbl_1_bss_1188[32];
extern float lbl_1_bss_1208;
extern u32 lbl_1_bss_120C[2];
extern void *lbl_1_bss_1214[2];
extern ANIMDATA *lbl_1_bss_121C;
extern HU3D_MODELID lbl_1_bss_1220;
extern HuVecF lbl_1_bss_1224;
extern s16 lbl_1_bss_1230[4];
extern MBMODELID lbl_1_bss_1238[4];
extern W01_WH_EFFECT_STORAGE lbl_1_bss_1240;
extern ANIMDATA *lbl_1_bss_1334;
extern W01_MASU_MODEL_WORK lbl_1_bss_1338[2];

extern HuVecF lbl_1_data_518;
extern HuVecF lbl_1_data_524;
extern s32 lbl_1_data_530[4];
extern s32 lbl_1_data_540[4];
extern HuVecF *lbl_1_data_5B4[];
extern s32 lbl_1_data_5C4[];
extern u32 lbl_1_data_578[2];
extern u32 lbl_1_data_580[2];
extern u32 lbl_1_data_588[2][3];
extern char *lbl_1_data_5AC[2];
extern ANIMDATA *lbl_1_bss_1378;
extern W01_MASU_MOTION_WORK lbl_1_bss_137C[3];
extern ANIMDATA *lbl_1_bss_13D0[2];
extern W01_WORK_13D8 lbl_1_bss_13D8;
extern HuVecF lbl_1_bss_1410;
extern HuVecF lbl_1_bss_141C;
extern HuVecF lbl_1_bss_1428;
extern s16 lbl_1_bss_1434;
extern s16 lbl_1_bss_1436;
extern W01_MOTION_WORK lbl_1_bss_1438[1];
extern s16 lbl_1_bss_1488;
extern W01_FLOAT_WORK *lbl_1_bss_148C;
extern s32 lbl_1_bss_1490;
extern W01_SPRING_WORK lbl_1_bss_1494;
extern W01_PATH_WORK lbl_1_bss_155C;
extern W01_NEXT_TIME_WORK lbl_1_bss_15E8;
extern s16 lbl_1_bss_15F4[17];
extern s16 lbl_1_bss_1616;
extern W01_STAR_MASU_WORK *lbl_1_bss_1618;
extern OSTick lbl_1_bss_161C;
extern float *lbl_1_bss_1620;

extern HuVecF lbl_1_data_42C[2][2];
extern HuVecF lbl_1_data_45C[2][2];
extern HuVecF lbl_1_data_7BC;
extern HuVecF lbl_1_data_7C8;
extern HuVecF lbl_1_data_7D4;
extern HuVecF lbl_1_data_7E0;
extern u32 lbl_1_data_3CC[6];

#include "REL/w01Dll/common.h"

HuVecF lbl_1_data_0[12] = {
    { 404.027008f, 1906.98206f, 1602.27002f },
    { 396.404999f, 2867.03394f, 1602.27002f },
    { 18.9190006f, 3527.53809f, 1154.26001f },
    { -754.16803f, 3896.76392f, 773.219971f },
    { -1705.32202f, 3967.57495f, 808.460022f },
    { -2371.5061f, 3583.61597f, 1288.19995f },
    { -2375.78809f, 3020.53711f, 2043.45996f },
    { -1626.78699f, 2637.13989f, 2411.20996f },
    { -698.375977f, 2401.67603f, 2472.97998f },
    { 53.3510017f, 1964.24194f, 2809.17993f },
    { 181.735001f, 1186.63196f, 3321.38989f },
    { -174.188995f, 353.940002f, 3609.05005f },
};

HuVecF lbl_1_data_90[12] = {
    { 404.802002f, 1890.11499f, 1602.85999f },
    { 439.242004f, 2983.51807f, 1617.85999f },
    { 927.995972f, 3885.56396f, 1689.91003f },
    { 1776.69104f, 3710.54199f, 2248.30005f },
    { 1953.88196f, 3086.91504f, 3105.08008f },
    { 1433.67896f, 2476.20288f, 3817.66992f },
    { 473.901001f, 2028.14001f, 3948.48999f },
    { -401.07901f, 2214.43799f, 3363.45996f },
    { -1225.10095f, 2654.67603f, 2796.34009f },
    { -2118.948f, 3099.05493f, 2348.8999f },
    { -3120.92212f, 3248.05298f, 1954.5f },
    { -4150.38916f, 3105.95801f, 1612.21997f },
};

HuVecF lbl_1_data_120[12] = {
    { 408.268005f, 1911.20703f, 1602.91003f },
    { 433.165009f, 2978.89502f, 1595.72998f },
    { 1236.23901f, 3596.29907f, 1349.63f },
    { 1914.06299f, 4131.42383f, 785.950012f },
    { 1573.44299f, 4661.84521f, -1.50999999f },
    { 544.370972f, 4590.06787f, -155.270004f },
    { -261.341003f, 4232.13916f, 424.76001f },
    { -1040.27795f, 3881.41211f, 1059.76001f },
    { -2057.96509f, 3734.85693f, 1021.5f },
    { -2606.16602f, 3961.7251f, 195.229996f },
    { -2294.73608f, 4305.59277f, -733.590027f },
    { -1306.53503f, 4351.29004f, -1093.0f },
};

HuVecF lbl_1_data_1B0[12] = {
    { 403.17099f, 1884.61694f, 1602.96997f },
    { 382.664001f, 2773.30005f, 1602.95996f },
    { -54.8709984f, 3486.78589f, 1583.27002f },
    { -745.963013f, 3927.35303f, 1297.28003f },
    { -834.242981f, 4531.97705f, 683.26001f },
    { -268.617004f, 4994.30078f, 244.289993f },
    { 581.713989f, 4841.61279f, 301.369995f },
    { 1314.87903f, 4417.08301f, 566.929993f },
    { 2000.25f, 3940.82007f, 873.309998f },
    { 2682.96606f, 3462.44092f, 1182.35999f },
    { 3422.87598f, 3048.47095f, 1444.17004f },
    { 4277.73877f, 2853.7959f, 1547.18994f },
};

HuVecF lbl_1_data_240[33] = {
    { 2920.08008f, 1710.0f, 2709.8501f },
    { 3089.85205f, 1706.49597f, 2852.22998f },
    { 3166.70996f, 1706.47205f, 2911.93994f },
    { 3250.04907f, 1706.69495f, 2961.80005f },
    { 3341.96191f, 1707.15503f, 2992.53003f },
    { 3438.21191f, 1703.54602f, 3002.6001f },
    { 3533.875f, 1697.70496f, 2991.15991f },
    { 3622.98096f, 1684.93701f, 2956.05005f },
    { 3702.10596f, 1666.28503f, 2902.58008f },
    { 3770.43896f, 1646.99902f, 2836.01001f },
    { 3822.20508f, 1628.02795f, 2756.01001f },
    { 3849.38599f, 1610.38403f, 2664.43994f },
    { 3855.09302f, 1594.14099f, 2568.65991f },
    { 3844.302f, 1577.95605f, 2473.30005f },
    { 3813.25195f, 1559.65198f, 2383.1001f },
    { 3757.56396f, 1534.15698f, 2307.8501f },
    { 3687.271f, 1497.80005f, 2251.36011f },
    { 3612.48706f, 1450.34302f, 2211.08008f },
    { 3538.58203f, 1392.55896f, 2185.26001f },
    { 3467.92212f, 1326.41199f, 2175.30005f },
    { 3401.72412f, 1255.21399f, 2180.31006f },
    { 3339.53296f, 1181.96399f, 2196.31006f },
    { 3279.57104f, 1108.56702f, 2218.93994f },
    { 3220.48389f, 1036.03894f, 2246.22998f },
    { 3160.87012f, 965.549988f, 2277.40991f },
    { 3098.6709f, 899.880981f, 2313.58008f },
    { 3032.37109f, 842.26001f, 2355.63989f },
    { 2962.41089f, 793.103027f, 2402.31006f },
    { 2890.0459f, 750.547974f, 2451.78003f },
    { 2816.50391f, 711.81897f, 2502.6499f },
    { 2742.33594f, 675.369995f, 2554.30005f },
    { 2667.83594f, 640.229004f, 2606.37988f },
    { 2593.18506f, 605.703003f, 2658.65991f },
};
extern HuVecF lbl_1_data_4E0[1];
extern int *lbl_1_data_4FC[2];
extern char lbl_1_data_504[];
extern char lbl_1_data_50D[];
extern HuVecF lbl_1_data_48C[7];
extern u32 lbl_1_data_550[3];
extern char *lbl_1_data_56C[3];
extern int lbl_1_data_5D4[4];
extern int lbl_1_data_5E4[4];
extern u32 lbl_1_data_5F4[3];
extern char *lbl_1_data_610[3];
extern HuVecF lbl_1_data_61C;
extern HuVecF lbl_1_data_628;
extern HuVecF lbl_1_data_644;
extern HuVecF lbl_1_data_650;
extern GXColor lbl_1_data_634;
extern char lbl_1_data_638[12];
extern char lbl_1_data_420[12];
extern u32 *lbl_1_data_3F4[2];
extern HuVecF lbl_1_data_3FC;
extern HuVecF lbl_1_data_408;
extern HuVecF lbl_1_data_414;
extern HuVecF lbl_1_data_674[4];
extern s32 lbl_1_data_6A4[12];
extern HuVecF lbl_1_data_6D4[6];
extern s32 lbl_1_data_71C[25];
extern GXColor lbl_1_data_65C[3];
extern GXColor lbl_1_data_668[3];
extern u32 lbl_1_data_7A8[5];
extern u32 lbl_1_data_7F8[3];
extern HuVecF lbl_1_data_780;

// The board loader runs constructors and registers the board when this module opens.
int _prolog(void)
{
    const VoidFunc *ctors = _ctors;

    while (*ctors != 0) {
        (**ctors)();
        ctors++;
    }
    ObjectSetup();
    return 0;
}

// The board loader runs the registered destructors when this module closes.
void _epilog(void)
{
    const VoidFunc *dtors = _dtors;

    while (*dtors != 0) {
        (**dtors)();
        dtors++;
    }
}

// The board loader calls this to register Towering Treetop setup and teardown.
void ObjectSetup(void)
{
    GWPartySet(TRUE);
    mbObjectSetup(MBNO_WORLD_1, MB1_Create, MB1_Kill);
}

// ObjectSetup registers this initializer; it loads the time-of-day board data
// and installs the board's movement, event, camera, and turn callbacks.
void MB1_Create(void)
{
    s16 *modelIds = lbl_1_bss_15F4;
    int boardNo = MBBoardNoGet();
    int timeOfDayIndex;
    int modelIndex;

    HuAudSndGrpSetSet(MSM_GRP_BRD01);
    lbl_1_bss_1618 = (W01_STAR_MASU_WORK *)GwSystem.boardWork;
    mbObjDirSet(W01_DATA_DAY, W01_DATA_NIGHT);
    mbMasuInit(MBTimeDayGet() ? W01_DATA_DAY : W01_DATA_NIGHT);
    modelIds[0] = mbObjCreate(
        (MBTimeDayGet() ? W01_DATA_DAY : W01_DATA_NIGHT) | W01_FILE_MASU, 0, FALSE);
    mbObjAttrSet(modelIds[0], W01_OBJ_ATTR_BOARD_MODEL);
    timeOfDayIndex = MBTimeDayGet() ? 0 : 1;
    for (modelIndex = 0; lbl_1_data_3F4[timeOfDayIndex][modelIndex] != 0; modelIndex++) {
        (&modelIds[1])[modelIndex] = mbObjCreate(
            (lbl_1_data_3F4[timeOfDayIndex][modelIndex] & 0xFFFF)
                | (MBTimeDayGet() ? W01_DATA_DAY : W01_DATA_NIGHT),
            0, FALSE);
        mbObjAttrSet((&modelIds[1])[modelIndex], W01_OBJ_ATTR_BOARD_MODEL);
    }
    (&modelIds[1])[modelIndex] = mbObjCreate(
        (MBTimeDayGet() ? W01_DATA_DAY : W01_DATA_NIGHT) | W01_FILE_GATE, 0, FALSE);
    mbObjLayerSet((&modelIds[1])[modelIndex], 1);
    modelIndex++;
    mbScrollInit((MBTimeDayGet() ? W01_DATA_DAY : W01_DATA_NIGHT) | W01_FILE_SCROLL);
    mbCapThrowColCreate(
        (MBTimeDayGet() ? W01_DATA_DAY : W01_DATA_NIGHT) | W01_FILE_CAP_THROW_COLLISION);
    mbLightFuncSet(MB1_LightSet, MB1_LightReset);
    if (mbSaveNewF) {
        memset(lbl_1_bss_1618, 0, 16);
        fn_1_139F8();
    }
    mbPlayerTurnInitHookSet(fn_1_914);
    mbPlayerTurnCloseHookSet(fn_1_944);
    fn_1_13B38();
    mbOpeningInstHookSet(fn_1_644);
    mbOpeningStarInstHookSet(0);
    mbOpeningViewSet(&lbl_1_data_408, &lbl_1_data_414, 10000.0f);
    mbev_ShopExInit(
        (MBTimeDayGet() ? W01_DATA_DAY : W01_DATA_NIGHT) | W01_FILE_SHOP, fn_1_9F4);
    mbev_ShopBackCreate(
        (MBTimeDayGet() ? W01_DATA_DAY : W01_DATA_NIGHT) | W01_FILE_SHOP_RETURN,
        -1, -1, TRUE);
    mbev_MasuMoveEndSet(fn_1_760);
    mbev_MasuMoveStartSet(fn_1_734);
    mbev_MasuHatenaSet(MB1Ev_MasuHatena);
    mbStarMoveHookSet(fn_1_13BC4);
    mbMapCameraSet(0, &lbl_1_data_3FC, 18500.0f);
    mbMapHookSet(fn_1_AC4);
    mbev_NextTimeSet(fn_1_B68);
    omAddObjEx(mbObjMan, 8204, 0, 0, -1, fn_1_6D4);
    fn_1_10A0();
    fn_1_2A1C();
    fn_1_45A0();
    fn_1_4D88();
    fn_1_5C7C();
    fn_1_7944();
    fn_1_8474();
    fn_1_9574();
    fn_1_ED00();
    fn_1_DEA4();
    fn_1_124C8();
    fn_1_10AA8();
    fn_1_12A4C();
    fn_1_13D04();
    HuDataDirClose(MBTimeDayGet() ? W01_DATA_DAY : W01_DATA_NIGHT);
}

// ObjectSetup registers this cleanup callback when the board is loaded.
void MB1_Kill(void)
{
    fn_1_4750();
    fn_1_7B20();
    fn_1_8798();
    fn_1_99E4();
    fn_1_10C68();
    fn_1_12CF0();
    fn_1_5EF0();
}

// MB1_Create installs this as the opening instruction callback; it shows the
// three Towering Treetop introduction messages before the board starts.
void fn_1_644(void)
{
    s16 winNo;

    winNo = mbWinCreate(2, W01_MESSAGE_OPENING_1, mbGuideSpeakerNoGet());
    mbWinWait(winNo);
    winNo = mbWinCreate(2, W01_MESSAGE_OPENING_2, mbGuideSpeakerNoGet());
    mbWinWait(winNo);
    winNo = mbWinCreate(2, W01_MESSAGE_OPENING_3, mbGuideSpeakerNoGet());
    mbWinWait(winNo);
}

// MB1_Create adds this object to mbObjMan; it runs board updates until exit.
void fn_1_6D4(OMOBJ *obj)
{
    if (mbExitCheck()) {
        omDelObjEx((OMOBJMAN *)HuPrcCurrentGet(), obj);
    } else {
        fn_1_13CC();
        fn_1_4004();
        fn_1_47B0();
        fn_1_5560(0);
        fn_1_7C14();
        fn_1_F0D0();
        fn_1_1330C();
        fn_1_13F50();
    }
}

// MB1_Create registers this callback when a player starts moving between spaces.
int fn_1_734(int playerNo, s16 id)
{
    fn_1_4D78(id);
    return 0;
}

// MB1_Create registers this callback after a space move to dispatch its effects.
int fn_1_760(int playerNo, s16 id)
{
    u32 mAttr = mbMasuMAttrGet(id);

    if (mAttr & W01_MASU_ATTR_EVENT_INDEX_MASK) {
        fn_1_7B70(mbev_MasuBitGet(mAttr, W01_MASU_ATTR_EVENT_INDEX_MASK) - 1);
    }
    if (mAttr & W01_MASU_ATTR_MOVE_HOOK_DASH) {
        mbPlayerMoveHookSet(playerNo, fn_1_2D08);
    }
    if (mAttr & W01_MASU_ATTR_MOVE_HOOK_PATH) {
        mbPlayerMoveHookSet(playerNo, fn_1_4F94);
    }
    if ((mAttr & W01_MASU_ATTR_MOVE_HOOK_SPRING_A)
        || (mAttr & W01_MASU_ATTR_MOVE_HOOK_SPRING_B)) {
        mbPlayerMoveHookSet(playerNo, fn_1_8860);
    }
    if (mAttr & W01_MASU_ATTR_MOVE_HOOK_FLOAT) {
        mbPlayerMoveHookSet(playerNo, fn_1_E234);
    }
    if (mAttr & W01_MASU_ATTR_MOVE_HOOK_STAR) {
        fn_1_126EC(playerNo, id);
    }
    return 0;
}

// MB1_Create installs this space-event callback for question-mark spaces.
int MB1Ev_MasuHatena(int playerNo, s16 id)
{
    u32 mAttr = mbMasuMAttrGet(id);

    if (mAttr & W01_MASU_ATTR_HATENA_SHOP) {
        fn_1_5FB4(playerNo, id);
        return 0;
    }
    if (mAttr & W01_MASU_ATTR_HATENA_STAR) {
        fn_1_9AEC(playerNo, id);
    }
    if (mAttr & W01_MASU_ATTR_HATENA_EVENT) {
        fn_1_F1F4(playerNo, id);
    }
    if (mAttr & W01_MASU_ATTR_HATENA_CAPSULE) {
        fn_1_10CB8(playerNo, id);
    }
    return 1;
}

// MB1_Create registers this hook before each player's turn begins.
void fn_1_914(int playerNo)
{
    fn_1_87E8();
    fn_1_12454();
    fn_1_4C6C(playerNo);
}

// MB1_Create registers this hook after each player's turn ends.
void fn_1_944(int playerNo)
{
    fn_1_2A08();
    fn_1_4CD0();
    fn_1_5C18();
    fn_1_44C0();
}

// mbLightFuncSet installs this callback to enable board lighting and daytime fog.
void MB1_LightSet(void)
{
    s16 *modelId = lbl_1_bss_15F4;

    Hu3DModelLightInfoSet(mbObjModelIDGet(modelId[0]), TRUE);
    if (GwSystem.curTime == 0) {
        Hu3DFogSet(6000.0f, 70000.0f, 200, 200, 200);
    }
}

// mbLightFuncSet installs this callback when board lighting is being cleared.
void MB1_LightReset(void)
{
}

// mbev_ShopExInit registers this callback to attach the day or night shop display; it ignores
// shopNo.
void fn_1_9F4(int modelId, int shopNo)
{
    BOOL dayF = (GwSystem.curTime == 0);
    s16 objId;

    objId = mbObjCreate(
        (dayF ? W01_DATA_DAY : W01_DATA_NIGHT) | W01_FILE_SHOP_MODEL,
        NULL, TRUE);
    mbObjAttrSet(objId, W01_OBJ_ATTR_BOARD_MODEL);
    mbObjHookSet((s16)modelId, lbl_1_data_420, objId);
    mbObjCullRadiusSet((s16)modelId, 400.0f);
    mbObjCullRadiusSet(objId, 400.0f);
}

// The map-view hook suspends ambient particles and daytime fog while the map is open.
void fn_1_AC4(int mapOpenF)
{
    if (mapOpenF != 0) {
        fn_1_139E8(0);
        if (GwSystem.curTime == 0) {
            Hu3DFogClear();
        }
    } else {
        fn_1_139E8(1);
        if (GwSystem.curTime == 0) {
            Hu3DFogSet(6000.0f, 70000.0f, 200, 200, 200);
        }
    }
}

// mbev_NextTimeSet installs this callback to show the display during time changes.
void fn_1_B68(void)
{
    W01_NEXT_TIME_WORK *work = &lbl_1_bss_15E8;
    HUPROCESS *process;
    s16 winNo;
    BOOL dayF;
    s32 i;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerDispSet(i, FALSE);
    }
    memset(work, 0, sizeof(*work));
    if (GwSystem.curTime == 0) {
        work->modelId = mbObjCreate(W01_MODEL_NEXT_TURN_DAY, NULL, FALSE);
    } else {
        work->modelId = mbObjCreate(W01_MODEL_NEXT_TURN_NIGHT, NULL, FALSE);
    }
    mbObjMotionSpeedSet(work->modelId, 0.0f);
    mbObjMotionShapeSpeedSet(work->modelId, 0.0f);
    fn_1_950C(FALSE);
    process = HuPrcChildCreate(fn_1_DD0, 8199, 32768, 0, mbMainProc);
    winNo = -1;
    do {
        if (work->active != FALSE) {
            if (winNo < 0) {
                winNo = mbWinCreateHelp(W01_MESSAGE_NEXT_TURN_HELP);
                mbWinPosSet(winNo, 228, 408);
            }
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                if (GwPlayer[i].comF == FALSE
                    && (HuPadBtnDown[GwPlayer[i].padNo] & PAD_BUTTON_START)) {
                    HuPrcKill(process);
                    work->forceEnd = TRUE;
                    break;
                }
            }
        }
        HuPrcVSleep();
    } while (work->forceEnd == FALSE);
    mbWipeWait();
    if (WipeCheckIn() == FALSE) {
        mbWipeFadeOut();
    }
    mbWinKill(winNo);
    mbObjKill(work->modelId);
    fn_1_950C(TRUE);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerDispSet(i, TRUE);
    }
    dayF = (GwSystem.curTime == 0);
    HuDataDirClose(dayF ? W01_DATA_DAY : W01_DATA_NIGHT);
}

// fn_1_B68 starts this process to show the time-change title and tour the changed board features.
void fn_1_DD0(void)
{
    W01_NEXT_TIME_WORK *work = &lbl_1_bss_15E8;
    HuVecF pos;
    s16 masuId;
    s16 time;

    pos.x = pos.y = pos.z = 0.0f;
    mbCameraMovePos(&pos, NULL, NULL, 10000.0f, -1.0f, 1);
    mbCameraMoveWait();
    mbWipeFadeIn();
    mbTelopTimeChangeCreate();
    while (mbTelopTimeChangeCheck()) {
        HuPrcVSleep();
    }
    mbWipeFadeOut();
    work->active = TRUE;
    if (GwSystem.curTime == 0) {
        masuId = mbMasuFind_MAttrIdGet(-1, W01_MASU_ATTR_MOVE_HOOK_SPRING_A);
        time = 0;
    } else {
        masuId = mbMasuFind_MAttrIdGet(-1, W01_MASU_ATTR_MOVE_HOOK_SPRING_B);
        time = 1;
    }
    mbMasuPosGet(masuId, &pos);
    pos.y += 1000.0f;
    mbCameraMovePos(&pos, &lbl_1_data_42C[time][0], NULL,
        5000.0f, -1.0f, 1);
    HuPrcVSleep();
    mbCameraMoveMasu(masuId, &lbl_1_data_42C[time][1], NULL,
        2000.0f, -1.0f, 240);
    mbCameraCurveTypeSet(1);
    mbWipeFadeIn();
    HuPrcSleep(60);
    mbObjMotionSpeedSet(work->modelId, 1.0f);
    mbObjMotionShapeSpeedSet(work->modelId, 1.0f);
    HuPrcSleep(120);
    mbWipeFadeOut();
    masuId = mbMasuFind_MAttrIdGet(-1, W01_MASU_ATTR_HATENA_SHOP);
    mbMasuPosGet(masuId, &pos);
    pos.y -= 500.0f;
    mbCameraMovePos(&pos, &lbl_1_data_45C[time][0], NULL,
        3000.0f, -1.0f, 1);
    HuPrcVSleep();
    pos.y += 1000.0f;
    mbCameraMovePos(&pos, &lbl_1_data_45C[time][1], NULL,
        1000.0f, -1.0f, 240);
    mbCameraCurveTypeSet(1);
    mbWipeFadeIn();
    HuPrcSleep(180);
    mbWipeFadeOut();
    work->forceEnd = TRUE;
    HuPrcEnd();
}

// MB1_Create builds the six spaces and seven connecting models along the upper curved path.
void fn_1_10A0(void)
{
    W01_PATH_WORK *work = &lbl_1_bss_155C;
    W01_PATH_MARKER *marker = work->marker;
    u32 attr;
    BOOL dayF;
    BOOL dayF2;
    s32 i;

    work->startPos.x = -1123.03f;
    work->startPos.y = 3430.0f;
    work->startPos.z = -152.14f;
    work->startMasuId = mbMasuFind_MAttrIdGet(-1, (1 << 4));
    work->endPos.x = 1090.968f;
    work->endPos.y = 3430.0f;
    work->endPos.z = 68.8458f;
    work->endMasuId = mbMasuFind_MAttrIdGet(-1, (1 << 5));
    PSVECSubtract(&work->endPos, &work->startPos, &work->delta);
    work->length = PSVECMag(&work->delta);
    work->angle = (float)((atan2(-work->delta.x, -work->delta.z) / M_PI)
        * 180.0);
    work->halfLength = 0.5f * work->length;
    for (i = 0; i < 6; i++, marker++) {
        dayF = (GwSystem.curTime == 0);
        marker->modelId = mbObjCreate(
            (dayF ? W01_DATA_DAY : W01_DATA_NIGHT) | W01_FILE_PATH_MARKER,
            NULL, TRUE);
        mbObjRotYSet(marker->modelId, work->angle);
        attr = mbev_MasuAttrGet(i + 1, 7);
        marker->masuId = mbMasuFind_MAttrMatchIdGet(-1, attr, 7);
    }
    work->endPoint[0].x = -1213.0f
        - (100.0f * (0.5f * mbSinDeg(work->angle)));
    work->endPoint[0].y = 3489.0f;
    work->endPoint[0].z = -161.0f
        - (100.0f * (0.5f * mbCosDeg(work->angle)));
    work->endPoint[1].x = 1169.0f
        + (100.0f * (0.5f * mbSinDeg(work->angle)));
    work->endPoint[1].y = 3489.0f;
    work->endPoint[1].z = 76.0f
        + (100.0f * (0.5f * mbCosDeg(work->angle)));
    for (i = 0; i < 7; i++) {
        dayF2 = (GwSystem.curTime == 0);
        work->modelId[i] = mbObjCreate(
            (dayF2 ? W01_DATA_DAY : W01_DATA_NIGHT) | W01_FILE_PATH_MODEL,
            NULL, TRUE);
    }
    work->initF = TRUE;
}

#define FN13CC_HEIGHT \
    ( \
        ( \
            ((sideLength * sideLength * sideLength * sideLength) \
                + ((halfLength * halfLength * halfLength * halfLength) \
                    + (spanLength * spanLength * spanLength * spanLength))) \
            - ( \
                2.0f * ( \
                    (2.0f * spanLength * spanLength * sideLength \
                        * sideLength) \
                    + ( \
                        ((halfLength * halfLength * sideLength * sideLength) \
                            + (halfLength * halfLength * spanLength \
                                * spanLength)) \
                        - (spanLength * spanLength * sideLength * sideLength) \
                    ) \
                ) \
            ) \
        ) \
    ) \
    / (4.0f * spanLength * spanLength)

// Estimate length from zero to endT using ten trapezoid intervals.
// w01CurveT uses this for upper-path and spring-route parameter corrections.
static inline float w01CurveLen2(W01CurveEval eval, HuVecF *p0, HuVecF *p1,
    HuVecF *p2, float endT)
{
    float baseT;
    float sampleT;
    float deltaT;
    float sampleLength;
    s32 div;
    s32 sampleNo;

    div = 10;
    baseT = 0.0f;
    deltaT = (endT - baseT) / div;
    sampleT = baseT;
    sampleLength = 0.0f;
    for (sampleNo = 0; sampleNo < div - 1; sampleNo++) {
        sampleT += deltaT;
        sampleLength += eval(p0, p1, p2, NULL,
            sampleT);
    }
    sampleLength = deltaT * 0.5
        * (eval(p0, p1, p2, NULL, baseT)
            + eval(p0, p1, p2, NULL, endT)
            + (2.0 * sampleLength));
    return sampleLength;
}

// Correct the quadratic path parameter for a distance using up to ten steps.
// Speeds below 0.1 in magnitude use 1.0 as the correction divisor.
static inline float w01CurveT(W01CurveEval eval, HuVecF *p0, HuVecF *p1,
    HuVecF *p2, float workT, float distance)
{
    float result;
    float slope;
    float oldT;
    float minLength;
    s32 stepCount;

    minLength = 0.1f;
    stepCount = 0;
    do {
        result = w01CurveLen2(eval, p0, p1, p2, workT) - distance;
        if (fabs(slope = eval(p0, p1, p2, NULL, workT)) < minLength) {
            slope = 1.0f;
        }
        oldT = workT;
        workT -= result / slope;
        stepCount++;
    } while (workT != oldT && stepCount < 10);
    return workT;
}

// Estimate the full quadratic path length with four midpoint refinements.
// Upper-path and spring-route positioning use the result to space their models along the curve.
static inline float w01CurveLen(W01CurveEval eval, HuVecF *p0, HuVecF *p1,
    HuVecF *p2)
{
    float baseT;
    float integrationStep;
    float length;
    float lengthSum;
    float simpsonLength;
    s32 count;
    s32 j;
    s32 divCount;

    count = 10;
    baseT = 0.0f;
    simpsonLength = 0.0f;
    integrationStep = 1.0f - baseT;
    length = 0.5f * (integrationStep
        * (eval(p0, p1, p2, NULL, baseT)
            + eval(p0, p1, p2, NULL, 1.0f)));
    for (divCount = 1; divCount <= count; divCount *= 2) {
        lengthSum = 0.0f;
        for (j = 1; j <= divCount; j++) {
            lengthSum += eval(p0, p1, p2, NULL,
                baseT + (integrationStep * ((float)j - 0.5f)));
        }
        lengthSum *= integrationStep;
        simpsonLength = (1.0f / 3.0f)
            * (length + (2.0f * lengthSum));
        integrationStep *= 0.5f;
        length = 0.5f * (length + lengthSum);
    }
    return simpsonLength;
}

// The board update eases the upper path's bend and repositions its models and spaces.
// A player on the path with movement finished supplies the bend target; otherwise use the midpoint.
void fn_1_13CC(void)
{
    W01_PATH_WORK *work;
    W01_PATH_MARKER *marker;
    HuVecF pos;
    HuVecF curvePos;
    HuVecF controlPos;
    HuVecF prevPos;
    HuVecF delta;
    HuVecF rot;
    Mtx matrix;
    s32 playerNo;
    s32 playerMasuId;
    s32 masuId;
    s32 currentMasuId;
    s32 modelNo;
    s32 i;
    float height;
    float halfLength;
    float sideLength;
    float spanLength;
    float finalLength;
    float distance;
    float t;
    float targetLength;

    work = &lbl_1_bss_155C;
    marker = work->marker;
    playerNo = GwSystem.turnPlayerNo;
    if (playerNo >= 0) {
        playerMasuId = GwPlayer[playerNo].masuId;
        masuId = work->startMasuId;
        while (1) {
            if (masuId == playerMasuId) {
                break;
            }
            currentMasuId = masuId;
            masuId = mbMasuMAttrFindLink(masuId, 7);
            if (masuId <= 0) {
                break;
            }
        }
        if (masuId == playerMasuId) {
            mbPlayerPosGet(playerNo, &pos);
            t = ((pos.z * work->delta.z)
                + (((pos.y * work->delta.y)
                    + ((pos.x * work->delta.x)
                        - (work->startPos.x * work->delta.x)))
                    - (work->startPos.y * work->delta.y))
                - (work->startPos.z * work->delta.z))
                / PSVECSquareMag(&work->delta);
            if (mbPlayerWorkGet(playerNo)->moveEndF != TRUE || t < 0.0f
                || t >= 1.0f) {
                targetLength = 0.5f * work->length;
            } else {
                targetLength = t * work->length;
            }
        } else {
            goto default_target_length;
        }
    } else {
    default_target_length:
        // When entered with no active player, this fallback leaves masuId unset for the later sound
        // check.
        targetLength = 0.5f * work->length;
    }
    if (work->initF == FALSE) {
        work->halfLength += 0.05f * (targetLength - work->halfLength);
        if (fabs(targetLength - work->halfLength) < 1.0f) {
            return;
        }
    } else {
        work->halfLength = targetLength;
        work->initF = FALSE;
        // The initial pass reads the current player's space without rechecking whether a player is
        // active.
        work->lastMasuId = GwPlayer[playerNo].masuId;
    }
    if (masuId >= 0 && work->lastMasuId != masuId) {
        mbAudFXPlay(0x594);
        work->lastMasuId = playerMasuId;
    }
    halfLength = work->halfLength;
    sideLength = work->length - halfLength;
    spanLength = 1.2f * work->length;
    height = sqrtf(FN13CC_HEIGHT);
    t = work->halfLength / work->length;
    controlPos.x = work->startPos.x + (t * work->delta.x);
    controlPos.y = work->startPos.y + (t * work->delta.y) - height;
    controlPos.z = work->startPos.z + (t * work->delta.z);
    finalLength = w01CurveLen((W01CurveEval)(u32)fn_1_14A90, &work->startPos,
        &controlPos, &work->endPos);
    t = 0.0f;
    distance = finalLength / 6.0f;
    prevPos = work->startPos;
    for (i = 0; i < 6; i++, marker++) {
        if (i < 5) {
            t = w01CurveT((W01CurveEval)(u32)fn_1_14A90, &work->startPos,
                &controlPos, &work->endPos, t, distance);
            mbBezierCalcV(&work->startPos, &controlPos, &work->endPos,
                &curvePos, t);
        } else {
            curvePos = work->endPos;
        }
        PSVECSubtract(&curvePos, &prevPos, &delta);
        pos.x = prevPos.x + (0.5f * delta.x);
        pos.y = prevPos.y + (0.5f * delta.y);
        pos.z = prevPos.z + (0.5f * delta.z);
        mbObjPosSetV(marker->modelId, &pos);
        mbObjRotGet(marker->modelId, &rot);
        rot.x = (float)((atan2(delta.y,
            sqrtf((delta.z * delta.z) + (delta.x * delta.x))) / M_PI)
            * 180.0);
        mbObjRotSetV(marker->modelId, &rot);
        mbMtxRot(matrix, rot.x, rot.y, rot.z);
        delta.x = 0.0f;
        delta.y = 50.0f;
        delta.z = 0.0f;
        PSMTXMultVec(matrix, &delta, &delta);
        mbMasuPosSet(marker->masuId, pos.x + delta.x, pos.y + delta.y,
            pos.z + delta.z);
        mbMasuRotSet(marker->masuId, 0.0f, 0.0f, rot.x);
        prevPos = curvePos;
        distance += finalLength / 6.0f;
    }
    modelNo = 0;
    marker = work->marker;
    for (i = 0; i < 7; i++, marker++) {
        if (i == 0) {
            mbObjPosGet(marker->modelId, &pos);
            pos.y -= 30.000002f;
            prevPos = pos;
            PSVECSubtract(&pos, &work->endPoint[0], &delta);
            PSVECNormalize(&delta, &delta);
            pos.x = work->endPoint[0].x + (100.0f * (0.5f * delta.x));
            pos.y = work->endPoint[0].y + (100.0f * (0.5f * delta.y));
            pos.z = work->endPoint[0].z + (100.0f * (0.5f * delta.z));
            mbObjPosSetV(work->modelId[modelNo], &pos);
            rot.x = HuAtan(delta.y,
                sqrtf((delta.z * delta.z) + (delta.x * delta.x)));
            rot.y = work->angle;
            rot.z = 0.0f;
            mbObjRotSetV(work->modelId[modelNo], &rot);
            modelNo++;
        } else if (i == 6) {
            PSVECSubtract(&work->endPoint[1], &prevPos, &delta);
            PSVECNormalize(&delta, &delta);
            pos.x = work->endPoint[1].x - (100.0f * (0.5f * delta.x));
            pos.y = work->endPoint[1].y - (100.0f * (0.5f * delta.y));
            pos.z = work->endPoint[1].z - (100.0f * (0.5f * delta.z));
            mbObjPosSetV(work->modelId[modelNo], &pos);
            rot.x = (float)((atan2(delta.y,
                sqrtf((delta.z * delta.z) + (delta.x * delta.x))) / M_PI)
                * 180.0);
            rot.y = work->angle;
            rot.z = 0.0f;
            mbObjRotSetV(work->modelId[modelNo], &rot);
            modelNo++;
        } else {
            mbObjPosGet(marker->modelId, &pos);
            pos.y -= 30.000002f;
            PSVECSubtract(&pos, &prevPos, &delta);
            prevPos = pos;
            pos.x -= 0.5f * delta.x;
            pos.y -= 0.5f * delta.y;
            pos.z -= 0.5f * delta.z;
            mbObjPosSetV(work->modelId[modelNo], &pos);
            rot.x = (float)((atan2(delta.y,
                sqrtf((delta.z * delta.z) + (delta.x * delta.x))) / M_PI)
                * 180.0);
            rot.y = work->angle;
            rot.z = 0.0f;
            mbObjRotSetV(work->modelId[modelNo], &rot);
            modelNo++;
        }
    }
}

#undef FN13CC_HEIGHT

void fn_1_2A08(void)
{
    lbl_1_bss_155C.initF = TRUE;
}

static const float lbl_1_rodata_F8[14] = {
    -80.0f, -90.0f, -125.0f, -80.0f, -100.0f, -125.0f, -110.0f,
    -80.0f, -90.0f, -80.0f, -80.0f, -80.0f, -80.0f, -80.0f
};

// MB1_Create builds the spring route and initializes its four hanging segments.
void fn_1_2A1C(void)
{
    W01_SPRING_WORK *work;
    W01_SPRING_SEGMENT_WORK *segment;
    HuVecF pos;
    s32 i;
    s32 dataDir1;
    s32 dataDir2;
    s32 dataDir3;
    BOOL dayF1;
    BOOL dayF2;
    BOOL dayF3;

    work = &lbl_1_bss_1494;
    segment = work->segment;
    work->startPos.x = 747.0f;
    work->startPos.y = 4993.0f;
    work->startPos.z = -1309.0f;
    work->startMasuId = mbMasuFind_MAttrIdGet(-1, W01_MASU_ATTR_MOVE_HOOK_DASH);
    work->endPos.x = 2708.0f;
    work->endPos.y = 4037.0f;
    work->endPos.z = 18.0f;
    work->endMasuId = mbMasuFind_MAttrIdGet(-1, (1 << 9));
    PSVECSubtract(&work->endPos, &work->startPos, &work->dir);
    work->length = PSVECMag(&work->dir);
    work->angle = HuAtan(-work->dir.x, -work->dir.z);
    work->state = -1;
    for (i = 0; i < 16; i++) {
        dayF1 = !GwSystem.curTime;
        if (dayF1 != FALSE) {
            dataDir1 = W01_DATA_DAY;
        } else {
            dataDir1 = W01_DATA_NIGHT;
        }
        work->pathModelId[i] = mbObjCreate(
            dataDir1 | W01_FILE_SPRING_MODEL, NULL, TRUE);
        mbObjRotYSet(work->pathModelId[i], work->angle);
    }
    dayF2 = !GwSystem.curTime;
    if (dayF2 != FALSE) {
        dataDir2 = W01_DATA_DAY;
    } else {
        dataDir2 = W01_DATA_NIGHT;
    }
    work->mainModelId = mbObjCreate(
        dataDir2 | W01_FILE_SPRING_MAIN_MODEL, NULL, TRUE);
    mbObjRotYSet(work->mainModelId, work->angle);
    fn_1_3214(0.05f);
    mbObjPosGet(work->mainModelId, &pos);
    for (i = 0; i < 4; i++, segment++) {
        dayF3 = !GwSystem.curTime;
        if (dayF3 != FALSE) {
            dataDir3 = W01_DATA_DAY;
        } else {
            dataDir3 = W01_DATA_NIGHT;
        }
        segment->modelId = mbObjCreate(
            dataDir3 | W01_FILE_SPRING_MODEL, NULL, TRUE);
        mbObjScaleSet(segment->modelId, 0.7f, 0.7f, 0.5f);
        segment->pos.x = pos.x;
        segment->pos.y = pos.y - (50.0f * (i + 1));
        segment->pos.z = pos.z;
        segment->vel.x = segment->vel.y = segment->vel.z = 0.0f;
    }
}

// The move hook carries a player along the spring route, then jumps them to its exit space.
void fn_1_2D08(int playerNo)
{
    W01_SPRING_WORK *work;
    W01_SPRING_SEGMENT_WORK *segment;
    HuVecF pos;
    HuVecF playerPos;
    HuVecF targetPos;
    HuVecF delta;
    s16 motionId;
    u32 time;
    float angle;
    float weight;

    work = &lbl_1_bss_1494;
    segment = work->segment;
    GwPlayer[playerNo].moveF = TRUE;
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbPlayerWorkGet(playerNo)->_unk08 = 100;
    mbObjPosGet(work->segment[3].modelId, &targetPos);
    targetPos.y += lbl_1_rodata_F8[GwPlayer[playerNo].charNo];
    motionId = mbPlayerMotionCreate(playerNo, W01_PLAYER_MOTION_SPRING_LAUNCH);
    mbPlayerPosGet(playerNo, &playerPos);
    PSVECSubtract(&targetPos, &playerPos, &delta);
    angle = HuAtan(work->dir.x, work->dir.z);
    mbPlayerRotYSet(playerNo, angle);
    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, FALSE);
    for (time = 0; time < 18; time++) {
        weight = (float)(s32)time / 18.0f;
        if (time == 15) {
            mbPlayerMotionShiftSet(playerNo, motionId, 0.0f, 4.0f, FALSE);
        }
        pos.x = playerPos.x + (weight * delta.x);
        pos.y = playerPos.y + (weight * delta.y)
            + (100.0f * mbSinDeg(180.0f * weight));
        pos.z = playerPos.z + (weight * delta.z);
        mbPlayerPosSetV(playerNo, &pos);
        HuPrcVSleep();
    }
    work->state = playerNo;
    mbAudFXPlay(0x594);
    for (time = 3; time < 54; time++) {
        weight = (float)(s32)time / 59.0f;
        if (time == 12) {
            CharFXPlay(GwPlayer[playerNo].charNo, 0x243);
        }
        fn_1_3214(weight);
        if ((((s32)time - 3) % 30) == 0) {
            omVibrate(playerNo, 20, 4, 4);
        }
        HuPrcVSleep();
    }
    work->state = -1;
    mbPlayerPosGet(playerNo, &playerPos);
    mbMasuPosGet(work->endMasuId, &targetPos);
    PSVECSubtract(&targetPos, &playerPos, &delta);
    angle = HuAtan(delta.x, delta.z);
    mbPlayerRotYSet(playerNo, angle);
    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 4.0f, FALSE);
    for (time = 0; time <= 18; time++) {
        weight = (float)(s32)time / 18.0f;
        if (time == 12) {
            mbPlayerMotionShiftSet(playerNo, 5, 0.0f, 8.0f, FALSE);
        }
        pos.x = playerPos.x + (weight * delta.x);
        pos.y = playerPos.y + (weight * delta.y)
            + (100.0f * (2.0f * mbSinDeg(180.0f * weight)));
        pos.z = playerPos.z + (weight * delta.z);
        mbPlayerPosSetV(playerNo, &pos);
        mbPlayerWorkGet(playerNo)->_unk08 = 18 - time;
        HuPrcVSleep();
    }
    mbPlayerMotionKill(playerNo, motionId);
    GwPlayer[playerNo].moveF = FALSE;
    mbPlayerColSnapPlayerSet(playerNo, TRUE);
}

#define FN3214_HEIGHT \
    ( \
        ( \
            ((valueB * valueB * valueB * valueB) \
                + ((valueA * valueA * valueA * valueA) \
                    + (valueC * valueC * valueC * valueC))) \
            - ( \
                2.0f * ( \
                    (2.0f * valueC * valueC * valueB * valueB) \
                    + ( \
                        ((valueA * valueA * valueB * valueB) \
                            + (valueA * valueA * valueC * valueC)) \
                        - (valueC * valueC * valueB * valueB) \
                    ) \
                ) \
            ) \
        ) \
    ) \
    / (4.0f * valueC * valueC)

// Spring setup, reset, and travel use this to bend the route and position its hanging attachment.
void fn_1_3214(float value)
{
    W01_SPRING_WORK *work;
    HuVecF modelPos;
    HuVecF controlPos;
    HuVecF prevPos;
    HuVecF curvePos;
    HuVecF targetPos;
    HuVecF delta;
    HuVecF rot;
    s32 i;
    float valueB;
    float valueA;
    float valueC;
    float finalLength;
    float scale;
    float valueATemp;
    float t;
    float temp;
    float distance;

    work = &lbl_1_bss_1494;
    valueATemp = value * work->length;
    valueA = valueATemp;
    valueB = work->length - valueA;
    valueC = 1.1f * work->length;
    temp = sqrtf(FN3214_HEIGHT);
    controlPos.x = work->startPos.x + (value * work->dir.x);
    controlPos.y = (work->startPos.y + (value * work->dir.y)) - temp;
    controlPos.z = work->startPos.z + (value * work->dir.z);
    targetPos = controlPos;
    finalLength = w01CurveLen((W01CurveEval)(u32)fn_1_14A90, &work->startPos,
        &controlPos, &work->endPos);
    t = 0.0f;
    distance = finalLength / 16.0f;
    prevPos = work->startPos;
    for (i = 0; i < 16; i++) {
        if (i < 15) {
            t = w01CurveT((W01CurveEval)(u32)fn_1_14A90, &work->startPos,
                &controlPos, &work->endPos, t, distance);
            mbBezierCalcV(&work->startPos, &controlPos, &work->endPos,
                &curvePos, t);
        } else {
            curvePos = work->endPos;
        }
        PSVECSubtract(&curvePos, &prevPos, &delta);
        modelPos.x = prevPos.x + (0.5f * delta.x);
        modelPos.y = prevPos.y + (0.5f * delta.y);
        modelPos.z = prevPos.z + (0.5f * delta.z);
        mbObjPosSetV(work->pathModelId[i], &modelPos);
        mbObjRotGet(work->pathModelId[i], &rot);
        rot.x = (float)((atan2(delta.y,
            sqrtf((delta.z * delta.z) + (delta.x * delta.x))) / M_PI)
            * 180.0);
        mbObjRotSetV(work->pathModelId[i], &rot);
        scale = PSVECMag(&delta) / 100.0f;
        mbObjScaleSet(work->pathModelId[i], 1.0f, 1.0f, scale);
        {
            PSVECSubtract(&curvePos, &prevPos, &delta);
            temp = (((targetPos.z * delta.z)
                + ((targetPos.x * delta.x) - (prevPos.x * delta.x)))
                - (prevPos.z * delta.z))
                / ((delta.x * delta.x) + (delta.z * delta.z));
            if (temp >= 0.0f && temp < 1.0f) {
                targetPos.y = prevPos.y + (temp * delta.y);
                mbObjPosSetV(work->mainModelId, &targetPos);
                mbObjRotSetV(work->mainModelId, &rot);
            }
        }
        prevPos = curvePos;
        distance += finalLength / 16.0f;
    }
}

#undef FN3214_HEIGHT

// The board update simulates the hanging spring segments and carries any player attached to them.
void fn_1_4004(void)
{
    W01_SPRING_WORK *work;
    W01_SPRING_SEGMENT_WORK *segment;
    HuVecF modelPos;
    HuVecF prevPos;
    HuVecF delta;
    HuVecF rot;
    s32 i;
    float distance;
    float force;

    work = &lbl_1_bss_1494;
    segment = work->segment;
    mbObjPosGet(work->mainModelId, &prevPos);
    for (i = 0; i < 4; i++, segment++) {
        PSVECSubtract(&prevPos, &segment->pos, &delta);
        distance = PSVECMag(&delta);
        force = 20.0f * (distance - 50.0f);
        PSVECNormalize(&delta, &delta);
        segment->vel.x += (1.0f / 60.0f) * (delta.x * force);
        segment->vel.y += -32.666668f
            + ((1.0f / 60.0f) * (delta.y * force));
        segment->vel.z += (1.0f / 60.0f) * (delta.z * force);
        segment->pos.x += (1.0f / 60.0f) * segment->vel.x;
        segment->pos.y += (1.0f / 60.0f) * segment->vel.y;
        segment->pos.z += (1.0f / 60.0f) * segment->vel.z;
        PSVECScale(&segment->vel, &segment->vel, 0.98f);

        PSVECSubtract(&segment->pos, &prevPos, &delta);
        PSVECNormalize(&delta, &delta);
        segment->pos.x = prevPos.x + (50.0f * delta.x);
        segment->pos.y = prevPos.y + (50.0f * delta.y);
        segment->pos.z = prevPos.z + (50.0f * delta.z);

        PSVECSubtract(&prevPos, &segment->pos, &delta);
        modelPos.x = segment->pos.x + (0.5f * delta.x);
        modelPos.y = segment->pos.y + (0.5f * delta.y);
        modelPos.z = segment->pos.z + (0.5f * delta.z);
        mbObjPosSetV(segment->modelId, &modelPos);

        rot.x = HuAtan(delta.y,
            sqrtf((delta.z * delta.z) + (delta.x * delta.x)));
        rot.y = HuAtan(-delta.x, -delta.z);
        rot.z = 0.0f;
        mbObjRotSetV(segment->modelId, &rot);
        prevPos = segment->pos;
    }
    if (work->state >= 0) {
        mbObjPosGet(work->segment[3].modelId, &modelPos);
        mbPlayerPosSet(work->state, modelPos.x,
            modelPos.y + lbl_1_rodata_F8[GwPlayer[work->state].charNo],
            modelPos.z);
    }
}

// The turn-close hook restores the spring route and stops its hanging segments.
void fn_1_44C0(void)
{
    W01_SPRING_WORK *work;
    W01_SPRING_SEGMENT_WORK *segment;
    HuVecF pos;
    s32 i;

    work = &lbl_1_bss_1494;
    segment = work->segment;
    fn_1_3214(0.05f);
    mbObjPosGet(work->mainModelId, &pos);
    for (i = 0; i < 4; i++, segment++) {
        segment->pos.x = pos.x;
        segment->pos.y = pos.y - (50.0f * (i + 1));
        segment->pos.z = pos.z;
        segment->vel.x = segment->vel.y = segment->vel.z = 0.0f;
    }
}

// MB1_Create calls this once to create the floating spaces and their visual effects.
void fn_1_45A0(void)
{
    W01_FLOAT_WORK *work;
    u32 attr;
    s32 i;

    lbl_1_bss_1490 = 7;
    work = mbMalloc(lbl_1_bss_1490 * sizeof(W01_FLOAT_WORK));
    lbl_1_bss_148C = work;
    for (i = 0; i < lbl_1_bss_1490; i++, work++) {
        work->modelId = mbObjCreate(
            (MBTimeDayGet() ? W01_DATA_DIR_DAY : W01_DATA_DIR_NIGHT)
                | W01_FILE_FLOATING_MASU,
            NULL, TRUE);
        mbObjAttrSet(work->modelId, HU3D_MOTATTR_LOOP);
        work->pos.x = lbl_1_data_48C[i].x;
        work->pos.y = 1000.0f + lbl_1_data_48C[i].y;
        work->pos.z = lbl_1_data_48C[i].z;
        mbObjPosSet(work->modelId, work->pos.x, work->pos.y - 1000.0f,
            work->pos.z);
        attr = mbev_MasuAttrGet(i + 1, W01_MASU_ATTR_EVENT_GROUP_MASK);
        work->masuId = mbMasuFind_MAttrMatchIdGet(-1, attr, W01_MASU_ATTR_EVENT_GROUP_MASK);
        work->maxTime = 240;
        work->time = mbRandMod(work->maxTime);
        work->speed = 1.0f;
    }
    lbl_1_bss_1488 = -1;
}

// MB1_Kill releases the floating-space work array when the board closes.
void fn_1_4750(void)
{
    if (lbl_1_bss_148C != NULL) {
        void *data = lbl_1_bss_148C;

        HuMemDirectFree(data);
        lbl_1_bss_148C = NULL;
    }
}

// The board update bobs the floating spaces and depresses the one occupied by the active player.
void fn_1_47B0(void)
{
    W01_FLOAT_WORK *work = lbl_1_bss_148C;
    HuVecF pos;
    HuVecF playerPos;
    HuVecF delta;
    s32 playerNo = GwSystem.turnPlayerNo;
    s32 i;
    float force;
    float time;
    float speed;
    float verticalDelta;
    float motionSpeed;

    if (playerNo >= 0) {
        mbPlayerPosGet(playerNo, &playerPos);
    }
    for (i = 0; i < lbl_1_bss_1490; i++, work++) {
        time = (float)work->time++ / (float)work->maxTime;
        speed = 5.0f + (5.0f * (1.0f + mbSinDeg(360.0f * time)));
        mbObjPosGet(work->modelId, &pos);
        force = -980.0f;
        if (playerNo >= 0) {
            PSVECSubtract(&pos, &playerPos, &delta);
            if (work->active != FALSE) {
                speed += 50.0f;
                if (lbl_1_bss_1488 != work->masuId) {
                    work->active = FALSE;
                }
            } else if (sqrtf((delta.x * delta.x) + (delta.z * delta.z))
                    < 170.0f
                && (delta.y > 0.0f
                    || mbPlayerWorkGet(playerNo)->moveEndF == TRUE)
                && work->masuId == lbl_1_bss_1488) {
                work->active = TRUE;
                force -= 50000.0f;
            } else {
                work->active = FALSE;
            }
        }
        verticalDelta = work->pos.y - pos.y;
        force += (500.0f * (verticalDelta - 1000.0f)) / speed;
        work->velocity.y += (1.0f / 60.0f) * force;
        pos.y += (1.0f / 60.0f) * work->velocity.y;
        work->velocity.y *= 0.9f;
        mbObjPosSetV(work->modelId, &pos);
        motionSpeed = fabs(force) / 1000.0;
        if (motionSpeed < 1.0f) {
            motionSpeed = 1.0f;
        } else if (motionSpeed > 3.0f) {
            motionSpeed = 3.0f;
        }
        work->speed += 0.1f * (motionSpeed - work->speed);
        mbObjMotionSpeedSet(work->modelId, work->speed);
        mbMasuPosSet(work->masuId, pos.x, pos.y, pos.z);
    }
}

// The turn-start hook settles the floating space beneath this player before movement begins.
void fn_1_4C6C(s32 playerNo)
{
    s32 i;

    lbl_1_bss_1488 = GwPlayer[playerNo].masuId;
    // Settle immediately: these updates run without waiting for another frame.
    for (i = 0; i < 100; i++) {
        fn_1_47B0();
    }
}

// The turn-close hook restores floating-space positions and clears their occupied states.
void fn_1_4CD0(void)
{
    W01_FLOAT_WORK *work = lbl_1_bss_148C;
    s32 i;

    for (i = 0; i < lbl_1_bss_1490; i++, work++) {
        work->active = FALSE;
        mbObjPosSet(work->modelId, work->pos.x, work->pos.y - 1000.0f,
            work->pos.z);
        work->speed = 1.0f;
    }
    lbl_1_bss_1488 = -1;
}

void fn_1_4D78(s16 masuId)
{
    lbl_1_bss_1488 = masuId;
}

// MB1_Create calls this to create moving path models for the board's path event.
void fn_1_4D88(void)
{
    W01_MOTION_WORK *work;
    BOOL dayF1;
    BOOL dayF2;
    s32 i;
    s32 j;

    work = lbl_1_bss_1438;
    lbl_1_bss_1436 = mbMasuFind_MAttrIdGet(-1, W01_MASU_ATTR_MOVE_HOOK_PATH);
    mbMasuPosGet(lbl_1_bss_1436, &lbl_1_bss_1428);
    lbl_1_bss_1434 = mbMasuFind_MAttrIdGet(-1, W01_MASU_ATTR_LINK);
    mbMasuPosGet(lbl_1_bss_1434, &lbl_1_bss_141C);
    PSVECSubtract(&lbl_1_bss_141C, &lbl_1_bss_1428, &lbl_1_bss_1410);
    for (i = 0; i < 1; i++, work++) {
        work->armLength[0] = 550.0f;
        work->armLength[1] = 150.0f;
        for (j = 0; j < 2; j++) {
            dayF1 = !GwSystem.curTime;
            work->modelId[j] = mbObjCreate(
                (dayF1 ? W01_DATA_DIR_DAY : W01_DATA_DIR_NIGHT)
                    | W01_FILE_MOTION_VARIANT_A,
                NULL, TRUE);
        }
        dayF2 = !GwSystem.curTime;
        work->modelId[2] = mbObjCreate(
            (dayF2 ? W01_DATA_DIR_DAY : W01_DATA_DIR_NIGHT)
                | W01_FILE_MOTION_VARIANT_B,
            NULL, TRUE);
        work->firstMass = 1.0f;
        work->secondMass = 20.0f;
        work->swingState[0] = work->swingState[1] = 0.0f;
        work->swingState[2] = work->swingState[3] = 0.0f;
        work->anchorPos = lbl_1_data_4E0[i];
        work->activePlayerNo = -1;
        fn_1_5888(i);
    }
}

// The path-space move hook calls this while the player rides the swinging path.
void fn_1_4F94(int playerNo)
{
    W01_MOTION_WORK *work;
    HuVecF pos;
    HuVecF playerPos;
    HuVecF targetPos;
    HuVecF delta;
    HuVecF delta2;
    HuVecF pos2;
    HuVecF playerPos2;
    s16 motionId;
    s32 remainingMoveFrames;
    u32 time;
    s32 time2;
    float angle;
    float rotY;
    float weight;
    float weight2;
    float rotWeight;
    void *hook;

    work = lbl_1_bss_1438;
    GwPlayer[playerNo].moveF = TRUE;
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbPlayerWorkGet(playerNo)->_unk08 = 100;
    GwPlayer[playerNo].masuIdNext = lbl_1_bss_1434;
        motionId = mbPlayerMotionCreate(playerNo, W01_PLAYER_MOTION_SPRING_LAUNCH);
    // Query the character's item hook here; the returned pointer is unused.
    hook = CharModelItemHookGet(GwPlayer[playerNo].charNo, 4, FALSE);
    mbPlayerPosGet(playerNo, &playerPos);
    // This preliminary subtraction reads targetPos before it is set; the boarding loop replaces
    // delta.
    PSVECSubtract(&targetPos, &playerPos, &delta);
    angle = (float)((atan2(lbl_1_bss_1410.x, lbl_1_bss_1410.z) / M_PI)
        * 180.0);
    mbPlayerRotYSet(playerNo, angle);
    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, FALSE);
    for (time = 0; time < 18; time++) {
        weight = (float)(s32)time / 18.0f;
        if (time == 12) {
            mbPlayerMotionShiftSet(playerNo, motionId, 0.0f, 8.0f, FALSE);
        }
        targetPos = work->endpointPos[0];
        targetPos.y -= 100.0f;
        PSVECSubtract(&targetPos, &playerPos, &delta);
        pos.x = playerPos.x + (weight * delta.x);
        pos.y = playerPos.y + (weight * delta.y)
            + (100.0f * mbSinDeg(180.0f * weight));
        pos.z = playerPos.z + (weight * delta.z);
        mbPlayerPosSetV(playerNo, &pos);
        HuPrcVSleep();
    }
    work->firstMass = 20.0f;
    work->swingState[2] = 2.0f;
    work->activePlayerNo = playerNo;
    mbAudFXPlay(0x594);
    for (time = 0; time < 120; time++) {
        HuPrcVSleep();
    }
    work->firstMass = 1.0f;
    work->activePlayerNo = -1;
    mbAudFXPlay(0x594);
    mbMasuPosGet(lbl_1_bss_1434, &targetPos);
    // Face the last boarding delta here; the destination delta below only drives the landing
    // position.
    angle = (float)((atan2(delta.x, delta.z) / M_PI) * 180.0);
    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, FALSE);
    mbPlayerPosGet(playerNo, &playerPos2);
    rotY = mbPlayerRotYGet(playerNo);
    PSVECSubtract(&targetPos, &playerPos2, &delta2);
    for (time2 = 0; time2 <= 18; time2++) {
        weight2 = (float)(s32)time2 / 18.0f;
        if ((u32)time2 == 12) {
            mbPlayerMotionShiftSet(playerNo, 5, 0.0f, 2.0f, FALSE);
        }
        pos2.x = playerPos2.x + (weight2 * delta2.x);
        pos2.y = playerPos2.y + (weight2 * delta2.y)
            + (200.0 * sin((M_PI * (180.0f * weight2)) / 180.0));
        pos2.z = playerPos2.z + (weight2 * delta2.z);
        mbPlayerPosSetV(playerNo, &pos2);
        rotWeight = (float)(s32)time2 / 6.0f;
        if (rotWeight > 1.0f) {
            rotWeight = 1.0f;
        }
        mbPlayerRotYSet(playerNo, mbAngleLerp(rotY, angle, rotWeight));
        remainingMoveFrames = 18 - time2;
        mbPlayerWorkGet(playerNo)->_unk08 = remainingMoveFrames;
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 2.0f, HU3D_MOTATTR_LOOP);
    mbPlayerMotionKill(playerNo, motionId);
    GwPlayer[playerNo].moveF = FALSE;
    mbPlayerColSnapPlayerSet(playerNo, TRUE);
}

// The board update object calls this each frame to advance the linked-path swing.
void fn_1_5560(s32 value)
{
    W01_MOTION_WORK *work;
    float angle;
    float massRatio;
    float lengthRatio;
    float gravity;
    float denominator;
    float acceleration0;
    float acceleration1;

    work = &lbl_1_bss_1438[value];
    angle = work->swingState[0] - work->swingState[1];
    massRatio = work->secondMass / (work->firstMass + work->secondMass);
    lengthRatio = work->armLength[1] / work->armLength[0];
    gravity = -1960.0f / work->armLength[0];
    denominator = lengthRatio - (massRatio * lengthRatio * mbCosRad(angle)
        * mbCosRad(angle));
    if (denominator != 0.0f) {
        acceleration0 = (gravity * gravity * lengthRatio
            * (-mbSinRad(work->swingState[0])
                + (massRatio * mbCosRad(angle) * mbSinRad(work->swingState[1])))
            - (massRatio * lengthRatio
                * ((lengthRatio * work->swingState[3] * work->swingState[3])
                    + (work->swingState[2] * work->swingState[2]
                        * mbCosRad(angle)))
                * mbSinRad(angle))) / denominator;
        acceleration1 = ((gravity * gravity * mbCosRad(angle)
            * mbSinRad(work->swingState[0]))
            - (gravity * gravity * mbSinRad(work->swingState[1]))
            + (((work->swingState[2] * work->swingState[2])
                + (massRatio * lengthRatio
                    * work->swingState[3] * work->swingState[3]
                    * mbCosRad(angle))) * mbSinRad(angle)))
            / denominator;
    } else {
        acceleration0 = acceleration1 = 0.0f;
    }
    work->swingState[2] += 0.016666668f * acceleration0;
    work->swingState[3] += 0.016666668f * acceleration1;
    work->swingState[0] += 0.016666668f * work->swingState[2];
    work->swingState[1] += 0.016666668f * work->swingState[3];
    if (work->activePlayerNo < 0) {
        work->swingState[2] *= 0.99f;
        work->swingState[3] *= 0.99f;
    }
    fn_1_5888(value);
}

// Setup and swing updates call this to place the two path segments and rider.
void fn_1_5888(s32 index)
{
    Mtx matrix;
    Mtx scale;
    Mtx rot;
    HuVecF pos;
    HuVecF previous;
    HuVecF delta;
    HuVecF normal;
    HuVecF points[2];
    Quaternion quat2;
    Quaternion quat1;
    Quaternion quat3;
    W01_MOTION_WORK *work;
    float angle;
    s32 i;

    work = &lbl_1_bss_1438[index];
    points[0].x = work->armLength[0] * mbSinRad(work->swingState[0]);
    points[0].y = -work->armLength[0] * mbCosRad(work->swingState[0]);
    points[0].z = 0.0f;
    points[1].x = points[0].x
        + (work->armLength[1] * mbSinRad(work->swingState[1]));
    points[1].y = points[0].y
        - (work->armLength[1] * mbCosRad(work->swingState[1]));
    points[1].z = 0.0f;
    angle = -((atan2(lbl_1_bss_1410.z, lbl_1_bss_1410.x) / M_PI) * 180.0);
    mbMtxRotAxisDeg(matrix, 'y', angle);
    mtxTransCat(matrix, work->anchorPos.x, work->anchorPos.y,
        work->anchorPos.z);
    quat2.x = sin(M_PI / 4.0);
    quat2.y = quat2.z = 0.0f;
    quat2.w = cos(M_PI / 4.0);
    normal.x = -lbl_1_bss_1410.z;
    normal.y = 0.0f;
    normal.z = lbl_1_bss_1410.x;
    PSVECNormalize(&normal, &normal);
    previous = work->anchorPos;
    for (i = 0; i < 2; i++) {
        PSMTXMultVec(matrix, &points[i], &pos);
        PSVECSubtract(&pos, &previous, &delta);
        work->endpointPos[i] = pos;
        mbObjPosSet(work->modelId[i], pos.x - (0.5f * delta.x),
            pos.y - (0.5f * delta.y), pos.z - (0.5f * delta.z));
        C_QUATRotAxisRad(&quat1, &normal, work->swingState[i]);
        PSQUATMultiply(&quat1, &quat2, &quat3);
        PSMTXQuat(rot, &quat3);
        PSMTXScale(scale, 1.0f, 1.0f, work->armLength[i] / 100.0f);
        PSMTXConcat(rot, scale, rot);
        mbObjMtxSet(work->modelId[i], &rot);
        if (work->activePlayerNo >= 0 && i == 0) {
            mbPlayerPosSet(work->activePlayerNo, pos.x, pos.y - 100.0f, pos.z);
        }
        previous = pos;
    }
    mbObjPosSetV(work->modelId[2], &pos);
}

// The end-of-turn callback resets the swinging path before the next player moves.
void fn_1_5C18(void)
{
    W01_MOTION_WORK *work = lbl_1_bss_1438;

    work->firstMass = 1.0f;
    work->secondMass = 20.0f;
    work->swingState[0] = work->swingState[1] = 0.0f;
    work->swingState[2] = work->swingState[3] = 0.0f;
}

// MB1_Create calls this to create the day/night coin field and load its animations.
void fn_1_5C7C(void)
{
    W01_WORK_13D8 *work;
    BOOL dayF;
    s32 time;
    BOOL dayF2;
    s32 dataDir;
    BOOL dayF3;
    s32 count;
    W01_COIN_ENTRY *data;
    s32 i;

    work = &lbl_1_bss_13D8;
    dayF = !GwSystem.curTime;
    if (dayF != FALSE) {
        time = 0;
    } else {
        time = 1;
    }
    dayF2 = !GwSystem.curTime;
    if (dayF2 != FALSE) {
        dataDir = W01_DATA_DIR_DAY;
    } else {
        dataDir = W01_DATA_DIR_NIGHT;
    }
    work->modelId = mbObjCreate(dataDir | W01_FILE_COIN_FIELD_MAIN, lbl_1_data_4FC[time], FALSE);
    mbObjMotionSet(work->modelId, 0, HU3D_MOTATTR_LOOP);
    mbObjPosSet(work->modelId, 0.0f, 4300.0f, -1350.0f);

    if (!GwSystem.curTime) {
        Hu3DModelObjPosGet(mbObjModelIDGet(work->modelId), lbl_1_data_504,
            &work->pos);
    } else {
        Hu3DModelObjPosGet(mbObjModelIDGet(work->modelId), lbl_1_data_50D,
            &work->pos);
    }

    dayF3 = !GwSystem.curTime;
    if (dayF3 != FALSE) {
        count = 50;
    } else {
        count = 30;
    }
    work->count = count;
    data = mbMalloc(work->count * sizeof(W01_COIN_ENTRY));
    work->data = data;
    for (i = 0; i < work->count; i++, data++) {
        if (!GwSystem.curTime) {
            data->modelId = mbCoinCreate2();
            mbCoinObjDispSet(data->modelId, FALSE);
        } else {
            data->modelId = mbObjCreate(W01_MODEL_COIN_NIGHT, NULL, TRUE);
            mbObjDispSet(data->modelId, FALSE);
        }
    }

    lbl_1_bss_13D0[0] = HuSprAnimRead(HuDataSelHeapReadNum(
        mbBoardDataNumGet(W01_DATA_STAR_ANIM_DAY), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprAnimLock(lbl_1_bss_13D0[0]);
    lbl_1_bss_13D0[1] = HuSprAnimRead(HuDataSelHeapReadNum(
        mbBoardDataNumGet(W01_DATA_STAR_ANIM_NIGHT), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprAnimLock(lbl_1_bss_13D0[1]);
}

// MB1_Kill releases the coin-event entries and its day/night particle animations.
void fn_1_5EF0(void)
{
    s32 i;

    if (lbl_1_bss_13D8.data) {
        void *data = lbl_1_bss_13D8.data;

        HuMemDirectFree(data);
        lbl_1_bss_13D8.data = NULL;
    }
    for (i = 0; i < 2; i++) {
        if (lbl_1_bss_13D0[i]) {
            HuSprAnimKill(lbl_1_bss_13D0[i]);
            lbl_1_bss_13D0[i] = NULL;
        }
    }
}

// The question-mark-space callback runs the falling-coin event for all players.
// Day coins increase their totals; night objects hit players and remove coins.
void fn_1_5FB4(s32 playerNo, s16 id)
{
    s32 masuCount;
    s32 playerCount;
    s32 dataCount;
    s32 playerF;
    s16 motionId;
    s16 linkMasuId;
    s32 randNo1;
    s32 randNo2;
    s16 playerNoTbl[4];
    HuVecF masuPos;
    HuVecF linkPos;
    HuVecF playerPos;
    s32 coinNum[4];
    int coinAdd[4];
    HuVecF delta;
    HuVecF pos;
    HuVecF startPos;
    s16 masuIdTbl[256];
    W01_COIN_ENTRY *data;
    s32 *dataOrder;
    s32 i;
    s32 j;
    W01_WORK_13D8 *work;
    s32 k;
    s32 curPlayerNo;
    s16 winId;
    float angle;
    float radius;
    float rotY;
    float weight;
    float weight2;

    work = &lbl_1_bss_13D8;
    dataOrder = mbMalloc(work->count * sizeof(s32));
    if (!GwSystem.curTime) {
        for (i = 0; i < 4; i++) {
            coinNum[i] = lbl_1_data_530[GwPlayer[i].rank] + mbRandMod(5) - 2;
            if (coinNum[i] < 0) {
                coinNum[i] = 0;
            }
        }
    } else {
        for (i = 0; i < 4; i++) {
            coinNum[i] = lbl_1_data_540[GwPlayer[i].rank] + mbRandMod(5) - 2;
            if (coinNum[i] < 0) {
                coinNum[i] = 0;
            }
        }
    }
    if (!GWTeamFGet()) {
        for (i = 0; i < 4; i++) {
            work->coinCount[i] = mbPlayerCoinGet(i);
        }
    } else {
        for (i = 0; i < 2; i++) {
            work->coinCount[i] = mbPlayerTeamCoinGet(i);
        }
    }
    data = work->data;
    for (i = 0; i < work->count; i++, data++) {
        data->active = TRUE;
        angle = 360.0f * frandf();
        radius = 1000.0f + (100.0f * (10.0f * frandf()));
        data->pos.x = work->pos.x;
        data->pos.y = work->pos.y + 650.0f;
        data->pos.z = work->pos.z;
        data->vel.x = radius * mbCosDeg(angle);
        data->vel.y = 2000.0f;
        data->vel.z = radius * mbSinDeg(angle);
        data->delay = mbRandMod(30) + 1;
        if (!GwSystem.curTime) {
            data->rot.y = 360.0f * frandf();
            mbCoinObjPosSetV(data->modelId, &data->pos);
            mbCoinObjDispSet(data->modelId, FALSE);
        } else {
            data->rot.x = 360.0f * frandf();
            data->rot.y = 360.0f * frandf();
            mbObjPosSetV(data->modelId, &data->pos);
            mbObjDispSet(data->modelId, FALSE);
        }
        data->state = 0;
    }
    motionId = mbPlayerMotionCreate(playerNo, W01_PLAYER_MOTION_STAR_COLLECT);
    for (i = 0; i < 4; i++) {
        work->playerMotion[i] = mbPlayerMotionCreate(i, W01_PLAYER_MOTION_STAR_REACTION);
    }
    mbPlayerPosGet(playerNo, &playerPos);
    linkMasuId = mbMasuAttrFindLink(id, W01_MASU_ATTR_LINK);
    mbMasuPosGet(linkMasuId, &linkPos);
    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, 0);
    mbPlayerPosGet(playerNo, &startPos);
    rotY = mbPlayerRotYGet(playerNo);
    PSVECSubtract(&linkPos, &startPos, &delta);
    {
        s32 moveI;
        s32 time;

        for (moveI = 0; moveI <= 18; moveI++) {
            weight = (float)moveI / 18.0f;
            if ((u32)moveI == 12) {
                mbPlayerMotionShiftSet(playerNo, 5, 0.0f, 2.0f, 0);
            }
            pos.x = startPos.x + (weight * delta.x);
            pos.y = startPos.y + (weight * delta.y)
                + (100.0 * sin((M_PI * (180.0f * weight)) / 180.0));
            pos.z = startPos.z + (weight * delta.z);
            mbPlayerPosSetV(playerNo, &pos);
            weight2 = (float)moveI / 6.0f;
            if (weight2 > 1.0f) {
                weight2 = 1.0f;
            }
            mbPlayerRotYSet(playerNo, mbAngleLerp(rotY, 180.0f, weight2));
            time = 18 - moveI;
            mbPlayerWorkGet(playerNo)->_unk08 = time;
            HuPrcVSleep();
        }
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 2.0f, HU3D_MOTATTR_LOOP);
    mbCameraMoveMasu(linkMasuId, &lbl_1_data_518, &lbl_1_data_524, 2875.0f,
        -1.0f, 30);
    mbCameraMoveWait();
    if (!GwSystem.curTime) {
        winId = mbWinCreate(2, W01_MESSAGE_STAR_DAY, 17);
    } else {
        winId = mbWinCreate(2, W01_MESSAGE_STAR_NIGHT, 18);
    }
    mbWinWait(winId);
    HuPrcSleep(30);
    mbObjMotionShiftSet(work->modelId, 1, 0.0f, 4.0f, 0);
    HuPrcSleep(30);
    mbPlayerMotionShiftSet(playerNo, motionId, 0.0f, 2.0f, 0);
    mbCameraShakeSet(30, 200.0f);
    mbAudFXPlay(0x596);
    for (i = 0; i < 4; i++) {
        omVibrate(i, 20, 20, 0);
    }
    for (i = 0; (u32)i < 60; i++) {
        fn_1_6D98();
        HuPrcVSleep();
    }
    mbWipeFadeOut();
    mbObjMotionSet(work->modelId, 0, HU3D_MOTATTR_LOOP);
    mbPlayerMotIdleSet(playerNo);
    mbPlayerRotYSet(playerNo, 0.0f);
    for (i = 0; i < work->count; i++) {
        dataOrder[i] = i;
    }
    for (i = 0; i < 100; i++) {
        randNo1 = mbRandMod(work->count);
        randNo2 = mbRandMod(work->count);
        j = dataOrder[randNo1];
        dataOrder[randNo1] = dataOrder[randNo2];
        dataOrder[randNo2] = j;
    }
    mbev_PlayerColMasuAdd(playerNo, id, TRUE);
    mbPlayerPosReset(playerNo);
    for (i = 0; i < 4; i++) {
        CharModelVoiceFlagSet(GwPlayer[i].charNo, FALSE);
    }
    // Players sharing a space appear together and are processed only once.
    playerF = 0;
    for (i = 0; i < 4; i++) {
        if (playerF & (1 << i)) {
            continue;
        }
        mbCameraPlayerViewSetFast(i, 2);
        mbCameraFocusMasuSet(GwPlayer[i].masuId);
        HuPrcVSleep();
        id = GwPlayer[i].masuId;
        masuCount = 0;
        for (j = 1; j < mbMasuNumGet(); j++) {
            for (k = 0; k < 4; k++) {
                if (j == GwPlayer[k].masuId) {
                    break;
                }
            }
            if (k < 4) {
                continue;
            }
            if (!mbMasuDispCheck(j)) {
                continue;
            }
            mbMasuPosGet(j, &masuPos);
            if (!mbCameraCullCheck(&masuPos, 200.0f)) {
                continue;
            }
            masuIdTbl[masuCount++] = j;
        }
        playerCount = 0;
        for (j = 0; j < 4; j++) {
            if (playerF & (1 << j)) {
                continue;
            }
            if (id == GwPlayer[j].masuId) {
                playerNoTbl[playerCount++] = j;
            }
        }
        data = work->data;
        for (j = 0; j < work->count; j++, data++) {
            data->active = FALSE;
            data->state = 1;
            data->vel.x = data->vel.y = data->vel.z = 0.0f;
            data->delay = (dataOrder[j] * 2) + 1;
            if (!GwSystem.curTime) {
                mbCoinObjDispSet(data->modelId, FALSE);
            } else {
                mbObjDispSet(data->modelId, FALSE);
            }
        }
        dataCount = 0;
        data = work->data;
        for (j = 0; j < playerCount; j++) {
            curPlayerNo = playerNoTbl[j];

            mbPlayerPosGet(curPlayerNo, &masuPos);
            for (k = 0; k < coinNum[curPlayerNo]; k++, data++) {
                data->active = TRUE;
                data->pos.x = masuPos.x;
                data->pos.y = masuPos.y + 2000.0f;
                data->pos.z = masuPos.z;
                data->targetY = masuPos.y + 150.0f;
                data->playerNo = curPlayerNo;
                dataCount++;
            }
        }
        if (masuCount > 0) {
            for (j = 0; j < work->count - dataCount; j++, data++) {
                id = masuIdTbl[mbRandMod(masuCount)];
                mbMasuPosGet(id, &masuPos);
                data->active = TRUE;
                data->pos.x = masuPos.x;
                data->pos.y = masuPos.y + 2000.0f;
                data->pos.z = masuPos.z;
                data->targetY = masuPos.y + 50.0f;
                data->playerNo = -1;
                masuPos.y += 50.0f;
                data->particleModelId = fn_1_7388(&masuPos);
                Hu3DModelAttrSet(data->particleModelId, 1);
            }
        }
        mbWipeFadeIn();
        while (fn_1_6D98()) {
            HuPrcVSleep();
        }
        for (j = 0; j < 4; j++) {
            coinAdd[j] = 0;
        }
        for (j = 0; j < playerCount; j++) {
            curPlayerNo = playerNoTbl[j];

            playerF |= 1 << curPlayerNo;
            if (!GwSystem.curTime) {
                coinAdd[curPlayerNo] = coinNum[curPlayerNo];
            } else {
                coinAdd[curPlayerNo] = -coinNum[curPlayerNo];
            }
        }
        // The same amounts also enable the floating totals for affected players.
        mbCoinAddAllProcExecV(coinAdd, coinAdd, TRUE);
        mbWipeFadeOut();
    }
    mbPlayerMotionKill(playerNo, motionId);
    for (i = 0; i < 4; i++) {
        mbPlayerMotionKill(i, work->playerMotion[i]);
        mbPlayerMotionSet(i, 1, HU3D_MOTATTR_LOOP);
        CharModelVoiceFlagSet(GwPlayer[i].charNo, TRUE);
    }
    mbCameraMoveMasu(linkMasuId, &lbl_1_data_518, &lbl_1_data_524, 2600.0f,
        -1.0f, -1);
    mbWipeFadeIn();
    HuMemDirectFree(dataOrder);
    if (!GwSystem.curTime) {
        winId = mbWinCreate(2, W01_MESSAGE_STAR_RESULT_DAY, 17);
    } else {
        winId = mbWinCreate(2, W01_MESSAGE_STAR_RESULT_NIGHT, 18);
    }
    mbWinWait(winId);
}

// The coin-event process advances falling objects each frame and counts active entries.
// Night hits bounce away and blink; the actual coin totals change after the animation.
s32 fn_1_6D98(void)
{
    W01_COIN_ENTRY *data;
    s32 i;
    s32 count;
    W01_WORK_13D8 *work;
    HuVecF pos;
    float angle;
    s32 playerNo;
    s32 teamNo;
    s32 teamNo3;
    s32 teamNo2;

    count = 0;
    work = &lbl_1_bss_13D8;
    data = work->data;
    for (i = 0; i < work->count; i++, data++) {
        if (!data->active) {
            continue;
        }
        count++;
        if (data->delay != 0) {
            if (--data->delay != 0) {
                continue;
            }
            if (!GwSystem.curTime) {
                mbCoinObjDispSet(data->modelId, TRUE);
            } else {
                mbObjDispSet(data->modelId, TRUE);
            }
        }
        data->vel.y += -32.666668f;
        data->pos.x += (1.0f / 60.0f) * data->vel.x;
        data->pos.y += (1.0f / 60.0f) * data->vel.y;
        data->pos.z += (1.0f / 60.0f) * data->vel.z;
        if (!GwSystem.curTime) {
            mbCoinObjPosSetV(data->modelId, &data->pos);
            data->rot.y += 4.0f;
            mbCoinObjRotSet(data->modelId, 0.0f, data->rot.y, 0.0f);
        } else {
            mbObjPosSetV(data->modelId, &data->pos);
            data->rot.y += 10.0f;
            data->rot.x += 20.0f;
            mbObjRotSetV(data->modelId, &data->rot);
        }
        switch (data->state) {
            case 1:
                if (data->playerNo >= 0 && GwSystem.curTime
                    && data->pos.y < 500.0f + data->targetY) {
                    if (mbObjMotionGet(mbPlayerObjIDGet(data->playerNo))
                        != work->playerMotion[data->playerNo]) {
                        mbPlayerMotionShiftSet(data->playerNo,
                            work->playerMotion[data->playerNo], 0.0f, 4.0f,
                            HU3D_MOTATTR_NONE);
                    }
                }
                if (data->pos.y < data->targetY) {
                    if (data->playerNo < 0) {
                        Hu3DModelAttrReset(data->particleModelId,
                            HU3D_ATTR_DISPOFF);
                        if (!GwSystem.curTime) {
                            mbCoinObjDispSet(data->modelId, FALSE);
                        } else {
                            mbObjDispSet(data->modelId, FALSE);
                        }
                        data->active = FALSE;
                    } else if (!GwSystem.curTime) {
                        mbCoinEffCreate(&data->pos);
                        mbCoinObjDispSet(data->modelId, FALSE);
                        data->active = FALSE;
                    } else {
                        angle = 360.0f * frandf();
                        data->vel.x = 100.0f * (6.0f * mbCosDeg(angle));
                        data->vel.y = 1000.0f;
                        data->vel.z = 100.0f * (6.0f * mbSinDeg(angle));
                        data->state = 2;
                        data->targetY -= 150.0f;
                        data->blinkTime = 0;
                        omVibrate(data->playerNo, 20, 7, 3);
                        mbPlayerMotionShiftSet(data->playerNo, 6, 0.0f, 4.0f,
                            HU3D_MOTATTR_LOOP);
                        work->hitReactionAmount[data->playerNo] = 0.5f;
                        if (!GWTeamFGet()) {
                            if (work->coinCount[data->playerNo] > 0) {
                                mbPlayerPosGet(data->playerNo, &pos);
                                pos.y += 100.0f;
                                fn_1_13D8C(&pos, 1);
                                work->coinCount[data->playerNo]--;
                            }
                        } else {
                            playerNo = data->playerNo;
                            teamNo = GwPlayer[playerNo].team;
                            teamNo3 = teamNo;
                            teamNo2 = teamNo3;
                            if (work->coinCount[teamNo2] > 0) {
                                mbPlayerPosGet(data->playerNo, &pos);
                                pos.y += 100.0f;
                                fn_1_13D8C(&pos, 1);
                                work->coinCount[teamNo2]--;
                            }
                        }
                        mbAudFXPlay(0x597);
                        CharFXPlay(GwPlayer[data->playerNo].charNo, 0x248);
                    }
                }
                break;
            case 2:
                if (++data->blinkTime & 1) {
                    mbObjDispSet(data->modelId, TRUE);
                } else {
                    mbObjDispSet(data->modelId, FALSE);
                }
                if (data->pos.y < data->targetY) {
                    mbObjDispSet(data->modelId, FALSE);
                    data->active = FALSE;
                }
                break;
        }
    }
    return count;
}

// The coin event creates a day burst or night animated puff at an unoccupied space.
HU3D_MODELID fn_1_7388(HuVecF *pos)
{
    HU3D_MODELID modelId;

    if (GwSystem.curTime == 0) {
        modelId = mbParticleCreate(lbl_1_bss_13D0[0], 10);
        mbParticleHookSet(modelId, fn_1_7448);
    } else {
        modelId = mbParticleCreate(lbl_1_bss_13D0[1], 1);
        mbParticleHookSet(modelId, fn_1_77AC);
        mbParticleAttrSet(modelId, MB_PARTICLE_ATTR_UPAUSE);
    }
    Hu3DModelLayerSet(modelId, 5);
    Hu3DModelPosSetV(modelId, pos);
    return modelId;
}

// The daytime landing-particle callback scatters a short burst and frees it when faded.
void fn_1_7448(HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx mtx)
{
    GXColor color = { 255, 255, 192, 192 };
    MBPARTICLEDATA *data;
    float angle;
    float radius;
    float time;
    s32 count;
    s32 i;

    if (particleP->mode == 0) {
        data = particleP->data;
        for (i = 0; i < particleP->num; i++, data++) {
            angle = 360.0f * frandf();
            radius = 200.0f + (100.0f * (2.0f * frandf()));
            data->pos.x = data->pos.y = data->pos.z = 0.0f;
            data->vel.x = radius * mbCosDeg(angle);
            data->vel.y = 500.0f + (100.0f * (5.0f * frandf()));
            data->vel.z = radius * mbSinDeg(angle);
            data->scale = 40.0f + (20.0f * frandf());
            data->rot.z = mbRandMod(360);
            data->color = color;
            data->time = 0;
            data->activeF = 30;
        }
        particleP->mode = 1;
    }
    count = 0;
    data = particleP->data;
    for (i = 0; i < particleP->num; i++, data++) {
        if (data->time <= data->activeF) {
            data->vel.y += -65.333336f;
            data->pos.x += (1.0f / 60.0f) * data->vel.x;
            data->pos.y += (1.0f / 60.0f) * data->vel.y;
            data->pos.z += (1.0f / 60.0f) * data->vel.z;
            data->rot.z += 10.0f;
            if ((data->activeF - data->time) < 20) {
                time = (float)(data->activeF - data->time) / 20.0f;
                data->color.a = 255.0f * time;
            }
            data->time++;
            count++;
        }
    }
    if (count == 0) {
        mbParticleKill(particleP->modelId);
    }
}

// The night landing-particle callback seeds the puff and frees it once playback pauses.
void fn_1_77AC(HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx mtx)
{
    GXColor color = { 255, 255, 255, 192 };
    MBPARTICLEDATA *data;
    s32 count;
    s32 i;

    if (particleP->mode == 0) {
        data = particleP->data;
        for (i = 0; i < particleP->num; i++, data++) {
            data->pos.x = 50.0f - (100.0f * frandf());
            data->pos.y = 0.0f;
            data->pos.z = 50.0f - (100.0f * frandf());
            data->scale = 200.0f + (20.0f * frandf());
            data->color = color;
            data->animSpeed = 0.5f;
        }
        particleP->mode = 1;
    }
    count = 0;
    data = particleP->data;
    for (i = 0; i < particleP->num; i++, data++) {
        if (data->pauseF == FALSE) {
            count++;
        }
    }
    if (count == 0) {
        mbParticleKill(particleP->modelId);
    }
}

// MB1_Create loads the three animated spaces and their two reusable falling effects.
void fn_1_7944(void)
{
    W01_MASU_MOTION_WORK *work;
    u32 attr;
    s32 i;

    work = lbl_1_bss_137C;
    lbl_1_bss_1378 = HuSprAnimDataRead(
        (MBTimeDayGet() ? W01_DATA_DIR_DAY : W01_DATA_DIR_NIGHT)
            | W01_FILE_MASU_MOTION_EFFECT);
    HuSprAnimLock(lbl_1_bss_1378);
    for (i = 0; i < 3; i++, work++) {
        work->modelId = mbObjCreate(
            (lbl_1_data_550[i] & 0xFFFF)
                | (MBTimeDayGet() ? W01_DATA_DIR_DAY : W01_DATA_DIR_NIGHT),
            NULL, FALSE);
        mbObjMotionSpeedSet(work->modelId, 0.0f);
    attr = mbev_MasuAttrGet(i + 1, W01_MASU_ATTR_EVENT_INDEX_MASK);
    work->masuId = mbMasuFind_MAttrMatchIdGet(
        -1, attr, W01_MASU_ATTR_EVENT_INDEX_MASK);
        strcpy(work->objName, lbl_1_data_56C[i]);

        work->effectModelId[0] = mbParticleCreate(lbl_1_bss_1378, 20);
        Hu3DModelLayerSet(work->effectModelId[0], 5);
        mbParticleHookSet(work->effectModelId[0], fn_1_7D84);
        Hu3DModelAttrSet(work->effectModelId[0], HU3D_ATTR_DISPOFF);

        work->effectModelId[1] = mbParticleCreate(lbl_1_bss_1378, 20);
        Hu3DModelLayerSet(work->effectModelId[1], 5);
        mbParticleHookSet(work->effectModelId[1], fn_1_80B4);
        Hu3DModelAttrSet(work->effectModelId[1], HU3D_ATTR_DISPOFF);
    }
}

// MB1_Kill releases the sprite animation shared by the three animated-space effects.
void fn_1_7B20(void)
{
    if (lbl_1_bss_1378) {
        HuSprAnimKill(lbl_1_bss_1378);
        lbl_1_bss_1378 = NULL;
    }
}

// The space-move completion callback starts the selected space motion and its particles.
void fn_1_7B70(s32 index)
{
    W01_MASU_MOTION_WORK *work = &lbl_1_bss_137C[index];
    HuVecF pos;

    mbObjMotionTimeSet(work->modelId, 0.0f);
    mbObjMotionSpeedSet(work->modelId, 1.0f);
    mbMasuPosGet(work->masuId, &pos);
    pos.y -= 100.0f;
    fn_1_7D14(work, &pos);
    mbAudFXPlay(0x590);
    work->active = TRUE;
}

// The board update follows each active space's model joint until its motion finishes.
void fn_1_7C14(void)
{
    W01_MASU_MOTION_WORK *work;
    s32 i;
    HU3D_MODELID modelId;
    HU3D_MODEL *modelP;
    HuVecF pos;

    work = lbl_1_bss_137C;
    for (i = 0; i < 3; i++, work++) {
        if (work->active) {
            if (mbObjMotionEndCheck(work->modelId)) {
                work->active = FALSE;
            }
            modelId = mbObjModelIDGet(work->modelId);
            modelP = &Hu3DData[modelId];
            // Sample one frame ahead when updating the space's collision position.
            Hu3DMotionExec(modelId, modelP->motId,
                1.0f + mbObjMotionTimeGet(work->modelId), FALSE);
            Hu3DModelObjPosGet(modelId, work->objName, &pos);
            pos.y += 7.5f;
            mbMasuPosSetV(work->masuId, &pos);
        }
    }
}

// Starting an animated space places and reveals both of its reusable particle models.
void fn_1_7D14(W01_MASU_MOTION_WORK *work, HuVecF *pos)
{
    s32 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelPosSetV(work->effectModelId[i], pos);
        Hu3DModelAttrReset(work->effectModelId[i], HU3D_ATTR_DISPOFF);
    }
}

// This animated-space particle callback drops a fading ring, then hides it for reuse.
void fn_1_7D84(HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx mtx)
{
    GXColor color = { 255, 255, 192, 192 };
    MBPARTICLEDATA *data;
    float angle;
    float radius;
    float ratio;
    s32 count;
    s32 i;

    if (particleP->mode == 0) {
        data = particleP->data;
        for (i = 0; i < particleP->num; i++, data++) {
            angle = 360.0f * frandf();
            radius = 100.0f + (100.0f * frandf());
            data->pos.x = radius * mbCosDeg(angle);
            data->pos.y = 0.0f;
            data->pos.z = radius * mbSinDeg(angle);
            data->vel.y = -500.0f - (500.0f * frandf());
            data->accel.y = -500.0f - (500.0f * frandf());
            data->scale = 100.0f + (20.0f * frandf());
            data->color = color;
            data->time = 0;
            data->activeF = 60.0f + (30.0f * frandf());
        }
        particleP->mode = 1;
    }
    count = 0;
    data = particleP->data;
    for (i = 0; i < particleP->num; i++, data++) {
        if (data->time <= data->activeF) {
            ratio = (float)data->time++ / (float)data->activeF;
            data->pos.y += 0.016666668f * data->vel.y;
            data->vel.y += 0.016666668f * data->accel.y;
            data->color.a = 192.0f * (1.0f - ratio);
            count++;
        }
    }
    if (count == 0) {
        particleP->mode = 0;
        Hu3DModelAttrSet(particleP->modelId, HU3D_ATTR_DISPOFF);
    }
}

// This animated-space particle callback spreads a falling burst, then hides it for reuse.
void fn_1_80B4(HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx mtx)
{
    GXColor color = { 255, 255, 192, 192 };
    MBPARTICLEDATA *data;
    float angle;
    float radius;
    float time;
    s32 count;
    s32 i;

    if (particleP->mode == 0) {
        data = particleP->data;
        for (i = 0; i < particleP->num; i++, data++) {
            angle = 360.0f * frandf();
            radius = 200.0f + (100.0f * (0.5f * frandf()));
            data->pos.x = radius * mbCosDeg(angle);
            data->pos.y = 0.0f;
            data->pos.z = radius * mbSinDeg(angle);
            data->vel.x = 100.0f * (10.0f * mbCosDeg(angle));
            data->vel.y = -1000.0f;
            data->vel.z = 100.0f * (10.0f * mbSinDeg(angle));
            data->scale = 100.0f + (20.0f * frandf());
            data->color = color;
            data->time = 0;
            data->activeF = 60.0f + (30.0f * frandf());
        }
        particleP->mode = 1;
    }
    count = 0;
    data = particleP->data;
    for (i = 0; i < particleP->num; i++, data++) {
        if (data->time <= data->activeF) {
            time = (float)data->time++ / (float)data->activeF;
            data->pos.x += (1.0f / 60.0f) * data->vel.x;
            data->pos.y += (1.0f / 60.0f) * data->vel.y;
            data->pos.z += (1.0f / 60.0f) * data->vel.z;
            data->vel.x *= 0.9f;
            data->vel.y *= 0.9f;
            data->vel.z *= 0.9f;
            data->color.a = 192.0f * (1.0f - time);
            count++;
        }
    }
    if (count == 0) {
        particleP->mode = 0;
        Hu3DModelAttrSet(particleP->modelId, HU3D_ATTR_DISPOFF);
    }
}

// MB1_Create loads the two spring routes and removes the link unavailable at this time.
// The two routes use opposite day/night states even though their assets share a directory.
void fn_1_8474(void)
{
    W01_MASU_MODEL_WORK *work;
    MASU *masuP;
    BOOL dayF;
    BOOL dayF2;
    s32 dataNum;
    BOOL dayF3;
    s32 dataDir;
    s32 i;
    s32 j;
    s32 k;

    work = lbl_1_bss_1338;
    dayF = !GwSystem.curTime;
    if (dayF != FALSE) {
        dayF3 = TRUE;
    } else {
        dayF3 = FALSE;
    }
    lbl_1_bss_1338[0].dayF = dayF3;
    lbl_1_bss_1338[1].dayF = lbl_1_bss_1338[0].dayF ^ TRUE;

    work = lbl_1_bss_1338;
    for (i = 0; i < 2; i++, work++) {
        if (work->dayF != FALSE) {
            dataNum = lbl_1_data_578[i];
        } else {
            dataNum = lbl_1_data_580[i];
        }
        dayF2 = !GwSystem.curTime;
        if (dayF2 != FALSE) {
            dataDir = W01_DATA_DIR_DAY;
        } else {
            dataDir = W01_DATA_DIR_NIGHT;
        }
        work->modelId = mbObjCreate((dataNum & 0xFFFF) | dataDir, NULL,
            FALSE);
        mbObjMotionSet(work->modelId, 0, 0);
        mbObjMotionSpeedSet(work->modelId, 0.0f);
        mbObjMotionShapeSet(work->modelId, 0, 0);
        mbObjMotionShapeSpeedSet(work->modelId, 0.0f);

        work->masuId = mbMasuFind_MAttrIdGet(-1, lbl_1_data_588[i][0]);
        work->linkMasuId[0] =
            mbMasuFind_MAttrIdGet(-1, lbl_1_data_588[i][1]);
        work->linkMasuId[1] =
            mbMasuFind_MAttrIdGet(-1, lbl_1_data_588[i][2]);

        masuP = mbMasuGet(work->masuId);
        if (work->dayF != FALSE) {
            for (j = 0; j < masuP->linkNum; j++) {
                if (masuP->linkTbl[j] == work->linkMasuId[1]) {
                    // Each later link overwrites the same removed slot.
                    for (k = j + 1; k < masuP->linkNum; k++) {
                        masuP->linkTbl[j] = masuP->linkTbl[k];
                    }
                    masuP->linkNum--;
                }
            }
        } else {
            for (j = 0; j < masuP->linkNum; j++) {
                if (masuP->linkTbl[j] == work->linkMasuId[0]) {
                    // Each later link overwrites the same removed slot.
                    for (k = j + 1; k < masuP->linkNum; k++) {
                        masuP->linkTbl[j] = masuP->linkTbl[k];
                    }
                    masuP->linkNum--;
                }
            }
        }

        strcpy(work->objName, lbl_1_data_5AC[i]);
        work->active = TRUE;
    }

    lbl_1_bss_1334 = HuSprAnimRead(HuDataSelHeapReadNum(
        mbBoardDataNumGet(W01_DATA_STAR_ANIM_DAY), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprAnimLock(lbl_1_bss_1334);
}

// MB1_Kill releases the animation used by the spring route's player trail.
void fn_1_8798(void)
{
    if (lbl_1_bss_1334) {
        HuSprAnimKill(lbl_1_bss_1334);
        lbl_1_bss_1334 = NULL;
    }
}

// Before a player's turn, place both spring spaces at their current model joints.
void fn_1_87E8(void)
{
    W01_MASU_MODEL_WORK *work;
    HuVecF pos;
    s32 i;

    work = lbl_1_bss_1338;
    for (i = 0; i < 2; i++, work++) {
        if (work->active != 0) {
            Hu3DModelObjPosGet(mbObjModelIDGet(work->modelId), work->objName,
                &pos);
            mbMasuPosSetV(work->masuId, &pos);
        }
    }
}

// The spring-space movement hook launches or hops the player to its available linked space.
// The route's day state selects the long spinning launch and trailing particles.
void fn_1_8860(int playerNo)
{
    s32 workNo2;
    s16 masuId;
    s32 particleNo;
    s32 workNo;
    W01_MASU_MODEL_WORK *work;
    HU3D_MODELID particleModelId;
    HU3D_MODELID modelId;
    s32 time;
    s32 i;
    s32 j;
    HuVecF playerPos;
    HuVecF masuPos;
    HuVecF pos;
    HuVecF delta;
    HuVecF playerRot;
    HuVecF moveDelta;
    HuVecF movePos;
    HuVecF startPos;
    float weight;
    float rotY;
    float weight2;
    float rotT;
    float srcRotY;

    masuId = GwPlayer[playerNo].masuId;
    if (mbMasuMAttrGet(masuId) & W01_MASU_ATTR_MOVE_HOOK_SPRING_A) {
        workNo = 0;
    } else {
        workNo = 1;
    }
    workNo2 = workNo;
    work = &lbl_1_bss_1338[workNo2];
    GwPlayer[playerNo].moveF = TRUE;
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbPlayerWorkGet(playerNo)->moveEndF = FALSE;
    if (work->dayF != FALSE) {
        particleModelId = mbParticleCreate(lbl_1_bss_1334, 200);
        Hu3DModelLayerSet(particleModelId, 5);
        mbParticleHookSet(particleModelId, fn_1_9134);
        Hu3DParticleBlendModeSet(particleModelId, 1);
        mbPlayerMotIdleSet(playerNo);
        mbObjMotionTimeSet(work->modelId, 0.0f);
        mbObjMotionSpeedSet(work->modelId, 1.0f);
        mbObjMotionShapeTimeSet(work->modelId, 0.0f);
        mbObjMotionShapeSpeedSet(work->modelId, 1.0f);
        modelId = mbObjModelIDGet(work->modelId);
        mbAudFXPlay(0x591);
        for (i = 0; (u32)i < 60; i++) {
            if ((u32)i == 30) {
                mbPlayerScaleSet(playerNo, 1.0f, 0.5f, 1.0f);
            }
            Hu3DModelObjPosGet(modelId, work->objName, &pos);
            mbPlayerPosSetV(playerNo, &pos);
            HuPrcVSleep();
        }
        mbAudFXPlay(0x592);
        mbPlayerPosGet(playerNo, &playerPos);
        mbMasuPosGet(work->linkMasuId[0], &masuPos);
        PSVECSubtract(&masuPos, &playerPos, &delta);
        rotY = (float)((atan2(delta.x, delta.z) / M_PI) * 180.0);
        mbPlayerRotYSet(playerNo, 180.0f + rotY);
        mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 2.0f, 0);
        mbPlayerRotGet(playerNo, &playerRot);
        mbPlayerScaleSet(playerNo, 1.0f, 1.0f, 1.0f);
        GwPlayer[playerNo].masuIdNext = work->linkMasuId[0];
        particleNo = 0;
        omVibrate(playerNo, 20, 7, 3);
        for (i = 0; (u32)i <= 48; i++) {
            weight = (float)i / 48.0f;
            if ((u32)i == 24) {
                CharModelVoiceFlagSet(GwPlayer[playerNo].charNo, FALSE);
                mbPlayerMotionShiftSet(playerNo, 5, 0.0f, 2.0f, 0);
            } else if ((u32)i == 30) {
                mbObjMotionSpeedSet(work->modelId, -1.0f);
                mbObjMotionShapeSpeedSet(work->modelId, -1.0f);
            }
            pos.x = playerPos.x + (weight * delta.x);
            pos.y = playerPos.y + (weight * delta.y)
                + (100.0f * (10.0f * mbSinDeg(180.0f * weight)));
            pos.z = playerPos.z + (weight * delta.z);
            mbPlayerPosSetV(playerNo, &pos);
            mbPlayerRotSet(playerNo,
                360.0f * mbSinDeg(90.0f * (1.0f - weight)),
                playerRot.y, playerRot.z);
            fn_1_939C(particleModelId, particleNo++, &pos);
            mbPlayerWorkGet(playerNo)->_unk08 = 48 - i;
            HuPrcVSleep();
        }
        CharModelVoiceFlagSet(GwPlayer[playerNo].charNo, TRUE);
        mbObjMotionTimeSet(work->modelId, 0.0f);
        mbObjMotionSpeedSet(work->modelId, 0.0f);
        mbObjMotionShapeTimeSet(work->modelId, 0.0f);
        mbObjMotionShapeSpeedSet(work->modelId, 0.0f);
        GwPlayer[playerNo].masuId = work->linkMasuId[0];
    } else {
        mbAudFXPlay(0x593);
        mbObjMotionTimeSet(work->modelId, 5.0f);
        mbObjMotionSpeedSet(work->modelId, 1.0f);
        mbObjMotionShapeTimeSet(work->modelId, 5.0f);
        mbObjMotionShapeSpeedSet(work->modelId, 1.0f);
        mbPlayerPosGet(playerNo, &playerPos);
        mbMasuPosGet(work->linkMasuId[1], &masuPos);
        PSVECSubtract(&masuPos, &playerPos, &delta);
        rotY = (float)((atan2(delta.x, delta.z) / M_PI) * 180.0);
        mbPlayerRotYSet(playerNo, rotY);
        GwPlayer[playerNo].masuIdNext = work->linkMasuId[1];
        omVibrate(playerNo, 20, 4, 4);
        mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, 0);
        mbPlayerPosGet(playerNo, &startPos);
        srcRotY = mbPlayerRotYGet(playerNo);
        PSVECSubtract(&masuPos, &startPos, &moveDelta);
        for (j = 0; j <= 30; j++) {
            weight2 = (float)j / 30.0f;
            if ((u32)j == 24) {
                mbPlayerMotionShiftSet(playerNo, 5, 0.0f, 2.0f, 0);
            }
            movePos.x = startPos.x + (weight2 * moveDelta.x);
            movePos.y = startPos.y + (weight2 * moveDelta.y)
                + (300.0 * sin(M_PI * (180.0f * weight2) / 180.0f));
            movePos.z = startPos.z + (weight2 * moveDelta.z);
            mbPlayerPosSetV(playerNo, &movePos);
            rotT = (float)j / 6.0f;
            if (rotT > 1.0f) {
                rotT = 1.0f;
            }
            mbPlayerRotYSet(playerNo, mbAngleLerp(srcRotY, rotY, rotT));
            time = 30 - j;
            mbPlayerWorkGet(playerNo)->_unk08 = time;
            HuPrcVSleep();
        }
        mbPlayerMotionShiftSet(
            playerNo, 1, 0.0f, 2.0f, HU3D_MOTATTR_LOOP);
        GwPlayer[playerNo].masuId = work->linkMasuId[1];
    }
    GwPlayer[playerNo].moveF = FALSE;
    mbPlayerColSnapPlayerSet(playerNo, TRUE);
    mbPlayerWorkGet(playerNo)->moveEndF = TRUE;
}

// The spring trail callback fades emitted particles and frees the completed trail.
void fn_1_9134(HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx mtx)
{
    MBPARTICLEDATA *data;
    float time;
    s32 count;
    s32 i;

    if (particleP->mode == 0) {
        data = particleP->data;
        for (i = 0; i < particleP->num; i++, data++) {
            data->pos.x = data->pos.y = data->pos.z = 0.0f;
            data->scale = 0.0f;
            data->color.r = 255;
            data->color.g = 255;
            data->color.b = mbRandMod(128) + 128;
            data->rot.z = 360.0f * frandf();
            data->time = -1;
            data->activeF = 30.0f + (30.0f * frandf());
        }
        particleP->mode = 1;
    }

    count = 0;
    data = particleP->data;
    for (i = 0; i < particleP->num; i++, data++) {
        if (data->time >= 0 && data->time <= data->activeF) {
            time = (float)data->time++ / (float)data->activeF;
            data->vel.y += -16.333334f;
            data->pos.y += (1.0f / 60.0f) * data->vel.y;
            data->color.a = 255.0f * (1.0f - time);
            data->rot.z += 8.0f;
            count++;
        }
    }

    if (particleP->mode == 2 && count == 0) {
        mbParticleKill(particleP->modelId);
    }
}

// Each spring-launch frame activates one trail particle near the player's position.
void fn_1_939C(HU3D_MODELID modelId, s32 particleNo, HuVecF *pos)
{
    MBPARTICLE *particleWorkP;
    MBPARTICLEDATA *particleDataP;
    MBPARTICLE *particleP;

    particleWorkP = Hu3DData[modelId].hookData;
    particleP = particleWorkP;
    particleDataP = &particleP->data[particleNo];
    particleDataP->pos.x = pos->x + 1.2f * frandf() * 100.0f - (1.2f * 50.0f);
    particleDataP->pos.y = pos->y + 1.2f * frandf() * 100.0f - (1.2f * 50.0f);
    particleDataP->pos.z = pos->z + 1.2f * frandf() * 100.0f - (1.2f * 50.0f);
    particleDataP->scale = 30.0f + 30.0f * frandf();
    particleDataP->time = 0;
    particleP->mode = 2;
}

// The time-change presentation hides or reveals the spring models and their space updates.
void fn_1_950C(BOOL dispF)
{
    W01_MASU_MODEL_WORK *work;
    s32 i;

    work = lbl_1_bss_1338;
    for (i = 0; i < 2; i++, work++) {
        mbObjDispSet(work->modelId, dispF);
        work->active = dispF;
    }
}

// MB1_Create calls this to load the day/night special-space effect and particles.
void fn_1_9574(void)
{
    W01_WH_EFFECT_WORK *work;
    MBMODELID *modelIdP;
    HuVecF pos;
    u32 attr;
    s32 i;

    work = lbl_1_bss_1240.work;
    modelIdP = lbl_1_bss_1238;
    modelIdP[0] = mbObjCreate(
        (MBTimeDayGet() ? W01_DATA_DAY : W01_DATA_NIGHT)
            | W01_FILE_EFFECT_ROOT, NULL, FALSE);
    mbObjAttrSet(modelIdP[0], W01_OBJ_ATTR_EFFECT_PAUSED);
    mbObjPosSet(modelIdP[0], 410.0f, 1900.0f, 1137.0f);

    modelIdP[1] = mbObjCreate(
        (MBTimeDayGet() ? W01_DATA_DAY : W01_DATA_NIGHT)
            | W01_FILE_EFFECT_CHILD, NULL, FALSE);
    mbObjAttrSet(modelIdP[1], W01_OBJ_ATTR_BOARD_MODEL);
    mbObjHookSet(modelIdP[0], lbl_1_data_638, modelIdP[1]);

    if (MBTimeDayGet()) {
        modelIdP[2] = mbObjCreate(W01_MODEL_LIGHT_DAY, lbl_1_data_5D4, FALSE);
    } else {
        modelIdP[2] = mbObjCreate(W01_MODEL_LIGHT_NIGHT, lbl_1_data_5E4, FALSE);
    }
    if (!MBTimeDayGet()) {
        modelIdP[3] = Hu3DLLightCreateV(
            mbObjModelIDGet(modelIdP[2]),
            &lbl_1_data_61C, &lbl_1_data_628, &lbl_1_data_634);
        Hu3DLLightStaticSet(
            mbObjModelIDGet(modelIdP[2]), modelIdP[3], TRUE);
        Hu3DLLightInfinitytSet(
            mbObjModelIDGet(modelIdP[2]), modelIdP[3]);
    }
    mbObjMotionSet(modelIdP[2], 1, HU3D_MOTATTR_LOOP);
    mbObjDispSet(modelIdP[2], FALSE);
    mbObjPosGet(modelIdP[0], &pos);
    mbObjPosSetV(modelIdP[2], &pos);

    for (i = 0; i < 3; i++, work++) {
        work->modelId = mbObjCreate(
            (lbl_1_data_5F4[i] & 0xFFFF)
                | (MBTimeDayGet() ? W01_DATA_DAY : W01_DATA_NIGHT),
            NULL, FALSE);
        strcpy(work->objName, lbl_1_data_610[i]);
        mbObjDispSet(work->modelId, FALSE);
        work->particleCount = fn_1_B434(&work->particle);
        work->state = 1;
        work->object = omAddObjEx(mbObjMan, 8204, 0, 0, -1, fn_1_ACDC);
        work->object->work[0] = i;
        work->playerNo = -1;
    }

    for (i = 0; i < 4; i++) {
        attr = mbev_MasuAttrGet(i + 2, W01_MASU_ATTR_EVENT_TYPE_MASK);
        lbl_1_bss_1230[i] =
            mbMasuFind_MAttrMatchIdGet(-1, attr, W01_MASU_ATTR_EVENT_TYPE_MASK);
    }

    lbl_1_bss_121C = HuSprAnimRead(HuDataSelHeapReadNum(
        (MBTimeDayGet() ? W01_DATA_DIR_DAY : W01_DATA_DIR_NIGHT)
            | W01_FILE_WH_EFFECT_STAR_ANIM,
        HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprAnimLock(lbl_1_bss_121C);
    lbl_1_bss_120C[0] = fn_1_C7DC(&lbl_1_bss_1214[0]);
    lbl_1_bss_120C[1] = fn_1_CAA4(&lbl_1_bss_1214[1]);

    lbl_1_bss_1220 = Hu3DHookFuncCreate(fn_1_BFA0);
    Hu3DModelCameraSet(lbl_1_bss_1220, 1);
    Hu3DModelLayerSet(lbl_1_bss_1220, 5);
    lbl_1_bss_1224.x = 12000.0f;
    lbl_1_bss_1224.y = 3000.0f;
    lbl_1_bss_1224.z = 2400.0f;
    HuDataDirClose(W01_DATA_DIR_LIGHT_EFFECT);
}

// MB1_Kill releases the special-space glow animation and particle buffers.
void fn_1_99E4(void)
{
    W01_WH_EFFECT_WORK *work;
    void *data;
    W01_WH_PARTICLE *particle;
    s32 i;

    work = lbl_1_bss_1240.work;
    if (lbl_1_bss_121C) {
        HuSprAnimKill(lbl_1_bss_121C);
        lbl_1_bss_121C = NULL;
    }
    for (i = 0; i < 2; i++) {
        if (lbl_1_bss_1214[i]) {
            data = lbl_1_bss_1214[i];
            HuMemDirectFree(data);
            lbl_1_bss_1214[i] = NULL;
        }
    }
    for (i = 0; i < 3; i++, work++) {
        particle = work->particle;
        HuMemDirectFree(particle);
        work->particle = NULL;
    }
}

// MB1Ev_MasuHatena calls this for the board's special-space event.
// It presents the day/night choice, launches the player, and moves them to a destination space.
void fn_1_9AEC(s32 playerNo, s16 id)
{
    MBMODELID *modelIdP;
    MBMODELID modelId;
    W01_WH_EFFECT_WORK *work;
    HuVecF linkPos;
    HuVecF objPos;
    HuVecF playerPos;
    HuVecF pos;
    HuVecF delta;
    HuVecF delta2;
    HuVecF pos2;
    HuVecF startPos;
    s16 dataOrder[4];
    s32 randNo1;
    s32 randNo2;
    s32 type;
    s16 linkMasuId;
    s16 helpWinId;
    s16 motionId;
    s16 masuId;
    s16 winId;
    s32 distance;
    s32 choice;
    s32 temp;
    s32 i;
    s32 k;
    s32 j;
    s32 time;
    float rotY;
    float weight;
    float weight2;
    float weight3;

    work = lbl_1_bss_1240.work;
    modelIdP = lbl_1_bss_1238;
    mbObjMotionSpeedSet(modelIdP[0], 1.0f);
    modelId = modelIdP[0];
    mbObjAttrReset(modelId, W01_OBJ_ATTR_EFFECT_PAUSED);
    mbObjDispSet(modelIdP[2], TRUE);
    mbObjMotionSet(modelIdP[2], 1, HU3D_MOTATTR_LOOP);
    mbPlayerRotateStart(playerNo, 180, 15);
    linkMasuId = mbMasuAttrFindLink(id, W01_MASU_ATTR_LINK);
    mbMasuPosGet(linkMasuId, &linkPos);
    if (MBTimeDayGet()) {
        winId = mbWinCreate(2, W01_MESSAGE_WH_INTRO_DAY, 8);
        mbAudFXDelaySet(30);
        mbAudFXPlay(W01_SOUND_WH_EVENT_DAY);
    } else {
        winId = mbWinCreate(2, W01_MESSAGE_WH_INTRO_NIGHT, 9);
        mbAudFXDelaySet(30);
        mbAudFXPlay(W01_SOUND_WH_EVENT_NIGHT);
    }
    mbWinWait(winId);
    if (MBTimeDayGet()) {
        winId = mbWinCreateChoice(2, W01_MESSAGE_WH_CHOICE_DAY, 8, 0);
    } else {
        winId = mbWinCreateChoice(2, W01_MESSAGE_WH_CHOICE_NIGHT, 9, 0);
    }
    // Read the stored choice once before waiting; this result is discarded.
    mbWinChoiceGet(winId);
    if (GwPlayer[playerNo].comF) {
        distance = mbMasuFind_IdStepGet(id, lbl_1_bss_1616);
        if (distance > 13 && frandf() < 0.8f) {
            mbComChoiceLeftSet();
        } else {
            mbComChoiceRightSet();
        }
    }
    mbWinWait(winId);
    choice = mbWinChoiceGet(winId);
    if (choice < 0 || choice == 1) {
        mbObjMotionSpeedSet(modelIdP[0], -1.0f);
        while (mbObjMotionTimeGet(modelIdP[0]) != 0.0f) {
            HuPrcVSleep();
        }
        mbObjDispSet(modelIdP[2], FALSE);
        return;
    }
    mbCameraMoveMasu(linkMasuId, &lbl_1_data_644, &lbl_1_data_650,
        2638.0f, -1.0f, 30);
    mbPlayerPosGet(playerNo, &playerPos);
    PSVECSubtract(&linkPos, &playerPos, &delta);
    delta.z += 100.0f;
    mbPlayerMotionShiftSet(playerNo, 3, 0.0f, 2.0f, 0);
    for (i = 0; (u32)i <= 30; i++) {
        weight = (float)i / 30.0f;
        pos.x = playerPos.x + (delta.x * weight);
        pos.y = playerPos.y + (delta.y * weight);
        pos.z = playerPos.z + (delta.z * weight);
        mbPlayerPosSetV(playerNo, &pos);
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 2.0f, HU3D_MOTATTR_LOOP);
    mbCameraMoveWait();
    mbObjMotionShiftSet(modelIdP[2], 2, 0.0f, 4.0f, 0);
    if (!GwSystem.curTime) {
        winId = mbWinCreate(2, W01_MESSAGE_WH_PLAYER_MOVE_DAY, 8);
    } else {
        winId = mbWinCreate(2, W01_MESSAGE_WH_PLAYER_MOVE_NIGHT, 9);
    }
    for (i = 0; i < 3; i++, work++) {
        fn_1_B7D8(i);
        mbObjDispSet(work->modelId, TRUE);
        Hu3DModelAttrReset(lbl_1_bss_1220, 1);
    }
    for (i = 0; (u32)i < 60; i++) {
        weight = (float)i / 60.0f;
        work = lbl_1_bss_1240.work;
        for (j = 0; j < 3; j++, work++) {
            work->pos.x = linkPos.x
                + (100.0f * (5.0f * (float)(j - 1)));
            work->pos.y = 1000.0f + ((1000.0f + linkPos.y)
                - (100.0f * (7.0f * mbSinDeg(90.0f * weight))));
            work->pos.z = linkPos.z;
            mbObjPosSet(work->modelId, work->pos.x, work->pos.y - 1000.0f,
                work->pos.z);
        }
        HuPrcVSleep();
    }
    mbWinWait(winId);
    motionId = mbPlayerMotionCreate(playerNo, W01_PLAYER_MOTION_SPRING_LAUNCH);
    helpWinId = mbWinCreateHelp(W01_MESSAGE_WH_CHOICE_HELP);
    mbWinPosSet(helpWinId, 216, 384);
    for (i = 0; i < 4; i++) {
        dataOrder[i] = i;
    }
    for (i = 0; i < 100; i++) {
        randNo1 = mbRandMod(4);
        randNo2 = mbRandMod(4);
        temp = dataOrder[randNo1];
        dataOrder[randNo1] = dataOrder[randNo2];
        dataOrder[randNo2] = temp;
    }
    choice = fn_1_A82C(playerNo);
    work = &lbl_1_bss_1240.work[choice];
    mbPlayerPosGet(playerNo, &playerPos);
    mbObjPosGet(work->modelId, &objPos);
    PSVECSubtract(&objPos, &playerPos, &delta);
    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 2.0f, 0);
    for (i = 0; (u32)i < 30; i++) {
        weight = (float)i / 30.0f;
        if ((u32)i == 24) {
            mbPlayerMotionShiftSet(playerNo, motionId, 0.0f, 2.0f,
                HU3D_MOTATTR_LOOP);
        }
        pos.x = playerPos.x + (delta.x * weight);
        pos.y = playerPos.y + (delta.y * weight)
            + (100.0f * (1.5f * mbSinDeg(180.0f * weight)));
        pos.z = playerPos.z + (delta.z * weight);
        mbPlayerPosSetV(playerNo, &pos);
        HuPrcVSleep();
    }
    omVibrate(playerNo, 20, 4, 4);
    fn_1_AFD4(choice, playerNo);
    mbWinKill(helpWinId);
    if (!GwSystem.curTime) {
        winId = mbWinCreate(2, W01_MESSAGE_WH_RESULT, 8);
    } else {
        winId = mbWinCreate(2, W01_MESSAGE_WH_RESULT, 9);
    }
    mbWinWait(winId);
    mbObjMotionShiftSet(modelIdP[2], 3, 0.0f, 4.0f, HU3D_MOTATTR_LOOP);
    for (i = 0; i < 3; i++) {
        if (i != choice) {
            fn_1_B97C(i);
            fn_1_B3F8(i);
        } else {
            fn_1_B9A8(i, 480);
        }
    }
    HuPrcSleep(30);
    type = dataOrder[choice];
    masuId = lbl_1_bss_1230[type];
    GwPlayer[playerNo].masuIdNext = masuId;
    lbl_1_data_644.x = -35.0f;
    lbl_1_data_644.y = lbl_1_data_644.z = 0.0f;
    mbCameraMovePlayer(playerNo, &lbl_1_data_644, NULL, 2400.0f, -1.0f, 60);
    fn_1_B01C(choice, type);
    fn_1_B3F8(choice);
    mbPlayerPosGet(playerNo, &playerPos);
    mbMasuPosGet(masuId, &pos);
    fn_1_AFD4(choice, -1);
    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, 0);
    mbPlayerPosGet(playerNo, &startPos);
    rotY = mbPlayerRotYGet(playerNo);
    PSVECSubtract(&pos, &startPos, &delta2);
    for (k = 0; k <= 30; k++) {
        weight3 = (float)k / 30.0f;
        if ((u32)k == 24) {
            mbPlayerMotionShiftSet(playerNo, 5, 0.0f, 2.0f, 0);
        }
        pos2.x = startPos.x + (weight3 * delta2.x);
        pos2.y = startPos.y + (weight3 * delta2.y)
            + (100.0 * sin((M_PI * (180.0f * weight3)) / 180.0));
        pos2.z = startPos.z + (weight3 * delta2.z);
        mbPlayerPosSetV(playerNo, &pos2);
        weight2 = (float)k / 6.0f;
        if (weight2 > 1.0f) {
            weight2 = 1.0f;
        }
        mbPlayerRotYSet(playerNo, mbAngleLerp(rotY, 0.0f, weight2));
        time = 30 - k;
        mbPlayerWorkGet(playerNo)->_unk08 = time;
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 2.0f, HU3D_MOTATTR_LOOP);
    omVibrate(playerNo, 20, 4, 4);
    HuPrcSleep(30);
    mbPlayerMotionShiftSet(playerNo, 12, 0.0f, 5.0f, 0);
    mbPlayerWinLoseVoicePlay(playerNo, 12, 579);
    mbPlayerMotionEndWait(playerNo);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
    mbPlayerMotionKill(playerNo, motionId);
    work = lbl_1_bss_1240.work;
    for (i = 0; i < 3; i++, work++) {
        mbObjDispSet(work->modelId, FALSE);
        Hu3DModelAttrSet(lbl_1_bss_1220, 1);
    }
    GwPlayer[playerNo].masuId = masuId;
    mbObjAttrSet(modelIdP[0], W01_OBJ_ATTR_EFFECT_PAUSED);
    mbObjMotionTimeSet(modelIdP[0], 0.0f);
    mbObjDispSet(modelIdP[2], FALSE);
}

// The special-space event lets the player choose one of three rides with a ten-second countdown.
// Left/right input moves between them; confirmation or timeout keeps the settled choice.
// A choice movement finishes before timeout is checked, and its rotation wait pauses the countdown.
s32 fn_1_A82C(s32 playerNo)
{
    HuVecF pos;
    float startX;
    float rotY;
    float weight;
    float rotY2;
    float x;
    float endX;
    s32 delay;
    s16 frame;
    s32 state;
    s32 choice;
    s32 choiceNew;
    s32 time;
    GAMEMESID mesId;
    s8 padNo;
    u16 btnDown;
    s16 frameMax;

    mbPlayerPosGet(playerNo, &pos);
    delay = (s32)(60.0f + (60.0f * frandf()));
    state = 0;
    choice = choiceNew = 1;
    padNo = GwPlayer[playerNo].padNo;
    mesId = GameMesTimerCreate(10);
    HuSprGrpDrawNoSet(GameMesGet(mesId)->grpId[0], 32);
    time = 600;
    while (TRUE) {
        if (state == 0) {
            // CPU input changes only on sampled frames; before the first sample, btnDown is unset.
            if (GwPlayer[playerNo].comF != FALSE) {
                if (delay == 0) {
                    btnDown = PAD_BUTTON_A;
                } else if (frandf() < 0.05f) {
                    if (frandf() < 0.5f) {
                        btnDown = PAD_BUTTON_LEFT;
                    } else {
                        btnDown = PAD_BUTTON_RIGHT;
                    }
                }
            } else {
                btnDown = HuPadBtnDown[padNo];
                if (mbPadStkXGet(padNo) < -20) {
                    btnDown |= PAD_BUTTON_LEFT;
                } else if (mbPadStkXGet(padNo) > 20) {
                    btnDown |= PAD_BUTTON_RIGHT;
                }
            }
            if (time < 0 || (btnDown & PAD_BUTTON_A)) {
                break;
            }
            if (choice > 0 && (btnDown & PAD_BUTTON_LEFT)) {
                choiceNew--;
            }
            if (choice < 2 && (btnDown & PAD_BUTTON_RIGHT)) {
                choiceNew++;
            }
            if (choice != choiceNew) {
                state = 1;
                frame = 0;
                frameMax = 30;
                if (choice < choiceNew) {
                    rotY = 90.0f;
                } else {
                    rotY = -90.0f;
                }
                rotY2 = rotY;
                mbPlayerRotYSet(playerNo, rotY2);
                mbPlayerMotionShiftSet(playerNo, 3, 0.0f, 4.0f,
                    HU3D_MOTATTR_LOOP);
                startX = pos.x + (100.0f * (5.0f * (choice - 1)));
                endX = pos.x + (100.0f * (5.0f * (choiceNew - 1)));
            }
        } else {
            weight = (float)frame++ / (float)frameMax;
            x = startX + (weight * (endX - startX));
            mbPlayerPosSet(playerNo, x, pos.y, pos.z);
            if (frame > frameMax) {
                choice = choiceNew;
                mbPlayerRotateStart(playerNo, 180, 15);
                while (mbPlayerRotateCheck(playerNo) == FALSE) {
                    HuPrcVSleep();
                }
                state = 0;
            }
        }
        if (delay != 0) {
            delay--;
        }
        if (time >= 0) {
            time--;
            if (time >= 0) {
                GameMesTimerValueSet(mesId, (time + 59) / 60);
            } else {
                GameMesTimerEnd(mesId);
                mesId = -1;
            }
        }
        HuPrcVSleep();
    }
    mbAudFXPlay(1);
    if (mesId >= 0) {
        GameMesTimerEnd(mesId);
        mesId = -1;
    }
    return choice;
}

// Each special-space object's update applies fading and vertical motion, carrying its rider.
void fn_1_ACDC(OMOBJ *object)
{
    W01_WH_EFFECT_WORK *work;
    HuVecF pos;
    float acceleration;
    float bounce;
    float deltaY;
    float alpha;

    work = &lbl_1_bss_1240.work[object->work[0]];
    switch (work->fadeState) {
    case 1:
        alpha = (float)work->time++ / (float)work->maxTime;
        mbObjAlphaSet(work->modelId,
            (s32)(255.0f * (1.0f - alpha)));
        if (work->time > work->maxTime) {
            work->fadeState = 0;
        }
    }

    if (work->pathMotionF != 0) {
        pos.x = work->pos.x;
        pos.y = work->pos.y - 1000.0f;
        pos.z = work->pos.z;
    } else {
        bounce = 5.0f * (1.0f + mbSinDeg(object->work[1])) + 5.0f;
        object->work[1] += 2;
        if (work->playerNo >= 0) {
            bounce += 50.0f;
        }

        mbObjPosGet(work->modelId, &pos);
        pos.x = work->pos.x;
        pos.z = work->pos.z;

        acceleration = -980.0f;
        deltaY = work->pos.y - pos.y;
        acceleration += 500.0f * (deltaY - 1000.0f) / bounce;
        work->bobVelocity.y += (1.0f / 60.0f) * acceleration;
        pos.y += (1.0f / 60.0f) * work->bobVelocity.y;
        work->bobVelocity.y *= 0.9f;
    }

    if (work->playerNo >= 0) {
        mbPlayerPosSetV(work->playerNo, &pos);
    }
    mbObjPosSetV(work->modelId, &pos);

    pos.y += 356.0f;
    fn_1_BB44(work, &pos);
}

// The special-space event attaches or detaches its rider; attaching also gives a downward impulse.
void fn_1_AFD4(s32 index, s32 state)
{
    W01_WH_EFFECT_WORK *work = &lbl_1_bss_1240.work[index];

    if (state >= 0) {
        work->bobVelocity.y += -833.333374f;
    }
    work->playerNo = state;
}

// The special-space event moves its chosen model and rider along the selected eight-second path.
void fn_1_B01C(s32 index, s32 type)
{
    W01_WH_EFFECT_WORK *work;
    HuVecF pos;
    HuVecF objPos;
    void *freeData;
    void *data;
    void *data2;
    HuVecF *source;
    HuVecF *points;
    s32 size;
    s32 i;
    s32 j;
    s32 count;
    s32 k;
    float scale;
    float step;
    float weight;
    W01_WH_PARTICLE *particle;

    work = &lbl_1_bss_1240.work[index];
    mbObjPosGet(work->modelId, &objPos);
    size = lbl_1_data_5C4[type] * sizeof(HuVecF);
    data = HuMemDirectMallocNum(HEAP_HEAP, size, HU_MEMNUM_OVL);
    source = data;
    memcpy(source, lbl_1_data_5B4[type], size);
    source[0] = objPos;
    work->pathMotionF = 1;
    work->pos.x = objPos.x;
    work->pos.y = 1000.0f + objPos.y;
    work->pos.z = objPos.z;
    scale = 1.0f / 50400.0f;
    step = 120.0f * scale;
    for (i = 0; (u32)i < 480; i++) {
        if ((u32)i < 120) {
            weight = (0.5f * scale) * (float)(i * i);
        } else {
            weight = (120.0f * (120.0f * (0.5f * scale)))
                + (step * ((float)i - 120.0f));
        }
        count = lbl_1_data_5C4[type];
        data2 = HuMemDirectMallocNum(HEAP_HEAP,
            count * sizeof(HuVecF), HU_MEMNUM_OVL);
        points = data2;
        memcpy(points, source, count * sizeof(HuVecF));
        for (j = 1; j < count; j++) {
            for (k = 0; k < count - j; k++) {
                points[k].x = points[k].x
                    + (weight * (points[k + 1].x - points[k].x));
                points[k].y = points[k].y
                    + (weight * (points[k + 1].y - points[k].y));
                points[k].z = points[k].z
                    + (weight * (points[k + 1].z - points[k].z));
            }
        }
        pos = points[0];
        freeData = points;
        HuMemDirectFree(freeData);
        work->pos.x = pos.x;
        work->pos.y = 1000.0f + pos.y;
        work->pos.z = pos.z;
        HuPrcVSleep();
    }
    work->bobVelocity.x = work->bobVelocity.y = work->bobVelocity.z = 0.0f;
    work->pos.y += 100.0f;
    work->pathMotionF = 0;
    particle = work->particle;
    for (i = 0; i < work->particleCount; i++, particle++) {
        if (particle->delay < 0) {
            particle->delay = 1;
        }
    }
    HuMemDirectFree(source);
}

// The special-space event starts a thirty-frame fade of the selected model.
void fn_1_B3F8(s32 index)
{
    W01_WH_EFFECT_WORK *work = &lbl_1_bss_1240.work[index];

    work->fadeState = 1;
    work->time = 0;
    work->maxTime = 30;
}

// Special-space setup creates particles from a subdivided polyhedron and initializes their motion.
s32 fn_1_B434(W01_WH_PARTICLE **particleP)
{
    HuVecF pos;
    Mtx matrix;
    HuVecF vertices[1024];
    W01_WH_PARTICLE *particle;
    s32 triangleCount;
    s32 particleCount;
    s32 i;

    triangleCount = fn_1_D1D0(vertices, 1.0f);
    for (i = 0; i < 2; i++) {
        triangleCount = fn_1_CCEC(vertices, triangleCount, 1.0f, vertices);
    }
    particleCount = fn_1_D320(vertices, triangleCount * 3, vertices);
    particle = mbMalloc(particleCount * sizeof(W01_WH_PARTICLE));
    *particleP = particle;

    mbMtxRot(matrix, 45.0f, 45.0f, 0.0f);
    for (i = 0; i < particleCount; i++, particle++) {
        particle->pos.x = particle->pos.y = particle->pos.z = 0.0f;
        particle->velocity.x = particle->velocity.y = particle->velocity.z = 0.0f;

        PSMTXMultVec(matrix, &vertices[i], &pos);
        particle->rot.x = HuAtan(pos.y,
            sqrtf((pos.z * pos.z) + (pos.x * pos.x))) - 90.0f;
        particle->rot.y = HuAtan(-pos.x, -pos.z);
        particle->rot.z = 0.0f;
        particle->baseRot = particle->rot;

        particle->acceleration.x = 0.5f + (0.5f * frandf());
        particle->acceleration.y = 0.5f + (0.5f * frandf());
        particle->acceleration.z = 0.5f + (0.5f * frandf());
        particle->delay = mbRandMod(60) + 1;
        particle->phase = 0.017453292f * particle->rot.x;
        particle->phaseVelocity = 0.0f;
    }
    return particleCount;
}

// The special-space event resets its particles, opacity, and rider before presenting the choices.
void fn_1_B7D8(s32 index)
{
    W01_WH_EFFECT_WORK *work;
    W01_WH_PARTICLE *particle;
    s32 i;

    work = &lbl_1_bss_1240.work[index];
    particle = work->particle;
    for (i = 0; i < work->particleCount; i++, particle++) {
        particle->pos.x = particle->pos.y = particle->pos.z = 0.0f;
        particle->velocity.x = particle->velocity.y = particle->velocity.z
            = 0.0f;
        particle->rot = particle->baseRot;
        particle->acceleration.x = 0.8f + (0.2f * frandf());
        particle->acceleration.y = 0.8f + (0.2f * frandf());
        particle->acceleration.z = 0.8f + (0.2f * frandf());
        particle->delay = mbRandMod(60) + 1;
        particle->phase = 0.017453292f * particle->rot.x;
        particle->phaseVelocity = 0.0f;
    }
    work->state = 1;
    work->playerNo = -1;
    work->bobVelocity.x = work->bobVelocity.y = work->bobVelocity.z = 0.0f;
    work->pathMotionF = 0;
    mbObjAlphaSet(work->modelId, 255);
}

// The special-space event releases an unchosen model's particles before fading it.
void fn_1_B97C(s32 index)
{
    W01_WH_EFFECT_WORK *work = &lbl_1_bss_1240.work[index];

    work->state = 0;
}

// Schedule 60 percent of the chosen ride's particles across duration, after a 60-frame delay.
// Some scheduled releases can outlast the path; the unscheduled particles are released at its end.
void fn_1_B9A8(s32 index, s32 duration)
{
    W01_WH_EFFECT_WORK *work;
    s32 value;
    s32 allocationSize;
    s32 *allocation;
    s32 *indices;
    s32 *order;
    W01_WH_PARTICLE *particle;
    s32 activeCount;
    s32 first;
    s32 second;
    s32 i;

    work = &lbl_1_bss_1240.work[index];
    allocationSize = work->particleCount * sizeof(s32);
    allocation = HuMemDirectMallocNum(
        HEAP_HEAP, allocationSize, HU_MEMNUM_OVL);
    indices = allocation;
    order = indices;
    activeCount = 0.6f * work->particleCount;
    for (i = 0; i < work->particleCount; i++) {
        order[i] = i;
    }
    for (i = 0; i < 100; i++) {
        first = mbRandMod(work->particleCount);
        second = mbRandMod(work->particleCount);
        value = order[first];
        order[first] = order[second];
        order[second] = value;
    }
    for (i = 0; i < activeCount; i++) {
        particle = &work->particle[order[i]];
        particle->delay = ((i * duration) / activeCount) + 60;
    }
    for (; i < work->particleCount; i++) {
        particle = &work->particle[order[i]];
        particle->delay = -1;
    }
    HuMemDirectFree(order);
    work->state = 0;
}

// Each ride object's update keeps waiting particles attached, then applies wind and gravity.
void fn_1_BB44(W01_WH_EFFECT_WORK *work, const HuVecF *origin)
{
    Mtx rotation;
    HuVecF offset;
    HuVecF rotated;
    HuVecF force;
    W01_WH_PARTICLE *particle;
    float spring;
    float scale;
    s32 i;

    particle = work->particle;
    offset.x = 0.0f;
    offset.y = 70.0f;
    offset.z = 0.0f;
    if (work->state != 0) {
        for (i = 0; i < work->particleCount; i++, particle++) {
            mbMtxRot(rotation, particle->rot.x, particle->rot.y,
                particle->rot.z);
            PSMTXMultVec(rotation, &offset, &rotated);
            PSVECAdd(origin, &rotated, &particle->pos);
        }
        return;
    }

    for (i = 0; i < work->particleCount; i++, particle++) {
        if (particle->delay != 0) {
            mbMtxRot(rotation, particle->rot.x, particle->rot.y,
                particle->rot.z);
            PSMTXMultVec(rotation, &offset, &rotated);
            PSVECAdd(origin, &rotated, &particle->pos);
            if (particle->delay > 0) {
                particle->delay--;
            }
            continue;
        }

        spring = (-980.0f * mbSinRad(particle->phase)) / 70.0f;
        particle->phaseVelocity += (1.0f / 60.0f) * spring;
        particle->phase += (1.0f / 60.0f) * particle->phaseVelocity;
        particle->rot.x = 57.29578f * particle->phase;

        force.x = 0.0f;
        force.y = -1.0f;
        force.z = 0.0f;
        mbMtxRot(rotation, particle->rot.x, particle->rot.y,
            particle->rot.z);
        PSMTXMultVec(rotation, &force, &force);
        scale = 70.0f
            * (particle->phaseVelocity * particle->phaseVelocity);
        PSVECScale(&force, &force, scale);

        particle->velocity.x +=
            (((1.0f / 60.0f) * lbl_1_bss_1224.x)
                * particle->acceleration.x)
            + ((1.0f / 60.0f)
                * (force.x - (10.0f * particle->velocity.x)));
        particle->velocity.y +=
            (((1.0f / 60.0f) * lbl_1_bss_1224.y)
                * particle->acceleration.y)
            + ((1.0f / 60.0f)
                * ((-980.0f + force.y)
                    - (10.0f * particle->velocity.y)));
        particle->velocity.z +=
            (((1.0f / 60.0f) * lbl_1_bss_1224.z)
                * particle->acceleration.z)
            + ((1.0f / 60.0f)
                * (force.z - (10.0f * particle->velocity.z)));

        particle->pos.x += (1.0f / 60.0f) * particle->velocity.x;
        particle->pos.y += (1.0f / 60.0f) * particle->velocity.y;
        particle->pos.z += (1.0f / 60.0f) * particle->velocity.z;
        particle->phaseVelocity *= 0.99f;
        particle->velocity.x *= 0.95f;
        particle->velocity.y *= 0.95f;
        particle->velocity.z *= 0.95f;
    }
}

// The special-space render hook draws all ride particles and depth-sorts their glow quads.
// The first ride model's visibility controls the entire group of particle drawings.
void fn_1_BFA0(HU3D_MODEL *model, Mtx *input)
{
    Mtx matrix;
    W01_SORT_ENTRY sort[1024];
    W01_WH_EFFECT_WORK *work;
    s32 count;
    s32 workIndex;

    work = lbl_1_bss_1240.work;
    if (!mbObjDispGet(work->modelId)) {
        return;
    }

    HuSprTexLoad(lbl_1_bss_121C, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP,
        GX_LINEAR);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
        GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX,
        GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
        GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
        GX_CC_RASC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_8_8);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZCompLoc(GX_FALSE);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetCullMode(GX_CULL_BACK);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
        GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    count = 0;
    for (workIndex = 0; workIndex < 3; workIndex++, work++) {
        W01_WH_PARTICLE *particle;
        s32 particleIndex;

        particle = work->particle;
        for (particleIndex = 0; particleIndex < work->particleCount;
             particleIndex++, particle++) {
            mbMtxRot(matrix, particle->rot.x, particle->rot.y,
                particle->rot.z);
            mtxTransCat(matrix, particle->pos.x, particle->pos.y,
                particle->pos.z);
            PSMTXConcat(*input, matrix, matrix);
            sort[count].pos.x = matrix[0][3];
            sort[count].pos.y = matrix[1][3];
            sort[count].pos.z = matrix[2][3];
            sort[count].workIndex = workIndex;
            sort[count].facing = matrix[2][1];
            count++;
            GXLoadPosMtxImm(matrix, GX_PNMTX0);
            GXCallDisplayList(lbl_1_bss_1214[0], lbl_1_bss_120C[0]);
        }
    }

    fn_1_D50C(sort, 0, count - 1);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
        GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0,
        GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    if (!GwSystem.curTime) {
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
            GX_LO_NOOP);
    } else {
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE,
            GX_LO_NOOP);
    }

    for (workIndex = 0; workIndex < count; workIndex++) {
        GXColor color;

        if (!GwSystem.curTime) {
            float facing;

            color = lbl_1_data_65C[sort[workIndex].workIndex];
            facing = fabs(sort[workIndex].facing);
            color.r = 255.0f * (1.0f - facing) + color.r * facing;
            color.g = 255.0f * (1.0f - facing) + color.g * facing;
            color.b = 255.0f * (1.0f - facing) + color.b * facing;
        } else {
            color = lbl_1_data_668[sort[workIndex].workIndex];
        }
        GXSetTevColor(GX_TEVREG0, color);
        PSMTXTrans(matrix, sort[workIndex].pos.x, sort[workIndex].pos.y,
            sort[workIndex].pos.z);
        GXLoadPosMtxImm(matrix, GX_PNMTX0);
        GXCallDisplayList(lbl_1_bss_1214[1], lbl_1_bss_120C[1]);
    }
}

static const HuVecF lbl_1_rodata_288[11] = {
    { 0.0f, -100.0f, 0.0f },
    { 0.0f, -60.0f, 0.0f },
    { 5.0f, -80.0f, -4.33012f },
    { 0.0f, -80.0f, 4.33012f },
    { -5.0f, -80.0f, -4.33012f },
    { 1.0f, -65.0f, -0.86602f },
    { 0.0f, -65.0f, 0.86602f },
    { -1.0f, -65.0f, -0.86602f },
    { 1.0f, 0.0f, -0.86602f },
    { 0.0f, 0.0f, 0.86602f },
    { -1.0f, 0.0f, -0.86602f }
};

static const s32 lbl_1_rodata_30C[15] = {
    GX_TRIANGLEFAN, 5, 0, 2, 4, 3, 2,
    GX_TRIANGLEFAN, 5, 1, 4, 2, 3, 4,
    -1
};

static const s32 lbl_1_rodata_348[19] = {
    GX_QUADS, 4, 5, 6, 9, 8,
    GX_QUADS, 4, 6, 7, 10, 9,
    GX_QUADS, 4, 7, 5, 8, 10,
    -1
};

// Ride display-list setup emits the indexed, colored geometry until the -1 terminator.
void fn_1_C694(const s32 *displayData, u32 color)
{
    s32 count;
    s32 i;
    s32 index;

    while (*displayData != -1) {
        count = displayData[1];
        GXBegin(displayData[0], GX_VTXFMT0, count);
        displayData += 2;
        for (i = 0; i < count; i++) {
            index = *displayData++;
            GXPosition3f32(
                0.7f * lbl_1_rodata_288[index].x,
                0.7f * lbl_1_rodata_288[index].y,
                0.7f * lbl_1_rodata_288[index].z);
            GXColor1u32(color);
        }
    }
}

// Special-space setup builds the colored particle geometry and returns its display-list size.
s32 fn_1_C7DC(void **displayList)
{
    u32 colorA;
    u32 colorB;
    void *allocation;
    void *listBuffer;
    void *list;
    u32 size;

    colorA = W01_COLOR_TREE_BARK_BIT_PATTERN;
    colorB = W01_COLOR_TREE_LIGHT_BIT_PATTERN;
    allocation = HuMemDirectMallocNum(HEAP_HEAP, 4096, HU_MEMNUM_OVL);
    listBuffer = allocation;
    DCInvalidateRange(listBuffer, 4096);
    GXBeginDisplayList(listBuffer, 4096);
    fn_1_C694(lbl_1_rodata_30C, colorA);
    fn_1_C694(lbl_1_rodata_348, colorB);
    size = GXEndDisplayList();
    list = HuMemDirectMallocNum(HEAP_HEAP, size, HU_MEMNUM_OVL);
    *displayList = list;
    memcpy(*displayList, listBuffer, size);
    DCFlushRange(*displayList, size);
    HuMemDirectFree(listBuffer);
    return size;
}

// Special-space setup builds the textured glow quad and returns its display-list size.
s32 fn_1_CAA4(void **displayList)
{
    void *buffer;
    void *listBuffer;
    void *list;
    u32 size;

    buffer = HuMemDirectMallocNum(HEAP_HEAP, 4096, HU_MEMNUM_OVL);
    listBuffer = buffer;
    DCInvalidateRange(listBuffer, 4096);
    GXBeginDisplayList(listBuffer, 4096);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-35.0f, 35.0f, 0.0f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(35.0f, 35.0f, 0.0f);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition3f32(35.0f, -35.0f, 0.0f);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(-35.0f, -35.0f, 0.0f);
    GXTexCoord2f32(0.0f, 1.0f);
    size = GXEndDisplayList();
    list = HuMemDirectMallocNum(HEAP_HEAP, size, HU_MEMNUM_OVL);
    *displayList = list;
    memcpy(*displayList, listBuffer, size);
    DCFlushRange(*displayList, size);
    HuMemDirectFree(listBuffer);
    return size;
}

// Ride particle setup splits each triangle into four and projects new vertices to a radius.
// Copying the input first allows the source and destination arrays to be the same.
s32 fn_1_CCEC(const HuVecF *src, s32 triangleCount, float inset,
    HuVecF *dst)
{
    HuVecF edge;
    HuVecF inner[3];
    s32 outputCount;
    HuVecF *copy;
    s32 allocationSize;
    HuVecF *allocation;
    HuVecF *triangle;
    s32 triangleIndex;
    s32 vertexIndex;
    s32 nextIndex;

    outputCount = 0;
    allocationSize = triangleCount * 3 * sizeof(HuVecF);
    allocation = HuMemDirectMallocNum(
        HEAP_HEAP, allocationSize, HU_MEMNUM_OVL);
    triangle = allocation;
    copy = triangle;
    memcpy(copy, src, allocationSize);

    for (triangleIndex = 0; triangleIndex < triangleCount;
         triangleIndex++, triangle += 3) {
        for (vertexIndex = 0; vertexIndex < 3; vertexIndex++) {
            nextIndex = vertexIndex + 1;
            if (nextIndex >= 3) {
                nextIndex = 0;
            }
            PSVECSubtract(
                &triangle[nextIndex], &triangle[vertexIndex], &edge);
            inner[vertexIndex].x =
                (0.5f * edge.x) + triangle[vertexIndex].x;
            inner[vertexIndex].y =
                (0.5f * edge.y) + triangle[vertexIndex].y;
            inner[vertexIndex].z =
                (0.5f * edge.z) + triangle[vertexIndex].z;
            PSVECNormalize(&inner[vertexIndex], &inner[vertexIndex]);
            PSVECScale(&inner[vertexIndex], &inner[vertexIndex], inset);
        }

        dst[outputCount++] = triangle[0];
        dst[outputCount++] = inner[0];
        dst[outputCount++] = inner[2];

        dst[outputCount++] = inner[0];
        dst[outputCount++] = triangle[1];
        dst[outputCount++] = inner[1];

        dst[outputCount++] = inner[2];
        dst[outputCount++] = inner[1];
        dst[outputCount++] = triangle[2];

        dst[outputCount++] = inner[0];
        dst[outputCount++] = inner[1];
        dst[outputCount++] = inner[2];
    }
    HuMemDirectFree(copy);
    return outputCount / 3;
}

// The particle-geometry helper expands a scaled tetrahedron into four indexed triangles.
s32 fn_1_D080(HuVecF *dst, float scale)
{
    s32 i;

    for (i = 0; i < 4; i++, dst += 3) {
        dst[0] = lbl_1_data_674[lbl_1_data_6A4[3 * i]];
        dst[1] = lbl_1_data_674[lbl_1_data_6A4[(3 * i) + 1]];
        dst[2] = lbl_1_data_674[lbl_1_data_6A4[(3 * i) + 2]];
        PSVECScale(&dst[0], &dst[0], scale);
        PSVECScale(&dst[1], &dst[1], scale);
        PSVECScale(&dst[2], &dst[2], scale);
    }
    return 4;
}

// Ride particle setup expands a scaled octahedron into its eight indexed triangles.
s32 fn_1_D1D0(HuVecF *dst, float scale)
{
    s32 i;

    for (i = 0; i < 8; i++, dst += 3) {
        dst[0] = lbl_1_data_6D4[lbl_1_data_71C[(3 * i) + 0]];
        dst[1] = lbl_1_data_6D4[lbl_1_data_71C[(3 * i) + 1]];
        dst[2] = lbl_1_data_6D4[lbl_1_data_71C[(3 * i) + 2]];
        PSVECScale(&dst[0], &dst[0], scale);
        PSVECScale(&dst[1], &dst[1], scale);
        PSVECScale(&dst[2], &dst[2], scale);
    }
    return 8;
}

// Ride particle setup compacts coincident vertices, keeping the first within each tolerance.
s32 fn_1_D320(HuVecF *src, s32 count, HuVecF *dst)
{
    float epsilon = 0.0001f;
    u8 *duplicate;
    HuVecF delta;
    s32 i;
    s32 j;
    s32 result;

    duplicate = mbMalloc(count);
    for (i = 0; i < count - 1; i++) {
        if (duplicate[i] != 0) {
            continue;
        }
        for (j = i + 1; j < count; j++) {
            if (duplicate[j] != 0) {
                continue;
            }
            PSVECSubtract(&src[j], &src[i], &delta);
            if (fabs(delta.x) < epsilon
                && fabs(delta.y) < epsilon
                && fabs(delta.z) < epsilon) {
                duplicate[j] = 1;
            }
        }
    }

    result = 0;
    for (i = 0; i < count; i++) {
        if (duplicate[i] == 0) {
            *dst++ = src[i];
            result++;
        }
    }
    HuMemDirectFree(duplicate);
    return result;
}

// The ride render hook sorts particles by camera-space depth before drawing their glow quads.
void fn_1_D50C(W01_SORT_ENTRY *entry, s32 first, s32 last)
{
    float pivot;
    W01_SORT_ENTRY temp;
    s32 left;
    s32 right;

    pivot = entry[(first + last) / 2].pos.z;
    left = first;
    right = last;
    for (;;) {
        while (entry[left].pos.z < pivot) {
            left++;
        }
        while (pivot < entry[right].pos.z) {
            right--;
        }
        if (left >= right) {
            break;
        }
        temp = entry[left];
        entry[left] = entry[right];
        entry[right] = temp;
        left++;
        right--;
    }
    if (first < left - 1) {
        fn_1_D50C(entry, first, left - 1);
    }
    if (right + 1 < last) {
        fn_1_D50C(entry, right + 1, last);
    }
}

// MB1_Create measures each segment of the launch path so the rider advances by distance.
void fn_1_DEA4(void)
{
    HuVecF a;
    HuVecF b;
    HuVecF c;
    HuVecF d;
    HuVecF slopeB;
    HuVecF slopeA;
    HuVecF *point;
    float lengthSum;
    float step;
    float length;
    float t;
    float finalLength;
    float simpsonLength;
    float result;
    int count;
    u32 i;
    int pointNo;
    int divCount;
    int j;

    lbl_1_bss_1208 = 0.0f;
    for (i = 0; i < 32; i++) {
        pointNo = i;
        if (pointNo > 32) {
            pointNo = 32;
        }
        point = &lbl_1_data_240[pointNo];
        if (pointNo == 0) {
            VECSubtract(&point[1], &point[0], &slopeA);
        } else {
            VECSubtract(&point[1], &point[-1], &slopeA);
        }
        if (pointNo == 31) {
            VECSubtract(&point[1], &point[0], &slopeB);
        } else {
            VECSubtract(&point[2], &point[0], &slopeB);
        }
        VECScale(&slopeA, &slopeA, 0.5f);
        VECScale(&slopeB, &slopeB, 0.5f);
        a = point[0];
        b = point[1];
        c = slopeA;
        d = slopeB;
        count = 10;
        t = 0.0f;
        simpsonLength = 0.0f;
        step = 1.0f - t;
        length = 0.5f * (step
            * (((W01CurveEval)(u32)fn_1_14BF0)(&a, &b, &c, &d, t)
                + ((W01CurveEval)(u32)fn_1_14BF0)(&a, &b, &c, &d, 1.0)));
        for (divCount = 1; divCount <= count; divCount *= 2) {
            lengthSum = 0.0f;
            for (j = 1; j <= divCount; j++) {
                lengthSum += ((W01CurveEval)(u32)fn_1_14BF0)(&a, &b, &c, &d,
                    t + (step * ((float)j - 0.5f)));
            }
            lengthSum *= step;
            simpsonLength = (1.0f / 3.0f) * (length + (2.0f * lengthSum));
            step *= 0.5f;
            length = 0.5f * (length + lengthSum);
        }
        result = simpsonLength;
        finalLength = result;
        lbl_1_bss_1188[i] = finalLength;
        lbl_1_bss_1208 += finalLength;
    }
}

// The launch-path solver estimates distance to a parameter using ten trapezoid intervals.
static inline float W01HermiteIntegrate(W01CurveEval eval,
    HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, float t)
{
    int i;
    int div;
    float sampleLength;
    float baseT;
    float sampleT;
    float deltaT;

    div = 10;
    baseT = 0.0f;
    deltaT = (t - baseT) / div;
    sampleT = baseT;
    sampleLength = 0.0f;
    for (i = 0; i < div - 1; i++) {
        sampleT += deltaT;
        sampleLength += eval(a, b, c, d, sampleT);
    }
    sampleLength = deltaT * 0.5
        * (eval(a, b, c, d, baseT) + eval(a, b, c, d, t)
            + (2.0 * sampleLength));
    return sampleLength;
}

// The launch movement estimates the curve parameter for its traveled distance with bounded steps.
// Sampled speeds below 0.1 in magnitude use 1.0 as the correction divisor.
static inline float W01CurveNewton(W01CurveEval eval, HuVecF *a,
    HuVecF *b, HuVecF *c, HuVecF *d, float t, float distance, int maxStep)
{
    int step;
    float sampleLength;
    float pathLength;
    float oldT;
    float minLength;

    minLength = 0.1f;
    step = 0;
    do {
        pathLength = W01HermiteIntegrate(eval, a, b, c, d, t) - distance;
        if (fabs(sampleLength = eval(a, b, c, d, t)) < minLength) {
            sampleLength = 1.0f;
        }
        oldT = t;
        t -= pathLength / sampleLength;
        step++;
    } while (t != oldT && step < maxStep);
    return t;
}

// After the launch path, hop the rider onto the destination space and return to idle.
static inline void W01PlayerJumpFinish(int playerNo, HuVecF *dstPos,
    float dstRotY, float jumpHeight, int maxTime)
{
    HuVecF srcPos;
    HuVecF pos;
    HuVecF delta;
    int time;
    int timeRemaining;
    float srcRotY;
    float t;
    float rotT;

    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    mbPlayerPosGet(playerNo, &srcPos);
    srcRotY = mbPlayerRotYGet(playerNo);
    VECSubtract(dstPos, &srcPos, &delta);
    for (time = 0; time <= maxTime; time++) {
        t = time / (float)maxTime;
        if ((u32)time == maxTime - 6) {
            mbPlayerMotionShiftSet(playerNo, 5, 0.0f, 2.0f,
                HU3D_MOTATTR_NONE);
        }
        pos.x = srcPos.x + (t * delta.x);
        pos.y = srcPos.y + (t * delta.y)
            + (jumpHeight * HuSin(180.0f * t));
        pos.z = srcPos.z + (t * delta.z);
        mbPlayerPosSetV(playerNo, &pos);
        rotT = time / 6.0f;
        if (rotT > 1.0f) {
            rotT = 1.0f;
        }
        mbPlayerRotYSet(playerNo, mbAngleLerp(srcRotY, dstRotY, rotT));
        timeRemaining = maxTime - time;
        mbPlayerWorkGet(playerNo)->_unk08 = timeRemaining;
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 2.0f, HU3D_MOTATTR_LOOP);
}

// The space-move completion callback installs this movement hook to carry a player
// along the launch path.
void fn_1_E234(int playerNo)
{
    HuVecF a;
    HuVecF b;
    HuVecF pos;
    HuVecF masuPos;
    HuVecF endPlayerPos;
    HuVecF playerPos;
    HuVecF c;
    HuVecF d;
    HuVecF delta;
    HuVecF rot;
    HuVecF preSlopeB;
    HuVecF preSlopeA;
    HuVecF slopeB;
    HuVecF slopeA;
    HuVecF *initialPoint;
    HuVecF *pathPoint;
    s16 masuId;
    u32 time;
    s16 linkMasuId;
    s16 motionId;
    s32 segmentNo;
    s32 initialPointNo;
    s32 pathPointNo;
    float t;
    float distance;
    float speed;
    float angle;

    segmentNo = 0;
    omVibrate(playerNo, 20, 7, 3);
    GwPlayer[playerNo].moveF = TRUE;
    mbPlayerWorkGet(playerNo)->_unk08 = 100;
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    motionId = mbPlayerMotionCreate(playerNo, W01_PLAYER_MOTION_PATH_RIDE);
    mbPlayerMotionShiftSet(playerNo, motionId, 0.0f, 8.0f, FALSE);
    masuId = GwPlayer[playerNo].masuId;
    linkMasuId = mbMasuLinkGet(masuId, 0);
    GwPlayer[playerNo].masuIdNext = linkMasuId;

    {
        initialPointNo = 0;
        if (initialPointNo > 32) {
            initialPointNo = 32;
        }
        initialPoint = &lbl_1_data_240[initialPointNo];
        if (initialPointNo == 0) {
            PSVECSubtract(&initialPoint[1], &initialPoint[0], &preSlopeA);
        } else {
            PSVECSubtract(&initialPoint[1], &initialPoint[-1], &preSlopeA);
        }
        if (initialPointNo == 31) {
            PSVECSubtract(&initialPoint[1], &initialPoint[0], &preSlopeB);
        } else {
            PSVECSubtract(&initialPoint[2], &initialPoint[0], &preSlopeB);
        }
        PSVECScale(&preSlopeA, &preSlopeA, 0.5f);
        PSVECScale(&preSlopeB, &preSlopeB, 0.5f);
        a = initialPoint[0];
        b = initialPoint[1];
        c = preSlopeA;
        d = preSlopeB;
    }

    t = 0.0f;
    distance = 0.0f;
    speed = lbl_1_bss_1208 / 108.0f;
    for (time = 0; time < 108; time++) {
        pathPointNo = segmentNo;
        if (pathPointNo > 32) {
            pathPointNo = 32;
        }
        pathPoint = &lbl_1_data_240[pathPointNo];
        if (pathPointNo == 0) {
            PSVECSubtract(&pathPoint[1], &pathPoint[0], &slopeA);
        } else {
            PSVECSubtract(&pathPoint[1], &pathPoint[-1], &slopeA);
        }
        if (pathPointNo == 31) {
            PSVECSubtract(&pathPoint[1], &pathPoint[0], &slopeB);
        } else {
            PSVECSubtract(&pathPoint[2], &pathPoint[0], &slopeB);
        }
        PSVECScale(&slopeA, &slopeA, 0.5f);
        PSVECScale(&slopeB, &slopeB, 0.5f);
        a = pathPoint[0];
        b = pathPoint[1];
        c = slopeA;
        d = slopeB;

        t = W01CurveNewton((W01CurveEval)(u32)fn_1_14BF0,
            &a, &b, &c, &d, t, distance, 10);
        mbHermiteCalcV(&a, &b, &c, &d, &pos, t);
        mbPlayerPosGet(playerNo, &playerPos);
        PSVECSubtract(&pos, &playerPos, &delta);
        rot.x = -HuAtan(delta.y,
            sqrtf((delta.z * delta.z) + (delta.x * delta.x)));
        rot.y = HuAtan(delta.x, delta.z);
        rot.z = 0.0f;
        mbPlayerRotSetV(playerNo, &rot);
        mbPlayerPosSetV(playerNo, &pos);
        distance += speed;
        if (distance >= lbl_1_bss_1188[segmentNo]) {
            distance -= lbl_1_bss_1188[segmentNo];
            segmentNo++;
            t -= 1.0f;
        }
        HuPrcVSleep();
    }

    GwPlayer[playerNo].masuId = linkMasuId;
    mbPlayerPosGet(playerNo, &endPlayerPos);
    mbMasuPosGet(linkMasuId, &masuPos);
    PSVECSubtract(&masuPos, &endPlayerPos, &delta);
    angle = HuAtan(delta.x, delta.z);
    mbPlayerRotSet(playerNo, 0.0f, angle, 0.0f);
    W01PlayerJumpFinish(playerNo, &masuPos, angle, 100.0f, 18);
    mbPlayerMotionKill(playerNo, motionId);
    GwPlayer[playerNo].moveF = FALSE;
    mbPlayerColSnapPlayerSet(playerNo, TRUE);
}

// MB1_Create calls this once to create the board's Biri group and its five members.
void fn_1_ED00(void)
{
    HuVecF pos;
    Mtx matrix;
    Mtx rot;
    Mtx trans;
    W01_BIRI_WORK *work;
    W01_BIRI_ENTRY *entry;
    BOOL dayF;
    BOOL entryDayF;
    s32 dataDir;
    s32 entryDataDir;
    float angle;
    float radius;
    s32 i;

    work = &lbl_1_bss_EFC;
    entry = work->entry;
    dayF = !GwSystem.curTime;
    if (dayF != FALSE) {
        dataDir = W01_DATA_DIR_DAY;
    } else {
        dataDir = W01_DATA_DIR_NIGHT;
    }
    work->modelId = mbObjCreate(DATANUM(dataDir, 41), NULL, FALSE);
    work->pos.x = 1871.0f;
    work->pos.y = 2479.0f;
    work->pos.z = 1078.0f;
    work->rot.x = -54.0f;
    work->rot.y = -4.0f;
    work->rot.z = 0.0f;
    work->baseRot = work->rot;
    work->phase = work->phaseVelocity = 0.0f;
    PSMTXTrans(trans, work->pos.x, work->pos.y, work->pos.z);
    mbMtxRot(rot, work->rot.x, work->rot.y, work->rot.z);
    PSMTXConcat(trans, rot, matrix);
    for (i = 0; i < 5; i++, entry++) {
        memset(entry, 0, sizeof(W01_BIRI_ENTRY));
        entryDayF = !GwSystem.curTime;
        if (entryDayF != FALSE) {
            entryDataDir = W01_DATA_DIR_DAY;
        } else {
            entryDataDir = W01_DATA_DIR_NIGHT;
        }
        entry->modelId = mbObjCreate(DATANUM(entryDataDir, 42), NULL, TRUE);
        mbObjBiriQCreate((MBMODELID)entry->modelId);
        mbObjAttrSet(entry->modelId, HU3D_MOTATTR_LOOP);
        angle = 360.0f * frandf();
        entry->phaseAngle = angle;
        radius = 100.0f * (1.6f * frandf());
        pos.x = radius * mbCosDeg(angle);
        pos.y = -420.0f;
        pos.z = radius * mbSinDeg(angle);
        PSMTXMultVec(matrix, &pos, &entry->homePos);
        entry->index = i;
        entry->bobAmplitude = 60.000003814697266f + (100.0f * frandf());
        entry->spinAngle = 360.0f * frandf();
        entry->spinSpeed = 8.0f * ((2.0f * frandf()) - 1.0f);
        entry->orbitRadius = entry->targetOrbitRadius = 80.0f
            + (100.0f * (0.5f * frandf()));
        entry->maxTime = entry->idleDuration = (s16)(30.0f
            + (12.0f * frandf()));
        entry->timer = mbRandMod(entry->maxTime);
        entry->work = work;
    }
}

// Biri cleanup calls mbObjBiriQKill five times on the same group scenery model and ignores its
// result;
// setup installs BiriQ hooks on the five member models instead.
void fn_1_F06C(void)
{
    s32 i;
    W01_BIRI_WORK *work;
    W01_BIRI_ENTRY *entry;

    work = &lbl_1_bss_EFC;
    entry = work->entry;
    for (i = 0; i < 5; i++, entry++) {
        mbObjBiriQKill((MBMODELID)work->modelId);
    }
}

// MB1_Create's board update object calls this each frame to swing the Biri group
// and update each member's movement.
void fn_1_F0D0(void)
{
    W01_BIRI_WORK *work;
    s32 i;
    W01_BIRI_ENTRY *entry;
    float spring;

    work = &lbl_1_bss_EFC;
    spring = (-100000.0f * mbSinRad(work->phase)) / 500.0f;
    work->phaseVelocity += (1.0f / 60.0f) * spring;
    work->phase += (1.0f / 60.0f) * work->phaseVelocity;
    work->rot.y = work->baseRot.y + (57.29578f * work->phase);
    work->phaseVelocity *= 0.9f;
    mbObjPosSetV(work->modelId, &work->pos);
    mbObjRotSetV(work->modelId, &work->rot);
    entry = work->entry;
    for (i = 0; i < 5; i++, entry++) {
        fn_1_F66C(entry);
    }
}

// The question-mark-space Biri event surrounds the player and removes coins on a hit.
void fn_1_F1F4(s32 playerNo, s16 id)
{
    s16 motionId[2];
    s8 padNo;
    W01_BIRI_WORK *work;
    W01_BIRI_ENTRY *entry;
    s32 audNo;
    s32 coinCount;
    s32 coinRemain;
    s32 count;
    s32 i;
    float weight;
    float rotY;
    HuVecF coinPos;
    HuVecF playerPos;

    work = &lbl_1_bss_EFC;
    entry = work->entry;
    motionId[0] = mbPlayerMotionCreate(playerNo, W01_PLAYER_MOTION_BIRI_TOSS);
    motionId[1] = mbPlayerMotionCreate(playerNo, W01_PLAYER_MOTION_BIRI_REACTION);
    mbCameraPlayerViewSet(playerNo, 1);
    mbPlayerMotionShiftSet(playerNo, motionId[0], 0.0f, 4.0f, 0);
    work->state = 1;
    audNo = mbAudFXPlay(0x595);
    HuPrcSleep(30);
    mbPlayerMotionShiftSet(playerNo, 9, 0.0f, 4.0f, 0);
    rotY = mbPlayerRotYGet(playerNo);
    for (i = 0; (u32)i <= 12; i++) {
        weight = (float)i / 12.0f;
        mbPlayerRotYSet(playerNo, mbAngleEaseOut(rotY, 0.0f, weight));
        HuPrcVSleep();
    }
    while (!mbPlayerMotionEndCheck(playerNo)) {
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, motionId[1], 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    work->state = 2;
    padNo = GwPlayer[playerNo].padNo;
    lbl_1_bss_EF8 = -1;
    coinCount = mbPlayerCoinGet(playerNo);
    coinRemain = coinCount;
    mbPlayerPosGet(playerNo, &playerPos);
    CharModelVoiceFlagSet(GwPlayer[playerNo].charNo, FALSE);
    HuPrcSleep(60);
    count = mbRandMod(4) + 1;
    fn_1_10998(count);
    while (lbl_1_bss_EF8 < 0) {
        HuPrcVSleep();
    }
    fn_1_10900();
    mbPlayerMotionShiftSet(playerNo, 15, 0.0f, 4.0f, HU3D_MOTATTR_LOOP);
    omVibrate(playerNo, 20, 7, 3);
    CharFXPlay(GwPlayer[playerNo].charNo, 0x240);
    coinPos.x = playerPos.x;
    coinPos.y = playerPos.y + 100.0f;
    coinPos.z = playerPos.z;
    if (coinRemain > 0) {
        if (coinRemain >= count) {
            fn_1_13D8C(&coinPos, count);
        } else {
            fn_1_13D8C(&coinPos, coinRemain);
        }
        coinRemain -= count;
    }
    for (i = 0; (u32)i < 30; i++) {
        weight = (float)i / 30.0f;
        coinPos.y = playerPos.y
            + (100.0f * (2.0f * mbSinDeg(180.0f * weight)));
        mbPlayerPosSet(playerNo, playerPos.x, coinPos.y, playerPos.z);
        HuPrcVSleep();
    }
    work->state = 0;
    mbAudFXStop(audNo);
    mbPlayerMotIdleSet(playerNo);
    HuPrcSleep(30);
    CharModelVoiceFlagSet(GwPlayer[playerNo].charNo, TRUE);
    mbPlayerMotionShiftSet(playerNo, 13, 0.0f, 4.0f, 0);
    mbCoinAddProcExec(playerNo, -(coinCount - coinRemain), -1, TRUE);
    for (i = 0; i < 2; i++) {
        mbPlayerMotionKill(playerNo, motionId[i]);
    }
}

// Day and night board scenery models loaded by MB1_Create.
u32 lbl_1_data_3CC[6] = {
    W01_MODEL_BOARD_SCENERY_DAY_1, W01_MODEL_BOARD_SCENERY_DAY_2,
    W01_MODEL_BOARD_SCENERY_DAY_3, W01_MODEL_BOARD_SCENERY_DAY_4,
    W01_MODEL_BOARD_SCENERY_DAY_5, 0,
};

u32 lbl_1_data_3E4[4] = {
    W01_MODEL_BOARD_SCENERY_DAY_1, W01_MODEL_BOARD_SCENERY_DAY_2,
    W01_MODEL_BOARD_SCENERY_NIGHT_3, 0,
};

u32 *lbl_1_data_3F4[2] = {
    lbl_1_data_3CC, lbl_1_data_3E4,
};

HuVecF lbl_1_data_3FC = { 0.0f, 0.0f, 1200.0f };
HuVecF lbl_1_data_408 = { -28.0f, 0.0f, 0.0f };
HuVecF lbl_1_data_414 = { 0.0f, 0.0f, 0.0f };
char lbl_1_data_420[12] = "b01_m080";

HuVecF lbl_1_data_42C[2][2] = {
    {
        { -50.0f, 15.0f, 0.0f },
        { -20.0f, 0.0f, 0.0f },
    },
    {
        { -50.0f, -15.0f, 0.0f },
        { -20.0f, 0.0f, 0.0f },
    },
};

HuVecF lbl_1_data_45C[2][2] = {
    {
        { 0.0f, 0.0f, 0.0f },
        { -20.0f, 0.0f, 0.0f },
    },
    {
        { 0.0f, 0.0f, 0.0f },
        { -20.0f, 0.0f, 0.0f },
    },
};

HuVecF lbl_1_data_48C[7] = {
    { -4414.0f, 2900.0f, 1070.0f },
    { -4244.0f, 2900.0f, 670.0f },
    { -3804.0f, 2900.0f, 600.0f },
    { 3526.0f, 3900.0f, 720.0f },
    { 3946.0f, 3550.0f, 560.0f },
    { 4346.0f, 3200.0f, 720.0f },
    { 4566.0f, 2850.0f, 1110.0f },
};

HuVecF lbl_1_data_4E0[1] = {
    { -1314.0f, 3120.0f, 1251.0f },
};

int lbl_1_data_4EC[2] = { W01_MODEL_COIN_FIELD_DAY, -1 };
int lbl_1_data_4F4[2] = { W01_MODEL_COIN_FIELD_NIGHT, -1 };
int *lbl_1_data_4FC[2] = { lbl_1_data_4EC, lbl_1_data_4F4 };
char lbl_1_data_504[9] = "b01_m210";
char lbl_1_data_50D[11] = "b01_m210n";

HuVecF lbl_1_data_518 = { -19.0f, 0.0f, 0.0f };
HuVecF lbl_1_data_524 = { 0.0f, 400.0f, 0.0f };
s32 lbl_1_data_530[4] = { 6, 8, 10, 12 };
s32 lbl_1_data_540[4] = { 5, 4, 3, 2 };
u32 lbl_1_data_550[3] = {
    W01_MODEL_MASU_EFFECT_K_H1,
    W01_MODEL_MASU_EFFECT_K_H2,
    W01_MODEL_MASU_EFFECT_K_H3,
};
char lbl_1_data_55C[5] = "k_h1";
char lbl_1_data_561[5] = "k_h2";
char lbl_1_data_566[6] = "k_h3";
char *lbl_1_data_56C[3] = {
    lbl_1_data_55C, lbl_1_data_561, lbl_1_data_566,
};
u32 lbl_1_data_578[2] = { W01_MODEL_MASU_SF_DAY, W01_MODEL_MASU_MF_DAY };
u32 lbl_1_data_580[2] = { W01_MODEL_MASU_SF_NIGHT, W01_MODEL_MASU_MF_NIGHT };
u32 lbl_1_data_588[2][3] = {
    { W01_MASU_ATTR_SPECIAL_A, W01_MASU_ATTR_SPECIAL_A_LINK_0,
        W01_MASU_ATTR_SPECIAL_A_LINK_1 },
    { W01_MASU_ATTR_SPECIAL_B, W01_MASU_ATTR_SPECIAL_B_LINK_0,
        W01_MASU_ATTR_SPECIAL_B_LINK_1 },
};
char lbl_1_data_5A0[5] = "sf_h";
char lbl_1_data_5A5[7] = "mf_h";
char *lbl_1_data_5AC[2] = { lbl_1_data_5A0, lbl_1_data_5A5 };
HuVecF *lbl_1_data_5B4[4] = {
    lbl_1_data_0, lbl_1_data_90, lbl_1_data_120, lbl_1_data_1B0,
};
s32 lbl_1_data_5C4[4] = { 12, 12, 12, 12 };
int lbl_1_data_5D4[4] = {
    W01_MODEL_LIGHT_DETAIL_DAY_1, W01_MODEL_LIGHT_DETAIL_DAY_1,
    W01_MODEL_LIGHT_DETAIL_DAY_2, -1,
};
int lbl_1_data_5E4[4] = {
    W01_MODEL_LIGHT_DETAIL_NIGHT_1, W01_MODEL_LIGHT_DETAIL_NIGHT_1,
    W01_MODEL_LIGHT_DETAIL_NIGHT_2, -1,
};
u32 lbl_1_data_5F4[3] = {
    W01_MODEL_WH_EFFECT_DAY_1, W01_MODEL_WH_EFFECT_DAY_2,
    W01_MODEL_WH_EFFECT_DAY_3,
};
char lbl_1_data_600[5] = "w_h1";
char lbl_1_data_605[5] = "w_h2";
char lbl_1_data_60A[6] = "w_h3";
char *lbl_1_data_610[3] = {
    lbl_1_data_600, lbl_1_data_605, lbl_1_data_60A,
};
HuVecF lbl_1_data_61C = { -10000.0f, 10000.0f, -10000.0f };
HuVecF lbl_1_data_628 = { 1.0f, -1.0f, -1.0f };
GXColor lbl_1_data_634 = { 255, 255, 255, 255 };
char lbl_1_data_638[12] = "b01_m200";
HuVecF lbl_1_data_644 = { -20.0f, 0.0f, 0.0f };
HuVecF lbl_1_data_650 = { 0.0f, 300.0f, 0.0f };
GXColor lbl_1_data_65C[3] = {
    { 255, 211, 229, 255 },
    { 191, 229, 255, 255 },
    { 255, 243, 155, 255 },
};
GXColor lbl_1_data_668[3] = {
    { 128, 32, 32, 255 },
    { 32, 32, 128, 255 },
    { 128, 128, 32, 255 },
};
HuVecF lbl_1_data_674[4] = {
    { 0.0f, 0.0f, 0.612372458f },
    { 0.577350259f, 0.0f, -0.204124138f },
    { -0.288675129f, 0.5f, -0.204124138f },
    { -0.288675129f, -0.5f, -0.204124138f },
};
s32 lbl_1_data_6A4[12] = {
    0, 2, 1, 0, 3, 2, 0, 1, 3, 1, 2, 3,
};
HuVecF lbl_1_data_6D4[6] = {
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, -1.0f, 0.0f },
    { -1.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, -1.0f },
    { 1.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f },
};
s32 lbl_1_data_71C[25] = {
    0, 5, 2, 0, 4, 5, 0, 3, 4, 0, 2, 3, 1,
    2, 5, 1, 5, 4, 1, 4, 3, 1, 3, 2, -1,
};
HuVecF lbl_1_data_780 = { 0.0f, 0.0f, 20.0f };

// The Biri approach calculation clamps inverse-sine inputs to the valid endpoint angles.
static inline float fn_1_F66CAsin(float ratio)
{
    if (ratio >= 1.0f) {
        return 1.5707964f;
    }
    if (ratio <= -1.0f) {
        return -1.5707964f;
    }
    return asinf(ratio);
}

// The Biri group update advances one member's idle, circling, attack, or return movement.
void fn_1_F66C(W01_BIRI_ENTRY *entry)
{
    W01_BIRI_WORK *work;
    HuVecF pos;
    HuVecF playerPos;
    HuVecF delta;
    HuVecF rot;
    HuVecF move;
    HuVecF move2;
    Mtx matrix;
    s32 playerNo;
    float time;
    float distance;
    float angle;
    float angle2;

    playerNo = GwSystem.turnPlayerNo;
    work = entry->work;
    switch (entry->state) {
        case 0: {
            float pitch;
            float ratio;
            float rotY;

            if (work->state == 1) {
                entry->state = 1;
                mbPlayerPosGet(playerNo, &playerPos);
                PSVECSubtract(&playerPos, &entry->pos, &delta);
                angle = (atan2(delta.x, delta.z) / M_PI) * 180.0f;
                distance = PSVECMag(&delta);
                if (distance < entry->orbitRadius) {
                    ratio = entry->orbitRadius
                        / sqrtf((entry->orbitRadius * entry->orbitRadius)
                            + (distance * distance));
                    if (ratio >= 1.0f) {
                        pitch = 1.5707964f;
                    } else if (ratio <= -1.0f) {
                        pitch = -1.5707964f;
                    } else {
                        pitch = asinf(ratio);
                    }
                    rotY = 1.5707964f - pitch;
                } else if (distance != 0.0f) {
                    rotY = 1.5707964f
                        - fn_1_F66CAsin(entry->orbitRadius / distance);
                } else {
                    rotY = 0.0f;
                }
                if (frand() & 1) {
                    entry->orbitAngle = angle + (57.29578f * rotY);
                    entry->orbitDirection = 1.0f;
                } else {
                    entry->orbitAngle = angle - (57.29578f * rotY);
                    entry->orbitDirection = -1.0f;
                }
                entry->targetY = 150.0f + playerPos.y
                    - (100.0f * (1.3f * frandf()));
                entry->orbitRadiusTransition = 0.0f;
            } else if (entry->delayTimer != 0) {
                entry->delayTimer--;
            } else if (entry->timer > entry->maxTime) {
                entry->timer = 0;
                entry->delayTimer = (s16)(30.0f + (30.0f * frandf()));
                work->phaseVelocity += 0.5f * mbCosDeg(entry->phaseAngle);
            } else {
                time = (float)entry->timer++ / (float)entry->maxTime;
                angle = 360.0f * time;
                pos.x = 0.5f * (entry->bobAmplitude * mbSinDeg(angle));
                pos.y = entry->bobAmplitude * (mbCosDeg(angle) - 1.0f);
                pos.z = 0.0f;
                mbMtxRotAxisDeg(matrix, 'y', entry->spinAngle);
                PSMTXMultVec(matrix, &pos, &pos);
                entry->spinAngle += entry->spinSpeed;
                mbMtxRot(matrix, work->rot.x, work->rot.y, work->rot.z);
                mtxTransCat(matrix, entry->homePos.x, entry->homePos.y,
                    entry->homePos.z);
                PSMTXMultVec(matrix, &pos, &pos);
                PSVECSubtract(&pos, &entry->pos, &delta);
                entry->pos = pos;
                entry->rot.x = (atan2(-delta.y,
                    sqrtf((delta.z * delta.z) + (delta.x * delta.x)))
                    / M_PI) * 180.0f;
                entry->rot.y = (atan2(delta.x, delta.z) / M_PI) * 180.0f;
            }
            break;
        }

        case 1: {
            if (entry->orbitRadius != entry->targetOrbitRadius ||
                entry->orbitRadiusTransition != 0.0f) {
                entry->orbitRadiusTransition -= 20.0f;
                entry->orbitRadius += (1.0f / 60.0f) * entry->orbitRadiusTransition;
                if (entry->orbitRadius < entry->targetOrbitRadius &&
                    entry->orbitRadiusTransition < 0.0f) {
                    entry->orbitAngle += entry->orbitDirection
                        * ((360.0f * (entry->targetOrbitRadius - entry->orbitRadius))
                            / (6.2831855f * entry->targetOrbitRadius));
                    entry->orbitRadiusTransition = 0.0f;
                    entry->orbitRadius = entry->targetOrbitRadius;
                }
            }
            mbPlayerPosGet(playerNo, &playerPos);
            pos.x = playerPos.x - (entry->orbitRadius * mbSinDeg(entry->orbitAngle));
            pos.y = entry->targetY;
            pos.z = playerPos.z - (entry->orbitRadius * mbCosDeg(entry->orbitAngle));
            PSVECSubtract(&pos, &entry->pos, &delta);
            rot.x = (atan2(-delta.y,
                sqrtf((delta.z * delta.z) + (delta.x * delta.x))) / M_PI)
                * 180.0f;
            rot.y = (atan2(delta.x, delta.z) / M_PI) * 180.0f;
            distance = PSVECMag(&delta);
            entry->orbitAngle += (360.0f * entry->orbitDirection)
                * (10.0f / (6.2831855f * distance));
            angle = 360.0f * (100.0f / (6.2831855f * distance));
            if (angle > 20.0f) {
                angle = 20.0f;
            }
            mbAngleAdd(&entry->rot.x, rot.x, angle);
            mbAngleAdd(&entry->rot.y, rot.y, angle);
            mbMtxRot(matrix, entry->rot.x, entry->rot.y, entry->rot.z);
            PSMTXMultVec(matrix, &lbl_1_data_780, &delta);
            PSVECAdd(&entry->pos, &delta, &entry->pos);
            if (work->state == 0) {
                entry->state = 6;
                entry->timer = 0;
                entry->maxTime = 180;
            }
            break;
        }

        case 2: {
            time = (float)entry->timer++ / (float)entry->maxTime;
            PSVECSubtract(&entry->targetPos, &entry->startPos, &move);
            PSVECScale(&move, &move, time);
            PSVECAdd(&entry->startPos, &move, &entry->pos);
            mbPlayerPosGet(playerNo, &playerPos);
            PSVECSubtract(&entry->pos, &playerPos, &delta);
            angle2 = (atan2(-delta.x, -delta.z) / M_PI) * 180.0f;
            entry->rot.x = -80.0f * (1.0f - mbCosDeg(90.0f * time));
            entry->rot.y = mbAngleLerp(entry->rot.y, angle2, time);
            if (entry->timer > entry->maxTime) {
                entry->state = 3;
                entry->timer = 0;
                entry->maxTime = 30;
            }
            break;
        }

        case 3:
            time = (float)entry->timer++ / (float)entry->maxTime;
            if (entry->timer > entry->maxTime) {
                entry->state = 4;
                entry->timer = 0;
                entry->maxTime = 9;
                entry->startPos = entry->pos;
                mbPlayerPosGet(playerNo, &playerPos);
                PSVECSubtract(&entry->pos, &playerPos, &delta);
                angle = (atan2(delta.z, delta.x) / M_PI) * 180.0f;
                entry->targetPos.x = playerPos.x + (50.0f * mbCosDeg(angle));
                entry->targetPos.y = entry->targetY;
                entry->targetPos.z = playerPos.z + (50.0f * mbSinDeg(angle));
            }
            break;

        case 4:
            time = (float)entry->timer++ / (float)entry->maxTime;
            PSVECSubtract(&entry->targetPos, &entry->startPos, &move2);
            PSVECScale(&move2, &move2, time);
            PSVECAdd(&entry->startPos, &move2, &entry->pos);
            entry->rot.x = -80.0f
                + (-40.0f * (1.0f - mbCosDeg(90.0f * time)));
            if (entry->timer > entry->maxTime) {
                entry->state = 5;
                lbl_1_bss_EF8 = entry->index;
                entry->timer = 0;
                entry->maxTime = 6;
            }
            break;

        case 5:
            time = (float)entry->timer++ / (float)entry->maxTime;
            if (entry->timer > entry->maxTime) {
                entry->state = 1;
                entry->orbitRadiusTransition = 600.0f;
            }
            break;

        case 6: {
            PSVECSubtract(&entry->homePos, &entry->pos, &delta);
            rot.x = (atan2(-delta.y,
                sqrtf((delta.z * delta.z) + (delta.x * delta.x))) / M_PI)
                * 180.0f;
            rot.y = (atan2(delta.x, delta.z) / M_PI) * 180.0f;
            distance = PSVECMag(&delta);
            if ((distance < 50.0f) || (++entry->timer > entry->maxTime)) {
                entry->state = 0;
                entry->pos = entry->homePos;
                entry->timer = 0;
                entry->maxTime = entry->idleDuration;
            } else {
                angle = 360.0f * (100.0f / (6.2831855f * distance));
                if (angle > 20.0f) {
                    angle = 20.0f;
                }
                mbAngleAdd(&entry->rot.x, rot.x, angle);
                mbAngleAdd(&entry->rot.y, rot.y, angle);
                mbMtxRot(matrix, entry->rot.x, entry->rot.y, entry->rot.z);
                PSMTXMultVec(matrix, &lbl_1_data_780, &delta);
                PSVECAdd(&entry->pos, &delta, &entry->pos);
            }
            break;
        }
    }
    mbObjPosSetV(entry->modelId, &entry->pos);
    mbObjRotSetV(entry->modelId, &entry->rot);
}

// The Biri state query reports when every member is idle or already returning home.
BOOL fn_1_1089C(void)
{
    W01_BIRI_ENTRY *entry = lbl_1_bss_EFC.entry;
    s32 i;

    for (i = 0; i < 5; i++, entry++) {
        if (entry->state != 0 && entry->state != 6) {
            return FALSE;
        }
    }
    return TRUE;
}

// After the first Biri hit, cancel remaining approaches and ease them back into circling.
void fn_1_10900(void)
{
    W01_BIRI_ENTRY *entry = lbl_1_bss_EFC.entry;
    // This color is initialized but unused by the Biri movement loop.
    GXColor color = { 0, 0, 0, 255 };
    s32 i;

    for (i = 0; i < 5; i++, entry++) {
        if (entry->state >= 1 && entry->state <= 4) {
            entry->state = 1;
            entry->orbitRadiusTransition = 600.0f;
        }
    }
}

// The Biri event starts a timed approach for the first count members before their hit.
void fn_1_10998(s32 count)
{
    W01_BIRI_ENTRY *entry = lbl_1_bss_EFC.entry;
    // This color is initialized but unused by the approach setup loop.
    GXColor color = { 0, 0, 0, 255 };
    s32 i;

    for (i = 0; i < count; i++, entry++) {
        entry->state = 2;
        entry->timer = 0;
        entry->maxTime = 12;
        entry->startPos = entry->pos;
        entry->targetPos.x = entry->startPos.x
            + (200.0f * mbSinDeg(entry->rot.y));
        entry->targetPos.y = entry->startPos.y + 50.0f;
        entry->targetPos.z = entry->startPos.z
            + (200.0f * mbCosDeg(entry->rot.y));
    }
}

// MB1_Create calls this to load the event's particle models and day/night space data.
void fn_1_10AA8(void)
{
    W01_PARTICLE_WORK *work = &lbl_1_bss_EE8;
    int i;
    int layer;

    for (i = 0; i < 5; i++) {
        work->modelId[i] = mbObjCreate(
            (lbl_1_data_7A8[i] & 0xFFFF)
            | (MBTimeDayGet() ? W01_DATA_DAY : W01_DATA_NIGHT),
            NULL, FALSE);
        if (i != 0) {
            mbObjDispSet(work->modelId[i], FALSE);
        }
    }
    work->modelId[5] = mbObjCreate(
        DATANUM((MBTimeDayGet() ? W01_DATA_DIR_DAY : W01_DATA_DIR_NIGHT), 48),
        NULL, FALSE);
    mbObjDispSet(work->modelId[5], FALSE);
    layer = mbMasuLayerGet();
    mbMasuLayerSet(1);
    mbMasuDataRead((MBTimeDayGet() ? W01_DATA_DIR_DAY : W01_DATA_DIR_NIGHT)
        | 1);
    mbMasuLayerSet(layer);
    lbl_1_bss_EE4 = HuSprAnimDataRead(
        DATANUM((MBTimeDayGet() ? W01_DATA_DIR_DAY : W01_DATA_DIR_NIGHT), 49));
    HuSprAnimLock(lbl_1_bss_EE4);
}

// MB1_Kill releases the button-mashing event's particle animation when the board closes.
void fn_1_10C68(void)
{
    if (lbl_1_bss_EE4) {
        HuSprAnimKill(lbl_1_bss_EE4);
        lbl_1_bss_EE4 = NULL;
    }
}

// The question-mark-space callback runs the five-second button-mashing event and may move players.
void fn_1_10CB8(s32 playerNo, s16 id)
{
    W01_PARTICLE_WORK *work;
    s32 validPlayerCount;
    s32 idleTime;
    s32 transitionF;
    s32 motionTime;
    s32 firstCountdown;
    s32 secondCountdown;
    s16 firstMasuId;
    s16 secondMasuId;
    s16 helpWinNo;
    s32 inputF;
    s8 padNo;
    Mtx hookMtx;
    Mtx itemMtx;
    HuVecF initialPos;
    HuVecF firstTarget;
    HuVecF secondTarget;
    HuVecF direction;
    HuVecF firstDelta;
    HuVecF firstPos;
    HuVecF firstStart;
    HuVecF secondDelta;
    HuVecF secondPos;
    HuVecF secondStart;
    s16 motionId;
    GAMEMESID gameMesId;
    s16 winNo;
    s32 soundId;
    u16 buttons;
    s32 mashCount;
    s32 timer;
    s32 soundTime;
    s32 i;
    s32 firstTime;
    s32 secondTime;
    float motionSpeed;
    float itemWeight;
    float targetAngle;
    float firstAngleWeight;
    float secondAngleWeight;
    float firstStartAngle;
    float secondStartAngle;
    float firstWeight;
    float secondWeight;

    soundTime = 0;
    inputF = 0;
    soundId = -1;
    work = &lbl_1_bss_EE8;
    motionId = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 100));
    mbPlayerPosGet(playerNo, &initialPos);
    firstMasuId = mbMasuAttrFindLink(id, W01_MASU_ATTR_LINK);
    mbMasuPosGet(firstMasuId, &firstTarget);
    PSVECSubtract(&firstTarget, &initialPos, &direction);
    targetAngle = (float)((atan2(direction.x, direction.z) / M_PI) * 180.0);
    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, 0);
    mbPlayerPosGet(playerNo, &firstStart);
    firstStartAngle = mbPlayerRotYGet(playerNo);
    PSVECSubtract(&firstTarget, &firstStart, &firstDelta);
    for (firstTime = 0; firstTime <= 18; firstTime++) {
        firstWeight = (float)firstTime / 18.0f;
        if ((u32)firstTime == 12) {
            mbPlayerMotionShiftSet(playerNo, 5, 0.0f, 2.0f, 0);
        }
        firstPos.x = firstStart.x + (firstWeight * firstDelta.x);
        firstPos.y = firstStart.y + (firstWeight * firstDelta.y)
            + (100.0 * sin((M_PI * (180.0f * firstWeight)) / 180.0));
        firstPos.z = firstStart.z + (firstWeight * firstDelta.z);
        mbPlayerPosSetV(playerNo, &firstPos);
        firstAngleWeight = (float)firstTime / 6.0f;
        if (firstAngleWeight > 1.0f) {
            firstAngleWeight = 1.0f;
        }
        mbPlayerRotYSet(playerNo,
            mbAngleLerp(firstStartAngle, targetAngle, firstAngleWeight));
        firstCountdown = 18 - firstTime;
        mbPlayerWorkGet(playerNo)->_unk08 = firstCountdown;
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 2.0f, HU3D_MOTATTR_LOOP);

    secondMasuId = mbMasuLinkGet(firstMasuId, 0);
    mbMasuPosGet(secondMasuId, &secondTarget);
    PSVECSubtract(&secondTarget, &firstTarget, &direction);
    targetAngle = (float)((atan2(direction.x, direction.z) / M_PI) * 180.0);
    HuPrcSleep(3);
    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, 0);
    mbPlayerPosGet(playerNo, &secondStart);
    secondStartAngle = mbPlayerRotYGet(playerNo);
    PSVECSubtract(&secondTarget, &secondStart, &secondDelta);
    for (secondTime = 0; secondTime <= 18; secondTime++) {
        secondWeight = (float)secondTime / 18.0f;
        if ((u32)secondTime == 12) {
            mbPlayerMotionShiftSet(playerNo, 5, 0.0f, 2.0f, 0);
        }
        secondPos.x = secondStart.x + (secondWeight * secondDelta.x);
        secondPos.y = (float)(secondStart.y + (secondWeight * secondDelta.y)
            + (100.0 * sin((M_PI * (180.0f * secondWeight)) / 180.0)));
        secondPos.z = secondStart.z + (secondWeight * secondDelta.z);
        mbPlayerPosSetV(playerNo, &secondPos);
        secondAngleWeight = (float)secondTime / 6.0f;
        if (secondAngleWeight > 1.0f) {
            secondAngleWeight = 1.0f;
        }
        mbPlayerRotYSet(playerNo,
            mbAngleLerp(secondStartAngle, targetAngle, secondAngleWeight));
        secondCountdown = 18 - secondTime;
        mbPlayerWorkGet(playerNo)->_unk08 = secondCountdown;
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 2.0f, HU3D_MOTATTR_LOOP);

    mbCameraMovePos(&lbl_1_data_7BC, &lbl_1_data_7C8, &lbl_1_data_7D4,
        4500.0f, -1.0f, 30);
    mbCameraMoveWait();
    HuPrcSleep(12);
    winNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W01, 15), -1);
    mbWinWait(winNo);

    motionTime = (s32)mbPlayerMotionTimeGet(playerNo);
    mbPlayerMotionSet(playerNo, motionId, 0);
    mbPlayerMotionTimeSet(playerNo, 0.0f);
    Hu3DMotionCalc(mbPlayerModelIDGet(playerNo));
    Hu3DModelObjMtxGet(mbObjModelIDGet(mbPlayerObjIDGet(playerNo)),
        CharModelItemHookGet(GwPlayer[playerNo].charNo, 4, 0), hookMtx);
    mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
    mbPlayerMotionTimeSet(playerNo, (float)motionTime);
    Hu3DMotionCalc(mbPlayerModelIDGet(playerNo));
    mbPlayerMotionShiftSet(playerNo, motionId, 0.0f, 30.0f, 0);
    mbObjDispSet(work->modelId[5], TRUE);
    PSMTXCopy(hookMtx, itemMtx);
    for (i = 0; (u32)i < 30; i++) {
        itemWeight = (float)i / 30.0f;
        itemMtx[0][3] = hookMtx[0][3]
            + (100.0f * (10.0f * (1.0f - itemWeight)));
        itemMtx[1][3] = hookMtx[1][3]
            + (100.0f * (5.0f * mbSinDeg(180.0f * itemWeight)));
        mbObjMtxSet(work->modelId[5], &itemMtx);
        HuPrcVSleep();
    }
    PSMTXIdentity(itemMtx);
    mbObjMtxSet(work->modelId[5], &itemMtx);
    mbObjHookSet(mbPlayerObjIDGet(playerNo),
        CharModelItemHookGet(GwPlayer[playerNo].charNo, 4, 0),
        work->modelId[5]);

    helpWinNo = mbWinCreateHelp(MESSNUM(MESS_BOARD_W01, 20));
    mbWinPosSet(helpWinNo, 192, 336);
    padNo = GwPlayer[playerNo].padNo;
    mashCount = 0;
    {
        idleTime = 0;
        motionSpeed = 0.0f;
        mbObjAttrSet(mbPlayerObjIDGet(playerNo), HU3D_MOTATTR_LOOP);
        transitionF = 0;
        gameMesId = GameMesCreate(1, 5, -1, -1);
        HuSprGrpDrawNoSet(GameMesGet(gameMesId)->grpId[0], 32);
        timer = 300;
        for (i = 0; (u32)i <= 300; i++) {
            if (GwPlayer[playerNo].comF) {
                if (frandf() < 0.2f) {
                    buttons = PAD_BUTTON_A;
                } else {
                    buttons = 0;
                }
            } else {
                buttons = HuPadBtnDown[padNo];
            }
            if (buttons & PAD_BUTTON_A) {
                if (mbObjMotionEndCheck(work->modelId[1])) {
                    if (mashCount == 0) {
                        mbObjDispSet(work->modelId[0], FALSE);
                    }
                    mbObjDispSet(work->modelId[1], TRUE);
                    mbObjMotionTimeSet(work->modelId[1], 0.0f);
                    mbObjMotionSpeedSet(work->modelId[1], 1.0f);
                    mbObjMotionShapeTimeSet(work->modelId[1], 0.0f);
                    mbObjMotionShapeSpeedSet(work->modelId[1], 1.0f);
                    transitionF = 1;
                }
                inputF = 1;
                mashCount++;
                idleTime = 0;
                motionSpeed = 1.0f;
            } else {
                if (++idleTime > 8) {
                    motionSpeed -= 0.1f;
                    if (motionSpeed < 0.0f) {
                        motionSpeed = 0.0f;
                    }
                }
            }
            mbPlayerMotionSpeedSet(playerNo, motionSpeed);
            if (timer >= 0) {
                timer--;
                if (timer >= 0) {
                    GameMesDispSet(gameMesId, 1, (s16)((timer + 59) / 60));
                } else {
                    GameMesDispSet(gameMesId, 2, -1);
                }
            }
            if (inputF) {
                if (soundTime == 0) {
                    mbAudFXPlay(0x598);
                    soundTime = 12;
                }
                inputF = 0;
            }
            if (soundTime != 0) {
                soundTime--;
            }
            HuPrcVSleep();
        }
    }

    mbWipeDissolveFadeOut();
    mbWinKill(helpWinNo);
    mbStatusDispForceSetAll(FALSE);
    mbCameraMovePos(NULL, NULL, NULL, 3200.0f, -1.0f, -1);
    mbCameraMoveWait();
    mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
    mbPlayerPosReset(playerNo);
    mbPlayerColSnapPlayerSet(playerNo, TRUE);
    mbObjHookReset(mbPlayerObjIDGet(playerNo));
    mbObjDispSet(work->modelId[5], FALSE);
    for (i = 0; i < 5; i++) {
        mbObjDispSet(work->modelId[i], FALSE);
    }
    mbObjDispSet(work->modelId[2], TRUE);
    mbObjAttrSet(work->modelId[2], (HU3D_MOTATTR_LOOP | HU3D_MOTATTR_SHAPE_LOOP));
    mbObjMotionTimeSet(work->modelId[2], 0.0f);
    mbObjMotionSpeedSet(work->modelId[2], 1.0f);
    mbObjMotionShapeTimeSet(work->modelId[2], 0.0f);
    mbObjMotionShapeSpeedSet(work->modelId[2], 1.0f);
    mbWipeDissolveFadeIn();
    soundId = mbAudFXPlay(0x599);
    winNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W01, 16), -1);
    mbWinWait(winNo);

    mbMasuLayerSet(1);
    validPlayerCount = 0;
    for (i = 0; i < 4; i++) {
        id = GwPlayer[i].masuId;
        if (mbMasuLinkNumGet(id) != 0) {
            validPlayerCount++;
        }
    }
    mbMasuLayerSet(0);
    if (validPlayerCount != 0 && mashCount > 5 && frandf() < 0.8f) {
        mbObjDispSet(work->modelId[2], FALSE);
        mbObjDispSet(work->modelId[3], TRUE);
        mbObjAttrSet(work->modelId[3], 0);
        mbObjMotionTimeSet(work->modelId[3], 0.0f);
        mbObjMotionSpeedSet(work->modelId[3], 1.0f);
        mbObjMotionShapeTimeSet(work->modelId[3], 0.0f);
        mbObjMotionShapeSpeedSet(work->modelId[3], 1.0f);
        mbCameraShakeSet(30, 500.0f);
        mbAudFXStop(soundId);
        mbAudFXPlay(0x59A);
        for (i = 0; i < 4; i++) {
            omVibrate((s16)i, 20, 20, 0);
        }
        winNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W01, 17), -1);
        HuPrcSleep(17);
        fn_1_11C14(&lbl_1_data_7E0);
        mbWinWait(winNo);
        while (work->particleModelId >= 0) {
            HuPrcVSleep();
        }
        mbWipeDissolveFadeOut();
        mbObjDispSet(work->modelId[3], FALSE);
        mbObjDispSet(work->modelId[0], TRUE);
        fn_1_12090();
        mbCameraMovePos(&lbl_1_data_7BC, &lbl_1_data_7C8, &lbl_1_data_7D4,
            4500.0f, -1.0f, -1);
        mbWipeDissolveFadeIn();
        winNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W01, 19), -1);
        mbWinWait(winNo);
    } else {
        mbObjAttrReset(work->modelId[2], (HU3D_MOTATTR_LOOP | HU3D_MOTATTR_SHAPE_LOOP));
        while (!mbObjMotionEndCheck(work->modelId[2])) {
            HuPrcVSleep();
        }
        mbAudFXStop(soundId);
        mbObjDispSet(work->modelId[2], FALSE);
        mbObjDispSet(work->modelId[4], TRUE);
        mbObjMotionTimeSet(work->modelId[4], 0.0f);
        mbObjMotionSpeedSet(work->modelId[4], 1.0f);
        mbObjMotionShapeTimeSet(work->modelId[4], 0.0f);
        mbObjMotionShapeSpeedSet(work->modelId[4], 1.0f);
        winNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W01, 18), -1);
        mbWinWait(winNo);
    }
    mbPlayerMotionKill(playerNo, motionId);
}

u32 lbl_1_data_7A8[5] = {
    DATANUM(DATA_w01, 43), DATANUM(DATA_w01, 44), DATANUM(DATA_w01, 45), DATANUM(DATA_w01, 46),
    DATANUM(DATA_w01, 47),
};
HuVecF lbl_1_data_7BC = { 0.0f, 950.0f, 3000.0f };
HuVecF lbl_1_data_7C8 = { -19.0f, 0.0f, 0.0f };
HuVecF lbl_1_data_7D4 = { 0.0f, 250.0f, 0.0f };
HuVecF lbl_1_data_7E0 = { 0.0f, 900.0f, 2500.0f };
char lbl_1_data_7EC[12] = "b01_m082";
u32 lbl_1_data_7F8[3] = { DATANUM(DATA_w01, 52), DATANUM(DATA_w01, 53), DATANUM(DATA_w01, 54) };

// A successful button-mashing event starts a burst of one hundred particles at the supplied
// position.
void fn_1_11C14(HuVecF *pos)
{
    W01_PARTICLE_WORK *work = &lbl_1_bss_EE8;

    work->particleModelId = mbParticleCreate(lbl_1_bss_EE4, 100);
    Hu3DModelLayerSet(work->particleModelId, 5);
    mbParticleHookSet(work->particleModelId, fn_1_11C84);
    Hu3DModelPosSetV(work->particleModelId, pos);
}

// The event particle hook initializes the burst, applies gravity and fading, then removes it.
void fn_1_11C84(HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx mtx)
{
    GXColor color = { 255, 255, 192, 192 };
    W01_PARTICLE_WORK *work = &lbl_1_bss_EE8;
    MBPARTICLEDATA *data;
    HuVecF dir;
    float angleY;
    float angleX;
    float time;
    float speed;
    int count;
    int i;

    if (particleP->mode == 0) {
        data = particleP->data;
        for (i = 0; i < particleP->num; i++, data++) {
            angleY = (45.0f * frandf()) - 22.5f;
            angleX = (30.0f * frandf()) - 10.0f;
            dir.x = mbSinDeg(angleY) * mbCosDeg(angleX);
            dir.y = mbSinDeg(angleX);
            dir.z = mbCosDeg(angleY) * mbCosDeg(angleX);
            data->pos.x = 100.0f * (15.0f * dir.x);
            data->pos.y = 100.0f * (2.0f * dir.y);
            data->pos.z = (100.0f * (4.0f * frandf())) - 200.0f;
            speed = 4000.0f + (4000.0f * frandf());
            PSVECScale(&dir, &data->vel, speed);
            data->scale = 30.0f + (20.0f * frandf());
            data->color = color;
            data->time = 0;
            data->activeF = 60.0f + (30.0f * frandf());
        }
        particleP->mode = 1;
    }
    count = 0;
    data = particleP->data;
    for (i = 0; i < particleP->num; i++, data++) {
        if (data->time <= data->activeF) {
            time = (float)data->time++ / (float)data->activeF;
            data->pos.x += (1.0f / 60.0f) * data->vel.x;
            data->pos.y += (1.0f / 60.0f) * data->vel.y;
            data->pos.z += (1.0f / 60.0f) * data->vel.z;
            data->vel.y += -16.333334f;
            data->color.a = 192.0f * (1.0f - time);
            count++;
        }
    }
    if (count == 0) {
        mbParticleKill(particleP->modelId);
        work->particleModelId = -1;
    }
}

// After the successful event, move each eligible player to the space linked in the alternate layer.
void fn_1_12090(void)
{
    s16 motionId[2];
    s16 nextMasuId[4];
    HuVecF startPos;
    HuVecF targetPos;
    HuVecF pos;
    HuVecF delta;
    s16 masuId;
    int time;
    int playerNo;

    mbMasuLayerSet(1);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        masuId = GwPlayer[playerNo].masuId;
        if (mbMasuLinkNumGet(masuId) != 0) {
            nextMasuId[playerNo] = mbMasuLinkGet(masuId, 0);
        } else {
            nextMasuId[playerNo] = 0;
        }
    }
    mbMasuLayerSet(0);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (nextMasuId[playerNo] != 0) {
            motionId[0] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 18));
            motionId[1] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 19));
            mbCameraPlayerViewSetFast(playerNo, 2);
            mbCameraShakeSet(30, 500.0f);
            mbWipeDissolveFadeIn();
            mbPlayerMotionShiftSet(playerNo, motionId[0], 0.0f, 4.0f, 0);
            mbMasuPosGet(GwPlayer[playerNo].masuId, &startPos);
            mbMasuPosGet(nextMasuId[playerNo], &targetPos);
            PSVECSubtract(&targetPos, &startPos, &delta);
            mbPlayerRotYSet(
                playerNo, (float)((atan2(delta.x, delta.z) / M_PI) * 180.0));
            GwPlayer[playerNo].masuIdNext = nextMasuId[playerNo];
            HuPrcVSleep();
            mbPlayerColSnapPlayerSet(playerNo, FALSE);
            for (time = 0; time <= 48U; time++) {
                float weight = (float)time / 48.0f;

                pos.x = startPos.x + (weight * (targetPos.x - startPos.x));
                pos.y = startPos.y + (weight * (targetPos.y - startPos.y))
                    + (100.0f * (7.5f * mbSinDeg(180.0f * weight)));
                pos.z = startPos.z + (weight * (targetPos.z - startPos.z));
                mbPlayerPosSetV(playerNo, &pos);
                HuPrcVSleep();
            }
            CharFXPlay(GwPlayer[playerNo].charNo, 0xC3);
            omVibrate(playerNo, 20, 7, 3);
            mbPlayerMotionShiftSet(playerNo, motionId[1], 0.0f, 4.0f, 0);
            HuPrcSleep(30);
            mbWipeDissolveFadeOut();
            mbPlayerRotYSet(playerNo, 0.0f);
            mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
            mbPlayerColSnapPlayerSet(playerNo, TRUE);
            mbPlayerMotionKill(playerNo, motionId[0]);
            mbPlayerMotionKill(playerNo, motionId[1]);
            GwPlayer[playerNo].masuId = nextMasuId[playerNo];
        }
    }
}

// The turn-start hook restores the button-mashing event's idle model and hides its outcome models.
void fn_1_12454(void)
{
    W01_PARTICLE_WORK *work = &lbl_1_bss_EE8;
    s32 i;

    for (i = 0; i < 5; i++) {
        if (i == 0) {
            mbObjDispSet(work->modelId[i], TRUE);
        } else {
            mbObjDispSet(work->modelId[i], FALSE);
        }
    }
}

// MB1_Create builds the gate models and stores the gate and linked-space positions.
void fn_1_124C8(void)
{
    W01_GATE_WORK *work = &lbl_1_bss_EB4;
    s16 linkMasuId;

    work->baseModelId = mbObjCreate(W01_MODEL_GATE_BASE, NULL, FALSE);
    mbObjDispSet(work->baseModelId, FALSE);
    mbObjLayerSet(work->baseModelId, 3);
    mbObjScaleSet(work->baseModelId, 2.0f, 2.0f, 2.0f);
    work->hookModelId = mbObjCreate(
        DATANUM((MBTimeDayGet() ? W01_DATA_DIR_DAY : W01_DATA_DIR_NIGHT), 24),
        NULL, FALSE);
    mbObjAttrSet(work->hookModelId, W01_OBJ_ATTR_EFFECT_PAUSED);
    work->hookChildModelId = mbObjCreate(
        DATANUM((MBTimeDayGet() ? W01_DATA_DIR_DAY : W01_DATA_DIR_NIGHT), 25),
        NULL, FALSE);
    mbObjAttrSet(work->hookChildModelId, HU3D_MOTATTR_LOOP);
    mbObjHookSet(work->hookModelId, lbl_1_data_7EC, work->hookChildModelId);
    work->masuId = mbMasuFind_MAttrIdGet(-1, W01_MASU_ATTR_MOVE_HOOK_STAR);
    mbMasuPosGet(work->masuId, &work->masuPos);
    linkMasuId = mbMasuAttrFindLink(work->masuId, W01_MASU_ATTR_LINK);
    mbMasuPosGet(linkMasuId, &work->linkPos);
    PSVECSubtract(&work->masuPos, &work->linkPos, &work->direction);
    work->rotY = (float)((atan2(work->direction.x, work->direction.z) / M_PI)
        * 180.0);
    mbObjRotSet(work->baseModelId, 0.0f, work->rotY, 0.0f);
    mbObjRotSet(work->hookModelId, 0.0f, work->rotY, 0.0f);
    mbObjPosSetV(work->hookModelId, &work->linkPos);
    HuDataDirClose(W01_DATA_DIR_LIGHT_EFFECT);
}

// fn_1_760 calls this for the gate space; the daytime branch shows its restriction.
// At night, animate the gate effect model and turn the player before invoking Teresa
// at the gate space.
void fn_1_126EC(s32 playerNo, s16 id)
{
    W01_GATE_WORK *work = &lbl_1_bss_EB4;
    HuVecF pos;
    HuVecF target;
    float time;
    float weight;
    float rotY;
    s16 winNo;
    MBMODELID hookModelId;
    u32 i;

    if (MBTimeDayGet()) {
        mbPlayerMotIdleSet(playerNo);
        winNo = mbWinCreate(2, MESSNUM(MESS_BOARD_W01, 21), -1);
        mbWinWait(winNo);
        return;
    }

    mbMoveNumDispSet(playerNo, FALSE);
    rotY = mbPlayerRotYGet(playerNo);
    mbObjDispSet(work->baseModelId, TRUE);
    mbev_CapTeresaFadeCreate(work->baseModelId);
    hookModelId = work->hookModelId;
    mbObjAttrReset(hookModelId, W01_OBJ_ATTR_EFFECT_PAUSED);
    omVibrate(playerNo, 20, 7, 3);
    target.x = work->linkPos.x + (0.6f * work->direction.x);
    target.y = work->linkPos.y + (0.6f * work->direction.y);
    target.z = work->linkPos.z + (0.6f * work->direction.z);
    for (i = 0; i <= 60; i++) {
        time = (float)(s32)i / 60.0f;
        weight = mbSinDeg(90.0f * time);
        pos.x = work->linkPos.x + (weight * (target.x - work->linkPos.x));
        pos.y = work->linkPos.y + (weight * (target.y - work->linkPos.y))
            + 150.0f;
        pos.z = work->linkPos.z + (weight * (target.z - work->linkPos.z));
        mbObjPosSetV(work->baseModelId, &pos);
        mbev_CapTeresaFadeSet(255.0f * time);
        if (i == 6) {
            mbPlayerMotionShiftSet(playerNo, 9, 0.0f, 4.0f, 0);
        }
        if (i > 6) {
            time = (float)(i - 6) / 12.0f;
            if (time > 1.0f) {
                time = 1.0f;
            }
            mbPlayerRotYSet(playerNo,
                mbAngleEaseOut(rotY, 180.0f + work->rotY, time));
        }
        HuPrcVSleep();
    }
    HuPrcSleep(30);
    mbWipeDissolveFadeOut();
    mbev_CapTeresaFadeKill(work->baseModelId);
    mbObjDispSet(work->baseModelId, FALSE);
    mbev_CapCallTeresa(playerNo, work->masuId);
    mbObjMotionTimeSet(work->hookModelId, 0.0f);
    mbObjAttrSet(work->hookModelId, W01_OBJ_ATTR_EFFECT_PAUSED);
    mbMoveNumDispSet(playerNo, TRUE);
    mbWipeDissolveFadeIn();
}

// MB1_Create initializes the drifting ambient particles and their draw callback.
void fn_1_12A4C(void)
{
    W01_SNOW_PARTICLE *particle;
    s32 i;

    particle = lbl_1_bss_4B0;
    for (i = 0; i < 64; i++, particle++) {
        particle->pos.x = 3000.0f * frandf();
        particle->pos.y = 6000.0f * frandf();
        particle->pos.z = 2000.0f * frandf();
        particle->angle = 6.2831855f * frandf();
        particle->angleSpeed = 0.0f;
        particle->scale = 0.7f + (0.3f * frandf());
        particle->textureNo = mbRandMod(3);
    }
    for (i = 0; i < 3; i++) {
        lbl_1_bss_4A4[i] = HuSprAnimRead(HuDataSelHeapReadNum(
            (lbl_1_data_7F8[i] & 0xFFFF)
                | (MBTimeDayGet() ? W01_DATA_DIR_DAY : W01_DATA_DIR_NIGHT),
            HU_MEMNUM_OVL, HEAP_MODEL));
        HuSprAnimLock(lbl_1_bss_4A4[i]);
    }
    lbl_1_bss_498 = fn_1_130C4(&lbl_1_bss_49C);
    lbl_1_bss_4A0 = Hu3DHookFuncCreate(fn_1_12D58);
    Hu3DModelCameraSet(lbl_1_bss_4A0, 1);
    Hu3DModelLayerSet(lbl_1_bss_4A0, 5);
    lbl_1_bss_48C.x = lbl_1_bss_48C.y = lbl_1_bss_48C.z = 0.0f;
    lbl_1_bss_480.x = 100.0f + (100.0f * (8.0f * frandf()));
    lbl_1_bss_480.y = 0.0f;
    lbl_1_bss_480.z = 0.0f;
    lbl_1_bss_EB0 = TRUE;
}

// MB1_Kill releases the three ambient particle textures when the board closes.
void fn_1_12CF0(void)
{
    s32 i;

    for (i = 0; i < 3; i++) {
        HuSprAnimKill(lbl_1_bss_4A4[i]);
        lbl_1_bss_4A4[i] = NULL;
    }
}

// The model draw hook renders drifting daytime particles, fading those close to the camera.
void fn_1_12D58(HU3D_MODEL *modelP, Mtx *mtx)
{
    W01_SNOW_PARTICLE *particle;
    Mtx modelMtx;
    Mtx rotMtx;
    GXColor color;
    s32 i;

    particle = lbl_1_bss_4B0;
    if (!lbl_1_bss_EB0 || !MBTimeDayGet()) {
        return;
    }
    for (i = 0; i < 3; i++) {
        HuSprTexLoad(lbl_1_bss_4A4[i], 0, i, GX_CLAMP, GX_CLAMP,
            GX_LINEAR);
    }
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
        GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
        GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A0,
        GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG,
        GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_SPOT);
    GXSetZCompLoc(GX_FALSE);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
        GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    for (i = 0; i < 64; i++, particle++) {
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, particle->textureNo,
            GX_COLOR0A0);
        PSMTXScale(modelMtx, particle->scale, particle->scale,
            particle->scale);
        mbMtxRotAxisRad(rotMtx, 'z', particle->angle);
        PSMTXConcat(modelMtx, rotMtx, modelMtx);
        mtxTransCat(modelMtx, particle->pos.x, particle->pos.y,
            particle->pos.z);
        PSMTXConcat(*mtx, modelMtx, modelMtx);
        GXLoadPosMtxImm(modelMtx, GX_PNMTX0);
        if (modelMtx[2][3] <= 0.0f && modelMtx[2][3] > -1000.0f) {
            color.a = 255.0f * (modelMtx[2][3] / -1000.0f);
        } else {
            color.a = 255;
        }
        GXSetTevColor(GX_TEVREG0, color);
        GXCallDisplayList(lbl_1_bss_49C, lbl_1_bss_498);
    }
}

// Ambient particle setup builds the textured quad display list used by its draw hook.
u32 fn_1_130C4(void **displayList)
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
    GXPosition3f32(-25.0f, 0.0f, 25.0f);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition3f32(25.0f, 0.0f, 25.0f);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(25.0f, 0.0f, -25.0f);
    GXTexCoord2f32(0.0f, 1.0f);
    GXPosition3f32(-25.0f, 0.0f, -25.0f);
    GXTexCoord2f32(0.0f, 0.0f);
    size = GXEndDisplayList();
    list = HuMemDirectMallocNum(HEAP_HEAP, size, HU_MEMNUM_OVL);
    *displayList = list;
    memcpy(*displayList, listBuffer, size);
    DCFlushRange(*displayList, size);
    HuMemDirectFree(listBuffer);
    return size;
}

// The board update drifts the ambient particles with wind and wraps them around the camera view.
void fn_1_1330C(void)
{
    W01_SNOW_PARTICLE *particle = lbl_1_bss_4B0;
    MBCAMERA *cameraP = mbCameraGet();
    HuVecF normal;
    HuVecF windDir;
    HuVecF cross;
    float dot;
    float mag;
    s32 i;

    if (!cameraP->dispOn) {
        return;
    }
    if (!lbl_1_bss_EB0) {
        return;
    }
    for (i = 0; i < 64; i++, particle++) {
        normal.x = -mbSinRad(particle->angle);
        normal.y = mbCosRad(particle->angle);
        normal.z = 0.0f;
        windDir.x = -particle->vel.x;
        windDir.y = -particle->vel.y;
        windDir.z = -particle->vel.z;
        PSVECAdd(&windDir, &lbl_1_bss_48C, &windDir);
        mag = PSVECMag(&windDir);
        if (mag != 0.0f) {
            PSVECNormalize(&windDir, &windDir);
        } else {
            windDir.x = windDir.y = windDir.z = 0.0f;
        }
        dot = PSVECDotProduct(&normal, &windDir);
        if (dot < 0.0f) {
            normal.x = -normal.x;
            normal.y = -normal.y;
            normal.z = -normal.z;
            dot = -dot;
        }
        PSVECCrossProduct(&normal, &windDir, &cross);
        if (cross.z < 0.0f) {
            particle->angleSpeed -= 0.002f
                * ((1.0f / 60.0f) * mag * dot);
        } else {
            particle->angleSpeed += 0.002f
                * ((1.0f / 60.0f) * mag * dot);
        }
        particle->angle += particle->angleSpeed;
        particle->angleSpeed *= 0.95f;
        mag *= 10.0f;
        particle->vel.x += (1.0f / 60.0f) * (normal.x * dot * mag);
        particle->vel.y += -16.333334f
            + ((1.0f / 60.0f) * (normal.y * dot * mag));
        particle->vel.z += (1.0f / 60.0f) * (normal.z * dot * mag);
        particle->pos.x += (1.0f / 60.0f) * particle->vel.x;
        particle->pos.y += (1.0f / 60.0f) * particle->vel.y;
        particle->pos.z += (1.0f / 60.0f) * particle->vel.z;
        PSVECScale(&particle->vel, &particle->vel, 0.95f);
        if (1500.0f + cameraP->eye.x < particle->pos.x) {
            particle->pos.x = -1500.0f + cameraP->eye.x
                + fmod(particle->pos.x - (1500.0f + cameraP->eye.x),
                    3000.0);
        } else if (-1500.0f + cameraP->eye.x > particle->pos.x) {
            particle->pos.x = 1500.0f + cameraP->eye.x
                + fmod(particle->pos.x - (-1500.0f + cameraP->eye.x),
                    3000.0);
        }
        if (particle->pos.y < 0.0f) {
            particle->pos.y = 6000.0f;
        }
        if (-1000.0f + cameraP->eye.z < particle->pos.z) {
            particle->pos.z = -3000.0f + cameraP->eye.z
                + fmod(particle->pos.z - (-1000.0f + cameraP->eye.z),
                    2000.0);
        } else if (-3000.0f + cameraP->eye.z > particle->pos.z) {
            particle->pos.z = -1000.0f + cameraP->eye.z
                + fmod(particle->pos.z - (-3000.0f + cameraP->eye.z),
                    2000.0);
        }
    }
    lbl_1_bss_48C.x += 0.1f * (lbl_1_bss_480.x - lbl_1_bss_48C.x);
    lbl_1_bss_48C.y += 0.1f * (lbl_1_bss_480.y - lbl_1_bss_48C.y);
    lbl_1_bss_48C.z += 0.1f * (lbl_1_bss_480.z - lbl_1_bss_48C.z);
    if (frandf() < 0.01f) {
        lbl_1_bss_480.x = (MBTimeDayGet() ? 100.0f : 800.0f)
            + (100.0f * (8.0f * frandf()));
        lbl_1_bss_480.y = 0.0f;
        lbl_1_bss_480.z = 0.0f;
    }
}

void fn_1_139E8(s32 value)
{
    lbl_1_bss_EB0 = value;
}

// New-board setup shuffles the candidate Star spaces and starts at the first shuffled entry.
void fn_1_139F8(void)
{
    s32 i;
    s32 index1;
    s32 index2;
    u8 value;

    lbl_1_bss_1618->index = 0;
    lbl_1_bss_1618->count = mbMasuMAttrListGet((1 << 7), NULL);
    for (i = 0; i < lbl_1_bss_1618->count; i++) {
        lbl_1_bss_1618->order[i] = i;
    }
    for (i = 0; i < 100; i++) {
        index1 = mbRandMod(lbl_1_bss_1618->count);
        index2 = mbRandMod(lbl_1_bss_1618->count);
        value = lbl_1_bss_1618->order[index1];
        lbl_1_bss_1618->order[index1] = lbl_1_bss_1618->order[index2];
        lbl_1_bss_1618->order[index2] = value;
    }
}

// Board setup activates the current shuffled Star space and creates its Star marker; it ignores the
// returned candidate count.
void fn_1_13B38(void)
{
    s32 count;
    s16 masuList[8];

    count = mbMasuMAttrListGet((1 << 7), masuList);
    lbl_1_bss_1616 = masuList[lbl_1_bss_1618->order[lbl_1_bss_1618->index]];
    mbMasuTypeSet(lbl_1_bss_1616, 7);
    mbStarMasuNextSet(lbl_1_bss_1616);
}

// The Star-move hook restores the old space to blue and advances to the next shuffled Star space.
// It discards the old position and refreshed candidate count.
void fn_1_13BC4(void)
{
    s32 count;
    HuVecF pos;
    s16 masuList[8];

    mbMasuPosGet(lbl_1_bss_1616, &pos);
    mbMasuTypeSet(lbl_1_bss_1616, 1);
    lbl_1_bss_1618->index = (lbl_1_bss_1618->index + 1) % lbl_1_bss_1618->count;
    count = mbMasuMAttrListGet((1 << 7), masuList);
    lbl_1_bss_1616 = masuList[lbl_1_bss_1618->order[lbl_1_bss_1618->index]];
    mbMasuTypeSet(lbl_1_bss_1616, 7);
    mbStarMasuNextSet(lbl_1_bss_1616);
}

s16 fn_1_13CBC(s32 playerNo)
{
    return mbMasuFind_TypeIdGet(GwPlayer[playerNo].masuId, 7, TRUE, TRUE);
}

// MB1_Create prepares thirty-two hidden coin models for the board's coin-loss effects.
void fn_1_13D04(void)
{
    W01_COIN_EFFECT *effect;
    s32 i;
    s16 modelId;

    for (effect = lbl_1_bss_0, i = 0; i < 32; i++, effect++) {
        modelId = mbObjCreate(mbBoardDataNumGet(DATANUM(DATA_board, 4)), NULL, TRUE);
        effect->modelId = modelId;
        effect->active = FALSE;
        mbObjDispSet(effect->modelId, FALSE);
    }
}

// Coin-loss events search for inactive coin effects and launch them.
// The pointer stays advanced between coins while each search allows 32 entries, so later searches
// can pass the end of the pool.
void fn_1_13D8C(const HuVecF *pos, s32 count)
{
    W01_COIN_EFFECT *effect;
    s32 i;
    s32 j;
    float angle;

    effect = lbl_1_bss_0;
    for (i = 0; i < count; i++) {
        for (j = 0; j < 32; j++, effect++) {
            if (!effect->active) {
                break;
            }
        }
        if (j >= 32) {
            break;
        }
        effect->active = TRUE;
        effect->pos = *pos;
        angle = mbRandMod(360);
        effect->vel.x = 100.0f * (3.0f * mbCosDeg(angle));
        effect->vel.y = 2800.0f + (100.0f * (2.0f * frandf()));
        effect->vel.z = 100.0f * (3.0f * mbSinDeg(angle));
        mbObjRotYSet(effect->modelId, mbRandMod(360));
        mbObjAlphaSet(effect->modelId, 255);
        effect->time = 0;
        effect->maxTime = 42;
    }
}

// The board update moves and spins active coin effects, fading them during their final five frames.
void fn_1_13F50(void)
{
    W01_COIN_EFFECT *effect;
    s32 i;
    float ratio;

    for (effect = lbl_1_bss_0, i = 0; i < 32; i++, effect++) {
        mbObjDispSet(effect->modelId, effect->active);
        if (effect->active) {
            effect->vel.y += -163.33334f;
            effect->pos.x += (1.0f / 60.0f) * effect->vel.x;
            effect->pos.y += (1.0f / 60.0f) * effect->vel.y;
            effect->pos.z += (1.0f / 60.0f) * effect->vel.z;
            mbObjPosSetV(effect->modelId, &effect->pos);
            mbObjRotYSet(effect->modelId, 10.0f + mbObjRotYGet(effect->modelId));
            if (effect->maxTime - effect->time < 5) {
                ratio = (float)(effect->maxTime - effect->time) / 5.0f;
                mbObjAlphaSet(effect->modelId, 255.0f * ratio);
            }
            effect->time++;
            if (effect->time > effect->maxTime) {
                effect->active = FALSE;
            }
        }
    }
}

float *lbl_1_bss_1620;
OSTick lbl_1_bss_161C;
W01_STAR_MASU_WORK *lbl_1_bss_1618;
s16 lbl_1_bss_1616;
s16 lbl_1_bss_15F4[17];
W01_NEXT_TIME_WORK lbl_1_bss_15E8;
W01_PATH_WORK lbl_1_bss_155C;
W01_SPRING_WORK lbl_1_bss_1494;
s32 lbl_1_bss_1490;
W01_FLOAT_WORK *lbl_1_bss_148C;
s16 lbl_1_bss_1488;
W01_MOTION_WORK lbl_1_bss_1438[1];
s16 lbl_1_bss_1436;
s16 lbl_1_bss_1434;
HuVecF lbl_1_bss_1428;
HuVecF lbl_1_bss_141C;
HuVecF lbl_1_bss_1410;
W01_WORK_13D8 lbl_1_bss_13D8;
ANIMDATA *lbl_1_bss_13D0[2];
W01_MASU_MOTION_WORK lbl_1_bss_137C[3];
ANIMDATA *lbl_1_bss_1378;
W01_MASU_MODEL_WORK lbl_1_bss_1338[2];
ANIMDATA *lbl_1_bss_1334;
W01_WH_EFFECT_STORAGE lbl_1_bss_1240;
MBMODELID lbl_1_bss_1238[4];
s16 lbl_1_bss_1230[4];
HuVecF lbl_1_bss_1224;
HU3D_MODELID lbl_1_bss_1220;
ANIMDATA *lbl_1_bss_121C;
void *lbl_1_bss_1214[2];
u32 lbl_1_bss_120C[2];
float lbl_1_bss_1208;
float lbl_1_bss_1188[32];
W01_BIRI_WORK lbl_1_bss_EFC;
s32 lbl_1_bss_EF8;
W01_PARTICLE_WORK lbl_1_bss_EE8;
ANIMDATA *lbl_1_bss_EE4;
W01_GATE_WORK lbl_1_bss_EB4;
s32 lbl_1_bss_EB0;
W01_SNOW_PARTICLE lbl_1_bss_4B0[64];
ANIMDATA *lbl_1_bss_4A4[3];
HU3D_MODELID lbl_1_bss_4A0;
void *lbl_1_bss_49C;
u32 lbl_1_bss_498;
HuVecF lbl_1_bss_48C;
HuVecF lbl_1_bss_480;
W01_COIN_EFFECT lbl_1_bss_0[32];
