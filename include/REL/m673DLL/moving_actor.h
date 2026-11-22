// Declares the moving actor and its event and motion states.

#ifndef REL_M673DLL_MOVING_ACTOR_H
#define REL_M673DLL_MOVING_ACTOR_H
#include "dolphin/types.h"
#include <vector>
#include <memory>
#include "REL/framework/singleton.h"
#include "dolphin/mtx.h"

#include "REL/framework/game_object2.h"
#include "REL/framework/model.h"
#include "REL/m673DLL/collision_body.h"
#include "REL/framework/event_hm.h"

#include "REL/m673DLL/colliding_particles.h"

#include "REL/m673DLL/follow_effect.h"
#include "REL/framework/model_motion.h"

// Controls the moving actor, its route, and its scene effects.
struct MovingActor : GameObject, ModelMotion, MinigameSingleton<MovingActor> {
    CollisionBody component; // Body used for actor collision checks.
    Vec velocity; // Current movement direction.
    // Returns the movement state.
    s32 getMovementState()
    {
        return movementState;
    }
    f32 headingAngle; // Current heading about the vertical axis.
    Position3 targetPosition; // Path point selected for the current movement.
    f32 targetAngle; // Heading toward the selected point.
    s32 movementState; // Current movement phase.
    Position3 effectPosition; // Position of the attached quad.
    f32 movementSpeed; // Displacement per movement update.
    f32 drawAlpha; // Alpha multiplier for the attached quad.
    u8 ready; // Signals completion of the current phase.
    f32 fallVelocityY; // Vertical displacement applied each fall frame.
    s32 elapsedFrames; // Frames since the start of the fall sequence.
    s32 delayFrames; // Frame delay before the actor falls.
    f32 fallVelocityX; // Horizontal X displacement per fall frame.
    f32 unk98; /* The game's code never reads this field. */
    f32 fallVelocityZ; // Horizontal Z displacement per fall frame.
    f32 transitionSeconds; // Time spent in the motion transition.
    f32 remainingSeconds; // Time remaining for motion selection.
    u32 approachSoundFrames; // Frame counter for the approach sound.
    u32 crossingSoundFrames; // Frame counter for the crossing sound.
    s32 soundHandle; // Handle of the actor's current sound.
    std::auto_ptr<OwnedModel> model; // Model positioned with the moving actor.
    Position3 modelPosition; // Position supplied to the owned model.
    s32 pointIndex; // Index of the current path point.
    std::vector<Vec> points; // Path points in the order chosen for this run.
    u32 currentMotion; // Data number of the current animation.
    std::auto_ptr<BurstEmitter> collidingParticles; // Directional particle effect.
    std::auto_ptr<FollowEffect> followEffect; // Quad drawn at the effect position.
    EventHandlerManager channel0Events; // Receives the moving actor's scene events on channel 0.
    EventHandlerManager channel1Events; // Registers this object on event channel 1 for its
                                        // lifetime.

    // Member function that advances an actor state.
    typedef void (MovingActor::*State)();
    State state; // Current frame update.
    // Sets the next state callback.
    void setState(State value)
    {
        state = value;
    }
    u32 motionResource; // Data number selected for the next motion.
    State motionState; // Current movement update.
    State nextMotionCallback; // Movement update selected for the next transition.
    s32 motionFrames; // Frames elapsed in the selected motion.
    s32 timedMotionChance; // Chance of selecting the timed motion.
    s32 crossingMotionChance; // Chance of selecting the crossing motion.
    // Starts the falling state, sound, and motion.
    void fn_1_AE54(const EventPacket &event);
    // Hides the owned model.
    void fn_1_AF1C(const EventPacket &event);
    // Starts movement and motion selection.
    void fn_1_AF5C(const EventPacket &event);
    // Hides the follow effect and starts the final turn.
    void fn_1_AFD8(const EventPacket &event);
    // Chooses the ending motion from the event result.
    void fn_1_B05C(const EventPacket &event);
    // Leaves the moving actor unchanged in its idle state.
    void fn_1_AE50();
    // Advances the delayed fall and updates the associated model.
    void fn_1_9BF4();
    // Advances movement, counts down the remaining time, and updates the follow effect.
    void fn_1_9EB0();
    // Chooses the next motion using the remaining time and motion probabilities.
    void fn_1_A718();
    // Finishes the active movement and turns back toward the default heading.
    void fn_1_AC34();
    // Advances the timed motion sequence and its sound effects.
    void fn_1_A34C();
    // Advances movement and returns to motion selection at the arena boundary.
    void fn_1_A1B4();
    // Shifts to the requested actor motion when it changes.
    inline void fn_1_9B44(u32 motion, u8 enabled);
    // Turns an angle toward its target using the shorter direction.
    u8 fn_1_B208(f32 *angle, f32 target);
    // Marks the actor ready.
    void fn_1_9BE8();
    // Resets the motion timer and selects the next path point.
    void fn_1_A2D4();
    // Selects the turning state and the next path point.
    void fn_1_A144();
    // Moves toward the selected position and switches state on arrival.
    void fn_1_A000();
    // Selects the turning state and the next path point.
    void fn_1_9F90();
    // Turns toward the target heading and selects the transition or steady motion.
    void fn_1_A9D8();
    // Creates the moving actor and its model, collision body, and effects.
    MovingActor(s32 dataNumber, Position3 *position, f32 angle);
    // Releases the actor model, effects, and event handlers.
    virtual ~MovingActor();
    // Advances the actor state and updates its model and particle group.
    virtual void update();
};

// Returns the angle from one point to another in the XZ plane.
f32 fn_1_B4F4(Point3d *from, Point3d *to);

// Selects the next path point and updates the movement direction and target heading.
void fn_1_AB5C(MovingActor *object);

#endif
