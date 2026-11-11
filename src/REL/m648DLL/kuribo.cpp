/* The kuribo: the manager that loads their model and motions, and the walkers it places on the model's objects. */
#include <cmath>
extern "C" {
#include <math.h>
#include "dolphin/mtx.h"
#include "dolphin/gx.h"
#include "game/hu3d.h"
#include "game/data.h"
}
#include "REL/framework/model.h"
#include "REL/framework/singleton.h"
#include "REL/framework/list_object.h"
#include <vector>
#include "REL/m648DLL/light_shadow.h"
#include "REL/m648DLL/kuribo_motion_owner.h"
#include "REL/m648DLL.h"
#include "humath.h"
#include "REL/m648DLL/kuribo.h"
#include "REL/framework/frame_time_inline.h"
extern "C" {
extern void hideObject__5ModelFPc(void *motionId, char *name);
extern void *__nw__FUl(s32);
}
extern "C" {
extern void __ct__4NodeFv(void *object);
Vec lbl_1_bss_D4[0x21];
#include "game/gamework.h"
}
static const int NUMBEROF_PMOTION = 3;
/* The framework version this game was built with converts seconds to frames in line (the random
   waits below). Its three phase durations open this file's .bss (0xC8), ahead of the walkers'
   shadow circle and their list head; the head's destructor ends the file, after the static
   initialiser. */
u32 RandomUnsigned();
float RandomFloat();
/* Removes the walkers when their manager shuts down. */

extern "C" { void fn_1_653C(); }
/* Creates the kuribo manager and the kuribo on its model's sixteen objects. */
KuriboMotionOwner::KuriboMotionOwner() : model(DATANUM(0x6b0000, 9)), motion(DATANUM(0x6b0000, 13))
{
    static char *objectNames[16] = {
        "kuri01", "kuri02", "kuri03", "kuri04", "kuri05", "kuri06", "kuri07", "kuri08",
        "kuri09", "kuri10", "kuri11", "kuri12", "kuri13", "kuri14", "kuri15", "kuri16",
    };
    s16 currentModel;
    s32 index;
    currentModel = model;
    fn_800269E8(currentModel, (1 << 0));
    m_mot_id.clear();
    m_mot_id.reserve(NUMBEROF_PMOTION);
    addMotion(DATANUM(0x6b0000, 10));
    addMotion(DATANUM(0x6b0000, 11));
    addMotion(DATANUM(0x6b0000, 12));
    // The walkers need all three motions.
    assert(m_mot_id.size() == NUMBEROF_PMOTION);

    for (index = 0; index < 0x21; index++) {
        f32 angle = 360.0f * ((f32)(index & 0x1F) / 32.0f) - 180.0f;
        f64 radians;
        angle += 90.0f;
        if (angle > 180.0f) angle -= 360.0f;
        radians = (M_PI * (f64)angle) / 180.0;
        radians = (f64)(f32)radians;
        lbl_1_bss_D4[index].x = (f32)(50.0 * cos(radians));
        lbl_1_bss_D4[index].y = 0.0f;
        lbl_1_bss_D4[index].z = (f32)(50.0 * sin(radians));
    }

    currentModel = motion;
    for (s32 spriteIndex = 0; spriteIndex < 0x10; spriteIndex++) {
        hideObject__5ModelFPc(&motion, objectNames[spriteIndex]);
        new Kuribo(model, spriteIndex);
    }
}

HU3D_MOTIONID KuriboMotionOwner::addMotion(unsigned int dataNum)
{
    HU3D_MOTIONID motionId = fn_80030458((model_id()), fn_80008DD0(dataNum, 0x10000000, 2));
    m_mot_id.push_back(motionId);
    return motionId;
}

KuriboMotionOwner::~KuriboMotionOwner()
{
    fn_1_653C();
}

extern "C" { void fn_1_6630(void); }

void KuriboMotionOwner::update()
{
    fn_1_6630();
}

/* Where object objectIndex of the model is now. */
void KuriboMotionOwner::fn_1_61D4(Vec *position, s32 objectIndex)
{
    static char *objectNames[16] = {
        "kuri01", "kuri02", "kuri03", "kuri04", "kuri05", "kuri06", "kuri07", "kuri08",
        "kuri09", "kuri10", "kuri11", "kuri12", "kuri13", "kuri14", "kuri15", "kuri16",
    };
    Hu3DModelObjPosGet((s16)motion, objectNames[objectIndex], position);
}

void KuriboMotionOwner::flagEnabled(int request)
{
    flags &= ~(1UL << request);
    switch (request) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        break;
    }
}

extern "C" { ListRoot<Kuribo> lbl_1_bss_26C; }

extern "C" { void fn_1_7C38(HU3D_MODEL *model, Mtx *matrix); }

Kuribo::Kuribo(const OwnedModel &member, s32 objectIndex) : model(member)
{
    s16 currentModel;
    HU3D_MODEL *model;

    this->objectIndex = objectIndex;
    this->updateCount = 0;
    this->insert4(&lbl_1_bss_26C);
    this->currentMotion = -1;
    currentModel = this->model;
    fn_80026A44(currentModel, (1 << 0));
    fn_80026B8C(currentModel, 0);
    fn_80026B64(currentModel, 1U);
    fn_800268B0(currentModel, 1.5f, 1.5f, 1.5f);
    if (GWMgNightFGet() != 0) {
        fn_800273EC(currentModel, 0.4f, 0.4f, 0.4f);
    }
    this->hookIndex = fn_800260F4(fn_1_7C38);
    model = &lbl_802C01C8[this->hookIndex];
    model->hookData = this;
    this->timer = (s32)(secondsToFrames(RandomFloat()) + secondsToFrames(1.0f));
    this->fn_1_67D4(0, 0);
    Kuribo::Callback callbackData = &Kuribo::fn_1_6D5C;
    this->callback = callbackData;
}

Kuribo::~Kuribo()
{
    fn_800262D0(hookIndex);
}

void fn_1_653C()
{
    while (!lbl_1_bss_26C.empty())
        delete lbl_1_bss_26C.back();
}

extern "C" {
void fn_1_6660(Kuribo *object);
void fn_1_66AC(Kuribo *object);
}

void fn_1_6630()
{
    lbl_1_bss_26C.visit(fn_1_6660);
}

void fn_1_6660(Kuribo *object)
{
    (object->*object->callback)();
    fn_1_66AC(object);
    object->updateCount++;
}

extern "C" { void setRotation__5ModelFP3Vec(void *object, Vec *rotation); }

extern "C" { void setPosition__5ModelFP3Vec(void *object, void *position); }

void fn_1_66AC(Kuribo *object)
{
    KuriboMotionOwner::getChecked().fn_1_61D4(&object->position, object->objectIndex);
    f64 angle = atan2((f64)object->position.x, -(f64)object->position.z) / M_PI * 180.0;
    angle = (f64)(f32)angle;
    Vec rotation;
    rotation.x = 0.0f;
    rotation.y = (f32)-angle;
    rotation.z = 0.0f;
    setRotation__5ModelFP3Vec(&object->model, &rotation);
    setPosition__5ModelFP3Vec(&object->model, &object->position);
}

void Kuribo::fn_1_67D4(s32 motionId, u8 queued)
{
    KuriboMotionOwner &manager = KuriboMotionOwner::getChecked();
    s16 modelId = model;
    s16 selectedMotion = manager.m_mot_id.at(motionId);
    if (currentMotion != motionId) {
        fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
        fn_8002CCC0(modelId, selectedMotion, 0.0f, 8.0f, queued == 0 ? ((1 << 30) | (1 << 0)) : 0);
        currentMotion = motionId;
    }
}

void Kuribo::fn_1_68D4()
{
    Callback completionCallback;
    Callback leftCallback;
    Callback centerCallback;
    Callback rightCallback;
    s32 completionState;
    completionState = currentRequest();
    if (completionState == 3) {
        completionCallback = &Kuribo::fn_1_7B4C;
        callback = completionCallback;
        return;
    }
    switch (RandomUnsigned() & 3) {
    case 0:
        timer = 10;
        fn_1_67D4(1, 1);
        leftCallback = &Kuribo::fn_1_71F8;
        callback = leftCallback;
        return;
    case 1:
    case 2:
        timer = (s32)(secondsToFrames(RandomFloat()) + secondsToFrames(1.0f));
        fn_1_67D4(2, 0);
        centerCallback = &Kuribo::fn_1_76B0;
        callback = centerCallback;
        return;
    default:
        timer = (s32)(secondsToFrames(RandomFloat()) + secondsToFrames(1.0f));
        fn_1_67D4(0, 0);
        rightCallback = &Kuribo::fn_1_6D5C;
        callback = rightCallback;
        return;
    }
}

void Kuribo::fn_1_6D5C()
{
    if ((timer -= 1) >= 0) return;
    fn_1_68D4();
}

void Kuribo::fn_1_71F8()
{
    if ((timer -= 1) >= 0) return;
    if (fn_8002D75C(model) == 0) return;
    fn_1_68D4();
}

void Kuribo::fn_1_76B0()
{
    if ((timer -= 1) >= 0) return;
    fn_1_68D4();
}

void Kuribo::fn_1_7B4C()
{
    fn_1_67D4(2, 0);
}

extern "C" {
void fn_1_7CB0(Kuribo *kuribo, f32 matrix[3][4]);
void fn_1_7C80(void *first, f32 matrix[3][4]);
}

void fn_1_7C38(HU3D_MODEL *model, Mtx *matrix)
{
    fn_8002472C();
    void *hookData = model->hookData;
    fn_1_7C80(hookData, *matrix);
}

void fn_1_7C80(void *first, f32 matrix[3][4])
{
    fn_1_7CB0((Kuribo *)first, matrix);
}

void fn_1_7CB0(Kuribo *kuribo, f32 matrix[3][4])
{
    s32 index;
    f32 baseX;
    f32 baseY;
    f32 baseZ;

    GXLoadPosMtxImm(matrix, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetChanCtrl((GXChannelID)4, (GXBool)0, (GXColorSrc)1, (GXColorSrc)1, GX_LIGHT_NULL, (GXDiffuseFn)0, (GXAttnFn)2);
    GXSetNumChans(1);
    GXSetNumTexGens(0);
    GXSetTevOrder((GXTevStageID)0, (GXTexCoordID)0xFF, (GXTexMapID)0xFF, (GXChannelID)4);
    GXSetTevOp((GXTevStageID)0, (GXTevMode)4);
    GXSetCullMode((GXCullMode)0);
    GXSetBlendMode((GXBlendMode)1, (GXBlendFactor)4, (GXBlendFactor)5, (GXLogicOp)0);
    GXSetZMode((GXBool)1, (GXCompare)3, (GXBool)0);
    GXSetVtxAttrFmt((GXVtxFmt)0, (GXAttr)9, (GXCompCnt)1, (GXCompType)4, 0);
    GXSetVtxAttrFmt((GXVtxFmt)0, (GXAttr)11, (GXCompCnt)1, (GXCompType)5, 0);
    GXClearVtxDesc();
    GXSetVtxDesc((GXAttr)9, (GXAttrType)1);
    GXSetVtxDesc((GXAttr)11, (GXAttrType)1);
    GXBegin((GXPrimitive)0xA0, (GXVtxFmt)0, 0x22);

    baseZ = kuribo->position.z;
    baseY = 1.0f + kuribo->position.y;
    baseX = kuribo->position.x;
    GXPosition3f32(baseX, baseY, baseZ);
    GXColor4u8(0, 0, 0, 0x80);
    for (index = 0; index < 33; ++index) {
        f32 vertexX;
        f32 vertexY;
        f32 vertexZ;

        vertexZ = kuribo->position.z + lbl_1_bss_D4[index].z;
        vertexY = 2.0f + kuribo->position.y;
        vertexX = kuribo->position.x + lbl_1_bss_D4[index].x;
        GXPosition3f32(vertexX, vertexY, vertexZ);
        GXColor4u8(0, 0, 0, 0);
    }
}
