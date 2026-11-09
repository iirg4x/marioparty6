#ifndef REL_M672DLL_MODEL_MOTION_BF64_H
#define REL_M672DLL_MODEL_MOTION_BF64_H
#include "REL/m672DLL/model_bf64.h"
class MotionEntry {
public:
    int dataNum;
    short motionId;
    MotionEntry() {}
    MotionEntry(const long &number, const short &motion)
        : dataNum(number), motionId(motion) {}
};
class ModelMotion : public OwnedModel {
public:
    std::vector<MotionEntry> motions;
    int currentDataNum;
    explicit ModelMotion(unsigned int);
    ~ModelMotion();
    short getId() { return (short)modelId; }
    void fn_1_1FD78(int dataNum);
    void fn_1_1FDD8(int dataNum, float blendTime, float start, unsigned long attr);
    void fn_1_1FE8C(int animationId);
    void fn_1_1FEE0(unsigned int dataNum, unsigned short attr);
    unsigned int fn_1_1FF1C() const;
    short fn_1_1FF74(int dataNumber);
};
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
