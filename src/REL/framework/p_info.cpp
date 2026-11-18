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
// Stores the player index used to look up configuration and input.
PlayerInfo::PlayerInfo(int playerNo)
    : playerNo(playerNo)
{
}
// Returns the human player controller index, or minus one for a computer player.
int PlayerInfo::padNo()
{
    return isComputer() ? -1 : lbl_80265728[playerNo].padNo;
}
// Returns the configured character number.
int PlayerInfo::characterNo()
{
    return lbl_80265728[playerNo].charNo;
}
// Returns the configured player group.
int PlayerInfo::groupNo()
{
    return lbl_80265728[playerNo].grpNo;
}
// Tests whether the player is computer controlled.
bool PlayerInfo::isComputer()
{
    return lbl_80265728[playerNo].type != 0;
}
// Returns the configured computer difficulty.
int PlayerInfo::difficulty()
{
    return lbl_80265728[playerNo].comDif;
}
// Starts the selected rumble pattern when framework rumble is enabled.
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

// Tests whether rumble is enabled but no human controller can provide it.
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

// Returns held buttons, or zero when the player has no controller.
int PlayerInfo::buttons()
{
    int pad = padNo();
    return pad < 0 ? 0 : lbl_802C0010[pad];
}

// Returns newly pressed buttons, or zero when the player has no controller.
int PlayerInfo::buttonsDown()
{
    int pad = padNo();
    return pad < 0 ? 0 : lbl_802C0008[pad];
}

// Returns repeating buttons, or zero when the player has no controller.
int PlayerInfo::buttonsRepeat()
{
    int pad = padNo();
    return pad < 0 ? 0 : lbl_802C0000[pad];
}

// Returns the main stick horizontal input, or zero without a controller.
float PlayerInfo::stickX()
{
    int pad = padNo();
    return pad < 0 ? 0.0f : lbl_8024C7D8[pad];
}

// Returns the main stick vertical input, or zero without a controller.
float PlayerInfo::stickY()
{
    int pad = padNo();
    return pad < 0 ? 0.0f : lbl_8024C7E8[pad];
}

// Returns the secondary stick horizontal input, or zero without a controller.
float PlayerInfo::subStickX()
{
    int pad = padNo();
    return pad < 0 ? 0.0f : lbl_8024C7F8[pad];
}

// Returns the secondary stick vertical input, or zero without a controller.
float PlayerInfo::subStickY()
{
    int pad = padNo();
    return pad < 0 ? 0.0f : lbl_8024C808[pad];
}

// Returns the left trigger input, or zero without a controller.
float PlayerInfo::triggerL()
{
    int pad = padNo();
    return pad < 0 ? 0.0f : lbl_8024C818[pad];
}

// Returns the right trigger input, or zero without a controller.
float PlayerInfo::triggerR()
{
    int pad = padNo();
    return pad < 0 ? 0.0f : lbl_8024C828[pad];
}

// Returns held directional-pad buttons, or zero without a controller.
int PlayerInfo::dpad()
{
    int pad = padNo();
    return pad < 0 ? 0 : lbl_802BFFE4[pad];
}

// Returns newly pressed directional-pad buttons, or zero without a controller.
int PlayerInfo::dpadDown()
{
    int pad = padNo();
    return pad < 0 ? 0 : lbl_802BFFDC[pad];
}

// Returns repeating directional-pad buttons, or zero without a controller.
int PlayerInfo::dpadRepeat()
{
    int pad = padNo();
    return pad < 0 ? 0 : lbl_802BFFE0[pad];
}

// Returns main stick direction bits, or zero without a controller.
int PlayerInfo::stickDirections()
{
    int pad = padNo();
    return pad < 0 ? 0 : fn_80006B90(pad);
}

// Returns secondary stick direction bits, or zero without a controller.
int PlayerInfo::subStickDirections()
{
    int pad = padNo();
    return pad < 0 ? 0 : fn_80006D00(pad);
}

// Returns the configured character height.
float PlayerInfo::height()
{
    return fn_8005C4C8(characterNo());
}

// Sets whether framework player rumble requests are enabled.
void setRumbleEnabled(unsigned char enabled)
{
    rumbleEnabled = enabled;
}
