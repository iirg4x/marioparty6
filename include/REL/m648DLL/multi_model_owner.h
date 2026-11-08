#ifndef REL_M648_MULTI_MODEL_OWNER_H
#define REL_M648_MULTI_MODEL_OWNER_H

#include "REL/framework/model.h"
#include "REL/m648DLL/framework/linknode.h"

class MotionHookModels;
typedef void (MotionHookModels::*CallbackSetup5548)();

class MotionHookModels : public Node {
public:
    void fn_1_4D7C();
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
    OwnedModel models[6];
    CallbackSetup5548 callback;
    u8 complete;
    u8 shouldRestart;
    s32 mode;
    s32 sequence;
    Vec position;
    s32 callbackCount;
    s32 attributeState;

    s16 selectMotion(s32 selection) const
    {
        s16 modelId = models[0];
        switch (selection) {
        case 0: modelId = models[0]; break;
        case 1: modelId = models[1]; break;
        case 2: modelId = models[2]; break;
        case 3: modelId = models[3]; break;
        case 4: modelId = models[4]; break;
        case 5: modelId = models[5]; break;
        }
        return modelId;
    }
};

MotionHookModels *fn_1_453C(MotionHookModels *, s32, s32);

#endif
