/* Builds and animates the tournament bracket models and player sprites. */
#include <math.h>
#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/hsfex.h"
#include "game/audio.h"
#include "game/sprite.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/gamework.h"
#include "game/mgdata.h"
#include "datadir_enum.h"
#include "REL/mgmtournamentdll/module_types.h"

#define MSM_SE_MGTOUR_CHAMPION_REVEAL 1251
#define MSM_SE_MGTOUR_MARKER_HIGHLIGHT 1253

typedef struct PlayerSpriteIDs {
    HUSPRID badge; /* Place-badge sprite in member 0; -1 before creation. */
    HUSPRID portrait; /* Character portrait in member 1; -1 before creation. */
} PlayerSpriteIDs;

HU3D_MODELID lbl_1_bss_E0[7]; /* Marker models for the seven bracket positions. */
OMOBJ *lbl_1_bss_DC; /* Bracket scene object; work[0] selects a pending side animation. */
s16 lbl_1_bss_D8; /* Markers enabled since the bracket update; capped at 100. */
extern OMOBJMAN *lbl_1_bss_12C;
void fn_1_31B8(void);
void fn_1_4424(OMOBJ *object);
void fn_1_4850(void);
s32 lbl_1_data_278[8] = {
    DATANUM(0xa10000, 0), DATANUM(0xa10000, 1),
    DATANUM(0xa10000, 2), DATANUM(0xa10000, 3),
    DATANUM(0xa10000, 4), DATANUM(0xa10000, 5),
    DATANUM(0xa10000, 6), DATANUM(0xa10000, 8)
};
s32 lbl_1_data_298 = DATANUM(0xa10000, 7);
Vec lbl_1_data_29C[7] = { { -600.0f, -110.0f, 190.0f }, { -200.0f, -110.0f, 190.0f },
                          { 200.0f, -110.0f, 190.0f },  { 600.0f, -110.0f, 190.0f },
                          { -300.0f, 360.0f, -210.0f }, { 300.0f, 360.0f, -210.0f },
                          { 0.0f, 810.0f, -680.0f } };
f32 lbl_1_data_2F0[7] = { 0.6f, 0.6f, 0.6f, 0.6f, 0.8f, 0.8f, 1.05f };
Vec lbl_1_data_30C = { 0.0f, 1000.0f, -1050.0f };
Vec lbl_1_data_318[2] = { { -200.0f, 0.0f, 0.0f }, { 200.0f, 0.0f, 0.0f } };
Vec lbl_1_data_330[2] = { { -200.0f, 0.0f, 0.0f }, { 200.0f, 0.0f, 0.0f } };
/* Called during tournament scene setup to create the bracket, player markers, and result
 * sprites. */
void fn_1_2E28(void)
{
    s32 modelIndex;

    lbl_1_bss_DC = fn_8003AB38(lbl_1_bss_12C, 4096, 8, 0, -1, fn_1_4424);
    lbl_1_bss_DC->work[0] = lbl_1_bss_DC->work[1] =
        lbl_1_bss_DC->work[2] = lbl_1_bss_DC->work[3] = 0;
    lbl_1_bss_DC->stat |= (1 << 8);
    modelIndex = 0;
    while (modelIndex < 8) {
        lbl_1_bss_DC->mdlId[modelIndex] = fn_80025A04(
            fn_80008DD0(lbl_1_data_278[modelIndex], 0x10000000, 2));
        fn_80026B64(lbl_1_bss_DC->mdlId[modelIndex], 1);
        fn_80026B8C(lbl_1_bss_DC->mdlId[modelIndex], 2);
        fn_800269E8(lbl_1_bss_DC->mdlId[modelIndex],
                         ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        modelIndex++;
    }
    fn_80027080(lbl_1_bss_DC->mdlId[0], "f-grid248");
    fn_80027080(lbl_1_bss_DC->mdlId[0], "f-grid214");
    fn_8002D86C(lbl_1_bss_DC->mdlId[1], 0.5f);
    fn_800269E8(lbl_1_bss_DC->mdlId[2], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                 ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
    fn_8002D414(lbl_1_bss_DC->mdlId[2], 0.0f);
    fn_800269E8(lbl_1_bss_DC->mdlId[3], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                 ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
    fn_8002D414(lbl_1_bss_DC->mdlId[3], 0.0f);
    fn_800269E8(lbl_1_bss_DC->mdlId[4], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                 ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
    fn_8002D414(lbl_1_bss_DC->mdlId[4], 0.0f);
    fn_800269E8(lbl_1_bss_DC->mdlId[6], (1 << 0));
    fn_800269E8(lbl_1_bss_DC->mdlId[6], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                 ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
    fn_8002D414(lbl_1_bss_DC->mdlId[6], 0.0f);
    fn_800269E8(lbl_1_bss_DC->mdlId[7], (1 << 0));
    fn_800269E8(lbl_1_bss_DC->mdlId[7], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                 ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
    fn_8002D414(lbl_1_bss_DC->mdlId[7], 0.0f);
    fn_80026790(lbl_1_bss_DC->mdlId[5], &lbl_1_data_30C);
    fn_1_4850();
    fn_1_31B8();
}

/* Tournament setup creates and places the seven animated markers on the bracket. */
void fn_1_31B8(void) {
    s16 index;

    index = 0;
    while (index < 7) {
        lbl_1_bss_E0[index] =
            fn_80025A04(fn_80008DD0(lbl_1_data_298, 0x10000000, 2));
        fn_80026B64(lbl_1_bss_E0[index], 1U);
        fn_80026B8C(lbl_1_bss_E0[index], 2);
        fn_80026790(lbl_1_bss_E0[index], &lbl_1_data_29C[index]);
        fn_800268B0(lbl_1_bss_E0[index], lbl_1_data_2F0[index], lbl_1_data_2F0[index],
                          lbl_1_data_2F0[index]);
        fn_800269E8(lbl_1_bss_E0[index], ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        fn_800269E8(lbl_1_bss_E0[index], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                  ((1 << 30) | (1 << 7)) |
                                                  ((1 << 30) | (1 << 10)));
        fn_8002D414(lbl_1_bss_E0[index], 0.0f);
        fn_8002D86C(lbl_1_bss_E0[index], 2.0f);
        index += 1;
    }
}

/* Selection and result handlers restart a bracket marker and enable or pause its animation. */
void fn_1_3394(s16 modelIndex, s16 enabled) {
    if (enabled != 0) {
        fn_80026A44(lbl_1_bss_E0[modelIndex],
            ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                         ((1 << 30) | (1 << 7)) |
                                                         ((1 << 30) | (1 << 10)));
    } else {
        fn_800269E8(lbl_1_bss_E0[modelIndex], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                       ((1 << 30) | (1 << 7)) |
                                                       ((1 << 30) | (1 << 10)));
    }
    fn_8002D414(lbl_1_bss_E0[modelIndex], 0.0f);
    if (enabled != 0) {
        /* Play one cue even when several markers are enabled before the next bracket update. */
        if (lbl_1_bss_D8 == 0) {
            HuAudFXPlay(MSM_SE_MGTOUR_MARKER_HIGHLIGHT);
        }
        lbl_1_bss_D8 += 1;
        if (lbl_1_bss_D8 > 100) {
            lbl_1_bss_D8 = 100;
        }
    }
}

/* Called by the result handler to hide the bracket piece on the losing side. */
void fn_1_34B0(s16 modelIndex) {
    fn_800269E8(lbl_1_bss_E0[modelIndex], (1 << 0));
}

/* Called from the tournament scene loop to move the bracket highlight to the current matchup. */
void fn_1_34F0(void)
{
    s16 mode;

    mode = tournamentLayoutGet();
    if (mode == 0) {
        fn_1_3394(0, 1);
        fn_1_3394(1, 1);
        fn_1_3394(2, 0);
        fn_1_3394(3, 0);
        fn_1_3394(4, 0);
        fn_1_3394(5, 0);
        fn_1_3394(6, 0);
        return;
    }
    if (mode == 1) {
        fn_1_3394(0, 0);
        fn_1_3394(1, 0);
        fn_1_3394(2, 1);
        fn_1_3394(3, 1);
        fn_1_3394(4, 0);
        fn_1_3394(5, 0);
        fn_1_3394(6, 0);
        return;
    }
    if (mode == 2) {
        if (MgModeWorkInt[12] == 0) {
            return;
        }
        /* Suppress the marker cue for this consolation highlight update. */
        lbl_1_bss_D8 = 1;
        fn_1_3394(0, 0);
        fn_1_3394(1, 1);
        fn_1_3394(2, 1);
        fn_1_3394(3, 0);
        fn_1_3394(4, 0);
        fn_1_3394(5, 0);
        fn_1_3394(6, 0);
        return;
    }
        fn_1_3394(0, 0);
        fn_1_3394(1, 0);
        fn_1_3394(2, 0);
        fn_1_3394(3, 0);
        fn_1_3394(4, 1);
        fn_1_3394(5, 1);
        fn_1_3394(6, 0);
}

/* Scene setup enables these bracket surfaces to receive character shadows. */
void fn_1_3E04(void) {
    fn_80027080(*lbl_1_bss_DC->mdlId, "f-bmerge14");
    fn_80027080(*lbl_1_bss_DC->mdlId, "f-bmerge38");
    fn_80027080(*lbl_1_bss_DC->mdlId, "f-bmerge22");
    fn_80027080(*lbl_1_bss_DC->mdlId, "f-bmerge35");
    fn_80027080(*lbl_1_bss_DC->mdlId, "f-bmerge24");
    fn_80027080(lbl_1_bss_DC->mdlId[3], "f-bmerge36");
    fn_80027080(lbl_1_bss_DC->mdlId[4], "f-bmerge37");
}

/* Called by the tournament scene to retrieve the central bracket model ID. */
s16 fn_1_3F00(void) {
    return lbl_1_bss_DC->mdlId[5];
}

/* The round result handler schedules the first side animation and saves that side for return. */
void fn_1_3F18(s16 side) {
    if ((u32) lbl_1_bss_DC->work[0] == 0U) {
        if (side == 0) {
            lbl_1_bss_DC->work[0] = 1U;
            MgModeWorkInt[13] = 1;
            return;
        }
        lbl_1_bss_DC->work[0] = 2U;
        MgModeWorkInt[13] = 2;
    }
}

/* Setup after a minigame restores the saved side result with its animations at their final
 * frame. */
void fn_1_3F8C(void) {
    if ((s32) MgModeWorkInt[13] == 1) {
        lbl_1_bss_DC->work[0] = -1;
        fn_80026A44(lbl_1_bss_DC->mdlId[3], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                       ((1 << 30) | (1 << 7)) |
                                                       ((1 << 30) | (1 << 10)));
        fn_800269E8(lbl_1_bss_DC->mdlId[3], ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        fn_8002D414(lbl_1_bss_DC->mdlId[3], fn_8002D530(lbl_1_bss_DC->mdlId[3]));
        fn_80026A44(lbl_1_bss_DC->mdlId[6], (1 << 0));
        fn_80026A44(lbl_1_bss_DC->mdlId[6], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                       ((1 << 30) | (1 << 7)) |
                                                       ((1 << 30) | (1 << 10)));
        fn_800269E8(lbl_1_bss_DC->mdlId[6], ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        fn_8002D414(lbl_1_bss_DC->mdlId[6], fn_8002D530(lbl_1_bss_DC->mdlId[6]));
        fn_80026790(lbl_1_bss_DC->mdlId[6], &lbl_1_data_318[0]);
        fn_80026A44(lbl_1_bss_DC->mdlId[7], (1 << 0));
        fn_80026A44(lbl_1_bss_DC->mdlId[7], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                       ((1 << 30) | (1 << 7)) |
                                                       ((1 << 30) | (1 << 10)));
        fn_800269E8(lbl_1_bss_DC->mdlId[7], ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        fn_8002D414(lbl_1_bss_DC->mdlId[7], fn_8002D530(lbl_1_bss_DC->mdlId[7]));
        fn_80026790(lbl_1_bss_DC->mdlId[7], &lbl_1_data_330[0]);
        fn_800269E8(lbl_1_bss_E0[1], (1 << 0));
        return;
    }
    if ((s32) MgModeWorkInt[13] == 2) {
        lbl_1_bss_DC->work[0] = -1;
        fn_80026A44(lbl_1_bss_DC->mdlId[4], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                       ((1 << 30) | (1 << 7)) |
                                                       ((1 << 30) | (1 << 10)));
        fn_800269E8(lbl_1_bss_DC->mdlId[4], ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        fn_8002D414(lbl_1_bss_DC->mdlId[4], fn_8002D530(lbl_1_bss_DC->mdlId[4]));
        fn_80026A44(lbl_1_bss_DC->mdlId[6], (1 << 0));
        fn_80026A44(lbl_1_bss_DC->mdlId[6], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                       ((1 << 30) | (1 << 7)) |
                                                       ((1 << 30) | (1 << 10)));
        fn_800269E8(lbl_1_bss_DC->mdlId[6], ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        fn_8002D414(lbl_1_bss_DC->mdlId[6], fn_8002D530(lbl_1_bss_DC->mdlId[6]));
        fn_80026790(lbl_1_bss_DC->mdlId[6], &lbl_1_data_318[1]);
        fn_80026A44(lbl_1_bss_DC->mdlId[7], (1 << 0));
        fn_80026A44(lbl_1_bss_DC->mdlId[7], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                       ((1 << 30) | (1 << 7)) |
                                                       ((1 << 30) | (1 << 10)));
        fn_800269E8(lbl_1_bss_DC->mdlId[7], ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        fn_8002D414(lbl_1_bss_DC->mdlId[7], fn_8002D530(lbl_1_bss_DC->mdlId[7]));
        fn_80026790(lbl_1_bss_DC->mdlId[7], &lbl_1_data_330[1]);
        fn_800269E8(lbl_1_bss_E0[2], (1 << 0));
    }
}

/* The champion presentation starts the center bracket animation and plays its reveal cue. */
void fn_1_43BC(void) {
    fn_80026A44(lbl_1_bss_DC->mdlId[2], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                   ((1 << 30) | (1 << 7)) |
                                                   ((1 << 30) | (1 << 10)));
    fn_8002D414(lbl_1_bss_DC->mdlId[2], 0.0f);
    HuAudFXPlay(MSM_SE_MGTOUR_CHAMPION_REVEAL);
}

/* Each object update starts pending side results and holds their completed animations.
 * The center champion animation is paused and rewound to frame zero when it ends. */
void fn_1_4424(OMOBJ *object)
{
    f32 maxTime;

    if (object->work[0] != 0) {
      if (object->work[0] == 1) {
        object->work[0] = -1;
        fn_80026A44(object->mdlId[3], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                 ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        fn_800269E8(object->mdlId[3], ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        fn_8002D414(object->mdlId[3], 0.0f);
        fn_80026A44(object->mdlId[6], (1 << 0));
        fn_80026A44(object->mdlId[6], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                 ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        fn_800269E8(object->mdlId[6], ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        fn_8002D414(object->mdlId[6], 0.0f);
        fn_80026790(object->mdlId[6], &lbl_1_data_318[0]);
        fn_80026A44(object->mdlId[7], (1 << 0));
        fn_80026A44(object->mdlId[7], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                 ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        fn_800269E8(object->mdlId[7], ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        fn_8002D414(object->mdlId[7], 0.0f);
        fn_80026790(object->mdlId[7], &lbl_1_data_330[0]);
      } else if (object->work[0] == 2) {
        object->work[0] = -1;
        fn_80026A44(object->mdlId[4], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                 ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        fn_800269E8(object->mdlId[4], ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        fn_8002D414(object->mdlId[4], 0.0f);
        fn_80026A44(object->mdlId[6], (1 << 0));
        fn_80026A44(object->mdlId[6], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                 ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        fn_800269E8(object->mdlId[6], ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        fn_8002D414(object->mdlId[6], 0.0f);
        fn_80026790(object->mdlId[6], &lbl_1_data_318[1]);
        fn_80026A44(object->mdlId[7], (1 << 0));
        fn_80026A44(object->mdlId[7], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                 ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        fn_800269E8(object->mdlId[7], ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        fn_8002D414(object->mdlId[7], 0.0f);
        fn_80026790(object->mdlId[7], &lbl_1_data_330[1]);
      }
        maxTime = fn_8002D530(object->mdlId[3]);
        if ((1.0f + fn_8002D4E0(object->mdlId[3])) >= maxTime) {
            fn_800269E8(object->mdlId[3], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                   ((1 << 30) | (1 << 7)) |
                                                   ((1 << 30) | (1 << 10)));
        }
        maxTime = fn_8002D530(object->mdlId[4]);
        if ((1.0f + fn_8002D4E0(object->mdlId[4])) >= maxTime) {
            fn_800269E8(object->mdlId[4], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                   ((1 << 30) | (1 << 7)) |
                                                   ((1 << 30) | (1 << 10)));
        }
        maxTime = fn_8002D530(object->mdlId[6]);
        if ((1.0f + fn_8002D4E0(object->mdlId[6])) >= maxTime) {
            fn_800269E8(object->mdlId[6], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                   ((1 << 30) | (1 << 7)) |
                                                   ((1 << 30) | (1 << 10)));
        }
        maxTime = fn_8002D530(object->mdlId[7]);
        if ((1.0f + fn_8002D4E0(object->mdlId[7])) >= maxTime) {
            fn_800269E8(object->mdlId[7], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                                   ((1 << 30) | (1 << 7)) |
                                                   ((1 << 30) | (1 << 10)));
        }
    }
    maxTime = fn_8002D530(object->mdlId[2]);
    if ((1.0f + fn_8002D4E0(object->mdlId[2])) >= maxTime) {
        fn_800269E8(object->mdlId[2], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                               ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        fn_8002D414(object->mdlId[2], 0.0f);
    }
    /* Let the next marker-selection pass play its first highlight cue. */
    lbl_1_bss_D8 = 0;
}

s32 lbl_1_data_3AC[4] = {
    DATANUM(0xa10000, 10), DATANUM(0xa10000, 12),
    DATANUM(0xa10000, 14), DATANUM(0xa10000, 15)
};
s32 lbl_1_data_3BC[2] = { DATANUM(0xa10000, 11), DATANUM(0xa10000, 13) };
s32 lbl_1_data_3C4[14] = {
    DATANUM(0xa10000, 16), DATANUM(0xa10000, 17),
    DATANUM(0xa10000, 18), DATANUM(0xa10000, 19),
    DATANUM(0xa10000, 20), DATANUM(0xa10000, 21),
    DATANUM(0xa10000, 22), DATANUM(0xa10000, 23),
    DATANUM(0xa10000, 24), DATANUM(0xa10000, 25),
    DATANUM(0xa10000, 26), DATANUM(0xa10000, 25),
    DATANUM(0xa10000, 25), DATANUM(0xa10000, 25)
};
ANIMDATA *lbl_1_data_3FC[10] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
HUSPR_GROUPID lbl_1_data_424[4] = { -1, -1, -1, -1 };
PlayerSpriteIDs lbl_1_data_42C[4] = { {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1} };

/* Bracket setup loads place-badge art and creates four sprite groups at zero scale. */
void fn_1_4850(void) {
    s16 index;

    index = 0;
    while (index < 4) {
        lbl_1_data_3FC[index] =
            HuSprAnimRead(fn_80008DD0(lbl_1_data_3AC[index], 0x10000000, 2));
        lbl_1_data_424[index] = HuSprGrpCreate(2);
        lbl_1_data_42C[index].badge = HuSprCreate(lbl_1_data_3FC[index], 8, 0);
        HuSprGrpMemberSet(lbl_1_data_424[index], 0, lbl_1_data_42C[index].badge);
        HuSprPosSet(lbl_1_data_424[index], 0, 0.0f, 0.0f);
        HuSprGrpScaleSet(lbl_1_data_424[index], 0.0f, 0.0f);
        index += 1;
    }
    lbl_1_data_3FC[8] =
        HuSprAnimRead(fn_80008DD0(lbl_1_data_3BC[0], 0x10000000, 2));
    lbl_1_data_3FC[9] =
        HuSprAnimRead(fn_80008DD0(lbl_1_data_3BC[1], 0x10000000, 2));
}

/* Tournament exit releases the four place-badge groups and their member sprites. */
void fn_1_4A1C(void) {
    s16 index;

    index = 0;
    while (index < 4) {
        if (lbl_1_data_424[index] >= 0) {
            HuSprGrpKill(lbl_1_data_424[index]);
        }
        index += 1;
    }
}

/* The champion presentation places a player's portrait and place badge beside its screen
 * position. */
void fn_1_4A94(s16 playerNo, s16 characterNo, s16 groupNo, Point3d worldPosition)
{
    Point3d projectedPosition;

    /* The first two places use different badge art according to the starting bracket side. */
    if ((MgModeWorkInt[playerNo] == 0 || MgModeWorkInt[playerNo] == 1) &&
        MgModeWorkInt[playerNo + 4] == 0) {
        HuSprGrpMemberKill(lbl_1_data_424[groupNo], 0);
        lbl_1_data_42C[groupNo].badge = HuSprCreate(lbl_1_data_3FC[groupNo + 8], 8, 0);
        HuSprGrpMemberSet(lbl_1_data_424[groupNo], 0, lbl_1_data_42C[groupNo].badge);
        HuSprPosSet(lbl_1_data_424[groupNo], 0, 0.0f, 0.0f);
    }
    if ((MgModeWorkInt[playerNo] == 2 || MgModeWorkInt[playerNo] == 3) &&
        MgModeWorkInt[playerNo + 4] == 1) {
        HuSprGrpMemberKill(lbl_1_data_424[groupNo], 0);
        lbl_1_data_42C[groupNo].badge = HuSprCreate(lbl_1_data_3FC[groupNo + 8], 8, 0);
        HuSprGrpMemberSet(lbl_1_data_424[groupNo], 0, lbl_1_data_42C[groupNo].badge);
        HuSprPosSet(lbl_1_data_424[groupNo], 0, 0.0f, 0.0f);
    }
    lbl_1_data_3FC[groupNo + 4] = HuSprAnimRead(
        fn_80008DD0(lbl_1_data_3C4[characterNo], 0x10000000, 2));
    lbl_1_data_42C[groupNo].portrait = HuSprCreate(lbl_1_data_3FC[groupNo + 4], 4, 0);
    HuSprGrpMemberSet(lbl_1_data_424[groupNo], 1, lbl_1_data_42C[groupNo].portrait);
    if (MgModeWorkInt[playerNo + 4] == 0) {
        if (MgModeWorkInt[playerNo] == 0 || MgModeWorkInt[playerNo] == 1) {
            HuSprPosSet(lbl_1_data_424[groupNo], 1, 8.0f, -18.0f);
        } else if (MgModeWorkInt[playerNo] == 2 || MgModeWorkInt[playerNo] == 3) {
            HuSprPosSet(lbl_1_data_424[groupNo], 1, 20.0f, -16.0f);
        }
    } else if (MgModeWorkInt[playerNo + 4] == 1) {
        if (MgModeWorkInt[playerNo] == 0 || MgModeWorkInt[playerNo] == 1) {
            HuSprPosSet(lbl_1_data_424[groupNo], 1, 14.0f, -14.0f);
        } else if (MgModeWorkInt[playerNo] == 2 || MgModeWorkInt[playerNo] == 3) {
            HuSprPosSet(lbl_1_data_424[groupNo], 1, 18.0f, -18.0f);
        }
    } else if (MgModeWorkInt[playerNo + 4] == 2) {
        HuSprPosSet(lbl_1_data_424[groupNo], 1, 24.0f, -13.0f);
    } else if (MgModeWorkInt[playerNo + 4] == 3) {
        HuSprPosSet(lbl_1_data_424[groupNo], 1, 16.0f, -16.0f);
    }
    Hu3D3Dto2D(&worldPosition, 1, &projectedPosition);
    HuSprGrpScaleSet(lbl_1_data_424[groupNo], 1.0f, 1.0f);
    HuSprGrpPosSet(lbl_1_data_424[groupNo], projectedPosition.x, projectedPosition.y);
}
