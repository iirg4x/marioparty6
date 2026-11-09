#ifndef REL_M603DLL_MODEL_MOTION_H
#define REL_M603DLL_MODEL_MOTION_H

#include "REL/m603DLL/linknode.h"
#define M603_OWNED_MODEL_DECLARATION
#include "REL/m603DLL/model.h"
#undef M603_OWNED_MODEL_DECLARATION

class MotionModel;
void fn_1_A12C();
int fn_1_D748(int key);
void fn_1_DDA8(s32 resourceState, Vec *position, Vec *direction);
class ModelCallbackNode : public Node {
public:
    typedef void (MotionModel::*Callback)();
    Callback callback;
    ~ModelCallbackNode();
};
class MotionModel : public ModelCallbackNode, public OwnedModel {
public:
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
    HU3D_MODELID primary_model() const { return modelId; }
    HU3D_MODELID model_id() const { return modelId; }
    MotionModel *next_node() { return static_cast<MotionModel *>(link0); }
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
