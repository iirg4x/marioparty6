#include "REL/framework/frame_time_inline.h"
/* Initializes the M648 tree screen's embedded sequence and timer state. */
#include "dolphin/types.h"
#include "dolphin/os.h"
#include "REL/framework/thread.h"
#include "REL/framework/game_object.h"
extern "C" {
#include "game/gamework.h"
#include "game/charman.h"
#include "game/flag.h"
#include "game/audio.h"
}
#include <stdio.h>
#include "REL/m648DLL.h"
#include "REL/framework/minigame_core.h"
extern "C" {
#include "game/mg/timer.h"
}
#include "REL/m648DLL/light_shadow.h"
#include "REL/m648DLL/kuribo_motion_owner.h"
#include "REL/m648DLL/animation_object.h"
#define __ct__11ThreadEntryFUl __ct__11ThreadEntryFUl
#define __ct__10TimerOwnerFi __ct__10TimerOwnerFi
#define __dt__10TimerOwnerFv __dt__10TimerOwnerFv

struct TimerOwner {
    s32 value;
    TimerOwner(int);
    ~TimerOwner();
    void setParameters(int, int, int);
    void setPosition(float, float);
    void setMode(int);
    void start();
};
struct TimerManager;
struct MinigameState : MinigameCore, ThreadEntry {
    TimerManager *resource;
    TimerOwner timer;
    s32 progress;
    bool nightMode;
    u8 unobservedAfterNightMode[3];
    u32 record;
    MinigameState();
    virtual ~MinigameState();
    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    virtual void fn_1_4148();
    virtual void run(void *);
    s32 fn_1_3ED0();
    void fn_1_3F2C();
    void fn_1_406C();
    u8 fn_1_40BC();
};
MinigameState::MinigameState() : ThreadEntry(0x8000), timer(0) {
    record = fn_8003C860(4);
}
MinigameState::~MinigameState() {}
struct MinigameSceneView {
    u8 unobservedPrefix[0x358];
    void *resource;
    u8 unobservedAfterResource[8];
    bool nightMode;
    u8 unobservedAfterNightMode[3];
    s32 record;
};

#include "REL/m648DLL/managers.h"
#include "REL/m648DLL/first_list_object.h"
#include "REL/m648DLL/animation_setup_owner.h"
#include "REL/m648DLL/player.h"

extern "C" {
void RandomSeed__FUl(void);
}

/* Starts the M648 screen resources and refreshes each player's night effect. */
void MinigameState::setupObjects(OMOBJMAN *)
{
    s32 player;
    s32 effect;
    s16 nightSetting;
    s16 nightFlag;
    OSGetTick();
    RandomSeed__FUl();
    nightSetting = lbl_802C0278;
    this->nightMode = ! nightSetting;
    new ParticleAnimationOwner();
    new CameraManager();
    new ScoreManager();
    if (fn_8006A124(0x30002) == 0)
    {
        this->resource = new TimerManager(this->record, 0);
    }
    new AnimationUpdateOwner();
    new KuriboMotionOwner();
    new FirstListObject();
    if (this->nightMode != 0)
    {
        new AnimationSetupOwner();
    }
    fn_8005DDEC(3);
    player = 0;
    while (player < 4)
    {
        nightFlag = lbl_802C0278;
        effect = (player << 1) + (u8)(nightFlag != 0);
        printf("CPlayer %d %d\n", player, lbl_80265728[player].charNo);
        new CPlayer(player, effect, this->nightMode);
        ++player;
    }
    new CallbackModelOwner(this->nightMode);
}

struct ThreadConsumer {
    unsigned char unknown_00[32]; /* No code names this field. */
    unsigned char thread[1];
};

void MinigameState::updateGame()
{
    ThreadEntry::resume(0);
    updateRequestObjects();
    fn_1_B7C4();
}

extern "C" {
void wipeIn__12MinigameCoreFv();
void showStart__12MinigameCoreFv();
u8 messageDone__12MinigameCoreFv();
void requestExit__12MinigameCoreFv();

}

void MinigameState::run(void *)
{
    s32 effectHandle;
    s32 bgmHandle;
    s32 openingCount;
    s32 closingCount;
    u8 finished;

    postRequest(0);
    wipeIn__12MinigameCoreFv();
    if (nightMode != 0) {
        openingCount = 0;
        while ((f32)openingCount < 12.0f) {
            ThreadEntry::suspend(0);
            ++openingCount;
        }
        HuAudFXPlay(MSM_SE_MG648_NIGHT_OPENING);
    } else {
        effectHandle = HuAudFXPlay(MSM_SE_MG648_DAY_OPENING);
        closingCount = 0;
        while ((f32)closingCount < 210.0f) {
            ThreadEntry::suspend(0);
            ++closingCount;
        }
        HuAudFXFadeOut(effectHandle, 0x190);
    }
    while (requestsCleared() == 0) {
        ThreadEntry::suspend(0);
    }
    bgmHandle = HuAudBGMPlay(MSM_STREAM_LAID_BACK);
    postRequest(1);
    showStart__12MinigameCoreFv();
    while (messageDone__12MinigameCoreFv() == 0) {
        ThreadEntry::suspend(0);
    }
    finished = 0;
    postRequest(2);
    timer.setParameters(0x708, 0, 0);
    if (fn_8006A124(0x30002) == 0) {
        timer.setPosition(286.0f, 416.0f);
    } else {
        timer.setMode(1);
        finished = 1;
    }
    progress = 0;
    while ((u8)fn_1_40BC() == 0) {
        if (fn_8006A124(0x30002) == 0 && finished == 0 && progress > 0x3F48) {
            timer.setMode(1);
            finished = 1;
        }
        ThreadEntry::suspend(0);
    }
    if (finished != 0) {
        timer.start();
    }
    HuAudSStreamFadeOut(bgmHandle, 0x64);
    if (fn_8006A124(0x30002) == 0) {
        fn_1_3F2C();
    } else {
        fn_1_406C();
    }
    requestExit__12MinigameCoreFv();
    for (;;) {
        ThreadEntry::suspend((void *)-1);
    }
}

extern "C" {
extern s32 lbl_1_bss_40;
extern s32 lbl_1_bss_48;
extern s32 lbl_1_bss_90;

extern s32 lbl_1_bss_94;
extern s32 lbl_1_bss_98;

}

extern "C" {
}

s32 MinigameState::fn_1_3ED0()
{
    s32 changed = 0;
    u32 frame = fn_1_B62C();
    if (record > frame) {
        record = frame;
        changed = 1;
    }
    return changed;
}

void MinigameState::fn_1_3F2C()
{
    u32 frame;
    u8 changed;
    TimerManager *timerOwner;
    s32 jingleSound;
    s32 jingleEnabled;
    u32 progress;

    fn_1_B934();
    fn_1_BC1C();
    MinigameCore::showFinish();
    while (!MinigameCore::messageDone()) {
        suspend(0);
    }

    postRequest(3);
    changed = 0;
    frame = fn_1_B62C();
    if (record > frame) {
        record = frame;
        changed = 1;
    }

    if (changed && !fn_8006A124(65551)) {
        progress = record;
        timerOwner = resource;
        MgTimerRecordSet(timerOwner->timer, progress);
        fn_8003C80C(4, record);
        MinigameCore::showRecord(record);
        while (!MinigameCore::messageDone()) {
            suspend(0);
        }
    }

    postRequest(4);
    jingleEnabled = fn_1_BEA8();
    if ((u8)jingleEnabled != 0) {
        jingleSound = MSM_STREAM_BAD_NEWS;
    } else {
        jingleSound = MSM_STREAM_WINNING;
    }
    HuAudJinglePlay(jingleSound);
    MinigameCore::showWinners();
    while (!MinigameCore::messageDone()) {
        suspend(0);
    }

}

void fn_1_B8BC(void);
void MinigameState::fn_1_406C()
{
    fn_1_B8BC();
    fn_1_BC1C();
    MinigameCore::showFinish();
    while (!MinigameCore::messageDone()) {
        suspend(0);
    }
}

extern "C" {
extern u8 isDone__10TimerOwnerFv(void *timerOwner);
}

/* Advances the sequence count and checks whether its completion work is ready. */
u8 MinigameState::fn_1_40BC()
{
    u8 ready;

    if (!fn_8006A124(0x30002)) {
        progress++;
        ready = 1;
        if (!fn_1_B848() && !isDone__10TimerOwnerFv(&timer)) {
            ready = 0;
        }
        return ready;
    }
    return isDone__10TimerOwnerFv(&timer);
}

extern "C" {
}
void MinigameState::fn_1_4148() {}
void MinigameState::finishGame() { fn_1_AE90(); deleteRequestObjects(); }
