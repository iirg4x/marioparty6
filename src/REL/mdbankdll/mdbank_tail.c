#include "dolphin.h"
#include "game/hu3d.h"
#include "game/sprite.h"

extern const HuVecF lbl_1_rodata_148;
extern HUSPR_GROUPID lbl_1_bss_194E[5];

void fn_1_2CD8(s16 index, HuVecF *worldPos, float offsetX, float offsetY)
{
    HuVecF screenPos = lbl_1_rodata_148;

    if (worldPos) {
        Hu3D3Dto2D(worldPos, 1, &screenPos);
    }
    HuSprPosSet(lbl_1_bss_194E[1], index,
        screenPos.x + offsetX, screenPos.y + offsetY);
}
