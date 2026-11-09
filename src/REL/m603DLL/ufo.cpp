#include "REL/m603DLL/singleton_32930.h"
#include "REL/m603DLL/ufo.h"
extern "C" {
#include "game/audio.h"
#include "REL/m603DLL.h"
}
#include "dolphin/gx.h"
#include "humath.h"

template <class Time> Time secondsToFrames(Time seconds);

#include <cmath>
#include "REL/m603DLL/score_owner.h"
#include "dolphin/types.h"
#include "datadir_enum.h"
extern "C" {
#include "game/hu3d.h"
#include "dolphin/mtx.h"
#include "game/hsfex.h"
}
#include "REL/m603DLL/callback_owner.h"
#include "REL/m603DLL/camera_owner.h"
#include "REL/m603DLL/random.h"

typedef Vec FloatTripletB520;

extern "C" {


extern const u8 lbl_1_data_A4C[];
extern const u8 lbl_1_data_EC[];
extern const char lbl_1_data_950[];
extern const char lbl_1_data_95A[];
const s32 lbl_1_rodata_598[3] = {DATANUM(0x3e0000, 17), DATANUM(0x3e0000, 18), DATANUM(0x3e0000, 19)};
extern void *__nw__FUl(unsigned long);
class CameraModelOwner;
}
void fn_1_C710(CameraModelOwner *);

struct CameraModelNode : Node {
    ~CameraModelNode();
};

struct CameraModelOwner : CameraModelNode {
    CameraModelOwner(const OwnedModel *,unsigned int,s32,s32);
    ~CameraModelOwner();
    void fn_1_CD7C(Mtx);
    void fn_1_CDD4(Mtx);
    void fn_1_C350();
    s16 primary_model() const { return modelId; }
    s16 secondary_model() const { return motionId; }
    typedef void (CameraModelOwner::*Callback)();
    Callback callbackState;
    void fn_1_C750();
    void fn_1_C8C0(s32);
    void fn_1_C990();
    void fn_1_C994();
    void fn_1_CA14(s32);
    void fn_1_CB64();
    s16 model_id() const { return motionId; }
    HU3D_MODELID first_model() const { return modelId; }
    HU3D_MODELID second_model() const { return motionId; }
    OwnedModel modelId;
    OwnedModel motionId;
    u8 state18;
    s32 cameraIndex;
    Vec position;
    s32 state2C;
    s32 cameraGroup;
    u8 groupFlags[4];
    f32 motionSpeed;
    f32 motionLength;
    f32 playbackSpeed;
    s32 countdown;
    s16 hookModelId;
    u32 drawCount;
    f32 depth;
    u8 state54;
    s32 callbackIndex;
    CameraModelOwner *next_node() { return static_cast<CameraModelOwner *>(link0); }
};

struct CameraModelList : Node {
 ~CameraModelList() {}
 Node *first_node() { return link0; }
 Node *last_node() { return link4; }
 Node *end_node() { return this; }
 Node *end() { return this; }
 CameraModelOwner *begin() { return static_cast<CameraModelOwner *>(first_node()); }
 CameraModelOwner *front() { return static_cast<CameraModelOwner *>(first_node()); }
 CameraModelOwner *back() { return static_cast<CameraModelOwner *>(last_node()); }
 bool empty() const { return link0 == this && link0 == link4; }
};
extern "C" {
long lbl_1_bss_590 = secondsToFrames(2L);
long lbl_1_bss_594 = (long)secondsToFrames(3.5);
long lbl_1_bss_598 = (long)secondsToFrames(3.5);
FloatTripletB520 lbl_1_bss_59C[0x21];
FloatTripletB520 lbl_1_bss_728[0x21];
FloatTripletB520 lbl_1_bss_8B4[0x21];
u8 lbl_1_bss_A40;
CameraModelList lbl_1_bss_A50;
}
template <class Time> inline Time secondsToFrames(Time seconds)
{
    return 60 * seconds;
}

static inline void CameraModelCreate(CameraModelPool *object, s32 modelIndex, s32 cameraGroup)
{
    new CameraModelOwner(&object->modelId, lbl_1_rodata_598[modelIndex], modelIndex, cameraGroup);
}

CameraModelPool::CameraModelPool()
    : modelId(DATANUM(0x3e0000, 16))
{
    f32 radians;
    f32 angle;
    s32 cameraGroup;
    s32 modelIndex;
    s32 i;

    s16 modelHandle = static_cast<s16>(modelId);
    fn_800269E8(modelHandle, (1 << 0));

    for (cameraGroup = 0; cameraGroup < 4; cameraGroup++) {
        for (modelIndex = 0; modelIndex < 3; modelIndex++) {
            CameraModelCreate(this, modelIndex, cameraGroup);
        }
    }

    for (i = 0; i < 0x21; i++) {
        angle = 360.0f * ((f32)(i & 0x1F) / 32.0f)
                - 180.0f;
        angle += 90.0f;
        if (angle > 180.0f) {
            angle -= 360.0f;
        }
        radians = (f32)((M_PI * (f64)angle) / 180.0);
        lbl_1_bss_728[i].x = (f32)(2.0 *
            (70.0 * std::cos((f64)radians)));
        lbl_1_bss_728[i].y = (f32)(2.0 *
            (70.0 * std::sin((f64)radians)));
        lbl_1_bss_728[i].z = 0.0f;
        lbl_1_bss_59C[i].x = (f32)(2.0 *
            (80.0 * std::cos((f64)radians)));
        lbl_1_bss_59C[i].y = (f32)(2.0 *
            (80.0 * std::sin((f64)radians)));
        lbl_1_bss_59C[i].z = 0.0f;
        lbl_1_bss_8B4[i].x = (f32)(50.0 *
            std::cos((f64)radians));
        lbl_1_bss_8B4[i].y = 0.0f;
        lbl_1_bss_8B4[i].z = (f32)(50.0 *
            std::sin((f64)radians));
    }
    lbl_1_bss_A40 = 0;
}

void fn_1_BF18();
void fn_1_C2EC();
void fn_1_C0FC();
void fn_1_C04C();

CameraModelPool::~CameraModelPool()
{
    fn_1_BF18();
}

void CameraModelPool::extra()
{
    if ((s32)(RandomUnsigned() & 0x1FFUL) == 0) fn_1_C0FC();
    fn_1_C04C();
}

void CameraModelPool::update(unsigned long bit)
{
    state &= ~(1UL << bit);
    switch (bit) {
    case 0:
    case 1:
    case 2:
        break;
    case 3:
        fn_1_C2EC();
        break;
    case 4:
        break;
    }
}

void fn_1_BB5C()
{
    lbl_1_bss_A40 = 1;
}





extern "C" {

char *lbl_1_data_9A4[3] = {"mapmodel10-ufo_act", "mapmodel12-ufo_act_b",
                              "mapmodel14-ufo_act_c"};

const u32 lbl_1_rodata_5F8[4] = {1, 2, 4, 8};
extern void __ct__4NodeFv(void *);
extern void insert4__4NodeFP4Node(void *, void *);

extern void __ct__10OwnedModelFUi(void *, s32);
}

extern void fn_1_CD34(HU3D_MODEL *, Mtx *);
extern "C" {

CameraModelOwner::CameraModelOwner(const OwnedModel *sourceModel, unsigned int motionData, s32 cameraIndex, s32 cameraGroup)
    : modelId(sourceModel), motionId(motionData)
{
    s16 primaryModelId;
    s16 foregroundModelId;
    s16 motionIdValue;
    CameraModelOwner::Callback callbackState;
    HU3D_MODEL *hookModel;

    this->state18 = 0;
    this->cameraIndex = cameraIndex;
    this->cameraGroup = cameraGroup;
    this->groupFlags[0] = 0;
    this->groupFlags[1] = 0;
    this->groupFlags[2] = 0;
    this->groupFlags[3] = 0;
    this->depth = 0.0f;
    this->state54 = 0;
    this->callbackIndex = -1;
    insert4(&lbl_1_bss_A50);

    primaryModelId = this->modelId;
    fn_80026A44(primaryModelId, 1);
    fn_80026B8C(primaryModelId, 0);
    fn_80026B64(primaryModelId, lbl_1_rodata_5F8[cameraGroup]);
    fn_800269E8(primaryModelId, 0x4000);

    foregroundModelId = this->motionId;
    fn_800269E8(foregroundModelId, 1);
    fn_80026B64(foregroundModelId, lbl_1_rodata_5F8[cameraGroup]);

    fn_8002741C(this->motionId, lbl_1_data_9A4[cameraIndex], this->modelId);
    motionId.fn_1_34228(lbl_1_data_9A4[this->cameraIndex]);
    motionIdValue = fn_8002D3C4(foregroundModelId);
    fn_8002C95C(foregroundModelId, motionIdValue);
    fn_800269E8(foregroundModelId, ((1 << 30) | (1 << 1)));

    this->state2C = 0;
    this->position.x = this->position.y = this->position.z = 0.0f;
    this->hookModelId = fn_800260F4(fn_1_CD34);
    hookModel = &lbl_802C01C8[this->hookModelId];
    hookModel->hookData = this;
    fn_80026B8C(this->hookModelId, 6);
    fn_80026B64(this->hookModelId, lbl_1_rodata_5F8[cameraGroup]);

    this->modelId.fn_1_33A3C(&this->position);
    callbackState = &CameraModelOwner::fn_1_C994;
    this->callbackState = callbackState;
}

}


CameraModelNode::~CameraModelNode() {}

CameraModelOwner::~CameraModelOwner()
{
    if (callbackIndex >= 0) {
        HuAudFXStop(callbackIndex);
        callbackIndex = -1;
    }
    fn_800262D0(hookModelId);
}

void fn_1_BF18()
{
    while (!lbl_1_bss_A50.empty()) delete lbl_1_bss_A50.back();
}

typedef void (*CameraVisitor)(CameraModelOwner *);
void fn_1_D270(CameraModelList *, CameraVisitor);
void fn_1_C04C()
{
    fn_1_D270(&lbl_1_bss_A50, fn_1_C710);
}

int fn_1_C07C(CameraModelOwner *current)
{
    int candidate;
    int step;
    for (candidate = 0; candidate < 4; candidate++) {
        if (!current->groupFlags[1] && !current->groupFlags[0]) return 1;
        if (candidate == 3) break;
        for (step = 0; step < 3; step++) current = current->next_node();
    }
    return 0;
}

void fn_1_C0FC()
{
    CameraModelOwner *entry;
    Node *end;
    s32 gameMode;
    s32 startIndex;
    s32 step;
    s32 firstStep;
    s32 secondStep;
    s32 thirdStep;
    gameMode = fn_1_1AB30();
    if (gameMode == 2) {
        entry = lbl_1_bss_A50.front();
        end = lbl_1_bss_A50.end_node();
        startIndex = ((u16)RandomUnsigned() * 3) >> 16;
        for (step = 0; step != startIndex; step++) entry = entry->next_node();
        if ((u8)fn_1_C07C(entry) != 0) {
            entry->fn_1_CA14(0);
            for (firstStep = 0; firstStep < 3; firstStep++) entry = entry->next_node();
            entry->fn_1_CA14(1);
            for (secondStep = 0; secondStep < 3; secondStep++) entry = entry->next_node();
            entry->fn_1_CA14(2);
            for (thirdStep = 0; thirdStep < 3; thirdStep++) entry = entry->next_node();
            entry->fn_1_CA14(3);
        }
    }
}

void fn_1_C280(int request)
{
    CameraModelOwner *node = lbl_1_bss_A50.front();
    Node *end = lbl_1_bss_A50.end_node();
    for (; node != end; node = node->next_node()) node->fn_1_C8C0(request);
}

void fn_1_C2EC()
{
    CameraModelOwner *node = lbl_1_bss_A50.front();
    Node *end = lbl_1_bss_A50.end_node();
    for (; node != end; node = node->next_node()) node->fn_1_C350();
}

void CameraModelOwner::fn_1_C350()
{
    if (callbackIndex >= 0) {
        HuAudFXStop(callbackIndex);
        callbackIndex = -1;
    }
}

extern s32 fn_1_3340(Vec *position, s32 cameraIndex, u8 expandedBounds);

s32 fn_1_C394(const Vec *center, s32 cameraIndex)
{
    Vec position;
    s32 angleIndex;
    f32 angleDegrees;
    f32 angle;

    angleIndex = 0;
    while (angleIndex < 8) {
        angleDegrees = (f32)(angleIndex * 0x168) / 8.0f - 180.0f;
        angle = (f32)((M_PI * (f64)angleDegrees) / 180.0);
        position.x = (f32)((f64)center->x + 150.0 * std::cos((f64)angle));
        position.y = (f32)((f64)center->y + 150.0 * std::sin((f64)angle));
        position.z = center->z;
        if ((u8)fn_1_3340(&position, cameraIndex, 1)) {
            return 1;
        }
        angleIndex++;
    }
    return 0;
}

s32 fn_1_C4E8(s32 cameraIndex, u8 markEntries, s32 currentValue)
{
    CameraModelOwner *node;
    Node *end;
    s32 matchedCount;

    node = lbl_1_bss_A50.front();
    end = &lbl_1_bss_A50;
    matchedCount = 0;

    while (node != end) {
        if (node->state18 != 0 && node->cameraGroup == cameraIndex && node->groupFlags[2] != 0) {
            if ((u8)fn_1_C394(&node->position, cameraIndex) != 0) {
                if (markEntries != 0) {
                    node->groupFlags[3] = 1;
                    node->countdown = (s32)((f32)(-(currentValue + 1)) * secondsToFrames(0.2f));
                }
                matchedCount += 3;
                currentValue++;
            }
        }
        node = node->next_node();
    }
    return matchedCount;
}

void fn_1_C710(CameraModelOwner *self)
{
    (self->*self->callbackState)();
    self->fn_1_C750();
}

void CameraModelOwner::fn_1_C750()
{
    s16 modelArgument;
    s16 model;
    Vec rotation;
    Point3d objectPosition;
    Vec translation;

    model = (s32)this->motionId.modelId;
    modelArgument = model;
    Hu3DModelObjPosGet(modelArgument, lbl_1_data_9A4[this->cameraIndex], &objectPosition);
    this->position.x = objectPosition.x;
    this->position.y = objectPosition.y;
    this->position.z = objectPosition.z;
    this->position.y += 50.0f;
    if (this->groupFlags[1] == 0) {
        this->depth += 3.0f;
        if (this->depth > 180.0f) {
            this->depth -= 360.0f;
        }
    }

    f64 angle = this->depth;
    rotation.x = 0.0f;
    rotation.y = angle;
    rotation.z = 0.0f;
    translation.x = 0.0f;
    translation.y = 0.0f;
    translation.z = 0.0f;
    modelId.fn_1_33ACC(&rotation);
    modelId.fn_1_33A3C(&translation);
}
void CameraModelOwner::fn_1_C8C0(s32 playerIndex)
{
    if (this->cameraGroup == playerIndex && this->groupFlags[1] == 0) {
        s16 modelId;

        if (this->callbackIndex >= 0) {
            HuAudFXStop(this->callbackIndex);
            this->callbackIndex = -1;
        }
        this->groupFlags[1] = 1;
        modelId = this->model_id();
        fn_800269E8(modelId, ((1 << 30) | (1 << 1)));
        {
            CameraModelOwner::Callback callback = &CameraModelOwner::fn_1_C990;
            this->callbackState = callback;
        }
    }
}
void CameraModelOwner::fn_1_C990() {}
void CameraModelOwner::fn_1_C994()
{
    HU3D_MODELID first = first_model();
    fn_800269E8(first, (1 << 0));
    HU3D_MODELID second = second_model();
    fn_800269E8(second, (1 << 0));
    state2C = 0;
    groupFlags[2] = 0;
    state18 = 0;
    groupFlags[0] = 0;
}
extern "C" const s32 lbl_1_rodata_624[4] = {0x634,0x635,0x636,0x637};

void CameraModelOwner::fn_1_CA14(s32 motionIndex)
{
    if (this->groupFlags[1] == 0) {
        this->callbackIndex = HuAudFXPlay(lbl_1_rodata_624[motionIndex]);
        s16 modelId = this->model_id();
        fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
        fn_80026A44(modelId, ((1 << 30) | (1 << 0)));
        fn_8002D414(modelId, 0.0f);
        this->motionSpeed = 0.8f + 1.2f * RandomFloat();
        this->motionLength = fn_8002D530(modelId);
        this->playbackSpeed = this->motionSpeed / 60.0f;
        fn_8002D86C(modelId, this->playbackSpeed);
        this->state2C = 0;
        this->state18 = 0;
        this->groupFlags[0] = 1;
        CameraModelOwner::Callback callback = &CameraModelOwner::fn_1_CB64;
        this->callbackState = callback;
    }
}
f32 lbl_1_data_9D4[3][8] = {
 {0.0f,0.175f,0.405f,0.6f,0.62f,0.703f,0.79f,1.0f},
 {0.0f,0.123f,0.25f,0.715f,0.8f,1.0f,1.0f,1.0f},
 {0.0f,0.06f,0.254f,0.66f,0.806f,0.886f,0.94f,1.0f}
};
void CameraModelOwner::fn_1_CB64()
{
    f32 progress;
    s32 frame;
    s16 modelId;
    Callback callback;

    groupFlags[2] = 0;
    progress = (f32)state2C * playbackSpeed / motionLength;
    for (frame = 0; frame < 7; frame++) {
        if (lbl_1_data_9D4[cameraIndex][frame] <= progress &&
            lbl_1_data_9D4[cameraIndex][frame + 1] > progress) {
            if ((frame & 1) != 0) {
                groupFlags[2] = 1;
            }
            break;
        }
    }

    if (state2C > 10) {
        modelId = model_id();
        if (fn_8002D75C(modelId) == 1) {
            if (callbackIndex >= 0) {
                HuAudFXStop(callbackIndex);
                callbackIndex = -1;
            }
            callback = &CameraModelOwner::fn_1_C994;
            this->callbackState = callback;
            return;
        }
    }
    {
        HU3D_MODELID firstModel = first_model();
        fn_80026A44(firstModel, 1);
        HU3D_MODELID secondModel = second_model();
        fn_80026A44(secondModel, 1);
        state18 = 1;
        groupFlags[0] = 1;
        state2C++;
    }
}

void fn_1_CD34(HU3D_MODEL *model, Mtx *matrix)
{
    fn_8002472C();
    void *hookData = model->hookData;
    static_cast<CameraModelOwner *>(hookData)->fn_1_CD7C(*matrix);
}

void CameraModelOwner::fn_1_CD7C(Mtx matrix)
{
    if (lbl_1_bss_A40 && groupFlags[3]) fn_1_CDD4(matrix);
}

void CameraModelOwner::fn_1_CDD4(Mtx matrix)
{
    Mtx inverse;
    Mtx transform;
    s32 frame;
    s32 frameIndex;
    s32 vertex;
    f32 scale;
    s32 colorOffset;
    ScoreDisplay *singleton;

    if (++countdown < 0) {
        return;
    }
    if (countdown == 0) {
        singleton = (&ScoreDisplay::getChecked());
        singleton->soundPending = 1;
    }

    if ((f64)countdown > 12.0) {
        if (state54 == 0) {
            state54 = 1;
            (&ScoreDisplay::getChecked())->fn_1_83CC(cameraGroup, 3);
        }
        countdown = 12;
    }

    frame = (s32)(255.0f *
                  ((f32)countdown / 12.0f));
    scale = (f32)((f64)(f32)(383 - frame) / 128.0);
    PSMTXInverse(matrix, inverse);
    inverse[0][3] = inverse[1][3] = inverse[2][3] = 0.0f;
    PSMTXTrans(transform, position.x, position.y,
               position.z);
    PSMTXConcat(matrix, transform, transform);
    PSMTXConcat(transform, inverse, transform);
    GXLoadPosMtxImm(transform, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(1);
    GXSetNumTexGens(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 0x42);
    colorOffset = (drawCount++ & 7) << 5;
    vertex = 0;
    while (vertex < 33) {
        GXPosition3f32(scale * lbl_1_bss_59C[vertex].x,
                       scale * lbl_1_bss_59C[vertex].y, 0.0f);
        GXWGFifo.u8 = 0xFF;
        GXWGFifo.u8 = (u8)colorOffset;
        GXWGFifo.u8 = 0;
        GXWGFifo.u8 = (u8)frame;
        GXPosition3f32(scale * lbl_1_bss_728[vertex].x,
                       scale * lbl_1_bss_728[vertex].y, 0.0f);
        GXWGFifo.u8 = 0xFF;
        GXWGFifo.u8 = (u8)colorOffset;
        GXWGFifo.u8 = 0;
        GXWGFifo.u8 = (u8)frame;
        vertex++;
    }
}


void fn_1_D2D0(Node *, Node *, CameraVisitor);
void fn_1_D270(CameraModelList *list, CameraVisitor visitor)
{
    fn_1_D2D0(list->begin(), list->end(), visitor);
}
void fn_1_D2D0(Node *current, Node *sentinel, CameraVisitor callback)
{
    while (current != sentinel) {
        callback(static_cast<CameraModelOwner *>(current));
        Node *next = current->link0;
        current = next;
    }
}
