#include "dolphin/gx.h"
#include "REL/framework/list_object.h"
#include "REL/m603DLL/model_motion.h"
#include "humath.h"
#include "REL/framework/frame_time_inline.h"

#include <cmath>
#include "REL/m603DLL/zako.h"
#include "REL/m603DLL/random.h"
#include "REL/m603DLL/score_owner.h"
#include "datadir_enum.h"

extern "C" {
#include "dolphin.h"
#include "dolphin/types.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/memory.h"
#include "game/gamework.h"
#include "REL/m603DLL.h"
}

#include "REL/m603DLL/draw_node.h"

extern "C" ListRoot<DrawNode> lbl_1_bss_588;

extern "C" {
char lbl_1_data_450[] = "mapmodel-dmy_a";
char lbl_1_data_45F[] = "mapmodel-dmy_b";
char lbl_1_data_46E[] = "mapmodel-dmy_c";
char lbl_1_data_47D[] = "mapmodel-dmy_d";
char lbl_1_data_48C[] = "mapmodel-dmy_e";
char lbl_1_data_49B[] = "mapmodel-dmy_f";
char lbl_1_data_4AA[] = "mapmodel-dmy_g";
char lbl_1_data_4B9[] = "mapmodel-dmy_h";
char lbl_1_data_4C8[] = "mapmodel-dmy_i";
char lbl_1_data_4D7[] = "mapmodel-dmy_j";
char lbl_1_data_4E6[] = "mapmodel-dmy_k";
char lbl_1_data_4F5[] = "mapmodel-dmy_l";
char lbl_1_data_504[] = "mapmodel-dmy_m";
char lbl_1_data_513[] = "mapmodel-dmy_n";
char lbl_1_data_522[] = "mapmodel-dmy_o";
char lbl_1_data_531[] = "mapmodel-dmy_p";
char lbl_1_data_540[] = "mapmodel-dmy_q";
char *lbl_1_data_550[17] = { lbl_1_data_450, lbl_1_data_45F, lbl_1_data_46E, lbl_1_data_47D, lbl_1_data_48C, lbl_1_data_49B, lbl_1_data_4AA, lbl_1_data_4B9, lbl_1_data_4C8, lbl_1_data_4D7, lbl_1_data_4E6, lbl_1_data_4F5, lbl_1_data_504, lbl_1_data_513, lbl_1_data_522, lbl_1_data_531, lbl_1_data_540 };
}

typedef Vec Vector3f9170;

extern "C" {
extern Vector3f9170 lbl_1_bss_264[0x21];
extern Vector3f9170 lbl_1_bss_3F0[0x21];
extern Vector3f9170 lbl_1_bss_D8[0x21];

extern u8 lbl_1_bss_D4;
extern s32 lbl_1_data_594;
const s32 lbl_1_rodata_3F0[4] = {1, 2, 4, 8};
extern s32 fn_1_1BBC4(void *, const void *);
}

extern void fn_1_A9EC(HU3D_MODEL *, Mtx *);

extern "C" {

}

void fn_1_DC8C(s32);
void fn_1_DB70(s32);
extern "C" {
}
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
    fn_80026B64(model, (u16)lbl_1_rodata_3F0[groupIndex]);
    fn_800269E8(model, 0x4000U);
    motion = this->secondModel;
    fn_80026A44(motion, 1U);
    fn_80026B64(motion, (u16)lbl_1_rodata_3F0[groupIndex]);
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
                       (u16)lbl_1_rodata_3F0[groupIndex]);
    firstModel.setPosition(&position);
    fn_1_A060(0, 0);
    {
        DrawNode::Callback callback = &DrawNode::fn_1_A238;
        this->callback = callback;
    }
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
DrawNode::~DrawNode() { fn_800262D0(hookModel); }
void fn_1_9794() {
    while (!lbl_1_bss_588.empty()) delete lbl_1_bss_588.back();
}

extern "C" {
struct RandomIndexTable { u32 values[64]; };
const RandomIndexTable lbl_1_rodata_450 = {{
    0, 1, 0, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    0, 1, 16, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    0, 1, 8, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    0, 0, 0, 8, 8, 8, 16, 16, 16, 16, 16, 16, 7, 7, 8, 8,
}};

}
void fn_1_9E90(DrawNode *);
void fn_1_98AC() { lbl_1_bss_588.visit(fn_1_9E90); }
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
    RandomIndexTable randomIndices = lbl_1_rodata_450;
    skipCount = randomIndices.values[RandomUnsigned() & 0x3F];
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
void fn_1_9B88(int request)
{
    DrawNode *node = lbl_1_bss_588.front();
    Node *end = lbl_1_bss_588.end_node();
    for (; node != end; node = node->next()) {
        node->fn_1_A160(request);
    }
}
void fn_1_9BF4()
{
    DrawNode *node = lbl_1_bss_588.front();
    Node *end = lbl_1_bss_588.end_node();
    for (; node != end; node = node->next()) {
        node->effectGate = 1;
    }
}
void fn_1_9C58()
{
    DrawNode *node = lbl_1_bss_588.front();
    Node *end = lbl_1_bss_588.end_node();
    for (; node != end; node = node->next()) {
        node->motionPending = 1;
        node->remainingFrames = node->cameraIndex * 4;
    }
}
s32 fn_1_3340(Vec *,s32,u8);
int fn_1_9CC8(const Vec *position, int cameraIndex)
{
    Vec worldPosition = *position;

    if ((u8)fn_1_3340(&worldPosition, cameraIndex, TRUE)) {
        return 1;
    } else {
        return 0;
    }
}
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
void fn_1_9E90(DrawNode *self) { (self->*self->callback)(); self->fn_1_9ED0(); }
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
inline void DrawNode::fn_1_A060(s32 motionId, u8 queued)
{
    DrawModelPool *motionSequence;
    s16 modelId;
    s16 selectedMotion;

    motionSequence = (&DrawModelPool::getChecked());
    modelId = this->firstModel;
    selectedMotion = motionSequence->m_mot_id.at(motionId);
    if (this->currentMotion != motionId) {
        fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
        fn_8002CCC0(modelId, selectedMotion, 0.0f, 8.0f,
                           queued == 0 ? ((1 << 30) | (1 << 0)) : 0);
        this->currentMotion = motionId;
    }
}
void DrawNode::fn_1_A160(s32 key)
{
    if (this->key == key && this->active == 0) {
        s16 modelA;
        s16 modelB;

        this->active = 1;
        modelA = this->model_a();
        fn_800269E8(modelA, ((1 << 30) | (1 << 1)));
        modelB = this->model_b();
        fn_800269E8(modelB, ((1 << 30) | (1 << 1)));
        {
            DrawNode::Callback callback = &DrawNode::fn_1_A5D4;
            this->callback = callback;
        }
        if (this->motionState != 0) {
            fn_1_DC8C(key);
        }
    }
}

void DrawNode::fn_1_A238()
{
    if (motionPending != 0 && --remainingFrames < 0) {
        s16 modelId = secondModel;

        fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
        fn_80026A44(modelId, ((1 << 30) | (1 << 0)));
        {
            DrawNode::Callback callback = &DrawNode::fn_1_A3A8;
            this->callback = callback;
        }
    }
    fn_1_A060(1, 0);
}

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
void DrawNode::fn_1_A524()
{
    s32 mode = currentRequest();

    if (mode == 1 || mode == 2) {
        s16 primaryModel;
        s16 secondaryModel;

        primaryModel = this->primary_model();
        fn_80026A44(primaryModel, 1);
        secondaryModel = this->secondary_model();
        fn_80026A44(secondaryModel, 1);
        DrawNode::Callback callback = &DrawNode::fn_1_A5D8;
        this->callback = callback;
    }
}

void DrawNode::fn_1_A5D4() {}
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
        modelId = this->model_id();
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

extern f32 lbl_1_data_5F4[17][12];
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
        modelId = model_id();
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

typedef struct {
    f32 x;
    f32 y;
    f32 unknown08;
} AAA4Point;

void fn_1_A9EC(HU3D_MODEL *model, Mtx *matrix)
{
    fn_8002472C();
    void *hookData = model->hookData;
    static_cast<DrawNode *>(hookData)->fn_1_AA34(*matrix);
}

void DrawNode::fn_1_AA34(Mtx matrix)
{
    if ((effectGate != 0) && (effectFlag != 0)) fn_1_AAA4(matrix);
    if (cameraIndex != 0xB) fn_1_B0C0(matrix);
}

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

            owner = (&ScoreDisplay::getChecked());
            owner->soundPending = 1;
        }
        if ((f64)effectFrame > 12.0) {
            if (fired == 0) {
                fired = 1;
                mode = 1;
                if (motionState != 0) {
                    mode = 3;
                }
                (&ScoreDisplay::getChecked())->fn_1_83CC(key, mode);
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
        GXSetNumTevStages(1U);
        GXSetChanCtrl(GX_COLOR0A0, 0U, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        GXSetNumChans(1U);
        GXSetNumTexGens(0U);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
        GXSetCullMode(GX_CULL_NONE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
        GXSetZMode(0U, GX_ALWAYS, 0U);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0U);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0U);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
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
        {
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
}

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
    GXSetNumTevStages(1U);
    GXSetChanCtrl(GX_COLOR0A0, 0U, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(1U);
    GXSetNumTexGens(0U);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetZMode(1U, GX_LEQUAL, 0U);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0U);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0U);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
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

extern "C" {
u8 lbl_1_bss_D4;
Vec lbl_1_bss_D8[0x21];
Vec lbl_1_bss_264[0x21];
Vec lbl_1_bss_3F0[0x21];
ListRoot<DrawNode> lbl_1_bss_588;
}
