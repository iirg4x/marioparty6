#ifndef REL_M603DLL_DRAW_OWNER_H
#define REL_M603DLL_DRAW_OWNER_H

#include "dolphin/types.h"
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"

class FadeWork : public ListNode<FadeWork> {
public:
    s32 progress;
    s32 context;
    s16 modelId;
    u8 active;
    FadeWork(s32 context);
    ~FadeWork();
    void fn_1_3B1C(void *context);
    void fn_1_3B8C(void *context);
    void fn_1_43A0(void *context);
};

class DrawListOwner : public RequestObject,
                              public MinigameSingleton<DrawListOwner> {
public:
 DrawListOwner();
 virtual ~DrawListOwner();
 virtual void flagEnabled(int);
 virtual void update();
 void fn_1_3714();
};

#endif
