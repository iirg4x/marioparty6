
#include <math.h>
extern "C" {
#include "game/gamework.h"
#include "game/pad.h"
#include "game/charman.h"
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "REL/m654DLL/player_height_view.h"

struct Player {
    int playerNo;
    static bool fn_1_1C6D4();
    static void fn_1_1D2F8(unsigned char);
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

extern "C" {
extern char lbl_1_data_1483[];
const u32 lbl_1_rodata_9E0[4] = {0x80000000, 0x40000000, 0x20000000, 0x10000000};
extern unsigned char lbl_1_data_1798;
}

bool fn_1_1C590(s32 *);
void fn_1_1C4E8(s32 *playerNo, s32 number)
{
    *playerNo = number;
}
s16 fn_1_1C4F0(s32 *playerNo)
{
    s16 padNo;
    if ((u8)fn_1_1C590(playerNo) != 0) {
        padNo = -1;
    } else {
        padNo = lbl_80265728[*playerNo].padNo;
    }
    return padNo;
}
s16 fn_1_1C558(s32 *playerNo)
{
    return lbl_80265728[*playerNo].charNo;
}
s16 fn_1_1C574(s32 *playerNo)
{
    return lbl_80265728[*playerNo].grpNo;
}
bool fn_1_1C590(s32 *playerNo)
{
    return lbl_80265728[*playerNo].type != 0;
}
s16 fn_1_1C5BC(s32 *playerNo)
{
    return lbl_80265728[*playerNo].comDif;
}

extern "C" unsigned char lbl_1_data_1798 = 1;

extern "C" {
void fn_8006A030(s16 playerNo, s16 duration, s16 off, s16 on);

void __msl_assertion_failed(const char *, const char *, int);
}

void fn_1_1C5D8(s32 *player, s32 type, s32 duration)
{
    if (lbl_1_data_1798) {
        int playerNo = *player;
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

bool Player::fn_1_1C6D4(void)
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
            (lbl_1_rodata_9E0[pad] & fn_80006FC0()) != 0) {
            ++unavailableCount;
        }
    }
    return unavailableCount == count;
}

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

float PlayerHeightView::fn_1_37688()
{
    short charNo=lbl_80265728[playerNo].charNo;
    return fn_8005C4C8((short)charNo);
}

void Player::fn_1_1D2F8(unsigned char value) { lbl_1_data_1798 = value; }
