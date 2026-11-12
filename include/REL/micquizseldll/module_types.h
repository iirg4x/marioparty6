/* Shared work, response, choice, and resource-table types for the Mic Quiz menu. */
#ifndef REL_MICQUIZSELDLL_MODULE_TYPES_H
#define REL_MICQUIZSELDLL_MODULE_TYPES_H

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
#include "game/window.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"

typedef void (*VoidFunc)(void);
typedef struct QuizMessageIds { s32 values[20]; } QuizMessageIds;
extern u16 lbl_1_data_388[];
extern s32 lbl_1_data_398[];
extern char * lbl_1_data_424[];
extern char *lbl_1_data_45C[];
extern char *lbl_1_data_47C[];
extern char *lbl_1_data_494[];
extern char *lbl_1_data_4B4[];
extern char *lbl_1_data_4D8[];
extern OMOBJ * lbl_1_bss_1C;
extern HUPROCESS * lbl_1_bss_20[11];

#endif
