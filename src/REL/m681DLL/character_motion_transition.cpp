#include "REL/m681DLL/character_motion.h"

float MotionOwner::fn_1_DB20()
{
    return getMotionOwnerTransitionTime(&charNo);
}

float MotionOwner::fn_1_DB48()
{
    return getMotionOwnerMaximumTime(&charNo);
}
