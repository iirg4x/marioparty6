#include "REL/m602Dll.h"

/* static */
MGSEQ_PARAM lbl_1_data_0 = {
    0,
    2,
    fn_1_10C,
    fn_1_210,
    fn_1_278,
    fn_1_2AC,
    fn_1_7B8,
    fn_1_8F0,
    fn_1_A5C,
    NULL,
    fn_1_AB8,
};

M602CameraParams lbl_1_data_28 = {
    8,
    32.0f,
    10.0f,
    5000.0f,
    1.2f,
    0.0f,
    0.0f,
    640.0f,
    480.0f,
    0.0f,
    1.0f,
    { 0.0f, 0.0f, 1000.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f },
};

M602CameraPose lbl_1_data_78 = { { 0.0f, 394.0f, 1990.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 394.0f, 0.0f } };

M602CameraPose lbl_1_data_9C[2] = {
    { { -50.0f, 200.0f, 980.0f }, { 0.0f, 1.0f, 0.0f }, { -500.0f, 200.0f, 0.0f } },
    { { 50.0f, 200.0f, 980.0f }, { 0.0f, 1.0f, 0.0f }, { 500.0f, 200.0f, 0.0f } },
};

static const Point3d lbl_1_rodata_10 = { 0.0f, 5000.0f, 1.0f };

/* const */
static const Point3d lbl_1_rodata_1C = { 0.0f, 1.0f, 0.0f };

/* const */
static const Point3d lbl_1_rodata_28 = { 0.0f, 0.0f, 0.0f };

s16 lbl_1_bss_10;

s32 lbl_1_bss_C;

HUPROCESS *lbl_1_bss_8;

s16 lbl_1_bss_4;

s16 lbl_1_bss_2;

s16 lbl_1_bss_0;

void fn_1_A0(void)
{
    lbl_1_bss_8 = omInitObjMan(300, 4096);
    omGameSysInit(lbl_1_bss_8);
    MgSeqCreatePrio(&lbl_1_data_0, 2000);
    MgSeqModeDelaySet(2, -1);
    MgSeqModeDelaySet(7, -1);
}

void fn_1_10C(s16 mode, s16 frameNo)
{
    Point3d sp20;
    Point3d sp14;
    Point3d sp8;

    lbl_1_bss_10 = 0;
    fn_1_AD8();
    sp20 = lbl_1_rodata_10;
    sp14 = lbl_1_rodata_1C;
    sp8 = lbl_1_rodata_28;
    Hu3DShadowCreate(30.0f, 20.0f, 5000.0f);
    Hu3DShadowTPLvlSet(0.5f);
    Hu3DShadowPosSet(&sp20, &sp14, &sp8);
    fn_1_411C();
    fn_1_179C();
    fn_1_5254();
    fn_1_9ED8();
    fn_1_A074();
    lbl_1_bss_C = HuAudBGMPlay(83);
    MgSeqModeNext();
}

void fn_1_210(s16 arg1, s16 frameNo)
{
    s8 sp8;
    u8 temp_r30;

    temp_r30 = fn_1_D54(frameNo);
    sp8 = fn_1_5C7C(frameNo);
    fn_1_A648(frameNo);
    fn_1_4AAC();
    if (temp_r30 != 0) {
        fn_1_6078();
        MgSeqModeNext();
    }
}

void fn_1_278(s16 mode, s16 frameNo)
{
    lbl_1_bss_10 = 1;
    fn_1_A81C();
    fn_1_4AAC();
}

void fn_1_2AC(s16 arg1, s16 frameNo)
{
    s16 temp_r28;
    s16 temp_r3;
    s16 temp_r3_2;

    if (lbl_1_bss_10 == 1) {
        if (frameNo == 0) {
            fn_1_A78C();
        }
        lbl_1_bss_10 = 0;
        lbl_1_bss_0 = 0;
        fn_1_2998();
        fn_1_22DC();
        fn_1_61E0();
        HuAudFXPlay(1571);
    } else if (lbl_1_bss_10 == 0) {
        fn_1_22DC();
        fn_1_61E0();
        if ((fn_1_2E3C(0) == 2) && (fn_1_2E3C(1) == 2) && (fn_1_2E3C(2) == 2)) {
            lbl_1_bss_10 = 1;
            fn_1_9F34();
            lbl_1_bss_10 = 2;
            lbl_1_bss_0 = 0;
            HuAudFXPlay(1578);
        }
    } else if (lbl_1_bss_10 == 2) {
        fn_1_22DC();
        fn_1_647C();
        if ((fn_1_9FD8() != 0) || (fn_1_4F90() != 0)) {
            lbl_1_bss_10 = 3;
            lbl_1_bss_0 = 0;
        }
    } else if (lbl_1_bss_10 == 3) {
        if (lbl_1_bss_0 == 2) {
            fn_1_A034();
        }
        if (lbl_1_bss_0 == 60) {
            temp_r3_2 = fn_1_2B28();
            if (temp_r3_2 == 0) {
                HuAudFXPanning(HuAudFXPlay(1572), 32);
            } else if (temp_r3_2 == 1) {
                HuAudFXPanning(HuAudFXPlay(1572), 64);
            } else {
                HuAudFXPanning(HuAudFXPlay(1572), 96);
            }
        }
        if (lbl_1_bss_0 > 150) {
            lbl_1_bss_10 = 4;
            lbl_1_bss_0 = 0;
        }
        fn_1_22DC();
        fn_1_6DB8();
    } else if (lbl_1_bss_10 == 4) {
        if (((s32) (fn_1_2E3C(0) & 3) != 0) && ((s32) (fn_1_2E3C(1) & 3) != 0) && ((s32) (fn_1_2E3C(2) & 3) != 0)) {
            if (lbl_1_bss_0 == 1) {
                fn_1_6F00();
                temp_r28 = fn_1_2D5C();
                temp_r3_2 = fn_1_5048();
                if (temp_r3_2 >= 0) {
                    temp_r3 = fn_1_502C(temp_r3_2);
                    if (temp_r28 == temp_r3) {
                        fn_1_8D90(temp_r3_2, 1);
                        temp_r3 = fn_1_50D4(temp_r3_2);
                        fn_1_A914(temp_r3_2, temp_r3);
                        lbl_1_bss_2 = fn_1_4A80();
                    } else {
                        fn_1_8D90(temp_r3_2, 0);
                    }
                } else {
                    fn_1_8D90(-1, 0);
                }
            }
            fn_1_22DC();
            temp_r3_2 = fn_1_6F14();
            if ((temp_r3_2 < 0) && (lbl_1_bss_2 <= 0)) {
                fn_1_4A98();
                lbl_1_bss_10 = 5;
                lbl_1_bss_0 = 0;
            }
        } else {
            fn_1_22DC();
            fn_1_6DB8();
            lbl_1_bss_0 = 0;
        }
    } else if (lbl_1_bss_10 == 5) {
        if ((fn_1_178C() >= 10) || (fn_1_5170() != 0)) {
            if (lbl_1_bss_0 >= 30) {
                MgSeqModeNext();
                HuAudSStreamFadeOut(lbl_1_bss_C, 100);
                lbl_1_bss_0 = 0;
            }
            if (fn_1_51D4() >= 0) {
                fn_1_3908();
            }
        } else if (lbl_1_bss_0 >= 60) {
            lbl_1_bss_10 = 1;
            lbl_1_bss_0 = 0;
        }
    }
    fn_1_4AAC();
    fn_1_4CD0(frameNo);
    fn_1_A81C();
    fn_1_1428();
    fn_1_1778(frameNo);
    lbl_1_bss_0 += 1;
    lbl_1_bss_2 -= 1;
    if (lbl_1_bss_2 < 0) {
        lbl_1_bss_2 = 0;
    }
}

void fn_1_7B8(s16 arg1, s16 frameNo)
{
    s16 winners[4];
    s16 temp_r30;
    s16 var_r31;

    temp_r30 = fn_1_51D4();
    if (frameNo == 1) {
        if (temp_r30 >= 0) {
            if (_CheckFlag(65551U) == 0) {
                GwPlayer[temp_r30].mgCoinBonus = 10;
            }
            var_r31 = 0;
            while (var_r31 < 4) {
                (&winners[0])[var_r31] = -1;
                var_r31 += 1;
            }
            (&winners[0])[temp_r30] = fn_1_5238(temp_r30);
            MgSeqWinnerSet(winners[0], winners[1], winners[2], winners[3]);
        } else {
            var_r31 = 0;
            while (var_r31 < 4) {
                winners[var_r31] = -1;
                var_r31 += 1;
            }
            MgSeqWinnerSet(winners[0], winners[1], winners[2], winners[3]);
        }
    }
    if (temp_r30 >= 0) {
        fn_1_3908();
    }
    fn_1_22DC();
    fn_1_A81C();
    fn_1_4AAC();
}

void fn_1_8F0(s16 arg1, s16 frameNo)
{
    s16 temp_r29;
    s16 var_r30;

    temp_r29 = fn_1_51D4();
    if (frameNo == 0) {
        lbl_1_bss_4 = 1;
    }
    fn_1_A81C();
    if (temp_r29 >= 0) {
        if (frameNo < 60) {
            var_r30 = fn_1_8E64(frameNo);
        }
        var_r30 = 0;
        if (frameNo < lbl_1_bss_4) {
            var_r30 = fn_1_34A4(frameNo);
            if (var_r30 > 0) {
                lbl_1_bss_4 = frameNo + 1;
            } else {
                lbl_1_bss_4 = frameNo + 2;
            }
        }
        if (var_r30 == -1) {
            fn_1_AB68(1);
            fn_1_9480(1);
            fn_1_48B4();
            fn_1_90B4();
            MgSeqModeNext();
        }
    } else if (frameNo == 0) {
        fn_1_2BB0(135, 3);
    } else if (frameNo < 135) {
        fn_1_22DC();
    } else {
        fn_1_22DC();
        var_r30 = fn_1_8E64((s16) (frameNo - 135));
        if (var_r30 != 0) {
            fn_1_90B4();
            MgSeqModeNext();
        }
    }
    fn_1_4AAC();
}

void fn_1_A5C(s16 mode, s16 frameNo)
{
    s16 temp_r31;

    temp_r31 = fn_1_51D4();
    fn_1_A81C();
    fn_1_AB68(0);
    fn_1_9480(0);
    if (temp_r31 >= 0) {
        fn_1_34A4(1);
    }
    fn_1_9378();
    fn_1_4AAC();
}

void fn_1_AB8(s16 mode, s16 frameNo)
{
    fn_1_22A0();
}

void fn_1_AD8(void)
{
    lbl_1_data_28.pos = lbl_1_data_78.pos;
    lbl_1_data_28.up = lbl_1_data_78.up;
    lbl_1_data_28.target = lbl_1_data_78.target;
    Hu3DCameraCreate(lbl_1_data_28.cameraBit);
    Hu3DCameraViewportSet(lbl_1_data_28.cameraBit, lbl_1_data_28.viewportX, lbl_1_data_28.viewportY, lbl_1_data_28.viewportW, lbl_1_data_28.viewportH, lbl_1_data_28.minZ, lbl_1_data_28.maxZ);
    Hu3DCameraPerspectiveSet(lbl_1_data_28.cameraBit, lbl_1_data_28.fov, lbl_1_data_28.nearPlane, lbl_1_data_28.farPlane, lbl_1_data_28.aspect);
    Hu3DCameraScissorSet(lbl_1_data_28.cameraBit, (u32) lbl_1_data_28.viewportX, (u32) lbl_1_data_28.viewportY, (u32) lbl_1_data_28.viewportW, (u32) lbl_1_data_28.viewportH);
    lbl_1_data_28.pos = lbl_1_data_9C[0].pos;
    lbl_1_data_28.up = lbl_1_data_9C[0].up;
    lbl_1_data_28.target = lbl_1_data_9C[0].target;
    Hu3DCameraPosSetV(lbl_1_data_28.cameraBit, &lbl_1_data_28.pos, &lbl_1_data_28.up, &lbl_1_data_28.target);
    lbl_1_bss_18 = 0;
}
