#include "REL/m648DLL/particle_entry.h"
#include "humath.h"
float RandomFloat(float, float);

float RandomFloat(void);

unsigned long RandomUnsigned(void);

extern "C" {
#include "dolphin/os.h"
}

extern "C" {
#include "REL/m648DLL/positioned_model_owner.h"
}

extern "C" {
#include "dolphin/types.h"
}

extern "C" {
#include "dolphin/gx.h"
}

extern "C" {
#include "dolphin/mtx.h"
}

extern "C" {
#include "game/hu3d.h"
}

extern "C" {
#include "game/data.h"
}

extern "C" {
#include "game/memory.h"
}

extern "C" {
#include "game/sprite.h"
}

extern "C" {
#include "REL/framework/assertion.h"
}

extern "C" {
#include "REL/m648DLL/camera_view.h"
}

extern "C" {
#include <math.h>
}

extern "C" {
extern void *__ct__4NodeFv(void *);
}

extern "C" {
extern u8 lbl_1_data_5C[];
}

#include "REL/m648DLL/list_object.h"

struct NodeOwner : Node { ~NodeOwner() {} };

struct ModelNode : NodeOwner {
    short modelId;
    ~ModelNode() { fn_800262D0(modelId); }
};

struct HookedModelCleanup : Node { s16 modelId; };

struct ParticleListRoot : Node {
    ~ParticleListRoot() {}
    Node *first_node() { return link0; }
    HookedModelCleanup *begin() { return static_cast<HookedModelCleanup *>(first_node()); }
    Node *end() { return this; }
    bool empty() const { return link0==this && link0==link4; }
    Node *last_node() { return link4; }
    ModelNode *back() { return static_cast<ModelNode *>(last_node()); }
    void fn_1_A630(void (*visitor)(HookedModelCleanup *));
};

struct AlternateParticleListRoot : Node {
    ~AlternateParticleListRoot() {}
    Node *first_node() { return link0; }
    HookedModelCleanup *begin() { return static_cast<HookedModelCleanup *>(first_node()); }
    Node *end() { return this; }
    bool empty() const { return link0==this && link0==link4; }
    Node *last_node() { return link4; }
    ModelNode *back() { return static_cast<ModelNode *>(last_node()); }
    void fn_1_A690(void (*visitor)(HookedModelCleanup *));
};

typedef ParticleListRoot ModelRoot;

extern "C" {
extern ModelRoot lbl_1_bss_5AC;
extern AlternateParticleListRoot lbl_1_bss_5C0;
}

#include "REL/m648DLL/managers.h"

extern "C" {
Vec lbl_1_bss_288[0x21];
}

extern "C" {
Vec lbl_1_bss_414[0x21];
}

extern "C" {
static const s32 animationDataNumbers8FA4[2] = {
    DATANUM(0x460000, 25), DATANUM(0x460000, 27)
};
}

void fn_1_9BA4(void);

void fn_1_A12C(void);

extern "C" {
void *__dt__4NodeFv(void *object, s16 mode);
}

extern "C" {
void __dl__FPv(void *object);
}

void fn_1_970C();
void fn_1_9C74();
void fn_1_A218();

ANIMDATA *fn_1_93AC(u32 entryIndex);

struct ParticleModelEntry : Node {
    HU3D_MODELID modelId;
    s32 state;
};

void fn_1_97C0(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

void fn_1_9D28(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
extern const GXColor lbl_1_rodata_2B8[8];
}

#include "REL/m648DLL/framework/linknode.h"

struct ParticleEntryNode : Node { ~ParticleEntryNode(); };

struct ParticleListNode {
    ParticleListNode *previous;
    ParticleListNode *next;
};

struct ParticleModelOwner : ParticleListNode {
    short model_id;
    int state;
    ~ParticleModelOwner();
};

extern "C" {
}

void operator delete(void *);

void fn_1_963C(void);

extern "C" {
}

void fn_1_973C(HookedModelCleanup *);

extern "C" {
}

extern "C" {
#include "dolphin.h"
}

extern "C" {
}

struct AlternateParticleEntryNode : Node { ~AlternateParticleEntryNode(); };

struct ParticleModelOwnerAlt : ParticleListNode {
    short model_id;
    int state;
    ~ParticleModelOwnerAlt();
};

extern "C" {
}

void operator delete(void *);

void fn_1_9BA4(void);

extern "C" {
}

void fn_1_9CA4(HookedModelCleanup *);

extern "C" {
}

extern "C" {
}

extern "C" {
}

#include "game/main.h"

#include "game/object.h"

#include "game/audio.h"

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/gamework.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

class EffectNode;

extern "C" void Replace__5ModelFs(s16 *, s16);

class LinkFields {
public:
    LinkFields *next;
    LinkFields *previous;
};

class EffectNode {
public:
    LinkFields links;
    s16 hookId;
    s32 sourceId;
    f32 interval;
    s32 mode;
    f32 progress;
    f32 updateRate;
    f32 threshold;
    s32 state;
    ~EffectNode();
};

extern "C" {
}

void fn_1_A0E8(HU3D_MODEL *model, Mtx *data);

#include "REL/m648DLL/framework/model.h"

void operator delete(void *);

typedef Model ReleasedModel;

struct OwnedReleasedModel : ReleasedModel { ~OwnedReleasedModel() {} };

struct DrawNode : NodeOwner,OwnedReleasedModel { ~DrawNode() {} };

struct PositionedModelListRoot : Node {
    ~PositionedModelListRoot() {}
    Node *first_node() { return link0; }
    PositionedModelOwner *front() { return static_cast<PositionedModelOwner *>(first_node()); }
    bool empty() const { return link0==this && link0==link4; }
    Node *last_node() { return link4; }
    DrawNode *back() { return static_cast<DrawNode *>(last_node()); }
};

extern PositionedModelListRoot lbl_1_bss_5D4;

void fn_1_A12C(void);

/* Recipient symbol bindings for a retained source (copy of m648DLL fn_1_728). */
struct PositionedModelListRoot;

class PositionedModelOwner;

extern "C" {
}

void fn_1_A248(PositionedModelOwner *);

void fn_1_A6F0(PositionedModelListRoot *, void (*)(PositionedModelOwner *));

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

extern "C" {
}

void fn_1_A898(Node *, Node *, void (*)(HookedModelCleanup *));

void fn_1_A838(Node *, Node *, void (*)(HookedModelCleanup *));

#include "REL/m648DLL/linknode.h"

void fn_1_A6F0(PositionedModelListRoot *list, void (*callback)(PositionedModelOwner *))
;

ParticleAnimationOwner::ParticleAnimationOwner() {
    s32 animationIndex;
    s32 directionIndex;
    for (animationIndex = 0; animationIndex < 2; ++animationIndex) {
        animations[animationIndex] =
            HuSprAnimRead(fn_80008DD0(animationDataNumbers8FA4[animationIndex],
                                              0x10000000, 2));
        HuSprAnimLock(animations[animationIndex]);
    }

    for (directionIndex = 0; directionIndex < 0x21; ++directionIndex) {
        f32 angle = 360.0f * ((f32)(directionIndex & 0x1F) / 32.0f) - 180.0f;
        f64 radians;

        angle += 90.0f;
        if (angle > 180.0f) {
            angle -= 360.0f;
        }
        radians = (M_PI * (f64)angle) / 180.0;
        radians = (f64)(f32)radians;
        lbl_1_bss_414[directionIndex].x = (f32)(0.7 * cos(radians));
        lbl_1_bss_414[directionIndex].y = (f32)(0.7 * sin(radians));
        lbl_1_bss_414[directionIndex].z = 0.0f;
        lbl_1_bss_288[directionIndex].x = (f32)cos(radians);
        lbl_1_bss_288[directionIndex].y = (f32)sin(radians);
        lbl_1_bss_288[directionIndex].z = 0.0f;
    }
}

void fn_1_963C(void);

ParticleAnimationOwner::~ParticleAnimationOwner() {
    fn_1_963C();
    fn_1_9BA4();
    fn_1_A12C();
    for (s32 spriteIndex = 0; spriteIndex < 2; spriteIndex++) HuSprAnimKill(animations[spriteIndex]);
}

ANIMDATA *fn_1_93AC(u32 entryIndex)
{
    return ParticleAnimationOwner::getChecked().animations[entryIndex];
}

void ParticleAnimationOwner::clearRequest(int request) { state &= ~(1UL << request); }

void ParticleAnimationOwner::update() { fn_1_970C(); fn_1_9C74(); fn_1_A218(); }

ParticleModelEntry *fn_1_946C(ParticleModelEntry *object, HuVecF *position)
{
    ANIMDATA *particleAnimation;
    ParticleModelEntry *node;

    node = object;
    __ct__4NodeFv(node);
    object->insert4(&lbl_1_bss_5AC);
    particleAnimation = fn_1_93AC(0);
    object->modelId = fn_8003216C(particleAnimation, 1);
    fn_80026790(object->modelId, position);
    fn_80026B8C(object->modelId, 5);
    fn_800324E0(object->modelId, 100.0f);
    fn_800326D0(object->modelId, fn_1_97C0);
    fn_80032620(object->modelId, 0.0f);
    fn_8003269C(object->modelId, 0);
    object->state = 0;
    return object;
}

ParticleEntryNode::~ParticleEntryNode() {}

extern "C" void fn_800262D0(short model_id);

ParticleModelOwner::~ParticleModelOwner() {
    fn_800262D0(model_id);
    if (this != 0) {
        __dt__4NodeFv(this, 0);
    }
}

void fn_1_963C() {
    while(!lbl_1_bss_5AC.empty()) delete lbl_1_bss_5AC.back();
}

void fn_1_970C()
{
    lbl_1_bss_5AC.fn_1_A630(fn_1_973C);
}

void fn_1_973C(struct HookedModelCleanup *owner)
{
    HU3D_PARTICLE *particle = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[owner->modelId].hookData);

    if (particle->dataCnt < 0 && owner) {
        fn_800262D0(owner->modelId);
        if (owner) {
            __dt__4NodeFv(owner, 0);
        }
        __dl__FPv(owner);
    }
}

void fn_1_97C0(HU3D_MODEL *model, HU3D_PARTICLE *owner, Mtx matrix)
{
    HU3D_PARTICLE_DATA *end = owner->data + owner->maxCnt;
    HU3D_PARTICLE_DATA *initializingEntry;
    HU3D_PARTICLE_DATA *entry;
    s32 green;
    s32 blue;
    s32 alpha;
    s32 updatedCount;

    if (owner->count == 0) {
        for (initializingEntry = owner->data; initializingEntry != end;
             ++initializingEntry) {
            initializingEntry->vel.x = 2.5f *
                                RandomFloat(-1.0f,
                                           1.0f);
            initializingEntry->vel.y = 2.5f *
                                RandomFloat(-1.0f,
                                           1.0f);
            initializingEntry->vel.z = 2.5f *
                                RandomFloat(-1.0f,
                                           1.0f);
            initializingEntry->pos.x = initializingEntry->pos.y =
                initializingEntry->pos.z =
                0.0f;
            initializingEntry->color.r = 0x80;
            initializingEntry->color.g = 0xFF;
            initializingEntry->color.b = 0xFF;
            initializingEntry->color.a = 0x80;
        }
    }

    updatedCount = 0;
    for (entry = owner->data; entry != end; ++entry) {
        entry->vel.y = (f32)(entry->vel.y - 0.27240694444444447);
        PSVECScale(&entry->vel, &entry->vel, 0.8f);
        PSVECAdd(&entry->vel, &entry->pos, &entry->pos);
        entry->color.r = 0xFF;
        green = entry->color.g;
        green += 0x20;
        if (green > 0xFF) {
            green = 0xFF;
        }
        entry->color.g = (u8)green;
        blue = entry->color.b;
        blue += 0x20;
        if (blue > 0xFF) {
            blue = 0xFF;
        }
        entry->color.b = (u8)blue;
        alpha = entry->color.a;
        alpha -= 8;
        if (alpha < 0) {
            alpha = 0;
            entry->scale = 0.0f;
            owner->dataCnt = -1;
        }
        entry->color.a = (u8)alpha;
        ++updatedCount;
    }

    owner->emitCnt = (s16)updatedCount;
    DCStoreRangeNoSync(owner->data, owner->maxCnt * 0x48);
}

ParticleModelEntry *fn_1_99D4(ParticleModelEntry *object, HuVecF *position)
{
    ANIMDATA *particleAnimation;
    ParticleModelEntry *node;

    node = object;
    __ct__4NodeFv(node);
    object->insert4(&lbl_1_bss_5C0);
    particleAnimation = fn_1_93AC(1);
    object->modelId = fn_8003216C(particleAnimation, 8);
    fn_80026790(object->modelId, position);
    fn_80026B8C(object->modelId, 5);
    fn_800324E0(object->modelId, 40.0f);
    fn_800326D0(object->modelId, fn_1_9D28);
    fn_80032620(object->modelId, 0.0f);
    fn_8003269C(object->modelId, 0);
    object->state = 0;
    return object;
}

AlternateParticleEntryNode::~AlternateParticleEntryNode() {}

ParticleModelOwnerAlt::~ParticleModelOwnerAlt() {
    fn_800262D0(model_id);
    if (this != 0) {
        __dt__4NodeFv(this, 0);
    }
}

void fn_1_9BA4() {
    while(!lbl_1_bss_5C0.empty()) delete lbl_1_bss_5C0.back();
}

void fn_1_9C74()
{
    lbl_1_bss_5C0.fn_1_A690(fn_1_9CA4);
}

void fn_1_9CA4(struct HookedModelCleanup *owner)
{
    HU3D_PARTICLE *particle = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[owner->modelId].hookData);

    if (particle->dataCnt < 0 && owner) {
        fn_800262D0(owner->modelId);
        if (owner) {
            __dt__4NodeFv(owner, 0);
        }
        __dl__FPv(owner);
    }
}

extern "C" {
const GXColor lbl_1_rodata_2B8[8] = {
    {0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xC0, 0xFF},
    {0xFF, 0xC0, 0xFF, 0xFF},
    {0xFF, 0xC0, 0xC0, 0xFF},
    {0xC0, 0xFF, 0xFF, 0xFF},
    {0xC0, 0xFF, 0xC0, 0xFF},
    {0xC0, 0xC0, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF},
};
}
void fn_1_9D28(HU3D_MODEL *model, HU3D_PARTICLE *owner, Mtx matrix)
{
    HU3D_PARTICLE_DATA *end = owner->data + owner->maxCnt;
    HU3D_PARTICLE_DATA *initializingEntry;
    HU3D_PARTICLE_DATA *entry;
    const GXColor *color;
    s32 alpha;

    if (owner->count == 0) {
        for (initializingEntry = owner->data; initializingEntry != end;
             ++initializingEntry) {
            initializingEntry->vel.x = 2.5f *
                RandomFloat(-1.0f, 1.0f);
            initializingEntry->vel.y = 2.5f *
                RandomFloat(-1.0f, 1.0f);
            initializingEntry->vel.z = RandomFloat(0.0f,
                                                       4.0f);
            initializingEntry->pos.x = initializingEntry->pos.y =
                initializingEntry->pos.z = 0.0f;
            color = &lbl_1_rodata_2B8[RandomUnsigned() & 7];
            initializingEntry->color = *color;
            initializingEntry->zRot = (f32)(M_PI *
                (f64)(360.0f * RandomFloat()) / 180.0);
        }
    }

    for (entry = owner->data; entry != end; ++entry) {
        PSVECAdd(&entry->vel, &entry->pos, &entry->pos);
        alpha = entry->color.a;
        alpha -= owner->emitCnt;
        if (alpha < 0) {
            alpha = 0;
            entry->scale = 0.0f;
            owner->dataCnt = -1;
        }
        entry->color.a = (u8)alpha;
        entry->zRot = (f32)(M_PI *
            (5.0 + 180.0 *
             ((f64)entry->zRot / M_PI)) / 180.0);
    }

    owner->emitCnt += 1;
    DCStoreRangeNoSync(owner->data, owner->maxCnt * 0x48);
}

PositionedModelOwner::PositionedModelOwner(const Vec *value)
    : model(fn_1_A0E8), position(*value)
{
    insert4(&lbl_1_bss_5D4);
    model.fn_1_33E80(this);
    model.fn_1_33CB0(5);
    updatesRemaining = 0xF;
}

EffectNode::~EffectNode() {
    if (&hookId != 0) {
        if (&hookId != 0) {
            Replace__5ModelFs(&hookId, -1);
        }
    }
    if (this != 0) {
        __dt__4NodeFv(this, 0);
    }
}

void fn_1_A0E8(HU3D_MODEL *model, Mtx *data) {
    PositionedModelOwner *hookData = static_cast<PositionedModelOwner *>(model->hookData);
    hookData->fn_1_A2F8(*data);
}

void fn_1_A12C() {
    while(!lbl_1_bss_5D4.empty()) delete lbl_1_bss_5D4.back();
}

void fn_1_A218()
{
    fn_1_A6F0(&lbl_1_bss_5D4,
                           fn_1_A248);
}

void fn_1_A248(PositionedModelOwner *object) { object->fn_1_A270(); }

void PositionedModelOwner::fn_1_A270() {
    updatesRemaining = updatesRemaining - 1;
    if ((updatesRemaining < 0) && this) {
        if (&model.modelId && &model.modelId) {
            Replace__5ModelFs(&model.modelId, -1);
        }
        if (this) {
            __dt__4NodeFv(this, 0);
        }
        __dl__FPv(this);
    }
}

void PositionedModelOwner::fn_1_A2F8(Mtx viewMatrix)
{
    Mtx inverseView;
    Mtx translatedView;
    s32 alpha;
    f32 radius;
    s32 index;

    PSMTXInverse(viewMatrix, inverseView);
    inverseView[0][3] = inverseView[1][3] = inverseView[2][3] = 0.0f;
    PSMTXTrans(translatedView, position.x, position.y, position.z);
    PSMTXConcat(viewMatrix, translatedView, translatedView);
    PSMTXConcat(translatedView, inverseView, translatedView);
    GXLoadPosMtxImm(translatedView, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(1);
    GXSetNumTexGens(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetZMode(GX_DISABLE, GX_ALWAYS, GX_DISABLE);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    alpha = updatesRemaining * 4;
    radius = 1.5f * (f32)(0x10 - updatesRemaining);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 0x42);
    for (index = 0; index < 0x21; ++index) {
        GXPosition3f32(radius * lbl_1_bss_288[index].x,
                       radius * lbl_1_bss_288[index].y, 0.0f);
        GXColor4u8(0xFF, 0xFF, 0xFF, alpha);
        GXPosition3f32(radius * lbl_1_bss_414[index].x,
                       radius * lbl_1_bss_414[index].y, 0.0f);
        GXColor4u8(0, 0xFF, 0xFF, 0);
    }
}

void ParticleListRoot::fn_1_A630(void (*visitor)(HookedModelCleanup *)) {
    fn_1_A898(begin(), end(), visitor);
}

void AlternateParticleListRoot::fn_1_A690(void (*visitor)(HookedModelCleanup *)) {
    fn_1_A838(begin(), end(), visitor);
}

void fn_1_A6F0(PositionedModelListRoot *list, void (*callback)(PositionedModelOwner *))
{
    unsigned char isEmpty = 0;
    if (list->link0 == list && list->link0 == list->link4)
        isEmpty = 1;

    if (!isEmpty) {
        unsigned char isDone;
        PositionedModelListRoot pending;
        list->moveAllTo(&pending);

        goto checkPending;
    processPending:
        {
            PositionedModelOwner *current = pending.front();
            current->unlink();
            current->insert4(list);
            callback(current);
        }
    checkPending:
        isDone = 0;
        if (pending.link0 == &pending && pending.link0 == pending.link4)
            isDone = 1;
        if (!isDone)
            goto processPending;
    }
}

// The three sentinels and their compiler-managed destructor registrations.
extern "C" {
ModelRoot lbl_1_bss_5AC;
AlternateParticleListRoot lbl_1_bss_5C0;
PositionedModelListRoot lbl_1_bss_5D4;
}

void fn_1_A838(Node *current,
                                        Node *sentinel,
                                        void (*callback)(HookedModelCleanup *))
{
    while (current != sentinel) {
        callback(static_cast<HookedModelCleanup *>(current));
        Node *next = current->link0;
        current = next;
    }
}

void fn_1_A898(Node *current,
                                        Node *sentinel,
                                        void (*callback)(HookedModelCleanup *))
{
    while (current != sentinel) {
        callback(static_cast<HookedModelCleanup *>(current));
        Node *next = current->link0;
        current = next;
    }
}
