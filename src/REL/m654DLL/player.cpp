#include "REL/m654DLL/sprite_manager.h"
#include "REL/m654DLL/display_provider.h"
#include "REL/m654DLL/player_height_view.h"
#include "REL/m654DLL/time_conversion.h"
#include "REL/framework/minigame_core.h"
#include "REL/m654DLL/sound_spatial.h"
#include "humath.h"
template<class Time> Time secondsToFrames(Time);
template<class Time> Time framesToSeconds(Time);
long RandomInteger(long limit);
#include "REL/m654DLL/framework_calls.h"
// The two contestants: their walk along queued points, their reactions to phase events and their wrapped facing.
#include <vector>
#include <memory>
extern "C" {
#include "game/mg/actman.h"
#include "REL/m654DLL/pad_control.h"
}
#include "REL/framework/chara_motion.h"
#include "REL/framework/event_packet.h"
#include "REL/framework/game_object.h"
#include "REL/m654DLL/framework/range_impl.h"
#include "REL/m654DLL/model_animation_owner.h"
#include "Runtime.PPCEABI.H/NMWException.h"

#include "REL/m654DLL/mode_object.h"

void createCharacterMotions(MotionOwner *, unsigned int *, s32);
void selectCharacterMotion(MotionOwner *, unsigned int);
u8 characterMotionEnded(s32 *);
void shiftCharacterMotion(MotionOwner *, unsigned int, float, float, u32);

/* Coordinates go through an inline unit conversion: the startup code proves one inline call per value. */
static inline float cmToWorld(float centimeters) { return centimeters; }

struct PlayerInitializationData {
    Vec position;
    Vec path[2];
};

extern "C" {
void __msl_assertion_failed(const char *condition, const char *file, int line);
}
void fn_1_1C4E8(s32 *playerNo, s32 number);
int fn_1_1C558(s32 *playerNo);
unsigned char fn_1_1C590(s32 *playerNo);
int fn_1_1C5D8(s32 *playerNo, s32 type, s32 duration);
void fn_1_9B98(s32 *playersByGroup);
extern "C" {

void fn_1_3274();
void fn_1_3398(int setupMode);
void fn_1_3494();
int fn_1_3528();
void fn_1_35F8();
void fn_1_3768();
int fn_1_37FC();
void fn_1_4BC0(void *state, bool enabled);
void fn_1_4C34(Player *owner, unsigned char enabled);
unsigned char fn_1_4B68(Player *object);
int fn_1_4C98(const struct PlayerFilter *filter, const struct PlayerFilter *other);

double fmod(double, double);
double atan2(double y, double x);
int lbl_1_bss_60 = fn_1_B80(1);
float lbl_1_bss_64 = framesToSeconds(framesToSeconds(980.665));
int lbl_1_bss_68 = secondsToFrames<long>(2);
int lbl_1_bss_6C = secondsToFrames(3.5);
int lbl_1_bss_70 = secondsToFrames(3.5);

const int entry_num = 2;
std::vector<Player *> sm_players;
int lbl_1_bss_8C;
PlayerInitializationData lbl_1_data_220[2] = {
    {{cmToWorld(-750.0f), cmToWorld(526.0f), cmToWorld(-1200.0f)}, {{cmToWorld(-650.0f), cmToWorld(526.0f), cmToWorld(-650.0f)}, {cmToWorld(-140.0f), cmToWorld(110.0f), cmToWorld(-60.0f)}}},
    {{cmToWorld(750.0f), cmToWorld(526.0f), cmToWorld(-1200.0f)}, {{cmToWorld(650.0f), cmToWorld(526.0f), cmToWorld(-650.0f)}, {cmToWorld(140.0f), cmToWorld(110.0f), cmToWorld(-60.0f)}}}
};

}

struct PlayerInfo : public PlayerHeightView {
    PlayerInfo(int no) { fn_1_1C4E8(&playerNo, no); }
    int number() const { return playerNo; }
};

class Player : public PlayerInfo, public MotionOwner, public GameObject, public EventListener {
public:
    Player(int playerId, PlayerInitializationData *spawnData);
    virtual ~Player();
    virtual void update();
    virtual void receive(const EventPacket *event);
    void fn_1_35F4();
    void fn_1_3E84();
    void fn_1_3F20();
    void fn_1_3FE0();
    void fn_1_404C();
    void fn_1_411C();
    void fn_1_41A8();
    void fn_1_4250();
    void fn_1_42DC();
    void fn_1_43A0();
    void fn_1_4428();
    void fn_1_44FC();
    void fn_1_4610();
    void fn_1_4654();
    void fn_1_46BC();
    void fn_1_4858();
    void fn_1_4A0C();
    void fn_1_4ACC();

    void (Player::*callback)();
    MGPLAYER *player;
    unsigned char inactive : 1;
    unsigned char started : 1;
    unsigned char reachedPlace : 1;
    unsigned char attributeEnabled : 1;
    unsigned char transitionStarted : 1;
    unsigned char flagsLow : 3;
    GameObject *display;
    std::vector<Vec> path;
    Vec position;
    Vec velocity;
    float rotationY;
    std::auto_ptr<PlayerControl> mode;
};

static const unsigned int playerMotions[3] = {0x93001E, 0x930006, 0x930028};

void fn_1_3274()
{
    s32 playerNumbers[2];
    int playerNo;
    Player *createdPlayer;
    PlayerInitializationData *playerData = lbl_1_data_220;
    int playerIndex;
    fn_1_9B98(playerNumbers);
    sm_players.clear();
    playerNo = 0;
    while (playerNo < 2) {
        createdPlayer = new Player(playerNumbers[playerNo], playerData++);
        sm_players.push_back(createdPlayer);
        playerNo++;
    }
    playerIndex = RandomInteger(2);
    Player *&chosen = sm_players[playerIndex];
    chosen->reachedPlace = 1;
    fn_8005DDEC(3);
    lbl_1_bss_8C = 0;
}

void fn_1_3398(int setupMode)
{
    int index;
    Player *player;

    (sm_players.size() == entry_num) ? (void)0
                               : __msl_assertion_failed("sm_players.size() == entry_num", "player.cpp", 0x67);
    for (index = 0; index < 2; index++) {
        player = sm_players[index];
        if (setupMode == 0) {
            player->fn_1_3E84();
        } else {
            player->fn_1_3FE0();
        }
    }
    if (setupMode == 0) {
        MinigameSingleton<ModelAnimationOwner>::getChecked().fn_1_2218();
    }
}

void fn_1_3494()
{
    int playerIndex;
    Player *player;

    (sm_players.size() == entry_num) ? (void)0
                               : __msl_assertion_failed("sm_players.size() == entry_num", "player.cpp", 0x78);
    playerIndex = 0;
    while (playerIndex < 2) {
        player = sm_players[playerIndex];
        player->fn_1_411C();
        playerIndex += 1;
    }
}

int fn_1_3528()
{
    int index;
    Player *player;
    void (Player::*expected)();

    (sm_players.size() == entry_num) ? (void)0
                               : __msl_assertion_failed("sm_players.size() == entry_num", "player.cpp", 0x84);
    index = 0;
    while (index < 2) {
        player = sm_players[index];
        expected = &Player::fn_1_35F4;
        if (player->callback != expected) return 0;
        index += 1;
    }
    return 1;
}

void Player::fn_1_35F4() {}

static inline double blendPlayerPosition(double start, double end, double weight)
{
    return (1.0 - weight) * start + weight * end;
}

void fn_1_35F8()
{
    int playerIndex;

    (sm_players.size() == entry_num) ? (void)0
                               : __msl_assertion_failed("sm_players.size() == entry_num", "player.cpp", 0x94);
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

void fn_1_3768()
{
    int playerIndex;
    Player *player;

    (sm_players.size() == entry_num) ? (void)0
                               : __msl_assertion_failed("sm_players.size() == entry_num", "player.cpp", 0xA6);
    playerIndex = 0;
    while (playerIndex < 2) {
        player = sm_players[playerIndex];
        player->fn_1_4A0C();
        playerIndex += 1;
    }
}

int fn_1_37FC() { return lbl_1_bss_8C; }

Player::Player(int playerId, PlayerInitializationData *spawnData)
    : PlayerInfo(playerId),
      MotionOwner(fn_1_1C558(&playerNo)),
      GameObject(0),
      callback(&Player::fn_1_35F4),
      inactive(1),
      started(0),
      reachedPlace(0),
      attributeEnabled(0),
      transitionStarted(0),
      display(fn_1_6EA8(playerId, fn_1_1C558(&playerNo),
                        (int)(spawnData->position.x == 0.0f ? 0.0f :
                              (spawnData->position.x < 0.0f ? -1.0f : 1.0f)))),
      path(spawnData->path, spawnData->path + 2),
      position(spawnData->position),
      rotationY(0.0f),
      mode(fn_1_1C590(&playerNo) != 0 ? static_cast<PlayerControl *>(new ComputerControl(this))
                                 : static_cast<PlayerControl *>(new HumanControl(this)))
{
    MGACTOR_PARAM playerParameters = {0.0f, 0.0f, 0, 0, (COL_NARROW_HOOK)fn_1_4C98, 0, 0, 0};
    short modelId;
    void (Player::*nextCallback)();

    playerParameters.height = fn_1_37688();
    playerParameters.radius = 42.0f;
    playerParameters.param = playerId;
    player = fn_80080728((short)playerId, &playerParameters, 1, 1, 0x33, 0);
    fn_80080DD4(player->actor, 1);
    fn_80087158(player, 1);
    fn_8007ED60(player);
    createCharacterMotions(&static_cast<MotionOwner &>(*this), const_cast<unsigned int *>(playerMotions), 3);
    modelId = (short)player->actor->mdlId;
    fn_80026B8C(modelId, 3);
    velocity.x = 0.0f;
    velocity.y = 0.0f;
    velocity.z = 0.0f;
    fn_80086AA8(player->actor, &position);
    fn_80086CA8(player->actor, rotationY);
    fn_80086E3C(player);
    fn_1_4BC0(this, 0);
    fn_1_4C34(this, 0);
    registerChannel(number() + 1);
    nextCallback = &Player::fn_1_35F4;
    callback = nextCallback;
}

Player::~Player() {}

void Player::update()
{
    (this->*callback)();
    fn_1_4ACC();
    started = 0;
}

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

void Player::fn_1_3E84()
{
    void (Player::*values)();

    inactive = 0;
    fn_80086F00(player, &position);
    fn_1_4BC0(this, true);
    fn_1_4C34(this, 1);
    fn_8007E818(player);
    values = &Player::fn_1_3F20;
    callback = values;
}

void Player::fn_1_3F20()
{
    void (Player::*initialPosition)();
    Vec *target = path.begin();

    if (fn_8007E8B4(player, target, (float)framesToSeconds(250.0), 10.0f)) {
        path.erase(path.begin());
        initialPosition = &Player::fn_1_35F4;
        callback = initialPosition;
    }
}

void Player::fn_1_3FE0()
{
    void (Player::*next)();

    fn_8007EEBC(player, 1);
    next = &Player::fn_1_404C;
    callback = next;
}

void Player::fn_1_404C()
{
    void (Player::*initialPosition)();
    Vec *target = path.begin();

    if (fn_8007E8B4(player, target, (float)framesToSeconds(1000.0), 20.0f) != 0) {
        path.erase(path.begin());
        fn_80080DD4(player->actor, 2);
        initialPosition = &Player::fn_1_35F4;
        callback = initialPosition;
    }
}

void Player::fn_1_411C()
{
    void (Player::*values)();

    fn_80087168(player, 1);
    mode->enter();
    values = &Player::fn_1_41A8;
    callback = values;
}

void Player::fn_1_41A8()
{
    if (attributeEnabled != 0) {
        fn_1_4654();
        return;
    }
    mode->update();
    if (fn_80087114(player, 0x200) != 0) {
        sendEvent(number() + 1, 1, &position, 0.0f);
    }
}

struct DisplayState {
    unsigned char unknown00[0x34]; /* No code names this field. */
    int progress;
};

void Player::fn_1_4250()
{
    DisplayState *sequence = (DisplayState *)display;
    int previousProgress = sequence->progress;
    int comparisonProgress = previousProgress;

    if (comparisonProgress < 3 && static_cast<DisplayProviderDP *>(display)->fn_1_66F4() >= 3) {
        transitionStarted = 1;
        lbl_1_bss_8C += 1;
        {
            int callbackId = playerNo;
            MinigameCore::addWinner(callbackId);
        }
    }
}

void Player::fn_1_42DC()
{
    void (Player::*expected)() = &Player::fn_1_41A8;
    void (Player::*resetPosition)();

    if (callback == expected) {
        inactive = 1;
        fn_80086E3C(player);
        fn_8007EDBC(player);
        selectCharacterMotion(&static_cast<MotionOwner &>(*this), 0x93001E);
        resetPosition = &Player::fn_1_43A0;
        callback = resetPosition;
    }
}

void Player::fn_1_43A0()
{
    void (Player::*state)();

    fn_1_1C5D8(&playerNo, 2, (int)secondsToFrames(0.8));
    state = &Player::fn_1_4428;
    callback = state;
}

void Player::fn_1_4428()
{
    void (Player::*values)();

    if (characterMotionEnded(&static_cast<MotionOwner &>(*this).charNo) != 0) {
        inactive = 0;
        fn_80086F00(player, &position);
        fn_8007ED60(player);
        fn_8008718C(player, 2, 180.0f + rotationY, (int)secondsToFrames(0.5));
        values = &Player::fn_1_44FC;
        callback = values;
    }
}

struct PlayerModelMotion {
    unsigned char unknown00[0x1C]; /* No code names this field. */
    short motionId;
};

struct PlayerMotionResource {
    unsigned char unknown00[0x48]; /* No code names this field. */
    PlayerModelMotion *model;
};

void Player::fn_1_44FC()
{
    switch (player->subMode) {
    case 1:
        fn_8005C2C8(player->charNo, ((PlayerMotionResource *)player->omObj)->model->motionId,
                    0.0f, 5.0f, ((1 << 30) | (1 << 0)));
        player->motNo = 0xE;
        player->timer = (float)secondsToFrames(1.5);
        player->subMode = 2;
        return;
    default:
        mode->enter();
        void (Player::*callbackData)() = &Player::fn_1_41A8;
        callback = callbackData;
        /* fall through */
    case 2:
        return;
    }
}

/* Marks the player as active for the setup step that invokes this helper. */
void Player::fn_1_4610()
{
    fn_80087158(player, (1 << 0));
    attributeEnabled = 1;
}

/* Enables common stick handling and restores the configured player mode data. */
void Player::fn_1_4654()
{
    void (Player::*copy)();

    fn_8007E818(player);
    copy = &Player::fn_1_46BC;
    callback = copy;
}

void Player::fn_1_46BC()
{
    Player *selected;

    if (started != 0 && reachedPlace != 0) {
        fn_8007EEBC(player, 1);
    }
    Vec *target = path.begin();
    if (fn_8007E8B4(player, target, (float)framesToSeconds(400.0), 10.0f) != 0 && fn_1_4B68(this) != 0) {
        if (reachedPlace != 0) {
            int index = 0;

            while (index < 2) {
                selected = sm_players[index];
                if (selected->reachedPlace == 0) {
                    selected->reachedPlace = 1;
                    break;
                }
                index++;
            }
            reachedPlace = 0;
        }
        if (transitionStarted != 0) {
            sendEvent(0, 4, &position, 0.0f);
        }
        void (Player::*next)() = &Player::fn_1_4858;
        callback = next;
    }
}

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
    fn_80027CD0(1, &cameraPosition, &cameraTarget, &cameraTarget);
    actorPosition = &player->actor->pos;
    targetAngle = (float)(180.0 * (atan2((double)(cameraPosition.x - actorPosition->x),
                                         (double)(cameraPosition.z - actorPosition->z)) /
                                   M_PI));
    fn_80086CB0(player->actor, &rotation);
    Range<float> range(0.0f, 360.0f, rotation);
    void (Player::*defaults)();
    fn_80086CA8(player->actor, range.fn_1_4E0C(targetAngle, 0.1));
    difference = range.fn_1_4DD0(targetAngle);
    absoluteDifference = __fabsf(difference);
    error = absoluteDifference;
    if (error < 1.0) {
        range.fn_1_4D94(targetAngle);
        fn_80087158(player, (1 << 2));
        defaults = &Player::fn_1_35F4;
        callback = defaults;
    }
}

void Player::fn_1_4A0C()
{
    unsigned int data;
    void (Player::*state)();

    if (transitionStarted != 0) {
        data = 0x930006;
    } else {
        data = 0x930028;
    }

    shiftCharacterMotion(&static_cast<MotionOwner &>(*this), data, (float)secondsToFrames(0.3), 0.0f, 0);

    state = &Player::fn_1_35F4;
    callback = state;
}

void Player::fn_1_4ACC()
{
    if (inactive == 0) {
        Vec originalPosition = position;
        fn_80086BC8(player->actor, &position);
        PSVECSubtract(&position, &originalPosition, &velocity);
        fn_80086CB0(player->actor, &rotationY);
        fn_1_BA00(1, (short)fn_1_1C558(&playerNo), &position);
    }
}

unsigned char fn_1_4B68(Player *object)
{
    u32 collisionCode = 0;
    fn_80086DD0(object->player->actor, &collisionCode);
    return (unsigned char)((collisionCode & 0x407F) != 0);
}

/* Updates the character's movement attribute and voice flag for the active state. */
void fn_1_4BC0(void *state, bool enabled)
{
    int character = fn_1_1C558((s32 *)state);

    if (enabled) {
        fn_8005C47C(character, 1);
    } else {
        fn_8005C430(character, 1);
    }
    fn_8005DFC0(character, enabled);
}

void fn_1_4C34(Player *owner, unsigned char enabled)
{
    int modelId = (short)owner->player->actor->mdlId;
    if (enabled) {
        fn_80026E44((short)modelId);
    } else {
        fn_80026EF0((short)modelId);
    }
}

struct PlayerFilter {
    int unknown00; /* the game's code never reads this field */
    int playerId;
    int flags;
};

int fn_1_4C98(const PlayerFilter *filter, const PlayerFilter *other)
{
    int index;
    Player *match;

    if (filter->flags != 0 || other->flags != 0) {
        return 0;
    }
    (sm_players.size() == entry_num) ? (void)0
                               : __msl_assertion_failed("sm_players.size() == entry_num", "player.cpp", 0x218);
    match = 0;
    index = 0;
    while (index < 2) {
        Player *entry = sm_players[index];
        if (filter->playerId == entry->player->actor->param) {
            match = entry;
            break;
        }
        index++;
    }
    if (match == 0) {
        return 0;
    }
    match->started = 1;
    return 1;
}

template<class Time> Time framesToSeconds(Time value)
{
    return (1.0 / 60.0) * value;
}

// Same-game framework time conversion.
template<class Time> Time secondsToFrames(Time seconds) { return 60 * seconds; }
