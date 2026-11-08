#include "REL/m648DLL/particle_entry.h"
#include "REL/m648DLL/time_conversion.h"
#include "REL/m648DLL/sound.h"
/* The first object of the module. MSL sqrtf's statics come from this file, so they are the ones at the start of .rodata. */
#include <cmath>

extern "C" {
#include "dolphin/types.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/sprite.h"
#include "REL/framework/assertion.h"
}

#include "REL/m648DLL/list_object.h"
#include "REL/framework/minigame_singleton.h"
extern "C" {
#include <math.h>
}
#include <new>

extern "C" {
extern void *lbl_1_bss_34;
extern char lbl_1_data_0[];
extern char lbl_1_data_A[];
}

inline void RequestObject::update() {}

class FirstListObject;

class FirstListObject : public RequestObject, public MinigameSingleton<FirstListObject> {
public:
    FirstListObject();
    virtual ~FirstListObject();
    virtual void clearRequest(int request);
    virtual void update();
};

#include "REL/m648DLL/managers.h"

FirstListObject::FirstListObject() {}

void fn_1_618();
void fn_1_728();
void fn_1_5B4();

FirstListObject::~FirstListObject()
{
    fn_1_618();
}

void FirstListObject::update()
{
    fn_1_728();
}

void FirstListObject::clearRequest(int bit)
{
    state &= ~(1UL << bit);
    switch (bit) {
    case 0:
    case 1:
    case 3:
    case 4:
        break;
    case 2:
        fn_1_5B4();
        break;
    }
}

extern "C" {
}

extern "C" {
#include "dolphin/mtx.h"
}

#include "datadir_enum.h"

#include "REL/m648DLL/framework/linknode.h"

extern "C" {
#include "game/hu3d.h"
}

#include "REL/m648DLL/framework/model.h"

extern "C" {
#include "game/audio.h"
}

#include "REL/m648DLL/positioned_model_owner.h"

#include "REL/m648DLL/released_model.h"

#include "REL/m648DLL/camera_effect_owner.h"

extern "C" { void *__ct__4NodeFv(void *object); }

extern "C" { void insert4__4NodeFP4Node(void *object, void *member); }

extern "C" { void __ct__10OwnedModelFUi(void *object, u32 flags); }

typedef CameraEffectOwner318 MotionCamera8B4;

struct CameraCallbackList;
typedef CameraCallbackList DrawRoot618;
extern "C" { extern DrawRoot618 lbl_1_bss_18; }

CameraEffectOwner318::CameraEffectOwner318(Vec *position)
    : modelIdA(DATANUM(0x6b0000, 5)), modelIdB(DATANUM(0x6b0000, 6))
{
    s16 currentModelA;

    this->stopped = 0;
    this->complete = 0;
    this->angle = 0.0f;
    this->timer = 0;
    insert4__4NodeFP4Node(this, & lbl_1_bss_18);
    this->startPosition = * position;
    this->fade = 0.0f;
    currentModelA = this->modelIdA;
    fn_80026B8C(currentModelA, 4);
    fn_80026B64(currentModelA, 1);
    fn_800269E8(currentModelA, 1);
    fn_80026E44(currentModelA);
    fn_80026F9C(currentModelA);
    currentModelA = this->modelIdB;
    fn_80026B8C(currentModelA, 4);
    fn_80026B64(currentModelA, 1);
    fn_80026A44(currentModelA, 1);
    this->fn_1_8B4();
}

void operator delete(void *);

struct NodeOwner305B0_4B4 : Node { ~NodeOwner305B0_4B4(); };

NodeOwner305B0_4B4::~NodeOwner305B0_4B4() {}

class ModelOwner : public NodeOwner305B0_198 {
public:
    void (ModelOwner::*callback)();
    ReleasedModel454 model0, model1;
    ~ModelOwner();
};

extern const u8 lbl_1_data_540[];

extern const u8 lbl_1_data_594[];

extern "C" { void *__dt__4NodeFv(void *object, s16 destroy); }

extern "C" { void __dl__FPv(void *object); }

ModelOwner::~ModelOwner() {}

/* fn_1_5B4: mechanical copy of fn_1_C2EC in m603DLL (100% alike), names and constants adapted by copier.py; to be placed in its file. */
struct ListNode {
    ListNode *next;
    ListNode *previous;
};

struct ListEntry : ListNode {
    ListEntry *next_node() { return static_cast<ListEntry *>(next); }
};

struct ListRoot : ListNode {
    ListNode *first_node() { return next; }
    ListNode *end_node() { return this; }
    ListEntry *front() { return static_cast<ListEntry *>(first_node()); }
};

extern "C" { extern ListRoot lbl_1_bss_588; }

extern "C" { extern ListRoot lbl_1_bss_A50; }

extern "C" { void fn_1_A160(ListEntry *, int); }

extern "C" { void fn_1_C8C0(ListEntry *, int); }

extern "C" { void fn_1_C350(ListEntry *); }

extern "C" { extern DrawRoot618 lbl_1_bss_18; }

struct DrawNode618;
class CameraRotationController;
struct CameraCallbackList : Node {
    Node *first_callback_node() { return link0; }
    CameraRotationController *begin();
    Node *end() { return this; }
    bool empty() const { return link0 == this && link0 == link4; }
    Node *last_node() { return link4; }
    DrawNode618 *back() { return reinterpret_cast<DrawNode618 *>(last_node()); }
    ListNode *first_node() { return reinterpret_cast<ListNode *>(link0); }
    ListEntry *front() { return static_cast<ListEntry *>(first_node()); }
    ListNode *end_node() { return reinterpret_cast<ListNode *>(this); }
};

void fn_1_5B4()
{
    ListEntry *node = lbl_1_bss_18.front();
    ListNode *end = lbl_1_bss_18.end_node();
    for (; node != end; node = node->next_node()) {
        reinterpret_cast<MotionCamera8B4 *>(node)->fn_1_8B4();
    }
}

extern "C" void *__dt__4NodeFv(void *object, s16 mode);

typedef OwnedModel ReleasedModel618;

struct NodeOwner : ListNode {
    u8 ownerState[0x0C];
    ~NodeOwner() { __dt__4NodeFv(this, 0); }
};

struct DrawNode618 : NodeOwner {
    ReleasedModel618 firstModel;
    ReleasedModel618 secondModel;
    ~DrawNode618() {}
};

extern "C" DrawRoot618 lbl_1_bss_18;

void fn_1_618()
{
    while (!lbl_1_bss_18.empty()) {
        delete lbl_1_bss_18.back();
    }
}

typedef DrawRoot618 ListCallbackState;

class CameraRotationController;
typedef CameraRotationController ListCallbackNode;

template <class Visitor> void visitCameraCallbacks(DrawRoot618 *, Visitor);
void fn_1_7A4(ListCallbackNode *);

extern "C" {
extern ListCallbackState lbl_1_bss_18;
}

void fn_1_728()
{
    visitCameraCallbacks(&lbl_1_bss_18,
                           fn_1_7A4);
}
typedef struct CallbackDataBF0 {
    u32 words[3];
} CallbackDataBF0;

class ModelState {
public:
    u32 nodeData[2];
    CallbackDataBF0 callback;
    s16 modelId;
    void fn_1_758();
};

typedef struct MotionProgress {
    u32 nodeData[2];
    CallbackDataBF0 callback;
    s16 modelId;
    s32 audioHandle;
    u8 stopped;
    s8 complete;
    Vec startPosition;
    Vec position;
    Vec movement;
    Vec resetPosition;
    f32 value;
} MotionProgress;

void ModelState::fn_1_758()
{
    s16 currentModel;
    s16 modelId;
    modelId = (s16)((ModelState *)this)->modelId;
    currentModel = modelId;
    fn_80026A44(currentModel, (1 << 0));
}

extern "C" {

}

extern "C" {
extern void *fn_1_10208(s32 size);
}

extern "C" {

}

typedef struct CallbackDataE4C {
    s32 thisAdjustment;
    s32 vtableOffset;
    void (*callback)(MotionProgress *);
} CallbackDataE4C;

extern "C" {

}

float RandomFloat();

class CameraRotationController : public Node {
public:
    void (CameraRotationController::*callback)();
    Model models[2];
    u8 unobservedAfterModels[0x14];
    Vec position;
    u8 unobservedAfterPosition[0x0C];
    Vec cameraPosition;
    f32 scale;
    f32 angle;
    int count;
    void fn_1_7F0();
};

void fn_1_7A4(CameraRotationController *object)
{
    (object->*object->callback)();
    object->fn_1_7F0();
    object->count++;
}

void CameraRotationController::fn_1_7F0()
{
    f64 angle;
    Vec rotation;

    if (this->angle > 180.0f) {
        this->angle = this->angle - 360.0f;
    }
    angle = this->angle;
    rotation.x = (f32)angle;
    rotation.y = 0.0f;
    rotation.z = 0.0f;
    models[0].fn_1_33A3C(&position);
    models[0].fn_1_33ACC(&rotation);
    models[0].fn_1_33C78(scale);
    models[1].fn_1_33A3C(&cameraPosition);
}

inline void CameraEffectOwner318::fn_1_8B4()
{
    s16 currentModel;
    void (MotionCamera8B4::*callback)();

    this->position.x = this->startPosition.x;
    this->position.y = this->startPosition.y;
    this->position.z = this->startPosition.z;
    this->resetPosition.x = this->startPosition.x;
    this->resetPosition.y = this->startPosition.y;
    this->resetPosition.z = this->startPosition.z;
    this->position.y += 7.0f;
    currentModel = this->modelIdA;
    fn_80026A44(currentModel, (1 << 0));
    callback = &CameraEffectOwner318::fn_1_BA8;
    this->callback = callback;
}

/* Starts the object's flight toward the camera; CameraEffectOwner318::fn_1_BF0 moves it from then on. */
void fn_1_984(CameraEffectOwner318 *effect, u8 stopped)
{
    Vec cameraPosition;
    Vec cameraTarget;
    Vec cameraUp;
    void *memory;
    CameraEffectOwner318::Callback callback;

    effect->stopped = stopped;
    effect->complete = 0;
    fn_80027CD0(1, &cameraPosition, &cameraTarget, &cameraUp);
    effect->groundHeight = (s32)cameraPosition.z;
    PSVECSubtract(&cameraPosition, &effect->position, &effect->movement);
    PSVECScale(&effect->movement, &effect->movement, 0.03f);
    effect->movement.y += 5.44813888888889;
    effect->updateCount = 0;
    effect->audioHandle = HuAudFXEmiterPlay(MSM_SE_MG648_OBJECT_FLIGHT_LOOP, &effect->position);
    fn_1_EC10(1, MSM_SE_MG648_OBJECT_FLIGHT_START, &effect->position);
    memory = ::operator new(0x10);
    if (memory != 0) {
        memory = fn_1_99D4((ParticleModelEntry *)memory, &effect->position);
    }
    callback = &CameraEffectOwner318::fn_1_BF0;
    effect->callback = callback;
}

extern "C" { void *__nw__FUl(s32 size); }

/* Starts the object's bounce; CameraEffectOwner318::fn_1_E4C moves it from then on. */
void fn_1_AB8(CameraEffectOwner318 *work)
{
    Vec cameraPosition;
    Vec cameraTarget;
    Vec cameraUp;
    CameraEffectOwner318::Callback callback;

    work->complete = 0;
    fn_80027CD0(1, &cameraPosition, &cameraTarget, &cameraUp);
    work->groundHeight = (s32)cameraPosition.z;
    work->movement.x = 3.0f * (RandomFloat() - 0.5f);
    work->movement.y = 4.0861044f;
    work->movement.z = 1.6666666f;
    work->updatesRemaining = 0x78;
    fn_1_EC10(1, MSM_SE_MG648_OBJECT_BOUNCE_START, &work->position);
    callback = &CameraEffectOwner318::fn_1_E4C;
    work->callback = callback;
}

void CameraEffectOwner318::fn_1_BA8()
{
    if ((fade += 0.13333334f) > 1.0f) {
        fade = 1.0f;
        complete = 1;
    }
}

void CameraEffectOwner318::fn_1_BF0()
{
    this->movement.y = (f32)((f64)this->movement.y - 0.27240694444444447);
    PSVECAdd(&this->position, &this->movement, &this->position);
    if (this->position.z > (f32)this->groundHeight - 20.0f) {
        this->fade = 0.0f;
        if (this->stopped == 0) {
            this->fn_1_8B4();
        } else {
            CameraEffectOwner318::Callback callback =
                &CameraEffectOwner318::fn_1_E00;
            this->callback = callback;
        }
        HuAudFXStop(this->audioHandle);
    } else {
        HuAudFXEmiterUpDate(this->audioHandle, &this->position);
    }
    this->angle += 20.0f;
    this->updateCount += 1;
    if ((this->updateCount & 3) == 0) {
        new PositionedModelOwner(&this->position);
    }
}

void CameraEffectOwner318::fn_1_E00()
{
    s16 currentModel;
    s16 model;

    model = (s16)((ModelState *)this)->modelId;
    currentModel = model;
    fn_800269E8(currentModel, (1 << 0));
}

void CameraEffectOwner318::fn_1_E4C()
{
    movement.y -= 0.27240694444444447;
    PSVECAdd(&position, &movement, &position);
    if (movement.y < 0.0f && position.y < 3.0f) {
        position.y = 3.0f;
        movement.y *= -0.8;
    }
    if (--updatesRemaining < 0) {
        fn_1_8B4();
    } else {
        f32 progress = (f32)updatesRemaining;
        fade = 0.05f * progress;
        if (fade > 1.0f)
            fade = 1.0f;
    }
    angle += 10.0f;
}

inline CameraRotationController *DrawRoot618::begin()
{ return static_cast<CameraRotationController *>(first_callback_node()); }

template <class Visitor> void visitCameraNodes(Node *, Node *, Visitor);

template <class Visitor>
void visitCameraCallbacks(DrawRoot618 *list, Visitor visitor)
{
    visitCameraNodes(list->begin(), list->end(), visitor);
}

template <class Visitor>
void visitCameraNodes(Node *current, Node *sentinel, Visitor callback)
{
    while (current != sentinel) {
        callback(static_cast<CameraRotationController *>(current));
        Node *next = current->link0;
        current = next;
    }
}

template <class Time> Time secondsToFrames(Time seconds)
{
    return 60 * seconds;
}

extern "C" {
s32 lbl_1_bss_0 = secondsToFrames(2L);
s32 lbl_1_bss_4 = (s32)secondsToFrames(3.5);
s32 lbl_1_bss_8 = (s32)secondsToFrames(3.5);
DrawRoot618 lbl_1_bss_18;
}
