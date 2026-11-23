// Runs Coins stolen and moves the models used in the event.

#include "msm_se.h"
#include "datadir_enum.h"
#include "datanum/charmot.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include "REL/framework/random.h"
#include "REL/framework/sound.h"
#include "REL/framework/square.h"
#include "REL/framework/vector_math.h"
#include "REL/framework/world_units.h"
#include "REL/m677DLL/coins_stolen_event.h"
#include "REL/m677DLL/coin_model_transform.h"
#include "REL/m677DLL/particle_animations.h"
#include "REL/m677DLL/player_work.h"
#include "game/charman.h"
#include "humath.h"

extern "C" {
#include "game/audio.h"
}

// Character motion resources preloaded for the effect sequence.
static const unsigned int motions[] = {
    CHARMOT_HSF_c000m1_306, CHARMOT_HSF_c000m1_315, CHARMOT_HSF_c000m1_322,
    CHARMOT_HSF_c000m1_323, CHARMOT_HSF_c000m1_325, CHARMOT_HSF_c000m1_467
};
// The game's code never reads this table.
static const float unknownConstants[] = {40.0f, 20.0f, 340.0f, 320.0f};
// Initial scale applied to the effect models.
static const Vec scale = {0.8f, 0.8f, 0.8f};

// Four rope-selection approach positions followed by the center of the runners' final waypoints.
Vec runnerDestinations[5] = {
    { cmToWorld(-400.0f), 0.0f, cmToWorld(400.0f) },
    { cmToWorld(-135.0f), 0.0f, cmToWorld(400.0f) },
    { cmToWorld(135.0f), 0.0f, cmToWorld(400.0f) },
    { cmToWorld(400.0f), 0.0f, cmToWorld(400.0f) },
    { cmToWorld(0.0f), 0.0f, cmToWorld(1200.0f) },
};
// The game's code never reads this table.
float lbl_1_data_A78[4] = {
    cmToWorld(1500.0f), cmToWorld(1200.0f), cmToWorld(1200.0f), cmToWorld(1500.0f),
};

// Initializes the falling effects and the timer for particle bursts.
CoinsStolenEvent::CoinsStolenEvent()
    : state(&CoinsStolenEvent::scatterFallingEffects), frame(0), effectEnabled(false),
        nextRunner(0),
      stateCountdown(0)
{
    setEffectCountdown((int)secondsToFrames(0.2));
    for (int i = 0; i < 20; ++i) {
        CoinsStolenEffectModel &entry = entries[i];
        vectorSet(&entry.position, 0.0f, 0.0f, 0.0f);
        double angle = RandomFloat(0.0f, 360.0f);
        entry.rotation.x = 0.0f;
        entry.rotation.y = (float)angle;
        entry.rotation.z = 0.0f;
        vectorSet(&entry.velocity, 0.0f, 0.0f, 0.0f);
        entry.elapsedTime = 0.0f;
        entry.startDelay = (float)(0.06 * (double)i);
        entry.active = true;
    }
}

// Releases the model owned by the effect entry.
CoinsStolenEffectModel::~CoinsStolenEffectModel() {}

// Default-constructs the model owned by an effect entry.
CoinsStolenEffectModel::CoinsStolenEffectModel() {}

// Releases the effect entries and runners.
CoinsStolenEvent::~CoinsStolenEvent()
{
}

// Advances the current scene state and updates the effects.
int CoinsStolenEvent::step()
{
    if (!state) {
        return 1;
    }
    (this->*state)();
    updateEffectTransforms();
    spawnTimedParticleBursts();
    return 0;
}

// Loads the character motions and creates the effect models.
void CoinsStolenEvent::begin()
{
    PlayerWork &owner = PlayerWork::getChecked();
    createCharacterMotions(&static_cast<MotionOwner &>(owner), motions, 6);
    for (int i = 0; i < 20; ++i) {
        CoinsStolenEffectModel *entry = &entries[i];
        entry->model.reset(new OwnedModel(DATANUM(_m677 << 16, 0x2C)));
        entry->model->setAttribute(1);
        entry->model->setShadow();
        entry->model->setScale(const_cast<Vec *>(&scale));
        entry->active = true;
    }
    configureSceneMode(0);
}

// Updates the effect positions and rotates their models.
void CoinsStolenEvent::updateEffectTransforms()
{
    PlayerWork &owner = PlayerWork::getChecked();
    float height = owner.height();
    Vec bodyPosition = {0.0f, 0.0f, 0.0f};
    for (int i = 0; i < 20; ++i) {
        CoinsStolenEffectModel *entry = &entries[i];
        switch (entry->type) {
            case 1:
                if (entry->active && secondsToFrames(entry->startDelay) < (float)frame) {
                    float elapsedSquared = entry->elapsedTime * entry->elapsedTime;
                    entry->position.y -= 19.6133f * elapsedSquared;
                    if (entry->position.y <= height) {
                        fn_8005D5A0(1, &entry->position);
                        entry->model->setAttribute(1);
                        entry->position.y = height;
                        entry->active = false;
                        soundPlay(MSM_SE_CMN_COIN_GET);
                    }
                    entry->elapsedTime += 1.0 / 60.0;
                }
                break;
            case 2:
                if (entry->active) {
                    updateFlyingEffect(i);
                }
                break;
            case 3:
                if (entry->active) {
                    runners[i].model->getObjectPosition("body", &bodyPosition);
                    bodyPosition.y += 120.0;
                    entry->position.x = bodyPosition.x;
                    entry->position.y = bodyPosition.y;
                    entry->position.z = bodyPosition.z;
                }
                break;
        }
    }
    for (int i = 0; i < 20; ++i) {
        CoinsStolenEffectModel *entry = &entries[i];
        updateCoinModelTransform(entry);
    }
    ++frame;
}

// Spawns timed particle bursts at the character's position.
void CoinsStolenEvent::spawnTimedParticleBursts()
{
    if (effectEnabled) {
        const Vec *origin;
        unsigned char expired = 0;
        if (effectCountdown > 0 && --effectCountdown == 0) {
            expired = 1;
        }
        if (expired) {
            ParticleAnimations *particles;
            PlayerWork &owner = PlayerWork::getChecked();
            origin = owner.getPositionPtr();
            Vec position = *origin;
            particles = &ParticleAnimations::getChecked();
            particles->createTanBurstParticles(&position, 16, 0.5f, 2.0f, 1);
            soundPlay(MSM_SE_GUIDE_KURIBO_TALK);
            setEffectCountdown((int)secondsToFrames(0.2));
        }
    }
}

// Places each falling effect above the character with a random horizontal offset.
void CoinsStolenEvent::scatterFallingEffects()
{
    PlayerWork &owner = PlayerWork::getChecked();
    PlayerWork &positionOwner = PlayerWork::getChecked();
    const Vec *position = positionOwner.getPositionPtr();
    for (int i = 0; i < 20; ++i) {
        CoinsStolenEffectModel *entry = &entries[i];
        entry->model->resetAttribute(1);
        entry->position.x = position->x;
        entry->position.y = position->y;
        entry->position.z = position->z;
        entry->position.x += RandomFloat(-30.0f, 30.0f);
        entry->position.z += RandomFloat(-30.0f, 30.0f);
        entry->position.y += 1000.0f;
        entry->type = 1;
    }
    shiftCharacterMotion(&static_cast<MotionOwner &>(owner), CHARMOT_HSF_c000m1_325,
                             16.0f, 0.0f, 0);
    setStateCountdown((int)secondsToFrames(1.0));
    setState(&CoinsStolenEvent::waitForOpeningCountdown);
}

// Starts the next character motion when the initial countdown ends.
void CoinsStolenEvent::waitForOpeningCountdown()
{
    PlayerWork &owner = PlayerWork::getChecked();
    unsigned char expired = 0;
    if (stateCountdown > 0 && --stateCountdown == 0) {
        expired = 1;
    }
    if (expired) {
        shiftCharacterMotion(&static_cast<MotionOwner &>(owner), CHARMOT_HSF_c000m1_467,
                             30.0f, 0.0f, loopMotion);
        setState(&CoinsStolenEvent::waitForFinalLanding);
    }
}

// Starts the character's next motion when the final falling effect lands.
void CoinsStolenEvent::waitForFinalLanding()
{
    if (!entries[19].active) {
        PlayerWork &owner = PlayerWork::getChecked();
        fn_1_1CFB4(&owner, false);
        shiftCharacterMotion(&static_cast<MotionOwner &>(owner), CHARMOT_HSF_c000m1_306,
                             16.0f, 0.0f, 0);
        CharFXPlay(owner.characterNo(), MSM_SE_CHARVOICE_HAPPY_CHEER);
        setState(&CoinsStolenEvent::startRunnerPaths);
    }
}

// Starts the runners on their paths after the character motion ends.
void CoinsStolenEvent::startRunnerPaths()
{
    PlayerWork &owner = PlayerWork::getChecked();
    if (owner.isMotionEnd()) {
        configureSceneMode(1);
        setStateCountdown((int)secondsToFrames(2.0));
        setState(&CoinsStolenEvent::waitForLeadRunnerToStop);
    }
}

// Changes the character motion and advances when the lead runner stops.
void CoinsStolenEvent::waitForLeadRunnerToStop()
{
    PlayerWork &owner = PlayerWork::getChecked();
    unsigned char expired = 0;
    if (stateCountdown > 0 && --stateCountdown == 0) {
        expired = 1;
    }
    if (expired) {
        shiftCharacterMotion(&static_cast<MotionOwner &>(owner), CHARMOT_HSF_c000m1_300,
                             16.0f, 0.0f, loopMotion);
        setStateCountdown((int)secondsToFrames(100.0));
    }
    if (runners[0].path.empty()) {
        fn_1_1CFB4(&owner, true);
        shiftCharacterMotion(&static_cast<MotionOwner &>(owner), CHARMOT_HSF_c000m1_323,
                             16.0f, 0.0f, 0);
        setStateCountdown((int)secondsToFrames(2.0));
        setState(&CoinsStolenEvent::enableParticleBursts);
    }
}

// Changes the runners' state and enables the timed particle bursts.
void CoinsStolenEvent::enableParticleBursts()
{
    PlayerWork &owner = PlayerWork::getChecked();
    unsigned char expired = 0;
    if (stateCountdown > 0 && --stateCountdown == 0) {
        expired = 1;
    }
    if (expired) {
        configureSceneMode(2);
        effectEnabled = true;
        setState(&CoinsStolenEvent::waitForLeadRunnerPath);
    }
}

// Waits for the lead runner to finish its path.
void CoinsStolenEvent::waitForLeadRunnerPath()
{
    PlayerWork &owner = PlayerWork::getChecked();
    Vec *origin = &owner.follower.position;
    Vec position = *origin;
    // The game initializes this function-local value once and never reads it.
    static int unknown = 0;
    if (runners[0].path.empty()) {
        shiftCharacterMotion(&static_cast<MotionOwner &>(owner), CHARMOT_HSF_c000m1_315,
                             60.0f, 0.0f, loopMotion);
        setStateCountdown((int)secondsToFrames(1.0));
        setState(&CoinsStolenEvent::launchEffectsAtRunners);
    }
}

// Launches one effect toward each runner, with a delay between launches.
void CoinsStolenEvent::launchEffectsAtRunners()
{
    unsigned char expired = 0;
    if (stateCountdown > 0 && --stateCountdown == 0) {
        expired = 1;
    }
    if (expired) {
        PlayerWork::getChecked().rumble(1, (int)secondsToFrames(0.1));
        float angle = (float)(M_PI * runners[nextRunner].waypointAngle / 180.0);
        CoinsStolenEffectModel *entry = &entries[nextRunner];
        double cosine, sine;
        sine = std::sin(angle);
        cosine = std::cos(angle);
        vectorSet(&entry->velocity, cosine, 1.0f, sine);
        entry->active = true;
        entry->type = 2;
        entry->model->resetAttribute(1);
        ++nextRunner;
        setStateCountdown((int)secondsToFrames(0.2));
        if (nextRunner >= 6) {
            setStateCountdown((int)secondsToFrames(1.0));
            effectEnabled = false;
            setState(&CoinsStolenEvent::sendRunnersToDestinations);
        }
    }
}

// Sends the runners to their final positions once the countdown ends.
void CoinsStolenEvent::sendRunnersToDestinations()
{
    PlayerWork &owner = PlayerWork::getChecked();
    unsigned char expired = 0;
    if (stateCountdown > 0 && --stateCountdown == 0) {
        expired = 1;
    }
    if (expired) {
        configureSceneMode(4);
        shiftCharacterMotion(&static_cast<MotionOwner &>(owner), CHARMOT_HSF_c000m1_322,
                             16.0f, 0.0f, loopMotion);
        setState(&CoinsStolenEvent::waitForFinalRunnerPath);
    }
}

// Ends the scene once the lead runner has no waypoints left.
void CoinsStolenEvent::waitForFinalRunnerPath()
{
    if (runners[0].path.empty()) {
        setState(0);
    }
}

// Sets positions, paths or effect directions for the supplied scene mode.
void CoinsStolenEvent::configureSceneMode(int mode)
{
    PlayerWork &owner = PlayerWork::getChecked();
    for (int i = 0; i < 6; ++i) {
        RouteRunner &runner = runners[i];
        switch (mode) {
            case 0: {
                Vec position;
                Vec initialPosition = runner.position;
                position.x = initialPosition.x;
                position.y = initialPosition.y;
                position.z = initialPosition.z;
                runner.waypointAngle = i * 60;
                position.x += 150.0 * std::cos(M_PI * runner.waypointAngle / 180.0);
                position.z += 150.0 * std::sin(M_PI * runner.waypointAngle / 180.0);
                Vec destination = position;
                runner.position.x = destination.x;
                runner.position.y = destination.y;
                runner.position.z = destination.z;
                break;
            }
            case 1:
                queueRunnerApproach(runner, i);
                break;
            case 2:
                runner.setState(&RouteRunner::moveBesidePlayer);
                break;
            case 3:
                entries[i].velocity.x = 2.0 * std::cos(M_PI * runner.waypointAngle / 180.0);
                entries[i].velocity.y = 1.0f;
                entries[i].velocity.z = 2.0 * std::sin(M_PI * runner.waypointAngle / 180.0);
                entries[i].active = true;
                entries[i].type = 2;
                break;
            case 4:
                queueRunnerDestination(runner, i);
                break;
        }
    }
}

// Updates the effect in flight and attaches it when it reaches the runner.
void CoinsStolenEvent::updateFlyingEffect(int index)
{
    CoinsStolenEffectModel *entry = &entries[index];
    Vec body;
    runners[index].model->getObjectPosition("body", &body);
    entry->velocity.x *= 0.95;
    entry->velocity.y -= 0.1;
    entry->velocity.z *= 0.95;

    Vec origin, current;
    vectorSet(&origin, body.x, 0.0, 0.0);
    vectorSet(&current, entry->position.x, 0.0, 0.0);
    float shift = entry->velocity.x * cmToWorld(10.0);
    if (square(PSVECSquareDistance(&origin, &current)) >= shift) {
        entry->position.x += shift;
    }
    vectorSet(&origin, 0.0, 0.0, body.z);
    vectorSet(&current, 0.0, 0.0, entry->position.z);
    shift = entry->velocity.z * cmToWorld(10.0);
    if (square(PSVECSquareDistance(&origin, &current)) >= shift) {
        entry->position.z += shift;
    }
    entry->position.y += entry->velocity.y * cmToWorld(20.0);
    if (entry->position.y <= body.y + cmToWorld(120.0) && entry->velocity.y <= 0.0f) {
        entry->type = 3;
        body.y += 120.0;
        entry->position.x = body.x;
        entry->position.y = body.y;
        entry->position.z = body.z;
        soundPlay(MSM_SE_MG677_STOLEN_COIN_ATTACH);
        soundPlay(MSM_SE_CMN_COIN_BOUNCE);
    }
}

// Builds the runner's path around the destination for the current character state.
void CoinsStolenEvent::queueRunnerApproach(RouteRunner &runner, int index)
{
    PlayerWork &owner = PlayerWork::getChecked();
    Vec &selected = runnerDestinations[owner.selectedRope()];
    Vec destination;
    destination.x = selected.x;
    destination.y = selected.y;
    destination.z = selected.z;
    if (owner.selectedRope() == 3) {
        runner.position.x -= 150.0;
        runner.position.z -= 150.0;
    }
    float radius;
    Vec waypoint;
    if (index == 5) {
        if (owner.selectedRope() <= 1) {
            radius = 150.0f;
        } else {
            radius = 100.0f;
        }
        waypoint.x = destination.x;
        waypoint.y = destination.y;
        waypoint.z = destination.z;
        waypoint.x += radius * std::cos(M_PI * runners[0].waypointAngle / 180.0);
        waypoint.z += radius * std::sin(M_PI * runners[0].waypointAngle / 180.0);
        runner.queueWaypoint(waypoint, runner.speed);
    }
    if (index == 4) {
        if (owner.selectedRope() >= 2) {
            radius = 150.0f;
        } else {
            radius = 100.0f;
        }
        waypoint.x = destination.x;
        waypoint.y = destination.y;
        waypoint.z = destination.z;
        waypoint.x += radius * std::cos(M_PI * runners[index - 1].waypointAngle / 180.0);
        waypoint.z += radius * std::sin(M_PI * runners[index - 1].waypointAngle / 180.0);
        runner.queueWaypoint(waypoint, runner.speed);
    }
    destination.x += 150.0 * std::cos(M_PI * runner.waypointAngle / 180.0);
    destination.z += 150.0 * std::sin(M_PI * runner.waypointAngle / 180.0);
    runner.queueWaypoint(destination, runner.speed);
    runner.setState(&RouteRunner::turnTowardPlayer);
}

// Queues the final destination and any intervening waypoints.
void CoinsStolenEvent::queueRunnerDestination(RouteRunner &runner, int index)
{
    PlayerWork &owner = PlayerWork::getChecked();
    Vec destination = runnerDestinations[4];
    Vec waypoint;

    if (index == 5) {
        waypoint = runners[0].getPosition();
        runner.queueWaypoint(waypoint, runner.speed);
    }
    if (index == 4) {
        waypoint = runners[index - 1].getPosition();
        runner.queueWaypoint(waypoint, runner.speed);
    }
    if (owner.selectedRope() == 3) {
        Vec intermediate = {0.0f, 0.0f, 0.0f};
        intermediate.x = -150.0f;
        intermediate.z = 1050.0f;
        intermediate.x += 150.0 * std::cos(M_PI * runner.waypointAngle / 180.0);
        intermediate.z += 150.0 * std::sin(M_PI * runner.waypointAngle / 180.0);
        runner.queueWaypoint(intermediate, runner.speed);
    }
    destination.x += 150.0 * std::cos(M_PI * runner.waypointAngle / 180.0);
    destination.z += 150.0 * std::sin(M_PI * runner.waypointAngle / 180.0);
    runner.queueWaypoint(destination, runner.speed);
}
