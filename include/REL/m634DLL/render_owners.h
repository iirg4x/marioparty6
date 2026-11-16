#ifndef REL_M634DLL_RENDER_OWNERS_H
#define REL_M634DLL_RENDER_OWNERS_H
#include "REL/m634DLL/mode_render_object.h"
extern "C" {
#include "game/animdata.h"
}

struct Camera1Renderer : ModeRenderObject {
    Camera1Renderer();
    // Releases the object and its member-owned resources.
    virtual ~Camera1Renderer() { }
    virtual void update();
    virtual void draw(Mtx *);
    virtual void mode(int);
};
struct Camera2Renderer : ModeRenderObject {
    Camera2Renderer();
    // Releases the object and its member-owned resources.
    virtual ~Camera2Renderer() { }
    virtual void update();
    virtual void draw(Mtx *);
    virtual void mode(int);
};
struct SpriteRenderer : ModeRenderObject {
    ANIMDATA *texture;
    s32 colorIndex;
    explicit SpriteRenderer(ANIMDATA *);
    // Releases the object and its member-owned resources.
    virtual ~SpriteRenderer() { }
    virtual void update();
    virtual void draw(Mtx *);
};
#endif
