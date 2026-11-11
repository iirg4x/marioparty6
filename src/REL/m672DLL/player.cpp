#include "REL/framework/sound.h"
#include "REL/framework/minigame_core.h"
#include "REL/m672DLL/event.h"
#include "REL/m672DLL/random.h"
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
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/phase_durations.h"
#include <new>
extern "C" float lbl_1_data_480 = 980.665f;

extern "C" {
#include "dolphin.h"
#include "game/animdata.h"
}

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

f32 fn_1_BE44(const Vec *direction)
{
    return (180.0 * atan2(direction->x, direction->z)) / M_PI;
}

extern "C" {
void *__nw__FUl(unsigned long);
PolyNode *__ct__8PolyNodeFi(void *, int);
ModelMotion *__ct__11ModelMotionFUi(ModelMotion *, int);
EventHandlerManager *fn_1_198E0(EventHandlerManager *, int);
PlayerController *fn_1_C90(PlayerController *, Player *, int);
PlayerController *fn_1_7C8(PlayerController *, Player *);
}
void fn_1_8E0C(s16 *, u8);
void fn_1_EA98(Player *, u32, u8);
template<class Receiver> void fn_1_EF74(EventHandlerManager *, const PlayerCallbackBinding &, int);
extern "C" {
void setPosition__5ModelFP3Vec(ModelMotion *, const Vec *);
void setRotationY__5ModelFf(ModelMotion *, float);
void setLayer__5ModelFs(ModelMotion *, short);
void setAttribute__5ModelFUl(ModelMotion *, unsigned int);
GroundSprite *fn_1_8CC8(GroundSprite *, ANIMDATA *, const Vec *);
void __msl_assertion_failed(const char *, const char *, int);

extern const unsigned char lbl_1_data_588[];
extern PlayerHandler lbl_1_data_484, lbl_1_data_490, lbl_1_data_49C;
extern PlayerHandler lbl_1_data_4A8, lbl_1_data_4F8;
}
static inline PlayerMemberFunction PlayerMember(PlayerHandler value)
{
    return PlayerMemberFunction(value);
}

static inline PlayerCallbackBinding BindPlayer(const PlayerMemberFunction &member, Player *owner)
{
    return PlayerCallbackBinding(member, owner);
}

PlayerController *fn_1_BE9C(Player *owner, int playerNo)
{
    if (lbl_80265728[playerNo].type != 0) {
        return new ComController(owner, lbl_80265728[playerNo].comDif);
    }

    return new PadController(owner);
}

Player::Player(int playerNo, const WorldPosition *position)
    : GameObject(1), playerNo(playerNo), controller(0), active(0),
      unknown54(0), movementElapsed(0.0f), sequence(0), effect(0),
      model(new ModelMotion(DATANUM(0x830000, 9))), origin(*position), callbacks(0),
      currentMotionIndex(-1), firstList(), secondList()
{

    MGACTOR_PARAM actorParam;
    WorldPosition floorPosition;
    Resources *resources;
    fn_1_EF74<Player>(&this->callbacks, BindPlayer(PlayerMember(&Player::fn_1_EB5C), this), 0);
    fn_1_EF74<Player>(&this->callbacks, BindPlayer(PlayerMember(&Player::fn_1_EBF4), this), 2);
    fn_1_EF74<Player>(&this->callbacks, BindPlayer(PlayerMember(&Player::fn_1_EC80), this), 3);
    fn_1_EF74<Player>(&this->callbacks, BindPlayer(PlayerMember(&Player::fn_1_ED84), this), 4);
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
    this->modelScale = 0.0f;
    setPosition__5ModelFP3Vec(this->model, &this->origin.value.value);
    setRotationY__5ModelFf(this->model, this->modelScale);
    setLayer__5ModelFs(this->model, 5);
    setAttribute__5ModelFUl(this->model, ((1 << 30) | (1 << 1)));
    fn_8002741C(this->model->id(), "itemhook_char", this->modelId);
    resources = &Resources::getChecked();
    this->effect.reset(new GroundSprite(resources->animation(1), &this->origin.value.value));
    this->effect->setAlpha(0.0f);
    this->effect->setRadius(200.0f);
    fn_1_8E0C((s16 *)(void *)this->effect.get(), 1);
    floorPosition = this->origin;
    floorPosition.value.value.y = 0.0f;
    this->sequence.reset(new RingParticles(resources->animation(3),
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

extern void fn_1_EA98(Player *, u32, u8);
extern "C" {
extern std::vector<CollisionShape *> lbl_1_bss_C;

}

static inline CollisionShape *&CollisionShapeAt(unsigned long index)
{
    return lbl_1_bss_C.at(index);
}
static inline s32 CollisionState(CollisionObject *object) { return object->state; }
static inline u8 CollisionActive(CollisionObject *object) { return object->active; }
static inline f32 CollisionRadius(CollisionObject *object) { return object->radius; }
static inline f32 SeparationLength(Point3d *value) { return PSVECMag(value); }
static inline void SetPoint(Point3d *position, double x, double y, double z)
{
    position->x = x;
    position->y = y;
    position->z = z;
}

extern "C" {
extern void resetAttribute__5ModelFUl(ModelMotion *,u32);

}

static inline const Point3d *PlayerTarget(Player *work)
{
    return &lbl_1_rodata_598[work->playerNo];
}
static inline f32 DirectionLength(Point3d *vector)
{
    return PSVECMag(vector);
}

extern "C" {
/* Position and offset vectors separated by an unknown interval. */
typedef struct Position {
    HuVecF position;
    unsigned char unk_0C[12]; /* No code names this field. */
    HuVecF offset;
} Position;

extern s32 lbl_1_data_F18;
extern float lbl_1_bss_1BC;
extern s8 lbl_1_bss_218, lbl_1_bss_219, lbl_1_data_2758;
s32 fn_1_367CC(s32 *player);

extern void fn_1_48BC(RingParticles *);
extern void fn_1_48C8(RingParticles *);

float lbl_1_bss_5C = 0.016666666666666666 * (0.016666666666666666 * lbl_1_data_480);

}

static inline s16 MotionModelId(ModelMotion *model) { return (s16)model->modelId; }
static inline ModelMotion *PlayerMotionModel(Player *work) { return work->model.pointer; }
static inline void SetGroundAlpha(GroundSprite *effect, f32 opacity)
{
    effect->alpha = opacity;
}
static inline void PlaceAtPosition(Player *work)
{
    fn_80086AA8(work->player->actor, &work->position.value);
    fn_80086CA8(work->player->actor, work->rotation);
    fn_80026790(work->modelId, &work->position.value);
    fn_80026808(work->modelId, 0.0f, 180.0f, 0.0f);
}

extern void fn_1_EA98(Player *work, u32 mode, u8 state);
extern "C" {

}

void fn_1_758(PlayerController *);
Player::~Player()
{
    fn_80080F38(player->actor);
}
void Player::update()
{
    fn_80086BC8(player->actor, &position.value);
    PlayerController *controllerValue = controller.get();
    fn_1_758(controllerValue);
    if (frameUpdate) {
        (this->*frameUpdate)();
    }
}

void Player::fn_1_CEC0()
{
    if (!(position.value.z < -200.0f)) {
        position.value.z = -200.0f;
        fn_80026790(modelId, &position.value);
        fn_80086B04(player->actor, &position.value);
    }
}

void fn_1_CF2C(Player *work)
{
    VectorValue adjustedPosition;
    f32 distance;
    f32 minimumDistance;
    s32 objectCount = lbl_1_bss_C.size();
    s32 index;

    adjustedPosition = work->position;
    adjustedPosition.value.y += 50.0f;
    for (index = 0; index < objectCount; ++index) {
        CollisionObject *object = (CollisionObject *)CollisionShapeAt(index);
        s32 state = CollisionState(object);
        if (CollisionActive(object) != 0 && state == 1) {
            EffectVector otherPosition = object->position;
            VectorValue fullCorrection;
            VectorValue correction;
            Point3d separation;
            PSVECSubtract(&adjustedPosition.value, &otherPosition.value, &separation);
            distance = SeparationLength(&separation);
            minimumDistance = 60.000004f + CollisionRadius(object);
            if (!(distance >= minimumDistance)) {
                correction.setScaled(&separation,
                                     (minimumDistance - distance) / distance);
                fullCorrection.setScaled(&correction.value, 1.0f);
                PSVECAdd(&fullCorrection.value, &adjustedPosition.value,
                         &adjustedPosition.value);
                SetPoint(&work->position.value, adjustedPosition.value.x,
                               work->position.value.y, adjustedPosition.value.z);
                fn_80086B04(work->player->actor, &work->position.value);
            }
        }
    }
}

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
    {
        PlayerUpdate movement;
        movement = &Player::fn_1_D21C;
        frameUpdate = movement;
    }
}
extern "C" {
const Point3d lbl_1_rodata_58C = {-1200.0f,549.0f,-1540.0f};
const Point3d lbl_1_rodata_598[4] = {{-950.0f,549.0f,-1390.0f},{-900.0f,549.0f,-1465.0f},{-850.0f,549.0f,-1615.0f},{-800.0f,549.0f,-1690.0f}};
}

void Player::fn_1_D21C()
{
    static const f32 lbl_1_rodata_5C8[4] = {22.5f,22.5f,22.5f,22.5f};
    Point3d direction;
    PlayerUpdate movement;
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
        magnitude = DirectionLength(&direction);
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
        movement = &Player::fn_1_DBF4;
        frameUpdate = movement;
        ++phase;
        break;
    }
}
void Player::fn_1_D8CC()
{
    HU3D_MODEL *model = &lbl_802C01C8[modelId];
    HSF_MATERIAL *material = model->hsf->material;
    s32 materialCount = model->hsf->materialNum;
    s32 materialIndex = 0;

    for (; materialIndex < materialCount; materialIndex++, material++) {
        VectorElement color;

        color.byte0 = material->color[0];
        color.byte1 = material->color[1];
        color.byte2 = material->color[2];
        secondList.push_back(color);
    }
}
void Player::fn_1_D97C()
{
    HU3D_MODEL *model = &lbl_802C01C8[modelId];
    HSF_MATERIAL *material = model->hsf->material;
    s32 materialCount = model->hsf->materialNum;
    s32 materialIndex = 0;

    for (; materialIndex < materialCount; materialIndex++, material++) {
        VectorElement color = secondList.at(materialIndex);
        material->color[0] = color.byte0;
        material->color[1] = color.byte1;
        material->color[2] = color.byte2;
    }

    model->ambR = 1.0f;
    model->ambG = 1.0f;
    model->ambB = 1.0f;
}
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
        VectorElement color = secondList.at(materialIndex);
        material->color[0] = (u8)(color.byte0 * redScale);
        material->color[1] = (u8)(color.byte1 * greenScale);
        material->color[2] = (u8)(color.byte2 * blueScale);
    }

    model->ambR = redScale;
    model->ambG = greenScale;
    model->ambB = blueScale;
}
void Player::fn_1_DBF4() { unknown54 = 1; }
void Player::fn_1_DC00() {}
void Player::fn_1_DC04()
{
    VectorValue movement;
    f32 opacity;
    f32 fadeIn;
    s16 volume;
    s16 risingVolume;

    switch (state) {
    case 0:
        if (elapsed < delay) break;
        resetAttribute__5ModelFUl(PlayerMotionModel(this), ((1 << 30) | (1 << 1)));
        fn_1_48BC(sequence.get());
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
        fn_80027508(MotionModelId(PlayerMotionModel(this)));
        rotation = 180.0f;
        position.value.x = origin.value.value.x;
        position.value.y = origin.value.value.y;
        position.value.z = origin.value.value.z;
        position.value.y += 278.7f;
        PlaceAtPosition(this);
        fn_80026E44(modelId);
        velocity.value.y = -3.3333333f;
        ++state;
    case 4:
        movement = velocity;
        velocity.value.y += 5.0f * -lbl_1_bss_5C;
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
        fn_1_48C8(sequence.get());
        ++state;
    case 5:
        if (fn_8002D75C(MotionModelId(PlayerMotionModel(this)))) {
            fn_800269E8(MotionModelId(PlayerMotionModel(this)), 1);
            effect.reset();
            ++state;
        }
        break;
    }
    if (state >= 3) {
        if (elapsed < 0.5f) {
            opacity = 0.4f * (1.0f - elapsed / 0.5f);
            SetGroundAlpha(effect.get(), opacity);
        }
    } else if (state == 1) {
        fadeIn = (f64)0.4f * elapsed;
        SetGroundAlpha(effect.get(), fadeIn);
    }
    if (state > 0) {
        if (soundFrame == 0)
            soundHandle = soundPlayAt(1, 0x8A5, &position.value);
        if (soundFrame == 121)
            soundPlayAt(1, 0x8A6, &position.value);
        if (soundFrame < 140) {
            volume = 8191.0f * (1.0f - 0.5f * ((f32)soundFrame / 140.0f));
            soundSetPitch(soundHandle, volume);
        }
        if (soundFrame > 150) {
            risingVolume = 8191.0f * (0.5f * ((f32)(soundFrame - 150) / 87.0f));
            soundSetPitch(soundHandle, risingVolume);
        }
        if (soundFrame > 236 && soundHandle != -1) {
            HuAudFXStop(soundHandle);
            soundHandle = -1;
        }
        ++soundFrame;
    }
    elapsed += 0.016666666666666666;
}
void Player::fn_1_E29C()
{
    fn_1_CF2C(this);
    fn_1_D120();
    fn_1_CEC0();
}

void Player::fn_1_E5EC()
{
    f32 movement = velocity.value.y;

    velocity.value.y += -lbl_1_bss_5C;
    movement = 0.5f * (movement + velocity.value.y);
    position.value.z += 30.0;
    position.value.y += movement;
    fn_80026790(modelId, &position.value);
    fn_80086B04(player->actor, &position.value);

    movementElapsed += 0.016666666666666666;
    if (!(movementElapsed < 2.0f)) {
        fn_800269E8(modelId, 1);
        {
            PlayerUpdate values;
            values = &Player::fn_1_DBF4;
            frameUpdate = values;
        }
    }
}
void Player::fn_1_E718()
{
    VectorValue adjustedPosition;
    EffectVector objectPosition;
    Point3d separation;
    f32 distance;
    f32 minimumDistance;
    s32 found = 0;
    s32 objectCount = lbl_1_bss_C.size();
    s32 index;

    adjustedPosition = position;
    adjustedPosition.value.y += 50.0f;
    for (index = 0; index < objectCount; ++index) {
        CollisionObject *object = (CollisionObject *)lbl_1_bss_C.at(index);
        s32 state = CollisionState(object);

        if (CollisionActive(object) != 0 && state == 2) {
            objectPosition = object->position;
            PSVECSubtract(&adjustedPosition.value, &objectPosition.value, &separation);
            distance = SeparationLength(&separation);
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
        SetPoint(&velocity.value, 0.0,
                       200.0f * lbl_1_bss_5C, 0.0);
        sendEvent(1, 3, 0.0f);
        fn_8006A030((s16)playerNo, 0x2D, 7, 3);
        {
            PlayerUpdate callbackValues = &Player::fn_1_E5EC;
            frameUpdate = callbackValues;
        }
    }
}
void Player::fn_1_E95C() {}
void Player::fn_1_E960() {
    firstList.clear();
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
extern "C" s16 fn_8005B0B8(s16, unsigned int);
void Player::fn_1_EA24(s32 dataNum)
{
    s16 motionId = fn_8005B0B8(charNo, (u32)dataNum);
    firstList.push_back(motionId);
}

void fn_1_EA98(Player *object, u32 motionId, u8 loop)
{
    s16 motion;

    if (object->currentMotionIndex != motionId) {
        motion = object->firstList.at(motionId);
        fn_800269E8(object->modelId, ((1 << 30) | (1 << 1)));
        fn_8005C2C8(object->charNo, motion,
                           0.0f, 8.0f, loop != 0 ? ((1 << 30) | (1 << 0)) : 0);
        fn_8005C390(object->charNo, 1.0f);
        object->currentMotionIndex = motionId;
    }
}
void Player::fn_1_EB5C(EventMessage *)
{
    PlayerUpdate initialValues;
    state = 0;
    elapsed = 0.0f;
    delay = RandomFloat(0.0f, 1.0f);
    initialValues = &Player::fn_1_DC04;
    frameUpdate = initialValues;
}
void Player::fn_1_EBF4(EventMessage *)
{
    PlayerUpdate values;
    fn_80087168(player, 1);
    controller->reset();
    values = &Player::fn_1_E29C;
    frameUpdate = values;
}
void Player::fn_1_EC80(EventMessage *)
{
    PlayerUpdate values;
    fn_80086E3C(player);
    if (active == 0) {
        short motion;
        currentMotionIndex = 0xFFFFFFFF;
        if (currentMotionIndex != 4) {
            motion = firstList.at(4);
            fn_800269E8(modelId, ((1 << 30) | (1 << 1)));
            fn_8005C2C8(charNo, motion, 0.0f,
                               8.0f, ((1 << 30) | (1 << 0)));
            fn_8005C390(charNo, 1.0f);
            currentMotionIndex = 4;
        }
        values = &Player::fn_1_E718;
        frameUpdate = values;
    }
}
void Player::fn_1_ED84(EventMessage *)
{
    PlayerUpdate values;
    fn_80086E3C(player);
    if (active != 0) {
        if (currentMotionIndex != 7) {
            short motion = firstList.at(7);
            fn_800269E8(modelId, ((1 << 30) | (1 << 1)));
            fn_8005C2C8(charNo, motion, 0.0f,
                               8.0f, 0);
            fn_8005C390(charNo, 1.0f);
            currentMotionIndex = 7;
        }
        MinigameCore::addWinner(playerNo);
        values = &Player::fn_1_E95C;
        frameUpdate = values;
    }
}

template<class Receiver>
struct PlayerCallbackRecord : EventCallback {
    PlayerMemberFunction method;
    Receiver *receiver;
    explicit PlayerCallbackRecord(const PlayerCallbackBinding &payload)
        : EventCallback(), method(payload.function), receiver(payload.receiver) {}
    virtual ~PlayerCallbackRecord() {}
    virtual void receive(EventMessage *message) { method(receiver, message); }
};
template<class Receiver>
void fn_1_EF74(EventHandlerManager *owner, const PlayerCallbackBinding &payload, int key)
{
    owner->addHandler(new PlayerCallbackRecord<Receiver>(payload), key);
}
