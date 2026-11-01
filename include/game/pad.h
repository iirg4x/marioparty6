#ifndef _GAME_PAD_H
#define _GAME_PAD_H

#include "dolphin.h"

extern u16 lbl_802C0010[4];
extern u16 lbl_802C0008[4];
extern u16 lbl_802C0000[4];
extern s8 lbl_802BFFFC[4];
extern s8 lbl_802BFFF8[4];
extern s8 lbl_802BFFF4[4];
extern s8 lbl_802BFFF0[4];
extern u8 lbl_802BFFEC[4];
extern u8 lbl_802BFFE8[4];
extern u8 lbl_802BFFE4[4];
extern u8 lbl_802BFFDC[4];

extern u8 lbl_802BFFE0[4];
extern s8 lbl_802BFFD8[4];
extern u16 lbl_802BFFD0[4];
extern u16 lbl_802BFFC8[4];
extern s32 lbl_802BFF84;
extern u16 lbl_802BFF80;

extern float lbl_8024C7D8[4];
extern float lbl_8024C7E8[4];
extern float lbl_8024C7F8[4];
extern float lbl_8024C808[4];
extern float lbl_8024C818[4];
extern float lbl_8024C828[4];

void fn_80005C98(void);
void fn_80005D90(void);
void fn_80006E70(s16 pad, s16 duration, s16 off, s16 on);
void fn_80006ECC(s16 pad);
void fn_80006F3C(void);
s16 HuPadStatGet(s16 pad);
u32 fn_80006FC0(void);
u16 fn_80006B90(s16 padNo);
u16 fn_80006D00(s16 padNo);
u16 fn_80184DD4(void);

#endif
