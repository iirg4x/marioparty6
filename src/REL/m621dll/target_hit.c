#include "REL/m621dll.h"

void fn_1_2A54(M621Player *player, M621Target *target)
{
    HuVecF pos;
    s32 delay;
    s32 score;
    s32 i;
    M621TargetPart *part;

    delay = 0;
    if (target->part[0].state == 2) {
        target->timer = 0;
        part = &target->part[0];
        PSVECSubtract(&part->actor->pos, &player->player->actor->pos, &pos);
        part->velocity = pos;
        part->velocity.y = 10.0f;
        PSVECNormalize(&part->velocity, &part->velocity);
        PSVECScale(&part->velocity, &part->velocity, 20.0f);
        MgActorColAttrSet(part->actor, COLBODY_ATTR_BODYCOL_OFF);
        MgActorColAttrReset(part->actor, COLBODY_ATTR_MESHCOL_OFF);
        part->timer = 0;
        part->state = 3;
        MgActorPosGet(part->actor, &pos);
        fn_1_420(&pos, 1776);
        pos.y += 75.0f;
        pos.z += 80.0f;
        fn_1_51F4(player->playerNo, &pos);
        score = 1;
        for (i = 1; i < target->partCount; i++) {
            part = &target->part[i];
            if (part->state == 2) {
                MgActorPushSet(part->actor, &lbl_1_data_30);
                MgActorColAttrSet(part->actor, COLBODY_ATTR_BODYCOL_OFF);
                part->timer = delay;
                delay += 10;
                part->state = 4;
                part->effectNo = player->playerNo;
                score++;
            }
        }
        MgActorKill(target->actor);
        target->actor = NULL;
        target->state = 2;
        player->score += score;
        if (player->score > 99) {
            player->score = 99;
        }
        MgScoreValueSet(player->scoreDisplay, player->score);
        if (player->delay <= 0) {
            omVibrate(player->player->playerNo, 10, 10, 0);
            player->delay = 0;
        }
    }
}

void fn_1_2C6C(M621Player *player, M621Target *target, s16 index)
{
    HuVecF pos;
    M621TargetPart *part = &target->part[index];

    if (target->part[0].state == 2 && part->state == 2) {
        PSVECSubtract(&part->actor->pos, &player->player->actor->pos, &pos);
        part->velocity = pos;
        part->velocity.y = 20.0f;
        PSVECNormalize(&part->velocity, &part->velocity);
        PSVECScale(&part->velocity, &part->velocity, 20.0f);
        MgActorColAttrSet(part->actor, COLBODY_ATTR_BODYCOL_OFF);
        MgActorColAttrReset(part->actor, COLBODY_ATTR_MESHCOL_OFF);
        part->timer = 0;
        part->state = 3;
        MgActorPosGet(part->actor, &pos);
        fn_1_420(&pos, 1776);
        pos.y += 50.0f;
        pos.z += 55.0f;
        fn_1_51F4(player->playerNo, &pos);
        player->score++;
        if (player->score > 99) {
            player->score = 99;
        }
        MgScoreValueSet(player->scoreDisplay, player->score);
        if (player->delay <= 0) {
            omVibrate(player->player->playerNo, 10, 10, 0);
            player->delay = 0;
        }
    }
}

int fn_1_2E68(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b)
{
    M621Player *player;
    s16 index;
    s32 i;
    M621Target *target;

    if (b->type == 2 || b->type == 3) {
        player = NULL;
        target = lbl_1_bss_40[a->type - 5];
        for (i = 0; i < 4; i++) {
            if (b->paramB - 256 == lbl_1_bss_10[i]->player->actor->no) {
                player = lbl_1_bss_10[i];
                break;
            }
        }
        if (player->attackF == 2) {
            return 0;
        }
        player->attackF = 2;
        if (player->player->actor->colGroundAttr & 1) {
            for (index = target->partCount - 1; index >= 0; index--) {
                if (target->part[index].state == 2) {
                    if (index == 0) {
                        fn_1_2A54(player, target);
                    } else {
                        fn_1_2C6C(player, target, index);
                    }
                    break;
                }
            }
        } else if (a->paramB == 1) {
            fn_1_2A54(player, target);
        } else {
            fn_1_2C6C(player, target, a->paramB - 1);
        }
    }
    return 0;
}

int fn_1_2FFC(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b)
{
    if (b->type >= 5 && b->type < 8) {
        return 1;
    }
    return 0;
}

void fn_1_3024(MGACTOR *actor, int param)
{
    M621TargetPart *part = (M621TargetPart *)param;

    if (part->state == 3) {
        fn_1_2E10(part);
    } else if (part->state == 2) {
        part->velocity.y = 0.0f;
    }
}

void fn_1_2E10(M621TargetPart *part)
{
    HuVecF pos;

    part->state = 5;
    MgActorPosGet(part->actor, &pos);
    part->effectModel = Hu3DModelLink(lbl_1_bss_36);
    part->timer = 5;
}
