#include "REL/m671DLL/light_layers.h"
/* The animations and models the whole minigame shares, loaded once at its start. */
#include <algorithm>
#include <functional>
#include <vector>
#include "dolphin/types.h"
extern "C" {
#include "game/data.h"
#include "game/sprite.h"
#include "game/mg/actman.h"
}

#include "REL/framework/game_object2.h"
#include "REL/m671DLL/resources.h"
#include "REL/m671DLL/player_object.h"
#include "REL/m671DLL/callback_receiver.h"
#include "REL/m671DLL/particle_game_object.h"

ANIMDATA *fn_1_6B8C(u32 dataNumber)
{
    ANIMDATA *animation;

    animation = HuSprAnimRead(fn_80008DD0(dataNumber, 0x10000000, 2));
    HuSprAnimLock(animation);
    return animation;
}

Resources::Resources()
{
    animations.push_back(fn_1_6B8C(DATANUM(0x820000, 9)));
    animations.push_back(fn_1_6B8C(DATANUM(0x820000, 10)));
    registerChannel(0);
    registerChannel(2);
}

Resources::~Resources()
{
    std::for_each(animations.begin(), animations.end(), std::ptr_fun(HuSprAnimKill));
    fn_1_773C(&models);
}

void Resources::fn_1_718C()
{
    OwnedModel *motionAnimation;
    OwnedModel *alternateAnimation;
    OwnedModel *thirdAnimation;
    s16 modelIds[1];

    new LightLayer0;
    new LightLayer4;
    motionAnimation = new OwnedModel(DATANUM(0x820000, 1));
    alternateAnimation = new OwnedModel(DATANUM(0x820000, 0));
    thirdAnimation = new OwnedModel(DATANUM(0x820000, 2));

    motionAnimation->setLightsDynamic(false);
    motionAnimation->setLayer(1);
    alternateAnimation->setAttribute(1);
    thirdAnimation->setLayer(1);
    motionAnimation->setShadowMap();
    thirdAnimation->setShadowMap();
    this->lightId = fn_80029D24(*motionAnimation, "tekkyu01-light1");

    this->models.push_back(motionAnimation);
    this->models.push_back(alternateAnimation);
    this->models.push_back(thirdAnimation);

    Hu3DBGColorSet(0, 0, 0);
    {
        Vec lightPosition = {0.0f, 100.0f, 10.0f};
        Vec targetPosition = {0.0f, 0.0f, 0.0f};
        Vec direction;
        Vec shadowPosition;

        PSVECSubtract(&targetPosition, &lightPosition, &direction);
        PSVECNormalize(&direction, &direction);
        fn_8002A3B0(80.0f, 100.0f, 2000.0f, 1);
        PSVECScale(&direction, &shadowPosition, -1100.0f);
        PSVECAdd(&shadowPosition, &targetPosition, &shadowPosition);
        {
            Vec up;

            up.x = 0.0f;
            up.y = 1.0f;
            up.z = 0.0f;
            fn_8002A674(&shadowPosition, &up, &targetPosition, 1);
        }
        fn_8002AA3C(0x0A, 0x0A, 0x40, 1);
        fn_8002A7D8(0.29f, 1);
    }

    modelIds[0] = *alternateAnimation;
    fn_800804E8(modelIds, 1, 0x28);
}

void Resources::fn_1_74B8()
{
    s32 index = 0;

    while (index < 4) {
        new Player(index);
        index++;
    }
}

const Vec lbl_1_rodata_24C = {0.0f, 400.0f, 0.0f};

void Resources::fn_1_751C()
{
    Vec position;

    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 0.0f;
    new CallbackReceiver(0x82000B, const_cast<Vec *>(&lbl_1_rodata_24C), &position);
}

Point3d lbl_1_data_298[4] = {
    {-530.0f, 250.0f, 230.0f},
    {-530.0f, 250.0f, -230.0f},
    {530.0f, 250.0f, -230.0f},
    {530.0f, 250.0f, 230.0f},
};

Point3d lbl_1_data_2C8[4] = {
    {0.0f, -67.5f, 0.0f},
    {0.0f, -112.5f, 0.0f},
    {0.0f, 112.5f, 0.0f},
    {0.0f, 67.5f, 0.0f},
};

void Resources::fn_1_75A4()
{
    int index;

    for (index = 0; index < 4; index++) {
        new TimedParticleEffect(this->animations[0],
                                &lbl_1_data_298[index], &lbl_1_data_2C8[index]);
    }
}

void Resources::fn_1_763C()
{
}

void Resources::receive(const EventPacket *event)
{
    switch (event->channel) {
    case 0:
        fn_1_76B8(event);
        return;
    case 2:
        createEffect(event);
    }
}

void Resources::fn_1_76B8(const EventPacket *event)
{
}

void Resources::createEffect(const EventPacket *event)
{
    Vec *position;
    switch (event->type) {
    case 0:
        position = static_cast<Vec *>(event->context);
        new ParticleGameObject(animations[0], position);
        break;
    }
}

void fn_1_773C(std::vector<OwnedModel *> *models)
{
    OwnedModel **first = models->begin();
    OwnedModel **last = models->end();
    while (first != last) {
        delete *first++;
    }
    models->clear();
}
