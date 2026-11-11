#ifndef REL_M603DLL_ZAKO_H
#define REL_M603DLL_ZAKO_H

#include <vector>
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
#include "REL/framework/model.h"
extern "C" {
#include "game/hu3d.h"
}

class DrawModelPool : public RequestObject, public MinigameSingleton<DrawModelPool> {
public:
    enum { NUMBEROF_PMOTION = 2 };
    std::vector<s16> m_mot_id;
    OwnedModel model;

    DrawModelPool();
    virtual ~DrawModelPool();
    virtual void flagEnabled(int bit);
    virtual void update();
    HU3D_MOTIONID fn_1_8EA0(unsigned int dataNum);
};

#endif
