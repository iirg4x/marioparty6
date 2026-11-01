#ifndef _GAME_CHARMAN_H
#define _GAME_CHARMAN_H

#include "dolphin.h"
#include "game/hu3d.h"
#include "game/armem.h"

#include "msm_se.h"
#include "datanum/charmot.h"

void fn_80056428(void);
AMEM_PTR fn_80056598(s16 charNo);
void fn_800565B4(s16 charNo);
void fn_8005664C(s16 charNo);
void fn_80056754(s16 charNo);
void fn_800567D8(s16 charNo);
void fn_8005702C(s16 charNo);
HU3D_MODELID fn_80057068(s16 charNo, s16 model);

HU3D_MOTIONID fn_8005B0B8(s16 charNo, unsigned int dataNum);
void fn_8005B398(s16 charNo, HU3D_MOTIONID motId, unsigned int motNo);
void fn_8005B420(s16 charNo, unsigned int motId);
void fn_8005B4BC(s16 charNo);
void fn_8005B7BC(s16 charNo);
void fn_8005BB8C(s16 charNo);
void fn_8005BE80(s16 charNo, HU3D_MOTIONID motId);

char **fn_8005C074(s16 charNo, s16 model);
char *fn_8005C0F0(s16 charNo, s16 model, s16 hookNo);

void fn_8005C16C(s16 charNo, float time);
float fn_8005C1B8(s16 charNo);
float fn_8005C1FC(s16 charNo);
BOOL fn_8005C240(s16 charNo);
HU3D_MOTIONID fn_8005C284(s16 charNo);
void fn_8005C2C8(s16 charNo, HU3D_MOTIONID motId, float start, float end, u32 attr);
float fn_8005C34C(s16 charNo);
void fn_8005C390(s16 charNo, float speed);
void fn_8005C3DC(s16 charNo, float start, float end);
void fn_8005C430(s16 charNo, u32 attr);
void fn_8005C47C(s16 charNo, u32 attr);
float fn_8005C4C8(s16 charNo);
HU3D_MODELID fn_8005C4E4(s16 charNo, s16 model, unsigned int *motDataNum,
    HU3D_MOTIONID *motId);
s32 fn_8005C750(s16 charNo);
void fn_8005C7A4(s16 charNo, char *objName);
void fn_8005D404(s16 cameraBit, HuVecF *pos);
void fn_8005D5A0(s16 cameraBit, HuVecF *pos);
void fn_8005D738(s16 charNo);
void fn_8005D864(s16 cameraBit, HuVecF *pos, HuVecF *rot);
void fn_8005DA5C(s16 charNo);
void fn_8005DB3C(s16 cameraBit, HuVecF *pos, HuVecF *rot);
void fn_8005DDEC(s16 layerNo);
void fn_8005DEB0(s16 charNo, s16 motNo, BOOL voiceOn);
void fn_8005DF74(s16 charNo, BOOL voicePanAuto);
void fn_8005DFC0(s16 charNo, BOOL fxFlag);
void fn_8005E070(s16 charNo, unsigned int dataNum, BOOL updateF);
s32 fn_8005E128(HU3D_MODELID modelId, HU3D_MOTIONID motId, s16 type, s16 npcNo);
s32 fn_8005E1D8(HU3D_MODELID modelId, HU3D_MOTIONID motId, s16 type);
void fn_8005F77C(s16 charNo, s16 stepFx);

void fn_8005F968(s16 charNo, HuVecF *pos);
void fn_8006065C(s16 cameraBit, float scale, HuVecF *pos);
void fn_800608C4(s16 cameraBit, HuVecF *pos, HuVecF *offset, float scale);
void fn_80060B10(s16 charNo, float yOfs, float ofsY);

unsigned int fn_80060E50(s16 charNo, s16 model);
unsigned int fn_80060EF0(s16 charNo, u16 motId);
void fn_80060F14(s16 charNo, s16 pan);
s16 fn_800591C8(s16 charNo, s16 seNo, u8 voiceFlag);
void fn_80060F40(s16 charNo, s16 vol);

void CharWinLoseVoicePlay(s16 charNo, unsigned int motId, s16 seId);
void fn_800611E4(s16 charNo1, s16 charNo2, s16 charNo3, s16 charNo4);

s16 fn_800613C0(s16 charNo, int motNo);
s16 fn_800614A4(s16 charNo, int motNo);

void fn_80056B7C(s16 *statList);
void fn_8005CF90(s16 cameraBit, HuVecF *pos, float scale);
void fn_8005FC84(s16 charNo, HuVecF *pos);
void fn_80061E00(s16 charNo, float scale);
u32 fn_80062094(s16 charNo);
HU3D_LIGHTID fn_800614AC(HuVecF *pos, HuVecF *dir, GXColor *color);
HU3D_LIGHTID fn_800615B4(float posX, float posY, float posZ, float dirX, float dirY,
    float dirZ, u8 r, u8 g, u8 b);
void fn_80061728(s32 func, float cutoff);
void fn_80061814(void);
void fn_80061904(s32 func, float cutoff, float brightness);
void fn_80061A04(u8 r, u8 g, u8 b, u8 a);
void fn_80061AE4(HuVecF *pos, HuVecF *aim);
void fn_80061BE0(float posX, float posY, float posZ, float aimX, float aimY, float aimZ);
void fn_80061D04(BOOL staticF);

extern unsigned int lbl_8022E718[14][6];

#endif
