#include "REL/m635dll.h"
#include "game/data.h"
#include "game/sprite.h"

typedef struct M635SpritePos {
    float x;
    float y;
} M635SpritePos;

extern M635SpritePos lbl_1_data_188[4];
extern u32 lbl_1_data_41C[12];
extern u32 lbl_1_data_44C[2];

void fn_1_2018(void)
{
    ANIMDATA *buttonAnim[12];
    ANIMDATA *backgroundAnim[2];
    int i;
    int member;
    s16 group;

    for (i = 0; i < 12; i++) {
        buttonAnim[i] = HuSprAnimRead(HuDataSelHeapReadNum(lbl_1_data_41C[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 2; i++) {
        backgroundAnim[i] = HuSprAnimRead(HuDataSelHeapReadNum(lbl_1_data_44C[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 4; i++) {
        group = lbl_1_bss_74[i].group = HuSprGrpCreate(14);
        HuSprGrpPosSet(group, lbl_1_data_188[i].x, lbl_1_data_188[i].y);
        for (member = 0; member < 12; member++) {
            HuSprGrpMemberSet(group, member, HuSprCreate(buttonAnim[member], 10, 0));
            HuSprPosSet(group, member, 0.0f, -5.0f);
            HuSprAttrSet(group, member, HUSPR_ATTR_DISPOFF);
        }
        for (member = 0; member < 2; member++) {
            HuSprGrpMemberSet(group, member + 12, HuSprCreate(backgroundAnim[member], 15, 0));
            HuSprAttrSet(group, member + 12, HUSPR_ATTR_DISPOFF);
            HuSprTPLvlSet(group, member + 12, 0.75f);
        }
        lbl_1_bss_74[i].member = -1;
        lbl_1_bss_74[i].state = 5;
        lbl_1_bss_74[i].scale = 0.0f;
    }
}
