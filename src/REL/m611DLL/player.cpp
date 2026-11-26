// Updates the players and their cranes, boxes and character models.
/* Retail fixes the next assert's line; the earlier lines are unknown: 7 lines are left blank. */







#include "REL/framework/list_object.h"
#include "REL/m611DLL/effect_node.h"
#include "REL/m611DLL/rotated_sprite_effect.h"
#include "REL/m611DLL/ground_sprite_effect.h"
#include "REL/m611DLL/player_state.h"
#include "REL/framework/phase_duration_statics.h"
#include "REL/framework/seconds_to_frames_inline.h"
#include "REL/framework/vector_math.h"
#include "REL/framework/random.h"
#include "REL/m611DLL/score_display.h"
#include "REL/m611DLL/motion_candidate_table.h"
#include <vector>
#include <cmath>
#include "REL/m611DLL/list_rotation.h"
#include "REL/framework/assertion.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/model.h"
#include "REL/framework/sound.h"
#include "REL/m611DLL/angle_inline.h"
#include "REL/m611DLL/particle_strip.h"
#include "REL/m611DLL/timed_flash.h"
#include "REL/framework/p_info.h"
#include "REL/m611DLL/burst_models.h"
extern "C" {
#include "game/audio.h"
#include "game/hu3d.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "datadir_enum.h"
#include "datanum/mario.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/pad.h"
#include "humath.h"
}

// Routines used by the player setup and per-frame update.
bool fn_1_1186C(PlayerState *);
HU3D_MOTIONID fn_1_CA9C(PlayerState *player, unsigned int dataNum);
f32 fn_1_117A4(const PlayerState *player);
void fn_1_D820(PlayerState *player);
void fn_1_D678(PlayerState *player);
void fn_1_D74C(PlayerState *player);
void fn_1_DA98(PlayerState *player, Vec *rotation);
void fn_1_DC38(PlayerState *player, Vec *position, Vec *rotation);
void fn_1_E298(PlayerState *player, Vec *targetPosition, Vec *angles);
void fn_1_D8F8(PlayerState *player);
void fn_1_D950(PlayerState *player);

// The sole winner when exactly one player wins. The minigame never reads this pointer.
PlayerState *lbl_1_bss_1AC;
// All players in update order.
ListRoot<PlayerState> lbl_1_bss_1BC;
// Returns the signed angle difference in degrees. Nothing in this minigame calls this.
f32 signedAngleDifference(f64 targetAngle, f64 currentAngle)
{
    s32 targetUnits = AngleToUnits(targetAngle);
    s32 currentUnits = AngleToUnits(currentAngle);
    s32 difference = WrapAngleDifference(targetUnits - currentUnits);
    return AngleUnitsToDegrees(difference);
}

// Limits the angle difference to the supplied step. Nothing in this minigame calls this.
f64 clampedAngleDifference(f64 targetAngle, f64 currentAngle, f64 limit)
{
    f32 difference = DifferenceInDegrees(targetAngle, currentAngle);
    return ClampAngleDifference(difference, limit);
}

// Starting angles in degrees, indexed by player.
struct PlayerStartAngles {
    f32 values[4];
};
// Starting angles shared by the two position calculations.
static const PlayerStartAngles lbl_1_rodata_718 = {{0.0f, 90.0f, 270.0f, 180.0f}};
// Computes a player start position on the scene circle.
static inline void PlayerStartPosition(Vec *position, s32 angleIndex)
{
    PlayerStartAngles offsets = lbl_1_rodata_718;
    f64 z;
    f64 x;
    f32 radius;
    f32 angleBase;
    f32 angle;
    radius = 1200.0f;
    angleBase = 45.0f + (180.0f + offsets.values[angleIndex]);
    angle = static_cast<f32>((M_PI * static_cast<f64>(angleBase)) / 180.0);
    z = radius * sin(angle);
    x = radius * cos(angle);
    position->x = x;
    position->y = 0.0f;
    position->z = z;
}

// Computes a player start position on the scene circle.
// Nothing in this minigame calls this.
void fn_1_BBFC(Vec *position, s32 angleIndex)
{
    PlayerStartAngles offsets = lbl_1_rodata_718;
    f32 radius;
    f32 angleBase;
    f32 angle;
    f64 x;
    f64 z;
    radius = 1200.0f;
    angleBase = 45.0f + (180.0f + offsets.values[angleIndex]);
    angle = static_cast<f32>((M_PI * static_cast<f64>(angleBase)) / 180.0);
    z = radius * sin(angle);
    x = radius * cos(angle);
    position->x = x;
    position->y = 0.0f;
    position->z = z;
}

// Deletes all players.
void fn_1_BD38()
{
    deleteEffects(lbl_1_bss_1BC);
}

// Crane model resources by player and day or night variant.
static const s32 sPlayerCraneModelIds[] = {
    DATANUM(0x460000, 6), DATANUM(0x460000, 6), DATANUM(0x460000, 7), DATANUM(0x460000, 7),
    DATANUM(0x460000, 8), DATANUM(0x460000, 8), DATANUM(0x460000, 9), DATANUM(0x460000, 9)
};

// Crane box model resources by player and day or night variant.
static const s32 sPlayerBoxModelIds[] = {
    DATANUM(0x460000, 10), DATANUM(0x460000, 10), DATANUM(0x460000, 11), DATANUM(0x460000, 11),
    DATANUM(0x460000, 12), DATANUM(0x460000, 12), DATANUM(0x460000, 13), DATANUM(0x460000, 13)
};

// Moving model resources by player and day or night variant.
static const s32 sPlayerTimedTransitionModelIds[] = {
    DATANUM(0x460000, 14), DATANUM(0x460000, 14), DATANUM(0x460000, 15), DATANUM(0x460000, 15),
    DATANUM(0x460000, 16), DATANUM(0x460000, 16), DATANUM(0x460000, 17), DATANUM(0x460000, 17)
};

// Creates the player models, motions, callbacks, and effect owners.
PlayerState::PlayerState(s32 playerNo, s32 modelVariant, bool dayMode)
    : callbackDisabled(0), active(0), unk3C(0.0f), movementLength(0.0f), unk48(0),
      bodyModel(lbl_80265728[playerNo].charNo, 3),
      craneModel(sPlayerCraneModelIds[modelVariant]),
      boxModel(sPlayerBoxModelIds[modelVariant]),
      timedTransitionModel(sPlayerTimedTransitionModelIds[modelVariant]),
      motionModel(DATANUM(0x460000, 29)),
      resetMotionModel(DATANUM(0x460000, 32)),
      firstStateModel(dayMode ? DATANUM(0x460000, 34) : DATANUM(0x460000, 36)),
      secondStateModel(dayMode ? DATANUM(0x460000, 33) : DATANUM(0x460000, 35)),
      winner(0), resultPoseComplete(0), updateCountdown(0), currentMotion(-1),
      motionEnabled(0), motionTimer(0), previousCandidateState(-1),
      score(0), hasCandidate(false), candidateChanged(0), effectTimer(0),
      collisionTimer(secondsToFrames(0.5f)), unk98(0),
      updateCount(0), selectedPosition(-1),
      motionActive(0), transitionStarted(0), countdownActive(0), unkCE(0),
      modelAttributeFlag(0), motionResetPending(0), angleEnabled(0), countdown(0),
      transitionCountdown(0), effectEnabled(0), effectHeight(0.0f), belowSpriteThreshold(0),
      activeAudioId(0), soundId(0)
{
    s16 modelId;
    insert4(&lbl_1_bss_1BC);
    this->playerIndex = playerNo;
    this->character = lbl_80265728[playerNo].charNo;
    this->comDif = lbl_80265728[playerNo].comDif;
    this->padNo = (lbl_80265728[playerNo].type != 0) ? -1 : lbl_80265728[playerNo].padNo;
    this->characterMotion = -1;
    this->m_mot_id.clear();
    this->m_mot_id.reserve(NUMBEROF_MOTION);
    fn_1_CA9C(this, MARIO_HSF_c000m1_328);
    assert(m_mot_id.size() == NUMBEROF_MOTION);
    fn_8005B4BC(static_cast<s16>(this->character));
    modelId = bodyModel;
    fn_80026B8C(modelId, 4);
    fn_80026B64(modelId, 1);
    fn_80026760(modelId, 0.0f, 10.0f, 15.0f);
    modelId = boxModel;
    fn_80026B8C(modelId, 4);
    fn_80026B64(modelId, 1);
    fn_800269E8(modelId, 0x40000002);
    modelId = timedTransitionModel;
    fn_80026B8C(modelId, 1);
    fn_80026B64(modelId, 1);
    if (dayMode) {
        fn_80026E44(modelId);
        fn_80026F9C(modelId);
    }
    fn_800269E8(modelId, 0x40000002);
    this->motionDuration = fn_8002D530(modelId);
    modelId = craneModel;
    fn_80026B8C(modelId, 1);
    fn_80026B64(modelId, 1);
    fn_800269E8(modelId, 0x40000002);
    if (dayMode) {
        fn_80026E44(modelId);
        fn_80026F9C(modelId);
    }
    fn_8002741C(craneModel, "Box_T", boxModel);
    fn_8002741C(craneModel, "Chara_T", bodyModel);
    modelId = motionModel;
    fn_80026B8C(modelId, 1);
    fn_80026B64(modelId, 1);
    fn_800269E8(modelId, 0x1);
    fn_800269E8(modelId, 0x40000002);
    this->firstMotion = fn_80030458(modelId, fn_80008DD0(DATANUM(0x460000, 30), 0x10000000, 2));
    this->secondMotion = fn_80030458(modelId, fn_80008DD0(DATANUM(0x460000, 31), 0x10000000,
        2));
    modelId = resetMotionModel;
    fn_80026B8C(modelId, 4);
    fn_80026B64(modelId, 1);
    fn_800269E8(modelId, 0x1);
    fn_800269E8(modelId, 0x40000002);
    modelId = firstStateModel;
    fn_80026B8C(modelId, 1);
    fn_80026B64(modelId, 1);
    fn_800269E8(modelId, 0x1);
    modelId = secondStateModel;
    fn_80026B8C(modelId, 1);
    fn_80026B64(modelId, 1);
    fn_800269E8(modelId, 0x1);
    fn_800269E8(modelId, 0x40000002);
    PlayerStartPosition(&this->position, playerNo);
    // Stores the starting rotation twice.
    this->rotationY = this->targetAngle = this->rotationY =
        90.0f + fn_1_117A4(this);
    this->unk4C = 500.0f;
    craneModel.setPosition(&this->position);
    void (PlayerState::*nextUpdate)() = &PlayerState::fn_1_D204;
    this->callback = nextUpdate;
    MinigameSingleton<ScoreDisplay>::getChecked().fn_1_12104(
        this->playerIndex, this->character);
    MinigameSingleton<ScoreDisplay>::getChecked().fn_1_12274(
        this->playerIndex, 0);
    this->particleStrip = new ParticleStrip;
    this->timedFlash = new TimedFlash;
    this->fn_1_10C04();
    fn_1_FA64(this, 0, 0);
}

// Deletes the player's particle strip and timed flash.
PlayerState::~PlayerState()
{
    delete this->particleStrip;
    delete this->timedFlash;
}

// Loads and records a character motion resource.
HU3D_MOTIONID fn_1_CA9C(PlayerState *player, unsigned int dataNum)
{
    HU3D_MOTIONID motionId = fn_8005B0B8(player->character, dataNum);
    player->m_mot_id.push_back(motionId);
    return motionId;
}

// Updates all player nodes and rotates their list order.
void fn_1_CB18()
{
    lbl_1_bss_1BC.visit(fn_1_D820);
    lbl_1_bss_1BC.visit(fn_1_EB48);
    rotateListFront(lbl_1_bss_1BC);
}

// Marks the highest ranked players and records a unique winner.
void fn_1_CBB0()
{
    PlayerState *players[4];
    s32 scores[4];
    PlayerState *player = lbl_1_bss_1BC.front();
    Node *end = lbl_1_bss_1BC.end_node();
    s32 index;
    s32 pass;
    s32 sortIndex;
    s32 leftScoreOrEntry;
    s32 rightScoreOrEntry;
    s32 tiedCount;
    s32 compareIndex;
    s32 markIndex;

    lbl_1_bss_1AC = 0;
    s32 order[4] = { 0, 1, 2, 3 };
    // The ranking does not use this value.
    s32 result = 0;
    index = 0;
    for (; player != end; player = player->next()) {
        players[index] = player;
        scores[index] = player->score;
        ++index;
    }

    for (pass = 0; pass < 3; pass++) {
        for (sortIndex = 0; sortIndex < 3; sortIndex++) {
            if (scores[sortIndex] < scores[sortIndex + 1]) {
                leftScoreOrEntry = scores[sortIndex];
                rightScoreOrEntry = scores[sortIndex + 1];
                scores[sortIndex] = rightScoreOrEntry;
                scores[sortIndex + 1] = leftScoreOrEntry;
                leftScoreOrEntry = order[sortIndex];
                rightScoreOrEntry = order[sortIndex + 1];
                order[sortIndex] = rightScoreOrEntry;
                order[sortIndex + 1] = leftScoreOrEntry;
            }
        }
    }

    tiedCount = 1;
    for (compareIndex = 1; compareIndex < 4; compareIndex++) {
        if (scores[0] == scores[compareIndex]) {
            ++tiedCount;
        }
    }
    if (tiedCount == 4 && scores[0] == 0) {
        return;
    }
    if (tiedCount == 1) {
        lbl_1_bss_1AC = players[order[0]];
    }
    for (markIndex = 0; markIndex < tiedCount; markIndex++) {
        players[order[markIndex]]->winner = 1;
        MinigameCore::addWinner(players[order[markIndex]]->playerIndex);
    }
}

// Marks the result entries and installs their turn callbacks.
void fn_1_CDEC()
{
    PlayerState *player = lbl_1_bss_1BC.front();
    Node *end = lbl_1_bss_1BC.end_node();
    for (; player != end; player = player->next()) {
        player->callbackDisabled = 1;
        void (PlayerState::*nextUpdate)() = &PlayerState::fn_1_CFBC;
        player->callback = nextUpdate;
        player->unk48 = 1;
    }
}

// Copies the current player positions into the supplied array.
void fn_1_CE90(Vec *positions)
{
    Node *end;
    PlayerState *firstPlayer;
    PlayerState *player;
    PlayerState *initialPlayer;
    PlayerState *nextPlayer;
    s32 index;
    firstPlayer = static_cast<PlayerState *>(lbl_1_bss_1BC.link0);
    initialPlayer = firstPlayer;
    player = initialPlayer;
    end = lbl_1_bss_1BC.end_node();
    index = 0;
    while (player != end) {
        positions[index].x = player->position.x;
        positions[index].y = player->position.y;
        positions[index].z = player->position.z;
        nextPlayer = static_cast<PlayerState *>(player->link0);
        player = nextPlayer;
        index++;
    }
}

// Tests whether any player still has pending state.
bool fn_1_CF20()
{
    PlayerState *player = lbl_1_bss_1BC.front();
    Node *end = lbl_1_bss_1BC.end_node();
    for (; player != end; player = player->next()) {
        if (player->active != 0) {
            return true;
        }
        if (player->countdownActive != 0) {
            return true;
        }
        if (player->resultPoseComplete == 0) {
            return true;
        }
    }
    return false;
}

// Returns the limited turn toward a target angle.
static inline f32 AngleStepToward(f64 targetAngle, f64 currentAngle, f32 limit)
{
    f32 difference = DifferenceInDegrees(targetAngle, currentAngle);
    return ClampAngleStep(difference, limit);
}

// Turns the player toward the result pose and updates its animation.
void PlayerState::fn_1_CFBC()
{
    f32 angleStep;

    if (this->active != 0) {
        this->advanceMotionState();
    } else if (this->countdownActive != 0) {
        this->advanceMotionCountdown();
    }

    angleStep = AngleStepToward(-45.0f, this->rotationY, 100.0f / 60.0f);
    this->rotationY += angleStep;
    if ((angleStep < 0.0f ? -angleStep : angleStep) < 0.01f) {
        fn_1_FA64(this, 0, 0);
        this->resultPoseComplete = 1;
        return;
    }
    this->audioEnabled = 1;
}

// Computes a player target position on the scene circle.
static inline void PlayerTargetPosition(Vec *position, PlayerState *player,
    const PlayerStartAngles &offsets, f32 radius)
{
    f64 z;
    f64 x;
    f32 angle;
    f32 angleBase;

    angleBase = 45.0f + (180.0f + offsets.values[player->playerIndex]);
    angle = static_cast<f32>((M_PI * static_cast<f64>(angleBase)) / 180.0);
    z = radius * sin(angle);
    x = radius * cos(angle);
    position->x = x;
    position->y = 0.0f;
    position->z = z;
}

// Moves toward the result position or settles into the result pose.
static inline void TargetMotion(PlayerState *player, Vec &direction)
{
    f32 angleStep;
    f32 distance = vectorMagnitude(&direction);
    if (distance > 25.0f) {
        PSVECScale(&direction, &direction, 1.0f / distance);
        fn_1_10848(player, &direction);
    } else {
        angleStep = AngleStepToward(0.0f, player->rotationY, 100.0f / 60.0f);
        player->rotationY += angleStep;
        if ((angleStep < 0.0f ? -angleStep : angleStep) < 0.01f) {
            void (PlayerState::*nextUpdate)() = &PlayerState::fn_1_FB24;
            player->callback = nextUpdate;
        } else {
            // The stored motion code is never read.
            s32 motion = 18;
            player->unk48 = motion;
        }
        player->movementLength = 0.0f;
    }
}

// Moves the player toward its result position and updates its animation.
void PlayerState::fn_1_D204()
{
    Vec target;
    Vec direction;
    PlayerStartAngles offsets = {{0.0f, 90.0f, 270.0f, 180.0f}};
    f32 radius = 800.0f;

    this->audioEnabled = 1;
    PlayerTargetPosition(&target, this, offsets, radius);
    PSVECSubtract(&target, &this->position, &direction);
    TargetMotion(this, direction);
    fn_1_FA64(this, 0, 0);
}

// Starts the closing motions and returns zero if any player won.
u8 fn_1_D5D0()
{
    // The character numbers are written here and never read.
    s16 characters[4];
    u8 result;
    s32 index;
    PlayerState *player;
    Node *end;

    player = lbl_1_bss_1BC.front();
    end = lbl_1_bss_1BC.end_node();
    result = 1;
    index = 0;

    for (; player != end; player = player->next()) {
        characters[index++] = static_cast<s16>(player->character);
        if (player->winner != 0) {
            fn_1_D678(player);
            result = 0;
        } else {
            fn_1_D74C(player);
        }
    }
    return result;
}

// Starts the final motion segment and installs its completion callback.
void fn_1_D678(PlayerState *player)
{
    s16 currentModel;
    s16 modelId;
    void (PlayerState::*nextUpdate)();

    modelId = static_cast<s16>(player->craneModel.modelId);
    currentModel = modelId;
    fn_80026A44(currentModel, 0x40000002);
    fn_800269E8(currentModel, 0x40000001);
    // The final segment starts on frame 391.
    fn_8002D730(currentModel, 391.0f, fn_8002D530(currentModel));
    fn_8002D414(currentModel, 391.0f);

    nextUpdate = &PlayerState::fn_1_D748;
    player->callback = nextUpdate;
}

// Empty final motion completion hook.
void PlayerState::fn_1_D748()
{
}

// Starts the initial motion segment and installs its completion callback.
void fn_1_D74C(PlayerState *player)
{
    s16 currentModel;
    s16 modelId;
    void (PlayerState::*nextUpdate)();

    modelId = static_cast<s16>(player->craneModel.modelId);
    currentModel = modelId;
    fn_80026A44(currentModel, 0x40000002);
    fn_80026A44(currentModel, 0x40000001);
    fn_8002D730(currentModel, 0.0f, 60.0f);
    fn_8002D414(currentModel, 0.0f);

    nextUpdate = &PlayerState::fn_1_D81C;
    player->callback = nextUpdate;
}

// Empty initial motion completion hook.
void PlayerState::fn_1_D81C()
{
}

// Reads the model joint position and invokes the player update callback.
void fn_1_D820(PlayerState *player)
{
    s16 craneModelId;
    s16 modelId;

    modelId = static_cast<s16>(player->craneModel.modelId);
    craneModelId = modelId;
    Hu3DModelObjPosGet(craneModelId, "Crane05S", &player->jointPosition);
    (player->*player->callback)();
    player->updateCount++;
}

// Updates the player model, motion, position, and sound effects.
void fn_1_D894(PlayerState *player)
{
    Vec rotation;
    Vec position;

    fn_1_DA98(player, &rotation);
    fn_1_DC38(player, &position, &rotation);
    fn_1_E298(player, &position, &rotation);
    fn_1_D8F8(player);
    fn_1_D950(player);
}

// Places the owned effect above the player position.
void fn_1_D8F8(PlayerState *player)
{
    Vec position;
    TimedFlash *flash;

    position.x = player->position.x;
    position.y = 250.0f + player->position.y;
    position.z = player->position.z;
    flash = player->timedFlash;
    flash->position.x = position.x;
    flash->position.y = position.y;
    flash->position.z = position.z;
}

// Per-player sound effect IDs for the spinning effect.
const s32 lbl_1_rodata_800[4] = { 1663, 1664, 1665, 1666 };

// Sprite offset from the player before the crane rotation.
const Vec lbl_1_rodata_810 = { 0.0f, 0.0f, -200.0f };

// Updates positional sound and spawns the flagged rotation effect.
void fn_1_D950(PlayerState *player)
{
    Mtx rotation;
    Vec effectPosition;

    if (player->audioEnabled != 0) {
        if (player->activeAudioId >= 0) {
            soundPanTo(1, player->activeAudioId, &player->position);
            soundSetVolumeAt(1, player->activeAudioId, &player->position);
        } else {
            player->activeAudioId =
                soundPlayAtWithVolume(1, lbl_1_rodata_800[player->playerIndex],
                           &player->position);
        }
    } else {
        HuAudFXStop(player->activeAudioId);
        player->activeAudioId = -1;
    }

    if ((player->updateCount & 0x7) == 0 && player->audioEnabled != 0) {
        player->audioEnabled = 0;
        PSMTXRotRad(rotation, 'y', MTXDegToRad(player->rotationY));
        PSMTXMultVec(rotation, &lbl_1_rodata_810, &effectPosition);
        effectPosition.x += player->position.x;
        effectPosition.z += player->position.z;
        new RotatedSpriteEffect(&effectPosition, player->rotationY);
    }
}

// Updates the crane angle and moves its character to the player position.
void fn_1_DA98(PlayerState *player, Vec *rotation)
{
    f32 angle;
    f32 distance;

    if (player->countdown > 0 && player->angleEnabled != 0) {
        angle = 360.0f * static_cast<f32>(player->countdown & 0x1F) /
                32.0f -180.0f;
        distance = static_cast<f32>(player->countdown) / 10.0f;
        rotation->x = static_cast<f32>(distance * cos(M_PI * static_cast<f64>(angle) /
                                           180.0));
        rotation->z = static_cast<f32>(distance * sin(M_PI * static_cast<f64>(angle) /
                                           180.0));
    } else {
        player->angleEnabled = 0;
        rotation->x = rotation->z = 0.0f;
    }
    rotation->y = player->rotationY;
    player->craneModel.setRotation(rotation);
    player->craneModel.setPosition(&player->position);
}

// Per-player sound effect IDs for the ground sprite.
const s32 lbl_1_rodata_828[4] = { 1694, 1695, 1696, 1697 };

// Updates the model effect and spawns effects at height transitions.
void fn_1_DC38(PlayerState *player, Vec *position, Vec *rotation)
{
    Vec modelPosition;
    s16 modelId;
    ParticleStrip *strip;
    s16 craneModelId = static_cast<s16>(player->craneModel.modelId);
    modelId = craneModelId;
    Hu3DModelObjPosGet(modelId, "Crane05S", &modelPosition);
    position->x = modelPosition.x;
    position->y = modelPosition.y;
    position->z = modelPosition.z;
    position->y -= player->effectHeight;
    if (position->y < 0.0f && (player->updateCount & 0x1) == 0) {
        new EffectNode(position, 250.0f);
    }
    player->timedTransitionModel.setRotation(rotation);
    player->timedTransitionModel.setPosition(position);
    if (player->effectEnabled != 0) {
        player->effectEnabled = 0;
        new RadialBurstModel(position);
    }
    if (position->y < 100.0f) {
        if (player->belowSpriteThreshold == 0) {
            player->belowSpriteThreshold = 1;
            new GroundSpriteEffect(position);
            soundPlayAtWithVolume(1, lbl_1_rodata_828[player->playerIndex], position);
        }
    } else {
        if (player->belowSpriteThreshold != 0) {
            player->belowSpriteThreshold = 0;
            new GroundSpriteEffect(position);
        }
    }
    strip = player->particleStrip;
    strip->start.x = modelPosition.x;
    strip->start.y = modelPosition.y;
    strip->start.z = modelPosition.z;
    strip->end.x = position->x;
    strip->end.y = position->y;
    strip->end.z = position->z;
}

// Selects and starts the model for the current transition state.
static inline s16 selectModelMotion(PlayerState *player, Point3d *soundPosition)
{
    s16 motionModelHandle;
    s16 secondStateModelId;
    s16 firstStateModelId;
    s16 result;

    result = -1;
    if (player->motionEnabled != 0) {
        switch (player->currentMotion) {
        case 0:
            firstStateModelId = static_cast<s16>(player->firstStateModel.modelId);
            result = firstStateModelId;
            break;
        case 1:
            secondStateModelId = static_cast<s16>(player->secondStateModel.modelId);
            result = secondStateModelId;
            break;
        case 2:
            motionModelHandle = static_cast<s16>(player->motionModel.modelId);
            result = motionModelHandle;
            if (player->transitionState == 0) {
                player->transitionState = 1;
                fn_80026A44(result, 0x40000002);
                fn_800269E8(result, 0x40000001);
                fn_8002D414(result, 0.0f);
                fn_8002C95C(result, player->firstMotion);
            }
            if (player->transitionState == 2) {
                player->transitionState = 3;
                fn_80026A44(result, 0x40000002);
                fn_800269E8(result, 0x40000001);
                fn_8002D414(result, 0.0f);
                fn_8002C95C(result, player->secondMotion);
                soundPlayAt(1, MSM_SE_GUIDE_OBJECT_CATCH, soundPosition);
            }
        }
    } else {
        player->motionModel.setAttribute(0x1);
        player->firstStateModel.setAttribute(0x1);
        player->secondStateModel.setAttribute(0x1);
    }
    return result;
}

// Selects the transition model and applies its motion attributes.
// Nothing in this minigame calls this.
s16 fn_1_DE44(PlayerState *player, Point3d *soundPosition)
{
    return selectModelMotion(player, soundPosition);
}

// Updates the selected model motion pause state.
void fn_1_DFC0(PlayerState *player, s16 modelId, f32 height)
{
    s16 storedModelId = static_cast<s16>(player->secondStateModel.modelId);

    if (modelId == storedModelId) {
        if (height < 0.0f) {
            fn_8002D414(modelId, 0.0f);
            fn_800269E8(modelId, 0x40000002);
            player->motionState = 0;
            return;
        }
        if (player->motionState == 0) {
            player->motionState = 1;
            fn_8002D414(modelId, 0.0f);
            fn_80026A44(modelId, 0x40000002);
            fn_80026A44(modelId, 0x40000001);
        }
    }
}

// Model offset before applying the crane rotation.
const Vec lbl_1_rodata_83C = { 0.0f, 0.0f, -160.0f };

// Blends from below the target to the rotated offset from the player.
void fn_1_E0A8(PlayerState *player, s16 modelId,
               Vec *targetPosition, Vec *angles, f32 blend)
{
    Mtx firstRotation;
    Mtx secondRotation;
    Vec baseOffset = lbl_1_rodata_83C;
    Vec blendedPosition;
    Vec rotatedOffset;
    f32 remaining = 1.0f - blend;

    PSMTXRotRad(firstRotation, 'X', MTXDegToRad(angles->x));
    PSMTXRotRad(secondRotation, 'Y', MTXDegToRad(angles->y));
    PSMTXConcat(secondRotation, firstRotation, firstRotation);
    PSMTXRotRad(secondRotation, 'Z', MTXDegToRad(angles->z));
    PSMTXConcat(secondRotation, firstRotation, firstRotation);
    PSMTXMultVecSR(firstRotation, &baseOffset, &rotatedOffset);
    rotatedOffset.x += player->position.x;
    rotatedOffset.y += player->position.y;
    rotatedOffset.z += player->position.z;
    blendedPosition.x = (targetPosition->x * remaining) +
                       (rotatedOffset.x * blend);
    blendedPosition.y = ((targetPosition->y - 170.0f) * remaining) +
                        (rotatedOffset.y * blend);
    blendedPosition.z = (targetPosition->z * remaining) +
                        (rotatedOffset.z * blend);
    fn_80026760(modelId, blendedPosition.x, blendedPosition.y,
                    blendedPosition.z);
    fn_80026808(modelId, 0.0f, player->rotationY, 0.0f);
}

// Returns the model used for the second motion.
static inline s16 getSecondMotionModelId(PlayerState *player)
{
    return static_cast<s16>(player->secondStateModel.modelId);
}

// Returns the model used for the timed motion.
static inline s16 getTimedMotionModelId(PlayerState *player)
{
    return static_cast<s16>(player->timedTransitionModel.modelId);
}

// Returns the model whose transition is reset.
static inline s16 getResetMotionModelId(PlayerState *player)
{
    return static_cast<s16>(player->resetMotionModel.modelId);
}

// Per-player sound effect IDs for the motion reset.
const s32 lbl_1_rodata_84C[4] = { 1689, 1690, 1691, 1692 };

// Updates the model transition and its associated score entry.
void fn_1_E298(PlayerState *player, Vec *targetPosition,
               Vec *angles)
{
    s16 modelId = selectModelMotion(player,
                            targetPosition);

    if (modelId >= 0 && player->currentMotion >= 0) {
        if (player->elapsed < secondsToFrames(5.33f)) {
            fn_1_DFC0(player, modelId, targetPosition->y);
            if (modelId == getSecondMotionModelId(player) && targetPosition->y > 0.0f) {
                Vec position = *targetPosition;

                // Spawn the burst at the model position, 170 units below the target.
                position.y -= 170.0f;
                new RotatingBurstModel(&position);
            }
            fn_80026760(modelId, targetPosition->x,
                            targetPosition->y - 170.0f,
                            targetPosition->z);
            fn_80026808(modelId, 0.0f, player->rotationY, 0.0f);
            if (player->modelAttributeFlag == 1 && player->currentMotion == 2) {
                fn_800269E8(modelId, 0x1);
            } else {
                fn_80026A44(modelId, 0x1);
            }
            if (player->motionResetPending != 0) {
                s16 resetMotionModelId;

                player->motionResetPending = 0;
                resetMotionModelId = getResetMotionModelId(player);
                Vec effectPosition = *targetPosition;

                // Play the sound at the model position, 170 units below the target.
                effectPosition.y -= 170.0f;
                soundPlayAt(1, lbl_1_rodata_84C[player->playerIndex],
                           &effectPosition);
                fn_80026760(resetMotionModelId, targetPosition->x,
                                targetPosition->y - 170.0f,
                                targetPosition->z);
                fn_80026808(resetMotionModelId, 0.0f,
                                player->rotationY, 0.0f);
                fn_80026A44(resetMotionModelId, 0x1);
                fn_80026A44(resetMotionModelId, 0x40000002);
                fn_80026A44(resetMotionModelId, 0x40000001);
                fn_8002D414(resetMotionModelId, 0.0f);
            }
        } else {
            f32 blend;
            if (player->transitionStarted == 0) {
                player->transitionStarted = 1;
                fn_8002D730(getTimedMotionModelId(player),
                    0.8 * player->motionDuration, player->motionDuration);
                fn_8002D414(getTimedMotionModelId(player), 0.8 * player->motionDuration);
            }
            blend = (player->elapsed - secondsToFrames(5.33f)) / secondsToFrames(0.2f);
            if (blend > 1.0f) {
                blend = 1.0f;
                MinigameSingleton<ScoreDisplay>::getChecked().fn_1_12274(
                    player->playerIndex, player->score);
            }
            fn_1_E0A8(player, modelId,
                      targetPosition, angles, blend);
        }
    }
}

// Updates rumble and refreshes the active flash effect.
void fn_1_E9AC(PlayerState *player)
{
    s32 rumbleType;
    bool noRumblePad;
    u8 refreshEffect;

    rumbleType = 0;
    noRumblePad = fn_1_1186C(player);
    if (player->transitionCountdown > 0) {
        rumbleType = 2;
        --player->transitionCountdown;
    }
    refreshEffect = 0;
    if (player->candidateChanged != 0) {
        player->candidateChanged = 0;
        player->effectTimer = static_cast<s32>(secondsToFrames(0.2f));
        refreshEffect = 1;
    }
    if (player->hasCandidate) {
        player->hasCandidate = false;
        if (noRumblePad) {
            --player->effectTimer;
            if (player->effectTimer > 0) {
                rumbleType = 1;
            } else if (static_cast<f32>(player->effectTimer) < -secondsToFrames(0.5f)) {
                player->effectTimer = static_cast<s32>(secondsToFrames(0.2f));
                refreshEffect = 1;
            }
        } else {
            refreshEffect = 1;
        }
    }
    if (refreshEffect != 0 && !noRumblePad) {
        fn_1_9370(player->timedFlash);
    }
    if (player->padNo >= 0 && rumbleType > 0) {
        if (rumbleType == 1) {
            fn_8006A030(static_cast<s16>(player->playerIndex), 20, 7, 3);
        } else {
            fn_8006A030(static_cast<s16>(player->playerIndex), 20, 20, 0);
        }
    }
    ++player->collisionTimer;
}

// Separates overlapping players, clamps their positions, and updates the effects.
void fn_1_EB48(PlayerState *player)
{
    void fn_1_E9AC(PlayerState *player);
    void fn_1_D894(PlayerState *player);
    u8 *clampFlags;

    player->clampFlags = 0;
    fn_1_EFAC(player);
    clampFlags = &player->clampFlags;
    *clampFlags |= fn_1_116D8(player);
    // Request 2 starts play after the start message clears.
    if (currentRequest() == 2) {
        fn_1_E9AC(player);
    }
    fn_1_D894(player);
}

// Separates two points when they lie inside the collision radius.
u8 fn_1_EBC0(PlayerState *, Vec *start, Vec *end, Vec *startOffset, Vec *endOffset)
{
    f32 deltaX;
    f32 deltaZ;
    f64 distance;
    f64 halfDeltaX;
    f64 halfDeltaZ;
    f64 midpointX;
    f64 midpointZ;

    // Minimum separation between overlapping players.
    static f64 separationDistance = 2.3 * 100;

    deltaX = end->x - start->x;
    deltaZ = end->z - start->z;
    distance = sqrt(static_cast<f64>(deltaX) * static_cast<f64>(deltaX)
        + static_cast<f64>(deltaZ) * static_cast<f64>(deltaZ));
    if (distance > separationDistance || distance == 0.0) {
        startOffset->x = 0.0f;
        startOffset->y = 0.0f;
        startOffset->z = 0.0f;
        endOffset->x = 0.0f;
        endOffset->y = 0.0f;
        endOffset->z = 0.0f;
        return 0;
    }

    halfDeltaX = static_cast<f64>(deltaX) * separationDistance / distance;
    halfDeltaZ = static_cast<f64>(deltaZ) * separationDistance / distance;
    midpointX = static_cast<f64>(end->x + start->x) * 0.5;
    midpointZ = static_cast<f64>(end->z + start->z) * 0.5;
    halfDeltaX *= 0.5;
    halfDeltaZ *= 0.5;
    startOffset->x = static_cast<f32>((midpointX - halfDeltaX) - static_cast<f64>(start->x));
    startOffset->z = static_cast<f32>((midpointZ - halfDeltaZ) - static_cast<f64>(start->z));
    endOffset->x = static_cast<f32>((midpointX + halfDeltaX) - static_cast<f64>(end->x));
    endOffset->z = static_cast<f32>((midpointZ + halfDeltaZ) - static_cast<f64>(end->z));
    return 1;
}

// Separates colliding player points and triggers collision feedback.
void fn_1_EFAC(PlayerState *player)
{
    Vec sourcePoints[4] = {
        { 0.0f, 0.0f, 180.0f },
        { 0.0f, 0.0f, 60.0f },
        { 0.0f, 0.0f, -60.0f },
        { 0.0f, 0.0f, -180.0f },
    };
    Mtx ownerRotation;
    Mtx otherRotation;
    Vec ownerPoints[5];
    Vec otherPoints[5];
    Node *end;
    PlayerState *other;
    f32 deltaX;
    f32 deltaZ;
    f64 centerDistance;
    Vec ownerOffset;
    Vec otherOffset;
    s32 pointIndex;
    s32 adjustedPointIndex;
    s32 ownerPointIndex;
    s32 otherPointIndex;
    u8 playCollisionSound;
    u8 collided;
    other = lbl_1_bss_1BC.front();
    end = lbl_1_bss_1BC.end_node();
    for (; other != end; other = other->next()) {
        if (player != other) {
            deltaX = other->position.x - player->position.x;
            deltaZ = other->position.z - player->position.z;
            centerDistance = sqrt(static_cast<f64>(deltaX) * static_cast<f64>(deltaX)
                + static_cast<f64>(deltaZ) * static_cast<f64>(deltaZ));
            if (!(centerDistance > 900.0) && centerDistance != 0.0) {
                PSMTXRotRad(ownerRotation, 'y', MTXDegToRad(player->rotationY));
                PSMTXRotRad(otherRotation, 'y', MTXDegToRad(other->rotationY));
                for (pointIndex = 0; pointIndex < 4; ++pointIndex) {
                    PSMTXMultVec(ownerRotation, &sourcePoints[pointIndex],
                        &ownerPoints[pointIndex]);
                    PSMTXMultVec(otherRotation, &sourcePoints[pointIndex],
                        &otherPoints[pointIndex]);
                    ownerPoints[pointIndex].x += player->position.x;
                    ownerPoints[pointIndex].z += player->position.z;
                    otherPoints[pointIndex].x += other->position.x;
                    otherPoints[pointIndex].z += other->position.z;
                }
                ownerPoints[4].x =
                    static_cast<f32>(0.25 * static_cast<f64>((3.0f * player->jointPosition.x)
                        + ownerPoints[0].x));
                ownerPoints[4].z =
                    static_cast<f32>(0.25 * static_cast<f64>((3.0f * player->jointPosition.z)
                        + ownerPoints[0].z));
                otherPoints[4].x =
                    static_cast<f32>(0.25 * static_cast<f64>((3.0f * other->jointPosition.x)
                        + otherPoints[0].x));
                otherPoints[4].z =
                    static_cast<f32>(0.25 * static_cast<f64>((3.0f * other->jointPosition.z)
                        + otherPoints[0].z));
                for (ownerPointIndex = 0; ownerPointIndex < 5; ++ownerPointIndex) {
                    for (otherPointIndex = 0; otherPointIndex < 5; ++otherPointIndex) {
                        collided = fn_1_EBC0(player, &ownerPoints[ownerPointIndex],
                                                    &otherPoints[otherPointIndex],
                                                    &ownerOffset, &otherOffset);
                        player->position.x += ownerOffset.x;
                        player->position.z += ownerOffset.z;
                        other->position.x += otherOffset.x;
                        other->position.z += otherOffset.z;
                        player->jointPosition.x += ownerOffset.x;
                        player->jointPosition.z += ownerOffset.z;
                        other->jointPosition.x += otherOffset.x;
                        other->jointPosition.z += otherOffset.z;
                        for (adjustedPointIndex = 0; adjustedPointIndex < 5; ++adjustedPointIndex) {
                            ownerPoints[adjustedPointIndex].x += ownerOffset.x;
                            ownerPoints[adjustedPointIndex].z += ownerOffset.z;
                            otherPoints[adjustedPointIndex].x += otherOffset.x;
                            otherPoints[adjustedPointIndex].z += otherOffset.z;
                        }
                        if (collided != 0) {
                            playCollisionSound = 0;
                            if (static_cast<f32>(player->collisionTimer) > secondsToFrames(0.5f)) {
                                player->collisionTimer = 0;
                                playCollisionSound = 1;
                            }
                            if (static_cast<f32>(other->collisionTimer) > secondsToFrames(0.5f)) {
                                other->collisionTimer = 0;
                                playCollisionSound = 1;
                            }
                            if (playCollisionSound != 0) {
                                // Play the contact sound.
                                soundPlayAtWithVolume(1, 0x687, &player->position);
                            }
                        }
                    }
                }
                fn_1_116D8(player);
                fn_1_116D8(other);
            }
        }
    }
}

// Returns the current player model identifier.
static inline s16 motionModelId(PlayerState *player)
{
    return static_cast<s16>(player->bodyModel.modelId);
}

// Starts the requested character motion when its index changes.
void fn_1_FA64(PlayerState *player, s32 motionId, u8 queued)
{
    s16 modelId;
    s16 selectedMotion;

    modelId = motionModelId(player);
    selectedMotion = player->m_mot_id.at(motionId);
    if (player->characterMotion != motionId) {
        fn_80026A44(modelId, 0x40000002);
        fn_8005C2C8(static_cast<s16>(player->character), selectedMotion, 0.0f, 8.0f,
                           queued == 0 ? 0x40000001 : 0);
        player->characterMotion = motionId;
    }
}

// Returns an inactive player to its idle input callback.
void PlayerState::fn_1_FB24()
{
    void (PlayerState::*nextUpdate)();

    this->unkA8 = 0;
    fn_1_FA64(this, 0, 0);
    // Request 2 starts play after the start message clears.
    if (currentRequest() == 2) {
        nextUpdate = &PlayerState::chooseMotionInput;
        this->callback = nextUpdate;
    }
}

// Reads human or CPU input and chooses movement or a model motion.
void PlayerState::chooseMotionInput()
{
    u8 trigger = 0;
    s32 candidateState;
    Vec direction;
    this->unk9C = 1;
    this->unkA8 = 0;
    candidateState = fn_1_14E80(&MinigameSingleton<MotionCandidateTable>::getChecked(),
                                 &this->position);
    if (this->padNo < 0) {
        switch (this->comDif) {
        case 0:
            if ((static_cast<s32>(RandomUnsigned()) & 0xFF) == 0) {
                trigger = 1;
            }
            break;
        case 1:
            if ((static_cast<s32>(RandomUnsigned()) & 0xFF) == 0) {
                trigger = 1;
            }
            if (candidateState >= 0) {
                trigger = 1;
            }
            break;
        case 2:
            if ((static_cast<s32>(RandomUnsigned()) & 0xFF) == 0) {
                trigger = 1;
            }
            if (candidateState >= 0) {
                trigger = 1;
            }
            break;
        case 3:
            if ((static_cast<s32>(RandomUnsigned()) & 0x1FF) == 0) {
                trigger = 1;
            }
            if (candidateState >= 0) {
                trigger = 1;
            }
            break;
        default:
            // No CPU trigger for this difficulty.
            break;
        }
        bool candidateFound;
        if (candidateState >= 0) {
            candidateFound = true;
        } else {
            candidateFound = false;
        }
        this->hasCandidate = candidateFound;
        if (this->previousCandidateState != candidateState) {
            this->previousCandidateState = candidateState;
            if (this->previousCandidateState >= 0) {
                this->candidateChanged = 1;
            }
        }
    } else {
        bool candidateFound;
        if (candidateState >= 0) {
            candidateFound = true;
        } else {
            candidateFound = false;
        }
        this->hasCandidate = candidateFound;
        if (this->previousCandidateState != candidateState) {
            this->previousCandidateState = candidateState;
            if (this->previousCandidateState >= 0) {
                this->candidateChanged = 1;
            }
        }
        if ((lbl_802C0008[this->padNo] & PAD_BUTTON_A) != 0) {
            trigger = 1;
        }
    }
    if (trigger != 0) {
        this->fn_1_FEE0();
        return;
    }
    this->fn_1_11554(&direction);
    if (direction.x * direction.x + direction.z * direction.z > 0.01) {
        this->audioEnabled = 1;
    }
    fn_1_10848(this, &direction);
    fn_1_FA64(this, 0, 0);
}

// Per-player sound effect IDs for starting the timed motion.
const s32 lbl_1_rodata_8E8[4] = { 1678, 1679, 1680, 1681 };

// Starts the selected model motion and installs its update callback.
void PlayerState::fn_1_FEE0()
{
    s32 motion;
    void (PlayerState::*nextUpdate)();

    motion = fn_1_14CD4(&MinigameSingleton<MotionCandidateTable>::getChecked(), &this->position);
    this->currentMotion = motion;

    switch (this->currentMotion) {
    case 0:
    case 1: {
        s16 modelId = this->craneModel;
        fn_80026A44(modelId, 0x40000002);
        fn_80026A44(modelId, 0x40000001);
        // The first segment ends on frame 390.
        fn_8002D730(modelId, 0.0f, 390.0f);
        fn_8002D414(modelId, 0.0f);
        modelId = this->boxModel;
        fn_80026A44(modelId, 0x40000002);
        fn_80026A44(modelId, 0x40000001);
        fn_8002D414(modelId, 0.0f);
        break;
    }
    case 2:
    case -1: {
        s16 modelId = this->craneModel;
        fn_80026A44(modelId, 0x40000002);
        fn_80026A44(modelId, 0x40000001);
        // This segment ends on frame 240.
        fn_8002D730(modelId, 0.0f, 240.0f);
        // Queries the motion duration and discards the result.
        fn_8002D530(modelId);
        fn_8002D414(modelId, 0.0f);
        break;
    }
    }

    this->active = 1;
    this->transitionState = 0;
    this->elapsed = 0.0f;
    this->soundCuePlayed = 0;
    this->soundId = soundPlayAtWithVolume(1, lbl_1_rodata_8E8[this->playerIndex],
                               &this->position);
    nextUpdate = &PlayerState::advanceMotionState;
    this->callback = nextUpdate;
}

// Per-player sound effect IDs played after 5.1 seconds.
const s32 lbl_1_rodata_900[4] = { 1683, 1684, 1685, 1686 };

// Advances the active model motion and its sound and transition state.
void PlayerState::advanceMotionState()
{
    s32 motionFrame;

    motionFrame = static_cast<s32>(this->elapsed++ - secondsToFrames(1.33f));
    if (motionFrame < 0) {
        this->motionEnabled = 0;
        return;
    }
    // Number of frames in the main part of the motion.
    static s32 motionLength = static_cast<s32>(secondsToFrames(0.55f));
    if (this->soundCuePlayed == 0 &&
        this->elapsed > secondsToFrames(5.1f)) {
        soundPlayAt(1, lbl_1_rodata_900[this->playerIndex], &this->position);
        this->soundCuePlayed = 1;
    }
    // Raise and lower the effect at 1700 world units per second.
    if (motionFrame < motionLength) {
        this->effectHeight = (1700.0f * static_cast<f32>(motionFrame)) /
                           secondsToFrames(1.0f);
        this->motionActive = 0;
        this->transitionStarted = 0;
        this->motionEnabled = 0;
        this->motionTimer = 0;
    } else {
        if (this->motionTimer == 0) {
            this->effectEnabled = 1;
            if (this->currentMotion == 0) {
                this->score += 1;
            }
            if (this->currentMotion == 1) {
                this->score += 3;
            }
        }
        if (this->motionTimer == motionLength) {
            this->effectEnabled = 1;
        }
        this->motionTimer += 1;
        if (this->motionActive == 0) {
            this->motionActive = 1;
            s16 currentModel;
            currentModel = this->timedTransitionModel;
            fn_80026A44(currentModel, 0x40000002);
            fn_80026A44(currentModel, 0x40000001);
            fn_8002D730(currentModel, 0.0f,
                                  this->motionDuration / 2.0f);
            fn_8002D414(currentModel, 0.0f);
            this->motionEnabled = 1;
        }
        motionFrame = static_cast<s32>(static_cast<f32>(motionFrame) -
                        (static_cast<f32>(motionLength) + secondsToFrames(0.56f)));
        if (motionFrame < 0) {
            motionFrame = 0;
        }
        this->effectHeight =
            (1700.0f * static_cast<f32>(motionLength - motionFrame)) /
            secondsToFrames(1.0f);
        if (this->effectHeight < 0.0f) {
            this->effectHeight = 0.0f;
        }
    }
    if (fn_8002D75C(this->craneModel) != 0) {
        HuAudFXStop(this->soundId);
        this->soundId = -1;
        this->active = 0;
        switch (this->currentMotion) {
        case 0:
        case 1:
            this->motionEnabled = 0;
            if (this->callbackDisabled == 0) {
                void (PlayerState::*nextUpdate)() = &PlayerState::chooseMotionInput;
                this->callback = nextUpdate;
            }
            break;
        case 2:
            this->countdownActive = 1;
            this->modelAttributeFlag = 0;
            this->unkD2 = 0;
            this->countdown = static_cast<s32>(secondsToFrames(3.0f));
            if (this->callbackDisabled == 0) {
                void (PlayerState::*nextUpdate)() = &PlayerState::advanceMotionCountdown;
                this->callback = nextUpdate;
            }
            break;
        default:
            this->motionEnabled = 0;
            fn_8002D730(this->timedTransitionModel,
                0.7 * this->motionDuration,
                         this->motionDuration);
            fn_8002D414(this->timedTransitionModel,
                0.7 * this->motionDuration);
            if (this->callbackDisabled == 0) {
                void (PlayerState::*nextUpdate)() = &PlayerState::finishMotion;
                this->callback = nextUpdate;
            }
            break;
        }
    } else {
        soundPanTo(1, this->soundId, &this->position);
        soundSetVolumeAt(1, this->soundId, &this->position);
    }
}

// Advances the motion countdown and returns to input when it expires.
void PlayerState::advanceMotionCountdown()
{
    void (PlayerState::*nextUpdate)();

    if (this->transitionState == 1) {
        this->transitionState = 2;
    }

    if (this->countdown-- < 0) {
        this->countdown = 0;
        this->countdownActive = 0;
        this->motionEnabled = 0;
        this->modelAttributeFlag = 0;
        if (this->callbackDisabled == 0) {
            nextUpdate = &PlayerState::chooseMotionInput;
            this->callback = nextUpdate;
        }
    }

    if (static_cast<f32>(this->countdown) == secondsToFrames(2.0f)) {
        fn_8002D730(this->timedTransitionModel,
                              0.7 * this->motionDuration,
                              this->motionDuration);
        fn_8002D414(this->timedTransitionModel,
                          0.7 * this->motionDuration);
        this->transitionCountdown = static_cast<s32>(secondsToFrames(0.5f));
        this->modelAttributeFlag = 1;
        this->motionResetPending = 1;
        this->angleEnabled = 1;
    }
}

// Returns to the input callback after the model motion ends.
void PlayerState::finishMotion()
{
    if (fn_8002D75C(this->timedTransitionModel) != 0) {
        void (PlayerState::*nextUpdate)() = &PlayerState::chooseMotionInput;
        this->callback = nextUpdate;
    }
}

// Chooses a random position on the horizontal plane.
static inline void randomizeTargetPosition(Vec *position)
{
    position->x = static_cast<f32>(100.0 *
        (11.0 *
         static_cast<f64>((RandomFloat() -0.5f) * 2.0f)));
    position->y = 0.0f;
    position->z = static_cast<f32>(100.0 *
        (11.0 *
         static_cast<f64>((RandomFloat() -0.5f) * 2.0f)));
}

// Computes a horizontal unit vector for the supplied heading.
static inline void facingDirection(Vec *facing, f32 angleRadians)
{
    f64 cosine;
    f64 sine;
    cosine = cos(static_cast<f64>(angleRadians));
    sine = sin(static_cast<f64>(angleRadians));
    facing->x = sine;
    facing->y = 0.0f;
    facing->z = cosine;
}

// Turns and moves the player according to the movement vector.
bool fn_1_10848(PlayerState *player, Vec *movement)
{
    Vec facing;
    Vec adjustedMovement;
    f32 movementScale;
    f32 angleRadians;
    f32 alignment;
    f32 scaleFactor;

    player->movementLength = PSVECMag(movement);
    if (player->movementLength < 0.01f) {
        fn_8002D86C(motionModelId(player), 1.0f);
        return false;
    }

    movementScale = player->movementLength;
    if (player->movementLength < 0.5) {
        movementScale = 2.0f * player->movementLength;
    }
    fn_8002D86C(motionModelId(player), movementScale);

    player->targetAngle = static_cast<f32>(180.0 *
        (atan2(static_cast<f64>(movement->x), static_cast<f64>(movement->z)) /
         M_PI));
    player->rotationY += AngleStepToward(player->targetAngle, player->rotationY,
                                        100.0f / 60.0f);

    angleRadians = static_cast<f32>((M_PI *
        static_cast<f64>(player->rotationY)) / 180.0);
    facingDirection(&facing, angleRadians);
    alignment = PSVECDotProduct(&facing, movement);
    if (alignment < 0.0f) {
        return true;
    }

    scaleFactor = 7.0f;
    scaleFactor *= (100.0 / 60.0) * alignment;
    PSVECScale(&facing, &adjustedMovement,
               scaleFactor * player->movementLength);
    PSVECAdd(&adjustedMovement, &player->position, &player->position);
    return true;
}

// Other scene quadrants, indexed by the current quadrant.
const s32 lbl_1_rodata_948[4][3] = { { 1, 2, 3 }, { 0, 2, 3 }, { 1, 0, 3 }, { 1, 2, 0 } };

// Scene coordinates of each quadrant.
const Vec lbl_1_rodata_978[4] = {
    { -1100.0f, 0.0f, -1100.0f },
    { 0.0f, 0.0f, -1100.0f },
    { -1100.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f },
};

// Chooses the next CPU movement target and resets its countdown.
void PlayerState::fn_1_10C04()
{
    s32 quadrantIndex;
    s32 randomIndex;
    s32 selected;
    switch (this->comDif) {
    case 0:
        randomizeTargetPosition(&this->targetPosition);
        break;
    case 3:
        if ((static_cast<s32>(RandomUnsigned()) & 0x1) != 0) {
            this->selectedPosition =
                fn_1_14A30(&MinigameSingleton<MotionCandidateTable>::getChecked(),
                           &this->targetPosition, &this->position, this->selectedPosition);
            break;
        }
        // Falls through when the random bit is clear.
    case 2:
        if ((static_cast<s32>(RandomUnsigned()) & 0x1) != 0) {
            this->selectedPosition =
                fn_1_15054(&MinigameSingleton<MotionCandidateTable>::getChecked(),
                           &this->targetPosition);
            break;
        }
        // Falls through when the random bit is clear.
    case 1:
        // This quadrant selection uses X and Y.
        quadrantIndex = (this->position.x < 0.0f ? 0 : 1) + (this->position.y < 0.0f ? 0 : 2);
        randomIndex = (static_cast<u16>(RandomUnsigned()) * 3) >> 16;
        selected = lbl_1_rodata_948[quadrantIndex][randomIndex];
        this->targetPosition.x =
            static_cast<f32>(lbl_1_rodata_978[selected].x
                + 100.0 * (11.0 * static_cast<f64>(RandomFloat())));
        this->targetPosition.y = 0.0f;
        this->targetPosition.z =
            static_cast<f32>(lbl_1_rodata_978[selected].z
                + 100.0 * (11.0 * static_cast<f64>(RandomFloat())));
        break;
    }
    this->updateCountdown = static_cast<s32>(secondsToFrames(5.0f));
}

// Moves the CPU player toward its target and refreshes expired targets.
void PlayerState::fn_1_10EF4(Vec *direction)
{
    f32 distance;
    f32 targetDistance;

    PSVECSubtract(&this->targetPosition, &this->position, direction);
    distance = PSVECMag(direction);
    targetDistance = distance;
    if (targetDistance > 20.0f) {
        PSVECScale(direction, direction, 1.0f / targetDistance);
        if (--this->updateCountdown >= 0) {
            return;
        }
        this->fn_1_10C04();
    } else {
        direction->x = direction->z = 0.0f;
        this->fn_1_10C04();
    }
}

// Builds the player movement vector from CPU state or the pad stick.
void PlayerState::fn_1_11554(Vec *direction)
{
    f32 stickX;
    f32 stickY;

    direction->x = 0.0f;
    direction->y = 0.0f;
    direction->z = 0.0f;
    // Request 2 starts play after the start message clears.
    if (currentRequest() == 2) {
        stickX = 0.0f;
        stickY = 0.0f;
        if (this->padNo < 0) {
            this->fn_1_10EF4(direction);
            if (this->comDif == 1) {
                PSVECScale(direction, direction, 0.6f);
            }
            if (this->comDif == 2) {
                PSVECScale(direction, direction, 0.7f);
            }
            if (this->comDif == 3) {
                PSVECScale(direction, direction, 0.8f);
            }
        } else {
            // Read the pad axes for a human player.
            stickX = lbl_8024C7D8[this->padNo];
            stickY = lbl_8024C7E8[this->padNo];
            vectorSet(direction, stickX, 0.0f, -stickY);
        }
    }
}

// Clamps the player position to the scene boundaries.
s32 fn_1_116D8(PlayerState *player)
{
    s32 clamped;

    clamped = 0;
    if (player->position.x > 1100.0f) {
        player->position.x = 1100.0f;
        clamped = 1;
    }
    if (player->position.z > 950.0f) {
        player->position.z = 950.0f;
        clamped = 1;
    }
    if (player->position.x < -1100.0f) {
        player->position.x = -1100.0f;
        clamped = 1;
    }
    if (player->position.z < -1100.0f) {
        player->position.z = -1100.0f;
        clamped = 1;
    }
    return clamped;
}

// Measures the player heading from the scene origin.
f32 fn_1_117A4(const PlayerState *player)
{
    f32 x = player->position.x;
    f32 z = player->position.z;
    f32 angle = static_cast<f32>(90.0 +
                      180.0 * (atan2(static_cast<f64>(x), static_cast<f64>(z)) /
                               M_PI));
    if (angle > 180.0f) {
        angle -= 360.0f;
    }
    return angle;
}

// Checks for enabled rumble with no controller able to rumble.
bool fn_1_1186C(PlayerState *)
{
    return noRumbleController();
}
