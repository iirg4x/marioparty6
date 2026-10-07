/* Animates a player after the obstacle removes them from the play area. */
#include "REL/m633dll.h"

/* Per-frame hit-reaction callback: lifts the character for 120 updates, then hides its model. */
void fn_1_36BC(OMOBJ *obj)
{
    Point3d hitReactionModelPosition;
    s16 modelId;
    u32 playerNo;

    playerNo = obj->work[0];
    modelId = lbl_1_bss_0.players[playerNo]->actor->mdlId;
    Hu3DModelPosGet(modelId, &hitReactionModelPosition);
    hitReactionModelPosition.x += 24.0f * lbl_1_bss_0.hitDirections[playerNo].x;
    hitReactionModelPosition.y += lbl_1_bss_0.hitVerticalOffsets[playerNo];
    hitReactionModelPosition.z += 24.0f * lbl_1_bss_0.hitDirections[playerNo].z;
    Hu3DModelPosSetV(modelId, &hitReactionModelPosition);
    lbl_1_bss_0.hitUpdateCounts[playerNo] += 1;
    lbl_1_bss_0.hitVerticalOffsets[playerNo] -= 1.2f;
    if ((s32) lbl_1_bss_0.hitUpdateCounts[playerNo] >= 120) {
        obj->objFunc = fn_1_3828;
        Hu3DModelAttrSet(modelId, HU3D_ATTR_DISPOFF);
    }
}
