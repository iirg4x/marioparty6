#ifndef REL_M603DLL_ZAKO_H
#define REL_M603DLL_ZAKO_H

#include <vector>
#include "REL/m603DLL/callback_owner.h"
#include "REL/framework/minigame_singleton.h"
#define M603_OWNED_MODEL_DECLARATION
#include "REL/m603DLL/model.h"

class DrawModelPool : public CallbackOwner, public MinigameSingleton<DrawModelPool> {
public:
    enum { NUMBEROF_PMOTION = 2 };
    std::vector<s16> m_mot_id;
    OwnedModel model;

    DrawModelPool();
    virtual ~DrawModelPool();
    virtual void update(unsigned long bit);
    virtual void extra();
    HU3D_MOTIONID fn_1_8EA0(unsigned int dataNum);
};

#endif
