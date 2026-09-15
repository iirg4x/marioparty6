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

M650Point lbl_1_data_1F8[5] = {
    { { -550.0f, 0.0f, 1400.0f }, 0 },
    { { -350.0f, 0.0f, 1800.0f }, 1 },
    { { 0.0f, 0.0f, 2000.0f }, 0 },
    { { 350.0f, 0.0f, 1800.0f }, 1 },
    { { 550.0f, 0.0f, 1400.0f }, 0 },
};
M650Point lbl_1_data_248[32] = {
    { { -700.0f, 0.0f, 1000.0f }, 0 },
    { { 0.0f, 0.0f, 1000.0f }, 0 },
    { { 700.0f, 0.0f, 1000.0f }, 0 },
    { { -1050.0f, 0.0f, 1500.0f }, 0 },
    { { -350.0f, 0.0f, 1500.0f }, 0 },
    { { 350.0f, 0.0f, 1500.0f }, 0 },
    { { 1500.0f, 0.0f, 1500.0f }, 0 },
    { { -700.0f, 0.0f, 1950.0f }, 0 },
    { { 700.0f, 0.0f, 1950.0f }, 0 },
    { { -1000.0f, 0.0f, 2450.0f }, 0 },
    { { -600.0f, 0.0f, 2450.0f }, 0 },
    { { -200.0f, 0.0f, 2450.0f }, 0 },
    { { 200.0f, 0.0f, 2450.0f }, 0 },
    { { 600.0f, 0.0f, 2450.0f }, 0 },
    { { 1000.0f, 0.0f, 2450.0f }, 0 },
    { { -700.0f, 0.0f, 2900.0f }, 1 },
    { { 0.0f, 0.0f, 2900.0f }, 1 },
    { { 700.0f, 0.0f, 2900.0f }, 1 },
    { { -1050.0f, 0.0f, 3400.0f }, 1 },
    { { -350.0f, 0.0f, 3400.0f }, 1 },
    { { 350.0f, 0.0f, 3400.0f }, 1 },
    { { 1050.0f, 0.0f, 3400.0f }, 1 },
    { { -700.0f, 0.0f, 3850.0f }, 1 },
    { { 0.0f, 0.0f, 3850.0f }, 1 },
    { { 700.0f, 0.0f, 3850.0f }, 1 },
    { { -1050.0f, 0.0f, 4300.0f }, 1 },
    { { -350.0f, 0.0f, 4300.0f }, 1 },
    { { 350.0f, 0.0f, 4300.0f }, 1 },
    { { 1050.0f, 0.0f, 4300.0f }, 1 },
    { { -700.0f, 0.0f, 4750.0f }, 1 },
    { { 0.0f, 0.0f, 4750.0f }, 1 },
    { { 700.0f, 0.0f, 4750.0f }, 1 },
};
s32 lbl_1_data_448[2] = { 7143426, 7143432 };
s32 lbl_1_data_450[2] = { 7143427, 7143433 };

s16 lbl_1_bss_4B6;
s16 lbl_1_bss_4B4;
s16 lbl_1_bss_4AC[4];
M650Cell lbl_1_bss_2AC[32][4];
s32 lbl_1_bss_2A8;

void fn_1_62E0(void)
{
    u16 cameraIDs[4] = { 1, 2, 4, 8 };
    s32 i;

    lbl_1_bss_2A8 = 0;
    memset(lbl_1_bss_2AC, 0, 512U);
    lbl_1_bss_4B6 = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_448[lbl_1_bss_0.night], 268435456, HEAP_MODEL));
    lbl_1_bss_4B4 = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_450[lbl_1_bss_0.night], 268435456, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_4B6, 65535U);
    Hu3DModelCameraSet(lbl_1_bss_4B4, 65535U);
    Hu3DModelAttrSet(lbl_1_bss_4B6, 1U);
    Hu3DModelAttrSet(lbl_1_bss_4B4, 1U);
    i = 0;
    while (i < 4) {
        lbl_1_bss_4AC[i] = Hu3DHookFuncCreate(fn_1_6478);
        Hu3DModelCameraSet(lbl_1_bss_4AC[i], cameraIDs[i]);
        i += 1;
    }
}

void fn_1_6478(HU3D_MODEL *modelP, Mtx *mtx)
{
    /* Retail initializes four rows through this three-row matrix.
     * Preserve the observed out-of-bounds behavior, not a portable fix. */
    Mtx spC;
    HSF_DATA *sp8;
    HU3D_MODEL *temp_r25;
    M650Cell *temp_r31;
    s32 var_r30;
    s32 var_r29;
    s32 var_r28;
    M650Player *temp_r26;
    s16 var_r27;
    f32 var_f31;
    f32 temp_f30;
    f32 temp_f29;
    HSF_OBJECT *temp_r24;
    HSF_OBJECT *temp_r23;
    HSF_OBJECT *temp_r22;

    Hu3DModelObjDrawInit();
    temp_r25 = &Hu3DData[lbl_1_bss_4B6];
    sp8 = temp_r25->hsf;
    temp_r24 = Hu3DModelObjPtrGet(lbl_1_bss_4B6, "m650_03N");
    temp_r23 = Hu3DModelObjPtrGet(lbl_1_bss_4B4, "m650_04N");
    temp_r22 = Hu3DModelObjPtrGet(lbl_1_bss_4B6, "shadow");
    var_r30 = 0;
    while (var_r30 < 32) {
        temp_r31 = &lbl_1_bss_2AC[var_r30][lbl_1_bss_2A8];
        if (temp_r31->state != 2) {
                switch (temp_r31->state) {
                case 2:
                    break;
                case 3:
                    var_r29 = 0;
                    while (var_r29 < 4) {
                        var_r28 = 0;
                        while (var_r28 < 4) {
                            if (var_r29 == var_r28) {
                                spC[var_r29][var_r28] = 1.0f;
                            } else {
                                spC[var_r29][var_r28] = 0.0f;
                            }
                            var_r28 += 1;
                        }
                        var_r29 += 1;
                    }
                    if (lbl_1_bss_0.recordChanged != 0) {
                        var_f31 = 350.0f;
                    } else {
                        var_f31 = 0.0f;
                    }
                    mtxTransCat(spC, lbl_1_data_1F8[var_r30].pos.x, 0.5f + lbl_1_data_1F8[var_r30].pos.y, var_f31 + lbl_1_data_1F8[var_r30].pos.z);
                    PSMTXConcat(*mtx, spC, spC);
                    if (lbl_1_data_1F8[var_r30].kind == 0) {
                        Hu3DModelObjPtrDraw(lbl_1_bss_4B6, temp_r24, spC);
                    } else {
                        Hu3DModelObjPtrDraw(lbl_1_bss_4B4, temp_r23, spC);
                    }
                    Hu3DModelObjPtrDraw(lbl_1_bss_4B6, temp_r22, spC);
                    break;
                default:
                    break;
                case 0:
                    var_r29 = 0;
                    while (var_r29 < 4) {
                        var_r28 = 0;
                        while (var_r28 < 4) {
                            if (var_r29 == var_r28) {
                                spC[var_r29][var_r28] = 1.0f;
                            } else {
                                spC[var_r29][var_r28] = 0.0f;
                            }
                            var_r28 += 1;
                        }
                        var_r29 += 1;
                    }
                    mtxTransCat(spC, lbl_1_data_248[var_r30].pos.x, 0.5f + lbl_1_data_248[var_r30].pos.y, lbl_1_data_248[var_r30].pos.z);
                    PSMTXConcat(*mtx, spC, spC);
                    if (lbl_1_data_248[var_r30].kind == 0) {
                        Hu3DModelObjPtrDraw(lbl_1_bss_4B6, temp_r24, spC);
                    } else {
                        Hu3DModelObjPtrDraw(lbl_1_bss_4B4, temp_r23, spC);
                    }
                    Hu3DModelObjPtrDraw(lbl_1_bss_4B6, temp_r22, spC);
                    break;
                case 1:
                    temp_r26 = &lbl_1_bss_28[lbl_1_bss_2A8];
                    if (lbl_1_data_248[var_r30].kind == 0) {
                        var_r27 = temp_r26->model7A;
                    } else {
                        var_r27 = temp_r26->model7C;
                    }
                    temp_f30 = Hu3DMotionTimeGet(var_r27);
                    temp_f29 = Hu3DMotionMaxTimeGet(var_r27);
                    if (temp_f30 >= temp_f29) {
                        if ((temp_r31->timer % 4) == 0) {
                            Hu3DModelAttrSet(var_r27, 1U);
                        } else if ((temp_r31->timer % 4) == 2) {
                            Hu3DModelAttrReset(var_r27, 1U);
                            Hu3DModelAttrReset(var_r27, 1U);
                        }
                        temp_r31->timer += 1;
                        if (temp_r31->timer >= 30) {
                            Hu3DModelAttrSet(var_r27, 1U);
                            temp_r31->state = 2;
                        }
                    }
                    break;
                }
        }
        var_r30 += 1;
    }
    lbl_1_bss_2A8 = (lbl_1_bss_2A8 + 1) % 4;
}

s16 fn_1_69B0(s16 player)
{
    M650Player *work = &lbl_1_bss_28[player];
    HuVecF pos;
    HuVecF reflected;
    HuVecF normal;
    s32 wallSounds[4] = { M650_EFFECT_2040, M650_EFFECT_2041, M650_EFFECT_2042, M650_EFFECT_2043 };
    s32 treeSounds[4] = { M650_EFFECT_2044, M650_EFFECT_2045, M650_EFFECT_2046, M650_EFFECT_2047 };
    s32 rockSounds[4] = { M650_EFFECT_2048, M650_EFFECT_2049, M650_EFFECT_2050, M650_EFFECT_2051 };
    int i;
    s16 model;

    Hu3DModelPosGet(work->model20, &pos);
    for (i = 0; i < 32; i++) {
        if (lbl_1_bss_2AC[i][player].state == 0 && fn_1_6EE4(&pos, &lbl_1_data_248[i].pos, 150.0f)) {
            if (lbl_1_data_248[i].kind == 0) {
                model = work->model7A;
            } else {
                model = work->model7C;
            }
            Hu3DModelPosSetV(model, &lbl_1_data_248[i].pos);
            Hu3DMotionTimeSet(model, 0.0f);
            Hu3DMotionSpeedSet(model, 1.0f);
            Hu3DModelAttrReset(model, 1);
            Hu3DModelPosSetV(work->model78, &lbl_1_data_248[i].pos);
            Hu3DMotionTimeSet(work->model78, 0.0f);
            Hu3DMotionSpeedSet(work->model78, 1.0f);
            Hu3DModelAttrReset(work->model78, 1);
            lbl_1_bss_2AC[i][player].state = 1;
            normal.x = pos.x - lbl_1_data_248[i].pos.x;
            normal.y = 0.0f;
            normal.z = pos.z - lbl_1_data_248[i].pos.z;
            C_VECReflect(&work->direction, &normal, &reflected);
            PSVECNormalize(&reflected, &work->reflected);
            if (lbl_1_data_248[i].kind == 0) {
                HuAudFXPlay(treeSounds[player]);
            } else {
                HuAudFXPlay(rockSounds[player]);
            }
            return 1;
        }
    }
    if (pos.x <= -1150.0f) {
        normal.x = 1.0f;
        normal.y = normal.z = 0.0f;
        C_VECReflect(&work->direction, &normal, &reflected);
        PSVECNormalize(&reflected, &work->reflected);
        HuAudFXPlay(wallSounds[player]);
        return 1;
    }
    if (pos.x >= 1150.0f) {
        normal.x = -1.0f;
        normal.y = normal.z = 0.0f;
        C_VECReflect(&work->direction, &normal, &reflected);
        PSVECNormalize(&reflected, &work->reflected);
        HuAudFXPlay(wallSounds[player]);
        return 1;
    }
    return 0;
}

s16 fn_1_6D88(s16 player, HuVecF *pos)
{
    HuVecF horizontal;
    M650Player *work;
    int candidate;

    work = &lbl_1_bss_28[player];
    horizontal.x = pos->x;
    horizontal.y = 0.0f;
    horizontal.z = pos->z;
    for (candidate = 0; candidate < 32; candidate++) {
        if (lbl_1_bss_2AC[candidate][player].state == 0 &&
            fn_1_6EE4(&horizontal, &lbl_1_data_248[candidate].pos, 90.0f)) {
            if (lbl_1_data_248[candidate].kind == 1 && pos->y < 160.0f) {
                return candidate;
            }
            if (lbl_1_data_248[candidate].kind == 0 && pos->y < 180.0f) {
                return candidate;
            }
        }
    }
    return -1;
}

s16 fn_1_6EE4(HuVecF *a, HuVecF *b, float radius)
{
    float distance;

    distance = PSVECSquareDistance(a, b);
    if (distance < radius * radius) {
        return 1;
    }
    return 0;
}

void fn_1_6F54(void)
{
    int row;
    int player;

    for (row = 0; row < 32; row++) {
        for (player = 0; player < 4; player++) {
            lbl_1_bss_2AC[row][player].state = 2;
        }
    }
    for (row = 0; row < 5; row++) {
        for (player = 0; player < 4; player++) {
            lbl_1_bss_2AC[row][player].state = 3;
        }
    }
}

s16 fn_1_7000(s16 unusedPlayer, HuVecF *pos, float angle, s16 candidate)
{
    HuVecF direction;
    HuVecF delta;
    HuVecF projection;
    HuVecF closest;
    float ratio;
    float factor;
    float threshold;
    float perpendicular;

    if (candidate >= 32) {
        return 0;
    }
    direction.x = 800.0 * sin((M_PI * angle) / 180.0);
    direction.y = 0.0f;
    direction.z = 800.0 * cos((M_PI * angle) / 180.0);
    delta.x = lbl_1_data_248[candidate].pos.x - pos->x;
    delta.y = 0.0f;
    delta.z = lbl_1_data_248[candidate].pos.z - pos->z;
    factor = PSVECDotProduct(&direction, &delta) / PSVECSquareMag(&direction);
    projection.x = factor * direction.x;
    projection.y = 0.0f;
    projection.z = factor * direction.z;
    ratio = sqrtf(PSVECSquareMag(&projection));
    closest.x = pos->x + projection.x;
    closest.y = 0.0f;
    closest.z = pos->z + projection.z;
    direction.x = lbl_1_data_248[candidate].pos.x - closest.x;
    direction.y = 0.0f;
    direction.z = lbl_1_data_248[candidate].pos.z - closest.z;
    perpendicular = sqrtf(PSVECSquareMag(&direction));
    ratio = (800.0f - ratio) / 800.0f;
    if (ratio < 0.0f) {
        ratio = 0.0f;
    }
    if (direction.x < 0.0f) {
        threshold = 150.0f + (45.0f * ratio);
    } else {
        threshold = 150.0f - (45.0f * ratio);
    }
    if (perpendicular <= threshold) {
        return 1;
    }
    return 0;
}
