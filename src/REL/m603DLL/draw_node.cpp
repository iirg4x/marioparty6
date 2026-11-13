#include "REL/m603DLL/draw_owner.h"
#include "REL/m603DLL/camera_group.h"
#include "dolphin/gx.h"
#include "REL/framework/list_object.h"
#include "REL/m603DLL/model_motion.h"
#include "humath.h"
#include "REL/framework/frame_time_inline.h"

#include <cmath>
#include "REL/m603DLL/zako.h"
#include "REL/framework/random.h"
#include "REL/m603DLL/score_owner.h"
#include "datadir_enum.h"

extern "C" {
#include "dolphin.h"
#include "dolphin/types.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/memory.h"
#include "game/gamework.h"
}

#include "REL/m603DLL/draw_node.h"

extern "C" {
u8 lbl_1_bss_D4;
Vec lbl_1_bss_D8[0x21];
Vec lbl_1_bss_264[0x21];
Vec lbl_1_bss_3F0[0x21];
ListRoot<DrawNode> lbl_1_bss_588;
}

extern "C" {
char *lbl_1_data_550[17] = {
    "mapmodel-dmy_a",
    "mapmodel-dmy_b",
    "mapmodel-dmy_c",
    "mapmodel-dmy_d",
    "mapmodel-dmy_e",
    "mapmodel-dmy_f",
    "mapmodel-dmy_g",
    "mapmodel-dmy_h",
    "mapmodel-dmy_i",
    "mapmodel-dmy_j",
    "mapmodel-dmy_k",
    "mapmodel-dmy_l",
    "mapmodel-dmy_m",
    "mapmodel-dmy_n",
    "mapmodel-dmy_o",
    "mapmodel-dmy_p",
    "mapmodel-dmy_q"
};
}

extern "C" {

const s32 targetCameraBits[4] = {1, 2, 4, 8};
}

/* Create a scoring target and drawing hook, and initialize the shared circle vertices. */
DrawNode::DrawNode(const OwnedModel *sourceModel, u32 modelData, s32 cameraIndex, s32 groupIndex)
    : firstModel(*sourceModel), secondModel(modelData)
{
    s16 model;
    s16 motion;
    HU3D_MODEL *hookModel;
    s16 motionId;
    s32 index;
    f32 radians;
    f32 angle;

    this->motionVisible = 0;
    this->motionPending = 0;
    this->cameraIndex = cameraIndex;
    this->key = groupIndex;
    this->motionRunning = 0;
    this->active = 0;
    this->scoringWindow = 0;
    this->effectFlag = 0;
    this->motionState = 0;
    this->fired = 0;
    this->effectGate = 0;
    insert4(&lbl_1_bss_588);
    this->currentMotion = -1;
    model = this->firstModel;
    fn_80026A44(model, 1U);
    fn_80026B8C(model, 0);
    fn_80026B64(model, (u16)targetCameraBits[groupIndex]);
    fn_800269E8(model, 0x4000U);
    motion = this->secondModel;
    fn_80026A44(motion, 1U);
    fn_80026B64(motion, (u16)targetCameraBits[groupIndex]);
    motionId = fn_8002D3C4(motion);
    fn_8002C95C(motion, motionId);
    fn_800269E8(motion, 0x40000002U);
    fn_80026A44(motion, 0x40000001U);
    fn_8002D86C(motion, 0.05f);
    fn_8002D414(motion, fn_8002D530(motion) / 2.0f);
    this->duration = 0;
    this->position.x =
        this->position.y = this->position.z = 0.0f;
    this->hookModel = fn_800260F4(fn_1_A9EC);
    hookModel = &lbl_802C01C8[this->hookModel];
    hookModel->hookData = this;
    fn_80026B8C(this->hookModel, 6);
    fn_80026B64(this->hookModel,
                       (u16)targetCameraBits[groupIndex]);
    firstModel.setPosition(&position);
    fn_1_A060(0, 0);
    DrawNode::Callback callback = &DrawNode::fn_1_A238;
    this->callback = callback;
    if (lbl_1_bss_D4 == 0) {
        lbl_1_bss_D4 = 1;
        for (index = 0; index < 0x21; index++) {
            angle = ((f32)(index & 0x1F) / 32.0f) * 360.0f - 180.0f;
            angle += 90.0f;
            if (angle > 180.0f) {
                angle -= 360.0f;
            }
            radians = (f32)((M_PI * (f64)angle) / 180.0);
            lbl_1_bss_264[index].x = (f32)(70.0 * std::cos((f64)radians));
            lbl_1_bss_264[index].y = (f32)(70.0 * std::sin((f64)radians));
            lbl_1_bss_264[index].z = 0.0f;
            lbl_1_bss_D8[index].x = (f32)(80.0 * std::cos((f64)radians));
            lbl_1_bss_D8[index].y = (f32)(80.0 * std::sin((f64)radians));
            lbl_1_bss_D8[index].z = 0.0f;
            lbl_1_bss_3F0[index].x = (f32)(50.0 * std::cos((f64)radians));
            lbl_1_bss_3F0[index].y = 0.0f;
            lbl_1_bss_3F0[index].z = (f32)(50.0 * std::sin((f64)radians));
        }
    }
}
/* Release the target drawing hook. */
DrawNode::~DrawNode() { fn_800262D0(hookModel); }
/* Delete every scoring target. */
void fn_1_9794() {
    while (!lbl_1_bss_588.empty()) delete lbl_1_bss_588.back();
}

/* Advance every target callback and its model transform. */
void fn_1_98AC() { lbl_1_bss_588.visit(fn_1_9E90); }
/* Check up to four corresponding targets for one that is idle. */
int fn_1_98DC(DrawNode *current)
{
    int candidate;
    int step;

    for (candidate = 0; candidate < 4; candidate++) {
        if (!current->active && !current->motionRunning) {
            return 1;
        }
        if (candidate == 3) {
            break;
        }
        for (step = 0; step < 17; step++) {
            current = current->next();
        }
    }
    return 0;
}
/* Choose and activate an idle target during the playing request. */
void fn_1_995C(void)
{
    DrawNode *entry;
    Node *end;
    s32 gameMode;
    s32 skipCount;
    s32 step;
    s32 firstStep;
    s32 secondStep;
    s32 thirdStep;
    bool randomMotion;
    f32 motionFrames;

    gameMode = currentRequest();
    if (gameMode != 2) {
        return;
    }

    entry = lbl_1_bss_588.front();
    end = lbl_1_bss_588.end_node();
    u32 randomIndices[64] = {
    0, 1, 0, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    0, 1, 16, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    0, 1, 8, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    0, 0, 0, 8, 8, 8, 16, 16, 16, 16, 16, 16, 7, 7, 8, 8,
};
    skipCount = randomIndices[RandomUnsigned() & 0x3F];
    randomMotion = 0;
    if (GWMgNightFGet() == 0) {
        randomMotion = ((s32)RandomUnsigned() & 0xF) == 0;
    }

    for (step = 0; step != skipCount; step++) {
        entry = entry->next();
    }

    if ((u8)fn_1_98DC(entry) != 0) {
        motionFrames = 0.5f +
                       2.0f * RandomFloat();
        entry->fn_1_A718(randomMotion, motionFrames);
        for (firstStep = 0; firstStep < 17; firstStep++) {
            entry = entry->next();
        }
        entry->fn_1_A718(randomMotion, motionFrames);
        for (secondStep = 0; secondStep < 17; secondStep++) {
            entry = entry->next();
        }
        entry->fn_1_A718(randomMotion, motionFrames);
        for (thirdStep = 0; thirdStep < 17; thirdStep++) {
            entry = entry->next();
        }
        entry->fn_1_A718(randomMotion, motionFrames);
    }
}
/* Send a request to every scoring target. */
void fn_1_9B88(int request)
{
    DrawNode *node = lbl_1_bss_588.front();
    Node *end = lbl_1_bss_588.end_node();
    for (; node != end; node = node->next()) {
        node->fn_1_A160(request);
    }
}
/* Enable the scoring effect for every target. */
void fn_1_9BF4()
{
    DrawNode *node = lbl_1_bss_588.front();
    Node *end = lbl_1_bss_588.end_node();
    for (; node != end; node = node->next()) {
        node->effectGate = 1;
    }
}
/* Schedule each target opening using its object index. */
void fn_1_9C58()
{
    DrawNode *node = lbl_1_bss_588.front();
    Node *end = lbl_1_bss_588.end_node();
    for (; node != end; node = node->next()) {
        node->motionPending = 1;
        node->remainingFrames = node->cameraIndex * 4;
    }
}
/* Test whether a copied world position projects into the camera view. */
int fn_1_9CC8(const Vec *position, int cameraIndex)
{
    Vec worldPosition = *position;

    if ((u8)fn_1_3340(&worldPosition, cameraIndex, TRUE)) {
        return 1;
    } else {
        return 0;
    }
}
/* Count visible scoring targets, optionally scheduling their photo effects. */
int fn_1_9D34(int playerIndex, unsigned char markEntries)
{
    Node *end;
    DrawNode *node;
    int weightedCount;
    int visibleCount;
    unsigned char inView;

    node = lbl_1_bss_588.front();
    end = lbl_1_bss_588.end_node();
    weightedCount = 0;
    visibleCount = 0;
    while (node != end) {
        if (node->motionVisible != 0 && node->key == playerIndex && node->scoringWindow != 0) {
            Vec position = node->position;

            if ((u8)fn_1_3340(&position, playerIndex, 1) != 0) {
                inView = 1;
            } else {
                inView = 0;
            }
            if (inView != 0) {
                if (markEntries != 0) {
                    node->effectFlag = 1;
                    node->effectFrame = (int)((f32)(-(visibleCount + 1)) * secondsToFrames(0.2f));
                }
                if (node->motionState != 0) {
                    weightedCount += 3;
                } else {
                    weightedCount++;
                }
                visibleCount++;
            }
        }
        node = node->next();
    }
    return weightedCount;
}
/* Run one target callback and update its model transform. */
void fn_1_9E90(DrawNode *self) { (self->*self->callback)(); self->fn_1_9ED0(); }
/* Read the animated object position and update the target and optional motion model. */
void DrawNode::fn_1_9ED0()
{
    Vec rotation;
    Point3d objectPosition;
    Vec translation;
    f64 angle;
    s16 modelId;

    modelId = secondModel;
    Hu3DModelObjPosGet(modelId, lbl_1_data_550[cameraIndex], &objectPosition);
    secondModel.hideObject(lbl_1_data_550[cameraIndex]);

    angle = 180.0 *
            (std::atan2((f64)objectPosition.x, -(f64)objectPosition.z) / M_PI);
    angle = (f64)(f32)angle;
    position.x = objectPosition.x;
    position.y = objectPosition.y;
    position.z = objectPosition.z;
    position.y += 50.0f;

    rotation.x = 0.0f;
    rotation.y = (f32)-angle;
    rotation.z = 0.0f;
    translation.x = 0.0f;
    translation.y = 0.0f;
    translation.z = 0.0f;
    if (motionState != 0) {
        fn_1_DDA8(resourceState, &objectPosition, &rotation);
    } else {
        firstModel.setRotation(&rotation);
        firstModel.setPosition(&objectPosition);
    }
}
/* Change the target motion when the requested motion differs. */
inline void DrawNode::fn_1_A060(s32 motionId, u8 queued)
{
    DrawModelPool *motionSequence;
    s16 modelId;
    s16 selectedMotion;

    motionSequence = &DrawModelPool::getChecked();
    modelId = this->firstModel;
    selectedMotion = motionSequence->m_mot_id.at(motionId);
    if (this->currentMotion != motionId) {
        fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
        fn_8002CCC0(modelId, selectedMotion, 0.0f, 8.0f,
                           queued == 0 ? ((1 << 30) | (1 << 0)) : 0);
        this->currentMotion = motionId;
    }
}
/* Handle the target request and select its next callback. */
void DrawNode::fn_1_A160(s32 key)
{
    if (this->key == key && this->active == 0) {
        s16 modelA;
        s16 modelB;

        this->active = 1;
        modelA = this->secondModel.id();
        fn_800269E8(modelA, ((1 << 30) | (1 << 1)));
        modelB = this->firstModel.id();
        fn_800269E8(modelB, ((1 << 30) | (1 << 1)));
        DrawNode::Callback callback = &DrawNode::fn_1_A5D4;
        this->callback = callback;
        if (this->motionState != 0) {
            fn_1_DC8C(key);
        }
    }
}

/* Wait for the staggered opening delay, then start the target animation. */
void DrawNode::fn_1_A238()
{
    if (motionPending != 0 && --remainingFrames < 0) {
        s16 modelId = secondModel;

        fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
        fn_80026A44(modelId, ((1 << 30) | (1 << 0)));
        DrawNode::Callback callback = &DrawNode::fn_1_A3A8;
        this->callback = callback;
    }
    fn_1_A060(1, 0);
}

/* Wait for the opening animation to finish and restore the target models. */
void DrawNode::fn_1_A3A8()
{
    if (duration > 10) {
        s16 modelForCheck = secondModel;

        if (fn_8002D75C(modelForCheck) == 1) {
            s16 modelForVisibility = firstModel;
            fn_800269E8(modelForVisibility, 1);
            s16 modelForRestore = secondModel;
            fn_800269E8(modelForRestore, 1);
            DrawNode::Callback callback = &DrawNode::fn_1_A524;
            this->callback = callback;
        }
    }
    fn_1_A060(1, 0);
}
/* Activate the target models when the request reaches intro or play. */
void DrawNode::fn_1_A524()
{
    s32 mode = currentRequest();

    if (mode == 1 || mode == 2) {
        s16 primaryModel;
        s16 secondaryModel;

        primaryModel = this->firstModel.id();
        fn_80026A44(primaryModel, 1);
        secondaryModel = this->secondModel.id();
        fn_80026A44(secondaryModel, 1);
        DrawNode::Callback callback = &DrawNode::fn_1_A5D8;
        this->callback = callback;
    }
}

/* Leave the target state unchanged. */
void DrawNode::fn_1_A5D4() {}
/* Reset the target motion, scoring flags and optional motion model. */
void DrawNode::fn_1_A5D8()
{
    s16 modelId = this->firstModel;
    fn_80026A44(modelId, 1);
    fn_1_A060(0, 0);
    if (this->motionState != 0) {
        fn_1_DB70(this->resourceState);
    }
    this->duration = 0;
    this->motionState = 0;
    this->scoringWindow = 0;
    this->motionVisible = 0;
    this->motionRunning = 0;
}
/* Start a target animation with the requested bonus flag and speed. */
void DrawNode::fn_1_A718(bool motionState, f32 motionFrames)
{
    s16 modelId;
    DrawNode::Callback callback;

    if (this->active == 0) {
        if (motionState != 0) {
            if ((this->resourceState = fn_1_D748(this->key)) >= 0) {
                this->motionState = motionState;
            }
        }
        modelId = this->secondModel.id();
        fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
        fn_80026A44(modelId, ((1 << 30) | (1 << 0)));
        fn_8002D414(modelId, 0.0f);
        this->motionFrames = motionFrames;
        this->motionMaximum = fn_8002D530(modelId);
        this->motionSpeed = this->motionFrames / 60.0f;
        fn_8002D86C(modelId, this->motionSpeed);
        this->duration = 0;
        this->motionVisible = 0;
        this->motionRunning = 1;
        callback = &DrawNode::fn_1_A858;
        this->callback = callback;
    }
}

f32 lbl_1_data_5F4[17][12] = {
    { 0.0f, 0.2f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 0.3f, 0.7f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 0.3f, 0.7f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 0.1f, 0.23f, 0.27f, 0.38f, 0.66f, 0.73f, 0.76f, 0.86f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 0.11f, 0.84f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 0.24f, 0.76f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 0.12f, 0.38f, 0.68f, 0.83f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 0.07f, 0.44f, 0.63f, 0.87f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 0.22f, 0.77f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 0.06f, 0.175f, 0.22f, 0.305f, 0.35f, 0.69f, 0.72f, 0.76f, 0.79f, 0.9f, 1.0f },
    { 0.0f, 0.12f, 0.4f, 0.66f, 0.83f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 0.21f, 0.77f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 0.16f, 0.81f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 0.11f, 0.84f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 0.23f, 0.76f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 0.12f, 0.81f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 0.2f, 0.79f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f }
};

/* Advance the target animation and select scoring windows from its timing table. */
void DrawNode::fn_1_A858(void)
{
    f32 progress;
    s32 frame;
    s16 modelId;
    DrawNode::Callback callback;

    scoringWindow = 0;
    progress = (f32)duration * motionSpeed / motionMaximum;
    for (frame = 0; frame < 11; frame++) {
        if (lbl_1_data_5F4[cameraIndex][frame] <= progress &&
            lbl_1_data_5F4[cameraIndex][frame + 1] > progress) {
            if ((frame & 1) != 0) {
                scoringWindow = 1;
            }
            break;
        }
    }

    if (duration > 10) {
        modelId = secondModel.id();
        if (fn_8002D75C(modelId) == 1) {
            callback = &DrawNode::fn_1_A5D8;
            this->callback = callback;
            return;
        }
    }
    motionVisible = 1;
    motionRunning = 1;
    duration++;
}

/* Dispatch the drawing hook to its target object. */
void fn_1_A9EC(HU3D_MODEL *model, Mtx *matrix)
{
    fn_8002472C();
    void *hookData = model->hookData;
    static_cast<DrawNode *>(hookData)->fn_1_AA34(*matrix);
}

/* Draw the enabled photo effect and the target ground shadow. */
void DrawNode::fn_1_AA34(Mtx matrix)
{
    if ((effectGate != 0) && (effectFlag != 0)) fn_1_AAA4(matrix);
    if (cameraIndex != 0xB) fn_1_B0C0(matrix);
}

/* Draw the expanding photo ring and award the target score once. */
void DrawNode::fn_1_AAA4(Mtx matrix)
{
    Mtx inverse;
    Mtx translated;
    s32 alpha;
    s32 mode;
    s32 color;
    f32 fade;

    if (++effectFrame >= 0) {
        if (effectFrame == 0) {
            ScoreDisplay *owner;

            owner = &ScoreDisplay::getChecked();
            owner->soundPending = 1;
        }
        if ((f64)effectFrame > 12.0) {
            if (fired == 0) {
                fired = 1;
                mode = 1;
                if (motionState != 0) {
                    mode = 3;
                }
                ScoreDisplay::getChecked().fn_1_83CC(key, mode);
            }
            effectFrame = 12;
        }

        alpha = (s32)(255.0f * ((f32)effectFrame / 12.0f));
        fade = (f32)((f64)(f32)(0x17F - alpha) / 128.0);
        PSMTXInverse(matrix, inverse);
        inverse[0][3] = inverse[1][3] = inverse[2][3] = 0.0f;
        PSMTXTrans(translated, position.x, position.y, position.z);
        PSMTXConcat(matrix, translated, translated);
        PSMTXConcat(translated, inverse, translated);
        GXLoadPosMtxImm(translated, GX_PNMTX0);
        configureColorDrawing();
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 0x42U);
        color = (colorIndex++ & 7) << 5;
        if (motionState != 0) {
            s32 i;
            for (i = 0; i < 0x21; i++) {
                GXPosition3f32(fade * lbl_1_bss_D8[i].x,
                               fade * lbl_1_bss_D8[i].y, 0.0f);
                GXColor4u8(0xFF, color, 0, (u8)alpha);
                GXPosition3f32(fade * lbl_1_bss_264[i].x,
                               fade * lbl_1_bss_264[i].y, 0.0f);
                GXColor4u8(0xFF, color, 0, (u8)alpha);
            }
            return;
        }
        s32 i;
        for (i = 0; i < 0x21; i++) {
            GXPosition3f32(fade * lbl_1_bss_D8[i].x,
                           fade * lbl_1_bss_D8[i].y, 0.0f);
            GXColor4u8(0xFF, 0, 0, (u8)alpha);
            GXPosition3f32(fade * lbl_1_bss_264[i].x,
                           fade * lbl_1_bss_264[i].y, 0.0f);
            GXColor4u8(0xFF, 0, 0, (u8)alpha);
        }
    }
}

/* Draw the translucent ground shadow beneath the target. */
void DrawNode::fn_1_B0C0(Mtx matrix)
{
    f32 x;
    f32 y;
    f32 z;
    f32 xOffset;
    f32 yPosition;
    f32 zOffset;
    s32 i;

    GXLoadPosMtxImm(matrix, GX_PNMTX0);
    configureColorDrawing(GX_TRUE, GX_LEQUAL);
    GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 0x22U);
    z = this->position.z;
    y = this->position.y - 49.0f;
    x = this->position.x;
    GXPosition3f32(x, y, z);
    GXColor4u8(0, 0, 0, 0x80);
    for (i = 0; i < 0x21; i++) {
        zOffset = this->position.z + lbl_1_bss_3F0[i].z;
        yPosition = this->position.y - 49.0f;
        xOffset = this->position.x + lbl_1_bss_3F0[i].x;
        GXPosition3f32(xOffset, yPosition, zOffset);
        GXColor4u8(0, 0, 0, 0);
    }
}
