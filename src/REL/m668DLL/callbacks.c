/* Burnstile stage callbacks, team results, camera wipes, and rotor hazards. */
#include "dolphin/math.h"
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

#define M668_SE_ROTOR_LOOP 2190
#define M668_SE_ROTOR_HIT 2188
#define M668_SE_STAGE_START 2189
#define M668_STREAM_STAGE_MUSIC 74

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

extern void *fn_1_178(s32 priority, u32 dataSize, OMOBJ_FUNC callback);
extern void fn_1_218(OMOBJ *obj, OMOBJ_FUNC callback);
extern void fn_1_1FC4(OMOBJ *obj);
extern void fn_1_6EC8(OMOBJ *obj);
extern void fn_1_3858(OMOBJ *obj);
extern /* Object callback created by fn_1_1A48; initializes shared particle positions and texture
 * offsets, installs their camera-layer draw hook, then disables this callback. */
void fn_1_4508(OMOBJ *obj);
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

/* Models, actor links, effects, and collision models used by the stage. */
typedef struct M668SceneWork_s {
    HU3D_MODELID sceneModel[2];
    HU3D_MODELID sceneLink[2];
    HU3D_MODELID actorModel[2];
    HU3D_MODELID actorLink[2];
    HU3D_MOTIONID actorMotion[2];
    u8 unreferencedBytes[4]; /* The stage starts these bytes at zero and never updates them. */
    M668EffectModel effect[2];
    HU3D_MODELID collisionModel[3];
} M668SceneWork;

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

#include "game/mg/actman.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/hu3d.h"
#include "humath.h"

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
extern double atan2(double, double);

static inline f32 M668Atan2f(f32 y, f32 x)
{
    return (f32)atan2((f64)y, (f64)x);
}

extern void fn_1_2A4(HuVecF *, f32, f32, f32);
extern f32 fn_1_404(const HuVecF *, const HuVecF *);
extern f32 fn_1_220(f32);

/* Angle helpers used by player movement and launch direction below. */

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

/* Returns the magnitude used to keep the wave overlay vertical offset nonpositive. */
static inline f32 M668Abs(f32 x)
{
    return (f32)fabs((f64)x);
}

#include "REL/m668DLL/rotor.h"
#include "game/mg/actman.h"

/* Player state used while characters wait for the result sequence. */
typedef struct M668PlayerFinishWork_s {
    s16 playerNo;
    s16 partnerNo;
    s16 charNo;
    s16 teamNo;
    s16 formationNo;
    MGPLAYER *player;
    s16 state;
    HuVecF position[3];
    s16 hitRotorIndex; /* Rotor index stored when this player is hit. */
    s32 transitionFlag;
    s32 timer60;
} M668PlayerFinishWork;

extern s32 lbl_1_bss_24;
extern HuVecF lbl_1_data_64[4];
void fn_1_6DF8(OMOBJ *obj);

#include "REL/m668DLL/rotor.h"

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
#include "dolphin/math.h"

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
#include "dolphin/math.h"
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

/* Select the configured two-team split during stage setup, falling back to player pairs. */
void fn_1_18E4(s32 out[4])
{
    /* Called during stage setup to use the configured two-team split, falling back to player pairs
     * otherwise. */
    TeamTemplate playerByTeam;
    TeamTemplate playersPerTeam;
    s32 playerIndex;

    playerByTeam = lbl_1_rodata_68;
    playersPerTeam = lbl_1_rodata_78;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        playerByTeam.group[GwPlayerConf[playerIndex].grpNo] = playerIndex;
        playersPerTeam.group[GwPlayerConf[playerIndex].grpNo]++;
    }

    if ((playerByTeam.group[0] == -1) || (playerByTeam.group[1] == -1) ||
        (playerByTeam.group[2] != -1) || (playerByTeam.group[3] != -1) ||
        (playersPerTeam.group[0] != 2) || (playersPerTeam.group[1] != 2)) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            out[playerIndex] = playerIndex / 2;
        }
        return;
    }

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        out[playerIndex] = GwPlayerConf[playerIndex].grpNo;
    }
}

const HuVecF lbl_1_rodata_88 = {-100.0f,8000.0f,0.0f};
const HuVecF lbl_1_rodata_94 = {0.0f,1.0f,0.5f};
const HuVecF lbl_1_rodata_A0 = {0.0f,0.0f,0.0f};

/* Stage setup allocates screen buffers, creates players and rotors, then starts
 * the stage callback chain. */
void fn_1_1A48(OMOBJ *obj)
{
    /* Called when the minigame stage object starts; allocates its buffers and creates the player
     * and rotor objects. */
    s32 teamOrder[4];
    HuVecF shadowPosition, shadowDirection, shadowTarget;
    s16 startOrder[2];
    s32 objIndex;
    M668StageWork *stageWork;

    stageWork = obj->data;
    lbl_1_bss_20 = stageWork;
    stageWork->streamHandle = -1;
    lbl_1_bss_644 =
        HuMemDirectMallocNum(HEAP_MODEL, GXGetTexBufferSize(640U, 480U, 4U, 0U, 0U), HU_MEMNUM_OVL);
    memset(lbl_1_bss_644, 0, GXGetTexBufferSize(640U, 480U, 4U, 0U, 0U));
    lbl_1_bss_640 =
        HuMemDirectMallocNum(HEAP_MODEL, GXGetTexBufferSize(40U, 32U, 4U, 0U, 0U), HU_MEMNUM_OVL);
    memset(lbl_1_bss_640, 0, GXGetTexBufferSize(40U, 32U, 4U, 0U, 0U));
    lbl_1_bss_3C =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_m668, 15), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_24 = -1;
    lbl_1_bss_18[0] = lbl_1_bss_18[1] = 0;
    fn_1_18E4(teamOrder);

    shadowPosition = lbl_1_rodata_88;
    shadowDirection = lbl_1_rodata_94;
    shadowTarget = lbl_1_rodata_A0;
    Hu3DShadowCreate((20.0f), (10.0f), (10000.0f));
    Hu3DShadowColSet(64U, 0U, 0U);
    Hu3DShadowPosSet(&shadowPosition, &shadowDirection, &shadowTarget);
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
        lbl_1_bss_2C[objIndex]->formationNo =
            (s16) (startOrder[teamOrder[objIndex]] + (teamOrder[objIndex] * 2));
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
    HuAudFXPlay(M668_SE_STAGE_START);
    fn_1_218(obj, fn_1_1FC4);
}

/* Stage callback starts music when the start announcement sound has played, then waits for play
 * mode. */
void fn_1_1FC4(OMOBJ *obj)
{
    /* Installed by fn_1_1A48; starts music on the start announcement sound flag and waits for
     * play mode. */
    M668StageWork *work = obj->data;

    if (lbl_1_bss_20->state == 0) {
        if ((work->streamHandle == -1) &&
            ((s32)(GameMesStatGet(MgSeqGameMesIdGet()) & 0x10) != 0)) {
            work->streamHandle = HuAudSStreamPlay(M668_STREAM_STAGE_MUSIC);
        }
        if (MgSeqModeGet() == 5) {
            fn_1_218(obj, fn_1_2064);
            return;
        }
    }
}

/* After play begins, selects the result team when one or both teams are out. */
void fn_1_2064(OMOBJ *obj)
{
    /* Stage callback after play begins; selects the result team and advances to result setup. */
    M668StageWork *stageWork;
    s32 resultTeam;

    stageWork = obj->data;
    if ((lbl_1_bss_20->state == 0) && ((lbl_1_bss_18[0] != 0) || (lbl_1_bss_18[1] != 0))) {
        if ((lbl_1_bss_18[0] != 0) && (lbl_1_bss_18[1] != 0)) {
            /* If both teams are out, randomly choose one team as the winner. */
            lbl_1_bss_24 = rand8() % 2;
        } else {
            if (lbl_1_bss_18[0] != 0) {
                resultTeam = 1;
            } else {
                resultTeam = 0;
            }
            lbl_1_bss_24 = resultTeam;
        }
        MgSeqModeNext();
        fn_1_218(obj, fn_1_216C);
        return;
    }
}

const M668TeamTemplate lbl_1_rodata_BC = {{-1, -1, -1, -1}};

/* After elimination, records the winning characters and fades the stage music. */
void fn_1_216C(OMOBJ *obj)
{
    /* Stage callback after a team is eliminated; records the winning characters and fades the stage
     * music. */
    M668TeamTemplate winner;
    s32 winnerCount;
    s32 playerIndex;
    M668StageWork *stageWork;

    stageWork = obj->data;
    winner = lbl_1_rodata_BC;
    winnerCount = 0;
    if (lbl_1_bss_20->state == 0) {
        playerIndex = 0;
        while (playerIndex < 4) {
            if (lbl_1_bss_24 == lbl_1_bss_2C[playerIndex]->teamNo) {
                winner.group[winnerCount] = (s32) lbl_1_bss_2C[playerIndex]->charNo;
                if (_CheckFlag(65551U) == 0) {
                    GwPlayer[playerIndex].mgCoinBonus = 10;
                }
                winnerCount += 1;
            }
            playerIndex += 1;
        }
        MgSeqWinnerSet((s16) winner.group[0], (s16) winner.group[1],
            (s16) winner.group[2], (s16) winner.group[3]);
        if (stageWork->streamHandle != -1) {
            HuAudSStreamFadeOut((s32) stageWork->streamHandle, 100);
        }
        fn_1_218(obj, fn_1_22D4);
    }
}

/* Result callback waits for pre-winner mode, then holds the sequence during the camera change. */
void fn_1_22D4(OMOBJ *obj)
{
    M668StageWork *stageWork;

    /* Called after result setup; waits for pre-winner mode before holding the sequence for the
     * camera transition. */
    stageWork = obj->data;
    if ((lbl_1_bss_20->state == 0) && (MgSeqModeGet() == 7)) {
        MgSeqModeChangeOff();
        fn_1_218(obj, fn_1_2340);
        return;
    }
}

/* Result callback polls the camera completion flag before releasing the sequence hold. */
void fn_1_2340(OMOBJ *obj)
{
    M668StageWork *stageWork;

    /* Polls the camera completion flag, then releases the sequence hold and ends the stage
     * callback chain. */
    stageWork = obj->data;
    if ((lbl_1_bss_20->state == 0) && (lbl_1_bss_8 == 1)) {
        MgSeqModeChangeOn();
        MgSeqModeNext();
        fn_1_218(obj, NULL);
        return;
    }
}

const GXColor lbl_1_rodata_CC = {0,0,0,255};

/* Post-layer-0 render hook covers the framebuffer with an opaque black quad. */
void fn_1_23B4(s16 layerNo)
{
    /* Registered during scene setup; covers the framebuffer in black after model layer 0 draws. */
    Mtx position;
    Mtx44 projection;
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

/* Post-layer-1 hook saves the scene, draws a 40x32 copy, clears that rectangle to black with a
* full-alpha quad, then captures it. */
void fn_1_27D0(s16 layerNo)
{
    /* Registered during scene setup after model layer 1; captures the scene, draws it at 40x32,
    * clears that rectangle to black with alpha 255, then copies the black rectangle. */
    Mtx position;
    Mtx44 projection;
    GXTexObj texture;
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

/* Registered on render layers 5 and 6 during scene setup; draws the animated
 * wave overlay while its stage flag is active. */
void fn_1_2FA0(s16 layerNo)
{
    f32 waveOffsetY[97];
    f32 waveOffsetX[97];
    Mtx position;
    Mtx44 projection;
    GXTexObj texture;
    s32 segmentIndex;

    if (lbl_1_data_AC != 0) {
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
        GXInitTexObj(&texture, lbl_1_bss_640, 40U, 32U, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, 0U);
        GXInitTexObjLOD(&texture, GX_LINEAR, GX_LINEAR, (0.0f), (0.0f), (0.0f), 0U, 0U, GX_ANISO_1);
        GXLoadTexObj(&texture, GX_TEXMAP0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetNumTevStages(1U);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
        memset(waveOffsetY, 0, sizeof(waveOffsetY));
        if (omPauseChk() == 0) {
            lbl_1_bss_10 += 1;
        }
        segmentIndex = 0;
        while (segmentIndex < 97) {
            waveOffsetX[segmentIndex] =
                (f32) (((1.5) * sin((4.0) * ((0.02617993877991494) *
                                             (f64) (f32) (lbl_1_bss_10 + segmentIndex)))) /
                       (640.0));
            waveOffsetY[segmentIndex] =
                -M668Abs((f32) (((4.0) * cos((0.039269908169872414) *
                                                 (f64) (f32) (lbl_1_bss_10 + (segmentIndex * 4)))) /
                                    (480.0)));
            segmentIndex += 1;
        }
        GXBegin(GX_QUADS, GX_VTXFMT0, 384U);
        segmentIndex = 0;
        while (segmentIndex < 96) {
            GXPosition3f32(waveOffsetX[segmentIndex + 1],
                           ((0.010416666977107525f) * (f32) (segmentIndex + 1)) +
                               waveOffsetY[segmentIndex + 1],
                           (0.0f));
            GXTexCoord2f32((0.0f), (f32)(segmentIndex + 1) / (96.0f));
            GXPosition3f32((1.0f) + waveOffsetX[segmentIndex + 1],
                           ((0.010416666977107525f) * (f32) (segmentIndex + 1)) +
                               waveOffsetY[segmentIndex + 1],
                           (0.0f));
            GXTexCoord2f32((1.0f), (f32)(segmentIndex + 1) / (96.0f));
            GXPosition3f32(
                (1.0f) + waveOffsetX[segmentIndex],
                ((0.010416666977107525f) * (f32) segmentIndex) + waveOffsetY[segmentIndex], (0.0f));
            GXTexCoord2f32((1.0f), (f32)segmentIndex / (96.0f));
            GXPosition3f32(
                waveOffsetX[segmentIndex],
                ((0.010416666977107525f) * (f32) segmentIndex) + waveOffsetY[segmentIndex], (0.0f));
            GXTexCoord2f32((0.0f), (f32)segmentIndex / (96.0f));
            segmentIndex += 1;
        }
    }
}

/* Scene setup creates the arena, actor and effect models, collision map, and lighting. */
void fn_1_3858(OMOBJ *obj)
{
    M668SceneWork *work = obj->data;
    HuVecF lightDirection;
    HU3D_LIGHTID light;

    work->collisionModel[0] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 3), HU_MEMNUM_OVL, HEAP_MODEL));
    work->collisionModel[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 5), HU_MEMNUM_OVL, HEAP_MODEL));
    work->collisionModel[2] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 1), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(work->collisionModel[0], 1U);
    Hu3DModelAttrSet(work->collisionModel[1], 1U);
    Hu3DModelAttrSet(work->collisionModel[2], 1U);
    MgActorColMapInit(work->collisionModel, 3, 128);

    work->sceneModel[0] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 0), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(work->sceneModel[0], 0);
    work->sceneModel[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 2), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(work->sceneModel[1], 0);
    Hu3DLayerHookSet(8, fn_1_23B4);

    work->actorModel[0] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 4), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(work->actorModel[0], 1);
    work->actorMotion[0] =
        Hu3DJointMotion(work->actorModel[0],
                        HuDataSelHeapReadNum(DATANUM(DATA_m668, 6), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(work->actorModel[0], work->actorMotion[0]);
    Hu3DModelAttrSet(work->actorModel[0], HU3D_MOTATTR_LOOP);
    work->actorModel[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 7), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(work->actorModel[1], 1);
    work->actorMotion[1] =
        Hu3DJointMotion(work->actorModel[1],
                        HuDataSelHeapReadNum(DATANUM(DATA_m668, 8), HU_MEMNUM_OVL, HEAP_MODEL));
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

    work->effect[0].model =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 11), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(work->effect[0].model, 2);
    work->effect[0].motion =
        Hu3DJointMotion(work->effect[0].model,
                        HuDataSelHeapReadNum(DATANUM(DATA_m668, 12), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(work->effect[0].model, work->effect[0].motion);
    Hu3DModelAttrSet(work->effect[0].model, HU3D_MOTATTR_LOOP);
    work->effect[1].model =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 13), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(work->effect[1].model, 6);
    work->effect[1].motion =
        Hu3DJointMotion(work->effect[1].model,
                        HuDataSelHeapReadNum(DATANUM(DATA_m668, 14), HU_MEMNUM_OVL, HEAP_MODEL));
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

/* Scene object callback currently performs no work while the stage is active. */
void fn_1_3CB8(OMOBJ *obj)
{
    void *work = obj->data;
    if (lbl_1_bss_20->state != 0) {
        return;
    }
}

const GXColor lbl_1_rodata_13C = {255,255,255,255};

/* Camera-layer hook installed by fn_1_4508; draws 64 textured particles and advances their
 * vertical positions while the stage is not paused. */
void fn_1_3CE4(s16 layerNo)
{
    Mtx positionMatrix;
    Mtx44 projectionMatrix;
    GXTexObj textureObject;
    HuVec2f texOffset;
    GXColor color;
    f32 particleX;
    f32 particleY;
    f32 particleSize;
    s32 i;

    GXSetTexCopySrc(0U, 0U, 640U, 480U);
    GXSetTexCopyDst(640U, 480U, GX_TF_RGB565, 0U);
    GXCopyTex(lbl_1_bss_644, 0U);
    GXSetViewport((0.0f), (0.0f), (640.0f),
                  (480.0f), (0.0f), (1.0f));
    GXSetScissor(0U, 0U, 640U, 480U);
    C_MTXOrtho(projectionMatrix, (0.0f), (1.0f),
               (0.0f), (1.0f),
               (0.0f), (1.0f));
    GXSetProjection(projectionMatrix, GX_ORTHOGRAPHIC);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0U);
    PSMTXIdentity(positionMatrix);
    GXLoadPosMtxImm(positionMatrix, 0U);
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
    GXInitTexObj(&textureObject, lbl_1_bss_644, 640U, 480U, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, 0U);
    GXInitTexObjLOD(&textureObject, GX_LINEAR, GX_LINEAR,
                    (0.0f), (0.0f), (0.0f),
                    0U, 0U, GX_ANISO_1);
    GXLoadTexObj(&textureObject, GX_TEXMAP1);
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
        particleSize = (0.125f);
        particleX = (f32)((f64)lbl_1_bss_340[i].x +
                         ((0.10000000149011612) *
                          sin((0.017453292519943295) * (f64)(lbl_1_bss_14 + (i * 5)))));
        particleY = lbl_1_bss_340[i].y; /* Draw this position before advancing it. */

        if (omPauseChk() == 0) {
            lbl_1_bss_340[i].y -= (0.004000000189989805f);
        }
        if (lbl_1_bss_340[i].y < -particleSize) {
            lbl_1_bss_340[i].y = (1.0f) + particleSize;
        }

        texOffset.x = lbl_1_bss_40[i].x;
        texOffset.y = lbl_1_bss_40[i].y;
        GXPosition3f32(particleX, particleY, (0.0f));
        GXTexCoord2f32((0.0f), (0.0f));
        GXTexCoord2f32(particleX + texOffset.x, particleY + texOffset.y);
        GXPosition3f32(particleX + particleSize, particleY, (0.0f));
        GXTexCoord2f32((1.0f), (0.0f));
        GXTexCoord2f32(texOffset.x + (particleX + particleSize), particleY + texOffset.y);
        GXPosition3f32(particleX + particleSize, particleY + particleSize, (0.0f));
        GXTexCoord2f32((1.0f), (1.0f));
        GXTexCoord2f32(texOffset.x + (particleX + particleSize),
                       texOffset.y + (particleY + particleSize));
        GXPosition3f32(particleX, particleY + particleSize, (0.0f));
        GXTexCoord2f32((0.0f), (1.0f));
        GXTexCoord2f32(particleX + texOffset.x, texOffset.y + (particleY + particleSize));
        i += 1;
    }
}

/* Particle setup seeds positions and texture offsets, then installs the camera
 * draw hook. */
void fn_1_4508(OMOBJ *obj)
{
    void *objectData;
    s32 index;

    objectData = obj->data;
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

/* Turn a player toward a heading by a blend fraction and set its forward push speed. */
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

/* Player setup callback creates its character actor at the assigned team formation. */
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
    /* Player creation replaces DATANUM(DATA_mariomot, 4) with DATANUM(DATA_mariomot, 117) in the
     * shared list. */
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

/* Waits for fade-in (phase 2) before enabling setup jumps and selecting gameplay callbacks. */
void fn_1_4BFC(OMOBJ *obj)
{
    void *work = obj->data;
    if (lbl_1_bss_20->state == 0 && MgSeqModeGet() == 2) {
        fn_1_218(obj, fn_1_4C64);
        return;
    }
}

/* During setup, jumps when the initial rotor sweep flags a grounded player, then selects the human
 * or CPU handler at phase 5. */
void fn_1_4C64(OMOBJ *obj)
{
    M668PlayerWork *work = obj->data;
    f32 velocity;

    if (lbl_1_bss_20->state == 0) {
        if (work->transitionFlag != 0 &&
            MgPlayerModeAttrCheck(work->player, MGPLAYER_MODEATTR_AIR) == 0) {
            if (work->player->actor->gravity) {
                velocity =
                    work->player->actor->gravity +
                    (work->player->actor->gravity *
                     (((-1.0f) +
                       sqrtf((1.0f) +
                                      ((8.0f) * ((30000.0f) / work->player->actor->gravity)))) /
                      2.0f));
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

/* Human-player callback selected by fn_1_4C64 at sequence phase 5; handles course falls and
 * prepares the motion and direction used after a rotor hit. */
void fn_1_4EE8(OMOBJ *obj)
{
    HuVecF downwardAxis;
    HuVecF upwardAxis;
    Point3d playerPosition;
    Point3d originPosition;
    Point3d playerDirection;
    Point3d screenPosition;
    int collisionMesh;
    M668PlayerWork *playerWork;
    f32 soundPan;
    f32 rotorSpeedScale;
    M668RotorWork *rotorWork;

    playerWork = obj->data;
    if (lbl_1_bss_20->state == 0) {
        if ((s32) lbl_1_bss_24 != -1) {
            MgPlayerComStkOn(playerWork->player);
        }
        if ((s32) lbl_1_bss_C != 0) {
            fn_1_218(obj, fn_1_6C24);
            return;
        }
        playerWork->position[1] = playerWork->position[0];
        MgActorPosGet(playerWork->player->actor, playerWork->position);
        if ((MgActorColMeshGet(playerWork->player->actor, &collisionMesh) != 0) &&
            (collisionMesh == 1)) {
            playerWork->state = 3;
            MgPlayerDespawn(playerWork->player);
            if ((fn_1_18B4(playerWork->partnerNo)->state != 1) && ((s32) lbl_1_bss_24 == -1)) {
                lbl_1_bss_18[playerWork->teamNo] = 1;
            }
            fn_1_218(obj, fn_1_6778);
            return;
        }
        if (playerWork->state == 2) {
            downwardAxis = lbl_1_rodata_1E0;
            upwardAxis = lbl_1_rodata_1EC;
            fn_1_2A4(&originPosition, (0.0f), (0.0f), (0.0f));
            MgActorPosGet(playerWork->player->actor, &playerPosition);
            fn_1_4A4(&playerDirection, &playerPosition, &originPosition);
            fn_1_4DC(&playerDirection, &playerDirection);
            fn_1_434(&playerWork->position[2], &playerDirection, &downwardAxis);
            fn_1_46C(&playerWork->position[2], &playerWork->position[2], &upwardAxis);
            fn_1_4DC(&playerWork->position[2], &playerWork->position[2]);
            fn_1_46C(&playerWork->position[2], &playerWork->position[2], &upwardAxis);
            fn_1_4DC(&playerWork->position[2], &playerWork->position[2]);
            rotorWork = (M668RotorWork *)fn_1_18CC(playerWork->hitRotorIndex);
            if (rotorWork->speedScale < (1.0f)) {
                rotorSpeedScale = (1.0f);
            } else {
                rotorSpeedScale = rotorWork->speedScale;
            }
            fn_1_6BC(&playerWork->position[2], &playerWork->position[2], (20.0f) * rotorSpeedScale);
            playerWork->position[2].x *= (f32)(rotorWork->speed > (0.0f) ? 1 : -1);
            playerWork->position[2].z *= (f32)(rotorWork->speed > (0.0f) ? 1 : -1);
            /* Rotor hits send players toward negative Z regardless of the rotor spin direction. */
            if (playerWork->position[2].z > (0.0f)) {
                playerWork->position[2].z = -playerWork->position[2].z;
            }
            MgPlayerDespawn(playerWork->player);
            CharMotionShiftSet(playerWork->charNo, playerWork->player->omObj->mtnId[5], (0.0f),
                               (4.0f), 0U);
            Hu3DModelRotSet((s16) playerWork->player->actor->mdlId, (0.0f),
                            (f32) ((f64) ((180.0f) * M668Atan2f(playerWork->position[2].x,
                                                                playerWork->position[2].z)) /
                                   (3.141592653589793)),
                            (0.0f));
            if ((fn_1_18B4(playerWork->partnerNo)->state != 1) && ((s32) lbl_1_bss_24 == -1)) {
                lbl_1_bss_18[playerWork->teamNo] = 1;
            }
            omVibrate(playerWork->playerNo, 20, 30, 0);
            Hu3D3Dto2D(&playerPosition, 1, &screenPosition);
            soundPan = (64.0f) * (screenPosition.x / (576.0f));
            soundPan = soundPan < (0.0f) ? (0.0f) : (soundPan > (64.0f) ? (64.0f) : soundPan);
            soundPan += (32.0f);
            HuAudFXPlayPan(M668_SE_ROTOR_HIT, (s16) soundPan);
            CharFXPlayPan(playerWork->charNo, 576, (s16) soundPan);
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

/* Computer-player callback selected by fn_1_4C64 at sequence phase 5; steers around rotors and
 * handles contact, jumps, and falls. */
void fn_1_54CC(OMOBJ *obj)
{
    M668FloatTable4 floatTableCopy;
    Point3d partnerDirection;
    Point3d teamCenterDirection;
    Point3d teamCenterOffset;
    Point3d teamSideAxis;
    Point3d avoidanceDirection;
    M668IntTable4 lookaheadFramesByDifficulty;
    Point3d originPosition;
    Point3d rotorDirection;
    Point3d nextRotorDirection;
    Point3d actorPosition;
    Point3d radialDirection;
    M668FloatTable4 jumpHeightByDifficulty;
    M668FloatTable4 jumpRollThresholdByDifficulty;
    Point3d downwardAxis;
    Point3d upwardAxis;
    Point3d modelPosition;
    Point3d originVector;
    Point3d centerDirection;
    Point3d screenPosition;
    int collisionMesh;
    f32 partnerDistance;
    f32 teamCenterDistance;
    f32 rotorAngleStep;
    f32 rotorApproachRate;
    M668RotorWork *rotorWork;
    M668RotorWork *hitRotor;
    M668PlayerWork *playerWork;
    f32 angularStep;
    f32 soundPan;
    s16 difficulty;
    s32 rotorIndex;

    playerWork = obj->data;
    difficulty = GwPlayerConf[playerWork->playerNo].comDif;
    /* This difficulty table is copied here, but its values are unused by this callback. */
    floatTableCopy = lbl_1_rodata_208;
    if (lbl_1_bss_20->state == 0) {
        if ((s32) lbl_1_bss_C != 0) {
            fn_1_218(obj, fn_1_6C24);
            return;
        }
        playerWork->position[1] = playerWork->position[0];
        MgActorPosGet(playerWork->player->actor, playerWork->position);
        if (((100.0f) * frandf()) < (1.0f)) {
            playerWork->sign = -playerWork->sign;
        }
        if ((s32) lbl_1_bss_24 != -1) {
            MgPlayerComStkOn(playerWork->player);
        } else if (MgPlayerModeAttrCheck(playerWork->player, 16U) == 0) {
            fn_1_4A4(&partnerDirection, playerWork->position,
                     fn_1_18B4(playerWork->partnerNo)->position);
            partnerDistance = fn_1_4DC(&partnerDirection, &partnerDirection);
            if (partnerDistance <= (100.0f)) {
                fn_1_4A4(&teamCenterDirection, &lbl_1_data_94[playerWork->teamNo],
                         playerWork->position);
                fn_1_4DC(&teamCenterDirection, &teamCenterDirection);
                fn_1_46C(&partnerDirection, &partnerDirection, &teamCenterDirection);
                fn_1_468C(playerWork, M668Atan2f(partnerDirection.x, partnerDirection.z),
                          (0.10000000149011612f), (7.0f));
            } else {
                fn_1_2A4(&teamSideAxis, (0.0f), (f32) playerWork->sign, (0.0f));
                fn_1_4A4(&teamCenterOffset, &lbl_1_data_94[playerWork->teamNo],
                         playerWork->position);
                teamCenterDistance = fn_1_4DC(&teamCenterOffset, &teamCenterOffset);
                fn_1_434(&avoidanceDirection, &teamSideAxis, &teamCenterOffset);
                fn_1_6BC(&teamCenterOffset, &teamCenterOffset,
                         -((200.0f) - teamCenterDistance) / (7.0f));
                fn_1_46C(&avoidanceDirection, &avoidanceDirection, &teamCenterOffset);
                fn_1_4DC(&avoidanceDirection, &avoidanceDirection);
                fn_1_468C(playerWork, M668Atan2f(avoidanceDirection.x, avoidanceDirection.z),
                          (0.10000000149011612f), (7.0f));
            }
        }
        if (MgPlayerModeAttrCheck(playerWork->player, 16U) == 0) {
            playerWork->transitionFlag = 0;
            rotorIndex = 0;
            while (rotorIndex < 1) {
                lookaheadFramesByDifficulty = lbl_1_rodata_218;
                rotorWork = (M668RotorWork *)fn_1_18CC(rotorIndex);
                rotorAngleStep = (f32) ((0.017453292519943295) *
                                        (f64) (rotorWork->speed * rotorWork->speedScale));
                angularStep = rotorAngleStep * (f32) lookaheadFramesByDifficulty.value[difficulty];
                originPosition = lbl_1_rodata_228;
                fn_1_2A4(&rotorDirection, (f32) sin(rotorWork->angle), (0.0f),
                         (f32) cos(rotorWork->angle));
                fn_1_2A4(&nextRotorDirection, (f32) sin(rotorWork->angle + angularStep), (0.0f),
                         (f32) cos(rotorWork->angle + angularStep));
                MgActorPosGet(playerWork->player->actor, &actorPosition);
                /* Compare the radial direction with this rotor now and after one difficulty-scaled
                 * step. */
                if (!(actorPosition.y > (150.0f)) &&
                    (actorPosition.y = (0.0f),
                     fn_1_4A4(&radialDirection, &actorPosition, &originPosition),
                     fn_1_4DC(&radialDirection, &radialDirection),
                     rotorApproachRate =
                         rotorWork->speed * ((rotorDirection.x * radialDirection.z) -
                                             (rotorDirection.z * radialDirection.x)),
                     (rotorApproachRate < (0.0f))) &&
                    (fn_1_404(&rotorDirection, &radialDirection) > (0.0f)) &&
                    (fn_1_404(&rotorDirection, &radialDirection) >=
                     fn_1_404(&rotorDirection, &nextRotorDirection))) {
                    playerWork->transitionFlag = 1;
                } else {
                    rotorIndex += 1;
                    continue;
                }
                break;
            }
            if (playerWork->transitionFlag != 0) {
                jumpHeightByDifficulty = lbl_1_rodata_234;
                jumpRollThresholdByDifficulty = lbl_1_rodata_244;
                /* Difficulty sets the random threshold for jumping before predicted rotor
                 * contact. */
                if (((100.0f) * frandf()) >= jumpRollThresholdByDifficulty.value[difficulty]) {
                    MgActorVelYSet(
                        playerWork->player->actor,
                        playerWork->player->actor->gravity
                            ? playerWork->player->actor->gravity +
                                  playerWork->player->actor->gravity *
                                      (((-1.0f) +
                                        sqrtf(
                                            (1.0f) +
                                            ((8.0f) * (((100.0f) *
                                                        jumpHeightByDifficulty.value[difficulty]) /
                                                       playerWork->player->actor->gravity)))) /
                                       2.0f)
                            : (0.0f));
                }
            }
        }
        if ((MgActorColMeshGet(playerWork->player->actor, &collisionMesh) != 0) &&
            (collisionMesh == 1)) {
            playerWork->state = 3;
            MgPlayerDespawn(playerWork->player);
            if ((fn_1_18B4(playerWork->partnerNo)->state != 1) &&
                ((s32) lbl_1_bss_24 == -1)) {
                lbl_1_bss_18[playerWork->teamNo] = 1;
            }
            fn_1_218(obj, fn_1_6778);
            return;
        }
        if (playerWork->state == 2) {
            downwardAxis = lbl_1_rodata_254;
            upwardAxis = lbl_1_rodata_260;
            fn_1_2A4(&originVector, (0.0f), (0.0f), (0.0f));
            MgActorPosGet(playerWork->player->actor, &modelPosition);
            fn_1_4A4(&centerDirection, &modelPosition, &originVector);
            fn_1_4DC(&centerDirection, &centerDirection);
            fn_1_434(&playerWork->position[2], &centerDirection, &downwardAxis);
            fn_1_46C(&playerWork->position[2], &playerWork->position[2], &upwardAxis);
            fn_1_4DC(&playerWork->position[2], &playerWork->position[2]);
            fn_1_46C(&playerWork->position[2], &playerWork->position[2], &upwardAxis);
            fn_1_4DC(&playerWork->position[2], &playerWork->position[2]);
            hitRotor = (M668RotorWork *)fn_1_18CC(playerWork->hitRotorIndex);
            fn_1_6BC(&playerWork->position[2], &playerWork->position[2],
                (20.0f) * (hitRotor->speedScale < (1.0f) ? (1.0f) : hitRotor->speedScale));
            playerWork->position[2].x *= (f32)(hitRotor->speed > (0.0f) ? 1 : -1);
            playerWork->position[2].z *= (f32)(hitRotor->speed > (0.0f) ? 1 : -1);
            /* Rotor hits send CPU players toward negative Z regardless of the rotor spin
             * direction. */
            if (playerWork->position[2].z > (0.0f)) {
                playerWork->position[2].z = -playerWork->position[2].z;
            }
            MgPlayerDespawn(playerWork->player);
            CharMotionShiftSet(playerWork->charNo, playerWork->player->omObj->mtnId[5], (0.0f),
                               (4.0f), 0U);
            Hu3DModelRotSet((s16) playerWork->player->actor->mdlId, (0.0f),
                            (f32) ((f64) ((180.0f) * M668Atan2f(playerWork->position[2].x,
                                                                playerWork->position[2].z)) /
                                   (3.141592653589793)),
                            (0.0f));
            if ((fn_1_18B4(playerWork->partnerNo)->state != 1) &&
                ((s32) lbl_1_bss_24 == -1)) {
                lbl_1_bss_18[playerWork->teamNo] = 1;
            }
            Hu3D3Dto2D(&modelPosition, 1, &screenPosition);
            soundPan = (64.0f) * (screenPosition.x / (576.0f));
            soundPan = soundPan < (0.0f)
                           ? (0.0f)
                           : (soundPan > (64.0f) ? (64.0f) : soundPan);
            soundPan += (32.0f);
            HuAudFXPlayPan(M668_SE_ROTOR_HIT, (s16) soundPan);
            CharFXPlayPan(playerWork->charNo, 576, (s16) soundPan);
            fn_1_218(obj, fn_1_664C);
            return;
        }
    }
}

/* Airborne player callback applies the launch motion until the camera transition or fall limit. */
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

/* Sets the fall motion and its displacement, requests rumble, then switches to arena-edge
 * handling. */
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
        CharMotionShiftSet(work->charNo, work->player->omObj->mtnId[8], (0.0f), (0.0f),
                           HU3D_MOTATTR_LOOP);
        work->position[2].x = (7.0f) * direction.x;
        work->position[2].y = (30.0f);
        work->position[2].z = (7.0f) * direction.z;
        /* Fall setup constrains the Z displacement to zero or negative values. */
        if (work->position[2].z > (0.0f)) {
            work->position[2].z = -work->position[2].z;
        }
        omVibrate(work->playerNo, 30, 30, 0);
        fn_1_218(obj, fn_1_6924);
    }
}

/* Fall callback resolves motion against the arena edges and bounces upward below the course. */
void fn_1_6924(OMOBJ *obj)
{
    HuVecF modelPosition;
    HuVecF normal;
    HuVecF reflectionCopy; /* Purpose unknown: target stores the full copy but does not read it. */
    M668PlayerWork *work;
    f32 distance;
    f32 savedY;
    f32 magnitude; /* Purpose unknown: target stores the normalization result but does not read
                    * it. */
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
                scale = M668Abs((2.0f) * fn_1_404(&work->position[2], &normal));
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

/* During the result camera transition, returns players to formation, hides losers, and
* schedules the winners' celebration callbacks. */
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
        CharMotionShiftSet(work->charNo, *work->player->omObj->mtnId, (0.0f), (0.0f),
                           HU3D_MOTATTR_LOOP);
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

/* Winner callback starts the celebration motion on its 61st active update. */
void fn_1_6DF8(OMOBJ *obj)
{
    M668PlayerFinishWork *work = obj->data;

    if (lbl_1_bss_20->state == 0 && work->timer60++ >= 60) {
        CharMotionShiftSet(work->charNo, work->player->omObj->mtnId[6], (0.0f), (4.0f), 0);
        fn_1_218(obj, NULL);
        return;
    }
}

/* Player object callback currently has no active-stage work. */
void fn_1_6E9C(OMOBJ *obj)
{
    void *work = obj->data;
    if (lbl_1_bss_20->state != 0) {
        return;
    }
}

/* Stage setup callback creates the camera and starts its motion, which is stopped before
* the result wipe. */
void fn_1_6EC8(OMOBJ *obj)
{
    HuVecF target;
    HuVecF position;
    HuVecF up;
    M668CameraWork *work = obj->data;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, (40.0f), (20.0f), (15000.0f), (1.2000000476837158f));
    Hu3DCameraViewportSet(1, (0.0f), (0.0f), (640.0f), (480.0f), (0.0f), (1.0f));
    work->motion =
        Hu3DMotionCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 10), HU_MEMNUM_OVL, HEAP_MODEL));
    work->model = Hu3DModelCameraCreate(work->motion, 1);
    /* Save the camera pose, discard the first controller, restore the pose, then recreate the
     * controller. */
    Hu3DCameraPosGet(1, &position, &up, &target);
    Hu3DModelKill(work->model);
    Hu3DCameraPosSetV(1, &position, &up, &target);
    work->model = Hu3DModelCameraCreate(work->motion, 1);
    Hu3DCameraMotionStart(work->model, 1);
    fn_1_218(obj, fn_1_7014);
}

/* Camera callback begins the result wipe when the sequence enters pre-winner mode. */
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

/* On its 61st active update, frames the winning team and requests the second wipe. */
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

/* On its 61st active update, sets the camera completion flag so the stage controller can
 * advance. */
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

/* Creates the rotor and shadow, seeds angle history, and starts its sound handler. */
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

        work->model =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m668, 9), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelLayerSet(work->model, 0);
        Hu3DModelPosSet(work->model, (0.0f), (0.0f), (0.0f));
        work->shadowModel = Hu3DModelLink(work->model);
        Hu3DModelLayerSet(work->shadowModel, 2);
        /* The shadow link ends up on layer 6, replacing the preceding layer 2 assignment. */
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

/* Rotor callback starts its loop sound at the initial pitch, then begins rotation. */
void fn_1_7508(OMOBJ *obj)
{
    M668RotorWork *work = obj->data;
    if (lbl_1_bss_20->state == 0 && MgSeqModeGet() == 2) {
        work->sound = HuAudFXPlay(M668_SE_ROTOR_LOOP);
        HuAudFXPitchSet(work->sound, 2000);
        fn_1_218(obj, fn_1_758C);
        return;
    }
}

const HuVecF lbl_1_rodata_2E0 = {0.0f,1.0f,0.0f};
const HuVecF lbl_1_rodata_2EC = {0.0f,-1.0f,0.0f};
const HuVecF lbl_1_rodata_2F8 = {0.0f,0.0f,0.0f};

/* Advances the initial blade sweep and flags nearby players for automatic setup jumps. */
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

    /* At a full turn, shift history a second time and reset the angle to zero, discarding
     * overshoot. */
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

/* Rotor callback waits for play mode, then restarts the loop sound and ramping sweep. */
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
            work->sound = HuAudFXPlay(M668_SE_ROTOR_LOOP);
            fn_1_218(obj, fn_1_7BE8);
            return;
        }
    }
}

const HuVecF lbl_1_rodata_30C = {0.0f,1.0f,0.0f};
const HuVecF lbl_1_rodata_318 = {0.0f,-1.0f,0.0f};
const HuVecF lbl_1_rodata_324 = {0.0f,0.0f,0.0f};

/* Ramps blade speed, updates its sound, and tests active players against the sweep. */
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
        /* Compute this frame's angle step before changing speedScale; sound pitch uses the
         * updated scale. */
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
            item->hitRotorIndex = work->index;
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
