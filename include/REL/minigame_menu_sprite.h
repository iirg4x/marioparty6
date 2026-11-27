#ifndef REL_MINIGAME_MENU_SPRITE_H
#define REL_MINIGAME_MENU_SPRITE_H

#include "humath.h"

/* Sprite resource, group placement, and transform for a minigame menu. */
typedef struct MinigameMenuSprite {
    /* Destination sprite group and member indices. */
    s16 groupNo;
    s16 memberNo;
    /* Index into the menu's loaded sprite animation resources. */
    s16 animNo;
    /* Sprite draw priority and initial animation bank. */
    s16 priority;
    s16 bank;
    /* Local screen position in pixels and X/Y scale. */
    HuVec2f pos;
    HuVec2f scale;
    /* Initial rotation in degrees around the screen's Z axis. */
    f32 zRot;
} MinigameMenuSprite;

#endif
