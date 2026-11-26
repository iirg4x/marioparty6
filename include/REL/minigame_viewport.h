#ifndef REL_MINIGAME_VIEWPORT_H
#define REL_MINIGAME_VIEWPORT_H


typedef struct MinigameViewport {
    float y; /* Vertical origin in screen pixels. */
    float x; /* Horizontal origin in screen pixels. */
    float width; /* Viewport width in screen pixels. */
    float height; /* Viewport height in screen pixels. */
} MinigameViewport;

#endif
