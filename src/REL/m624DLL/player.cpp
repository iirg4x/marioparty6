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
 *
 *
 *
 *
 *
 *
 */
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "dolphin/types.h"
#include "REL/framework/sound.h"
#include "REL/framework/player_table_1vs1.h"
#include "REL/m624DLL/game_constants.h"

#include "dolphin/mtx.h"

#include "PowerPC_EABI_Support/Runtime/NMWException.h"

extern "C" {
#include "dolphin/os.h"
}

extern "C" {
#include "dolphin/gx.h"
}

extern "C" {
#include "math.h"
}

extern "C" {
#include "game/charman.h"
}

#include "REL/m624DLL/player_setup.h"

#include "REL/m624DLL/scene_sequence_contract.h"

extern "C" {
#include "game/gamework.h"
}

extern "C" {
#include "game/object.h"
}

#include "REL/m624DLL/player_base.h"

#include "REL/framework/linknode.h"

extern "C" {
#include "dolphin.h"
}

extern "C" {
#include "humath.h"
}

extern "C" {
#include "game/hu3d.h"
}

extern "C" {
#include <string.h>
}

ListRoot<Player> lbl_1_bss_9C;

extern "C" { s32 lbl_1_bss_A4; }

// PlayerRunner construction uses the C++ allocation expression below.

void fn_1_60B8(void)
{
    s32 items[2];
    Position *bounds;
    s32 index;

    static StartPositions startPositions = {
        { { cmToWorld(-860.0f), cmToWorld(525.0f), cmToWorld(-1250.0f) } },
        { { cmToWorld(-720.0f), cmToWorld(525.0f), cmToWorld(-1250.0f) } },
    };
    bounds = &startPositions.first;

    makePlayerTable1vs1(items);
    index = 0;
    while (index < 2) {
        new PlayerRunner(items[index], bounds++);
        index++;
    }
    fn_8005DDEC(4);
    lbl_1_bss_A4 = 0;
}

extern "C" { extern GW_PLAYER_CONF lbl_80265728[]; }

void fn_1_6A9C(Player *object);

static inline void setMode(Player *player, Player::Mode next)
{
    player->mode = next;
}

void fn_1_6B3C(Player *object);

void fn_1_6B74(Player *owner, u32 dataNum);

void fn_1_6BEC(Player *owner, s32 motion, f32 startFrame, f32 endFrame, u8 shift);

void fn_1_6CD8(Player *owner, s32 motion, u8 shift);

u32 fn_1_6DCC(Player *self);

u8 fn_1_6E08(Player *self);

void fn_1_6E4C(Player *self, float speed);

f32 fn_1_6E88(Player *owner);

// Starts a motion from m_motion_id, unless it is already playing and has not ended.
inline void Player::setMotion(s32 motion, f32 startFrame, f32 endFrame, u8 shift)
{
    s32 shiftFlags;

    assert(motion < m_motion_id.size());
    if (m_motion != motion || fn_1_6E08(this)) {
        fn_8005C430((s16)character, ((1 << 30) | (1 << 1)));
        if (shift) {
            shiftFlags = ((1 << 30) | (1 << 0));
        } else {
            shiftFlags = 0;
        }
        fn_8005C2C8((s16)character, m_motion_id.at(motion), startFrame, endFrame, shiftFlags);
        m_motion = (s16)motion;
    }
}

void fn_1_6EF4(Player *owner, u8 enabled);

void fn_1_6F68(Player *task, s32 mode);

void fn_1_6214()
{
    lbl_1_bss_9C.visit(fn_1_6A9C);
    Player *node = lbl_1_bss_9C.front();
    if (node != lbl_1_bss_9C.end_node()) {
        node->unlink();
        node->insert4(&lbl_1_bss_9C);
    }
}

void fn_1_6298()
{
    lbl_1_bss_9C.visit(fn_1_6B3C);
}

void fn_1_62C8()
{
    Player *node = lbl_1_bss_9C.front();
    Node *sentinel = lbl_1_bss_9C.end_node();
    for (; node != sentinel; node = node->next_node()) {
        setMode(node, &Player::moveToStart);

    }
}

void fn_1_635C()
{
    Player *node = lbl_1_bss_9C.front();
    Node *sentinel = lbl_1_bss_9C.end_node();
    for (; node != sentinel; node = node->next_node()) {
        setMode(node, &Player::startRunning);

    }
    soundPlay(0xD);
}

void fn_1_63F8()
{
    Player *node = lbl_1_bss_9C.front();
    Node *end = lbl_1_bss_9C.end_node();
    for (; node != end; node = node->next_node()) node->stopRespawning();
}

void fn_1_6468()
{
    Player *node = lbl_1_bss_9C.front();
    Node *end = lbl_1_bss_9C.end_node();
    for (; node != end; node = node->next_node()) node->showResult();
}

void fn_1_64D8()
{
    while (!lbl_1_bss_9C.empty()) {
        delete lbl_1_bss_9C.back();
    }
}

s32 fn_1_65A0(void)
{
    return lbl_1_bss_A4;
}

Player *fn_1_65B0(s32 key)
{
    Player *node = lbl_1_bss_9C.front();
    Node *end = lbl_1_bss_9C.end_node();
    for (; node != end; node = node->next_node()) {
        if (node->playerNo == key) return node;
    }
    return 0;
}

Player *fn_1_6624()
{
    Player *node = lbl_1_bss_9C.front();
    Node *end = lbl_1_bss_9C.end_node();
    for (; node != end; node = node->next_node()) {
        if (node->get_group() >= 2) return node;
    }
    return 0;
}

Player::Player(s32 playerNo)
    : mode(&Player::fn_1_7060), playerNo(playerNo), character(lbl_80265728[playerNo].charNo),
      group(lbl_80265728[playerNo].grpNo),
      pad(lbl_80265728[playerNo].type != 0 ? -1 : lbl_80265728[playerNo].padNo),
      difficulty(lbl_80265728[playerNo].comDif), countdown(0), frame(0), m_motion(-1)
{
    insert4(&lbl_1_bss_9C);
}

inline Player::~Player() {}

void fn_1_6A9C(Player *object)
{
    object->prepare();
    (object->*object->mode)();
    object->finish();
    object->countDown();
    object->frame += 1;
}

void fn_1_6B3C(Player *object)
{
    object->update();
}

void fn_1_6B74(Player *owner, u32 dataNum)
{
    s16 motionId = fn_8005B0B8((s16)owner->character, dataNum);
    owner->m_motion_id.push_back(motionId);
}

void fn_1_6BEC(Player *owner, s32 motion, f32 startFrame, f32 endFrame, u8 shift)
{
    owner->setMotion(motion, startFrame, endFrame, shift);
}

void fn_1_6CD8(Player *owner, s32 motion, u8 shift)
{
    owner->setMotion(motion, 0.0f, 16.0f, shift);
}

u32 fn_1_6DCC(Player *self)
{
    return (u32) fn_8005C284((s16) self->character) >> 31U;
}

u8 fn_1_6E08(Player *self)
{
    return (u8) (fn_8005C240((s16) self->character) != 0);
}

void fn_1_6E4C(Player *self, float speed)
{
    fn_8005C390((s16) self->character, speed);
}

/* Returns the remaining time for a shifted motion, or the regular motion time. */
f32 fn_1_6E88(Player *owner)
{
    if (fn_8005C284((s16)owner->character) >= 0) {
        return fn_8005C34C((s16)owner->character) - 1.0f;
    }
    return fn_8005C1B8((s16)owner->character);
}

void fn_1_6EF4(Player *owner, u8 enabled)
{
    if (enabled != 0) {
        fn_8005C47C((s16)owner->character, 1U);
    } else {
        fn_8005C430((s16)owner->character, 1U);
    }
    fn_8005DFC0((s16)owner->character, enabled);
}

void fn_1_6F68(Player *task, s32 mode)
{
    switch (mode) {
    case 0:
        if (task->countdown <= 0) {
            s32 convertedCountdown;

            fn_8006A030((s16)task->playerNo, (s16)secondsToFrames(1L), 0x14, 0);
            convertedCountdown = (s32)secondsToFrames(0.6);
            task->countdown = convertedCountdown;
        }
        break;
    case 1:
        fn_8006A030((s16)task->playerNo, (s16)secondsToFrames(0.4), 0x14, 0);
        return;
    case 2:
        fn_8006A030((s16)task->playerNo, 0x14, 7, 3);
        break;
    }
}

void Player::fn_1_7060()
{
}

void Player::moveToStart()
{
}
