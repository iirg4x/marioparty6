#include "game/memory.h"

typedef struct AnimData_s ANIMDATA;
typedef s16 HUSPRID;
typedef s16 HUSPR_GROUPID;

extern s16 lbl_1_data_10E;
extern float lbl_1_rodata_194;
extern float lbl_1_rodata_198;
extern float lbl_1_rodata_280;
extern float lbl_1_rodata_284;

void *HuAR_ARAMtoMRAMFileRead(unsigned int dataNum, u32 num, HEAPID heap);
ANIMDATA *HuSprAnimRead(void *data);
HUSPRID HuSprCreate(ANIMDATA *animation, s16 priority, s16 bank);
HUSPR_GROUPID HuSprGrpCreate(s16 spriteCount);
void HuSprGrpMemberSet(HUSPR_GROUPID group, s16 member, HUSPRID sprite);
void HuSprTPLvlSet(HUSPR_GROUPID group, s16 member, float level);
void HuSprGrpPosSet(HUSPR_GROUPID group, float x, float y);
void HuSprScaleSet(HUSPR_GROUPID group, s16 member, float x, float y);
void HuSprGrpKill(HUSPR_GROUPID group);

void fn_1_A58C(void)
{
    ANIMDATA *animation;
    void *fileData;
    s16 sprite;

    fileData = HuAR_ARAMtoMRAMFileRead(0xF20033, 0x30000000, HEAP_MODEL);
    animation = HuSprAnimRead(fileData);
    lbl_1_data_10E = HuSprGrpCreate(1);
    sprite = HuSprCreate(animation, 1, 0);
    HuSprGrpMemberSet(lbl_1_data_10E, 0, sprite);
    HuSprTPLvlSet(lbl_1_data_10E, 0, lbl_1_rodata_280);
    HuSprGrpPosSet(lbl_1_data_10E, lbl_1_rodata_194,
        lbl_1_rodata_198);
    HuSprScaleSet(lbl_1_data_10E, 0, lbl_1_rodata_284,
        lbl_1_rodata_284);
}

void fn_1_A698(void)
{
    if (lbl_1_data_10E != -1) {
        HuSprGrpKill(lbl_1_data_10E);
    }
}
