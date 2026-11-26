#ifndef REL_M634DLL_TEXTURECONTROLLER_H
#define REL_M634DLL_TEXTURECONTROLLER_H

#include "REL/m634DLL/scene.h"
#include "REL/framework/game_object2.h"
#include "dolphin/types.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
extern "C" {
#include "game/main.h"
}
extern "C" {
#include "game/object.h"
}
extern "C" {
#include "game/charman.h"
}
extern "C" {
#include "game/hu3d.h"
}
extern "C" {
#include "game/gamework.h"
}
extern "C" {
#include "game/wipe.h"
}
extern "C" {
#include "game/mg/timer.h"
}
extern "C" {
#include "game/animdata.h"
}
extern "C" {
#include "game/sprite.h"
}
#include "REL/m634DLL/mode_render_object.h"

struct TextureController : MinigameSingleton<TextureController>, ModeRenderObject {
    GXTexObj texture;
    unsigned char unk34[12]; /* No code names this field. */
    unsigned char frameBuffer[640 * 480 * 2];
    TextureController();
    virtual ~TextureController();
    virtual void update();
    virtual void draw(Mtx *);
};

#endif
