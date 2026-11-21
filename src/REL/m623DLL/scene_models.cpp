// Course models, terrain sampling and drawing for the motion strip.

extern "C" {
#include "game/sprite.h"
#include "dolphin/types.h"
#include "dolphin/os.h"
#include <stdio.h>
#include "dolphin/mtx/GeoTypes.h"
#include <math.h>
#include "game/hu3d.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/sprput.h"
#include "humath.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
}

#include <new>
#include "REL/m623DLL/scene_collision.h"
#include "REL/framework/debug_geometry.h"
#include "REL/framework/list_object.h"
#include "REL/framework/linknode.h"
#include "REL/framework/model.h"
#include "REL/m623DLL/animation_sample.h"
#include "REL/framework/category_sphere.h"
#include "datadir_enum.h"
#include "REL/framework/world_units.h"
#include "REL/framework/singleton.h"
#include "REL/m623DLL/scene_model_owner.h"
#include "REL/m623DLL/scene_storage.h"
#include "REL/m623DLL/fire_particle.h"

// The animation shared by the fire effects.
ANIMDATA *lbl_1_bss_90;
// The texture animation used by the scene path.
ANIMDATA *lbl_1_bss_94;
// The samples used to shape the path motion.
f32 lbl_1_bss_98[128];
// The vertices across the animated path strip.
AnimationSample lbl_1_bss_298[30];

void fn_1_3158(SceneModel *scene);

void fn_1_3424(HU3D_MODEL *model, Mtx *matrix);

// Reads the animation data for the supplied resource.
ANIMDATA *fn_1_1FA8(unsigned long resource)
{
    ANIMDATA *animation = HuSprAnimDataRead(resource);
    HuSprAnimLock(animation);
    return animation;
}

// Loads the scene models and initializes the path and strip samples.
SceneModel::SceneModel(unsigned char dayMode)
    : OwnedModel(fn_1_3424), unlitModel(dayMode ? DATANUM(0x520000, 0) : DATANUM(0x520000, 1)),
      secondModel(dayMode ? DATANUM(0x520000, 2) : DATANUM(0x520000, 3)),
      thirdModel(dayMode ? DATANUM(0x520000, 4) : DATANUM(0x520000, 5)),
      fourthModel(dayMode ? DATANUM(0x520000, 6) : DATANUM(0x520000, 7)),
      fifthModel(dayMode ? DATANUM(0x520000, 8) : DATANUM(0x520000, 9)),
      shadowReceiver(dayMode ? DATANUM(0x520000, 10) : DATANUM(0x520000, 11)),
      seventhModel(dayMode ? DATANUM(0x520000, 14) : DATANUM(0x520000, 15)),
      eighthModel(dayMode ? DATANUM(0x520000, 16) : DATANUM(0x520000, 17)),
      ninthModel(dayMode ? DATANUM(0x520000, 26) : DATANUM(0x520000, 27)),
      revealModel(dayMode ? DATANUM(0x520000, 20) : DATANUM(0x520000, 21)),
      pathModel(DATANUM(0x520000, 33)), interactionSphere(0)
{
    int pair;
    int opacity;
    int index;
    int firstVertex;
    int secondVertex;
    this->enabled = 1;
    this->dayMode = dayMode;
    this->unlitModel.setLightsDynamic(false);
    this->unlitModel.setLayer(0);
    this->unlitModel.setAttribute(0x40000001);
    this->secondModel.setLayer(1);
    this->secondModel.setAttribute(0x40000001);
    this->secondModel.setAttribute(0x4000);
    this->thirdModel.setLayer(1);
    this->thirdModel.setAttribute(0x40000001);
    this->thirdModel.setAttribute(0x4000);
    this->fourthModel.setLayer(1);
    this->fourthModel.setAttribute(0x40000001);
    this->fourthModel.setAttribute(0x4000);
    this->fifthModel.setLayer(1);
    this->fifthModel.setAttribute(0x40000001);
    this->fifthModel.setAttribute(0x4000);
    this->shadowReceiver.setLayer(4);
    this->shadowReceiver.setAttribute(0x40000001);
    this->shadowReceiver.setAttribute(0x4000);
    this->shadowReceiver.setShadowMap();
    this->seventhModel.setLayer(2);
    this->seventhModel.setAttribute(0x40000001);
    this->seventhModel.setAttribute(0x4000);
    this->eighthModel.setLayer(3);
    this->eighthModel.setAttribute(0x40000001);
    this->eighthModel.setAttribute(0x4000);
    this->ninthModel.setLayer(4);
    this->ninthModel.setAttribute(0x40000001);
    this->ninthModel.setAttribute(0x4000);
    fn_8002C2AC(3);
    this->revealModel.setAttribute(0x40000002);
    this->revealModel.setLayer(4);
    this->revealModel.setAttribute(0x4000);
    lbl_1_bss_94 = fn_1_1FA8(DATANUM(0x520000, 55));
    this->setHookData(this);
    this->setLayer(6);
    fn_1_3158(this);
    for (index = 0; index < 128; index++) {
        f32 degrees = 360.0f * ((f32) index / 128.0f) - 180.0f;
        f32 radians = (f32) (M_PI * (f64) degrees / 180.0);
        lbl_1_bss_98[index] = (f32) (20.0 * sin(radians));
    }
    for (pair = 0; pair < 15; pair++) {
        firstVertex = pair * 2;
        secondVertex = pair * 2 + 1;
        opacity = 0xFF;
        if (this->dayMode == 0) {
            opacity = 0x50;
        }
        if (pair == 0 || pair == 14) {
            opacity = 0;
        }
        if (pair == 1 || pair == 13) {
            opacity /= 4;
        }
        if (pair == 2 || pair == 12) {
            opacity /= 2;
        }
        lbl_1_bss_298[firstVertex].opacity = opacity;
        lbl_1_bss_298[secondVertex].opacity = opacity / 4;
        lbl_1_bss_298[firstVertex].x = lbl_1_bss_298[secondVertex].x =
            50.0f + 100.0f * (f32) (7 - pair);
        lbl_1_bss_298[firstVertex].texcoord = lbl_1_bss_298[secondVertex].texcoord =
            (f32) pair / 14.0f;
        f32 degrees = 180.0f * ((f32) pair / 14.0f);
        f32 radians = (f32) (M_PI * (f64) degrees / 180.0);
        lbl_1_bss_298[firstVertex].zOffset = (f32) (300.0 * -sin(radians));
        lbl_1_bss_298[secondVertex].zOffset = (f32) (200.0 * -sin(radians));
    }
    interactionSphere.clearFlag7();
}

// The model hooks that place the fire effects.
char *lbl_1_data_A4[5] = { "fire", "fire1", "fire2", "fire3", "fire4" };

// Releases the strip animation and scene model resources.
SceneModel::~SceneModel()
{
    HuSprAnimKill(lbl_1_bss_94);
    if (lbl_1_bss_90) {
        HuSprAnimKill(lbl_1_bss_90);
    }
}

// Sets attribute 1 on the model.
// Nothing in this minigame calls this.
void fn_1_2C24(Model *model)
{
    model->setAttribute(1);
}

// The center of the scene interaction sphere.
const Vec lbl_1_rodata_D8 = { 46700.0f, 0.0f, 3000.0f };

// Configures the day or night scene model and returns the active model.
Model *fn_1_2C50(void)
{
    SceneModel *scene;
    Vec position;
    int index;
    FireParticle *fire;
    f32 radius;
    scene = &SceneModel::getChecked();
    scene->unlitModel.setAttribute(1);
    scene->secondModel.setAttribute(1);
    scene->thirdModel.setAttribute(1);
    scene->fourthModel.setAttribute(1);
    scene->fifthModel.setAttribute(1);
    scene->shadowReceiver.setAttribute(1);
    scene->seventhModel.setAttribute(1);
    scene->eighthModel.setAttribute(1);
    scene->ninthModel.setAttribute(1);
    scene->enabled = 0;
    if (scene->dayMode != 0) {
        scene->stageModel = new OwnedModel(DATANUM(0x520000, 22));
        scene->shadowModel = new OwnedModel(DATANUM(0x520000, 24));
    } else {
        scene->stageModel = new OwnedModel(DATANUM(0x520000, 23));
        scene->shadowModel = new OwnedModel(DATANUM(0x520000, 25));
    }
    scene->shadowModel->setAttribute(0x40000001);
    scene->shadowModel->setShadow();
    scene->stageModel->setLightsDynamic(false);
    scene->stageModel->setShadowMap();
    scene->interactionSphere = lbl_1_rodata_D8;
    radius = 2000.0f;
    scene->interactionSphere.radius = radius;
    scene->interactionSphere.setFlag7();
    if (scene->dayMode == 0) {
        scene->stageModel->updateMotion();
        lbl_1_bss_90 = fn_1_1FA8(DATANUM(0x520000, 51));
        for (index = 0; index < 5; ++index) {
            fire = new FireParticle(lbl_1_bss_90);
            scene->stageModel->getObjectPosition(lbl_1_data_A4[index], &position);
            fn_80026760(ParticleModelId(fire), position.x, position.y, position.z);
            printf("fire  %f %f %f\n", position.x, position.y, position.z);
        }
    }
    return scene->stageModel;
}

// Submits red and green debug triangles at the scene bounds.
// Nothing in this minigame calls this.
void fn_1_2F64(void)
{
    Vec corners[4];
    corners[3].x = corners[2].x = corners[1].x = corners[0].x = fn_1_1D98();
    corners[0].y = corners[1].y = lbl_1_bss_44 - cmToWorld(500.0f);
    corners[2].y = corners[3].y = lbl_1_bss_44 + cmToWorld(500.0f);
    corners[0].z = corners[3].z = lbl_1_bss_48 - cmToWorld(1000.0f);
    corners[1].z = corners[2].z = lbl_1_bss_48 + cmToWorld(1000.0f);

    fn_1_1C324(&corners[0], &corners[1], &corners[2], 0xFF000088);
    fn_1_1C324(&corners[2], &corners[3], &corners[0], 0xFF000088);

    corners[3].x = corners[2].x = corners[1].x = corners[0].x = fn_1_1E5C();

    fn_1_1C324(&corners[0], &corners[1], &corners[2], 0x00FF0088);
    fn_1_1C324(&corners[2], &corners[3], &corners[0], 0x00FF0088);
}

// Advances the scene model state.
void SceneModel::update()
{
    phase++;
}

// Clears attributes 0x40000002 on the scenery model revealed during play.
void fn_1_30A8()
{
    SceneModel *owner = &SceneModel::getChecked();
    owner->revealModel.resetAttribute(0x40000002);
}

// Acknowledges the scene model request.
void SceneModel::flagEnabled(int request)
{
    flags &= ~(1UL << request);
    switch (request) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        break;
    }
}

extern const Point3d lbl_1_rodata_F0;
// The world up vector.
const Point3d lbl_1_rodata_F0 = { 0.0f, 1.0f, 0.0f };

// Samples model geometry into the scene path positions and rotations.
void fn_1_3158(SceneModel *scene)
{
    Mtx lookAtMatrix;
    Mtx transposedMatrix;
    Point3d modelPosition;
    f32 maxTime;
    f32 sampleTime;
    f32 xDifference;
    f32 yDifference;
    f32 zDifference;
    HU3D_MODELID modelId;
    int sample;
    int segment;

    modelId = scene->pathModel.id();
    maxTime = fn_8002D530(modelId);
    fn_80026A44(modelId, 0x40000002);

    for (sample = 0; sample < 1001; ++sample) {
        sampleTime = (maxTime * (f32) sample) / 1000.0f;
        fn_8002D414(modelId, sampleTime);
        fn_80030644(modelId);
        Hu3DModelObjPosGet(modelId, "grid1", &modelPosition);
        modelPosition.y = 25.0f + fn_1_1ACC(modelPosition.x, modelPosition.z);
        scene->positions[sample] = modelPosition;
    }

    Point3d up = lbl_1_rodata_F0;
    for (segment = 0; segment < 1000; ++segment) {
        xDifference = scene->positions[segment].x - scene->positions[segment + 1].x;
        yDifference = scene->positions[segment].y - scene->positions[segment + 1].y;
        zDifference = scene->positions[segment].z - scene->positions[segment + 1].z;
        if (((zDifference * zDifference) +
             ((xDifference * xDifference) + (yDifference * yDifference))) > 100.0f) {
            scene->visible[segment] = 1;
            Point3d segmentStart = scene->positions[segment];
            Point3d segmentEnd = scene->positions[segment + 1];
            C_MTXLookAt(lookAtMatrix, &segmentStart, &up, &segmentEnd);
            PSMTXTranspose(lookAtMatrix, transposedMatrix);
            C_QUATMtx(&scene->rotations[segment], transposedMatrix);
        } else {
            scene->visible[segment] = 0;
        }
    }
}

// Dispatches the scene model's drawing hook.
void fn_1_3424(HU3D_MODEL *model, Mtx *matrix)
{
    SceneModel *scene = static_cast<SceneModel *>(model->hookData);
    scene->drawMotion(*matrix);
}

// The textured path vertex descriptors.
const GXVtxDescList lbl_1_rodata_100[4] = {
    { GX_VA_POS, GX_DIRECT },
    { GX_VA_CLR0, GX_DIRECT },
    { GX_VA_TEX0, GX_DIRECT },
    { GX_VA_NULL, GX_NONE },
};
// The textured path vertex formats.
const GXVtxAttrFmtList lbl_1_rodata_120[4] = {
    { GX_VA_POS, GX_POS_XYZ, GX_F32, 0 },
    { GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 },
    { GX_VA_TEX0, GX_TEX_ST, GX_F32, 0 },
    { GX_VA_NULL, GX_POS_XY, GX_U8, 0 },
};

// Configures the textured strip render state.
void fn_1_3468(void)
{
    Mtx matrix;
    MtxPtr textureMatrix;

    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
    HuSprTexLoad(lbl_1_bss_94, 0, 0, GX_REPEAT, GX_CLAMP, GX_LINEAR);
    PSMTXIdentity(matrix);
    textureMatrix = matrix;
    GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX2x4);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetCullMode(GX_CULL_BACK);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZCompLoc(GX_TRUE);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 0xFF);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDescv(lbl_1_rodata_100);
    GXSetVtxAttrFmtv(GX_VTXFMT0, lbl_1_rodata_120);
}

// Draws the strip using the supplied phase and distance.
// Nothing in this minigame calls this.
void fn_1_3618(Mtx positionMatrix, int phase, f32 zOffset)
{
    int evenRecordIndex;
    int oddRecordIndex;
    unsigned char firstAlpha;
    unsigned char secondAlpha;
    f32 firstX;
    f32 firstZ;
    f32 firstTexturePosition;
    f32 secondX;
    f32 secondZ;
    f32 secondTexturePosition;

    GXLoadPosMtxImm(positionMatrix, GX_PNMTX0);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 30);

    for (int segment = 0; segment < 15; ++segment) {
        evenRecordIndex = segment * 2;
        oddRecordIndex = (segment * 2) + 1;
        phase = (phase + 15) & 0x7F;

        firstZ = (lbl_1_bss_298[evenRecordIndex].zOffset + lbl_1_bss_98[phase]) - zOffset;
        firstX = lbl_1_bss_298[evenRecordIndex].x;
        GXPosition3f32(firstX, 0.0f, firstZ);
        firstAlpha = lbl_1_bss_298[evenRecordIndex].opacity;
        GXColor4u8(0xFF, 0xFF, 0xFF, firstAlpha);
        firstTexturePosition = lbl_1_bss_298[evenRecordIndex].texcoord;
        GXTexCoord2f32(firstTexturePosition, 1.0f);

        secondZ =
            zOffset + (lbl_1_bss_298[oddRecordIndex].zOffset + lbl_1_bss_98[(phase + 32) & 0x7F]);
        secondX = lbl_1_bss_298[oddRecordIndex].x;
        GXPosition3f32(secondX, 0.0f, secondZ);
        secondAlpha = lbl_1_bss_298[oddRecordIndex].opacity;
        GXColor4u8(0, 0xFF, 0xFF, secondAlpha);
        secondTexturePosition = lbl_1_bss_298[oddRecordIndex].texcoord;
        GXTexCoord2f32(secondTexturePosition, 0.0f);
    }
}

// Invokes the strip render-state setup.
static inline void prepareMotionRender()
{
    fn_1_3468();
}

// Invokes the strip render-state preparation.
static inline void initializeMotionRender()
{
    prepareMotionRender();
}

// Draws the strip after scaling its segment distance.
static inline void drawMotionStrip(Mtx positionMatrix, int phase, f32 zOffset)
{
    int evenRecordIndex;
    int oddRecordIndex;

    GXLoadPosMtxImm(positionMatrix, GX_PNMTX0);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 30);
    for (int segment = 0; segment < 15; ++segment) {
        evenRecordIndex = segment * 2;
        oddRecordIndex = (segment * 2) + 1;
        phase = (phase + 15) & 0x7F;
        GXPosition3f32(lbl_1_bss_298[evenRecordIndex].x, 0.0f,
                       (lbl_1_bss_298[evenRecordIndex].zOffset + lbl_1_bss_98[phase]) -
                           (7.0f * zOffset));
        GXColor4u8(0xFF, 0xFF, 0xFF, lbl_1_bss_298[evenRecordIndex].opacity);
        GXTexCoord2f32(lbl_1_bss_298[evenRecordIndex].texcoord, 1.0f);
        GXPosition3f32(lbl_1_bss_298[oddRecordIndex].x, 0.0f,
                       (7.0f * zOffset) + (lbl_1_bss_298[oddRecordIndex].zOffset +
                                           lbl_1_bss_98[(phase + 32) & 0x7F]));
        GXColor4u8(0, 0xFF, 0xFF, lbl_1_bss_298[oddRecordIndex].opacity);
        GXTexCoord2f32(lbl_1_bss_298[oddRecordIndex].texcoord, 0.0f);
    }
}

// Draws visible path segments around the current camera position.
void SceneModel::drawMotion(Mtx modelMatrix)
{
    Mtx inverseMatrix;
    Mtx localMatrix;
    int segment;
    int phase;
    f32 cameraXDifference;
    f32 absoluteDifference;
    f32 segmentDistance;
    f32 xDifference;
    f32 zDifference;
    if (this->enabled == 0) {
        return;
    }
    initializeMotionRender();
    Point3d cameraOrigin = { 0.0f, 0.0f, 0.0f };
    Point3d cameraPosition;
    PSMTXInverse(modelMatrix, inverseMatrix);
    PSMTXMultVec(inverseMatrix, &cameraOrigin, &cameraPosition);
    segment = this->phase & 0xF;
    for (; segment < 999; segment += 16) {
        phase = this->phase + (segment * 4);
        if (this->visible[segment] != 0) {
            cameraXDifference = cameraPosition.x - this->positions[segment].x;
            if (cameraXDifference < 0.0f) {
                absoluteDifference = -cameraXDifference;
            } else {
                absoluteDifference = cameraXDifference;
            }
            if (absoluteDifference < 10000.0f) {
                PSMTXQuat(localMatrix, &this->rotations[segment]);
                PSMTXTransApply(localMatrix, localMatrix, this->positions[segment].x,
                                this->positions[segment].y, this->positions[segment].z);
                PSMTXConcat(modelMatrix, localMatrix, localMatrix);
                segmentDistance = this->positions[segment + 1].x - this->positions[segment].x;
                xDifference = segmentDistance;
                segmentDistance = this->positions[segment + 1].z - this->positions[segment].z;
                zDifference = segmentDistance;
                segmentDistance = sqrtf(xDifference * xDifference + zDifference * zDifference);
                drawMotionStrip(localMatrix, phase, segmentDistance);
            }
        }
    }
}
