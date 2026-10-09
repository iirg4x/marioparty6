/* Manages game sprites by reusing loaded animation data and placing sprites in one group. */
#define _MATH_H
#include "game/esprite.h"
#include "game/data.h"
#include "game/sprite.h"

typedef struct esprite_s {
    s16 groupMemberIndex; /* Slot used to address this sprite in the shared sprite group. */
    s16 animationSlot; /* Index of the shared animation record used by this sprite. */
} ESPRITE;

typedef struct espanim_s {
    /* 0x00 */ unsigned int dataNumber; /* Resource number for this animation. */
    /* 0x04 */ u16 referenceCount; /* Number of live sprites using this animation. */
    /* 0x08 */ ANIMDATA *animation; /* Sprite animation data shared by those sprites. */
} ESPANIM;

ESPRITE esprite[HUSPR_MAX];
ESPANIM espanim[HUSPR_MAX];

static HUSPR_GROUPID gid; /* Group containing every sprite managed by this file. */

/* Called by game initialization before game or board code requests managed sprites. */
void espInit(void) {
    s32 slotIndex;

    gid = HuSprGrpCreate(HUSPR_MAX);
    for (slotIndex = 0; slotIndex < HUSPR_MAX; slotIndex++) {
        esprite[slotIndex].groupMemberIndex = slotIndex;
        esprite[slotIndex].animationSlot = HUSPR_NONE;
    }
    for (slotIndex = 0; slotIndex < HUSPR_MAX; slotIndex++) {
        espanim[slotIndex].referenceCount = 0;
    }
}

/* Game and board code call this when creating an animation-backed sprite. */
s16 espEntry(unsigned int dataNumber, s16 priority, s16 bank)
{
    ESPANIM *unusedAnimation;
    ESPANIM *animation;
    ESPRITE *spriteSlot;
    void *animationResource;
    s16 spriteId;
    s16 slotIndex;
    s32 animationSlotIndex;

    spriteSlot = esprite;
    for (slotIndex = 0; slotIndex < HUSPR_MAX; spriteSlot++, slotIndex++) {
        if (spriteSlot->animationSlot == HUSPR_NONE) {
            break;
        }
    }
    if (slotIndex == HUSPR_MAX) {
        return HUSPR_NONE;
    }
    animation = espanim;
    unusedAnimation = NULL;
    for (animationSlotIndex = 0; animationSlotIndex < HUSPR_MAX;
         animation++, animationSlotIndex++) {
        if (animation->referenceCount != 0) {
            if (animation->dataNumber == dataNumber) {
                unusedAnimation = NULL; /* Keep the cached animation alive for its other sprites. */
                break;
            }
        } else if (unusedAnimation == NULL) {
            unusedAnimation = animation;
        }
    }
    if (animationSlotIndex == HUSPR_MAX) {
        if (unusedAnimation == NULL) {
            return HUSPR_NONE;
        }
        animationResource = HuDataSelHeapReadNum(dataNumber, HU_MEMNUM_OVL, HEAP_MODEL);
        if (animationResource == NULL) {
            return HUSPR_NONE;
        }
        unusedAnimation->dataNumber = dataNumber;
        unusedAnimation->animation = HuSprAnimRead(animationResource);
        animation = unusedAnimation;
    }
    spriteId = HuSprCreate(animation->animation, priority, bank);
    if (spriteId == HUSPR_NONE) {
        OSReport("Error: Esprite Max Over!\n");
        if (unusedAnimation != NULL) {
            /* This call loaded the animation, but could not create a sprite to use it. */
            HuSprAnimKill(animation->animation);
        }
        return HUSPR_NONE;
    }
    animation->referenceCount++;
    spriteSlot->animationSlot = animation - espanim;
    HuSprGrpMemberSet(gid, spriteSlot->groupMemberIndex, spriteId);
    return slotIndex;
}

/* Game and board cleanup code call this to remove a sprite from the group and decrement its
 * animation live-user count; the sprite manager releases the sprite's animation reference and
 * frees the animation data when no sprites use it. */
void espKill(s16 spriteSlotIndex)
{
    HuSprGrpMemberKill(gid, esprite[spriteSlotIndex].groupMemberIndex);
    espanim[esprite[spriteSlotIndex].animationSlot].referenceCount--;
    esprite[spriteSlotIndex].animationSlot = HUSPR_NONE;
}

/* Returns the shared group so callers can configure sprites through sprite APIs. */
HUSPR_GROUPID espGrpIDGet(void)
{
    return gid;
}

/* Game and board display code call this to clear the sprite's hidden flag. */
void espDispOn(s16 spriteSlotIndex)
{
    HuSprAttrReset(gid, esprite[spriteSlotIndex].groupMemberIndex, HUSPR_ATTR_DISPOFF);
}

/* Game and board display code call this to hide the sprite until it is enabled again. */
void espDispOff(s16 spriteSlotIndex)
{
    HuSprAttrSet(gid, esprite[spriteSlotIndex].groupMemberIndex, HUSPR_ATTR_DISPOFF);
}

void espAttrSet(s16 spriteSlotIndex, u16 attributes)
{
    HuSprAttrSet(gid, esprite[spriteSlotIndex].groupMemberIndex, attributes);
}

void espAttrReset(s16 spriteSlotIndex, u16 attributes)
{
    HuSprAttrReset(gid, esprite[spriteSlotIndex].groupMemberIndex, attributes);
}

void espPosSet(s16 spriteSlotIndex, float posX, float posY)
{
    HuSprPosSet(gid, esprite[spriteSlotIndex].groupMemberIndex, posX, posY);
}

void espScaleSet(s16 spriteSlotIndex, float scaleX, float scaleY)
{
    HuSprScaleSet(gid, esprite[spriteSlotIndex].groupMemberIndex, scaleX, scaleY);
}

void espZRotSet(s16 spriteSlotIndex, float zRot)
{
    HuSprZRotSet(gid, esprite[spriteSlotIndex].groupMemberIndex, zRot);
}

void espTPLvlSet(s16 spriteSlotIndex, float alphaLevel)
{
    HuSprTPLvlSet(gid, esprite[spriteSlotIndex].groupMemberIndex, alphaLevel);
}

void espColorSet(s16 spriteSlotIndex, u8 red, u8 green, u8 blue)
{
    HuSprColorSet(gid, esprite[spriteSlotIndex].groupMemberIndex, red, green, blue);
}

void espSpeedSet(s16 spriteSlotIndex, float speed)
{
    HuSprSpeedSet(gid, esprite[spriteSlotIndex].groupMemberIndex, speed);
}

void espBankSet(s16 spriteSlotIndex, s16 bank)
{
    HuSprBankSet(gid, esprite[spriteSlotIndex].groupMemberIndex, bank);
}

void espDrawNoSet(s16 spriteSlotIndex, s16 drawNo)
{
    HuSprDrawNoSet(gid, esprite[spriteSlotIndex].groupMemberIndex, drawNo);
}

void espPriSet(s16 spriteSlotIndex, s16 priority)
{
    HuSprPriSet(gid, esprite[spriteSlotIndex].groupMemberIndex, priority);
}
