#include <dolphin/mtx/GeoTypes.h>

#define HU3D_ATTR_DISPOFF (1 << 0)

typedef s16 HU3D_MODELID;

extern HU3D_MODELID lbl_1_bss_3A[2];

void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
void Hu3DModelAttrReset(HU3D_MODELID modelId, u32 attr);

void fn_1_8410(s16 modelNo, s16 display)
{
    if (display != 0) {
        Hu3DModelAttrReset(lbl_1_bss_3A[modelNo], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_3A[modelNo], HU3D_ATTR_DISPOFF);
    }
}
