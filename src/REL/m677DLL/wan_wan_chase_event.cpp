// Runs the Wan-wan chase and moves the player along its route.

#include "msm_se.h"
#include "datadir_enum.h"
#include "datanum/charmot.h"
#include "REL/m677DLL/path_follower.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include "REL/framework/vector_math.h"
#include "REL/framework/interpolation.h"
#include "REL/m677DLL/wan_wan_chase_event.h"
#include <math.h>
#include "REL/framework/world_units.h"
#include "REL/framework/random.h"
#include "REL/framework/square.h"
#include "REL/framework/sound.h"
#include "REL/framework/model_motion.h"
#include "REL/m677DLL/particle_animations.h"
#include "REL/m677DLL/player_work.h"

extern "C" {
#include "game/data.h"
}

static void updateRouteMovement(WanWanChaseEvent *walk);
static void appendDecreasingArcRoute(WanWanChaseEvent *walk, std::vector<Vec> *route);
static void appendIncreasingArcRoute(WanWanChaseEvent *walk, std::vector<Vec> *route);
static bool withinApproachRadius(WanWanChaseEvent *walk, const Vec *first, const Vec *last);
static void updateWalkEffects(WanWanChaseEvent *walk);

// Four rope-selection approach positions followed by points for the chase paths and ellipse.
static Vec chaseRoutePoints[11] = {
    { cmToWorld(-400.0f), 0.0f, cmToWorld(400.0f) },
    { cmToWorld(-135.0f), 0.0f, cmToWorld(400.0f) },
    { cmToWorld(135.0f), 0.0f, cmToWorld(400.0f) },
    { cmToWorld(400.0f), 0.0f, cmToWorld(400.0f) },
    { 0.0f, 0.0f, cmToWorld(667.0f) },
    { cmToWorld(-400.0f), 0.0f, cmToWorld(450.0f) },
    { 0.0f, 0.0f, cmToWorld(600.0f) },
    { cmToWorld(400.0f), 0.0f, cmToWorld(450.0f) },
    { 0.0f, 0.0f, cmToWorld(300.0f) },
    { 0.0f, 0.0f, cmToWorld(1200.0f) },
    { 0.0f, 0.0f, cmToWorld(1000.0f) },
};

// Half the width of the curved section of the route.
static float chaseLoopHalfWidth = (chaseRoutePoints[7].x - chaseRoutePoints[5].x) / 2.0f;

// Half the depth of the curved section of the route.
static float chaseLoopHalfDepth = (chaseRoutePoints[6].z - chaseRoutePoints[8].z) / 2.0f;

// Center of the curved section of the route.
static Vec chaseLoopCenter = {
    (chaseRoutePoints[5].x + chaseRoutePoints[7].x) / 2.0f,
    0.0f,
    (chaseRoutePoints[6].z + chaseRoutePoints[8].z) / 2.0f,
};

// Route that sweeps 450 degrees around the chase ellipse with decreasing angles.
static std::vector<Vec> firstChaseLoopRoute;

// Route that sweeps 450 degrees around the same ellipse with increasing angles.
static std::vector<Vec> secondChaseLoopRoute;

// Character motions loaded for the walk.
static const unsigned int motionData[3] = { CHARMOT_HSF_c000m1_324, CHARMOT_HSF_c000m1_323,
                                            CHARMOT_HSF_c000m1_325 };

// Builds the two routes used after approaching the character.
WanWanChaseEvent::WanWanChaseEvent()
    : state(&WanWanChaseEvent::startCharacterMotionBeforeChase), stage(0),
      stepSoundCountdown(0), rumbleCountdown(0),
      heading(0.0f, 360.0f, 180.0f), targetHeading(180.0f), countdown(0)
{
    firstChaseLoopRoute.clear();
    secondChaseLoopRoute.clear();
    appendDecreasingArcRoute(this, &firstChaseLoopRoute);
    appendIncreasingArcRoute(this, &secondChaseLoopRoute);
}

// Releases the path walk's owned state.
WanWanChaseEvent::~WanWanChaseEvent()
{
}

// Runs the current state and updates the moving model.
int WanWanChaseEvent::step()
{
    if (!state)
        return 1;
    (this->*state)();
    if (stage >= 1) {
        if (!fn_1_221B4(&follower))
            updateRouteMovement(this);
        updateWalkEffects(this);
    }
    return 0;
}

// Loads the character's motions and sets up the model for the walk.
void WanWanChaseEvent::begin()
{
    PlayerWork *character = &PlayerWork::getChecked();
    createCharacterMotions(&static_cast<MotionOwner &>(*character), motionData, 3);
    double initialZ = 1000.0;
    vectorSet(&follower.position, 0.0f, 0.0f, initialZ);
    fn_1_22150(&follower, 7.0f);
    model.reset(new ModelMotion(DATANUM(_m677 << 16, 0x25)));
    model->addMotion(DATANUM(_m677 << 16, 0x26));
    model->setMotion(DATANUM(_m677 << 16, 0x26));
    model->setAttribute(loopMotion);
    model->setAttribute(0x1);
    model->setLayer(2);
    model->setShadow();
    model->setPosition(&follower.position);
    model->setRotationY(heading.get());
    countdown = 0;
    motionTime = 0.0f;
    stepSoundCountdown = 1;
    rumbleCountdown = 3;
}

// Starts a character motion and waits before walking.
void WanWanChaseEvent::startCharacterMotionBeforeChase()
{
    PlayerWork &character = PlayerWork::getChecked();
    shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_323, 16.0f,
                         0.0f, loopMotion);
    setCountdown(secondsToFrames<double>(2.0));
    setState(&WanWanChaseEvent::waitBeforeRoute);
}

// Waits for the countdown before entering the route state.
void WanWanChaseEvent::waitBeforeRoute()
{
    bool expired = false;
    if (countdown > 0 && --countdown == 0)
        expired = true;
    if (expired) {
        setState(&WanWanChaseEvent::walkRoute);
    }
}

// Approaches the character and starts the route for the chosen rope pair.
void WanWanChaseEvent::walkRoute()
{
    PlayerWork *character = &PlayerWork::getChecked();
    Vec *position = &character->getPosition();
    switch (stage) {
    case 0: {
        route.push_back(chaseRoutePoints[character->selectedRope()]);
        if (character->selectedRope() / 2 == 0)
            appendIncreasingArcRoute(this, &route);
        else
            appendDecreasingArcRoute(this, &route);
        model->resetAttribute(0x1);
        stage = 1;
        return;
    }
    case 1: {
        shiftCharacterMotion(&static_cast<MotionOwner &>(*character), CHARMOT_HSF_c000m1_324,
                             (float) secondsToFrames<double>(0.2), 0.0f, 0);
        fn_1_1CEA8(character,
                   (float) (180.0 * (atan2(follower.position.x - position->x,
                                           follower.position.z - position->z) /
                                     M_PI)),
                   0);
        character->rumble(1, 24);
        fn_1_22254(&follower, &route);
        stage = 2;
        return;
    }
    case 2:
        if (withinApproachRadius(this, position, &follower.position)) {
            if (character->selectedRope() / 2 == 0)
                enqueuePlayerRoute(character, &secondChaseLoopRoute, 7.0f);
            else
                enqueuePlayerRoute(character, &firstChaseLoopRoute, 7.0f);
            setState(&WanWanChaseEvent::finishRoute);
        }
    }
}

// At route completion, disables the character effect, pauses character motion,
// and hides the chase model.
void WanWanChaseEvent::finishRoute()
{
    if (!fn_1_221B4(&follower))
        return;
    PlayerWork *character = &PlayerWork::getChecked();
    fn_1_1CF40(character, false);
    character->setAttribute(pauseMotion);
    model->setAttribute(0x1);
    setState(0);
}

// Moves the model along the path and updates its heading.
static void updateRouteMovement(WanWanChaseEvent *walk)
{
    fn_1_22454(&walk->follower);
    walk->targetHeading = fn_1_221AC(&walk->follower);
    walk->heading.step(walk->targetHeading, 0.1);
    walk->model->setPosition(&walk->follower.position);
    walk->model->setRotationY(walk->heading.get());
}

// Builds the route with a decreasing arc angle.
static void appendDecreasingArcRoute(WanWanChaseEvent *, std::vector<Vec> *route)
{
    route->push_back(chaseRoutePoints[8]);
    Vec position;
    position.y = 0.0f;
    for (int index = 0; index <= 64; ++index) {
        float angle =
            (float) (M_PI * interpolateValue(270.0, -180.0, (double) index / 64.0) / 180.0);
        position.x = chaseLoopCenter.x + chaseLoopHalfWidth * cos(angle);
        position.z = chaseLoopCenter.z + chaseLoopHalfDepth * sin(angle);
        route->push_back(position);
    }
    route->push_back(chaseRoutePoints[9]);
}

// Builds the route with an increasing arc angle.
static void appendIncreasingArcRoute(WanWanChaseEvent *, std::vector<Vec> *route)
{
    route->push_back(chaseRoutePoints[8]);
    Vec position;
    position.y = 0.0f;
    for (int index = 0; index <= 64; ++index) {
        float angle =
            (float) (M_PI * interpolateValue(-90.0, 360.0, (double) index / 64.0) / 180.0);
        position.x = chaseLoopCenter.x + chaseLoopHalfWidth * cos(angle);
        position.z = chaseLoopCenter.z + chaseLoopHalfDepth * sin(angle);
        route->push_back(position);
    }
    route->push_back(chaseRoutePoints[10]);
    route->push_back(chaseRoutePoints[9]);
}

// Stores a radius and computes its square for distance tests.
struct Distance {
    float value;
    // Stores the radius for distance tests.
    Distance(float amount) : value(amount)
    {
    }
    // Returns the square of the radius.
    double squared() const
    {
        return square(value);
    }
};

// Returns the squared distance between two positions.
static inline float squareDistance(const Vec *a, const Vec *b)
{
    return PSVECSquareDistance(a, b);
}

// Tests whether the positions lie within the walk's approach radius.
static bool withinApproachRadius(WanWanChaseEvent *, const Vec *first, const Vec *second)
{
    Distance radius(200.0f);
    return squareDistance(first, second) <= radius.squared();
}

// Tracks motion time and emits the walk's particle effects.
static void updateWalkEffects(WanWanChaseEvent *walk)
{
    ParticleAnimations *resource;
    PlayerWork *character;
    int nextCountdown;
    float currentTime = walk->model->getMotionTime();
    if (currentTime < walk->motionTime) {
        resource = &ParticleAnimations::getChecked();
        resource->createTanBurstParticles(&walk->follower.position, 3, 1.0f, 0.5f, 4);
        unsigned char rumbleDue = 0;
        if (walk->rumbleCountdown > 0 && --walk->rumbleCountdown == 0)
            rumbleDue = 1;
        if (rumbleDue) {
            walk->rumbleCountdown = 1;
            character = &PlayerWork::getChecked();
            character->rumble(0, (int) secondsToFrames<double>(0.12));
        }
        unsigned char stepSoundDue = 0;
        if (walk->stepSoundCountdown > 0 && --walk->stepSoundCountdown == 0)
            stepSoundDue = 1;
        if (stepSoundDue) {
            nextCountdown = (int) RandomFloat(2.0f, 6.0f);
            walk->stepSoundCountdown = nextCountdown;
            soundPlay(MSM_SE_GUIDE_EVENT_NPC_TALK);
        }
    }
    walk->motionTime = currentTime;
}
