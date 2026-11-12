#include "REL/m639DLL/shadow_fit.h"
#include "REL/m639DLL/lighting_types.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
// The shadow fit, the sprite quads and the lighthouse lights that follow the camera.
#include "REL/m639DLL/transition.h"
#include <vector>
#include <cmath>
#include "REL/framework/game_object.h"
#include "REL/framework/camera_frame.h"
#include "REL/framework/scene_shadow.h"
#include "REL/m639DLL/mode_game.h"
#include "REL/m639DLL/mode_selection.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/model.h"
#include "REL/m639DLL/resource_parameters.h"
#include "REL/m639DLL/sphere.h"

extern "C" {
#include "dolphin/gx.h"
#include "REL/m639DLL.h"
}
#include "humath.h"

void collectGameObjects(int category, std::vector<GameObject *> *list);

#include "REL/m639DLL/lighthouse_light.h"

extern "C" {
extern const GXVtxDescList lbl_1_rodata_430[];
extern const GXVtxAttrFmtList lbl_1_rodata_448[];
extern float lbl_1_bss_C8;
extern char *lbl_1_data_508[4];
extern AnimationResourcePair lbl_1_data_5B8[5];
void __msl_assertion_failed(const char *, const char *, int);
void fn_1_DECC(std::vector<Sphere> *positions);
void fn_1_C858();
void fn_1_CD58();
void fn_1_D69C(LighthouseLight *owner);


void fn_1_111DC(void *object);
}

// Builds a shadow from the model positions collected by both component lists.
void fn_1_C28C(Vec *position)
{
    CameraFrame shadow;
    std::vector<Sphere> actorPositions;
    fn_1_68D8(&actorPositions);
    int actorLoopLimit = actorPositions.size();
    std::vector<Sphere> selectedPositions;
    fn_1_DECC(&selectedPositions);
    int selectedLoopLimit = selectedPositions.size();

    std::vector<Sphere *> positions;
    for (int index = 0; index < actorLoopLimit; ++index) {
        positions.push_back(&actorPositions[index]);
    }

    for (int index = 0; index < selectedLoopLimit; ++index) {
        positions.push_back(&selectedPositions[index]);
    }

    CameraPerspective context;
    context.marginLeft = 8;
    context.marginRight = 8;
    context.marginTop = 8;
    context.marginBottom = 8;
    context.fovRadians = 0.17453292f;
    shadow.position.x = 0.0f;
    shadow.position.y = 0.0f;
    shadow.position.z = 0.0f;
    PSVECScale(position, &shadow.target, -1.0f);
    shadow.up.x = 0.0f;
    shadow.up.y = 1.0f;
    shadow.up.z = 0.0f;

    float range = fitCameraToSpheres(&shadow, &context, &positions);
    Hu3DShadowCreate(10.0f, 0.1f, range);
    Hu3DShadowPosSet(&shadow.position, &shadow.up, &shadow.target);
    Hu3DShadowSizeSet(0x1E0);
    Hu3DShadowTPLvlSet(0.4f);
}
