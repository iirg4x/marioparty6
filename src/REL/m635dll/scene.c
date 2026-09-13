#include "REL/m635dll.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "datadir_enum.h"

extern HuVecF lbl_1_data_A8[2];
extern HuVecF lbl_1_data_C0[2];
extern HuVecF lbl_1_data_D8;
extern HuVecF lbl_1_data_E4;
extern GXColor lbl_1_data_F0[2];
extern u32 lbl_1_data_3E4[2];
extern u32 lbl_1_data_3EC[2];
extern u32 lbl_1_data_3F4[2][2];
extern u32 lbl_1_data_404[2][2];
extern u32 lbl_1_data_414[2];

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
