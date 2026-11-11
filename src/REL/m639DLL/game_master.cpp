#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include "REL/framework/minigame_core.h"
#include "REL/m639DLL/camera_owner.h"
#include "REL/m639DLL/scene_owner.h"
#include "REL/m639DLL/participant_owner.h"
#include "REL/m639DLL/mode_game.h"
#include "REL/m639DLL/mode_selection.h"
#include "REL/m639DLL/timer_owner.h"
#include "REL/framework/game_object_man.h"
#include "REL/framework/thread.h"
#include "REL/framework/sound.h"
#include "REL/framework/list_object.h"
#include "humath.h"

extern "C" {
int lbl_1_bss_18 = secondsToFrames(2L);
int lbl_1_bss_1C = (int)secondsToFrames(3.5);
int lbl_1_bss_20 = (int)secondsToFrames(3.5);
int lbl_1_bss_24 = minutesToFrames(5L);
int lbl_1_bss_28 = secondsToFrames(30L);
float lbl_1_bss_2C = (1.0 / 60.0) * ((1.0 / 60.0) * cmToWorld(980.665f));
}
extern "C" {
#include "game/audio.h"
#include "game/wipe.h"
void fn_1_5F08();
void fn_1_C28C();
}
void RandomSeed(unsigned long);
void process_event_queue();
void sendEvent(int, unsigned long, float);
#include "REL/m639DLL/event_manager.h"
#include "REL/m639DLL/scene_object_owner.h"

// The game state: its events, objects and timer, and the state of the players. The timer counts down
// from the game length less its closing seconds.
struct OwnedSceneChild {
    EventManager baseState;
    GameObjectManager memberState;
    TimerOwner message;
    long timeLeft;
    M639StateOwner callbackState;
    OwnedSceneChild() : message(0), timeLeft(lbl_1_bss_24 - lbl_1_bss_28)
    {
        long timer_total = timeLeft;
        assert(timer_total > 0);
    }
};

SceneObjectOwner::SceneObjectOwner() : ThreadEntry(0x8000), childOwner(new OwnedSceneChild) {}
SceneChildOwner::~SceneChildOwner() { delete child; }
SceneObjectOwner::~SceneObjectOwner() {}
void SceneObjectOwner::setupObjects(OMOBJMAN *)
{
    unsigned char localState[12];

    RandomSeed(OSGetTick());
    new CameraOwner;
    new SceneOwner;
    fn_1_5F08();
    new ParticipantOwner;
    SceneOwner::getChecked().fn_1_5AAC((Vec *)localState);
    fn_1_C28C();
    soundSetCameraLayout(0);
    soundPlay(0x771);
}


void SceneObjectOwner::updateGame() {
    resume(0);
    process_event_queue();
    updateGameObjects();
    updateRequestObjects();
}
extern "C" {
extern const char lbl_1_data_F0[], lbl_1_data_FA[];

extern void fn_1_6058(void);
extern void fn_1_626C(void);
extern void fn_1_63A4(void);

}


void SceneObjectOwner::run(void *initialArgument)
{
    s32 *modeCounter;
    TimerOwner *message;
    s32 music;
    s32 frame;
    u8 showMessage;
    s32 jingle;
    OwnedSceneChild *counterState;
    OwnedSceneChild *messageState;

    counterState = childOwner.child;
    modeCounter = &counterState->timeLeft;
    messageState = childOwner.child;
    message = &messageState->message;

    MinigameCore::wipeIn();
    while (WipeCheckIn() != 0)
        ((ThreadEntry *)(&static_cast<ThreadEntry &>(*this)))->suspend((void *)(0));

    postRequest(0);
    while (requestsCleared() == 0)
        ((ThreadEntry *)(&static_cast<ThreadEntry &>(*this)))->suspend((void *)(0));

    music = HuAudBGMPlay(MSM_STREAM_LAID_BACK);
    postRequest(1);
    MinigameCore::showStart();
    while (MinigameCore::messageDone() == 0)
        ((ThreadEntry *)(&static_cast<ThreadEntry &>(*this)))->suspend((void *)(0));

    postRequest(2);
    fn_1_6058();
    while ((u8)fn_1_112C() == 0) {
        showMessage = 0;
        if (*modeCounter > 0) {
            if (--*modeCounter == 0)
                showMessage = 1;
        }
        if (showMessage != 0) {
            message->setPosition(288.0f, 72.0f);
            message->setParameters(lbl_1_bss_28, 0, 0);
            message->setMode(1);
        }
        ((ThreadEntry *)(&static_cast<ThreadEntry &>(*this)))->suspend((void *)(0));
    }

    sendEvent(0, 2, 0.0f);
    M639StateOwner::getChecked().fn_1_2CAC();
    if (*modeCounter <= 0)
        message->start();
    else
        message->reset();

    postRequest(3);
    fn_1_626C();
    MinigameCore::showFinish();
    HuAudSStreamFadeOut(music, 0x64);

    frame = 0;
    while (frame < secondsToFrames(3L) || requestsCleared() == 0) {
        ((ThreadEntry *)(&static_cast<ThreadEntry &>(*this)))->suspend((void *)(0));
        ++frame;
    }

    fn_1_63A4();
    postRequest(4);
    MinigameCore::showWinners();
    if (M639StateOwner::getChecked().fn_1_2CAC() != -1)
        jingle = MSM_STREAM_WINNING;
    else
        jingle = MSM_STREAM_BAD_NEWS;
    HuAudJinglePlay(jingle);

    while (MinigameCore::messageDone() == 0)
        ((ThreadEntry *)(&static_cast<ThreadEntry &>(*this)))->suspend((void *)(0));
    MinigameCore::requestExit();
    for (;;)
        ((ThreadEntry *)(&static_cast<ThreadEntry &>(*this)))->suspend((void *)(-1));
}




/* Checks the player's selected mode and the two available mode states. */
static inline s32 * ModeOf112C(OwnedSceneChild *modeState)
{
    return &modeState->timeLeft;
}

static inline bool modeIsActive(OwnedSceneChild *modeState)
{
    return *ModeOf112C(modeState) <= 0;
}

static inline OwnedSceneChild *playerModeState(SceneObjectOwner *player)
{
    return player->childOwner.child;
}

static inline bool playerHasSelection(SceneObjectOwner *player)
{
    bool playerSelected = false;
    bool modeActive = modeIsActive(player->childOwner.child);
    if (modeActive) {
        if (playerModeState(player)->message.isDone() != 0) {
            playerSelected = true;
        }
    }
    return playerSelected;
}


static inline bool anyModeSelected()
{
    bool modeAvailable = true;
    if (!ModeGame::getChecked().fn_1_3B0(0)) {
        if (!ModeGame::getChecked().fn_1_3B0(1)) {
            modeAvailable = false;
        }
    }
    return modeAvailable;
}

static inline s32 eitherSelected(bool playerSelected, bool modeAvailable)
{
    return playerSelected || modeAvailable ? 1 : 0;
}

s32 SceneObjectOwner::fn_1_112C()
{
    s32 *mode;
    bool playerSelected;
    bool modeAvailable;
    s32 result;
    bool hasPlayer;
    bool hasMode;
    OwnedSceneChild *modeState;
    bool activeResult;
    bool modeActive;
    OwnedSceneChild *selectedState;

    playerSelected = false;
    modeState = this->childOwner.child;
    mode = &modeState->timeLeft;
    activeResult = *mode <= 0;
    modeActive = activeResult;
    if (modeActive) {
        selectedState = this->childOwner.child;
        if (selectedState->message.isDone() != 0) {
            playerSelected = true;
        }
    }
    hasPlayer = playerSelected;
    modeAvailable = true;
    if (!ModeGame::getChecked().fn_1_3B0(0)) {
        if (!ModeGame::getChecked().fn_1_3B0(1)) {
            modeAvailable = false;
        }
    }
    hasMode = modeAvailable;
    result = 1;
    if (!hasPlayer && !hasMode) {
        result = 0;
    }
    return result;
}

void SceneObjectOwner::finishGame() { deleteRequestObjects(); }
