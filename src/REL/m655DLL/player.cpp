#include "REL/m655DLL/game_constants.h"
#include "REL/framework/frame_durations.h"
#include "REL/m655DLL/screen_sprite_api.h"
#include "REL/m655DLL/rope_api.h"
#include "REL/m655DLL/player_list.h"
#include "REL/framework/random.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/sound.h"
#include "REL/framework/player_table_1vs1.h"
#include "REL/framework/range.h"
#include "REL/m655DLL/icon_renderer.h"
#include <new>
#include "REL/m655DLL/player_motion_interface.h"
#include "REL/m655DLL/screen_sprite_singleton.h"
#include "REL/framework/p_info.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/singleton.h"
#include "REL/m655DLL/goal.h"
#include "REL/framework/vector3.h"
#include "humath.h"
#include <math.h>
#include "REL/framework/p_chara.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_packet.h"
#include "dolphin/os.h"
#include <vector>
#include "REL/framework/minigame_core.h"
#include "REL/framework/model.h"
#include "REL/framework/frame_interpolator.h"
#include "datanum/charmot.h"
#include "datanum/mario.h"
#include "game/hu3d.h"
#include "game/charman.h"
#include "datadir_enum.h"
#include "REL/framework/chara_motion.h"
#include "REL/framework/category_sphere.h"
#include "REL/m655DLL/player_state.h"
#include "REL/framework/world_units.h"
enum { entry_num = 2 };
extern "C" const unsigned int lbl_1_rodata_458[9] = {0x00930000, 0x00930001, 0x0093003C,
                                                     0x008E009C, 0x0093003F, 0x00930004,
                                                     0x00930022, 0x00930006, 0x00930028};
/* Invoke the attached interface, then update the actor's motion state. */
/* Starting positions, written by the unit's static initialiser from the course constants. */
/* The two players start at different horizontal positions.
 * Their heights and depths are shared.
 * The three destination positions use the same height.
 * Destination depths are closer to the camera.
 * Coordinates below are expressed in world units.
 */
extern "C" {

PlayerStartPosition lbl_1_data_458[entry_num] = {
    {{cmToWorld(-150.0f), cmToWorld(405.0f), cmToWorld(-750.0f)}},
    {{cmToWorld(90.0f), cmToWorld(405.0f), cmToWorld(-750.0f)}},
};
Vec lbl_1_data_470[3] = {
    {cmToWorld(-250.0f), cmToWorld(405.0f), cmToWorld(-650.0f)},
    {cmToWorld(0.0f), cmToWorld(405.0f), cmToWorld(-650.0f)},
    {cmToWorld(250.0f), cmToWorld(405.0f), cmToWorld(-650.0f)},
};
std::vector<PlayerWork *> sm_players;
s32 lbl_1_bss_CC;
s32 lbl_1_bss_D0;
/* Individually accessed bytes at .bss 0xD4..0xE0. */
u8 lbl_1_bss_D4[0xC];
}
void fn_1_7FC4(PlayerWork *, PlayerWork *);
void fn_1_8684(PlayerWork *);
void fn_1_A070(PlayerWork *, u8, f32);
void fn_1_A71C(PlayerWork *player);
static inline void setPlayerState(PlayerWork *player, PlayerWork::State next)
{ player->state = next; }
void fn_1_7798(void) {
    s32 modes[entry_num];
    s32 index;
    PlayerWork *player;
    const PlayerStartPosition *templateData;
    templateData = lbl_1_data_458;
    makePlayerTable1vs1(modes);
    sm_players.clear();
    for (index = 0; index < entry_num; ++index) {
        player = new PlayerWork(modes[index], index, templateData++);
        sm_players.push_back(player);
    }
    fn_8005DDEC(3);
    lbl_1_bss_CC = 0;
    lbl_1_bss_D0 = 0;
    lbl_1_bss_D4[0] = 1;
    lbl_1_bss_D4[1] = 0;
    lbl_1_bss_D4[2] = 1;
}
void fn_1_78D4(void) {
    s32 playerIndex;

/* Retain assertion source location.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
    assert(sm_players.size() == entry_num);
    playerIndex = 0;
    while (playerIndex < 2) {
        PlayerWork *player = sm_players[playerIndex];
        player->resource.reset(new IconRenderer(player->mode, player->characterNo()));
        player->resource->fn_1_59C4(player->resourceNumber);
        player->resource->fn_1_5A88();
        playerIndex += 1;
    }
}
void fn_1_79E8(void) {

    assert(sm_players.size() == entry_num);
    for (s32 index = 0; index < 2; ++index) {
        PlayerWork *player = sm_players[index];
        player->currentMotion()->setPosition();
        setPlayerState(player, &PlayerWork::fn_1_8C44);
    }
}
s32 fn_1_7AC4(void) {
    PlayerWork::State firstState, secondState, thirdState;

/* Retain assertion source location. */
    assert(sm_players.size() == entry_num);
    for (s32 playerIndex = 0; playerIndex < 2; ++playerIndex) {
        PlayerWork *player = sm_players[playerIndex];
        firstState = &PlayerWork::fn_1_8B64;
        if (player->state != firstState) {
            secondState = &PlayerWork::fn_1_8C44;
            if (player->state != secondState) {
                thirdState = &PlayerWork::fn_1_9344;
                if (player->state != thirdState) {
                    return 0;
                }
            }
        }
    }
    return 1;
}
void fn_1_7BF8(void) {
    assert(sm_players.size() == entry_num);
    s32 playerIndex;
    playerIndex = 0;

    while (playerIndex < 2) {

        fn_1_A71C(sm_players[playerIndex]);

        playerIndex += 1;
    }
}

s32 fn_1_7C98(void) {

    return lbl_1_bss_D0;
}

s32 fn_1_7CA8(void) {

    return lbl_1_bss_CC;
}

s32 fn_1_7CB8(void) {
    s32 playerIndex;
    PlayerWork *player;
    if (lbl_1_bss_D0 <= 0)
        return -1;
    if (lbl_1_bss_CC > 0)
        return -1;

    assert(sm_players.size() == entry_num);
    std::vector<int> candidates;
    playerIndex = 0;
    while (playerIndex < 2) {
        player = sm_players[playerIndex];
        if ((u8)player->flags.unknownFlag0 != 0) {
            candidates.push_back(player->mode);
        }
        ++playerIndex;
    }
    return candidates[RandomUnsigned() % candidates.size()];
}
PlayerWork *fn_1_7E1C(s32 setupMode) {
    int index;
    PlayerWork *player;
    assert(sm_players.size() == entry_num);
    for (index = 0; index < entry_num; ++index) {

        player = sm_players[index];
        if (player->mode == setupMode) {
            return player;
        }
    }
    assert(!"No player found.\n");
    return 0;
}
s32 fn_1_7EF4(PlayerList *list) {
    PlayerWork *player;
    s32 index;

    assert(sm_players.size() == entry_num);
    list->clear();
    index = 0;
    while (index < 2) {
        player = sm_players[index];
        Sphere *entry = &player->position;
        list->push_back(entry);
        index = 1 + index;
    }
    return list->size();
}

u8 characterMotionEnded(s32 *);

void sendEvent(int channel, unsigned long value, float delay);

void fn_1_A7D4(PlayerWork *object);

inline PlayerInputInterface::PlayerInputInterface(PlayerWork *object) {
    owner = object;
}

inline HumanPlayerInput::HumanPlayerInput(PlayerWork *object) : PlayerInputInterface(object) {}

static inline f64 minimumMovement(f64 maximum, f64 distance) {
    return maximum < distance ? maximum : distance;
}

static inline f32 directionDistance(const Vec *target, const Vec *position, Vec &direction) {
    PSVECSubtract(target, position, &direction);
    return PSVECMag(&direction);
}

struct MovementVector : Vec {
    MovementVector &scale(f32 factor) {
        PSVECScale(this, this, factor);
        return *this;
    }
    void addTo(Vec &position) const {
        PSVECAdd(this, &position, &position);
    }
};

void fn_1_937C(PlayerWork *object);

struct TargetPosition {
    Vec value;
};
extern "C" const TargetPosition lbl_1_rodata_540;

Vec *fn_1_A004(PlayerWork *object, Vec *position);

u8 fn_1_A320(PlayerWork *object, Vec *position);

static inline void shiftPlayerMotion(PlayerWork *work, int motion, u32 attributes) {
    shiftCharacterMotion(&static_cast<Character &>(*work), motion, (f32)secondsToFrames(0.3), 0.0f,
                         attributes);
}

static inline f64 interpolateValue(f32 start, f32 end, f64 weight) {
    return (1.0 - weight) * start + weight * end;
}

static inline f32 interpolateHeight(f32 start, f64 end, f64 weight) {
    return (f32)((1.0 - weight) * start + weight * end);
}

/* The initial rise uses the first fifth of the curve.
 * The remaining four fifths bring the height to its maximum.
 */

static inline f64 evaluateHeightCurve(f32 blend) {
    if ((f64)blend < 0.2) {
        return interpolateValue(0.0, 0.5, (f64)blend / 0.2);
    } else {
        return interpolateValue(0.5, 1.0, ((f64)blend - 0.2) / 0.8);
    }
}
extern "C" {
char *fn_8005C0F0(s16 character, s16 item, s16 variant);
}

static inline f32 angleFromDirection(f32 y, f32 x) {
    return (f32)atan2((f64)y, (f64)x);
}

static inline void translateFor(Mtx matrix, f32 x, f32 y, f32 z) {
    PSMTXTrans(matrix, x, y, z);
}

static inline void translateApplyFor(Mtx source, Mtx result, f32 x, f32 y, f32 z) {
    PSMTXTransApply(source, result, x, y, z);
}

static inline Vector3 &scaleFor(Vector3 &vector, f32 scale) {
    PSVECScale(&vector, &vector, scale);
    return vector;
}

inline Vec &operator+=(Vec &value, const Vec &other) {
    PSVECAdd(&other, &value, &value);
    return value;
}

void fn_1_7FC4(PlayerWork *left, PlayerWork *right) {
    Sphere *leftTransition;
    Sphere *rightTransition;

    leftTransition = &left->position;
    rightTransition = &right->position;
    if ((u8)resolveSphereContact(leftTransition,
                                 rightTransition, 0.5f, 1.0f)) {
        float amount = (float)framesToSeconds(100.0);
        if (leftTransition->center.z < rightTransition->center.z) {
            leftTransition->center.z -= amount;
        } else {
            rightTransition->center.z -= amount;
        }
    }
}

/* Release debug hook; its argument is evaluated. */
static inline void DebugValue(const PlayerWork::State &value) {}
PlayerWork::PlayerWork(s32 player, s32 mode, const PlayerStartPosition *position)
    : PlayerConfigSelection(player, 1), GameObject(0), EventListener(),
      state(&PlayerWork::fn_1_8B64), mode(mode), scale((f32)(0.8 * height())),
      position(position->value, (f32)(0.3 * height())), firstRotation(0.0f, 360.0f, 0.0f),
      rotation(0.0f, 360.0f, 0.0f), transition(0.0f), resourceNumber(mode == 0 ? 0 : 2),
      resource(0), firstRegisteredValue(0), secondRegisteredValue(1),
      consumer(isComputer() ? (PlayerInputInterface *)new ComputerPlayerInput(this)
                            : new HumanPlayerInput(this)) {
    this->setLayer(3);
    this->setShadow();
    EventListener::registerChannel(0);
    this->offset.x = 0.0f;
    this->offset.y = 0.0f;
    this->offset.z = 0.0f;
    fn_1_8684(this);
    fn_1_A7D4(this);
    selectCharacterMotion(&static_cast<Character &>(*this), CHARMOT_HSF_c000m1_300);
    this->firstRegisteredValue.setRadius((f32)(0.5 * height()));
    this->secondRegisteredValue.setRadius(this->scale);
    DebugValue(&PlayerWork::fn_1_8C44);
    DebugValue(&PlayerWork::fn_1_8B64);
}

PlayerWork::~PlayerWork() {}

void PlayerWork::update() {
    (this->*this->state)();
    fn_1_A8AC(&sm_players, fn_1_7FC4);
    fn_1_8684(this);
}

void PlayerWork::receive(const EventPacket *event) {
    switch (event->type) {
    case 2:
        fn_1_8CA4(this);
        return;
    case 3:
        fn_1_937C(this);
    }
}

void fn_1_8684(PlayerWork *self) {
    Vec range = {0.0f, 0.0f, 0.0f};
    f32 targetDepth;
    f32 currentDepth;
    f32 positionDepth;
    f32 transition;
    f32 blend;
    f32 heightCurve;
    f32 curveInput;
    range.x = self->rotation.get();
    range.y = self->firstRotation.get();
    self->setPosition(&self->position.center);
    self->setRotation(&range);
    soundPanCharacterVoice(1, (s16)self->characterNo(), &self->position.center);
    targetDepth = lbl_1_data_470[self->resourceNumber].z;
    currentDepth = Goal::getChecked().fn_1_3470()->z;
    positionDepth = self->position.center.z;
    if ((positionDepth - targetDepth) / (currentDepth - targetDepth) < 0.0f) {
        blend = 0.0f;
    } else {
        if ((positionDepth - targetDepth) / (currentDepth - targetDepth) > 1.0f) {
            blend = 1.0f;
        } else {
            blend = (positionDepth - targetDepth) / (currentDepth - targetDepth);
        }
    }
    curveInput = blend;
    heightCurve = (f32)evaluateHeightCurve(curveInput);
    transition = (f32)((1.0 - cos(M_PI * (f64)(2.0f * heightCurve))) / 2.0);
    f32 target = interpolateHeight(self->scale, 839.99994f, transition);
    self->secondRegisteredValue.setRadius(
        weightedHeight(self->secondRegisteredValue.getRadius(), target));
    fn_1_884(&MinigameSingleton<ScreenSpriteWork>::getChecked(), transition);
    self->firstRegisteredValue = Vec(self->position.center);
    self->secondRegisteredValue = Vec(self->position.center);
    self->firstRegisteredValue.center.y += self->firstRegisteredValue.getRadius();
    self->secondRegisteredValue.center.y += self->secondRegisteredValue.getRadius();
}
void PlayerWork::fn_1_8B64() {}

void fn_1_8B68(PlayerWork *owner, s32 direction) {
    if (direction != 0) {
        s32 step = direction == 0 ? 0 : (direction < 0 ? -1 : 1);
        s32 candidate = owner->resourceNumber + step;

        while (candidate >= 0 && candidate < 3) {
            if (lbl_1_bss_D4[candidate] == 0) {
                lbl_1_bss_D4[owner->resourceNumber] = 0;
                lbl_1_bss_D4[candidate] = 1;
                owner->resourceNumber = candidate;
                soundPlay(0x81E);
                return;
            }
            candidate += step;
        }
    }
}

void PlayerWork::fn_1_8C44() {
    consumer->invoke();
    resource->fn_1_59C4(resourceNumber);
}

void fn_1_8CA4(PlayerWork *object) {

    if (!object->flags.unknownFlag0) {
        object->flags.unknownFlag0 = 1;
        shiftCharacterMotion(&static_cast<Character &>(*object), CHARMOT_HSF_c000m1_301,
                             (f32)secondsToFrames(0.1), 0.0f, ((1 << 30) | (1 << 0)));
        object->resource->fn_1_5ADC();
        soundPlay(0x81F);
        setPlayerState(object, &PlayerWork::fn_1_8D7C);
    }
}

void PlayerWork::fn_1_8D7C(void) {
    const Vec *target = &lbl_1_data_470[resourceNumber];
    MovementVector direction;
    f32 distance;
    f64 maximumDistance;
    f64 limitedDistance;
    f64 arrivalDistance;
    f64 remainingDistance;
    distance = directionDistance(target, &position.center, direction);
    maximumDistance = framesToSeconds(250.0);
    limitedDistance = minimumMovement(maximumDistance, distance);
    if (limitedDistance > 0.0) {
        firstRotation.step((f32)(180.0 * ((f64)atan2f(direction.x, direction.z) / M_PI)), 0.1);
        direction.scale((f32)(limitedDistance / (f64)distance)).addTo(position.center);
        arrivalDistance = 28.0 * maximumDistance;
        remainingDistance = sqrt((f64)PSVECSquareDistance(target, &position.center));
        if (remainingDistance <= arrivalDistance) {
            sendEvent(resourceNumber + 1, 0, 0.0f);
        }
        return;
    }
    if (++lbl_1_bss_D0 >= 2) {
        sendEvent(0, 1, 0.0f);
    }
    transition.set(0.0f, 1.0f, (s32)secondsToFrames(0.4));
    shiftCharacterMotion(&static_cast<Character &>(*this), CHARMOT_HSF_c000m1_372,
                         (f32)secondsToFrames(0.4), 0.0f, ((1 << 30) | (1 << 0)));
    soundPlayAt(1, 0x820, &position.center);
    setPlayerState(this, &PlayerWork::fn_1_9210);
}

void PlayerWork::fn_1_9210() {

    fn_1_A070(this, 1, this->transition.update());
    this->firstRotation.step(0.0f, 0.2);
    if (fabs(this->firstRotation.distanceTo(0.0f)) > 1.0 ||
        (u8)characterMotionShiftInactive(&charNo) == 0) {
        return;
    }

    this->firstRotation.set(0.0f);
    if (this->transition.isFinished()) {
        setPlayerState(this, &PlayerWork::fn_1_9344);
    }
}

void PlayerWork::fn_1_9344() {
    fn_1_A070(this, 1, 1.0f);
}

static inline void updateState(PlayerWork *object) {
    setPlayerState(object, &PlayerWork::fn_1_9428);
    shiftCharacterMotion(&static_cast<Character &>(*object), MARIO_HSF_c000m1_620,
                         (f32)secondsToFrames(0.3), 0.0f, ((1 << 30) | (1 << 0)));
}

void fn_1_937C(PlayerWork *object) {
    if (object->flags.unknownFlag0) {
        updateState(object);
    }
}

void PlayerWork::fn_1_9428() {
    fn_1_A070(this, 1, 1.0f);
    if (static_cast<Character &>(*this).fn_1_DB20() < 9.0f) {
        return;
    }
    f64 secondValue;
    f64 firstValue;

    firstValue = (f64)framesToSeconds(0xC8);
    secondValue = (f64)framesToSeconds(0x1F4);

    this->offset.x = 0.0f;
    this->offset.y = (f32)secondValue;
    this->offset.z = (f32)firstValue;
    setPlayerState(this, &PlayerWork::fn_1_9554);
}

extern "C" const TargetPosition lbl_1_rodata_540 = {{0.0f, 0.0f, 0.0f}};

void PlayerWork::fn_1_9554(void) {
    bool isCurrentPlayer2;
    Vec previousPosition = position.center;
    Vec statusPosition;
    Vec movement;
    TargetPosition targetPosition;
    f32 lowerLimit;
    PlayerWork::State next;
    u8 isCurrentPlayer;
    PSVECAdd(&this->offset, &position.center, &position.center);
    position.center.y -= gravity;
    this->offset.y -= gravity;
    fn_1_A070(this, 0, 1.0f);
    f32 heightLimit = -450.0f;
    if (previousPosition.z < heightLimit) {
        if (position.center.z >= heightLimit) {
            soundPlayAt(1, 0x821, &position.center);
        }
    }
    isCurrentPlayer2 = resourceNumber == static_cast<Goal &>(Goal::getChecked())[0];
    if (isCurrentPlayer2) {
        Goal::getChecked().fn_1_3430(position.center.x);
    }
    lowerLimit = (f32)((f64)Goal::getChecked().fn_1_3470()->z - 60.0);
    if (!fn_1_A320(this, &statusPosition)) {
        return;
    }
    if (statusPosition.z < lowerLimit) {
        return;
    }
    {
        shiftCharacterMotion(&static_cast<Character &>(*this), CHARMOT_HSF_c000m1_304,
                             (f32)secondsToFrames(0.3), 0.0f, ((1 << 30) | (1 << 4)));
        targetPosition = lbl_1_rodata_540;
        targetPosition.value.z = (f32)framesToSeconds(1600.0);
        fn_1_A004(this, &movement);
        PSVECAdd(&targetPosition.value, &movement, &movement);
        fn_1_B8E0((Rope *)fn_1_B140(resourceNumber), &movement, &targetPosition.value, 1.0f);
        next = isCurrentPlayer2 ? &PlayerWork::fn_1_988C : &PlayerWork::fn_1_9B40;
        state = next;
    }
}

void PlayerWork::fn_1_988C(void) {
    f32 height;

    PSVECAdd(&this->offset, &this->position.center, &this->position.center);
    this->offset.y -= gravity;
    this->rotation.step(0.0f, 0.05);

    Goal::getChecked().fn_1_3430(this->position.center.x);

    height = Goal::getChecked().fn_1_3470()->y;
    if (this->position.center.y > height) {
        return;
    }

    this->position.center.y = height;
    this->offset.x = 0.0f;
    this->offset.y = 0.0f;
    this->offset.z = 0.0f;
    this->rotation.set(0.0f);
    this->flags.atLimit = 1;
    lbl_1_bss_CC += 1;
    MinigameCore::addWinner(number());
    this->resetAttribute(((1 << 30) | (1 << 1)));
    setPlayerState(this, &PlayerWork::fn_1_9A80);
}

void PlayerWork::fn_1_9A80(void) {
    if (!(u8)characterMotionShiftInactive(&charNo) || !characterMotionEnded(&charNo)) {
        return;
    }
    shiftCharacterMotion(&static_cast<Character &>(*this), CHARMOT_HSF_c000m1_300,
                         (f32)secondsToFrames(0.3), 0.0f, ((1 << 30) | (1 << 0)));
    setPlayerState(this, &PlayerWork::fn_1_8B64);
}

/* Updates the moving object's position and starts its finish state at the limit. */
void PlayerWork::fn_1_9B40() {
    f32 previousHeight;
    f32 contextHeight;
    f32 upperLimit;
    f32 lowerLimit;
    f32 floorHeight;

    previousHeight = this->position.center.y;
    PSVECAdd(&this->offset, &this->position.center, &this->position.center);
    this->offset.y -= gravity;
    this->rotation.step(0.0f, 0.05);

    contextHeight = Goal::getChecked().fn_1_3470()->y;
    upperLimit = contextHeight + cmToWorld(0.0f);
    lowerLimit = contextHeight - 500.0f;
    floorHeight = -1000.0f;

    if (this->position.center.y < upperLimit) {
        shiftPlayerMotion(this, CHARMOT_HSF_c000m1_344, (1 << 30) | (1 << 0));
        if (previousHeight >= upperLimit) {
            soundPlayAt(1, 0x822, &this->position.center);
        }
    }
    if (this->position.center.y < lowerLimit) {
        this->firstRegisteredValue.clearFlag7();
        this->secondRegisteredValue.clearFlag7();
    }
    if (!(this->position.center.y > floorHeight)) {
        this->position.center.y = floorHeight;
        this->offset.x = cmToWorld(0.0f);
        this->offset.y = cmToWorld(0.0f);
        this->offset.z = cmToWorld(0.0f);
        this->setAttribute(1);
        fn_8005DFC0((s16)characterNo(), 0);
        rumble(1, (s32)secondsToFrames(0.4));
        setPlayerState(this, &PlayerWork::fn_1_8B64);
    }
}

void fn_1_9DC8(PlayerWork *object) {
    object->firstRegisteredValue.setFlag7();
    object->secondRegisteredValue.setFlag7();
    selectCharacterMotion(&static_cast<Character &>(*object), DATANUM(0x930000, 63));
    object->setAttribute((1 << 30) | (1 << 0));
    object->resetAttribute(1);
    fn_8005DFC0((s16)object->characterNo(), 1);
}

void fn_1_9E40(PlayerWork *object, Vec *offset) {
    Vec position;
    char *item;
    Vec *positionAddress;
    Vec *resultAddress;

    item = fn_8005C0F0(object->characterNo(), 2, 4);
    object->getObjectPosition(item, &position);
    PSVECSubtract(&object->position.center, &position, &position);
    resultAddress = &position;
    positionAddress = resultAddress;
    PSVECAdd(offset, positionAddress, &object->position.center);
}

void fn_1_9ED8(PlayerWork *self) {
    self->resourceNumber = Goal::getChecked()[0];
    self->offset.x = 0.0f;
    self->offset.y = 0.0f;
    self->offset.z = 0.0f;
    shiftPlayerMotion(self, CHARMOT_HSF_c000m1_304, (1 << 30) | (1 << 4));
    setPlayerState(self, &PlayerWork::fn_1_988C);
}

inline Vec *fn_1_A004(PlayerWork *work, Vec *modelPosition) {
    char *item = fn_8005C0F0((s16)work->characterNo(), 2, 0);
    work->getObjectPosition(item, modelPosition);
    return modelPosition;
}

void fn_1_A070(PlayerWork *work, u8 mode, f32 amount) {
    Mtx translation;
    Mtx rotation;
    Vec modelPosition;
    Vec targetPosition;
    Vector3 direction;
    f32 angle;
    f32 rotationAmount;
    GameObject *model2;
    model2 = fn_1_B140(work->resourceNumber);
    if ((u8)fn_1_B8E0((Rope *)model2, fn_1_A004(work, &modelPosition), &targetPosition, amount) &&
        !mode) {
        PSVECSubtract(&targetPosition, &modelPosition, &direction);
        angle = (f32)(180.0 * ((f64)angleFromDirection(direction.z, direction.y) / M_PI));
        rotationAmount = (f32)(0.2 * work->rotation.distanceTo(angle));
        work->rotation.add(rotationAmount);
        PSVECAdd(&direction, &work->position.center, &work->position.center);
        work->offset += scaleFor(direction, 0.5f);
        translateFor(translation, -targetPosition.x, -targetPosition.y, -targetPosition.z);
        PSMTXRotRad(rotation, 'x', MTXDegToRad(-rotationAmount));
        PSMTXConcat(rotation, translation, translation);
        translateApplyFor(translation, translation, targetPosition.x, targetPosition.y,
                          targetPosition.z);
        PSMTXMultVec(translation, &work->position.center, &work->position.center);
    }
}

u8 fn_1_A320(PlayerWork *work, Vec *position) {
    f32 heightDifference;
    f64 endX;
    f32 interpolation;
    f64 endHeight;
    f64 endZ;
    heightDifference = work->position.center.y - Goal::getChecked().fn_1_3470()->y;
    if (heightDifference < 0.0f) {
        return 0;
    }
    interpolation =
        (f32)((f64)(work->offset.y / gravity) + sqrt((2.0 * (f64)heightDifference) / (f64)gravity));
    endZ = work->position.center.z + interpolation * work->offset.z;
    endHeight = Goal::getChecked().fn_1_3470()->y;
    endX = work->position.center.x + interpolation * work->offset.x;
    position->x = endX;
    position->y = endHeight;
    position->z = endZ;
    return 1;
}

/* Update the actor's alternate joint motion and resource action. */

void shiftCharacterMotion(MotionOwner *character, unsigned int dataNum, float start, float blend,
                          unsigned long attributes);

/* Release debug hook; its argument is evaluated. */
static inline void DebugValue(bool value) {}

void fn_1_A71C(PlayerWork *actor) {
    if (actor->flags.atLimit) {
        shiftCharacterMotion(&static_cast<Character &>(*actor), CHARMOT_HSF_c000m1_306, 16.0f, 0.0f,
                             0);
    } else {
        DebugValue(!actor->flags.unknownFlag0);
    }
    setPlayerState(actor, &PlayerWork::fn_1_8B64);
}

void createCharacterMotions(MotionOwner *, const unsigned int *, s32);
void setCharacterMotionAttributes(MotionOwner *, unsigned int, u16);

void fn_1_A7D4(PlayerWork *object) {
    createCharacterMotions(&static_cast<Character &>(*object), lbl_1_rodata_458, 9);
    setCharacterMotionAttributes(&static_cast<Character &>(*object), 0x8E009C, 1);
}
