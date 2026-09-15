#include "REL/m650/m650.h"

void fn_1_A0(void);
void fn_1_F0(s16 mode, s16 frame);
void fn_1_168(s16 mode, s16 frame);
void fn_1_188(s16 mode, s16 frame);
void fn_1_1C8(s16 mode, s16 frame);
void fn_1_200(s16 mode, s16 frame);
void fn_1_304(s16 mode, s16 frame);
void fn_1_3AC(s16 mode, s16 frame);
void fn_1_3B0(s16 mode, s16 frame);
void fn_1_3B4(s16 mode, s16 frame);
void fn_1_3B8(void);
void fn_1_484(void);
void fn_1_550(void);
void fn_1_8EC(void);
void fn_1_F90(OMOBJ *obj);
void fn_1_1228(void);
void fn_1_14FC(void);
void fn_1_1830(void);
void fn_1_1AE4(void);
void fn_1_2010(s16 arg0);
void fn_1_21CC(OMOBJ *obj);
void fn_1_22A8(s16 player, s16 motion);
void fn_1_2320(s16 arg0, s16 arg1);
void fn_1_23C4(s16 player, s16 motion, float blend);
void fn_1_2464(s16 player);
void fn_1_2558(void);
void fn_1_25B0(void);
void fn_1_26D0(void);
void fn_1_273C(s16 arg0);
void fn_1_289C(s16 player);
s16 fn_1_2CAC(s16 arg0);
void fn_1_2F1C(s16 arg0);
void fn_1_33F8(s16 player);
void fn_1_3F28(s16 player);
void fn_1_4430(s16 player);
void fn_1_473C(s16 arg0);
void fn_1_48C0(s16 arg0);
void fn_1_49C8(s16 arg0);
void fn_1_4BD4(s16 arg0);
s16 fn_1_4D58(s16 arg0);
void fn_1_5258(void);
void fn_1_5308(void);
void fn_1_5354(void);
void fn_1_5394(void);
void fn_1_54D4(void);
void fn_1_5544(s16 arg0);
void fn_1_5600(s16 unusedPlayer, f32 value, f32 *out0, f32 *out1);
s16 fn_1_5728(void);
void fn_1_5A24(s16 arg0);
void fn_1_5E40(void);
void fn_1_62E0(void);
void fn_1_6478(HU3D_MODEL *modelP, Mtx *mtx);
s16 fn_1_69B0(s16 player);
s16 fn_1_6D88(s16 player, HuVecF *pos);
s16 fn_1_6EE4(HuVecF *a, HuVecF *b, float radius);
void fn_1_6F54(void);
s16 fn_1_7000(s16 unusedPlayer, HuVecF *pos, float angle, s16 candidate);

extern GXColor lbl_1_data_40[2];
extern Point3d lbl_1_data_28;
extern Point3d lbl_1_data_34;
extern s16 lbl_1_bss_4AC[4];
extern s16 lbl_1_bss_4B4;
extern s16 lbl_1_bss_4B6;
extern s32 lbl_1_bss_2A8;
extern s32 lbl_1_data_48[4];
extern s32 lbl_1_data_80[2];
extern s32 lbl_1_data_88[2];
extern s32 lbl_1_data_90[2];
extern s32 lbl_1_data_98[2];
extern s32 lbl_1_data_A0[2];

MGSEQ_PARAM lbl_1_data_0 = {
    300,
    1,
    fn_1_F0,
    fn_1_168,
    fn_1_188,
    fn_1_1C8,
    fn_1_200,
    fn_1_304,
    fn_1_3AC,
    fn_1_3B0,
    fn_1_3B4,
};

/* Eight unreferenced retail BSS bytes; original purpose unknown. */
u8 lbl_1_bss_20[8];
OMOBJMAN *lbl_1_bss_1C;
M650Scene lbl_1_bss_0;

void fn_1_A0(void)
{
    lbl_1_bss_1C = omInitObjMan(256, 8192);
    omGameSysInit(lbl_1_bss_1C);
    MgSeqCreate(&lbl_1_data_0);
}

void fn_1_F0(s16 mode, s16 frame)
{
    s16 night;

    memset(&lbl_1_bss_0, 0, sizeof(lbl_1_bss_0));
    night = GwMgNightF;
    lbl_1_bss_0.night = night;
    lbl_1_bss_0.winner = -1;
    fn_1_14FC();
    fn_1_1830();
    fn_1_1AE4();
    fn_1_62E0();
    fn_1_5258();
    MgSeqModeNext();
}

void fn_1_168(s16 mode, s16 frame)
{
    fn_1_550();
}

void fn_1_188(s16 mode, s16 frame)
{
    if (frame == 0) {
        lbl_1_bss_0.audio = HuAudBGMPlay(78);
    }
}

void fn_1_1C8(s16 mode, s16 frame)
{
    if (frame == 0) {
        fn_1_5308();
        fn_1_54D4();
        fn_1_2558();
    }
}

void fn_1_200(s16 mode, s16 frame)
{
    int player;

    if (frame == 0) {
        HuAudSStreamFadeOut(lbl_1_bss_0.audio, 100);
        fn_1_25B0();
        if (_CheckFlag(196610) == 0) {
            if (lbl_1_bss_0.winner == -1) {
                MgSeqDrawSet();
            } else {
                MgSeqWinnerSet(lbl_1_bss_28[lbl_1_bss_0.winner].charNo, -1, -1, -1);
                player = lbl_1_bss_0.winner;
                if (_CheckFlag(65551) == 0) {
                    GwPlayer[player].mgCoinBonus = 10;
                }
            }
            lbl_1_bss_0.state = 0;
            lbl_1_bss_0.frame = 0;
        }
    }
}

void fn_1_304(s16 mode, s16 frame)
{
    int player;
    int score;

    if (_CheckFlag(196610) != 0) {
        MgSeqModeSet(9);
        for (player = 0; player < 4; player++) {
            score = lbl_1_bss_28[player].unk76;
            GwPlayer[player].mgScore = score;
        }
    } else if (lbl_1_bss_0.winner == -1) {
        MgSeqModeNext();
    } else {
        fn_1_8EC();
    }
}

void fn_1_3AC(s16 mode, s16 frame) { }

void fn_1_3B0(s16 mode, s16 frame) { }

void fn_1_3B4(s16 mode, s16 frame) { }

void fn_1_3B8(void)
{
    OM_CAMERA_VIEW view;
    int i;

    view.center.x = 0.0f;
    view.center.y = 150.0f;
    view.center.z = 0.0f;
    view.rot.x = -15.0f;
    view.rot.y = -180.0f;
    view.rot.z = 0.0f;
    view.zoom = 1000.0f;
    for (i = 0; i < 4; i++) {
        omCameraViewMoveMulti(lbl_1_data_150[i], &view, 120, 2);
    }
}

void fn_1_484(void)
{
    OM_CAMERA_VIEW view;
    int i;

    view.center.x = 0.0f;
    view.center.y = 300.0f;
    view.center.z = 0.0f;
    view.rot.x = -5.0f;
    view.rot.y = -180.0f;
    view.rot.z = 0.0f;
    view.zoom = 1300.0f;
    for (i = 0; i < 4; i++) {
        omCameraViewMoveMulti(lbl_1_data_150[i], &view, 60, 0);
    }
}

void fn_1_550(void)
{
    HuVecF pos;
    HuVecF target;
    HuVecF up;

    switch (lbl_1_bss_0.state) {
    case 0:
        MgSeqModeChangeOff();
        lbl_1_bss_0.state++;
        break;
    case 1:
        if (lbl_1_bss_0.frame++ >= 60) {
            lbl_1_bss_0.frame = 0;
            lbl_1_bss_0.state++;
            fn_1_3B8();
        } else if (lbl_1_bss_0.frame == 30) {
            HuAudFXPlay(1001);
        }
        break;
    case 2:
        if (lbl_1_bss_0.frame++ >= 60) {
            lbl_1_bss_0.frame = 0;
            lbl_1_bss_0.state++;
        }
        break;
    case 3:
        if (fn_1_4D58(lbl_1_bss_0.frame++)) {
            fn_1_484();
            lbl_1_bss_0.frame = 0;
            lbl_1_bss_0.state++;
            pos.x = -1500.0f;
            pos.y = 9000.0f;
            pos.z = -2000.0f;
            up.x = 0.0f;
            up.y = 1.0f;
            up.z = 0.0f;
            target.x = target.y = 0.0f;
            target.z = 2500.0f;
            Hu3DShadowMultiPosSet(&pos, &up, &target, 15);
        }
        break;
    case 4:
        if (lbl_1_bss_0.frame++ >= 60) {
            fn_1_5394();
            lbl_1_bss_0.frame = 0;
            MgSeqModeNext();
        }
        break;
    }
}

void fn_1_8EC(void)
{
    HuVecF shadowPos;
    HuVecF shadowTarget;
    HuVecF shadowUp;
    M650Player *temp_r31;
    s32 var_r30;
    s32 var_r29;

    switch (lbl_1_bss_0.state) {
    case 0:
        MgSeqModeChangeOff();
        WipeCreate(2, 0, 60);
        lbl_1_bss_0.frame = 0;
        lbl_1_bss_0.state += 1;
        return;
    case 1:
        if (WipeCheck() != 0) {
            break;
        }
            lbl_1_bss_0.unk12 = 1;
            fn_1_6F54();
            fn_1_26D0();
            if (lbl_1_bss_0.recordChanged != 0) {
                fn_1_1228();
            }
            shadowPos.x = -1000.0f;
            shadowPos.y = 5000.0f;
            shadowPos.z = -1000.0f;
            shadowUp.x = 0.0f;
            shadowUp.y = 1.0f;
            shadowUp.z = 0.0f;
            shadowTarget.x = shadowTarget.y = 0.0f;
            shadowTarget.z = 2500.0f;
            Hu3DShadowMultiPosSet(&shadowPos, &shadowUp, &shadowTarget, 15);
            for (var_r30 = 0; var_r30 < 4; var_r30++) {
                temp_r31 = &lbl_1_bss_28[var_r30];
                if (var_r30 == lbl_1_bss_0.winner) {
                    CharModelKill(temp_r31->charNo);
                    temp_r31->model = CharModelCreate(temp_r31->charNo, 2);
                    var_r29 = 0;
                    while (var_r29 < 5) {
                        temp_r31->motionIDs[var_r29] = CharMotionCreate(temp_r31->charNo, (u32) lbl_1_data_58[var_r29].file);
                        var_r29 += 1;
                    }
                }
                Hu3DModelShadowSet(temp_r31->model);
                if (lbl_1_bss_0.recordChanged == 0) {
                    Hu3DModelPosSet(temp_r31->model20, -200.0f, 0.0f, 1200.0f);
                    Hu3DModelRotSet(temp_r31->model20, 0.0f, 170.0f, 0.0f);
                    Hu3DModelAttrReset(temp_r31->model52, 1U);
                    Hu3DModelPosSet(temp_r31->model52, -180.0f, 5.0f, 1100.0f);
                    Hu3DModelAttrReset(temp_r31->model54, 1U);
                    Hu3DModelPosSet(temp_r31->model54, -180.0f, 0.2f, 1100.0f);
                    Hu3DMotionSet(temp_r31->model20, temp_r31->jointMotionIDs[3]);
                }
                fn_1_23C4((s16) var_r30, 3, 1.0f);
                CenterM[var_r30].x = 0.0f;
                CenterM[var_r30].y = 150.0f;
                CenterM[var_r30].z = 900.0f;
                CRotM[var_r30].x = -5.0f;
                CRotM[var_r30].y = 180.0f;
                CRotM[var_r30].z = 0.0f;
                CZoomM[var_r30] = 800.0f;
                if (lbl_1_bss_0.winner == var_r30) {
                    Hu3DCameraScissorSet((s32) lbl_1_data_150[var_r30], 0U, 0U, 640U, 480U);
                    Hu3DCameraViewportSet((s32) lbl_1_data_150[var_r30], 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
                } else {
                    Hu3DCameraScissorSet((s32) lbl_1_data_150[var_r30], 0U, 0U, 0U, 0U);
                    Hu3DCameraViewportSet((s32) lbl_1_data_150[var_r30], 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
                }
            }
            lbl_1_bss_0.state += 1;
        break;
    case 2:
        WipeCreate(1, 5, 60);
        lbl_1_bss_0.state += 1;
        return;
    case 3:
        if (WipeCheck() == 0) {
            if (lbl_1_bss_0.recordChanged == 0) {
                fn_1_2320(lbl_1_bss_0.winner, 4);
                MgSeqModeNext();
                return;
            }
            MgTimerRecordSet(lbl_1_bss_0.timer, -1);
            lbl_1_bss_0.state += 1;
            return;
        }
        break;
    case 4:
        if ((f32) lbl_1_bss_0.frame++ > 150.0f) {
            fn_1_2320(lbl_1_bss_0.winner, 4);
            MgSeqModeNext();
            lbl_1_bss_0.frame = 0;
            lbl_1_bss_0.state = 11;
        }
        if (lbl_1_bss_0.frame == 110) {
            lbl_1_bss_0.unk16 = 1;
            if (lbl_1_bss_0.night != 0) {
                HuAudFXPlayPan(2036, 96);
                return;
            }
            HuAudFXPlayPan(2036, 48);
        }
        break;
    }
}

void fn_1_F90(OMOBJ *obj)
{
    Point3d sp8;                                    /* compiler-managed */
    M650Player *temp_r31;
    s16 winner;

    winner = lbl_1_bss_0.winner;
    temp_r31 = &lbl_1_bss_28[winner];
    switch (lbl_1_bss_0.unk16) {
    case 1:
        Hu3DModelPosGet(temp_r31->model52, &sp8);
        sp8.x += temp_r31->pos.x;
        sp8.y += temp_r31->pos.y;
        sp8.z += temp_r31->pos.z;
        if (sp8.y <= 0.5f) {
            HuAudFXPlayPan(2036, (s16) (64.0f + (sp8.x / 50.0f)));
            temp_r31->pos.x *= 0.5;
            temp_r31->pos.y *= -0.5;
            temp_r31->pos.z *= 0.5;
            sp8.y = 0.5f;
            if (temp_r31->pos.y < 1.0f) {
                lbl_1_bss_0.unk16 += 1;
                if (lbl_1_bss_0.night == 0) {
                    HuAudFXPlayPan(1001, 96);
                } else {
                    HuAudFXPlayPan(1001, 32);
                }
            }
        }
        temp_r31->pos.y -= 1.6333333f;
        Hu3DModelPosSetV(temp_r31->model52, (Point3d *) &sp8);
        Hu3DModelPosSet(temp_r31->model54, sp8.x, 0.2f, sp8.z);
        break;
    case 2:
        Hu3DModelPosGet(temp_r31->model20, (Point3d *) &sp8);
        if (temp_r31->unk48 < 10.0f) {
            temp_r31->unk48 += 0.2f;
        }
        sp8.x += temp_r31->unk48 * (f32) (1 - (lbl_1_bss_0.night * 2));
        Hu3DModelPosSetV(temp_r31->model20, (Point3d *) &sp8);
        break;
    }
}

void fn_1_1228(void)
{
    OMOBJ *sp8;
    M650Player *temp_r31;
    s16 winner;

    winner = lbl_1_bss_0.winner;
    temp_r31 = &lbl_1_bss_28[winner];
    sp8 = omAddObjEx(lbl_1_bss_1C, 100, 0U, 0U, -1, fn_1_F90);
    Hu3DModelPosSet(temp_r31->model20, -700.0f + (f32) (lbl_1_bss_0.night * 1400), 0.0f, 1300.0f);
    Hu3DModelRotSet(temp_r31->model20, 0.0f, 90.0f + (180.0f * (f32) lbl_1_bss_0.night), 0.0f);
    Hu3DMotionSet(temp_r31->model20, temp_r31->jointMotionIDs[1]);
    Hu3DModelAttrReset(temp_r31->model52, 1U);
    Hu3DModelPosSet(temp_r31->model52, -550.0f + (1100.0f * (f32) lbl_1_bss_0.night), 200.0f, 1300.0f);
    Hu3DModelAttrReset(temp_r31->model54, 1U);
    Hu3DModelPosSet(temp_r31->model54, -550.0f + (1100.0f * (f32) lbl_1_bss_0.night), 0.2f, 1300.0f);
    temp_r31->pos.x = 30.0f - (60.0f * (f32) lbl_1_bss_0.night);
    temp_r31->pos.y = 15.0f;
    temp_r31->pos.z = 0.0f;
    temp_r31->unk48 = 0.0f;
}
