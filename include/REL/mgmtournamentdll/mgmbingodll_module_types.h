/* Additional window geometry and scene declarations included by Mount Duel. */
#ifndef REL_MGMTOURNAMENTDLL_MGMBINGODLL_MODULE_TYPES_H
#define REL_MGMTOURNAMENTDLL_MGMBINGODLL_MODULE_TYPES_H

#include <math.h>
#include "dolphin.h"
#include "game/main.h"
#include "game/object.h"
#include "game/data.h"
#include "game/mgdata.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/process.h"
#include "game/memory.h"
#include "game/gamework.h"
#include "datadir_enum.h"
#include "string.h"

/* Additional position-vector and position-table declarations available to the scene. */

/* Preview cleanup and additional window/scene routine declarations. */

void fn_1_4F8(void);

HU3D_MODELID fn_1_0(s16 gameNo, s16 timeOfDay, Point3d *position);
void fn_1_2A4(s16 gameCount, u8 *gameOrder);
void fn_1_43C(void);
void fn_1_5E0(s16 gameNo, s16 timeOfDay);
void fn_1_6F4(void);
void fn_1_7F0(void);
void fn_1_824(void);

#endif /* REL_MGMTOURNAMENTDLL_MGMBINGODLL_MODULE_TYPES_H */
