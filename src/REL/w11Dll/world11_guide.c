#include "dolphin.h"
#include "humath.h"

#include "game/board/guide.h"
#include "game/board/tutorial.h"

extern void mbNormPosto3D(HuVecF *src, s16 cameraMask, HuVecF *dst);

void fn_1_127C(float x, float y, float z)
{
    HuVecF pos;
    OMOBJ *guide;
    s16 modelId;

    pos.x = x;
    pos.y = y;
    pos.z = z;
    guide = mbTutorialGuideGet();
    modelId = mbGuideModelGet(guide);
    mbNormPosto3D(&pos, 4, &pos);
    mbObjPosSetV(modelId, &pos);
}
