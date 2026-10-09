/* Background, finish marker, and course-surface animation for Asteroad Rage. */
#define M659_DATA_BACKGROUND_MODEL DATANUM(DATA_m659, 0)
#define M659_DATA_BACKGROUND_MOTION DATANUM(DATA_m659, 1)
#define M659_DATA_FINISH_MODEL DATANUM(DATA_m659, 2)
#define M659_DATA_FINISH_MOTION DATANUM(DATA_m659, 3)
#define M659_DATA_COURSE_SURFACE_MODEL DATANUM(DATA_m659, 4)
#define M659_DATA_COURSE_SURFACE_MOTION DATANUM(DATA_m659, 5)
#include "game/object.h"
#include "game/memory.h"
#include "game/data.h"
#include "game/mg/seqman.h"
#include "string.h"

/* Course model labels and zeroed cells retained with the scene data. */
char lbl_1_data_198[62] =
    "\0\0\0\0\0\0\0\0\0\0\0\0"
    "col\0\0\0\0\0\0\0"
    "post_R\0\0\0\0"
    "post_L\0\0\0\0"
    "post_R_C\0\0"
    "post_L_C";

/* Scene state begins after a 40-byte prefix not read by these routines. */
typedef struct M659Scene {
    unsigned char unreadSceneBytes[40]; /* Prefix bytes with no established scene-field behavior in
                                         * this module. */
    HuVecF position; /* Zero-initialized position returned by fn_1_4F4C; not updated in this
                      * module. */
    float backgroundScale; /* Background scale, from 0.6 to 1.0. */
    float surfaceDepth; /* Course surface depth in world units. */
    float surfaceSpeed; /* Depth removed from the course surface per frame. */
} M659Scene;

OMOBJ *lbl_1_bss_58;
void fn_1_329C(OMOBJMAN *manager);
void fn_1_530C(OMOBJ *obj);
void fn_1_564C(OMOBJ *obj);
int fn_1_1840(s16 player);

/* Creates the scenery object, initializes its scale and depth, and creates obstacle management. */
void fn_1_4E68(OMOBJMAN *manager)
{
    M659Scene *sceneData;
    OMOBJ *obj;
    obj = omAddObjEx(manager, 30, 5, 3, -1, fn_1_530C);
    lbl_1_bss_58 = obj;
    obj->stat |= 0x100;
    sceneData = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M659Scene), HU_MEMNUM_OVL);
    memset(sceneData, 0, sizeof(M659Scene));
    sceneData->backgroundScale = 0.6f;
    sceneData->surfaceDepth = 7500;
    sceneData->surfaceSpeed = sceneData->surfaceDepth / 2040;
    fn_1_329C(manager);
}

void fn_1_4F48(void)
{
}

/* Copies the current scenery position to the caller. */
void fn_1_4F4C(HuVecF *position)
{
    M659Scene *sceneData = lbl_1_bss_58->data;
    position->x = sceneData->position.x;
    position->y = sceneData->position.y;
    position->z = sceneData->position.z;
}

/* Creates the background and finish marker models during scenery startup. */
void fn_1_4F88(OMOBJ *obj)
{
    M659Scene *sceneData = obj->data;
    HU3D_MODELID model = 0;
    HU3D_MOTIONID motion = 0;
    model = obj->mdlId[0] = Hu3DModelCreate(
        HuDataSelHeapReadNum(M659_DATA_BACKGROUND_MODEL, HU_MEMNUM_OVL, HEAP_MODEL));
    motion = obj->mtnId[0] = Hu3DJointMotion(
        model, HuDataSelHeapReadNum(M659_DATA_BACKGROUND_MOTION, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(model, motion);
    Hu3DMotionSpeedSet(model, 1);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0, 0, 17000);
    Hu3DModelRotSet(model, 0, 0, 5);
    Hu3DModelScaleSet(model, sceneData->backgroundScale, sceneData->backgroundScale,
                      sceneData->backgroundScale);
    Hu3DModelCameraSet(model, 3);
    model = obj->mdlId[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(M659_DATA_FINISH_MODEL, HU_MEMNUM_OVL, HEAP_MODEL));
    motion = obj->mtnId[1] = Hu3DJointMotion(
        model, HuDataSelHeapReadNum(M659_DATA_FINISH_MOTION, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(model, motion);
    Hu3DMotionSpeedSet(model, 1);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 5);
    Hu3DModelPosSet(model, 0, 0, 17000);
    Hu3DModelRotSet(model, 0, 0, 5);
    Hu3DModelScaleSet(model, sceneData->backgroundScale, sceneData->backgroundScale,
                      sceneData->backgroundScale);
    Hu3DModelCameraSet(model, 3);
    Hu3DZClearLayerSet(1);
    Hu3DBGColorSet(0, 0, 50);
}

/* Creates the moving course surface model during scenery startup. */
void fn_1_51F0(OMOBJ *obj)
{
    M659Scene *sceneData = obj->data;
    HU3D_MODELID model;
    HU3D_MOTIONID motion;
    model = obj->mdlId[3] = Hu3DModelCreate(
        HuDataSelHeapReadNum(M659_DATA_COURSE_SURFACE_MODEL, HU_MEMNUM_OVL, HEAP_MODEL));
    motion = obj->mtnId[2] = Hu3DJointMotion(
        model, HuDataSelHeapReadNum(M659_DATA_COURSE_SURFACE_MOTION, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(model, motion);
    Hu3DMotionSpeedSet(model, 1);
    Hu3DModelLayerSet(model, 1);
    Hu3DModelPosSet(model, 0, 0, sceneData->surfaceDepth);
    Hu3DModelRotSet(model, 0, 180, 0);
    Hu3DModelCameraSet(model, 3);
}

/* Scenery object startup callback that creates its models and installs its frame callback. */
void fn_1_530C(OMOBJ *obj)
{
    fn_1_4F88(obj);
    fn_1_51F0(obj);
    obj->objFunc = fn_1_564C;
}

/* Scenery callback that grows the background and moves the course surface during active modes. */
void fn_1_564C(OMOBJ *obj)
{
    M659Scene *sceneData = obj->data;
    float backgroundScale = sceneData->backgroundScale;
    HU3D_MODELID model;
    if (MgSeqModeGet() == 5 || MgSeqModeGet() == 3 ||
        (MgSeqModeGet() == 6 && fn_1_1840(0) == 0 && fn_1_1840(1) == 0)) {
        model = obj->mdlId[0];
        Hu3DModelScaleSet(model, backgroundScale, backgroundScale, backgroundScale);
        model = obj->mdlId[1];
        Hu3DModelScaleSet(model, backgroundScale, backgroundScale, backgroundScale);
        /* Both background models use this frame's scale; the growth applies on the next update. */
        sceneData->backgroundScale += 0.0002f;
        if (sceneData->backgroundScale > 1.0f) {
            sceneData->backgroundScale = 1.0f;
        }
        model = obj->mdlId[3];
        Hu3DModelPosSet(model, 0, 0, sceneData->surfaceDepth);
        /* The surface uses this frame's depth; the stored depth decreases for the next update. */
        sceneData->surfaceDepth -= sceneData->surfaceSpeed;
        if (sceneData->surfaceDepth < 0.0f) {
            sceneData->surfaceDepth = 0.0f;
        }
    }
}

/* Resets background and course surface transforms for the results presentation. */
void fn_1_57BC(void)
{
    M659Scene *sceneData = lbl_1_bss_58->data;
    float ignoredScale = sceneData->backgroundScale; /* Read here but not used by the fixed result
                                                      * scale below. */
    HU3D_MODELID model;
    model = lbl_1_bss_58->mdlId[0];
    Hu3DModelScaleSet(model, 1, 1, 1);
    Hu3DModelPosSet(model, 0, 0, 12000);
    model = lbl_1_bss_58->mdlId[1];
    Hu3DModelScaleSet(model, 1, 1, 1);
    Hu3DModelPosSet(model, 0, 0, 12000);
    model = lbl_1_bss_58->mdlId[3];
    Hu3DModelPosSet(model, 0, 0, 0);
    Hu3DModelScaleSet(model, 0.7f, 0.7f, 1);
}
