#include "REL/framework/sound.h"
#include "REL/framework/sphere.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/event.h"
#include "REL/framework/random.h"
extern "C" {
#include <math.h>
#include "game/audio.h"
}
#include "REL/m672DLL/player.h"
#include "REL/framework/linknode.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "datadir_enum.h"
#include "game/hu3d.h"
#include "humath.h"
extern "C" {
#include "game/charman.h"
}
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "REL/m672DLL/resources.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/phase_durations.h"
#include <new>
#include "REL/m672DLL/gravity.h"

extern "C" {
#include "dolphin.h"
#include "game/animdata.h"
}

#include "REL/m672DLL/member_handler.h"

// Interpolates from start toward end along the shortest wrapped degree difference.
f32 fn_1_BD40(f32 start, f32 end, f32 t)
{
    f32 delta;
    f32 result;
    delta = fmod(end - start, 360.0);
    if (delta < 0.0f) {
        delta += 360.0f;
    }
    if (delta > 180.0f) {
        delta -= 360.0f;
    }
    result = fmod(start + t * delta, 360.0);
    if (result < 0.0f) {
        result += 360.0f;
    }
    return result;
}

// Converts the direction's x/z atan2 result from radians to degrees.
f32 fn_1_BE44(const Vec *direction)
{
    return (180.0 * atan2(direction->x, direction->z)) / M_PI;
}

void fn_1_EA98(Player *, u32, u8);

// Creates a computer or pad controller according to the selected player's type.
PlayerController *fn_1_BE9C(Player *owner, int playerNo)
{
    if (lbl_80265728[playerNo].type != 0) {
        return new ComController(owner, lbl_80265728[playerNo].comDif);
    }

    return new PadController(owner);
}

// Initializes player callbacks/controllers, actor/model state, effects, and the floor position.
Player::Player(int playerNo, const EffectVector *position)
    : GameObject(1), playerNo(playerNo), controller(0), active(0),
      finished(0), movementElapsed(0.0f), ringParticles(0), groundSprite(0),
      model(new ModelMotion(DATANUM(0x830000, 9))), origin(*position), callbacks(0),
      currentMotionIndex(-1), motionIds(), materialColors()
{

    MGACTOR_PARAM actorParam;
    EffectVector floorPosition;
    Resources *resources;
    registerMemberHandler<Player>(&this->callbacks,
        bindMember(std::mem_fun(&Player::fn_1_EB5C), this), 0);
    registerMemberHandler<Player>(&this->callbacks,
        bindMember(std::mem_fun(&Player::fn_1_EBF4), this), 2);
    registerMemberHandler<Player>(&this->callbacks,
        bindMember(std::mem_fun(&Player::fn_1_EC80), this), 3);
    registerMemberHandler<Player>(&this->callbacks,
        bindMember(std::mem_fun(&Player::fn_1_ED84), this), 4);
    this->controller.reset(fn_1_BE9C(this, playerNo));
    actorParam.height = 150.0f;
    actorParam.radius = 60.000004f;
    actorParam.param = playerNo;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.correctHookParam = 0;
    actorParam.narrowHook = 0;
    actorParam.correctHook = 0;
    static unsigned int lbl_1_data_4B4[] = {
        DATANUM(0x930000, 21),
        DATANUM(0x930000, 118),
        DATANUM(0x930000, 118),
        DATANUM(0x930000, 3),
        DATANUM(0x930000, 4),
        DATANUM(0x930000, 5),
        DATANUM(0x930000, 7),
        DATANUM(0x930000, 8),
        DATANUM(0x930000, 10),
        DATANUM(0x930000, 9),
        DATANUM(0x930000, 15),
        DATANUM(0x930000, 16),
        DATANUM(0x930000, 17),
        DATANUM(0x930000, 18),
        DATANUM(0x930000, 22),
        DATANUM(0x930000, 22),
        0,
    };
    this->player = fn_80080728(playerNo, &actorParam, 8, 1, 1, lbl_1_data_4B4);
    this->modelId = this->player->actor->mdlId;
    this->charNo = this->player->charNo;
    fn_80026B8C(this->modelId, 3);
    fn_8005DDEC(4);
    fn_8005DFC0(this->charNo, 0);
    fn_80087158(this->player, (1 << 0));
    fn_1_E960();
    this->rotation = 180.0f;
    this->position.value.x = 0.0f;
    this->position.value.y = 0.0f;
    this->position.value.z = 0.0f;
    this->velocity.value.x = 0.0f;
    this->velocity.value.y = 0.0f;
    this->velocity.value.z = 0.0f;
    fn_80086AA8(this->player->actor, &this->position.value);
    fn_80086CA8(this->player->actor, this->rotation);
    fn_80026808(this->modelId, 0.0f, this->rotation, 0.0f);
    fn_80086E3C(this->player);
    fn_1_EA98(this, 0, 1);
    fn_1_D8CC();
    this->modelYaw = 0.0f;
    this->model->setPosition(&this->origin.value);
    this->model->setRotationY(this->modelYaw);
    this->model->setLayer(5);
    this->model->setAttribute(((1 << 30) | (1 << 1)));
    fn_8002741C(this->model->id(), "itemhook_char", this->modelId);
    resources = &Resources::getChecked();
    this->groundSprite.reset(new GroundSprite(resources->animation(1), &this->origin.value));
    this->groundSprite->setAlpha(0.0f);
    this->groundSprite->setRadius(200.0f);
    fn_1_8E0C(this->groundSprite.get(), 1);
    floorPosition = this->origin;
    floorPosition.value.y = 0.0f;
    this->ringParticles.reset(new RingParticles(resources->animation(3),
                                           resources->animation(0), 10, 2,
                                           floorPosition));
    this->soundFrame = 0;
    this->soundHandle = -1;
    this->setUpdate(&Player::fn_1_DC00);
}

extern "C" {
extern const Point3d lbl_1_rodata_58C;
extern const Point3d lbl_1_rodata_598[4];

}

// Returns the collision object's state field.
static inline s32 CollisionState(CollisionObject *object) { return object->state; }
// Returns the collision object's active field.
static inline u8 CollisionActive(CollisionObject *object) { return object->active; }
// Returns the collision object's radius field.
static inline f32 CollisionRadius(CollisionObject *object) { return object->radius; }

// Returns the target point selected by the player's index.
static inline const Point3d *PlayerTarget(Player *work)
{
    return &lbl_1_rodata_598[work->playerNo];
}

// Applies the player's position and rotation to the actor and model.
static inline void PlaceAtPosition(Player *work)
{
    fn_80086AA8(work->player->actor, &work->position.value);
    fn_80086CA8(work->player->actor, work->rotation);
    fn_80026790(work->modelId, &work->position.value);
    fn_80026808(work->modelId, 0.0f, 180.0f, 0.0f);
}

// Releases the actor through the game actor cleanup routine.
Player::~Player()
{
    fn_80080F38(player->actor);
}
// Copies actor position into the player, updates the controller, and runs the frame callback.
void Player::update()
{
    fn_80086BC8(player->actor, &position.value);
    PlayerController *controllerValue = controller.get();
    fn_1_758(controllerValue);
    if (frameUpdate) {
        (this->*frameUpdate)();
    }
}

// Clamps the player z position to -200 and updates actor/model position when needed.
void Player::fn_1_CEC0()
{
    if (!(position.value.z < -200.0f)) {
        position.value.z = -200.0f;
        fn_80026790(modelId, &position.value);
        fn_80086B04(player->actor, &position.value);
    }
}

// Separates the player from active state-1 collision objects that overlap its radius.
void fn_1_CF2C(Player *work)
{
    EffectVector adjustedPosition;
    f32 distance;
    f32 minimumDistance;
    s32 objectCount = collisionShapes.size();
    s32 index;

    adjustedPosition = work->position;
    adjustedPosition.value.y += 50.0f;
    for (index = 0; index < objectCount; ++index) {
        CollisionObject *object = (CollisionObject *)collisionShapes.at(index);
        s32 state = CollisionState(object);
        if (CollisionActive(object) != 0 && state == 1) {
            EffectVector otherPosition = object->position;
            EffectVector fullCorrection;
            EffectVector correction;
            Point3d separation;
            PSVECSubtract(&adjustedPosition.value, &otherPosition.value, &separation);
            distance = vectorMagnitude(&separation);
            minimumDistance = 60.000004f + CollisionRadius(object);
            if (!(distance >= minimumDistance)) {
                correction.setScaled(&separation,
                                     (minimumDistance - distance) / distance);
                // The full correction is still passed through the scale operation.
                fullCorrection.setScaled(&correction.value, 1.0f);
                PSVECAdd(&fullCorrection.value, &adjustedPosition.value,
                         &adjustedPosition.value);
                work->position.set( adjustedPosition.value.x,
                               work->position.value.y, adjustedPosition.value.z);
                fn_80086B04(work->player->actor, &work->position.value);
            }
        }
    }
}

// Activates the player once it reaches z <= -2880 and selects the entrance update.
void Player::fn_1_D120()
{
    if (position.value.z > -2880.0f) {
        return;
    }

    fade = 0.0f;
    phase = 0;
    fn_80086E3C(player);
    fn_1_EA98(this, 2, 1);
    active = 1;
    fn_80026808(modelId, 0.0f,
                    180.0f, 0.0f);
    fn_80026FCC(modelId);
    sendEvent(1, 2, 0.0f);

    setUpdate(&Player::fn_1_D21C);
}
extern "C" {
const Point3d lbl_1_rodata_58C = {-1200.0f,549.0f,-1540.0f};
const Point3d lbl_1_rodata_598[4] =
    {{-950.0f,549.0f,-1390.0f},{-900.0f,549.0f,-1465.0f},
     {-850.0f,549.0f,-1615.0f},{-800.0f,549.0f,-1690.0f}};
}

// Drives the player entrance phases, fade values, path movement, and final rotation.
void Player::fn_1_D21C()
{
    // All four player-indexed rotation targets use the same 22.5 degree value.
    static const f32 lbl_1_rodata_5C8[4] = {22.5f,22.5f,22.5f,22.5f};
    Point3d direction;

    f32 fadeOut;
    f32 fadeIn;
    f32 magnitude;

    switch (phase) {
    case 3:
        break;
    case 0:
        position.value.z -= 8.333333333333334;
        fn_80026790(modelId, &position.value);
        fn_80086B04(player->actor, &position.value);
        fadeOut = (3800.0f + position.value.z) / 1200.0f;
        if (fadeOut > 1.0f) fadeOut = 1.0f;
        fn_1_DA64(fadeOut, fadeOut, fadeOut);
        if (position.value.z > -3800.0f) break;
        position.value.x = lbl_1_rodata_58C.x;
        position.value.y = lbl_1_rodata_58C.y;
        position.value.z = lbl_1_rodata_58C.z;
        rotation = 90.0f;
        fn_80026808(modelId, 0.0f, 90.0f, 0.0f);
        fn_1_EA98(this, 1, 1);
        ++phase;
    case 1:
        PSVECSubtract(PlayerTarget(this), &position.value, &direction);
        magnitude = vectorMagnitude(&direction);
        if (magnitude > 0.0f) PSVECNormalize(&direction, &direction);
        rotation = fn_1_BE44(&direction);
        fn_80026808(modelId, 0.0f, rotation, 0.0f);
        position.value.x += 3.3333333333333335 * direction.x;
        position.value.z += 3.3333333333333335 * direction.z;
        fadeIn = (position.value.x - lbl_1_rodata_58C.x < 0.0f ?
                  -(position.value.x - lbl_1_rodata_58C.x) :
                  position.value.x - lbl_1_rodata_58C.x) /
                 (1000.0f + lbl_1_rodata_58C.x < 0.0f ?
                  -(1000.0f + lbl_1_rodata_58C.x) :
                  1000.0f + lbl_1_rodata_58C.x);
        if (fadeIn > 1.0f) fadeIn = 1.0f;
        fn_1_DA64(fadeIn, fadeIn, fadeIn);
        fn_80026790(modelId, &position.value);
        fn_80086B04(player->actor, &position.value);
        if (position.value.x < lbl_1_rodata_598[playerNo].x) break;
        ++phase;
    case 2:
        rotation = fn_1_BD40(rotation,
                                           lbl_1_rodata_5C8[playerNo], 0.2f);
        fn_80026808(modelId, 0.0f, rotation, 0.0f);
        if ((lbl_1_rodata_5C8[playerNo] - rotation < 0.0f ?
             -(lbl_1_rodata_5C8[playerNo] - rotation) :
             lbl_1_rodata_5C8[playerNo] - rotation) > 1.0f) break;
        rotation = lbl_1_rodata_5C8[playerNo];
        fn_80026808(modelId, 0.0f, rotation, 0.0f);
        fn_1_EA98(this, 0, 1);
        setUpdate(&Player::fn_1_DBF4);
        ++phase;
        break;
    }
}
// Saves each model material's first three color bytes into materialColors.
void Player::fn_1_D8CC()
{
    HU3D_MODEL *model = &lbl_802C01C8[modelId];
    HSF_MATERIAL *material = model->hsf->material;
    s32 materialCount = model->hsf->materialNum;
    s32 materialIndex = 0;

    for (; materialIndex < materialCount; materialIndex++, material++) {
        VectorElement color;

        color.red = material->color[0];
        color.green = material->color[1];
        color.blue = material->color[2];
        materialColors.push_back(color);
    }
}
// Restores saved material colors and sets all model ambient channels to one.
void Player::fn_1_D97C()
{
    HU3D_MODEL *model = &lbl_802C01C8[modelId];
    HSF_MATERIAL *material = model->hsf->material;
    s32 materialCount = model->hsf->materialNum;
    s32 materialIndex = 0;

    for (; materialIndex < materialCount; materialIndex++, material++) {
        VectorElement color = materialColors.at(materialIndex);
        material->color[0] = color.red;
        material->color[1] = color.green;
        material->color[2] = color.blue;
    }

    model->ambR = 1.0f;
    model->ambG = 1.0f;
    model->ambB = 1.0f;
}
// Scales saved material colors by RGB inputs and writes the same values to ambient channels.
void Player::fn_1_DA64(f32 redScale, f32 greenScale, f32 blueScale)
{
    HU3D_MODEL *model;
    HSF_MATERIAL *material;
    s32 materialCount;
    s32 materialIndex;

    model = &lbl_802C01C8[modelId];
    material = model->hsf->material;
    materialCount = model->hsf->materialNum;
    materialIndex = 0;

    for (; materialIndex < materialCount; materialIndex++, material++) {
        VectorElement color = materialColors.at(materialIndex);
        material->color[0] = (u8)(color.red * redScale);
        material->color[1] = (u8)(color.green * greenScale);
        material->color[2] = (u8)(color.blue * blueScale);
    }

    model->ambR = redScale;
    model->ambG = greenScale;
    model->ambB = blueScale;
}
// Marks the player finished.
void Player::fn_1_DBF4() { finished = 1; }
// Empty player update callback.
void Player::fn_1_DC00() {}
// Advances the entrance state machine, model/groundSprite opacity, and sound timing.
void Player::fn_1_DC04()
{
    EffectVector movement;
    f32 opacity;
    f32 fadeIn;
    s16 pitch;
    s16 risingPitch;

    switch (state) {
    case 0:
        if (elapsed < delay) break;
        model.get()->resetAttribute(((1 << 30) | (1 << 1)));
        fn_1_48BC(ringParticles.get());
        elapsed = 0.0f;
        ++state;
    case 1:
        if (elapsed < 1.0f) break;
        elapsed = 0.0f;
        fn_1_EA98(this, 3, 1);
        ++state;
    case 2:
        if (elapsed < 1.0f) break;
        elapsed = 0.0f;
        ++state;
    case 3:
        fn_80027508(model.get()->id());
        rotation = 180.0f;
        position.value.x = origin.value.x;
        position.value.y = origin.value.y;
        position.value.z = origin.value.z;
        position.value.y += 278.7f;
        PlaceAtPosition(this);
        fn_80026E44(modelId);
        velocity.value.y = -3.3333333f;
        ++state;
    case 4:
        movement = velocity;
        velocity.value.y += 5.0f * -gravityPerFrame;
        movement.value.y = 0.5f * (movement.value.y + velocity.value.y);
        PSVECAdd(&movement.value, &position.value, &position.value);
        PlaceAtPosition(this);
        if (position.value.y > 0.0f) break;
        fn_8006A030((s16)playerNo, 45, 7, 3);
        fn_80086F00(player, &position.value);
        fn_1_EA98(this, 4, 1);
        fn_80086AA8(player->actor, &position.value);
        fn_80086CA8(player->actor, rotation);
        fn_80026808(modelId, 0.0f, 180.0f, 0.0f);
        fn_8005DFC0(charNo, 1);
        fn_1_48C8(ringParticles.get());
        ++state;
    case 5:
        if (fn_8002D75C(model.get()->id())) {
            fn_800269E8(model.get()->id(), 1);
            groundSprite.reset();
            ++state;
        }
        break;
    }
    if (state >= 3) {
        if (elapsed < 0.5f) {
            opacity = 0.4f * (1.0f - elapsed / 0.5f);
            groundSprite->setAlpha(opacity);
        }
    } else if (state == 1) {
        fadeIn = (f64)0.4f * elapsed;
        groundSprite->setAlpha(fadeIn);
    }
    if (state > 0) {
        if (soundFrame == 0)
            soundHandle = soundPlayAt(1, 0x8A5, &position.value);
        if (soundFrame == 121)
            soundPlayAt(1, 0x8A6, &position.value);
        if (soundFrame < 140) {
            pitch = 8191.0f * (1.0f - 0.5f * ((f32)soundFrame / 140.0f));
            soundSetPitch(soundHandle, pitch);
        }
        if (soundFrame > 150) {
            risingPitch = 8191.0f * (0.5f * ((f32)(soundFrame - 150) / 87.0f));
            soundSetPitch(soundHandle, risingPitch);
        }
        if (soundFrame > 236 && soundHandle != -1) {
            HuAudFXStop(soundHandle);
            soundHandle = -1;
        }
        ++soundFrame;
    }
    elapsed += (1.0 / 60.0);
}
// Resolves collisions, checks entrance activation, and clamps the player position.
void Player::fn_1_E29C()
{
    fn_1_CF2C(this);
    fn_1_D120();
    fn_1_CEC0();
}

// Applies vertical motion and a z offset, then marks the player finished after two seconds.
void Player::fn_1_E5EC()
{
    f32 movement = velocity.value.y;

    velocity.value.y += -gravityPerFrame;
    movement = 0.5f * (movement + velocity.value.y);
    position.value.z += 30.0;
    position.value.y += movement;
    fn_80026790(modelId, &position.value);
    fn_80086B04(player->actor, &position.value);

    movementElapsed += (1.0 / 60.0);
    if (!(movementElapsed < 2.0f)) {
        fn_800269E8(modelId, 1);

        setUpdate(&Player::fn_1_DBF4);
    }
}
// Searches active state-2 collision objects and starts the hit response on overlap.
void Player::fn_1_E718()
{
    EffectVector adjustedPosition;
    EffectVector objectPosition;
    Point3d separation;
    f32 distance;
    f32 minimumDistance;
    s32 found = 0;
    s32 objectCount = collisionShapes.size();
    s32 index;

    adjustedPosition = position;
    adjustedPosition.value.y += 50.0f;
    for (index = 0; index < objectCount; ++index) {
        CollisionObject *object = (CollisionObject *)collisionShapes.at(index);
        s32 state = CollisionState(object);

        if (CollisionActive(object) != 0 && state == 2) {
            objectPosition = object->position;
            PSVECSubtract(&adjustedPosition.value, &objectPosition.value, &separation);
            distance = vectorMagnitude(&separation);
            minimumDistance = 60.000004f + CollisionRadius(object);
            if (distance >= minimumDistance) {
                continue;
            }
            found = 1;
            break;
        }
    }

    if ((u8)found != 0) {
        CharFXPlay(charNo, 0x240);
        fn_1_EA98(this, 6, 1);
        velocity.set( 0.0,
                       200.0f * gravityPerFrame, 0.0);
        sendEvent(1, 3, 0.0f);
        fn_8006A030((s16)playerNo, 0x2D, 7, 3);
        setUpdate(&Player::fn_1_E5EC);
    }
}
// Empty player callback used after the winner motion.
void Player::fn_1_E95C() {}
// Clears and repopulates the player's motion list, then prepares character motions.
void Player::fn_1_E960() {
    motionIds.clear();
    fn_1_EA24(DATANUM(0x930000, 0));
    fn_1_EA24(DATANUM(0x930000, 1));
    fn_1_EA24(DATANUM(0x930000, 2));
    fn_1_EA24(DATANUM(0x930000, 21));
    fn_1_EA24(DATANUM(0x930000, 22));
    fn_1_EA24(DATANUM(0x930000, 118));
    fn_1_EA24(DATANUM(0x930000, 20));
    fn_1_EA24(DATANUM(0x930000, 38));
    fn_1_EA24(DATANUM(0x930000, 40));
    fn_8005B4BC(charNo);
}
// Resolves one character motion and appends its model id to motionIds.
void Player::fn_1_EA24(s32 dataNum)
{
    s16 motionId = fn_8005B0B8(charNo, (u32)dataNum);
    motionIds.push_back(motionId);
}

// Shifts the model to a new indexed motion when motionId differs from the current index.
void fn_1_EA98(Player *object, u32 motionId, u8 loop)
{
    s16 motion;

    if (object->currentMotionIndex != motionId) {
        motion = object->motionIds.at(motionId);
        fn_800269E8(object->modelId, ((1 << 30) | (1 << 1)));
        fn_8005C2C8(object->charNo, motion,
                           0.0f, 8.0f, loop != 0 ? ((1 << 30) | (1 << 0)) : 0);
        fn_8005C390(object->charNo, 1.0f);
        object->currentMotionIndex = motionId;
    }
}
// Resets entrance state/timing, chooses a random delay, and selects the entrance update.
void Player::fn_1_EB5C(EventMessage *)
{

    state = 0;
    elapsed = 0.0f;
    delay = RandomFloat(0.0f, 1.0f);
    setUpdate(&Player::fn_1_DC04);
}
// Enables player input, resets the controller, and selects the collision update.
void Player::fn_1_EBF4(EventMessage *)
{

    fn_80087168(player, 1);
    controller->reset();
    setUpdate(&Player::fn_1_E29C);
}
// Stops actor motion and, when inactive, starts motion 4 and the collision search.
void Player::fn_1_EC80(EventMessage *)
{

    fn_80086E3C(player);
    if (active == 0) {
        // The index is reset immediately before fn_1_EA98's subsequent motion comparison.
        currentMotionIndex = 0xFFFFFFFF;
        fn_1_EA98(this, 4, 1);
        setUpdate(&Player::fn_1_E718);
    }
}
// Stops actor motion and, when active, plays motion 7, records the winner, and idles.
void Player::fn_1_ED84(EventMessage *)
{

    fn_80086E3C(player);
    if (active != 0) {
        fn_1_EA98(this, 7, 0);
        MinigameCore::addWinner(playerNo);
        setUpdate(&Player::fn_1_E95C);
    }
}
