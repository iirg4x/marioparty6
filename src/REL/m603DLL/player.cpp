#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/m603DLL/frame_time.h"
#include "REL/m603DLL/night.h"
#include "REL/m603DLL/callback_owner.h"
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
#include "REL/m603DLL/random.h"
#include "REL/m603DLL/sprite_fade.h"
#include "REL/framework/minigame_core.h"
#define M603_OWNED_MODEL_DECLARATION
#include "REL/m603DLL/model.h"
extern "C" {
#include "game/data.h"
extern "C" {
#include "game/pad.h"
}
#include "game/audio.h"
}
#include "REL/m603DLL/player.h"
#include "REL/m603DLL/score_owner.h"
// These existing intrusive-list accessor bodies operate on the proven zero-offset Node base.
struct CharacterList : Node {
    ~CharacterList() {}
    bool empty() const { return link0 == this && link0 == link4; }
    Node *first_node() { return link0; }
    PlayerModelOwner *begin() { return static_cast<PlayerModelOwner *>(first_node()); }
    Node *last_node() { return link4; }
    Node *end_node() { return this; }
    PlayerModelOwner *front() { return static_cast<PlayerModelOwner *>(first_node()); }
    PlayerModelOwner *back() { return static_cast<PlayerModelOwner *>(last_node()); }
};
extern "C" CharacterList lbl_1_bss_AC;
typedef PlayerModelOwner::Callback MotionCallback5A6C;
extern "C" {
extern s32 lbl_1_data_F18;
extern float lbl_1_bss_1BC;
extern s8 lbl_1_bss_218;
extern s8 lbl_1_bss_219;
extern s8 lbl_1_data_2758;
s32 fn_1_367CC(s32 *player);
}
void fn_1_5994(Vec *vector) {
    vector->x = 0.0f;
    vector->y = 0.0f;
    vector->z = -250.0f;
}
void fn_1_59C8() {
    while (!lbl_1_bss_AC.empty()) {
        delete lbl_1_bss_AC.back();
    }
}
PlayerModelOwner::PlayerModelOwner(s32 playerIndex, bool alternate)
    : active(1), playerIndex(playerIndex), characterId(lbl_80265728[playerIndex].charNo),
      controllerId(lbl_80265728[playerIndex].type != 0 ? -1 : lbl_80265728[playerIndex].padNo),
      difficulty(lbl_80265728[playerIndex].comDif), callbackCount(1),
      firstModel(lbl_80265728[playerIndex].charNo, 0),
      secondModel(alternate ? DATANUM(0x3e0000, 8) : DATANUM(0x3e0000, 9)),
      flag4C(0), selected(0), ready(0), recordedPlayerValue(0), timer54(0x3C),
      motionChangeTimer(0), randomMotion(0.0f), alternate(alternate)
{
    s16 modelId;
    Vec rotation, modelPosition;
    MotionCallback5A6C callback;
    s32 i;
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
    this->position.x = 0.0f;
    this->position.y = 0.0f;
    this->position.z = -250.0f;
    this->angle = 0.0f;
    this->position.x = 0.0f;
    this->position.y = 0.0f;
    this->position.z = -300.0f;
    rotation.x = 0.0f;
    rotation.y = 180.0f;
    rotation.z = 0.0f;
    this->firstModel.fn_1_33A3C(&this->position);
    this->firstModel.fn_1_33ACC(& rotation);
    modelPosition.x = this->position.x;
    modelPosition.y = this->position.y;
    modelPosition.z = this->position.z;
    modelPosition.x -= 50.0f;
    modelPosition.z -= 100.0f;
    this->secondModel.fn_1_33A3C(& modelPosition);
    this->secondModel.fn_1_33ACC(& rotation);
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

PlayerModelOwner::~PlayerModelOwner()
{
}

extern "C" {

/* Appends the created motion and returns its handle to the caller. */
HU3D_MOTIONID PlayerModelOwner::fn_1_61DC(unsigned int dataNum)
{
    HU3D_MOTIONID motionId = fn_8005B0B8(this->characterId, dataNum);
    this->m_mot_id.push_back(motionId);
    return motionId;
}

}

void fn_1_6ADC(Node *);

template<class T> void fn_1_7AC0(T *, T *, void (*)(T *));
template<class List> void fn_1_7A60(List *, void (*)(Node *));

extern "C" { extern PlayerModelOwner *lbl_1_bss_9C; }

/* Updates all callbacks, then moves the first character to the list's opposite end. */
void fn_1_6258()
{
    fn_1_7A60(&lbl_1_bss_AC, fn_1_6ADC);
    PlayerModelOwner *node = lbl_1_bss_AC.front();
    if (node != lbl_1_bss_AC.end_node()) {
        node->unlink();
        node->insert4(&lbl_1_bss_AC);
    }
}

struct CharacterIndexOrder {
    int values[4];
};

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
    CharacterIndexOrder order = { { 0, 1, 2, 3 } };
    int result = 0;
    index = 0;
    for (; node != end; node = node->next_node()) {
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
                firstValue = order.values[sortIndex];
                secondValue = order.values[sortIndex + 1];
                order.values[sortIndex] = secondValue;
                order.values[sortIndex + 1] = firstValue;
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
        lbl_1_bss_9C = entries[order.values[0]];
    }
    for (markIndex = 0; markIndex < tiedCount; markIndex++) {
        entries[order.values[markIndex]]->selected = 1;
        MinigameCore::addWinner(entries[order.values[markIndex]]->playerIndex);
    }
    return false;
}

struct CallbackSingleton {
    CallbackSingleton();
    ~CallbackSingleton();
};

struct CallbackBase : CallbackSingleton {
    CallbackBase();
    virtual ~CallbackBase();
    virtual void begin();
    virtual int step() = 0;
    virtual void fn_1_17104();
    virtual int fn_1_17188();
};

int CallbackBase::fn_1_17188()
{
    return 1;
}

/* Reports whether exactly four character entries have their ready byte set. */
bool fn_1_6594()
{
    PlayerModelOwner *node = lbl_1_bss_AC.front();
    Node *end = lbl_1_bss_AC.end_node();
    int count = 0;
    for (; node != end; node = node->next_node()) {
        if (node->ready) ++count;
    }
    return count == 4;
}

/* Collects the character identifiers in a local table during the list walk. */
void fn_1_6614()
{
    short characters[4];
    PlayerModelOwner *node = lbl_1_bss_AC.front();
    Node *end = lbl_1_bss_AC.end_node();
    int index = 0;
    for (; node != end; node = node->next_node()) {
        characters[index++] = node->characterId;
    }
}

extern "C" { extern s16 lbl_802C0278; }

extern s32 fn_1_C4E8(s32 playerIndex, u8 markEntries, s32 currentValue);

static inline BOOL nightBonusActive75A8(void)
{
    return NightActive();
}

/* fn_1_6690: mechanical copy of fn_1_BC1C in m648DLL (100% alike), names and constants adapted by copier.py; to be placed in its file. */

extern "C" { extern CharacterList lbl_1_bss_5FC; }

void fn_1_6690()
{
    PlayerModelOwner *node = lbl_1_bss_AC.front();
    Node *end = lbl_1_bss_AC.end_node();
    for (; node != end; node = node->next_node()) {
        node->active = 0;
    }
}

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

    for (; node != sentinel; node = node->next_node()) {
        ScoreDisplay::getChecked().fn_1_8268(node->playerIndex, node->characterId);
    }
}

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
    for (; node != sentinel; node = node->next_node()) {
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

void fn_1_6884(void)
{
    PlayerModelOwner *node = lbl_1_bss_AC.front();
    Node *sentinel = lbl_1_bss_AC.end_node();
    for (; node != sentinel; node = node->next_node()) {
        PlayerModelOwner::Callback values = &PlayerModelOwner::fn_1_70D8;
        node->callback = values;
    }
}

void fn_1_6918(s32 modelId, Vec *position, Vec *direction)
{
    Node *sentinel;
    PlayerModelOwner *first = static_cast<PlayerModelOwner *>(lbl_1_bss_AC.link0);
    PlayerModelOwner *middle = first;
    PlayerModelOwner *node = middle;
    sentinel = &lbl_1_bss_AC;
    for (; node != sentinel; ) {
        if (node->playerIndex == modelId) {
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

void fn_1_6ADC(Node *node)
{
    (static_cast<PlayerModelOwner *>(node)->*
        static_cast<PlayerModelOwner *>(node)->callback)();
    static_cast<PlayerModelOwner *>(node)->callbackCount++;
}

extern "C" {

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

}

void PlayerModelOwner::fn_1_6BE0()
{
    s16 modelId = firstModel;

    if (active == 0) {
        firstModel.fn_1_33950(1);
        secondModel.fn_1_33950(1);
        {
            Callback transformState = &PlayerModelOwner::fn_1_72CC;
            this->callback = transformState;
        }
        fn_1_6B20(0, 0);
        return;
    }
    fn_1_6B20(1, 0);
}

void PlayerModelOwner::fn_1_6D64()
{
    if (this->active == 0) {
        this->firstModel.fn_1_33950(1);
        this->secondModel.fn_1_33950(1);
        {
            PlayerModelOwner::Callback transformState = &PlayerModelOwner::fn_1_72CC;
            this->callback = transformState;
        }
    }
    fn_1_6B20(0, 0);
}

extern "C" {

void PlayerModelOwner::fn_1_6E64()
{
    this->position.x = (f32)(this->playerIndex * 0xB4 - 0x10E);
    this->position.y = 0.0f;
    this->position.z = -400.0f;
    this->firstModel.fn_1_33A3C(&this->position);
    {
        Vec origin = { 0.0f, 0.0f, 0.0f };
        this->firstModel.fn_1_33ACC(&origin);
        {
            Vec secondPosition;
            secondPosition.x = this->position.x;
            secondPosition.y = this->position.y;
            secondPosition.z = this->position.z;
            secondPosition.z += 150.0f;
            this->secondModel.fn_1_33A3C(&secondPosition);
            this->secondModel.fn_1_33ACC(&origin);
        }
    }
}

void PlayerModelOwner::fn_1_6F60()
{
    fn_1_6E64();
    fn_1_6B20(0, 0);
}

void PlayerModelOwner::fn_1_70D8()
{
    fn_1_6E64();
    if (selected != 0) {
        fn_1_6B20(2, 1);
    } else {
        fn_1_6B20(3, 1);
    }
}

}

extern "C" {

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

}

extern "C" {

extern s16 lbl_802C0278;

f32 PlayerModelOwner::fn_1_734C(bool *buttonPressed)
{
    s32 playerValue;
    f32 motionOffset = 0.0f;

    if (--this->motionChangeTimer < 0) {
        this->motionChangeTimer = (RandomUnsigned() & 0x3F) + 8;
        this->randomMotion = (f32)(2.0 * (RandomFloat() - 0.5));
    }
    {
        if (std::fabs(this->randomMotion) < 0.3) {
            motionOffset = 0.0f;
        } else {
            motionOffset = this->randomMotion;
        }
    }

    playerValue = fn_1_9D34(this->playerIndex, 0);
    if (NightActive()) {
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

}

float fn_1_7544(PlayerModelOwner *player, bool *buttonPressed)
{
    u32 buttons = lbl_802C0010[player->controllerId];

    if (buttons & PAD_BUTTON_A) {
        *buttonPressed = 1;
    }
    return lbl_8024C7D8[player->controllerId];
}

extern "C" {

void PlayerModelOwner::fn_1_759C()
{
    ready = 1;
}

}

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

    inputMode = fn_1_1AB30();
    if (inputMode == 0 || inputMode != 2 || this->ready == 1U) {
        return;
    }
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
        if (nightBonusActive75A8()) {
            fn_1_C280(this->playerIndex);
            playerValue += fn_1_C4E8(this->playerIndex, 1, playerValue);
        }
        this->recordedPlayerValue = playerValue;
        new FadeWork(this->playerIndex);
    }
    this->angle += 2.0f * motion;
}

void fn_1_794C()
{
    long characters[4];
    PlayerModelOwner *node = lbl_1_bss_AC.front();
    Node *end = lbl_1_bss_AC.end_node();
    int index = 0;
    for (; node != end; node = node->next_node()) {
        characters[index++] = node->characterId;
    }
    new SpriteFade(characters);
}

typedef void (*ListVisitor)(Node *);

/* Dispatches the visitor from the first entry up to the list sentinel. */
template<class List>
void fn_1_7A60(List *list, ListVisitor visitor)
{
    fn_1_7AC0<Node>(list->begin(), list->end_node(), visitor);
}

/* Follows each entry's next link after invoking the visitor. */
template<class T>
void fn_1_7AC0(T *current, T *sentinel, void (*callback)(T *))
{
    while (current != sentinel) {
        callback(current);
        T *next = current->link0;
        current = next;
    }
}

extern "C" {

    s32 lbl_1_bss_90 = secondsToFrames(2L);
    s32 lbl_1_bss_94 = (s32)secondsToFrames(3.5);
    s32 lbl_1_bss_98 = (s32)secondsToFrames(3.5);
    PlayerModelOwner *lbl_1_bss_9C;
    CharacterList lbl_1_bss_AC;
}
