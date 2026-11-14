#ifndef REL_M655DLL_INTERPOLATION_H
#define REL_M655DLL_INTERPOLATION_H

#include "dolphin/types.h"

template <class Result, class Start, class End>
inline Result interpolateValue(Start start, End end, f64 weight)
{
    return (1.0 - weight) * start + weight * end;
}

#endif
