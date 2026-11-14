#include <new>
#include "REL/m672DLL/hanging_cage.h"
#include <vector>
#include "REL/m672DLL/animated_owner.h"
#include "REL/framework/event_manager.h"
#include "REL/m672DLL/singleton_callback_owner.h"
extern "C" {
#include "dolphin/types.h"
#include "game/data.h"
#include "game/sprite.h"
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <functional>
#include "REL/m672DLL/resources.h"
#include "REL/m672DLL/scene_sequence.h"
#include "REL/m672DLL/scene_effect.h"
#include "REL/m672DLL/player.h"
#include "REL/m672DLL/spinner.h"

extern "C" {
ANIMDATA *fn_1_672C(u32 dataNumber);
}

// Loads and locks the animation identified by dataNumber.
ANIMDATA *fn_1_672C(u32 dataNumber)
{
    ANIMDATA *animation;

    animation = HuSprAnimRead(fn_80008DD0(dataNumber, 0x10000000, 2));
    HuSprAnimLock(animation);
    return animation;
}

// Loads four shared animations and registers resource event channels 0 and 2.
Resources::Resources()
{
    animations.push_back(fn_1_672C(DATANUM(0x830000, 0xE)));
    animations.push_back(fn_1_672C(DATANUM(0x830000, 0xF)));
    animations.push_back(fn_1_672C(DATANUM(0x830000, 0x10)));
    animations.push_back(fn_1_672C(DATANUM(0x830000, 0x11)));
    registerChannel(0);
    registerChannel(2);
}

// Releases shared animations and removes model entries through fn_1_749C.
Resources::~Resources()
{
    std::for_each(animations.begin(), animations.end(), std::ptr_fun(HuSprAnimKill));
    fn_1_749C(&models);
}

extern "C" char lbl_1_data_166[0xE];

const Vec lbl_1_rodata_1E0 = { 0.0f, 4500.0f, 2000.0f };

const Vec lbl_1_rodata_1EC = { 0.0f, 0.0f, -1400.0f };

// Creates shared light/model/shadow objects, configures lighting, and creates four cages.
void fn_1_6E04(Resources *object)
{
    OwnedModel *lightAnimation;
    OwnedModel *animation;
    OwnedModel *shadowAnimation;

    lightAnimation = new OwnedModel(DATANUM(0x830000, 0x1));
    animation = new OwnedModel(DATANUM(0x830000, 0x0));
    shadowAnimation = new OwnedModel(DATANUM(0x830000, 0x2));
    lightAnimation->setLightsDynamic(false);
    lightAnimation->setLayer(1);
    animation->setAttribute(1);
    shadowAnimation->setLayer(1);
    shadowAnimation->setShadowMap();
    object->lightId = fn_80029D24(*lightAnimation, lbl_1_data_166);
    object->models.push_back(lightAnimation);
    object->models.push_back(animation);
    object->models.push_back(shadowAnimation);
    Hu3DBGColorSet(0, 0, 0);
    Vec lightOrigin = lbl_1_rodata_1E0;
    Vec lightTarget = lbl_1_rodata_1EC;
    Point3d lightDirection;
    Point3d shadowPosition;
    Point3d shadowUp;
    short modelIds[1];
    int index;
    GXColor shadowColor = {255, 255, 255, 255};
    static const EffectVector lbl_1_rodata_1FC[4] = { {{ -700.0f, 2400.0f, -2600.0f }}, {{
        700.0f, 2400.0f, -2600.0f }}, {{ -800.0f, 2450.0f, -500.0f }}, {{ 800.0f, 2450.0f,
        -500.0f }} };
    PSVECSubtract(&lightTarget, &lightOrigin, &lightDirection);
    PSVECNormalize(&lightDirection, &lightDirection);
    fn_8002A3B0(20.0f, 1000.0f, 10000.0f, 1);
    PSVECScale(&lightDirection, &shadowPosition, -9000.0f);
    PSVECAdd(&shadowPosition, &lightTarget, &shadowPosition);
    shadowUp.x = 0.0f;
    shadowUp.y = 1.0f;
    shadowUp.z = 0.0f;
    fn_8002A674(&shadowPosition, &shadowUp, &lightTarget, 1);
    fn_8002AA3C(0, 0, 0x40, 1);
    fn_8002A7D8(0.3f, 1);
    modelIds[0] = *animation;
    fn_800804E8(modelIds, 1, 0x28);
    Point3d lightOffset;
    lightOffset.x = 0.0f;
    lightOffset.y = 0.0f;
    lightOffset.z = 0.0f;
    new SceneEffect(DATANUM(0x830000, 0xC), &lightOffset);
    for (index = 0; index < 4; ++index) {
        new HangingCage(DATANUM(0x830000, 0xA), &lbl_1_rodata_1FC[index]);
    }
}

char lbl_1_data_166[0xE] = "inf_light1";

extern "C" const EffectVector lbl_1_rodata_248[4] = {
    {{-450.0f, 0.0f, -300.0f}}, {{-150.0f, 0.0f, -300.0f}},
    {{150.0f, 0.0f, -300.0f}}, {{450.0f, 0.0f, -300.0f}}
};

// Creates one Player object at each of the four stored world positions.
void fn_1_71BC(void *state)
{
    int entry;
    for (entry = 0; entry < 4; ++entry) {
        new Player(entry, &lbl_1_rodata_248[entry]);
    }
}

// Creates the animated owner and five spinner objects from shared animations.
void fn_1_7230(Resources *manager)
{

    EffectVector initialPosition = {{850.0f, 558.0f, -1540.0f}};
    s32 index;

    new AnimatedOwner(DATANUM(0x830000, 0x12), &initialPosition, -45.0f);

    EffectVector entryPositions[5] = {
        {{-400.0f, 0.0f, -2000.0f}},
        {{400.0f, 0.0f, -2000.0f}},
        {{0.0f, 0.0f, -1350.0f}},
        {{-400.0f, 0.0f, -700.0f}},
        {{400.0f, 0.0f, -700.0f}}
    };

    static const s32 lbl_1_rodata_2C0[5] = { DATANUM(0x830000, 0x04), DATANUM(0x830000, 0x05),
        DATANUM(0x830000, 0x06), DATANUM(0x830000, 0x07), DATANUM(0x830000, 0x08) };

    for (index = 0; index < 5; ++index) {
        new Spinner(lbl_1_rodata_2C0[index], entryPositions[index],
                    manager->animations[0], 0xF, 4,
                    manager->animations[1], 4,
                    manager->animations[0], 0x50, 4, index);
    }
}

// Empty resource callback.
void fn_1_7404(void *) {}
// Empty resource callback.
void fn_1_7408(void *) {}
void fn_1_7484(void *, const EventPacket *);
void fn_1_7488(void *, const EventPacket *);

// Dispatches resource events on channel 0 or channel 2.
void Resources::receive(const EventPacket *event)
{
    switch (event->channel) {
    case 0:
        fn_1_7484(this, event);
        return;
    case 2:
        fn_1_7488(this, event);
    }
}

// Empty channel-0 event handler.
void fn_1_7484(void *, const EventPacket *) {}

// Both event-type switch arms are empty and produce no state changes.
void fn_1_7488(void *unused, const EventPacket *event)
{
    // Both event-type arms leave the resources unchanged.
    switch (event->type) {
    case 0:
        break;
    default:
        break;
    }
}
