#ifndef REL_M603DLL_DRAW_NODE_H
#define REL_M603DLL_DRAW_NODE_H
#include "REL/framework/list.h"
#include "REL/framework/model.h"
extern "C" {
#include "game/hu3d.h"
}
struct DrawNode;
void fn_1_9B88(int request);
extern "C" int fn_1_9D34(int playerIndex, unsigned char markEntries);
/* Animate and score one camera-specific photography target. */
struct DrawNode : ListNode<DrawNode> {
    typedef void (DrawNode::*Callback)();
    Callback callback;
    s32 currentMotion;
    DrawNode(const OwnedModel *,u32,s32,s32);
    ~DrawNode();
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

void fn_1_98AC(void);
void fn_1_995C(void);
void fn_1_9794(void);
void fn_1_9C58(void);
void fn_1_9BF4(void);

void fn_1_A9EC(HU3D_MODEL *, Mtx *);
void fn_1_9E90(DrawNode *);

#endif
