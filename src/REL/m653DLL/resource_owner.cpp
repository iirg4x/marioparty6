extern "C" {
#include "game/hu3d.h"
}
#include "REL/m653DLL/exhaust_smoke.h"
#include "REL/m653DLL/model_image.h"
#include "REL/m653DLL/moving_object.h"
/* The resource owner: the player table, the shared animations and the models it releases. */
#include "REL/m653DLL/native_resource.h"
#include "REL/m653DLL/car.h"
#include "REL/m653DLL/particle_effects.h"
#include "REL/framework/p_chara.h"
#include "datadir_enum.h"
#include "REL/framework/player_table_1vs1.h"

extern "C" {
#include "REL/m653DLL.h"
s32 lbl_1_bss_40[2];
extern Car *lbl_1_bss_5C[2];
}
void fn_1_FC4(void);
extern "C" {

void fn_1_74AC(std::vector<OwnedReleasedModel *> *models);
s32 fn_1_696C(s32 index);
void fn_1_73DC(void);
s32 fn_1_73E0(void);
void fn_1_73E8(void);

}

ANIMDATA *fn_1_6920(unsigned long resource)
{
    ANIMDATA *animation = HuSprAnimRead(fn_80008DD0(resource, 0x10000000, 2));
    HuSprAnimLock(animation);
    return animation;
}

s32 fn_1_696C(s32 index) { return lbl_1_bss_40[index]; }

ResourceOwner::ResourceOwner()
{
    int index;
    makePlayerTable1vs1(lbl_1_bss_40);
    animations.push_back(fn_1_6920(DATANUM(0x700000, 11)));
    int itemCount = cleanup.size();
    index = 0;
    while (index < itemCount) {
        cleanup[index]->setAttribute(1);
        ++index;
    }
    registerChannel(1);
    fn_1_FC4();
}

struct AnimationKiller {
    void (*function)(ANIMDATA *);
    AnimationKiller(void (*operation)(ANIMDATA *)) : function(operation) {}
    void operator()(ANIMDATA *animation) const { function(animation); }
};

template<class Iterator, class Function>
static inline Function forEachResource(Iterator first, Iterator last, Function operation)
{
    while (first != last) {
        operation(*first);
        ++first;
    }
    return operation;
}

ResourceOwner::~ResourceOwner()
{
    forEachResource(animations.begin(), animations.end(), AnimationKiller(HuSprAnimKill));
    fn_1_74AC(&cleanup);
}

Car *ResourceOwner::fn_1_7118(s32 resourceIndex)
{
    s32 dataNumberBase = DATANUM(0x700000, 13);
    OwnedReleasedModel *modelResource = new OwnedReleasedModel(dataNumberBase + resourceIndex);
    Car *model = new Car(modelResource, new OwnedReleasedModel(DATANUM(0x700000, 17)),
        0, new PlayerInfo(lbl_1_bss_40[resourceIndex]), fn_1_7370());
    lbl_1_bss_5C[resourceIndex] = model;
    modelResource->setShadow();
    return model;
}

void ResourceOwner::fn_1_7238(s32 resourceIndex)
{
    s32 dataNumberBase = DATANUM(0x700000, 13);
    OwnedReleasedModel *modelResource = new OwnedReleasedModel(dataNumberBase + resourceIndex);
    ModelImageObject *object;
    Vec position;

    object = new ModelImageObject(modelResource, new OwnedReleasedModel(DATANUM(0x700000, 17)),
                          new PlayerConfigSelection(lbl_1_bss_40[resourceIndex], 1));

    modelResource->setShadow();
    position.x = 0.0f;
    position.y = 100.0f;
    position.z = 243.14915f;
    fn_1_6594(object, &position);
}

void fn_1_736C() {}

ExhaustSmoke *ResourceOwner::fn_1_7370()
{
    ExhaustSmoke *resource = new ExhaustSmoke(animations[0]);
    return resource;
}

void fn_1_73DC(void) {}

s32 fn_1_73E0(void) { return 0; }

void fn_1_73E8(void)
{
    new MovingObject;
}

void ResourceOwner::receive(const EventPacket *context)
{
    switch (context->type) {
    case 0: {
        Vec *position = (Vec *)context->context;
        new RingBurst(animations.begin()[0], position);
        break;
    }
    }
}

void fn_1_74AC(std::vector<OwnedReleasedModel *> *models)
{
    OwnedReleasedModel **first = models->begin();
    OwnedReleasedModel **last = models->end();
    while (first != last) delete *first++;
    models->clear();
}
