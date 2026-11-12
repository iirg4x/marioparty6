#include "REL/framework/list_object.h"
#include "REL/m604DLL/manager_owner.h"
#include "REL/m604DLL/score_owner.h"
#include "REL/m604DLL/stage.h"
#include "REL/m604DLL/animation_menu.h"
#include "REL/m604DLL/camera_owner.h"
#include "REL/m604DLL/camera_hook_owner.h"
#include "REL/m604DLL/patapata.h"
#include "REL/m604DLL/player.h"
#include "REL/m604DLL/a_set.h"
#include "REL/framework/timer.h"
#include "REL/framework/phase_durations.h"
#include "dolphin/os.h"
#include "msm_stream.h"
extern "C" {
#include "game/gamework.h"
#include "game/audio.h"
#include "REL/framework/assertion.h"
#include <math.h>
}

// The game's two lists: its players and its Noko.
struct ObjectSets {
    ASet<Player> players;
    ASet<Noko> nokos;
};

/* Builds the manager's list heads and initializes its model state. */
CleanupSequenceOwner::CleanupSequenceOwner() : ThreadEntry(0x8000), m_sets(new ObjectSets), m_timer(new TimerOwner(0))
{
    selectedValue = 0;
    mode = GWMgNightFGet();
    nightFlag = 0;
    timer = 0;
    timerActive = 0;
}

CleanupSequenceOwner::~CleanupSequenceOwner()
{
}

void RandomSeed(unsigned long seed);

extern "C" { BOOL fn_8006A124(u32 flags); }

void updateRequestObjects(void);
u8 fn_1_6FA4(void *object);

void fn_1_7054(void *object);

void fn_1_7104(void *object);

void fn_1_71F4(void *object);

void fn_1_73A4(CleanupSequenceOwner *object);

void fn_1_7534(CleanupSequenceOwner *object);

void fn_1_7614(CleanupSequenceOwner *object);

u8 fn_1_6E88(CleanupSequenceOwner *object);

extern "C" void fn_800611E4(s16, s16, s16, s16);

extern void fn_1_72E4(void *argument);

extern void deleteRequestObjects(void);

void CleanupSequenceOwner::setupObjects(OMOBJMAN *)
{
    s32 playerIndex;
    s32 cameraIndex;

    RandomSeed(OSGetTick());
    if (this->mode == 0) {
        this->nightFlag = fn_8006A124(0x30002U);
    }

    new ParticleAnimations;
    new CameraOwner;
    for (cameraIndex = 0; cameraIndex < 4; cameraIndex++) {
        new CameraHookOwner(cameraIndex);
    }
    new Stage;
    new M604ScoreOwner;
    new Patapata;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (this->mode == 0) {
            new DayPlayer(playerIndex);
        } else {
            new NightPlayer(playerIndex);
        }
    }
    this->selectionState = -1;
}

void CleanupSequenceOwner::updateGame()
{
    this->resume(0);
    updateRequestObjects();
    ASet<Player> &players = ASet<Player>::getChecked();
    players.forEach(&Player::update);
}

void CleanupSequenceOwner::run(void *argument)
{
    s32 bgmId;
    s32 frames60;
    s32 frames120;
    s32 jingleId;
    postRequest(0);
    MinigameCore::wipeIn();
    fn_1_7104(this);
    while (requestsCleared() == 0)
    {
        suspend(0);
    }
    frames60 = 0;
    while ((f32) frames60 < 60.0f)
    {
        suspend(0);
        frames60++;
    }
    if (mode == 0)
    {
        bgmId = HuAudBGMPlay(MSM_STREAM_JAZZY);
    }
    else
    {
        bgmId = HuAudBGMPlay(MSM_STREAM_FRANTIC);
    }
    postRequest(1);
    MinigameCore::showStart();
    while (MinigameCore::messageDone() == 0)
    {
        suspend(0);
    }
    fn_1_71F4(this);
    postRequest(2);
    m_timer->setParameters(0x708, 0, 0);
    if (mode == 0)
    {
        m_timer->setMode(1);
        timerActive = 1;
    }
    M604ScoreOwner &scores = M604ScoreOwner::getChecked();
    scores.fn_1_14FF8();
    while (fn_1_6E88(this) == 0)
    {
        suspend(0);
    }
    fn_1_7054(this);
    HuAudSStreamFadeOut(bgmId, 0x64);
    postRequest(3);
    if (timerActive == 1)
    {
        m_timer->start();
    }
    MinigameCore::showFinish();
    frames120 = 0;
    while (frames120 < 0x78)
    {
        suspend(0);
        frames120++;
    }
    while (fn_1_6FA4(this) == 0)
    {
        suspend(0);
    }
    if (nightFlag == 0)
    {
        postRequest(4);
        fn_1_73A4(this);
        fn_1_7614(this);
        if (selectionState != - 1)
        {
            jingleId = MSM_STREAM_WINNING;
        }
        else
        {
            jingleId = MSM_STREAM_BAD_NEWS;
        }
        HuAudJinglePlay((s16) jingleId);
        MinigameCore::showWinners();
        while (MinigameCore::messageDone() == 0)
        {
        suspend(0);
        }
    }
    else
    {
        fn_1_7534(this);
    }
    MinigameCore::requestExit();
    for (;;)
    {
        suspend(0);
    }
}

// Tells the game the characters of the four players.
void fn_1_6D94(void)
{
    s16 voiceIds[4];
    ASet<Player> &players = ASet<Player>::getChecked();
    Player *node = players.front();
    Node *end = players.end_node();
    s32 count = 0;
    while (node != end) {
        voiceIds[count] = node->getCharacter();
        node = node->next();
        count++;
    }
    fn_800611E4(voiceIds[0], voiceIds[1], voiceIds[2], voiceIds[3]);
}

void CleanupSequenceOwner::finishGame() {
    fn_1_72E4(this);
    deleteRequestObjects();
}

// The round is over: when the time is up or, at night, when a player is down at the bottom.

u8 fn_1_6E88(CleanupSequenceOwner *owner)
{
    if (owner->timerActive == 1) {
        if (owner->m_timer->isDone() != 0) {
            return 1;
        }
    }
    if (owner->mode == 1) {
        if (owner->timer == 0x3F48) {
            owner->m_timer->setMode(1);
            owner->timerActive = 1;
        }
        owner->timer++;
        ASet<Player> &players = ASet<Player>::getChecked();
        Player *node = players.front();
        Node *end = players.end_node();
        while (node != end) {
            if (node->getHeight() == 0) {
                return 1;
            }
            node = node->next();
        }
    }
    return 0;
}

// Whether every player has finished its last motion.
u8 fn_1_6FA4(void *)
{
    ASet<Player> &players = ASet<Player>::getChecked();
    Player *node = players.front();
    Node *end = players.end_node();
    while (node != end) {
        if (node->animationPhase > 0) return 0;
        node = node->next();
    }
    return 1;
}

void fn_1_7054(void *)
{
    ASet<Player> &players = ASet<Player>::getChecked();
    Player *node = players.front();
    Node *end = players.end_node();
    while (node != end) {
        node->finish();
        node = node->next();
    }
}

void fn_1_7104(void *)
{
    ASet<Player> &players = ASet<Player>::getChecked();
    Player *node = players.front();
    Node *end = players.end_node();
    while (node != end) {
        node->ready();
        node = node->next();
    }
}

void fn_1_71F4(void *)
{
    ASet<Player> &players = ASet<Player>::getChecked();
    Player *node = players.front();
    Node *end = players.end_node();
    while (node != end) {
        node->start();
        node = node->next();
    }
}

void fn_1_72E4(void *)
{
    ASet<Player>::getChecked().deleteAll();
}

/* Debug hook. */
static inline void DebugScore(s32 value) { }

// The winning height: the highest by day, the lowest at night; nobody wins a tie at night or a day without a climb.

void fn_1_73A4(CleanupSequenceOwner *context)
{
    ASet<Player> &players = ASet<Player>::getChecked();
    Player *initialNode = players.front();
    Player *node = players.front();
    Node *end = players.end_node();
    s32 selectedValue = 0;
    u8 allMatch;
    if (context->mode == 1) {
        selectedValue = 0xC8;
    }
    while (node != end) {
        s32 value = node->getHeight();
        if (context->mode == 0) {
            if (value > selectedValue) {
                selectedValue = value;
            }
        } else {
            if (value < selectedValue) {
                selectedValue = value;
            }
        }
        node = node->next();
    }
    context->selectedValue = selectedValue;
    if (!context->mode) {
        if (context->selectedValue != 0) {
            context->selectionState = 0xFF;
        }
    } else {
        allMatch = 1;
        node = initialNode;
        while (node != end) {
            if (context->selectedValue != node->getHeight()) {
                allMatch = 0;
            }
            node = node->next();
        }
        if (allMatch == 1) {
            context->selectionState = -1;
            return;
        }
        if (context->selectedValue != 0) {
            context->selectionState = -1;
            return;
        }
        context->selectionState = 0xFF;
    }
}

// The night game's scores: each player's height.

void fn_1_7534(CleanupSequenceOwner *context)
{
    ASet<Player> &players = ASet<Player>::getChecked();
    Player *initialNode = players.front();
    Player *node = players.front();
    Node *end = players.end_node();
    s32 playerIndex = 0;
    while (node != end) {
        s32 observed = node->getHeight();
        DebugScore(observed);
        GWMgScoreSet(playerIndex, node->getHeight());
        node = node->next();
        playerIndex++;
    }
    context->selectionState = -1;
}

// Tells each player whether it won.

void fn_1_7614(CleanupSequenceOwner *context)
{
    ASet<Player> &players = ASet<Player>::getChecked();
    Player *node = players.front();
    Node *end = players.end_node();
    while (node != end) {
        if (context->selectionState != -1) {
            if (context->selectedValue == node->getHeight()) {
                node->result(1);
            } else {
                node->result(0);
            }
        } else {
            node->result(0);
        }
        node = node->next();
    }
}

// The two lists' heads; ObjectSets constructs both.
template <> ASet<Player> *ASet<Player>::sm_ptr = NULL;
template <> ASet<Noko> *ASet<Noko>::sm_ptr = NULL;
