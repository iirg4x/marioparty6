#include "REL/m635dll.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "datadir_enum.h"
#include "game/sprite.h"
#include "game/charman.h"
#include "game/audio.h"
#include "game/frand.h"
#include "string.h"

typedef struct M635SpritePos {
    float x;
    float y;
} M635SpritePos;

float lbl_1_data_A0[2] = { 7.0f / 6.0f, 5.0f / 3.0f };
HuVecF lbl_1_data_A8[2] = { { -200, 0, 0 }, { 200, 0, 0 } };
HuVecF lbl_1_data_C0[2] = { { -200, 0, 0 }, { 200, 0, 0 } };
HuVecF lbl_1_data_D8 = { 500, 2000, 400 };
HuVecF lbl_1_data_E4 = { -5, -20, -4 };
GXColor lbl_1_data_F0[2] = { { 192, 192, 192, 192 }, { 192, 192, 192, 192 } };
HuVecF lbl_1_data_F8[4] = {
    { -280, 0, -150 }, { -120, 0, -150 },
    { 120, 0, -150 }, { 280, 0, -150 }
};
HuVecF lbl_1_data_128[4] = {
    { -210, 0, -450 }, { -70, 0, -450 },
    { 70, 0, -450 }, { 210, 0, -450 }
};
M635Motion lbl_1_data_158[6] = {
    { DATANUM(DATA_mario, 84), 0 },
    { CHARMOT_HSF_c000m1_300, 1 },
    { CHARMOT_HSF_c000m1_306, 0 },
    { CHARMOT_HSF_c000m1_307, 0 },
    { CHARMOT_HSF_c000m1_316, 0 },
    { DATANUM(DATA_mario, 21), 0 }
};
M635SpritePos lbl_1_data_188[4] = { { 120, 160 }, { 220, 160 }, { 358, 160 }, { 458, 160 } };
s16 lbl_1_data_1A8[20][8] = {
    { 1, 20, 1, 30, 1, 30, 1, 20 },
    { 71, 90, 71, 100, 71, 100, 71, 90 },
    { 111, 130, 111, 140, 111, 140, 111, 130 },
    { 151, 170, 151, 180, 151, 180, 151, 170 },
    { 191, 210, 191, 220, 191, 220, 191, 210 },
    { 231, 250, 231, 260, 231, 260, 231, 250 },
    { 271, 290, 271, 300, 271, 300, 271, 290 },
    { 311, 330, 311, 340, 311, 340, 311, 330 },
    { 351, 370, 351, 380, 351, 380, 351, 370 },
    { 401, 430, 401, 430, 401, 430, 370, 370 },
    { 431, 460, 431, 460, 431, 460, 370, 370 },
    { 461, 490, 461, 490, 461, 490, 370, 370 },
    { 491, 520, 491, 520, 491, 520, 370, 370 },
    { 521, 550, 521, 550, 521, 550, 370, 370 },
    { 551, 580, 551, 580, 551, 580, 370, 370 },
    { 581, 610, 581, 610, 581, 610, 370, 370 },
    { 611, 640, 611, 640, 611, 640, 370, 370 },
    { 641, 670, 641, 670, 641, 670, 370, 370 },
    { 671, 700, 671, 700, 671, 671, 370, 370 },
    { 710, 830, 710, 830, 710, 830, 710, 830 }
};
M635PositionStep lbl_1_data_2E8[21] = {
    { { -150, -150 }, 20 }, { { -180, -180 }, 20 },
    { { -180, -188 }, 10 }, { { -196, -188 }, 10 },
    { { -196, -204 }, 10 }, { { -212, -204 }, 10 },
    { { -212, -220 }, 10 }, { { -228, -220 }, 10 },
    { { -228, -236 }, 10 }, { { -236, -236 }, 10 },
    { { -238, -238 }, 15 }, { { -240, -240 }, 15 },
    { { -242, -242 }, 15 }, { { -244, -244 }, 15 },
    { { -246, -246 }, 15 }, { { -248, -248 }, 15 },
    { { -250, -250 }, 15 }, { { -252, -252 }, 15 },
    { { -254, -254 }, 15 }, { { -256, -256 }, 15 },
    { { -356, -356 }, 15 }
};
u32 lbl_1_data_3E4[2] = { DATANUM(DATA_m635, 0), DATANUM(DATA_m635, 1) };
u32 lbl_1_data_3EC[2] = { DATANUM(DATA_m635, 2), DATANUM(DATA_m635, 3) };
u32 lbl_1_data_3F4[2][2] = {
    { DATANUM(DATA_m635, 5), DATANUM(DATA_m635, 6) },
    { DATANUM(DATA_m635, 7), DATANUM(DATA_m635, 8) }
};
u32 lbl_1_data_404[2][2] = {
    { DATANUM(DATA_m635, 11), DATANUM(DATA_m635, 12) },
    { DATANUM(DATA_m635, 13), DATANUM(DATA_m635, 14) }
};
u32 lbl_1_data_414[2] = { DATANUM(DATA_m635, 15), DATANUM(DATA_m635, 16) };
u32 lbl_1_data_41C[12] = {
    DATANUM(DATA_m635, 22), DATANUM(DATA_m635, 23),
    DATANUM(DATA_m635, 24), DATANUM(DATA_m635, 25),
    DATANUM(DATA_m635, 26), DATANUM(DATA_m635, 27),
    DATANUM(DATA_m635, 28), DATANUM(DATA_m635, 29),
    DATANUM(DATA_m635, 30), DATANUM(DATA_m635, 31),
    DATANUM(DATA_m635, 32), DATANUM(DATA_m635, 33)
};
u32 lbl_1_data_44C[2] = { DATANUM(DATA_m635, 20), DATANUM(DATA_m635, 21) };

void fn_1_1774(OMOBJMAN *objman)
{
    OMOBJ *obj;

    Hu3DCameraCreate(1);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(1, 20.0f, 60.0f, 25000.0f, 1.2f);
    Hu3DCameraPosSet(1, 0.0f, 800.0f, 2000.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 100.0f, -500.0f);
    obj = omAddObjEx(objman, 32730, 0, 0, -1, omOutView);
    Center.x = Center.y = 0.0f;
    Center.z = -400.0f;
    CRot.x = -30.0f;
    CRot.y = 0.0f;
    CRot.z = 0.0f;
    CZoom = 3000.0f;
}

void fn_1_195C(void)
{
    s16 id;
    int team;
    s16 light;
    s16 nightF;
    HuVecF shadowPos;
    HuVecF shadowTarget;
    HuVecF shadowUp;

    nightF = GwMgNightF;
    lbl_1_bss_4.nightF = nightF;
    light = Hu3DGLightCreateV(&lbl_1_data_D8, &lbl_1_data_E4, &lbl_1_data_F0[lbl_1_bss_4.nightF]);
    Hu3DGLightStaticSet(light, 1);
    Hu3DGLightInfinitytSet(light);
    lbl_1_bss_4.light = light;
    Hu3DShadowCreate(30.0f, 20.0f, 10000.0f);
    shadowPos.x = 500.0f;
    shadowPos.y = 2000.0f;
    shadowPos.z = 400.0f;
    shadowUp.y = 1.0f;
    shadowUp.x = shadowUp.z = 0.0f;
    shadowTarget.x = shadowTarget.y = shadowTarget.z = 0.0f;
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
    id = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_3E4[lbl_1_bss_4.nightF], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(id, HU3D_CAM0);
    Hu3DModelPosSet(id, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(id, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(id, 1.0f, 1.0f, 1.0f);
    Hu3DModelShadowMapSet(id);
    Hu3DModelAttrSet(id, HU3D_MOTATTR_LOOP);
    if (lbl_1_bss_4.nightF == 0) {
        Hu3DModelShadowMapTPLvlSet(id, 0.8f);
    } else {
        Hu3DModelShadowMapTPLvlSet(id, 0.3f);
    }
    lbl_1_bss_4.model_4E = id;
    id = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_3EC[lbl_1_bss_4.nightF], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(id, HU3D_CAM0);
    Hu3DModelPosSet(id, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(id, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(id, 1.0f, 1.0f, 1.0f);
    lbl_1_bss_4.model_50 = id;
    for (team = 0; team < 2; team++) {
        id = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_3F4[lbl_1_bss_4.nightF][team], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(id, HU3D_CAM0);
        Hu3DModelPosSetV(id, &lbl_1_data_C0[team]);
        Hu3DMotionSpeedSet(id, 0.0f);
        Hu3DModelShadowMapSet(id);
        if (lbl_1_bss_4.nightF == 0) {
            Hu3DModelShadowMapTPLvlSet(id, 0.8f);
        } else {
            Hu3DModelShadowMapTPLvlSet(id, 0.3f);
        }
        lbl_1_bss_4.team[team].unk_10.model = id;
        id = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_404[lbl_1_bss_4.nightF][team], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(id, HU3D_CAM0);
        Hu3DMotionSpeedSet(id, 0.0f);
        lbl_1_bss_4.team[team].unk_20 = id;
        id = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m635, 4), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(id, HU3D_CAM0);
        Hu3DModelPosSetV(id, &lbl_1_data_A8[team]);
        Hu3DMotionSpeedSet(id, 0.0f);
        lbl_1_bss_4.team[team].unk_14.model = id;
        id = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m635, 9), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(id, HU3D_CAM0);
        Hu3DModelPosSetV(id, &lbl_1_data_A8[team]);
        Hu3DMotionSpeedSet(id, 0.0f);
        lbl_1_bss_4.team[team].unk_18.model = id;
        id = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m635, 10), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(id, HU3D_CAM0);
        Hu3DModelPosSetV(id, &lbl_1_data_A8[team]);
        Hu3DMotionSpeedSet(id, 0.0f);
        lbl_1_bss_4.team[team].unk_1C.model = id;
        id = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_414[lbl_1_bss_4.nightF], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(id, HU3D_CAM0);
        Hu3DModelPosSetV(id, &lbl_1_data_A8[team]);
        Hu3DMotionSpeedSet(id, 0.0f);
        lbl_1_bss_4.team[team].unk_22 = id;
        id = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m635, 17), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(id, HU3D_CAM0);
        Hu3DModelPosSetV(id, &lbl_1_data_A8[team]);
        Hu3DMotionSpeedSet(id, 0.0f);
        lbl_1_bss_4.team[team].unk_24 = id;
    }
}

void fn_1_2014(void)
{
}

void fn_1_2018(void)
{
    ANIMDATA *buttonAnim[12];
    ANIMDATA *backgroundAnim[2];
    int i;
    int member;
    s16 group;

    for (i = 0; i < 12; i++) {
        buttonAnim[i] = HuSprAnimRead(HuDataSelHeapReadNum(lbl_1_data_41C[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 2; i++) {
        backgroundAnim[i] = HuSprAnimRead(HuDataSelHeapReadNum(lbl_1_data_44C[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 4; i++) {
        group = lbl_1_bss_74[i].group = HuSprGrpCreate(14);
        HuSprGrpPosSet(group, lbl_1_data_188[i].x, lbl_1_data_188[i].y);
        for (member = 0; member < 12; member++) {
            HuSprGrpMemberSet(group, member, HuSprCreate(buttonAnim[member], 10, 0));
            HuSprPosSet(group, member, 0.0f, -5.0f);
            HuSprAttrSet(group, member, HUSPR_ATTR_DISPOFF);
        }
        for (member = 0; member < 2; member++) {
            HuSprGrpMemberSet(group, member + 12, HuSprCreate(backgroundAnim[member], 15, 0));
            HuSprAttrSet(group, member + 12, HUSPR_ATTR_DISPOFF);
            HuSprTPLvlSet(group, member + 12, 0.75f);
        }
        lbl_1_bss_74[i].member = -1;
        lbl_1_bss_74[i].state = 5;
        lbl_1_bss_74[i].scale = 0.0f;
    }
}

void fn_1_2268(s16 team, s16 player)
{
    M635Sprite *sprite;
    s16 member;

    sprite = &lbl_1_bss_74[player + team * 2];
    if (sprite->state == 4) {
        member = lbl_1_bss_4.team[team].buttonIndex[player] * 2;
        HuSprAttrSet(sprite->group, member, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(sprite->group, member + 1, HUSPR_ATTR_DISPOFF);
        sprite->timer = 0;
    }
}

void fn_1_2330(void)
{
    M635Sprite *sprite;
    int i;
    s16 member;

    for (i = 0; i < 4; i++) {
        sprite = &lbl_1_bss_74[i];
        member = lbl_1_bss_4.team[i / 2].buttonIndex[i % 2] * 2;
        switch (sprite->state) {
        case 5:
            break;
        case 0:
            break;
        case 1:
            sprite->scale += 0.2;
            if (sprite->scale >= 1.0f) {
                sprite->scale = 1.0f;
                sprite->state = 0;
            }
            HuSprScaleSet(sprite->group, member, sprite->scale, sprite->scale);
            HuSprScaleSet(sprite->group, 12, sprite->scale, sprite->scale);
            break;
        case 3:
            sprite->timer--;
            if (sprite->timer <= 0) {
                HuSprAttrSet(sprite->group, sprite->member + 1, HUSPR_ATTR_DISPOFF);
                HuSprAttrSet(sprite->group, 13, HUSPR_ATTR_DISPOFF);
                sprite->state = 5;
                sprite->scale = 0.0f;
            }
            break;
        case 4:
            if (sprite->scale < 1.0f) {
                sprite->scale += 0.2;
            }
            if (sprite->timer % 8 == 0) {
                HuSprAttrSet(sprite->group, sprite->member, HUSPR_ATTR_DISPOFF);
                HuSprAttrReset(sprite->group, sprite->member + 1, HUSPR_ATTR_DISPOFF);
            } else if (sprite->timer % 8 == 4) {
                HuSprAttrSet(sprite->group, sprite->member + 1, HUSPR_ATTR_DISPOFF);
                HuSprAttrReset(sprite->group, sprite->member, HUSPR_ATTR_DISPOFF);
            }
            HuSprScaleSet(sprite->group, sprite->member, sprite->scale, sprite->scale);
            HuSprScaleSet(sprite->group, sprite->member + 1, sprite->scale, sprite->scale);
            HuSprScaleSet(sprite->group, 13, sprite->scale, sprite->scale);
            sprite->timer++;
            break;
        }
    }
}

void fn_1_25EC(s16 team, s16 player, s16 state)
{
    M635Sprite *sprite;
    int i;
    s16 member;

    sprite = &lbl_1_bss_74[player + team * 2];
    sprite->state = state;
    member = lbl_1_bss_4.team[team].buttonIndex[player] * 2;
    for (i = 0; i < 12; i++) {
        HuSprAttrSet(sprite->group, i, HUSPR_ATTR_DISPOFF);
    }
    for (i = 0; i < 2; i++) {
        HuSprAttrSet(sprite->group, i + 12, HUSPR_ATTR_DISPOFF);
    }
    switch (state) {
    case 1:
        sprite->scale = 0.0f;
        HuSprAttrReset(sprite->group, member, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(sprite->group, 12, HUSPR_ATTR_DISPOFF);
        HuSprScaleSet(sprite->group, member, 0.0f, 0.0f);
        HuSprScaleSet(sprite->group, 12, 0.0f, 0.0f);
        sprite->member = member;
        break;
    case 3:
        sprite->timer = 8;
        sprite->scale = 1.0f;
        HuSprAttrReset(sprite->group, member + 1, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(sprite->group, 13, HUSPR_ATTR_DISPOFF);
        break;
    case 4:
        HuSprAttrReset(sprite->group, 13, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(sprite->group, member, HUSPR_ATTR_DISPOFF);
        sprite->member = member;
        sprite->timer = 0;
        sprite->scale = 0.0f;
        break;
    }
}

s16 fn_1_27E4(s16 team, s16 player)
{
    return lbl_1_bss_74[player + team * 2].state;
}

void fn_1_280C(s16 team, s16 player, s16 member)
{
    M635Sprite *sprite;
    int i;

    sprite = &lbl_1_bss_74[player + team * 2];
    for (i = 0; i < 12; i++) {
        HuSprAttrSet(sprite->group, i, HUSPR_ATTR_DISPOFF);
    }
    HuSprAttrReset(sprite->group, member, HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(sprite->group, 13, HUSPR_ATTR_DISPOFF);
}

void fn_1_28A8(s16 team, s16 player)
{
    M635Sprite *sprite;

    sprite = &lbl_1_bss_74[player + team * 2];
    HuSprAttrSet(sprite->group, 13, HUSPR_ATTR_DISPOFF);
}

void fn_1_2904(void)
{
    int i;

    for (i = 0; i < 4; i++) {
        HuSprGrpKill(lbl_1_bss_74[i].group);
    }
}

void fn_1_2954(void)
{
    int i;
    s16 charNo;
    int member;
    int motion;
    s16 player;
    s16 memberCountA;
    s16 memberCountB;
    s16 model;
    s16 lookupCountA;
    s16 lookupCountB;
    s16 team;
    s16 motionId;
    s16 motionTime;

    lookupCountA = 0;
    lookupCountB = 0;
    memberCountA = 0;
    memberCountB = 0;
    for (i = 0; i < 4; i++) {
        lbl_1_bss_BC[i].playerNo = i;
        charNo = GwPlayerConf[i].charNo;
        lbl_1_bss_BC[i].charNo = charNo;
        lbl_1_bss_BC[i].padNo = GwPlayerConf[i].padNo;
        team = lbl_1_bss_BC[i].teamNo = GwPlayerConf[i].grpNo;
        if (team == 0) {
            lbl_1_bss_BC[i].memberNo = memberCountA;
            memberCountA++;
        } else {
            lbl_1_bss_BC[i].memberNo = memberCountB;
            memberCountB++;
        }
        lbl_1_bss_BC[i].difficulty = GwPlayerConf[i].comDif;
        lbl_1_bss_BC[i].comF = GwPlayerConf[i].type;
        lbl_1_bss_BC[i].timer = fn_1_1308(GwPlayerConf[i].comDif);
        if (team == 0) {
            lbl_1_bss_B4[0][lookupCountA++] = i;
        } else {
            lbl_1_bss_B4[1][lookupCountB++] = i;
        }
    }
    for (i = 0; i < 2; i++) {
        for (member = 0; member < 2; member++) {
            player = lbl_1_bss_B4[i][member];
            charNo = lbl_1_bss_BC[player].charNo;
            model = CharModelCreate(charNo, 2);
            Hu3DModelPosSetV(model, &lbl_1_data_F8[member + i * 2]);
            lbl_1_bss_BC[player].z = lbl_1_data_F8[member + i * 2].z;
            lbl_1_bss_BC[player].model = model;
            for (motion = 0; motion < 6; motion++) {
                motionId = CharMotionCreate(charNo, lbl_1_data_158[motion].dataNum);
                lbl_1_bss_BC[player].motions[motion] = motionId;
            }
            Hu3DModelShadowSet(model);
            CharMotionDataClose(charNo);
            CharMotionSet(charNo, lbl_1_bss_BC[player].motions[0]);
            motionTime = CharMotionMaxTimeGet(charNo);
            CharMotionTimeSet(charNo, motionTime);
        }
    }
}

void fn_1_2CE8(void)
{
    int member;
    int team;
    s16 state;
    s16 player;
    s16 model;
    float z;

    for (team = 0; team < 2; team++) {
        for (member = 0; member < 2; member++) {
            player = lbl_1_bss_B4[team][member];
            state = lbl_1_bss_BC[player].state;
            if (state != 0) {
                model = lbl_1_bss_BC[player].model;
                z = lbl_1_bss_BC[player].z;
                z += (lbl_1_data_2E8[state].z[member] - lbl_1_data_2E8[state - 1].z[member])
                    / lbl_1_data_2E8[state].frames;
                if (z <= lbl_1_data_2E8[state].z[member]) {
                    z = lbl_1_data_2E8[state].z[member];
                } else {
                    lbl_1_bss_BC[player].z = z;
                    Hu3DModelPosSet(model,
                        lbl_1_data_F8[member + team * 2].x,
                        lbl_1_data_F8[member + team * 2].y,
                        lbl_1_bss_BC[player].z);
                }
            }
        }
    }
}

void fn_1_2F08(s16 player, s16 state)
{
    if (state < 0 || state > 11) {
        return;
    }
    lbl_1_bss_BC[player].state = state;
}

void fn_1_2F40(s16 player)
{
    if (lbl_1_bss_BC[player].state > 8) {
        lbl_1_bss_BC[player].state++;
    } else if (lbl_1_bss_BC[player].state == 8) {
        lbl_1_bss_BC[player].state = 10;
    } else if (lbl_1_bss_BC[player].state == 0) {
        lbl_1_bss_BC[player].state = 1;
    } else if (lbl_1_bss_BC[player].state == 1) {
        if (lbl_1_bss_BC[player].memberNo == 0) {
            lbl_1_bss_BC[player].state = 3;
        } else {
            lbl_1_bss_BC[player].state = 2;
        }
    } else if (lbl_1_bss_BC[player].state < 8) {
        lbl_1_bss_BC[player].state += 2;
    }
}

BOOL fn_1_30C8(s16 player, s16 charNo)
{
    float maxTime;
    M635Player *work;

    work = &lbl_1_bss_BC[player];
    maxTime = CharMotionMaxTimeGet(charNo);
    fn_1_31BC(charNo);
    CharMotionShiftSet(charNo, lbl_1_bss_BC[player].motions[0],
        0.0f, 8.0f, lbl_1_data_158[0].attr);
    omVibrate(player, 20, 4, 4);
    fn_1_3B7C(lbl_1_bss_BC[player].teamNo, lbl_1_bss_BC[player].memberNo, player);
    return TRUE;
}

void fn_1_31BC(s16 charNo)
{
    int i;

    for (i = 0; i < 2; i++) {
        CharModelCryCreate(charNo, 30.0f, 0.8f);
    }
}

void fn_1_3218(s16 player, s16 motion, float time)
{
    s16 charNo;

    if (motion < 0 || motion >= 6) {
        return;
    }
    charNo = lbl_1_bss_BC[player].charNo;
    CharMotionShiftSet(charNo, lbl_1_bss_BC[player].motions[motion],
        0.0f, time, lbl_1_data_158[motion].attr);
}

s16 fn_1_32E0(s16 team, s16 member)
{
    if (team < 0 || team >= 2 || member < 0 || member >= 2) {
        return 0;
    }
    return lbl_1_bss_B4[team][member];
}

void fn_1_3340(void)
{
    int team;
    int member;
    s16 player;
    s16 model;

    for (team = 0; team < 2; team++) {
        for (member = 0; member < 2; member++) {
            player = lbl_1_bss_B4[team][member];
            model = lbl_1_bss_BC[player].model;
            Hu3DModelPosSetV(model, &lbl_1_data_128[member + team * 2]);
        }
    }
}

void fn_1_33F8(void)
{
}

void fn_1_33FC(s16 team)
{
    s16 player;
    int member;
    M635Team *work;

    work = &lbl_1_bss_4.team[team];
    work->unk_0E++;
    Hu3DMotionSpeedSet(lbl_1_bss_4.team[team].unk_22, 1.0f);
    if (work->unk_0E >= 20) {
        Hu3DMotionSpeedSet(lbl_1_bss_4.team[team].unk_24, 1.0f);
        for (member = 0; member < 2; member++) {
            player = fn_1_32E0(team, member);
            fn_1_3218(player, 4, 8.0f);
            fn_1_2F40(player);
            OSReport("%d\n", lbl_1_bss_BC[player].state);
            omVibrate(player, 30, 20, 0);
        }
    }
}

void fn_1_3738(s16 team, s16 value)
{
    s16 player;
    int member;
    s16 players[2];
    s16 chars[2];
    M635Team *work;

    work = &lbl_1_bss_4.team[team];
    if (work->unk_0E != value) {
        for (member = 0; member < 2; member++) {
            player = lbl_1_bss_B4[team][member];
            players[member] = lbl_1_bss_BC[player].playerNo;
            chars[member] = lbl_1_bss_BC[player].charNo;
            fn_1_30C8(players[member], chars[member]);
            fn_1_2F40(player);
        }
        if (value > 9) {
            if (team == 0) {
                HuAudFXPlay(1863);
            } else {
                HuAudFXPlay(1864);
            }
        }
    }
    work->unk_0E = value;
}

void fn_1_3AC4(s16 team)
{
    M635Model *model;
    M635Team *work;

    work = &lbl_1_bss_4.team[team];
    if (work->unk_0E < 10) {
        work->unk_0E++;
    }
    if (work->unk_14.time >= 430) {
        model = &work->unk_14;
        model->time = 401;
        Hu3DMotionTimeSet(model->model, model->time);
        Hu3DMotionSpeedSet(model->model, 1.0f);
    }
}

void fn_1_3B7C(s16 team, s16 member, s16 player)
{
    M635Model *model;
    M635Team *work;

    work = &lbl_1_bss_4.team[team];
    if (member != 0) {
        model = &work->unk_18;
    } else {
        model = &work->unk_1C;
    }
    if (model->time >= 430) {
        model->time = 401;
        Hu3DMotionTimeSet(model->model, model->time);
        Hu3DMotionSpeedSet(model->model, 1.0f);
    }
}

void fn_1_3C34(void)
{
    M635Model *model;
    s16 step;
    int team;
    M635Team *work;

    for (team = 0; team < 2; team++) {
        work = &lbl_1_bss_4.team[team];
        step = work->unk_0E;
        if (step != 0) {
            model = &work->unk_14;
            model->time = Hu3DMotionTimeGet(model->model);
            if (model->time < lbl_1_data_1A8[step - 1][0]) {
                if (lbl_1_data_1A8[step - 1][0] < 400 || lbl_1_data_1A8[step - 1][0] > 700) {
                    model->time = lbl_1_data_1A8[step - 1][0];
                    Hu3DMotionTimeSet(model->model, model->time);
                    if (team == 0) {
                        HuAudFXPlay(1859);
                    } else {
                        HuAudFXPlay(1860);
                    }
                }
                Hu3DMotionSpeedSet(model->model, 1.0f);
            }
            if (model->time >= lbl_1_data_1A8[step - 1][1]) {
                Hu3DMotionSpeedSet(model->model, 0.0f);
            }
            if (model->time == 769) {
                Hu3DMotionSpeedSet(lbl_1_bss_4.team[team].unk_20, 1.0f);
            } else if (model->time == 710) {
                if (team == 0) {
                    HuAudFXPlay(1861);
                } else {
                    HuAudFXPlay(1862);
                }
            }
            model = &work->unk_18;
            model->time = Hu3DMotionTimeGet(model->model);
            if (model->time < lbl_1_data_1A8[step - 1][2]) {
                model->time = lbl_1_data_1A8[step - 1][2];
                Hu3DMotionTimeSet(model->model, model->time);
                Hu3DMotionSpeedSet(model->model, 1.0f);
            }
            if (model->time >= lbl_1_data_1A8[step - 1][3]) {
                Hu3DMotionSpeedSet(model->model, 0.0f);
            }
            model = &work->unk_1C;
            model->time = Hu3DMotionTimeGet(model->model);
            if (model->time < lbl_1_data_1A8[step - 1][4]) {
                model->time = lbl_1_data_1A8[step - 1][4];
                Hu3DMotionTimeSet(model->model, model->time);
                Hu3DMotionSpeedSet(model->model, 1.0f);
            }
            if (model->time >= lbl_1_data_1A8[step - 1][5]) {
                Hu3DMotionSpeedSet(model->model, 0.0f);
            }
            model = &work->unk_10;
            model->time = Hu3DMotionTimeGet(model->model);
            if (model->time < lbl_1_data_1A8[step - 1][6]) {
                model->time = lbl_1_data_1A8[step - 1][6];
                Hu3DMotionTimeSet(model->model, model->time);
                Hu3DMotionSpeedSet(model->model, 1.0f);
            }
            if (model->time >= lbl_1_data_1A8[step - 1][7]) {
                Hu3DMotionSpeedSet(model->model, 0.0f);
            }
        }
    }
}

void fn_1_4114(void)
{
    s16 model;
    s16 direction;
    float x;
    float rotation;

    memset(&lbl_1_bss_68, 0, sizeof(lbl_1_bss_68));
    direction = frandmod(2);
    lbl_1_bss_68.direction = direction;
    if (lbl_1_bss_4.nightF == 0) {
        model = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m635, 18), HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m635, 19), HU_MEMNUM_OVL, HEAP_MODEL));
    }
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet(model, HU3D_CAM0);
    Hu3DModelShadowSet(model);
    if (direction != 0) {
        x = -600.0f;
        rotation = 90.0f;
    } else {
        x = 600.0f;
        rotation = -90.0f;
    }
    Hu3DModelPosSet(model, x, 0.0f, -550.0f);
    Hu3DModelRotSet(model, 0.0f, rotation, 0.0f);
    lbl_1_bss_68.x = x;
    lbl_1_bss_68.model = model;
}

void fn_1_42A0(void)
{
    if (lbl_1_bss_68.timer > 0) {
        lbl_1_bss_68.timer--;
        return;
    }
    if (lbl_1_bss_68.direction != 0) {
        lbl_1_bss_68.x += lbl_1_data_A0[lbl_1_bss_4.nightF];
        if (lbl_1_bss_68.x >= 600.0f) {
            lbl_1_bss_68.timer = 120;
            lbl_1_bss_68.direction = 0;
            Hu3DModelRotSet(lbl_1_bss_68.model, 0.0f, -90.0f, 0.0f);
        }
    } else {
        lbl_1_bss_68.x -= lbl_1_data_A0[lbl_1_bss_4.nightF];
        if (lbl_1_bss_68.x <= -600.0f) {
            lbl_1_bss_68.timer = 120;
            lbl_1_bss_68.direction = 1;
            Hu3DModelRotSet(lbl_1_bss_68.model, 0.0f, 90.0f, 0.0f);
        }
    }
    Hu3DModelPosSet(lbl_1_bss_68.model, lbl_1_bss_68.x, 0.0f, -550.0f);
}

void fn_1_448C(void)
{
    Hu3DModelAttrSet(lbl_1_bss_68.model, HU3D_ATTR_DISPOFF);
}
