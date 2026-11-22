// Declares the solid-color overlay drawn across the screen.

#ifndef REL_M673DLL_SCREEN_OVERLAY_H
#define REL_M673DLL_SCREEN_OVERLAY_H
#include "REL/framework/game_object2.h"
#include "dolphin/gx.h"

// Draws a solid color over the frame.
struct ScreenOverlay : RenderObject {
    GXColor color; // RGB input for the overlay.

    // Creates a screen overlay using the supplied color.
    ScreenOverlay(GXColor value);

    // Releases the screen-overlay render object.
    virtual ~ScreenOverlay();

    // Keeps the overlay color between draws.
    virtual void update();

    // Draws a screen-sized rectangle with stored RGB and a separate TEV constant for alpha.
    virtual void draw(Mtx *);
};

#endif
