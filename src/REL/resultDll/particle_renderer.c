/* Builds and draws the animated particle effects shown on result screens. */
#define RESULT_FLOAT_SIGNLESS_MASK 0x7FFFFFFF
#define RESULT_DISPLAY_LIST_CAPACITY 131072
#define RESULT_DATA_ID_MASK 0xFFFF0000
#define RESULT_DATA_ID_PATTERN DATANUM(DATA_result, 0)
#define RESULT_PARTICLE_ORIENT_TO_WORLD_FLAG (1 << 4)
#define RESULT_PARTICLE_ANIMATE_FLAG (1 << 3)
#define RESULT_PARTICLE_LOOP_ANIMATION_FLAG (1 << 0)
#define RESULT_PARTICLE_NO_COUNT_FLAG (1 << 1)

#include "dolphin/math.h"
#include "game/hu3d.h"
typedef struct ResultDrawSortEntry {
    s32 key; /* Signed integer key preserving the numerical order of particle depth. */
    s32 index; /* Particle's position in the unsorted renderer array. */
} ResultDrawSortEntry;
extern ResultDrawSortEntry *lbl_1_bss_14A8;

#include "game/memory.h"

#include "game/memory.h"
#include "string.h"
#include "dolphin/math.h"
#include "game/hu3d.h"
#include "game/animdata.h"

#include "game/memory.h"
#include "dolphin/os/OSCache.h"

/* Allocates the model's vertex and texture coordinate buffers. */
static inline void *ResultBufferAlloc(s32 size, u32 allocationNo)
{
    return HuMemDirectMallocNum(HEAP_MODEL, size, allocationNo);
}

/* Makes a model-owned buffer visible to the graphics processor. */
static inline void *ResultDisplayListAlloc(s32 size, u32 allocationNo)
{
    void *buffer;

    buffer = HuMemDirectMallocNum(HEAP_MODEL, size, allocationNo);
    DCFlushRange(buffer, size);
    return buffer;
}

typedef struct ResultParticleEntry {
    s16 time;
    s16 state;
    s16 camera;
    s16 cameraBit;
    Vec velocity;
    f32 scaleSpeed;
    f32 verticalAccel;
    f32 speedDecay;
    f32 driftSpeed;
    f32 driftAngle;
    f32 spinSpeed;
    f32 scale;
    f32 rotationX;
    f32 rotationY;
    f32 zRot;
    Vec pos;
    u32 color;
    u8 animationState[12];
    u8 visible : 1;
    u8 renderFlags : 7;
    u8 textureState[3];
} ResultParticleEntry;

typedef struct ResultParticleRenderer {
    s16 state;
    s16 group;
    Vec pos;
    Vec spawnCenter;
    void *work;
    s8 blendMode;
    u8 attributes;
    s16 sortEnabled;
    s16 modelId;
    s16 particleCount;
    s32 count;
    s32 prevCounter;
    s32 countLimit;
    GXTevColorArg colorInput[4];
    GXTevAlphaArg alphaInput[4];
    s32 color0;
    s32 color1;
    u32 displayListSize;
    ANIMDATA *anim;
    HuVec2f *patternCoords;
    ResultParticleEntry *particles;
    Vec *vertices;
    HuVec2f *textureCoords;
    void *colorBuffer;
    void *displayList;
    void *callback;
} ResultParticleRenderer;

extern void fn_1_B5D8(HU3D_MODEL *model, Mtx *matrix);

/* Creates the result effect's particle pool and indexed quad display list. */

#include "game/memory.h"
#include "dolphin/os.h"

extern u32 lbl_1_bss_14AC;

#include "game/memory.h"
#include "dolphin/os.h"

#include "dolphin/math.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/sprite.h"

extern s32 lbl_1_bss_1530[32];
extern s32 lbl_1_bss_14B0[32];

/* Sorts result entries in ascending key order, then finishes short ranges by insertion. */

/* Draws the animated, optionally depth-sorted particles used by result effects. */
#include "dolphin/math.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/animdata.h"

typedef struct ResultDrawParticle {
    s16 time;
    s16 state;
    s16 attributes;
    s16 cameraBit;
    Vec velocity;
    f32 scaleSpeed;
    f32 verticalAccel;
    f32 speedDecay;
    f32 driftSpeed;
    f32 driftAngle;
    f32 spinSpeed;
    f32 scale;
    f32 rotationX;
    f32 rotationY;
    f32 rotationZ;
    Vec pos;
    GXColor color;
    s16 animBank;
    s16 animNo;
    f32 animSpeed;
    f32 animTime;
    u8 visible : 1;
    u8 animStopped : 1;
    u8 renderFlags : 6;
    u8 textureState[3];
} ResultDrawParticle;

typedef struct ResultDrawRenderer ResultDrawRenderer;
typedef void (*ResultDrawHook)(HU3D_MODEL *, ResultDrawRenderer *, Mtx);

struct ResultDrawRenderer {
    s16 state;
    s16 group;
    Vec pos;
    Vec spawnCenter;
    void *work;
    u8 blendMode;
    u8 attributes;
    s16 sortEnabled;
    s16 modelId;
    s16 particleCount;
    u32 count;
    u32 prevCounter;
    u32 countLimit;
    GXTevColorArg colorInput[4];
    GXTevAlphaArg alphaInput[4];
    GXColor color0;
    GXColor color1;
    u32 displayListSize;
    ANIMDATA *anim;
    HuVec2f *patternCoords;
    ResultDrawParticle *particles;
    Vec *vertices;
    HuVec2f *textureCoords;
    GXColor *colors;
    void *displayList;
    ResultDrawHook hook;
};

extern Vec lbl_1_data_8E8[4];
extern Vec lbl_1_data_918[4];
extern f32 lbl_1_data_8E0[2];
extern u32 minimumVcount;
extern u32 GlobalCounter;
void fn_1_B404(ResultDrawSortEntry *entries, s32 entryCount);
void HuSprTexLoad(ANIMDATA *anim, s16 bmpNo, s16 texMapId, GXTexWrapMode wrapS,
    GXTexWrapMode wrapT, GXTexFilter filter);

/* Converts a finite depth into a signed integer key with the same numerical ordering. */
static inline void ResultDepthKeySet(s32 *key, f32 depth)
{
    union {
        f32 depth;
        s32 bits;
    } representation;
    f32 value;
    s32 bits;

    value = depth;
    representation.depth = value;
    bits = representation.bits;
    if (bits < 0) {
        bits = -(bits & RESULT_FLOAT_SIGNLESS_MASK);
    }
    *key = bits;
}

/* The particle's depth along the camera's view axis: its position scaled by the camera's z axis,
 * summed. */
static inline f32 ResultParticleDepth(Vec *position, f32 axisX, f32 axisY, f32 axisZ)
{
    f32 x;
    f32 y;
    f32 z;

    x = position->x;
    y = position->y;
    z = position->z;
    x *= axisX;
    y *= axisY;
    z *= axisZ;
    return z + (x + y);
}

/* Applies the scale and translation to a particle's four corners, two values at a time: each
 * corner coordinate times its scale plus its translation, as the corners interleave x, y and z. */
static inline void ResultQuadScaleAdd(register Vec *vertices, Vec *quadCorners, Vec *position,
                                      f32 size)
{
    f32 translationValues[4];
    f32 scaleValues[2];
    register Vec *corners;
    register f32 *translation;
    register f32 *scale;
    register f32 translationXY;
    register f32 translationZW;
    register f32 translationYZ;
    register f32 scalePair;
    register f32 first;
    register f32 second;
    register f32 third;

    translationValues[0] = translationValues[3] = position->x;
    translationValues[1] = position->y;
    translationValues[2] = position->z;
    scaleValues[0] = scaleValues[1] = size;
    corners = quadCorners;
    translation = translationValues;
    scale = scaleValues;
    asm {
        psq_l scalePair, 0(scale), 0, 0
        psq_l translationXY, 0(translation), 0, 0
        psq_l translationZW, 8(translation), 0, 0
        psq_l translationYZ, 4(translation), 0, 0
        psq_l first, 0(corners), 0, 0
        psq_l second, 8(corners), 0, 0
        psq_l third, 16(corners), 0, 0
        ps_madd first, first, scalePair, translationXY
        ps_madd second, second, scalePair, translationZW
        ps_madd third, third, scalePair, translationYZ
        psq_st first, 0(vertices), 0, 0
        psq_st second, 8(vertices), 0, 0
        psq_st third, 16(vertices), 0, 0
        psq_l first, 24(corners), 0, 0
        psq_l second, 32(corners), 0, 0
        psq_l third, 40(corners), 0, 0
        ps_madd first, first, scalePair, translationXY
        ps_madd second, second, scalePair, translationZW
        ps_madd third, third, scalePair, translationYZ
        psq_st first, 24(vertices), 0, 0
        psq_st second, 32(vertices), 0, 0
        psq_st third, 40(vertices), 0, 0
    }
}

/* Hidden particles keep a collapsed quad in the vertex buffer: one pair of values stored over all
 * four corners. */
static inline void ResultQuadClear(register Vec *vertices)
{
    register f32 *zero = lbl_1_data_8E0;
    register f32 pair;

    asm {
        psq_l pair, 0(zero), 0, 0
        psq_st pair, 0(vertices), 0, 0
        psq_st pair, 8(vertices), 0, 0
        psq_st pair, 16(vertices), 0, 0
        psq_st pair, 24(vertices), 0, 0
        psq_st pair, 32(vertices), 0, 0
        psq_st pair, 40(vertices), 0, 0
    }
}

/* Copies a complete sprite pattern's texture coordinates into its draw buffer, a coordinate pair at
 * a time. */
static inline void ResultQuadTexCopy(register HuVec2f *textureCoords,
    register HuVec2f *patternCoords)
{
    register f32 first;
    register f32 second;
    register f32 third;
    register f32 fourth;

    asm {
        psq_l first, 0(patternCoords), 0, 0
        psq_l second, 8(patternCoords), 0, 0
        psq_l third, 16(patternCoords), 0, 0
        psq_l fourth, 24(patternCoords), 0, 0
        psq_st first, 0(textureCoords), 0, 0
        psq_st second, 8(textureCoords), 0, 0
        psq_st third, 16(textureCoords), 0, 0
        psq_st fourth, 24(textureCoords), 0, 0
    }
}

/* Model hook updates sprite frames, builds visible quads, and submits their indexed draw list. */

#include "dolphin/math.h"
#include "game/hu3d.h"
#include "game/gamework.h"

extern u32 lbl_1_data_968[6];

/* Allocates general-heap storage for result-screen work buffers. */
void *fn_1_AB3C(s32 size) {
    return HuMemDirectMallocNum(HEAP_HEAP, size, HU_MEMNUM_OVL);
}

/* Returns a model-heap buffer tagged with the supplied memory number. */
void *fn_1_AB6C(s32 size, u32 allocationNo)
{
    return HuMemDirectMallocNum(HEAP_MODEL, size, allocationNo);
}

/* Clears a model-owned result-effect buffer before it is used. */
void *fn_1_ABA0(s32 size, u32 allocationNo)
{
    void *buffer;

    buffer = HuMemDirectMallocNum(HEAP_MODEL, size, allocationNo);
    memset(buffer, 0, size);
    return buffer;
}

/* Allocates and flushes a result-effect buffer for graphics processor reads. */
void *fn_1_ABFC(s32 size, u32 allocationNo) {
    void *buffer;

    buffer = HuMemDirectMallocNum(2, size, allocationNo);
    DCFlushRange(buffer, size);
    return buffer;
}

/* Initializes result-effect sorting storage when the results overlay starts. */
void fn_1_AC54(void)
{
    void *buffer = HuMemDirectMallocNum(HEAP_HEAP, 16384, HU_MEMNUM_OVL);

    lbl_1_bss_14A8 = buffer;
    lbl_1_bss_14AC = 0;
    OSReport("------------------ResultEffectInit-------------------\n");
}

/* Frees result-effect sorting storage when the results overlay closes. */
void fn_1_ACB8(void) {
    if (lbl_1_bss_14A8 != 0) {
        void *data = (void *)lbl_1_bss_14A8;
        HuMemDirectFree(data);
        lbl_1_bss_14A8 = 0;
    }
    OSReport("------------------ResultEffectClose-------------------\n");
}

/* Creates the result particle model, buffers, sprite coordinates, and indexed quad list. */
s16 fn_1_AD24(ANIMDATA *anim, s16 particleCount)
{
    s16 modelId;
    s16 bitmapFormat;
    s16 particleIndex;
    HU3D_MODEL *model;
    ResultParticleRenderer *renderer;
    ResultParticleEntry *particle;
    ANIMLAYER *layer;
    ANIMBMP *bitmap;
    HuVec2f *coords;
    void *displayList;
    Vec *vertices;
    f32 widthScale;
    f32 heightScale;
    s32 vertexIndex;

    modelId = Hu3DHookFuncCreate(fn_1_B5D8);
    Hu3DModelCameraSet(modelId, 1);
    model = &Hu3DData[modelId];
    model->hookData = renderer = fn_1_ABA0(sizeof(ResultParticleRenderer), model->mallocNo);
    renderer->anim = anim;
    anim->useNum++;
    renderer->particleCount = particleCount;
    renderer->blendMode = 0;
    renderer->prevCounter = -1;
    renderer->modelId = modelId;
    bitmapFormat = renderer->anim->bmp->dataFmt & ANIM_BMP_FMTMASK;
    if (bitmapFormat == ANIM_BMP_I8 || bitmapFormat == ANIM_BMP_I4) {
        renderer->colorInput[0] = GX_CC_ZERO;
        renderer->colorInput[1] = GX_CC_ONE;
        renderer->colorInput[2] = GX_CC_RASC;
        renderer->colorInput[3] = GX_CC_ZERO;
    } else {
        renderer->colorInput[0] = GX_CC_ZERO;
        renderer->colorInput[1] = GX_CC_TEXC;
        renderer->colorInput[2] = GX_CC_RASC;
        renderer->colorInput[3] = GX_CC_ZERO;
    }
    renderer->alphaInput[0] = GX_CA_ZERO;
    renderer->alphaInput[1] = GX_CA_TEXA;
    renderer->alphaInput[2] = GX_CA_RASA;
    renderer->alphaInput[3] = GX_CA_ZERO;
    renderer->particles = particle = fn_1_ABA0(particleCount * sizeof(ResultParticleEntry),
        model->mallocNo);
    for (particleIndex = 0; particleIndex < particleCount; particleIndex++, particle++) {
        particle->cameraBit = -1;
        particle->color = -1;
        particle->visible = 1;
    }
    renderer->vertices = vertices = ResultBufferAlloc(particleCount * sizeof(Vec) * 4,
        model->mallocNo);
    renderer->textureCoords = coords = ResultBufferAlloc(particleCount * 8 * 4,
        model->mallocNo);
    renderer->patternCoords = ResultBufferAlloc(anim->patNum * 8 * 4, model->mallocNo);
    for (particleIndex = 0; particleIndex < anim->patNum; particleIndex++) {
        layer = anim->pat[particleIndex].layer;
        bitmap = &anim->bmp[layer->bmpNo];
        widthScale = 1.0f / bitmap->sizeX;
        heightScale = 1.0f / bitmap->sizeY;
        coords = &renderer->patternCoords[particleIndex * 4];
        coords[0].x = widthScale * (layer->startX + layer->vtx[0]);
        coords[0].y = heightScale * (layer->startY + layer->vtx[1]);
        coords[1].x = widthScale * (layer->startX + layer->vtx[2]);
        coords[1].y = heightScale * (layer->startY + layer->vtx[3]);
        coords[2].x = widthScale * (layer->startX + layer->vtx[4]);
        coords[2].y = heightScale * (layer->startY + layer->vtx[5]);
        coords[3].x = widthScale * (layer->startX + layer->vtx[6]);
        coords[3].y = heightScale * (layer->startY + layer->vtx[7]);
    }
    renderer->displayList = displayList = ResultDisplayListAlloc(particleCount * 96 + 128,
        model->mallocNo);
    /* The recorder receives a fixed capacity, while the buffer size depends on particle count. */
    GXBeginDisplayList(displayList, RESULT_DISPLAY_LIST_CAPACITY);
    GXBegin(GX_QUADS, GX_VTXFMT0, particleCount * 4);
    for (particleIndex = 0; particleIndex < particleCount; particleIndex++) {
        vertexIndex = particleIndex * 4;
        GXPosition1x16(vertexIndex);
        GXColor1x16(particleIndex);
        GXTexCoord1x16(vertexIndex);
        GXPosition1x16(vertexIndex + 1);
        GXColor1x16(particleIndex);
        GXTexCoord1x16(vertexIndex + 1);
        GXPosition1x16(vertexIndex + 2);
        GXColor1x16(particleIndex);
        GXTexCoord1x16(vertexIndex + 2);
        GXPosition1x16(vertexIndex + 3);
        GXColor1x16(particleIndex);
        GXTexCoord1x16(vertexIndex + 3);
    }
    renderer->displayListSize = GXEndDisplayList();
    return modelId;
}

/* Releases a result particle model's animation and model-owned allocations. */
void fn_1_B39C(s16 modelId)
{
    HU3D_MODEL *model = &Hu3DData[modelId];
    ResultParticleRenderer *hookData = model->hookData;

    HuSprAnimKill(hookData->anim);
    HuMemDirectFreeNum(HEAP_MODEL, model->mallocNo);
    model->hsf = NULL;
}

/* Sorts particles by depth, using quicksort for broad ranges and insertion for short ranges. */
void fn_1_B404(ResultDrawSortEntry *entries, s32 entryCount)
{
    s32 savedIndex;
    s32 movedKey;
    s32 movedIndex;
    u8 *startStack = (u8 *)lbl_1_bss_1530;
    u8 *endStack = (u8 *)lbl_1_bss_14B0;
    s32 pivot;
    s32 rightValue;
    s32 leftValue;
    s32 start;
    s32 end;
    s32 stackOffset;
    s32 right;
    s32 left;
    ResultDrawSortEntry *leftEntry;
    ResultDrawSortEntry *rightEntry;

    start = 0;
    end = entryCount - 1;
    /* Both pending-range stacks use the same byte offset; each stored index occupies four bytes. */
    stackOffset = 0;
    for (;;) {
        if (end - start <= 10) {
            if (stackOffset == 0) {
                break;
            }
            stackOffset -= sizeof(s32);
            start = *(s32 *)(startStack + stackOffset);
            end = *(s32 *)(endStack + stackOffset);
        }
        pivot = entries[(start + end) >> 1].key;
        left = start;
        right = end;
        leftEntry = &entries[left];
        rightEntry = &entries[right];
        for (;;) {
            while (leftEntry->key < pivot) {
                left++;
                leftEntry++;
            }
            while (pivot < rightEntry->key) {
                right--;
                rightEntry--;
            }
            if (left >= right) {
                break;
            }
            leftValue = leftEntry->index;
            rightValue = rightEntry->index;
            rightEntry->index = leftValue;
            leftEntry->index = rightValue;
            leftValue = leftEntry->key;
            rightValue = rightEntry->key;
            rightEntry->key = leftValue;
            leftEntry->key = rightValue;
            left++;
            right--;
            leftEntry++;
            rightEntry--;
        }
        if (left - start > end - right) {
            if (left - start > 10) {
                *(s32 *)(startStack + stackOffset) = start;
                *(s32 *)(endStack + stackOffset) = left - 1;
                stackOffset += sizeof(s32);
            }
            start = right + 1;
        } else {
            if (end - right > 10) {
                *(s32 *)(startStack + stackOffset) = right + 1;
                *(s32 *)(endStack + stackOffset) = end;
                stackOffset += sizeof(s32);
            }
            end = left - 1;
        }
    }
    for (left = 1, leftEntry = &entries[left]; left < entryCount; left++, leftEntry++) {
        pivot = leftEntry->key;
        savedIndex = leftEntry->index;
        right = left - 1;
        rightEntry = leftEntry - 1;
        while (right >= 0 && entries[right].key > pivot) {
            movedIndex = rightEntry->index;
            movedKey = rightEntry->key;
            rightEntry[1].index = movedIndex;
            rightEntry[1].key = movedKey;
            right--;
            rightEntry--;
        }
        rightEntry++;
        rightEntry->key = pivot;
        rightEntry->index = savedIndex;
    }
}

/* Updates and draws result particles from the model hook on each active frame. */
void fn_1_B5D8(HU3D_MODEL *model, Mtx *matrix)
{
    Mtx inverse;
    Mtx rotation;
    Vec rotatedCorners[4];
    Vec cameraCorners[4];
    ROMtx reordered;
    Vec *vertices;
    u32 animationStep;
    ResultDrawHook hook;
    ANIMDATA *anim;
    s32 orientToWorld;
    HuVec2f *textureCoords;
    ResultDrawSortEntry *entry;
    f32 cameraX;
    f32 cameraY;
    f32 cameraZ;
    union {
        f32 depth;
        s32 bits;
    } representation;
    s32 bits;
    s32 particleCount;
    Vec *corners;
    s32 index;
    ResultDrawRenderer *renderer;
    ResultDrawParticle *particle;
    ANIMBANK *bank;
    ANIMFRAME *frame;
    Vec *rotatingCorners;
    f32 sine;
    f32 cosine;

    renderer = model->hookData;
    anim = renderer->anim;
    if (HmfInverseMtxF3X3(*matrix, inverse) == 0) {
        PSMTXIdentity(inverse);
    }
    PSMTXReorder(inverse, reordered);
    if (Hu3DPauseF == 0 || (model->attr & HU3D_ATTR_NOPAUSE) != 0) {
        /* A recorded counter suppresses repeated hook calls; sprite animation below still advances
         * on every eligible draw. */
        if (renderer->hook && renderer->prevCounter != GlobalCounter) {
            hook = renderer->hook;
            hook(model, renderer, *matrix);
        }
    } else if (renderer->prevCounter == -1) {
        return;
    }
    if (renderer->sortEnabled != 0 && renderer->colors) {
        particleCount = renderer->particleCount;
        cameraX = inverse[0][2];
        cameraY = inverse[1][2];
        cameraZ = inverse[2][2];
        particle = renderer->particles;
        entry = lbl_1_bss_14A8;
        for (index = 0; index < particleCount; index++, particle++, entry++) {
            representation.depth = ResultParticleDepth(&particle->pos, cameraX, cameraY, cameraZ);
            bits = representation.bits;
            if (bits < 0) {
                bits = -(bits & RESULT_FLOAT_SIGNLESS_MASK);
            }
            entry->key = bits;
            entry->index = index;
        }
        fn_1_B404(lbl_1_bss_14A8, particleCount);
    } else {
        /* Without a color buffer, sorting is disabled in the renderer until it is enabled again. */
        renderer->sortEnabled = 0;
    }
    particle = renderer->particles;
    vertices = renderer->vertices;
    textureCoords = renderer->textureCoords;
    PSMTXROMultVecArray(reordered, lbl_1_data_8E8, cameraCorners, 4);
    orientToWorld = 0;
    if ((renderer->attributes & 0x10) != 0) {
        orientToWorld = 1;
    }
    for (index = 0; index < renderer->particleCount; index++, particle++) {
        if (renderer->sortEnabled != 0) {
            particle = &renderer->particles[lbl_1_bss_14A8[index].index];
            renderer->colors[index] = particle->color;
        }
        if (particle->scale && (particle->cameraBit & Hu3DCameraBit) != 0 &&
            particle->visible != 0) {
            if (orientToWorld == 0) {
                if (0.0f == particle->rotationZ) {
                    corners = cameraCorners;
                } else {
                    sine = 0.5 * sin((M_PI * particle->rotationZ) / 180.0);
                    cosine = 0.5 * cos((M_PI * particle->rotationZ) / 180.0);
                    rotatingCorners = lbl_1_data_918;
                    rotatingCorners[0].x = rotatingCorners[3].y = -sine - cosine;
                    rotatingCorners[0].y = rotatingCorners[1].x = -sine + cosine;
                    rotatingCorners[1].y = rotatingCorners[2].x = sine + cosine;
                    rotatingCorners[2].y = rotatingCorners[3].x = sine - cosine;
                    PSMTXROMultVecArray(reordered, rotatingCorners, rotatedCorners, 4);
                    corners = rotatedCorners;
                }
            } else if (0.0f == particle->rotationX && 0.0f == particle->rotationY &&
                0.0f == particle->rotationZ) {
                corners = lbl_1_data_8E8;
            } else {
                mtxRot(rotation, particle->rotationX, particle->rotationY, particle->rotationZ);
                PSMTXMultVecArray(rotation, lbl_1_data_8E8, rotatedCorners, 4);
                corners = rotatedCorners;
            }
            ResultQuadScaleAdd(vertices, corners, &particle->pos, particle->scale);
        } else {
            ResultQuadClear(vertices);
        }
        vertices += 4;
        bank = &anim->bank[particle->animBank];
        frame = &bank->frame[particle->animNo];
        if ((renderer->attributes & 0x8) != 0 && particle->animStopped == 0 &&
            (Hu3DPauseF == 0 || (model->attr & HU3D_ATTR_NOPAUSE) != 0)) {
            for (animationStep = 0; animationStep < (s32)particle->animSpeed * minimumVcount;
                animationStep++) {
                particle->animTime += 1.0f;
                if (particle->animTime >= frame->time) {
                    particle->animNo++;
                    particle->animTime -= frame->time;
                    frame = &bank->frame[particle->animNo];
                    /* Rewinding or clamping animNo leaves frame at the end entry and does not stop
                     * this loop. The next timing check therefore uses that entry's duration. */
                    if (particle->animNo >= bank->timeNum || frame->time == -1) {
                        if ((renderer->attributes & 0x1) == 0) {
                            particle->visible = 0;
                            particle->animStopped = 1;
                            particle->animNo = bank->timeNum - 1;
                        } else {
                            particle->animNo = 0;
                        }
                    }
                }
            }
            particle->animTime += particle->animSpeed * minimumVcount - (s32)animationStep;
            if (particle->animTime >= frame->time) {
                particle->animNo++;
                particle->animTime -= frame->time;
                frame = &bank->frame[particle->animNo];
                if (particle->animNo >= bank->timeNum || frame->time == -1) {
                    if ((renderer->attributes & 0x1) == 0) {
                        particle->visible = 0;
                        particle->animStopped = 1;
                        particle->animNo = bank->timeNum - 1;
                    } else {
                        particle->animNo = 0;
                    }
                }
            }
        }
        ResultQuadTexCopy(
            textureCoords,
            &renderer
                 ->patternCoords[anim->bank[particle->animBank].frame[particle->animNo].pat * 4]);
        textureCoords += 4;
    }
    DCFlushRangeNoSync(renderer->vertices, renderer->particleCount * sizeof(Vec) * 4);
    if (renderer->sortEnabled != 0) {
        DCFlushRangeNoSync(renderer->colors, renderer->particleCount * sizeof(GXColor));
    } else {
        DCFlushRangeNoSync(renderer->particles,
                           renderer->particleCount * sizeof(ResultDrawParticle));
    }
    DCFlushRange(renderer->textureCoords, renderer->particleCount * sizeof(HuVec2f) * 4);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, renderer->alphaInput[0], renderer->alphaInput[1],
        renderer->alphaInput[2], renderer->alphaInput[3]);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP,
                  GX_AF_NONE);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetZCompLoc(0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetArray(GX_VA_POS, renderer->vertices, sizeof(Vec));
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    if (renderer->sortEnabled != 0 && renderer->colors) {
        GXSetArray(GX_VA_CLR0, renderer->colors, sizeof(GXColor));
    } else {
        GXSetArray(GX_VA_CLR0, &renderer->particles->color, sizeof(ResultDrawParticle));
    }
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetArray(GX_VA_TEX0, renderer->textureCoords, sizeof(HuVec2f));
    GXSetCullMode(GX_CULL_NONE);
    GXLoadPosMtxImm(*matrix, 0);
    if (shadowModelDrawF != 0) {
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ONE, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
        GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
    } else {
        GXSetTevColorIn(GX_TEVSTAGE0, renderer->colorInput[0], renderer->colorInput[1],
            renderer->colorInput[2], renderer->colorInput[3]);
        /* This renderer enables depth writes when the ZWRITE_OFF flag is set. */
        if ((model->attr & HU3D_ATTR_ZWRITE_OFF) != 0) {
            GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
        } else {
            GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        }
    }
    GXSetTevColor(GX_TEVREG0, renderer->color0);
    GXSetTevColor(GX_TEVREG1, renderer->color1);
    switch (renderer->blendMode) {
    case HU3D_PARTICLE_BLEND_NORMAL:
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        break;
    case HU3D_PARTICLE_BLEND_ADDCOL:
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
        break;
    case HU3D_PARTICLE_BLEND_INVCOL:
        GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_INVDSTCLR, GX_LO_NOOP);
        break;
    }
    HuSprTexLoad(renderer->anim, 0, 0, 0, 0, 1);
    GXCallDisplayList(renderer->displayList, renderer->displayListSize);
    totalPolyCnt += renderer->particleCount;
    if (shadowModelDrawF == 0) {
        if ((renderer->attributes & 0x2) == 0 && Hu3DPauseF == 0) {
            renderer->count++;
        }
        if (renderer->countLimit != 0 && renderer->countLimit <= renderer->count) {
            if ((renderer->attributes & 0x1) != 0) {
                renderer->count = 0;
            } else {
                renderer->count = renderer->countLimit;
            }
        }
        renderer->prevCounter = GlobalCounter;
    }
}

/* Stores the callback used by a result model's hook data. */
void fn_1_C420(s16 modelId, s32 callback)
{
    HU3D_MODEL *model;
    ResultParticleRenderer *hookData;

    model = &Hu3DData[modelId];
    hookData = model->hookData;
    hookData->callback = (void *)callback;
}

/* Clamps the language index and looks up the result archive prefix. All six prefixes are
 * identical, so data numbers are returned unchanged. */
s32 fn_1_C45C(s32 dataNum)
{
    s32 language;
    s32 languageIndex;

    language = GwCommon.languageNo;
    languageIndex = language;
    if (languageIndex < 0) {
        languageIndex = 0;
    } else if (languageIndex > 5) {
        languageIndex = 5;
    }
    if ((dataNum & RESULT_DATA_ID_MASK) != RESULT_DATA_ID_PATTERN) {
        return dataNum;
    }
    return (dataNum & 0xFFFF) | lbl_1_data_968[languageIndex];
}

s32 lbl_1_bss_1530[32];
s32 lbl_1_bss_14B0[32];
u32 lbl_1_bss_14AC;
ResultDrawSortEntry *lbl_1_bss_14A8;

f32 lbl_1_data_8E0[2] = {0.0f, 0.0f};
Vec lbl_1_data_8E8[4] = {
    { -0.5f, 0.5f, 0.0f }, { 0.5f, 0.5f, 0.0f }, { 0.5f, -0.5f, 0.0f }, { -0.5f, -0.5f, 0.0f }
};
Vec lbl_1_data_918[4] = {
    { -0.5f, 0.5f, 0.0f }, { 0.5f, 0.5f, 0.0f }, { 0.5f, -0.5f, 0.0f }, { -0.5f, -0.5f, 0.0f }
};
f32 lbl_1_data_948[4][2] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.25f}, {1.0f, 0.25f}};
u32 lbl_1_data_968[6] = {RESULT_DATA_ID_PATTERN, RESULT_DATA_ID_PATTERN,
    RESULT_DATA_ID_PATTERN, RESULT_DATA_ID_PATTERN, RESULT_DATA_ID_PATTERN,
    RESULT_DATA_ID_PATTERN};
