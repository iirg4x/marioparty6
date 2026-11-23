// Updates and draws the icon that marks the selected rope.

#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/motion_attributes.h"
#include "REL/m677DLL/icon_renderer.h"
#include "REL/framework/world_units.h"

extern "C" {
#include "game/hu3d.h"
#include "game/hsfex.h"
}
#include "dolphin/mtx.h"
#include "datadir_enum.h"

#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"

#define numberof(array) (int)(sizeof(array) / sizeof((array)[0]))

namespace {

// Positions used by the rope selection icon.
Vec icon_pos[] = {
    {cmToWorld(-400.0f), cmToWorld(200.0f), cmToWorld(350.0f)},
    {cmToWorld(-135.0f), cmToWorld(200.0f), cmToWorld(350.0f)},
    {cmToWorld(135.0f), cmToWorld(200.0f), cmToWorld(350.0f)},
    {cmToWorld(400.0f), cmToWorld(200.0f), cmToWorld(350.0f)}
};
}

// Sets the icon's render layer and model attributes.
IconRenderer::IconRenderer()
    : GameObject(9),
      OwnedModel(DATANUM(_m677 << 16, 0x13)),
      position(icon_pos[2]),
      transition(0.0f)
{
    setLayer(6);
    setAttribute(loopMotion);
}

// Releases the icon's model state.
IconRenderer::~IconRenderer()
{
}

// Places the icon at the indexed rope position.
void IconRenderer::setSlot(int pos)
{
    assert((pos >= 0) && (pos < numberof(icon_pos)));
    position.x = icon_pos[pos].x;
    position.y = icon_pos[pos].y;
    position.z = icon_pos[pos].z;
}

// Grows the icon toward scale 0.5 while moving it laterally.
void IconRenderer::beginGrow()
{
    transition.set(0.5f, secondsToFrames<double>(0.2));
}

// Shrinks the icon toward zero scale while moving it laterally.
void IconRenderer::beginShrink()
{
    transition.set(0.0f, secondsToFrames<double>(0.1));
}

// Updates the icon in the current camera frame.
void IconRenderer::update()
{
    Vec eye;
    Vec target;
    Vec up;
    Mtx matrix;
    fn_80027CD0(1, &eye, &up, &target);
    C_MTXLookAt(matrix, &eye, &up, &target);
    PSMTXTranspose(matrix, matrix);
    updatePosition(matrix);
    faceCamera(matrix);
    setUniformScale(transition.update());
}

// Interpolates the icon's camera-relative horizontal offset and rotates it into world space.
void IconRenderer::updatePosition(Mtx matrix)
{
    f64 x;
    f32 current = transition.value;
    f64 ratio = (f64)current / 0.5;
    f64 xOffset = (1.0 - ratio) * cmToWorld(-50.0) + ratio * cmToWorld(-100.0);
    f64 translatedXOffset = xOffset;
    x = (f32)translatedXOffset;
    Vec translation;
    Vec *iconPosition;

    translation.x = (f32)x;
    translation.y = 0.0f;
    translation.z = 0.0f;
    PSMTXMultVecSR(matrix, &translation, &translation);
    PSVECAdd(&this->position, &translation, &translation);
    iconPosition = &translation;
    setPosition(iconPosition);
}

// Aligns the icon with the camera and applies a fixed roll.
void IconRenderer::faceCamera(Mtx matrix)
{
    Vec rotation;
    Mtx local;
    MTXRotDeg(local, 'z', 45.0f);
    PSMTXConcat(matrix, local, local);
    Hu3DMtxRotGet(local, &rotation);
    setRotation(&rotation);
}
