// Runs the Tarai falls event and animates the player's reaction.

#include "msm_se.h"
#include "datanum/charmot.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include "REL/framework/vector_math.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/random.h"
#include "REL/framework/sound.h"
#include "REL/framework/world_units.h"
#include "REL/m677DLL/tarai_falls_event.h"
#include "REL/m677DLL/particle_animations.h"
#include "REL/m677DLL/player_work.h"
#include "datadir_enum.h"
#include <memory>

#include "game/hu3d.h"

// Clearance subtracted when placing the model at the character.
static int taraiHeightOffset = cmToWorld(222.0f);

// Character motions loaded for the event.
static const unsigned int motions[] = { CHARMOT_HSF_c000m1_325, CHARMOT_HSF_c000m1_322 };

// Returns the character's height minus the model clearance.
float getTaraiContactHeight()
{
    PlayerWork &character = PlayerWork::getChecked();
    return character.height() - 20.0;
}

// Owns the event model and interpolates its height.
class TaraiFallsModel : public OwnedModel {
  public:
    // Loads the model and sets its starting attributes and rotation.
    TaraiFallsModel() : OwnedModel(DATANUM(_m677 << 16, 0x1A)), height(0.0f)
    {
        fn_8002C2AC(5);
        setLayer(5);
        setShadow();
        setAttribute(pauseMotion);
        setAttribute(1);
        setRotationY(RandomFloat(-40.0f, 40.0f));
    }

    // Height transition applied to the model.
    FrameInterpolator<float> height;
};

// Returns the Y coordinate of the model's named object.
static inline float objectHeight(Model *model, char *name)
{
    Vec position;
    model->getObjectPosition(name, &position);
    return position.y;
}

// Starts at the first motion state.
TaraiFallsEvent::TaraiFallsEvent() : state(&TaraiFallsEvent::startCharacterMotion), countdown(0)
{
}

// Destroys the callback and its owned model.
TaraiFallsEvent::~TaraiFallsEvent()
{
}

// Loads the character motions and the callback model.
void TaraiFallsEvent::begin()
{
    PlayerWork &character = PlayerWork::getChecked();
    createCharacterMotions(&static_cast<MotionOwner &>(character), motions, 2);
    model.reset(new TaraiFallsModel);
}

// Updates the callback and advances the model's height.
int TaraiFallsEvent::step()
{
    if (state) {
        model->updateMotion();
        (this->*state)();
        TaraiFallsModel *movingModel = model.get();
        Vec position;
        movingModel->getPosition(&position);
        position.y = movingModel->height.update();
        movingModel->setPosition(&position);
    }
    return !state;
}

// Starts the first character motion.
void TaraiFallsEvent::startCharacterMotion()
{
    startEventCharacterMotion();
    setState(&TaraiFallsEvent::revealModelAfterMotion);
}

// Positions and shows the model when the character motion ends.
void TaraiFallsEvent::revealModelAfterMotion()
{
    PlayerWork *character = &PlayerWork::getChecked();
    if (characterMotionEnded(&character->charNo)) {
        Vec &characterPosition = character->getPosition();
        float height = characterPosition.y + getTaraiContactHeight() - taraiHeightOffset;
        TaraiFallsModel *heightModel = model.get();
        heightModel->height.set(height, 0);
        Vec &position = character->getPosition();
        TaraiFallsModel *movingModel = model.get();
        movingModel->setPosition(&position);
        movingModel->resetAttribute(pauseMotion);
        movingModel->resetAttribute(1);
        setState(&TaraiFallsEvent::playImpactEffects);
    }
}

// Plays the impact effects when the model reaches the character.
void TaraiFallsEvent::playImpactEffects()
{
    float height = getTaraiContactHeight();
    if (!(objectHeight(model.get(), "nulltarai") > height)) {
        PlayerWork *character = &PlayerWork::getChecked();
        ParticleAnimations *particles = &ParticleAnimations::getChecked();
        Vec position = character->getPosition();
        position.y += height + cmToWorld(50.0);
        particles->createYellowBurstParticles(&position, 12, 0.2f, 3.0f);
        character->rumble(2, 24);
        soundPlay(MSM_SE_MG677_TARAI_IMPACT);
        setState(&TaraiFallsEvent::scaleCharacterUnderModel);
    }
}

// Scales the character with the model until the motion advances.
void TaraiFallsEvent::scaleCharacterUnderModel()
{
    PlayerWork *character = &PlayerWork::getChecked();
    Vec characterScale;
    vectorSet(&characterScale, 1.0f,
              objectHeight(model.get(), "nulltarai") / getTaraiContactHeight(), 1.0f);
    character->setScale(&characterScale);
    float time = model->getMotionTime();
    if (!(time < 34.0f)) {
        character->setUniformScale(1.0f);
        shiftCharacterMotion(&static_cast<MotionOwner &>(*character), CHARMOT_HSF_c000m1_322, 8.0f,
                             0.0f, loopMotion);
        setState(&TaraiFallsEvent::startHeightInterpolation);
    }
}

// Sets the model's height interpolation and countdown.
void TaraiFallsEvent::startHeightInterpolation()
{
    float time = model->getMotionTime();
    if (!(time < 42.0f)) {
        PlayerWork *character = &PlayerWork::getChecked();
        Vec &characterPosition = character->getPosition();
        float height = characterPosition.y;
        TaraiFallsModel *heightModel = model.get();
        heightModel->height.set(height, (int) (94.0f - time));
        countdown = (int) (94.0f - time);
        setState(&TaraiFallsEvent::playCueAndWaitForMotion);
    }
}

// Plays the countdown sound and ends the callback when the motion ends.
void TaraiFallsEvent::playCueAndWaitForMotion()
{
    if (::updateCountdown(countdown)) {
        soundPlay(MSM_SE_MG677_TARAI_MOTION_CUE);
    }
    if (model->isMotionEnd()) {
        setState(0);
    }
}
