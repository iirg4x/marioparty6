#ifndef M652DLL_CAMERA_ARC_MOTION_H
#define M652DLL_CAMERA_ARC_MOTION_H

#include "dolphin/mtx.h"
#include "REL/framework/range.h"

class CameraArcMotion {
public:
    u16 cameraBit;
    Range<float> angle;
    f32 horizontalMotion;
    f32 forwardMotion;
    u32 flags_primary;
    u32 flags_secondary;

    CameraArcMotion(u16 cameraBit, f32 angle);
    ~CameraArcMotion();
    void AddSecondaryFlags(u32 flags);
    void AddPrimaryFlags(u32 flags);
    void SetPrimaryFlag200();
    void fn_1_AC40();
    void fn_1_AC50();
    void fn_1_AE90(void *player);
    void fn_1_B02C();

    void fn_1_AC60(const Vec *movement, f32 amount);
};

#endif
