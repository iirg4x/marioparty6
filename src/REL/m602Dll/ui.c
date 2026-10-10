/* Creates and updates Odd Card Out score, timer, and message UI. */
#define _MATH_H
#include "REL/m602Dll.h"

#define MSM_SE_M602_PLAYER_INTRO 1569

u8 lbl_1_data_4B0[3] = { 64, 64, 64 };

/* Screen positions for the four player score panels. */
struct _struct_lbl_1_data_4B4_0xC lbl_1_data_4B4[4] = {
    { 66.0f, 58.0f }, { 184.0f, 58.0f }, { 391.0f, 58.0f }, { 509.0f, 58.0f }
};

struct _struct_lbl_1_data_4B4_0xC lbl_1_data_4E4[2] = { { 6.0f, 0.0f }, { 30.0f, 0.0f } };

char lbl_1_data_4FC[16] = "m602_field-dai1";

char lbl_1_data_50C[16] = "m602_field-dai2";

char lbl_1_data_51C[16] = "m602_field-dai3";

char lbl_1_data_52C[16] = "m602_field-dai4";

s16 lbl_1_bss_49A[4];

s16 lbl_1_bss_492[4];

s16 lbl_1_bss_48A[4];

s16 lbl_1_bss_482[4];

s16 lbl_1_bss_480;

MGTIMER *lbl_1_bss_47C;

u8 lbl_1_bss_478;

/* Intro setup disables timer access until the first answer round creates a timer. */
void fn_1_9ED8(void)
{
    lbl_1_bss_478 = 0;
}

/* CPU response timing reads the remaining timer frames, or -1 when the timer is inactive. */
s32 fn_1_9EEC(void)
{
    if (lbl_1_bss_478 != 0) {
        return MgTimerValueGet(lbl_1_bss_47C);
    }
    return -1;
}

/* The answer-round callback replaces the previous timer and starts a 300-frame countdown. */
void fn_1_9F34(void)
{
    if (lbl_1_bss_478 != 0) {
        MgTimerKill(lbl_1_bss_47C);
        lbl_1_bss_478 = 0;
    }
    lbl_1_bss_478 = 1;
    lbl_1_bss_47C = MgTimerCreate(0);
    MgTimerParamSet(lbl_1_bss_47C, 300, 0, 0);
    MgTimerModeOnSet(lbl_1_bss_47C, 1);
}

/* The answer-round callback ends the input window when the timer is inactive or reaches zero. */
s32 fn_1_9FD8(void)
{
    if (lbl_1_bss_478 == 0) {
        return 1;
    }
    if (MgTimerDoneCheck(lbl_1_bss_47C) != 0) {
        return 1;
    }
    return 0;
}

/* The card-reveal callback hides the active timer display. */
void fn_1_A034(void)
{
    if (lbl_1_bss_478 != 0) {
        MgTimerRecordDispOff(lbl_1_bss_47C);
    }
}

/* Intro setup creates each player score panel, its two score markers, and the model beside the
 * player. */
void fn_1_A074(void)
{
    struct {
        /* The leading bytes are never read; the two entries hold the score-marker animations. */
        u8 unreadBytes[4];
        ANIMDATA *entry[2];
    } anim;
    s16 player;
    s16 member;
    s16 sprite;

    anim.entry[0] =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_mgconst, 30), HU_MEMNUM_OVL, HEAP_MODEL));
    anim.entry[1] =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_mgconst, 31), HU_MEMNUM_OVL, HEAP_MODEL));
    player = 0;
    while (player < 4) {
        Point3d modelPos;
        lbl_1_bss_492[player] = 0;
        lbl_1_bss_49A[player] = MgScoreBoxCreateChar(100, 36, GwPlayerConf[player].charNo);
        MgScoreBoxColorSet(lbl_1_bss_49A[player], lbl_1_data_4B0[0], lbl_1_data_4B0[1],
                           lbl_1_data_4B0[2]);
        MgScoreBoxPosSet(lbl_1_bss_49A[player], lbl_1_data_4B4[player].x,
                         lbl_1_data_4B4[player].y);
        MgScoreBoxDispSet(lbl_1_bss_49A[player], 0);
        lbl_1_bss_48A[player] = HuSprGrpCreate(5);
        member = 1;
        sprite = HuSprCreate(anim.entry[0], member, 0);
        HuSprGrpMemberSet(lbl_1_bss_48A[player], member, sprite);
        HuSprPosSet(lbl_1_bss_48A[player], member, lbl_1_data_4E4[0].x, lbl_1_data_4E4[0].y);
        member = 2;
        sprite = HuSprCreate(anim.entry[0], member, 0);
        HuSprGrpMemberSet(lbl_1_bss_48A[player], member, sprite);
        HuSprPosSet(lbl_1_bss_48A[player], member, lbl_1_data_4E4[1].x, lbl_1_data_4E4[1].y);
        member = 3;
        sprite = HuSprCreate(anim.entry[1], member, 0);
        HuSprGrpMemberSet(lbl_1_bss_48A[player], member, sprite);
        HuSprPosSet(lbl_1_bss_48A[player], member, lbl_1_data_4E4[0].x, lbl_1_data_4E4[0].y);
        HuSprAttrSet(lbl_1_bss_48A[player], member, HUSPR_ATTR_DISPOFF);
        member = 4;
        sprite = HuSprCreate(anim.entry[1], member, 0);
        HuSprGrpMemberSet(lbl_1_bss_48A[player], member, sprite);
        HuSprPosSet(lbl_1_bss_48A[player], member, lbl_1_data_4E4[1].x, lbl_1_data_4E4[1].y);
        HuSprAttrSet(lbl_1_bss_48A[player], member, HUSPR_ATTR_DISPOFF);
        HuSprGrpPosSet(lbl_1_bss_48A[player], lbl_1_data_4B4[player].x,
                       lbl_1_data_4B4[player].y);
        HuSprGrpScaleSet(lbl_1_bss_48A[player], 0.0f, 0.0f);
        lbl_1_bss_482[player] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m602, 11), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_482[player], 8U);
        Hu3DModelLayerSet(lbl_1_bss_482[player], 1);
        Hu3DModelAttrSet(lbl_1_bss_482[player], HU3D_MOTATTR_LOOP);
        if (player == 0) {
            Hu3DModelObjPosGet(lbl_1_bss_23C[0], lbl_1_data_4FC, &modelPos);
        }
        if (player == 1) {
            Hu3DModelObjPosGet(lbl_1_bss_23C[0], lbl_1_data_50C, &modelPos);
        }
        if (player == 2) {
            Hu3DModelObjPosGet(lbl_1_bss_23C[0], lbl_1_data_51C, &modelPos);
        }
        if (player == 3) {
            Hu3DModelObjPosGet(lbl_1_bss_23C[0], lbl_1_data_52C, &modelPos);
        }
        modelPos.z += 0.25f;
        Hu3DModelPosSetV(lbl_1_bss_482[player], &modelPos);
        Hu3DMotionSpeedSet(lbl_1_bss_482[player], 0.0001f);
        player += 1;
    }
}

static const s16 lbl_1_rodata_1AC[6] = { 60, 105, 150, 195, 240, 0 };

/* The intro callback changes the model beside each player to its presentation pose during
* that player's scheduled frames. */
void fn_1_A648(s16 frameNo)
{
    s16 player;
    s32 highlightOn;

    player = 0;
    while (player < 4) {
        if ((lbl_1_rodata_1AC[player] <= frameNo) && (frameNo < lbl_1_rodata_1AC[player + 1])) {
            highlightOn = 1;
            if (frameNo == lbl_1_rodata_1AC[player]) {
                HuAudFXPlay(MSM_SE_M602_PLAYER_INTRO);
            }
        } else {
            highlightOn = 0;
        }
        switch (highlightOn) {
        case 0:
            Hu3DMotionTimeSet(lbl_1_bss_482[player], 0.5f);
            break;
        case 1:
            Hu3DMotionTimeSet(lbl_1_bss_482[player], 2.5f);
            break;
        }
        player += 1;
    }
}

/* At the first round start, show all score panels and restore their marker groups to full scale. */
void fn_1_A78C(void)
{
    s16 player;

    player = 0;
    while (player < 4) {
        MgScoreBoxDispSet(lbl_1_bss_49A[player], 1);
        HuSprGrpScaleSet(lbl_1_bss_48A[player], 1.0f, 1.0f);
        player += 1;
    }
}

/* Each gameplay frame sets the player-side model motion time to the pose for its current score. */
void fn_1_A81C(void)
{
    s16 player;

    player = 0;
    while (player < 4) {
        switch (lbl_1_bss_492[player]) {
        case 0:
            Hu3DMotionTimeSet(lbl_1_bss_482[player], 0.5f);
            break;
        case 1:
            Hu3DMotionTimeSet(lbl_1_bss_482[player], 1.5f);
            break;
        case 2:
            Hu3DMotionTimeSet(lbl_1_bss_482[player], 2.5f);
            break;
        }
        player += 1;
    }
}

/* After a correct answer, save the player score and show the corresponding two score markers. */
void fn_1_A914(s16 player, s16 score)
{
    lbl_1_bss_492[player] = score;
    switch (score) {
    case 0:
        HuSprAttrReset(lbl_1_bss_48A[player], 1, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(lbl_1_bss_48A[player], 2, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(lbl_1_bss_48A[player], 3, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(lbl_1_bss_48A[player], 4, HUSPR_ATTR_DISPOFF);
        break;
    case 1:
        HuSprAttrSet(lbl_1_bss_48A[player], 1, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(lbl_1_bss_48A[player], 2, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(lbl_1_bss_48A[player], 3, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(lbl_1_bss_48A[player], 4, HUSPR_ATTR_DISPOFF);
        break;
    case 2:
        HuSprAttrSet(lbl_1_bss_48A[player], 1, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(lbl_1_bss_48A[player], 2, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(lbl_1_bss_48A[player], 3, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(lbl_1_bss_48A[player], 4, HUSPR_ATTR_DISPOFF);
        break;
    }
}

/* Score display query: return the score most recently applied to this player panel. */
s16 fn_1_AB40(s16 player)
{
    return lbl_1_bss_492[player];
}

/* Score display query: report two score steps. */
s32 fn_1_AB5C(void)
{
    return 2;
}

/* Score display hook; no additional update is performed here. */
void fn_1_AB64(void)
{

}

/* Each results frame flashes the winner model between its first and third poses; reset restarts the
 * cycle. */
void fn_1_AB68(s32 reset)
{
    s16 winner;
    s16 player;
    s32 highlightOn;

    winner = fn_1_51D4();
    if (reset != 0) {
        lbl_1_bss_480 = 0;
    }
    highlightOn = lbl_1_bss_480 / 10;
    if (highlightOn != 0) {
        highlightOn %= 2;
    } else {
        highlightOn = 0;
    }
    player = 0;
    while (player < 4) {
        if (player == winner) {
            switch (highlightOn) {
            case 0:
                Hu3DMotionTimeSet(lbl_1_bss_482[player], 0.5f);
                break;
            case 1:
                Hu3DMotionTimeSet(lbl_1_bss_482[player], 2.5f);
                break;
            }
        }
        player += 1;
    }
    lbl_1_bss_480 += 1;
}
