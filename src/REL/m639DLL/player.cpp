// The players: a sampled input history, their motion, and the lists of them the game walks.
// The asserts keep the original file name and lines (player.cpp, lines 98, 99 and 166). NDEBUG is
// undefined because the build defines it for every object while the game kept these asserts.
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include <vector>
#include <queue>
#include <memory>
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/sound.h"
#include <math.h>
extern "C" {
#include "game/charman.h"
#include "game/data.h"
#include "game/gamework.h"
}
#include "datadir_enum.h"
#include "REL/framework/game_object.h"
#include "REL/m639DLL/player_control.h"
#include "REL/m639DLL/mode_selection.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/model.h"
#include "REL/framework/p_chara.h"
#include "REL/m639DLL/sphere.h"
#include "REL/m639DLL/player.h"
// Model supplies its own conversion to the Hu3D model id.

// The start position of each player slot.
extern "C" {
extern const Vec lbl_1_rodata_2D0[4] = {
    { 719.37067f, -841.4495f, 1805.898f },
    { 401.1726f, -841.4495f, 1487.7003f },
    { 82.97457f, -841.4495f, 1169.5018f },
    { -235.2234f, -841.4495f, 851.30396f }
};
}

// Durations in frames, and the gravity per frame.
// These constants keep translation-unit-local initialized storage.
// Game durations include the closing interval.
#include "REL/m639DLL/game_constants.h"
// Gravity uses world units per frame squared.
// Additional frame durations, kept in their initialized order.
#include "REL/m639DLL/frame_durations.h"
// The two 3.5-second values have separate storage.
// Frame and world-unit conversions are defined by the framework.

/* Creates a zeroed input history and seeds it with sixty inactive samples. */
InputWindow::InputWindow() : activeCount(0)
{
    for (int sample = 0; sample < 60; sample++) {
        queue.push(false);
    }
}

/* The number of active samples in the window. */
int InputWindow::count() const
{
    return activeCount;
}

/* Replaces the oldest sampled input and keeps the count of active samples current. */
void InputWindow::update(const bool active)
{
    if (active) activeCount++;
    if (queue.front()) {
        activeCount--;
    }
    queue.pop();
    queue.push(active);
}

/* Creates each configured player's motion actor and checks both motion groups. */
void fn_1_5F08()
{
    int members[2] = { 0, 0 };
    for (int player = 0; player < 4; player++) {
        int motion =
            (lbl_80265728[player].grpNo == 0)
                ? 0
                : 1;
        if (members[motion] >= 2)
        {
            motion =
                (motion == 0)
                    ? 1
                    : 0;
        }
        int *groupCount = &members[motion];
        int resource = *groupCount + (motion * 2);
        new Player(player, motion, resource);
        (*groupCount)++;
    }
    assert(members[0] == 2);
    assert(members[1] == 2);
}
static inline void setCallback(Player *player, void (Player::*next)()) { player->callback = next; }
void fn_1_6058()
{
    std::vector<Player *> actors;
    fn_1_6444(&actors);
    int index;
    int count = actors.size();
    for (index = 0; index < count; index++) {
        Player *actor = actors[index];
        actor->unknownModelState = 1;
        actor->hook->clear();
        setCallback(actor, &Player::fn_1_7050);
    }
}

void fn_1_626C()
{
    Player *actor;
    int selectedMode = -1;
    int mode;
    int count;
    mode = M639StateOwner::getChecked().fn_1_2CAC();
    switch (mode) {
    case 0: selectedMode = 0; break;
    case 1: selectedMode = 1; break;
    }
    std::vector<Player *> actors;
    fn_1_6444(&actors);
    count = actors.size();
    for (int index = 0; index < count; index++) {
        actor = actors[index];
        if (actor->mode == selectedMode) {
            actor->useAlternateMotion = 1;
            MinigameCore::addWinner(actor->number());
        }
        actor->fn_1_7654();
    }
}
void fn_1_63A4()
{
    std::vector<Player *> actors;
    fn_1_6444(&actors);
    int count = actors.size();
    for (int index = 0; index < count; index++) {
        actors[index]->fn_1_7704();
    }
}
void fn_1_6444(std::vector<Player *> *actors)
{
    std::vector<GameObject *> first;
    std::vector<GameObject *> second;
    collectGameObjects(0, &first);
    collectGameObjects(1, &second);
    first.insert(first.end(), second.begin(), second.end());
    int count = first.size();
    actors->clear();
    for (int index = 0; index < count; index++) {
        actors->push_back(static_cast<Player *>(first[index]));
    }
}
int fn_1_67B4(unsigned int playerNumber)
{
    std::vector<GameObject *> objects;
    collectGameObjects((int)playerNumber / 2, &objects);
    int n = objects.size();
    assert(n > 0);
    for (int index = 0; index < n; index++) {
        Player *player = static_cast<Player *>(objects[index]);
        if (player->selectionNumber - 1 == (int)playerNumber) {
            return player->characterNo();
        }
    }
    return -1;
}

/* Adds every collected actor's adjusted model position to the sequence. */
void fn_1_68D8(std::vector<Sphere> *spheres)
{
    std::vector<Player *> actors;
    int index;
    int loopLimit;
    Player *actorValue;

    fn_1_6444(&actors);
    loopLimit = actors.size();
    spheres->clear();
    index = 0;
    while (index < loopLimit) {
        Sphere value;
        actorValue = actors[index];
        fn_800267CC(*actorValue, &value.center);
        value.center.x -= 120.0;
        value.center.y += 120.0;
        value.center.z += 120.0;
        {
            float radius = 210.0f;
            value.radius = radius;
        }
        spheres->push_back(value);
        index += 1;
    }
}

Player::Player(long value, long mode, long selector)
    : PlayerConfigSelection(value, 2), GameObject(mode), callback(&Player::fn_1_704C),
      itemHook(fn_8005C0F0((short)characterNo(), 4, 0)), mode(mode), selectionNumber(selector + 1), selectionIndex(selector),
      hookModel(mode == 0 ? DATANUM(0x620000, 10) : DATANUM(0x620000, 11)), unknownModelState(0), useAlternateMotion(0),
      targetTime(0.0f), previousTime(0.0f), blend(0.0f), direction(0.0f), previousBeat(0.0f), countdown(0), hook(isComputer() ? static_cast<PlayerControl *>(new MotionCycleOwner(this)) : new ScalarSourceOwner(this))
{
    setLayer(5);
    setShadow();
    fn_1_77B4();
    setPosition(const_cast<Vec *>(&lbl_1_rodata_2D0[selector]));
    rotation.x = 0.0f;
    rotation.y = -45.0f;
    rotation.z = 0.0f;
    setRotation(&rotation);
    short hookModelId = *this;
    fn_8002741C((short)hookModelId, (char *)itemHook, hookModel);
    hookModel.setShadow();
    selectCharacterMotion(&static_cast<MotionOwner &>(*this), DATANUM(0x8e0000, 115));
}

/* Releases the nested motion, input sequence, model, and owner resources. */
Player::~Player() {}

/* Runs the actor's hook and action, then forwards its indexed effect position. */
void Player::update()
{
    hook->update();
    (this->*callback)();
    soundPanCharacterVoice(1, (short)characterNo(), const_cast<Vec *>(&lbl_1_rodata_2D0[selectionIndex]));
}

void Player::fn_1_704C() {}

static inline float blendInput(float oldBlend, float inputBlend)
{
    return (float)(0.8 * (double)oldBlend + 0.2 * (double)inputBlend);
}

static inline bool countDown(int &value)
{
    bool expired = false;
    if (value > 0) {
        if (--value == 0) {
            expired = true;
        }
    }
    return expired;
}
void Player::fn_1_7050()
{
    bool reached;
    if (targetTime == 0.0f)
    {
        if (previousTime != 0.0f)
        {
            setCountdown(secondsToFrames(1L));
        }
        else
        {
            if (countDown(countdown))
            {
                this->direction = 0.0f;
            }
        }
    }
    float difference = targetTime - previousTime;
    float signedDirection;
    if (difference == 0.0f)
    {
        signedDirection = 0.0f;
    }
    else
    {
        float directionSign;
        if (difference < 0.0f)
        {
            directionSign = - 1.0f;
        }
        else
        {
            directionSign = 1.0f;
        }
        signedDirection = directionSign;
    }
    float direction = signedDirection;
    bool directionIsZero = this->direction == 0.0f;
    bool crossed = direction != 0.0f;
    if (directionIsZero && crossed)
    {
        this->direction = direction;
    }
    reached = fabs(targetTime) >= 1.0 && this->direction == direction;
    if (reached)
    {
        this->direction = -this->direction;
        sendEvent(selectionNumber, 0, 0.0f);
    }
    previousTime = targetTime;
    inputHistory.update(reached);
    blend = blendInput(blend, 0.2f * (float)inputHistory.count());
    if (blend < 0.1)
    {
        if (characterMotionShiftInactive((s32 *)&static_cast<MotionOwner &>(*this)))
        {
            shiftCharacterMotion(&static_cast<MotionOwner &>(*this), DATANUM(0x8e0000, 115), 16.0f, 0.0f, ((1 << 30) | (1 << 0)));
            resetAttribute(((1 << 30) | (1 << 1)));
        }
    }
    else
    {
        shiftCharacterMotion(&static_cast<MotionOwner &>(*this), DATANUM(0x8e0000, 116), 8.0f, 5.0f, ((1 << 30) | (1 << 0)));
        setMotionSpeed(blend);
        float beat = static_cast<MotionOwner &>(*this).fn_1_DB20();
        if ((previousBeat < 5.0 && beat >= 5.0) || (previousBeat < 15.0 && beat >= 15.0))
        {
            soundPlayAt(1, 0x76B, const_cast<Vec *>(&lbl_1_rodata_2D0[selectionIndex]));
        }
        previousBeat = beat;
    }
}

/* Advances the actor's turn angle and applies the resulting rotation to its model. */
static inline float nextTurnAngle(float angle, double offset)
{
    return angle * 0.9 + offset;
}

void Player::fn_1_7528()
{
    rotation.y = nextTurnAngle(rotation.y, -18.0);
    setRotation(&rotation);
}

void fn_1_75BC(Player *target, float input)
{
    float normalizedInput;
    if ((input / 0.7f) < -1.0f) {
        normalizedInput = -1.0f;
    } else if ((input / 0.7f) > 1.0f) {
        normalizedInput = 1.0f;
    } else {
        normalizedInput = input / 0.7f;
    }
    target->targetTime = normalizedInput;
}

/* Starts the actor's looping motion and installs its turn update action. */
void Player::fn_1_7654()
{
    fn_8005C7A4((short)characterNo(), (char *)itemHook);
    shiftCharacterMotion(&static_cast<MotionOwner &>(*this), CHARMOT_HSF_c000m1_300, 16.0f, 0.0f, ((1 << 30) | (1 << 0)));
    resetAttribute(((1 << 30) | (1 << 1)));
    setCallback(this, &Player::fn_1_7528);
}

/* Selects the actor's reset motion and installs the idle action. */
void Player::fn_1_7704()
{
    unsigned int motion;
    if (useAlternateMotion) motion = CHARMOT_HSF_c000m1_306;
    else motion = CHARMOT_HSF_c000m1_351;
    shiftCharacterMotion(&static_cast<MotionOwner &>(*this), motion, 16.0f, 0.0f, 0);
    setCallback(this, &Player::fn_1_704C);
}

static const unsigned int characterMotions[5] = {
    DATANUM(0x8e0000, 115), DATANUM(0x8e0000, 116), DATANUM(0x930000, 0), DATANUM(0x930000, 6), DATANUM(0x930000, 40)
};

void Player::fn_1_77B4()
{
    createCharacterMotions(&static_cast<MotionOwner &>(*this), characterMotions, 5);
}
