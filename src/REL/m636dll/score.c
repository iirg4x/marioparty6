/* Picture resources, target pixels and two-point team score displays for Pixel Perfect. */
#include "dolphin/math.h"
#include "REL/m636dll.h"

#define PIXEL_EARNED_POINT_ANIM DATANUM(DATA_mgconst, 31)
#define PIXEL_EMPTY_POINT_ANIM DATANUM(DATA_mgconst, 30)

#define PIXEL_SCENE_IDLE_RESOURCE DATANUM(DATA_m636, 22)
#define PIXEL_SCENE_LEFT_RESULT_RESOURCE DATANUM(DATA_m636, 23)
#define PIXEL_SCENE_RIGHT_RESULT_RESOURCE DATANUM(DATA_m636, 24)
#define PIXEL_PICTURE_0_INTRO_RESOURCE DATANUM(DATA_m636, 25)
#define PIXEL_PICTURE_1_INTRO_RESOURCE DATANUM(DATA_m636, 26)
#define PIXEL_PICTURE_2_INTRO_RESOURCE DATANUM(DATA_m636, 27)
#define PIXEL_PICTURE_0_RESULT_RESOURCE DATANUM(DATA_m636, 28)
#define PIXEL_PICTURE_1_RESULT_RESOURCE DATANUM(DATA_m636, 29)
#define PIXEL_PICTURE_2_RESULT_RESOURCE DATANUM(DATA_m636, 30)
#define PIXEL_PICTURE_0_FINISH_RESOURCE DATANUM(DATA_m636, 31)
#define PIXEL_PICTURE_1_FINISH_RESOURCE DATANUM(DATA_m636, 32)
#define PIXEL_PICTURE_2_FINISH_RESOURCE DATANUM(DATA_m636, 33)
#define PIXEL_PLAY_CAMERA_RESOURCE DATANUM(DATA_m636, 48)
#define PIXEL_OPENING_CAMERA_RESOURCE DATANUM(DATA_m636, 49)
#define PIXEL_PICTURE_0_CELL_0_RESOURCE DATANUM(DATA_m636, 59)
#define PIXEL_PICTURE_0_CELL_1_RESOURCE DATANUM(DATA_m636, 60)
#define PIXEL_PICTURE_0_CELL_2_RESOURCE DATANUM(DATA_m636, 61)
#define PIXEL_PICTURE_0_CELL_3_RESOURCE DATANUM(DATA_m636, 62)
#define PIXEL_PICTURE_0_CELL_4_RESOURCE DATANUM(DATA_m636, 63)
#define PIXEL_PICTURE_0_CELL_5_RESOURCE DATANUM(DATA_m636, 64)
#define PIXEL_PICTURE_0_CELL_6_RESOURCE DATANUM(DATA_m636, 65)
#define PIXEL_PICTURE_0_CELL_7_RESOURCE DATANUM(DATA_m636, 66)
#define PIXEL_PICTURE_0_CELL_8_RESOURCE DATANUM(DATA_m636, 67)
#define PIXEL_PICTURE_0_CELL_9_RESOURCE DATANUM(DATA_m636, 68)
#define PIXEL_PICTURE_0_CELL_10_RESOURCE DATANUM(DATA_m636, 69)
#define PIXEL_PICTURE_0_CELL_11_RESOURCE DATANUM(DATA_m636, 70)
#define PIXEL_PICTURE_0_CELL_12_RESOURCE DATANUM(DATA_m636, 71)
#define PIXEL_PICTURE_0_CELL_13_RESOURCE DATANUM(DATA_m636, 72)
#define PIXEL_PICTURE_0_CELL_14_RESOURCE DATANUM(DATA_m636, 73)
#define PIXEL_PICTURE_0_CELL_15_RESOURCE DATANUM(DATA_m636, 74)
#define PIXEL_PICTURE_1_CELL_0_RESOURCE DATANUM(DATA_m636, 75)
#define PIXEL_PICTURE_1_CELL_1_RESOURCE DATANUM(DATA_m636, 76)
#define PIXEL_PICTURE_1_CELL_2_RESOURCE DATANUM(DATA_m636, 77)
#define PIXEL_PICTURE_1_CELL_3_RESOURCE DATANUM(DATA_m636, 78)
#define PIXEL_PICTURE_1_CELL_4_RESOURCE DATANUM(DATA_m636, 79)
#define PIXEL_PICTURE_1_CELL_5_RESOURCE DATANUM(DATA_m636, 80)
#define PIXEL_PICTURE_1_CELL_6_RESOURCE DATANUM(DATA_m636, 81)
#define PIXEL_PICTURE_1_CELL_7_RESOURCE DATANUM(DATA_m636, 82)
#define PIXEL_PICTURE_1_CELL_8_RESOURCE DATANUM(DATA_m636, 83)
#define PIXEL_PICTURE_1_CELL_9_RESOURCE DATANUM(DATA_m636, 84)
#define PIXEL_PICTURE_1_CELL_10_RESOURCE DATANUM(DATA_m636, 85)
#define PIXEL_PICTURE_1_CELL_11_RESOURCE DATANUM(DATA_m636, 86)
#define PIXEL_PICTURE_1_CELL_12_RESOURCE DATANUM(DATA_m636, 87)
#define PIXEL_PICTURE_1_CELL_13_RESOURCE DATANUM(DATA_m636, 88)
#define PIXEL_PICTURE_1_CELL_14_RESOURCE DATANUM(DATA_m636, 89)
#define PIXEL_PICTURE_1_CELL_15_RESOURCE DATANUM(DATA_m636, 90)
#define PIXEL_PICTURE_2_CELL_0_RESOURCE DATANUM(DATA_m636, 91)
#define PIXEL_PICTURE_2_CELL_1_RESOURCE DATANUM(DATA_m636, 92)
#define PIXEL_PICTURE_2_CELL_2_RESOURCE DATANUM(DATA_m636, 93)
#define PIXEL_PICTURE_2_CELL_3_RESOURCE DATANUM(DATA_m636, 94)
#define PIXEL_PICTURE_2_CELL_4_RESOURCE DATANUM(DATA_m636, 95)
#define PIXEL_PICTURE_2_CELL_5_RESOURCE DATANUM(DATA_m636, 96)
#define PIXEL_PICTURE_2_CELL_6_RESOURCE DATANUM(DATA_m636, 97)
#define PIXEL_PICTURE_2_CELL_7_RESOURCE DATANUM(DATA_m636, 98)
#define PIXEL_PICTURE_2_CELL_8_RESOURCE DATANUM(DATA_m636, 99)
#define PIXEL_PICTURE_2_CELL_9_RESOURCE DATANUM(DATA_m636, 100)
#define PIXEL_PICTURE_2_CELL_10_RESOURCE DATANUM(DATA_m636, 101)
#define PIXEL_PICTURE_2_CELL_11_RESOURCE DATANUM(DATA_m636, 102)
#define PIXEL_PICTURE_2_CELL_12_RESOURCE DATANUM(DATA_m636, 103)
#define PIXEL_PICTURE_2_CELL_13_RESOURCE DATANUM(DATA_m636, 104)
#define PIXEL_PICTURE_2_CELL_14_RESOURCE DATANUM(DATA_m636, 105)
#define PIXEL_PICTURE_2_CELL_15_RESOURCE DATANUM(DATA_m636, 106)

/* Byte positions of the two team scores and their sprite groups. */
#define PIXEL_TEAM_POINTS_OFFSET 252
#define PIXEL_SCORE_BOXES_OFFSET 610
#define PIXEL_SCORE_GROUPS_OFFSET 614

/* Score-box centers in screen pixels, as left X/Y then right X/Y. */
float lbl_1_data_230[4] = {44.0f, 56.0f, 528.0f, 56.0f};

/* Starting world positions, two for each team from left to right. */
Point3d lbl_1_data_240[4] = {
    { -650.0f, 0.0f, 0.0f }, { -250.0f, 0.0f, 0.0f }, { 250.0f, 0.0f, 0.0f }, { 650.0f, 0.0f, 0.0f }
};

/* X/Z offsets from world coordinates to each team's floor-pixel grid. */
f32 lbl_1_data_270[2][2] = {{850.0f, 400.0f}, {-50.0f, 400.0f}};

/* Camera motion resources, opening first and play view second. */
u32 lbl_1_data_280[2] = {PIXEL_OPENING_CAMERA_RESOURCE, PIXEL_PLAY_CAMERA_RESOURCE};

/* Opening animation resources for the three selectable pictures. */
u32 lbl_1_data_288[3] = {
    PIXEL_PICTURE_0_INTRO_RESOURCE, PIXEL_PICTURE_1_INTRO_RESOURCE, PIXEL_PICTURE_2_INTRO_RESOURCE
};

/* Result animation resources for the three selectable pictures. */
u32 lbl_1_data_294[3] = {
    PIXEL_PICTURE_0_RESULT_RESOURCE, PIXEL_PICTURE_1_RESULT_RESOURCE,
    PIXEL_PICTURE_2_RESULT_RESOURCE
};

/* Final-round picture resources for the three selectable pictures. */
u32 lbl_1_data_2A0[3] = {
    PIXEL_PICTURE_0_FINISH_RESOURCE, PIXEL_PICTURE_1_FINISH_RESOURCE,
    PIXEL_PICTURE_2_FINISH_RESOURCE
};

/* One reveal model resource per cell, grouped by picture. */
u32 lbl_1_data_2AC[3][16] = {
    {PIXEL_PICTURE_0_CELL_0_RESOURCE, PIXEL_PICTURE_0_CELL_1_RESOURCE, PIXEL_PICTURE_0_CELL_2_RESOURCE, PIXEL_PICTURE_0_CELL_3_RESOURCE, PIXEL_PICTURE_0_CELL_4_RESOURCE, PIXEL_PICTURE_0_CELL_5_RESOURCE, PIXEL_PICTURE_0_CELL_6_RESOURCE, PIXEL_PICTURE_0_CELL_7_RESOURCE, PIXEL_PICTURE_0_CELL_8_RESOURCE, PIXEL_PICTURE_0_CELL_9_RESOURCE, PIXEL_PICTURE_0_CELL_10_RESOURCE, PIXEL_PICTURE_0_CELL_11_RESOURCE, PIXEL_PICTURE_0_CELL_12_RESOURCE, PIXEL_PICTURE_0_CELL_13_RESOURCE, PIXEL_PICTURE_0_CELL_14_RESOURCE, PIXEL_PICTURE_0_CELL_15_RESOURCE},
    {PIXEL_PICTURE_1_CELL_0_RESOURCE, PIXEL_PICTURE_1_CELL_1_RESOURCE, PIXEL_PICTURE_1_CELL_2_RESOURCE, PIXEL_PICTURE_1_CELL_3_RESOURCE, PIXEL_PICTURE_1_CELL_4_RESOURCE, PIXEL_PICTURE_1_CELL_5_RESOURCE, PIXEL_PICTURE_1_CELL_6_RESOURCE, PIXEL_PICTURE_1_CELL_7_RESOURCE, PIXEL_PICTURE_1_CELL_8_RESOURCE, PIXEL_PICTURE_1_CELL_9_RESOURCE, PIXEL_PICTURE_1_CELL_10_RESOURCE, PIXEL_PICTURE_1_CELL_11_RESOURCE, PIXEL_PICTURE_1_CELL_12_RESOURCE, PIXEL_PICTURE_1_CELL_13_RESOURCE, PIXEL_PICTURE_1_CELL_14_RESOURCE, PIXEL_PICTURE_1_CELL_15_RESOURCE},
    {PIXEL_PICTURE_2_CELL_0_RESOURCE, PIXEL_PICTURE_2_CELL_1_RESOURCE, PIXEL_PICTURE_2_CELL_2_RESOURCE, PIXEL_PICTURE_2_CELL_3_RESOURCE, PIXEL_PICTURE_2_CELL_4_RESOURCE, PIXEL_PICTURE_2_CELL_5_RESOURCE, PIXEL_PICTURE_2_CELL_6_RESOURCE, PIXEL_PICTURE_2_CELL_7_RESOURCE, PIXEL_PICTURE_2_CELL_8_RESOURCE, PIXEL_PICTURE_2_CELL_9_RESOURCE, PIXEL_PICTURE_2_CELL_10_RESOURCE, PIXEL_PICTURE_2_CELL_11_RESOURCE, PIXEL_PICTURE_2_CELL_12_RESOURCE, PIXEL_PICTURE_2_CELL_13_RESOURCE, PIXEL_PICTURE_2_CELL_14_RESOURCE, PIXEL_PICTURE_2_CELL_15_RESOURCE}
};

/* Scene motion resources, ordered idle, left result and right result. */
u32 lbl_1_data_36C[3] = {
    PIXEL_SCENE_IDLE_RESOURCE, PIXEL_SCENE_LEFT_RESULT_RESOURCE, PIXEL_SCENE_RIGHT_RESULT_RESOURCE
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
        if (side == 0) MgScoreBoxColorSet(scoreBox, 250, 0, 30);
        else MgScoreBoxColorSet(scoreBox, 0, 50, 250);
    }
    lbl_1_bss_0.earnedPointAnim = HuSprAnimDataRead(PIXEL_EARNED_POINT_ANIM);
    lbl_1_bss_0.emptyPointAnim = HuSprAnimDataRead(PIXEL_EMPTY_POINT_ANIM);
    for (side = 0; side < 2; side++) {
        scoreGroup = lbl_1_bss_0.scoreGroups[side] = HuSprGrpCreate(4);
        for (pointIndex = 0; pointIndex < 2; pointIndex++) {
            HuSprGrpMemberSet(scoreGroup, pointIndex,
                              HuSprCreate(lbl_1_bss_0.earnedPointAnim, 0, 0));
            HuSprPosSet(scoreGroup, pointIndex,
                        pointIndex * 24 + lbl_1_data_230[side * 2] - (12.0f),
                        lbl_1_data_230[side * 2 + 1]);
            HuSprAttrSet(scoreGroup, pointIndex, HUSPR_ATTR_DISPOFF);
            /* Empty markers occupy the same positions as earned ones, at half opacity. */
            HuSprGrpMemberSet(scoreGroup, pointIndex + 2,
                              HuSprCreate(lbl_1_bss_0.emptyPointAnim, 0, 0));
            HuSprTPLvlSet(scoreGroup, pointIndex + 2, (0.5f));
            HuSprPosSet(scoreGroup, pointIndex + 2,
                        pointIndex * 24 + lbl_1_data_230[side * 2] - (12.0f),
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
        HUSPR_GROUPID scoreBox = *(s16 *)((s8 *)&lbl_1_bss_0 + PIXEL_SCORE_BOXES_OFFSET + side * 2);
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
        scoreGroup = (*(s16 *) ((s8 *) ((s32 *) ((u8 *) &lbl_1_bss_0 + (side * 2))) +
                                (PIXEL_SCORE_GROUPS_OFFSET)));
        pointIndex = 0;
        while (pointIndex < (s32) (*(s32 *) ((s8 *) ((s32 *) ((u8 *) &lbl_1_bss_0 + (side * 4))) +
                                             (PIXEL_TEAM_POINTS_OFFSET)))) {
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
