#ifndef REL_M681DLL_MODEL_MOTION_H
#define REL_M681DLL_MODEL_MOTION_H

#include <vector>
extern "C" {
#include "game/charman.h"
}

/* Needs OwnedModel: from REL/framework/model.h, or from owned_model.h for a file on the module's Model. */
class ModelMotionEntry {
public:
    int dataNum;
    short motionId;
    ModelMotionEntry() {}
    ModelMotionEntry(const long &number, const short &motion)
        : dataNum(number), motionId(motion) {}
};

class ModelMotion : public OwnedModel {
public:
    std::vector<ModelMotionEntry> motions;
    int currentDataNum;
    short id() const { return modelId; }
    explicit ModelMotion(unsigned int);
    ~ModelMotion();
    void fn_1_1A058(unsigned long resourceId);
    void fn_1_1A0F4(const unsigned long *resourceIds, unsigned long count);
    void fn_1_1A1B8(int dataNum);
    void fn_1_1A218(int dataNum, float blendTime, float start, unsigned long attr);
    void fn_1_1A2CC(int animationId);
    void fn_1_1A320(unsigned int dataNum, unsigned short attr);
    unsigned int fn_1_1A35C() const;
    short fn_1_1A3B4(int dataNumber);
};

#endif
