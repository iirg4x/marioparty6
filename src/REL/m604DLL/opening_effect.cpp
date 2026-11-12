// The effect shown at the start: a model and its particles that slide in and leave.
#include "REL/m604DLL/camera_effect_owner.h"
#include "REL/m604DLL/stage_effects.h"
#include "REL/m604DLL/camera_owner.h"

#include "REL/m604DLL/player.h"

void Player::fn_1_0() {}

void Player::fn_1_4() {}

extern "C" {
#include "game/gamework.h"
}

static const unsigned long sModels[2] = {0x3F002E, 0x3F0017};

CameraEffectOwner::CameraEffectOwner() : time(0.0f)
{
    model.reset(new OwnedModel(sModels[GWMgNightFGet()]));
    model->setCamera(1);
    model->setAttribute((1 << 30) | (1 << 1));
    model->resetAttribute(1);
    model->setLayer(3);
    model->setShadow();
    position.x = -400.0f;
    position.y = 100.0f;
    position.z = 0.0f;
    rotation.x = 0.0f;
    rotation.y = 0.0f;
    rotation.z = 0.0f;
    model->setPosition(&position);
    model->setRotation(&rotation);
    particle.reset(new ModelParticles(0));
}

CameraEffectOwner::~CameraEffectOwner() {}

Vec CameraEffectOwner::getPosition() const
{
    return position;
}

void CameraEffectOwner::flagEnabled(int bit)
{
    flags &= ~(1 << bit);
}

void CameraEffectOwner::update()
{
    if (time <= 1.0f) {
        position.x = -400.0f + 150.0f * time;
    } else if (time > 2.0f) {
        position.x += -3.3333333333333335;
        if (position.x < -500.0f) {
            delete this;
            return;
        }
    }
    model->setPosition(&position);
    model->setRotation(&rotation);
    time += 0.016666666666666666;
}
