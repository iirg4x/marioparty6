// Owns the board models and updates the map lighting.

#include "REL/m677DLL/seer_terror_map.h"

#include "REL/m677DLL/game_constants.h"
#include "REL/framework/scene_shadow_object.h"
#include "REL/framework/world_units.h"

#include "datadir_enum.h"

extern "C" {
#include "game/hu3d.h"
}

// Initializes the map models, lights, and shadow camera.
SeerTerrorMap::SeerTerrorMap()
    : GameObject(8),
      firstModel(DATANUM(_m677 << 16, 0x0)),
      secondModel(DATANUM(_m677 << 16, 0x1)),
      joint(DATANUM(_m677 << 16, 0x2)),
      scene(new CameraFacingSceneModels)
{
    // Renders the two map meshes in separate layers.
    firstModel.setLayer(3);
    secondModel.setLayer(0);
    fn_1_191FC();

    // The shadow camera faces down toward the center of the map.
    Vec direction = {
        0.0f,
        -1.0f,
        -0.001f
    };
    Vec target = { 0.0f, 150.0f, 0.0f };
    configureShadowCamera(
        1,
        &direction,
        &target,
        2000.0f,
        820.0f,
        120.0f
    );
    Hu3DShadowTPLvlSet(0.3f);
}

// Releases the board's scene models.
SeerTerrorMap::~SeerTerrorMap()
{
}

// Rotates the two meshes relative to the current camera.
void SeerTerrorMap::update()
{
    scene->updateJointRotations();
}

// Sets the direction of the two dynamic lights in the first map model.
void SeerTerrorMap::fn_1_191FC()
{
    static char *lightNames[] = { "light1", "light2" };

    // These lights are stored in the model data.
    int lights = firstModel.setLightsDynamic(false);
    assert(lights > 0);
    for (int index = 0; index < 2; ++index) {
        HU3D_LIGHTID light = fn_80029D24(firstModel, lightNames[index]);
        Vec position;
        Vec direction;
        GXColor color;
        fn_800297D4((short) light, &position, &direction, &color);
        static const Vec target = { 0.0f, 0.0f, 1.0f };
        Hu3DGLightPosSetV((short) light, &position, const_cast<Vec *>(&target));
    }
}
