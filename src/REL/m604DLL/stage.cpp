#include "REL/m604DLL/camera_masks.h"
// The stage: its models, the lights and shadows, and the models that follow the camera.
#include <cmath>
#include <memory>
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/model.h"

extern "C" {
#include "dolphin/gx.h"
}
#include "humath.h"

#include "REL/m604DLL/camera_owner.h"

#include "REL/m604DLL/stage.h"

#include "REL/m604DLL/stage_effects.h"

/* A position returned by the camera owner. */

struct StageColors {
    GXColor entries[2];
};

extern "C" {

extern s16 lbl_802C01A6;
}

extern "C" {
#include "game/gamework.h"
}

const u32 lbl_1_rodata_420[2] = { 0x3F001B, 0x3F0004 };

const u32 lbl_1_rodata_428[2] = { 0x3F001C, 0x3F0005 };

const u32 lbl_1_rodata_430[2] = { 0x3F002F, 0x3F0018 };

const u32 lbl_1_rodata_438[3] = { 0x3F002A, 0x3F002B, 0x3F002C };

const u32 lbl_1_rodata_444[3] = { 0x3F0013, 0x3F0014, 0x3F0015 };

const u32 lbl_1_rodata_450[13] = { 0x3F001D, 0x3F001E, 0x3F001F, 0x3F0020, 0x3F0021, 0x3F0022, 0x3F0023, 0x3F0024, 0x3F0025, 0x3F0026, 0x3F0027, 0x3F0028, 0x3F0029 };

const u32 lbl_1_rodata_484[13] = { 0x3F0006, 0x3F0007, 0x3F0008, 0x3F0009, 0x3F000A, 0x3F000B, 0x3F000C, 0x3F000D, 0x3F000E, 0x3F000F, 0x3F0010, 0x3F0011, 0x3F0012 };

const Vec lbl_1_rodata_4B8[3] = { { -600.0f, 0.0f, 0.0f }, { 600.0f, 0.0f, 0.0f }, { -600.0f, 0.0f, 0.0f } };

void Stage::drawCallback(HU3D_MODEL *model, Mtx *matrix)
{
    Stage *stage = (Stage *)model->hookData;
    stage->draw(matrix);
}

Stage::Stage()
{
    s16 night = GWMgNightFGet();
    Vec modelPosition = { 0.0f, 0.0f, 0.0f };
    Hu3DBGColorSet(0, 0, 0);
    Vec lightPosition = { 0.0f, 4500.0f, 4500.0f };
    Vec targetPosition = { 0.0f, 0.0f, 0.0f };
    StageColors colors = { { { 255, 255, 255, 255 }, { 128, 128, 255, 255 } } };
    Vec lightDirection;
    PSVECSubtract(&targetPosition, &lightPosition, &lightDirection);
    PSVECNormalize(&lightDirection, &lightDirection);
    fn_800285BC();
    lightId = fn_800286D0(&lightPosition, &lightDirection, &colors.entries[night]);
    Hu3DGLightInfinitytSet((s16)lightId);
    Hu3DGLightStaticSet((s16)lightId, 1);
    fn_8002A3B0(30.0f, 1000.0f, 8000.0f, 0x0F);

    Vec shadowPosition;
    PSVECScale(&lightDirection, &shadowPosition, -1500.0f);
    PSVECAdd(&shadowPosition, &targetPosition, &shadowPosition);
    Vec shadowUp;
    shadowUp.x = 0.0f;
    shadowUp.y = 1.0f;
    shadowUp.z = 0.0f;
    fn_8002A674(&shadowPosition, &shadowUp, &targetPosition, (s16)0x0F);
    fn_8002AA3C(0, 0, 0x20, (s16)0x0F);
    fn_8002A8CC(0x10, 0x0F);
    fn_8002A7D8(0.2f, 0x0F);

    floorModel.reset(new OwnedModel(lbl_1_rodata_420[night]));
    floorModel->setLayer(1);
    floorModel->setPosition(&modelPosition);
    backModel.reset(new OwnedModel(lbl_1_rodata_428[night]));
    backModel->setLayer(1);
    backModel->setPosition(&modelPosition);
    backModel->setShadowMap();

    for (int index = 0; index < 3; index++) {
        if (night == 0) {
            movingModels[index].reset(new OwnedModel(lbl_1_rodata_438[night]));
        } else {
            movingModels[index].reset(new OwnedModel(lbl_1_rodata_444[night]));
        }
        OwnedModel *model = movingModels[index].get();
        model->setLayer(1);
        model->setPosition(&movingPositions[index]);
        angles[index] = 0.0f;
        movingPositions[index] = lbl_1_rodata_4B8[index];
    }

    for (int index = 0; index < 13; index++) {
        int reverseIndex = 12 - index;
        if (night == 0) {
            heightModels[index].reset(new OwnedModel(lbl_1_rodata_450[index]));
        } else {
            heightModels[index].reset(new OwnedModel(lbl_1_rodata_484[index]));
        }
        OwnedModel *model = heightModels[index].get();
        model->setLayer(1);
        model->setPosition(&modelPosition);
        model->setAttribute((1 << 30) | (1 << 1));
        model->resetShadow();
        heightStates[index] = 0;
    }

    s16 hook = fn_800260F4((HU3D_MODEL_HOOK)drawCallback);
    HU3D_MODEL *hookModel = &lbl_802C01C8[hook];
    hookModel->hookData = this;
    fn_80026B8C((s16)hook, 0);
    fn_80026B64((s16)hook, 0x0F);
    hookId = (s16)hook;

    if (night == 0) {
        new LensFlare;
    } else {
        new WeatherParticles;
    }
    shadowModel.reset(new OwnedModel(lbl_1_rodata_430[night]));
    shadowModel->setAttribute(1);
    shadowModel->resetShadow();

    for (int index = 0; index < 4; index++) {
        shadowCopies[index].reset(new OwnedModel(*shadowModel));
        OwnedModel *model = shadowCopies[index].get();
        model->setLayer(1);
        model->setAttribute((1 << 30) | (1 << 0));
        model->resetAttribute(1);
        model->setCamera(0x0F);
        model->resetShadow();
        Vec position;
        if (night == 0) {
            position.x = 0.0f;
            position.y = (f64)(2000.0f + 4000.0f * index);
            position.z = -1000.0f;
        } else {
            position.x = 0.0f;
            position.y = (f64)(20000.0f - 4000.0f * index);
            position.z = -1000.0f;
        }
        model->setPosition(&position);
        targetPositions[index] = position;
    }
}

Stage::~Stage()
{
    fn_800262D0(hookId);
    Hu3DGLightKill((s16)lightId);
}

void Stage::update()
{
    Vec *position = movingPositions;
    int index = 0;
    f32 distance;

    while (index < 3) {
        if (index == 0 || index == 2) {
            position->x += 0.3333333333333333;
            if (position->x > 700.0f) {
                position->x = -600.0f;
            }
        } else {
            position->x += -0.3333333333333333;
            if (position->x < -700.0f) {
                position->x = 600.0f;
            }
        }
        if (position->x < 0.0f) {
            distance = -position->x;
        } else {
            distance = position->x;
        }
        if (distance > 550.0f) {
            movingModels[index]->setAttribute(1);
        } else {
            movingModels[index]->resetAttribute(1);
        }
        angles[index] += 0.1f;
        index += 1;
        position++;
    }
}

void Stage::flagEnabled(int bit)
{
    flags &= ~(1 << bit);
}

const f32 lbl_1_rodata_568[13] = { 0.1f, 0.25f, 0.3f, 0.38f, 0.48f, 0.56f, 0.61f, 0.66f, 0.71f, 0.75f, 0.76f, 0.77f, 0.79f };

const f32 lbl_1_rodata_59C[3] = { 0.0f, 1500.0f, 2500.0f };

void Stage::draw(Mtx *matrix)
{
    const CameraOwner *recordOwner = &MinigameSingleton<CameraOwner>::getChecked();
    Vec cameraPosition = recordOwner->fn_1_878C(lbl_802C01A6);
    Point3d modelPosition;
    Point3d difference;
    s32 modelIndex;
    s32 movingIndex;
    s32 shadowIndex;
    OwnedModel *model;
    cameraPosition.z = 0.0f;
    cameraPosition.y -= 300.0f;
    for (modelIndex = 0; modelIndex < 13; modelIndex++)
    {
        modelPosition.x = cameraPosition.x;
        modelPosition.y = cameraPosition.y;
        modelPosition.z = cameraPosition.z;
        modelPosition.y *= lbl_1_rodata_568[modelIndex];
        if (modelPosition.y < 0.0f)
        {
            modelPosition.y = 0.0f;
        }
        heightModels[modelIndex]->setPosition(&modelPosition);
        if (heightStates[modelIndex] != 1 && cameraPosition.y > 100.0f * (15.0f * (f32) modelIndex))
        {
            if (GWMgNightFGet() == 0)
            {
                heightModels[modelIndex]->resetAttribute(0x40000002);
            }
            heightStates[modelIndex] = 1;
        }
    }
    for (movingIndex = 0; movingIndex < 3; movingIndex++)
    {
        modelPosition.x = cameraPosition.x;
        modelPosition.y = cameraPosition.y;
        modelPosition.z = cameraPosition.z;
        modelPosition.y *= 0.75f;
        modelPosition.y = (f32)((f64) modelPosition.y + (2000.0 + (100.0 * (0.3f * std::sin((M_PI * (f64) angles[movingIndex]) / 180.0)))));
        modelPosition.y += lbl_1_rodata_59C[movingIndex];
        modelPosition.x = movingPositions[movingIndex].x;
        modelPosition.z = -1000.0f - (100.0f * (10.0f * (f32) movingIndex));
        movingModels[movingIndex]->setPosition(&modelPosition);
    }
    for (shadowIndex = 0; shadowIndex < 4; shadowIndex++)
    {
        model = shadowCopies[shadowIndex].get();
        PSVECSubtract(&cameraPosition, & targetPositions[shadowIndex], & difference);
        if (difference.y > 700.0f || difference.y < -1400.0f)
        {
            model->setAttribute(1);
        }
        else
        {
            model->resetAttribute(1);
        }
    }
}
