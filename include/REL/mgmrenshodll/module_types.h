/* Common engine interfaces used by Endurance Alley source files. */
#ifndef REL_MGMRENSHODLL_MODULE_TYPES_H
#define REL_MGMRENSHODLL_MODULE_TYPES_H

#include <math.h>
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/player.h"
#include "game/board/window.h"
#include "game/window.h"
#include "game/printfunc.h"
#include "string.h"
#include "stdio.h"
#include "game/esprite.h"
#include "game/mic.h"
#include "game/thpmain.h"

void fn_1_E9C(void);
void fn_1_EEBC(void);
void fn_1_1028(void);
void fn_1_102C(Vec *difference);
void fn_1_106C(s16 selectedCamera);
void fn_1_2F9C(void);
f32 fn_1_4A2C(void);
f32 fn_1_4ADC(void);
f32 fn_1_4C3C(void);
void fn_1_4C4C(void);
void fn_1_4C70(void);
void fn_1_4C84(void);
void fn_1_4CA8(void);
void fn_1_4CBC(void);
void fn_1_4CF0(void);
void fn_1_4D24(void);
void fn_1_4D58(void);
void fn_1_4D8C(void);
s16 fn_1_4E00(void);
void fn_1_4E24(void);
s16 fn_1_4E58(void);
void fn_1_4EB0(void);
void fn_1_4F0C(float zOffset);
void fn_1_14B8(void);
void fn_1_15CC(void);
void fn_1_16D0(void);
void fn_1_17DC(void);
void fn_1_1938(void);
s16 fn_1_1A78(void);
void fn_1_2038(void);
void fn_1_221C(void);
s16 fn_1_2A00(void);
void fn_1_94A8(void);
void fn_1_9978(void);
void fn_1_9A70(void);
void fn_1_9B9C(void);
void fn_1_9D2C(void);
void fn_1_9E20(void);
s16 fn_1_A00C(void);
s16 fn_1_A01C(s16 player);
s16 fn_1_A05C(s16 player);
s16 fn_1_A094(s16 player);
void fn_1_A0CC(void);
void fn_1_A348(void);
void fn_1_D5A0(void);
void fn_1_D964(void);
HU3D_MODELID fn_1_0(s16 minigameIndex, s16 stillIndex, Point3d *previewPosition);
void fn_1_2A4(s16 minigameCount, u8 *minigameOrder);
void fn_1_43C(void);
void fn_1_4F8(void);
void fn_1_5E0(s16 minigameIndex, s16 stillIndex);
void fn_1_6F4(void);
void fn_1_7F0(void);
void fn_1_824(void);
void fn_1_78FC(void);
void fn_1_7D08(u16 responseActive);
void fn_1_7D7C(void);
void fn_1_7D94(s16 pathIndex, s16 segmentFrames);
void fn_1_7E34(void);
void fn_1_7E5C(void);
void fn_1_8600(void);
void fn_1_8B1C(s16 advancePhaseTimers);

#endif
