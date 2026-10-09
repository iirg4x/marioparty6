// Runs the config menu and board gameplay for the TT Wars minigame.
#include "dolphin/math.h"
#include "REL/ttwarsdll/include/game/hu3d.h"
#include "REL/ttwarsdll/include/game/object.h"
#include "game/frand.h"
#include "game/pad.h"
#include "game/printfunc.h"
#include "game/memory.h"
#include "game/wipe.h"
#include "game/main.h"
#include "game/data.h"
#include "game/gamework.h"
#include "datadir_enum.h"

typedef struct TTWarsRecord_s {
    f32 tileX; // Board-space X coordinate, in model units.
    f32 tileY; // Board-space Y coordinate, in model units.
    f32 tileZ; // Board-space Z coordinate, in model units.
    s32 isSafeTile; // Nonzero for a safe tile; zero marks a hole.
    HU3D_MODELID tileModel; // Model displayed for this board tile.
} TTWarsRecord;

extern OMOBJ *lbl_1_bss_1464[256];
extern TTWarsRecord lbl_1_bss_1C[16][16];
extern s32 lbl_1_bss_4;
extern s32 lbl_1_bss_8;
extern char lbl_1_data_EE[];
extern char lbl_1_data_120[];
extern char lbl_1_data_122[];
extern char lbl_1_data_124[];
extern char lbl_1_data_126[];
s32 fn_1_49C4(void);
void fn_1_1198(s32 direction);
void fn_1_20E0(void);
extern s32 lbl_1_bss_1430;
extern u16 lbl_1_bss_1446[4];
extern HUPROCESS *lbl_1_bss_1870;
extern s32 lbl_1_bss_14;
extern s32 lbl_1_bss_10;
extern s32 lbl_1_bss_1434;
extern s32 lbl_1_bss_142C;
extern s32 lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_1864;
extern s32 lbl_1_bss_141C[4];
void fn_1_1DCC(void);
s32 fn_1_2F70(s32 column, s32 row);
s32 fn_1_2E80(s32 column, s32 row);
void fn_1_25E0(OMOBJ *obj);
void fn_1_24A8(OMOBJ *obj);
extern u32 lbl_1_bss_0;
extern u32 lbl_1_bss_18;
extern char lbl_1_data_9C[];
extern char lbl_1_data_A8[];
extern char lbl_1_data_BA[];
extern char lbl_1_data_CC[];
extern char lbl_1_data_DB[];
extern char lbl_1_data_12F[];
extern char lbl_1_data_136[];
extern char lbl_1_data_13C[];
extern char lbl_1_data_13F[];
extern char lbl_1_data_147[];
extern f32 CZoom;
extern Vec Center;
extern f32 lbl_1_bss_1450;
extern char lbl_1_data_5E[];
void fn_1_30FC(void);
void fn_1_31E8(void);
void fn_1_47F8(void);
void fn_1_7E4(OMOBJ *obj);
void fn_1_81C(OMOBJ *obj);
void fn_1_C38(OMOBJ *obj);
void fn_1_3424(BOOL showMarkers);
void fn_1_34A8(BOOL showMarkers);
void fn_1_21B8(void);
s32 fn_1_300C(void);
void fn_1_1318(void);
void fn_1_352C(void);
extern HU3D_MODELID lbl_1_bss_1438[2];
extern HU3D_MODELID lbl_1_bss_143C[4];
extern HU3D_MODELID lbl_1_bss_1444;
extern HU3D_MODELID lbl_1_bss_144E;
extern HU3D_MODELID lbl_1_bss_1454[8];
void fn_1_F88(s32 direction);
void fn_1_308C(void);
void fn_1_18A4(void);
void fn_1_344(OMOBJ *obj);
extern OMOBJ *lbl_1_bss_1868;
extern s32 lbl_1_bss_186C;
extern Point3d lbl_1_data_0;
extern s32 lbl_1_data_18;
extern OM_CAMERA_VIEW lbl_1_data_1C;
extern char lbl_1_data_38[];
extern Point3d lbl_1_data_C;
void fn_1_5BC(OMOBJ *obj);
void fn_1_3704(void);
void fn_1_D88(void);

typedef struct TTWarsObjectData_s {
    s32 playerIndex; // Player whose score increases when this piece falls through.
    s32 registrySlot; // Slot in lbl_1_bss_1464 for this piece.
    s32 state; // Piece states: waiting, falling, blocked, or leaving.
    Vec position; // Piece position in model units.
    f32 fallFrames; // Fall-step timer; reset after advancing a row, but retained when blocked.
    s32 column; // Board column selected for this piece.
    s32 row; // Logical row, starting at -3 above the board and advancing to the board height on
             // exit.
} TTWarsObjectData;

Point3d lbl_1_data_0 = {0.0f, 10000.0f, 10000.0f};
Point3d lbl_1_data_C = {0.0f, 0.0f, 0.0f};
s32 lbl_1_data_18 = -1;
OM_CAMERA_VIEW lbl_1_data_1C = {{0.0f, 150.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 1950.0f};
char lbl_1_data_38[] = "******* TTWARS ObjectSetup *********\n";
char lbl_1_data_5E[] = "*** main mode error(%d)!!\n";

HUPROCESS *lbl_1_bss_1870;

s32 lbl_1_bss_186C;

OMOBJ *lbl_1_bss_1868;

OMOBJ *lbl_1_bss_1864;

OMOBJ *lbl_1_bss_1464[256];

HU3D_MODELID lbl_1_bss_1454[8];

f32 lbl_1_bss_1450;

HU3D_MODELID lbl_1_bss_144E;

u16 lbl_1_bss_1446[4];

HU3D_MODELID lbl_1_bss_1444;

HU3D_MODELID lbl_1_bss_143C[4];

HU3D_MODELID lbl_1_bss_1438[2];

s32 lbl_1_bss_1434;

s32 lbl_1_bss_1430;

s32 lbl_1_bss_142C;

s32 lbl_1_bss_141C[4];

TTWarsRecord lbl_1_bss_1C[16][16];

u32 lbl_1_bss_18;

s32 lbl_1_bss_14;

s32 lbl_1_bss_10;

s32 lbl_1_bss_C;

s32 lbl_1_bss_8;

s32 lbl_1_bss_4;

u32 lbl_1_bss_0;

// Creates the TT Wars object manager, camera, lighting, and initial scene callbacks during overlay
// entry.
void fn_1_A0(void) {
    OSReport(lbl_1_data_38);
    lbl_1_bss_1870 = omInitObjMan(600, 8192);
    omGameSysInit(lbl_1_bss_1870);
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, (45.0f), (10.0f), (20000.0f), (1.2f));
    omCameraViewSet(&lbl_1_data_1C);
    lbl_1_bss_1868 = omAddObjEx(lbl_1_bss_1870, 32730, 0U, 0U, -1, omOutView);
    // omOutView ignores this work value; it does not affect the single-camera update.
    lbl_1_bss_1868->work[0] = 1;
    lbl_1_bss_186C =
        (s32) Hu3DGLightCreate((0.0f), (0.0f), (0.0f), (0.0f), (0.0f), (0.0f), 0U, 0U, 0U);
    Hu3DGLightPosAimSetV((s16) lbl_1_bss_186C, &lbl_1_data_0, &lbl_1_data_C);
    Hu3DGLightStaticSet((s16) lbl_1_bss_186C, 1);
    Hu3DGLightInfinitytSet((s16) lbl_1_bss_186C);
    Hu3DGLightColorSet((s16) lbl_1_bss_186C, (*(u8 *) ((s8 *) (&lbl_1_data_18) + (0))),
                       (*(u8 *) ((s8 *) (&lbl_1_data_18) + (1))),
                       (*(u8 *) ((s8 *) (&lbl_1_data_18) + (2))),
                       (*(u8 *) ((s8 *) (&lbl_1_data_18) + (3))));
    Hu3DBGColorSet(0U, 0U, 100U);
    lbl_1_bss_1450 = (0.0f);
    lbl_1_bss_1434 = 0;
    lbl_1_bss_1430 = 0;
    lbl_1_bss_142C = 0;
    lbl_1_bss_C = 0;
    lbl_1_bss_0 = 3;
    lbl_1_bss_1864 = omAddObjEx(lbl_1_bss_1870, 104, 7U, 0U, -1, fn_1_344);
    lbl_1_bss_1864->work[0] = 1000;
}

// Loads the board models and installs the frame callback after the setup object is first updated.
void fn_1_344(OMOBJ *obj) {
    s32 i;

    lbl_1_bss_8 = 8;
    lbl_1_bss_4 = 6;
    lbl_1_bss_1454[0] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_ttwars, 5), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_1454[0], HU3D_ATTR_DISPOFF);
    Hu3DModelRotSet(lbl_1_bss_1454[0], (0.0f), (0.0f), (0.0f));
    lbl_1_bss_144E = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_ttwars, 4), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_144E, HU3D_ATTR_DISPOFF);
    lbl_1_bss_1444 = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_ttwars, 6), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_1444, HU3D_ATTR_DISPOFF);
    for (i = 0; i < 256; i++) {
        *(s32 *)(void *)((u8 *)(void *)lbl_1_bss_1464 + i * 4) = 0;
    }
    omMakeGroupEx((OMOBJMAN *)lbl_1_bss_1870, 0, 256);
    // The returned group member list is ignored; pieces use lbl_1_bss_1464 instead.
    omGetGroupMemberListEx((OMOBJMAN *)lbl_1_bss_1870, 0);
    ((HU3D_MODELID *) lbl_1_bss_1446)[0] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_ttwars, 0), HU_MEMNUM_OVL, HEAP_MODEL));
    ((HU3D_MODELID *) lbl_1_bss_1446)[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_ttwars, 1), HU_MEMNUM_OVL, HEAP_MODEL));
    ((HU3D_MODELID *) lbl_1_bss_1446)[2] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_ttwars, 2), HU_MEMNUM_OVL, HEAP_MODEL));
    ((HU3D_MODELID *) lbl_1_bss_1446)[3] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_ttwars, 3), HU_MEMNUM_OVL, HEAP_MODEL));
    for (i = 0; i < 4; i++) {
        Hu3DModelAttrSet(((HU3D_MODELID *)lbl_1_bss_1446)[i], HU3D_ATTR_DISPOFF);
        // Adding HU3D_ATTR_NONE changes no flags; the source model remains hidden.
        Hu3DModelAttrSet(((HU3D_MODELID *)lbl_1_bss_1446)[i], HU3D_ATTR_NONE);
    }
    WipeCreate(1, 0, 20);
    obj->objFunc = fn_1_5BC;
}

// Advances the minigame state machine and handles exit requests once per object-manager frame.
void fn_1_5BC(OMOBJ *obj)
{
    if ((HuPadBtnDown[0] & PAD_BUTTON_X) != 0) {
        fn_1_30FC();
    }
    if (obj->work[0] > 1001) {
        fn_1_31E8();
    }
    fn_1_47F8();
    if (omSysExitReq != 0) {
        WipeCreate(2, 0, 60);
        obj->objFunc = fn_1_7E4;
    }
    // An exit request schedules the wipe callback for the next update; this frame still runs its
    // state.
    switch (obj->work[0]) {
        case 1000:
            lbl_1_bss_1450 += (1.0f);
            if ((15.0f) <= lbl_1_bss_1450) {
                obj->work[0] = 1001;
                lbl_1_bss_18 = 0;
                lbl_1_bss_10 = 4;
                lbl_1_bss_1450 = (0.0f);
            }
            break;
        case 1001:
            fn_1_81C(obj);
            break;
        case 1002:
            fn_1_C38(obj);
            break;
        case 1003:
            if ((s32)lbl_1_bss_10 != 0) {
                fn_1_3424(0);
                fn_1_34A8(0);
                obj->work[0] = 1002;
            } else {
                fn_1_3424(0);
                fn_1_34A8(1);
                fn_1_21B8();
                obj->work[0] = 1004;
            }
            break;
        case 1004:
            fn_1_3424(0);
            fn_1_34A8(1);
            if (fn_1_300C() != 0) {
                obj->work[0] = 1005;
            }
            break;
        case 1005:
            fn_1_3424(1);
            fn_1_34A8(0);
            fn_1_1318();
            break;
        case 1006:
            fn_1_3424(0);
            fn_1_34A8(0);
            if (fn_1_300C() != 0) {
                obj->work[0] = 1005;
                fn_1_352C();
            }
            break;
        case 1007:
            break;
        default:
            OSReport(lbl_1_data_5E, obj->work[0]);
            break;
    }
}

// Returns to the previous overlay once the exit wipe has completed.
void fn_1_7E4(OMOBJ *obj) {
    if (WipeCheck() == 0) {
        omOvlReturnEx(1, 1);
    }
}

// Reads configuration-menu input while the main callback is in the settings state.
void fn_1_81C(OMOBJ *obj) {
    if ((HuPadDStkRep[0] & 8) != 0) {
        lbl_1_bss_18 -= 1;
        if ((s32)lbl_1_bss_18 < 0) {
            lbl_1_bss_18 = 0;
        }
    } else if ((HuPadDStkRep[0] & 4) != 0) {
        lbl_1_bss_18 += 1;
        if ((s32)lbl_1_bss_18 > 3) {
            lbl_1_bss_18 = 3;
        }
    } else if ((HuPadDStkRep[0] & 1) != 0) {
        switch ((s32)lbl_1_bss_18) {
        case 0:
            lbl_1_bss_8 -= 1;
            if ((s32)lbl_1_bss_8 < 4) {
                lbl_1_bss_8 = 4;
            }
            if ((s32)lbl_1_bss_0 > (s32)(lbl_1_bss_8 - 1)) {
                lbl_1_bss_0 = lbl_1_bss_8 - 1;
            }
            break;
        case 1:
            lbl_1_bss_4 -= 1;
            if ((s32)lbl_1_bss_4 < 4) {
                lbl_1_bss_4 = 4;
            }
            break;
        case 2:
            lbl_1_bss_0 -= 1;
            if ((s32)lbl_1_bss_0 < 2) {
                lbl_1_bss_0 = 2;
            }
            break;
        case 3:
            lbl_1_bss_10 -= 1;
            if ((s32)lbl_1_bss_10 < 1) {
                lbl_1_bss_10 = 1;
            }
            break;
        }
    } else if ((HuPadDStkRep[0] & 2) != 0) {
        switch ((s32)lbl_1_bss_18) {
        case 0:
            lbl_1_bss_8 += 1;
            if ((s32)lbl_1_bss_8 > 16) {
                lbl_1_bss_8 = 16;
            }
            break;
        case 1:
            lbl_1_bss_4 += 1;
            if ((s32)lbl_1_bss_4 > 16) {
                lbl_1_bss_4 = 16;
            }
            break;
        case 2:
            lbl_1_bss_0 += 1;
            if ((s32)lbl_1_bss_0 > (s32)(lbl_1_bss_8 - 1)) {
                lbl_1_bss_0 = lbl_1_bss_8 - 1;
            }
            break;
        case 3:
            lbl_1_bss_10 += 1;
            if ((s32)lbl_1_bss_10 > 4) {
                lbl_1_bss_10 = 4;
            }
            break;
        }
    } else if ((HuPadBtnDown[0] & PAD_BUTTON_A) != 0) {
        obj->work[0] = 1003;
        fn_1_3704();
    }
    fn_1_D88();
}

// Selects the next piece's owner (zero chooses randomly) and starts placement after confirmation.
void fn_1_C38(OMOBJ *obj) {
    if ((HuPadDStkRep[0] & 8) != 0) {
        lbl_1_bss_1434 -= 1;
        if ((s32) lbl_1_bss_1434 < 0) {
            lbl_1_bss_1434 = 0;
            return;
        }
    } else if ((HuPadDStkRep[0] & 4) != 0) {
        lbl_1_bss_1434 += 1;
        if ((s32) lbl_1_bss_1434 > (s32) lbl_1_bss_10) {
            lbl_1_bss_1434 = lbl_1_bss_10;
            return;
        }
    } else if ((HuPadBtnDown[0] & PAD_BUTTON_A) != 0) {
        if ((s32) lbl_1_bss_1434 == 0) {
            lbl_1_bss_14 = 1;
        } else {
            lbl_1_bss_14 = 0;
        }
        fn_1_21B8();
        obj->work[0] = 1004;
    }
}

// Draws the current board dimensions, hole count, and player-count settings from the menu callback.
void fn_1_D88(void)
{
    print8(216, 120, (1.5f), lbl_1_data_9C);
    if ((s32)lbl_1_bss_18 == 0) {
        fontcolor = 14;
    } else {
        fontcolor = 15;
    }
    print8(216, 144, (1.5f), lbl_1_data_A8, lbl_1_bss_8, 4, 16);
    if ((s32)lbl_1_bss_18 == 1) {
        fontcolor = 14;
    } else {
        fontcolor = 15;
    }
    print8(216, 168, (1.5f), lbl_1_data_BA, lbl_1_bss_4, 4, 16);
    if ((s32)lbl_1_bss_18 == 2) {
        fontcolor = 14;
    } else {
        fontcolor = 15;
    }
    print8(216, 192, (1.5f), lbl_1_data_CC, lbl_1_bss_0, lbl_1_bss_8 - 1);
    if ((s32)lbl_1_bss_18 == 3) {
        fontcolor = 14;
    } else {
        fontcolor = 15;
    }
    print8(216, 216, (1.5f), lbl_1_data_DB, lbl_1_bss_10);
}

// Rotates the selected row's safe-tile/hole pattern when gameplay input requests a horizontal move.
void fn_1_F88(s32 direction)
{
    s32 index;
    TTWarsRecord saved;
    if (direction == 0) {
            fn_1_1198(0);
            saved.isSafeTile = lbl_1_bss_1C[lbl_1_bss_1430][0].isSafeTile;
            index = 1;
            while (index <= lbl_1_bss_8 - 1) {
                lbl_1_bss_1C[lbl_1_bss_1430][index - 1].isSafeTile =
                    lbl_1_bss_1C[lbl_1_bss_1430][index].isSafeTile;
                index += 1;
            }
            lbl_1_bss_1C[lbl_1_bss_1430][lbl_1_bss_8 - 1].isSafeTile = saved.isSafeTile;
            fn_1_20E0();
    } else if (direction == 1) {
            fn_1_1198(1);
            saved.isSafeTile = lbl_1_bss_1C[lbl_1_bss_1430][lbl_1_bss_8 - 1].isSafeTile;
            index = lbl_1_bss_8 - 2;
            while (index >= 0) {
                lbl_1_bss_1C[lbl_1_bss_1430][index + 1].isSafeTile =
                    lbl_1_bss_1C[lbl_1_bss_1430][index].isSafeTile;
                index -= 1;
            }
            lbl_1_bss_1C[lbl_1_bss_1430][0].isSafeTile = saved.isSafeTile;
            fn_1_20E0();
    } else {
        return;
    }
}

// Wraps registered pieces in the selected row to their new columns before rotating its tile
// pattern.
void fn_1_1198(s32 direction)
{
    s32 index;
    index = 0;
    while (index < 256) {
        if (lbl_1_bss_1464[index] != NULL) {
            TTWarsObjectData *data = lbl_1_bss_1464[index]->data;
            if (data->row == lbl_1_bss_1430) {
                if (direction == 1) {
                    data->column += 1;
                    if (data->column > lbl_1_bss_8 - 1) {
                        data->column = 0;
                    }
                } else if (direction == 0) {
                    data->column -= 1;
                    if (data->column < 0) {
                        data->column = lbl_1_bss_8 - 1;
                    }
                }
                data->position.x = (50.0f) + ((100.0f) * data->column - (50.0f) * lbl_1_bss_8);
            }
        }
        index += 1;
    }
}

// Handles controller 1's row selection, row rotation, and piece-placement requests during gameplay.
// Updates the row markers each frame.
void fn_1_1318(void)
{
    if ((HuPadDStkRep[0] & 8) != 0) {
        if (lbl_1_bss_1430-- <= 0) {
            lbl_1_bss_1430 = 0;
        }
    } else if ((HuPadDStkRep[0] & 4) != 0) {
        if (lbl_1_bss_1430++ >= lbl_1_bss_4 - 1) {
            lbl_1_bss_1430 = lbl_1_bss_4 - 1;
        }
    } else if ((HuPadDStkDown[0] & 2) != 0) {
        fn_1_F88(1);
        fn_1_308C();
        lbl_1_bss_1864->work[0] = 1006;
    } else if ((HuPadDStkDown[0] & 1) != 0) {
        fn_1_F88(0);
        fn_1_308C();
        lbl_1_bss_1864->work[0] = 1006;
    } else if ((HuPadBtnDown[0] & PAD_BUTTON_Y) != 0) {
        lbl_1_bss_1864->work[0] = 1003;
        lbl_1_bss_142C = 0;
        fn_1_1DCC();
    }
    fn_1_18A4();
}

// Positions the four markers around the currently selected board row during gameplay.
void fn_1_18A4(void)
{
    Vec position;
    position.x = (50.0f) * -lbl_1_bss_8 - (50.0f);
    position.y =
        (100.0f) * lbl_1_bss_4 - ((50.0f) + ((100.0f) * lbl_1_bss_1430 + (50.0f) * lbl_1_bss_4));
    position.z = (70.0f);
    Hu3DModelPosSet(lbl_1_bss_143C[0], position.x, position.y, position.z);
    position.x = (50.0f) + (50.0f) * lbl_1_bss_8;
    position.y =
        (100.0f) * lbl_1_bss_4 - ((50.0f) + ((100.0f) * lbl_1_bss_1430 + (50.0f) * lbl_1_bss_4));
    position.z = (70.0f);
    Hu3DModelPosSet(lbl_1_bss_143C[1], position.x, position.y, position.z);
    position.x = (0.0f);
    position.y = (75.0f) + ((100.0f) * lbl_1_bss_4 -
                            ((50.0f) + ((100.0f) * lbl_1_bss_1430 + (50.0f) * lbl_1_bss_4)));
    position.z = (70.0f);
    Hu3DModelPosSet(lbl_1_bss_143C[2], position.x, position.y, position.z);
    position.x = (0.0f);
    position.y =
        ((100.0f) * lbl_1_bss_4 - ((50.0f) + ((100.0f) * lbl_1_bss_1430 + (50.0f) * lbl_1_bss_4))) -
        (75.0f);
    position.z = (70.0f);
    Hu3DModelPosSet(lbl_1_bss_143C[3], position.x, position.y, position.z);
}

// Positions the two markers around the currently selected column during player placement.
void fn_1_1DCC(void)
{
    Vec position;
    position.x = ((50.0f) + ((100.0f) * lbl_1_bss_142C - (50.0f) * lbl_1_bss_8)) - (100.0f);
    position.y = (100.0f) * lbl_1_bss_4 - ((50.0f) + ((-300.0f) + (50.0f) * lbl_1_bss_4));
    position.z = (70.0f);
    Hu3DModelPosSet(lbl_1_bss_1438[0], position.x, position.y, position.z);
    position.x = (100.0f) + ((50.0f) + ((100.0f) * lbl_1_bss_142C - (50.0f) * lbl_1_bss_8));
    position.y = (100.0f) * lbl_1_bss_4 - ((50.0f) + ((-300.0f) + (50.0f) * lbl_1_bss_4));
    position.z = (70.0f);
    Hu3DModelPosSet(lbl_1_bss_1438[1], position.x, position.y, position.z);
}

// Updates tile visibility after a row rotation requested by the gameplay input callback.
void fn_1_20E0(void) {
    s32 column;

    column = 0;
    while (column < (s32) lbl_1_bss_8) {
        if ((s32) (*(s32 *) ((s8 *) ((void *) ((u8 *) (s32 *) ((u8 *) &lbl_1_bss_1C +
                                                               (lbl_1_bss_1430 * 320)) +
                                                               (column * 20))) +
                             (12))) != 0) {
            Hu3DModelAttrReset(
                (*(s16 *) ((s8 *) ((void *) ((u8 *) (s32 *) ((u8 *) &lbl_1_bss_1C +
                                                             (lbl_1_bss_1430 * 320)) +
                                             (column * 20))) +
                           (16))),
                1U);
        } else {
            Hu3DModelAttrSet(
                (*(s16 *) ((s8 *) ((void *) ((u8 *) (s32 *) ((u8 *) &lbl_1_bss_1C +
                                                             (lbl_1_bss_1430 * 320)) +
                                             (column * 20))) +
                           (16))),
                1U);
        }
        column += 1;
    }
}

// Creates a piece object for the chosen player at the current column selection.
void fn_1_21B8(void)
{
    TTWarsObjectData *data;
    OMOBJ *obj;
    obj = omAddObjEx(lbl_1_bss_1870, 100, 1, 0, 0, fn_1_24A8);
    obj->data = HuMemDirectMallocNum(HEAP_HEAP, 36, HU_MEMNUM_OVL);
    if (lbl_1_bss_14 != 0) {
        obj->work[0] = frandmod(lbl_1_bss_10);
    } else {
        obj->work[0] = lbl_1_bss_1434 - 1;
    }
    data = obj->data;
    data->position.x = (50.0f) + ((100.0f) * lbl_1_bss_142C - (50.0f) * lbl_1_bss_8);
    data->position.y = (100.0f) * lbl_1_bss_4 - ((50.0f) + ((-300.0f) + (50.0f) * lbl_1_bss_4));
    data->position.z = (0.0f);
    data->fallFrames = (0.0f);
    data->state = 0;
    data->playerIndex = obj->work[0];
    lbl_1_bss_C += 1;
}

// Removes a piece model and its object-manager entry when the piece reaches its leaving state.
void fn_1_23F4(OMOBJ *obj)
{

    // This saved data pointer is unused; omDelObjEx frees the piece's private data below.
    void *objectData = obj->data;
    s32 index;
    Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DModelKill(obj->mdlId[0]);
    index = 0;
    while (index < 256) {
        if (obj == lbl_1_bss_1464[index]) {
            lbl_1_bss_1464[index] = NULL;
            break;
        }
        index += 1;
    }
    omDelObjEx(lbl_1_bss_1870, obj);
}

// Links the piece model and registers the piece during its first object-manager update.
void fn_1_24A8(OMOBJ *obj)
{
    f32 *data = obj->data;
    s32 index;
    obj->mdlId[0] = Hu3DModelLink(((HU3D_MODELID *)lbl_1_bss_1446)[obj->work[0]]);
    omSetTra(obj, data[3], data[4], data[5]);
    omSetRot(obj, (0.0f), (0.0f), (0.0f));
    omSetSca(obj, (1.0f), (1.0f), (1.0f));
    for (index = 0; index < 256; index += 1) {
        if (NULL == lbl_1_bss_1464[index]) {
            lbl_1_bss_1464[index] = obj;
            ((s32 *)data)[1] = index;
            break;
        }
    }
    Hu3DModelAttrReset(obj->mdlId[0], HU3D_ATTR_DISPOFF);
    obj->objFunc = fn_1_25E0;
}

// Updates a piece's input, fall animation, score, and removal state once per object-manager frame.
void fn_1_25E0(OMOBJ *obj)
{
    TTWarsObjectData *data = obj->data;
    switch (data->state) {
        case 0:
            if (lbl_1_bss_1864->work[0] != 1004) {
                break;
            }
            if ((HuPadDStkRep[0] & 2) != 0) {
                if (lbl_1_bss_142C++ >= lbl_1_bss_8 - 1) {
                    lbl_1_bss_142C = lbl_1_bss_8 - 1;
                }
            } else if ((HuPadDStkRep[0] & 1) != 0) {
                if (lbl_1_bss_142C-- <= 0) {
                    lbl_1_bss_142C = 0;
                }
            } else if ((HuPadBtnDown[0] & PAD_BUTTON_A) != 0) {
                if (fn_1_2F70(lbl_1_bss_142C, -3) == 0) {
                    data->column = lbl_1_bss_142C;
                    data->row = -3;
                    if (fn_1_2E80(data->column, data->row) == 0) {
                        data->state = 1;
                    } else {
                        data->state = 2;
                    }
                }
            } else if ((HuPadBtnDown[0] & PAD_BUTTON_Y) != 0) {
                data->state = 3;
                lbl_1_bss_1864->work[0] = 1005;
                // Clear this frame's button presses before returning to row selection.
                HuPadBtnDown[0] = 0;
            }
            data->position.x = (50.0f) + ((100.0f) * lbl_1_bss_142C - (50.0f) * lbl_1_bss_8);
            fn_1_1DCC();
            break;
        case 1:
            if (fn_1_2E80(data->column, data->row) != 0) {
                data->state = 2;
                data->position.y = (100.0f) * lbl_1_bss_4 -
                                   ((50.0f) + ((100.0f) * data->row + (50.0f) * lbl_1_bss_4));
            } else if (data->fallFrames >= (20.0f)) {
                data->fallFrames = (0.0f);
                data->row += 1;
                if (data->row >= lbl_1_bss_4) {
                    lbl_1_bss_141C[data->playerIndex] += 1;
                    data->state = 3;
                } else {
                    data->position.y =
                        (100.0f) * lbl_1_bss_4 -
                        ((50.0f) + ((100.0f) * data->row + (50.0f) * lbl_1_bss_4));
                }
            } else {
                data->fallFrames += (1.0f);
                data->position.y -= (5.0f);
            }
            break;
        case 2:
            break;
        case 3:
            fn_1_23F4(obj);
            break;
    }
    // This transform write also runs after state 3 deletes the object and frees the saved data.
    omSetTra(obj, data->position.x, data->position.y, data->position.z);
}

// Returns whether the cell below is a safe tile or contains a blocked piece.
s32 fn_1_2E80(s32 column, s32 row)
{
    s32 index;
    index = 0;
    while (index < 256) {
        if (lbl_1_bss_1464[index] != NULL) {
            s32 *data = lbl_1_bss_1464[index]->data;
            if (data[7] == column && data[8] == row + 1 && data[2] == 2) {
                return 1;
            }
        }
        index += 1;
    }
    if (row + 1 > (s32)lbl_1_bss_4 - 1) {
        return 0;
    }
    if (row + 1 < 0) {
        return 0;
    }
    return (s32) (*(
        s32 *) ((s8 *) ((void *) ((u8 *) (s32 *) ((u8 *) &lbl_1_bss_1C + ((row + 1) * 320)) +
                                  (column * 20))) +
                (12)));
}

// Returns whether a blocked piece occupies the cell below the supplied column and row.
s32 fn_1_2F70(s32 column, s32 row)
{
    s32 index;
    index = 0;
    while (index < 256) {
        if (lbl_1_bss_1464[index] != NULL) {
            s32 *data = lbl_1_bss_1464[index]->data;
            if (data[7] == column && data[8] == row + 1 && data[2] == 2) {
                return 1;
            }
        }
        index += 1;
    }
    return 0;
}

// Returns true when no pieces are registered or every registered piece is blocked.
s32 fn_1_300C(void)
{
    s32 index;
    index = 0;
    while (index < 256) {
        if (lbl_1_bss_1464[index] != NULL) {
            s32 *data = lbl_1_bss_1464[index]->data;
            if (data[2] != 2) {
                return 0;
            }
        }
        index += 1;
    }
    return 1;
}

// Puts every registered piece into the falling state before the next turn begins.
void fn_1_308C(void)
{
    s32 index;
    index = 0;
    while (index < 256) {
        if (lbl_1_bss_1464[index] != NULL) {
            s32 *data = lbl_1_bss_1464[index]->data;
            data[2] = 1;
        }
        index += 1;
    }
}

// Prints the board's safe cells and current hole count when the debug button is pressed.
void fn_1_30FC(void)
{
    s32 row;
    s32 column;
    OSReport(lbl_1_data_EE);
    row = 0;
    while (row < lbl_1_bss_4) {
        column = 0;
        while (column < lbl_1_bss_8) {
            if (*(s32 *)((u8 *)&lbl_1_bss_1C + row * 320 + column * 20 + 12) != 0) {
                OSReport(lbl_1_data_120);
            } else {
                OSReport(lbl_1_data_122);
            }
            column += 1;
        }
        OSReport(lbl_1_data_124);
        row += 1;
    }
    OSReport(lbl_1_data_126, fn_1_49C4());
}

// Draws the score panel while play has advanced beyond the initial board setup state.
void fn_1_31E8(void)
{
    s32 colors[4] = {10, 12, 9, 14};
    GXColor color;
    s32 index;
    color.r = 100;
    color.g = 100;
    color.b = 100;
    color.a = 192;
    printWin(492, 60, 108, 132, &color);
    fontcolor = 15;
    print8(516, 72, (1.5f), lbl_1_data_12F);
    index = 0;
    while (index < (s32)lbl_1_bss_10) {
        fontcolor = colors[index];
        // Every score row prints zero as its player label, while the score comes from that row's
        // player.
        print8(516, 72 + (index + 1) * 24, (1.5f), lbl_1_data_136, 0, lbl_1_bss_141C[index]);
        index += 1;
    }
    if (lbl_1_bss_1864->work[0] == 1002) {
        fontcolor = 15;
        print8(492, 72 + lbl_1_bss_1434 * 24, (1.5f), lbl_1_data_13C);
    }
    printWin(492, 204, 108, 36, &color);
    fontcolor = 15;
    print8(504, 216, (1.5f), lbl_1_data_13F, lbl_1_bss_C);
    printWin(492, 252, 96, 36, &color);
    fontcolor = 15;
    print8(504, 264, (1.5f), lbl_1_data_147);
}

// The main callback shows or hides the four row-selection markers for its current state.
void fn_1_3424(BOOL showMarkers)
{
    s32 i = 0;
    while (i < 4) {
        if (showMarkers) {
            Hu3DModelAttrReset(((HU3D_MODELID *)lbl_1_bss_143C)[i], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(((HU3D_MODELID *)lbl_1_bss_143C)[i], HU3D_ATTR_DISPOFF);
        }
        i++;
    }
}

// The main callback shows or hides the two column-selection markers for its current state.
void fn_1_34A8(BOOL showMarkers)
{
    s32 i = 0;
    while (i < 2) {
        if (showMarkers) {
            Hu3DModelAttrReset(((HU3D_MODELID *)lbl_1_bss_1438)[i], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(((HU3D_MODELID *)lbl_1_bss_1438)[i], HU3D_ATTR_DISPOFF);
        }
        i++;
    }
}

// After a row rotation settles, makes the first all-hole column safe, then opens one random hole
// in each row. Leaves the board unchanged if no all-hole column exists.
void fn_1_352C(void)
{
    s32 column;
    s32 row;
    s32 iteration;
    s32 slot;
    s32 complete;
    complete = 0;
    column = 0;
    while (column < lbl_1_bss_8) {
        row = 0;
        while (row < lbl_1_bss_4) {
            if (*(s32 *)((u8 *)&lbl_1_bss_1C + row * 320 + column * 20 + 12) != 0) {
                break;
            }
            row += 1;
        }
        if (row == lbl_1_bss_4) {
            complete = 1;
            row = 0;
            while (row < lbl_1_bss_4) {
                *(s32 *)((u8 *)&lbl_1_bss_1C + row * 320 + column * 20 + 12) = 1;
                Hu3DModelAttrReset(
                    *(s16 *) ((u8 *) &lbl_1_bss_1C + row * 320 + column * 20 + 16), 1);
                row += 1;
            }
        } else {
            complete = 0;
        }
        if (complete != 0) {
            break;
        }
        column += 1;
    }
    if (complete != 0) {
        row = 0;
        while (row < lbl_1_bss_4) {
            iteration = 0;
            while (iteration < 1) {
                do {
                    slot = frandmod(lbl_1_bss_8);
                } while (*(s32 *)((u8 *)&lbl_1_bss_1C + row * 320 + slot * 20 + 12) == 0);
                *(s32 *)((u8 *)&lbl_1_bss_1C + row * 320 + slot * 20 + 12) = 0;
                Hu3DModelAttrSet(*(s16 *) ((u8 *) &lbl_1_bss_1C + row * 320 + slot * 20 + 16),
                                 1);
                iteration += 1;
            }
            row += 1;
        }
    }
}

// Builds the configured board, randomly marks its holes, and adds a board-size offset to camera
// zoom.
void fn_1_3704(void)
{
    s32 row;
    s32 column;
    s32 iteration;
    Vec position;
    row = 0;
    while (row < lbl_1_bss_4) {
        column = 0;
        while (column < lbl_1_bss_8) {
            position.x = (50.0f) + ((100.0f) * column - (50.0f) * lbl_1_bss_8);
            position.y =
                (100.0f) * lbl_1_bss_4 - ((50.0f) + ((100.0f) * row + (50.0f) * lbl_1_bss_4));
            position.z = (0.0f);
            lbl_1_bss_1C[row][column].tileModel = Hu3DModelLink(lbl_1_bss_144E);
            Hu3DModelAttrSet(lbl_1_bss_1C[row][column].tileModel, HU3D_ATTR_DISPOFF);
            Hu3DModelPosSet(lbl_1_bss_1C[row][column].tileModel, position.x, position.y,
                            position.z);
            lbl_1_bss_1C[row][column].tileX = position.x;
            lbl_1_bss_1C[row][column].tileY = position.y;
            lbl_1_bss_1C[row][column].tileZ = position.z;
            Hu3DModelAttrReset(lbl_1_bss_1C[row][column].tileModel, HU3D_ATTR_DISPOFF);
            lbl_1_bss_1C[row][column].isSafeTile = 1;
            column += 1;
        }
        row += 1;
    }
    row = 0;
    while (row < lbl_1_bss_4) {
        s32 slot;
        s32 count = lbl_1_bss_0;
        iteration = 0;
        while (iteration < count) {
            do {
                slot = frandmod(lbl_1_bss_8);
            } while (lbl_1_bss_1C[row][slot].isSafeTile == 0);
            lbl_1_bss_1C[row][slot].isSafeTile = 0;
            Hu3DModelAttrSet(lbl_1_bss_1C[row][slot].tileModel, HU3D_ATTR_DISPOFF);
            iteration += 1;
        }
        row += 1;
    }
    lbl_1_bss_143C[0] = Hu3DModelLink(lbl_1_bss_1444);
    Hu3DModelScaleSet(lbl_1_bss_143C[0], (0.5f), (0.5f), (0.5f));
    Hu3DModelRotSet(lbl_1_bss_143C[0], (0.0f), (0.0f), (-90.0f));
    lbl_1_bss_143C[1] = Hu3DModelLink(lbl_1_bss_1444);
    Hu3DModelScaleSet(lbl_1_bss_143C[1], (0.5f), (0.5f), (0.5f));
    Hu3DModelRotSet(lbl_1_bss_143C[1], (0.0f), (0.0f), (90.0f));
    lbl_1_bss_143C[2] = Hu3DModelLink(lbl_1_bss_1444);
    Hu3DModelScaleSet(lbl_1_bss_143C[2], (0.5f), (0.5f), (0.5f));
    Hu3DModelRotSet(lbl_1_bss_143C[2], (0.0f), (0.0f), (180.0f));
    lbl_1_bss_143C[3] = Hu3DModelLink(lbl_1_bss_1444);
    Hu3DModelScaleSet(lbl_1_bss_143C[3], (0.5f), (0.5f), (0.5f));
    Hu3DModelRotSet(lbl_1_bss_143C[3], (0.0f), (0.0f), (0.0f));
    fn_1_18A4();
    lbl_1_bss_1438[0] = Hu3DModelLink(lbl_1_bss_1444);
    Hu3DModelScaleSet(lbl_1_bss_1438[0], (0.5f), (0.5f), (0.5f));
    Hu3DModelRotSet(lbl_1_bss_1438[0], (0.0f), (0.0f), (-90.0f));
    lbl_1_bss_1438[1] = Hu3DModelLink(lbl_1_bss_1444);
    Hu3DModelScaleSet(lbl_1_bss_1438[1], (0.5f), (0.5f), (0.5f));
    Hu3DModelRotSet(lbl_1_bss_1438[1], (0.0f), (0.0f), (90.0f));
    fn_1_1DCC();
    fn_1_3424(0);
    fn_1_34A8(0);
    Hu3DModelAttrReset(lbl_1_bss_1454[0], HU3D_ATTR_DISPOFF);
    Hu3DModelScaleSet(lbl_1_bss_1454[0], lbl_1_bss_8, lbl_1_bss_4, (1.0f));
    Hu3DModelPosSet(lbl_1_bss_1454[0], (0.0f), lbl_1_bss_4 / 2.0f, (0.0f));
    if (lbl_1_bss_8 > lbl_1_bss_4) {
        iteration = lbl_1_bss_8;
    } else if (lbl_1_bss_8 < lbl_1_bss_4) {
        iteration = lbl_1_bss_4 + 2;
    } else {
        iteration = lbl_1_bss_8;
    }
    CZoom += (100.0f) * (iteration - 8);
}

// Applies the second controller's zoom and camera-center input on each main-state frame.
void fn_1_47F8(void)
{

    // This color is initialized but never used to draw a panel.
    GXColor unusedColor;
    unusedColor.r = 0;
    unusedColor.g = 0;
    unusedColor.b = 144;
    unusedColor.a = 192;
    CZoom += HuPadTrigL[1] / 5;
    CZoom -= HuPadTrigR[1] / 5;
    Center.x -= (10.0f) * (HuPadStkX[1] / (44.0f));
    Center.y -= (10.0f) * (HuPadStkY[1] / (44.0f));
}

// Counts the board cells marked as holes for the debug display.
s32 fn_1_49C4(void)
{
    s32 count;
    s32 column;
    s32 row;
    count = 0;
    column = 0;
    while (column < lbl_1_bss_8) {
        row = 0;
        while (row < lbl_1_bss_4) {
            if (*(s32 *)((u8 *)&lbl_1_bss_1C + row * 320 + column * 20 + 12) == 0) {
                count += 1;
            }
            row += 1;
        }
        column += 1;
    }
    return count;
}

char lbl_1_data_9C[] = "CONFIG MENU";
char lbl_1_data_A8[] = "X SIZE:%d (%d-%d)";
char lbl_1_data_BA[] = "Y SIZE:%d (%d-%d)";
char lbl_1_data_CC[] = "HOLE:%d (2-%d)";
char lbl_1_data_DB[] = "PLAYER(S):%d (1-4)";
char lbl_1_data_EE[] = "------------------------------------------------\n";
char lbl_1_data_120[] = "#";
char lbl_1_data_122[] = " ";
char lbl_1_data_124[] = "\n";
char lbl_1_data_126[] = "HOLE:%d\n";
char lbl_1_data_12F[] = "RAMDOM";
char lbl_1_data_136[] = "%d:%d";
char lbl_1_data_13C[] = ">>";
char lbl_1_data_13F[] = "GAME:%d";
char lbl_1_data_147[] = "Y:SKIP";
