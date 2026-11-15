#ifndef REL_M634DLL_MODEL_ACTION_H
#define REL_M634DLL_MODEL_ACTION_H
#include <memory>
#include "REL/m634DLL/particle_object_base.h"
#include "REL/framework/model.h"
class ModelActionOwner : public ParticleObjectBase {
public:
    ModelActionOwner(u32 firstId, s32 secondId, s32 thirdId);
    virtual ~ModelActionOwner();
    virtual void update();
    std::auto_ptr<OwnedModel> firstAction, secondAction;
};
#endif
