#include "REL/m640/m640.h"

void fn_1_A0(void);
void fn_1_F0(s16 mode, s16 frame);
void fn_1_114(s16 mode, s16 frame);
void fn_1_13C(s16 mode, s16 frame);
void fn_1_170(s16 mode, s16 frame);
void fn_1_1B8(s16 mode, s16 frame);
void fn_1_220(s16 mode, s16 frame);
void fn_1_240(s16 mode, s16 frame);
void fn_1_268(s16 mode, s16 frame);
void fn_1_26C(s16 mode, s16 frame);
void fn_1_270(s16 frame);
void fn_1_60C(void);
void fn_1_868(s16 frame);
void fn_1_86C(void);
void fn_1_8D4(void);
s32 fn_1_BEC(void);
s16 fn_1_EC8(void);
s16 fn_1_112C(s16 frame);
void fn_1_15C8(void);
s32 fn_1_1C48(s32 frame);
void fn_1_26B8(s16 player, s16 motion);
void fn_1_2820(void);
s16 fn_1_2FF8(void);
void fn_1_3090(void);
s16 fn_1_3274(M640Player *player, HuVecF *rot);
s16 fn_1_33D0(float angle);
void fn_1_345C(s16 side, s16 slot, s16 index);
void fn_1_351C(s16 side, s16 slot);
void fn_1_355C(void);
void fn_1_36D8(M640Player *player);
s16 fn_1_37F4(s16 side, M640Player *player);
u16 fn_1_39F0(s16 side, M640Player *player);
void fn_1_3C24(s16 side, s16 slot);
void fn_1_4C90(s16 side);
void fn_1_4CD8(void);
void fn_1_4DDC(s16 side, s16 part);
void fn_1_4FFC(s16 side, s16 part);
void fn_1_519C(s16 side, s16 part);
void fn_1_52D0(OMOBJ *obj);
void fn_1_5A28(void);
void fn_1_5DF0(void);
void fn_1_60AC(OMOBJ *obj);
void fn_1_623C(void);

Point3d lbl_1_data_28 = { 500.0f, 2000.0f, 400.0f };
Point3d lbl_1_data_34 = { -5.0f, -20.0f, -4.0f };
GXColor lbl_1_data_40 = { 224, 224, 224, 224 };
M640MotionEntry lbl_1_data_44[5] = {
    { 9633792, 1073741825 },
    { 9633793, 1073741825 },
    { 9633798, 0 },
    { 9633799, 0 },
    { 9633803, 0 },
};
s32 lbl_1_data_6C[2] = { 6488068, 6488069 };
f32 lbl_1_data_74[2] = { -450.0f, 450.0f };
s32 lbl_1_data_7C[2] = { 6488074, 6488075 };
u32 lbl_1_data_84[12][2] = {
    { 0, 0 },
    { 0, 0 },
    { 6488090, 6488091 },
    { 6488095, 6488096 },
    { 0, 0 },
    { 0, 0 },
    { 6488110, 6488111 },
    { 6488115, 6488116 },
    { 0, 0 },
    { 0, 0 },
    { 6488130, 6488131 },
    { 6488135, 6488136 },
};
u32 lbl_1_data_E4[12][4] = {
    { 0, 6488081, 6488082, 6488083 },
    { 6488084, 0, 6488085, 6488086 },
    { 6488087, 6488088, 0, 6488089 },
    { 6488092, 6488093, 6488094, 0 },
    { 0, 6488101, 6488102, 6488103 },
    { 6488104, 0, 6488105, 6488106 },
    { 6488107, 6488108, 0, 6488109 },
    { 6488112, 6488113, 6488114, 0 },
    { 0, 6488121, 6488122, 6488123 },
    { 6488124, 0, 6488125, 6488126 },
    { 6488127, 6488128, 0, 6488129 },
    { 6488132, 6488133, 6488134, 0 },
};
s32 lbl_1_data_1A4[12] = {
    6488077,
    6488078,
    6488079,
    6488080,
    6488097,
    6488098,
    6488099,
    6488100,
    6488117,
    6488118,
    6488119,
    6488120,
};
s32 lbl_1_data_1D4[12] = {
    6488138,
    6488139,
    6488140,
    6488141,
    6488142,
    6488143,
    6488144,
    6488145,
    6488146,
    6488147,
    6488148,
    6488149,
};
M640Team lbl_1_bss_4FC[2];
M640Scene lbl_1_bss_4E8;
M640Player lbl_1_bss_3F8[4];
OMOBJ *lbl_1_bss_3F4;
OMOBJ *lbl_1_bss_3F0;
s16 lbl_1_bss_3E4[5];
s16 lbl_1_bss_3CC[3][4];
M640Piece lbl_1_bss_C[12][2];
void *lbl_1_bss_8;

void fn_1_86C(void)
{
    memset(&lbl_1_bss_4E8, 0, sizeof(lbl_1_bss_4E8));
    memset(lbl_1_bss_4FC, 0, sizeof(lbl_1_bss_4FC));
    lbl_1_bss_4E8.finishCount = 0;
    fn_1_8D4();
    fn_1_15C8();
    fn_1_2820();
    fn_1_5A28();
    fn_1_5DF0();
}

void fn_1_8D4(void)
{
    int i;
    OMOBJ *obj;
    Hu3DCameraCreate(1);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(1, 20.0f, 60.0f, 25000.0f, 1.2f);
    Hu3DCameraScissorSet(1, 0, 0, 640, 480);
    Hu3DCameraCreate(2);
    Hu3DCameraViewportSet(2, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(2, 20.0f, 60.0f, 25000.0f, 1.2f);
    Hu3DCameraScissorSet(2, 0, 480, 640, 480);
    obj = omAddObjEx(lbl_1_bss_4, 32730, 0, 0, -1, omOutViewMulti);
    obj->work[0] = 2;
    for (i = 0; i < 2; i++) {
        CenterM[i].x = i * 1300;
        CenterM[i].y = 150.0f - i * 75;
        CenterM[i].z = 0.0f;
        CRotM[i].x = -5.0f;
        CRotM[i].y = 0.0f;
        CRotM[i].z = 0.0f;
        CZoomM[i] = 1400.0f + i * 400;
    }
}

s32 fn_1_BEC(void)
{
    /* The retail initializer includes these three hook names and a null;
     * this local is copied even though no subsequent use survives. */
    char *hooks[] = { "itemhook_fx1", "itemhook_fx2", "itemhook_fx3", NULL };
    int i, j;
    float time;
    Hu3DMotionSpeedSet(lbl_1_bss_4FC[0].model3, 1.5f);
    Hu3DMotionSpeedSet(lbl_1_bss_4FC[0].model4, 1.5f);
    Hu3DMotionSpeedSet(lbl_1_bss_4FC[1].model3, 1.5f);
    Hu3DMotionSpeedSet(lbl_1_bss_4FC[1].model4, 1.5f);
    time = Hu3DMotionTimeGet(lbl_1_bss_4FC[0].model3);
    if (time >= 90.0f) {
        for (i = 0; i < 2; i++) {
            Hu3DModelHookReset(lbl_1_bss_4FC[i].model4);
            Hu3DModelAttrSet(lbl_1_bss_4FC[i].model3, 1073741828);
            Hu3DModelAttrSet(lbl_1_bss_4FC[i].model4, 1073741828);
            Hu3DMotionTimeSet(lbl_1_bss_4FC[i].model3, 90.0f);
            Hu3DMotionTimeSet(lbl_1_bss_4FC[i].model4, 90.0f);
            lbl_1_bss_4FC[i].activeRecord = -1;
            for (j = 0; j < 4; j++) {
                Hu3DModelAttrSet(lbl_1_bss_4FC[i].hookModels[j], 1);
            }
        }
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 12; j++) {
                Hu3DModelPosSet(lbl_1_bss_C[j][i].model, i * 1300, 400.0f, 200.0f);
                Hu3DModelAttrSet(lbl_1_bss_C[j][i].model, 1);
                lbl_1_bss_C[j][i].state = 0;
            }
        }
        return 1;
    }
    return 0;
}

s16 fn_1_EC8(void)
{
    char *hooks[] = { "itemhook_fx0", "itemhook_fx1", "itemhook_fx2", "itemhook_fx3" };
    int j, i;
    float time;
    Hu3DMotionSpeedSet(lbl_1_bss_4FC[0].model3, 2.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_4FC[0].model4, 2.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_4FC[1].model3, 2.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_4FC[1].model4, 2.0f);
    time = Hu3DMotionTimeGet(lbl_1_bss_4FC[0].model3);
    if (time <= 1.0f) {
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 4; j++) {
                if (j == 0) {
                    lbl_1_bss_4FC[i].auxModels[j] = Hu3DModelCreate(HuDataSelHeapReadNum(6488076, 268435456, HEAP_MODEL));
                } else {
                    lbl_1_bss_4FC[i].auxModels[j] = Hu3DModelCreate(HuDataSelHeapReadNum(6488073, 268435456, HEAP_MODEL));
                }
                Hu3DMotionSpeedSet(lbl_1_bss_4FC[i].auxModels[j], 0.0f);
                Hu3DMotionTimeSet(lbl_1_bss_4FC[i].auxModels[j], 0.0f);
                Hu3DModelHookSet(lbl_1_bss_4FC[i].model4, hooks[j], lbl_1_bss_4FC[i].auxModels[j]);
                Hu3DModelAttrSet(lbl_1_bss_4FC[i].auxModels[j], 1);
            }
        }
        return 1;
    }
    return 0;
}

s16 fn_1_112C(s16 frame)
{
    int i;
    float height, overlap, aspect;
    if ((float)frame == 60.0f) {
        Hu3DMotionSpeedSet(lbl_1_bss_4FC[0].model3, 1.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_4FC[1].model3, 1.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_4FC[0].model4, 1.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_4FC[1].model4, 1.0f);
    }
    lbl_1_bss_4E8.cameraProgress += 4.0f;
    {
        u16 cameras[] = { 1, 2 };
        height = 480.0f - 4.0f * frame;
        overlap = 40.0f * frame / 60.0f;
        for (i = 0; i < 2; i++) {
            CenterM[i].y -= 0.33333334f;
            CenterM[i].z += 1.6666666f;
            CRotM[i].x -= 0.016666668f;
            CZoomM[i] += 3.3333333f;
        }
        for (i = 0; i < 2; i++) {
            /* The viewport's vertical origin and extent share a two-word
             * stack-backed pair across both camera branches. */
            float viewport[2];
            switch (i) {
            case 0:
                viewport[0] = 0.0f;
                viewport[1] = height + overlap;
                Hu3DCameraScissorSet(cameras[i], 0, 0, 640, height - 2.0f);
                aspect = 576.0 / (1.0f + height);
                Hu3DCameraPerspectiveSet(cameras[i], 20.0f, 60.0f, 25000.0f, aspect);
                break;
            case 1:
                viewport[0] = height - overlap;
                viewport[1] = (480.0f - height) + overlap;
                Hu3DCameraScissorSet(cameras[i], 0, 2.0f + height, 640, (480.0f - height) - 2.0f);
                aspect = 2.4f;
                Hu3DCameraPerspectiveSet(cameras[i], 20.0f, 60.0f, 25000.0f, aspect);
                break;
            }
            Hu3DCameraViewportSet(cameras[i], 0.0f, viewport[0], 640.0f, viewport[1], 0.0f, 1.0f);
        }
    }
    if (lbl_1_bss_4E8.cameraProgress > 240.0f) {
        return 1;
    }
    return 0;
}

void fn_1_15C8(void)
{
    HuVecF pos, center, up;
    s16 model;
    int side, j;
    s16 light;
    u16 cameras[] = { 1, 2 };
    char *hooks[] = { "itemhook_fx0", "itemhook_fx1", "itemhook_fx2", "itemhook_fx3" };
    light = Hu3DGLightCreateV(&lbl_1_data_28, &lbl_1_data_34, &lbl_1_data_40);
    Hu3DGLightStaticSet(light, 1);
    Hu3DGLightInfinitytSet(light);
    Hu3DShadowCreate(30.0f, 20.0f, 13000.0f);
    pos.x = 500.0f;
    pos.y = 8000.0f;
    pos.z = 2000.0f;
    up.y = 1.0f;
    up.x = up.z = 0.0f;
    center.x = center.y = center.z = 0.0f;
    Hu3DShadowPosSet(&pos, &up, &center);
    for (side = 0; side < 2; side++) {
        model = Hu3DModelCreate(HuDataSelHeapReadNum(6488065, 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(model, cameras[side]);
        Hu3DModelShadowMapSet(model);
        Hu3DModelPosSet(model, side * 1300, 0.0f, 0.0f);
        lbl_1_bss_4FC[side].model0 = model;
        model = Hu3DModelCreate(HuDataSelHeapReadNum(6488066, 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(model, cameras[side]);
        Hu3DModelPosSet(model, side * 1300, 0.0f, 0.0f);
        lbl_1_bss_4FC[side].model1 = model;
        model = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_6C[side], 268435456, HEAP_MODEL));
        Hu3DModelPosSet(model, side * 1300, 0.0f, 0.0f);
        Hu3DModelCameraSet(model, cameras[side]);
        lbl_1_bss_4FC[side].model2 = model;
        model = Hu3DModelCreate(HuDataSelHeapReadNum(6488070, 268435456, HEAP_MODEL));
        Hu3DModelPosSet(model, side * 1300, 0.0f, 0.0f);
        Hu3DModelCameraSet(model, cameras[side]);
        lbl_1_bss_4FC[side].model3 = model;
        Hu3DModelShadowMapSet(model);
        Hu3DMotionSpeedSet(model, 0.0f);
        model = Hu3DModelCreate(HuDataSelHeapReadNum(6488071, 268435456, HEAP_MODEL));
        Hu3DModelPosSet(model, side * 1300, 0.0f, 0.0f);
        Hu3DModelCameraSet(model, cameras[side]);
        lbl_1_bss_4FC[side].model4 = model;
        Hu3DMotionSpeedSet(model, 0.0f);
        Hu3DModelShadowSet(model);
        model = Hu3DModelCreate(HuDataSelHeapReadNum(6488072, 268435456, HEAP_MODEL));
        Hu3DModelPosSet(model, side * 1300, 0.0f, 0.0f);
        Hu3DModelCameraSet(model, cameras[side]);
        Hu3DModelAttrSet(model, 1073741825);
        lbl_1_bss_4FC[side].model5 = model;
        for (j = 0; j < 4; j++) {
            if (j == 0) {
                model = Hu3DModelCreate(HuDataSelHeapReadNum(6488076, 268435456, HEAP_MODEL));
            } else {
                model = Hu3DModelCreate(HuDataSelHeapReadNum(6488073, 268435456, HEAP_MODEL));
            }
            Hu3DModelHookSet(lbl_1_bss_4FC[side].model4, hooks[j], model);
            Hu3DModelCameraSet(model, cameras[side]);
            Hu3DModelAttrSet(model, 1);
            Hu3DMotionSpeedSet(model, 0.0f);
            lbl_1_bss_4FC[side].hookModels[j] = model;
        }
        model = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_7C[1], 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(model, cameras[side]);
        Hu3DModelPosSet(model, side * 1300, 0.0f, 0.0f);
        Hu3DMotionSpeedSet(model, 0.0f);
        lbl_1_bss_4FC[side].model6 = model;
    }
}

s32 fn_1_1C48(s32 frame)
{
    OM_CAMERA_VIEW view;
    HuVecF rot, pos;
    int i;
    M640Team *team = &lbl_1_bss_4FC[lbl_1_bss_4E8.winnerSide];
    M640Piece *piece;
    int camera;
    float height, overlap, remaining, aspect;
    u16 cameras[] = { 1, 2 };

    if ((s16)frame == 0) {
        for (i = 0; i < 2; i++) {
            fn_1_26B8(team->players[i]->playerNo, 1);
        }
        view.center.x = lbl_1_bss_4E8.winnerSide * 1300;
        view.center.y = 100.0f;
        view.center.z = -320.0f;
        view.rot.x = -5.0f;
        view.rot.y = view.rot.z = 0.0f;
        view.zoom = 2500.0f;
        omCameraViewMoveSimpleMulti(cameras[lbl_1_bss_4E8.winnerSide], &view, 80);
    }
    if ((s16)frame < 25) {
        for (i = 0; i < 2; i++) {
            Hu3DModelRotGet(team->players[i]->model, &rot);
            rot.y += 3.0f - 6.0f * i;
            Hu3DModelRotSetV(team->players[i]->model, &rot);
        }
    } else if ((s16)frame < 66) {
        for (i = 0; i < 2; i++) {
            Hu3DModelPosGet(team->players[i]->model, &pos);
            pos.x += 5.0f - 10.0f * i;
            pos.z += 2.0f;
            if ((s16)frame > 48 && pos.y > 0.0f) {
                pos.y -= 4.0f;
            }
            Hu3DModelPosSetV(team->players[i]->model, &pos);
        }
    } else if ((s16)frame < 96) {
        for (i = 0; i < 2; i++) {
            Hu3DModelRotGet(team->players[i]->model, &rot);
            rot.y -= 3.0f - 6.0f * i;
            Hu3DModelRotSetV(team->players[i]->model, &rot);
        }
    } else if ((s16)frame == 96) {
        for (i = 0; i < 2; i++) {
            fn_1_26B8(team->players[i]->playerNo, 0);
        }
    }
    if ((s16)frame < 60) {
        u16 cameraIds[] = { 1, 2 };
        remaining = 4.0f * (60 - (s16)frame);
        if (lbl_1_bss_4E8.winnerSide != 0) {
            height = remaining;
        } else {
            height = 480.0f - remaining;
        }
        overlap = 40.0f * (60 - (s16)frame) / 60.0f;
        for (camera = 0; camera < 2; camera++) {
            float viewport[2];
            switch (camera) {
            case 0:
                viewport[0] = 0.0f;
                viewport[1] = height + overlap;
                Hu3DCameraScissorSet(cameraIds[camera], 0, 0, 640, height - 2.0f);
                aspect = 576.0 / (1.0f + height);
                break;
            case 1:
                viewport[0] = height - overlap;
                viewport[1] = (480.0f - height) + overlap;
                Hu3DCameraScissorSet(cameraIds[camera], 0, 2.0f + height, 640, (480.0f - height) - 2.0f);
                aspect = 576.0 / (1.0f + (480.0f - height));
                break;
            }
            if (camera == lbl_1_bss_4E8.winnerSide) {
                Hu3DCameraPerspectiveSet(cameraIds[camera], 20.0f, 60.0f, 25000.0f, aspect);
            }
            Hu3DCameraViewportSet(cameraIds[camera], 0.0f, viewport[0], 640.0f, viewport[1], 0.0f, 1.0f);
        }
    }
    if (lbl_1_bss_4E8.unkC != 0) {
        switch (team->parts[0]) {
        case 0:
            if ((s16)frame == 67) {
                for (i = 0; i < 4; i++) {
                    piece = &lbl_1_bss_C[team->parts[i]][lbl_1_bss_4E8.winnerSide];
                    if (piece->motionsA[1] != 0) {
                        Hu3DMotionStartEndSet(piece->model, 27.0f, 240.0f);
                    }
                }
            }
            break;
        case 4:
            if ((s16)frame == 100) {
                for (i = 0; i < 4; i++) {
                    piece = &lbl_1_bss_C[team->parts[i]][lbl_1_bss_4E8.winnerSide];
                    if (piece->motionsA[1] != 0) {
                        Hu3DMotionStartEndSet(piece->model, 60.0f, 240.0f);
                    }
                }
            }
            break;
        case 8:
            if ((s16)frame == 90) {
                for (i = 0; i < 4; i++) {
                    piece = &lbl_1_bss_C[team->parts[i]][lbl_1_bss_4E8.winnerSide];
                    if (piece->motionsA[1] != 0) {
                        Hu3DMotionStartEndSet(piece->model, 50.0f, 240.0f);
                    }
                }
            }
            break;
        }
    }
    if ((s16)frame == 40) {
        HuAudFXPlay(1917);
        if ((s16)(team->parts[0] / 4) == (s16)(team->parts[1] / 4)
            && (s16)(team->parts[0] / 4) == (s16)(team->parts[2] / 4)
            && (s16)(team->parts[0] / 4) == (s16)(team->parts[3] / 4)) {
            lbl_1_bss_4E8.unkC = 1;
        }
        if (lbl_1_bss_4E8.unkC != 0) {
            for (i = 0; i < 4; i++) {
                piece = &lbl_1_bss_C[team->parts[i]][lbl_1_bss_4E8.winnerSide];
                if (piece->motionsA[1] != 0) {
                    Hu3DMotionSet(piece->model, piece->motionsA[1]);
                    Hu3DMotionSpeedSet(piece->model, 1.0f);
                    Hu3DModelAttrSet(piece->model, 1073741825);
                }
            }
        } else {
            for (i = 0; i < 4; i++) {
                piece = &lbl_1_bss_C[team->parts[i]][lbl_1_bss_4E8.winnerSide];
                if (piece->motionsA[0] != 0) {
                    Hu3DMotionSet(piece->model, piece->motionsA[0]);
                    Hu3DMotionSpeedSet(piece->model, 1.0f);
                    Hu3DModelAttrSet(piece->model, 1073741825);
                }
            }
        }
    }
    if ((s16)frame == 150) {
        for (i = 0; i < 2; i++) {
            fn_1_26B8(team->players[i]->playerNo, 2);
        }
        return 1;
    }
    return 0;
}

void fn_1_26B8(s16 player, s16 motion)
{
    if (motion == 4) {
        CharMotionSpeedSet(lbl_1_bss_3F8[player].charNo, 1.5f);
        CharMotionSet(lbl_1_bss_3F8[player].charNo, lbl_1_bss_3F8[player].motions[motion]);
        Hu3DModelAttrReset(lbl_1_bss_3F8[player].model, 1073741825);
        return;
    }
    CharMotionSpeedSet(lbl_1_bss_3F8[player].charNo, 1.0f);
    CharMotionShiftSet(lbl_1_bss_3F8[player].charNo, lbl_1_bss_3F8[player].motions[motion], 0.0f, 8.0f, lbl_1_data_44[motion].attr);
}

void fn_1_2820(void)
{
    HuVecF lightPos, lightDir;
    int player;
    s16 model;
    int slot;
    s16 side, character, invalid;
    s16 motion;
    float angle;
    u16 cameras[] = { 1, 2 };
    char *slotHooks[] = { "R_slot_hook", "L_slot_hook" };
    char *smokeHooks[] = { "R_lump_hook", "L_lump_hook" };
    s16 sides[] = { 0, 0, 1, 1 };
    s16 counts[2];
    GXColor color;
    invalid = counts[0] = counts[1] = 0;
    memset(lbl_1_bss_3F8, 0, sizeof(lbl_1_bss_3F8));
    for (player = 0; player < 4; player++) {
        if (GwPlayerConf[player].grpNo >= 2) {
            invalid = 1;
            break;
        }
        counts[GwPlayerConf[player].grpNo]++;
    }
    for (player = 0; player < 4; player++) {
        lbl_1_bss_3F8[player].playerNo = player;
        character = GwPlayerConf[player].charNo;
        lbl_1_bss_3F8[player].charNo = character;
        lbl_1_bss_3F8[player].padNo = GwPlayerConf[player].padNo;
        lbl_1_bss_3F8[player].sfx28 = lbl_1_bss_3F8[player].sfx24 = -1;
        if (invalid || counts[0] > 2 || counts[1] > 2) {
            side = sides[player];
            lbl_1_bss_3F8[player].side = side;
        } else {
            side = GwPlayerConf[player].grpNo;
            lbl_1_bss_3F8[player].side = side;
        }
        if (GwPlayerConf[player].type == 1) {
            lbl_1_bss_3F8[player].comDif = GwPlayerConf[player].comDif;
        } else {
            lbl_1_bss_3F8[player].comDif = -1;
        }
        model = CharModelCreate(character, 4);
        lbl_1_bss_3F8[player].model = model;
        Hu3DModelShadowSet(model);
        Hu3DModelCameraSet(model, cameras[side]);
        for (slot = 0; slot < 5; slot++) {
            motion = CharMotionCreate(character, lbl_1_data_44[slot].file);
            lbl_1_bss_3F8[player].motions[slot] = motion;
        }
        CharMotionShiftSet(character, lbl_1_bss_3F8[player].motions[0], 0.0f, 0.0f, lbl_1_data_44[0].attr);
        for (slot = 0; slot < 2; slot++) {
            if (lbl_1_bss_4FC[side].players[slot] == NULL) {
                lbl_1_bss_4FC[side].players[slot] = &lbl_1_bss_3F8[player];
                break;
            }
        }
        lbl_1_bss_3F8[player].slot = slot;
        Hu3DModelPosSet(model, lbl_1_data_74[slot] + side * 1300, 20.0f, 200.0f);
        Hu3DModelRotSet(model, 0.0f, 10.0f - 20.0f * slot, 0.0f);
        model = Hu3DModelCreate(HuDataSelHeapReadNum(6488064, 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(model, cameras[side]);
        Hu3DModelHookSet(lbl_1_bss_4FC[side].model0, slotHooks[slot], model);
        Hu3DMotionSpeedSet(model, 0.0f);
        Hu3DMotionTimeSet(model, 45.0f);
        angle = 30.0f * frandmod(4);
        Hu3DModelRotSet(model, 0.0f, angle, 0.0f);
        lbl_1_bss_3F8[player].model2C = model;
        model = Hu3DModelCreate(HuDataSelHeapReadNum(6488137, 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(model, cameras[side]);
        Hu3DModelHookSet(lbl_1_bss_4FC[side].model0, smokeHooks[slot], model);
        Hu3DModelAttrSet(model, 1073741825);
        Hu3DMotionSpeedSet(model, 0.0f);
        lbl_1_bss_3F8[player].model20 = model;
        model = Hu3DModelCreate(HuDataSelHeapReadNum(6488067, 268435456, HEAP_MODEL));
        Hu3DModelPosSet(model, side * 1300, 0.0f, 0.0f);
        Hu3DModelCameraSet(model, cameras[side]);
        Hu3DMotionSpeedSet(model, 0.0f);
        lbl_1_bss_3F8[player].model1E = model;
        if (slot != 0) {
            Hu3DModelScaleSet(model, -1.0f, 1.0f, 1.0f);
            Hu3DModelAttrSet(model, 8388608);
        }
    }
    lightPos.x = 0.0f;
    lightPos.y = 400.0f;
    lightPos.z = 1000.0f;
    lightDir.x = 0.0f;
    lightDir.y = -0.1f;
    lightDir.z = -1.0f;
    color.r = color.g = color.b = color.a = 192;
    CharLightCreateV(&lightPos, &lightDir, &color);
    CharLightInfinitytSet();
}

s16 fn_1_2FF8(void)
{
    int i;
    if (lbl_1_bss_4E8.finishCount) {
        return lbl_1_bss_4E8.finishCount;
    }
    for (i = 0; i < 2; i++) {
        if (lbl_1_bss_4FC[i].parts[3]) {
            lbl_1_bss_4E8.finishCount++;
            lbl_1_bss_4E8.winnerSide = i;
        }
    }
    return lbl_1_bss_4E8.finishCount;
}

void fn_1_3090(void)
{
    M640Player *player;
    int i;
    float time, maxTime;
    for (i = 0; i < 4; i++) {
        player = &lbl_1_bss_3F8[i];
        time = Hu3DMotionTimeGet(player->model1E);
        if (time > 60.0f) {
            Hu3DModelAttrSet(player->model1E, 1073741828);
            Hu3DModelAttrReset(player->model1E, 1073741825);
            Hu3DMotionTimeSet(player->model1E, 60.0f);
        } else if (time) {
            Hu3DModelAttrSet(player->model1E, 1073741828);
            Hu3DModelAttrReset(player->model1E, 1073741825);
            Hu3DMotionTimeSet(player->model1E, time);
        }
        time = Hu3DMotionTimeGet(player->model20);
        maxTime = Hu3DMotionMaxTimeGet(player->model20);
        if (time >= maxTime) {
        } else if (time > 60.0f) {
            Hu3DMotionStartEndSet(player->model20, 81.0f, 140.0f);
            Hu3DModelAttrReset(player->model20, 1073741825);
            Hu3DMotionTimeSet(player->model20, 81.0f);
        } else if (time) {
            Hu3DModelAttrSet(player->model20, 1073741828);
            Hu3DModelAttrReset(player->model20, 1073741825);
            Hu3DMotionTimeSet(player->model20, 0.0f);
        }
        Hu3DMotionTimeSet(player->model2C, 45.0f);
    }
}

s16 fn_1_3274(M640Player *player, HuVecF *rot)
{
    float angle = player->angle;
    float distance;
    if (rot->y == angle) {
        /* The original equality path leaves the scalar result undefined. */
        return;
    }
    distance = fabs(rot->y - angle);
    if (distance > 1.8f) {
        if (rot->y < player->angle) {
            rot->y += 1.8f;
        } else {
            rot->y -= 1.8f;
        }
    } else {
        rot->y = player->angle;
        return 1;
    }
    return 0;
}

s16 fn_1_33D0(float angle)
{
    angle += 15.0f;
    while (angle < 0.0f) {
        angle += 120.0f;
    }
    while (angle >= 120.0f) {
        angle -= 120.0f;
    }
    return angle / 30.0f;
}

void fn_1_345C(s16 side, s16 slot, s16 index)
{
    M640Player *player;
    player = lbl_1_bss_4FC[side].players[slot];
    {
        s16 times[] = { 5, 15, 25, 35, 45 };
        Hu3DMotionTimeSet(player->model2C, times[index]);
    }
}

void fn_1_351C(s16 side, s16 slot)
{
    M640Player *player;
    player = lbl_1_bss_4FC[side].players[slot];
    player->state = 1;
}

void fn_1_355C(void)
{
    HuVecF rot;
    M640Player *player;
    int i;
    for (i = 0; i < 4; i++) {
        player = &lbl_1_bss_3F8[i];
        Hu3DModelRotGet(player->model2C, &rot);
        if (rot.y < 0.0f) {
            rot.y += 360.0f;
        }
        if (player->state >= 1 && player->state <= 4) {
            if (player->speed > 0.1f) {
                player->speed -= 0.1f;
            } else {
                player->speed = 0.0f;
            }
        }
        if (player->sfx28 != -1) {
            HuAudFXStop(player->sfx28);
            player->sfx28 = -1;
        }
        if (player->sfx24 != -1) {
            s16 sounds[] = { 1928, 1929, 1930, 1931 };
            HuAudFXPlay(sounds[player->slot + player->side * 2]);
            player->sfx24 = -1;
        }
        Hu3DModelRotSet(player->model2C, rot.x, rot.y - player->speed, rot.z);
    }
}

void fn_1_36D8(M640Player *player)
{
    s16 base[] = { 140, 100, 60, 10 };
    s16 range[] = { 120, 40, 20, 40 };
    s16 chance[] = { 80, 60, 20, 5 };
    s16 roll;
    if (player->comDif != -1) {
        player->delay = base[player->comDif] + frandmod(range[player->comDif]);
        roll = frandmod(100);
        if (roll < chance[player->comDif]) {
            player->error = 1;
        } else {
            player->error = 0;
        }
    }
}

s16 fn_1_37F4(s16 side, M640Player *player)
{
    HuVecF rot;
    s16 bucket;
    float angle;
    s16 buckets[] = { 0, 3, 2, 1 };
    s16 delays[] = { 8, 6, 2, 1 };
    if (player->delay != 0) {
        player->delay--;
        return 0;
    }
    Hu3DModelRotGet(player->model2C, &rot);
    angle = rot.y - 34.2f;
    bucket = fn_1_33D0(angle);
    bucket = buckets[bucket];
    if (player->error != 0) {
        if (bucket == lbl_1_bss_4FC[side].targetBucket) {
            return 0;
        }
        return 1;
    }
    if (bucket == lbl_1_bss_4FC[side].targetBucket) {
        return 1;
    }
    return 0;
}

u16 fn_1_39F0(s16 side, M640Player *player)
{
    if (player->comDif == -1) {
        return HuPadBtn[player->padNo] & 256;
    }
    if (fn_1_37F4(side, player) != 0) {
        return 256;
    }
    return 0;
}

void fn_1_3C24(s16 side, s16 slot)
{
    HuVecF rot;
    M640Player *player = lbl_1_bss_4FC[side].players[slot];
    s16 buckets[] = { 0, 3, 2, 1 };
    M640Team *team = &lbl_1_bss_4FC[side];
    M640Piece *piece;
    s16 time;

    Hu3DModelRotGet(player->model2C, &rot);
    if (rot.y < 0.0f) {
        rot.y += 360.0f;
    }
    switch (player->state) {
    case 1:
        {
            s16 sounds[] = { 1932, 1933, 1934, 1935 };
            player->sfx28 = HuAudFXPlay(sounds[slot + side * 2]);
        }
        player->state = 2;
        {
            s16 sounds[] = { 1920, 1921, 1922, 1923 };
            player->sfx24 = HuAudFXPlay(sounds[slot + side * 2]);
        }
        Hu3DMotionTimeSet(player->model1E, 0.0f);
        Hu3DMotionStartEndSet(player->model1E, 0.0f, Hu3DMotionMaxTimeGet(player->model1E));
        Hu3DModelAttrReset(player->model1E, 1073741828);
        Hu3DMotionSpeedSet(player->model1E, 1.0f);
        Hu3DMotionSpeedSet(player->model20, 1.0f);
        Hu3DMotionTimeSet(player->model20, 0.0f);
        Hu3DModelAttrReset(player->model20, 1073741828);
        Hu3DMotionStartEndSet(player->model20, 0.0f, 80.0f);
        fn_1_345C(side, slot, lbl_1_bss_4FC[side].targetBucket);
        break;
    case 2:
        if (player->speed < 1.8f) {
            player->speed += 0.1;
        } else {
            player->speed = 1.8f;
        }
        time = Hu3DMotionTimeGet(player->model1E);
        if (time == 60) {
            HuAudFXStop(player->sfx28);
            player->sfx28 = -1;
            player->state = 3;
            Hu3DMotionStartEndSet(player->model1E, 61.0f, 80.0f);
            Hu3DMotionStartEndSet(player->model20, 61.0f, 80.0f);
            Hu3DModelAttrSet(player->model1E, 1073741825);
            Hu3DModelAttrSet(player->model20, 1073741825);
            fn_1_36D8(player);
        } else if (time == 40) {
            omVibrate(player->playerNo, 20, 4, 4);
        }
        break;
    case 3:
        if (fn_1_39F0(side, player) != 0) {
            player->unk1C = 0;
            fn_1_26B8(player->playerNo, 4);
            player->state = 4;
        }
        break;
    case 4:
        player->unk1C++;
        if ((s16)CharModelTimingHookNoGet(player->charNo) != 0) {
            player->sfx24 = -1;
            player->speed = 0.0f;
            player->unk38 = fn_1_33D0(rot.y);
            player->unk38 = buckets[player->unk38];
            player->angle = (s32)((15.0f + rot.y) / 30.0f) * 30;
            if (player->angle == 0 && rot.y >= 345.0f) {
                player->angle = 360;
            }
            Hu3DModelAttrReset(player->model1E, 1073741825);
            Hu3DMotionTimeSet(player->model1E, 81.0f);
            Hu3DMotionStartEndSet(player->model1E, 81.0f, 110.0f);
            Hu3DModelAttrReset(player->model20, 1073741825);
            Hu3DMotionStartEndSet(player->model20, 81.0f, 140.0f);
            player->state = 5;
            if (player->unk38 == lbl_1_bss_4FC[side].targetBucket) {
                fn_1_4FFC(side, player->unk38);
                {
                    s16 sounds[] = { 1924, 1925, 1926, 1927 };
                    HuAudFXPlay(sounds[slot + side * 2]);
                }
                player->sfx24 = -1;
            } else {
                fn_1_4DDC(side, player->unk38);
                {
                    s16 sounds[] = { 1928, 1929, 1930, 1931 };
                    HuAudFXPlay(sounds[slot + side * 2]);
                }
                player->sfx24 = -1;
            }
        }
        break;
    case 5:
        if (CharMotionTimeGet(player->charNo) >= 62.0f) {
            fn_1_26B8(player->playerNo, 0);
        }
        if (Hu3DMotionTimeGet(player->model1E) >= 109.0f) {
            s16 sounds[] = { 1932, 1933, 1934, 1935 };
            player->sfx28 = HuAudFXPlay(sounds[slot + side * 2]);
            Hu3DMotionStartEndSet(player->model1E, 0.0f, 60.0f);
            Hu3DMotionTimeSet(player->model1E, 60.0f);
            Hu3DModelAttrSet(player->model1E, 1073741828);
            player->state = 6;
        }
        fn_1_3274(player, &rot);
        break;
    case 6:
        if (fn_1_3274(player, &rot) != 0) {
            fn_1_345C(side, slot, 4);
        }
        if (team->activeRecord != -1
            && ((piece = &lbl_1_bss_C[team->activeRecord][side], piece->state == 0)
                || piece->state == 3)) {
            HuAudFXStop(player->sfx28);
            player->sfx28 = -1;
            if (team->targetBucket == player->unk38) {
                team->parts[team->targetBucket] = team->activeRecord;
                team->targetBucket++;
            }
            team->activeRecord = -1;
            if (team->targetBucket < 4) {
                team->players[1 - slot]->state = 1;
            }
            player->state = 0;
            fn_1_345C(side, slot, 4);
            HuAudFXStop(player->sfx28);
            player->sfx28 = -1;
        }
        break;
    }
    Hu3DModelRotSet(player->model2C, rot.x, rot.y - player->speed, rot.z);
}

void fn_1_4C90(s16 side)
{
    int i;
    for (i = 0; i < 2; i++) {
        fn_1_3C24(side, i);
    }
}

void fn_1_4CD8(void)
{
    int side, slot;
    for (side = 0; side < 2; side++) {
        for (slot = 0; slot < 2; slot++) {
            fn_1_3C24(side, slot);
        }
    }
    if (fn_1_2FF8()) {
        MgSeqModeNext();
    }
}

void fn_1_4DDC(s16 side, s16 part)
{
    M640Piece *piece;
    s16 variant;
    M640Team *team;
    int i;
    team = &lbl_1_bss_4FC[side];
    variant = frandmod(3);
    for (i = 0; i < 3; i++) {
        if (lbl_1_bss_C[part + variant * 4][side].state == 0) {
            break;
        }
        variant = (variant + 1) % 3;
    }
    if (i == 3) {
        OSReport("no part is waiting\n");
        return;
    }
    Hu3DMotionSpeedSet(lbl_1_bss_4FC[side].model6, 1.0f);
    Hu3DMotionTimeSet(lbl_1_bss_4FC[side].model6, 0.0f);
    piece = &lbl_1_bss_C[part + variant * 4][side];
    if (piece->motionsB[team->targetBucket] != 0) {
        Hu3DMotionSet(piece->model, piece->motionsB[team->targetBucket]);
        Hu3DMotionSpeedSet(piece->model, 1.0f);
        piece->state = 4;
        piece->variant = variant;
        Hu3DModelAttrReset(piece->model, 1);
        Hu3DModelPosSet(piece->model, side * 1300, 0.0f, 0.0f);
        team->activeRecord = part + variant * 4;
    }
    piece->variant = variant;
    piece->part = part;
}

void fn_1_4FFC(s16 side, s16 part)
{
    M640Piece *piece;
    s16 variant;
    int i;
    M640Team *team = &lbl_1_bss_4FC[side];
    variant = frandmod(3);
    for (i = 0; i < 3; i++) {
        if (lbl_1_bss_C[part + variant * 4][side].state == 0) {
            break;
        }
        variant = (variant + 1) % 3;
    }
    if (i == 3) {
        OSReport("no part is waiting\n");
        return;
    }
    piece = &lbl_1_bss_C[part + variant * 4][side];
    team->activeRecord = part + variant * 4;
    piece->state = 1;
    piece->variant = variant;
    piece->part = part;
    Hu3DModelAttrReset(piece->model, 1);
    Hu3DModelPosSet(piece->model, side * 1300, 400.0f, 200.0f);
    piece->frame = 0;
    piece->speed = 1.0f;
}

void fn_1_519C(s16 side, s16 part)
{
    M640Piece *piece;
    int i;
    s16 variant;
    M640Team *team = &lbl_1_bss_4FC[side];
    if (part == 0) {
        variant = frandmod(3);
    } else {
        for (i = 0; i < 3; i++) {
            if (lbl_1_bss_C[i * 4][side].state != 0) {
                break;
            }
        }
        if (i < 3) {
            variant = (i + part) % 3;
        } else {
            variant = 0;
        }
    }
    piece = &lbl_1_bss_C[part + variant * 4][side];
    piece->state = 5;
    piece->speed = 1.0f;
    piece->part = part;
    Hu3DModelAttrReset(piece->model, 1);
}

void fn_1_52D0(OMOBJ *obj)
{
    HuVecF pos;
    M640Piece *piece;
    M640Team *team;
    int side, i;
    HSF_DATA *hsf;
    float time, maxTime;
    float heights[] = { 40.0f, 100.0f, 160.0f, 220.0f };
    char *hooks[] = { "itemhook1", "itemhook2", "itemhook3", "itemhook4" };
    float soundTimes[4][4] = {
        { 0.0f, 40.0f, 47.0f, 40.0f },
        { 35.0f, 0.0f, 44.0f, 36.0f },
        { 35.0f, 33.0f, 0.0f, 33.0f },
        { 32.0f, 30.0f, 33.0f, 0.0f }
    };
    for (side = 0; side < 2; side++) {
        team = &lbl_1_bss_4FC[side];
        for (i = 0; i < 12; i++) {
            piece = &lbl_1_bss_C[i][side];
            Hu3DModelPosGet(piece->model, &pos);
            if (pos.y <= 300.0f) {
                Hu3DModelShadowSet(piece->model);
            } else {
                Hu3DModelShadowReset(piece->model);
            }
            switch (piece->state) {
            case 1:
                if (piece->frame++ > 10) {
                    pos.y -= piece->speed;
                    piece->speed += 1.0f;
                }
                if (pos.y <= heights[team->targetBucket]) {
                    pos.y = heights[team->targetBucket];
                    piece->speed *= -0.18f;
                    if (piece->part == 0) {
                        HuAudFXPlay(1906);
                    } else {
                        HuAudFXPlay(1907);
                    }
                    piece->state = 2;
                    Hu3DMotionSpeedSet(team->auxModels[team->targetBucket], 1.0f);
                    Hu3DModelAttrReset(team->auxModels[team->targetBucket], 1);
                    Hu3DMotionTimeSet(team->auxModels[team->targetBucket], 0.0f);
                }
                break;
            case 2:
                pos.y -= piece->speed;
                piece->speed += 1.0f;
                if (pos.y <= heights[team->targetBucket]) {
                    pos.y = heights[team->targetBucket];
                    piece->speed = 0.0f;
                    piece->state = 3;
                }
                break;
            case 4:
                maxTime = Hu3DMotionMaxTimeGet(piece->model);
                time = Hu3DMotionTimeGet(piece->model);
                if (time == soundTimes[team->targetBucket][piece->part]) {
                    HuAudFXPlay(1918);
                }
                if (time >= maxTime) {
                    if (piece->part == 1) {
                        Hu3DMotionSet(piece->model, -1);
                    } else {
                        Hu3DMotionTimeSet(piece->model, 0.0f);
                        Hu3DMotionSet(piece->model, piece->motionsA[0]);
                        Hu3DMotionSpeedSet(piece->model, 0.0f);
                    }
                    Hu3DModelPosSet(piece->model, side * 1300, 400.0f, 200.0f);
                    pos.x = side * 1300;
                    pos.y = 400.0f;
                    pos.z = 200.0f;
                    Hu3DModelAttrSet(piece->model, 1);
                    piece->state = 0;
                    Hu3DModelTPLvlSet(piece->model, 1.0f);
                    if (i == 11) {
                        hsf = Hu3DData[piece->model].hsf;
                        memcpy(hsf->material, lbl_1_bss_8, hsf->materialNum * sizeof(HSF_MATERIAL));
                    }
                    return;
                }
                if (20.0f + time > maxTime) {
                    Hu3DModelTPLvlSet(piece->model, (maxTime - time) / 20.0f);
                }
                break;
            case 5:
                pos.y -= piece->speed;
                piece->speed += 1.0f;
                if (pos.y <= heights[piece->part]) {
                    if (piece->part == 0) {
                        HuAudFXPlay(1906);
                    } else {
                        HuAudFXPlay(1907);
                    }
                    pos.y = heights[piece->part];
                    piece->speed *= -0.18f;
                    piece->state = 6;
                    Hu3DModelAttrReset(team->hookModels[piece->part], 1);
                    Hu3DMotionSpeedSet(team->hookModels[piece->part], 1.0f);
                }
                break;
            case 6:
                pos.y -= piece->speed;
                piece->speed += 1.0f;
                if (pos.y <= heights[piece->part]) {
                    pos.y = heights[piece->part];
                    piece->speed *= -0.18f;
                    if (fabsf(piece->speed) < 1.0f) {
                        piece->speed = 0.0f;
                        piece->state = 7;
                        Hu3DModelHookSet(team->model4, hooks[piece->part], piece->model);
                        pos.x = pos.y = pos.z = 0.0f;
                    }
                }
                break;
            }
            Hu3DModelPosSet(piece->model, pos.x, pos.y, pos.z);
        }
    }
}

void fn_1_5A28(void)
{
    int i, side, j;
    HSF_DATA *hsf;
    u16 cameras[] = { 1, 2 };
    memset(lbl_1_bss_C, 0, sizeof(lbl_1_bss_C));
    for (side = 0; side < 2; side++) {
        for (i = 0; i < 12; i++) {
            lbl_1_bss_C[i][side].model = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_1A4[i], 268435456, HEAP_MODEL));
            lbl_1_bss_C[i][side].part = i % 4;
            Hu3DModelPosSet(lbl_1_bss_C[i][side].model, side * 1300, 400.0f, 200.0f);
            Hu3DModelCameraSet(lbl_1_bss_C[i][side].model, cameras[side]);
            Hu3DModelAttrSet(lbl_1_bss_C[i][side].model, 1);
            for (j = 0; j < 2; j++) {
                if (lbl_1_data_84[i][j] != 0) {
                    lbl_1_bss_C[i][side].motionsA[j] = Hu3DJointMotion(lbl_1_bss_C[i][side].model, HuDataSelHeapReadNum(lbl_1_data_84[i][j], 268435456, HEAP_MODEL));
                }
            }
            for (j = 0; j < 4; j++) {
                if (lbl_1_data_E4[i][j] != 0) {
                    lbl_1_bss_C[i][side].motionsB[j] = Hu3DJointMotion(lbl_1_bss_C[i][side].model, HuDataSelHeapReadNum(lbl_1_data_E4[i][j], 268435456, HEAP_MODEL));
                }
            }
            if (side == 0 && i == 11) {
                hsf = Hu3DData[lbl_1_bss_C[i][side].model].hsf;
                OSReport("testtesttest");
                lbl_1_bss_8 = HuMemDirectMallocNum(HEAP_HEAP, hsf->materialNum * sizeof(HSF_MATERIAL), 268435456);
                memcpy(lbl_1_bss_8, hsf->material, hsf->materialNum * sizeof(HSF_MATERIAL));
            }
        }
    }
    lbl_1_bss_3F0 = omAddObjEx(lbl_1_bss_4, 50, 0, 0, -1, fn_1_52D0);
}

void fn_1_5DF0(void)
{
    float heights[] = { 0.0f, 60.0f, 120.0f, 180.0f };
    int i, j;
    s16 variant, selected;
    for (i = 0; i < 5; i++) {
        variant = frandmod(12);
        lbl_1_bss_3E4[i] = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_1D4[variant], 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_3E4[i], 3);
        Hu3DModelPosSet(lbl_1_bss_3E4[i], -750.0f + i * 480, 75.0f, -300.0f);
    }
    for (i = 0; i < 3; i++) {
        variant = frandmod(3);
        for (j = 0; j < 4; j++) {
            selected = (variant + j) % 3;
            lbl_1_bss_3CC[i][j] = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_1D4[j + selected * 4], 268435456, HEAP_MODEL));
            Hu3DModelCameraSet(lbl_1_bss_3CC[i][j], 3);
            Hu3DModelPosSet(lbl_1_bss_3CC[i][j], -750.0f + i * 800, 75.0f + heights[j], -900.0f);
        }
    }
    lbl_1_bss_3F4 = omAddObjEx(lbl_1_bss_4, 80, 0, 0, -1, fn_1_60AC);
}

void fn_1_60AC(OMOBJ *obj)
{
    HuVecF pos;
    int i, j;
    for (i = 0; i < 5; i++) {
        Hu3DModelPosGet(lbl_1_bss_3E4[i], &pos);
        pos.x += 2.0f;
        if (pos.x >= 1950.0f) {
            pos.x -= 2700.0f;
        }
        Hu3DModelPosSet(lbl_1_bss_3E4[i], pos.x, pos.y, pos.z);
    }
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) {
            Hu3DModelPosGet(lbl_1_bss_3CC[i][j], &pos);
            pos.x -= 2.0f;
            if (pos.x <= -750.0f) {
                pos.x += 2700.0f;
            }
            Hu3DModelPosSet(lbl_1_bss_3CC[i][j], pos.x, pos.y, pos.z);
        }
    }
}

void fn_1_623C(void)
{
}
