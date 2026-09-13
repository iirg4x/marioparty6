#include "REL/m651dll.h"
#include "game/main.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/hsfex.h"
#include "game/memory.h"
#include "game/pad.h"
#include "datadir_enum.h"
#include "math.h"

M651Player lbl_1_bss_A8[2];
M651Work64 lbl_1_bss_64;
M651Work14 lbl_1_bss_14;
float lbl_1_bss_10;
unsigned int lbl_1_data_70[6] = {
    DATANUM(DATA_mario, 33), DATANUM(DATA_mariomot, 26), DATANUM(DATA_mario, 118),
    DATANUM(DATA_mariomot, 27), DATANUM(DATA_mariomot, 34), 0
};
int lbl_1_data_88[4] = { 15, 10, 8, 6 };
float lbl_1_data_98 = 0.020000001f;
HuVecF lbl_1_data_9C = { 0.0f, 40.0f, 2300.0f };
float lbl_1_data_A8 = 1.0f;
s32 lbl_1_data_AC = -1;
s32 lbl_1_data_B0 = -8192;

void fn_1_4A0(void)
{
    lbl_1_data_98 += 0.016666668f;
    lbl_1_bss_14.unk_04 += 0.02f;
    if (lbl_1_bss_14.unk_04 > 2.6f) {
        lbl_1_bss_14.unk_04 = 2.6f;
    }
    lbl_1_bss_64.unk_38 += 0.002;
    if (lbl_1_bss_A8[0].unk_24 == 1 || lbl_1_bss_A8[1].unk_24 == 1) {
        lbl_1_bss_64.unk_30 -= 0.01;
        if (lbl_1_bss_64.unk_30 > 0.0f) {
            lbl_1_bss_64.unk_30 = 0.0f;
        }
    } else {
        lbl_1_bss_64.unk_30 += 0.0001;
        if (lbl_1_bss_64.unk_30 > 1.2f) {
            lbl_1_bss_64.unk_30 = 1.2f;
        }
    }
}

void fn_1_62C(void)
{
    int i;
    for (i = 0; i < 5; i++) {
        Hu3DModelAttrSet(lbl_1_bss_14.modelIds32[i], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_14.modelIds1E[i], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_69C(OMOBJ *obj)
{
    int i;
    for (i = 0; i < 5; i++) {
        if (Hu3DMotionEndCheck(lbl_1_bss_14.modelIds32[i]) == 1) {
            Hu3DModelAttrSet(lbl_1_bss_14.modelIds32[i], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_14.modelIds1E[i], HU3D_ATTR_DISPOFF);
        }
    }
}

void fn_1_72C(OMOBJ *obj)
{
    int i;
    int count = 0;
    int time;

    for (i = 0; i < 5; i++) {
        if (Hu3DMotionEndCheck(lbl_1_bss_14.modelIds32[i]) == 1) {
            if (lbl_1_bss_14.unk_00 == 1) {
                count++;
                if (count == 5) {
                    obj->objFunc = fn_1_69C;
                    return;
                }
                continue;
            }
            Hu3DModelAttrSet(lbl_1_bss_14.modelIds32[i], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_14.modelIds1E[i], HU3D_ATTR_DISPOFF);
            lbl_1_bss_14.delayFrames[i]--;
            if (lbl_1_bss_14.delayFrames[i] == 0) {
                time = rand8() % 64;
                lbl_1_bss_14.triggeredF[i] = 0;
                if (i != 4) {
                    lbl_1_bss_14.delayFrames[i] = rand8() % 256 + 1;
                    Hu3DModelAttrReset(lbl_1_bss_14.modelIds32[i], HU3D_ATTR_DISPOFF);
                    Hu3DModelAttrReset(lbl_1_bss_14.modelIds1E[i], HU3D_ATTR_DISPOFF);
                    Hu3DMotionSpeedSet(lbl_1_bss_14.modelIds32[i], lbl_1_bss_14.unk_04);
                    Hu3DMotionTimeSet(lbl_1_bss_14.modelIds32[i], time);
                    Hu3DMotionSpeedSet(lbl_1_bss_14.modelIds1E[i], lbl_1_bss_14.unk_04);
                    Hu3DMotionTimeSet(lbl_1_bss_14.modelIds1E[i], time);
                }
            }
        } else {
            Mtx mtx;
            char *names[5] = { "meteo-astreS", "meteo-astreM", "meteo-astreLL", "meteo-astreL", "kuriboo_null" };
            HuVecF pos;
            HuVecF scale;

            Hu3DModelObjMtxGet(lbl_1_bss_14.modelIds32[i], names[i], mtx);
            Hu3DMtxTransGet(mtx, &pos);
            Hu3DMtxScaleGet(mtx, &scale);
            if (80.0f * lbl_1_bss_64.unk_24 > pos.z && lbl_1_bss_14.triggeredF[i] == 0 && i != 4) {
                Hu3DModelAttrReset(lbl_1_bss_14.modelIds28[i], HU3D_MOTATTR_PAUSE);
                Hu3DModelPosSetV(lbl_1_bss_14.modelIds28[i], &pos);
                Hu3DModelScaleSet(lbl_1_bss_14.modelIds28[i], 1.0f, 1.0f, 1.0f);
                Hu3DModelAttrSet(lbl_1_bss_14.modelIds32[i], HU3D_ATTR_DISPOFF);
                Hu3DModelAttrSet(lbl_1_bss_14.modelIds1E[i], HU3D_ATTR_DISPOFF);
                Hu3DMotionTimeSet(lbl_1_bss_14.modelIds28[i], 0.0f);
                lbl_1_bss_14.triggeredF[i] = 1;
            }
        }
        if (i == 4 && (lbl_1_bss_A8[0].pos.z < 1700.0f || lbl_1_bss_A8[1].pos.z < 1700.0f) && lbl_1_bss_64.unk_40 == 0) {
            lbl_1_bss_64.unk_40 = 1;
            if (rand8() % 100 < 40) {
                Hu3DModelAttrReset(lbl_1_bss_14.modelIds32[4], HU3D_MOTATTR_PAUSE);
                Hu3DMotionSpeedSet(lbl_1_bss_14.modelIds32[4], lbl_1_bss_14.unk_04);
            }
        }
    }
}

void fn_1_C20(s16 layerNo)
{
    int i;
    for (i = 0; i < 4; i++) {
        Mtx mtx;
        char *names[5] = { "meteo-astreS", "meteo-astreM", "meteo-astreLL", "meteo-astreL", "kuriboo_null" };
        HuVecF pos;
        HuVecF rot;
        HuVecF scale;

        Hu3DModelObjMtxGet(lbl_1_bss_14.modelIds32[i], names[i], mtx);
        Hu3DMtxTransGet(mtx, &pos);
        Hu3DMtxRotGet(mtx, &rot);
        Hu3DMtxScaleGet(mtx, &scale);
        Hu3DModelPosSetV(lbl_1_bss_14.modelIds1E[i], &pos);
        Hu3DModelRotSetV(lbl_1_bss_14.modelIds1E[i], &rot);
        Hu3DModelScaleSetV(lbl_1_bss_14.modelIds1E[i], &scale);
    }
}

void fn_1_D28(void)
{
    int i;
    OMOBJ *obj;
    HU3D_MOTIONID motionId;
    u32 modelData[5] = { DATANUM(DATA_m651, 8), DATANUM(DATA_m651, 9), DATANUM(DATA_m651, 10), DATANUM(DATA_m651, 11), DATANUM(DATA_m651, 16) };
    u32 motionData[5] = { DATANUM(DATA_m651, 18), DATANUM(DATA_m651, 19), DATANUM(DATA_m651, 20), DATANUM(DATA_m651, 21), DATANUM(DATA_m651, 17) };

    for (i = 0; i < 5; i++) {
        if (i == 0) {
            lbl_1_bss_14.modelIds1E[0] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m651, 14), HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            lbl_1_bss_14.modelIds1E[i] = Hu3DModelLink(lbl_1_bss_14.modelIds1E[0]);
        }
        Hu3DModelLayerSet(lbl_1_bss_14.modelIds1E[i], 6);
        Hu3DModelAttrSet(lbl_1_bss_14.modelIds1E[i], HU3D_MOTATTR_LOOP);
        Hu3DLayerHookSet(6, fn_1_C20);
    }
    for (i = 0; i < 5; i++) {
        if (i == 0) {
            lbl_1_bss_14.modelIds28[0] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m651, 15), HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            lbl_1_bss_14.modelIds28[i] = Hu3DModelLink(lbl_1_bss_14.modelIds28[0]);
        }
        Hu3DModelLayerSet(lbl_1_bss_14.modelIds28[i], 5);
        Hu3DModelAttrReset(lbl_1_bss_14.modelIds28[i], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(lbl_1_bss_14.modelIds28[i], HU3D_MOTATTR_PAUSE);
        lbl_1_bss_14.triggeredF[i] = 0;
    }
    lbl_1_bss_14.unk_1C = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m651, 12), HU_MEMNUM_OVL, HEAP_MODEL));
    motionId = Hu3DJointMotion(lbl_1_bss_14.unk_1C, HuDataSelHeapReadNum(DATANUM(DATA_m651, 13), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(lbl_1_bss_14.unk_1C, motionId);
    Hu3DModelAttrSet(lbl_1_bss_14.unk_1C, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(lbl_1_bss_14.modelIds1E[4], HU3D_ATTR_DISPOFF);
    for (i = 0; i < 5; i++) {
        lbl_1_bss_14.modelIds32[i] = Hu3DModelCreate(HuDataSelHeapReadNum(modelData[i], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelLayerSet(lbl_1_bss_14.modelIds32[i], 5);
        lbl_1_bss_14.motionIds[i] = Hu3DJointMotion(lbl_1_bss_14.modelIds32[i], HuDataSelHeapReadNum(motionData[i], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_14.delayFrames[i] = rand8() % 4 + 1;
        Hu3DModelAttrReset(lbl_1_bss_14.modelIds32[i], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrReset(lbl_1_bss_14.modelIds32[i], HU3D_ATTR_DISPOFF);
        Hu3DMotionSet(lbl_1_bss_14.modelIds32[i], lbl_1_bss_14.motionIds[i]);
        Hu3DMotionTimeSet(lbl_1_bss_14.modelIds32[i], rand8() % 64);
    }
    Hu3DModelAttrSet(lbl_1_bss_14.modelIds32[4], HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrReset(lbl_1_bss_14.modelIds32[4], HU3D_MOTATTR_LOOP);
    Hu3DModelHookSet(lbl_1_bss_14.modelIds32[4], "kuriboo_null", lbl_1_bss_14.unk_1C);
    obj = omAddObjEx(fn_1_A0(), 336, 0, 0, 0, fn_1_72C);
    obj->data = &lbl_1_bss_14;
    lbl_1_bss_14.unk_00 = 0;
    lbl_1_bss_14.unk_04 = 1.0f;
}

void fn_1_1220(void)
{
    Hu3DCameraCreate(HU3D_CAM0);
    Hu3DCameraPerspectiveSet(HU3D_CAM0, 45.0f, 20.0f, 15000.0f, 1.2f);
    Hu3DCameraPosSet(HU3D_CAM0, 0.0f, 300.0f, 3100.0f, 0.0f, 1.0f, 0.0f, 0.0f, -100.0f, 0.0f);
    Hu3DCameraViewportSet(HU3D_CAM0, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
}

void fn_1_1344(void)
{
    HU3D_LIGHTID lightId;

    lightId = Hu3DGLightCreate(0.0f, 1000.0f, 1000.0f, 0.0f, -1.0f, -1.0f, 255, 255, 255);
    Hu3DGLightStaticSet(lightId, TRUE);
    Hu3DGLightInfinitytSet(lightId);
}

void fn_1_13D8(void)
{
    int groupNo;
    int playerNo;
    OMOBJ *obj;
    int unused = 0;
    u32 modelData[2] = { DATANUM(DATA_m651, 22), DATANUM(DATA_m651, 24) };
    u32 motionData[2] = { DATANUM(DATA_m651, 23), DATANUM(DATA_m651, 25) };

    for (playerNo = 0; playerNo < 4; playerNo++) {
        groupNo = GwPlayerConf[playerNo].grpNo;
        if (groupNo == 0 || groupNo == 1) {
            fn_1_10C(playerNo);
            lbl_1_bss_A8[groupNo].groupNo = groupNo;
            lbl_1_bss_A8[groupNo].playerNo = playerNo;
            lbl_1_bss_A8[groupNo].characterNo = GwPlayerConf[playerNo].charNo;
            lbl_1_bss_A8[groupNo].padNo = GwPlayerConf[playerNo].padNo;
            lbl_1_bss_A8[groupNo].modelId = CharModelMotListCreate(lbl_1_bss_A8[groupNo].characterNo, 1, lbl_1_data_70, lbl_1_bss_A8[groupNo].motionIds);
            CharMotionSet(lbl_1_bss_A8[groupNo].characterNo, lbl_1_bss_A8[groupNo].motionIds[1]);
            Hu3DModelCameraSet(lbl_1_bss_A8[groupNo].modelId, HU3D_CAM0);
            Hu3DModelAttrSet(lbl_1_bss_A8[groupNo].modelId, HU3D_MOTATTR_LOOP);
            lbl_1_bss_A8[groupNo].unk_30 = Hu3DModelCreate(HuDataSelHeapReadNum(modelData[groupNo], HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_A8[groupNo].unk_32 = Hu3DJointMotion(lbl_1_bss_A8[groupNo].unk_30, HuDataSelHeapReadNum(motionData[groupNo], HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DMotionSet(lbl_1_bss_A8[groupNo].unk_30, lbl_1_bss_A8[groupNo].unk_32);
            Hu3DModelAttrReset(lbl_1_bss_A8[groupNo].unk_30, HU3D_MOTATTR_LOOP);
            Hu3DModelAttrSet(lbl_1_bss_A8[groupNo].unk_30, HU3D_MOTATTR_PAUSE);
            obj = omAddObjEx(fn_1_A0(), 400, 0, 0, 0, fn_1_18DC);
            obj->data = &lbl_1_bss_A8[groupNo];
            lbl_1_bss_A8[groupNo].pos.x = -100.0f + 200.0f * groupNo;
            lbl_1_bss_A8[groupNo].pos.y = 0.0f;
            lbl_1_bss_A8[groupNo].pos.z = 2000.0f;
            Hu3DModelPosSetV(lbl_1_bss_A8[groupNo].modelId, &lbl_1_bss_A8[groupNo].pos);
            lbl_1_bss_A8[groupNo].unk_20 = 0;
            lbl_1_bss_A8[groupNo].unk_24 = 0;
            lbl_1_bss_A8[groupNo].unk_28 = 0;
            lbl_1_bss_A8[groupNo].unk_2C = 0;
            lbl_1_bss_A8[groupNo].unk_64 = 0.0f;
            lbl_1_bss_A8[groupNo].cpuF = GwPlayerConf[playerNo].type;
            lbl_1_bss_A8[groupNo].difficulty = GwPlayerConf[playerNo].comDif;
            lbl_1_bss_A8[groupNo].unk_6E = (rand8() % 2 - 1) + lbl_1_data_88[lbl_1_bss_A8[groupNo].difficulty];
            lbl_1_bss_A8[groupNo].fxNo = -1;
        }
    }
}

void fn_1_18DC(OMOBJ *obj)
{
    obj->objFunc = fn_1_18EC;
}

void fn_1_18EC(OMOBJ *obj)
{
    if (MgSeqModeGet() == 5) {
        obj->objFunc = fn_1_192C;
    }
}

void fn_1_192C(OMOBJ *obj)
{
    M651Player *work = obj->data;
    int input = 0;

    if (work->cpuF) {
        work->unk_6E--;
        if (work->unk_6E == 0) {
            work->unk_6E = (rand8() % 2 - 1) + lbl_1_data_88[work->difficulty];
            input = 1;
        }
    } else if (HuPadBtnDown[work->padNo] & PAD_BUTTON_A) {
        input = 1;
    }
    if (input == 1) {
        work->unk_2C = 1;
        work->pos.z += 20.0f;
        if (work->pos.z < 1700.0f) {
            work->pos.z += 20.0f;
        }
        work->unk_20 += 6;
        if (work->unk_20 > 30) {
            work->unk_20 = 30;
        }
    }
    if (work->unk_20 > 0) {
        work->unk_20--;
        Hu3DMotionSpeedSet(work->modelId, 1.8f);
    } else {
        Hu3DMotionSpeedSet(work->modelId, 1.0f);
    }
    work->pos.z -= lbl_1_data_98;
    if (work->pos.z > 2300.0f) {
        work->pos.z = 2300.0f;
    }
    if (work->pos.z < 1700.0f) {
        if (work->fxNo == -1) {
            work->fxNo = CharFXPlay(work->characterNo, 581);
        }
        if (work->motionIds[2] != Hu3DMotionIDGet(work->modelId) && Hu3DMotionShiftIDGet(work->modelId) == HU3D_MOTIONID_NONE) {
            Hu3DMotionShiftSet(work->modelId, work->motionIds[2], 0.0f, 30.0f, HU3D_MOTATTR_LOOP);
        }
        if (Hu3DMotionShiftIDGet(work->modelId) != HU3D_MOTIONID_NONE) {
            work->pos.y -= 2.7f;
            if (work->pos.y < -81.0f) {
                work->pos.y = -81.0f;
            }
        }
    }
    if (work->pos.z > 1700.0f) {
        if (work->motionIds[1] != Hu3DMotionIDGet(work->modelId) && Hu3DMotionShiftIDGet(work->modelId) == HU3D_MOTIONID_NONE) {
            Hu3DMotionShiftSet(work->modelId, work->motionIds[1], 0.0f, 30.0f, HU3D_MOTATTR_LOOP);
        }
        if (Hu3DMotionShiftIDGet(work->modelId) != HU3D_MOTIONID_NONE) {
            work->pos.y += 2.7f;
            if (work->pos.y > 0.0f) {
                work->pos.y = 0.0f;
            }
        }
    }
    Hu3DModelPosSetV(work->modelId, &work->pos);
    if (work->pos.z < 1000.0f && work->unk_28 != 1) {
        work->unk_24 = 1;
    }
    if (work->unk_28 == 1) {
        if (work->unk_24 == 1) {
            char *hooks[2] = { "P1", "P2" };
            Hu3DModelPosSet(work->modelId, 0.0f, work->pos.y, 0.0f);
            Hu3DModelHookSet(work->unk_30, hooks[work->groupNo], work->modelId);
            Hu3DModelAttrReset(work->unk_30, HU3D_MOTATTR_PAUSE);
            fn_1_B0(work->playerNo);
            CharFXPlay(work->characterNo, 576);
            omVibrate(work->playerNo, 20, 20, 0);
        } else {
            HuVecF target = lbl_1_data_9C;
            HuVecF pos = work->pos;
            PSVECSubtract(&target, &pos, &work->unk_58);
            lbl_1_bss_10 = PSVECMag(&work->unk_58) / 120.0f;
            OSReport("winner speed %f\n", lbl_1_bss_10);
            PSVECNormalize(&work->unk_58, &work->unk_58);
            Hu3DMotionSpeedSet(work->modelId, 0.8f);
            Hu3DMotionShiftSet(work->modelId, work->motionIds[1], 0.0f, 60.0f, HU3D_MOTATTR_LOOP);
        }
        obj->objFunc = fn_1_2034;
    }
}

void fn_1_1E60(void)
{
    int player;

    if (lbl_1_bss_A8[0].unk_28 == 1 || lbl_1_bss_A8[1].unk_28 == 1) {
        return;
    }
    if (lbl_1_bss_A8[0].unk_24 != 0 || lbl_1_bss_A8[1].unk_24 != 0) {
        OSReport("finish.\n");
        lbl_1_bss_14.unk_00 = 1;
        if (lbl_1_bss_A8[0].unk_24 == 1 && lbl_1_bss_A8[1].unk_24 == 1) {
            if (lbl_1_bss_A8[0].unk_2C == 0 && lbl_1_bss_A8[1].unk_2C == 0) {
                lbl_1_bss_A8[0].unk_24 = 1;
                lbl_1_bss_A8[1].unk_24 = 1;
                OSReport("draw.\n");
            } else {
                player = rand8() % 2;
                lbl_1_bss_A8[player].unk_24 = 1;
                lbl_1_bss_A8[1 - player].unk_24 = 0;
                OSReport("random.\n");
            }
        }
        OSReport("%d ( %d ) : %d ( %d )\n", lbl_1_bss_A8[0].groupNo, lbl_1_bss_A8[0].unk_2C, lbl_1_bss_A8[1].groupNo, lbl_1_bss_A8[1].unk_2C);
        lbl_1_bss_A8[0].unk_28 = 1;
        lbl_1_bss_A8[1].unk_28 = 1;
    }
}

void fn_1_2034(OMOBJ *obj)
{
    M651Player *work = obj->data;
    HuVecF target;
    HuVecF pos;

    if (work->unk_24 == 1) {
        if (work->pos.z > 0.0f) {
            char *hooks[2] = { "P1", "P2" };
            Hu3DModelObjPosGet(work->unk_30, hooks[work->groupNo], &work->pos);
            return;
        }
        lbl_1_bss_64.unk_0C = 1;
        lbl_1_bss_64.unk_14 = 0;
        lbl_1_bss_64.unk_10 = lbl_1_bss_64.unk_24 / 120.0f;
        OSReport("void : %f\n", lbl_1_bss_64.unk_10);
        Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelHookReset(work->unk_30);
        obj->objFunc = fn_1_242C;
        HuAudFXFadeOut(lbl_1_data_AC, 500);
        HuAudFXPlay(2058);
        return;
    }
    target = lbl_1_data_9C;
    pos = work->pos;
    PSVECSubtract(&target, &pos, &work->unk_58);
    if (work->pos.x != lbl_1_data_9C.x || work->pos.y != lbl_1_data_9C.y || work->pos.z != lbl_1_data_9C.z) {
        PSVECNormalize(&work->unk_58, &work->unk_58);
    }
    work->pos.x += lbl_1_bss_10 * work->unk_58.x;
    work->pos.y += lbl_1_bss_10 * work->unk_58.y;
    work->pos.z += lbl_1_bss_10 * work->unk_58.z;
    if (work->pos.x < 1.0f + lbl_1_data_9C.x && work->pos.x > lbl_1_data_9C.x - 1.0f) {
        work->pos.x = lbl_1_data_9C.x;
    }
    if (work->pos.y < 1.0f + lbl_1_data_9C.y && work->pos.y > lbl_1_data_9C.y - 1.0f) {
        work->pos.y = lbl_1_data_9C.y;
    }
    if (work->pos.z < 1.0f + lbl_1_data_9C.z && work->pos.z > lbl_1_data_9C.z - 1.0f) {
        work->pos.z = lbl_1_data_9C.z;
    }
    Hu3DModelPosSetV(work->modelId, &work->pos);
    if (MgSeqModeGet() == 6) {
        Hu3DMotionShiftSet(work->modelId, work->motionIds[0], 0.0f, 40.0f, HU3D_MOTATTR_LOOP);
        obj->objFunc = fn_1_23D4;
    }
}

void fn_1_23D4(OMOBJ *obj)
{
    M651Player *work = obj->data;
    if (Hu3DMotionEndCheck(work->modelId)) {
        Hu3DMotionTimeSet(work->modelId, 35.0f);
    }
}

void fn_1_242C(OMOBJ *obj)
{
    M651Player *work = obj->data;
    if (MgSeqModeGet() == 7 && work->unk_24 == 1) {
        Hu3DMotionSet(work->modelId, work->motionIds[4]);
        Hu3DModelAttrReset(work->modelId, HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrSet(work->modelId, HU3D_MOTATTR_LOOP);
        CharModelVoiceFlagSet(work->characterNo, 0);
        obj->objFunc = fn_1_24BC;
    }
}

void fn_1_24BC(OMOBJ *obj)
{
    M651Player *work = obj->data;
    work->unk_4C.x += 12.0f * work->unk_58.x;
    work->unk_4C.y += 12.0f * work->unk_58.y;
    work->unk_4C.z += 12.0f * work->unk_58.z;
    Hu3DModelPosSet(work->modelId, work->unk_4C.x, work->unk_4C.y, work->unk_4C.z);
    Hu3DModelRotSet(work->modelId, 90.0f, 0.0f, work->unk_64);
    Hu3DModelAttrReset(work->modelId, HU3D_ATTR_DISPOFF);
    OSReport("pos : %f, %f, %f\n", work->unk_58.x, work->unk_58.y, work->unk_58.z);
}

void fn_1_25B0(void)
{
    OMOBJ *obj;

    lbl_1_bss_64.modelIds[0] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m651, 0), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_64.modelIds[0], HU3D_MOTATTR_LOOP);
    lbl_1_bss_64.modelIds[2] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m651, 2), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_64.modelIds[2], HU3D_MOTATTR_LOOP);
    lbl_1_bss_64.modelIds[1] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m651, 1), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_64.modelIds[1], HU3D_MOTATTR_SHAPE_LOOP);
    Hu3DModelAttrSet(lbl_1_bss_64.modelIds[1], HU3D_MOTATTR_LOOP);
    lbl_1_bss_64.modelIds[3] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m651, 4), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_64.modelIds[3], HU3D_MOTATTR_PAUSE);
    Hu3DMotionSpeedSet(lbl_1_bss_64.modelIds[3], 2.0f);
    lbl_1_bss_64.modelIds[4] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m651, 3), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrReset(lbl_1_bss_64.modelIds[4], HU3D_MOTATTR_LOOP);
    lbl_1_bss_64.unk_30 = 0.0f;
    lbl_1_bss_64.unk_34 = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m651, 5), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_64.unk_38 = 1.0f;
    Hu3DModelAttrSet(lbl_1_bss_64.unk_34, HU3D_MOTATTR_LOOP);
    lbl_1_bss_64.unk_3C = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m651, 6), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_64.unk_3E = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m651, 7), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_64.unk_3C, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(lbl_1_bss_64.unk_3E, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(lbl_1_bss_64.unk_3E, HU3D_ATTR_DISPOFF);
    Hu3DModelLayerSet(lbl_1_bss_64.modelIds[4], 0);
    Hu3DModelLayerSet(lbl_1_bss_64.modelIds[0], 1);
    Hu3DModelLayerSet(lbl_1_bss_64.modelIds[2], 2);
    Hu3DModelLayerSet(lbl_1_bss_64.modelIds[1], 3);
    Hu3DModelLayerSet(lbl_1_bss_64.unk_34, 4);
    Hu3DModelLayerSet(lbl_1_bss_64.unk_3C, 4);
    Hu3DModelLayerSet(lbl_1_bss_64.unk_3E, 4);
    obj = omAddObjEx(fn_1_A0(), 336, 0, 0, 0, fn_1_29A4);
    obj->data = &lbl_1_bss_64;
    lbl_1_bss_64.unk_14 = 0;
    lbl_1_bss_64.unk_18 = 0.0f;
    lbl_1_bss_64.unk_1C = 0.0f;
    lbl_1_bss_64.unk_20 = 0.0f;
    lbl_1_bss_64.unk_24 = 0.0f;
    lbl_1_bss_64.unk_28 = 0.0f;
    lbl_1_bss_64.unk_2C = 1.0f;
    lbl_1_bss_64.unk_0C = 0;
    lbl_1_bss_64.unk_40 = 0;
}

void fn_1_29A4(OMOBJ *obj)
{
    M651Work64 *work = obj->data;
    if (lbl_1_bss_64.unk_14 == 55) {
        HuAudFXPlay(2053);
        HuAudFXPlay(2054);
        lbl_1_data_AC = HuAudFXPlay(2055);
        HuAudFXPitchSet(lbl_1_data_AC, lbl_1_data_B0);
    }
    lbl_1_bss_64.unk_14++;
    if (lbl_1_bss_64.unk_14 >= 80 && lbl_1_bss_64.unk_14 < 200) {
        lbl_1_bss_64.unk_1C += 0.00833;
        if (lbl_1_bss_64.unk_1C > 1.0f) lbl_1_bss_64.unk_1C = 1.0f;
    }
    lbl_1_bss_64.unk_18 += 1.0f;
    Hu3DModelScaleSet(lbl_1_bss_64.modelIds[0], lbl_1_bss_64.unk_1C, lbl_1_bss_64.unk_1C, lbl_1_bss_64.unk_1C);
    Hu3DModelRotSet(lbl_1_bss_64.modelIds[0], 0.0f, 0.0f, lbl_1_bss_64.unk_18);
    if (lbl_1_bss_64.unk_14 < 120) {
        lbl_1_bss_64.unk_24 += 0.00833;
        lbl_1_bss_64.unk_28 += 0.00833;
        if (lbl_1_bss_64.unk_24 > 1.0f) lbl_1_bss_64.unk_24 = 1.0f;
        if (lbl_1_bss_64.unk_28 > 1.0f) lbl_1_bss_64.unk_28 = 1.0f;
    }
    lbl_1_bss_64.unk_20 += 2.0f;
    Hu3DModelScaleSet(lbl_1_bss_64.modelIds[1], lbl_1_bss_64.unk_24, lbl_1_bss_64.unk_24, lbl_1_bss_64.unk_24);
    Hu3DModelRotSet(lbl_1_bss_64.modelIds[1], 0.0f, 0.0f, lbl_1_bss_64.unk_20);
    Hu3DModelScaleSet(lbl_1_bss_64.modelIds[2], lbl_1_bss_64.unk_24, lbl_1_bss_64.unk_24, lbl_1_bss_64.unk_24);
    Hu3DModelRotSet(lbl_1_bss_64.modelIds[2], 0.0f, 0.0f, lbl_1_bss_64.unk_20);
    if (Hu3DMotionEndCheck(lbl_1_bss_64.modelIds[4]) == 1) {
        lbl_1_bss_64.unk_2C -= 2.0f;
        Hu3DModelRotSet(lbl_1_bss_64.modelIds[4], 0.0f, 0.0f, lbl_1_bss_64.unk_2C);
    }
    if (MgSeqModeGet() == 5) obj->objFunc = fn_1_2D8C;
}

void fn_1_2D8C(OMOBJ *obj)
{
    M651Work64 *work = obj->data;
    if (lbl_1_bss_64.unk_0C == 1) {
        Hu3DModelAttrSet(lbl_1_bss_64.modelIds[4], HU3D_MOTATTR_SHAPE_REV);
        lbl_1_bss_64.unk_14++;
        lbl_1_bss_64.unk_1C -= 0.00833;
        if (lbl_1_bss_64.unk_1C < 0.0f) lbl_1_bss_64.unk_1C = 0.0f;
        lbl_1_bss_64.unk_24 -= lbl_1_bss_64.unk_10;
        lbl_1_bss_64.unk_28 -= lbl_1_bss_64.unk_10;
        if (lbl_1_bss_64.unk_24 < 0.0f) lbl_1_bss_64.unk_24 = 0.0f;
        if (lbl_1_bss_64.unk_28 < 0.0f) lbl_1_bss_64.unk_28 = 0.0f;
        if (lbl_1_bss_64.unk_24 < 0.2f) Hu3DModelAttrReset(lbl_1_bss_64.modelIds[3], HU3D_MOTATTR_PAUSE);
        if ((float)lbl_1_bss_64.unk_14 == 90.0f) HuAudFXPlay(2059);
        if ((float)lbl_1_bss_64.unk_14 > 120.0f) {
            fn_1_190();
            obj->objFunc = NULL;
        }
    } else {
        lbl_1_bss_64.unk_24 += 0.004f;
        if (lbl_1_bss_64.unk_24 > 3.0f) lbl_1_bss_64.unk_24 = 3.0f;
        lbl_1_bss_64.unk_28 += 0.004f;
        if (lbl_1_bss_64.unk_28 > 1.5f) lbl_1_bss_64.unk_28 = 1.5f;
        Hu3DMotionSpeedSet(lbl_1_bss_64.modelIds[0], lbl_1_data_A8);
        lbl_1_data_A8 += 0.05f;
        if (lbl_1_data_A8 > 4.0f) lbl_1_data_A8 = 4.0f;
        {
            float times[6] = { 405.0f, 426.0f, 494.0f, 540.0f, 590.0f, 630.0f };
            s16 pan[6] = { 32, 32, 96, 32, 32, 96 };
            int i = 0;
            float time = Hu3DMotionTimeGet(lbl_1_bss_64.unk_34);
            for (i = 0; i < 6; i++) {
                if (time == times[i]) HuAudFXPlayPan(2060, pan[i]);
            }
        }
        {
            float times[19] = { 386.0f, 490.0f, 580.0f, 666.0f, 737.0f, 760.0f, 830.0f, 870.0f, 900.0f, 932.0f, 955.0f, 1000.0f, 1024.0f, 1064.0f, 1096.0f, 1130.0f, 1150.0f, 1170.0f, 1180.0f };
            int i = 0;
            float time = Hu3DMotionTimeGet(lbl_1_bss_64.modelIds[1]);
            for (i = 0; i < 19; i++) {
                if (time == times[i]) HuAudFXPlay(2056);
            }
        }
    }
    if (lbl_1_data_B0 != 0) {
        lbl_1_data_B0 += 10;
        if (lbl_1_data_B0 > 0) lbl_1_data_B0 = 0;
        HuAudFXPitchSet(lbl_1_data_AC, lbl_1_data_B0);
    }
    lbl_1_bss_64.unk_18 += 1.0f + lbl_1_bss_64.unk_30;
    Hu3DModelScaleSet(lbl_1_bss_64.modelIds[0], lbl_1_bss_64.unk_1C, lbl_1_bss_64.unk_1C, lbl_1_bss_64.unk_1C);
    Hu3DModelRotSet(lbl_1_bss_64.modelIds[0], 0.0f, 0.0f, lbl_1_bss_64.unk_18);
    lbl_1_bss_64.unk_20 += 2.0f + lbl_1_bss_64.unk_30;
    Hu3DModelScaleSet(lbl_1_bss_64.modelIds[1], lbl_1_bss_64.unk_24, lbl_1_bss_64.unk_24, lbl_1_bss_64.unk_24);
    Hu3DModelRotSet(lbl_1_bss_64.modelIds[1], 0.0f, 0.0f, lbl_1_bss_64.unk_20);
    Hu3DModelScaleSet(lbl_1_bss_64.modelIds[2], lbl_1_bss_64.unk_28, lbl_1_bss_64.unk_28, lbl_1_bss_64.unk_28);
    Hu3DModelRotSet(lbl_1_bss_64.modelIds[2], 0.0f, 0.0f, lbl_1_bss_64.unk_20);
    lbl_1_bss_64.unk_2C -= 2.0f + lbl_1_bss_64.unk_30;
    Hu3DModelRotSet(lbl_1_bss_64.modelIds[4], 0.0f, 0.0f, lbl_1_bss_64.unk_2C);
    if (lbl_1_bss_A8[0].pos.z < 1700.0f || lbl_1_bss_A8[1].pos.z < 1700.0f) {
        Hu3DModelAttrReset(lbl_1_bss_64.unk_3E, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_64.unk_3E, HU3D_MOTATTR_LOOP);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_64.unk_3E, HU3D_MOTATTR_LOOP);
    }
    if (lbl_1_bss_A8[0].unk_24 == 1 || lbl_1_bss_A8[1].unk_24 == 1) {
        Hu3DModelAttrReset(lbl_1_bss_64.unk_3E, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrReset(lbl_1_bss_64.unk_3C, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(lbl_1_bss_64.unk_34, HU3D_ATTR_DISPOFF);
    }
}

void fn_1_35D8(OMOBJ *obj)
{
    M651Player *work = obj->data;
}

void fn_1_35EC(M651Player *work, float y1, float y2, float x1, float x2)
{
    work->unk_34.x = x1;
    work->unk_34.y = y1;
    work->unk_34.z = 1000.0f;
    work->unk_40.x = x2;
    work->unk_40.y = y2;
    work->unk_40.z = 1000.0f;
    work->unk_4C = work->unk_34;
    PSVECSubtract(&work->unk_40, &work->unk_34, &work->unk_58);
    PSVECNormalize(&work->unk_58, &work->unk_58);
    Hu3DModelAttrReset(work->modelId, HU3D_MOTATTR_LOOP);
    CharMotionSet(work->characterNo, work->motionIds[0]);
    if (x1 < x2) {
        work->unk_64 = -90.0f;
    } else {
        work->unk_64 = 90.0f;
    }
}

void fn_1_36F8(void)
{
    int player;
    int direction;
    int winnerDirection;
    int height;
    int winner;

    if (lbl_1_bss_A8[0].unk_24 == 1 && lbl_1_bss_A8[1].unk_24 == 1) {
        player = rand8() % 2;
        direction = rand8() % 2;
        {
            float ys[2] = { 250.0f, -250.0f };
            fn_1_35EC(&lbl_1_bss_A8[player], ys[direction], ys[1 - direction], -1600.0f, 1600.0f);
            fn_1_35EC(&lbl_1_bss_A8[1 - player], ys[direction], ys[1 - direction], 1600.0f, -1600.0f);
        }
    } else {
        winner = lbl_1_bss_A8[0].unk_24 == 1 ? 1 : 0;
        {
            HU3D_MODELID modelId = lbl_1_bss_A8[winner].modelId;
            Hu3DModelAttrSet(modelId, HU3D_MOTATTR_PAUSE);
            CharMotionShiftSet(lbl_1_bss_A8[winner].characterNo, lbl_1_bss_A8[winner].motionIds[3], 0.0f, 15.0f, 0);
        }
        winnerDirection = rand8() % 2;
        height = rand8() % 2;
        {
            float ys[2] = { 250.0f, -250.0f };
            float xs[2] = { -1600.0f, 1600.0f };
            fn_1_35EC(&lbl_1_bss_A8[1 - winner], ys[height], ys[1 - height], xs[winnerDirection], xs[1 - winnerDirection]);
        }
    }
}

void fn_1_3F30(void)
{
    fn_1_1344();
    fn_1_1220();
    fn_1_13D8();
    fn_1_25B0();
    fn_1_D28();
}
