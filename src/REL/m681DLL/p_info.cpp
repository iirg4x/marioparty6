/* Player configuration, controller input and the controller-number sequence. */
#include "dolphin/types.h"
extern "C" {
#include "game/gamework.h"
#include "game/pad.h"
#include "game/charman.h"
#include "game/object.h"
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "REL/framework/chara_motion.h"

// The framework's character: its model handle follows the motion owner.
struct Character : MotionOwner {
    s16 modelId;
    Character(unsigned int characterNo, unsigned int modelNo);
    ~Character();
};

class PlayerIdentity {
public:
    s32 playerIndex;
    PlayerIdentity(s32 index) { SetPlayerIndex(index); }
    void SetPlayerIndex(s32 playerIndex);
};

class PlayerConfigSelection : public PlayerIdentity, public Character {
public:
    PlayerConfigSelection(s32 playerIndex, s32 mode);
    ~PlayerConfigSelection();
};

extern "C" {
void __msl_assertion_failed(const char *, const char *, int);
unsigned char lbl_1_data_16D0 = 1;
extern const u32 lbl_1_rodata_950[4] = { 0x80000000, 0x40000000, 0x20000000, 0x10000000 };
}

/* Selects a character configuration and applies its game mode. */
PlayerConfigSelection::PlayerConfigSelection(s32 playerIndex, s32 mode)
    : PlayerIdentity(playerIndex), Character(lbl_80265728[playerIndex].charNo, mode)
{
}

PlayerConfigSelection::~PlayerConfigSelection() {}

void PlayerIdentity::SetPlayerIndex(s32 playerIndex)
{
    this->playerIndex = playerIndex;
}

extern "C" {

}
bool fn_1_1BDDC(s32 *playerIndex);
extern "C" {

}
s16 fn_1_1BD3C(s32 *playerIndex)
{
    s16 padNo;
    if ((u8)fn_1_1BDDC(playerIndex) != 0) {
        padNo = -1;
    } else {
        padNo = lbl_80265728[*playerIndex].padNo;
    }
    return padNo;
}
extern "C" {

}
s32 fn_1_1BDA4(s32 *playerIndex)
{
    return lbl_80265728[*playerIndex].charNo;
}
extern "C" {

}
s16 fn_1_1BDC0(s32 *playerIndex)
{
    return lbl_80265728[*playerIndex].grpNo;
}
extern "C" {

}
bool fn_1_1BDDC(s32 *playerIndex)
{
    return lbl_80265728[*playerIndex].type != 0;
}
extern "C" {

}
s16 fn_1_1BE08(s32 *playerIndex)
{
    return lbl_80265728[*playerIndex].comDif;
}
extern "C" {

}

void fn_1_1BE24(s32 *playerIndex, s32 type, s32 duration)
{
    if (lbl_1_data_16D0) {
        s32 playerNo = *playerIndex;
        if (playerNo >= 0) {
            switch (type) {
            case 0:
                fn_8006A030((s16)playerNo, (s16)duration, 4, 4);
                break;
            case 1:
                fn_8006A030((s16)playerNo, (s16)duration, 7, 3);
                break;
            case 2:
                fn_8006A030((s16)playerNo, (s16)duration, 0x14, 0);
                break;
            default:
                (!"rumble type error") ? (void)0 : __msl_assertion_failed("!\"rumble type error\"", "p_info.cpp", 0x4b);
                break;
            }
        }
    }
}

extern "C" bool fn_1_1BF20(void);

bool fn_1_1BF20(void)
{
    if ((s32)lbl_80265E30.vibrateF == 0) return 0;

    std::vector<s16> pads(4);
    pads.clear();
    s32 playerIndex;
    s32 padIndex;
    s32 count;
    s32 unavailableCount;
    for (playerIndex = 0; playerIndex < 4; ++playerIndex) {
        if (lbl_80265728[playerIndex].type == 0) {
            pads.push_back(lbl_80265728[playerIndex].padNo);
        }
    }
    count = pads.size();
    if (count <= 0) return 1;

    unavailableCount = 0;
    for (padIndex = 0; padIndex < count; ++padIndex) {
        s16 pad = (s16)pads[padIndex];
        if (HuPadStatGet(pad) != 0 ||
            (lbl_1_rodata_950[pad] & fn_80006FC0()) != 0) {
            ++unavailableCount;
        }
    }
    return unavailableCount == count;
}

struct Player {
    int playerNo;
    bool isComputer() { return lbl_80265728[playerNo].type != 0; }
    int padNo() { return isComputer() ? -1 : lbl_80265728[playerNo].padNo; }
    int fn_1_36C6C();
    int fn_1_36D1C();
    int fn_1_36DCC();
    float fn_1_36E7C();
    float fn_1_36F44();
    float fn_1_3700C();
    float fn_1_370D4();
    float fn_1_3719C();
    float fn_1_37264();
    int fn_1_3732C();
    int fn_1_373D8();
    int fn_1_37484();
    int fn_1_37530();
    int fn_1_375DC();
};

int Player::fn_1_36C6C() { int pad = padNo(); return pad < 0 ? 0 : lbl_802C0010[pad]; }

int Player::fn_1_36D1C() { int pad = padNo(); return pad < 0 ? 0 : lbl_802C0008[pad]; }

int Player::fn_1_36DCC() { int pad = padNo(); return pad < 0 ? 0 : lbl_802C0000[pad]; }

float Player::fn_1_36E7C() { int pad = padNo(); return pad < 0 ? 0.0f : lbl_8024C7D8[pad]; }

float Player::fn_1_36F44() { int pad = padNo(); return pad < 0 ? 0.0f : lbl_8024C7E8[pad]; }

float Player::fn_1_3700C() { int pad = padNo(); return pad < 0 ? 0.0f : lbl_8024C7F8[pad]; }

float Player::fn_1_370D4() { int pad = padNo(); return pad < 0 ? 0.0f : lbl_8024C808[pad]; }

float Player::fn_1_3719C() { int pad = padNo(); return pad < 0 ? 0.0f : lbl_8024C818[pad]; }

float Player::fn_1_37264() { int pad = padNo(); return pad < 0 ? 0.0f : lbl_8024C828[pad]; }

int Player::fn_1_3732C() { int pad = padNo(); return pad < 0 ? 0 : lbl_802BFFE4[pad]; }

int Player::fn_1_373D8() { int pad = padNo(); return pad < 0 ? 0 : lbl_802BFFDC[pad]; }

int Player::fn_1_37484() { int pad = padNo(); return pad < 0 ? 0 : lbl_802BFFE0[pad]; }

int Player::fn_1_37530() { int pad = padNo(); return pad < 0 ? 0 : fn_80006B90(pad); }

int Player::fn_1_375DC() { int pad = padNo(); return pad < 0 ? 0 : fn_80006D00(pad); }

struct PlayerHeightView {
    int playerNo;
    float fn_1_37688();
};

float PlayerHeightView::fn_1_37688()
{
    short charNo = lbl_80265728[playerNo].charNo;
    return fn_8005C4C8((short)charNo);
}

void fn_1_1CCE8(unsigned char value)
{
    lbl_1_data_16D0 = value;
}
