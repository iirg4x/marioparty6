/* Talkie Walkie sequence and player action declarations. */
#include "dolphin/types.h"
#include <math.h>
#include "REL/m667dll/recovered.h"
#include "dolphin/gx.h"
#include "dolphin/gx/GXVert.h"
#include "dolphin/mtx.h"
#include "dolphin/types.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/flag.h"
#include "game/frand.h"
#include "game/gamemes.h"
#include "game/gamework.h"
#include "game/hsfex.h"
#include "game/hu3d.h"
#include "game/main.h"
#include "game/memory.h"
#include "game/mg/actman.h"
#include "game/mg/seqman.h"
#include "game/mic.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "humath.h"
#include "string.h"

extern f32 lbl_1_data_AC[14];
extern HuVecF lbl_1_data_230[4][2];
extern f32 lbl_1_data_E4[14];
