// A model with motions registered by their data numbers and played through those bindings.
#ifndef REL_FRAMEWORK_MODEL_MOTION_H
#define REL_FRAMEWORK_MODEL_MOTION_H

#include <utility>
#include <vector>
#include "REL/framework/model.h"

class ModelMotion : public OwnedModel {
public:
    // A registered motion: its data number and the Hu3D motion read from it.
    typedef std::pair<unsigned long, short> Entry;

    explicit ModelMotion(unsigned int dataNum);
    ~ModelMotion();
    void addMotion(unsigned long dataNum);
    void addMotions(const unsigned long *dataNums, unsigned long count);
    void setMotion(int dataNum);
    void shiftMotion(int dataNum, float length, float start, unsigned long attr);
    void setOverlayMotion(int dataNum);
    void setMotionAttribute(unsigned int dataNum, unsigned short attr);
    bool shiftDone() const;
    short motion(int dataNum);

    std::vector<Entry> motions;
    int currentDataNum; // The data number setMotion or shiftMotion set last; -1 before any.
};

#endif
