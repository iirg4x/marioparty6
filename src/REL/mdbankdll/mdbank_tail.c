#include <dolphin/mtx/GeoTypes.h>

extern const Vec lbl_1_rodata_148;
extern s16 lbl_1_bss_194E[5];

void fn_1_2CD8(s16 index, Vec *worldPos, float offsetX, float offsetY)
{
    Vec screenPos = lbl_1_rodata_148;

    if (worldPos) {
        Hu3D3Dto2D(worldPos, 1, &screenPos);
    }
    HuSprPosSet(lbl_1_bss_194E[1], index,
        screenPos.x + offsetX, screenPos.y + offsetY);
}
