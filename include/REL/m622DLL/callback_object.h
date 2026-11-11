// One contestant: its character model, motions and the state callback run each frame.
#ifndef M622DLL_CALLBACK_OBJECT_H
#define M622DLL_CALLBACK_OBJECT_H

#include <vector>
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/model.h"
#include "REL/framework/linknode.h"

#include "REL/m622DLL/scene_models.h"

struct CallbackNode : Node {};

class CallbackObject : public CallbackNode {
public:
    enum { NUMBEROF_MOTION = 8 };
    typedef void (CallbackObject::*Callback)();

    Vec position;
    Callback callback;
    s32 count;
    s32 frameCount;
    u32 unknown28;
    s32 controller;
    s32 character;
    s32 padIndex;
    s32 state;
    s32 mode;
    f32 rotationY;
    OwnedModel model;
    u8 unknown46[6]; /* No code names this field. */
    s32 availabilityChecked;
    s32 pulsedVibrationFrames;
    s32 continuousVibrationFrames;
    s32 mgCoin;
    s32 eventStep;
    u8 motionEnded;
    u8 active;
    ModelEntry *requestList;
    std::vector<s16> m_mot_id;

    CallbackObject(s32 playerNo);
    ~CallbackObject();
    s16 fn_1_6F34(u32 dataNum);
    void fn_1_7058();
    void fn_1_7088();
    void fn_1_7520();
    void fn_1_770C();
    bool fn_1_8250();
    void fn_1_7200();
    void fn_1_7244();
    void fn_1_7274();
    void fn_1_72B8();
    void fn_1_733C();
    void fn_1_73A4();
    void fn_1_77F4();
    void fn_1_7960();
    void fn_1_7B78();
    void fn_1_7F60();
    void fn_1_7FEC();
};

extern "C" {
void fn_1_7460(CallbackObject *object, s32 motionIndex, u8 queued);
}

#endif
