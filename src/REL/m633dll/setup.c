#include "REL/m633dll.h"

void fn_1_258C(void)
{
    Point3d sp14;
    Point3d sp8;
    f32 temp_f1;

    Hu3DModelObjPosGet(lbl_1_bss_0.unk110, lbl_1_data_160, &sp14);
    sp14.y -= 50.0f;
    sp8 = sp14;
    PSVECNormalize(&sp8, &sp8);
    lbl_1_bss_0.unk198 = 90;
    fn_1_5ED0(&sp14, &sp8);
    temp_f1 = PSVECMag(&sp14);
    sp14.x += sp8.x * -(2.0f * temp_f1);
    sp14.z += sp8.z * -(2.0f * temp_f1);
    sp8.x = -sp8.x;
    sp8.z = -sp8.z;
    fn_1_5ED0(&sp14, &sp8);
    HuAudFXPlay(1849);
}

void fn_1_26B4(OMOBJ *obj)
{

}

void fn_1_26B8(OMOBJ *obj)
{
    Point3d sp18;
    Point3d spC;
    MGPLAYER *player;
    s16 sp8;
    u32 playerNo;
    f32 temp_f1;

    playerNo = obj->work[0];
    player = lbl_1_bss_0.unk040[playerNo];
    sp8 = player->actor->mdlId;
    switch (obj->work[1]) {
    case 0:
        fn_1_70A8(0);
        obj->work[1] = 1;
        break;
    case 1:
        if (lbl_1_bss_0.unk10C == 2) {
            Hu3DModelObjPosGet(lbl_1_bss_0.unk110, lbl_1_data_160, &spC);
            spC.y -= 50.0f;
            sp18 = spC;
            PSVECNormalize(&sp18, &sp18);
            lbl_1_bss_0.unk198 = 90;
            fn_1_5ED0(&spC, &sp18);
            temp_f1 = PSVECMag(&spC);
            spC.x += sp18.x * -(2.0f * temp_f1);
            spC.z += sp18.z * -(2.0f * temp_f1);
            sp18.x = -sp18.x;
            sp18.z = -sp18.z;
            fn_1_5ED0(&spC, &sp18);
            HuAudFXPlay(1849);
            obj->work[1] = 2;
            fn_1_70A8(1);
        }
        break;
    case 2:
        if (lbl_1_bss_0.unk108 == 0) {
            obj->work[1] = 4;
        }
        break;
    }
}

void fn_1_28A4(OMOBJ *obj)
{
    Point3d sp80;
    Point3d sp74;
    Point3d sp68;
    Point3d sp5C;
    Point3d sp50;
    Point3d sp44;
    Point3d sp38;
    Point3d sp2C;
    Point3d sp20;
    Point3d sp14;
    Point3d sp8;
    f32 temp_f31;
    f32 temp_f30;
    f32 temp_f29;
    f32 temp_f28;
    s32 var_r31;
    u32 temp_r30;
    u16 var_r29;
    MGPLAYER *temp_r28;
    s32 var_r27;
    s32 var_r26;
    s32 var_r25;
    u16 var_r24;
    u16 var_r23;
    u16 var_r22;
    s16 modelId;
    s32 sound;


    temp_r30 = obj->work[0];
    if (GwPlayerConf[temp_r30].type != 0) {
        var_r24 = lbl_1_bss_0.unkAD0;
        var_r23 = lbl_1_bss_0.unkAD2;
    } else {
        var_r24 = HuPadBtn[lbl_1_bss_0.unk060[temp_r30]];
        var_r23 = HuPadBtnDown[lbl_1_bss_0.unk060[temp_r30]];
    }
    if (lbl_1_bss_0.unk108 == 0) {
        var_r22 = 24576;
        var_r29 = var_r24 & var_r22;
        var_r25 = 0;
        if (var_r29 == var_r22) {
            var_r29 = 0;
        }
        if (lbl_1_bss_0.unk0F4 != var_r29) {
            var_r25 = 1;
        }
        lbl_1_bss_0.unk0F4 = var_r29;
        if (var_r29 == 8192) {
            lbl_1_bss_0.unk194 += 358.0f;
            if (var_r25 != 0) {
                CharMotionShiftSet((s16) lbl_1_bss_0.unk050[temp_r30], lbl_1_bss_0.unk040[temp_r30]->omObj->mtnId[12], 6.0f, 6.0f, 1073741828U);
                Hu3DMotionShiftSet(lbl_1_bss_0.unk110, lbl_1_bss_0.unk116[0], 6.0f, 6.0f, 1073741828U);
            }
            if (lbl_1_bss_0.unkAE4 == -1) {
                lbl_1_bss_0.unkAE4 = HuAudFXPlay(1852);
            }
        } else if (var_r29 == 16384) {
            lbl_1_bss_0.unk194 += 2.0f;
            if (var_r25 != 0) {
                Hu3DModelAttrReset((s16) lbl_1_bss_0.unk040[temp_r30]->actor->mdlId, 1073741828U);
                CharMotionShiftSet((s16) lbl_1_bss_0.unk050[temp_r30], lbl_1_bss_0.unk040[temp_r30]->omObj->mtnId[12], 6.0f, 6.0f, 0U);
                Hu3DMotionShiftSet(lbl_1_bss_0.unk110, lbl_1_bss_0.unk116[0], 6.0f, 6.0f, 0U);
            }
            if (lbl_1_bss_0.unkAE4 == -1) {
                lbl_1_bss_0.unkAE4 = HuAudFXPlay(1852);
            }
        } else {
            temp_r28 = lbl_1_bss_0.unk040[temp_r30];
            modelId = temp_r28->actor->mdlId;
            if ((Hu3DMotionShiftIDGet(modelId) < 0) && (temp_r28->omObj->mtnId[13] != Hu3DMotionIDGet(modelId))) {
                CharMotionShiftSet(temp_r28->charNo, temp_r28->omObj->mtnId[13], 0.0f, 6.0f, 0U);
                Hu3DMotionShiftSet(lbl_1_bss_0.unk110, lbl_1_bss_0.unk114, 0.0f, 6.0f, 0U);
            }
            if (lbl_1_bss_0.unkAE4 != -1) {
                HuAudFXStop(lbl_1_bss_0.unkAE4);
                lbl_1_bss_0.unkAE4 = -1;
            }
        }
        while (lbl_1_bss_0.unk194 >= 360.0f) {
            lbl_1_bss_0.unk194 -= 360.0f;
        }
        Hu3DModelRotSet(lbl_1_bss_0.unk110, 0.0f, lbl_1_bss_0.unk194, 0.0f);
        Hu3DModelRotSet(lbl_1_bss_0.unk112, 0.0f, lbl_1_bss_0.unk194, 0.0f);
        var_r29 = var_r23;
        if ((lbl_1_bss_0.unk10C == 2) && ((s32) (var_r29 & 256) != 0)) {
            Hu3DModelObjPosGet(lbl_1_bss_0.unk110, lbl_1_data_160, &sp14);
            sp14.y -= 50.0f;
            sp20 = sp14;
            PSVECNormalize(&sp20, &sp20);
            lbl_1_bss_0.unk198 = 90;
            fn_1_5ED0(&sp14, &sp20);
            temp_f28 = PSVECMag(&sp14);
            sp14.x += sp20.x * -(2.0f * temp_f28);
            sp14.z += sp20.z * -(2.0f * temp_f28);
            sp20.x = -sp20.x;
            sp20.z = -sp20.z;
            fn_1_5ED0(&sp14, &sp20);
            HuAudFXPlay(1849);
            fn_1_70A8(1);
            omVibrate((s16) temp_r30, 20, 4, 4);
            if (lbl_1_bss_0.unkAE4 != -1) {
                HuAudFXStop(lbl_1_bss_0.unkAE4);
                lbl_1_bss_0.unkAE4 = -1;
            }
        }
        Hu3DModelObjPosGet(lbl_1_bss_0.unk110, lbl_1_data_160, &sp80);
        sp74 = sp80;
        PSVECNormalize(&sp74, &sp74);
        temp_f30 = PSVECMag(&sp80);
        sp80.x += sp74.x * -(2.0f * temp_f30);
        sp80.z += sp74.z * -(2.0f * temp_f30);
        Hu3DModelPosSetV(lbl_1_bss_0.unk180[1], &sp80);
        Hu3DModelObjPosGet(lbl_1_bss_0.unk110, lbl_1_data_160, &sp68);
        Hu3DModelObjPosGet(lbl_1_bss_0.unk110, lbl_1_data_160, &sp50);
        PSVECNormalize(&sp50, &sp50);
        temp_f29 = PSVECMag(&sp68);
        sp5C.x = sp68.x + (sp50.x * -(2.0f * temp_f29));
        sp5C.y = sp68.y = 0.0f;
        sp5C.z = sp68.z + (sp50.z * -(2.0f * temp_f29));
        var_r31 = 0;
        while (var_r31 < 4) {
            sp44 = lbl_1_bss_0.unk040[var_r31]->actor->pos;
            var_r26 = 0;
            sp44.y = 0.0f;
            if ((lbl_1_bss_0.unk070[var_r31] != 0) && (lbl_1_bss_0.unk080[var_r31] == 0)) {
                PSVECSubtract(&sp68, &sp44, &sp38);
                temp_f31 = PSVECMag(&sp38);
                if (temp_f31 < lbl_1_bss_0.unkA34) {
                    var_r26 += 1;
                }
                PSVECSubtract(&sp5C, &sp44, &sp38);
                temp_f31 = PSVECMag(&sp38);
                if (temp_f31 < lbl_1_bss_0.unkA34) {
                    var_r26 += 1;
                }
                if (var_r26 != 0) {
                    OSReport(lbl_1_data_177, temp_f31);
                    MgPlayerAttrSet(lbl_1_bss_0.unk040[var_r31], 1U);
                    MgPlayerDespawn(lbl_1_bss_0.unk040[var_r31]);
                    lbl_1_bss_0.unk080[var_r31] = 1;
                    lbl_1_bss_0.unk0E0 -= 1;
                    lbl_1_bss_0.unk0E4[var_r31]->objFunc = fn_1_34B0;
                    sp2C = lbl_1_bss_0.unk040[var_r31]->actor->pos;
                    Hu3D3Dto2D(&sp44, 1, &sp8);
                    var_r27 = (s32) sp8.x;
                    var_r27 /= 5;
                    if ((s32) var_r27 < 48) {
                        var_r27 = 48;
                    } else if ((s32) var_r27 > 127) {
                        var_r27 = 127;
                    }
                    sound = HuAudFXPlay(1851);
                    HuAudFXPanning(sound, (s16) var_r27);
                }
            }
            var_r31 += 1;
        }
    }
}

void fn_1_32C0(OMOBJ *obj)
{
    Point3d sp8;
    MGPLAYER *player;
    s16 model;
    u32 playerNo;

    playerNo = obj->work[0];
    player = lbl_1_bss_0.unk040[playerNo];
    model = player->actor->mdlId;
    switch (obj->work[1]) {
    case 0:
        sp8 = player->actor->pos;
        Hu3DModelPosSetV(model, &sp8);
        obj->work[1] = 1;
        obj->work[2] = 0;
        break;
    case 1:
        if (lbl_1_bss_0.unk108 != 0) {
            obj->work[2] += 1;
            if ((u32) obj->work[2] >= 30U) {
                obj->work[1] = 3;
                obj->work[2] = 0;
                CharMotionShiftSet(lbl_1_bss_0.unk040[playerNo]->charNo, lbl_1_bss_0.unk040[playerNo]->omObj->mtnId[14], 0.0f, 6.0f, 0U);
            }
        }
    case 2:
        break;
    case 3:
        if ((Hu3DMotionShiftIDGet(model) == -1) && (Hu3DMotionEndCheck(model) != 0)) {
            CharMotionShiftSet(lbl_1_bss_0.unk040[playerNo]->charNo, lbl_1_bss_0.unk040[playerNo]->omObj->mtnId[0], 0.0f, 6.0f, 0U);
            obj->work[1] = 4;
        }
        break;
    }
}

void fn_1_34A8(void)
{

}

void fn_1_34AC(OMOBJ *obj)
{

}

void fn_1_34B0(OMOBJ *obj)
{
    Point3d sp20;
    Point3d sp14;
    Point3d sp8;
    s32 effect;
    s32 panning;
    u32 playerNo;

    playerNo = obj->work[0];
    omVibrate((s16) playerNo, 60, 20, 0);
    obj->objFunc = fn_1_36BC;
    sp20 = lbl_1_bss_0.unk040[playerNo]->actor->pos;
    PSVECNormalize(&sp20, &sp20);
    lbl_1_bss_0.unk090[playerNo] = sp20;
    lbl_1_bss_0.unk0C0[playerNo] = 0;
    lbl_1_bss_0.unk0D0[playerNo] = 40.0f;
    CharMotionShiftSet(lbl_1_bss_0.unk040[playerNo]->charNo,
                       lbl_1_bss_0.unk040[playerNo]->omObj->mtnId[11],
                       0.0f, 5.0f, 0U);
    sp8 = lbl_1_bss_0.unk040[playerNo]->actor->pos;
    effect = CharFXPlay((s16) lbl_1_bss_0.unk050[playerNo], 576);
    Hu3D3Dto2D(&sp8, 1, &sp14);
    panning = (s32) sp14.x;
    panning /= 5;
    if (panning < 48) {
        panning = 48;
    } else if (panning > 127) {
        panning = 127;
    }
    HuAudFXPanning(effect, (s16) panning);
}
