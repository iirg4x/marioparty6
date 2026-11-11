#ifndef REL_M648_MULTI_MODEL_OWNER_H
#define REL_M648_MULTI_MODEL_OWNER_H

#include "REL/framework/model.h"
#include "REL/framework/list.h"

// A player's hook model and the six item models it can hold on its "itemhook_R" object.
class MotionHookModels : public ListNode<MotionHookModels> {
public:
    typedef void (MotionHookModels::*Callback)();
    MotionHookModels(s32 sequence, s32 mode);
    ~MotionHookModels();
    void fn_1_4D7C();
    s16 fn_1_4DEC(s32 selection) const;
    u8 fn_1_4B20(s32 sequenceType);
    void fn_1_52D8();
    void fn_1_5388();
    void fn_1_538C();
    void fn_1_543C();
    void fn_1_548C();
    void fn_1_5548();
    void fn_1_5800();
    void fn_1_58B4();
    OwnedModel hookModel;
    OwnedModel itemModel0;
    OwnedModel itemModel1;
    OwnedModel itemModel2;
    OwnedModel itemModel3;
    OwnedModel itemModel4;
    OwnedModel itemModel5;
    Callback callback;
    u8 complete;
    u8 shouldRestart;
    s32 mode;
    s32 sequence;
    Vec position;
    s32 callbackCount;
    s32 attributeState;
};

#endif
