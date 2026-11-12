/* Spacing preserves the assertion's source line.
 */
/* Preserve the assertion's source line. */
// The map: the gear and star models on their stand, their light and shadow, and the motion that
// the attackers' landings drive (attacker.cpp).
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/m624DLL/camera_transition.h"
#include "REL/framework/sound.h"
#include "REL/m624DLL/game_constants.h"
#include "REL/m624DLL/scene_sequence_contract.h"
#include "REL/m624DLL/model_commands.h"

extern "C" {
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "game/mg/actman.h"
}

// framework/scene_shadow.cpp: aims the shadow camera along a light.
void configureShadowCamera(short mask, const Vec *direction, Vec *target, f32 range, f32 falloff,
                           f32 scale);

void fn_1_4D40(SceneOwner *owner);
void fn_1_5094(SceneOwner *owner, f32 speed);
// Hides each object a list names; the list ends with 0.
void fn_1_564C(Model *model, char **names);
void fn_1_569C(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);

// Objects hidden in the cluster and linked copies of the gear model.
char *lbl_1_data_51C[31] = {
    "gearA01", "gearA03", "gearA04", "gearC01", "gearC02", "gearC02a",
    "gearD6", "gearE05", "gearF04", "gearF9", "shaft01", "shaft02",
    "shaft05", "shaft09", "shaft10", "shaft11", "star01", "star01a",
    "star02", "star02a", "star7", "star9", "star13", "star14",
    "star15", "starshadow04", "starshadow5", "starshadow6", "starshadow7", "mesh",
    0
};

// Shadow objects, hidden in the animated and linked copies.
char *lbl_1_data_5BC[5] = { "shadow01", "shadow02", "shadow04", "shadow05", 0 };

// Objects hidden in the animated and cluster copies.
char *lbl_1_data_5F8[6] = { "gearA04a", "gearD01", "gearD03", "gearD6a", "gearD8", 0 };

// Objects of the animated and linked copies that keep no shadow map.
char *lbl_1_data_610[9] = { "gearD6a", "shaft01", "shaft02", "shaft05", "shaft09",
                            "shaft10", "shaft11", "star01a", "star02a" };

// Builds the map's models, aims the shadow camera along the stand model's light and starts a sound.
SceneOwner::SceneOwner()
    : baseModel(DATANUM(0x530000, 0)),
      secondModel(DATANUM(0x530000, 1)),
      startModel(DATANUM(0x530000, 4)),
      fourthModel(DATANUM(0x530000, 5)),
      animatedModel(DATANUM(0x530000, 2)),
      clusterModel(DATANUM(0x530000, 2)),
      linkedModel(animatedModel),
      lastModel(DATANUM(0x530000, 3)),
      speedTransition(1.0f),
      positionTransition(0.0f),
      initialized(1)
{
    Point3d direction;
    Point3d position;
    GXColor ambient;
    s16 light_id;
    s32 lights;

    baseModel.setLayer(1);
    baseModel.setShadowMapObject("1pdodai");
    startModel.setLayer(1);
    startModel.setAttribute(((1 << 30) | (1 << 1)));
    startModel.setShadowMapObject("start");
    fourthModel.setLayer(1);
    fourthModel.setShadowMap();
    fn_1_4D40(this);
    lights = fn_80029850(baseModel, 1);
    assert(lights > 0);
    light_id = fn_80029D24(baseModel, "inf_light1");
    assert(light_id >= 0);
    fn_800297D4((s16)light_id, &position, &direction, &ambient);
    Point3d color = { 0.0f, 655.0f, -150.0f };
    configureShadowCamera(1, &direction, &color, 2000.0f, 1200.0f, 5.0f);
    Hu3DShadowTPLvlSet(0.3f);
    soundPlay(0x6FF);
}

SceneOwner::~SceneOwner() {}

void fn_1_4D40(SceneOwner *owner) {
    f32 speed;
    s32 firstObject;
    s32 thirdObject;
    s16 clusterModel;
    s16 hookModel;
    owner->animatedModel.setLayer(1);
    owner->animatedModel.setAttribute(((1 << 30) | (1 << 0)));
    owner->animatedModel.setShadowMap();
    fn_1_564C(&owner->animatedModel, lbl_1_data_5BC);
    fn_1_564C(&owner->animatedModel, lbl_1_data_5F8);
    firstObject = 0;
    while (firstObject < 9) {
        owner->animatedModel.resetObjectShadowMap(lbl_1_data_610[firstObject]);
        firstObject += 1;
    }
    owner->clusterModel.setLayer(2);
    owner->clusterModel.setAttribute(((1 << 30) | (1 << 0)));
    fn_1_564C(&owner->clusterModel, lbl_1_data_51C);
    fn_1_564C(&owner->clusterModel, lbl_1_data_5F8);

    fn_80026AE4(owner->clusterModel, 0, ((s32)((3U << 30) | (1U << 0))));

    fn_80027884(owner->clusterModel, fn_1_569C);
    owner->linkedModel.setLayer(3);
    owner->linkedModel.setAttribute(((1 << 30) | (1 << 0)));
    owner->linkedModel.setShadowMap();
    fn_1_564C(&owner->linkedModel, lbl_1_data_51C);
    fn_1_564C(&owner->linkedModel, lbl_1_data_5BC);
    thirdObject = 0;
    while (thirdObject < 9) {
        owner->linkedModel.resetObjectShadowMap(lbl_1_data_610[thirdObject]);
        thirdObject += 1;
    }
    owner->lastModel.setAttribute(((1 << 30) | (1 << 0)));
    fn_1_4F00(owner);
    speed = owner->speedTransition.value;
    fn_1_5094(owner, speed);
}

void fn_1_4F00(SceneOwner *owner)
{
    s16 modelIds[2] = {owner->secondModel, owner->lastModel};
    s32 index;

    fn_800804E8(modelIds, 2, 0x10);
    for (index = 0; index < 2; index++) {
        fn_80080D7C(index, 1U << index);
        fn_800269E8((s32)modelIds[index], 1);
    }
}

u8 fn_1_4FBC(SceneOwner *owner)
{
    s16 timeMotion;
    s16 endMotion;
    s16 resetMotion;
    Vec result;

    if (fn_8002D4E0(owner->startModel) != 0.0f) {

        if (!fn_8002D75C(owner->startModel)) {
            return 0;
        }
    }

    fn_8002D414(owner->startModel, 0.0f);
    owner->startModel.resetAttribute(0x40000002);
    owner->startModel.getObjectPosition("door", &result);
    soundPlayAt(1, 0x703, &result);
    return 1;
}

inline void fn_1_5094(SceneOwner *owner, f32 speed)
{
    fn_8002D86C(owner->animatedModel, speed);
    fn_8002D86C(owner->clusterModel, speed);
    fn_8002D30C(owner->clusterModel, 0, speed);
    fn_8002D86C(owner->linkedModel, speed);
    fn_8002D86C(owner->lastModel, speed);
}

void fn_1_5138(SceneOwner *owner)
{
    f32 scaledRate;
    f32 clampedRate;
    f32 directionValue;
    f32 rotation;
    f32 rotationForScale;
    f64 direction;

    if (owner->initialized != 0) {
        rotation = owner->speedTransition.value;
        if (rotation < 0.0) {
            direction = -1.0;
        } else {
            direction = 1.0;
        }
        directionValue = (f32)direction;
        rotationForScale = owner->speedTransition.value;
        scaledRate = (f32)(1.0299999993294477 * (f64)rotationForScale);
        if (scaledRate < -3.0f) {
            clampedRate = -3.0f;
        } else if (scaledRate > 3.0f) {
            clampedRate = 3.0f;
        } else {
            clampedRate = scaledRate;
        }
        owner->speedTransition.set(clampedRate, directionValue, secondsToFrames(2L));
    }
}

f32 fn_1_5284(SceneOwner *owner) {
    return owner->speedTransition.value;
}

void fn_1_528C(SceneOwner *owner)
{
    f64 selectedRate;
    f32 rate;
    f32 currentRate;

    currentRate = owner->speedTransition.value;
    if (currentRate < 0.0) {
        selectedRate = 1.0;
    } else {
        selectedRate = -1.0;
    }
    rate = (f32)selectedRate;
    owner->speedTransition.set(rate, secondsToFrames(1L));
    owner->initialized = 0;
    soundPlay(0x700);
}

extern "C" {
extern "C++" { extern void fn_1_6298(void); }
}

void fn_1_5344(SceneOwner *owner)
{
    if (currentRequest() == 2) {
        owner->positionTransition.set(-20.0f, 0.0f, secondsToFrames(2L));
    }
    fn_1_6298();
    fn_1_40F4(&GameCamera::getChecked(), 1.0f);
    soundPlay(0x701);
}

void fn_1_5410(SceneOwner *owner, Vec *position)
{
    owner->animatedModel.setPosition(position);
    owner->clusterModel.setPosition(position);
    owner->linkedModel.setPosition(position);
    owner->lastModel.setPosition(position);
}

void SceneOwner::flagEnabled(int bit)
{
    flags &= ~(1UL << bit);
    if (bit == 1) {
        fn_1_2168(&manager);
    }
}

void SceneOwner::update()
{
    f64 positionY;
    positionY = positionTransition.update();
    Vec zeroPosition;
    zeroPosition.x = 0.0f;
    zeroPosition.y = (f32)positionY;
    zeroPosition.z = 0.0f;
    fn_1_5410(this, &zeroPosition);
    speedTransition.update();
    if (initialized == 0 &&
        speedTransition.isFinished()) {
        initialized = 1;
    }
    fn_1_5094(this, speedTransition);
    lastModel.resetAttribute(1);
    lastModel.updateMotion();
    lastModel.setAttribute(1);
}

void fn_1_564C(Model *model, char **commands)
{
    char **command = commands;
    while (*command != 0) {
        model->hideObject(*command++);
    }
}

void fn_1_569C(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material) {
    GXSetNumChans(0U);
    GXSetNumTexGens(1U);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetNumTevStages(1U);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 0U, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 0U, GX_TEVPREV);
    GXSetZMode(1U, GX_GREATER, 0U);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
}
