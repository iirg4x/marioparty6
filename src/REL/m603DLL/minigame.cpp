#include "REL/m603DLL/ufo.h"
#include "REL/m603DLL/draw_node.h"
#include "REL/framework/frame_time_inline.h"
#include "REL/framework/list_object.h"
#include "REL/m603DLL/flow.h"
#include "REL/m603DLL/camera_group.h"
#include "REL/m603DLL/shadow_owner.h"
#include "REL/m603DLL/draw_owner.h"
#include "REL/m603DLL/zako.h"
#include "REL/m603DLL/camera_owner.h"
#include "REL/m603DLL/score_owner.h"
#include "REL/m603DLL/sprite_fade.h"
#include "REL/m603DLL/player.h"
#include "dolphin/types.h"
#include "dolphin/os.h"
#include "REL/framework/thread.h"

#include "REL/framework/random.h"
#include "msm_stream.h"

/* Create the game thread and its timer. */
MinigameThreadOwner::MinigameThreadOwner() : ThreadEntry(0x8000),timer(0) {}
/* Destroy the timer and game-thread bases. */
MinigameThreadOwner::~MinigameThreadOwner() {}

extern "C" {
#include "game/audio.h"
#include "game/wipe.h"
#include "game/gamework.h"
}

/* Create the stage, cameras, targets, score display and four players. */
void MinigameThreadOwner::setupObjects(OMOBJMAN *)
{
    s32 playerIndex;

    RandomSeed(OSGetTick());
    dayMode = !GWMgNightFGet();

    new ShadowModelOwner(dayMode);
    new CameraOwner();
    new DrawModelPool();
    new ScoreDisplay();
    if (dayMode == 0) {
        new CameraModelPool();
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        new PlayerModelOwner(playerIndex, dayMode);
    }
    fn_1_794C();
    new DrawListOwner();
}
/* Resume the game thread, update request objects and advance the players. */
void MinigameThreadOwner::updateGame()
{
    this->resume(0);
    updateRequestObjects();
    fn_1_6258();
}

/* Run the opening, timed photography round, results and exit request. */
void MinigameThreadOwner::run(void *)
{
    int musicHandle;

    postRequest(0);
    MinigameCore::wipeIn();
    musicHandle = HuAudBGMPlay(MSM_STREAM_SLOW_AND_STEADY);
    while (!requestsCleared()) {
        suspend(0);
    }
    fn_1_654();
    postRequest(1);
    MinigameCore::showStart();
    while (!MinigameCore::messageDone()) {
        suspend(0);
    }
    postRequest(2);
    frame = 0;
    timer.setParameters(0x258,0,0);
    timer.setPosition(286.0f,416.0f);
    timer.setMode(1);
    while (!fn_1_99C()) {
        suspend(0);
    }
    timer.start();
    postRequest(3);
    HuAudSStreamFadeOut(musicHandle, 0x64);
    fn_1_60C();
    fn_1_7C4();
    MinigameCore::showWinners();
    while (!MinigameCore::messageDone()) {
        suspend(0);
    }
    MinigameCore::requestExit();
    for (;;) {
        suspend((void *)-1);
    }
}

/* Show the finish message and wait for it to close. */
void MinigameThreadOwner::fn_1_60C()
{
    MinigameCore::showFinish();
    while (!MinigameCore::messageDone()) {
        suspend(0);
    }
}

/* Reset target state and run the camera and sprite transition. */
void MinigameThreadOwner::fn_1_654()
{
    int openingFrames;
    int secondFrames;
    int thirdFrames;
    int closingFrames;
    CameraOwner *cameraOwner;
    SpriteFade *spriteFade;

    fn_1_9C58();
    for (openingFrames = 0; openingFrames < 0x3C; openingFrames++) {
        suspend(0);
    }
    WipeCreate(2, 6, 0x3C);
    for (secondFrames = 0; secondFrames < 0x3C; secondFrames++) {
        suspend(0);
    }
    cameraOwner = &CameraOwner::getChecked();
    cameraOwner->transitionRequested = 1;
    fn_1_6690();
    WipeCreate(1, 6, 0x3C);
    for (thirdFrames = 0; thirdFrames < 0x3C; thirdFrames++) {
        suspend(0);
    }
    spriteFade = &SpriteFade::getChecked();
    spriteFade->transitionState = 0;
    for (closingFrames = 0; closingFrames < 0x3C; closingFrames++) {
        suspend(0);
    }
}

/* Prepare the results, reveal the scores and select the result jingle. */
void MinigameThreadOwner::fn_1_7C4()
{
    int frame;
    int wipeFrames;
    int closingFrames;
    bool allScoresZero;
    int jingle;

    fn_1_66F4();
    fn_1_9BF4();
    if (dayMode == 0) {
        fn_1_BB5C();
    }
    for (frame = 0; (float)frame < 120.0f; frame++) {
        suspend(0);
    }
    WipeCreate(2, 6, 0x3C);
    for (wipeFrames = 0; wipeFrames < 0x3C; wipeFrames++) {
        suspend(0);
    }
    HuAudFadeOut(0x96);
    allScoresZero = fn_1_62DC();
    fn_1_67A0();
    ScoreDisplay::getChecked().fn_1_83F4();
    DrawListOwner::getChecked().fn_1_3714();
    postRequest(4);
    WipeCreate(1, 6, 0x3C);
    for (closingFrames = 0; closingFrames < 0x78; closingFrames++) {
        suspend(0);
    }
    fn_1_6884();
    if (allScoresZero != 0) {
        jingle = MSM_STREAM_BAD_NEWS;
    } else {
        jingle = MSM_STREAM_WINNING;
    }
    HuAudJinglePlay(jingle);
    if (allScoresZero != 0) {
        fn_1_6614();
    }
}

/* Advance the frame count and finish when the timer or all players are done. */
bool MinigameThreadOwner::fn_1_99C()
{
    bool done;
    frame++;
    done = 1;
    if (timer.isDone() == 0 && fn_1_6594() == 0) {
        done = 0;
    }
    return done;
}

/* Leave game state unchanged. */
void MinigameThreadOwner::fn_1_A08() {}

/* Delete all players and request objects. */
void MinigameThreadOwner::finishGame()
{
    fn_1_59C8();
    deleteRequestObjects();
}
