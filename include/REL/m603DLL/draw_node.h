#ifndef REL_M603DLL_DRAW_NODE_H
#define REL_M603DLL_DRAW_NODE_H
#include "REL/framework/linknode.h"
#define M603_OWNED_MODEL_DECLARATION
#include "REL/m603DLL/model.h"
class DrawNode;
void fn_1_9B88(int request);
extern "C" int fn_1_9D34(int playerIndex, unsigned char markEntries);
struct DrawMotionNode : Node {
    typedef void (DrawNode::*Callback)();
    Callback callback;
    s32 currentMotion;
    ~DrawMotionNode();
};
struct DrawNode : DrawMotionNode {
    DrawNode(const OwnedModel *,u32,s32,s32);
    ~DrawNode();
    DrawNode *next_node() { return static_cast<DrawNode *>(link0); }
    void fn_1_9ED0();
    OwnedModel firstModel;
    OwnedModel secondModel;
    u8 motionVisible, motionPending;
    s32 remainingFrames, cameraIndex;
    Vec position;
    s32 duration, key;
    u8 motionRunning, active, scoringWindow, effectFlag;
    f32 motionFrames, motionMaximum, motionSpeed;
    s32 effectFrame;
    bool motionState;
    s32 resourceState;
    HU3D_MODELID hookModel;
    s32 colorIndex;
    u8 fired, effectGate;
    s16 model_a() const { return secondModel.modelId; }
    s16 model_b() const { return firstModel.modelId; }
    s16 primary_model() const { return firstModel.modelId; }
    s16 secondary_model() const { return secondModel.modelId; }
    s16 model_id() const { return secondModel.modelId; }
    inline void fn_1_A060(s32,u8);
    void fn_1_A160(s32);
    void fn_1_A238();
    void fn_1_A3A8();
    void fn_1_A524();
    void fn_1_A5D4();
    void fn_1_A5D8();
    void fn_1_A718(bool,f32);
    void fn_1_A858();
    void fn_1_AA34(Mtx);
    void fn_1_AAA4(Mtx);
    void fn_1_B0C0(Mtx);
};

#endif
