#include "REL/m633dll.h"

/* The word return is unused by every caller; no return value is defined. */
int fn_1_5ED0(Point3d *pos, Point3d *dir)
{
    OMOBJ *obj;
    M633Segment *segment;
    float angle;

    dir->y = 0.0f;
    angle = (180.0 * (atan2(dir->x, dir->z) / 3.141592653589793)) - 90.0;
    obj = omAddObjEx(lbl_1_bss_0.unk000, 10000, 1, 0, 0, fn_1_60F4);
    obj->grpNo = -1;
    obj->memberNo = 0;
    segment = &lbl_1_bss_0.unk19C[lbl_1_bss_0.unk108];
    segment->unk08 = *pos;
    segment->unk20 = *pos;
    segment->unk14 = *dir;
    obj->work[0] = (u32)segment;
    obj->mdlId[0] = Hu3DModelLink(lbl_1_bss_0.unk11A);
    Hu3DModelLayerSet(obj->mdlId[0], 6);
    omSetRot(obj, 0.0f, angle, 0.0f);
    omSetTra(obj, pos->x, pos->y, pos->z);
    omSetSca(obj, 1.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(obj->mdlId[0], 1.0f, 0.0f, 0.0f);
    segment->unk04 = 0;
    lbl_1_bss_0.unk108 += 1;
}

void fn_1_60F4(OMOBJ *obj)
{
    MGACTOR_COLMAP_POLY spEC;
    Point3d spE0;
    Point3d spD4;
    Point3d reflectedPos;
    Point3d spBC;
    Point3d modelPos;
    Point3d spA4;
    Point3d sp98;
    Point3d sp8C;
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
    f32 temp_f1;
    f32 temp_f30;
    f32 temp_f29;
    f32 temp_f27;
    f32 var_f31;
    s16 temp_r26;
    s32 var_r30;
    M633Segment *temp_r31;

    temp_r31 = (M633Segment *) obj->work[0];
    switch (temp_r31->unk04) {
    case 1:
        break;
    case 0:
        spE0.x = 22.5f * temp_r31->unk14.x;
        spE0.y = 22.5f * temp_r31->unk14.y;
        spE0.z = 22.5f * temp_r31->unk14.z;
        spD4.x = temp_r31->unk20.x + spE0.x;
        spD4.y = temp_r31->unk20.y + spE0.y;
        spD4.z = temp_r31->unk20.z + spE0.z;
        if ((MgActorColMapPolyGet(&temp_r31->unk20, &spD4, 1U, &spEC) == 1) && (spBC = *(Point3d *)((HSF_FACE *)spEC.obj->mesh.face->data)[spEC.triNo].nbt, (((u32) (spEC.code & 128) == 0) != 0)) && (PSVECDotProduct(&temp_r31->unk14, &spBC) <= 0.0f)) {
            C_VECReflect(&temp_r31->unk14, &spBC, &spE0);
            reflectedPos.x = spEC.pos.x + spE0.x;
            reflectedPos.y = spEC.pos.y + spE0.y;
            reflectedPos.z = spEC.pos.z + spE0.z;
            fn_1_5ED0(&reflectedPos, &spE0);
            temp_r31->unk04 = 1;
            temp_r31->unk20 = spEC.pos;
            Hu3DModelAttrSet(lbl_1_bss_0.unk11C[temp_r31->unk00], 1U);
            lbl_1_bss_0.unkAD4 = 1;
        } else {
            temp_r31->unk20 = spD4;
        }
        break;
    }
    if (temp_r31->unk04 == 0) {
        var_f31 = 1.0f;
        if (lbl_1_bss_0.unk198 <= 0) {
            var_f31 = (f32) (lbl_1_bss_0.unk198 + 6) / 6.0f;
            var_f31 += 0.2f;
            if (var_f31 > 1.0f) {
                var_f31 = 1.0f;
            }
        }
        modelPos = temp_r31->unk20;
        temp_r26 = lbl_1_bss_0.unk11C[temp_r31->unk00];
        Hu3DModelPosSet(temp_r26, modelPos.x, (70.0f + modelPos.y) - (70.0f * (1.0f - var_f31)), modelPos.z);
        Hu3DModelScaleSet(temp_r26, var_f31, var_f31, var_f31);
        Hu3DModelAttrReset(temp_r26, 1U);
    }
    if (lbl_1_bss_0.unk198 <= 0) {
        if (lbl_1_bss_0.unk198 < -6) {
            lbl_1_bss_0.unk108 -= 1;
            if ((lbl_1_bss_0.unk108 == 0) && (lbl_1_bss_0.unkADC == 0)) {
                fn_1_70A8(0);
            }
            Hu3DModelAttrSet(lbl_1_bss_0.unk11C[temp_r31->unk00], 1U);
            Hu3DModelKill(*obj->mdlId);
            *obj->mdlId = -1;
            omDelObjEx(lbl_1_bss_0.unk000, obj);
            return;
        }
        temp_f27 = (f32) (lbl_1_bss_0.unk198 + 6) / 6.0f;
        PSVECSubtract(&temp_r31->unk20, &temp_r31->unk08, &spE0);
        temp_f1 = PSVECMag(&spE0);
        omSetSca(obj, temp_f1 / 100.0f, temp_f27, temp_f27);
        Hu3DModelScaleSet(*obj->mdlId, temp_f1 / 100.0f, temp_f27, temp_f27);
        var_r30 = 0;
        while (var_r30 < 4) {
            spA4 = lbl_1_bss_0.unk040[var_r30]->actor->pos;
            sp98 = temp_r31->unk08;
            sp8C = temp_r31->unk20;
            if ((s32) lbl_1_bss_0.unk070[var_r30] != 0) {
                f32 temp_f26;
                f32 hitRadius;

                hitRadius = 90.0f * temp_f27;
                spA4.y = sp98.y = sp8C.y = 0.0f;
                PSVECSubtract(&sp8C, &sp98, &sp80);
                PSVECSubtract(&spA4, &sp98, &sp74);
                temp_f30 = PSVECDotProduct(&sp80, &sp74);
                temp_f26 = PSVECMag(&sp80);
                if ((temp_f30 >= 0.0f) && (temp_f30 <= (temp_f26 * temp_f26))) {
                    PSVECNormalize(&sp80, &sp80);
                    temp_f30 = PSVECDotProduct(&sp74, &sp80);
                    sp68.x = sp98.x + (temp_f30 * sp80.x);
                    sp68.y = sp98.y + (temp_f30 * sp80.y);
                    sp68.z = sp98.z + (temp_f30 * sp80.z);
                    PSVECSubtract(&sp68, &spA4, &sp68);
                    temp_f26 = PSVECMag(&sp68);
                    if (temp_f26 < hitRadius) {
                        if ((s32) lbl_1_bss_0.unk080[var_r30] == 0) {
                            OSReport(lbl_1_data_1A0, temp_f26);
                            MgPlayerAttrSet(lbl_1_bss_0.unk040[var_r30], 1U);
                            MgPlayerDespawn(lbl_1_bss_0.unk040[var_r30]);
                            lbl_1_bss_0.unk080[var_r30] = 1;
                            lbl_1_bss_0.unk0E0 -= 1;
                            (lbl_1_bss_0.unk0E4[var_r30])->objFunc = fn_1_34B0;
                            sp5C = lbl_1_bss_0.unk040[var_r30]->actor->pos;
                            fn_1_24EC(1851, &sp5C);
                        }
                    }
                }
            }
            var_r30 += 1;
        }
        return;
    }
    if (lbl_1_bss_0.unk198 > 1) {
        f32 unitScale;

        unitScale = 1.0f;
        PSVECSubtract(&temp_r31->unk20, &temp_r31->unk08, &spE0);
        temp_f1 = PSVECMag(&spE0);
        omSetSca(obj, temp_f1 / 100.0f, unitScale, unitScale);
        Hu3DModelScaleSet(*obj->mdlId, temp_f1 / 100.0f, unitScale, unitScale);
        var_r30 = 0;
        while (var_r30 < 4) {
            sp50 = lbl_1_bss_0.unk040[var_r30]->actor->pos;
            sp44 = temp_r31->unk08;
            sp38 = temp_r31->unk20;
            if ((s32) lbl_1_bss_0.unk070[var_r30] != 0) {
                f32 temp_f24;
                f32 hitRadius;

                hitRadius = 90.0f * unitScale;
                sp50.y = sp44.y = sp38.y = 0.0f;
                PSVECSubtract(&sp38, &sp44, &sp2C);
                PSVECSubtract(&sp50, &sp44, &sp20);
                temp_f29 = PSVECDotProduct(&sp2C, &sp20);
                temp_f24 = PSVECMag(&sp2C);
                if ((temp_f29 >= 0.0f) && (temp_f29 <= (temp_f24 * temp_f24))) {
                    PSVECNormalize(&sp2C, &sp2C);
                    temp_f29 = PSVECDotProduct(&sp20, &sp2C);
                    sp14.x = sp44.x + (temp_f29 * sp2C.x);
                    sp14.y = sp44.y + (temp_f29 * sp2C.y);
                    sp14.z = sp44.z + (temp_f29 * sp2C.z);
                    PSVECSubtract(&sp14, &sp50, &sp14);
                    temp_f24 = PSVECMag(&sp14);
                    if ((temp_f24 < hitRadius) && ((s32) lbl_1_bss_0.unk080[var_r30] == 0)) {
                        OSReport(lbl_1_data_1A0, temp_f24);
                        MgPlayerAttrSet(lbl_1_bss_0.unk040[var_r30], 1U);
                        MgPlayerDespawn(lbl_1_bss_0.unk040[var_r30]);
                        CharMotionShiftSet((lbl_1_bss_0.unk040[var_r30])->charNo, (((lbl_1_bss_0.unk040[var_r30])->omObj)->mtnId)[11], 0.0f, 5.0f, 0U);
                        lbl_1_bss_0.unk080[var_r30] = 1;
                        lbl_1_bss_0.unk0E0 -= 1;
                        (lbl_1_bss_0.unk0E4[var_r30])->objFunc = fn_1_34B0;
                        sp8 = lbl_1_bss_0.unk040[var_r30]->actor->pos;
                        fn_1_24EC(1851, &sp8);
                    }
                }
            }
            var_r30 += 1;
        }
    }
}

void fn_1_6DE8(void)
{
    s32 i, finished;
    switch (lbl_1_bss_0.unk10C) {
    case 3:
        lbl_1_bss_0.unkA34 = 0.0f;
        break;
    case 2:
        lbl_1_bss_0.unkA34 = 90.0f;
        break;
    case 0:
        finished = 0;
        for (i = 0; i < 2; i++) {
            if (Hu3DMotionEndCheck(lbl_1_bss_0.unk180[i]) == 1) {
                fn_1_70A8(2);
                finished = 1;
            } else if (i == 0) {
                float time, maxTime;
                maxTime = Hu3DMotionMotionMaxTimeGet(lbl_1_bss_0.unk180[i]);
                time = Hu3DMotionTimeGet(lbl_1_bss_0.unk180[i]);
                lbl_1_bss_0.unkA34 = (90.0f * time) / maxTime;
            }
        }
        if (finished && lbl_1_bss_0.unkADC == 0) {
            for (i = 0; i < 4; i++) {
                if (lbl_1_bss_0.unk070[i] == 0) {
                    omVibrate((s16)i, 20, 7, 3);
                    break;
                }
            }
        }
        break;
    case 1:
        for (i = 0; i < 2; i++) {
            if (Hu3DMotionEndCheck(lbl_1_bss_0.unk180[i]) == 1) {
                fn_1_70A8(3);
            } else if (i == 0) {
                float time, maxTime;
                maxTime = Hu3DMotionMotionMaxTimeGet(lbl_1_bss_0.unk180[i]);
                time = Hu3DMotionTimeGet(lbl_1_bss_0.unk180[i]);
                lbl_1_bss_0.unkA34 = (90.0f * time) / maxTime;
            }
        }
        break;
    }
    if (lbl_1_bss_0.unkA34 > 90.0f) {
        lbl_1_bss_0.unkA34 = 90.0f;
    }
}

void fn_1_70A8(s32 arg0)
{
    s16 var_r30;
    s32 var_r29;
    s32 var_r28;
    s16 var_r27;
    s16 var_r26;
    s16 var_r25;

    /* var_r25 is not initialized on entry. */
    /* var_r26 is not initialized on entry. */
    /* var_r27 is not initialized on entry. */
    /* var_r28 is not initialized on entry. */
    /* var_r30 is not initialized on entry. */
    switch (arg0) {                                 /* irregular */
    case 3:
        var_r28 = 1;
        var_r27 = lbl_1_bss_0.unk184[arg0];
        var_r26 = lbl_1_bss_0.unk18C[arg0];
        var_r25 = *lbl_1_bss_0.unk030->mtnId;
        var_r30 = 0;
        break;
    case 2:
        var_r28 = 1;
        var_r27 = lbl_1_bss_0.unk184[arg0];
        var_r26 = lbl_1_bss_0.unk18C[arg0];
        var_r25 = lbl_1_bss_0.unk030->mtnId[2];
        var_r30 = 2;
        break;
    case 0:
        var_r28 = 0;
        var_r27 = lbl_1_bss_0.unk184[arg0];
        var_r26 = lbl_1_bss_0.unk18C[arg0];
        var_r25 = lbl_1_bss_0.unk030->mtnId[1];
        var_r30 = 1;
        break;
    case 1:
        var_r28 = 0;
        var_r27 = lbl_1_bss_0.unk184[arg0];
        var_r26 = lbl_1_bss_0.unk18C[arg0];
        var_r25 = lbl_1_bss_0.unk030->mtnId[3];
        var_r30 = 3;
        break;
    }
    Hu3DMotionSet(lbl_1_bss_0.unk180[0], var_r27);
    Hu3DMotionSet(lbl_1_bss_0.unk180[1], var_r26);
    Hu3DMotionSet(*lbl_1_bss_0.unk030->mdlId, var_r25);
    var_r29 = 0;
    while (var_r29 < 4) {
        Hu3DModelAttrSet(lbl_1_bss_0.unk034[var_r29], 1U);
        var_r29 += 1;
    }
    Hu3DModelAttrReset(lbl_1_bss_0.unk034[var_r30], 1U);
    Hu3DMotionTimeSet(lbl_1_bss_0.unk034[var_r30], 0.0f);
    if (var_r28 != 0) {
        Hu3DModelAttrSet(lbl_1_bss_0.unk180[0], 1073741825U);
        Hu3DModelAttrSet(lbl_1_bss_0.unk180[1], 1073741825U);
        Hu3DModelAttrSet(*lbl_1_bss_0.unk030->mdlId, 1073741825U);
        Hu3DModelAttrSet(lbl_1_bss_0.unk034[var_r30], 1073741825U);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_0.unk180[0], 1073741825U);
        Hu3DModelAttrReset(lbl_1_bss_0.unk180[1], 1073741825U);
        Hu3DModelAttrReset(*lbl_1_bss_0.unk030->mdlId, 1073741825U);
        Hu3DModelAttrReset(lbl_1_bss_0.unk034[var_r30], 1073741825U);
    }
    lbl_1_bss_0.unk10C = arg0;
}
