#include "REL/m654DLL/sprite_manager.h"
#include "REL/framework/sound.h"
#include "REL/framework/player_table_1vs1.h"
#include "REL/m654DLL/display_provider.h"
#include "REL/framework/p_info.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include "REL/framework/minigame_core.h"
#include "humath.h"
#include <cmath>
using std::fmod;
#include "REL/framework/random.h"
#include "REL/framework/event.h"
// The two contestants: their walk along queued points, their reactions to phase events and their wrapped facing.
#include <vector>
#include <memory>

extern "C" {
#include "game/mg/actman.h"
}
#include "REL/m654DLL/pad_control.h"
#include "REL/framework/chara_motion.h"
#include "REL/framework/event_packet.h"
#include "REL/framework/game_object.h"
#include "REL/framework/range.h"
#include "REL/m654DLL/model_animation_owner.h"

#include "REL/m654DLL/mode_object.h"

// Tests fn_8005C240; characterMotionShiftInactive tests a different engine routine.
u8 characterMotionEnded(s32 *);

#include "REL/m654DLL/player.h"
#include "REL/m654DLL.h"

int fn_1_4C98(COL_NARROW_PARAM *filter, COL_NARROW_PARAM *other);

// One-minute time limit and gravity in world units per frame squared.
int lbl_1_bss_60 = minutesToFrames(1L);
float lbl_1_bss_64 = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
#include "REL/framework/phase_durations.h"

const int entry_num = 2;
std::vector<Player *> sm_players;
int lbl_1_bss_8C;
// Each player's start point and the two points of its walk in, given in centimetres.
PlayerInitializationData lbl_1_data_220[2] = {
    {
        {cmToWorld(-750.0f), cmToWorld(526.0f), cmToWorld(-1200.0f)},
        {
            {cmToWorld(-650.0f), cmToWorld(526.0f), cmToWorld(-650.0f)},
            {cmToWorld(-140.0f), cmToWorld(110.0f), cmToWorld(-60.0f)}
        }
    },
    {
        {cmToWorld(750.0f), cmToWorld(526.0f), cmToWorld(-1200.0f)},
        {
            {cmToWorld(650.0f), cmToWorld(526.0f), cmToWorld(-650.0f)},
            {cmToWorld(140.0f), cmToWorld(110.0f), cmToWorld(-60.0f)}
        }
    }
};

static const unsigned int playerMotions[3] = {
    0x93001E, 0x930006, 0x930028
};

// Linearly interpolates a player position component.
static inline double blendPlayerPosition(double start, double end, double weight)
{
    return (1.0 - weight) * start + weight * end;
}

// Creates both players and randomly chooses which one leads the exit walk.
void fn_1_3274()
{
    s32 playerNumbers[2];
    int playerNo;
    Player *createdPlayer;
    PlayerInitializationData *playerData = lbl_1_data_220;
    int playerIndex;
    makePlayerTable1vs1(playerNumbers);
    sm_players.clear();
    playerNo = 0;
    while (playerNo < 2) {
        createdPlayer = new Player(playerNumbers[playerNo], playerData++);
        sm_players.push_back(createdPlayer);
        playerNo++;
    }
    playerIndex = RandomInteger(2);
    Player *&chosen = sm_players[playerIndex];
    chosen->leadsExit = 1;
    fn_8005DDEC(3);
    lbl_1_bss_8C = 0;
}

// Starts the requested entrance step for both players.
void fn_1_3398(int setupMode)
{
    int index;
    Player *player;

    assert(sm_players.size() == entry_num);
    for (index = 0; index < 2; index++) {
        player = sm_players[index];
        if (setupMode == 0)
            player->fn_1_3E84();
        else
            player->fn_1_3FE0();
    }
    if (setupMode == 0) {
        MinigameSingleton<ModelAnimationOwner>::getChecked().fn_1_2218();
    }
}
// Enables actor control for both players.
void fn_1_3494()
{
    int playerIndex;
    Player *player;
    assert(sm_players.size() == entry_num);
    for (playerIndex = 0; playerIndex < 2; playerIndex += 1) {
        player = sm_players[playerIndex];
        player->fn_1_411C();
    }
}
// Reports whether both players have reached their idle state.
unsigned char fn_1_3528()
{
    int index;
    Player *player;
    void (Player::*expected)();
    assert(sm_players.size() == entry_num);
    for (index = 0; index < 2; index += 1) {
        player = sm_players[index];
        expected = &Player::fn_1_35F4;
        if (player->callback != expected) return 0;
    }
    return 1;
}

// Leaves the player state idle.
void Player::fn_1_35F4() {}

// Marks both players as finishing and queues their final positions.
void fn_1_35F8()
{
    int playerIndex;
    assert(sm_players.size() == entry_num);
    playerIndex = 0;
    while (playerIndex < 2) {
        Player *player = sm_players[playerIndex];
        Vec position;
        position.x = (float)blendPlayerPosition(-140.0, 140.0, (double)playerIndex);
        position.y = 110.0f;
        position.z = 200.0f;
        player->fn_1_4610();
        player->path.push_back(position);
        playerIndex++;
    }
}
// Selects the result motion for both players.
void fn_1_3768()
{
    int playerIndex;
    Player *player;
    assert(sm_players.size() == entry_num);
    playerIndex = 0;
    while (playerIndex < 2) {
        player = sm_players[playerIndex];
        player->fn_1_4A0C();
        playerIndex += 1;
    }
}

// Returns the number of players who have reached three score marks.
int fn_1_37FC() { return lbl_1_bss_8C; }

// Creates the actor, collision hook, score display and human or computer control.
Player::Player(int playerId, PlayerInitializationData *spawnData)
    : PlayerInfo(playerId),
      MotionOwner(characterNo()),
      GameObject(0),
      callback(&Player::fn_1_35F4),
      inactive(1),
      collidedThisFrame(0),
      leadsExit(0),
      finishing(0),
      hasWon(0),
      display(fn_1_6EA8(playerId, characterNo(),
                        (int)(spawnData->position.x == 0.0f ? 0.0f :
                              (spawnData->position.x < 0.0f ? -1.0f : 1.0f)))),
      path(spawnData->path, spawnData->path + 2),
      position(spawnData->position),
      rotationY(0.0f),
      mode(isComputer() ? static_cast<PlayerControl *>(new ComputerControl(this))
                                 : static_cast<PlayerControl *>(new HumanControl(this)))
{
    MGACTOR_PARAM playerParameters = {0.0f, 0.0f, 0, 0, fn_1_4C98, 0, 0, 0};
    short modelId;
    void (Player::*nextCallback)();

    playerParameters.height = height();
    playerParameters.radius = 42.0f;
    playerParameters.param = playerId;
    player = fn_80080728((short)playerId, &playerParameters, 1, 1, 0x33, 0);
    fn_80080DD4(player->actor, 1);
    fn_80087158(player, 1);
    fn_8007ED60(player);
    createCharacterMotions(&static_cast<MotionOwner &>(*this), playerMotions, 3);
    modelId = (short)player->actor->mdlId;
    fn_80026B8C(modelId, 3);
    velocity.x = 0.0f;
    velocity.y = 0.0f;
    velocity.z = 0.0f;
    fn_80086AA8(player->actor, &position);
    fn_80086CA8(player->actor, rotationY);
    fn_80086E3C(player);
    fn_1_4BC0(0);
    fn_1_4C34(0);
    registerChannel(number() + 1);
    // The constructor stores the idle callback again after initialization.
    nextCallback = &Player::fn_1_35F4;
    callback = nextCallback;
}

// Releases the player control and queued path.
Player::~Player() {}

// Runs the player state, samples actor movement and clears the collision flag.
void Player::update()
{
    (this->*callback)();
    fn_1_4ACC();
    collidedThisFrame = 0;
}

// Handles score and reaction events.
void Player::receive(const EventPacket *event)
{
    switch (event->type) {
    case 2:
        fn_1_4250();
        return;
    case 3:
        fn_1_42DC();
    }
}

// Enables actor movement and starts the first entrance walk.
void Player::fn_1_3E84()
{
    void (Player::*nextCallback)();

    inactive = 0;
    fn_80086F00(player, &position);
    fn_1_4BC0(true);
    fn_1_4C34(1);
    fn_8007E818(player);
    nextCallback = &Player::fn_1_3F20;
    callback = nextCallback;
}

// Walks to the first queued point and returns to idle when it is reached.
void Player::fn_1_3F20()
{
    void (Player::*nextCallback)();
    Vec *target = path.begin();

    if (fn_8007E8B4(player, target, (float)framesToSeconds(250.0), 10.0f)) {
        path.erase(path.begin());
        nextCallback = &Player::fn_1_35F4;
        callback = nextCallback;
    }
}

// Starts the second entrance walk.
void Player::fn_1_3FE0()
{
    void (Player::*next)();

    fn_8007EEBC(player, 1);
    next = &Player::fn_1_404C;
    callback = next;
}

// Walks to the next queued point and changes actor mode on arrival.
void Player::fn_1_404C()
{
    void (Player::*nextCallback)();
    Vec *target = path.begin();

    if (fn_8007E8B4(player, target, (float)framesToSeconds(1000.0), 20.0f) != 0) {
        path.erase(path.begin());
        fn_80080DD4(player->actor, 2);
        nextCallback = &Player::fn_1_35F4;
        callback = nextCallback;
    }
}

// Enters the selected controller and starts gameplay input.
void Player::fn_1_411C()
{
    void (Player::*nextCallback)();

    fn_80087168(player, 1);
    mode->enter();
    nextCallback = &Player::fn_1_41A8;
    callback = nextCallback;
}

// Runs gameplay control or starts the finish walk, and sends panel press events.
void Player::fn_1_41A8()
{
    if (finishing != 0) {
        fn_1_4654();
        return;
    }
    mode->update();
    if (fn_80087114(player, 0x200) != 0) {
        sendEvent(number() + 1, 1, &position, 0.0f);
    }
}

// Adds a score mark and registers the player as a winner on the third mark.
void Player::fn_1_4250()
{
    DisplayProvider *scoreDisplay = display;
    int previousProgress = scoreDisplay->activeCount;
    int comparisonProgress = previousProgress;

    if (comparisonProgress < 3 && display->fn_1_66F4() >= 3) {
        hasWon = 1;
        lbl_1_bss_8C += 1;
        int winnerPlayerNo = playerNo;
        MinigameCore::addWinner(winnerPlayerNo);
    }
}

// Stops gameplay control and starts the reaction motion.
void Player::fn_1_42DC()
{
    void (Player::*expected)() = &Player::fn_1_41A8;
    void (Player::*nextCallback)();

    if (callback == expected) {
        inactive = 1;
        fn_80086E3C(player);
        fn_8007EDBC(player);
        selectCharacterMotion(&static_cast<MotionOwner &>(*this), 0x93001E);
        nextCallback = &Player::fn_1_43A0;
        callback = nextCallback;
    }
}

// Rumbles the controller and waits for the reaction motion.
void Player::fn_1_43A0()
{
    void (Player::*nextCallback)();

    rumble(2, (int)secondsToFrames(0.8));
    nextCallback = &Player::fn_1_4428;
    callback = nextCallback;
}
// Restores actor movement after the reaction motion and starts its turn.
void Player::fn_1_4428()
{
    void (Player::*nextCallback)();
    if (characterMotionEnded(&static_cast<MotionOwner &>(*this).charNo) != 0) {
        inactive = 0;
        fn_80086F00(player, &position);
        fn_8007ED60(player);
        fn_8008718C(player, 2, 180.0f + rotationY, (int)secondsToFrames(0.5));
        nextCallback = &Player::fn_1_44FC;
        callback = nextCallback;
    }
}
// Waits through the actor turn and resumes gameplay control.
void Player::fn_1_44FC()
{
    void (Player::*nextCallback)();
    switch (player->subMode) {
    case 1:
        fn_8005C2C8(player->charNo, player->omObj->mtnId[0xE],
                    0.0f, 5.0f, ((1 << 30) | (1 << 0)));
        player->motNo = 0xE;
        player->timer = (float)secondsToFrames(1.5);
        player->subMode = 2;
        return;
    default:
        mode->enter();
        nextCallback = &Player::fn_1_41A8;
        callback = nextCallback;
        /* fall through */
    case 2:
        return;
    }
}
// Sets the actor attribute and marks this player as finishing.
void Player::fn_1_4610()
{
    fn_80087158(player, (1 << 0));
    finishing = 1;
}
// Enables the actor walk and switches to the queued finish path.
void Player::fn_1_4654()
{
    void (Player::*nextCallback)();
    fn_8007E818(player);
    nextCallback = &Player::fn_1_46BC;
    callback = nextCallback;
}
// Walks to the finish point, passes the lead to the other player and signals a winner.
void Player::fn_1_46BC()
{
    Player *selected;
    if (collidedThisFrame != 0 && leadsExit != 0) {
        fn_8007EEBC(player, 1);
    }
    Vec *target = path.begin();
    if (fn_8007E8B4(player, target, (float)framesToSeconds(400.0), 10.0f) != 0 && fn_1_4B68() != 0) {
        if (leadsExit != 0) {
            int index = 0;
            while (index < 2) {
                selected = sm_players[index];
                if (selected->leadsExit == 0) {
                    selected->leadsExit = 1;
                    break;
                }
                index++;
            }
            leadsExit = 0;
        }
        if (hasWon != 0) {
            sendEvent(0, 4, &position, 0.0f);
        }
        void (Player::*next)() = &Player::fn_1_4858;
        callback = next;
    }
}
// Turns the actor toward the camera and returns to idle when aligned.
void Player::fn_1_4858()
{
    Vec *actorPosition;
    Vec cameraPosition;
    Vec cameraTarget;
    float rotation;
    float targetAngle;
    float difference;
    float absoluteDifference;
    float error;
    if (player->motNo != 0) {
        return;
    }
    // The original passes the same destination for camera up and target.
    fn_80027CD0(1, &cameraPosition, &cameraTarget, &cameraTarget);
    actorPosition = &player->actor->pos;
    targetAngle = (float)(180.0 * (std::atan2((double)(cameraPosition.x - actorPosition->x),
                                         (double)(cameraPosition.z - actorPosition->z)) /
                                   M_PI));
    fn_80086CB0(player->actor, &rotation);
    Range<float> range(0.0f, 360.0f, rotation);
    void (Player::*nextCallback)();
    fn_80086CA8(player->actor, range.step(targetAngle, 0.1));
    difference = range.distanceTo(targetAngle);
    absoluteDifference = __fabsf(difference);
    error = absoluteDifference;
    if (error < 1.0) {
        // This updates only the local range; the snapped angle is never applied to the actor.
        range.set(targetAngle);
        fn_80087158(player, (1 << 2));
        nextCallback = &Player::fn_1_35F4;
        callback = nextCallback;
    }
}
// Selects the win or loss motion and returns to idle.
void Player::fn_1_4A0C()
{
    unsigned int data;
    void (Player::*nextCallback)();
    if (hasWon != 0) {
        data = 0x930006;
    } else {
        data = 0x930028;
    }
    shiftCharacterMotion(&static_cast<MotionOwner &>(*this), data, (float)secondsToFrames(0.3), 0.0f, 0);
    nextCallback = &Player::fn_1_35F4;
    callback = nextCallback;
}
// Samples actor position and facing, derives velocity and pans character audio.
void Player::fn_1_4ACC()
{
    if (inactive == 0) {
        Vec originalPosition = position;
        fn_80086BC8(player->actor, &position);
        PSVECSubtract(&position, &originalPosition, &velocity);
        fn_80086CB0(player->actor, &rotationY);
        soundPanCharacterVoice(1, (short)characterNo(), &position);
    }
}
// Reports whether the actor collision code contains any bit in 0x407F.
unsigned char Player::fn_1_4B68()
{
    u32 collisionCode = 0;
    fn_80086DD0(player->actor, &collisionCode);
    return (unsigned char)((collisionCode & 0x407F) != 0);
}
// Toggles character attribute bit 1 and the paired engine character setting.
void Player::fn_1_4BC0(bool enabled)
{
    int character = characterNo();
    if (enabled) {
        fn_8005C47C(character, 1);
    } else {
        fn_8005C430(character, 1);
    }
    fn_8005DFC0(character, enabled);
}
// Toggles the actor model through the engine model helpers.
void Player::fn_1_4C34(unsigned char enabled)
{
    int modelId = (short)player->actor->mdlId;
    if (enabled) {
        fn_80026E44((short)modelId);
    } else {
        fn_80026EF0((short)modelId);
    }
}
// Marks the player named by a type-zero collision pair as collided this frame.
int fn_1_4C98(COL_NARROW_PARAM *filter, COL_NARROW_PARAM *other)
{
    int index;
    Player *match;
    if (filter->type != 0 || other->type != 0) {
        return 0;
    }
    assert(sm_players.size() == entry_num);
    match = 0;
    index = 0;
    while (index < 2) {
        Player *entry = sm_players[index];
        if (filter->paramB == entry->player->actor->param) {
            match = entry;
            break;
        }
        index++;
    }
    if (match == 0) {
        return 0;
    }
    match->collidedThisFrame = 1;
    return 1;
}
