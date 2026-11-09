#include <math.h>

#include "REL/m652DLL/framework/linknode.h"

extern "C" {
#include "game/gamework.h"
}

extern "C" {
#include "game/mgdata.h"
}

extern "C" {
#include "game/flag.h"
}
#include "REL/m652DLL/framework/model.h"
extern "C" {
#include "game/hu3d.h"
}
#define SINGLETON32930_INLINE_BASE_DESTRUCTOR
#include "REL/m652DLL/singleton_32930.h"
#undef SINGLETON32930_INLINE_BASE_DESTRUCTOR

extern "C" {
#include "game/charman.h"
}

extern "C" {
#include "game/data.h"
}

extern "C" {
#include "game/mg/timer.h"
}

extern "C" {
#include "game/sprite.h"
}

extern "C" {
#include "game/audio.h"
}

extern "C" {
#include "game/window.h"
}

#include "game/main.h"

#include "game/object.h"

#include "game/wipe.h"

void operator delete(void *);

extern "C" { unsigned char lbl_1_bss_108, lbl_1_bss_109; }

struct OwnedModel:Model {
    explicit OwnedModel(void *);
    explicit OwnedModel(unsigned int);
    explicit OwnedModel(HU3D_MODEL_HOOK);
    OwnedModel(int,int);
    explicit OwnedModel(const OwnedModel*);
    ~OwnedModel();
    void apply_defaults();
    void load(unsigned int);
    operator short() const { return modelId; }
    short id() const { return modelId; }
    void created(short value);
};

inline void OwnedModel::created(short value)
{
    modelId = value;
    if (lbl_1_bss_108 != 0) {
        fn_1_33950((1 << 14));
    }
    if (lbl_1_bss_109 != 0) {
        fn_1_33950(((1 << 30) | (1 << 6)));
        fn_1_33950(((1 << 30) | (1 << 0)));
    }
}

void OwnedModel::apply_defaults()
{
    if(lbl_1_bss_108) fn_1_33950((1 << 14));
    if(lbl_1_bss_109) {
        fn_1_33950(((1 << 30) | (1 << 6)));
        fn_1_33950(((1 << 30) | (1 << 0)));
    }
}

OwnedModel::OwnedModel(void *data)
{
    created(fn_80025A04(data));
}

Model::~Model() { Replace(-1); }

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
    case 0: mask=(1 << 0); break;
    case 1: mask=(1 << 1); break;
    case 2: mask=(1 << 2); break;
    case 3: mask=(1 << 3); break;
    }
    return mask;
}

extern "C" { extern char lbl_1_data_1662[]; }

extern "C" { extern char lbl_1_data_1658[]; }

extern "C" { void __msl_assertion_failed(const char *,const char *,int); }

struct ModelObject : Model {
    HSF_OBJECT *lookup(char *name) {
        HSF_OBJECT *object=fn_80026BF4((short)modelId,name);
        object ? (void)0 : __msl_assertion_failed(lbl_1_data_1662,lbl_1_data_1658,230);
        return object;
    }
    void fn_1_33EC0(Mtx);
    void fn_1_33FC8(char*);
    void fn_1_34050(char*);
    void fn_1_341A0(char*);
    void fn_1_34228(char*);
    void fn_1_342AC(char*);
    void fn_1_34330(char*);
    void fn_1_343E8(char*);
    Mtx& matrix() { return lbl_802C01C8[modelId].mtx; }
};

extern "C" { extern char lbl_1_data_2958[]; }

extern "C" { extern char lbl_1_data_295E[]; }

extern "C" { extern char lbl_1_data_2968[]; }

struct TimerOwner {
    MGTIMER *timer;
    TimerOwner(int);
    ~TimerOwner();
    void fn_1_3AFE8(int,int,int);
    void fn_1_3B060(float,float);
    void fn_1_3B0A0(float*,float*);
    void fn_1_3B0E0(int);
    void fn_1_3B1AC();
    void fn_1_3B284(int);
    void fn_1_3B2D0(unsigned char,unsigned char,unsigned char);
};

extern "C" {

extern s32 lbl_1_data_F18;

extern float lbl_1_bss_1BC;

extern s8 lbl_1_bss_218;

extern s8 lbl_1_bss_219;

extern s8 lbl_1_data_2758;

s32 fn_1_367CC(s32 *player);
}

s32 fn_1_190A8(s32 index)
{
    s32 mask;
    mask = 0;
    switch (index) {
    case 0:
        mask = 1;
        break;
    case 1:
        mask = 2;
        break;
    case 2:
        mask = 4;
        break;
    case 3:
        mask = 8;
        break;
    }
    return mask;
}

OwnedModel::OwnedModel(int charNo,int variant)
{
    created(fn_80057068(charNo,model_variant_mask(variant)));
}

extern "C" unsigned char lbl_1_bss_108;

extern "C" unsigned char lbl_1_bss_109;

static inline short model_id(const Model *source)
{
    return source->modelId;
}

OwnedModel *fn_1_19210(OwnedModel *self, const OwnedModel *source)
{
    self->created(fn_80025D4C(model_id(source)));
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

extern "C" {
extern char lbl_1_data_26B8[];

void __msl_assertion_failed(const char *, const char *, int);
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

extern "C" {
extern char lbl_1_data_1650[], lbl_1_data_1658[];
}

void Model::fn_1_33D68(char *name, HuVecF *pos, HuVecF *dir, GXColor *color)
{
    short light = fn_80029D24((short)modelId, name);
    (light >= 0) ? (void)0 : __msl_assertion_failed(lbl_1_data_1650, lbl_1_data_1658, 197);
    fn_800297D4((short)light, pos, dir, color);
}

int Model::fn_1_33DF8(bool moving)
{
    return fn_80029850(modelId, !moving);
}

void Model::fn_1_33E3C(HuVecF *color)
{
    HU3D_MODEL *model = &lbl_802C01C8[modelId];
    model->ambR = color->x;
    model->ambG = color->y;
    model->ambB = color->z;
}

void Model::fn_1_33E80(void *hook) { lbl_802C01C8[modelId].hookData = hook; }

float *Model::fn_1_33EA0() { return lbl_802C01C8[modelId].mtx[0]; }

/* Consumed signed model ID selects its matrix in the SDK model array. */

inline Mtx &Model::matrix() { return lbl_802C01C8[modelId].mtx; }

void Model::copy_matrix(Mtx source)
{
    Mtx &destination = matrix();
    PSMTXCopy(source, destination);
}

extern "C" {
extern char lbl_1_data_1662[], lbl_1_data_1658[];
void fn_8002736C(short);
}

HSF_OBJECT *Model::fn_1_33F28(char *name)
{
    HSF_OBJECT *object = fn_80026BF4((short)modelId, name);
    object ? (void)0 : __msl_assertion_failed(lbl_1_data_1662, lbl_1_data_1658, 230);
    return object;
}

void Model::fn_1_33F98() { fn_80026E44(modelId); }

extern "C" signed char lbl_1_bss_218, lbl_1_bss_219;

extern "C" { extern char lbl_1_data_1662[]; }

extern "C" { extern char lbl_1_data_1658[]; }

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

extern "C" {

extern unsigned char lbl_1_bss_108;

extern unsigned char lbl_1_bss_109;
}

void fn_1_1A220(void)
{
    lbl_1_bss_108 = 1;
}

void fn_1_1A234(void)
{
    lbl_1_bss_109 = 1;
}

/* Model operation assertion messages. */
char lbl_1_data_1650[8] = "id >= 0";
char lbl_1_data_1658[10] = "model.cpp";
char lbl_1_data_1662[14] = "p != NULL";
