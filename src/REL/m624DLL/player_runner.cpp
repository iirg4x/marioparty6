/* Spacing preserves the assertion's source line.
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

#include "REL/m624DLL/range.h"
int fn_1_9484(COL_NARROW_PARAM *, COL_NARROW_PARAM *);

static inline void setMode(Player *player, Player::Mode next) {
    player->mode = next;
}

static inline void setRunnerState(PlayerRunner *runner, void (PlayerRunner::*next)()) {
    setMode(runner, static_cast<Player::Mode>(next));
}
PlayerRunner::PlayerRunner(s32 item, Position *bounds) : Player(item) {
    s16 modelId;
    PlayerControl *selectedEffect;
    this->despawned = 0;
    this->respawnBlocked = 0;
    this->hasWon = 0;
    this->spawn_position = *bounds;
    this->current_position = *bounds;
    this->scale = -1.0f;
    this->lastCollisionModel = 0;
    this->lastYaw = -1.0f;
    this->countdown = 0;
    selectedEffect = this->pad >= 0 ? static_cast<PlayerControl *>(new ManualControl(this))
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
        this->rotation.x = 0.0f;
        this->rotation.y = 0.0f;
        this->rotation.z = 0.0f;
        fn_80086CA8(this->player->actor, this->rotation.y);
    }
}
ControlOwner::~ControlOwner() {
    delete value;
}
PlayerRunner::~PlayerRunner() {
    fn_80080F38(player->actor);
}
s32 fn_1_79F4(void) {
    PlayerRunner *node;
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
    while ((Node *)node != end) {
        status = node->group;
        if (status < 2) {
            checkNode = node;
            if (fn_1_911C(checkNode) != 0) {
                count++;
            }
        }
        nextNode = (PlayerRunner *)node->link0;
        node = nextNode;
    }
    return count;
}
void PlayerRunner::prepare() {
    PlayerControl *consumer = cleanup_owner.value;
    consumer->update();
}
void fn_1_9188(PlayerRunner *owner);
void PlayerRunner::moveToStart() {
    this->despawned = 0;
    fn_80086F00(this->player, &this->spawn_position.value);
    fn_8007ED60(this->player);
    fn_8007E818(this->player);
    fn_1_9078(this, 1);
    fn_1_9188(this);
    fn_1_4FBC(&SceneOwner::getChecked());

    setRunnerState(this, &PlayerRunner::fn_1_7BD0);
}
void PlayerRunner::fn_1_7BD0() {
    Vec destination = this->spawn_position.value;
    destination.z += cmToWorld(400.0f);
    if (fn_8007E8B4(this->player, &destination, (f32)framesToSeconds(229.99999999999997), 10.0f)) {

        setRunnerState(this, &Player::fn_1_7060);
    }
}
void fn_1_7DAC(PlayerRunner *);
void fn_1_7E20(PlayerRunner *);
void fn_1_9218(PlayerRunner *);
u8 fn_1_7E90(PlayerRunner *);
u8 fn_1_82BC(PlayerRunner *);
void PlayerRunner::startRunning() {
    fn_8007E888(this->player);

    setRunnerState(this, &PlayerRunner::fn_1_7D2C);
}
void PlayerRunner::fn_1_7D2C() {
    if (this->respawnBlocked != 0) {
        fn_1_8CF4(this);
    } else {
        fn_1_7DAC(this);
        fn_1_7E20(this);
        fn_1_9218(this);
        if (fn_1_7E90(this) == 0) {
            if (fn_1_82BC(this) != 0) {
                return;
            }
        }
    }
}
extern void fn_1_8584(PlayerRunner *owner);
void fn_1_6F68(Player *owner, s32 mode);
extern s32 lbl_1_bss_A4;
void fn_1_6CD8(Player *owner, s32 motionIndex, u8 shift);
void fn_1_6B74(Player *owner, u32 motionFile);
void fn_1_7DAC(PlayerRunner *owner) {
    Point3d position;
    f32 minimumZ;
    fn_80086BC8(owner->player->actor, &position);
    minimumZ = -860.0f;
    if (position.z < minimumZ) {
        position.z = minimumZ;
        fn_80086B04(owner->player->actor, &position);
    }
}
void fn_1_7E20(PlayerRunner *owner) {
    u32 mask;
    Point3d position;
    fn_80086BC8(owner->player->actor, &position);
    mask = 1;
    if (position.z > -650.0) {
        mask |= 2;
    }
    fn_80080DD4(owner->player->actor, mask);
}
const Point3d lbl_1_rodata_368[2] = {{-163.86f, 215.07f, -12.11f}, {136.7f, 252.7f, 5.27f}};

u8 fn_1_7E90(PlayerRunner *owner) {
    Point3d position;
    s32 thresholdIndex;
    fn_80086BC8(owner->player->actor, &position);
    thresholdIndex = 0;
    while (thresholdIndex < 2) {
        const Point3d *threshold = &lbl_1_rodata_368[thresholdIndex];
        if (!(position.y >= threshold->y)) {
            if (!(sqrt(Square(position.x - threshold->x) + Square(position.z - threshold->z)) >
                  cmToWorld(100))) {
                f32 heightDifference = threshold->y - position.y;
                f32 modelHeight = 2.0 * fn_8005C4C8((s16)owner->character);
                f32 fade = Clamp(1.0 - heightDifference / modelHeight, 0.0, 1.0);
                owner->scale = owner->character == 6 ? fade : -1.0f;
                if (fade <= 0.0f) {
                    owner->scale = 1.0f;
                    owner->finish();
                    owner->scale = -1.0f;
                    fn_1_9078(owner, 0);
                    fn_1_8584(owner);
                    return 1;
                }
            }
        }
        thresholdIndex++;
    }
    if (position.y < -300.0f) {
        fn_1_8584(owner);
        return 1;
    }
    return 0;
}
u8 fn_1_82BC(PlayerRunner *self) {
    u32 collision_code = 0;
    if (fn_80086DD0(self->player->actor, &collision_code) == 0) {
        return 0;
    }
    if ((collision_code & 2) != 0) {
        if (lbl_1_bss_A4 < 1) {
            MinigameCore::addWinner(self->playerNo);
            self->hasWon = 1;
            lbl_1_bss_A4 += 1;
            fn_1_8B58(self);
        } else {
            fn_1_8CF4(self);
        }
        return 1;
    }
    return 0;
}
const Point3d lbl_1_rodata_3B8 = {701.0f, 0.0f, 87.0f};
static inline void launchPlayer(PlayerRunner *owner, Point3d &referencePosition,
                                Point3d &direction) {
    Point3d *directionVector;
    Point3d *scaleInput;
    f32 angle;
    f32 limit;
    f64 cosine;
    f64 sine;
    f64 targetAngle;
    f64 clampedAngle;
    PSVECSubtract(&owner->updated_position.value, &referencePosition, &direction);
    targetAngle = atan2((f64)direction.x, (f64)direction.z);
    if (targetAngle < -(M_PI * 40 / 180)) {
        clampedAngle = -(M_PI * 40 / 180);
    } else if (targetAngle > -(M_PI * 10 / 180)) {
        clampedAngle = -(M_PI * 10 / 180);
    } else {
        clampedAngle = targetAngle;
    }
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
    owner->rotation.y =
        (f32)(180.0 * ((M_PI + atan2((f64)owner->velocity.x, (f64)owner->velocity.z)) / M_PI));
    fn_1_8584(owner);
    fn_1_6F68(owner, 1);
}

void fn_1_837C(PlayerRunner *owner) {
    Point3d referencePosition = lbl_1_rodata_3B8;
    Point3d direction;
    launchPlayer(owner, referencePosition, direction);
}

// Descriptive name for the shared despawn operation.
static inline void beginDespawn(PlayerRunner *owner) {
    double currentZ;
    double currentX;
    PlayerControl *cleanupOwner;
    currentZ = owner->current_position.value.z;
    currentX = owner->current_position.value.x;
    owner->updated_position.value.x = currentX;
    owner->updated_position.value.y = 0.0f;
    owner->updated_position.value.z = currentZ;
    cleanupOwner = owner->cleanup_owner.value;
    cleanupOwner->reset();
    owner->despawned = 1;
    fn_80086E3C(owner->player);
    fn_8007EDBC(owner->player);
    fn_1_6CD8(owner, 0, 1);
    setRunnerState(owner, &PlayerRunner::fn_1_868C);
}
void fn_1_8584(PlayerRunner *owner) {
    beginDespawn(owner);
}
void PlayerRunner::fn_1_868C() {
    Vec delta;
    double deltaX;
    double deltaZ;
    f32 distance;
    bool timerExpired;
    bool distanceExceeded;
    s32 nextCountdown;
    this->rotation.x = (f32)((f64)this->rotation.x - framesToSeconds(60.0));
    timerExpired = this->current_position.value.y < -2000.0f;
    deltaZ = this->current_position.value.z - this->updated_position.value.z;
    deltaX = this->current_position.value.x - this->updated_position.value.x;
    delta.x = deltaX;
    delta.y = 0.0f;
    delta.z = deltaZ;
    distance = PSVECMag(&delta);
    distanceExceeded = distance > 3000.0f;
    if ((u8)timerExpired || (u8)distanceExceeded) {
        fn_1_9078(this, 0);
        fn_8005DFC0((s16)this->character, 0);
        if ((u8)timerExpired) {
            fn_1_6F68(this, 2);
        }
        nextCountdown = secondsToFrames(2L);
        this->countdown = nextCountdown;

        setRunnerState(this, &PlayerRunner::fn_1_8840);
    }
}
void PlayerRunner::fn_1_8840() {
    if (this->respawnBlocked == 0) {
        countDown();
        if (this->countdown <= 0) {
            if (fn_1_4FBC(&SceneOwner::getChecked())) {
                this->despawned = 0;
                fn_80086F00(this->player, &this->spawn_position.value);
                fn_8007ED60(this->player);
                fn_8008697C(this->player, &this->spawn_position.value);
                fn_80086CC4(this->player->actor, 0.0f);
                fn_8007E818(this->player);
                fn_1_9078(this, 1);
                fn_1_9188(this);

                setRunnerState(this, &PlayerRunner::fn_1_89C0);
            }
        }
    }
}
void PlayerRunner::fn_1_89C0() {
    Position target;
    f32 targetOffset;
    f32 chaseAngle;
    targetOffset = cmToWorld(400.0f);
    chaseAngle = (f32)framesToSeconds(229.99999999999997);
    target = this->spawn_position;
    target.value.z += targetOffset;
    this->current_position.value.z += chaseAngle;
    fn_80086B04(this->player->actor, &this->current_position.value);
    if (fn_8007E8B4(this->player, &target.value, chaseAngle, 10.0f) != 0 ||
        this->current_position.value.z > target.value.z) {
        fn_8007E888(this->player);
        if (this->respawnBlocked == 0) {

            setRunnerState(this, &PlayerRunner::fn_1_7D2C);
        } else {
            fn_1_8CF4(this);
        }
        fn_8005DFC0((s16)this->character, 1);
    }
}
void PlayerRunner::stopRespawning() {
    fn_80087158(this->player, 1);
    this->respawnBlocked = 1;
}
void fn_1_8B58(PlayerRunner *task) {
    fn_8007E818(task->player);

    setRunnerState(task, &PlayerRunner::fn_1_8BD8);
}
void PlayerRunner::fn_1_8BD8() {
    Vec actorPosition;
    Vec chasePosition;
    double chaseX;
    double chaseY;
    double chaseZ;
    MGACTOR *actor;
    actor = player->actor;
    fn_80086BC8(actor, &actorPosition);
    if (actorPosition.y < spawn_position.value.y) {
        actorPosition.y = spawn_position.value.y;
        fn_80086AA8(actor, &actorPosition);
        actor->colGroundAttr |= 2;
    }
    chaseZ = spawn_position.value.z + cmToWorld(400.0f);
    chaseY = spawn_position.value.y;
    chaseX = 800.0;
    chasePosition.x = (f32)chaseX;
    chasePosition.y = (f32)chaseY;
    chasePosition.z = (f32)chaseZ;
    if (fn_8007E8B4(player, &chasePosition, (f32)framesToSeconds(200.0), 10.0f) != 0) {
        fn_1_8CF4(this);
    }
}
void fn_1_8CF4(PlayerRunner *object) {
    fn_80087158(object->player, 1U);

    setRunnerState(object, &PlayerRunner::fn_1_8D78);
}
void PlayerRunner::fn_1_8D78() {
    Vec camera_position;
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
    Range<float> yaw_range(0.0f, 360.0f, actor_yaw);
    fn_80086CA8(this->player->actor, yaw_range.fn_1_9AE0(angle, 0.2));
}

void PlayerRunner::showResult() {
    s32 mode;
    if (this->hasWon != 0) {
        mode = 1;
    } else {
        mode = 2;
    }
    fn_1_6CD8(this, mode, 0);

    setRunnerState(this, &Player::fn_1_7060);
}

void PlayerRunner::finish() {
    s32 modelId;
    modelId = (s16)this->player->actor->mdlId;
    if (this->despawned == 0) {
        HuVecF previousPosition = this->current_position.value;
        fn_80086BC8(this->player->actor, &this->current_position.value);
        PSVECSubtract(&this->current_position.value, &previousPosition, &this->velocity);
        fn_1_97DC(this);
        fn_80086CB0(this->player->actor, &this->rotation.y);
        this->rotation.x = 0.0f;
        this->rotation.z = 0.0f;
        this->m_motion = -1;
        if (this->scale >= 0.0f) {
            fn_800268B0(modelId, this->scale, this->scale, this->scale);
        }
        soundPanCharacterVoice(1, (s16)this->character, &this->current_position.value);
        return;
    }
    PSVECAdd(&this->velocity, &this->current_position.value, &this->current_position.value);
    this->velocity.y -= gravity;
    fn_80026790(modelId, &this->current_position.value);
    soundPanCharacterVoice(1, (s16)this->character, &this->current_position.value);
    fn_80026838(modelId, &this->rotation);
}
void fn_1_9078(PlayerRunner *owner, u8 enabled) {
    if (enabled != 0) {
        fn_8005C47C((s16)owner->character, 1);
        return;
    }
    fn_8005C430((s16)owner->character, 1);
}
void PlayerRunner::update() {
    if (fn_1_911C(this) != 0) {
        fn_1_6F68(this, 0);
    }
}
inline u8 fn_1_911C(PlayerRunner *entry) {
    u32 collisionCode;
    if (entry->despawned != 0) {
        return 0U;
    }
    collisionCode = 0;
    fn_80086DD0(entry->player->actor, &collisionCode);
    return (u8)((collisionCode & 0x407F) != 0);
}
void fn_1_9188(PlayerRunner *owner) {
    s16 shadowModelId = (s16)owner->player->actor->mdlId;
    fn_80026E44(shadowModelId);
}
void fn_1_91D0(PlayerRunner *owner) {
    s16 shadowModelId = (s16)owner->player->actor->mdlId;
    fn_80026EF0(shadowModelId);
}
void fn_1_9218(PlayerRunner *owner) {
    Point3d *position = &owner->player->actor->pos;
    MGACTOR_COLMAP_POLY polygon;
    s32 sample;
    if (position->y < 0.0f) {
        s16 belowGroundModelId = (s16)owner->player->actor->mdlId;
        fn_80026EF0(belowGroundModelId);
        return;
    }
    if (fn_1_911C(owner) != 0) {
        s16 codeHitModelId = (s16)owner->player->actor->mdlId;
        fn_80026E44(codeHitModelId);
        return;
    }
    for (sample = 0; sample < 3; sample++) {
        f32 offset;
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

int fn_1_9484(COL_NARROW_PARAM *source, COL_NARROW_PARAM *operation) {
    PlayerRunner *target = (PlayerRunner *)fn_1_65B0(source->paramB);
    if (target == 0) {
        return 0;
    }
    if (operation->type == 0) {
        PlayerRunner *other = (PlayerRunner *)fn_1_65B0(operation->paramB);
        target->collisionPosition.x = other->current_position.value.x;
        target->collisionPosition.y = other->current_position.value.y;
        target->collisionPosition.z = other->current_position.value.z;
        target->collisionPending = 1;
        return 1;
    } else if (operation->type != 1) {
        return 0;
    } else {
        Point3d direction;
        Point3d referencePosition = lbl_1_rodata_3B8;
        launchPlayer(target, referencePosition, direction);

        soundPlayAt(1, 0x702, &target->current_position.value);
        return 1;
    }
}
void fn_1_97DC(PlayerRunner *owner) {
    HSF_OBJECT *collisionModel;
    f32 actorYaw;
    f32 modelYaw;
    f32 yawDifference;
    collisionModel = owner->player->actor->colObj;
    if (collisionModel != 0 && collisionModel == owner->lastCollisionModel) {
        modelYaw = collisionModel->mesh.curr.rot.y;
        Range<float> range(0.0f, 360.0f, 0.0f);
        if (owner->lastYaw >= 0.0f) {
            yawDifference = range.distance(owner->lastYaw, modelYaw);
            fn_80086CB0(owner->player->actor, &actorYaw);
            fn_80086CA8(owner->player->actor, actorYaw += yawDifference);
        }
        owner->lastCollisionModel = collisionModel;
        owner->lastYaw = range.wrap(modelYaw);
    } else {
        owner->lastCollisionModel = collisionModel;
        owner->lastYaw = -1.0f;
    }
}
void PlayerRunner::fn_1_9900() {
    const u32 NUMBEROF_MOTION = 3;
    m_motion_id.clear();
    fn_1_6B74(this, DATANUM(0x930000, 34));
    fn_1_6B74(this, DATANUM(0x930000, 6));
    fn_1_6B74(this, DATANUM(0x930000, 40));
    assert(m_motion_id.size() == NUMBEROF_MOTION);
    fn_8005B4BC((s16)character);
}
