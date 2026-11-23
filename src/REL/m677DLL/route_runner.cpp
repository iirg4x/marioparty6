// Moves event models along queued waypoints and turns them toward their heading.

#include "msm_se.h"
#include "datadir_enum.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include "REL/framework/vector_math.h"
#include <math.h>
#include "REL/framework/sound.h"
#include "REL/framework/world_units.h"
#include "REL/framework/range.h"
#include "REL/m677DLL/route_runner.h"
#include "REL/framework/event_manager.h"
#include "REL/m677DLL/player_work.h"
#include "REL/m677DLL/particle_animations.h"

extern "C" {
#include "game/data.h"

// Four rope-selection approach positions followed by the runner's starting position.
Vec routePositions[5] = {
    { cmToWorld(-400.0f), 0.0f, cmToWorld(400.0f) },
    { cmToWorld(-135.0f), 0.0f, cmToWorld(400.0f) },
    { cmToWorld(135.0f), 0.0f, cmToWorld(400.0f) },
    { cmToWorld(400.0f), 0.0f, cmToWorld(400.0f) },
    { cmToWorld(0.0f), 0.0f, cmToWorld(1200.0f) },
};
}

// Initializes the route model and its movement and turning state.
RouteRunner::RouteRunner() : GameObject(12), heading(0.0f, 360.0f, 180.0f)
{
    direction = 180.0f;
    unk64 = 12.0f;
    movementRate = 0.0f;
    speed = (float) framesToSeconds<double>(300.0);
    idle = true;
    turned = true;
    setSoundCountdown((int) secondsToFrames<double>(0.2));

    PlayerWork &character = PlayerWork::getChecked();
    model.reset(new ModelMotion(DATANUM(_m677 << 16, 0x27)));
    // Motion resources preloaded for the route model.
    static const unsigned long motions[3] = {
        DATANUM(_m677 << 16, 0x28), DATANUM(_m677 << 16, 0x2A), DATANUM(_m677 << 16, 0x2B)
    };
    model->addMotions(motions, 3);
    model->setMotion(DATANUM(_m677 << 16, 0x2A));
    model->setAttribute(loopMotion);
    model->setShadow();

    setState(&RouteRunner::waitIdle);
    vectorSet(&position, 0.0f, 0.0f, 0.0f);
    vectorSet(&target, 0.0f, 0.0f, 0.0f);
    position = routePositions[4];
    model->setPosition(&position);
    model->setRotationY(heading.get());
}

// Leaves the route runner in its idle state.
void RouteRunner::waitIdle()
{
}

// Releases the route model and waypoint queue.
RouteRunner::~RouteRunner()
{
}

// Moves toward the active waypoint and handles arrival.
unsigned char RouteRunner::move()
{
    ParticleAnimations *particles = &ParticleAnimations::getChecked();
    aimAtWaypoint(this);

    position.x += movementRate * velocity.x;
    position.y += movementRate * velocity.y;
    position.z += movementRate * velocity.z;

    if (distance(position, target) >= speed) {
        direction =
            (float) (180.0 * (atan2((target.x - position.x), (target.z - position.z)) / M_PI));
    }

    turned = false;
    if (distance(position, target) < movementRate) {
        position = target;
        path.pop_front();
        if (path.empty()) {
            idle = true;
        } else {
            target = path.front();
        }
    }
    if (soundDue()) {
        particles->createTanBurstParticles(&position, 4, 0.4f, 0.01f, 4);
        soundPlay(MSM_SE_GUIDE_FOOTSTEP_SLOW);
        setSoundCountdown((int) secondsToFrames<double>(0.2));
    }
    return idle;
}

// Turns the model toward the target heading and reports arrival.
bool RouteRunner::turn()
{
    heading.step(direction, 0.1);
    if (matchesWrappedAngle(&heading, direction)) {
        turned = true;
    }
    return turned;
}

// Queues a waypoint and starts the movement motion.
void RouteRunner::queueWaypoint(Vec value, float amount)
{
    this->path.push_back(value);
    this->movementRate = amount;
    this->model->shiftMotion(DATANUM(_m677 << 16, 0x2A), 4.0f, 0.0f, loopMotion);
    this->idle = 0;
}

// Changes the target heading and marks the turn as pending.
static void setTargetHeading(RouteRunner *runner, float heading, float value)
{
    runner->direction = heading;
    runner->unk64 = value;
    runner->turned = false;
}

// Runs the current state and applies movement and heading to the model.
void RouteRunner::update()
{
    (this->*state)();
    selectWaypoint(this);
    if (idle == 0) {
        move();
    }
    if (!turned) {
        turn();
    }
    model->setPosition(&position);
    model->setRotationY(heading.get());
}

// Returns the distance between two positions.
double RouteRunner::distance(Vec from, Vec to)
{
    Vec delta;
    PSVECSubtract(&from, &to, &delta);
    return std::sqrt(PSVECSquareMag(&delta));
}

// Sets the movement direction toward the active waypoint.
void aimAtWaypoint(RouteRunner *runner)
{
    Vec delta;
    PSVECSubtract(&runner->target, &runner->position, &delta);
    PSVECNormalize(&delta, &runner->velocity);
}

// Selects the first queued waypoint as the target.
void selectWaypoint(RouteRunner *runner)
{
    if (!runner->path.empty()) {
        runner->target = runner->path.front();
        runner->idle = 0;
    }
}

// Steers the route model toward the character.
void RouteRunner::turnTowardPlayer()
{
    PlayerWork &character = PlayerWork::getChecked();
    Vec *characterPosition = &character.follower.position;
    Vec targetPosition = *characterPosition;
    if (this->path.empty()) {
        this->model->shiftMotion(DATANUM(_m677 << 16, 0x28), 32.0f, 0.0f, loopMotion);
        float angle = (float) (180.0
                               * (atan2((targetPosition.x - this->position.x),
                                        (targetPosition.z - this->position.z))
                                  / M_PI));
        this->direction = angle;
        this->unk64 = 3.0f;
        this->turned = false;
        this->setState(&RouteRunner::waitIdle);
    }
}

// Queues a waypoint around the character and starts the runner's next motion.
void RouteRunner::moveBesidePlayer()
{
    PlayerWork &character = PlayerWork::getChecked();
    Vec *characterPosition = &character.follower.position;
    Vec waypoint = *characterPosition;
    waypoint.x = (float) (waypoint.x + 100.0 * cos((M_PI * this->waypointAngle) / 180.0));
    waypoint.z = (float) (waypoint.z + 100.0 * sin((M_PI * this->waypointAngle) / 180.0));
    queueWaypoint(waypoint, speed);
    this->model->shiftMotion(DATANUM(_m677 << 16, 0x2B), 4.0f, 0.0f, 1);
    this->setState(&RouteRunner::waitIdle);
}

// Tests whether the angle equals the current wrapped heading.
bool matchesWrappedAngle(Range<float> *heading, float angle)
{
    return heading->current == heading->wrap(angle);
}
