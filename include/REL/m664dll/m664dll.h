/* Wrasslin' Rapids player control, raft motion and course-effect state. */
#ifndef M664DLL_RECOVERED_H
#define M664DLL_RECOVERED_H

#include "dolphin/types.h"
#include "dolphin/mtx/GeoTypes.h"
#include "game/mg/actman.h"
#include "game/charman.h"

typedef struct M664PlayerRecord {
    MGPLAYER *player; /* Actor-manager player and its collision actor. */
    s16 characterNo; /* Character selected in the player configuration. */
    s16 modelId; /* Character model belonging to the player actor. */
    s16 motionIds[7]; /* Character motions loaded for this minigame. */
    s16 currentMotion; /* Hit/fall reaction index into motionIds; zero is the unset reaction
                        * marker. */
    s16 savedMotionId; /* Actor motion restored after a temporary hit reaction. */
    s32 actionState; /* Zero: normal play; negative: leaving play; positive: hit reaction. */
    s32 sequenceCount; /* Remaining-player count recorded on elimination during main play; zero
                        * until recorded. */
    s32 actionTimer; /* Scripted-action counter; hit triggers reset it, natural falls do not. */
    s32 movementTimer; /* Scripted-flight frame counter; the model hides at frame 120 while this
                        * keeps advancing. */
    Point3d movementOrigin; /* Starting position, then the current scripted-flight position. */
    Point3d flightDestination; /* Destination chosen when a course prop knocks the player away. */
    Point3d movementDirection; /* Per-frame displacement toward the flight destination. */
    f32 launchHeight; /* Player's world Y coordinate when scripted flight begins. */
    s32 padNo; /* Controller port copied from the player configuration. */
    s16 fallEffectModelId; /* Animated effect shown at the water surface when the player falls. */
    s16 fallEffectMotionId; /* Motion played by the falling-player effect. */
    s32 stunSoundLatched; /* Latches the stun sound until the player reaches stun type 6. */
} M664PlayerRecord;

typedef struct M664PlayerSettings {
    s32 difficulty; /* Configured computer difficulty, 0-3. */
    s32 type; /* Nonzero selects computer-controlled input. */
    /* AI chase frames, action delay or airborne action frames, opponent index (-1: none),
     * return-to-center flag (only cleared here), stick X/Y, pressed/held buttons,
     * and a final word not read by this module. */
    s32 sessionState[9];
} M664PlayerSettings;
typedef M664PlayerSettings M664InputSettings;

typedef struct M664MotionState {
    s16 modelId[2]; /* Raft models moved together; the second supplies collision geometry. */
    Point3d position; /* Shared world position of the raft models. */
    Point3d rotation; /* Shared rotation in degrees. */
    Point3d targetRotation; /* Pitch and roll approached during raft sway, in degrees. */
    s32 heightIncreasing; /* Nonzero advances the vertical-bob phase toward +90 degrees. */
    s32 yawIncreasing; /* Nonzero advances the yaw-sway phase toward +180 degrees. */
    f32 heightPhase; /* Vertical-bob phase in degrees, clamped to -90 through +90. */
    f32 yawPhase; /* Yaw-sway phase in degrees, clamped to -180 through +180. */
} M664MotionState;

typedef struct M664ModelPair {
    s16 modelId; /* Animated opening-scene model. */
    s16 objectModelId; /* Companion model positioned at the opening model's fook00 object. */
} M664ModelPair;

typedef M664ModelPair ModelPair;

typedef struct M664SoundHandles {
    s32 soundHandle[2]; /* Opening and course effects; -1 is absent, -2 stops course restarts. */
} M664SoundHandles;

typedef struct M664EffectModel {
    s16 modelId; /* Falling course-prop model. */
    Point3d position; /* Current world position along the prop's flight. */
    Point3d startPosition; /* Release position sampled from the course model's bom object. */
    f32 destinationX; /* Flight destination X coordinate in world units. */
    f32 destinationY; /* Flight destination Y coordinate in world units. */
    f32 destinationZ; /* Flight destination Z coordinate in world units. */
    u8 unknown28[16]; /* Unused bytes; no code in this module reads them. */
    s32 state38; /* 0: waiting; 1: release motion; 2: flight; 3: impact scale pulse; 4: finished. */
    s32 motionFrame; /* Flight frames; interpolation divides by 120 and continues past it until
                      * contact. */
    s32 state40; /* Flight: 0 before contact, -1 miss, 1 landing, 2 player hit; then scale-pulse
                  * count. */
    s16 effectModelId; /* Impact-effect model shown at the prop's position when landing, missing or
                        * hitting a player. */
    s16 effectMotionId; /* Motion played by the impact effect. */
} M664EffectModel;

#endif
