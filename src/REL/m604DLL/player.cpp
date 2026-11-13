#include "REL/m604DLL/camera_masks.h"
/* The players: shared behavior, followed by the day and night games.
 *
 * Every player starts on the ground before the 49-step course, its item hidden.
 * DayPlayer climbs; the highest step reached is the score, and the highest wins.
 * NightPlayer starts above the course and climbs down; the lowest wins.
 * A player stands on a step it lands on, bumps its head when jumping into a step,
 * and walks off the end of a step to fall to the one below.
 * When the round ends, each player rises to the height it reached. The winner
 * holds the item, and the other players drop theirs. */

#include "REL/m604DLL/player.h"

#include "REL/m604DLL/a_set.h"
#include "REL/m604DLL/patapata.h"

#include "REL/m604DLL/day_computer_input.h"
#include "REL/m604DLL/computer_player_configuration.h"

#include "REL/m604DLL/player_position_history.h"
#include "REL/m604DLL/player_particle_effect.h"

#include "REL/m604DLL/camera_owner.h"
#include "REL/m604DLL/camera_effect_owner.h"

#include "REL/m604DLL/score_owner.h"
#include "REL/m604DLL/effect_calls.h"

#include "REL/framework/minigame_core.h"
#include "REL/framework/phase_durations.h"

extern "C" {
#include "humath.h"
#include "game/gamework.h"
#include "game/charman.h"
#include "game/audio.h"
#include "datanum/mario.h"

}
#include <math.h>

// Moves the angle `start` the fraction t of the shorter way to `end` (degrees).
f32 fn_1_E4F8(f32 start, f32 end, f32 t)
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

// The facing (degrees) of a direction in the ground plane.
f32 fn_1_E5FC(Vec *direction)
{
    return (180.0 * atan2(direction->x, direction->z)) / M_PI;
}

const s32 lbl_1_rodata_760[4] = { 48, 56, 72, 80 };

const s32 lbl_1_rodata_770[4] = { 1593, 1594, 1595, 1596 };

Player::Player(s32 playerIndex)
    : playerIndex(playerIndex), step(-1), motion(-1), animationPhase(1), character(lbl_80265728[playerIndex].charNo),
      state(0), unknown50(0.0f), unknown54(0), jumpTime(0.0f), finished(0), groundTime(0.0f), elapsedFrames(0),
      updateCount(0), model(new OwnedModel(lbl_80265728[playerIndex].charNo, 3)), item(0), transitionActive(0),
      fallTime(0.0f), timer(0.0f)
{
    ASet<Player>::getChecked().insert(this);
    m_mot_id.clear();
    m_mot_id.reserve(NUMBEROF_MOTION);
    addMotion(CHARMOT_HSF_c000m1_300);
    addMotion(CHARMOT_HSF_c000m1_301);
    addMotion(CHARMOT_HSF_c000m1_302);
    addMotion(CHARMOT_HSF_c000m1_303);
    addMotion(CHARMOT_HSF_c000m1_303);
    addMotion(CHARMOT_HSF_c000m1_344);
    addMotion(CHARMOT_HSF_c000m1_304);
    addMotion(CHARMOT_HSF_c000m1_306);
    addMotion(CHARMOT_HSF_c000m1_307);
    addMotion(CHARMOT_HSF_c000m1_376);
    addMotion(MARIO_HSF_c000m1_433);

    assert(m_mot_id.size() == NUMBEROF_MOTION);
    fn_8005B4BC(character);
    model->setLayer(3);
    model->setCamera(cameraMasks[playerIndex]);
    model->setShadow();
    shadowActive = 1;
    fn_8005DDEC(5);
    selectMotion(0, 0, 0, 4.0f);
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 0.0f;
    rotation.x = 0.0f;
    rotation.y = 0.0f;
    rotation.z = 0.0f;
    velocity.x = 0.0f;
    velocity.y = 0.0f;
    velocity.z = 0.0f;
    setState(&Player::fn_1_EB98);
    nightMode = GWMgNightFGet();
    createItem();
    fn_8005F77C(character, 0);
    fn_8002DC74(m_mot_id.at(10), 1);
    fn_8005E070(character, CHARMOT_HSF_c000m1_301, 0);
    fn_8005E070(character, CHARMOT_HSF_c000m1_302, 0);
    fn_8005E070(character, CHARMOT_HSF_c000m1_304, 0);
    fn_80060F14(character, lbl_1_rodata_760[playerIndex]);
    soundHandle = HuAudFXPlay(lbl_1_rodata_770[playerIndex]);
    HuAudFXPanning(soundHandle, lbl_1_rodata_760[playerIndex]);
    HuAudFXVolSet(soundHandle, 0);
}

const u32 lbl_1_rodata_784[2] = { 0x3F0030, 0x3F0019 };

void Player::fn_1_EB98()
{
}

Player::~Player()
{
    HuAudFXStop(soundHandle);
    releaseItem();
}

void Player::addMotion(u32 dataNum)
{
    s16 created;
    s32 motionId;

    created = fn_8005B0B8(character, dataNum);
    motionId = created;
    m_mot_id.push_back(motionId);
}

void Player::selectMotion(s32 index, u8 force, u8 keepAudio, f32 blend)
{
    s16 modelId;
    s16 selected;
    modelId = *model;
    selected = m_mot_id.at(index);
    if (motion != index || force != 0) {
        fn_80026A44(modelId, 0x40000002U);
        fn_8005C2C8(character, selected, 0.0f, blend, keepAudio == 0 ? 0x40000001U : 0);
        motion = index;
        if (index == 5) {
            HuAudFXVolSet(soundHandle, 0x7F);
            CharFXPlay(character, 0x246);
            return;
        }
        HuAudFXVolSet(soundHandle, 0);
    }
}

s32 Player::getMotion()
{
    return motion;
}

u8 Player::isComputer()
{
    return lbl_80265728[playerIndex].type != 0;
}

// Places the item at the model's hand and keeps its offset from the player.
void Player::attachItem()
{
    s16 modelId = *model;
    char *hook = fn_8005C0F0(character, 8, 4);
    Vec hookPosition;
    Hu3DModelObjPosGet(*model, hook, &hookPosition);
    PSVECSubtract(&hookPosition, &position, &itemOffset);
    item->setAttribute((1 << 30) | (1 << 1));
    item->resetAttribute(1);
    item->setMotionTime(0.0f);
}

// The item of the game (lbl_1_rodata_784 by day and by night), hidden until a player reaches the top.
void Player::createItem()
{
    Vec origin = {0.0f, 0.0f, 0.0f};
    item.reset(new OwnedModel(lbl_1_rodata_784[nightMode]));
    item->setAttribute((1 << 30) | (1 << 1));
    item->setAttribute(1);
    item->setCamera(cameraMasks[playerIndex]);
    item->setLayer(3);
    item->setUniformScale(0.0f);
    item->setPosition(&origin);
    item->resetShadow();
    itemFade = 0.0f;
    itemAngle = 0.0f;
    itemPosition.x = 0.0f;
    itemPosition.y = 0.0f;
    itemPosition.z = 0.0f;
}

// The item goes once it has faded in fully.
void Player::releaseItem()
{
    if (1.0f == itemFade) {
        item.reset();
    }
}

// Turns back to face the camera and stands.
void Player::fn_1_F198()
{
    switch (state) {
    case 0:
        if (motion != 0 && motion != 6 && motion != 2) {
            selectMotion(6, 0, 1, 16.0f);
        }
        state++;
        /* fall through */
    case 1:
        if (motion == 6 && fn_8005C240(character) == 0) {
            break;
        }
        {
            selectMotion(1, 0, 0, 4.0f);
            state++;
        }
        /* fall through */
    case 2: {
        CameraOwner *cameras;
        f32 target;

        cameras = &MinigameSingleton<CameraOwner>::getChecked();
        target = fn_1_8894(cameras, playerIndex);
        rotation.y += (target - rotation.y) / 12.0f;
        if (!((target - rotation.y < 0.0f ? -(target - rotation.y) : target - rotation.y) > 1.0f)) {
            selectMotion(0, 0, 0, 4.0f);
            animationPhase = 0;
            setState(&Player::fn_1_F554);
            state = 4;
        }
        break;
    }
    }
}

// Faces the camera; on a step it floats with the item it holds.
void Player::fn_1_F554()
{
    CameraOwner *cameras;
    f32 target;
    cameras = &MinigameSingleton<CameraOwner>::getChecked();
    target = fn_1_8894(cameras, playerIndex);
    rotation.y += (target - rotation.y) / 12.0f;
    if (height != 0)
    {
        itemAngle += 2.0f;
        position.y = (f32)((f64)(height * 100) + 100.0 * (0.3f * sin((M_PI * (f64) itemAngle) / 180.0)));
        PSVECAdd(&position, &itemOffset, &itemPosition);
        item->setPosition(&itemPosition);
    }
    animationPhase = 0;
}

// The day game's controller: the computer's for a computer player, the pad's otherwise.
PlayerInputBase *fn_1_F6D8(Player *player, s32 playerIndex)
{
    if (lbl_80265728[playerIndex].type != 0) {
        return new DayComputerInput(player, lbl_80265728[playerIndex].comDif);
    }

    return new HumanPlayerConfiguration(player);
}

DayPlayer::DayPlayer(s32 playerIndex)
    : Player(playerIndex), controller(fn_1_F6D8(this, playerIndex)), history(0), particle(0)
{
    height = 0;
    history.reset(new PlayerPositionHistory(&position, cameraMasks[playerIndex]));
    if (this->playerIndex == 0) {
        position.x = -400.0f;
        position.y = 100.0f;
        rotation.y = 45.0f;
        particle.reset(new ModelParticles(playerIndex));
    }
}

DayPlayer::~DayPlayer()
{
}

void DayPlayer::update()
{
    if (m_state)
    {
        (this->*m_state)();
    }
    position.z = -35.0f;
    model->setPosition(&position);
    model->setRotation(&rotation);
    if (motion == 3 && jumpTime < 0.3f)
    {
        fn_1_37E4(history.get(), &position);
    }
    updateCount++;
    if (position.y < 1000.0f)
    {
        if (shadowActive == 0)
        {
            model->setShadow();
            shadowActive = 1;
        }
    }
    else
    {
        if (shadowActive == 1)
        {
            model->resetShadow();
            shadowActive = 0;
        }
    }
    CameraOwner &cameras = MinigameSingleton<CameraOwner>::getChecked();
    Vec savedPosition = cameras.fn_1_878C(playerIndex);
    Vec secondPosition;
    Vec firstPosition;
    secondPosition.x = firstPosition.x = position.x;
    secondPosition.z = firstPosition.z = position.z;
    if (finished == 0)
    {
        secondPosition.y = firstPosition.y = 300.0f + position.y;
    }
    else
    {
        if (state == 4)
        {
            secondPosition.y = firstPosition.y = savedPosition.y;
        }
        else
        {
            secondPosition.y = firstPosition.y = position.y;
        }
    }
    if (secondPosition.y < 300.0f)
    {
        secondPosition.y = firstPosition.y = 300.0f;
    }
    secondPosition.z = 2500.0f + position.z;
    secondPosition.x = firstPosition.x = 0.4f * position.x;
    fn_1_8730(&cameras, playerIndex, &firstPosition, &secondPosition);
    if (finished != 1)
    {
        M604ScoreOwner &scoreOwner = M604ScoreOwner::getChecked();
        scoreOwner.fn_1_150C8(playerIndex, height);
    }
}

// Player 0 falls in with the opening effect, then lands.
void DayPlayer::fn_1_FF5C()
{
    if (this->playerIndex != 0)
    {
        return;
    }
    f32 elapsedSeconds = (f32) this->elapsedFrames / 60.0f;
    if (elapsedSeconds > 3.0f)
    {
        return;
    }
    CameraEffectOwner &randomSource = CameraEffectOwner::getChecked();
    Vec randomPosition = randomSource.getPosition();
    if (this->elapsedFrames == 0)
    {
        this->position = randomPosition;
    }
    if (this->elapsedFrames == 0x3C)
    {
        selectMotion(3, 0, 1, 4.0f);
        this->updateCount = 0;
    }
    if (this->elapsedFrames == 0x6E)
    {
        Vec recordPosition = { 0.0f, - 50.0f, 0.0f };
        fn_1_153C4(this->particle.get(), & recordPosition);
    }
    if (this->elapsedFrames == 0x77)
    {
        selectMotion(6, 0, 1, 4.0f);
    }
    if (this->motion == 6 && fn_8005C240(this->character) == 1)
    {
        selectMotion(1, 0, 0, 4.0f);
    }
    if (this->motion == 1)
    {
        this->rotation.y += - this->rotation.y / 12.0f;
    }
    if (this->elapsedFrames < 0x3C)
    {
        this->position = randomPosition;
    }
    else
    {
        this->velocity.y = 15.0f - (70.0f * (0.5f * (elapsedSeconds - 1.0f)));
        this->position.y += this->velocity.y;
        this->position.x += 4.1666665f;
        if (this->position.y < 0.0f)
        {
            this->position.y = 0.0f;
        }
        if (this->position.x > 0.0f)
        {
            this->position.x = 0.0f;
        }
    }
    if (this->elapsedFrames == 0xAF)
    {
        this->particle.reset();
        selectMotion(0, 0, 0, 4.0f);
    }
    this->elapsedFrames++;
}

// The round starts: stand, then take the controls.
void DayPlayer::fn_1_104DC()
{
    selectMotion(0, 0, 0, 4.0f);
    setState(&DayPlayer::fn_1_105D8);
}

// The controls: walk, turn and jump; land on a step, bump into the one above, fall off the course.
void DayPlayer::fn_1_105D8()
{
    s32 player = playerIndex;
    Vec startPosition = position;
    PlayerInputBase *input = controller.get();
    u32 buttons;
    f32 radians;
    s32 stepIndex;
    fn_1_6DC(input);
    buttons = input->stick.pressedButtons;
    Vec target;
    target.x = input->stick.x;
    target.z = - input->stick.y;
    target.y = input->stick.magnitude;
    velocity.x = 0.0f;
    velocity.y = 0.0f;
    velocity.z = 0.0f;
    s16 itemModel = *model;
    f32 magnitude = target.y;
    f32 travel;
    if (magnitude != 0.0f)
    {
        f32 targetAngle = (f32)(180.0 * atan2((f64)target.x, (f64)target.z) / M_PI);
        rotation.y = fn_1_E4F8(rotation.y, targetAngle, 0.2f);
    }
    radians = (f32)((M_PI * (f64) rotation.y) / 180.0);
    travel = 0.0f;
    if (motion == 3)
    {
        travel = (f32)(8.333333333333334 * (f64) magnitude);
    }
    else
    {
        if (magnitude != 0.0f)
        {
            if (motion != 5)
            {
                selectMotion(2, 0, 0, 4.0f);
            }
            travel = (f32)(8.333333333333334 * (f64) magnitude);
        }
        else
        {
            if (motion != 5)
            {
                selectMotion(0, 0, 0, 4.0f);
            }
        }
    }
    velocity.x = (f32)((f64) travel * sin((f64) radians));
    velocity.z = (f32)((f64) travel * cos((f64) radians));
    if ((buttons & 0x100U) != 0 && groundTime != 0.0f && (motion == 0 || motion == 6 || motion == 2))
    {
        selectMotion(3, 0, 1, 4.0f);
        jumpTime = 0.0f;
        groundTime = 0.0f;
        fallTime = 0.0f;
        fn_1_2DE8(history.get(), & position);
        updateCount = 0;
    }
    if (motion == 3 && jumpTime <= 0.55f)
    {
        jumpTime += 1.0 / 60.0;
        velocity.y = 50.0f - 125.0f * jumpTime;
    }
    else
    {
        velocity.y = - 18.75f;
    }
    PSVECAdd(& position, & velocity, & position);
    if (velocity.y < 0.0f)
    {
        Vec intersection;
        Vec secondPoint = position;
        secondPoint.y += -10.0f;
        Vec firstPoint = startPosition;
        firstPoint.y += 10.0f;
        s32 collisionResult;
        Patapata &manager = MinigameSingleton<Patapata>::getChecked();
        collisionResult = fn_1_C5F8(&manager, 0, player, & stepIndex, &firstPoint, &secondPoint, & intersection.x);
        if (collisionResult != - 1)
        {
            if (collisionResult == 0)
            {
                position = intersection;
                if (motion != 0 && (motion == 3 || motion == 5))
                {
                    if (motion == 5)
                    {
                        HuAudFXVolSet(soundHandle, 0);
                        fn_8006A030((s16) player, 0x14, 7, 3);
                    }
                    selectMotion(6, 0, 1, 4.0f);
                    fallTime = 0.0f;
                }
            }
            else
            {
                position = intersection;
                if (motion != 0 && (motion == 3 || motion == 5))
                {
                    if (motion == 5)
                    {
                        HuAudFXVolSet(soundHandle, 0);
                        fn_8006A030((s16) player, 0x14, 7, 3);
                    }
                    selectMotion(6, 0, 1, 4.0f);
                    fallTime = 0.0f;
                }
            }
            groundTime += 1.0 / 60.0;
            step = stepIndex;
        }
        else
        {
            if (motion != 5)
            {
                if ((fallTime > 0.7f) && (position.y > 0.0f))
                {
                    selectMotion(5, 0, 0, 4.0f);
                }
                else
                {
                    fallTime += 1.0 / 60.0;
                }
            }
            groundTime = 0.0f;
        }
    }
    if (velocity.y > 0.0f)
    {
        Vec intersection;
        Vec secondPoint = position;
        secondPoint.y += 90.0f;
        Vec firstPoint = startPosition;
        firstPoint.y += 80.0f;
        s32 collisionResult;
        Patapata &manager = MinigameSingleton<Patapata>::getChecked();
        collisionResult = fn_1_C5F8(&manager, 1, player, & stepIndex, &firstPoint, &secondPoint, & intersection.x);
        if ((collisionResult != - 1) && (collisionResult == 0))
        {
            Vec hitPosition = { 0.0f, 0.0f, 0.0f };
            fn_8005D864((s16) cameraMasks[player], & position, & hitPosition);
            position.y += -20.0f;
            jumpTime = 1.0f;
            selectMotion(5, 0, 0, 4.0f);
        }
    }
    if (position.y < 0.0f)
    {
        if (motion != 0 && (motion == 3 || motion == 5))
        {
            if (motion == 5)
            {
                fn_8006A030((s16) player, 0x14, 7, 3);
            }
            selectMotion(6, 0, 1, 16.0f);
            fallTime = 0.0f;
        }
        position.y = 0.0f;
        step = - 1;
        groundTime += 1.0 / 60.0;
    }
    if (position.x < - 170.0f)
    {
        position.x = - 170.0f;
    }
    if (position.x > 170.0f)
    {
        position.x = 170.0f;
    }
    if (motion == 6 && fn_8005C240((s16) character) == 1)
    {
        selectMotion(0, 0, 0, 4.0f);
    }
    height = (s32)((50.0f + position.y) / 100.0f);
}

// The round is over: rise back to the player's height, the winner with the item.
void DayPlayer::fn_1_11574()
{
    f32 heightDifference;

    f32 progress;
    f32 cameraAngle;
    CameraOwner *cameras;
    u8 settled;

    cameras = &MinigameSingleton<CameraOwner>::getChecked();
    cameraAngle = fn_1_8894(cameras, this->playerIndex);
    this->rotation.y += (cameraAngle - this->rotation.y) / 12.0f;
    settled = 1;
    switch (this->state) {
    case 0:
        this->timer += 0.016666666666666666;
        if (this->timer < 0.4f) {
            return;
        }
        ++this->state;
        /* fall through */
    case 1:
        this->rise = ((f32)(this->height * 100) - this->position.y) / 180.0f;
        this->jumpTime = 0.0f;
        ++this->state;
        break;
    case 2:
        if (this->height != 0) {
            attachItem();
        } else {
            selectMotion(0, 0, 0, 4.0f);
        }
        ++this->state;
        /* fall through */
    case 3:
        if (this->position.y - (f32)(this->height * 100) < 0.0f) {
            heightDifference = -(this->position.y - (f32)(this->height * 100));
        } else {
            heightDifference = this->position.y - (f32)(this->height * 100);
        }
        if (heightDifference > 2.0f) {
            this->position.y += this->rise;
            settled = 0;
        }
        this->position.x += 0.05f * -this->position.x;
        f32 positionMagnitude;
        if (this->position.x < 0.0f) {
            positionMagnitude = -this->position.x;
        } else {
            positionMagnitude = this->position.x;
        }
        if (positionMagnitude > 10.0f) {
            settled = 0;
        }
        if (this->height != 0) {
            if (this->itemFade < 1.0f) {
                settled = 0;
            }
        }
        if (settled != 0) {
            setState(&Player::fn_1_F554);
            ++this->state;
        }
        break;
    default:
        break;
    }

    if (this->height != 0) {
        PSVECAdd(&this->position, &this->itemOffset, &this->itemPosition);
        this->item->setPosition(&this->itemPosition);
        progress = this->itemFade;
        progress += 0.016666666666666666;
        if (progress > 1.0f) {
            progress = 1.0f;
        }
        this->item->setUniformScale(progress);
        this->itemFade = progress;
    }
}

// Lost: the item falls away below the course.
void DayPlayer::fn_1_11AE4()
{
    s16 itemModel;

    position.y = (f32)((f64)position.y - 6.666666666666667);
    itemPosition.y += 20.0f;
    item->setPosition(&itemPosition);
    itemModel = *item;
    if (fn_8002D75C(itemModel) == 1) {
        item->setAttribute(1);
    }
    if (position.y < 0.0f) {
        selectMotion(8, 0, 1, 4.0f);
        position.y = 0.0f;
    }
}

struct PanValues {
    s32 values[4];
};

void DayPlayer::result(u8 won)
{
    if (won != 0)
    {
        CharFXPlay((s16) this->character, 0x241);
        selectMotion(10, 0, 0, 4.0f);
        MinigameCore::addWinner(this->playerIndex);
        return;
    }
    if (this->height != 0)
    {
        PanValues panValues = { { 0x30, 0x38, 0x48, 0x50 } };
        HuAudFXPlayPan(0x63D, (s16) panValues.values[this->playerIndex]);
        selectMotion(5, 0, 0, 4.0f);
        fn_80060F40((s16) this->character, 0);
        item->resetAttribute((1 << 30) | (1 << 1));
        setState(&DayPlayer::fn_1_11AE4);
        this->transitionActive = 1;
        fn_8006A030((s16) this->playerIndex, 0x5A, 7, 3);
        return;
    }
    selectMotion(8, 0, 1, 4.0f);
}

void DayPlayer::finish()
{
    this->finished = 1;
    if (this->height != 0) {
        selectMotion(9, 0, 0, 8.0f);
        setState(&DayPlayer::fn_1_11574);
        return;
    }
    setState(&Player::fn_1_F198);
    this->position.y = 0.0f;
    this->height = 0;
}

// The night game's controller: the computer's for a computer player, the pad's otherwise.
PlayerInputBase *fn_1_120BC(Player *player, s32 playerIndex)
{
    if (lbl_80265728[playerIndex].type != 0) {
        return new ComputerPlayerConfiguration(player, lbl_80265728[playerIndex].comDif);
    }
    return new HumanPlayerConfiguration(player);
}

NightPlayer::NightPlayer(s32 playerIndex) : Player(playerIndex), controller(fn_1_120BC(this, playerIndex)), effect(0)
{
    height = 0xC8;
    selectMotion(9, 0, 0, 4.0f);
    effect.reset(new PlayerParticleEffect(&position, cameraMasks[playerIndex]));
    position.y = 26000.0f;
}

NightPlayer::~NightPlayer()
{
}

void NightPlayer::update()
{
    if (m_state)
    {
        (this->* m_state)();
    }
    position.z = - 35.0f;
    model->setPosition(&position);
    model->setRotation(&rotation);
    if (motion == 4)
    {
        fn_1_24C0(effect.get(), &position);
    }
    else
    {
        fn_1_278C(effect.get());
    }
    updateCount++;
    if (position.y < 1000.0f)
    {
        if (shadowActive == 0)
        {
            fn_80026E44(*model);
            fn_80026F9C(*model);
            shadowActive = 1;
        }
    }
    else
    {
        if (1 == shadowActive)
        {
            fn_80026EF0(*model);
            fn_80026FCC(*model);
            shadowActive = 0;
        }
    }
    CameraOwner &cameraOwner = CameraOwner::getChecked();
    Vec cachedPosition = cameraOwner.fn_1_878C(playerIndex);
    Vec secondPosition;
    Vec firstPosition;
    secondPosition.x = firstPosition.x = position.x;
    secondPosition.z = firstPosition.z = position.z;
    if (finished == 0)
    {
        secondPosition.y = firstPosition.y = position.y - 200.0f;
    }
    else
    {
        if (state == 4)
        {
            secondPosition.y = firstPosition.y = cachedPosition.y;
        }
        else
        {
            secondPosition.y = firstPosition.y = position.y;
        }
    }
    if (secondPosition.y < 300.0f)
    {
        secondPosition.y = firstPosition.y = 300.0f;
    }
    secondPosition.z = 2500.0f + position.z;
    secondPosition.x = firstPosition.x = 0.4f * position.x;
    fn_1_8730(&cameraOwner, playerIndex, & firstPosition, & secondPosition);
    if (finished != 1)
    {
        M604ScoreOwner &scoreOwner = M604ScoreOwner::getChecked();
        scoreOwner.fn_1_150C8(playerIndex, height);
    }
}

// Rises from below into view, swaying, with the item.
void NightPlayer::fn_1_128EC()
{
    f32 wave;
    f32 progress;
    f32 elapsedCount;
    f32 divisor;
    f32 targetHeight;
    if (this->elapsedFrames == 1)
    {
        attachItem();
        this->item->setUniformScale(1.0f);
    }
    this->elapsedFrames++;
    if ((f32)this->elapsedFrames < 30.0f) return;
    divisor = 12.0f;
    elapsedCount = this->elapsedFrames - 30.0f;
    wave = 50.0 * sin((M_PI * (f64)(5.0f * elapsedCount)) / 180.0);
    progress = 1.0f - (((0.2f * this->elapsedFrames) - 30.0f) / divisor);
    if (progress < 0.4f) progress = 0.4f;
    wave *= progress;
    targetHeight = 20000.0f + wave;
    this->position.y += (targetHeight - this->position.y) / divisor;
    PSVECAdd(&this->position, &this->itemOffset, &this->itemPosition);
    this->item->setPosition(&this->itemPosition);
}

// The round starts: stand, drop the item and take the controls.
void NightPlayer::fn_1_12BCC()
{

    selectMotion(0, 0, 0, 4.0f);
    fn_1_2544(effect.get());
    PanValues panValues = { { 0x30, 0x38, 0x48, 0x50 } };
    HuAudFXPlayPan(0x63D, (s16)panValues.values[playerIndex]);
    item->resetAttribute((1 << 30) | (1 << 1));
    setState(&NightPlayer::fn_1_12D2C);
}

// The controls: walk, turn and jump down the course; a player who lands on the ground is out.
void NightPlayer::fn_1_12D2C()
{
    s16 currentMotionId = *this->item;
    PlayerInputBase *input;
    u32 buttons;
    s32 stepIndex;
    s16 itemModel;
    f32 magnitude;
    f32 travel;
    f32 radians;

    if (fn_8002D75C(currentMotionId) == 1) {
        this->item->setAttribute(1);
    }
    s32 player = this->playerIndex;
    Vec startPosition = this->position;
    input = this->controller.get();
    fn_1_6DC(input);
    buttons = input->stick.pressedButtons;
    Vec direction;
    direction.x = input->stick.x;
    direction.z = -input->stick.y;
    direction.y = input->stick.magnitude;
    this->velocity.x = 0.0f;
    this->velocity.y = 0.0f;
    this->velocity.z = 0.0f;

    itemModel = *this->model;
    magnitude = direction.y;
    if (magnitude != 0.0f) {
        f32 targetAngle = (f32)(180.0 * atan2((f64)direction.x, (f64)direction.z) / M_PI);
        this->rotation.y = fn_1_E4F8(this->rotation.y, targetAngle, 0.2f);
    }

    radians = (f32)((M_PI * (f64)this->rotation.y) / 180.0);
    travel = 0.0f;
    if (this->motion == 3) {
        travel = (f32)(8.333333333333334 * (f64)magnitude);
    } else if (magnitude != 0.0f) {
        if ((this->motion != 5) && (this->motion != 4)) {
            this->selectMotion(2, 0, 0, 4.0f);
        }
        travel = (f32)(8.333333333333334 * (f64)magnitude);
    } else if ((this->motion != 5) && (this->motion != 4)) {
        this->selectMotion(0, 0, 0, 4.0f);
    }
    this->velocity.x = (f32)((f64)travel * sin((f64)radians));
    this->velocity.z = (f32)((f64)travel * cos((f64)radians));
    if ((buttons & 0x100U) != 0 && this->groundTime != 0.0f &&
        (this->motion == 0 || this->motion == 6 ||
         this->motion == 2)) {
        this->selectMotion(3, 0, 1, 4.0f);
        this->jumpTime = 0.0f;
        this->groundTime = 0.0f;
        this->fallTime = 0.0f;
    }
    if (this->motion == 3 && this->jumpTime <= 0.55f) {
        this->jumpTime += 1.0 / 60.0;
        this->velocity.y = 50.0f - 125.0f * this->jumpTime;
    } else {
        this->velocity.y = -18.75f;
    }
    PSVECAdd(&this->position, &this->velocity, &this->position);
    if (this->velocity.y < 0.0f) {
        Vec intersection;
        Vec secondPoint = this->position;
        secondPoint.y += -10.0f;
        Vec firstPoint = startPosition;
        firstPoint.y += 10.0f;
        s32 collisionResult;
        Patapata &manager = MinigameSingleton<Patapata>::getChecked();
        collisionResult = fn_1_C5F8(&manager, 0, player, &stepIndex,
                                  &firstPoint, &secondPoint, &intersection.x);
        if (collisionResult != -1) {
            if (collisionResult == 0) {
                this->position = intersection;
                if (this->motion != 0 &&
                    (this->motion == 3 ||
                     this->motion == 5 ||
                     this->motion == 4)) {
                    if (this->motion == 5 ||
                        this->motion == 4) {
                        fn_8006A030((s16)player, 0x14, 7, 3);
                    }
                    this->selectMotion(6, 0, 1, 4.0f);
                    this->fallTime = 0.0f;
                }
            } else {
                this->position = intersection;
                if (this->motion != 0 &&
                    (this->motion == 3 ||
                     this->motion == 5 ||
                     this->motion == 4)) {
                    if (this->motion == 5 ||
                        this->motion == 4) {
                        fn_8006A030((s16)player, 0x14, 7, 3);
                    }
                    this->selectMotion(6, 0, 1, 4.0f);
                    this->fallTime = 0.0f;
                }
            }
            this->groundTime += 1.0 / 60.0;
            this->step = stepIndex;
        } else {
            if ((this->motion != 5) &&
                (this->fallTime > 0.7f) && (this->position.y > 0.0f)) {
                this->selectMotion(4, 0, 1, 2.0f);
            }
            if (this->motion != 3) {
                this->selectMotion(4, 0, 1, 2.0f);
            }
            this->groundTime = 0.0f;
        }
    }
    if (this->velocity.y > 0.0f) {
        Vec intersection;
        Vec secondPoint = this->position;
        secondPoint.y += 90.0f;
        Vec firstPoint = startPosition;
        firstPoint.y += 80.0f;
        s32 collisionResult;
        Patapata &manager = MinigameSingleton<Patapata>::getChecked();
        collisionResult = fn_1_C5F8(&manager, 1, player, &stepIndex,
                                  &firstPoint, &secondPoint, &intersection.x);
        if ((collisionResult != -1) && (collisionResult == 0)) {
            Vec hitPosition = { 0.0f, 0.0f, 0.0f };
            fn_8005D864((s16)cameraMasks[player],
                                &this->position, &hitPosition);
            this->position.y += -20.0f;
            this->jumpTime = 1.0f;
            this->selectMotion(5, 0, 0, 4.0f);
        }
    }
    if (this->position.y < 0.0f) {
        setState(&Player::fn_1_F198);
        fn_8006A030((s16)player, 0x14, 7, 3);
        this->position.y = 0.0f;
        this->height = 0;
        return;
    }
    if (this->position.x < -170.0f) {
        this->position.x = -170.0f;
    }
    if (this->position.x > 170.0f) {
        this->position.x = 170.0f;
    }
    if (this->motion == 6 &&
        fn_8005C240((s16)this->character) == 1) {
        this->selectMotion(0, 0, 0, 4.0f);
    }
    this->height = (s32)((50.0f + this->position.y) / 100.0f);
}

// The round is over: settle at the player's height, the winner with the item.
void NightPlayer::fn_1_13CE4()
{
    f32 heightDifference;
    f32 horizontalMagnitude;
    f32 cameraAngle;
    f32 progress;
    u8 settled;
    CameraOwner *cameras = &CameraOwner::getChecked();
    cameraAngle = fn_1_8894(cameras, this->playerIndex);
    this->rotation.y += (cameraAngle - this->rotation.y) / 12.0f;
    settled = 1;
    switch (this->state)
    {
    case 0 :
        this->timer += 0.016666666666666666;
        if (this->timer < 0.4f)
        {
            return;
        }
        ++this->state;
    case 1 :
        this->rise = ((f32)(this->height * 100) - this->position.y) / 180.0f;
        this->jumpTime = 0.0f;
        ++this->state;
        break;
    case 2 :
        if (this->height != 0)
        {
            attachItem();
        }
        else
        {
            selectMotion(0, 0, 0, 4.0f);
        }
        ++this->state;
    case 3 :
        if ((this->position.y - (f32)(this->height * 100)) < 0.0f)
        {
            heightDifference = - (this->position.y - (f32)(this->height * 100));
        }
        else
        {
            heightDifference = this->position.y - (f32)(this->height * 100);
        }
        if (heightDifference > 2.0f)
        {
            this->position.y += this->rise;
            settled = 0;
        }
        this->position.x += 0.05f * - this->position.x;
        if (this->position.x < 0.0f)
        {
            horizontalMagnitude = - this->position.x;
        }
        else
        {
            horizontalMagnitude = this->position.x;
        }
        if (horizontalMagnitude > 10.0f)
        {
            settled = 0;
        }
        if ((this->height != 0) && (this->itemFade < 1.0f))
        {
            settled = 0;
        }
        if (settled != 0)
        {
            setState(&Player::fn_1_F554);
            ++this->state;
        }
        break;
    default :
        break;
    }
    if (this->height != 0)
    {
        PSVECAdd(& this->position, & this->itemOffset, & this->itemPosition);
        this->item->setPosition(&this->itemPosition);
        progress = this->itemFade;
        progress += 0.016666666666666666;
        if (progress > 1.0f)
        {
            progress = 1.0f;
        }
        this->item->setUniformScale(progress);
        this->itemFade = progress;
    }
}

// Lost: the item falls away below the course.
void NightPlayer::fn_1_14254()
{
    s16 motionIdForCheck;

    motionIdForCheck = *item;
    position.y = (f32)((f64)position.y - 6.666666666666667);
    itemPosition.y += 20.0f;
    item->setPosition(&itemPosition);
    if (fn_8002D75C(motionIdForCheck) == 1) {
        item->setAttribute(1);
    }
    if (position.y < 0.0f) {
        selectMotion(8, 0, 1, 4.0f);
        position.y = 0.0f;
    }
}

void NightPlayer::result(u8 won)
{
    if (won != 0)
    {
        if (this->height != 0)
        {
            CharFXPlay((s16) this->character, 0x241);
            this->selectMotion( 10, 0, 0, 4.0f);
        }
        else
        {
            this->selectMotion( 7, 0, 1, 4.0f);
        }
        MinigameCore::addWinner(this->playerIndex);
        return;
    }
    if (this->height != 0)
    {
        PanValues panValues = { { 0x30, 0x38, 0x48, 0x50 } };
        HuAudFXPlayPan(0x63D, (s16) panValues.values[this->playerIndex]);
        this->selectMotion( 5, 0, 0, 4.0f);
        fn_80060F40((s16) this->character, 0);
        this->item->resetAttribute((1 << 30) | (1 << 1));
        setState(&NightPlayer::fn_1_14254);
        this->transitionActive = 1;
        fn_8006A030((s16) this->playerIndex, 0x5A, 7, 3);
        return;
    }
    this->selectMotion( 8, 0, 1, 4.0f);
}

void NightPlayer::finish()
{
    fn_1_278C(this->effect.get());
    finished = 1;
    if (height != 0)
    {
        selectMotion(9, 0, 0, 8.0f);
        setState(&NightPlayer::fn_1_13CE4);
    }
}
