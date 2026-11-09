#include "REL/m603DLL/frame_time.h"
#include "REL/m603DLL/callback_owner.h"
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
#include "REL/m603DLL/framework/thread.h"


#include "REL/m603DLL/random.h"
#include "msm_stream.h"
#include "REL/m603DLL/singleton_32930.h"

MinigameThreadOwner::MinigameThreadOwner() : ThreadEntry(0x8000),timer(0) {}
MinigameThreadOwner::~MinigameThreadOwner() {}

extern "C" {
#include "game/wipe.h"
extern s16 lbl_802C0278;
}






void fn_1_794C(void);

void MinigameThreadOwner::setupObjects(OMOBJMAN *)
{
    s32 playerIndex;
    s16 nightSetting;

    RandomSeed(OSGetTick());
    nightSetting = lbl_802C0278;
    nightMode = !nightSetting;

    new ShadowModelOwner(nightMode);
    new CameraOwnerBBC();
    new DrawModelPool();
    new ScoreDisplay();
    if (nightMode == 0) {
        new CameraModelPool();
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        new PlayerModelOwner(playerIndex, nightMode);
    }
    fn_1_794C();
    new DrawListOwner();
}
void MinigameThreadOwner::updateGame()
{
    this->resume(0);
    fn_1_1A9F8();
    fn_1_6258();
}


void fn_1_9C58(void);
void fn_1_6690(void);
bool fn_1_1A96C(void);
void fn_1_66F4(void);
void fn_1_9BF4(void);
void fn_1_BB5C(void);
extern "C" {
}
bool fn_1_62DC(void);
void fn_1_67A0(void);
void fn_1_6884(void);
void fn_1_6614(void);
extern "C" {
void HuAudFadeOut(int);
int HuAudJinglePlay(int);
int HuAudBGMPlay(int);
void HuAudSStreamFadeOut(int, int);
}

void MinigameThreadOwner::run(void *)
{
    int musicHandle;

    fn_1_1A8D4(0);
    MinigameCore::wipeIn();
    musicHandle = HuAudBGMPlay(MSM_STREAM_SLOW_AND_STEADY);
    while (!fn_1_1A96C()) {
        suspend(0);
    }
    fn_1_654();
    fn_1_1A8D4(1);
    MinigameCore::showStart();
    while (!MinigameCore::messageDone()) {
        suspend(0);
    }
    fn_1_1A8D4(2);
    frame = 0;
    timer.setParameters(0x258,0,0);
    timer.setPosition(286.0f,416.0f);
    timer.setMode(1);
    while (!fn_1_99C()) {
        suspend(0);
    }
    timer.start();
    fn_1_1A8D4(3);
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

void MinigameThreadOwner::fn_1_60C()
{
    MinigameCore::showFinish();
    while (!MinigameCore::messageDone()) {
        suspend(0);
    }
}

void MinigameThreadOwner::fn_1_654()
{
    int openingFrames;
    int secondFrames;
    int thirdFrames;
    int closingFrames;
    CameraOwnerBBC *openingState;
    SpriteFade *closingState;

    fn_1_9C58();
    openingFrames = 0;
    while (openingFrames < 0x3C) {
        suspend(0);
        openingFrames++;
    }
    WipeCreate(2, 6, 0x3C);
    secondFrames = 0;
    while (secondFrames < 0x3C) {
        suspend(0);
        secondFrames++;
    }
    openingState = (&CameraOwnerBBC::getChecked());
    openingState->transitionRequested = 1;
    fn_1_6690();
    WipeCreate(1, 6, 0x3C);
    thirdFrames = 0;
    while (thirdFrames < 0x3C) {
        suspend(0);
        thirdFrames++;
    }
    closingState = (&SpriteFade::getChecked());
    closingState->transitionState = 0;
    closingFrames = 0;
    while (closingFrames < 0x3C) {
        suspend(0);
        closingFrames++;
    }
}

void MinigameThreadOwner::fn_1_7C4()
{
    int frame;
    int wipeFrames;
    int closingFrames;
    bool musicState;
    s16 jingle;

    fn_1_66F4();
    fn_1_9BF4();
    if (nightMode == 0) {
        fn_1_BB5C();
    }
    frame = 0;
    while ((float)frame < 120.0f) {
        suspend(0);
        frame++;
    }
    WipeCreate(2, 6, 0x3C);
    for (wipeFrames = 0; wipeFrames < 0x3C; wipeFrames++) {
        suspend(0);
    }
    HuAudFadeOut(0x96);
    musicState = fn_1_62DC();
    fn_1_67A0();
    (&ScoreDisplay::getChecked())->fn_1_83F4();
    DrawListOwner::getChecked().fn_1_3714();
    fn_1_1A8D4(4);
    WipeCreate(1, 6, 0x3C);
    for (closingFrames = 0; closingFrames < 0x78; closingFrames++) {
        suspend(0);
    }
    fn_1_6884();
    if (musicState != 0) {
        jingle = MSM_STREAM_BAD_NEWS;
    } else {
        jingle = MSM_STREAM_WINNING;
    }
    HuAudJinglePlay(jingle);
    if (musicState != 0) {
        fn_1_6614();
    }
}


unsigned char fn_1_6594();

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

void MinigameThreadOwner::fn_1_A08() {}

void MinigameThreadOwner::finishGame()
{
    fn_1_59C8();
    fn_1_1AA68();
}

extern "C" {
long lbl_1_bss_0 = secondsToFrames(2L);
long lbl_1_bss_4 = (long)secondsToFrames(3.5);
long lbl_1_bss_8 = (long)secondsToFrames(3.5);
}
