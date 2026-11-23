// Controls the player's rope selection, motion, and reactions.

#include "msm_se.h"
#include "datanum/charmot.h"
#include "datanum/mario.h"
#include "REL/framework/range.h"
#include "REL/m677DLL/rope.h"
#include "REL/m677DLL/motion_attributes.h"
#include "REL/m677DLL/path_follower.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/framework/vector_math.h"
#include <math.h>
#include <vector>

#include "REL/framework/event.h"
#include "REL/framework/event_callback_registration.h"
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/sound.h"
#include "REL/framework/world_units.h"
#include "REL/m677DLL/player_work.h"

extern "C" {
#include "game/charman.h"
#include "game/data.h"
}

// Motions loaded for the character controlled by the player.
const unsigned int playerMotions[] = {
    CHARMOT_HSF_c000m1_300, CHARMOT_HSF_c000m1_301, CHARMOT_HSF_c000m1_302,
    CHARMOT_HSF_c000m1_345, MARIO_HSF_c000m1_402, MARIO_HSF_c000m1_403
};

// Height to which the falling character is returned on landing.
float playerFloorHeight;

// Target position for each rope selection.
static Vec playerRopePositions[4] = {
    { cmToWorld(-400.0f), 0.0f, cmToWorld(400.0f) },
    { cmToWorld(-135.0f), 0.0f, cmToWorld(400.0f) },
    { cmToWorld(135.0f), 0.0f, cmToWorld(400.0f) },
    { cmToWorld(400.0f), 0.0f, cmToWorld(400.0f) },
};

// Sets up the character model, rope selection, and movement state.
PlayerWork::PlayerWork(int player, const Vec &start)
    : PlayerConfigSelection(player, 2), GameObject(0), events(0), state(&PlayerWork::fn_1_1CEA4),
      follower(start, 0.0f), heading(0.0f, 360.0f, 180.0f), targetHeading(180.0f),
      motionEffect(new IconRenderer),
      materialColors(new MaterialColors(&static_cast<Model &>(*this))), ropeChosen(0),
      ropeSelection(0, 4, 0), updatingRopePull(0), transition(0.0f, 0),
      action(static_cast<PlayerInfo &>(*this).isComputer()
                 ? static_cast<PlayerInputInterface *>(new ComputerPlayerInput(this))
                 : new HumanPlayerInput(this))
{
    motionEffect->setSlot(fn_1_7B6C(&ropeSelection));
    setLayer(2);
    setShadow();
    fn_8005DDEC(5);
    vectorSet(&offset, 0.0f, 0.0f, 0.0f);
    setPosition(&follower.position);
    setRotationY(heading.get());
    createCharacterMotions(&static_cast<MotionOwner &>(*this), playerMotions, 6);
    selectCharacterMotion(&static_cast<MotionOwner &>(*this), CHARMOT_HSF_c000m1_300);
    registerEventCallback(&events, std::bind1st(std::mem_fun(&PlayerWork::fn_1_1D70C), this), 2);
    registerEventCallback(&events, std::bind1st(std::mem_fun(&PlayerWork::fn_1_1D1E0), this), 4);
    registerEventCallback(&events, std::bind1st(std::mem_fun(&PlayerWork::fn_1_1D748), this), 5);
    char **eyeBitmaps = fn_8005C074(characterNo(), 4);
    materialColors->addTextureFilter(eyeBitmaps[0]);
    materialColors->addTextureFilter(eyeBitmaps[1]);
    playerFloorHeight = 0.0f;
}

// Releases the character's owned models and effects.
PlayerWork::~PlayerWork()
{
}

// Runs the character's action, updates its motion, and moves the selected rope during a pull.
void PlayerWork::update()
{
    (this->*state)();
    fn_1_1CD74(this);
    if (updatingRopePull)
        fn_1_1D684(this);
}

// Integrates character motion and updates the model's position and heading.
void fn_1_1CD74(PlayerWork *object)
{
    float previousHeight;
    previousHeight = object->follower.position.y;
    PSVECAdd(&object->offset, &object->follower.position, &object->follower.position);
    object->offset.y = object->offset.y - gravity;
    if (object->follower.position.y <= playerFloorHeight) {
        object->follower.position.y = playerFloorHeight;
        object->offset.y = rangeMaximum(0.0f, object->offset.y);
    }
    object->heading.step(object->targetHeading, 0.2);
    object->setPosition(&object->follower.position);
    object->setRotationY(object->heading.get());
}

// Leaves the current character action idle.
void PlayerWork::fn_1_1CEA4()
{
}

// Sets the target heading and optionally applies it immediately.
void fn_1_1CEA8(PlayerWork *self, float heading, bool applyImmediately)
{
    self->targetHeading = heading;
    if (applyImmediately)
        self->heading.set(heading);
}

// Sets the character material color blend.
void PlayerWork::fn_1_1CEFC(float value)
{
    materialColors->applyMaterialColorScale(value);
}

// Changes the character effect and synchronizes its enabled state.
void fn_1_1CF40(PlayerWork *self, bool enabled)
{
    int character = (short) static_cast<PlayerInfo &>(*self).characterNo();
    if (enabled)
        fn_8005C47C(character, 1);
    else
        fn_8005C430(character, 1);
    fn_8005DFC0(character, enabled);
}

// Sets the character effect's enabled state.
void fn_1_1CFB4(PlayerWork *self, bool enabled)
{
    fn_8005DFC0((short) static_cast<PlayerInfo &>(*self).characterNo(), enabled);
}

// Starts the current character action.
void fn_1_1CFEC(PlayerWork *work)
{
    work->action->beginInput();
    work->setPlayerState(&PlayerWork::fn_1_1D06C);
}

// Runs the active character action.
void PlayerWork::fn_1_1D06C()
{
    action->updateInput();
}

// Moves the rope selection in the requested direction.
void fn_1_1D0B4(PlayerWork *self, int direction)
{
    if (direction != 0) {
        int transitionValue;
        int sign;
        if (direction == 0) {
            transitionValue = 0;
        } else {
            if (direction < 0)
                sign = -1;
            else
                sign = 1;
            transitionValue = sign;
        }
        self->ropeSelection.add(transitionValue);
        self->motionEffect->setSlot(fn_1_7B6C(&self->ropeSelection));
        soundPlay(MSM_SE_MG677_ROPE_SELECTION_MOVE);
    }
}

// Locks the rope selection and starts the character's approach.
void fn_1_1D144(PlayerWork *self)
{
    if (!self->ropeChosen) {
        self->ropeChosen = true;
        self->motionEffect->beginShrink();
        enqueuePlayerWaypoint(self, &playerRopePositions[fn_1_7B6C(&self->ropeSelection)], 2.5f);
        sendEvent(0, 4, 0.0f);
        soundPlay(MSM_SE_MG677_ROPE_SELECTION_CONFIRM);
    }
}

// Confirms the rope selection in response to an event.
void PlayerWork::fn_1_1D1E0(const EventPacket &)
{
    fn_1_1D144(this);
}

// Starts the rope pull interpolation.
void fn_1_1D27C(PlayerWork *self)
{
    self->updatingRopePull = true;
    self->transition.set(0.0f, 1.0f, (long) secondsToFrames<double>(0.2));
}

// Selects a movement motion from the path speed.
int fn_1_1D2E8(double value)
{
    if (value < 3.0)
        return CHARMOT_HSF_c000m1_301;
    else if (value < 6.0)
        return CHARMOT_HSF_c000m1_302;
    else
        return CHARMOT_HSF_c000m1_345;
}

// Sets the queued path speed and starts the corresponding character motion.
static inline void startQueuedPathMotion(PlayerWork *self, float speed)
{
    fn_1_22150(&self->follower, speed);
    unsigned int dataNumber;
    if (speed < 3.0) {
        dataNumber = CHARMOT_HSF_c000m1_301;
    } else if (speed < 6.0) {
        dataNumber = CHARMOT_HSF_c000m1_302;
    } else {
        dataNumber = CHARMOT_HSF_c000m1_345;
    }
    shiftCharacterMotion(&static_cast<MotionOwner &>(*self), dataNumber,
                         secondsToFrames<double>(0.2), 0.0f, loopMotion);
}

// Enqueues a waypoint and starts the motion for its path speed.
void enqueuePlayerWaypoint(PlayerWork *self, const Vec *target, float speed)
{
    fn_1_221D8(&self->follower, target);
    startQueuedPathMotion(self, speed);
    self->setPlayerState(&PlayerWork::fn_1_1D55C);
}

// Enqueues a route and starts the motion for its path speed.
void enqueuePlayerRoute(PlayerWork *self, std::vector<Vec> *path, float speed)
{
    fn_1_22254(&self->follower, path);
    startQueuedPathMotion(self, speed);
    self->setPlayerState(&PlayerWork::fn_1_1D55C);
}

// Advances along the path until the character reaches the last point.
void PlayerWork::fn_1_1D55C()
{
    vectorSet(&offset, 0.0f, 0.0f, 0.0f);
    bool finished = fn_1_22454(&follower);
    targetHeading = fn_1_221AC(&follower);
    if (finished) {
        setPlayerState(&PlayerWork::fn_1_1CEA4);
    }
}

// Gets the position of the character model object used to blend the rope endpoint.
Vec *fn_1_1D618(PlayerWork *self, Vec *position)
{
    char *name = fn_8005C0F0((short) static_cast<PlayerInfo &>(*self).characterNo(), 4, 0);
    static_cast<Model &>(*self).getObjectPosition(name, position);
    return position;
}

// Blends the selected rope endpoint toward the character.
void fn_1_1D684(PlayerWork *self)
{
    Vec position;
    Rope *state;
    state = fn_1_1DEB8(fn_1_7B6C(&self->ropeSelection));
    fn_1_1E638(state, fn_1_1D618(self, &position), self->transition.update());
}

// Signals the character's motion effect.
void PlayerWork::fn_1_1D70C(const EventPacket &)
{
    motionEffect.get()->beginGrow();
}

// Returns the character to its idle action in response to an event.
void PlayerWork::fn_1_1D748(const EventPacket &)
{
    setPlayerState(&PlayerWork::fn_1_1CEA4);
}
