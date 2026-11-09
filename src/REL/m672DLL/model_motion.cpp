/* Model motion resources and the ordered collection of their joint-motion IDs. */
extern "C" {
#include <math.h>
#include "game/hu3d.h"
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <functional>
#include <vector>
#include "REL/m672DLL/model_motion_bf64.h"

extern "C" {
void fn_1_1FC18(ModelMotion *sequence, unsigned long resourceId);
void fn_1_1FCB4(ModelMotion *sequence, const unsigned long *resourceIds, unsigned long count);
}

ModelMotion::ModelMotion(unsigned int dataNum)
    : OwnedModel(dataNum), currentDataNum(-1) {}

ModelMotion::~ModelMotion() {}

void fn_1_1FC18(ModelMotion *sequence, unsigned long resourceId)
{
    void *heapMotion;
    short jointMotion;
    unsigned long recordResourceId;
    int jointIndex;
    short motionId;
    MotionEntry value;
    heapMotion = fn_80008DD0(resourceId, 0x10000000, 2);
    motionId = (int)(short)sequence->modelId;
    jointMotion = fn_80030458(motionId, heapMotion);
    jointIndex = (short)jointMotion;
    recordResourceId = resourceId;
    value.dataNum = recordResourceId;
    value.motionId = jointIndex;
    sequence->motions.push_back(value);
}

void fn_1_1FCB4(ModelMotion *sequence, const unsigned long *resourceIds, unsigned long count)
{
    const unsigned long *current;
    const unsigned long *limit;
    unsigned long resourceId;
    short jointMotion;
    void *heapMotion;
    unsigned long recordResourceId;
    int jointIndex;
    short motionId;
    MotionEntry value;
    limit = resourceIds + count;
    current = resourceIds;
    for (; current != limit; ++current) {
        resourceId = *current;
        heapMotion = fn_80008DD0(resourceId, 0x10000000, 2);
        motionId = (int)(short)sequence->modelId;
        jointMotion = fn_80030458(motionId, heapMotion);
        jointIndex = (short)jointMotion;
        recordResourceId = resourceId;
        value.dataNum = recordResourceId;
        value.motionId = jointIndex;
        sequence->motions.push_back(value);
    }
}

bool fn_1_1FF60(MotionEntry &, int);
extern "C" {
void __msl_assertion_failed(const char *, const char *, int);
}

static inline void AddMotion1FF74(ModelMotion *owner, unsigned long resourceId)
{
    void *heapMotion;
    short jointMotion;

    heapMotion = fn_80008DD0(resourceId, 0x10000000, 2);
    jointMotion = fn_80030458(owner->getId(), heapMotion);
    MotionEntry value(resourceId, (short)jointMotion);
    owner->motions.push_back(value);
}


void ModelMotion::fn_1_1FD78(int dataNum)
{
    fn_8002C95C(getId(), fn_1_1FF74(dataNum));
    currentDataNum = dataNum;
}

void ModelMotion::fn_1_1FDD8(int dataNum, float blendTime, float start, unsigned long attr)
{
    if (dataNum != currentDataNum || fn_8002D75C(getId())) {
        fn_1_33950(((1 << 30) | (1 << 1)));
        fn_8002CCC0(getId(), fn_1_1FF74(dataNum), start, blendTime, attr);
        currentDataNum = dataNum;
    }
}

void ModelMotion::fn_1_1FE8C(int animationId)
{
    fn_8002CAA8(*this, fn_1_1FF74(animationId));
}

void ModelMotion::fn_1_1FEE0(unsigned int dataNum, unsigned short attr)
{
    fn_8002DC74(fn_1_1FF74(dataNum), attr);
}

unsigned int ModelMotion::fn_1_1FF1C() const
{
    return (unsigned int)fn_8002D3EC(*this) >> 31U;
}

bool fn_1_1FF60(MotionEntry &entry, int number)
{
    return entry.dataNum == number;
}

short ModelMotion::fn_1_1FF74(int dataNumber)
{
    MotionEntry *it;
    MotionEntry *end;

    it = this->motions.begin();
    end = this->motions.end();
    it = std::find_if(it, end, std::bind2nd(std::ptr_fun(fn_1_1FF60), dataNumber));
    (it != end)
        ? (void)0
        : __msl_assertion_failed("it != end &&\"Requested motion is not registered.\"",
                                 "model_motion.cpp", 0x5E);
    if (it == end) {
        AddMotion1FF74(this, dataNumber);
        return this->motions.back().motionId;
    }
    return (short)it->motionId;
}
