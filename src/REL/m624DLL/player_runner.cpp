#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/m624DLL/scene_sequence_contract.h"
#include "REL/framework/sound.h"
#include "REL/framework/minigame_core.h"
#include "REL/m624DLL/game_constants.h"
#include "REL/framework/phase_durations.h"
#include "REL/framework/world_units.h"
#include "dolphin/mtx.h"
#include "math.h"
extern "C" {
#include "dolphin/types.h"
}
extern "C" {
#include "dolphin/os.h"
}
#include "REL/m624DLL/player_task.h"
#include "REL/m624DLL/player_callbacks.h"
#include "game/mg/actman.h"
#include "REL/m624DLL/player_setup.h"
#include "REL/m624DLL/player_math.h"
#include "REL/m624DLL/tail_types.h"
extern "C" {
#include "game/hu3d.h"
}
extern "C" {
#include "game/mg/colman.h"
}
extern "C" {
#include "datadir_enum.h"
}
extern "C" {
#include "game/charman.h"
}
#include "humath.h"
#include "REL/framework/singleton.h"
#include "REL/m624DLL.h"
#include "REL/m624DLL/range.h"
int fn_1_9484(COL_NARROW_PARAM *, COL_NARROW_PARAM *);
void fn_1_9078(PlayerRunner *, u8);
void *operator new(unsigned long);
PlayerRunner::PlayerRunner(s32 item, Position *bounds) : Player(item) { s16 modelId;
    PlayerControl *selectedEffect;
    this->despawned = 0; this->respawnBlocked = 0;
    this->state46 = 0;
    this->spawn_position = *bounds;
    this->current_position = *bounds;
    this->scale = -1.0f;
    this->lastCollisionModel = 0;
    this->lastYaw = -1.0f;
    this->countdown = 0;
    selectedEffect = this->pad >= 0
        ? static_cast<PlayerControl *>(new ManualControl(this))
        : new ComControl(this);
    this->cleanup_owner.value = selectedEffect;
    this->collisionPending = 0;
 {
        MGACTOR_PARAM parameters = {0.0f, 0.0f, 0, 0, fn_1_9484, 0, 0, 0};
        parameters.height = fn_8005C4C8((s16)this->character);
        parameters.radius = 42.0f;
        parameters.param = item;
        this->player = fn_80080728((s16)item, &parameters, 8, 1U, 0x23U, 0);
        fn_80087158(this->player, 1U);
        fn_80086E3C(this->player);
        fn_1_9900();
        modelId = (s16)this->player->actor->mdlId;
        fn_80026B8C(modelId, 4);
        fn_1_9078(this, 0);
        this->velocity.x = 0.0f;
        this->velocity.y = 0.0f;
        this->velocity.z = 0.0f;
        fn_80086AA8(this->player->actor, &this->spawn_position.value);
        this->rotationX = 0.0f;
        this->angle = 0.0f;
        this->rotationZ = 0.0f;
        fn_80086CA8(this->player->actor, this->angle); }
}
void operator delete(void *);
ControlOwner::~ControlOwner() { delete value; }
PlayerRunner::~PlayerRunner() { fn_80080F38(player->actor); }
s32 fn_1_79F4(void) { PlayerRunner *node;
    s32 count;
    Node *end;
    PlayerRunner *checkNode;
    PlayerRunner *firstNode;
    PlayerRunner *startNode;
    s32 status;
    PlayerRunner *nextNode;
    firstNode = (PlayerRunner *)lbl_1_bss_9C.link0;
    startNode = firstNode;
    node = startNode;
    end = &lbl_1_bss_9C;
    count = 0;
    while ((Node *)node != end) { status = node->group;
        if (status < 2) { checkNode = node;
            if (fn_1_911C(checkNode) != 0) { count++; }
        }
        nextNode = (PlayerRunner *)node->link0;
        node = nextNode; }
    return count; }
void PlayerRunner::prepare() { PlayerControl *consumer = cleanup_owner.value;
    consumer->slot10(); }
typedef PlayerTaskState PlayerState;
void fn_1_9078(PlayerRunner *owner, u8 enabled);
void fn_1_9188(PlayerRunner *owner);
extern "C" void __msl_assertion_failed(const char *expression, const char *file, int line);
struct SceneOwner;
void PlayerRunner::slot14() { PlayerState templateState;
    PlayerState initialState;
    this->despawned = 0;
    fn_80086F00(this->player, &this->spawn_position.value);
    fn_8007ED60(this->player);
    fn_8007E818(this->player);
    fn_1_9078(this, 1);
    fn_1_9188(this);
    fn_1_4FBC(&SceneOwner::getChecked());
    templateState = static_cast<PlayerTaskState>(&PlayerRunner::fn_1_7BD0);
    initialState = templateState;
    this->mode = initialState; }
typedef PlayerTaskState ChaseState;
void PlayerRunner::fn_1_7BD0() { Vec destination = this->spawn_position.value;
    ChaseState defaultState;
    ChaseState state;
    destination.z += cmToWorld(400.0f);
    if (fn_8007E8B4(this->player, &destination,
                         (f32)framesToSeconds(229.99999999999997), 10.0f)) {
        defaultState = static_cast<PlayerTaskState>(&Player::fn_1_7060);
        state = defaultState;
        this->mode = state; }
}
void fn_1_7DAC(PlayerRunner *);
void fn_1_7E20(PlayerRunner *);
void fn_1_9218(PlayerRunner *);
u8 fn_1_7E90(PlayerRunner *);
u8 fn_1_82BC(PlayerRunner *);
void PlayerRunner::slot18() { fn_8007E888(this->player);
    PlayerTaskState defaults = static_cast<PlayerTaskState>(&PlayerRunner::fn_1_7D2C);
    PlayerTaskState copy = defaults;
    this->mode = copy; }
void PlayerRunner::fn_1_7D2C() {
    if (this->respawnBlocked != 0) { fn_1_8CF4(this); } else { fn_1_7DAC(this);
        fn_1_7E20(this);
        fn_1_9218(this);
        if (fn_1_7E90(this) == 0) {
            if (fn_1_82BC(this) != 0) { return; }
        }
    }
}
extern void fn_1_8584(PlayerRunner *owner);
void fn_1_6F68(Player *owner, s32 mode);
extern "C" { void fn_80086BC8(MGACTOR *actor, Point3d *position); }
extern "C" { void fn_80080DD4(MGACTOR *actor, u32 mask); }
extern s32 lbl_1_bss_A4;
extern "C" { void fn_80086B04(MGACTOR *, Point3d *); }
typedef MGACTOR_COLMAP_POLY ShadowCollisionPolygon;
void fn_1_6CD8(Player *owner, s32 motionIndex, u8 shift);
extern "C" { void fn_80086CB0(MGACTOR *actor, f32 *rotation); }
extern "C" { void fn_80086CA8(MGACTOR *actor, f32 rotation); }
void fn_1_97DC(PlayerRunner *owner);
extern "C" { void fn_1_7AC8(PlayerRunner *); }
extern "C" { void fn_1_7CAC(PlayerRunner *); }
extern "C" { void fn_1_8E70(ModeResetOwner *); }
void fn_1_6B74(Player *owner, u32 motionFile);
void fn_1_7DAC(PlayerRunner *owner) { Point3d position;
    f32 minimumZ;
    fn_80086BC8(owner->player->actor, &position);
    minimumZ = -860.0f;
    if (position.z < minimumZ) { position.z = minimumZ;
        fn_80086B04(owner->player->actor, &position); }
}
void fn_1_7E20(PlayerRunner *owner) { u32 mask;
    Point3d position;
    fn_80086BC8(owner->player->actor, &position);
    mask = 1;
    if (position.z > -650.0) { mask |= 2; }
    fn_80080DD4(owner->player->actor, mask); }
const Point3d lbl_1_rodata_368[2] = { { -163.86f, 215.07f, -12.11f }, { 136.7f, 252.7f, 5.27f } };
static inline int ShadowRadius() { return 100; }
u8 fn_1_7E90(PlayerRunner *owner) { Point3d position;
    s32 thresholdIndex;
    fn_80086BC8(owner->player->actor,
                  &position);
    thresholdIndex = 0;
    while (thresholdIndex < 2) { const Point3d *threshold = &lbl_1_rodata_368[thresholdIndex];
        if (!(position.y >= threshold->y)) {
            if (!(sqrt(Square(position.x - threshold->x) +
                        Square(position.z - threshold->z)) > ShadowRadius())) {
                f32 heightDifference = threshold->y - position.y;
                f32 modelHeight = 2.0 * fn_8005C4C8((s16)owner->character);
                f32 fade = Clamp(1.0 - heightDifference / modelHeight, 0.0, 1.0);
                owner->scale = owner->character == 6 ? fade : -1.0f;
                if (fade <= 0.0f) { owner->scale = 1.0f;
                    owner->finish();
                    owner->scale = -1.0f;
                    fn_1_9078(owner, 0);
                    fn_1_8584(owner);
                    return 1; }
            }
        }
        thresholdIndex++; }
    if (position.y < -300.0f) { fn_1_8584(owner);
        return 1; }
    return 0; }
extern "C" BOOL fn_80086DD0(MGACTOR *actor, u32 *code);
u8 fn_1_82BC(PlayerRunner *self) { u32 collision_code = 0;
    if (fn_80086DD0(self->player->actor, &collision_code) == 0) { return 0; }
    if ((collision_code & 2) != 0) {
        if (lbl_1_bss_A4 < 1) { MinigameCore::addWinner(self->playerNo);
            self->state46 = 1;
            lbl_1_bss_A4 += 1;
            fn_1_8B58(self); } else { fn_1_8CF4(self); }
        return 1; }
    return 0; }
const Point3d lbl_1_rodata_3B8 = { 701.0f, 0.0f, 87.0f };
void fn_1_837C(PlayerRunner *owner) { Point3d referencePosition = lbl_1_rodata_3B8;
    Point3d direction;
    Point3d *scaleInput;
    Point3d *directionVector;
    f64 sine;
    f64 cosine;
    f64 clampedAngle;
    f64 targetAngle;
    f32 angle;
    f32 limit;
    PSVECSubtract(&owner->updated_position.value, &referencePosition,
                  &direction);
    targetAngle = atan2((f64)direction.x, (f64)direction.z);
    if (targetAngle < -(M_PI * 40 / 180)) { clampedAngle = -(M_PI * 40 / 180);
    } else if (targetAngle > -(M_PI * 10 / 180)) { clampedAngle = -(M_PI * 10 / 180);
    } else { clampedAngle = targetAngle; }
    angle = (f32)clampedAngle;
    cosine = cos((f64)angle);
    sine = sin((f64)angle);
    direction.x = (f32)sine;
    direction.y = 1.0f;
    direction.z = (f32)cosine;
    limit = (f32)framesToSeconds(0x12C0L);
    PSVECNormalize(&direction, &direction);
    directionVector = &direction;
    scaleInput = directionVector;
    PSVECScale(scaleInput, &owner->velocity, limit);
    owner->angle =
        (f32)(180.0 * ((M_PI +
                        atan2((f64)owner->velocity.x,
                              (f64)owner->velocity.z)) /
                       M_PI));
    fn_1_8584(owner);
    fn_1_6F68(owner, 1);
}
struct SceneOwner;
static inline void setChaseState(PlayerTaskState &state) {
    state = static_cast<PlayerTaskState>(&PlayerRunner::fn_1_868C);
}
void fn_1_8584(PlayerRunner *owner) { double currentX;
    double currentZ;
    PlayerControl *cleanupOwner;
    PlayerTaskState initialState;
    currentZ = owner->current_position.value.z;
    currentX = owner->current_position.value.x;
    owner->updated_position.value.x = currentX;
    owner->updated_position.value.y = 0.0f;
    owner->updated_position.value.z = currentZ;
    cleanupOwner = owner->cleanup_owner.value;
    cleanupOwner->slot0C();
    owner->despawned = 1;
    fn_80086E3C(owner->player);
    fn_8007EDBC(owner->player);
    fn_1_6CD8(owner, 0, 1);
    setChaseState(initialState);
    owner->setState(initialState);
}
void PlayerRunner::fn_1_868C() { Vec delta;
    PlayerTaskState templateState;
    PlayerTaskState state;
    double deltaX;
    double deltaZ;
    f32 distance;
    bool timerExpired;
    bool distanceExceeded;
    s32 nextCountdown;
    this->rotationX =
        (f32)((f64)this->rotationX -
              framesToSeconds(60.0));
    timerExpired = this->current_position.value.y <
                   -2000.0f;
    deltaZ = this->current_position.value.z -
             this->updated_position.value.z;
    deltaX = this->current_position.value.x -
             this->updated_position.value.x;
    delta.x = deltaX;
    delta.y = 0.0f;
    delta.z = deltaZ;
    distance = PSVECMag(&delta);
    distanceExceeded = distance > 3000.0f;
    if ((u8)timerExpired || (u8)distanceExceeded) { fn_1_9078(this, 0);
        fn_8005DFC0((s16)this->character, 0);
        if ((u8)timerExpired) { fn_1_6F68(this, 2);
        }
        nextCountdown = secondsToFrames(2L);
        this->countdown = nextCountdown;
        templateState = static_cast<PlayerTaskState>(&PlayerRunner::fn_1_8840);
        state = templateState;
        this->mode = state;
    }
}
void PlayerRunner::fn_1_8840() { PlayerTaskState initialState;
    PlayerTaskState fallbackState;
    if (this->respawnBlocked == 0) { countDown();
        if (this->countdown <= 0) {
            if (fn_1_4FBC(&SceneOwner::getChecked())) { this->despawned = 0;
                fn_80086F00(this->player, &this->spawn_position.value);
                fn_8007ED60(this->player);
                fn_8008697C(this->player, &this->spawn_position.value);
                fn_80086CC4(this->player->actor, 0.0f);
                fn_8007E818(this->player);
                fn_1_9078(this, 1);
                fn_1_9188(this);
                initialState = static_cast<PlayerTaskState>(&PlayerRunner::fn_1_89C0);
                fallbackState = initialState;
                this->mode = fallbackState;
            }
        }
    }
}
void PlayerRunner::fn_1_89C0() { Position target;
    PlayerTaskState initialState;
    PlayerTaskState fallbackState;
    f32 targetOffset;
    f32 chaseAngle;
    targetOffset = cmToWorld(400.0f);
    chaseAngle = (f32)framesToSeconds(229.99999999999997);
    target = this->spawn_position;
    target.value.z += targetOffset;
    this->current_position.value.z += chaseAngle;
    fn_80086B04(this->player->actor, &this->current_position.value);
    if (fn_8007E8B4(this->player, &target.value, chaseAngle,
                         10.0f) != 0 ||
        this->current_position.value.z > target.value.z) { fn_8007E888(this->player);
        if (this->respawnBlocked == 0) {
            initialState = static_cast<PlayerTaskState>(&PlayerRunner::fn_1_7D2C);
            fallbackState = initialState;
            this->mode = fallbackState;
        } else { fn_1_8CF4(this);
        }
        fn_8005DFC0((s16)this->character, 1);
    }
}
void PlayerRunner::slot20() { fn_80087158(this->player, 1);
    this->respawnBlocked = 1;
}
void fn_1_8B58(PlayerRunner *task) { fn_8007E818(task->player);
    PlayerTaskState defaults = static_cast<PlayerTaskState>(&PlayerRunner::fn_1_8BD8);
    PlayerTaskState copy = defaults;
    task->mode = copy;
}
void PlayerRunner::fn_1_8BD8() { Vec actorPosition;
    Vec chasePosition;
    double chaseX;
    double chaseY;
    double chaseZ;
    MGACTOR *actor;
    actor = player->actor;
    fn_80086BC8(actor, &actorPosition);
    if (actorPosition.y < spawn_position.value.y) { actorPosition.y = spawn_position.value.y;
        fn_80086AA8(actor, &actorPosition);
        actor->colGroundAttr |= 2;
    }
    chaseZ = spawn_position.value.z + cmToWorld(400.0f);
    chaseY = spawn_position.value.y;
    chaseX = 800.0;
    chasePosition.x = (f32)chaseX;
    chasePosition.y = (f32)chaseY;
    chasePosition.z = (f32)chaseZ;
    if (fn_8007E8B4(player, &chasePosition,
                         (f32)framesToSeconds(200.0), 10.0f) != 0) { fn_1_8CF4(this);
    }
}
extern "C" { typedef PlayerTaskState M624PlayerSettings; }
void fn_1_8CF4(void *object) { M624PlayerSettings source;
    M624PlayerSettings settings;
    fn_80087158(static_cast<PlayerRunner *>(object)->player, 1U);
    source = static_cast<PlayerTaskState>(&PlayerRunner::fn_1_8D78);
    settings = source;
    static_cast<PlayerRunner *>(object)->mode = settings;
}
extern "C" {
void PlayerRunner::fn_1_8D78() { Vec camera_position;
    Vec camera_target;
    f32 actor_yaw;
    f32 angle;
    Vec *actor_position;
    fn_80027CD0(1, &camera_position, &camera_target, &camera_target);
    actor_position = &this->player->actor->pos;
    angle = (f32)(180.0 * (atan2((f64)(camera_position.x - actor_position->x),
                                 (f64)(camera_position.z - actor_position->z)) /
                           M_PI));
    fn_80086CB0(this->player->actor, &actor_yaw);
    Range yaw_range(0.0f, 360.0f, actor_yaw);
    fn_80086CA8(this->player->actor,
                   yaw_range.fn_1_9AE0(angle, 0.2));
}
}
extern "C" { typedef PlayerTaskState ModeResetData; }
extern "C" {
void PlayerRunner::slot1C() { s32 mode;
    ModeResetData defaults;
    ModeResetData copy;
    if (this->state46 != 0) { mode = 1;
    } else { mode = 2;
    }
    fn_1_6CD8(this, mode, 0);
    defaults = &Player::fn_1_7060;
    copy = defaults;
    this->mode = copy;
}
}
void PlayerRunner::finish() { s32 modelId;
    modelId = (s16)this->player->actor->mdlId;
    if (this->despawned == 0) { HuVecF previousPosition = this->current_position.value;
        fn_80086BC8(this->player->actor, &this->current_position.value);
        PSVECSubtract(&this->current_position.value, &previousPosition, &this->velocity);
        fn_1_97DC(this);
        fn_80086CB0(this->player->actor, &this->angle);
        this->rotationX = 0.0f;
        this->rotationZ = 0.0f;
        this->m_motion = -1;
        if (this->scale >= 0.0f) {
            fn_800268B0(modelId, this->scale, this->scale,
                              this->scale);
        }
        soundPanCharacterVoice(1, (s16)this->character, &this->current_position.value);
        return;
    }
    PSVECAdd(&this->velocity, &this->current_position.value,
             &this->current_position.value);
    this->velocity.y -= gravity;
    fn_80026790(modelId, &this->current_position.value);
    soundPanCharacterVoice(1, (s16)this->character, &this->current_position.value);
    fn_80026838(modelId, reinterpret_cast<Vec *>(&this->rotationX));
}
void fn_1_9078(PlayerRunner *owner, u8 enabled) {
    if (enabled != 0) { fn_8005C47C((s16)owner->character, 1);
        return;
    }
    fn_8005C430((s16)owner->character, 1);
}
u8 fn_1_911C(PlayerRunner *self);
void PlayerRunner::update() {
    if (fn_1_911C(this) != 0) { fn_1_6F68(this, 0);
    }
}
inline u8 fn_1_911C(PlayerRunner *entry) { u32 collisionCode;
    if (entry->despawned != 0) { return 0U;
    }
    collisionCode = 0;
    fn_80086DD0(entry->player->actor, &collisionCode);
    return (u8) ((collisionCode & 0x407F) != 0);
}
void fn_1_9188(PlayerRunner *owner) { s16 shadowModelId = (s16)owner->player->actor->mdlId;
    fn_80026E44(shadowModelId);
}
void fn_1_91D0(PlayerRunner *owner) { s16 shadowModelId = (s16)owner->player->actor->mdlId;
    fn_80026EF0(shadowModelId);
}
void fn_1_9218(PlayerRunner *owner) { Point3d *position = &owner->player->actor->pos;
    ShadowCollisionPolygon polygon;
    s32 sample;
    if (position->y < 0.0f) { s16 belowGroundModelId = (s16)owner->player->actor->mdlId;
        fn_80026EF0(belowGroundModelId);
        return;
    }
    if (fn_1_911C(owner) != 0) { s16 codeHitModelId = (s16)owner->player->actor->mdlId;
        fn_80026E44(codeHitModelId);
        return;
    }
    for (sample = 0; sample < 3; sample++) { f32 offset;
        f64 aboveX;
        f64 aboveY;
        f64 aboveZ;
        f64 belowX;
        f64 belowY;
        f64 belowZ;
        f64 offsetValue;
        Point3d above;
        Point3d below;
        offsetValue = 100.0 * (0.8 * (f64)(sample - 1));
        offset = (f32)offsetValue;
        aboveZ = position->z + offset;
        aboveY = position->y + cmToWorld(3000.0f);
        aboveX = position->x;
        above.x = aboveX;
        above.y = aboveY;
        above.z = aboveZ;
        belowZ = position->z + offset;
        belowY = position->y;
        belowX = position->x;
        below.x = belowX;
        below.y = belowY;
        below.z = belowZ;
        if (fn_80086910(&above, &below, 3, &polygon) != 0) {
            s16 collisionModelId = (s16)owner->player->actor->mdlId;
            fn_80026EF0(collisionModelId);
            return;
        }
    }
    s16 clearPathModelId = (s16)owner->player->actor->mdlId;
    fn_80026E44(clearPathModelId);
}
struct CollisionTarget {
    u8 unknown00[0x54]; /* No code names this field. */
    Point3d position;
};
static inline void fn_1_8584_for_9484(PlayerRunner *owner) { double currentZ;
    double currentX;
    PlayerControl *cleanupOwner;
    PlayerRunner::Callback initialState;
    PlayerRunner::Callback templateState;
    currentZ = owner->current_position.value.z;
    currentX = owner->current_position.value.x;
    owner->updated_position.value.x = currentX;
    owner->updated_position.value.y = 0.0f;
    owner->updated_position.value.z = currentZ;
    cleanupOwner = owner->cleanup_owner.value;
    cleanupOwner->slot0C();
    owner->despawned = 1;
    fn_80086E3C(owner->player);
    fn_8007EDBC(owner->player);
    fn_1_6CD8(owner, 0, 1);
    setChaseState(templateState);
    initialState = templateState;
    owner->mode = initialState;
}
int fn_1_9484(COL_NARROW_PARAM *source,
                         COL_NARROW_PARAM *operation) {
    PlayerRunner *target = (PlayerRunner *)fn_1_65B0(source->paramB);
    if (target == 0) { return 0;
    }
    if (operation->type == 0) {
        CollisionTarget *other =
            (CollisionTarget *)fn_1_65B0(operation->paramB);
        target->collisionPosition.x = other->position.x;
        target->collisionPosition.y = other->position.y;
        target->collisionPosition.z = other->position.z;
        target->collisionPending = 1;
        return 1;
    } else if (operation->type != 1) { return 0;
    } else { Point3d direction;
        Point3d *scaleInput;
        Point3d *directionVector;
        f64 clampedAngle;
        f64 angle;
        f64 sine;
        f64 cosine;
        f32 speed;
        f32 angleForTrig;
        Point3d referencePosition = lbl_1_rodata_3B8;
        PSVECSubtract(&target->updated_position.value, &referencePosition,
                      &direction);
        angle = atan2((f64)direction.x, (f64)direction.z);
        if (angle < -(M_PI * 40 / 180)) { clampedAngle = -(M_PI * 40 / 180);
        } else if (angle > -(M_PI * 10 / 180)) { clampedAngle = -(M_PI * 10 / 180);
        } else { clampedAngle = angle;
        }
        angleForTrig = (f32)clampedAngle;
        cosine = cos((f64)angleForTrig);
        sine = sin((f64)angleForTrig);
        direction.x = (f32)sine;
        direction.y = 1.0f;
        direction.z = (f32)cosine;
        speed = (f32)framesToSeconds(0x12C0L);
        PSVECNormalize(&direction, &direction);
        directionVector = &direction;
        scaleInput = directionVector;
        PSVECScale(scaleInput, &target->velocity, speed);
        target->angle =
            (f32)(180.0 * ((M_PI +
                            atan2((f64)target->velocity.x,
                                  (f64)target->velocity.z)) /
                           M_PI));
        fn_1_8584_for_9484(target);
        fn_1_6F68(target, 1);
        soundPlayAt(1, 0x702, &target->current_position.value);
        return 1;
    }
}
void fn_1_97DC(PlayerRunner *owner) { HSF_OBJECT *collisionModel;
    f32 actorYaw;
    f32 modelYaw;
    f32 yawDifference;
    collisionModel = owner->player->actor->colObj;
    if (collisionModel != 0 && collisionModel == owner->lastCollisionModel) {
        modelYaw = collisionModel->mesh.curr.rot.y;
        Range range(0.0f, 360.0f, 0.0f);
        if (owner->lastYaw >= 0.0f) { yawDifference = range.fn_1_999C(owner->lastYaw, modelYaw);
            fn_80086CB0(owner->player->actor, &actorYaw);
            fn_80086CA8(owner->player->actor,
                           actorYaw += yawDifference);
        }
        owner->lastCollisionModel = collisionModel;
        owner->lastYaw = range.fn_1_1370(modelYaw);
    } else { owner->lastCollisionModel = collisionModel;
        owner->lastYaw = -1.0f;
    }
}
extern "C" void fn_8005B4BC(s16 motionId);
void PlayerRunner::fn_1_9900() { const u32 NUMBEROF_MOTION = 3;
    m_motion_id.clear();
    fn_1_6B74(this, DATANUM(0x930000, 34));
    fn_1_6B74(this, DATANUM(0x930000, 6));
    fn_1_6B74(this, DATANUM(0x930000, 40));
    assert(m_motion_id.size() == NUMBEROF_MOTION);
    fn_8005B4BC((s16)character);
}

f32 Range::fn_1_999C(f32 startAngle, f32 endAngle)
{
    f32 start = fn_1_1370(startAngle);
    f32 difference = fn_1_1370(endAngle) - start;
    f32 wrapDifference;
    f32 absoluteDifference;

    if (difference < 0.0f) {
        absoluteDifference = -difference;
    } else {
        absoluteDifference = difference;
    }
    wrapDifference = absoluteDifference;
    return wrapDifference < period / 2.0f
               ? difference
               : (period - wrapDifference) *
                     (f32)(difference < 0.0f ? 1 : -1);
}

float Range::fn_1_9AE0(float value, double rate) {
    return current = fn_1_1370(current + rate * fn_1_9D3C(value));
}

inline float Range::fn_1_1370(float value) {
    if (value < this->minimum) {
        return this->maximum - (f32) fmod((f64) (this->minimum - value), (f64) this->period);
    }
    if (value >= this->maximum) {
        return this->minimum + (f32) fmod((f64) (value - this->maximum), (f64) this->period);
    }
    return value;
}

static inline float fn_1_9B3C(Range *range, float value) {
    if (value < range->minimum) {
        return range->maximum - (f32) fmod((f64) (range->minimum - value), (f64) range->period);
    }
    if (value >= range->maximum) {
        return range->minimum + (f32) fmod((f64) (value - range->maximum), (f64) range->period);
    }
    return value;
}

static inline float rangeMinimum(float a, float b) { return a < b ? a : b; }

static inline float rangeMaximum(float a, float b) { return a > b ? a : b; }

Range::Range(float a, float b, float initial) {
    minimum = rangeMinimum(a, b);
    maximum = rangeMaximum(a, b);
    period = maximum - minimum;
    current = fn_1_1370(initial);
}

float Range::fn_1_9D3C(float value) {
    float difference;
    float currentValue = current;
    float absoluteDifference;
    float thresholdDistance;

    difference = fn_1_9B3C(this, value) - fn_1_9B3C(this, currentValue);
    if (difference < 0.0f) {
        thresholdDistance = -difference;
    } else {
        thresholdDistance = difference;
    }
    absoluteDifference = thresholdDistance;
    return absoluteDifference < period / 2.0f
        ? difference
        : (period - absoluteDifference) * (f32)(difference < 0.0f ? 1 : -1);
}
