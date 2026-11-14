// The contestants: each builds its character, keeps its motions and runs its state callback every frame.
extern "C" {
#include "game/charman.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/pad.h"
}
#include "datadir_enum.h"
#include "REL/framework/sound.h"
#include "REL/framework/list.h"
#include "REL/m622DLL/callback_object.h"
#include "REL/framework/phase_durations.h"
#include "REL/framework/random.h"
#include "REL/framework/list_object.h"
#include "REL/m622DLL/scene_models.h"
#include "REL/m622DLL/score_display_owner.h"
// The contestants.
extern "C" { extern ListRoot<CallbackObject> lbl_1_bss_E8; }
extern "C" {
extern u8 lbl_1_bss_D8;
extern u8 lbl_1_bss_D9;
}
void fn_1_72E8(CallbackObject *);
void fn_1_8238(CallbackObject *owner);
static const Vec lbl_1_rodata_260[4] = {
    {210.0f, 0.0f, -655.0f},
    {210.0f, 0.0f, -375.0f},
    {210.0f, 0.0f, -85.0f},
    {210.0f, 0.0f, 205.0f},
};

/* Offsets the selected lane position to the player entry position. */
void fn_1_6748(Vec *result, s32 index)
{
    f64 x;
    f64 y;
    f64 z;

    z = lbl_1_rodata_260[index].z - 50.0f;
    y = lbl_1_rodata_260[index].y;
    x = lbl_1_rodata_260[index].x - 2000.0f;

    result->x = x;
    result->y = y;
    result->z = z;
}

/* Offsets the selected lane position to the player entry position. */
static inline void setPlayerEntryPosition(Vec *result, s32 index)
{
    f64 z;
    f64 y;
    f64 x;

    z = lbl_1_rodata_260[index].z - 50.0f;
    y = lbl_1_rodata_260[index].y;
    x = lbl_1_rodata_260[index].x - 2000.0f;

    result->x = x;
    result->y = y;
    result->z = z;
}

/* Deletes every player in the player list. */
void fn_1_67F8()
{
    while (!lbl_1_bss_E8.empty()) {
        delete lbl_1_bss_E8.back();
    }
}

/* Initializes a player's character overlay, model list, and world position. */
CallbackObject::CallbackObject(s32 playerNo)
    : rotationY(90.0f), model(lbl_80265728[playerNo].charNo, 2), pulsedVibrationFrames(0),
      continuousVibrationFrames(0), mgCoin(0), requestList(0)
{
    s16 currentModel;
    s32 groupNo;

    insert4(&lbl_1_bss_E8);
    controller = playerNo;
    character = lbl_80265728[playerNo].charNo;
    mode = lbl_80265728[playerNo].comDif;
    if (lbl_80265728[playerNo].type != 0) {
        groupNo = -1;
    } else {
        groupNo = lbl_80265728[playerNo].padNo;
    }
    padIndex = groupNo;
    state = -1;
    m_mot_id.clear();
    m_mot_id.reserve(NUMBEROF_MOTION);
    fn_1_6F34(DATANUM(0x930000, 0));
    fn_1_6F34(DATANUM(0x930000, 1));
    fn_1_6F34(DATANUM(0x930000, 2));
    fn_1_6F34(DATANUM(0x8e0000, 167));
    fn_1_6F34(DATANUM(0x930000, 15));
    fn_1_6F34(DATANUM(0x930000, 21));
    fn_1_6F34(DATANUM(0x930000, 38));
    fn_1_6F34(DATANUM(0x930000, 40));
    assert(m_mot_id.size() == NUMBEROF_MOTION);
    fn_8005B4BC((s16)character);
    currentModel = model;
    fn_80026B8C(currentModel, 3);
    fn_80026B64(currentModel, 1);
    fn_1_8D74(&ScoreDisplayOwner::getChecked(), controller, character);
    fn_1_8FC8(&ScoreDisplayOwner::getChecked(), controller, 0);
    setPlayerEntryPosition(&position, controller);
    fn_1_8238(this);
    fn_1_7460(this, 0, 0);
    Callback next = &CallbackObject::fn_1_7088;
    callback = next;
}

/* Releases the object through its member and base destructors. */
CallbackObject::~CallbackObject()
{
}

/* Loads a character motion, appends its handle and returns it. */
s16 CallbackObject::fn_1_6F34(u32 dataNum)
{
    s16 motionId = fn_8005B0B8(character, dataNum);
    m_mot_id.push_back(motionId);
    return motionId;
}

/* Resets coin sound flags, updates every player and rotates the first player to the back. */
void fn_1_6FB0()
{
    lbl_1_bss_D8 = 0;
    lbl_1_bss_D9 = 0;
    lbl_1_bss_E8.visit(fn_1_72E8);
    CallbackObject *node = lbl_1_bss_E8.front();
    if (node != lbl_1_bss_E8.end_node()) {
        node->unlink();
        node->insert4(&lbl_1_bss_E8);
    }
}

/* Returns without changing state. */
void fn_1_7054()
{
}

/* Selects looping character motion 0. */
void CallbackObject::fn_1_7058()
{
    fn_1_7460(this, 0, 0);
}

/* Moves the player toward the lane and selects the arrival callback on reaching it. */
void CallbackObject::fn_1_7088()
{
    position.x += 11.666667f;
    if (position.x > 210.0f) {
        position.x = 210.0f;
        Callback next = &CallbackObject::fn_1_7520;
        callback = next;
    }
    fn_1_7460(this, 2, 0);
}

/* Selects each player result motion, transfers earned coins and reports whether all earned none. */
u8 fn_1_7138(void)
{
    /* This array receives one ID per player and is never read.
       The four-player loop writes past its two slots. */
    s16 characterIds[2];
    Node *listEnd;
    s32 playerIndex;
    s16 mgCoin;
    CallbackObject *listHead;
    CallbackObject *firstEntry;
    CallbackObject *entry;
    CallbackObject *nextEntry;
    u8 allWaiting;
    s32 index;

    listHead = static_cast<CallbackObject *>(lbl_1_bss_E8.link0);
    firstEntry = listHead;
    entry = firstEntry;
    listEnd = &lbl_1_bss_E8;
    allWaiting = 1;
    index = 0;
    while (entry != listEnd) {
        characterIds[index++] = (s16)entry->character;
        if (entry->mgCoin > 0) {
            entry->fn_1_7200();
            allWaiting = 0;
        } else {
            entry->fn_1_7274();
        }
        mgCoin = (s16)entry->mgCoin;
        playerIndex = entry->controller;
        lbl_80265750[playerIndex].mgCoin = mgCoin;
        nextEntry = static_cast<CallbackObject *>(entry->link0);
        entry = nextEntry;
    }
    return allWaiting;
}

/* Selects the callback that plays character motion 6. */
void CallbackObject::fn_1_7200()
{
    Callback value = &CallbackObject::fn_1_7244;
    callback = value;
}

/* Selects nonlooping character motion 6. */
void CallbackObject::fn_1_7244()
{
    fn_1_7460(this, 6, 1);
}

/* Selects the callback that plays character motion 7. */
void CallbackObject::fn_1_7274()
{
    Callback value = &CallbackObject::fn_1_72B8;
    callback = value;
}

/* Selects nonlooping character motion 7. */
void CallbackObject::fn_1_72B8()
{
    fn_1_7460(this, 7, 1);
}

/* Runs the player callback, updates vibration and model placement, and counts the frame. */
void fn_1_72E8(CallbackObject *object)
{
    (object->*object->callback)();
    object->fn_1_73A4();
    object->fn_1_733C();
    object->count++;
}

/* Applies the player rotation and world position to its model. */
void CallbackObject::fn_1_733C()
{
    Vec rotation = {0.0f, 0.0f, 0.0f};
    rotation.y = rotationY;
    model.setRotation(&rotation);
    model.setPosition(&position);
}

/* Advances vibration countdowns and issues vibration for a connected controller. */
void CallbackObject::fn_1_73A4() {
    s32 vibrationType;

    vibrationType = 0;
    if (continuousVibrationFrames > 0) {
        vibrationType = 2;
        continuousVibrationFrames = continuousVibrationFrames - 1;
    }
    if (pulsedVibrationFrames > 0) {
        vibrationType = 1;
        pulsedVibrationFrames = pulsedVibrationFrames - 1;
    }
    if ((padIndex >= 0) && (vibrationType > 0)) {
        if (vibrationType == 1) {
            fn_8006A030((s16) controller, 0x14, 7, 3);
            return;
        }
        fn_8006A030((s16) controller, 0x14, 0x14, 0);
    }
}

/* Waits for the character motion to finish, then switches to motion 5. */
static inline void updateCompletedCharacterMotion(CallbackObject *owner)
{
    s16 modelIdCopy;
    s16 motionId;

    if (owner->motionEnded == 0) {
        if (fn_8005C284((s16)owner->character) < 0 &&
            fn_8005C240((s16)owner->character) != 0) {
            owner->motionEnded = 1;
        }
    } else {
        modelIdCopy = owner->model;
        motionId = owner->m_mot_id.at(5);
        if (owner->state != 5) {
            fn_80026A44(modelIdCopy, ((1 << 30) | (1 << 1)));
            fn_8005C2C8((s16)owner->character, motionId,
                               0.0f, 8.0f,
                               ((1 << 30) | (1 << 0)));
            owner->state = 5;
        }
    }
}

/* Copies the player coin count to its score display. */
static inline void applyCounterEvent(CallbackObject *owner)
{
    fn_1_8FC8(&ScoreDisplayOwner::getChecked(), owner->controller, owner->mgCoin);
}

/* Changes the character motion when its index differs, choosing looping from the queued flag. */
void fn_1_7460(CallbackObject *owner, s32 motionId,
                             u8 queued)
{
    s16 modelId;
    s16 selectedMotion;

    modelId = owner->model;
    selectedMotion = owner->m_mot_id.at(motionId);
    if (owner->state != motionId) {
        fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
        fn_8005C2C8((s16)owner->character, selectedMotion,
                           0.0f, 8.0f,
                           queued == 0 ? ((1 << 30) | (1 << 0)) : 0);
        owner->state = motionId;
    }
}

/* Turns the arriving player toward the stage and enters the play callback on request 2. */
void CallbackObject::fn_1_7520()
{
    CallbackObject::Callback callback;

    this->rotationY -= 3.0f;
    if (this->rotationY < 0.0f) {
        this->rotationY = 0.0f;
        fn_1_7460(this, 0, 0);
    } else {
        fn_1_7460(this, 1, 0);
    }

    if (currentRequest() == 2) {
        this->rotationY = 0.0f;
        callback = &CallbackObject::fn_1_7FEC;
        this->callback = callback;
    }
}

/* Reports whether the player controller has button 0x0100 pressed. */
bool fn_1_76D0(const CallbackObject *object)
{
    u32 buttons = lbl_802C0008[object->padIndex];
    return (buttons & 0x0100U) != 0;
}

/* Waits for the character motion to finish, then switches to motion 5. */
void CallbackObject::fn_1_770C()
{
    s16 motionId;
    s16 modelIdCopy;
    s16 modelId;

    if (motionEnded == 0) {
        if (fn_8005C284((s16)character) < 0 &&
            fn_8005C240((s16)character) != 0) {
            motionEnded = 1;
        }
    } else {
        modelId = (s16)model.modelId;
        modelIdCopy = modelId;
        motionId = m_mot_id.at(5);
        if (state != 5) {
            fn_80026A44(modelIdCopy, ((1 << 30) | (1 << 1)));
            fn_8005C2C8((s16)character, motionId,
                               0.0f, 8.0f,
                               ((1 << 30) | (1 << 0)));
            state = 5;
        }
    }
}

/* Advances the reaction motion and returns to play after 102 frames. */
void CallbackObject::fn_1_77F4()
{
    eventStep++;
    updateCompletedCharacterMotion(this);
    if ((f32)eventStep >
        102.0f) {
        CallbackObject::Callback callback =
            &CallbackObject::fn_1_7FEC;
        this->callback = callback;
        return;
    }
}

/* Waits for scrolling before starting the reaction and returns to play after 162 frames. */
void CallbackObject::fn_1_7960()
{
    eventStep++;
    if (active == 0) {
        if (requestList->fn_1_1F5C() != 0) {
            pulsedVibrationFrames = 30;
            active = 1;
            motionEnded = 0;
            fn_1_7460(this, 4, 1);
        }
        return;
    }
    updateCompletedCharacterMotion(this);
    if ((f32)eventStep > 162.0f) {
        CallbackObject::Callback callback =
            &CallbackObject::fn_1_7FEC;
        this->callback = callback;
        return;
    }
}

/* Handles the grabbed object, awards coins or starts a reaction, and times out the grab. */
void CallbackObject::fn_1_7B78()
{
    s32 requestType;

    frameCount++;
    if ((f32)frameCount > 45.0f) {
        frameCount = 0;
        fn_1_8238(this);
        requestList = 0;
        fn_1_7460(this, 0, 0);
        Callback next = &CallbackObject::fn_1_7F60;
        callback = next;
        return;
    }

    if (requestList != 0) {
        if (frameCount == 14) {
            if ((u8)requestList->fn_1_1F84(model, (s16)character) != 0) {
                requestType = requestList->fn_1_20C0();
                switch (requestType) {
    case 0:
        mgCoin++;
        if (lbl_1_bss_D8 == 0) {
            soundPlayAt(1, MSM_SE_CMN_COIN_GET, &position);
            lbl_1_bss_D8 = 1;
        }
        applyCounterEvent(this);
        return;
    case 1:
        mgCoin += 5;
        if (lbl_1_bss_D9 == 0) {
            soundPlayAt(1, MSM_SE_CMN_COIN_GET_LARGE, &position);
            lbl_1_bss_D9 = 1;
        }
        applyCounterEvent(this);
        return;
    case 2: {
        continuousVibrationFrames = 0x1E;
        eventStep = 0;
        motionEnded = 0;
                    fn_1_7460(this, 4, 1);
        Callback next = &CallbackObject::fn_1_77F4;
        callback = next;
        return;
    }
    case 3: {
        eventStep = 0;
        active = 0;
        Callback next = &CallbackObject::fn_1_7960;
        callback = next;
        break;
    }
    }
                }
            }
        }
}

/* Waits ten frames before returning to the play callback. */
void CallbackObject::fn_1_7F60()
{
    if ((f32)(frameCount++) > 9.0f) {
        CallbackObject::Callback callback = &CallbackObject::fn_1_7FEC;
        this->callback = callback;
        return;
    }
}

/* Checks player or computer input and starts a grab during request 2. */
void CallbackObject::fn_1_7FEC()
{
    if (currentRequest() != 2) {
        fn_1_7460(this, 0, 0);
        return;
    }

    bool playAlternateMotion;
    playAlternateMotion = padIndex < 0
        ? fn_1_8250()
        : fn_1_76D0(this);

    if (playAlternateMotion != 0) {
        fn_1_7460(this, 3, 1);
        requestList = fn_1_1DB8(controller);
        frameCount = 0;
        Callback callback = &CallbackObject::fn_1_7B78;
        this->callback = callback;
        return;
    }

    fn_1_7460(this, 0, 0);
}

/* Accepts the four computer difficulty values without changing player state. */
void fn_1_8238(CallbackObject *owner)
{
    switch (owner->mode) {
    case 0:
    case 1:
    case 2:
    case 3:
        break;
    }
}

/* Uses lane availability, randomness and computer difficulty to decide when to grab. */
bool CallbackObject::fn_1_8250()
{
    s32 listIndex = fn_1_1E68(controller);
    if (listIndex < 0) {
        availabilityChecked = 0;
        return 0;
    }

    if (((s32)RandomUnsigned() & 7) != 0) {
        return 0;
    }
    if (availabilityChecked != 0) {
        return 0;
    }
    availabilityChecked = 1;

    switch (mode) {
    case 0:
        if (listIndex >= 2) {
            if (((s32)RandomUnsigned() & 1) == 0) {
                return 1;
            }
            break;
        }
        if (((s32)RandomUnsigned() & 3) == 0) {
            return 1;
        }
        break;
    case 1:
        if (((s32)RandomUnsigned() & 1) == 0) {
            return 1;
        }
        break;
    case 2:
        if (((s32)RandomUnsigned() & 1) == 0) {
            return 1;
        }
        if (((s32)RandomUnsigned() & 7) == 0) {
            return 0;
        }
        if (listIndex >= 2) {
            break;
        }
        return 1;
    case 3:
        if (((s32)RandomUnsigned() & 7) == 0) {
            return 1;
        }
        if (((s32)RandomUnsigned() & 0xF) == 0) {
            return 0;
        }
        if (listIndex >= 2) {
            break;
        }
        return 1;
    default:
        break;
    }
    return 0;
}

extern "C" {
s32 lbl_1_bss_D4;
u8 lbl_1_bss_D8;
u8 lbl_1_bss_D9;
ListRoot<CallbackObject> lbl_1_bss_E8;
}
