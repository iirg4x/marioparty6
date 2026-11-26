#ifndef GAME_SPRITE_TYPES_H
#define GAME_SPRITE_TYPES_H

#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "game/animdata.h"
#include "humath.h"

typedef struct HuSprite_s HUSPRITE;
typedef struct HuSprRect_s {
    /* Left and top coordinates of the sprite rectangle. */
    float x0;
    float y0;
    /* Right and bottom coordinates of the sprite rectangle. */
    float x1;
    float y1;
} HUSPR_RECT;

typedef s16 HUSPRID;
typedef s16 HUSPR_GROUPID;

typedef void (*HUSPR_FUNC)(HUSPRITE *sp);
typedef void (*HUSPR_3DHOOK)(HUSPRITE *sp, Mtx *matrix, s16 layerIdx, HUSPR_RECT *rectVtx,
                             HUSPR_RECT *rectST);

typedef struct HuSpr3DData_s {
    /* Grid dimensions for generated sprite vertices. */
    int col;
    int row;
    /* Depth multiplier applied to the sprite's 3D geometry. */
    float depthScale;
    /* Rotation of the generated sprite geometry in degrees. */
    HuVecF rot;
    /* Generated positions and texture coordinates. */
    HuVecF *vtx;
    HuVec2f *st;
} HUSPR_3DDATA;

struct HuSprite_s {
    /* Sprite tint and draw layer. */
    u8 r;
    u8 g;
    u8 b;
    u8 drawNo;
    /* Animation frame and texture bank. */
    s16 animNo;
    s16 bank;
    /* Rendering and animation behavior flags. */
    s16 attr;
    /* Nonzero when cached transform or drawing state needs refresh. */
    s16 dirty;
    /* Sprite draw priority within its layer. */
    s16 prio;
    /* Animation time in frames. */
    float time;
    /* Local position in screen or group coordinates. */
    HuVec2f pos;
    /* Rotation around the sprite's Z axis in degrees. */
    float zRot;
    /* Horizontal and vertical sprite scale. */
    HuVec2f scale;
    /* Animation playback speed. */
    float speed;
    /* Sprite transparency. */
    float a;
    /* Texture wrap mode for each axis. */
    GXTexWrapMode wrapS;
    GXTexWrapMode wrapT;
    /* Texture-coordinate scale for each axis. */
    s16 uvScaleX;
    s16 uvScaleY;
    /* Group transform used when the sprite belongs to a transformed group. */
    Mtx *groupMtx;
    union {
        /* Animation data or custom sprite draw callback. */
        ANIMDATA *data;
        HUSPR_FUNC func;
    };
    /* Current animation pattern and frame data. */
    ANIMPAT *patP;
    ANIMFRAME *frameP;
    /* Four work values reserved for sprite callbacks and effects. */
    s16 work[4];
    /* Optional background animation and selected bank. */
    ANIMDATA *bg;
    u16 bgBank;
    /* Optional clipping rectangle in screen pixels. */
    s16 scissorX;
    s16 scissorY;
    s16 scissorW;
    s16 scissorH;
    /* Per-corner vertex colors for the sprite quad. */
    GXColor vtxColor[4];
    /* Optional 3D drawing hook and its generated geometry. */
    HUSPR_3DHOOK hook3D;
    HUSPR_3DDATA *data3D;
};

typedef struct HuSprGroup_s {
    /* Number of sprite slots in the group. */
    s16 sprNum;
    /* Group position and rotation in screen coordinates. */
    HuVec2f pos;
    float zRot;
    /* Group scale and rotation pivot. */
    HuVec2f scale;
    HuVec2f center;
    /* Sprite IDs for each group member slot. */
    HUSPRID *sprId;
    /* Transform matrix shared by the group's members. */
    Mtx mtx;
    /* Group-specific work values. */
    s16 work[4];
} HUSPR_GROUP;

#endif
