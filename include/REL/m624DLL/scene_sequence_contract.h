#ifndef M624_SCENE_SEQUENCE_PROVIDER_CONTRACT_H
#define M624_SCENE_SEQUENCE_PROVIDER_CONTRACT_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/singleton.h"
#include "REL/framework/list_object.h"
#include "REL/framework/model.h"
#include <vector>
#include "REL/framework/frame_interpolator.h"

#include "REL/m624DLL/sequence_manager.h"

// The map (man_map.cpp): its models, its two transitions and the sequence of attackers.
struct SceneOwner : RequestObject, MinigameSingleton<SceneOwner> {
    OwnedModel baseModel;
    OwnedModel secondModel;
    OwnedModel startModel;
    OwnedModel fourthModel;
    OwnedModel animatedModel;
    OwnedModel clusterModel;
    OwnedModel linkedModel;
    OwnedModel lastModel;
    FrameInterpolator<float> speedTransition;
    FrameInterpolator<float> positionTransition;
    u8 initialized;
    ActorSequence manager;

    SceneOwner();
    virtual ~SceneOwner();
    virtual void flagEnabled(int bit);
    virtual void update();
};

u8 fn_1_4FBC(SceneOwner *);
void fn_1_528C(SceneOwner *);
void fn_1_5344(SceneOwner *);
#endif
