#ifndef REL_M672DLL_MODEL_MOTION_BF64_H
#define REL_M672DLL_MODEL_MOTION_BF64_H
#include "REL/framework/model_motion.h"

class ModelMotionPointer {
public:
    ModelMotion *pointer;
    explicit ModelMotionPointer(ModelMotion *value) : pointer(value) {}
    ~ModelMotionPointer() { delete pointer; }
    void operator=(const ModelMotionPointer &value) { pointer = value.pointer; }
    void operator=(ModelMotion *value) { pointer = value; }
    operator ModelMotion *() const { return pointer; }
    ModelMotion *operator->() const { return pointer; }
};

#endif
