// Runs the Koopa-bomb event and animates its models.

#include "msm_se.h"
#include "datadir_enum.h"
#include "datanum/charmot.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include "REL/framework/vector_math.h"
#include "REL/m677DLL/koopa_bomb_event.h"
#include "REL/framework/sound.h"
#include "REL/framework/world_units.h"
#include "REL/m677DLL/seer_terror_camera.h"
#include "REL/m677DLL/host_character.h"
#include "REL/m677DLL/particle_animations.h"
#include "REL/m677DLL/player_work.h"

// Scale applied to the second model.
static const Vec secondScale = {3.0f, 3.0f, 3.0f};
// Character headings in rope selection order.
static const float rotations[] = {90.0f, 90.0f, 270.0f, 270.0f};
// Motion frames for the first model sound cues.
static const int soundFrames[] = {40, 120, 200, 280};
// Character motions loaded for the event.
static unsigned int motions[] = {
    CHARMOT_HSF_c000m1_324, CHARMOT_HSF_c000m1_323, CHARMOT_HSF_c000m1_333,
    CHARMOT_HSF_c000m1_357, CHARMOT_HSF_c000m1_377, CHARMOT_HSF_c000m1_399
};

// Starting position of the two models.
static Vec initialPosition = {0.0f, cmToWorld(2000.0f), cmToWorld(300.0f)};
// Destination for the character after the host model shifts its motion.
static Vec destination = {0.0f, 0.0f, cmToWorld(1000.0f)};

// Starts the sequence with the full list of sound cues.
KoopaBombEvent::KoopaBombEvent()
    : state(&KoopaBombEvent::startMotionAndRevealModel), countdown(0),
      pendingFrames(soundFrames, soundFrames + 4)
{
}

// The owning pointers release both scene models.
KoopaBombEvent::~KoopaBombEvent()
{
}

// Runs the current state and applies its position and scale to the display models.
int KoopaBombEvent::step()
{
    if (!state) {
        return 1;
    }
    (this->*state)();
    secondModel->setPosition(&position);
    firstModel->setPosition(&position);
    firstModel->setScale(&scale);
    playQueuedMotionCue();
    return 0;
}

// Plays the next queued cue once the first model reaches its motion frame.
void KoopaBombEvent::playQueuedMotionCue()
{
    if (!pendingFrames.empty()) {
        if (!(firstModel->getMotionTime() < *pendingFrames.begin())) {
            soundPlay(MSM_SE_MG677_KOOPA_BOMB_MOTION_CUE);
            pendingFrames.erase(pendingFrames.begin());
        }
    }
}

// Loads the two models and the character's motions for this sequence.
void KoopaBombEvent::begin()
{
    PlayerWork &owner = PlayerWork::getChecked();
    createCharacterMotions(&static_cast<MotionOwner &>(owner), motions, 6);
    position.x = initialPosition.x;
    position.y = initialPosition.y;
    position.z = initialPosition.z;
    vectorSet(&scale, 0.4f, 0.4f, 0.4f);
    firstModel.reset(new OwnedModel(DATANUM(_m677 << 16, 0x30)));
    secondModel.reset(new OwnedModel(DATANUM(_m677 << 16, 0x21)));
    firstModel->setAttribute(pauseMotion);
    firstModel->setAttribute(1);
    firstModel->hideObject("cube47");
    firstModel->setShadow();
    secondModel->setAttribute(pauseMotion);
    secondModel->setAttribute(1);
    firstModel->setMotionSpeed(1.5f);
    secondModel->setMotionSpeed(0.4f);
    firstModel->setLayer(2);
    secondModel->setLayer(5);
    secondModel->setScale(const_cast<Vec *>(&secondScale));
    verticalSpeed = 2.0f;
    unk30 = 0;
    verticalFrames = 0;
    countdown = 0;
}

// Starts the first character motion and enables the first display model.
void KoopaBombEvent::startMotionAndRevealModel()
{
    PlayerWork &owner = PlayerWork::getChecked();
    shiftCharacterMotion(&static_cast<MotionOwner &>(owner), CHARMOT_HSF_c000m1_323,
        16.0f, 0.0f, loopMotion);
    firstModel->resetAttribute(1);
    setState(&KoopaBombEvent::lowerModels);
}

// Brings the models down to the floor and starts the landing effects.
void KoopaBombEvent::lowerModels()
{
    float floor = 30.0f;
    position.y -= (1.0 / 60.0) * (verticalFrames * ((9.80665f / 2.0f) * verticalSpeed));
    ++verticalFrames;
    firstModel->setPosition(&position);
    if (!(position.y > floor)) {
        position.y = floor;
        soundPlay(MSM_SE_MG677_HEAVY_LANDING);
        ParticleAnimations &particles = ParticleAnimations::getChecked();
        particles.createGrayImpactParticles(&position, 32, 0.5f, 2.0f, 1);
        PlayerWork &owner = PlayerWork::getChecked();
        shiftCharacterMotion(&static_cast<MotionOwner &>(owner), CHARMOT_HSF_c000m1_324,
            16.0f, 0.0f, 0);
        fn_1_1CEA8(&owner, rotations[owner.selectedRope()], false);
        owner.rumble(0, 24);
        HostCharacter &model = HostCharacter::getChecked();
        model.fn_1_189CC();
        SeerTerrorCamera::getChecked().startShake(0.2f, 0.2f);
        setCountdown(secondsToFrames<double>(1.5));
        setState(&KoopaBombEvent::releaseMotionAfterLanding);
    }
}

// Releases the first model's motion after the landing delay.
void KoopaBombEvent::releaseMotionAfterLanding()
{
    if (::updateCountdown(countdown)) {
        firstModel->resetAttribute(pauseMotion);
        setState(&KoopaBombEvent::sendCharacterToDestination);
    }
}

// Shifts the host model's motion and sends the character to the destination.
void KoopaBombEvent::sendCharacterToDestination()
{
    HostCharacter &model = HostCharacter::getChecked();
    PlayerWork &owner = PlayerWork::getChecked();
    model.shiftMotion(DATANUM(_m677 << 16, 0x9), 16.0f, 0.0f, 0);
    enqueuePlayerWaypoint(&owner, &destination, 6.0f);
    setCountdown(secondsToFrames<double>(0.5));
    setState(&KoopaBombEvent::startSecondModel);
}

// Waits for the first model's motion and starts the second model with camera and rumble effects.
void KoopaBombEvent::startSecondModel()
{
    PlayerWork &owner = PlayerWork::getChecked();
    if (owner.isIdle()) {
        fn_1_1CF40(&owner, false);
        owner.setAttribute(pauseMotion);
    }
    if (firstModel->isMotionEnd() && ::updateCountdown(countdown)) {
        secondModel->resetAttribute(1);
        secondModel->resetAttribute(pauseMotion);
        owner.rumble(2, (long)secondsToFrames<double>(0.8));
        SeerTerrorCamera::getChecked().startShake(1.0f, 1.5f);
        soundPlay(MSM_SE_MG677_KOOPA_BOMB_DETONATE);
        setCountdown(secondsToFrames<double>(0.2));
        setState(&KoopaBombEvent::hideFirstModel);
    }
}

// Updates the host model and hides the first display model when the delay expires.
void KoopaBombEvent::hideFirstModel()
{
    if (::updateCountdown(countdown)) {
        HostCharacter &model = HostCharacter::getChecked();
        model.setMaterialColorBlend(0.8f);
        firstModel->setAttribute(1);
        setState(&KoopaBombEvent::startFinalMotionAndCamera);
    }
}

// Begins the final host motion and camera move when the second model finishes.
void KoopaBombEvent::startFinalMotionAndCamera()
{
    if (secondModel->isMotionEnd()) {
        HostCharacter &model = HostCharacter::getChecked();
        model.shiftMotion(DATANUM(_m677 << 16, 0x4), secondsToFrames<long>(1), 0.0f, pauseMotion);
        SeerTerrorCamera::getChecked().transitionToPreset(2, 3.0f);
        setCountdown(secondsToFrames<double>(0.4));
        setState(&KoopaBombEvent::waitForFinalMotionShift);
    }
}

// Waits for the host motion shift and counts down before ending the sequence.
void KoopaBombEvent::waitForFinalMotionShift()
{
    HostCharacter &model = HostCharacter::getChecked();
    while (!model.shiftDone()) {
        return;
    }
    model.spawnJawParticles();
    if (::updateCountdown(countdown)) {
        setState(0);
    }
}

// The dual-model sequence leaves this callback empty.
void KoopaBombEvent::fn_1_17104()
{
}
