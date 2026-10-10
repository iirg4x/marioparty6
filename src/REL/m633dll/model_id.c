/* Animates a player after the obstacle removes them from the play area. */
#include "REL/m633dll.h"

/* Installed by fn_1_34B0 after a hazard hit; each update moves the model outward and adds a
 * vertical offset that starts at +40 and drops by 1.2, then hides it after 120 updates. */
void fn_1_36BC(OMOBJ *playerObject)
{
    Point3d hitReactionModelPosition;
    s16 playerModelId;
    u32 playerSlot;

    playerSlot = playerObject->work[0];
    playerModelId = lbl_1_bss_0.players[playerSlot]->actor->mdlId;
    Hu3DModelPosGet(playerModelId, &hitReactionModelPosition);
    hitReactionModelPosition.x += 24.0f * lbl_1_bss_0.hitDirections[playerSlot].x;
    hitReactionModelPosition.y += lbl_1_bss_0.hitVerticalOffsets[playerSlot];
    hitReactionModelPosition.z += 24.0f * lbl_1_bss_0.hitDirections[playerSlot].z;
    Hu3DModelPosSetV(playerModelId, &hitReactionModelPosition);
    lbl_1_bss_0.hitUpdateCounts[playerSlot] += 1;
    lbl_1_bss_0.hitVerticalOffsets[playerSlot] -= 1.2f;
    if ((s32) lbl_1_bss_0.hitUpdateCounts[playerSlot] >= 120) {
        playerObject->objFunc = fn_1_3828;
        Hu3DModelAttrSet(playerModelId, HU3D_ATTR_DISPOFF);
    }
}
