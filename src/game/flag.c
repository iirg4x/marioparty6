// Stores and accesses save, common, board, and system game flags.
#include "dolphin.h"
#include "game/gamework.h"

#define FLAG_GROUP_MASK 0xFFFF0000

static u8 _Sys_Flag[16];

// Selects the byte array for a flag group; called by the flag check, set, and clear functions.
static u8 *GetFlagPtr(u32 flagId)
{
	u8 *flagBytes;
	u32 flagGroup = flagId >> 16;
	if((flagId & FLAG_GROUP_MASK) == (3 << 16)) {
		flagBytes = _Sys_Flag;
	} else {
		flagBytes = &lbl_80265B70.flag[flagGroup][0];
	}
	return flagBytes;
}

// Returns zero when the flag is clear, or the selected bit mask when it is set.
BOOL fn_8006A124(u32 flagId)
{
	u8 *flagBytes = GetFlagPtr(flagId);
	u16 flagIndex = flagId;
	return flagBytes[flagIndex/8] & (1 << (flagIndex % 8));
}

// Sets one flag bit when game code records board, minigame, or system state.
void fn_8006A1C4(u32 flagId)
{
	u8 *flagBytes = GetFlagPtr(flagId);
	u16 flagIndex = flagId;
	flagBytes[flagIndex/8] |= (1 << (flagIndex % 8));
}

// Clears one flag bit when game code resets board, minigame, or system state.
void fn_8006A268(u32 flagId)
{
	u8 *flagBytes = GetFlagPtr(flagId);
	u16 flagIndex = flagId;
	flagBytes[flagIndex/8] &= ~(1 << (flagIndex % 8));
}

// Called by GWInit at startup to clear the separate system-group flag bytes.
void _InitFlag(void)
{
	memset(_Sys_Flag, 0, sizeof(_Sys_Flag));
}
