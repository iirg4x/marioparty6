#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/framework/frame_time_inline.h"
#include "REL/framework/list_object.h"
#include "REL/m603DLL/draw_node.h"
#include "REL/m603DLL/draw_owner.h"
#include "REL/m603DLL/ufo.h"
#include "REL/m603DLL/player_position.h"
#include "msm_se.h"
extern "C" {
#include "dolphin/mtx.h"
#include "REL/m603DLL.h"
#include "REL/framework/assertion.h"
#include "dolphin/pad.h"
}
#include <cmath>
#include <cstdio>
extern "C" {
#include "dolphin/types.h"
#include "game/main.h"
#include "game/object.h"
#include "game/charman.h"
#include "game/hu3d.h"
#include "game/gamework.h"
#include "game/wipe.h"
#include "game/mg/timer.h"
}
#include "REL/framework/linknode.h"
#include "REL/framework/random.h"
#include "REL/m603DLL/sprite_fade.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/model.h"
extern "C" {
#include "game/data.h"
extern "C" {
#include "game/pad.h"
}
#include "game/audio.h"
}
#include "REL/m603DLL/player.h"
#include "REL/m603DLL/score_owner.h"

// Every player's model owner; and the sole highest scorer once the scores are ranked (zero while
// the top scores are tied or before the ranking).
extern "C" {
PlayerModelOwner *lbl_1_bss_9C;
ListRoot<PlayerModelOwner> lbl_1_bss_AC;
}

/* Set the supplied position to (0, 0, -250). */
void fn_1_5994(Vec *vector)
{
    vector->x = 0.0f;
    vector->y = 0.0f;
    vector->z = -250.0f;
}
/* Delete all player model owners. */
void fn_1_59C8()
{
    while (!lbl_1_bss_AC.empty()) {
        delete lbl_1_bss_AC.back();
    }
}
/* Create the player models and load their four character motions. */
PlayerModelOwner::PlayerModelOwner(s32 playerIndex, bool alternate)
    : active(1),
      playerIndex(playerIndex),
      characterId(lbl_80265728[playerIndex].charNo),
      controllerId(lbl_80265728[playerIndex].type != 0 ? -1 : lbl_80265728[playerIndex].padNo),
      difficulty(lbl_80265728[playerIndex].comDif),
      callbackCount(1),
      firstModel(lbl_80265728[playerIndex].charNo, 0),
      secondModel(alternate ? DATANUM(0x3e0000, 8) : DATANUM(0x3e0000, 9)),
      flag(0),
      selected(0),
      ready(0),
      recordedPlayerValue(0),
      timer54(0x3C),
      motionChangeTimer(0),
      randomMotion(0.0f),
      alternate(alternate)
{
    s16 modelId;
    Vec rotation;
    Vec modelPosition;
    PlayerModelOwner::Callback callback;

    insert4(&lbl_1_bss_AC);
    this->currentMotion = - 1;
    this->m_mot_id.clear();
    this->m_mot_id.reserve(NUMBEROF_MOTION);
    fn_1_61DC(CHARMOT_HSF_c000m1_300);
    fn_1_61DC(CHARMOT_HSF_c000m1_323);
    fn_1_61DC(CHARMOT_HSF_c000m1_306);
    fn_1_61DC(CHARMOT_HSF_c000m1_351);
    assert(m_mot_id.size() == NUMBEROF_MOTION);
    fn_8005B4BC((s16) this->characterId);
    fn_8002DC74(this->m_mot_id.at(0), 1);
    modelId = this->firstModel;
    fn_80026B8C(modelId, 4);
    fn_80026B64(modelId, 1);
    fn_800269E8(modelId, ((1 << 30) | (1 << 1)));
    fn_800269E8(modelId, ((1 << 30) | (1 << 7)));
    fn_800269E8(modelId, ((1 << 30) | (1 << 4)));
    fn_800269E8(modelId, (1 << 14));
    // This initial position is overwritten below after the angle is cleared.
    fn_1_5994(&this->position);

    this->angle = 0.0f;
    this->position.x = 0.0f;
    this->position.y = 0.0f;
    this->position.z = -300.0f;
    rotation.x = 0.0f;
    rotation.y = 180.0f;
    rotation.z = 0.0f;
    this->firstModel.setPosition(&this->position);
    this->firstModel.setRotation(& rotation);
    modelPosition.x = this->position.x;
    modelPosition.y = this->position.y;
    modelPosition.z = this->position.z;
    modelPosition.x -= 50.0f;
    modelPosition.z -= 100.0f;
    this->secondModel.setPosition(& modelPosition);
    this->secondModel.setRotation(& rotation);
    if (playerIndex == 0)
    {
        modelId = (s16)this->firstModel;
        fn_80026A44(modelId, 1);
        fn_80026E44(modelId);
        fn_80026F9C(modelId);
        modelId = (s16)this->secondModel;
        fn_80026A44(modelId, 1);
        fn_80026E44(modelId);
        fn_80026F9C(modelId);
    }
    else
    {
        modelId = (s16)this->firstModel;
        fn_800269E8(modelId, 1);
        modelId = (s16)this->secondModel;
        fn_800269E8(modelId, 1);
    }
    callback = & PlayerModelOwner::fn_1_6BE0;
    this->callback = callback;
}

/* Finish the player model lifetime. */
PlayerModelOwner::~PlayerModelOwner()
{
}

/* Appends the created motion and returns its handle to the caller. */
/* Load a character motion and append its identifier to the motion list. */
HU3D_MOTIONID PlayerModelOwner::fn_1_61DC(unsigned int dataNum)
{
    HU3D_MOTIONID motionId = fn_8005B0B8(this->characterId, dataNum);
    this->m_mot_id.push_back(motionId);
    return motionId;
}

/* Updates all callbacks, then moves the first character to the list's opposite end. */
/* Visit the player list and rotate its first entry to the back. */
void fn_1_6258()
{
    lbl_1_bss_AC.visit(fn_1_6ADC);
    PlayerModelOwner *node = lbl_1_bss_AC.front();
    if (node != lbl_1_bss_AC.end_node()) {
        node->unlink();
        node->insert4(&lbl_1_bss_AC);
    }
}

/* Rank the scores, mark tied winners and report an all-zero tie. */
bool fn_1_62DC()
{
    PlayerModelOwner *entries[4];
    int scores[4];
    PlayerModelOwner *node = lbl_1_bss_AC.front();
    Node *end = lbl_1_bss_AC.end_node();
    int index;
    int pass;
    int sortIndex;
    int firstValue;
    int secondValue;
    int tiedCount;
    int compareIndex;
    int markIndex;

    lbl_1_bss_9C = 0;
    int order[4] = { 0, 1, 2, 3 };
    /* This initialized result is never read. */
    int result = 0;
    index = 0;
    for (; node != end; node = node->next()) {
        entries[index] = node;
        scores[index] = ScoreDisplay::getChecked().fn_1_83E4(node->playerIndex);
        std::printf("chk %d %d\n", index, scores[index]);
        index++;
    }
    for (pass = 0; pass < 3; pass++) {
        for (sortIndex = 0; sortIndex < 3; sortIndex++) {
            if (scores[sortIndex] < scores[sortIndex + 1]) {
                firstValue = scores[sortIndex];
                secondValue = scores[sortIndex + 1];
                scores[sortIndex] = secondValue;
                scores[sortIndex + 1] = firstValue;
                firstValue = order[sortIndex];
                secondValue = order[sortIndex + 1];
                order[sortIndex] = secondValue;
                order[sortIndex + 1] = firstValue;
            }
        }
    }
    tiedCount = 1;
    for (compareIndex = 1; compareIndex < 4; compareIndex++) {
        if (scores[0] == scores[compareIndex]) {
            tiedCount++;
        }
    }
    if (tiedCount == 4 && scores[0] == 0) {
        return true;
    }
    if (tiedCount == 1) {
        lbl_1_bss_9C = entries[order[0]];
    }
    for (markIndex = 0; markIndex < tiedCount; markIndex++) {
        entries[order[markIndex]]->selected = 1;
        MinigameCore::addWinner(entries[order[markIndex]]->playerIndex);
    }
    return false;
}

/* Returns 1; nothing in this module calls it. The result type is not shown. */
int fn_1_658C()
{
    return 1;
}

/* Reports whether exactly four character entries have their ready byte set. */
bool fn_1_6594()
{
    PlayerModelOwner *node = lbl_1_bss_AC.front();
    Node *end = lbl_1_bss_AC.end_node();
    int count = 0;
    for (; node != end; node = node->next()) {
        if (node->ready) ++count;
    }
    return count == 4;
}

/* Collect the character identifiers into a local table whose result is discarded. */
void fn_1_6614()
{
    /* The character identifiers stored below are never read. */
    short characters[4];
    PlayerModelOwner *node = lbl_1_bss_AC.front();
    Node *end = lbl_1_bss_AC.end_node();
    int index = 0;
    for (; node != end; node = node->next()) {
        characters[index++] = node->characterId;
    }
}

/* Clear the active flag of every player model owner in the list. */
void fn_1_6690()
{
    PlayerModelOwner *node = lbl_1_bss_AC.front();
    Node *end = lbl_1_bss_AC.end_node();
    for (; node != end; node = node->next()) {
        node->active = 0;
    }
}

/* Create one score display for each player. */
void fn_1_66F4(void)
{
    Node *sentinel;
    PlayerModelOwner *first;
    PlayerModelOwner *middle;
    PlayerModelOwner *node;

    first = static_cast<PlayerModelOwner *>(lbl_1_bss_AC.link0);
    middle = first;
    node = middle;
    sentinel = &lbl_1_bss_AC;

    for (; node != sentinel; node = node->next()) {
        ScoreDisplay::getChecked().fn_1_8268(node->playerIndex, node->characterId);
    }
}

/* Clear both player models and select their lineup callback. */
void fn_1_67A0(void)
{
    Node *sentinel;
    PlayerModelOwner *first;
    PlayerModelOwner *middle;
    PlayerModelOwner *node;

    first = static_cast<PlayerModelOwner *>(lbl_1_bss_AC.link0);
    middle = first;
    node = middle;
    sentinel = &lbl_1_bss_AC;
    for (; node != sentinel; node = node->next()) {
        s16 model;
        model = node->firstModel;
        fn_80026A44(model, 1);
        fn_80026E44(model);
        fn_80026F9C(model);
        model = node->secondModel;
        fn_80026A44(model, 1);
        fn_80026E44(model);
        fn_80026F9C(model);
        PlayerModelOwner::Callback callback = &PlayerModelOwner::fn_1_6F60;
        node->callback = callback;
    }
}

/* Select the winner lineup callback for every player. */
void fn_1_6884(void)
{
    PlayerModelOwner *node = lbl_1_bss_AC.front();
    Node *sentinel = lbl_1_bss_AC.end_node();
    for (; node != sentinel; node = node->next()) {
        PlayerModelOwner::Callback values = &PlayerModelOwner::fn_1_70D8;
        node->callback = values;
    }
}

/* Compute the selected player camera vectors from the player angle. */
void fn_1_6918(s32 playerIndex, Vec *position, Vec *direction)
{
    Node *sentinel;
    PlayerModelOwner *first = static_cast<PlayerModelOwner *>(lbl_1_bss_AC.link0);
    PlayerModelOwner *middle = first;
    PlayerModelOwner *node = middle;
    sentinel = &lbl_1_bss_AC;
    for (; node != sentinel; ) {
        if (node->playerIndex == playerIndex) {
            direction->x = 0.0f;
            direction->y = 120.0f;
            direction->z = 0.0f;
            Vec source = {0.0f, 120.0f, -100.0f};
            Mtx rotation;
            PSMTXRotRad(rotation, 0x79, MTXDegToRad(-node->angle));
            PSMTXMultVecSR(rotation, &source, position);
        }
        PlayerModelOwner *next = static_cast<PlayerModelOwner *>(node->link0);
        node = next;
    }
}

/* Write the fixed distant camera vectors for the selected player. */
void fn_1_6A14(s32 modelId, Vec *position, Vec *direction)
{
    Node *sentinel;
    PlayerModelOwner *first;
    PlayerModelOwner *current;
    PlayerModelOwner *node;
    first = static_cast<PlayerModelOwner *>(lbl_1_bss_AC.link0);
    current = first;
    node = current;
    sentinel = &lbl_1_bss_AC;

    for (; node != sentinel; ) {
        if (node->playerIndex == modelId) {
            direction->x = 0.0f;
            direction->y = 120.0f;
            direction->z = 0.0f;
            position->x = 0.0f;
            position->y = 120.0f;
            position->z = -1000.0f;
        }
        PlayerModelOwner *next = static_cast<PlayerModelOwner *>(node->link0);
        node = next;
    }
}

/* Leave this player unchanged during the list visit. */
void fn_1_6ADC(PlayerModelOwner *owner)
{
    (owner->*owner->callback)();
    owner->callbackCount++;
}

/* Change the character motion when the requested motion differs. */
void PlayerModelOwner::fn_1_6B20(s32 motionId, u8 queued)
{
    s16 modelId;
    s16 selectedMotion;

    modelId = this->firstModel;
    selectedMotion = this->m_mot_id.at(motionId);
    if (this->currentMotion != motionId) {
    fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
        fn_8005C2C8((s16)this->characterId, selectedMotion, 0.0f, 8.0f,
        queued == 0 ? ((1 << 30) | (1 << 0)) : 0);
        this->currentMotion = motionId;
    }
}

/* Choose the idle motion or enter the active player callback. */
void PlayerModelOwner::fn_1_6BE0()
{
    /* The first-model ID is evaluated and then discarded. */
    s16 modelId = firstModel;

    if (active == 0) {
        firstModel.setAttribute(1);
        secondModel.setAttribute(1);
        Callback transformState = &PlayerModelOwner::fn_1_72CC;
        this->callback = transformState;
        fn_1_6B20(0, 0);
        return;
    }
    fn_1_6B20(1, 0);
}

/* Enter the active player callback when ready and select motion zero. */
void PlayerModelOwner::fn_1_6D64()
{
    if (this->active == 0) {
        this->firstModel.setAttribute(1);
        this->secondModel.setAttribute(1);
        PlayerModelOwner::Callback transformState = &PlayerModelOwner::fn_1_72CC;
        this->callback = transformState;
    }
    fn_1_6B20(0, 0);
}

/* Position both models in the player lineup and clear their rotation. */
void PlayerModelOwner::fn_1_6E64()
{
    this->position.x = (f32)(this->playerIndex * 0xB4 - 0x10E);
    this->position.y = 0.0f;
    this->position.z = -400.0f;
    this->firstModel.setPosition(&this->position);
    Vec origin = { 0.0f, 0.0f, 0.0f };
    this->firstModel.setRotation(&origin);
    Vec secondPosition;
    secondPosition.x = this->position.x;
    secondPosition.y = this->position.y;
    secondPosition.z = this->position.z;
    secondPosition.z += 150.0f;
    this->secondModel.setPosition(&secondPosition);
    this->secondModel.setRotation(&origin);
}

/* Place the player in the lineup with motion zero. */
void PlayerModelOwner::fn_1_6F60()
{
    fn_1_6E64();
    fn_1_6B20(0, 0);
}

/* Place the player in the lineup with the winner or loser motion. */
void PlayerModelOwner::fn_1_70D8()
{
    fn_1_6E64();
    if (selected != 0) {
        fn_1_6B20(2, 1);
    } else {
        fn_1_6B20(3, 1);
    }
}

/* Process player input and clamp the camera angle to plus or minus 23 degrees. */
void PlayerModelOwner::fn_1_72CC()
{
    fn_1_75A8();
    if (angle > 23.0f) {
        angle = 23.0f;
    }
    if (angle < -23.0f) {
        angle = -23.0f;
    }
}

/* Generate CPU camera motion and shutter input from difficulty and visible targets. */
f32 PlayerModelOwner::fn_1_734C(bool *buttonPressed)
{
    s32 playerValue;
    f32 motionOffset = 0.0f;

    if (--this->motionChangeTimer < 0) {
        this->motionChangeTimer = (RandomUnsigned() & 0x3F) + 8;
        this->randomMotion = (f32)(2.0 * (RandomFloat() - 0.5));
    }
    if (std::fabs(this->randomMotion) < 0.3) {
        motionOffset = 0.0f;
    } else {
        motionOffset = this->randomMotion;
    }
    playerValue = fn_1_9D34(this->playerIndex, 0);
    if (GWMgNightFGet() != 0) {
        playerValue += fn_1_C4E8(this->playerIndex, 0, playerValue);
    }
    switch (this->difficulty) {
    case 0:
        *buttonPressed = (u8)RandomUnsigned() == 0;
        break;
    case 1:
    case 2:
    case 3:
        if (playerValue >= this->difficulty) {
            u32 inputMasks[4] = { 0, 63, 31, 7 };
            *buttonPressed = (inputMasks[this->difficulty] &
                              (s32)RandomUnsigned()) == 0;
        }
        if (((s32)RandomUnsigned() & 0x3FF) == 0) {
            *buttonPressed = 1;
        }
        break;
    default:
        break;
    }
    return motionOffset;
}

/* Read the player shutter button and horizontal stick input. */
float fn_1_7544(PlayerModelOwner *player, bool *buttonPressed)
{
    u32 buttons = lbl_802C0010[player->controllerId];

    if (buttons & PAD_BUTTON_A) {
        *buttonPressed = 1;
    }
    return lbl_8024C7D8[player->controllerId];
}

/* Set the player ready flag. */
void PlayerModelOwner::fn_1_759C()
{
    ready = 1;
}

/* Handle shutter input, play its sound and update the camera angle. */
void PlayerModelOwner::fn_1_75A8()
{
    static const s32 triggerSoundIds[4] = {
        MSM_SE_MG603_PLAYER_ONE_TRIGGER, MSM_SE_MG603_PLAYER_TWO_TRIGGER,
        MSM_SE_MG603_PLAYER_THREE_TRIGGER, MSM_SE_MG603_PLAYER_FOUR_TRIGGER
    };
    static const s32 nonAlternateSoundIds[4] = {
        MSM_SE_MG603_PLAYER_ONE_NONALTERNATE_TRIGGER,
        MSM_SE_MG603_PLAYER_TWO_NONALTERNATE_TRIGGER,
        MSM_SE_MG603_PLAYER_THREE_NONALTERNATE_TRIGGER,
        MSM_SE_MG603_PLAYER_FOUR_NONALTERNATE_TRIGGER
    };
    s32 inputMode;
    bool buttonPressed;
    f32 motion;
    s32 playerValue;

    inputMode = currentRequest();
    // The zero test is redundant with the following non-two test.
    if (inputMode == 0 || inputMode != 2 || this->ready == 1U) {
        return;
    }
    // Both input paths overwrite this initial motion value.
    /* Both input branches replace this initial motion value. */
    motion = 0.0f;
    buttonPressed = false;
    if (this->controllerId < 0) {
        motion = fn_1_734C(&buttonPressed);
    } else {
        motion = fn_1_7544(this, &buttonPressed);
        if (buttonPressed) {
            fn_8006A030((s16)this->playerIndex, 0x14, 7, 3);
        }
    }

    if (buttonPressed) {
        HuAudFXPlay(triggerSoundIds[this->playerIndex]);
        if (this->alternate == 0) {
            HuAudFXPlay(nonAlternateSoundIds[this->playerIndex]);
        }
        this->ready = 1;
        fn_1_9B88(this->playerIndex);
        playerValue = fn_1_9D34(this->playerIndex, 1);
        if (GWMgNightFGet() != 0) {
            fn_1_C280(this->playerIndex);
            playerValue += fn_1_C4E8(this->playerIndex, 1, playerValue);
        }
        this->recordedPlayerValue = playerValue;
        new FadeWork(this->playerIndex);
    }
    this->angle += 2.0f * motion;
}

/* Create the four-photo fade using the player character identifiers. */
void fn_1_794C()
{
    long characters[4];
    PlayerModelOwner *node = lbl_1_bss_AC.front();
    Node *end = lbl_1_bss_AC.end_node();
    int index = 0;
    for (; node != end; node = node->next()) {
        characters[index++] = node->characterId;
    }
    new SpriteFade(characters);
}
