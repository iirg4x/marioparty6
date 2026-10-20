/* Declares the minigame scene's actors, sequences and camera state. */
#ifndef REL_M619DLL_SCENE_H
#define REL_M619DLL_SCENE_H
#include "REL/m619Dll/player.h"

typedef struct M619PlayerPose {
    M619Player *player; /* Player whose pose is being animated. */
    HuVecF position; /* Position used by the pose animation. */
    HuVecF rotation; /* Rotation used by the pose animation, in degrees. */
    s32 mode; /* Pose-animation mode. */
    s32 frame; /* Frames elapsed in the current pose. */
    f32 angle; /* Angle used to place or rotate the pose. */
    HU3D_MODELID firstModel; /* First model attached to the pose. */
    HU3D_MODELID secondModel; /* Second model attached to the pose. */
} M619PlayerPose;
typedef struct M619RadialActor {
    M619Player *player; /* Player associated with this radial actor. */
    M619ModelQueueNode modelQueue; /* Models queued to follow the actor. */
    M619MotionNode collision; /* Collision node used while the actor moves. */
    M619ModelMotion modelMotion; /* Attached model and its particle effect. */
    HuVecF position; /* Actor position in world coordinates. */
    HuVecF rotation; /* Actor rotation in degrees. */
    s32 kind; /* Actor behavior selected by its callback. */
    s32 frame; /* Frames elapsed in the current actor phase. */
    s32 mode; /* Actor callback mode. */
    f32 scale; /* Actor model scale. */
    f32 angle; /* Angle used to place or rotate the actor. */
    HU3D_MODELID firstModel; /* First model used by the actor. */
    HU3D_MODELID secondModel; /* Second model used by the actor. */
    s16 active; /* Whether the actor update is active. */
} M619RadialActor;
typedef struct M619SceneAnimation {
    M619PlayerPose poses[4]; /* Per-player poses used during the scene animation. */
    M619RadialActor actors[4]; /* Radial actors updated by the scene animation. */
    M619ModelMotion modelMotion; /* Model motion attached to the animation. */
    M619ParticleEffect effects[3]; /* Repeating ring effects around the scene. */
    HU3D_MODELID model; /* Model used by the scene animation. */
    s32 frame; /* Frames elapsed in the animation. */
    s32 phase; /* Current animation phase. */
    s32 callbackValueA; /* First value passed between animation callbacks. */
    s32 callbackValueB; /* Second value passed between animation callbacks. */
    s32 effectFrame; /* Frame counter for the animation's effect. */
    f32 progress; /* Update counter used to trigger callbacks at 600 and 1200 updates. */
    f32 height; /* Height used by the animated scene model. */
    f32 scale; /* Scale applied to the animated scene model. */
} M619SceneAnimation;
typedef struct M619SelectionRecord {
    HuVecF position; /* Position of the selectable model group. */
    HuVecF rotation; /* Rotation of the selectable model group, in degrees. */
    HuVecF impulse; /* Motion impulse applied to the selection record. */
    M619ModelQueueNode modelQueue; /* Models queued to follow this record. */
    M619MotionNode collision; /* Collision node used during selection movement. */
    f32 amplitude; /* Current oscillation amplitude. */
    f32 angle; /* Current oscillation angle, in radians. */
    f32 angleStep; /* Angle increment per update, in radians. */
    f32 speed; /* Movement speed used by the selection update. */
    f32 offset; /* Positional offset used by the selection update. */
    f32 correctionX; /* X correction applied to the movement. */
    f32 correctionZ; /* Z correction applied to the movement. */
    f32 damping; /* Movement damping factor. */
    s32 initialized; /* Whether this selection record has been initialized. */
    s32 mode; /* Movement mode used by this record. */
    s32 countdown; /* Remaining frames in the current selection step. */
    s32 motionState; /* State of the associated model motion. */
    s32 frame; /* Frames elapsed in the current movement step. */
    HU3D_MODELID models[3]; /* Models displayed for this selection record. */
} M619SelectionRecord;
typedef struct M619Selection {
    s32 state; /* Overall selection sequence state. */
    s32 status; /* Selection status flags. */
    s32 playerOrder[4]; /* Player indices in selection order. */
    s32 groupOrder[3]; /* Group indices in selection order. */
    s32 selectionState; /* Current player-selection phase. */
    s32 groupState; /* Current group-selection phase. */
    M619SelectionRecord records[3][8]; /* Selection records for each group. */
} M619Selection;
typedef struct M619ModelTransition {
    M619ParticleEffect flash; /* Particle burst or ring emitted by the transition. */
    HuVecF position; /* World position followed by the transition effect. */
    s32 phase; /* Current transition phase. */
    s32 frame; /* Frames elapsed in the current phase. */
    s32 audioPan; /* Pan value used for the transition sound. */
    HU3D_MODELID modelA; /* First model used by the transition. */
    HU3D_MODELID modelB; /* Second model used by the transition. */
} M619ModelTransition;
typedef struct M619Launcher {
    HuVecF position; /* Launcher position in world coordinates. */
    HuVecF rotation; /* Launcher rotation in degrees. */
    M619ModelTransition transitions[2]; /* The two model-transition effects. */
    f32 rotationOffsetA; /* First rotation offset, in degrees. */
    f32 rotationOffsetB; /* Second rotation offset, in degrees. */
    f32 motionRadius; /* Radius of the launcher's circular movement. */
    f32 motionPhase; /* Current phase of the circular movement, in degrees. */
    f32 positionYBase; /* Base height for the launcher position. */
    f32 positionYOffset; /* Height offset applied during movement. */
    f32 positionZBase; /* Base Z coordinate for the launcher position. */
    f32 positionZOffset; /* Z offset applied during movement. */
    HU3D_MODELID positionModel; /* Model moved along the launcher path. */
    HU3D_MODELID rotationModel; /* Model rotated during the launch sequence. */
} M619Launcher;
typedef struct M619CameraOrbit {
    HuVecF position; /* Camera position in world coordinates. */
    HuVecF target; /* Point the camera looks toward. */
    f32 phase; /* Current orbit angle, in degrees. */
    f32 elevation; /* Camera elevation above the target, in degrees. */
} M619CameraOrbit;

/* One scene object owns the player list, shared effects, models and sequence state. */
typedef struct M619Scene {
    M619Player *players[4]; /* Player states indexed by player slot. */
    M619ModelQueueNode *modelQueue; /* Head of the scene's queued-model list. */
    M619SceneAnimation sceneAnimation; /* State for the scene-wide animation. */
    M619Selection selection; /* Player and group selection state. */
    M619Launcher launcher; /* Launcher movement and transition state. */
    s32 frame; /* Frames elapsed in the current scene phase. */
    s32 finishFlag; /* Set when the scene is ready to finish. */
    s32 pulseState; /* State of the scene pulse effect. */
    s32 pulseFrame; /* Frames elapsed in the pulse effect. */
    s16 startState; /* Scene startup phase. */
    s16 displayState; /* Display and result phase. */
    s16 audioState; /* Scene audio phase. */
    s16 finishState; /* Scene completion phase. */
    s16 winnerState; /* Winner presentation phase. */
    s16 exitState; /* Exit-sequence phase. */
    s16 eligiblePlayers[4]; /* Player slots eligible for the current round. */
    s16 nightLighting; /* Whether the scene uses night lighting. */
    s16 adjustmentEnabled; /* Whether scene adjustment is enabled. */
    s16 winnerCount; /* Number of winners recorded for the result. */
    f32 materialTransparency[4][10]; /* Transparency values for scene materials. */
    HU3D_MODELID models[33]; /* Models loaded for the scene. */
    HU3D_MOTIONID defaultAnimation[1]; /* Default animation loaded for the scene model. */
    ANIMDATA *animations[1]; /* Animation data used by particle effects. */
    M619CameraOrbit camera; /* Scene camera position and orbit state. */
    s16 actionTimer; /* Frames remaining in the current action. */
    s16 actionPhase; /* Current action-sequence phase. */
} M619Scene;
#endif
