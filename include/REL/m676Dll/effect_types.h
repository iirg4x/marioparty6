/* Work records and mesh data used by the minigame's scene effects. */
#ifndef REL_M676DLL_EFFECT_TYPES_H
#define REL_M676DLL_EFFECT_TYPES_H
#include "game/object.h"
#include "game/hu3d.h"
#include "REL/m676Dll/particle_types.h"
#include "REL/m676Dll/prop_types.h"
typedef struct M676EffectVertex {
    s16 x,y,z; /* Mesh-local position in the effect disc. */
} M676EffectVertex;
typedef struct M676EffectUV {
    s16 s,t; /* Texture coordinates for one effect-disc vertex. */
} M676EffectUV;
typedef struct M676TextureWork {
    GXTexObj outputTexture; /* Captured scene texture used by the indirect effect. */
    void *textureBuffer; /* RGB565 image buffer for the captured scene. */
    ANIMDATA *textureData, *animationData; /* Repeating textures used to distort the capture. */
    Mtx textureMatrices[4]; /* Camera and scrolling transforms for the four texture stages. */
    f32 scrollOffsetX, scrollOffsetZ; /* Accumulating world-plane offsets used to scroll the
                                       * effect. */
} M676TextureWork;
typedef struct M676EffectWork {
    s32 enabled; /* Set after the texture hooks have been initialized. */
    M676TextureWork texture; /* Captured scene texture and its transforms. */
} M676EffectWork;
typedef struct M676ShadowRecord {
    s32 active; /* Whether the custom draw hook renders this shadow disc. */
    f32 radius; /* Disc radius in world units. */
    Vec *position; /* World position used by the disc; drawing adds 3 units to Y. */
} M676ShadowRecord;
typedef struct M676ShadowWork {
    M676ShadowRecord *records[10]; /* Shadow discs registered for drawing. */
    f32 xOffsets[10], zOffsets[10]; /* Unit-circle vertex offsets for the disc fan. */
} M676ShadowWork;

typedef M676PropWork M676SceneEffectWork;
#endif
