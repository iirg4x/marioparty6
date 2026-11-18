#ifndef REL_FRAMEWORK_CAMERA_WORK_H
#define REL_FRAMEWORK_CAMERA_WORK_H
#include "REL/framework/camera_frame.h"

struct CameraVector {
    Vec value;
    // Copies the three vector components and returns the stored vector.
    Vec &operator=(const Vec &value) {
        this->value.x = value.x; this->value.y = value.y; this->value.z = value.z;
        return this->value;
    }
    const Vec &getValue() const;
};

// A camera frame, its movement targets, and its activation flag.
struct CameraWork {
    CameraFrame frame;
    CameraVector cameraPosition; // Position approached by frame.position.
    CameraVector cameraTarget; // Target approached by frame.target.
    u32 unk3C[3]; /* No code names this field. */
    u32 unk48; /* No code names this field. */
    u8 enabled;
    // Copies the position into the camera frame.
    void setPosition(const Vec &value) {
        frame.position.x = value.x; frame.position.y = value.y; frame.position.z = value.z;
    }
    // Copies the target into the camera frame.
    void setTarget(const Vec &value) {
        frame.target.x = value.x; frame.target.y = value.y; frame.target.z = value.z;
    }
    // Sets the frame position from a camera vector.
    void setPosition(const CameraVector &position) { setPosition(position.getValue()); }
    // Sets the frame target from a camera vector.
    void setTarget(const CameraVector &target) { setTarget(target.getValue()); }
};
#endif
