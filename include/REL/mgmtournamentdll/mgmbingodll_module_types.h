/* Additional window geometry and scene declarations included by Mount Duel. */
#ifndef REL_MGMTOURNAMENTDLL_MGMBINGODLL_MODULE_TYPES_H
#define REL_MGMTOURNAMENTDLL_MGMBINGODLL_MODULE_TYPES_H

#include "dolphin/math.h"
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

/* Holds the frame and message windows with their screen-space geometry in pixels. */
typedef struct MgmbingoWindowWork {
    HUWINID frameWindow; /* Window ID for the decorative frame. */
    f32 frameWidth; /* Frame width in pixels. */
    f32 frameHeight; /* Frame height in pixels. */
    f32 frameX; /* Frame left edge in screen pixels. */
    f32 frameY; /* Frame top edge in screen pixels. */
    HUWINID textWindow; /* Window ID for the text displayed over the frame. */
    f32 textWidth; /* Text window width in pixels. */
    f32 textHeight; /* Text window height in pixels. */
    f32 textX; /* Text window left edge in screen pixels. */
    f32 textY; /* Text window top edge in screen pixels. */
} MgmbingoWindowWork;

extern MgmbingoWindowWork lbl_1_bss_C8;
extern OMOBJ *lbl_1_bss_A8;
extern OMOBJ *lbl_1_bss_A30;
extern OMOBJ *lbl_1_bss_ACC[4];

/* Additional position-vector and position-table declarations available to the scene. */
extern Point3d lbl_1_bss_A3C[4];
extern const Point3d lbl_1_data_5A4[5][5];

extern HU3D_MODELID lbl_1_bss_39C;
extern HUSPR_GROUPID lbl_1_bss_A2;
extern ANIMDATA *lbl_1_bss_98;
extern MGDATA *lbl_1_bss_90;
extern s16 lbl_1_bss_A0;
extern s16 lbl_1_bss_AE0;
extern s16 lbl_1_bss_AF8;

extern const f32 lbl_1_rodata_94;
extern const f32 lbl_1_rodata_98;
extern const f32 lbl_1_rodata_9C;
extern const f32 lbl_1_rodata_A0;
extern const f32 lbl_1_rodata_A4;
extern const f32 lbl_1_rodata_A8;
extern const f32 lbl_1_rodata_AC;
extern const f32 lbl_1_rodata_B0;
extern const f32 lbl_1_rodata_B4;
extern const f32 lbl_1_rodata_148;

/* Preview cleanup and additional window/scene routine declarations. */
void fn_1_3A60(HUWINID winId, u32 mess, s16 c);
void fn_1_4F8(void);
void fn_1_109E4(void);

#endif /* REL_MGMTOURNAMENTDLL_MGMBINGODLL_MODULE_TYPES_H */
