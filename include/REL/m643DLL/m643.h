/* State records shared by Jump the Gun's gameplay and camera code. */
#ifndef REL_M643_H
#define REL_M643_H
#include "game/mg/actman.h"

/* Introduction timing and audio used by the minigame sequence callbacks. */
typedef struct M643SequenceWork_s {
    s32 phaseFrames; /* Frames elapsed in the introduction. */
    s16 musicHandle; /* Background stream handle; -1 until music starts. */
    s32 introSoundHandle; /* Opening sound handle; -1 when absent or faded out. */
} M643SequenceWork;

/* One horizontal aiming direction's edge detection and key-repeat timers. */
typedef struct M643InputPrefix_s {
    s32 held, pressed, released, step; /* Held state, input edges, and this frame's aim step. */
    u8 repeatFrames, pressCooldownFrames; /* Delays before repeat and accepting another press. */
    u8 unusedBytes[2]; /* Not accessed by the aiming callbacks. */
} M643InputPrefix;

/* A reusable cannon shot, its particles, and its curved or deflected flight. */
typedef struct M643ShotPrefix_s {
    struct M643TeamScenePrefix_s *cannon; /* Cannon whose muzzle and aim are copied at launch. */
    s16 slotIndex, teamIndex; /* Shot slot (0 or 1) and owning team (0 or 1). */
    HU3D_MODELID arrivalEffectModel; /* Animated effect shown when a new platform appears. */
    HU3D_MOTIONID arrivalEffectMotion; /* Motion restarted on platform arrival. */
    HU3D_MODELID projectileModel, trailParticles, impactParticles; /* Shot, smoke, and sparks. */
    u16 inUse; /* Set on a firing request and cleared when the shot disappears. */
    HuVecF launchDirection; /* Muzzle-to-target unit vector; zero for negligible distance. */
    f32 speed, launchDistance; /* Units per frame and muzzle-to-target distance. */
    HuVecF position; /* Current projectile position in world coordinates. */
    f32 elapsedSeconds, durationSeconds; /* Elapsed and total curved-flight time. */
    s32 targetPlatform; /* Platform index selected when the shot was requested. */
    HuVecF curveStart, curveEnd, startTangent, endTangent; /* Hermite flight endpoints and
                                                            * tangents. */
    HuVecF fallVelocity, spinVelocity, rotationRadians; /* Movement and spin per frame; orientation
                                                         * in radians. */
} M643ShotPrefix;

/* Work data used by the map scene's object and model list. */
typedef struct M643MapSceneWork_s {
    OMOBJ *mapObject; /* Environment models, including the hidden finish-area collision map. */
    HU3D_MODELID collisionModels[39]; /* Hidden platform models used by actor collision. */
} M643MapSceneWork;

/* Visible lowered platforms and the raised platforms revealed by one team's shots. */
typedef struct M643PlatformWork_s {
    s16 teamIndex; /* Team whose camera, collision mask, and revealed flags are used. */
    HU3D_MODELID raisedModels[39]; /* Revealed platform models, initially hidden. */
    HU3D_MODELID loweredModels[39]; /* Day or night platform models visible before a hit. */
} M643PlatformWork;

/* One team's cannon operator, models, aiming input, and reusable projectile slots. */
typedef struct M643TeamScenePrefix_s {
    s16 playerIndex, characterId, teamIndex; /* Configured player, character, and team (0 or 1). */
    HU3D_MODELID barrelModel, baseModel; /* Cannon barrel and base, rotated toward the aim point. */
    HU3D_MOTIONID cannonMotions[3]; /* Initial, reload, and firing motions. */
    HU3D_MODELID loadedShotModel; /* Projectile attached to the cannon, hidden during firing. */
    HU3D_MOTIONID operatorBaseMotion; /* Motion for the model placed beneath the operator. */
    HU3D_MODELID muzzleEffectModel; /* Effect positioned at the cannon muzzle when firing. */
    HU3D_MOTIONID muzzleEffectMotion; /* Motion restarted for each muzzle effect. */
    HU3D_MODELID reloadEffectModel; /* Effect positioned at the cannon base's effect hook. */
    HU3D_MOTIONID reloadEffectMotion; /* Motion restarted when reloading. */
    OMOBJ *operatorObject; /* Character and the animated model placed below it. */
    HuVecF cannonPosition, muzzlePosition; /* World positions of the cannon and its firing hook. */
    f32 firingMotionFrames; /* Length of the cannon's firing motion. */
    HU3D_MODELID aimMarkerModel; /* Marker placed at the selected platform. */
    HU3D_MOTIONID aimMarkerMotion; /* Looping marker animation. */
    HuVecF aimPosition; /* World target copied by a projectile at launch. */
    s16 aimPlatform; /* Selected platform index, clamped to the camera's range. */
    s16 previousAimPlatform; /* Previous selection used to detect an aiming sound. */
    f32 yawRadians; /* Smoothed horizontal cannon angle. */
    f32 pitchRadians; /* Smoothed vertical cannon angle. */
    s32 shotCount; /* Number of reusable projectile slots (two). */
    M643ShotPrefix **shots; /* Projectile work records owned by this cannon. */
    u8 unusedShotTail[4]; /* Not accessed by the cannon callbacks. */
    s32 phaseFrames; /* Intro, reload, result, or computer-aim frame counter. */
    M643InputPrefix aimInput[2]; /* Left and right stick direction state. */
    s32 cooldownFrames; /* Firing cooldown, clamped to zero. */
    s16 operatorMotion; /* Current character motion index; -1 before its first motion. */
    u8 unusedMotionTail[2]; /* Not accessed by the cannon callbacks. */
    f32 bobDegrees; /* Vertical bobbing phase, wrapped at 360 degrees. */
    HU3D_MODELID operatorFollowModel; /* Auxiliary model positioned with the operator. */
    u8 unusedModelTail[4]; /* Not accessed by the cannon callbacks. */
    s16 unusedSetupValue; /* Set to one at setup; no cannon callback reads it. */
    s16 computerTargetPlatform; /* Computer aim target; zero requests a new target after firing. */
    s16 observedRunnerPlatform; /* Runner platform recorded when the computer aim target was
                                 * chosen. */
    s16 reactionFrames; /* Remaining computer shot delay, counted after the cooldown reaches
                         * zero. */
} M643TeamScenePrefix;

/* Fields shared by the runner and shooter player state records. */
typedef struct M643PlayerIdentityPrefix_s {
    s16 playerIndex, characterId, teamIndex; /* Configured player, character, and team (0 or 1). */
} M643PlayerIdentityPrefix;

/* Runner control, last touched platform, fall recovery, and computer jumping state. */
typedef struct M643PlayerPrefix_s {
    s16 playerIndex; /* Configured player index, from 0 through 3. */
    s16 characterId; /* Character used for the runner's model and motions. */
    s16 teamIndex; /* Owning team, 0 or 1. */
    MGPLAYER *player; /* Runner's actor, input, and character animation state. */
    u16 respawnPending; /* One while despawned; cleared when the rescue releases the runner. */
    s16 lastPlatform; /* Last platform touched on the ground, from 0 through 38. */
    f32 rescueSpinRadians; /* Runner's Y rotation while being carried after a fall. */
    HuVecF position; /* World position cached for depth locking, shadows, and rescue. */
    s32 phaseFrames; /* Introduction, result, or computer-platform search frame counter. */
    struct M643CameraEffectWork_s *rescue; /* Object that retrieves the fallen runner. */
    HU3D_MODELID shadowModel; /* Separate shadow shown beyond the course's ends. */
    s32 fallSoundPlayed; /* Prevents the fall cue from playing again before despawning. */
    s32 finished; /* One after reaching the finish area. */
    s16 jumpTarget; /* Computer jump destination; zero means no active target. */
    s16 jumpDelayFrames; /* Difficulty-dependent wait before a computer jump. */
    s16 previousJumpTarget; /* Last target that established the computer jump delay. */
} M643PlayerPrefix;

/* Rescue model that approaches, lifts, returns, and releases one fallen runner. */
typedef struct M643CameraEffectWork_s {
    s16 teamIndex; /* Team camera used for the rescue's approach. */
    HU3D_MODELID model; /* Model whose item hook carries the runner. */
    HU3D_MOTIONID motion; /* Looping rescue animation. */
    s16 rescueRequested; /* One from the fall request until the rescue departs. */
    HuVecF position; /* Current rescue position in world coordinates. */
    M643PlayerPrefix *runner; /* Runner retrieved by this rescue. */
    s32 carryingRunner; /* One during lifting and return, zero during approach and idle. */
    HuVecF curveStart; /* Approach starts above the camera. */
    HuVecF curveEnd; /* Approach ends at the runner's X, fixed Y=-500 and Z=100, above its fall
                      * position. */
    HuVecF startTangent; /* Hermite approach tangent at its start. */
    HuVecF endTangent; /* Downward Hermite approach tangent at its end. */
    f32 elapsedSeconds; /* Elapsed approach time. */
    f32 durationSeconds; /* Total approach time, based on distance and movement speed. */
    u8 unusedCurveTail[8]; /* Not accessed by the rescue callbacks. */
    HuVecF rotationRadians; /* Travel orientation, eased toward zero near the runner. */
} M643CameraEffectWork;

/* Result-view timing shared by the callbacks that switch to the winning team's camera. */
typedef struct M643ViewSequenceWork_s {
    u8 unusedBytes[4]; /* Not accessed by the result-view callbacks. */
    s32 phaseFrames; /* Outgoing-wipe wait and winner-camera fade-in frame counter. */
} M643ViewSequenceWork;

/* Work data for the per-team view controller. */
typedef struct M643ViewControlWork_s {
    s16 teamIndex; /* Team whose runner and camera are followed. */
    f32 cameraDepth; /* Camera's world Z position, computed from view distance and field of view. */
    s32 phaseFrames; /* Intro tracking and split-screen transition frame counter. */
    s32 shakeFrames; /* Remaining viewport-shake frames after a platform is revealed. */
} M643ViewControlWork;

extern M643PlayerPrefix *lbl_1_bss_174[2];
extern M643PlayerIdentityPrefix *lbl_1_bss_15C[4];
extern M643ViewControlWork *lbl_1_bss_154[2];

extern M643TeamScenePrefix *lbl_1_bss_16C[2];
extern void *lbl_1_bss_14C[2];
M643PlayerPrefix *fn_1_1640(s16 teamIndex);
M643TeamScenePrefix *fn_1_165C(s16 teamIndex);
void *fn_1_1678(s16 teamIndex);
M643ViewControlWork *fn_1_1694(s16 teamIndex);
#endif
