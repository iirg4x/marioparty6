#ifndef REL_M648_KURIBO_MOTION_OWNER_H
#define REL_M648_KURIBO_MOTION_OWNER_H
#include <vector>

extern "C" {
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "game/hu3d.h"
}

#include "REL/framework/model.h"
#include "REL/framework/singleton.h"
#include "REL/framework/list_object.h"

class KuriboMotionOwner : public RequestObject, public MinigameSingleton<KuriboMotionOwner> {
public:
    std::vector<HU3D_MOTIONID> m_mot_id;
    OwnedModel model;
    OwnedModel motion;
    KuriboMotionOwner();
    virtual ~KuriboMotionOwner();
    virtual void flagEnabled(int request);
    virtual void update();
    HU3D_MOTIONID addMotion(unsigned int dataNum);
    void fn_1_61D4(Vec *position, s32 objectIndex);
    /* Returns the owned model ID. */
    HU3D_MODELID model_id() const { return model.modelId; }
};

#endif
