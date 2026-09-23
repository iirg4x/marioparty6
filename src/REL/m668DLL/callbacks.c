#define _MATH_H
#include "REL/m668DLL/player.h"
#include "humath.h"
#include "game/frand.h"
#include "game/hsfex.h"
#include "game/gamework.h"
#include "game/main.h"
#include "game/data.h"
#include "datadir_enum.h"
#include "game/memory.h"
#include "game/sprite.h"
#include "game/charman.h"
#include "game/audio.h"
#include "REL/m668DLL/rotor.h"

typedef struct TeamTemplate {
    s32 group[4];
} TeamTemplate;

extern const TeamTemplate lbl_1_rodata_68;
extern const TeamTemplate lbl_1_rodata_78;

extern const HuVecF lbl_1_rodata_88;
extern const HuVecF lbl_1_rodata_94;
extern const HuVecF lbl_1_rodata_A0;
extern s32 lbl_1_bss_18[2];

extern s32 lbl_1_bss_24;
extern s16 *lbl_1_bss_28[1];
extern void fn_1_54CC(OMOBJ *);
extern void fn_1_4EE8(OMOBJ *);
extern M668PlayerWork *lbl_1_bss_2C[4];
extern ANIMDATA *lbl_1_bss_3C;
extern void *lbl_1_bss_640;
extern void *lbl_1_bss_644;

extern void *fn_1_178(s32 objectCount, u32 dataSize, OMOBJ_FUNC callback);
extern void fn_1_218(OMOBJ *obj, OMOBJ_FUNC callback);
extern void fn_1_1FC4(OMOBJ *obj);
extern void fn_1_6EC8(OMOBJ *obj);
extern void fn_1_3858(OMOBJ *obj);
extern void fn_1_4508(OMOBJ *obj);
extern void fn_1_494C(OMOBJ *obj);
extern void fn_1_72CC(OMOBJ *obj);

#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/gamemes.h"
#include "game/mg/seqman.h"
#include "game/gamework.h"
#include "game/flag.h"
#include "game/frand.h"

typedef struct M668TeamTemplate_s {
    s32 group[4];
} M668TeamTemplate;

extern M668StageWork *lbl_1_bss_20;
extern void fn_1_218(OMOBJ *obj, OMOBJ_FUNC callback);
extern void fn_1_2064(OMOBJ *obj);
extern void fn_1_216C(OMOBJ *obj);
extern void fn_1_22D4(OMOBJ *obj);
extern void fn_1_2340(OMOBJ *obj);
extern u32 MgSeqModeNext(void);
extern s32 lbl_1_bss_18[2];
extern s32 lbl_1_bss_24;
extern M668PlayerWork *lbl_1_bss_2C[4];
extern const M668TeamTemplate lbl_1_rodata_BC;
extern s32 lbl_1_bss_8;

#include "game/hu3d.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"

extern const GXColor lbl_1_rodata_CC;
extern const GXColor lbl_1_rodata_E0;
extern void *lbl_1_bss_640;
extern void *lbl_1_bss_644;

#include "game/mg/actman.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/hu3d.h"
#include "humath.h"

typedef struct M668EffectModel_s {
    HU3D_MODELID model;
    HU3D_MOTIONID motion;
} M668EffectModel;

/* Allocator requests 38 decimal bytes. Bytes20..23 have no observed use. */
typedef struct M668SceneWork_s {
    HU3D_MODELID sceneModel[2];
    HU3D_MODELID sceneLink[2];
    HU3D_MODELID actorModel[2];
    HU3D_MODELID actorLink[2];
    HU3D_MOTIONID actorMotion[2];
    u8 unknown20[4];
    M668EffectModel effect[2];
    HU3D_MODELID collisionModel[3];
} M668SceneWork;
typedef char M668SceneWork_size[(sizeof(M668SceneWork) == 38) ? 1 : -1];

extern M668StageWork *lbl_1_bss_20;
extern HuVecF lbl_1_bss_340[64];
extern HuVecF lbl_1_bss_40[64];
extern void fn_1_218(OMOBJ *, OMOBJ_FUNC);
extern void fn_1_23B4(s16);
extern void fn_1_27D0(s16);
extern void fn_1_2FA0(s16);
extern void fn_1_3CB8(OMOBJ *);
extern void fn_1_3CE4(s16);
extern f32 fn_1_16B4(f32, f32);
extern void fn_1_2A4(HuVecF *, f32, f32, f32);
extern f32 fn_1_4DC(HuVecF *, const HuVecF *);

#include "REL/m668DLL/rotor.h"

/* The retail callback retains the common work fetch and inactive-stage guard. */


#include "game/mg/actman.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/hu3d.h"
#include "humath.h"

/* Allocator requests 38 decimal bytes. Bytes20..23 have no observed use. */

typedef char M668SceneWork_size[(sizeof(M668SceneWork) == 38) ? 1 : -1];

extern M668StageWork *lbl_1_bss_20;
extern HuVecF lbl_1_bss_340[64];
extern HuVecF lbl_1_bss_40[64];
extern void fn_1_218(OMOBJ *, OMOBJ_FUNC);
extern void fn_1_23B4(s16);
extern void fn_1_27D0(s16);
extern void fn_1_2FA0(s16);
extern void fn_1_3CB8(OMOBJ *);
extern void fn_1_3CE4(s16);
extern f32 fn_1_16B4(f32, f32);
extern void fn_1_2A4(HuVecF *, f32, f32, f32);
extern f32 fn_1_4DC(HuVecF *, const HuVecF *);

#include "REL/m668DLL/player.h"

extern double fmod(double, double);
extern double cos(double);
extern double sin(double);
extern void fn_1_2A4(HuVecF *, f32, f32, f32);
extern f32 fn_1_404(const HuVecF *, const HuVecF *);
extern f32 fn_1_220(f32);

/* Float wrapper around the SDK double operation; the divisor is the
 * existing retail 360-degree provider, not a second literal pool. */


#include "REL/m668DLL/player.h"
#include "game/gamework.h"
#include "game/main.h"

extern const MGACTOR_PARAM lbl_1_rodata_198;
extern HuVecF lbl_1_data_64[4];
extern unsigned int lbl_1_data_3C[10];
extern M668PlayerWork *lbl_1_bss_2C[4];
extern void fn_1_4BFC(OMOBJ *obj);

#include "REL/m668DLL/rotor.h"
#include "game/mg/seqman.h"
extern void fn_1_4C64(OMOBJ *obj);


#include "REL/m668DLL/player.h"
#include "game/pad.h"
extern s32 lbl_1_bss_C;
extern void fn_1_46C(HuVecF *dst, const HuVecF *a, const HuVecF *b);
extern void fn_1_2A4(HuVecF *dst, f32 x, f32 y, f32 z);
extern void fn_1_4A4(HuVecF *dst, const HuVecF *a, const HuVecF *b);
extern f32 fn_1_4DC(HuVecF *dst, const HuVecF *src);
extern void fn_1_6C24(OMOBJ *obj);
extern void fn_1_6778(OMOBJ *obj);
extern void fn_1_6924(OMOBJ *obj);

#include "REL/m668DLL/player.h"
#include "game/pad.h"
#include "humath.h"

extern s32 lbl_1_bss_C;
extern Point3d lbl_1_data_94[2];
extern void fn_1_218(OMOBJ *, OMOBJ_FUNC);
extern void fn_1_6C24(OMOBJ *);
extern void fn_1_46C(HuVecF *, const HuVecF *, const HuVecF *);
extern void fn_1_4A4(HuVecF *, const HuVecF *, const HuVecF *);
extern f32 fn_1_4DC(HuVecF *, const HuVecF *);
extern f32 fn_1_404(const HuVecF *, const HuVecF *);
extern void fn_1_6BC(HuVecF *, const HuVecF *, f32);

extern inline f64 m668_fabs64(f64 x)
{
    return __fabs(x);
}

extern inline f32 m668_fabs32(f32 x)
{
    return m668_fabs64((f64)x);
}

#include "REL/m668DLL/rotor.h"
#include "game/mg/actman.h"

/* Consumed player-work layout; names describe observed uses, not original names. */
typedef struct M668PlayerFinishWork_s {
    s16 playerNo;
    s16 partnerNo;
    s16 charNo;
    s16 teamNo;
    s16 formationNo;
    MGPLAYER *player;
    s16 state;
    HuVecF position[3];
    s16 playerSelector;
    s32 transitionFlag;
    s32 timer60;
} M668PlayerFinishWork;

extern s32 lbl_1_bss_24;
extern HuVecF lbl_1_data_64[4];
void fn_1_6DF8(OMOBJ *obj);

#include "REL/m668DLL/rotor.h"

/* The retail callback retains the common work fetch and inactive-stage guard. */


#include "game/mg/seqman.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/wipe.h"
#include "game/data.h"
#include "game/memory.h"

typedef struct M668CameraWork {
    s16 model;
    s16 motion;
    s32 timer;
} M668CameraWork;
extern M668StageWork *lbl_1_bss_20;
extern s32 lbl_1_bss_8;
extern s32 lbl_1_bss_C;
extern s32 lbl_1_bss_24;
typedef struct M668CameraPair { HuVecF points[2]; } M668CameraPair;
extern const M668CameraPair lbl_1_rodata_2A0, lbl_1_rodata_2B8;
extern const HuVecF lbl_1_rodata_2D0;
void fn_1_218(OMOBJ *, OMOBJ_FUNC);
void fn_1_7014(OMOBJ *);
void fn_1_70A0(OMOBJ *);
void fn_1_7240(OMOBJ *);
u32 MgSeqModeGet(void);

#include "REL/m668DLL/rotor.h"
#include "game/audio.h"
#include "game/mg/seqman.h"
extern void fn_1_758C(OMOBJ *);


#include "REL/m668DLL/rotor.h"
#include "game/audio.h"
#include "game/mg/seqman.h"
extern void fn_1_7BE8(OMOBJ *);


#include "humath.h"
#include "game/sprite.h"
#include "game/main.h"
#include "game/object.h"
#include "string.h"
#include "math.h"

extern double sin(double);
extern double cos(double);




extern s32 lbl_1_data_AC;
extern s32 lbl_1_bss_10;
extern void *lbl_1_bss_640;

extern HuVecF lbl_1_bss_40[64];
extern HuVecF lbl_1_bss_340[64];
extern ANIMDATA *lbl_1_bss_3C;
extern void *lbl_1_bss_644;
extern s32 lbl_1_bss_14;
extern const GXColor lbl_1_rodata_13C;
extern void HuSprTexLoad(ANIMDATA *anim, s16 bmpNo, s16 texMapId,
                         GXTexWrapMode wrapS, GXTexWrapMode wrapT,
                         GXTexFilter filter);

#include "REL/m668DLL/player.h"
#include "game/pad.h"
#include "game/audio.h"
extern s32 lbl_1_bss_C, lbl_1_bss_24, lbl_1_bss_18[2];
extern M668PlayerWork *lbl_1_bss_2C[4];
extern s16 *lbl_1_bss_28[];
extern const HuVecF lbl_1_rodata_1E0, lbl_1_rodata_1EC;
extern f64 atan2(f64, f64);
extern void fn_1_2A4(HuVecF *, f32, f32, f32);
extern void fn_1_4A4(HuVecF *, const HuVecF *, const HuVecF *);
extern void fn_1_434(HuVecF *, const HuVecF *, const HuVecF *);
extern void fn_1_46C(HuVecF *, const HuVecF *, const HuVecF *);
extern f32 fn_1_4DC(HuVecF *, const HuVecF *);
extern void fn_1_6BC(HuVecF *, const HuVecF *, f32);
extern void fn_1_6C24(OMOBJ *);
extern void fn_1_6778(OMOBJ *);
extern void fn_1_664C(OMOBJ *);
static inline f32 M668Atan2f(f32 y, f32 x) { return (f32)atan2((f64)y, (f64)x); }

#include "REL/m668DLL/rotor.h"
#include "game/data.h"
#include "game/hu3d.h"
#include "game/main.h"

extern void fn_1_7508(OMOBJ *);




typedef struct { f32 value[4]; } M668FloatTable4;
typedef struct { s32 value[4]; } M668IntTable4;
extern const M668FloatTable4 lbl_1_rodata_208,lbl_1_rodata_234,lbl_1_rodata_244;
extern const M668IntTable4 lbl_1_rodata_218;
extern const HuVecF lbl_1_rodata_228,lbl_1_rodata_254,lbl_1_rodata_260;
#include "REL/m668DLL/rotor.h"
#include "REL/m668DLL/player.h"
#include "game/audio.h"
#include "game/hu3d.h"
#include "humath.h"
#include <math.h>
extern double sin(double);
extern double cos(double);

typedef struct M668VecBound_s {
    HuVecF xyz;
    f32 margin;
} M668VecBound;

extern const HuVecF lbl_1_rodata_2E0, lbl_1_rodata_2EC, lbl_1_rodata_2F8;
extern M668PlayerWork *lbl_1_bss_2C[4];
extern void fn_1_2A4(HuVecF *, f32, f32, f32);
extern void fn_1_434(HuVecF *, const HuVecF *, const HuVecF *);
extern void fn_1_46C(HuVecF *, const HuVecF *, const HuVecF *);
extern void fn_1_4A4(HuVecF *, const HuVecF *, const HuVecF *);
extern f32 fn_1_4DC(HuVecF *, const HuVecF *);
extern void fn_1_6BC(HuVecF *, const HuVecF *, f32);
extern f32 fn_1_404(const HuVecF *, const HuVecF *);
extern s32 fn_1_1844(const M668VecBound *, const M668VecBound *);
extern void fn_1_7B30(OMOBJ *);

#include "REL/m668DLL/player.h"
#include "game/audio.h"
#include "game/hu3d.h"
#include "humath.h"

extern double sin(double);
extern double cos(double);

extern s32 lbl_1_bss_C;
extern s32 lbl_1_bss_24;
extern M668PlayerWork *lbl_1_bss_2C[4];
extern const HuVecF lbl_1_rodata_30C;
extern const HuVecF lbl_1_rodata_318;
extern const HuVecF lbl_1_rodata_324;

extern void fn_1_2A4(HuVecF *, f32, f32, f32);
extern void fn_1_434(HuVecF *, const HuVecF *, const HuVecF *);
extern void fn_1_46C(HuVecF *, const HuVecF *, const HuVecF *);
extern void fn_1_4A4(HuVecF *, const HuVecF *, const HuVecF *);
extern f32 fn_1_4DC(HuVecF *, const HuVecF *);
extern void fn_1_6BC(HuVecF *, const HuVecF *, f32);
extern f32 fn_1_404(const HuVecF *, const HuVecF *);
extern s32 fn_1_1844(const M668VecBound *, const M668VecBound *);

static inline M668PlayerWork *fn_1_18B4(s32 index) { return lbl_1_bss_2C[index]; }
static inline s16 *fn_1_18CC(s32 index) { return lbl_1_bss_28[index]; }

const TeamTemplate lbl_1_rodata_68 = {{-1, -1, -1, -1}};
const TeamTemplate lbl_1_rodata_78 = {{0, 0, 0, 0}};

void fn_1_18E4(s32 out[4])
{
    TeamTemplate teamPlayer;
    TeamTemplate teamCount;
    s32 player;

    teamPlayer = lbl_1_rodata_68;
    teamCount = lbl_1_rodata_78;
    for (player = 0; player < 4; player++) {
        teamPlayer.group[GwPlayerConf[player].grpNo] = player;
        teamCount.group[GwPlayerConf[player].grpNo]++;
    }

    if ((teamPlayer.group[0] == -1) || (teamPlayer.group[1] == -1) ||
        (teamPlayer.group[2] != -1) || (teamPlayer.group[3] != -1) ||
        (teamCount.group[0] != 2) || (teamCount.group[1] != 2)) {
        for (player = 0; player < 4; player++) {
            out[player] = player / 2;
        }
        return;
    }

    for (player = 0; player < 4; player++) {
        out[player] = GwPlayerConf[player].grpNo;
    }
}

const HuVecF lbl_1_rodata_88 = {-100.0f,8000.0f,0.0f};
const HuVecF lbl_1_rodata_94 = {0.0f,1.0f,0.5f};
const HuVecF lbl_1_rodata_A0 = {0.0f,0.0f,0.0f};

void fn_1_1A48(OMOBJ *obj)
{
    s32 teamOrder[4];
    HuVecF sp44, sp38, sp2C;
    s16 startOrder[2];
    s32 objIndex;
    M668StageWork *data;

    data = obj->data;
    lbl_1_bss_20 = data;
    data->streamHandle = -1;
    lbl_1_bss_644 = HuMemDirectMallocNum(HEAP_MODEL, GXGetTexBufferSize(640U, 480U, 4U, 0U, 0U), 268435456U);
    memset(lbl_1_bss_644, 0, GXGetTexBufferSize(640U, 480U, 4U, 0U, 0U));
    lbl_1_bss_640 = HuMemDirectMallocNum(HEAP_MODEL, GXGetTexBufferSize(40U, 32U, 4U, 0U, 0U), 268435456U);
    memset(lbl_1_bss_640, 0, GXGetTexBufferSize(40U, 32U, 4U, 0U, 0U));
    lbl_1_bss_3C = HuSprAnimRead(HuDataSelHeapReadNum(8323087, 268435456, HEAP_MODEL));
    lbl_1_bss_24 = -1;
    lbl_1_bss_18[0] = lbl_1_bss_18[1] = 0;
    fn_1_18E4(teamOrder);

    sp44 = lbl_1_rodata_88;
    sp38 = lbl_1_rodata_94;
    sp2C = lbl_1_rodata_A0;
    Hu3DShadowCreate((20.0f), (10.0f), (10000.0f));
    Hu3DShadowColSet(64U, 0U, 0U);
    Hu3DShadowPosSet(&sp44, &sp38, &sp2C);
    Hu3DShadowTPLvlSet((0.5f));
    fn_1_178(10, 8U, fn_1_6EC8);
    fn_1_178(5, 38U, fn_1_3858);
    fn_1_178(10, 2U, fn_1_4508);

    startOrder[0] = rand8() % 2;
    startOrder[1] = rand8() % 2;
    objIndex = 0;
    while (objIndex < 4) {
        lbl_1_bss_2C[objIndex] = fn_1_178(30, 84U, fn_1_494C);
        lbl_1_bss_2C[objIndex]->playerNo = objIndex;
        lbl_1_bss_2C[objIndex]->teamNo = (s16)teamOrder[objIndex];
        lbl_1_bss_2C[objIndex]->formationNo = (s16)(startOrder[teamOrder[objIndex]] + (teamOrder[objIndex] * 2));
        startOrder[teamOrder[objIndex]] = (startOrder[teamOrder[objIndex]] + 1) % 2;
        objIndex++;
    }
    objIndex = 0;
    while (objIndex < 1) {
        lbl_1_bss_28[objIndex] = fn_1_178(20, 88U, fn_1_72CC);
        *lbl_1_bss_28[objIndex] = objIndex;
        objIndex++;
    }
    CharEffectLayerSet(7);
    HuAudFXPlay(2189);
    fn_1_218(obj, fn_1_1FC4);
}

void fn_1_1FC4(OMOBJ *arg0)
{
    M668StageWork *work = arg0->data;

    if (lbl_1_bss_20->state == 0) {
        if ((work->streamHandle == -1) &&
            ((s32)(GameMesStatGet(MgSeqGameMesIdGet()) & 16) != 0)) {
            work->streamHandle = HuAudSStreamPlay(74);
        }
        if (MgSeqModeGet() == 5) {
            fn_1_218(arg0, fn_1_2064);
            return;
        }
    }
}

void fn_1_2064(OMOBJ *arg0)
{
    M668StageWork *sp8;
    s32 var_r31;

    sp8 = arg0->data;
    if ((lbl_1_bss_20->state == 0) && ((lbl_1_bss_18[0] != 0) || (lbl_1_bss_18[1] != 0))) {
        if ((lbl_1_bss_18[0] != 0) && (lbl_1_bss_18[1] != 0)) {
            lbl_1_bss_24 = rand8() % 2;
        } else {
            if (lbl_1_bss_18[0] != 0) {
                var_r31 = 1;
            } else {
                var_r31 = 0;
            }
            lbl_1_bss_24 = var_r31;
        }
        MgSeqModeNext();
        fn_1_218(arg0, fn_1_216C);
        return;
    }
}

const M668TeamTemplate lbl_1_rodata_BC = {{-1, -1, -1, -1}};

void fn_1_216C(OMOBJ *arg0)
{
    M668TeamTemplate winner;
    s32 var_r29;
    s32 var_r31;
    M668StageWork *temp_r30;

    temp_r30 = arg0->data;
    winner = lbl_1_rodata_BC;
    var_r29 = 0;
    if (lbl_1_bss_20->state == 0) {
        var_r31 = 0;
        while (var_r31 < 4) {
            if (lbl_1_bss_24 == lbl_1_bss_2C[var_r31]->teamNo) {
                winner.group[var_r29] = (s32) lbl_1_bss_2C[var_r31]->charNo;
                if (_CheckFlag(65551U) == 0) {
                    GwPlayer[var_r31].mgCoinBonus = 10;
                }
                var_r29 += 1;
            }
            var_r31 += 1;
        }
        MgSeqWinnerSet((s16) winner.group[0], (s16) winner.group[1],
            (s16) winner.group[2], (s16) winner.group[3]);
        if (temp_r30->streamHandle != -1) {
            HuAudSStreamFadeOut((s32) temp_r30->streamHandle, 100);
        }
        fn_1_218(arg0, fn_1_22D4);
    }
}

void fn_1_22D4(OMOBJ *arg0)
{
    M668StageWork *sp8;

    sp8 = arg0->data;
    if ((lbl_1_bss_20->state == 0) && (MgSeqModeGet() == 7)) {
        MgSeqModeChangeOff();
        fn_1_218(arg0, fn_1_2340);
        return;
    }
}

void fn_1_2340(OMOBJ *arg0)
{
    M668StageWork *sp8;

    sp8 = arg0->data;
    if ((lbl_1_bss_20->state == 0) && (lbl_1_bss_8 == 1)) {
        MgSeqModeChangeOn();
        MgSeqModeNext();
        fn_1_218(arg0, NULL);
        return;
    }
}

const GXColor lbl_1_rodata_CC = {0,0,0,255};

void fn_1_23B4(s16 layerNo)
{
    Mtx position;
    Mtx44 projection;
    /* Retail performs this entry color copy; drawing uses independent colors. */
    const GXColor color = lbl_1_rodata_CC;

    GXSetViewport((0.0f), (0.0f), (640.0f), (480.0f), (0.0f), (1.0f));
    GXSetScissor(0U, 0U, 640U, 480U);
    C_MTXOrtho(projection, (0.0f), (1.0f), (0.0f), (1.0f), (0.0f), (1.0f));
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0U);
    PSMTXIdentity(position);
    GXLoadPosMtxImm(position, 0U);
    GXSetNumChans(1U);
    GXSetChanCtrl(GX_COLOR0, 0U, GX_SRC_REG, GX_SRC_REG, 0U, GX_DF_NONE, GX_AF_NONE);
    GXSetCullMode(GX_CULL_NONE);
    GXSetColorUpdate(1U);
    GXSetAlphaUpdate(1U);
    GXSetAlphaCompare(GX_ALWAYS, 255U, GX_AOP_OR, GX_ALWAYS, 255U);
    GXSetZMode(0U, GX_LESS, 0U);
    GXSetNumChans(1U);
    GXSetChanCtrl(GX_COLOR0, 0U, GX_SRC_REG, GX_SRC_VTX, 0U, GX_DF_NONE, GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0U);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0U);
    GXSetNumTexGens(0U);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetNumTevStages(1U);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ZERO, GX_LO_NOOP);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4U);
    GXPosition3f32((0.0f), (0.0f), (0.0f));
    GXColor4u8(0U, 0U, 0U, 255U);
    GXPosition3f32((1.0f), (0.0f), (0.0f));
    GXColor4u8(0U, 0U, 0U, 255U);
    GXPosition3f32((1.0f), (1.0f), (0.0f));
    GXColor4u8(0U, 0U, 0U, 255U);
    GXPosition3f32((0.0f), (1.0f), (0.0f));
    GXColor4u8(0U, 0U, 0U, 255U);
}

const GXColor lbl_1_rodata_E0 = {0,0,0,255};

void fn_1_27D0(s16 layerNo)
{
    Mtx position;
    Mtx44 projection;
    GXTexObj texture;
    /* Retail performs this entry color copy; drawing uses independent colors. */
    const GXColor color = lbl_1_rodata_E0;
    u8 white;

    GXSetTexCopySrc(0U, 0U, 640U, 480U);
    GXSetTexCopyDst(640U, 480U, GX_TF_RGB565, 0U);
    GXCopyTex(lbl_1_bss_644, 1U);
    GXSetViewport((0.0f), (0.0f), (640.0f), (480.0f), (0.0f), (1.0f));
    GXSetScissor(0U, 0U, 640U, 480U);
    C_MTXOrtho(projection, (0.0f), (1.0f), (0.0f), (1.0f), (0.0f), (1.0f));
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0U);
    PSMTXIdentity(position);
    GXLoadPosMtxImm(position, 0U);
    GXSetNumChans(1U);
    GXSetChanCtrl(GX_COLOR0, 0U, GX_SRC_REG, GX_SRC_REG, 0U, GX_DF_NONE, GX_AF_NONE);
    GXSetCullMode(GX_CULL_NONE);
    GXSetColorUpdate(1U);
    GXSetAlphaUpdate(0U);
    GXSetZCompLoc(1U);
    GXSetAlphaCompare(GX_GREATER, 0U, GX_AOP_AND, GX_LEQUAL, 255U);
    GXSetZMode(0U, GX_LESS, 0U);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0U);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0U);
    GXSetNumTexGens(1U);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 60U, 0U, 125U);
    GXInitTexObj(&texture, lbl_1_bss_644, 640U, 480U, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, 0U);
    GXInitTexObjLOD(&texture, GX_LINEAR, GX_LINEAR, (0.0f), (0.0f), (0.0f), 0U, 0U, GX_ANISO_1);
    GXLoadTexObj(&texture, GX_TEXMAP0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetNumTevStages(1U);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4U);
    GXPosition3f32((0.0f), (0.0f), (0.0f));
    GXTexCoord2f32((0.0f), (0.0f));
    GXPosition3f32((0.0625f), (0.0f), (0.0f));
    GXTexCoord2f32((1.0f), (0.0f));
    GXPosition3f32((0.0625f), (0.06666667014360428f), (0.0f));
    GXTexCoord2f32((1.0f), (1.0f));
    GXPosition3f32((0.0f), (0.06666667014360428f), (0.0f));
    GXTexCoord2f32((0.0f), (1.0f));
    GXSetNumChans(1U);
    GXSetChanCtrl(GX_COLOR0, 0U, GX_SRC_REG, GX_SRC_REG, 0U, GX_DF_NONE, GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0U);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0U);
    GXSetNumTexGens(0U);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 60U, 0U, 125U);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetNumTevStages(1U);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    white = 255U;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4U);
    GXPosition3f32((0.0f), (0.0f), (0.0f));
    GXColor4u8(white, white, white, white);
    GXPosition3f32((0.0625f), (0.0f), (0.0f));
    GXColor4u8(white, white, white, white);
    GXPosition3f32((0.0625f), (0.06666667014360428f), (0.0f));
    GXColor4u8(white, white, white, white);
    GXPosition3f32((0.0f), (0.06666667014360428f), (0.0f));
    GXColor4u8(white, white, white, white);
    Hu3DFbCopyExec(0, 0, 40, 32, GX_TF_RGB565, 0, lbl_1_bss_640);
}

void fn_1_2FA0(s16 layerNo)
{
    f32 sp21C[97];
    f32 sp98[97];
    Mtx sp68;
    Mtx44 sp28;
    GXTexObj sp8;
    s32 var_r31;

    if (lbl_1_data_AC != 0) {
        GXSetViewport((0.0f), (0.0f), (640.0f), (480.0f), (0.0f), (1.0f));
        GXSetScissor(0U, 0U, 640U, 480U);
        C_MTXOrtho(sp28, (0.0f), (1.0f), (0.0f), (1.0f), (0.0f), (1.0f));
        GXSetProjection(sp28, GX_ORTHOGRAPHIC);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0U);
        PSMTXIdentity(sp68);
        GXLoadPosMtxImm(sp68, 0U);
        GXSetNumChans(1U);
        GXSetChanCtrl(GX_COLOR0, 0U, GX_SRC_REG, GX_SRC_REG, 0U, GX_DF_NONE, GX_AF_NONE);
        GXSetCullMode(GX_CULL_NONE);
        GXSetColorUpdate(1U);
        GXSetAlphaUpdate(0U);
        GXSetZCompLoc(1U);
        GXSetAlphaCompare(GX_GREATER, 0U, GX_AOP_AND, GX_LEQUAL, 255U);
        GXSetZMode(0U, GX_LESS, 0U);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0U);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0U);
        GXSetNumTexGens(1U);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 60U, 0U, 125U);
        GXInitTexObj(&sp8, lbl_1_bss_640, 40U, 32U, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, 0U);
        GXInitTexObjLOD(&sp8, GX_LINEAR, GX_LINEAR, (0.0f), (0.0f), (0.0f), 0U, 0U, GX_ANISO_1);
        GXLoadTexObj(&sp8, GX_TEXMAP0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetNumTevStages(1U);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
        memset(sp21C, 0, sizeof(sp21C));
        if (omPauseChk() == 0) {
            lbl_1_bss_10 += 1;
        }
        var_r31 = 0;
        while (var_r31 < 97) {
            sp98[var_r31] = (f32)(((1.5) * sin((4.0) * ((0.02617993877991494) * (f64)(f32)(lbl_1_bss_10 + var_r31)))) / (640.0));
            sp21C[var_r31] = -m668_fabs32((f32)(((4.0) * cos((0.039269908169872414) * (f64)(f32)(lbl_1_bss_10 + (var_r31 * 4)))) / (480.0)));
            var_r31 += 1;
        }
        GXBegin(GX_QUADS, GX_VTXFMT0, 384U);
        var_r31 = 0;
        while (var_r31 < 96) {
            GXPosition3f32(sp98[var_r31 + 1], ((0.010416666977107525f) * (f32)(var_r31 + 1)) + sp21C[var_r31 + 1], (0.0f));
            GXTexCoord2f32((0.0f), (f32)(var_r31 + 1) / (96.0f));
            GXPosition3f32((1.0f) + sp98[var_r31 + 1], ((0.010416666977107525f) * (f32)(var_r31 + 1)) + sp21C[var_r31 + 1], (0.0f));
            GXTexCoord2f32((1.0f), (f32)(var_r31 + 1) / (96.0f));
            GXPosition3f32((1.0f) + sp98[var_r31], ((0.010416666977107525f) * (f32)var_r31) + sp21C[var_r31], (0.0f));
            GXTexCoord2f32((1.0f), (f32)var_r31 / (96.0f));
            GXPosition3f32(sp98[var_r31], ((0.010416666977107525f) * (f32)var_r31) + sp21C[var_r31], (0.0f));
            GXTexCoord2f32((0.0f), (f32)var_r31 / (96.0f));
            var_r31 += 1;
        }
    }
}

void fn_1_3858(OMOBJ *obj)
{
    M668SceneWork *work = obj->data;
    HuVecF lightDirection;
    HU3D_LIGHTID light;

    work->collisionModel[0] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 3), HU_MEMNUM_OVL, HEAP_MODEL));
    work->collisionModel[1] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 5), HU_MEMNUM_OVL, HEAP_MODEL));
    work->collisionModel[2] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 1), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(work->collisionModel[0], 1U);
    Hu3DModelAttrSet(work->collisionModel[1], 1U);
    Hu3DModelAttrSet(work->collisionModel[2], 1U);
    MgActorColMapInit(work->collisionModel, 3, 128);

    work->sceneModel[0] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 0), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(work->sceneModel[0], 0);
    work->sceneModel[1] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 2), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(work->sceneModel[1], 0);
    Hu3DLayerHookSet(8, fn_1_23B4);

    work->actorModel[0] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 4), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(work->actorModel[0], 1);
    work->actorMotion[0] = Hu3DJointMotion(work->actorModel[0], HuDataSelHeapReadNum(DATANUM(DATA_m668, 6), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(work->actorModel[0], work->actorMotion[0]);
    Hu3DModelAttrSet(work->actorModel[0], HU3D_MOTATTR_LOOP);
    work->actorModel[1] = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 7), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(work->actorModel[1], 1);
    work->actorMotion[1] = Hu3DJointMotion(work->actorModel[1], HuDataSelHeapReadNum(DATANUM(DATA_m668, 8), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(work->actorModel[1], work->actorMotion[1]);
    Hu3DModelAttrSet(work->actorModel[1], HU3D_MOTATTR_LOOP);
    Hu3DLayerHookSet(9, fn_1_27D0);

    work->sceneLink[0] = Hu3DModelLink(work->sceneModel[0]);
    Hu3DModelLayerSet(work->sceneLink[0], 2);
    work->sceneLink[1] = Hu3DModelLink(work->sceneModel[1]);
    Hu3DModelLayerSet(work->sceneLink[1], 2);
    work->actorLink[0] = Hu3DModelLink(work->actorModel[0]);
    Hu3DMotionSet(work->actorLink[0], work->actorMotion[0]);
    Hu3DModelLayerSet(work->actorLink[0], 5);
    Hu3DModelAttrSet(work->actorLink[0], HU3D_MOTATTR_LOOP);
    work->actorLink[1] = Hu3DModelLink(work->actorModel[1]);
    Hu3DMotionSet(work->actorLink[1], work->actorMotion[1]);
    Hu3DModelLayerSet(work->actorLink[1], 5);
    Hu3DModelAttrSet(work->actorLink[1], HU3D_MOTATTR_LOOP);

    work->effect[0].model = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 11), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(work->effect[0].model, 2);
    work->effect[0].motion = Hu3DJointMotion(work->effect[0].model, HuDataSelHeapReadNum(DATANUM(DATA_m668, 12), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(work->effect[0].model, work->effect[0].motion);
    Hu3DModelAttrSet(work->effect[0].model, HU3D_MOTATTR_LOOP);
    work->effect[1].model = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 13), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(work->effect[1].model, 6);
    work->effect[1].motion = Hu3DJointMotion(work->effect[1].model, HuDataSelHeapReadNum(DATANUM(DATA_m668, 14), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(work->effect[1].model, work->effect[1].motion);
    Hu3DModelAttrSet(work->effect[1].model, HU3D_MOTATTR_LOOP);
    Hu3DLayerHookSet(5, fn_1_2FA0);
    Hu3DLayerHookSet(6, fn_1_2FA0);

    fn_1_2A4(&lightDirection, (0.0f), (-1000.0f), (-1000.0f));
    fn_1_4DC(&lightDirection, &lightDirection);
    light = Hu3DGLightCreate((0.0f), (1000.0f), (1000.0f),
                             lightDirection.x, lightDirection.y, lightDirection.z,
                             255U, 255U, 255U);
    Hu3DGLightStaticSet(light, 1);
    Hu3DGLightInfinitytSet(light);
    Hu3DAmbColorSet((0.75f), (0.75f), (0.75f));
    Hu3DModelShadowMapSet(work->sceneLink[1]);
    Hu3DModelShadowMapSet(work->actorLink[0]);
    fn_1_218(obj, fn_1_3CB8);
}

void fn_1_3CB8(OMOBJ *obj)
{
    void *work = obj->data;
    if (lbl_1_bss_20->state != 0) {
        return;
    }
}

const GXColor lbl_1_rodata_13C = {255,255,255,255};

void fn_1_3CE4(s16 layerNo)
{
    Mtx sp74;
    Mtx44 sp34;
    GXTexObj sp14;
    HuVec2f texOffset;
    GXColor color;
    f32 temp_f30;
    f32 temp_f29;
    f32 halfSize;
    s32 i;

    GXSetTexCopySrc(0U, 0U, 640U, 480U);
    GXSetTexCopyDst(640U, 480U, GX_TF_RGB565, 0U);
    GXCopyTex(lbl_1_bss_644, 0U);
    GXSetViewport((0.0f), (0.0f), (640.0f),
                  (480.0f), (0.0f), (1.0f));
    GXSetScissor(0U, 0U, 640U, 480U);
    C_MTXOrtho(sp34, (0.0f), (1.0f),
               (0.0f), (1.0f),
               (0.0f), (1.0f));
    GXSetProjection(sp34, GX_ORTHOGRAPHIC);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0U);
    PSMTXIdentity(sp74);
    GXLoadPosMtxImm(sp74, 0U);
    GXSetNumChans(1U);
    GXSetChanCtrl(GX_COLOR0, 0U, GX_SRC_REG, GX_SRC_REG, 0U, GX_DF_NONE, GX_AF_NONE);
    GXSetCullMode(GX_CULL_NONE);
    GXSetColorUpdate(1U);
    GXSetAlphaUpdate(0U);
    GXSetZCompLoc(1U);
    GXSetAlphaCompare(GX_ALWAYS, 255U, GX_AOP_OR, GX_ALWAYS, 255U);
    GXSetZMode(0U, GX_LESS, 0U);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX1, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0U);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0U);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_F32, 0U);
    GXSetNumTexGens(2U);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 60U, 0U, 125U);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, 60U, 0U, 125U);
    color = lbl_1_rodata_13C;
    HuSprTexLoad(lbl_1_bss_3C, 0, 0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    GXInitTexObj(&sp14, lbl_1_bss_644, 640U, 480U, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, 0U);
    GXInitTexObjLOD(&sp14, GX_LINEAR, GX_LINEAR,
                    (0.0f), (0.0f), (0.0f),
                    0U, 0U, GX_ANISO_1);
    GXLoadTexObj(&sp14, GX_TEXMAP1);
    GXSetNumTevStages(2U);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    if (omPauseChk() == 0) {
        lbl_1_bss_14 += 1;
    }
    GXBegin(GX_QUADS, GX_VTXFMT0, 256U);
    i = 0;
    while (i < 64) {
        halfSize = (0.125f);
        temp_f30 = (f32)((f64)lbl_1_bss_340[i].x +
                         ((0.10000000149011612) *
                          sin((0.017453292519943295) * (f64)(lbl_1_bss_14 + (i * 5)))));
        temp_f29 = lbl_1_bss_340[i].y;

        if (omPauseChk() == 0) {
            lbl_1_bss_340[i].y -= (0.004000000189989805f);
        }
        if (lbl_1_bss_340[i].y < -halfSize) {
            lbl_1_bss_340[i].y = (1.0f) + halfSize;
        }

        texOffset.x = lbl_1_bss_40[i].x;
        texOffset.y = lbl_1_bss_40[i].y;
        GXPosition3f32(temp_f30, temp_f29, (0.0f));
        GXTexCoord2f32((0.0f), (0.0f));
        GXTexCoord2f32(temp_f30 + texOffset.x, temp_f29 + texOffset.y);
        GXPosition3f32(temp_f30 + halfSize, temp_f29, (0.0f));
        GXTexCoord2f32((1.0f), (0.0f));
        GXTexCoord2f32(texOffset.x + (temp_f30 + halfSize), temp_f29 + texOffset.y);
        GXPosition3f32(temp_f30 + halfSize, temp_f29 + halfSize, (0.0f));
        GXTexCoord2f32((1.0f), (1.0f));
        GXTexCoord2f32(texOffset.x + (temp_f30 + halfSize), texOffset.y + (temp_f29 + halfSize));
        GXPosition3f32(temp_f30, temp_f29 + halfSize, (0.0f));
        GXTexCoord2f32((0.0f), (1.0f));
        GXTexCoord2f32(temp_f30 + texOffset.x, texOffset.y + (temp_f29 + halfSize));
        i += 1;
    }
}

void fn_1_4508(OMOBJ *obj)
{
    void *sp8;
    s32 index;

    sp8 = obj->data;
    if (lbl_1_bss_20->state == 0) {
        for (index = 0; index < 64; index++) {
            lbl_1_bss_340[index].x = fn_1_16B4((-0.125f), (1.125f));
            lbl_1_bss_340[index].y = fn_1_16B4((-0.125f), (1.125f));
            lbl_1_bss_340[index].z = (0.0f);
            lbl_1_bss_40[index].x = fn_1_16B4((-0.004999999888241291f), (0.004999999888241291f));
            lbl_1_bss_40[index].y = fn_1_16B4((-0.004999999888241291f), (0.004999999888241291f));
            lbl_1_bss_40[index].z = (0.0f);
        }
        Hu3DCameraLayerHookSet(1, 15, fn_1_3CE4);
        fn_1_218(obj, NULL);
    }
}

static inline f32 wrap_degrees(f32 angle)
{
    return (f32)fmod((f64)angle, (360.0));
}

void fn_1_468C(M668PlayerWork *work, f32 desired, f32 blend, f32 speed)
{
    HuVecF desiredDir;
    HuVecF currentDir;
    f32 angle;
    f32 current;
    f32 difference;
    f32 cross;

    work->player->actor->rotY = wrap_degrees(work->player->actor->rotY);
    current = (f32)((3.141592653589793) * (f64)(work->player->actor->rotY / (180.0f)));
    fn_1_2A4(&desiredDir, (f32)sin((f64)desired), (0.0f), (f32)cos((f64)desired));
    fn_1_2A4(&currentDir, (f32)sin((f64)current), (0.0f), (f32)cos((f64)current));
    difference = fn_1_220(fn_1_404(&desiredDir, &currentDir));
    cross = desiredDir.x * currentDir.z - desiredDir.z * currentDir.x;
    angle = current;
    if (cross >= (0.0f)) {
        angle += difference * blend;
    } else if (cross < (0.0f)) {
        angle -= difference * blend;
    }
    while (angle > (6.283185307179586)) {
        angle = (f32)((f64)angle - (6.283185307179586));
    }
    while (angle < (-6.283185307179586)) {
        angle += (6.283185307179586);
    }
    work->player->actor->rotY = (f32)((f64)((180.0f) * angle) / (3.141592653589793));
    fn_1_2A4(&work->player->actor->push, (0.0f), (0.0f), speed);
}

const MGACTOR_PARAM lbl_1_rodata_198 = {0.0f,0.0f,0,0,NULL,0,0,NULL};

void fn_1_494C(OMOBJ *obj)
{
    M668PlayerWork *work = obj->data;
    MGACTOR_PARAM param;
    s32 player;
    M668PlayerWork *peer;
    f32 sign;

    work->charNo = GwPlayerConf[work->playerNo].charNo;
    work->state = 0;
    work->transitionFlag = 0;
    for (player = 0; player < 4; player++) {
        if (player != work->playerNo) {
            peer = lbl_1_bss_2C[player];
            if (work->teamNo == peer->teamNo) {
                work->partnerNo = (s16)player;
            }
        }
    }
    work->origin = lbl_1_data_64[work->formationNo];
    param = lbl_1_rodata_198;
    param.height = (150.0f);
    param.radius = (40.0f);
    param.param = work->playerNo;
    param.type = 0;
    param.attr = 0;
    param.correctHookParam = 0;
    param.narrowHook = NULL;
    param.correctHook = NULL;
    work->player = MgPlayerCreate(work->playerNo, &param, 2, 1U, 35U, lbl_1_data_3C);
    MgPlayerComStkOn(work->player);
    MgPlayerVibrateCreate(work->player);
    Hu3DModelLayerSet((s16)work->player->actor->mdlId, 7);
    MgActorColMaskSet(work->player->actor, 7U);
    Hu3DModelShadowSet((s16)work->player->actor->mdlId);
    work->position[1] = lbl_1_data_64[work->formationNo];
    work->position[0] = work->position[1];
    MgActorPosSet(work->player->actor, &work->position[0]);
    if (rand8() % 2) {
        sign = (-1.0f);
    } else {
        sign = (1.0f);
    }
    work->sign = (s32)sign;
    fn_1_218(obj, fn_1_4BFC);
}

void fn_1_4BFC(OMOBJ *obj)
{
    void *work = obj->data;
    if (lbl_1_bss_20->state == 0 && MgSeqModeGet() == 2) {
        fn_1_218(obj, fn_1_4C64);
        return;
    }
}

static inline f32 M668SqrtTarget(f32 x)
{
    /* SDK sqrtf rounds the Newton result through a volatile float store. */
    volatile f32 y;
    if (x > (0.0f)) {
        f64 guess = __frsqrte((f64)x);
        guess = (0.5) * guess * ((3.0) - guess * guess * x);
        guess = (0.5) * guess * ((3.0) - guess * guess * x);
        guess = (0.5) * guess * ((3.0) - guess * guess * x);
        y = (f32)(x * guess);
        return y;
    }
    return x;
}

void fn_1_4C64(OMOBJ *obj)
{
    M668PlayerWork *work = obj->data;
    f32 velocity;

    if (lbl_1_bss_20->state == 0) {
        if (work->transitionFlag != 0 && MgPlayerModeAttrCheck(work->player, MGPLAYER_MODEATTR_AIR) == 0) {
            if (work->player->actor->gravity) {
                velocity = work->player->actor->gravity +
                           (work->player->actor->gravity *
                            (((-1.0f) +
                              M668SqrtTarget((1.0f) +
                                             ((8.0f) *
                                              ((30000.0f) / work->player->actor->gravity)))) / 2.0f));
            } else {
                velocity = (0.0f);
            }
            MgActorVelYSet(work->player->actor, velocity);
        }
        if (MgSeqModeGet() == 5) {
            work->state = 1;
            if (GwPlayerConf[work->playerNo].type != 0) {
                fn_1_218(obj, fn_1_54CC);
                return;
            }
            MgPlayerComStkOff(work->player);
            fn_1_218(obj, fn_1_4EE8);
            return;
        }
    }
}

const HuVecF lbl_1_rodata_1E0 = {0.0f,-1.0f,0.0f};
const HuVecF lbl_1_rodata_1EC = {0.0f,1.0f,0.0f};

void fn_1_4EE8(OMOBJ *obj)
{
    HuVecF template1;
    HuVecF template2;
    Point3d sp30;
    Point3d sp24;
    Point3d sp18;
    Point3d spC;
    int sp8;
    M668PlayerWork *temp_r31;
    f32 temp_f31;
    f32 var_f30;
    M668RotorWork *temp_r20;

    temp_r31 = obj->data;
    if (lbl_1_bss_20->state == 0) {
        if ((s32) lbl_1_bss_24 != -1) {
            MgPlayerComStkOn(temp_r31->player);
        }
        if ((s32) lbl_1_bss_C != 0) {
            fn_1_218(obj, fn_1_6C24);
            return;
        }
        temp_r31->position[1] = temp_r31->position[0];
        MgActorPosGet(temp_r31->player->actor, temp_r31->position);
        if ((MgActorColMeshGet(temp_r31->player->actor, &sp8) != 0) && (sp8 == 1)) {
            temp_r31->state = 3;
            MgPlayerDespawn(temp_r31->player);
            if ((fn_1_18B4(temp_r31->partnerNo)->state != 1) && ((s32) lbl_1_bss_24 == -1)) {
                lbl_1_bss_18[temp_r31->teamNo] = 1;
            }
            fn_1_218(obj, fn_1_6778);
            return;
        }
        if (temp_r31->state == 2) {
            template1 = lbl_1_rodata_1E0;
            template2 = lbl_1_rodata_1EC;
            fn_1_2A4(&sp24, (0.0f), (0.0f), (0.0f));
            MgActorPosGet(temp_r31->player->actor, &sp30);
            fn_1_4A4(&sp18, &sp30, &sp24);
            fn_1_4DC(&sp18, &sp18);
            fn_1_434(&temp_r31->position[2], &sp18, &template1);
            fn_1_46C(&temp_r31->position[2], &temp_r31->position[2], &template2);
            fn_1_4DC(&temp_r31->position[2], &temp_r31->position[2]);
            fn_1_46C(&temp_r31->position[2], &temp_r31->position[2], &template2);
            fn_1_4DC(&temp_r31->position[2], &temp_r31->position[2]);
            temp_r20 = (M668RotorWork *)fn_1_18CC(temp_r31->playerSelector);
            if (temp_r20->speedScale < (1.0f)) {
                var_f30 = (1.0f);
            } else {
                var_f30 = temp_r20->speedScale;
            }
            fn_1_6BC(&temp_r31->position[2], &temp_r31->position[2], (20.0f) * var_f30);
            temp_r31->position[2].x *= (f32)(temp_r20->speed > (0.0f) ? 1 : -1);
            temp_r31->position[2].z *= (f32)(temp_r20->speed > (0.0f) ? 1 : -1);
            if (temp_r31->position[2].z > (0.0f)) {
                temp_r31->position[2].z = -temp_r31->position[2].z;
            }
            MgPlayerDespawn(temp_r31->player);
            CharMotionShiftSet(temp_r31->charNo, temp_r31->player->omObj->mtnId[5], (0.0f), (4.0f), 0U);
            Hu3DModelRotSet((s16) temp_r31->player->actor->mdlId, (0.0f), (f32) ((f64) ((180.0f) * M668Atan2f(temp_r31->position[2].x, temp_r31->position[2].z)) / (3.141592653589793)), (0.0f));
            if ((fn_1_18B4(temp_r31->partnerNo)->state != 1) && ((s32) lbl_1_bss_24 == -1)) {
                lbl_1_bss_18[temp_r31->teamNo] = 1;
            }
            omVibrate(temp_r31->playerNo, 20, 30, 0);
            Hu3D3Dto2D(&sp30, 1, &spC);
            temp_f31 = (64.0f) * (spC.x / (576.0f));
            temp_f31 = temp_f31 < (0.0f) ? (0.0f) : (temp_f31 > (64.0f) ? (64.0f) : temp_f31);
            temp_f31 += (32.0f);
            HuAudFXPlayPan(2188, (s16) temp_f31);
            CharFXPlayPan(temp_r31->charNo, 576, (s16) temp_f31);
            fn_1_218(obj, fn_1_664C);
            return;
        }
    }
}

const M668FloatTable4 lbl_1_rodata_208 = {{7.0f,5.0f,3.0f,1.0f}};
const M668IntTable4 lbl_1_rodata_218 = {{19,21,23,25}};
const HuVecF lbl_1_rodata_228 = {0.0f,0.0f,0.0f};
const M668FloatTable4 lbl_1_rodata_234 = {{300.0f,270.0f,250.0f,220.0f}};
const M668FloatTable4 lbl_1_rodata_244 = {{80.0f,64.0f,32.0f,0.0f}};
const HuVecF lbl_1_rodata_254 = {0.0f,-1.0f,0.0f};
const HuVecF lbl_1_rodata_260 = {0.0f,1.0f,0.0f};

void fn_1_54CC(OMOBJ *obj)
{
    M668FloatTable4 sp1B4;
    Point3d sp1A8;
    Point3d sp19C;
    Point3d sp190;
    Point3d sp184;
    Point3d sp178;
    M668IntTable4 sp168;
    Point3d sp15C;
    Point3d sp150;
    Point3d sp144;
    Point3d sp138;
    Point3d sp12C;
    M668FloatTable4 sp11C;
    M668FloatTable4 sp10C;
    Point3d sp100;
    Point3d spF4;
    Point3d spE8;
    Point3d spDC;
    Point3d spD0;
    Point3d spC4;
    int sp90;
    f32 sp8C;
    f32 sp88;
    f32 sp84;
    f32 sp80;
    M668RotorWork *sp2C;
    M668RotorWork *sp20;
    M668PlayerWork *temp_r31;
    f32 temp_f24;
    f32 temp_f28;
    s16 temp_r26;
    s32 var_r28;

    temp_r31 = obj->data;
    temp_r26 = GwPlayerConf[temp_r31->playerNo].comDif;
    sp1B4 = lbl_1_rodata_208;
    if (lbl_1_bss_20->state == 0) {
        if ((s32) lbl_1_bss_C != 0) {
            fn_1_218(obj, fn_1_6C24);
            return;
        }
        temp_r31->position[1] = temp_r31->position[0];
        MgActorPosGet(temp_r31->player->actor, temp_r31->position);
        if (((100.0f) * frandf()) < (1.0f)) {
            temp_r31->sign = -temp_r31->sign;
        }
        if ((s32) lbl_1_bss_24 != -1) {
            MgPlayerComStkOn(temp_r31->player);
        } else if (MgPlayerModeAttrCheck(temp_r31->player, 16U) == 0) {
            fn_1_4A4(&sp1A8, temp_r31->position,
                     fn_1_18B4(temp_r31->partnerNo)->position);
            sp8C = fn_1_4DC(&sp1A8, &sp1A8);
            if (sp8C <= (100.0f)) {
                fn_1_4A4(&sp19C, &lbl_1_data_94[temp_r31->teamNo], temp_r31->position);
                fn_1_4DC(&sp19C, &sp19C);
                fn_1_46C(&sp1A8, &sp1A8, &sp19C);
                fn_1_468C(temp_r31, M668Atan2f(sp1A8.x, sp1A8.z), (0.10000000149011612f), (7.0f));
            } else {
                fn_1_2A4(&sp184, (0.0f), (f32) temp_r31->sign, (0.0f));
                fn_1_4A4(&sp190, &lbl_1_data_94[temp_r31->teamNo], temp_r31->position);
                sp88 = fn_1_4DC(&sp190, &sp190);
                fn_1_434(&sp178, &sp184, &sp190);
                fn_1_6BC(&sp190, &sp190, -((200.0f) - sp88) / (7.0f));
                fn_1_46C(&sp178, &sp178, &sp190);
                fn_1_4DC(&sp178, &sp178);
                fn_1_468C(temp_r31, M668Atan2f(sp178.x, sp178.z), (0.10000000149011612f), (7.0f));
            }
        }
        if (MgPlayerModeAttrCheck(temp_r31->player, 16U) == 0) {
            temp_r31->transitionFlag = 0;
            var_r28 = 0;
            while (var_r28 < 1) {
                sp168 = lbl_1_rodata_218;
                sp2C = (M668RotorWork *)fn_1_18CC(var_r28);
                sp84 = (f32) ((0.017453292519943295) * (f64) (sp2C->speed * sp2C->speedScale));
                temp_f24 = sp84 * (f32) sp168.value[temp_r26];
                sp15C = lbl_1_rodata_228;
                fn_1_2A4(&sp150, (f32)sin(sp2C->angle), (0.0f), (f32)cos(sp2C->angle));
                fn_1_2A4(&sp144, (f32)sin(sp2C->angle + temp_f24), (0.0f), (f32)cos(sp2C->angle + temp_f24));
                MgActorPosGet(temp_r31->player->actor, &sp138);
                if (!(sp138.y > (150.0f)) && (sp138.y = (0.0f), fn_1_4A4(&sp12C, &sp138, &sp15C), fn_1_4DC(&sp12C, &sp12C), sp80 = sp2C->speed * ((sp150.x * sp12C.z) - (sp150.z * sp12C.x)), (sp80 < (0.0f))) && (fn_1_404(&sp150, &sp12C) > (0.0f)) && (fn_1_404(&sp150, &sp12C) >= fn_1_404(&sp150, &sp144))) {
                    temp_r31->transitionFlag = 1;
                } else {
                    var_r28 += 1;
                    continue;
                }
                break;
            }
            if (temp_r31->transitionFlag != 0) {
                sp11C = lbl_1_rodata_234;
                sp10C = lbl_1_rodata_244;
                if (((100.0f) * frandf()) >= sp10C.value[temp_r26]) {
                    MgActorVelYSet(temp_r31->player->actor,
                        temp_r31->player->actor->gravity ?
                        temp_r31->player->actor->gravity +
                        temp_r31->player->actor->gravity * (((-1.0f) +
                        M668SqrtTarget((1.0f) + ((8.0f) * (((100.0f) * sp11C.value[temp_r26]) /
                        temp_r31->player->actor->gravity)))) / 2.0f) : (0.0f));
                }
            }
        }
        if ((MgActorColMeshGet(temp_r31->player->actor, &sp90) != 0) && (sp90 == 1)) {
            temp_r31->state = 3;
            MgPlayerDespawn(temp_r31->player);
            if ((fn_1_18B4(temp_r31->partnerNo)->state != 1) &&
                ((s32) lbl_1_bss_24 == -1)) {
                lbl_1_bss_18[temp_r31->teamNo] = 1;
            }
            fn_1_218(obj, fn_1_6778);
            return;
        }
        if (temp_r31->state == 2) {
            sp100 = lbl_1_rodata_254;
            spF4 = lbl_1_rodata_260;
            fn_1_2A4(&spDC, (0.0f), (0.0f), (0.0f));
            MgActorPosGet(temp_r31->player->actor, &spE8);
            fn_1_4A4(&spD0, &spE8, &spDC);
            fn_1_4DC(&spD0, &spD0);
            fn_1_434(&temp_r31->position[2], &spD0, &sp100);
            fn_1_46C(&temp_r31->position[2], &temp_r31->position[2], &spF4);
            fn_1_4DC(&temp_r31->position[2], &temp_r31->position[2]);
            fn_1_46C(&temp_r31->position[2], &temp_r31->position[2], &spF4);
            fn_1_4DC(&temp_r31->position[2], &temp_r31->position[2]);
            sp20 = (M668RotorWork *)fn_1_18CC(temp_r31->playerSelector);
            fn_1_6BC(&temp_r31->position[2], &temp_r31->position[2],
                (20.0f) * (sp20->speedScale < (1.0f) ? (1.0f) : sp20->speedScale));
            temp_r31->position[2].x *= (f32)(sp20->speed > (0.0f) ? 1 : -1);
            temp_r31->position[2].z *= (f32)(sp20->speed > (0.0f) ? 1 : -1);
            if (temp_r31->position[2].z > (0.0f)) {
                temp_r31->position[2].z = -temp_r31->position[2].z;
            }
            MgPlayerDespawn(temp_r31->player);
            CharMotionShiftSet(temp_r31->charNo, temp_r31->player->omObj->mtnId[5], (0.0f), (4.0f), 0U);
            Hu3DModelRotSet((s16) temp_r31->player->actor->mdlId, (0.0f),
                (f32)((f64)((180.0f) * M668Atan2f(temp_r31->position[2].x, temp_r31->position[2].z)) / (3.141592653589793)), (0.0f));
            if ((fn_1_18B4(temp_r31->partnerNo)->state != 1) &&
                ((s32) lbl_1_bss_24 == -1)) {
                lbl_1_bss_18[temp_r31->teamNo] = 1;
            }
            Hu3D3Dto2D(&spE8, 1, &spC4);
            temp_f28 = (64.0f) * (spC4.x / (576.0f));
            temp_f28 = temp_f28 < (0.0f)
                           ? (0.0f)
                           : (temp_f28 > (64.0f) ? (64.0f) : temp_f28);
            temp_f28 += (32.0f);
            HuAudFXPlayPan(2188, (s16) temp_f28);
            CharFXPlayPan(temp_r31->charNo, 576, (s16) temp_f28);
            fn_1_218(obj, fn_1_664C);
            return;
        }
    }
}

void fn_1_664C(OMOBJ *obj)
{
    M668PlayerWork *work = obj->data;
    HuVecF position;

    if (lbl_1_bss_20->state == 0) {
        if (lbl_1_bss_C != 0) {
            fn_1_218(obj, fn_1_6C24);
            return;
        }
        Hu3DModelAttrReset(work->player->actor->mdlId, 1U);
        Hu3DModelPosGet(work->player->actor->mdlId, &position);
        if (position.y <= (-200.0f)) {
            position.y = (-200.0f);
            Hu3DModelPosSetV(work->player->actor->mdlId, &position);
            work->state = 3;
            fn_1_218(obj, fn_1_6778);
            return;
        }
        fn_1_46C(&position, &position, &work->position[2]);
        Hu3DModelPosSetV(work->player->actor->mdlId, &position);
    }
}

void fn_1_6778(OMOBJ *obj)
{
    M668PlayerWork *work = obj->data;
    HuVecF modelPosition;
    HuVecF zeroVector;
    HuVecF direction;

    if (lbl_1_bss_20->state == 0) {
        Hu3DModelAttrReset(work->player->actor->mdlId, 1U);
        if (lbl_1_bss_C != 0) {
            fn_1_218(obj, fn_1_6C24);
            return;
        }
        fn_1_2A4(&zeroVector, (0.0f), (0.0f), (0.0f));
        Hu3DModelPosGet(work->player->actor->mdlId, &modelPosition);
        fn_1_4A4(&direction, &modelPosition, &zeroVector);
        fn_1_4DC(&direction, &direction);
        CharMotionShiftSet(work->charNo, work->player->omObj->mtnId[8], (0.0f), (0.0f), 1073741825U);
        work->position[2].x = (7.0f) * direction.x;
        work->position[2].y = (30.0f);
        work->position[2].z = (7.0f) * direction.z;
        if (work->position[2].z > (0.0f)) {
            work->position[2].z = -work->position[2].z;
        }
        omVibrate(work->playerNo, 30, 30, 0);
        fn_1_218(obj, fn_1_6924);
    }
}

void fn_1_6924(OMOBJ *obj)
{
    HuVecF modelPosition;
    HuVecF normal;
    HuVecF reflectionCopy; /* Purpose unknown: target stores the full copy but does not read it. */
    M668PlayerWork *work;
    f32 distance;
    f32 savedY;
    f32 magnitude; /* Purpose unknown: target stores the normalization result but does not read it. */
    f32 scale;
    s32 i;

    work = obj->data;
    if (lbl_1_bss_20->state == 0) {
        Hu3DModelAttrReset(work->player->actor->mdlId, 1U);
        if (lbl_1_bss_C != 0) {
            fn_1_218(obj, fn_1_6C24);
            return;
        }
        Hu3DModelPosGet(work->player->actor->mdlId, &modelPosition);
        fn_1_46C(&modelPosition, &modelPosition, &work->position[2]);
        i = 0;
        while (i < 2) {
            fn_1_4A4(&normal, &modelPosition, &lbl_1_data_94[i]);
            distance = fn_1_4DC(&normal, &normal);
            if (distance <= (350.0f)) {
                savedY = work->position[2].y;
                work->position[2].y = (0.0f);
                magnitude = fn_1_4DC(&work->position[2], &work->position[2]);
                reflectionCopy = work->position[2];
                scale = m668_fabs32((2.0f) * fn_1_404(&work->position[2], &normal));
                work->position[2].x += work->position[2].x + normal.x * scale;
                work->position[2].y = (0.0f);
                work->position[2].z += work->position[2].z + normal.z * scale;
                fn_1_4DC(&work->position[2], &work->position[2]);
                fn_1_6BC(&work->position[2], &work->position[2], (7.0f));
                work->position[2].y += savedY;
                fn_1_6BC(&normal, &normal, (351.0f));
                fn_1_46C(&modelPosition, &lbl_1_data_94[i], &normal);
            } else {
                i += 1;
                continue;
            }
            break;
        }
        Hu3DModelPosSetV(work->player->actor->mdlId, &modelPosition);
        if ((modelPosition.y < (-180.0f)) && (work->position[2].y < (0.0f))) {
            work->position[2].y = (0.800000011920929f) * -work->position[2].y;
        }
    }
}

void fn_1_6C24(OMOBJ *obj)
{
    M668PlayerFinishWork *work = obj->data;

    if (lbl_1_bss_20->state == 0) {
        work->timer60 = 0;
        MgPlayerDespawn(work->player);
        Hu3DModelPosSetV(work->player->actor->mdlId, &lbl_1_data_64[work->formationNo]);
        Hu3DModelRotSet(work->player->actor->mdlId, (0.0f), (0.0f), (0.0f));
        Hu3DModelAttrReset(work->player->actor->mdlId, 1U);
        Hu3DModelScaleSet(work->player->actor->mdlId, (1.0f), (1.0f), (1.0f));
        CharMotionShiftSet(work->charNo, *work->player->omObj->mtnId, (0.0f), (0.0f), 1073741825U);
        Hu3DModelScaleSet(work->player->actor->mdlId, (1.0f), (1.0f), (1.0f));
        work->timer60 = 0;
        if (lbl_1_bss_24 == work->teamNo) {
            fn_1_218(obj, fn_1_6DF8);
            return;
        }
        Hu3DModelAttrSet(work->player->actor->mdlId, 1U);
        fn_1_218(obj, NULL);
    }
}

void fn_1_6DF8(OMOBJ *obj)
{
    M668PlayerFinishWork *work = obj->data;

    if (lbl_1_bss_20->state == 0 && work->timer60++ >= 60) {
        CharMotionShiftSet(work->charNo, work->player->omObj->mtnId[6], (0.0f), (4.0f), 0);
        fn_1_218(obj, NULL);
        return;
    }
}

void fn_1_6E9C(OMOBJ *obj)
{
    void *work = obj->data;
    if (lbl_1_bss_20->state != 0) {
        return;
    }
}

void fn_1_6EC8(OMOBJ *obj)
{
    HuVecF target;
    HuVecF position;
    HuVecF up;
    M668CameraWork *work = obj->data;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, (40.0f), (20.0f), (15000.0f), (1.2000000476837158f));
    Hu3DCameraViewportSet(1, (0.0f), (0.0f), (640.0f), (480.0f), (0.0f), (1.0f));
    work->motion = Hu3DMotionCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 10), HU_MEMNUM_OVL, HEAP_MODEL));
    work->model = Hu3DModelCameraCreate(work->motion, 1);
    Hu3DCameraPosGet(1, &position, &up, &target);
    Hu3DModelKill(work->model);
    Hu3DCameraPosSetV(1, &position, &up, &target);
    work->model = Hu3DModelCameraCreate(work->motion, 1);
    Hu3DCameraMotionStart(work->model, 1);
    fn_1_218(obj, fn_1_7014);
}

void fn_1_7014(OMOBJ *obj)
{
    M668CameraWork *work = obj->data;
    if (lbl_1_bss_20->state == 0 && MgSeqModeGet() == 7) {
        Hu3DModelKill(work->model);
        WipeCreate(2, 0, 60);
        work->timer = 0;
        fn_1_218(obj, fn_1_70A0);
        return;
    }
}

const M668CameraPair lbl_1_rodata_2A0 = {{{-550.0f,200.0f,950.0f},{550.0f,200.0f,950.0f}}};
const M668CameraPair lbl_1_rodata_2B8 = {{{-550.0f,200.0f,0.0f},{550.0f,200.0f,0.0f}}};
const HuVecF lbl_1_rodata_2D0 = {0.0f,1.0f,0.0f};

void fn_1_70A0(OMOBJ *obj)
{
    M668CameraWork *work = obj->data;
    if (lbl_1_bss_20->state == 0) {
        if (work->timer++ >= 60) {
            M668CameraPair position = lbl_1_rodata_2A0;
            M668CameraPair target = lbl_1_rodata_2B8;
            HuVecF up = lbl_1_rodata_2D0;
            Hu3DCameraPerspectiveSet(1, (40.0f), (20.0f), (15000.0f), (1.2000000476837158f));
            Hu3DCameraPosSetV(1, &position.points[lbl_1_bss_24], &up, &target.points[lbl_1_bss_24]);
            WipeCreate(1, 5, 60);
            work->timer = 0;
            lbl_1_bss_C = 1;
            fn_1_218(obj, fn_1_7240);
            return;
        }
    }
}

void fn_1_7240(OMOBJ *obj)
{
    M668CameraWork *work = obj->data;
    if (lbl_1_bss_20->state == 0) {
        if (work->timer++ >= 60) {
            lbl_1_bss_8++;
            fn_1_218(obj, NULL);
            return;
        }
    }
}

void fn_1_72CC(OMOBJ *obj)
{
    M668RotorWork *work = obj->data;
    s32 index;

    if (lbl_1_bss_20->state == 0) {
        work->speedScale = (1.0f);
        work->speed = (1.5f);
        if (rand8() % 2 == 1) {
            work->speed *= (-1.0f);
        }

        work->model = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 9), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelLayerSet(work->model, 0);
        Hu3DModelPosSet(work->model, (0.0f), (0.0f), (0.0f));
        work->shadowModel = Hu3DModelLink(work->model);
        Hu3DModelLayerSet(work->shadowModel, 2);
        Hu3DModelLayerSet(work->shadowModel, 6);

        work->angle = (f32)((0.017453292519943295) * (f64)((10.0f) * (f32)work->index));
        for (index = 0; index < 16; index++) {
            work->history[index] = work->angle;
        }

        Hu3DModelRotSet(work->model, (0.0f),
                        (f32)((f64)((180.0f) * work->angle) / (3.141592653589793)),
                        (0.0f));
        Hu3DModelRotSet(work->shadowModel, (0.0f),
                        (f32)((f64)((180.0f) * work->angle) / (3.141592653589793)),
                        (0.0f));
        Hu3DModelShadowSet(work->shadowModel);
        fn_1_218(obj, fn_1_7508);
    }
}

void fn_1_7508(OMOBJ *obj)
{
    M668RotorWork *work = obj->data;
    if (lbl_1_bss_20->state == 0 && MgSeqModeGet() == 2) {
        work->sound = HuAudFXPlay(2190);
        HuAudFXPitchSet(work->sound, 2000);
        fn_1_218(obj, fn_1_758C);
        return;
    }
}

const HuVecF lbl_1_rodata_2E0 = {0.0f,1.0f,0.0f};
const HuVecF lbl_1_rodata_2EC = {0.0f,-1.0f,0.0f};
const HuVecF lbl_1_rodata_2F8 = {0.0f,0.0f,0.0f};

void fn_1_758C(OMOBJ *obj)
{
    M668RotorWork *work = obj->data;
    HuVecF up;
    HuVecF down;
    HuVecF begin1;
    HuVecF begin2;
    HuVecF radial;
    M668VecBound edge1;
    M668VecBound edge2;
    HuVecF center;
    HuVecF fromCenter;
    M668VecBound objectBound;
    f32 delta = (f32)((0.017453292519943295) * (f64)(work->speed * work->speedScale));
    f32 arcStep;
    f32 sweepAngle;
    M668PlayerWork *item;
    s32 index;
    s32 playerIndex;

    if (lbl_1_bss_20->state != 0) {
        return;
    }

    for (index = 15; index >= 1; index--) {
        work->history[index] = work->history[index - 1];
    }
    work->history[0] = work->angle;
    work->angle += delta;
    Hu3DModelRotSet(work->model, (0.0f),
                    (f32)((f64)((180.0f) * work->angle) / (3.141592653589793)),
                    (0.0f));
    Hu3DModelRotSet(work->shadowModel, (0.0f),
                    (f32)((f64)((180.0f) * work->angle) / (3.141592653589793)),
                    (0.0f));

    up = lbl_1_rodata_2E0;
    down = lbl_1_rodata_2EC;
    center = lbl_1_rodata_2F8;
    arcStep = (25.0f) * delta;
    if (delta > (0.0f)) {
        fn_1_2A4(&begin1, (f32)sin(work->angle), (0.0f), (f32)cos(work->angle));
        fn_1_2A4(&begin2, (f32)sin(work->angle + arcStep), (0.0f), (f32)cos(work->angle + arcStep));
    } else {
        fn_1_2A4(&begin2, (f32)sin(work->angle), (0.0f), (f32)cos(work->angle));
        fn_1_2A4(&begin1, (f32)sin(work->angle + arcStep), (0.0f), (f32)cos(work->angle + arcStep));
    }
    fn_1_434(&edge1.xyz, &begin1, &up);
    fn_1_434(&edge2.xyz, &begin2, &down);
    edge1.margin = (25.0f);
    edge2.margin = (25.0f);

    sweepAngle = work->angle + (0.5f * arcStep);
    fn_1_2A4(&radial, (f32)sin(sweepAngle), (0.0f), (f32)cos(sweepAngle));
    fn_1_6BC(&radial, &radial, (-100.0f));
    fn_1_46C(&center, &center, &radial);
    fn_1_4DC(&radial, &radial);

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        item = fn_1_18B4(playerIndex);
        objectBound.xyz = item->position[0];
        objectBound.margin = (40.0f);
        fn_1_4A4(&fromCenter, &center, &item->position[0]);
        fn_1_4DC(&fromCenter, &fromCenter);
        if (fn_1_404(&fromCenter, &radial) > (0.0f) &&
            fn_1_1844(&edge1, &objectBound) != 0 &&
            fn_1_1844(&edge2, &objectBound) != 0 &&
            item->position[0].y < (100.0f)) {
            item->transitionFlag = 1;
        } else {
            item->transitionFlag = 0;
        }
    }

    if (work->angle >= (6.283185307179586) || work->angle <= (-6.283185307179586)) {
        for (index = 15; index >= 1; index--) {
            work->history[index] = work->history[index - 1];
        }
        work->history[0] = work->angle;
        work->angle = (0.0f);
        Hu3DModelRotSet(work->model, (0.0f),
                        (f32)((f64)((180.0f) * work->angle) / (3.141592653589793)),
                        (0.0f));
        Hu3DModelRotSet(work->shadowModel, (0.0f),
                        (f32)((f64)((180.0f) * work->angle) / (3.141592653589793)),
                        (0.0f));
        HuAudFXStop(work->sound);
        fn_1_218(obj, fn_1_7B30);
        return;
    }
}

void fn_1_7B30(OMOBJ *obj)
{
    s32 index;
    M668RotorWork *work = obj->data;
    if (lbl_1_bss_20->state == 0) {
        for (index = 15; index >= 1; index--) {
            work->history[index] = work->history[index - 1];
        }
        work->history[0] = work->angle;
        if (MgSeqModeGet() == 5) {
            work->sound = HuAudFXPlay(2190);
            fn_1_218(obj, fn_1_7BE8);
            return;
        }
    }
}

const HuVecF lbl_1_rodata_30C = {0.0f,1.0f,0.0f};
const HuVecF lbl_1_rodata_318 = {0.0f,-1.0f,0.0f};
const HuVecF lbl_1_rodata_324 = {0.0f,0.0f,0.0f};

void fn_1_7BE8(OMOBJ *obj)
{
    M668RotorWork *work = obj->data;
    HuVecF up;
    HuVecF down;
    HuVecF begin1;
    HuVecF begin2;
    HuVecF radial;
    M668VecBound edge1;
    M668VecBound edge2;
    HuVecF center;
    HuVecF fromCenter;
    M668VecBound objectBound;
    f32 delta;
    f32 sweepAngle;
    f32 theta;
    s16 pitch;
    s32 historyIndex;
    s32 playerIndex;
    M668PlayerWork *item;

    if (lbl_1_bss_20->state == 0) {
        delta = (f32)((0.017453292519943295) * (f64)(work->speed * work->speedScale));
        if (lbl_1_bss_24 == -1) {
            work->speedScale += (0.004000000189989805f);
        } else {
            work->speedScale = (f32)(work->speedScale + (-work->speedScale / (10.0f)));
            if (work->speedScale < (0.0f)) {
                work->speedScale = (0.0f);
            }
        }
        if (work->speedScale > (0.009999999776482582f)) {
            pitch = (s16)((2000.0f) + ((1000.0f) * work->speedScale));
            if (pitch > 8192) {
                pitch = 8192;
            }
            HuAudFXPitchSet(work->sound, pitch);
        } else if (work->sound != -1) {
            HuAudFXStop(work->sound);
            work->sound = -1;
        }

        historyIndex = 15;
        while (historyIndex >= 1) {
            work->history[historyIndex] = work->history[historyIndex - 1];
            historyIndex -= 1;
        }
        work->history[0] = work->angle;
        work->angle = (f32)(work->angle + delta);
        Hu3DModelRotSet(work->model, (0.0f),
                        (f32)((f64)((180.0f) * work->angle) / (3.141592653589793)),
                        (0.0f));
        Hu3DModelRotSet(work->shadowModel, (0.0f),
                        (f32)((f64)((180.0f) * work->angle) / (3.141592653589793)),
                        (0.0f));

        up = lbl_1_rodata_30C;
        down = lbl_1_rodata_318;
        center = lbl_1_rodata_324;
        sweepAngle = (3.0f) * delta;
        if (delta > (0.0f)) {
            fn_1_2A4(&begin1, (f32)sin(work->angle), (0.0f),
                     (f32)cos(work->angle));
            fn_1_2A4(&begin2, (f32)sin(work->angle + sweepAngle), (0.0f),
                     (f32)cos(work->angle + sweepAngle));
        } else {
            fn_1_2A4(&begin2, (f32)sin(work->angle), (0.0f),
                     (f32)cos(work->angle));
            fn_1_2A4(&begin1, (f32)sin(work->angle + sweepAngle), (0.0f),
                     (f32)cos(work->angle + sweepAngle));
        }
        fn_1_434(&edge1.xyz, &begin1, &up);
        fn_1_434(&edge2.xyz, &begin2, &down);
        edge1.margin = (25.0f);
        edge2.margin = (25.0f);

        theta = work->angle + (0.5f * sweepAngle);
        fn_1_2A4(&radial, (f32)sin(theta), (0.0f), (f32)cos(theta));
        fn_1_6BC(&radial, &radial, (-100.0f));
        fn_1_46C(&center, &center, &radial);
        fn_1_4DC(&radial, &radial);

        playerIndex = 0;
        while (playerIndex < 4) {
            item = fn_1_18B4(playerIndex);
            if (item->state == 1) {
                objectBound.xyz = item->position[0];
                objectBound.margin = (40.0f);
                fn_1_4A4(&fromCenter, &center, &item->position[0]);
                fn_1_4DC(&fromCenter, &fromCenter);
                if ((fn_1_404(&fromCenter, &radial) > (0.0f)) &&
                    (fn_1_1844(&edge1, &objectBound) != 0) &&
                    (fn_1_1844(&edge2, &objectBound) != 0) &&
                    (item->position[0].y < (100.0f))) {
                    item->state = 2;
                    item->playerSelector = work->index;
                }
            }
            playerIndex += 1;
        }

        if (lbl_1_bss_C != 0) {
            Hu3DModelAttrSet(work->model, 1U);
            Hu3DModelAttrSet(work->shadowModel, 1U);
            fn_1_218(obj, NULL);
            return;
        }
    }
}
