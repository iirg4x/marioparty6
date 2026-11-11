/* The players: their models, motions, input and the per-frame update of the list at .bss 0x5FC. */
#include <vector>
#include <cmath>
extern "C" {
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "datadir_enum.h"
#include "game/hu3d.h"
#include "game/pad.h"
#include "game/charman.h"
#include "game/flag.h"
#include "game/gamework.h"
#include "REL/framework/assertion.h"
}

#include "REL/framework/model.h"
#include "REL/framework/linknode.h"
#include "REL/framework/singleton.h"
#include "REL/m648DLL/managers.h"
#include "REL/framework/sound.h"
#include "REL/m648DLL.h"
#include "REL/m648DLL/multi_model_owner.h"
#include "REL/framework/frame_time_inline.h"
#include "msm_se.h"

u32 RandomUnsigned();
float RandomFloat();

static const int NUMBEROF_MOTION = 6;

/* The framework version of this game converts seconds to frames in line
   (the player timers below). Its phase durations open this file's .bss (0x5E0),
   ahead of the selected entry and the player list head, whose destructor ends
   the file. */

extern "C" { Node *lbl_1_bss_5EC; }
class CPlayer;

#include "REL/m648DLL/flying_object.h"
#include "REL/m648DLL/player.h"

extern "C" { ListRoot<CPlayer> lbl_1_bss_5FC; }
extern "C" {
void fn_1_C148(CPlayer *object);
}

void operator delete(void *);

extern "C" {
extern const u8 lbl_1_data_5C[];
extern const u8 lbl_1_data_440[];

extern void *lbl_1_bss_A4;
extern const char lbl_1_data_428[];
extern const char lbl_1_data_432[];
void fn_1_1BCF4(Node *);
void fn_1_1BDB8(Node *, const void *);
void fn_1_1D6D8(void *, s32);
void fn_1_1DE18(s16 *, s32);
void fn_1_1DAB8(s16 *, u32);

extern void *lbl_1_bss_20;
void fn_1_2070C(s32, const Vec *, Vec *, f32, f32, f32);
}

void fn_1_AE34(Vec *position, s32 index)
{
    position->x = (f32)(index * 0x104 - 0x186);
    position->y = 0.0f;
    position->z = 0.0f;
}

void fn_1_AE90()
{
    while (!lbl_1_bss_5FC.empty()) {
        delete lbl_1_bss_5FC.back();
    }
}

/* Configures a model's layer, camera, visibility, culling, and shadows. */
void fn_1_AF34(s16 modelId, u8 showModel)
{
    fn_80026B8C(modelId, 3);
    fn_80026B64(modelId, 1);
    if (showModel) {
        fn_80026A44(modelId, (1 << 0));
    } else {
        fn_800269E8(modelId, (1 << 0));
    }
    fn_800269E8(modelId, ((1 << 30) | (1 << 1)));
    fn_800269E8(modelId, (1 << 14));
    fn_80026E44(modelId);
    fn_80026F9C(modelId);
}

static const s32 sPlayerShapeModels[15] = {
    DATANUM(0x6b0000, 0x12), DATANUM(0x6b0000, 0x12), DATANUM(0x6b0000, 0x13), DATANUM(0x6b0000, 0x12), DATANUM(0x6b0000, 0x12),
    DATANUM(0x6b0000, 0x13), DATANUM(0x6b0000, 0x12), DATANUM(0x6b0000, 0x14), DATANUM(0x6b0000, 0x12), DATANUM(0x6b0000, 0x15),
    DATANUM(0x6b0000, 0x14), DATANUM(0x6b0000, 0x15), DATANUM(0x6b0000, 0x15), DATANUM(0x6b0000, 0x15), DATANUM(0x6b0000, 0x15),
};

/* Creates player playerNo with its six motions, its indicators and its idle motion. */
CPlayer::CPlayer(s32 playerNo, s32 effect, bool nightMode)
    : gameOver(0),
      characterModel(lbl_80265728[playerNo].charNo, 2),
      shapeModel(sPlayerShapeModels[playerNo]),
      scale(1.0f),
      finished(0),
      selected(0),
      measure(0x4650),
      strongRumble(0),
      weakRumble(0),
      finalMeasure(0),
      rank(0),
      height(-90.0f),
      active(0)
{
    char *hook;
    insert4(&lbl_1_bss_5FC);
    this->playerNo = playerNo;
    this->characterNo = lbl_80265728[playerNo].charNo;
    this->difficulty = lbl_80265728[playerNo].comDif;
    this->padNo = lbl_80265728[playerNo].type != 0 ? -1 : lbl_80265728[playerNo].padNo;
    this->currentMotion = -1;
    m_mot_id.clear();
    m_mot_id.reserve(NUMBEROF_MOTION);
    addMotion(DATANUM(0x930000, 0));
    addMotion(DATANUM(0x8e0000, 0x99));
    addMotion(DATANUM(0x8e0000, 0x98));
    addMotion(DATANUM(0x930000, 6));
    addMotion(DATANUM(0x930000, 0x28));
    addMotion(DATANUM(0x930000, 1));
    assert(m_mot_id.size() == NUMBEROF_MOTION);
    fn_8005B4BC((s16)this->characterNo);
    fn_1_AF34((s16)characterModel, 1);
    fn_1_AF34((s16)shapeModel, 1);
    hook = fn_8005C0F0((s16)this->characterNo, 4, 0);
    fn_8002741C(characterModel, hook, shapeModel);
    position.x = (f32)((playerNo * 0x104) - 0x186);
    position.y = 0.0f;
    position.z = 0.0f;
    ScoreManager::getChecked().fn_1_D77C(this->playerNo, this->characterNo);
    ScoreManager::getChecked().fn_1_D8D8(this->playerNo, 0);
    spriteA = new MotionHookModels(playerNo, this->difficulty);
    Vec spritePosition;
    spritePosition.x = position.x - 84.0f;
    spritePosition.y = 0.0f;
    spritePosition.z = 10.0f + position.z;
    spriteB = new FlyingObject(&spritePosition);
    fn_1_C2E8(1, 0);
    Callback next = &CPlayer::fn_1_BD30;
    callback = next;
}

CPlayer::~CPlayer() {}

/* Returns the smallest measure among active entries with a valid owner index. */
s32 fn_1_B62C()
{
    s32 minimum = 0x4650;
    CPlayer *node = lbl_1_bss_5FC.front();
    Node *end = lbl_1_bss_5FC.end_node();
    for (; node != end; node = node->next()) {
        if (node->selected && node->padNo >= 0 && node->measure < minimum) {
            minimum = node->measure;
        }
    }
    return minimum;
}

void fn_1_B6B8(Vec *positions)
{
    Node *sentinel;
    CPlayer *head;
    CPlayer *entry;
    CPlayer *start;
    CPlayer *next;
    s32 index;

    head = static_cast<CPlayer *>(lbl_1_bss_5FC.link0);
    start = head;
    entry = start;
    sentinel = &lbl_1_bss_5FC;
    index = 0;
    while (entry != sentinel) {
        positions[index].x = entry->position.x;
        positions[index].y = entry->position.y;
        positions[index].z = entry->position.z;
        next = static_cast<CPlayer *>(entry->link0);
        entry = next;
        index++;
    }
}

s16 CPlayer::addMotion(u32 dataNum)
{
    s16 motionId = fn_8005B0B8((s16)characterNo, dataNum);
    m_mot_id.push_back(motionId);
    return motionId;
}

void fn_1_B7C4()
{
    lbl_1_bss_5FC.visit(fn_1_C148);
    CPlayer *node = lbl_1_bss_5FC.front();
    if (node != lbl_1_bss_5FC.end_node()) {
        node->unlink();
        node->insert4(&lbl_1_bss_5FC);
    }
}

u8 fn_1_B848(void)
{
    Node *root;
    CPlayer *firstNode;
    CPlayer *entry;
    CPlayer *node;
    CPlayer *nextNode;

    firstNode = static_cast<CPlayer *>(lbl_1_bss_5FC.link0);
    entry = firstNode;
    node = entry;
    root = &lbl_1_bss_5FC;
    for (; node != root; node = nextNode) {
        if (node->finished) {
            return 1;
        }
        nextNode = static_cast<CPlayer *>(node->link0);
    }
    return 0;
}

void fn_1_B8BC(void)
{
    CPlayer *node = lbl_1_bss_5FC.front();
    Node *end = lbl_1_bss_5FC.end_node();
    for (; node != end; node = node->next()) {
        s32 playerNo;
        s32 score;
        score = node->rank;
        playerNo = node->playerNo;
        lbl_80265750[playerNo].mgScore = score;
    }
}

extern "C" {
extern void *lbl_1_bss_2C;

/* Selects an eligible queued entry among the four available records. */
void fn_1_B934(void);
}

void fn_1_B934(void)
{
    CPlayer *node = lbl_1_bss_5FC.front();
    Node *end = lbl_1_bss_5FC.end_node();
    CPlayer *entries[4];
    s32 scores[4];
    s32 order[4] = {0, 1, 2, 3};
    s32 unusedValue = 0; /* Never read; its initial 0 is still stored. */
    s32 entryCount = 0;
    s32 inactiveCount = 0;

    for (; node != end; node = node->next()) {
        entries[entryCount] = node;
        s32 value = node->finished ? node->measure : 0x8CA0;
        scores[entryCount] = value + ((0xA - node->rank) * 0x8CA0);
        if (node->finished == 0) {
            ++inactiveCount;
        }
        ++entryCount;
    }

    if (inactiveCount == 4) {
        return;
    }

        for (s32 pass = 0; pass < 3; ++pass) {
            for (s32 index = 0; index < 3; ++index) {
                if (scores[index] > scores[index + 1]) {
                    s32 score = scores[index];
                    s32 nextScore = scores[index + 1];
                    scores[index] = nextScore;
                    scores[index + 1] = score;
                    score = order[index];
                    nextScore = order[index + 1];
                    order[index] = nextScore;
                    order[index + 1] = score;
                }
            }
        }

        s32 foundDifferentScore = 0;
        s32 firstGroupCount = 0;
        for (entryCount = 0; entryCount < 4; ++entryCount) {
            if (foundDifferentScore == 0) {
                ++firstGroupCount;
            }
            if (entryCount < 3 && scores[entryCount] != scores[entryCount + 1]) {
                ++foundDifferentScore;
            }
        }

        s32 randomIndex = (firstGroupCount * (u16)RandomUnsigned()) >> 16;
        entries[order[randomIndex]]->selected = 1;
        MinigameCore::addWinner(entries[order[randomIndex]]->playerNo);
        lbl_1_bss_5EC = entries[order[randomIndex]];
        RequestOwner8C40::getChecked().fn_1_8C40(lbl_1_bss_5EC);
}

void fn_1_BC1C()
{
    CPlayer *node = lbl_1_bss_5FC.front();
    Node *end = lbl_1_bss_5FC.end_node();
    for (; node != end; node = node->next()) {
        node->gameOver = 1;
    }
}

void fn_1_BC80(void)
{
}

void fn_1_BC84()
{
    ListRoot<CPlayer> *root;
    CPlayer *node;

    node = lbl_1_bss_5FC.front();
    root = &lbl_1_bss_5FC;
    for (; static_cast<Node *>(node) != static_cast<Node *>(root); node = node->next()) {
        node->active = 1;
        node->timer = static_cast<s32>(static_cast<f32>(node->playerNo) * secondsToFrames(0.2f));
    }
}

extern "C" {
s32 fn_1_CCE4(CPlayer *owner);
}

void CPlayer::fn_1_BD30()
{
    if (active != 0) {
        if (--timer < 0) {
            fn_1_C2E8(2, 1);
            Callback next = &CPlayer::fn_1_BDDC;
            this->callback = next;
            callbackState = 0;
            return;
        }
    }
    fn_1_C2E8(1, 0);
}

void CPlayer::fn_1_BDDC()
{
    if (fn_8005C284((s16)characterNo) < 0 &&
        fn_8005C240((s16)characterNo) != 0) {
        Callback next = &CPlayer::fn_1_C3A8;
        callback = next;
    }
    if (callbackState == 0x26) {
        soundPlayAt(1, MSM_SE_MG648_PLAYER_MOTION, &position);
    }
    if (callbackState == 0x2C) {
        fn_1_984(spriteB, 0);
    }
    ++callbackState;
}

extern "C" {
void fn_1_BF50(CPlayer *entry);
void fn_1_BFC4(CPlayer *entry);
}

/* Records each entry's character and dispatches its active or waiting work. */
s32 fn_1_BEA8()
{
    s16 characters[4];
    CPlayer *node = lbl_1_bss_5FC.front();
    Node *end = lbl_1_bss_5FC.end_node();
    int allWaiting = 1;
    int index = 0;

    for (; node != end; node = node->next()) {
        characters[index++] = node->characterNo;
        if (node->selected) {
            fn_1_BF50(node);
            allWaiting = 0;
        } else {
            fn_1_BFC4(node);
        }
    }
    return allWaiting;
}

void fn_1_BF50(CPlayer *entry)
{
    {
        CPlayer::Callback value = &CPlayer::fn_1_BF94;
        entry->callback = value;
    }
}

void CPlayer::fn_1_BF94()
{
    fn_1_C2E8(3, 1);
}

void fn_1_BFC4(CPlayer *entry)
{
    {
        CPlayer::Callback value = &CPlayer::fn_1_C008;
        entry->callback = value;
    }
}

void CPlayer::fn_1_C008()
{
    fn_1_C2E8(4, 1);
}

void CPlayer::fn_1_C038()
{
    scale -= 0.1;
    if (scale > 0.0f) {
        s16 modelId = (s16)shapeModel.modelId;
        fn_800268B0(modelId, scale, scale, scale);
    } else {
        scale = 0.0f;
        s16 resetModelId = (s16)shapeModel.modelId;
        fn_800269E8(resetModelId, 1);
    }
    if ((height += 3.0f) > 0.0f) {
        height = 0.0f;
        fn_1_C2E8(0, 0);
    } else {
        fn_1_C2E8(5, 0);
    }
}

extern "C" {
void fn_1_C22C(CPlayer *object);
void fn_1_C1C4(CPlayer *object);
}

void fn_1_C148(CPlayer *object)
{
    (object->*object->callback)();
    if (currentRequest() == 2) fn_1_C22C(object);
    fn_1_C1C4(object);
    if (currentRequest() != 2 || object->finished == 1) return;
    object->finalMeasure++;
}

void fn_1_C1C4(CPlayer *object)
{
    Vec position = { 0.0f, 0.0f, 0.0f };
    position.y = object->height;
    object->characterModel.setRotation(&position);
    object->characterModel.setPosition(&object->position);
}

void fn_1_C22C(CPlayer *player)
{
    s32 vibrationType;
    vibrationType = 0;
    if (player->weakRumble > 0) {
        vibrationType = 2;
        --player->weakRumble;
    }
    if (player->strongRumble > 0) {
        vibrationType = 1;
        --player->strongRumble;
    }
    if (player->padNo >= 0 && vibrationType > 0) {
        if (vibrationType == 1) {
            fn_8006A030((s16)player->playerNo, 0x14, 7, 3);
            return;
        }
        fn_8006A030((s16)player->playerNo, 0x14, 0x14, 0);
    }
}

void CPlayer::fn_1_C2E8(s32 motionId, u8 queued)
{
    s16 modelId = (s16)characterModel;
    s16 selectedMotion = m_mot_id.at(motionId);
    if (currentMotion != motionId) {
        fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
        fn_8005C2C8((s16)characterNo, selectedMotion, 0.0f, 8.0f,
                    queued == 0 ? ((1 << 30) | (1 << 0)) : 0);
        currentMotion = motionId;
    }
}

extern "C" {
void fn_1_CAF4(CPlayer *owner);

extern void *lbl_1_bss_24;
}

/* Selects an idle motion and enters the completion callback at state two. */
void CPlayer::fn_1_C3A8()
{
    Callback next;
    fn_1_C2E8(1, 0);
    if (currentRequest() == 2) {
        fn_1_CAF4(this);
        next = &CPlayer::fn_1_C7BC;
        this->callback = next;
    }
}

/* Advances the motion callback state after the selected character animation completes. */
void CPlayer::fn_1_C498()
{
    if (fn_8005C284((s16)characterNo) < 0 &&
        fn_8005C240((s16)characterNo) != 0) {
        if (finished != 0 || gameOver != 0) {
            Callback next = &CPlayer::fn_1_C038;
            this->callback = next;
        } else {
            fn_1_CAF4(this);
            MotionHookModels *modelObject = spriteA;
            modelObject->shouldRestart = 1;
            Callback next = &CPlayer::fn_1_C7BC;
            this->callback = next;
        }
    }
    if (callbackState == 0x26) {
        soundPlayAt(1, MSM_SE_MG648_PLAYER_MOTION, &position);
    }
    if (callbackState == 0x2C) {
        if (spriteA->fn_1_4B20(command) != 0 &&
            gameOver == 0) {
            rank++;
            ScoreManager::getChecked().fn_1_D8D8(playerNo, rank);
            if (rank >= 10 && fn_8006A124(0x30002U) == 0) {
                TimerManager::getChecked().fn_1_D438();
                measure = finalMeasure;
                finished = 1;
                spriteA->fn_1_5800();
                fn_1_984(spriteB, 1);
            } else {
                spriteA->fn_1_548C();
                fn_1_984(spriteB, 0);
            }
        } else {
            weakRumble = 0xC;
            fn_1_AB8(spriteB);
        }
    }
    callbackState++;
}

s32 fn_1_C700(CPlayer *owner)
{
    s32 motionIndex = -1;
    u32 buttons = lbl_802C0008[owner->padNo];

    if ((buttons & PAD_BUTTON_A) != 0) motionIndex = 0;
    if ((buttons & PAD_BUTTON_B) != 0) motionIndex = 1;
    if ((buttons & PAD_BUTTON_X) != 0) motionIndex = 2;
    if ((buttons & PAD_BUTTON_Y) != 0) motionIndex = 3;
    if ((buttons & PAD_TRIGGER_L) != 0) motionIndex = 4;
    if ((buttons & PAD_TRIGGER_R) != 0) motionIndex = 5;
    if ((buttons & 0x4000) != 0) motionIndex = 4;
    if ((buttons & 0x2000) != 0) motionIndex = 5;
    return motionIndex;
}

/* Selects the requested motion and installs its completion callback. */
void CPlayer::fn_1_C7BC()
{
    s32 requestedMotion;

    if (gameOver != 0) {
        Callback next = &CPlayer::fn_1_C038;
        callback = next;
        return;
    }

    MotionHookModels *work = spriteA;
    if (work->complete == 0) {
        return;
    }
    FlyingObject *object = spriteB;
    if (object->complete == 0) return;

    requestedMotion = padNo < 0
        ? fn_1_CCE4(this)
        : fn_1_C700(this);
    command = requestedMotion;

    if (requestedMotion >= 0) {
        fn_1_C2E8(2, 1);
        Callback next = &CPlayer::fn_1_C498;
        callback = next;
        callbackState = 0;
        return;
    }

    fn_1_C2E8(1, 0);
}

/* Checks the callback list's invalid-owner limit and pending-state count. */
bool CPlayer::allPlayersFinished()
{
    Node *end;
    CPlayer *node = lbl_1_bss_5FC.front();
    int inactiveCount;
    int invalidCount;

    end = lbl_1_bss_5FC.end_node();
    inactiveCount = 0;
    invalidCount = 0;

    for (; node != end; node = node->next()) {
        if (padNo >= 0) {
            if (finished == 0) {
                ++inactiveCount;
            }
        } else {
            ++invalidCount;
        }
    }
    return invalidCount < 4 ? !inactiveCount : false;
}

/* Updates the callback timing value after checking the active work list. */
void fn_1_CAF4(CPlayer *owner)
{
    s32 timingIndex = owner->difficulty;
    f32 randomValue;

    if (owner->allPlayersFinished()) {
        timingIndex = 4;
    }

    randomValue = RandomFloat();
    {
        f32 firstWeights[5] = { 2.5f, 1.5f, 1.0f, 0.5f, 0.1f };
        f32 secondWeights[5] = { 1.0f, 0.5f, 0.25f, 0.1f, 0.0f };
        owner->decisionTimer = (s32)(secondsToFrames(randomValue * firstWeights[timingIndex])
                                   + secondsToFrames(secondWeights[timingIndex]));
    }
}

static inline s32 fn_1_CCE4ReadValue(MotionHookModels *data)
{
    return data->attributeState;
}

/* Chooses a callback result after the tracked entries finish their work. */
s32 fn_1_CCE4(CPlayer *owner)
{
    s32 selection = owner->difficulty;
    if (owner->allPlayersFinished())
    {
        selection = 3;
        if ((f32)owner->decisionTimer > 6.0f)
        {
            owner->decisionTimer = 6;
        }
    }
    if (--owner->decisionTimer > 0)
    {
        return -1;
    }
    owner->decisionTimer = 0;
    s32 selected = ((u16)RandomUnsigned() * 6) >> 16;
    s32 masks[4] = { 1, 3, 7, 15 };
    return ((masks[selection] & (s32)RandomUnsigned()) == 0) ? selected : fn_1_CCE4ReadValue(owner->spriteA);
}
