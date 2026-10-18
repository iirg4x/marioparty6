/* Microphone quiz setup, player choices, and round presentation. */
#include "REL/micquizdll/include/game/mic.h"
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

/* Audio cues for quiz presentation, panel movement, and score changes. */
#define MSM_SE_MICQUIZ_SCORE_TICK 1272
#define MSM_SE_MICQUIZ_PANEL_REVEAL 1276
#define MSM_SE_MICQUIZ_PANEL_MOVE 1277
#define MSM_SE_MICQUIZ_ALTERNATE_RESULT_MARKER 1279
#define MSM_SE_MICQUIZ_RESULT_MARKER 1280
#define MSM_SE_MICQUIZ_STANDINGS 1281
#define MSM_SE_MICQUIZ_ROUND_SET_COMPLETE 1284
#define MSM_SE_MICQUIZ_RESULTS_ANNOUNCE 1287
#define MSM_SE_MICQUIZ_PLAYER_REACTION 1289
#define MSM_SE_MICQUIZ_PLAYER_CELEBRATE 1290
#define MSM_SE_MICQUIZ_RESULT_MODEL_DROP 1291
#define MSM_SE_MICQUIZ_POINT_CHANGE_EFFECT 1293
#define MSM_SE_MICQUIZ_PRESENTER_SEQUENCE_BEAT 1294
#define MSM_SE_MICQUIZ_OPENING_MODEL_TRANSITION 1295
#define MSM_SE_MICQUIZ_PANEL_RETURN 1297
#define MSM_SE_MICQUIZ_OPENING_MODEL_REVEAL 1298
#define MSM_SE_MICQUIZ_EVENT_PANEL_APPEAR 1299
#define MSM_STREAM_MICQUIZ_WIN_JINGLE 93
#define MSM_STREAM_MICQUIZ_INTRODUCTION 95
#define MSM_STREAM_MICQUIZ_GRID_SELECTION 96
#define MSM_STREAM_MICQUIZ_OPENING_PRESENTATION 97
#define MSM_STREAM_MICQUIZ_ALTERNATE_GRID_SELECTION 98
#define MSM_STREAM_MICQUIZ_RESULTS 99

/* Screen positions for the four repeating event decorations. */
struct _struct_lbl_1_data_21C_0x8 {
              f32 positionX;
              f32 positionY;
};

/* Screen positions used by the tutorial pointer. */
typedef struct MicquizPositionData_s {
    f32 x;
    f32 y;
} MicquizPositionData;

/* Response fields read after the microphone choice session completes. */
typedef struct MicquizResponse {
    s16 result; /* Response returned by the microphone session. */
    u16 score; /* Recognition score of the first candidate. */
    s16 wordCount; /* Number of words in the first candidate. */
    s16 *words; /* Indices of the first candidate words. */
    s32 recognizedWord; /* First speech word or one-based controller choice; minus one if
                         * unavailable. */
} MicquizResponse;

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

/* Dialogue messages whose speaker cues use separate left and right pan positions. */
typedef struct QuizAnswerIds { s32 values[20]; } QuizAnswerIds;

/* Speaker cue numbers indexed by the dialogue sound callback. */
typedef struct QuizSoundIds { s32 values[17]; } QuizSoundIds;

/* Scene, question, and microphone display state. */
/* These bytes are not read by this mode; their purpose is unknown. */
u8 lbl_1_bss_5CE[10];
s16 lbl_1_bss_5CC;
MicquizChoiceRecord lbl_1_bss_54C[8];
HUPROCESS *lbl_1_bss_548;
s32 lbl_1_bss_544;
s32 lbl_1_bss_540;
s32 lbl_1_bss_53C;
s32 lbl_1_bss_538;
s32 lbl_1_bss_534;
s32 lbl_1_bss_530;
s32 lbl_1_bss_52C;
s32 lbl_1_bss_528;
s32 lbl_1_bss_524;
s32 lbl_1_bss_520;
s32 lbl_1_bss_51C;
s32 lbl_1_bss_518;
s32 lbl_1_bss_514;
s32 lbl_1_bss_510;
s16 lbl_1_bss_50C;
s32 lbl_1_bss_508;
s16 lbl_1_bss_504;
s32 lbl_1_bss_500;
s32 lbl_1_bss_4FC;
s32 lbl_1_bss_4F8;
ANIMDATA *lbl_1_bss_4F4;
/* These bytes are not read by this mode; their purpose is unknown. */
u8 lbl_1_bss_4EE[6];
s16 lbl_1_bss_4EC;
s32 lbl_1_bss_4E8;
f32 lbl_1_bss_4E4;
f32 lbl_1_bss_4E0;
s32 lbl_1_bss_4DC;
MicquizModeWorkView *lbl_1_bss_4D8;
ANIMDATA *lbl_1_bss_4D4;
ANIMDATA *lbl_1_bss_4CC[2];
ANIMDATA *lbl_1_bss_4C8;
ANIMDATA *lbl_1_bss_4C4;
ANIMDATA *lbl_1_bss_4C0;
ANIMDATA *lbl_1_bss_4BC;
ANIMDATA *lbl_1_bss_4B8;
ANIMDATA *lbl_1_bss_4B4;
ANIMDATA *lbl_1_bss_4B0;
ANIMDATA *lbl_1_bss_4AC;
s16 lbl_1_bss_4A8;
ANIMDATA *lbl_1_bss_4A4;
ANIMDATA *lbl_1_bss_4A0;
ANIMDATA *lbl_1_bss_49C;
ANIMDATA *lbl_1_bss_498;
ANIMDATA *lbl_1_bss_494;
ANIMDATA *lbl_1_bss_490;
ANIMDATA *lbl_1_bss_48C;
ANIMDATA *lbl_1_bss_488;
ANIMDATA *lbl_1_bss_480[2];
s16 lbl_1_bss_47E;
/* These bytes are not read by this mode; their purpose is unknown. */
u8 lbl_1_bss_476[8];
s16 lbl_1_bss_474;
s16 lbl_1_bss_472;
s16 lbl_1_bss_470;
s16 lbl_1_bss_46E;
s16 lbl_1_bss_46C;
s16 lbl_1_bss_46A;
s16 lbl_1_bss_468;
s16 lbl_1_bss_466;
HUSPR_GROUPID lbl_1_bss_460[3];
ANIMDATA *lbl_1_bss_450[4];
ANIMDATA *lbl_1_bss_44C;
ANIMDATA *lbl_1_bss_448;
ANIMDATA *lbl_1_bss_444;
ANIMDATA *lbl_1_bss_440;
s32 lbl_1_bss_43C;
ANIMDATA *lbl_1_bss_438;
s16 lbl_1_bss_434;
ANIMDATA *lbl_1_bss_424[4];
s16 lbl_1_bss_41C[4];
ANIMDATA *lbl_1_bss_3EC[4][3];
s16 lbl_1_bss_3D4[4][3];
ANIMDATA *lbl_1_bss_3D0;
s16 lbl_1_bss_3CC;
ANIMDATA *lbl_1_bss_3BC[4];
s16 lbl_1_bss_3B4[4];
MGSCORE *lbl_1_bss_3AC[2];
s16 lbl_1_bss_3AA;
s16 lbl_1_bss_3A8;
HUPROCESS *lbl_1_bss_3A4;
s32 lbl_1_bss_3A0;
s32 lbl_1_bss_39C;
ANIMDATA *lbl_1_bss_398;
ANIMDATA *lbl_1_bss_394;
s16 lbl_1_bss_390;
s16 lbl_1_bss_38E;
/* These bytes are not read by this mode; their purpose is unknown. */
u8 lbl_1_bss_388[6];
s16 lbl_1_bss_386;
s16 lbl_1_bss_384;
s16 lbl_1_bss_382;
s16 lbl_1_bss_380;
s16 lbl_1_bss_37E;
s16 lbl_1_bss_37C;
s16 lbl_1_bss_37A;
s16 lbl_1_bss_378;
HU3D_MODELID lbl_1_bss_376;
HU3D_MOTIONID lbl_1_bss_368[7];
HUPROCESS *lbl_1_bss_340[10];
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
void *lbl_1_bss_28;
OMOBJ *lbl_1_bss_24;
OMOBJ *lbl_1_bss_20;
s32 lbl_1_bss_1C;
f32 lbl_1_bss_18;
ANIMDATA *lbl_1_bss_14;
HUPROCESS *lbl_1_bss_10;
HUPROCESS *lbl_1_bss_C;
MicquizResponse *lbl_1_bss_8;
s32 lbl_1_bss_4;
HUPROCESS *lbl_1_bss_0;

s32 HuMCButtonGet(void);
s32 HuMCSelWinCheck(void);
void HuMCClose(void);
s32 HuMCMicGet();
void HuMCSessionClose(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern s32 omovlevtno;
extern u8 MgModeWork[256];
s32 HuMCMicGet(void);
extern s32 HuMCMicGet(void);
extern void HuMCClose(void);
extern void HuMCSessionClose(void);
extern s32 HuMCSelWinContextSet(s16 window, s32 flags, s32 playerNo);
extern void HuMCSelWinChoiceGet(void);
extern void HuMCSelWinContextKill(void);
extern u16 *HuMCCurResponseGet(void);
extern s32 HuMCUnkResponseCheck(void);
extern s32 HuMCNewResponseGet(u32 messageId);
extern s32 HuMCSelWinCheck(void);
extern s32 HuMCButtonGet(void);
extern s32 HuMCButtonDownGet(void);
int HuMCSelModeGet(void);
void HuMCSessionSet(s16 playerNo);

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
s32 fn_1_20138(void);
void fn_1_2029C(void);
void fn_1_20A50(u32 messageId, s16 messageSpeed, u8 windowIndex);
void fn_1_20B74(s32 mode);
void fn_1_20E64(u32 messageType);
void fn_1_21C44(void);
void fn_1_24294(void);
void fn_1_245C0(void);
void fn_1_24EC8(void);
void fn_1_25F20(void);
void fn_1_26224(void);
void fn_1_266A0(s32 action);
void fn_1_26A74(void);
void fn_1_26ACC(void);
void fn_1_28944(void);
void fn_1_28F68(s32 playerIndex);
void fn_1_2D34C(void);
void fn_1_2DA0C(void);
void fn_1_2DA6C(void);
void fn_1_2ECA8(void);
void fn_1_2F52C(u32 message);
void fn_1_2F9E4(s32 messageIndex);
void fn_1_306A0(s32 primaryMessageIndex, s32 secondaryMessageIndex);
void fn_1_30B0C(s32 primaryMessageIndex, s32 secondaryMessageIndex);
void fn_1_30E54(s32 messageIndex);
void fn_1_310F0(s32 success, s32 choice);
void fn_1_3186C(s32 motionNo, f32 blendFrames, u32 attr);
void fn_1_5EDC(void);
void fn_1_A290(void);
void fn_1_A308(void);
void fn_1_A348(OMOBJ *obj);
void fn_1_A3B4(s32 presenterNo, s32 motionNo, f32 blendFrames, u32 attr);
void fn_1_A484(s32 presenterNo);
void fn_1_A748(s32 playerNo);
void fn_1_A790(s32 wipeType, s32 duration);
void fn_1_A7E4(s32 wipeType, s32 duration);
void fn_1_A838(OM_CAMERA_VIEW *cameraView, s32 duration, u32 cameraMask);
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
void fn_1_20148(void);
void fn_1_20778(void);
void fn_1_208E4(void);
short fn_1_20CD4(short choice);
void fn_1_20D08(s32 windowNo, u32 messNum, s16 insertMesNo);
void fn_1_20E14(s32 visible);
void fn_1_20F50(s32 messageType);
void fn_1_214C0(HUPROCESS *parentProcess);
void fn_1_21A98(void);
void fn_1_21B80(void);
void fn_1_24138(void);
void fn_1_244A4(void);
void fn_1_24A54(s32 mode);
void fn_1_25120(void);
void fn_1_25B34(void);
void fn_1_25C88(void);
void fn_1_25DDC(void);
void fn_1_26528(void);
void fn_1_265E4(void);
void fn_1_269C0(void);
void fn_1_26A1C(void);
void fn_1_2A2D8(char *path, s32 mode);
void fn_1_2A344(s32 questionType);
s32 fn_1_2D214(s32 response, s32 expected);
s32 fn_1_2CDCC(s32 playerIndex, s32 responseMode);
void fn_1_2D22C(void);
void fn_1_2D748(s32 displayState);
void fn_1_2D95C(void);
void fn_1_2D9BC(void);
void fn_1_2DABC(s32 select, s32 padNo);
void fn_1_2DBFC(s32 padNo, s32 direction);
void fn_1_2E2B0(void);
void fn_1_2E6C4(s32 motionState);
void fn_1_2E828(void);
void fn_1_2EAA0(void);
void fn_1_2EBB0(void);
void fn_1_2FCA0(HUPROCESS* parent);
void fn_1_2FCF0(void);
void fn_1_2FD28(void);
void fn_1_2FE74(void);
s32 fn_1_3003C(void);
void fn_1_3011C(s32 value);
s32 fn_1_301AC(s32 minimum, s32 limit);
s32 fn_1_301E8(s32 chancePercent);
void fn_1_30248(s32 primaryMessageIndex, s32 secondaryMessageIndex);
s32 fn_1_310DC(void);
void fn_1_315C8(void);
int fn_1_316A4(int key);
int fn_1_316F4(int key);
void fn_1_31760(void);
void fn_1_31910(s32 enabled);
void fn_1_74C0(void);
void fn_1_75A4(void);
void fn_1_83F0(void);
void fn_1_8488(void);
void fn_1_8520(void);
int _prolog(void);
void _epilog(void);
s32 fn_1_A0(void);
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
void fn_1_24D94(void);
s32 fn_1_281D4(void);
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
void fn_1_200D0(s32 state);
void fn_1_200E0(void);
void fn_1_200F4(void);
void fn_1_20DBC(void);
void fn_1_20DE8(void);
void fn_1_20FBC(s16 layerNo);
void fn_1_215A4(void);
void fn_1_23748(void);
void fn_1_23F20(void);
void fn_1_25230(void);
void fn_1_26DD8(void);
void fn_1_27070(void);
void fn_1_27308(void);
void fn_1_27344(void);
void fn_1_27530(s32 accepted);
void fn_1_275CC(void);
void fn_1_2765C(void);
void fn_1_27730(s32 accepted, s32 questionType);
void fn_1_27840(void);
void fn_1_27A50(void);
void fn_1_27CC0(void);
void fn_1_27E18(void);
void fn_1_27F70(void);
void fn_1_27F74(void);
u8 fn_1_27F78(void);
u8 fn_1_27F8C(void);
void fn_1_27FA0(s8 playerNo, u16 points, s32 state);
s32 fn_1_27FD8(void);
s32 fn_1_27FEC(s32 playerNo);
f32 fn_1_28004(void);
s16 fn_1_206FC(s32 windowNo);
void fn_1_20B24(void);
void fn_1_20C94(void);
void fn_1_28178(void);
void fn_1_2819C(void);
s32 fn_1_281C0(void);
void fn_1_281E8(void);
void fn_1_28200(void);
void fn_1_28218(void);
void fn_1_284C4(u32 message);
void fn_1_28704(u32 message);
void fn_1_28B70(void);
void fn_1_28D6C(void);
void fn_1_2920C(void);
void fn_1_29408(u32 message);
void fn_1_2967C(void);
void fn_1_29878(void);
void fn_1_29A74(void);
void fn_1_29C70(void);
void fn_1_29E80(void);
void fn_1_2A07C(s32 messageIndex);
void fn_1_2A4B0(void);
void fn_1_2A4DC(void);
void fn_1_2A508(s32 quizMode);
void fn_1_2BBF4(s32 mode);
void fn_1_2DCEC(void);
void fn_1_2EF84(s32 messageType, s32 alternate);
void fn_1_2F76C(s32 messageIndex);
void fn_1_2FC5C(void);
void fn_1_2FC60(void);
void fn_1_30FC4(void);
void fn_1_31924(int base);
void fn_1_31A0C(s32 baseIndex);
void fn_1_31B20(void);
void fn_1_31C1C(void);
void fn_1_31CEC(HUWINID work, u32 answerId, s16 soundIndex);

/* Model and character resource lists used while constructing the quiz scene. */
s32 lbl_1_data_0[7] = {
    DATANUM(DATA_micquizishi, 122), DATANUM(DATA_micquizishi, 123),
    DATANUM(DATA_micquizishi, 124), DATANUM(DATA_micquizishi, 125),
    DATANUM(DATA_micquizishi, 126), DATANUM(DATA_micquizishi, 127),
    DATANUM(DATA_micquizishi, 128)
};
s32 lbl_1_data_1C[11] = {
    DATANUM(DATA_mgconst, 0), DATANUM(DATA_mgconst, 1),
    DATANUM(DATA_mgconst, 2), DATANUM(DATA_mgconst, 3),
    DATANUM(DATA_mgconst, 4), DATANUM(DATA_mgconst, 5),
    DATANUM(DATA_mgconst, 6), DATANUM(DATA_mgconst, 7),
    DATANUM(DATA_mgconst, 8), DATANUM(DATA_mgconst, 9),
    DATANUM(DATA_mgconst, 10)
};
s32 lbl_1_data_48[5] = { 10, 20, 30, 40, 50 };
/* Character-name and microphone-choice message lists. */
u32 lbl_1_data_5C[17] = {
    MESSNUM(MESS_QUIZ_CHARA, 12), MESSNUM(MESS_QUIZ_CHARA, 13),
    MESSNUM(MESS_QUIZ_CHARA, 14), MESSNUM(MESS_QUIZ_CHARA, 15),
    MESSNUM(MESS_QUIZ_CHARA, 16), MESSNUM(MESS_QUIZ_CHARA, 17),
    0, 1,
    2, 3,
    4, 5,
    6, 7,
    9, 10,
    MESSNUM(MESS_MIC_RETYPING, 22)
};
u32 lbl_1_data_A0[18] = {
    0, 1,
    2, 3,
    4, 5,
    6, 7,
    8, 9,
    10, MESSNUM(MESS_QUIZ_CHARA, 12),
    MESSNUM(MESS_QUIZ_CHARA, 13), MESSNUM(MESS_QUIZ_CHARA, 14),
    MESSNUM(MESS_QUIZ_CHARA, 15), MESSNUM(MESS_QUIZ_CHARA, 16),
    MESSNUM(MESS_QUIZ_CHARA, 17), MESSNUM(MESS_MIC_RETYPING, 22)
};
u32 lbl_1_data_E8[17] = {
    0, 1,
    2, 3,
    4, 5,
    6, 7,
    9, 10,
    MESSNUM(MESS_QUIZ_CHARA, 12), MESSNUM(MESS_QUIZ_CHARA, 13),
    MESSNUM(MESS_QUIZ_CHARA, 14), MESSNUM(MESS_QUIZ_CHARA, 15),
    MESSNUM(MESS_QUIZ_CHARA, 16), MESSNUM(MESS_QUIZ_CHARA, 17),
    MESSNUM(MESS_MIC_RETYPING, 22)
};
u32 lbl_1_data_12C[] = {
    MESSNUM(MESS_QUIZ_CHARA, 34), MESSNUM(MESS_QUIZ_CHARA, 35),
    MESSNUM(MESS_QUIZ_CHARA, 36), MESSNUM(MESS_QUIZ_CHARA, 37),
    MESSNUM(MESS_QUIZ_CHARA, 38), MESSNUM(MESS_QUIZ_CHARA, 39),
    MESSNUM(MESS_QUIZ_CHARA, 40), MESSNUM(MESS_QUIZ_CHARA, 41),
    MESSNUM(MESS_QUIZ_CHARA, 42), MESSNUM(MESS_QUIZ_CHARA, 43),
    MESSNUM(MESS_QUIZ_CHARA, 44), MESSNUM(MESS_QUIZ_CHARA, 45),
    MESSNUM(MESS_QUIZ_CHARA, 46), MESSNUM(MESS_QUIZ_CHARA, 47),
    MESSNUM(MESS_QUIZ_CHARA, 48), MESSNUM(MESS_QUIZ_CHARA, 49),
    MESSNUM(MESS_QUIZ_CHARA, 50), MESSNUM(MESS_QUIZ_CHARA, 51),
    MESSNUM(MESS_QUIZ_CHARA, 52), MESSNUM(MESS_QUIZ_CHARA, 53)
};
u32 lbl_1_data_17C[] = {
    MESSNUM(MESS_QUIZ_CHARA, 58), MESSNUM(MESS_QUIZ_CHARA, 59),
    MESSNUM(MESS_QUIZ_CHARA, 60), MESSNUM(MESS_QUIZ_CHARA, 61),
    MESSNUM(MESS_QUIZ_CHARA, 62), MESSNUM(MESS_QUIZ_CHARA, 63),
    MESSNUM(MESS_QUIZ_CHARA, 64), MESSNUM(MESS_QUIZ_CHARA, 65),
    MESSNUM(MESS_QUIZ_CHARA, 66), MESSNUM(MESS_QUIZ_CHARA, 67),
    MESSNUM(MESS_QUIZ_CHARA, 68), MESSNUM(MESS_QUIZ_CHARA, 69),
    MESSNUM(MESS_QUIZ_CHARA, 70), MESSNUM(MESS_QUIZ_CHARA, 71),
    MESSNUM(MESS_QUIZ_CHARA, 72), MESSNUM(MESS_QUIZ_CHARA, 73),
    MESSNUM(MESS_QUIZ_CHARA, 74), MESSNUM(MESS_QUIZ_CHARA, 75),
    MESSNUM(MESS_QUIZ_CHARA, 76), MESSNUM(MESS_QUIZ_CHARA, 77)
};
u32 lbl_1_data_1CC[2] = { MESSNUM(MESS_QUIZ_CHARA, 55), MESSNUM(MESS_QUIZ_CHARA, 56) };
u32 lbl_1_data_1D4[8] = {
    MESSNUM(MESS_QUIZ_CHARA, 25), MESSNUM(MESS_QUIZ_CHARA, 26),
    MESSNUM(MESS_QUIZ_CHARA, 27), MESSNUM(MESS_QUIZ_CHARA, 28),
    MESSNUM(MESS_QUIZ_CHARA, 29), MESSNUM(MESS_QUIZ_CHARA, 30),
    MESSNUM(MESS_QUIZ_CHARA, 31), MESSNUM(MESS_QUIZ_CHARA, 32)
};
s32 lbl_1_data_1F4 = -1;
s32 lbl_1_data_1F8 = -1;
s32 lbl_1_data_1FC[3] = { -1, -1, -1 };
s16 lbl_1_data_208 = -1;
s16 lbl_1_data_20A = -1;
s16 lbl_1_data_20C = -1;
s16 lbl_1_data_20E = -1;
s16 lbl_1_data_210[4] = { -1, -1, -1, -1 };
s16 lbl_1_data_218 = -1;
s16 lbl_1_data_21A = -1;
struct _struct_lbl_1_data_21C_0x8 lbl_1_data_21C[4] = {
    222.0f, 40.0f, 357.0f, 55.0f, 227.0f, 74.0f, 345.0f, 80.0f
};
u32 lbl_1_data_23C[16] = {
    MESSNUM(MESS_MICQUIZ_I, 0), MESSNUM(MESS_MICQUIZ_I, 1),
    MESSNUM(MESS_MICQUIZ_I, 2), MESSNUM(MESS_MICQUIZ_I, 3),
    MESSNUM(MESS_MICQUIZ_I, 5), MESSNUM(MESS_MICQUIZ_I, 6),
    MESSNUM(MESS_MICQUIZ_I, 7), MESSNUM(MESS_MICQUIZ_I, 8),
    MESSNUM(MESS_MICQUIZ_I, 11), MESSNUM(MESS_MICQUIZ_I, 12),
    MESSNUM(MESS_MICQUIZ_I, 13), MESSNUM(MESS_MICQUIZ_I, 14),
    MESSNUM(MESS_MICQUIZ_I, 15), MESSNUM(MESS_MICQUIZ_I, 16),
    MESSNUM(MESS_MICQUIZ_I, 17), MESSNUM(MESS_MICQUIZ_I, 18)
};

#include "REL/micquizdll/quiz_functions.h"

/* These bytes are not read by this mode; their purpose is unknown. */
u8 lbl_1_data_384[4] = { 255, 255, 255, 255 };
/* Character voice cues for introductions and score reactions. */
s16 lbl_1_data_388[11] = { 839, 833, 869, 893, 881, 815, 887, 821, 875, 857, 827 };
s16 lbl_1_data_39E[] = {
    MSM_SE_CHARMIC_MARIO, MSM_SE_CHARMIC_LUIGI, MSM_SE_CHARMIC_PEACH, MSM_SE_CHARMIC_YOSHI,
    MSM_SE_CHARMIC_WARIO, MSM_SE_CHARMIC_DAISY, MSM_SE_CHARMIC_WALUIGI, MSM_SE_CHARMIC_KINOPIO,
    MSM_SE_CHARMIC_TERESA, MSM_SE_CHARMIC_MINIKOOPA, MSM_SE_CHARMIC_KINOPIKO
};
s16 lbl_1_data_3B4[] = { 579, 555, 699, 795, 747, 483, 771, 507, 723, 651, 531 };
s16 lbl_1_data_3CA[11] = { 579, 555, 699, 795, 747, 483, 771, 507, 723, 651, 531 };
s16 lbl_1_data_3E0[12] = { 838, 832, 868, 892, 880, 814, 886, 820, 874, 856, 826, MSM_SE_CMN_01 };
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
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 6),
    DATANUM(DATA_mariomot, 40), DATANUM(DATA_mariomot, 30),
    DATANUM(DATA_mariomot, 23), DATANUM(DATA_mariomot, 87),
    DATANUM(DATA_mariomot, 21), DATANUM(DATA_mariomot, 36),
    DATANUM(DATA_mariomot, 37), 0
};
s32 lbl_1_data_448[2][4] = {
    DATANUM(DATA_micquiz, 3), DATANUM(DATA_micquiz, 4),
    DATANUM(DATA_micquiz, 5), DATANUM(DATA_micquiz, 6),
    DATANUM(DATA_micquiz, 7), DATANUM(DATA_micquiz, 8),
    DATANUM(DATA_micquiz, 9), DATANUM(DATA_micquiz, 10)
};
s32 lbl_1_data_468[4] = {
    DATANUM(DATA_micquiz, 12), DATANUM(DATA_micquiz, 13),
    DATANUM(DATA_micquiz, 14), DATANUM(DATA_micquiz, 15)
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
    520.1544189453125f, 65.0f, -315.9830017089844f, -10.0f,
    -75.0f, 0.0f, 1277.0f, 571.9180908203125f,
    65.0f, -122.7979965209961f, -10.0f, -75.0f,
    0.0f, 1277.0f, 623.681396484375f, 65.0f,
    70.38719940185547f, -10.0f, -75.0f, 0.0f,
    1277.0f, 675.4456787109375f, 65.0f, 263.5726013183594f,
    -10.0f, -75.0f, 0.0f, 1277.0f
};
OM_CAMERA_VIEW lbl_1_data_5C0 = { 390.0f, 165.0f, 30.0f, -10.0f, -75.0f, 0.0f, 1869.0f };
OM_CAMERA_VIEW lbl_1_data_5DC = { 0.0f, 100.0f, -10.0f, -3.700000047683716f, 0.0f, 0.0f, 1310.0f };
OM_CAMERA_VIEW lbl_1_data_5F8 = {
    -600.0f, 140.0f, -175.0f, -9.479999542236328f, 58.75f, 0.0f, 1746.0f
};
OM_CAMERA_VIEW lbl_1_data_614 = { 0.0f, 500.0f, -35.0f, -20.5f, 0.0f, 0.0f, 5365.0f };
OM_CAMERA_VIEW lbl_1_data_630 = { 0.0f, 185.0f, -35.0f, -6.0f, 0.0f, 0.0f, 1590.0f };
OM_CAMERA_VIEW lbl_1_data_64C = { 0.0f, 120.0f, 0.0f, 0.5f, 0.0f, 0.0f, 3113.0f };
Vec lbl_1_data_668[4] = {
    520.1544189453125f, 0.0f, -315.9830017089844f, 571.9180908203125f,
    0.0f, -122.7979965209961f, 623.681396484375f, 0.0f,
    70.38719940185547f, 675.4456787109375f, 0.0f, 263.5726013183594f
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
    -651.7999877929688f, 30.0f, -84.5999984741211f, -548.2000122070312f, 30.0f, -255.39999389648438f
};
Vec lbl_1_data_7FC[2] = { 0.0f, 58.75f, 0.0f, 0.0f, 58.75f, 0.0f };
Vec lbl_1_data_814 = { -600.0f, 0.0f, -170.0f };
Vec lbl_1_data_820 = { 0.0f, 58.75f, 0.0f };
s32 lbl_1_data_82C[4] = {
    DATANUM(DATA_micquiz, 82), DATANUM(DATA_micquiz, 83),
    DATANUM(DATA_micquiz, 84), DATANUM(DATA_micquiz, 85)
};
f32 lbl_1_data_83C[4][3] = {
    422.35430908203125f, 0.0f, -289.77801513671875f, 474.11810302734375f,
    0.0f, -96.59259796142578f, 525.8814086914062f, 0.0f,
    96.59259796142578f, 577.6456909179688f, 0.0f, 289.77801513671875f
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
    fn_1_A0();
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
s32 fn_1_A0(void) {
    OMOVLHIS *history;
    f32 characterHeight;
    s32 index;
    s32 column;
    u32 textureSize;
    s32 micInitResult;

    OSReport("******* MQ ObjectSetup *********\n");
    OSReport("          HEAP_HEAP Malloc Size %x\n", HuMemUsedMallocSizeGet(HEAP_HEAP));
    lbl_1_bss_288 = omInitObjMan(200, 8192);
    omGameSysInit(lbl_1_bss_288);
    Hu3DCameraCreate(7);
    Hu3DCameraPerspectiveSet(4, 20.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DCameraViewportSet(4, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(4, 0, 0, 640, 480);
    history = omOvlHisGet(0);
    lbl_1_bss_4D8 = (void *)MgModeWork;
    if (omovlevtno == 0) {
        fn_1_38B0();
    }
    lbl_1_bss_20 = omAddObjEx(lbl_1_bss_288, 32730, 0, 0, -1, omOutViewMulti);
    lbl_1_bss_20->work[0] = 3;
    lbl_1_bss_2C = omAddObjEx(lbl_1_bss_288, 100, 0, 0, -1, fn_1_A348);
    lbl_1_bss_24 = omAddObjEx(lbl_1_bss_288, 100, 0, 0, -1, (OMOBJ_FUNC)fn_1_E824);

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

    lbl_1_bss_4D8->compactPauseDisplay = 1;
    lbl_1_bss_4D8->playerCount = 0;
    for (index = 0; index < 4; index++) {
        if (GwPlayerConf[index].type != 0) {
            lbl_1_bss_4D8->playerActive[index] = 0;
        } else {
            lbl_1_bss_4D8->playerActive[index] = 1;
            lbl_1_bss_4D8->playerCount++;
        }
    }
    HuWinInit(1);
    fn_1_2029C();

    lbl_1_bss_236 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 67), HU_MEMNUM_OVL, HEAP_MODEL));
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        Hu3DModelAttrSet(lbl_1_bss_236, HU3D_MOTATTR_PAUSE);
    } else {
        Hu3DMotionTimeSet(lbl_1_bss_236, Hu3DMotionMaxTimeGet(lbl_1_bss_236));
    }
    Hu3DModelCameraSet(lbl_1_bss_236, 4);
    Hu3DModelLayerSet(lbl_1_bss_236, 0);

    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_4D8->playerActive[index] != 0) {
            lbl_1_bss_2F4[index] = GwPlayerConf[index].charNo;
            lbl_1_bss_2FC[index] = CharModelMotListCreate(
                lbl_1_bss_2F4[index], 4, lbl_1_data_420, lbl_1_bss_2AC[index]);
            if (lbl_1_bss_2F4[index] == 5) {
                Hu3DModelAmbSet(lbl_1_bss_2FC[index], 0.7142f, 0.7142f, 0.7142f);
            }
            characterHeight = CharModelHeightGet(lbl_1_bss_2F4[index]) / 2.0f;
            Hu3DModelAttrSet(lbl_1_bss_2FC[index], HU3D_MOTATTR_LOOP);
            CharMotionSet(lbl_1_bss_2F4[index], lbl_1_bss_2AC[index][0]);
            {
                Vec modelOffset;
                modelOffset.x = 100.0f * sin(-1.2566370614359172);
                modelOffset.y = 0.0f;
                modelOffset.z = 100.0f * cos(-1.2566370614359172);

                Hu3DModelPosSet(lbl_1_bss_2FC[index], lbl_1_data_668[index].x - modelOffset.x,
                                lbl_1_data_668[index].y - modelOffset.y,
                                lbl_1_data_668[index].z - modelOffset.z);
                Hu3DModelRotSet(lbl_1_bss_2FC[index], lbl_1_data_758[index][0],
                                lbl_1_data_758[index][1], lbl_1_data_758[index][2]);
                Hu3DModelCameraSet(lbl_1_bss_2FC[index], 4);

                lbl_1_bss_1F0[index] = Hu3DModelCreate(
                    HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 93), HU_MEMNUM_OVL, HEAP_MODEL));
                Hu3DModelCameraSet(lbl_1_bss_1F0[index], 4);
                Hu3DModelPosSet(lbl_1_bss_1F0[index], lbl_1_data_668[index].x - modelOffset.x,
                                0.0f, lbl_1_data_668[index].z - modelOffset.z);
                Hu3DModelRotSet(lbl_1_bss_1F0[index], lbl_1_data_758[index][0],
                                lbl_1_data_758[index][1], lbl_1_data_758[index][2]);
                Hu3DModelAttrSet(lbl_1_bss_1F0[index], HU3D_ATTR_DISPOFF);
                Hu3DModelAttrSet(lbl_1_bss_1F0[index], HU3D_MOTATTR_LOOP);
                Hu3DModelLayerSet(lbl_1_bss_1F0[index], 1);
            }
        }
    }

    CharEffectLayerSet(1);
    Hu3DZClearLayerSet(10);
    lbl_1_bss_256 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 49), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_256, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(lbl_1_bss_256, 0);
    Hu3DModelPosSet(lbl_1_bss_256, 0.0f, 0.0f, 0.0f);
    Hu3DModelCameraSet(lbl_1_bss_256, 1);
    lbl_1_bss_254 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 16), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_254, 1);
    Hu3DModelAttrSet(lbl_1_bss_254, HU3D_MOTATTR_LOOP);
    Hu3DModelPosSet(lbl_1_bss_254, 0.0f, 0.0f, -10.0f);
    Hu3DModelAttrSet(lbl_1_bss_254, HU3D_ATTR_DISPOFF);
    lbl_1_bss_252 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 17), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_252, 1);
    Hu3DModelAttrSet(lbl_1_bss_252, HU3D_MOTATTR_LOOP);
    Hu3DModelPosSet(lbl_1_bss_252, 0.0f, 0.0f, -10.0f);
    Hu3DModelAttrSet(lbl_1_bss_252, HU3D_ATTR_DISPOFF);
    if (lbl_1_bss_4D8->presenterLayout != 0) {
        Hu3DModelAttrReset(lbl_1_bss_252, HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_254, HU3D_ATTR_DISPOFF);
    }

    HuMemMemoryNumSet(lbl_1_bss_30 = HuSprAnimMake(640, 480, ANIM_BMP_RGB5A3),
        HU_MEMNUM_OVL);
    lbl_1_bss_30->bmp->palData = NULL;
    lbl_1_bss_30->bmp->palNum = 0;
    textureSize = GXGetTexBufferSize(640, 480, GX_TF_RGB5A3, GX_FALSE, 0);
    lbl_1_bss_30->bmp->data = HuMemDirectMallocNum(
        HEAP_MODEL, textureSize, HU_MEMNUM_OVL);
    HuMemMemoryNumSet(lbl_1_bss_34 = HuSprAnimMake(640, 480, ANIM_BMP_RGB5A3),
        HU_MEMNUM_OVL);
    lbl_1_bss_34->bmp->palData = NULL;
    lbl_1_bss_34->bmp->palNum = 0;
    textureSize = GXGetTexBufferSize(640, 480, GX_TF_RGB5A3, GX_FALSE, 0);
    lbl_1_bss_34->bmp->data = HuMemDirectMallocNum(
        HEAP_MODEL, textureSize, HU_MEMNUM_OVL);

    lbl_1_bss_23E =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 77), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_23E, HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet(lbl_1_bss_23E, 4);
    fn_1_19DD4(1, 1, 0);
    lbl_1_bss_23C =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 78), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_23C, HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet(lbl_1_bss_23C, 4);
    lbl_1_bss_23A =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 79), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_23A, HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet(lbl_1_bss_23A, 4);
    lbl_1_bss_21E =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 64), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_21E, HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet(lbl_1_bss_21E, 4);
    lbl_1_bss_21C =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 66), HU_MEMNUM_OVL, HEAP_MODEL));
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        Hu3DModelAttrSet(lbl_1_bss_21C, HU3D_MOTATTR_PAUSE);
    } else {
        Hu3DMotionTimeSet(lbl_1_bss_21C, Hu3DMotionMaxTimeGet(lbl_1_bss_21C));
    }
    Hu3DModelCameraSet(lbl_1_bss_21C, 4);
    lbl_1_bss_238 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 65), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_238, HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet(lbl_1_bss_238, 4);
    lbl_1_bss_234 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 68), HU_MEMNUM_OVL, HEAP_MODEL));
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        Hu3DModelAttrSet(lbl_1_bss_234, HU3D_MOTATTR_PAUSE);
    } else {
        Hu3DMotionTimeSet(lbl_1_bss_234, Hu3DMotionMaxTimeGet(lbl_1_bss_234));
    }
    Hu3DModelCameraSet(lbl_1_bss_234, 4);
    lbl_1_bss_232 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 69), HU_MEMNUM_OVL, HEAP_MODEL));
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        Hu3DModelAttrSet(lbl_1_bss_232, HU3D_MOTATTR_LOOP);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_232, HU3D_ATTR_DISPOFF);
    }
    Hu3DModelCameraSet(lbl_1_bss_232, 4);
    lbl_1_bss_230 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 70), HU_MEMNUM_OVL, HEAP_MODEL));
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        Hu3DModelAttrSet(lbl_1_bss_230, HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrSet(lbl_1_bss_230, HU3D_ATTR_DISPOFF);
    } else {
        Hu3DMotionTimeSet(lbl_1_bss_230, Hu3DMotionMaxTimeGet(lbl_1_bss_230));
    }
    Hu3DModelCameraSet(lbl_1_bss_230, 4);
    for (index = 0; index < 6; index++) {
        lbl_1_bss_224[index] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 71), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_224[index], HU3D_MOTATTR_LOOP);
        if (lbl_1_bss_4D8->presenterLayout == 0) {
            Hu3DMotionStartEndSet(lbl_1_bss_224[index], 1.0f, 120.0f);
        } else {
            Hu3DMotionStartEndSet(lbl_1_bss_224[index], 161.0f, 280.0f);
            Hu3DMotionTimeSet(lbl_1_bss_224[index], Hu3DMotionMaxTimeGet(lbl_1_bss_224[index]));
        }
        Hu3DModelRotSet(lbl_1_bss_224[index], 0.0f, lbl_1_data_478[index], 0.0f);
        Hu3DModelPosSet(lbl_1_bss_224[index], 0.0f, lbl_1_data_490[index], 0.0f);
        Hu3DModelCameraSet(lbl_1_bss_224[index], 4);
    }
    lbl_1_bss_222 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 72), HU_MEMNUM_OVL, HEAP_MODEL));
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        Hu3DModelAttrSet(lbl_1_bss_222, HU3D_MOTATTR_PAUSE);
    } else {
        Hu3DMotionTimeSet(lbl_1_bss_222, Hu3DMotionMaxTimeGet(lbl_1_bss_222));
    }
    Hu3DModelCameraSet(lbl_1_bss_222, 4);
    lbl_1_bss_220 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 73), HU_MEMNUM_OVL, HEAP_MODEL));
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        Hu3DModelAttrSet(lbl_1_bss_220, HU3D_MOTATTR_PAUSE);
    } else {
        Hu3DMotionTimeSet(lbl_1_bss_220, Hu3DMotionMaxTimeGet(lbl_1_bss_220));
    }
    Hu3DModelCameraSet(lbl_1_bss_220, 4);

    lbl_1_bss_1F8 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 92), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_1F8, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_1F8, HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet(lbl_1_bss_1F8, 4);
    Hu3DModelLayerSet(lbl_1_bss_1F8, 1);
    lbl_1_bss_202 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 94), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelPosSet(lbl_1_bss_202, 0.0f, 0.0f, -350.0f);
    Hu3DModelAttrSet(lbl_1_bss_202, HU3D_ATTR_DISPOFF);
    Hu3DModelCameraSet(lbl_1_bss_202, 4);
    fn_1_320F8();

    for (index = 0; index < 4; index++) {
        lbl_1_bss_214[index] = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_82C[index], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_214[index], HU3D_MOTATTR_PAUSE);
        Hu3DModelCameraSet(lbl_1_bss_214[index], 4);
        lbl_1_bss_20C[index] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 87), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_20C[index], HU3D_MOTATTR_LOOP);
        Hu3DModelCameraSet(lbl_1_bss_20C[index], 4);
        Hu3DModelPosSet(lbl_1_bss_20C[index], lbl_1_data_83C[index][0],
                        lbl_1_data_83C[index][1], lbl_1_data_83C[index][2]);
        lbl_1_bss_204[index] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 88), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_204[index], HU3D_MOTATTR_LOOP);
        Hu3DModelCameraSet(lbl_1_bss_204[index], 4);
        Hu3DModelPosSet(lbl_1_bss_204[index], lbl_1_data_83C[index][0],
                        lbl_1_data_83C[index][1], lbl_1_data_83C[index][2]);
        Hu3DModelLayerSet(lbl_1_bss_204[index], 1);
        if (lbl_1_bss_4D8->presenterLayout != 0) {
            Hu3DMotionTimeSet(lbl_1_bss_214[index], 45.0f);
        } else {
            Hu3DMotionTimeSet(lbl_1_bss_214[index], 0.0f);
            Hu3DModelAttrSet(lbl_1_bss_204[index], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_204[index], HU3D_MOTATTR_PAUSE);
        }
    }

    for (index = 0, column = 0; index < 4; index++) {
        if (lbl_1_bss_4D8->playerActive[index] != 0) {
            lbl_1_bss_1FA[index] = Hu3DModelLink(lbl_1_bss_202);
            Hu3DModelPosSet(lbl_1_bss_1FA[index],
                lbl_1_data_698[lbl_1_bss_4D8->playerCount - 1].playerPositions[column].x,
                lbl_1_data_698[lbl_1_bss_4D8->playerCount - 1].playerPositions[column].y,
                -350.0f);
            column++;
            Hu3DModelAttrSet(lbl_1_bss_1FA[index], HU3D_ATTR_DISPOFF);
            Hu3DModelCameraSet(lbl_1_bss_1FA[index], 4);
        }
    }
    lbl_1_bss_1EC[0] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 75), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_1EC[1] = Hu3DModelLink(lbl_1_bss_1EC[0]);
    Hu3DModelAttrSet(lbl_1_bss_1EC[0], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_1EC[1], HU3D_ATTR_DISPOFF);
    Hu3DModelLayerSet(lbl_1_bss_1EC[0], 3);
    Hu3DModelLayerSet(lbl_1_bss_1EC[1], 3);

    lbl_1_bss_1E8 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_effect, 1), HU_MEMNUM_OVL, HEAP_MODEL));
    HuDataDirClose(DATA_effect);
    lbl_1_bss_314[0] = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 50), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_310 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_310, 0, HuSprCreate(lbl_1_bss_314[0], 50, 2));
    HuSprAttrSet(lbl_1_bss_310, 0, HUSPR_ATTR_DISPOFF);
    HuPrcChildCreate(fn_1_1B7B4, 100, 4096, 0, lbl_1_bss_288);
    lbl_1_bss_304.x = 0.0f;
    lbl_1_bss_304.y = 0.0f;
    lbl_1_bss_30C = 2;
    lbl_1_bss_1E2 = Hu3DMotionCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 89), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_1E0 = Hu3DMotionCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 90), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_1DE = Hu3DMotionCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 91), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_1DC = Hu3DJointMotion(
        lbl_1_bss_23E, HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 80), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_4F4 = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 27), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_4EC, 0, HuSprCreate(lbl_1_bss_4F4, 49, 0));
    HuSprPosSet(lbl_1_bss_4EC, 0, 288.0f, 240.0f);
    HuSprScaleSet(lbl_1_bss_4EC, 0, 2.0f, 2.0f);
    HuSprTPLvlSet(lbl_1_bss_4EC, 0, 0.0f);
    HuSprColorSet(lbl_1_bss_4EC, 0, 255, 0, 0);
    HuSprAttrSet(lbl_1_bss_4EC, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(lbl_1_bss_4EC, 0, 8);
    HuSprDrawNoSet(lbl_1_bss_4EC, 0, 0);

    lbl_1_bss_248 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 47), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_248, 1);
    Hu3DModelAttrSet(lbl_1_bss_248, HU3D_ATTR_DISPOFF);
    lbl_1_bss_246 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 48), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_246, 1);
    Hu3DModelAttrSet(lbl_1_bss_246, HU3D_ATTR_DISPOFF);
    lbl_1_bss_244 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 39), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_244, 1);
    Hu3DModelAttrSet(lbl_1_bss_244, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_244, HU3D_MOTATTR_PAUSE);
    lbl_1_bss_1D6 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 97), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_1D6, 2);
    Hu3DMotionSpeedSet(lbl_1_bss_1D6, 0.7f);
    Hu3DModelPosSet(lbl_1_bss_1D6, 0.0f, 0.0f, 1700.0f);
    Hu3DModelAttrSet(lbl_1_bss_1D6, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(lbl_1_bss_1D6, HU3D_ATTR_DISPOFF);
    lbl_1_bss_1D8 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 98), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_1D8, 2);
    Hu3DModelPosSet(lbl_1_bss_1D8, 0.0f, 0.0f, 1700.0f);
    Hu3DModelAttrSet(lbl_1_bss_1D8, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(lbl_1_bss_1D8, HU3D_ATTR_DISPOFF);

    lbl_1_bss_1D4 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 96), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_1D4, 2);
    Hu3DModelPosSet(lbl_1_bss_1D4, 0.0f, 0.0f, 200.0f);
    Hu3DModelAttrSet(lbl_1_bss_1D4, HU3D_ATTR_DISPOFF);
    lbl_1_bss_1D2 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 95), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_1D2, 2);
    Hu3DModelAttrSet(lbl_1_bss_1D2, HU3D_ATTR_DISPOFF);
    Hu3DModelPosSet(lbl_1_bss_1D2, 0.0f, 0.0f, -10.0f);
    lbl_1_bss_1D0 = Hu3DModelLink(lbl_1_bss_1D2);
    Hu3DModelCameraSet(lbl_1_bss_1D0, 2);
    Hu3DModelRotSet(lbl_1_bss_1D0, 0.0f, 180.0f, 0.0f);
    Hu3DModelAttrSet(lbl_1_bss_1D0, HU3D_ATTR_DISPOFF);
    Hu3DModelTPLvlSet(lbl_1_bss_1D0, 0.0f);
    Hu3DModelPosSet(lbl_1_bss_1D0, 0.0f, 0.0f, -10.0f);
    lbl_1_bss_250 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 16), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_250, 2);
    Hu3DModelPosSet(lbl_1_bss_250, 0.0f, 0.0f, -10.0f);
    Hu3DModelAttrSet(lbl_1_bss_250, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(lbl_1_bss_250, HU3D_ATTR_DISPOFF);
    lbl_1_bss_24E = Hu3DModelLink(lbl_1_bss_250);
    Hu3DModelCameraSet(lbl_1_bss_24E, 2);
    Hu3DModelPosSet(lbl_1_bss_24E, 0.0f, 0.0f, -10.0f);
    Hu3DModelAttrSet(lbl_1_bss_24E, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(lbl_1_bss_24E, HU3D_ATTR_DISPOFF);
    if (lbl_1_bss_4D8->presenterLayout != 0) {
        Hu3DModelAttrReset(lbl_1_bss_24E, HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_250, HU3D_ATTR_DISPOFF);
    }
    lbl_1_bss_24C =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 18), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_24C, 2);
    Hu3DModelAttrSet(lbl_1_bss_24C, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_24C, HU3D_MOTATTR_LOOP);
    lbl_1_bss_24A = Hu3DModelLink(lbl_1_bss_24C);
    Hu3DModelPosSet(lbl_1_bss_24A, 0.0f, 0.0f, 50.0f);
    Hu3DModelCameraSet(lbl_1_bss_24A, 1);
    Hu3DModelAttrSet(lbl_1_bss_24A, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_24A, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(lbl_1_bss_24A, 1);
    lbl_1_bss_6C =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 20), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_68 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 41), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_64 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 22), HU_MEMNUM_OVL, HEAP_MODEL));
    for (index = 0; index < 5; index++) {
        for (column = 0; column < 5; column++) {
            if (column == 0 && index == 0) {
                lbl_1_bss_198[column].model[index] = Hu3DModelCreate(
                    HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 19), HU_MEMNUM_OVL, HEAP_MODEL));
            } else {
                lbl_1_bss_198[column].model[index] = Hu3DModelLink(lbl_1_bss_198[0].model[0]);
            }
            Hu3DModelPosSet(lbl_1_bss_198[column].model[index], 100.0f * column,
                           -300.0f + (500.0f - 100.0f * index), 0.0f);
            Hu3DModelLayerSet(lbl_1_bss_198[column].model[index], 1);
            Hu3DModelAttrSet(lbl_1_bss_198[column].model[index], HU3D_MOTATTR_PAUSE);
            Hu3DModelAttrSet(lbl_1_bss_198[column].model[index], HU3D_MOTATTR_LOOP);
            lbl_1_bss_166[column][index] = Hu3DAnimCreate(lbl_1_bss_6C,
                lbl_1_bss_198[column].model[index], "vq_panel");
            Hu3DAnimBankSet(lbl_1_bss_166[column][index], 0);
            lbl_1_bss_134[column][index] = Hu3DAnimCreate(lbl_1_bss_68,
                lbl_1_bss_198[column].model[index], "vq_color");
            Hu3DAnimBankSet(lbl_1_bss_134[column][index], 0);
            lbl_1_bss_102[column].model[index] = Hu3DAnimCreate(lbl_1_bss_64,
                lbl_1_bss_198[column].model[index], "vq_ivent");
            Hu3DAnimBankSet(lbl_1_bss_102[column].model[index], 0);
            Hu3DModelAttrReset(lbl_1_bss_198[column].model[index], HU3D_ATTR_DISPOFF);
            Hu3DModelCameraSet(lbl_1_bss_198[column].model[index], 1);
        }
    }
    lbl_1_bss_62 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 25), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_62, HU3D_ATTR_DISPOFF);
    Hu3DModelCameraSet(lbl_1_bss_62, 1);
    Hu3DModelLayerSet(lbl_1_bss_62, 1);
    lbl_1_bss_60 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 26), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_60, HU3D_ATTR_DISPOFF);
    Hu3DModelCameraSet(lbl_1_bss_60, 1);
    Hu3DModelLayerSet(lbl_1_bss_60, 1);
    lbl_1_bss_100 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 23), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_100, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_100, HU3D_MOTATTR_PAUSE);
    Hu3DModelCameraSet(lbl_1_bss_100, 1);
    lbl_1_bss_FE =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 24), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_FE, 1);
    Hu3DModelScaleSet(lbl_1_bss_FE, 0.9f, 0.9f, 0.9f);
    Hu3DModelAttrSet(lbl_1_bss_FE, HU3D_ATTR_DISPOFF);
    for (index = 0; index < 4; index++) {
        for (column = 0; column < 3; column++) {
            lbl_1_bss_CC[index][column] = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 86), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_B4[index][column] = Hu3DAnimCreate(
                lbl_1_bss_CC[index][column], lbl_1_bss_214[index],
                lbl_1_data_888[column]);
            Hu3DAnimBankSet(lbl_1_bss_B4[index][column], 0);
        }
    }
    lbl_1_bss_FC =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 74), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_FC, 1);
    Hu3DModelScaleSet(lbl_1_bss_FC, 0.5f, 0.5f, 0.5f);
    Hu3DModelLayerSet(lbl_1_bss_FC, 2);
    Hu3DModelAttrSet(lbl_1_bss_FC, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(lbl_1_bss_FC, HU3D_ATTR_DISPOFF);
    Hu3DModelPosSet(lbl_1_bss_FC, -425.0f, 240.0f, 10.0f);
    lbl_1_bss_5C = 0;
    omCameraViewSetMulti(1, &lbl_1_data_4FC);
    omCameraViewSetMulti(2, &lbl_1_data_518);
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
    fn_1_245C0();
    lbl_1_bss_4D8->pauseAllowed = 1;
    if (omovlevtno == 0 || omovlevtno == 2 || omovlevtno == 3) {
        Hu3DModelAttrSet(lbl_1_bss_23C, HU3D_MOTATTR_PAUSE);
        if (omovlevtno == 2 || omovlevtno == 3) {
            omCameraViewSetMulti(4, &lbl_1_data_5DC);
        } else {
            omCameraViewSetMulti(4, &lbl_1_data_64C);
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
        if (lbl_1_bss_4D8->presenterLayout == 0) {
            fn_1_3BFC();
        }
        fn_1_3E7C();
        OSReport("-----------------------------First!!\n");
        lbl_1_bss_258 = 0;
    } else {
        if (fn_1_281D4() == 7) {
            lbl_1_bss_4D8->questionColumn = lbl_1_bss_4D8->savedQuestionColumn;
        } else if (fn_1_281D4() == 2 || fn_1_281D4() == 5) {
            lbl_1_bss_4D8->questionLevel = lbl_1_bss_4D8->savedQuestionLevel;
        }
        omCameraViewSetMulti(4, &lbl_1_data_534);
        fn_1_19DD4(0, 1, 0);
        OSReport("-----------------------------Not First!!\n");
        lbl_1_bss_258 = 1;
        fn_1_E6C0(lbl_1_bss_4D8->questionLevel, lbl_1_bss_4D8->questionColumn,
                   lbl_1_bss_4D8->answeringPlayer, lbl_1_bss_4D8->roundResult);
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
        if (lbl_1_bss_4D8->presenterLayout == 0) {
            fn_1_3BFC();
        }
        fn_1_3E7C();
        Hu3DModelAttrSet(lbl_1_bss_238, HU3D_ATTR_DISPOFF);
        if (lbl_1_bss_4D8->presenterLayout == 0) {
            for (index = 0; index < 2; index++) {
                Hu3DModelPosSet(lbl_1_bss_2A6[index], lbl_1_data_7E4[index].x,
                               lbl_1_data_7E4[index].y, lbl_1_data_7E4[index].z);
                Hu3DModelRotSet(lbl_1_bss_2A6[index], lbl_1_data_7FC[index].x,
                               lbl_1_data_7FC[index].y, lbl_1_data_7FC[index].z);
            }
        }
    }
    lbl_1_bss_58 = HuSprAnimRead(HuDataReadNum(DATANUM(DATA_micquiz, 51), HU_MEMNUM_OVL));
    lbl_1_bss_54 = HuSprAnimRead(HuDataReadNum(DATANUM(DATA_micquiz, 52), HU_MEMNUM_OVL));
    OSReport("**************** MQ ObjectSetup End ******************\n");
    HuPrcChildCreate(fn_1_7688, 100, 6144, 0, lbl_1_bss_288);
    HuPrcChildCreate(fn_1_1E448, 100, 6144, 0, lbl_1_bss_288);
    fn_1_214C0(lbl_1_bss_288);
    fn_1_25230();
    lbl_1_bss_4D8->pauseAllowed = 1;
}

/* Initialize persistent quiz selections, scores, and character choices on the first overlay
 * entry. */
void fn_1_38B0(void) {
    s32 index;
    s32 playerIndex;

    lbl_1_bss_4D8->questionCount = 0;
    lbl_1_bss_4D8->currentPlayer = 0;
    lbl_1_bss_4D8->answeringPlayer = -1;
    lbl_1_bss_4D8->roundResult = 1;
    lbl_1_bss_4D8->roundPoints = 0;
    lbl_1_bss_4D8->questionLevel = 0;
    lbl_1_bss_4D8->questionColumn = 0;
    lbl_1_bss_4D8->compactPauseDisplay = 1;
    lbl_1_bss_4D8->pauseAllowed = 1;
    lbl_1_bss_4D8->setupValue = 1;
    lbl_1_bss_4D8->pauseDisplayActive = 0;

    for (index = 0; index < 5; index++) {
        for (playerIndex = 0; playerIndex < 5; playerIndex++) {
            lbl_1_bss_4D8->questionFlags[playerIndex][index] = 0;
        }
    }

    lbl_1_bss_4D8->questionEvent = 8;
    for (index = 0; index < 4; index++) {
        lbl_1_bss_4D8->playerScore[index] = 0;
        lbl_1_bss_4D8->playerCharacter[index] = GwPlayerConf[index].charNo;
        lbl_1_bss_4D8->playerRank[index] = 1;
    }
    lbl_1_bss_4D8->presenterLayout = 0;
}

/* Replace or change a player panel model when the round presentation changes its motion mode. */
void fn_1_3A8C(s32 modelIndex, s32 motionMode) {
    if (motionMode == 0) {
        Hu3DModelKill(lbl_1_bss_20C[modelIndex]);
        lbl_1_bss_20C[modelIndex] =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 87), HU_MEMNUM_OVL, 2));
        Hu3DModelAttrSet(lbl_1_bss_20C[modelIndex], HU3D_MOTATTR_LOOP);
        Hu3DModelCameraSet(lbl_1_bss_20C[modelIndex], 4);
        Hu3DModelPosSet(lbl_1_bss_20C[modelIndex], lbl_1_data_83C[modelIndex][0],
                        lbl_1_data_83C[modelIndex][1], lbl_1_data_83C[modelIndex][2]);
    } else if (motionMode == 1) {
        Hu3DMotionSet(lbl_1_bss_20C[modelIndex], lbl_1_bss_1E0);
    } else if (motionMode == 2) {
        Hu3DMotionSet(lbl_1_bss_20C[modelIndex], lbl_1_bss_1DE);
    }
}

/* Load the two presenter models, their motions, and the hooked item during scene setup. */
void fn_1_3BFC(void) {
    s32 motionIndex;
    s32 presenterIndex;

    presenterIndex = 0;
    while (presenterIndex < 2) {
        if (presenterIndex == 0) {
            lbl_1_bss_2A6[presenterIndex] = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 0), HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            lbl_1_bss_2A6[presenterIndex] = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 1), HU_MEMNUM_OVL, HEAP_MODEL));
        }
        motionIndex = 0;
        while (motionIndex < 4) {
            lbl_1_bss_296[presenterIndex][motionIndex] = Hu3DJointMotion(
                lbl_1_bss_2A6[presenterIndex],
                HuDataSelHeapReadNum(lbl_1_data_448[presenterIndex][motionIndex],
                                     HU_MEMNUM_OVL, HEAP_MODEL));
            motionIndex += 1;
        }
        Hu3DModelAttrSet(lbl_1_bss_2A6[presenterIndex], HU3D_MOTATTR_LOOP);
        Hu3DMotionSet(lbl_1_bss_2A6[presenterIndex], lbl_1_bss_296[presenterIndex][0]);
        Hu3DModelPosSet(lbl_1_bss_2A6[presenterIndex], lbl_1_data_7B4[presenterIndex].x,
                        lbl_1_data_7B4[presenterIndex].y, lbl_1_data_7B4[presenterIndex].z);
        Hu3DModelRotSet(lbl_1_bss_2A6[presenterIndex], lbl_1_data_7CC[presenterIndex].x,
                        lbl_1_data_7CC[presenterIndex].y, lbl_1_data_7CC[presenterIndex].z);
        Hu3DModelCameraSet(lbl_1_bss_2A6[presenterIndex], 4U);
        presenterIndex += 1;
    }
    lbl_1_bss_2A6[2] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 2), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_2A6[2], 4U);
    Hu3DModelHookSet(lbl_1_bss_2A6[1], "gN01m1-itemhook_R", lbl_1_bss_2A6[2]);
}

/* Load the single-presenter model and its motions during scene setup. */
void fn_1_3E7C(void) {
    s32 index;

    lbl_1_bss_294 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 11), HU_MEMNUM_OVL, HEAP_MODEL));
    for (index = 0; index < 4; index++) {
        lbl_1_bss_28C[index] = Hu3DJointMotion(
            lbl_1_bss_294,
            HuDataSelHeapReadNum(lbl_1_data_468[index], HU_MEMNUM_OVL, HEAP_MODEL)
        );
    }
    /* Only the two-presenter layout uses the individual presenter models. */
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        Hu3DModelAttrSet(lbl_1_bss_294, HU3D_ATTR_DISPOFF);
    }
    fn_1_A4CC(0, (0.0f), HU3D_MOTATTR_LOOP);
    Hu3DModelPosSet(lbl_1_bss_294, lbl_1_data_814.x, lbl_1_data_814.y, lbl_1_data_814.z);
    Hu3DModelRotSet(lbl_1_bss_294, lbl_1_data_820.x, lbl_1_data_820.y, lbl_1_data_820.z);
    Hu3DModelCameraSet(lbl_1_bss_294, 4);
}

/* Capture the first camera image into its texture after the camera layer draws. */
void fn_1_3FE4(s16 layerNo) {
    fn_1_407C(0, 0, 640, 480, 5, lbl_1_bss_30->bmp->data);
    Hu3DZClear();
}

/* Capture the second camera image into its texture after the camera layer draws. */
void fn_1_4030(s16 layerNo) {
    fn_1_407C(0, 0, 640, 480, 5, lbl_1_bss_34->bmp->data);
    Hu3DZClear();
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
    OM_CAMERA_VIEW wideCamera;
    OM_CAMERA_VIEW presenterCamera;
    OM_CAMERA_VIEW playerCamera;
    OM_CAMERA_VIEW *wideView;
    OM_CAMERA_VIEW *presenterView;
    OM_CAMERA_VIEW *playerView;
    s32 playerNo;

    HuAudSStreamFadeOut(lbl_1_bss_4E8, 1000);
    fn_1_1AFC4(30, 200);
    HuPrcSleep(30);
    fn_1_A3B4(0, 3, (8.0f), HU3D_MOTATTR_LOOP);
    fn_1_A3B4(1, 3, (8.0f), HU3D_MOTATTR_LOOP);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (lbl_1_bss_4D8->playerActive[playerNo] != 0) {
            fn_1_A678(playerNo, 4, (8.0f), HU3D_MOTATTR_LOOP);
        }
    }
    HuPrcSleep(45);
    wideCamera = lbl_1_data_614;
    wideView = &wideCamera;
    fn_1_A838(wideView, 120, 4);
    lbl_1_bss_4E8 = HuAudSStreamPlay(MSM_STREAM_MICQUIZ_OPENING_PRESENTATION);
    Hu3DModelAttrReset(lbl_1_bss_234, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrReset(lbl_1_bss_236, HU3D_MOTATTR_PAUSE);
    for (playerNo = 0; playerNo < 6; playerNo++) {
        Hu3DMotionStartEndSet(lbl_1_bss_224[playerNo], (121.0f),
                              (160.0f));
        Hu3DModelAttrReset(lbl_1_bss_224[playerNo], HU3D_MOTATTR_LOOP);
    }
    do {
        HuPrcVSleep();
    } while (Hu3DMotionEndCheck(lbl_1_bss_224[0]) == 0);
    for (playerNo = 0; playerNo < 6; playerNo++) {
        Hu3DMotionStartEndSet(lbl_1_bss_224[playerNo], (161.0f),
                              (280.0f));
        Hu3DModelAttrSet(lbl_1_bss_224[playerNo], HU3D_MOTATTR_LOOP);
    }
    HuPrcChildCreate(fn_1_48D8, 100, 6144, 0, lbl_1_bss_288);
    HuAudFXPlay(MSM_SE_MICQUIZ_OPENING_MODEL_TRANSITION);
    Hu3DModelAttrSet(lbl_1_bss_232, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(lbl_1_bss_230, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrReset(lbl_1_bss_230, HU3D_ATTR_DISPOFF);
    HuPrcSleep(120);
    HuAudFXPlay(MSM_SE_MICQUIZ_OPENING_MODEL_REVEAL);
    Hu3DModelAttrReset(lbl_1_bss_1F8, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(lbl_1_bss_24A, HU3D_ATTR_DISPOFF);
    HuPrcSleep(60);
    Hu3DModelAttrSet(lbl_1_bss_254, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(lbl_1_bss_252, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_24A, HU3D_ATTR_DISPOFF);
    HuPrcSleep(60);
    fn_1_1AED8(30);
    HuPrcSleep(30);
    presenterCamera = lbl_1_data_5C0;
    presenterView = &presenterCamera;
    fn_1_A838(presenterView, 120, 4);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        Hu3DModelAttrReset(lbl_1_bss_214[playerNo], HU3D_MOTATTR_PAUSE);
    }
    HuPrcSleep(45);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        Hu3DModelAttrReset(lbl_1_bss_204[playerNo], HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrReset(lbl_1_bss_204[playerNo], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_214[playerNo], HU3D_MOTATTR_PAUSE);
    }
    HuPrcChildCreate(fn_1_4868, 100, 4096, 0, lbl_1_bss_288);
    Hu3DModelAttrReset(lbl_1_bss_220, HU3D_MOTATTR_PAUSE);
    HuPrcSleep(120);
    playerCamera = lbl_1_data_5F8;
    playerView = &playerCamera;
    fn_1_A838(playerView, 120, 4);
    HuPrcChildCreate(fn_1_4808, 100, 4096, 0, lbl_1_bss_288);
    Hu3DModelAttrReset(lbl_1_bss_222, HU3D_MOTATTR_PAUSE);
    HuPrcSleep(150);
    Hu3DModelAttrReset(lbl_1_bss_21C, HU3D_MOTATTR_PAUSE);
    Hu3DMotionSet(lbl_1_bss_294, lbl_1_bss_28C[2]);
    Hu3DMotionTimeSet(lbl_1_bss_294, (100.0f));
    Hu3DModelAttrReset(lbl_1_bss_294, HU3D_ATTR_DISPOFF);
    HuPrcSleep(30);
    fn_1_1E3F8(60, (25.0f), (0.85f), (90.0f));
    HuAudFXPlay(MSM_SE_BRD00_115);
    fn_1_A3B4(0, 2, (8.0f), 0);
    fn_1_A3B4(1, 2, (8.0f), 0);
    HuPrcSleep(8);
    HuPrcChildCreate(fn_1_4B00, 100, 4096, 0, lbl_1_bss_288);
    HuPrcChildCreate(fn_1_4CB8, 100, 4096, 0, lbl_1_bss_288);
    HuPrcSleep(70);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (lbl_1_bss_4D8->playerActive[playerNo] != 0) {
            fn_1_A678(playerNo, 0, (8.0f), HU3D_MOTATTR_LOOP);
        }
    }
    fn_1_A4CC(0, (8.0f), HU3D_MOTATTR_LOOP);
    HuPrcSleep(8);
    Hu3DModelAttrSet(lbl_1_bss_1F8, HU3D_ATTR_DISPOFF);
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

    HuAudFXPlay(MSM_SE_BRD00_103);
    HuSprAttrReset(lbl_1_bss_4EC, 0, HUSPR_ATTR_DISPOFF);
    angle = 0;
    while (angle < 360) {
        if (sin((3.141592653589793 * fn_1_4A68((f32)angle)) / 180.0) < 0.0) {
            opacity = -sin((3.141592653589793 * fn_1_4A68((f32)angle)) / 180.0);
        } else {
            opacity = sin((3.141592653589793 * fn_1_4A68((f32)angle)) / 180.0);
        }
        HuSprTPLvlSet(lbl_1_bss_4EC, 0, (f32)opacity);
        HuPrcVSleep();
        angle += 2;
    }
    HuSprAttrSet(lbl_1_bss_4EC, 0, HUSPR_ATTR_DISPOFF);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Wrap animation angles into the signed half-turn range before trigonometric evaluation. */
f32 fn_1_4A68(f32 angle)
{
    angle = (f32)fmod(angle, (360.0));
    if (angle < (-180.0f)) {
        angle += (360.0f);
    } else if (angle > (180.0f)) {
        angle -= (360.0f);
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

    Hu3DModelPosGet(lbl_1_bss_2A6[0], &position);
    Hu3DModelRotGet(lbl_1_bss_2A6[0], &rotation);
    positionStep.x = 0.0f;
    positionStep.y = 10.0f;
    positionStep.z = 8.0f;
    rotationStep.x = 5.0f;
    rotationStep.y = 0.0f;
    rotationStep.z = 10.0f;
    frame = 0;
    for (frame = 0; frame < 120; frame++) {
        HuPrcVSleep();
        if (Hu3DMotionTimeGet(lbl_1_bss_2A6[0]) > 10.0f) {
            Hu3DModelAttrSet(lbl_1_bss_2A6[0], HU3D_MOTATTR_PAUSE);
        }
        position.x += positionStep.x;
        position.y += positionStep.y;
        position.z += positionStep.z;
        rotation.x += rotationStep.x;
        rotation.y += rotationStep.y;
        rotation.z += rotationStep.z;
        Hu3DModelPosSet(lbl_1_bss_2A6[0], position.x, position.y, position.z);
        Hu3DModelRotSet(lbl_1_bss_2A6[0], rotation.x, rotation.y, rotation.z);
    }
    Hu3DModelAttrSet(lbl_1_bss_2A6[0], HU3D_ATTR_DISPOFF);
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

    Hu3DModelPosGet(lbl_1_bss_2A6[1], &position);
    Hu3DModelRotGet(lbl_1_bss_2A6[1], &rotation);
    positionStep.x = 0.0f;
    positionStep.y = 10.0f;
    positionStep.z = -8.0f;
    rotationStep.x = 5.0f;
    rotationStep.y = 0.0f;
    rotationStep.z = -10.0f;
    frame = 0;
    for (frame = 0; frame < 120; frame++) {
        HuPrcVSleep();
        if (Hu3DMotionTimeGet(lbl_1_bss_2A6[1]) > 10.0f) {
            Hu3DModelAttrSet(lbl_1_bss_2A6[1], HU3D_MOTATTR_PAUSE);
        }
        position.x += positionStep.x;
        position.y += positionStep.y;
        position.z += positionStep.z;
        rotation.x += rotationStep.x;
        rotation.y += rotationStep.y;
        rotation.z += rotationStep.z;
        Hu3DModelPosSet(lbl_1_bss_2A6[1], position.x, position.y, position.z);
        Hu3DModelRotSet(lbl_1_bss_2A6[1], rotation.x, rotation.y, rotation.z);
    }
    Hu3DModelAttrSet(lbl_1_bss_2A6[1], HU3D_ATTR_DISPOFF);
    HuPrcVSleep();
    HuPrcEnd();
}

/* Present the final score order and winner reactions, then return the replay choice. */
s32 fn_1_4E70(void)
{
    s32 statusCounts[4];
    s32 scoreOrder[4];
    OM_CAMERA_VIEW openingCamera;
    OM_CAMERA_VIEW resultCamera;
    OM_CAMERA_VIEW *openingCameraView;
    OM_CAMERA_VIEW *resultCameraView;
    s16 choice;
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
            if (lbl_1_bss_4D8->playerRank[index] == 1) {
                statusCounts[activeCount]++;
            }
            index++;
        }
    }
    fn_1_8488();
    openingCamera = lbl_1_data_5F8;
    openingCameraView = &openingCamera;
    fn_1_A838(openingCameraView, 120, 4);
    fn_1_19DD4(1, 1, 0);
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        Hu3DModelAttrSet(lbl_1_bss_250, HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_24E, HU3D_ATTR_DISPOFF);
    }
    Hu3DModelAttrReset(lbl_1_bss_1D2, HU3D_ATTR_DISPOFF);
    for (index = 0, activeCount = 0; index < 4; index++) {
        if (lbl_1_bss_4D8->playerActive[index] != 0) {
            Hu3DModelPosSet(
                lbl_1_bss_2FC[index],
                lbl_1_data_698[lbl_1_bss_4D8->playerCount - 1]
                    .playerPositions[activeCount].x,
                lbl_1_data_698[lbl_1_bss_4D8->playerCount - 1]
                    .playerPositions[activeCount].y,
                lbl_1_data_698[lbl_1_bss_4D8->playerCount - 1]
                    .playerPositions[activeCount].z);
            activeCount++;
            Hu3DModelRotSet(lbl_1_bss_2FC[index], 0.0f, 0.0f, 0.0f);
        }
    }
    fn_1_1614C();
    fn_1_16620();
    fn_1_5EDC();
    lbl_1_bss_78 = HuAudFXPlay(MSM_SE_CMN_32);
    HuAudSStreamFadeOut(lbl_1_bss_4E8, 1200);
    fn_1_5FA0();
    resultCamera = lbl_1_data_630;
    resultCameraView = &resultCamera;
    fn_1_A838(resultCameraView, 120, 4);
    fn_1_6628();
    for (index = 0; index < 4; index++) {
        scoreOrder[index] = lbl_1_bss_4D8->playerScore[index];
    }
    fn_1_A97C(scoreOrder, 4);
    for (index = 0; index < 4; index++) {
        lbl_1_bss_7C[index] = -1;
    }
    for (index = 0, activeCount = 0; index < 4; index++) {
        if (lbl_1_bss_4D8->playerScore[index] == scoreOrder[0]) {
            lbl_1_bss_7C[activeCount] = index;
            activeCount++;
        }
    }
    for (index = 0, activeCount = 0; index < 4; index++) {
        if (lbl_1_bss_4D8->playerActive[index] != 0) {
            Hu3DModelPosSet(
                lbl_1_bss_1F0[index],
                lbl_1_data_698[lbl_1_bss_4D8->playerCount - 1]
                    .playerPositions[activeCount].x,
                lbl_1_data_698[lbl_1_bss_4D8->playerCount - 1]
                    .playerPositions[activeCount].y,
                lbl_1_data_698[lbl_1_bss_4D8->playerCount - 1]
                    .playerPositions[activeCount].z);
            activeCount++;
            Hu3DModelRotSet(lbl_1_bss_1F0[index], 0.0f, 0.0f, 0.0f);
        }
    }
    if (fn_1_632C() != 0) {
        fn_1_5FA0();
        fn_1_18168();
        fn_1_5B74();
        lbl_1_bss_4E8 = HuAudSStreamPlay(MSM_STREAM_RESULT);
    } else {
        if (fn_1_632C() == 0) {
            fn_1_16A24();
            fn_1_16ECC(lbl_1_bss_7C[0]);
        }
        HuAudFXPlay(MSM_SE_MICQUIZ_RESULTS_ANNOUNCE);
        if (lbl_1_bss_4D8->playerCount == statusCounts[0]) {
            Hu3DModelAttrReset(lbl_1_bss_1FA[lbl_1_bss_7C[0]], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(lbl_1_bss_1FA[lbl_1_bss_7C[1]], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(lbl_1_bss_1FA[lbl_1_bss_7C[2]], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(lbl_1_bss_1FA[lbl_1_bss_7C[3]], HU3D_ATTR_DISPOFF);
            fn_1_17B6C();
        } else {
            switch (statusCounts[0]) {
            case 1:
                Hu3DModelAttrReset(lbl_1_bss_1FA[lbl_1_bss_7C[0]], HU3D_ATTR_DISPOFF);
                fn_1_171CC(lbl_1_bss_7C[0], lbl_1_bss_7C[1], lbl_1_bss_7C[2], lbl_1_bss_7C[3]);
                break;
            case 2:
                Hu3DModelAttrReset(lbl_1_bss_1FA[lbl_1_bss_7C[0]], HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(lbl_1_bss_1FA[lbl_1_bss_7C[1]], HU3D_ATTR_DISPOFF);
                fn_1_1745C(lbl_1_bss_7C[0], lbl_1_bss_7C[1], lbl_1_bss_7C[2], lbl_1_bss_7C[3]);
                break;
            case 3:
                Hu3DModelAttrReset(lbl_1_bss_1FA[lbl_1_bss_7C[0]], HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(lbl_1_bss_1FA[lbl_1_bss_7C[1]], HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(lbl_1_bss_1FA[lbl_1_bss_7C[2]], HU3D_ATTR_DISPOFF);
                fn_1_1778C(lbl_1_bss_7C[0], lbl_1_bss_7C[1], lbl_1_bss_7C[2], lbl_1_bss_7C[3]);
                break;
            }
        }
        HuAudFXPlay(MSM_SE_CMN_33);
        lbl_1_bss_74 = HuAudJinglePlay(MSM_STREAM_MICQUIZ_WIN_JINGLE);
        do {
            HuPrcVSleep();
        } while (HuAudSStreamStatGet(lbl_1_bss_74) != 0);
        lbl_1_bss_74 = 0;
        for (index = 0; index < 4; index++) {
            Hu3DModelAttrSet(lbl_1_bss_1FA[lbl_1_bss_7C[index]], HU3D_ATTR_DISPOFF);
        }
        lbl_1_bss_4E8 = HuAudSStreamPlay(MSM_STREAM_MICQUIZ_RESULTS);
        Hu3DModelTPLvlSet(lbl_1_bss_1D0, 0.0f);
        Hu3DModelAttrReset(lbl_1_bss_1D0, HU3D_ATTR_DISPOFF);
        angle = 0.0;
        for (index = 0; index < 10; index++) {
            HuPrcVSleep();
            transparency = (f32)sin((3.141592653589793 * angle) / 180.0);
            Hu3DModelTPLvlSet(lbl_1_bss_1D0, transparency);
            angle += 9.0f;
        }
        Hu3DModelAttrSet(lbl_1_bss_1D2, HU3D_ATTR_DISPOFF);
        angle = 90.0f;
        for (index = 0; index < 10; index++) {
            HuPrcVSleep();
            transparency = (f32)sin((3.141592653589793 * angle) / 180.0);
            Hu3DModelTPLvlSet(lbl_1_bss_1D0, transparency);
            angle += 9.0f;
        }
        Hu3DModelTPLvlSet(lbl_1_bss_1D0, 0.0f);
        Hu3DModelAttrSet(lbl_1_bss_1D0, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_1D6, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_1D4, HU3D_ATTR_DISPOFF);
        HuPrcChildCreate(fn_1_62C8, 100, 6144, 0, lbl_1_bss_288);
        HuPrcSleep(10);
        Hu3DModelAttrReset(lbl_1_bss_1D8, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_1D8, HU3D_MOTATTR_PAUSE);
        HuPrcSleep(45);
        fn_1_5B74();
        angle = 90.0f;
        for (index = 0; index < 20; index++) {
            HuPrcVSleep();
            transparency = (f32)sin((3.141592653589793 * angle) / 180.0);
            Hu3DModelTPLvlSet(lbl_1_bss_1D0, transparency);
            angle += 4.5f;
        }
        HuPrcChildCreate(fn_1_6514, 100, 4096, 0, lbl_1_bss_288);
        for (index = 0; index < 4; index++) {
            if (lbl_1_bss_7C[index] != -1) {
                fn_1_A678(lbl_1_bss_7C[index], 1, 8.0f, 0);
            }
        }
        HuPrcSleep(180);
        lastPlaceRank = fn_1_6478();
        if (lbl_1_bss_4D8->presenterLayout != 0 && fn_1_640C() == 0) {
            fn_1_186E8();
            HuPrcSleep(120);
            fn_1_18800();
            HuPrcSleep(120);
            for (index = 0; index < 4; index++) {
                if (lbl_1_bss_4D8->playerActive[index] != 0
                    && lbl_1_bss_4D8->playerRank[index] == lastPlaceRank) {
                    Hu3DModelAttrReset(lbl_1_bss_1F0[index], HU3D_ATTR_DISPOFF);
                    HuAudFXPlay(MSM_SE_MICQUIZ_PLAYER_CELEBRATE);
                    fn_1_A678(index, 3, (16.0f), 0);
                }
            }
            HuPrcSleep(93);
            for (index = 0; index < 4; index++) {
                if (lbl_1_bss_4D8->playerActive[index] != 0
                    && lbl_1_bss_4D8->playerRank[index] == lastPlaceRank) {
                    Hu3DModelAttrSet(lbl_1_bss_1F0[index], HU3D_ATTR_DISPOFF);
                    fn_1_A678(index, 6, (8.0f), HU3D_MOTATTR_LOOP);
                }
            }
        }
    }
    fn_1_17D68();
    choice = HuWinChoiceGet(lbl_1_bss_37A, -1);
    if (choice == 0) {
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
        if ((s32) lbl_1_bss_284 > 255) {
            lbl_1_bss_284 = 255;
        }
        lbl_1_bss_280 = (s32) ((f32) lbl_1_bss_280 + (2.0f * lbl_1_bss_274));
        if ((s32) lbl_1_bss_280 > 255) {
            lbl_1_bss_280 = 255;
        }
        lbl_1_bss_27C = (s32) ((f32) lbl_1_bss_27C + (2.0f * lbl_1_bss_274));
        if ((s32) lbl_1_bss_27C > 255) {
            lbl_1_bss_27C = 255;
        }
        lbl_1_bss_278 = (s32) ((f32) lbl_1_bss_278 + (2.0f * lbl_1_bss_274));
        if ((s32) lbl_1_bss_278 > 255) {
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
    lbl_1_bss_274 = (2.0f);
    lbl_1_bss_270 = (2.0f);
    lbl_1_bss_26C = (2.0f);
    lbl_1_bss_268 = (2.0f);
}

/* Fade the three scene lights toward black over 120 scheduler frames during a transition. */
void fn_1_5FA0(void) {
    s32 frame;

    frame = 0;
    while (frame < 120) {
        HuPrcVSleep();
        lbl_1_bss_284 = (s32) ((f32) lbl_1_bss_284 - lbl_1_bss_274);
        if ((s32) lbl_1_bss_284 < 0) {
            lbl_1_bss_284 = 0;
        }
        lbl_1_bss_280 = (s32) ((f32) lbl_1_bss_280 - lbl_1_bss_274);
        if ((s32) lbl_1_bss_280 < 0) {
            lbl_1_bss_280 = 0;
        }
        lbl_1_bss_27C = (s32) ((f32) lbl_1_bss_27C - lbl_1_bss_274);
        if ((s32) lbl_1_bss_27C < 0) {
            lbl_1_bss_27C = 0;
        }
        lbl_1_bss_278 = (s32) ((f32) lbl_1_bss_278 - lbl_1_bss_274);
        if ((s32) lbl_1_bss_278 < 0) {
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

    rotation = (0.0f);
    for (;;) {
        HuPrcVSleep();
        Hu3DModelRotSet(lbl_1_bss_1D8, (0.0f), rotation,
                        (0.0f));
        rotation += (0.6f);
    }
}

/* Report whether every active player has a zero score when choosing result effects. */
s32 fn_1_632C(void) {
    s32 index;

    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_4D8->playerActive[index] != 0
            && lbl_1_bss_4D8->playerScore[index] != 0) {
            return 0;
        }
    }
    return 1;
}

/* Report whether every active player has a nonzero score when choosing result effects. */
s32 fn_1_639C(void) {
    s32 index;

    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_4D8->playerActive[index] != 0
            && lbl_1_bss_4D8->playerScore[index] == 0) {
            return 0;
        }
    }
    return 1;
}

/* Report whether every active player shares first place during result presentation. */
s32 fn_1_640C(void) {
    s32 index;

    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_4D8->playerActive[index] != 0
            && lbl_1_bss_4D8->playerRank[index] != 1) {
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
        if (lbl_1_bss_4D8->playerActive[index] != 0
            && lbl_1_bss_4D8->playerRank[index] > maximum) {
            maximum = lbl_1_bss_4D8->playerRank[index];
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
                position.x = lbl_1_data_698[lbl_1_bss_4D8->playerCount - 1]
                                 .playerPositions[lbl_1_bss_7C[activePlayerCount]].x;
                position.y = 480.0f;
                position.z = 100.0f
                    + lbl_1_data_698[lbl_1_bss_4D8->playerCount - 1]
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

    Hu3DModelPosSet(lbl_1_bss_202, (0.0f), (0.0f),
                    (-350.0f));
    Hu3DModelScaleSet(lbl_1_bss_202, (0.0f), (1.0f),
                      (1.0f));
    Hu3DModelAttrReset(lbl_1_bss_202, HU3D_ATTR_DISPOFF);

    angle = (0.0f);
    sineValue = (0.0f);
    for (;;) {
        HuPrcVSleep();
        sineValue = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        Hu3DModelScaleSet(lbl_1_bss_202, sineValue, (1.0f),
                          (1.0f));
        angle += (3.0f);
        if (angle >= (90.0f)) {
            break;
        }
    }
    Hu3DModelScaleSet(lbl_1_bss_202, (1.0f), (1.0f),
                      (1.0f));

    angle = (0.0f);
    positionOffset = (0.0f);
    for (;;) {
        HuPrcVSleep();
        positionOffset = (f32)((220.0)
            * sin(((3.141592653589793) * (f64)angle) / (180.0)));
        Hu3DModelPosSet(lbl_1_bss_202, positionOffset, (0.0f),
                        (-350.0f));
        angle += (1.5f);
        if (angle >= (720.0f)) {
            break;
        }
    }

    angle = (90.0f);
    sineValue = (0.0f);
    if (fn_1_632C() != 0) {
        HuAudFXStop(lbl_1_bss_78);
        HuAudFXPlay(MSM_SE_MICQUIZ_RESULT_MARKER);
    }

    for (;;) {
        HuPrcVSleep();
        sineValue = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        Hu3DModelScaleSet(lbl_1_bss_202, sineValue, (1.0f),
                          (1.0f));
        angle += (3.0f);
        if (angle >= (180.0f)) {
            break;
        }
    }
    Hu3DModelScaleSet(lbl_1_bss_202, (0.0f), (0.0f),
                      (0.0f));
    Hu3DModelAttrSet(lbl_1_bss_202, HU3D_ATTR_DISPOFF);
}

/* Select the quiz presentation mode and show its introduction before play continues. */
void fn_1_69D0(u32 selection) {
    switch (selection) {
    case 0:
        fn_1_E7F4(0);
        fn_1_20B74(0);
        fn_1_20A50(lbl_1_data_23C[selection], 1, 0);
        fn_1_20C94();
        fn_1_20B24();
        break;
    case 1:
        fn_1_E7F4(1);
        fn_1_20B74(1);
        fn_1_20A50(lbl_1_data_23C[selection], 1, 1);
        fn_1_20C94();
        fn_1_20B24();
        break;
    case 2:
        fn_1_E7F4(2);
        fn_1_20B74(0);
        fn_1_20A50(lbl_1_data_23C[selection], 1, 0);
        fn_1_20C94();
        fn_1_20B24();
        break;
    case 3:
        fn_1_E7F4(3);
        fn_1_20B74(1);
        fn_1_20A50(lbl_1_data_23C[selection], 1, 1);
        fn_1_20C94();
        fn_1_20B24();
        break;
    case 4:
        fn_1_E7F4(4);
        fn_1_20B74(3);
        fn_1_20A50(lbl_1_data_23C[selection], 1, 3);
        fn_1_20C94();
        fn_1_20B24();
        break;
    case 5:
        fn_1_E7F4(5);
        fn_1_20B74(3);
        fn_1_20A50(lbl_1_data_23C[selection], 1, 3);
        fn_1_20C94();
        fn_1_20B24();
        break;
    case 6:
        fn_1_E7F4(6);
        fn_1_20B74(3);
        fn_1_20A50(lbl_1_data_23C[selection], 1, 3);
        fn_1_20C94();
        fn_1_20B24();
        break;
    case 7:
        fn_1_E7F4(7);
        lbl_1_bss_4D8->savedQuestionColumn = lbl_1_bss_4D8->questionColumn;
        lbl_1_bss_4D8->questionColumn = 5;
        fn_1_20B74(3);
        fn_1_20A50(lbl_1_data_23C[selection], 1, 3);
        fn_1_20C94();
        fn_1_20B24();
    }
}

/* Hide the presenters and active player display models while switching scenes. */
void fn_1_74C0(void) {
    s32 playerIndex;

    if (lbl_1_bss_4D8->presenterLayout == 0) {
        Hu3DModelAttrSet(lbl_1_bss_2A6[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_2A6[1], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_294, HU3D_ATTR_DISPOFF);
    }
    playerIndex = 0;
    while (playerIndex < 4) {
        Hu3DModelAttrSet(lbl_1_bss_214[playerIndex], HU3D_ATTR_DISPOFF);
        if (lbl_1_bss_4D8->playerActive[playerIndex] != 0) {
            Hu3DModelAttrSet(lbl_1_bss_2FC[playerIndex], HU3D_ATTR_DISPOFF);
        }
        playerIndex += 1;
    }
}

/* Show the presenters and active player display models after switching scenes. */
void fn_1_75A4(void) {
    s32 playerIndex;

    if (lbl_1_bss_4D8->presenterLayout == 0) {
        Hu3DModelAttrReset(lbl_1_bss_2A6[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_2A6[1], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_294, HU3D_ATTR_DISPOFF);
    }
    playerIndex = 0;
    while (playerIndex < 4) {
        Hu3DModelAttrReset(lbl_1_bss_214[playerIndex], HU3D_ATTR_DISPOFF);
        if (lbl_1_bss_4D8->playerActive[playerIndex] != 0) {
            Hu3DModelAttrReset(lbl_1_bss_2FC[playerIndex], HU3D_ATTR_DISPOFF);
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
    OM_CAMERA_VIEW *initialCameraView;
    OM_CAMERA_VIEW *playerCameraView;
    OM_CAMERA_VIEW *exitCameraView;
    OM_CAMERA_VIEW *questionCameraView;
    OM_CAMERA_VIEW *promptCameraView;
    OM_CAMERA_VIEW *resultCameraView;
    s32 playerIndex;
    OM_CAMERA_VIEW cameraViews[6];

    history = omOvlHisGet(0);
    if (lbl_1_bss_258 == 0) {
        lbl_1_bss_4E8 = HuAudSStreamPlay(MSM_STREAM_MICQUIZ_INTRODUCTION);
        fn_1_A790(0, 60);
        HuPrcSleep(10);
        if (omovlevtno == 0) {
            cameraViews[5] = lbl_1_data_5DC;
            initialCameraView = &cameraViews[5];
            fn_1_A838(initialCameraView, 120, 4);
        }
        Hu3DModelAttrSet(lbl_1_bss_238, HU3D_ATTR_DISPOFF);
        if (omovlevtno == 0) {
            fn_1_E91C();
        }
        if (omovlevtno == 0 && fn_1_EF58() == 0) {
                fn_1_A3B4(1, 0, (8.0f), HU3D_MOTATTR_LOOP);
                lbl_1_bss_4D8->pauseAllowed = 0;
                fn_1_1B9D4();
                lbl_1_bss_4D8->pauseAllowed = 1;
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
                fn_1_A790(0, 20);
                omOvlHisChg(0, DLL_micquizdll, 2, history->stat);
                lbl_1_bss_4D8->questionColumn = 6;
                lbl_1_bss_4D8->currentPlayer = 0;
                lbl_1_bss_4D8->questionEvent = 3;
                lbl_1_bss_4D8->compactPauseDisplay = 0;
                omOvlCallEx(DLL_micquizishidll, 1, 0, 0);
                for (;;) {
                    HuPrcVSleep();
                }
        }
        if ((omovlevtno == 2 || omovlevtno == 0) && fn_1_F084() == 0) {
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
                fn_1_A790(0, 20);
                omOvlHisChg(0, DLL_micquizdll, 3, history->stat);
                lbl_1_bss_4D8->questionColumn = 7;
                lbl_1_bss_4D8->currentPlayer = 0;
                lbl_1_bss_4D8->questionEvent = 8;
                omOvlCallEx(DLL_micquizishidll, 1, 0, 0);
                for (;;) {
                    HuPrcVSleep();
                }
        }

        if (lbl_1_bss_37A != -1) {
            HuWinExClose(lbl_1_bss_37A);
            lbl_1_bss_37A = -1;
        }
        omOvlHisChg(0, DLL_micquizdll, 1, history->stat);
        fn_1_A3B4(1, 0, (8.0f), HU3D_MOTATTR_LOOP);
        fn_1_F1B0();
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (lbl_1_bss_4D8->playerActive[playerIndex] != 0) {
                s32 modelIndex;

                cameraViews[4] = lbl_1_data_550[playerIndex];
                playerCameraView = &cameraViews[4];
                fn_1_A838(playerCameraView, 45, 4);
                for (modelIndex = 0; modelIndex < 2; modelIndex++) {
                    Hu3DModelPosSet(lbl_1_bss_2A6[modelIndex],
                                    lbl_1_data_7E4[modelIndex].x,
                                    lbl_1_data_7E4[modelIndex].y,
                                    lbl_1_data_7E4[modelIndex].z);
                    Hu3DModelRotSet(lbl_1_bss_2A6[modelIndex],
                                    lbl_1_data_7FC[modelIndex].x,
                                    lbl_1_data_7FC[modelIndex].y,
                                    lbl_1_data_7FC[modelIndex].z);
                }
                HuAudFXPlay(lbl_1_data_388[lbl_1_bss_4D8->playerCharacter[playerIndex]]);
                fn_1_3A8C(playerIndex, 1);
                fn_1_A678(playerIndex, 5, (8.0f), 0);
                fn_1_F3CC(playerIndex);
                HuPrcSleep(8);
                fn_1_A748(playerIndex);
                if (lbl_1_bss_37A != -1) {
                    HuWinExClose(lbl_1_bss_37A);
                    lbl_1_bss_37A = -1;
                }
                fn_1_A678(playerIndex, 0, (16.0f), HU3D_MOTATTR_LOOP);
                fn_1_3A8C(playerIndex, 0);
            }
        }
        if (lbl_1_bss_37A != -1) {
            HuWinExClose(lbl_1_bss_37A);
            lbl_1_bss_37A = -1;
        }
        {
            cameraViews[3] = lbl_1_data_5C0;
            exitCameraView = &cameraViews[3];
            fn_1_A838(exitCameraView, 60, 4);
        }
        Hu3DModelAttrSet(lbl_1_bss_21E, HU3D_MOTATTR_PAUSE);
        fn_1_F568();
        HuAudSStreamFadeOut(lbl_1_bss_4E8, 2000);
        fn_1_19D5C();
        lbl_1_bss_4D8->questionCount = 1;
        fn_1_3A8C(lbl_1_bss_4D8->currentPlayer, 2);
        {
            cameraViews[2] = lbl_1_data_534;
            questionCameraView = &cameraViews[2];
            fn_1_A838(questionCameraView, 90, 4);
        }
        fn_1_8520();
        fn_1_19DD4(0, 1, 1);
        lbl_1_bss_4E8 = HuAudSStreamPlay(MSM_STREAM_MICQUIZ_GRID_SELECTION);
        fn_1_F6B4();
        if (HuMCMicGet() == 1) {
            fn_1_24A54(lbl_1_bss_4D8->currentPlayer);
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
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        lbl_1_bss_4E8 = HuAudSStreamPlay(MSM_STREAM_MICQUIZ_GRID_SELECTION);
    } else {
        lbl_1_bss_4E8 = HuAudSStreamPlay(MSM_STREAM_MICQUIZ_ALTERNATE_GRID_SELECTION);
    }
    HuPrcSleep(10);
    if (fn_1_8590() == 0) {
        cameraViews[1] = lbl_1_data_5C0;
        promptCameraView = &cameraViews[1];
        fn_1_A838(promptCameraView, 60, 4);
    }
    fn_1_ABF4();
    fn_1_E80C();
    {
    lbl_1_bss_4D8->questionCount++;
    do {
        lbl_1_bss_4D8->currentPlayer++;
        if (lbl_1_bss_4D8->currentPlayer == 4) {
            lbl_1_bss_4D8->currentPlayer = 0;
        }
        currentPlayer = lbl_1_bss_4D8->currentPlayer;
    } while (lbl_1_bss_4D8->playerActive[currentPlayer] == 0);
    }
    fn_1_A898();
    if (lbl_1_bss_4D8->questionCount == lbl_1_bss_4D8->playerCount * 3 + 1) {
        fn_1_129D4();
    }
    if (lbl_1_bss_4D8->questionCount == lbl_1_bss_4D8->playerCount * 5) {
        HuAudFXPlay(MSM_SE_MICQUIZ_ROUND_SET_COMPLETE);
        fn_1_123A0();
        HuPrcSleep(120);
    }
    completionStep = lbl_1_bss_4D8->playerCount * 5 + 1;
    if (lbl_1_bss_4D8->questionCount == completionStep) {
        lbl_1_bss_4D8->pauseAllowed = 0;
        result = fn_1_4E70();
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        HuAudSStreamFadeOut(lbl_1_bss_4E8, 600);
        fn_1_A790(0, 20);
        omOvlHisChg(0, DLL_micquizdll, 0, history->stat);
        if (result != 0) {
            HuMCSessionClose();
            HuMCClose();
            fn_1_A308();
            omOvlReturnEx(1, 1);
        } else {
            if (HuMCMicGet() == 1) {
                HuMCSessionClose();
                HuMCClose();
            }
            fn_1_A308();
            omOvlReturnEx(0, 1);
            OSReport("          HEAP_HEAP Malloc Size %x\n",
                     HuMemUsedMallocSizeGet(HEAP_HEAP));
        }
        HuPrcEnd();
        for (;;) {
            HuPrcVSleep();
        }
    }
    if (lbl_1_bss_4D8->questionCount != lbl_1_bss_4D8->playerCount * 5) {
        if (lbl_1_bss_4D8->questionCount == lbl_1_bss_4D8->playerCount * 3 + 1 &&
            lbl_1_bss_4D8->presenterLayout != 0) {
            fn_1_119FC();
        } else {
            fn_1_1170C();
        }
    }
    {
        cameraViews[0] = lbl_1_data_534;
        resultCameraView = &cameraViews[0];
        fn_1_A838(resultCameraView, 90, 4);
    }
    fn_1_8520();
    if (HuMCMicGet() == 1) {
        fn_1_24A54(lbl_1_bss_4D8->currentPlayer);
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
    lbl_1_bss_4D8->pauseAllowed = 0;
    fn_1_AAFC();
    fn_1_FE40();
    HuAudSStreamFadeOut(lbl_1_bss_4E8, 600);
    fn_1_A570();
    if (HuMCMicGet() == 1) {
        HuMCClose();
    }
    omOvlHisChg(0, DLL_micquizdll, 1, history->stat);
    GameMesPauseEnable(0);
    _SetFlag(FLAG_MG_PAUSE_OFF);
    fn_1_A308();
    lbl_1_bss_4D8->compactPauseDisplay = 0;
    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        lbl_1_bss_4D8->responseRecord[playerIndex] = -1;
        (&lbl_1_bss_4D8->responseRecord[2])[playerIndex] = -1;
    }
    if (lbl_1_bss_4D8->questionColumn == 2) {
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
    omOvlCallEx((enum omOvl_e)lbl_1_data_894[lbl_1_bss_4D8->questionColumn], 1, 0, 0);
}

/* Particle emitter settings for the two directions of the score effect. */
HU3D_PARMAN_PARAM lbl_1_data_AEC = {
    60, 0, 1.600000023841858f, 50.0f, 7.0f, 0.0f, -0.05000000074505806f, 0.0f,
    2.0f, 1.0f, 5.0f, 1.034999966621399f, 2, 255, 128, 128,
    0, 255, 64, 32, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 255, 255, 255,
    255, 255, 255, 64, 255, 0, 0, 0,
    0, 0, 0, 0, 0
};
HU3D_PARMAN_PARAM lbl_1_data_B3C = {
    60, 0, 1.600000023841858f, 50.0f, 7.0f, 0.0f, 0.05000000074505806f, 0.0f,
    2.0f, 1.0f, 30.0f, 0.9800000190734863f, 2, 255, 255, 255,
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

    if (lbl_1_bss_4D8->presenterLayout != 0) {
        Hu3DModelAttrSet(lbl_1_bss_294, HU3D_ATTR_DISPOFF);
    } else {
        presenterIndex = 0;
        while (presenterIndex < 2) {
            Hu3DModelAttrSet(lbl_1_bss_2A6[presenterIndex], HU3D_ATTR_DISPOFF);
            presenterIndex += 1;
        }
    }
    Hu3DModelAttrSet(lbl_1_bss_222, HU3D_ATTR_DISPOFF);
}

/* Show the current presenter layout and its companion model for the results scene. */
void fn_1_8488(void) {
    s32 presenterIndex;

    if (lbl_1_bss_4D8->presenterLayout != 0) {
        Hu3DModelAttrReset(lbl_1_bss_294, HU3D_ATTR_DISPOFF);
    } else {
        presenterIndex = 0;
        while (presenterIndex < 2) {
            Hu3DModelAttrReset(lbl_1_bss_2A6[presenterIndex], HU3D_ATTR_DISPOFF);
            presenterIndex += 1;
        }
    }
    Hu3DModelAttrReset(lbl_1_bss_222, HU3D_ATTR_DISPOFF);
}

/* Hide each active player model before leaving the selection scene. */
void fn_1_8520(void) {
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 4) {
        if (lbl_1_bss_4D8->playerActive[playerIndex] != 0) {
            Hu3DModelAttrSet(lbl_1_bss_2FC[playerIndex], HU3D_ATTR_DISPOFF);
        }
        playerIndex += 1;
    }
}

/* Present the selected quiz outcome with the appropriate camera, message, and player reactions. */
s32 fn_1_8590(void) {
    OM_CAMERA_VIEW firstBranchCamera;
    OM_CAMERA_VIEW phaseZeroFixedCamera;
    OM_CAMERA_VIEW phaseZeroModeSixCamera;
    OM_CAMERA_VIEW phaseZeroOtherCamera;
    OM_CAMERA_VIEW stateZeroCamera;
    OM_CAMERA_VIEW stateTwoCamera;
    OM_CAMERA_VIEW phaseOneCamera;
    OM_CAMERA_VIEW phaseTwoCamera;
    s32 phaseThreeMode;
    OM_CAMERA_VIEW *firstBranchView;
    s32 phaseZeroMode;
    OM_CAMERA_VIEW *phaseZeroFixedView;
    OM_CAMERA_VIEW *phaseZeroModeSixView;
    OM_CAMERA_VIEW *phaseZeroOtherView;
    s32 phaseZeroModeAfterMotion;
    OM_CAMERA_VIEW *stateZeroView;
    OM_CAMERA_VIEW *stateTwoView;
    OM_CAMERA_VIEW *phaseOneView;
    OM_CAMERA_VIEW *phaseTwoView;
    HUPROCESS *process;
    s32 result;

    process = 0;
    result = 0;
    if (lbl_1_bss_4D8->responseRecord[0] == 1
        && ((phaseThreeMode = lbl_1_bss_4D8->questionEvent), phaseThreeMode == 3)) {
        fn_1_DF9C(0);
        HuPrcSleep(30);
        firstBranchCamera = lbl_1_data_550[lbl_1_bss_4D8->responseRecord[2]];
        firstBranchView = (OM_CAMERA_VIEW *)&firstBranchCamera;
        fn_1_A838(firstBranchView, 60, 4);
        result = 0;
        if (lbl_1_bss_4D8->presenterLayout != 0) {
            process = HuPrcCreate(fn_1_9860, 100, 6144, 0);
            process->property = (void *)lbl_1_bss_4D8->responseRecord[2];
            fn_1_1153C();
            HuPrcSleep(185);
            fn_1_A678(lbl_1_bss_4D8->responseRecord[2], 0, (16.0f), HU3D_MOTATTR_LOOP);
        } else {
            HuAudFXPlay(
                lbl_1_data_3E0[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_4D8->responseRecord[2]]]);
            fn_1_A678(lbl_1_bss_4D8->responseRecord[2], 8, (8.0f), 0);
            HuPrcSleep(8);
            fn_1_A748(lbl_1_bss_4D8->responseRecord[2]);
            fn_1_1065C(lbl_1_bss_4D8->responseRecord[2]);
            HuPrcSleep(120);
            fn_1_A678(lbl_1_bss_4D8->responseRecord[2], 0, (16.0f), HU3D_MOTATTR_LOOP);
        }
    } else if (lbl_1_bss_4D8->responseRecord[0] == 1 && lbl_1_bss_4D8->responseRecord[1] == 0) {
        fn_1_DF9C(1);
        HuPrcSleep(30);
        if (lbl_1_bss_4D8->presenterLayout != 0) {
            phaseZeroMode = lbl_1_bss_4D8->questionEvent;
            if (phaseZeroMode != 6) {
                phaseZeroFixedCamera = lbl_1_data_5C0;
                phaseZeroFixedView = &phaseZeroFixedCamera;
                fn_1_A838(phaseZeroFixedView, 60, 4);
                result = 1;
            } else {
                phaseZeroModeSixCamera = lbl_1_data_550[lbl_1_bss_4D8->responseRecord[3]];
                phaseZeroModeSixView = (OM_CAMERA_VIEW *)&phaseZeroModeSixCamera;
                fn_1_A838(phaseZeroModeSixView, 60, 4);
                result = 0;
            }
        } else {
            phaseZeroOtherCamera = lbl_1_data_550[lbl_1_bss_4D8->responseRecord[3]];
            phaseZeroOtherView = (OM_CAMERA_VIEW *)&phaseZeroOtherCamera;
            fn_1_A838(phaseZeroOtherView, 60, 4);
            result = 0;
        }
        fn_1_3A8C(lbl_1_bss_4D8->responseRecord[3], 2);
        fn_1_9284(1);
        if (lbl_1_bss_4D8->presenterLayout != 0 &&
            ((phaseZeroModeAfterMotion = lbl_1_bss_4D8->questionEvent),
             phaseZeroModeAfterMotion != 6)) {
            process = HuPrcCreate(fn_1_9860, 100, 6144, 0);
            process->property = (void *)lbl_1_bss_4D8->responseRecord[2];
            fn_1_112B0();
            HuPrcSleep(185);
            fn_1_A678(lbl_1_bss_4D8->responseRecord[2], 0, (16.0f), HU3D_MOTATTR_LOOP);
        }
    } else if (lbl_1_bss_4D8->responseRecord[0] == 0) {
        fn_1_DF9C(1);
        HuPrcSleep(30);
        stateZeroCamera = lbl_1_data_550[lbl_1_bss_4D8->responseRecord[2]];
        stateZeroView = (OM_CAMERA_VIEW *)&stateZeroCamera;
        fn_1_A838(stateZeroView, 60, 4);
        result = 0;
        fn_1_3A8C(lbl_1_bss_4D8->responseRecord[2], 2);
        fn_1_9284(0);
    } else if (lbl_1_bss_4D8->responseRecord[0] == 2) {
        fn_1_DF9C(0);
        HuPrcSleep(30);
        stateTwoCamera = lbl_1_data_550[lbl_1_bss_4D8->responseRecord[2]];
        stateTwoView = (OM_CAMERA_VIEW *)&stateTwoCamera;
        fn_1_A838(stateTwoView, 60, 4);
        result = 0;
        if (lbl_1_bss_4D8->presenterLayout != 0) {
            process = HuPrcCreate(fn_1_9860, 100, 6144, 0);
            process->property = (void *)lbl_1_bss_4D8->responseRecord[2];
            fn_1_11424();
            HuPrcSleep(185);
            fn_1_A678(lbl_1_bss_4D8->responseRecord[2], 0, (16.0f), HU3D_MOTATTR_LOOP);
        } else {
            HuAudFXPlay(
                lbl_1_data_3E0[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_4D8->responseRecord[2]]]);
            fn_1_A678(lbl_1_bss_4D8->responseRecord[2], 8, (8.0f), 0);
            HuPrcSleep(8);
            fn_1_A748(lbl_1_bss_4D8->responseRecord[2]);
            fn_1_1042C();
            HuPrcSleep(120);
            fn_1_A678(lbl_1_bss_4D8->responseRecord[2], 0, (16.0f), HU3D_MOTATTR_LOOP);
        }
    } else if (lbl_1_bss_4D8->responseRecord[0] == 1 && lbl_1_bss_4D8->responseRecord[1] == 1) {
        fn_1_DF9C(0);
        HuPrcSleep(30);
        phaseOneCamera = lbl_1_data_5C0;
        phaseOneView = &phaseOneCamera;
        fn_1_A838(phaseOneView, 60, 4);
        result = 1;
        if (lbl_1_bss_4D8->presenterLayout != 0) {
            process = HuPrcCreate(fn_1_9860, 100, 6144, 0);
            process->property = (void *)lbl_1_bss_4D8->responseRecord[2];
            process = HuPrcCreate(fn_1_9860, 100, 6144, 0);
            process->property = (void *)lbl_1_bss_4D8->responseRecord[3];
            fn_1_1153C();
            HuPrcSleep(185);
            fn_1_A678(lbl_1_bss_4D8->responseRecord[2], 0, (16.0f), HU3D_MOTATTR_LOOP);
            fn_1_A678(lbl_1_bss_4D8->responseRecord[3], 0, (16.0f), HU3D_MOTATTR_LOOP);
        } else {
            HuAudFXPlay(
                lbl_1_data_3E0[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_4D8->responseRecord[2]]]);
            fn_1_A678(lbl_1_bss_4D8->responseRecord[2], 8, (8.0f), 0);
            HuAudFXPlay(
                lbl_1_data_3E0[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_4D8->responseRecord[3]]]);
            fn_1_A678(lbl_1_bss_4D8->responseRecord[3], 8, (8.0f), 0);
            HuPrcSleep(8);
            fn_1_A748(lbl_1_bss_4D8->responseRecord[2]);
            fn_1_1065C(lbl_1_bss_4D8->responseRecord[2]);
            fn_1_10934(lbl_1_bss_4D8->responseRecord[3]);
            fn_1_A678(lbl_1_bss_4D8->responseRecord[2], 0, (16.0f), HU3D_MOTATTR_LOOP);
            fn_1_A678(lbl_1_bss_4D8->responseRecord[3], 0, (16.0f), HU3D_MOTATTR_LOOP);
        }
    } else if (lbl_1_bss_4D8->responseRecord[0] == 1 && lbl_1_bss_4D8->responseRecord[1] == 2) {
        fn_1_DF9C(0);
        HuPrcSleep(30);
        phaseTwoCamera = lbl_1_data_550[lbl_1_bss_4D8->responseRecord[2]];
        phaseTwoView = (OM_CAMERA_VIEW *)&phaseTwoCamera;
        fn_1_A838(phaseTwoView, 60, 4);
        result = 0;
        if (lbl_1_bss_4D8->presenterLayout != 0) {
            process = HuPrcCreate(fn_1_9860, 100, 6144, 0);
            process->property = (void *)lbl_1_bss_4D8->responseRecord[2];
            fn_1_11424();
            HuPrcSleep(185);
            fn_1_A678(lbl_1_bss_4D8->responseRecord[2], 0, (16.0f), HU3D_MOTATTR_LOOP);
        } else {
            HuAudFXPlay(
                lbl_1_data_3E0[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_4D8->responseRecord[2]]]);
            fn_1_A678(lbl_1_bss_4D8->responseRecord[2], 8, (8.0f), 0);
            HuPrcSleep(8);
            fn_1_10E28();
            HuPrcSleep(120);
            fn_1_A678(lbl_1_bss_4D8->responseRecord[2], 0, (16.0f), HU3D_MOTATTR_LOOP);
        }
    }
    return result;
}

/* Apply the current selection outcome to a player, including score and model feedback. */
void fn_1_9284(s32 objectOffset) {
    s32 firstMode;
    s32 secondMode;
    s32 thirdMode;
    s32 lastMode;
    s32 modelIndex;

    firstMode = lbl_1_bss_4D8->questionEvent;
    if (firstMode == 0) {
        fn_1_1264C(lbl_1_bss_4D8->roundPoints, lbl_1_bss_4D8->roundPoints * 2);
        fn_1_9934(0, (&lbl_1_bss_4D8->responseRecord[2])[objectOffset]);
        HuAudFXPlay(lbl_1_data_3B4[lbl_1_bss_4D8->playerCharacter[
            (&lbl_1_bss_4D8->responseRecord[2])[objectOffset]]]);
        CharMotionVoiceOnSet(
            lbl_1_bss_4D8->playerCharacter[(&lbl_1_bss_4D8->responseRecord[2])[objectOffset]],
            (s16)lbl_1_data_420[7], 0);
        fn_1_A678((&lbl_1_bss_4D8->responseRecord[2])[objectOffset], 7,
                  (8.0f), 0);
        fn_1_A748((&lbl_1_bss_4D8->responseRecord[2])[objectOffset]);
        fn_1_A678((&lbl_1_bss_4D8->responseRecord[2])[objectOffset], 0,
                  (8.0f), HU3D_MOTATTR_LOOP);
        fn_1_A088(lbl_1_bss_4D8->answeringPlayer, lbl_1_bss_4D8->roundPoints * 2);
    } else {
        secondMode = lbl_1_bss_4D8->questionEvent;
        if (secondMode == 1) {
        fn_1_12810(lbl_1_bss_4D8->roundPoints, lbl_1_bss_4D8->roundPoints * 3);
        fn_1_9934(1, (&lbl_1_bss_4D8->responseRecord[2])[objectOffset]);
        HuAudFXPlay(lbl_1_data_3B4[lbl_1_bss_4D8->playerCharacter[
            (&lbl_1_bss_4D8->responseRecord[2])[objectOffset]]]);
        CharMotionVoiceOnSet(
            lbl_1_bss_4D8->playerCharacter[(&lbl_1_bss_4D8->responseRecord[2])[objectOffset]],
            (s16)lbl_1_data_420[7], 0);
        fn_1_A678((&lbl_1_bss_4D8->responseRecord[2])[objectOffset], 7,
                  (8.0f), 0);
        fn_1_A748((&lbl_1_bss_4D8->responseRecord[2])[objectOffset]);
        fn_1_A678((&lbl_1_bss_4D8->responseRecord[2])[objectOffset], 0,
                  (8.0f), HU3D_MOTATTR_LOOP);
            fn_1_A088(lbl_1_bss_4D8->answeringPlayer, lbl_1_bss_4D8->roundPoints * 3);
        } else {
            thirdMode = lbl_1_bss_4D8->questionEvent;
            if (thirdMode == 6) {
                fn_1_9934(6, (&lbl_1_bss_4D8->responseRecord[2])[objectOffset]);
                HuPrcSleep(60);
            } else {
                fn_1_100BC();
                HuAudFXPlay(lbl_1_data_3B4[lbl_1_bss_4D8->playerCharacter[
                    (&lbl_1_bss_4D8->responseRecord[2])[objectOffset]]]);
                CharMotionVoiceOnSet(
                    lbl_1_bss_4D8
                        ->playerCharacter[(&lbl_1_bss_4D8->responseRecord[2])[objectOffset]],
                    (s16) lbl_1_data_420[7], 0);
                fn_1_A678((&lbl_1_bss_4D8->responseRecord[2])[objectOffset], 7,
                          (8.0f), 0);
                fn_1_A748((&lbl_1_bss_4D8->responseRecord[2])[objectOffset]);
                fn_1_A678((&lbl_1_bss_4D8->responseRecord[2])[objectOffset], 0,
                          (8.0f), HU3D_MOTATTR_LOOP);
                fn_1_A088(lbl_1_bss_4D8->answeringPlayer, lbl_1_bss_4D8->roundPoints);
            }
        }
    }
    lastMode = lbl_1_bss_4D8->questionEvent;
    if (lastMode != 6) {
        modelIndex = (&lbl_1_bss_4D8->responseRecord[2])[objectOffset];
        Hu3DModelKill(lbl_1_bss_20C[modelIndex]);
        lbl_1_bss_20C[modelIndex] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 87), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_20C[modelIndex], HU3D_MOTATTR_LOOP);
        Hu3DModelCameraSet(lbl_1_bss_20C[modelIndex], 4);
        Hu3DModelPosSet(lbl_1_bss_20C[modelIndex], lbl_1_data_83C[modelIndex][0],
                        lbl_1_data_83C[modelIndex][1], lbl_1_data_83C[modelIndex][2]);
    }
}

/* Play one player reaction in a child process, then restore that player's looping motion. */
void fn_1_9860(void) {
    HUPROCESS *process;
    s32 playerNo;

    process = HuPrcCurrentGet();
    playerNo = (s32)process->property;
    Hu3DModelAttrReset(lbl_1_bss_1F0[playerNo], HU3D_ATTR_DISPOFF);
    HuAudFXPlay(MSM_SE_MICQUIZ_PLAYER_REACTION);
    fn_1_A678(playerNo, 3, (16.0f), 0);
    HuPrcSleep(16);
    do {
        HuPrcVSleep();
    } while (Hu3DMotionEndCheck(lbl_1_bss_2FC[playerNo]) == 0);
    Hu3DModelAttrSet(lbl_1_bss_1F0[playerNo], HU3D_ATTR_DISPOFF);
    fn_1_A678(playerNo, 6, (16.0f), HU3D_MOTATTR_LOOP);
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

    modelId = Hu3DModelLink(lbl_1_bss_198[0].model[0]);
    colorAnimation = Hu3DAnimCreate(lbl_1_bss_68, modelId, "vq_color");
    Hu3DAnimBankSet(colorAnimation, playerIndex);
    eventAnimation = Hu3DAnimCreate(lbl_1_bss_64, modelId, "vq_ivent");
    Hu3DAnimBankSet(eventAnimation, mode + 1);

    scale = 4.0f;
    alpha = 0.0f;
    rotation = 0.0;
    angle = lbl_1_data_550[playerIndex].rot.y;
    sineOffset = (f32)(200.0 * sin((3.141592653589793 * angle) / 180.0));
    cosineOffset = (f32)(200.0 * cos((3.141592653589793 * angle) / 180.0));
    position.x = sineOffset + lbl_1_data_550[playerIndex].center.x;
    position.y = 50.0f + lbl_1_data_550[playerIndex].center.y;
    position.z = cosineOffset + lbl_1_data_550[playerIndex].center.z;
    spawnPosition = position;

    Hu3DModelPosSet(modelId, position.x, position.y, position.z);
    Hu3DModelRotSet(modelId, 0.0f, 180.0f + angle, 0.0f);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    Hu3DModelCameraSet(modelId, 4);
    Hu3DModelLayerSet(modelId, 2);
    Hu3DModelScaleSet(modelId, 4.0f, 4.0f, 1.0f);
    Hu3DModelTPLvlSet(modelId, 0.0f);

    if (mode == 0 || mode == 1) {
        HuAudFXPlay(MSM_SE_MICQUIZ_EVENT_PANEL_APPEAR);
    } else if (mode == 6) {
        HuAudFXPlay(MSM_SE_MICQUIZ_EVENT_PANEL_APPEAR);
    }

    for (;;) {
        HuPrcVSleep();
        scale = (f32)(0.5 + 3.5 * (1.0 - sin((3.141592653589793 * rotation) / 180.0)));
        rotation += 2.0f;
        Hu3DModelScaleSet(modelId, scale, scale, 1.0f);
        if (rotation >= 90.0f) {
            Hu3DModelScaleSet(modelId, 0.5f, 0.5f, 0.5f);
            break;
        }
        Hu3DModelTPLvlSet(modelId, alpha);
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
    Hu3DAnimKill(colorAnimation);
    Hu3DAnimKill(eventAnimation);
    Hu3DModelKill(modelId);
    HuPrcVSleep(120);
}

/* Start an asynchronous score increase, packing the player and point count into process data. */
void fn_1_9DB8(s32 playerNo, s32 points) {
    HUPROCESS *process = NULL;

    process = HuPrcChildCreate(fn_1_9E28, 100, 6144, 0, lbl_1_bss_288);
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

    process = HuPrcCurrentGet();
    property = (u32)process->property;
    playerNo = (u8)(property >> 16);
    requested = (u16)property;
    sleepFrames = 1;
    initialValue = lbl_1_bss_4D8->playerScore[playerNo];
    for (;;) {
        HuPrcSleep(sleepFrames);
        if (lbl_1_bss_4D8->playerScore[playerNo] == initialValue + requested) {
            break;
        }
        lbl_1_bss_4D8->playerScore[playerNo]++;
        if (lbl_1_bss_4D8->playerScore[playerNo] == 999) {
            break;
        }
        HuAudFXPlay(MSM_SE_MICQUIZ_SCORE_TICK);
    }
    HuAudFXPlay(lbl_1_data_39E[lbl_1_bss_4D8->playerCharacter[playerNo]]);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Start an asynchronous score decrease, packing the player and point count into process data. */
void fn_1_9F38(s32 playerNo, s32 points) {
    HUPROCESS *process = NULL;

    process = HuPrcChildCreate(fn_1_9FA8, 100, 6144, 0, lbl_1_bss_288);
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

    process = HuPrcCurrentGet();
    property = (u32)process->property;
    playerNo = (u8)(property >> 16);
    requested = (u16)property;
    sleepFrames = 1;
    initialValue = lbl_1_bss_4D8->playerScore[playerNo];
    for (;;) {
        HuPrcSleep(sleepFrames);
        if (lbl_1_bss_4D8->playerScore[playerNo] == initialValue - requested) {
            break;
        }
        lbl_1_bss_4D8->playerScore[playerNo]--;
        if (lbl_1_bss_4D8->playerScore[playerNo] == 0) {
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
    oldValue = lbl_1_bss_4D8->playerScore[playerNo];
    if (increment <= 50) {
        sleepFrames = 5;
    } else if (increment <= 100) {
        sleepFrames = 3;
    } else if (increment <= 150) {
        sleepFrames = 1;
    }
    for (;;) {
        HuPrcSleep(sleepFrames);
        if (lbl_1_bss_4D8->playerScore[playerNo] == oldValue + increment) {
            return;
        }
        lbl_1_bss_4D8->playerScore[playerNo]++;
        if (lbl_1_bss_4D8->playerScore[playerNo] >= 999) {
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
    oldValue = lbl_1_bss_4D8->playerScore[playerNo];
    if (decrement >= -50) {
        sleepFrames = 5;
    } else if (decrement >= -100) {
        sleepFrames = 3;
    } else if (decrement >= -150) {
        sleepFrames = 1;
    }
    for (;;) {
        HuPrcSleep(sleepFrames);
        if (lbl_1_bss_4D8->playerScore[playerNo] == oldValue - decrement) {
            return;
        }
        lbl_1_bss_4D8->playerScore[playerNo]--;
        if (lbl_1_bss_4D8->playerScore[playerNo] == 0) {
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
        omOvlReturnEx(1, 1);
    }
}

/* Release scene models and the two camera capture animations when leaving the overlay. */
void fn_1_A308(void) {
    Hu3DAllKill();
    HuSprAnimKill(lbl_1_bss_30);
    HuSprAnimKill(lbl_1_bss_34);
}

/* Watch for a system exit request each object update and begin the outgoing wipe. */
void fn_1_A348(OMOBJ *obj) {
    if (omSysExitReq != 0) {
        HuAudSStreamFadeOut(lbl_1_bss_4E8, 600);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        obj->objFunc = (OMOBJ_FUNC)fn_1_A290;
    }
}

/* Change a two-presenter model's motion only when the requested motion differs. */
void fn_1_A3B4(s32 presenterNo, s32 motionNo, f32 blendFrames, u32 attr) {
    HU3D_MOTIONID currentMotion;

    currentMotion = Hu3DMotionIDGet(lbl_1_bss_2A6[presenterNo]);
    if (currentMotion != lbl_1_bss_296[presenterNo][motionNo]) {
        Hu3DMotionShiftSet(lbl_1_bss_2A6[presenterNo], lbl_1_bss_296[presenterNo][motionNo],
                           (0.0f), blendFrames, attr);
    }
}

/* Yield until the selected two-presenter model finishes its current motion. */
void fn_1_A484(s32 presenterNo) {
    for (;;) {
        HuPrcVSleep();
        if (Hu3DMotionEndCheck(lbl_1_bss_2A6[presenterNo]) != 0) {
            break;
        }
    }
}

/* Change the single-presenter model's motion only when the requested motion differs. */
void fn_1_A4CC(s32 motionNo, f32 blendFrames, u32 attr) {
    HU3D_MOTIONID currentMotion;

    currentMotion = Hu3DMotionIDGet(lbl_1_bss_294);
    if (currentMotion != lbl_1_bss_28C[motionNo]) {
        Hu3DMotionShiftSet(lbl_1_bss_294, lbl_1_bss_28C[motionNo],
                           (0.0f), blendFrames, attr);
    }
}

/* Wait for any existing wipe, then hide the scene with an outgoing wipe. */
void fn_1_A570(void) {
    f32 bounds[3];

    bounds[0] = (-130.0f);
    bounds[1] = (920.0f);
    bounds[2] = (-600.0f);
    bounds[0] += (108.0f) * lbl_1_bss_4D8->questionLevel;
    bounds[1] -= (108.0f) * lbl_1_bss_4D8->questionColumn;
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    fn_1_A7E4(0, 20);
}

/* Change an active player character's motion only when the requested motion differs. */
void fn_1_A678(s32 playerNo, s32 motionNo, f32 blendFrames, u32 attr) {
    HU3D_MOTIONID currentMotion;

    currentMotion = Hu3DMotionIDGet(lbl_1_bss_2FC[playerNo]);
    if (currentMotion != lbl_1_bss_2AC[playerNo][motionNo]) {
        CharMotionShiftSet(lbl_1_bss_2F4[playerNo], lbl_1_bss_2AC[playerNo][motionNo],
                           (0.0f), blendFrames, attr);
    }
}

/* Yield until the selected player character finishes its current motion. */
void fn_1_A748(s32 playerNo) {
    for (;;) {
        HuPrcVSleep();
        if (CharMotionEndCheck(lbl_1_bss_2F4[playerNo]) != 0) {
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
void fn_1_A838(OM_CAMERA_VIEW *cameraView, s32 duration, u32 cameraMask) {
    s32 frame;

    omCameraViewMoveSimpleMulti(cameraMask, cameraView, duration);
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
        orderedValues[playerNo] = lbl_1_bss_4D8->playerScore[playerNo];
    }
    fn_1_A97C(orderedValues, 4);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        for (rankIndex = 0; rankIndex < 4; rankIndex++) {
            if (lbl_1_bss_4D8->playerScore[playerNo] == orderedValues[rankIndex]) {
                lbl_1_bss_4D8->playerRank[playerNo] = rankIndex + 1;
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
    switch (lbl_1_bss_4D8->questionEvent) {
    case 2:
        lbl_1_bss_4D8->savedQuestionLevel = lbl_1_bss_4D8->questionLevel;
        lbl_1_bss_4D8->questionLevel--;
        if (lbl_1_bss_4D8->questionLevel == 0) {
            lbl_1_bss_4D8->questionLevel = 0;
        }
        break;
    case 5:
        lbl_1_bss_4D8->savedQuestionLevel = lbl_1_bss_4D8->questionLevel;
        lbl_1_bss_4D8->questionLevel++;
        if (lbl_1_bss_4D8->questionLevel >= 4) {
            lbl_1_bss_4D8->questionLevel = 4;
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
    playerNo = lbl_1_bss_4D8->answeringPlayer;
    if (fn_1_281D4() == 6 &&
        (lbl_1_bss_4D8->responseRecord[0] != 1 || lbl_1_bss_4D8->responseRecord[1] != 1)) {
        if (lbl_1_bss_4D8->responseRecord[0] == 2 || lbl_1_bss_4D8->responseRecord[1] == 2)
        {
            return;
        }
        fn_1_18B50();
        HuPrcSleep(120);
        if (HuMCMicGet() == 1)
        {
            fn_1_24A54(playerNo);
            wait_for_response : if (fn_1_BE70() == - 1) { goto wait_for_response; } fn_1_2D748(0);
            if (lbl_1_bss_8C == 0)
            {
                for (index = 0; index < 4; index++)
                {
                    lbl_1_bss_9C[index] = -1;
                    lbl_1_bss_90[index] = -1;
                }
                for (index = 0, outIndex = 0; index < 4; index++)
                {
                    if (index != playerNo && lbl_1_bss_4D8->playerActive[index] != 0)
                    {
                        lbl_1_bss_9C[outIndex] = index;
                        lbl_1_bss_90[outIndex] = lbl_1_bss_4D8->playerCharacter[index];
                        ++outIndex;
                    }
                }
                if (lbl_1_bss_4D8->playerCount == 2)
                {
                    pointDonor = lbl_1_bss_9C[0];
                }
                else
                {
                    randomIndex = frandmod(lbl_1_bss_4D8->playerCount - 1);
                    pointDonor = lbl_1_bss_9C[randomIndex];
                }
                fn_1_24D94();
                fn_1_19294(pointDonor);
                HuPrcSleep(120);
            }
            else
            {
                if ((s32)(lbl_1_bss_8C - 1) == lbl_1_bss_4D8->playerCharacter[playerNo])
                {
                    fn_1_2D748(0);
                    fn_1_18E98(lbl_1_bss_8C - 1);
                    HuPrcSleep(120);
                    fn_1_18B50();
                    fn_1_2D748(1);
                    goto wait_for_response;
                }
                else
                {
                    if (! (fn_1_BE04(lbl_1_bss_8C - 1)))
                    {
                        fn_1_2D748(0);
                        fn_1_18FEC(lbl_1_bss_8C - 1);
                        HuPrcSleep(120);
                        fn_1_18B50();
                        fn_1_2D748(1);
                        goto wait_for_response;
                    }
                    else
                    {
                        fn_1_24D94();
                        fn_1_19140(lbl_1_bss_8C - 1);
                        pointDonor = fn_1_BD9C(lbl_1_bss_8C - 1);
                        fn_1_24D94();
                    }
                }
            }
        }
        else
        {
            result = fn_1_196BC(lbl_1_bss_4D8->answeringPlayer);
            if (result == lbl_1_bss_4D8->playerCount - 1)
            {
                if (lbl_1_bss_4D8->playerCount == 2) {
                    pointDonor = lbl_1_bss_9C[0];
                } else {
                    randomIndex = frandmod(lbl_1_bss_4D8->playerCount - 1);
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
        Hu3DModelPosSet(lbl_1_bss_1EC[0], positionA.x, positionA.y, positionA.z);
        Hu3DModelPosSet(lbl_1_bss_1EC[1], positionB.x, positionB.y, positionB.z);
        Hu3DModelRotSet(lbl_1_bss_1EC[0], 0.0f, - 75.0f, 0.0f);
        Hu3DModelRotSet(lbl_1_bss_1EC[1], 0.0f, - 75.0f, 0.0f);
        Hu3DModelAttrReset(lbl_1_bss_1EC[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_1EC[1], HU3D_ATTR_DISPOFF);
        fallAngle = 0.0f;
        HuAudFXPlay(MSM_SE_MICQUIZ_RESULT_MODEL_DROP);
        do
        {
            HuPrcVSleep();
            offset = (f32)(200.0 * sin((3.141592653589793 * fallAngle) / 180.0));
            Hu3DModelPosSet(lbl_1_bss_1EC[0], positionA.x, positionA.y - offset, positionA.z);
            Hu3DModelPosSet(lbl_1_bss_1EC[1], positionB.x, positionB.y - offset, positionB.z);
            fallAngle += 2.25f;
        }
        while (! (fallAngle >= 90.0f));
        positionA.y -= offset;
        positionB.y -= offset;
        Hu3DModelPosSet(lbl_1_bss_1EC[0], positionA.x, positionA.y, positionA.z);
        Hu3DModelPosSet(lbl_1_bss_1EC[1], positionB.x, positionB.y, positionB.z);
        HuPrcSleep(30);
        effectPositionA = positionA;
        effectPositionB = positionB;
        effectPositionA.y += 20.0f;
        effectPositionB.y -= 120.0f;
        lbl_1_bss_1E4[0] = fn_1_C1DC(lbl_1_bss_1E8, 0, &effectPositionA);
        lbl_1_bss_1E4[1] = fn_1_C1DC(lbl_1_bss_1E8, 1, &effectPositionB);
        Hu3DParManAttrSet(lbl_1_bss_1E4[0], HU3D_PARMAN_ATTR_TIMEUP);
        Hu3DParManAttrSet(lbl_1_bss_1E4[1], HU3D_PARMAN_ATTR_TIMEUP);
        if (lbl_1_bss_4D8->playerScore[pointDonor] < lbl_1_bss_4D8->roundPoints) {
            score = lbl_1_bss_4D8->playerScore[pointDonor];
        } else {
            score = lbl_1_bss_4D8->roundPoints;
        }
        HuAudFXPlay(MSM_SE_MICQUIZ_POINT_CHANGE_EFFECT);
        Hu3DParManAttrReset(lbl_1_bss_1E4[1], HU3D_PARMAN_ATTR_TIMEUP);
        HuAudFXPlay((s32) lbl_1_data_3E0[lbl_1_bss_4D8->playerCharacter[pointDonor]]);
        fn_1_A678(pointDonor, 8, 8.0f, 0);
        fn_1_9F38(pointDonor, score);
        HuPrcSleep(80);
        Hu3DParManAttrSet(lbl_1_bss_1E4[1], HU3D_PARMAN_ATTR_TIMEUP);
        HuPrcSleep(40);
        HuAudFXPlay(MSM_SE_MICQUIZ_POINT_CHANGE_EFFECT);
        Hu3DParManAttrReset(lbl_1_bss_1E4[0], HU3D_PARMAN_ATTR_TIMEUP);
        HuPrcSleep(20);
        CharMotionVoiceOnSet(lbl_1_bss_4D8->playerCharacter[playerNo], lbl_1_data_420[5], 0);
        fn_1_A678(playerNo, 5, 8.0f, 0);
        fn_1_9DB8(playerNo, score);
        HuPrcSleep(60);
        Hu3DParManAttrSet(lbl_1_bss_1E4[0], HU3D_PARMAN_ATTR_TIMEUP);
        HuPrcSleep(100);
        fallAngle = 0.0f;
        HuAudFXPlay(MSM_SE_MICQUIZ_RESULT_MODEL_DROP);
        do
        {
            HuPrcVSleep();
            offset = (f32)(250.0 * sin((3.141592653589793 * fallAngle) / 180.0));
            Hu3DModelPosSet(lbl_1_bss_1EC[0], positionA.x, positionA.y + offset, positionA.z);
            Hu3DModelPosSet(lbl_1_bss_1EC[1], positionB.x, positionB.y + offset, positionB.z);
            fallAngle += 2.25f;
        }
        while (! (fallAngle >= 90.0f));
        Hu3DModelAttrSet(lbl_1_bss_1EC[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_1EC[1], HU3D_ATTR_DISPOFF);
        fn_1_3A8C(playerNo, 0);
        fn_1_1959C();
        fn_1_A678(playerNo, 0, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_A678(pointDonor, 0, 8.0f, HU3D_MOTATTR_LOOP);
        if (lbl_1_bss_37A != - 1)
        {
            HuWinExClose(lbl_1_bss_37A);
            lbl_1_bss_37A = - 1;
        }
        if (lbl_1_bss_4D8->responseRecord[0] == 1)
        {
            process = HuPrcCreate(fn_1_9860, 100U, 6144U, 0);
            process->property = (void *)(s32)lbl_1_bss_4D8->responseRecord[2];
            fn_1_112B0();
            HuPrcSleep(185);
            fn_1_A678(lbl_1_bss_4D8->responseRecord[2], 0, 16.0f, HU3D_MOTATTR_LOOP);
        }
    }
}

/* Find the active player using a character, or return the supplied character value if absent. */
s32 fn_1_BD9C(s32 requested) {
    s32 index;

    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_4D8->playerActive[index] != 0
            && lbl_1_bss_4D8->playerCharacter[index] == requested) {
            return index;
        }
    }
    return requested;
}

/* Test whether the requested character belongs to an active player. */
s32 fn_1_BE04(s32 requested) {
    s32 index;

    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_4D8->playerActive[index] != 0
            && lbl_1_bss_4D8->playerCharacter[index] == requested) {
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
    playerNo = fn_1_27FD8();
    lbl_1_bss_1C = 1;
    fn_1_2D748(1);
    process = HuPrcChildCreate(fn_1_C9B4, 100, 6144, 0, lbl_1_bss_288);
    HuMCSessionSet((s16)playerNo);
    HuMCSelWinContextSet(lbl_1_bss_AE, 0, lbl_1_bss_4D8->currentPlayer);
    HuMCSelWinChoiceGet();
    lbl_1_bss_4D8->pauseAllowed = 1;
    HuPrcKill(process);
    fn_1_2D748(0);
    lbl_1_bss_8 = (MicquizResponse *)HuMCCurResponseGet();
    HuMCSelWinContextKill();
    lbl_1_bss_8C = lbl_1_bss_8->recognizedWord;
    lbl_1_bss_1C = 0;

    firstParent = lbl_1_bss_288;
    if (lbl_1_data_208 != -1) {
        HuPrcChildCreate(fn_1_2FCF0, 100, 6144, 0, firstParent);
    }

    if (lbl_1_bss_8->result == -1 || lbl_1_bss_8->result == -2) {
        fn_1_2D748(0);
        fn_1_18C68();
        HuPrcSleep(120);
        fn_1_2D748(1);
        return -1;
    }
    if (lbl_1_bss_8->result == -4) {
        fn_1_2D748(0);
        fn_1_18D80();
        HuPrcSleep(120);
        fn_1_2D748(1);
        return -1;
    }

    if (HuMCUnkResponseCheck() != 0) {
        lbl_1_bss_4D8->pauseAllowed = 0;
        fn_1_2D748(1);
        process = HuPrcChildCreate(fn_1_C9B4, 100, 6144, 0, lbl_1_bss_288);
        lbl_1_bss_1C = 1;
        HuPrcVSleep();
        fn_1_2D748(1);
        secondParent = lbl_1_bss_288;
        if (lbl_1_data_208 != -1) {
            HuPrcChildCreate(fn_1_2FCF0, 100, 6144, 0, secondParent);
        }
        response = HuMCNewResponseGet(MESSNUM(MESS_MIC_RETYPING, 106));
        if (response == -1) {
            fn_1_2D748(0);
            HuPrcChildCreate(fn_1_2FD28, 100, 6144, 0, lbl_1_bss_548);
            lbl_1_bss_4D8->pauseAllowed = 1;
            return -1;
        }
        lbl_1_bss_8C = response;
        lbl_1_bss_1C = 1;
        HuPrcKill(process);
        lbl_1_bss_4D8->pauseAllowed = 1;
        fn_1_2D748(0);
    }

    thirdParent = lbl_1_bss_288;
    if (lbl_1_data_208 != -1) {
        HuPrcChildCreate(fn_1_2FCF0, 100, 6144, 0, thirdParent);
    }
    return 0;
}

/* Create a directional particle emitter at a supplied position for the score sequence. */
HU3D_PARMANID fn_1_C1DC(ANIMDATA *animation, s32 mode, HuVecF *position) {
    HU3D_PARMANID particleManager;
    HU3D_MODELID modelId;

    if (mode == 0) {
        particleManager = Hu3DParManCreate(animation, 100, &lbl_1_data_AEC);
    } else {
        particleManager = Hu3DParManCreate(animation, 100, &lbl_1_data_B3C);
    }
    Hu3DParManAttrSet(particleManager,
                      (HU3D_PARMAN_ATTR_RANDSPEED70 | HU3D_PARMAN_ATTR_RANDSCALE70 |
                       HU3D_PARMAN_ATTR_SCALEJITTER));
    if (mode == 0) {
        Hu3DParManRotSet(particleManager, 90.0f, 0.0f, 0.0f);
    } else {
        Hu3DParManRotSet(particleManager, -90.0f, 0.0f, 0.0f);
    }
    modelId = Hu3DParManModelIDGet(particleManager);
    if (mode == 0) {
        Hu3DParticleBlendModeSet(modelId, HU3D_PARTICLE_BLEND_ADDCOL);
    } else {
        Hu3DParticleBlendModeSet(modelId, HU3D_PARTICLE_BLEND_ADDCOL);
    }
    Hu3DModelLayerSet(modelId, 2);
    Hu3DModelCameraSet(modelId, 4);
    Hu3DParManPosSet(particleManager, position->x, position->y, position->z);
    return particleManager;
}

/* Transfer points between two players one at a time, stopping if the donor reaches zero. */
void fn_1_C320(s32 playerNo, s32 otherPlayerNo, s32 amount) {
    s32 startingValue;
    s32 sleepFrames;

    sleepFrames = 1;
    startingValue = lbl_1_bss_4D8->playerScore[playerNo];
    if (amount <= 50) {
        sleepFrames = 5;
    } else if (amount <= 100) {
        sleepFrames = 3;
    } else if (amount <= 150) {
        sleepFrames = 1;
    }
    while (1) {
        HuPrcSleep(sleepFrames);
        if (lbl_1_bss_4D8->playerScore[playerNo] == startingValue + amount) {
            break;
        }
        if (lbl_1_bss_4D8->playerScore[otherPlayerNo] != 0) {
            lbl_1_bss_4D8->playerScore[otherPlayerNo]--;
            lbl_1_bss_4D8->playerScore[playerNo]++;
            if (lbl_1_bss_4D8->playerScore[otherPlayerNo] != 0) {
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
    playerNo = lbl_1_bss_4D8->currentPlayer;
    inputPlayer = playerNo;
    if (HuMCMicGet() == 1) {
mic_retry:
        lbl_1_bss_1C = 1;
        fn_1_2D748(1);
        process = HuPrcChildCreate(fn_1_C9B4, 100, 6144, 0, lbl_1_bss_288);
        HuMCSessionSet(-2);
        HuMCSelWinContextSet(lbl_1_bss_B0, 0, lbl_1_bss_4D8->currentPlayer);
        HuMCSelWinChoiceGet();
        lbl_1_bss_4D8->pauseAllowed = 1;
        HuPrcKill(process);
        fn_1_2D748(0);
        lbl_1_bss_8 = (MicquizResponse *)HuMCCurResponseGet();
        HuMCSelWinContextKill();
        choice = lbl_1_bss_8->recognizedWord;
        lbl_1_bss_1C = 0;

        firstParent = lbl_1_bss_288;
        if (lbl_1_data_208 != -1) {
            HuPrcChildCreate(fn_1_2FCF0, 100, 6144, 0, firstParent);
        }
        if (lbl_1_bss_8->result == -1) {
            fn_1_2D748(1);
            fn_1_11FA8();
            goto mic_retry;
        }
        if (lbl_1_bss_8->result == -2) {
            fn_1_2D748(1);
            fn_1_11FA8();
            goto mic_retry;
        }
        if (lbl_1_bss_8->result == -4) {
            fn_1_2D748(1);
            fn_1_121A4();
            goto mic_retry;
        }

        if (HuMCUnkResponseCheck() != 0) {
            lbl_1_bss_4D8->pauseAllowed = 0;
            fn_1_2D748(1);
            process = HuPrcChildCreate(fn_1_C9B4, 100, 6144, 0, lbl_1_bss_288);
            lbl_1_bss_1C = 1;
            HuPrcVSleep();
            fn_1_2D748(1);
            secondParent = lbl_1_bss_288;
            if (lbl_1_data_208 != -1) {
                HuPrcChildCreate(fn_1_2FCF0, 100, 6144, 0, secondParent);
            }
            response = HuMCNewResponseGet(MESSNUM(MESS_MIC_RETYPING, 0));
            if (response == -1) {
                fn_1_2D748(0);
                HuPrcChildCreate(fn_1_2FD28, 100, 6144, 0, lbl_1_bss_548);
                lbl_1_bss_4D8->pauseAllowed = 1;
                return -1;
            }
            choice = response;
            lbl_1_bss_1C = 0;
            HuPrcKill(process);
            lbl_1_bss_4D8->pauseAllowed = 1;
            fn_1_2D748(0);
        }
        thirdParent = lbl_1_bss_288;
        if (lbl_1_data_208 != -1) {
            HuPrcChildCreate(fn_1_2FCF0, 100, 6144, 0, thirdParent);
        }
        if (fn_1_CBAC(0, choice) == 0) {
            fn_1_2D748(1);
            goto mic_retry;
        }
    } else {

    Hu3DModelAttrReset(lbl_1_bss_FC, HU3D_ATTR_DISPOFF);
    HuPrcSleep(5);
    for (;;) {
        HuPrcVSleep();
        if ((HuPadDStkRep[inputPlayer] & 8) != 0) {
            HuAudFXPlay(MSM_SE_CMN_01);
            lbl_1_bss_5C--;
            if (lbl_1_bss_5C < 0) {
                lbl_1_bss_5C = 4;
            }
        } else if ((HuPadDStkRep[inputPlayer] & 4) != 0) {
            HuAudFXPlay(MSM_SE_CMN_01);
            lbl_1_bss_5C++;
            if (lbl_1_bss_5C > 4) {
                lbl_1_bss_5C = 0;
            }
        }
        if ((HuPadBtnDown[inputPlayer] & PAD_BUTTON_A) != 0) {
            HuAudFXPlay(MSM_SE_CMN_03);
            Hu3DModelAttrSet(lbl_1_bss_FC, HU3D_ATTR_DISPOFF);
            if (fn_1_CBAC(0, lbl_1_bss_5C) != 0) {
                break;
            }
            Hu3DModelAttrReset(lbl_1_bss_FC, HU3D_ATTR_DISPOFF);
        }
        Hu3DModelPosSet(lbl_1_bss_FC, -425.0f,
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
                    lbl_1_bss_4D8->pauseAllowed = 0;
                    if (lbl_1_bss_0 == 0) {
                        lbl_1_bss_0 = HuPrcChildCreate(fn_1_27308, 100, 8192, 0, lbl_1_bss_548);
                    }
                    fn_1_2D748(3);
                    selectionActive = 1;
                    }
            } else {
                selectionActive = 0;
            }
            continue;
        }
        if (HuMCButtonGet() != 0) {
            lbl_1_bss_4D8->pauseAllowed = 0;
            if (lbl_1_bss_1C != 0 && lbl_1_bss_0 == 0) {
                lbl_1_bss_0 = HuPrcChildCreate(fn_1_27308, 100, 8192, 0, lbl_1_bss_548);
            }
            fn_1_2D748(3);
        } else if (lbl_1_bss_0 != 0) {
            HuPrcKill(lbl_1_bss_0);
            lbl_1_bss_0 = 0;
        }
        if (HuMCButtonDownGet() != 0) {
            if (selectionActive == 0) {
                lbl_1_bss_4D8->pauseAllowed = 0;
                if (lbl_1_bss_0 == 0) {
                    lbl_1_bss_0 = HuPrcChildCreate(fn_1_27308, 100, 8192, 0, lbl_1_bss_548);
                }
                fn_1_2D748(3);
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
        HuAudFXPlay(MSM_SE_CMN_05);
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
        lbl_1_bss_4D8->questionColumn = (u8)column;
        lbl_1_bss_4D8->questionLevel = (u8)levelIndex;
        fn_1_FAF4();

        scale = (0.0f);
        angle = (0.0f);
        HuSprGrpScaleSet(lbl_1_bss_5CC, (1.0f), (1.0f));
        for (;;) {
            HuPrcVSleep();
            scale = (f32)((1.0)
                - sin(((3.141592653589793) * (f64)angle) / (180.0)));
            HuSprGrpScaleSet(lbl_1_bss_5CC, scale, scale);
            angle += (3.0f);
            if (angle >= (90.0f)) {
                break;
            }
        }
        HuSprGrpAttrSet(lbl_1_bss_5CC, HUSPR_ATTR_DISPOFF);
        HuSprGrpScaleSet(lbl_1_bss_5CC, (0.0f), (0.0f));
    } else {
        if (fn_1_E69C(levelIndex, column, 2) != 0U) {
            HuAudFXPlay(MSM_SE_CMN_05);
            fn_1_11BB0();
            HuPrcSleep(120);
        } else {
            HuAudFXPlay(MSM_SE_CMN_05);
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
    mode = lbl_1_bss_4D8->currentPlayer;
    fn_1_DD10(levelIndex, column, mode);
    HuAudFXPlay(MSM_SE_MICQUIZ_PANEL_REVEAL);

    angle = 0.0f;
    rotationY = 0.0f;
    for (;;) {
        HuPrcVSleep();
        rotationY = (f32)(180.0 * sin((3.141592653589793 * (f64)angle) / 180.0));
        Hu3DModelRotSet(lbl_1_bss_198[levelIndex].model[column], 0.0f, rotationY, 0.0f);
        angle += 7.5f;
        if (angle >= 90.0f) {
            break;
        }
    }

    Hu3DModelRotSet(lbl_1_bss_198[levelIndex].model[column], 0.0f, 180.0f, 0.0f);
    Hu3DModelPosGet(lbl_1_bss_198[levelIndex].model[column], &selectedPosition);
    Hu3DModelLayerSet(lbl_1_bss_198[levelIndex].model[column], 2);
    Hu3DModelPosSet(lbl_1_bss_100, selectedPosition.x, selectedPosition.y,
                    10.0f + selectedPosition.z);
    Hu3DModelLayerSet(lbl_1_bss_100, 1);
    Hu3DModelAttrReset(lbl_1_bss_100, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(lbl_1_bss_100, HU3D_MOTATTR_PAUSE);

    if (selection == 8) {
        fn_1_E7F4(8);
        return;
    }
    Hu3DModelPosGet(lbl_1_bss_198[levelIndex].model[column], &currentPosition);
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
            Hu3DModelPosSetV(lbl_1_bss_198[levelIndex].model[column], &currentPosition);
            angle += rotationStep;
            Hu3DModelRotSet(lbl_1_bss_198[levelIndex].model[column], 0.0f, angle, 0.0f);
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
            Hu3DModelPosSet(lbl_1_bss_FE, currentPosition.x, currentPosition.y,
                            5.0f + currentPosition.z);
            Hu3DModelLayerSet(lbl_1_bss_FE, 6);
            Hu3DModelAttrReset(lbl_1_bss_FE, HU3D_ATTR_DISPOFF);
            Hu3DModelTPLvlSet(lbl_1_bss_FE, 0.0f);
            opacityAngle = 0.0f;
            for (;;) {
                HuPrcVSleep();
                opacity = (f32)sin((3.141592653589793 * opacityAngle) / 180.0);
                Hu3DModelTPLvlSet(lbl_1_bss_FE, opacity);
                opacityAngle += 9.0f;
                if (opacityAngle >= 180.0f) {
                    break;
                }
            }
            Hu3DModelTPLvlSet(lbl_1_bss_FE, 0.0f);
            Hu3DModelAttrSet(lbl_1_bss_FE, HU3D_ATTR_DISPOFF);
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
            Hu3DModelPosSetV(lbl_1_bss_198[levelIndex].model[column], &currentPosition);
            angle += rotationStep;
            Hu3DModelRotSet(lbl_1_bss_198[levelIndex].model[column], 0.0f, angle, 0.0f);
        }
    }
}

/* Enlarge and fade a model at the supplied position during a grid reveal. */
void fn_1_D6EC(HU3D_MODELID modelId, f32 x, f32 y, f32 z) {
    f32 phase;
    f32 scale;
    f32 transparency;

    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    Hu3DModelPosSet(modelId, x, y, z - (5.0f));
    Hu3DModelLayerSet(modelId, 6);
    phase = (0.0f);
    scale = (1.0f);
    for (;;) {
        HuPrcVSleep();
        scale = (f32)((1.0) + ((0.5) * sin(((3.141592653589793) * phase) / (180.0))));
        transparency = (f32)((1.0) - sin(((3.141592653589793) * phase) / (180.0)));
        Hu3DModelTPLvlSet(modelId, transparency);
        Hu3DModelScaleSet(modelId, scale, scale, scale);
        phase += (2.0f);
        if (phase >= (90.0f)) {
            break;
        }
    }
}

/* Resume the selected grid panel's motion when a reveal starts. */
void fn_1_D870(s32 column, s32 row)
{
    Hu3DModelAttrReset(lbl_1_bss_198[column].model[row], HU3D_MOTATTR_PAUSE);
}

/* Pause and rewind the selected grid panel's motion between reveals. */
void fn_1_D8C4(s32 column, s32 row) {
    Hu3DModelAttrSet(lbl_1_bss_198[column].model[row], HU3D_MOTATTR_PAUSE);
    Hu3DMotionTimeSet(lbl_1_bss_198[column].model[row], (0.0f));
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
        if (lbl_1_bss_4D8->presenterLayout == 0) {
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

    questionIndex = lbl_1_bss_4D8->questionCount / lbl_1_bss_4D8->playerCount + 1;
    playerCount = lbl_1_bss_4D8->playerCount;
    currentPlayer = lbl_1_bss_4D8->currentPlayer;
    category = lbl_1_bss_4D8->playerRank[currentPlayer];
    if (lbl_1_bss_4D8->presenterLayout == 0) {
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

            if (lbl_1_bss_4D8->presenterLayout != 0) {
                if ((answer + 4 == 5) && (lbl_1_bss_4D8->questionLevel == 4)) {
                    continue;
                }
                if (answer + 4 == 6) {
                    if (fn_1_639C() != 0) {
                    } else {
                        continue;
                    }
                }
            } else if ((answer == 2) && (lbl_1_bss_4D8->questionLevel == 0)) {
                continue;
            }
            if (lbl_1_bss_4D8->presenterLayout != 0) {
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
                    Hu3DModelRotSet(lbl_1_bss_198[column].model[row], (0.0f), (180.0f), (0.0f));
                }
                if (animationBank == - 1)
                {
                    Hu3DAnimBankSet(
                        lbl_1_bss_134[column][row],
                        4U);
                    Hu3DModelRotSet(lbl_1_bss_198[column].model[row], (0.0f), (180.0f), (0.0f));
                }
            }
            column += 1;
        }
        row += 1;
    }
}

/* Show the selected grid cell's result marker with a scale and opacity animation. */
void fn_1_DF9C(s32 mode) {
    f32 unused = 700.0f;
    s16 modelId;
    Vec position;
    f32 elapsed;
    f32 scale;
    f32 alpha;

    if (mode != 0) {
        modelId = Hu3DModelLink(lbl_1_bss_248);
    } else {
        modelId = Hu3DModelLink(lbl_1_bss_246);
    }

    scale = 3.0f;
    alpha = 0.0f;
    position.x = 100.0f * (f32)lbl_1_bss_4D8->questionLevel;
    position.y = (-300.0f)
        + (500.0f
           - 100.0f * (f32)lbl_1_bss_4D8->questionColumn);
    position.z = 10.0f;
    Hu3DModelPosSet(modelId, position.x, position.y, position.z);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    Hu3DModelCameraSet(modelId, 1);
    Hu3DModelLayerSet(modelId, 2);
    Hu3DModelScaleSet(modelId, 3.0f, 3.0f, 1.0f);
    Hu3DModelTPLvlSet(modelId, 0.0f);

    Hu3DModelPosSet(lbl_1_bss_244, position.x, position.y, position.z);
    Hu3DModelCameraSet(lbl_1_bss_244, 1);
    Hu3DModelLayerSet(lbl_1_bss_244, 1);
    elapsed = 90.0f;
    scale = 3.0f;
    if (mode != 0) {
        HuAudFXPlay(MSM_SE_MICQUIZ_ALTERNATE_RESULT_MARKER);
    } else {
        HuAudFXPlay(MSM_SE_MICQUIZ_RESULT_MARKER);
    }
    do {
        HuPrcVSleep();
        scale = (f32)(1.0 + (2.0 * sin(3.141592653589793 * (f64)elapsed / 180.0)));
        alpha = (f32)(1.0 - sin((f64)elapsed * 3.141592653589793 / 180.0));
        Hu3DModelScaleSet(modelId, scale, scale, 1.0f);
        Hu3DModelTPLvlSet(modelId, alpha);
        elapsed += 3.0f;
    } while (!(elapsed >= 180.0f));

    Hu3DModelScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    Hu3DModelTPLvlSet(modelId, 1.0f);
    Hu3DModelScaleSet(lbl_1_bss_244, 0.0f, 0.0f, 1.0f);
    Hu3DModelAttrReset(lbl_1_bss_244, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(lbl_1_bss_244, HU3D_MOTATTR_PAUSE);

    scale = 0.0f;
    alpha = 1.0f;
    elapsed = 0.0f;
    do {
        HuPrcVSleep();
        scale = (f32)(sin(3.141592653589793 * (f64)elapsed / 180.0) * 4.0);
        alpha = (f32)(1.0 - sin((f64)elapsed * 3.141592653589793 / 180.0));
        Hu3DModelScaleSet(lbl_1_bss_244, scale, scale, 1.0f);
        Hu3DModelTPLvlSet(lbl_1_bss_244, alpha);
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
                ((row != (s32) lbl_1_bss_4D8->questionColumn) ||
                 (column != (s32) lbl_1_bss_4D8->questionLevel))) {
                if (fn_1_E69C(column, row, QUIZ_CELL_HAS_RESULT_FLAG) != 0U) {
                    markerModel = Hu3DModelLink(lbl_1_bss_248);
                } else if (fn_1_E69C(column, row, QUIZ_CELL_NO_RESULT_FLAG) != 0U) {
                    markerModel = Hu3DModelLink(lbl_1_bss_246);
                }
                Hu3DModelPosSet(markerModel, 100.0f * (f32) column,
                                -300.0f + (500.0f - (100.0f * (f32) row)), 10.0f);
                Hu3DModelAttrReset(markerModel, HU3D_ATTR_DISPOFF);
                Hu3DModelCameraSet(markerModel, 1U);
                Hu3DModelLayerSet(markerModel, 2);
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
    lbl_1_bss_4D8->questionFlags[row][column] |= mask;
}

/* Clear the requested state bits from one question-grid cell. */
void fn_1_E670(s32 row, s32 column, s32 mask)
{
    lbl_1_bss_4D8->questionFlags[row][column] &= ~mask;
}

/* Test the requested state bits of one question-grid cell. */
u32 fn_1_E69C(s32 row, s32 column, s32 mask)
{
    return lbl_1_bss_4D8->questionFlags[row][column] & mask;
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
        lbl_1_bss_4D8->questionFlags[row][column] |= playerFlag;
    }
    if (hasResult != 0) {
        lbl_1_bss_4D8->questionFlags[row][column] |= QUIZ_CELL_HAS_RESULT_FLAG;
        lbl_1_bss_4D8->questionFlags[row][column] &= ~QUIZ_CELL_NO_RESULT_FLAG;
    } else {
        lbl_1_bss_4D8->questionFlags[row][column] |= QUIZ_CELL_NO_RESULT_FLAG;
        lbl_1_bss_4D8->questionFlags[row][column] &= ~QUIZ_CELL_HAS_RESULT_FLAG;
    }
}

/* Select the event used by the following question presentation. */
void fn_1_E7F4(s16 selection) {
    lbl_1_bss_4D8->questionEvent = selection;
}

/* Clear the selected event before the next grid choice. */
void fn_1_E80C(void) {
    lbl_1_bss_4D8->questionEvent = 8;
}

/* Update all four three-digit score displays and advance the active model-piece effects. */
void fn_1_E824(void) {
    s32 index;
    s32 value;
    s32 hundreds;
    s32 tens;
    s32 ones;

    for (index = 0; index < 4; index++) {
        value = lbl_1_bss_4D8->playerScore[index];
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
    fn_1_20B74(2);
    fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_A3B4(1, 1, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 0), 1, 2);
    fn_1_20C94();
    fn_1_20B24();
    fn_1_A3B4(0, 0, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_A3B4(1, 0, 8.0f, HU3D_MOTATTR_LOOP);

    fn_1_20B74(0);
    fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 1), 1, 0);
    fn_1_20C94();
    fn_1_20B24();
    fn_1_A3B4(0, 0, 8.0f, HU3D_MOTATTR_LOOP);

    fn_1_20B74(1);
    fn_1_A3B4(1, 1, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 2), 1, 1);
    fn_1_20C94();
}

/* Show the initial quiz choice prompt and return its selected response. */
s16 fn_1_EF58(void) {
    fn_1_20B74(1);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 3), 1, 1);
    fn_1_20C94();
    return HuWinChoiceGet(lbl_1_bss_37A, -1);
}

/* Show the alternate quiz choice prompt and return its selected response. */
s16 fn_1_F084(void) {
    fn_1_20B74(1);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 59), 1, 1);
    fn_1_20C94();
    return HuWinChoiceGet(lbl_1_bss_37A, -1);
}

/* Show the left presenter's follow-up and restore its idle motion after the text finishes. */
void fn_1_F1B0(void) {
    fn_1_20B74(0);
    fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 4), 1, 0);
    fn_1_20C94();
    fn_1_20B24();
    fn_1_A3B4(0, 0, 8.0f, HU3D_MOTATTR_LOOP);
}

/* Introduce a player by number and character name before that player takes a turn. */
void fn_1_F3CC(s32 playerIndex) {
    char playerNumber[20];

    fn_1_20B74(0);
    sprintf(playerNumber, "%d", playerIndex + 1);
    fn_1_20D08(0, (u32)playerNumber, 0);
    fn_1_20D08(0, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerIndex]], 1);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 5), 1, 0);
    fn_1_20C94();
}

/* Show the shared presenter prompt and close its window when the text finishes. */
void fn_1_F568(void) {
    fn_1_20B74(2);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 6), 1, 2);
    fn_1_20C94();
    fn_1_20B24();
}

/* Address the current player by character name before the next selection. */
void fn_1_F6B4(void) {
    fn_1_20B74(0);
    fn_1_20D08(0, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_4D8->currentPlayer]], 0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 7), 1, 0);
    fn_1_20C94();
}

/* Address the current player with the prompt for the active presenter layout. */
void fn_1_F824(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(0);
        fn_1_20D08(0, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_4D8->currentPlayer]],
                   0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 8), 1, 0);
        fn_1_20C94();
    } else {
        fn_1_20B74(3);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_4D8->currentPlayer]],
                   0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 100), 1, 3);
        fn_1_20C94();
    }
}

/* Announce the selected grid column and level, then pause before closing the window. */
void fn_1_FAF4(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        fn_1_20D08(1, lbl_1_data_40C[lbl_1_bss_4D8->questionColumn], 0);
        fn_1_20D08(1, lbl_1_data_3F8[lbl_1_bss_4D8->questionLevel], 1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 11), 1, 1);
    } else {
        fn_1_20B74(3);
        fn_1_20D08(3, lbl_1_data_40C[lbl_1_bss_4D8->questionColumn], 0);
        fn_1_20D08(3, lbl_1_data_3F8[lbl_1_bss_4D8->questionLevel], 1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 74), 1, 3);
    }
    fn_1_20C94();
    HuPrcSleep(30);
    fn_1_20B24();
}

/* Show the round-count transition message before the quiz overlay changes. */
void fn_1_FE40(void) {
    char questionCountText[32];

    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(2);
        sprintf(questionCountText, "%d", lbl_1_bss_4D8->questionCount);
        fn_1_20D08(2, (u32)questionCountText, 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 12), 1, 2);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 75), 1, 3);
    }
    fn_1_20C94();
    HuPrcSleep(120);
    fn_1_20B24();
}

/* Announce the answering player and the current point amount during response resolution. */
void fn_1_100BC(void) {
    char number[16];

    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        fn_1_20D08(
            1, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[(s8) lbl_1_bss_4D8->answeringPlayer]],
            0);
        sprintf(number, "%d", lbl_1_bss_4D8->roundPoints);
        fn_1_20D08(1, (u32)number, 1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 14), 1, 1);
    } else {
        fn_1_20B74(3);
        fn_1_20D08(
            3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[(s8) lbl_1_bss_4D8->answeringPlayer]],
            0);
        sprintf(number, "%d", lbl_1_bss_4D8->roundPoints);
        fn_1_20D08(3, (u32)number, 1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 77), 1, 3);
    }
    fn_1_20C94();
    fn_1_20B24();
}

/* Show the layout-specific response follow-up and close the window afterward. */
void fn_1_1042C(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 51), 1, 1);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 107), 1, 3);
    }
    fn_1_20C94();
    fn_1_20B24();
}

/* Show player feedback with a character name in the two-presenter layout. */
void fn_1_1065C(s32 playerIndex) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(0);
        fn_1_20D08(0, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerIndex]], 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 41), 1, 0);
        fn_1_20C94();
        fn_1_20B24();
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 97), 1, 3);
        fn_1_20C94();
        fn_1_20B24();
    }
}

/* Show the right presenter's named-player feedback when the two-presenter layout is active. */
void fn_1_10934(s32 playerIndex) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        fn_1_20D08(1, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerIndex]], 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 42), 1, 1);
        fn_1_20C94();
        fn_1_20B24();
    }
}

/* Show single-presenter feedback naming the first stored response participant. */
void fn_1_10AE4(void) {
    fn_1_20B74(3);
    fn_1_20D08(
        3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[(s8) lbl_1_bss_4D8->responseRecord[2]]], 0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 106), 1, 3);
    fn_1_20C94();
}

/* Show single-presenter feedback naming both stored response participants. */
void fn_1_10C58(void) {
    fn_1_20B74(3);
    fn_1_20D08(
        3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[(s8) lbl_1_bss_4D8->responseRecord[2]]], 0);
    fn_1_20D08(
        3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[(s8) lbl_1_bss_4D8->responseRecord[3]]], 1);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 108), 1, 3);
    fn_1_20C94();
}

/* Choose one of two feedback lines for the answering player, then close the presenter window. */
void fn_1_10E28(void) {
    s32 choice;

    choice = frandmod(2);
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        if (choice != 0) {
            fn_1_20B74(0);
            fn_1_20D08(
                0, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_4D8->answeringPlayer]],
                0);
            fn_1_20A50(MESSNUM(MESS_MICQUIZ, 41), 1, 0);
        } else {
            fn_1_20B74(1);
            fn_1_20D08(
                1, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_4D8->answeringPlayer]],
                0);
            fn_1_20A50(MESSNUM(MESS_MICQUIZ, 42), 1, 1);
        }
    } else {
        fn_1_20B74(3);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_4D8->answeringPlayer]],
                   0);
        if (choice != 0) {
            fn_1_20A50(MESSNUM(MESS_MICQUIZ, 97), 1, 3);
        } else {
            fn_1_20A50(MESSNUM(MESS_MICQUIZ, 98), 1, 3);
        }
    }
    fn_1_20C94();
    fn_1_20B24();
}

/* Show the named participant's single-presenter feedback during the reaction sequence. */
void fn_1_112B0(void) {
    fn_1_20B74(3);
    fn_1_20D08(
        3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[(s8) lbl_1_bss_4D8->responseRecord[2]]], 0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 106), 1, 3);
    fn_1_20C94();
}

/* Show the single-presenter response follow-up and wait for its text to finish. */
void fn_1_11424(void) {
    fn_1_20B74(3);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 107), 1, 3);
    fn_1_20C94();
}

/* Name both stored participants in the single-presenter response follow-up. */
void fn_1_1153C(void) {
    fn_1_20B74(3);
    fn_1_20D08(
        3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[(s8) lbl_1_bss_4D8->responseRecord[2]]], 0);
    fn_1_20D08(
        3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[(s8) lbl_1_bss_4D8->responseRecord[3]]], 1);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 108), 1, 3);
    fn_1_20C94();
}

/* Announce the current player and, in the two-presenter layout, the round count. */
void fn_1_1170C(void) {
    char questionCountText[16];
    u32 messageId;
    u32 alternateCharacterMessage;

    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(0);
        sprintf(questionCountText, "%d", lbl_1_bss_4D8->questionCount);
        fn_1_20D08(0, (u32)questionCountText, 0);
        messageId = lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_4D8->currentPlayer]];
        fn_1_20D08(0, messageId, 1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 15), 1, 0);
    } else {
        fn_1_20B74(3);
        alternateCharacterMessage =
            lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_4D8->currentPlayer]];
        fn_1_20D08(3, alternateCharacterMessage, 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 78), 1, 3);
    }
    fn_1_20C94();
}

/* Announce the round count and current player in the single-presenter layout. */
void fn_1_119FC(void) {
    char questionCountText[20];

    fn_1_20B74(3);
    sprintf(questionCountText, "%d", lbl_1_bss_4D8->questionCount);
    fn_1_20D08(3, (u32)questionCountText, 0);
    fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_4D8->currentPlayer]], 1);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 70), 1, 3);
    fn_1_20C94();
}

/* Show the exhausted-column explanation after a question-grid choice is rejected. */
void fn_1_11BB0(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 10), 1, 0);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 73), 1, 3);
    }
    fn_1_20C94();
}

/* Show the unavailable-cell explanation after a question-grid choice is rejected. */
void fn_1_11DAC(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 8), 1, 0);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 71), 1, 3);
    }
    fn_1_20C94();
}

/* Show the layout-specific selection explanation and wait before continuing. */
void fn_1_11FA8(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 9), 1, 0);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 72), 1, 3);
    }
    fn_1_20C94();
}

/* Show the layout-specific round follow-up and wait before continuing. */
void fn_1_121A4(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 55), 1, 0);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 114), 1, 3);
    }
    fn_1_20C94();
}

/* Name the current player in the layout-specific round follow-up. */
void fn_1_123A0(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        s32 playerIndex;

        fn_1_20B74(1);
        playerIndex = lbl_1_bss_4D8->currentPlayer;
        fn_1_20D08(1, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerIndex]], 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 43), 1, 1);
    } else {
        s32 playerIndex;

        fn_1_20B74(3);
        playerIndex = lbl_1_bss_4D8->currentPlayer;
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerIndex]], 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 99), 1, 3);
    }
    fn_1_20C94();
}

/* Present two formatted point amounts for a score-change explanation. */
void fn_1_1264C(s32 firstValue, s32 secondValue) {
    char firstText[20];
    char secondText[20];

    fn_1_20B74(0);
    sprintf(firstText, "%d", firstValue);
    sprintf(secondText, "%d", secondValue);
    fn_1_20D08(0, (u32)firstText, 0);
    fn_1_20D08(0, (u32)secondText, 1);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 45), 1, 0);
    fn_1_20C94();
    fn_1_20B24();
}

/* Present the alternate two-amount explanation for a score change. */
void fn_1_12810(s32 firstValue, s32 secondValue) {
    char firstText[20];
    char secondText[20];

    fn_1_20B74(0);
    sprintf(firstText, "%d", firstValue);
    sprintf(secondText, "%d", secondValue);
    fn_1_20D08(0, (u32)firstText, 0);
    fn_1_20D08(0, (u32)secondText, 1);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 46), 1, 0);
    fn_1_20C94();
    fn_1_20B24();
}

/* Present score standings and tied-player reactions, then select the closing commentary. */
void fn_1_129D4(void)
{
    OM_CAMERA_VIEW firstCameraView;
    OM_CAMERA_VIEW secondCameraView;
    s32 statusCounts[4];
    s32 scoreOrder[4];
    s32 index;
    s32 otherIndex;
    s32 firstPlayer;
    s32 orderIndex;
    s32 state;
    s32 firstStatePlayer;
    s32 presentationChoice;
    s32 waitFrame;

    presentationChoice = frandmod(2);
    fn_1_8488();
    firstCameraView = lbl_1_data_5F8;
    omCameraViewMoveSimpleMulti(4, &firstCameraView, 120);
    for (orderIndex = 0; orderIndex < 120; orderIndex++) {
        HuPrcVSleep();
    }
    HuAudFXPlay(MSM_SE_MICQUIZ_STANDINGS);
    fn_1_13B54();
    if (presentationChoice == 0) {
        lbl_1_bss_4D8->presenterLayout = 1;
        lbl_1_bss_4D8->pauseAllowed = 0;
        fn_1_40FC();
        lbl_1_bss_4D8->pauseAllowed = 1;
        fn_1_13878();
    }
    secondCameraView = lbl_1_data_5C0;
    omCameraViewMoveSimpleMulti(4, &secondCameraView, 120);
    for (waitFrame = 0; waitFrame < 120; waitFrame++) {
        HuPrcVSleep();
    }
    if (fn_1_632C() != 0) {
        firstPlayer = -1;
        for (index = 0; index < 4; index++) {
            if (lbl_1_bss_4D8->playerActive[index] != 0) {
                if (firstPlayer == -1) {
                    firstPlayer = index;
                }
                HuAudFXPlay(lbl_1_data_3E0[lbl_1_bss_4D8->playerCharacter[index]]);
                fn_1_A678(index, 2, 8.0f, 0);
            }
        }
        fn_1_15EF8();
        HuPrcSleep(8);
        fn_1_A748(firstPlayer);
        HuPrcSleep(60);
        for (index = 0; index < 4; index++) {
            if (lbl_1_bss_4D8->playerActive[index] != 0) {
                fn_1_A678(index, 0, 8.0f, HU3D_MOTATTR_LOOP);
            }
        }
        return;
    }

    for (otherIndex = 0; otherIndex < 4; otherIndex++) {
        for (index = 0, statusCounts[otherIndex] = 0; index < 4; index++) {
            if (lbl_1_bss_4D8->playerRank[index] == 1) {
                statusCounts[otherIndex]++;
            }
        }
    }
    for (otherIndex = 0, state = 4; otherIndex < 3; otherIndex++, state--) {
        firstStatePlayer = 0;
        for (index = 0; index < 4; index++) {
            if (lbl_1_bss_4D8->playerRank[index] == state) {
                if (firstStatePlayer == 0) {
                    if (lbl_1_bss_4D8->playerActive[index] != 0) {
                        fn_1_13D70(state, index);
                        HuAudFXPlay(lbl_1_data_3E0[lbl_1_bss_4D8->playerCharacter[index]]);
                        fn_1_A678(index, 2, 8.0f, 0);
                        HuPrcSleep(8);
                        fn_1_A748(index);
                        HuPrcSleep(60);
                        fn_1_A678(index, 0, 16.0f, HU3D_MOTATTR_LOOP);
                    }
                } else if (lbl_1_bss_4D8->playerActive[index] != 0) {
                    fn_1_142F4(state, index);
                    HuAudFXPlay(lbl_1_data_3E0[lbl_1_bss_4D8->playerCharacter[index]]);
                    fn_1_A678(index, 2, 8.0f, 0);
                    HuPrcSleep(8);
                    fn_1_A748(index);
                    HuPrcSleep(60);
                    fn_1_A678(index, 0, 16.0f, HU3D_MOTATTR_LOOP);
                }
                firstStatePlayer = 1;
            }
        }
    }
    HuPrcSleep(30);
    for (index = 0; index < 4; index++) {
        scoreOrder[index] = lbl_1_bss_4D8->playerScore[index];
    }
    fn_1_A97C(scoreOrder, 4);
    for (index = 0; index < 4; index++) {
        lbl_1_bss_7C[index] = -1;
    }
    for (index = 0, otherIndex = 0; index < 4; index++) {
        if (lbl_1_bss_4D8->playerScore[index] == scoreOrder[0]) {
            lbl_1_bss_7C[otherIndex] = index;
            otherIndex++;
        }
    }
    if (lbl_1_bss_4D8->playerCount == statusCounts[0]) {
        fn_1_15AB0(lbl_1_bss_7C[0]);
    } else {
        switch (statusCounts[0]) {
        case 1: {
            fn_1_148E4(lbl_1_bss_7C[0]);
            HuAudFXPlay(lbl_1_data_3CA[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_7C[0]]]);
            fn_1_13740(lbl_1_bss_7C[0], -1, -1, -1, 7);
            HuPrcSleep(8);
            fn_1_A748(lbl_1_bss_7C[0]);
            fn_1_13740(lbl_1_bss_7C[0], -1, -1, -1, 0);
            break;
        }
        case 2: {
            fn_1_14E48(lbl_1_bss_7C[0], lbl_1_bss_7C[1]);
            HuAudFXPlay(lbl_1_data_3CA[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_7C[0]]]);
            HuAudFXPlay(lbl_1_data_3CA[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_7C[1]]]);
            fn_1_13740(lbl_1_bss_7C[0], lbl_1_bss_7C[1], -1, -1, 7);
            HuPrcSleep(8);
            fn_1_A748(lbl_1_bss_7C[0]);
            fn_1_13740(lbl_1_bss_7C[0], lbl_1_bss_7C[1], -1, -1, 0);
            break;
        }
        case 3: {
            fn_1_15424(lbl_1_bss_7C[0], lbl_1_bss_7C[1], lbl_1_bss_7C[2]);
            HuAudFXPlay(lbl_1_data_3CA[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_7C[0]]]);
            HuAudFXPlay(lbl_1_data_3CA[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_7C[1]]]);
            HuAudFXPlay(lbl_1_data_3CA[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_7C[2]]]);
            fn_1_13740(lbl_1_bss_7C[0], lbl_1_bss_7C[1], lbl_1_bss_7C[2], -1, 7);
            HuPrcSleep(8);
            fn_1_A748(lbl_1_bss_7C[0]);
            fn_1_13740(lbl_1_bss_7C[0], lbl_1_bss_7C[1], lbl_1_bss_7C[2], -1, 0);
            break;
        }
        }
    }
    if (lbl_1_bss_4D8->presenterLayout == 0) {
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
            motionId = Hu3DMotionIDGet(lbl_1_bss_2FC[playerNo]);
            if (motionId != lbl_1_bss_2AC[playerNo][motionIndex]) {
                CharMotionShiftSet(lbl_1_bss_2F4[playerNo], lbl_1_bss_2AC[playerNo][motionIndex],
                                   0.0f, 8.0f, 0);
            }
        }
    }
}

/* Play the single presenter's three-part standings dialogue with presenter motion changes. */
void fn_1_13878(void) {
    s16 introMotionId;
    s16 speakingMotionId;

    fn_1_20B74(3);
    introMotionId = Hu3DMotionIDGet(lbl_1_bss_294);
    if (introMotionId != lbl_1_bss_28C[3]) {
        Hu3DMotionShiftSet(lbl_1_bss_294, lbl_1_bss_28C[3],
                           (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
    }
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 60), 1, 3);
    fn_1_20C94();
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 61), 1, 3);
    speakingMotionId = Hu3DMotionIDGet(lbl_1_bss_294);
    if (speakingMotionId != lbl_1_bss_28C[1]) {
        Hu3DMotionShiftSet(lbl_1_bss_294, lbl_1_bss_28C[1],
                           (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
    }
    fn_1_20C94();
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 62), 1, 3);
    fn_1_20C94();
    fn_1_20B24();
}

/* Play the left presenter's standings line, then restore its idle motion. */
void fn_1_13B54(void) {
    fn_1_20B74(0);
    fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 18), 1, 0);
    fn_1_20C94();
    fn_1_A3B4(0, 0, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_20B24();
}

/* Announce a player's placing and score with the active presenter layout. */
void fn_1_13D70(s32 messageNo, s32 playerNo) {
    char playerScore[19];

    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(0);
        fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20D08(0, lbl_1_data_8A0[messageNo - 1], 0);
        fn_1_20D08(0, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerNo]], 1);
        sprintf(playerScore, "%d", lbl_1_bss_4D8->playerScore[playerNo]);
        fn_1_20D08(0, (u32)playerScore, 2);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 19), 1, 0);
        fn_1_20C94();
        fn_1_A3B4(0, 0, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20B24();
    } else {
        fn_1_20B74(3);
        fn_1_A4CC(1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20D08(3, lbl_1_data_8A0[messageNo - 1], 0);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerNo]], 1);
        sprintf(playerScore, "%d", lbl_1_bss_4D8->playerScore[playerNo]);
        fn_1_20D08(3, (u32)playerScore, 2);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 63), 1, 3);
        fn_1_20C94();
        fn_1_20B24();
    }
}

/* Announce the alternate placing and score line while the presenter speaks. */
void fn_1_142F4(s32 messageNo, s32 playerNo) {
    char playerScore[20];

    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        fn_1_A3B4(1, 1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20D08(1, lbl_1_data_8A0[messageNo - 1], 0);
        fn_1_20D08(1, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerNo]], 1);
        sprintf(playerScore, "%d", lbl_1_bss_4D8->playerScore[playerNo]);
        fn_1_20D08(1, (u32)playerScore, 2);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 20), 1, 1);
        fn_1_20C94();
        fn_1_A3B4(1, 0, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20B24();
    } else {
        fn_1_20B74(3);
        fn_1_A4CC(1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20D08(3, lbl_1_data_8A0[messageNo - 1], 0);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerNo]], 1);
        sprintf(playerScore, "%d", lbl_1_bss_4D8->playerScore[playerNo]);
        fn_1_20D08(3, (u32)playerScore, 2);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 64), 1, 3);
        fn_1_20C94();
        fn_1_A4CC(0, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20B24();
    }
}

/* Name a player and their score during the standings dialogue. */
void fn_1_148E4(s32 playerIndex) {
    char playerScore[21];

    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        fn_1_A3B4(1, 1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20D08(1, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerIndex]], 0);
        sprintf(playerScore, "%d", lbl_1_bss_4D8->playerScore[playerIndex]);
        fn_1_20D08(1, (u32)playerScore, 1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 21), 1, 1);
        fn_1_20C94();
        fn_1_A3B4(1, 0, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20B24();
    } else {
        fn_1_20B74(3);
        fn_1_A4CC(1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerIndex]], 0);
        sprintf(playerScore, "%d", lbl_1_bss_4D8->playerScore[playerIndex]);
        fn_1_20D08(3, (u32)playerScore, 1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 65), 1, 3);
        fn_1_20C94();
        fn_1_A4CC(0, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20B24();
    }
}

/* Describe one player relative to another, inserting the first player's score. */
void fn_1_14E48(s32 playerNo, s32 otherPlayerNo) {
    char playerScore[20];

    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(0);
        fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20D08(0, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerNo]], 0);
        fn_1_20D08(0, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[otherPlayerNo]], 1);
        sprintf(playerScore, "%d", lbl_1_bss_4D8->playerScore[playerNo]);
        fn_1_20D08(0, (u32)playerScore, 2);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 22), 1, 0);
        fn_1_20C94();
        fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
    } else {
        fn_1_20B74(3);
        fn_1_A4CC(1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerNo]], 0);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[otherPlayerNo]], 1);
        sprintf(playerScore, "%d", lbl_1_bss_4D8->playerScore[playerNo]);
        fn_1_20D08(3, (u32)playerScore, 2);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 66), 1, 3);
        fn_1_20C94();
        fn_1_A4CC(0, 8.0f, HU3D_MOTATTR_LOOP);
    }
    fn_1_20B24();
}

/* Describe one player relative to two others, inserting that player's score. */
void fn_1_15424(s32 playerNo, s32 firstOtherPlayer, s32 secondOtherPlayer) {
    char playerScore[20];

    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(0);
        fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20D08(0, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerNo]], 0);
        fn_1_20D08(0, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[firstOtherPlayer]], 1);
        fn_1_20D08(0, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[secondOtherPlayer]], 2);
        sprintf(playerScore, "%d", lbl_1_bss_4D8->playerScore[playerNo]);
        fn_1_20D08(0, (u32)playerScore, 3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 23), 1, 0);
        fn_1_20C94();
        fn_1_A3B4(0, 0, 8.0f, HU3D_MOTATTR_LOOP);
    } else {
        fn_1_20B74(3);
        fn_1_A4CC( 1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerNo]], 0);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[firstOtherPlayer]], 1);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[secondOtherPlayer]], 2);
        sprintf(playerScore, "%d", lbl_1_bss_4D8->playerScore[playerNo]);
        fn_1_20D08(3, (u32)playerScore, 3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 67), 1, 3);
        fn_1_20C94();
        fn_1_A4CC( 0, 8.0f, HU3D_MOTATTR_LOOP);
    }
    fn_1_20B24();
}

/* Show the score follow-up while the active presenter speaks. */
void fn_1_15AB0(s32 playerIndex) {
    char playerScore[20];

    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(0);
        fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
        sprintf(playerScore, "%d", lbl_1_bss_4D8->playerScore[playerIndex]);
        fn_1_20D08(0, (u32)playerScore, 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 24), 1, 0);
        fn_1_20C94();
        fn_1_A3B4(0, 0, 8.0f, HU3D_MOTATTR_LOOP);
    } else {
        fn_1_20B74(3);
        fn_1_A4CC(1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 68), 1, 3);
        fn_1_20C94();
        fn_1_A4CC(0, 8.0f, HU3D_MOTATTR_LOOP);
    }
    fn_1_20B24();
}

/* Show the layout-specific standings follow-up and close the dialogue window. */
void fn_1_15EF8(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 52), 1, 1);
        fn_1_20C94();
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 105), 1, 3);
        fn_1_20C94();
    }
    fn_1_20B24();
}

/* Present the first final-results line with the active presenters' speaking motions. */
void fn_1_1614C(void) {

    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(2);
        fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_A3B4(1, 1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 30), 1, 2);
        fn_1_20C94();
        HuPrcSleep(120);
        fn_1_A3B4(0, 0, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_A3B4(1, 0, 8.0f, HU3D_MOTATTR_LOOP);
    } else {
        fn_1_20B74(3);
        fn_1_A4CC(1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 86), 1, 3);
        fn_1_20C94();
        HuPrcSleep(120);
        fn_1_A4CC(0, 8.0f, HU3D_MOTATTR_LOOP);
    }
    fn_1_20B24();
}

/* Present the next final-results line and restore the speaking presenter's idle motion. */
void fn_1_16620(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        fn_1_A3B4(1, 1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 31), 1, 1);
        fn_1_20C94();
        HuPrcSleep(120);
        fn_1_A3B4(1, 0, 8.0f, HU3D_MOTATTR_LOOP);
    } else {
        fn_1_20B74(3);
        fn_1_A4CC( 1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 87), 1, 3);
        fn_1_20C94();
        HuPrcSleep(120);
        fn_1_A4CC( 0, 8.0f, HU3D_MOTATTR_LOOP);
    }
    fn_1_20B24();
}

/* Present the shared final-results follow-up, pausing before restoring the idle motion. */
void fn_1_16A24(void) {
    s16 score;

    score = fn_1_16E30();
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(0);
        fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 32), 1, 0);
        fn_1_20C94();
        HuPrcSleep(120);
        fn_1_A3B4(0, 0, 8.0f, HU3D_MOTATTR_LOOP);
    } else {
        fn_1_20B74(3);
        fn_1_A4CC(1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 88), 1, 3);
        fn_1_20C94();
        HuPrcSleep(120);
        fn_1_A4CC(0, 8.0f, HU3D_MOTATTR_LOOP);
    }
    fn_1_20B24();
}

/* Return the last active player's positive score encountered while scanning the result list. */
s16 fn_1_16E30(void) {
    s16 value;
    s16 index;

    value = 0;
    index = 0;
    while (index < 4) {
        if (lbl_1_bss_4D8->playerActive[index] != 0
            && lbl_1_bss_4D8->playerScore[index] > 0) {
            value = lbl_1_bss_4D8->playerScore[index];
        }
        index++;
    }
    return value;
}

/* Read a player's score in the final-results dialogue and pause before closing it. */
void fn_1_16ECC(s32 playerIndex)
{
    char scoreText[20];

    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        sprintf(scoreText, "%d", lbl_1_bss_4D8->playerScore[playerIndex]);
        fn_1_20D08(1, (u32)scoreText, 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 33), 1, 1);
        fn_1_20C94();
    } else {
        fn_1_20B74(3);
        sprintf(scoreText, "%d", lbl_1_bss_4D8->playerScore[playerIndex]);
        fn_1_20D08(3, (u32)scoreText, 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 89), 1, 3);
        fn_1_20C94();
    }
    HuPrcSleep(120);
    fn_1_20B24();
}

/* Announce the sole winner by character name during final results. */
void fn_1_171CC(s32 playerIndex, s32 secondPlayer, s32 thirdPlayer, s32 fourthPlayer) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(2);
        fn_1_20D08(2, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerIndex]], 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 34), 1, 2);
    } else {
        fn_1_20B74(3);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerIndex]], 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 90), 1, 3);
    }
    fn_1_20C94();
}

/* Announce two tied winners by character name during final results. */
void fn_1_1745C(s32 firstPlayer, s32 secondPlayer, s32 thirdPlayer, s32 fourthPlayer)
{
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(2);
        fn_1_20D08(2, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[firstPlayer]], 0);
        fn_1_20D08(2, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[secondPlayer]], 1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 35), 1, 2);
    } else {
        fn_1_20B74(3);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[firstPlayer]], 0);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[secondPlayer]], 1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 91), 1, 3);
    }
    fn_1_20C94();
}

/* Announce three tied winners by character name during final results. */
void fn_1_1778C(s32 firstPlayer, s32 secondPlayer, s32 thirdPlayer, s32 fourthPlayer)
{
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(2);
        fn_1_20D08(2, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[firstPlayer]], 0);
        fn_1_20D08(2, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[secondPlayer]], 1);
        fn_1_20D08(2, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[thirdPlayer]], 2);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 36), 1, 2);
    } else {
        fn_1_20B74(3);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[firstPlayer]], 0);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[secondPlayer]], 1);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[thirdPlayer]], 2);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 92), 1, 3);
    }
    fn_1_20C94();
}

/* Show the all-player tie message during final results. */
void fn_1_17B6C(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(2);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 37), 1, 2);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 93), 1, 3);
    }
    fn_1_20C94();
}

/* Ask the replay question after final results and leave its window ready for a choice. */
void fn_1_17D68(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 38), 1, 0);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 94), 1, 3);
    }
    fn_1_20C94();
}

/* Show the closing line after a replay choice and pause before leaving the results. */
void fn_1_17F64(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(2);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 39), 1, 2);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 95), 1, 3);
    }
    fn_1_20C94();
    HuPrcSleep(120);
}

/* Present the alternate final-results reactions for every active player. */
void fn_1_18168(void)
{
    s32 playerNo;

    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 53), 1, 1);
        fn_1_20C94();
        fn_1_20B74(2);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 54), 1, 2);
        fn_1_20C94();
        for (playerNo = 0; playerNo < 4; playerNo++) {
            if (lbl_1_bss_4D8->playerActive[playerNo] != 0) {
                fn_1_A678(playerNo, 2, 8.0f, 0);
            }
        }
        return;
    }

    fn_1_20B74(3);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 113), 1, 3);
    fn_1_20C94();
    HuPrcSleep(120);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (lbl_1_bss_4D8->playerActive[playerNo] != 0) {
            Hu3DModelAttrReset(lbl_1_bss_1F0[playerNo], HU3D_ATTR_DISPOFF);
            HuAudFXPlay(MSM_SE_MICQUIZ_PLAYER_CELEBRATE);
            fn_1_A678(playerNo, 3, 8.0f, 0);
        }
    }
    HuPrcSleep(93);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (lbl_1_bss_4D8->playerActive[playerNo] != 0) {
            Hu3DModelAttrSet(lbl_1_bss_1F0[playerNo], HU3D_ATTR_DISPOFF);
            fn_1_A678(playerNo, 6, 8.0f, HU3D_MOTATTR_LOOP);
        }
    }
}

/* Show the single-presenter introduction before naming players at the lowest finishing rank. */
void fn_1_186E8(void) {
    fn_1_20B74(3);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 109), 1, 3);
    fn_1_20C94();
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
        if (lbl_1_bss_4D8->playerActive[playerIndex] != 0
            && lbl_1_bss_4D8->playerRank[playerIndex] == lastPlaceRank) {
            lastPlacePlayers[lastPlacePlayerCount] = playerIndex;
            lastPlaceCharacterMessages[lastPlacePlayerCount] =
                lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerIndex]];
            lastPlacePlayerCount++;
        }
    }

    resultCount = lastPlacePlayerCount;
    if (resultCount == 1) {
        messageId = MESSNUM(MESS_MICQUIZ, 110);
        fn_1_20D08(3, lastPlaceCharacterMessages[0], 0);
    } else if (resultCount == 2) {
        messageId = MESSNUM(MESS_MICQUIZ, 111);
        fn_1_20D08(3, lastPlaceCharacterMessages[0], 0);
        fn_1_20D08(3, lastPlaceCharacterMessages[1], 1);
    } else if (resultCount == 3) {
        messageId = MESSNUM(MESS_MICQUIZ, 112);
        fn_1_20D08(3, lastPlaceCharacterMessages[0], 0);
        fn_1_20D08(3, lastPlaceCharacterMessages[1], 1);
        fn_1_20D08(3, lastPlaceCharacterMessages[2], 2);
    }
    fn_1_20B74(3);
    fn_1_20A50(messageId, 1, 3);
    fn_1_20C94();
}

/* Show the single-presenter explanation before the response retry or follow-up. */
void fn_1_18B50(void) {
    fn_1_20B74(3);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_I, 9), 1, 3);
    fn_1_20C94();
}

/* Show the single-presenter response line and wait before continuing. */
void fn_1_18C68(void) {
    fn_1_20B74(3);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 102), 1, 3);
    fn_1_20C94();
}

/* Show the single-presenter round follow-up and wait before continuing. */
void fn_1_18D80(void) {
    fn_1_20B74(3);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 114), 1, 3);
    fn_1_20C94();
}

/* Name the selected character in the single-presenter comparison dialogue. */
void fn_1_18E98(s32 messageIndex) {
    s32 messageId;

    fn_1_20B74(3);
    messageId = lbl_1_data_A0[messageIndex];
    fn_1_20D08(3, messageId, 0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_I, 21), 1, 3);
    fn_1_20C94();
}

/* Name the selected character in the alternate single-presenter comparison line. */
void fn_1_18FEC(s32 messageIndex) {
    s32 messageId;

    fn_1_20B74(3);
    messageId = lbl_1_data_A0[messageIndex];
    fn_1_20D08(3, messageId, 0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_I, 22), 1, 3);
    fn_1_20C94();
}

/* Show the single-presenter result line for the supplied character message. */
void fn_1_19140(s32 messageIndex) {
    s32 messageId;

    fn_1_20B74(3);
    messageId = lbl_1_data_A0[messageIndex];
    fn_1_20D08(3, messageId, 0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_I, 10), 1, 3);
    fn_1_20C94();
}

/* Play the timed single-presenter explanation, ending with the named player. */
void fn_1_19294(s32 playerIndex) {
    fn_1_20B74(3);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_I, 23), 1, 3);
    fn_1_20C94();
    HuPrcSleep(120);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_I, 29), 1, 3);
    fn_1_20C94();
    HuPrcSleep(120);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_I, 30), 1, 3);
    fn_1_20C94();
    HuPrcSleep(120);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_I, 31), 1, 3);
    fn_1_20C94();
    HuPrcSleep(120);
    fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerIndex]], 0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_I, 25), 1, 3);
    fn_1_20C94();
}

/* Show the single-presenter closing explanation and pause for 120 frames. */
void fn_1_1959C(void) {
    fn_1_20B74(3);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_I, 32), 1, 3);
    fn_1_20C94();
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
    playerCount = lbl_1_bss_4D8->playerCount;
    for (currentPlayer = 0; currentPlayer < 4; currentPlayer++) {
        lbl_1_bss_9C[currentPlayer] = -1;
        lbl_1_bss_90[currentPlayer] = -1;
    }
    for (currentPlayer = 0, activeCount = 0; currentPlayer < 4; currentPlayer++) {
        if (currentPlayer != playerNo && lbl_1_bss_4D8->playerActive[currentPlayer] != 0) {
            lbl_1_bss_9C[activeCount] = currentPlayer;
            lbl_1_bss_90[activeCount] = lbl_1_bss_4D8->playerCharacter[currentPlayer];
            activeMask |= 1 << currentPlayer;
            activeCount++;
        }
    }

    fn_1_20B74(3);
    if (playerCount == 2) {
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_90[0]], 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_I, 26), 1, 3);
    } else if (playerCount == 3) {
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_90[0]], 0);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_90[1]], 1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_I, 27), 1, 3);
    } else if (playerCount == 4) {
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_90[0]], 0);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_90[1]], 1);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_90[2]], 2);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_I, 28), 1, 3);
    }
    fn_1_20C94();
    HuWinDisablePlayerSet(lbl_1_bss_380, activeMask);
    choice = fn_1_20CD4(-1);

    fn_1_20148();
    return choice;
}

/* Insert a player's character name into the current single-presenter result message. */
void fn_1_19C84(s32 playerNo) {
    s32 messageId;

    messageId = lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerNo]];
    fn_1_20D08(3, messageId, 0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_I, 10), 1, 3);
    fn_1_20C94();
}

/* Advance the camera surface model's motion for a short randomized interval. */
void fn_1_19D5C(void) {
    Hu3DMotionSet(lbl_1_bss_23E, lbl_1_bss_1DC);
    Hu3DModelAttrReset(lbl_1_bss_23E, HU3D_MOTATTR_PAUSE);
    HuPrcSleep(frandmod(10) + 5);
    Hu3DModelAttrSet(lbl_1_bss_23E, HU3D_MOTATTR_PAUSE);
}

/* Switch camera capture textures and viewports, optionally showing the transition model. */
void fn_1_19DD4(s32 cameraIndex, s32 firstCapture, s32 showTransition) {
    switch (cameraIndex) {
    case 0:
        if (firstCapture == 0) {
            Hu3DAnimKill(lbl_1_bss_242);
            Hu3DCameraLayerHookReset(4, 0);
        }
        lbl_1_bss_242 = Hu3DAnimCreate(lbl_1_bss_30, lbl_1_bss_23E, "vq_test");
        Hu3DCameraLayerHookSet(4, 0, fn_1_3FE4);
        fn_1_1A210();
        if (showTransition != 0) {
            Hu3DModelAttrReset(lbl_1_bss_24C, HU3D_ATTR_DISPOFF);
            HuPrcSleep(10);
            Hu3DModelAttrSet(lbl_1_bss_24C, HU3D_ATTR_DISPOFF);
        }
        fn_1_1A270();
        Hu3DCameraPerspectiveSet(1, (20.0f), (10.0f), (20000.0f), (1.2f));
        Hu3DCameraViewportSet(1, (0.0f), (0.0f), (640.0f), (480.0f), (0.0f), (1.0f));
        Hu3DCameraScissorSet(1, 0U, 0U, 640U, 480U);
        Hu3DCameraPerspectiveSet(2, (20.0f), (10.0f), (20000.0f), (1.2f));
        Hu3DCameraViewportSet(2, (0.0f), (0.0f), (0.0f), (0.0f), (0.0f), (1.0f));
        Hu3DCameraScissorSet(2, 0U, 0U, 0U, 0U);
        break;
    case 1:
        if (firstCapture == 0) {
            Hu3DAnimKill(lbl_1_bss_240);
            Hu3DCameraLayerHookReset(4, 0);
        }
        lbl_1_bss_240 = Hu3DAnimCreate(lbl_1_bss_34, lbl_1_bss_23E, "vq_test");
        Hu3DCameraLayerHookSet(4, 0, fn_1_4030);
        Hu3DCameraPerspectiveSet(1, (20.0f), (10.0f), (20000.0f), (1.2f));
        Hu3DCameraViewportSet(1, (0.0f), (0.0f), (0.0f), (0.0f), (0.0f), (1.0f));
        Hu3DCameraScissorSet(1, 0U, 0U, 0U, 0U);
        Hu3DCameraPerspectiveSet(2, (20.0f), (10.0f), (20000.0f), (1.2f));
        Hu3DCameraViewportSet(2, (0.0f), (0.0f), (640.0f), (480.0f), (0.0f), (1.0f));
        Hu3DCameraScissorSet(2, 0U, 0U, 640U, 480U);
        fn_1_1A210();
        if (showTransition != 0) {
            Hu3DModelAttrReset(lbl_1_bss_24C, HU3D_ATTR_DISPOFF);
            HuPrcSleep(10);
            Hu3DModelAttrSet(lbl_1_bss_24C, HU3D_ATTR_DISPOFF);
        }
        fn_1_1A270();
        break;
    }
    lbl_1_bss_B2 = cameraIndex;
}

/* Hide the layout-specific camera surface model while capture views change. */
void fn_1_1A210(void) {
    if (lbl_1_bss_4D8->presenterLayout != 0) {
        Hu3DModelAttrSet(lbl_1_bss_252, HU3D_ATTR_DISPOFF);
        return;
    }
    Hu3DModelAttrSet(lbl_1_bss_254, HU3D_ATTR_DISPOFF);
}

/* Show the layout-specific camera surface model after capture views change. */
void fn_1_1A270(void) {
    if (lbl_1_bss_4D8->presenterLayout != 0) {
        Hu3DModelAttrReset(lbl_1_bss_252, HU3D_ATTR_DISPOFF);
        return;
    }
    Hu3DModelAttrReset(lbl_1_bss_254, HU3D_ATTR_DISPOFF);
}

/* Create the paired particle systems used by the player event-panel effect. */
s32 fn_1_1A2D0(s16 cameraMask, s32 hookType) {
    HU3D_PARTICLE *work;
    s32 index;
    s16 particleId;

    lbl_1_bss_AC = Hu3DParticleCreate(lbl_1_bss_58, 100);
    particleId = lbl_1_bss_AC;
    work = (HU3D_PARTICLE *)Hu3DData[particleId].hookData;
    Hu3DParticleBlendModeSet(particleId, HU3D_PARTICLE_BLEND_ADDCOL);
    Hu3DModelLayerSet(particleId, 6);
    Hu3DModelPosSet(particleId, 0.0f, 0.0f, 0.0f);
    Hu3DParticleAttrSet(particleId, (HU3D_PARTICLE_ATTR_SCALEY | (1 << 5) | (1 << 6)));
    if (hookType == 0) {
        Hu3DParticleHookSet(particleId, fn_1_1A968);
    } else {
        Hu3DParticleHookSet(particleId, fn_1_1AC20);
    }
    for (index = 0; index < work->maxCnt; index++) {
        memset(&work->data[index], 0, sizeof(work->data[index]));
        work->data[index].time = -1;
        work->data[index].cameraBit = cameraMask;
    }

    lbl_1_bss_AA = Hu3DParticleCreate(lbl_1_bss_54, 100);
    particleId = lbl_1_bss_AA;
    work = (HU3D_PARTICLE *)Hu3DData[particleId].hookData;
    Hu3DParticleBlendModeSet(particleId, HU3D_PARTICLE_BLEND_ADDCOL);
    Hu3DModelLayerSet(particleId, 6);
    Hu3DModelPosSet(particleId, 0.0f, 0.0f, 0.0f);
    Hu3DParticleAttrSet(particleId, (HU3D_PARTICLE_ATTR_SCALEY | (1 << 5) | (1 << 6)));
    Hu3DParticleHookSet(particleId, fn_1_1A968);
    for (index = 0; index < work->maxCnt; index++) {
        memset(&work->data[index], 0, sizeof(work->data[index]));
        work->data[index].time = -1;
        work->data[index].cameraBit = HU3D_CAM2;
    }
}

/* Create a selected particle system and initialize its entries before an effect burst. */
s32 fn_1_1A524(s16 cameraMask, s32 hookType, s32 particleType) {
    HU3D_PARTICLE *work;
    HU3D_PARTICLE_DATA *entries;
    s32 index;

    if (particleType == 0) {
        lbl_1_bss_A8 = Hu3DParticleCreate(lbl_1_bss_58, 100);
        Hu3DParticleBlendModeSet(lbl_1_bss_A8, HU3D_PARTICLE_BLEND_ADDCOL);
    } else {
        lbl_1_bss_A8 = Hu3DParticleCreate(lbl_1_bss_54, 100);
        Hu3DParticleBlendModeSet(lbl_1_bss_A8, HU3D_PARTICLE_BLEND_ADDCOL);
    }

    work = (HU3D_PARTICLE *)Hu3DData[lbl_1_bss_A8].hookData;
    Hu3DModelLayerSet(lbl_1_bss_A8, 6);
    Hu3DModelPosSet(lbl_1_bss_A8, 0.0f, 0.0f, 0.0f);
    Hu3DParticleAttrSet(lbl_1_bss_A8, (HU3D_PARTICLE_ATTR_SCALEY | (1 << 5) | (1 << 6)));
    if (hookType == 0) {
        Hu3DParticleHookSet(lbl_1_bss_A8, fn_1_1A968);
    } else {
        Hu3DParticleHookSet(lbl_1_bss_A8, fn_1_1AC20);
    }

    for (index = 0; index < work->maxCnt; index++) {
        memset(&work->data[index], 0, sizeof(work->data[index]));
        work->data[index].time = -1;
        work->data[index].cameraBit = cameraMask;
    }
}

/* Release both particle systems after their event-panel presentation is finished. */
void fn_1_1A6E8(void) {
    Hu3DModelKill(lbl_1_bss_AC);
    Hu3DModelKill(lbl_1_bss_AA);
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
    Hu3DModelRotSet(modelId, 0.0f, rotationY, 0.0f);
    Hu3DModelPosSet(modelId, positionX, positionY, positionZ);
    hook = (HU3D_PARTICLE *)Hu3DData[modelId].hookData;
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

    Hu3DModelRotSet(lbl_1_bss_A8, 0.0f, rotationY, 0.0f);
    Hu3DModelPosSet(lbl_1_bss_A8, positionX, positionY, positionZ);
    hook = (HU3D_PARTICLE *)Hu3DData[lbl_1_bss_A8].hookData;
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
            angle = (2.0f) * frandf();
            radius = frandf();
            /* This random sample's value is unused. */
            randomValue = rand8() % 14;
            entry->time++;
            entry->parManId = rand8() % 32;
            entry->vel.x = (f32)(sin((3.141592653589793) * (f64)angle) * radius * (3.0));
            entry->vel.y = (f32)(cos((3.141592653589793) * (f64)angle) * radius * (3.0));
            entry->vel.z = (0.0f);
            entry->pos.x += sin((3.141592653589793) * (f64)angle) * radius * (20.0);
            entry->pos.y += cos((3.141592653589793) * (f64)angle) * radius * (20.0);
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
                entry->scale = (0.0f);
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
                sin(3.141592653589793 * (f64)angle)));
            particle->vel.y = (f32)(3.0 * ((f64)magnitude *
                cos(3.141592653589793 * (f64)angle)));
            particle->vel.z = 0.0f;
            particle->pos.x = (f32)((f64)particle->pos.x +
                80.0 * ((f64)magnitude * sin(3.141592653589793 * (f64)angle)));
            particle->pos.y = (f32)((f64)particle->pos.y +
                80.0 * ((f64)magnitude * cos(3.141592653589793 * (f64)angle)));
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
        timing = omObjGetWork((OMOBJ *)lbl_1_bss_28, QuizOverlayWork);
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
        work = (QuizOverlayWork *)((OMOBJ *)lbl_1_bss_28)->work;
        work->flags.removeObject = 1;
        while (lbl_1_bss_28 != 0) {
            HuPrcVSleep();
        }
    }
    lbl_1_bss_28 = omAddObjEx(lbl_1_bss_288, 101, 0, 0, -1, fn_1_1B28C);
    omSetStatBit(lbl_1_bss_28, (OM_STAT_NOPAUSE | OM_STAT_SPRPAUSE));
    if (duration <= 0) {
        duration = 1;
    }

    work = (QuizOverlayWork *)((OMOBJ *)lbl_1_bss_28)->work;
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
    work->cameraModel = Hu3DHookFuncCreate(fn_1_1B3EC);
    Hu3DModelCameraSet(work->cameraModel, 4);
    Hu3DModelLayerSet(work->cameraModel, 1);
}

/* Report whether the full-screen overlay has reached its held opacity or has been removed. */
s32 fn_1_1B18C(void) {
    s32 active;
    QuizOverlayWork *timing;

    if ((u32)lbl_1_bss_28 == 0) {
        return 1;
    }
    timing = omObjGetWork((OMOBJ *)lbl_1_bss_28, QuizOverlayWork);
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
        hookWork = omObjGetWork((OMOBJ *)lbl_1_bss_28, QuizOverlayWork);
        Hu3DModelCameraSet(hookWork->cameraModel, cameraMask);
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

    motion = omObjGetWork(object, QuizOverlayWork);
    if (motion->flags.removeObject) {
        if (motion->cameraModel != -1) {
            Hu3DModelKill(motion->cameraModel);
        }
        lbl_1_bss_28 = 0;
        omDelObjEx(HuPrcCurrentGet(), object);
    } else if (!motion->flags.stopUpdating) {
        /* Advance the screen overlay opacity by its per-frame fade rate. */
        alphaFloat = __OSu8tof32(&motion->color.a);
        alphaFloat += motion->rate;
        /* Keep the current opacity in the byte used by the overlay draw hook. */
        motion->color.a = __OSf32tou8(alphaFloat);
        if (motion->remainingFrames > 0) {
            motion->remainingFrames--;
        } else if (motion->rate > (0.0f)) {
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
        timing = omObjGetWork((OMOBJ *)lbl_1_bss_28, QuizOverlayWork);
        left = (0.0f);
        right = (640.0f);
        top = (0.0f);
        bottom = (480.0f);
        C_MTXOrtho(projection, top, bottom, left, right,
                   (0.0f), (10.0f));
        GXSetProjection(projection, GX_ORTHOGRAPHIC);
        PSMTXIdentity(identity);
        GXLoadPosMtxImm(identity, 0);
        GXSetCurrentMtx(0);
        GXSetViewport((0.0f), (0.0f), right,
                      (1.0f) + bottom,
                      (0.0f), (1.0f));
        GXSetScissor(0, 0, (u32)right, (u32)((1.0f) + bottom));
        GXClearVtxDesc();
        GXSetChanMatColor(GX_COLOR0A0, timing->color);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG,
                      0, GX_DF_NONE, GX_AF_NONE);
        GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_REG,
                      0, GX_DF_NONE, GX_AF_NONE);
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

    angle = (0.0f);
    for (;;) {
        HuPrcVSleep();
        sineOffset = (f32) ((5.0) * sin(((3.141592653589793) * (f64) angle) / (180.0)));
        if ((s32) lbl_1_bss_30C == 0) {
            HuSprBankSet(lbl_1_bss_310, 0, 0);
            HuSprPosSet(lbl_1_bss_310, 0, lbl_1_bss_304.x, lbl_1_bss_304.y - sineOffset);
        } else if ((s32) lbl_1_bss_30C == 1) {
            HuSprBankSet(lbl_1_bss_310, 0, 1);
            HuSprPosSet(lbl_1_bss_310, 0, lbl_1_bss_304.x, lbl_1_bss_304.y + sineOffset);
        } else if ((s32) lbl_1_bss_30C == 2) {
            HuSprBankSet(lbl_1_bss_310, 0, 2);
            HuSprPosSet(lbl_1_bss_310, 0, lbl_1_bss_304.x + sineOffset,
                        lbl_1_bss_304.y - sineOffset);
        }
        angle += (18.0f);
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
    OM_CAMERA_VIEW cameraView;
    s32 frame;

    HuPrcChildCreate(fn_1_1BBD8, 100, 4096, 0, lbl_1_bss_288);
    fn_1_20E64(7);
    fn_1_1DFDC();
    HuPrcChildCreate(fn_1_1BB18, 100, 4096, 0, lbl_1_bss_288);
    cameraView = lbl_1_data_534;
    omCameraViewMoveSimpleMulti(4, &cameraView, 90);
    for (frame = 0; frame < 90; frame++) {
        HuPrcVSleep();
    }
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

    Hu3DModelPosGet(lbl_1_bss_2A6[0], &leftPosition);
    Hu3DModelPosGet(lbl_1_bss_2A6[1], &rightPosition);
    distance = (0.0f);
    frame = 0;
    while (frame < 60) {
        HuPrcVSleep();
        distance += (5.0f);
        Hu3DModelPosSet(lbl_1_bss_2A6[0], leftPosition.x - distance, leftPosition.y,
                        leftPosition.z);
        Hu3DModelPosSet(lbl_1_bss_2A6[1], rightPosition.x + distance, rightPosition.y,
                        rightPosition.z);
        frame += 1;
    }
    HuPrcEnd();
waitAfterExit:
    HuPrcVSleep();
    goto waitAfterExit;
}

/* Wait for controller Start during the tutorial, then return through the overlay history. */
void fn_1_1BBD8(void) {
    OMOVLHIS *history;

    history = omOvlHisGet(0);
    do {
        HuPrcVSleep();
    } while ((s32) (HuPadBtnDown[0] & PAD_BUTTON_START) == 0);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 20);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 20);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    omOvlHisChg(0, DLL_micquizdll, 2, history->stat);
    omOvlReturnEx(0, 1);
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
        rotationOffset = (f32)(360.0 * sin((3.141592653589793 * angle) / 180.0));
        angle += 1.5f;
        Hu3DModelRotSet(lbl_1_bss_198[levelIndex].model[columnIndex], 0.0f,
                        180.0f + rotationOffset, 0.0f);
        if ((angle >= 45.0f) && (bankChanged == 0)) {
            bankChanged = 1;
            Hu3DAnimBankSet(lbl_1_bss_102[levelIndex].model[columnIndex], (u16)(bank + 1));
        }
    } while (!(angle >= 90.0f));
}

/* Alternate presenter dialogue at the end of the tutorial demonstration. */
void fn_1_1BE00(void) {
    fn_1_20B74(1);
    fn_1_A3B4(1, 1, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 27), 1, 1);
    fn_1_20C94();
    fn_1_A3B4(1, 0, 8.0f, HU3D_MOTATTR_LOOP);

    fn_1_20B74(0);
    fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 28), 1, 0);
    fn_1_20C94();
    fn_1_A3B4(0, 0, 8.0f, HU3D_MOTATTR_LOOP);

    fn_1_20B74(1);
    fn_1_A3B4(1, 1, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 29), 1, 1);
    fn_1_20C94();
}

/* Explain three panel states in the tutorial, rotating the selected panel between lines. */
void fn_1_1C300(s32 levelIndex, s32 columnIndex)
{
    fn_1_20B74(1);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 23), 1, 1);
    fn_1_20C94();
    fn_1_1BC74(levelIndex, columnIndex, 1);

    fn_1_20B74(0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 24), 1, 0);
    fn_1_20C94();
    fn_1_1BC74(levelIndex, columnIndex, 2);

    fn_1_20B74(1);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 25), 1, 1);
    fn_1_20C94();
    fn_1_1BC74(levelIndex, columnIndex, 3);

    fn_1_20B74(0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 26), 1, 0);
    fn_1_20C94();
}

/* Point at the tutorial example cell and demonstrate its event reveal. */
void fn_1_1CAE4(void) {
    HuSprAttrReset(lbl_1_bss_310, 0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_304.x = lbl_1_data_E4C[4];
    lbl_1_bss_304.y = lbl_1_data_E4C[5];
    fn_1_20B74(0);
    fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 22), 1, 0);
    fn_1_20C94();
    fn_1_A3B4(0, 0, 8.0f, HU3D_MOTATTR_LOOP);
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

    fn_1_20B74(0);
    fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 18), 1, 0);
    fn_1_20C94();
    fn_1_A3B4(0, 0, 8.0f, HU3D_MOTATTR_LOOP);

    lbl_1_bss_304.x = lbl_1_data_E4C[2];
    lbl_1_bss_304.y = lbl_1_data_E4C[3];
    fn_1_20B74(1);
    fn_1_A3B4(1, 1, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 19), 1, 1);
    fn_1_20C94();
    fn_1_A3B4(1, 0, 8.0f, HU3D_MOTATTR_LOOP);

    lbl_1_bss_304.x = lbl_1_data_E4C[0];
    lbl_1_bss_304.y = lbl_1_data_E4C[1];
    fn_1_20B74(0);
    fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 20), 1, 0);
    fn_1_20C94();
    fn_1_A3B4(0, 0, 8.0f, HU3D_MOTATTR_LOOP);

    fn_1_CE28(0, 0, 8, 0);
    fn_1_20B74(1);
    fn_1_A3B4(1, 1, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 21), 1, 1);
    fn_1_20C94();
    fn_1_A3B4(1, 0, 8.0f, HU3D_MOTATTR_LOOP);
    HuSprAttrSet(lbl_1_bss_310, 0, HUSPR_ATTR_DISPOFF);
}

/* Run the pointer-position demonstration while the two presenters explain the grid. */
void fn_1_1D580(void) {
    lbl_1_bss_70 = HuPrcChildCreate(fn_1_1B94C, 100, 4096, 0, lbl_1_bss_288);
    HuSprAttrReset(lbl_1_bss_310, 0, HUSPR_ATTR_DISPOFF);

    fn_1_20B74(0);
    fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 16), 1, 0);
    fn_1_20C94();
    fn_1_A3B4(0, 0, 8.0f, HU3D_MOTATTR_LOOP);

    fn_1_20B74(1);
    fn_1_A3B4(1, 1, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 17), 1, 1);
    fn_1_20C94();
    fn_1_A3B4(1, 0, 8.0f, HU3D_MOTATTR_LOOP);

    HuPrcKill(lbl_1_bss_70);
    HuSprAttrSet(lbl_1_bss_310, 0, HUSPR_ATTR_DISPOFF);
}

/* Explain the five grid rows, alternating presenters and moving the tutorial pointer. */
void fn_1_1D994(void) {
    s32 messageIndex;

    fn_1_20B74(0);
    fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 10), 1, 0);
    fn_1_20C94();
    fn_1_A3B4(0, 0, 8.0f, HU3D_MOTATTR_LOOP);
    HuSprAttrReset(lbl_1_bss_310, 0, HUSPR_ATTR_DISPOFF);

    messageIndex = 0;
    while (messageIndex < 5) {
        lbl_1_bss_304.x = lbl_1_data_DFC[messageIndex][0];
        lbl_1_bss_304.y = lbl_1_data_DFC[messageIndex][1];
        fn_1_20B74(1);
        fn_1_A3B4(1, 1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 11) + messageIndex, 1, 1);
        fn_1_20C94();
        fn_1_A3B4(1, 0, 8.0f, HU3D_MOTATTR_LOOP);
        messageIndex++;
        if (messageIndex >= 5) {
            break;
        }
        lbl_1_bss_304.x = lbl_1_data_DFC[messageIndex][0];
        lbl_1_bss_304.y = lbl_1_data_DFC[messageIndex][1];
        fn_1_20B74(0);
        fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 11) + messageIndex, 1, 0);
        fn_1_20C94();
        fn_1_A3B4(0, 0, 8.0f, HU3D_MOTATTR_LOOP);
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
        fn_1_20B74(0);
        fn_1_A3B4(0, 1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 0) + messageId, 1, 0);
        fn_1_20C94();
        fn_1_A3B4(0, 0, 8.0f, HU3D_MOTATTR_LOOP);
        messageId++;

        fn_1_20B74(1);
        fn_1_A3B4(1, 1, 8.0f, HU3D_MOTATTR_LOOP);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_TUTORIAL, 0) + messageId, 1, 1);
        fn_1_20C94();
        fn_1_A3B4(1, 0, 8.0f, HU3D_MOTATTR_LOOP);
        messageId++;
    }

    fn_1_A3B4(1, 0, 8.0f, HU3D_MOTATTR_LOOP);
}

/* Configure the duration, amplitude, decay, and angular step of a camera shake. */
void fn_1_1E3F8(u16 duration, f32 scale, f32 scaleDecay, f32 angleStep) {
    lbl_1_bss_4C = (s32) duration;
    lbl_1_bss_44 = scale;
    lbl_1_bss_40 = scaleDecay;
    lbl_1_bss_38 = angleStep;
    lbl_1_bss_3C = (90.0f);
}

/* Apply the decaying vertical camera shake each scheduler frame while its timer is active. */
void fn_1_1E448(void) {
    for (;;) {
        HuPrcVSleep();
        lbl_1_bss_50 = (0.0f);
        if ((s32) lbl_1_bss_4C <= 0) {
            continue;
        }
        lbl_1_bss_50 =
            (f32) ((f64) lbl_1_bss_44 * sin(((3.141592653589793) * (f64) lbl_1_bss_3C) / (180.0)));
        lbl_1_bss_3C += lbl_1_bss_38;
        lbl_1_bss_44 *= lbl_1_bss_40;
        lbl_1_bss_4C -= 1;
        CenterM[2].y = lbl_1_data_5F8.center.y + lbl_1_bss_50;
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
        playerOrder[index] = lbl_1_bss_4D8->playerScore[index];
        sortedValues[index] = lbl_1_bss_4D8->playerScore[index];
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
        playerKinds[index] = lbl_1_bss_4D8->playerCharacter[index];
        playerValues[index] = lbl_1_bss_4D8->playerScore[index];
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
        if (lbl_1_bss_4D8->playerCount == 4) {
            fn_1_1FC10(playerKinds[3]);
        }
        break;
        }
    }
}

/* Show and close the single-presenter standings summary. */
s32 fn_1_1E944(void)
{
    fn_1_20B74(3);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ, 69), 1, 3);
    fn_1_20C94();
    fn_1_20B24();
}

/* Test whether every active player has rank one for the closing standings commentary. */
s32 fn_1_1EA90(void)
{
    s32 index;

    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_4D8->playerActive[index] != 0
            && lbl_1_bss_4D8->playerRank[index] != 1) {
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
        if (lbl_1_bss_4D8->playerActive[index] != 0
            && lbl_1_bss_4D8->playerRank[index] == 1) {
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
        if (lbl_1_bss_4D8->playerActive[index] != 0) {
            if (lbl_1_bss_4D8->playerRank[index] == 1) {
                firstValue = lbl_1_bss_4D8->playerScore[index];
            }
            if (lbl_1_bss_4D8->playerRank[index] == 2) {
                secondValue = lbl_1_bss_4D8->playerScore[index];
            }
        }
    }
    return firstValue - secondValue;
}

/* Play the two-presenter closing exchange used for the selected standings case. */
void fn_1_1EC38(void) {
    fn_1_20B74(1);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_MORIAGE, 14), 1, 1);
    fn_1_20C94();
    fn_1_20B74(0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_MORIAGE, 15), 1, 0);
    fn_1_20C94();
    fn_1_20B24();
}

/* Play the alternate two-presenter closing exchange for the selected standings case. */
void fn_1_1EE70(void) {
    fn_1_20B74(1);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_MORIAGE, 12), 1, 1);
    fn_1_20C94();
    fn_1_20B74(0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_MORIAGE, 13), 1, 0);
    fn_1_20C94();
    fn_1_20B24();
}

/* Name the leading character in the closing standings exchange, then close the dialogue. */
void fn_1_1F0A8(s32 messageIndex) {
    fn_1_20B74(1);
    fn_1_20D08(1, lbl_1_data_A0[messageIndex], 0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_MORIAGE, 0), 1, 1);
    fn_1_20C94();
    fn_1_20B74(0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_MORIAGE, 1), 1, 0);
    fn_1_20C94();
    fn_1_20B24();
}

/* Name the leading character in the closing line used for a score gap from 100 through 149. */
void fn_1_1F318(s32 messageIndex) {
    fn_1_20B74(1);
    fn_1_20D08(1, lbl_1_data_A0[messageIndex], 0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_MORIAGE, 2), 1, 1);
    fn_1_20C94();
    fn_1_20B24();
}

/* Name the leading character in the closing line used for a score gap of at least 50. */
void fn_1_1F4A0(s32 messageIndex) {
    fn_1_20B74(1);
    fn_1_20D08(1, lbl_1_data_A0[messageIndex], 0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_MORIAGE, 3), 1, 1);
    fn_1_20C94();
    fn_1_20B24();
}

/* Name the leading character in the closing line used for a score gap of at least 20. */
void fn_1_1F628(s32 messageIndex) {
    fn_1_20B74(1);
    fn_1_20D08(1, lbl_1_data_A0[messageIndex], 0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_MORIAGE, 4), 1, 1);
    fn_1_20C94();
    fn_1_20B24();
}

/* Insert the remaining score gap as a number in the closing standings line. */
void fn_1_1F7B0(s32 value) {
    char number[16];

    fn_1_20B74(1);
    sprintf(number, "%d", value);
    fn_1_20D08(1, (u32)number, 0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_MORIAGE, 5), 1, 1);
    fn_1_20C94();
    fn_1_20B24();
}

/* Play the two-leader closing exchange. The caller supplies character IDs, but this function
 * uses those values as playerCharacter indices before looking up the character names. */
void fn_1_1F938(s32 firstPlayer, s32 secondPlayer) {
    u32 firstMessageId;
    u32 secondMessageId;

    fn_1_20B74(1);
    firstMessageId = lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[firstPlayer]];
    fn_1_20D08(1, firstMessageId, 0);
    secondMessageId = lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[secondPlayer]];
    fn_1_20D08(1, secondMessageId, 1);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_MORIAGE, 6), 1, 1);
    fn_1_20C94();
    fn_1_20B74(0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_MORIAGE, 7), 1, 0);
    fn_1_20C94();
    fn_1_20B24();
}

/* Play the three-way-tie closing exchange. The caller supplies the remaining character's ID,
 * but this function uses it as a playerCharacter index before looking up the name. */
void fn_1_1FC10(s32 playerIndex) {
    fn_1_20B74(1);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_MORIAGE, 8), 1, 1);
    fn_1_20C94();
    fn_1_20B74(0);
    fn_1_20D08(0, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerIndex]], 0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_MORIAGE, 9), 1, 0);
    fn_1_20C94();
    fn_1_20B24();
}

/* Play the closing two-presenter exchange used when no named-player line is needed. */
void fn_1_1FE98(void) {
    fn_1_20B74(1);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_MORIAGE, 10), 1, 1);
    fn_1_20C94();
    fn_1_20B74(0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_MORIAGE, 11), 1, 0);
    fn_1_20C94();
    fn_1_20B24();
}
