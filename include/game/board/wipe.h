#ifndef _BOARD_WIPE_H
#define _BOARD_WIPE_H

#include "dolphin/types.h"

void mbWipeWait(void);
void mbWipeCreate(s16 mode, s16 type, s16 durationFrames);
void mbWipeFadeIn(void);
void mbWipeFadeOut(void);
void mbWipeFadeOutTime(int durationFrames);
void mbWipeWhiteFadeInTime(int durationFrames);
void mbWipeWhiteFadeOutTime(int durationFrames);
void mbWipeSpecialCreate(int mode, int type, int durationFrames);
void mbWipeSpecialWait(void);
void mbWipeSpecialKill(void);
void mbWipeSpecialInit(void);
void mbWipeSpecialClose(void);
void mbWipeDissolveFadeInTime(int time);
void mbWipeDissolveFadeOut(void);
void mbWipeDissolveFadeOutTime(int time);
void mbWipeDissolveFadeIn(void);
BOOL mbWipeSpecialStatGet(void);
BOOL mbWipeSpecialCheck(void);
void mbWipeSpecialFadeOutCreate(int type, int durationFrames);
void mbWipeSpecialFadeInCreate(int type, int durationFrames);

#endif
