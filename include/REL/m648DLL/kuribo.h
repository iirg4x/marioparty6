#ifndef REL_M648_KURIBO_H
#define REL_M648_KURIBO_H
#include "REL/framework/list.h"

/* One kuribo: walks by a callback chosen at random and follows object objectIndex of the manager's model. */
class Kuribo : public ListNode<Kuribo> {
public:
    typedef void (Kuribo::*Callback)();
    Callback callback;
    s32 currentMotion;
    OwnedModel model;
    s16 hookIndex;
    s32 objectIndex;
    Vec position;
    s32 updateCount;
    s32 timer;
    Kuribo(const OwnedModel &member, s32 objectIndex);
    ~Kuribo();
    void fn_1_67D4(s32 motionId, u8 queued);
    void fn_1_68D4();
    void fn_1_6D5C();
    void fn_1_71F8();
    void fn_1_76B0();
    void fn_1_7B4C();
};
#endif
