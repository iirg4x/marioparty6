#include "REL/m672DLL/resources.h"
#include "REL/m672DLL/particle_game_object.h"
#include "REL/m672DLL/ac4c_owner.h"
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/camera_frame.h"
class Resources;
#include "REL/m672DLL/random.h"
#include "dolphin/types.h"

extern "C" {
#include "dolphin/os.h"
}

extern "C" {
#include "math.h"
#include "game/animdata.h"
}

#include "dolphin/mtx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

extern "C" {
#include "game/hu3d.h"
}

extern "C" {
#include "game/charman.h"
}

#include "REL/m672DLL/framework/range.h"

#include "humath.h"

#include "game/main.h"

#include "game/object.h"

#include "game/gamework.h"

#include "game/wipe.h"

#include "game/mg/timer.h"

extern "C" {
#include "game/data.h"
}

#include "REL/m672DLL/position_bf64.h"

extern "C" {
#include "game/flag.h"
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

struct CallbackReceiver;

typedef void (CallbackReceiver::*MemberCallback)(int);

struct CallbackBase {
    virtual ~CallbackBase();
    virtual void run(int value) = 0;
};

struct CallbackPayload {
    MemberCallback callback;
    CallbackReceiver *receiver;
};

struct CallbackRecord : CallbackBase {
    MemberCallback callback;
    CallbackReceiver *receiver;
    virtual ~CallbackRecord();
    virtual void run(int value);
    CallbackRecord(const CallbackPayload& payload)
        : CallbackBase(), callback(payload.callback), receiver(payload.receiver) {}
};

struct CallbackOwner {
    void *vptr;
    void *tree;
};

void *operator new(unsigned long);

extern "C" void fn_1_20784(CallbackOwner *owner, CallbackBase *record, int key);

extern "C" { void addHandler__19EventHandlerManagerFP13EventCallbacki(CallbackOwner *owner, CallbackBase *record, int key); }

extern "C" {

extern void __dl__FPv(void *);

struct DestructibleObject { s32 *vtable; };
}

void operator delete(void *);

struct IntRange { int minimum,maximum,period,current; };

struct Transition { int frames,elapsed; float current,start,end; };

extern "C" { float fn_1_1990(Range *,float,float); }

extern "C" { float fn_1_1954(Range *,float); }

extern "C" { bool fn_1_12CC(Range *,float); }

extern "C" { void fn_1_1314(Range *,float,double); }

extern "C" { double fn_1_1428(double); }

extern "C" { int fn_1_1D7D0(IntRange *,int); }

extern "C" { void fn_1_3258(Transition *,float,float,int); }

extern "C" { void fn_1_3214(Transition *,float,int); }

struct RangeOwner {
    unsigned char unknown_00[80]; /* No code names this field. */
    Range range;
    float target,other;
    unsigned char unknown_68[13]; /* No code names this field. */
    bool done;
};

struct CompareOwner {
    unsigned char unknown_00[108]; /* No code names this field. */
    int first,second; };

struct CurveOwner {
    unsigned char unknown_00[32]; /* No code names this field. */
    float value; };

extern "C" { void fn_1_CC58(CurveOwner *,int); }

struct TwoWords { unsigned long first,second; };

struct Record44 {
    unsigned char unknown_00[44]; /* No code names this field. */
};

struct TransitionOwner {
    unsigned char unknown_00[32]; /* No code names this field. */
    Transition transition; };

extern "C" void fn_1_18208(TransitionOwner *);

struct Character;

extern "C" int fn_1_36794(Character *);

struct ModelMotion;

extern "C" { void fn_1_34C08(ModelMotion *,int,float,float,unsigned long); }

extern "C" { int fn_1_22D94(int); }

extern "C" void fn_1_1D900(Range *,float);

struct VectorPair { Vec first,second,third; };

struct FloatOwner {
    unsigned char unknown_00[24]; /* No code names this field. */
    float delta;
    unsigned char unknown_1C[20]; /* No code names this field. */
    float value; };

struct RenderConsumer;

struct RenderOwner {
    unsigned char unknown_00[288]; /* No code names this field. */
    RenderConsumer *consumer; RenderConsumer *get(){return consumer;} };

extern "C" { void fn_1_1F054(RenderConsumer *,Mtx); }

extern "C" { void fn_1_1F150(RenderConsumer *); }

extern "C" { void fn_1_1F1F0(RenderConsumer *); }

extern "C" { void fn_1_1F514(RenderConsumer *,Mtx); }

struct CameraParam { int first,second; float value; };

struct GlobalParam { int first,second; float firstValue,secondValue; };

extern "C" { extern CameraParam lbl_1_data_15C0[]; }

extern "C" { extern GlobalParam lbl_1_data_1680; }

typedef void (*Procedure)();

struct ThreadOwner {
    unsigned char unknown_00[8]; /* No code names this field. */
    OSThread thread; OSThread *get(){return &thread;} };

extern "C" unsigned char lbl_1_bss_BF0;

#include "REL/m672DLL/motion_owner_b2e4.h"

struct DeleteCallback : VirtualPrefix {
    virtual ~DeleteCallback();
    unsigned char unk_0C[40]; /* No code names this field. */
    int current,end;
    unsigned char unk3C[44]; /* No code names this field. */
    void(DeleteCallback::*callback)();
    void fn_1_4BB8(); void fn_1_6130();
};

extern "C" void Replace__5ModelFs(void *, short);

extern "C" void *__dt__19EventHandlerManagerFv(void *, short);

extern "C" void *__dt__10GameObjectFv(void *, short);

extern "C" void *lbl_1_data_43C[];

extern "C" { extern void *lbl_1_data_43C[]; }

extern "C" void __ct__10GameObjectFi(void *, int);

extern "C" void *__ct__10OwnedModelFUi(void *, unsigned int);

extern "C" void __ct__19EventHandlerManagerFi(void *, int);

extern "C" void fn_1_33CB0__5ModelFs(void *, int);

extern "C" void fn_1_33A3C__5ModelFP3Vec(void *, const float *);

extern "C" void fn_1_33950__5ModelFUl(void *, unsigned int);

template<class Receiver> void fn_1_BBD8(EventHandlerManager *, const AC4CPayload &, int);

static inline AC4CMemberFunction MakeAC4CMember(AC4CCallback fn) {
    return AC4CMemberFunction(fn);
}

static inline AC4CPayload BindAC4C(const AC4CMemberFunction &fn, AC4COwner *owner) {
    return AC4CPayload(fn, owner);
}

AC4COwner::AC4COwner(int firstMode, const float *position)
    : GameObject(4), OwnedModel((unsigned int)firstMode), state(0), nextState(0),
      time(0.0f), active(0), child(0)
{
    fn_1_33CB0(1);
    fn_1_33A3C((Vec *)position);
    fn_1_33950(((1 << 30) | (1 << 1)));
    fn_1_BBD8<AC4COwner>(&child,
             BindAC4C(MakeAC4CMember(&AC4COwner::fn_1_AF34), this), 1);
    fn_1_BBD8<AC4COwner>(&child,
             BindAC4C(MakeAC4CMember(&AC4COwner::fn_1_AF78), this), 3);
    setUpdate(&AC4COwner::fn_1_B050);
}

AC4COwner::~AC4COwner() {}

void AC4COwner::update()
{
    if (updateCallback) {
        (this->*updateCallback)();
    }
}

extern "C" {

typedef struct {
    Vec value;
} MotionVector;

extern const HuVecF lbl_1_data_60C;

extern const HuVecF lbl_1_data_600;

extern const HuVecF lbl_1_data_5F4;

void AC4COwner::fn_1_AF34(EventMessage *)
{
    {
        AC4CUpdate value = &AC4COwner::fn_1_B054;
        this->updateCallback = value;
    }
}

}

extern "C" {

extern s32 fn_1_11218(s32, s32, Vec *);

const MotionVector lbl_1_rodata_4A4 = {{0.0f, 0.0f, -2900.0f}};

void AC4COwner::fn_1_AF78(EventMessage *)
{
    MotionVector position;
    AC4CUpdate movement;
    HU3D_MODELID motionModelId;
    s16 *modelIdAddress = (s16 *)this;
    s16 *modelIdSlot;
    s16 modelId;

    if (modelIdAddress != 0) {
        modelIdAddress += 8;
    }
    modelIdSlot = modelIdAddress;
    motionModelId = (s16)(*modelIdSlot);
    modelId = motionModelId;
    fn_8002D730(modelId, 0.0f,
                          118.0f);
    position = lbl_1_rodata_4A4;
    fn_1_11218(1, 0x8A8, &position.value);
    movement = &AC4COwner::fn_1_B140;
    this->updateCallback = movement;
}

}

extern "C" {

/* Position and offset vectors separated by an unknown interval. */
typedef struct Position {
    HuVecF position;
    unsigned char unk_0C[12]; /* No code names this field. */
    HuVecF offset;
} Position;

extern s32 lbl_1_data_F18;

extern float lbl_1_bss_1BC;

extern unsigned char lbl_1_bss_218;

extern unsigned char lbl_1_bss_219;

extern s8 lbl_1_data_2758;

s32 fn_1_367CC(s32 *player);

void AC4COwner::fn_1_B050()
{
}

}

extern "C" {

extern void fn_1_33988__5ModelFUl(void *, u32);

}

void AC4COwner::fn_1_B054()
{
    s16 *modelIdAddress;
    s16 *modelIdSlot;
    s16 modelId;
    s16 motionModelId;

    this->time += 0.016666666666666666;
    if (this->time < 2.0f) {
        return;
    }

    fn_1_33988(((1 << 30) | (1 << 1)));
    modelIdAddress = (s16 *)this;
    if (modelIdAddress != 0) {
        modelIdAddress += 8;
    }
    modelIdSlot = modelIdAddress;
    modelId = (s16)(*modelIdSlot);
    motionModelId = modelId;
    fn_8002D730(motionModelId, 0.0f,
                          59.0f);
    {
        AC4CUpdate callback;
        callback = &AC4COwner::fn_1_B050;
        this->updateCallback = callback;
    }
}
extern "C" {

typedef struct {
    Vec value;
} VecStorageB140;

extern void *__nw__FUl(u32 size);

extern void __msl_assertion_failed(const char *expression, const char *file, int line);

const s32 lbl_1_rodata_4C8[3] = {40, 53, 58};

const VecStorageB140 lbl_1_rodata_4D4 = {{0.0f, 0.0f, -2900.0f}};

}

void AC4COwner::fn_1_B140()
{
    VecStorageB140 movement;
    Resources *resourceRoot;
    VecStorageB140 position;
    AC4CUpdate callback;

    this->state += 1;
    if (this->state >= lbl_1_rodata_4C8[this->nextState]) {
        if (this->active == 0) {
            movement = lbl_1_rodata_4D4;
            fn_1_11218(1, 0x8A9, &movement.value);
            this->active = 1;
        }

        resourceRoot = &Resources::getChecked();
        position.value.x = 0.0f;
        position.value.y = 0.0f;
        position.value.z = -2900.0f;
        new ParticleGameObject(resourceRoot->animation(0), &position.value);

        this->nextState += 1;
        if (this->nextState >= 3) {
            callback = &AC4COwner::fn_1_B050;
            this->updateCallback = callback;
        }
    }
}
extern "C" {

extern void *__nw__FUl(unsigned long);

extern void *__ct__15RisingParticlesFP10AnimData_siiPC13WorldPosition(void *, int, int, int, const PositionB2E4 *);

}

struct ChildSetup {
    s32 resourceId;
    PositionB2E4 childPosition;
};

extern "C" { extern const u8 lbl_1_data_42C[]; }

static inline void ReadChildResourceB2E4(s32 *out, void *resources,
                                          void ***slotOut)
{
    *out = (int)((Resources *)resources)->animations.at(0);
}

static inline void *CreateMotionB2E4(s32 type)
{
    void *motion = __nw__FUl(2);
    if (motion != 0) {
        motion = __ct__10OwnedModelFUi(motion, type);
    }
    return motion;
}

static inline int ChildResourceB2E4(void *resources)
{
    return (int)((Resources *)resources)->animations.at(0);
}

static inline void *CreateChildB2E4(void *resources, PositionB2E4 *position)
{
    void *child = __nw__FUl(0x44);
    if (child != 0) {
        PositionB2E4 childPosition = *position;
        PositionB2E4 *positionValue = &childPosition;
        child = __ct__15RisingParticlesFP10AnimData_siiPC13WorldPosition(child, ChildResourceB2E4(resources),
                         0x1E, 5, positionValue);
    }
    return child;
}

HangingCage::HangingCage(s32 type, const PositionB2E4 *position) : GameObject(0)
{
    void **resourceSlot;
    ChildSetup setup;
    void *resourceRoot;
    void *motionResult;
    void *motion;
    void *child;
    VirtualPointee *newChild;
    f32 angle;

    this->position = *position;

    motion = __nw__FUl(2);
    if (motion != 0) {
        motion = __ct__10OwnedModelFUi(motion, type);
    }
    motionResult = motion;
    this->motion.value = (OwnedModel *)motionResult;
    this->secondModel.value = 0;
    this->child.value = 0;
    this->angle = 0.0f;
    this->rotation = 0.0f;
    this->frame = 0;
    this->fade = 0.0f;
    void *motionForFlags = this->motion.value;
    fn_1_33CB0__5ModelFs(motionForFlags, 4);
    void *motionForPosition = this->motion.value;
    fn_1_33A3C__5ModelFP3Vec(motionForPosition, &position->value.value.x);
    void *motionForState = this->motion.value;
    void *manager;
    void *resourceList;
    fn_1_33950__5ModelFUl(motionForState, ((1 << 30) | (1 << 1)));

    Resources::getChecked();
    manager = Resources::self;
    resourceList = manager;
    resourceRoot = resourceList;
    child = __nw__FUl(0x44);
    if (child != 0) {
        setup.childPosition = *position;
        PositionB2E4 *positionValue = &setup.childPosition;
        ReadChildResourceB2E4(&setup.resourceId, resourceRoot, &resourceSlot);
        child = __ct__15RisingParticlesFP10AnimData_siiPC13WorldPosition(
            child, setup.resourceId, 0x1E, 5, positionValue);
    }
    newChild = (VirtualPointee *)child;

    this->child = newChild;

    for (;;) {
        angle = RandomFloat(0.0f, 359.9f);
        this->angle = angle;
        if ((angle < 45.0f) ||
            ((angle >= 135.0f) && (angle <= 225.0f)) ||
            (angle > 315.0f)) {
            break;
        }
    }
    if ((angle >= 135.0f) && (angle <= 225.0f)) {
        this->rotation = 180.0f;
    }
}

extern "C" unsigned char lbl_1_bss_218, lbl_1_bss_219;

inline
void OwnedModel::apply_defaults()
{
    if(lbl_1_bss_218) fn_1_33950((1 << 14));
    if(lbl_1_bss_219) {
        fn_1_33950(((1 << 30) | (1 << 6)));
        fn_1_33950(((1 << 30) | (1 << 0)));
    }
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

struct ModelMotionEntry { int dataNum; short motionId; };

struct ModelMotion : Model {
    struct Sequence {
        unsigned long unknown00, count;
        ModelMotionEntry *storage;
    } motions;
    int currentDataNum;
    short id() const { return modelId; }
};

extern "C" short fn_1_34DA4(ModelMotion *, int);

extern "C" { extern char lbl_1_data_26CA[]; }

extern "C" { extern char lbl_1_data_26C0[]; }

struct ModelObject : Model {
    HSF_OBJECT *lookup(char *name) {
        HSF_OBJECT *object=fn_80026BF4((short)modelId,name);
        object ? (void)0 : __msl_assertion_failed(lbl_1_data_26CA,lbl_1_data_26C0,230);
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

extern "C" void fn_1_25674(CameraFrame *,Vec *,Vec *,Vec *);

struct CameraPose { Vec position; Quaternion rotation; };

extern "C" void fn_1_26F34(const CameraFrame *,CameraPose *);

struct DirectionView {
    unsigned char unknown00[32]; /* No code names this field. */
    Vec from,to,direction;
};

struct ShadowVectors {
    unsigned char unknown00[44]; /* No code names this field. */
    short cameraBit;
    Vec direction;
    void fn_1_39104(float);
};

struct WindowMessageView {
    unsigned char unknown00[12]; /* No code names this field. */
    short windowId;
    void fn_1_196F0(unsigned long);
};

struct PlayerHeightView {
    int playerNo;
    float fn_1_37688();
};

ChildPointer::~ChildPointer() { delete value; }

extern "C" {

extern void __dl__FPv(void *value);
}

Owner::~Owner() { delete value; }

struct ChainEffect {
    unsigned char unknown00[0x30]; /* No code names this field. */
    float angle;
};

struct ChainPosition {
    Point3d value;
};

class ChainTarget {
public:
    unsigned char unknown_00[0x18]; /* No code names this field. */
    Point3d position;
    void setPosition(ChainPosition value) {
        position.x = value.value.x;
        position.y = value.value.y;
        position.z = value.value.z;
    }
};

extern "C" ChainEffect *fn_1_33F28__5ModelFPc(void *, const char *);

static inline float SinRadiansB7B8(float radians)
{
    return (float)sin(radians);
}

extern "C" { void *__dt__10GameObjectFv(void *, short); }

HangingCage::~HangingCage() {}

void HangingCage::update()
{
    ChainPosition origin;
    Mtx firstMatrix;
    Mtx secondMatrix;
    Mtx translation;
    ChainEffect *firstChain;
    ChainEffect *secondChain;

    ++this->frame;
    this->fade = (float)(1.0 - (0.016666666666666666 * (0.027f * (float)this->frame)));
    if (this->fade < 0.0f) {
        this->fade = 0.0f;
    }
    firstChain = fn_1_33F28__5ModelFPc(this->getModel(), "kusari");
    this->angle += 2.0;
    if (this->angle > 360.0f) {
        this->angle -= 360.0f;
    }
    firstChain->angle = this->fade * (6.0f * SinRadiansB7B8(MTXDegToRad(this->angle)));
    secondChain = fn_1_33F28__5ModelFPc(this->getModel(), "ori_top01");
    if ((this->angle >= 45.0f && this->angle < 135.0f) ||
        (this->angle > 225.0f && this->angle <= 315.0f)) {
        this->rotation += 4.0;
        if (this->rotation > 360.0f) {
            this->rotation -= 360.0f;
        }
        secondChain->angle = this->fade * (3.0f * SinRadiansB7B8(MTXDegToRad(this->rotation)));
    }
    origin.value.x = 0.0f;
    origin.value.y = 0.0f;
    origin.value.z = 0.0f;
    PSMTXRotRad(firstMatrix, 'z', MTXDegToRad(firstChain->angle));
    PSMTXTrans(translation, 0.0f, -1794.0f, 0.0f);
    PSMTXConcat(firstMatrix, translation, firstMatrix);
    PSMTXRotRad(secondMatrix, 'z', MTXDegToRad(secondChain->angle));
    PSMTXTrans(translation, 0.0f, -130.0f, 0.0f);
    PSMTXConcat(secondMatrix, translation, secondMatrix);
    PSMTXConcat(firstMatrix, secondMatrix, firstMatrix);
    PSMTXMultVec(firstMatrix, &origin.value, &origin.value);
    PSVECAdd(&origin.value, &this->position.value.value, &origin.value);
    this->getTarget()->setPosition(origin);
}

template<class Receiver>
void fn_1_BBD8(EventHandlerManager *owner, const AC4CPayload &payload, int key) {
    owner->addHandler(new MemberCallbackRecord<Receiver>(payload), key);
}
