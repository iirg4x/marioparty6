/* Sprite and sprite-group data used by menus and game scenes. */
#ifndef _GAME_SPRITE_H
#define _GAME_SPRITE_H

#include "dolphin.h"
#include "humath.h"

#define HUSPR_DRAWNO_FRONT 0
#define HUSPR_DRAWNO_BACK 127

#define HUSPR_MAX 400
#define HUSPR_GROUP_MAX 256

#define HUSPR_NONE -1
#define HUSPR_GROUP_NONE -1

#define HUSPR_ATTR_NOANIM (1 << 0)
#define HUSPR_ATTR_LOOP (1 << 1)
#define HUSPR_ATTR_DISPOFF (1 << 2)
#define HUSPR_ATTR_LINEAR (1 << 3)
#define HUSPR_ATTR_FUNC (1 << 4)
#define HUSPR_ATTR_NOPAUSE (1 << 5)
#define HUSPR_ATTR_REVERSE (1 << 6)
#define HUSPR_ATTR_ADDCOL (1 << 7)
#define HUSPR_ATTR_INVCOL (1 << 8)
#define HUSPR_ATTR_3D (1 << 9)
#define HUSPR_ATTR_VTXCOLOR (1 << 10)
#define HUSPR_ATTR_VTXCOLOR_ADD (1 << 11)

#include "game/animdata.h"
#include "game/data.h"

#define HuSprDispOff(grpId, memberNo) HuSprAttrSet(grpId, memberNo, HUSPR_ATTR_DISPOFF)
#define HuSprDispOn(grpId, memberNo) HuSprAttrReset(grpId, memberNo, HUSPR_ATTR_DISPOFF)

#define HuSprAnimDataRead(fileNum)                                                                 \
    HuSprAnimRead(HuDataSelHeapReadNum(fileNum, HU_MEMNUM_OVL, HEAP_MODEL))

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

extern HUSPRITE *HuSprData;
extern HUSPR_GROUP HuSprGrpData[HUSPR_GROUP_MAX];

void HuSprInit(void);
void HuSprClose(void);
void HuSprExec(s16 drawNo);
void HuSprBegin(void);
HUSPRITE *HuSprCall(void);
void HuSprFinish(void);
void HuSprPauseSet(BOOL value);
ANIMDATA *HuSprAnimRead(void *data);
void HuSprAnimLock(ANIMDATA *anim);
HUSPRID HuSprCreate(ANIMDATA *anim, s16 prio, s16 bank);
HUSPRID HuSprFuncCreate(HUSPR_FUNC func, s16 prio);
HUSPR_GROUPID HuSprGrpCreate(s16 sprNum);
HUSPR_GROUPID HuSprGrpCopy(HUSPR_GROUPID grpId);
void HuSprGrpMemberSet(HUSPR_GROUPID grpId, s16 memberNo, HUSPRID sprId);
void HuSprGrpMemberKill(HUSPR_GROUPID grpId, s16 memberNo);
void HuSprGrpKill(HUSPR_GROUPID grpId);
void HuSprKill(HUSPRID sprId);
void HuSprAnimKill(ANIMDATA *anim);
void HuSprAttrSet(HUSPR_GROUPID grpId, s16 memberNo, s32 attr);
void HuSprAttrReset(HUSPR_GROUPID grpId, s16 memberNo, s32 attr);
void HuSprGrpAttrSet(HUSPR_GROUPID grpId, s32 attr);
void HuSprGrpAttrReset(HUSPR_GROUPID grpId, s32 attr);

void HuSprPosSet(HUSPR_GROUPID grpId, s16 memberNo, float posX, float posY);
void HuSprZRotSet(HUSPR_GROUPID grpId, s16 memberNo, float z_rot);
void HuSprScaleSet(HUSPR_GROUPID grpId, s16 memberNo, float scaleX, float scaleY);
void HuSprTPLvlSet(HUSPR_GROUPID grpId, s16 memberNo, float tpLvl);
void HuSprColorSet(HUSPR_GROUPID grpId, s16 memberNo, u8 r, u8 g, u8 b);
void HuSprSpeedSet(HUSPR_GROUPID grpId, s16 memberNo, float speed);
void HuSprBankSet(HUSPR_GROUPID grpId, s16 memberNo, s16 bank);
void HuSprGrpPosSet(HUSPR_GROUPID grpId, float posX, float posY);
void HuSprGrpCenterSet(HUSPR_GROUPID grpId, float centerX, float centerY);
void HuSprGrpZRotSet(HUSPR_GROUPID grpId, float z_rot);
void HuSprGrpScaleSet(HUSPR_GROUPID grpId, float scaleX, float scaleY);
void HuSprGrpTPLvlSet(HUSPR_GROUPID grpId, float tpLvl);
void HuSprGrpDrawNoSet(HUSPR_GROUPID grpId, s32 drawNo);
void HuSprDrawNoSet(HUSPR_GROUPID grpId, s16 memberNo, s32 drawNo);
void HuSprPriSet(HUSPR_GROUPID grpId, s16 memberNo, s16 prio);
void HuSprGrpScissorSet(HUSPR_GROUPID grpId, s16 x, s16 y, s16 w, s16 h);
void HuSprScissorSet(HUSPR_GROUPID grpId, s16 memberNo, s16 x, s16 y, s16 w, s16 h);
ANIMDATA *HuSprAnimMake(s16 sizeX, s16 sizeY, s16 dataFmt);
void HuSprBGSet(HUSPR_GROUPID grpId, s16 memberNo,  ANIMDATA *bg, s16 bgBank);
void HuSprSprBGSet(HUSPRID sprId, ANIMDATA *bg, s16 bgBank);
void AnimDebug(ANIMDATA *anim);

void HuSprDispInit(void);
void HuSprDisp(HUSPRITE *sp);
void HuSprExecLayerInit(void);
void HuSprExecLayerCameraSet(s16 drawNo, s16 camera, s16 layer);
/* Battle Bridge calls this layer setter without a visible declaration. */
#ifndef __MWERKS__
void HuSprExecLayerSet(s16 drawNo, s16 layer);
#endif
void HuSpr3DDisp(HUSPRITE *sp, Mtx *matrix, s16 layerIdx, HUSPR_RECT *rectVtx, HUSPR_RECT *rectST);
void HuSpr3DHookSet(HUSPRID sprId, HUSPR_3DHOOK hook3D);
HUSPR_3DDATA *HuSpr3DDataCreate(HUSPRID sprId, int size);
void HuSpr3DSet(HUSPRID sprId);
void HuSpr3DRotSet(HUSPRID sprId, float x, float y, float z);
void HuSpr3DFovSet(HUSPRID sprId, float fov);
void HuSprVtxColorSet(HUSPR_GROUPID grpId, s16 memberNo, GXColor *vtxColor);

#endif
