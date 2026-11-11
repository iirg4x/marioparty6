/* Player configuration, controller input and rumble. The assert prints this file's name. */
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/framework/p_info.h"
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
/* Rumble is enabled until setRumbleEnabled clears it. */
unsigned char rumbleEnabled = 1;
/* Each controller's PAD_CHANn_BIT. */
extern const unsigned long padChannelBits[4] = {
    PAD_CHAN0_BIT, PAD_CHAN1_BIT, PAD_CHAN2_BIT, PAD_CHAN3_BIT
};
}

PlayerInfo::PlayerInfo(int playerNo)
    : playerNo(playerNo)
{
}

int PlayerInfo::padNo()
{
    return isComputer() ? -1 : lbl_80265728[playerNo].padNo;
}

int PlayerInfo::characterNo()
{
    return lbl_80265728[playerNo].charNo;
}

s16 PlayerInfo::groupNo()
{
    return lbl_80265728[playerNo].grpNo;
}

bool PlayerInfo::isComputer()
{
    return lbl_80265728[playerNo].type != 0;
}

int PlayerInfo::difficulty()
{
    return lbl_80265728[playerNo].comDif;
}

void PlayerInfo::rumble(s32 type, s32 duration)
{
    if (rumbleEnabled) {
        s32 playerNo = this->playerNo;
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
                assert(!"rumble type error");
                break;
            }
        }
    }
}

bool noRumbleController(void)
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
            (padChannelBits[pad] & fn_80006FC0()) != 0) {
            ++unavailableCount;
        }
    }
    return unavailableCount == count;
}

int PlayerInfo::buttons()
{
    int pad = padNo();
    return pad < 0 ? 0 : lbl_802C0010[pad];
}

int PlayerInfo::buttonsDown()
{
    int pad = padNo();
    return pad < 0 ? 0 : lbl_802C0008[pad];
}

int PlayerInfo::buttonsRepeat()
{
    int pad = padNo();
    return pad < 0 ? 0 : lbl_802C0000[pad];
}

float PlayerInfo::stickX()
{
    int pad = padNo();
    return pad < 0 ? 0.0f : lbl_8024C7D8[pad];
}

float PlayerInfo::stickY()
{
    int pad = padNo();
    return pad < 0 ? 0.0f : lbl_8024C7E8[pad];
}

float PlayerInfo::subStickX()
{
    int pad = padNo();
    return pad < 0 ? 0.0f : lbl_8024C7F8[pad];
}

float PlayerInfo::subStickY()
{
    int pad = padNo();
    return pad < 0 ? 0.0f : lbl_8024C808[pad];
}

float PlayerInfo::triggerL()
{
    int pad = padNo();
    return pad < 0 ? 0.0f : lbl_8024C818[pad];
}

float PlayerInfo::triggerR()
{
    int pad = padNo();
    return pad < 0 ? 0.0f : lbl_8024C828[pad];
}

int PlayerInfo::dpad()
{
    int pad = padNo();
    return pad < 0 ? 0 : lbl_802BFFE4[pad];
}

int PlayerInfo::dpadDown()
{
    int pad = padNo();
    return pad < 0 ? 0 : lbl_802BFFDC[pad];
}

int PlayerInfo::dpadRepeat()
{
    int pad = padNo();
    return pad < 0 ? 0 : lbl_802BFFE0[pad];
}

int PlayerInfo::stickDirections()
{
    int pad = padNo();
    return pad < 0 ? 0 : fn_80006B90(pad);
}

int PlayerInfo::subStickDirections()
{
    int pad = padNo();
    return pad < 0 ? 0 : fn_80006D00(pad);
}

float PlayerInfo::height()
{
    return fn_8005C4C8(characterNo());
}

void setRumbleEnabled(unsigned char enabled)
{
    rumbleEnabled = enabled;
}
