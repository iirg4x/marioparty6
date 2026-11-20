#ifndef REL_M611DLL_MOTION_TRANSFORM_H
#define REL_M611DLL_MOTION_TRANSFORM_H
#include "REL/framework/model.h"
// Applies a model's heading and source position.
inline void applyMotionTransform(OwnedModel &model, f64 heading, Vec *sourcePosition)
{
    Vec rotation;
    rotation.x = 0.0f;
    rotation.y = heading;
    rotation.z = 0.0f;
    model.setRotation(&rotation);
    model.setPosition(sourcePosition);
}
// Rotates a forward step by the model heading.
template <class MovingModel>
inline void rotateForwardStep(MovingModel *owner, Mtx &rotation,
                                     Vec &source, Vec &transformed)
{
    PSMTXRotRad(rotation, 'y', MTXDegToRad(owner->heading));
    PSMTXMultVec(rotation, &source, &transformed);
}

#endif
