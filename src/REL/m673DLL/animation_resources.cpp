// Loads the scene models and animations and creates its objects.

#include "humath.h"
#include "dolphin/types.h"
#include "game/animdata.h"
extern "C" {
#include "datadir_enum.h"
#include "game/data.h"
#include "game/sprite.h"
}
#include "REL/framework/model.h"
#include "REL/framework/event_hm.h"
#include "REL/m673DLL/scene_resources.h"
#include <algorithm>
#include <functional>
#include "REL/m673DLL/player_actor.h"
#include "REL/m673DLL/screen_overlay.h"
#include "REL/m673DLL/callback_receiver.h"
#include "REL/m673DLL/scene_billboard.h"
#include "REL/m673DLL/scene_model.h"
#include "REL/m673DLL/fade_overlay.h"
#include <vector>
#include <new>
#include "dolphin/gx/GXStruct.h"
#include "REL/m673DLL/moving_actor.h"

// Reads and locks an animation resource.
ANIMDATA *fn_1_5894(unsigned long dataNumber)
{
    ANIMDATA *animation = HuSprAnimDataRead(dataNumber);
    HuSprAnimLock(animation);
    return animation;
}

// Loads the scene models and animations and registers their event receivers.
SceneResources::SceneResources()
{
    animations.push_back(fn_1_5894(DATANUM(0x840000, 0x0B)));
    animations.push_back(fn_1_5894(DATANUM(0x840000, 0x0C)));
    animations.push_back(fn_1_5894(DATANUM(0x840000, 0x05)));
    animations.push_back(fn_1_5894(DATANUM(0x840000, 0x06)));
    animations.push_back(fn_1_5894(DATANUM(0x840000, 0x07)));
    models.push_back(new OwnedModel(DATANUM(0x840000, 0x08)));
    models.push_back(new OwnedModel(DATANUM(0x840000, 0x09)));
    int count = models.size();
    for (int index = 0; index < count; ++index) {
        models[index]->setAttribute(1);
    }
    registerChannel(0);
    registerChannel(2);
}

// Kills the scene animations and deletes the owned models.
SceneResources::~SceneResources()
{
    std::for_each(animations.begin(), animations.end(), std::ptr_fun(HuSprAnimKill));
    fn_1_6898(&models);
}

// Origin used to position the shadow light.
const Vec lbl_1_rodata_200 = { 0.0f, 4500.0f, 2000.0f };

// Target of the shadow light.
const Vec lbl_1_rodata_20C = { 0.0f, 0.0f, 0.0f };

// Initial colour copied during shadow setup.
const GXColor lbl_1_rodata_218 = { 0xFF, 0xFF, 0xFF, 0xFF };

// Positions of the scene billboards.
const Vec lbl_1_rodata_21C[8] = {
    { 0.0f, 372.0f, -520.0f },
    { 637.0f, 372.0f, 0.0f },
    { 0.0f, 372.0f, 580.0f },
    { -637.0f, 372.0f, 0.0f },
    { 641.0f, 223.0f, 519.0f },
    { 641.0f, 223.0f, -519.0f },
    { -641.0f, 223.0f, 519.0f },
    { -641.0f, 223.0f, -519.0f }
};

// Creates the scene models, lighting, and background objects.
void fn_1_6104(SceneResources *resources)
{
    OwnedModel *layerOneModel;
    OwnedModel *hiddenModel;
    OwnedModel *shadowModel;
    OwnedModel *layerFourModel;
    s32 modelIndex;
    s32 billboardIndex;
    s16 modelId;

    layerOneModel = new OwnedModel(DATANUM(0x840000, 0x01));
    hiddenModel = new OwnedModel(DATANUM(0x840000, 0x00));
    shadowModel = new OwnedModel(DATANUM(0x840000, 0x02));
    layerFourModel = new OwnedModel(DATANUM(0x840000, 0x03));

    layerOneModel->setLightsDynamic(false);
    layerOneModel->setLayer(1);
    hiddenModel->setAttribute(1);
    shadowModel->setLayer(1);
    shadowModel->setShadowMap();
    layerFourModel->setLayer(4);
    resources->models.push_back(layerOneModel);
    resources->models.push_back(hiddenModel);
    resources->models.push_back(shadowModel);
    resources->models.push_back(layerFourModel);
    Hu3DBGColorSet(0, 0, 0);

    Vec origin = lbl_1_rodata_200;
    Vec lightTarget = lbl_1_rodata_20C;
    Vec direction;
    Vec shadowPosition;
    Vec lightUp;
    GXColor shadowColor = lbl_1_rodata_218;

    HuSubVecF(&direction, &lightTarget, &origin);
    PSVECNormalize(&direction, &direction);
    fn_8002A3B0(20.0f, 1000.0f, 8000.0f, 1);
    PSVECScale(&direction, &shadowPosition, -3200.0f);
    PSVECAdd(&shadowPosition, &lightTarget, &shadowPosition);
    lightUp.x = 0.0f;
    lightUp.y = 1.0f;
    lightUp.z = 0.0f;
    fn_8002A674(&shadowPosition, &lightUp, &lightTarget, 1);
    fn_8002AA3C(0, 0, 0x40, 1);
    fn_8002A7D8(0.4f, 1);

    modelId = *hiddenModel;
    fn_800804E8(&modelId, 1, 40);
    new FadeOverlay();

    for (modelIndex = 0; modelIndex < 2; ++modelIndex) {
        Vec position;

        position.x = 0.0f;
        position.y = 0.0f;
        position.z = 0.0f;
        new SceneModel(resources->models[modelIndex], &position, modelIndex == 0 ? (u8)0 : (u8)1);
    }
    for (billboardIndex = 0; billboardIndex < 8; ++billboardIndex) {
        new SceneBillboard(&lbl_1_rodata_21C[billboardIndex], billboardIndex < 4 ? (u8)1 : (u8)0);
    }
    new CallbackReceiver();
    GXColor color = { 0x14, 0x0A, 0x14, 0xFF };
    new ScreenOverlay(color);
}

// Starting positions for the four players.
const Position3 lbl_1_rodata_29C[4] = {
    { { -250.0f, 0.0f, -250.0f } },
    { { 250.0f, 0.0f, -250.0f } },
    { { -250.0f, 0.0f, 250.0f } },
    { { 250.0f, 0.0f, 250.0f } }
};

// Creates the four player actors at their starting positions.
void fn_1_6638(SceneResources *)
{
    int entry;

    for (entry = 0; entry < 4; ++entry) {
        new PlayerActor(entry, &lbl_1_rodata_29C[entry]);
    }
}

// Creates the moving actor at its starting position.
void fn_1_66AC(SceneResources *)
{
    Position3 startPosition;
    f32 angle;

    angle = 0.0f;
    startPosition.value.x = 0.0f;
    startPosition.value.y = 0.0f;
    startPosition.value.z = 0.0f;
    startPosition.value.x = 0.0f;
    startPosition.value.y = 300.0f;
    startPosition.value.z = -250.0f;
    new MovingActor(DATANUM(0x840000, 0x0D), &startPosition, angle);
}

// Leaves the scene resources unchanged in this setup callback.
void fn_1_677C(SceneResources *)
{
}

// Leaves the scene resources unchanged in the final setup callback.
void fn_1_6780(SceneResources *)
{
}

// Dispatches events to the scene's registered channels.
void SceneResources::receive(const EventPacket *event)
{
    switch (event->channel) {
    case 0:
        fn_1_67FC(this, event);
        return;
    case 2:
        fn_1_6800(this, event);
    }
}

// Ignores the event for this resource callback.
void fn_1_67FC(SceneResources *resources, const EventPacket *event)
{
}
