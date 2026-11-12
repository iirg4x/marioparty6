// The game controller: the round's thread (start, the timed fight, the finish and the winners),
// the world it owns, and the stage models it shows and hides on the arena's events. The file
// name is descriptive: the game's assert texts name only dossun.cpp and player_table_1vs1.cpp in
// this module. This file links after dossun.cpp, and the game's files link in name order, so its
// name sorts after that one.
#include "dolphin/mtx.h"

#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"

#include "dolphin/types.h"

#include "math.h"

#include "dolphin/gx.h"

#include "REL/m653DLL/exhaust_smoke.h"

#include "REL/framework/phase_durations.h"

#include "REL/framework/event_hm.h"

#include "REL/framework/event.h"

#include "REL/m653DLL/allocation.h"

#include "REL/m653DLL/random.h"

#include "REL/framework/model.h"

#include "REL/m653DLL/native_resource.h"

#include "datadir_enum.h"

#include "game/hu3d.h"

#include "REL/m653DLL/car.h"

#include "REL/framework/p_chara.h"

#include "REL/m653DLL/boundary_plane.h"

#include "REL/framework/game_object_man.h"

#include "REL/m653DLL/motion_work.h"

#include "REL/m653DLL/model_image.h"

#include "REL/framework/event_manager.h"

#include "REL/framework/category_sphere.h"

#include "REL/m653DLL.h"

#include "REL/m653DLL/game_world.h"

#include "REL/m653DLL/object_calls.h"

#include "REL/framework/scene_shadow.h"

#include "REL/framework/sound.h"

#include "REL/framework/thread.h"

#include "REL/m653DLL/game_controller.h"

#include "REL/m653DLL/camera_controller.h"

#include "dolphin.h"

#include "game/main.h"

#include "game/object.h"

extern "C" {
#include "game/audio.h"
}

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/gamework.h"

#include "game/memory.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/sprite.h"

extern "C" {
#include "game/wipe.h"
}

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "REL/framework/singleton.h"

#include "REL/framework/assertion.h"

#include "REL/framework/minigame_core.h"

#include "msm_stream.h"

#include <functional>

GameWorld::GameWorld()
    : model0(DATANUM(0x700000, 0)), model1(DATANUM(0x700000, 1)), model2(DATANUM(0x700000, 2)),
      model3(DATANUM(0x700000, 3)), model4(DATANUM(0x700000, 4)), model5(DATANUM(0x700000, 5)),
      model7(DATANUM(0x700000, 7)), model8(DATANUM(0x700000, 8)), model9(DATANUM(0x700000, 9))
{
    points[0].center.x = -800.0f,
    points[0].center.y = 0.0f,
    points[0].center.z = -1600.0f;
    points[1].center.x = 800.0f,
    points[1].center.y = 0.0f,
    points[1].center.z = -1600.0f;
    points[2].center.x = -800.0f,
    points[2].center.y = 0.0f,
    points[2].center.z = -1000.0f;
    points[3].center.x = 800.0f,
    points[3].center.y = 0.0f,
    points[3].center.z = -1000.0f;
    model2.setAttribute(((1 << 30) | (1 << 7)));
    model3.setAttribute(((1 << 30) | (1 << 7)));
    model4.setAttribute(((1 << 30) | (1 << 7)));
    model5.setAttribute(((1 << 30) | (1 << 7)));
    model7.setAttribute(((1 << 30) | (1 << 7)));
    model2.setShadowMap();
    model3.setShadowMap();
    model4.setShadowMap();
    model5.setShadowMap();
    model7.setShadowMap();
    model0.setLightsDynamic(0);
}

M653GameController::M653GameController()
    : ThreadEntry(0x10000), world(), timer(0)
{
    RandomSeed(OSGetTick());
    world.reset(new GameWorld);
    enableDefaultAttributeA();
    count = 0;
    registerChannel(0);
    registerChannel(3);
    fn_1_9210();
    lbl_1_bss_65 = 0;
}

M653GameController::~M653GameController()
{
}

static const Vec fieldDirection = { 0.0f, -1.0f, 0.0f };

static const float spriteScale = 12.0f;

void M653GameController::setupObjects(OMOBJMAN *)
{
    new CameraController653;
    ResourceOwner *state = &MinigameSingleton<ResourceOwner>::getChecked();
    state->fn_1_73DC();
    state->fn_1_73E0();
    state->fn_1_73E8();
    state->fn_1_7118(0);
    state->fn_1_7118(1);
    new SceneShadow(1, &fieldDirection, spriteScale, 0x64);
}

void M653GameController::updateGame() {
    ThreadEntry::resume(0);
    process_event_queue();
    updateGameObjects();
    if (lbl_1_bss_64) {
        fn_1_4EFC(fn_1_81AC);
    }
    fn_1_965C();
}

void fn_1_81AC(s32 firstCategory, void *firstOwner, s32 secondCategory, void *secondOwner)
{
    switch (firstCategory) {
    case 0:
        if (secondCategory == 0) {
            fn_1_3BC4(static_cast<M653MotionWork *>(firstOwner),
                      static_cast<M653MotionWork *>(secondOwner));
            soundPlay(0x815);
            return;
        }
        break;
    default:
        break;
    }
}

// The winner's player index, -1 until the round has one.
extern "C" {
s32 lbl_1_data_238 = -1;
}

void M653GameController::run(void *initialArgument)
{
    s32 audioStream;
    s32 mode;
    s32 finishDelayFrame;
    s32 wipeFrame;
    s32 playerIndex;
    bool success;

    lbl_1_bss_64 = 0;
    sendEvent(2, 0, 0.0f);
    MinigameCore::wipeIn();
    audioStream = HuAudSStreamPlay(MSM_STREAM_TIME_TO_DUEL);
    fn_1_8640(this);
    sendEvent(2, 1, 0.0f);
    MinigameCore::showStart();
    while (MinigameCore::messageDone() == 0) {
        ThreadEntry::suspend(0);
    }

    timer.setParameters(secondsToFrames(0x1EL), secondsToFrames(0L), 0);
    timer.setMode( 1);
    lbl_1_bss_64 = 1;
    success = 0;
    sendEvent(2, 2, 0.0f);
    s32 zero = 0;
    for (;;) {
        if (timer.isDone() != 0) {
            break;
        }
        if (fn_1_10A4(lbl_1_bss_5C[0]) != 0) {
            break;
        }
        if (fn_1_10A4(lbl_1_bss_5C[1]) != 0) {
            break;
        }
        ThreadEntry::suspend(0);
    }

    lbl_1_bss_64 = 0;
    success = (fn_1_10A4(lbl_1_bss_5C[0]) != 0 &&
               fn_1_10A4(lbl_1_bss_5C[1]) != 0) ||
              timer.isDone() != 0;
    if (success == 0) {
        mode = fn_1_10A4(lbl_1_bss_5C[0]) ? 1 : 0;
        MinigameCore::addWinner(fn_1_696C(mode));
        lbl_1_data_238 = mode;
    }

    sendEvent(2, 3, 0.0f);
    HuAudSStreamFadeOut(audioStream, 0x64);
    HuAudFadeOut(0x3E8);
    lbl_1_bss_65 = 1;
    MinigameCore::showFinish();
    timer.start();
    finishDelayFrame = 0;
    while (finishDelayFrame < secondsToFrames(2L)) {
        ThreadEntry::suspend(0);
        finishDelayFrame += 1;
    }
    WipeColorSet(0xFF, 0xFF, 0xFF);
    if (success == 0) {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 0x3C);
    }
    wipeFrame = 0;
    while (wipeFrame < 0x3C) {
        ThreadEntry::suspend(0);
        wipeFrame += 1;
    }
    if (success == 0) {
        sendEvent(0, 1, 0.0f);
        for (playerIndex = 0; playerIndex < 2; ++playerIndex) {
            delete lbl_1_bss_5C[playerIndex];
        }
        ResourceOwner &resource = MinigameSingleton<ResourceOwner>::getChecked();
        resource.fn_1_7238(lbl_1_data_238);
    }
    if (success == 0) {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREV, 0x3C);
    }
    sendEvent(2, 4, 0.0f);
    HuAudJinglePlay(success != 0 ? 0x45 : 0x43);
    MinigameCore::showWinners();
    while (MinigameCore::messageDone() == 0) {
        ThreadEntry::suspend(0);
    }
    MinigameCore::requestExit();
    for (;;) {
        ThreadEntry::suspend((void *)-1);
    }
}

void fn_1_8640(M653GameController *owner)
{
    Mtx firstMatrix;
    Mtx secondMatrix;
    GameWorld *firstWorld;
    GameWorld *secondWorld;
    Model *firstTriangleModel;
    Model *secondTriangleModel;
    s16 firstModelId;
    s16 secondModelId;
    s32 updated;

    for (;;) {
        updated = 0;
        firstWorld = owner->world.value;
        firstTriangleModel = &firstWorld->model8;
        secondWorld = owner->world.value;
        secondTriangleModel = &secondWorld->model9;
        if (firstTriangleModel->isMotionEnd() == 0) {
            firstModelId = (s16)firstTriangleModel->modelId;
            fn_80023398(firstModelId, "1p_triangle", firstMatrix);
            fn_1_1424(lbl_1_bss_5C[0], firstMatrix);
            updated += 1;
        }
        if (secondTriangleModel->isMotionEnd() == 0) {
            secondModelId = (s16)secondTriangleModel->modelId;
            fn_80023398(secondModelId, "2p_triangle", secondMatrix);
            fn_1_1424(lbl_1_bss_5C[1], secondMatrix);
            updated += 1;
        }
        if (updated == 0) {
            break;
        }
        owner->ThreadEntry::suspend((void *)(0));
    }
}

OwnedModel *fn_1_872C(M653GameController *owner, s32 selector)
{
    switch (selector) {
    case 0:
        return &owner->world.get()->model7;
    case 1:
        return &owner->world.get()->model2;
    case 2:
        return &owner->world.get()->model3;
    case 3:
        return &owner->world.get()->model4;
    case 4:
        return &owner->world.get()->model5;
    default:
        return 0;
    }
}

void fn_1_87C4(M653GameController *owner, s32 selector)
{
    OwnedModel *selectedValue = fn_1_872C(owner, selector);
    if (selectedValue != 0) {
        selectedValue->setAttribute((1 << 0));
    }
}

void fn_1_8880(M653GameController *owner, s32 selector)
{
    OwnedModel *selectedValue = fn_1_872C(owner, selector);
    if (selectedValue != 0) {
        selectedValue->resetAttribute(((1 << 30) | (1 << 7)));
        sendEvent(3, selector, 3.0f);
        fn_1_9660(selector);
    }
}

void M653GameController::receive(const EventPacket *control)
{
    switch (control->channel) {
    case 0:
        switch (control->type) {
        case 0:
            fn_1_8880(this, count++);
            soundPlay(0x816);
            soundPlay(0x817);
            break;
        default:
            break;
        }
        break;
    case 3:
        fn_1_87C4(this, control->type);
    }
}

void M653GameController::finishGame() {}

/* The two players' objects and the round's state flags. */
extern "C" {
Car *lbl_1_bss_5C[2];
u8 lbl_1_bss_64;
u8 lbl_1_bss_65;
}
