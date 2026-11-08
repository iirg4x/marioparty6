#ifndef REL_M663DLL_SCORE_H
#define REL_M663DLL_SCORE_H

#include "dolphin/types.h"
#include "humath.h"

typedef struct M663ScoreAttribute {
    s16 spriteId;
    s16 groupId;
    s16 memberId;
    u8 unk_06[6]; /* No code names this field. */
    HuVec2f position;
    s16 width;
    s16 height;
    f32 depth;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    f32 transparency;
    s16 animationId;
} M663ScoreAttribute;

typedef struct M663SpriteGroup {
    s16 groupId;
    HuVec2f position;
    f32 scaleX;
    f32 scaleY;
} M663SpriteGroup;

extern M663ScoreAttribute lbl_1_bss_120[14];
extern M663SpriteGroup lbl_1_bss_350[2];
void fn_1_D83C(void);
void fn_1_DC48(void);

#endif
