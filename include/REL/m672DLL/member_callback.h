#ifndef REL_M672DLL_MEMBER_CALLBACK_H
#define REL_M672DLL_MEMBER_CALLBACK_H
#include <functional>
#include "REL/framework/event_hm.h"

template<class Callable>
void registerEventCallback(EventHandlerManager *owner, const Callable &payload, int key);

#endif
