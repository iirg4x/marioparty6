#include "REL/m621dll.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/memory.h"
#include "datadir_enum.h"
#include "game/frand.h"
#include "string.h"
#include "math.h"
#include "game/gamework.h"
#include "game/gamemes.h"

struct M621Effect_s {
    s16 active;
    HU3D_MODELID model;
};

void fn_1_5168(OMOBJ *obj);

struct M621Environment_s {
    /* The 16-byte allocation's first four bytes have no recovered access. */
    u8 unk_00[4];
    OMOBJ *modelObj;
    HU3D_MODELID hookModel;
    HU3D_MOTIONID hookMotion;
    HU3D_MODELID model;
};

M621Environment *lbl_1_bss_4C;
M621Target *lbl_1_bss_40[3];
HU3D_MODELID lbl_1_bss_3E;
HU3D_MODELID lbl_1_bss_3C;
HU3D_MODELID lbl_1_bss_3A;
HU3D_MODELID lbl_1_bss_38;
HU3D_MODELID lbl_1_bss_36;
HU3D_MOTIONID lbl_1_bss_34;
HU3D_MODELID lbl_1_bss_2C[4];
HU3D_MOTIONID lbl_1_bss_24[4];
M621Effect *lbl_1_bss_20;
M621Player *lbl_1_bss_10[4];
s32 lbl_1_bss_C;
s16 lbl_1_bss_8;

HuVecF lbl_1_data_30 = {0.0f, 0.0f, 0.0f};
s32 lbl_1_data_3C[2][5] = {
    {DATANUM(DATA_m621, 2), -1, DATANUM(DATA_m621, 3),
        DATANUM(DATA_m621, 1), DATANUM(DATA_m621, 0)},
    {DATANUM(DATA_m621, 5), DATANUM(DATA_m621, 6), DATANUM(DATA_m621, 7),
        DATANUM(DATA_m621, 4), DATANUM(DATA_m621, 0)}
};
s32 lbl_1_data_64[2][2] = {
    {DATANUM(DATA_m621, 33), -1},
    {DATANUM(DATA_m621, 35), DATANUM(DATA_m621, 34)}
};
s32 lbl_1_data_74[2][3] = {
    {DATANUM(DATA_m621, 15), DATANUM(DATA_m621, 17), DATANUM(DATA_m621, 27)},
    {DATANUM(DATA_m621, 16), DATANUM(DATA_m621, 18), DATANUM(DATA_m621, 28)}
};
s32 lbl_1_data_8C[4] = {
    DATANUM(DATA_m621, 8), DATANUM(DATA_m621, 9),
    DATANUM(DATA_m621, 10), DATANUM(DATA_m621, 11)
};
s32 lbl_1_data_9C[4][2] = {
    {DATANUM(DATA_m621, 21), DATANUM(DATA_m621, 29)},
    {DATANUM(DATA_m621, 22), DATANUM(DATA_m621, 30)},
    {DATANUM(DATA_m621, 23), DATANUM(DATA_m621, 31)},
    {DATANUM(DATA_m621, 24), DATANUM(DATA_m621, 32)}
};
/* Unreferenced initializer; vector type inferred from its three float words. */
HuVecF lbl_1_data_BC = {0.0f, 1.0f, 0.0f};
char *lbl_1_data_258[21] = {
    "621sambo-samboitiA", "621sambo-samboitiB", "621sambo-samboitiC",
    "621sambo-samboitiD", "621sambo-samboitiE", "621sambo-samboitiF",
    "621sambo-samboitiG", "621sambo-samboitiI", "621sambo-samboitiJ",
    "621sambo-samboitiK", "621sambo-samboitiL", "621sambo-samboitiM",
    "621sambo-samboitiN", "621sambo-samboitiO", "621sambo-samboitiP",
    "621sambo-samboitiQ", "621sambo-samboitiR", "621sambo-samboitiS",
    "621sambo-samboitiT", "621sambo-samboitiU", "621sambo-samboitiV"
};
u32 lbl_1_data_2AC = ~1U;

unsigned int lbl_1_data_2B0[17] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mariomot, 3),
    DATANUM(DATA_mariomot, 4), DATANUM(DATA_mariomot, 5),
    DATANUM(DATA_mariomot, 56), DATANUM(DATA_mariomot, 24),
    DATANUM(DATA_mariomot, 6), DATANUM(DATA_mariomot, 40),
    DATANUM(DATA_mariomot, 15), DATANUM(DATA_mariomot, 16),
    DATANUM(DATA_mariomot, 17), DATANUM(DATA_mariomot, 18),
    DATANUM(DATA_mariomot, 22), DATANUM(DATA_mariomot, 22), 0
};
char *lbl_1_data_334[4] = {
    "621sambo-P1null", "621sambo-P2null", "621sambo-P3null", "621sambo-P4null"
};
HuVecF lbl_1_data_344[4] = {
    {55.0f, 420.0f, 0.0f}, {175.0f, 420.0f, 0.0f},
    {401.0f, 420.0f, 0.0f}, {521.0f, 420.0f, 0.0f}
};


s32 fn_1_420(HuVecF *pos, s16 soundId)
{
    HuVecF screenPos;
    s16 pan;
    s16 volume;

    Hu3D3Dto2D(pos, HU3D_CAM0, &screenPos);
    if (screenPos.x > 608.0f) {
        pan = 80;
    } else if (screenPos.x < 32.0f) {
        pan = 48;
    } else {
        pan = 48.0f + (32.0f * (screenPos.x - 32.0f)) / 576.0f;
    }
    if (screenPos.y > 400.0f) {
        volume = 127;
    } else if (screenPos.y < 40.0f) {
        volume = 96;
    } else {
        volume = 96.0f + (31.0f * (screenPos.y - 40.0f)) / 360.0f;
    }
    return HuAudFXPlayVolPan(soundId, volume, pan);
}

s32 fn_1_594(s16 pan, s16 soundId)
{
    return HuAudFXPlayVolPan(soundId, 127, pan);
}

void fn_1_5C8(s32 sound)
{
    HuAudFXStop(sound);
}

void fn_1_5F0(OMOBJ *obj)
{
    s32 i;

    frandom(OSGetTick());
    lbl_1_bss_8 = _CheckFlag(FLAG_INST_DECA);
    fn_1_A0(10, sizeof(M621Camera), fn_1_B4C);
    lbl_1_bss_4C = fn_1_A0(10, 16, fn_1_D28);
    for (i = 0; i < 4; i++) {
        lbl_1_bss_10[i] = fn_1_A0(20, sizeof(M621Player), fn_1_13EC);
        lbl_1_bss_10[i]->playerNo = i;
    }
    lbl_1_bss_C = 0;
    fn_1_27A0();
    lbl_1_bss_20 = fn_1_A0(10, 84, fn_1_5054);
    fn_1_140(obj, fn_1_70C);
}

void fn_1_70C(OMOBJ *obj)
{
    M621Round *work = obj->data;

    if (MgSeqModeGet() == MGSEQ_MODE_FADEIN) {
        work->sound = fn_1_594(64, 1778);
        work->timer = 0;
        fn_1_140(obj, fn_1_778);
        return;
    }
}

void fn_1_778(OMOBJ *obj)
{
    s16 i;
    M621Round *work = obj->data;
    s16 winnerCount;
    s16 bestScore;
    M621Player *winners[4];

    if (MgSeqModeGet() == MGSEQ_MODE_FADEIN) {
        work->timer++;
        if (work->timer == 220) {
            HuAudFXStop(work->sound);
        }
    }
    if (MgSeqModeGet() == MGSEQ_MODE_PREWIN) {
        if (!lbl_1_bss_8) {
            winnerCount = 0;
            bestScore = 0;
            for (i = 0; i < 4; i++) {
                if (lbl_1_bss_10[i]->score > 0) {
                    if (lbl_1_bss_10[i]->score > bestScore) {
                        winners[0] = lbl_1_bss_10[i];
                        winnerCount = 1;
                        bestScore = lbl_1_bss_10[i]->score;
                    } else if (bestScore == lbl_1_bss_10[i]->score) {
                        winners[winnerCount] = lbl_1_bss_10[i];
                        winnerCount++;
                    }
                }
            }
            for (i = 0; i < winnerCount; i++) {
                winners[i]->winnerF = TRUE;
                fn_1_52D4(winners[i]->playerNo, 10);
            }
            switch (winnerCount) {
                case 0:
                    MgSeqWinnerSet(CHARNO_NONE, CHARNO_NONE, CHARNO_NONE, CHARNO_NONE);
                    break;
                case 1:
                    MgSeqWinnerSet1(winners[0]->player->charNo);
                    break;
                case 2:
                    MgSeqWinnerSet2(winners[0]->player->charNo, winners[1]->player->charNo);
                    break;
                case 3:
                    MgSeqWinnerSet3(winners[0]->player->charNo, winners[1]->player->charNo,
                        winners[2]->player->charNo);
                    break;
                case 4:
                    MgSeqWinnerSet(winners[0]->player->charNo, winners[1]->player->charNo,
                        winners[2]->player->charNo, winners[3]->player->charNo);
                    break;
            }
        }
        work->timer = 0;
        fn_1_140(obj, fn_1_A5C);
        return;
    }
}

void fn_1_A5C(OMOBJ *obj)
{
    M621Round *work = obj->data;
    s32 i;

    work->timer++;
    if ((lbl_1_bss_C >= 4 && work->timer >= 60) || work->timer >= 180) {
        if (lbl_1_bss_8) {
            for (i = 0; i < 4; i++) {
                fn_1_5328(lbl_1_bss_10[i]->playerNo, lbl_1_bss_10[i]->score);
            }
            MgSeqModeSet(MGSEQ_MODE_FADEOUT);
        } else {
            MgSeqModeNext();
        }
        fn_1_140(obj, NULL);
        return;
    }
}

void fn_1_B4C(OMOBJ *obj)
{
    M621Camera *work = obj->data;

    Hu3DCameraCreate(HU3D_CAM0);
    Hu3DCameraPerspectiveSet(HU3D_CAM0, 45.0f, 20.0f, 15000.0f, 1.2f);
    Hu3DCameraViewportSet(HU3D_CAM0, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    work->motion[0] = Hu3DMotionCreate(HuDataSelHeapReadNum(DATANUM(DATA_m621, 25), HU_MEMNUM_OVL, HEAP_MODEL));
    work->motion[1] = Hu3DMotionCreate(HuDataSelHeapReadNum(DATANUM(DATA_m621, 26), HU_MEMNUM_OVL, HEAP_MODEL));
    work->cameraModel = Hu3DModelCameraCreate(work->motion[0], HU3D_CAM0);
    Hu3DMotionSpeedSet(work->cameraModel, 1.0f);
    Hu3DCameraMotionStart(work->cameraModel, HU3D_CAM0);
    work->state = 0;
    fn_1_140(obj, fn_1_C90);
}

void fn_1_C90(OMOBJ *obj)
{
    M621Camera *work = obj->data;

    if (MgSeqModeGet() == MGSEQ_MODE_MAIN) {
        work->state = 0;
        fn_1_140(obj, fn_1_CEC);
        return;
    }
}

void fn_1_CEC(OMOBJ *obj)
{
    M621Camera *work = obj->data;

    fn_1_140(obj, NULL);
}

void fn_1_D28(OMOBJ *obj)
{
    COL_ATTRPARAM attr;
    s32 i;
    M621Environment *work = obj->data;

    work->modelObj = omAddObjEx(lbl_1_bss_0, 32730, 5, 2, -1, NULL);
    for (i = 0; i < 5; i++) {
        if (lbl_1_data_3C[fn_1_5340()][i] != -1) {
            work->modelObj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
                lbl_1_data_3C[fn_1_5340()][i], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            work->modelObj->mdlId[i] = 0;
        }
    }
    for (i = 0; i < 2; i++) {
        if (lbl_1_data_64[fn_1_5340()][i] != -1) {
            work->modelObj->mtnId[i] = Hu3DJointMotion(work->modelObj->mdlId[i],
                HuDataSelHeapReadNum(lbl_1_data_64[fn_1_5340()][i], HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DMotionSet(work->modelObj->mdlId[i], work->modelObj->mtnId[i]);
        } else {
            work->modelObj->mtnId[i] = 0;
        }
    }
    for (i = 0; i < 4; i++) {
        Hu3DModelAttrSet(work->modelObj->mdlId[i], HU3D_MOTATTR_LOOP);
    }
    for (i = 4; i < 5; i++) {
        Hu3DModelDispOff(work->modelObj->mdlId[i]);
    }
    MgActorColMapInit(&work->modelObj->mdlId[4], 1, 40);
    MgActorColAttrParamGet(&attr, 1);
    attr.yDeviate = 0.0f;
    MgActorColAttrParamSet(&attr, 1);
    work->hookModel = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m621, 20),
        HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(work->hookModel, HU3D_MOTATTR_LOOP);
    Hu3DModelScaleSet(work->hookModel, 0.6f, 0.6f, 0.6f);
    work->hookMotion = Hu3DJointMotion(work->hookModel,
        HuDataSelHeapReadNum(DATANUM(DATA_m621, 36), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(work->hookModel, work->hookMotion);
    work->model = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m621, 19),
        HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(work->model, HU3D_MOTATTR_LOOP);
    Hu3DModelHookSet(work->model, "jango_mot", work->hookModel);
    fn_1_140(obj, NULL);
}

void fn_1_106C(M621Player *work, s32 motionNo, float blendTime, u32 attr)
{
    if (work->motionNo != motionNo) {
        work->motionNo = motionNo;
        CharMotionShiftSet(work->charNo, work->player->omObj->mtnId[motionNo],
            0.0f, blendTime, attr);
    }
}

int fn_1_10E8(HuVecF *src, HuVecF *dst)
{
    int result;

    if (PSVECSquareMag(src) < 0.000001) {
        dst->x = 0.01f * ((float)(u32)frandmod(20) - 10.0f);
        dst->z = 0.01f * ((float)(u32)frandmod(20) - 10.0f);
        dst->y = frandmod(1) != 0U ? 0.01f : -0.01f;
        PSVECNormalize(dst, dst);
        result = 0;
    } else {
        PSVECNormalize(src, dst);
        result = 1;
    }
    return result;
}

int fn_1_1240(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b)
{
    HuVecF delta;
    HuVecF up;
    HuVecF horizontal;
    M621Player *player;
    float dot;

    if (a->type != 0 || b->type < 5 || b->type >= 8 || b->paramB == 0) {
        return 0;
    }
    player = lbl_1_bss_10[a->paramB];
    PSVECSubtract(&b->point, &a->point, &delta);
    fn_1_10E8(&delta, &delta);
    up.x = 0.0f;
    up.y = 1.0f;
    up.z = 0.0f;
    dot = PSVECDotProduct(&up, &delta);
    if (dot < -cos(M_PI / 3.0)) {
        horizontal = delta;
        horizontal.y = 0.0f;
        if (fn_1_5350(PSVECSquareMag(&horizontal)) < 0.001) {
            horizontal.x = 1.0f;
        } else {
            fn_1_10E8(&horizontal, &horizontal);
        }
        PSVECScale(&horizontal, &horizontal, -50.0f);
        a->normPos = horizontal;
    }
    return 1;
}

void fn_1_13EC(OMOBJ *obj)
{
    M621Player *work = obj->data;
    MGACTOR_PARAM param = {0};
    HuVecF pos;
    HSF_OBJECT *hsf;
    HU3D_MODELID model;

    work->timer = 0;
    work->charNo = GwPlayerConf[work->playerNo].charNo;
    work->attackF = 0;
    work->score = 0;
    work->winnerF = 0;
    param.height = 150.0f;
    param.radius = 40.0f;
    param.param = work->playerNo;
    param.type = 0;
    param.attr = 0;
    param.correctHookParam = 0;
    param.narrowHook = fn_1_1240;
    param.correctHook = NULL;
    work->player = MgPlayerCreate(work->playerNo, &param, 8, HU3D_CAM0,
        MGPLAYER_ACTFLAG_WALK | MGPLAYER_ACTFLAG_JUMP | MGPLAYER_ACTFLAG_PUNCH
        | MGPLAYER_ACTFLAG_KICK | MGPLAYER_ACTFLAG_STUN, lbl_1_data_2B0);
    work->computerF = GwPlayerConf[work->playerNo].type;
    work->difficulty = GwPlayerConf[work->playerNo].comDif;
    work->cpuState = 0;
    work->target = NULL;
    work->cpuDelay = 0;
    work->cpuTimer = 0;
    work->delay = 0;
    MgPlayerComStkOn(work->player);
    Hu3DModelShadowSet(work->player->actor->mdlId);
    model = Hu3DModelCreate(HuDataSelHeapReadNum(
        lbl_1_data_8C[work->playerNo], HU_MEMNUM_OVL, HEAP_MODEL));
    hsf = Hu3DModelObjPtrGet(model, lbl_1_data_334[work->playerNo]);
    pos.x = hsf->mesh.base.pos.x;
    pos.y = hsf->mesh.base.pos.y;
    pos.z = hsf->mesh.base.pos.z;
    MgActorPosSet(work->player->actor, &pos);
    work->baseRotation = hsf->mesh.base.rot.y;
    MgActorRotYSet(work->player->actor, work->baseRotation);
    work->basePos = pos;
    work->scoreBox = MgScoreBoxCreateChar(90, 40, work->charNo);
    MgScoreBoxPosSet(work->scoreBox, lbl_1_data_344[work->playerNo].x,
        lbl_1_data_344[work->playerNo].y);
    MgScoreBoxDispSet(work->scoreBox, FALSE);
    work->scoreDisplay = MgScoreCreate(DATANUM(DATA_mgconst, 49), -1, 1);
    MgScoreMaxDigitSet(work->scoreDisplay, 2);
    MgScoreDispOff(work->scoreDisplay);
    MgScorePosSet(work->scoreDisplay, 10.0f + lbl_1_data_344[work->playerNo].x,
        lbl_1_data_344[work->playerNo].y);
    MgScoreColorSet(work->scoreDisplay, 255, 216, 21);
    MgScoreValueSet(work->scoreDisplay, 0);
    CharEffectLayerSet(5);
    work->timer = 0;
    fn_1_140(obj, fn_1_175C);
}

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

M621Target *fn_1_1C7C(void)
{
    s32 index;

    index = 3.0f * frandf();
    if (index >= 3) {
        index = 0;
    }
    if (lbl_1_bss_40[index]->state != 1) {
        return NULL;
    }
    return lbl_1_bss_40[index];
}

M621Target *fn_1_1D08(M621Player *work)
{
    HuVecF pos;
    HuVecF delta;
    s8 occupied[3];
    s32 i;
    M621Target *target;
    float nearestDistance;
    float distance;

    target = NULL;
    if (frandf() < 0.55f) {
        MgActorPosGet(work->player->actor, &pos);
        for (i = 0; i < 3; i++) {
            if (lbl_1_bss_40[i]->state == 1) {
                if (target == NULL) {
                    target = lbl_1_bss_40[i];
                    PSVECSubtract(&target->pos, &pos, &delta);
                    delta.y = 0.0f;
                    nearestDistance = PSVECSquareMag(&delta);
                } else {
                    PSVECSubtract(&lbl_1_bss_40[i]->pos, &pos, &delta);
                    delta.y = 0.0f;
                    distance = PSVECSquareMag(&delta);
                    if (distance < nearestDistance) {
                        target = lbl_1_bss_40[i];
                        nearestDistance = distance;
                    }
                }
            }
        }
    } else {
        for (i = 0; i < 3; i++) {
            occupied[i] = 0;
        }
        for (i = 0; i < 4; i++) {
            if (lbl_1_bss_10[i]->computerF && lbl_1_bss_10[i]->target != NULL
                && lbl_1_bss_10[i]->target->state == 1) {
                occupied[lbl_1_bss_10[i]->target->targetNo] = 1;
            }
        }
        for (i = 0; i < 3; i++) {
            if (!occupied[i] && lbl_1_bss_40[i]->state == 1) {
                target = lbl_1_bss_40[i];
                break;
            }
        }
    }
    return target;
}

s16 fn_1_1F9C(M621Player *work)
{
    switch (work->difficulty) {
    case 0:
        return (s32)(60.0f * frandf()) + 120;
    case 1:
        return (s32)(60.0f * frandf()) + 60;
    case 2:
        return (s32)(50.0f * frandf()) + 50;
    default:
        return 0;
    }
}

s16 fn_1_2074(M621Player *work)
{
    HuVecF targetPos;
    HuVecF playerPos;
    HuVecF delta;

    if (work->difficulty < 2) {
        return 0;
    }
    if (work->target == NULL || work->target->state != 1) {
        return 0;
    }
    MgActorPosGet(work->target->part[0].actor, &targetPos);
    MgActorPosGet(work->player->actor, &playerPos);
    PSVECSubtract(&targetPos, &playerPos, &delta);
    if (delta.y < 250.0f && delta.x * delta.x + delta.z * delta.z < 40000.0f) {
        return 1;
    }
    return 0;
}

void fn_1_215C(OMOBJ *obj)
{
    HuVecF partPos;
    HuVecF pos;
    HuVecF targetPos;
    HuVecF delta;
    HuVecF stick;
    float height;
    M621Player *work = obj->data;

    if (work->target == NULL || work->target->state != 1) {
        work->target = fn_1_1D08(work);
        work->cpuDelay = fn_1_1F9C(work) / 2;
        work->cpuTimer = 0;
    }
    if (work->target != NULL) {
        MgActorPosGet(work->player->actor, &pos);
        switch (work->cpuState) {
        case 0:
            targetPos = work->target->pos;
            PSVECSubtract(&targetPos, &pos, &delta);
            height = delta.y;
            PSVECNormalize(&delta, &stick);
            PSVECScale(&stick, &stick, 20.0f);
            stick.z = (s32)stick.z;
            stick.x = (s32)stick.x;
            PSVECScale(&stick, &stick, 4.0f);
            if (work->cpuDelay > 0) {
                work->cpuDelay--;
            }
            if (work->cpuDelay <= 0 && fn_1_2074(work)) {
                MgPlayerPadSet(work->player, stick.x, -stick.z, PAD_BUTTON_A, PAD_BUTTON_A);
                work->cpuState = 1;
                work->cpuTimer = 0;
                return;
            }
            if (PSVECSquareMag(&delta) < 14400.0f) {
                if (work->cpuDelay <= 0) {
                    MgPlayerPadSet(work->player, stick.x, -stick.z, PAD_BUTTON_B, PAD_BUTTON_B);
                    work->cpuDelay = fn_1_1F9C(work);
                } else {
                    MgPlayerPadSet(work->player, stick.x, -stick.z, 0, 0);
                }
                work->cpuTimer = 0;
                return;
            }
            if (work->player->actor->colGroundAttr & 256) {
                MgPlayerPadSet(work->player, stick.x, -stick.z, PAD_BUTTON_A, PAD_BUTTON_A);
                work->cpuState = 2;
                return;
            }
            MgPlayerPadSet(work->player, stick.x, -stick.z, 0, 0);
            work->cpuTimer++;
            if (work->cpuTimer > 40) {
                work->target = NULL;
                return;
            }
            break;
        case 1:
            MgActorPosGet(work->target->part[0].actor, &partPos);
            PSVECSubtract(&partPos, &pos, &delta);
            height = delta.y;
            PSVECNormalize(&delta, &delta);
            PSVECScale(&delta, &delta, 40.0f);
            if (work->player->actor->velY < 0.0f && height < 150.0f) {
                MgPlayerPadSet(work->player, delta.x, -delta.z, PAD_BUTTON_B, PAD_BUTTON_B);
                work->cpuState = 0;
                return;
            }
            MgPlayerPadSet(work->player, delta.x, -delta.z, 0, PAD_BUTTON_A);
            if (!MgPlayerModeAttrCheck(work->player, MGPLAYER_MODEATTR_AIR)) {
                work->cpuState = 0;
                return;
            }
            break;
        case 2:
            MgActorPosGet(work->target->part[0].actor, &partPos);
            PSVECSubtract(&partPos, &pos, &delta);
            height = delta.y;
            PSVECNormalize(&delta, &delta);
            PSVECScale(&delta, &delta, 80.0f);
            MgPlayerPadSet(work->player, delta.x, -delta.z, 0, PAD_BUTTON_A);
            if (!MgPlayerModeAttrCheck(work->player, MGPLAYER_MODEATTR_AIR)) {
                work->cpuState = 0;
            }
            break;
        }
    }
}

u32 fn_1_26A4(void)
{
    u32 mask;

    for (mask = 1U << 31; mask != 1; mask >>= 1) {
        if ((mask & lbl_1_data_2AC) != 0) {
            lbl_1_data_2AC &= ~mask;
            return mask;
        }
    }
    return 0;
}

void fn_1_270C(u32 mask)
{
    lbl_1_data_2AC |= mask;
}

s16 fn_1_272C(void)
{
    s16 count;
    u32 mask;

    count = 0;
    for (mask = 1U << 31; mask != 1; mask >>= 1) {
        if ((mask & lbl_1_data_2AC) != 0) {
            count++;
        }
    }
    return count > 23 ? 23 : count;
}

void fn_1_27A0(void)
{
    s32 i;

    lbl_1_bss_3E = Hu3DModelCreateData(DATANUM(DATA_m621, 12));
    lbl_1_bss_3C = Hu3DModelCreateData(DATANUM(DATA_m621, 13));
    lbl_1_bss_3A = Hu3DModelCreateData(DATANUM(DATA_m621, 14));
    lbl_1_bss_38 = Hu3DModelCreateData(lbl_1_data_74[fn_1_5340()][0]);
    lbl_1_bss_36 = Hu3DModelCreateData(lbl_1_data_74[fn_1_5340()][1]);
    lbl_1_bss_34 = Hu3DJointMotionData(lbl_1_bss_36, lbl_1_data_74[fn_1_5340()][2]);
    Hu3DModelDispOff(lbl_1_bss_3E);
    Hu3DModelDispOff(lbl_1_bss_3C);
    Hu3DModelDispOff(lbl_1_bss_38);
    Hu3DModelDispOff(lbl_1_bss_36);
    Hu3DModelDispOff(lbl_1_bss_3A);
    for (i = 0; i < 3; i++) {
        lbl_1_bss_40[i] = fn_1_A0(30, sizeof(M621Target), fn_1_29E8);
        lbl_1_bss_40[i]->targetNo = i;
        lbl_1_bss_40[i]->initialF = 1;
        lbl_1_bss_40[i]->model = Hu3DModelLink(lbl_1_bss_38);
    }
}

void fn_1_29E8(OMOBJ *obj)
{
    M621Target *target = obj->data;

    if (MgSeqModeGet() == MGSEQ_MODE_FADEIN) {
        target->state = 0;
        target->timer = 150;
        target->positionNo = -1;
        fn_1_140(obj, fn_1_3678);
        return;
    }
}

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

void fn_1_2E10(M621TargetPart *part)
{
    HuVecF pos;

    part->state = 5;
    MgActorPosGet(part->actor, &pos);
    part->effectModel = Hu3DModelLink(lbl_1_bss_36);
    part->timer = 5;
}

int fn_1_2E68(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b)
{
    M621Player *player;
    s32 index;
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

void fn_1_3084(M621Target *target)
{
    MGACTOR_PARAM param;
    HuVecF pos;
    s32 count;
    s32 i;
    float height;

    count = fn_1_272C();
    if (target->initialF) {
        target->partCount = (s32)(5.0f * frandf()) + 3;
        target->rotation = 0.0f;
    } else {
        if (count >= 7) {
            target->partCount = (s32)(5.0f * frandf()) + 3;
            if (target->partCount > 7) {
                target->partCount = 7;
            }
        } else {
            target->partCount = count;
        }
        target->rotation = 360.0f * frandf();
    }
    target->height = 90.0f * (target->partCount + 1);
    target->turnTimer = 30;
    param.height = 150.0f + (80.0f + 90.0f * (target->partCount - 2));
    param.radius = 130.0f;
    param.type = target->targetNo + 5;
    param.param = 0;
    param.narrowHook = fn_1_2FFC;
    param.correctHook = NULL;
    param.correctHookParam = 0;
    param.attr = COLBODY_ATTR_MESHCOL_OFF;
    target->actor = MgActorCreate(&param, -1);
    MgActorColMaskSet(target->actor, 1);
    pos = target->pos;
    pos.y -= target->height;
    MgActorPosSet(target->actor, &pos);
    param.height = 90.0f;
    param.radius = 60.0f;
    param.type = target->targetNo + 5;
    param.narrowHook = fn_1_2E68;
    param.correctHook = fn_1_3024;
    param.attr = COLBODY_ATTR_MESHCOL_OFF;
    target->part[0].model = Hu3DModelLink(lbl_1_bss_3E);
    target->part[0].mask = fn_1_26A4();
    target->part[0].velocity.x = target->part[0].velocity.y = target->part[0].velocity.z = 0.0f;
    Hu3DModelDispOn(target->part[0].model);
    Hu3DModelShadowSet(target->part[0].model);
    param.height = 150.0f;
    param.param = 1;
    param.correctHookParam = (int)&target->part[0];
    target->part[0].actor = MgActorCreate(&param, target->part[0].model);
    MgActorColMaskSet(target->part[0].actor, target->part[0].mask);
    target->headRotation = target->rotation;
    if (target->headRotation > 70.0f) {
        target->headRotation = 70.0f;
    } else if (target->headRotation < -70.0f) {
        target->headRotation = -70.0f;
    }
    MgActorRotYSet(target->part[0].actor, target->headRotation);
    pos = target->pos;
    pos.y += (80.0f + 90.0f * (target->partCount - 2)) - target->height;
    MgActorPosSet(target->part[0].actor, &pos);
    target->part[0].actor->gravity = 0.0f;
    target->part[0].state = 1;
    for (i = 1; i < target->partCount; i++) {
        target->part[i].model = Hu3DModelLink(lbl_1_bss_3C);
        target->part[i].mask = fn_1_26A4();
        target->part[i].velocity.x = target->part[i].velocity.y = target->part[i].velocity.z = 0.0f;
        Hu3DModelDispOn(target->part[i].model);
        Hu3DModelShadowSet(target->part[i].model);
        if (i == 1) {
            height = 80.0f;
        } else {
            height = 90.0f;
        }
        param.height = height;
        param.param = i + 1;
        param.correctHookParam = (int)&target->part[i];
        target->part[i].actor = MgActorCreate(&param, target->part[i].model);
        MgActorColMaskSet(target->part[i].actor, target->part[i].mask);
        pos = target->pos;
        pos.y += 90.0f * (target->partCount - i - 1) - target->height;
        MgActorPosSet(target->part[i].actor, &pos);
        target->part[i].actor->gravity = 0.0f;
        MgActorRotYSet(target->part[i].actor, target->rotation);
        target->part[i].state = 1;
    }
}

void fn_1_3678(OMOBJ *obj)
{
    HuVecF delta;
    HSF_OBJECT *hsf;
    s32 i;
    M621Target *target = obj->data;

    if (MgSeqModeGet() == MGSEQ_MODE_FINISH || MgSeqModeGet() == MGSEQ_MODE_PREWIN
        || MgSeqModeGet() == MGSEQ_MODE_WINNER) {
        fn_1_140(obj, NULL);
        return;
    }
    target->timer--;
    if ((u32)MgSeqTimerValueGet() <= (u32)(target->timer + 60)) {
        fn_1_140(obj, NULL);
        return;
    }
    if (target->timer < 30) {
        do {
            target->positionNo = 21.0f * frandf();
            hsf = Hu3DModelObjPtrGet(lbl_1_bss_3A, lbl_1_data_258[target->positionNo]);
            target->pos.x = hsf->mesh.base.pos.x;
            target->pos.y = hsf->mesh.base.pos.y;
            target->pos.z = hsf->mesh.base.pos.z;
            for (i = 0; i < 3; i++) {
                if (target->targetNo == lbl_1_bss_40[i]->targetNo
                    || lbl_1_bss_40[i]->positionNo < 0) {
                    continue;
                }
                PSVECSubtract(&target->pos, &lbl_1_bss_40[i]->pos, &delta);
                delta.y = 0.0f;
                if (PSVECSquareMag(&delta) < 62500.0f) {
                    target->positionNo = 22;
                    break;
                }
            }
        } while (target->positionNo >= 21);
        Hu3DModelPosSetV(target->model, &target->pos);
        Hu3DModelDispOn(target->model);
        Hu3DMotionTimeSet(target->model, 0.0f);
        Hu3DModelLayerSet(target->model, 1);
        if (target->initialF) {
            for (i = 0; i < 4; i++) {
                lbl_1_bss_10[i]->delay = 30;
                omVibrate(lbl_1_bss_10[i]->player->playerNo, 30, 4, 4);
            }
        }
        fn_1_140(obj, fn_1_390C);
        return;
    }
}

void fn_1_390C(OMOBJ *obj)
{
    s32 i;
    M621Target *target = obj->data;

    if (MgSeqModeGet() == MGSEQ_MODE_FINISH || MgSeqModeGet() == MGSEQ_MODE_PREWIN
        || MgSeqModeGet() == MGSEQ_MODE_WINNER) {
        fn_1_140(obj, NULL);
        return;
    }
    target->timer--;
    if (target->timer <= 0) {
        fn_1_3084(target);
        target->timer = 0;
        if (target->initialF) {
            for (i = 0; i < 4; i++) {
                lbl_1_bss_10[i]->delay = target->partCount * 6;
                omVibrate(lbl_1_bss_10[i]->player->playerNo, target->partCount * 6, 4, 4);
            }
            target->initialF = 0;
            if (lbl_1_data_37E) {
                fn_1_594(64, 1775);
                lbl_1_data_37E = 0;
            }
        } else {
            fn_1_420(&target->pos, 1775);
        }
        fn_1_140(obj, fn_1_3A78);
        return;
    }
}

void fn_1_3A78(OMOBJ *obj)
{
    HuVecF pos;
    float time;
    float height;
    s16 i;
    M621Target *target = obj->data;

    if (MgSeqModeGet() == MGSEQ_MODE_FINISH || MgSeqModeGet() == MGSEQ_MODE_PREWIN
        || MgSeqModeGet() == MGSEQ_MODE_WINNER) {
        target->timer = 0;
        fn_1_140(obj, fn_1_4CB0);
        return;
    }
    target->timer++;
    time = 1.0f - (float)target->timer / (target->partCount * 6);
    if (time < 0.0f) {
        time = 0.0f;
    }
    height = 25.0f * ((s16)(target->height * time) / 25);
    for (i = 0; i < target->partCount; i++) {
        pos = target->pos;
        if (i == 0) {
            pos.y += 1.0f + ((80.0f + 90.0f * (target->partCount - 2)) - height);
        } else {
            pos.y += 1.0f + (90.0f * (target->partCount - i - 1) - height);
        }
        MgActorPosSet(target->part[i].actor, &pos);
    }
    pos = target->pos;
    pos.y = 1.0f + (pos.y - height);
    MgActorPosSet(target->actor, &pos);
    if (time <= 0.0f) {
        for (i = 0; i < target->partCount; i++) {
            target->part[i].timer = 0;
            target->part[i].state = 2;
        }
        MgActorColAttrReset(target->part[target->partCount - 1].actor, COLBODY_ATTR_MESHCOL_OFF);
        MgActorColAttrReset(target->actor, COLBODY_ATTR_MESHCOL_OFF);
        target->state = 1;
        fn_1_140(obj, fn_1_3E18);
        return;
    }
}

void fn_1_3E18(OMOBJ *obj)
{
    MGACTOR_COLMAP_POLY poly;
    HuVecF pos;
    HuVecF belowPos;
    HuVecF push;
    HuVecF currentPos;
    HuVecF top;
    HuVecF bottom;
    float wobble;
    float maxHeight;
    float height;
    s16 i;
    s16 j;
    s16 standingCount;
    s16 inactiveCount;
    M621TargetPart *part;
    M621TargetPart *below;
    M621Target *target = obj->data;

    inactiveCount = 0;
    standingCount = 0;
    maxHeight = 0.0f;
    if (MgSeqModeGet() == MGSEQ_MODE_FINISH || MgSeqModeGet() == MGSEQ_MODE_PREWIN
        || MgSeqModeGet() == MGSEQ_MODE_WINNER) {
        target->timer = 0;
        fn_1_140(obj, fn_1_47B4);
        return;
    }
    if (Hu3DMotionEndCheck(target->model)) {
        Hu3DModelDispOff(target->model);
    }
    for (i = 0; i < target->partCount; i++) {
        M621TargetPart *currentPart = &target->part[i];
        switch (currentPart->state) {
        case 1:
            break;
        case 0:
            inactiveCount++;
            break;
        case 2:
            standingCount++;
            break;
        }
    }
    if (inactiveCount == target->partCount) {
        target->state = 2;
        if ((u32)MgSeqTimerValueGet() / 60 <= 1) {
            fn_1_140(obj, NULL);
            return;
        }
        target->timer = 30.0f + 90.0f * frandf();
        target->positionNo = -1;
        fn_1_140(obj, fn_1_3678);
        return;
    }
    if (MgSeqModeGet() == MGSEQ_MODE_MAIN) {
        target->turnTimer--;
        if (target->turnTimer <= 0) {
            target->turnRandom = frandf();
            target->turnTimer = 30;
        }
        if (target->turnRandom < 0.4f) {
            target->rotation -= 0.3f;
            if (target->rotation < -180.0f) {
                target->rotation = 360.0f + target->rotation;
            }
            target->headRotation -= 0.3f;
            if (target->headRotation < -70.0f) {
                target->headRotation = -70.0f;
            }
        } else if (target->turnRandom > 0.6f) {
            target->rotation += 0.3f;
            if (target->rotation > 180.0f) {
                target->rotation -= 360.0f;
            }
            target->headRotation += 0.3f;
            if (target->headRotation > 70.0f) {
                target->headRotation = 70.0f;
            }
        }
        push.x = push.y = 0.0f;
        push.z = 1.0f;
        if (target->part[0].state == 2) {
            MgActorRotYSet(target->part[0].actor, target->headRotation);
        }
        for (i = 1; i < target->partCount; i++) {
            if (target->part[i].state == 2) {
                MgActorRotYSet(target->part[i].actor, target->rotation);
            }
        }
    } else {
        push.x = push.y = push.z = 0.0f;
    }
    if (target->actor != NULL) {
        MgActorPosGet(target->actor, &target->pos);
        MgActorRotYSet(target->actor, target->rotation);
        MgActorPushSet(target->actor, &push);
    }
    for (i = 0; i < target->partCount; i++) {
        part = &target->part[i];
        below = NULL;
        for (j = i + 1; j < target->partCount; j++) {
            if (target->part[j].state == 2) {
                below = &target->part[j];
                break;
            }
        }
        switch (part->state) {
        case 2:
            part->timer++;
            standingCount--;
            wobble = 10.0 * sin((M_PI * (((360.0f * part->timer) / 120.0f)
                + (120.0f * standingCount))) / 180.0);
            if (below != NULL) {
                part->velocity.y -= 0.7f;
                MgActorPosGet(part->actor, &pos);
                PSVECAdd(&pos, &part->velocity, &pos);
                MgActorPosGet(below->actor, &belowPos);
                if (i == 0 && 80.0f + belowPos.y >= pos.y) {
                    pos.y = 80.0f + belowPos.y;
                    part->velocity.y = 0.0f;
                } else if (i != 0 && 90.0f + belowPos.y >= pos.y) {
                    pos.y = 90.0f + belowPos.y;
                    part->velocity.y = 0.0f;
                }
                pos.x = target->pos.x + wobble;
                pos.z = target->pos.z;
                MgActorPosSetRaw(part->actor, &pos);
                height = (pos.y - target->pos.y) + (i == 0 ? 150.0f : 90.0f);
            } else {
                MgActorPosGet(part->actor, &currentPos);
                top = bottom = currentPos;
                top.y = 10000.0f;
                bottom.y = -10000.0f;
                if (MgActorColMapPolyGet(&top, &bottom, ~0U, &poly)) {
                    if (currentPos.y < poly.pos.y) {
                        currentPos.y = poly.pos.y;
                        MgActorPosSet(part->actor, &currentPos);
                    } else if (currentPos.y > poly.pos.y) {
                        MgActorColAttrReset(part->actor, COLBODY_ATTR_MESHCOL_OFF);
                        part->actor->gravity = 150.0f;
                        MgActorPosGet(part->actor, &pos);
                        pos.x = target->pos.x + wobble;
                        pos.z = target->pos.z;
                        MgActorPosSetRaw(part->actor, &pos);
                    }
                }
                height = i == 0 ? 150.0f : 90.0f;
            }
            if (height > maxHeight) {
                maxHeight = height;
            }
            break;
        case 3:
            part->timer++;
            if (part->timer % 2 == 0) {
                Hu3DModelDispOn(part->model);
            } else {
                Hu3DModelDispOff(part->model);
            }
            part->velocity.y -= 0.7f;
            MgActorPosGet(part->actor, &pos);
            PSVECAdd(&pos, &part->velocity, &pos);
            MgActorPosSetRaw(part->actor, &pos);
            break;
        case 4:
            part->timer--;
            if (part->timer <= 0) {
                MgActorPosGet(part->actor, &pos);
                fn_1_420(&pos, 1776);
                pos.y += 50.0f;
                pos.z += 55.0f;
                fn_1_51F4(part->effectNo, &pos);
                fn_1_2E10(part);
            }
            break;
        case 5:
            part->timer--;
            if (part->timer == 0) {
                MgActorKill(part->actor);
                Hu3DModelKill(part->model);
                part->state = 0;
                fn_1_270C(part->mask);
            }
            break;
        }
    }
    if (target->actor != NULL) {
        ColBodyGetSafe(target->actor->no)->param.height = maxHeight;
    }
}

void fn_1_47B4(OMOBJ *obj)
{
    HuVecF pos;
    HuVecF belowPos;
    float wobble;
    s16 i;
    s16 j;
    s16 inactiveCount;
    s16 standingCount;
    M621TargetPart *part;
    M621TargetPart *below;
    M621Target *target = obj->data;

    inactiveCount = 0;
    standingCount = 0;
    for (i = 0; i < target->partCount; i++) {
        M621TargetPart *currentPart = &target->part[i];
        switch (currentPart->state) {
        case 0:
            inactiveCount++;
            break;
        }
    }
    if (inactiveCount == target->partCount) {
        fn_1_140(obj, NULL);
        return;
    }
    if (target->actor != NULL) {
        MgActorPushSet(target->actor, &lbl_1_data_30);
    }
    if (target->timer == 0) {
        Hu3DModelPosSetV(target->model, &target->pos);
        Hu3DModelDispOn(target->model);
        Hu3DMotionTimeSet(target->model, 0.0f);
        Hu3DModelLayerSet(target->model, 1);
    }
    target->timer++;
    if (target->timer >= 30) {
        target->timer = 0;
        if (lbl_1_data_380) {
            fn_1_594(64, 1777);
            lbl_1_data_380 = 0;
        }
        fn_1_140(obj, fn_1_4CB0);
        return;
    }
    for (i = 0; i < target->partCount; i++) {
        part = &target->part[i];
        below = NULL;
        for (j = i + 1; j < target->partCount; j++) {
            if (target->part[j].state == 2) {
                below = &target->part[j];
                break;
            }
        }
        switch (part->state) {
        case 2:
            part->timer++;
            standingCount--;
            wobble = 10.0 * sin((M_PI * (((360.0f * part->timer) / 120.0f)
                + (120.0f * standingCount))) / 180.0);
            if (below != NULL) {
                part->velocity.y -= 0.7f;
                MgActorPosGet(part->actor, &pos);
                PSVECAdd(&pos, &part->velocity, &pos);
                MgActorPosGet(below->actor, &belowPos);
                if (i == 0 && 80.0f + belowPos.y >= pos.y) {
                    pos.y = 80.0f + belowPos.y;
                    part->velocity.y = 0.0f;
                } else if (i != 0 && 90.0f + belowPos.y >= pos.y) {
                    pos.y = 90.0f + belowPos.y;
                    part->velocity.y = 0.0f;
                }
                pos.x = target->pos.x + wobble;
                pos.z = target->pos.z;
                MgActorPosSetRaw(part->actor, &pos);
            } else {
                MgActorColAttrReset(part->actor, COLBODY_ATTR_MESHCOL_OFF);
                part->actor->gravity = 150.0f;
            }
            break;
        case 3:
            part->timer++;
            if (part->timer % 2 == 0) {
                Hu3DModelDispOn(part->model);
            } else {
                Hu3DModelDispOff(part->model);
            }
            part->velocity.y -= 0.7f;
            MgActorPosGet(part->actor, &pos);
            PSVECAdd(&pos, &part->velocity, &pos);
            MgActorPosSetRaw(part->actor, &pos);
            break;
        case 4:
            fn_1_2E10(part);
            break;
        case 5:
            part->timer--;
            if (part->timer == 0) {
                MgActorKill(part->actor);
                Hu3DModelKill(part->model);
                part->state = 0;
                fn_1_270C(part->mask);
            }
            break;
        }
    }
}

void fn_1_4CB0(OMOBJ *obj)
{
    HuVecF pos;
    HuVecF belowPos;
    s16 i;
    s16 j;
    s16 inactiveCount;
    /* This countdown remains in the cleanup phase after its wobble is gone. */
    s16 standingCount;
    M621TargetPart *part;
    M621TargetPart *below;
    M621Target *target = obj->data;

    inactiveCount = 0;
    standingCount = 0;
    for (i = 0; i < target->partCount; i++) {
        M621TargetPart *currentPart = &target->part[i];
        switch (currentPart->state) {
        case 0:
            inactiveCount++;
            break;
        }
    }
    if (inactiveCount == target->partCount) {
        fn_1_140(obj, NULL);
        return;
    }
    for (i = 0; i < target->partCount; i++) {
        part = &target->part[i];
        below = NULL;
        for (j = i + 1; j < target->partCount; j++) {
            if (target->part[j].state == 2) {
                below = &target->part[j];
                break;
            }
        }
        switch (part->state) {
        case 2:
            part->timer++;
            standingCount--;
            MgActorColAttrSet(part->actor, COLBODY_ATTR_MESHCOL_OFF);
            part->actor->gravity = 150.0f;
            if (below != NULL) {
                part->velocity.y -= 0.7f;
                MgActorPosGet(part->actor, &pos);
                PSVECAdd(&pos, &part->velocity, &pos);
                MgActorPosGet(below->actor, &belowPos);
                if (i == 0 && 80.0f + belowPos.y >= pos.y) {
                    pos.y = 80.0f + belowPos.y;
                    part->velocity.y = 0.0f;
                } else if (i != 0 && 90.0f + belowPos.y >= pos.y) {
                    pos.y = 90.0f + belowPos.y;
                    part->velocity.y = 0.0f;
                }
                pos.x = target->pos.x;
                pos.z = target->pos.z;
                MgActorPosSetRaw(part->actor, &pos);
            } else {
                part->actor->gravity = 150.0f;
                MgActorPosGet(part->actor, &pos);
            }
            if (pos.y < -200.0f) {
                fn_1_2E10(part);
            }
            break;
        case 3:
            part->timer++;
            if (part->timer % 2 == 0) {
                Hu3DModelDispOn(part->model);
            } else {
                Hu3DModelDispOff(part->model);
            }
            part->velocity.y -= 0.7f;
            MgActorPosGet(part->actor, &pos);
            PSVECAdd(&pos, &part->velocity, &pos);
            MgActorPosSetRaw(part->actor, &pos);
            break;
        case 4:
            fn_1_2E10(part);
            break;
        case 5:
            part->timer--;
            if (part->timer == 0) {
                MgActorKill(part->actor);
                Hu3DModelKill(part->model);
                part->state = 0;
                fn_1_270C(part->mask);
            }
            break;
        }
    }
}

void fn_1_5054(OMOBJ *obj)
{
    s32 i;
    M621Effect *work = obj->data;

    for (i = 0; i < 4; i++) {
        lbl_1_bss_2C[i] = Hu3DModelCreateData(lbl_1_data_9C[i][0]);
        lbl_1_bss_24[i] = Hu3DJointMotionData(lbl_1_bss_2C[i], lbl_1_data_9C[i][1]);
        Hu3DModelDispOff(lbl_1_bss_2C[i]);
    }
    memset(work, 0, 21 * sizeof(M621Effect));
    fn_1_140(obj, fn_1_5168);
}

void fn_1_5168(OMOBJ *obj)
{
    M621Effect *effect;
    s32 i;
    M621Effect *work = obj->data;

    for (i = 0; i < 21; i++) {
        effect = &work[i];
        if (effect->active == 1 && Hu3DMotionEndCheck(effect->model)) {
            Hu3DModelKill(effect->model);
            effect->active = 0;
        }
    }
}

void fn_1_51F4(s16 effectNo, HuVecF *pos)
{
    M621Effect *effect;
    s32 i;

    for (i = 0; i < 21; i++) {
        effect = &lbl_1_bss_20[i];
        if (effect->active == 0) {
            effect->model = Hu3DModelLink(lbl_1_bss_2C[effectNo]);
            Hu3DMotionSet(effect->model, lbl_1_bss_24[effectNo]);
            Hu3DModelDispOn(effect->model);
            Hu3DModelPosSetV(effect->model, pos);
            Hu3DModelLayerSet(effect->model, 2);
            effect->active = 1;
            break;
        }
    }
}

void fn_1_52D4(s32 playerNo, s16 coins)
{
    if (!_CheckFlag(FLAG_MG_PRACTICE)) {
        GwPlayer[playerNo].mgCoinBonus = coins;
    }
}

void fn_1_5328(s32 playerNo, s32 score)
{
    GwPlayer[playerNo].mgScore = score;
}

s16 fn_1_5340(void)
{
    return GwMgNightF;
}

float fn_1_5350(float value)
{
    return fn_1_537C(value);
}

double fn_1_537C(double value)
{
    return __fabs(value);
}

s16 lbl_1_data_37E = 1;
s16 lbl_1_data_380 = 1;
