/* Initializes and runs the board events, model movement, and particle effects. */
#include "math.h"
#include "dolphin/types.h"
#include "game/object.h"
#include "game/board/effect.h"
#include "game/board/main.h"
#include "game/memory.h"
#include "game/flag.h"
#include "game/main.h"
#include "game/board/object.h"
#include "REL/w05Dll/work.h"
#include "dolphin/gx.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "game/process.h"
#include <string.h>
#include "game/board/player.h"
#include "game/board/masu.h"
#include "game/board/camera.h"
#include "game/board/window.h"
#include "game/board/audio.h"
#include "game/board/coin.h"
#include "game/board/comchoice.h"
#include "messdir_enum.h"
#include "game/hu3d.h"
#include "game/board/status.h"
#include "game/board/capsule.h"
#include "msm_se.h"
#define W05_RANDOM_RANGE 268435456U
#define W05_RANDOM_VALUE_MASK 32767
#define W05_BRANCH_ATTR_FLAGS 0x1840U
f32 mbSinDeg(f32 angle);
f32 mbCosDeg(f32 angle);
s16 lbl_1_bss_18;
OMOBJ *lbl_1_bss_14;
OMOBJ *lbl_1_bss_10;
OMOBJ *lbl_1_bss_C;
void *lbl_1_bss_8;
u8 *lbl_1_bss_4;
OMOBJ *lbl_1_bss_0;
extern char *lbl_1_data_40[][2];

typedef struct W05EffectWork_s {
    s32 modelId; /* Board particle model that receives the effect. */
    s32 count; /* Number of yellow particles still active in this effect. */
    void *animation; /* Sprite animation data retained for the particle model. */
} W05_EFFECT_WORK;

typedef struct W05EffectOwner_s {
    s32 modelId; /* Board particle model that owns the active particles. */
    s32 activeCount; /* Number of yellow particles still active in this effect. */
    void *animation; /* Sprite animation data retained for the particle model. */
} W05_EFFECT_OWNER;

typedef struct W05ColorEffectWork_s {
    s32 modelId; /* Board particle model used for colored particles. */
    s32 activeCount; /* Particles that have not finished fading. */
    s32 state; /* Effect state stored with the board particle task. */
    ANIMDATA *animation; /* Sprite animation data retained for the particle model. */
} W05_COLOR_EFFECT_WORK;

/* Retail w05Dll .rodata 0x0..0x3A0. */

/* Sets up the start, raised midpoint, and destination for a moving board object. */

extern s32 mbWipeSpecialCheck(void);
extern s32 mbWipeSpecialStatGet(void);

/* Creates particles across the board event's sampled geometry. */

/* Emits a yellow burst in the plane of the supplied rotation. */

/* Emits stationary white particles within the event bounds and starts their scale transition. */

/* Emits sixteen yellow particles from a position, with randomized speed and direction. */

/* Emits stationary fading particles over a rounded area around the supplied position. */

/* Emits a rotating ring of particles in the plane defined by the supplied rotation. */

/* Emits an upward burst, spreading each particle around the supplied position. */

/* Moves a board effect along its attachment's vertical axis and emits fading particles. */

extern f32 lbl_1_data_558[];
extern f32 lbl_1_data_590[];

/* Scales a player on the board attachment, holds it there, then spins it upward. */

/* Raises the effect over 24 frames, then hides it and removes the object next frame. */

void mbMapSprAdd(s16 spriteId, s16 spaceId);

extern HuVecF lbl_1_data_3A8;
extern HuVecF lbl_1_data_3B4;
extern f32 lbl_1_data_3C0;
extern HuVecF lbl_1_data_418;
extern HuVecF lbl_1_data_424;
extern f32 lbl_1_data_430;
extern HuVecF lbl_1_data_434;
extern HuVecF lbl_1_data_440;
extern f32 lbl_1_data_44C;
extern HuVecF lbl_1_data_450;
extern HuVecF lbl_1_data_45C;
extern f32 lbl_1_data_468;
extern HuVecF lbl_1_data_46C;
extern HuVecF lbl_1_data_478;
extern f32 lbl_1_data_484;
extern HuVecF lbl_1_data_488;
extern HuVecF lbl_1_data_494;
extern f32 lbl_1_data_4A0;
void mbWipeFadeIn(void);
void mbWipeFadeOut(void);
void mbWipeWait(void);
void mbTelopTimeChangeCreate(void);
BOOL mbTelopTimeChangeCheck(void);

/* Hides the players while the time-change camera sequence and help window run. */

/* Resolves the board resource's model for the current time of day. */

/* Creates the board's hooked machinery and chooses its initial space type. */

/* Attaches both board models to the selected hook and resets its event motion. */

/* Chooses a board link by target-space distance, with difficulty-based random detours. */

extern HuVecF lbl_1_data_4D4;
extern HuVecF lbl_1_data_4E0;
int mbGuideSpeakerNoGet(void);

/* Rehooks the board machinery around its day/night transition and updates the special space. */

void mbStarGetExec(int playerNo);

void mbev_CapObjMotionSet(int modelId, int time, int motionNo, int nextMotion,
                        u32 attr, u32 nextAttr, BOOL shift, BOOL nextShift);
s16 mbev_CapMasuPrevGet(s16 spaceId, HuVecF *position);

/* Runs the star purchase and carries the player through the changing board machinery. */

void mbev_CapPlayerRotate(int playerNo, float angle);

void mbZtarGetExec(int playerNo);

/* Runs the second hook's Ztar event, then launches the player back to the marked board space. */

extern u32 mbCapEffNum;
extern s16 *mbCapEffData;

#define W05_EFFECT_RAND_NEXT() \
    do { \
        if (++mbCapEffNum >= 1024) { \
            mbCapEffNum = 0; \
        } \
    } while (0)

extern HuVecF lbl_1_data_8E4;
extern HuVecF lbl_1_data_8F0;
extern HuVecF lbl_1_data_8FC;
extern HuVecF lbl_1_data_908;

/* The next value of the board's shared effect noise table, from -1 to 1. */

/* Offsets a shared noise sample for the ambient spray's radial placement. */

/* The board's per-frame update: waves the sea surface and scrolls its texture, emits the ambient
   spray, fades the night lamps, sways both hooks and carries what hangs on them. */

typedef struct W05TwinEffectWork_s {
    s32 modelId[2];
    s32 activeCount[2];
    ANIMDATA *animation;
} W05_TWIN_EFFECT_WORK;

/* Emits a rising blue-gray particle with scale and opacity based on strength. */

/* Captures the particle's final color before sampling its lifetime variation. */

/* The hooks' own copy of the rising particle (fn_1_19A90), with its noise read through
 * W05EffectRand. */

/* While the first hook swings, emits rising particles around it each frame: four rounds of one
   particle on each side of the hook and two behind it, stronger the faster it swings. */

/* While the second hook swings, emits rising particles around it each frame: four rounds of one
   particle on each side of the hook and one behind it, stronger the faster it swings. */

extern HuVecF lbl_1_data_6BC[];

extern BOOL mbSaveNewF;

void mbScrollInit(int dataNum);
int mbCapThrowColCreate(int dataNum);
void mbev_ShopInit(int dataNum);
void mbev_ShopBackCreate(int dataNum, int motionNum, BOOL nightF, BOOL allocF);
void mbev_NextTimeSet(void (*nextTimeHook)(void));
void mbLightFuncSet(void (*createHook)(void), void (*killHook)(void));

extern s32 lbl_1_data_50[];

void mbBranchMAttrSet(u32 attr);
void mbOpeningInstHookSet(void (*hook)(void));
void mbOpeningStarInstHookSet(void (*hook)(void));
void mbBranchComStarHookSet();

void mbMapHookSet();
void mbCapThrowHookSet();
void mbOpeningViewSet(HuVecF *rotation, HuVecF *position, f32 zoom);
void mbMapCameraSet(const HuVecF *rotation, const HuVecF *position, f32 zoom);

/* Allocates the board's state and initializes its lights and moving attachments. */

void mbev_CapStatusDispSetAll(int enabled, int immediate);
int mbev_CapCoinDisp(int playerNo, int coins, int display, int wait);
void mbev_Scroll(int playerNo, int mode);

/* Offers the coin event, runs its timed movement, then returns the player to the board. */

/* Starts the player's movement state and the computer player's input timers. */

/* Gets the raft's player attachment transform and its forward-facing rotation. */

typedef struct W05ArrivalWork_s {
    u8 objectState[132];
    s32 eventRequested;
    s32 arrivalModel;
    u8 eventState[260];
    s32 movingModel;
} W05_ARRIVAL_WORK;

int mbCoinAddExec(int playerNo, int coinNum);
void mbev_CapPlayerMotShiftWait(int playerNo, int motionNo, u32 attr, BOOL shiftF);

/* Offers the raft ride for ten coins, follows its motion and camera, and handles the optional
   detour. The board-space event calls this while the current player's move is suspended. */

/* Steers the computer player toward falling coins and schedules its jump input. */

/* Stops a board model's motion while the raft event is inactive. */

/* Allows a board model's motion to advance while the raft event is active. */

/* Updates raft motion speed and emits spray at the animated strokes each frame. */

/* Emits spray around the raft's attachment while its movement effect is active. */

/* Allocates and initializes the state used to move a board model. */

typedef struct W05ParticleWork_s {
    s32 modelId; /* Board particle model advanced by the task callback. */
    s32 activeCount; /* Particles currently visible or moving. */
} W05_PARTICLE_WORK;

/* Follows the raft attachment each frame and finishes its motion when stopped. */

/* Advances the board particles, hiding the model when none remain active. */

/* Starts a particle in the first free slot, retaining only the supplied alpha. */

void mbev_CapBezierGetV(f32 time, HuVecF *start, HuVecF *control,
                       HuVecF *end, HuVecF *position);

/* Moves the model along its final arc, then plays the arrival motion.
   The particle emitter replaces the uninitialized RGB components with white. */

typedef struct W05TeresaSceneWork_s {
    u8 modelState[32];
    s32 motionModels[2]; /* Animated models selected by the two event spaces. */
    s32 spaces[2]; /* Spaces whose model motion accompanies Boo's entrance. */
    u8 spaceState[16];
    s32 defaultMotionModel; /* Model used when neither event space matches. */
} W05_TERESA_SCENE_WORK;

int mbBGRead(int dataNum);
void mbBGReadWait(int readId);
void mbev_CapCallTeresa(int playerNo, int spaceId);
void mbev_CapTeresaFadeCreate(int modelId);
void mbev_CapTeresaFadeSet(float alpha);
void mbev_CapTeresaFadeKill(int modelId);

void mbWipeDissolveFadeIn(void);

/* Brings Boo toward the player at night before running the Boo space event. */

/* Starts a shrinking board particle, with optional gravity and rotation. */

/* Moves and shrinks active particles, using a curved scale transition when requested. */

void mbev_CapVecChase(float weight, HuVecF *start, HuVecF *end, HuVecF *out);
/* Updates the light transition and records a skip button; removes itself on board exit. */

typedef struct W05TwinParticleWork_s {
    s32 modelId[2];
    s32 activeCount[2];
    ANIMDATA *animation;
} W05_TWIN_PARTICLE_WORK;

/* Starts a particle in either effect model; its object state is read before the null check. */

extern s32 lbl_1_data_50[];

/* Creates a flying model in the first vacant slot and sets its launch direction. */

/* Collects a flying coin or knocks away a nearby hazard when it reaches the player. */

extern HuVecF lbl_1_data_0;
extern HuVecF lbl_1_data_C;

/* Raises and bobs two event models, then lowers them and shakes the camera on impact. */

/* Builds the models attached to the board's linked event spaces during setup. */

typedef struct W05SurfaceVertex_s {
    HuVecF phase; /* Independent wave phases in degrees for each axis. */
    HuVecF amplitude; /* Displacement applied around the original vertex position. */
    HuVecF phaseStep; /* Added to the phases when the surface animation advances. */
    HuVecF position; /* Original model vertex coordinates. */
} W05_SURFACE_VERTEX;

typedef struct W05DrawWork_s {
    u8 sceneState[80];
    s32 modelId;
    s32 surfaceHookModel;
    s32 effectHookModel;
    u8 modelState[508];
    void *displayList;
    u32 displayListSize;
    f32 (*textureCoords)[2];
    ANIMDATA *animation;
    f32 (*baseWarpCoords)[2];
    f32 (*warpCoords)[2];
    ANIMDATA *warpAnimation;
    GXColor *colors;
    HuVecF *positions;
    s32 vertexCount;
    f32 textureOffsetX;
    f32 textureOffsetY;
    s32 scrollElapsed;
    s32 scrollDuration;
    f32 scrollAngle;
    f32 scrollAngleRange;
    f32 scrollSpeedS;
    f32 scrollSpeedT;
    HuVecF boundsStart;
    HuVecF boundsSize;
    W05_SURFACE_VERTEX *surfaceVertices;
    s32 warpDisabled;
} W05_DRAW_WORK;

extern GXColor lbl_1_data_8C0, lbl_1_data_8C4, lbl_1_data_8C8;
extern f32 lbl_1_data_8CC[2][3];

/* Builds the board surface's texture arrays and triangle display list at board setup. */

/* Draws the board surface, optionally distorting its texture with a second map. */

/* Gets a Donkey Kong attachment's board transform after updating both models. */

/* Gets the selected Bowser attachment's board transform through its linked models. */

/* Carries a player to the marked space, then restores the camera and raft poses. */

extern f32 lbl_1_data_850[][2];
void mbWipeSpecialFadeInCreate(int type, int durationFrames);
void mbWipeSpecialFadeOutCreate(int type, int durationFrames);

/* Jumps the player onto a linked board mechanism and returns after its time-change animation. */

extern HuVecF lbl_1_data_770[2];
extern HuVecF lbl_1_data_788[14];

/* Runs the two characters' lifting event and moves each player to another player's board space. */

extern f32 lbl_1_data_728[][2];
f32 mbev_CapAngleSumLerp(float weight, float start, float end);

/* Moves the tossing model between launch points, then turns and releases flying items. */

extern HuVecF lbl_1_data_64C;
extern HuVecF lbl_1_data_658;
extern f32 lbl_1_data_664;
extern f32 lbl_1_data_668[][2];
extern f32 lbl_1_data_680[][2];
extern f32 lbl_1_data_698[];
extern f32 lbl_1_data_6A4[];
extern f32 lbl_1_data_6B0[];
void mbWipeDissolveFadeInTime(int durationFrames);
void mbWipeDissolveFadeOutTime(int durationFrames);

void mbev_CapPlayerMotShiftSet(int modelId, int motionNo, u32 attr, BOOL shiftF);

/* Plays the cannon event's camera, character and three machinery motions before restoring play. */

s16 mbCoinDispCreate(HuVecF *position, int coinNum, int sign, BOOL playSe);

/* Charges up to five coins, carries the player off the raft, then restores the destination view. */

/* Advances flying items through launch, falling, collision and disappearance states. */

/* Applies player input, jump physics and reaction motions during the flying-item event. */

extern s32 lbl_1_data_748[3];
extern HuVecF lbl_1_data_754[2];

void mbCapCapsuleGet(int playerNo, int capsuleNo);
int mbCapUseMesGet(int capsuleNo);
OMOBJ *mbev_CapEffCoinCreate(void);
void mbev_CapEffCoinKill(OMOBJ *object);
void mbev_CapCoinAdd(OMOBJ *object, int playerNo, int coinNum, BOOL highF);

/* Runs the two characters' board event and resolves the player's reward choice. */

void fn_1_A0(void);
void fn_1_F4(void);
void fn_1_1D50(void);
void fn_1_1DBC(OMOBJ *object);
void fn_1_1FB8(void);
void fn_1_2010(void);
void fn_1_2094(void);
void fn_1_20C0(s32 mode, f32 x, f32 y, f32 z);
int fn_1_20F8(int playerNo, s16 spaceId);
void fn_1_21D8(int playerNo);
s32 fn_1_2228(void);
int fn_1_2230(int playerNo, s16 spaceId);
int fn_1_2238(int playerNo, s16 spaceId);
int fn_1_231C(s16 spaceId, u32 attr, s16 *links, BOOL endF);
void fn_1_2324(s32 enabled);
void fn_1_246C(s32 mode);
void fn_1_24C4(void);
void fn_1_285C(void);
s32 fn_1_28C8(s32 playerNo, s32 linkCount, s16 *links, s32 forceNearest);
void fn_1_2B50(void);
void fn_1_2B54(void);
void fn_1_3308(s32 hookIndex);
void fn_1_3540(void);
void fn_1_35F0(s32 fadeEnabled, s32 specialFade, s32 explainTransition);
void fn_1_42B4(s32 argument);
void fn_1_433C(int playerNo);
void fn_1_5900(int playerNo);
void fn_1_757C(s32 hookIndex);
void fn_1_7610(s32 hookIndex);
void fn_1_76A4(s32 selector, HuVecF *position, HuVecF *rotation, Mtx matrix);
void fn_1_7AF8(s32 selector, HuVecF *position, HuVecF *rotation, Mtx matrix);
void fn_1_80AC(s32 playerNo, s32 state);
void fn_1_80D4(s32 modelId, s32 state);
void fn_1_80FC(s32 playerNo, s32 state);
void fn_1_8124(s32 modelId, s32 state);
void fn_1_814C(OMOBJ *object);
void fn_1_8670(OMOBJ *object);
void fn_1_8A2C(HuVecF *destination, u32 parameter);
void fn_1_8B58(OMOBJ *object);
void fn_1_8FC0(s32 playerNo, s32 spaceId);
void fn_1_94B4(void);
void fn_1_94E0(s32 fadeIn);
void fn_1_9528(int playerNo, int currentSpace);
void fn_1_B078(HuVecF *position, HuVecF *rotation, Mtx matrix);
void fn_1_B304(OMOBJ *object);
void fn_1_BB68(s32 count, f32 strength, s32 bothSides);
void fn_1_C398(s32 playerNo);
void fn_1_CE08(s32 playerNo, u32 spaceAttribute, s32 duration);
void fn_1_D390(s32 playerNo, s32 spaceId, s32 motionNo, s32 soundId, HuVecF *offset);
s32 fn_1_DFB0(s32 playerNo, s32 spaceId, s32 *occupiedSlots, HuVecF *position,
              s32 skipFirst);
void fn_1_E120(OMOBJ *object);
void fn_1_E4A0(int playerNo, s32 inputSpace);
void fn_1_F5C4(s32 playerNo);
s32 fn_1_F87C(s32 inputEnabled);
void fn_1_101F8(void);
void fn_1_1024C(void);
void fn_1_10A18(s32 modelId);
s32 fn_1_10B90(s32 remainingFrames);
void fn_1_114E4(void);
void fn_1_11538(void);
s32 fn_1_115CC(s32 remainingFrames);
void fn_1_12288(void);
void fn_1_122DC(s32 type, HuVecF *position, f32 direction, f32 strength);
s32 fn_1_12720(s32 modelIndex);
void fn_1_12D20(int playerNo, int spaceId);
void fn_1_13C3C(int playerNo, int spaceId);
void fn_1_15558(OMOBJ *object);
void fn_1_15C98(void);
void fn_1_160E4(s32 playerNo, s32 spaceId);
void fn_1_1698C(void);
void fn_1_1761C(HU3D_MODEL *hookModel, Mtx *matrix);
void fn_1_17A84(HU3D_MODEL *hookModel, Mtx *hookMatrix);
void fn_1_19460(void);
void fn_1_1964C(OMOBJ *obj);
void fn_1_197F8(void);
s32 fn_1_1980C(void);
void fn_1_19864(HuVecF *position, HuVecF *velocity, f32 scale,
               f32 rotation, f32 gravity, GXColor *color, s32 duration, s32 additive);
void fn_1_19A90(HuVecF *position, f32 strength);
void fn_1_1A13C(void);
void fn_1_1C0AC(void);
void fn_1_1D82C(void);
void fn_1_1D95C(OMOBJ *object);
void fn_1_1DC08(void);
s32 fn_1_1DC1C(void);
void fn_1_1DC6C(s32 value);
s32 fn_1_1DCE4(HuVecF position, HuVecF velocity, f32 scale,
              f32 duration, f32 rotationSpeed, f32 gravity, GXColor color);
void fn_1_1DF14(HuVecF *position, HuVecF *rotation);
void fn_1_1E3C0(s32 count);
void fn_1_1E6D0(HuVecF *position);
void fn_1_1EA60(s32 particleIndex);
void fn_1_1EAF4(void);
void fn_1_1EC00(OMOBJ *object);
void fn_1_1EE24(void);
int fn_1_1EE38(void);
s32 fn_1_1EE80(HuVecF position, HuVecF velocity, f32 scale,
              f32 rotationSpeed, f32 ageStep, GXColor color);
void fn_1_1F090(HuVecF *position, f32 radius, f32 height, s32 count);
void fn_1_1F57C(HuVecF position, HuVecF rotation, s32 count);
void fn_1_1F9B0(HuVecF position, s32 count);
/* Selects the day or night archive for Castaway Bay board assets, leaving other directories
 * unchanged. */
static inline int W05BoardDataNumGet(int dataNum) {
    int directory;

    dataNum = mbBoardDataNumGet(dataNum);
    directory = DIRNUM(dataNum);
    if (directory != DATA_w05 && directory != DATA_w05n) {
        return dataNum;
    }
    dataNum = FILENUM(dataNum);
    if (GwSystem.curTime == 0) {
        dataNum |= DATA_w05;
    } else {
        dataNum |= DATA_w05n;
    }
    return dataNum;
}

static inline f32 W05SinDeg(f32 angle) {
    return mbSinDeg(angle);
}

static inline f32 W05CosDeg(f32 angle) {
    return mbCosDeg(angle);
}

/* Advances the board effect noise index and returns its signed sample in the range -1 to 1. */
static inline f32 W05EffectRand(void) {
    s16 sample;

    W05_EFFECT_RAND_NEXT();
    sample = mbCapEffData[mbCapEffNum];
    return 3.051851e-05f * sample;
}

static inline f32 W05AmbientOffset(void) {
    return W05EffectRand() - 0.5f;
}

/* Selects the day or night archive for Castaway Bay effect assets. */
static inline int W05EffectDataNumGet(int dataNum) {
    int directory;

    dataNum = mbBoardDataNumGet(dataNum);
    directory = DIRNUM(dataNum);
    if (directory != DATA_w05 && directory != DATA_w05n) {
        return dataNum;
    }
    dataNum = FILENUM(dataNum);
    if (GwSystem.curTime == 0) {
        dataNum |= DATA_w05;
    } else {
        dataNum |= DATA_w05n;
    }
    return dataNum;
}

/* Saves the particle's final color, then samples the shared noise table for its lifetime. */
static inline f32 W05ParticleLifetimeSample(GXColor *particleColor, GXColor *color) {
    s16 sample;

    *particleColor = *color;
    W05_EFFECT_RAND_NEXT();
    sample = mbCapEffData[mbCapEffNum];
    return 3.051851e-05f * sample;
}

/* Creates a rising particle around the hook during its movement effects. */
static inline void W05HookRiseParticle(HuVecF *position, f32 strength) {
    HuVecF velocity;
    GXColor color;
    GXColor particleColor;
    f32 tint;
    f32 scale;
    s32 additive;

    velocity.x = 0.0f;
    velocity.y = 5.0f * (0.8f + 0.3f * W05EffectRand());
    velocity.z = 0.0f;
    tint = W05EffectRand();
    color.r = 150.0f + 63.0f * tint;
    color.g = 150.0f + 63.0f * tint;
    color.b = 190.0f + 63.0f * tint;
    color.a = (96.0f + 63.0f * W05EffectRand()) - 64.0f * strength;
    scale = strength * (1.0f + 0.25f *
        (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
    if (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE) < 0.7f) {
        additive = 0;
    } else {
        additive = 1;
    }
    fn_1_19864(position, &velocity,
        scale * (100.0f * (0.75f + 0.5f * W05EffectRand())),
        360.0f * W05EffectRand(), 0.8166667f, &particleColor,
        60.0f * (0.25f + 0.05f * W05ParticleLifetimeSample(&particleColor, &color)), additive);
}

/* Selects the day or night archive for Castaway Bay setup assets. */
static inline int W05SetupDataNumGet(int dataNum) {
    int directory;

    dataNum = mbBoardDataNumGet(dataNum);
    directory = DIRNUM(dataNum);
    if (directory != DATA_w05 && directory != DATA_w05n) {
        return dataNum;
    }
    dataNum = FILENUM(dataNum);
    if (GwSystem.curTime == 0) {
        dataNum |= DATA_w05;
    } else {
        dataNum |= DATA_w05n;
    }
    return dataNum;
}

static inline void *W05BoardAlloc(int size) {
    return HuMemDirectMallocNum(HEAP_HEAP, size, HU_MEMNUM_OVL);
}

static inline void W05ModelPauseOn(MBMODELID modelId) {
    mbObjAttrSet(modelId, HU3D_MOTATTR_PAUSE);
}

static inline void W05ModelPauseOff(MBMODELID modelId) {
    mbObjAttrReset(modelId, HU3D_MOTATTR_PAUSE);
}

/* Selects the day or night archive for Castaway Bay surface assets. */
static inline int W05SurfaceDataNumGet(int dataNum) {
    int directory;

    dataNum = mbBoardDataNumGet(dataNum);
    directory = DIRNUM(dataNum);
    if (directory != DATA_w05 && directory != DATA_w05n) {
        return dataNum;
    }
    dataNum = FILENUM(dataNum);
    if (GwSystem.curTime == 0) {
        dataNum |= DATA_w05;
    } else {
        dataNum |= DATA_w05n;
    }
    return dataNum;
}

static inline void *W05SurfaceScratchAlloc(void) {
    return HuMemDirectMallocNum(HEAP_HEAP, 65536, HU_MEMNUM_OVL);
}

static inline void *W05SurfaceListAlloc(s32 size, u32 allocationNo) {
    return HuMemDirectMallocNum(HEAP_MODEL, size, allocationNo);
}
void fn_1_A0(void) {
    GwSystem.partyF = TRUE;
    mbObjectSetup(4, fn_1_F4, fn_1_1D50);
}

HuVecF lbl_1_data_0 = {2885.0f, 295.0f, 2670.0f};
HuVecF lbl_1_data_C = {2885.0f, 295.0f, 2870.0f};
char *lbl_1_data_40[2][2] = {{"shiphook1", "shiphook2"}, {"shiphook3", "shiphook4"}};
s32 lbl_1_data_50[3] = {DATANUM(DATA_board, 4), DATANUM(DATA_w05, 39), DATANUM(DATA_w05, 38)};
f32 lbl_1_data_5C[84] = {
    -1005.0f, 1.0f, 5265.0f,
    300.0f, 50.0f, 1500.0f,
    -5095.0f, 400.0f, -190.0f,
    -4715.0f, 25.0f, -4835.0f,
    -4635.0f, 530.0f, -1435.0f,
    -4885.0f, 460.0f, -820.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 12.0f, 0.0f,
    0.0f, -10.0f, 0.0f,
    0.0f, 105.0f, 0.0f,
    0.0f, -10.0f, 0.0f,
    0.0f, -10.0f, 0.0f,
    -1887.0f, 0.0f, -216.0f,
    -1887.0f, 78.0f, -617.0f,
    -1887.0f, 279.0f, -1014.0f,
    -1887.0f, 479.0f, -1410.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    -3955.0f, 791.0f, 100.0f,
    -3487.0f, 791.0f, -200.0f,
    -3020.0f, 791.0f, -200.0f,
    -2470.0f, 791.0f, 90.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, -45.0f, 0.0f,
    0.0f, -75.0f, 0.0f,
    0.0f, -105.0f, 0.0f
};
char *lbl_1_data_1DC[4] = {"ukyasihook1", "ukyasihook2", "ukyasihook3", "ukyasihook4"};
char *lbl_1_data_214[4] = {"yasihook1", "yasihook2", "yasihook3", "yasihook4"};
char *lbl_1_data_254[5] = {"pslhook1", "pslhook2", "pslhook3", "pslhook4", "pslhook5"};
char *lbl_1_data_280[3] = {"rbhook1", "rbhook2", "rbhook3"};
char *lbl_1_data_2A0[2] = {"taruhook1", "taruhook2"};
char *lbl_1_data_2CC[3] = {"bananahook1", "bananahook2", "bananahook3"};
char *lbl_1_data_300[4] = {"hanahook1", "hanahook2", "hanahook3", "hanahook4"};
f32 lbl_1_data_310[12] = {
    4102.0f, 656.0f, 2067.0f,
    -553.0f, 129.0f, 2898.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, -18.0f, 0.0f
};
HuVecF lbl_1_data_340 = {-37.0f, 0.0f, 0.0f};
HuVecF lbl_1_data_34C = {136.0f, 90.0f, 952.0f};
f32 lbl_1_data_358[4] = {14510.0f, 276.0f, 0.0f, 0.0f};
HuVecF lbl_1_data_368 = {0.0f, -159.0f, 1304.0f};
f32 lbl_1_data_374 = 16248.0f;

/* Creates Castaway Bay's board models, attachments, event hooks, and per-frame tasks during
 * setup. */
void fn_1_F4(void) {
    Mtx attachmentMatrix;
    Mtx hookMatrix;
    Mtx worldMatrix;
    char hookName[32];
    HuVecF dockPosition;
    HuVecF linkedPosition;
    HuVecF dockDirection;
    W05_BOARD_WORK *board;
    s16 backgroundModel;
    int modelId;
    int hookNo;
    int spaceId;
    int boardNo;
    int dockNo;
    int firstLink;
    int secondLink;

    boardNo = MBBoardNoGet();
    HuAudSndGrpSetSet(26);
    mbMasuInit(W05SetupDataNumGet(DATANUM(DATA_w05, 0)));
    lbl_1_bss_4 = (u8 *)GwSystem.boardWork;
    if (mbSaveNewF) {
        *lbl_1_bss_4 = 0;
    }
    board = lbl_1_bss_8 = W05BoardAlloc(sizeof(W05_BOARD_WORK));
    memset(board, 0, sizeof(W05_BOARD_WORK));
    board->lightId = -1;
    board->lightMoving = 0;
    board->lightElapsed = 0;
    board->lightDuration = 60;
    board->lightRotation.x = board->lightRotation.y = board->lightRotation.z = 0.0f;
    board->lightTargetRotation.x = board->lightTargetRotation.y = board->lightTargetRotation.z =
        0.0f;
    board->firstHookIndex = -1;
    board->secondHookIndex = -1;
    board->boardEffectModel = -1;
    board->particleInitState[1] = 0;
    board->skipRequested = 0;
    board->skipEnabled = 0;
    board->hookSide = 0;
    board->firstHookMoving = 0;
    board->firstHookPhaseWeight = 1.0f;
    board->secondHookPhaseWeight = 1.0f;
    board->firstHookSway[0] = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
    board->firstHookSway[1] = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
    board->firstHookSway[2] = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
    board->secondHookSway[0] = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
    board->secondHookSway[1] = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
    board->secondHookSway[2] = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
    board->hookMovementScale = 0.0f;
    board->secondHookSwing = 0.0f;
    board->movementFactorX = 0.0f;
    board->movementFactorZ = 0.0f;
    board->particlesEnabled = 1;
    board->splashTimer = 0;
    board->secondHookMoving = 0;
    board->eventEnabled = 0;
    fn_1_80AC(-1, 0);
    fn_1_80D4(-1, 0);
    fn_1_80FC(-1, 0);
    fn_1_8124(-1, 0);
    lbl_1_bss_18 = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 3)), NULL, FALSE);
    mbObjPosSet(lbl_1_bss_18, 0.0f, 0.0f, 0.0f);
    backgroundModel = lbl_1_bss_18;
    mbObjAttrSet(backgroundModel, HU3D_MOTATTR_LOOP);
    mbObjCullRadiusSet(lbl_1_bss_18, -1.0f);
    mbScrollInit(W05SetupDataNumGet(DATANUM(DATA_w05, 1)));
    mbCapThrowColCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 2)));
    mbev_ShopInit(W05SetupDataNumGet(DATANUM(DATA_w05, 14)));
    mbev_ShopBackCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 13)), -1, FALSE, TRUE);
    mbev_NextTimeSet(fn_1_24C4);
    board->skipRequested = 0;
    board->skipEnabled = 0;
    mbLightFuncSet(fn_1_2010, fn_1_2094);
    modelId = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 5)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    Hu3DMotionCalc(mbObjModelIDGet(lbl_1_bss_18));
    modelId = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 12)), NULL, TRUE);
    board->backgroundAttachmentModel = modelId;
    Hu3DMotionCalc(mbObjModelIDGet(modelId));
    for (hookNo = 0; hookNo < 4; hookNo++) {
        mbObjHookSet(lbl_1_bss_18, lbl_1_data_1DC[hookNo], modelId);
        Hu3DModelObjMtxGet(mbObjModelIDGet(lbl_1_bss_18), lbl_1_data_1DC[hookNo], hookMatrix);
        Hu3DModelObjMtxGet(mbObjModelIDGet(modelId), "saruhook", attachmentMatrix);
        PSMTXConcat(hookMatrix, attachmentMatrix, worldMatrix);
        board->movingModelPositions[hookNo].x = worldMatrix[0][3];
        board->movingModelPositions[hookNo].y = worldMatrix[1][3];
        board->movingModelPositions[hookNo].z = worldMatrix[2][3];
    }
    modelId = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 16)), NULL, FALSE);
    board->rockingModel = modelId;
    mbObjPosSetV(modelId, &lbl_1_data_0);
    mbObjLayerSet(modelId, 3);
    mbObjMotionSpeedSet(modelId, 0.0f);
    modelId = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 17)), NULL, FALSE);
    board->rockingEffectModel = modelId;
    mbObjPosSetV(modelId, &lbl_1_data_C);
    mbObjLayerSet(modelId, 3);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjDispSet(modelId, FALSE);
    modelId = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 19)), NULL, FALSE);
    board->raftParentModel = modelId;
    mbObjLayerSet(modelId, 3);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjMotionTimeSet(modelId, mbObjMotionMaxTimeGet(modelId));
    modelId = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 18)), NULL, FALSE);
    board->raftModel = modelId;
    mbObjLayerSet(modelId, 3);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjHookSet(board->raftParentModel, "ikadahook", modelId);
    modelId = mbObjCreate(DATANUM(DATA_capsule, 59), NULL, FALSE);
    board->raftArrivalModel = modelId;
    mbObjLayerSet(modelId, 3);
    mbObjDispSet(modelId, FALSE);
    fn_1_15C98();
    modelId = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 23)), NULL, FALSE);
    board->backgroundMotionModel = modelId;
    mbObjMotionSpeedSet(modelId, 0.0f);
    for (spaceId = 1, dockNo = 0; spaceId < mbMasuNumGet(); spaceId++) {
        hookNo = dockNo;
        if (mbMasuMAttrGet(spaceId) & 1) {
            if (dockNo >= 2) {
                break;
            }
            firstLink = mbMasuAttrFindLink(spaceId, 1 << 13);
            mbMasuPosGet(firstLink, &dockPosition);
            secondLink = mbMasuAttrFindLink(firstLink, 1 << 13);
            mbMasuPosGet(secondLink, &linkedPosition);
            PSVECSubtract(&dockPosition, &linkedPosition, &dockDirection);
            modelId = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 20)), NULL, TRUE);
            board->dockModels[hookNo] = modelId;
            mbObjMotionSpeedSet(modelId, 0.0f);
            mbObjPosSetV(modelId, &linkedPosition);
            mbObjRotSet(modelId, 0.0f,
                180.0 * (atan2(dockDirection.x, dockDirection.z) / M_PI), 0.0f);
            modelId = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 21)), NULL, TRUE);
            board->lampModels[hookNo] = modelId;
            mbObjLayerSet(modelId, 3);
            mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
            Hu3DMotionCalc(mbObjModelIDGet(board->dockModels[hookNo]));
            Hu3DModelObjMtxGet(mbObjModelIDGet(board->dockModels[hookNo]), "lhhook",
                               attachmentMatrix);
            mbObjPosSet(modelId, attachmentMatrix[0][3], attachmentMatrix[1][3],
                        attachmentMatrix[2][3]);
            if (GwSystem.curTime == 0) {
                mbObjDispSet(modelId, FALSE);
            }
            board->dockSpaces[hookNo] = spaceId;
            dockNo++;
        }
    }
    board->lampAlpha = 1.0f;
    board->lampEnabled = 1;
    modelId = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 7)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    for (hookNo = 0; hookNo < 4; hookNo++) {
        mbObjHookSet(lbl_1_bss_18, lbl_1_data_214[hookNo], modelId);
    }
    modelId = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 6)), NULL, FALSE);
    for (hookNo = 0; hookNo < 5; hookNo++) {
        mbObjHookSet(lbl_1_bss_18, lbl_1_data_254[hookNo], modelId);
    }
    modelId = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 8)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    for (hookNo = 0; hookNo < 3; hookNo++) {
        mbObjHookSet(lbl_1_bss_18, lbl_1_data_280[hookNo], modelId);
    }
    modelId = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 9)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    for (hookNo = 0; hookNo < 2; hookNo++) {
        mbObjHookSet(lbl_1_bss_18, lbl_1_data_2A0[hookNo], modelId);
    }
    modelId = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 10)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    for (hookNo = 0; hookNo < 3; hookNo++) {
        mbObjHookSet(lbl_1_bss_18, lbl_1_data_2CC[hookNo], modelId);
    }
    modelId = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 11)), NULL, FALSE);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    for (hookNo = 0; hookNo < 3; hookNo++) {
        mbObjHookSet(lbl_1_bss_18, lbl_1_data_300[hookNo], modelId);
    }
    if (GwSystem.curTime == 0) {
        modelId = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 42)), NULL, FALSE);
        mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
        mbObjMotionSpeedSet(modelId, 2.0f);
        mbObjRotSet(modelId, 0.0f, 90.0f, 0.0f);
        mbObjScaleSet(modelId, 1.0f, 1.0f, 1.0f);
        for (hookNo = 0; hookNo < 12; hookNo++) {
            sprintf(hookName, "gullhook%d", hookNo + 1);
            mbObjHookSet(lbl_1_bss_18, hookName, modelId);
        }
    } else {
        modelId = mbObjCreate(W05SetupDataNumGet(DATANUM(DATA_w05, 40)), NULL, FALSE);
        mbObjMotionCreate(modelId, W05SetupDataNumGet(DATANUM(DATA_w05, 41)));
        mbObjMotionSet(modelId, 1, HU3D_MOTATTR_LOOP);
        mbObjHookSet(lbl_1_bss_18, "alienhook", modelId);
    }
    modelId = mbObjCreate(DATANUM(DATA_w05, 15), NULL, FALSE);
    mbObjLayerSet(modelId, 1);
    for (hookNo = 0; hookNo < 3; hookNo++) {
        modelId = mbObjCreate(W05SetupDataNumGet(lbl_1_data_50[hookNo]), NULL, TRUE);
        board->eventModels[hookNo] = modelId;
        mbObjLayerSet(modelId, 3);
        mbObjDispSet(modelId, FALSE);
    }
    fn_1_2B54();
    fn_1_1698C();
    fn_1_19460();
    fn_1_1D82C();
    fn_1_1EAF4();
    mbBranchMAttrSet(W05_BRANCH_ATTR_FLAGS);
    HuDataDirClose(DATA_w05);
    HuDataDirClose(DATA_w05n);
    mbPlayerTurnCloseHookSet(fn_1_21D8);
    mbev_MasuMoveStartSet(fn_1_2230);
    mbev_MasuMoveEndSet(fn_1_2238);
    mbev_MasuHatenaSet(fn_1_20F8);
    mbev_MasuLinkTblHookSet(fn_1_231C);
    mbMapHookSet(fn_1_2324);
    mbCapThrowHookSet(fn_1_246C);
    mbOpeningInstHookSet(fn_1_1FB8);
    mbOpeningStarInstHookSet(fn_1_2B50);
    mbBranchComStarHookSet(fn_1_28C8);
    lbl_1_bss_0 = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_1DBC);
    mbOpeningViewSet(&lbl_1_data_340, &lbl_1_data_34C, lbl_1_data_358[0]);
    mbMapCameraSet(NULL, &lbl_1_data_368, lbl_1_data_374);
}

/* Tears down Castaway Bay's main model and cannon collision resources when board setup ends. */
void fn_1_1D50(void) {
    if (lbl_1_bss_18 >= 0) {
        mbObjKill(lbl_1_bss_18);
        lbl_1_bss_18 = -1;
    }
    mbCapThrowColCreate(-1);
    fn_1_197F8();
    lbl_1_bss_0 = 0;
}

/* Updates the board light transition and watches for a skip input; runs each frame until board
 * exit. */
void fn_1_1DBC(OMOBJ *object) {
    Mtx rotation;
    HuVecF angles;
    HuVecF position;
    HuVecF direction;
    W05_BOARD_WORK *board;
    f32 progress;
    s32 controller;

    board = lbl_1_bss_8;
    if (mbExitCheck() || lbl_1_bss_0 == NULL) {
        omDelObjEx(mbObjMan, object);
        lbl_1_bss_0 = NULL;
        return;
    }
    if (board->lightId != -1 && board->lightMoving != 0) {
        progress = (f32)++board->lightElapsed / (f32)board->lightDuration;
        mbev_CapVecChase(progress * progress, &board->lightRotation,
                        &board->lightTargetRotation, &angles);
        mtxRot(rotation, angles.x, angles.y, angles.z);
        position = board->lightPosition;
        PSMTXMultVec(rotation, &board->lightDirection, &direction);
        Hu3DGLightPosSetV(board->lightId, &position, &direction);
        if (progress >= 1.0f) {
            board->lightMoving = 0;
            board->lightRotation = board->lightTargetRotation;
        }
    }
    if (board->skipEnabled != 0 && board->skipRequested == 0) {
        for (controller = 0; controller < GW_PLAYER_MAX; controller++) {
            if (HuPadBtnDown[controller] & 0x1000) {
                board->skipRequested = 1;
            }
        }
    }
}

/* Shows the Castaway Bay guide before the hooks move for the day/night change. */
void fn_1_1FB8(void) {
    mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 48), mbGuideSpeakerNoGet());
    mbWinTopWait();
    mbWipeDissolveFadeOutTime(1);
    fn_1_35F0(0, 0, 1);
    mbWipeDissolveFadeInTime(60);
}

/* Applies the sea lighting tint for the current day/night setting. */
void fn_1_2010(void) {
    void *boardWork;

    boardWork = lbl_1_bss_8;
    if ((u8) (((*(u8 *)((s8 *)(&GwSystem) + 16)) >> 6U) & 1) == 0) {
        Hu3DBGColorSet(127U, 127U, 192U);
    } else {
        Hu3DBGColorSet(64U, 64U, 127U);
    }
    Hu3DModelLightInfoSet(mbObjModelIDGet(lbl_1_bss_18), 1);
}

void fn_1_2094(void) {
    Hu3DBGColorSet(255U, 255U, 255U);
}

/* Records an active board request with a mode and three coordinates. */
void fn_1_20C0(s32 mode, f32 x, f32 y, f32 z) {
    s32 *work = (s32 *)lbl_1_bss_8;

    work[81] = 1;
    ((f32 *)work)[76] = x;
    ((f32 *)work)[77] = y;
    ((f32 *)work)[78] = z;
    work[80] = mode;
}

/* Routes a question-mark landing to the Castaway Bay event selected by its space attributes. */
int fn_1_20F8(int playerNo, s16 spaceId) {
    if ((u32) (mbMasuMAttrGet(spaceId) & 1) == 0) {
        if ((u32) (mbMasuMAttrGet(spaceId) & 0x20) != 0) {
            fn_1_E4A0(playerNo, (s16) spaceId);
        } else if ((u32) (mbMasuMAttrGet(spaceId) & 8) != 0) {
            if ((u8) (((*(u8 *)((s8 *)(&GwSystem) + 16)) >> 6U) & 1) == 0) {
                fn_1_12D20(playerNo, (s16) spaceId);
            } else {
                fn_1_13C3C(playerNo, (s16) spaceId);
            }
        } else if ((u32) (mbMasuMAttrGet(spaceId) & 0x2000) != 0) {
            fn_1_160E4(playerNo, (s16) spaceId);
        }
    }
    return 1;
}

/* At player turn close, restores the second hook's attachments and resets the active marked
 * space. */
void fn_1_21D8(int playerNo) {
    void *boardWork;

    boardWork = lbl_1_bss_8;
    if ((s32) (*(s32 *)((s8 *)(boardWork) + 704)) != 0) {
        fn_1_3308(1);
        fn_1_3540();
        (*(s32 *)((s8 *)(boardWork) + 704)) = 0;
    }
}

s32 fn_1_2228(void) {
    return 0;
}

int fn_1_2230(int playerNo, s16 spaceId) {
    return 0;
}

/* Handles a player after a move ends, dispatching event spaces to their board sequence. */
int fn_1_2238(int playerNo, s16 spaceId) {
    void *work;

    work = lbl_1_bss_8;
    if ((u32) (mbMasuMAttrGet(spaceId) & 0x10) != 0) {
        fn_1_42B4(playerNo);
    } else if ((u32) (mbMasuMAttrGet(spaceId) & 1) == 0) {
        if ((u32) (mbMasuMAttrGet(spaceId) & 0x400) != 0) {
            mbMoveNumDispSet(playerNo, 0);
            fn_1_8FC0(playerNo, (s16) spaceId);
            mbMoveNumDispSet(playerNo, 1);
        } else if ((u32) (mbMasuMAttrGet(spaceId) & 2) != 0) {
            mbMoveNumDispSet(playerNo, 0);
            fn_1_9528(playerNo, (s16) spaceId);
        }
    }
    return 0;
}

int fn_1_231C(s16 spaceId, u32 attr, s16 *links, BOOL endF) {
    return -1;
}

/* Adds the two linked-space map markers while the board map is enabled. */
void fn_1_2324(s32 enabled) {
    s32 space;
    s32 *boardWork;

    boardWork = lbl_1_bss_8;
    if (enabled != 0) {
        boardWork[175] = 1;
        switch (*(u8 *)lbl_1_bss_4) {
        case 0:
            for (space = 1; space < mbMasuNumGet(); space++) {
                if ((mbMasuMAttrGet(space) & 0x800) != 0) {
                    mbMapSprAdd(14, space);
                }
                if ((mbMasuMAttrGet(space) & 0x1000) != 0) {
                    mbMapSprAdd(15, space);
                }
            }
            break;
        case 1:
        default:
            for (space = 1; space < mbMasuNumGet(); space++) {
                if ((mbMasuMAttrGet(space) & 0x1000) != 0) {
                    mbMapSprAdd(14, space);
                }
                if ((mbMasuMAttrGet(space) & 0x800) != 0) {
                    mbMapSprAdd(15, space);
                }
            }
            break;
        }
        boardWork[121] = 0;
    } else {
        boardWork[175] = 0;
        boardWork[121] = 1;
    }
}

/* Updates the night cannon event's enabled state when its board hook is called. */
void fn_1_246C(s32 mode) {
    s32 *work = (s32 *) lbl_1_bss_8;

    if (GwSystem.curTime != 0) {
        if (mode != 0) {
            work[83] = 0;
            return;
        }
        work[83] = 1;
    }
}

HuVecF lbl_1_data_3A8 = {-3250.0f, 338.0f, 4762.0f};
HuVecF lbl_1_data_3B4 = {328.0f, -5.0f, 0.0f};
f32 lbl_1_data_3C0 = 2500.0f;
f32 lbl_1_data_3C4[21] = {
    -3250.0f, 838.0f, 3462.0f,
    328.0f, 30.0f, 0.0f,
    500.0f, 3853.0f, 195.0f,
    1294.0f, 305.0f, -6.0f,
    0.0f, 3211.0f, 3853.0f,
    695.0f, 1294.0f, 325.0f,
    -36.0f, 0.0f, 511.0f
};
HuVecF lbl_1_data_418 = {902.0f, 1511.0f, -1149.0f};
HuVecF lbl_1_data_424 = {315.0f, 9.0f, 0.0f};
f32 lbl_1_data_430 = 2092.0f;
HuVecF lbl_1_data_434 = {902.0f, 1011.0f, -1149.0f};
HuVecF lbl_1_data_440 = {360.0f, -20.0f, 0.0f};
f32 lbl_1_data_44C = 1592.0f;
HuVecF lbl_1_data_450 = {3295.0f, 1309.0f, 4016.0f};
HuVecF lbl_1_data_45C = {315.0f, 30.0f, 0.0f};
f32 lbl_1_data_468 = 2000.0f;
HuVecF lbl_1_data_46C = {1890.0f, 209.0f, 3035.0f};
HuVecF lbl_1_data_478 = {349.0f, -60.0f, 0.0f};
f32 lbl_1_data_484 = 1200.0f;
HuVecF lbl_1_data_488 = {-3304.0f, 1138.0f, 4758.0f};
HuVecF lbl_1_data_494 = {352.0f, 351.0f, 0.0f};
f32 lbl_1_data_4A0 = 1500.0f;

/* Runs the time-change camera tour and guide window while the players are hidden. */
void fn_1_24C4(void) {
    HuVec2f windowPosition;
    s32 window;
    W05_BOARD_WORK *board;
    s32 frame;

    board = lbl_1_bss_8;
    for (frame = 0; frame < 4; frame++) {
        mbPlayerDispSet(frame, 0);
    }
    mbCameraCenterSetV(&lbl_1_data_488);
    mbCameraRotSetV(&lbl_1_data_494);
    mbCameraZoomSet(lbl_1_data_4A0);
    mbWipeFadeIn();
    mbTelopTimeChangeCreate();
    while (mbTelopTimeChangeCheck() != 0) {
        HuPrcVSleep();
    }
    mbWipeFadeOut();
    HuPrcVSleep();
    mbCameraCenterSetV(&lbl_1_data_3A8);
    mbCameraRotSetV(&lbl_1_data_3B4);
    mbCameraZoomSet(lbl_1_data_3C0);
    mbCameraCenterSetV(&lbl_1_data_418);
    mbCameraRotSetV(&lbl_1_data_424);
    mbCameraZoomSet(lbl_1_data_430);
    if (board->skipRequested == 0) {
        mbCameraMovePos(&lbl_1_data_434, &lbl_1_data_440, NULL, lbl_1_data_44C,
                       -1.0f, 160);
    }
    window = mbWinCreateHelp(MESSNUM(MESS_BOARD_OPE, 12));
    mbWinPosGet(window, &windowPosition);
    windowPosition.y = 408.0f;
    mbWinPosSet(window, windowPosition.x, windowPosition.y);
    board->skipEnabled = 1;
    if (board->skipRequested == 0) {
        mbWipeFadeIn();
    }
    for (frame = 0; frame < 60.0f; frame++) {
        fn_1_285C();
    }
    for (frame = 0; frame < 60.0f; frame++) {
        fn_1_285C();
    }
    if (board->skipRequested == 0) {
        mbWipeFadeOut();
    }
    mbWipeWait();
    while (board->skipRequested == 0 && mbCameraMoveCheck() == 0) {
        fn_1_285C();
    }
    mbCameraEyeSetV(&lbl_1_data_450);
    mbCameraCenterSetV(&lbl_1_data_450);
    mbCameraRotSetV(&lbl_1_data_45C);
    mbCameraZoomSet(lbl_1_data_468);
    mbCameraMoveOnSet(0);
    fn_1_285C();
    mbCameraMoveOnSet(1);
    if (board->skipRequested == 0) {
        mbCameraMovePos(&lbl_1_data_46C, &lbl_1_data_478, NULL, lbl_1_data_484,
                       -1.0f, 300);
    }
    if (board->skipRequested == 0) {
        mbWipeFadeIn();
    }
    for (frame = 0; frame < 120.0f; frame++) {
        fn_1_285C();
    }
    if (board->skipRequested == 0) {
        mbWipeFadeOut();
    }
    mbWipeWait();
    fn_1_285C();
    mbWipeWait();
    for (frame = 0; frame < 4; frame++) {
        mbPlayerDispSet(frame, 1);
    }
    board->skipEnabled = 0;
    mbWinKill(window);
}

/* Sleeps until skipping is enabled and requested; fades out once both wipe checks are clear. */
void fn_1_285C(void) {
    W05_BOARD_WORK *state;

    state = lbl_1_bss_8;
    if (state->skipRequested != 0
        && state->skipEnabled != 0) {
        if (mbWipeSpecialCheck() == 0
            && mbWipeSpecialStatGet() == 0) {
            mbWipeFadeOut();
        }
    } else {
        HuPrcVSleep();
    }
}

const s8 lbl_1_rodata_5C[4] = {30, 20, 10};

/* Chooses a linked space near the day target or far from the night target, with a COM detour. */
s32 fn_1_28C8(s32 playerNo, s32 linkCount, s16 *links, s32 forceNearest) {
    void *boardSnapshot[1];
    s32 selectedLink;
    s32 closestDistance;
    s32 farthestDistance;
    s32 targetType;
    s32 distance;
    s32 linkIndex;

    boardSnapshot[0] = lbl_1_bss_8;
    selectedLink = -1;
    closestDistance = 9999;
    farthestDistance = 0;
    switch (*(u8 *)lbl_1_bss_4) {
    case 0:
        targetType = 7;
        break;
    default:
        targetType = 10;
        break;
    }
    if (forceNearest != 0) {
        for (linkIndex = 0; linkIndex < linkCount; linkIndex++) {
            distance = mbMasuFind_TypeStepGet2(links[linkIndex], targetType, TRUE, TRUE);
            if (distance < closestDistance) {
                selectedLink = linkIndex;
                closestDistance = distance;
            }
        }
        return selectedLink;
    }
    if (targetType == 7) {
        for (linkIndex = 0; linkIndex < linkCount; linkIndex++) {
            distance = mbMasuFind_TypeStepGet2(links[linkIndex], targetType, TRUE, TRUE);
            if (distance < closestDistance) {
                selectedLink = linkIndex;
                closestDistance = distance;
            }
        }
        if (selectedLink < 0 ||
            (closestDistance > 20 &&
             mbRandMod(100) < lbl_1_rodata_5C[GwPlayer[playerNo].comDif])) {
            linkIndex = 0;
            while (1) {
                if (mbMasuGet(links[linkIndex])->linkNum != 0 && mbRandMod(1000) < 500) {
                    break;
                }
                linkIndex++;
                if (linkIndex >= linkCount) {
                    linkIndex = 0;
                }
            }
            selectedLink = linkIndex;
        }
        return selectedLink;
    }
    for (linkIndex = 0; linkIndex < linkCount; linkIndex++) {
        distance = mbMasuFind_TypeStepGet2(links[linkIndex], targetType, TRUE, TRUE);
        if (distance > farthestDistance) {
            selectedLink = linkIndex;
            farthestDistance = distance;
        }
    }
    if (selectedLink < 0 ||
        (farthestDistance > 20 &&
         mbRandMod(100) < lbl_1_rodata_5C[GwPlayer[playerNo].comDif])) {
        linkIndex = 0;
        while (1) {
            if (mbMasuGet(links[linkIndex])->linkNum != 0 && mbRandMod(1000) < 500) {
                break;
            }
            linkIndex++;
            if (linkIndex >= linkCount) {
                linkIndex = 0;
            }
        }
        selectedLink = linkIndex;
    }
    return selectedLink;
}

void fn_1_2B50(void) {
}

/* Creates the hook machinery and their linked attachments during board setup. */
void fn_1_2B54(void) {
    W05_BOARD_WORK *board = lbl_1_bss_8;
    int modelId;

    modelId = board->hookModels[0] =
        mbObjCreate(W05BoardDataNumGet(DATANUM(DATA_w05, 32)), NULL, FALSE);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjMotionTimeSet(modelId, 0.0f);
    modelId = board->hookModels[1] =
        mbObjCreate(W05BoardDataNumGet(DATANUM(DATA_w05, 33)), NULL, FALSE);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjMotionTimeSet(modelId, 0.0f);
    modelId = board->firstHookModel =
        mbObjCreate(W05BoardDataNumGet(DATANUM(DATA_w05, 24)), NULL, FALSE);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjMotionTimeSet(modelId, 0.0f);
    modelId = board->firstHookAttachmentModel =
        mbObjCreate(W05BoardDataNumGet(DATANUM(DATA_w05, 25)), NULL, FALSE);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjHookSet(board->firstHookModel, "haghook", modelId);
    modelId = board->secondHookModel =
        mbObjCreate(W05BoardDataNumGet(DATANUM(DATA_w05, 26)), NULL, FALSE);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjMotionTimeSet(modelId, 0.0f);
    modelId = board->secondHookModels[0] =
        mbObjCreate(W05BoardDataNumGet(DATANUM(DATA_w05, 27)), NULL, FALSE);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjMotionTimeSet(modelId, mbObjMotionMaxTimeGet(modelId) / 2.0f);
    mbObjHookSet(board->secondHookModel, "daihook", modelId);
    modelId = board->secondHookModels[1] =
        mbObjCreate(W05BoardDataNumGet(DATANUM(DATA_w05, 28)), NULL, FALSE);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjMotionTimeSet(modelId, mbObjMotionMaxTimeGet(modelId) / 3.0f);
    mbObjHookSet(board->secondHookModels[0], "canonhook", modelId);
    modelId = board->secondHookModels[2] =
        mbObjCreate(W05BoardDataNumGet(DATANUM(DATA_w05, 29)), NULL, FALSE);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjMotionTimeSet(modelId, 0.0f);
    mbObjHookSet(board->secondHookModel, "yukahook", modelId);
    modelId = board->secondHookAttachmentModel =
        mbObjCreate(W05BoardDataNumGet(DATANUM(DATA_w05, 30)), NULL, FALSE);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjMotionTimeSet(modelId, 0.0f);
    mbObjHookSet(board->secondHookModels[1], "canonhook2", modelId);
    modelId = board->risingEffectModel =
        mbObjCreate(W05BoardDataNumGet(DATANUM(DATA_w05, 31)), NULL, FALSE);
    mbObjDispSet(board->risingEffectModel, FALSE);
    mbObjLayerSet(board->risingEffectModel, 3);
    fn_1_3308(1);
    fn_1_3540();
}

/* Reattaches the hook machinery for the current time of day during board setup or turn close. */
void fn_1_3308(s32 hookIndex) {
    W05_BOARD_WORK *board = lbl_1_bss_8;

    mbObjHookReset(board->hookModels[0]);
    mbObjHookReset(board->hookModels[1]);
    board->firstHookIndex = -1;
    board->secondHookIndex = -1;
    switch (*lbl_1_bss_4) {
    case 0:
        mbObjMotionSpeedSet(board->hookModels[hookIndex], 0.0f);
        mbObjMotionTimeSet(board->hookModels[hookIndex], 0.0f);
        mbObjHookSet(board->hookModels[hookIndex], lbl_1_data_40[hookIndex][1],
                     board->firstHookModel);
        mbObjHookSet(board->hookModels[hookIndex], lbl_1_data_40[hookIndex][0],
                     board->secondHookModel);
        break;
    default:
    case 1:
        mbObjMotionSpeedSet(board->hookModels[hookIndex], 0.0f);
        mbObjMotionTimeSet(board->hookModels[hookIndex], 0.0f);
        mbObjHookSet(board->hookModels[hookIndex], lbl_1_data_40[hookIndex][0],
                     board->firstHookModel);
        mbObjHookSet(board->hookModels[hookIndex], lbl_1_data_40[hookIndex][1],
                     board->secondHookModel);
        break;
    }
    if (board->eventEnabled != 0) {
        if (*lbl_1_bss_4 == 0) {
            mbObjMotionTimeSet(board->eventModel, 0.0f);
        } else {
            mbObjMotionTimeSet(board->eventModel,
                               mbObjMotionMaxTimeGet(board->eventModel) / 2.0f);
        }
    }
}

/* Marks the board's existing special space as the day or night event during setup and turn
 * close. */
void fn_1_3540(void) {
    void *boardWorkSnapshot[1];
    s32 space;
    s32 selectedSpace;

    boardWorkSnapshot[0] = (void *)lbl_1_bss_8;
    space = 1;
    while (space < mbMasuNumGet()) {
        if (mbMasuTypeGet((s16)space) == 7 || mbMasuTypeGet((s16)space) == 10) {
            break;
        }
        space++;
    }

    selectedSpace = space;
    switch ((s32)*(u8 *)lbl_1_bss_4) {
    case 0:
        mbMasuTypeSet((s16)selectedSpace, 7);
        break;
    default:
        mbMasuTypeSet((s16)selectedSpace, 10);
        break;
    }
}

HuVecF lbl_1_data_4D4 = {16.0f, 294.0f, 2384.0f};
HuVecF lbl_1_data_4E0 = {-35.0f, 0.0f, 0.0f};
HuVecF lbl_1_data_4EC = {0.0f, 0.0f, 1.0f};

/* Moves the hooks between day and night poses while hiding the players and changing the camera. */
void fn_1_35F0(s32 fadeEnabled, s32 specialFade, s32 explainTransition) {
    W05_BOARD_WORK *board;
    s32 cameraModel;
    s32 playerNo;
    s32 soundId;
    f32 motionMaxTime;
    f32 phaseWeight;

    board = lbl_1_bss_8;
    mbCameraStackPush();
    if (fadeEnabled != 0) {
        if (specialFade != 0) {
            mbWipeSpecialFadeInCreate(1, 1);
        } else {
            mbWipeDissolveFadeOutTime(1);
        }
    }
    mbCameraMoveWait();
    cameraModel = mbObjCreate(DATANUM(DATA_capsule, 68), NULL, 0);
    mbObjDispSet(cameraModel, 0);
    mbObjPosSetV(cameraModel, &lbl_1_data_4D4);
    mbCameraMoveObj(cameraModel, &lbl_1_data_4E0, NULL, 5482.0f, -1.0f, -1);
    mbCameraMoveWait();
    for (playerNo = 0; playerNo < 4; playerNo++) {
        mbPlayerDispSet(playerNo, 0);
    }
    switch (*lbl_1_bss_4) {
    case 0:
        fn_1_3308(0);
        if (specialFade != 0) {
            mbWipeSpecialFadeOutCreate(1, 60);
        } else {
            mbWipeDissolveFadeInTime(60);
        }
        if (explainTransition != 0) {
            mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 49), mbGuideSpeakerNoGet());
            mbWinTopWait();
        }
        soundId = mbAudFXPlay(1511);
        mbObjMotionSpeedSet(board->hookModels[0], 1.0f);
        mbObjMotionTimeSet(board->hookModels[0], 0.0f);
        mbObjMotionTimeSet(board->firstHookAttachmentModel, 0.0f);
        mbObjMotionSpeedSet(board->firstHookAttachmentModel, 1.0f);
        board->firstHookMoving = 1;
        board->secondHookMoving = 1;
        board->hookSide = 1;
        while (mbObjMotionEndCheck(board->hookModels[0]) == 0) {
            motionMaxTime = mbObjMotionMaxTimeGet(board->hookModels[0]);
            phaseWeight = mbObjMotionTimeGet(board->hookModels[0]) / motionMaxTime;
            phaseWeight = (f32)sin(M_PI * (180.0f * phaseWeight) / 180.0);
            board->firstHookPhaseWeight = phaseWeight;
            board->secondHookPhaseWeight = phaseWeight;
            HuPrcVSleep();
        }
        mbAudFXStop(soundId);
        board->firstHookMoving = 0;
        board->secondHookMoving = 0;
        board->hookSide = 0;
        if (explainTransition != 0) {
            mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 50), mbGuideSpeakerNoGet());
            mbWinTopWait();
        }
        if (specialFade != 0) {
            mbWipeSpecialFadeInCreate(1, 1);
        } else {
            mbWipeDissolveFadeOutTime(1);
        }
        if (explainTransition == 0) {
            *lbl_1_bss_4 = 1;
        }
        fn_1_3308(1);
        fn_1_3540();
        break;
    case 1:
    default:
        *lbl_1_bss_4 = 1;
        fn_1_3308(0);
        if (specialFade != 0) {
            mbWipeSpecialFadeOutCreate(1, 60);
        } else {
            mbWipeDissolveFadeInTime(60);
        }
        soundId = mbAudFXPlay(1511);
        mbObjMotionSpeedSet(board->hookModels[0], 1.0f);
        mbObjMotionTimeSet(board->hookModels[0], 0.0f);
        board->firstHookMoving = 1;
        board->secondHookMoving = 1;
        board->hookSide = 1;
        mbObjMotionTimeSet(board->firstHookAttachmentModel, 0.0f);
        mbObjMotionSpeedSet(board->firstHookAttachmentModel, 1.0f);
        while (mbObjMotionEndCheck(board->hookModels[0]) == 0) {
            motionMaxTime = mbObjMotionMaxTimeGet(board->hookModels[0]);
            phaseWeight = mbObjMotionTimeGet(board->hookModels[0]) / motionMaxTime;
            phaseWeight = (f32)sin(M_PI * (180.0f * phaseWeight) / 180.0);
            board->firstHookPhaseWeight = phaseWeight;
            board->secondHookPhaseWeight = phaseWeight;
            HuPrcVSleep();
        }
        mbAudFXStop(soundId);
        board->firstHookMoving = 0;
        board->secondHookMoving = 0;
        board->hookSide = 0;
        if (specialFade != 0) {
            mbWipeSpecialFadeInCreate(1, 1);
        } else {
            mbWipeDissolveFadeOutTime(1);
        }
        *lbl_1_bss_4 = 0;
        fn_1_3308(1);
        fn_1_3540();
        break;
    }
    mbCameraStackPop(1);
    if (explainTransition == 0) {
        mbCameraPlayerViewSet(GwSystem.turnPlayerNo, 0);
        mbCameraMoveWait();
    }
    mbObjKill(cameraModel);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        mbPlayerDispSet(playerNo, 1);
    }
    if (fadeEnabled != 0) {
        if (specialFade != 0) {
            mbWipeSpecialFadeOutCreate(1, 60);
            return;
        }
        mbWipeDissolveFadeInTime(60);
    }
}

/* Sends the current player's star event to the active hook and closes its night data. */
void fn_1_42B4(s32 argument) {
    void *moduleWork = lbl_1_bss_8;

    switch (*lbl_1_bss_4) {
    case 0:
        fn_1_433C(argument);
        break;
    case 1:
        fn_1_5900(argument);
        break;
    default:
        fn_1_5900(argument);
        break;
    }
    HuDataDirClose(DATA_capsulechar1);
}

/* Runs the first hook's star purchase sequence and carries the player through its motion. */
void fn_1_433C(int playerNo) {
    int motionData[16];
    HuVecF playerPosition;
    HuVecF playerHookPosition;
    HuVecF guideHookPosition;
    HuVecF direction;
    HuVecF guideRotation;
    HuVecF position;
    int readId;
    int soundId;
    float facingAngle;
    float guideFacingAngle;
    W05_BOARD_WORK *board;
    int guideModel;
    s32 frame;
    int destinationSpace;
    int previousSpace;
    float motionFrames;
    float phase;
    u32 choiceMessage;
    u32 purchaseMessage;
    u32 declineMessage;
    u32 coinMessage;
    u32 departureMessage;
    u32 farewellMessage;

    board = lbl_1_bss_8;
    mbMoveNumDispSet(playerNo, FALSE);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    if ((readId = mbBGRead(DATANUM(DATA_capsulechar1, 0))) != -1) {
        mbBGReadWait(readId);
    }
    motionData[0] = DATANUM(DATA_capsulechar1, 15);
    motionData[1] = DATANUM(DATA_capsulechar1, 19);
    motionData[2] = DATANUM(DATA_capsulechar1, 20);
    motionData[3] = DATANUM(DATA_capsulechar1, 24);
    motionData[4] = DATANUM(DATA_capsulechar1, 25);
    motionData[5] = DATANUM(DATA_capsulechar1, 26);
    motionData[6] = -1;
    guideModel = mbObjCreate(DATANUM(DATA_capsulechar1, 14), motionData, FALSE);
    mbObjMotionSet(guideModel, 1, HU3D_MOTATTR_LOOP);
    mbObjDispSet(guideModel, FALSE);
    mbObjLayerSet(guideModel, 3);
    HuPrcVSleep();
    mbPlayerPosGet(playerNo, &playerPosition);
    fn_1_76A4(0, &playerHookPosition, NULL, NULL);
    fn_1_76A4(1, &guideHookPosition, NULL, NULL);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    PSVECSubtract(&playerHookPosition, &playerPosition, &direction);
    mbPlayerRotSet(playerNo, 0.0f,
        180.0 * (atan2(direction.x, direction.z) / M_PI), 0.0f);
    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, 0);
    for (frame = 0; frame <= 30.0f; frame++) {
        phase = frame / 30.0f;
        fn_1_76A4(0, &playerHookPosition, NULL, NULL);
        position.x = playerPosition.x + phase * (playerHookPosition.x - playerPosition.x);
        position.y = playerPosition.y + phase * (playerHookPosition.y - playerPosition.y) +
                     200.0 * sin(M_PI * (180.0f * phase) / 180.0);
        position.z = playerPosition.z + phase * (playerHookPosition.z - playerPosition.z);
        mbPlayerPosSetV(playerNo, &position);
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_80AC(playerNo, 0);
    board->hookMovementScale = 1.0f;
    mbMusBoardFadeOut(0, 0, 1000, 1000, 30, FALSE);
    PSVECSubtract(&guideHookPosition, &playerHookPosition, &direction);
    mbPlayerRotateStart(playerNo, 180.0 * (atan2(direction.x, direction.z) / M_PI), 15);
    while (mbPlayerRotateCheck(playerNo) == 0) {
        HuPrcVSleep();
    }
    mbCameraPlayerViewSet(GwSystem.turnPlayerNo, 1);
    mbCameraMoveWait();
    mbAudFXPlay(1512);
    mbObjDispSet(guideModel, TRUE);
    for (frame = 0; frame <= mbObjMotionMaxTimeGet(board->firstHookModel); frame++) {
        mbObjMotionTimeSet(board->firstHookModel, frame);
        fn_1_76A4(0, &playerHookPosition, NULL, NULL);
        fn_1_76A4(1, &guideHookPosition, NULL, NULL);
        PSVECSubtract(&playerHookPosition, &guideHookPosition, &direction);
        mbPlayerPosSetV(playerNo, &playerHookPosition);
        mbObjPosSetV(guideModel, &guideHookPosition);
        mbObjRotSet(guideModel, 0.0f,
            180.0 * (atan2(direction.x, direction.z) / M_PI), 0.0f);
        HuPrcVSleep();
    }
    fn_1_80D4(guideModel, 1);
    mbAudFXDelaySet(30);
    mbAudFXPlay(936);
    mbev_CapPlayerMotShiftSet(guideModel, 2, 0, TRUE);
    mbev_CapPlayerMotShiftSet(guideModel, 1, HU3D_MOTATTR_LOOP, TRUE);
    if (mbPlayerCoinGet(playerNo) >= 20) {
        if (GwSystem.curTime == 0) {
            choiceMessage = MESSNUM(MESS_BOARD_W05, 0);
        } else {
            choiceMessage = MESSNUM(MESS_BOARD_W05, 6);
        }
        mbWinCreateChoice(1, choiceMessage, mbGuideSpeakerNoGet(), 0);
        if (GwPlayer[playerNo].comF) {
            mbComChoiceLeftSet();
        }
        mbWinTopWait();
        if (mbWinTopChoiceGet() == 0) {
            mbAudFXDelaySet(30);
            mbAudFXPlay(935);
            if (GwSystem.curTime == 0) {
                purchaseMessage = MESSNUM(MESS_BOARD_W05, 1);
            } else {
                purchaseMessage = MESSNUM(MESS_BOARD_W05, 7);
            }
            mbWinCreate(2, purchaseMessage, mbGuideSpeakerNoGet());
            mbWinTopWait();
            mbPlayerRotateStart(playerNo, 0, 15);
            while (mbPlayerRotateCheck(playerNo) == 0) {
                HuPrcVSleep();
            }
            mbCoinAddDispExec(playerNo, -20, TRUE, FALSE);
            mbAudFXDelaySet(30);
            mbAudFXPlay(935);
            mbev_CapObjMotionSet(guideModel, 30, 6, 1, 0, HU3D_MOTATTR_LOOP, TRUE, TRUE);
            mbStarGetExec(playerNo);
            mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbMusPlay(0, 30, 127, 0);
        } else {
            mbAudFXDelaySet(30);
            mbAudFXPlay(937);
            mbObjMotionShiftSet(guideModel, 5, 0.0f, 8.0f, 0);
            if (GwSystem.curTime == 0) {
                declineMessage = MESSNUM(MESS_BOARD_W05, 3);
            } else {
                declineMessage = MESSNUM(MESS_BOARD_W05, 9);
            }
            mbWinCreate(2, declineMessage, mbGuideSpeakerNoGet());
            mbWinTopWait();
        }
    } else {
        mbAudFXDelaySet(30);
        mbAudFXPlay(937);
        mbObjMotionShiftSet(guideModel, 3, 0.0f, 8.0f, 0);
        if (GwSystem.curTime == 0) {
            coinMessage = MESSNUM(MESS_BOARD_W05, 2);
        } else {
            coinMessage = MESSNUM(MESS_BOARD_W05, 8);
        }
        mbWinCreate(2, coinMessage, mbGuideSpeakerNoGet());
        mbWinTopWait();
    }
    mbObjMotionShiftSet(guideModel, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    if (GwSystem.curTime == 0) {
        departureMessage = MESSNUM(MESS_BOARD_W05, 4);
    } else {
        departureMessage = MESSNUM(MESS_BOARD_W05, 10);
    }
    mbWinCreate(2, departureMessage, mbGuideSpeakerNoGet());
    mbWinTopWait();
    for (frame = 0; frame < GW_PLAYER_MAX; frame++) {
        if (frame != playerNo) {
            mbPlayerDispSet(frame, FALSE);
        }
    }
    mbCameraPlayerViewSet(GwSystem.turnPlayerNo, 2);
    mbCameraMoveWait();
    mbev_CapPlayerMotShiftSet(guideModel, 4, 0, TRUE);
    mbObjMotionShiftSet(guideModel, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbObjMotionSpeedSet(board->hookModels[1], 0.0f);
    mbObjMotionTimeSet(board->hookModels[1], 0.0f);
    mbObjHookSet(board->hookModels[1], lbl_1_data_40[1][1], board->firstHookModel);
    PSVECSubtract(&playerHookPosition, &guideHookPosition, &direction);
    mbPlayerRotateStart(playerNo, 180.0 * (atan2(direction.x, direction.z) / M_PI), 15);
    while (mbPlayerRotateCheck(playerNo) == 0) {
        HuPrcVSleep();
    }
    mbObjMotionTimeSet(board->firstHookAttachmentModel, 0.0f);
    mbObjMotionSpeedSet(board->firstHookAttachmentModel, 1.0f);
    board->firstHookMoving = 1;
    board->secondHookMoving = 1;
    soundId = mbAudFXPlay(1511);
    motionFrames = 2.0f * (mbObjMotionMaxTimeGet(board->hookModels[1]) / 5.0f);
    omVibrate(playerNo, 240, 4, 4);
    for (frame = 0; frame <= motionFrames; frame++) {
        board->firstHookPhaseWeight = sin(M_PI * (180.0f * (frame / motionFrames)) / 180.0);
        board->secondHookPhaseWeight = 0.0f;
        mbObjMotionTimeSet(board->hookModels[1], frame);
        fn_1_76A4(0, &playerHookPosition, NULL, NULL);
        fn_1_76A4(1, &guideHookPosition, NULL, NULL);
        PSVECSubtract(&playerHookPosition, &guideHookPosition, &direction);
        mbPlayerPosSetV(playerNo, &playerHookPosition);
        mbPlayerRotSet(playerNo, 0.0f,
            180.0 * (atan2(direction.x, direction.z) / M_PI), 0.0f);
        mbObjPosSetV(guideModel, &guideHookPosition);
        mbObjRotSet(guideModel, 0.0f,
            180.0 * (atan2(direction.x, direction.z) / M_PI), 0.0f);
        HuPrcVSleep();
    }
    mbObjMotionSpeedSet(board->hookModels[1], 0.0f);
    mbAudFXStop(soundId);
    board->firstHookMoving = 0;
    board->secondHookMoving = 0;
    for (frame = 1; frame < mbMasuNumGet(); frame++) {
        if (mbMasuAttrGet(frame) == (1 << 15)) {
            destinationSpace = frame;
            break;
        }
    }
    if (frame >= mbMasuNumGet()) {
        destinationSpace = GwPlayer[playerNo].masuId;
    }
    previousSpace = mbev_CapMasuPrevGet(destinationSpace, &playerHookPosition);
    mbPlayerPosGet(playerNo, &playerPosition);
    mbev_PlayerColMasuSet(playerNo, previousSpace, FALSE);
    fn_1_80AC(-1, 0);
    PSVECSubtract(&playerHookPosition, &playerPosition, &direction);
    mbPlayerRotSet(playerNo, 0.0f,
        180.0 * (atan2(direction.x, direction.z) / M_PI), 0.0f);
    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, 0);
    for (frame = 0; frame <= 30.0f; frame++) {
        phase = frame / 30.0f;
        position.x = playerPosition.x + phase * (playerHookPosition.x - playerPosition.x);
        position.y = playerPosition.y + phase * (playerHookPosition.y - playerPosition.y) +
                     200.0 * sin(M_PI * (180.0f * phase) / 180.0);
        position.z = playerPosition.z + phase * (playerHookPosition.z - playerPosition.z);
        mbPlayerPosSetV(playerNo, &position);
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    GwPlayer[playerNo].masuId = previousSpace;
    mbPlayerPosGet(playerNo, &playerPosition);
    mbMasuPosGet(destinationSpace, &playerHookPosition);
    PSVECSubtract(&playerHookPosition, &playerPosition, &direction);
    mbPlayerRotSet(playerNo, 0.0f,
        180.0 * (atan2(direction.x, direction.z) / M_PI), 0.0f);
    mbPlayerMotionShiftSet(playerNo, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    for (frame = 1; frame <= 30.0f; frame++) {
        phase = frame / 30.0f;
        position.x = playerPosition.x + phase * (playerHookPosition.x - playerPosition.x);
        position.y = playerPosition.y + phase * (playerHookPosition.y - playerPosition.y);
        position.z = playerPosition.z + phase * (playerHookPosition.z - playerPosition.z);
        mbPlayerPosSetV(playerNo, &position);
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    GwPlayer[playerNo].masuId = destinationSpace;
    mbPlayerColSnapPlayerSet(playerNo, TRUE);
    mbPlayerRotateStart(playerNo, 0, 15);
    while (mbPlayerRotateCheck(playerNo) == 0) {
        HuPrcVSleep();
    }
    fn_1_76A4(1, &guideHookPosition, &guideRotation, NULL);
    mbPlayerRotGet(playerNo, &playerHookPosition);
    PSVECSubtract(&playerHookPosition, &guideHookPosition, &direction);
    guideFacingAngle = guideRotation.y;
    facingAngle = 180.0 * (atan2(direction.x, direction.z) / M_PI);
    /* The computed facing angle is ignored; both interpolation endpoints use the guide's angle. */
    for (frame = 0; frame <= 20; frame++) {
        mbObjRotSet(guideModel, 0.0f,
            mbev_CapAngleSumLerp(frame / 20.0f, guideFacingAngle, guideFacingAngle), 0.0f);
        HuPrcVSleep();
    }
    mbAudFXPlay(936);
    mbObjMotionShiftSet(guideModel, 6, 0.0f, 8.0f, 0);
    if (GwSystem.curTime == 0) {
        farewellMessage = MESSNUM(MESS_BOARD_W05, 5);
    } else {
        farewellMessage = MESSNUM(MESS_BOARD_W05, 11);
    }
    mbWinCreate(2, farewellMessage, mbGuideSpeakerNoGet());
    mbWinTopWait();
    mbMusBoardFadeOut(0, 0, 1000, 1000, -1, FALSE);
    mbWipeSpecialFadeInCreate(3, 1);
    mbObjMotionTimeSet(board->firstHookModel, 0.0f);
    mbObjMotionSpeedSet(board->firstHookModel, 0.0f);
    fn_1_80D4(-1, 0);
    *lbl_1_bss_4 = 2;
    fn_1_3308(1);
    fn_1_3540();
    for (frame = 0; frame < GW_PLAYER_MAX; frame++) {
        mbPlayerDispSet(frame, TRUE);
    }
    mbev_PlayerColMasu(playerNo, GwPlayer[playerNo].masuId, TRUE);
    mbObjKill(guideModel);
    mbWipeSpecialFadeOutCreate(3, 60);
    mbMoveNumDispSet(playerNo, TRUE);
}

/* Runs the second hook's Ztar event, then returns the player to the marked board space. */
void fn_1_5900(int playerNo) {
    Mtx rotationMatrix;
    int motionData[16];
    HuVecF playerPosition;
    HuVecF playerHookPosition;
    HuVecF guideHookPosition;
    HuVecF direction;
    HuVecF rotation;
    HuVecF curveControl;
    HuVecF previousPosition;
    HuVecF position;
    HuVecF puffPosition;
    HuVecF puffRotation;
    GXColor color;
    W05_BOARD_WORK *board;
    OMOBJ *playerObject;
    int guideModel;
    int cameraModel;
    int destinationSpace;
    int readId;
    int soundId;
    int frame;
    float phase;
    float motionFrames;

    board = lbl_1_bss_8;
    mbMoveNumDispSet(playerNo, FALSE);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    if ((readId = mbBGRead(DATANUM(DATA_capsulechar1, 0))) != -1) {
        mbBGReadWait(readId);
    }
    motionData[0] = DATANUM(DATA_capsulechar1, 1);
    motionData[1] = DATANUM(DATA_capsulechar1, 3);
    motionData[2] = DATANUM(DATA_capsulechar1, 4);
    motionData[3] = DATANUM(DATA_capsulechar1, 8);
    motionData[4] = DATANUM(DATA_capsulechar1, 10);
    motionData[5] = -1;
    guideModel = mbObjCreate(DATANUM(DATA_capsulechar1, 0), motionData, FALSE);
    mbObjMotionSet(guideModel, 1, HU3D_MOTATTR_LOOP);
    mbObjDispSet(guideModel, FALSE);
    mbObjLayerSet(guideModel, 3);
    HuPrcVSleep();
    mbPlayerPosGet(playerNo, &playerPosition);
    fn_1_7AF8(0, &playerHookPosition, NULL, NULL);
    fn_1_7AF8(1, &guideHookPosition, NULL, NULL);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    PSVECSubtract(&playerHookPosition, &playerPosition, &direction);
    mbPlayerRotSet(playerNo, 0.0f,
        180.0 * (atan2(direction.x, direction.z) / M_PI), 0.0f);
    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, 0);
    for (frame = 0; frame <= 30.0f; frame++) {
        phase = frame / 30.0f;
        fn_1_7AF8(0, &playerHookPosition, NULL, NULL);
        position.x = playerPosition.x + phase * (playerHookPosition.x - playerPosition.x);
        position.y = playerPosition.y + phase * (playerHookPosition.y - playerPosition.y) +
            200.0 * sin(M_PI * (180.0f * phase) / 180.0);
        position.z = playerPosition.z + phase * (playerHookPosition.z - playerPosition.z);
        mbPlayerPosSetV(playerNo, &position);
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_80FC(playerNo, 0);
    board->secondHookSwing = 1.0f;
    mbMusBoardFadeOut(0, 0, 1000, 1000, 28, FALSE);
    cameraModel = mbObjCreate(DATANUM(DATA_capsule, 68), NULL, FALSE);
    mbObjDispSet(cameraModel, FALSE);
    mbObjPosSetV(cameraModel, &playerHookPosition);
    position.x = position.z = 0.0f;
    position.y = 100.0f;
    mbCameraMoveObj(cameraModel, NULL, &position, 2000.0f, -1.0f, 12);
    mbCameraMoveWait();
    PSVECSubtract(&guideHookPosition, &playerHookPosition, &direction);
    mbev_CapPlayerRotate(playerNo, 180.0 * (atan2(direction.x, direction.z) / M_PI));
    mbAudFXPlay(1512);
    mbObjDispSet(guideModel, TRUE);
    for (frame = 0; frame <= mbObjMotionMaxTimeGet(board->secondHookModel); frame++) {
        mbObjMotionTimeSet(board->secondHookModel, frame);
        fn_1_7AF8(0, &playerHookPosition, NULL, NULL);
        fn_1_7AF8(1, &guideHookPosition, NULL, NULL);
        PSVECSubtract(&playerHookPosition, &guideHookPosition, &direction);
        mbPlayerPosSetV(playerNo, &playerHookPosition);
        mbObjPosSetV(guideModel, &guideHookPosition);
        mbObjRotSet(guideModel, 0.0f,
            180.0 * (atan2(direction.x, direction.z) / M_PI), 0.0f);
        HuPrcVSleep();
    }
    fn_1_8124(guideModel, 1);
    mbAudFXDelaySet(30);
    mbAudFXPlay(971);
    if (mbPlayerStarGet(playerNo) >= 1) {
        mbev_CapPlayerMotShiftSet(guideModel, 2, 0, TRUE);
        mbObjMotionShiftSet(guideModel, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 12), 13);
        mbWinTopWait();
    } else {
        mbev_CapPlayerMotShiftSet(guideModel, 2, 0, TRUE);
        mbObjMotionShiftSet(guideModel, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 13), 13);
        mbWinTopWait();
        mbAudFXDelaySet(30);
        mbAudFXPlay(972);
        mbObjMotionShiftSet(guideModel, 5, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 14), 13);
        mbWinTopWait();
        mbAudFXDelaySet(30);
        mbAudFXPlay(971);
        mbObjMotionShiftSet(guideModel, 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 16), 13);
        mbWinTopWait();
    }
    mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 15), 13);
    mbWinTopWait();
    mbAudFXDelaySet(30);
    mbAudFXPlay(973);
    mbObjMotionShiftSet(guideModel, 3, 0.0f, 8.0f, 0);
    mbZtarGetExec(playerNo);
    mbMusPlay(0, 28, 127, 0);
    mbAudFXDelaySet(30);
    mbAudFXPlay(971);
    mbObjMotionShiftSet(guideModel, 2, 0.0f, 8.0f, 0);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 17), 13);
    mbWinTopWait();
    mbAudFXDelaySet(30);
    mbAudFXPlay(973);
    mbev_CapPlayerMotShiftSet(guideModel, 3, 0, TRUE);
    mbPlayerMotionShiftSet(playerNo, 9, 0.0f, 8.0f, 0);
    HuPrcSleep(6);
    mbAudFXPlay(1523);
    mbObjMotionTimeSet(board->secondHookModels[2], 0.0f);
    mbObjMotionSpeedSet(board->secondHookModels[2], 1.0f);
    if (GwPlayer[playerNo].metalF) {
        mbPlayerEffectSet(playerNo, FALSE);
    }
    do {
        fn_1_7AF8(0, &playerHookPosition, &rotation, NULL);
        mbPlayerPosSetV(playerNo, &playerHookPosition);
        mbPlayerRotSetV(playerNo, &rotation);
        HuPrcVSleep();
    } while (mbObjMotionTimeGet(board->secondHookModels[2]) <
             mbObjMotionMaxTimeGet(board->secondHookModels[2]));
    mbObjMotionShiftSet(guideModel, 2, 0.0f, 8.0f, 0);
    mbObjPosGet(cameraModel, &playerPosition);
    playerHookPosition.x = 100.0f + playerPosition.x;
    playerHookPosition.y = 300.0f + playerPosition.y;
    playerHookPosition.z = playerPosition.z - 300.0f;
    for (frame = 1; frame < 60.0f; frame++) {
        phase = frame / 60.0f;
        mbev_CapVecChase(phase, &playerPosition, &playerHookPosition, &position);
        mbObjPosSetV(cameraModel, &position);
        HuPrcVSleep();
    }
    mbCameraMoveWait();
    HuPrcSleep(60);
    mbObjMotionShiftSet(guideModel, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_80FC(-1, 0);
    mbPlayerMotionSet(playerNo, 1, 0);
    playerObject = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_814C);
    playerObject->work[0] = 0;
    playerObject->work[1] = 0;
    playerObject->work[2] = 0;
    playerObject->work[3] = playerNo;
    HuPrcSleep(30);
    soundId = mbAudFXPlay(1513);
    motionFrames = mbObjMotionMaxTimeGet(board->secondHookModels[0]);
    for (frame = 1; frame <= 105.0f; frame++) {
        phase = frame / 105.0f;
        mbObjMotionTimeSet(board->secondHookModels[0], motionFrames / 2.0f +
            (motionFrames / 5.0f) * sin(M_PI * (360.0f * phase) / 180.0));
        HuPrcVSleep();
    }
    if (soundId != 0) {
        mbAudFXStop(soundId);
    }
    HuPrcSleep(6);
    soundId = mbAudFXPlay(1513);
    motionFrames = mbObjMotionMaxTimeGet(board->secondHookModels[1]);
    for (frame = 1; frame <= 120.0f; frame++) {
        phase = frame / 120.0f;
        mbObjMotionTimeSet(board->secondHookModels[1], motionFrames / 3.0f +
            (motionFrames / 2.0f) * sin(M_PI * (360.0f * phase) / 180.0) +
            (motionFrames / 4.0f) * sin(M_PI * (90.0f * phase) / 180.0));
        HuPrcVSleep();
    }
    if (soundId != 0) {
        mbAudFXStop(soundId);
    }
    mbAudFXPlay(1522);
    HuPrcSleep(6);
    mbev_CapPlayerMotShiftSet(guideModel, 3, 0, TRUE);
    mbObjMotionTimeSet(board->secondHookAttachmentModel, 0.0f);
    mbObjMotionSpeedSet(board->secondHookAttachmentModel, 2.0f);
    while (mbObjMotionTimeGet(board->secondHookAttachmentModel) <
           mbObjMotionMaxTimeGet(board->secondHookAttachmentModel)) {
        HuPrcVSleep();
    }
    mbAudFXPlay(1514);
    mbCameraShakeSet(12, 50.0f);
    playerObject->work[0]++;
    fn_1_7AF8(3, NULL, &puffRotation, NULL);
    fn_1_7AF8(4, &puffPosition, NULL, NULL);
    puffPosition.x += 0.5 * (100.0 * (cos(M_PI * puffRotation.x / 180.0) *
        sin(M_PI * puffRotation.y / 180.0)));
    puffPosition.y += 0.5 * (100.0 * sin(M_PI * puffRotation.x / 180.0));
    puffPosition.z += 0.5 * (100.0 * (cos(M_PI * puffRotation.x / 180.0) *
        cos(M_PI * puffRotation.y / 180.0)));
    fn_1_1F57C(puffPosition, puffRotation, 16);
    omVibrate(playerNo, 20, 20, 0);
    board->movementFactorX = 0.0f;
    board->movementFactorZ = -1.0f;
    if (GwPlayer[playerNo].metalF) {
        mbPlayerEffectSet(playerNo, TRUE);
    }
    HuPrcSleep(60);
    mbWipeSpecialFadeInCreate(3, 1);
    fn_1_8124(-1, 0);
    for (frame = 1; frame < mbMasuNumGet(); frame++) {
        if (mbMasuAttrGet(frame) == (1 << 15)) {
            destinationSpace = frame;
            break;
        }
    }
    if (frame >= mbMasuNumGet()) {
        destinationSpace = GwPlayer[playerNo].masuId;
    }
    mbMasuPosGet(destinationSpace, &playerHookPosition);
    mbObjPosSetV(cameraModel, &playerHookPosition);
    mbCameraMoveOnSet(FALSE);
    mbCameraMoveWait();
    mbCameraMoveOnSet(TRUE);
    playerPosition.x = 1000.0f + playerHookPosition.x;
    playerPosition.y = 1000.0f + playerHookPosition.y;
    playerPosition.z = playerHookPosition.z - 1000.0f;
    curveControl.x = playerHookPosition.x;
    curveControl.y = 1000.0f + playerHookPosition.y;
    curveControl.z = playerHookPosition.z;
    previousPosition = playerPosition;
    mbev_PlayerColMasuSet(playerNo, destinationSpace, TRUE);
    mbObjMotionTimeSet(board->secondHookModel, 0.0f);
    mbObjMotionSpeedSet(board->secondHookModel, 0.0f);
    mbObjMotionTimeSet(board->secondHookModels[0],
        mbObjMotionMaxTimeGet(board->secondHookModels[0]) / 2.0f);
    mbObjMotionTimeSet(board->secondHookModels[1],
        mbObjMotionMaxTimeGet(board->secondHookModels[1]) / 3.0f);
    mbObjMotionTimeSet(board->secondHookModels[2], 0.0f);
    mbObjMotionSpeedSet(board->secondHookModels[2], 0.0f);
    mbObjDispSet(guideModel, FALSE);
    *lbl_1_bss_4 = 0;
    fn_1_3308(1);
    fn_1_3540();
    mbWipeSpecialFadeOutCreate(3, 60);
    for (frame = 0; frame <= 30.0f; frame++) {
        phase = frame / 30.0f;
        mtxRot(rotationMatrix, 0.0f, 1440.0f * phase, 0.0f);
        mbev_CapBezierGetV(phase, &playerPosition, &curveControl, &playerHookPosition, &position);
        PSVECSubtract(&position, &previousPosition, &direction);
        rotation.x = 90.0 + 180.0 * (atan2(-direction.y,
            sqrtf(direction.x * direction.x + direction.z * direction.z)) / M_PI);
        rotation.y = 180.0 * (atan2(direction.x, direction.z) / M_PI);
        rotation.z = 0.0f;
        mbPlayerPosSetV(playerNo, &position);
        mbPlayerRotSetV(playerNo, &rotation);
        mbPlayerMtxSet(playerNo, &rotationMatrix);
        previousPosition = position;
        direction.x = direction.y = direction.z = 0.0f;
        /* The caller sets only opacity; fn_1_1EE80 forces the particle RGB channels to white. */
        color.a = 192;
        fn_1_1EE80(position, direction,
            100.0f * (0.5f + 0.5f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            10.0f * (-0.5f + 3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)),
            0.5f + 0.3f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)), color);
        if (frame == 27) {
            fn_1_1F090(&playerHookPosition, 200.0f, 100.0f, 16);
        }
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbAudFXPlay(1515);
    mbCameraShakeSet(75, 150.0f);
    omVibrate(playerNo, 20, 20, 0);
    for (frame = 0; frame < GW_PLAYER_MAX; frame++) {
        if (frame != playerNo && destinationSpace == GwPlayer[frame].masuId) {
            omVibrate(frame, 20, 20, 0);
        }
    }
    fn_1_1F090(&playerHookPosition, 300.0f, 150.0f, 20);
    HuPrcSleep(6);
    fn_1_1F090(&playerHookPosition, 300.0f, 150.0f, 24);
    HuPrcSleep(6);
    fn_1_1F090(&playerHookPosition, 400.0f, 200.0f, 28);
    HuPrcSleep(6);
    fn_1_1F090(&playerHookPosition, 400.0f, 200.0f, 32);
    HuPrcSleep(6);
    fn_1_1F090(&playerHookPosition, 500.0f, 250.0f, 32);
    HuPrcSleep(6);
    mbPlayerPosSetV(playerNo, &playerHookPosition);
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    PSMTXIdentity(rotationMatrix);
    mbPlayerMtxSet(playerNo, &rotationMatrix);
    mbPlayerScaleSet(playerNo, 1.0f, 1.0f, 1.0f);
    GwPlayer[playerNo].masuId = destinationSpace;
    mbPlayerColSnapPlayerSet(playerNo, TRUE);
    mbCameraPlayerViewSet(GwSystem.turnPlayerNo, 2);
    for (frame = 0; frame < GW_PLAYER_MAX; frame++) {
        if (destinationSpace == GwPlayer[frame].masuId) {
            mbPlayerMotionShiftSet(frame, 6, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        }
    }
    HuPrcSleep(120);
    mbMusBoardFadeOut(0, 0, 1000, 1000, -1, FALSE);
    HuPrcSleep(60);
    for (frame = 0; frame < GW_PLAYER_MAX; frame++) {
        if (destinationSpace == GwPlayer[frame].masuId) {
            mbPlayerMotionShiftSet(frame, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        }
    }
    mbObjKill(guideModel);
    mbObjKill(cameraModel);
    mbMoveNumDispSet(playerNo, TRUE);
}

/* Reattaches the first hook's model to the selected named point during event setup. */
void fn_1_757C(s32 hookIndex) {
    W05_BOARD_WORK *boardWork = lbl_1_bss_8;

    if (boardWork->firstHookIndex != -1) {
        mbObjHookReset((s16)boardWork->hookModels[boardWork->firstHookIndex]);
    }
    boardWork->firstHookIndex = hookIndex;
    mbObjHookSet((s16)boardWork->hookModels[hookIndex],
                 lbl_1_data_40[hookIndex][0],
                 (s16)boardWork->firstHookModel);
}

/* Reattaches the second hook's model to the selected named point during event setup. */
void fn_1_7610(s32 hookIndex) {
    W05_BOARD_WORK *boardWork = lbl_1_bss_8;

    if (boardWork->secondHookIndex != -1) {
        mbObjHookReset((s16)boardWork->hookModels[boardWork->secondHookIndex]);
    }
    boardWork->secondHookIndex = hookIndex;
    mbObjHookSet((s16)boardWork->hookModels[hookIndex],
                 lbl_1_data_40[hookIndex][0],
                 (s16)boardWork->secondHookModel);
}

/* Event sequences use this to read a first-hook attachment's position, orientation, or matrix. */
void fn_1_76A4(s32 selector, HuVecF *position, HuVecF *rotation, Mtx matrix) {
    Mtx parentMatrix;
    Mtx attachmentMatrix;
    HuVecF direction;
    s32 savedHookIndex[1];
    W05_BOARD_WORK *board;

    board = lbl_1_bss_8;
    savedHookIndex[0] = board->firstHookIndex;
    if (board->hookSide == 0) {
        switch ((s32)*(u8 *)lbl_1_bss_4) {
        case 0:
            Hu3DMotionCalc(mbObjModelIDGet(board->hookModels[1]));
            Hu3DModelObjMtxGet(mbObjModelIDGet(board->hookModels[1]),
                              lbl_1_data_40[1][1], parentMatrix);
            break;
        default:
            Hu3DMotionCalc(mbObjModelIDGet(board->hookModels[1]));
            Hu3DModelObjMtxGet(mbObjModelIDGet(board->hookModels[1]),
                              lbl_1_data_40[1][0], parentMatrix);
            break;
        }
    } else {
        switch ((s32)*(u8 *)lbl_1_bss_4) {
        case 0:
            Hu3DMotionCalc(mbObjModelIDGet(board->hookModels[0]));
            Hu3DModelObjMtxGet(mbObjModelIDGet(board->hookModels[0]),
                              lbl_1_data_40[0][1], parentMatrix);
            break;
        default:
            Hu3DMotionCalc(mbObjModelIDGet(board->hookModels[0]));
            Hu3DModelObjMtxGet(mbObjModelIDGet(board->hookModels[0]),
                              lbl_1_data_40[0][0], parentMatrix);
            break;
        }
    }
    Hu3DMotionCalc(mbObjModelIDGet(board->firstHookModel));
    switch (selector) {
    case 0:
        Hu3DModelObjMtxGet(mbObjModelIDGet(board->firstHookModel), "DKchrhook",
                          attachmentMatrix);
        PSMTXConcat(parentMatrix, attachmentMatrix, attachmentMatrix);
        break;
    case 2:
        Hu3DModelObjMtxGet(mbObjModelIDGet(board->firstHookModel), "DKsplash",
                          attachmentMatrix);
        PSMTXConcat(parentMatrix, attachmentMatrix, attachmentMatrix);
        break;
    case 5:
        Hu3DModelObjMtxGet(mbObjModelIDGet(board->firstHookModel), "chihook",
                          attachmentMatrix);
        PSMTXConcat(parentMatrix, attachmentMatrix, attachmentMatrix);
        break;
    default:
        Hu3DModelObjMtxGet(mbObjModelIDGet(board->firstHookModel), "DKhook",
                          attachmentMatrix);
        PSMTXConcat(parentMatrix, attachmentMatrix, attachmentMatrix);
        break;
    }
    if (matrix != NULL) {
        PSMTXCopy(attachmentMatrix, matrix);
    }
    if (position != NULL) {
        position->x = attachmentMatrix[0][3];
        position->y = attachmentMatrix[1][3];
        position->z = attachmentMatrix[2][3];
    }
    if (rotation != NULL) {
        attachmentMatrix[0][3] = attachmentMatrix[1][3] = attachmentMatrix[2][3] = 0.0f;
        PSMTXMultVec(attachmentMatrix, &lbl_1_data_4EC, &direction);
        rotation->x = 180.0 *
            (atan2(-direction.y, sqrtf(direction.x * direction.x + direction.z * direction.z)) /
             M_PI);
        rotation->y = 180.0 * (atan2(direction.x, direction.z) / M_PI);
        rotation->z = 0.0f;
    }
}

HuVecF lbl_1_data_51C = {0.0f, 0.0f, 1.0f};

/* Event sequences use this to read a second-hook attachment's position, orientation, or matrix. */
void fn_1_7AF8(s32 selector, HuVecF *position, HuVecF *rotation, Mtx matrix) {
    Mtx parentMatrix;
    Mtx attachmentMatrix;
    Mtx baseMatrix;
    Mtx cannonMatrix;
    HuVecF direction;
    s32 savedHookIndex[1];
    W05_BOARD_WORK *board;

    board = lbl_1_bss_8;
    savedHookIndex[0] = board->secondHookIndex;
    if (board->hookSide == 0) {
        switch ((s32)*(u8 *)lbl_1_bss_4) {
        case 0:
            Hu3DMotionCalc(mbObjModelIDGet(board->hookModels[1]));
            Hu3DModelObjMtxGet(mbObjModelIDGet(board->hookModels[1]),
                              lbl_1_data_40[1][0], parentMatrix);
            break;
        default:
            Hu3DMotionCalc(mbObjModelIDGet(board->hookModels[1]));
            Hu3DModelObjMtxGet(mbObjModelIDGet(board->hookModels[1]),
                              lbl_1_data_40[1][1], parentMatrix);
            break;
        }
    } else {
        switch ((s32)*(u8 *)lbl_1_bss_4) {
        case 0:
            Hu3DMotionCalc(mbObjModelIDGet(board->hookModels[0]));
            Hu3DModelObjMtxGet(mbObjModelIDGet(board->hookModels[0]),
                              lbl_1_data_40[0][0], parentMatrix);
            break;
        default:
            Hu3DMotionCalc(mbObjModelIDGet(board->hookModels[0]));
            Hu3DModelObjMtxGet(mbObjModelIDGet(board->hookModels[0]),
                              lbl_1_data_40[0][1], parentMatrix);
            break;
        }
    }
    Hu3DMotionCalc(mbObjModelIDGet(board->secondHookModel));
    switch (selector) {
    case 0:
        Hu3DModelObjMtxGet(mbObjModelIDGet(board->secondHookModel), "yukahook",
                          cannonMatrix);
        PSMTXConcat(parentMatrix, cannonMatrix, cannonMatrix);
        Hu3DMotionCalc(mbObjModelIDGet(board->secondHookModels[2]));
        Hu3DModelObjMtxGet(mbObjModelIDGet(board->secondHookModels[2]), "kochrhook",
                          attachmentMatrix);
        PSMTXConcat(cannonMatrix, attachmentMatrix, attachmentMatrix);
        break;
    case 2:
        Hu3DModelObjMtxGet(mbObjModelIDGet(board->secondHookModel), "KPsplash",
                          attachmentMatrix);
        PSMTXConcat(parentMatrix, attachmentMatrix, attachmentMatrix);
        break;
    case 1:
        Hu3DModelObjMtxGet(mbObjModelIDGet(board->secondHookModel), "koopahook",
                          attachmentMatrix);
        PSMTXConcat(parentMatrix, attachmentMatrix, attachmentMatrix);
        break;
    case 3:
        Hu3DMotionCalc(mbObjModelIDGet(board->secondHookModels[0]));
        Hu3DMotionCalc(mbObjModelIDGet(board->secondHookModels[1]));
        Hu3DModelObjMtxGet(mbObjModelIDGet(board->secondHookModel), "daihook", baseMatrix);
        PSMTXConcat(parentMatrix, baseMatrix, baseMatrix);
        Hu3DModelObjMtxGet(mbObjModelIDGet(board->secondHookModels[0]), "canonhook",
                          cannonMatrix);
        PSMTXConcat(baseMatrix, cannonMatrix, cannonMatrix);
        Hu3DModelObjMtxGet(mbObjModelIDGet(board->secondHookModels[1]), "chrhook",
                          attachmentMatrix);
        PSMTXConcat(cannonMatrix, attachmentMatrix, attachmentMatrix);
        break;
    default:
        Hu3DMotionCalc(mbObjModelIDGet(board->secondHookModels[0]));
        Hu3DMotionCalc(mbObjModelIDGet(board->secondHookModels[1]));
        Hu3DModelObjMtxGet(mbObjModelIDGet(board->secondHookModel), "daihook", baseMatrix);
        PSMTXConcat(parentMatrix, baseMatrix, baseMatrix);
        Hu3DModelObjMtxGet(mbObjModelIDGet(board->secondHookModels[0]), "canonhook",
                          cannonMatrix);
        PSMTXConcat(baseMatrix, cannonMatrix, cannonMatrix);
        Hu3DModelObjMtxGet(mbObjModelIDGet(board->secondHookModels[1]), "bullethook",
                          attachmentMatrix);
        PSMTXConcat(cannonMatrix, attachmentMatrix, attachmentMatrix);
        break;
    }
    if (matrix != NULL) {
        PSMTXCopy(attachmentMatrix, matrix);
    }
    if (position != NULL) {
        position->x = attachmentMatrix[0][3];
        position->y = attachmentMatrix[1][3];
        position->z = attachmentMatrix[2][3];
    }
    if (rotation != NULL) {
        attachmentMatrix[0][3] = attachmentMatrix[1][3] = attachmentMatrix[2][3] = 0.0f;
        PSMTXMultVec(attachmentMatrix, &lbl_1_data_51C, &direction);
        rotation->x = 180.0 *
            (atan2(-direction.y, sqrtf(direction.x * direction.x + direction.z * direction.z)) /
             M_PI);
        rotation->y = 180.0 * (atan2(direction.x, direction.z) / M_PI);
        rotation->z = 0.0f;
    }
}

/* Records the player and state used by the first hook event's board update. */
void fn_1_80AC(s32 playerNo, s32 state) {
    s32 *work = (s32 *)lbl_1_bss_8;

    work[124] = playerNo;
    work[125] = state;
}

/* Records the model and state used by the first hook event's board update. */
void fn_1_80D4(s32 modelId, s32 state) {
    s32 *work = (s32 *)lbl_1_bss_8;

    work[126] = modelId;
    work[127] = state;
}

/* Records the player and state used by the second hook event's board update. */
void fn_1_80FC(s32 playerNo, s32 state) {
    s32 *work = (s32 *)lbl_1_bss_8;

    work[128] = playerNo;
    work[129] = state;
}

/* Records the model and state used by the second hook event's board update. */
void fn_1_8124(s32 modelId, s32 state) {
    s32 *work = (s32 *)lbl_1_bss_8;

    work[130] = modelId;
    work[131] = state;
}

f32 lbl_1_data_558[14] = {
    1.0f, 1.0f, 1.0f, 1.0f, 0.7f, 1.0f, 0.65f, 1.0f, 0.7f, 0.7f, 1.0f, 0.7f, 0.7f, 0.7f
};
f32 lbl_1_data_590[14] = {
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.4f, 1.0f, 1.4f, 1.4f, 1.4f
};
HuVecF lbl_1_data_5C8[6] = {
    {-1403.0f, 406.0f, -1309.0f}, {-1453.0f, 406.0f, -1300.0f},
    {-629.0f, 423.0f, -939.0f}, {-553.0f, 432.0f, -919.0f},
    {286.0f, 248.0f, -641.0f}, {286.0f, 248.0f, -691.0f}
};
HuVecF lbl_1_data_610[3] = {
    {0.0f, -150.0f, 0.0f},
    {0.0f, -150.0f, -50.0f},
    {0.0f, -50.0f, -150.0f}
};

/* Moves the player from the cannon hook and spins them free during the event's release sequence. */
void fn_1_814C(OMOBJ *object) {
    Mtx attachmentMatrix;
    Mtx rotationMatrix;
    HuVecF position;
    HuVecF rotation;
    HuVecF scale;
    HuVecF velocity;
    W05_BOARD_WORK *board;
    GXColor color;
    s32 playerNo;
    f32 progress;

    board = lbl_1_bss_8;
    playerNo = object->work[3];
    if (mbExitCheck() || object->work[2] != 0) {
        omDelObjEx(mbObjMan, object);
        return;
    }
    switch ((s32)object->work[0]) {
    case 0:
        progress = (f32)++object->work[1] / 30.0f;
        scale.x = scale.z = lbl_1_data_558[GwPlayer[playerNo].charNo];
        scale.y = progress * lbl_1_data_590[GwPlayer[playerNo].charNo];
        fn_1_7AF8(3, &position, &rotation, NULL);
        mbPlayerPosSetV(playerNo, &position);
        mbPlayerRotSetV(playerNo, &rotation);
        mbPlayerScaleSetV(playerNo, &scale);
        if (progress >= 1.0f) {
            object->work[0]++;
            object->work[1] = 0;
        }
        break;
    case 1:
        fn_1_7AF8(3, &position, &rotation, NULL);
        mbPlayerPosSetV(playerNo, &position);
        mbPlayerRotSetV(playerNo, &rotation);
        break;
    case 2:
        progress = (f32)++object->work[1] / 60.0f;
        mtxRot(rotationMatrix, 0.0f, 1440.0f * progress, 0.0f);
        fn_1_7AF8(3, &position, &rotation, attachmentMatrix);
        attachmentMatrix[0][3] = attachmentMatrix[1][3] = attachmentMatrix[2][3] = 0.0f;
        velocity.x = 0.0f;
        velocity.y = 1.0f;
        velocity.z = 0.0f;
        PSMTXMultVec(attachmentMatrix, &velocity, &velocity);
        PSVECScale(&velocity, &velocity, 2000.0f * progress);
        PSVECAdd(&position, &velocity, &position);
        mbPlayerPosSetV(playerNo, &position);
        mbPlayerRotSetV(playerNo, &rotation);
        mbPlayerMtxSet(playerNo, &rotationMatrix);
        velocity.x = velocity.y = velocity.z = 0.0f;
        color.a = 128;
        fn_1_1EE80(position, velocity,
            100.0f * (0.5f + 0.5f *
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            10.0f * (-0.5f + (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            0.5f + 0.3f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)), color);
        if (progress >= 1.0f) {
            object->work[0]++;
            object->work[1] = 0;
        }
        break;
    default:
        omDelObjEx(mbObjMan, object);
        return;
    }
}

/* Raises the hook's model effect over the board, then hides it when the effect finishes. */
void fn_1_8670(OMOBJ *object) {
    Mtx attachmentMatrix;
    Mtx rotationMatrix;
    HuVecF position;
    HuVecF rotation;
    HuVecF velocity;
    GXColor color;
    W05_BOARD_WORK *board;
    s32 modelId;
    f32 progress;

    board = lbl_1_bss_8;
    modelId = board->risingEffectModel;
    if (mbExitCheck() || object->work[2] != 0) {
        omDelObjEx(mbObjMan, object);
        return;
    }
    switch ((s32)object->work[0]) {
    case 0:
        progress = (f32)++object->work[1] / 24.0f;
        mtxRot(rotationMatrix, 0.0f, 1440.0f * progress, 0.0f);
        fn_1_7AF8(3, &position, &rotation, attachmentMatrix);
        attachmentMatrix[0][3] = attachmentMatrix[1][3] = attachmentMatrix[2][3] = 0.0f;
        velocity.x = 0.0f;
        velocity.y = 1.0f;
        velocity.z = 0.0f;
        PSMTXMultVec(attachmentMatrix, &velocity, &velocity);
        PSVECScale(&velocity, &velocity, 800.0f * progress);
        PSVECAdd(&position, &velocity, &position);
        mbObjPosSetV((s16)modelId, &position);
        mbObjDispSet((s16)modelId, 1);
        velocity.x = velocity.y = velocity.z = 0.0f;
        color.a = 128;
        fn_1_1EE80(position, velocity,
            100.0f * (0.5f + 0.5f *
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            10.0f * (-0.5f + (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            0.5f + 0.3f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)), color);
        if (progress >= 1.0f) {
            object->work[0]++;
            object->work[1] = 0;
        }
        break;
    default:
        mbObjDispSet((s16)modelId, 0);
        object->work[2] = 1;
        break;
    }
}

/* Starts the cannon-hook travel effect by creating its frame-update object for a destination. */
void fn_1_8A2C(HuVecF *destination, u32 parameter) {
    HuVecF start;
    OMOBJ *object;

    fn_1_7AF8(3, &start, NULL, NULL);
    object = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_8B58);
    object->trans.x = start.x;
    object->trans.y = start.y;
    object->trans.z = start.z;
    object->rot.x = start.x + (destination->x - start.x) / 2.0f;
    object->rot.y = 1000.0f + destination->y;
    object->rot.z = start.z + (destination->z - start.z) / 2.0f;
    object->scale.x = destination->x;
    object->scale.y = destination->y;
    object->scale.z = destination->z;
    object->work[0] = 0;
    object->work[1] = 0;
    object->work[2] = parameter;
    object->work[3] = 0;
}

/* Advances the moving model along its hook-to-destination arc on each object-manager update. */
void fn_1_8B58(OMOBJ *object) {
    HuVecF position;
    HuVecF velocity;
    GXColor color;
    W05_ARRIVAL_WORK *board;
    s32 modelId;
    f32 eventChance;
    f32 progress;

    board = lbl_1_bss_8;
    modelId = board->movingModel;
    if (mbExitCheck() || object->work[3] != 0) {
        omDelObjEx(mbObjMan, object);
        return;
    }
    switch ((s32)object->work[0]) {
    case 0:
        progress = 0.6f + 0.4f * ((f32)++object->work[1] / object->work[2]);
        mbev_CapBezierGetV(progress,
                           &object->trans, &object->rot, &object->scale, &position);
        mbObjPosSetV(modelId, &position);
        mbObjDispSet(modelId, TRUE);
        velocity.x = velocity.y = velocity.z = 0.0f;
        color.a = 128;
        fn_1_1EE80(position, velocity,
            100.0f * (0.5f + 0.5f *
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            10.0f * (-0.5f + (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            0.5f + 0.3f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)), color);
        if (object->work[1] >= object->work[2]) {
            mbCameraShakeSet(75, 30.000002f);
            mbObjDispSet(modelId, FALSE);
            mbObjPosSetV(board->arrivalModel, &position);
            mbObjMotionTimeSet(board->arrivalModel, 0.0f);
            mbObjMotionSpeedSet(board->arrivalModel, 1.0f);
            mbObjDispSet(board->arrivalModel, TRUE);
            if (GwSystem.curTime == 0) {
                eventChance = 0.15f;
            } else {
                eventChance = 0.1f;
            }
            if (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE) < eventChance) {
                board->eventRequested = 1;
            }
            object->work[0]++;
            object->work[1] = 0;
        }
        break;
    case 1:
        if (++object->work[1] >= 5) {
            object->work[0]++;
            object->work[1] = 0;
        }
        break;
    default:
        object->work[3] = 1;
        break;
    }
}

/* Handles the Boo-marked space after a move: by night Boo carries the player between linked
 * spaces. */
void fn_1_8FC0(s32 playerNo, s32 spaceId) {
    int motionData[16];
    HuVecF playerPosition;
    HuVecF endPosition;
    HuVecF startPosition;
    HuVecF direction;
    HuVecF position;
    W05_TERESA_SCENE_WORK *board;
    s32 sourceSpace;
    s32 linkedSpace;
    s32 readId;
    s32 booModel;
    s32 motionModel;
    s32 elapsed;
    f32 progress;
    f32 scale;

    board = lbl_1_bss_8;
    if (GwSystem.curTime == 0) {
        mbev_CapCallTeresa(playerNo, spaceId);
        return;
    }
linkedSpace = mbMasuAttrFindLink(spaceId, (1 << 13));
    mbMasuPosGet(linkedSpace, &endPosition);
sourceSpace = mbMasuAttrFindLink(linkedSpace, (1 << 13));
    mbMasuPosGet(sourceSpace, &startPosition);
    mbPlayerPosGet(playerNo, &playerPosition);
    if (board->spaces[0] == spaceId) {
        motionModel = board->motionModels[0];
    } else if (board->spaces[1] == spaceId) {
        motionModel = board->motionModels[1];
    } else {
        motionModel = board->defaultMotionModel;
    }
    readId = mbBGRead(DATA_capsuleshop);
    if (readId != -1) {
        mbBGReadWait(readId);
    }
    motionData[0] = DATANUM(DATA_capsuleshop, 9);
    motionData[1] = DATANUM(DATA_capsuleshop, 11);
    motionData[2] = DATANUM(DATA_capsuleshop, 10);
    motionData[3] = -1;
    booModel = mbObjCreate(DATANUM(DATA_capsuleshop, 8), motionData, TRUE);
    mbObjMotionSet(booModel, 1, HU3D_MOTATTR_LOOP);
    mbObjPosSetV(booModel, &startPosition);
    PSVECSubtract(&endPosition, &startPosition, &direction);
    mbObjRotSet(booModel, 0.0f, 180.0 * (atan2(direction.x, direction.z) / M_PI), 0.0f);
    mbObjScaleSet(booModel, 2.0f, 2.0f, 2.0f);
    mbObjLayerSet(booModel, 3);
    mbev_CapTeresaFadeCreate(booModel);
    mbev_CapTeresaFadeSet(0.0f);
    mbAudFXPlay(1507);
    mbObjMotionSpeedSet(motionModel, 1.0f);
    omVibrate(playerNo, 20, 7, 3);
    PSVECSubtract(&endPosition, &startPosition, &direction);
    mbPlayerRotateStart(playerNo, 180.0 + 180.0 * (atan2(direction.x, direction.z) / M_PI), 15);
    while (!mbPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    while (!mbObjMotionEndCheck(motionModel)) {
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 9, 0.0f, 8.0f, 0);
    mbAudFXPlay(MSM_SE_BRD00_142);
    for (elapsed = 0; (f32)elapsed <= 60.0f; elapsed++) {
        progress = (f32)elapsed / 60.0f;
        scale = 2.0 * sin(M_PI * (90.0f * (progress * progress)) / 180.0);
        position.x = startPosition.x + progress * (endPosition.x - startPosition.x);
        position.y = 120.00001f +
            (startPosition.y + progress * (endPosition.y - startPosition.y));
        position.z = startPosition.z + progress * (endPosition.z - startPosition.z);
        mbObjPosSetV(booModel, &position);
        mbObjScaleSet(booModel, scale, scale, scale);
        mbev_CapTeresaFadeSet(255.0f * progress);
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbWipeDissolveFadeOutTime(1);
    mbev_CapTeresaFadeKill(booModel);
    mbObjKill(booModel);
    mbev_CapCallTeresa(playerNo, spaceId);
    mbAudFXPlay(1508);
    mbObjMotionSpeedSet(motionModel, -1.0f);
    mbWipeDissolveFadeIn();
}

void fn_1_94B4(void) {
    fn_1_35F0(0, 1, 0);
}

/* Starts the special board wipe used when the raft event fades between scenes. */
void fn_1_94E0(s32 fadeIn) {
    s32 unusedValue;
    if (fadeIn)
    {
        mbWipeSpecialFadeInCreate(1, 1);
        return;
    }
mbWipeSpecialFadeOutCreate(1, 60);
}

/* Offers the raft ride and runs its camera, travel, and optional detour when a player lands
 * there. */
void fn_1_9528(int playerNo, int currentSpace) {
    Mtx playerMatrix;
    int motionData[16];
    HuVecF linkedPosition;
    HuVecF launchPosition;
    HuVecF approachPosition;
    HuVecF destination;
    HuVecF destinationPosition;
    HuVecF spacePosition;
    HuVecF playerPosition;
    HuVecF attachmentPosition;
    HuVecF rotation;
    HuVecF position;
    HuVecF cameraRotation;
    HuVecF firstDeparture;
    HuVecF secondDeparture;
    HuVecF thirdDeparture;
    W05_BOARD_WORK *board;
    OMOBJ *raftObject;
    OMOBJ *firstDepartureObject;
    OMOBJ *secondDepartureObject;
    OMOBJ *thirdDepartureObject;
    HuVecF *firstTarget;
    HuVecF *secondTarget;
    HuVecF *thirdTarget;
    int modelId;
    int frame;
    int destinationSpace;
    int soundId;
    int vibrationFrames;
    int fallMotion;
    int rideMotion;
    int firstLinkedSpace;
    int secondLinkedSpace;
    int targetSpace;
    int readId;
    int choice;
    int estimatedCoins;
    int accepted;
    int detour;
    int cameraTurned;
    int cameraZooming;
    int restored;
    int boarded;
    f32 progress;
    f32 zoom;
    f32 cameraProgress;
    f32 facingAngle;
    f32 startFacingAngle;

    board = lbl_1_bss_8;
    accepted = 0;
    detour = 0;
    cameraTurned = 0;
    cameraZooming = 0;
    restored = 0;
    boarded = 0;
    mbMasuPosGet(currentSpace, &spacePosition);
firstLinkedSpace = mbMasuAttrFindLink(currentSpace, (1 << 13));
    mbMasuPosGet(firstLinkedSpace, &linkedPosition);
secondLinkedSpace = mbMasuAttrFindLink(firstLinkedSpace, (1 << 13));
    mbMasuPosGet(secondLinkedSpace, &launchPosition);
    approachPosition = launchPosition;
    approachPosition.x -= 200.0f;
    for (frame = 1; frame < mbMasuNumGet(); frame++) {
        if (mbMasuMAttrGet(frame) & 0x40) {
            destinationSpace = frame;
            break;
        }
    }
    mbMasuPosGet(destinationSpace, &destination);
    targetSpace = destinationSpace;
    mbMasuPosGet(targetSpace, &destinationPosition);
    mbCameraPlayerViewSet(playerNo, 0);
    mbPlayerPosGet(playerNo, &playerPosition);
    PSVECSubtract(&linkedPosition, &playerPosition, &position);
    mbPlayerRotateStart(playerNo, (s16)(180.0 * (atan2(position.x, position.z) / M_PI)), 15);
    HuPrcVSleep();
    mbCameraMoveWait();
    readId = mbBGRead(DATANUM(DATA_capsuleshop, 0));
    if (readId != -1) {
        mbBGReadWait(readId);
    }
    motionData[0] = DATANUM(DATA_capsuleshop, 1);
    motionData[1] = DATANUM(DATA_capsuleshop, 2);
    motionData[2] = DATANUM(DATA_capsuleshop, 3);
    motionData[3] = -1;
    modelId = mbObjCreate(DATANUM(DATA_capsuleshop, 0), motionData, TRUE);
    mbObjMotionSet(modelId, 1, HU3D_MOTATTR_LOOP);
    mbObjLayerSet(modelId, 3);
    HuDataDirClose(DATA_capsuleshop);
    HuPrcVSleep();
    raftObject = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_B304);
    raftObject->work[0] = raftObject->work[1] = raftObject->work[2] = raftObject->work[3] = 0;
    board->raftMotionFrame = 0;
    board->raftMotionFrames = 0;
    board->raftMotionProgress = 0.0f;
    board->raftPreviousProgress = 0.0f;
    board->raftMotionBoost = 0.5f;
    board->raftEventState = 1;
    ((W05_ARRIVAL_WORK *)board)->eventRequested = 0;
    HuPrcVSleep();
    rideMotion = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mario, 214));
    HuPrcVSleep();
    fallMotion = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 34));
    HuPrcVSleep();
    PSVECSubtract(&approachPosition, &launchPosition, &position);
    mbObjRotSet(modelId, 0.0f, 180.0 * (atan2(position.x, position.z) / M_PI), 0.0f);
    mbObjMotionShiftSet(modelId, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    for (frame = 0; frame <= 15.0f; frame++) {
        progress = frame / 15.0f;
        mbev_CapVecChase(progress, &launchPosition, &approachPosition, &position);
        mbObjPosSetV(modelId, &position);
        HuPrcVSleep();
    }
    PSVECSubtract(&linkedPosition, &approachPosition, &position);
    mbObjRotSet(modelId, 0.0f, 180.0 * (atan2(position.x, position.z) / M_PI), 0.0f);
    mbObjMotionShiftSet(modelId, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    for (frame = 0; frame <= 30.0f; frame++) {
        progress = frame / 30.0f;
        mbev_CapVecChase(progress, &approachPosition, &linkedPosition, &position);
        position.y += 3.0 * (100.0 * sin(M_PI * (180.0f * progress) / 180.0));
        mbObjPosSetV(modelId, &position);
        HuPrcVSleep();
    }
    mbObjPosSetV(modelId, &linkedPosition);
    PSVECSubtract(&spacePosition, &linkedPosition, &position);
    mbObjRotGet(modelId, &rotation);
    startFacingAngle = rotation.y;
    facingAngle = 180.0 * (atan2(position.x, position.z) / M_PI);
    mbObjMotionShiftSet(modelId, 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    for (frame = 0; frame <= 12.0f; frame++) {
        progress = frame / 12.0f;
        rotation.x = rotation.z = 0.0f;
        rotation.y = mbev_CapAngleSumLerp(progress, startFacingAngle, facingAngle);
        mbObjRotSetV(modelId, &rotation);
        HuPrcVSleep();
    }
    mbObjRotSet(modelId, 0.0f, facingAngle, 0.0f);
    mbObjMotionShiftSet(modelId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    if (mbPlayerCoinGet(playerNo) >= 10) {
        do {
            mbAudFXPlay(961);
            mbWinCreateChoice(1, MESSNUM(MESS_BOARD_W05, 19), 9, 0);
            if (GwPlayer[playerNo].comF) {
                estimatedCoins = mbPlayerCoinGet(playerNo) -
                    (f32)(GwPlayer[playerNo].comDif * 10) *
                    (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
                if ((f32)((3 - GwPlayer[playerNo].comDif) * 10) <
                    100.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)) &&
                    *(u8 *)lbl_1_bss_4 != 0) {
                    estimatedCoins = 0;
                }
                if (estimatedCoins >= 1) {
                    mbComChoiceUpSet();
                } else {
                    mbComChoiceDownSet();
                }
            }
            mbWinTopWait();
            choice = mbWinTopChoiceGet();
            if (choice == 0) {
                mbCoinAddExec(playerNo, -10);
                mbAudFXPlay(960);
                mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 21), 9);
                mbWinTopWait();
                accepted = 1;
            } else if (choice == 1 || choice == -1) {
                mbAudFXPlay(962);
                mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 22), 9);
                mbWinTopWait();
                accepted = 0;
            } else {
                mbev_Scroll(playerNo, 0);
                mbStatusDispSetAll(TRUE);
            }
        } while (choice == 2);
    } else {
        mbAudFXPlay(962);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 20), 9);
        mbWinTopWait();
        accepted = 0;
    }
    if (accepted == 0) {
        PSVECSubtract(&approachPosition, &linkedPosition, &position);
        mbObjRotSet(modelId, 0.0f, 180.0 * (atan2(position.x, position.z) / M_PI), 0.0f);
        mbObjMotionShiftSet(modelId, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        for (frame = 0; frame <= 30.0f; frame++) {
            progress = frame / 30.0f;
            mbev_CapVecChase(progress, &linkedPosition, &approachPosition, &position);
            position.y += 3.0 * (100.0 * sin(M_PI * (180.0f * progress) / 180.0));
            mbObjPosSetV(modelId, &position);
            HuPrcVSleep();
        }
        PSVECSubtract(&launchPosition, &approachPosition, &position);
        mbObjRotSet(modelId, 0.0f, 180.0 * (atan2(position.x, position.z) / M_PI), 0.0f);
        mbObjMotionShiftSet(modelId, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        for (frame = 0; frame <= 15.0f; frame++) {
            progress = frame / 15.0f;
            mbev_CapVecChase(progress, &approachPosition, &launchPosition, &position);
            mbObjPosSetV(modelId, &position);
            HuPrcVSleep();
        }
        mbPlayerRotateStart(playerNo, 0, 15);
        while (mbPlayerRotateCheck(playerNo) == 0) {
            HuPrcVSleep();
        }
        mbCameraPlayerViewSet(playerNo, 2);
        mbCameraMoveWait();
        restored = 1;
    } else {
        boarded = 1;
        mbWipeSpecialFadeInCreate(1, 1);
        for (frame = 0; frame < GW_PLAYER_MAX; frame++) {
            if (playerNo != frame) {
                mbPlayerDispSet(frame, FALSE);
            }
        }
        fn_1_B078(&attachmentPosition, &rotation, NULL);
        mbPlayerPosGet(playerNo, &playerPosition);
        mbPlayerRotSetV(playerNo, &rotation);
        mbPlayerMotionShiftSet(playerNo, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbPlayerPosSetV(playerNo, &attachmentPosition);
        mbCameraPlayerViewSet(playerNo, 0);
        mbCameraMoveWait();
        mbPlayerMotionShiftSet(playerNo, rideMotion, 0.0f, 8.0f, 0);
        mbAudFXPlay(1506);
        mbWipeSpecialFadeOutCreate(1, 60);
        raftObject->work[0]++;
        omVibrate(playerNo, 240, 4, 4);
        mbCameraRotGet(&cameraRotation);
        zoom = mbCameraZoomGet();
        cameraProgress = 0.0f;
        cameraTurned = 0;
        cameraZooming = 0;
        soundId = mbAudFXPlay(1504);
        vibrationFrames = 0;
        while (raftObject->work[2] == 0) {
            fn_1_B078(&playerPosition, &rotation, playerMatrix);
            playerMatrix[0][3] = playerMatrix[1][3] = playerMatrix[2][3] = 0.0f;
            mbPlayerMtxSet(playerNo, &playerMatrix);
            mbPlayerPosSetV(playerNo, &playerPosition);
            mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
            if (board->raftMotionProgress > 0.15f && cameraTurned == 0) {
                cameraProgress += 0.033333335f;
                if (cameraProgress > 1.0f) {
                    cameraProgress = 1.0f;
                }
                rotation.x = -27.0f;
                rotation.y += 180.0f;
                rotation.z = 0.0f;
                rotation.x = mbev_CapAngleSumLerp(cameraProgress, cameraRotation.x, rotation.x);
                rotation.y = mbev_CapAngleSumLerp(cameraProgress, cameraRotation.y, rotation.y);
                mbCameraRotSetV(&rotation);
                mbCameraZoomSet(zoom + cameraProgress * (650.0f - zoom));
                if (board->raftMotionProgress >= 0.35f) {
                    cameraTurned = 1;
                    mbWipeDissolveFadeOutTime(1);
                    board->raftEventState = 0;
                    if (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE) > 0.2f &&
                        *(u8 *)lbl_1_bss_4 == 0) {
                        fn_1_C398(playerNo);
                        detour = 1;
                    }
                    board->raftMotionBoost = 0.0f;
                    frame = mbObjMotionTimeGet(board->raftParentModel);
                    mbObjMotionTimeSet(board->raftParentModel, frame + 1);
                    mbCameraEyeSetV(&playerPosition);
                    mbCameraMoveWait();
                    mbCameraRotSetV(&cameraRotation);
                    mbCameraZoomSet(zoom);
                    mbWipeDissolveFadeInTime(5);
                    board->raftEventState = 1;
                }
            } else if (detour != 0) {
                if (board->raftMotionProgress >= 0.46f && board->raftPreviousProgress < 0.46f) {
firstTarget = &lbl_1_data_5C8[(frand() & W05_RANDOM_VALUE_MASK) & 1];
                    fn_1_7AF8(3, &firstDeparture, NULL, NULL);
                    firstDepartureObject = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_8B58);
                    firstDepartureObject->trans.x = firstDeparture.x;
                    firstDepartureObject->trans.y = firstDeparture.y;
                    firstDepartureObject->trans.z = firstDeparture.z;
                    firstDepartureObject->rot.x =
                        firstDeparture.x + (firstTarget->x - firstDeparture.x) / 2.0f;
                    firstDepartureObject->rot.y = 1000.0f + firstTarget->y;
                    firstDepartureObject->rot.z =
                        firstDeparture.z + (firstTarget->z - firstDeparture.z) / 2.0f;
                    firstDepartureObject->scale.x = firstTarget->x;
                    firstDepartureObject->scale.y = firstTarget->y;
                    firstDepartureObject->scale.z = firstTarget->z;
                    firstDepartureObject->work[0] = 0;
                    firstDepartureObject->work[1] = 0;
                    firstDepartureObject->work[2] = 10;
                    firstDepartureObject->work[3] = 0;
                    mbAudFXPlay(1515);
                } else if (board->raftMotionProgress >= 0.51f &&
                           board->raftPreviousProgress < 0.51f) {
secondTarget = &lbl_1_data_5C8[(frand() & W05_RANDOM_VALUE_MASK) & 1] + 2;
                    fn_1_7AF8(3, &secondDeparture, NULL, NULL);
                    secondDepartureObject = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_8B58);
                    secondDepartureObject->trans.x = secondDeparture.x;
                    secondDepartureObject->trans.y = secondDeparture.y;
                    secondDepartureObject->trans.z = secondDeparture.z;
                    secondDepartureObject->rot.x =
                        secondDeparture.x + (secondTarget->x - secondDeparture.x) / 2.0f;
                    secondDepartureObject->rot.y = 1000.0f + secondTarget->y;
                    secondDepartureObject->rot.z =
                        secondDeparture.z + (secondTarget->z - secondDeparture.z) / 2.0f;
                    secondDepartureObject->scale.x = secondTarget->x;
                    secondDepartureObject->scale.y = secondTarget->y;
                    secondDepartureObject->scale.z = secondTarget->z;
                    secondDepartureObject->work[0] = 0;
                    secondDepartureObject->work[1] = 0;
                    secondDepartureObject->work[2] = 10;
                    secondDepartureObject->work[3] = 0;
                    mbAudFXPlay(1515);
                } else if (board->raftMotionProgress >= 0.57f &&
                           board->raftPreviousProgress < 0.57f) {
thirdTarget = &lbl_1_data_5C8[(frand() & W05_RANDOM_VALUE_MASK) & 1] + 4;
                    fn_1_7AF8(3, &thirdDeparture, NULL, NULL);
                    thirdDepartureObject = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_8B58);
                    thirdDepartureObject->trans.x = thirdDeparture.x;
                    thirdDepartureObject->trans.y = thirdDeparture.y;
                    thirdDepartureObject->trans.z = thirdDeparture.z;
                    thirdDepartureObject->rot.x =
                        thirdDeparture.x + (thirdTarget->x - thirdDeparture.x) / 2.0f;
                    thirdDepartureObject->rot.y = 1000.0f + thirdTarget->y;
                    thirdDepartureObject->rot.z =
                        thirdDeparture.z + (thirdTarget->z - thirdDeparture.z) / 2.0f;
                    thirdDepartureObject->scale.x = thirdTarget->x;
                    thirdDepartureObject->scale.y = thirdTarget->y;
                    thirdDepartureObject->scale.z = thirdTarget->z;
                    thirdDepartureObject->work[0] = 0;
                    thirdDepartureObject->work[1] = 0;
                    thirdDepartureObject->work[2] = 25;
                    thirdDepartureObject->work[3] = 0;
                    mbAudFXPlay(1515);
                }
                if (((W05_ARRIVAL_WORK *)board)->eventRequested != 0) {
                    if (board->raftMotionProgress < 0.51) {
                        fn_1_D390(playerNo, destinationSpace, fallMotion, soundId,
                                  &lbl_1_data_610[0]);
                    } else if (board->raftMotionProgress < 0.57) {
                        fn_1_D390(playerNo, destinationSpace, fallMotion, soundId,
                                  &lbl_1_data_610[1]);
                    } else {
                        fn_1_D390(playerNo, destinationSpace, fallMotion, soundId,
                                  &lbl_1_data_610[2]);
                    }
                    soundId = -1;
                    restored = 1;
                    GwPlayer[playerNo].moveNum = 1;
                    goto ride_finished;
                }
            }
            if (board->raftMotionProgress > 0.35f) {
                vibrationFrames++;
                if (vibrationFrames % 20 == 1) {
                    omVibrate(playerNo, 20, 7, 3);
                }
            } else if (board->raftMotionProgress > 0.75f) {
                vibrationFrames++;
                if (vibrationFrames % 20 == 1) {
                    omVibrate(playerNo, 20, 4, 4);
                }
            }
            if (cameraZooming == 0 && board->raftMotionProgress >= 0.8f) {
                zoom = mbCameraZoomGet();
                cameraZooming = 1;
            } else if (board->raftMotionProgress >= 0.8f) {
                cameraProgress = (board->raftMotionProgress - 0.8f) / 0.2f;
                mbCameraZoomSet(zoom + cameraProgress * (1800.0f - zoom));
                cameraRotation.y = 33.0f * cameraProgress;
                mbCameraRotSetV(&cameraRotation);
            }
            HuPrcVSleep();
        }
        if (soundId != -1) {
                mbAudFXStop(soundId);
            }
            PSVECSubtract(&destination, &playerPosition, &position);
            fn_1_B078(&playerPosition, &rotation, NULL);
            mbPlayerPosSetV(playerNo, &playerPosition);
            mbPlayerRotSetV(playerNo, &rotation);
            PSMTXIdentity(playerMatrix);
            mbPlayerMtxSet(playerNo, &playerMatrix);
            mbPlayerPosGet(playerNo, &playerPosition);
            mbPlayerRotSet(playerNo, 0.0f, 180.0 * (atan2(position.x, position.z) / M_PI), 0.0f);
            GwPlayer[playerNo].masuIdNext = destinationSpace;
            mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, 0);
            for (frame = 0; frame <= 30.0f; frame++) {
                progress = frame / 30.0f;
                position.x = playerPosition.x + progress * (destination.x - playerPosition.x);
                position.y = (playerPosition.y + progress * (destination.y - playerPosition.y)) +
                    3.0 * (100.0 * sin(M_PI * (180.0f * progress) / 180.0));
                position.z = playerPosition.z + progress * (destination.z - playerPosition.z);
                mbPlayerPosSetV(playerNo, &position);
                HuPrcVSleep();
            }
            GwPlayer[playerNo].masuId = destinationSpace;
            currentSpace = destinationSpace;
            mbev_CapPlayerMotShiftWait(playerNo, 1, HU3D_MOTATTR_LOOP, TRUE);
            mbCameraMoveWait();
ride_finished:
        ;
    }
    if (restored == 0) {
        mbWipeSpecialFadeInCreate(1, 1);
        mbCameraPlayerViewSetFast(playerNo, 2);
        mbCameraMoveWait();
    }
    raftObject->work[3] = 1;
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbPlayerMotionKill(playerNo, rideMotion);
    mbPlayerMotionKill(playerNo, fallMotion);
    mbObjDispSet(((W05_ARRIVAL_WORK *)board)->arrivalModel, FALSE);
    mbObjKill(modelId);
    for (frame = 0; frame < GW_PLAYER_MAX; frame++) {
        mbPlayerDispSet(frame, TRUE);
    }
    mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
    if (restored == 0) {
        mbCameraPlayerViewSetFast(playerNo, 2);
    } else {
        mbCameraPlayerViewSet(playerNo, 2);
    }
    mbCameraMoveWait();
    if (restored == 0) {
        mbWipeSpecialFadeOutCreate(1, 60);
    }
    mbMoveNumDispSet(playerNo, TRUE);
    if (boarded != 0 && GwPlayer[playerNo].moveNum <= 1 && !_CheckFlag(FLAG_BOARD_DEBUG)) {
        mbMoveNumDispSet(playerNo, FALSE);
    }
}

HuVecF lbl_1_data_634 = {0.0f, 0.0f, 1.0f};

/* Gets the raft hook transform for the ride event and its per-frame player update. */
void fn_1_B078(HuVecF *position, HuVecF *rotation, Mtx matrix) {
    W05_BOARD_WORK *board;
    Mtx raftMatrix;
    Mtx playerMatrix;
    HuVecF direction;

    board = lbl_1_bss_8;
    Hu3DModelObjMtxGet(mbObjModelIDGet(board->raftParentModel), "ikadahook", raftMatrix);
    Hu3DModelObjMtxGet(mbObjModelIDGet(board->raftModel), "playerhook", playerMatrix);
    PSMTXConcat(raftMatrix, playerMatrix, playerMatrix);
    if (matrix != NULL) {
        PSMTXCopy(playerMatrix, matrix);
    }
    if (position != NULL) {
        position->x = playerMatrix[0][3];
        position->y = playerMatrix[1][3];
        position->z = playerMatrix[2][3];
    }
    if (rotation != NULL) {
        playerMatrix[0][3] = playerMatrix[1][3] = playerMatrix[2][3] = 0.0f;
        PSMTXMultVec(playerMatrix, &lbl_1_data_634, &direction);
        rotation->x = 180.0 *
            (atan2(-direction.y, sqrtf(direction.x * direction.x + direction.z * direction.z)) /
             M_PI);
        rotation->y = 180.0 * (atan2(direction.x, direction.z) / M_PI);
        rotation->z = 0.0f;
    }
}

/* Updates the raft motion and ride effects once per object-manager frame. */
void fn_1_B304(OMOBJ *object) {
    W05_BOARD_WORK *board;
    HuVecF position;
    HuVecF rotation;
    f32 progress;
    f32 phase;
    f32 previousProgress;
    f32 motionTime;
    f32 motionMaxTime;
    s32 modelId;
    f32 motionSpeed;

    board = lbl_1_bss_8;
    if (mbExitCheck() != 0 || object->work[3] != 0) {
        omDelObjEx(mbObjMan, object);
        return;
    }
    modelId = mbObjModelIDGet(board->raftParentModel);
    motionTime = mbObjMotionTimeGet(board->raftParentModel);
    motionMaxTime = mbObjMotionMaxTimeGet(board->raftParentModel);
    motionSpeed = mbObjMotionSpeedGet(board->raftParentModel);
    progress = motionTime / motionMaxTime;
    board->raftMotionFrame = motionTime;
    board->raftMotionFrames = motionMaxTime;
    board->raftPreviousProgress = board->raftMotionProgress;
    board->raftMotionProgress = progress;
    if (board->raftEventState == 0) {
        W05ModelPauseOn(board->raftParentModel);
        return;
    }
    W05ModelPauseOff(board->raftParentModel);
    switch (object->work[0]) {
    case 0:
        mbObjMotionTimeSet(board->raftParentModel, 0.0f);
        mbObjMotionSpeedSet(board->raftParentModel, 0.0f);
        object->work[1] = 0;
        break;
    case 1:
        if (progress < 0.15f) {
            mbObjMotionSpeedSet(board->raftParentModel, 1.0f);
        } else if (progress < 0.32f) {
            phase = (progress - 0.15f) / 0.15f;
            if (phase > 1.0f) {
                phase = 1.0f;
            }
            mbObjMotionSpeedSet(board->raftParentModel,
                               1.0f + phase * board->raftMotionBoost);
        } else if (progress < 0.9f) {
            mbObjMotionSpeedSet(board->raftParentModel, 1.0f + board->raftMotionBoost);
        } else {
            phase = (progress - 0.9f) / 0.1f;
            mbObjMotionSpeedSet(board->raftParentModel,
                               1.0 + board->raftMotionBoost * cos(M_PI * (90.0f * phase) / 180.0));
        }
        fn_1_B078(&position, &rotation, NULL);
        if (progress >= 0.0f && progress < 0.15f) {
            phase = progress / 0.15f;
            fn_1_BB68(1, phase, 0);
        } else if (progress > 0.85f) {
            phase = (progress - 0.85) / 0.15f;
            fn_1_BB68(1, cos(M_PI * (90.0f * phase) / 180.0), 0);
        } else if (progress >= 0.39f && progress < 0.41) {
        } else if (progress >= 0.57f && progress < 0.6) {
        } else if (progress >= 0.68f && progress < 0.72) {
        } else if ((progress >= 0.41 && progress < 0.45) ||
                   (progress >= 0.6 && progress < 0.65) ||
                   (progress >= 0.72 && progress < 0.76)) {
            previousProgress = board->raftPreviousProgress;
            if ((previousProgress >= 0.4 && previousProgress < 0.41) ||
                (previousProgress >= 0.59 && previousProgress < 0.6) ||
                (previousProgress >= 0.71 && previousProgress < 0.72)) {
                mbAudFXPlay(1506);
            }
            fn_1_BB68(3, 1.2f, 1);
        } else {
            fn_1_BB68(2, 1.0f, 0);
        }
        if (progress >= 1.0f) {
            object->work[2] = 1;
            object->work[0]++;
        }
        break;
    case 2:
        break;
    default:
        omDelObjEx(mbObjMan, object);
        break;
    }
}

/* Emits raft spray particles during the ride object's motion update. */
void fn_1_BB68(s32 count, f32 strength, s32 bothSides) {
    Mtx matrix;
    HuVecF position;
    s32 particleIndex;

    fn_1_B078(NULL, NULL, matrix);
    strength *= 0.75f;
    for (particleIndex = 0; particleIndex < count * 2; particleIndex++) {
        position.x = 2.0f * (100.0f * (-0.5f +
            3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
        position.y = strength * (0.5f * (100.0f *
            (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))));
        position.z = 100.0f * (1.5f + 0.5f *
            (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
        PSMTXMultVec(matrix, &position, &position);
        fn_1_19A90(&position, strength);
        if (bothSides != 0) {
            position.x = 2.0f * (100.0f * (-0.5f +
                3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
            position.y = strength * (0.5f * (100.0f *
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))));
            position.z = -(100.0f * (1.0f +
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))));
            PSMTXMultVec(matrix, &position, &position);
            fn_1_19A90(&position, strength);
        }
    }
    for (particleIndex = 0; particleIndex < count; particleIndex++) {
        position.x = 100.0f * (1.0f + 0.5f *
            (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
        position.y = strength * (0.5f * (100.0f *
            (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))));
        position.z = 2.5f * (100.0f * (-0.5f +
            (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))));
        PSMTXMultVec(matrix, &position, &position);
        fn_1_19A90(&position, strength);
        position.x = -(100.0f * (1.0f + 0.5f *
            (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))));
        position.y = strength * (0.5f * (100.0f *
            (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))));
        position.z = 2.0f * (100.0f * (-0.5f +
            (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))));
        PSMTXMultVec(matrix, &position, &position);
        fn_1_19A90(&position, strength);
    }
}

HuVecF lbl_1_data_64C = {-791.0f, 488.0f, 1690.0f};
HuVecF lbl_1_data_658 = {322.0f, 0.0f, 0.0f};
f32 lbl_1_data_664 = 2400.0f;
f32 lbl_1_data_668[3][2] = {
    {-0.2f, 0.6f},
    {0.2f, 0.6f},
    {0.2f, -0.6f}
};
f32 lbl_1_data_680[3][2] = {
    {0.3f, 0.3f},
    {0.0f, 0.3f},
    {0.0f, 0.2f}
};
f32 lbl_1_data_698[3] = {190.0f, 160.0f, 120.0f};
f32 lbl_1_data_6A4[3] = {-1.0f, 0.0f, 1.0f};
f32 lbl_1_data_6B0[3] = {-1.0f, -1.0f, -1.0f};
HuVecF lbl_1_data_6BC[9] = {
    {0.0f, 0.0f, 0.0f},
    {-80.0f, 0.0f, -80.0f},
    {80.0f, 0.0f, -80.0f},
    {-80.0f, 0.0f, 80.0f},
    {80.0f, 0.0f, 80.0f},
    {-100.0f, 0.0f, 0.0f},
    {100.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, -100.0f},
    {0.0f, 0.0f, 100.0f}
};
f32 lbl_1_data_728[4][2] = {
    {-1.0f, -0.0f},
    {-0.7f, 0.3f},
    {-0.3f, 0.7f},
    {0.0f, 1.0f}
};
s32 lbl_1_data_748[3] = {1, 2, 3};
HuVecF lbl_1_data_754[2] = {
    {2550.0f, 135.0f, 2755.0f},
    {3220.0f, 135.0f, 2755.0f}
};

/* Runs the optional raft detour when its ride event reaches the turn point. */
void fn_1_C398(s32 playerNo) {
    HuVecF cameraEye;
    HuVecF modelPosition;
    HuVecF modelRotation;
    HuVecF burstPosition;
    HuVecF burstRotation;
    int motions[16];
    W05_BOARD_WORK *board;
    s32 cameraModel;
    s32 phase;
    s32 characterModel;
    s32 frame;
    s32 soundId;
    f32 progress;
    f32 motionMaxTime;
    f32 motionStartTime;

    board = lbl_1_bss_8;
    mbCameraEyeGet(&cameraEye);
    cameraModel = mbObjCreate(DATANUM(DATA_capsule, 68), NULL, 0);
    mbObjDispSet(cameraModel, 0);
    mbObjPosSetV(cameraModel, &lbl_1_data_64C);
    mbCameraMoveObj(cameraModel, &lbl_1_data_658, NULL, lbl_1_data_664, -1.0f, -1);
    mbCameraMoveWait();
    motions[0] = DATANUM(DATA_capsulechar1, 1);
    motions[1] = DATANUM(DATA_capsulechar1, 2);
    motions[2] = DATANUM(DATA_capsulechar1, 3);
    motions[3] = DATANUM(DATA_capsulechar1, 4);
    motions[4] = DATANUM(DATA_capsulechar1, 8);
    motions[5] = DATANUM(DATA_capsulechar1, 10);
    motions[6] = -1;
    characterModel = mbObjCreate(DATANUM(DATA_capsulechar1, 0), motions, 0);
    mbObjMotionSet(characterModel, 1, HU3D_MOTATTR_LOOP);
    mbObjLayerSet(characterModel, 3);
    mbObjMotionTimeSet(board->secondHookModel, mbObjMotionMaxTimeGet(board->secondHookModel));
    Hu3DMotionCalc(mbObjModelIDGet(board->secondHookModel));
    HuPrcVSleep();
    fn_1_7AF8(0, &modelPosition, NULL, NULL);
    mbObjPosSetV(characterModel, &modelPosition);
    fn_1_8124(characterModel, 0);
    mbWipeDissolveFadeInTime(5);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 23), -1);
    mbWinTopWait();
    mbAudFXDelaySet(30);
    mbAudFXPlay(971);
    mbev_CapPlayerMotShiftSet(characterModel, 3, 0, 1);
    for (phase = 0; phase < 3; phase++) {
        if (phase == 0) {
            mbObjMotionShiftSet(characterModel, 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbObjRotGet(characterModel, &modelRotation);
            for (frame = 1; frame <= 60.0f; frame++) {
                progress = (f32)frame / 60.0f;
                mbObjRotSet(characterModel, 0.0f,
                    mbev_CapAngleSumLerp(progress, modelRotation.y,
                                        lbl_1_data_698[phase]), 0.0f);
                HuPrcVSleep();
            }
            mbObjMotionShiftSet(characterModel, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        }
        soundId = mbAudFXPlay(1513);
        motionMaxTime = mbObjMotionMaxTimeGet(board->secondHookModels[0]);
        motionStartTime = mbObjMotionTimeGet(board->secondHookModels[0]);
        for (frame = 1; frame <= 60.0f; frame++) {
            progress = (f32)frame / 60.0f;
            mbObjMotionTimeSet(board->secondHookModels[0],
                motionStartTime + (motionMaxTime * lbl_1_data_680[phase][0]) *
                    sin(M_PI * (180.0f * progress) / 180.0) +
                (motionMaxTime * lbl_1_data_668[phase][0]) *
                    sin(M_PI * (90.0f * progress) / 180.0));
            HuPrcVSleep();
        }
        if (soundId != -1) {
            mbAudFXStop(soundId);
        }
        HuPrcSleep(6);
        if (phase == 0) {
            soundId = mbAudFXPlay(1513);
            motionMaxTime = mbObjMotionMaxTimeGet(board->secondHookModels[1]);
            motionStartTime = mbObjMotionTimeGet(board->secondHookModels[1]);
            for (frame = 1; frame <= 60.0f; frame++) {
                progress = (f32)frame / 60.0f;
                mbObjMotionTimeSet(board->secondHookModels[1],
                    motionStartTime + (motionMaxTime * lbl_1_data_680[phase][1]) *
                        sin(M_PI * (180.0f * progress) / 180.0) +
                    (motionMaxTime * lbl_1_data_668[phase][1]) *
                        sin(M_PI * (90.0f * progress) / 180.0));
                HuPrcVSleep();
            }
            if (soundId != -1) {
                mbAudFXStop(soundId);
            }
            HuPrcSleep(6);
        }
        mbAudFXPlay(1522);
        mbAudFXDelaySet(30);
        mbAudFXPlay(973);
        mbev_CapPlayerMotShiftSet(characterModel, 4, 0, 1);
        mbObjMotionTimeSet(board->secondHookAttachmentModel, 0.0f);
        mbObjMotionSpeedSet(board->secondHookAttachmentModel, 2.0f);
        while (mbObjMotionTimeGet(board->secondHookAttachmentModel) <
               mbObjMotionMaxTimeGet(board->secondHookAttachmentModel)) {
            HuPrcVSleep();
        }
        mbAudFXPlay(1514);
        mbCameraShakeSet(12, 50.0f);
        fn_1_7AF8(3, NULL, &burstRotation, NULL);
        fn_1_7AF8(4, &burstPosition, NULL, NULL);
        burstPosition.x += 0.5 * (100.0 *
            (cos(M_PI * burstRotation.x / 180.0) * sin(M_PI * burstRotation.y / 180.0)));
        burstPosition.y += 0.5 * (100.0 * sin(M_PI * burstRotation.x / 180.0));
        burstPosition.z += 0.5 * (100.0 *
            (cos(M_PI * burstRotation.x / 180.0) * cos(M_PI * burstRotation.y / 180.0)));
        fn_1_1F57C(burstPosition, burstRotation, 16);
        omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_8670);
        board->movementFactorX = lbl_1_data_6A4[phase];
        board->movementFactorZ = lbl_1_data_6B0[phase];
        HuPrcSleep(30);
    }
    mbAudFXDelaySet(30);
    mbAudFXPlay(971);
    mbev_CapPlayerMotShiftSet(characterModel, 3, HU3D_MOTATTR_LOOP, 1);
    HuPrcSleep(120);
    mbWipeDissolveFadeOutTime(1);
    mbObjMotionTimeSet(board->secondHookModel, 0.0f);
    mbObjMotionSpeedSet(board->secondHookModel, 0.0f);
    mbObjMotionTimeSet(board->secondHookModels[0],
                       mbObjMotionMaxTimeGet(board->secondHookModels[0]) / 2.0f);
    mbObjMotionTimeSet(board->secondHookModels[1],
                       mbObjMotionMaxTimeGet(board->secondHookModels[1]) / 3.0f);
    mbObjMotionTimeSet(board->secondHookModels[2], 0.0f);
    mbObjMotionSpeedSet(board->secondHookModels[2], 0.0f);
    fn_1_8124(-1, 0);
    mbObjKill(characterModel);
    mbCameraFocusPlayerSet(playerNo);
    mbCameraEyeSetV(&cameraEye);
    mbCameraMoveOnSet(0);
    mbCameraMoveWait();
    mbCameraMoveOnSet(1);
    mbObjKill(cameraModel);
}

/* Carries the player along an arc to the first space with the event's supplied attribute. */
void fn_1_CE08(s32 playerNo, u32 spaceAttribute, s32 duration) {
    Mtx playerMatrix;
    HuVecF startPosition;
    HuVecF position;
    HuVecF velocity;
    HuVecF destination;
    HuVecF rotation;
    HuVecF displacement;
    GXColor color;
    W05_BOARD_WORK *board;
    s32 step;
    s32 destinationSpace;
    f32 distance;
    f32 progress;

    board = lbl_1_bss_8;
    for (step = 1; step < mbMasuNumGet(); step++) {
        if (spaceAttribute & mbMasuMAttrGet(step)) {
            break;
        }
    }
    destinationSpace = step;
    mbMasuPosGet(destinationSpace, &destination);
    mbPlayerPosGet(playerNo, &startPosition);
    PSVECSubtract(&destination, &startPosition, &displacement);
    distance = PSVECMag(&displacement);
    PSMTXIdentity(playerMatrix);
    mbPlayerMtxSet(playerNo, &playerMatrix);
    mbPlayerMotionSet(playerNo, 9, 0);
    board->raftEventState = 0;
    mbObjMotionSpeedSet(board->raftModel, 1.0f);
    GwPlayer[playerNo].masuIdNext = destinationSpace;
    for (step = 1; step <= duration; step++) {
        if (step == (s32)(0.33f * duration)) {
            mbPlayerMotionShiftSet(playerNo, 6, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        }
        progress = (f32)step / duration;
        mbev_CapVecChase(progress, &startPosition, &destination, &position);
        position.y += distance * sin(M_PI * (180.0f * progress) / 180.0);
        rotation.x = -720.0 * sin(M_PI * (90.0f * progress) / 180.0);
        rotation.y = 360.0 * sin(M_PI * (90.0f * progress) / 180.0);
        rotation.z = 0.0f;
        mbPlayerPosSetV(playerNo, &position);
        mbPlayerRotSetV(playerNo, &rotation);
        velocity.x = velocity.y = velocity.z = 0.0f;
        color.a = 128;
        fn_1_1EE80(position, velocity,
            100.0f * (0.5f + 0.5f *
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            10.0f * (-0.5f + (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            0.5f + 0.3f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)), color);
        HuPrcVSleep();
    }
    GwPlayer[playerNo].masuId = destinationSpace;
    mbPlayerColSnapPlayerSet(playerNo, 1);
    mbCameraMoveWait();
    mbCameraPlayerViewSet(playerNo, 0);
    mbCameraMoveWait();
    mbObjMotionSpeedSet(board->raftModel, 0.0f);
    mbObjMotionTimeSet(board->raftModel, 0.0f);
    mbObjMotionSpeedSet(board->raftParentModel, 0.0f);
    mbObjMotionTimeSet(board->raftParentModel,
                      mbObjMotionMaxTimeGet(board->raftParentModel));
    HuPrcSleep(90);
}

/* Runs the raft ride event: deducts coins, moves the player with the raft hook, then lands at
 * spaceId. */
void fn_1_D390(s32 playerNo, s32 spaceId, s32 motionNo, s32 soundId, HuVecF *offset) {
    Mtx matrix;
    HuVecF startPosition;
    HuVecF attachmentPosition;
    HuVecF position;
    HuVecF velocity;
    HuVecF spacePosition;
    HuVecF rotation;
    HuVecF effectPosition;
    GXColor color;
    f32 distance;
    W05_BOARD_WORK *board;
    s32 coinLoss;
    s32 duration;
    s32 frame;
    s32 particleIndex;
    f32 progress;

    board = lbl_1_bss_8;
    mbMasuPosGet(spaceId, &spacePosition);
    mbPlayerPosGet(playerNo, &startPosition);
    attachmentPosition = startPosition;
    PSVECAdd(&attachmentPosition, offset, &attachmentPosition);
    PSVECSubtract(&spacePosition, &startPosition, &effectPosition);
    distance = PSVECMag(&effectPosition);
    PSMTXIdentity(matrix);
    mbPlayerMtxSet(playerNo, &matrix);
    mbPlayerMotionSet(playerNo, 9, 0);
    mbPlayerMotionSpeedSet(playerNo, 0.5f);
    board->raftEventState = 0;
    mbObjMotionTimeSet(board->raftModel, 0.0f);
    mbObjMotionSpeedSet(board->raftModel, 1.0f);
    coinLoss = mbPlayerCoinGet(playerNo);
    if (coinLoss > 5) {
        coinLoss = 5;
    }
    mbCoinAddDispExec(playerNo, -coinLoss, 0, 1);
    effectPosition = startPosition;
    effectPosition.y += 250.0f;
    if (coinLoss >= 1) {
        mbCoinDispCreate(&effectPosition, -coinLoss, -1, 1);
    }
    duration = 60;
    for (frame = 1; frame <= duration; frame++) {
        if (frame == (s32)(0.33f * duration)) {
            mbPlayerMotionShiftSet(playerNo, 6, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        }
        progress = (f32)frame / (f32)duration;
        mbev_CapVecChase(progress, &startPosition, &attachmentPosition, &position);
        position.y += 600.0 * sin(M_PI * (180.0f * progress) / 180.0);
        rotation.x = -720.0 * sin(M_PI * (90.0f * progress) / 180.0);
        rotation.y = 360.0 * sin(M_PI * (90.0f * progress) / 180.0);
        rotation.z = 0.0f;
        mbPlayerPosSetV(playerNo, &position);
        mbPlayerRotSetV(playerNo, &rotation);
        velocity.x = velocity.y = velocity.z = 0.0f;
        /* The caller sets only opacity; fn_1_1EE80 forces the particle RGB channels to white. */
        color.a = 128;
        fn_1_1EE80(position, velocity,
            100.0f * (0.5f + 0.5f *
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            10.0f * (-0.5f +
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            0.5f + 0.3f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)), color);
        if (progress >= 0.5f && position.y <= startPosition.y) {
            for (particleIndex = 0; particleIndex < 6; particleIndex++) {
                effectPosition.x = startPosition.x + 2.0f * (100.0f * (-0.5f +
                    3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
                effectPosition.y = (startPosition.y + 100.0f * (-0.5f +
                    3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))) - 100.0f;
                effectPosition.z = startPosition.z + 2.0f * (100.0f * (-0.5f +
                    3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
                fn_1_19A90(&effectPosition,
                    1.0f + 3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
            }
        }
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, motionNo, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbCameraFocusReset();
    rotation.x = rotation.y = rotation.z = 0.0f;
    progress = 0.0f;
    for (frame = 0; frame < 120.0f; frame++) {
        mbObjMotionTimeSet(board->raftParentModel,
                           0.5f + mbObjMotionTimeGet(board->raftParentModel));
        Hu3DModelObjMtxGet(mbObjModelIDGet(board->raftParentModel), "ikadahook", matrix);
        position.x = matrix[0][3];
        position.y = matrix[1][3];
        position.z = matrix[2][3];
        PSVECAdd(&position, offset, &position);
        attachmentPosition = position;
        progress += 0.02f;
        if (progress > 1.0f) {
            progress = 1.0f;
        }
        position.y -= 100.0 * sin(M_PI * (180.0f * progress) / 180.0);
        rotation.y += 5.0f;
        mbPlayerPosSetV(playerNo, &position);
        mbPlayerRotSetV(playerNo, &rotation);
        for (particleIndex = 0; particleIndex < 5; particleIndex++) {
            effectPosition.x = attachmentPosition.x + 2.0f * (100.0f * (-0.5f +
                3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
            effectPosition.y = attachmentPosition.y + 100.0f * (-0.5f +
                3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
            effectPosition.z = attachmentPosition.z + 2.0f * (100.0f * (-0.5f +
                3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
            fn_1_19A90(&effectPosition,
                0.5f + 3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        }
        HuPrcVSleep();
    }
    mbAudFXStop(soundId);
    mbWipeSpecialFadeInCreate(1, 1);
    mbPlayerPosSetV(playerNo, &spacePosition);
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    GwPlayer[playerNo].masuId = spaceId;
    mbPlayerDispSet(playerNo, 1);
    mbPlayerMotionSet(playerNo, 6, HU3D_MOTATTR_LOOP);
    mbCameraPlayerViewSetFast(playerNo, 0);
    mbCameraMoveWait();
    mbObjMotionSpeedSet(board->raftModel, 0.0f);
    mbObjMotionTimeSet(board->raftModel, 0.0f);
    mbObjMotionSpeedSet(board->raftParentModel, 0.0f);
    mbObjMotionTimeSet(board->raftParentModel, mbObjMotionMaxTimeGet(board->raftParentModel));
    mbWipeSpecialFadeOutCreate(1, 30);
    HuPrcSleep(90);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    HuPrcSleep(10);
}

s32 fn_1_DFB0(s32 playerNo, s32 spaceId, s32 *occupiedSlots, HuVecF *position,
              s32 skipFirst) {
    HuVecF playerPosition;
    HuVecF offsetPosition;
    HuVecF spacePosition;
    s32 slot;
    s32 otherPlayer;

    mbMasuPosGet(spaceId, &spacePosition);
    occupiedSlots[playerNo] = -1;
    for (slot = 0; slot < 9; slot++) {
        if (skipFirst != 0 && slot == 0) {
            continue;
        }
        for (otherPlayer = 0; otherPlayer < GW_PLAYER_MAX; otherPlayer++) {
            if (otherPlayer != playerNo && slot == occupiedSlots[otherPlayer]) {
                break;
            }
        }
        if (otherPlayer < GW_PLAYER_MAX) {
            continue;
        }
        for (otherPlayer = 0; otherPlayer < GW_PLAYER_MAX; otherPlayer++) {
            if (otherPlayer != playerNo && spaceId == GwPlayer[otherPlayer].masuId) {
                mbPlayerPosGet(otherPlayer, &playerPosition);
                PSVECAdd(&spacePosition, &lbl_1_data_6BC[slot], &offsetPosition);
                PSVECSubtract(&playerPosition, &offsetPosition, &offsetPosition);
                if (PSVECMag(&offsetPosition) < 75.0f) {
                    break;
                }
            }
        }
        if (otherPlayer >= GW_PLAYER_MAX) {
            break;
        }
    }
    occupiedSlots[playerNo] = slot;
    PSVECAdd(&spacePosition, &lbl_1_data_6BC[slot], position);
    return slot;
}

/* Follows the raft hook after the ride and removes the follower when the event ends. */
void fn_1_E120(OMOBJ *object) {
    HuVecF position;
    HuVecF rotation;
    W05_BOARD_WORK *board;
    s32 *modelId;

    board = lbl_1_bss_8;
    modelId = object->data;
    if (mbExitCheck() || object->work[3] != 0) {
        if (object->work[3] != 0) {
            mbObjMotionTimeSet(board->raftParentModel,
                              mbObjMotionMaxTimeGet(board->raftParentModel));
        }
        omDelObjEx(mbObjMan, object);
        return;
    }
    switch (object->work[0]) {
    case 0:
        mbObjMotionTimeSet(board->raftParentModel, 4770.0f);
        mbObjMotionSpeedSet(board->raftParentModel, 5.0f);
        object->work[0]++;
        break;
    case 1:
        fn_1_B078(&position, &rotation, NULL);
        mbObjPosSetV(*modelId, &position);
        mbObjRotSetV(*modelId, &rotation);
        mbObjDispSet(*modelId, 1);
        break;
    }
}

/* Runs the linked-space encounter, staging its characters, camera, dialogue and player choice. */
void fn_1_E4A0(int playerNo, s32 inputSpace) {
    int motionData[16];
    HuVecF destination;
    HuVecF source;
    HuVecF direction;
    HuVecF cameraRotation;
    HuVecF position;
    HuVecF spacePosition;
    HuVecF linkedPosition;
    HuVec2f windowPosition;
    W05_BOARD_WORK *board;
    s32 spaceId;
    int linkedSpace;
    int readId;
    s32 modelId;
    s32 cameraModel;
    s32 frame;
    s32 choice;
    s32 window;
    s32 remainingFrames;
    s32 timer;
    s32 playerFinished;
    s32 modelFinished;
    s32 flyingFinished;
    float startAngle;
    float endAngle;
    float phase;

    board = lbl_1_bss_8;
    spaceId = inputSpace;
    mbMasuPosGet(spaceId, &spacePosition);
linkedSpace = mbMasuAttrFindLink(spaceId, (1 << 13));
    mbMasuPosGet(linkedSpace, &linkedPosition);
    PSVECSubtract(&linkedPosition, &spacePosition, &direction);
    board->effectOrigin = linkedPosition;
    startAngle = -150.0f;
    board->playerFacingAngle = fmod(180.0f + startAngle, 360.0);
    board->effectForward = board->effectOrigin;
    board->effectForward.x += 600.0 * sin(M_PI * (startAngle - 90.0f) / 180.0);
    board->effectForward.z += 600.0 * cos(M_PI * (startAngle - 90.0f) / 180.0);
    board->effectForwardAngle = board->playerFacingAngle - 90.0f;
    board->effectBackward = board->effectOrigin;
    board->effectBackward.x += 600.0 * sin(M_PI * (90.0f + startAngle) / 180.0);
    board->effectBackward.z += 600.0 * cos(M_PI * (90.0f + startAngle) / 180.0);
    board->effectBackwardAngle = 90.0f + board->playerFacingAngle;
    mbPlayerRotateStart(playerNo, 180.0 * (atan2(direction.x, direction.z) / M_PI), 15);
    if ((readId = mbBGRead(DATANUM(DATA_capsulechar3, 0))) != -1) {
        mbBGReadWait(readId);
    }
    motionData[0] = DATANUM(DATA_capsulechar3, 7);
    motionData[1] = DATANUM(DATA_capsulechar3, 8);
    motionData[2] = DATANUM(DATA_capsulechar3, 9);
    motionData[3] = DATANUM(DATA_capsulechar3, 10);
    motionData[4] = -1;
    modelId = mbObjCreate(DATANUM(DATA_capsulechar3, 6), motionData, FALSE);
    mbObjMotionSet(modelId, 1, HU3D_MOTATTR_LOOP);
    mbObjPosSetV(modelId, &linkedPosition);
    HuDataDirClose(DATANUM(DATA_capsulechar3, 0));
    mbObjLayerSet(modelId, 3);
    cameraModel = mbObjCreate(DATANUM(DATA_capsule, 68), NULL, TRUE);
    mbObjDispSet(cameraModel, FALSE);
    mbObjPosSetV(cameraModel, &linkedPosition);
    mbAudFXPlay(1501);
    destination = linkedPosition;
    source = board->movingModelPositions[1];
    PSVECSubtract(&destination, &source, &direction);
    startAngle = 180.0 * (atan2(direction.x, direction.z) / M_PI);
    mbObjRotSet(modelId, 0.0f, startAngle, 0.0f);
    mbObjMotionShiftSet(modelId, 3, 0.0f, 8.0f, 0);
    for (frame = 0; frame <= 30.0f; frame++) {
        phase = frame / 30.0f;
        mbev_CapVecChase(phase, &source, &destination, &position);
        position.y += 3.0 * (100.0 * sin(M_PI * (180.0f * phase) / 180.0));
        mbObjPosSetV(modelId, &position);
        HuPrcVSleep();
    }
    mbObjMotionShiftSet(modelId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    PSVECSubtract(&spacePosition, &linkedPosition, &direction);
    endAngle = 180.0 * (atan2(direction.x, direction.z) / M_PI);
    for (frame = 0; frame <= 6.0f; frame++) {
        phase = frame / 6.0f;
        mbObjRotSet(modelId, 0.0f,
            mbev_CapAngleSumLerp(sin(M_PI * (90.0f * phase) / 180.0),
                startAngle, endAngle), 0.0f);
        HuPrcVSleep();
    }
    mbWipeSpecialFadeInCreate(1, 1);
    mbCameraStackPush();
    PSVECSubtract(&linkedPosition, &spacePosition, &direction);
    cameraRotation.x = 352.0f;
    cameraRotation.y = 30.0f;
    cameraRotation.z = 0.0f;
    position.x = 0.0f;
    position.y = 300.0f;
    position.z = 0.0f;
    mbCameraMoveObj(cameraModel, &cameraRotation, &position, 2258.0f, -1.0f, -1);
    mbCameraMoveWait();
    for (frame = 0; frame < GW_PLAYER_MAX; frame++) {
        if (frame != playerNo) {
            mbPlayerDispSet(frame, FALSE);
        }
    }
    mbWipeSpecialFadeOutCreate(1, 60);
    do {
        mbAudFXPlay(999);
        mbWinCreateChoice(1, MESSNUM(MESS_BOARD_W05, 24), 16, 0);
        if (GwPlayer[playerNo].comF) {
            mbComChoiceLeftSet();
        }
        mbWinTopWait();
        choice = mbWinTopChoiceGet();
        switch (choice) {
        case 0:
            break;
        case 1:
        case -1:
            mbAudFXPlay(1000);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 26), 16);
            mbWinTopWait();
            break;
        default:
            mbev_Scroll(playerNo, 0);
            mbStatusDispSetAll(TRUE);
            break;
        }
    } while (choice == 2);
    if (choice == 0) {
        mbAudFXPlay(1501);
        mbObjRotSet(modelId, 0.0f, 180.0f + startAngle, 0.0f);
        mbObjMotionShiftSet(modelId, 3, 0.0f, 8.0f, 0);
        for (frame = 0; frame <= 30.0f; frame++) {
            phase = frame / 30.0f;
            mbev_CapVecChase(phase, &destination, &source, &position);
            position.y += 3.0 * (100.0 * sin(M_PI * (180.0f * phase) / 180.0));
            mbObjPosSetV(modelId, &position);
            HuPrcVSleep();
        }
        mbObjMotionShiftSet(modelId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        startAngle = fmod(180.0f + startAngle, 360.0);
        PSVECSubtract(&destination, &source, &direction);
        endAngle = 180.0 * (atan2(direction.x, direction.z) / M_PI);
        for (frame = 0; frame <= 6.0f; frame++) {
            phase = frame / 6.0f;
            mbObjRotSet(modelId, 0.0f,
                mbev_CapAngleSumLerp(sin(M_PI * (90.0f * phase) / 180.0),
                    startAngle, endAngle), 0.0f);
            HuPrcVSleep();
        }
        mbev_CapStatusDispSetAll(FALSE, FALSE);
        destination = spacePosition;
        source = linkedPosition;
        mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, 0);
        for (frame = 0; frame <= 30.0f; frame++) {
            phase = frame / 30.0f;
            mbev_CapVecChase(phase, &destination, &source, &position);
            position.y += 2.0 * (100.0 * sin(M_PI * (180.0f * phase) / 180.0));
            mbPlayerPosSetV(playerNo, &position);
            HuPrcVSleep();
        }
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbCameraMoveWait();
        PSVECSubtract(&spacePosition, &linkedPosition, &direction);
        mbPlayerRotateStart(playerNo, 180.0 * (atan2(direction.x, direction.z) / M_PI), 15);
        mbAudFXPlay(999);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 25), 16);
        mbWinTopWait();
        board->collectedCoins = 0;
        fn_1_F5C4(playerNo);
        fn_1_10A18(modelId);
        fn_1_11538();
        window = mbWinCreateHelp(MESSNUM(MESS_BOARD_W05, 29));
        mbWinPosGet(window, &windowPosition);
        mbWinPosSet(window, windowPosition.x, 64.0f + windowPosition.y);
        remainingFrames = 600;
        timer = GameMesCreate(1, remainingFrames / 60, -1, -1);
        HuSprGrpDrawNoSet(GameMesGet(timer)->grpId[0], 32);
        do {
            playerFinished = fn_1_F87C(remainingFrames);
            modelFinished = fn_1_10B90(remainingFrames);
            flyingFinished = fn_1_115CC(remainingFrames);
            HuPrcVSleep();
            if (timer >= 0) {
                GameMesDispSet(timer, 1, (remainingFrames + 59) / 60);
            }
            if (remainingFrames < 0 && timer >= 0) {
                GameMesDispSet(timer, 2, -1);
                timer = -1;
            }
            remainingFrames--;
        } while (remainingFrames >= 0 || !playerFinished || !modelFinished || !flyingFinished);
        mbWinKill(window);
        fn_1_101F8();
        fn_1_114E4();
        fn_1_12288();
        mbPlayerPosGet(playerNo, &destination);
        PSVECSubtract(&linkedPosition, &destination, &direction);
        phase = PSVECMag(&direction) / 10.0f;
        if (phase > 1.0f) {
            mbPlayerMoveExec(playerNo, NULL, &source, phase, NULL, TRUE);
        }
        PSVECSubtract(&spacePosition, &linkedPosition, &direction);
        mbPlayerRotateStart(playerNo, 180.0 * (atan2(direction.x, direction.z) / M_PI), 15);
        while (mbPlayerRotateCheck(playerNo) == 0) {
            HuPrcVSleep();
        }
        mbCoinAddDispExec(playerNo, board->collectedCoins, FALSE, TRUE);
        mbev_CapCoinDisp(playerNo, board->collectedCoins, TRUE, TRUE);
        if (board->collectedCoins > 0) {
            mbAudFXPlay(999);
            mbObjMotionShiftSet(modelId, 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 27), 16);
        } else {
            mbAudFXPlay(1000);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 28), 16);
        }
        mbWinTopWait();
    }
    mbWipeSpecialFadeInCreate(1, 1);
    mbev_CapStatusDispSetAll(TRUE, FALSE);
    for (frame = 0; frame < GW_PLAYER_MAX; frame++) {
        mbPlayerDispSet(frame, TRUE);
    }
    mbPlayerPosSetV(playerNo, &spacePosition);
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
    mbCameraPlayerViewSetFast(playerNo, 0);
    mbCameraMoveWait();
    mbWipeSpecialFadeOutCreate(1, 60);
    mbObjKill(cameraModel);
    mbObjKill(modelId);
}

/* Initializes the board event's per-player movement and computer-player timers. */
void fn_1_F5C4(s32 playerNo) {
    W05_BOARD_WORK *board;
    W05_PLAYER_MOTION *motion;
    void *allocation;

    board = lbl_1_bss_8;
    allocation = HuMemDirectMallocNum(HEAP_HEAP, sizeof(W05_PLAYER_MOTION), HU_MEMNUM_OVL);
    motion = allocation;
    board->playerMotion = motion;
    memset(motion, 0, sizeof(W05_PLAYER_MOTION));
    motion->playerNo = playerNo;
    motion->state = 0;
    motion->stateFrames = 0;
    motion->jumpHoldFrames = motion->reactionFrames = 0;
    motion->stickX = 0;
    motion->stickY = 0;
    motion->buttons = 0;
    motion->buttonsDown = 0;
    motion->movementIndex = 0;
    motion->position.x = motion->position.y = motion->position.z = 0.0f;
    motion->velocity.x = motion->velocity.y = motion->velocity.z = 0.0f;
    motion->rotation.x = motion->rotation.y = motion->rotation.z = 0.0f;
    motion->comJumpWait = 120.0f +
        (45.0f * (f32)GwPlayer[playerNo].comDif +
         45.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
    motion->comJumpHold = 20 - GwPlayer[playerNo].comDif * 4;
    motion->comMoveDirection = 0;
    motion->comMoveWait = 30.0f * (f32)GwPlayer[playerNo].comDif +
        60.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
    motion->comMoveFrames = 20 - GwPlayer[playerNo].comDif * 4;
}

/* Reads or clears player input, advances the event movement state, and places the player. */
s32 fn_1_F87C(s32 inputEnabled) {
    HuVecF position;
    HuVecF rotation;
    W05_BOARD_WORK *board;
    W05_PLAYER_MOTION *player;
    s32 active;
    s32 historyIndex;
    f32 averagedSpeed;
    f32 movementSpeed;
    f32 curveWeight;

    board = lbl_1_bss_8;
    player = board->playerMotion;
    active = 0;
    if (inputEnabled > 0) {
        if (GwPlayer[player->playerNo].comF == 0) {
            player->stickX = HuPadStkX[GwPlayer[player->playerNo].padNo];
            player->stickY = HuPadStkY[GwPlayer[player->playerNo].padNo];
            player->buttons = HuPadBtn[GwPlayer[player->playerNo].padNo];
            player->buttonsDown = HuPadBtnDown[GwPlayer[player->playerNo].padNo];
        } else {
            fn_1_1024C();
        }
    } else {
        player->stickX = 0;
        player->stickY = 0;
        player->buttons = 0;
        player->buttonsDown = 0;
    }
    switch (player->state) {
    case 0:
    case 1:
        if (fabs(player->stickX) > 8.0) {
            movementSpeed = 0.22f * player->stickX;
        } else {
            movementSpeed = 0.0f;
        }
        player->movementHistory[player->movementIndex] = movementSpeed;
        if (++player->movementIndex >= 8) {
            player->movementIndex = 0;
        }
        historyIndex = 0;
        averagedSpeed = 0.0f;
        for (; historyIndex < 8; historyIndex++) {
            averagedSpeed += player->movementHistory[historyIndex];
        }
        averagedSpeed *= 0.125f;
        player->position.x += averagedSpeed;
        if (movementSpeed < 0.0f) {
            player->facingDirection = -1.0f;
        } else if (movementSpeed > 0.0f) {
            player->facingDirection = 1.0f;
        }
        if ((player->buttonsDown & 0x100) != 0) {
            player->state = 2;
            player->velocity.y = 35.0f;
            player->jumpHoldFrames = 10;
            mbPlayerMotionShiftSet(player->playerNo, 4, 0.0f, 8.0f, 0);
        } else if (fabs(averagedSpeed) <= 0.1f) {
            mbPlayerMotionShiftSet(player->playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        } else if (fabs(averagedSpeed) <= 7.500000476837158) {
            mbPlayerMotionShiftSet(player->playerNo, 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        } else {
            mbPlayerMotionShiftSet(player->playerNo, 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        }
        player->rotation.y += 20.0f * player->facingDirection;
        active = 1;
        break;
    case 2:
        player->position.y += player->velocity.y;
        if ((player->buttons & 0x100) != 0 && --player->jumpHoldFrames > 0) {
            player->velocity.y -= 1.6333334f;
        } else {
            player->velocity.y -= 3.266667f;
            player->jumpHoldFrames = 0;
        }
        if (fabs(player->stickX) > 8.0) {
            movementSpeed = 0.22f * player->stickX;
        } else {
            movementSpeed = 0.0f;
        }
        if (player->facingDirection * movementSpeed < 0.0f) {
            movementSpeed *= 0.5f;
        }
        player->movementHistory[player->movementIndex] = movementSpeed;
        if (++player->movementIndex >= 8) {
            player->movementIndex = 0;
        }
        historyIndex = 0;
        averagedSpeed = 0.0f;
        for (; historyIndex < 8; historyIndex++) {
            averagedSpeed += player->movementHistory[historyIndex];
        }
        averagedSpeed *= 0.125f;
        player->position.x += averagedSpeed;
        if (player->position.y <= 0.0f) {
            player->state = 0;
        }
        break;
    case 3:
        if (player->stateFrames == 0) {
            mbPlayerMotionShiftSet(player->playerNo, 9, 0.0f, 8.0f, 0);
            player->velocity.y = 35.0f;
            player->reactionFrames = 0;
            player->stateFrames = 60;
        }
        player->position.y += player->velocity.y;
        player->velocity.y -= 3.266667f;
        if (player->position.y <= 0.0f) {
            mbPlayerMotionShiftSet(player->playerNo, 6, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            player->position.y = 0.0f;
            player->state = 5;
        }
        break;
    case 5:
        if (--player->stateFrames <= 0) {
            player->state = 0;
            player->stateFrames = 0;
        }
        active = 1;
        break;
    }
    if (player->position.x > 600.0f) {
        player->position.x = 600.0f;
    }
    if (player->position.x < -600.0f) {
        player->position.x = -600.0f;
    }
    if (player->position.y < 0.0f) {
        player->position.y = 0.0f;
    }
    if (player->rotation.y < -90.0f) {
        player->rotation.y = -90.0f;
    }
    if (player->rotation.y > 90.0f) {
        player->rotation.y = 90.0f;
    }
    curveWeight = fabs(0.0016666667f * player->position.x);
    if (player->position.x < 0.0f) {
        mbev_CapVecChase(curveWeight, &board->effectOrigin, &board->effectBackward, &position);
    } else {
        mbev_CapVecChase(curveWeight, &board->effectOrigin, &board->effectForward, &position);
    }
    position.y += player->position.y;
    rotation = player->rotation;
    rotation.y += board->playerFacingAngle;
    mbPlayerPosSetV(player->playerNo, &position);
    mbPlayerRotSetV(player->playerNo, &rotation);
    return active;
}

/* Releases the temporary buffer retained by the board event work. */
void fn_1_101F8(void) {
    void *savedBuffer[1];
    void *releasedBuffer;
    W05_BOARD_WORK *boardWork = lbl_1_bss_8;

    savedBuffer[0] = boardWork->playerMotion;
    releasedBuffer = boardWork->playerMotion;
    HuMemDirectFree(releasedBuffer);
    boardWork->playerMotion = NULL;
}

/* Chooses computer-player stick and button input from the nearby flying models. */
void fn_1_1024C(void) {
    W05_FLYING_MODEL candidates[32];
    W05_FLYING_MODEL swapModel;
    HuVecF playerPosition;
    HuVecF difference;
    W05_BOARD_WORK *board;
    W05_PLAYER_MOTION *player;
    W05_FLYING_MODEL *model;
    s32 modelIndex;
    s32 candidateIndex;
    s32 candidateCount;
    s32 rejected;
    s32 difficulty;
    s32 playerNo;
    f32 movementScale;
    f32 smoothedStickX;
    f32 smoothingWeight;

    board = lbl_1_bss_8;
    player = board->playerMotion;
    model = board->flyingModels;
    movementScale = 1.0f;
    player->stickX = 0;
    player->stickY = 0;
    player->buttons = 0;
    player->buttonsDown = 0;
    mbPlayerPosGet(player->playerNo, &playerPosition);
    difficulty = GwPlayer[player->playerNo].comDif;
    playerNo = player->playerNo;
    if (board->flyingModels) {
        model = board->flyingModels;
        modelIndex = 0;
        candidateCount = 0;
        for (; modelIndex < 32; modelIndex++, model++) {
            rejected = 0;
            if (model->modelId != -1 && model->state == 2 && model->type == 0) {
                if (model->position.y > 500.0f + board->effectOrigin.y + 100.0f * difficulty) {
                    rejected = 1;
                }
                switch (difficulty) {
                case 0:
                    break;
                case 1:
                    if (model->position.y < 100.0f + board->effectOrigin.y) {
                        rejected = 1;
                    }
                    break;
                case 2:
                    if (model->position.y < 200.0f + board->effectOrigin.y) {
                        rejected = 1;
                    }
                    break;
                default:
                    PSVECSubtract(&playerPosition, &model->position, &difference);
                    if (model->position.y < 300.0f + board->effectOrigin.y &&
                        fabs(difference.x) > 300.0) {
                        rejected = 1;
                    }
                    break;
                }
                if (rejected == 0) {
                    memcpy(&candidates[candidateCount], model, sizeof(W05_FLYING_MODEL));
                    candidateCount++;
                }
            }
        }
        if (candidateCount <= 0) {
            model = board->flyingModels;
            modelIndex = 0;
            candidateCount = 0;
            for (; modelIndex < 32; modelIndex++, model++) {
                rejected = 0;
                if (model->modelId != -1 && model->state <= 2 && model->type == 0) {
                    memcpy(&candidates[candidateCount], model, sizeof(W05_FLYING_MODEL));
                    candidateCount++;
                }
            }
            movementScale = 0.3f * (0.1f * difficulty);
        }
        if (candidateCount > 0) {
            for (modelIndex = 0; modelIndex < candidateCount - 1; modelIndex++) {
                for (candidateIndex = modelIndex + 1; candidateIndex < candidateCount;
                     candidateIndex++) {
                    if (candidates[modelIndex].position.y > candidates[candidateIndex].position.y) {
                        swapModel = candidates[candidateIndex];
                        candidates[candidateIndex] = candidates[modelIndex];
                        candidates[modelIndex] = swapModel;
                    }
                }
            }
            PSVECSubtract(&candidates[0].position, &playerPosition, &difference);
            if (fabs(difference.x) > 20.0) {
                if (difference.x > 0.0f) {
                    player->stickX = 64.0f * movementScale;
                }
                if (difference.x < 0.0f) {
                    player->stickX = -64.0f * movementScale;
                }
            } else if (player->state == 0 || player->state == 1) {
                if (candidateCount > 5 &&
                    100.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)) >
                    (f32)(difficulty * 30)) {
player->buttonsDown = PAD_BUTTON_A;
                }
            } else if (player->state == 2) {
player->buttons = PAD_BUTTON_A;
            }
            if (--player->comJumpWait <= 0) {
                if (player->comJumpWait == 0) {
player->buttonsDown = PAD_BUTTON_A;
                }
player->buttons = PAD_BUTTON_A;
                if (--player->comJumpHold <= 0) {
                    player->comJumpWait = 45.0f * GwPlayer[playerNo].comDif +
                        45.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
                    player->comJumpHold = 20 - GwPlayer[playerNo].comDif * 4;
                }
            }
            smoothingWeight = 0.35f + 0.2f * GwPlayer[playerNo].comDif;
            smoothedStickX = (f32)player->comMoveDirection + smoothingWeight *
                ((f32)player->stickX - (f32)player->comMoveDirection);
            player->stickX = player->comMoveDirection = smoothedStickX;
        }
    }
}

/* Initializes the movement state for a model in the board event. */
void fn_1_10A18(s32 modelId) {
    W05_BOARD_WORK *board;
    W05_MODEL_MOTION *motion;
    void *allocation;

    board = lbl_1_bss_8;
    board->motion = allocation =
        HuMemDirectMallocNum(HEAP_HEAP, sizeof(W05_MODEL_MOTION), HU_MEMNUM_OVL);
    motion = allocation;
    memset(motion, 0, sizeof(W05_MODEL_MOTION));
    motion->modelId = modelId;
    motion->state = 0;
    motion->elapsed = 0;
    motion->delay = 0;
    motion->direction = 1;
    motion->status = 0;
    motion->angle = motion->secondAngle = 0.0f;
    motion->delayTimer =
        60.0f * (3.0f + 2.0f * (3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
    mbObjPosGet(modelId, &motion->position);
    motion->displacement.x = motion->displacement.y = motion->displacement.z = 0.0f;
    mbObjRotGet(modelId, &motion->rotation);
    motion->rotationDelta.x = motion->rotationDelta.y = motion->rotationDelta.z = 0.0f;
}

/* Advances the event model's movement state while the event has time remaining. */
s32 fn_1_10B90(s32 remainingFrames) {
    HuVecF difference;
    HuVecF position;
    W05_BOARD_WORK *board;
    W05_MODEL_MOTION *motion;
    s32 active;
    s32 nextPosition;
    f32 progress;
    f32 launchDirection;

    board = lbl_1_bss_8;
    motion = board->motion;
    active = 0;
    if (motion->delayTimer > 0) {
        motion->delayTimer--;
    }
    switch (motion->state) {
    case 0:
        if (++motion->elapsed >= motion->delay && remainingFrames > 0) {
            motion->elapsed = 0;
            motion->delay = 0;
            motion->state = 1;
        }
        active = 1;
        break;
    case 1:
        do {
            motion->status = (frand() & 0x7FFF) & 3;
        } while (motion->direction == motion->status);
        if ((f32)remainingFrames < 30.0f) {
            motion->elapsed = 0;
            motion->delay = 60;
            motion->state = 0;
        } else {
            motion->elapsed = 0;
            motion->delay = 24;
            motion->state = 2;
        }
        active = 1;
        break;
    case 2:
        if (motion->elapsed == 0) {
            motion->position = board->movingModelPositions[motion->direction];
            if (motion->status > motion->direction) {
                nextPosition = motion->direction + 1;
            } else if (motion->status < motion->direction) {
                nextPosition = motion->direction - 1;
            }
            motion->rotationDelta = board->movingModelPositions[nextPosition];
            PSVECSubtract(&motion->rotationDelta, &motion->position, &difference);
            mbObjRotSet(motion->modelId, 0.0f,
                        180.0 * (atan2(difference.x, difference.z) / M_PI), 0.0f);
            mbAudFXPlay(1501);
            mbObjMotionShiftSet(motion->modelId, 3, 0.0f, 8.0f, 0);
        }
        progress = (f32)++motion->elapsed / (f32)motion->delay;
        mbev_CapVecChase(progress, &motion->position, &motion->rotationDelta, &position);
        position.y += 3.0 * (100.0 * sin(M_PI * (180.0f * progress) / 180.0));
        mbObjPosSetV(motion->modelId, &position);
        if (progress >= 1.0f) {
            if (motion->status > motion->direction) {
                motion->direction++;
            } else if (motion->status < motion->direction) {
                motion->direction--;
            }
            mbObjMotionShiftSet(motion->modelId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            if (motion->status == motion->direction) {
                motion->elapsed = 0;
                motion->delay = 12;
                motion->state = 3;
            } else {
                motion->elapsed = 0;
                motion->delay = 24;
            }
        }
        break;
    case 3:
        if (motion->elapsed == 0) {
            mbObjMotionShiftSet(motion->modelId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbObjRotGet(motion->modelId, &position);
            motion->angle = position.y;
            PSVECSubtract(&board->effectOrigin,
                          &board->movingModelPositions[motion->direction], &difference);
            motion->secondAngle = 180.0 * (atan2(difference.x, difference.z) / M_PI);
        }
        progress = (f32)++motion->elapsed / (f32)motion->delay;
        mbObjRotSet(motion->modelId, 0.0f,
                    mbev_CapAngleSumLerp(progress, motion->angle, motion->secondAngle), 0.0f);
        if (progress >= 1.0f) {
            motion->elapsed = 0;
            motion->delay = 24;
            motion->state = 4;
            mbObjMotionShiftSet(motion->modelId, 4, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        }
        break;
    case 4:
        progress = (f32)++motion->elapsed / (f32)motion->delay;
        if ((motion->elapsed & 3) == 0) {
            mbAudFXPlay(1502);
            mbObjPosGet(motion->modelId, &position);
            position.y += 100.0f;
            launchDirection = lbl_1_data_728[motion->direction][0] +
                (lbl_1_data_728[motion->direction][1] - lbl_1_data_728[motion->direction][0]) *
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
            if (GwSystem.curTime == 0) {
                if ((frand() & 7) == 0) {
                    fn_1_122DC(1, &position, launchDirection, progress);
                } else {
                    fn_1_122DC(0, &position, launchDirection, progress);
                }
            } else if (motion->delayTimer <= 0) {
                fn_1_122DC(2, &position, launchDirection, progress);
                motion->delayTimer = 60.0f * (3.0f + 2.0f *
                    (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
            } else if ((frand() & 3) == 0) {
                fn_1_122DC(1, &position, launchDirection, progress);
            } else {
                fn_1_122DC(0, &position, launchDirection, progress);
            }
        }
        if (progress >= 1.0f || (f32)remainingFrames < 30.0f) {
            motion->elapsed = 0;
            motion->delay = 60.0f * (0.1f + 0.1f *
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
            motion->state = 0;
            mbObjMotionShiftSet(motion->modelId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        }
        active = 1;
        break;
    }
    return active;
}

/* Releases the temporary flying-item records when the board event resets them. */
void fn_1_114E4(void) {
    void *savedBuffer[1];
    void *releasedBuffer;
    W05_BOARD_WORK *boardWork = lbl_1_bss_8;

    savedBuffer[0] = boardWork->motion;
    releasedBuffer = boardWork->motion;
    HuMemDirectFree(releasedBuffer);
    boardWork->motion = NULL;
}

/* Allocates and clears the 32 flying-item records during board event setup. */
void fn_1_11538(void) {
    W05_BOARD_WORK *parent;
    u8 *records;
    u8 *entry;
    s32 i;

    parent = lbl_1_bss_8;
    records = (u8 *)HuMemDirectMallocNum(0, 2560, HU_MEMNUM_OVL);
    entry = records;
    parent->flyingModels = (W05_FLYING_MODEL *)entry;
memset(entry, 0, 2560);
    i = 0;
    while (i < 32) {
        *(s32 *)entry = -1;
        i++;
entry += 80;
    }
}

/* Advances active flying items once per event update and removes them when their time expires. */
s32 fn_1_115CC(s32 remainingFrames) {
    HuVecF position;
    HuVecF particleVelocity;
    HuVecF particlePosition;
    GXColor color;
    W05_BOARD_WORK *board;
    W05_FLYING_MODEL *model;
    s32 activeCount;
    s32 modelIndex;
    f32 progress;
    f32 bounceSpeed;
    f32 bounceAngle;

    board = lbl_1_bss_8;
    model = board->flyingModels;
    activeCount = 0;
    modelIndex = 0;
    for (; modelIndex < 32; modelIndex++, model++) {
        if (model->modelId != -1) {
            switch (model->state) {
            case 0:
                progress = (f32)++model->elapsed / (f32)model->duration;
                mbev_CapVecChase(progress, &model->position, &model->velocity, &position);
                position.y += 3.0 * (100.0 * sin(M_PI * (180.0f * progress) / 180.0));
                switch (model->type) {
                case 0:
                    model->rotationSpeed.y += 5.0f;
                    break;
                case 1:
                    break;
                case 2:
                    particlePosition.x = position.x + 100.0f * (-0.5f +
                        3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
                    particlePosition.y = position.y + 100.0f * (-0.5f +
                        3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
                    particlePosition.z = position.z + 100.0f * (-0.5f +
                        3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
                    particleVelocity.x = particleVelocity.y = particleVelocity.z = 0.0f;
                    color.r = 255;
                    color.g = 255;
                    color.b = 0;
                    color.a = 255;
                    fn_1_1DCE4(particlePosition, particleVelocity,
                        100.0f * (0.5f + 0.3f *
                            (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
                        60.0f * (0.5f + 0.25f *
                            (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
                        0.0f, 0.0f, color);
                    break;
                }
                mbObjPosSetV(model->modelId, &position);
                mbObjRotSetV(model->modelId, &model->rotationSpeed);
                if (progress >= 1.0f) {
                    model->position = position;
                    model->rotation.x = model->rotation.y = model->rotation.z = 0.0f;
                    model->elapsed = 0;
                    model->duration = 0;
                    model->state = 1;
                }
                break;
            case 1:
                if (remainingFrames <= 0) {
                    mbObjKill(model->modelId);
                    model->modelId = -1;
                }
                if (++model->elapsed >= model->delay) {
                    model->elapsed = 0;
                    model->duration = 0;
                    model->state = 2;
                }
                break;
            case 2:
                PSVECAdd(&model->position, &model->rotation, &model->position);
                model->rotation.y -= model->drag;
                model->rotation.y *= 0.95f;
                switch (model->type) {
                case 0:
                    model->rotationSpeed.y += 4.0f + 2.5f *
                        (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
                    break;
                case 1:
                    model->rotationSpeed.x += 4.0f + 2.5f *
                        (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
                    break;
                case 2:
                    particlePosition.x = model->position.x + 100.0f * (-0.5f +
                        3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
                    particlePosition.y = model->position.y + 100.0f * (-0.5f +
                        3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
                    particlePosition.z = model->position.z + 100.0f * (-0.5f +
                        3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
                    particleVelocity.x = particleVelocity.y = particleVelocity.z = 0.0f;
                    color.r = 255;
                    color.g = 255;
                    color.b = 0;
                    color.a = 255;
                    fn_1_1DCE4(particlePosition, particleVelocity,
                        100.0f * (0.5f + 0.3f *
                            (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
                        60.0f * (0.5f + 0.25f *
                            (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
                        0.0f, 0.0f, color);
                    break;
                }
                mbObjPosSetV(model->modelId, &model->position);
                mbObjRotSetV(model->modelId, &model->rotationSpeed);
                if (model->position.y <= 50.0f + board->effectOrigin.y) {
                    model->position.y = 50.0f + board->effectOrigin.y;
                    mbObjPosSetV(model->modelId, &model->position);
                    bounceSpeed = 100.0f * (0.1f + 0.05f *
                        (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
                    bounceAngle = 360.0f *
                        (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
                    model->rotation.x = bounceSpeed * sin(M_PI * bounceAngle / 180.0);
                    model->rotation.z = bounceSpeed * cos(M_PI * bounceAngle / 180.0);
                    model->rotation.y = 100.0f * (0.2f + 0.1f *
                        (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
                    model->elapsed = 0;
                    model->duration = 18;
                    model->state = 3;
                    mbObjKill(model->modelId);
                    model->modelId = -1;
                }
                if (remainingFrames <= 0 && model->modelId >= 0) {
                    mbObjKill(model->modelId);
                    model->modelId = -1;
                    fn_1_1E6D0(&model->position);
                }
                if (remainingFrames >= 0) {
                    fn_1_12720(modelIndex);
                }
                break;
            case 3:
                PSVECAdd(&model->position, &model->rotation, &model->position);
                model->rotation.y -= model->drag;
                if (model->type == 0) {
                    model->rotationSpeed.y += 10.0f;
                }
                mbObjPosSetV(model->modelId, &model->position);
                mbObjRotSetV(model->modelId, &model->rotationSpeed);
                if (model->motionState <= 0) {
                    if ((model->elapsed & 1) != 0) {
                        mbObjDispSet(model->modelId, 0);
                    } else {
                        mbObjDispSet(model->modelId, 1);
                    }
                } else {
                    model->motionState--;
                }
                if (++model->elapsed >= model->duration) {
                    mbObjKill(model->modelId);
                    model->modelId = -1;
                }
                break;
            case 4:
                break;
            }
            activeCount++;
        }
    }
    if (activeCount > 0) {
        return 0;
    }
    return 1;
}

/* Frees the flying-item record array when the board event ends. */
void fn_1_12288(void) {
    void *savedBuffer[1];
    void *releasedBuffer;
    W05_BOARD_WORK *boardWork = lbl_1_bss_8;

    savedBuffer[0] = boardWork->flyingModels;
    releasedBuffer = boardWork->flyingModels;
    HuMemDirectFree(releasedBuffer);
    boardWork->flyingModels = NULL;
}

/* Creates one flying item and initializes its launch, spin and fall state for the event. */
void fn_1_122DC(s32 type, HuVecF *position, f32 direction, f32 strength) {
    W05_BOARD_WORK *board;
    W05_FLYING_MODEL *model;
    s32 slot;
    f32 magnitude;
    s32 dataNum;
    s32 resolved;
    s32 directory;

    board = lbl_1_bss_8;
    model = board->flyingModels;
    for (slot = 0; slot < 32; slot++, model++) {
        if (model->modelId == -1) {
            break;
        }
    }
    if (slot < 32) {
        dataNum = lbl_1_data_50[type];
        dataNum = mbBoardDataNumGet(dataNum);
        directory = DIRNUM(dataNum);
        if (directory != DATA_w05 && directory != DATA_w05n) {
            resolved = dataNum;
        } else {
            dataNum = FILENUM(dataNum);
            if (GwSystem.curTime == 0) {
                dataNum |= DATA_w05;
            } else {
                dataNum |= DATA_w05n;
            }
            resolved = dataNum;
        }
        model->modelId = mbObjCreate(resolved, NULL, TRUE);
        mbObjLayerSet(model->modelId, 3);
        mbObjPosSetV(model->modelId, position);
        model->state = 0;
        model->type = type;
        model->elapsed = 0;
        model->duration = 60.0 * (0.8 + 0.2f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
        model->delay = 30.0f * strength;
        model->motionState = 0;
        model->drag = 0.98f;
        model->position = *position;
        model->rotation.x = model->rotation.y = model->rotation.z = 0.0f;
        model->rotationSpeed.x = model->rotationSpeed.y = model->rotationSpeed.z = 0.0f;
        switch (model->type) {
        case 0:
            model->rotationSpeed.y = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
            break;
        case 1:
            model->rotationSpeed.x = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
            model->rotationSpeed.y = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
            model->rotationSpeed.z = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
            break;
        }
        magnitude = fabs(direction);
        if (direction < 0.0f) {
            mbev_CapVecChase(magnitude, &board->effectOrigin, &board->effectBackward,
                            &model->velocity);
        } else {
            mbev_CapVecChase(magnitude, &board->effectOrigin, &board->effectForward,
                            &model->velocity);
        }
        model->velocity.y += 100.0f * (10.0f + 2.0f * strength);
    }
}

/* Checks a flying item against the active player and applies its coin or hit reaction. */
s32 fn_1_12720(s32 modelIndex) {
    HuVecF modelPosition;
    HuVecF playerPosition;
    HuVecF difference;
    HuVecF burstRotation;
    W05_BOARD_WORK *board;
    W05_PLAYER_MOTION *player;
    W05_FLYING_MODEL *model;
    s32 collided;
    f32 magnitude;
    f32 angle;

    board = lbl_1_bss_8;
    player = board->playerMotion;
    model = &board->flyingModels[modelIndex];
    collided = 0;
    if (model->modelId == -1) {
        return 0;
    }
    if (player->stateFrames != 0) {
        return 0;
    }
    mbPlayerPosGet(player->playerNo, &playerPosition);
    mbObjPosGet(model->modelId, &modelPosition);
    PSVECSubtract(&playerPosition, &modelPosition, &difference);
    magnitude = sqrtf(difference.x * difference.x + difference.z * difference.z);
    switch (model->type) {
    case 0:
        if (magnitude < 150.0f && modelPosition.y < 200.0f + playerPosition.y &&
            modelPosition.y > playerPosition.y - 30.000002f) {
            mbCoinEffCreate(&modelPosition);
            mbObjKill(model->modelId);
            model->modelId = -1;
            board->collectedCoins++;
            collided = 1;
        }
        break;
    case 2:
        if (magnitude < 150.0f && modelPosition.y < 200.0f + playerPosition.y &&
            modelPosition.y > playerPosition.y - 30.000002f) {
            mbCoinEffCreate(&modelPosition);
            mbObjKill(model->modelId);
            model->modelId = -1;
            board->collectedCoins += 5;
            collided = 1;
        }
        break;
    case 1:
        if (magnitude < 100.0f && modelPosition.y < 200.0f + playerPosition.y &&
            modelPosition.y > playerPosition.y - 30.000002f) {
            magnitude = 100.0f * (0.1f + 0.05f *
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
            angle = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
            model->rotation.x = magnitude * sin(M_PI * angle / 180.0);
            model->rotation.z = magnitude * cos(M_PI * angle / 180.0);
            model->rotation.y = 100.0f * (0.2f + 0.1f *
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
            model->elapsed = 0;
            model->duration = 30;
            model->state = 3;
            model->motionState = 18;
            burstRotation.x = 90.0f * (-0.5f +
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
            burstRotation.y = 360.0f * (-0.5f +
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
            burstRotation.z = 0.0f;
            fn_1_1DF14(&modelPosition, &burstRotation);
            omVibrate(player->playerNo, 20, 7, 3);
            mbAudFXPlay(1503);
            player->state = 3;
            collided = 1;
        }
        break;
    }
    return collided;
}

/* Runs the night linked-space encounter, presents its reward choice and restores the board
 * scene. */
void fn_1_12D20(int playerNo, int spaceId) {
    int motionData[16];
    HuVecF characterPositions[2];
    HuVecF characterRotations[2];
    char coinMessage[32];
    HuVecF playerPosition;
    HuVecF destinationPosition;
    HuVecF cameraRotation;
    HuVecF direction;
    HuVecF cameraOffset;
    int characterModels[2];
    W05_BOARD_WORK * board;
    OMOBJ * coinEffect;
    int playerMotion;
    int linkedSpace;
    int backgroundRead;
    int cameraModel;
    int capsule;
    int soundId;
    int coinCount;
    int index;
    f32 rotationWeight;
    board = lbl_1_bss_8;
    soundId = - 1;
linkedSpace = mbMasuAttrFindLink(spaceId, (1 << 13));
    mbMasuPosGet(linkedSpace, & destinationPosition);
    mbPlayerPosGet(playerNo, & playerPosition);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    if ((backgroundRead = mbBGRead(DATANUM(DATA_capsulechar3, 0))) != - 1)
    {
        mbBGReadWait(backgroundRead);
    }
    playerMotion = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 49));
    mbWipeSpecialFadeInCreate(1, 1);
    cameraModel = mbObjCreate(DATANUM(DATA_capsule, 68), NULL, TRUE);
    cameraOffset = playerPosition;
    cameraOffset.z -= 200.0f;
    mbObjDispSet(cameraModel, FALSE);
    mbObjPosSetV(cameraModel, & cameraOffset);
    cameraOffset.x = cameraOffset.z = 0.0f;
    cameraOffset.y = 200.0f;
    cameraRotation.x = - 20.0f;
    cameraRotation.y = - 25.0f;
    cameraRotation.z = 0.0f;
    mbCameraMoveObj(cameraModel, & cameraRotation, & cameraOffset, 1750.0f, - 1.09f, - 1);
    PSVECSubtract(& destinationPosition, & playerPosition, & direction);
    mbPlayerRotSet(playerNo, 0.0f, 180.0 * (atan2(direction.x, direction.z) / M_PI), 0.0f);
    for (index = 0; index < 4; index++)
    {
        if (index != playerNo)
        {
            mbPlayerDispSet(index, FALSE);
        }
    }
    for (index = 0; index < 2; index++)
    {
        motionData[0] = DATANUM(DATA_capsulechar3, 12);
        motionData[1] = DATANUM(DATA_capsulechar3, 13);
        motionData[2] = DATANUM(DATA_capsulechar3, 14);
        motionData[3] = - 1;
        characterModels[index] = mbObjCreate(DATANUM(DATA_capsulechar3, 11), motionData, FALSE);
        mbObjMotionSet(characterModels[index], 1, HU3D_MOTATTR_LOOP);
        mbObjLayerSet(characterModels[index], 3);
        characterPositions[index] = lbl_1_data_754[index];
        PSVECSubtract(& playerPosition, & characterPositions[index], & direction);
        characterRotations[index].x = characterRotations[index].z = 0.0f;
        characterRotations[index].y = 180.0 * (atan2(direction.x, direction.z) / M_PI);
        mbObjPosSetV(characterModels[index], & characterPositions[index]);
        mbObjRotSetV(characterModels[index], & characterRotations[index]);
    }
    HuDataDirClose(DATANUM(DATA_capsulechar3, 0));
    mbCameraMoveWait();
    mbWipeSpecialFadeOutCreate(1, 60);
    mbObjMotionShiftSet(characterModels[0], 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbAudFXPosPlay(960, & characterPositions[0]);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 30), 15);
    mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo), 0);
    mbWinTopWait();
    mbObjMotionShiftSet(characterModels[0], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbObjMotionShiftSet(characterModels[1], 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbAudFXPosPlay(961, & characterPositions[1]);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 31), 15);
    mbWinTopWait();
    mbObjMotionShiftSet(characterModels[0], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbObjMotionShiftSet(characterModels[1], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbWinCreateChoice(2, MESSNUM(MESS_BOARD_W05, 32), - 1, 0);
    if (GwPlayer[playerNo].comF != 0)
    {
        mbComChoiceLeftSet();
    }
    mbWinTopWait();
    if (mbWinTopChoiceGet() == 0)
    {
        mbPlayerMotionShiftSet(playerNo, playerMotion, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
        mbObjMotionShiftSet(characterModels[0], 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbObjMotionShiftSet(characterModels[1], 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        for (index = 1; (f32) index <= 102.0f; index++)
        {
            rotationWeight = (f32) index / 102.0f;
            rotationWeight = sin(M_PI * (90.0f * (rotationWeight * rotationWeight)) / 180.0);
            mbObjRotSet(characterModels[0], characterRotations[0].x,
                        characterRotations[0].y + 360.0f * rotationWeight, characterRotations[0].z);
            mbObjRotSet(characterModels[1], characterRotations[1].x,
                        characterRotations[1].y - 360.0f * rotationWeight, characterRotations[1].z);
            HuPrcVSleep();
        }
        mbObjMotionShiftSet(characterModels[0], 3, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
        mbObjMotionShiftSet(characterModels[1], 3, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
        HuPrcSleep(60);
        mbObjMotionShiftSet(characterModels[0], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbObjMotionShiftSet(characterModels[1], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbAudFXPlay(1518);
        omVibrate(playerNo, 20, 7, 3);
        mbCameraShakeSet(60, 100.0f);
        HuPrcSleep(62);
        mbPlayerMotionShiftSet(playerNo, 9, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
        omVibrate(playerNo, 20, 7, 3);
        mbCameraShakeSet(12, 150.0f);
        HuPrcSleep(12);
        soundId = mbAudFXPlay(1509);
        board->rockingState = 0;
        board->rockingElapsed = 0;
        board->rockingFinished = 0;
        board->rockingResetRequested = 0;
        omAddObjEx(mbObjMan, - 32768, 0, 0, - 1, fn_1_15558);
        HuPrcSleep(30);
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        capsule = lbl_1_data_748[(frand() & 0x7FFF) % 3];
        while (board->rockingState < 1)
        {
            HuPrcVSleep();
        }
        if ((frand() & 0x1F) != 0)
        {
            mbObjMotionShiftSet(characterModels[0], 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbAudFXPosPlay(960, & characterPositions[0]);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 33), 15);
            mbWinTopWait();
            mbObjMotionShiftSet(characterModels[0], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbObjMotionShiftSet(characterModels[1], 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbAudFXPosPlay(960, & characterPositions[1]);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 34), 15);
            mbWinTopWait();
            mbObjMotionShiftSet(characterModels[1], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            if (mbPlayerCapsuleNumGet(playerNo) < mbPlayerCapsuleMaxGet() &&
                (3 & mbRandMod(32768)) != 0) {
                mbCapCapsuleGet(playerNo, capsule);
                mbPlayerCapsuleAdd(playerNo, capsule);
                mbPlayerWinLoseVoicePlay(playerNo, 12, 579);
                mbPlayerMotionShiftSet(playerNo, 12, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
                mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 37), - 1);
                mbWinTopInsertMesSet(mbCapUseMesGet(capsule), 0);
                mbWinTopWait();
            } else {
                coinCount = mbRandMod(6) + 6 + GwPlayer[playerNo].comDif * 3;
                for (index = 0; index < mbPlayerCapsuleNumGet(playerNo); index++)
                {
                    if (mbPlayerCapsuleGet(playerNo, index) >= 0 &&
                        mbPlayerCapsuleGet(playerNo, index) <= 3) {
                        continue;
                    }
                    break;
                }
                if (index >= mbPlayerCapsuleNumGet(playerNo))
                {
                    coinCount += 10;
                }
                coinEffect = mbev_CapEffCoinCreate();
                mbev_CapCoinAdd(coinEffect, playerNo, coinCount, 800);
                mbev_CapEffCoinKill(coinEffect);
                sprintf(coinMessage, "%d", coinCount);
                mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 39), - 1);
                mbWinTopInsertMesSet((u32) coinMessage, 0);
                mbWinTopWait();
            }
            board->rockingState++;
            while (board->rockingFinished == 0)
            {
                HuPrcVSleep();
            }
            HuPrcSleep(30);
        }
        else
        {
            board->rockingState++;
            while (board->rockingFinished == 0)
            {
                HuPrcVSleep();
            }
            HuPrcSleep(30);
            mbObjMotionShiftSet(characterModels[0], 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbAudFXPosPlay(962, & characterPositions[0]);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 35), 15);
            mbWinTopWait();
            mbObjMotionShiftSet(characterModels[0], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbObjMotionShiftSet(characterModels[1], 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbAudFXPosPlay(962, & characterPositions[1]);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 36), 15);
            mbWinTopWait();
            mbObjMotionShiftSet(characterModels[1], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbPlayerMotionShiftSet(playerNo, 13, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 38), - 1);
            mbWinTopWait();
        }
    }
    mbWipeSpecialFadeInCreate(1, 1);
    if (soundId != - 1)
    {
        mbAudFXStop(soundId);
    }
    cameraRotation.x = - 35.0f;
    cameraRotation.y = cameraRotation.z = 0.0f;
    cameraOffset.x = cameraOffset.z = 0.0f;
    cameraOffset.y = 100.0f;
    mbCameraMovePlayer(playerNo, & cameraRotation, & cameraOffset, 2000.0f, - 1.0f, - 1);
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbCameraMoveWait();
    for (index = 0; index < 4; index++)
    {
        mbPlayerDispSet(index, TRUE);
    }
    mbPlayerMotionKill(playerNo, playerMotion);
    mbObjKill(cameraModel);
    for (index = 0; index < 2; index++)
    {
        mbObjKill(characterModels[index]);
    }
    mbWipeSpecialFadeOutCreate(1, 60);
}

HuVecF lbl_1_data_770[2] = {
    {2550.0f, 135.0f, 2755.0f},
    {3220.0f, 135.0f, 2755.0f}
};
HuVecF lbl_1_data_788[14] = {
    {0.0f, 120.0f, 0.0f},
    {0.0f, 133.0f, 0.0f},
    {0.0f, 140.0f, 0.0f},
    {0.0f, 110.0f, 0.0f},
    {0.0f, 133.0f, 0.0f},
    {0.0f, 140.0f, 0.0f},
    {0.0f, 170.0f, 0.0f},
    {0.0f, 60.0f, 0.0f},
    {0.0f, 120.0f, 0.0f},
    {0.0f, 115.0f, 0.0f},
    {0.0f, 60.0f, 0.0f},
    {0.0f, 115.0f, 0.0f},
    {0.0f, 115.0f, 0.0f},
    {0.0f, 115.0f, 0.0f}
};

/* Runs the day linked-space lift scene and resolves the player's choice about moving another
 * player. */
void fn_1_13C3C(int playerNo, int spaceId) {
    int playerMotions[16];
    int motionData[16];
    int liftMotionData[16];
    HuVecF characterPositions[2];
    HuVecF characterRotations[2];
    HuVecF playerPosition;
    HuVecF destinationPosition;
    HuVecF cameraRotation;
    HuVecF direction;
    HuVecF cameraOffset;
    HuVecF liftPosition;
    int playerSpaces[4];
    int liftMotions[4];
    int characterModels[2];
    W05_BOARD_WORK *board;
    s32 playerOffset;
    int linkedSpace;
    int backgroundRead;
    int cameraModel;
    int liftModel;
    int soundId;
    int declined;
    int index;
    int transferIndex;
    int destinationPlayer;
    int movedPlayer;
    f32 progress;

    board = lbl_1_bss_8;
    soundId = -1;
    declined = 0;
linkedSpace = mbMasuAttrFindLink(spaceId, (1 << 13));
    mbMasuPosGet(linkedSpace, &destinationPosition);
    mbPlayerPosGet(playerNo, &playerPosition);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    if ((backgroundRead = mbBGRead(DATANUM(DATA_capsulechar3, 0))) != -1) {
        mbBGReadWait(backgroundRead);
    }
    mbWipeSpecialFadeInCreate(1, 1);
    playerMotions[0] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 49));
    playerMotions[1] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 23));
    playerMotions[2] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 25));
    for (index = 0; index < 4; index++) {
        liftMotions[index] = mbPlayerMotionCreate(index, DATANUM(DATA_mario, 216));
    }
    cameraModel = mbObjCreate(DATANUM(DATA_capsule, 68), NULL, TRUE);
    cameraOffset = playerPosition;
    cameraOffset.z -= 200.0f;
    mbObjDispSet(cameraModel, FALSE);
    mbObjPosSetV(cameraModel, &cameraOffset);
    cameraOffset.x = cameraOffset.z = 0.0f;
    cameraOffset.y = 200.0f;
    cameraRotation.x = -20.0f;
    cameraRotation.y = -25.0f;
    cameraRotation.z = 0.0f;
    mbCameraMoveObj(cameraModel, &cameraRotation, &cameraOffset, 1750.0f, -1.09f, -1);
    PSVECSubtract(&destinationPosition, &playerPosition, &direction);
    mbPlayerRotSet(playerNo, 0.0f, 180.0 * (atan2(direction.x, direction.z) / M_PI), 0.0f);
    for (index = 0; index < 4; index++) {
        if (index != playerNo) {
            mbPlayerDispSet(index, FALSE);
        }
    }
    for (index = 0; index < 2; index++) {
        motionData[0] = DATANUM(DATA_capsulechar3, 12);
        motionData[1] = DATANUM(DATA_capsulechar3, 13);
        motionData[2] = DATANUM(DATA_capsulechar3, 14);
        motionData[3] = -1;
        characterModels[index] = mbObjCreate(DATANUM(DATA_capsulechar3, 11), motionData, FALSE);
        mbObjMotionSet(characterModels[index], 1, HU3D_MOTATTR_LOOP);
        mbObjLayerSet(characterModels[index], 3);
        characterPositions[index] = lbl_1_data_770[index];
        PSVECSubtract(&playerPosition, &characterPositions[index], &direction);
        characterRotations[index].x = characterRotations[index].z = 0.0f;
        characterRotations[index].y = 180.0 * (atan2(direction.x, direction.z) / M_PI);
        mbObjPosSetV(characterModels[index], &characterPositions[index]);
        mbObjRotSetV(characterModels[index], &characterRotations[index]);
    }
    mbCameraMoveWait();
    mbWipeSpecialFadeOutCreate(1, 60);
    mbObjMotionShiftSet(characterModels[0], 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbAudFXPosPlay(960, &characterPositions[0]);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 46), 15);
    mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo), 0);
    mbWinTopWait();
    mbObjMotionShiftSet(characterModels[0], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbObjMotionShiftSet(characterModels[1], 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbAudFXPosPlay(961, &characterPositions[1]);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 31), 15);
    mbWinTopWait();
    mbObjMotionShiftSet(characterModels[0], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbObjMotionShiftSet(characterModels[1], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbWinCreateChoice(2, MESSNUM(MESS_BOARD_W05, 32), -1, 0);
    if (GwPlayer[playerNo].comF) {
        mbComChoiceLeftSet();
    }
    mbWinTopWait();
    if (mbWinTopChoiceGet() != 0) {
        declined = 1;
    } else {
        mbPlayerMotionShiftSet(playerNo, playerMotions[0], 0.0f, 8.0f, 0);
        mbObjMotionShiftSet(characterModels[0], 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbObjMotionShiftSet(characterModels[1], 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        for (index = 1; index <= 102.0f; index++) {
            progress = index / 102.0f;
            progress = sin(M_PI * (90.0f * (progress * progress)) / 180.0);
            mbObjRotSet(characterModels[0], characterRotations[0].x,
                characterRotations[0].y + 360.0f * progress, characterRotations[0].z);
            mbObjRotSet(characterModels[1], characterRotations[1].x,
                characterRotations[1].y - 360.0f * progress, characterRotations[1].z);
            HuPrcVSleep();
        }
        mbObjMotionShiftSet(characterModels[0], 3, 0.0f, 8.0f, 0);
        mbObjMotionShiftSet(characterModels[1], 3, 0.0f, 8.0f, 0);
        HuPrcSleep(60);
        mbObjMotionShiftSet(characterModels[0], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbObjMotionShiftSet(characterModels[1], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbAudFXPlay(1518);
        omVibrate(playerNo, 20, 7, 3);
        mbCameraShakeSet(60, 100.0f);
        HuPrcSleep(62);
        mbPlayerMotionShiftSet(playerNo, 9, 0.0f, 8.0f, 0);
        omVibrate(playerNo, 20, 7, 3);
        mbCameraShakeSet(12, 150.0f);
        HuPrcSleep(12);
    }
    if (declined == 0) {
        soundId = mbAudFXPlay(1509);
        board->rockingState = 0;
        board->rockingElapsed = 0;
        board->rockingFinished = 0;
        board->rockingResetRequested = 0;
        omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_15558);
        HuPrcSleep(30);
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        while (board->rockingState < 1) {
            HuPrcVSleep();
        }
        mbObjMotionShiftSet(characterModels[0], 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbAudFXPosPlay(960, &characterPositions[0]);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 40), 15);
        mbWinTopWait();
        mbObjMotionShiftSet(characterModels[0], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbObjMotionShiftSet(characterModels[1], 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbAudFXPosPlay(960, &characterPositions[1]);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 41), 15);
        mbWinTopWait();
        mbObjMotionShiftSet(characterModels[0], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbObjMotionShiftSet(characterModels[1], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    } else {
        mbObjMotionShiftSet(characterModels[0], 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbAudFXPosPlay(962, &characterPositions[0]);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 42), 15);
        mbWinTopWait();
        mbObjMotionShiftSet(characterModels[0], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbObjMotionShiftSet(characterModels[1], 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbAudFXPosPlay(962, &characterPositions[1]);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 43), 15);
        mbWinTopWait();
        mbObjMotionShiftSet(characterModels[0], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        mbObjMotionShiftSet(characterModels[1], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    }
    mbPlayerRotateStart(playerNo, 0, 15);
    liftMotionData[0] = DATANUM(DATA_capsulechar3, 16);
    liftMotionData[1] = -1;
    liftModel = mbObjCreate(DATANUM(DATA_capsulechar3, 15), liftMotionData, FALSE);
    mbObjMotionSet(liftModel, 1, HU3D_MOTATTR_LOOP);
    mbObjDispSet(liftModel, FALSE);
    mbObjLayerSet(liftModel, 3);
    HuDataDirClose(DATANUM(DATA_capsulechar3, 0));
    while (mbPlayerRotateCheck(playerNo) == 0) {
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, playerMotions[1], 0.0f, 8.0f, 0);
    HuPrcSleep(60);
    mbObjDispSet(liftModel, TRUE);
    for (index = 1; index <= 90.0f; index++) {
        progress = index / 90.0f;
        mbPlayerPosGet(playerNo, &playerPosition);
        PSVECAdd(&playerPosition, &lbl_1_data_788[GwPlayer[playerNo].charNo], &liftPosition);
        liftPosition.y += 8.0 * (100.0 * cos(M_PI * (90.0f * progress) / 180.0));
        mbObjPosSetV(liftModel, &liftPosition);
        if (index == 60) {
            mbPlayerMotionShiftSet(playerNo, playerMotions[2], 0.0f, 8.0f, 0);
        }
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, liftMotions[playerNo], 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    HuPrcSleep(8);
    for (index = 1; index <= 120.0f; index++) {
        progress = index / 120.0f;
        cameraOffset = playerPosition;
        cameraOffset.y += 8.0 * (100.0 * cos(M_PI *
            (90.0f * (1.0f - progress)) / 180.0));
        mbPlayerPosSetV(playerNo, &cameraOffset);
        PSVECAdd(&cameraOffset, &lbl_1_data_788[GwPlayer[playerNo].charNo], &liftPosition);
        mbObjPosSetV(liftModel, &liftPosition);
        HuPrcVSleep();
    }
    mbWipeSpecialFadeInCreate(1, 1);
    for (index = 0; index < 4; index++) {
        mbPlayerDispSet(index, FALSE);
        playerSpaces[index] = GwPlayer[index].masuId;
    }
    mbObjDispSet(liftModel, FALSE);
    if (mbRandMod(32768) & 1) {
        playerOffset = mbRandMod(3) + 1;
    } else {
        playerOffset = -(mbRandMod(3) + 1);
    }
    for (transferIndex = 0; transferIndex < 4; transferIndex++) {
        movedPlayer = playerNo + transferIndex;
        if (movedPlayer >= 4) {
            movedPlayer -= 4;
        } else if (movedPlayer < 0) {
            movedPlayer += 4;
        }
        destinationPlayer = movedPlayer + playerOffset;
        if (destinationPlayer >= 4) {
            destinationPlayer -= 4;
        } else if (destinationPlayer < 0) {
            destinationPlayer += 4;
        }
        GwPlayer[movedPlayer].masuId = playerSpaces[destinationPlayer];
        mbMasuPosGet(playerSpaces[destinationPlayer], &playerPosition);
        mbPlayerPosSetV(movedPlayer, &playerPosition);
        mbPlayerRotSet(movedPlayer, 0.0f, 0.0f, 0.0f);
        mbPlayerColSnapPlayerSet(movedPlayer, TRUE);
        mbev_PlayerColMasu(movedPlayer, playerSpaces[destinationPlayer], TRUE);
        cameraOffset.x = cameraOffset.z = 0.0f;
        cameraOffset.y = 100.0f;
        cameraRotation.x = -30.0f;
        cameraRotation.z = 0.0f;
        cameraRotation.y = 0.0f;
        mbCameraMoveMasu(playerSpaces[destinationPlayer], &cameraRotation, &cameraOffset,
            -1.0f, -1.0f, -1);
        mbCameraMoveWait();
        mbWipeSpecialFadeOutCreate(1, 60);
        mbObjDispSet(liftModel, TRUE);
        mbPlayerPosSetV(movedPlayer, &playerPosition);
        mbPlayerDispSet(movedPlayer, TRUE);
        mbPlayerColSnapPlayerSet(movedPlayer, FALSE);
        mbPlayerMotionSet(movedPlayer, liftMotions[movedPlayer], HU3D_MOTATTR_LOOP);
        for (index = 1; index <= 120.0f; index++) {
            progress = index / 120.0f;
            cameraOffset = playerPosition;
            cameraOffset.y += 8.0 * (100.0 * cos(M_PI * (90.0f * progress) / 180.0));
            mbPlayerPosSetV(movedPlayer, &cameraOffset);
            PSVECAdd(&cameraOffset, &lbl_1_data_788[GwPlayer[movedPlayer].charNo], &liftPosition);
            mbObjPosSetV(liftModel, &liftPosition);
            HuPrcVSleep();
        }
        mbPlayerColSnapPlayerSet(movedPlayer, FALSE);
        mbPlayerMotionShiftSet(movedPlayer, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        for (index = 1; index <= 120.0f; index++) {
            progress = index / 120.0f;
            cameraOffset = playerPosition;
            cameraOffset.y += 8.0 * (100.0 * cos(M_PI *
                (90.0f * (1.0f - progress)) / 180.0));
            /* The departing lift keeps using the initiating player's character offset. */
            PSVECAdd(&cameraOffset, &lbl_1_data_788[GwPlayer[playerNo].charNo], &liftPosition);
            mbObjPosSetV(liftModel, &liftPosition);
            HuPrcVSleep();
        }
        mbObjDispSet(liftModel, FALSE);
        mbWipeSpecialFadeInCreate(1, 1);
        mbPlayerDispSet(movedPlayer, FALSE);
        mbev_PlayerColMasu(movedPlayer, playerSpaces[destinationPlayer], TRUE);
    }
    cameraOffset.x = cameraOffset.z = 0.0f;
    cameraOffset.y = 200.0f;
    cameraRotation.x = -20.0f;
    cameraRotation.y = -25.0f;
    cameraRotation.z = 0.0f;
    mbCameraMoveObj(cameraModel, &cameraRotation, &cameraOffset, 1750.0f, -1.09f, -1);
    mbCameraMoveWait();
    mbev_PlayerColMasu(playerNo, GwPlayer[playerNo].masuId, TRUE);
    for (index = 0; index < 4; index++) {
        mbPlayerDispSet(index, FALSE);
    }
    mbWipeSpecialFadeOutCreate(1, 60);
    mbAudFXPosPlay(960, &characterPositions[0]);
    mbObjMotionShiftSet(characterModels[0], 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 44), 15);
    mbWinTopWait();
    mbObjMotionShiftSet(characterModels[0], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbObjMotionShiftSet(characterModels[1], 3, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbAudFXPosPlay(961, &characterPositions[1]);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W05, 45), 15);
    mbWinTopWait();
    mbObjMotionShiftSet(characterModels[0], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbObjMotionShiftSet(characterModels[1], 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbWipeSpecialFadeInCreate(1, 1);
    board->rockingResetRequested = 1;
    if (soundId != -1) {
        mbAudFXStop(soundId);
    }
    cameraRotation.x = -35.0f;
    cameraRotation.y = cameraRotation.z = 0.0f;
    cameraOffset.x = cameraOffset.z = 0.0f;
    cameraOffset.y = 100.0f;
    mbCameraMovePlayer(playerNo, &cameraRotation, &cameraOffset, 2000.0f, -1.0f, -1);
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbCameraMoveWait();
    for (index = 0; index < 4; index++) {
        mbPlayerDispSet(index, TRUE);
    }
    mbPlayerMotionKill(playerNo, playerMotions[0]);
    mbPlayerMotionKill(playerNo, playerMotions[1]);
    mbPlayerMotionKill(playerNo, playerMotions[2]);
    for (index = 0; index < 4; index++) {
        mbPlayerMotionKill(index, liftMotions[index]);
    }
    mbObjKill(cameraModel);
    for (index = 0; index < 2; index++) {
        mbObjKill(characterModels[index]);
    }
    mbObjKill(liftModel);
    mbWipeSpecialFadeOutCreate(1, 60);
}

/* Updates the rocking event model and signals its completion to the waiting event process. */
void fn_1_15558(OMOBJ *object) {
    W05_BOARD_WORK *board;
    f32 progress;
    f32 scale;

    board = lbl_1_bss_8;
    if (mbExitCheck() || board->rockingFinished != 0) {
        omDelObjEx(mbObjMan, object);
        return;
    }
    if (board->rockingResetRequested != 0) {
        mbObjPosSetV(board->rockingModel, &lbl_1_data_0);
        mbObjMotionSpeedSet(board->rockingModel, 0.0f);
        mbObjMotionTimeSet(board->rockingModel, 0.0f);
        mbObjPosSetV(board->rockingEffectModel, &lbl_1_data_C);
        mbObjDispSet(board->rockingEffectModel, FALSE);
        board->rockingFinished = 1;
        return;
    }
    switch (board->rockingState) {
    case 0:
        progress = (f32)++board->rockingElapsed / 60.0f;
        mbObjPosSet(board->rockingModel, lbl_1_data_0.x,
            lbl_1_data_0.y + 200.0 * sin(M_PI * (90.0f * progress) / 180.0),
            lbl_1_data_0.z);
        mbObjMotionSpeedSet(board->rockingModel, 1.0f);
        mbObjPosSet(board->rockingEffectModel, lbl_1_data_C.x,
            lbl_1_data_C.y + 200.0 * sin(M_PI * (90.0f * progress) / 180.0),
            lbl_1_data_C.z);
        if ((scale = progress * progress) < 0.001) {
            scale = 0.001f;
        }
        mbObjScaleSet(board->rockingEffectModel, scale, scale, scale);
        mbObjDispSet(board->rockingEffectModel, TRUE);
        if (progress >= 1.0f) {
            board->rockingElapsed = 0;
            board->rockingState++;
        }
        break;
    case 1:
        if ((board->rockingElapsed = (s32)((f32)board->rockingElapsed +
            (2.0f + 2.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))))) > 360) {
            board->rockingElapsed -= 360;
        }
        mbObjPosSet(board->rockingModel, lbl_1_data_0.x,
            (200.0f + lbl_1_data_0.y) + 0.5 *
                (100.0 * sin(M_PI * board->rockingElapsed / 180.0)), lbl_1_data_0.z);
        mbObjPosSet(board->rockingEffectModel, lbl_1_data_C.x,
            (200.0f + lbl_1_data_C.y) + 0.5 *
                (100.0 * sin(M_PI * board->rockingElapsed / 180.0)), lbl_1_data_C.z);
        break;
    case 2:
        mbObjPosGet(board->rockingModel, &board->rockingPosition);
        mbObjPosGet(board->rockingEffectModel, &board->rockingEffectPosition);
        board->rockingElapsed = 0;
        board->rockingState++;
        return;
    default:
        progress = (f32)++board->rockingElapsed / 30.0f;
        board->rockingPosition.y -= 10.0f * (1.0f + 2.0f * progress);
        mbObjPosSetV(board->rockingModel, &board->rockingPosition);
        mbObjMotionSpeedSet(board->rockingModel, -1.0f);
        board->rockingEffectPosition.y -= 10.0f * (1.0f + 2.0f * progress);
        mbObjPosSetV(board->rockingEffectModel, &board->rockingEffectPosition);
        if ((scale = (1.0f - progress) * (1.0f - progress)) < 0.001f) {
            scale = 0.001f;
        }
        mbObjScaleSet(board->rockingEffectModel, scale, scale, scale);
        if (board->rockingPosition.y <= lbl_1_data_0.y) {
            mbObjPosSetV(board->rockingModel, &lbl_1_data_0);
            mbObjPosSetV(board->rockingEffectModel, &lbl_1_data_C);
            mbObjDispSet(board->rockingEffectModel, FALSE);
            board->rockingFinished = 1;
            mbAudFXPlay(1510);
            if (GwSystem.turnPlayerNo >= 0) {
                omVibrate(GwSystem.turnPlayerNo, 20, 7, 3);
            }
            mbCameraShakeSet(24, 150.0f);
        }
        break;
    }
}

/* Creates the event model and marks the linked hook spaces used by the board event. */
void fn_1_15C98(void) {
    HuVecF spacePosition;
    HuVecF linkPosition;
    HuVecF direction;
    s32 linkedSpace;
    W05_BOARD_WORK *board;
    s32 space;
    s32 selectedSpace;
    s32 hookIndex;
    s32 modelId;

    board = lbl_1_bss_8;
    {
        s32 dataNum;
        s32 resolved;
        s32 directory;

        dataNum = DATANUM(DATA_w05, 35);
        dataNum = mbBoardDataNumGet(dataNum);
        directory = DIRNUM(dataNum);
        if (directory != DATA_w05 && directory != DATA_w05n) {
            resolved = dataNum;
        } else {
            dataNum = FILENUM(dataNum);
            if (GwSystem.curTime == 0) {
                dataNum |= DATA_w05;
            } else {
                dataNum |= DATA_w05n;
            }
            resolved = dataNum;
        }
        modelId = mbObjCreate(resolved, NULL, FALSE);
    }
    board->eventModel = modelId;
    mbObjMotionSpeedSet(modelId, 0.0f);
    if (*(u8 *)lbl_1_bss_4 == 0) {
        mbObjMotionTimeSet(modelId, 0.0f);
    } else {
        mbObjMotionTimeSet(modelId, mbObjMotionMaxTimeGet(modelId) / 2.0f);
    }
    board->eventEnabled = 1;
    {
        s32 dataNum;
        s32 resolved;
        s32 directory;

        dataNum = DATANUM(DATA_w05, 36);
        dataNum = mbBoardDataNumGet(dataNum);
        directory = DIRNUM(dataNum);
        if (directory != DATA_w05 && directory != DATA_w05n) {
            resolved = dataNum;
        } else {
            dataNum = FILENUM(dataNum);
            if (GwSystem.curTime == 0) {
                dataNum |= DATA_w05;
            } else {
                dataNum |= DATA_w05n;
            }
            resolved = dataNum;
        }
        modelId = mbObjCreate(resolved, NULL, FALSE);
    }
    board->eventLampModel = modelId;
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    space = 1;
    hookIndex = 0;
    while (space < mbMasuNumGet()) {
        if ((mbMasuMAttrGet(space) & 0x2000) != 0) {
            selectedSpace = space;
linkedSpace = mbMasuAttrFindLink(selectedSpace, (1 << 13));
            mbMasuPosGet(selectedSpace, &spacePosition);
            mbMasuPosGet(linkedSpace, &linkPosition);
            PSVECSubtract(&spacePosition, &linkPosition, &direction);
            {
                s32 dataNum;
                s32 resolved;
                s32 directory;

                dataNum = DATANUM(DATA_w05, 34);
                dataNum = mbBoardDataNumGet(dataNum);
                directory = DIRNUM(dataNum);
                if (directory != DATA_w05 && directory != DATA_w05n) {
                    resolved = dataNum;
                } else {
                    dataNum = FILENUM(dataNum);
                    if (GwSystem.curTime == 0) {
                        dataNum |= DATA_w05;
                    } else {
                        dataNum |= DATA_w05n;
                    }
                    resolved = dataNum;
                }
                modelId = mbObjCreate(resolved, NULL, TRUE);
            }
            board->linkedHookModels[hookIndex] = modelId;
            mbObjPosSetV(modelId, &linkPosition);
            {
                s32 dataNum;
                s32 resolved;
                s32 directory;

                dataNum = DATANUM(DATA_w05, 37);
                dataNum = mbBoardDataNumGet(dataNum);
                directory = DIRNUM(dataNum);
                if (directory != DATA_w05 && directory != DATA_w05n) {
                    resolved = dataNum;
                } else {
                    dataNum = FILENUM(dataNum);
                    if (GwSystem.curTime == 0) {
                        dataNum |= DATA_w05;
                    } else {
                        dataNum |= DATA_w05n;
                    }
                    resolved = dataNum;
                }
                modelId = mbObjCreate(resolved, NULL, TRUE);
            }
            board->linkedPunchModels[hookIndex] = modelId;
            mbObjRotSet(modelId, 0.0f,
                        180.0 * (atan2(direction.x, direction.z) / M_PI), 0.0f);
            mbObjMotionSpeedSet(modelId, 0.0f);
            mbObjHookSet(board->linkedHookModels[hookIndex], "ukiwahook", board->eventModel);
            mbObjHookSet(board->linkedHookModels[hookIndex], "lamphook", board->eventLampModel);
            mbObjHookSet(board->linkedHookModels[hookIndex], "punchhook",
                         board->linkedPunchModels[hookIndex]);
            board->linkedSpaces[hookIndex] = space;
            hookIndex++;
        }
        space++;
    }
}

f32 lbl_1_data_850[14][2] = {
    {-180.0f, 400.0f},
    {-200.0f, 400.0f},
    {-230.0f, 400.0f},
    {-220.0f, 400.0f},
    {-220.0f, 400.0f},
    {-230.0f, 400.0f},
    {-230.0f, 400.0f},
    {-180.0f, 400.0f},
    {-220.0f, 370.0f},
    {-209.99998f, 400.0f},
    {-180.0f, 400.0f},
    {-209.99998f, 400.0f},
    {-209.99998f, 400.0f},
    {-209.99998f, 400.0f}
};

/* Plays the linked-space jump event and returns the player to the destination board space. */
void fn_1_160E4(s32 playerNo, s32 spaceId) {
    HuVecF startPosition;
    HuVecF linkedPosition;
    HuVecF direction;
    HuVecF landingPosition;
    HuVecF burstRotation;
    HuVecF position;
    s32 motions[8];
    W05_BOARD_WORK *board;
    s32 frame;
    s32 hookIndex;
    s32 startSpace;
    s32 linkedSpace;
    s32 modelId;
    f32 progress;
    f32 endTime;

    board = lbl_1_bss_8;
    for (frame = 0; frame < 5; frame++) {
        if (board->linkedSpaces[frame] == spaceId) {
            break;
        }
    }
    hookIndex = frame;
    startSpace = spaceId;
linkedSpace = mbMasuAttrFindLink(startSpace, (1 << 13));
    mbMasuPosGet(startSpace, &startPosition);
    mbMasuPosGet(linkedSpace, &linkedPosition);
    PSVECSubtract(&linkedPosition, &startPosition, &direction);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbCameraPlayerViewSet(playerNo, 0);
    mbCameraMoveWait();
    motions[0] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 56));
    motions[1] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 8));
    if (PSVECMag(&direction) > 0.0f) {
        PSVECNormalize(&direction, &position);
    }
    PSVECScale(&position, &position, lbl_1_data_850[GwPlayer[playerNo].charNo][0]);
    PSVECAdd(&linkedPosition, &position, &landingPosition);
    landingPosition.y += lbl_1_data_850[GwPlayer[playerNo].charNo][1];
    mbPlayerRotateStart(playerNo, 180.0 * (atan2(direction.x, direction.z) / M_PI), 15);
    while (mbPlayerRotateCheck(playerNo) == 0) {
        HuPrcVSleep();
    }
    CharFXPlay(GwPlayer[playerNo].charNo, 593);
    mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 8.0f, 0);
    for (frame = 1; frame <= 36.0f; frame++) {
        progress = (f32)frame / 36.0f;
        mbev_CapVecChase(progress, &startPosition, &landingPosition, &position);
        position.y += 2.0 * (100.0 * sin(M_PI * (180.0f * progress) / 180.0));
        mbPlayerPosSetV(playerNo, &position);
        if (frame == 27) {
            mbPlayerMotionShiftSet(playerNo, motions[0], 0.0f, 8.0f, 0);
        }
        HuPrcVSleep();
    }
    modelId = board->linkedPunchModels[hookIndex];
    mbObjMotionTimeSet(modelId, 0.0f);
    mbObjMotionSpeedSet(modelId, 1.0f);
    mbAudFXPlay(1516);
    position.x = position.z = 0.0f;
    position.y = 300.0f;
    mbCameraMoveMasu(linkedSpace, NULL, &position, -1.0f, -1.0f, 30);
    if (PSVECMag(&direction) > 0.0f) {
        PSVECNormalize(&direction, &position);
    }
    PSVECScale(&position, &position, -130.0f);
    PSVECAdd(&linkedPosition, &position, &position);
    position.y += 475.0f;
    burstRotation.x = 90.0f + 10.0f * (-0.5f +
        3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
    burstRotation.y = 180.0 * (atan2(direction.x, direction.z) / M_PI);
    burstRotation.z = 0.0f;
    fn_1_1DF14(&position, &burstRotation);
    omVibrate(playerNo, 20, 7, 3);
    mbPlayerMotionShiftSet(playerNo, motions[1], 999.0f, 8.0f, HU3D_MOTATTR_REV);
    for (frame = 1; frame <= 45.0f; frame++) {
        progress = (f32)frame / 45.0f;
        mbev_CapVecChase(progress, &landingPosition, &startPosition, &position);
        position.y += 4.0 * (100.0 * sin(M_PI * (180.0f * progress) / 180.0));
        mbPlayerPosSetV(playerNo, &position);
        if (frame == 39) {
            mbPlayerMotionShiftSet(playerNo, 5, 0.0f, 8.0f, 0);
        }
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    modelId = board->eventLampModel;
    mbObjMotionSpeedSet(modelId, 1.0f);
    mbAudFXPlay(1517);
    modelId = board->eventModel;
    if (*(u8 *)lbl_1_bss_4 == 0) {
        mbObjMotionTimeSet(modelId, 0.0f);
        endTime = mbObjMotionMaxTimeGet(modelId) / 2.0f;
    } else {
        mbObjMotionTimeSet(modelId, mbObjMotionMaxTimeGet(modelId) / 2.0f);
        endTime = mbObjMotionMaxTimeGet(modelId);
    }
    mbObjMotionSpeedSet(modelId, 1.0f);
    while (mbObjMotionTimeGet(modelId) < endTime) {
        HuPrcVSleep();
    }
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbCameraMoveWait();
    mbWipeSpecialFadeInCreate(1, 1);
    fn_1_35F0(0, 1, 0);
    mbCameraPlayerViewSetFast(playerNo, 0);
    mbCameraMoveWait();
    mbPlayerPosSetV(playerNo, &startPosition);
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    modelId = board->eventLampModel;
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbPlayerMotionKill(playerNo, motions[0]);
    mbPlayerMotionKill(playerNo, motions[1]);
    mbWipeSpecialFadeOutCreate(1, 60);
}

GXColor lbl_1_data_8C0 = {255, 255, 255, 255};
GXColor lbl_1_data_8C4 = {255, 255, 255, 255};
GXColor lbl_1_data_8C8 = {127, 127, 127, 24};
f32 lbl_1_data_8CC[2][3] = {
    {0.0f, 0.0f, 0.5f},
    {0.0f, 0.5f, 0.0f}
};
HuVecF lbl_1_data_8E4 = {-9.0f, 905.0f, -1570.0f};
HuVecF lbl_1_data_8F0 = {-1124.0f, 615.0f, -2222.0f};
HuVecF lbl_1_data_8FC = {-62.0f, 210.0f, -636.0f};
HuVecF lbl_1_data_908 = {1267.0f, -170.0f, -422.0f};

/* Builds the sea surface display list and installs its render and wave-update callbacks. */
void fn_1_1698C(void) {
    W05_DRAW_WORK *board;
    HU3D_MODEL *model;
    HSF_DATA *hsf;
    HSF_BUFFER *faces;
    HSF_BUFFER *vertices;
    HSF_FACE *face;
    HuVecF *vertex;
    W05_SURFACE_VERTEX *surfaceVertex;
    GXColor *color;
    HuVecF *positions;
    f32 (*textureCoord)[2];
    f32 (*warpCoord)[2];
    s32 hookModel;
    s32 modelId;
    s32 sourceModel;
    s32 pointCount;
    s32 lineCount;
    s32 triangleCount;
    s32 quadCount;
    s32 otherCount;
    void *displayListStart;
    void *displayList;
    s32 vertexCount;
    s32 faceBufferIndex;
    s32 faceIndex;
    f32 minX;
    f32 maxX;
    f32 minZ;
    f32 maxZ;
    f32 width;
    f32 depth;
    f32 depthWeight;

    board = lbl_1_bss_8;
    board->modelId = modelId = mbObjCreate(W05SurfaceDataNumGet(DATANUM(DATA_w05, 4)), NULL, FALSE);
    mbObjDispSet(modelId, FALSE);
    sourceModel = mbObjModelIDGet(modelId);
    model = &Hu3DData[sourceModel];
    hsf = model->hsf;
    faces = hsf->face;
    vertices = hsf->vertex;
    pointCount = lineCount = triangleCount = quadCount = otherCount = 0;
    for (faceBufferIndex = 0; faceBufferIndex < hsf->faceNum; faceBufferIndex++, faces++) {
        face = faces->data;
        for (faceIndex = 0; faceIndex < faces->count; faceIndex++, face++) {
            switch (face->type & HSF_FACE_MASK) {
            case 0:
                pointCount++;
                break;
            case 1:
                lineCount++;
                break;
            case HSF_FACE_TRI:
                triangleCount++;
                break;
            case HSF_FACE_QUAD:
                quadCount++;
                break;
            default:
                otherCount++;
                break;
            }
        }
    }
    vertexCount = vertices->count;
    board->vertexCount = vertexCount;
    board->textureCoords = textureCoord =
        HuMemDirectMallocNum(HEAP_MODEL, vertexCount * 8, model->mallocNo);
    memset(board->textureCoords, 0, vertexCount * 8);
    board->baseWarpCoords = warpCoord =
        HuMemDirectMallocNum(HEAP_MODEL, vertexCount * 8, model->mallocNo);
    memset(board->baseWarpCoords, 0, vertexCount * 8);
    board->warpCoords = HuMemDirectMallocNum(HEAP_MODEL, vertexCount * 8, model->mallocNo);
    memset(board->warpCoords, 0, vertexCount * 8);
    board->textureOffsetX = 0.0f;
    board->textureOffsetY = 0.0f;
    board->scrollElapsed = board->scrollDuration = 0;
    board->scrollAngle = 0.0f;
    board->scrollAngleRange = 0.0f;
    board->scrollSpeedS = board->scrollSpeedT = 0.0f;
    board->colors = color = HuMemDirectMallocNum(HEAP_MODEL, vertexCount * 4, model->mallocNo);
    memset(color, 0, vertexCount * 4);
    board->positions = positions =
        HuMemDirectMallocNum(HEAP_MODEL, vertexCount * 12, model->mallocNo);
    memset(positions, 0, vertexCount * 12);
    board->surfaceVertices = surfaceVertex =
        HuMemDirectMallocNum(HEAP_MODEL, vertexCount * 48, model->mallocNo);
    memset(surfaceVertex, 0, vertexCount * 48);
    board->warpDisabled = 0;
    board->animation =
        HuSprAnimRead(HuDataReadNum(W05SurfaceDataNumGet(DATANUM(DATA_w05, 44)), HU_MEMNUM_OVL));
    board->warpAnimation =
        HuSprAnimRead(HuDataReadNum(W05SurfaceDataNumGet(DATANUM(DATA_w05, 45)), HU_MEMNUM_OVL));
    vertex = vertices->data;
    for (faceBufferIndex = 0; faceBufferIndex < vertexCount;
         faceBufferIndex++, surfaceVertex++, vertex++) {
        surfaceVertex->phase.x = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        surfaceVertex->phase.y = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        surfaceVertex->phase.z = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        if (vertex->z > -1000.0f) {
            depthWeight = 1.0f;
        } else if (vertex->z < -2000.0f) {
            depthWeight = 0.0f;
        } else {
            depthWeight = (2000.0f + vertex->z) / 1000.0f;
        }
        surfaceVertex->amplitude.x =
            0.3f * (100.0f * (3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
        surfaceVertex->amplitude.y =
            depthWeight * (0.6f * (100.0f * (3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE))));
        surfaceVertex->amplitude.z =
            0.3f * (100.0f * (3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
        surfaceVertex->phaseStep.x = 0.5f + (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        surfaceVertex->phaseStep.y = 0.5f + (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        surfaceVertex->phaseStep.z = 0.5f + (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        surfaceVertex->position.x = vertex->x;
        surfaceVertex->position.y = vertex->y;
        surfaceVertex->position.z = vertex->z;
    }
    memcpy(board->positions, vertices->data, vertexCount * 12);
    vertex = vertices->data;
    maxX = vertex->x;
    minX = maxX;
    maxZ = vertex->z;
    minZ = maxZ;
    board->boundsStart.y = vertex->y;
    for (faceBufferIndex = 0; faceBufferIndex < vertexCount; faceBufferIndex++, vertex++) {
        if (minX > vertex->x) {
            minX = vertex->x;
        }
        if (maxX < vertex->x) {
            maxX = vertex->x;
        }
        if (minZ > vertex->z) {
            minZ = vertex->z;
        }
        if (maxZ < vertex->z) {
            maxZ = vertex->z;
        }
    }
    width = maxX - minX;
    depth = maxZ - minZ;
    board->boundsStart.x = minX;
    board->boundsStart.z = minZ;
    board->boundsSize.x = width;
    board->boundsSize.y = 0.0f;
    board->boundsSize.z = depth;
    vertex = vertices->data;
    for (faceBufferIndex = 0; faceBufferIndex < vertexCount;
         faceBufferIndex++, vertex++, textureCoord++, warpCoord++, color++) {
        (*textureCoord)[0] = (vertex->x - minX) / width;
        (*textureCoord)[1] = (vertex->z - minZ) / depth;
        (*warpCoord)[0] = (vertex->x - minX) / (0.05f * width);
        (*warpCoord)[1] = (vertex->z - minZ) / (0.05f * depth);
        color->r = color->g = color->b = color->a = 255;
    }
    DCFlushRange(board->textureCoords, vertexCount * 8);
    DCFlushRange(board->baseWarpCoords, vertexCount * 8);
    DCFlushRange(board->colors, vertexCount * 4);
    DCFlushRange(board->positions, vertexCount * 4);
    displayListStart = displayList = W05SurfaceScratchAlloc();
    DCFlushRange(displayList, 65536);
    GXBeginDisplayList(displayListStart, 65536);
    GXBegin(GX_TRIANGLES, GX_VTXFMT0, triangleCount * 3);
    faces = hsf->face;
    for (faceBufferIndex = 0; faceBufferIndex < hsf->faceNum; faceBufferIndex++, faces++) {
        face = faces->data;
        for (faceIndex = 0; faceIndex < faces->count; faceIndex++, face++) {
            switch (face->type & HSF_FACE_MASK) {
            default:
                break;
            case HSF_FACE_TRI:
                GXPosition1x16(face->index[0].vertex);
                GXColor1x16(face->index[0].vertex);
                GXTexCoord1x16(face->index[0].vertex);
                GXTexCoord1x16(face->index[0].vertex);
                GXPosition1x16(face->index[2].vertex);
                GXColor1x16(face->index[2].vertex);
                GXTexCoord1x16(face->index[2].vertex);
                GXTexCoord1x16(face->index[2].vertex);
                GXPosition1x16(face->index[1].vertex);
                GXColor1x16(face->index[1].vertex);
                GXTexCoord1x16(face->index[1].vertex);
                GXTexCoord1x16(face->index[1].vertex);
                break;
            }
        }
    }
    board->displayListSize = GXEndDisplayList();
    board->displayList = W05SurfaceListAlloc(board->displayListSize, model->mallocNo);
    memcpy(board->displayList, displayList, board->displayListSize);
    DCFlushRange(board->displayList, board->displayListSize);
    HuMemDirectFree(displayList);
    board->surfaceHookModel = hookModel = Hu3DHookFuncCreate(fn_1_1761C);
    Hu3DModelCameraSet(hookModel, 1);
    board->effectHookModel = Hu3DHookFuncCreate(fn_1_17A84);
    Hu3DModelCameraSet(board->effectHookModel, 1);
    Hu3DModelLayerSet(board->effectHookModel, 7);
}

/* Draws the board's sea surface from the model hook during the 3D render pass. */
void fn_1_1761C(HU3D_MODEL *hookModel, Mtx *matrix) {
    HSF_BUFFER *faces;
    HSF_BUFFER *vertices;
    W05_DRAW_WORK *board;
    HSF_DATA *hsf;
    HU3D_MODEL *model;
    s32 textureFormat;
    s32 modelId;

    board = lbl_1_bss_8;
    modelId = mbObjModelIDGet(board->modelId);
    model = &Hu3DData[modelId];
    hsf = model->hsf;
    faces = hsf->face;
    vertices = hsf->vertex;
    GXLoadPosMtxImm(*matrix, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(2);
    GXSetNumChans(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,
                     GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY,
                     GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
    GXSetTevColor(GX_TEVREG2, lbl_1_data_8C8);
    GXSetChanAmbColor(GX_COLOR0A0, lbl_1_data_8C0);
    GXSetChanMatColor(GX_COLOR0A0, lbl_1_data_8C4);
    textureFormat = board->animation->bmp->dataFmt & ANIM_BMP_FMTMASK;
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
    if (textureFormat == ANIM_BMP_I8 || textureFormat == ANIM_BMP_I4) {
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXA, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    } else {
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    }
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_TRUE, GX_TEVPREV);
    HuSprTexLoad(board->animation, 0, GX_TEXMAP0, GX_MIRROR, GX_MIRROR, GX_LINEAR);
    if (board->warpDisabled == 0) {
        GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY,
                         GX_FALSE, GX_PTIDENTITY);
        GXSetNumIndStages(1);
        GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
        GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
        GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_TRUE, GX_FALSE, GX_ITM_1);
        GXSetIndTexMtx(GX_ITM_1, lbl_1_data_8CC, -1);
        HuSprTexLoad(board->warpAnimation, 0, GX_TEXMAP1, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    }
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL,
                  GX_DF_CLAMP, GX_AF_NONE);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetZCompLoc(GX_TRUE);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    GXSetCullMode(GX_CULL_BACK);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetArray(GX_VA_POS, board->positions, sizeof(HuVecF));
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetArray(GX_VA_CLR0, board->colors, sizeof(GXColor));
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetArray(GX_VA_TEX0, board->textureCoords, 8);
    GXSetVtxDesc(GX_VA_TEX1, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_F32, 0);
    GXSetArray(GX_VA_TEX1, board->warpCoords, 8);
    GXCallDisplayList(board->displayList, board->displayListSize);
    GXSetNumIndStages(0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevDirect(GX_TEVSTAGE1);
}

/* Updates sea waves, water flow, hook motion and nearby particles from the render hook. */
void fn_1_17A84(HU3D_MODEL *hookModel, Mtx *hookMatrix) {
    HuVecF position;
    HSF_BUFFER *faceBuffer;
    HSF_BUFFER *vertexBuffer;
    W05_BOARD_WORK *board;
    HU3D_MODEL *model;
    HSF_DATA *hsf;
    W05_SEA_WAVE *wave;
    HuVecF *vertex;
    W05_TEX_COORD *texCoordBase;
    W05_TEX_COORD *texCoord;
    f32 angle;
    f32 distance;
    f32 factorX;
    f32 factorZ;
    f32 wavePhaseX;
    f32 wavePhaseY;
    f32 wavePhaseZ;
    f32 firstHookHeightPhase;
    f32 firstHookPitchPhase;
    f32 firstHookRollPhase;
    f32 secondHookHeightPhase;
    f32 secondHookPitchPhase;
    f32 secondHookRollPhase;
    f32 firstHookPitchNoise;
    f32 firstHookHeightNoise;
    f32 firstHookRollNoise;
    f32 secondHookPitchNoise;
    f32 secondHookHeightNoise;
    f32 secondHookRollNoise;
    f32 firstAmbientNoise;
    f32 firstAmbientOffset;
    f32 secondAmbientNoise;
    f32 secondAmbientOffset;
    f32 thirdAmbientNoise;
    f32 thirdAmbientOffset;
    f32 fourthAmbientNoise;
    f32 fourthAmbientOffset;
    int modelId;
    s32 i;
    s16 firstHookPitchSample;
    s16 firstHookHeightSample;
    s16 firstHookRollSample;
    s16 secondHookPitchSample;
    s16 secondHookHeightSample;
    s16 secondHookRollSample;
    s16 firstAmbientSample;
    s16 secondAmbientSample;
    s16 thirdAmbientSample;
    s16 fourthAmbientSample;

    board = lbl_1_bss_8;
    if (mbExitCheck()) {
        return;
    }
    modelId = mbObjModelIDGet(board->seaModel);
    model = &Hu3DData[modelId];
    hsf = model->hsf;
    faceBuffer = hsf->face;
    vertexBuffer = hsf->vertex;
    vertex = board->seaVertices;
    wave = board->seaWaves;
    for (i = 0; i < board->seaVertexCount; wave++, vertex++, i++) {
        if (!omPauseChk()) {
            if ((wave->phase.x += wave->speed.x) >= 360.0f) {
                wave->phase.x -= 360.0f;
            }
            if ((wave->phase.y += wave->speed.y) >= 360.0f) {
                wave->phase.y -= 360.0f;
            }
            if ((wave->phase.z += wave->speed.z) >= 360.0f) {
                wave->phase.z -= 360.0f;
            }
        }
        vertex->x =
            wave->base.x + wave->amplitude.x * (wavePhaseX = wave->phase.x, W05SinDeg(wavePhaseX));
        vertex->y = (wave->base.y - wave->amplitude.y) +
                    wave->amplitude.y * (wavePhaseY = wave->phase.y, W05SinDeg(wavePhaseY));
        vertex->z =
            wave->base.z + wave->amplitude.z * (wavePhaseZ = wave->phase.z, W05SinDeg(wavePhaseZ));
    }
    texCoordBase = board->seaTexCoordBase;
    texCoord = board->seaTexCoords;
    if (!omPauseChk()) {
        if (++board->seaFlowTimer >= board->seaFlowDuration) {
            board->seaFlowTimer = 0;
            board->seaFlowDuration =
                60.0f * (10.0f + 10.0f * (3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
            board->seaFlowAmplitude =
                20.0f + 20.0f * (3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE));
            board->seaFlowSpeedS = 0.002f;
            board->seaFlowSpeedT = 0.002f;
        }
        distance = (f32)board->seaFlowTimer / (f32)board->seaFlowDuration;
        board->seaFlowAngle =
            45.0 + board->seaFlowAmplitude * sin(M_PI * (180.0f * distance) / 180.0);
        angle = 0.7f + 0.3 * sin(M_PI * (180.0f * distance) / 180.0);
        board->seaFlowOffsetS -=
            angle * (board->seaFlowSpeedS * sin(M_PI * board->seaFlowAngle / 180.0));
        board->seaFlowOffsetT -=
            angle * (board->seaFlowSpeedT * cos(M_PI * board->seaFlowAngle / 180.0));
    }
    for (i = 0; i < board->seaVertexCount; texCoordBase++, texCoord++, i++) {
        texCoord->s = texCoordBase->s + board->seaFlowOffsetS;
        texCoord->t = texCoordBase->t + board->seaFlowOffsetT;
    }
    DCFlushRangeNoSync(board->seaTexCoordsAlt, board->seaVertexCount * sizeof(W05_TEX_COORD));
    DCFlushRangeNoSync(board->seaTexCoords, board->seaVertexCount * sizeof(W05_TEX_COORD));
    DCFlushRangeNoSync(board->seaVertices, board->seaVertexCount * sizeof(HuVecF));
    PPCSync();
    if (board->firstHookMoving == 0 && board->particlesEnabled != 0) {
        for (i = 0; i < 5; i++) {
            angle = 230.0f * (-0.5f + 3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
            W05_EFFECT_RAND_NEXT();
            firstAmbientSample = mbCapEffData[mbCapEffNum];
            firstAmbientNoise = 3.051851e-05f * firstAmbientSample;
            firstAmbientOffset = firstAmbientNoise - 0.5f;
            distance = 100.0f * (1.5f + 0.5f * firstAmbientOffset);
            position.x = lbl_1_data_8E4.x + distance * W05SinDeg(angle);
            position.y = lbl_1_data_8E4.y;
            position.z = lbl_1_data_8E4.z + distance * W05CosDeg(angle);
            fn_1_19A90(&position,
                       1.3f + 0.2f * (-0.5f + 3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
        }
        for (i = 0; i < 3; i++) {
            angle = -150.0f;
            W05_EFFECT_RAND_NEXT();
            secondAmbientSample = mbCapEffData[mbCapEffNum];
            secondAmbientNoise = 3.051851e-05f * secondAmbientSample;
            secondAmbientOffset = secondAmbientNoise - 0.5f;
            distance = 100.0f * (6.0f * secondAmbientOffset);
            position.x = lbl_1_data_8F0.x + distance * W05SinDeg(angle);
            position.y = lbl_1_data_8F0.y;
            position.z = lbl_1_data_8F0.z + distance * W05CosDeg(angle);
            fn_1_19A90(&position,
                       1.0f + 0.3f * (-0.5f + 3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
        }
        for (i = 0; i < 3; i++) {
            angle = 160.0f;
            W05_EFFECT_RAND_NEXT();
            thirdAmbientSample = mbCapEffData[mbCapEffNum];
            thirdAmbientNoise = 3.051851e-05f * thirdAmbientSample;
            thirdAmbientOffset = thirdAmbientNoise - 0.5f;
            distance = 100.0f * (4.0f * thirdAmbientOffset);
            position.x = lbl_1_data_8FC.x + distance * W05SinDeg(angle);
            position.y = lbl_1_data_8FC.y;
            position.z = lbl_1_data_8FC.z + distance * W05CosDeg(angle);
            fn_1_19A90(&position,
                       1.0f + 0.3f * (-0.5f + 3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
        }
    }
    if ((board->hookSide != 0 || board->firstHookMoving == 0) && board->particlesEnabled != 0) {
        for (i = 0; i < 5; i++) {
            angle = 165.0f;
            W05_EFFECT_RAND_NEXT();
            fourthAmbientSample = mbCapEffData[mbCapEffNum];
            fourthAmbientNoise = 3.051851e-05f * fourthAmbientSample;
            fourthAmbientOffset = fourthAmbientNoise - 0.5f;
            distance = 100.0f * (4.5f * fourthAmbientOffset);
            position.x = lbl_1_data_908.x + distance * W05SinDeg(angle);
            position.y = lbl_1_data_908.y;
            position.z = lbl_1_data_908.z + distance * W05CosDeg(angle);
            fn_1_19A90(&position,
                       1.3f + 0.2f * (-0.5f + 3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
        }
        angle = 165.0f;
        distance = -225.0f;
        position.x = lbl_1_data_908.x + distance * W05SinDeg(angle);
        position.y = lbl_1_data_908.y;
        position.z = lbl_1_data_908.z + distance * W05CosDeg(angle);
        fn_1_19A90(&position,
                   1.3f + 0.2f * (-0.5f + 3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
    }
    if (board->particlesEnabled != 0) {
        fn_1_1E3C0(12);
    }
    if (GwSystem.curTime) {
        if (board->lampEnabled != 0) {
            if (board->lampAlpha < 1.0f) {
                if ((board->lampAlpha += 0.1f) > 1.0f) {
                    board->lampAlpha = 1.0f;
                }
                for (i = 0; i < 2; i++) {
                    mbObjAlphaSet(board->lampModels[i], 255.0f * board->lampAlpha);
                    mbObjDispSet(board->lampModels[i], TRUE);
                }
            }
        } else if (board->lampAlpha > 0.0f) {
            if ((board->lampAlpha -= 0.1f) < 0.0f) {
                board->lampAlpha = 0.0f;
                for (i = 0; i < 2; i++) {
                    mbObjDispSet(board->lampModels[i], FALSE);
                }
            }
            for (i = 0; i < 2; i++) {
                mbObjAlphaSet(board->lampModels[i], 255.0f * board->lampAlpha);
            }
        }
    }
    if (board->secondHookMoving != 0) {
        if (--board->splashTimer <= 0) {
            fn_1_76A4(5, &position, NULL, NULL);
            position.y += 100.0f;
            fn_1_1F9B0(position, 12);
            board->splashTimer =
                18.0f * (1.0f + 0.1f * (3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
        }
    }
    if (board->firstHookMoving != 0) {
        fn_1_1A13C();
        fn_1_1C0AC();
    }
    distance = board->hookMovementScale *= 0.99f;
    mbObjPosSet(
        board->firstHookModel, 0.0f,
        5.0f * (firstHookHeightPhase = board->firstHookSway[1], W05SinDeg(firstHookHeightPhase)),
        0.0f);
    mbObjRotSet(board->firstHookModel,
                (firstHookPitchPhase = board->firstHookSway[0], W05SinDeg(firstHookPitchPhase)),
                0.0f,
                (firstHookRollPhase = board->firstHookSway[2], W05SinDeg(firstHookRollPhase)) +
                    10.0f * W05SinDeg(360.0f * distance));
    W05_EFFECT_RAND_NEXT();
    firstHookPitchSample = mbCapEffData[mbCapEffNum];
    firstHookPitchNoise = 3.051851e-05f * firstHookPitchSample;
    if ((board->firstHookSway[0] += 3.0f * firstHookPitchNoise) >= 360.0f) {
        board->firstHookSway[0] -= 360.0f;
    }
    W05_EFFECT_RAND_NEXT();
    firstHookHeightSample = mbCapEffData[mbCapEffNum];
    firstHookHeightNoise = 3.051851e-05f * firstHookHeightSample;
    if ((board->firstHookSway[1] += 3.0f * firstHookHeightNoise) >= 360.0f) {
        board->firstHookSway[1] -= 360.0f;
    }
    W05_EFFECT_RAND_NEXT();
    firstHookRollSample = mbCapEffData[mbCapEffNum];
    firstHookRollNoise = 3.051851e-05f * firstHookRollSample;
    if ((board->firstHookSway[2] += 3.0f * firstHookRollNoise) >= 360.0f) {
        board->firstHookSway[2] -= 360.0f;
    }
    distance = board->secondHookSwing *= 0.99f;
    factorX = board->movementFactorX *= 0.98f;
    factorZ = board->movementFactorZ *= 0.98f;
    mbObjPosSet(
        board->secondHookModel, 0.0f,
        5.0f * (secondHookHeightPhase = board->secondHookSway[1], W05SinDeg(secondHookHeightPhase)),
        0.0f);
    mbObjRotSet(
        board->secondHookModel,
        (secondHookPitchPhase = board->secondHookSway[0], W05SinDeg(secondHookPitchPhase)) +
            5.0 * sin(M_PI * (360.0f * factorX) / 180.0),
        0.0f,
        5.0f * W05SinDeg(360.0f * factorZ) +
            ((secondHookRollPhase = board->secondHookSway[2], W05SinDeg(secondHookRollPhase)) +
             10.0f * W05SinDeg(360.0f * distance)));
    W05_EFFECT_RAND_NEXT();
    secondHookPitchSample = mbCapEffData[mbCapEffNum];
    secondHookPitchNoise = 3.051851e-05f * secondHookPitchSample;
    if ((board->secondHookSway[0] += 3.0f * secondHookPitchNoise) >= 360.0f) {
        board->secondHookSway[0] -= 360.0f;
    }
    W05_EFFECT_RAND_NEXT();
    secondHookHeightSample = mbCapEffData[mbCapEffNum];
    secondHookHeightNoise = 3.051851e-05f * secondHookHeightSample;
    if ((board->secondHookSway[1] += 3.0f * secondHookHeightNoise) >= 360.0f) {
        board->secondHookSway[1] -= 360.0f;
    }
    W05_EFFECT_RAND_NEXT();
    secondHookRollSample = mbCapEffData[mbCapEffNum];
    secondHookRollNoise = 3.051851e-05f * secondHookRollSample;
    if ((board->secondHookSway[2] += 3.0f * secondHookRollNoise) >= 360.0f) {
        board->secondHookSway[2] -= 360.0f;
    }
    if (board->firstHookPlayer >= 0) {
        fn_1_76A4(board->firstHookPlayerPoint, &position, NULL, NULL);
        mbPlayerPosSetV(board->firstHookPlayer, &position);
    }
    if (board->firstHookObject >= 0) {
        fn_1_76A4(board->firstHookObjectPoint, &position, NULL, NULL);
        mbObjPosSetV(board->firstHookObject, &position);
    }
    if (board->secondHookPlayer >= 0) {
        fn_1_7AF8(board->secondHookPlayerPoint, &position, NULL, NULL);
        mbPlayerPosSetV(board->secondHookPlayer, &position);
    }
    if (board->secondHookObject >= 0) {
        fn_1_7AF8(board->secondHookObjectPoint, &position, NULL, NULL);
        mbObjPosSetV(board->secondHookObject, &position);
    }
}

/* Creates the two particle models used for the paired hook effect during board setup. */
void fn_1_19460(void) {
    W05_TWIN_EFFECT_WORK *work;
    OMOBJ *obj;
    MBPARTICLE *particle;
    MBPARTICLEDATA *particleData;
    s32 particleIndex;
    s32 effectIndex;
    HU3D_MODEL *model;
    ANIMDATA *animation;
    s32 modelId;

    obj = lbl_1_bss_C = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_1964C);
    work = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(W05_TWIN_EFFECT_WORK),
        HU_MEMNUM_OVL);
    memset(work, 0, sizeof(W05_TWIN_EFFECT_WORK));
    work->animation = animation = HuSprAnimRead(HuDataReadNum(
        W05EffectDataNumGet(DATANUM(DATA_w05, 43)),
        HU_MEMNUM_OVL));
    for (effectIndex = 0; effectIndex < 2; effectIndex++) {
        work->modelId[effectIndex] = modelId = mbParticleCreate(animation, 512);
        model = &Hu3DData[work->modelId[effectIndex]];
        particle = model->hookData;
        particleData = particle->data;
        for (particleIndex = 0; particleIndex < particle->num;
             particleIndex++, particleData++) {
            particleData->dispF = 0;
        }
        if (particleIndex == 0) {
            Hu3DModelLayerSet(work->modelId[effectIndex], 5);
            particle->blendMode = MB_PARTICLE_BLEND_NORMAL;
        } else {
            Hu3DModelLayerSet(work->modelId[effectIndex], 6);
            particle->blendMode = MB_PARTICLE_BLEND_ADDCOL;
        }
        mbParticleAttrSet(work->modelId[effectIndex], MB_PARTICLE_ATTR_STOPCNT);
        work->activeCount[effectIndex] = 0;
    }
}

/* Advances the paired hook particles each object-manager update and destroys them on exit. */
void fn_1_1964C(OMOBJ *obj) {
    MBPARTICLEDATA *particleData;
    W05_TWIN_EFFECT_WORK *work;
    MBPARTICLE *particle;
    s32 effectIndex;
    s32 particleIndex;
    HU3D_MODEL *model;
    f32 alpha;

    work = obj->data;
    if (mbExitCheck() || lbl_1_bss_C == NULL) {
        for (effectIndex = 0; effectIndex < 2; effectIndex++) {
            if (work->modelId[effectIndex] != -1) {
                mbParticleKill(work->modelId[effectIndex]);
            }
            work->modelId[effectIndex] = -1;
        }
        omDelObjEx(mbObjMan, obj);
        lbl_1_bss_C = NULL;
        return;
    }
    for (effectIndex = 0; effectIndex < 2; effectIndex++) {
        model = &Hu3DData[work->modelId[effectIndex]];
        particle = model->hookData;
        particleData = particle->data;
        for (particleIndex = 0; particleIndex < particle->num;
             particleIndex++, particleData++) {
            if (particleData->dispF) {
                PSVECAdd(&particleData->pos, &particleData->vel, &particleData->pos);
                particleData->vel.y -= particleData->accel.y;
                alpha = particleData->speedDecay - particleData->colorIdx;
                particleData->speedDecay = alpha;
                particleData->color.a = particleData->accel.z * alpha;
                if (alpha <= 0.0f) {
                    particleData->dispF = 0;
                    work->activeCount[effectIndex]--;
                }
            }
        }
    }
}

void fn_1_197F8(void) {
    lbl_1_bss_C = 0;
}

/* Returns the number of particles left in the two hook effect models. */
s32 fn_1_1980C(void) {
    void *state = lbl_1_bss_C;
    s32 *values;

    if (lbl_1_bss_C == NULL) {
        return 0;
    }

    values = (s32 *) ((void **) state)[23];
    return values[2] + values[3];
}

/* Starts a particle in the selected hook effect model; the board update advances it. */
void fn_1_19864(HuVecF *position, HuVecF *velocity, f32 scale,
               f32 rotation, f32 gravity, GXColor *color, s32 duration, s32 additive) {
    W05_TWIN_PARTICLE_WORK *work;
    OMOBJ *object;
    HU3D_MODEL *model;
    MBPARTICLE *particle;
    MBPARTICLEDATA *particleData;
    s32 particleIndex;
    s32 effectIndex;

    object = lbl_1_bss_C;
    work = object->data;
    if (lbl_1_bss_C != NULL) {
        if (additive == 0) {
            model = &Hu3DData[work->modelId[0]];
        } else {
            model = &Hu3DData[work->modelId[1]];
        }
        particle = model->hookData;
        particleData = particle->data;
        for (particleIndex = 0; particleIndex < particle->num; particleIndex++, particleData++) {
            if (!particleData->dispF) {
                break;
            }
        }
        if (particleIndex < particle->num) {
            particleData->time = particleData->activeF = duration;
            particleData->pos = *position;
            particleData->scale = scale;
            particleData->color = *color;
            particleData->weight = rotation;
            particleData->animBank = 0;
            particleData->animNo = 0;
            particleData->animSpeed = 0.0f;
            particleData->animTime = 0.0f;
            particleData->dispF = 1;
            if (velocity == NULL) {
                particleData->vel.x = particleData->vel.y = particleData->vel.z = 0.0f;
            } else {
                particleData->vel = *velocity;
            }
            particleData->accel.y = gravity;
            particleData->accel.z = color->a;
            particleData->speedDecay = 1.0f;
            particleData->colorIdx = 1.0f / (f32)duration;
            if (additive == 0) {
                effectIndex = 0;
            } else {
                effectIndex = 1;
            }
            work->activeCount[effectIndex]++;
        }
    }
}

/* Emits a rising blue-gray particle; the board update calls this for sea spray and hook wakes. */
void fn_1_19A90(HuVecF *position, f32 strength) {
    HuVecF velocity;
    GXColor color;
    GXColor particleColor;
    f32 tint;
    f32 scale;
    f32 heightRandom;
    f32 colorRandom;
    f32 alphaRandom;
    f32 sizeRandom;
    f32 rotationRandom;
    f32 timeRandom;
    s16 heightSample;
    s16 colorSample;
    s16 alphaSample;
    s16 sizeSample;
    s16 rotationSample;
    s16 timeSample;
    s32 additive;

    velocity.x = 0.0f;
    W05_EFFECT_RAND_NEXT();
    heightSample = mbCapEffData[mbCapEffNum];
    heightRandom = 3.051851e-05f * heightSample;
    velocity.y = 5.0f * (0.8f + 0.3f * heightRandom);
    velocity.z = 0.0f;
    W05_EFFECT_RAND_NEXT();
    colorSample = mbCapEffData[mbCapEffNum];
    colorRandom = 3.051851e-05f * colorSample;
    tint = colorRandom;
    color.r = 150.0f + 63.0f * tint;
    color.g = 150.0f + 63.0f * tint;
    color.b = 190.0f + 63.0f * tint;
    W05_EFFECT_RAND_NEXT();
    alphaSample = mbCapEffData[mbCapEffNum];
    alphaRandom = 3.051851e-05f * alphaSample;
    color.a = (96.0f + 63.0f * alphaRandom) - 64.0f * strength;
    scale = strength * (1.0f + 0.25f *
        (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
    if (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE) < 0.7f) {
        additive = 0;
    } else {
        additive = 1;
    }
    W05_EFFECT_RAND_NEXT();
    sizeSample = mbCapEffData[mbCapEffNum];
    sizeRandom = 3.051851e-05f * sizeSample;
    W05_EFFECT_RAND_NEXT();
    rotationSample = mbCapEffData[mbCapEffNum];
    rotationRandom = 3.051851e-05f * rotationSample;
    particleColor = color;
    W05_EFFECT_RAND_NEXT();
    timeSample = mbCapEffData[mbCapEffNum];
    timeRandom = 3.051851e-05f * timeSample;
    fn_1_19864(position, &velocity,
        scale * (100.0f * (0.75f + 0.5f * sizeRandom)),
        360.0f * rotationRandom, 0.8166667f, &particleColor,
        60.0f * (0.25f + 0.05f * timeRandom), additive);
}

/* Emits the first hook's rising wake while the board update is moving the hooks. */
void fn_1_1A13C(void) {
    Mtx hookMatrix;
    HuVecF hookPosition;
    HuVecF hookRotation;
    HuVecF position;
    W05_BOARD_WORK *board;
    f32 radius;
    s32 round;

    board = lbl_1_bss_8;
    if (board->firstHookPhaseWeight <= 0.0f) {
        return;
    }
    fn_1_76A4(2, &hookPosition, &hookRotation, hookMatrix);
    for (round = 0; round < 4; round++) {
        radius = 4.0f * (100.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
        position.x = 100.0 + radius * sin(-M_PI / 4);
        position.y =
            50.0f + 0.5f * (100.0f * (-0.5f + 3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
        position.z = radius * cos(-M_PI / 4);
        PSMTXMultVec(hookMatrix, &position, &position);
        W05HookRiseParticle(&position, 1.5f * board->firstHookPhaseWeight);

        radius = 4.0f * (100.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
        position.x = 100.0 + radius * sin(-3 * M_PI / 4);
        position.y =
            50.0f + 0.5f * (100.0f * (-0.5f + 3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
        position.z = radius * cos(-3 * M_PI / 4);
        PSMTXMultVec(hookMatrix, &position, &position);
        W05HookRiseParticle(&position, 1.5f * board->firstHookPhaseWeight);

        position.x =
            -800.0f + 3.0f * (100.0f * (3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
        position.y =
            50.0f + 0.5f * (100.0f * (-0.5f + 3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
        position.z = 300.0f + 0.5f * (100.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
        PSMTXMultVec(hookMatrix, &position, &position);
        W05HookRiseParticle(&position, 1.5f * board->firstHookPhaseWeight);

        position.x =
            -800.0f + 3.0f * (100.0f * (3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
        position.y =
            50.0f + 0.5f * (100.0f * (-0.5f + 3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
        position.z =
            -300.0f + 0.5f * (100.0f * (3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
        PSMTXMultVec(hookMatrix, &position, &position);
        W05HookRiseParticle(&position, 1.5f * board->firstHookPhaseWeight);
    }
}

/* Emits the second hook's rising wake while the board update is moving the hooks. */
void fn_1_1C0AC(void) {
    Mtx hookMatrix;
    HuVecF hookPosition;
    HuVecF hookRotation;
    HuVecF position;
    W05_BOARD_WORK *board;
    f32 radius;
    s32 round;

    board = lbl_1_bss_8;
    if (board->secondHookPhaseWeight <= 0.0f) {
        return;
    }
    fn_1_7AF8(2, &hookPosition, &hookRotation, hookMatrix);
    for (round = 0; round < 4; round++) {
        radius = 4.0f * (100.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
        position.x = 100.0 + radius * sin(-M_PI / 4);
        position.y =
            50.0f + 0.5f * (100.0f * (-0.5f + 3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
        position.z = radius * cos(-M_PI / 4);
        PSMTXMultVec(hookMatrix, &position, &position);
        W05HookRiseParticle(&position, 1.5f * board->secondHookPhaseWeight);

        radius = 4.0f * (100.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
        position.x = 100.0 + radius * sin(-3 * M_PI / 4);
        position.y =
            50.0f + 0.5f * (100.0f * (-0.5f + 3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
        position.z = radius * cos(-3 * M_PI / 4);
        PSMTXMultVec(hookMatrix, &position, &position);
        W05HookRiseParticle(&position, 1.5f * board->secondHookPhaseWeight);

        position.x = -900.0f + 100.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        position.y =
            50.0f + 0.5f * (100.0f * (-0.5f + 3.7252903e-09f * (f32) mbRandMod(W05_RANDOM_RANGE)));
        position.z = 100.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        PSMTXMultVec(hookMatrix, &position, &position);
        W05HookRiseParticle(&position, 1.5f * board->secondHookPhaseWeight);
    }
}

/* Creates the yellow particle model and registers its per-frame board update. */
void fn_1_1D82C(void) {
    W05_EFFECT_WORK *work;
    MBPARTICLE *particle;
    MBPARTICLEDATA *particleData;
    s32 particleIndex;
    OMOBJ *particleObject;
    HU3D_MODEL *model;
    ANIMDATA *animation;
    s32 modelId;
    W05_EFFECT_WORK *allocatedWork;

    particleObject = (OMOBJ *)(lbl_1_bss_10 = omAddObjEx(mbObjMan, -32768, 0, 0,
                                                -1, fn_1_1D95C));
    allocatedWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(W05_EFFECT_WORK), HU_MEMNUM_OVL);
    particleObject->data = allocatedWork;
    work = allocatedWork;
    memset(work, 0, sizeof(W05_EFFECT_WORK));
    animation = HuSprAnimRead(HuDataReadNum(DATANUM(DATA_capsule, 41), HU_MEMNUM_OVL));
    work->animation = animation;
    work->modelId = modelId = mbParticleCreate(animation, 512);
    Hu3DModelLayerSet(work->modelId, 5);
    work->count = 0;
    model = &Hu3DData[work->modelId];
    particle = model->hookData;
    particleData = particle->data;
    for (particleIndex = 0; particleIndex < particle->num; particleIndex++, particleData++) {
        particleData->dispF = 0;
    }
    mbParticleAttrSet(work->modelId, MB_PARTICLE_ATTR_STOPCNT);
}

/* Advances yellow particles and releases the effect after board exit; registered by fn_1_1D82C. */
void fn_1_1D95C(OMOBJ *object) {
    W05_PARTICLE_WORK *work;
    HU3D_MODEL *model;
    MBPARTICLE *particle;
    MBPARTICLEDATA *particleData;
    s32 particleIndex;
    f32 phase;
    f32 scaleFactor;
    f32 cosine;
    f32 sine;

    work = object->data;
    if (mbExitCheck() || lbl_1_bss_10 == NULL) {
        mbParticleKill(work->modelId);
        omDelObjEx(mbObjMan, object);
        lbl_1_bss_10 = NULL;
        return;
    }
    if (work->activeCount <= 0) {
        Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF);
        return;
    }
    Hu3DModelAttrReset(work->modelId, HU3D_ATTR_DISPOFF);
    model = &Hu3DData[work->modelId];
    particle = model->hookData;
    particleData = particle->data;
    for (particleIndex = 0; particleIndex < particle->num; particleIndex++, particleData++) {
        if (!particleData->dispF) {
            continue;
        }
        PSVECAdd(&particleData->pos, &particleData->vel, &particleData->pos);
        if (particleData->colorIdx) {
            particleData->vel.y -= particleData->colorIdx;
        }
        if (particleData->time == 0) {
            particleData->scale = particleData->accel.x * particleData->accel.y;
            particleData->accel.y -= particleData->accel.z;
        } else {
            if (particleData->accel.y > 0.8f) {
                phase = 5.0f * (particleData->accel.y - 0.8f);
                cosine = mbCosDeg(90.0f * phase);
                scaleFactor = cosine;
            } else {
                phase = 1.25f * particleData->accel.y;
                sine = mbSinDeg(90.0f * phase);
                scaleFactor = sine;
            }
            particleData->scale = scaleFactor * particleData->accel.x;
            particleData->accel.y -= particleData->accel.z;
        }
        particleData->weight += particleData->scaleBase;
        if (particleData->accel.y <= 0.0f) {
            particleData->time = 0;
            particleData->scale = 0.0f;
            particleData->dispF = 0;
            work->activeCount--;
        }
    }
}

/* Clears the global handle before the yellow particle task is prepared. */
void fn_1_1DC08(void) {
    lbl_1_bss_10 = 0;
}

/* Returns the number of active yellow particles, or zero when its task is absent. */
s32 fn_1_1DC1C(void) {
    OMOBJ *obj;
    W05_EFFECT_WORK *work;

    obj = (OMOBJ *)lbl_1_bss_10;
    if ((OMOBJ *)lbl_1_bss_10 == NULL) {
        return 0;
    } else {
        work = obj->data;
        return work->count;
    }
}

/* Changes the blend mode used by the yellow particle model. */
void fn_1_1DC6C(s32 blendMode) {
    OMOBJ *obj;

    obj = (OMOBJ *)lbl_1_bss_10;
    if ((void *)lbl_1_bss_10 != NULL) {
        W05_EFFECT_WORK *work;
        MBPARTICLE *particle;
        HU3D_MODEL *model;

        work = obj->data;
        model = &Hu3DData[work->modelId];
        particle = model->hookData;
        particle->blendMode = blendMode;
    }
}

/* Starts a yellow particle with the supplied motion, size, lifetime, and color. */
s32 fn_1_1DCE4(HuVecF position, HuVecF velocity, f32 scale,
              f32 duration, f32 rotationSpeed, f32 gravity, GXColor color) {
    W05_PARTICLE_WORK *work;
    OMOBJ *object;
    HU3D_MODEL *model;
    MBPARTICLE *particle;
    MBPARTICLEDATA *particleData;
    s32 particleIndex;

    if (lbl_1_bss_10 == NULL) {
        return -1;
    }
    object = lbl_1_bss_10;
    work = object->data;
    model = &Hu3DData[work->modelId];
    particle = model->hookData;
    particleData = particle->data;
    for (particleIndex = 0; particleIndex < particle->num; particleIndex++, particleData++) {
        if (particleData->scale <= 0.0f) {
            break;
        }
    }
    if (particleIndex >= particle->num) {
        return -1;
    }
    particleData->dispF = 1;
    particleData->time = particleData->activeF = 0;
    particleData->pos.x = position.x;
    particleData->pos.y = position.y;
    particleData->pos.z = position.z;
    particleData->vel.x = velocity.x;
    particleData->vel.y = velocity.y;
    particleData->vel.z = velocity.z;
    particleData->accel.x = scale;
    particleData->accel.y = 1.0f;
    if (duration) {
        particleData->accel.z = 1.0f / duration;
    } else {
        particleData->accel.z = 1.0f;
    }
    particleData->colorIdx = gravity;
    particleData->scaleBase = rotationSpeed;
    particleData->scale = scale;
    particleData->color = color;
    particleData->weight = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
    particleData->time = 0;
    work->activeCount++;
    return particleIndex;
}

/* Emits sixteen yellow particles around a point, rotated with the supplied event pose. */
void fn_1_1DF14(HuVecF *position, HuVecF *rotation) {
    Mtx rotationMatrix;
    HuVecF velocity;
    GXColor color;
    f32 azimuth;
    f32 elevation;
    f32 speed;
    s32 emitted;

    if (rotation != NULL) {
        mtxRot(rotationMatrix, rotation->x, rotation->y, rotation->z);
    } else {
        PSMTXIdentity(rotationMatrix);
    }
    for (emitted = 0; emitted < 16; emitted++) {
        azimuth = 45.0f * emitted +
            10.0f * (-0.5f + 3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        if (emitted & 1) {
            elevation = 10.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        } else {
            elevation = -10.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        }
        speed = 5.0f * (0.5f + 3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        velocity.x = speed *
            (cos(M_PI * elevation / 180.0) * sin(M_PI * azimuth / 180.0));
        velocity.y = speed * sin(M_PI * elevation / 180.0);
        velocity.z = speed *
            (cos(M_PI * elevation / 180.0) * cos(M_PI * azimuth / 180.0));
        PSMTXMultVec(rotationMatrix, &velocity, &velocity);
        color.r = 255;
        color.g = 255;
        color.b = 0;
        color.a = 255;
        fn_1_1DCE4(*position, velocity,
            100.0f * (0.5f + 0.3f *
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            60.0f * (0.5f + 0.25f *
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            0.0f, 0.16333334f, color);
    }
}

/* Emits stationary white particles throughout the board effect bounds. */
void fn_1_1E3C0(s32 count) {
    HuVecF position;
    HuVecF velocity;
    GXColor color;
    W05_BOARD_WORK *board;
    s32 particleIndex;
    s32 emitted;

    board = lbl_1_bss_8;
    for (emitted = 0; emitted < count; emitted++) {
        position.x = 300.0f + board->particleBoundsStart.x +
            (board->particleBoundsSize.x - 600.0f) *
            (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        position.y = 20.0f + (board->particleBoundsStart.y +
            board->particleBoundsSize.y *
            (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
        position.z = board->particleBoundsStart.z + board->particleBoundsSize.z *
            (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        velocity.x = velocity.y = velocity.z = 0.0f;
        color.r = color.g = color.b = color.a = 255;
        particleIndex = fn_1_1DCE4(position, velocity,
            100.0f * (0.2f + 0.05f *
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            60.0f * (0.2f + 0.1f *
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            0.0f, 0.0f, color);
        if (particleIndex >= 0) {
            fn_1_1EA60(particleIndex);
        }
    }
}

/* Emits a sixteen-particle yellow ring around an event model. */
void fn_1_1E6D0(HuVecF *position) {
    HuVecF velocity;
    GXColor color;
    f32 angle;
    f32 speed;
    s32 emitted;

    angle = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
    for (emitted = 0; emitted < 16; emitted++) {
        speed = 5.0f * (0.5f + (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
        angle += 22.5f * emitted +
            10.0f * (-0.5f + (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
        velocity.x = speed * sin(M_PI * angle / 180.0);
        velocity.y = speed * cos(M_PI * angle / 180.0);
        velocity.z = 0.0f;
        color.r = 255;
        color.g = 255;
        color.b = 0;
        color.a = 255;
        fn_1_1DCE4(*position, velocity,
            100.0f * (0.5f + 0.3f *
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            60.0f * (0.5f + 0.25f *
                (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            0.0f, 0.0f, color);
    }
}

/* Primes a newly emitted particle so the yellow effect update can advance it. */
void fn_1_1EA60(s32 particleIndex) {
    MBPARTICLEDATA *particleData;
    OMOBJ *particleObject;
    W05_EFFECT_OWNER *owner;
    MBPARTICLE *particle;
    HU3D_MODEL *model;

    if ((void *)lbl_1_bss_10 != NULL) {
        particleObject = (OMOBJ *)lbl_1_bss_10;
        owner = particleObject->data;
        model = &Hu3DData[owner->modelId];
        particle = model->hookData;
        particleData = &particle->data[particleIndex];
        particleData->time = 1;
        particleData->scale = 1e-05f;
    }
}

/* Creates the colored particle model and registers its per-frame fade update. */
void fn_1_1EAF4(void) {
    W05_COLOR_EFFECT_WORK *work;
    OMOBJ *particleObject;
    MBPARTICLE *particle;
    HU3D_MODEL *model;
    ANIMDATA *animation;
    s32 modelId;
    W05_COLOR_EFFECT_WORK *allocatedWork;

    particleObject = (OMOBJ *)(lbl_1_bss_14 = omAddObjEx(mbObjMan, -32768, 0, 0,
                                                -1, fn_1_1EC00));
    allocatedWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(W05_COLOR_EFFECT_WORK),
        HU_MEMNUM_OVL);
    particleObject->data = allocatedWork;
    work = allocatedWork;
    memset(work, 0, sizeof(W05_COLOR_EFFECT_WORK));
    animation = HuSprAnimRead(HuDataReadNum(DATANUM(DATA_capsule, 46), HU_MEMNUM_OVL));
    work->animation = animation;
    work->modelId = modelId = mbParticleCreate(animation, 512);
    Hu3DModelLayerSet(work->modelId, 5);
    work->activeCount = 0;
    model = &Hu3DData[work->modelId];
    particle = model->hookData;
    particle->blendMode = MB_PARTICLE_BLEND_NORMAL;
    mbParticleColorCreate(work->modelId);
}

/* Advances colored particles and removes the task after board exit; registered by fn_1_1EAF4. */
void fn_1_1EC00(OMOBJ *object) {
    W05_PARTICLE_WORK *work;
    HU3D_MODEL *model;
    MBPARTICLE *particle;
    MBPARTICLEDATA *particleData;
    s32 particleIndex;

    work = object->data;
    if (mbExitCheck() || lbl_1_bss_14 == NULL) {
        mbParticleKill(work->modelId);
        lbl_1_bss_14 = NULL;
        omDelObjEx(mbObjMan, object);
        return;
    }
    if (work->activeCount <= 0) {
        Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF);
        return;
    }
    Hu3DModelAttrReset(work->modelId, HU3D_ATTR_DISPOFF);
    model = &Hu3DData[work->modelId];
    particle = model->hookData;
    particleData = particle->data;
    for (particleIndex = 0; particleIndex < particle->num; particleIndex++, particleData++) {
        if (particleData->scale <= 0.0f) {
            continue;
        }
        particleData->pos.x += particleData->vel.x;
        particleData->pos.y += particleData->vel.y;
        particleData->pos.z += particleData->vel.z;
        particleData->weight += particleData->accel.z;
        if (particleData->weight >= 360.0f) {
            particleData->weight -= 360.0f;
        }
        particleData->speedDecay += particleData->colorIdx;
        particleData->color.a = particleData->scaleBase *
            (1.0f - particleData->speedDecay / 16.0f);
        if (particleData->speedDecay >= 16.0f) {
            particleData->dispF = 0;
            particleData->pauseF = 0;
            particleData->time = 0;
            particleData->scale = 0.0f;
            work->activeCount--;
        }
    }
}

/* Clears the global handle before the colored particle task is prepared. */
void fn_1_1EE24(void) {
    lbl_1_bss_14 = 0;
}

/* Returns the colored effect's active particle count, or zero when its task is absent. */
int fn_1_1EE38(void) {
    u32 *effectCounts;

    if (lbl_1_bss_14 == NULL) {
        return 0;
    }
    effectCounts = ((OMOBJ *)lbl_1_bss_14)->data;
    return (int)effectCounts[1];
}

/* Starts a drifting colored particle; its alpha is retained for the fade update. */
s32 fn_1_1EE80(HuVecF position, HuVecF velocity, f32 scale,
              f32 rotationSpeed, f32 ageStep, GXColor color) {
    W05_PARTICLE_WORK *work;
    OMOBJ *object;
    HU3D_MODEL *model;
    MBPARTICLE *particle;
    MBPARTICLEDATA *particleData;
    s32 particleIndex;

    object = lbl_1_bss_14;
    work = object->data;
    model = &Hu3DData[work->modelId];
    particle = model->hookData;
    particleData = particle->data;
    for (particleIndex = 0; particleIndex < particle->num; particleIndex++, particleData++) {
        if (particleData->scale <= 0.0f) {
            break;
        }
    }
    if (particleIndex >= particle->num) {
        return -1;
    }
    particleData->time = particleData->activeF = 0;
    particleData->dispF = 1;
    particleData->pauseF = 0;
    particleData->pos.x = position.x;
    particleData->pos.y = position.y;
    particleData->pos.z = position.z;
    particleData->vel.x = velocity.x;
    particleData->vel.y = velocity.y;
    particleData->vel.z = velocity.z;
    particleData->accel.z = rotationSpeed;
    particleData->scale = scale;
    particleData->color = color;
    particleData->color.r = particleData->color.g = particleData->color.b = 255;
    particleData->weight = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
    particleData->time = 0;
    particleData->animTime = 0.0f;
    particleData->animSpeed = 0.0f;
    particleData->speedDecay = 0.0f;
    particleData->colorIdx = ageStep;
    particleData->scaleBase = particleData->color.a;
    work->activeCount++;
    return particleIndex;
}

/* Emits stationary fading particles across a rounded volume around the event point. */
void fn_1_1F090(HuVecF *position, f32 radius, f32 height, s32 count) {
    HuVecF emissionPosition;
    HuVecF velocity;
    GXColor color;
    s32 emitted;
    f32 angle;
    f32 distance;
    f32 radialFraction;
    f32 shade;

    for (emitted = 0; emitted < count; emitted++) {
        angle = 360.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        radialFraction = 3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE);
        distance = radialFraction * radius;
        emissionPosition.x = position->x + distance * sin(M_PI * angle / 180.0);
        emissionPosition.y = 50.0 + (position->y +
            (height * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))) *
            cos(M_PI * (90.0f * radialFraction) / 180.0));
        emissionPosition.z = position->z + distance * cos(M_PI * angle / 180.0);
        velocity.x = velocity.y = velocity.z = 0.0f;
        shade = 63.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        color.r = 192.0f + shade;
        color.g = 192.0f + shade;
        color.b = 192.0f + shade;
        color.a = 192.0f + 63.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        fn_1_1EE80(emissionPosition, velocity,
            100.0f * (3.0f + 3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)),
            10.0f * (-0.5f + 3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)),
            0.33f + 0.2f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)), color);
    }
}

/* Emits a rotating ring of fading particles in the supplied event plane. */
void fn_1_1F57C(HuVecF position, HuVecF rotation, s32 count) {
    Mtx rotationMatrix;
    HuVecF emissionPosition;
    HuVecF velocity;
    GXColor color;
    s32 emitted;
    s32 shade;
    s32 angle;
    f32 sine;
    f32 cosine;

    mtxRot(rotationMatrix, rotation.x, rotation.y, rotation.z);
    for (emitted = 0; emitted < count; emitted++) {
        emissionPosition.x = position.x;
        emissionPosition.y = position.y;
        emissionPosition.z = position.z;
        angle = (360.0f / count) * emitted;
        sine = mbSinDeg(angle);
        velocity.x = 0.075f * (100.0f * sine) *
            (0.9f + 0.1f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
        velocity.y = 0.0f;
        cosine = mbCosDeg(angle);
        velocity.z = 0.075f * (100.0f * cosine) *
            (0.9f + 0.1f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
        PSMTXMultVec(rotationMatrix, &velocity, &velocity);
        shade = 63.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        color.r = (u8)(shade + 192);
        color.g = (u8)(shade + 192);
        color.b = (u8)(shade + 192);
        color.a = 192.0f + 63.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        fn_1_1EE80(emissionPosition, velocity, 200.0f,
            10.0f * (-0.5f + (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            0.33f, color);
    }
}

/* Emits an upward burst of fading particles around the supplied event point. */
void fn_1_1F9B0(HuVecF position, s32 count) {
    HuVecF emissionPosition;
    HuVecF velocity;
    GXColor color;
    s32 emitted;
    s32 shade;
    s32 angle;
    f32 sine;
    f32 cosine;

    for (emitted = 0; emitted < count; emitted++) {
        emissionPosition.x = position.x;
        emissionPosition.y = position.y;
        emissionPosition.z = position.z;
        angle = (360.0f / count) * emitted;
        sine = mbSinDeg(angle);
        velocity.x = 0.05f * (100.0f * sine) *
            (0.9f + 0.1f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
        velocity.y = 5.0f *
            (0.9f + 0.1f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
        cosine = mbCosDeg(angle);
        velocity.z = 0.05f * (100.0f * cosine) *
            (0.9f + 0.1f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE)));
        shade = 63.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        color.r = (u8)(shade + 192);
        color.g = (u8)(shade + 192);
        color.b = (u8)(shade + 192);
        color.a = 192.0f + 63.0f * (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE));
        fn_1_1EE80(emissionPosition, velocity, 100.0f,
            10.0f * (-0.5f + (3.7252903e-09f * (f32)mbRandMod(W05_RANDOM_RANGE))),
            0.33f, color);
    }
}
