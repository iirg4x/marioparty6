#include "REL/framework/game_object_man.h"
#include <new>
#include "REL/m628DLL/act_player.h"
#include "REL/framework/random.h"

extern "C"
{
#include "game/sprite.h"
}

#include "REL/framework/model.h"

#include "REL/framework/event_packet.h"

#include "REL/framework/game_object.h"

#include "REL/m628DLL/animation_owner.h"
#include <vector>
#include <algorithm>
#include <functional>

#include "REL/m628DLL/scene_model.h"
#include "REL/m628DLL/light_setup.h"

extern "C"
{
#include "game/gamework.h"
}

#include "REL/m628DLL/player_actor.h"
#include "REL/m628DLL/effects.h"

// Loads and locks one sprite animation resource.
ANIMDATA *fn_1_FE08(u32 dataNum)
{
    ANIMDATA *anim = HuSprAnimDataRead(dataNum);
    HuSprAnimLock(anim);
    return anim;
}

// Loads the round models and sprite animations.
SceneAnimations::SceneAnimations()
{
    animations.push_back(fn_1_FE08(DATANUM(0x570000, 9)));
    animations.push_back(fn_1_FE08(DATANUM(0x570000, 10)));
    animations.push_back(fn_1_FE08(DATANUM(0x570000, 11)));
    animations.push_back(fn_1_FE08(DATANUM(0x570000, 12)));
    animations.push_back(fn_1_FE08(DATANUM(0x570000, 14)));
    animations.push_back(fn_1_FE08(DATANUM(0x570000, 15)));
    animations.push_back(fn_1_FE08(DATANUM(0x570000, 16)));
    animations.push_back(fn_1_FE08(DATANUM(0x570000, 17)));
    models.push_back(new OwnedModel(DATANUM(0x570000, 8)));
    s32 modelCount = models.size();
    s32 modelIndex = 0;
    while (modelIndex < modelCount)
    {
        models[modelIndex]->setAttribute(1 << 0);
        ++modelIndex;
    }
    registerChannel(1);
}

void fn_1_116AC(std::vector<OwnedModel *> *models);

// Releases the round model vectors.
SceneAnimations::~SceneAnimations()
{
    std::for_each(animations.begin(), animations.end(), std::ptr_fun(HuSprAnimKill));
    fn_1_116AC(&models);
}

const Vec elevatedLightTarget = {0.0f, 100.0f, 10.0f};

const Vec groundLightTarget = {0.0f, 0.0f, 0.0f};

// Creates the board models and configures their lighting.
void fn_1_10760(SceneAnimations *owner)
{
    Hu3DBGColorSet(0, 0, 0);

    OwnedModel *firstModelEntry = new OwnedModel(DATANUM(0x570000, 0));

    OwnedModel *secondModelEntry = new OwnedModel(DATANUM(0x570000, 1));

    OwnedModel *thirdModelEntry = new OwnedModel(DATANUM(0x570000, 2));

    OwnedModel *fourthModelEntry = new OwnedModel(DATANUM(0x570000, 3));
    thirdModelEntry->setLayer(7);
    fourthModelEntry->setLayer(2);
    fourthModelEntry->setShadowMap();
    firstModelEntry->setAttribute(1 << 0);
    secondModelEntry->setLightsDynamic(0);
    secondModelEntry->setLayer(2);
    new LightSetup((s16 *)secondModelEntry);
    owner->models.push_back(firstModelEntry);
    owner->models.push_back(secondModelEntry);
    owner->models.push_back(thirdModelEntry);
    owner->models.push_back(fourthModelEntry);
    new SceneModelObject(new OwnedModel(DATANUM(0x570000, 5)), 1);
    new SceneModelObject(new OwnedModel(DATANUM(0x570000, 6)), 0);
    Vec target = elevatedLightTarget;
    Vec ground = groundLightTarget;
    Vec direction;
    Vec position;
    PSVECSubtract(&ground, &target, &direction);
    PSVECNormalize(&direction, &direction);
    fn_8002A3B0(80.0f, 100.0f, 8000.0f, 1);
    PSVECScale(&direction, &position, -1200.0f);
    PSVECAdd(&position, &ground, &position);
    Vec up;
    up.x = 0.0f;
    up.y = 1.0f;
    up.z = 0.0f;
    fn_8002A674(&position, &up, &ground, 1);
    fn_8002AA3C(10, 10, 64, 1);
    fn_8002A8CC(192, 1);
    fn_8002A7D8(0.3f, 1);
    s16 boardModelId = *firstModelEntry;
    fn_800804E8(&boardModelId, 1, 0x28);
}

// Counts nonzero-group players through the given player index.
s32 fn_1_10B5C(s32 playerIndex)
{
    s32 groupCount;
    s32 index;
    groupCount = 0;
    if (lbl_80265728[playerIndex].grpNo == 0)
    {
        groupCount = 0;
    }
    else
    {
        index = 0;
        while (index <= playerIndex)
        {
            if (lbl_80265728[index].grpNo != 0)
            {
                groupCount += 1;
            }
            index += 1;
        }
    }
    return groupCount;
}

// Creates each player actor and attaches its lever models.
void fn_1_10BD8(SceneAnimations *animations)
{
    s32 playerNo;
    s32 groupCount;
    HsfObject_s *leftLever;
    HsfObject_s *rightLever;

    for (playerNo = 0; playerNo < 4; playerNo++)
    {
        groupCount = fn_1_10B5C(playerNo);
        if (groupCount == 0)
        {
            leftLever = animations->models[3]->getObject("leverl");
            rightLever = animations->models[3]->getObject("leverr");
            new PlayerActor(
                playerNo, groupCount, &leftLever->mesh.base.rot.x, &rightLever->mesh.base.rot.x);
        }
        else
        {
            new ActPlayer(playerNo, groupCount);
        }
    }
}

// Position supplied when creating a round object.
struct RoundPlacement : Vec
{
    // Assigns the three position coordinates.
    void setPosition(f64 a, f64 b, f64 c)
    {
        x = a;
        y = b;
        z = c;
    }
};

extern "C" const Vec lbl_1_rodata_2DC[4] = {{0.0f, (100.0f * 0.3f), -660.0f},
                                            {0.0f, (100.0f * 0.3f), 660.0f},
                                            {-660.0f, (100.0f * 0.3f), 0.0f},
                                            {660.0f, (100.0f * 0.3f), 0.0f}};

extern "C" const GXColor lbl_1_rodata_30C[2] = {{0x40, 0xF0, 0xF0, 0xFF}, {0x40, 0x40, 0xF0, 0xFF}};

extern "C" const GXColor lbl_1_rodata_314[2] = {{0x64, 0x64, 0xC8, 0xA0}, {0x64, 0xC8, 0xC8, 0xA0}};

extern "C" const GXColor lbl_1_rodata_31C[2] = {{0x8C, 0x8C, 0xC8, 0xA0}, {0x8C, 0xC8, 0xC8, 0xA0}};

// Creates and places the round visual effects.
void fn_1_10D34(SceneAnimations *owner)
{
    s32 spriteIndex;
    s32 itemIndex;
    s32 overlayIndex;
    s32 boundaryIndex;
    PointLineEffect *sprite;
    SpriteParticleEffect *item;
    RoundPlacement spritePosition;
    RoundPlacement spriteRotation;
    RoundPlacement itemPosition;
    RoundPlacement itemRotation;
    RoundPlacement overlayPosition;
    RoundPlacement overlayRotation;
    RoundPlacement messagePosition;

    for (spriteIndex = 0; spriteIndex < 10; spriteIndex++)
    {
        sprite = new PointLineEffect;
        spritePosition.x = 0.0f;
        spritePosition.y = (100.0f * 0.3f);
        spritePosition.z = 0.0f;
        spriteRotation.setPosition(0.0, (spriteIndex & 1) == 0 ? 0.0f : 270.0f, 0.0);
        fn_1_7660(sprite, &spritePosition);
        sprite->setRotation(&spriteRotation);
        sprite->setSpinRate(RandomFloat(20.0f, 45.0f));
        fn_1_76C0(sprite, &lbl_1_rodata_314[(u8)((spriteIndex & 1) == 0)]);
    }

    for (itemIndex = 0; itemIndex < 6; itemIndex++)
    {
        item = new SpriteParticleEffect(owner->animations[(itemIndex % 3) + 4]);
        itemPosition.x = 0.0f;
        itemPosition.y = (100.0f * 0.3f);
        itemPosition.z = 0.0f;
        itemRotation.setPosition(0.0, (itemIndex & 1) == 0 ? 0.0f : 270.0f, 0.0);
        item->fn_1_87F0(&itemPosition);
        fn_1_880C(item, &itemRotation);
        item->fn_1_8828(&lbl_1_rodata_31C[(u8)((itemIndex & 1) == 0)]);
    }

    for (overlayIndex = 0; overlayIndex < 2; overlayIndex++)
    {
        TexturedQuadEffect *quadEffect = new TexturedQuadEffect(owner->animations[3]);
        overlayPosition.x = 0.0f;
        overlayPosition.y = 0.0f;
        overlayPosition.z = 0.0f;
        overlayRotation.setPosition(0.0, (overlayIndex & 1) == 0 ? 0.0f : 90.0f, 0.0);
        quadEffect->position = overlayPosition;
        quadEffect->rotation = overlayRotation;
        GXColor color = lbl_1_rodata_314[(u8)((overlayIndex & 1) == 0)];
        color.a = 0x50;
        quadEffect->color = color;
    }

    for (boundaryIndex = 0; boundaryIndex < 4; boundaryIndex++)
    {
        DriftingParticleEffect *boundary = new DriftingParticleEffect(
            owner->animations[1], (lbl_1_rodata_2DC[boundaryIndex].x == 0.0f ? (u8)1 : (u8)0));
        boundary->fn_1_B988(&lbl_1_rodata_2DC[boundaryIndex]);
        fn_1_B9E8(boundary, &lbl_1_rodata_30C[boundaryIndex < 2 ? 0 : 1]);
    }

    new TextureController;

    TexturedModelEffect *modelEffect = new TexturedModelEffect(
        *owner->models[0], owner->animations[0], owner->animations[2], owner->animations[1]);
    messagePosition.x = 0.0f;
    messagePosition.y = (100.0f * 0.3f);
    messagePosition.z = 0.0f;
    fn_1_C420(modelEffect, &messagePosition);

    new ParticleOwner(owner->animations[1], 32);
    new ParticleOwner(owner->animations[7], 32);
    new FadingLineEffect;
}

// The objects of the two event categories add these handlers to their game object's virtuals.
struct EventObject : GameObject
{
    virtual void dispatchFirst(int value);
    virtual void dispatchSecond(int value);
};

// Forwards the event to the mode and effect object groups.
void SceneAnimations::receive(const EventPacket *event)
{
    std::vector<EventObject *> firstCollection;
    std::vector<EventObject *> secondCollection;
    s32 firstCount;
    s32 secondCount;
    EventObject *firstObject;
    EventObject *secondObject;
    s32 firstIndex;
    s32 secondIndex;

    collectGameObjects(0x50, (std::vector<GameObject *> *)&firstCollection);
    collectGameObjects(0x60, (std::vector<GameObject *> *)&secondCollection);
    firstCount = firstCollection.size();
    secondCount = secondCollection.size();
    for (firstIndex = 0; firstIndex < firstCount; firstIndex++)
    {
        firstObject = firstCollection[firstIndex];
        firstObject->dispatchFirst(event->type);
    }
    for (secondIndex = 0; secondIndex < secondCount; secondIndex++)
    {
        secondObject = secondCollection[secondIndex];
        secondObject->dispatchSecond(event->type);
    }
}

// Deletes each owned model and clears the vector.
void fn_1_116AC(std::vector<OwnedModel *> *models)
{
    OwnedModel **first = models->begin();
    OwnedModel **last = models->end();
    while (first != last)
    {
        delete *first++;
    }
    models->clear();
}
