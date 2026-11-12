#ifndef REL_M653DLL_CALLBACK_PAYLOAD_H
#define REL_M653DLL_CALLBACK_PAYLOAD_H
#include "REL/m653DLL/car.h"
#include <functional>
typedef std::mem_fun1_t<void,Car,EventMessage *> CarEventMethod;
typedef std::binder1st<CarEventMethod> CarEventBinding;
template<class Functor> void fn_1_1CA0(EventHandlerManager *, const Functor &, int);
#endif
