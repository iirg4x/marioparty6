#include "REL/m656/m656.h"

u32 lbl_1_data_28[5] = { 9633792, 9306145, 9633818, 9633798, 0 };
s32 lbl_1_data_3C[8] = { 7536640, 7536641, 7536642, 7536643, 7536644, 7536645, 7536662, 7536663 };
f32 lbl_1_data_5C[8] = { 100.0f, 150.0f, 250.0f, 100.0f, 150.0f, 250.0f, 150.0f, 150.0f };
M656Entry10 lbl_1_data_7C[4][16] = {
    {
        { 0, 1500.0f, 0.0f, 0.0f },
        { 1, 2200.0f, 150.0f, 0.0f },
        { 5, 2800.0f, 200.0f, -0.5f },
        { 7, 3300.0f, 300.0f, -1.0f },
        { 2, 4200.0f, 200.0f, 0.0f },
        { 4, 4800.0f, -100.0f, 1.0f },
        { 1, 5400.0f, 200.0f, 0.0f },
        { 3, 5750.0f, 0.0f, -1.0f },
        { 5, 6450.0f, 300.0f, 0.0f },
        { 4, 6800.0f, 100.0f, 0.0f },
        { 6, 7650.0f, -250.0f, 0.0f },
        { 2, 8500.0f, 400.0f, 0.0f },
        { 1, 8700.0f, -300.0f, 0.0f },
        { 1, 9450.0f, -300.0f, 1.5f },
        { 5, 10350.0f, 250.0f, 0.5f },
        { 2, 10350.0f, -250.0f, -0.5f },
    },
    {
        { 0, 1500.0f, 0.0f, 0.0f },
        { 1, 2200.0f, 150.0f, 0.0f },
        { 5, 2800.0f, 200.0f, -0.5f },
        { 5, 3600.0f, 300.0f, 0.0f },
        { 2, 4200.0f, -300.0f, 0.5f },
        { 2, 4950.0f, -200.0f, 0.0f },
        { 1, 5700.0f, 300.0f, 0.0f },
        { 6, 6200.0f, 200.0f, 0.0f },
        { 2, 6750.0f, -100.0f, 0.0f },
        { 1, 7200.0f, 300.0f, 0.0f },
        { 5, 8250.0f, -200.0f, 0.0f },
        { 2, 8500.0f, 400.0f, 0.0f },
        { 3, 8750.0f, 0.0f, 0.0f },
        { 1, 9450.0f, -300.0f, 1.5f },
        { 5, 10350.0f, 250.0f, 0.5f },
        { 2, 10350.0f, -250.0f, -0.5f },
    },
    {
        { 0, 1500.0f, 0.0f, 0.0f },
        { 1, 2200.0f, 150.0f, 0.0f },
        { 5, 2800.0f, 200.0f, -0.5f },
        { 5, 3600.0f, 300.0f, 0.0f },
        { 7, 4200.0f, -300.0f, 0.5f },
        { 2, 4950.0f, -200.0f, 0.0f },
        { 1, 5700.0f, 300.0f, -1.0f },
        { 3, 5750.0f, 0.0f, -1.0f },
        { 7, 6300.0f, 300.0f, -1.0f },
        { 2, 7200.0f, 200.0f, 0.0f },
        { 4, 7800.0f, -100.0f, 1.0f },
        { 1, 8400.0f, 200.0f, 0.0f },
        { 5, 8750.0f, 0.0f, -1.0f },
        { 1, 9450.0f, -300.0f, 1.5f },
        { 5, 10350.0f, 250.0f, 0.5f },
        { 2, 10350.0f, -250.0f, -0.5f },
    },
    {
        { 0, 1500.0f, 0.0f, 0.0f },
        { 1, 2200.0f, 150.0f, 0.0f },
        { 5, 2800.0f, 200.0f, -0.5f },
        { 6, 3200.0f, 200.0f, 0.0f },
        { 2, 3750.0f, -100.0f, 0.0f },
        { 1, 4200.0f, 300.0f, 0.0f },
        { 5, 5250.0f, -200.0f, 0.0f },
        { 3, 5750.0f, 0.0f, 0.0f },
        { 5, 6450.0f, 300.0f, 0.0f },
        { 4, 6800.0f, 100.0f, 0.0f },
        { 6, 7650.0f, -250.0f, 0.0f },
        { 2, 8500.0f, 400.0f, 0.0f },
        { 1, 8700.0f, -300.0f, 0.0f },
        { 1, 9450.0f, -300.0f, 1.5f },
        { 5, 10350.0f, 250.0f, 0.5f },
        { 2, 10350.0f, -250.0f, -0.5f },
    },
};
char lbl_1_data_47C[20] = "656submarine-meteoA";
char lbl_1_data_490[20] = "656submarine-meteoB";
char lbl_1_data_4A4[20] = "656submarine-meteoC";
char lbl_1_data_4B8[20] = "656submarine-meteoD";
char lbl_1_data_4CC[20] = "656submarine-meteoE";
char lbl_1_data_4E0[20] = "656submarine-meteoF";
char lbl_1_data_4F4[20] = "656submarine-meteoG";
char lbl_1_data_508[20] = "656submarine-meteoH";
char lbl_1_data_51C[20] = "656submarine-meteoI";
char lbl_1_data_530[20] = "656submarine-meteoJ";
char lbl_1_data_544[20] = "656submarine-meteoK";
char lbl_1_data_558[20] = "656submarine-meteoL";
char lbl_1_data_56C[20] = "656submarine-meteoM";
char lbl_1_data_580[20] = "656submarine-meteoN";
char lbl_1_data_594[20] = "656submarine-meteoO";
char lbl_1_data_5A8[20] = "656submarine-meteoP";
char lbl_1_data_5BC[20] = "656submarine-meteoQ";
char lbl_1_data_5D0[20] = "656submarine-meteoR";
char lbl_1_data_5E4[20] = "656submarine-meteoS";
char lbl_1_data_5F8[20] = "656submarine-meteoT";
char *lbl_1_data_60C[20] = {
    lbl_1_data_47C,
    lbl_1_data_490,
    lbl_1_data_4A4,
    lbl_1_data_4B8,
    lbl_1_data_4CC,
    lbl_1_data_4E0,
    lbl_1_data_4F4,
    lbl_1_data_508,
    lbl_1_data_51C,
    lbl_1_data_530,
    lbl_1_data_544,
    lbl_1_data_558,
    lbl_1_data_56C,
    lbl_1_data_580,
    lbl_1_data_594,
    lbl_1_data_5A8,
    lbl_1_data_5BC,
    lbl_1_data_5D0,
    lbl_1_data_5E4,
    lbl_1_data_5F8,
};
s16 lbl_1_data_65C[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
char lbl_1_data_66C[14] = "RGBA8universe";
char lbl_1_data_67A[22] = "656submarine-drawPC01";
char lbl_1_data_690[22] = "656submarine-drawPC02";
char lbl_1_data_6A6[20] = "656submarine-losePC";
s16 lbl_1_data_6BA[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

const s32 lbl_1_rodata_88[2] = { 1, 2 };
M656Work60 *lbl_1_bss_AC[2];
M656Work4 *lbl_1_bss_A4[2];
M656Work50 *lbl_1_bss_24[2][16];
M656Work18 *lbl_1_bss_1C[2];
s16 lbl_1_bss_18;
s32 lbl_1_bss_10[2];
s32 lbl_1_bss_C;
u32 lbl_1_bss_8;

M656Work60 *fn_1_4F4C(s32 group)
{
    return lbl_1_bss_AC[group];
}

M656Work50 *fn_1_4F64(s16 group, s32 index)
{
    return lbl_1_bss_24[group][index];
}

M656Work18 *fn_1_4F88(s16 group)
{
    return lbl_1_bss_1C[group];
}

void fn_1_4FA4(s16 *players)
{
    s16 group;
    s16 player;
    int missing;
    for (group = 0; group < 2; group++) {
        missing = 1;
        for (player = 0; player < 4; player++) {
            if (group == GwPlayerConf[player].grpNo) {
                players[group] = player;
                missing = 0;
                break;
            }
        }
        if (missing) {
            players[0] = 0;
            players[1] = 1;
            return;
        }
    }
}

void fn_1_5050(OMOBJ *obj)
{
    s16 players[2];
    int group;
    M656SceneWork *work;
    s16 light;
    s16 pattern;
    work = obj->data;
    pattern = (s32)rand8() % 4;
    lbl_1_bss_18 = -1;
    lbl_1_bss_10[0] = lbl_1_bss_10[1] = 0;
    lbl_1_bss_C = 1;
    lbl_1_bss_8 = 0;
    work->stream = -1;
    work->fx = -1;
    fn_1_4FA4(players);
    light = Hu3DGLightCreate(0.0f, 1000.0f, 1000.0f, 0.0f, -1.0f, -1.0f, 255, 255, 255);
    Hu3DGLightStaticSet(light, 1);
    Hu3DGLightInfinitytSet(light);
    fn_1_16C(10, 2, fn_1_5780);
    fn_1_16C(10, 528, fn_1_5FF8);
    for (group = 0; group < 2; group++) {
        lbl_1_bss_A4[group] = fn_1_16C(10, 4, fn_1_8B4C);
        lbl_1_bss_A4[group]->group = group;
        lbl_1_bss_A4[group]->pattern = pattern;
    }
    for (group = 0; group < 2; group++) {
        lbl_1_bss_AC[group] = fn_1_16C(20, 96, fn_1_6990);
        lbl_1_bss_AC[group]->index = group;
        lbl_1_bss_AC[group]->playerNo = players[group];
        lbl_1_bss_AC[group]->group = group;
    }
    fn_1_20C(obj, fn_1_5338);
}

void fn_1_5338(OMOBJ *obj)
{
    M656SceneWork *temp_r31;

    temp_r31 = obj->data;
    if ((temp_r31->stream == -1) && (MgSeqModeGet() == 2)) {
        temp_r31->stream = HuAudSStreamPlay(87);
        temp_r31->fx = HuAudFXPlayPan(2084, 64);
    }
    if (MgSeqModeGet() == 5) {
        fn_1_20C(obj, fn_1_53C4);
        return;
    }
}

void fn_1_53C4(OMOBJ *obj)
{
    M656SceneWork *sp8;

    sp8 = obj->data;
    if (((s32) lbl_1_bss_10[0] != 0) || ((s32) lbl_1_bss_10[1] != 0)) {
        MgSeqModeNext();
        fn_1_20C(obj, fn_1_5434);
        return;
    }
}

void fn_1_5434(OMOBJ *obj)
{
    M656SceneWork *work = obj->data;
    s32 winners[2] = {-1, -1};
    int index;
    int count;
    count = 0;
    if (lbl_1_bss_10[0] != 0 && lbl_1_bss_10[1] != 0) {
        if (lbl_1_bss_C == 0) {
            lbl_1_bss_18 = (s32)rand8() % 2;
        }
    } else {
        lbl_1_bss_18 = lbl_1_bss_10[0] != 0 ? 0 : 1;
    }
    for (index = 0; index < 2; index++) {
        if ((s16)lbl_1_bss_18 == fn_1_4F4C(index)->group) {
            winners[count] = fn_1_4F4C(index)->charNo;
            GWMgCoinBonusSet(fn_1_4F4C(index)->playerNo, 10);
            count++;
        }
    }
    MgSeqWinnerSet(winners[0], winners[1], -1, -1);
    if (work->stream != -1) {
        HuAudSStreamFadeOut(work->stream, 100);
    }
    if (work->fx != -1) {
        HuAudFXFadeOut(work->fx, 1000);
    }
    fn_1_20C(obj, fn_1_5624);
}

void fn_1_5624(OMOBJ *obj)
{
    M656SceneWork *temp_r31;

    temp_r31 = obj->data;
    if (MgSeqModeGet() == 7) {
        WipeCreate(2, 0, 60);
        temp_r31->frame = 0;
        lbl_1_bss_8 = 1;
        fn_1_20C(obj, fn_1_56A0);
        return;
    }
}

void fn_1_56A0(OMOBJ *obj)
{
    M656SceneWork *work;
    work = obj->data;
    if (work->frame++ >= 60) {
        work->frame = 0;
        WipeCreate(1, 5, 60);
        lbl_1_bss_8 = 2;
        fn_1_20C(obj, fn_1_5724);
        return;
    }
}

void fn_1_5724(OMOBJ *obj)
{
    M656SceneWork *work;
    work = obj->data;
    if (work->frame++ >= 60) {
        MgSeqModeNext();
        fn_1_20C(obj, NULL);
        return;
    }
}

void fn_1_5780(OMOBJ *obj)
{
    HuVecF eye, target, up, axis, initialUp;
    float pitch, yaw, roll;
    /* Retail captures the allocated camera work without consuming it here. */
    M656CameraWork *work = obj->data;
    Hu3DCameraCreate(3);
    Hu3DCameraPerspectiveSet(3, 45.0f, 20.0f, 15000.0f, 2.2857144f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 280.0f, 0.0f, 1.0f);
    Hu3DCameraViewportSet(2, 0.0f, 216.0f, 640.0f, 280.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 640, 224);
    Hu3DCameraScissorSet(2, 0, 256, 640, 224);
    pitch = yaw = roll = 0.0f;
    eye.x = 500.0 * (sin(yaw) * cos(pitch));
    eye.y = 500.0 * -sin(pitch);
    eye.z = 700.0 + 500.0 * (cos(yaw) * cos(pitch));
    target.x = 0.0f;
    target.y = 0.0f;
    target.z = 700.0f;
    initialUp.x = sin(yaw) * sin(pitch);
    initialUp.y = cos(pitch);
    initialUp.z = cos(yaw) * sin(pitch);
    fn_1_4E0(&axis, &eye, &target);
    fn_1_518(&axis, &axis);
    up.x = initialUp.x * (axis.x * axis.x + (1.0f - axis.x * axis.x) * cos(roll))
        + initialUp.y * ((axis.x * axis.y) * (1.0 - cos(roll)) - axis.z * sin(roll))
        + initialUp.z * ((axis.x * axis.z) * (1.0 - cos(roll)) + axis.y * sin(roll));
    up.y = initialUp.y * (axis.y * axis.y + (1.0f - axis.y * axis.y) * cos(roll))
        + initialUp.x * ((axis.x * axis.y) * (1.0 - cos(roll)) + axis.z * sin(roll))
        + initialUp.z * ((axis.y * axis.z) * (1.0 - cos(roll)) - axis.x * sin(roll));
    up.z = initialUp.z * (axis.z * axis.z + (1.0f - axis.z * axis.z) * cos(roll))
        + (initialUp.x * ((axis.x * axis.z) * (1.0 - cos(roll)) - axis.y * sin(roll))
        + initialUp.y * ((axis.y * axis.z) * (1.0 - cos(roll)) + axis.x * sin(roll)));
    fn_1_518(&up, &up);
    Hu3DCameraPosSetV(1, &eye, &up, &target);
    Hu3DCameraPosSetV(2, &eye, &up, &target);
    fn_1_20C(obj, fn_1_5D20);
}

void fn_1_5D20(OMOBJ *obj)
{
    M656CameraWork *sp8;

    sp8 = obj->data;
    if (MgSeqModeGet() == 7) {
        fn_1_20C(obj, fn_1_5D70);
        return;
    }
}

void fn_1_5D70(OMOBJ *obj)
{
    M656CameraWork *work;
    HU3D_MODELID model;
    work = obj->data;
    if (lbl_1_bss_8 == 2U) {
        s16 motion;
        work->state = 0;
        Hu3DCameraPerspectiveSet(3, 45.0f, 20.0f, 15000.0f, 1.3333334f);
        Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
        Hu3DCameraScissorSet(1, 0, 0, 640, 480);
        motion = Hu3DMotionCreate(HuDataSelHeapReadNum(7536651, 268435456, HEAP_MODEL));
        model = Hu3DModelCameraCreate(motion, 1);
        Hu3DMotionSpeedSet(model, 1.0f);
        Hu3DCameraMotionStart(model, 1);
        Hu3DCameraViewportSet(2, 0.0f, 480.0f, 640.0f, 480.0f, 0.0f, 1.0f);
        Hu3DCameraScissorSet(2, 0, 480, 640, 480);
        fn_1_20C(obj, fn_1_5F38);
        return;
    }
}

void fn_1_5F38(OMOBJ *obj)
{
    M656CameraWork *sp8;

    sp8 = obj->data;
}

s16 fn_1_5F4C(s32 index)
{
    HU3D_MODELID model;
    if (lbl_1_data_65C[index] == -1) {
        model = lbl_1_data_65C[index] = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_3C[index], 268435456, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(lbl_1_data_65C[index]);
    }
    return model;
}

void fn_1_5FF8(OMOBJ *obj)
{
    static const HuVecF lbl_1_rodata_F0 = { -100.0f, 8000.0f, 0.0f };
    static const HuVecF lbl_1_rodata_FC = { 0.0f, 1.0f, 0.0f };
    static const HuVecF lbl_1_rodata_108 = { 0.0f, 0.0f, 0.0f };
    HuVecF shadowPos;
    HuVecF shadowUp;
    HuVecF shadowTarget;
    M656EnvironmentWork *work;
    int index;
    ANIMDATA *anim;

    work = obj->data;
    anim = HuSprAnimRead(HuDataSelHeapReadNum(7536650, 268435456, HEAP_MODEL));
    work->sprGroup = HuSprGrpCreate(11);
    HuSprGrpPosSet(work->sprGroup, 0.0f, 240.0f);
    for (index = 0; index < 10; index++) {
        HUSPRID sprite = HuSprCreate(anim, 50, 0);
        HuSprGrpMemberSet(work->sprGroup, index, sprite);
        HuSprPosSet(work->sprGroup, index, index * 64 + (index >= 8 ? 32 : 0), 0.0f);
    }
    anim = HuSprAnimRead(HuDataSelHeapReadNum(7536649, 268435456, HEAP_MODEL));
    HuSprGrpMemberSet(work->sprGroup, index, HuSprCreate(anim, 50, 0));
    HuSprPosSet(work->sprGroup, 10, 496.0f, 0.0f);
    work->model2 = Hu3DModelCreate(HuDataSelHeapReadNum(7536652, 268435456, HEAP_MODEL));
    work->model4 = Hu3DModelCreate(HuDataSelHeapReadNum(7536653, 268435456, HEAP_MODEL));
    {
        s16 motion = Hu3DJointMotion(work->model4, HuDataSelHeapReadNum(7536654, 268435456, HEAP_MODEL));
        Hu3DMotionSet(work->model4, motion);
    }
    Hu3DModelAttrSet(work->model4, 1073741825U);
    Hu3DModelLayerSet(work->model4, 0);
    Hu3DModelAttrSet(work->model2, 1U);
    Hu3DModelAttrSet(work->model4, 1U);
    Hu3DMotionSpeedSet(work->model4, 0.0f);
    shadowPos = lbl_1_rodata_F0;
    shadowUp = lbl_1_rodata_FC;
    shadowTarget = lbl_1_rodata_108;
    Hu3DShadowCreate(10.0f, 10.0f, 10000.0f);
    Hu3DShadowColSet(16, 16, 16);
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
    Hu3DShadowTPLvlSet(0.3f);
    Hu3DModelShadowMapSet(work->model2);
    work->model2E = Hu3DModelCreate(HuDataSelHeapReadNum(7536657, 268435456, HEAP_MODEL));
    {
        s16 motion = Hu3DJointMotion(work->model2E, HuDataSelHeapReadNum(7536659, 268435456, HEAP_MODEL));
        Hu3DMotionSet(work->model2E, motion);
    }
    Hu3DMotionSpeedSet(work->model2E, 0.0f);
    for (index = 0; index < 20; index++) {
        if (index == 0) {
            work->models[index] = fn_1_5F4C((s32)(rand8() % 100) > 50 ? 1 : 4);
        } else {
            work->models[index] = fn_1_5F4C((s32)rand8() % 8);
        }
        Hu3DModelCameraSet(work->models[index], 1);
        Hu3DModelAttrSet(work->models[index], 1);
        Hu3DModelHookSet(work->model2E, lbl_1_data_60C[index], work->models[index]);
        work->steps[index].x = 0.008726646259971648 * (double)(4.0f * frandf() - 2.0f);
        work->steps[index].y = 0.008726646259971648 * (double)(4.0f * frandf() - 2.0f);
        work->steps[index].z = 0.008726646259971648 * (double)(4.0f * frandf() - 2.0f);
    }
    fn_1_20C(obj, fn_1_65E0);
}

void fn_1_65E0(OMOBJ *obj)
{
    M656EnvironmentWork *work;
    int index;
    work = obj->data;
    if (lbl_1_bss_8 == 2U) {
        for (index = 0; index < 11; index++) {
            HuSprAttrSet(work->sprGroup, index, 4);
        }
        Hu3DModelAttrReset(work->model2, 1);
        Hu3DModelAttrReset(work->model4, 1);
        Hu3DMotionSpeedSet(work->model4, 1.0f);
        for (index = 0; index < 20; index++) {
            Hu3DModelAttrReset(work->models[index], 1);
        }
        Hu3DMotionSpeedSet(work->model2E, 1.0f);
        fn_1_20C(obj, fn_1_66D0);
        return;
    }
}

void fn_1_66D0(OMOBJ *obj)
{
    M656EnvironmentWork *temp_r31;
    s32 var_r30;

    temp_r31 = obj->data;
    var_r30 = 0;
    while (var_r30 < 20) {
        fn_1_4A8(&temp_r31->angles[var_r30], &temp_r31->angles[var_r30], &temp_r31->steps[var_r30]);
        Hu3DModelRotSet(temp_r31->models[var_r30], (f32) ((f64) (180.0f * temp_r31->angles[var_r30].x) / 3.141592653589793), (f32) ((f64) (180.0f * temp_r31->angles[var_r30].y) / 3.141592653589793), (f32) ((f64) (180.0f * temp_r31->angles[var_r30].z) / 3.141592653589793));
        var_r30 += 1;
    }
}

s32 fn_1_67E8(M656Work60 *work)
{
    M656Bounds playerBounds, itemBounds;
    M656Sphere player, item;
    int index, result;
    M656Work50 *entry;
    result = -1;
    player.center = work->pos;
    player.radius = 75.0f;
    fn_1_24C8(&playerBounds, &player);
    for (index = 0; index < 16; index++) {
        entry = fn_1_4F64(work->group, index);
        if (entry->state == 0) {
            item = entry->sphere;
            fn_1_24C8(&itemBounds, &item);
            if (fn_1_27C0(&playerBounds, &itemBounds) && fn_1_2BE8(&player, &item) <= 0.0f) {
                result = index;
                break;
            }
        }
    }
    return result;
}

void fn_1_690C(M656Work60 *work, s16 motion, float blend, u32 attr)
{
    if (work->motion != motion) {
        work->motion = motion;
        CharMotionShiftSet(work->charNo, work->childObj->mtnId[work->motion], 0.0f, blend, attr);
    }
}

void fn_1_6990(OMOBJ *obj)
{
    HU3D_MODELID model;
    M656Work60 *work;
    int index;
    ANIMDATA *anim;

    work = obj->data;
    work->charNo = GwPlayerConf[work->playerNo].charNo;
    work->motion = -1;
    work->state = 0;
    work->fraction = 0.0f;
    fn_1_2E0(&work->target, 0.0f, 0.0f, 0.0f);
    work->childObj = omAddObjEx(lbl_1_bss_0, 101, 1U, 5U, 0, NULL);
    omSetStatBit(work->childObj, 256U);
    model = work->childObj->mdlId[0] = CharModelCreate(work->charNo, 4);
    Hu3DModelCameraSet(model, (u16)lbl_1_rodata_88[work->group]);
    Hu3DModelLayerSet(model, 7);
    for (index = 0; index < 5; index++) {
        work->childObj->mtnId[index] = CharMotionCreate(work->charNo, lbl_1_data_28[index]);
    }
    CharMotionDataClose(work->charNo);
    fn_1_690C(work, 2, 0.0f, 1073741825U);
    Hu3DModelPosSet(model, -1000.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 90.0f, 0.0f);
    Hu3DModelCameraSet(model, (u16)lbl_1_rodata_88[work->group]);
    fn_1_2E0(&work->velocity, 0.0f, 0.0f, 0.0f);
    if (work->group == 0) {
        work->model50 = Hu3DModelCreate(HuDataSelHeapReadNum(7536646, 268435456, HEAP_MODEL));
    } else {
        work->model50 = Hu3DModelLink(fn_1_4F4C(0)->model50);
    }
    Hu3DModelAttrSet(work->model50, 1U);
    Hu3DModelCameraSet(work->model50, (u16)lbl_1_rodata_88[work->group]);
    Hu3DModelLayerSet(work->model50, 7);
    if (work->group == 0) {
        work->model38 = Hu3DModelCreate(HuDataSelHeapReadNum(7536647, 268435456, HEAP_MODEL));
    } else {
        work->model38 = Hu3DModelLink(fn_1_4F4C(0)->model38);
    }
    Hu3DModelLayerSet(work->model38, 4);
    Hu3DModelCameraSet(work->model38, (u16)lbl_1_rodata_88[work->group]);
    Hu3DModelPosSet(work->model38, 0.0f, 0.0f, -600.0f);
    work->texScroll = Hu3DTexScrollCreate(work->model38, lbl_1_data_66C);
    anim = HuSprAnimRead(HuDataSelHeapReadNum(work->charNo + 10158080, 268435456, HEAP_MODEL));
    work->sprGroup = HuSprGrpCreate(1);
    HuSprGrpPosSet(work->sprGroup, 0.0f, 240.0f);
    HuSprGrpMemberSet(work->sprGroup, 0, HuSprCreate(anim, 50, 0));
    HuSprPosSet(work->sprGroup, 0, 76.0f, (float)(work->group == 0 ? -16 : 16));
    work->model48 = Hu3DModelCreate(HuDataSelHeapReadNum(7536655, 268435456, HEAP_MODEL));
    Hu3DMotionSpeedSet(work->model48, 0.0f);
    work->model4A = Hu3DModelCreate(HuDataSelHeapReadNum(7536656, 268435456, HEAP_MODEL));
    Hu3DMotionSpeedSet(work->model4A, 0.0f);
    fn_1_20C(obj, fn_1_6E4C);
}

void fn_1_6E4C(OMOBJ *obj)
{
    M656Work60 *temp_r31;

    temp_r31 = obj->data;
    if (MgSeqModeGet() == 2) {
        temp_r31->timer = 0;
        fn_1_20C(obj, fn_1_6EA8);
        return;
    }
}

void fn_1_6EA8(OMOBJ *obj)
{
    M656Work60 *work;
    float position;
    work = obj->data;
    position = (float)work->timer / 240.0f;
    work->timer++;
    if (work->timer >= 240) {
        Hu3DModelPosSet(work->childObj->mdlId[0], 0.0f, 0.0f, 0.0f);
        fn_1_690C(work, 1, 10.0f, 1073741825U);
        fn_1_20C(obj, fn_1_7024);
        return;
    }
    position *= 1000.0f;
    position = -(1000.0f - position);
    Hu3DModelPosSet(work->childObj->mdlId[0], position, 0.0f, 0.0f);
}

void fn_1_7024(OMOBJ *obj)
{
    M656Work60 *temp_r31;

    temp_r31 = obj->data;
    if (MgSeqModeGet() == 5) {
        if (temp_r31->motion != 2) {
            temp_r31->motion = 2;
            CharMotionShiftSet(temp_r31->charNo, temp_r31->childObj->mtnId[temp_r31->motion], 0.0f, 10.0f, 1073741825U);
        }
        Hu3DTexScrollPosMoveSet(temp_r31->texScroll, -0.001f, 0.0f, 0.0f);
        temp_r31->state = 1;
        if (GwPlayerConf[temp_r31->playerNo].type != 0) {
            fn_1_20C(obj, fn_1_78C8);
            return;
        }
        fn_1_20C(obj, fn_1_7130);
        return;
    }
}

void fn_1_7130(OMOBJ *obj)
{
    static const Point3d lbl_1_rodata_168 = { 0.0f, 0.0f, 0.0f };
    HuVecF stick, delta, velocity, screen;
    M656Work60 *work = obj->data;
    int found;
    stick = lbl_1_rodata_168;
    if (lbl_1_bss_8 == 2) {
        fn_1_20C(obj, fn_1_8680);
        return;
    }
    stick.x = HuPadStkX[GwPlayerConf[work->playerNo].padNo];
    if (stick.x < 30.0f && stick.x > -30.0f) stick.x = 0.0f;
    stick.y = HuPadStkY[GwPlayerConf[work->playerNo].padNo];
    if (stick.y < 30.0f && stick.y > -30.0f) stick.y = 0.0f;
    if (stick.x || stick.y) {
        fn_1_518(&stick, &stick);
        lbl_1_bss_C = 0;
    }
    fn_1_6F8(&delta, &stick, 0.5f);
    velocity.z = 0.0f;
    velocity.x = work->velocity.x + delta.x;
    velocity.y = work->velocity.y + delta.y;
    if (PSVECMag(&velocity) > 10.0f) {
        fn_1_518(&velocity, &velocity);
        fn_1_6F8(&velocity, &velocity, 10.0f);
    }
    work->velocity.x = velocity.x;
    work->velocity.y = velocity.y;
    delta.x = velocity.x;
    delta.y = velocity.y;
    Hu3DModelPosGet(work->childObj->mdlId[0], &work->pos);
    work->pos.x += work->velocity.x;
    work->pos.y += work->velocity.y;
    if (work->pos.x > 940.0f || work->pos.x < -940.0f) work->pos.x -= work->velocity.x;
    if (work->pos.y > 240.0f || work->pos.y < -240.0f) work->pos.y -= work->velocity.y;
    Hu3DModelPosSetV(work->childObj->mdlId[0], &work->pos);
    work->progress += 1.0f;
    work->fraction = work->progress / 2350.0f;
    if (work->fraction >= 1.0f) work->fraction = 1.0f;
    HuSprPosSet(work->sprGroup, 0, 76.0f + 415.0f * work->fraction,
        (float)(work->group == 0 ? -16 : 16));
    if (fn_1_4F88(work->group)->active == 1) {
        if (fn_1_4F88(work->group)->pos.x < work->pos.x) {
            work->state = 3;
            lbl_1_bss_10[work->group] = 1;
            fn_1_20C(obj, fn_1_85F8);
            return;
        }
    }
    found = fn_1_67E8(work);
    if (found != -1) {
        float pan;
        fn_1_4F64(work->group, found)->state = 1;
        work->state = 2;
        work->timer = 0;
        Hu3DTexScrollPosMoveSet(work->texScroll, 0.0f, 0.0f, 0.0f);
        Hu3DMotionTimeSet(work->model50, 0.0f);
        Hu3DModelAttrReset(work->model50, 1);
        Hu3DModelPosSetV(work->model50, &work->pos);
        omVibrate(work->playerNo, 20, 7, 3);
        Hu3D3Dto2D(&work->pos, (s16)lbl_1_rodata_88[work->group], &screen);
        pan = 64.0f * (screen.x / 576.0f);
        pan = pan < 0.0f ? 0.0f : pan > 64.0f ? 64.0f : pan;
        pan += 32.0f;
        HuAudFXPlayPan(2083, (s16)pan);
        CharFXPlay(work->charNo, 582);
        fn_1_20C(obj, fn_1_8424);
        return;
    }
}

void fn_1_78C8(OMOBJ *obj)
{
    static const Point3d lbl_1_rodata_1A0 = { 0.0f, 0.0f, 0.0f };
    M656Work60 *work = obj->data;
    HuVecF stick = lbl_1_rodata_1A0;
    HuVecF delta, velocity;
    float difficultySpeed[4] = { 0.2f, 0.4f, 0.6f, 0.8f };
    M656Sphere player;
    int index, nearestDistance, found;
    M656Work50 *nearest;

    nearestDistance = 99999;
    nearest = NULL;
    if (lbl_1_bss_8 == 2) {
        fn_1_20C(obj, fn_1_8680);
        return;
    }
    player.center = work->pos;
    player.radius = 75.0f;
    for (index = 0; index < 16; index++) {
        M656Work50 *entry = fn_1_4F64(work->group, index);
        M656Sphere item = entry->sphere;
        if (entry->state == 0 && !(entry->pos.x > 940.0f + item.radius)
            && !(entry->pos.x < -(940.0f + item.radius))) {
            double distance = fn_1_2C8C(&player, &item);
            if (distance < (float)nearestDistance) {
                nearestDistance = distance;
                nearest = entry;
            }
        }
    }
    if (fn_1_4F88(work->group)->active == 1) {
        stick.x = difficultySpeed[GwPlayerConf[work->playerNo].comDif];
    } else if (nearest) {
        M656Sphere item;
        float radii[4] = { 200.0f, 250.0f, 300.0f, 320.0f };
            static const Point3d lbl_1_rodata_1CC = { 0.0f, 1.0f, 0.0f };
            static const Point3d lbl_1_rodata_1D8 = { 0.0f, -1.0f, 0.0f };
        M656Work50 *entry = nearest;
        HuVecF up, down, direction;
        player.radius = radii[GwPlayerConf[work->playerNo].comDif];
        item = entry->sphere;
        item.radius *= 1.5f;
        if (!(fn_1_2C8C(&player, &item) <= 1.0f)) goto wander;
        up = lbl_1_rodata_1CC;
        down = lbl_1_rodata_1D8;
        fn_1_4E0(&direction, &work->pos, &entry->pos);
        fn_1_518(&direction, &direction);
        if (entry->pos.y > 0.0f) {
            fn_1_4A8(&direction, &direction, &down);
        } else {
            fn_1_4A8(&direction, &direction, &up);
        }
        fn_1_518(&direction, &direction);
        stick.x = direction.x * difficultySpeed[GwPlayerConf[work->playerNo].comDif];
        stick.y = direction.y * difficultySpeed[GwPlayerConf[work->playerNo].comDif];
    } else {
        HuVecF target, direction;
        float distance;
wander:
        target = work->target;
        fn_1_4E0(&direction, &target, &work->pos);
        distance = fn_1_518(&direction, &direction);
        if (distance < 100.0f) {
            work->target.x = 2.0f * (940.0f * frandf()) - 940.0f;
            work->target.y = 2.0f * (240.0f * frandf()) - 240.0f;
        }
        stick.x = direction.x * (1.0f - difficultySpeed[GwPlayerConf[work->playerNo].comDif]);
        stick.y = direction.y * (1.0f - difficultySpeed[GwPlayerConf[work->playerNo].comDif]);
    }
    if (stick.x || stick.y) lbl_1_bss_C = 0;
    fn_1_6F8(&delta, &stick, 0.5f);
    velocity.z = 0.0f;
    velocity.x = work->velocity.x + delta.x;
    velocity.y = work->velocity.y + delta.y;
    if (PSVECMag(&velocity) > 10.0f) {
        fn_1_518(&velocity, &velocity);
        fn_1_6F8(&velocity, &velocity, 10.0f);
    }
    work->velocity.x = velocity.x;
    work->velocity.y = velocity.y;
    delta.x = velocity.x;
    delta.y = velocity.y;
    Hu3DModelPosGet(work->childObj->mdlId[0], &work->pos);
    work->pos.x += work->velocity.x;
    work->pos.y += work->velocity.y;
    if (work->pos.x > 940.0f || work->pos.x < -940.0f) work->pos.x -= work->velocity.x;
    if (work->pos.y > 240.0f || work->pos.y < -240.0f) work->pos.y -= work->velocity.y;
    Hu3DModelPosSetV(work->childObj->mdlId[0], &work->pos);
    work->progress += 1.0f;
    work->fraction = work->progress / 2350.0f;
    if (work->fraction >= 1.0f) work->fraction = 1.0f;
    HuSprPosSet(work->sprGroup, 0, 76.0f + 415.0f * work->fraction,
        (float)(work->group == 0 ? -16 : 16));
    if (fn_1_4F88(work->group)->active == 1) {
        if (fn_1_4F88(work->group)->pos.x < work->pos.x) {
            work->state = 3;
            lbl_1_bss_10[work->group] = 1;
            fn_1_20C(obj, fn_1_85F8);
            return;
        }
    }
    found = fn_1_67E8(work);
    if (found != -1) {
        HuVecF screen;
        float pan;
        fn_1_4F64(work->group, found)->state = 1;
        work->state = 2;
        work->timer = 0;
        Hu3DTexScrollPosMoveSet(work->texScroll, 0.0f, 0.0f, 0.0f);
        Hu3DMotionTimeSet(work->model50, 0.0f);
        Hu3DModelAttrReset(work->model50, 1);
        Hu3DModelPosSetV(work->model50, &work->pos);
        omVibrate(work->playerNo, 20, 7, 3);
        Hu3D3Dto2D(&work->pos, (s16)lbl_1_rodata_88[work->group], &screen);
        pan = 64.0f * (screen.x / 576.0f);
        pan = pan < 0.0f ? 0.0f : pan > 64.0f ? 64.0f : pan;
        pan += 32.0f;
        HuAudFXPlayPan(2083, (s16)pan);
        CharFXPlay(work->charNo, 582);
        fn_1_20C(obj, fn_1_8424);
        return;
    }
}

void fn_1_8424(OMOBJ *obj)
{
    M656Work60 *work;
    float angle;
    work = obj->data;
    if (lbl_1_bss_8 == 2U) {
        fn_1_20C(obj, fn_1_8680);
        return;
    }
    angle = (1.0f / 120.0f) * (120 - work->timer);
    angle *= 1080.0f;
    Hu3DModelRotSet(work->childObj->mdlId[0], 0.0f, 90.0f, angle);
    if (work->timer++ >= 120) {
        Hu3DModelRotSet(work->childObj->mdlId[0], 0.0f, 90.0f, 0.0f);
        work->state = 1;
        Hu3DTexScrollPosMoveSet(work->texScroll, -0.001f, 0.0f, 0.0f);
        fn_1_2E0(&work->velocity, 0.0f, 0.0f, 0.0f);
        if (GwPlayerConf[work->playerNo].type != 0) {
            fn_1_20C(obj, fn_1_78C8);
            return;
        }
        fn_1_20C(obj, fn_1_7130);
        return;
    }
}

void fn_1_85F8(OMOBJ *obj)
{
    M656Work60 *temp_r31;

    temp_r31 = obj->data;
    if ((u32) lbl_1_bss_8 == 2U) {
        fn_1_20C(obj, fn_1_8680);
        return;
    }
    temp_r31->pos.x += 10.0f;
    Hu3DModelPosSetV(*temp_r31->childObj->mdlId, &temp_r31->pos);
}

void fn_1_8680(OMOBJ *obj)
{
    M656Work60 *work;
    work = obj->data;
    HuSprAttrSet(work->sprGroup, 0, 4);
    Hu3DModelAttrSet(work->model38, 1);
    Hu3DModelPosSet(work->childObj->mdlId[0], 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(work->childObj->mdlId[0], 0.0f, 0.0f, 0.0f);
    Hu3DModelCameraSet(work->childObj->mdlId[0], 1);
    Hu3DModelCameraSet(work->model50, 1);
    Hu3DModelAttrSet(work->model50, 1);
    if (lbl_1_bss_18 == -1) {
        fn_1_690C(work, 2, 0.0f, 0);
        work->timer = 0;
        Hu3DMotionSpeedSet(work->model4A, 1.0f);
        if (work->group == 0) {
            Hu3DModelHookSet(work->model4A, lbl_1_data_67A, work->childObj->mdlId[0]);
            fn_1_20C(obj, fn_1_8A94);
            return;
        }
        Hu3DModelHookSet(work->model4A, lbl_1_data_690, work->childObj->mdlId[0]);
        fn_1_20C(obj, NULL);
        return;
    }
    if (lbl_1_bss_18 == work->group) {
        fn_1_690C(work, 0, 0.0f, 1073741825U);
        Hu3DModelShadowSet(work->childObj->mdlId[0]);
        fn_1_20C(obj, fn_1_8920);
        return;
    }
    Hu3DMotionSpeedSet(work->model48, 1.0f);
    Hu3DModelHookSet(work->model48, lbl_1_data_6A6, work->childObj->mdlId[0]);
    work->timer = 0;
    fn_1_20C(obj, fn_1_89F0);
}

void fn_1_8920(OMOBJ *obj)
{
    M656Work60 *temp_r31;

    temp_r31 = obj->data;
    if (MgSeqModeGet() == 8) {
        if (((s16) lbl_1_bss_18 == temp_r31->group) && (temp_r31->motion != 3)) {
            temp_r31->motion = 3;
            CharMotionShiftSet(temp_r31->charNo, temp_r31->childObj->mtnId[temp_r31->motion], 0.0f, 4.0f, 0U);
        }
        fn_1_20C(obj, fn_1_89DC);
        return;
    }
}

void fn_1_89DC(OMOBJ *obj)
{
    M656Work60 *sp8;

    sp8 = obj->data;
}

void fn_1_89F0(OMOBJ *obj)
{
    static const Point3d lbl_1_rodata_1F4 = { 830.5958f, -133.321f, -3667.3f };
    HuVecF pos;
    M656Work60 *work;
    work = obj->data;
    if (work->timer++ == 120) {
        pos = lbl_1_rodata_1F4;
        Hu3DMotionTimeSet(work->model50, 0.0f);
        Hu3DModelAttrReset(work->model50, 1U);
        Hu3DModelPosSetV(work->model50, &pos);
        fn_1_20C(obj, NULL);
        return;
    }
}

void fn_1_8A94(OMOBJ *obj)
{
    static const Point3d lbl_1_rodata_200 = { -108.0f, 150.0f, -2260.0f };
    HuVecF pos;
    M656Work60 *work;
    work = obj->data;
    if (work->timer++ == 120) {
        pos = lbl_1_rodata_200;
        Hu3DMotionTimeSet(work->model50, 0.0f);
        Hu3DModelAttrReset(work->model50, 1U);
        Hu3DModelPosSetV(work->model50, &pos);
        fn_1_20C(obj, NULL);
        return;
    }
}

void fn_1_8B38(OMOBJ *obj)
{
    void *sp8;

    sp8 = obj->data;
}

void fn_1_8B4C(OMOBJ *obj)
{
    M656Work4 *work;
    int index;
    work = obj->data;
    for (index = 0; index < 16; index++) {
        lbl_1_bss_24[work->group][index] = fn_1_16C(10, sizeof(M656Work50), fn_1_8E24);
        lbl_1_bss_24[work->group][index]->group = work->group;
        lbl_1_bss_24[work->group][index]->index = index;
        lbl_1_bss_24[work->group][index]->entry = &lbl_1_data_7C[work->pattern][index];
    }
    lbl_1_bss_1C[work->group] = fn_1_16C(10, sizeof(M656Work18), fn_1_9398);
    lbl_1_bss_1C[work->group]->group = work->group;
    fn_1_20C(obj, fn_1_8CBC);
}

void fn_1_8CBC(OMOBJ *obj)
{
    M656Work4 *work;
    work = obj->data;
    if (fn_1_4F4C(work->group)->fraction >= 1.0f) {
        fn_1_4F88(work->group)->active = 1;
        fn_1_20C(obj, fn_1_8D64);
        return;
    }
}

void fn_1_8D64(OMOBJ *obj)
{
    M656Work4 *sp8;

    sp8 = obj->data;
}

s16 fn_1_8D78(s32 index)
{
    HU3D_MODELID model;
    if (lbl_1_data_6BA[index] == -1) {
        model = lbl_1_data_6BA[index] = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_3C[index], 268435456, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(lbl_1_data_6BA[index]);
    }
    return model;
}

void fn_1_8E24(OMOBJ *obj)
{
    M656Work50 *work;
    work = obj->data;
    work->state = 0;
    work->model = fn_1_8D78(work->entry->kind);
    Hu3DModelCameraSet(work->model, (u16)lbl_1_rodata_88[work->group]);
    work->pos.x = work->entry->x;
    work->pos.y = work->entry->y;
    work->pos.z = 0.0f;
    Hu3DModelPosSetV(work->model, &work->pos);
    work->sphere.center = work->pos;
    work->sphere.radius = lbl_1_data_5C[work->entry->kind];
    if (work->group == 0) {
        work->axis.x = 2.0f * frandf() - 1.0f;
        work->axis.y = 2.0f * frandf() - 1.0f;
        work->axis.z = 2.0f * frandf() - 1.0f;
        fn_1_518(&work->axis, &work->axis);
        work->step = 0.008726646259971648 * (double)(6.0f * frandf() - 3.0f);
    } else {
        work->axis = fn_1_4F64(0, work->index)->axis;
        work->step = fn_1_4F64(0, work->index)->step;
    }
    fn_1_20C(obj, fn_1_90B0);
}

void fn_1_90B0(OMOBJ *obj)
{
    M656Work50 *sp8;

    sp8 = obj->data;
    if (MgSeqModeGet() == 5) {
        fn_1_20C(obj, fn_1_9100);
        return;
    }
}

void fn_1_9100(OMOBJ *obj)
{
    Mtx matrix;
    Qtrn rotation;
    HuVecF angles;
    M656Work50 *work;

    work = obj->data;
    if (lbl_1_bss_8 == 2) {
        Hu3DModelAttrSet(work->model, 1);
        fn_1_20C(obj, NULL);
        return;
    }
    if (fn_1_4F4C(work->group)->state == 1) {
        work->pos.x -= 5.0f;
    }
    if (work->pos.x <= 1500.0f) {
        work->angle += work->step;
        C_QUATRotAxisRad(&rotation, &work->axis, work->angle);
        PSMTXQuat(matrix, &rotation);
        Hu3DMtxRotGet(matrix, &angles);
        Hu3DModelRotSet(work->model, angles.x, angles.y, angles.z);
        if (fn_1_4F4C(work->group)->state == 1) {
            work->pos.y += work->entry->deltaY;
        }
    }
    work->sphere.center = work->pos;
    Hu3DModelPosSetV(work->model, &work->pos);
    if (work->state == 1) {
        fn_1_4E0(&work->velocity, &work->pos, &fn_1_4F4C(work->group)->pos);
        fn_1_518(&work->velocity, &work->velocity);
        fn_1_6F8(&work->velocity, &work->velocity, 20.0f);
        work->timer = 0;
        fn_1_20C(obj, fn_1_92E4);
        return;
    }
}

void fn_1_92E4(OMOBJ *obj)
{
    M656Work50 *work;
    work = obj->data;
    if (lbl_1_bss_8 == 2U) {
        Hu3DModelAttrSet(work->model, 1);
        fn_1_20C(obj, NULL);
        return;
    }
    fn_1_4A8(&work->pos, &work->pos, &work->velocity);
    Hu3DModelPosSetV(work->model, &work->pos);
    if (work->timer++ >= 120) {
        Hu3DModelAttrSet(work->model, 1);
        fn_1_20C(obj, NULL);
        return;
    }
}

void fn_1_9398(OMOBJ *arg0)
{
    M656Work18 *temp_r31;

    temp_r31 = arg0->data;
    temp_r31->angle = 0.0f;
    temp_r31->active = 0;
    fn_1_2E0(&temp_r31->pos, 1500.0f, 0.0f, 0.0f);
    temp_r31->model = Hu3DModelCreate(HuDataSelHeapReadNum(7536660, 268435456, HEAP_MODEL));
    Hu3DModelCameraSet(temp_r31->model, (u16) lbl_1_rodata_88[temp_r31->group]);
    Hu3DModelRotSet(temp_r31->model, temp_r31->angle, 0.0f, 0.0f);
    Hu3DModelAttrSet(temp_r31->model, 1U);
    Hu3DModelScaleSet(temp_r31->model, 1.3f, 1.3f, 1.3f);
    Hu3DModelPosSetV(temp_r31->model, &temp_r31->pos);
    fn_1_20C(arg0, fn_1_94C8);
}

void fn_1_94C8(OMOBJ *obj)
{
    M656Work18 *temp_r31;

    temp_r31 = obj->data;
    if ((u32) lbl_1_bss_8 == 2U) {
        Hu3DModelAttrSet(temp_r31->model, 1U);
        fn_1_20C(obj, NULL);
        return;
    }
    if (temp_r31->active != 0) {
        Hu3DModelAttrReset(temp_r31->model, 1U);
        fn_1_20C(obj, fn_1_9558);
        return;
    }
}

void fn_1_9558(OMOBJ *obj)
{
    M656Work18 *temp_r31;

    temp_r31 = obj->data;
    if ((u32) lbl_1_bss_8 == 2U) {
        Hu3DModelAttrSet(temp_r31->model, 1U);
        fn_1_20C(obj, NULL);
        return;
    }
    temp_r31->pos.x -= 5.0f;
    Hu3DModelPosSetV(temp_r31->model, &temp_r31->pos);
    temp_r31->angle += 1.0f;
    Hu3DModelRotSet(temp_r31->model, temp_r31->angle, 0.0f, 0.0f);
}
