#include "REL/m603DLL/draw_owner.h"
#include "REL/m603DLL/camera_group.h"
#include "REL/m603DLL/ufo.h"
extern "C" {
#include "game/audio.h"
#include "REL/m603DLL.h"
}
#include "dolphin/gx.h"
#include "humath.h"

#include "REL/framework/frame_time_inline.h"

#include <cmath>
#include "REL/m603DLL/score_owner.h"
#include "dolphin/types.h"
#include "datadir_enum.h"
extern "C" {
#include "game/hu3d.h"
#include "dolphin/mtx.h"
#include "game/hsfex.h"
}
#include "REL/framework/list_object.h"
#include "REL/m603DLL/camera_owner.h"
#include "REL/framework/random.h"

extern "C" {

const s32 lbl_1_rodata_598[3] = {DATANUM(0x3e0000, 17), DATANUM(0x3e0000, 18), DATANUM(0x3e0000, 19)};
}

/* Animate, score and draw one UFO for a camera. */
struct CameraModelOwner : ListNode<CameraModelOwner> {
    CameraModelOwner(const OwnedModel *,unsigned int,s32,s32);
    ~CameraModelOwner();
    void fn_1_CD7C(Mtx);
    void fn_1_CDD4(Mtx);
    void stopSound();
    typedef void (CameraModelOwner::*Callback)();
    Callback callbackState;
    void fn_1_C750();
    void fn_1_C8C0(s32);
    void fn_1_C990();
    void fn_1_C994();
    void fn_1_CA14(s32);
    void fn_1_CB64();
    OwnedModel modelId;
    OwnedModel animatedModel;
    u8 state18;
    s32 ufoKind;
    Vec position;
    s32 state;
    s32 cameraGroup;
    u8 motionRunning;
    u8 stopped;
    u8 scoringWindow;
    u8 effectFlag;
    f32 motionSpeed;
    f32 motionLength;
    f32 playbackSpeed;
    s32 countdown;
    s16 hookModelId;
    u32 drawCount;
    f32 rotationAngle;
    u8 state54;
    s32 soundHandle;
};

extern "C" {
Vec lbl_1_bss_59C[0x21];
Vec lbl_1_bss_728[0x21];
Vec lbl_1_bss_8B4[0x21];
u8 lbl_1_bss_A40;
ListRoot<CameraModelOwner> lbl_1_bss_A50;
}

/* Create one UFO model pair for the supplied kind and camera. */
static inline void CameraModelCreate(CameraModelPool *object, s32 modelIndex, s32 cameraGroup)
{
    new CameraModelOwner(&object->modelId, lbl_1_rodata_598[modelIndex], modelIndex, cameraGroup);
}

/* Create three UFO kinds per camera and initialize the effect circles. */
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

/* Delete all UFO objects. */
CameraModelPool::~CameraModelPool()
{
    fn_1_BF18();
}

/* Occasionally start UFO motions and update every UFO. */
void CameraModelPool::update()
{
    if ((s32)(RandomUnsigned() & 0x1FFUL) == 0) fn_1_C0FC();
    fn_1_C04C();
}

/* Clear a request flag and stop UFO sounds for request three. */
void CameraModelPool::flagEnabled(int bit)
{
    flags &= ~(1UL << bit);
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

/* Enable UFO scoring effects. */
void fn_1_BB5C()
{
    lbl_1_bss_A40 = 1;
}

extern "C" {

char *lbl_1_data_9A4[3] = {"mapmodel10-ufo_act", "mapmodel12-ufo_act_b",
                              "mapmodel14-ufo_act_c"};

const u32 ufoCameraBits[4] = {1, 2, 4, 8};

}

/* Create and link a camera-specific UFO model pair and effect drawing hook. */
CameraModelOwner::CameraModelOwner(const OwnedModel *sourceModel, unsigned int motionData, s32 ufoKind, s32 cameraGroup)
    : modelId(*sourceModel), animatedModel(motionData)
{
    s16 primaryModelId;
    s16 foregroundModelId;
    s16 motionIdValue;
    CameraModelOwner::Callback callbackState;
    HU3D_MODEL *hookModel;

    this->state18 = 0;
    this->ufoKind = ufoKind;
    this->cameraGroup = cameraGroup;
    this->motionRunning = 0;
    this->stopped = 0;
    this->scoringWindow = 0;
    this->effectFlag = 0;
    this->rotationAngle = 0.0f;
    this->state54 = 0;
    this->soundHandle = -1;
    insert4(&lbl_1_bss_A50);

    primaryModelId = this->modelId;
    fn_80026A44(primaryModelId, 1);
    fn_80026B8C(primaryModelId, 0);
    fn_80026B64(primaryModelId, ufoCameraBits[cameraGroup]);
    fn_800269E8(primaryModelId, 0x4000);

    foregroundModelId = this->animatedModel;
    fn_800269E8(foregroundModelId, 1);
    fn_80026B64(foregroundModelId, ufoCameraBits[cameraGroup]);

    fn_8002741C(this->animatedModel, lbl_1_data_9A4[ufoKind], this->modelId);
    animatedModel.hideObject(lbl_1_data_9A4[this->ufoKind]);
    motionIdValue = fn_8002D3C4(foregroundModelId);
    fn_8002C95C(foregroundModelId, motionIdValue);
    fn_800269E8(foregroundModelId, ((1 << 30) | (1 << 1)));

    this->state = 0;
    this->position.x = this->position.y = this->position.z = 0.0f;
    this->hookModelId = fn_800260F4(fn_1_CD34);
    hookModel = &lbl_802C01C8[this->hookModelId];
    hookModel->hookData = this;
    fn_80026B8C(this->hookModelId, 6);
    fn_80026B64(this->hookModelId, ufoCameraBits[cameraGroup]);

    this->modelId.setPosition(&this->position);
    callbackState = &CameraModelOwner::fn_1_C994;
    this->callbackState = callbackState;
}

/* Stop this UFO sound and release its drawing hook. */
CameraModelOwner::~CameraModelOwner()
{
    if (soundHandle >= 0) {
        HuAudFXStop(soundHandle);
        soundHandle = -1;
    }
    fn_800262D0(hookModelId);
}

/* Delete every UFO from its list. */
void fn_1_BF18()
{
    while (!lbl_1_bss_A50.empty()) delete lbl_1_bss_A50.back();
}

/* Visit all UFOs to advance their callbacks and transforms. */
void fn_1_C04C()
{
    lbl_1_bss_A50.visit(fn_1_C710);
}

/* Check whether one UFO kind is idle in any of the four camera groups. */
int fn_1_C07C(CameraModelOwner *current)
{
    int candidate;
    int step;
    for (candidate = 0; candidate < 4; candidate++) {
        if (!current->stopped && !current->motionRunning) return 1;
        if (candidate == 3) break;
        for (step = 0; step < 3; step++) current = current->next();
    }
    return 0;
}

/* Start one randomly selected UFO kind across all four cameras during play. */
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
    gameMode = currentRequest();
    if (gameMode == 2) {
        entry = lbl_1_bss_A50.front();
        end = lbl_1_bss_A50.end_node();
        startIndex = ((u16)RandomUnsigned() * 3) >> 16;
        for (step = 0; step != startIndex; step++) entry = entry->next();
        if ((u8)fn_1_C07C(entry) != 0) {
            entry->fn_1_CA14(0);
            for (firstStep = 0; firstStep < 3; firstStep++) entry = entry->next();
            entry->fn_1_CA14(1);
            for (secondStep = 0; secondStep < 3; secondStep++) entry = entry->next();
            entry->fn_1_CA14(2);
            for (thirdStep = 0; thirdStep < 3; thirdStep++) entry = entry->next();
            entry->fn_1_CA14(3);
        }
    }
}

/* Stop the UFOs belonging to a player camera. */
void fn_1_C280(int request)
{
    CameraModelOwner *node = lbl_1_bss_A50.front();
    Node *end = lbl_1_bss_A50.end_node();
    for (; node != end; node = node->next()) node->fn_1_C8C0(request);
}

/* Stop every UFO sound. */
void fn_1_C2EC()
{
    CameraModelOwner *node = lbl_1_bss_A50.front();
    Node *end = lbl_1_bss_A50.end_node();
    for (; node != end; node = node->next()) node->stopSound();
}

/* Stop this UFO sound and clear its handle. */
void CameraModelOwner::stopSound()
{
    if (soundHandle >= 0) {
        HuAudFXStop(soundHandle);
        soundHandle = -1;
    }
}

/* Test eight points around a UFO against the camera visibility interval. */
s32 fn_1_C394(const Vec *center, s32 cameraIndex)
{
    Vec position;
    s32 angleIndex;
    f32 angleDegrees;
    f32 angle;

    for (angleIndex = 0; angleIndex < 8; angleIndex++) {
        angleDegrees = (f32)(angleIndex * 0x168) / 8.0f - 180.0f;
        angle = (f32)((M_PI * (f64)angleDegrees) / 180.0);
        position.x = (f32)((f64)center->x + 150.0 * std::cos((f64)angle));
        position.y = (f32)((f64)center->y + 150.0 * std::sin((f64)angle));
        position.z = center->z;
        if ((u8)fn_1_3340(&position, cameraIndex, 1)) {
            return 1;
        }
    }
    return 0;
}

/* Count visible scoring UFOs and optionally schedule their three-point effects. */
s32 fn_1_C4E8(s32 cameraIndex, u8 markEntries, s32 currentValue)
{
    CameraModelOwner *node;
    Node *end;
    s32 matchedCount;

    node = lbl_1_bss_A50.front();
    end = &lbl_1_bss_A50;
    matchedCount = 0;

    while (node != end) {
        if (node->state18 != 0 && node->cameraGroup == cameraIndex && node->scoringWindow != 0) {
            if ((u8)fn_1_C394(&node->position, cameraIndex) != 0) {
                if (markEntries != 0) {
                    node->effectFlag = 1;
                    node->countdown = (s32)((f32)(-(currentValue + 1)) * secondsToFrames(0.2f));
                }
                matchedCount += 3;
                currentValue++;
            }
        }
        node = node->next();
    }
    return matchedCount;
}

/* Invoke one UFO callback and update its model transform. */
void fn_1_C710(CameraModelOwner *self)
{
    (self->*self->callbackState)();
    self->fn_1_C750();
}

/* Track the animated UFO object and spin its foreground model unless stopped. */
void CameraModelOwner::fn_1_C750()
{
    s16 modelArgument;
    s16 model;
    Vec rotation;
    Point3d objectPosition;
    Vec translation;

    model = (s32)this->animatedModel.modelId;
    modelArgument = model;
    Hu3DModelObjPosGet(modelArgument, lbl_1_data_9A4[this->ufoKind], &objectPosition);
    this->position.x = objectPosition.x;
    this->position.y = objectPosition.y;
    this->position.z = objectPosition.z;
    this->position.y += 50.0f;
    if (this->stopped == 0) {
        this->rotationAngle += 3.0f;
        if (this->rotationAngle > 180.0f) {
            this->rotationAngle -= 360.0f;
        }
    }

    f64 angle = this->rotationAngle;
    rotation.x = 0.0f;
    rotation.y = angle;
    rotation.z = 0.0f;
    translation.x = 0.0f;
    translation.y = 0.0f;
    translation.z = 0.0f;
    modelId.setRotation(&rotation);
    modelId.setPosition(&translation);
}
/* Stop this UFO motion and sound when its camera matches the player. */
void CameraModelOwner::fn_1_C8C0(s32 playerIndex)
{
    if (this->cameraGroup == playerIndex && this->stopped == 0) {
        s16 modelId;

        stopSound();
        this->stopped = 1;
        modelId = this->animatedModel.id();
        fn_800269E8(modelId, ((1 << 30) | (1 << 1)));
        CameraModelOwner::Callback callback = &CameraModelOwner::fn_1_C990;
        this->callbackState = callback;
    }
}
/* Leave the stopped UFO state unchanged. */
void CameraModelOwner::fn_1_C990() {}
/* Hide both UFO models and clear its motion and scoring flags. */
void CameraModelOwner::fn_1_C994()
{
    HU3D_MODELID first = modelId.id();
    fn_800269E8(first, (1 << 0));
    HU3D_MODELID second = animatedModel.id();
    fn_800269E8(second, (1 << 0));
    state = 0;
    scoringWindow = 0;
    state18 = 0;
    motionRunning = 0;
}
const s32 lbl_1_rodata_624[4] = {0x634,0x635,0x636,0x637};

/* Start a UFO motion and sound at a randomized playback speed. */
void CameraModelOwner::fn_1_CA14(s32 motionIndex)
{
    if (this->stopped == 0) {
        this->soundHandle = HuAudFXPlay(lbl_1_rodata_624[motionIndex]);
        s16 modelId = this->animatedModel.id();
        fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
        fn_80026A44(modelId, ((1 << 30) | (1 << 0)));
        fn_8002D414(modelId, 0.0f);
        this->motionSpeed = 0.8f + 1.2f * RandomFloat();
        this->motionLength = fn_8002D530(modelId);
        this->playbackSpeed = this->motionSpeed / 60.0f;
        fn_8002D86C(modelId, this->playbackSpeed);
        this->state = 0;
        this->state18 = 0;
        this->motionRunning = 1;
        CameraModelOwner::Callback callback = &CameraModelOwner::fn_1_CB64;
        this->callbackState = callback;
    }
}
f32 lbl_1_data_9D4[3][8] = {
 {0.0f,0.175f,0.405f,0.6f,0.62f,0.703f,0.79f,1.0f},
 {0.0f,0.123f,0.25f,0.715f,0.8f,1.0f,1.0f,1.0f},
 {0.0f,0.06f,0.254f,0.66f,0.806f,0.886f,0.94f,1.0f}
};
/* Advance UFO scoring windows and return to the hidden state after its motion. */
void CameraModelOwner::fn_1_CB64()
{
    f32 progress;
    s32 frame;
    s16 modelId;
    Callback callback;

    scoringWindow = 0;
    progress = (f32)state * playbackSpeed / motionLength;
    for (frame = 0; frame < 7; frame++) {
        if (lbl_1_data_9D4[ufoKind][frame] <= progress &&
            lbl_1_data_9D4[ufoKind][frame + 1] > progress) {
            if ((frame & 1) != 0) {
                scoringWindow = 1;
            }
            break;
        }
    }

    if (state > 10) {
        modelId = animatedModel.id();
        if (fn_8002D75C(modelId) == 1) {
            stopSound();
            callback = &CameraModelOwner::fn_1_C994;
            this->callbackState = callback;
            return;
        }
    }
    HU3D_MODELID firstModel = this->modelId.id();
    fn_80026A44(firstModel, 1);
    HU3D_MODELID secondModel = animatedModel.id();
    fn_80026A44(secondModel, 1);
    state18 = 1;
    motionRunning = 1;
    state++;
}

/* Dispatch the model drawing hook to its UFO object. */
void fn_1_CD34(HU3D_MODEL *model, Mtx *matrix)
{
    fn_8002472C();
    void *hookData = model->hookData;
    static_cast<CameraModelOwner *>(hookData)->fn_1_CD7C(*matrix);
}

/* Draw the UFO effect when both effect flags are enabled. */
void CameraModelOwner::fn_1_CD7C(Mtx matrix)
{
    if (lbl_1_bss_A40 && effectFlag) fn_1_CDD4(matrix);
}

/* Draw a growing colored ring and award the UFO score after its delay. */
void CameraModelOwner::fn_1_CDD4(Mtx matrix)
{
    Mtx inverse;
    Mtx transform;
    s32 frame;
    s32 vertex;
    f32 scale;
    s32 colorOffset;
    ScoreDisplay *singleton;

    if (++countdown < 0) {
        return;
    }
    if (countdown == 0) {
        singleton = &ScoreDisplay::getChecked();
        singleton->soundPending = 1;
    }

    if ((f64)countdown > 12.0) {
        if (state54 == 0) {
            state54 = 1;
            ScoreDisplay::getChecked().fn_1_83CC(cameraGroup, 3);
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
    configureColorDrawing();
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 0x42);
    colorOffset = (drawCount++ & 7) << 5;
    for (vertex = 0; vertex < 33; vertex++) {
        GXPosition3f32(scale * lbl_1_bss_59C[vertex].x,
                       scale * lbl_1_bss_59C[vertex].y, 0.0f);
        GXColor4u8(0xFF, (u8)colorOffset, 0, (u8)frame);
        GXPosition3f32(scale * lbl_1_bss_728[vertex].x,
                       scale * lbl_1_bss_728[vertex].y, 0.0f);
        GXColor4u8(0xFF, (u8)colorOffset, 0, (u8)frame);
    }
}
