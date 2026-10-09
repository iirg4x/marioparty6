/* Manages sprite animation, groups, ordering, and frame-by-frame drawing. */
#define _MATH_H
#define M_PI 3.141592653589793
double sin(double x);
#include "game/sprite.h"
#include "game/memory.h"
#include "game/init.h"

#include "dolphin/mtx.h"

#define HUSPR_ORDER_MAX 768

#define SPRITE_DIRTY_ATTR 0x1
#define SPRITE_DIRTY_XFORM 0x2
#define SPRITE_DIRTY_COLOR 0x4
#define ANIM_POINTER_ABSOLUTE_MASK 0xFFFF0000

typedef struct HuSprOrder_s {
    u16 groupId;
    u16 sprId;
    u16 priority;
    u16 nextOrder;
} HUSPR_ORDER;

HUSPR_GROUP HuSprGrpData[HUSPR_GROUP_MAX];
static HUSPR_ORDER HuSprOrder[HUSPR_ORDER_MAX];

static s16 HuSprOrderNum;
static s16 HuSprOrderNo;
static BOOL HuSprPauseF;
HUSPRITE *HuSprData;

static void HuSprOrderEntry(HUSPR_GROUPID grpId, HUSPRID sprId);

/* System startup calls this to initialize sprite storage before frame drawing. */
void HuSprInit(void)
{
    s16 spriteIndex;
    HUSPRITE *sprite;
    HUSPR_GROUP *group;
    if(!HuSprData) {
        HuSprData = HuMemDirectMalloc(HEAP_HEAP, sizeof(HUSPRITE)*HUSPR_MAX);
    }
    for(sprite = &HuSprData[1], spriteIndex=1; spriteIndex<HUSPR_MAX; spriteIndex++, sprite++) {
        sprite->data = NULL;
    }
    for(group = HuSprGrpData, spriteIndex=0; spriteIndex<HUSPR_GROUP_MAX; spriteIndex++, group++) {
        group->sprNum = 0;
    }
    HuSprExecLayerInit();
    sprite = &HuSprData[0];
    sprite->prio = 0;
    sprite->data = (void *)1;
    HuSprPauseF = FALSE;
}

/* Scene teardown calls this to release groups and sprites when sprite use ends. */
void HuSprClose(void)
{
    s16 entryIndex;
    HUSPR_GROUP *group;
    HUSPRITE *sprite;
    
    for(group = HuSprGrpData, entryIndex=0; entryIndex<HUSPR_GROUP_MAX; entryIndex++, group++) {
        if(group->sprNum != 0) {
            HuSprGrpKill(entryIndex);
        }
    }
    for(sprite = &HuSprData[1], entryIndex=1; entryIndex<HUSPR_MAX; entryIndex++, sprite++) {
        if(sprite->data) {
            HuSprKill(entryIndex);
        }
    }
    HuSprExecLayerInit();
    HuSprPauseF = FALSE;
}

/* Hu3DExec calls this for each draw layer during the camera rendering pass. */
void HuSprExec(s16 drawNo)
{
    HUSPRITE *sprite;
    while(sprite = HuSprCall()) {
        if(!(sprite->attr & HUSPR_ATTR_DISPOFF) && sprite->drawNo == drawNo) {
            HuSprDisp(sprite);
        }
    }
}

/* Hu3DExec calls this before camera drawing to build transforms and sprite order. */
void HuSprBegin(void)
{
    Mtx translationMatrix, rotationMatrix;
    s16 groupIndex, memberIndex;
    Vec axis = {0, 0, 1};
    HUSPR_GROUP *group;
    group = HuSprGrpData;
    HuSprOrderNum = 1;
    HuSprOrder[0].nextOrder = 0;
    HuSprOrder[0].priority = -1;
    for(groupIndex=0; groupIndex<HUSPR_GROUP_MAX; groupIndex++, group++) {
        if(group->sprNum != 0) {
            MTXTrans(translationMatrix, group->center.x * group->scale.x,
                     group->center.y * group->scale.y, 0.0f);
            MTXRotAxisDeg(rotationMatrix, &axis, group->zRot);
            MTXConcat(rotationMatrix, translationMatrix, group->mtx);
            MTXScale(translationMatrix, group->scale.x, group->scale.y, 1.0f);
            MTXConcat(group->mtx, translationMatrix, group->mtx);
            mtxTransCat(group->mtx, group->pos.x, group->pos.y, 0);
            for(memberIndex=0; memberIndex<group->sprNum; memberIndex++) {
                if(group->sprId[memberIndex] != -1) {
                    HuSprOrderEntry(groupIndex, group->sprId[memberIndex]);
                }
            }
        }
    }
    HuSprOrderNo = 0;
}

/* HuSprBegin calls this for each populated member to insert it by sprite priority. */
static void HuSprOrderEntry(HUSPR_GROUPID grpId, HUSPRID sprId)
{
    HUSPR_ORDER *order = &HuSprOrder[HuSprOrderNum];
    s16 priority = HuSprData[sprId].prio;
    s16 previousOrder, nextOrder;
    if(HuSprOrderNum >= HUSPR_ORDER_MAX) {
        OSReport("Order Max Over!\n");
        return;
    }
    nextOrder = HuSprOrder[0].nextOrder;
    for (previousOrder = 0; nextOrder != 0;
         previousOrder = nextOrder, nextOrder = HuSprOrder[nextOrder].nextOrder) {
        if(HuSprOrder[nextOrder].priority < priority) {
            break;
        }
    }
    order->nextOrder = HuSprOrder[previousOrder].nextOrder;
    HuSprOrder[previousOrder].nextOrder = HuSprOrderNum;
    order->priority = priority;
    order->groupId = grpId;
    order->sprId = sprId;
    HuSprOrderNum++;
}

/* HuSprExec calls this repeatedly to walk the frame's priority-ordered sprites. */
HUSPRITE *HuSprCall(void)
{
    HuSprOrderNo = HuSprOrder[HuSprOrderNo].nextOrder;
    if(HuSprOrderNo != 0) {
        HUSPR_ORDER *order = &HuSprOrder[HuSprOrderNo];
        HUSPRITE *sprite = &HuSprData[order->sprId];
        sprite->groupMtx = &HuSprGrpData[order->groupId].mtx;
        if(sprite->attr & HUSPR_ATTR_FUNC) {
            return sprite;
        }
        sprite->frameP = &sprite->data->bank[sprite->bank].frame[sprite->animNo];
        sprite->patP = &sprite->data->pat[sprite->frameP->pat];
        return sprite;
    } else {
        return NULL;
    }
}

/* HuSprFinish calls this while advancing a sprite, wrapping or holding at an end. */
static inline void SpriteCalcFrame(HUSPRITE *sprite, ANIMBANK *bank, ANIMFRAME **frame,
                                   s16 loopEnabled)
{
    if(sprite->time >= (*frame)->time) {
        sprite->animNo++;
        sprite->time -= (*frame)->time;
        if(sprite->animNo >= bank->timeNum || (*frame)[1].time == -1) {
            if(loopEnabled) {
                sprite->animNo = 0;
            } else {
                sprite->animNo = bank->timeNum-1;
            }
        }
        *frame = &bank->frame[sprite->animNo];
    } else if(sprite->time < 0) {
        sprite->animNo--;
        if(sprite->animNo < 0) {
            if(loopEnabled) {
                sprite->animNo = bank->timeNum-1;
            } else {
                sprite->animNo = 0;
            }
        }
        *frame = &bank->frame[sprite->animNo];
        sprite->time += (*frame)->time;
    }
}

/* Hu3DExec calls this after its camera draws to advance sprite animations. */
void HuSprFinish(void)
{
    ANIMDATA *anim;
    ANIMBANK *bank;
    ANIMFRAME *frame;
    HUSPRITE *sprite;
    s16 spriteIndex;
    s16 frameStep;
    s16 loopEnabled;
    s16 timeIncrement;
    
    for(sprite = &HuSprData[1], spriteIndex=1; spriteIndex<HUSPR_MAX; spriteIndex++, sprite++) {
        if(sprite->data && !(sprite->attr & HUSPR_ATTR_FUNC)) {
            if(!HuSprPauseF || (sprite->attr & HUSPR_ATTR_NOPAUSE)) {
                anim = sprite->data;
                bank = &anim->bank[sprite->bank];
                frame = &bank->frame[sprite->animNo];
                loopEnabled = (sprite->attr & HUSPR_ATTR_LOOP) ? 0 : 1;
                if(!(sprite->attr & HUSPR_ATTR_NOANIM)) {
                    timeIncrement = (sprite->attr & HUSPR_ATTR_REVERSE) ? -1 : 1;
                    for(frameStep=0; frameStep<(s32)sprite->speed*minimumVcount; frameStep++) {
                        sprite->time += timeIncrement;
                        SpriteCalcFrame(sprite, bank, &frame, loopEnabled);
                    }
                    sprite->time += (sprite->speed*(float)minimumVcount)-frameStep;
                    SpriteCalcFrame(sprite, bank, &frame, loopEnabled);
                }
                sprite->dirty = 0;
            }
        }
    }
}

/* Pause and scene logic call this to gate animation updates for paused sprites. */
void HuSprPauseSet(BOOL value)
{
    HuSprPauseF = value;
}

/* After animation data loads, this fixes relative pointers; already-absolute data skips fixups and
 * increments useNum. */
ANIMDATA *HuSprAnimRead(void *data)
{
    s16 index;
    ANIMBMP *bmp;
    ANIMBANK *bank;
    ANIMPAT *pat;
    
    ANIMDATA *anim = (ANIMDATA *)data;
    if((u32)anim->bank & ANIM_POINTER_ABSOLUTE_MASK) {
        anim->useNum++;
        return anim;
    }
    bank = (ANIMBANK *)((u32)anim->bank+(u32)data);
    anim->bank = bank;
    pat = (ANIMPAT *)((u32)anim->pat+(u32)data);
    anim->pat = pat;
    bmp = (ANIMBMP *)((u32)anim->bmp+(u32)data);
    anim->bmp = bmp;
    for(index=0; index<anim->bankNum; index++, bank++) {
        bank->frame = (ANIMFRAME *)((u32)bank->frame+(u32)data);
    }
    for(index=0; index<anim->patNum; index++, pat++) {
        pat->layer = (ANIMLAYER *)((u32)pat->layer+(u32)data);
    }
    for(index=0; index<anim->bmpNum; index++, bmp++) {
        bmp->palData = (void *)((u32)bmp->palData+(u32)data);
        bmp->data = (void *)((u32)bmp->data+(u32)data);
    }
    anim->useNum = 0;
    return anim;
}

/* Adds one sprite's reference to shared animation data. */
void HuSprAnimLock(ANIMDATA *anim)
{
    anim->useNum++;
}

/* HUD and game sprite setup call this to allocate a slot with standard defaults. */
HUSPRID HuSprCreate(ANIMDATA *anim, s16 prio, s16 bank)
{
    HUSPRITE *sprite;
    s16 spriteIndex;
    for(sprite = &HuSprData[1], spriteIndex=1; spriteIndex<HUSPR_MAX; spriteIndex++, sprite++) {
        if(!sprite->data) {
            break;
        }
    }
    if(spriteIndex == HUSPR_MAX) {
        OSReport("Error: Sprite Max Over!\n");
        return HUSPR_NONE;
    }
    sprite->data = anim;
    sprite->speed = 1.0f;
    sprite->animNo = 0;
    sprite->bank = bank;
    sprite->time = 0.0f;
    sprite->attr = HUSPR_ATTR_LINEAR;
    sprite->drawNo = 0;
    sprite->r = sprite->g = sprite->b = sprite->a = 255;
    sprite->pos.x = sprite->pos.y = sprite->zRot = 0.0f;
    sprite->prio = prio;
    sprite->scale.x = sprite->scale.y = 1.0f;
    sprite->wrapS = sprite->wrapT = GX_CLAMP;
    sprite->uvScaleX = sprite->uvScaleY = 1;
    sprite->bg = NULL;
    sprite->scissorX = sprite->scissorY = 0;
    sprite->scissorW = 640;
    sprite->scissorH = 480;
    sprite->hook3D = NULL;
    sprite->data3D = NULL;
    if(anim) {
        HuSprAnimLock(anim);
    }
    return spriteIndex;
}

/* Callback-based sprite setup calls this to create a sprite with custom rendering. */
HUSPRID HuSprFuncCreate(HUSPR_FUNC func, s16 prio)
{
    HUSPRITE *sprite;
    HUSPRID sprId = HuSprCreate(NULL, prio, 0);
    if(sprId == HUSPR_NONE) {
        return HUSPR_NONE;
    }
    sprite = &HuSprData[sprId];
    sprite->func = func;
    sprite->attr |= HUSPR_ATTR_FUNC;
    return sprId;
}

/* HUD and game sprite setup call this to allocate empty member slots. */
HUSPR_GROUPID HuSprGrpCreate(s16 sprNum)
{
    HUSPR_GROUP *group;
    s16 groupIndex, memberIndex;
    for(group = HuSprGrpData, groupIndex=0; groupIndex<HUSPR_GROUP_MAX; groupIndex++, group++) {
        if(group->sprNum == 0) {
            break;
        }
    }
    if(groupIndex == HUSPR_GROUP_MAX) {
        return HUSPR_GROUP_NONE;
    }
    group->sprId = HuMemDirectMalloc(HEAP_HEAP, sizeof(HUSPRID)*sprNum);
    for(memberIndex=0; memberIndex<sprNum; memberIndex++) {
        group->sprId[memberIndex] = HUSPR_NONE;
    }
    group->sprNum = sprNum;
    group->pos.x = group->pos.y = group->zRot = group->center.x = group->center.y = 0.0f;
    group->scale.x = group->scale.y = 1.0f;
    return groupIndex;
}

/* Sprite clients call this when they need a group copy with independent members. */
HUSPR_GROUPID HuSprGrpCopy(HUSPR_GROUPID grpId)
{
    HUSPR_GROUP *newGroup;
    HUSPR_GROUP *sourceGroup = &HuSprGrpData[grpId];
    HUSPR_GROUPID newGroupId = HuSprGrpCreate(sourceGroup->sprNum);
    s16 memberIndex;
    if(newGroupId == HUSPR_GROUP_NONE) {
        return HUSPR_GROUP_NONE;
    }
    newGroup = &HuSprGrpData[newGroupId];
    newGroup->pos.x = sourceGroup->pos.x;
    newGroup->pos.y = sourceGroup->pos.y;
    newGroup->zRot = sourceGroup->zRot;
    newGroup->scale.x = sourceGroup->scale.x;
    newGroup->scale.y = sourceGroup->scale.y;
    newGroup->center.x = sourceGroup->center.x;
    newGroup->center.y = sourceGroup->center.y;
    for(memberIndex=0; memberIndex<sourceGroup->sprNum; memberIndex++) {
        if(sourceGroup->sprId[memberIndex] != HUSPR_NONE) {
            HUSPRITE *sourceSprite = &HuSprData[sourceGroup->sprId[memberIndex]];
            s16 newSpriteId =
                HuSprCreate(sourceSprite->data, sourceSprite->prio, sourceSprite->bank);
            HuSprData[newSpriteId] = *sourceSprite;
            HuSprGrpMemberSet(newGroupId, memberIndex, newSpriteId);
        }
    }
    return newGroupId;
}

/* Sprite setup calls this to place a sprite in an unused group member slot. */
void HuSprGrpMemberSet(HUSPR_GROUPID grpId, s16 memberNo, HUSPRID sprId)
{
    HUSPR_GROUP *group = &HuSprGrpData[grpId];
    HUSPRITE *sprite = &HuSprData[sprId];
    if(group->sprNum == 0 || group->sprNum <= memberNo || group->sprId[memberNo] != HUSPR_NONE) {
        return;
    }
    group->sprId[memberNo] = sprId;
}

/* Group owners call this when removing one member and freeing its sprite data. */
void HuSprGrpMemberKill(HUSPR_GROUPID grpId, s16 memberNo)
{
    HUSPR_GROUP *group = &HuSprGrpData[grpId];
    if(group->sprNum == 0 || group->sprNum <= memberNo || group->sprId[memberNo] == HUSPR_NONE) {
        return;
    }
    HuSprKill(group->sprId[memberNo]);
    group->sprId[memberNo] = HUSPR_NONE;
}

/* Group owners call this when removing a group and releasing all of its members. */
void HuSprGrpKill(HUSPR_GROUPID grpId)
{
    HUSPR_GROUP *group = &HuSprGrpData[grpId];
    s16 memberIndex;
    for(memberIndex=0; memberIndex<group->sprNum; memberIndex++) {
        if(group->sprId[memberIndex] != HUSPR_NONE) {
            HuSprKill(group->sprId[memberIndex]);
        }
    }
    group->sprNum = 0;
    HuMemDirectFree(group->sprId);
}

/* Group and sprite owners call this to release animation, background, and mesh data. */
void HuSprKill(HUSPRID sprId)
{
    HUSPRITE *sprite = &HuSprData[sprId];
    if(!sprite->data) {
        return;
    }
    if(!(sprite->attr & HUSPR_ATTR_FUNC)) {
        HuSprAnimKill(sprite->data);
    }
    if(sprite->bg) {
        HuSprAnimKill(sprite->bg);
        sprite->bg = NULL;
    }
    if(sprite->attr & HUSPR_ATTR_3D) {
        if(sprite->data3D) {
            HuMemDirectFree(sprite->data3D);
        }
    }
    sprite->data = NULL;
}

/* Sprite destruction calls this to free animation data after its final user releases it. */
void HuSprAnimKill(ANIMDATA *anim)
{
    if(--anim->useNum <= 0) {
        if(anim->bmpNum & ANIM_BMP_ALLOC) {
            if(anim->bmp->data) {
                HuMemDirectFree(anim->bmp->data);
            }
            if(anim->bmp->palData) {
                HuMemDirectFree(anim->bmp->palData);
            }
        }
        HuMemDirectFree(anim);
    }
}

/* Sprite clients call this to add attributes and refresh the member's render state. */
void HuSprAttrSet(HUSPR_GROUPID grpId, s16 memberNo, s32 attr)
{
    HUSPR_GROUP *group = &HuSprGrpData[grpId];
    HUSPRITE *sprite;
    if(group->sprNum == 0 || group->sprNum <= memberNo || group->sprId[memberNo] == HUSPR_NONE) {
        return;
    }
    sprite = &HuSprData[group->sprId[memberNo]];
    sprite->attr |= attr;
    sprite->dirty |= SPRITE_DIRTY_ATTR;
}

/* Sprite clients call this to clear attributes and refresh the member's render state. */
void HuSprAttrReset(HUSPR_GROUPID grpId, s16 memberNo, s32 attr)
{
    HUSPR_GROUP *group = &HuSprGrpData[grpId];
    HUSPRITE *sprite;
    if(group->sprNum == 0 || group->sprNum <= memberNo || group->sprId[memberNo] == HUSPR_NONE) {
        return;
    }
    sprite = &HuSprData[group->sprId[memberNo]];
    sprite->attr &= ~attr;
    sprite->dirty |= SPRITE_DIRTY_ATTR;
}

/* Group clients call this to apply attribute bits to every populated member. */
void HuSprGrpAttrSet(HUSPR_GROUPID grpId, s32 attr)
{
    s16 memberIndex;
    HUSPR_GROUP *group = &HuSprGrpData[grpId];
    for(memberIndex=0; memberIndex<group->sprNum; memberIndex++) {
        if(group->sprId[memberIndex] != HUSPR_NONE) {
            HuSprAttrSet(grpId, memberIndex, attr);
        }
    }
}

/* Group clients call this to clear attribute bits from every populated member. */
void HuSprGrpAttrReset(HUSPR_GROUPID grpId, s32 attr)
{
    s16 memberIndex;
    HUSPR_GROUP *group = &HuSprGrpData[grpId];
    for(memberIndex=0; memberIndex<group->sprNum; memberIndex++) {
        if(group->sprId[memberIndex] != HUSPR_NONE) {
            HuSprAttrReset(grpId, memberIndex, attr);
        }
    }
}

/* Sprite clients query this for a member's attributes while the group slot is live. */
u16 HuSprAttrGet(HUSPR_GROUPID grpId, s16 memberNo)
{
    HUSPR_GROUP *group = &HuSprGrpData[grpId];
    HUSPRITE *sprite;
    if(group->sprNum == 0 || group->sprNum <= memberNo || group->sprId[memberNo] == HUSPR_NONE) {
        return;
    }
    sprite = &HuSprData[group->sprId[memberNo]];
    return sprite->attr;
}

/* Sprite clients call this to set local x/y position before the next draw update. */
void HuSprPosSet(HUSPR_GROUPID grpId, s16 memberNo, float posX, float posY)
{
    HUSPRITE *sprite = &HuSprData[HuSprGrpData[grpId].sprId[memberNo]];
    sprite->pos.x = posX;
    sprite->pos.y = posY;
    sprite->dirty |= SPRITE_DIRTY_XFORM;
}

/* Sprite clients call this to set the member's z rotation before the next draw. */
void HuSprZRotSet(HUSPR_GROUPID grpId, s16 memberNo, float zRot)
{
    HUSPRITE *sprite = &HuSprData[HuSprGrpData[grpId].sprId[memberNo]];
    sprite->zRot = zRot;
    sprite->dirty |= SPRITE_DIRTY_XFORM;
}

/* Sprite clients call this to set x/y scale before the next draw update. */
void HuSprScaleSet(HUSPR_GROUPID grpId, s16 memberNo, float scaleX, float scaleY)
{
    HUSPRITE *sprite = &HuSprData[HuSprGrpData[grpId].sprId[memberNo]];
    sprite->scale.x = scaleX;
    sprite->scale.y = scaleY;
    sprite->dirty |= SPRITE_DIRTY_XFORM;
}

/* Sprite clients set transparency here; a level of 1.0 maps to opaque alpha. */
void HuSprTPLvlSet(HUSPR_GROUPID grpId, s16 memberNo, float tpLvl)
{
    HUSPRITE *sprite = &HuSprData[HuSprGrpData[grpId].sprId[memberNo]];
    sprite->a = tpLvl*255;
    sprite->dirty |= SPRITE_DIRTY_COLOR;
}

/* Sprite clients call this to set RGB tint before the next draw update. */
void HuSprColorSet(HUSPR_GROUPID grpId, s16 memberNo, u8 r, u8 g, u8 b)
{
    HUSPRITE *sprite = &HuSprData[HuSprGrpData[grpId].sprId[memberNo]];
    sprite->r = r;
    sprite->g = g;
    sprite->b = b;
    sprite->dirty |= SPRITE_DIRTY_COLOR;
}

/* Sprite clients set the member's animation speed multiplier through this API. */
void HuSprSpeedSet(HUSPR_GROUPID grpId, s16 memberNo, float speed)
{
    HUSPR_GROUP *group = &HuSprGrpData[grpId];
    HuSprData[group->sprId[memberNo]].speed = speed;
}

/* Sprite clients switch banks and reset frame time; reverse mode uses the previously selected
 * bank's frame and duration. */
void HuSprBankSet(HUSPR_GROUPID grpId, s16 memberNo, s16 bank)
{
    HUSPRITE *sprite = &HuSprData[HuSprGrpData[grpId].sprId[memberNo]];
    ANIMDATA *anim = sprite->data;
    ANIMBANK *bankData = &anim->bank[sprite->bank];
    ANIMFRAME *frame = &bankData->frame[sprite->animNo];
    sprite->bank = bank;
    if(sprite->attr & HUSPR_ATTR_REVERSE) {
        sprite->animNo = bankData->timeNum-1;
        frame = &bankData->frame[sprite->animNo];
        sprite->time = frame->time;
    } else {
        sprite->time = 0;
        sprite->animNo = 0;
    }
}

/* Sprite clients select a current-bank frame and reset elapsed time; an index at or past
 * bank->timeNum logs an error and resets to frame 0. */
void HuSprAnimNoSet(HUSPR_GROUPID grpId, s16 memberNo, s16 animNo)
{
    HUSPRITE *sprite = &HuSprData[HuSprGrpData[grpId].sprId[memberNo]];
    ANIMDATA *anim = sprite->data;
    ANIMBANK *bankData = &anim->bank[sprite->bank];
    if(bankData->timeNum <= animNo) {
        OSReport("Error: AnimNoSet Over %d\n", animNo);
        animNo = 0;
    }
    sprite->animNo = animNo;
    sprite->time = 0;
}

/* Sprite clients select a frame and pause its advance until the no-animation bit clears. */
void HuSprAnimNoSetPause(HUSPR_GROUPID grpId, s16 memberNo, s16 animNo)
{
    HUSPRITE *sprite = &HuSprData[HuSprGrpData[grpId].sprId[memberNo]];
    HuSprAnimNoSet(grpId, memberNo, animNo);
    sprite->attr |= HUSPR_ATTR_NOANIM;
}

/* Group clients set world position here before the next transform build and draw. */
void HuSprGrpPosSet(HUSPR_GROUPID grpId, float posX, float posY)
{
    HUSPR_GROUP *group = &HuSprGrpData[grpId];
    s16 memberIndex;
    group->pos.x = posX;
    group->pos.y = posY;
    for(memberIndex=0; memberIndex<group->sprNum; memberIndex++) {
        if(group->sprId[memberIndex] != -1) {
            HuSprData[group->sprId[memberIndex]].dirty |= SPRITE_DIRTY_XFORM;
        }
    }
}

/* Group clients set the pivot used when rotation and scale are built for drawing. */
void HuSprGrpCenterSet(HUSPR_GROUPID grpId, float centerX, float centerY)
{
    HUSPR_GROUP *group = &HuSprGrpData[grpId];
    s16 memberIndex;
    group->center.x = centerX;
    group->center.y = centerY;
    for(memberIndex=0; memberIndex<group->sprNum; memberIndex++) {
        if(group->sprId[memberIndex] != HUSPR_NONE) {
            HuSprData[group->sprId[memberIndex]].dirty |= SPRITE_DIRTY_XFORM;
        }
    }
}

/* Group clients set z rotation here before the next transform build and draw. */
void HuSprGrpZRotSet(HUSPR_GROUPID grpId, float zRot)
{
    HUSPR_GROUP *group = &HuSprGrpData[grpId];
    s16 memberIndex;
    group->zRot = zRot;
    for(memberIndex=0; memberIndex<group->sprNum; memberIndex++) {
        if(group->sprId[memberIndex] != HUSPR_NONE) {
            HuSprData[group->sprId[memberIndex]].dirty |= SPRITE_DIRTY_XFORM;
        }
    }
}

/* Group clients set x/y scale here before the next transform build and draw. */
void HuSprGrpScaleSet(HUSPR_GROUPID grpId, float scaleX, float scaleY)
{
    HUSPR_GROUP *group = &HuSprGrpData[grpId];
    s16 memberIndex;
    group->scale.x = scaleX;
    group->scale.y = scaleY;
    for(memberIndex=0; memberIndex<group->sprNum; memberIndex++) {
        if(group->sprId[memberIndex] != HUSPR_NONE) {
            HuSprData[group->sprId[memberIndex]].dirty |= SPRITE_DIRTY_XFORM;
        }
    }
}

/* Group clients set alpha here for every populated sprite before drawing. */
void HuSprGrpTPLvlSet(HUSPR_GROUPID grpId, float tpLvl)
{
    HUSPR_GROUP *group = &HuSprGrpData[grpId];
    s16 memberIndex;
    for(memberIndex=0; memberIndex<group->sprNum; memberIndex++) {
        if(group->sprId[memberIndex] != HUSPR_NONE) {
            HuSprData[group->sprId[memberIndex]].a = tpLvl*255;
            HuSprData[group->sprId[memberIndex]].dirty |= SPRITE_DIRTY_COLOR;
        }
    }
}

/* Group clients assign a draw-layer number to each populated member with this call. */
void HuSprGrpDrawNoSet(HUSPR_GROUPID grpId, s32 drawNo)
{
    HUSPR_GROUP *group = &HuSprGrpData[grpId];
    s16 memberIndex;
    for(memberIndex=0; memberIndex<group->sprNum; memberIndex++) {
        if(group->sprId[memberIndex] != HUSPR_NONE) {
            HuSprData[group->sprId[memberIndex]].drawNo = drawNo;
        }
    }
}

/* Sprite clients assign a member to a draw layer such as the front or back pass. */
void HuSprDrawNoSet(HUSPR_GROUPID grpId, s16 memberNo, s32 drawNo)
{
    HUSPRITE *sprite = &HuSprData[HuSprGrpData[grpId].sprId[memberNo]];
    sprite->drawNo = drawNo;
}

/* Sprite clients set priority here before HuSprBegin builds the next sprite order. */
void HuSprPriSet(HUSPR_GROUPID grpId, s16 memberNo, s16 prio)
{
    HUSPRITE *sprite = &HuSprData[HuSprGrpData[grpId].sprId[memberNo]];
    sprite->prio = prio;
}

/* Applies one scissor rectangle to each populated member of a group. */
/* Group clients apply a pixel scissor rectangle to each populated member. */
void HuSprGrpScissorSet(HUSPR_GROUPID grpId, s16 x, s16 y, s16 width, s16 height)
{
    HUSPR_GROUP *group = &HuSprGrpData[grpId];
    s16 memberIndex;
    for(memberIndex=0; memberIndex<group->sprNum; memberIndex++) {
        if(group->sprId[memberIndex] != HUSPR_NONE) {
            HuSprScissorSet(grpId, memberIndex, x, y, width, height);
        }
    }
}

/* Sprite clients set the member's screen-space scissor rectangle in pixel units. */
void HuSprScissorSet(HUSPR_GROUPID grpId, s16 memberNo, s16 x, s16 y, s16 width, s16 height)
{
    HUSPRITE *sprite = &HuSprData[HuSprGrpData[grpId].sprId[memberNo]];
    sprite->scissorX = x;
    sprite->scissorY = y;
    sprite->scissorW = width;
    sprite->scissorH = height;
}

static s16 bitSizeTbl[11] = { 32, 24, 16, 8, 4, 16, 8, 8, 4, 8, 4 };

/* Sprite creation code calls this to make a one-frame animation for a new image. */
ANIMDATA *HuSprAnimMake(s16 sizeX, s16 sizeY, s16 dataFmt)
{
    ANIMLAYER *layer;
    ANIMBMP *bmp;
    ANIMDATA *anim;
    ANIMPAT *pat;
    ANIMFRAME *frame;
    void *allocationCursor;
    ANIMBANK *bank;
    ANIMDATA *newAnim;

    anim = newAnim =
        HuMemDirectMalloc(HEAP_MODEL, sizeof(ANIMDATA) + sizeof(ANIMBANK) + sizeof(ANIMFRAME) +
                                          sizeof(ANIMPAT) + sizeof(ANIMLAYER) + sizeof(ANIMBMP));

    /* The animation records share one block, laid out in type order. */
    bank = allocationCursor = &newAnim[1];
    anim->bank = bank;
    frame = allocationCursor = (ANIMBANK *)allocationCursor + 1;
    bank->frame = frame;
    pat = allocationCursor = (ANIMFRAME *)allocationCursor + 1;
    anim->pat = pat;
    layer = allocationCursor = (ANIMPAT *)allocationCursor + 1;
    pat->layer = layer;
    bmp = allocationCursor = (ANIMLAYER *)allocationCursor + 1;
    anim->bmp = bmp;
    anim->useNum = 0;
    anim->bankNum = 1;
    anim->patNum = 1;
    anim->bmpNum = (1|ANIM_BMP_ALLOC);
    bank->timeNum = 1;
    bank->unk = 10;
    frame->pat = 0;
    frame->time = 10; /* The one-frame image remains on screen for ten ticks. */
    frame->shiftX = frame->shiftY = frame->flip = 0;
    pat->layerNum = 1;
    pat->centerX = sizeX/2;
    pat->centerY = sizeY/2;
    pat->sizeX = sizeX;
    pat->sizeY = sizeY;
    layer->alpha = 255;
    layer->flip = 0;
    layer->bmpNo = 0;
    layer->startX = layer->startY = 0;
    layer->sizeX = sizeX;
    layer->sizeY = sizeY;
    layer->shiftX = layer->shiftY = 0;
    layer->vtx[0] = layer->vtx[1] = 0;
    layer->vtx[2] = sizeX;
    layer->vtx[3] = 0;
    layer->vtx[4] = sizeX;
    layer->vtx[5] = sizeY;
    layer->vtx[6] = 0;
    layer->vtx[7] = sizeY;
    bmp->pixSize = bitSizeTbl[dataFmt];
    bmp->dataFmt = dataFmt;
    bmp->palNum = 0;
    bmp->sizeX = sizeX;
    bmp->sizeY = sizeY;
    bmp->dataSize = sizeX*sizeY*bitSizeTbl[dataFmt]/8;
    bmp->palData = NULL;
    bmp->data = NULL;
    return anim;
}

/* Attaches a background animation to one group member. */
void HuSprBGSet(HUSPR_GROUPID grpId, s16 memberNo, ANIMDATA *bg, s16 bgBank)
{
    HUSPRID sprId = HuSprGrpData[grpId].sprId[memberNo];
    HuSprSprBGSet(sprId, bg, bgBank);
}

/* Configures a sprite's repeated, non-linear-filtered background animation. */
void HuSprSprBGSet(HUSPRID sprId, ANIMDATA *bg, s16 bgBank)
{
    HUSPRITE *sprite = &HuSprData[sprId];
    sprite->bg = bg;
    sprite->bgBank = bgBank;
    sprite->wrapT = sprite->wrapS = GX_REPEAT;
    sprite->attr &= ~HUSPR_ATTR_LINEAR;
    HuSprAnimLock(bg);
}

/* Installs the callback used to draw a sprite with its custom 3D geometry. */
void HuSpr3DHookSet(HUSPRID sprId, HUSPR_3DHOOK hook3D)
{
    HUSPRITE *sprite = &HuSprData[sprId];
    sprite->hook3D = hook3D;
}

/* Allocates per-sprite 3D drawing data and marks the sprite for the 3D path. */
HUSPR_3DDATA *HuSpr3DDataCreate(HUSPRID sprId, int size)
{
    HUSPRITE *sprite = &HuSprData[sprId];
    sprite->data3D = HuMemDirectMalloc(HEAP_HEAP, size);
    sprite->attr |= HUSPR_ATTR_3D;
    return sprite->data3D;
}

/* Builds a mesh sized from the maximum layer width and height, with max(1, size/16) subdivisions
 * per axis. */
void HuSpr3DSet(HUSPRID sprId)
{
    HUSPR_3DDATA *data3D;
    HUSPRITE *sprite;
    int patternIndex;
    int layerIndex;
    int maxWidth;
    int maxHeight;
    int gridSize;
    int columnCount;
    int rowCount;
    
    sprite = &HuSprData[sprId];
    HuSpr3DHookSet(sprId, HuSpr3DDisp);
    maxWidth=maxHeight=0;
    for(patternIndex=0; patternIndex<sprite->data->patNum; patternIndex++) {
        for(layerIndex=0; layerIndex<sprite->data->pat[patternIndex].layerNum; layerIndex++) {
            if(maxWidth < sprite->data->pat[patternIndex].layer[layerIndex].sizeX) {
                maxWidth = sprite->data->pat[patternIndex].layer[layerIndex].sizeX;
            }
            if(maxHeight < sprite->data->pat[patternIndex].layer[layerIndex].sizeY) {
                maxHeight = sprite->data->pat[patternIndex].layer[layerIndex].sizeY;
            }
        }
    }
    if(maxWidth < 16) {
        columnCount = 1;
    } else {
        columnCount = maxWidth/16;
    }
    if(maxHeight < 16) {
        rowCount = 1;
    } else {
        rowCount = maxHeight/16;
    }
    gridSize = (columnCount+1)*(rowCount+1);
    data3D = HuSpr3DDataCreate(sprId, sizeof(HUSPR_3DDATA) + (gridSize * sizeof(HuVecF)) +
                                          (gridSize * sizeof(HuVec2f)));
    data3D->vtx = (HuVecF *)&data3D[1];
    data3D->st = (HuVec2f *)&data3D->vtx[gridSize];
    data3D->rot.x = data3D->rot.y = data3D->rot.z = 0;
    data3D->col = columnCount;
    data3D->row = rowCount;
    data3D->depthScale = 2*HuSin(30);
}

/* Sprite clients set custom mesh rotation here when per-sprite 3D data is present. */
void HuSpr3DRotSet(HUSPRID sprId, float x, float y, float z)
{
    HUSPRITE *sprite = &HuSprData[sprId];
    HUSPR_3DDATA *data3D = sprite->data3D;
    if(data3D) {
        data3D->rot.x = x;
        data3D->rot.y = y;
        data3D->rot.z = z;
    }
}

/* Sprite clients set mesh depth scaling here from half the supplied field of view. */
void HuSpr3DFovSet(HUSPRID sprId, float fov)
{
    HUSPRITE *sprite = &HuSprData[sprId];
    HUSPR_3DDATA *data3D = sprite->data3D;
    if(data3D) {
        data3D->depthScale = 2*HuSin(fov/2);
    }
}

/* Enables and copies the four corner colors used to tint a member's vertices. */
void HuSprVtxColorSet(HUSPR_GROUPID grpId, s16 memberNo, GXColor *vtxColor)
{
    HUSPRID sprId = HuSprGrpData[grpId].sprId[memberNo];
    HUSPRITE *sprite = &HuSprData[sprId];
    s16 cornerIndex;
    sprite->attr |= HUSPR_ATTR_VTXCOLOR;
    for(cornerIndex=0; cornerIndex<4; cornerIndex++) {
        sprite->vtxColor[cornerIndex] = vtxColor[cornerIndex];
    }
}

/* Disables per-vertex color tinting for the selected group member. */
void HuSprVtxColorReset(HUSPR_GROUPID grpId, s16 memberNo)
{
    HUSPRID sprId = HuSprGrpData[grpId].sprId[memberNo];
    HUSPRITE *sprite = &HuSprData[sprId];
    sprite->attr &= ~HUSPR_ATTR_VTXCOLOR;
}

/* AnimDebug prints an animation's pattern, bank, frame, and bitmap metadata. */
void AnimDebug(ANIMDATA *anim)
{
    ANIMPAT *pat;
    ANIMLAYER *layer;
    s16 patternIndex;
    s16 layerIndex;
    ANIMFRAME *frame;
    ANIMBANK *bank;
    ANIMBMP *bmp;

    OSReport("patNum %d,bankNum %d,bmpNum %d\n", anim->patNum, anim->bankNum,
             anim->bmpNum & ANIM_BMP_NUM_MASK);
    pat = anim->pat;
    for(patternIndex=0; patternIndex<anim->patNum; patternIndex++) {
        OSReport("PATTERN%d:\n", patternIndex);
        OSReport("\tlayerNum %d,center (%d,%d),size (%d,%d)\n", pat->layerNum, pat->centerX,
                 pat->centerX, pat->sizeX, pat->sizeY);
        layer = pat->layer;
        for(layerIndex=0; layerIndex<pat->layerNum; layerIndex++) {
            OSReport("\t\tfileNo %d,flip %x\n", layer->bmpNo, layer->flip);
            OSReport("\t\tstart (%d,%d),size (%d,%d),shift (%d,%d)\n", layer->startX, layer->startY,
                     layer->sizeX, layer->sizeY, layer->shiftX, layer->shiftY);
            if(layerIndex != pat->layerNum-1) {
                OSReport("\n");
            }
            layer++;
        }
        pat++;
    }
    bank = anim->bank;
    for(patternIndex=0; patternIndex<anim->bankNum; patternIndex++) {
        OSReport("BANK%d:\n", patternIndex);
        OSReport("\ttimeNum %d\n", bank->timeNum);
        frame = bank->frame;
        for(layerIndex=0; layerIndex<bank->timeNum; layerIndex++) {
            OSReport("\t\tpat %d,time %d,shift(%d,%d),flip %x\n", frame->pat, frame->time,
                     frame->shiftX, frame->shiftY, frame->flip);
            frame++;
        }
        bank++;
    }
    bmp = anim->bmp;
    for(patternIndex=0; patternIndex<anim->bmpNum & ANIM_BMP_NUM_MASK; patternIndex++) {
        OSReport("BMP%d:\n", patternIndex);
        OSReport("\tpixSize %d,palNum %d,size (%d,%d)\n", bmp->pixSize, bmp->palNum, bmp->sizeX,
                 bmp->sizeY);
        bmp++;
    }
}
