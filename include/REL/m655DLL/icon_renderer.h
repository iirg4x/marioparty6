#ifndef REL_M655DLL_ICON_RENDERER_H
#define REL_M655DLL_ICON_RENDERER_H
#include "REL/framework/game_object2.h"
#include "REL/framework/frame_interpolator.h"
#include "game/sprite.h"
struct IconPosition {
    float components[3];
};
struct IconRenderer : RenderObject {
    IconRenderer(s32 primaryAnimation, s32 secondaryAnimation);
    virtual ~IconRenderer();
    virtual void update();
    void fn_1_59C4(int pos);
    void fn_1_5A88();
    void fn_1_5ADC();
    virtual void draw(Mtx *projection);
    ANIMDATA *primaryAnimation;
    ANIMDATA *secondaryAnimation;
    ANIMDATA *idleAnimation;
    IconPosition initialVector;
    FrameInterpolator<float> transition;
/* Unknown final four bytes of the 0x44-byte object. */
    u8 unknown40[4]; /* No code names this field. */
};
#endif
