#include <new>
#include "REL/m655DLL/game_constants.h"
#include "REL/m655DLL/effect_model.h"
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"

extern "C" {
#include "game/charman.h"
}
#include "REL/framework/frame_time.h"
#include "REL/framework/model.h"
#include "REL/framework/scene_shadow.h"
#include "dolphin/types.h"
#include "REL/framework/world_units.h"

static inline f32 interpolateCoordinate(f32 first, f32 second)
{
    return 0.9 * first + 0.1 * second;
}

EffectModel::EffectModel() : OwnedModel(0x720000)
{
    GXColor color;
    s32 lights;
    Vec initialPosition = { 0.0f, 0.0f, 0.0f };
    initialPosition.x = -30.0f;
    setLayer(1);
    setPosition(&initialPosition);
    lights = setLightsDynamic(false);

/* Retain assertion source location.
 *
 */
    assert(lights > 0);
    Vec basePosition = { 0.0f, -1.0f, 0.0f };
    Vec resultPosition;
    Vec modelPosition;
    setLight("inf_light1", &modelPosition, &resultPosition, &color);
    resultPosition.x = interpolateCoordinate(basePosition.x, resultPosition.x);
    resultPosition.y = interpolateCoordinate(basePosition.y, resultPosition.y);
    resultPosition.z = interpolateCoordinate(basePosition.z, resultPosition.z);
    new SceneShadow(1, &resultPosition, 5.0f, 10);
}

EffectModel::~EffectModel() {}
