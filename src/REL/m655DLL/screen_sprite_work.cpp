#include "REL/framework/vector3.h"
#include "REL/framework/frame_interpolator.h"
#include "REL/m655DLL/game_constants.h"
#include "REL/m655DLL/screen_sprite_work.h"

#include "REL/m655DLL/screen_sprite_api.h"
#include <memory>
#include "REL/framework/frame_time.h"
#include "REL/m655DLL/screen_sprite_singleton.h"
float RandomFloat(float minimum, float maximum);
#include "humath.h"
/* Screen distortion and particles drifting across it. */
extern "C" {
#include "dolphin/types.h"
}
#include "dolphin/mtx.h"
#include "dolphin/gx.h"
extern "C" {
#include "dolphin/os.h"
}
#include "REL/framework/model.h"
#include "game/memory.h"
extern "C" {
#include "game/data.h"
}
extern "C" {
#include "game/hu3d.h"
}
extern "C" {
#include "game/sprite.h"
}
extern "C" {
#include "game/audio.h"
}
#undef offsetof
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <list>
#include "REL/framework/game_object.h"

#include "REL/framework/game_object2.h"
using std::abort;

extern "C" {

#include "game/sprput.h"
}

/* The module's statics: the frame count and gravity every unit derives, the camera drift, the
 * capture request and the screen object's own singleton pointer. */
#include "REL/framework/world_units.h"

struct ScreenSpriteWork;

extern "C" {
Vec lbl_1_bss_8;
u8 lbl_1_bss_14;
}

/* One particle: where it started, where it is drawn, its size and the frames it has left. */

struct ParticleOffset {
    Vec value;
    ParticleOffset() {}
    ParticleOffset &operator+=(const Vec &other)
    {
        PSVECAdd(&other, &value, &value);
        return *this;
    }
};

static inline f32 interpolatePoint(f64 from, f64 to, const f64 &weight)
{
    return (f32)(((1.0 - weight) * from) + (weight * to));
}

/* Signed-character capture buffer. */

/* Module C++ callback declarations. */
bool fn_1_0(ScreenParticle *payload);
void fn_1_BA8(ScreenSpriteWork *work);
void fn_1_E40(ScreenSpriteWork *owner);
void fn_1_1074(ScreenSpriteWork *work);
void fn_1_10F4(ScreenSpriteWork *work);
void fn_1_125C(ScreenSpriteWork *work);
void fn_1_151C(ScreenSpriteWork *work, Mtx matrix);
void fn_1_1744(ScreenParticle *quad);
/* End callback declarations. */

bool fn_1_0(ScreenParticle *payload)
{
    f64 weight;
    payload->position = payload->origin;
    PSVECAdd(&lbl_1_bss_8, &payload->origin, &payload->origin);
    weight = secondsToFrames(0.5);
    payload->position.x = interpolatePoint(payload->origin.x, payload->position.x, weight);
    payload->position.y = interpolatePoint(payload->origin.y, payload->position.y, weight);
    payload->position.z = interpolatePoint(payload->origin.z, payload->position.z, weight);
    return --payload->remainingFrames > 0;
}

ScreenSpriteWork::ScreenSpriteWork()
    : RenderObject(2),
      captureBuffer(GXGetTexBufferSize(0x280, 0x1E0, GX_TF_RGB565, 0, 0)),
      transition(0.0f), traveledDistance(0.0f),
      spriteTexture(HuSprAnimRead(fn_80008DD0(DATANUM(0x720000, 30), 0x10000000, 2)))
{
    f32 viewPerspective;

    setLayer(2);
    {
        GXInitTexObj(&texture, captureBuffer.begin(), 0x280, 0x1E0, GX_TF_RGB565, GX_CLAMP,
                     GX_CLAMP, 0);
    }
    fn_80027D8C(1, &perspective, &viewPerspective, &viewPerspective);
    lbl_1_bss_14 = 1;
    lbl_1_bss_8.x = 0.0f;
    lbl_1_bss_8.y = 0.0f;
    lbl_1_bss_8.z = 0.0f;
}

ScreenSpriteWork::~ScreenSpriteWork()
{
}

void fn_1_884(ScreenSpriteWork *owner, float value)
{
    owner->transition.set(value, (int)secondsToFrames(0.1));
}

void ScreenSpriteWork::update()
{
    f32 stepDistance;

    transition.update();
    fn_1_BA8(this);
    stepDistance = 10.0f;
    while (traveledDistance >= stepDistance) {
        traveledDistance -= stepDistance;
        if ((float)transition > 0.0) {
            fn_1_E40(this);
        }
    }
    std::list<ScreenParticle>::iterator current = motions.begin();
    std::list<ScreenParticle>::iterator end = motions.end();
    while (current != end) {
        if (!fn_1_0(&*current)) {
            motions.erase(current);
        }
        ++current;
    }
    lbl_1_bss_14 = 1;
}

void fn_1_BA8(ScreenSpriteWork *work)
{
    Vec previousPosition = work->position;
    Vector3 cameraVector;

    fn_80027CD0(1, &work->position, &cameraVector, &cameraVector);
    if ((float)work->transition <= 0.0) {
        work->traveledDistance = 0.0f;
        PSVECScale(&cameraVector, &lbl_1_bss_8, -1.0f);
        return;
    }
    PSVECSubtract(&work->position, &previousPosition, &cameraVector);
    work->traveledDistance += cameraVector.length();
    PSVECScale(&cameraVector, &cameraVector, -1.0f);
    lbl_1_bss_8.x = weightedHeight(lbl_1_bss_8.x, cameraVector.x);
    lbl_1_bss_8.y = weightedHeight(lbl_1_bss_8.y, cameraVector.y);
    lbl_1_bss_8.z = weightedHeight(lbl_1_bss_8.z, cameraVector.z);
}

void fn_1_E40(ScreenSpriteWork *owner)
{
    f64 randomX;
    f64 randomY;
    f64 randomZ;
    f32 coordinateScale;
    ParticleOffset offset;

    coordinateScale = 1200.0f;
    randomZ = coordinateScale * RandomFloat(-1.0f, 1.0f);
    randomY = coordinateScale * RandomFloat(-1.0f, 1.0f);
    randomX = coordinateScale * RandomFloat(-1.0f, 1.0f);
    offset.value.x = randomX;
    offset.value.y = randomY;
    offset.value.z = randomZ;
    ScreenParticle element((offset += owner->position).value);
    owner->motions.push_back(element);
}

void ScreenSpriteWork::draw(Mtx *matrix)
{
    f32 value = transition.value;
    if (value > 0.0) {
        fn_1_10F4(this);
        fn_1_125C(this);
        fn_1_151C(this, *matrix);
    }
    if (lbl_1_bss_14 != 0) {
        lbl_1_bss_14 = 0;
        fn_1_1074(this);
    }
}

void fn_1_1074(ScreenSpriteWork *work)
{
    GXSetTexCopySrc(0, 0, 0x280, 0x1E0);
    GXSetTexCopyDst(640, 480, GX_TF_RGB565, 0);
    {
        GXCopyTex(work->captureBuffer.begin(), 0);
    }
    GXPixModeSync();
}

static const GXColor spriteTevColor = {9, 9, 9, 0};
static const GXVtxDescList spriteVertexDescriptors[] = {
    {GX_VA_POS, GX_DIRECT},
    {GX_VA_TEX0, GX_DIRECT},
    {GX_VA_NULL, GX_NONE},
};
static const GXVtxAttrFmtList spriteVertexFormats[] = {
    {GX_VA_POS, GX_POS_XYZ, GX_F32, 0},
    {GX_VA_TEX0, GX_TEX_ST, GX_F32, 0},
    {GX_VA_NULL, (GXCompCnt)0, (GXCompType)0, 0},
};

void fn_1_10F4(ScreenSpriteWork *work)
{
    GXColor tevColor = spriteTevColor;
    f32 alpha = work->transition.value;
    tevColor.a = (u8)(232.0 * alpha);
    GXSetTevColor(GX_TEVREG0, tevColor);
    GXSetNumChans(0);
    GXLoadTexObj(&work->texture, GX_TEXMAP0);
    HuSprTexLoad((ANIMDATA *)work->spriteTexture, 0, 1, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    GXSetCullMode(GX_CULL_NONE);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZCompLoc(GX_TRUE);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 0xFF);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
                   GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDescv(spriteVertexDescriptors);
    GXSetVtxAttrFmtv(GX_VTXFMT0, spriteVertexFormats);
}

void fn_1_125C(ScreenSpriteWork *work)
{
    f32 projection[3][4];
    f32 halfWidth;
    f32 halfHeight;
    f32 tangent;
    f32 (*positionMatrix)[4];

    PSMTXIdentity(projection);
    positionMatrix = projection;
    GXLoadPosMtxImm(positionMatrix, GX_PNMTX0);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    tangent = (f32)tan((M_PI *
            (f64)(work->perspective / 2.0f)) /
            180.0);
    halfWidth = 1.2f * tangent;
    halfHeight = tangent;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-halfWidth, halfHeight, -1.0f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(halfWidth, halfHeight, -1.0f);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition3f32(halfWidth, -halfHeight, -1.0f);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(-halfWidth, -halfHeight, -1.0f);
    GXTexCoord2f32(0.0f, 1.0f);
}

static const f32 spriteDistortionMatrix[2][3] = {
    {0.02f, 0.0f, 0.0f},
    {0.0f, -0.02f, 0.0f}
};

void fn_1_151C(ScreenSpriteWork *work, Mtx matrix)
{
    f32 lightMatrix[3][4];

    GXLoadPosMtxImm(matrix, GX_PNMTX0);
    C_MTXLightPerspective(lightMatrix, work->perspective, 1.2f,
                          0.5f, -0.5f, 0.5f, 0.5f);
    PSMTXConcat(lightMatrix, matrix, lightMatrix);
    GXLoadTexMtxImm(lightMatrix, 0x1E, GX_MTX3x4);
    GXSetNumTexGens(2);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0,
                      GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,
                      GX_FALSE, GX_PTIDENTITY);
    GXSetIndTexMtx(GX_ITM_0, spriteDistortionMatrix, 0);
    GXSetNumIndStages(1);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetNumTevStages(1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_TRUE, GX_FALSE,
                    GX_ITM_0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_ZERO, GX_CC_ZERO,
                    GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_A0);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_FALSE, GX_TEVPREV);
    std::list<ScreenParticle>::iterator entry = work->motions.begin();
    std::list<ScreenParticle>::iterator end = work->motions.end();
    while (entry != end) {
        fn_1_1744(&*entry);
        ++entry;
    }
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetNumIndStages(0);
}

static inline void spritePosition(const f32 *coordinates, s32 index)
{
    GXPosition3f32(coordinates[index], coordinates[index + 1],
                    coordinates[index + 2]);
}

void fn_1_1744(ScreenParticle *quad)
{
    f32 vertexCoordinates[12];
    f64 leftX;
    f64 leftY;
    f64 leftZ;
    f64 lowerX;
    f64 lowerY;
    f64 lowerZ;
    f64 upperX;
    f64 upperY;
    f64 upperZ;
    f64 rightX;
    f64 rightY;
    f64 rightZ;

    leftZ = quad->origin.z;
    leftY = quad->origin.y;
    leftX = quad->origin.x - quad->extent;
    vertexCoordinates[0] = (f32) leftX;
    vertexCoordinates[1] = (f32) leftY;
    vertexCoordinates[2] = (f32) leftZ;
    lowerZ = quad->position.z;
    lowerY = quad->position.y;
    lowerX = quad->position.x - quad->extent;
    vertexCoordinates[3] = (f32) lowerX;
    vertexCoordinates[4] = (f32) lowerY;
    vertexCoordinates[5] = (f32) lowerZ;
    upperZ = quad->position.z;
    upperY = quad->position.y;
    upperX = quad->position.x + quad->extent;
    vertexCoordinates[6] = (f32) upperX;
    vertexCoordinates[7] = (f32) upperY;
    vertexCoordinates[8] = (f32) upperZ;
    rightZ = quad->origin.z;
    rightY = quad->origin.y;
    rightX = quad->origin.x + quad->extent;
    vertexCoordinates[9] = (f32) rightX;
    vertexCoordinates[10] = (f32) rightY;
    vertexCoordinates[11] = (f32) rightZ;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    spritePosition(vertexCoordinates, 0);
    GXTexCoord2f32(0.0f, 0.0f);
    spritePosition(vertexCoordinates, 3);
    GXTexCoord2f32(0.0f, 1.0f);
    spritePosition(vertexCoordinates, 6);
    GXTexCoord2f32(1.0f, 1.0f);
    spritePosition(vertexCoordinates, 9);
    GXTexCoord2f32(1.0f, 0.0f);
}
