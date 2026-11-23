// Animates the falling Togezoh and the player's reaction.

#include "msm_se.h"
#include "datanum/charmot.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include "REL/framework/vector_math.h"
#include "REL/framework/sound.h"
#include "REL/framework/world_units.h"
#include "REL/m677DLL/togezoh_falls_event.h"
#include "REL/m677DLL/particle_animations.h"
#include "REL/m677DLL/player_work.h"
#include "datadir_enum.h"
#include <math.h>
#include <memory>

// Updates one falling model from its delayed start through landing.
class TogezohFallsModel {
public:
    TogezohFallsModel(const OwnedModel &source, int modelIndex);
    ~TogezohFallsModel();
    void placeAboveCharacter(Vec &initialPosition);
    void advanceFall(int frame);
private:
    std::auto_ptr<OwnedModel> model;
    int index;
    Vec position;
    Vec direction;
    Vec rotation;
    Vec scale;
    float verticalSpeed;
    float fallTime;
    float delay;
    float angle;
    // Phase of the delayed fall and landing.
    int state;
    void advancePitch();
};

// Starts the callback in its initial state.
TogezohFallsEvent::TogezohFallsEvent()
    : state(&TogezohFallsEvent::startMotionAndPrepareChildren), frame(0)
{
}

// Releases the callback's owned child models.
TogezohFallsEvent::~TogezohFallsEvent()
{
}

// Runs the active state and reports completion.
int TogezohFallsEvent::step()
{
    if (state) {
        (this->*state)();
    }
    return !state;
}

// Loads the character motions and creates the six child models.
void TogezohFallsEvent::begin()
{
    PlayerWork &character = PlayerWork::getChecked();
    static const unsigned int motions[] = {
        CHARMOT_HSF_c000m1_385, CHARMOT_HSF_c000m1_322, CHARMOT_HSF_c000m1_325
    };
    createCharacterMotions(&static_cast<MotionOwner &>(character), motions, 3);
    model.reset(new OwnedModel(DATANUM(_m677 << 16, 0x1C)));
    model->setAttribute(1);
    for (int index = 0; index < 6; ++index) {
        children[index].reset(new TogezohFallsModel(*model, index));
    }
}

// Starts the character animation and places all six models directly above the character.
void TogezohFallsEvent::startMotionAndPrepareChildren()
{
    PlayerWork &character = PlayerWork::getChecked();
    shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_325,
        16.0f, 0.0f,
                         0);
    Vec &position = character.getPosition();
    for (int index = 0; index < 6; ++index) {
        children[index]->placeAboveCharacter(position);
    }
    setState(&TogezohFallsEvent::updateChildren);
}

// Updates all six children and ends the callback after four seconds.
void TogezohFallsEvent::updateChildren()
{
    for (int index = 0; index < 6; ++index) {
        children[index]->advanceFall(frame);
    }
    if (secondsToFrames<double>(4.0) < (double)frame) {
        setState(0);
    }
    ++frame;
}

// Sets a child model's starting direction, rotation, and scale.
TogezohFallsModel::TogezohFallsModel(const OwnedModel &source, int modelIndex)
    : model(new OwnedModel(source)), index(modelIndex), verticalSpeed(0.0f), fallTime(0.0f),
      delay(0.2 * modelIndex), angle(60.0 * modelIndex), state(0)
{
    vectorSet(&position, 0.0f, 0.0f, 0.0f);
    vectorSet(&direction, 0.0f, -1.0f, 0.0f);
    vectorSet(&rotation, 180.0f, angle, 0.0f);
    vectorSet(&scale, 0.8f, 0.8f, 0.8f);
    model->setAttribute(1);
    model->setLayer(2);
    model->setRotation(&rotation);
    model->setScale(&scale);
    model->setShadow();
}

// Releases the child model.
TogezohFallsModel::~TogezohFallsModel()
{
}

// Places and reveals the child model above the supplied position.
void TogezohFallsModel::placeAboveCharacter(Vec &initialPosition)
{
    position = initialPosition;
    position.y += 1000.0f;
    model->setPosition(&position);
    model->resetAttribute(1);
}

// Advances a child model after its delay and handles its effect on the character.
void TogezohFallsModel::advanceFall(int frame)
{
    PlayerWork &character = PlayerWork::getChecked();
    ParticleAnimations &particles = ParticleAnimations::getChecked();
    switch (state) {
        case 0:
            if (secondsToFrames(delay) < frame) {
                state = 1;
            }
            break;
        case 1: {
            fallTime += 1.0 / 60.0;
            float elapsedSquared = fallTime * fallTime;
            verticalSpeed -= 4.903325f * elapsedSquared;
            position.y += verticalSpeed;
            if (position.y < character.height() + cmToWorld(50.0)) {
                particles.createYellowBurstParticles(&position, 4, 0.3f, 3.0f);
                fallTime = 0.0f;
                verticalSpeed = 1.0f;
                state = 2;
                float radians = M_PI * angle / 180.0;
                direction.x = 5.0 * sin(radians);
                direction.z = 5.0 * cos(radians);
                shiftCharacterMotion(&static_cast<MotionOwner &>(character),
                                     CHARMOT_HSF_c000m1_385, 4.0f, 0.0f, 0);
                character.rumble(2, (int)secondsToFrames(0.1));
                soundPlay(MSM_SE_MG677_TOGEZOH_CONTACT);
            }
            break;
        }
        case 2: {
            fallTime += 1.0 / 60.0;
            float elapsedSquared = fallTime * fallTime;
            verticalSpeed -= 4.903325f * elapsedSquared;
            position.x += direction.x;
            position.y += verticalSpeed;
            position.z += direction.z;
            direction.x *= 0.98;
            direction.z *= 0.98;
            advancePitch();
            if (position.y < 0.0) {
                position.y = 0.0f;
                state = 3;
                particles.createGrayImpactParticles(&position, 32, 0.3f, 2.0f, 1);
                if (index == 5) {
                    shiftCharacterMotion(&static_cast<MotionOwner &>(character),
                                         CHARMOT_HSF_c000m1_322, 16.0f, 0.0f, loopMotion);
                }
            }
            break;
        }
        case 3:
            // Keeps the final transform after landing.
            break;
    }
    model->setPosition(&position);
    model->setRotation(&rotation);
}

// Advances the model's pitch to at most one full turn.
void TogezohFallsModel::advancePitch()
{
    rotation.x += 6.0f;
    if (rotation.x > 360.0f) {
        rotation.x = 360.0f;
    }
}
