// A model's registered motions.
//
// A minigame object reads each motion it plays once, by its data number, and plays it by
// that number afterwards: addMotion reads the data and makes a Hu3D motion of this model
// from it, and motion() finds the Hu3D motion registered for a data number. The setters
// wrap the Hu3D motion calls with that lookup.
//
// The file's name and line 94 are printed by motion's assert; the text above that line is not
// known, so these comments stand in for it. Function and member names are descriptive.
//
// The build defines NDEBUG for every object; this assert is compiled in.
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include <algorithm>
#include <functional>
#include "REL/framework/model_motion.h"
extern "C" {
#include "game/data.h"
#include "game/hu3d.h"
}

// The model is made from dataNum; no motion is set yet.
ModelMotion::ModelMotion(unsigned int dataNum) : OwnedModel(dataNum), currentDataNum(-1) {}

ModelMotion::~ModelMotion() {}

// Reads the motion data and registers it as a motion of this model.
void ModelMotion::addMotion(unsigned long dataNum)
{
    void *data = fn_80008DD0(dataNum, 0x10000000, 2);
    const short motion = fn_80030458(*this, data);
    motions.push_back(std::make_pair(dataNum, motion));
}

// Registers count motions, in order.
void ModelMotion::addMotions(const unsigned long *dataNums, unsigned long count)
{
    const unsigned long *end = dataNums + count;
    for (const unsigned long *dataNum = dataNums; dataNum != end; ++dataNum) {
        addMotion(*dataNum);
    }
}

// Plays a registered motion (Hu3DMotionSet).
void ModelMotion::setMotion(int dataNum)
{
    fn_8002C95C(*this, motion(dataNum));
    currentDataNum = dataNum;
}

// Blends into a registered motion over length frames from start (Hu3DMotionShiftSet).
// Nothing happens when dataNum is the current motion and fn_8002D75C reports false for
// the model.
void ModelMotion::shiftMotion(int dataNum, float length, float start, unsigned long attr)
{
    if (dataNum != currentDataNum || fn_8002D75C(*this)) {
        setAttribute(0x40000002);
        fn_8002CCC0(*this, motion(dataNum), start, length, attr);
        currentDataNum = dataNum;
    }
}

// Plays a registered motion over the current one (Hu3DMotionOverlaySet).
void ModelMotion::setOverlayMotion(int dataNum)
{
    fn_8002CAA8(*this, motion(dataNum));
}

// Sets a registered motion's attribute (Hu3DMotionAttrSet).
void ModelMotion::setMotionAttribute(unsigned int dataNum, unsigned short attr)
{
    fn_8002DC74(motion(dataNum), attr);
}

// True when the model has no shift motion (Hu3DMotionShiftIDGet is negative).
bool ModelMotion::shiftDone() const
{
    return fn_8002D3EC(*this) < 0;
}

// The find predicate of motion(): whether entry was registered for dataNum.
bool hasDataNum(const ModelMotion::Entry &entry, int dataNum)
{
    return entry.first == dataNum;
}

// The Hu3D motion registered for dataNum. An unregistered number asserts; when the
// assert returns, the number is registered and its new motion returned.
short ModelMotion::motion(int dataNum)
{
    std::vector<Entry>::const_iterator it = motions.begin();
    std::vector<Entry>::const_iterator end = motions.end();
    it = std::find_if(it, end, std::bind2nd(std::ptr_fun(hasDataNum), dataNum));
    assert(it != end &&"Requested motion is not registered.");
    if (it == end) {
        addMotion(dataNum);
        return motions.back().second;
    }
    return it->second;
}
