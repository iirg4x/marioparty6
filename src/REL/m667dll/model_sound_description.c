/* Talkie Walkie model and motion descriptions, selected for the current mode. */
#include <math.h>
#include "game/main.h"
#include "game/hu3d.h"
#include "game/hsfex.h"
#include "REL/m667dll/recovered.h"
#include "humath.h"
#include "game/gamework.h"
#include "dolphin/types.h"

/* Scene setup requests these descriptors while creating camera, map, entry, and player models. */
M667ModelDescription *fn_1_8F58(s32 descriptionIndex)
{
    if (fn_1_8FC4() == 0) return lbl_1_data_AB8[descriptionIndex];
    return lbl_1_data_AD4[descriptionIndex];
}

/* Return zero. */
s32 fn_1_8FBC(void) { return 0; }

/* Report whether the current minigame uses its night-mode descriptions. */
s16 fn_1_8FC4(void) { return lbl_802C0278; }
/* Night and daytime model and motion descriptor records selected by fn_1_8F58. */

/* Camera-motion resources shared by the day and night scenes. */
M667ModelDescription lbl_1_data_558[5] = {
    { 0x007E0036, 3, 0, 0, -1 },
    { 0x007E0037, 3, 0, 0, -1 },
    { 0x007E0038, 3, 0, 0, -1 },
    { 0x007E0039, 3, 0, 0, -1 },
    { 0, 8, 0, -1, -1 }
};
/* Collision-model resource shared by both scene modes. */
M667ModelDescription lbl_1_data_5A8[2] = {
    { 0x007E0000, 2, 0, 0, -1 },
    { 0, 8, 0, -1, -1 }
};
/* Daytime map models and their attached joint motions. */
M667ModelDescription lbl_1_data_5C8[10] = {
    { 0x007E0001, 0, 0, 0, -1 },
    { 0x007E0004, 0, 0, 0, -1 },
    { 0x007E0007, 0, 0, 0, -1 },
    { 0x007E0002, 0, 0, 0, -1 },
    { 0x007E0005, 0, 0, 0, -1 },
    { 0x007E0009, 0, 0, 2, -1 },
    { 0x007E0003, 1, 0, 0, 3 },
    { 0x007E0006, 1, 0, 0, 4 },
    { 0x007E000A, 1, 0, 0, 5 },
    { 0, 8, 0, -1, -1 }
};
/* Nighttime map models and their attached joint motions. */
M667ModelDescription lbl_1_data_668[6] = {
    { 0x007E000B, 0, 0, 0, -1 },
    { 0x007E0004, 0, 0, 0, -1 },
    { 0x007E000D, 0, 0, 0, -1 },
    { 0x007E000C, 0, 0, 0, -1 },
    { 0x007E0003, 1, 0, 0, 3 },
    { 0, 8, 0, -1, -1 }
};
/* Daytime ending resources used when one player is recorded for the result. */
M667ModelDescription lbl_1_data_6C8[11] = {
    { 0x007E0025, 0, 0, 0, -1 },
    { 0x007E0026, 1, 0, 0, 0 },
    { 0x007E0008, 0, 0, 0, -1 },
    { 0x007E0021, 0, 0, 0, -1 },
    { 0x007E0022, 0, 0, 0, -1 },
    { 0x007E0023, 0, 0, 0, -1 },
    { 0x007E0024, 0, 0, 0, -1 },
    { 0x007E0004, 0, 0, 0, -1 },
    { 0x007E0005, 0, 0, 0, -1 },
    { 0x007E0006, 1, 0, 0, 7 },
    { 0, 8, 0, -1, -1 }
};
/* Nighttime ending resources used when one player is recorded for the result. */
M667ModelDescription lbl_1_data_778[9] = {
    { 0x007E002B, 0, 0, 0, -1 },
    { 0x007E0026, 1, 0, 0, 0 },
    { 0x007E000E, 0, 0, 0, -1 },
    { 0x007E0027, 0, 0, 0, -1 },
    { 0x007E0028, 0, 0, 0, -1 },
    { 0x007E0029, 0, 0, 0, -1 },
    { 0x007E002A, 0, 0, 0, -1 },
    { 0x007E0004, 0, 0, 0, -1 },
    { 0, 8, 0, -1, -1 }
};
/* Daytime ending resources used when three players are recorded for the result. */
M667ModelDescription lbl_1_data_808[6] = {
    { 0x007E002C, 0, 0, 0, -1 },
    { 0x007E0007, 0, 0, 0, -1 },
    { 0x007E0004, 0, 0, 0, -1 },
    { 0x007E0005, 0, 0, 0, -1 },
    { 0x007E0006, 1, 0, 0, 3 },
    { 0, 8, 0, -1, -1 }
};
/* Nighttime ending resources used when three players are recorded for the result. */
M667ModelDescription lbl_1_data_868[4] = {
    { 0x007E002D, 0, 0, 0, -1 },
    { 0x007E000D, 0, 0, 0, -1 },
    { 0x007E0004, 0, 0, 0, -1 },
    { 0, 8, 0, -1, -1 }
};
/* Daytime player models and one joint motion attached to each player model. */
M667ModelDescription lbl_1_data_8A8[9] = {
    { 0x007E000F, 0, 0, 0, -1 },
    { 0x007E0011, 0, 0, 0, -1 },
    { 0x007E0013, 0, 0, 0, -1 },
    { 0x007E0015, 0, 0, 0, -1 },
    { 0x007E0010, 1, 0, 0, 0 },
    { 0x007E0012, 1, 0, 0, 1 },
    { 0x007E0014, 1, 0, 0, 2 },
    { 0x007E0016, 1, 0, 0, 3 },
    { 0, 8, 0, -1, -1 }
};
/* Nighttime player models and one joint motion attached to each player model. */
M667ModelDescription lbl_1_data_938[9] = {
    { 0x007E0017, 0, 0, 0, -1 },
    { 0x007E0018, 0, 0, 0, -1 },
    { 0x007E0019, 0, 0, 0, -1 },
    { 0x007E001A, 0, 0, 0, -1 },
    { 0x007E0010, 1, 0, 0, 0 },
    { 0x007E0012, 1, 0, 0, 1 },
    { 0x007E0014, 1, 0, 0, 2 },
    { 0x007E0016, 1, 0, 0, 3 },
    { 0, 8, 0, -1, -1 }
};
/* Entry actor models, linked model copies, and their attached joint motions. */
M667ModelDescription lbl_1_data_9C8[15] = {
    { 0x007E001B, 0, 0, 3, -1 },
    { 0x007E001C, 0, 0, 3, -1 },
    { 0x007E001D, 0, 0, 3, -1 },
    { 0x007E001E, 0, 0, 3, -1 },
    { 0x007E002E, 2, 0, 0, -1 },
    { 0x007E0030, 2, 0, 0, -1 },
    { 0x007E0032, 2, 0, 0, -1 },
    { 0x007E0034, 2, 0, 0, -1 },
    { 0x007E001F, 2, 0, 4, -1 },
    { 0x007E002F, 1, 0, 0, 12 },
    { 0x007E0031, 1, 0, 0, 13 },
    { 0x007E0033, 1, 0, 0, 14 },
    { 0x007E0035, 1, 0, 0, 15 },
    { 0x007E0020, 1, 0, 0, 16 },
    { 0, 8, 0, -1, -1 }
};
/* Camera, collision, player, entry, map, single-player result, and team-result descriptors. */
M667ModelDescription *lbl_1_data_AB8[7] = {
    lbl_1_data_558, lbl_1_data_5A8, lbl_1_data_8A8, lbl_1_data_9C8, lbl_1_data_5C8,
    lbl_1_data_6C8, lbl_1_data_808
};
/* The same descriptor roles with nighttime player, map, and result resources. */
M667ModelDescription *lbl_1_data_AD4[7] = {
    lbl_1_data_558, lbl_1_data_5A8, lbl_1_data_938, lbl_1_data_9C8, lbl_1_data_668,
    lbl_1_data_778, lbl_1_data_868
};
