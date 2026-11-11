#ifndef REL_MICQUIZISHIDLL_MODULE_BASE_H
#define REL_MICQUIZISHIDLL_MODULE_BASE_H
#include "math.h"
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
#include "REL/micquizishidll/shared_types.h"

void fn_1_1E364(void);
void fn_1_2558(void);
void fn_1_2670(void);
s32 fn_1_2A18(Vec *position0, f32 radius0, Vec *position1, f32 radius1);
f32 fn_1_2AD8(f32 start, f32 end, f32 amount);
void fn_1_1FB04(void);
void fn_1_20B70(void);
void fn_1_21230(void);
void fn_1_21764(OMOBJ *object);
void fn_1_21B14(OMOBJ *object);
void fn_1_21E00(void);
void fn_1_22058(void);
void fn_1_229E0(void);
void fn_1_239D4(OMOBJ *object);
void fn_1_24FA4(void);
void fn_1_25A90(void);
void fn_1_25DF4(void);
void fn_1_211C4(u8 firstFlag, u8 secondFlag);


#endif
