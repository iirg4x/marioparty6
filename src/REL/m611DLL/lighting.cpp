#include "datadir_enum.h"
#include "REL/m611DLL/game_requests.h"
#include "REL/m611DLL/motion_light_owner.h"
#include "REL/framework/model.h"
#include "REL/framework/scene_shadow.h"
extern "C" {
#include "dolphin/mtx.h"
#include "dolphin/types.h"
#include "math.h"
#include "game/hu3d.h"
#include "humath.h"
#include <dolphin/gx.h>
}

// Creates the scene models and configures the daytime shadow camera.
MotionLightOwner::MotionLightOwner(bool dayMode)
    : primaryModel(dayMode ? DATANUM(0x460000, 0) :
                                    DATANUM(0x460000, 3)),
      secondaryModel(dayMode ? DATANUM(0x460000, 2) :
                                      DATANUM(0x460000, 4))
{
    s16 primaryModelHandle;

    primaryModelHandle = primaryModel;
    fn_80029850(primaryModelHandle, 1);
    this->primaryModel.setLayer(3);
    this->primaryModel.setAttribute(0x40000001);
    this->secondaryModel.setLayer(0);
    this->secondaryModel.setAttribute(0x40000001);
    if (dayMode) {
        Vec source = {0.0f, -1.0f, 0.01f};
        Vec target = {0.0f, 0.0f, 0.0f};
        configureShadowCamera(1, &source, &target, 3000.0f, 3000.0f, 5.0f);
        Hu3DShadowTPLvlSet(0.2f);
    }
}

// Leaves the scene lighting unchanged during the frame update.
void MotionLightOwner::update()
{
}

// Acknowledges scene requests without changing the lighting models.
void MotionLightOwner::flagEnabled(int request)
{
    acknowledgeSceneRequest(flags, request);
}

// Begins drawing and forwards the camera matrix to the lighting renderer.
// Nothing in this minigame calls this.
void fn_1_A8D8(void *, const Mtx camera)
{
    MotionLightOwner *owner;
    MotionLightOwner *sceneLighting;

    fn_8002472C();
    sceneLighting = MinigameSingleton<MotionLightOwner>::self;
    owner = sceneLighting;
    fn_1_A928(owner, camera);
}

// Loads the draw matrix and renders the ground ring.
void fn_1_A928(MotionLightOwner *, const Mtx matrix)
{
    GXLoadPosMtxImm(matrix, GX_PNMTX0);
    fn_1_A958();
}

// Builds and draws the translucent annular ground strip.
void fn_1_A958()
{
    f32 vertexX[80];
    f32 vertexZ[80];
    s32 segment;
    s32 vertexCount;
    f32 angle;
    f32 radians;

    vertexCount = 0;
    for (segment = 0; segment < 33; ++segment) {
        angle = (360.0f * (static_cast<f32>(segment & 0x1F) / 32.0f)) - 180.0f;
        angle += 90.0f;
        if (angle > 180.0f) {
            angle -= 360.0f;
        }
        radians = static_cast<f32>((M_PI * static_cast<f64>(angle)) / 180.0);
        vertexX[vertexCount] = static_cast<f32>(100.0 * (11.5 * cos(radians)));
        vertexZ[vertexCount] = static_cast<f32>(100.0 * (11.5 * sin(radians)));
        vertexCount += 1;
        vertexX[vertexCount] = static_cast<f32>(100.0 * (4.5 * cos(radians)));
        vertexZ[vertexCount] = static_cast<f32>(100.0 * (4.5 * sin(radians)));
        vertexCount += 1;
    }

    configurePositionColorDrawing(false);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, static_cast<u16>(vertexCount));
    for (segment = 0; segment < vertexCount; ++segment) {
        GXPosition3f32(vertexX[segment], 0.0f, vertexZ[segment]);
        GXColor4u8(64, 64, 64, 255);
    }
}

// Releases the lighting models and their inherited registrations.
MotionLightOwner::~MotionLightOwner()
{
}
