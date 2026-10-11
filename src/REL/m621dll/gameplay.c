/* Player, target-stack, camera, sound and effect behavior for the minigame. */
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

#define M621_SE_TARGET_APPEAR 1775
#define M621_SE_SEGMENT_HIT 1776
#define M621_SE_TARGET_COLLAPSE 1777
#define M621_SE_ROUND_START 1778
#define M621_AVAILABLE_COLLISION_MASK 0xFFFFFFFEU

struct M621Effect_s {
    s16 active; /* Nonzero while the linked effect model is playing. */
    HU3D_MODELID model; /* Linked model instance for this effect. */
};

void fn_1_5168(OMOBJ *obj);

struct M621Environment_s {
    u8 reservedBytes[4]; /* Unused bytes; their game meaning is unknown. */
    OMOBJ *modelObj; /* Scene model object containing the stage assets. */
    HU3D_MODELID hookModel; /* Hook model attached to the character model. */
    HU3D_MOTIONID hookMotion; /* Hook animation motion. */
    HU3D_MODELID model; /* Character model receiving the hook model. */
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
u32 lbl_1_data_2AC = M621_AVAILABLE_COLLISION_MASK;

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

/* Project a world position to the screen and play a sound with position-based volume and pan. */
s32 fn_1_420(HuVecF *worldPosition, s16 soundId)
{
    HuVecF screenPos;
    s16 pan;
    s16 volume;

    Hu3D3Dto2D(worldPosition, HU3D_CAM0, &screenPos);
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

/* Play a sound at full volume with the requested stereo pan. */
s32 fn_1_594(s16 pan, s16 soundId)
{
    return HuAudFXPlayVolPan(soundId, 127, pan);
}

/* Stop the audio effect identified by its active sound handle. */
void fn_1_5C8(s32 soundHandle)
{
    HuAudFXStop(soundHandle);
}

/* Round-controller init callback, created by sequence init: initialize state and create camera,
 * environment, player and effect objects. */
void fn_1_5F0(OMOBJ *obj)
{
    s32 playerIndex;

    /* This computes and discards a random value from the current tick; the shared random seed is
     * unchanged. */
    frandom(OSGetTick());
    lbl_1_bss_8 = _CheckFlag(FLAG_INST_DECA);
    fn_1_A0(10, sizeof(M621Camera), fn_1_B4C);
    lbl_1_bss_4C = fn_1_A0(10, 16, fn_1_D28);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        lbl_1_bss_10[playerIndex] = fn_1_A0(20, sizeof(M621Player), fn_1_13EC);
        lbl_1_bss_10[playerIndex]->playerNo = playerIndex;
    }
    lbl_1_bss_C = 0;
    fn_1_27A0();
    lbl_1_bss_20 = fn_1_A0(10, 84, fn_1_5054);
    fn_1_140(obj, fn_1_70C);
}

/* Round update: start the round sound on fade-in, then hand updates to the round-phase callback. */
void fn_1_70C(OMOBJ *obj)
{
    M621Round *round = obj->data;

    if (MgSeqModeGet() == MGSEQ_MODE_FADEIN) {
        round->sound = fn_1_594(64, M621_SE_ROUND_START);
        round->timer = 0;
        fn_1_140(obj, fn_1_778);
        return;
    }
}

/* During fade-in, stop the round-start sound at update 220. In pre-winner mode, select positive-
* score leaders and ties unless instruction/demo mode is active; the coin helper separately
* suppresses practice bonuses. */
void fn_1_778(OMOBJ *obj)
{
    s16 playerIndex;
    M621Round *round = obj->data;
    s16 winnerCount;
    s16 bestScore;
    M621Player *winners[4];

    if (MgSeqModeGet() == MGSEQ_MODE_FADEIN) {
        round->timer++;
        if (round->timer == 220) {
            HuAudFXStop(round->sound);
        }
    }
    if (MgSeqModeGet() == MGSEQ_MODE_PREWIN) {
        if (!lbl_1_bss_8) {
            winnerCount = 0;
            bestScore = 0;
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                if (lbl_1_bss_10[playerIndex]->score > 0) {
                    if (lbl_1_bss_10[playerIndex]->score > bestScore) {
                        winners[0] = lbl_1_bss_10[playerIndex];
                        winnerCount = 1;
                        bestScore = lbl_1_bss_10[playerIndex]->score;
                    } else if (bestScore == lbl_1_bss_10[playerIndex]->score) {
                        winners[winnerCount] = lbl_1_bss_10[playerIndex];
                        winnerCount++;
                    }
                }
            }
            for (playerIndex = 0; playerIndex < winnerCount; playerIndex++) {
                winners[playerIndex]->winnerF = TRUE;
                fn_1_52D4(winners[playerIndex]->playerNo, 10);
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
        round->timer = 0;
        fn_1_140(obj, fn_1_A5C);
        return;
    }
}

/* Wait at least 60 updates for all players to face their starting angle, or time out at 180;
 * instruction/demo mode stores all four scores and fades out, otherwise the sequence advances. */
void fn_1_A5C(OMOBJ *obj)
{
    M621Round *round = obj->data;
    s32 playerIndex;

    round->timer++;
    if ((lbl_1_bss_C >= 4 && round->timer >= 60) || round->timer >= 180) {
        if (lbl_1_bss_8) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                fn_1_5328(lbl_1_bss_10[playerIndex]->playerNo, lbl_1_bss_10[playerIndex]->score);
            }
            MgSeqModeSet(MGSEQ_MODE_FADEOUT);
        } else {
            MgSeqModeNext();
        }
        fn_1_140(obj, NULL);
        return;
    }
}

/* Camera-object init callback, created during round setup: load both camera motions before enabling
 * the phase callback. */
void fn_1_B4C(OMOBJ *obj)
{
    M621Camera *camera = obj->data;

    Hu3DCameraCreate(HU3D_CAM0);
    Hu3DCameraPerspectiveSet(HU3D_CAM0, 45.0f, 20.0f, 15000.0f, 1.2f);
    Hu3DCameraViewportSet(HU3D_CAM0, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    camera->motion[0] =
        Hu3DMotionCreate(HuDataSelHeapReadNum(DATANUM(DATA_m621, 25), HU_MEMNUM_OVL, HEAP_MODEL));
    camera->motion[1] =
        Hu3DMotionCreate(HuDataSelHeapReadNum(DATANUM(DATA_m621, 26), HU_MEMNUM_OVL, HEAP_MODEL));
    camera->cameraModel = Hu3DModelCameraCreate(camera->motion[0], HU3D_CAM0);
    Hu3DMotionSpeedSet(camera->cameraModel, 1.0f);
    Hu3DCameraMotionStart(camera->cameraModel, HU3D_CAM0);
    camera->unusedState = 0;
    fn_1_140(obj, fn_1_C90);
}

/* Wait until the sequence enters main mode, then advance to the callback that disables
 * camera-object updates. */
void fn_1_C90(OMOBJ *obj)
{
    M621Camera *camera = obj->data;

    if (MgSeqModeGet() == MGSEQ_MODE_MAIN) {
        camera->unusedState = 0;
        fn_1_140(obj, fn_1_CEC);
        return;
    }
}

/* Disable the camera object's phase callback after main mode begins. */
void fn_1_CEC(OMOBJ *obj)
{
    M621Camera *camera = obj->data;

    fn_1_140(obj, NULL);
}

/* Environment-object init callback, created during round setup: load scene models, motions,
 * collision and hook assets. */
void fn_1_D28(OMOBJ *obj)
{
    COL_ATTRPARAM attr;
    s32 modelIndex;
    M621Environment *environment = obj->data;

    environment->modelObj = omAddObjEx(lbl_1_bss_0, 32730, 5, 2, -1, NULL);
    for (modelIndex = 0; modelIndex < 5; modelIndex++) {
        if (lbl_1_data_3C[fn_1_5340()][modelIndex] != -1) {
            environment->modelObj->mdlId[modelIndex] = Hu3DModelCreate(HuDataSelHeapReadNum(
                lbl_1_data_3C[fn_1_5340()][modelIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            /* An absent scene asset uses model ID 0; the loop-attribute pass below still applies to
             * that ID. */
            environment->modelObj->mdlId[modelIndex] = 0;
        }
    }
    for (modelIndex = 0; modelIndex < 2; modelIndex++) {
        if (lbl_1_data_64[fn_1_5340()][modelIndex] != -1) {
            environment->modelObj->mtnId[modelIndex] =
                Hu3DJointMotion(environment->modelObj->mdlId[modelIndex],
                                HuDataSelHeapReadNum(lbl_1_data_64[fn_1_5340()][modelIndex],
                                                     HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DMotionSet(environment->modelObj->mdlId[modelIndex],
                          environment->modelObj->mtnId[modelIndex]);
        } else {
            environment->modelObj->mtnId[modelIndex] = 0;
        }
    }
    for (modelIndex = 0; modelIndex < 4; modelIndex++) {
        Hu3DModelAttrSet(environment->modelObj->mdlId[modelIndex], HU3D_MOTATTR_LOOP);
    }
    for (modelIndex = 4; modelIndex < 5; modelIndex++) {
        Hu3DModelDispOff(environment->modelObj->mdlId[modelIndex]);
    }
    MgActorColMapInit(&environment->modelObj->mdlId[4], 1, 40);
    MgActorColAttrParamGet(&attr, 1);
    attr.yDeviate = 0.0f;
    MgActorColAttrParamSet(&attr, 1);
    environment->hookModel = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m621, 20),
        HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(environment->hookModel, HU3D_MOTATTR_LOOP);
    Hu3DModelScaleSet(environment->hookModel, 0.6f, 0.6f, 0.6f);
    environment->hookMotion = Hu3DJointMotion(environment->hookModel,
        HuDataSelHeapReadNum(DATANUM(DATA_m621, 36), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(environment->hookModel, environment->hookMotion);
    environment->model = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m621, 19),
        HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(environment->model, HU3D_MOTATTR_LOOP);
    Hu3DModelHookSet(environment->model, "jango_mot", environment->hookModel);
    fn_1_140(obj, NULL);
}

/* Request a character motion when it differs from this helper's cached request; gameplay actor
 * updates select their own motions separately. */
void fn_1_106C(M621Player *player, s32 motionNo, float blendTime, u32 motionAttr)
{
    if (player->motionNo != motionNo) {
        player->motionNo = motionNo;
        CharMotionShiftSet(player->charNo, player->player->omObj->mtnId[motionNo],
            0.0f, blendTime, motionAttr);
    }
}

/* Normalize a direction for collision response, generating a random fallback when its squared
* magnitude is below 0.000001. The return value reports whether it met that threshold; callers use
* the output and ignore the result. */
int fn_1_10E8(HuVecF *sourceDirection, HuVecF *normalizedDirection)
{
    int result;

    if (PSVECSquareMag(sourceDirection) < 0.000001) {
        normalizedDirection->x = 0.01f * ((float)(u32)frandmod(20) - 10.0f);
        normalizedDirection->z = 0.01f * ((float)(u32)frandmod(20) - 10.0f);
        /* Modulus 1 is always zero, so the fallback's Y component is -0.01 before normalization. */
        normalizedDirection->y = frandmod(1) != 0U ? 0.01f : -0.01f;
        PSVECNormalize(normalizedDirection, normalizedDirection);
        result = 0;
    } else {
        PSVECNormalize(sourceDirection, normalizedDirection);
        result = 1;
    }
    return result;
}

/* Narrow-collision hook: ignore unrelated contacts and adjust the player response for a collision
 * from above. */
int fn_1_1240(COL_NARROW_PARAM *firstCollider, COL_NARROW_PARAM *secondCollider)
{
    HuVecF delta;
    HuVecF up;
    HuVecF horizontal;
    M621Player *player;
    float dot;

    if (firstCollider->type != 0 || secondCollider->type < 5 || secondCollider->type >= 8 ||
        secondCollider->paramB == 0) {
        return 0;
    }
    player = lbl_1_bss_10[firstCollider->paramB];
    PSVECSubtract(&secondCollider->point, &firstCollider->point, &delta);
    fn_1_10E8(&delta, &delta); /* Replace delta in place with its unit direction. */
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
        firstCollider->normPos = horizontal;
    }
    return 1;
}

/* Player-object init callback, created once per player during round setup: create its actor and
 * score displays, then begin intro updates. */
void fn_1_13EC(OMOBJ *obj)
{
    M621Player *player = obj->data;
    MGACTOR_PARAM param = {0};
    HuVecF pos;
    HSF_OBJECT *hsf;
    HU3D_MODELID model;

    player->timer = 0;
    player->charNo = GwPlayerConf[player->playerNo].charNo;
    player->attackF = 0;
    player->score = 0;
    player->winnerF = 0;
    param.height = 150.0f;
    param.radius = 40.0f;
    param.param = player->playerNo;
    param.type = 0;
    param.attr = 0;
    param.correctHookParam = 0;
    param.narrowHook = fn_1_1240;
    param.correctHook = NULL;
    /* The player constructor replaces the landing-motion entry in the shared list with its
     * alternate motion before loading the character. */
    player->player = MgPlayerCreate(player->playerNo, &param, 8, HU3D_CAM0,
        MGPLAYER_ACTFLAG_WALK | MGPLAYER_ACTFLAG_JUMP | MGPLAYER_ACTFLAG_PUNCH
        | MGPLAYER_ACTFLAG_KICK | MGPLAYER_ACTFLAG_STUN, lbl_1_data_2B0);
    player->computerF = GwPlayerConf[player->playerNo].type;
    player->difficulty = GwPlayerConf[player->playerNo].comDif;
    player->cpuState = 0;
    player->target = NULL;
    player->cpuDelay = 0;
    player->cpuTimer = 0;
    player->vibrationDelay = 0;
    MgPlayerComStkOn(player->player);
    Hu3DModelShadowSet(player->player->actor->mdlId);
    model = Hu3DModelCreate(HuDataSelHeapReadNum(
        lbl_1_data_8C[player->playerNo], HU_MEMNUM_OVL, HEAP_MODEL));
    hsf = Hu3DModelObjPtrGet(model, lbl_1_data_334[player->playerNo]);
    pos.x = hsf->mesh.base.pos.x;
    pos.y = hsf->mesh.base.pos.y;
    pos.z = hsf->mesh.base.pos.z;
    MgActorPosSet(player->player->actor, &pos);
    player->baseRotation = hsf->mesh.base.rot.y;
    MgActorRotYSet(player->player->actor, player->baseRotation);
    player->basePos = pos;
    player->scoreBox = MgScoreBoxCreateChar(90, 40, player->charNo);
    MgScoreBoxPosSet(player->scoreBox, lbl_1_data_344[player->playerNo].x,
        lbl_1_data_344[player->playerNo].y);
    MgScoreBoxDispSet(player->scoreBox, FALSE);
    player->scoreDisplay = MgScoreCreate(DATANUM(DATA_mgconst, 49), -1, 1);
    MgScoreMaxDigitSet(player->scoreDisplay, 2);
    MgScoreDispOff(player->scoreDisplay);
    MgScorePosSet(player->scoreDisplay, 10.0f + lbl_1_data_344[player->playerNo].x,
        lbl_1_data_344[player->playerNo].y);
    MgScoreColorSet(player->scoreDisplay, 255, 216, 21);
    MgScoreValueSet(player->scoreDisplay, 0);
    CharEffectLayerSet(5);
    player->timer = 0;
    fn_1_140(obj, fn_1_175C);
}

/* Run the player intro animation until main mode, then show score displays and enable normal
 * control. */
void fn_1_175C(OMOBJ *obj)
{
    M621Player *player = obj->data;

    player->vibrationDelay--;
    player->timer++;
    if (player->timer == 155) {
        fn_1_106C(player, 7, 1.0f, 0);
    } else if (player->timer == 205) {
        fn_1_106C(player, 0, 5.0f, HU3D_MOTATTR_LOOP);
    }
    if (MgSeqModeGet() == MGSEQ_MODE_MAIN) {
        MgScoreBoxDispSet(player->scoreBox, TRUE);
        MgScoreDispOn(player->scoreDisplay);
        MgPlayerComStkOff(player->player);
        player->timer = 0;
        fn_1_140(obj, fn_1_1844);
        return;
    }
}

/* Update player attacks, computer control and ground placement; stop input when the sequence
 * finishes. */
void fn_1_1844(OMOBJ *obj)
{
    M621Player *player = obj->data;
    MGACTOR_COLMAP_POLY poly;
    HuVecF pos;
    HuVecF from;
    HuVecF to;

    player->vibrationDelay--;
    if (player->player->mode == MGPLAYER_MODE_PUNCH || player->player->mode == MGPLAYER_MODE_KICK) {
        if (!player->attackF) {
            player->attackF = TRUE;
        }
    } else {
        player->attackF = FALSE;
        player->unusedState = 0;
        if (player->computerF) {
            fn_1_215C(obj);
        }
    }
    MgActorPosGet(player->player->actor, &pos);
    from = to = pos;
    from.y = 10000.0f;
    to.y = -10000.0f;
    if (MgActorColMapPolyGet(&from, &to, ~0U, &poly) && pos.y < poly.pos.y) {
        pos.y = poly.pos.y;
        MgActorPosSet(player->player->actor, &pos);
    }
    if (MgSeqModeGet() == MGSEQ_MODE_FINISH) {
        MgPlayerPadSet(player->player, 0, 0, 0, 0);
        player->timer = 3;
        fn_1_140(obj, fn_1_19C4);
        return;
    }
}

/* After FINISH, clear gameplay input while preserving Start and Z; wait for the engine's idle
 * motion, then freeze stick-driven movement and begin facing alignment. */
void fn_1_19C4(OMOBJ *obj)
{
    M621Player *player = obj->data;

    MgPlayerPadSet(player->player, 0, 0, 0, 0);
    if (player->player->motNo == 0) {
        MgPlayerComStkOn(player->player);
        fn_1_140(obj, fn_1_1A3C);
        return;
    }
}

/* After the player reaches idle motion, rotate toward the starting angle and count it once
 * aligned; the saved starting position is not restored. */
void fn_1_1A3C(OMOBJ *obj)
{
    M621Player *player = obj->data;
    float rotation;

    MgPlayerPadSet(player->player, 0, 0, 0, 0);
    MgActorRotYGet(player->player->actor, &rotation);
    if (rotation == player->baseRotation) {
        lbl_1_bss_C++;
        fn_1_140(obj, fn_1_1B5C);
        return;
    }
    if (rotation < player->baseRotation) {
        rotation += 6.0f;
        if (rotation > player->baseRotation) {
            rotation = player->baseRotation;
        }
    } else {
        rotation -= 6.0f;
        if (rotation < player->baseRotation) {
            rotation = player->baseRotation;
        }
    }
    MgActorRotYSet(player->player->actor, rotation);
}

/* Hold the aligned player until winner mode; disable its update callback early when the
 * instruction/demo flag is set. */
void fn_1_1B5C(OMOBJ *obj)
{
    M621Player *player = obj->data;

    MgPlayerPadSet(player->player, 0, 0, 0, 0);
    if (lbl_1_bss_8) {
        fn_1_140(obj, NULL);
        return;
    }
    if (MgSeqModeGet() == MGSEQ_MODE_WINNER) {
        player->timer = 0;
        fn_1_140(obj, fn_1_1BF4);
        return;
    }
}

/* Winner-phase player callback, installed when winner mode begins: play the selected winner or
 * non-winner motion. */
void fn_1_1BF4(OMOBJ *obj)
{
    M621Player *player = obj->data;

    if (player->winnerF) {
        fn_1_106C(player, 8, 4.0f, 0);
    } else {
        fn_1_106C(player, 9, 4.0f, 0);
    }
    fn_1_140(obj, NULL);
}

/* Pick a random target slot; if frandf rounds to 1.0, force index 3 back to 0. Return it only if
 * that slot is active. */
M621Target *fn_1_1C7C(void)
{
    s32 targetIndex;

    targetIndex = 3.0f * frandf();
    if (targetIndex >= 3) {
        targetIndex = 0;
    }
    if (lbl_1_bss_40[targetIndex]->state != 1) {
        return NULL;
    }
    return lbl_1_bss_40[targetIndex];
}

/* Choose an active target for computer control, favoring the nearest target or an unclaimed one. */
M621Target *fn_1_1D08(M621Player *player)
{
    HuVecF pos;
    HuVecF delta;
    s8 occupied[3];
    s32 candidateIndex;
    M621Target *target;
    float nearestDistance;
    float distance;

    target = NULL;
    if (frandf() < 0.55f) {
        MgActorPosGet(player->player->actor, &pos);
        for (candidateIndex = 0; candidateIndex < 3; candidateIndex++) {
            if (lbl_1_bss_40[candidateIndex]->state == 1) {
                if (target == NULL) {
                    target = lbl_1_bss_40[candidateIndex];
                    PSVECSubtract(&target->position, &pos, &delta);
                    delta.y = 0.0f;
                    nearestDistance = PSVECSquareMag(&delta);
                } else {
                    PSVECSubtract(&lbl_1_bss_40[candidateIndex]->position, &pos, &delta);
                    delta.y = 0.0f;
                    distance = PSVECSquareMag(&delta);
                    if (distance < nearestDistance) {
                        target = lbl_1_bss_40[candidateIndex];
                        nearestDistance = distance;
                    }
                }
            }
        }
    } else {
        for (candidateIndex = 0; candidateIndex < 3; candidateIndex++) {
            occupied[candidateIndex] = 0;
        }
        for (candidateIndex = 0; candidateIndex < 4; candidateIndex++) {
            if (lbl_1_bss_10[candidateIndex]->computerF &&
                lbl_1_bss_10[candidateIndex]->target != NULL &&
                lbl_1_bss_10[candidateIndex]->target->state == 1) {
                occupied[lbl_1_bss_10[candidateIndex]->target->targetNo] = 1;
            }
        }
        for (candidateIndex = 0; candidateIndex < 3; candidateIndex++) {
            if (!occupied[candidateIndex] && lbl_1_bss_40[candidateIndex]->state == 1) {
                target = lbl_1_bss_40[candidateIndex];
                break;
            }
        }
    }
    return target;
}

/* Computer-player updates request a random action delay at difficulties 0 through 2; other
 * difficulty settings receive zero delay. */
s16 fn_1_1F9C(M621Player *player)
{
    switch (player->difficulty) {
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

/* At difficulty 2 or higher, report when the selected target is within 200 horizontal units and
 * less than 250 units above the player, including targets below the player. */
s16 fn_1_2074(M621Player *player)
{
    HuVecF targetPos;
    HuVecF playerPos;
    HuVecF delta;

    if (player->difficulty < 2) {
        return 0;
    }
    if (player->target == NULL || player->target->state != 1) {
        return 0;
    }
    MgActorPosGet(player->target->part[0].actor, &targetPos);
    MgActorPosGet(player->player->actor, &playerPos);
    PSVECSubtract(&targetPos, &playerPos, &delta);
    if (delta.y < 250.0f && delta.x * delta.x + delta.z * delta.z < 40000.0f) {
        return 1;
    }
    return 0;
}

/* Drive computer-player movement and target selection during the normal player update. */
void fn_1_215C(OMOBJ *obj)
{
    HuVecF partPos;
    HuVecF pos;
    HuVecF targetPos;
    HuVecF delta;
    HuVecF stick;
    float height;
    M621Player *player = obj->data;

    if (player->target == NULL || player->target->state != 1) {
        player->target = fn_1_1D08(player);
        player->cpuDelay = fn_1_1F9C(player) / 2;
        player->cpuTimer = 0;
    }
    if (player->target != NULL) {
        MgActorPosGet(player->player->actor, &pos);
        switch (player->cpuState) {
        case 0:
            targetPos = player->target->position;
            PSVECSubtract(&targetPos, &pos, &delta);
            height = delta.y;
            PSVECNormalize(&delta, &stick);
            PSVECScale(&stick, &stick, 20.0f);
            stick.z = (s32)stick.z;
            stick.x = (s32)stick.x;
            PSVECScale(&stick, &stick, 4.0f);
            if (player->cpuDelay > 0) {
                player->cpuDelay--;
            }
            if (player->cpuDelay <= 0 && fn_1_2074(player)) {
                MgPlayerPadSet(player->player, stick.x, -stick.z, PAD_BUTTON_A, PAD_BUTTON_A);
                player->cpuState = 1;
                player->cpuTimer = 0;
                return;
            }
            if (PSVECSquareMag(&delta) < 14400.0f) {
                if (player->cpuDelay <= 0) {
                    MgPlayerPadSet(player->player, stick.x, -stick.z, PAD_BUTTON_B, PAD_BUTTON_B);
                    player->cpuDelay = fn_1_1F9C(player);
                } else {
                    MgPlayerPadSet(player->player, stick.x, -stick.z, 0, 0);
                }
                player->cpuTimer = 0;
                return;
            }
            if (player->player->actor->colGroundAttr & 0x100) {
                MgPlayerPadSet(player->player, stick.x, -stick.z, PAD_BUTTON_A, PAD_BUTTON_A);
                player->cpuState = 2;
                return;
            }
            MgPlayerPadSet(player->player, stick.x, -stick.z, 0, 0);
            player->cpuTimer++;
            if (player->cpuTimer > 40) {
                player->target = NULL;
                return;
            }
            break;
        case 1:
            MgActorPosGet(player->target->part[0].actor, &partPos);
            PSVECSubtract(&partPos, &pos, &delta);
            height = delta.y;
            PSVECNormalize(&delta, &delta);
            PSVECScale(&delta, &delta, 40.0f);
            if (player->player->actor->velY < 0.0f && height < 150.0f) {
                MgPlayerPadSet(player->player, delta.x, -delta.z, PAD_BUTTON_B, PAD_BUTTON_B);
                player->cpuState = 0;
                return;
            }
            MgPlayerPadSet(player->player, delta.x, -delta.z, 0, PAD_BUTTON_A);
            if (!MgPlayerModeAttrCheck(player->player, MGPLAYER_MODEATTR_AIR)) {
                player->cpuState = 0;
                return;
            }
            break;
        case 2:
            MgActorPosGet(player->target->part[0].actor, &partPos);
            PSVECSubtract(&partPos, &pos, &delta);
            height = delta.y;
            PSVECNormalize(&delta, &delta);
            PSVECScale(&delta, &delta, 80.0f);
            MgPlayerPadSet(player->player, delta.x, -delta.z, 0, PAD_BUTTON_A);
            if (!MgPlayerModeAttrCheck(player->player, MGPLAYER_MODEATTR_AIR)) {
                player->cpuState = 0;
            }
            break;
        }
    }
}

/* When building a target stack, reserve the next available segment mask; the scan stops before bit
 * 0. */
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

/* Return the released segment mask to the available set. */
void fn_1_270C(u32 collisionMask)
{
    lbl_1_data_2AC |= collisionMask;
}

/* Return the number of available segment masks, capped at 23. */
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

/* Load shared target models and create the three target-slot objects. */
void fn_1_27A0(void)
{
    s32 targetSlot;

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
    for (targetSlot = 0; targetSlot < 3; targetSlot++) {
        lbl_1_bss_40[targetSlot] = fn_1_A0(30, sizeof(M621Target), fn_1_29E8);
        lbl_1_bss_40[targetSlot]->targetNo = targetSlot;
        lbl_1_bss_40[targetSlot]->firstStackF = 1;
        lbl_1_bss_40[targetSlot]->model = Hu3DModelLink(lbl_1_bss_38);
    }
}

/* On fade-in, initialize this target slot and start its 150-update spawn countdown. */
void fn_1_29E8(OMOBJ *obj)
{
    M621Target *target = obj->data;

    if (MgSeqModeGet() == MGSEQ_MODE_FADEIN) {
        target->state = 0;
        target->timer = 150;
        target->spawnPointNo = -1;
        fn_1_140(obj, fn_1_3678);
        return;
    }
}

/* When the top segment is hit, knock down the stack and score the top plus each remaining standing
 * segment. */
void fn_1_2A54(M621Player *player, M621Target *target)
{
    HuVecF pos;
    s32 delay;
    s32 score;
    s32 partIndex;
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
        fn_1_420(&pos, M621_SE_SEGMENT_HIT);
        pos.y += 75.0f;
        pos.z += 80.0f;
        fn_1_51F4(player->playerNo, &pos);
        score = 1;
        for (partIndex = 1; partIndex < target->partCount; partIndex++) {
            part = &target->part[partIndex];
            if (part->state == 2) {
                MgActorPushSet(part->actor, &lbl_1_data_30);
                MgActorColAttrSet(part->actor, COLBODY_ATTR_BODYCOL_OFF);
                part->timer = delay;
                delay += 10;
                part->state = 4;
                part->effectPlayerNo = player->playerNo;
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
        if (player->vibrationDelay <= 0) {
            omVibrate(player->player->playerNo, 10, 10, 0);
            player->vibrationDelay = 0; /* A hit does not arm another vibration cooldown. */
        }
    }
}

/* Release the selected target segment and update the stack and player score state. */
void fn_1_2C6C(M621Player *player, M621Target *target, s16 partIndex)
{
    HuVecF pos;
    M621TargetPart *part = &target->part[partIndex];

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
        fn_1_420(&pos, M621_SE_SEGMENT_HIT);
        pos.y += 50.0f;
        pos.z += 55.0f;
        fn_1_51F4(player->playerNo, &pos);
        player->score++;
        if (player->score > 99) {
            player->score = 99;
        }
        MgScoreValueSet(player->scoreDisplay, player->score);
        if (player->vibrationDelay <= 0) {
            omVibrate(player->player->playerNo, 10, 10, 0);
            player->vibrationDelay = 0; /* A hit does not arm another vibration cooldown. */
        }
    }
}

/* Begin the segment's five-update removal countdown and link an effect model. The linked model
 * remains hidden at the origin and is not released by this module. */
void fn_1_2E10(M621TargetPart *part)
{
    HuVecF pos;

    part->state = 5;
    /* The fetched position is not used by this cleanup step. */
    MgActorPosGet(part->actor, &pos);
    part->effectModel = Hu3DModelLink(lbl_1_bss_36);
    part->timer = 5;
}

/* Narrow-collision hook: handle a player's attack contact with a target segment. */
int fn_1_2E68(COL_NARROW_PARAM *firstCollider, COL_NARROW_PARAM *secondCollider)
{
    M621Player *player;
    s32 partIndex;
    s32 playerIndex;
    M621Target *target;

    if (secondCollider->type == 2 || secondCollider->type == 3) {
        player = NULL;
        target = lbl_1_bss_40[firstCollider->type - 5];
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (secondCollider->paramB - 256 == lbl_1_bss_10[playerIndex]->player->actor->no) {
                player = lbl_1_bss_10[playerIndex];
                break;
            }
        }
        if (player->attackF == 2) {
            return 0;
        }
        player->attackF = 2;
        if (player->player->actor->colGroundAttr & 0x1) {
            for (partIndex = target->partCount - 1; partIndex >= 0; partIndex--) {
                if (target->part[partIndex].state == 2) {
                    if (partIndex == 0) {
                        fn_1_2A54(player, target);
                    } else {
                        fn_1_2C6C(player, target, partIndex);
                    }
                    break;
                }
            }
        } else if (firstCollider->paramB == 1) {
            fn_1_2A54(player, target);
        } else {
            fn_1_2C6C(player, target, firstCollider->paramB - 1);
        }
    }
    return 0;
}

/* Narrow-collision hook: accept contacts when the second collider is a target-part type from 5
 * through 7. */
int fn_1_2FFC(COL_NARROW_PARAM *firstCollider, COL_NARROW_PARAM *secondCollider)
{
    if (secondCollider->type >= 5 && secondCollider->type < 8) {
        return 1;
    }
    return 0;
}

/* Mesh-correction hook: begin removal for a knocked-away segment, or clear a standing segment's
 * local vertical velocity after the correction. */
void fn_1_3024(MGACTOR *actor, int param)
{
    M621TargetPart *part = (M621TargetPart *)param;

    if (part->state == 3) {
        fn_1_2E10(part);
    } else if (part->state == 2) {
        part->velocity.y = 0.0f;
    }
}

/* Build a random initial stack; its size is not clamped and can reach eight if frandf returns 1.0.
 * Later stacks use fewer available masks directly, or clamp a random size to seven. */
void fn_1_3084(M621Target *target)
{
    MGACTOR_PARAM param;
    HuVecF pos;
    s32 count;
    s32 partIndex;
    float height;

    count = fn_1_272C();
    if (target->firstStackF) {
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
    target->stackHeight = 90.0f * (target->partCount + 1);
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
    pos = target->position;
    pos.y -= target->stackHeight;
    MgActorPosSet(target->actor, &pos);
    param.height = 90.0f;
    param.radius = 60.0f;
    param.type = target->targetNo + 5;
    param.narrowHook = fn_1_2E68;
    param.correctHook = fn_1_3024;
    param.attr = COLBODY_ATTR_MESHCOL_OFF;
    target->part[0].model = Hu3DModelLink(lbl_1_bss_3E);
    target->part[0].collisionMask = fn_1_26A4();
    target->part[0].velocity.x = target->part[0].velocity.y = target->part[0].velocity.z = 0.0f;
    Hu3DModelDispOn(target->part[0].model);
    Hu3DModelShadowSet(target->part[0].model);
    param.height = 150.0f;
    param.param = 1;
    param.correctHookParam = (int)&target->part[0];
    target->part[0].actor = MgActorCreate(&param, target->part[0].model);
    MgActorColMaskSet(target->part[0].actor, target->part[0].collisionMask);
    /* Clamp the head directly to -70..70 degrees; the stack angle is not wrapped first. */
    target->headRotation = target->rotation;
    if (target->headRotation > 70.0f) {
        target->headRotation = 70.0f;
    } else if (target->headRotation < -70.0f) {
        target->headRotation = -70.0f;
    }
    MgActorRotYSet(target->part[0].actor, target->headRotation);
    pos = target->position;
    pos.y += (80.0f + 90.0f * (target->partCount - 2)) - target->stackHeight;
    MgActorPosSet(target->part[0].actor, &pos);
    target->part[0].actor->gravity = 0.0f;
    target->part[0].state = 1;
    for (partIndex = 1; partIndex < target->partCount; partIndex++) {
        target->part[partIndex].model = Hu3DModelLink(lbl_1_bss_3C);
        target->part[partIndex].collisionMask = fn_1_26A4();
        target->part[partIndex].velocity.x = target->part[partIndex].velocity.y =
            target->part[partIndex].velocity.z = 0.0f;
        Hu3DModelDispOn(target->part[partIndex].model);
        Hu3DModelShadowSet(target->part[partIndex].model);
        if (partIndex == 1) {
            height = 80.0f;
        } else {
            height = 90.0f;
        }
        param.height = height;
        param.param = partIndex + 1;
        param.correctHookParam = (int)&target->part[partIndex];
        target->part[partIndex].actor = MgActorCreate(&param, target->part[partIndex].model);
        MgActorColMaskSet(target->part[partIndex].actor, target->part[partIndex].collisionMask);
        pos = target->position;
        pos.y += 90.0f * (target->partCount - partIndex - 1) - target->stackHeight;
        MgActorPosSet(target->part[partIndex].actor, &pos);
        target->part[partIndex].actor->gravity = 0.0f;
        MgActorRotYSet(target->part[partIndex].actor, target->rotation);
        target->part[partIndex].state = 1;
    }
}

/* During the spawn countdown, choose a separated scene position and display the target once
 * fewer than 30 updates remain; the next callback builds the stack at zero. */
void fn_1_3678(OMOBJ *obj)
{
    HuVecF delta;
    HSF_OBJECT *hsf;
    s32 targetSlot;
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
            /* A random value rounded to 1.0 selects index 21, one past lbl_1_data_258; the code
             * looks up and dereferences that entry before the loop condition retries indices of 21
             * or greater. */
            target->spawnPointNo = 21.0f * frandf();
            hsf = Hu3DModelObjPtrGet(lbl_1_bss_3A, lbl_1_data_258[target->spawnPointNo]);
            target->position.x = hsf->mesh.base.pos.x;
            target->position.y = hsf->mesh.base.pos.y;
            target->position.z = hsf->mesh.base.pos.z;
            for (targetSlot = 0; targetSlot < 3; targetSlot++) {
                if (target->targetNo == lbl_1_bss_40[targetSlot]->targetNo
                    || lbl_1_bss_40[targetSlot]->spawnPointNo < 0) {
                    continue;
                }
                PSVECSubtract(&target->position, &lbl_1_bss_40[targetSlot]->position, &delta);
                delta.y = 0.0f;
                if (PSVECSquareMag(&delta) < 62500.0f) {
                    target->spawnPointNo = 22; /* Retry after finding a nearby occupied position. */
                    break;
                }
            }
        } while (target->spawnPointNo >= 21);
        Hu3DModelPosSetV(target->model, &target->position);
        Hu3DModelDispOn(target->model);
        Hu3DMotionTimeSet(target->model, 0.0f);
        Hu3DModelLayerSet(target->model, 1);
        if (target->firstStackF) {
            for (targetSlot = 0; targetSlot < 4; targetSlot++) {
                lbl_1_bss_10[targetSlot]->vibrationDelay = 30;
                omVibrate(lbl_1_bss_10[targetSlot]->player->playerNo, 30, 4, 4);
            }
        }
        fn_1_140(obj, fn_1_390C);
        return;
    }
}

/* Target-slot update after the appearance timer: build the stack and notify players of the new
 * target. */
void fn_1_390C(OMOBJ *obj)
{
    s32 playerIndex;
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
        if (target->firstStackF) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                lbl_1_bss_10[playerIndex]->vibrationDelay = target->partCount * 6;
                omVibrate(lbl_1_bss_10[playerIndex]->player->playerNo, target->partCount * 6, 4, 4);
            }
            target->firstStackF = 0;
            if (lbl_1_data_37E) {
                fn_1_594(64, M621_SE_TARGET_APPEAR);
                lbl_1_data_37E = 0;
            }
        } else {
            fn_1_420(&target->position, M621_SE_TARGET_APPEAR);
        }
        fn_1_140(obj, fn_1_3A78);
        return;
    }
}

/* Target-slot assembly update: raise the segments and enable target collision when assembly
 * completes. */
void fn_1_3A78(OMOBJ *obj)
{
    HuVecF pos;
    float time;
    float height;
    s16 partIndex;
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
    /* Integer truncation and division keep the remaining rise offset in 25-unit steps. */
    height = 25.0f * ((s16)(target->stackHeight * time) / 25);
    for (partIndex = 0; partIndex < target->partCount; partIndex++) {
        pos = target->position;
        if (partIndex == 0) {
            pos.y += 1.0f + ((80.0f + 90.0f * (target->partCount - 2)) - height);
        } else {
            pos.y += 1.0f + (90.0f * (target->partCount - partIndex - 1) - height);
        }
        MgActorPosSet(target->part[partIndex].actor, &pos);
    }
    pos = target->position;
    pos.y = 1.0f + (pos.y - height);
    MgActorPosSet(target->actor, &pos);
    if (time <= 0.0f) {
        for (partIndex = 0; partIndex < target->partCount; partIndex++) {
            target->part[partIndex].timer = 0;
            target->part[partIndex].state = 2;
        }
        MgActorColAttrReset(target->part[target->partCount - 1].actor, COLBODY_ATTR_MESHCOL_OFF);
        MgActorColAttrReset(target->actor, COLBODY_ATTR_MESHCOL_OFF);
        target->state = 1;
        fn_1_140(obj, fn_1_3E18);
        return;
    }
}

/* Live target-slot update each actor tick: move the stack and update segment states, collision
 * height and falling effects. */
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
    s16 partIndex;
    s16 belowPartIndex;
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
    for (partIndex = 0; partIndex < target->partCount; partIndex++) {
        M621TargetPart *currentPart = &target->part[partIndex];
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
        target->spawnPointNo = -1;
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
        for (partIndex = 1; partIndex < target->partCount; partIndex++) {
            if (target->part[partIndex].state == 2) {
                MgActorRotYSet(target->part[partIndex].actor, target->rotation);
            }
        }
    } else {
        push.x = push.y = push.z = 0.0f;
    }
    if (target->actor != NULL) {
        MgActorPosGet(target->actor, &target->position);
        MgActorRotYSet(target->actor, target->rotation);
        MgActorPushSet(target->actor, &push);
    }
    for (partIndex = 0; partIndex < target->partCount; partIndex++) {
        part = &target->part[partIndex];
        below = NULL;
        for (belowPartIndex = partIndex + 1; belowPartIndex < target->partCount; belowPartIndex++) {
            if (target->part[belowPartIndex].state == 2) {
                below = &target->part[belowPartIndex];
                break;
            }
        }
        switch (part->state) {
        case 2:
            part->timer++;
            /* Decrement before computing wobble; the remaining standing-segment count sets this
             * segment's phase offset. */
            standingCount--;
            wobble = 10.0 * sin((M_PI * (((360.0f * part->timer) / 120.0f)
                + (120.0f * standingCount))) / 180.0);
            if (below != NULL) {
                part->velocity.y -= 0.7f;
                MgActorPosGet(part->actor, &pos);
                PSVECAdd(&pos, &part->velocity, &pos);
                MgActorPosGet(below->actor, &belowPos);
                if (partIndex == 0 && 80.0f + belowPos.y >= pos.y) {
                    pos.y = 80.0f + belowPos.y;
                    part->velocity.y = 0.0f;
                } else if (partIndex != 0 && 90.0f + belowPos.y >= pos.y) {
                    pos.y = 90.0f + belowPos.y;
                    part->velocity.y = 0.0f;
                }
                pos.x = target->position.x + wobble;
                pos.z = target->position.z;
                MgActorPosSetRaw(part->actor, &pos);
                height = (pos.y - target->position.y) + (partIndex == 0 ? 150.0f : 90.0f);
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
                        pos.x = target->position.x + wobble;
                        pos.z = target->position.z;
                        MgActorPosSetRaw(part->actor, &pos);
                    }
                }
                height = partIndex == 0 ? 150.0f : 90.0f;
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
                fn_1_420(&pos, M621_SE_SEGMENT_HIT);
                pos.y += 50.0f;
                pos.z += 55.0f;
                fn_1_51F4(part->effectPlayerNo, &pos);
                fn_1_2E10(part);
            }
            break;
        case 5:
            part->timer--;
            if (part->timer == 0) {
                MgActorKill(part->actor);
                Hu3DModelKill(part->model);
                part->state = 0;
                fn_1_270C(part->collisionMask);
            }
            break;
        }
    }
    if (target->actor != NULL) {
        ColBodyGetSafe(target->actor->no)->param.height = maxHeight;
    }
}

/* During finish, pre-winner, or winner cleanup, stop target movement and show its collapse; after
 * 30 updates, play the shared collapse sound once before falling cleanup. */
void fn_1_47B4(OMOBJ *obj)
{
    HuVecF pos;
    HuVecF belowPos;
    float wobble;
    s16 partIndex;
    s16 belowPartIndex;
    s16 inactiveCount;
    s16 wobblePhaseIndex;
    M621TargetPart *part;
    M621TargetPart *below;
    M621Target *target = obj->data;

    inactiveCount = 0;
    wobblePhaseIndex = 0;
    for (partIndex = 0; partIndex < target->partCount; partIndex++) {
        M621TargetPart *currentPart = &target->part[partIndex];
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
        Hu3DModelPosSetV(target->model, &target->position);
        Hu3DModelDispOn(target->model);
        Hu3DMotionTimeSet(target->model, 0.0f);
        Hu3DModelLayerSet(target->model, 1);
    }
    target->timer++;
    if (target->timer >= 30) {
        target->timer = 0;
        if (lbl_1_data_380) {
            fn_1_594(64, M621_SE_TARGET_COLLAPSE);
            lbl_1_data_380 = 0;
        }
        fn_1_140(obj, fn_1_4CB0);
        return;
    }
    for (partIndex = 0; partIndex < target->partCount; partIndex++) {
        part = &target->part[partIndex];
        below = NULL;
        for (belowPartIndex = partIndex + 1; belowPartIndex < target->partCount; belowPartIndex++) {
            if (target->part[belowPartIndex].state == 2) {
                below = &target->part[belowPartIndex];
                break;
            }
        }
        switch (part->state) {
        case 2:
            part->timer++;
            /* The phase index starts at zero and descends for each standing segment. */
            wobblePhaseIndex--;
            wobble = 10.0 * sin((M_PI * (((360.0f * part->timer) / 120.0f)
                + (120.0f * wobblePhaseIndex))) / 180.0);
            if (below != NULL) {
                part->velocity.y -= 0.7f;
                MgActorPosGet(part->actor, &pos);
                PSVECAdd(&pos, &part->velocity, &pos);
                MgActorPosGet(below->actor, &belowPos);
                if (partIndex == 0 && 80.0f + belowPos.y >= pos.y) {
                    pos.y = 80.0f + belowPos.y;
                    part->velocity.y = 0.0f;
                } else if (partIndex != 0 && 90.0f + belowPos.y >= pos.y) {
                    pos.y = 90.0f + belowPos.y;
                    part->velocity.y = 0.0f;
                }
                pos.x = target->position.x + wobble;
                pos.z = target->position.z;
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
                fn_1_270C(part->collisionMask);
            }
            break;
        }
    }
}

/* Update falling and removal states after collapse, stopping when every segment is removed.
 * Assembly can also install this callback while segments are rising; state 1 is left unchanged. */
void fn_1_4CB0(OMOBJ *obj)
{
    HuVecF pos;
    HuVecF belowPos;
    s16 partIndex;
    s16 belowPartIndex;
    s16 inactiveCount;

    s16 standingCount;
    M621TargetPart *part;
    M621TargetPart *below;
    M621Target *target = obj->data;

    inactiveCount = 0;
    standingCount = 0;
    for (partIndex = 0; partIndex < target->partCount; partIndex++) {
        M621TargetPart *currentPart = &target->part[partIndex];
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
    for (partIndex = 0; partIndex < target->partCount; partIndex++) {
        part = &target->part[partIndex];
        below = NULL;
        for (belowPartIndex = partIndex + 1; belowPartIndex < target->partCount; belowPartIndex++) {
            if (target->part[belowPartIndex].state == 2) {
                below = &target->part[belowPartIndex];
                break;
            }
        }
        switch (part->state) {
        case 2:
            part->timer++;
            standingCount--; /* This cleanup counter is not read by the falling-segment logic. */
            MgActorColAttrSet(part->actor, COLBODY_ATTR_MESHCOL_OFF);
            part->actor->gravity = 150.0f;
            if (below != NULL) {
                part->velocity.y -= 0.7f;
                MgActorPosGet(part->actor, &pos);
                PSVECAdd(&pos, &part->velocity, &pos);
                MgActorPosGet(below->actor, &belowPos);
                if (partIndex == 0 && 80.0f + belowPos.y >= pos.y) {
                    pos.y = 80.0f + belowPos.y;
                    part->velocity.y = 0.0f;
                } else if (partIndex != 0 && 90.0f + belowPos.y >= pos.y) {
                    pos.y = 90.0f + belowPos.y;
                    part->velocity.y = 0.0f;
                }
                pos.x = target->position.x;
                pos.z = target->position.z;
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
                fn_1_270C(part->collisionMask);
            }
            break;
        }
    }
}

/* Effect-object init callback, created during round setup: load four reusable segment effects and
 * initialize the pool. */
void fn_1_5054(OMOBJ *obj)
{
    s32 effectIndex;
    M621Effect *effectPool = obj->data;

    for (effectIndex = 0; effectIndex < 4; effectIndex++) {
        lbl_1_bss_2C[effectIndex] = Hu3DModelCreateData(lbl_1_data_9C[effectIndex][0]);
        lbl_1_bss_24[effectIndex] =
            Hu3DJointMotionData(lbl_1_bss_2C[effectIndex], lbl_1_data_9C[effectIndex][1]);
        Hu3DModelDispOff(lbl_1_bss_2C[effectIndex]);
    }
    memset(effectPool, 0, 21 * sizeof(M621Effect));
    fn_1_140(obj, fn_1_5168);
}

/* Effect-object update callback: release linked models after their motions end. */
void fn_1_5168(OMOBJ *obj)
{
    M621Effect *effect;
    s32 effectIndex;
    M621Effect *effectPool = obj->data;

    for (effectIndex = 0; effectIndex < 21; effectIndex++) {
        effect = &effectPool[effectIndex];
        if (effect->active == 1 && Hu3DMotionEndCheck(effect->model)) {
            Hu3DModelKill(effect->model);
            effect->active = 0;
        }
    }
}

/* Start the requested segment effect at a world position in the first free slot; drop it if all 21
 * slots are active. */
void fn_1_51F4(s16 effectPlayerNo, HuVecF *effectPosition)
{
    M621Effect *effect;
    s32 effectIndex;

    for (effectIndex = 0; effectIndex < 21; effectIndex++) {
        effect = &lbl_1_bss_20[effectIndex];
        if (effect->active == 0) {
            effect->model = Hu3DModelLink(lbl_1_bss_2C[effectPlayerNo]);
            Hu3DMotionSet(effect->model, lbl_1_bss_24[effectPlayerNo]);
            Hu3DModelDispOn(effect->model);
            Hu3DModelPosSetV(effect->model, effectPosition);
            Hu3DModelLayerSet(effect->model, 2);
            effect->active = 1;
            break;
        }
    }
}

/* Set a player's minigame coin bonus outside practice mode. */
void fn_1_52D4(s32 playerNo, s16 coins)
{
    if (!_CheckFlag(FLAG_MG_PRACTICE)) {
        GwPlayer[playerNo].mgCoinBonus = coins;
    }
}

/* Store a player's minigame score in the game-wide player record. */
void fn_1_5328(s32 playerNo, s32 score)
{
    GwPlayer[playerNo].mgScore = score;
}

/* Return the current night-setting flag used to select scene assets. */
s16 fn_1_5340(void)
{
    return GwMgNightF;
}

/* Return the absolute value of a float through the double-precision helper. */
float fn_1_5350(float value)
{
    return fn_1_537C(value);
}

/* Return the absolute value of a double. */
double fn_1_537C(double value)
{
    return fabs(value);
}

s16 lbl_1_data_37E = 1;
s16 lbl_1_data_380 = 1;
