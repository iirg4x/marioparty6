// The asserts keep the original file name and lines (man_map.cpp, lines 58 and 72); the text of the
// lines before and between them is unknown. NDEBUG is undefined because the build defines it for
// every object while the game kept these asserts.
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/m639DLL/material_animation.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
// The map scene: its five models, the scene light and the events that start its motion.
#include "REL/m639DLL/scene_owner.h"
#include "REL/m639DLL/model_pair_owner.h"
#include "REL/m639DLL/lighthouse_light.h"
#include "REL/m639DLL/mode_game.h"
#include "REL/framework/sound.h"
// Lighthouse entry points are declared in lighthouse_light.h.
#include "REL/m639DLL/game_constants.h"

extern "C" {
#include "game/data.h"
}

/* Lighthouse entry points are declared in lighthouse_light.h.
   The constructor registers both lights in their game-object group.
 */

SceneOwner::SceneOwner()
    : sceneModel0(DATANUM(0x620000, 0)),
      sceneModel1(DATANUM(0x620000, 1)),
      sceneModel2(DATANUM(0x620000, 2)),
      animationModel(DATANUM(0x620000, 7)),
      lightModel(DATANUM(0x620000, 5))
{
    int lights;

    // The map's three models; the second one's material is animated.
    sceneModel0.setLayer(0);
    sceneModel1.setLayer(1);
    sceneModel1.setAttribute(((1 << 30) | (1 << 0)));
    sceneModel2.setLayer(2);
    sceneModel2.setAttribute(((1 << 30) | (1 << 0)));
    new MaterialAnimation(&sceneModel1);

    animationModel.setLayer(2);
    animationModel.setAttribute(((1 << 30) | (1 << 1)));

    // The lighthouses' two lights and the fans, placed at the light model's nodes.
    lightModel.setLayer(3);
    // Each light adds itself to the game-object list, so no pointer is kept here.
    new LighthouseLight(DATANUM(0x620000, 3), 0);
    new LighthouseLight(DATANUM(0x620000, 4), 1);
    fn_1_DE1C(&lightModel);

    // The players' charge.
    new ModeGame;

    // The map needs its scene light.
    lights = fn_80029850(lightModel, 1);
    assert(lights > 0);
    fn_8002C2AC(1);
}
SceneOwner::~SceneOwner() {}
// The lookup below returns a model light id.
// The signed halfword id selects the light whose position,
// direction and color are written into the supplied outputs.
// Only the direction is returned to this query's caller.
// Finds the named model light and returns its current direction to the caller.
Vec *SceneOwner::fn_1_5AAC(Vec *direction)
{
    Vec position;
    GXColor color;
    short light_id = fn_80029D24(lightModel, "inf_light1");
    assert(light_id >= 0);
    fn_800297D4((s16)light_id, &position, direction, &color);
    return direction;
}

// Event zero runs the scene callback; event three resumes model motion and plays a sound.
void SceneOwner::flagEnabled(int event)
{
    switch (event) {
    case 0:
        fn_1_CD58();
        break;
    case 3:
        animationModel.resetAttribute(((1 << 30) | (1 << 1)));
        soundPlay(1901);
        flags &= ~(1UL << event);
        break;
    default:
        flags &= ~(1UL << event);
        break;
    }
}

void SceneOwner::update()
{
    if (fn_1_CE60() != 0) {
        flags &= ~1;
    }
}
