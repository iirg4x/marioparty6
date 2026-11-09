#include "REL/m652DLL/player_config.h"
/* Player configuration, controller input and the controller-number sequence. */
#include <math.h>
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

extern "C" {
void __msl_assertion_failed(const char *, const char *, int);
unsigned char lbl_1_data_1670 = 1;
char lbl_1_data_1671[] = "rumble type error";
char lbl_1_data_1683[] = "!\"rumble type error\"";
char lbl_1_data_1698[] = "p_info.cpp";
extern const unsigned long lbl_1_rodata_8E8[4] = {
    PAD_CHAN0_BIT, PAD_CHAN1_BIT, PAD_CHAN2_BIT, PAD_CHAN3_BIT
};
}

#include "REL/m652DLL/player_identity.h"

extern "C" {

Player::Player(int value) : playerNo(value) {}
}

s16 fn_1_1B250(s32 *playerNo)
{
    s16 padNo;
    if ((u8)fn_1_1B2F0(playerNo) != 0) {
        padNo = -1;
    } else {
        padNo = lbl_80265728[*playerNo].padNo;
    }
    return padNo;
}
int fn_1_1B2B8(s32 *playerNo) { return lbl_80265728[*playerNo].charNo; }

s16 fn_1_1B2D4(s32 *playerNo) { return lbl_80265728[*playerNo].grpNo; }
bool fn_1_1B2F0(s32 *playerNo) { return lbl_80265728[*playerNo].type != 0; }

int fn_1_1B31C(s32 *playerNo) { return lbl_80265728[*playerNo].comDif; }

void Player::fn_1_1B338(s32 type, s32 duration)
{
    if (lbl_1_data_1670) {
        s32 playerNo = this->playerNo;
        if (playerNo >= 0) {
            switch (type) {
            case 0: fn_8006A030((s16)playerNo, (s16)duration, 4, 4); break;
            case 1: fn_8006A030((s16)playerNo, (s16)duration, 7, 3); break;
            case 2: fn_8006A030((s16)playerNo, (s16)duration, 0x14, 0); break;
            default:
                (!lbl_1_data_1671) ? (void)0 :
                    __msl_assertion_failed(lbl_1_data_1683, lbl_1_data_1698, 0x4B);
                break;
            }
        }
    }
}

bool fn_1_1B434(void)
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
            (lbl_1_rodata_8E8[pad] & fn_80006FC0()) != 0) {
            ++unavailableCount;
        }
    }
    return unavailableCount == count;
}

inline int Player::padNo() { return isComputer() ? -1 : lbl_80265728[playerNo].padNo; }

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

float Player::fn_1_37688()
{
    short charNo = lbl_80265728[playerNo].charNo;
    return fn_8005C4C8((short)charNo);
}

void fn_1_1C1FC(unsigned char value) { lbl_1_data_1670 = value; }
