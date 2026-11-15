#include "REL/framework/event_manager.h"
#include "REL/m634DLL/player_callback.h"
#include "REL/m634DLL/scene.h"
#include "REL/framework/game_object_man.h"
extern "C" {
#include "dolphin/types.h"
}

#include "dolphin/os.h"

#include "dolphin/gx.h"

#include "dolphin/mtx.h"

#include "REL/m634DLL/actor_tables.h"

#include "dolphin.h"

#include "humath.h"

#include "REL/framework/model.h"

#include "REL/framework/singleton.h"

extern "C" {
#include "game/hu3d.h"
}

extern "C" {
#include "game/object.h"
}

extern "C" {
#include "game/gamework.h"
}

extern "C" {
#include "game/audio.h"
}

extern "C" {
#include "game/sprite.h"
}

extern "C" {
#include "game/memory.h"
}

extern "C" {
#include "game/data.h"
}

extern "C" {
#include "game/main.h"
}

extern "C" {
#include "game/pad.h"
}

extern "C" {
#include "game/window.h"
}

extern "C" {
#include "game/wipe.h"
}

extern "C" {
#include "game/frand.h"
}

extern "C" {
#include "game/charman.h"
}

extern "C" {
#include "game/process.h"
}

extern "C" {
#include "game/hsfex.h"
}

extern "C" {
#include "game/gamemes.h"
}

extern "C" {
#include "game/flag.h"
}

extern "C" {
#include "game/armem.h"
}

extern "C" {
#include "game/mgdata.h"
}

extern "C" {
#include "game/printfunc.h"
}

extern "C" {
#include "game/saveload.h"
}

extern "C" {
#include "game/esprite.h"
}

extern "C" {
#include "game/thpmain.h"
}

extern "C" {
#include "game/mg/seqman.h"
}

extern "C" {
#include "game/mg/actman.h"
}

extern "C" {
#include "game/mg/timer.h"
}

extern "C" {
#include "game/mg/score.h"
}

extern "C" {
#include "game/board/object.h"
}

extern "C" {
#include "game/board/main.h"
}

extern "C" {
#include "game/board/audio.h"
}

extern "C" {
#include "game/board/tutorial.h"
}

extern "C" {
#include "game/board/masu.h"
}

extern "C" {
#include "game/board/player.h"
}

#include "REL/m634DLL/collision_shape.h"

#include "REL/framework/event_hm.h"

#include "datadir_enum.h"

#include "REL/m634DLL/model_owner.h"

#include "REL/m634DLL/camera_control.h"

#include "REL/m634DLL/ActModels.h"

#include "REL/m634DLL/act_player.h"

#include "REL/m634DLL/actor_position.h"

#include "REL/framework/sound.h"

#include "REL/framework/minigame_core.h"

#include "REL/m634DLL/camera.h"

#include "REL/framework/frame_time.h"

#include <functional>

#include <math.h>

#include <string.h>

#include <vector>

#include <memory>

static const int NUMBEROF_MOTION = 8;

extern "C" { void __msl_assertion_failed(const char *condition, const char *file, int line); }

extern "C" { extern void *lbl_1_bss_20; }

extern "C" { extern const s32 lbl_1_rodata_34[3]; extern const Vec lbl_1_rodata_60; extern const Vec lbl_1_rodata_84[4]; }

extern "C" { extern char *lbl_1_data_34[4]; }

void fn_1_3C8C(ActPlayer *, Mtx *);

extern "C" { extern f32 lbl_1_data_44; }

void fn_1_1728(ActPlayer *object, u32 motionId, u8 mode, u8 loop, f32 blend);

void fn_1_22C4(ActPlayer *camera);

void fn_1_22FC(ActPlayer *camera, f32 angle);

static inline s16 GetNightFlag()
{
    return lbl_802C0278;
}

struct Scene;

int fn_1_3BBC(COL_NARROW_PARAM *unused, COL_NARROW_PARAM *state);

extern "C" { int lbl_1_bss_0 = secondsToFrames(2L); }

extern "C" { int lbl_1_bss_4 = (int)secondsToFrames(3.5); }

extern "C" { int lbl_1_bss_8 = (int)secondsToFrames(3.5); }

extern "C" { float lbl_1_bss_C = (float)(0.016666666666666666 * (0.016666666666666666 * (double)lbl_1_data_44)); }

template <> Scene *MinigameSingleton<Scene>::self = 0;
template <> GameObjectManager *MinigameSingleton<GameObjectManager>::self = 0;

extern "C" const s32 lbl_1_rodata_10[2] = {0x5D000E, 0x5D000F};
extern "C" const s32 lbl_1_rodata_18[2] = {0x5D0036, 0x5D0037};
extern "C" const s32 lbl_1_rodata_20[2] = {0x5D0010, 0x5D0011};
extern "C" const s32 lbl_1_rodata_28[2] = {0x5D0038, 0x5D0039};

extern "C" { s32 lbl_1_data_0[4] = {0x005D0002, 0x005D0003, 0x005D0004, 0x005D0005}; }

struct TransitionColorScale {
    float red;
    float green;
    float blue;
};

struct CameraVector {
    f32 x;
    f32 y;
    f32 z;
};

struct MotionValues {
    f32 x;
    f32 speed;
    f32 y;
};

ActPlayer *fn_1_EC8C(Scene *, s32);

ActModels::ActModels()
{
    int index;

    for (index = 0; index < 4; index++) {
        models[index].reset(new OwnedModel((unsigned int)lbl_1_data_0[index]));
        fn_80030644(*models[index]);
    }
}

extern "C" { char *lbl_1_data_34[4] = {"p1", "p2", "p3", "p4"}; }
extern "C" { f32 lbl_1_data_44 = 980.665f; }

extern "C" {
unsigned int lbl_1_data_48[0x11] = {
    0x0093006C, 0x0093006D, 0x0093006D, 0x00930003, 0x00930004, 0x00930006, 0x008E00A8, 0x00930008, 0x0093000A, 0x00930009, 0x0093000F, 0x00930010, 0x00930011, 0x00930012, 0x00930016, 0x00930016, 0x00000000,
};
}

extern "C" {
Vec lbl_1_data_8C[4] = {
    {-400.0f, 300.0f, -300.0f},
    {-400.0f, 300.0f, 300.0f},
    {500.0f, 300.0f, 0.0f},
    {300.0f, 300.0f, 300.0f},
};
}

ActModels::~ActModels() {}

u8 ActModels::fn_1_2C0(s32 index, Vec *position, Vec *rotation) {
        s16 modelId = *models[index];
        HSF_OBJECT *object = fn_80026BF4(modelId, lbl_1_data_34[index]);
        if (object != 0) {
            *position = object->mesh.curr.pos;
            *rotation = object->mesh.curr.rot;
            rotation->y += 90.0f;
        }
        return fn_8002D75C(modelId) != 0;
    }

ActPlayerPart *fn_1_3A0(ActPlayer *work, s32 playerIndex, s32 alternate)
{
    if (lbl_80265728[playerIndex].type != 0) {
        if (alternate == 0) {
            return new PlayerChoicePart(work, lbl_80265728[playerIndex].comDif);
        } else {
            return new PlayerMotionPart(work, lbl_80265728[playerIndex].comDif);
        }
    } else {
        return new PlayerPadPart(work);
    }
}

void fn_1_4A8(HU3D_MODEL *model, Mtx *matrix) { fn_1_3C8C(static_cast<ActPlayer *>(model->hookData), matrix); }

extern "C" const s32 lbl_1_rodata_34[3] = {1, 2, 3};

inline ActPlayerPart::~ActPlayerPart() {}

static inline s16 getNightFlag() { return lbl_802C0278; }

static inline ActPlayerPart *makePart(ActPlayer *work, s32 playerIndex, s32 alternate)
{
    if (lbl_80265728[playerIndex].type != 0) {
        if (alternate == 0) {
            return new PlayerChoicePart(work, lbl_80265728[playerIndex].comDif);
        } else {
            return new PlayerMotionPart(work, lbl_80265728[playerIndex].comDif);
        }
    } else {
        return new PlayerPadPart(work);
    }
}

void fn_1_4A8(HU3D_MODEL *model, Mtx *matrix);

ActPlayer::ActPlayer(s32 index, s32 mode, s32 category)
    : GameObject(category), shape(50.0f), unknown70(0.0f), unknown74(0), playerIndex(index),
      unknownA4(0), unknownB0(1), unknownB4(0), playerMode(mode), transitionStep(0), unknownC4(0), unknownC8(0),
      firstHandlers(1), secondHandlers(0), motionId(-1)
{
    MgActorParam_s actorParam;
    s32 cameraMask;
    s32 night;

    fn_1_3ED0(&firstHandlers, std::bind1st(std::mem_fun(&ActPlayer::fn_1_353C), this), 0);
    fn_1_3ED0(&firstHandlers, std::bind1st(std::mem_fun(&ActPlayer::fn_1_3638), this), 2);
    fn_1_3ED0(&firstHandlers, std::bind1st(std::mem_fun(&ActPlayer::fn_1_36D4), this), 3);
    fn_1_3ED0(&firstHandlers, std::bind1st(std::mem_fun(&ActPlayer::fn_1_37FC), this), 4);
    fn_1_3ED0(&firstHandlers, std::bind1st(std::mem_fun(&ActPlayer::fn_1_3A28), this), 5);
    fn_1_3ED0(&secondHandlers, std::bind1st(std::mem_fun(&ActPlayer::fn_1_3D18), this), 1);
    const Vec &initialPosition = lbl_1_data_8C[playerMode];
    position.x = initialPosition.x;
    position.y = initialPosition.y;
    position.z = initialPosition.z;
    velocity.x = 0.0f;
    velocity.y = 0.0f;
    velocity.z = 0.0f;
    shape.position.x = 0.0f;
    shape.position.y = 0.0f;
    shape.position.z = 0.0f;
    if (playerMode == 0) setShapeKind(0);
    else setShapeKind(lbl_1_rodata_34[playerMode - 1]);
    actorParam.height = 150.0f;
    actorParam.radius = 45.0f;
    actorParam.param = mode;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.correctHookParam = 0;
    actorParam.narrowHook = fn_1_3BBC;
    actorParam.correctHook = 0;
    player = fn_80080728((s16)index, &actorParam, (s16)4, (u16)2, (u32)1, lbl_1_data_48);
    fn_8008697C(player, &position);
    fn_8007ED60(player);
    modelId = player->actor->mdlId;
    characterNumber = player->charNo;
    fn_80026E44(modelId);
    fn_8005DDEC((s32)1);
    fn_80026B8C(modelId, (u32)1);
    cameraMask = 2;
    if (playerMode == 0) cameraMask |= 1;
    fn_80026B64(modelId, (u16)cameraMask);
    loadMotions();
    fn_8005F77C(characterNumber, (s32)0);
    fn_80087158(player, (u16)1);
    fn_1_1814();
    s32 alternate = playerMode == 0 ? 0 : 1;
    part.reset(makePart(this, index, alternate));
    loadColors();
    static NightScale lbl_1_data_104[2] = { { 2.0f, 2.0f, 2.0f }, { 0.5f, 0.5f, 0.5f } };
    static Vec lbl_1_data_11C = { 0.85f, 0.85f, 0.85f };
    night = getNightFlag();
    fn_1_1F9C(lbl_1_data_104[night].x, lbl_1_data_104[night].y, lbl_1_data_104[night].z);
    fn_800268E0(modelId, &lbl_1_data_11C);
    fn_8005DFC0(characterNumber, (u32)1);
    if (playerMode == 0) {
        bodyModel.reset(new OwnedModel(fn_1_4A8));
        bodyModel->setCamera(3);
        bodyModel->setLayer(0);
        bodyModel->setHookData(this);
    }
    setState(&ActPlayer::fn_1_2A20);
}

ActPlayer::~ActPlayer()
{
    fn_80080F38(player->actor);
    if (playerMode == 0) {
        fn_80028CB8(modelId, secondaryLightId);
    }
}

void ActPlayer::loadMotion(u32 dataNum)
{
    m_motion_id.push_back(fn_8005B0B8(characterNumber, dataNum));
}

void ActPlayer::loadMotions()
{
    m_motion_id.clear();
    loadMotion(DATANUM(0x930000, 0));
    loadMotion(DATANUM(0x930000, 108));
    loadMotion(DATANUM(0x930000, 109));
    loadMotion(DATANUM(0x930000, 97));
    loadMotion(DATANUM(0x930000, 24));
    loadMotion(DATANUM(0x8e0000, 168));
    loadMotion(DATANUM(0x930000, 39));
    loadMotion(DATANUM(0x930000, 40));
    m_motion_id.size() == NUMBEROF_MOTION
        ? (void)0
        : __msl_assertion_failed("m_motion_id.size() == NUMBEROF_MOTION", "act_player.cpp", 0x167);
    fn_8005B4BC(characterNumber);
}

void fn_1_1728(ActPlayer *object, u32 motionId, u8 mode, u8 loop, f32 blend)
{
    s16 motion;

    if (object->motionId != motionId || mode != 0) {
        motion = object->m_motion_id.at(motionId);
        fn_8005C430(object->characterNumber, ((1 << 30) | (1 << 1)));
        if (mode == 1) {
            fn_8005BE80(object->characterNumber, motion);
        } else {
            fn_8005C2C8(object->characterNumber, motion,
                               0.0f, blend, loop == 0 ? ((1 << 30) | (1 << 0)) : 0);
        }
        fn_8005C390(object->characterNumber, 1.0f);
        object->motionId = motionId;
    }
}

/* The start of the unit's pool: MSL sqrtf's statics, which <vector> brings to 0x0, then these. */

void ActPlayer::fn_1_1814()
{
    s32 nightMode;
    s16 nightFlag = lbl_802C0278;
    s32 index;

    nightMode = nightFlag;
    shownModel = -1;
    for (index = 0; index < 2; index++) {
        if (nightMode == 0) {
            if (playerMode == 0) {
                fn_1_194C(index, lbl_1_rodata_10[index]);
            } else {
                fn_1_194C(index, lbl_1_rodata_20[index]);
            }
        } else if (playerMode == 0) {
            fn_1_194C(index, lbl_1_rodata_18[index]);
        } else {
            fn_1_194C(index, lbl_1_rodata_28[index]);
        }
    }
    elapsedTime = 0.0f;
    transitionReady = 1;
    fn_1_1A84();
}

void ActPlayer::fn_1_194C(s32 index, u32 value)
{
    models[index].reset(new OwnedModel(value));
    models[index]->setRotationY(0.0f);
    models[index]->setLayer(1);
}

void ActPlayer::fn_1_1A34()
{
    char *hookName;
    hookName = fn_8005C0F0(characterNumber, 4, 0);
    fn_8005C7A4(characterNumber, hookName);
}

void ActPlayer::fn_1_1A84()
{
    s16 motionModelId;
    char *hookName;
    if (transitionReady != 0) {
        motionModelId = *models[1];
        elapsedTime = 0.0f;
        fn_800269E8(*models[0], 1);
        fn_80026A44(motionModelId, 1);
        fn_8002CF98(motionModelId, 0.0f);
        fn_80026A44(motionModelId, ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 1)));
        fn_800269E8(motionModelId, ((1 << 30) | (1 << 6)) | ((1 << 30) | (1 << 0)));
        hookName = fn_8005C0F0(characterNumber, 4, 0);
        fn_8002741C(modelId, hookName, *models[1]);
        shownModel = 1;
        transitionReady = 0;
    }
}

void ActPlayer::fn_1_1B90()
{
    s16 motionModelId;
    char *hookName;
    motionModelId = *models[0];
    fn_8002CF98(motionModelId, 0.0f);
    transitionReady = 1;
    if (shownModel != 0) {
        fn_800269E8(*models[1], 1);
        fn_80026A44(motionModelId, 1);
        fn_80026A44(motionModelId, ((1 << 30) | (1 << 6)) | ((1 << 30) | (1 << 0)));
        fn_80026A44(motionModelId, ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 1)));
        hookName = fn_8005C0F0(characterNumber, 4, 0);
        fn_8002741C(modelId, hookName, *models[0]);
        shownModel = 0;
    }
}

void ActPlayer::fn_1_1C8C()
{
    if (shownModel == 1) {
        transitionReady = 1;
    }
}

void ActPlayer::loadColors()
{
    GXColor firstValue;
    GXColor secondValue;
    HU3D_MODEL *model;
    HSF_MATERIAL *material;
    s32 count;
    s32 firstIndex;
    s32 secondIndex;

    model = &lbl_802C01C8[modelId];
    material = model->hsf->material;
    count = model->hsf->materialNum;
    for (firstIndex = 0; firstIndex < count; firstIndex++, material++) {
        firstValue.r = material->color[0];
        firstValue.g = material->color[1];
        firstValue.b = material->color[2];
        firstColors.push_back(firstValue);
    }

    model = &lbl_802C01C8[*models[1]];
    material = model->hsf->material;
    count = model->hsf->materialNum;
    for (secondIndex = 0; secondIndex < count; secondIndex++, material++) {
        secondValue.r = material->color[0];
        secondValue.g = material->color[1];
        secondValue.b = material->color[2];
        secondColors.push_back(secondValue);
    }
}

void ActPlayer::fn_1_1DE8()
{
    HSF_MATERIAL *material;
    HU3D_MODEL *model;
    s32 count;
    s32 firstIndex;
    s32 secondIndex;

    model = &lbl_802C01C8[modelId];
    material = model->hsf->material;
    count = model->hsf->materialNum;
    for (firstIndex = 0; firstIndex < count; firstIndex++, material++) {
        GXColor color = firstColors.at(firstIndex);
        material->color[0] = color.r;
        material->color[1] = color.g;
        material->color[2] = color.b;
    }
    model->ambR = 1.0f;
    model->ambG = 1.0f;
    model->ambB = 1.0f;

    model = &lbl_802C01C8[*models[1]];
    material = model->hsf->material;
    count = model->hsf->materialNum;
    for (secondIndex = 0; secondIndex < count; secondIndex++, material++) {
        GXColor color = secondColors.at(secondIndex);
        material->color[0] = color.r;
        material->color[1] = color.g;
        material->color[2] = color.b;
    }
    model->ambR = 1.0f;
    model->ambG = 1.0f;
    model->ambB = 1.0f;
}

void ActPlayer::fn_1_1F9C(f32 redScale,
                           f32 greenScale, f32 blueScale)
{
    HU3D_MODEL *model;
    HSF_MATERIAL *material;
    s32 materialCount;
    s32 firstMaterialIndex;
    s32 secondMaterialIndex;

    model = &lbl_802C01C8[modelId];
    material = model->hsf->material;
    materialCount = model->hsf->materialNum;
    for (firstMaterialIndex = 0; firstMaterialIndex < materialCount;
         firstMaterialIndex++, material++) {
        material->color[0] *= redScale;
        material->color[1] *= greenScale;
        material->color[2] *= blueScale;
    }
    model->ambR = redScale;
    model->ambG = greenScale;
    model->ambB = blueScale;

    model = &lbl_802C01C8[*models[1]];
    material = model->hsf->material;
    materialCount = model->hsf->materialNum;
    for (secondMaterialIndex = 0; secondMaterialIndex < materialCount;
         secondMaterialIndex++, material++) {
        material->color[0] *= redScale;
        material->color[1] *= greenScale;
        material->color[2] *= blueScale;
    }
    model->ambR = redScale;
    model->ambG = greenScale;
    model->ambB = blueScale;
}

void ActPlayer::fn_1_21C4()
{
    fn_80026A44(modelId, 1);
}

void ActPlayer::fn_1_21F8()
{
    fn_800269E8(modelId, 1);
}

extern "C" const Vec lbl_1_rodata_60 = {0.0f, 0.0f, 60.000004f};

void ActPlayer::fn_1_222C()
{
    Mtx rotation;

    fn_80086BC8(player->actor, &position);
    {
        Vec rotatedOffset = lbl_1_rodata_60;
        PSMTXRotRad(rotation, 0x79, MTXDegToRad(angle));
        PSMTXMultVec(rotation, &rotatedOffset, &rotatedOffset);
        PSVECAdd(&position, &rotatedOffset, &shape.position);
    }
}

void fn_1_22C4(ActPlayer *camera) {
    fn_80086CB0(camera->player->actor, &camera->angle);
}

void fn_1_22FC(ActPlayer *camera, f32 angle)
{
    camera->angle = angle;
    fn_80086CA8(camera->player->actor, camera->angle);
}

void ActPlayer::fn_1_2340()
{
    if (shownModel == 1) {
        elapsedTime += 1.0f;
        if (elapsedTime > 43.0f) {
            elapsedTime = 0.0f;
            if (transitionReady == 1) {
                fn_800269E8(*models[1], ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 1)));
                shownModel = -1;
            } else {
                fn_8002CF98(*models[1], 0.0f);
                fn_80026A44(*models[1], ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 1)));
            }
        }
    }
}

void ActPlayer::update()
{
    fn_1_2340();
    fn_1_222C();
    fn_80086CB0(player->actor, &angle);
    if (state) {
        (this->*state)();
    }
    position.y = 0.0f;
    fn_80086B04(player->actor, &position);
    unknown74++;
}

void ActPlayer::fn_1_2614()
{
    MotionValues motionValues;
    ActPlayerPart * motionNode;
    f32 motionSpeed;
    motionNode = part.get();
    motionNode->updateInput();
    u32 nodeState2 = motionNode->input.pressedButtons;
    s32 nodeValue = motionNode->input.heldButtons;
    motionValues.x = motionNode->input.x;
    motionValues.y = - motionNode->input.y;
    motionValues.speed = motionNode->input.magnitude;
    shape.enabled = 0;
    if (player->motNo != 0 && motionId != DATANUM(0x8e0000, 168))
    {
        motionSpeed = 1.5f * motionValues.speed;
        fn_8005C390(characterNumber, motionSpeed);
        if (motionSpeed != 0.0f)
        {
            fn_1_1A84();
        }
        else
        {
            if (shownModel == 1)
            {
                transitionReady = 1;
            }
        }
    }
    if ((nodeState2 & 0x100) == 0 || unknownB0 == 0)
    {
        return;
    }
    soundPlayAt(2, 0x73F, &position);
    shape.enabled = 1;
    unknownB0 = 0;
    unknownB4 = 0;
    shape.collisionKind = - 1;
    fn_8007EEBC(player, 0);
    fn_80087158(player, 1);
    fn_80087158(player, 0x400);
    shiftMotion(5, 0, 1, 8.0f);
    fn_1_1B90();
    player->mode = 0xA;
    {
        ActState handler = &ActPlayer::fn_1_32BC;
        this->state = handler;
    }
}

void ActPlayer::fn_1_2A20()
{
}

extern "C" { TransitionColorScale lbl_1_data_178[2] = { {0.9f, 0.9f, 0.9f}, {1.0f, 1.0f, 1.0f} }; }

extern "C" { s32 lbl_1_data_190[4] = {0x70, 0x7c, 0x8c, 0xa4}; }

static inline void restoreTransitionColors(ActPlayer *effect)
{
    HSF_MATERIAL *material;
    HU3D_MODEL *model;
    s32 count;
    s32 firstIndex;
    s32 secondIndex;
    model = &lbl_802C01C8[effect->modelId];
    material = model->hsf->material;
    count = model->hsf->materialNum;
    for (firstIndex = 0; firstIndex < count; firstIndex++, material++) {
        GXColor color = effect->firstColors.at(firstIndex);
        material->color[0] = color.r;
        material->color[1] = color.g;
        material->color[2] = color.b;
    }
    model->ambR = 1.0f;
    model->ambG = 1.0f;
    model->ambB = 1.0f;

    model = &lbl_802C01C8[*effect->models[1]];
    material = model->hsf->material;
    count = model->hsf->materialNum;
    for (secondIndex = 0; secondIndex < count; secondIndex++, material++) {
        GXColor color = effect->secondColors.at(secondIndex);
        material->color[0] = color.r;
        material->color[1] = color.g;
        material->color[2] = color.b;
    }
    model->ambR = 1.0f;
    model->ambG = 1.0f;
    model->ambB = 1.0f;
}

static inline void scaleTransitionColors(ActPlayer *effect,
                             f32 redScale, f32 greenScale,
                             f32 blueScale)
{
    HU3D_MODEL *model;
    HSF_MATERIAL *material;
    s32 materialCount;
    s32 firstMaterialIndex;
    s32 secondMaterialIndex;
    model = &lbl_802C01C8[effect->modelId];
    material = model->hsf->material;
    materialCount = model->hsf->materialNum;
    for (firstMaterialIndex = 0; firstMaterialIndex < materialCount;
         firstMaterialIndex++, material++) {
        material->color[0] *= redScale;
        material->color[1] *= greenScale;
        material->color[2] *= blueScale;
    }
    model->ambR = redScale;
    model->ambG = greenScale;
    model->ambB = blueScale;

    model = &lbl_802C01C8[*effect->models[1]];
    material = model->hsf->material;
    materialCount = model->hsf->materialNum;
    for (secondMaterialIndex = 0; secondMaterialIndex < materialCount;
         secondMaterialIndex++, material++) {
        material->color[0] *= redScale;
        material->color[1] *= greenScale;
        material->color[2] *= blueScale;
    }
    model->ambR = redScale;
    model->ambG = greenScale;
    model->ambB = blueScale;
}

void ActPlayer::fn_1_2A24()
{
    if (this->transitionStep == lbl_1_data_190[this->playerMode])
    {
        s32 nightMode2 = GetNightFlag();
        restoreTransitionColors(this);
        scaleTransitionColors(this, lbl_1_data_178[nightMode2].red, lbl_1_data_178[nightMode2].green, lbl_1_data_178[nightMode2].blue);
        fn_8005F77C(this->characterNumber, 1);
    }
    ++this->transitionStep;
    switch (this->transitionStage)
    {
    case 0 :
        {
            ActModels &models = ActModels::getChecked();
            Vector3 position; Vec rotation;
            if (!models.fn_1_2C0(this->playerMode, &position, &rotation))
            {
                fn_80086B04(this->player->actor, &position);
                this->position = position;
                fn_80086CA8(this->player->actor, rotation.y);
                this->rotation = rotation;
                fn_80026790(this->modelId, &this->position);
                fn_80026838(this->modelId, &this->rotation);
                return;
            }
            this->shiftMotion(2, 0, 0, 8.0f);
            ++this->transitionStage;
        }
    case 1 :
        this->rotation.y += 0.1f * - this->rotation.y;
        fn_80026838(this->modelId, & this->rotation);
        fn_80086CA8(this->player->actor, this->rotation.y);
        {
            if (!((this->rotation.y < 0.0f ? -this->rotation.y : this->rotation.y) > 2.0f))
            {
                this->rotation.y = 0.0f;
                fn_80026838(this->modelId, & this->rotation);
                fn_80086CA8(this->player->actor, this->rotation.y);
                fn_8007EEBC(this->player, 0);
                this->shiftMotion(1, 0, 0, 8.0f);
                if (this->shownModel == 1)
                {
                    this->transitionReady = 1;
                }
                ActState next = & ActPlayer::fn_1_2A20;
                this->state = next;
                ++this->transitionStage;
            }
        }
        break;
    }
}

void ActPlayer::fn_1_32B0()
{
    unknownA4 = 1;
}

void ActPlayer::fn_1_32BC()
{
    ActState initialValues;
    ActPlayerPart *node = part.get();
    node->updateInput();
    if (shape.getCollisionKind() != -1) {
        shape.enabled = 0;
    }
    if (unknownB4++ >= 0xF) {
        unknownB0 = 1;
        fn_80087168(player, 1);
        fn_80087168(player, 0x400);
        shiftMotion(1, 0, 0, 8.0f);
        initialValues = &ActPlayer::fn_1_2614;
        state = initialValues;
    }
}

void ActPlayer::fn_1_33F8(u8 activate)
{
    fn_80086E3C(player);
    if (activate != 0) {
        shiftMotion(6, 0, 1, 8.0f);
        MinigameCore::addWinner(playerIndex);
    } else {
        shiftMotion(7, 0, 1, 8.0f);
    }
}

void ActPlayer::fn_1_353C(const EventPacket &event)
{
    ActState dataState;

    transitionStage = 0;
    fn_80086E3C(player);
    shiftMotion(3, 0, 0, 8.0f);
    dataState = &ActPlayer::fn_1_2A24;
    state = dataState;
}

void ActPlayer::fn_1_3638(const EventPacket &event)
{
    ActState next;
    fn_80086F00(player, &position);
    fn_80087168(player, 1);
    part->reset();
    next = &ActPlayer::fn_1_2614;
    state = next;
}

void ActPlayerPart::reset() {}

void ActPlayer::fn_1_36D4(const EventPacket &event)
{
    ActState next;
    fn_80086E3C(player);
    fn_80087158(player, 1);
    fn_1_1A34();
    shiftMotion(0, 0, 0, 8.0f);
    next = &ActPlayer::fn_1_32B0;
    state = next;
}

extern "C" const Vec lbl_1_rodata_84[4] = {
    {-380.0f, 0.0f, -320.0f},
    {75.0f, 0.0f, -5.0f},
    {169.0f, 0.0f, 80.0f},
    {275.0f, 0.0f, 195.0f},
};
void ActPlayer::fn_1_37FC(const EventPacket &event)
{
    s32 selection = *static_cast<s32 *>(event.context);
    u8 activate;

    fn_80086E3C(player);
    shiftMotion(0, 0, 0, 8.0f);

    const Vec &targetPosition = lbl_1_rodata_84[this->playerMode];
    position.x = targetPosition.x;
    position.y = targetPosition.y;
    position.z = targetPosition.z;
    fn_80026790(modelId, &position);
    if (this->playerMode == 0) {
        CameraSceneWork &camera = CameraSceneWork::getChecked();
        f64 angle = camera.fn_1_111E0();
        rotation.x = 0.0f;
        rotation.y = (f32)angle;
        rotation.z = 0.0f;
    } else {
        rotation.x = 0.0f;
        rotation.y = 10.0f;
        rotation.z = 0.0f;
    }
    fn_80026838(modelId, &rotation);

    activate = 0;
    if (selection != -1) {
        if (selection == 0) {
            if (this->playerMode == 0) {
                activate = 1;
            } else {
                activate = 0;
            }
        } else {
            if (this->playerMode != 0) {
                activate = 1;
            } else {
                activate = 0;
            }
        }
    }
    if (activate != 1) {
        fn_800269E8(modelId, 1);
    }
}

void ActPlayer::fn_1_3A28(const EventPacket &event)
{
    s32 selection = *static_cast<s32 *>(event.context);
    u8 activate = 0;

    if (selection != -1) {
        if (selection == 0) {
            if (this->playerMode == 0) {
                activate = 1;
            } else {
                activate = 0;
            }
        } else if (this->playerMode != 0) {
            activate = 1;
        } else {
            activate = 0;
        }
    }
    fn_1_33F8(activate);
}

int fn_1_3BBC(COL_NARROW_PARAM *unused, COL_NARROW_PARAM *state)
{
    s32 mode;
    Scene *object;
    ActPlayer *result;
    Scene *singleton;
    Scene *instance;

    if (state->type) {
        return 0;
    }
    mode = state->paramB;
    if (mode == 0) {
        return 0;
    }
    object = &MinigameSingleton<Scene>::getChecked();
    result = fn_1_EC8C(object, mode);
    result->fn_1_3C74();
    return 1;
}

void ActPlayer::fn_1_3C74()
{
    unknownC4 = 1;
    unknownC8 += 1;
}

extern "C" { Point3d lbl_1_data_1E8 = {0.0f, 0.0f, -1000.0f}; }

extern "C" { Point3d lbl_1_data_1F4 = {0.0f, 0.0f, 1.0f}; }

extern "C" { GXColor lbl_1_data_200 = {0xff, 0xff, 0xff, 0xff}; }

void fn_1_3C8C(ActPlayer *object, Mtx *matrix) {
    if (lbl_802C01A6 == 0) {
        object->secondaryLightId = fn_800288B4(object->modelId, &lbl_1_data_1E8, &lbl_1_data_1F4, &lbl_1_data_200);
        fn_800296E0(object->modelId, object->secondaryLightId, 1);
        fn_80028B58(object->modelId, object->secondaryLightId);
        return;
    }
    fn_80028CB8(object->modelId, object->secondaryLightId);
}

void ActPlayer::fn_1_3D18(const EventPacket &event)
{
    s32 selectedPlayerId;
    s32 activePlayerId;
    selectedPlayerId = *static_cast<s32 *>(event.context);
    activePlayerId = shape.shapeKind;
    if (selectedPlayerId == activePlayerId) {
        fn_8006A030((s16)playerIndex, 0x14, 7, 3);
    }
}
