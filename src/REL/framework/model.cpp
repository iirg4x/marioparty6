// Hu3D model ownership: creating, releasing and placing the models minigame objects own.
#include <cstddef>
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"

#include "REL/framework/model.h"

extern "C" {
#include "dolphin/mtx.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/hu3d.h"
}

// Set once by a minigame before it creates its models; every model created afterwards gets the
// matching attributes in apply_defaults.
unsigned char defaultAttributeA;
unsigned char defaultAttributesB;

// Gives a newly created model the attributes the minigame asked for.
void OwnedModel::apply_defaults()
{
    if (defaultAttributeA)
        setAttribute(1 << 14);
    if (defaultAttributesB) {
        setAttribute((1 << 30) | (1 << 6));
        setAttribute((1 << 30) | (1 << 0));
    }
}

// Creates a model from model data already in memory.
OwnedModel::OwnedModel(void *data)
{
    created(fn_80025A04(data));
}

// Creates a model from a data resource.
OwnedModel::OwnedModel(unsigned int resource)
{
    modelId = -1;
    load(resource);
}

// Reads a data resource and replaces the owned model with one made from it.
void OwnedModel::load(unsigned int resource)
{
    Replace(fn_80025A04(fn_80008DD0(resource, 0x10000000, 2)));
    apply_defaults();
}

// Creates a model that Hu3D draws through the given callback.
OwnedModel::OwnedModel(HU3D_MODEL_HOOK hook)
{
    created(fn_800260F4(hook));
}

// The character model functions select a costume variant by one bit of a mask.
int characterVariantMask(int variant)
{
    int mask;
    mask = 0;
    switch (variant) {
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

// Creates the model of a character in one of its costume variants.
OwnedModel::OwnedModel(int character, int variant)
{
    created(fn_80057068(character, characterVariantMask(variant)));
}

// Creates a copy of another owned model.
OwnedModel::OwnedModel(const OwnedModel &source)
{
    created(fn_80025D4C(source));
}

// Releases the current model, if any, and takes the new handle.
short Model::Replace(short replacement)
{
    if (modelId >= 0) {
        fn_800262D0(modelId);
    }
    return modelId = replacement;
}

// Turns the given Hu3D model attributes on.
void Model::setAttribute(unsigned long attributes)
{
    fn_800269E8(modelId, attributes);
}

// Turns the given Hu3D model attributes off.
void Model::resetAttribute(unsigned long attributes)
{
    fn_80026A44(modelId, attributes);
}

// Copies a position, rotation and scale to the model.
void Model::setTransform(Transform *transform)
{
    short handle = modelId;
    fn_80026760(handle, transform->pos.x, transform->pos.y, transform->pos.z);
    fn_80026808(handle, transform->rot.x, transform->rot.y, transform->rot.z);
    fn_800268B0(handle, transform->scale.x, transform->scale.y, transform->scale.z);
}

// Places the model in world space.
void Model::setPosition(Vec *position)
{
    fn_80026760(modelId, position->x, position->y, position->z);
}

// Reads the model's world position into the vector and returns it.
Vec *Model::getPosition(Vec *position)
{
    fn_800267CC((short)modelId, position);
    return position;
}

// Sets the model's rotation in degrees about each axis.
void Model::setRotation(Vec *rotation)
{
    fn_80026808(modelId, rotation->x, rotation->y, rotation->z);
}

// Reads the model's rotation into the vector and returns it.
Vec *Model::getRotation(Vec *rotation)
{
    fn_80026874((short)modelId, rotation);
    return rotation;
}

// Turns the model about the vertical axis only.
void Model::setRotationY(float angle)
{
    fn_80026808(modelId, 0.0f, angle, 0.0f);
}

// Returns the model's rotation about the vertical axis.
float Model::getRotationY()
{
    Vec rotation;
    fn_80026874((short)modelId, &rotation);
    return rotation.y;
}

// Scales the model separately along each axis.
void Model::setScale(Vec *scale)
{
    fn_800268B0(modelId, scale->x, scale->y, scale->z);
}

// Scales the model by the same amount along every axis.
void Model::setUniformScale(float scale)
{
    fn_800268B0(modelId, scale, scale, scale);
}

void Model::setTransparency(float level)
{
    fn_80026CBC(modelId, level);
}

void Model::setLayer(short layer)
{
    fn_80026B8C(modelId, layer);
}

void Model::setCamera(unsigned short cameraBit)
{
    fn_80026B64(modelId, cameraBit);
}

bool Model::isInView(unsigned short cameraBit)
{
    return fn_8002803C(modelId, cameraBit);
}

// Sets one of the model's named lights.
void Model::setLight(char *name, Vec *position, Vec *direction, GXColor *color)
{
    short id = fn_80029D24((short)modelId, name);
    assert(id >= 0);
    fn_800297D4((short)id, position, direction, color);
}

int Model::setLightsDynamic(bool dynamic) {
    return fn_80029850(modelId, !dynamic);
}

void Model::setAmbient(Vec *color) {
    HU3D_MODEL *model = &lbl_802C01C8[modelId];
    model->ambR = color->x;
    model->ambG = color->y;
    model->ambB = color->z;
}

void Model::setHookData(void *data) {
    lbl_802C01C8[modelId].hookData = data;
}

float *Model::getMatrix() {
    return lbl_802C01C8[modelId].mtx[0];
}

inline Mtx &Model::matrix() { return lbl_802C01C8[modelId].mtx; }

void Model::copy_matrix(Mtx source) {
    Mtx &destination = matrix();
    PSMTXCopy(source, destination);
}

// Finds one of the model's objects by name; the object must exist.
HsfObject_s *Model::getObject(char *name) {
    HSF_OBJECT *p = fn_80026BF4((short)modelId, name);
    assert(p != NULL);
    return p;
}

void Model::setShadow()
{
    fn_80026E44(modelId);
}

void Model::setObjectShadow(char *name)
{
    HSF_OBJECT *object = getObject(name);
    HSF_CONSTDATA *constant = (HSF_CONSTDATA *)object->constData;
    constant->attr |= (1 << 10);
}

void Model::resetObjectShadow(char *name)
{
    HSF_OBJECT *object = getObject(name);
    HSF_CONSTDATA *constant = (HSF_CONSTDATA *)object->constData;
    constant->attr &= ~(1 << 10);
}

void Model::resetShadow()
{
    fn_80026EF0(modelId);
}

void Model::setShadowMap()
{
    fn_80026FFC(modelId);
}

void Model::setShadowMapObject(char *name)
{
    fn_80027080(modelId, name);
}

void Model::resetShadowMap()
{
    fn_8002736C(modelId);
}

void Model::resetObjectShadowMap(char *name)
{
    HSF_OBJECT *object = getObject(name);
    HSF_CONSTDATA *constant = (HSF_CONSTDATA *)object->constData;
    constant->attr &= ~(1 << 3);
}

void Model::hideObject(char *name)
{
    HSF_OBJECT *object = getObject(name);
    object->flags |= (1 << 10);
}

void Model::showObject(char *name)
{
    HSF_OBJECT *object = getObject(name);
    object->flags &= ~(1 << 10);
}

// Hides an object and everything under it: its own scale goes to zero.
void Model::hideObjectTree(char *name)
{
    HSF_OBJECT *object = getObject(name);
    object->flags |= (1 << 10);
    HSF_TRANSFORM *base = &object->mesh.base;
    base->scale.x = 0.0f;
    base->scale.y = 0.0f;
    base->scale.z = 0.0f;
}

void Model::showObjectTree(char *name)
{
    HSF_OBJECT *object = getObject(name);
    object->flags &= ~(1 << 10);
    HSF_TRANSFORM *base = &object->mesh.base;
    base->scale.x = 1.0f;
    base->scale.y = 1.0f;
    base->scale.z = 1.0f;
}

Vec &Model::getObjectPosition(char *name, Vec *position)
{
    Hu3DModelObjPosGet((short)modelId, name, position);
    return *position;
}

void Model::setMotionTime(float time)
{
    fn_8002D414(modelId, time);
}

void Model::setOverlayMotionTime(float time)
{
    fn_8002CB64(modelId, time);
}

// During a motion blend the incoming motion's time is reported, one frame behind its engine clock.
float Model::getMotionTime()
{
    float time;
    if (fn_8002D3EC((short)modelId) < 0) {
        time = fn_8002D4E0((short)modelId);
    } else {
        time = fn_8002D508((short)modelId) - 1.0f;
    }
    return time;
}

float Model::getOverlayMotionTime()
{
    return fn_8002CB0C((short)modelId);
}

float Model::getMotionMaxTime()
{
    return fn_8002D530((short)modelId);
}

float Model::getOverlayMotionMaxTime()
{
    return fn_8002CB24((short)modelId);
}

void Model::setMotionSpeed(float speed)
{
    fn_8002D86C(modelId, speed);
    fn_8002D894(modelId, speed);
}

bool Model::isMotionEnd()
{
    bool done = false;
    if (fn_8002D3EC((short)modelId) < 0 && fn_8002D75C((short)modelId)) {
        done = true;
    }
    return done;
}

void Model::updateMotion()
{
    fn_80030644(modelId);
}

void Model::clearOverlayMotion()
{
    fn_8002CAF0(modelId);
}

void Model::setObjectAttribute(char *name, unsigned long attributes)
{
    fn_8002D8BC(modelId, name, attributes);
}

void Model::setHook(char *name, short hookModel)
{
    fn_8002741C(modelId, name, hookModel);
}

// Moves another model to the world position of one of this model's objects.
void Model::placeModelAtObject(char *name, short other)
{
    Vec position;
    Hu3DModelObjPosGet(modelId, name, &position);
    fn_80026760(other, position.x, position.y, position.z);
}

void enableDefaultAttributeA()
{
    defaultAttributeA = 1;
}

void enableDefaultAttributesB()
{
    defaultAttributesB = 1;
}
