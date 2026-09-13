#include "REL/m621dll.h"
#include "game/gamemes.h"

void fn_1_175C(OMOBJ *obj)
{
    M621Player *work = obj->data;

    work->delay--;
    work->timer++;
    if (work->timer == 155) {
        fn_1_106C(work, 7, 1.0f, 0);
    } else if (work->timer == 205) {
        fn_1_106C(work, 0, 5.0f, HU3D_MOTATTR_LOOP);
    }
    if (MgSeqModeGet() == MGSEQ_MODE_MAIN) {
        MgScoreBoxDispSet(work->scoreBox, TRUE);
        MgScoreDispOn(work->scoreDisplay);
        MgPlayerComStkOff(work->player);
        work->timer = 0;
        fn_1_140(obj, fn_1_1844);
        return;
    }
}

void fn_1_1844(OMOBJ *obj)
{
    M621Player *work = obj->data;
    MGACTOR_COLMAP_POLY poly;
    HuVecF pos;
    HuVecF from;
    HuVecF to;

    work->delay--;
    if (work->player->mode == MGPLAYER_MODE_PUNCH || work->player->mode == MGPLAYER_MODE_KICK) {
        if (!work->attackF) {
            work->attackF = TRUE;
        }
    } else {
        work->attackF = FALSE;
        work->unk_26 = 0;
        if (work->computerF) {
            fn_1_215C(obj);
        }
    }
    MgActorPosGet(work->player->actor, &pos);
    from = to = pos;
    from.y = 10000.0f;
    to.y = -10000.0f;
    if (MgActorColMapPolyGet(&from, &to, -1, &poly) && pos.y < poly.pos.y) {
        pos.y = poly.pos.y;
        MgActorPosSet(work->player->actor, &pos);
    }
    if (MgSeqModeGet() == MGSEQ_MODE_FINISH) {
        MgPlayerPadSet(work->player, 0, 0, 0, 0);
        work->timer = 3;
        fn_1_140(obj, fn_1_19C4);
        return;
    }
}

void fn_1_19C4(OMOBJ *obj)
{
    M621Player *work = obj->data;

    MgPlayerPadSet(work->player, 0, 0, 0, 0);
    if (work->player->motNo == 0) {
        MgPlayerComStkOn(work->player);
        fn_1_140(obj, fn_1_1A3C);
        return;
    }
}

void fn_1_1A3C(OMOBJ *obj)
{
    M621Player *work = obj->data;
    float rotation;

    MgPlayerPadSet(work->player, 0, 0, 0, 0);
    MgActorRotYGet(work->player->actor, &rotation);
    if (rotation == work->baseRotation) {
        lbl_1_bss_C++;
        fn_1_140(obj, fn_1_1B5C);
        return;
    }
    if (rotation < work->baseRotation) {
        rotation += 6.0f;
        if (rotation > work->baseRotation) {
            rotation = work->baseRotation;
        }
    } else {
        rotation -= 6.0f;
        if (rotation < work->baseRotation) {
            rotation = work->baseRotation;
        }
    }
    MgActorRotYSet(work->player->actor, rotation);
}

void fn_1_1B5C(OMOBJ *obj)
{
    M621Player *work = obj->data;

    MgPlayerPadSet(work->player, 0, 0, 0, 0);
    if (lbl_1_bss_8) {
        fn_1_140(obj, NULL);
        return;
    }
    if (MgSeqModeGet() == MGSEQ_MODE_WINNER) {
        work->timer = 0;
        fn_1_140(obj, fn_1_1BF4);
        return;
    }
}

void fn_1_1BF4(OMOBJ *obj)
{
    M621Player *work = obj->data;

    if (work->winnerF) {
        fn_1_106C(work, 8, 4.0f, 0);
    } else {
        fn_1_106C(work, 9, 4.0f, 0);
    }
    fn_1_140(obj, NULL);
}
