#ifndef REL_M671_MATH_CALLBACK_H
#define REL_M671_MATH_CALLBACK_H
#include "REL/m671DLL/animation_owner.h"
#include <functional>
typedef std::binder1st<std::mem_fun1_t<void, AnimationOwner, EventMessage *> > CallbackPayload;
#endif
