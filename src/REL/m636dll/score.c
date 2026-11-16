/* Picture resources, target pixels and two-point team score displays for Pixel Perfect. */
#include <math.h>
#include "REL/m636dll.h"

/* Score-box centers in screen pixels, as left X/Y then right X/Y. */
float lbl_1_data_230[4] = {44.0f, 56.0f, 528.0f, 56.0f};

/* Starting world positions, two for each team from left to right. */
Point3d lbl_1_data_240[4] = {
    { -650.0f, 0.0f, 0.0f }, { -250.0f, 0.0f, 0.0f }, { 250.0f, 0.0f, 0.0f }, { 650.0f, 0.0f, 0.0f }
};

/* X/Z offsets from world coordinates to each team's floor-pixel grid. */
/* pixel opening camera resource. */
/* pixel play camera resource. */
f32 lbl_1_data_270[2][2] = {{850.0f, 400.0f}, {-50.0f, 400.0f}};

/* Camera motion resources, opening first and play view second. */
u32 lbl_1_data_280[2] = {DATANUM(0x5f0000, 49), DATANUM(0x5f0000, 48)};

/* Opening animation resources for the three selectable pictures. */
/* pixel picture 2 intro resource. */
/* pixel picture 1 intro resource. */
/* pixel picture 0 intro resource. */
u32 lbl_1_data_288[3] = {
    DATANUM(0x5f0000, 25), DATANUM(0x5f0000, 26), DATANUM(0x5f0000, 27)
};

/* Result animation resources for the three selectable pictures. */
/* pixel picture 2 result resource. */
/* pixel picture 1 result resource. */
/* pixel picture 0 result resource. */
u32 lbl_1_data_294[3] = {
    DATANUM(0x5f0000, 28), DATANUM(0x5f0000, 29),
    DATANUM(0x5f0000, 30)
};

/* Final-round picture resources for the three selectable pictures. */
/* pixel picture 2 finish resource. */
/* pixel picture 1 finish resource. */
/* pixel picture 0 finish resource. */
u32 lbl_1_data_2A0[3] = {
    DATANUM(0x5f0000, 31), DATANUM(0x5f0000, 32),
    DATANUM(0x5f0000, 33)
};

/* One reveal model resource per cell, grouped by picture. */
/* pixel picture 2 cell 15 resource. */
/* pixel picture 2 cell 14 resource. */
/* pixel picture 2 cell 13 resource. */
/* pixel picture 2 cell 12 resource. */
/* pixel picture 2 cell 11 resource. */
/* pixel picture 2 cell 10 resource. */
/* pixel picture 2 cell 9 resource. */
/* pixel picture 2 cell 8 resource. */
/* pixel picture 2 cell 7 resource. */
/* pixel picture 2 cell 6 resource. */
/* pixel picture 2 cell 5 resource. */
/* pixel picture 2 cell 4 resource. */
/* pixel picture 2 cell 3 resource. */
/* pixel picture 2 cell 2 resource. */
/* pixel picture 2 cell 1 resource. */
/* pixel picture 2 cell 0 resource. */
/* pixel picture 1 cell 15 resource. */
/* pixel picture 1 cell 14 resource. */
/* pixel picture 1 cell 13 resource. */
/* pixel picture 1 cell 12 resource. */
/* pixel picture 1 cell 11 resource. */
/* pixel picture 1 cell 10 resource. */
/* pixel picture 1 cell 9 resource. */
/* pixel picture 1 cell 8 resource. */
/* pixel picture 1 cell 7 resource. */
/* pixel picture 1 cell 6 resource. */
/* pixel picture 1 cell 5 resource. */
/* pixel picture 1 cell 4 resource. */
/* pixel picture 1 cell 3 resource. */
/* pixel picture 1 cell 2 resource. */
/* pixel picture 1 cell 1 resource. */
/* pixel picture 1 cell 0 resource. */
/* pixel picture 0 cell 15 resource. */
/* pixel picture 0 cell 14 resource. */
/* pixel picture 0 cell 13 resource. */
/* pixel picture 0 cell 12 resource. */
/* pixel picture 0 cell 11 resource. */
/* pixel picture 0 cell 10 resource. */
/* pixel picture 0 cell 9 resource. */
/* pixel picture 0 cell 8 resource. */
/* pixel picture 0 cell 7 resource. */
/* pixel picture 0 cell 6 resource. */
/* pixel picture 0 cell 5 resource. */
/* pixel picture 0 cell 4 resource. */
/* pixel picture 0 cell 3 resource. */
/* pixel picture 0 cell 2 resource. */
/* pixel picture 0 cell 1 resource. */
/* pixel picture 0 cell 0 resource. */
u32 lbl_1_data_2AC[3][16] = {
    {DATANUM(0x5f0000, 59), DATANUM(0x5f0000, 60), DATANUM(0x5f0000, 61), DATANUM(0x5f0000, 62),
        DATANUM(0x5f0000, 63), DATANUM(0x5f0000, 64), DATANUM(0x5f0000, 65), DATANUM(0x5f0000, 66),
            DATANUM(0x5f0000, 67), DATANUM(0x5f0000, 68), DATANUM(0x5f0000, 69),
        DATANUM(0x5f0000, 70), DATANUM(0x5f0000, 71), DATANUM(0x5f0000, 72), DATANUM(0x5f0000, 73),
            DATANUM(0x5f0000, 74)},
    {DATANUM(0x5f0000, 75), DATANUM(0x5f0000, 76), DATANUM(0x5f0000, 77), DATANUM(0x5f0000, 78),
        DATANUM(0x5f0000, 79), DATANUM(0x5f0000, 80), DATANUM(0x5f0000, 81), DATANUM(0x5f0000, 82),
            DATANUM(0x5f0000, 83), DATANUM(0x5f0000, 84), DATANUM(0x5f0000, 85),
        DATANUM(0x5f0000, 86), DATANUM(0x5f0000, 87), DATANUM(0x5f0000, 88), DATANUM(0x5f0000, 89),
            DATANUM(0x5f0000, 90)},
    {DATANUM(0x5f0000, 91), DATANUM(0x5f0000, 92), DATANUM(0x5f0000, 93), DATANUM(0x5f0000, 94),
        DATANUM(0x5f0000, 95), DATANUM(0x5f0000, 96), DATANUM(0x5f0000, 97), DATANUM(0x5f0000, 98),
            DATANUM(0x5f0000, 99), DATANUM(0x5f0000, 100), DATANUM(0x5f0000, 101),
        DATANUM(0x5f0000, 102), DATANUM(0x5f0000, 103), DATANUM(0x5f0000, 104),
        DATANUM(0x5f0000, 105), DATANUM(0x5f0000, 106)}
};

/* Scene motion resources, ordered idle, left result and right result. */
/* pixel scene right result resource. */
/* pixel scene left result resource. */
/* pixel scene idle resource. */
u32 lbl_1_data_36C[3] = {
    DATANUM(0x5f0000, 22), DATANUM(0x5f0000, 23), DATANUM(0x5f0000, 24)
};

/* Three 16-by-16 binary pictures, read as 4-by-4 target blocks for individual rounds. */
u8 lbl_1_data_378[768] = {
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1,
    0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0,
    1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0,
    0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0,
    0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0,
    1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0,
    1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1,
    0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1,
    0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,
    1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0,
    1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1
};

M636WorkingData lbl_1_bss_0;

/* Scene setup creates red and blue score boxes and two earned/empty point markers for each team. */
void fn_1_6478(void)
{
    s32 side, pointIndex;
    HUSPR_GROUPID scoreGroup, scoreBox;
    for (side = 0; side < 2; side++) {
        scoreBox = lbl_1_bss_0.scoreBoxes[side] = MgScoreBoxCreate(64, 40);
        MgScoreBoxPosSet(scoreBox, lbl_1_data_230[side * 2], lbl_1_data_230[side * 2 + 1]);
        MgScoreBoxDispSet(scoreBox, 0);
        /* pixel earned point anim. */
        if (side == 0) MgScoreBoxColorSet(scoreBox, 250, 0, 30);
        /* pixel empty point anim. */
        else MgScoreBoxColorSet(scoreBox, 0, 50, 250);
    }
    lbl_1_bss_0.earnedPointAnim = HuSprAnimDataRead(DATANUM(0x9b0000, 31));
    lbl_1_bss_0.emptyPointAnim = HuSprAnimDataRead(DATANUM(0x9b0000, 30));
    for (side = 0; side < 2; side++) {
        scoreGroup = lbl_1_bss_0.scoreGroups[side] = HuSprGrpCreate(4);
        for (pointIndex = 0; pointIndex < 2; pointIndex++) {
            HuSprGrpMemberSet(scoreGroup, pointIndex,
                              HuSprCreate(lbl_1_bss_0.earnedPointAnim, 0, 0));
            HuSprPosSet(scoreGroup, pointIndex,
                        pointIndex * 24 + lbl_1_data_230[side * 2] - 12.0f,
                        lbl_1_data_230[side * 2 + 1]);
            HuSprAttrSet(scoreGroup, pointIndex, HUSPR_ATTR_DISPOFF);
            /* Empty markers occupy the same positions as earned ones, at half opacity. */
            HuSprGrpMemberSet(scoreGroup, pointIndex + 2,
                              HuSprCreate(lbl_1_bss_0.emptyPointAnim, 0, 0));
            HuSprTPLvlSet(scoreGroup, pointIndex + 2, 0.5f);
            HuSprPosSet(scoreGroup, pointIndex + 2,
                        pointIndex * 24 + lbl_1_data_230[side * 2] - 12.0f,
                        lbl_1_data_230[side * 2 + 1]);
            HuSprAttrSet(scoreGroup, pointIndex + 2, HUSPR_ATTR_DISPOFF);
        }
    }
    fn_1_6790(0);
}

/* Scene setup and the play sequence show or hide both score-box backgrounds. */
void fn_1_6790(BOOL visible)
{
    s32 side;

    for (side = 0; side < 2; side++) {
        HUSPR_GROUPID scoreBox = lbl_1_bss_0.scoreBoxes[side];
        MgScoreBoxDispSet(scoreBox, visible);
    }
}

/* Each play-frame update shows earned point markers and empty markers for the remaining points. */
void fn_1_67F4(void) {
    s16 scoreGroup;
    s32 side;
    s32 pointIndex;

    side = 0;
    while (side < 2) {
        scoreGroup = lbl_1_bss_0.scoreGroups[side];
        pointIndex = 0;
        while (pointIndex < lbl_1_bss_0.teamPoints[side]) {
            HuSprAttrReset(scoreGroup, (s16) pointIndex, HUSPR_ATTR_DISPOFF);
            HuSprAttrSet(scoreGroup, (s16) (pointIndex + 2), HUSPR_ATTR_DISPOFF);
            pointIndex += 1;
        }
        while (pointIndex < 2) {
            HuSprAttrSet(scoreGroup, (s16) pointIndex, HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(scoreGroup, (s16) (pointIndex + 2), HUSPR_ATTR_DISPOFF);
            pointIndex += 1;
        }
        side += 1;
    }
}
