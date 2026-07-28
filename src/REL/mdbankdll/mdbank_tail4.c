#include <dolphin/mtx/GeoTypes.h>

typedef struct MdbankObject {
    u16 stat;
    s16 objNext;
    s16 prio;
    s16 prev;
    s16 next;
    s16 nextNo;
    s16 grpNo;
    u16 memberNo;
    u32 mode;
    void (*objFunc)(struct MdbankObject *obj);
    Vec trans;
    Vec rot;
    Vec scale;
    u16 modelCount;
    s16 *mdlId;
    u16 motionCount;
    s16 *motionId;
    u32 work[4];
} MDBANK_OBJECT;

extern const Vec lbl_1_rodata_148;
extern const float lbl_1_rodata_154;
extern MDBANK_OBJECT *lbl_1_bss_14;
extern float lbl_1_bss_1700[4];
extern s16 lbl_1_bss_194E[5];
extern s16 lbl_1_data_964[4];

void Hu3D3Dto2D(Vec *src, s16 cameraBit, Vec *dst);
void HuSprPosSet(s16 groupId, s16 memberNo, float x, float y);
void HuSprScaleSet(s16 groupId, s16 memberNo, float x, float y);
void HuSprAttrReset(s16 groupId, s16 memberNo, s32 attr);

void fn_1_2D98(s16 index, Vec *worldPos, float offsetX, float offsetY)
{
    MDBANK_OBJECT *obj = lbl_1_bss_14;
    Vec screenPos;

    if (obj->work[index] != 1) {
        lbl_1_bss_1700[index] = lbl_1_rodata_154;
        screenPos = lbl_1_rodata_148;
        if (worldPos) {
            Hu3D3Dto2D(worldPos, 1, &screenPos);
        }
        HuSprPosSet(lbl_1_bss_194E[1], index,
            screenPos.x + offsetX, screenPos.y + offsetY);
        HuSprScaleSet(lbl_1_bss_194E[1], index,
            lbl_1_rodata_154, lbl_1_rodata_154);
        HuSprAttrReset(lbl_1_bss_194E[1], index, 4);
        obj->work[index] = 1;
        lbl_1_data_964[index] = 1;
    }
}
