#ifndef REL_M603DLL_DRAW_OWNER_H
#define REL_M603DLL_DRAW_OWNER_H

#include "REL/m603DLL/callback_owner.h"
#include "REL/framework/minigame_singleton.h"

class FadeNode : public Node {
public:
    s32 progress;
    s32 context;
    ~FadeNode();
};

class FadeWork : public FadeNode {
public:
    s16 modelId;
    u8 active;
    FadeWork(s32 context);
    ~FadeWork();
    void fn_1_3B1C(void *context);
    void fn_1_3B8C(void *context);
    void fn_1_43A0(void *context);
};

class DrawListOwner : public CallbackOwner,
                              public MinigameSingleton<DrawListOwner> {
public:
 DrawListOwner();
 virtual ~DrawListOwner();
 virtual void update(unsigned long);
 virtual void extra();
 void fn_1_3714();
};

#endif
