#include "REL/m652DLL/map_singleton.h"
#include "REL/framework/sound.h"
#include "REL/framework/player_table_1vs1.h"
#include "REL/framework/singleton.h"
#include "REL/m652DLL/player_functions.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include "REL/framework/minigame_core.h"
#include <vector>
#include "REL/m652DLL/range_operations.h"
#include "dolphin/types.h"
extern "C" {
#include "humath.h"
#include "game/charman.h"
}
#include "REL/m652DLL/player_work.h"
#include <memory>
#include "REL/m652DLL/tail_types.h"
#include "game/hsfex.h"
extern "C" {
#include "game/mg/actman.h"
}
#include "REL/m652DLL/model_child.h"
#include "REL/framework/chara_motion.h"
#include "REL/framework/p_info.h"
#include <math.h>
#include "dolphin/mtx.h"
#include "game/hu3d.h"
#include "REL/m652DLL/framework/range.h"
extern "C" {
#include "dolphin/os.h"
}
#include "REL/m652DLL/racer_control.h"
#include "datanum/charmot.h"
#include "REL/m652DLL/player_state.h"
void sendEvent(int, unsigned long, float);
extern "C" {
// Each target object is one float, read at offset zero.
extern const unsigned int lbl_1_rodata_320[4] = {0x930022, 0x93003F, 0x930006, 0x930028};
const float lbl_1_rodata_330[1] = {0.0f};
const float lbl_1_rodata_334[1] = {405.0f};
const float lbl_1_rodata_338[1] = {100.0f};
}
/* The players of the two sides. */

struct Racer;

#include "REL/m652DLL/player_algorithms.h"

extern "C" {

int lbl_1_bss_88 = minutesToFrames(5L);
int lbl_1_bss_8C = secondsToFrames<long>(30);
float lbl_1_bss_90 = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
int lbl_1_bss_94 = secondsToFrames<long>(2);
int lbl_1_bss_98 = Frames07EF8(3.5f);
int lbl_1_bss_9C = Frames07EF8(3.5f);
static const int entry_num = 2;
std::vector<Racer *> sm_players;
int lbl_1_bss_B8;
}

extern "C" {
int fn_1_56A8(int value);
void fn_1_56D0(void);
void fn_1_596C(void);
void fn_1_59BC(void);
void fn_1_5A0C(void);
s32 fn_1_5A5C(void);
}

long RandomInteger(long limit);

int fn_1_56A8(int value)
{
    return RandomInteger(value);
}

void fn_1_56D0(void)
{
    static Point3d startingPosition = {lbl_1_rodata_330[0], lbl_1_rodata_334[0], lbl_1_rodata_338[0]};
    s32 players[entry_num];
    makePlayerTable1vs1(players);
    std::vector<int> indices(2);
    indices.clear();
    int insertionIndex;
    int index;
    for (insertionIndex = 0; insertionIndex < 2; ++insertionIndex)
        indices.push_back(insertionIndex);
    ShuffleRange56D0(indices.begin(), indices.end());
    sm_players.clear();
    for (index = 0; index < 2; ++index) {
        int &selectedIndex = indices.at(index);
        s32 *player = players + selectedIndex;
        Racer *object = new Racer(*player, selectedIndex, &startingPosition);
        sm_players.push_back(object);
    }
    fn_8005DDEC(2);
    lbl_1_bss_B8 = 0;
}

void fn_1_596C(void)
{
    Racer::fn_1_5B3C(&Racer::fn_1_6134);
}

void fn_1_59BC(void)
{
    Racer::fn_1_5B3C(&Racer::fn_1_68B4);
}

void fn_1_5A0C(void)
{
    Racer::fn_1_5B3C(&Racer::fn_1_6C30);
}

s32 fn_1_5A5C(void)
{
    s32 index;
    Racer *player;
    assert(sm_players.size() == entry_num);
    index = 0;
    while (index < 2) {
        player = sm_players[index];
        ItemUpdate5B3C expected = &Racer::fn_1_5B28;
        if (player->callback != expected) {
            return 0;
        }
        index += 1;
    }
    return 1;
}

void Racer::fn_1_5B28() {}

int Racer::fn_1_5B2C() { return lbl_1_bss_B8; }

// Runs one update on both racers.
void Racer::fn_1_5B3C(ItemUpdate5B3C update)
{
    int index;
    Racer *item;

    assert(sm_players.size() == entry_num);
    index = 0;
    while (index < 2) {
        item = sm_players[index];
        (item->*update)();
        index += 1;
    }
}

extern "C" {
const MGACTOR_PARAM lbl_1_rodata_33C = {0};
}

void createCharacterMotions(MotionOwner *, unsigned int *, long);

Racer::Racer(int character, int index, Point3d *position)
    : PlayerInfo(character), MotionOwner(characterNo()), GameObject(1),
      callback(&Racer::fn_1_5B28), index(index), playerMask((unsigned short)(1U << index)),
      position(*position), rotation(180.0f),
      firstRegistered(index), secondRegistered(index + 2),
      child(isComputer() ? (RacerControl *)new ComputerControl(this) : new HumanControl(this))
{
    MGACTOR_PARAM actorParam = lbl_1_rodata_33C;
    float firstProperty;
    float secondProperty;
    short modelId;
    actorParam.height = height();
    actorParam.radius = 42.0f;
    actorParam.param = character;
    player = fn_80080728((s16) character, &actorParam, 1, (unsigned short) playerMask, 3U, 0);
    fn_80087158(player, (1 << 1));
    fn_80087158(player, (1 << 0));
    fn_800805CC(player->actor, 4U);
    fn_8007ED60(player);
    modelId = (short)player->actor->mdlId;
    fn_80026B8C(modelId, 2);
    createCharacterMotions(&static_cast<MotionOwner &>(*this), const_cast<unsigned int *>(lbl_1_rodata_320), 4);
    fn_80086AA8(player->actor, position);
    motion.x = lbl_1_rodata_330[0];
    motion.y = lbl_1_rodata_330[0];
    motion.z = lbl_1_rodata_330[0];
    firstProperty = 350.0f;
    firstRegistered.radius = firstProperty;
    secondProperty = 150.0f;
    secondRegistered.radius = secondProperty;
    firstRegistered = *position;
    secondRegistered = *position;
    fn_80086CA8(player->actor, rotation);
    fn_1_7094(this);
    {
        void(Racer::* callback)() = & Racer::fn_1_5B28;
        this->callback = callback;
    }
    new PlayerItem(this, index);
}

Racer::~Racer() {}

void Racer::update()
{
    (this->*callback)();
    fn_1_7124(this);
    fn_1_6CE8(this);
}

void Racer::fn_1_6134()
{
    fn_80087168(player, 1);
    child->reset();
    void (Racer::*nextCallback)();
    nextCallback = &Racer::fn_1_61C0;
    callback = nextCallback;
}

void Racer::fn_1_61C0()
{
    if (flags.attributeEnabled != 0) {
        fn_1_6A38();
    } else {
        child->update();
        if (fn_1_6258() == 0) {
            fn_1_6300();
            if (fn_1_63E8()) {
                return;
            }
        }
    }
}

extern "C" {
class MapManager;

extern void __msl_assertion_failed(const char *, const char *, int);

}
void fn_1_524C(MapManager *, int);
void shiftCharacterMotion(MotionOwner *, unsigned int, float, float, unsigned long);

unsigned char Racer::fn_1_6258()
{
    if (position.y < -200.0f) {
        fn_1_64F0();
        return 1;
    }
    return 0;
}

unsigned int Racer::fn_1_62AC()
{
    u32 code = 0;
    if (fn_80086DD0(player->actor, &code)) return code;
    return 0;
}

void Racer::fn_1_6300()
{
    if (this->flags.collision4 == 0) {
        u32 collisionCode = 0;
        unsigned int code;
        if (fn_80086DD0(this->player->actor, &collisionCode) != 0) {
            code = collisionCode;
        } else {
            code = 0;
        }
        if ((code & 0x4u) != 0u) {
            fn_1_524C(&MinigameSingleton<MapManager>::getChecked(), this->index);
            this->flags.collision4 = 1;
        }
    }
}

unsigned char Racer::fn_1_63E8()
{
    u32 collisionCode = 0;
    unsigned int code;
    int object;
    if (fn_80086DD0(this->player->actor, &collisionCode) != 0) {
        code = collisionCode;
    } else {
        code = 0;
    }
    if ((code & 0x2u) == 0) {
        return 0;
    }
    if (lbl_1_bss_B8 < 1) {
        object = this->playerNo;
        MinigameCore::addWinner(object);
        if (!isComputer()) {
            sendEvent(0, 1, lbl_1_rodata_330[0]);
        }
        this->flags.hasWon = 1;
        lbl_1_bss_B8++;
        this->fn_1_68F8();
    } else {
        this->fn_1_6A38();
    }
    return 1;
}

void Racer::fn_1_64F0()
{
    int mode;
    int actorState;
    void (Racer::*values)();
    this->motion.x = lbl_1_rodata_330[0];
    this->flags.stateFlag = 1;
    fn_80087158(this->player, 1);
    fn_80086E3C(this->player);
    shiftCharacterMotion(&static_cast<MotionOwner &>(*this), CHARMOT_HSF_c000m1_344,
              (float)secondsToFrames(0.3), lbl_1_rodata_330[0],
              ((1 << 30) | (1 << 0)));
    if (this->index == 0) { mode = 0x80F; }
    else { mode = 0x810; }
    actorState = mode;
    soundPlayAt((short)this->playerMask, actorState, &this->position);
    values = &Racer::fn_1_65FC;
    this->callback = values;
}

void sendEvent(int, unsigned long, float);

void Racer::fn_1_65FC()
{
    void (Racer::*nextCallback)();

    motion.y -= lbl_1_bss_90;
    if (this->position.y > -2000.0f) {
        return;
    }
    motion.x = lbl_1_rodata_330[0];
    motion.y = lbl_1_rodata_330[0];
    motion.z = lbl_1_rodata_330[0];
    fn_1_7370(this, 0);
    rumble(2, 0x18);
    sendEvent(index + 2, 0, 1.0f);
    nextCallback = &Racer::fn_1_5B28;
    this->callback = nextCallback;
}

void Racer::fn_1_66F4()
{
    if (fn_1_6FAC(this) != 0) {
        if (flags.attributeEnabled != 0) {
            fn_1_6A38();
            return;
        }
        fn_1_6134();
        return;
    }
    if (position.y < -200.0f) {
        fn_1_64F0();
    }
}

extern "C" {
const HuVecF lbl_1_rodata_380 = {0.0f, 0.0f, 0.0f};
}
template<class T> T fn_1_77B0(T value);
void sendEvent(int channel, unsigned long value, float delay);

void Racer::fn_1_68B4() {
    fn_80087158(player, 1);
    flags.attributeEnabled = 1;
}

void Racer::fn_1_68F8()
{
    void (Racer::*values)();
    fn_80086CBC(player->actor, 1.0f);
    fn_8007E818(player);
    values = &Racer::fn_1_6978;
    callback = values;
}

void Racer::fn_1_6978()
{
    HuVecF target = lbl_1_rodata_380;
    target.y = lbl_1_rodata_334[0];
    target.z = -5050.0f;
    if (fn_8007E8B4(player, &target, (float)fn_1_77B0(200.0), 30.0f) != 0) {
        sendEvent(index + 1, 2, lbl_1_rodata_330[0]);
        fn_1_6A38();
    }
}

void Racer::fn_1_6A38()
{
    void (Racer::*copy)();
    fn_80087158(player, 1);
    copy = &Racer::fn_1_6AA4;
    callback = copy;
}

extern "C" {

typedef void (Racer::*PointSet)();

}

void Racer::fn_1_6AA4()
{
    Vec *actorPosition;
    Vec cameraPosition;
    Vec cameraTarget;
    float rotation;
    float targetAngle;
    float difference;
    float absoluteDifference;
    float error;

    fn_80027CD0(playerMask, &cameraPosition, &cameraTarget,
                     &cameraTarget);
    actorPosition = &player->actor->pos;
    targetAngle = (float)(180.0 *
                          (atan2((double)(cameraPosition.x - actorPosition->x),
                                 (double)(cameraPosition.z - actorPosition->z)) /
                           M_PI));
    fn_80086CB0(player->actor, &rotation);
    Range range(lbl_1_rodata_330[0], 360.0f, rotation);
    fn_80086CA8(player->actor,
                   fn_1_7628(&range, targetAngle, 0.2));
    difference = fn_1_75EC(&range, targetAngle);
    absoluteDifference = __fabsf(difference);
    error = absoluteDifference;
    if (error < 1.0) {
        PointSet defaults = &Racer::fn_1_5B28;
        callback = defaults;
    }
}

void shiftCharacterMotion(MotionOwner *, unsigned int, float, float, unsigned long);

void Racer::fn_1_6C30()
{
    unsigned int motion;
    if (flags.hasWon) motion = CHARMOT_HSF_c000m1_306;
    else motion = CHARMOT_HSF_c000m1_351;
    shiftCharacterMotion(&static_cast<MotionOwner &>(*this), motion, 16.0f, lbl_1_rodata_330[0], 0);
    void (Racer::*action)() = &Racer::fn_1_5B28;
    callback = action;
}

extern "C" {

static const Vec rotationOrigin = {0.0f, 0.0f, 0.0f};

}

void fn_1_6CE8(Racer *object)
{
    float clampedX;
    float positionX;
    float clampedY;
    float positionY;
    float clampedZ;
    float positionZ;
    int modelId = (short)object->player->actor->mdlId;

    if (object->flags.stateFlag == 0U) {
        Vec previousPosition = object->position;
        fn_80086BC8(object->player->actor, &object->position);
        PSVECSubtract(&object->position, &previousPosition, &object->motion);
        fn_80086CB0(object->player->actor, &object->rotation);
    } else {
        PSVECAdd(&object->motion, &object->position, &object->position);
        fn_80026790((short)modelId, &object->position);
        fn_80086AA8(object->player->actor, &object->position);
        Vec rotation = rotationOrigin;
        rotation.y = object->rotation;
        fn_80026838((short)modelId, &rotation);
        fn_80086CA8(object->player->actor, object->rotation);
    }

    positionX = object->position.x;
    if (positionX < -550.0f) {
        clampedX = -550.0f;
    } else if (positionX > 550.0f) {
        clampedX = 550.0f;
    } else {
        clampedX = positionX;
    }
    object->firstRegistered.center.x = clampedX;
    positionY = object->position.y + lbl_1_rodata_338[0];
    if (positionY < -500.0f) {
        clampedY = -500.0f;
    } else if (positionY > 500.0f) {
        clampedY = 500.0f;
    } else {
        clampedY = positionY;
    }
    object->firstRegistered.center.y = clampedY;
    positionZ = object->position.z;
    if (positionZ < -5600.0f) {
        clampedZ = -5600.0f;
    } else if (positionZ > -120.00001f) {
        clampedZ = -120.00001f;
    } else {
        clampedZ = positionZ;
    }
    object->firstRegistered.center.z = clampedZ;
    object->secondRegistered = object->position;
    {
        float verticalSpeed = object->secondRegistered.radius;
        object->secondRegistered.center.y += verticalSpeed;
    }
    soundPanCharacterVoice(object->playerMask, (short)object->characterNo(),
              &object->position);
}

inline u8 fn_1_6FAC(Racer *self) {
    u32 colCode;

    if (self->flags.stateFlag) {
        return 0U;
    }
    colCode = 0;
    fn_80086DD0(self->player->actor, &colCode);
    return (u8) ((colCode & 0x407F) != 0);
}

u8 fn_1_7020(Racer *playerState) {
    u32 collisionCode;

    if (playerState->flags.stateFlag) {
        return 0U;
    }
    collisionCode = 0;
    fn_80086DD0(playerState->player->actor, &collisionCode);
    return (u8) ((collisionCode & 0xBF80) != 0);
}

float RandomFloat(float angle);

extern "C" {

typedef HuVecF Vector7124;

}

void fn_1_7094(Racer *actor)
{
    short shadowModelId = (short)actor->player->actor->mdlId;
    fn_80026E44(shadowModelId);
}

void fn_1_70DC(Racer *actor)
{
    short shadowModelId = (short)actor->player->actor->mdlId;
    fn_80026EF0(shadowModelId);
}

void fn_1_7124(Racer *actor)
{
    MGACTOR_COLMAP_POLY polygon;
    Vector7124 point;
    Vector7124 groundPoint;
    short shadowModelId;
    short hitModelId;
    short resetModelId;
    float radius;
    float baseAngle;
    float angle;
    Vector7124 *pointPointer;
    Vector7124 *sourcePointer;
    int index;

    if (fn_1_6FAC(actor) != 0) {
        shadowModelId = actor->player->actor->mdlId;
        fn_80026E44(shadowModelId);
        return;
    }

    radius = lbl_1_rodata_338[0];
    baseAngle = RandomFloat(6.2831855f);
    for (index = 0; index < 3; index++) {
        double sineOffset;
        double cosineOffset;

        angle = baseAngle + ((M_PI * 360.0 / 180.0) * (double)index) /
                3.0;
        cosineOffset = radius * std::cos(angle);
        sineOffset = radius * std::sin(angle);
        point.x = sineOffset;
        point.y = 10.0f;
        point.z = cosineOffset;
        pointPointer = &point;
        sourcePointer = pointPointer;
        groundPoint.x = sourcePointer->x;
        groundPoint.y = sourcePointer->y;
        groundPoint.z = sourcePointer->z;
        PSVECAdd(&actor->position, &point, &point);
        PSVECAdd(&actor->position, &groundPoint, &groundPoint);
        groundPoint.y = -1000.0f;
        if (fn_80086910(&point, &groundPoint, 1, &polygon) != 0) {
            hitModelId = actor->player->actor->mdlId;
            fn_80026E44(hitModelId);
            return;
        }
    }

    resetModelId = actor->player->actor->mdlId;
    fn_80026EF0(resetModelId);
}

void fn_1_7370(Racer *self, unsigned char enabled) {
    int character = self->characterNo();
    if (enabled) {
        fn_8005C47C(character, 1);
    } else {
        fn_8005C430(character, 1);
    }
    fn_8005DFC0(character, enabled);
}

void shiftCharacterMotion(MotionOwner *, unsigned int, float, float, unsigned long);
extern "C" {

typedef void (Racer::*CallbackWords)();

}

void fn_1_73E4(Racer *work)
{
    int character;
    CallbackWords copy;

    work->rotation = lbl_1_rodata_330[0];
    shiftCharacterMotion(&static_cast<MotionOwner &>(*work), CHARMOT_HSF_c000m1_376,
                         lbl_1_rodata_330[0],
              lbl_1_rodata_330[0], ((1 << 30) | (1 << 0)));
    character = work->characterNo();
    fn_8005C47C((short)character, 1);
    fn_8005DFC0((short)character, 1);
    copy = &Racer::fn_1_74B0;
    work->callback = copy;
}

void Racer::fn_1_74B0() {}

void fn_1_74B4(Racer *work, const Vec *offset)
{
    int character = (short)work->player->actor->mdlId;
    char *hook = fn_8005C0F0((short)work->characterNo(), 1, 4);
    Vec objectPosition;
    Vec *positionForAddition;
    Vec *position;

    Hu3DModelObjPosGet(character, hook, &objectPosition);
    PSVECSubtract(&work->position, &objectPosition, &objectPosition);
    position = &objectPosition;
    positionForAddition = position;
    PSVECAdd(offset, positionForAddition, &work->position);
}

void fn_1_754C(Racer *work)
{
    void (Racer::*values)();
    work->flags.stateFlag = 0;
    fn_80086F00(work->player, &work->position);
    fn_8008697C(work->player, &work->position);
    fn_80086CC4(work->player->actor, lbl_1_rodata_330[0]);
    values = &Racer::fn_1_66F4;
    work->callback = values;
}

template<> inline float fn_1_75EC<float>(Range *self, float value)
{
    return self->fn_1_7CE0(self->current, value);
}
template<> inline float fn_1_7628<float>(Range *range, float step, double rate)
{
    float progress = range->fn_1_7CE0(range->current, step);
    return range->current = range->fn_1_1370(range->current + rate * progress);
}
template<class T> T RangeT<T>::fn_1_1370(T value) {
    if (value < this->minimum) {
        return this->maximum - (f32) std::fmod((f64) (this->minimum - value), (f64) this->period);
    }
    if (value >= this->maximum) {
        return this->minimum + (f32) std::fmod((f64) (value - this->maximum), (f64) this->period);
    }
    return value;
}

template<class T> T fn_1_77B0(T value) {
    return 0.016666666666666666 * value;
}

template<class T> inline T rangeMinimum(T a, T b) { return a < b ? a : b; }
template<class T> inline T rangeMaximum(T a, T b) { return a > b ? a : b; }

template<class T> RangeT<T>::RangeT(T a, T b, T initial)
{
    minimum = rangeMinimum(a, b);
    maximum = rangeMaximum(a, b);
    period = maximum - minimum;
    current = fn_1_1370(initial);
}

template<class T> T RangeT<T>::fn_1_7CE0(T first, T second) const
{
    T wrappedSecond;
    T wrappedFirst;
    T difference;
    T magnitude;

    if (second < this->minimum) {
        wrappedSecond = this->maximum -
            (T)std::fmod((double)(this->minimum - second), (double)this->period);
    } else if (second >= this->maximum) {
        wrappedSecond = this->minimum +
            (T)std::fmod((double)(second - this->maximum), (double)this->period);
    } else {
        wrappedSecond = second;
    }

    if (first < this->minimum) {
        wrappedFirst = this->maximum -
            (T)std::fmod((double)(this->minimum - first), (double)this->period);
    } else if (first >= this->maximum) {
        wrappedFirst = this->minimum +
            (T)std::fmod((double)(first - this->maximum), (double)this->period);
    } else {
        wrappedFirst = first;
    }

    difference = wrappedSecond - wrappedFirst;
    magnitude = Abs7CE0(difference);
    return magnitude < this->period / 2.0f
        ? difference
        : (this->period - magnitude) *
          (T)(difference < lbl_1_rodata_330[0] ? 1 : -1);
}
