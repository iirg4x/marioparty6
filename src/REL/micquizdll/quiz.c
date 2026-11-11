/* Microphone quiz setup, player choices, and round presentation. */
#include "game/mic.h"
#include "math.h"
#include "REL/micquizdll/source_types.h"
#include "dolphin.h"
#include "dolphin/types.h"
#include "dolphin/os.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "string.h"
#include "stdio.h"
#include "humath.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm_se.h"
#include "msm_stream.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/audio.h"
#include "game/sprite.h"
#include "game/memory.h"
#include "game/data.h"
#include "game/main.h"
#include "game/pad.h"
#include "game/window.h"
#include "game/wipe.h"
#include "game/frand.h"
#include "game/charman.h"
#include "game/process.h"
#include "game/hsfex.h"
#include "game/gamemes.h"
#include "game/flag.h"
#include "game/armem.h"
#include "game/mgdata.h"
#include "game/printfunc.h"
#include "game/saveload.h"
#include "game/esprite.h"
#include "game/thpmain.h"
#include "game/mg/seqman.h"
#include "game/mg/actman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/board/object.h"
#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/tutorial.h"
#include "game/board/masu.h"
#include "game/board/player.h"
#include "dolphin/os/OSFastCast.h"
#include "game/init.h"

/* Grid cells retain the answering player and the presence of a round result. */
#define QUIZ_CELL_PLAYER0_FLAG (1 << 2)
#define QUIZ_CELL_PLAYER1_FLAG (1 << 3)
#define QUIZ_CELL_PLAYER2_FLAG (1 << 4)
#define QUIZ_CELL_PLAYER3_FLAG (1 << 5)
#define QUIZ_CELL_HAS_RESULT_FLAG (1 << 6)
#define QUIZ_CELL_NO_RESULT_FLAG (1 << 7)

/* Screen positions used by the tutorial pointer. */
typedef struct MicquizPositionData_s {
    f32 x;
    f32 y;
} MicquizPositionData;

typedef void (*VoidFunc)(void);

/* World positions for the players in each active-player layout. */
typedef struct QuizRoundView_s {
    HuVecF playerPositions[4]; /* Packed positions for active players. */
} QuizRoundView;

/* Object work controlling the full-screen color overlay and its opacity transition. */
typedef struct {
    struct {
        u8 removeObject : 1;
        u8 stopUpdating : 1;
        u8 reserved : 6;
    } flags;
    u8 targetAlpha; /* Opacity held after a fade-in, from 0 to 255. */
    s16 remainingFrames; /* Frames remaining in the transition. */
    s16 duration; /* Requested transition length in frames. */
    HU3D_MODELID cameraModel; /* Full-screen rectangle draw hook. */
    GXColor color; /* Current screen overlay color; alpha ranges from 0 to 255. */
    f32 rate; /* Signed opacity change per frame. */
} QuizOverlayWork;

/* Scene, question, and microphone display state. */
#include "REL/micquiz/quiz_common.h"

ANIMDATA *lbl_1_bss_314[11];
s16 lbl_1_bss_310;
s32 lbl_1_bss_30C;
MicquizPosition lbl_1_bss_304;
HU3D_MODELID lbl_1_bss_2FC[4];
s16 lbl_1_bss_2F4[4];
s16 lbl_1_bss_2AC[4][9];
HU3D_MODELID lbl_1_bss_2A6[3];
HU3D_MOTIONID lbl_1_bss_296[2][4];
HU3D_MODELID lbl_1_bss_294;
HU3D_MOTIONID lbl_1_bss_28C[4];
HUPROCESS *lbl_1_bss_288;
s32 lbl_1_bss_284;
s32 lbl_1_bss_280;
s32 lbl_1_bss_27C;
s32 lbl_1_bss_278;
f32 lbl_1_bss_274;
f32 lbl_1_bss_270;
f32 lbl_1_bss_26C;
f32 lbl_1_bss_268;
s32 lbl_1_bss_264;
s32 lbl_1_bss_260;
s32 lbl_1_bss_25C;
s32 lbl_1_bss_258;
HU3D_MODELID lbl_1_bss_256;
s16 lbl_1_bss_254;
s16 lbl_1_bss_252;
HU3D_MODELID lbl_1_bss_250;
HU3D_MODELID lbl_1_bss_24E;
s16 lbl_1_bss_24C;
HU3D_MODELID lbl_1_bss_24A;
s16 lbl_1_bss_248;
s16 lbl_1_bss_246;
HU3D_MODELID lbl_1_bss_244;
s16 lbl_1_bss_242;
s16 lbl_1_bss_240;
s16 lbl_1_bss_23E;
HU3D_MODELID lbl_1_bss_23C;
HU3D_MODELID lbl_1_bss_23A;
HU3D_MODELID lbl_1_bss_238;
HU3D_MODELID lbl_1_bss_236;
HU3D_MODELID lbl_1_bss_234;
HU3D_MODELID lbl_1_bss_232;
HU3D_MODELID lbl_1_bss_230;
HU3D_MODELID lbl_1_bss_224[6];
s16 lbl_1_bss_222;
HU3D_MODELID lbl_1_bss_220;
HU3D_MODELID lbl_1_bss_21E;
HU3D_MODELID lbl_1_bss_21C;
HU3D_MODELID lbl_1_bss_214[4];
HU3D_MODELID lbl_1_bss_20C[4];
HU3D_MODELID lbl_1_bss_204[4];
HU3D_MODELID lbl_1_bss_202;
HU3D_MODELID lbl_1_bss_1FA[4];
HU3D_MODELID lbl_1_bss_1F8;
HU3D_MODELID lbl_1_bss_1F0[4];
HU3D_MODELID lbl_1_bss_1EC[2];
ANIMDATA *lbl_1_bss_1E8;
HU3D_PARMANID lbl_1_bss_1E4[2];
HU3D_MOTIONID lbl_1_bss_1E2;
HU3D_MOTIONID lbl_1_bss_1E0;
HU3D_MOTIONID lbl_1_bss_1DE;
s16 lbl_1_bss_1DC;
/* These bytes are not read by this mode; their purpose is unknown. */
u8 lbl_1_bss_1DA[2];
HU3D_MODELID lbl_1_bss_1D8;
HU3D_MODELID lbl_1_bss_1D6;
HU3D_MODELID lbl_1_bss_1D4;
HU3D_MODELID lbl_1_bss_1D2;
HU3D_MODELID lbl_1_bss_1D0;
/* These bytes are not read by this mode; their purpose is unknown. */
u8 lbl_1_bss_1CA[6];
MicquizModelSlots lbl_1_bss_198[5];
s16 lbl_1_bss_166[5][5];
s16 lbl_1_bss_134[5][5];
MicquizModelSlots lbl_1_bss_102[5];
HU3D_MODELID lbl_1_bss_100;
HU3D_MODELID lbl_1_bss_FE;
HU3D_MODELID lbl_1_bss_FC;
ANIMDATA *lbl_1_bss_CC[4][3];
s16 lbl_1_bss_B4[4][3];
s16 lbl_1_bss_B2;
s16 lbl_1_bss_B0;
s16 lbl_1_bss_AE;
s16 lbl_1_bss_AC;
s16 lbl_1_bss_AA;
s16 lbl_1_bss_A8;
s32 lbl_1_bss_9C[3];
s32 lbl_1_bss_90[3];
s32 lbl_1_bss_8C;
s32 lbl_1_bss_7C[4];
s32 lbl_1_bss_78;
s32 lbl_1_bss_74;
HUPROCESS *lbl_1_bss_70;
ANIMDATA *lbl_1_bss_6C;
ANIMDATA *lbl_1_bss_68;
ANIMDATA *lbl_1_bss_64;
HU3D_MODELID lbl_1_bss_62;
HU3D_MODELID lbl_1_bss_60;
s32 lbl_1_bss_5C;
ANIMDATA *lbl_1_bss_58;
ANIMDATA *lbl_1_bss_54;
f32 lbl_1_bss_50;
s32 lbl_1_bss_4C;
/* These bytes are not read by this mode; their purpose is unknown. */
u8 lbl_1_bss_48[4];
f32 lbl_1_bss_44;
f32 lbl_1_bss_40;
f32 lbl_1_bss_3C;
f32 lbl_1_bss_38;
ANIMDATA *lbl_1_bss_34;
ANIMDATA *lbl_1_bss_30;
OMOBJ *lbl_1_bss_2C;
OMOBJ *lbl_1_bss_28;
OMOBJ *lbl_1_bss_24;
OMOBJ *lbl_1_bss_20;
s32 lbl_1_bss_1C;
f32 micQuizPauseBgScroll;
ANIMDATA *micQuizPauseBgAnim;
HUPROCESS *micQuizMicPulseProc;
HUPROCESS *micQuizIndicatorSwingProc;
MicquizResponse *micQuizResponse;
s32 micQuizTimeoutHandled;
HUPROCESS *micQuizRingProc;

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern s32 lbl_802C0248;
extern u8 MgModeWork[256];

f32 fn_1_4A68(f32 angle);
void fn_1_5B74(void);
void fn_1_5FA0(void);
void fn_1_A4CC(s32 motionNo, f32 blendFrames, u32 attr);
void fn_1_E474(void);
void fn_1_E634(HuVecF *out, f32 x, f32 y, f32 z);
void fn_1_E644(s32 row, s32 column, s32 mask);
void fn_1_E670(s32 row, s32 column, s32 mask);
u32 fn_1_E69C(s32 row, s32 column, s32 mask);
s32 fn_1_1EA90(void);
s32 fn_1_1EAFC(void);
s32 fn_1_1EB70(void);
void fn_1_5EDC(void);
void fn_1_A290(void);
void fn_1_A308(void);
void fn_1_A348(OMOBJ *obj);
void fn_1_A3B4(s32 presenterNo, s32 motionNo, f32 blendFrames, u32 attr);
void fn_1_A484(s32 presenterNo);
void fn_1_A748(s32 playerNo);
void fn_1_A790(s32 wipeType, s32 duration);
void fn_1_A7E4(s32 wipeType, s32 duration);
void fn_1_A838(OM_CAMERA_VIEW cameraView, s32 duration, u32 cameraMask);
s32 fn_1_BD9C(s32 requested);
s32 fn_1_BE04(s32 requested);
void fn_1_D6EC(HU3D_MODELID modelId, f32 x, f32 y, f32 z);
void fn_1_D870(s32 column, s32 row);
void fn_1_D8C4(s32 column, s32 row);
s32 fn_1_D94C(s32 ignored, s32 column);
s32 fn_1_D9AC(s32 threshold, s32 tableIndex);
void fn_1_DD10(s32 row, s32 column, s32 bank);
void fn_1_DD68(s32 row, s32 column, s32 bank);
void fn_1_DDC4(void);
void fn_1_19D5C(void);
void fn_1_19DD4(s32 cameraIndex, s32 firstCapture, s32 showTransition);
void fn_1_1A210(void);
void fn_1_1A270(void);
void fn_1_1A6E8(void);
s32 fn_1_1B18C(void);
void fn_1_1B1F8(u16 cameraMask);
s32 fn_1_1B250(void);
void fn_1_1B7B4(void);
void fn_1_1B94C(void);
void fn_1_1BB18(void);
void fn_1_1BBD8(void);
void fn_1_1E3F8(u16 duration, f32 amplitude, f32 amplitudeDecay, f32 angleStep);
void fn_1_1E448(void);
void fn_1_74C0(void);
void fn_1_75A4(void);
void fn_1_83F0(void);
void fn_1_8488(void);
void fn_1_8520(void);
int _prolog(void);
void _epilog(void);
s32 ObjectSetup(void);
void fn_1_38B0(void);
void fn_1_3BFC(void);
void fn_1_3E7C(void);
void fn_1_3FE4(s16 layerNo);
void fn_1_4030(s16 layerNo);
void fn_1_407C(s16 x, s16 y, s32 width, s32 height, enum _GXTexFmt format,
                void *destination);
void fn_1_40FC(void);
void fn_1_4808(void);
void fn_1_4868(void);
void fn_1_9DB8(s32 playerNo, s32 points);
void fn_1_9E28(void);
void fn_1_9F38(s32 playerNo, s32 points);
void fn_1_A18C(s32 playerNo, s32 decrement);
void fn_1_A570(void);
void fn_1_A898(void);
void fn_1_A97C(s32 *values, s32 count);
void fn_1_AA1C(s32 *values, s32 *secondaryValues, s32 *tertiaryValues, s32 count);
void fn_1_AAFC(void);
void fn_1_3A8C(s32 modelIndex, s32 motionMode);
s32 fn_1_8590(void);
void fn_1_9284(s32 objectOffset);
void fn_1_9860(void);
void fn_1_A088(s32 playerNo, s32 increment);
void fn_1_9934(s32 mode, s32 playerIndex);
void fn_1_9FA8(void);
void fn_1_A678(s32 playerNo, s32 motionNo, f32 blendFrames, u32 attr);
void fn_1_ABF4(void);
void fn_1_48D8(void);
void fn_1_4B00(void);
void fn_1_4CB8(void);
s32 fn_1_4E70(void);
void fn_1_62C8(void);
s32 fn_1_632C(void);
s32 fn_1_639C(void);
s32 fn_1_640C(void);
s32 fn_1_6478(void);
void fn_1_6514(void);
void fn_1_6628(void);
void fn_1_69D0(u32 selection);
void fn_1_7688(void);
void fn_1_8234(void);
s32 fn_1_BE70(void);
HU3D_PARMANID fn_1_C1DC(ANIMDATA *animation, s32 mode, HuVecF *position);
s32 fn_1_C458(void);
s32 fn_1_DA48(void);
void fn_1_DF9C(s32 mode);
void fn_1_E6C0(s32 playerNo, s32 fieldIndex, s32 flagType, s32 enabled);
void fn_1_E80C(void);
s16 fn_1_196BC(s32 playerNo);
void fn_1_19C84(s32 playerNo);
void fn_1_1A724(f32 positionX, f32 positionY, f32 positionZ, s32 modelMode, f32 rotationY);
void fn_1_1AED8(s16 requestedDuration);
void fn_1_1A968(HU3D_MODEL *model, HU3D_PARTICLE *hook, Mtx matrix);
void fn_1_1AC20(HU3D_MODEL *model, HU3D_PARTICLE *particleData, Mtx matrix);
void fn_1_1AFC4(s16 duration, u8 targetAlpha);
void fn_1_E824(void);
void fn_1_129D4(void);
void fn_1_16ECC(s32 playerIndex);
void fn_1_1745C(s32 firstPlayer, s32 secondPlayer, s32 thirdPlayer, s32 fourthPlayer);
void fn_1_1778C(s32 firstPlayer, s32 secondPlayer, s32 thirdPlayer, s32 fourthPlayer);
void fn_1_171CC(s32 playerIndex, s32 secondPlayer, s32 thirdPlayer, s32 fourthPlayer);
void fn_1_11FA8(void);
void fn_1_121A4(void);
void fn_1_123A0(void);
void fn_1_1264C(s32 firstValue, s32 secondValue);
void fn_1_12810(s32 firstValue, s32 secondValue);
void fn_1_17B6C(void);
void fn_1_17D68(void);
void fn_1_17F64(void);
void fn_1_100BC(void);
void fn_1_1042C(void);
void fn_1_1065C(s32 playerIndex);
void fn_1_10934(s32 playerIndex);
void fn_1_10AE4(void);
void fn_1_10C58(void);
void fn_1_10E28(void);
void fn_1_112B0(void);
void fn_1_11424(void);
void fn_1_1153C(void);
void fn_1_1170C(void);
void fn_1_119FC(void);
void fn_1_15EF8(void);
void fn_1_E91C(void);
void fn_1_1614C(void);
void fn_1_16620(void);
void fn_1_16A24(void);
s16 fn_1_16E30(void);
void fn_1_186E8(void);
void fn_1_18168(void);
void fn_1_18800(void);
void fn_1_18B50(void);
void fn_1_18C68(void);
void fn_1_18D80(void);
void fn_1_18E98(s32 messageIndex);
void fn_1_18FEC(s32 messageIndex);
void fn_1_19140(s32 messageIndex);
void fn_1_19294(s32 playerIndex);
void fn_1_1959C(void);
s16 fn_1_EF58(void);
s16 fn_1_F084(void);
void fn_1_F1B0(void);
void fn_1_F3CC(s32 playerIndex);
void fn_1_F568(void);
void fn_1_F6B4(void);
void fn_1_FAF4(void);
void fn_1_F824(void);
void fn_1_FE40(void);
void fn_1_13740(s32 firstPlayer, s32 secondPlayer, s32 thirdPlayer, s32 fourthPlayer,
                s32 motionIndex);
void fn_1_13878(void);
void fn_1_13B54(void);
void fn_1_13D70(s32 messageNo, s32 playerNo);
void fn_1_142F4(s32 messageNo, s32 playerNo);
void fn_1_148E4(s32 playerIndex);
void fn_1_14E48(s32 playerNo, s32 otherPlayerNo);
void fn_1_15424(s32 playerNo, s32 firstOtherPlayer, s32 secondOtherPlayer);
void fn_1_15AB0(s32 playerIndex);
void fn_1_C320(s32 playerNo, s32 otherPlayerNo, s32 amount);
void fn_1_C9B4(void);
s32 fn_1_CBAC(s32 levelIndex, s32 column);
void fn_1_CE28(s32 levelIndex, s32 column, s32 selection, s32 flags);
void fn_1_E7F4(s16 selection);
void fn_1_1A854(f32 positionX, f32 positionY, f32 positionZ, f32 rotationY);
void fn_1_11BB0(void);
void fn_1_11DAC(void);
s32 fn_1_1A2D0(s16 entryValue, s32 hookType);
s32 fn_1_1A524(s16 entryValue, s32 hookType, s32 particleType);
void fn_1_1B28C(OMOBJ *object);
void fn_1_1B3EC(HU3D_MODEL *model, Mtx *matrix);
void fn_1_1B9D4(void);
void fn_1_1BC74(s32 levelIndex, s32 columnIndex, s32 bank);
void fn_1_1BE00(void);
void fn_1_1C300(s32 levelIndex, s32 columnIndex);
void fn_1_1CAE4(void);
void fn_1_1CD58(void);
void fn_1_1D580(void);
void fn_1_1D994(void);
void fn_1_1DFDC(void);
s32 fn_1_1E56C(void);
s32 fn_1_1E944(void);
void fn_1_1EC38(void);
void fn_1_1EE70(void);
void fn_1_1F0A8(s32 messageIndex);
void fn_1_1F318(s32 messageIndex);
void fn_1_1F4A0(s32 messageIndex);
void fn_1_1F628(s32 messageIndex);
void fn_1_1F7B0(s32 value);
void fn_1_1F938(s32 firstPlayer, s32 secondPlayer);
void fn_1_1FC10(s32 playerIndex);
void fn_1_1FE98(void);

/* These bytes are not read by this mode; their purpose is unknown. */
u8 lbl_1_data_384[4] = { 255, 255, 255, 255 };
/* Character voice cues for introductions and score reactions. */
s16 lbl_1_data_388[11] = { MSM_SE_CHARMIC_INTRO_CALL,         MSM_SE_CHARMIC_LUIGI_INTRO_CALL,
                           MSM_SE_CHARMIC_PEACH_INTRO_CALL,   MSM_SE_CHARMIC_YOSHI_INTRO_CALL,
                           MSM_SE_CHARMIC_WARIO_INTRO_CALL,   MSM_SE_CHARMIC_DAISY_INTRO_CALL,
                           MSM_SE_CHARMIC_WALUIGI_INTRO_CALL, MSM_SE_CHARMIC_KINOPIO_INTRO_CALL,
                           MSM_SE_CHARMIC_TERESA_INTRO_CALL,  MSM_SE_CHARMIC_MINIKOOPA_INTRO_CALL,
                           MSM_SE_CHARMIC_KINOPIKO_INTRO_CALL };
s16 lbl_1_data_39E[] = {
    MSM_SE_CHARMIC_SCORE_TALLY_CHEER,         MSM_SE_CHARMIC_LUIGI_SCORE_TALLY_CHEER,
    MSM_SE_CHARMIC_PEACH_SCORE_TALLY_CHEER,   MSM_SE_CHARMIC_YOSHI_SCORE_TALLY_CHEER,
    MSM_SE_CHARMIC_WARIO_SCORE_TALLY_CHEER,   MSM_SE_CHARMIC_DAISY_SCORE_TALLY_CHEER,
    MSM_SE_CHARMIC_WALUIGI_SCORE_TALLY_CHEER, MSM_SE_CHARMIC_KINOPIO_SCORE_TALLY_CHEER,
    MSM_SE_CHARMIC_TERESA_SCORE_TALLY_CHEER,  MSM_SE_CHARMIC_MINIKOOPA_SCORE_TALLY_CHEER,
    MSM_SE_CHARMIC_KINOPIKO_SCORE_TALLY_CHEER
};
s16 lbl_1_data_3B4[] = {
    MSM_SE_CHARVOICE_HAPPY_CHEER,         MSM_SE_CHARVOICE_LUIGI_HAPPY_CHEER,
    MSM_SE_CHARVOICE_PEACH_HAPPY_CHEER,   MSM_SE_CHARVOICE_YOSHI_HAPPY_CHEER,
    MSM_SE_CHARVOICE_WARIO_HAPPY_CHEER,   MSM_SE_CHARVOICE_DAISY_HAPPY_CHEER,
    MSM_SE_CHARVOICE_WALUIGI_HAPPY_CHEER, MSM_SE_CHARVOICE_KINOPIO_HAPPY_CHEER,
    MSM_SE_CHARVOICE_TERESA_HAPPY_CHEER,  MSM_SE_CHARVOICE_MINIKOOPA_HAPPY_CHEER,
    MSM_SE_CHARVOICE_KINOPIKO_HAPPY_CHEER
};
s16 lbl_1_data_3CA[11] = {
    MSM_SE_CHARVOICE_HAPPY_CHEER,         MSM_SE_CHARVOICE_LUIGI_HAPPY_CHEER,
    MSM_SE_CHARVOICE_PEACH_HAPPY_CHEER,   MSM_SE_CHARVOICE_YOSHI_HAPPY_CHEER,
    MSM_SE_CHARVOICE_WARIO_HAPPY_CHEER,   MSM_SE_CHARVOICE_DAISY_HAPPY_CHEER,
    MSM_SE_CHARVOICE_WALUIGI_HAPPY_CHEER, MSM_SE_CHARVOICE_KINOPIO_HAPPY_CHEER,
    MSM_SE_CHARVOICE_TERESA_HAPPY_CHEER,  MSM_SE_CHARVOICE_MINIKOOPA_HAPPY_CHEER,
    MSM_SE_CHARVOICE_KINOPIKO_HAPPY_CHEER
};
s16 lbl_1_data_3E0[12] = {
    MSM_SE_CHARMIC_POINT_REACTION,          MSM_SE_CHARMIC_LUIGI_POINT_REACTION,
    MSM_SE_CHARMIC_PEACH_POINT_REACTION,    MSM_SE_CHARMIC_YOSHI_POINT_REACTION,
    MSM_SE_CHARMIC_WARIO_POINT_REACTION,    MSM_SE_CHARMIC_DAISY_POINT_REACTION,
    MSM_SE_CHARMIC_WALUIGI_POINT_REACTION,  MSM_SE_CHARMIC_KINOPIO_POINT_REACTION,
    MSM_SE_CHARMIC_TERESA_POINT_REACTION,   MSM_SE_CHARMIC_MINIKOOPA_POINT_REACTION,
    MSM_SE_CHARMIC_KINOPIKO_POINT_REACTION, 0
};
u32 lbl_1_data_3F8[] = {
    MESSNUM(MESS_QUIZ_LV, 0), MESSNUM(MESS_QUIZ_LV, 1),
    MESSNUM(MESS_QUIZ_LV, 2), MESSNUM(MESS_QUIZ_LV, 3),
    MESSNUM(MESS_QUIZ_LV, 4)
};
u32 lbl_1_data_40C[] = {
    MESSNUM(MESS_QUIZ_LV, 10), MESSNUM(MESS_QUIZ_LV, 11),
    MESSNUM(MESS_QUIZ_LV, 12), MESSNUM(MESS_QUIZ_LV, 13),
    MESSNUM(MESS_QUIZ_LV, 14)
};
unsigned int lbl_1_data_420[10] = {
    DATANUM(0x930000, 0), DATANUM(0x930000, 6),
    DATANUM(0x930000, 40), DATANUM(0x930000, 30),
    DATANUM(0x930000, 23), DATANUM(0x930000, 87),
    DATANUM(0x930000, 21), DATANUM(0x930000, 36),
    DATANUM(0x930000, 37), 0
};
s32 lbl_1_data_448[2][4] = {
    DATANUM(0xa20000, 3), DATANUM(0xa20000, 4),
    DATANUM(0xa20000, 5), DATANUM(0xa20000, 6),
    DATANUM(0xa20000, 7), DATANUM(0xa20000, 8),
    DATANUM(0xa20000, 9), DATANUM(0xa20000, 10)
};
s32 lbl_1_data_468[4] = {
    DATANUM(0xa20000, 12), DATANUM(0xa20000, 13),
    DATANUM(0xa20000, 14), DATANUM(0xa20000, 15)
};
f32 lbl_1_data_478[6] = { -50.0f, -70.0f, -95.0f, 45.0f, 57.0f, 84.0f };
f32 lbl_1_data_490[6] = { 200.0f, 100.0f, 0.0f, 300.0f, 0.0f, 0.0f };
Point3d lbl_1_data_4A8 = { -10000.0f, 10000.0f, 0.0f };
Point3d lbl_1_data_4B4 = { 0.0f, 0.0f, 0.0f };
GXColor lbl_1_data_4C0 = { 255, 255, 255, 255 };
Point3d lbl_1_data_4C4 = { 10000.0f, 10000.0f, 0.0f };
Point3d lbl_1_data_4D0 = { 0.0f, 0.0f, 0.0f };
GXColor lbl_1_data_4DC = { 255, 255, 255, 255 };
Point3d lbl_1_data_4E0 = { 0.0f, 10000.0f, 10000.0f };
Point3d lbl_1_data_4EC = { 0.0f, 0.0f, 0.0f };
GXColor lbl_1_data_4F8 = { 255, 255, 255, 255 };
/* Camera views for the opening, round transitions, and results. */
OM_CAMERA_VIEW lbl_1_data_4FC = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2265.0f };
OM_CAMERA_VIEW lbl_1_data_518 = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2265.0f };
OM_CAMERA_VIEW lbl_1_data_534 = { 0.0f, 225.0f, 0.0f, 0.0f, 0.0f, 0.0f, 926.0f };
OM_CAMERA_VIEW lbl_1_data_550[4] = {
    520.1544f, 65.0f, -315.983f, -10.0f,
    -75.0f, 0.0f, 1277.0f, 571.9181f,
    65.0f, -122.798f, -10.0f, -75.0f,
    0.0f, 1277.0f, 623.6814f, 65.0f,
    70.3872f, -10.0f, -75.0f, 0.0f,
    1277.0f, 675.4457f, 65.0f, 263.5726f,
    -10.0f, -75.0f, 0.0f, 1277.0f
};
OM_CAMERA_VIEW lbl_1_data_5C0 = { 390.0f, 165.0f, 30.0f, -10.0f, -75.0f, 0.0f, 1869.0f };
OM_CAMERA_VIEW lbl_1_data_5DC = { 0.0f, 100.0f, -10.0f, -3.7f, 0.0f, 0.0f, 1310.0f };
OM_CAMERA_VIEW lbl_1_data_5F8 = {
    -600.0f, 140.0f, -175.0f, -9.48f, 58.75f, 0.0f, 1746.0f
};
OM_CAMERA_VIEW lbl_1_data_614 = { 0.0f, 500.0f, -35.0f, -20.5f, 0.0f, 0.0f, 5365.0f };
OM_CAMERA_VIEW lbl_1_data_630 = { 0.0f, 185.0f, -35.0f, -6.0f, 0.0f, 0.0f, 1590.0f };
OM_CAMERA_VIEW lbl_1_data_64C = { 0.0f, 120.0f, 0.0f, 0.5f, 0.0f, 0.0f, 3113.0f };
Vec lbl_1_data_668[4] = {
    520.1544f, 0.0f, -315.983f, 571.9181f,
    0.0f, -122.798f, 623.6814f, 0.0f,
    70.3872f, 675.4457f, 0.0f, 263.5726f
};
QuizRoundView lbl_1_data_698[4] = {
    0.0f, 0.0f, -400.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, -75.0f, 0.0f, -400.0f, 75.0f,
    0.0f, -400.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    -150.0f, 0.0f, -400.0f, 0.0f, 0.0f, -400.0f, 150.0f, 0.0f,
    -400.0f, 0.0f, 0.0f, 0.0f, -225.0f, 0.0f, -400.0f, -75.0f,
    0.0f, -400.0f, 75.0f, 0.0f, -400.0f, 225.0f, 0.0f, -400.0f
};
/* Player model rotations in degrees for the four display positions. */
f32 lbl_1_data_758[4][3] = {
    0.0f, -75.0f, 0.0f, 0.0f, -75.0f, 0.0f, 0.0f, -75.0f, 0.0f, 0.0f, -75.0f, 0.0f
};
/* These bytes are not read by this mode; their purpose is unknown. */
u8 lbl_1_data_788[44] = {
    66, 72, 0, 0, 66, 72, 0, 0, 0, 0, 0, 0, 66, 72, 0, 0,
    66, 72, 0, 0, 0, 0, 0, 0, 65, 200, 0, 0, 66, 150, 0, 0,
    66, 72, 0, 0, 66, 72, 0, 0, 66, 150, 0, 0
};
Point3d lbl_1_data_7B4[2] = { -100.0f, 0.0f, 0.0f, 100.0f, 0.0f, 0.0f };
Point3d lbl_1_data_7CC[2] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
Vec lbl_1_data_7E4[2] = {
    -651.8f, 30.0f, -84.6f, -548.2f, 30.0f, -255.4f
};
Vec lbl_1_data_7FC[2] = { 0.0f, 58.75f, 0.0f, 0.0f, 58.75f, 0.0f };
Vec lbl_1_data_814 = { -600.0f, 0.0f, -170.0f };
Vec lbl_1_data_820 = { 0.0f, 58.75f, 0.0f };
s32 lbl_1_data_82C[4] = {
    DATANUM(0xa20000, 82), DATANUM(0xa20000, 83),
    DATANUM(0xa20000, 84), DATANUM(0xa20000, 85)
};
f32 lbl_1_data_83C[4][3] = {
    422.3543f, 0.0f, -289.778f, 474.1181f,
    0.0f, -96.5926f, 525.8814f, 0.0f,
    96.5926f, 577.6457f, 0.0f, 289.778f
};
char *lbl_1_data_888[3] = { "vq_math", "vq_math2", "vq_math3" };
u16 lbl_1_data_894[6] = { 105, 103, 103, 103, 103, 103 };
u32 lbl_1_data_8A0[] = {
    MESSNUM(MESS_MICQUIZ, 116), MESSNUM(MESS_MICQUIZ, 117),
    MESSNUM(MESS_MICQUIZ, 118), MESSNUM(MESS_MICQUIZ, 119)
};

/* Run module initializers, then build the Speak Up scene when the overlay loads. */
int _prolog(void) {
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    ObjectSetup();
    return 0;
}

/* Run module finalizers when the overlay unloads. */
void _epilog(void) {
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Create the quiz scene, presenter and player models, cameras, and round processes on entry. */
s32 ObjectSetup(void) {
    OMOVLHIS *history;
    f32 characterHeight;
    s32 index;
    s32 column;
    u32 textureSize;
    s32 micInitResult;

    OSReport("******* MQ ObjectSetup *********\n");
    OSReport("          HEAP_HEAP Malloc Size %x\n", HuMemUsedMallocSizeGet(HEAP_HEAP));
    lbl_1_bss_288 = fn_8003A8BC(200, 8192);
    fn_80069F1C(lbl_1_bss_288);
    Hu3DCameraCreate(7);
    Hu3DCameraPerspectiveSet(4, 20.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DCameraViewportSet(4, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(4, 0, 0, 640, 480);
    history = omOvlHisGet(0);
    micQuizModeWork = MgModeWorkGet();
    if (lbl_802C0248 == 0) {
        fn_1_38B0();
    }
    lbl_1_bss_20 = fn_8003AB38(lbl_1_bss_288, 32730, 0, 0, -1, fn_8003DAA4);
    lbl_1_bss_20->work[0] = 3;
    lbl_1_bss_2C = fn_8003AB38(lbl_1_bss_288, 100, 0, 0, -1, fn_1_A348);
    lbl_1_bss_24 = fn_8003AB38(lbl_1_bss_288, 100, 0, 0, -1, (OMOBJ_FUNC)fn_1_E824);

    lbl_1_bss_264 = (s16)Hu3DGLightCreate(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0);
    Hu3DGLightPosAimSetV(lbl_1_bss_264, &lbl_1_data_4A8, &lbl_1_data_4B4);
    Hu3DGLightStaticSet(lbl_1_bss_264, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_264);
    Hu3DGLightColorSet(lbl_1_bss_264, lbl_1_data_4C0.r, lbl_1_data_4C0.g, lbl_1_data_4C0.b,
                       lbl_1_data_4C0.a);
    lbl_1_bss_260 = (s16)Hu3DGLightCreate(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0);
    Hu3DGLightPosAimSetV(lbl_1_bss_260, &lbl_1_data_4C4, &lbl_1_data_4D0);
    Hu3DGLightStaticSet(lbl_1_bss_260, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_260);
    Hu3DGLightColorSet(lbl_1_bss_260, lbl_1_data_4DC.r, lbl_1_data_4DC.g, lbl_1_data_4DC.b,
                       lbl_1_data_4DC.a);
    lbl_1_bss_25C = (s16)Hu3DGLightCreate(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0);
    Hu3DGLightPosAimSetV(lbl_1_bss_25C, &lbl_1_data_4E0, &lbl_1_data_4EC);
    Hu3DGLightStaticSet(lbl_1_bss_25C, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_25C);
    Hu3DGLightColorSet(lbl_1_bss_25C, lbl_1_data_4F8.r, lbl_1_data_4F8.g, lbl_1_data_4F8.b,
                       lbl_1_data_4F8.a);

    micQuizModeWork->compactPauseDisplay = 1;
    micQuizModeWork->playerCount = 0;
    for (index = 0; index < 4; index++) {
        if (lbl_80265728[index].type != 0) {
            micQuizModeWork->playerActive[index] = 0;
        } else {
            micQuizModeWork->playerActive[index] = 1;
            micQuizModeWork->playerCount++;
        }
    }
    HuWinInit(1);
    MicQuizWinCreate();

    lbl_1_bss_236 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 67), 0x10000000, 2));
    if (micQuizModeWork->presenterLayout == 0) {
        fn_800269E8(lbl_1_bss_236, ((1 << 30) | (1 << 1)));
    } else {
        fn_8002D414(lbl_1_bss_236, fn_8002D530(lbl_1_bss_236));
    }
    fn_80026B64(lbl_1_bss_236, 4);
    fn_80026B8C(lbl_1_bss_236, 0);

    for (index = 0; index < 4; index++) {
        if (micQuizModeWork->playerActive[index] != 0) {
            lbl_1_bss_2F4[index] = lbl_80265728[index].charNo;
            lbl_1_bss_2FC[index] = fn_8005C4E4(
                lbl_1_bss_2F4[index], 4, lbl_1_data_420, lbl_1_bss_2AC[index]);
            if (lbl_1_bss_2F4[index] == 5) {
                fn_800273EC(lbl_1_bss_2FC[index], 0.7142f, 0.7142f, 0.7142f);
            }
            characterHeight = fn_8005C4C8(lbl_1_bss_2F4[index]) / 2.0f;
            fn_800269E8(lbl_1_bss_2FC[index], ((1 << 30) | (1 << 0)));
            fn_8005BE80(lbl_1_bss_2F4[index], lbl_1_bss_2AC[index][0]);
            {
                Vec modelOffset;
                modelOffset.x = 100.0f * sin(-(M_PI * 72 / 180));
                modelOffset.y = 0.0f;
                modelOffset.z = 100.0f * cos(-(M_PI * 72 / 180));

                fn_80026760(lbl_1_bss_2FC[index], lbl_1_data_668[index].x - modelOffset.x,
                                lbl_1_data_668[index].y - modelOffset.y,
                                lbl_1_data_668[index].z - modelOffset.z);
                fn_80026808(lbl_1_bss_2FC[index], lbl_1_data_758[index][0],
                                lbl_1_data_758[index][1], lbl_1_data_758[index][2]);
                fn_80026B64(lbl_1_bss_2FC[index], 4);

                lbl_1_bss_1F0[index] = fn_80025A04(
                    fn_80008DD0(DATANUM(0xa20000, 93), 0x10000000, 2));
                fn_80026B64(lbl_1_bss_1F0[index], 4);
                fn_80026760(lbl_1_bss_1F0[index], lbl_1_data_668[index].x - modelOffset.x,
                                0.0f, lbl_1_data_668[index].z - modelOffset.z);
                fn_80026808(lbl_1_bss_1F0[index], lbl_1_data_758[index][0],
                                lbl_1_data_758[index][1], lbl_1_data_758[index][2]);
                fn_800269E8(lbl_1_bss_1F0[index], (1 << 0));
                fn_800269E8(lbl_1_bss_1F0[index], ((1 << 30) | (1 << 0)));
                fn_80026B8C(lbl_1_bss_1F0[index], 1);
            }
        }
    }

    fn_8005DDEC(1);
    fn_8002C2AC(10);
    lbl_1_bss_256 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 49), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_256, ((1 << 30) | (1 << 0)));
    fn_80026B8C(lbl_1_bss_256, 0);
    fn_80026760(lbl_1_bss_256, 0.0f, 0.0f, 0.0f);
    fn_80026B64(lbl_1_bss_256, 1);
    lbl_1_bss_254 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 16), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_254, 1);
    fn_800269E8(lbl_1_bss_254, ((1 << 30) | (1 << 0)));
    fn_80026760(lbl_1_bss_254, 0.0f, 0.0f, -10.0f);
    fn_800269E8(lbl_1_bss_254, (1 << 0));
    lbl_1_bss_252 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 17), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_252, 1);
    fn_800269E8(lbl_1_bss_252, ((1 << 30) | (1 << 0)));
    fn_80026760(lbl_1_bss_252, 0.0f, 0.0f, -10.0f);
    fn_800269E8(lbl_1_bss_252, (1 << 0));
    if (micQuizModeWork->presenterLayout != 0) {
        fn_80026A44(lbl_1_bss_252, (1 << 0));
    } else {
        fn_80026A44(lbl_1_bss_254, (1 << 0));
    }

    fn_8000C200(lbl_1_bss_30 = HuSprAnimMake(640, 480, 2),
        0x10000000);
    lbl_1_bss_30->bmp->palData = NULL;
    lbl_1_bss_30->bmp->palNum = 0;
    textureSize = GXGetTexBufferSize(640, 480, GX_TF_RGB5A3, GX_FALSE, 0);
    lbl_1_bss_30->bmp->data = HuMemDirectMallocNum(
        2, textureSize, 0x10000000);
    fn_8000C200(lbl_1_bss_34 = HuSprAnimMake(640, 480, 2),
        0x10000000);
    lbl_1_bss_34->bmp->palData = NULL;
    lbl_1_bss_34->bmp->palNum = 0;
    textureSize = GXGetTexBufferSize(640, 480, GX_TF_RGB5A3, GX_FALSE, 0);
    lbl_1_bss_34->bmp->data = HuMemDirectMallocNum(
        2, textureSize, 0x10000000);

    lbl_1_bss_23E =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 77), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_23E, ((1 << 30) | (1 << 0)));
    fn_80026B64(lbl_1_bss_23E, 4);
    fn_1_19DD4(1, 1, 0);
    lbl_1_bss_23C =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 78), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_23C, ((1 << 30) | (1 << 0)));
    fn_80026B64(lbl_1_bss_23C, 4);
    lbl_1_bss_23A =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 79), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_23A, ((1 << 30) | (1 << 0)));
    fn_80026B64(lbl_1_bss_23A, 4);
    lbl_1_bss_21E =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 64), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_21E, ((1 << 30) | (1 << 0)));
    fn_80026B64(lbl_1_bss_21E, 4);
    lbl_1_bss_21C =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 66), 0x10000000, 2));
    if (micQuizModeWork->presenterLayout == 0) {
        fn_800269E8(lbl_1_bss_21C, ((1 << 30) | (1 << 1)));
    } else {
        fn_8002D414(lbl_1_bss_21C, fn_8002D530(lbl_1_bss_21C));
    }
    fn_80026B64(lbl_1_bss_21C, 4);
    lbl_1_bss_238 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 65), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_238, ((1 << 30) | (1 << 0)));
    fn_80026B64(lbl_1_bss_238, 4);
    lbl_1_bss_234 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 68), 0x10000000, 2));
    if (micQuizModeWork->presenterLayout == 0) {
        fn_800269E8(lbl_1_bss_234, ((1 << 30) | (1 << 1)));
    } else {
        fn_8002D414(lbl_1_bss_234, fn_8002D530(lbl_1_bss_234));
    }
    fn_80026B64(lbl_1_bss_234, 4);
    lbl_1_bss_232 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 69), 0x10000000, 2));
    if (micQuizModeWork->presenterLayout == 0) {
        fn_800269E8(lbl_1_bss_232, ((1 << 30) | (1 << 0)));
    } else {
        fn_800269E8(lbl_1_bss_232, (1 << 0));
    }
    fn_80026B64(lbl_1_bss_232, 4);
    lbl_1_bss_230 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 70), 0x10000000, 2));
    if (micQuizModeWork->presenterLayout == 0) {
        fn_800269E8(lbl_1_bss_230, ((1 << 30) | (1 << 1)));
        fn_800269E8(lbl_1_bss_230, (1 << 0));
    } else {
        fn_8002D414(lbl_1_bss_230, fn_8002D530(lbl_1_bss_230));
    }
    fn_80026B64(lbl_1_bss_230, 4);
    for (index = 0; index < 6; index++) {
        lbl_1_bss_224[index] = fn_80025A04(
            fn_80008DD0(DATANUM(0xa20000, 71), 0x10000000, 2));
        fn_800269E8(lbl_1_bss_224[index], ((1 << 30) | (1 << 0)));
        if (micQuizModeWork->presenterLayout == 0) {
            fn_8002D730(lbl_1_bss_224[index], 1.0f, 120.0f);
        } else {
            fn_8002D730(lbl_1_bss_224[index], 161.0f, 280.0f);
            fn_8002D414(lbl_1_bss_224[index], fn_8002D530(lbl_1_bss_224[index]));
        }
        fn_80026808(lbl_1_bss_224[index], 0.0f, lbl_1_data_478[index], 0.0f);
        fn_80026760(lbl_1_bss_224[index], 0.0f, lbl_1_data_490[index], 0.0f);
        fn_80026B64(lbl_1_bss_224[index], 4);
    }
    lbl_1_bss_222 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 72), 0x10000000, 2));
    if (micQuizModeWork->presenterLayout == 0) {
        fn_800269E8(lbl_1_bss_222, ((1 << 30) | (1 << 1)));
    } else {
        fn_8002D414(lbl_1_bss_222, fn_8002D530(lbl_1_bss_222));
    }
    fn_80026B64(lbl_1_bss_222, 4);
    lbl_1_bss_220 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 73), 0x10000000, 2));
    if (micQuizModeWork->presenterLayout == 0) {
        fn_800269E8(lbl_1_bss_220, ((1 << 30) | (1 << 1)));
    } else {
        fn_8002D414(lbl_1_bss_220, fn_8002D530(lbl_1_bss_220));
    }
    fn_80026B64(lbl_1_bss_220, 4);

    lbl_1_bss_1F8 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 92), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_1F8, (1 << 0));
    fn_800269E8(lbl_1_bss_1F8, ((1 << 30) | (1 << 0)));
    fn_80026B64(lbl_1_bss_1F8, 4);
    fn_80026B8C(lbl_1_bss_1F8, 1);
    lbl_1_bss_202 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 94), 0x10000000, 2));
    fn_80026760(lbl_1_bss_202, 0.0f, 0.0f, -350.0f);
    fn_800269E8(lbl_1_bss_202, (1 << 0));
    fn_80026B64(lbl_1_bss_202, 4);
    fn_1_320F8();

    for (index = 0; index < 4; index++) {
        lbl_1_bss_214[index] = fn_80025A04(
            fn_80008DD0(lbl_1_data_82C[index], 0x10000000, 2));
        fn_800269E8(lbl_1_bss_214[index], ((1 << 30) | (1 << 1)));
        fn_80026B64(lbl_1_bss_214[index], 4);
        lbl_1_bss_20C[index] = fn_80025A04(
            fn_80008DD0(DATANUM(0xa20000, 87), 0x10000000, 2));
        fn_800269E8(lbl_1_bss_20C[index], ((1 << 30) | (1 << 0)));
        fn_80026B64(lbl_1_bss_20C[index], 4);
        fn_80026760(lbl_1_bss_20C[index], lbl_1_data_83C[index][0],
                        lbl_1_data_83C[index][1], lbl_1_data_83C[index][2]);
        lbl_1_bss_204[index] = fn_80025A04(
            fn_80008DD0(DATANUM(0xa20000, 88), 0x10000000, 2));
        fn_800269E8(lbl_1_bss_204[index], ((1 << 30) | (1 << 0)));
        fn_80026B64(lbl_1_bss_204[index], 4);
        fn_80026760(lbl_1_bss_204[index], lbl_1_data_83C[index][0],
                        lbl_1_data_83C[index][1], lbl_1_data_83C[index][2]);
        fn_80026B8C(lbl_1_bss_204[index], 1);
        if (micQuizModeWork->presenterLayout != 0) {
            fn_8002D414(lbl_1_bss_214[index], 45.0f);
        } else {
            fn_8002D414(lbl_1_bss_214[index], 0.0f);
            fn_800269E8(lbl_1_bss_204[index], (1 << 0));
            fn_800269E8(lbl_1_bss_204[index], ((1 << 30) | (1 << 1)));
        }
    }

    for (index = 0, column = 0; index < 4; index++) {
        if (micQuizModeWork->playerActive[index] != 0) {
            lbl_1_bss_1FA[index] = fn_80025D4C(lbl_1_bss_202);
            fn_80026760(lbl_1_bss_1FA[index],
                lbl_1_data_698[micQuizModeWork->playerCount - 1].playerPositions[column].x,
                lbl_1_data_698[micQuizModeWork->playerCount - 1].playerPositions[column].y,
                -350.0f);
            column++;
            fn_800269E8(lbl_1_bss_1FA[index], (1 << 0));
            fn_80026B64(lbl_1_bss_1FA[index], 4);
        }
    }
    lbl_1_bss_1EC[0] = fn_80025A04(
        fn_80008DD0(DATANUM(0xa20000, 75), 0x10000000, 2));
    lbl_1_bss_1EC[1] = fn_80025D4C(lbl_1_bss_1EC[0]);
    fn_800269E8(lbl_1_bss_1EC[0], (1 << 0));
    fn_800269E8(lbl_1_bss_1EC[1], (1 << 0));
    fn_80026B8C(lbl_1_bss_1EC[0], 3);
    fn_80026B8C(lbl_1_bss_1EC[1], 3);

    lbl_1_bss_1E8 =
        HuSprAnimRead(fn_80008DD0(DATANUM(0x210000, 1), 0x10000000, 2));
    fn_800097F4(0x210000);
    lbl_1_bss_314[0] = HuSprAnimRead(
        fn_80008DD0(DATANUM(0xa20000, 50), 0x10000000, 2));
    lbl_1_bss_310 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_310, 0, HuSprCreate(lbl_1_bss_314[0], 50, 2));
    HuSprAttrSet(lbl_1_bss_310, 0, HUSPR_ATTR_DISPOFF);
    fn_8000DAC0(fn_1_1B7B4, 100, 4096, 0, lbl_1_bss_288);
    lbl_1_bss_304.x = 0.0f;
    lbl_1_bss_304.y = 0.0f;
    lbl_1_bss_30C = 2;
    lbl_1_bss_1E2 = fn_8002C4C0(
        fn_80008DD0(DATANUM(0xa20000, 89), 0x10000000, 2));
    lbl_1_bss_1E0 = fn_8002C4C0(
        fn_80008DD0(DATANUM(0xa20000, 90), 0x10000000, 2));
    lbl_1_bss_1DE = fn_8002C4C0(
        fn_80008DD0(DATANUM(0xa20000, 91), 0x10000000, 2));
    lbl_1_bss_1DC = fn_80030458(
        lbl_1_bss_23E, fn_80008DD0(DATANUM(0xa20000, 80), 0x10000000, 2));
    micQuizFlashAnim = HuSprAnimRead(
        fn_80008DD0(DATANUM(0xa20000, 27), 0x10000000, 2));
    HuSprGrpMemberSet(micQuizFlashGroup, 0, HuSprCreate(micQuizFlashAnim, 49, 0));
    HuSprPosSet(micQuizFlashGroup, 0, 288.0f, 240.0f);
    HuSprScaleSet(micQuizFlashGroup, 0, 2.0f, 2.0f);
    HuSprTPLvlSet(micQuizFlashGroup, 0, 0.0f);
    HuSprColorSet(micQuizFlashGroup, 0, 255, 0, 0);
    HuSprAttrSet(micQuizFlashGroup, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(micQuizFlashGroup, 0, 8);
    HuSprDrawNoSet(micQuizFlashGroup, 0, 0);

    lbl_1_bss_248 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 47), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_248, 1);
    fn_800269E8(lbl_1_bss_248, (1 << 0));
    lbl_1_bss_246 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 48), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_246, 1);
    fn_800269E8(lbl_1_bss_246, (1 << 0));
    lbl_1_bss_244 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 39), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_244, 1);
    fn_800269E8(lbl_1_bss_244, (1 << 0));
    fn_800269E8(lbl_1_bss_244, ((1 << 30) | (1 << 1)));
    lbl_1_bss_1D6 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 97), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_1D6, 2);
    fn_8002D86C(lbl_1_bss_1D6, 0.7f);
    fn_80026760(lbl_1_bss_1D6, 0.0f, 0.0f, 1700.0f);
    fn_800269E8(lbl_1_bss_1D6, ((1 << 30) | (1 << 0)));
    fn_800269E8(lbl_1_bss_1D6, (1 << 0));
    lbl_1_bss_1D8 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 98), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_1D8, 2);
    fn_80026760(lbl_1_bss_1D8, 0.0f, 0.0f, 1700.0f);
    fn_800269E8(lbl_1_bss_1D8, ((1 << 30) | (1 << 1)));
    fn_800269E8(lbl_1_bss_1D8, (1 << 0));

    lbl_1_bss_1D4 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 96), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_1D4, 2);
    fn_80026760(lbl_1_bss_1D4, 0.0f, 0.0f, 200.0f);
    fn_800269E8(lbl_1_bss_1D4, (1 << 0));
    lbl_1_bss_1D2 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 95), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_1D2, 2);
    fn_800269E8(lbl_1_bss_1D2, (1 << 0));
    fn_80026760(lbl_1_bss_1D2, 0.0f, 0.0f, -10.0f);
    lbl_1_bss_1D0 = fn_80025D4C(lbl_1_bss_1D2);
    fn_80026B64(lbl_1_bss_1D0, 2);
    fn_80026808(lbl_1_bss_1D0, 0.0f, 180.0f, 0.0f);
    fn_800269E8(lbl_1_bss_1D0, (1 << 0));
    fn_80026CBC(lbl_1_bss_1D0, 0.0f);
    fn_80026760(lbl_1_bss_1D0, 0.0f, 0.0f, -10.0f);
    lbl_1_bss_250 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 16), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_250, 2);
    fn_80026760(lbl_1_bss_250, 0.0f, 0.0f, -10.0f);
    fn_800269E8(lbl_1_bss_250, ((1 << 30) | (1 << 0)));
    fn_800269E8(lbl_1_bss_250, (1 << 0));
    lbl_1_bss_24E = fn_80025D4C(lbl_1_bss_250);
    fn_80026B64(lbl_1_bss_24E, 2);
    fn_80026760(lbl_1_bss_24E, 0.0f, 0.0f, -10.0f);
    fn_800269E8(lbl_1_bss_24E, ((1 << 30) | (1 << 0)));
    fn_800269E8(lbl_1_bss_24E, (1 << 0));
    if (micQuizModeWork->presenterLayout != 0) {
        fn_80026A44(lbl_1_bss_24E, (1 << 0));
    } else {
        fn_80026A44(lbl_1_bss_250, (1 << 0));
    }
    lbl_1_bss_24C =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 18), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_24C, 2);
    fn_800269E8(lbl_1_bss_24C, (1 << 0));
    fn_800269E8(lbl_1_bss_24C, ((1 << 30) | (1 << 0)));
    lbl_1_bss_24A = fn_80025D4C(lbl_1_bss_24C);
    fn_80026760(lbl_1_bss_24A, 0.0f, 0.0f, 50.0f);
    fn_80026B64(lbl_1_bss_24A, 1);
    fn_800269E8(lbl_1_bss_24A, (1 << 0));
    fn_800269E8(lbl_1_bss_24A, ((1 << 30) | (1 << 0)));
    fn_80026B8C(lbl_1_bss_24A, 1);
    lbl_1_bss_6C =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 20), 0x10000000, 2));
    lbl_1_bss_68 =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 41), 0x10000000, 2));
    lbl_1_bss_64 =
        HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 22), 0x10000000, 2));
    for (index = 0; index < 5; index++) {
        for (column = 0; column < 5; column++) {
            if (column == 0 && index == 0) {
                lbl_1_bss_198[column].model[index] = fn_80025A04(
                    fn_80008DD0(DATANUM(0xa20000, 19), 0x10000000, 2));
            } else {
                lbl_1_bss_198[column].model[index] = fn_80025D4C(lbl_1_bss_198[0].model[0]);
            }
            fn_80026760(lbl_1_bss_198[column].model[index], 100.0f * column,
                           -300.0f + (500.0f - 100.0f * index), 0.0f);
            fn_80026B8C(lbl_1_bss_198[column].model[index], 1);
            fn_800269E8(lbl_1_bss_198[column].model[index], ((1 << 30) | (1 << 1)));
            fn_800269E8(lbl_1_bss_198[column].model[index], ((1 << 30) | (1 << 0)));
            lbl_1_bss_166[column][index] = fn_80030B2C(lbl_1_bss_6C,
                lbl_1_bss_198[column].model[index], "vq_panel");
            Hu3DAnimBankSet(lbl_1_bss_166[column][index], 0);
            lbl_1_bss_134[column][index] = fn_80030B2C(lbl_1_bss_68,
                lbl_1_bss_198[column].model[index], "vq_color");
            Hu3DAnimBankSet(lbl_1_bss_134[column][index], 0);
            lbl_1_bss_102[column].model[index] = fn_80030B2C(lbl_1_bss_64,
                lbl_1_bss_198[column].model[index], "vq_ivent");
            Hu3DAnimBankSet(lbl_1_bss_102[column].model[index], 0);
            fn_80026A44(lbl_1_bss_198[column].model[index], (1 << 0));
            fn_80026B64(lbl_1_bss_198[column].model[index], 1);
        }
    }
    lbl_1_bss_62 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 25), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_62, (1 << 0));
    fn_80026B64(lbl_1_bss_62, 1);
    fn_80026B8C(lbl_1_bss_62, 1);
    lbl_1_bss_60 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 26), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_60, (1 << 0));
    fn_80026B64(lbl_1_bss_60, 1);
    fn_80026B8C(lbl_1_bss_60, 1);
    lbl_1_bss_100 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 23), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_100, (1 << 0));
    fn_800269E8(lbl_1_bss_100, ((1 << 30) | (1 << 1)));
    fn_80026B64(lbl_1_bss_100, 1);
    lbl_1_bss_FE =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 24), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_FE, 1);
    fn_800268B0(lbl_1_bss_FE, 0.9f, 0.9f, 0.9f);
    fn_800269E8(lbl_1_bss_FE, (1 << 0));
    for (index = 0; index < 4; index++) {
        for (column = 0; column < 3; column++) {
            lbl_1_bss_CC[index][column] = HuSprAnimRead(
                fn_80008DD0(DATANUM(0xa20000, 86), 0x10000000, 2));
            lbl_1_bss_B4[index][column] = fn_80030B2C(
                lbl_1_bss_CC[index][column], lbl_1_bss_214[index],
                lbl_1_data_888[column]);
            Hu3DAnimBankSet(lbl_1_bss_B4[index][column], 0);
        }
    }
    lbl_1_bss_FC =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 74), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_FC, 1);
    fn_800268B0(lbl_1_bss_FC, 0.5f, 0.5f, 0.5f);
    fn_80026B8C(lbl_1_bss_FC, 2);
    fn_800269E8(lbl_1_bss_FC, ((1 << 30) | (1 << 0)));
    fn_800269E8(lbl_1_bss_FC, (1 << 0));
    fn_80026760(lbl_1_bss_FC, -425.0f, 240.0f, 10.0f);
    lbl_1_bss_5C = 0;
    fn_8003E488(1, &lbl_1_data_4FC);
    fn_8003E488(2, &lbl_1_data_518);
    if (HuMCMicGet() == 1) {
        OSReport("                                  HuMCInit(HUWIN_LANG_JAPAN)\n");
        micInitResult = HuMCInit(0);
        switch (micInitResult) {
        case -128:
            break;
        case -1:
            ASSERT(FALSE);
            break;
        case -3:
        case -2:
            break;
        case 0:
            break;
        default:
            break;
        }
        lbl_1_bss_B0 = HuMCContextCreate("/mic/ctx/micquiz_genre");
        lbl_1_bss_AE = HuMCContextCreate("/mic/ctx/micquiz_dokan");
    }
    MicQuizMicHudCreate();
    micQuizModeWork->pauseAllowed = 1;
    if (lbl_802C0248 == 0 || lbl_802C0248 == 2 || lbl_802C0248 == 3) {
        fn_800269E8(lbl_1_bss_23C, ((1 << 30) | (1 << 1)));
        if (lbl_802C0248 == 2 || lbl_802C0248 == 3) {
            fn_8003E488(4, &lbl_1_data_5DC);
        } else {
            fn_8003E488(4, &lbl_1_data_64C);
            fn_1_38B0();
        }
        for (index = 0; index < 5; index++) {
            for (column = 0; column < 5; column++) {
                if (column == 0) {
                    Hu3DAnimBankSet(lbl_1_bss_166[column][index], 0);
                    fn_1_E644(column, index, 1);
                } else {
                    Hu3DAnimBankSet(lbl_1_bss_166[column][index], 1);
                    fn_1_E670(column, index, 1);
                }
            }
        }
        if (micQuizModeWork->presenterLayout == 0) {
            fn_1_3BFC();
        }
        fn_1_3E7C();
        OSReport("-----------------------------First!!\n");
        lbl_1_bss_258 = 0;
    } else {
        if (MicQuizQuestionEventGet() == 7) {
            micQuizModeWork->questionColumn = micQuizModeWork->savedQuestionColumn;
        } else if (MicQuizQuestionEventGet() == 2 || MicQuizQuestionEventGet() == 5) {
            micQuizModeWork->questionLevel = micQuizModeWork->savedQuestionLevel;
        }
        fn_8003E488(4, &lbl_1_data_534);
        fn_1_19DD4(0, 1, 0);
        OSReport("-----------------------------Not First!!\n");
        lbl_1_bss_258 = 1;
        fn_1_E6C0(micQuizModeWork->questionLevel, micQuizModeWork->questionColumn,
                   micQuizModeWork->answeringPlayer, micQuizModeWork->roundResult);
        for (index = 0; index < 5; index++) {
            for (column = 0; column < 5; column++) {
                if (fn_1_E69C(column, index, 1) || fn_1_E69C(column, index, 2)) {
                    Hu3DAnimBankSet(lbl_1_bss_166[column][index], 0);
                } else {
                    Hu3DAnimBankSet(lbl_1_bss_166[column][index], 1);
                }
            }
        }
        fn_1_DDC4();
        fn_1_E474();
        if (micQuizModeWork->presenterLayout == 0) {
            fn_1_3BFC();
        }
        fn_1_3E7C();
        fn_800269E8(lbl_1_bss_238, (1 << 0));
        if (micQuizModeWork->presenterLayout == 0) {
            for (index = 0; index < 2; index++) {
                fn_80026760(lbl_1_bss_2A6[index], lbl_1_data_7E4[index].x,
                               lbl_1_data_7E4[index].y, lbl_1_data_7E4[index].z);
                fn_80026808(lbl_1_bss_2A6[index], lbl_1_data_7FC[index].x,
                               lbl_1_data_7FC[index].y, lbl_1_data_7FC[index].z);
            }
        }
    }
    lbl_1_bss_58 = HuSprAnimRead(fn_80008634(DATANUM(0xa20000, 51), 0x10000000));
    lbl_1_bss_54 = HuSprAnimRead(fn_80008634(DATANUM(0xa20000, 52), 0x10000000));
    OSReport("**************** MQ ObjectSetup End ******************\n");
    fn_8000DAC0(fn_1_7688, 100, 6144, 0, lbl_1_bss_288);
    fn_8000DAC0(fn_1_1E448, 100, 6144, 0, lbl_1_bss_288);
    MicQuizPauseInit(lbl_1_bss_288);
    MicQuizPauseDispCreate();
    micQuizModeWork->pauseAllowed = 1;
}

/* Initialize persistent quiz selections, scores, and character choices on the first overlay
 * entry. */
void fn_1_38B0(void) {
    s32 index;
    s32 playerIndex;

    micQuizModeWork->questionCount = 0;
    micQuizModeWork->currentPlayer = 0;
    micQuizModeWork->answeringPlayer = -1;
    micQuizModeWork->roundResult = 1;
    micQuizModeWork->roundPoints = 0;
    micQuizModeWork->questionLevel = 0;
    micQuizModeWork->questionColumn = 0;
    micQuizModeWork->compactPauseDisplay = 1;
    micQuizModeWork->pauseAllowed = 1;
    micQuizModeWork->randomEnabled = 1;
    micQuizModeWork->pauseDisplayActive = 0;

    for (index = 0; index < 5; index++) {
        for (playerIndex = 0; playerIndex < 5; playerIndex++) {
            micQuizModeWork->questionFlags[playerIndex][index] = 0;
        }
    }

    micQuizModeWork->questionEvent = 8;
    for (index = 0; index < 4; index++) {
        micQuizModeWork->playerScore[index] = 0;
        micQuizModeWork->playerCharacter[index] = lbl_80265728[index].charNo;
        micQuizModeWork->playerRank[index] = 1;
    }
    micQuizModeWork->presenterLayout = 0;
}

/* Replace or change a player panel model when the round presentation changes its motion mode. */
void fn_1_3A8C(s32 modelIndex, s32 motionMode) {
    if (motionMode == 0) {
        fn_800262D0(lbl_1_bss_20C[modelIndex]);
        lbl_1_bss_20C[modelIndex] =
            fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 87), 0x10000000, 2));
        fn_800269E8(lbl_1_bss_20C[modelIndex], ((1 << 30) | (1 << 0)));
        fn_80026B64(lbl_1_bss_20C[modelIndex], 4);
        fn_80026760(lbl_1_bss_20C[modelIndex], lbl_1_data_83C[modelIndex][0],
                        lbl_1_data_83C[modelIndex][1], lbl_1_data_83C[modelIndex][2]);
    } else if (motionMode == 1) {
        fn_8002C95C(lbl_1_bss_20C[modelIndex], lbl_1_bss_1E0);
    } else if (motionMode == 2) {
        fn_8002C95C(lbl_1_bss_20C[modelIndex], lbl_1_bss_1DE);
    }
}

/* Load the two presenter models, their motions, and the hooked item during scene setup. */
void fn_1_3BFC(void) {
    s32 motionIndex;
    s32 presenterIndex;

    presenterIndex = 0;
    while (presenterIndex < 2) {
        if (presenterIndex == 0) {
            lbl_1_bss_2A6[presenterIndex] = fn_80025A04(
                fn_80008DD0(DATANUM(0xa20000, 0), 0x10000000, 2));
        } else {
            lbl_1_bss_2A6[presenterIndex] = fn_80025A04(
                fn_80008DD0(DATANUM(0xa20000, 1), 0x10000000, 2));
        }
        motionIndex = 0;
        while (motionIndex < 4) {
            lbl_1_bss_296[presenterIndex][motionIndex] = fn_80030458(
                lbl_1_bss_2A6[presenterIndex],
                fn_80008DD0(lbl_1_data_448[presenterIndex][motionIndex],
                                     0x10000000, 2));
            motionIndex += 1;
        }
        fn_800269E8(lbl_1_bss_2A6[presenterIndex], ((1 << 30) | (1 << 0)));
        fn_8002C95C(lbl_1_bss_2A6[presenterIndex], lbl_1_bss_296[presenterIndex][0]);
        fn_80026760(lbl_1_bss_2A6[presenterIndex], lbl_1_data_7B4[presenterIndex].x,
                        lbl_1_data_7B4[presenterIndex].y, lbl_1_data_7B4[presenterIndex].z);
        fn_80026808(lbl_1_bss_2A6[presenterIndex], lbl_1_data_7CC[presenterIndex].x,
                        lbl_1_data_7CC[presenterIndex].y, lbl_1_data_7CC[presenterIndex].z);
        fn_80026B64(lbl_1_bss_2A6[presenterIndex], 4U);
        presenterIndex += 1;
    }
    lbl_1_bss_2A6[2] = fn_80025A04(
        fn_80008DD0(DATANUM(0xa20000, 2), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_2A6[2], 4U);
    fn_8002741C(lbl_1_bss_2A6[1], "gN01m1-itemhook_R", lbl_1_bss_2A6[2]);
}

/* Load the single-presenter model and its motions during scene setup. */
void fn_1_3E7C(void) {
    s32 index;

    lbl_1_bss_294 =
        fn_80025A04(fn_80008DD0(DATANUM(0xa20000, 11), 0x10000000, 2));
    for (index = 0; index < 4; index++) {
        lbl_1_bss_28C[index] = fn_80030458(
            lbl_1_bss_294,
            fn_80008DD0(lbl_1_data_468[index], 0x10000000, 2)
        );
    }
    /* Only the two-presenter layout uses the individual presenter models. */
    if (micQuizModeWork->presenterLayout == 0) {
        fn_800269E8(lbl_1_bss_294, (1 << 0));
    }
    fn_1_A4CC(0, 0.0f, ((1 << 30) | (1 << 0)));
    fn_80026760(lbl_1_bss_294, lbl_1_data_814.x, lbl_1_data_814.y, lbl_1_data_814.z);
    fn_80026808(lbl_1_bss_294, lbl_1_data_820.x, lbl_1_data_820.y, lbl_1_data_820.z);
    fn_80026B64(lbl_1_bss_294, 4);
}

/* Capture the first camera image into its texture after the camera layer draws. */
void fn_1_3FE4(s16 layerNo) {
    fn_1_407C(0, 0, 640, 480, 5, lbl_1_bss_30->bmp->data);
    fn_8002BF48();
}

/* Capture the second camera image into its texture after the camera layer draws. */
void fn_1_4030(s16 layerNo) {
    fn_1_407C(0, 0, 640, 480, 5, lbl_1_bss_34->bmp->data);
    fn_8002BF48();
}

/* Copy a framebuffer rectangle into a texture for the camera capture callbacks. */
void fn_1_407C(s16 x, s16 y, s32 width, s32 height, enum _GXTexFmt format,
                void *destination) {
    GXSetTexCopySrc((u16)x, (u16)y, (u16)width, (u16)height);
    GXSetTexCopyDst((u16)width, (u16)height, format, 0);
    GXCopyTex(destination, 1);
}

/* Play the opening camera sequence and presenter reactions before the round begins. */
void fn_1_40FC(void) {
    s32 playerNo;

    HuAudSStreamFadeOut(micQuizStreamNo, 1000);
    fn_1_1AFC4(30, 200);
    HuPrcSleep(30);
    fn_1_A3B4(0, 3, 8.0f, ((1 << 30) | (1 << 0)));
    fn_1_A3B4(1, 3, 8.0f, ((1 << 30) | (1 << 0)));
    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (micQuizModeWork->playerActive[playerNo] != 0) {
            fn_1_A678(playerNo, 4, 8.0f, ((1 << 30) | (1 << 0)));
        }
    }
    HuPrcSleep(45);
    fn_1_A838(lbl_1_data_614, 120, 4);
    micQuizStreamNo = HuAudSStreamPlay(MSM_STREAM_BOWSER_QUIZ);
    fn_80026A44(lbl_1_bss_234, ((1 << 30) | (1 << 1)));
    fn_80026A44(lbl_1_bss_236, ((1 << 30) | (1 << 1)));
    for (playerNo = 0; playerNo < 6; playerNo++) {
        fn_8002D730(lbl_1_bss_224[playerNo], 121.0f,
                              160.0f);
        fn_80026A44(lbl_1_bss_224[playerNo], ((1 << 30) | (1 << 0)));
    }
    do {
        HuPrcVSleep();
    } while (fn_8002D75C(lbl_1_bss_224[0]) == 0);
    for (playerNo = 0; playerNo < 6; playerNo++) {
        fn_8002D730(lbl_1_bss_224[playerNo], 161.0f,
                              280.0f);
        fn_800269E8(lbl_1_bss_224[playerNo], ((1 << 30) | (1 << 0)));
    }
    fn_8000DAC0(fn_1_48D8, 100, 6144, 0, lbl_1_bss_288);
    HuAudFXPlay(MSM_SE_MICQUIZ_OPENING_MODEL_TRANSITION);
    fn_800269E8(lbl_1_bss_232, (1 << 0));
    fn_80026A44(lbl_1_bss_230, ((1 << 30) | (1 << 1)));
    fn_80026A44(lbl_1_bss_230, (1 << 0));
    HuPrcSleep(120);
    HuAudFXPlay(MSM_SE_MICQUIZ_OPENING_MODEL_REVEAL);
    fn_80026A44(lbl_1_bss_1F8, (1 << 0));
    fn_80026A44(lbl_1_bss_24A, (1 << 0));
    HuPrcSleep(60);
    fn_800269E8(lbl_1_bss_254, (1 << 0));
    fn_80026A44(lbl_1_bss_252, (1 << 0));
    fn_800269E8(lbl_1_bss_24A, (1 << 0));
    HuPrcSleep(60);
    fn_1_1AED8(30);
    HuPrcSleep(30);
    fn_1_A838(lbl_1_data_5C0, 120, 4);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        fn_80026A44(lbl_1_bss_214[playerNo], ((1 << 30) | (1 << 1)));
    }
    HuPrcSleep(45);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        fn_80026A44(lbl_1_bss_204[playerNo], ((1 << 30) | (1 << 1)));
        fn_80026A44(lbl_1_bss_204[playerNo], (1 << 0));
        fn_800269E8(lbl_1_bss_214[playerNo], ((1 << 30) | (1 << 1)));
    }
    fn_8000DAC0(fn_1_4868, 100, 4096, 0, lbl_1_bss_288);
    fn_80026A44(lbl_1_bss_220, ((1 << 30) | (1 << 1)));
    HuPrcSleep(120);
    fn_1_A838(lbl_1_data_5F8, 120, 4);
    fn_8000DAC0(fn_1_4808, 100, 4096, 0, lbl_1_bss_288);
    fn_80026A44(lbl_1_bss_222, ((1 << 30) | (1 << 1)));
    HuPrcSleep(150);
    fn_80026A44(lbl_1_bss_21C, ((1 << 30) | (1 << 1)));
    fn_8002C95C(lbl_1_bss_294, lbl_1_bss_28C[2]);
    fn_8002D414(lbl_1_bss_294, 100.0f);
    fn_80026A44(lbl_1_bss_294, (1 << 0));
    HuPrcSleep(30);
    fn_1_1E3F8(60, 25.0f, 0.85f, 90.0f);
    HuAudFXPlay(MSM_SE_BRD00_KOOPA_APPEAR);
    fn_1_A3B4(0, 2, 8.0f, 0);
    fn_1_A3B4(1, 2, 8.0f, 0);
    HuPrcSleep(8);
    fn_8000DAC0(fn_1_4B00, 100, 4096, 0, lbl_1_bss_288);
    fn_8000DAC0(fn_1_4CB8, 100, 4096, 0, lbl_1_bss_288);
    HuPrcSleep(70);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (micQuizModeWork->playerActive[playerNo] != 0) {
            fn_1_A678(playerNo, 0, 8.0f, ((1 << 30) | (1 << 0)));
        }
    }
    fn_1_A4CC(0, 8.0f, ((1 << 30) | (1 << 0)));
    HuPrcSleep(8);
    fn_800269E8(lbl_1_bss_1F8, (1 << 0));
}

/* Play a five-cue sound sequence in a presentation child process, then end it. */
void fn_1_4808(void) {
    HuAudFXPlay(MSM_SE_MICQUIZ_PRESENTER_SEQUENCE_BEAT);
    HuPrcSleep(22);
    HuAudFXPlay(MSM_SE_MICQUIZ_PRESENTER_SEQUENCE_BEAT);
    HuPrcSleep(15);
    HuAudFXPlay(MSM_SE_MICQUIZ_PRESENTER_SEQUENCE_BEAT);
    HuPrcSleep(4);
    HuAudFXPlay(MSM_SE_MICQUIZ_PRESENTER_SEQUENCE_BEAT);
    HuPrcSleep(29);
    HuAudFXPlay(MSM_SE_MICQUIZ_PRESENTER_SEQUENCE_BEAT);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Play the alternate six-cue sound sequence in a presentation child process, then end it. */
void fn_1_4868(void) {
    HuAudFXPlay(MSM_SE_MICQUIZ_PRESENTER_SEQUENCE_BEAT);
    HuPrcSleep(5);
    HuAudFXPlay(MSM_SE_MICQUIZ_PRESENTER_SEQUENCE_BEAT);
    HuPrcSleep(12);
    HuAudFXPlay(MSM_SE_MICQUIZ_PRESENTER_SEQUENCE_BEAT);
    HuPrcSleep(18);
    HuAudFXPlay(MSM_SE_MICQUIZ_PRESENTER_SEQUENCE_BEAT);
    HuPrcSleep(20);
    HuAudFXPlay(MSM_SE_MICQUIZ_PRESENTER_SEQUENCE_BEAT);
    HuPrcSleep(15);
    HuAudFXPlay(MSM_SE_MICQUIZ_PRESENTER_SEQUENCE_BEAT);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Pulse the overlay sprite opacity through a full cycle in a temporary presentation process. */
void fn_1_48D8(void)
{
    f64 opacity;
    s32 angle;

    HuAudFXPlay(MSM_SE_BRD00_BANNER_SWOOSH_IN);
    HuSprAttrReset(micQuizFlashGroup, 0, HUSPR_ATTR_DISPOFF);
    angle = 0;
    while (angle < 360) {
        if (sin((M_PI * fn_1_4A68((f32)angle)) / 180.0) < 0.0) {
            opacity = -sin((M_PI * fn_1_4A68((f32)angle)) / 180.0);
        } else {
            opacity = sin((M_PI * fn_1_4A68((f32)angle)) / 180.0);
        }
        HuSprTPLvlSet(micQuizFlashGroup, 0, (f32)opacity);
        HuPrcVSleep();
        angle += 2;
    }
    HuSprAttrSet(micQuizFlashGroup, 0, HUSPR_ATTR_DISPOFF);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Wrap animation angles into the signed half-turn range before trigonometric evaluation. */
f32 fn_1_4A68(f32 angle)
{
    angle = (f32)fmod(angle, 360.0);
    if (angle < -180.0f) {
        angle += 360.0f;
    } else if (angle > 180.0f) {
        angle -= 360.0f;
    }
    return angle;
}

/* Move and spin the left presenter out of the scene during the opening transition. */
void fn_1_4B00(void)
{
    HuVecF position;
    HuVecF rotation;
    HuVecF positionStep;
    HuVecF rotationStep;
    s32 frame;

    fn_800267CC(lbl_1_bss_2A6[0], &position);
    fn_80026874(lbl_1_bss_2A6[0], &rotation);
    positionStep.x = 0.0f;
    positionStep.y = 10.0f;
    positionStep.z = 8.0f;
    rotationStep.x = 5.0f;
    rotationStep.y = 0.0f;
    rotationStep.z = 10.0f;
    frame = 0;
    for (frame = 0; frame < 120; frame++) {
        HuPrcVSleep();
        if (fn_8002D4E0(lbl_1_bss_2A6[0]) > 10.0f) {
            fn_800269E8(lbl_1_bss_2A6[0], ((1 << 30) | (1 << 1)));
        }
        position.x += positionStep.x;
        position.y += positionStep.y;
        position.z += positionStep.z;
        rotation.x += rotationStep.x;
        rotation.y += rotationStep.y;
        rotation.z += rotationStep.z;
        fn_80026760(lbl_1_bss_2A6[0], position.x, position.y, position.z);
        fn_80026808(lbl_1_bss_2A6[0], rotation.x, rotation.y, rotation.z);
    }
    fn_800269E8(lbl_1_bss_2A6[0], (1 << 0));
    HuPrcVSleep();
    HuPrcEnd();
}

/* Move and spin the right presenter out of the scene during the opening transition. */
void fn_1_4CB8(void)
{
    HuVecF position;
    HuVecF rotation;
    HuVecF positionStep;
    HuVecF rotationStep;
    s32 frame;

    fn_800267CC(lbl_1_bss_2A6[1], &position);
    fn_80026874(lbl_1_bss_2A6[1], &rotation);
    positionStep.x = 0.0f;
    positionStep.y = 10.0f;
    positionStep.z = -8.0f;
    rotationStep.x = 5.0f;
    rotationStep.y = 0.0f;
    rotationStep.z = -10.0f;
    frame = 0;
    for (frame = 0; frame < 120; frame++) {
        HuPrcVSleep();
        if (fn_8002D4E0(lbl_1_bss_2A6[1]) > 10.0f) {
            fn_800269E8(lbl_1_bss_2A6[1], ((1 << 30) | (1 << 1)));
        }
        position.x += positionStep.x;
        position.y += positionStep.y;
        position.z += positionStep.z;
        rotation.x += rotationStep.x;
        rotation.y += rotationStep.y;
        rotation.z += rotationStep.z;
        fn_80026760(lbl_1_bss_2A6[1], position.x, position.y, position.z);
        fn_80026808(lbl_1_bss_2A6[1], rotation.x, rotation.y, rotation.z);
    }
    fn_800269E8(lbl_1_bss_2A6[1], (1 << 0));
    HuPrcVSleep();
    HuPrcEnd();
}

/* Present the final score order and winner reactions, then return the replay choice. */
s32 fn_1_4E70(void)
{
    s32 statusCounts[4];
    s32 scoreOrder[4];
    OM_CAMERA_VIEW *resultCameraView;
    s32 index;
    s32 activeCount;
    s32 scoreIndex;
    s32 lastPlaceRank;
    f32 angle;
    f32 transparency;

    for (activeCount = 0; activeCount < 4; activeCount++) {
        index = 0;
        statusCounts[activeCount] = 0;
        while (index < 4) {
            if (micQuizModeWork->playerRank[index] == 1) {
                statusCounts[activeCount]++;
            }
            index++;
        }
    }
    fn_1_8488();
    fn_1_A838(lbl_1_data_5F8, 120, 4);
    fn_1_19DD4(1, 1, 0);
    if (micQuizModeWork->presenterLayout == 0) {
        fn_800269E8(lbl_1_bss_250, (1 << 0));
    } else {
        fn_800269E8(lbl_1_bss_24E, (1 << 0));
    }
    fn_80026A44(lbl_1_bss_1D2, (1 << 0));
    for (index = 0, activeCount = 0; index < 4; index++) {
        if (micQuizModeWork->playerActive[index] != 0) {
            fn_80026760(
                lbl_1_bss_2FC[index],
                lbl_1_data_698[micQuizModeWork->playerCount - 1]
                    .playerPositions[activeCount].x,
                lbl_1_data_698[micQuizModeWork->playerCount - 1]
                    .playerPositions[activeCount].y,
                lbl_1_data_698[micQuizModeWork->playerCount - 1]
                    .playerPositions[activeCount].z);
            activeCount++;
            fn_80026808(lbl_1_bss_2FC[index], 0.0f, 0.0f, 0.0f);
        }
    }
    fn_1_1614C();
    fn_1_16620();
    fn_1_5EDC();
    lbl_1_bss_78 = HuAudFXPlay(MSM_SE_CMN_DRUMROLL);
    HuAudSStreamFadeOut(micQuizStreamNo, 1200);
    fn_1_5FA0();
    fn_1_A838(lbl_1_data_630, 120, 4);
    fn_1_6628();
    for (index = 0; index < 4; index++) {
        scoreOrder[index] = micQuizModeWork->playerScore[index];
    }
    fn_1_A97C(scoreOrder, 4);
    for (index = 0; index < 4; index++) {
        lbl_1_bss_7C[index] = -1;
    }
    for (index = 0, activeCount = 0; index < 4; index++) {
        if (micQuizModeWork->playerScore[index] == scoreOrder[0]) {
            lbl_1_bss_7C[activeCount] = index;
            activeCount++;
        }
    }
    for (index = 0, activeCount = 0; index < 4; index++) {
        if (micQuizModeWork->playerActive[index] != 0) {
            fn_80026760(
                lbl_1_bss_1F0[index],
                lbl_1_data_698[micQuizModeWork->playerCount - 1]
                    .playerPositions[activeCount].x,
                lbl_1_data_698[micQuizModeWork->playerCount - 1]
                    .playerPositions[activeCount].y,
                lbl_1_data_698[micQuizModeWork->playerCount - 1]
                    .playerPositions[activeCount].z);
            activeCount++;
            fn_80026808(lbl_1_bss_1F0[index], 0.0f, 0.0f, 0.0f);
        }
    }
    if (fn_1_632C() != 0) {
        fn_1_5FA0();
        fn_1_18168();
        fn_1_5B74();
        micQuizStreamNo = HuAudSStreamPlay(MSM_STREAM_TOO_BAD);
    } else {
        if (fn_1_632C() == 0) {
            fn_1_16A24();
            fn_1_16ECC(lbl_1_bss_7C[0]);
        }
        HuAudFXPlay(MSM_SE_MICQUIZ_RESULTS_ANNOUNCE);
        if (micQuizModeWork->playerCount == statusCounts[0]) {
            fn_80026A44(lbl_1_bss_1FA[lbl_1_bss_7C[0]], (1 << 0));
            fn_80026A44(lbl_1_bss_1FA[lbl_1_bss_7C[1]], (1 << 0));
            fn_80026A44(lbl_1_bss_1FA[lbl_1_bss_7C[2]], (1 << 0));
            fn_80026A44(lbl_1_bss_1FA[lbl_1_bss_7C[3]], (1 << 0));
            fn_1_17B6C();
        } else {
            switch (statusCounts[0]) {
            case 1:
                fn_80026A44(lbl_1_bss_1FA[lbl_1_bss_7C[0]], (1 << 0));
                fn_1_171CC(lbl_1_bss_7C[0], lbl_1_bss_7C[1], lbl_1_bss_7C[2], lbl_1_bss_7C[3]);
                break;
            case 2:
                fn_80026A44(lbl_1_bss_1FA[lbl_1_bss_7C[0]], (1 << 0));
                fn_80026A44(lbl_1_bss_1FA[lbl_1_bss_7C[1]], (1 << 0));
                fn_1_1745C(lbl_1_bss_7C[0], lbl_1_bss_7C[1], lbl_1_bss_7C[2], lbl_1_bss_7C[3]);
                break;
            case 3:
                fn_80026A44(lbl_1_bss_1FA[lbl_1_bss_7C[0]], (1 << 0));
                fn_80026A44(lbl_1_bss_1FA[lbl_1_bss_7C[1]], (1 << 0));
                fn_80026A44(lbl_1_bss_1FA[lbl_1_bss_7C[2]], (1 << 0));
                fn_1_1778C(lbl_1_bss_7C[0], lbl_1_bss_7C[1], lbl_1_bss_7C[2], lbl_1_bss_7C[3]);
                break;
            }
        }
        HuAudFXPlay(MSM_SE_CMN_DRUMROLL_END_HIT);
        lbl_1_bss_74 = HuAudJinglePlay(MSM_STREAM_QUIZ_MASTER);
        do {
            HuPrcVSleep();
        } while (HuAudSStreamStatGet(lbl_1_bss_74) != 0);
        lbl_1_bss_74 = 0;
        for (index = 0; index < 4; index++) {
            fn_800269E8(lbl_1_bss_1FA[lbl_1_bss_7C[index]], (1 << 0));
        }
        micQuizStreamNo = HuAudSStreamPlay(MSM_STREAM_WELL_DONE);
        fn_80026CBC(lbl_1_bss_1D0, 0.0f);
        fn_80026A44(lbl_1_bss_1D0, (1 << 0));
        angle = 0.0;
        for (index = 0; index < 10; index++) {
            HuPrcVSleep();
            transparency = (f32)sin((M_PI * angle) / 180.0);
            fn_80026CBC(lbl_1_bss_1D0, transparency);
            angle += 9.0f;
        }
        fn_800269E8(lbl_1_bss_1D2, (1 << 0));
        angle = 90.0f;
        for (index = 0; index < 10; index++) {
            HuPrcVSleep();
            transparency = (f32)sin((M_PI * angle) / 180.0);
            fn_80026CBC(lbl_1_bss_1D0, transparency);
            angle += 9.0f;
        }
        fn_80026CBC(lbl_1_bss_1D0, 0.0f);
        fn_800269E8(lbl_1_bss_1D0, (1 << 0));
        fn_80026A44(lbl_1_bss_1D6, (1 << 0));
        fn_80026A44(lbl_1_bss_1D4, (1 << 0));
        fn_8000DAC0(fn_1_62C8, 100, 6144, 0, lbl_1_bss_288);
        HuPrcSleep(10);
        fn_80026A44(lbl_1_bss_1D8, (1 << 0));
        fn_80026A44(lbl_1_bss_1D8, ((1 << 30) | (1 << 1)));
        HuPrcSleep(45);
        fn_1_5B74();
        angle = 90.0f;
        for (index = 0; index < 20; index++) {
            HuPrcVSleep();
            transparency = (f32)sin((M_PI * angle) / 180.0);
            fn_80026CBC(lbl_1_bss_1D0, transparency);
            angle += 4.5f;
        }
        fn_8000DAC0(fn_1_6514, 100, 4096, 0, lbl_1_bss_288);
        for (index = 0; index < 4; index++) {
            if (lbl_1_bss_7C[index] != -1) {
                fn_1_A678(lbl_1_bss_7C[index], 1, 8.0f, 0);
            }
        }
        HuPrcSleep(180);
        lastPlaceRank = fn_1_6478();
        if (micQuizModeWork->presenterLayout != 0 && fn_1_640C() == 0) {
            fn_1_186E8();
            HuPrcSleep(120);
            fn_1_18800();
            HuPrcSleep(120);
            for (index = 0; index < 4; index++) {
                if (micQuizModeWork->playerActive[index] != 0
                    && micQuizModeWork->playerRank[index] == lastPlaceRank) {
                    fn_80026A44(lbl_1_bss_1F0[index], (1 << 0));
                    HuAudFXPlay(MSM_SE_MICQUIZ_PLAYER_CELEBRATE);
                    fn_1_A678(index, 3, 16.0f, 0);
                }
            }
            HuPrcSleep(93);
            for (index = 0; index < 4; index++) {
                if (micQuizModeWork->playerActive[index] != 0
                    && micQuizModeWork->playerRank[index] == lastPlaceRank) {
                    fn_800269E8(lbl_1_bss_1F0[index], (1 << 0));
                    fn_1_A678(index, 6, 8.0f, ((1 << 30) | (1 << 0)));
                }
            }
        }
    }
    fn_1_17D68();
    if (MicQuizWinChoiceGet(-1) == 0) {
        return 0;
    }
    fn_1_17F64();
    return 1;
}

/* Brighten the three scene lights over 120 scheduler frames during a presentation transition. */
void fn_1_5B74(void) {
    s32 frame;

    frame = 0;
    while (frame < 120) {
        HuPrcVSleep();
        lbl_1_bss_284 = (s32) ((f32) lbl_1_bss_284 + (2.0f * lbl_1_bss_274));
        if (lbl_1_bss_284 > 255) {
            lbl_1_bss_284 = 255;
        }
        lbl_1_bss_280 = (s32) ((f32) lbl_1_bss_280 + (2.0f * lbl_1_bss_274));
        if (lbl_1_bss_280 > 255) {
            lbl_1_bss_280 = 255;
        }
        lbl_1_bss_27C = (s32) ((f32) lbl_1_bss_27C + (2.0f * lbl_1_bss_274));
        if (lbl_1_bss_27C > 255) {
            lbl_1_bss_27C = 255;
        }
        lbl_1_bss_278 = (s32) ((f32) lbl_1_bss_278 + (2.0f * lbl_1_bss_274));
        if (lbl_1_bss_278 > 255) {
            lbl_1_bss_278 = 255;
        }
        Hu3DGLightColorSet((s16) lbl_1_bss_264, (u8) lbl_1_bss_284, (u8) lbl_1_bss_280,
                           (u8) lbl_1_bss_27C, (u8) lbl_1_bss_278);
        Hu3DGLightColorSet((s16) lbl_1_bss_260, (u8) lbl_1_bss_284, (u8) lbl_1_bss_280,
                           (u8) lbl_1_bss_27C, (u8) lbl_1_bss_278);
        Hu3DGLightColorSet((s16) lbl_1_bss_25C, (u8) lbl_1_bss_284, (u8) lbl_1_bss_280,
                           (u8) lbl_1_bss_27C, (u8) lbl_1_bss_278);
        frame += 1;
    }
}

/* Restore the scene light color and fade rates before a lighting transition. */
void fn_1_5EDC(void) {
    lbl_1_bss_284 = (s32) lbl_1_data_4F8.r;
    lbl_1_bss_280 = (s32) lbl_1_data_4F8.g;
    lbl_1_bss_27C = (s32) lbl_1_data_4F8.b;
    lbl_1_bss_278 = (s32) lbl_1_data_4F8.a;
    lbl_1_bss_274 = 2.0f;
    lbl_1_bss_270 = 2.0f;
    lbl_1_bss_26C = 2.0f;
    lbl_1_bss_268 = 2.0f;
}

/* Fade the three scene lights toward black over 120 scheduler frames during a transition. */
void fn_1_5FA0(void) {
    s32 frame;

    frame = 0;
    while (frame < 120) {
        HuPrcVSleep();
        lbl_1_bss_284 = (s32) ((f32) lbl_1_bss_284 - lbl_1_bss_274);
        if (lbl_1_bss_284 < 0) {
            lbl_1_bss_284 = 0;
        }
        lbl_1_bss_280 = (s32) ((f32) lbl_1_bss_280 - lbl_1_bss_274);
        if (lbl_1_bss_280 < 0) {
            lbl_1_bss_280 = 0;
        }
        lbl_1_bss_27C = (s32) ((f32) lbl_1_bss_27C - lbl_1_bss_274);
        if (lbl_1_bss_27C < 0) {
            lbl_1_bss_27C = 0;
        }
        lbl_1_bss_278 = (s32) ((f32) lbl_1_bss_278 - lbl_1_bss_274);
        if (lbl_1_bss_278 < 0) {
            lbl_1_bss_278 = 0;
        }
        Hu3DGLightColorSet((s16) lbl_1_bss_264, (u8) lbl_1_bss_284, (u8) lbl_1_bss_280,
                           (u8) lbl_1_bss_27C, (u8) lbl_1_bss_278);
        Hu3DGLightColorSet((s16) lbl_1_bss_260, (u8) lbl_1_bss_284, (u8) lbl_1_bss_280,
                           (u8) lbl_1_bss_27C, (u8) lbl_1_bss_278);
        Hu3DGLightColorSet((s16) lbl_1_bss_25C, (u8) lbl_1_bss_284, (u8) lbl_1_bss_280,
                           (u8) lbl_1_bss_27C, (u8) lbl_1_bss_278);
        frame += 1;
    }
}

/* Rotate the results model continuously in the child process started for the winner sequence. */
void fn_1_62C8(void) {
    f32 rotation;

    rotation = 0.0f;
    for (;;) {
        HuPrcVSleep();
        fn_80026808(lbl_1_bss_1D8, 0.0f, rotation,
                        0.0f);
        rotation += 0.6f;
    }
}

/* Report whether every active player has a zero score when choosing result effects. */
s32 fn_1_632C(void) {
    s32 index;

    for (index = 0; index < 4; index++) {
        if (micQuizModeWork->playerActive[index] != 0
            && micQuizModeWork->playerScore[index] != 0) {
            return 0;
        }
    }
    return 1;
}

/* Report whether every active player has a nonzero score when choosing result effects. */
s32 fn_1_639C(void) {
    s32 index;

    for (index = 0; index < 4; index++) {
        if (micQuizModeWork->playerActive[index] != 0
            && micQuizModeWork->playerScore[index] == 0) {
            return 0;
        }
    }
    return 1;
}

/* Report whether every active player shares first place during result presentation. */
s32 fn_1_640C(void) {
    s32 index;

    for (index = 0; index < 4; index++) {
        if (micQuizModeWork->playerActive[index] != 0
            && micQuizModeWork->playerRank[index] != 1) {
            return 0;
        }
    }
    return 1;
}

/* Find the lowest finishing rank among active players for the final reaction sequence. */
s32 fn_1_6478(void) {
    u8 maximum;
    u8 index;

    maximum = 1;
    index = 0;
    while (index < 4) {
        if (micQuizModeWork->playerActive[index] != 0
            && micQuizModeWork->playerRank[index] > maximum) {
            maximum = micQuizModeWork->playerRank[index];
        }
        index++;
    }
    return maximum;
}

/* Emit falling model pieces above the listed winners every 40 frames during results. */
void fn_1_6514(void) {
    HuVecF position;
    s32 playerNo;
    s32 activePlayerCount;

    while (1) {
        playerNo = 0;
        activePlayerCount = 0;
        while (playerNo < 4) {
            if (lbl_1_bss_7C[playerNo] != -1) {
                position.x = lbl_1_data_698[micQuizModeWork->playerCount - 1]
                                 .playerPositions[lbl_1_bss_7C[activePlayerCount]].x;
                position.y = 480.0f;
                position.z = 100.0f
                    + lbl_1_data_698[micQuizModeWork->playerCount - 1]
                          .playerPositions[lbl_1_bss_7C[activePlayerCount]].z;
                activePlayerCount++;
                fn_1_32250(&position, 5);
            }
            playerNo++;
        }
        HuPrcSleep(40);
    }
}

/* Widen, sway, and hide the center model during the final score presentation. */
void fn_1_6628(void) {
    f32 angle;
    f32 sineValue;
    f32 positionOffset;
    s32 playerNo;

    fn_80026760(lbl_1_bss_202, 0.0f, 0.0f,
                    -350.0f);
    fn_800268B0(lbl_1_bss_202, 0.0f, 1.0f,
                      1.0f);
    fn_80026A44(lbl_1_bss_202, (1 << 0));

    angle = 0.0f;
    sineValue = 0.0f;
    for (;;) {
        HuPrcVSleep();
        sineValue = (f32)sin((M_PI * (f64)angle) / 180.0);
        fn_800268B0(lbl_1_bss_202, sineValue, 1.0f,
                          1.0f);
        angle += 3.0f;
        if (angle >= 90.0f) {
            break;
        }
    }
    fn_800268B0(lbl_1_bss_202, 1.0f, 1.0f,
                      1.0f);

    angle = 0.0f;
    positionOffset = 0.0f;
    for (;;) {
        HuPrcVSleep();
        positionOffset = (f32)(220.0
            * sin((M_PI * (f64)angle) / 180.0));
        fn_80026760(lbl_1_bss_202, positionOffset, 0.0f,
                        -350.0f);
        angle += 1.5f;
        if (angle >= 720.0f) {
            break;
        }
    }

    angle = 90.0f;
    sineValue = 0.0f;
    if (fn_1_632C() != 0) {
        HuAudFXStop(lbl_1_bss_78);
        HuAudFXPlay(MSM_SE_MICQUIZ_RESULT_MARKER);
    }

    for (;;) {
        HuPrcVSleep();
        sineValue = (f32)sin((M_PI * (f64)angle) / 180.0);
        fn_800268B0(lbl_1_bss_202, sineValue, 1.0f,
                          1.0f);
        angle += 3.0f;
        if (angle >= 180.0f) {
            break;
        }
    }
    fn_800268B0(lbl_1_bss_202, 0.0f, 0.0f,
                      0.0f);
    fn_800269E8(lbl_1_bss_202, (1 << 0));
}

/* Select the quiz presentation mode and show its introduction before play continues. */
void fn_1_69D0(u32 selection) {
    switch (selection) {
    case 0:
        fn_1_E7F4(0);
        MicQuizWinOpen(0);
        MicQuizWinMesSet(micQuizSelectionMes[selection], 1, 0);
        MicQuizWinMesWait();
        MicQuizWinClose();
        break;
    case 1:
        fn_1_E7F4(1);
        MicQuizWinOpen(1);
        MicQuizWinMesSet(micQuizSelectionMes[selection], 1, 1);
        MicQuizWinMesWait();
        MicQuizWinClose();
        break;
    case 2:
        fn_1_E7F4(2);
        MicQuizWinOpen(0);
        MicQuizWinMesSet(micQuizSelectionMes[selection], 1, 0);
        MicQuizWinMesWait();
        MicQuizWinClose();
        break;
    case 3:
        fn_1_E7F4(3);
        MicQuizWinOpen(1);
        MicQuizWinMesSet(micQuizSelectionMes[selection], 1, 1);
        MicQuizWinMesWait();
        MicQuizWinClose();
        break;
    case 4:
        fn_1_E7F4(4);
        MicQuizWinOpen(3);
        MicQuizWinMesSet(micQuizSelectionMes[selection], 1, 3);
        MicQuizWinMesWait();
        MicQuizWinClose();
        break;
    case 5:
        fn_1_E7F4(5);
        MicQuizWinOpen(3);
        MicQuizWinMesSet(micQuizSelectionMes[selection], 1, 3);
        MicQuizWinMesWait();
        MicQuizWinClose();
        break;
    case 6:
        fn_1_E7F4(6);
        MicQuizWinOpen(3);
        MicQuizWinMesSet(micQuizSelectionMes[selection], 1, 3);
        MicQuizWinMesWait();
        MicQuizWinClose();
        break;
    case 7:
        fn_1_E7F4(7);
        micQuizModeWork->savedQuestionColumn = micQuizModeWork->questionColumn;
        micQuizModeWork->questionColumn = 5;
        MicQuizWinOpen(3);
        MicQuizWinMesSet(micQuizSelectionMes[selection], 1, 3);
        MicQuizWinMesWait();
        MicQuizWinClose();
    }
}

/* Hide the presenters and active player display models while switching scenes. */
void fn_1_74C0(void) {
    s32 playerIndex;

    if (micQuizModeWork->presenterLayout == 0) {
        fn_800269E8(lbl_1_bss_2A6[0], (1 << 0));
        fn_800269E8(lbl_1_bss_2A6[1], (1 << 0));
    } else {
        fn_800269E8(lbl_1_bss_294, (1 << 0));
    }
    playerIndex = 0;
    while (playerIndex < 4) {
        fn_800269E8(lbl_1_bss_214[playerIndex], (1 << 0));
        if (micQuizModeWork->playerActive[playerIndex] != 0) {
            fn_800269E8(lbl_1_bss_2FC[playerIndex], (1 << 0));
        }
        playerIndex += 1;
    }
}

/* Show the presenters and active player display models after switching scenes. */
void fn_1_75A4(void) {
    s32 playerIndex;

    if (micQuizModeWork->presenterLayout == 0) {
        fn_80026A44(lbl_1_bss_2A6[0], (1 << 0));
        fn_80026A44(lbl_1_bss_2A6[1], (1 << 0));
    } else {
        fn_80026A44(lbl_1_bss_294, (1 << 0));
    }
    playerIndex = 0;
    while (playerIndex < 4) {
        fn_80026A44(lbl_1_bss_214[playerIndex], (1 << 0));
        if (micQuizModeWork->playerActive[playerIndex] != 0) {
            fn_80026A44(lbl_1_bss_2FC[playerIndex], (1 << 0));
        }
        playerIndex += 1;
    }
}

/* Drive the quiz overlay from its opening through question selection and the next quiz overlay. */
void fn_1_7688(void) {
    OMOVLHIS *history;
    s32 completionStep;
    s32 result;
    s32 currentPlayer;
    OM_CAMERA_VIEW *resultCameraView;
    s32 playerIndex;

    history = omOvlHisGet(0);
    if (lbl_1_bss_258 == 0) {
        micQuizStreamNo = HuAudSStreamPlay(MSM_STREAM_SPEAK_UP_THEME);
        fn_1_A790(0, 60);
        HuPrcSleep(10);
        if (lbl_802C0248 == 0) {
            fn_1_A838(lbl_1_data_5DC, 120, 4);
        }
        fn_800269E8(lbl_1_bss_238, (1 << 0));
        if (lbl_802C0248 == 0) {
            fn_1_E91C();
        }
        if (lbl_802C0248 == 0 && fn_1_EF58() == 0) {
                fn_1_A3B4(1, 0, 8.0f, ((1 << 30) | (1 << 0)));
                micQuizModeWork->pauseAllowed = 0;
                fn_1_1B9D4();
                micQuizModeWork->pauseAllowed = 1;
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
                fn_1_A790(0, 20);
                omOvlHisChg(0, DLL_micquizdll, 2, history->stat);
                micQuizModeWork->questionColumn = 6;
                micQuizModeWork->currentPlayer = 0;
                micQuizModeWork->questionEvent = 3;
                micQuizModeWork->compactPauseDisplay = 0;
                omOvlCallEx(DLL_micquizishidll, 1, 0, 0);
                for (;;) {
                    HuPrcVSleep();
                }
        }
        if ((lbl_802C0248 == 2 || lbl_802C0248 == 0) && fn_1_F084() == 0) {
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
                fn_1_A790(0, 20);
                omOvlHisChg(0, DLL_micquizdll, 3, history->stat);
                micQuizModeWork->questionColumn = 7;
                micQuizModeWork->currentPlayer = 0;
                micQuizModeWork->questionEvent = 8;
                omOvlCallEx(DLL_micquizishidll, 1, 0, 0);
                for (;;) {
                    HuPrcVSleep();
                }
        }

        if (micQuizActiveWin != -1) {
            HuWinExClose(micQuizActiveWin);
            micQuizActiveWin = -1;
        }
        omOvlHisChg(0, DLL_micquizdll, 1, history->stat);
        fn_1_A3B4(1, 0, 8.0f, ((1 << 30) | (1 << 0)));
        fn_1_F1B0();
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (micQuizModeWork->playerActive[playerIndex] != 0) {
                s32 modelIndex;

                fn_1_A838(lbl_1_data_550[playerIndex], 45, 4);
                for (modelIndex = 0; modelIndex < 2; modelIndex++) {
                    fn_80026760(lbl_1_bss_2A6[modelIndex],
                                    lbl_1_data_7E4[modelIndex].x,
                                    lbl_1_data_7E4[modelIndex].y,
                                    lbl_1_data_7E4[modelIndex].z);
                    fn_80026808(lbl_1_bss_2A6[modelIndex],
                                    lbl_1_data_7FC[modelIndex].x,
                                    lbl_1_data_7FC[modelIndex].y,
                                    lbl_1_data_7FC[modelIndex].z);
                }
                HuAudFXPlay(lbl_1_data_388[micQuizModeWork->playerCharacter[playerIndex]]);
                fn_1_3A8C(playerIndex, 1);
                fn_1_A678(playerIndex, 5, 8.0f, 0);
                fn_1_F3CC(playerIndex);
                HuPrcSleep(8);
                fn_1_A748(playerIndex);
                if (micQuizActiveWin != -1) {
                    HuWinExClose(micQuizActiveWin);
                    micQuizActiveWin = -1;
                }
                fn_1_A678(playerIndex, 0, 16.0f, ((1 << 30) | (1 << 0)));
                fn_1_3A8C(playerIndex, 0);
            }
        }
        if (micQuizActiveWin != -1) {
            HuWinExClose(micQuizActiveWin);
            micQuizActiveWin = -1;
        }
        {
            fn_1_A838(lbl_1_data_5C0, 60, 4);
        }
        fn_800269E8(lbl_1_bss_21E, ((1 << 30) | (1 << 1)));
        fn_1_F568();
        HuAudSStreamFadeOut(micQuizStreamNo, 2000);
        fn_1_19D5C();
        micQuizModeWork->questionCount = 1;
        fn_1_3A8C(micQuizModeWork->currentPlayer, 2);
        {
            fn_1_A838(lbl_1_data_534, 90, 4);
        }
        fn_1_8520();
        fn_1_19DD4(0, 1, 1);
        micQuizStreamNo = HuAudSStreamPlay(MSM_STREAM_QUIZ_SONG);
        fn_1_F6B4();
        if (HuMCMicGet() == 1) {
            MicQuizMicHudShow(micQuizModeWork->currentPlayer);
        }
        while (fn_1_C458() == -1) {
        }
        fn_1_8234();
        for (;;) {
            HuPrcVSleep();
        }
    } else {
    fn_1_83F0();
    fn_1_A790(0, 60);
    if (micQuizModeWork->presenterLayout == 0) {
        micQuizStreamNo = HuAudSStreamPlay(MSM_STREAM_QUIZ_SONG);
    } else {
        micQuizStreamNo = HuAudSStreamPlay(MSM_STREAM_MICQUIZ_ALTERNATE_GRID_SELECTION);
    }
    HuPrcSleep(10);
    if (fn_1_8590() == 0) {
        fn_1_A838(lbl_1_data_5C0, 60, 4);
    }
    fn_1_ABF4();
    fn_1_E80C();
    {
    micQuizModeWork->questionCount++;
    do {
        micQuizModeWork->currentPlayer++;
        if (micQuizModeWork->currentPlayer == 4) {
            micQuizModeWork->currentPlayer = 0;
        }
        currentPlayer = micQuizModeWork->currentPlayer;
    } while (micQuizModeWork->playerActive[currentPlayer] == 0);
    }
    fn_1_A898();
    if (micQuizModeWork->questionCount == micQuizModeWork->playerCount * 3 + 1) {
        fn_1_129D4();
    }
    if (micQuizModeWork->questionCount == micQuizModeWork->playerCount * 5) {
        HuAudFXPlay(MSM_SE_MICQUIZ_ROUND_SET_COMPLETE);
        fn_1_123A0();
        HuPrcSleep(120);
    }
    completionStep = micQuizModeWork->playerCount * 5 + 1;
    if (micQuizModeWork->questionCount == completionStep) {
        micQuizModeWork->pauseAllowed = 0;
        result = fn_1_4E70();
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        HuAudSStreamFadeOut(micQuizStreamNo, 600);
        fn_1_A790(0, 20);
        omOvlHisChg(0, DLL_micquizdll, 0, history->stat);
        if (result != 0) {
            HuMCSessionClose();
            HuMCClose();
            fn_1_A308();
            fn_8003A5E4(1, 1);
        } else {
            if (HuMCMicGet() == 1) {
                HuMCSessionClose();
                HuMCClose();
            }
            fn_1_A308();
            fn_8003A5E4(0, 1);
            OSReport("          HEAP_HEAP Malloc Size %x\n",
                     HuMemUsedMallocSizeGet(HEAP_HEAP));
        }
        HuPrcEnd();
        for (;;) {
            HuPrcVSleep();
        }
    }
    if (micQuizModeWork->questionCount != micQuizModeWork->playerCount * 5) {
        if (micQuizModeWork->questionCount == micQuizModeWork->playerCount * 3 + 1 &&
            micQuizModeWork->presenterLayout != 0) {
            fn_1_119FC();
        } else {
            fn_1_1170C();
        }
    }
    {
        fn_1_A838(lbl_1_data_534, 90, 4);
    }
    fn_1_8520();
    if (HuMCMicGet() == 1) {
        MicQuizMicHudShow(micQuizModeWork->currentPlayer);
    }
    while (fn_1_C458() == -1) {
    }
    fn_1_8234();
    for (;;) {
        HuPrcVSleep();
    }
    }
}

/* Close the microphone session and current scene, then load the selected quiz overlay. */
void fn_1_8234(void) {
    OMOVLHIS *history;
    s32 playerIndex;

    history = omOvlHisGet(0);
    micQuizModeWork->pauseAllowed = 0;
    fn_1_AAFC();
    fn_1_FE40();
    HuAudSStreamFadeOut(micQuizStreamNo, 600);
    fn_1_A570();
    if (HuMCMicGet() == 1) {
        HuMCClose();
    }
    omOvlHisChg(0, DLL_micquizdll, 1, history->stat);
    GameMesPauseEnable(0);
    fn_8006A1C4(65547);
    fn_1_A308();
    micQuizModeWork->compactPauseDisplay = 0;
    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        micQuizModeWork->answerResult[playerIndex] = -1;
        micQuizModeWork->answerPlayer[playerIndex] = -1;
    }
    if (micQuizModeWork->questionColumn == 2) {
        if (frandmod(2) == 0U) {
            OSReport("&&&&&&&&&&&&&&&&&&&&&&&&&omOvlCall( DLL_micquizmyokodll )\n");
            omOvlCallEx(DLL_micquizmyokodll, 1, 0, 0);
            return;
        }
        OSReport("&&&&&&&&&&&&&&&&&&&&&&&&&omOvlCall( DLL_micquizishidll )\n");
        omOvlCallEx(DLL_micquizishidll, 1, 0, 0);
        return;
    }
    OSReport("&&&&&&&&&&&&&&&&&&&&&&&&&omOvlCall( QuizDll[StWork->QKind] )\n");
    omOvlCallEx((OMOVL)lbl_1_data_894[micQuizModeWork->questionColumn], 1, 0, 0);
}

/* Particle emitter settings for the two directions of the score effect. */
HU3D_PARMAN_PARAM lbl_1_data_AEC = {
    60, 0, 1.6f, 50.0f, 7.0f, 0.0f, -0.05f, 0.0f,
    2.0f, 1.0f, 5.0f, 1.035f, 2, 255, 128, 128,
    0, 255, 64, 32, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 255, 255, 255,
    255, 255, 255, 64, 255, 0, 0, 0,
    0, 0, 0, 0, 0
};
HU3D_PARMAN_PARAM lbl_1_data_B3C = {
    60, 0, 1.6f, 50.0f, 7.0f, 0.0f, 0.05f, 0.0f,
    2.0f, 1.0f, 30.0f, 0.98f, 2, 255, 255, 255,
    255, 255, 255, 64, 255, 0, 0, 0,
    0, 0, 0, 0, 0, 255, 128, 128,
    0, 255, 64, 32, 0, 0, 0, 0,
    0, 0, 0, 0, 0
};
/* Event chances indexed by player count, rank, and turn for each layout. */
s32 lbl_1_data_B8C[3][4][5] = {
    10, 15, 20, 30, 40, 10, 25, 40, 50, 60, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 10, 10, 10, 20, 30, 10, 20, 30, 40, 50, 10, 25,
    40, 50, 60, 0, 0, 0, 0, 0, 10, 10, 10, 20, 30, 10, 15, 20,
    30, 40, 10, 20, 30, 40, 50, 10, 25, 40, 50, 60
};
s32 lbl_1_data_C7C[3][4][5] = {
    0, 0, 0, 50, 50, 0, 0, 0, 50, 50, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 50, 50, 0, 0, 0, 50, 50, 0, 0,
    0, 50, 50, 0, 0, 0, 0, 0, 0, 0, 0, 50, 50, 0, 0, 0,
    50, 50, 0, 0, 0, 50, 50, 0, 0, 0, 50, 50
};
/* Cumulative percentage thresholds used to choose an event category. */
s32 lbl_1_data_D6C[4][4] = { 29, 39, 70, 99, 24, 49, 74, 99, 29, 59, 89, 99, 19, 59, 89, 99 };
s32 lbl_1_data_DAC[4][4] = { 29, 59, 79, 99, 24, 49, 74, 99, 19, 39, 79, 99, 14, 29, 79, 99 };

/* Hide the current presenter layout and its companion model during a presentation change. */
void fn_1_83F0(void) {
    s32 presenterIndex;

    if (micQuizModeWork->presenterLayout != 0) {
        fn_800269E8(lbl_1_bss_294, (1 << 0));
    } else {
        presenterIndex = 0;
        while (presenterIndex < 2) {
            fn_800269E8(lbl_1_bss_2A6[presenterIndex], (1 << 0));
            presenterIndex += 1;
        }
    }
    fn_800269E8(lbl_1_bss_222, (1 << 0));
}

/* Show the current presenter layout and its companion model for the results scene. */
void fn_1_8488(void) {
    s32 presenterIndex;

    if (micQuizModeWork->presenterLayout != 0) {
        fn_80026A44(lbl_1_bss_294, (1 << 0));
    } else {
        presenterIndex = 0;
        while (presenterIndex < 2) {
            fn_80026A44(lbl_1_bss_2A6[presenterIndex], (1 << 0));
            presenterIndex += 1;
        }
    }
    fn_80026A44(lbl_1_bss_222, (1 << 0));
}

/* Hide each active player model before leaving the selection scene. */
void fn_1_8520(void) {
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 4) {
        if (micQuizModeWork->playerActive[playerIndex] != 0) {
            fn_800269E8(lbl_1_bss_2FC[playerIndex], (1 << 0));
        }
        playerIndex += 1;
    }
}

/* Present the selected quiz outcome with the appropriate camera, message, and player reactions. */
s32 fn_1_8590(void) {
    HUPROCESS *process;
    s32 result;

    process = 0;
    result = 0;
    if (micQuizModeWork->answerResult[0] == 1
        && MicQuizQuestionEventGet() == 3) {
        fn_1_DF9C(0);
        HuPrcSleep(30);
        fn_1_A838(lbl_1_data_550[micQuizModeWork->answerPlayer[0]], 60, 4);
        result = 0;
        if (micQuizModeWork->presenterLayout != 0) {
            process = fn_8000D818(fn_1_9860, 100, 6144, 0);
            process->property = (void *)micQuizModeWork->answerPlayer[0];
            fn_1_1153C();
            HuPrcSleep(185);
            fn_1_A678(micQuizModeWork->answerPlayer[0], 0, 16.0f, ((1 << 30) | (1 << 0)));
        } else {
            HuAudFXPlay(
                lbl_1_data_3E0[micQuizModeWork->playerCharacter[micQuizModeWork->answerPlayer[0]]]);
            fn_1_A678(micQuizModeWork->answerPlayer[0], 8, 8.0f, 0);
            HuPrcSleep(8);
            fn_1_A748(micQuizModeWork->answerPlayer[0]);
            fn_1_1065C(micQuizModeWork->answerPlayer[0]);
            HuPrcSleep(120);
            fn_1_A678(micQuizModeWork->answerPlayer[0], 0, 16.0f, ((1 << 30) | (1 << 0)));
        }
    } else if (micQuizModeWork->answerResult[0] == 1 && micQuizModeWork->answerResult[1] == 0) {
        fn_1_DF9C(1);
        HuPrcSleep(30);
        if (micQuizModeWork->presenterLayout != 0) {
            if (MicQuizQuestionEventGet() != 6) {
                fn_1_A838(lbl_1_data_5C0, 60, 4);
                result = 1;
            } else {
                fn_1_A838(lbl_1_data_550[micQuizModeWork->answerPlayer[1]], 60, 4);
                result = 0;
            }
        } else {
            fn_1_A838(lbl_1_data_550[micQuizModeWork->answerPlayer[1]], 60, 4);
            result = 0;
        }
        fn_1_3A8C(micQuizModeWork->answerPlayer[1], 2);
        fn_1_9284(1);
        if (micQuizModeWork->presenterLayout != 0 &&
            MicQuizQuestionEventGet() != 6) {
            process = fn_8000D818(fn_1_9860, 100, 6144, 0);
            process->property = (void *)micQuizModeWork->answerPlayer[0];
            fn_1_112B0();
            HuPrcSleep(185);
            fn_1_A678(micQuizModeWork->answerPlayer[0], 0, 16.0f, ((1 << 30) | (1 << 0)));
        }
    } else if (micQuizModeWork->answerResult[0] == 0) {
        fn_1_DF9C(1);
        HuPrcSleep(30);
        fn_1_A838(lbl_1_data_550[micQuizModeWork->answerPlayer[0]], 60, 4);
        result = 0;
        fn_1_3A8C(micQuizModeWork->answerPlayer[0], 2);
        fn_1_9284(0);
    } else if (micQuizModeWork->answerResult[0] == 2) {
        fn_1_DF9C(0);
        HuPrcSleep(30);
        fn_1_A838(lbl_1_data_550[micQuizModeWork->answerPlayer[0]], 60, 4);
        result = 0;
        if (micQuizModeWork->presenterLayout != 0) {
            process = fn_8000D818(fn_1_9860, 100, 6144, 0);
            process->property = (void *)micQuizModeWork->answerPlayer[0];
            fn_1_11424();
            HuPrcSleep(185);
            fn_1_A678(micQuizModeWork->answerPlayer[0], 0, 16.0f, ((1 << 30) | (1 << 0)));
        } else {
            HuAudFXPlay(
                lbl_1_data_3E0[micQuizModeWork->playerCharacter[micQuizModeWork->answerPlayer[0]]]);
            fn_1_A678(micQuizModeWork->answerPlayer[0], 8, 8.0f, 0);
            HuPrcSleep(8);
            fn_1_A748(micQuizModeWork->answerPlayer[0]);
            fn_1_1042C();
            HuPrcSleep(120);
            fn_1_A678(micQuizModeWork->answerPlayer[0], 0, 16.0f, ((1 << 30) | (1 << 0)));
        }
    } else if (micQuizModeWork->answerResult[0] == 1 && micQuizModeWork->answerResult[1] == 1) {
        fn_1_DF9C(0);
        HuPrcSleep(30);
        fn_1_A838(lbl_1_data_5C0, 60, 4);
        result = 1;
        if (micQuizModeWork->presenterLayout != 0) {
            process = fn_8000D818(fn_1_9860, 100, 6144, 0);
            process->property = (void *)micQuizModeWork->answerPlayer[0];
            process = fn_8000D818(fn_1_9860, 100, 6144, 0);
            process->property = (void *)micQuizModeWork->answerPlayer[1];
            fn_1_1153C();
            HuPrcSleep(185);
            fn_1_A678(micQuizModeWork->answerPlayer[0], 0, 16.0f, ((1 << 30) | (1 << 0)));
            fn_1_A678(micQuizModeWork->answerPlayer[1], 0, 16.0f, ((1 << 30) | (1 << 0)));
        } else {
            HuAudFXPlay(
                lbl_1_data_3E0[micQuizModeWork->playerCharacter[micQuizModeWork->answerPlayer[0]]]);
            fn_1_A678(micQuizModeWork->answerPlayer[0], 8, 8.0f, 0);
            HuAudFXPlay(
                lbl_1_data_3E0[micQuizModeWork->playerCharacter[micQuizModeWork->answerPlayer[1]]]);
            fn_1_A678(micQuizModeWork->answerPlayer[1], 8, 8.0f, 0);
            HuPrcSleep(8);
            fn_1_A748(micQuizModeWork->answerPlayer[0]);
            fn_1_1065C(micQuizModeWork->answerPlayer[0]);
            fn_1_10934(micQuizModeWork->answerPlayer[1]);
            fn_1_A678(micQuizModeWork->answerPlayer[0], 0, 16.0f, ((1 << 30) | (1 << 0)));
            fn_1_A678(micQuizModeWork->answerPlayer[1], 0, 16.0f, ((1 << 30) | (1 << 0)));
        }
    } else if (micQuizModeWork->answerResult[0] == 1 && micQuizModeWork->answerResult[1] == 2) {
        fn_1_DF9C(0);
        HuPrcSleep(30);
        fn_1_A838(lbl_1_data_550[micQuizModeWork->answerPlayer[0]], 60, 4);
        result = 0;
        if (micQuizModeWork->presenterLayout != 0) {
            process = fn_8000D818(fn_1_9860, 100, 6144, 0);
            process->property = (void *)micQuizModeWork->answerPlayer[0];
            fn_1_11424();
            HuPrcSleep(185);
            fn_1_A678(micQuizModeWork->answerPlayer[0], 0, 16.0f, ((1 << 30) | (1 << 0)));
        } else {
            HuAudFXPlay(
                lbl_1_data_3E0[micQuizModeWork->playerCharacter[micQuizModeWork->answerPlayer[0]]]);
            fn_1_A678(micQuizModeWork->answerPlayer[0], 8, 8.0f, 0);
            HuPrcSleep(8);
            fn_1_10E28();
            HuPrcSleep(120);
            fn_1_A678(micQuizModeWork->answerPlayer[0], 0, 16.0f, ((1 << 30) | (1 << 0)));
        }
    }
    return result;
}

/* Apply the current selection outcome to a player, including score and model feedback. */
void fn_1_9284(s32 objectOffset) {
    s32 modelIndex;

    if (MicQuizQuestionEventGet() == 0) {
        fn_1_1264C(micQuizModeWork->roundPoints, micQuizModeWork->roundPoints * 2);
        fn_1_9934(0, (&micQuizModeWork->answerPlayer[0])[objectOffset]);
        HuAudFXPlay(lbl_1_data_3B4[micQuizModeWork->playerCharacter[
            (&micQuizModeWork->answerPlayer[0])[objectOffset]]]);
        fn_8005DEB0(
            micQuizModeWork->playerCharacter[(&micQuizModeWork->answerPlayer[0])[objectOffset]],
            (s16)lbl_1_data_420[7], 0);
        fn_1_A678((&micQuizModeWork->answerPlayer[0])[objectOffset], 7,
                  8.0f, 0);
        fn_1_A748((&micQuizModeWork->answerPlayer[0])[objectOffset]);
        fn_1_A678((&micQuizModeWork->answerPlayer[0])[objectOffset], 0,
                  8.0f, ((1 << 30) | (1 << 0)));
        fn_1_A088(micQuizModeWork->answeringPlayer, micQuizModeWork->roundPoints * 2);
    } else {
        if (MicQuizQuestionEventGet() == 1) {
        fn_1_12810(micQuizModeWork->roundPoints, micQuizModeWork->roundPoints * 3);
        fn_1_9934(1, (&micQuizModeWork->answerPlayer[0])[objectOffset]);
        HuAudFXPlay(lbl_1_data_3B4[micQuizModeWork->playerCharacter[
            (&micQuizModeWork->answerPlayer[0])[objectOffset]]]);
        fn_8005DEB0(
            micQuizModeWork->playerCharacter[(&micQuizModeWork->answerPlayer[0])[objectOffset]],
            (s16)lbl_1_data_420[7], 0);
        fn_1_A678((&micQuizModeWork->answerPlayer[0])[objectOffset], 7,
                  8.0f, 0);
        fn_1_A748((&micQuizModeWork->answerPlayer[0])[objectOffset]);
        fn_1_A678((&micQuizModeWork->answerPlayer[0])[objectOffset], 0,
                  8.0f, ((1 << 30) | (1 << 0)));
            fn_1_A088(micQuizModeWork->answeringPlayer, micQuizModeWork->roundPoints * 3);
        } else {
            if (MicQuizQuestionEventGet() == 6) {
                fn_1_9934(6, (&micQuizModeWork->answerPlayer[0])[objectOffset]);
                HuPrcSleep(60);
            } else {
                fn_1_100BC();
                HuAudFXPlay(lbl_1_data_3B4[micQuizModeWork->playerCharacter[
                    (&micQuizModeWork->answerPlayer[0])[objectOffset]]]);
                fn_8005DEB0(
                    micQuizModeWork
                        ->playerCharacter[(&micQuizModeWork->answerPlayer[0])[objectOffset]],
                    (s16) lbl_1_data_420[7], 0);
                fn_1_A678((&micQuizModeWork->answerPlayer[0])[objectOffset], 7,
                          8.0f, 0);
                fn_1_A748((&micQuizModeWork->answerPlayer[0])[objectOffset]);
                fn_1_A678((&micQuizModeWork->answerPlayer[0])[objectOffset], 0,
                          8.0f, ((1 << 30) | (1 << 0)));
                fn_1_A088(micQuizModeWork->answeringPlayer, micQuizModeWork->roundPoints);
            }
        }
    }
    if (MicQuizQuestionEventGet() != 6) {
        modelIndex = (&micQuizModeWork->answerPlayer[0])[objectOffset];
        fn_800262D0(lbl_1_bss_20C[modelIndex]);
        lbl_1_bss_20C[modelIndex] = fn_80025A04(
            fn_80008DD0(DATANUM(0xa20000, 87), 0x10000000, 2));
        fn_800269E8(lbl_1_bss_20C[modelIndex], ((1 << 30) | (1 << 0)));
        fn_80026B64(lbl_1_bss_20C[modelIndex], 4);
        fn_80026760(lbl_1_bss_20C[modelIndex], lbl_1_data_83C[modelIndex][0],
                        lbl_1_data_83C[modelIndex][1], lbl_1_data_83C[modelIndex][2]);
    }
}

/* Play one player reaction in a child process, then restore that player's looping motion. */
void fn_1_9860(void) {
    HUPROCESS *process;
    s32 playerNo;

    process = fn_8000DD9C();
    playerNo = (s32)process->property;
    fn_80026A44(lbl_1_bss_1F0[playerNo], (1 << 0));
    HuAudFXPlay(MSM_SE_MICQUIZ_PLAYER_REACTION);
    fn_1_A678(playerNo, 3, 16.0f, 0);
    HuPrcSleep(16);
    do {
        HuPrcVSleep();
    } while (fn_8002D75C(lbl_1_bss_2FC[playerNo]) == 0);
    fn_800269E8(lbl_1_bss_1F0[playerNo], (1 << 0));
    fn_1_A678(playerNo, 6, 16.0f, ((1 << 30) | (1 << 0)));
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Display a colored event panel near a player and trigger its particle effect as it shrinks
 * away. */
void fn_1_9934(s32 mode, s32 playerIndex) {
    Vec position;
    Vec spawnPosition;
    f32 alpha;
    f32 scale;
    f32 angle;
    f32 sineOffset;
    f32 cosineOffset;
    f32 rotation;
    s16 modelId;
    s16 colorAnimation;
    s16 eventAnimation;

    modelId = fn_80025D4C(lbl_1_bss_198[0].model[0]);
    colorAnimation = fn_80030B2C(lbl_1_bss_68, modelId, "vq_color");
    Hu3DAnimBankSet(colorAnimation, playerIndex);
    eventAnimation = fn_80030B2C(lbl_1_bss_64, modelId, "vq_ivent");
    Hu3DAnimBankSet(eventAnimation, mode + 1);

    scale = 4.0f;
    alpha = 0.0f;
    rotation = 0.0;
    angle = lbl_1_data_550[playerIndex].rot.y;
    sineOffset = (f32)(200.0 * sin((M_PI * angle) / 180.0));
    cosineOffset = (f32)(200.0 * cos((M_PI * angle) / 180.0));
    position.x = sineOffset + lbl_1_data_550[playerIndex].center.x;
    position.y = 50.0f + lbl_1_data_550[playerIndex].center.y;
    position.z = cosineOffset + lbl_1_data_550[playerIndex].center.z;
    spawnPosition = position;

    fn_80026760(modelId, position.x, position.y, position.z);
    fn_80026808(modelId, 0.0f, 180.0f + angle, 0.0f);
    fn_80026A44(modelId, (1 << 0));
    fn_80026B64(modelId, 4);
    fn_80026B8C(modelId, 2);
    fn_800268B0(modelId, 4.0f, 4.0f, 1.0f);
    fn_80026CBC(modelId, 0.0f);

    if (mode == 0 || mode == 1) {
        HuAudFXPlay(MSM_SE_MICQUIZ_EVENT_PANEL_APPEAR);
    } else if (mode == 6) {
        HuAudFXPlay(MSM_SE_MICQUIZ_EVENT_PANEL_APPEAR);
    }

    for (;;) {
        HuPrcVSleep();
        scale = (f32)(0.5 + 3.5 * (1.0 - sin((M_PI * rotation) / 180.0)));
        rotation += 2.0f;
        fn_800268B0(modelId, scale, scale, 1.0f);
        if (rotation >= 90.0f) {
            fn_800268B0(modelId, 0.5f, 0.5f, 0.5f);
            break;
        }
        fn_80026CBC(modelId, alpha);
        alpha += 0.025f;
        if (alpha >= 1.0f) {
            alpha = 1.0f;
        }
    }

    fn_1_1A2D0(4, 0);
    if (mode == 0 || mode == 1) {
        fn_1_1A724(spawnPosition.x, spawnPosition.y, spawnPosition.z, 0, angle);
    } else if (mode == 6) {
        fn_1_1A724(spawnPosition.x, spawnPosition.y, spawnPosition.z, 1, angle);
    }
    fn_80031010(colorAnimation);
    fn_80031010(eventAnimation);
    fn_800262D0(modelId);
    HuPrcVSleep(120);
}

/* Start an asynchronous score increase, packing the player and point count into process data. */
void fn_1_9DB8(s32 playerNo, s32 points) {
    HUPROCESS *process = NULL;

    process = fn_8000DAC0(fn_1_9E28, 100, 6144, 0, lbl_1_bss_288);
    process->property = (void *)((playerNo << 16) | (u16)points);
}

/* Count a player's score upward one point per frame, stopping at the requested total or 999. */
void fn_1_9E28(void) {
    HUPROCESS *process;
    u32 property;
    u8 playerNo;
    u16 requested;
    s32 initialValue;
    s32 sleepFrames;

    process = fn_8000DD9C();
    property = (u32)process->property;
    playerNo = (u8)(property >> 16);
    requested = (u16)property;
    sleepFrames = 1;
    initialValue = micQuizModeWork->playerScore[playerNo];
    for (;;) {
        HuPrcSleep(sleepFrames);
        if (micQuizModeWork->playerScore[playerNo] == initialValue + requested) {
            break;
        }
        micQuizModeWork->playerScore[playerNo]++;
        if (micQuizModeWork->playerScore[playerNo] == 999) {
            break;
        }
        HuAudFXPlay(MSM_SE_MICQUIZ_SCORE_TICK);
    }
    HuAudFXPlay(lbl_1_data_39E[micQuizModeWork->playerCharacter[playerNo]]);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Start an asynchronous score decrease, packing the player and point count into process data. */
void fn_1_9F38(s32 playerNo, s32 points) {
    HUPROCESS *process = NULL;

    process = fn_8000DAC0(fn_1_9FA8, 100, 6144, 0, lbl_1_bss_288);
    process->property = (void *)((playerNo << 16) | (u16)points);
}

/* Count a player's score downward one point per frame, stopping at the requested total or zero. */
void fn_1_9FA8(void) {
    HUPROCESS *process;
    u32 property;
    u8 playerNo;
    u16 requested;
    s32 initialValue;
    s32 sleepFrames;

    process = fn_8000DD9C();
    property = (u32)process->property;
    playerNo = (u8)(property >> 16);
    requested = (u16)property;
    sleepFrames = 1;
    initialValue = micQuizModeWork->playerScore[playerNo];
    for (;;) {
        HuPrcSleep(sleepFrames);
        if (micQuizModeWork->playerScore[playerNo] == initialValue - requested) {
            break;
        }
        micQuizModeWork->playerScore[playerNo]--;
        if (micQuizModeWork->playerScore[playerNo] == 0) {
            break;
        }
        HuAudFXPlay(MSM_SE_MICQUIZ_SCORE_TICK);
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Animate a score increase synchronously, using shorter delays for larger awards. */
void fn_1_A088(s32 playerNo, s32 increment) {
    s32 sleepFrames;
    s32 oldValue;

    sleepFrames = 1;
    oldValue = micQuizModeWork->playerScore[playerNo];
    if (increment <= 50) {
        sleepFrames = 5;
    } else if (increment <= 100) {
        sleepFrames = 3;
    } else if (increment <= 150) {
        sleepFrames = 1;
    }
    for (;;) {
        HuPrcSleep(sleepFrames);
        if (micQuizModeWork->playerScore[playerNo] == oldValue + increment) {
            return;
        }
        micQuizModeWork->playerScore[playerNo]++;
        if (micQuizModeWork->playerScore[playerNo] >= 999) {
            return;
        }
        HuAudFXPlay(MSM_SE_MICQUIZ_SCORE_TICK);
    }
}

/* Animate a score decrease synchronously, stopping when the requested total or zero is reached. */
void fn_1_A18C(s32 playerNo, s32 decrement) {
    s32 sleepFrames;
    s32 oldValue;

    sleepFrames = 1;
    oldValue = micQuizModeWork->playerScore[playerNo];
    if (decrement >= -50) {
        sleepFrames = 5;
    } else if (decrement >= -100) {
        sleepFrames = 3;
    } else if (decrement >= -150) {
        sleepFrames = 1;
    }
    for (;;) {
        HuPrcSleep(sleepFrames);
        if (micQuizModeWork->playerScore[playerNo] == oldValue - decrement) {
            return;
        }
        micQuizModeWork->playerScore[playerNo]--;
        if (micQuizModeWork->playerScore[playerNo] == 0) {
            return;
        }
        HuAudFXPlay(MSM_SE_MICQUIZ_SCORE_TICK);
    }
}

/* After the exit wipe ends, close microphone work, release the scene, and return to the caller. */
void fn_1_A290(void) {
    OMOVLHIS *history;

    history = omOvlHisGet(0);
    if (WipeCheck() == 0) {
        omOvlHisChg(0, DLL_micquizdll, 0, history->stat);
        HuMCSessionClose();
        if (HuMCMicGet() == 1) {
            HuMCClose();
        }
        fn_1_A308();
        fn_8003A5E4(1, 1);
    }
}

/* Release scene models and the two camera capture animations when leaving the overlay. */
void fn_1_A308(void) {
    fn_80025630();
    HuSprAnimKill(lbl_1_bss_30);
    HuSprAnimKill(lbl_1_bss_34);
}

/* Watch for a system exit request each object update and begin the outgoing wipe. */
void fn_1_A348(OMOBJ *obj) {
    if (omSysExitReq != 0) {
        HuAudSStreamFadeOut(micQuizStreamNo, 600);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        obj->objFunc = (OMOBJ_FUNC)fn_1_A290;
    }
}

/* Change a two-presenter model's motion only when the requested motion differs. */
void fn_1_A3B4(s32 presenterNo, s32 motionNo, f32 blendFrames, u32 attr) {
    HU3D_MOTIONID currentMotion;

    currentMotion = fn_8002D3C4(lbl_1_bss_2A6[presenterNo]);
    if (currentMotion != lbl_1_bss_296[presenterNo][motionNo]) {
        fn_8002CCC0(lbl_1_bss_2A6[presenterNo], lbl_1_bss_296[presenterNo][motionNo],
                           0.0f, blendFrames, attr);
    }
}

/* Yield until the selected two-presenter model finishes its current motion. */
void fn_1_A484(s32 presenterNo) {
    for (;;) {
        HuPrcVSleep();
        if (fn_8002D75C(lbl_1_bss_2A6[presenterNo]) != 0) {
            break;
        }
    }
}

/* Change the single-presenter model's motion only when the requested motion differs. */
void fn_1_A4CC(s32 motionNo, f32 blendFrames, u32 attr) {
    HU3D_MOTIONID currentMotion;

    currentMotion = fn_8002D3C4(lbl_1_bss_294);
    if (currentMotion != lbl_1_bss_28C[motionNo]) {
        fn_8002CCC0(lbl_1_bss_294, lbl_1_bss_28C[motionNo],
                           0.0f, blendFrames, attr);
    }
}

/* Wait for any existing wipe, then hide the scene with an outgoing wipe. */
void fn_1_A570(void) {
    f32 bounds[3];

    bounds[0] = -130.0f;
    bounds[1] = 920.0f;
    bounds[2] = -600.0f;
    bounds[0] += 108.0f * micQuizModeWork->questionLevel;
    bounds[1] -= 108.0f * micQuizModeWork->questionColumn;
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    fn_1_A7E4(0, 20);
}

/* Change an active player character's motion only when the requested motion differs. */
void fn_1_A678(s32 playerNo, s32 motionNo, f32 blendFrames, u32 attr) {
    HU3D_MOTIONID currentMotion;

    currentMotion = fn_8002D3C4(lbl_1_bss_2FC[playerNo]);
    if (currentMotion != lbl_1_bss_2AC[playerNo][motionNo]) {
        fn_8005C2C8(lbl_1_bss_2F4[playerNo], lbl_1_bss_2AC[playerNo][motionNo],
                           0.0f, blendFrames, attr);
    }
}

/* Yield until the selected player character finishes its current motion. */
void fn_1_A748(s32 playerNo) {
    for (;;) {
        HuPrcVSleep();
        if (fn_8005C240(lbl_1_bss_2F4[playerNo]) != 0) {
            break;
        }
    }
}

/* Start an incoming wipe and yield until it finishes during a scene transition. */
void fn_1_A790(s32 wipeType, s32 duration) {
    WipeCreate(WIPE_MODE_IN, (s16)wipeType, (s16)duration);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
}

/* Start an outgoing wipe and yield until it finishes during a scene transition. */
void fn_1_A7E4(s32 wipeType, s32 duration) {
    WipeCreate(WIPE_MODE_OUT, (s16)wipeType, (s16)duration);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
}

/* Move the selected cameras to a view and yield for the requested transition duration. */
void fn_1_A838(OM_CAMERA_VIEW cameraView, s32 duration, u32 cameraMask) {
    s32 frame;

    fn_8003E8E4(cameraMask, &cameraView, duration);
    for (frame = 0; frame < duration; frame++) {
        HuPrcVSleep();
    }
}

/* Assign finishing ranks from the four scores, giving tied scores the same rank. */
void fn_1_A898(void) {
    s32 orderedValues[4];
    s32 playerNo;
    s32 rankIndex;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        orderedValues[playerNo] = micQuizModeWork->playerScore[playerNo];
    }
    fn_1_A97C(orderedValues, 4);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        for (rankIndex = 0; rankIndex < 4; rankIndex++) {
            if (micQuizModeWork->playerScore[playerNo] == orderedValues[rankIndex]) {
                micQuizModeWork->playerRank[playerNo] = rankIndex + 1;
                break;
            }
        }
    }
}

/* Sort scores in descending order before ranking players or choosing winners. */
void fn_1_A97C(s32 *values, s32 count) {
    s32 outer;
    s32 inner;
    s32 largestIndex;
    s32 largestValue;
    s32 value;

    for (outer = 0; outer < count - 1; outer++) {
        largestValue = values[outer];
        largestIndex = outer;
        for (inner = outer + 1; inner < count; inner++) {
            if (values[inner] > largestValue) {
                largestValue = values[inner];
                largestIndex = inner;
            }
        }
        value = values[outer];
        values[outer] = values[largestIndex];
        values[largestIndex] = value;
    }
}

/* Sort values in descending order while keeping their two associated arrays together. */
void fn_1_AA1C(s32 *values, s32 *secondaryValues, s32 *tertiaryValues, s32 count) {
    s32 outer;
    s32 inner;
    s32 largestIndex;
    s32 largestValue;
    s32 value;
    s32 secondaryValue;
    s32 tertiaryValue;

    for (outer = 0; outer < count - 1; outer++) {
        largestValue = values[outer];
        largestIndex = outer;
        for (inner = outer + 1; inner < count; inner++) {
            if (values[inner] > largestValue) {
                largestValue = values[inner];
                largestIndex = inner;
            }
        }
        value = values[outer];
        secondaryValue = secondaryValues[outer];
        tertiaryValue = tertiaryValues[outer];
        values[outer] = values[largestIndex];
        secondaryValues[outer] = secondaryValues[largestIndex];
        tertiaryValues[outer] = tertiaryValues[largestIndex];
        values[largestIndex] = value;
        secondaryValues[largestIndex] = secondaryValue;
        tertiaryValues[largestIndex] = tertiaryValue;
    }
}

/* Update the board selection level after the current outcome, clamping upward moves at four. */
void fn_1_AAFC(void) {
    switch (micQuizModeWork->questionEvent) {
    case 2:
        micQuizModeWork->savedQuestionLevel = micQuizModeWork->questionLevel;
        micQuizModeWork->questionLevel--;
        if (micQuizModeWork->questionLevel == 0) {
            micQuizModeWork->questionLevel = 0;
        }
        break;
    case 5:
        micQuizModeWork->savedQuestionLevel = micQuizModeWork->questionLevel;
        micQuizModeWork->questionLevel++;
        if (micQuizModeWork->questionLevel >= 4) {
            micQuizModeWork->questionLevel = 4;
        }
        break;
    }
}

/* Resolve the current quiz response with messages, score changes, player reactions, and effects. */
void fn_1_ABF4(void) {
    f32 offset;
    s32 playerNo;
    s32 pointDonor;
    s32 index;
    s32 outIndex;
    s32 score;
    s32 otherPlayer;
    s32 randomIndex;
    s16 result;
    f32 fallAngle;
    Vec positionA;
    Vec positionB;
    Vec effectPositionA;
    Vec effectPositionB;
    s32 counter;
    HUPROCESS * process = 0;
    playerNo = micQuizModeWork->answeringPlayer;
    if (MicQuizQuestionEventGet() == 6 &&
        (micQuizModeWork->answerResult[0] != 1 || micQuizModeWork->answerResult[1] != 1)) {
        if (micQuizModeWork->answerResult[0] == 2 || micQuizModeWork->answerResult[1] == 2)
        {
            return;
        }
        fn_1_18B50();
        HuPrcSleep(120);
        if (HuMCMicGet() == 1)
        {
            MicQuizMicHudShow(playerNo);
            for (;;)
            {
                while (fn_1_BE70() == -1)
                {
                }
                MicQuizMicHudStateSet(0);
                if (lbl_1_bss_8C == 0)
                {
                    for (index = 0; index < 4; index++)
                    {
                        lbl_1_bss_9C[index] = -1;
                        lbl_1_bss_90[index] = -1;
                    }
                    for (index = 0, outIndex = 0; index < 4; index++)
                    {
                        if (index != playerNo && micQuizModeWork->playerActive[index] != 0)
                        {
                            lbl_1_bss_9C[outIndex] = index;
                            lbl_1_bss_90[outIndex] = micQuizModeWork->playerCharacter[index];
                            ++outIndex;
                        }
                    }
                    if (micQuizModeWork->playerCount == 2)
                    {
                        pointDonor = lbl_1_bss_9C[0];
                    }
                    else
                    {
                        randomIndex = frandmod(micQuizModeWork->playerCount - 1);
                        pointDonor = lbl_1_bss_9C[randomIndex];
                    }
                    MicQuizMicHudHide();
                    fn_1_19294(pointDonor);
                    HuPrcSleep(120);
                    break;
                }
                if ((s32)(lbl_1_bss_8C - 1) == micQuizModeWork->playerCharacter[playerNo])
                {
                    MicQuizMicHudStateSet(0);
                    fn_1_18E98(lbl_1_bss_8C - 1);
                    HuPrcSleep(120);
                    fn_1_18B50();
                    MicQuizMicHudStateSet(1);
                    continue;
                }
                if (! (fn_1_BE04(lbl_1_bss_8C - 1)))
                {
                    MicQuizMicHudStateSet(0);
                    fn_1_18FEC(lbl_1_bss_8C - 1);
                    HuPrcSleep(120);
                    fn_1_18B50();
                    MicQuizMicHudStateSet(1);
                    continue;
                }
                MicQuizMicHudHide();
                fn_1_19140(lbl_1_bss_8C - 1);
                pointDonor = fn_1_BD9C(lbl_1_bss_8C - 1);
                MicQuizMicHudHide();
                break;
            }
        }
        else
        {
            result = fn_1_196BC(micQuizModeWork->answeringPlayer);
            if (result == micQuizModeWork->playerCount - 1)
            {
                if (micQuizModeWork->playerCount == 2) {
                    pointDonor = lbl_1_bss_9C[0];
                } else {
                    randomIndex = frandmod(micQuizModeWork->playerCount - 1);
                    pointDonor = lbl_1_bss_9C[randomIndex];
                }
                fn_1_19294(pointDonor);
                HuPrcSleep(120);
            }
            else
            {
                pointDonor = lbl_1_bss_9C[result];
            }
            fn_1_19C84(pointDonor);
        }
        fn_1_3A8C(playerNo, 1);
        positionA.x = lbl_1_data_668[playerNo].x;
        positionA.y = lbl_1_data_668[playerNo].y + 450.0f;
        positionA.z = lbl_1_data_668[playerNo].z;
        positionB.x = lbl_1_data_668[pointDonor].x;
        positionB.y = lbl_1_data_668[pointDonor].y + 450.0f;
        positionB.z = lbl_1_data_668[pointDonor].z;
        fn_80026760(lbl_1_bss_1EC[0], positionA.x, positionA.y, positionA.z);
        fn_80026760(lbl_1_bss_1EC[1], positionB.x, positionB.y, positionB.z);
        fn_80026808(lbl_1_bss_1EC[0], 0.0f, - 75.0f, 0.0f);
        fn_80026808(lbl_1_bss_1EC[1], 0.0f, - 75.0f, 0.0f);
        fn_80026A44(lbl_1_bss_1EC[0], (1 << 0));
        fn_80026A44(lbl_1_bss_1EC[1], (1 << 0));
        fallAngle = 0.0f;
        HuAudFXPlay(MSM_SE_MICQUIZ_RESULT_MODEL_DROP);
        do
        {
            HuPrcVSleep();
            offset = (f32)(200.0 * sin((M_PI * fallAngle) / 180.0));
            fn_80026760(lbl_1_bss_1EC[0], positionA.x, positionA.y - offset, positionA.z);
            fn_80026760(lbl_1_bss_1EC[1], positionB.x, positionB.y - offset, positionB.z);
            fallAngle += 2.25f;
        }
        while (! (fallAngle >= 90.0f));
        positionA.y -= offset;
        positionB.y -= offset;
        fn_80026760(lbl_1_bss_1EC[0], positionA.x, positionA.y, positionA.z);
        fn_80026760(lbl_1_bss_1EC[1], positionB.x, positionB.y, positionB.z);
        HuPrcSleep(30);
        effectPositionA = positionA;
        effectPositionB = positionB;
        effectPositionA.y += 20.0f;
        effectPositionB.y -= 120.0f;
        lbl_1_bss_1E4[0] = fn_1_C1DC(lbl_1_bss_1E8, 0, &effectPositionA);
        lbl_1_bss_1E4[1] = fn_1_C1DC(lbl_1_bss_1E8, 1, &effectPositionB);
        Hu3DParManAttrSet(lbl_1_bss_1E4[0], (1 << 0));
        Hu3DParManAttrSet(lbl_1_bss_1E4[1], (1 << 0));
        if (micQuizModeWork->playerScore[pointDonor] < micQuizModeWork->roundPoints) {
            score = micQuizModeWork->playerScore[pointDonor];
        } else {
            score = micQuizModeWork->roundPoints;
        }
        HuAudFXPlay(MSM_SE_MICQUIZ_POINT_CHANGE_EFFECT);
        Hu3DParManAttrReset(lbl_1_bss_1E4[1], (1 << 0));
        HuAudFXPlay((s32) lbl_1_data_3E0[micQuizModeWork->playerCharacter[pointDonor]]);
        fn_1_A678(pointDonor, 8, 8.0f, 0);
        fn_1_9F38(pointDonor, score);
        HuPrcSleep(80);
        Hu3DParManAttrSet(lbl_1_bss_1E4[1], (1 << 0));
        HuPrcSleep(40);
        HuAudFXPlay(MSM_SE_MICQUIZ_POINT_CHANGE_EFFECT);
        Hu3DParManAttrReset(lbl_1_bss_1E4[0], (1 << 0));
        HuPrcSleep(20);
        fn_8005DEB0(micQuizModeWork->playerCharacter[playerNo], lbl_1_data_420[5], 0);
        fn_1_A678(playerNo, 5, 8.0f, 0);
        fn_1_9DB8(playerNo, score);
        HuPrcSleep(60);
        Hu3DParManAttrSet(lbl_1_bss_1E4[0], (1 << 0));
        HuPrcSleep(100);
        fallAngle = 0.0f;
        HuAudFXPlay(MSM_SE_MICQUIZ_RESULT_MODEL_DROP);
        do
        {
            HuPrcVSleep();
            offset = (f32)(250.0 * sin((M_PI * fallAngle) / 180.0));
            fn_80026760(lbl_1_bss_1EC[0], positionA.x, positionA.y + offset, positionA.z);
            fn_80026760(lbl_1_bss_1EC[1], positionB.x, positionB.y + offset, positionB.z);
            fallAngle += 2.25f;
        }
        while (! (fallAngle >= 90.0f));
        fn_800269E8(lbl_1_bss_1EC[0], (1 << 0));
        fn_800269E8(lbl_1_bss_1EC[1], (1 << 0));
        fn_1_3A8C(playerNo, 0);
        fn_1_1959C();
        fn_1_A678(playerNo, 0, 8.0f, ((1 << 30) | (1 << 0)));
        fn_1_A678(pointDonor, 0, 8.0f, ((1 << 30) | (1 << 0)));
        if (micQuizActiveWin != - 1)
        {
            HuWinExClose(micQuizActiveWin);
            micQuizActiveWin = - 1;
        }
        if (micQuizModeWork->answerResult[0] == 1)
        {
            process = fn_8000D818(fn_1_9860, 100U, 6144U, 0);
            process->property = (void *)(s32)micQuizModeWork->answerPlayer[0];
            fn_1_112B0();
            HuPrcSleep(185);
            fn_1_A678(micQuizModeWork->answerPlayer[0], 0, 16.0f, ((1 << 30) | (1 << 0)));
        }
    }
}

/* Find the active player using a character, or return the supplied character value if absent. */
s32 fn_1_BD9C(s32 requested) {
    s32 index;

    for (index = 0; index < 4; index++) {
        if (micQuizModeWork->playerActive[index] != 0
            && micQuizModeWork->playerCharacter[index] == requested) {
            return index;
        }
    }
    return requested;
}

/* Test whether the requested character belongs to an active player. */
s32 fn_1_BE04(s32 requested) {
    s32 index;

    for (index = 0; index < 4; index++) {
        if (micQuizModeWork->playerActive[index] != 0
            && micQuizModeWork->playerCharacter[index] == requested) {
            return 1;
        }
    }
    return 0;
}

/* Collect the selected player's microphone response and schedule feedback for accepted or rejected
 * input. */
s32 fn_1_BE70(void) {
    HUPROCESS *process;
    s32 response;
    s32 playerNo;
    HUPROCESS *firstParent;
    HUPROCESS *secondParent;
    HUPROCESS *thirdParent;

    process = 0;
    playerNo = MicQuizCurrentPlayerGet();
    lbl_1_bss_1C = 1;
    MicQuizMicHudStateSet(1);
    process = fn_8000DAC0(fn_1_C9B4, 100, 6144, 0, lbl_1_bss_288);
    HuMCSessionSet((s16)playerNo);
    HuMCSelWinContextSet(lbl_1_bss_AE, 0, micQuizModeWork->currentPlayer);
    HuMCSelWinChoiceGet();
    micQuizModeWork->pauseAllowed = 1;
    HuPrcKill(process);
    MicQuizMicHudStateSet(0);
    micQuizResponse = (MicquizResponse *)HuMCCurResponseGet();
    HuMCSelWinContextKill();
    lbl_1_bss_8C = micQuizResponse->recognizedWord;
    lbl_1_bss_1C = 0;

    firstParent = lbl_1_bss_288;
    if (micQuizTempMesWin != -1) {
        fn_8000DAC0(MicQuizTempMesClose, 100, 6144, 0, firstParent);
    }

    if (micQuizResponse->result == -1 || micQuizResponse->result == -2) {
        MicQuizMicHudStateSet(0);
        fn_1_18C68();
        HuPrcSleep(120);
        MicQuizMicHudStateSet(1);
        return -1;
    }
    if (micQuizResponse->result == -4) {
        MicQuizMicHudStateSet(0);
        fn_1_18D80();
        HuPrcSleep(120);
        MicQuizMicHudStateSet(1);
        return -1;
    }

    if (HuMCUnkResponseCheck() != 0) {
        micQuizModeWork->pauseAllowed = 0;
        MicQuizMicHudStateSet(1);
        process = fn_8000DAC0(fn_1_C9B4, 100, 6144, 0, lbl_1_bss_288);
        lbl_1_bss_1C = 1;
        HuPrcVSleep();
        MicQuizMicHudStateSet(1);
        secondParent = lbl_1_bss_288;
        if (micQuizTempMesWin != -1) {
            fn_8000DAC0(MicQuizTempMesClose, 100, 6144, 0, secondParent);
        }
        response = HuMCNewResponseGet(MESSNUM(MESS_MIC_RETYPING, 106));
        if (response == -1) {
            MicQuizMicHudStateSet(0);
            fn_8000DAC0(MicQuizTempMes, 100, 6144, 0, micQuizParentProc);
            micQuizModeWork->pauseAllowed = 1;
            return -1;
        }
        lbl_1_bss_8C = response;
        lbl_1_bss_1C = 1;
        HuPrcKill(process);
        micQuizModeWork->pauseAllowed = 1;
        MicQuizMicHudStateSet(0);
    }

    thirdParent = lbl_1_bss_288;
    if (micQuizTempMesWin != -1) {
        fn_8000DAC0(MicQuizTempMesClose, 100, 6144, 0, thirdParent);
    }
    return 0;
}

/* Create a directional particle emitter at a supplied position for the score sequence. */
HU3D_PARMANID fn_1_C1DC(ANIMDATA *animation, s32 mode, HuVecF *position) {
    HU3D_PARMANID particleManager;
    HU3D_MODELID modelId;

    if (mode == 0) {
        particleManager = fn_800334F4(animation, 100, &lbl_1_data_AEC);
    } else {
        particleManager = fn_800334F4(animation, 100, &lbl_1_data_B3C);
    }
    Hu3DParManAttrSet(particleManager,
                      ((1 << 2) | (1 << 5) |
                       (1 << 6)));
    if (mode == 0) {
        Hu3DParManRotSet(particleManager, 90.0f, 0.0f, 0.0f);
    } else {
        Hu3DParManRotSet(particleManager, -90.0f, 0.0f, 0.0f);
    }
    modelId = Hu3DParManModelIDGet(particleManager);
    if (mode == 0) {
        fn_8003269C(modelId, 1);
    } else {
        fn_8003269C(modelId, 1);
    }
    fn_80026B8C(modelId, 2);
    fn_80026B64(modelId, 4);
    Hu3DParManPosSet(particleManager, position->x, position->y, position->z);
    return particleManager;
}

/* Transfer points between two players one at a time, stopping if the donor reaches zero. */
void fn_1_C320(s32 playerNo, s32 otherPlayerNo, s32 amount) {
    s32 startingValue;
    s32 sleepFrames;

    sleepFrames = 1;
    startingValue = micQuizModeWork->playerScore[playerNo];
    if (amount <= 50) {
        sleepFrames = 5;
    } else if (amount <= 100) {
        sleepFrames = 3;
    } else if (amount <= 150) {
        sleepFrames = 1;
    }
    while (1) {
        HuPrcSleep(sleepFrames);
        if (micQuizModeWork->playerScore[playerNo] == startingValue + amount) {
            break;
        }
        if (micQuizModeWork->playerScore[otherPlayerNo] != 0) {
            micQuizModeWork->playerScore[otherPlayerNo]--;
            micQuizModeWork->playerScore[playerNo]++;
            if (micQuizModeWork->playerScore[otherPlayerNo] != 0) {
                HuAudFXPlay(MSM_SE_MICQUIZ_SCORE_TICK);
                continue;
            }
        }
        break;
    }
}

/* Choose a question-grid column through microphone input or the current player's controller. */
s32 fn_1_C458(void) {
    HUPROCESS *process;
    s32 inputPlayer;
    s32 choice;
    s32 response;
    HUPROCESS *firstParent;
    HUPROCESS *secondParent;
    HUPROCESS *thirdParent;
    s32 playerNo;

    process = 0;
    playerNo = micQuizModeWork->currentPlayer;
    inputPlayer = playerNo;
    if (HuMCMicGet() == 1) {
        for (;;) {
            lbl_1_bss_1C = 1;
            MicQuizMicHudStateSet(1);
            process = fn_8000DAC0(fn_1_C9B4, 100, 6144, 0, lbl_1_bss_288);
            HuMCSessionSet(-2);
            HuMCSelWinContextSet(lbl_1_bss_B0, 0, micQuizModeWork->currentPlayer);
            HuMCSelWinChoiceGet();
            micQuizModeWork->pauseAllowed = 1;
            HuPrcKill(process);
            MicQuizMicHudStateSet(0);
            micQuizResponse = (MicquizResponse *)HuMCCurResponseGet();
            HuMCSelWinContextKill();
            choice = micQuizResponse->recognizedWord;
            lbl_1_bss_1C = 0;

            firstParent = lbl_1_bss_288;
            if (micQuizTempMesWin != -1) {
                fn_8000DAC0(MicQuizTempMesClose, 100, 6144, 0, firstParent);
            }
            if (micQuizResponse->result == -1) {
                MicQuizMicHudStateSet(1);
                fn_1_11FA8();
                continue;
            }
            if (micQuizResponse->result == -2) {
                MicQuizMicHudStateSet(1);
                fn_1_11FA8();
                continue;
            }
            if (micQuizResponse->result == -4) {
                MicQuizMicHudStateSet(1);
                fn_1_121A4();
                continue;
            }

            if (HuMCUnkResponseCheck() != 0) {
                micQuizModeWork->pauseAllowed = 0;
                MicQuizMicHudStateSet(1);
                process = fn_8000DAC0(fn_1_C9B4, 100, 6144, 0, lbl_1_bss_288);
                lbl_1_bss_1C = 1;
                HuPrcVSleep();
                MicQuizMicHudStateSet(1);
                secondParent = lbl_1_bss_288;
                if (micQuizTempMesWin != -1) {
                    fn_8000DAC0(MicQuizTempMesClose, 100, 6144, 0, secondParent);
                }
                response = HuMCNewResponseGet(MESSNUM(MESS_MIC_RETYPING, 0));
                if (response == -1) {
                    MicQuizMicHudStateSet(0);
                    fn_8000DAC0(MicQuizTempMes, 100, 6144, 0, micQuizParentProc);
                    micQuizModeWork->pauseAllowed = 1;
                    return -1;
                }
                choice = response;
                lbl_1_bss_1C = 0;
                HuPrcKill(process);
                micQuizModeWork->pauseAllowed = 1;
                MicQuizMicHudStateSet(0);
            }
            thirdParent = lbl_1_bss_288;
            if (micQuizTempMesWin != -1) {
                fn_8000DAC0(MicQuizTempMesClose, 100, 6144, 0, thirdParent);
            }
            if (fn_1_CBAC(0, choice) != 0) {
                break;
            }
            MicQuizMicHudStateSet(1);
        }
    } else {

    fn_80026A44(lbl_1_bss_FC, (1 << 0));
    HuPrcSleep(5);
    for (;;) {
        HuPrcVSleep();
        if ((lbl_802BFFE0[inputPlayer] & 8) != 0) {
            HuAudFXPlay(MSM_SE_CMN_CURSOR_MOVE);
            lbl_1_bss_5C--;
            if (lbl_1_bss_5C < 0) {
                lbl_1_bss_5C = 4;
            }
        } else if ((lbl_802BFFE0[inputPlayer] & 4) != 0) {
            HuAudFXPlay(MSM_SE_CMN_CURSOR_MOVE);
            lbl_1_bss_5C++;
            if (lbl_1_bss_5C > 4) {
                lbl_1_bss_5C = 0;
            }
        }
        if ((lbl_802C0008[inputPlayer] & PAD_BUTTON_A) != 0) {
            HuAudFXPlay(MSM_SE_CMN_MENU_ADVANCE);
            fn_800269E8(lbl_1_bss_FC, (1 << 0));
            if (fn_1_CBAC(0, lbl_1_bss_5C) != 0) {
                break;
            }
            fn_80026A44(lbl_1_bss_FC, (1 << 0));
        }
        fn_80026760(lbl_1_bss_FC, -425.0f,
                        40.0f + (-300.0f + (500.0f - (100.0f * (f32)lbl_1_bss_5C))),
                        10.0f);
    }
    }
    return 0;
}

/* Monitor microphone selection input each frame and update its on-screen activity feedback. */
void fn_1_C9B4(void) {
    s32 selectionActive;

    selectionActive = 0;
    for (;;) {
        HuPrcVSleep();
        if (lbl_1_bss_1C == 0) {
            continue;
        }
        if (HuMCSelWinCheck() != 0) {
            if ((s16)HuMCSelModeGet() == 1) {
                if (selectionActive == 0) {
                    micQuizModeWork->pauseAllowed = 0;
                    if (micQuizRingProc == 0) {
                        micQuizRingProc = fn_8000DAC0(MicQuizRingEmit, 100, 8192, 0,
                                                      micQuizParentProc);
                    }
                    MicQuizMicHudStateSet(3);
                    selectionActive = 1;
                    }
            } else {
                selectionActive = 0;
            }
            continue;
        }
        if (HuMCButtonGet() != 0) {
            micQuizModeWork->pauseAllowed = 0;
            if (lbl_1_bss_1C != 0 && micQuizRingProc == 0) {
                micQuizRingProc = fn_8000DAC0(MicQuizRingEmit, 100, 8192, 0, micQuizParentProc);
            }
            MicQuizMicHudStateSet(3);
        } else if (micQuizRingProc != 0) {
            HuPrcKill(micQuizRingProc);
            micQuizRingProc = 0;
        }
        if (HuMCButtonDownGet() != 0) {
            if (selectionActive == 0) {
                micQuizModeWork->pauseAllowed = 0;
                if (micQuizRingProc == 0) {
                    micQuizRingProc = fn_8000DAC0(MicQuizRingEmit, 100, 8192, 0, micQuizParentProc);
                }
                MicQuizMicHudStateSet(3);
            }
        } else {
            selectionActive = 0;
        }
    }
}

/* Validate a chosen grid column, reveal its next available cell, and resolve the selected event.
 * fn_1_D94C ignores the incoming level and finds the first unrevealed row instead. */
s32 fn_1_CBAC(s32 levelIndex, s32 column) {
    s32 row;
    s32 selection;
    f32 scale;
    f32 angle;

    row = fn_1_D94C(levelIndex, column);
    if (row == -1) {
        HuAudFXPlay(MSM_SE_CMN_CHOICE_DENIED);
        fn_1_11BB0();
        HuPrcSleep(120);
        return 0;
    }

    levelIndex = row;
    if (fn_1_E69C(levelIndex, column, 1) != 0U) {
        fn_1_E670(levelIndex, column, 1);
        fn_1_E644(levelIndex, column, 2);
        if (levelIndex + 1 < 5) {
            fn_1_E644(levelIndex + 1, column, 1);
        }
        micQuizModeWork->questionColumn = (u8)column;
        micQuizModeWork->questionLevel = (u8)levelIndex;
        fn_1_FAF4();

        scale = 0.0f;
        angle = 0.0f;
        HuSprGrpScaleSet(micQuizMicHudGroup, 1.0f, 1.0f);
        for (;;) {
            HuPrcVSleep();
            scale = (f32)(1.0
                - sin((M_PI * (f64)angle) / 180.0));
            HuSprGrpScaleSet(micQuizMicHudGroup, scale, scale);
            angle += 3.0f;
            if (angle >= 90.0f) {
                break;
            }
        }
        HuSprGrpAttrSet(micQuizMicHudGroup, HUSPR_ATTR_DISPOFF);
        HuSprGrpScaleSet(micQuizMicHudGroup, 0.0f, 0.0f);
    } else {
        if (fn_1_E69C(levelIndex, column, 2) != 0U) {
            HuAudFXPlay(MSM_SE_CMN_CHOICE_DENIED);
            fn_1_11BB0();
            HuPrcSleep(120);
        } else {
            HuAudFXPlay(MSM_SE_CMN_CHOICE_DENIED);
            fn_1_11DAC();
        }
        return 0;
    }

    selection = fn_1_DA48();
    fn_1_CE28(levelIndex, column, selection, 0);
    return 1;
}

/* Reveal and move a selected grid panel, showing its outcome before the round continues. */
void fn_1_CE28(s32 levelIndex, s32 column, s32 selection, s32 flags)
{
    Point3d currentPosition;
    Point3d startPosition;
    Point3d movementStep;
    Point3d destinationPosition;
    Point3d selectedPosition;
    f32 angle = 0.0f;
    f32 elapsedFrames;
    f32 rotationStep;
    f32 rotationY;
    f32 opacityAngle;
    f32 opacity;
    s32 mode;

    if (selection != 8) {
        fn_1_DD68(levelIndex, column, selection);
    }
    mode = micQuizModeWork->currentPlayer;
    fn_1_DD10(levelIndex, column, mode);
    HuAudFXPlay(MSM_SE_MICQUIZ_PANEL_REVEAL);

    angle = 0.0f;
    rotationY = 0.0f;
    for (;;) {
        HuPrcVSleep();
        rotationY = (f32)(180.0 * sin((M_PI * (f64)angle) / 180.0));
        fn_80026808(lbl_1_bss_198[levelIndex].model[column], 0.0f, rotationY, 0.0f);
        angle += 7.5f;
        if (angle >= 90.0f) {
            break;
        }
    }

    fn_80026808(lbl_1_bss_198[levelIndex].model[column], 0.0f, 180.0f, 0.0f);
    fn_800267CC(lbl_1_bss_198[levelIndex].model[column], &selectedPosition);
    fn_80026B8C(lbl_1_bss_198[levelIndex].model[column], 2);
    fn_80026760(lbl_1_bss_100, selectedPosition.x, selectedPosition.y,
                    10.0f + selectedPosition.z);
    fn_80026B8C(lbl_1_bss_100, 1);
    fn_80026A44(lbl_1_bss_100, (1 << 0));
    fn_80026A44(lbl_1_bss_100, ((1 << 30) | (1 << 1)));

    if (selection == 8) {
        fn_1_E7F4(8);
        return;
    }
    fn_800267CC(lbl_1_bss_198[levelIndex].model[column], &currentPosition);
    startPosition = currentPosition;
    {
        elapsedFrames = 0.0f;
        destinationPosition.x = 0.0f;
        destinationPosition.y = 0.0f;
        destinationPosition.z = 1500.0f;

        angle = 180.0f;
        rotationStep = 30.0f;
        HuAudFXPlay(MSM_SE_MICQUIZ_PANEL_MOVE);
        for (;;) {
            HuPrcVSleep();
            movementStep.x = (destinationPosition.x - currentPosition.x) / (60.0f - elapsedFrames);
            movementStep.y = (destinationPosition.y - currentPosition.y) / (60.0f - elapsedFrames);
            movementStep.z = (destinationPosition.z - currentPosition.z) / (60.0f - elapsedFrames);
            currentPosition.x += movementStep.x;
            currentPosition.y += movementStep.y;
            currentPosition.z += movementStep.z;
            elapsedFrames += 1.0f;
            if (elapsedFrames > 60.0f) {
                currentPosition.x = destinationPosition.x;
                currentPosition.y = destinationPosition.y;
                currentPosition.z = destinationPosition.z;
                break;
            }
            fn_80026790(lbl_1_bss_198[levelIndex].model[column], &currentPosition);
            angle += rotationStep;
            fn_80026808(lbl_1_bss_198[levelIndex].model[column], 0.0f, angle, 0.0f);
        }

        if (selection != 8) {
        if (selection == 0 || selection == 1) {
            HuPrcSleep(3);
            fn_1_1A524(1, 0, 0);
            fn_1_1A854(currentPosition.x, currentPosition.y,
                       10.0f + currentPosition.z, 0.0f);
        } else if (selection == 3) {
            fn_1_1A524(1, 1, 0);
            fn_1_1A854(currentPosition.x, currentPosition.y,
                       10.0f + currentPosition.z, 0.0f);
            HuPrcSleep(15);
            fn_80026760(lbl_1_bss_FE, currentPosition.x, currentPosition.y,
                            5.0f + currentPosition.z);
            fn_80026B8C(lbl_1_bss_FE, 6);
            fn_80026A44(lbl_1_bss_FE, (1 << 0));
            fn_80026CBC(lbl_1_bss_FE, 0.0f);
            opacityAngle = 0.0f;
            for (;;) {
                HuPrcVSleep();
                opacity = (f32)sin((M_PI * opacityAngle) / 180.0);
                fn_80026CBC(lbl_1_bss_FE, opacity);
                opacityAngle += 9.0f;
                if (opacityAngle >= 180.0f) {
                    break;
                }
            }
            fn_80026CBC(lbl_1_bss_FE, 0.0f);
            fn_800269E8(lbl_1_bss_FE, (1 << 0));
        } else if (selection == 2) {
            fn_1_D6EC(lbl_1_bss_62, currentPosition.x,
                      currentPosition.y, currentPosition.z);
        } else if (selection == 4 || selection == 7 || selection == 5) {
            fn_1_D6EC(lbl_1_bss_60, currentPosition.x,
                      currentPosition.y, currentPosition.z);
        } else if (selection == 6) {
            fn_1_1A524(1, 0, 1);
            fn_1_1A854(currentPosition.x, currentPosition.y,
                       10.0f + currentPosition.z, 0.0f);
        }
            if (flags == 0) {
                fn_1_69D0(selection);
            }
        } else {
            if (flags == 0) {
                fn_1_E7F4(8);
            }
        }

        if (flags != 0) {
            fn_1_1C300(levelIndex, column);
        } else {
            HuPrcSleep(30);
        }

        elapsedFrames = 0.0f;
        destinationPosition = startPosition;
        angle = 180.0f;
        rotationStep = 30.0f;
        HuAudFXPlay(MSM_SE_MICQUIZ_PANEL_RETURN);
        for (;;) {
            HuPrcVSleep();
            movementStep.x = (destinationPosition.x - currentPosition.x) / (60.0f - elapsedFrames);
            movementStep.y = (destinationPosition.y - currentPosition.y) / (60.0f - elapsedFrames);
            movementStep.z = (destinationPosition.z - currentPosition.z) / (60.0f - elapsedFrames);
            currentPosition.x += movementStep.x;
            currentPosition.y += movementStep.y;
            currentPosition.z += movementStep.z;
            elapsedFrames += 1.0f;
            if (elapsedFrames > 60.0f) {
                currentPosition.x = destinationPosition.x;
                currentPosition.y = destinationPosition.y;
                currentPosition.z = destinationPosition.z;
                break;
            }
            fn_80026790(lbl_1_bss_198[levelIndex].model[column], &currentPosition);
            angle += rotationStep;
            fn_80026808(lbl_1_bss_198[levelIndex].model[column], 0.0f, angle, 0.0f);
        }
    }
}

/* Enlarge and fade a model at the supplied position during a grid reveal. */
void fn_1_D6EC(HU3D_MODELID modelId, f32 x, f32 y, f32 z) {
    f32 phase;
    f32 scale;
    f32 transparency;

    fn_80026A44(modelId, (1 << 0));
    fn_80026760(modelId, x, y, z - 5.0f);
    fn_80026B8C(modelId, 6);
    phase = 0.0f;
    scale = 1.0f;
    for (;;) {
        HuPrcVSleep();
        scale = (f32)(1.0 + (0.5 * sin((M_PI * phase) / 180.0)));
        transparency = (f32)(1.0 - sin((M_PI * phase) / 180.0));
        fn_80026CBC(modelId, transparency);
        fn_800268B0(modelId, scale, scale, scale);
        phase += 2.0f;
        if (phase >= 90.0f) {
            break;
        }
    }
}

/* Resume the selected grid panel's motion when a reveal starts. */
void fn_1_D870(s32 column, s32 row)
{
    fn_80026A44(lbl_1_bss_198[column].model[row], ((1 << 30) | (1 << 1)));
}

/* Pause and rewind the selected grid panel's motion between reveals. */
void fn_1_D8C4(s32 column, s32 row) {
    fn_800269E8(lbl_1_bss_198[column].model[row], ((1 << 30) | (1 << 1)));
    fn_8002D414(lbl_1_bss_198[column].model[row], 0.0f);
}

/* Find the first unrevealed cell in a grid column, returning -1 when the column is exhausted. */
s32 fn_1_D94C(s32 ignored, s32 column) {
    s32 levelIndex;

    for (levelIndex = 0; levelIndex < 5; levelIndex++) {
        if (fn_1_E69C(levelIndex, column, 2) == 0) {
            return levelIndex;
        }
    }
    return -1;
}

/* Map a percentage roll through the layout's cumulative event thresholds. */
s32 fn_1_D9AC(s32 threshold, s32 tableIndex) {
    s32 index;
    s32 value;

    for (index = 0; index < 4; index++) {
        if (micQuizModeWork->presenterLayout == 0) {
            value = lbl_1_data_D6C[tableIndex - 1][index];
        } else {
            value = lbl_1_data_DAC[tableIndex - 1][index];
        }
        if (threshold <= value) {
            return index;
        }
    }
}

/* Choose a valid event from the round and rank probabilities, rejecting unavailable outcomes. */
s32 fn_1_DA48(void)
{
    s32 questionIndex;
    s32 playerCount;
    s32 currentPlayer;
    s32 category;
    s32 chance;
    s32 answerRoll;
    s32 chancePassedResult;
    s32 chancePassed;
    s32 answer;
    s32 chanceRoll;

    questionIndex = micQuizModeWork->questionCount / micQuizModeWork->playerCount + 1;
    playerCount = micQuizModeWork->playerCount;
    currentPlayer = micQuizModeWork->currentPlayer;
    category = micQuizModeWork->playerRank[currentPlayer];
    if (micQuizModeWork->presenterLayout == 0) {
        chance = lbl_1_data_B8C[playerCount - 2][category - 1][questionIndex - 1];
    } else {
        chance = lbl_1_data_C7C[playerCount - 2][category - 1][questionIndex - 1];
    }
    chanceRoll = frandmod(100);
    if (chance == 0) {
        chancePassed = 0;
    } else {
        if (chanceRoll < chance) {
            chancePassed = 1;
        } else {
            chancePassed = 0;
        }
    }
    chancePassedResult = chancePassed;
    if (chancePassedResult != 0) {
        for (;;) {
            answerRoll = frandmod(100);
            answer = fn_1_D9AC(answerRoll, category);

            if (micQuizModeWork->presenterLayout != 0) {
                if ((answer + 4 == 5) && (micQuizModeWork->questionLevel == 4)) {
                    continue;
                }
                if (answer + 4 == 6) {
                    if (fn_1_639C() != 0) {
                    } else {
                        continue;
                    }
                }
            } else if ((answer == 2) && (micQuizModeWork->questionLevel == 0)) {
                continue;
            }
            if (micQuizModeWork->presenterLayout != 0) {
                answer += 4;
            }
            return answer;
        }
    }
    return 8;
}

/* Set a grid cell's character-color animation bank during its reveal. */
void fn_1_DD10(s32 row, s32 column, s32 bank) {
    Hu3DAnimBankSet(lbl_1_bss_134[row][column],
                    (u16)bank);
}

/* Set a grid cell's event animation bank during its reveal. */
void fn_1_DD68(s32 row, s32 column, s32 bank) {
    Hu3DAnimBankSet(lbl_1_bss_102[row].model[column], (u16)(bank + 1));
}

/* Restore event banks and facing angles for previously revealed grid cells during scene setup. */
void fn_1_DDC4(void) {
    s32 animationBank;
    s32 row;
    s32 column;
    row = 0;
    while (row < 5)
    {
        column = 0;
        while (column < 5)
        {
            if (fn_1_E69C(column, row, 2) != 0U)
            {
                if (fn_1_E69C(column, row, QUIZ_CELL_PLAYER0_FLAG) != 0U)
                {
                    animationBank = 0;
                }
                else
                {
                    if (fn_1_E69C(column, row, QUIZ_CELL_PLAYER1_FLAG) != 0U)
                    {
                        animationBank = 1;
                    }
                    else
                    {
                        if (fn_1_E69C(column, row, QUIZ_CELL_PLAYER2_FLAG) != 0U)
                        {
                            animationBank = 2;
                        }
                        else
                        {
                            if (fn_1_E69C(column, row, QUIZ_CELL_PLAYER3_FLAG) != 0U)
                            {
                                animationBank = 3;
                            }
                            else
                            {
                                animationBank = - 1;
                            }
                        }
                    }
                }
                if (animationBank != - 1)
                {
                    Hu3DAnimBankSet(
                        lbl_1_bss_134[column][row],
                        (u16) animationBank);
                    fn_80026808(lbl_1_bss_198[column].model[row], 0.0f, 180.0f, 0.0f);
                }
                if (animationBank == - 1)
                {
                    Hu3DAnimBankSet(
                        lbl_1_bss_134[column][row],
                        4U);
                    fn_80026808(lbl_1_bss_198[column].model[row], 0.0f, 180.0f, 0.0f);
                }
            }
            column += 1;
        }
        row += 1;
    }
}

/* Show the selected grid cell's result marker with a scale and opacity animation. */
void fn_1_DF9C(s32 mode) {
    f32 unknown = 700.0f; /* The game's code never reads this field. */
    s16 modelId;
    Vec position;
    f32 elapsed;
    f32 scale;
    f32 alpha;

    if (mode != 0) {
        modelId = fn_80025D4C(lbl_1_bss_248);
    } else {
        modelId = fn_80025D4C(lbl_1_bss_246);
    }

    scale = 3.0f;
    alpha = 0.0f;
    position.x = 100.0f * (f32)micQuizModeWork->questionLevel;
    position.y = -300.0f
        + (500.0f
           - 100.0f * (f32)micQuizModeWork->questionColumn);
    position.z = 10.0f;
    fn_80026760(modelId, position.x, position.y, position.z);
    fn_80026A44(modelId, (1 << 0));
    fn_80026B64(modelId, 1);
    fn_80026B8C(modelId, 2);
    fn_800268B0(modelId, 3.0f, 3.0f, 1.0f);
    fn_80026CBC(modelId, 0.0f);

    fn_80026760(lbl_1_bss_244, position.x, position.y, position.z);
    fn_80026B64(lbl_1_bss_244, 1);
    fn_80026B8C(lbl_1_bss_244, 1);
    elapsed = 90.0f;
    scale = 3.0f;
    if (mode != 0) {
        HuAudFXPlay(MSM_SE_MICQUIZ_ALTERNATE_RESULT_MARKER);
    } else {
        HuAudFXPlay(MSM_SE_MICQUIZ_RESULT_MARKER);
    }
    do {
        HuPrcVSleep();
        scale = (f32)(1.0 + (2.0 * sin(M_PI * (f64)elapsed / 180.0)));
        alpha = (f32)(1.0 - sin((f64)elapsed * M_PI / 180.0));
        fn_800268B0(modelId, scale, scale, 1.0f);
        fn_80026CBC(modelId, alpha);
        elapsed += 3.0f;
    } while (!(elapsed >= 180.0f));

    fn_800268B0(modelId, 1.0f, 1.0f, 1.0f);
    fn_80026CBC(modelId, 1.0f);
    fn_800268B0(lbl_1_bss_244, 0.0f, 0.0f, 1.0f);
    fn_80026A44(lbl_1_bss_244, (1 << 0));
    fn_80026A44(lbl_1_bss_244, ((1 << 30) | (1 << 1)));

    scale = 0.0f;
    alpha = 1.0f;
    elapsed = 0.0f;
    do {
        HuPrcVSleep();
        scale = (f32)(sin(M_PI * (f64)elapsed / 180.0) * 4.0);
        alpha = (f32)(1.0 - sin((f64)elapsed * M_PI / 180.0));
        fn_800268B0(lbl_1_bss_244, scale, scale, 1.0f);
        fn_80026CBC(lbl_1_bss_244, alpha);
        elapsed += 6.0f;
    } while (!(elapsed >= 90.0f));
    HuPrcSleep(30);
}

/* Recreate result markers for previously revealed cells other than the current selection. */
void fn_1_E474(void) {
    s16 markerModel;
    s32 row;
    s32 column;

    /* A marked cell is expected to select one of the two marker models. */
    row = 0;
    while (row < 5) {
        column = 0;
        while (column < 5) {
            if ((fn_1_E69C(column, row, 2) != 0U) &&
                ((row != (s32) micQuizModeWork->questionColumn) ||
                 (column != (s32) micQuizModeWork->questionLevel))) {
                if (fn_1_E69C(column, row, QUIZ_CELL_HAS_RESULT_FLAG) != 0U) {
                    markerModel = fn_80025D4C(lbl_1_bss_248);
                } else if (fn_1_E69C(column, row, QUIZ_CELL_NO_RESULT_FLAG) != 0U) {
                    markerModel = fn_80025D4C(lbl_1_bss_246);
                }
                fn_80026760(markerModel, 100.0f * (f32) column,
                                -300.0f + (500.0f - (100.0f * (f32) row)), 10.0f);
                fn_80026A44(markerModel, (1 << 0));
                fn_80026B64(markerModel, 1U);
                fn_80026B8C(markerModel, 2);
            }
            column += 1;
        }
        row += 1;
    }
}

/* Fill a position vector for the scene and effect helpers. */
void fn_1_E634(HuVecF *out, f32 x, f32 y, f32 z)
{
    out->x = x;
    out->y = y;
    out->z = z;
}

/* Add the requested state bits to one question-grid cell. */
void fn_1_E644(s32 row, s32 column, s32 mask)
{
    micQuizModeWork->questionFlags[row][column] |= mask;
}

/* Clear the requested state bits from one question-grid cell. */
void fn_1_E670(s32 row, s32 column, s32 mask)
{
    micQuizModeWork->questionFlags[row][column] &= ~mask;
}

/* Test the requested state bits of one question-grid cell. */
u32 fn_1_E69C(s32 row, s32 column, s32 mask)
{
    return micQuizModeWork->questionFlags[row][column] & mask;
}

/* Record which player answered a grid cell and choose its result marker. */
void fn_1_E6C0(s32 row, s32 column, s32 answeringPlayer, s32 hasResult) {
    s32 playerFlag;

    if (hasResult != 0) {
        switch (answeringPlayer) {
        case 0:
            playerFlag = QUIZ_CELL_PLAYER0_FLAG;
            break;
        case 1:
            playerFlag = QUIZ_CELL_PLAYER1_FLAG;
            break;
        case 2:
            playerFlag = QUIZ_CELL_PLAYER2_FLAG;
            break;
        case 3:
            playerFlag = QUIZ_CELL_PLAYER3_FLAG;
            break;
        }
        micQuizModeWork->questionFlags[row][column] |= playerFlag;
    }
    if (hasResult != 0) {
        micQuizModeWork->questionFlags[row][column] |= QUIZ_CELL_HAS_RESULT_FLAG;
        micQuizModeWork->questionFlags[row][column] &= ~QUIZ_CELL_NO_RESULT_FLAG;
    } else {
        micQuizModeWork->questionFlags[row][column] |= QUIZ_CELL_NO_RESULT_FLAG;
        micQuizModeWork->questionFlags[row][column] &= ~QUIZ_CELL_HAS_RESULT_FLAG;
    }
}

/* Select the event used by the following question presentation. */
void fn_1_E7F4(s16 selection) {
    micQuizModeWork->questionEvent = selection;
}

/* Clear the selected event before the next grid choice. */
void fn_1_E80C(void) {
    micQuizModeWork->questionEvent = 8;
}

/* Update all four three-digit score displays and advance the active model-piece effects. */
void fn_1_E824(void) {
    s32 index;
    s32 value;
    s32 hundreds;
    s32 tens;
    s32 ones;

    for (index = 0; index < 4; index++) {
        value = micQuizModeWork->playerScore[index];
        hundreds = value / 100;
        tens = (value - hundreds * 100) / 10;
        ones = value - hundreds * 100 - tens * 10;
        Hu3DAnimBankSet(lbl_1_bss_B4[index][0], (u16)hundreds);
        Hu3DAnimBankSet(lbl_1_bss_B4[index][1], (u16)tens);
        Hu3DAnimBankSet(lbl_1_bss_B4[index][2], (u16)ones);
    }
    fn_1_31FCC();
}

/* Introduce the quiz with alternating presenter dialogue before the first choice. */
void fn_1_E91C(void) {
    MicQuizWinOpen(2);
    fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
    fn_1_A3B4(1, 1, 8.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 0), 1, 2);
    MicQuizWinMesWait();
    MicQuizWinClose();
    fn_1_A3B4(0, 0, 8.0f, ((1 << 30) | (1 << 0)));
    fn_1_A3B4(1, 0, 8.0f, ((1 << 30) | (1 << 0)));

    MicQuizWinOpen(0);
    fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 1), 1, 0);
    MicQuizWinMesWait();
    MicQuizWinClose();
    fn_1_A3B4(0, 0, 8.0f, ((1 << 30) | (1 << 0)));

    MicQuizWinOpen(1);
    fn_1_A3B4(1, 1, 8.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 2), 1, 1);
    MicQuizWinMesWait();
}

/* Show the initial quiz choice prompt and return its selected response. */
s16 fn_1_EF58(void) {
    MicQuizWinOpen(1);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 3), 1, 1);
    MicQuizWinMesWait();
    return HuWinChoiceGet(micQuizActiveWin, -1);
}

/* Show the alternate quiz choice prompt and return its selected response. */
s16 fn_1_F084(void) {
    MicQuizWinOpen(1);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 59), 1, 1);
    MicQuizWinMesWait();
    return HuWinChoiceGet(micQuizActiveWin, -1);
}

/* Show the left presenter's follow-up and restore its idle motion after the text finishes. */
void fn_1_F1B0(void) {
    MicQuizWinOpen(0);
    fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 4), 1, 0);
    MicQuizWinMesWait();
    MicQuizWinClose();
    fn_1_A3B4(0, 0, 8.0f, ((1 << 30) | (1 << 0)));
}

/* Introduce a player by number and character name before that player takes a turn. */
void fn_1_F3CC(s32 playerIndex) {
    char playerNumber[20];

    MicQuizWinOpen(0);
    sprintf(playerNumber, "%d", playerIndex + 1);
    MicQuizWinInsertMesSet(0, (u32)playerNumber, 0);
    MicQuizWinInsertMesSet(0, micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerIndex]],
                           1);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 5), 1, 0);
    MicQuizWinMesWait();
}

/* Show the shared presenter prompt and close its window when the text finishes. */
void fn_1_F568(void) {
    MicQuizWinOpen(2);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 6), 1, 2);
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Address the current player by character name before the next selection. */
void fn_1_F6B4(void) {
    MicQuizWinOpen(0);
    MicQuizWinInsertMesSet(0,
            micQuizCharaNameMes[
                    micQuizModeWork->playerCharacter[micQuizModeWork->currentPlayer]], 0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 7), 1, 0);
    MicQuizWinMesWait();
}

/* Address the current player with the prompt for the active presenter layout. */
void fn_1_F824(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinInsertMesSet(0,
                micQuizCharaNameMes[
                        micQuizModeWork->playerCharacter[micQuizModeWork->currentPlayer]],
                   0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 8), 1, 0);
        MicQuizWinMesWait();
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3,
                micQuizCharaNameMes[
                        micQuizModeWork->playerCharacter[micQuizModeWork->currentPlayer]],
                   0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 100), 1, 3);
        MicQuizWinMesWait();
    }
}

/* Announce the selected grid column and level, then pause before closing the window. */
void fn_1_FAF4(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        MicQuizWinInsertMesSet(1, lbl_1_data_40C[micQuizModeWork->questionColumn], 0);
        MicQuizWinInsertMesSet(1, lbl_1_data_3F8[micQuizModeWork->questionLevel], 1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 11), 1, 1);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3, lbl_1_data_40C[micQuizModeWork->questionColumn], 0);
        MicQuizWinInsertMesSet(3, lbl_1_data_3F8[micQuizModeWork->questionLevel], 1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 74), 1, 3);
    }
    MicQuizWinMesWait();
    HuPrcSleep(30);
    MicQuizWinClose();
}

/* Show the round-count transition message before the quiz overlay changes. */
void fn_1_FE40(void) {
    char questionCountText[32];

    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(2);
        sprintf(questionCountText, "%d", micQuizModeWork->questionCount);
        MicQuizWinInsertMesSet(2, (u32)questionCountText, 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 12), 1, 2);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 75), 1, 3);
    }
    MicQuizWinMesWait();
    HuPrcSleep(120);
    MicQuizWinClose();
}

/* Announce the answering player and the current point amount during response resolution. */
void fn_1_100BC(void) {
    char number[16];

    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        MicQuizWinInsertMesSet(
            1, micQuizCharaNameMes[
                    micQuizModeWork->playerCharacter[(s8) micQuizModeWork->answeringPlayer]],
            0);
        sprintf(number, "%d", micQuizModeWork->roundPoints);
        MicQuizWinInsertMesSet(1, (u32)number, 1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 14), 1, 1);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(
            3, micQuizCharaNameMes[
                    micQuizModeWork->playerCharacter[(s8) micQuizModeWork->answeringPlayer]],
            0);
        sprintf(number, "%d", micQuizModeWork->roundPoints);
        MicQuizWinInsertMesSet(3, (u32)number, 1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 77), 1, 3);
    }
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Show the layout-specific response follow-up and close the window afterward. */
void fn_1_1042C(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 51), 1, 1);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 107), 1, 3);
    }
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Show player feedback with a character name in the two-presenter layout. */
void fn_1_1065C(s32 playerIndex) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinInsertMesSet(0,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerIndex]], 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 41), 1, 0);
        MicQuizWinMesWait();
        MicQuizWinClose();
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 97), 1, 3);
        MicQuizWinMesWait();
        MicQuizWinClose();
    }
}

/* Show the right presenter's named-player feedback when the two-presenter layout is active. */
void fn_1_10934(s32 playerIndex) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        MicQuizWinInsertMesSet(1,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerIndex]], 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 42), 1, 1);
        MicQuizWinMesWait();
        MicQuizWinClose();
    }
}

/* Show single-presenter feedback naming the first stored response participant. */
void fn_1_10AE4(void) {
    MicQuizWinOpen(3);
    MicQuizWinInsertMesSet(
        3, micQuizCharaNameMes[
                micQuizModeWork->playerCharacter[(s8) micQuizModeWork->answerPlayer[0]]], 0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 106), 1, 3);
    MicQuizWinMesWait();
}

/* Show single-presenter feedback naming both stored response participants. */
void fn_1_10C58(void) {
    MicQuizWinOpen(3);
    MicQuizWinInsertMesSet(
        3, micQuizCharaNameMes[
                micQuizModeWork->playerCharacter[(s8) micQuizModeWork->answerPlayer[0]]], 0);
    MicQuizWinInsertMesSet(
        3, micQuizCharaNameMes[
                micQuizModeWork->playerCharacter[(s8) micQuizModeWork->answerPlayer[1]]], 1);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 108), 1, 3);
    MicQuizWinMesWait();
}

/* Choose one of two feedback lines for the answering player, then close the presenter window. */
void fn_1_10E28(void) {
    s32 choice;

    choice = frandmod(2);
    if (micQuizModeWork->presenterLayout == 0) {
        if (choice != 0) {
            MicQuizWinOpen(0);
            MicQuizWinInsertMesSet(
                0, micQuizCharaNameMes[
                        micQuizModeWork->playerCharacter[micQuizModeWork->answeringPlayer]],
                0);
            MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 41), 1, 0);
        } else {
            MicQuizWinOpen(1);
            MicQuizWinInsertMesSet(
                1, micQuizCharaNameMes[
                        micQuizModeWork->playerCharacter[micQuizModeWork->answeringPlayer]],
                0);
            MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 42), 1, 1);
        }
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3,
                micQuizCharaNameMes[
                        micQuizModeWork->playerCharacter[micQuizModeWork->answeringPlayer]],
                   0);
        if (choice != 0) {
            MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 97), 1, 3);
        } else {
            MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 98), 1, 3);
        }
    }
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Show the named participant's single-presenter feedback during the reaction sequence. */
void fn_1_112B0(void) {
    MicQuizWinOpen(3);
    MicQuizWinInsertMesSet(
        3, micQuizCharaNameMes[
                micQuizModeWork->playerCharacter[(s8) micQuizModeWork->answerPlayer[0]]], 0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 106), 1, 3);
    MicQuizWinMesWait();
}

/* Show the single-presenter response follow-up and wait for its text to finish. */
void fn_1_11424(void) {
    MicQuizWinOpen(3);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 107), 1, 3);
    MicQuizWinMesWait();
}

/* Name both stored participants in the single-presenter response follow-up. */
void fn_1_1153C(void) {
    MicQuizWinOpen(3);
    MicQuizWinInsertMesSet(
        3, micQuizCharaNameMes[
                micQuizModeWork->playerCharacter[(s8) micQuizModeWork->answerPlayer[0]]], 0);
    MicQuizWinInsertMesSet(
        3, micQuizCharaNameMes[
                micQuizModeWork->playerCharacter[(s8) micQuizModeWork->answerPlayer[1]]], 1);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 108), 1, 3);
    MicQuizWinMesWait();
}

/* Announce the current player and, in the two-presenter layout, the round count. */
void fn_1_1170C(void) {
    char questionCountText[16];
    u32 messageId;
    u32 alternateCharacterMessage;

    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        sprintf(questionCountText, "%d", micQuizModeWork->questionCount);
        MicQuizWinInsertMesSet(0, (u32)questionCountText, 0);
        messageId =
                micQuizCharaNameMes[
                        micQuizModeWork->playerCharacter[micQuizModeWork->currentPlayer]];
        MicQuizWinInsertMesSet(0, messageId, 1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 15), 1, 0);
    } else {
        MicQuizWinOpen(3);
        alternateCharacterMessage =
            micQuizCharaNameMes[micQuizModeWork->playerCharacter[micQuizModeWork->currentPlayer]];
        MicQuizWinInsertMesSet(3, alternateCharacterMessage, 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 78), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Announce the round count and current player in the single-presenter layout. */
void fn_1_119FC(void) {
    char questionCountText[20];

    MicQuizWinOpen(3);
    sprintf(questionCountText, "%d", micQuizModeWork->questionCount);
    MicQuizWinInsertMesSet(3, (u32)questionCountText, 0);
    MicQuizWinInsertMesSet(3,
            micQuizCharaNameMes[
                    micQuizModeWork->playerCharacter[micQuizModeWork->currentPlayer]], 1);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 70), 1, 3);
    MicQuizWinMesWait();
}

/* Show the exhausted-column explanation after a question-grid choice is rejected. */
void fn_1_11BB0(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 10), 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 73), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Show the unavailable-cell explanation after a question-grid choice is rejected. */
void fn_1_11DAC(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 8), 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 71), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Show the layout-specific selection explanation and wait before continuing. */
void fn_1_11FA8(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 9), 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 72), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Show the layout-specific round follow-up and wait before continuing. */
void fn_1_121A4(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 55), 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 114), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Name the current player in the layout-specific round follow-up. */
void fn_1_123A0(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        s32 playerIndex;

        MicQuizWinOpen(1);
        playerIndex = micQuizModeWork->currentPlayer;
        MicQuizWinInsertMesSet(1,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerIndex]], 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 43), 1, 1);
    } else {
        s32 playerIndex;

        MicQuizWinOpen(3);
        playerIndex = micQuizModeWork->currentPlayer;
        MicQuizWinInsertMesSet(3,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerIndex]], 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 99), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Present two formatted point amounts for a score-change explanation. */
void fn_1_1264C(s32 firstValue, s32 secondValue) {
    char firstText[20];
    char secondText[20];

    MicQuizWinOpen(0);
    sprintf(firstText, "%d", firstValue);
    sprintf(secondText, "%d", secondValue);
    MicQuizWinInsertMesSet(0, (u32)firstText, 0);
    MicQuizWinInsertMesSet(0, (u32)secondText, 1);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 45), 1, 0);
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Present the alternate two-amount explanation for a score change. */
void fn_1_12810(s32 firstValue, s32 secondValue) {
    char firstText[20];
    char secondText[20];

    MicQuizWinOpen(0);
    sprintf(firstText, "%d", firstValue);
    sprintf(secondText, "%d", secondValue);
    MicQuizWinInsertMesSet(0, (u32)firstText, 0);
    MicQuizWinInsertMesSet(0, (u32)secondText, 1);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 46), 1, 0);
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Present score standings and tied-player reactions, then select the closing commentary. */
void fn_1_129D4(void)
{
    s32 statusCounts[4];
    s32 scoreOrder[4];
    s32 index;
    s32 otherIndex;
    s32 firstPlayer;
    s32 state;
    s32 firstStatePlayer;
    s32 presentationChoice;

    presentationChoice = frandmod(2);
    fn_1_8488();
    fn_1_A838(lbl_1_data_5F8, 120, 4);
    HuAudFXPlay(MSM_SE_MICQUIZ_STANDINGS);
    fn_1_13B54();
    if (presentationChoice == 0) {
        micQuizModeWork->presenterLayout = 1;
        micQuizModeWork->pauseAllowed = 0;
        fn_1_40FC();
        micQuizModeWork->pauseAllowed = 1;
        fn_1_13878();
    }
    fn_1_A838(lbl_1_data_5C0, 120, 4);
    if (fn_1_632C() != 0) {
        firstPlayer = -1;
        for (index = 0; index < 4; index++) {
            if (micQuizModeWork->playerActive[index] != 0) {
                if (firstPlayer == -1) {
                    firstPlayer = index;
                }
                HuAudFXPlay(lbl_1_data_3E0[micQuizModeWork->playerCharacter[index]]);
                fn_1_A678(index, 2, 8.0f, 0);
            }
        }
        fn_1_15EF8();
        HuPrcSleep(8);
        fn_1_A748(firstPlayer);
        HuPrcSleep(60);
        for (index = 0; index < 4; index++) {
            if (micQuizModeWork->playerActive[index] != 0) {
                fn_1_A678(index, 0, 8.0f, ((1 << 30) | (1 << 0)));
            }
        }
        return;
    }

    for (otherIndex = 0; otherIndex < 4; otherIndex++) {
        for (index = 0, statusCounts[otherIndex] = 0; index < 4; index++) {
            if (micQuizModeWork->playerRank[index] == 1) {
                statusCounts[otherIndex]++;
            }
        }
    }
    for (otherIndex = 0, state = 4; otherIndex < 3; otherIndex++, state--) {
        firstStatePlayer = 0;
        for (index = 0; index < 4; index++) {
            if (micQuizModeWork->playerRank[index] == state) {
                if (firstStatePlayer == 0) {
                    if (micQuizModeWork->playerActive[index] != 0) {
                        fn_1_13D70(state, index);
                        HuAudFXPlay(lbl_1_data_3E0[micQuizModeWork->playerCharacter[index]]);
                        fn_1_A678(index, 2, 8.0f, 0);
                        HuPrcSleep(8);
                        fn_1_A748(index);
                        HuPrcSleep(60);
                        fn_1_A678(index, 0, 16.0f, ((1 << 30) | (1 << 0)));
                    }
                } else if (micQuizModeWork->playerActive[index] != 0) {
                    fn_1_142F4(state, index);
                    HuAudFXPlay(lbl_1_data_3E0[micQuizModeWork->playerCharacter[index]]);
                    fn_1_A678(index, 2, 8.0f, 0);
                    HuPrcSleep(8);
                    fn_1_A748(index);
                    HuPrcSleep(60);
                    fn_1_A678(index, 0, 16.0f, ((1 << 30) | (1 << 0)));
                }
                firstStatePlayer = 1;
            }
        }
    }
    HuPrcSleep(30);
    for (index = 0; index < 4; index++) {
        scoreOrder[index] = micQuizModeWork->playerScore[index];
    }
    fn_1_A97C(scoreOrder, 4);
    for (index = 0; index < 4; index++) {
        lbl_1_bss_7C[index] = -1;
    }
    for (index = 0, otherIndex = 0; index < 4; index++) {
        if (micQuizModeWork->playerScore[index] == scoreOrder[0]) {
            lbl_1_bss_7C[otherIndex] = index;
            otherIndex++;
        }
    }
    if (micQuizModeWork->playerCount == statusCounts[0]) {
        fn_1_15AB0(lbl_1_bss_7C[0]);
    } else {
        switch (statusCounts[0]) {
        case 1: {
            fn_1_148E4(lbl_1_bss_7C[0]);
            HuAudFXPlay(lbl_1_data_3CA[micQuizModeWork->playerCharacter[lbl_1_bss_7C[0]]]);
            fn_1_13740(lbl_1_bss_7C[0], -1, -1, -1, 7);
            HuPrcSleep(8);
            fn_1_A748(lbl_1_bss_7C[0]);
            fn_1_13740(lbl_1_bss_7C[0], -1, -1, -1, 0);
            break;
        }
        case 2: {
            fn_1_14E48(lbl_1_bss_7C[0], lbl_1_bss_7C[1]);
            HuAudFXPlay(lbl_1_data_3CA[micQuizModeWork->playerCharacter[lbl_1_bss_7C[0]]]);
            HuAudFXPlay(lbl_1_data_3CA[micQuizModeWork->playerCharacter[lbl_1_bss_7C[1]]]);
            fn_1_13740(lbl_1_bss_7C[0], lbl_1_bss_7C[1], -1, -1, 7);
            HuPrcSleep(8);
            fn_1_A748(lbl_1_bss_7C[0]);
            fn_1_13740(lbl_1_bss_7C[0], lbl_1_bss_7C[1], -1, -1, 0);
            break;
        }
        case 3: {
            fn_1_15424(lbl_1_bss_7C[0], lbl_1_bss_7C[1], lbl_1_bss_7C[2]);
            HuAudFXPlay(lbl_1_data_3CA[micQuizModeWork->playerCharacter[lbl_1_bss_7C[0]]]);
            HuAudFXPlay(lbl_1_data_3CA[micQuizModeWork->playerCharacter[lbl_1_bss_7C[1]]]);
            HuAudFXPlay(lbl_1_data_3CA[micQuizModeWork->playerCharacter[lbl_1_bss_7C[2]]]);
            fn_1_13740(lbl_1_bss_7C[0], lbl_1_bss_7C[1], lbl_1_bss_7C[2], -1, 7);
            HuPrcSleep(8);
            fn_1_A748(lbl_1_bss_7C[0]);
            fn_1_13740(lbl_1_bss_7C[0], lbl_1_bss_7C[1], lbl_1_bss_7C[2], -1, 0);
            break;
        }
        }
    }
    if (micQuizModeWork->presenterLayout == 0) {
        fn_1_1E56C();
        return;
    }
    fn_1_1E944();
}

/* Start the requested reaction motion for each listed player, skipping -1 entries. */
void fn_1_13740(s32 firstPlayer, s32 secondPlayer, s32 thirdPlayer, s32 fourthPlayer,
                s32 motionIndex) {
    s32 players[4];
    s32 index;
    s32 playerNo;
    s16 motionId;

    players[0] = firstPlayer;
    players[1] = secondPlayer;
    players[2] = thirdPlayer;
    players[3] = fourthPlayer;
    for (index = 0; index < 4; index++) {
        if (players[index] != -1) {
            playerNo = players[index];
            motionId = fn_8002D3C4(lbl_1_bss_2FC[playerNo]);
            if (motionId != lbl_1_bss_2AC[playerNo][motionIndex]) {
                fn_8005C2C8(lbl_1_bss_2F4[playerNo], lbl_1_bss_2AC[playerNo][motionIndex],
                                   0.0f, 8.0f, 0);
            }
        }
    }
}

/* Play the single presenter's three-part standings dialogue with presenter motion changes. */
void fn_1_13878(void) {
    s16 introMotionId;
    s16 speakingMotionId;

    MicQuizWinOpen(3);
    introMotionId = fn_8002D3C4(lbl_1_bss_294);
    if (introMotionId != lbl_1_bss_28C[3]) {
        fn_8002CCC0(lbl_1_bss_294, lbl_1_bss_28C[3],
                           0.0f, 8.0f, ((1 << 30) | (1 << 0)));
    }
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 60), 1, 3);
    MicQuizWinMesWait();
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 61), 1, 3);
    speakingMotionId = fn_8002D3C4(lbl_1_bss_294);
    if (speakingMotionId != lbl_1_bss_28C[1]) {
        fn_8002CCC0(lbl_1_bss_294, lbl_1_bss_28C[1],
                           0.0f, 8.0f, ((1 << 30) | (1 << 0)));
    }
    MicQuizWinMesWait();
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 62), 1, 3);
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Play the left presenter's standings line, then restore its idle motion. */
void fn_1_13B54(void) {
    MicQuizWinOpen(0);
    fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 18), 1, 0);
    MicQuizWinMesWait();
    fn_1_A3B4(0, 0, 8.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinClose();
}

/* Announce a player's placing and score with the active presenter layout. */
void fn_1_13D70(s32 messageNo, s32 playerNo) {
    char playerScore[19];

    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinInsertMesSet(0, lbl_1_data_8A0[messageNo - 1], 0);
        MicQuizWinInsertMesSet(0, micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerNo]],
                               1);
        sprintf(playerScore, "%d", micQuizModeWork->playerScore[playerNo]);
        MicQuizWinInsertMesSet(0, (u32)playerScore, 2);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 19), 1, 0);
        MicQuizWinMesWait();
        fn_1_A3B4(0, 0, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinClose();
    } else {
        MicQuizWinOpen(3);
        fn_1_A4CC(1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinInsertMesSet(3, lbl_1_data_8A0[messageNo - 1], 0);
        MicQuizWinInsertMesSet(3, micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerNo]],
                               1);
        sprintf(playerScore, "%d", micQuizModeWork->playerScore[playerNo]);
        MicQuizWinInsertMesSet(3, (u32)playerScore, 2);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 63), 1, 3);
        MicQuizWinMesWait();
        MicQuizWinClose();
    }
}

/* Announce the alternate placing and score line while the presenter speaks. */
void fn_1_142F4(s32 messageNo, s32 playerNo) {
    char playerScore[20];

    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        fn_1_A3B4(1, 1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinInsertMesSet(1, lbl_1_data_8A0[messageNo - 1], 0);
        MicQuizWinInsertMesSet(1, micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerNo]],
                               1);
        sprintf(playerScore, "%d", micQuizModeWork->playerScore[playerNo]);
        MicQuizWinInsertMesSet(1, (u32)playerScore, 2);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 20), 1, 1);
        MicQuizWinMesWait();
        fn_1_A3B4(1, 0, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinClose();
    } else {
        MicQuizWinOpen(3);
        fn_1_A4CC(1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinInsertMesSet(3, lbl_1_data_8A0[messageNo - 1], 0);
        MicQuizWinInsertMesSet(3, micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerNo]],
                               1);
        sprintf(playerScore, "%d", micQuizModeWork->playerScore[playerNo]);
        MicQuizWinInsertMesSet(3, (u32)playerScore, 2);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 64), 1, 3);
        MicQuizWinMesWait();
        fn_1_A4CC(0, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinClose();
    }
}

/* Name a player and their score during the standings dialogue. */
void fn_1_148E4(s32 playerIndex) {
    char playerScore[21];

    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        fn_1_A3B4(1, 1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinInsertMesSet(1,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerIndex]], 0);
        sprintf(playerScore, "%d", micQuizModeWork->playerScore[playerIndex]);
        MicQuizWinInsertMesSet(1, (u32)playerScore, 1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 21), 1, 1);
        MicQuizWinMesWait();
        fn_1_A3B4(1, 0, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinClose();
    } else {
        MicQuizWinOpen(3);
        fn_1_A4CC(1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinInsertMesSet(3,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerIndex]], 0);
        sprintf(playerScore, "%d", micQuizModeWork->playerScore[playerIndex]);
        MicQuizWinInsertMesSet(3, (u32)playerScore, 1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 65), 1, 3);
        MicQuizWinMesWait();
        fn_1_A4CC(0, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinClose();
    }
}

/* Describe one player relative to another, inserting the first player's score. */
void fn_1_14E48(s32 playerNo, s32 otherPlayerNo) {
    char playerScore[20];

    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinInsertMesSet(0, micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerNo]],
                               0);
        MicQuizWinInsertMesSet(0,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[otherPlayerNo]], 1);
        sprintf(playerScore, "%d", micQuizModeWork->playerScore[playerNo]);
        MicQuizWinInsertMesSet(0, (u32)playerScore, 2);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 22), 1, 0);
        MicQuizWinMesWait();
        fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
    } else {
        MicQuizWinOpen(3);
        fn_1_A4CC(1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinInsertMesSet(3, micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerNo]],
                               0);
        MicQuizWinInsertMesSet(3,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[otherPlayerNo]], 1);
        sprintf(playerScore, "%d", micQuizModeWork->playerScore[playerNo]);
        MicQuizWinInsertMesSet(3, (u32)playerScore, 2);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 66), 1, 3);
        MicQuizWinMesWait();
        fn_1_A4CC(0, 8.0f, ((1 << 30) | (1 << 0)));
    }
    MicQuizWinClose();
}

/* Describe one player relative to two others, inserting that player's score. */
void fn_1_15424(s32 playerNo, s32 firstOtherPlayer, s32 secondOtherPlayer) {
    char playerScore[20];

    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinInsertMesSet(0, micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerNo]],
                               0);
        MicQuizWinInsertMesSet(0,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[firstOtherPlayer]], 1);
        MicQuizWinInsertMesSet(0,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[secondOtherPlayer]], 2);
        sprintf(playerScore, "%d", micQuizModeWork->playerScore[playerNo]);
        MicQuizWinInsertMesSet(0, (u32)playerScore, 3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 23), 1, 0);
        MicQuizWinMesWait();
        fn_1_A3B4(0, 0, 8.0f, ((1 << 30) | (1 << 0)));
    } else {
        MicQuizWinOpen(3);
        fn_1_A4CC( 1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinInsertMesSet(3, micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerNo]],
                               0);
        MicQuizWinInsertMesSet(3,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[firstOtherPlayer]], 1);
        MicQuizWinInsertMesSet(3,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[secondOtherPlayer]], 2);
        sprintf(playerScore, "%d", micQuizModeWork->playerScore[playerNo]);
        MicQuizWinInsertMesSet(3, (u32)playerScore, 3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 67), 1, 3);
        MicQuizWinMesWait();
        fn_1_A4CC( 0, 8.0f, ((1 << 30) | (1 << 0)));
    }
    MicQuizWinClose();
}

/* Show the score follow-up while the active presenter speaks. */
void fn_1_15AB0(s32 playerIndex) {
    char playerScore[20];

    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
        sprintf(playerScore, "%d", micQuizModeWork->playerScore[playerIndex]);
        MicQuizWinInsertMesSet(0, (u32)playerScore, 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 24), 1, 0);
        MicQuizWinMesWait();
        fn_1_A3B4(0, 0, 8.0f, ((1 << 30) | (1 << 0)));
    } else {
        MicQuizWinOpen(3);
        fn_1_A4CC(1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 68), 1, 3);
        MicQuizWinMesWait();
        fn_1_A4CC(0, 8.0f, ((1 << 30) | (1 << 0)));
    }
    MicQuizWinClose();
}

/* Show the layout-specific standings follow-up and close the dialogue window. */
void fn_1_15EF8(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 52), 1, 1);
        MicQuizWinMesWait();
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 105), 1, 3);
        MicQuizWinMesWait();
    }
    MicQuizWinClose();
}

/* Present the first final-results line with the active presenters' speaking motions. */
void fn_1_1614C(void) {

    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(2);
        fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
        fn_1_A3B4(1, 1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 30), 1, 2);
        MicQuizWinMesWait();
        HuPrcSleep(120);
        fn_1_A3B4(0, 0, 8.0f, ((1 << 30) | (1 << 0)));
        fn_1_A3B4(1, 0, 8.0f, ((1 << 30) | (1 << 0)));
    } else {
        MicQuizWinOpen(3);
        fn_1_A4CC(1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 86), 1, 3);
        MicQuizWinMesWait();
        HuPrcSleep(120);
        fn_1_A4CC(0, 8.0f, ((1 << 30) | (1 << 0)));
    }
    MicQuizWinClose();
}

/* Present the next final-results line and restore the speaking presenter's idle motion. */
void fn_1_16620(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        fn_1_A3B4(1, 1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 31), 1, 1);
        MicQuizWinMesWait();
        HuPrcSleep(120);
        fn_1_A3B4(1, 0, 8.0f, ((1 << 30) | (1 << 0)));
    } else {
        MicQuizWinOpen(3);
        fn_1_A4CC( 1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 87), 1, 3);
        MicQuizWinMesWait();
        HuPrcSleep(120);
        fn_1_A4CC( 0, 8.0f, ((1 << 30) | (1 << 0)));
    }
    MicQuizWinClose();
}

/* Present the shared final-results follow-up, pausing before restoring the idle motion. */
void fn_1_16A24(void) {
    s16 score;

    score = fn_1_16E30();
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 32), 1, 0);
        MicQuizWinMesWait();
        HuPrcSleep(120);
        fn_1_A3B4(0, 0, 8.0f, ((1 << 30) | (1 << 0)));
    } else {
        MicQuizWinOpen(3);
        fn_1_A4CC(1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 88), 1, 3);
        MicQuizWinMesWait();
        HuPrcSleep(120);
        fn_1_A4CC(0, 8.0f, ((1 << 30) | (1 << 0)));
    }
    MicQuizWinClose();
}

/* Return the last active player's positive score encountered while scanning the result list. */
s16 fn_1_16E30(void) {
    s16 value;
    s16 index;

    value = 0;
    index = 0;
    while (index < 4) {
        if (micQuizModeWork->playerActive[index] != 0
            && micQuizModeWork->playerScore[index] > 0) {
            value = micQuizModeWork->playerScore[index];
        }
        index++;
    }
    return value;
}

/* Read a player's score in the final-results dialogue and pause before closing it. */
void fn_1_16ECC(s32 playerIndex)
{
    char scoreText[20];

    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        sprintf(scoreText, "%d", micQuizModeWork->playerScore[playerIndex]);
        MicQuizWinInsertMesSet(1, (u32)scoreText, 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 33), 1, 1);
        MicQuizWinMesWait();
    } else {
        MicQuizWinOpen(3);
        sprintf(scoreText, "%d", micQuizModeWork->playerScore[playerIndex]);
        MicQuizWinInsertMesSet(3, (u32)scoreText, 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 89), 1, 3);
        MicQuizWinMesWait();
    }
    HuPrcSleep(120);
    MicQuizWinClose();
}

/* Announce the sole winner by character name during final results. */
void fn_1_171CC(s32 playerIndex, s32 secondPlayer, s32 thirdPlayer, s32 fourthPlayer) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(2);
        MicQuizWinInsertMesSet(2,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerIndex]], 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 34), 1, 2);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerIndex]], 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 90), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Announce two tied winners by character name during final results. */
void fn_1_1745C(s32 firstPlayer, s32 secondPlayer, s32 thirdPlayer, s32 fourthPlayer)
{
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(2);
        MicQuizWinInsertMesSet(2,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[firstPlayer]], 0);
        MicQuizWinInsertMesSet(2,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[secondPlayer]], 1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 35), 1, 2);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[firstPlayer]], 0);
        MicQuizWinInsertMesSet(3,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[secondPlayer]], 1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 91), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Announce three tied winners by character name during final results. */
void fn_1_1778C(s32 firstPlayer, s32 secondPlayer, s32 thirdPlayer, s32 fourthPlayer)
{
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(2);
        MicQuizWinInsertMesSet(2,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[firstPlayer]], 0);
        MicQuizWinInsertMesSet(2,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[secondPlayer]], 1);
        MicQuizWinInsertMesSet(2,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[thirdPlayer]], 2);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 36), 1, 2);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinInsertMesSet(3,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[firstPlayer]], 0);
        MicQuizWinInsertMesSet(3,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[secondPlayer]], 1);
        MicQuizWinInsertMesSet(3,
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[thirdPlayer]], 2);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 92), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Show the all-player tie message during final results. */
void fn_1_17B6C(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(2);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 37), 1, 2);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 93), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Ask the replay question after final results and leave its window ready for a choice. */
void fn_1_17D68(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 38), 1, 0);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 94), 1, 3);
    }
    MicQuizWinMesWait();
}

/* Show the closing line after a replay choice and pause before leaving the results. */
void fn_1_17F64(void) {
    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(2);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 39), 1, 2);
    } else {
        MicQuizWinOpen(3);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 95), 1, 3);
    }
    MicQuizWinMesWait();
    HuPrcSleep(120);
}

/* Present the alternate final-results reactions for every active player. */
void fn_1_18168(void)
{
    s32 playerNo;

    if (micQuizModeWork->presenterLayout == 0) {
        MicQuizWinOpen(1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 53), 1, 1);
        MicQuizWinMesWait();
        MicQuizWinOpen(2);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 54), 1, 2);
        MicQuizWinMesWait();
        for (playerNo = 0; playerNo < 4; playerNo++) {
            if (micQuizModeWork->playerActive[playerNo] != 0) {
                fn_1_A678(playerNo, 2, 8.0f, 0);
            }
        }
        return;
    }

    MicQuizWinOpen(3);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 113), 1, 3);
    MicQuizWinMesWait();
    HuPrcSleep(120);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (micQuizModeWork->playerActive[playerNo] != 0) {
            fn_80026A44(lbl_1_bss_1F0[playerNo], (1 << 0));
            HuAudFXPlay(MSM_SE_MICQUIZ_PLAYER_CELEBRATE);
            fn_1_A678(playerNo, 3, 8.0f, 0);
        }
    }
    HuPrcSleep(93);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (micQuizModeWork->playerActive[playerNo] != 0) {
            fn_800269E8(lbl_1_bss_1F0[playerNo], (1 << 0));
            fn_1_A678(playerNo, 6, 8.0f, ((1 << 30) | (1 << 0)));
        }
    }
}

/* Show the single-presenter introduction before naming players at the lowest finishing rank. */
void fn_1_186E8(void) {
    MicQuizWinOpen(3);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 109), 1, 3);
    MicQuizWinMesWait();
}

/* List the active players sharing the lowest finishing rank in the single-presenter results. */
void fn_1_18800(void) {
    s32 playerIndex;
    s32 lastPlaceRank;
    s32 lastPlacePlayerCount;
    s32 resultCount;
    s32 lastPlacePlayers[4];
    u32 lastPlaceCharacterMessages[4];
    u32 messageId;

    lastPlaceRank = fn_1_6478();

    for (playerIndex = 0, lastPlacePlayerCount = 0; playerIndex < 4; playerIndex++) {
        if (micQuizModeWork->playerActive[playerIndex] != 0
            && micQuizModeWork->playerRank[playerIndex] == lastPlaceRank) {
            lastPlacePlayers[lastPlacePlayerCount] = playerIndex;
            lastPlaceCharacterMessages[lastPlacePlayerCount] =
                micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerIndex]];
            lastPlacePlayerCount++;
        }
    }

    resultCount = lastPlacePlayerCount;
    if (resultCount == 1) {
        messageId = MESSNUM(MESS_MICQUIZ, 110);
        MicQuizWinInsertMesSet(3, lastPlaceCharacterMessages[0], 0);
    } else if (resultCount == 2) {
        messageId = MESSNUM(MESS_MICQUIZ, 111);
        MicQuizWinInsertMesSet(3, lastPlaceCharacterMessages[0], 0);
        MicQuizWinInsertMesSet(3, lastPlaceCharacterMessages[1], 1);
    } else if (resultCount == 3) {
        messageId = MESSNUM(MESS_MICQUIZ, 112);
        MicQuizWinInsertMesSet(3, lastPlaceCharacterMessages[0], 0);
        MicQuizWinInsertMesSet(3, lastPlaceCharacterMessages[1], 1);
        MicQuizWinInsertMesSet(3, lastPlaceCharacterMessages[2], 2);
    }
    MicQuizWinOpen(3);
    MicQuizWinMesSet(messageId, 1, 3);
    MicQuizWinMesWait();
}

/* Show the single-presenter explanation before the response retry or follow-up. */
void fn_1_18B50(void) {
    MicQuizWinOpen(3);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_I, 9), 1, 3);
    MicQuizWinMesWait();
}

/* Show the single-presenter response line and wait before continuing. */
void fn_1_18C68(void) {
    MicQuizWinOpen(3);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 102), 1, 3);
    MicQuizWinMesWait();
}

/* Show the single-presenter round follow-up and wait before continuing. */
void fn_1_18D80(void) {
    MicQuizWinOpen(3);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 114), 1, 3);
    MicQuizWinMesWait();
}

/* Name the selected character in the single-presenter comparison dialogue. */
void fn_1_18E98(s32 messageIndex) {
    s32 messageId;

    MicQuizWinOpen(3);
    messageId = micQuizCharaNameMes[messageIndex];
    MicQuizWinInsertMesSet(3, messageId, 0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_I, 21), 1, 3);
    MicQuizWinMesWait();
}

/* Name the selected character in the alternate single-presenter comparison line. */
void fn_1_18FEC(s32 messageIndex) {
    s32 messageId;

    MicQuizWinOpen(3);
    messageId = micQuizCharaNameMes[messageIndex];
    MicQuizWinInsertMesSet(3, messageId, 0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_I, 22), 1, 3);
    MicQuizWinMesWait();
}

/* Show the single-presenter result line for the supplied character message. */
void fn_1_19140(s32 messageIndex) {
    s32 messageId;

    MicQuizWinOpen(3);
    messageId = micQuizCharaNameMes[messageIndex];
    MicQuizWinInsertMesSet(3, messageId, 0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_I, 10), 1, 3);
    MicQuizWinMesWait();
}

/* Play the timed single-presenter explanation, ending with the named player. */
void fn_1_19294(s32 playerIndex) {
    MicQuizWinOpen(3);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_I, 23), 1, 3);
    MicQuizWinMesWait();
    HuPrcSleep(120);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_I, 29), 1, 3);
    MicQuizWinMesWait();
    HuPrcSleep(120);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_I, 30), 1, 3);
    MicQuizWinMesWait();
    HuPrcSleep(120);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_I, 31), 1, 3);
    MicQuizWinMesWait();
    HuPrcSleep(120);
    MicQuizWinInsertMesSet(3, micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerIndex]],
                           0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_I, 25), 1, 3);
    MicQuizWinMesWait();
}

/* Show the single-presenter closing explanation and pause for 120 frames. */
void fn_1_1959C(void) {
    MicQuizWinOpen(3);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_I, 32), 1, 3);
    MicQuizWinMesWait();
    HuPrcSleep(120);
}

/* Build a list of the other active players, ask for a choice, and restore window input masks. */
s16 fn_1_196BC(s32 playerNo)
{
    s32 currentPlayer;
    s32 activeCount;
    s32 playerCount;
    u8 activeMask;
    s16 choice;

    activeMask = 0;
    playerCount = micQuizModeWork->playerCount;
    for (currentPlayer = 0; currentPlayer < 4; currentPlayer++) {
        lbl_1_bss_9C[currentPlayer] = -1;
        lbl_1_bss_90[currentPlayer] = -1;
    }
    for (currentPlayer = 0, activeCount = 0; currentPlayer < 4; currentPlayer++) {
        if (currentPlayer != playerNo && micQuizModeWork->playerActive[currentPlayer] != 0) {
            lbl_1_bss_9C[activeCount] = currentPlayer;
            lbl_1_bss_90[activeCount] = micQuizModeWork->playerCharacter[currentPlayer];
            activeMask |= 1 << currentPlayer;
            activeCount++;
        }
    }

    MicQuizWinOpen(3);
    if (playerCount == 2) {
        MicQuizWinInsertMesSet(3, micQuizCharaNameMes[lbl_1_bss_90[0]], 0);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_I, 26), 1, 3);
    } else if (playerCount == 3) {
        MicQuizWinInsertMesSet(3, micQuizCharaNameMes[lbl_1_bss_90[0]], 0);
        MicQuizWinInsertMesSet(3, micQuizCharaNameMes[lbl_1_bss_90[1]], 1);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_I, 27), 1, 3);
    } else if (playerCount == 4) {
        MicQuizWinInsertMesSet(3, micQuizCharaNameMes[lbl_1_bss_90[0]], 0);
        MicQuizWinInsertMesSet(3, micQuizCharaNameMes[lbl_1_bss_90[1]], 1);
        MicQuizWinInsertMesSet(3, micQuizCharaNameMes[lbl_1_bss_90[2]], 2);
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_I, 28), 1, 3);
    }
    MicQuizWinMesWait();
    HuWinDisablePlayerSet(micQuizDialogWin3, activeMask);
    choice = MicQuizWinChoiceGet(-1);

    MicQuizWinPlayerMaskSet();
    return choice;
}

/* Insert a player's character name into the current single-presenter result message. */
void fn_1_19C84(s32 playerNo) {
    s32 messageId;

    messageId = micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerNo]];
    MicQuizWinInsertMesSet(3, messageId, 0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_I, 10), 1, 3);
    MicQuizWinMesWait();
}

/* Advance the camera surface model's motion for a short randomized interval. */
void fn_1_19D5C(void) {
    fn_8002C95C(lbl_1_bss_23E, lbl_1_bss_1DC);
    fn_80026A44(lbl_1_bss_23E, ((1 << 30) | (1 << 1)));
    HuPrcSleep(frandmod(10) + 5);
    fn_800269E8(lbl_1_bss_23E, ((1 << 30) | (1 << 1)));
}

/* Switch camera capture textures and viewports, optionally showing the transition model. */
void fn_1_19DD4(s32 cameraIndex, s32 firstCapture, s32 showTransition) {
    switch (cameraIndex) {
    case 0:
        if (firstCapture == 0) {
            fn_80031010(lbl_1_bss_242);
            fn_800258A8(4, 0);
        }
        lbl_1_bss_242 = fn_80030B2C(lbl_1_bss_30, lbl_1_bss_23E, "vq_test");
        fn_8002583C(4, 0, fn_1_3FE4);
        fn_1_1A210();
        if (showTransition != 0) {
            fn_80026A44(lbl_1_bss_24C, (1 << 0));
            HuPrcSleep(10);
            fn_800269E8(lbl_1_bss_24C, (1 << 0));
        }
        fn_1_1A270();
        Hu3DCameraPerspectiveSet(1, 20.0f, 10.0f, 20000.0f, 1.2f);
        Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
        Hu3DCameraScissorSet(1, 0U, 0U, 640U, 480U);
        Hu3DCameraPerspectiveSet(2, 20.0f, 10.0f, 20000.0f, 1.2f);
        Hu3DCameraViewportSet(2, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        Hu3DCameraScissorSet(2, 0U, 0U, 0U, 0U);
        break;
    case 1:
        if (firstCapture == 0) {
            fn_80031010(lbl_1_bss_240);
            fn_800258A8(4, 0);
        }
        lbl_1_bss_240 = fn_80030B2C(lbl_1_bss_34, lbl_1_bss_23E, "vq_test");
        fn_8002583C(4, 0, fn_1_4030);
        Hu3DCameraPerspectiveSet(1, 20.0f, 10.0f, 20000.0f, 1.2f);
        Hu3DCameraViewportSet(1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        Hu3DCameraScissorSet(1, 0U, 0U, 0U, 0U);
        Hu3DCameraPerspectiveSet(2, 20.0f, 10.0f, 20000.0f, 1.2f);
        Hu3DCameraViewportSet(2, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
        Hu3DCameraScissorSet(2, 0U, 0U, 640U, 480U);
        fn_1_1A210();
        if (showTransition != 0) {
            fn_80026A44(lbl_1_bss_24C, (1 << 0));
            HuPrcSleep(10);
            fn_800269E8(lbl_1_bss_24C, (1 << 0));
        }
        fn_1_1A270();
        break;
    }
    lbl_1_bss_B2 = cameraIndex;
}

/* Hide the layout-specific camera surface model while capture views change. */
void fn_1_1A210(void) {
    if (micQuizModeWork->presenterLayout != 0) {
        fn_800269E8(lbl_1_bss_252, (1 << 0));
        return;
    }
    fn_800269E8(lbl_1_bss_254, (1 << 0));
}

/* Show the layout-specific camera surface model after capture views change. */
void fn_1_1A270(void) {
    if (micQuizModeWork->presenterLayout != 0) {
        fn_80026A44(lbl_1_bss_252, (1 << 0));
        return;
    }
    fn_80026A44(lbl_1_bss_254, (1 << 0));
}

/* Create the paired particle systems used by the player event-panel effect. */
s32 fn_1_1A2D0(s16 cameraMask, s32 hookType) {
    HU3D_PARTICLE *work;
    s32 index;
    s16 particleId;

    lbl_1_bss_AC = fn_8003216C(lbl_1_bss_58, 100);
    particleId = lbl_1_bss_AC;
    work = lbl_802C01C8[particleId].hookData;
    fn_8003269C(particleId, 1);
    fn_80026B8C(particleId, 6);
    fn_80026760(particleId, 0.0f, 0.0f, 0.0f);
    fn_80032704(particleId, ((1 << 2) | (1 << 5) | (1 << 6)));
    if (hookType == 0) {
        fn_800326D0(particleId, fn_1_1A968);
    } else {
        fn_800326D0(particleId, fn_1_1AC20);
    }
    for (index = 0; index < work->maxCnt; index++) {
        memset(&work->data[index], 0, sizeof(work->data[index]));
        work->data[index].time = -1;
        work->data[index].cameraBit = cameraMask;
    }

    lbl_1_bss_AA = fn_8003216C(lbl_1_bss_54, 100);
    particleId = lbl_1_bss_AA;
    work = lbl_802C01C8[particleId].hookData;
    fn_8003269C(particleId, 1);
    fn_80026B8C(particleId, 6);
    fn_80026760(particleId, 0.0f, 0.0f, 0.0f);
    fn_80032704(particleId, ((1 << 2) | (1 << 5) | (1 << 6)));
    fn_800326D0(particleId, fn_1_1A968);
    for (index = 0; index < work->maxCnt; index++) {
        memset(&work->data[index], 0, sizeof(work->data[index]));
        work->data[index].time = -1;
        work->data[index].cameraBit = (1 << 2);
    }
}

/* Create a selected particle system and initialize its entries before an effect burst. */
s32 fn_1_1A524(s16 cameraMask, s32 hookType, s32 particleType) {
    HU3D_PARTICLE *work;
    HU3D_PARTICLE_DATA *entries;
    s32 index;

    if (particleType == 0) {
        lbl_1_bss_A8 = fn_8003216C(lbl_1_bss_58, 100);
        fn_8003269C(lbl_1_bss_A8, 1);
    } else {
        lbl_1_bss_A8 = fn_8003216C(lbl_1_bss_54, 100);
        fn_8003269C(lbl_1_bss_A8, 1);
    }

    work = lbl_802C01C8[lbl_1_bss_A8].hookData;
    fn_80026B8C(lbl_1_bss_A8, 6);
    fn_80026760(lbl_1_bss_A8, 0.0f, 0.0f, 0.0f);
    fn_80032704(lbl_1_bss_A8, ((1 << 2) | (1 << 5) | (1 << 6)));
    if (hookType == 0) {
        fn_800326D0(lbl_1_bss_A8, fn_1_1A968);
    } else {
        fn_800326D0(lbl_1_bss_A8, fn_1_1AC20);
    }

    for (index = 0; index < work->maxCnt; index++) {
        memset(&work->data[index], 0, sizeof(work->data[index]));
        work->data[index].time = -1;
        work->data[index].cameraBit = cameraMask;
    }
}

/* Release both particle systems after their event-panel presentation is finished. */
void fn_1_1A6E8(void) {
    fn_800262D0(lbl_1_bss_AC);
    fn_800262D0(lbl_1_bss_AA);
}

/* Place and orient one paired particle system, activating all of its inactive entries. */
void fn_1_1A724(f32 positionX, f32 positionY, f32 positionZ, s32 modelMode, f32 rotationY) {
    HU3D_PARTICLE_DATA *entry;
    s16 modelId;
    HU3D_PARTICLE *hook;
    s32 index;
    HuVecF vector;

    if (modelMode == 0) {
        modelId = lbl_1_bss_AC;
    } else {
        modelId = lbl_1_bss_AA;
    }
    fn_80026808(modelId, 0.0f, rotationY, 0.0f);
    fn_80026760(modelId, positionX, positionY, positionZ);
    hook = lbl_802C01C8[modelId].hookData;
    entry = hook->data;
    vector.x = 0.0f;
    vector.y = 0.0f;
    vector.z = 0.0f;
    index = 0;
    while (index < hook->maxCnt) {
        if (entry->time < 0) {
            entry->time = 0;
        }
        index++;
        entry++;
    }
}

/* Place and orient the single particle system, activating all of its inactive entries. */
void fn_1_1A854(f32 positionX, f32 positionY, f32 positionZ, f32 rotationY) {
    HU3D_PARTICLE *hook;
    HU3D_PARTICLE_DATA *entry;
    HuVecF vector;
    s32 index;

    fn_80026808(lbl_1_bss_A8, 0.0f, rotationY, 0.0f);
    fn_80026760(lbl_1_bss_A8, positionX, positionY, positionZ);
    hook = lbl_802C01C8[lbl_1_bss_A8].hookData;
    entry = hook->data;
    vector.x = 0.0f;
    vector.y = 0.0f;
    vector.z = 0.0f;
    index = 0;
    while (index < hook->maxCnt) {
        if (entry->time < 0) {
            entry->time = 0;
        }
        index++;
        entry++;
    }
}

/* Update an outward particle burst each draw, fading active particles until they become
 * inactive. */
void fn_1_1A968(HU3D_MODEL *model, HU3D_PARTICLE *hook, Mtx matrix) {
    HU3D_PARTICLE_DATA *entry;
    s32 index;
    s32 randomValue;
    s32 timer;
    f32 angle;
    f32 radius;

    entry = hook->data;
    index = 0;
    while (index < hook->maxCnt) {
        if (entry->time == 0) {
            angle = 2.0f * frandf();
            radius = frandf();
            /* This random sample's value is unused. */
            randomValue = rand8() % 14;
            entry->time++;
            entry->parManId = rand8() % 32;
            entry->vel.x = (f32)(sin(M_PI * (f64)angle) * radius * 3.0);
            entry->vel.y = (f32)(cos(M_PI * (f64)angle) * radius * 3.0);
            entry->vel.z = 0.0f;
            entry->pos.x += sin(M_PI * (f64)angle) * radius * 20.0;
            entry->pos.y += cos(M_PI * (f64)angle) * radius * 20.0;
            entry->color.r = 255;
            entry->color.g = 255;
            entry->color.b = 255;
            entry->color.a = 255;
            entry->scale = (f32)(rand8() % 20);
        }
        if (entry->time == 1) {
            timer = entry->color.a - 10;
            entry->pos.y += entry->vel.y;
            entry->parManId++;
            entry->pos.x += entry->vel.x;
            if (timer <= 0) {
                timer = 0;
                entry->scale = 0.0f;
                entry->time = -1;
            }
            entry->color.a = (u8)timer;
        }
        index++;
        entry++;
    }
}

/* Update the alternate particle burst each draw, moving particles inward while they fade. */
void fn_1_1AC20(HU3D_MODEL *model, HU3D_PARTICLE *particleData, Mtx matrix) {
    HU3D_PARTICLE_DATA *particle;
    s32 index;
    f32 angle;
    f32 magnitude;
    s32 randomValue;
    s32 alpha;

    particle = particleData->data;
    index = 0;
    while (index < particleData->maxCnt) {
        if (particle->time == 0) {
            angle = 2.0f * frandf();
            magnitude = frandf();
            /* This random sample's value is unused. */
            randomValue = rand8() % 14;
            particle->time += 1;
            particle->parManId = rand8() % 32;
            particle->vel.x = (f32)(3.0 * ((f64)magnitude *
                sin(M_PI * (f64)angle)));
            particle->vel.y = (f32)(3.0 * ((f64)magnitude *
                cos(M_PI * (f64)angle)));
            particle->vel.z = 0.0f;
            particle->pos.x = (f32)((f64)particle->pos.x +
                80.0 * ((f64)magnitude * sin(M_PI * (f64)angle)));
            particle->pos.y = (f32)((f64)particle->pos.y +
                80.0 * ((f64)magnitude * cos(M_PI * (f64)angle)));
            particle->color.r = 255;
            particle->color.g = 255;
            particle->color.b = 255;
            particle->color.a = 255;
            particle->scale = rand8() % 20;
        }
        if (particle->time == 1) {
            alpha = particle->color.a - 8;
            particle->pos.y -= particle->vel.y;
            particle->parManId += 1;
            particle->pos.x -= particle->vel.x;
            if (alpha <= 0) {
                alpha = 0;
                particle->scale = 0.0f;
                particle->time = -1;
            }
            particle->color.a = alpha;
        }
        index += 1;
        particle += 1;
    }
}

/* Start fading the full-screen overlay out over at least one frame. */
void fn_1_1AED8(s16 requestedDuration) {
    QuizOverlayWork *timing;
    f32 durationFloat;

    if (lbl_1_bss_28 != 0) {
        if (requestedDuration <= 0) {
            requestedDuration = 1;
        }
        timing = ((QuizOverlayWork *)(&(lbl_1_bss_28->work[0])));
        timing->duration = requestedDuration;
        OSs16tof32(&requestedDuration, &durationFloat);
        timing->rate = (f32)(-(s32)timing->color.a) / durationFloat;
        timing->flags.stopUpdating = 0;
        timing->remainingFrames = timing->duration;
    }
}

/* Create a full-screen color overlay that fades toward the requested opacity. */
void fn_1_1AFC4(s16 duration, u8 targetAlpha) {
    QuizOverlayWork *work;

    if (lbl_1_bss_28 != 0) {
        work = ((QuizOverlayWork *)(&(lbl_1_bss_28->work[0])));
        work->flags.removeObject = 1;
        while (lbl_1_bss_28 != 0) {
            HuPrcVSleep();
        }
    }
    lbl_1_bss_28 = fn_8003AB38(lbl_1_bss_288, 101, 0, 0, -1, fn_1_1B28C);
    omSetStatBit(lbl_1_bss_28, ((1 << 5) | (1 << 7)));
    if (duration <= 0) {
        duration = 1;
    }

    work = ((QuizOverlayWork *)(&(lbl_1_bss_28->work[0])));
    work->flags.removeObject = 0;
    work->flags.stopUpdating = 0;
    work->color.r = 0;
    work->color.g = 0;
    work->color.b = 0;
    work->color.a = 0;
    work->targetAlpha = targetAlpha;
    work->rate = (f32)(targetAlpha - work->color.a) / (f32)duration;
    work->remainingFrames = duration;
    work->duration = duration;
    work->cameraModel = fn_800260F4(fn_1_1B3EC);
    fn_80026B64(work->cameraModel, 4);
    fn_80026B8C(work->cameraModel, 1);
}

/* Report whether the full-screen overlay has reached its held opacity or has been removed. */
s32 fn_1_1B18C(void) {
    s32 active;
    QuizOverlayWork *timing;

    if ((u32)lbl_1_bss_28 == 0) {
        return 1;
    }
    timing = ((QuizOverlayWork *)(&(lbl_1_bss_28->work[0])));
    if (timing->flags.stopUpdating != 0) {
        active = 1;
    } else {
        active = 0;
    }
    return active;
}

/* Choose which cameras render the active full-screen overlay model. */
void fn_1_1B1F8(u16 cameraMask) {
    QuizOverlayWork *hookWork;
    if (lbl_1_bss_28 != 0) {
        hookWork = ((QuizOverlayWork *)(&(lbl_1_bss_28->work[0])));
        fn_80026B64(hookWork->cameraModel, cameraMask);
    }
}

/* Report when the full-screen overlay object has been removed. */
s32 fn_1_1B250(void) {
    s32 result;

    if (lbl_1_bss_28 != NULL) {
        result = 0;
    } else {
        result = 1;
    }
    return result;
}

/* Advance the full-screen overlay opacity each object update, holding or deleting it at the end. */
void fn_1_1B28C(OMOBJ *object)
{
    QuizOverlayWork *motion;
    f32 alphaFloat;

    motion = ((QuizOverlayWork *)(&(object->work[0])));
    if (motion->flags.removeObject) {
        if (motion->cameraModel != -1) {
            fn_800262D0(motion->cameraModel);
        }
        lbl_1_bss_28 = 0;
        omDelObjEx(fn_8000DD9C(), object);
    } else if (!motion->flags.stopUpdating) {
        /* Advance the screen overlay opacity by its per-frame fade rate. */
        alphaFloat = __OSu8tof32(&motion->color.a);
        alphaFloat += motion->rate;
        /* Keep the current opacity in the byte used by the overlay draw hook. */
        motion->color.a = __OSf32tou8(alphaFloat);
        if (motion->remainingFrames > 0) {
            motion->remainingFrames--;
        } else if (motion->rate > 0.0f) {
            motion->flags.stopUpdating = 1;
            motion->color.a = motion->targetAlpha;
        } else {
            motion->flags.removeObject = 1;
        }
    }
}

GXColor lbl_1_data_DF7 = { 255, 255, 255, 255 };

/* Draw the active full-screen color overlay as a blended rectangle during its model draw hook. */
void fn_1_1B3EC(HU3D_MODEL *model, Mtx *matrix)
{
    QuizOverlayWork *timing;
    Mtx44 projection;
    Mtx identity;
    f32 left;
    f32 right;
    f32 top;
    f32 bottom;

    if (lbl_1_bss_28 != 0) {
        timing = ((QuizOverlayWork *)(&(lbl_1_bss_28->work[0])));
        left = 0.0f;
        right = 640.0f;
        top = 0.0f;
        bottom = 480.0f;
        C_MTXOrtho(projection, top, bottom, left, right,
                   0.0f, 10.0f);
        GXSetProjection(projection, GX_ORTHOGRAPHIC);
        PSMTXIdentity(identity);
        GXLoadPosMtxImm(identity, GX_PNMTX0);
        GXSetCurrentMtx(GX_PNMTX0);
        GXSetViewport(0.0f, 0.0f, right,
                      1.0f + bottom,
                      0.0f, 1.0f);
        GXSetScissor(0, 0, (u32)right, (u32)(1.0f + bottom));
        GXClearVtxDesc();
        GXSetChanMatColor(GX_COLOR0A0, timing->color);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG,
                      GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_REG,
                      GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
        GXSetNumTexGens(0);
        GXSetNumTevStages(1);
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_S, GX_RGBX8, 0);
        GXSetZMode(GX_ENABLE, GX_ALWAYS, GX_DISABLE);
        GXSetAlphaUpdate(GX_DISABLE);
        GXSetColorUpdate(GX_ENABLE);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition2u16((u16)left, (u16)top);
        GXPosition2u16((u16)right, (u16)top);
        GXPosition2u16((u16)right, (u16)bottom);
        GXPosition2u16((u16)left, (u16)bottom);
        GXSetChanMatColor(GX_COLOR0A0, lbl_1_data_DF7);
    }
}

/* Screen positions for the tutorial pointer demonstrations. */
f32 lbl_1_data_DFC[5][2] = {
    252.0f, 99.0f, 252.0f, 144.0f, 252.0f, 189.0f, 252.0f, 234.0f, 252.0f, 279.0f
};
MicquizPositionData lbl_1_data_E24[5] = {
    279.0f, 108.0f, 330.0f, 108.0f, 379.0f, 108.0f, 428.0f, 108.0f, 477.0f, 108.0f
};
f32 lbl_1_data_E4C[] = { 327.0f, 97.0f, 372.0f, 97.0f, 327.0f, 142.0f, 0.0f };

/* Animate the tutorial pointer each frame with the selected vertical or diagonal bounce. */
void fn_1_1B7B4(void) {
    f32 sineOffset;
    f32 angle;

    angle = 0.0f;
    for (;;) {
        HuPrcVSleep();
        sineOffset = (f32) (5.0 * sin((M_PI * (f64) angle) / 180.0));
        if (lbl_1_bss_30C == 0) {
            HuSprBankSet(lbl_1_bss_310, 0, 0);
            HuSprPosSet(lbl_1_bss_310, 0, lbl_1_bss_304.x, lbl_1_bss_304.y - sineOffset);
        } else if (lbl_1_bss_30C == 1) {
            HuSprBankSet(lbl_1_bss_310, 0, 1);
            HuSprPosSet(lbl_1_bss_310, 0, lbl_1_bss_304.x, lbl_1_bss_304.y + sineOffset);
        } else if (lbl_1_bss_30C == 2) {
            HuSprBankSet(lbl_1_bss_310, 0, 2);
            HuSprPosSet(lbl_1_bss_310, 0, lbl_1_bss_304.x + sineOffset,
                        lbl_1_bss_304.y - sineOffset);
        }
        angle += 18.0f;
    }
}

/* Move the tutorial pointer through five stored positions, pausing at each one. */
void fn_1_1B94C(void) {
    s32 positionIndex;

    lbl_1_bss_30C = 0;
    positionIndex = 0;
    while (positionIndex < 5) {
        lbl_1_bss_304.x = lbl_1_data_E24[positionIndex].x;
        lbl_1_bss_304.y = lbl_1_data_E24[positionIndex].y;
        HuPrcSleep(120);
        positionIndex += 1;
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Run the tutorial dialogue, camera transition, pointer demonstrations, and example panel
 * reveal. */
void fn_1_1B9D4(void) {
    fn_8000DAC0(fn_1_1BBD8, 100, 4096, 0, lbl_1_bss_288);
    MicQuizInstructionSet(7);
    fn_1_1DFDC();
    fn_8000DAC0(fn_1_1BB18, 100, 4096, 0, lbl_1_bss_288);
    fn_1_A838(lbl_1_data_534, 90, 4);
    fn_1_19DD4(0, 1, 1);
    fn_1_1D994();
    fn_1_1D580();
    fn_1_1CD58();
    fn_1_1CAE4();
    fn_1_1BE00();
}

/* Slide both presenters outward for 60 frames in the tutorial transition process. */
void fn_1_1BB18(void) {
    Point3d rightPosition;
    Point3d leftPosition;
    f32 distance;
    s32 frame;

    fn_800267CC(lbl_1_bss_2A6[0], &leftPosition);
    fn_800267CC(lbl_1_bss_2A6[1], &rightPosition);
    distance = 0.0f;
    frame = 0;
    while (frame < 60) {
        HuPrcVSleep();
        distance += 5.0f;
        fn_80026760(lbl_1_bss_2A6[0], leftPosition.x - distance, leftPosition.y,
                        leftPosition.z);
        fn_80026760(lbl_1_bss_2A6[1], rightPosition.x + distance, rightPosition.y,
                        rightPosition.z);
        frame += 1;
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Wait for controller Start during the tutorial, then return through the overlay history. */
void fn_1_1BBD8(void) {
    OMOVLHIS *history;

    history = omOvlHisGet(0);
    do {
        HuPrcVSleep();
    } while ((s32) (lbl_802C0008[0] & PAD_BUTTON_START) == 0);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 20);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 20);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    omOvlHisChg(0, DLL_micquizdll, 2, history->stat);
    fn_8003A5E4(0, 1);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Turn a tutorial panel through a full rotation, changing its event bank halfway through. */
void fn_1_1BC74(s32 levelIndex, s32 columnIndex, s32 bank)
{
    f32 rotationOffset;
    f32 angle;
    s32 bankChanged;
    s16 selectedModel;

    bankChanged = 0;
    selectedModel = lbl_1_bss_198[levelIndex].model[columnIndex];
    angle = 0.0f;
    do {
        HuPrcVSleep();
        rotationOffset = (f32)(360.0 * sin((M_PI * angle) / 180.0));
        angle += 1.5f;
        fn_80026808(lbl_1_bss_198[levelIndex].model[columnIndex], 0.0f,
                        180.0f + rotationOffset, 0.0f);
        if ((angle >= 45.0f) && (bankChanged == 0)) {
            bankChanged = 1;
            Hu3DAnimBankSet(lbl_1_bss_102[levelIndex].model[columnIndex], (u16)(bank + 1));
        }
    } while (!(angle >= 90.0f));
}

/* Alternate presenter dialogue at the end of the tutorial demonstration. */
void fn_1_1BE00(void) {
    MicQuizWinOpen(1);
    fn_1_A3B4(1, 1, 8.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 27), 1, 1);
    MicQuizWinMesWait();
    fn_1_A3B4(1, 0, 8.0f, ((1 << 30) | (1 << 0)));

    MicQuizWinOpen(0);
    fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 28), 1, 0);
    MicQuizWinMesWait();
    fn_1_A3B4(0, 0, 8.0f, ((1 << 30) | (1 << 0)));

    MicQuizWinOpen(1);
    fn_1_A3B4(1, 1, 8.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 29), 1, 1);
    MicQuizWinMesWait();
}

/* Explain three panel states in the tutorial, rotating the selected panel between lines. */
void fn_1_1C300(s32 levelIndex, s32 columnIndex)
{
    MicQuizWinOpen(1);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 23), 1, 1);
    MicQuizWinMesWait();
    fn_1_1BC74(levelIndex, columnIndex, 1);

    MicQuizWinOpen(0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 24), 1, 0);
    MicQuizWinMesWait();
    fn_1_1BC74(levelIndex, columnIndex, 2);

    MicQuizWinOpen(1);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 25), 1, 1);
    MicQuizWinMesWait();
    fn_1_1BC74(levelIndex, columnIndex, 3);

    MicQuizWinOpen(0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 26), 1, 0);
    MicQuizWinMesWait();
}

/* Point at the tutorial example cell and demonstrate its event reveal. */
void fn_1_1CAE4(void) {
    HuSprAttrReset(lbl_1_bss_310, 0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_304.x = lbl_1_data_E4C[4];
    lbl_1_bss_304.y = lbl_1_data_E4C[5];
    MicQuizWinOpen(0);
    fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 22), 1, 0);
    MicQuizWinMesWait();
    fn_1_A3B4(0, 0, 8.0f, ((1 << 30) | (1 << 0)));
    HuSprAttrSet(lbl_1_bss_310, 0, HUSPR_ATTR_DISPOFF);
    fn_1_CE28(0, 1, 0, 1);
    HuSprAttrSet(lbl_1_bss_310, 0, HUSPR_ATTR_DISPOFF);
}

/* Move the tutorial pointer between example cells while the presenters explain their behavior. */
void fn_1_1CD58(void) {
    HuSprAttrReset(lbl_1_bss_310, 0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_30C = 2;
    lbl_1_bss_304.x = lbl_1_data_E4C[0];
    lbl_1_bss_304.y = lbl_1_data_E4C[1];

    MicQuizWinOpen(0);
    fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 18), 1, 0);
    MicQuizWinMesWait();
    fn_1_A3B4(0, 0, 8.0f, ((1 << 30) | (1 << 0)));

    lbl_1_bss_304.x = lbl_1_data_E4C[2];
    lbl_1_bss_304.y = lbl_1_data_E4C[3];
    MicQuizWinOpen(1);
    fn_1_A3B4(1, 1, 8.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 19), 1, 1);
    MicQuizWinMesWait();
    fn_1_A3B4(1, 0, 8.0f, ((1 << 30) | (1 << 0)));

    lbl_1_bss_304.x = lbl_1_data_E4C[0];
    lbl_1_bss_304.y = lbl_1_data_E4C[1];
    MicQuizWinOpen(0);
    fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 20), 1, 0);
    MicQuizWinMesWait();
    fn_1_A3B4(0, 0, 8.0f, ((1 << 30) | (1 << 0)));

    fn_1_CE28(0, 0, 8, 0);
    MicQuizWinOpen(1);
    fn_1_A3B4(1, 1, 8.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 21), 1, 1);
    MicQuizWinMesWait();
    fn_1_A3B4(1, 0, 8.0f, ((1 << 30) | (1 << 0)));
    HuSprAttrSet(lbl_1_bss_310, 0, HUSPR_ATTR_DISPOFF);
}

/* Run the pointer-position demonstration while the two presenters explain the grid. */
void fn_1_1D580(void) {
    lbl_1_bss_70 = fn_8000DAC0(fn_1_1B94C, 100, 4096, 0, lbl_1_bss_288);
    HuSprAttrReset(lbl_1_bss_310, 0, HUSPR_ATTR_DISPOFF);

    MicQuizWinOpen(0);
    fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 16), 1, 0);
    MicQuizWinMesWait();
    fn_1_A3B4(0, 0, 8.0f, ((1 << 30) | (1 << 0)));

    MicQuizWinOpen(1);
    fn_1_A3B4(1, 1, 8.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 17), 1, 1);
    MicQuizWinMesWait();
    fn_1_A3B4(1, 0, 8.0f, ((1 << 30) | (1 << 0)));

    HuPrcKill(lbl_1_bss_70);
    HuSprAttrSet(lbl_1_bss_310, 0, HUSPR_ATTR_DISPOFF);
}

/* Explain the five grid rows, alternating presenters and moving the tutorial pointer. */
void fn_1_1D994(void) {
    s32 messageIndex;

    MicQuizWinOpen(0);
    fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 10), 1, 0);
    MicQuizWinMesWait();
    fn_1_A3B4(0, 0, 8.0f, ((1 << 30) | (1 << 0)));
    HuSprAttrReset(lbl_1_bss_310, 0, HUSPR_ATTR_DISPOFF);

    messageIndex = 0;
    while (messageIndex < 5) {
        lbl_1_bss_304.x = lbl_1_data_DFC[messageIndex][0];
        lbl_1_bss_304.y = lbl_1_data_DFC[messageIndex][1];
        MicQuizWinOpen(1);
        fn_1_A3B4(1, 1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 11) + messageIndex, 1, 1);
        MicQuizWinMesWait();
        fn_1_A3B4(1, 0, 8.0f, ((1 << 30) | (1 << 0)));
        messageIndex++;
        if (messageIndex >= 5) {
            break;
        }
        lbl_1_bss_304.x = lbl_1_data_DFC[messageIndex][0];
        lbl_1_bss_304.y = lbl_1_data_DFC[messageIndex][1];
        MicQuizWinOpen(0);
        fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 11) + messageIndex, 1, 0);
        MicQuizWinMesWait();
        fn_1_A3B4(0, 0, 8.0f, ((1 << 30) | (1 << 0)));
        messageIndex++;
        if (messageIndex >= 5) {
            break;
        }
    }
    HuSprAttrSet(lbl_1_bss_310, 0, HUSPR_ATTR_DISPOFF);
}

/* Play the opening tutorial dialogue, alternating the two presenters for ten lines. */
void fn_1_1DFDC(void) {
    s32 messageId;

    for (messageId = 0; messageId < 10;) {
        MicQuizWinOpen(0);
        fn_1_A3B4(0, 1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 0) + messageId, 1, 0);
        MicQuizWinMesWait();
        fn_1_A3B4(0, 0, 8.0f, ((1 << 30) | (1 << 0)));
        messageId++;

        MicQuizWinOpen(1);
        fn_1_A3B4(1, 1, 8.0f, ((1 << 30) | (1 << 0)));
        MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_TUTORIAL, 0) + messageId, 1, 1);
        MicQuizWinMesWait();
        fn_1_A3B4(1, 0, 8.0f, ((1 << 30) | (1 << 0)));
        messageId++;
    }

    fn_1_A3B4(1, 0, 8.0f, ((1 << 30) | (1 << 0)));
}

/* Configure the duration, amplitude, decay, and angular step of a camera shake. */
void fn_1_1E3F8(u16 duration, f32 scale, f32 scaleDecay, f32 angleStep) {
    lbl_1_bss_4C = (s32) duration;
    lbl_1_bss_44 = scale;
    lbl_1_bss_40 = scaleDecay;
    lbl_1_bss_38 = angleStep;
    lbl_1_bss_3C = 90.0f;
}

/* Apply the decaying vertical camera shake each scheduler frame while its timer is active. */
void fn_1_1E448(void) {
    for (;;) {
        HuPrcVSleep();
        lbl_1_bss_50 = 0.0f;
        if (lbl_1_bss_4C <= 0) {
            continue;
        }
        lbl_1_bss_50 =
            (f32) ((f64) lbl_1_bss_44 * sin((M_PI * (f64) lbl_1_bss_3C) / 180.0));
        lbl_1_bss_3C += lbl_1_bss_38;
        lbl_1_bss_44 *= lbl_1_bss_40;
        lbl_1_bss_4C -= 1;
        lbl_80266B18[2].y = lbl_1_data_5F8.center.y + lbl_1_bss_50;
    }
}

/* Choose closing standings commentary from the active players' tied ranks and score gaps. */
s32 fn_1_1E56C(void) {
    s32 playerOrder[4];
    s32 playerKinds[4];
    s32 playerValues[4];
    s32 sortedValues[4];
    s32 index;
    s32 firstSortIndex;
    s32 firstOtherIndex;
    s32 firstMaxIndex;
    s32 isLowScore;
    s32 firstMaxValue;
    s32 firstValue;
    s32 firstSwapValue;
    s32 secondSortIndex;
    s32 secondOtherIndex;
    s32 secondMaxIndex;
    s32 secondMaxValue;
    s32 savedPlayerValue;
    s32 savedPlayerKind;
    s32 savedSortedValue;
    s32 result;

    for (index = 0; index < 4; index++) {
        playerOrder[index] = micQuizModeWork->playerScore[index];
        sortedValues[index] = micQuizModeWork->playerScore[index];
    }

    for (firstSortIndex = 0; firstSortIndex < 3; firstSortIndex++) {
        firstMaxValue = playerOrder[firstSortIndex];
        firstMaxIndex = firstSortIndex;
        for (firstOtherIndex = firstSortIndex + 1; firstOtherIndex < 4; firstOtherIndex++) {
            if (playerOrder[firstOtherIndex] > firstMaxValue) {
                firstMaxValue = playerOrder[firstOtherIndex];
                firstMaxIndex = firstOtherIndex;
            }
        }
        firstSwapValue = playerOrder[firstSortIndex];
        playerOrder[firstSortIndex] = playerOrder[firstMaxIndex];
        playerOrder[firstMaxIndex] = firstSwapValue;
    }

    for (index = 0; index < 4; index++) {
        playerKinds[index] = micQuizModeWork->playerCharacter[index];
        playerValues[index] = micQuizModeWork->playerScore[index];
    }

    for (secondSortIndex = 0; secondSortIndex < 3; secondSortIndex++) {
        secondMaxValue = sortedValues[secondSortIndex];
        secondMaxIndex = secondSortIndex;
        for (secondOtherIndex = secondSortIndex + 1; secondOtherIndex < 4; secondOtherIndex++) {
            if (sortedValues[secondOtherIndex] > secondMaxValue) {
                secondMaxValue = sortedValues[secondOtherIndex];
                secondMaxIndex = secondOtherIndex;
            }
        }
        savedSortedValue = sortedValues[secondSortIndex];
        savedPlayerKind = playerKinds[secondSortIndex];
        savedPlayerValue = playerValues[secondSortIndex];
        sortedValues[secondSortIndex] = sortedValues[secondMaxIndex];
        playerKinds[secondSortIndex] = playerKinds[secondMaxIndex];
        playerValues[secondSortIndex] = playerValues[secondMaxIndex];
        sortedValues[secondMaxIndex] = savedSortedValue;
        playerKinds[secondMaxIndex] = savedPlayerKind;
        playerValues[secondMaxIndex] = savedPlayerValue;
    }

    if (playerValues[0] <= 19) {
        isLowScore = 1;
    } else {
        isLowScore = 0;
    }
    if (isLowScore != 0) {
        if (fn_1_632C() != 0) {
            fn_1_1EC38();
        } else {
            fn_1_1EE70();
        }
    } else {
        if (fn_1_1EA90() != 0) {
        fn_1_1FE98();
        return;
    }

    firstValue = fn_1_1EAFC();
    switch (firstValue) {
    case 1:
        result = fn_1_1EB70();
        if (result >= 150) {
            fn_1_1F0A8(playerKinds[0]);
        } else if (result >= 100) {
            fn_1_1F318(playerKinds[0]);
        } else if (result >= 50) {
            fn_1_1F4A0(playerKinds[0]);
        } else if (result >= 20) {
            fn_1_1F628(playerKinds[0]);
        } else {
            fn_1_1F7B0(result);
        }
        break;
    case 2:
        fn_1_1F938(playerKinds[0], playerKinds[1]);
        break;
    case 3:
        if (micQuizModeWork->playerCount == 4) {
            fn_1_1FC10(playerKinds[3]);
        }
        break;
        }
    }
}

/* Show and close the single-presenter standings summary. */
s32 fn_1_1E944(void)
{
    MicQuizWinOpen(3);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ, 69), 1, 3);
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Test whether every active player has rank one for the closing standings commentary. */
s32 fn_1_1EA90(void)
{
    s32 index;

    for (index = 0; index < 4; index++) {
        if (micQuizModeWork->playerActive[index] != 0
            && micQuizModeWork->playerRank[index] != 1) {
            return 0;
        }
    }
    return 1;
}

/* Count active players at rank one for the closing standings commentary. */
s32 fn_1_1EAFC(void) {
    s32 count;
    s32 index;

    count = 0;
    for (index = 0; index < 4; index++) {
        if (micQuizModeWork->playerActive[index] != 0
            && micQuizModeWork->playerRank[index] == 1) {
            count++;
        }
    }
    return count;
}

/* Compute the score gap between players at ranks one and two for closing commentary. */
s32 fn_1_1EB70(void) {
    s32 firstValue;
    s32 secondValue;
    s32 index;

    firstValue = 0;
    secondValue = 0;
    for (index = 0; index < 4; index++) {
        if (micQuizModeWork->playerActive[index] != 0) {
            if (micQuizModeWork->playerRank[index] == 1) {
                firstValue = micQuizModeWork->playerScore[index];
            }
            if (micQuizModeWork->playerRank[index] == 2) {
                secondValue = micQuizModeWork->playerScore[index];
            }
        }
    }
    return firstValue - secondValue;
}

/* Play the two-presenter closing exchange used for the selected standings case. */
void fn_1_1EC38(void) {
    MicQuizWinOpen(1);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_MORIAGE, 14), 1, 1);
    MicQuizWinMesWait();
    MicQuizWinOpen(0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_MORIAGE, 15), 1, 0);
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Play the alternate two-presenter closing exchange for the selected standings case. */
void fn_1_1EE70(void) {
    MicQuizWinOpen(1);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_MORIAGE, 12), 1, 1);
    MicQuizWinMesWait();
    MicQuizWinOpen(0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_MORIAGE, 13), 1, 0);
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Name the leading character in the closing standings exchange, then close the dialogue. */
void fn_1_1F0A8(s32 messageIndex) {
    MicQuizWinOpen(1);
    MicQuizWinInsertMesSet(1, micQuizCharaNameMes[messageIndex], 0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_MORIAGE, 0), 1, 1);
    MicQuizWinMesWait();
    MicQuizWinOpen(0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_MORIAGE, 1), 1, 0);
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Name the leading character in the closing line used for a score gap from 100 through 149. */
void fn_1_1F318(s32 messageIndex) {
    MicQuizWinOpen(1);
    MicQuizWinInsertMesSet(1, micQuizCharaNameMes[messageIndex], 0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_MORIAGE, 2), 1, 1);
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Name the leading character in the closing line used for a score gap of at least 50. */
void fn_1_1F4A0(s32 messageIndex) {
    MicQuizWinOpen(1);
    MicQuizWinInsertMesSet(1, micQuizCharaNameMes[messageIndex], 0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_MORIAGE, 3), 1, 1);
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Name the leading character in the closing line used for a score gap of at least 20. */
void fn_1_1F628(s32 messageIndex) {
    MicQuizWinOpen(1);
    MicQuizWinInsertMesSet(1, micQuizCharaNameMes[messageIndex], 0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_MORIAGE, 4), 1, 1);
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Insert the remaining score gap as a number in the closing standings line. */
void fn_1_1F7B0(s32 value) {
    char number[16];

    MicQuizWinOpen(1);
    sprintf(number, "%d", value);
    MicQuizWinInsertMesSet(1, (u32)number, 0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_MORIAGE, 5), 1, 1);
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Play the two-leader closing exchange. The caller supplies character IDs, but this function
 * uses those values as playerCharacter indices before looking up the character names. */
void fn_1_1F938(s32 firstPlayer, s32 secondPlayer) {
    u32 firstMessageId;
    u32 secondMessageId;

    MicQuizWinOpen(1);
    firstMessageId = micQuizCharaNameMes[micQuizModeWork->playerCharacter[firstPlayer]];
    MicQuizWinInsertMesSet(1, firstMessageId, 0);
    secondMessageId = micQuizCharaNameMes[micQuizModeWork->playerCharacter[secondPlayer]];
    MicQuizWinInsertMesSet(1, secondMessageId, 1);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_MORIAGE, 6), 1, 1);
    MicQuizWinMesWait();
    MicQuizWinOpen(0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_MORIAGE, 7), 1, 0);
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Play the three-way-tie closing exchange. The caller supplies the remaining character's ID,
 * but this function uses it as a playerCharacter index before looking up the name. */
void fn_1_1FC10(s32 playerIndex) {
    MicQuizWinOpen(1);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_MORIAGE, 8), 1, 1);
    MicQuizWinMesWait();
    MicQuizWinOpen(0);
    MicQuizWinInsertMesSet(0, micQuizCharaNameMes[micQuizModeWork->playerCharacter[playerIndex]],
                           0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_MORIAGE, 9), 1, 0);
    MicQuizWinMesWait();
    MicQuizWinClose();
}

/* Play the closing two-presenter exchange used when no named-player line is needed. */
void fn_1_1FE98(void) {
    MicQuizWinOpen(1);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_MORIAGE, 10), 1, 1);
    MicQuizWinMesWait();
    MicQuizWinOpen(0);
    MicQuizWinMesSet(MESSNUM(MESS_MICQUIZ_MORIAGE, 11), 1, 0);
    MicQuizWinMesWait();
    MicQuizWinClose();
}
