/* Model motion resources and the ordered collection of their joint-motion IDs. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "REL/m681DLL/model_motion_base.h"
#include "REL/m681DLL/model_motion.h"
extern "C" {
#include "game/data.h"
#include "game/hu3d.h"
}

extern "C" {
}
bool fn_1_1A3A0(ModelMotionEntry *entry, int number);
extern "C" {
void __msl_assertion_failed(const char *, const char *, int);
}

ModelMotion::ModelMotion(unsigned int dataNum)
    : OwnedModel(dataNum), currentDataNum(-1) {}

ModelMotion::~ModelMotion() {}

void ModelMotion::fn_1_1A058(unsigned long resourceId)
{
    void *heapMotion;
    short jointMotion;
    unsigned long recordResourceId;
    int jointIndex;
    short motion;
    ModelMotionEntry value;
    heapMotion = fn_80008DD0(resourceId, 0x10000000, 2);
    motion = (int)(short)modelId;
    jointMotion = fn_80030458(motion, heapMotion);
    jointIndex = (short)jointMotion;
    recordResourceId = resourceId;
    value.dataNum = recordResourceId;
    value.motionId = jointIndex;
    motions.push_back(value);
}

void ModelMotion::fn_1_1A0F4(const unsigned long *resourceIds, unsigned long count)
{
    const unsigned long *current;
    const unsigned long *limit;
    unsigned long resourceId;
    short jointMotion;
    void *heapMotion;
    unsigned long recordResourceId;
    int jointIndex;
    short motion;
    ModelMotionEntry value;
    limit = resourceIds + count;
    current = resourceIds;
    for (; current != limit; ++current) {
        resourceId = *current;
        heapMotion = fn_80008DD0(resourceId, 0x10000000, 2);
        motion = (int)(short)this->modelId;
        jointMotion = fn_80030458(motion, heapMotion);
        jointIndex = (short)jointMotion;
        recordResourceId = resourceId;
        value.dataNum = recordResourceId;
        value.motionId = jointIndex;
        this->motions.push_back(value);
    }
}

class MotionComparator {
public:
    bool (*compare)(ModelMotionEntry *, int);
    MotionComparator(bool (*function)(ModelMotionEntry *, int))
        : compare(function) {}
    bool operator()(ModelMotionEntry *entry, int number) const {
        return compare(entry, number);
    }
};

class MotionPredicate {
public:
    MotionComparator comparator;
    int dataNumber;
    MotionPredicate(const MotionComparator &function, const int &number)
        : comparator(function), dataNumber(number) {}
    bool operator()(ModelMotionEntry *entry) const {
        return comparator(entry, dataNumber);
    }
};

static inline MotionPredicate BindMotion1FF74(
    const MotionComparator &function, const int &number)
{
    int requested = number;
    return MotionPredicate(function, requested);
}

static inline ModelMotionEntry *FindMotion1FF74(
    ModelMotionEntry *first, ModelMotionEntry *last, MotionPredicate predicate)
{
    while (first != last && !predicate(first)) {
        ++first;
    }
    return first;
}

static inline void AddMotion1FF74(ModelMotion *owner, unsigned long resourceId)
{
    void *heapMotion;
    short jointMotion;

    heapMotion = fn_80008DD0(resourceId, 0x10000000, 2);
    jointMotion = fn_80030458(owner->id(), heapMotion);
    ModelMotionEntry value(resourceId, (short)jointMotion);
    owner->motions.push_back(value);
}

void ModelMotion::fn_1_1A1B8(int dataNum)
{
    fn_8002C95C(id(), fn_1_1A3B4(dataNum));
    currentDataNum = dataNum;
}

void ModelMotion::fn_1_1A218(int dataNum, float blendTime, float start, unsigned long attr)
{
    if (dataNum != currentDataNum || fn_8002D75C(id())) {
        fn_1_33950(((1 << 30) | (1 << 1)));
        fn_8002CCC0(id(), fn_1_1A3B4(dataNum), start, blendTime, attr);
        currentDataNum = dataNum;
    }
}

void ModelMotion::fn_1_1A2CC(int animationId)
{
    fn_8002CAA8(*this, fn_1_1A3B4(animationId));
}

void ModelMotion::fn_1_1A320(unsigned int dataNum, unsigned short attr)
{
    fn_8002DC74(fn_1_1A3B4(dataNum), attr);
}

unsigned int ModelMotion::fn_1_1A35C() const
{
    return (unsigned int)fn_8002D3EC(*this) >> 31U;
}

bool fn_1_1A3A0(ModelMotionEntry *entry, int number)
{
    return entry->dataNum == number;
}

short ModelMotion::fn_1_1A3B4(int dataNumber)
{
    ModelMotionEntry *it;
    ModelMotionEntry *end;

    it = this->motions.begin();
    end = this->motions.end();
    it = FindMotion1FF74(it, end,
        BindMotion1FF74(MotionComparator(fn_1_1A3A0), dataNumber));
    (it != end && "Requested motion is not registered.")
        ? (void)0
        : __msl_assertion_failed("it != end &&\"Requested motion is not registered.\"",
                                 "model_motion.cpp", 0x5E);
    if (it == end) {
        AddMotion1FF74(this, dataNumber);
        return this->motions.back().motionId;
    }
    return (short)it->motionId;
}
