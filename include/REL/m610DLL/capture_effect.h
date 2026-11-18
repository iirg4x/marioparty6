#ifndef M610_CAPTURE_EFFECT_H
#define M610_CAPTURE_EFFECT_H

#include "dolphin/types.h"
#include "dolphin/gx.h"
#include "REL/framework/model.h"

#include "REL/framework/game_object.h"
#include "game/animdata.h"
/* Captures the screen and draws the scrolling textured effect. */
struct CaptureEffect : GameObject, OwnedModel
{
    CaptureEffect(ANIMDATA *, ANIMDATA *);
    virtual ~CaptureEffect();
    virtual void update();
    void fn_1_3BC(Mtx);
    void fn_1_514(MtxPtr);
    /* Texture object backed by the captured frame pixels. */
    GXTexObj texture;
    /* Frame capture buffer in the texture format used by GX. */
    u8 pixels[640 * 480 * 2] ATTRIBUTE_ALIGN(32);
    /* Texture animation data used to draw the captured layer. */
    ANIMDATA *resource;
    ANIMDATA *unk96044; /* The game's code never reads this field. */
    /* Selects the capture effect drawing mode. */
    s32 mode;
    /* Scroll offsets supplied to the indirect texture matrices. */
    f32 scrollX, scrollZ;
    f32 unk96054; /* The game's code never reads this field. */
    f32 unk96058; /* The game's code never reads this field. */
    /* Number of completed effect updates. */
    u32 updateCount;
};

#endif
