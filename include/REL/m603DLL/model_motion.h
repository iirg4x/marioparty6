#ifndef REL_M603DLL_MODEL_MOTION_H
#define REL_M603DLL_MODEL_MOTION_H

#include "REL/framework/list.h"
#include "REL/framework/model.h"
extern "C" {
#include "game/hu3d.h"
}

class MotionModel;
void fn_1_A12C();
int fn_1_D748(int key);
void fn_1_DDA8(s32 resourceState, Vec *position, Vec *direction);
class MotionModel : public ListNode<MotionModel> {
public:
    typedef void (MotionModel::*Callback)();
    Callback callback;
    OwnedModel model;
    MotionModel(s32 resourceStateValue, s32 cameraIndex);
    ~MotionModel() {}
    short motionId;
    unsigned char visible;
    unsigned char positionReady;
    unsigned char secondaryState;
    Vec position;
    Vec direction;
    int resourceState;
    int cameraIndex;
    HU3D_MODELID primary_model() const { return model.modelId; }
    HU3D_MODELID model_id() const { return model.modelId; }
    static void fn_1_D6C8(MotionModel *self);
    void fn_1_D708();
    void fn_1_D7D4();
    void fn_1_D858();
    void fn_1_D8D8(Vec *, Vec *);
    void fn_1_D9CC();
    void fn_1_DA9C();
    void fn_1_DD9C();
    void fn_1_DDA0();
    void fn_1_DDA4();
};

#endif
