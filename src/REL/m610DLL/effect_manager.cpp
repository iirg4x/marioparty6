#include <algorithm>
#include <functional>
#include <new>
#include "REL/m610DLL/segment_spawner.h"
#include "REL/m610DLL/moving_model_owner.h"
#include "dolphin/mtx.h"

extern "C"
{
#include "dolphin/types.h"
}

#include "REL/m610DLL/effect_manager.h"

#include "REL/m610DLL/model_group.h"

#include "REL/framework/p_chara.h"

#include "REL/m610DLL/capture_effect.h"

extern "C"
{
#include "game/hu3d.h"
}

#include "REL/m610DLL/playing_piece.h"

#include "REL/framework/hu_archive.h"

#include "REL/framework/model.h"

#include "REL/framework/singleton.h"

#include "datadir_enum.h"

extern "C"
{
#include "game/gamework.h"
}

extern "C"
{
#include "game/sprite.h"
}

#include "REL/m610DLL/torch_effect.h"

#include "REL/m610DLL/particle_emitter.h"

#include "REL/m610DLL/effect_functions.h"

#include "REL/framework/tri_colli.h"

#include "REL/m610DLL/scene_effect.h"

#include "REL/m610DLL/particle_game_object.hpp"

#include <cmath>

#include <cstdio>

#include <vector>
#include <memory>

/* Read and lock a sprite animation resource. */
ANIMDATA *fn_1_DDB4(unsigned long dataNum)
{
    ANIMDATA *anim = HuSprAnimDataRead(dataNum);
    HuSprAnimLock(anim);
    return anim;
}

void fn_1_F390(std::vector<OwnedModel *> *handles);

/* Model files selected by the day/night mode. */
static const int lbl_1_rodata_518[2] = { DATANUM(0x450000, 8), DATANUM(0x450000, 9) };

/* Load the scene resources, set the background and register effect events. */
EffectManager::EffectManager() : collisionTree(new Octree(new HuArchive(DATANUM(0x450000, 6))))
{
    animations.push_back(fn_1_DDB4(DATANUM(0x450000, 15)));
    animations.push_back(fn_1_DDB4(DATANUM(0x450000, 16)));
    animations.push_back(fn_1_DDB4(DATANUM(0x450000, 17)));
    animations.push_back(fn_1_DDB4(DATANUM(0x450000, 18)));
    animations.push_back(fn_1_DDB4(DATANUM(0x450000, 19)));
    animations.push_back(fn_1_DDB4(DATANUM(0x450000, 20)));
    animations.push_back(fn_1_DDB4(DATANUM(0x450000, 21)));
    models.push_back(new OwnedModel(lbl_1_rodata_518[GWMgNightFGet()]));
    models.push_back(new OwnedModel(DATANUM(0x450000, 7)));
    int modelCount = models.size();
    for (int modelIndex = 0; modelIndex < modelCount; ++modelIndex)
        models[modelIndex]->setAttribute(0x1);
    if (GWMgNightFGet() == 0)
        Hu3DBGColorSet(210, 200, 220);
    else
        Hu3DBGColorSet(100, 90, 160);
    registerChannel(1);
    fn_1_1E54();
}

/* Release the loaded sprite animations and models. */
EffectManager::~EffectManager()
{
    std::for_each(animations.begin(), animations.end(), std::ptr_fun(HuSprAnimKill));
    fn_1_F390(&models);
}

/* Set a position on the X axis. */
static inline Vec &alongXAxis(Vec &position, float value)
{
    position.x = value;
    position.y = 0.0f;
    position.z = 0.0f;
    return position;
}

/* Create and position one player playing piece. */
PlayerPiece *EffectManager::fn_1_E9B8(int playerIndex)
{
    int resourceNumber = DATANUM(0x450000, 22);
    s16 nightMode = lbl_802C0278;
    if (nightMode != 0)
        resourceNumber += 4;
    OwnedModel *animationResource = new OwnedModel(resourceNumber + playerIndex);
    PlayerPiece *piece = new PlayerPiece(
        animationResource, new OwnedModel(DATANUM(0x450000, 30)),
        new TriangleCollision(collisionTree.get()), new PlayerInfo(playerIndex), fn_1_EDCC());
    lbl_1_bss_54[playerIndex] = piece;
    animationResource->setShadow();
    if (GWMgNightFGet() != 0)
        animationResource->resetObjectShadow("corn01");

    float positionValue = (float) fn_1_F370(playerIndex, 4, 800);
    Vec position;
    piece->fn_1_1FAC(&alongXAxis(position, positionValue));
    return piece;
}

/* Position of the winning player in the result presentation. */
const Vec lbl_1_rodata_530 = { 0.0f, 0.0f, -4000.0f };

/* Create the winning player model at the result position. */
void EffectManager::fn_1_EBAC(int playerIndex)
{
    ResultModelOwner *result;
    int dataNumber = DATANUM(0x450000, 22);
    s16 nightMode = lbl_802C0278;
    if (nightMode != 0)
        dataNumber += 4;

    OwnedModel *modelResource = new OwnedModel(dataNumber + playerIndex);

    result = new ResultModelOwner(modelResource, new OwnedModel(DATANUM(0x450000, 30)),
                                  new PlayerConfigSelection(playerIndex, 1));

    modelResource->setShadow();
    s16 nightModeForName = lbl_802C0278;
    if (nightModeForName != 0)
        modelResource->resetObjectShadow("corn01");
    result->fn_1_C358(&lbl_1_rodata_530);
}

/* Create the torch particles at the named night scenery positions. */
void EffectManager::fn_1_ECE8()
{
    TorchEffect *torch;
    s16 nightMode = lbl_802C0278;
    char name[0x80];
    Vec position;
    if (nightMode != 0)
    {
        OwnedModel resource(DATANUM(0x450000, 12));
        for (int index = 0; index < 6; ++index)
        {
            std::sprintf(name, "torch%02d", index + 1);
            resource.getObjectPosition(name, &position);
            torch = new TorchEffect(*this->animations.begin());
            torch->Particle::setPosition(&position);
        }
    }
}

/* Create the particle emitter attached to a playing piece. */
ParticleEmitterState *EffectManager::fn_1_EDCC() const
{
    ParticleEmitterState *resource = new ParticleEmitterState(this->animations[1]);
    return resource;
}

/* Load and configure the day or night scenery models. */
void EffectManager::fn_1_EE3C()
{
    s16 nightMode = lbl_802C0278;
    int nameVariant;
    int nameBase;
    if (nightMode == 0)
        nameVariant = 0;
    else
        nameVariant = 3;
    nameBase = nameVariant;

    OwnedModel *firstName = new OwnedModel(DATANUM(0x450000, 0) + nameBase);
    OwnedModel *secondName = new OwnedModel(DATANUM(0x450000, 1) + nameBase);
    OwnedModel *thirdName = new OwnedModel(DATANUM(0x450000, 2) + nameBase);
    firstName->setLightsDynamic(0);
    firstName->setShadowMap();
    firstName->setLayer(2);
    secondName->setLayer(2);
    thirdName->setLayer(0);
    this->models.push_back(firstName);
    this->models.push_back(secondName);
    this->models.push_back(thirdName);
}

/* Create the screen capture effect with the mode-specific texture. */
CaptureEffect *EffectManager::fn_1_EFC4()
{
    CaptureEffect *result;
    s16 nightMode = lbl_802C0278;
    if (nightMode == 0)
        result = new CaptureEffect(this->animations[2], this->animations[3]);
    else
        result = new CaptureEffect(this->animations[2], this->animations[6]);
    return result;
}

/* Create animated scenery, falling obstacles and the collision segment model. */
void EffectManager::fn_1_F0BC()
{
    OwnedModel resource(DATANUM(0x450000, 11));
    char effectName[0x80];
    char modelName[0x80];
    Vec effectPosition;
    Vec modelPosition;

    for (int effectIndex = 0; effectIndex < 6; ++effectIndex)
    {
        std::sprintf(effectName, "battan%d", effectIndex + 1);
        resource.getObjectPosition(effectName, &effectPosition);
        SceneEffect *effectObject = new SceneEffect(*this->models.begin());
        effectObject->position.x = effectPosition.x;
        effectObject->position.y = effectPosition.y;
        effectObject->position.z = effectPosition.z;
    }

    for (int modelIndex = 0; modelIndex < 4; ++modelIndex)
    {
        std::sprintf(modelName, "dossun%d", modelIndex + 1);
        resource.getObjectPosition(modelName, &modelPosition);
        new MovingModelOwner(this->models[1], this->animations[5], &modelPosition);
    }

    s16 nightMode = lbl_802C0278;
    int resourceVariant = nightMode == 0 ? 0 : 1;
    OwnedModel *animationResource = new OwnedModel(resourceVariant + DATANUM(0x450000, 13));
    new SegmentSpawner(animationResource);
}

/* Requests zero and one create different particle objects at the supplied position. */
void EffectManager::receive(const EventPacket *request)
{
    Vec *firstPosition;
    Vec *secondPosition;
    switch (request->type)
    {
    case 0:
    {
        firstPosition = static_cast<Vec *>(request->context);
        new ParticleCloudObject(animations[1], firstPosition);
        return;
    }
    case 1:
    {
        secondPosition = static_cast<Vec *>(request->context);
        new ParticleGameObject(animations[1], secondPosition);
    }
    }
}

/* Return the centered position of an equally spaced item. */
s32 fn_1_F370(s32 index, s32 count, s32 spacing)
{
    --count;
    return spacing * index - spacing * count / 2;
}

/* Delete every owned model and clear the pointer vector. */
void fn_1_F390(std::vector<OwnedModel *> *handles)
{
    std::vector<OwnedModel *>::iterator first = handles->begin();
    std::vector<OwnedModel *>::iterator last = handles->end();
    while (first != last)
        delete *first++;
    handles->clear();
}
