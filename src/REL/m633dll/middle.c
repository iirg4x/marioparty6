#include "REL/m633dll.h"

void fn_1_3828(OMOBJ *obj)
{

}

void fn_1_382C(void)
{

}

void fn_1_3830(s32 arg0)
{
    if ((GwPlayerConf[arg0].type != 0) && ((s32) lbl_1_bss_0.unk080[arg0] == 0)) {
        if ((s32) lbl_1_bss_0.unk070[arg0] == 0) {
            fn_1_38C0(arg0);
            return;
        }
        fn_1_3FFC(arg0);
    }
}

void fn_1_38C0(s32 arg0)
{
    Point3d sp2C;
    Point3d sp20;
    Point3d sp14;
    Point3d sp8;
    M633AI *temp_r31;
    MGPLAYER *temp_r29;
    f32 var_f31;
    f32 var_f30;
    f32 var_f29;
    f32 var_f28;
    f32 temp_f27;
    f32 temp_f26;
    f32 temp_f25;
    f32 temp_f24;
    s32 temp_r3;
    s32 var_r28;

    temp_r31 = &lbl_1_bss_0.unkA40[arg0];
    lbl_1_bss_0.unkAD0 = 0;
    lbl_1_bss_0.unkAD2 = 0;
    if (lbl_1_bss_0.unk0E0 != 0) {
        if ((f32) temp_r31->unk00 < 0.0f) {
            if (frandmod(100) < (u32) lbl_1_data_260[GwPlayerConf[arg0].comDif]) {
                var_r28 = 1;
            } else {
                var_r28 = 0;
            }
            temp_r31->unk08 = var_r28;
            switch (temp_r31->unk08) {              /* switch 1; irregular */
                do {
                case 0:                             /* switch 1 */
loop_9:
                    temp_r3 = frandmod(4);
                    if (temp_r3 == arg0) {
                        goto loop_9;
                    }
                } while ((s32) lbl_1_bss_0.unk080[temp_r3] != 0);
                temp_r31->unk14 = temp_r3;
                temp_r31->unk00 = frandmod(60) + 60;
                break;
            case 1:                                 /* switch 1 */
                temp_f26 = 250.0f;
                temp_f27 = (f32) (u32) frandmod(360);
                temp_r31->unk0C = temp_f27;
                temp_r31->unk18.x = temp_f26 * HuCos(temp_f27) - temp_f26 * HuSin(temp_f27);
                temp_r31->unk18.y = 0.0f;
                temp_r31->unk18.z = temp_f26 * HuSin(temp_f27) + temp_f26 * HuCos(temp_f27);
                temp_r31->unk00 = frandmod(180) + 180;
                break;
            }
        }
        switch (temp_r31->unk08) {                  /* switch 2; irregular */
        case 0:                                     /* switch 2 */
            temp_r3 = temp_r31->unk14;
            temp_r29 = lbl_1_bss_0.unk040[temp_r3];
            Hu3DModelObjPosGet(lbl_1_bss_0.unk110, lbl_1_data_188, &sp2C);
            sp20 = temp_r29->actor->pos;
            break;
        case 1:                                     /* switch 2 */
            sp20 = temp_r31->unk18;
            Hu3DModelObjPosGet(lbl_1_bss_0.unk110, lbl_1_data_188, &sp2C);
            break;
        }
        temp_r31->unk00 -= 1;
        Hu3DModelRotGet(lbl_1_bss_0.unk110, &sp14);
        var_f31 = (f32) (180.0 * (atan2((f64) sp20.x, (f64) sp20.z) / 3.141592653589793));
        var_f30 = sp14.y;
        while (var_f31 < 0.0f) {
            var_f31 += 360.0f;
        }
        while (var_f31 > 360.0f) {
            var_f31 -= 360.0f;
        }
        while (var_f30 < 0.0f) {
            var_f30 += 360.0f;
        }
        while (var_f30 > 360.0f) {
            var_f30 -= 360.0f;
        }
        temp_f25 = var_f31 - var_f30;
        if ((f32) abs((s32) temp_f25) >= 15.0f) {
            lbl_1_bss_0.unkAD8 = 1;
        } else if (abs((s32) temp_f25) < 7) {
            lbl_1_bss_0.unkAD8 = 0;
        }
        if (lbl_1_bss_0.unkAD8 != 0) {
            PSVECSubtract(&sp20, &sp2C, &sp20);
            temp_f27 = (sp2C.x * sp20.z) - (sp2C.z * sp20.x);
            if (temp_f27 >= 0.0f) {
                lbl_1_bss_0.unkAD0 = 8192;
            } else {
                lbl_1_bss_0.unkAD0 = 16384;
            }
        }
        Hu3DModelRotGet(lbl_1_bss_0.unk110, &sp8);
        var_f29 = sp8.y;
        var_f28 = (f32) (180.0 * (atan2((f64) sp20.z, (f64) sp20.x) / 3.141592653589793));
        while (var_f29 < 0.0f) {
            var_f29 += 360.0f;
        }
        while (var_f29 > 360.0f) {
            var_f29 -= 360.0f;
        }
        while (var_f28 < 0.0f) {
            var_f28 += 360.0f;
        }
        while (var_f28 > 360.0f) {
            var_f28 -= 360.0f;
        }
        temp_f24 = var_f28 - var_f29;
        if ((lbl_1_bss_0.unk10C == 2) && (((f32) abs((s32) temp_f24) < 20.0f) || ((f32) temp_r31->unk00 < 0.0f))) {
            lbl_1_bss_0.unkAD2 = 256;
            temp_r31->unk00 = -1;
        }
    }
}

void fn_1_3FFC(s32 playerNo)
{
    Point3d pos;
    MGPLAYER *player;
    M633AI *ai;
    player = lbl_1_bss_0.unk040[playerNo];
    ai = &lbl_1_bss_0.unkA40[playerNo];
    if (lbl_1_bss_0.unk108 == 0) {
        fn_1_55D0(player, ai);
        if (ai->unk08 == 2) {
            pos = player->actor->pos;
            ai->unk0C = 180.0 * (atan2(pos.z, pos.x) / 3.141592653589793);
            ai->unk18 = pos;
            ai->unk00 = 0;
        }
        ai->unk08 = 0;
    } else if (ai->unk08 == 0) {
        if (frandmod(100) < (u32)lbl_1_data_270[GwPlayerConf[playerNo].comDif]) {
            ai->unk08 = 2;
        } else {
            ai->unk08 = 1;
        }
    }
    if (ai->unk08 == 1 && lbl_1_bss_0.unkAD4 != 0
        && frandmod(100) < (u32)lbl_1_data_280[GwPlayerConf[playerNo].comDif]) {
        ai->unk08 = 2;
    }
    switch (ai->unk08) {
    case 0:
    case 1:
        fn_1_55D0(player, ai);
        break;
    case 2:
        switch (lbl_1_bss_0.unk108) {
        case 0: fn_1_55D0(player, ai); break;
        case 1: fn_1_45EC(player, ai, NULL); break;
        case 2: fn_1_4A00(player, ai, NULL, NULL); break;
        default: fn_1_51D4(player, ai); break;
        }
        break;
    }
}

s32 fn_1_4260(Point3d *start, Point3d *end, Point3d *pos, Point3d *nearest, float *distance)
{
    Point3d line, offset, delta;
    float projection, magnitude;

    if (start->x == end->x && start->y == end->y) {
        return 0;
    }
    PSVECSubtract(end, start, &line);
    PSVECSubtract(pos, start, &offset);
    projection = PSVECDotProduct(&line, &offset);
    magnitude = PSVECMag(&line);
    if (projection >= 0.0f && projection <= magnitude * magnitude) {
        PSVECNormalize(&line, &line);
        projection = PSVECDotProduct(&offset, &line);
        nearest->x = start->x + projection * line.x;
        nearest->y = start->y + projection * line.y;
        nearest->z = start->z + projection * line.z;
        if (distance) {
            PSVECSubtract(nearest, pos, &delta);
            *distance = PSVECMag(&delta);
        }
        return 1;
    }
    return 0;
}

s32 fn_1_43D4(M633Segment *segment, Point3d *pos, Point3d *nearest, float *distance)
{
    Point3d start, end;
    s32 sp8;
    sp8 = 0;
    start = segment->unk08;
    end = segment->unk20;
    start.y = end.y = pos->y = 0.0f;
    return fn_1_4260(&start, &end, pos, nearest, distance);
}

void fn_1_4598(M633Segment *segment, Point3d *pos)
{
    pos->x = (segment->unk08.x + segment->unk20.x) / 2.0f;
    pos->y = 0.0f;
    pos->z = (segment->unk08.z + segment->unk20.z) / 2.0f;
}

void fn_1_45EC(MGPLAYER *player, M633AI *ai, M633Segment *segment)
{
    Point3d pos, nearest, delta;
    float distance;
    Point3d end, start;
    s32 sp8;
    M633Segment *current;

    current = segment == NULL ? lbl_1_bss_0.unk19C : segment;
    pos = player->actor->pos;
    if (current->unk08.x == current->unk20.x && current->unk08.z == current->unk20.z) {
        fn_1_55D0(player, ai);
        return;
    }
    pos.y = 0.0f;
    sp8 = 0;
    start = current->unk08;
    end = current->unk20;
    start.y = end.y = pos.y = 0.0f;
    if (fn_1_4260(&start, &end, &pos, &nearest, &distance)) {
        if (distance < 100.0f) {
            PSVECSubtract(&pos, &nearest, &delta);
            PSVECNormalize(&delta, &delta);
            MgPlayerPadSet(player, (s32)(56.0f * delta.x), (s32)(-56.0f * delta.z), 0, 0);
            return;
        }
        ai->unk0C = 180.0 * (atan2(pos.z, pos.x) / 3.141592653589793);
        ai->unk18 = pos;
        return;
    }
    PSVECSubtract(&pos, &current->unk20, &delta);
    distance = PSVECMag(&delta);
    if (distance < 300.0f) {
        PSVECNormalize(&delta, &delta);
        ai->unk00 = 0;
        ai->unk18.x = pos.x + 100.0f * delta.x;
        ai->unk18.y = 0.0f;
        ai->unk18.z = pos.z + 100.0f * delta.z;
        fn_1_55D0(player, ai);
        return;
    }
    ai->unk0C = 180.0 * (atan2(pos.z, pos.x) / 3.141592653589793);
    ai->unk18 = pos;
    fn_1_55D0(player, ai);
}

void fn_1_4A00(MGPLAYER *player, M633AI *ai, M633Segment *first, M633Segment *second)
{
    Point3d midFirst, midSecond;
    Point3d nearest, projectedFirst, projectedSecond, delta, pos;
    float distance, firstDistance, secondDistance;
    M633Segment *a, *b;
    s32 firstHit, secondHit;

    a = first == NULL ? lbl_1_bss_0.unk19C : first;
    b = second == NULL ? &lbl_1_bss_0.unk19C[1] : second;
    pos = player->actor->pos;
    pos.y = 0.0f;
    firstHit = fn_1_43D4(a, &pos, &projectedFirst, &firstDistance);
    secondHit = fn_1_43D4(b, &pos, &projectedSecond, &secondDistance);
    fn_1_4598(a, &midFirst);
    fn_1_4598(b, &midSecond);
    if (firstHit == 0 && secondHit == 0) {
        PSVECSubtract(&pos, &a->unk20, &delta);
        distance = PSVECMag(&delta);
        if (distance < 300.0f) {
            fn_1_45EC(player, ai, a);
            return;
        }
        PSVECSubtract(&pos, &b->unk20, &delta);
        distance = PSVECMag(&delta);
        if (distance < 300.0f) {
            fn_1_45EC(player, ai, b);
            return;
        }
        fn_1_55D0(player, ai);
        return;
    }
    if (firstHit != secondHit) {
        if (firstHit != 0) {
            PSVECSubtract(&pos, &b->unk20, &delta);
            distance = PSVECMag(&delta);
            if (distance < 300.0f) {
                fn_1_45EC(player, ai, b);
            } else {
                fn_1_45EC(player, ai, a);
            }
            return;
        }
        PSVECSubtract(&pos, &a->unk20, &delta);
        distance = PSVECMag(&delta);
        if (distance < 300.0f) {
            fn_1_45EC(player, ai, a);
        } else {
            fn_1_45EC(player, ai, b);
        }
        return;
    }
    if (fn_1_4260(&projectedFirst, &projectedSecond, &pos, &nearest, &distance) == 0) {
        if (firstDistance < secondDistance) {
            fn_1_45EC(player, ai, a);
        } else {
            fn_1_45EC(player, ai, b);
        }
    }
    nearest.x = (projectedFirst.x + projectedSecond.x) / 2.0f;
    nearest.y = 0.0f;
    nearest.z = (projectedFirst.z + projectedSecond.z) / 2.0f;
    PSVECSubtract(&nearest, &pos, &delta);
    if (PSVECMag(&delta) > 70.0f) {
        ai->unk00 = 0;
        ai->unk18 = nearest;
    }
}

void fn_1_51D4(MGPLAYER *player, M633AI *ai)
{
    Point3d pos, delta, nearest;
    M633Segment *selected[2] = { NULL, NULL };
    float distances[2] = { 99999.0f, 99999.0f };
    float distance, endDistance, selectedDistance;
    M633Segment *segment;
    s32 i, j;

    pos = player->actor->pos;
    pos.y = 0.0f;
    segment = lbl_1_bss_0.unk19C;
    for (i = 0; i < lbl_1_bss_0.unk108; segment++, i++) {
        s32 hit = fn_1_43D4(segment, &pos, &nearest, &distance);
        PSVECSubtract(&pos, &segment->unk20, &delta);
        endDistance = PSVECMag(&delta);
        for (j = 0; j < 2; j++) {
            if (selected[j] == NULL) {
                if (hit != 0 || endDistance < 300.0f) {
                    selected[j] = segment;
                    distances[j] = hit != 0 ? distance : endDistance;
                }
            } else {
                selectedDistance = hit != 0 ? distance : endDistance;
                if (selectedDistance < distances[j]) {
                    selected[j] = segment;
                    distances[j] = selectedDistance;
                } else {
                    continue;
                }
            }
            break;
        }
    }
    if (selected[0] == NULL && selected[1] == NULL) {
        fn_1_55D0(player, ai);
        return;
    }
    if (selected[0] != NULL && selected[1] != NULL) {
        fn_1_4A00(player, ai, selected[0], selected[1]);
        return;
    }
    if (selected[0] != NULL) {
        fn_1_45EC(player, ai, selected[0]);
        return;
    }
    fn_1_45EC(player, ai, selected[1]);
}

void fn_1_55D0(MGPLAYER *player, M633AI *ai)
{
    Point3d sp38;
    Point3d sp2C;
    Point3d sp20;
    Point3d sp14;
    Point3d sp8;
    f32 var_f31;
    f32 var_f30;
    f32 temp_f29;
    f32 temp_f28;
    f32 temp_f27;
    f32 temp_f26;
    f32 temp_f25;
    f32 temp_f24;
    s32 var_r28;
    s32 var_r29;

    var_r29 = 0;
    if (ai->unk00 < 0) {
        ai->unk00 += 1;
        if (ai->unk00 >= 0) {
            var_r29 = 1;
        }
    } else {
        sp38 = player->actor->pos;
        sp2C = ai->unk18;
        ai->unk00 += 1;
        if (ai->unk00 >= 90) {
            var_r29 = 1;
            ai->unk0C = (f32) (180.0 * (atan2((f64) sp38.z, (f64) sp38.x) / 3.141592653589793));
            ai->unk10 = -ai->unk10;
        }
        sp38.y = 0.0f;
        PSVECSubtract(&sp2C, &sp38, &sp20);
        temp_f27 = PSVECMag(&sp20);
        if (temp_f27 < 20.0f) {
            var_r29 = 1;
        }
        fn_1_5B20(player, &sp38, &sp2C, &sp20);
        MgPlayerPadSet(player, (s32) (56.0f * sp20.x), (s32) (-56.0f * sp20.z), 0, 0);
    }
    var_r28 = frandmod(100);
    if ((GwPlayerConf[player->playerNo == 3].comDif != 0) || (GwPlayerConf[player->playerNo == 2].comDif != 0)) {
        sp14 = player->actor->pos;
        Hu3DModelRotGet(lbl_1_bss_0.unk110, &sp8);
        var_f31 = (f32) (180.0 * (atan2((f64) sp14.x, (f64) sp14.z) / 3.141592653589793));
        var_f30 = sp8.y;
        while (var_f31 < 0.0f) {
            var_f31 += 360.0f;
        }
        while (var_f31 > 360.0f) {
            var_f31 -= 360.0f;
        }
        while (var_f30 < 0.0f) {
            var_f30 += 360.0f;
        }
        while (var_f30 > 360.0f) {
            var_f30 -= 360.0f;
        }
        temp_f26 = var_f31 - var_f30;
        if ((abs((s32) temp_f26) < 30) && (ai->unk00 < 0)) {
            var_r28 = 100;
            var_r29 = 1;
        }
    }
    if (var_r29 != 0) {
        if (var_r28 < 10) {
            ai->unk00 = -(frandmod(120) + 60);
            return;
        }
        if (frandmod(100) < 4U) {
            ai->unk10 = -ai->unk10;
        }
        temp_f29 = (f32) (u32) (frandmod(30) + 35);
        ai->unk0C += temp_f29 * ai->unk10;
        temp_f28 = ai->unk0C;
        temp_f29 = 310.0f;
        temp_f25 = temp_f29 * HuCos(temp_f28) - temp_f29 * HuSin(temp_f28);
        temp_f24 = temp_f29 * HuSin(temp_f28) + temp_f29 * HuCos(temp_f28);
        ai->unk18.x = temp_f25;
        ai->unk18.y = 0.0f;
        ai->unk18.z = temp_f24;
        ai->unk00 = 0;
    }
}

void fn_1_5B20(MGPLAYER *player, Point3d *first, Point3d *second, Point3d *third)
{
    Point3d sp24;
    Point3d sp18;
    Point3d spC;
    MGPLAYER *sp8;
    f32 temp_f31;
    f32 temp_f30;
    f32 temp_f29;
    f32 var_f28;
    f32 temp_f27;
    f32 temp_f26;
    s32 var_r27;
    s32 var_r30;
    MGPLAYER *temp_r28;

    var_f28 = 0.0f;
    var_r27 = 0;
    spC = *first;
    PSVECSubtract(second, first, third);
    temp_f26 = PSVECMag(third);
    sp8 = player;
    temp_f30 = (f32) (180.0 * (atan2((f64) third->z, (f64) third->x) / 3.141592653589793));
    var_r30 = 0;
    while (var_r30 < 4) {
        temp_r28 = lbl_1_bss_0.unk040[var_r30];
        if ((sp8 != temp_r28) && (lbl_1_bss_0.unk070[var_r30] != 0) && (lbl_1_bss_0.unk080[var_r30] == 0)) {
            sp24 = temp_r28->actor->pos;
            sp24.y = 0.0f;
            PSVECSubtract(second, &sp24, &sp18);
            temp_f27 = PSVECMag(&sp18);
            if (!(temp_f26 < temp_f27) && !(temp_f27 < 40.0f)) {
                var_r27 = 1;
                PSVECSubtract(&spC, &sp24, &sp18);
                temp_f31 = PSVECMag(&sp18);
                if (temp_f31 < 140.0f) {
                    temp_f29 = (sp24.x - second->x) * HuSin(temp_f30)
                        + (second->z - sp24.z) * HuCos(temp_f30);
                    temp_f29 /= 90.0f;
                    if (temp_f29 >= 0.0f) {
                        var_f28 += 80.0f / (1.0f + temp_f29);
                    } else {
                        var_f28 -= 80.0f / (1.0f - temp_f29);
                    }
                }
            }
        }
        var_r30 += 1;
    }
    if (var_r27 != 0) {
        temp_f30 += var_f28;
        third->x = (f32) cos((3.141592653589793 * (f64) temp_f30) / 180.0);
        third->z = (f32) sin((3.141592653589793 * (f64) temp_f30) / 180.0);
    }
    temp_f31 = PSVECMag(third);
    if (temp_f31 != 0.0f) {
        third->x /= temp_f31;
        third->y /= temp_f31;
        third->z /= temp_f31;
    }
}
