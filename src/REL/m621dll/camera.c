#include "REL/m621dll.h"
#include "game/data.h"
#include "game/memory.h"
#include "datadir_enum.h"

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
