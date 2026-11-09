#include "dolphin/types.h"
extern "C" {
#include "math.h"
#include "game/hu3d.h"
#include "game/charman.h"
#include "game/data.h"
}
#include "REL/m681DLL/character_sequence.h"

extern "C" void __msl_assertion_failed(const char *, const char *, int);


#include "REL/m681DLL/owned_model.h"

extern "C" unsigned char lbl_1_bss_F0[1], lbl_1_bss_F1[7];

void OwnedModel::apply_defaults()
{
    if(lbl_1_bss_F0[0]) fn_1_33950((1 << 14));
    if(lbl_1_bss_F1[0]) {
        fn_1_33950(((1 << 30) | (1 << 6)));
        fn_1_33950(((1 << 30) | (1 << 0)));
    }
}

OwnedModel::OwnedModel(void *data)
{
    created(fn_80025A04(data));
}


OwnedModel::OwnedModel(unsigned int dataNum)
{
    modelId=-1;
    load(dataNum);
}

void OwnedModel::load(unsigned int dataNum)
{
    Replace(fn_80025A04(fn_80008DD0(dataNum,0x10000000,2)));
    apply_defaults();
}

OwnedModel::OwnedModel(HU3D_MODEL_HOOK hook)
{
    created(fn_800260F4(hook));
}

/* The standalone variant-to-mask helper is retained at fn_1_336F0.
 * Its inline return is visible in the character construction path. */
static inline int model_variant_mask(int variant)
{
    int mask=0;
    switch(variant) {
    case 0: mask=1; break;
    case 1: mask=2; break;
    case 2: mask=4; break;
    case 3: mask=8; break;
    }
    return mask;
}


s32 fn_1_18D00(s32 variant)
{
    s32 variantMask;
    variantMask = 0;
    switch (variant) {                                 /* irregular */
    case 0:
        variantMask = 1;
        break;
    case 1:
        variantMask = 2;
        break;
    case 2:
        variantMask = 4;
        break;
    case 3:
        variantMask = 8;
        break;
    }
    return variantMask;
}

OwnedModel::OwnedModel(int charNo,int variant)
{
    created(fn_80057068(charNo,model_variant_mask(variant)));
}

OwnedModel *fn_1_18E68(OwnedModel *self, const OwnedModel *source)
{
    self->created(fn_80025D4C(*source));
    return self;
}

short Model::Replace(short value)
{
    if (modelId >= 0) {
        fn_800262D0(modelId);
    }
    return modelId = value;
}

void Model::fn_1_33950(unsigned long flags)
{
    fn_800269E8(modelId, flags);
}

void Model::fn_1_33988(unsigned long flags)
{
    fn_80026A44(modelId, flags);
}


void Model::fn_1_339C0(Transform *transform)
{
    short handle = modelId;
    fn_80026760(handle, transform->pos.x, transform->pos.y, transform->pos.z);
    fn_80026808(handle, transform->rot.x, transform->rot.y, transform->rot.z);
    fn_800268B0(handle, transform->scale.x, transform->scale.y, transform->scale.z);
}

void Model::fn_1_33A3C(HuVecF *pos)
{
    fn_80026760(modelId, pos->x, pos->y, pos->z);
}

HuVecF *Model::fn_1_33A84(HuVecF *pos)
{
    fn_800267CC((short)modelId, pos);
    return pos;
}

void Model::fn_1_33ACC(HuVecF *rot)
{
    fn_80026808(modelId, rot->x, rot->y, rot->z);
}

HuVecF *Model::fn_1_33B14(HuVecF *rot)
{
    fn_80026874((short)modelId, rot);
    return rot;
}

void Model::fn_1_33B5C(float angle)
{
    fn_80026808(modelId, 0.0f, angle, 0.0f);
}

float Model::fn_1_33BAC()
{
    HuVecF rot;
    fn_80026874((short)modelId, &rot);
    return rot.y;
}

void Model::fn_1_33BE8(HuVecF *scale)
{
    fn_800268B0(modelId, scale->x, scale->y, scale->z);
}

void Model::fn_1_33C30(float scale)
{
    fn_800268B0(modelId, scale, scale, scale);
}


void Model::fn_1_33C78(float level)
{
    fn_80026CBC(modelId, level);
}

void Model::fn_1_33CB0(short layer)
{
    fn_80026B8C(modelId, layer);
}

void Model::fn_1_33CE8(unsigned short camera)
{
    fn_80026B64(modelId, camera);
}

bool Model::fn_1_33D20(unsigned short camera)
{
    return fn_8002803C(modelId, camera);
}


void Model::fn_1_33D68(char *name, HuVecF *pos, HuVecF *dir, GXColor *color)
{
    short id = fn_80029D24((short)modelId, name);
    (id >= 0) ? (void)0 : __msl_assertion_failed("id >= 0", "model.cpp", 197);
    fn_800297D4((short)id, pos, dir, color);
}

void Model::fn_1_33DF8(bool moving)
{
    fn_80029850(modelId, !moving);
}

void Model::fn_1_33E3C(HuVecF *color)
{
    HU3D_MODEL *model = &lbl_802C01C8[modelId];
    model->ambR = color->x;
    model->ambG = color->y;
    model->ambB = color->z;
}

void Model::setHookData(void *hook) { lbl_802C01C8[modelId].hookData = hook; }

float *Model::fn_1_33EA0() { return lbl_802C01C8[modelId].mtx[0]; }


struct ModelObject : Model {
    HSF_OBJECT *lookup(char *name) {
        HSF_OBJECT *p=fn_80026BF4((short)modelId,name);
        p ? (void)0 : __msl_assertion_failed("p != NULL","model.cpp",230);
        return p;
    }
    void fn_1_33EC0(Mtx);
    void copy_matrix(Mtx source);
    void fn_1_33FC8(char*);
    void fn_1_34050(char*);
    void fn_1_341A0(char*);
    void fn_1_34228(char*);
    void fn_1_342AC(char*);
    void fn_1_34330(char*);
    void fn_1_343E8(char*);
    Mtx& matrix() { return lbl_802C01C8[modelId].mtx; }
};
void ModelObject::copy_matrix(Mtx source)
{
    Mtx &destination = matrix();
    PSMTXCopy(source, destination);
}


HSF_OBJECT *Model::fn_1_33F28(char *name)
{
    HSF_OBJECT *p = fn_80026BF4((short)modelId, name);
    p ? (void)0 : __msl_assertion_failed("p != NULL", "model.cpp", 230);
    return p;
}


void Model::fn_1_33F98() { fn_80026E44(modelId); }


void ModelObject::fn_1_33FC8(char *name) {
    HSF_OBJECT *object=lookup(name);
    HSF_CONSTDATA *constant=(HSF_CONSTDATA*)object->constData;
    constant->attr |= (1 << 10);
}

void ModelObject::fn_1_34050(char *name) {
    HSF_OBJECT *object=lookup(name);
    HSF_CONSTDATA *constant=(HSF_CONSTDATA*)object->constData;
    constant->attr &= ~(1 << 10);
}


void Model::fn_1_340D8() { fn_80026EF0(modelId); }

void Model::fn_1_34108() { fn_80026FFC(modelId); }


extern "C" { void fn_8002736C(short); }

void Model::fn_1_34138(char *name)
{
    fn_80027080(modelId, name);
}

void Model::fn_1_34170()
{
    fn_8002736C(modelId);
}


void ModelObject::fn_1_341A0(char *name) {
    HSF_OBJECT *object=lookup(name);
    HSF_CONSTDATA *constant=(HSF_CONSTDATA*)object->constData;
    constant->attr &= ~(1 << 3);
}

void ModelObject::fn_1_34228(char *name) { HSF_OBJECT *object=lookup(name); object->flags |= (1 << 10); }

void ModelObject::fn_1_342AC(char *name) { HSF_OBJECT *object=lookup(name); object->flags &= ~(1 << 10); }

void ModelObject::fn_1_34330(char *name)
{
    HSF_OBJECT *object=lookup(name);
    object->flags |= (1 << 10);
    HSF_TRANSFORM *base=&object->mesh.base;
    base->scale.x=0.0f;
    base->scale.y=0.0f;
    base->scale.z=0.0f;
}

void ModelObject::fn_1_343E8(char *name)
{
    HSF_OBJECT *object=lookup(name);
    object->flags &= ~(1 << 10);
    HSF_TRANSFORM *base=&object->mesh.base;
    base->scale.x=1.0f;
    base->scale.y=1.0f;
    base->scale.z=1.0f;
}


HuVecF *Model::fn_1_344A0(char *name, HuVecF *pos)
{
    Hu3DModelObjPosGet((short)modelId, name, pos);
    return pos;
}

void Model::fn_1_344F0(float time)
{
    fn_8002D414(modelId, time);
}

void Model::fn_1_34528(float time)
{
    fn_8002CB64(modelId, time);
}

float Model::fn_1_34560()
{
    float time;
    if (fn_8002D3EC((short)modelId) < 0) {
        time = fn_8002D4E0((short)modelId);
    } else {
        time = fn_8002D508((short)modelId) - 1.0f;
    }
    return time;
}

float Model::fn_1_345E4() { return fn_8002CB0C((short)modelId); }

float Model::fn_1_34618() { return fn_8002D530((short)modelId); }

float Model::fn_1_3464C() { return fn_8002CB24((short)modelId); }

void Model::fn_1_34680(float speed)
{
    fn_8002D86C(modelId, speed);
    fn_8002D894(modelId, speed);
}

int Model::fn_1_346CC()
{
    int done = 0;
    if (fn_8002D3EC((short)modelId) < 0 && fn_8002D75C((short)modelId)) {
        done = 1;
    }
    return done;
}

void Model::fn_1_34734() { fn_80030644(modelId); }

void Model::fn_1_34764() { fn_8002CAF0(modelId); }

void Model::fn_1_34794(char *name, unsigned long attr) { fn_8002D8BC(modelId, name, attr); }

void Model::fn_1_347D4(char *name, short hook) { fn_8002741C(modelId, name, hook); }

void Model::fn_1_34814(char *name, short other)
{
    HuVecF pos;
    Hu3DModelObjPosGet(modelId, name, &pos);
    fn_80026760(other, pos.x, pos.y, pos.z);
}


void fn_1_19E78(void)
{
    lbl_1_bss_F0[0] = 1;
}

void fn_1_19E8C(void)
{
    lbl_1_bss_F1[0] = 1;
}

extern "C" {
unsigned char lbl_1_bss_F0[1];
unsigned char lbl_1_bss_F1[7];
}
