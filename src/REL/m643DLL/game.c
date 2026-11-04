/* Jump the Gun's scene, player, camera, and projectile callbacks. */
#include <math.h>
#include "datadir_enum.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "REL/m643DLL/m643.h"
#include "game/mg/actman.h"
#include "game/mg/seqman.h"
#include "game/wipe.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/gamework.h"
#include "game/charman.h"
#include "game/audio.h"
#include "msm_stream.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/flag.h"
#include "game/pad.h"
#include "game/gamemes.h"



extern M643ViewControlWork *lbl_1_bss_154[2];
extern M643PlayerPrefix *lbl_1_bss_174[2];
extern s16 lbl_1_bss_148;
extern s32 lbl_1_bss_144;
extern const s32 lbl_1_rodata_60[2];
extern HuVecF lbl_1_data_B8, lbl_1_data_AC;
void fn_1_2E38(OMOBJ *object);
void fn_1_2EF4(OMOBJ *object);
void fn_1_2F5C(OMOBJ *object);
void fn_1_2FBC(OMOBJ *object);
void fn_1_3300(OMOBJ *object);
void fn_1_3374(OMOBJ *object);
void fn_1_3680(OMOBJ *object);
void fn_1_36DC(OMOBJ *object);
void fn_1_3A98(OMOBJ *object);

extern M643TeamScenePrefix *lbl_1_bss_16C[2];
extern M643PlatformWork *lbl_1_bss_14C[2];

void fn_1_40FC(OMOBJ *object);

extern const s16 lbl_1_rodata_178[4];
extern const s32 lbl_1_rodata_60[2];
extern HuVecF lbl_1_data_B8;
extern HuVecF lbl_1_data_C4[4][39];
extern s32 lbl_1_bss_8;
extern s16 lbl_1_bss_148;
void fn_1_64B4(OMOBJ *object);
void fn_1_6674(OMOBJ *object);
void fn_1_6750(OMOBJ *object);
void fn_1_696C(OMOBJ *object);
void fn_1_6B98(OMOBJ *object);
void fn_1_58DC(OMOBJ *object);
void fn_1_5438(OMOBJ *object);

extern s32 lbl_1_bss_C[2][39];

#define M643_AIM_LINE_COLOR_BIT_PATTERN 0xDEFB
#define M643_PLATFORM_LINE_COLOR_BIT_PATTERN 0xD800
#define M643_SHOT_PATH_COLOR_BIT_PATTERN 0x07E0
extern const u32 lbl_1_rodata_110[2];
extern unsigned int lbl_1_data_60[10];
void fn_1_4D8C(OMOBJ *object);
void fn_1_4EF8(OMOBJ *object);
void fn_1_5298(OMOBJ *object);
void fn_1_B130(OMOBJ *object);

BOOL fn_1_BCB0(M643PlayerPrefix *work);
void fn_1_5D94(OMOBJ *object);

typedef struct M643TeamTemplate_s {
    s32 group[4]; /* Player index assigned to each configured group; -1 means empty. */
} M643TeamTemplate;
extern const M643TeamTemplate lbl_1_rodata_180;
extern u32 lbl_1_data_50[2][2];
void fn_1_2A98(OMOBJ *object);

void fn_1_90A8(OMOBJ *object);
void fn_1_96A4(OMOBJ *object);
void fn_1_9A48(OMOBJ *object);
void fn_1_9AF4(OMOBJ *object);
void fn_1_9C74(OMOBJ *object);

extern const HuVecF lbl_1_rodata_190, lbl_1_rodata_19C, lbl_1_rodata_1A8, lbl_1_rodata_1B4;
extern const HuVecF lbl_1_rodata_1C0, lbl_1_rodata_1CC, lbl_1_rodata_1D8;
void fn_1_23C4(OMOBJ *object);
void fn_1_83C8(OMOBJ *object);
void fn_1_1EB0(OMOBJ *object);

extern u32 lbl_1_data_28[2][5];
extern OMOBJMAN *lbl_1_bss_0;
void fn_1_26F4(OMOBJ *object);

extern u32 lbl_1_data_88[3], lbl_1_data_94[2], lbl_1_data_9C[2], lbl_1_data_A4[2];
extern const u32 lbl_1_rodata_108[2];
void fn_1_A138(OMOBJ *object);
void fn_1_8C70(OMOBJ *object);

s32 fn_1_C5B0(M643TeamScenePrefix *work);

extern const HuVecF lbl_1_rodata_410, lbl_1_rodata_41C;
void fn_1_A444(OMOBJ *object);
void fn_1_A6F0(OMOBJ *object);

extern const GXVtxDescList lbl_1_rodata_378[3];
extern const GXVtxAttrFmtList lbl_1_rodata_390[3];
extern const HuVecF lbl_1_rodata_3C0;

extern const HuVecF lbl_1_rodata_42C;

extern const s32 lbl_1_rodata_138[4];

void fn_1_2058(OMOBJ *object);

extern M643PlayerPrefix *lbl_1_bss_174[2];
extern M643PlayerIdentityPrefix *lbl_1_bss_15C[4];
extern s16 lbl_1_bss_148;
extern s32 lbl_1_bss_144;
void fn_1_2168(OMOBJ *object);
void fn_1_21AC(OMOBJ *object);
void fn_1_2300(OMOBJ *object);
void fn_1_2354(OMOBJ *object);

extern const s32 lbl_1_rodata_60[2];
void fn_1_B21C(OMOBJ *object);
void fn_1_B408(OMOBJ *object);

extern s32 lbl_1_bss_8;
extern HuVecF lbl_1_data_C4[4][39];
void fn_1_B7F0(OMOBJ *object);
void fn_1_B95C(OMOBJ *object);
void fn_1_BC10(OMOBJ *object);

M643PlayerPrefix *fn_1_1640(s16 teamIndex);
M643TeamScenePrefix *fn_1_165C(s16 teamIndex);
void *fn_1_1678(s16 teamIndex);
M643ViewControlWork *fn_1_1694(s16 teamIndex);
void fn_1_16B0(s32 out[4]);
void fn_1_17A0(OMOBJ *object);
void fn_1_1EB0(OMOBJ *object);
void fn_1_1F10(OMOBJ *object);
void fn_1_1F98(OMOBJ *object);
void fn_1_2058(OMOBJ *object);
void fn_1_2168(OMOBJ *object);
void fn_1_21AC(OMOBJ *object);
void fn_1_2300(OMOBJ *object);
void fn_1_2354(OMOBJ *object);
void fn_1_23C4(OMOBJ *object);
void fn_1_26F4(OMOBJ *object);
s32 fn_1_2708(M643PlatformWork *work, s16 platform);
void fn_1_2838(OMOBJ *object);
void fn_1_2A98(OMOBJ *object);
void fn_1_2D90(OMOBJ *object);
void fn_1_2E38(OMOBJ *object);
void fn_1_2EF4(OMOBJ *object);
void fn_1_2F5C(OMOBJ *object);
void fn_1_2FBC(OMOBJ *object);
void fn_1_3300(OMOBJ *object);
void fn_1_3374(OMOBJ *object);
void fn_1_3680(OMOBJ *object);
void fn_1_36DC(OMOBJ *object);
void fn_1_3A98(OMOBJ *object);
void fn_1_40FC(OMOBJ *object);
void fn_1_44B0(OMOBJ *object);
s32 fn_1_44C4(M643PlayerPrefix *work, f32 radius);
BOOL fn_1_4664(M643PlayerPrefix *work);
void fn_1_4880(M643PlayerPrefix *work);
BOOL fn_1_49B8(M643PlayerPrefix *work);
void fn_1_4AEC(OMOBJ *object);
void fn_1_4D8C(OMOBJ *object);
void fn_1_4EF8(OMOBJ *object);
void fn_1_5298(OMOBJ *object);
void fn_1_5438(OMOBJ *object);
void fn_1_58DC(OMOBJ *object);
void fn_1_5D94(OMOBJ *object);
void fn_1_6450(OMOBJ *object);
void fn_1_64B4(OMOBJ *object);
void fn_1_6674(OMOBJ *object);
void fn_1_6750(OMOBJ *object);
void fn_1_696C(OMOBJ *object);
void fn_1_6B98(OMOBJ *object);
void fn_1_6CA0(M643TeamScenePrefix *work, s16 *minPlatform, s16 *maxPlatform);
s32 fn_1_6DE0(M643TeamScenePrefix *work, f32 radius);
void fn_1_6FA8(M643TeamScenePrefix *work, s16 motion, f32 blend, u32 flags);
void fn_1_7028(M643TeamScenePrefix *work);
void fn_1_72E0(M643TeamScenePrefix *work);
void fn_1_7554(M643TeamScenePrefix *work);
void fn_1_7D1C(HU3D_MODEL *model, Mtx mtx);
void fn_1_83C8(OMOBJ *object);
void fn_1_8C70(OMOBJ *object);
void fn_1_8CD4(OMOBJ *object);
void fn_1_8F94(OMOBJ *object);
void fn_1_90A8(OMOBJ *object);
void fn_1_96A4(OMOBJ *object);
void fn_1_98E4(OMOBJ *object);
void fn_1_9A48(OMOBJ *object);
void fn_1_9AF4(OMOBJ *object);
void fn_1_9C74(OMOBJ *object);
void fn_1_9D7C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_9F60(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_A138(OMOBJ *object);
void fn_1_A444(OMOBJ *object);
void fn_1_A6F0(OMOBJ *object);
void fn_1_AFF8(OMOBJ *object);
void fn_1_B130(OMOBJ *object);
void fn_1_B21C(OMOBJ *object);
void fn_1_B408(OMOBJ *object);
void fn_1_B7F0(OMOBJ *object);
void fn_1_B95C(OMOBJ *object);
void fn_1_BC10(OMOBJ *object);
BOOL fn_1_BCB0(M643PlayerPrefix *work);
s32 fn_1_C5B0(M643TeamScenePrefix *work);

/* Camera layouts, computer difficulty settings, and scene positions. */
const s32 lbl_1_rodata_60[2] = {1, 2};
const f32 lbl_1_rodata_68[2][2][4] = {
    0.0f, 0.0f, 640.0f, 480.0f, 0.0f, -20.0f, 640.0f, 280.0f,
    0.0f, 440.0f, 640.0f, 80.0f, 0.0f, 170.0f, 640.0f, 310.0f
};
const f32 lbl_1_rodata_A8[2][2][4] = {
    0.0f, 0.0f, 640.0f, 478.0f, 0.0f, 0.0f, 640.0f, 238.0f,
    0.0f, 482.0f, 640.0f, 238.0f, 0.0f, 242.0f, 640.0f, 238.0f
};
const f32 lbl_1_rodata_E8[2][2] = {
    1.3333334f, 2.6666667f, 2.6666667f, 2.6666667f
};
const f32 lbl_1_rodata_F8[2][2] = {900.0f, 530.0f, 530.0f, 570.0f};
const u32 lbl_1_rodata_108[2] = {DATANUM(0x660000, 41), DATANUM(0x660000, 42)};
const u32 lbl_1_rodata_110[2] = {2, 4};
const f32 lbl_1_rodata_118[4] = {20.0f, 50.0f, 70.0f, 85.0f};
const f32 lbl_1_rodata_128[4] = {40.0f, 60.0f, 75.0f, 90.0f};
const s32 lbl_1_rodata_138[4] = {60, 50, 40, 30};
const f32 lbl_1_rodata_148[4] = {100.0f, 50.0f, 30.0f, 0.0f};
const f32 lbl_1_rodata_158[4] = {0.0f, 10.0f, 20.0f, 40.0f};
const f32 lbl_1_rodata_168[4] = {1.0f, 0.5f, 0.2f, 0.0f};
const s16 lbl_1_rodata_178[4] = {60, 40, 2, 2};
const M643TeamTemplate lbl_1_rodata_180 = {-1, -1, -1, -1};
const HuVecF lbl_1_rodata_190 = {-2000.0f, -500.0f, -3000.0f};
const HuVecF lbl_1_rodata_19C = {1.0f, 3000.0f, 1.0f};
const HuVecF lbl_1_rodata_1A8 = {0.0f, 1.0f, 0.0f};
const HuVecF lbl_1_rodata_1B4 = {0.0f, 0.0f, 0.0f};
const HuVecF lbl_1_rodata_1C0 = {1.0f, 1000.0f, -500.0f};
const HuVecF lbl_1_rodata_1CC = {0.0f, 1000.0f, 0.0f};
const HuVecF lbl_1_rodata_1D8 = {0.0f, 0.0f, 0.0f};

/* Environment models selected by the day or night setting during map setup. */
u32 lbl_1_data_28[2][5] = {
    {DATANUM(0x660000, 13), DATANUM(0x660000, 0), DATANUM(0x660000, 2),
        DATANUM(0x660000, 21), DATANUM(0x660000, 4)},
    {DATANUM(0x660000, 14), DATANUM(0x660000, 1), DATANUM(0x660000, 3),
        DATANUM(0x660000, 22), DATANUM(0x660000, 4)}
};
/* Additional platform models selected by time of day and team. */
u32 lbl_1_data_50[2][2] = {
    {DATANUM(0x660000, 17), DATANUM(0x660000, 18)},
    {DATANUM(0x660000, 19), DATANUM(0x660000, 20)}
};
/* Shared runner/operator motions. Runner creation replaces entry 4 with motion file 117;
 * operators also load the final zero entry as character motion file 0. */
unsigned int lbl_1_data_60[10] = {
    DATANUM(0x930000, 0), DATANUM(0x930000, 1), DATANUM(0x930000, 2),
    DATANUM(0x930000, 3), DATANUM(0x930000, 4), DATANUM(0x930000, 6),
    DATANUM(0x930000, 40), DATANUM(0x930000, 63), DATANUM(0x930000, 5), 0
};
/* Cannon idle, reload, and firing motions. */
u32 lbl_1_data_88[3] = {DATANUM(0x660000, 32), DATANUM(0x660000, 33), DATANUM(0x660000, 34)};
/* Team-specific aiming marker models and their associated motions. */
u32 lbl_1_data_94[2] = {DATANUM(0x660000, 26), DATANUM(0x660000, 27)};
u32 lbl_1_data_9C[2] = {DATANUM(0x660000, 28), DATANUM(0x660000, 29)};
/* Looping models beneath the cannon operator, selected by time of day. */
u32 lbl_1_data_A4[2] = {DATANUM(0x660000, 9), DATANUM(0x660000, 10)};
HuVecF lbl_1_data_AC = {-2060.0f, 200.9037f, 0.0f};
HuVecF lbl_1_data_B8 = {2160.0f, 200.9037f, 100.0f};
/* Four selectable courses, each with 39 platform positions in world units. */
HuVecF lbl_1_data_C4[4][39] = {
    {
        {-1900.0f, 300.0f, -28.0f},
        {-1800.0f, 400.0f, -28.0f},
        {-1700.0f, 500.0f, -28.0f},
        {-1600.0f, 600.0f, -28.0f},
        {-1500.0f, 500.0f, -28.0f},
        {-1400.0f, 400.0f, -28.0f},
        {-1300.0f, 300.0f, -28.0f},
        {-1200.0f, 400.0f, -28.0f},
        {-1100.0f, 500.0f, -28.0f},
        {-1000.0f, 400.0f, -28.0f},
        {-900.0f, 300.0f, -28.0f},
        {-800.0f, 400.0f, -28.0f},
        {-700.0f, 300.0f, -28.0f},
        {-600.0f, 200.0f, -28.0f},
        {-500.0f, 300.0f, -28.0f},
        {-400.0f, 200.0f, -28.0f},
        {-300.0f, 300.0f, -28.0f},
        {-200.0f, 200.0f, -28.0f},
        {-100.0f, 300.0f, -28.0f},
        {0.0f, 400.0f, -28.0f},
        {100.0f, 500.0f, -28.0f},
        {200.0f, 600.0f, -28.0f},
        {300.0f, 500.0f, -28.0f},
        {400.0f, 400.0f, -28.0f},
        {500.0f, 500.0f, -28.0f},
        {600.0f, 400.0f, -28.0f},
        {700.0f, 500.0f, -28.0f},
        {800.0f, 400.0f, -28.0f},
        {900.0f, 300.0f, -28.0f},
        {1000.0f, 400.0f, -28.0f},
        {1100.0f, 300.0f, -28.0f},
        {1200.0f, 400.0f, -28.0f},
        {1300.0f, 300.0f, -28.0f},
        {1400.0f, 200.0f, -28.0f},
        {1500.0f, 300.0f, -28.0f},
        {1600.0f, 400.0f, -28.0f},
        {1700.0f, 500.0f, -28.0f},
        {1800.0f, 400.0f, -28.0f},
        {1900.0f, 300.0f, -28.0f}
    },
    {
        {-1900.0f, 300.0f, -28.0f},
        {-1800.0f, 400.0f, -28.0f},
        {-1700.0f, 500.0f, -28.0f},
        {-1600.0f, 400.0f, -28.0f},
        {-1500.0f, 300.0f, -28.0f},
        {-1400.0f, 200.0f, -28.0f},
        {-1300.0f, 300.0f, -28.0f},
        {-1200.0f, 400.0f, -28.0f},
        {-1100.0f, 300.0f, -28.0f},
        {-1000.0f, 200.0f, -28.0f},
        {-900.0f, 300.0f, -28.0f},
        {-800.0f, 400.0f, -28.0f},
        {-700.0f, 500.0f, -28.0f},
        {-600.0f, 400.0f, -28.0f},
        {-500.0f, 300.0f, -28.0f},
        {-400.0f, 400.0f, -28.0f},
        {-300.0f, 300.0f, -28.0f},
        {-200.0f, 200.0f, -28.0f},
        {-100.0f, 300.0f, -28.0f},
        {0.0f, 400.0f, -28.0f},
        {100.0f, 300.0f, -28.0f},
        {200.0f, 400.0f, -28.0f},
        {300.0f, 500.0f, -28.0f},
        {400.0f, 600.0f, -28.0f},
        {500.0f, 500.0f, -28.0f},
        {600.0f, 400.0f, -28.0f},
        {700.0f, 500.0f, -28.0f},
        {800.0f, 400.0f, -28.0f},
        {900.0f, 300.0f, -28.0f},
        {1000.0f, 200.0f, -28.0f},
        {1100.0f, 300.0f, -28.0f},
        {1200.0f, 400.0f, -28.0f},
        {1300.0f, 500.0f, -28.0f},
        {1400.0f, 600.0f, -28.0f},
        {1500.0f, 500.0f, -28.0f},
        {1600.0f, 400.0f, -28.0f},
        {1700.0f, 500.0f, -28.0f},
        {1800.0f, 400.0f, -28.0f},
        {1900.0f, 300.0f, -28.0f}
    },
    {
        {-1900.0f, 300.0f, -28.0f},
        {-1800.0f, 400.0f, -28.0f},
        {-1700.0f, 300.0f, -28.0f},
        {-1600.0f, 200.0f, -28.0f},
        {-1500.0f, 300.0f, -28.0f},
        {-1400.0f, 400.0f, -28.0f},
        {-1300.0f, 300.0f, -28.0f},
        {-1200.0f, 400.0f, -28.0f},
        {-1100.0f, 300.0f, -28.0f},
        {-1000.0f, 200.0f, -28.0f},
        {-900.0f, 300.0f, -28.0f},
        {-800.0f, 400.0f, -28.0f},
        {-700.0f, 500.0f, -28.0f},
        {-600.0f, 600.0f, -28.0f},
        {-500.0f, 500.0f, -28.0f},
        {-400.0f, 400.0f, -28.0f},
        {-300.0f, 300.0f, -28.0f},
        {-200.0f, 200.0f, -28.0f},
        {-100.0f, 300.0f, -28.0f},
        {0.0f, 400.0f, -28.0f},
        {100.0f, 300.0f, -28.0f},
        {200.0f, 400.0f, -28.0f},
        {300.0f, 500.0f, -28.0f},
        {400.0f, 400.0f, -28.0f},
        {500.0f, 500.0f, -28.0f},
        {600.0f, 600.0f, -28.0f},
        {700.0f, 500.0f, -28.0f},
        {800.0f, 400.0f, -28.0f},
        {900.0f, 300.0f, -28.0f},
        {1000.0f, 200.0f, -28.0f},
        {1100.0f, 300.0f, -28.0f},
        {1200.0f, 400.0f, -28.0f},
        {1300.0f, 500.0f, -28.0f},
        {1400.0f, 400.0f, -28.0f},
        {1500.0f, 500.0f, -28.0f},
        {1600.0f, 600.0f, -28.0f},
        {1700.0f, 500.0f, -28.0f},
        {1800.0f, 400.0f, -28.0f},
        {1900.0f, 300.0f, -28.0f}
    },
    {
        {-1900.0f, 300.0f, -28.0f},
        {-1800.0f, 400.0f, -28.0f},
        {-1700.0f, 300.0f, -28.0f},
        {-1600.0f, 200.0f, -28.0f},
        {-1500.0f, 300.0f, -28.0f},
        {-1400.0f, 400.0f, -28.0f},
        {-1300.0f, 500.0f, -28.0f},
        {-1200.0f, 600.0f, -28.0f},
        {-1100.0f, 500.0f, -28.0f},
        {-1000.0f, 400.0f, -28.0f},
        {-900.0f, 300.0f, -28.0f},
        {-800.0f, 200.0f, -28.0f},
        {-700.0f, 300.0f, -28.0f},
        {-600.0f, 400.0f, -28.0f},
        {-500.0f, 500.0f, -28.0f},
        {-400.0f, 400.0f, -28.0f},
        {-300.0f, 300.0f, -28.0f},
        {-200.0f, 400.0f, -28.0f},
        {-100.0f, 500.0f, -28.0f},
        {0.0f, 600.0f, -28.0f},
        {100.0f, 500.0f, -28.0f},
        {200.0f, 400.0f, -28.0f},
        {300.0f, 300.0f, -28.0f},
        {400.0f, 400.0f, -28.0f},
        {500.0f, 500.0f, -28.0f},
        {600.0f, 600.0f, -28.0f},
        {700.0f, 500.0f, -28.0f},
        {800.0f, 400.0f, -28.0f},
        {900.0f, 300.0f, -28.0f},
        {1000.0f, 200.0f, -28.0f},
        {1100.0f, 300.0f, -28.0f},
        {1200.0f, 400.0f, -28.0f},
        {1300.0f, 300.0f, -28.0f},
        {1400.0f, 200.0f, -28.0f},
        {1500.0f, 300.0f, -28.0f},
        {1600.0f, 400.0f, -28.0f},
        {1700.0f, 300.0f, -28.0f},
        {1800.0f, 400.0f, -28.0f},
        {1900.0f, 300.0f, -28.0f}
    }
};

/* Per-team runner, cannon, camera, and platform work, plus the four player identities. */
M643PlayerPrefix *lbl_1_bss_174[2];
M643TeamScenePrefix *lbl_1_bss_16C[2];
M643PlayerIdentityPrefix *lbl_1_bss_15C[4];
M643ViewControlWork *lbl_1_bss_154[2];
M643PlatformWork *lbl_1_bss_14C[2];
s16 lbl_1_bss_148; /* Winning team index; -1 means there is no winning team. */
s32 lbl_1_bss_144; /* Set when the result-view timer elapses, without checking wipe completion. */
s32 lbl_1_bss_C[2][39]; /* Nonzero for platforms revealed by each team. */
s32 lbl_1_bss_8; /* Course layout index, chosen from 0 through 3 at startup. */

/* Callers use this lookup to reach the runner work for one of the two teams. */
M643PlayerPrefix *fn_1_1640(s16 teamIndex)
{
    return lbl_1_bss_174[teamIndex];
}

/* The gameplay camera uses this lookup to reach a team's cannon and its aim point. */
M643TeamScenePrefix *fn_1_165C(s16 teamIndex)
{
    return lbl_1_bss_16C[teamIndex];
}

/* Projectile arrivals use this lookup to reach the team platform models they reveal. */
void *fn_1_1678(s16 teamIndex)
{
    return lbl_1_bss_14C[teamIndex];
}

/* Platform arrivals use this lookup to request a brief shake of the team camera. */
M643ViewControlWork *fn_1_1694(s16 teamIndex)
{
    return lbl_1_bss_154[teamIndex];
}

/* Scene setup maps players to the four runner/shooter slots, falling back to player order
 * when any configured group has no player. */
void fn_1_16B0(s32 playerGroups[4])
{
    M643TeamTemplate playersByGroup;
    s32 playerIndex;
    playersByGroup = lbl_1_rodata_180;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        playersByGroup.group[lbl_80265728[playerIndex].grpNo] = playerIndex;
    }
    if (playersByGroup.group[0] == -1 || playersByGroup.group[1] == -1 ||
            playersByGroup.group[2] == -1 || playersByGroup.group[3] == -1) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++)
            playerGroups[playerIndex] = playerIndex;
        return;
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++)
        playerGroups[playerIndex] = lbl_80265728[playerIndex].grpNo;
}

/* Called by startup's sequence-init callback to set the scene, map callback, and four player
 * objects before the introduction begins. */
void fn_1_17A0(OMOBJ *object)
{
    M643SequenceWork *work = object->data;
    s32 playerGroups[4];
    HU3D_LIGHTID light;
    s16 team, isNight;
    s32 index;
    s16 nightMode = lbl_802C0278;
    isNight = nightMode == 0 ? 0 : 1;
    /* No background stream or opening sound is active until its sequence callback starts it. */
    work->musicHandle = -1;
    work->introSoundHandle = -1;
    lbl_1_bss_8 = rand8() % 4;
    lbl_1_bss_148 = -1;
    lbl_1_bss_144 = 0;
    fn_1_16B0(playerGroups);
    for (index = 0; index < 39; index++) lbl_1_bss_C[0][index] = lbl_1_bss_C[1][index] = 0;
    if (isNight == 0) {
        HuVecF direction = lbl_1_rodata_190;
        fn_1_4D0(&direction, &direction);
        light = Hu3DGLightCreate(2000.0f, 500.0f, 3000.0f,
            direction.x, direction.y, direction.z, 192, 192, 192);
        Hu3DGLightStaticSet(light, TRUE);
        Hu3DGLightInfinitytSet(light);
    } else {
        HuVecF direction;
        fn_1_298(&direction, 0.0f, -1000.0f, -3000.0f);
        fn_1_4D0(&direction, &direction);
        light = Hu3DGLightCreate(0.0f, 1000.0f, 3000.0f,
            direction.x, direction.y, direction.z, 64, 64, 255);
        Hu3DGLightStaticSet(light, TRUE);
        Hu3DGLightInfinitytSet(light);
        fn_1_298(&direction, 0.0f, -3000.0f, 2000.0f);
        fn_1_4D0(&direction, &direction);
        light = Hu3DGLightCreate(0.0f, 3000.0f, -2000.0f,
            direction.x, direction.y, direction.z, 255, 255, 255);
        Hu3DGLightStaticSet(light, TRUE);
        Hu3DGLightInfinitytSet(light);
    }
    if (isNight == 0) {
        HuVecF position = lbl_1_rodata_19C;
        HuVecF up = lbl_1_rodata_1A8;
        HuVecF target = lbl_1_rodata_1B4;
        fn_1_4D0(&up, &up);
        fn_8002A3B0(60.0f, 1.0f, 60000.0f, 3);
        fn_8002AA3C(0, 0, 0, 3);
        fn_8002A674(&position, &up, &target, 3);
        fn_8002A7D8(0.45f, 3);
    } else {
        HuVecF position = lbl_1_rodata_1C0;
        HuVecF up = lbl_1_rodata_1CC;
        HuVecF target = lbl_1_rodata_1D8;
        fn_1_4D0(&up, &up);
        fn_8002A3B0(60.0f, 1.0f, 60000.0f, 3);
        fn_8002AA3C(0, 0, 0, 3);
        fn_8002A674(&position, &up, &target, 3);
        fn_8002A7D8(0.45f, 3);
        Hu3DShadowSizeSet(64);
    }
    fn_1_16C(10, 8, fn_1_2D90);
    fn_1_16C(10, 92, fn_1_23C4);
    for (index = 0; index < 4; index++) {
        team = playerGroups[index] < 2 ? 0 : 1;
        if (playerGroups[index] % 2 == 0) {
            lbl_1_bss_174[team] = fn_1_16C(30, 60, fn_1_4AEC);
            lbl_1_bss_174[team]->identity.playerIndex = index;
            lbl_1_bss_174[team]->identity.teamIndex = team;
            lbl_1_bss_15C[index] = &lbl_1_bss_174[team]->identity;
        } else {
            lbl_1_bss_16C[team] = fn_1_16C(30, 172, fn_1_83C8);
            lbl_1_bss_16C[team]->identity.playerIndex = index;
            lbl_1_bss_16C[team]->identity.teamIndex = team;
            lbl_1_bss_15C[index] = &lbl_1_bss_16C[team]->identity;
        }
    }
    fn_8005DDEC(7);
    fn_1_20C(object, fn_1_1EB0);
}

/* Scheduled by fn_1_17A0; waits for fade-in, then starts the timed introduction. */
void fn_1_1EB0(OMOBJ *object)
{
    M643SequenceWork *work = object->data;
    if (fn_80071A8C() == 2) {
        fn_80071B08();
        work->phaseFrames = 0; /* Reset the frame counter when the introduction begins. */
        fn_1_20C(object, fn_1_1F10);
        return;
    }
}

/* Scheduled by fn_1_1EB0; starts the opening cue on fade-in and advances its 290-frame timer. */
void fn_1_1F10(OMOBJ *object)
{
    M643SequenceWork *work = object->data;

    if (work->introSoundHandle == -1 && fn_80071A8C() == 2) {
        work->introSoundHandle = HuAudFXPlay(1959);
    }
    work->phaseFrames++;
    if (work->phaseFrames >= 290) {
        fn_80071A9C();
        fn_1_20C(object, fn_1_1F98);
        return;
    }
}

/* Scheduled after the intro timer; fades the cue when its message appears, starts music when the
 * message sound plays, and waits for play mode. */
void fn_1_1F98(OMOBJ *object)
{
    M643SequenceWork *work = object->data;

    if (work->introSoundHandle != -1 &&
        (GameMesStatGet(fn_80071DF4()) & GAMEMES_STAT_EXIST)) {
        HuAudFXFadeOut(work->introSoundHandle, 500);
        work->introSoundHandle = -1;
    }
    if (work->musicHandle == -1 &&
        (GameMesStatGet(fn_80071DF4()) & GAMEMES_STAT_FXPLAY)) {
        work->musicHandle = HuAudSStreamPlay(MSM_STREAM_MGWARS);
    }
    if (fn_80071A8C() == 5) {
        fn_1_20C(object, fn_1_2058);
        return;
    }
}

/* Scheduled in play mode; selects a team after either runner object sets its finish flag. */
void fn_1_2058(OMOBJ *object)
{
    void *objectData = object->data;

    /* Wait for either team to finish; a simultaneous finish gets a random winner. */
    if (lbl_1_bss_174[0]->finished || lbl_1_bss_174[1]->finished) {
        if (lbl_1_bss_174[0]->finished && lbl_1_bss_174[1]->finished) {
            lbl_1_bss_148 = (s16)(rand8() % 2);
        } else {
            lbl_1_bss_148 = (s16)(lbl_1_bss_174[0]->finished ? 0 : 1);
        }
        fn_80071AC4(6);
        fn_1_20C(object, fn_1_2168);
        return;
    }
}

/* Runs after a team wins to release the sequence hold and wait for the finish phase to advance. */
void fn_1_2168(OMOBJ *object)
{
    void *objectData = object->data;

    /* Let the sequence manager advance once its finish message ends. */
    fn_80071B30();
    fn_1_20C(object, fn_1_21AC);
}

/* Runs after the finish request, registers the winning characters, and sets each winner's
 * minigame coin bonus to 10 outside practice mode. */
void fn_1_21AC(OMOBJ *object)
{
    M643SequenceWork *work = object->data;
    s32 winnerCharacterIds[4] = {-1, -1, -1, -1};
    s32 winnerCount = 0;
    s32 playerIndex;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_148 == lbl_1_bss_15C[playerIndex]->teamIndex) {
            winnerCharacterIds[winnerCount] = lbl_1_bss_15C[playerIndex]->characterId;
            if (!fn_8006A124(65551)) {
                /* Practice games show the winner but do not award the coin bonus. */
                lbl_80265750[playerIndex].mgCoinBonus = 10;
            }
            winnerCount++;
        }
    }
    fn_80071C18(winnerCharacterIds[0], winnerCharacterIds[1], winnerCharacterIds[2],
                   winnerCharacterIds[3]);
    if (work->musicHandle != -1) {
        HuAudSStreamFadeOut(work->musicHandle, 100);
    }
    fn_1_20C(object, fn_1_2300);
}

/* Waits for the sequence manager's pre-win mode, then advances to the result transition. */
void fn_1_2300(OMOBJ *object)
{
    void *objectData = object->data;

    if (fn_80071A8C() == 7) {
        fn_80071B08();
        fn_1_20C(object, fn_1_2354);
        return;
    }
}

/* Runs after the pre-win transition and advances the sequence when results are ready. */
void fn_1_2354(OMOBJ *object)
{
    void *objectData = object->data;

    if (lbl_1_bss_144 == 1 || lbl_1_bss_148 == -1) {
        fn_80071A9C();
        fn_80071B30();
        fn_1_20C(object, NULL);
        return;
    }
}

/* Called by sequence setup to create the map, its 39 platform positions, and two team platform
 * objects before play starts. */
void fn_1_23C4(OMOBJ *object)
{
    M643MapSceneWork *work = object->data;
    HU3D_MODELID collisionModels[40];
    s32 index;
    s16 isNight;
    s16 nightMode = lbl_802C0278;
    isNight = nightMode == 0 ? 0 : 1;
    work->mapObject = fn_8003AB38(lbl_1_bss_0, 32730, 5, 0, -1, NULL);
    for (index = 0; index < 5; index++) {
        work->mapObject->mdlId[index] = fn_80025A04(
            fn_80008DD0(lbl_1_data_28[isNight][index], 0x10000000,
                2));
    }
    fn_80026B8C(work->mapObject->mdlId[1], 1);
    fn_80026B8C(work->mapObject->mdlId[2], 1);
    fn_80026B64(work->mapObject->mdlId[1], lbl_1_rodata_60[0]);
    fn_80026B64(work->mapObject->mdlId[2], lbl_1_rodata_60[1]);
    if (isNight == 0)
        Hu3DReflectMapSet(fn_80008DD0(DATANUM(0x660000, 44), 0x10000000, 2));
    else Hu3DReflectMapSet(fn_80008DD0(DATANUM(0x660000, 45), 0x10000000, 2));
    fn_80029850(work->mapObject->mdlId[0], TRUE);
    fn_80026FFC(work->mapObject->mdlId[3]);
    for (index = 0; index < 39; index++) {
        HuVecF position = lbl_1_data_C4[lbl_1_bss_8][index];
        work->collisionModels[index] =
            fn_80025A04(fn_80008DD0(DATANUM(0x660000, 8), 0x10000000, 2));
        fn_80026790(work->collisionModels[index], &position);
    }
    collisionModels[0] = work->mapObject->mdlId[4];
    for (index = 0; index < 39; index++) collisionModels[index + 1] = work->collisionModels[index];
    /* Collision geometry stays hidden; the team objects manage the visible platform models. */
    for (index = 0; index < 40; index++)
        fn_800269E8(collisionModels[index], (1 << 0));
    fn_800804E8(collisionModels, 40, 128);
    fn_80080E8C();
    for (index = 1; index < 40; index++) fn_80080D7C(index, 0);
    for (index = 0; index < 2; index++) {
        lbl_1_bss_14C[index] = fn_1_16C(10, 158, fn_1_2838);
        lbl_1_bss_14C[index]->teamIndex = index;
    }
    fn_1_20C(object, fn_1_26F4);
}

void fn_1_26F4(OMOBJ *object)
{
    void *objectData = object->data;
}

/* Shot arrival reveals a team's platform model, enables its collision, and reports whether
 * that team's collision bit was already set. */
s32 fn_1_2708(M643PlatformWork *work, s16 platform)
{
    u32 collisionMask;
    s32 previousTeamMask = 0;
    /* Reveal this platform and add its collision bit to the selected team's mask. */
    fn_80026A44(work->raisedModels[platform], (1 << 0));
    collisionMask = fn_80080DAC(platform + 1);
    previousTeamMask = collisionMask & lbl_1_rodata_110[work->teamIndex];
    collisionMask |= lbl_1_rodata_110[work->teamIndex];
    fn_80080D7C(platform + 1, collisionMask);
    lbl_1_bss_C[work->teamIndex][platform] = 1;
    fn_1_1694(work->teamIndex)->shakeFrames = 5;
    fn_8006A030(fn_1_1640(work->teamIndex)->identity.playerIndex, 20, 7, 3);
    return previousTeamMask;
}

/* Called by map setup to create each team's hidden raised and visible lowered platform models. */
void fn_1_2838(OMOBJ *object)
{
    M643PlatformWork *work = object->data;
    s32 platformIndex;
    s16 isNight;
    s16 nightMode = lbl_802C0278;
    isNight = nightMode == 0 ? 0 : 1;
    for (platformIndex = 0; platformIndex < 39; platformIndex++) {
        HuVecF position = lbl_1_data_C4[lbl_1_bss_8][platformIndex];
        work->raisedModels[platformIndex] = fn_80025A04(
            fn_80008DD0(DATANUM(0x660000, 12), 0x10000000, 2));
        fn_80026790(work->raisedModels[platformIndex], &position);
        fn_80026B64(work->raisedModels[platformIndex], lbl_1_rodata_60[work->teamIndex]);
        fn_80026B8C(work->raisedModels[platformIndex], 6);
        fn_800269E8(work->raisedModels[platformIndex], (1 << 0));
        fn_80026808(work->raisedModels[platformIndex], 0.0f, 180.0f, 0.0f);
    }
    for (platformIndex = 0; platformIndex < 39; platformIndex++) {
        HuVecF position = lbl_1_data_C4[lbl_1_bss_8][platformIndex];
        work->loweredModels[platformIndex] = fn_80025A04(
            fn_80008DD0(lbl_1_data_50[isNight][work->teamIndex], 0x10000000,
                2));
        fn_80026790(work->loweredModels[platformIndex], &position);
        fn_80026B64(work->loweredModels[platformIndex], lbl_1_rodata_60[work->teamIndex]);
        fn_80026B8C(work->loweredModels[platformIndex], 2);
    }
    fn_1_20C(object, fn_1_2A98);
}

/* Runs each frame for one team to show nearby lowered platforms and any revealed raised models. */
void fn_1_2A98(OMOBJ *object)
{
    M643PlatformWork *work = object->data;
    s32 platformIndex;
    s16 isNight;
    f32 platformX, visibleLeft, visibleRight;
    HuVecF cameraPosition, cameraTarget, cameraUp;
    s16 nightMode = lbl_802C0278;
    isNight = nightMode == 0 ? 0 : 1;
    fn_80027CD0(lbl_1_rodata_60[work->teamIndex], &cameraPosition, &cameraUp, &cameraTarget);
    visibleLeft = cameraTarget.x - 1300.0f;
    visibleRight = 1300.0f + cameraTarget.x;
    for (platformIndex = 0; platformIndex < 39; platformIndex++) {
        platformX = lbl_1_data_C4[lbl_1_bss_8][platformIndex].x;
        if (platformX > visibleLeft && platformX < visibleRight)
            fn_80026A44(work->loweredModels[platformIndex], (1 << 0));
        else fn_800269E8(work->loweredModels[platformIndex], (1 << 0));
        if (lbl_1_bss_C[work->teamIndex][platformIndex]) {
            if (platformX > visibleLeft && platformX < visibleRight)
                fn_80026A44(work->raisedModels[platformIndex], (1 << 0));
            else fn_800269E8(work->raisedModels[platformIndex], (1 << 0));
        }
    }
    if (isNight == 0) {
        HuVecF position, up, target;
        fn_1_298(&position, 200.0f + cameraPosition.x, 1500.0f, -200.0f);
        fn_1_298(&up, 0.0f, 1.0f, 0.0f);
        fn_1_298(&target, cameraTarget.x, cameraTarget.y, cameraTarget.z);
        fn_8002A674(&position, &up, &target, lbl_1_rodata_60[work->teamIndex]);
    } else {
        HuVecF position, up, target;
        fn_1_298(&position, cameraPosition.x, 1000.0f, -500.0f);
        fn_1_298(&up, 0.0f, 1.0f, 0.0f);
        fn_1_298(&target, cameraTarget.x, cameraTarget.y, cameraTarget.z);
        fn_8002A674(&position, &up, &target, lbl_1_rodata_60[work->teamIndex]);
    }
}

/* Called by scene setup to create both team cameras and schedule the result-view controller. */
void fn_1_2D90(OMOBJ *object)
{
    void *objectData = object->data;
    s32 teamIndex;
    Hu3DCameraCreate(3);
    for (teamIndex = 0; teamIndex < 2; teamIndex++) {
        lbl_1_bss_154[teamIndex] = fn_1_16C(10, 16, fn_1_3374);
        lbl_1_bss_154[teamIndex]->teamIndex = teamIndex;
    }
    fn_1_20C(object, fn_1_2E38);
}

/* Waits for pre-win mode, then starts the winner's camera wipe and viewing timer. */
void fn_1_2E38(OMOBJ *object)
{
    M643ViewSequenceWork *work = object->data;
    if (lbl_1_bss_148 != -1 && fn_80071A8C() == 7) {
        WipeCreate(2, 0, 60);
        work->phaseFrames = 0;
        if (lbl_1_bss_148 == 0) {
            fn_1_20C(object, fn_1_2EF4);
            return;
        }
        if (lbl_1_bss_148 == 1) {
            fn_1_20C(object, fn_1_2F5C);
            return;
        }
    }
}

/* For a team 0 win, waits 60 frames during the outgoing wipe before selecting the result camera. */
void fn_1_2EF4(OMOBJ *object)
{
    M643ViewSequenceWork *work = object->data;
    work->phaseFrames++;
    if (work->phaseFrames >= 60) {
        work->phaseFrames = 0;
        fn_1_20C(object, fn_1_2FBC);
        return;
    }
}

/* For a team 1 win, waits 60 frames during the outgoing wipe before selecting the result camera. */
void fn_1_2F5C(OMOBJ *object)
{
    M643ViewSequenceWork *work = object->data;
    work->phaseFrames++;
    if (work->phaseFrames >= 60) {
        fn_1_20C(object, fn_1_2FBC);
        return;
    }
}

/* Sets the full-screen winner camera after the outgoing-wipe wait and starts a 60-frame fade-in. */
void fn_1_2FBC(OMOBJ *object)
{
    M643ViewSequenceWork *work = object->data;
    f32 distance, angle, zoom, tangent;
    Hu3DCameraPerspectiveSet(lbl_1_rodata_60[lbl_1_bss_148], 30.0f, 100.0f, 15000.0f,
                             1.2f);
    Hu3DCameraViewportSet(lbl_1_rodata_60[lbl_1_bss_148], 0.0f, 0.0f, 640.0f, 480.0f,
                          0.0f, 1.0f);
    Hu3DCameraScissorSet(lbl_1_rodata_60[lbl_1_bss_148], 0, 0, 640, 480);
    Hu3DCameraViewportSet(lbl_1_rodata_60[(lbl_1_bss_148 + 1) % 2], 640.0f, 480.0f, 576.0f,
                          480.0f, 0.1f, 1.0f);
    Hu3DCameraScissorSet(lbl_1_rodata_60[(lbl_1_bss_148+1)%2], 640, 480, 640, 480);
    distance = 450.0f;
    angle = 0.5235988f;
    tangent = (f32)tan(angle / 2.0f);
    zoom = distance / tangent;
    Hu3DCameraPosSet(lbl_1_rodata_60[lbl_1_bss_148],
        lbl_1_data_B8.x, 300.0f + lbl_1_data_B8.y, lbl_1_data_B8.z + zoom,
        0.0f, 1.0f, 0.0f,
        lbl_1_data_B8.x, 150.0f + lbl_1_data_B8.y, lbl_1_data_B8.z);
    WipeCreate(1, 5, 60);
    work->phaseFrames = 0;
    fn_1_20C(object, fn_1_3300);
}

/* Signals result readiness on the 61st timer update, without checking wipe completion. */
void fn_1_3300(OMOBJ *object)
{
    M643ViewSequenceWork *work = object->data;
    if (work->phaseFrames++ >= 60) {
        lbl_1_bss_144++;
        fn_1_20C(object, NULL);
        return;
    } else {
        return;
    }
}

/* Initializes a team's gameplay camera before fade-in and schedules its startup wait. */
void fn_1_3374(OMOBJ *object)
{
    M643ViewControlWork *work = object->data;
    f32 distance, angle, tangent;
    HuVecF pos, target, up;
    Hu3DCameraPerspectiveSet(lbl_1_rodata_60[work->teamIndex], 30.0f, 100.0f, 20000.0f,
                             lbl_1_rodata_E8[work->teamIndex][0]);
    Hu3DCameraViewportSet(lbl_1_rodata_60[work->teamIndex], lbl_1_rodata_68[work->teamIndex][0][0],
                          lbl_1_rodata_68[work->teamIndex][0][1],
                          lbl_1_rodata_68[work->teamIndex][0][2],
                          lbl_1_rodata_68[work->teamIndex][0][3], 0.0f, 1.0f);
    Hu3DCameraScissorSet(lbl_1_rodata_60[work->teamIndex],
        lbl_1_rodata_A8[work->teamIndex][0][0], lbl_1_rodata_A8[work->teamIndex][0][1],
        lbl_1_rodata_A8[work->teamIndex][0][2], lbl_1_rodata_A8[work->teamIndex][0][3]);
    distance = lbl_1_rodata_F8[work->teamIndex][0];
    angle = 0.5235988f;
    tangent = (f32)tan(0.5f * angle);
    work->cameraDepth = distance / tangent;
    fn_1_298(&target, lbl_1_data_AC.x, 550.0f, lbl_1_data_AC.z);
    target.x += 500.0f;
    pos = target;
    pos.z = work->cameraDepth;
    fn_1_298(&up, 0.0f, 1.0f, 0.0f);
    Hu3DCameraPosSetV(lbl_1_rodata_60[work->teamIndex], &pos, &up, &target);
    work->shakeFrames = 0;
    fn_1_20C(object, fn_1_3680);
}

/* Waits for sequence fade-in before handing the team camera to its runner-follow callback. */
void fn_1_3680(OMOBJ *object)
{
    M643ViewControlWork *work = object->data;
    if (fn_80071A8C() == 2) {
        work->phaseFrames = 0;
        fn_1_20C(object, fn_1_36DC);
        return;
    }
}

/* Tracks the runner during the introduction, with viewport shake before the split-screen change. */
void fn_1_36DC(OMOBJ *object)
{
    M643ViewControlWork *work = object->data;
    f32 distance, angle, shakeX, shakeY, tangent;
    HuVecF pos, target, up, actorPos;
    distance = lbl_1_rodata_F8[work->teamIndex][0];
    angle = 0.5235988f;
    tangent = (f32)tan(0.5f * angle);
    work->cameraDepth = distance / tangent;
    fn_80086BC8(fn_1_1640(work->teamIndex)->player->actor, &actorPos);
    fn_1_298(&target, actorPos.x, 550.0f, actorPos.z);
    target.x += 500.0f;
    pos = target;
    if (target.x > 2060.0f) {
        target.x = 2060.0f;
    }
    if (pos.x > 2060.0f) {
        pos.x = 2060.0f;
    }
    pos.z = work->cameraDepth;
    fn_1_298(&up, 0.0f, 1.0f, 0.0f);
    Hu3DCameraPosSetV(lbl_1_rodata_60[work->teamIndex], &pos, &up, &target);
    if (work->shakeFrames > 0) {
        shakeX = 4.0f * frandf() - 2.0f;
        shakeY = 4.0f * frandf() - 2.0f;
        Hu3DCameraViewportSet(
            lbl_1_rodata_60[work->teamIndex], shakeX + lbl_1_rodata_68[work->teamIndex][0][0],
            shakeY + lbl_1_rodata_68[work->teamIndex][0][1], lbl_1_rodata_68[work->teamIndex][0][2],
            lbl_1_rodata_68[work->teamIndex][0][3], 0.0f, 1.0f);
        work->shakeFrames--;
        if (work->shakeFrames == 0) {
            Hu3DCameraViewportSet(
                lbl_1_rodata_60[work->teamIndex], lbl_1_rodata_68[work->teamIndex][0][0],
                lbl_1_rodata_68[work->teamIndex][0][1], lbl_1_rodata_68[work->teamIndex][0][2],
                lbl_1_rodata_68[work->teamIndex][0][3], 0.0f, 1.0f);
        }
    }
    if (work->phaseFrames++ >= 230) {
        work->phaseFrames = 0;
        fn_1_20C(object, fn_1_3A98);
        return;
    }
}

/* Animates this team's viewport and scissor into the split-screen layout over 60 frames. */
void fn_1_3A98(OMOBJ *object)
{
    M643ViewControlWork *work = object->data;
    f32 t, distance, angle, tangent;
    f32 viewport[4], scissor[4];
    HuVecF pos, target, up, actorPos;
    work->phaseFrames++;
    t = (f32)work->phaseFrames / 60.0f;
    viewport[0] = (1.0f - t) * lbl_1_rodata_68[work->teamIndex][0][0] +
                  t * lbl_1_rodata_68[work->teamIndex][1][0];
    viewport[1] = (1.0f - t) * lbl_1_rodata_68[work->teamIndex][0][1] +
                  t * lbl_1_rodata_68[work->teamIndex][1][1];
    viewport[2] = (1.0f - t) * lbl_1_rodata_68[work->teamIndex][0][2] +
                  t * lbl_1_rodata_68[work->teamIndex][1][2];
    viewport[3] = (1.0f - t) * lbl_1_rodata_68[work->teamIndex][0][3] +
                  t * lbl_1_rodata_68[work->teamIndex][1][3];
    Hu3DCameraViewportSet(lbl_1_rodata_60[work->teamIndex], viewport[0], viewport[1], viewport[2],
                          viewport[3], 0.0f, 1.0f);
    if (work->teamIndex == 0) {
        Hu3DCameraPerspectiveSet(lbl_1_rodata_60[work->teamIndex], 30.0f, 100.0f, 20000.0f,
                                 viewport[2] / viewport[3]);
    } else {
        Hu3DCameraPerspectiveSet(lbl_1_rodata_60[work->teamIndex], 30.0f, 100.0f, 20000.0f,
                                 lbl_1_rodata_68[work->teamIndex][1][2] /
                                     lbl_1_rodata_68[work->teamIndex][1][3]);
    }
    scissor[0] = (1.0f - t) * lbl_1_rodata_A8[work->teamIndex][0][0] +
                 t * lbl_1_rodata_A8[work->teamIndex][1][0];
    scissor[1] = (1.0f - t) * lbl_1_rodata_A8[work->teamIndex][0][1] +
                 t * lbl_1_rodata_A8[work->teamIndex][1][1];
    scissor[2] = (1.0f - t) * lbl_1_rodata_A8[work->teamIndex][0][2] +
                 t * lbl_1_rodata_A8[work->teamIndex][1][2];
    scissor[3] = (1.0f - t) * lbl_1_rodata_A8[work->teamIndex][0][3] +
                 t * lbl_1_rodata_A8[work->teamIndex][1][3];
    Hu3DCameraScissorSet(lbl_1_rodata_60[work->teamIndex], scissor[0], scissor[1], scissor[2],
                         scissor[3]);
    distance =
        (1.0f - t) * lbl_1_rodata_F8[work->teamIndex][0] + t * lbl_1_rodata_F8[work->teamIndex][1];
    angle = 0.5235988f;
    tangent = (f32)tan(0.5f * angle);
    work->cameraDepth = distance / tangent;
    fn_80086BC8(fn_1_1640(work->teamIndex)->player->actor, &actorPos);
    fn_1_298(&target, actorPos.x, 550.0f, actorPos.z);
    target.x += 500.0f;
    pos = target;
    if (target.x > 2060.0f) target.x = 2060.0f;
    if (pos.x > 2060.0f) pos.x = 2060.0f;
    pos.z = work->cameraDepth;
    fn_1_298(&up, 0.0f, 1.0f, 0.0f);
    Hu3DCameraPosSetV(lbl_1_rodata_60[work->teamIndex], &pos, &up, &target);
    if (work->phaseFrames >= 60) {
        fn_1_20C(object, fn_1_40FC);
        return;
    }
}

/* Follows the runner in split-screen play until pre-win, with brief shake on platform hits. */
void fn_1_40FC(OMOBJ *object)
{
    M643ViewControlWork *work = object->data;
    f32 shakeX, shakeY;
    HuVecF actorPos, scenePos, midpoint = {0.0f, 0.0f, 0.0f}, pos, target, up;
    fn_1_298(&up, 0.0f, 1.0f, 0.0f);
    fn_80086BC8(fn_1_1640(work->teamIndex)->player->actor, &actorPos);
    scenePos = fn_1_165C(work->teamIndex)->aimPosition;
    fn_1_460(&midpoint, &actorPos, &scenePos);
    fn_1_6B0(&midpoint, &midpoint, 0.5f);
    /* The midpoint is calculated but the view continues to follow the runner. */
    fn_1_298(&target, actorPos.x, 550.0f, actorPos.z);
    target.x += 500.0f;
    pos = target;
    if (target.x > 2060.0f) target.x = 2060.0f;
    if (pos.x > 2060.0f) pos.x = 2060.0f;
    pos.z = work->cameraDepth;
    Hu3DCameraPosSetV(lbl_1_rodata_60[work->teamIndex], &pos, &up, &target);
    if (work->shakeFrames > 0) {
        shakeX = 4.0f * frandf() - 2.0f;
        shakeY = 4.0f * frandf() - 2.0f;
        Hu3DCameraViewportSet(
            lbl_1_rodata_60[work->teamIndex], shakeX + lbl_1_rodata_68[work->teamIndex][1][0],
            shakeY + lbl_1_rodata_68[work->teamIndex][1][1], lbl_1_rodata_68[work->teamIndex][1][2],
            lbl_1_rodata_68[work->teamIndex][1][3], 0.0f, 1.0f);
        work->shakeFrames--;
        if (work->shakeFrames == 0) {
            Hu3DCameraViewportSet(
                lbl_1_rodata_60[work->teamIndex], lbl_1_rodata_68[work->teamIndex][1][0],
                lbl_1_rodata_68[work->teamIndex][1][1], lbl_1_rodata_68[work->teamIndex][1][2],
                lbl_1_rodata_68[work->teamIndex][1][3], 0.0f, 1.0f);
        }
    }
    if (fn_80071A8C() == 7) {
        fn_1_20C(object, NULL);
        return;
    }
}

void fn_1_44B0(OMOBJ *object)
{
    void *objectData = object->data;
}

/* Searches already revealed platforms from the far end toward the current one and returns the
 * first platform within the caller's distance limit. */
s32 fn_1_44C4(M643PlayerPrefix *work, f32 radius)
{
    s32 platformIndex;
    HuVecF currentPosition, candidatePosition, jumpDelta;
    for (platformIndex = 38; platformIndex >= work->lastPlatform + 1; platformIndex--) {
        if (lbl_1_bss_C[work->identity.teamIndex][platformIndex] != 0) {
            currentPosition = lbl_1_data_C4[lbl_1_bss_8][work->lastPlatform];
            candidatePosition = lbl_1_data_C4[lbl_1_bss_8][platformIndex];
            fn_1_498(&jumpDelta, &candidatePosition, &currentPosition);
            /* Downhill jumps extend the search reach; uphill jumps reduce it. */
            if (jumpDelta.y < 0.0f) {
                jumpDelta.x *= 0.8f;
                jumpDelta.y *= 0.6f;
            }
            if (jumpDelta.y > 0.0f) {
                jumpDelta.y *= 1.25f;
            }
            if (fn_1_2A8(&jumpDelta) <= radius) {
                return platformIndex;
            }
        }
    }
    return 0;
}

/* Called by the runner callbacks each frame; pans the fall sound, then despawns a runner below
 * the course and resets its return animation state. */
BOOL fn_1_4664(M643PlayerPrefix *work)
{
    f32 pan;
    HuVecF screen;
    if (work->position.y < (-100.0f) && work->fallSoundPlayed == 0 && work->respawnPending == 0) {
        Hu3D3Dto2D(&work->position, lbl_1_rodata_60[work->identity.teamIndex], &screen);
        pan = screen.x / 576.0f;
        pan *= 127.0f;
        pan = pan < 0.0f ? 0.0f : (pan > 127.0f ? 127.0f : pan);
        HuAudFXPlayPan(work->identity.teamIndex == 0 ? 1957 : 1958,
                       (s16)pan);
        work->fallSoundPlayed = 1;
    }
    if (work->position.y < (-1000.0f)) {
        work->respawnPending = 1;
        /* Despawn also pauses the player and disables its collision body while it falls away. */
        fn_80086E3C(work->player);
        work->rescueSpinRadians = 0.0f;
        fn_8002C95C(work->player->actor->mdlId, work->player->omObj->mtnId[7]);
        fn_8005C2C8(work->identity.characterId, work->player->omObj->mtnId[7],
            0.0f, 4.0f, ((1 << 30) | (1 << 0)));
        work->previousJumpTarget = 0;
        work->jumpTarget = 0;
        work->fallSoundPlayed = 0;
        return TRUE;
    }
    return FALSE;
}

/* Active runner callbacks lock its depth to the course and update the shadow's height scaling. */
void fn_1_4880(M643PlayerPrefix *work)
{
    f32 scale;
    HuVecF shadowPos;
    fn_80086BC8(work->player->actor, &work->position);
    work->position.z = 100.0f;
    fn_80086B04(work->player->actor, &work->position);
    scale = 1.0f - (work->position.y - 200.0f) / 800.0f;
    shadowPos = work->position;
    shadowPos.y = 200.0f;
    fn_80026790(work->shadowModel, &shadowPos);
    fn_800268B0(work->shadowModel, scale, 1.0f, scale);
    /* The separate shadow is visible only beyond the platform course's ends. */
    if (shadowPos.x < (-1910.0f) || shadowPos.x > 1900.0f) {
        fn_80026A44(work->shadowModel, (1 << 0));
    } else {
        fn_800269E8(work->shadowModel, (1 << 0));
    }
}

/* Called during runner updates to detect the finish area or record the last platform touched. */
BOOL fn_1_49B8(M643PlayerPrefix *work)
{
    HuVecF position, delta;
    int mesh;
    fn_80086BC8(work->player->actor, &position);
    if (fn_80086CE8(work->player->actor, &mesh)) {
        fn_1_498(&delta, &lbl_1_data_B8, &position);
        if (mesh == 0 && fn_1_3C8(&delta) < 62500.0f) {
            work->finished = 1;
            return TRUE;
        }
        /* If the earlier tests pass, the height lookup reads mesh - 1 before checking mesh > 0,
         * so mesh zero accesses the preceding table entry. */
        if (fn_80087148(work->player, (1 << 4)) == 0 &&
            work->player->actor->velY <= 0.0f &&
            position.y > lbl_1_data_C4[lbl_1_bss_8][mesh - 1].y && mesh > 0) {
            work->lastPlatform = mesh - 1;
        }
    }
    return FALSE;
}

/* Creates a team's runner and shadow during scene setup, then schedules its fade-in callback. */
void fn_1_4AEC(OMOBJ *object)
{
    M643PlayerPrefix *work = object->data;
    work->identity.characterId = lbl_80265728[work->identity.playerIndex].charNo;
    work->respawnPending = 0;
    work->lastPlatform = 0;
    work->jumpTarget = 0;
    work->previousJumpTarget = 0;
    work->jumpDelayFrames = 0;
    work->fallSoundPlayed = 0;
    work->finished = 0;
    work->shadowModel =
        fn_80025A04(fn_80008DD0(DATANUM(0x660000, 15), 0x10000000, 2));
    fn_80026B64(work->shadowModel, lbl_1_rodata_60[work->identity.teamIndex]);
    fn_80026B8C(work->shadowModel, 4);
    fn_800269E8(work->shadowModel, (1 << 22));
    fn_80026A44(work->shadowModel, (1 << 0));
    {
    MGACTOR_PARAM params = {0};
    params.height = 150.0f;
    params.radius = 40.0f;
    params.param = work->identity.playerIndex;
    params.type = 0;
    params.attr = 0;
    params.correctHookParam = 0;
    params.narrowHook = NULL;
    params.correctHook = NULL;
    work->player = fn_80080728(work->identity.playerIndex, &params, 4,
        lbl_1_rodata_60[work->identity.teamIndex], 3, lbl_1_data_60);
    }
    fn_8007E818(work->player);
    fn_80026B64(work->player->actor->mdlId, lbl_1_rodata_60[work->identity.teamIndex]);
    fn_80026B8C(work->player->actor->mdlId, 6);
    fn_80080DD4(work->player->actor, lbl_1_rodata_110[work->identity.teamIndex]);
    work->position = lbl_1_data_AC;
    fn_80086AA8(work->player->actor, &work->position);
    work->rescue = fn_1_16C(40, 104, fn_1_B130);
    work->rescue->teamIndex = work->identity.teamIndex;
    work->rescue->runner = work;
    fn_1_20C(object, fn_1_4D8C);
}

/* Updates the runner shadow during fade-in and starts the pre-game wait when fade-in begins. */
void fn_1_4D8C(OMOBJ *object)
{
    M643PlayerPrefix *work = object->data;
    f32 scale;
    HuVecF shadowPos;
    fn_80086BC8(work->player->actor, &work->position);
    work->position.z = 100.0f;
    fn_80086B04(work->player->actor, &work->position);
    scale = 1.0f - (work->position.y - 200.0f) / 800.0f;
    shadowPos = work->position;
    shadowPos.y = 200.0f;
    fn_80026790(work->shadowModel, &shadowPos);
    fn_800268B0(work->shadowModel, scale, 1.0f, scale);
    if (shadowPos.x < (-1910.0f) || shadowPos.x > 1900.0f) {
        fn_80026A44(work->shadowModel, (1 << 0));
    } else {
        fn_800269E8(work->shadowModel, (1 << 0));
    }
    if (fn_80071A8C() == 2) {
        work->phaseFrames = 0;
        fn_1_20C(object, fn_1_4EF8);
        return;
    }
}

/* Opening callback launches the runner after 180 frames, then guides it to the starting
 * platform. */
void fn_1_4EF8(OMOBJ *object)
{
    M643PlayerPrefix *work = object->data;
    f32 scale, velocity;
    HuVecF destination, shadowPos;
    fn_80086BC8(work->player->actor, &work->position);
    work->position.z = 100.0f;
    fn_80086B04(work->player->actor, &work->position);
    scale = 1.0f - (work->position.y - 200.0f) / 800.0f;
    shadowPos = work->position;
    shadowPos.y = 200.0f;
    fn_80026790(work->shadowModel, &shadowPos);
    fn_800268B0(work->shadowModel, scale, 1.0f, scale);
    if (shadowPos.x < (-1910.0f) || shadowPos.x > 1900.0f) {
        fn_80026A44(work->shadowModel, (1 << 0));
    } else {
        fn_800269E8(work->shadowModel, (1 << 0));
    }
    work->phaseFrames++;
    if (work->phaseFrames == 180) {
        if (work->player->actor->gravity) {
            velocity = work->player->actor->gravity +
                ((-1.0f + sqrtf(1.0f + 8.0f *
                (30000.0f / work->player->actor->gravity))) /
                2.0f) * work->player->actor->gravity;
        } else {
            velocity = 0.0f;
        }
        fn_80086CC4(work->player->actor, velocity);
    } else if (work->phaseFrames >= 180) {
        destination = lbl_1_data_C4[lbl_1_bss_8][0];
        destination.z = 100.0f;
        if (fn_8007E8B4(work->player, &destination, 5.0f, 10.0f)) {
            fn_1_20C(object, fn_1_5298);
            return;
        }
    }
}

/* Waits for main play mode before choosing the human or computer runner update callback. */
void fn_1_5298(OMOBJ *object)
{
    M643PlayerPrefix *work = object->data;
    f32 scale;
    HuVecF shadowPos;
    fn_80086BC8(work->player->actor, &work->position);
    work->position.z = 100.0f;
    fn_80086B04(work->player->actor, &work->position);
    scale = 1.0f - (work->position.y - 200.0f) / 800.0f;
    shadowPos = work->position;
    shadowPos.y = 200.0f;
    fn_80026790(work->shadowModel, &shadowPos);
    fn_800268B0(work->shadowModel, scale, 1.0f, scale);
    if (shadowPos.x < (-1910.0f) || shadowPos.x > 1900.0f) {
        fn_80026A44(work->shadowModel, (1 << 0));
    } else {
        fn_800269E8(work->shadowModel, (1 << 0));
    }
    if (fn_80071A8C() == 5) {
        if (lbl_80265728[work->identity.playerIndex].type != 0) {
            fn_1_20C(object, fn_1_58DC);
            return;
        } else {
            fn_8007E888(work->player);
            fn_1_20C(object, fn_1_5438);
            return;
        }
    }
}

/* Updates a human-controlled runner each frame and hands off on a fall or finish. */
void fn_1_5438(OMOBJ *object)
{
    M643PlayerPrefix *work = object->data;
    if (fn_80071A8C() == 6) {
        fn_8007E818(work->player);
        fn_1_20C(object, fn_1_6750);
        return;
    }
    fn_1_4880(work);
    if (fn_1_4664(work)) {
        fn_1_20C(object, fn_1_6450);
        return;
    }
    if (fn_1_49B8(work)) {
        fn_8007E818(work->player);
        fn_1_20C(object, fn_1_6750);
        return;
    }
}

/* Updates a computer-controlled runner each frame and handles falls, finishes, and rescues. */
void fn_1_58DC(OMOBJ *object)
{
    M643PlayerPrefix *work = object->data;
    if (fn_80071A8C() == 6) {
        fn_1_20C(object, fn_1_6750);
        return;
    }
    fn_1_4880(work);
    if (fn_1_4664(work)) {
        fn_1_20C(object, fn_1_6450);
        return;
    }
    if (fn_1_49B8(work)) {
        fn_1_20C(object, fn_1_6750);
        return;
    }
    if (fn_1_BCB0(work)) {
        fn_1_20C(object, fn_1_5D94);
        return;
    }
}

/* Guides the computer runner toward the finish after it jumps from the final platforms,
 * continuing fall and finish checks each frame. */
void fn_1_5D94(OMOBJ *object)
{
    M643PlayerPrefix *work = object->data;
    f32 scale, velocity;
    HuVecF shadowPos;
    int mesh;
    BOOL jumping;
    if (fn_80071A8C() == 6) {
        fn_1_20C(object, fn_1_6750);
        return;
    }
    fn_80086BC8(work->player->actor, &work->position);
    work->position.z = 100.0f;
    fn_80086B04(work->player->actor, &work->position);
    scale = 1.0f - (work->position.y - 200.0f) / 800.0f;
    shadowPos = work->position;
    shadowPos.y = 200.0f;
    fn_80026790(work->shadowModel, &shadowPos);
    fn_800268B0(work->shadowModel, scale, 1.0f, scale);
    if (shadowPos.x < (-1910.0f) || shadowPos.x > 1900.0f) {
        fn_80026A44(work->shadowModel, (1 << 0));
    } else {
        fn_800269E8(work->shadowModel, (1 << 0));
    }
    if (fn_1_4664(work)) {
        fn_1_20C(object, fn_1_6450);
        return;
    }
    if (fn_1_49B8(work)) {
        fn_1_20C(object, fn_1_6750);
        return;
    }
    if (!fn_8007E8B4(work->player, &lbl_1_data_B8,
            7.0f, 10.0f)) {
        if (fn_80086CE8(work->player->actor, &mesh)) {
            jumping = fn_80087148(work->player, (1 << 4)) ? TRUE : FALSE;
            if (!jumping) {
                if (work->player->actor->gravity) {
                    velocity = work->player->actor->gravity +
                        ((-1.0f + sqrtf(1.0f + 8.0f *
                        (30000.0f / work->player->actor->gravity))) /
                        2.0f) * work->player->actor->gravity;
                } else {
                    velocity = 0.0f;
                }
                fn_80086CC4(work->player->actor, velocity);
            }
        }
    }
}

/* After a fall, requests the rescue effect once it is idle, then waits for the runner's return. */
void fn_1_6450(OMOBJ *object)
{
    M643PlayerPrefix *work = object->data;
    if (work->rescue->rescueRequested == 0) {
        work->rescue->rescueRequested = 1;
        fn_1_20C(object, fn_1_64B4);
        return;
    }
}

/* Spins the fallen runner during the rescue; resumes play after release,
 * or hands it to the result sequence if the game has ended. */
void fn_1_64B4(OMOBJ *object)
{
    M643PlayerPrefix *work = object->data;
    work->rescueSpinRadians += 0.08726646259971647;
    fn_80026790(work->player->actor->mdlId, &work->position);
    fn_80026808(work->player->actor->mdlId,
        0.0f, 180.0f * work->rescueSpinRadians / 3.141592653589793,
        0.0f);
    if (work->respawnPending == 0) {
        if (fn_80071A8C() == 6) {
            fn_8007E818(work->player);
            fn_1_20C(object, fn_1_6750);
            return;
        }
        if (fn_80071A8C() == 7) {
            fn_8007E818(work->player);
            work->phaseFrames = 0;
            fn_1_20C(object, fn_1_696C);
            return;
        }
        if (fn_80071A8C() == 8) {
            fn_8007E818(work->player);
            fn_1_20C(object, fn_1_6B98);
            return;
        }
        fn_80086F00(work->player, &work->position);
        fn_8007E818(work->player);
        work->player->actor->rotY = 90.0f;
        fn_80026808(work->player->actor->mdlId,
            0.0f, 90.0f, 0.0f);
        fn_1_20C(object, fn_1_6674);
        return;
    }
}

/* After rescue, waits for a recorded collision result before restoring human or computer
 * control. */
void fn_1_6674(OMOBJ *object)
{
    M643PlayerPrefix *work = object->data;
    int mesh;
    if (fn_80086CE8(work->player->actor, &mesh)) {
        if (lbl_80265728[work->identity.playerIndex].type != 0) {
            s16 difficulty = lbl_80265728[work->identity.playerIndex].comDif;
            work->jumpDelayFrames = lbl_1_rodata_178[difficulty];
            fn_1_20C(object, fn_1_58DC);
            return;
        } else {
            fn_8007E888(work->player);
            fn_1_20C(object, fn_1_5438);
            return;
        }
    }
}

/* After a finish, clears this callback for a losing team without stopping its actor;
 * guides the winning runner to the goal and waits for pre-win. */
void fn_1_6750(OMOBJ *object)
{
    M643PlayerPrefix *work = object->data;
    f32 scale;
    HuVecF destination = lbl_1_data_B8;
    HuVecF shadowPos;
    destination.z = 100.0f;
    if (lbl_1_bss_148 != -1 && lbl_1_bss_148 != work->identity.teamIndex) {
        fn_1_20C(object, NULL);
        return;
    }
    fn_80086BC8(work->player->actor, &work->position);
    work->position.z = 100.0f;
    fn_80086B04(work->player->actor, &work->position);
    scale = 1.0f - (work->position.y - 200.0f) / 800.0f;
    shadowPos = work->position;
    shadowPos.y = 200.0f;
    fn_80026790(work->shadowModel, &shadowPos);
    fn_800268B0(work->shadowModel, scale, 1.0f, scale);
    if (shadowPos.x < (-1910.0f) || shadowPos.x > 1900.0f) {
        fn_80026A44(work->shadowModel, (1 << 0));
    } else {
        fn_800269E8(work->shadowModel, (1 << 0));
    }
    if (lbl_1_bss_148 == work->identity.teamIndex) {
        fn_8007E8B4(work->player, &destination, 7.0f, 10.0f);
    }
    if (fn_80071A8C() == 7) {
        work->phaseFrames = 0;
        fn_1_20C(object, fn_1_696C);
        return;
    }
}

/* When the pre-win timer expires or winner mode starts, places the winning runner beside
 * the goal and prepares its result animation. */
void fn_1_696C(OMOBJ *object)
{
    M643PlayerPrefix *work = object->data;
    f32 scale;
    HuVecF shadowPos;
    fn_80086BC8(work->player->actor, &work->position);
    work->position.z = 100.0f;
    fn_80086B04(work->player->actor, &work->position);
    scale = 1.0f - (work->position.y - 200.0f) / 800.0f;
    shadowPos = work->position;
    shadowPos.y = 200.0f;
    fn_80026790(work->shadowModel, &shadowPos);
    fn_800268B0(work->shadowModel, scale, 1.0f, scale);
    if (shadowPos.x < (-1910.0f) || shadowPos.x > 1900.0f) {
        fn_80026A44(work->shadowModel, (1 << 0));
    } else {
        fn_800269E8(work->shadowModel, (1 << 0));
    }
    if (work->phaseFrames++ >= 60 || fn_80071A8C() == 8) {
        if (lbl_1_bss_148 == work->identity.teamIndex) {
            work->position = lbl_1_data_B8;
            work->position.x -= 100.0f;
            fn_80026790(work->player->actor->mdlId, &work->position);
            fn_80086B04(work->player->actor, &work->position);
            fn_80086CA8(work->player->actor, 0.0f);
            fn_80026E44(work->player->actor->mdlId);
            fn_800269E8(work->shadowModel, (1 << 0));
        }
        fn_1_20C(object, fn_1_6B98);
        return;
    }
}

/* Starts the runner's win or loss motion when the winner sequence begins, then stops updating. */
void fn_1_6B98(OMOBJ *object)
{
    M643PlayerPrefix *work = object->data;
    fn_80086BC8(work->player->actor, &work->position);
    work->position.z = 100.0f;
    fn_80086B04(work->player->actor, &work->position);
    if (fn_80071A8C() == 8) {
        if (lbl_1_bss_148 == work->identity.teamIndex) {
            fn_8005C2C8(work->identity.characterId, work->player->omObj->mtnId[5],
                0.0f, 4.0f, 0);
        } else {
            fn_8005C2C8(work->identity.characterId, work->player->omObj->mtnId[6],
                0.0f, 4.0f, 0);
        }
        fn_1_20C(object, NULL);
        return;
    }
}

/* Shooter updates call this to find the minimum and maximum platform indices near their camera. */
void fn_1_6CA0(M643TeamScenePrefix *work, s16 *minPlatform, s16 *maxPlatform)
{
    HuVecF cameraPosition, cameraTarget, cameraUp;
    fn_80027CD0(lbl_1_rodata_60[work->identity.teamIndex], &cameraPosition, &cameraUp,
                     &cameraTarget);
    for (*maxPlatform = 0; *maxPlatform < 38; (*maxPlatform)++) {
        if (lbl_1_data_C4[lbl_1_bss_8][*maxPlatform].x > 300.0f + cameraPosition.x) {
            break;
        }
    }
    for (*minPlatform = 38; *minPlatform > 0; (*minPlatform)--) {
        if (lbl_1_data_C4[lbl_1_bss_8][*minPlatform].x < cameraPosition.x - 1000.0f) {
            break;
        }
    }
}

/* Computer aiming chooses the farthest unrevealed platform ahead of its runner within
 * the weighted jump distance; zero means no candidate was found. */
s32 fn_1_6DE0(M643TeamScenePrefix *work, f32 radius)
{
    M643PlayerPrefix *runner = fn_1_1640(work->identity.teamIndex);
    s16 currentPlatform = runner->lastPlatform;
    s32 platformIndex;
    HuVecF runnerPosition, platformPosition, jumpDelta;
    for (platformIndex = 38; platformIndex >= currentPlatform + 1; platformIndex--) {
        if (lbl_1_bss_C[work->identity.teamIndex][platformIndex] == 0) {
            runnerPosition = lbl_1_data_C4[lbl_1_bss_8][currentPlatform];
            platformPosition = lbl_1_data_C4[lbl_1_bss_8][platformIndex];
            fn_1_498(&jumpDelta, &platformPosition, &runnerPosition);
            if (jumpDelta.y < 0.0f) {
                jumpDelta.x *= 0.8f;
                jumpDelta.y *= 0.6f;
            }
            if (jumpDelta.y > 0.0f) {
                jumpDelta.y *= 1.25f;
            }
            if (fn_1_2A8(&jumpDelta) <= radius) {
                return platformIndex;
            }
        }
    }
    return 0;
}

/* Cannon callbacks blend the operator into a new motion only when its motion index changes. */
void fn_1_6FA8(M643TeamScenePrefix *work, s16 motion, f32 blend, u32 flags)
{
    if (motion != work->operatorMotion) {
        work->operatorMotion = motion;
        fn_8005C2C8(work->identity.characterId,
            work->operatorObject->mtnId[work->operatorMotion],
            0.0f, blend, flags);
    }
}

/* Human cannon updates use the previous frame's stick state for an initial aim step,
 * a 20-frame repeat delay, then steps every three frames; current input is sampled afterward. */
void fn_1_7028(M643TeamScenePrefix *work)
{
    s32 directionIndex;
    M643InputPrefix *input;
    HuVecF stick;
    for (directionIndex = 0; directionIndex < 2; directionIndex++) {
        input = &work->aimInput[directionIndex];
        if (input->pressed) {
            if (input->pressCooldownFrames) {
                input->step = 0;
            } else {
                input->step = input->pressed;
                input->pressCooldownFrames = 3;
                input->repeatFrames = 20;
            }
        } else {
            if (!input->held) {
                input->step = 0;
            } else {
                if (--input->repeatFrames) {
                    input->step = 0;
                } else {
                    input->step = input->held;
                    input->repeatFrames = 3;
                }
            }
        }
        if (input->pressCooldownFrames) input->pressCooldownFrames--;
        if (input->released) input->released = 0;
        input->pressed = 0;
    }
    fn_1_298(&stick, lbl_802BFFFC[lbl_80265728[work->identity.playerIndex].padNo] / 56.0f,
        0.0f, -lbl_802BFFF8[lbl_80265728[work->identity.playerIndex].padNo] / 56.0f);
    if (stick.x <= (-0.5f)) {
        if (!work->aimInput[0].pressed && !work->aimInput[0].held)
            work->aimInput[0].pressed = 1;
        work->aimInput[0].held = 1;
    } else if (work->aimInput[0].held) {
        work->aimInput[0].released = 1;
        work->aimInput[0].held = 0;
    }
    if (stick.x >= 0.5f) {
        if (!work->aimInput[1].pressed && !work->aimInput[1].held)
            work->aimInput[1].pressed = 1;
        work->aimInput[1].held = 1;
    } else if (work->aimInput[1].held) {
        work->aimInput[1].released = 1;
        work->aimInput[1].held = 0;
    }
}

/* Intro and gameplay callbacks reserve a free projectile, start the firing animation and
 * muzzle effect, and play the team's firing sound at the cannon's screen position. */
void fn_1_72E0(M643TeamScenePrefix *work)
{
    s32 shotIndex;
    int soundId;
    f32 pan;
    HuVecF position, screen;
    for (shotIndex = 0; shotIndex < work->shotCount; shotIndex++) {
        if (work->shots[shotIndex]->inUse == 0) {
            work->shots[shotIndex]->inUse = 1;
            work->shots[shotIndex]->targetPlatform = work->aimPlatform;
            work->cooldownFrames = 90;
            fn_8002C95C(work->barrelModel, work->cannonMotions[2]);
            work->firingMotionFrames = fn_8002D530(work->barrelModel);
            /* A zero mask leaves the barrel's existing model attributes unchanged. */
            fn_800269E8(work->barrelModel, 0);
            fn_8002D86C(work->barrelModel, 1.0f);
            fn_800269E8(work->loadedShotModel, (1 << 0));
            fn_1_6FA8(work, 8, 4.0f, 0);
            fn_8002C95C(work->muzzleEffectModel, work->muzzleEffectMotion);
            fn_80026A44(work->muzzleEffectModel, (1 << 0));
            Hu3DModelObjPosGet(work->barrelModel, "643houdaiEFFnull", &position);
            fn_80026790(work->muzzleEffectModel, &position);
            fn_8006A030(work->identity.playerIndex, 20, 7, 3);
            Hu3D3Dto2D(&position, lbl_1_rodata_60[work->identity.teamIndex], &screen);
            pan = screen.x / 576.0f;
            pan *= 127.0f;
            pan = pan < 0.0f ? 0.0f :
                (pan > 127.0f ? 127.0f : pan);
            if (work->identity.teamIndex == 0) soundId = 1951;
            else soundId = 1952;
            HuAudFXPlayPan(soundId, (s16)pan);
            break;
        }
    }
}

/* Cannon callbacks keep the cannon, operator, and effects beside the moving camera,
 * easing the barrel toward the selected platform and restoring idle after firing. */
void fn_1_7554(M643TeamScenePrefix *work)
{
    f32 current, desired, difference, cross;
    HuVecF cameraPosition, cameraTarget, cameraUp;
    HuVecF position, delta, desiredDirection, currentDirection;
    HuVecF playerPosition, shadowPosition, hookPosition;
    fn_80027CD0(lbl_1_rodata_60[work->identity.teamIndex], &cameraPosition, &cameraUp,
                     &cameraTarget);
    work->cannonPosition.x = 600.0f + cameraPosition.x;
    work->cannonPosition.y = cameraPosition.y - 100.0f +
        30.0 * sin(0.017453292519943295 * work->bobDegrees);
    work->cannonPosition.z = 550.0f;
    work->bobDegrees += 1.0f;
    if (work->bobDegrees >= 360.0f) work->bobDegrees = 0.0f;
    /* Sample the muzzle from the barrel's previous transform before updating its position
     * and aim. */
    Hu3DModelObjPosGet(work->barrelModel, "643houdaiEFFnull", &work->muzzlePosition);
    position = work->cannonPosition;
    fn_80026790(work->barrelModel, &position);
    fn_80026790(work->baseModel, &position);
    fn_1_498(&delta, &work->aimPosition, &work->muzzlePosition);
    desired = atan2f(delta.x, delta.z);
    fn_1_298(&desiredDirection, (f32)sin(desired), 0.0f, (f32)cos(desired));
    fn_1_298(&currentDirection, (f32)sin(work->yawRadians), 0.0f, (f32)cos(work->yawRadians));
    difference = fn_1_214(fn_1_3F8(&desiredDirection, &currentDirection));
    cross = desiredDirection.x * currentDirection.z - desiredDirection.z * currentDirection.x;
    current = work->yawRadians;
    if (cross >= 0.0f) current += 0.1f * difference;
    else if (cross < 0.0f) current -= 0.1f * difference;
    while (current > 6.283185307179586) current -= 6.283185307179586;
    while (current < (-6.283185307179586)) current += 6.283185307179586;
    work->yawRadians = current;
    desired = atan2f(delta.z, delta.y);
    fn_1_298(&desiredDirection, (f32)sin(desired), 0.0f, (f32)cos(desired));
    fn_1_298(&currentDirection, (f32)sin(work->pitchRadians), 0.0f, (f32)cos(work->pitchRadians));
    difference = fn_1_3F8(&desiredDirection, &currentDirection);
    /* Pitch uses the dot product directly; zero Y components force the nonnegative branch. */
    cross = desiredDirection.y * currentDirection.z - desiredDirection.z * currentDirection.y;
    current = work->pitchRadians;
    if (cross >= 0.0f) current += 0.1f * difference;
    else if (cross < 0.0f) current -= 0.1f * difference;
    while (current > 6.283185307179586) current -= 6.283185307179586;
    while (current < (-6.283185307179586)) current += 6.283185307179586;
    work->pitchRadians = current;
    fn_80026808(work->barrelModel, 180.0f * work->pitchRadians / 3.141592653589793,
        180.0 + 180.0f * work->yawRadians / 3.141592653589793, 0.0f);
    fn_80026808(work->baseModel, 0.0f,
        180.0 + 180.0f * work->yawRadians / 3.141592653589793, 0.0f);
    fn_1_298(&playerPosition, work->cannonPosition.x, work->cannonPosition.y - 100.0f,
             work->cannonPosition.z);
    playerPosition.x += 200.0 * -cos(3.141592653589793 + work->yawRadians);
    playerPosition.z += 200.0 * sin(3.141592653589793 + work->yawRadians);
    fn_80026790(work->operatorObject->mdlId[0], &playerPosition);
    fn_80026790(work->operatorFollowModel, &playerPosition);
    fn_1_298(&shadowPosition, work->cannonPosition.x, work->cannonPosition.y - 120.0f,
        work->cannonPosition.z - 100.0f);
    fn_80026790(work->operatorObject->mdlId[1], &shadowPosition);
    fn_80026808(work->operatorObject->mdlId[1], 0.0f,
        135.0 + 180.0f * work->yawRadians / 3.141592653589793, 0.0f);
    Hu3DModelObjPosGet(work->baseModel, "643houdaibace-643EFF3Null", &hookPosition);
    fn_80026790(work->reloadEffectModel, &hookPosition);
    if (work->operatorMotion == 8 && fn_8002D4E0(work->operatorObject->mdlId[0]) ==
                                   fn_8002D530(work->operatorObject->mdlId[0])) {
        fn_1_6FA8(work, 0, 4.0f, ((1 << 30) | (1 << 0)));
    }
}

/* Position and RGB565 color formats for the cannon's trajectory line hook. */
const GXVtxDescList lbl_1_rodata_378[3] = {9, 1, 11, 1, 255, 0};
const GXVtxAttrFmtList lbl_1_rodata_390[3] = {9, 1, 4, 0, 11, 0, 0, 0, 255, 0, 0, 0};
const HuVecF lbl_1_rodata_3C0 = {0.0f, 0.0f, -1.0f};

/* When invoked as a model draw hook, draws the cannon aim line, platform markers, and
 * a sampled curved shot path using the attached team work. */
void fn_1_7D1C(HU3D_MODEL *model, Mtx mtx)
{
    M643TeamScenePrefix *work = model->hookData;
    GXLoadPosMtxImm(mtx, GX_PNMTX0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0, FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, FALSE, GX_TEVPREV);
    GXSetCullMode(GX_CULL_NONE);
    GXSetColorUpdate(TRUE);
    GXSetAlphaUpdate(FALSE);
    GXSetZCompLoc(TRUE);
    GXSetZMode(TRUE, GX_LESS, TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 255, GX_AOP_OR, GX_ALWAYS, 255);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDescv(lbl_1_rodata_378);
    GXSetVtxAttrFmtv(GX_VTXFMT0, lbl_1_rodata_390);
    {
        HuVecF up = lbl_1_rodata_3C0;
        HuVecF start = work->cannonPosition;
        HuVecF end = work->aimPosition;
        HuVecF curveStart = start;
        HuVecF curveEnd = end;
        HuVecF tangent, endTangent, position, previous;
        f32 distance, forwardDot, progress, progressSquared, progressCubed;
        s32 sampleIndex;
        fn_1_498(&tangent, &curveEnd, &curveStart);
        distance = fn_1_4D0(&tangent, &tangent);
        forwardDot = fn_1_3F8(&tangent, &up);
        fn_1_6B0(&tangent, &tangent, 3.0f * distance * forwardDot);
        fn_1_298(&endTangent, 0.0f, 0.0f,
            3.0f * -distance * (1.0f - forwardDot));
        GXBegin(GX_LINES, GX_VTXFMT0, 280);
        GXPosition3f32(start.x, start.y, start.z);
        GXWGFifo.u16 = M643_AIM_LINE_COLOR_BIT_PATTERN;
        GXPosition3f32(end.x, end.y, end.z);
        GXWGFifo.u16 = M643_AIM_LINE_COLOR_BIT_PATTERN;
        for (sampleIndex = 0; sampleIndex < 39; sampleIndex++) {
            HuVecF platform = lbl_1_data_C4[lbl_1_bss_8][sampleIndex];
            HuVecF offset = platform;
            offset.z += 300.0f;
            GXPosition3f32(platform.x, platform.y, platform.z);
            GXWGFifo.u16 = M643_PLATFORM_LINE_COLOR_BIT_PATTERN;
            GXPosition3f32(offset.x, offset.y, offset.z);
            GXWGFifo.u16 = M643_PLATFORM_LINE_COLOR_BIT_PATTERN;
        }
        position = curveStart;
        for (sampleIndex = 0; sampleIndex < 100; sampleIndex++) {
            HuVecF weightedVector;
            progress = sampleIndex / 100.0f;
            progressSquared = progress * progress;
            progressCubed = progress * progress * progress;
            previous = position;
            fn_1_6B0(&position, &curveStart,
                     1.0f + (2.0f * progressCubed - 3.0f * progressSquared));
            fn_1_6B0(&weightedVector, &curveEnd, -2.0f * progressCubed + 3.0f * progressSquared);
            fn_1_460(&position, &position, &weightedVector);
            fn_1_6B0(&weightedVector, &tangent,
                     progress + (progressCubed - 2.0f * progressSquared));
            fn_1_460(&position, &position, &weightedVector);
            fn_1_6B0(&weightedVector, &endTangent, progressCubed - progressSquared);
            fn_1_460(&position, &position, &weightedVector);
            GXPosition3f32(previous.x, previous.y, previous.z);
            GXWGFifo.u16 = M643_SHOT_PATH_COLOR_BIT_PATTERN;
            GXPosition3f32(position.x, position.y, position.z);
            GXWGFifo.u16 = M643_SHOT_PATH_COLOR_BIT_PATTERN;
        }
    }
    GXEnd();
}

/* Scene setup creates the team's cannon, operator, aiming marker, and two projectile slots,
 * then schedules the opening demonstration. */
void fn_1_83C8(OMOBJ *object)
{
    M643TeamScenePrefix *work = object->data;
    HuVecF cameraPos, cameraTarget, cameraUp;
    HU3D_MODELID characterModel;
    s32 i;
    s16 night;
    s16 nightFlag = lbl_802C0278;
    night = nightFlag == 0 ? 0 : 1;
    work->identity.characterId = lbl_80265728[work->identity.playerIndex].charNo;
    work->aimPlatform = 0;
    work->previousAimPlatform = work->aimPlatform;
    work->shotCount = 2;
    work->phaseFrames = 0;
    work->operatorMotion = -1;
    work->bobDegrees = 0.0f;
    work->computerTargetPlatform = 0;
    work->observedRunnerPlatform = 0;
    work->reactionFrames = 60;
    work->unusedSetupValue = 1;
    fn_80027CD0(lbl_1_rodata_60[work->identity.teamIndex], &cameraPos, &cameraUp,
                     &cameraTarget);
    work->cannonPosition.x = 500.0f + cameraPos.x;
    work->cannonPosition.y = cameraPos.y - 200.0f;
    work->cannonPosition.z = 500.0f;
    work->aimMarkerModel = fn_80025A04(
        fn_80008DD0(lbl_1_data_94[work->identity.teamIndex], 0x10000000, 2));
    work->aimMarkerMotion =
        fn_80030458(work->aimMarkerModel,
            fn_80008DD0(lbl_1_data_9C[work->identity.teamIndex],
                                                             0x10000000, 2));
    fn_8002C95C(work->aimMarkerModel, work->aimMarkerMotion);
    fn_800269E8(work->aimMarkerModel, ((1 << 30) | (1 << 0)));
    /* The opening target uses course zero and a fixed Z of zero; gameplay later uses the selected
     * course. */
    work->aimPosition = lbl_1_data_C4[0][work->aimPlatform];
    work->aimPosition.z = 0.0f;
    fn_80026790(work->aimMarkerModel, &work->aimPosition);
    fn_80026B64(work->aimMarkerModel, lbl_1_rodata_60[work->identity.teamIndex]);
    fn_80026B8C(work->aimMarkerModel, 5);
    fn_800269E8(work->aimMarkerModel, (1 << 22));
    work->yawRadians = 3.1415927f;
    work->pitchRadians = 0.0f;
    work->barrelModel =
        fn_80025A04(fn_80008DD0(DATANUM(0x660000, 30), 0x10000000, 2));
    {
        HuVecF position = work->cannonPosition;
        fn_80026790(work->barrelModel, &position);
    }
    fn_80026B64(work->barrelModel, lbl_1_rodata_60[work->identity.teamIndex]);
    fn_80026B8C(work->barrelModel, 7);
    for (i = 0; i < 3; i++)
        work->cannonMotions[i] = fn_80030458(
            work->barrelModel, fn_80008DD0(lbl_1_data_88[i], 0x10000000, 2));
    fn_8002C95C(work->barrelModel, work->cannonMotions[0]);
    /* A zero mask leaves the barrel's existing model attributes unchanged. */
    fn_800269E8(work->barrelModel, 0);
    work->baseModel =
        fn_80025A04(fn_80008DD0(DATANUM(0x660000, 31), 0x10000000, 2));
    fn_80026790(work->baseModel, &work->cannonPosition);
    fn_80026B64(work->baseModel, lbl_1_rodata_60[work->identity.teamIndex]);
    fn_80026B8C(work->baseModel, 7);
    work->muzzleEffectModel =
        fn_80025A04(fn_80008DD0(DATANUM(0x660000, 37), 0x10000000, 2));
    fn_80026B64(work->muzzleEffectModel, lbl_1_rodata_60[work->identity.teamIndex]);
    fn_80026B8C(work->muzzleEffectModel, 7);
    fn_800269E8(work->muzzleEffectModel, (1 << 0));
    work->muzzleEffectMotion =
        fn_80030458(work->muzzleEffectModel,
                        fn_80008DD0(DATANUM(0x660000, 38), 0x10000000, 2));
    fn_8002C95C(work->muzzleEffectModel, work->muzzleEffectMotion);
    work->loadedShotModel =
        fn_80025A04(fn_80008DD0(DATANUM(0x660000, 7), 0x10000000, 2));
    fn_80026B64(work->loadedShotModel, lbl_1_rodata_60[work->identity.teamIndex]);
    fn_80026B8C(work->loadedShotModel, 7);
    fn_800269E8(work->loadedShotModel, (1 << 0));
    fn_8002741C(work->barrelModel, "643houdaibace-643killerNull1", work->loadedShotModel);
    work->operatorObject = fn_8003AB38(lbl_1_bss_0, 101, 2, 10, 0, NULL);
    omSetStatBit(work->operatorObject, (1 << 8));
    characterModel = work->operatorObject->mdlId[0] =
        fn_80057068(work->identity.characterId, 4);
    fn_80026B64(characterModel, lbl_1_rodata_60[work->identity.teamIndex]);
    fn_80026B8C(characterModel, 7);
    for (i = 0; i < 10; i++)
        work->operatorObject->mtnId[i] =
            fn_8005B0B8(work->identity.characterId, lbl_1_data_60[i]);
    fn_8005B4BC(work->identity.characterId);
    fn_1_6FA8(work, 0, 0.0f, ((1 << 30) | (1 << 0)));
    {
        HuVecF position;
        fn_1_298(&position, work->cannonPosition.x, work->cannonPosition.y - 100.0f,
                 400.0f + work->cannonPosition.z);
        fn_80026790(characterModel, &position);
    }
    fn_80026808(characterModel, 0.0f, 180.0f, 0.0f);
    work->operatorObject->mdlId[1] =
        fn_80025A04(fn_80008DD0(lbl_1_data_A4[night], 0x10000000, 2));
    fn_80026B64(work->operatorObject->mdlId[1], lbl_1_rodata_60[work->identity.teamIndex]);
    fn_80026B8C(work->operatorObject->mdlId[1], 6);
    work->operatorBaseMotion =
        fn_80030458(work->operatorObject->mdlId[1],
                        fn_80008DD0(DATANUM(0x660000, 11), 0x10000000, 2));
    fn_8002C95C(work->operatorObject->mdlId[1], work->operatorBaseMotion);
    fn_800269E8(work->operatorObject->mdlId[1], ((1 << 30) | (1 << 0)));
    {
        HuVecF position;
        fn_1_298(&position, work->cannonPosition.x, work->cannonPosition.y - 120.0f,
                 work->cannonPosition.z);
        fn_80026790(work->operatorObject->mdlId[1], &position);
    }
    work->reloadEffectModel =
        fn_80025A04(fn_80008DD0(lbl_1_rodata_108[night], 0x10000000, 2));
    fn_80026B64(work->reloadEffectModel, lbl_1_rodata_60[work->identity.teamIndex]);
    fn_80026B8C(work->reloadEffectModel, 7);
    work->reloadEffectMotion =
        fn_80030458(work->reloadEffectModel,
                        fn_80008DD0(DATANUM(0x660000, 43), 0x10000000, 2));
    fn_8002C95C(work->reloadEffectModel, work->reloadEffectMotion);
    fn_800269E8(work->reloadEffectModel, (1 << 0));
    fn_8002D86C(work->reloadEffectModel, 1.3333334f);
    work->operatorFollowModel =
        fn_80025A04(fn_80008DD0(DATANUM(0x660000, 16), 0x10000000, 2));
    fn_80026B8C(work->operatorFollowModel, 7);
    fn_800269E8(work->operatorFollowModel, (1 << 22));
    fn_80026B64(work->operatorFollowModel, lbl_1_rodata_60[work->identity.teamIndex]);
    fn_80026A44(work->operatorFollowModel, (1 << 0));
    work->shots =
        HuMemDirectMallocNum(HEAP_HEAP, work->shotCount * sizeof(M643ShotPrefix *), 0x10000000);
    for (i = 0; i < work->shotCount; i++) {
        work->shots[i] = fn_1_16C(40, 148, fn_1_A138);
        work->shots[i]->slotIndex = i;
        work->shots[i]->teamIndex = work->identity.teamIndex;
        work->shots[i]->cannon = work;
    }
    fn_1_20C(object, fn_1_8C70);
}

/* Keeps the cannon aligned with the camera after setup, then starts its intro timer at fade-in. */
void fn_1_8C70(OMOBJ *object)
{
    M643TeamScenePrefix *work = object->data;
    fn_1_7554(work);
    if (fn_80071A8C() == 2) {
        work->phaseFrames = 0;
        fn_1_20C(object, fn_1_8CD4);
        return;
    }
}

/* During the intro, fires a demonstration shot after 90 frames and clears the firing cooldown. */
void fn_1_8CD4(OMOBJ *object)
{
    M643TeamScenePrefix *work = object->data;
    fn_1_7554(work);
    if (work->phaseFrames++ == 90) {
        fn_1_72E0(work);
        work->cooldownFrames = 0;
        work->phaseFrames = 0;
        fn_1_20C(object, fn_1_8F94);
        return;
    }
}

/* Finishes the cannon's demonstration and reload effects, then selects human or computer
 * controls when the main game sequence begins. */
void fn_1_8F94(OMOBJ *object)
{
    M643TeamScenePrefix *work = object->data;
    fn_1_7554(work);
    if (work->phaseFrames++ == (s32)work->firingMotionFrames) {
        fn_8002C95C(work->barrelModel, work->cannonMotions[1]);
        fn_8002D86C(work->barrelModel, 1.8666667f);
        fn_80026A44(work->loadedShotModel, (1 << 0));
        fn_8002C95C(work->reloadEffectModel, work->reloadEffectMotion);
        fn_8002D86C(work->reloadEffectModel, 5.3333335f);
        fn_80026A44(work->reloadEffectModel, (1 << 0));
    }
    if (fn_80071A8C() == 5) {
        work->phaseFrames = 0;
        if (lbl_80265728[work->identity.playerIndex].type != 0) {
            fn_1_20C(object, fn_1_96A4);
            return;
        } else {
            fn_1_20C(object, fn_1_90A8);
            return;
        }
    }
}

/* Each human cannon frame steps the aim across nearby platforms and fires on a new A press
 * once the cooldown expires; finish mode switches to the result wait. */
void fn_1_90A8(OMOBJ *object)
{
    M643TeamScenePrefix *work = object->data;
    s16 padIndex = lbl_80265728[work->identity.playerIndex].padNo;
    s16 minPlatform, maxPlatform;
    int soundId;
    f32 pan;
    HuVecF position, screen;
    if (fn_80071A8C() == 6) {
        fn_1_20C(object, fn_1_9A48);
        return;
    }
    fn_1_7028(work);
    work->previousAimPlatform = work->aimPlatform;
    if (work->aimInput[0].step) work->aimPlatform--;
    else if (work->aimInput[1].step) work->aimPlatform++;
    fn_1_6CA0(work, &minPlatform, &maxPlatform);
    if (work->aimPlatform < minPlatform) work->aimPlatform = minPlatform;
    if (work->aimPlatform > maxPlatform) work->aimPlatform = maxPlatform;
    work->aimPosition = lbl_1_data_C4[lbl_1_bss_8][work->aimPlatform];
    position = work->aimPosition;
    position.z = 0.0f;
    fn_80026790(work->aimMarkerModel, &position);
    if (work->previousAimPlatform != work->aimPlatform) {
        Hu3D3Dto2D(&position, lbl_1_rodata_60[work->identity.teamIndex], &screen);
        pan = screen.x / 576.0f;
        pan *= 127.0f;
        pan = pan < 0.0f ? 0.0f :
            (pan > 127.0f ? 127.0f : pan);
        if (work->identity.teamIndex == 0) soundId = 1955;
        else soundId = 1956;
        HuAudFXPlayPan(soundId, (s16)pan);
    }
    fn_1_7554(work);
    work->cooldownFrames--;
    if (work->cooldownFrames < 0) work->cooldownFrames = 0;
    if (work->cooldownFrames == 0 && (lbl_802C0008[padIndex] & PAD_BUTTON_A)) {
        fn_1_72E0(work);
        work->phaseFrames = 0;
        fn_1_20C(object, fn_1_98E4);
        return;
    }
}

/* Each computer cannon frame updates its target and firing decision, moves the aiming marker,
 * and starts the reload wait after a firing request. */
void fn_1_96A4(OMOBJ *object)
{
    M643TeamScenePrefix *work = object->data;
    s32 fireRequested;
    int soundId;
    f32 pan;
    HuVecF position, screen;
    if (fn_80071A8C() == 6) {
        fn_1_20C(object, fn_1_9A48);
        return;
    }
    work->previousAimPlatform = work->aimPlatform;
    fireRequested = fn_1_C5B0(work);
    work->aimPosition = lbl_1_data_C4[lbl_1_bss_8][work->aimPlatform];
    position = work->aimPosition;
    position.z = 0.0f;
    fn_80026790(work->aimMarkerModel, &position);
    if (work->previousAimPlatform != work->aimPlatform) {
        Hu3D3Dto2D(&position, lbl_1_rodata_60[work->identity.teamIndex], &screen);
        pan = screen.x / 576.0f;
        pan *= 127.0f;
        pan = pan < 0.0f ? 0.0f :
            (pan > 127.0f ? 127.0f : pan);
        if (work->identity.teamIndex == 0) soundId = 1955;
        else soundId = 1956;
        HuAudFXPlayPan(soundId, (s16)pan);
    }
    work->cooldownFrames--;
    if (work->cooldownFrames < 0) work->cooldownFrames = 0;
    if (fireRequested) {
        work->phaseFrames = 0;
        fn_1_20C(object, fn_1_98E4);
        return;
    }
}

/* After firing, keeps the cannon positioned and counts down its cooldown; once the firing
 * motion ends, starts the reload effects and restores the appropriate controller. */
void fn_1_98E4(OMOBJ *object)
{
    M643TeamScenePrefix *work = object->data;
    if (fn_80071A8C() == 6) {
        fn_1_20C(object, fn_1_9A48);
        return;
    }
    fn_1_7554(work);
    work->cooldownFrames--;
    if (work->cooldownFrames < 0) work->cooldownFrames = 0;
    if (work->phaseFrames++ >= work->firingMotionFrames) {
        work->phaseFrames = 0;
        fn_8002C95C(work->barrelModel, work->cannonMotions[1]);
        fn_8002D86C(work->barrelModel, 1.8666667f);
        fn_80026A44(work->loadedShotModel, (1 << 0));
        fn_8002C95C(work->reloadEffectModel, work->reloadEffectMotion);
        fn_8002D86C(work->reloadEffectModel, 2.6666667f);
        fn_80026A44(work->reloadEffectModel, (1 << 0));
        if (lbl_80265728[work->identity.playerIndex].type != 0) {
            fn_1_20C(object, fn_1_96A4);
            return;
        } else {
            fn_1_20C(object, fn_1_90A8);
            return;
        }
    }
}

/* After finish mode, waits for pre-win before restoring the operator's idle motion and timer. */
void fn_1_9A48(OMOBJ *object)
{
    M643TeamScenePrefix *work = object->data;
    if (fn_80071A8C() == 7) {
        work->phaseFrames = 0;
        fn_1_6FA8(work, 0, 4.0f, ((1 << 30) | (1 << 0)));
        fn_1_20C(object, fn_1_9AF4);
        return;
    }
}

/* During pre-win, moves the winning operator beside the goal after a short wait and hides its
 * cannon;
 * each team's callback hides its own aim marker before waiting for the result motion. */
void fn_1_9AF4(OMOBJ *object)
{
    M643TeamScenePrefix *work = object->data;
    if (work->phaseFrames++ >= 60 || fn_80071A8C() == 8) {
        if (lbl_1_bss_148 == work->identity.teamIndex) {
            HuVecF position = lbl_1_data_B8;
            position.x += 100.0f;
            position.z = 0.0f;
            fn_80026790(work->operatorObject->mdlId[0], &position);
            fn_80026808(work->operatorObject->mdlId[0], 0.0f, 0.0f, 0.0f);
            fn_80026790(work->operatorFollowModel, &position);
            fn_800269E8(work->operatorFollowModel, (1 << 0));
            fn_800269E8(work->barrelModel, (1 << 0));
            fn_800269E8(work->baseModel, (1 << 0));
            fn_800269E8(work->loadedShotModel, (1 << 0));
            fn_800269E8(work->operatorObject->mdlId[1], (1 << 0));
            fn_80026E44(work->operatorObject->mdlId[0]);
        }
        fn_800269E8(work->aimMarkerModel, (1 << 0));
        fn_1_20C(object, fn_1_9C74);
        return;
    }
}

/* Starts the cannon operator's win or loss motion at winner mode, then stops its callback. */
void fn_1_9C74(OMOBJ *object)
{
    M643TeamScenePrefix *work = object->data;
    if (fn_80071A8C() == 8) {
        if (lbl_1_bss_148 == work->identity.teamIndex) fn_1_6FA8(work, 5, 0.0f, 0);
        else fn_1_6FA8(work, 6, 0.0f, 0);
        fn_1_20C(object, NULL);
        return;
    }
}

/* The impact particle hook spreads eight yellow sparks and fades them until their slots
 * become reusable; the active time value stays at one throughout the fade. */
void fn_1_9D7C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx)
{
    HU3D_PARTICLE_DATA *data = particle->data;
    s32 particleIndex;
    s32 fadeAlpha;
    for (particleIndex = 0; particleIndex < particle->maxCnt; particleIndex++, data++) {
        if (data->time == 0) {
            data->time++;
            data->vel.x = 10.0 * sin(0.7853981633974483 * particleIndex);
            data->vel.y = 10.0 * -cos(0.7853981633974483 * particleIndex);
            data->vel.z = 0.0f;
            data->accel.y = 0.0f;
            data->color.r = 255;
            data->color.g = 255;
            data->color.b = 0;
            data->color.a = 255;
            data->scale = 16.0f;
        }
        if (data->time == 1) {
            data->vel.y += data->accel.y;
            data->accel.y -= 0.05f;
            fn_1_460(&data->pos, &data->pos, &data->vel);
            fadeAlpha = data->color.a - 10;
            if (fadeAlpha < 0) {
                fadeAlpha = 0;
                data->scale = 0.0f;
                data->time = -1;
            }
            data->color.a = fadeAlpha;
        }
    }
}

/* The projectile trail hook gives new puffs small random velocities, darkens and expands
 * them, then frees each slot once its alpha fades out. */
void fn_1_9F60(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx)
{
    HU3D_PARTICLE_DATA *data = particle->data;
    s32 particleIndex;
    s32 fadeAlpha;
    for (particleIndex = 0; particleIndex < particle->maxCnt; particleIndex++, data++) {
        if (data->time == 0) {
            data->time++;
            data->vel.x = 2.0f * frandf() - 1.0f;
            data->vel.y = 2.0f * frandf();
            data->vel.z = 2.0f * frandf() - 1.0f;
            data->color.r = 192;
            data->color.g = 128;
            data->color.b = 128;
            data->color.a = 127;
            data->scale = 128.0f;
        }
        if (data->time == 1) {
            data->color.r -= 15;
            if (data->color.r < 64) data->color.r = 64;
            data->color.g -= 10;
            if (data->color.g < 64) data->color.g = 64;
            data->color.b -= 10;
            if (data->color.b < 64) data->color.b = 64;
            fn_1_460(&data->pos, &data->pos, &data->vel);
            data->scale *= 1.01f;
            fadeAlpha = data->color.a - 2;
            if (fadeAlpha < 0) {
                fadeAlpha = 0;
                data->scale = 0.0f;
                data->time = -1;
            }
            data->color.a = fadeAlpha;
        }
    }
}

/* Creates a shot's models and particles when the team scene schedules a projectile. */
void fn_1_A138(OMOBJ *object)
{
    M643ShotPrefix *work = object->data;
    work->inUse = 0;
    work->projectileModel =
        fn_80025A04(fn_80008DD0(DATANUM(0x660000, 7), 0x10000000, 2));
    fn_80026B64(work->projectileModel, lbl_1_rodata_60[work->teamIndex]);
    fn_80026B8C(work->projectileModel, 6);
    fn_800269E8(work->projectileModel, (1 << 0));
    work->trailParticles = fn_8003216C(
        HuSprAnimRead(fn_80008634(DATANUM(0x210000, 2), 0x10000000)), 96);
    fn_80026B64(work->trailParticles, lbl_1_rodata_60[work->teamIndex]);
    fn_80026B8C(work->trailParticles, 7);
    {
        HU3D_PARTICLE *particle = lbl_802C01C8[work->trailParticles].hookData;
        HU3D_PARTICLE_DATA *data;
        s32 i;
        fn_8003269C(work->trailParticles, 0);
        fn_800326D0(work->trailParticles, fn_1_9F60);
        fn_80026760(work->trailParticles, 0.0f, 0.0f, 0.0f);
        data = particle->data;
        for (i = 0; i < particle->maxCnt; i++, data++) {
            data->time = -1;
            data->scale = 0.0f;
        }
    }
    work->impactParticles = fn_8003216C(
        HuSprAnimRead(fn_80008634(DATANUM(0x210000, 0), 0x10000000)), 8);
    {
        HU3D_PARTICLE *particle = lbl_802C01C8[work->impactParticles].hookData;
        HU3D_PARTICLE_DATA *data;
        s32 i;
        fn_8003269C(work->impactParticles, 1);
        fn_800326D0(work->impactParticles, fn_1_9D7C);
        fn_80026760(work->impactParticles, 0.0f, 0.0f, 0.0f);
        fn_80026B64(work->impactParticles, lbl_1_rodata_60[work->teamIndex]);
        fn_80026B8C(work->impactParticles, 7);
        fn_800269E8(work->impactParticles, (1 << 22));
        data = particle->data;
        for (i = 0; i < particle->maxCnt; i++, data++) {
            data->time = -1;
            data->scale = 0.0f;
        }
    }
    work->arrivalEffectModel = fn_80025A04(
        fn_80008DD0(DATANUM(0x660000, 39), 0x10000000, 2));
    fn_80026B64(work->arrivalEffectModel, lbl_1_rodata_60[work->teamIndex]);
    fn_80026B8C(work->arrivalEffectModel, 7);
    fn_800269E8(work->arrivalEffectModel, (1 << 0));
    work->arrivalEffectMotion =
        fn_80030458(work->arrivalEffectModel,
            fn_80008DD0(DATANUM(0x660000, 40), 0x10000000, 2));
    fn_8002C95C(work->arrivalEffectModel, work->arrivalEffectMotion);
    fn_1_20C(object, fn_1_A444);
}

/* Reference directions used when preparing a projectile's curved flight. */
const HuVecF lbl_1_rodata_410 = {0.0f, 0.0f, -1.0f};
const HuVecF lbl_1_rodata_41C = {0.0f, 1.0f, 0.0f};

/* Idle projectile callback: on a firing request, snapshots the muzzle and target,
 * builds the curved path, and starts the visible shot with duration measured in seconds. */
void fn_1_A444(OMOBJ *object)
{
    M643ShotPrefix *work = object->data;
    HuVecF referenceDirection = lbl_1_rodata_410;
    HuVecF unusedUp = lbl_1_rodata_41C;
    f32 distance, dot;
    if (work->inUse == 1) {
        HuVecF start = work->cannon->muzzlePosition;
        HuVecF end = work->cannon->aimPosition;
        fn_1_498(&work->launchDirection, &end, &start);
        distance = fn_1_4D0(&work->launchDirection, &work->launchDirection);
        work->speed = 25.0f;
        work->elapsedSeconds = 0.0f;
        work->curveStart = start;
        work->position = start;
        work->curveEnd = end;
        fn_1_498(&work->startTangent, &work->curveEnd, &work->curveStart);
        fn_1_4D0(&work->startTangent, &work->startTangent);
        dot = fn_1_3F8(&work->startTangent, &referenceDirection);
        fn_1_6B0(&work->startTangent, &work->startTangent, 3.0f * distance * dot);
        fn_1_298(&work->endTangent, 0.0f, 0.0f,
            3.0f * -distance * (1.0f - dot));
        fn_80026790(work->projectileModel, &work->position);
        fn_80026A44(work->projectileModel, (1 << 0));
        fn_80026808(work->projectileModel,
            180.0f * work->cannon->pitchRadians / 3.141592653589793,
            180.0f * work->cannon->yawRadians / 3.141592653589793,
            0.0f);
        work->durationSeconds = distance / work->speed / 60.0f;
        work->launchDistance = distance;
        fn_1_20C(object, fn_1_A6F0);
        return;
    }
}

/* Downward reference direction for the flying projectile's orientation. */
const HuVecF lbl_1_rodata_42C = {0.0f, -1.0f, 0.0f};

/* Each flying-shot frame advances its curved path and smoke trail. A platform already
 * revealed for the firing team deflects it into a falling spin; an opponent-only reveal
 * does not deflect it. Otherwise arrival reveals the target for the firing team. */
void fn_1_A6F0(OMOBJ *object)
{
    M643ShotPrefix *work = object->data;
    Mtx matrix;
    HuVecF weightedVector;
    HuVecF previousPosition = work->position;
    HuVecF position;
    f32 progress;
    work->elapsedSeconds += 0.016666668f;
    progress = work->elapsedSeconds / work->durationSeconds;
    if (progress > 1.0f) progress = 1.0f;
    fn_1_6B0(&position, &work->curveStart,
             1.0f + (2.0f * progress * progress * progress - 3.0f * progress * progress));
    fn_1_6B0(&weightedVector, &work->curveEnd,
             -2.0f * progress * progress * progress + 3.0f * progress * progress);
    fn_1_460(&position, &position, &weightedVector);
    fn_1_6B0(&weightedVector, &work->startTangent,
             progress + (progress * progress * progress - 2.0f * progress * progress));
    fn_1_460(&position, &position, &weightedVector);
    fn_1_6B0(&weightedVector, &work->endTangent,
             progress * progress * progress - progress * progress);
    fn_1_460(&position, &position, &weightedVector);
    work->position = position;
    fn_80026790(work->projectileModel, &work->position);
    {
        HuVecF up = lbl_1_rodata_42C;
        /* Both orientation-helper failure results are ignored, even for a degenerate travel
         * step. */
        fn_1_77C(matrix, &previousPosition, &position, &up);
        fn_1_E74(&work->rotationRadians, matrix);
    }
    fn_80026808(work->projectileModel,
        180.0f * work->rotationRadians.x / 3.141592653589793,
        180.0f * work->rotationRadians.y / 3.141592653589793,
        180.0f * work->rotationRadians.z / 3.141592653589793);
    {
        s32 emitted = 0;
        HU3D_PARTICLE *particle = lbl_802C01C8[work->trailParticles].hookData;
        HU3D_PARTICLE_DATA *data = particle->data;
        HuVecF origin = work->position;
        HuVecF direction;
        s32 i;
        fn_1_498(&direction, &previousPosition, &position);
        fn_1_4D0(&direction, &direction);
        fn_1_6B0(&direction, &direction, 150.0f);
        fn_1_460(&origin, &origin, &direction);
        for (i = 0; i < particle->maxCnt; i++, data++) {
            if (data->time < 0) {
                data->time = 0;
                data->pos = origin;
                if (++emitted >= 2) break;
            }
        }
    }
    if (work->elapsedSeconds > work->durationSeconds - 0.06666667f &&
            (fn_80080DAC(work->targetPlatform + 1) & lbl_1_rodata_110[work->teamIndex])) {
        work->fallVelocity.x = 10.0f * frandf() - 5.0f;
        work->fallVelocity.y = 10.0f * frandf();
        work->fallVelocity.z = 10.0f;
        work->spinVelocity.x = 0.008726646259971648 * (10.0f * frandf() - 5.0f);
        work->spinVelocity.y = 0.008726646259971648 * (10.0f * frandf() - 5.0f);
        work->spinVelocity.z = 0.008726646259971648 * (10.0f * frandf() - 5.0f);
        {
            s32 emitted = 0;
            HU3D_PARTICLE *particle = lbl_802C01C8[work->impactParticles].hookData;
            HU3D_PARTICLE_DATA *data = particle->data;
            HuVecF origin = work->position;
            s32 i;
            for (i = 0; i < particle->maxCnt; i++, data++) {
                if (data->time < 0) {
                    data->time = 0;
                    data->pos = origin;
                    if (++emitted >= 8) break;
                }
            }
        }
        {
            HuVecF screen;
            f32 pan;
            Hu3D3Dto2D(&work->position, lbl_1_rodata_60[work->teamIndex], &screen);
            pan = screen.x / 576.0f;
            pan *= 127.0f;
            pan = pan < 0.0f ? 0.0f :
                (pan > 127.0f ? 127.0f : pan);
            HuAudFXPlayPan(work->teamIndex == 0 ? 1960 : 1961, (s16)pan);
        }
        fn_1_20C(object, fn_1_AFF8);
        return;
    }
    if (work->elapsedSeconds > work->durationSeconds) {
        if (!fn_1_2708(fn_1_1678(work->teamIndex), work->targetPlatform)) {
            HuVecF screen;
            f32 pan;
            fn_8002C95C(work->arrivalEffectModel, work->arrivalEffectMotion);
            fn_80026A44(work->arrivalEffectModel, (1 << 0));
            fn_80026790(work->arrivalEffectModel, &work->position);
            Hu3D3Dto2D(&work->position, lbl_1_rodata_60[work->teamIndex], &screen);
            pan = screen.x / 576.0f;
            pan *= 127.0f;
            pan = pan < 0.0f ? 0.0f :
                (pan > 127.0f ? 127.0f : pan);
            HuAudFXPlayPan(work->teamIndex == 0 ? 1953 : 1954, (s16)pan);
        }
        fn_800269E8(work->projectileModel, (1 << 0));
        work->inUse = 0;
        fn_1_20C(object, fn_1_A444);
        return;
    }
}

/* After a shot hits an existing platform, lets it tumble under gravity and frees its slot
 * once it falls below the course. */
void fn_1_AFF8(OMOBJ *object)
{
    M643ShotPrefix *work = object->data;
    fn_1_460(&work->position, &work->position, &work->fallVelocity);
    work->fallVelocity.y -= 0.5f;
    fn_1_460(&work->rotationRadians, &work->rotationRadians, &work->spinVelocity);
    fn_80026790(work->projectileModel, &work->position);
    fn_80026808(work->projectileModel,
        180.0f * work->rotationRadians.x / 3.141592653589793,
        180.0f * work->rotationRadians.y / 3.141592653589793,
        180.0f * work->rotationRadians.z / 3.141592653589793);
    if (work->position.y < (-1000.0f)) {
        fn_800269E8(work->projectileModel, (1 << 0));
        work->inUse = 0;
        fn_1_20C(object, fn_1_A444);
        return;
    }
}

/* Runner setup creates a hidden looping rescue model and leaves it waiting for a fall request. */
void fn_1_B130(OMOBJ *object)
{
    M643CameraEffectWork *work = object->data;
    work->rescueRequested = 0;
    work->model = fn_80025A04(
        fn_80008DD0(DATANUM(0x660000, 35), 0x10000000, 2));
    work->motion =
        fn_80030458(work->model,
            fn_80008DD0(DATANUM(0x660000, 36), 0x10000000, 2));
    fn_80026B64(work->model, (u16)lbl_1_rodata_60[work->teamIndex]);
    fn_80026B8C(work->model, 6);
    fn_800269E8(work->model, (1 << 0));
    fn_8002C95C(work->model, work->motion);
    fn_800269E8(work->model, ((1 << 30) | (1 << 0)));
    work->carryingRunner = 0;
    fn_1_20C(object, fn_1_B21C);
}

/* Idle rescue callback: on a runner's fall request, starts a curved approach from the camera
 * toward the runner below the course; finish mode stops further rescues. */
void fn_1_B21C(OMOBJ *object)
{
    M643CameraEffectWork *work = object->data;
    HuVecF cameraPos, cameraTarget, cameraUp, actorPos, delta;
    f32 distance;

    fn_80027CD0(lbl_1_rodata_60[work->teamIndex], &cameraPos, &cameraUp, &cameraTarget);
    work->carryingRunner = 0;
    if (fn_80071A8C() == 6) {
        fn_1_20C(object, NULL);
        return;
    }
    if (work->rescueRequested == 1) {
        fn_80086BC8(work->runner->player->actor, &actorPos);
        work->curveStart.x = 300.0f + cameraPos.x;
        work->curveStart.y = 100.0f + cameraPos.y;
        work->curveStart.z = cameraPos.z;
        work->position = work->curveStart;
        work->curveEnd.x = actorPos.x;
        work->curveEnd.y = -500.0f;
        work->curveEnd.z = 100.0f;
        fn_1_498(&delta, &work->curveEnd, &work->curveStart);
        work->elapsedSeconds = 0.0f;
        distance = fn_1_4D0(&delta, &delta);
        work->durationSeconds = (distance / 20.0f) / 60.0f;
        fn_1_6B0(&work->startTangent, &delta, 3.5f * distance);
        fn_1_298(&work->endTangent, 0.0f, 3.5f * -distance, 0.0f);
        fn_80026A44(work->model, (1 << 0));
        fn_1_20C(object, fn_1_B408);
        return;
    }
}

/* Rescue approach callback moves along the curved path, first facing its travel direction,
 * then easing its rotation to zero before lifting the runner. */
void fn_1_B408(OMOBJ *object)
{
    M643CameraEffectWork *work = object->data;
    Mtx matrix;
    HuVecF weightedVector, previousPosition = work->position, position;
    f32 progress;

    work->carryingRunner = 0;
    work->elapsedSeconds += 0.016666668f;
    progress = work->elapsedSeconds / work->durationSeconds;
    if (progress > 1.0f) {
        progress = 1.0f;
    }
    fn_1_6B0(&position, &work->curveStart,
        1.0f + (2.0f*progress*progress*progress - 3.0f*progress*progress));
    fn_1_6B0(&weightedVector, &work->curveEnd,
        -2.0f*progress*progress*progress + 3.0f*progress*progress);
    fn_1_460(&position, &position, &weightedVector);
    fn_1_6B0(&weightedVector, &work->startTangent,
        progress + (progress*progress*progress - 2.0f*progress*progress));
    fn_1_460(&position, &position, &weightedVector);
    fn_1_6B0(&weightedVector, &work->endTangent, progress*progress*progress - progress*progress);
    fn_1_460(&position, &position, &weightedVector);
    work->position = position;
    fn_80026790(work->model, &work->position);
    if (work->elapsedSeconds < 0.5f * work->durationSeconds) {
        HuVecF up = {0.0f, -1.0f, 0.0f};
        /* Both orientation-helper failure results are ignored, even for a degenerate travel
         * step. */
        fn_1_77C(matrix, &previousPosition, &position, &up);
        fn_1_E74(&work->rotationRadians, matrix);
        fn_80026808(work->model,
            (180.0f * work->rotationRadians.x) / 3.141592653589793,
            (180.0f * work->rotationRadians.y) / 3.141592653589793,
            (180.0f * work->rotationRadians.z) / 3.141592653589793);
    } else {
        work->rotationRadians.x += -work->rotationRadians.x / 10.0f;
        work->rotationRadians.y += -work->rotationRadians.y / 10.0f;
        work->rotationRadians.z += -work->rotationRadians.z / 10.0f;
        fn_80026808(work->model,
            (180.0f * work->rotationRadians.x) / 3.141592653589793,
            (180.0f * work->rotationRadians.y) / 3.141592653589793,
            (180.0f * work->rotationRadians.z) / 3.141592653589793);
    }
    if (work->elapsedSeconds >= work->durationSeconds) {
        HuVecF unusedVector = {0.0f, 0.0f, 0.0f};
        fn_1_20C(object, fn_1_B7F0);
        return;
    }
}

/* After approach, places the runner 300 units below the item hook and lifts the rescue
 * until the
 * rescue's Y position is 200 units above the last platform; finish stops this callback. */
void fn_1_B7F0(OMOBJ *object)
{
    M643CameraEffectWork *work = object->data;
    HuVecF hookPosition;
    work->carryingRunner = 1;
    if (fn_80071A8C() == 6) {
        fn_1_20C(object, NULL);
        return;
    }
    Hu3DModelObjPosGet(work->model, "g008m5_000-itemhook_S", &hookPosition);
    work->position.y += 20.0f;
    fn_80026790(work->model, &work->position);
    hookPosition.y += 20.0f;
    hookPosition.y -= 300.0f;
    work->runner->position = hookPosition;
    fn_80026790(work->runner->player->actor->mdlId, &hookPosition);
    fn_80086B04(work->runner->player->actor, &hookPosition);
    if (work->position.y > 200.0f + lbl_1_data_C4[lbl_1_bss_8][work->runner->lastPlatform].y) {
        fn_1_20C(object, fn_1_B95C);
        return;
    }
}

/* After lifting the runner, moves the rescue over the last platform, carrying the actor
 * beneath its hook, then clears the fall state so the runner can respawn. */
void fn_1_B95C(OMOBJ *object)
{
    M643CameraEffectWork *work = object->data;
    HuVecF platformPosition = lbl_1_data_C4[lbl_1_bss_8][work->runner->lastPlatform];
    HuVecF returnStep, hookPosition;
    platformPosition.z = -20.0f;
    work->carryingRunner = 1;
    Hu3DModelObjPosGet(work->model, "g008m5_000-itemhook_S", &hookPosition);
    if (fn_80071A8C() == 6) {
        fn_1_20C(object, NULL);
        return;
    }
    returnStep.x = (platformPosition.x - work->position.x) / 20.0f;
    returnStep.y = 0.0f;
    returnStep.z = (platformPosition.z - work->position.z) / 20.0f;
    fn_1_460(&work->position, &work->position, &returnStep);
    fn_1_460(&hookPosition, &hookPosition, &returnStep);
    fn_80026790(work->model, &work->position);
    hookPosition.y -= 300.0f;
    work->runner->position = hookPosition;
    fn_80026790(work->runner->player->actor->mdlId, &hookPosition);
    fn_80086B04(work->runner->player->actor, &hookPosition);
    if (fabsf(work->position.x-platformPosition.x) < 10.0f &&
        fabsf(work->position.z-platformPosition.z) < 10.0f) {
        work->runner->respawnPending = 0;
        work->runner->player->actor->velY = 0.0f;
        fn_1_20C(object, fn_1_BC10);
        return;
    }
}

/* After releasing the runner, sends the rescue model upward and clears its request and
 * carrying flags once it is above the course. */
void fn_1_BC10(OMOBJ *object)
{
    M643CameraEffectWork *work = object->data;
    work->position.y += 20.0f;
    fn_80026790(work->model, &work->position);
    if (work->position.y > 1000.0f) {
        /* The display flag is cleared here, leaving the model visible above the playfield. */
        fn_80026A44(work->model, (1 << 0));
        work->rescueRequested = 0;
        work->carryingRunner = 0;
        fn_1_20C(object, fn_1_B21C);
        return;
    }
}

/* Computer runner updates select reachable revealed platforms, delay jumps by difficulty,
 * and steer in midair; reaching the final platforms starts the direct run to the goal. */
BOOL fn_1_BCB0(M643PlayerPrefix *work)
{
    s16 difficulty = lbl_80265728[work->identity.playerIndex].comDif;
    BOOL airborne = fn_80087148(work->player, (1 << 4)) ? TRUE : FALSE;
    int mesh;
    work->phaseFrames++;
    if (fn_80086CE8(work->player->actor, &mesh) && !airborne &&
        (work->lastPlatform == 38 || work->lastPlatform == 37 || work->lastPlatform == 36 ||
         work->lastPlatform == 35)) {
        fn_8007E818(work->player);
        fn_80086CC4(work->player->actor, work->player->actor->gravity ?
            work->player->actor->gravity +
                ((-1.0f + sqrtf(1.0f + 8.0f *
                (30000.0f / work->player->actor->gravity))) /
                2.0f) * work->player->actor->gravity : 0.0f);
        return TRUE;
    }
    if (work->jumpDelayFrames != 0) work->jumpDelayFrames--;
    if (fn_80086CE8(work->player->actor, &mesh) && !airborne) {
        s32 target = 0;
        if (work->phaseFrames % 10 == 0) {
            target = fn_1_44C4(work, 380.0f + lbl_1_rodata_148[difficulty] * frandf());
        }
        if (target != 0) {
            work->jumpTarget = target;
            if (work->previousJumpTarget != work->jumpTarget) {
                work->previousJumpTarget = work->jumpTarget;
                if (work->jumpDelayFrames == 0)
                    work->jumpDelayFrames = lbl_1_rodata_178[difficulty];
            }
            if (work->jumpDelayFrames == 0) {
                fn_8007E818(work->player);
        fn_80086CC4(work->player->actor, work->player->actor->gravity ?
            work->player->actor->gravity +
                        ((-1.0f + sqrtf(1.0f + 8.0f *
                        (30000.0f / work->player->actor->gravity))) /
                        2.0f) * work->player->actor->gravity : 0.0f);
            }
        } else {
            HuVecF destination = lbl_1_data_C4[lbl_1_bss_8][work->lastPlatform];
            HuVecF position, delta, push;
            f32 length, speed;
            destination.x += lbl_1_rodata_158[difficulty];
            fn_80086BC8(work->player->actor, &position);
            if (destination.x > position.x) {
                fn_1_498(&delta, &destination, &position);
                length = fn_1_2A8(&delta);
                speed = length < 3.0f ? length : 3.0f;
                fn_1_298(&push, 0.0f, 0.0f, speed);
                fn_80086CA8(work->player->actor, 90.0f);
                fn_80086C8C(work->player->actor, &push);
            }
        }
    }
    if (airborne && work->jumpTarget != 0 && work->jumpTarget != work->lastPlatform) {
        HuVecF destination = lbl_1_data_C4[lbl_1_bss_8][work->jumpTarget];
        f32 speed;
        destination.z = 100.0f;
        speed = 7.5f - lbl_1_rodata_168[difficulty] * (1.5f * frandf());
        if (fn_8007E8B4(work->player, &destination, speed, 5.0f)) work->jumpTarget = 0;
    }
    return FALSE;
}

/* Computer cannon updates choose an unrevealed platform ahead of the runner, introduce
 * difficulty-dependent aim errors, and request a shot after the cooldown and reaction delay. */
s32 fn_1_C5B0(M643TeamScenePrefix *work)
{
    M643PlayerPrefix *runner = fn_1_1640(work->identity.teamIndex);
    s16 difficulty = lbl_80265728[work->identity.playerIndex].comDif;
    s32 fireRequested = 0;
    HuVecF cameraUp, cameraTarget, cameraPos;
    s16 minPlatform, maxPlatform;
    fn_80027CD0(lbl_1_rodata_60[work->identity.teamIndex], &cameraPos, &cameraUp,
                     &cameraTarget);
    for (maxPlatform = 0; maxPlatform < 38; maxPlatform++) {
        if (lbl_1_data_C4[lbl_1_bss_8][maxPlatform].x > 300.0f + cameraPos.x) break;
    }
    for (minPlatform = 38; minPlatform > 0; minPlatform--) {
        if (lbl_1_data_C4[lbl_1_bss_8][minPlatform].x < cameraPos.x - 1000.0f) break;
    }
    if (work->computerTargetPlatform == 0 || work->observedRunnerPlatform != runner->lastPlatform) {
        work->observedRunnerPlatform = runner->lastPlatform;
        work->computerTargetPlatform = fn_1_6DE0(work, 380.0f);
        if (100.0f * frandf() > lbl_1_rodata_118[difficulty]) {
            work->computerTargetPlatform += rand8() % 3 - 1;
        }
        if (work->computerTargetPlatform < 0) work->computerTargetPlatform = 0;
        if (work->computerTargetPlatform > 38) work->computerTargetPlatform = 38;
    }
    work->phaseFrames++;
    if (work->phaseFrames % 10 == 0) {
        f32 aimChance = 100.0f * frandf();
        if (aimChance < lbl_1_rodata_128[difficulty]) {
            if (work->aimPlatform < work->computerTargetPlatform) work->aimPlatform++;
            if (work->aimPlatform > work->computerTargetPlatform) work->aimPlatform--;
        } else work->aimPlatform += rand8() % 3 - 1;
    }
    if (work->aimPlatform < minPlatform) work->aimPlatform = minPlatform;
    if (work->aimPlatform > maxPlatform) work->aimPlatform = maxPlatform;
    if (work->cooldownFrames == 0 && work->reactionFrames != 0) work->reactionFrames--;
    if (work->cooldownFrames == 0 && work->reactionFrames == 0 &&
        work->computerTargetPlatform != 0 &&
        (work->aimPlatform == work->computerTargetPlatform ||
         100.0f * frandf() > lbl_1_rodata_128[difficulty])) {
        work->computerTargetPlatform = 0;
        /* This reports the request even if both projectile slots are occupied. */
        fn_1_72E0(work);
        fireRequested = 1;
        if (lbl_1_rodata_138[difficulty] == 0) work->reactionFrames = 0;
        else work->reactionFrames = rand8() % lbl_1_rodata_138[difficulty];
    }
    fn_1_7554(work);
    return fireRequested;
}
