/* Microphone capture and spoken-response support used by game modes. */
#include "dolphin/math.h"
#include "dolphin.h"
#include "game/armem.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/main.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/window.h"
#include "messdir_enum.h"

#define MIC_ALLOC_TAG 805306368
#define MIC_SESSION_ALLOC_TAG 805306369
#define MIC_STATUS_SPRITE_DATA_NUM DATANUM(DATA_win, 50)
#define MIC_DEVICE_ALERT_SPRITE_DATA_NUM DATANUM(DATA_win, 51)
#define MIC_RECOGNIZED_WORD_CONFIRM_MESSAGE MESSNUM(MESS_MICQUIZ, 57)
#define MIC_WRONG_DEVICE_MESSAGE MESSNUM(MESS_SAF_TEST, 21)
#define MIC_DEVICE_UNAVAILABLE_MESSAGE MESSNUM(MESS_SAF_TEST, 20)
#define MIC_THRESHOLD_MISSED_RESPONSE 4294967294
#define MIC_ENGINE_CONDITION_RESPONSE 4294967292
#define MIC_RESPONSE_NO_RESULT 4294967295
#define MIC_FPSCR_FLAGS_KEEP_MASK 0xffffffef
#define MIC_CACHE_LINE_LAST_BYTE 31
#define MIC_CACHE_LINE_ALIGN_MASK 0xffffffe0
#define MIC_VOLUME_SAMPLE_BUFFER_SIZE 88200
#define MIC_RESPONSE_RECORD_SIZE 96
#define MIC_ID_NONE_VALUE 65535
#define MIC_SCORE_UNAVAILABLE 65535
#define MIC_CAPTURE_BUFFER_SIZE 12288
#define MIC_CHOICE_PROCESS_STACK_SIZE 8192
#define MIC_RESPONSE_CAPACITY 256
#define MIC_RESULT_WORD_CAPACITY 64
#define MIC_VOLUME_SAMPLE_COUNT 44100
#define MIC_VOLUME_ENERGY_SAMPLE_COUNT 1024
#define MIC_MES_HOME 11
#define MIC_MES_SPACE 16
#define MIC_MES_CHOICE 15
#define MIC_MES_INSERT 31

typedef void (*MCResponseCallback)(u16 *response);

extern s32 currentHeapHandle;

void *memcpy(void *, const void *, u32);
void *memset(void *, int, u32);
char *strcpy(char *, const char *);
char *strcat(char *, const char *);
int strcmp(const char *, const char *);
u32 strlen(const char *);
double log10(double);
void *HuMemDirectRealloc(HEAPID, void *, s32);
s32 WipeCheckIn(void);
void MICInit(void);
void M2SInit(void);
void M2SOpen(void);
void M2SClose(void);

typedef struct MicResultNode_s MicResultNode_s;

static void InitMCSelWin(void);
static void MCExtHandler(void);
static void MCSelWinFunc(void);
static void MCSelWinContextProc(void);
static void MCAnswerMain(void);
static void MCDeviceMesExec(void);
static void MCListenerFunc(void);
static void *MCThreadFunc(void *);
static void MCThreadWakeup(OSAlarm *, OSContext *);
static void MicNotifyCallBack(u32 callbackContext, s32 event,
                              u32 sample, u32 value);
static void MicResultCallBack(u32 callbackContext,
                              u32 callbackData, u32 resultNode);
static void MicResultExec(u32 callbackContext, u32 callbackData,
                          MicResultNode_s *root);
static s32 MicResultGet(s32);
static s32 MicWriteResponseBuf(s16 responseType, u16 *entries);
static void *MCDVDRead(const char *);
static char *MakeMCFilename(char *, char *);
static s32 ActivateContext(s16);
void HuMCMicSet(s32);
inline s32 HuMCMicSaveGet(void);
int HuMCProbe(u32 channel);
s32 HuMCMount(s32);
void HuMCMicSprKill(void);
void HuMCSelWinKill(void);
BOOL HuMCSelWinCheck(void);
void HuMCSelModeSet(s16);
void HuMCSelWinContextKill(void);
void HuMCSessionExportReset(void);
static int MicWriteResponse(s16, s32, void *, u16, s32);

#define s_Silence_Error_8023abff "Silence Error!!!!!!!!!!\n"
#define lbl_8023ABFF s_Silence_Error_8023abff
#define s_Mount_OK__8023ac18 "Mount OK!\n"
#define s_MIC_Error__Engine__x___8023ac23 "MIC Error: Engine(%x).\n"
#define s_threshold__d_not_reached_8023ace6 (lbl_8023A988 + 862)
#define s_Abnormal_condition___d__s_at_sam_8023ad00 (lbl_8023A988 + 888)
#define s_SPEECH_DETECTED_8023ad28 (lbl_8023A988 + 928)
#define s_LISTENING_8023ad39 (lbl_8023A988 + 945)
#define s_STOP_LISTEN_8023ad44 (lbl_8023A988 + 956)
#define s_SILENCE_DETECTED_8023ad51 (lbl_8023A988 + 969)

typedef struct MCVolData_s {
    s16 *sample; /* Ring buffer of microphone PCM samples. */
    s32 sampleNo; /* Next write position in the 44,100-sample PCM history buffer. */
    u32 index; /* Microphone driver's next sample index. */
} MCVolData_s;

typedef struct MCLanguageData_s {
    char japanese[39]; /* Japanese language asset name. */
    char english[41]; /* English language asset name. */
    char *file[6]; /* Language asset path selected for each game language. */
} MCLanguageData_s;

typedef struct MCLngFileTbl_s {
    char *file[6]; /* Recognition-language data path for each game language. */
} MCLngFileTbl_s;

typedef struct MCResponseEntry_s {
    u16 score; /* Recognition score for this candidate. */
    s16 count; /* Number of recognized words in result. */
    s16 *result; /* Word indices in shared speech storage, or the pad window process's selected
                  * value. */
} MCResponseEntry_s;

typedef struct MCResponse_s {
    s16 status; /* Zero for success; negative values report errors, cancellation, timeout or no
                 * result. */
    u16 score; /* Recognition score of the first candidate. */
    s16 count; /* Number of recognized words in the first candidate. */
    s16 *result; /* Word indices of the first candidate. */
    s32 word; /* First speech-word index, a one-based pad-choice value, or -1 when unavailable. */
    MCResponseEntry_s entry[10]; /* Candidates in recognition rank order. */
} MCResponse_s;

typedef struct MCResponseData_s {
    u32 data[24]; /* One 96-byte microphone response record. */
} MCResponseData_s;

typedef struct MCContextData_s {
    char path[64]; /* Base path used to load this recognition context. */
    void *ctxData; /* Loaded .ctx recognition-context file. */
    void *gcdData; /* Loaded .gcd grammar data file. */
    void *wrdData; /* Loaded .wrd word data file. */
    void *binData; /* Loaded .bin word-index table. */
    void *context; /* Recognition-engine context created from the loaded files. */
} MCContextData_s;

struct MicResultNode_s {
    s32 status; /* Recognition-engine status copied to every candidate; not read here. */
    u16 count; /* Number of entries in result, including engine boundary entries. */
    u16 score; /* Recognition score for this candidate. */
    s32 *result; /* Engine word-offset sequence for the candidate. */
    struct MicResultNode_s *next; /* Next candidate in rank order. */
};

typedef struct MicResponseEntry_s {
    u16 score; /* Recognition score copied to the response queue. */
    s16 count; /* Number of recognized words in result. */
    s16 *result; /* Word indices stored in the shared result buffer. */
} MicResponseEntry_s;

typedef struct MCSelWinWork_s {
    s16 winId; /* Active choice-window ID, or -1 when no window exists. */
    s16 choice; /* Selection-window state: idle, display, or hide. */
    HUPROCESS *proc; /* Process that updates the selection window. */
    f32 x; /* Requested window x position, including alignment sentinels. */
    f32 y; /* Requested window y position, including alignment sentinels. */
    u8 *item; /* Encoded message-choice list passed to the window system. */
    u8 *order; /* Shuffled message indices used by the displayed choices. */
} MCSelWinWork_s;

static u8 lbl_80287554[264];
static MCVolData_s MCVolData;
static char MCFileName[64];
static OSAlarm MCThreadAlarm;
static u8 MCUnkResponseData[96];
static MCSelWinWork_s MCSelWinWork;
static u8 MCCurResponse[96];
static u32 PlayerSession[4] ATTRIBUTE_ALIGN(32);
static OSMessageQueue MCMessageQueue;
static OSThread MCThread;

static u32 gap_10_802C0564_sbss;
static u8 *MCThreadStack;
static void *LngData;
static void *M2SBuffer;
static void *MicBuffer;
static s16 *MCResultData;
static u16 gap_10_802C054E_sbss;
static s16 MCResultNum;
static s32 MCResponseLastNo;
static s32 MCResponseNo;
static MCContextData_s *MCContext;
static u16 gap_10_802C053E_sbss;
static s16 ContextCur;
static MCContextData_s *MCContextP;
static s32 MCStat;
static HUPROCESS *MCAnswerProc;
static s32 MCListenF;
static MCResponseCallback MCContextCallback;
static u16 gap_10_802C0526_sbss;
static s16 MCSprGrpId;
static OSMessage MCMessageArray[2];
static OSThreadQueue MCThreadQueue;
static s32 MC_gsapiEngineError;
static s32 MicOpenF;
static void *MCSessionP;
static s8 MCSessionTimer[4];
static MCResponseCallback MCCallback;
static s32 MCSelWinMaxTime;
static s32 ValidResultF;
static s32 LanguageNo;
static s32 MCWrongDeviceF;
static HUPROCESS *MCListenerProc;
static s32 MCInitF;
static u32 MCButtonDown;
static u32 MCButton;
static u8 *MCResponseBuf;
static void *MC_gsapiEngine;

static s32 MCSprStat = 3;
static s16 MCSessionCur = -1;
static s16 MCSessionPrev = -1;
static s32 HeapNum = MIC_ALLOC_TAG;
static s16 MCYesNoCtxId = -1;
static s32 MCMicValue = -1;
static s32 MCThreshold = 4000;
static s32 M2SShift = -1;

static char lbl_802BF984[] = "US";
static char lbl_802BF987[] = ".ctx";
static char lbl_802BF98C[] = ".gcd";
static char lbl_802BF991[] = ".wrd";
static char lbl_802BF996[] = ".bin";
static char lbl_802BF99B[] = "Ouch!\n";
static char lbl_802BF9A2[] = "BAD SNR";
static char gap_09_802BF9AA_sdata[6] = {0, 0, 0, 0, 0, 0};

static char lbl_8023A988[] = "/mic/lng/asr16v220_jpj200_float_4b.lng";
static char lbl_8023A9AF[] = "/mic/lng/asr16v220_enu300_float_4b.lng";
static char *LngFileTbl[] = {
    lbl_8023A988, lbl_8023A9AF, lbl_8023A9AF,
    lbl_8023A9AF, lbl_8023A9AF, lbl_8023A9AF
};

static const f32 lbl_802C1E48 = 0.0f;
static const f32 lbl_802C1E4C = -100.0f;
static const f64 lbl_802C1E50 = -10000.0;
static const f32 lbl_802C1E58 = 16.0f;
static const f64 lbl_802C1E60 = -10001.0;
static const f32 lbl_802C1E68 = 560.0f;
static const f32 lbl_802C1E6C = 0.8f;
static const f64 lbl_802C1E70 = -10002.0;
static const f32 lbl_802C1E78 = 40.0f;
static const f64 lbl_802C1E80 = -10003.0;
static const f32 lbl_802C1E88 = 440.0f;
static const f32 lbl_802C1E8C = 60.0f;
static const f64 lbl_802C1E90 = 4503601774854144.0;
static const f32 lbl_802C1E98 = -10000.0f;
static const f32 lbl_802C1E9C = 200.0f;
static const f32 lbl_802C1EA0 = 0.9f;
static const f32 lbl_802C1EA4 = 288.0f;
static const f32 lbl_802C1EA8 = 240.0f;
static const f32 lbl_802C1EAC = 20.0f;
static const f64 lbl_802C1EB0 = 10.0;
static const f32 lbl_802C1EB8 = 1099511600000.0f;
static const f64 lbl_802C1EC0 = 4503599627370496.0;

/* Called during game startup to reset microphone state and detect the device. */

void HuMCSysInit(void)

{
  short playerIndex;

  MCResponseBuf = (u8 *)(MCContext = 0);
  MCInitF = 0;
  MicOpenF = 1;
  MCAnswerProc = 0;
  MCThreadStack = 0;
  ContextCur = MIC_ID_NONE_VALUE;
  HeapNum = MIC_ALLOC_TAG;
  InitMCSelWin();
  /* Disable floating-point exception reporting before initializing speech support. */
  OSSetErrorHandler(OS_ERROR_FPE,0);
  __OSFpscrEnableBits = __OSFpscrEnableBits & MIC_FPSCR_FLAGS_KEEP_MASK;
  if (HuMCProbe(1) == 0) {
    HuMCMicSet(1);
  }
  else {
    HuMCMicSet(2);
  }
  MCSessionCur = MCSessionPrev = -1;
  for (playerIndex = 0; playerIndex < 4; playerIndex++) {
    PlayerSession[playerIndex] = 0;
    *(u8 *)((int)&MCSessionTimer + (int)playerIndex) = 0;
  }
  return;
}

/* Called by speech modes to load recognition data and open the recognition engine. */
/* The incoming value is ignored; the saved game language selects recognition data. Prerecord,
* stream-mode, capture-start, final threshold and thread-creation results are also ignored. */

s32 HuMCInit(s16 languageIndex)
{
  s32 apiResult;
  s16 contextIndex;
  s32 result;

  result = 0;
  if (MCResponseBuf != 0) {
    /* Existing response storage causes an immediate return without a defined status value. */
    return;
  }
  MCWrongDeviceF = 0;
  if (MCMicValue != -1) {
    HuMCMicSet(MCMicValue);
  }
  MCResponseBuf = HuMemDirectMallocNum(0,24576,MIC_ALLOC_TAG);
  MCResponseNo = MCResponseLastNo = 0;
  MCResultData = HuMemDirectMallocNum(0,128,MIC_ALLOC_TAG);
  MCResultNum = 0;
  MCContext = HuMemDirectMallocNum(0,sizeof(MCContextData_s) * 8,MIC_ALLOC_TAG);
  for (contextIndex = 0; contextIndex < 8; contextIndex++) {
    MCContext[contextIndex].context = 0;
  }
  MCSprGrpId = -1;
  ContextCur = -1;
  MCYesNoCtxId = -1;
  if (HuMCMicSaveGet() != 1) {
    return 0;
  }

  M2SShift = -1;
  gsapi_SetUserData(1000);
  if (MC_gsapiEngine != 0) {
    gsapi_EngineClose(MC_gsapiEngine);
  }
  MCThreshold = 4000;
  MICInit();
  M2SInit();
  if ((M2SBuffer = HuMemDirectMallocNum(0,256,MIC_ALLOC_TAG)) == 0) {
    OSReport("Mic Error: Not enough Memory\n");
    HuMemDirectFreeNum(0,MIC_ALLOC_TAG);
    return -128;
  }
  if (M2SSetBuffer(M2SBuffer) == 0) {
    OSReport("Mic Error: M2SSetBuffer()\n");
    HuMemDirectFreeNum(0,MIC_ALLOC_TAG);
    return -128;
  }
  if ((MicBuffer = HuMemDirectMallocNum(0,MIC_CAPTURE_BUFFER_SIZE,MIC_ALLOC_TAG)) == 0) {
    OSReport("Mic Error: Not enough Memory\n");
    HuMemDirectFreeNum(0,MIC_ALLOC_TAG);
    return -128;
  }

  /* Keep the mount status for the caller, but continue engine setup even when mounting fails. */
  result = HuMCMount(1);
  M2SSetPrerecordSamples(100);
  M2SSetMode(3);
  MICStart(1);
  if ((apiResult = gsapi_Init(MicResultCallBack,0)) != 0) {
    OSReport("Mic Error: gsapi_Init() %x\n",apiResult);
    HuMemDirectFreeNum(0,MIC_ALLOC_TAG);
    return -128;
  }
  languageIndex = GWLanguageGet();
  LanguageNo = languageIndex;
  if ((LngData = MCDVDRead(LngFileTbl[LanguageNo])) == 0) {
    OSReport("Mic Error: Read Language Failue\n");
    gsapi_Close();
    HuMemDirectFreeNum(0,MIC_ALLOC_TAG);
    return -128;
  }
  if ((apiResult = gsapi_LanguageLoadBuffer(LngData,0)) != 0) {
    OSReport("Mic Error: gsapi_LanguageLoadBuffer().%x\n",apiResult);
    gsapi_Close();
    HuMemDirectFreeNum(0,MIC_ALLOC_TAG);
    return -128;
  }
  if ((apiResult = gsapi_EngineOpen(0,&MC_gsapiEngine)) != 0) {
    OSReport("Mic Error: gsapi_EngineOpen().%x\n",apiResult);
    gsapi_Close();
    HuMemDirectFreeNum(0,MIC_ALLOC_TAG);
    return -128;
  }
  MCSessionP = 0;
  if ((apiResult = gsapi_NotifySetCallback(MicNotifyCallBack)) != 0) {
    OSReport("Mic Error: gsapi_NotifySetCallback().%x\n",apiResult);
    gsapi_Close();
    HuMemDirectFreeNum(0,MIC_ALLOC_TAG);
    return -128;
  }
  if ((apiResult = gsapi_EngineSetMode(MC_gsapiEngine,1)) != 0) {
    OSReport("Mic Error: gsapi_EngineSetMode().%x\n",apiResult);
    gsapi_Close();
    HuMemDirectFreeNum(0,MIC_ALLOC_TAG);
    return -128;
  }

  gsapi_EngineSetParam(MC_gsapiEngine,9,MCThreshold);
  MCStat = 0;
  MCListenF = 0;
  MCContextCallback = 0;
  OSInitMessageQueue(&MCMessageQueue,MCMessageArray,2);
  MCThreadStack = HuMemDirectMallocNum(0,16384,MIC_ALLOC_TAG);
  OSCreateThread(&MCThread,MCThreadFunc,0,MCThreadStack + 16384,
                 16384,OS_PRIORITY_MAX,1);
  OSResumeThread(&MCThread);
  OSInitThreadQueue(&MCThreadQueue);
  MCInitF = 1;
  OSReport("HEAP HEAP Malloc Size %x\n",HuMemUsedMallocSizeGet(0));
  OSReport("OSAlloc Size Left %dkb(%x)\n",OSCheckHeap(currentHeapHandle) / 1024,
           OSCheckHeap(currentHeapHandle));
  MCAnswerProc = HuPrcCreate(MCAnswerMain,65000,16384,0);
  HuPrcSetStat(MCAnswerProc,HU_PRC_STAT_PAUSE_ON | HU_PRC_STAT_UPAUSE_ON);
  MCMicValue = -1;
  return result;
}

/* Called when a speech-enabled mode exits to stop capture and release its resources. */

void HuMCClose(void)

{
  if (MCResponseBuf != 0) {
    MCInitF = 0;
    if ((MCThreadStack != 0) && (HuMCMicSaveGet() == 1)) {
      OSSendMessage(&MCMessageQueue,(OSMessage)2,1);
      OSSleepThread(&MCThreadQueue);
    }
    HuMCMicSprKill();
    if (HuMCMicSaveGet() == 1) {
      MICStop(0);
      MICStop(1);
    }
    HuMCSessionExportReset();
    if (MCAnswerProc != 0) {
      HuPrcKill(MCAnswerProc);
    }
    MCAnswerProc = 0;
    if (MCListenerProc != 0) {
      HuPrcKill(MCListenerProc);
    }
    MCListenerProc = 0;
    if (MCThreadStack != 0) {
      OSCancelThread(&MCThread);
    }
    MCThreadStack = 0;
    HuMCSelWinKill();
    if (MC_gsapiEngine != 0) {
      gsapi_EngineClose(MC_gsapiEngine);
      gsapi_LanguageUnLoad();
      /* Call EngineClose again after unloading the language; both close results are ignored. */
      gsapi_EngineClose(MC_gsapiEngine);
      gsapi_Close();
      MC_gsapiEngine = 0;
    }
    HuMemDirectFreeNum(0,MIC_ALLOC_TAG);
    HuMemDirectFreeNum(2,MIC_ALLOC_TAG);
    MCResponseBuf = (u8 *)(MCContext = 0);
    MICUnmount(0);
    MICUnmount(1);
    if (MCMicValue != -1) {
      HuMCMicSet(MCMicValue);
    }
    MCMicValue = -1;
  }
  return;
}

/* Called by a speech mode to load its word files and create a recognition context. */

s16 HuMCContextCreate(char *path)

{
  char *localizedPath;
  char *extensionScan;
  char *contextFilename;
  MCContextData_s *contextSlot;
  s16 contextIndex;
  s32 contextError;

  if (HuMCMicSaveGet() != 1) {
    return -1;
  }
  localizedPath = HuMemDirectMalloc(0,256);
  strcpy(localizedPath,path);
  /* All non-Japanese languages append US to the full path; the dot extensionScan does not alter
   * it. */
  if (LanguageNo != 0) {
    extensionScan = localizedPath;
    while ((*extensionScan != '\0') && (*extensionScan != '.')) {
      extensionScan++;
    }
    switch (LanguageNo) {
      case 1:
        strcat(localizedPath,lbl_802BF984);
        break;
      default:
        strcat(localizedPath,lbl_802BF984);
        break;
    }
  }
  /* Compare the original path in occupied slots before the first free slot; stored paths may end in
   * US. */
  for (contextIndex = 0; contextIndex < 8; contextIndex++) {
    if (MCContext[contextIndex].context == 0) {
      break;
    }
    if (strcmp(path,MCContext[contextIndex].path) == 0) {
      HuMemDirectFree(localizedPath);
      return contextIndex;
    }
  }
  if (contextIndex == 8) {
    HuMemDirectFree(localizedPath);
    return -1;
  }
  contextSlot = &MCContext[contextIndex];
  strcpy(contextSlot->path,localizedPath);
  contextFilename = MakeMCFilename(localizedPath,lbl_802BF987);
  contextSlot->ctxData = MCDVDRead(contextFilename);
  contextFilename = MakeMCFilename(localizedPath,lbl_802BF98C);
  contextSlot->gcdData = MCDVDRead(contextFilename);
  contextFilename = MakeMCFilename(localizedPath,lbl_802BF991);
  contextSlot->wrdData = MCDVDRead(contextFilename);
  contextFilename = MakeMCFilename(localizedPath,lbl_802BF996);
  contextSlot->binData = MCDVDRead(contextFilename);
  /* Context-data errors are reported without aborting; parameter results are ignored, and the
   * slot contextIndex is still returned. */
  contextError = gsapi_ContextSetCtxData(contextSlot->ctxData,&contextSlot->context);
  if (contextError != 0) {
    OSReport("Error CTX %x\n",contextError);
  }
  contextError = gsapi_ContextSetGcdData(contextSlot->context,contextSlot->gcdData);
  if (contextError != 0) {
    OSReport("Error GCD %x\n",contextError);
  }
  contextError = gsapi_ContextSetWrdData(contextSlot->context,contextSlot->wrdData);
  if (contextError != 0) {
    OSReport("Error WRD %x\n",contextError);
  }
  gsapi_ContextSetParam(contextSlot->context,9,MCThreshold);
  if ((omcurovl == DLL_m669dll) || (omcurovl == DLL_m670dll) || (omcurovl == DLL_m665dll) ||
      (omcurovl == DLL_micquizdll) || (omcurovl == DLL_micquizishidll) ||
      (omcurovl == DLL_micquizohdedll) || (omcurovl == DLL_micquizmyokodll)) {
    gsapi_ContextSetParam(contextSlot->context,10,200);
  }
  else {
    gsapi_ContextSetParam(contextSlot->context,10,100);
  }
  HuMemDirectFree(localizedPath);
  return contextIndex;
}

/* When a speech mode releases a context, request deactivation, ignore its result, and free its
 * .ctx, .gcd and .wrd files. The .bin buffer remains until microphone shutdown. */

void HuMCContextKill(short contextId)

{
  MCContextData_s *context;

  if (contextId >= 0) {
    context = &MCContext[contextId];
    if (context->context) {
      gsapi_ContextDeActivate(context->context);
      HuMemDirectFree(context->ctxData);
      HuMemDirectFree(context->gcdData);
      HuMemDirectFree(context->wrdData);
      context->context = NULL;
    }
  }
}

/* Sets the recognition threshold used when later contexts are configured. */

void HuMCThresholdSet(u32 threshold)

{
  MCThreshold = threshold;
  return;
}

/* Activates one-shot recognition, queues its start command and returns one. Mode and parameter
* results are ignored after activation. Pad fallback requests choices and returns zero. */

static char lbl_8023AB60[] = "MIC Error: gsapi_ContextActivate() %x\n";

s32 HuMCContextSet(s16 contextId)

{
  s32 result;
  if (HuMCSelWinCheck()) {
    MCResponseNo = MCResponseLastNo = 0;
    HuMCSelModeSet(1);
    return 0;
  }
  if (HuMCMicSaveGet() != 1) {
    return 0;
  }
  if ((MCStat != 0) || (MicOpenF != 0) || (contextId < 0) || (MC_gsapiEngine == 0)) {
    return 0;
  }
  if ((result = ActivateContext(contextId)) != 0) {
    OSReport(lbl_8023AB60,result);
    return 0;
  }
  MCContextP = &MCContext[contextId];
  MCResponseNo = MCResponseLastNo = 0;
  gsapi_EngineSetMode(MC_gsapiEngine,1);
  gsapi_EngineSetParam(MC_gsapiEngine,1,0);
  gsapi_EngineSetParam(MC_gsapiEngine,0,0);
  OSSendMessage(&MCMessageQueue,0,1);
  MCStat = 1;
  MCListenF = 0;
  MCContextCallback = 0;
  return 1;
}

/* Called while waiting for a one-shot speech result; waits for a queued response. */

static char lbl_8023AB87[] = "Error!!!!!!!!!!!!!!!!!!\n";

s32 HuMCStatGet(void)

{
  if (HuMCSelWinCheck()) {
    while (MCResponseNo == 0) {
      HuPrcVSleep();
    }
    return HuMCResponseGet2();
  }
  if (HuMCMicSaveGet() != 1) {
    return -1;
  }
  if ((MCStat != 1) || (MicOpenF != 0)) {
    return -2;
  }
  while (MCResponseNo == 0) {
    if (MCListenF != 0) {
      /* If listening stopped without a response, stop and restart the engine before waiting
       * again. */
      OSReport(lbl_8023AB87);
      OSSendMessage(&MCMessageQueue,(OSMessage)2,1);
      OSSleepThread(&MCThreadQueue);
      OSSendMessage(&MCMessageQueue,0,1);
      MCListenF = 0;
    }
    HuPrcVSleep();
  }
  return HuMCResponseGet2();
}

/* Called after a one-shot wait to stop listening and fetch its queued response. */

int HuMCResponseGet2(void)

{
  s16 response;

  if (HuMCSelWinCheck()) {
    HuMCSelModeSet(2);
    if (((MCResponse_s *)MCResponseBuf)[MCResponseLastNo].count) {
      goto sel_response_valid;
    }
    return -1;
sel_response_valid:
    return ((MCResponse_s *)MCResponseBuf)[MCResponseLastNo].word;
  }
  if (HuMCMicSaveGet() != 1) {
    return -1;
  }
  if ((MCStat != 1) || (MicOpenF != 0)) {
    return -2;
  }
  OSSendMessage(&MCMessageQueue,(OSMessage)2,1);
  OSSleepThread(&MCThreadQueue);
  MCStat = 0;
  if (MCResponseNo == 0) {
    return -1;
  }
  response = ((MCResponse_s *)MCResponseBuf)[MCResponseLastNo].status;
  if (response != 0) {
    return response;
  }
  if (MCResponseNo == MCResponseLastNo) {
    return -1;
  }
  if (((MCResponse_s *)MCResponseBuf)[MCResponseLastNo].count) {
    goto response_valid;
  }
  return -1;
response_valid:
  return ((MCResponse_s *)MCResponseBuf)[MCResponseLastNo].word;
}

/* Called by a speech mode to activate continuous recognition with an optional callback. Mode
* and parameter results are ignored; successful activation still queues a start and returns one. */

s32 HuMCContextCallbackSet(s16 contextId,MCResponseCallback responseCallback)

{
  s32 result;

  if (HuMCSelWinCheck()) {
    /* Pad fallback retains the existing callback when the supplied callback is NULL. */
    if (responseCallback != 0) {
      MCContextCallback = responseCallback;
    }
    MCResponseNo = MCResponseLastNo = 0;
    HuMCSelModeSet(1);
    return 0;
  }
  if (HuMCMicSaveGet() != 1) {
    return 0;
  }
  if ((MCStat != 0) || (MicOpenF != 0) || (contextId < 0) || (MC_gsapiEngine == 0)) {
    return 0;
  }
  if ((result = ActivateContext(contextId)) != 0) {
    OSReport(lbl_8023AB60,result);
    return 0;
  }
  ContextCur = contextId;
  MCContextP = &MCContext[contextId];
  MCResponseLastNo = MCResponseNo = 0;
  gsapi_EngineSetMode(MC_gsapiEngine,2);
  gsapi_EngineSetParam(MC_gsapiEngine,1,1);
  gsapi_EngineSetParam(MC_gsapiEngine,0,1);
  OSSendMessage(&MCMessageQueue,0,1);
  if (responseCallback != 0) {
    MCContextCallback = responseCallback;
  }
  else {
    MCContextCallback = 0;
  }
  MCStat = 2;
  return 1;
}

/* Called by a continuous-recognition mode to stop listening and fetch a response. */

int HuMCResponseGet(void)

{
  s16 response;

  if (HuMCSelWinCheck()) {
    HuMCSelModeSet(2);
    if (((MCResponse_s *)MCResponseBuf)[MCResponseLastNo].count) {
      goto response_valid;
    }
    return -1;
response_valid:
    return ((MCResponse_s *)MCResponseBuf)[MCResponseLastNo].word;
  }
  if (HuMCMicSaveGet() != 1) {
    return -1;
  }
  if ((MCStat != 2) || (MicOpenF != 0)) {
    return -1;
  }
  MCContextCallback = 0;
  OSSendMessage(&MCMessageQueue,(OSMessage)2,1);
  OSSleepThread(&MCThreadQueue);
  response = ((MCResponse_s *)MCResponseBuf)[MCResponseLastNo].status;
  MCStat = 0;
  if (response != 0) {
    return response;
  }
  if (MCResponseNo == MCResponseLastNo) {
    return -1;
  }
  /* Unlike one-shot retrieval, return the stored word without checking its recognized-word
   * count. */
  return ((MCResponse_s *)MCResponseBuf)[MCResponseLastNo].word;
}

/* Returns the next queued record; when none is ready, resets its summary fields
   but leaves previous candidate entries intact. */

u16 *HuMCCurResponseGet(void)
{
    if (MCResponseNo > MCResponseLastNo) {
        *(MCResponseData_s *)MCCurResponse =
            *(MCResponseData_s *)(MCResponseBuf + (MCResponseLastNo * MIC_RESPONSE_RECORD_SIZE));
        MCResponseLastNo++;
        if (MCResponseNo <= MCResponseLastNo) {
            MCResponseLastNo = MCResponseNo;
        }
    } else {
        *(s16 *)&MCCurResponse[0] = -1;
        *(u16 *)&MCCurResponse[2] = MIC_SCORE_UNAVAILABLE;
        *(s16 *)&MCCurResponse[4] = 0;
        *(s32 *)&MCCurResponse[8] = 0;
        *(s32 *)&MCCurResponse[12] = -1;
        MCResponseLastNo = MCResponseNo;
    }
    return (u16 *)MCCurResponse;
}

/* Returns the cached talk-button mask when microphone mode is selected and response storage
 * exists. */

s32 HuMCButtonGet(void)

{
  if ((HuMCMicSaveGet() != 1) || (MCResponseBuf == 0)) {
    return 0;
  }
  return MCButton;
}

/* Returns cached new talk-button presses when microphone mode is selected and response storage
 * exists. */

s32 HuMCButtonDownGet(void)

{
  if ((HuMCMicSaveGet() != 1) || (MCResponseBuf == 0)) {
    return 0;
  }
  return MCButtonDown;
}

/* Polls the microphone driver for up to half a second during device setup. */

int HuMCProbe(u32 channel)
{
  int result;
  OSTick start;

  start = OSGetTick();
  while (OSTicksToMilliseconds(OSGetTick() - start) < 500) {
    result = MICProbeEx(channel);
    if (result != -1) {
      break;
    }
  }
  return result;
}

static char lbl_8023ABA0[] = "MIC Error: M2SSetActiveChannel() %d\n";
static char lbl_8023ABC5[] = "M2SSetShifts Error\n";

/* Mounts chan for speech setup, accepting an already-mounted result. After device and channel
* checks, logs shift rejection but continues. Gain and channel-1 start results are ignored,
* so their failures do not change the returned success status. */
inline s32 HuMCMount(s32 channel)
{
  s32 result;
  s32 deviceIdResult;
  s32 channelResult;
  s32 probeStatus;
  OSTick probeStartTick;
  s32 probeResult;
  s32 deviceId;

  M2SClose();
  MicOpenF = 1;
  MCWrongDeviceF = 0;
  probeStartTick = OSGetTick();
  while (OSTicksToMilliseconds(OSGetTick() - probeStartTick) < 500) {
    if ((probeResult = MICProbeEx(channel)) != -1) {
      break;
    }
  }
  probeStatus = probeResult;
  if ((result = probeStatus) != 0) {
    return result;
  }
  if ((result = MICMount(channel, MicBuffer, MIC_CAPTURE_BUFFER_SIZE, MCExtHandler)) != 0) {
    if (result != -4) {
      return result;
    }
    result = 0;
  }
  deviceIdResult = MICGetDeviceID(channel, &deviceId);
  if (deviceIdResult != 0) {
    return deviceIdResult;
  }
  if (deviceId != 0) {
    MCWrongDeviceF = 1;
    return -2;
  }
  channelResult = M2SSetActiveChannel(channel);
  if (channelResult == 0) {
    OSReport(lbl_8023ABA0, channelResult);
    return -128;
  }
  MICSetGain(channel, 0);
  if (M2SSetShifts(M2SShift) == 0) {
    OSReport(lbl_8023ABC5);
  }
  MicOpenF = 0;
  MICStart(1);
  M2SOpen();
  return result;
}

/* Polls a channel for up to half a second while a lost microphone is retried. */
static inline s32 MCProbeSub(s32 channel)
{
  s32 probeStatus;
  OSTick probeStartTick;

  probeStartTick = OSGetTick();
  while (OSTicksToMilliseconds(OSGetTick() - probeStartTick) < 500) {
    if ((probeStatus = MICProbeEx(channel)) != -1) {
      break;
    }
  }
  return probeStatus;
}

/* Attempts microphone setup after a reconnect, accepting an already-mounted result. The answer
 * process and warning retry this helper. After device and channel checks, a rejected shift is
 * logged but setup continues; gain and channel-1 start results are ignored. */
static inline s32 MCMountSub(s32 channel)
{
  s32 result;
  s32 deviceIdResult;
  s32 deviceId;
  s32 channelResult;

  M2SClose();
  MicOpenF = 1;
  MCWrongDeviceF = 0;
  if ((result = MCProbeSub(channel)) != 0) {
    return result;
  }
  if ((result = MICMount(channel, MicBuffer, MIC_CAPTURE_BUFFER_SIZE, MCExtHandler)) != 0) {
    if (result != -4) {
      return result;
    }
    result = 0;
  }
  deviceIdResult = MICGetDeviceID(channel, &deviceId);
  if (deviceIdResult != 0) {
    return deviceIdResult;
  }
  if (deviceId != 0) {
    MCWrongDeviceF = 1;
    return -2;
  }
  if ((channelResult = M2SSetActiveChannel(channel)) == 0) {
    OSReport(lbl_8023ABA0, channelResult);
    return -128;
  }
  MICSetGain(channel, 0);
  if (M2SSetShifts(M2SShift) == 0) {
    OSReport(lbl_8023ABC5);
  }
  MicOpenF = 0;
  MICStart(1);
  M2SOpen();
  return result;
}

/* The microphone driver's detach callback marks capture unavailable and stops the speech stream. */
void MCExtHandler(void)

{
  MicOpenF = 1;
  M2SStop();
  OSReport(lbl_802BF99B);
  return;
}

/* Saves the requested PCM bit shift and attempts to apply it immediately;
   rejection is ignored and the saved value is retried during mounting. */
void HuMCShiftsSet(u32 shifts)

{
  M2SShift = shifts;
  M2SSetShifts(shifts);
  return;
}

/* Creates an initially hidden microphone-status sprite at x and y;
   the answer process shows it while recognition is running. */
void HuMCMicSprCreate(f32 x,f32 y)

{
  void *spriteFile;
  ANIMDATA *spriteAnimation;
  HUSPRID statusSprite;

  if (HuMCMicSaveGet() == 1) {
    spriteFile = HuAR_ARAMtoMRAMFileRead(MIC_STATUS_SPRITE_DATA_NUM,MIC_ALLOC_TAG,2);
    spriteAnimation = HuSprAnimRead(spriteFile);
    MCSprGrpId = HuSprGrpCreate(1);
    statusSprite = HuSprCreate(spriteAnimation,1,0);
    HuSprGrpMemberSet(MCSprGrpId,0,statusSprite);
    HuSprGrpPosSet(MCSprGrpId,lbl_802C1E48,lbl_802C1E48);
    HuSprPosSet(MCSprGrpId,0,x,y);
    HuSprAttrSet(MCSprGrpId,0,HUSPR_ATTR_DISPOFF);
  }
  return;
}

/* Microphone shutdown removes the status sprite group and clears its stored group ID. */
void HuMCMicSprKill(void)

{
  if (MCSprGrpId != -1) {
    HuSprGrpKill(MCSprGrpId);
    MCSprGrpId = -1;
  }
  return;
}

/* Returns a pending microphone choice when one exists, otherwise the saved game microphone mode. */
s32 HuMCMicGet(void)

{
  if (MCMicValue != -1) {
    return MCMicValue;
  }
  return GwCommon.mic;
}

/* Returns the saved microphone mode used by capture and pad-answer processes. */
inline s32 HuMCMicSaveGet(void)

{
  return GwCommon.mic;
}

/* Updates the saved microphone mode when speech storage is absent;
   otherwise defers the choice until microphone shutdown. */
void HuMCMicSet(s32 micChoice)

{
  if (MCResponseBuf != 0) {
    MCMicValue = micChoice;
  }
  else {
    GwCommon.mic = (u8)micChoice;
  }
}

/* Game startup clears the pad-answer window ID and its update-process pointer. */
void InitMCSelWin(void)

{
  MCSelWinWork.winId = MIC_ID_NONE_VALUE;
  MCSelWinWork.proc = 0;
  return;
}

/* In pad-answer mode, creates choice-window storage and its update process,
   saving x and y for later option layout. */
void HuMCSelWinCreate(f32 x,f32 y)

{
  s32 micMode;
  MCSelWinWork.winId = MIC_ID_NONE_VALUE;
  micMode = GwCommon.mic;
  if (micMode == 2) {
    MCSelWinWork.winId = HuWinCreate((double)lbl_802C1E4C,(double)lbl_802C1E4C,32,32,0);
    MCSelWinWork.x = x;
    MCSelWinWork.y = y;
    MCSelWinWork.item = (u8 *)HuMemDirectMallocNum(0,256,MIC_ALLOC_TAG);
    *MCSelWinWork.item = 0;
    MCSelWinWork.order = HuMemDirectMallocNum(0,64,MIC_ALLOC_TAG);
    MCSelWinWork.choice = 0;
    MCSelWinWork.proc = HuPrcCreate(MCSelWinFunc,65000,MIC_CHOICE_PROCESS_STACK_SIZE,0);
    HuPrcSetStat(MCSelWinWork.proc,HU_PRC_STAT_PAUSE_ON | HU_PRC_STAT_UPAUSE_ON);
  }
  return;
}

/* Shutdown removes the pad-answer window and its process,
   then frees the encoded choices and option-order buffers. */
void HuMCSelWinKill(void)

{
  if (MCSelWinWork.winId != -1) {
    HuWinKill(MCSelWinWork.winId);
  }
  MCSelWinWork.winId = MIC_ID_NONE_VALUE;
  if (MCSelWinWork.proc != 0) {
    HuPrcKill(MCSelWinWork.proc);
  }
  MCSelWinWork.proc = 0;
  if (MCSelWinWork.item != 0) {
    HuMemDirectFree(MCSelWinWork.item);
  }
  MCSelWinWork.item = 0;
  if (MCSelWinWork.order != 0) {
    HuMemDirectFree(MCSelWinWork.order);
  }
  MCSelWinWork.order = 0;
  return;
}

static char lbl_8023ABD9[] = "Error: HuMCSelWinItemRandSet() %d<%d\n";

/* In pad-answer mode, prepares a hidden window for padNo from shuffled message choices, sorted
* by message index. It swaps fixedItem to the first slot before shuffling the remaining slots, so
* fixedItem remains in the shown subset whenever shownCount is positive. */
s32 HuMCSelWinItemRandSet(s32 messageBase,s16 itemCount,s16 fixedItem,
                          s16 shownCount,s16 padNo)
{
  s32 swappedItem;
  s16 randomItemIndex;
  s16 orderingPass;
  u8 *choiceCursor;
  s16 itemIndex;
  HuVec2f windowSize;
  f32 windowX;
  f32 windowY;

  choiceCursor = MCSelWinWork.item;
  if (HuMCMicSaveGet() != 2) {
    return -1;
  }
  if (MCSelWinWork.winId != -1) {
    HuWinKill(MCSelWinWork.winId);
  }
  if (itemCount < shownCount) {
    OSReport(lbl_8023ABD9,itemCount,shownCount);
    return -1;
  }

  for (itemIndex = 0; itemIndex < itemCount; itemIndex++) {
    MCSelWinWork.order[itemIndex] = (u8)itemIndex;
  }
  swappedItem = MCSelWinWork.order[0];
  MCSelWinWork.order[0] = MCSelWinWork.order[fixedItem];
  MCSelWinWork.order[fixedItem] = swappedItem;
  for (orderingPass = 0; orderingPass < 10; orderingPass++) {
    for (itemIndex = 1; itemIndex < itemCount; itemIndex++) {
      randomItemIndex = frandmod(itemCount - 1) + 1;
      swappedItem = MCSelWinWork.order[itemIndex];
      MCSelWinWork.order[itemIndex] = MCSelWinWork.order[randomItemIndex];
      MCSelWinWork.order[randomItemIndex] = swappedItem;
    }
  }
  for (orderingPass = 0; orderingPass < shownCount - 1; orderingPass++) {
    for (itemIndex = 0; itemIndex < shownCount - orderingPass - 1; itemIndex++) {
      if (MCSelWinWork.order[itemIndex] > MCSelWinWork.order[itemIndex + 1]) {
        swappedItem = MCSelWinWork.order[itemIndex];
        MCSelWinWork.order[itemIndex] = MCSelWinWork.order[itemIndex + 1];
        MCSelWinWork.order[itemIndex + 1] = swappedItem;
      }
    }
  }

  /* Start a new page and encode a space, choice marker and message insertion for each option. */
  *choiceCursor++ = MIC_MES_HOME;
  for (itemIndex = 0; itemIndex < shownCount; itemIndex++) {
    *choiceCursor++ = MIC_MES_SPACE;
    *choiceCursor++ = MIC_MES_CHOICE;
    *choiceCursor++ = MIC_MES_INSERT;
    *choiceCursor++ = (u8)(itemIndex + 1);
    if (itemIndex + 1 < shownCount) {
      *choiceCursor++ = 10;
    }
    HuWinInsertMesSizeGet(messageBase + MCSelWinWork.order[itemIndex],(s16)itemIndex);
  }
  *choiceCursor++ = 0;
  HuWinMesMaxSizeGet(1,&windowSize,MCSelWinWork.item);

  if (lbl_802C1E50 == MCSelWinWork.x) {
    windowX = lbl_802C1E58;
  }
  else if (lbl_802C1E60 == MCSelWinWork.x) {
    windowX = lbl_802C1E68 - lbl_802C1E6C * windowSize.x;
  }
  else {
    windowX = MCSelWinWork.x;
  }
  if (lbl_802C1E70 == MCSelWinWork.y) {
    windowY = lbl_802C1E78;
  }
  else if (lbl_802C1E80 == MCSelWinWork.y) {
    windowY = lbl_802C1E88 - lbl_802C1E6C * windowSize.y;
  }
  else {
    windowY = MCSelWinWork.y;
  }

  MCSelWinWork.winId = HuWinCreate(windowX,windowY,(s16)windowSize.x,(s16)windowSize.y,0);
  HuWinScaleSet(MCSelWinWork.winId,lbl_802C1E6C,lbl_802C1E6C);
  for (itemIndex = 0; itemIndex < itemCount; itemIndex++) {
      HuWinInsertMesSet(MCSelWinWork.winId, messageBase + MCSelWinWork.order[itemIndex],
                        (s16) itemIndex);
  }
  HuWinAttrSet(MCSelWinWork.winId,HUWIN_ATTR_NOCANCEL);
  HuWinAttrSet(MCSelWinWork.winId,HUWIN_ATTR_UPAUSE);
  winData[MCSelWinWork.winId].padMask = 1 << padNo;
  HuWinDispOff(MCSelWinWork.winId);
  if (MCSelWinWork.proc != 0) {
    HuPrcKill(MCSelWinWork.proc);
  }
  MCSelWinWork.proc = HuPrcCreate(MCSelWinFunc,65000,MIC_CHOICE_PROCESS_STACK_SIZE,0);
  HuPrcSetStat(MCSelWinWork.proc,HU_PRC_STAT_PAUSE_ON | HU_PRC_STAT_UPAUSE_ON);
  return MCSelWinWork.winId;
}

/* Prepares all message choices for the given pad; the window stays hidden until recognition is
 * requested. */
void HuMCSelWinItemSet(s32 messageBase,s16 itemCount,s16 padNo)

{
  HuMCSelWinItemRandSet(messageBase,itemCount,0,itemCount,padNo);
  return;
}

/* Tests whether a pad-answer window ID exists; it does not test whether the window is visible. */
inline BOOL HuMCSelWinCheck(void)

{
  if (MCSelWinWork.winId == -1) {
    return FALSE;
  }
  return TRUE;
}

/* Returns the pad-answer window's requested state, or -1 when no window exists. */
int HuMCSelModeGet(void)

{
  if (HuMCSelWinCheck() == FALSE) {
    return -1;
  }
  return MCSelWinWork.choice;
}

/* Requests a pad-answer window state; hiding an active choice also cancels its selection. */
void HuMCSelModeSet(short requestedState)

{
  HUWIN *choiceWindow;

  if (MCSelWinWork.winId != -1) {
    if (requestedState == 2) {
      choiceWindow = &winData[MCSelWinWork.winId];
      if (choiceWindow->stat == 3) {
        choiceWindow->stat = 0;
        choiceWindow->choice = -1;
      }
    }
    MCSelWinWork.choice = requestedState;
  }
}

/* Runs for a pad-answer window: shows requested choices, queues the selected answer or
 * cancellation,
 * invokes the response callback, and hides the window. Pauses temporarily hide its
 * sprites. */
static void MCSelWinFunc(void)
{
  HUWIN *choiceWindow;
  MCResponseCallback responseCallback;
  s16 selectedChoice;
  s16 selectionDelay;
  s16 responseWord;

  for (;;) {
    switch (MCSelWinWork.choice) {
      case 1:
        if (*MCSelWinWork.item == 0) {
          MCSelWinWork.choice = 0;
          break;
        }
        HuWinDispOn(MCSelWinWork.winId);
        HuWinMesSet(MCSelWinWork.winId,(u32)MCSelWinWork.item);
        choiceWindow = &winData[MCSelWinWork.winId];
        while (choiceWindow->stat != 0) {
          if (omUPauseFlag + omPauseChk() != 0) {
            HuWinDispOff(MCSelWinWork.winId);
          } else {
            HuWinDispOn(MCSelWinWork.winId);
          }
          HuPrcVSleep();
        }
        HuWinChoiceSet(MCSelWinWork.winId,0);
        while (choiceWindow->stat != 0) {
          if (omUPauseFlag + omPauseChk() != 0) {
            HuWinDispOff(MCSelWinWork.winId);
          } else {
            HuWinDispOn(MCSelWinWork.winId);
          }
          HuPrcVSleep();
        }
        selectedChoice = choiceWindow->choice;
        if (selectedChoice >= 0) {
          responseWord = MCSelWinWork.order[selectedChoice];
          for (selectionDelay = 0; selectionDelay < 10; selectionDelay++) {
            if (omUPauseFlag + omPauseChk() != 0) {
              HuWinDispOff(MCSelWinWork.winId);
            } else {
              HuWinDispOn(MCSelWinWork.winId);
            }
            HuPrcVSleep();
          }
          /* The queued result points to this process's local responseWord; the summary word is
           * one-based. */
          MicWriteResponse(0,1,&responseWord,10000,responseWord + 1);
        } else if (selectedChoice == -1) {
          MicWriteResponse(-2,0,NULL,0,-1);
        }
        if (MCContextCallback) {
          responseCallback = MCContextCallback;
          responseCallback(HuMCCurResponseGet());
        }
        MCSelWinWork.choice = 2;
        break;

      case 2:
        HuWinDispOff(MCSelWinWork.winId);
        MCSelWinWork.choice = 0;
        break;
    }
    HuPrcVSleep();
  }
}

/* Starts a one-shot button-driven answer process when microphone support is enabled, replacing any
 * existing listener. Starting it clears the previously configured timeout. */
void HuMCSelWinContextSet(s16 contextId, MCResponseCallback responseCallback,
                          u8 padNo)

{
  if (HuMCMicSaveGet() == 0) {
    return;
  }
  if (MCListenerProc != 0) {
    HuMCSelWinContextKill();
  }
  MCCallback = responseCallback;
  MCListenerProc = HuPrcCreate(MCSelWinContextProc,65000,MIC_CHOICE_PROCESS_STACK_SIZE,0);
  MCListenerProc->property = (void *)(((u32)padNo << 16) | (u16)contextId);
  MCSelWinMaxTime = 0;
  return;
}

/* Cancels the one-shot answer process by queuing a no-result response, hiding pad choices or
 * stopping active one-shot speech recognition, then removing its listener process. */
void HuMCSelWinContextKill(void)
{
  MicWriteResponse(-1,0,NULL,0,-1);
  HuMCResponseGet2();
  if (MCListenerProc) {
    HuPrcKill(MCListenerProc);
  }
  MCListenerProc = 0;
}

/* Sets the choice timeout in frames using seconds and the supplied frame rate. */
static inline void MCSetTimeout(f32 seconds, const f32 *frameRate)
{
  MCSelWinMaxTime = *frameRate * seconds;
}

/* Configures the one-shot answer timeout in seconds, converted to an integer count at 60 frames
 * per second. Starting a new answer process clears this timeout. */
void HuMCSelWinMaxTimeSet(f32 seconds)

{
  MCSetTimeout(seconds, &lbl_802C1E8C);
  return;
}

/* Returns the remaining answer timeout in frames, represented as a float. */
f32 HuMCSelWinMaxTimeGet(void)

{
  return (f32)MCSelWinMaxTime;
}

/* Discards the previous queue positions, waits for a response, then returns the latest successful
 * response's first word or -1 if the newly queued records contain no success. */
int HuMCSelWinChoiceGet(void)

{
  MCResponse_s *response;
  s16 zero;
  s16 index;

  MCResponseNo = MCResponseLastNo = zero = 0;
  for (;;) {
    if (zero != MCResponseNo) {
      break;
    }
    HuPrcVSleep();
  }
  index = MCResponseNo - 1;
  while (index >= 0) {
    response = (MCResponse_s *)(MCResponseBuf + index * MIC_RESPONSE_RECORD_SIZE);
    if (response->status == 0) {
      break;
    }
    index--;
  }
  if (index == -1) {
    return -1;
  }
  return *response->result;
}

/* Waits for the microphone talk button or the selected pad's R button, runs one-shot recognition
 *
 * or pad choices, and ends after a response or timeout. Speech retries recover stopped listening.
 */
static void MCSelWinContextProc(void)
{
  HUPROCESS *listenerProcess;
  u32 packedContext;
  u8 padNo;
  s32 contextId;
  s32 initialButton;
  s16 queuedResponseCount;
  s16 releaseWaitFrames;
  s16 *latestResponse;

  listenerProcess = HuPrcCurrentGet();
  packedContext = (u32)listenerProcess->property;
  padNo = packedContext >> 16;
  contextId = (u16)packedContext;
  initialButton = HuMCButtonGet();
  for (;;) {
    if (((HuMCMicSaveGet() == 1) && (HuMCButtonDownGet() || initialButton)) ||
        (HuMCSelWinCheck() && (HuPadBtnDown[padNo] & PAD_TRIGGER_R))) {
      initialButton = 0;
      HuMCContextSet(contextId);
      MCContextCallback = MCCallback;
      if (HuMCMicSaveGet() == 1) {
        queuedResponseCount = MCResponseNo;
        while (HuMCButtonGet()) {
          if (queuedResponseCount != MCResponseNo) {
            latestResponse = (s16 *)(MCResponseBuf + (MCResponseNo - 1) * MIC_RESPONSE_RECORD_SIZE);
            if ((*latestResponse == 0) || (*latestResponse == -2) || (*latestResponse == -4)) {
              break;
            }
            queuedResponseCount = MCResponseNo;
            HuMCResponseGet2();
            HuMCContextSet(contextId);
            MCContextCallback = MCCallback;
          }
          if (MCSelWinMaxTime != 0) {
            MCSelWinMaxTime--;
            if (MCSelWinMaxTime == 0) {
              MicWriteResponse(-3,0,NULL,0,-1);
              break;
            }
          }
          if (MCListenF != 0) {
            OSReport(lbl_8023ABFF);
            OSSendMessage(&MCMessageQueue,(OSMessage)2,1);
            OSSleepThread(&MCThreadQueue);
            OSSendMessage(&MCMessageQueue,0,1);
            MCListenF = 0;
          }
          HuPrcVSleep();
        }
        if (queuedResponseCount == MCResponseNo) {
          releaseWaitFrames = 0;
          while (releaseWaitFrames < 60) {
            if (queuedResponseCount != MCResponseNo) {
              break;
            }
            HuPrcVSleep();
            releaseWaitFrames++;
          }
          if (releaseWaitFrames == 60) {
            MicWriteResponse(-1,0,NULL,0,-1);
          }
        }
        HuMCResponseGet2();
        break;
      }
      else {
        while (MCResponseNo == 0) {
          if (MCSelWinMaxTime != 0) {
            MCSelWinMaxTime--;
            if (MCSelWinMaxTime == 0) {
              MicWriteResponse(-3,0,NULL,0,-1);
              break;
            }
          }
          HuPrcVSleep();
        }
        HuMCResponseGet2();
        break;
      }
    }
    if (MCSelWinMaxTime != 0) {
      MCSelWinMaxTime--;
      if (MCSelWinMaxTime == 0) {
        MicWriteResponse(-3,0,NULL,0,-1);
        break;
      }
    }
    HuPrcVSleep();
  }
  MCListenerProc = NULL;
  HuPrcEnd();
  for (;;) {
    HuPrcVSleep();
  }
}

/* When microphone support is enabled, replaces the current listener process and stores its context,
 * response callback, and pad number for continuous button-driven recognition. */
void HuMCListenerCreate(s16 contextId, MCResponseCallback responseCallback,
                        u8 padNo)

{
  s32 micMode;

  micMode = GwCommon.mic;
  if (micMode != 0) {
    MCCallback = responseCallback;
    if (MCListenerProc != 0) {
      HuPrcKill(MCListenerProc);
    }
    MCListenerProc = HuPrcCreate(MCListenerFunc,65000,MIC_CHOICE_PROCESS_STACK_SIZE,0);
    MCListenerProc->property = (void *)(((u32)padNo << 16) | (u16)contextId);
  }
  return;
}

/* Queues a cancellation response, hides or removes pad choices, stops active continuous speech
 * recognition, and removes the listener process. */
void HuMCListenerKill(void)

{
  MicWriteResponse(-1,0,0,0,-1);
  HuMCResponseGet();
  if (HuMCSelWinCheck()) {
    HuMCSelWinKill();
  }
  if (MCListenerProc != 0) {
    HuPrcKill(MCListenerProc);
  }
  MCListenerProc = 0;
  return;
}

/* Repeatedly listens while the microphone talk button is held. If no response is queued on
* release, waits up to 60 frames for one and then sleeps one additional frame before stopping.
* A pad R press starts choices but requests hiding on the following frame. Pending valid
* responses are forwarded to the callback. */
static void MCListenerFunc(void)
{
  HUPROCESS *listenerProcess;
  u32 packedContext;
  u8 padNo;
  u16 contextId;
  s16 queuedResponseCount;
  s16 releaseWaitFrames;

  listenerProcess = HuPrcCurrentGet();
  packedContext = (u32)listenerProcess->property;
  padNo = packedContext >> 16;
  contextId = packedContext;
  queuedResponseCount = 100;
  for (;;) {
    if (((HuMCMicSaveGet() == 1) && HuMCButtonGet()) ||
        (HuMCSelWinCheck() && (HuPadBtnDown[padNo] & PAD_TRIGGER_R))) {
      MCResponseCallback contextCallback;

      contextCallback = MCCallback;
      HuMCContextCallbackSet(contextId,contextCallback);
      queuedResponseCount = MCResponseNo;
      if (HuMCMicSaveGet() == 1) {
        while (HuMCButtonGet()) {
          HuPrcVSleep();
        }
        if (queuedResponseCount == MCResponseNo) {
          for (releaseWaitFrames = 0; releaseWaitFrames < 60; releaseWaitFrames++) {
            if (queuedResponseCount != MCResponseNo) {
              break;
            }
            HuPrcVSleep();
          }
          HuPrcVSleep();
        }
      } else {
        HuPrcVSleep();
      }
      HuMCResponseGet();
    }
    if ((ValidResultF != 0) && MCCallback) {
      MCResponseCallback resultCallback;

      resultCallback = MCCallback;
      (*resultCallback)(HuMCCurResponseGet());
      ValidResultF = 0;
    }
    HuPrcVSleep();
  }
}

/* Runs while speech support is open: retries a lost microphone, reads talk-button transitions,
 * dispatches queued responses, and shows the status sprite only while recognition is running. */
static void MCAnswerMain(void)
{
  /* previousButton and previousResponseCount have no initial values before their first
   * comparisons. */
  u32 previousButton;
  s16 previousResponseCount;
  MCResponseCallback responseCallback;

  ValidResultF = 0;
  for (;;) {
    do {
      HuPrcVSleep();
    } while (HuMCSelWinCheck() || (HuMCMicSaveGet() != 1));

    if ((MicOpenF != 0) && (HuMCMicSaveGet() == 1)) {
      if ((omcurovl != DLL_mdseldll) && (omcurovl != DLL_s01dll) &&
          (omcurovl != DLL_s02dll) && (omcurovl != DLL_s03dll)) {
        s32 mountResult;

        mountResult = -3;
        while ((WipeCheckIn() != 0) || (Hu3DPauseF != 0)) {
          if ((mountResult = MCMountSub(1)) == 0) {
            break;
          }
          HuPrcVSleep();
        }
        if (mountResult != 0) {
          MCDeviceMesExec();
        }
        OSReport(s_Mount_OK__8023ac18);
      } else {
        while (MCMountSub(1) != 0) {
          HuPrcVSleep();
        }
      }
    }

    /* Ignore the button-read result; a failed read leaves the previous cached mask in use. */
    MICGetButton(1,&MCButton);
    MCButton &= 0x10;
    MCButtonDown = MCButton & (previousButton ^ MCButton);
    previousButton = MCButton;
    if (MCContextCallback && (previousResponseCount != MCResponseNo)) {
      if (previousResponseCount < MCResponseNo) {
        responseCallback = MCContextCallback;
        (*responseCallback)(HuMCCurResponseGet());
        if (ValidResultF != 0) {
          ValidResultF = 0;
        }
      }
      previousResponseCount = MCResponseNo;
    }
    if (MCSprStat == 4) {
      OSReport(s_MIC_Error__Engine__x___8023ac23,MC_gsapiEngineError);
    }
    if (MCSprGrpId != -1) {
      if (MCSprStat == 1) {
        HuSprAttrReset(MCSprGrpId,0,HUSPR_ATTR_DISPOFF);
      } else {
        HuSprAttrSet(MCSprGrpId,0,HUSPR_ATTR_DISPOFF);
      }
    }
  }
}

/* Selects the session ID used by later context activation in microphone mode; IDs below four,
 * including -1 to skip session switching and -2 to restore the baseline session, are accepted. */
void HuMCSessionSet(short playerIndex)

{
  s32 micMode;

  micMode = GwCommon.mic;
  if ((micMode == 1) && (playerIndex < 4)) {
    MCSessionCur = playerIndex;
  }
  return;
}

/* Releases every saved player session, clears current and previous session IDs, and frees the
 * allocations tagged for player-session storage. */
void HuMCSessionClose(void)

{
    short playerIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (PlayerSession[playerIndex] != 0) {
            gsapi_EngineSessionDataFree(PlayerSession[playerIndex]);
    }
  }
  MCSessionCur = MCSessionPrev = -1;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        PlayerSession[playerIndex] = 0;
  }
  HuMemDirectFreeNum(0,MIC_SESSION_ALLOC_TAG);
  return;
}

/* In microphone mode, restores the baseline engine session before freeing this player's saved
 * session; both engine results are ignored. */
void HuMCSessionKill(short playerIndex)

{
  s32 micMode;

  micMode = GwCommon.mic;
  if ((micMode != 1) || (MC_gsapiEngine == 0)) {
    return;
  }
  if (PlayerSession[playerIndex] != 0) {
    gsapi_EngineSessionDataImport(MC_gsapiEngine,MCSessionP,0);
    gsapi_EngineSessionDataFree(PlayerSession[playerIndex]);
    PlayerSession[playerIndex] = 0;
  }
  return;
}

/* Searches backward for the latest successful response. Saves it for confirmation when
* at least two distinct first words occur among its first three candidate slots with nonzero
* scores and the first distinct word's signed score minus the second's is at most 1000.
* The score comparison does not use an absolute difference. */
s32 HuMCUnkResponseCheck(void)
{
  MCResponse_s *latestResponse;
  s32 scanIndex;
  s16 candidateWords[3];
  s16 candidateScores[3];
  s16 distinctWordCount;
  s32 knownWordIndex;

  ((MCResponse_s *)MCUnkResponseData)->entry[0].score =
      ((MCResponse_s *)MCUnkResponseData)->entry[1].score =
      ((MCResponse_s *)MCUnkResponseData)->entry[2].score = 0;
  for (scanIndex = MCResponseNo - 1; scanIndex >= 0; scanIndex--) {
    latestResponse = (MCResponse_s *)(MCResponseBuf + scanIndex * MIC_RESPONSE_RECORD_SIZE);
    if (latestResponse->status == 0) {
      break;
    }
  }
  if (scanIndex == -1) {
    return 0;
  }
  if (latestResponse->entry[0].score == 0) {
    return 0;
  }

  scanIndex = distinctWordCount = 0;
  for (; scanIndex < 3; scanIndex++) {
    if (latestResponse->entry[scanIndex].score != 0) {
      for (knownWordIndex = 0; knownWordIndex < distinctWordCount; knownWordIndex++) {
        if (*latestResponse->entry[scanIndex].result == candidateWords[knownWordIndex]) {
          break;
        }
      }
      if (knownWordIndex == distinctWordCount) {
        candidateWords[distinctWordCount] = *latestResponse->entry[scanIndex].result;
        candidateScores[distinctWordCount] = latestResponse->entry[scanIndex].score;
        distinctWordCount++;
      }
    }
  }
  if (distinctWordCount <= 1) {
    return 0;
  }
  if ((candidateScores[0] - candidateScores[1]) <= 1000) {
    *(MCResponse_s *)MCUnkResponseData = *latestResponse;
    return 1;
  }
  return 0;
}

static char lbl_8023AC3B[] =
    "/mic/ctx/mic_yesno\0"
    "Reset Session Export >>>%x\n\0"
    "Session Export >>>%x:%x\n\0"
    "Session Import >>>%x:%x\n\0"
    "Session Import >>>%x\n";

/* Confirms up to three distinct words from the saved ambiguous response with successive yes/no
 *
 * prompts. If the yes/no context cannot be created, returns the first candidate without
 * confirmation. */
s32 HuMCNewResponseGet(s32 messageBase)
{
  MCResponse_s *savedResponse;
  HuVec2f windowSize;
  s16 candidateWords[3];
  s16 candidateScores[3];
  s16 distinctWordCount;
  s32 candidateIndex;
  s32 knownWordIndex;
  s32 confirmationChoice;
  s32 confirmationWindow;

  savedResponse = (MCResponse_s *)MCUnkResponseData;
  if (savedResponse->entry[0].score == 0) {
    return -1;
  }
  if (MCYesNoCtxId == -1) {
    MCYesNoCtxId = HuMCContextCreate(lbl_8023AC3B);
    if (MCYesNoCtxId == -1) {
      return *savedResponse->entry[0].result;
    }
  }

  candidateIndex = distinctWordCount = 0;
  for (; candidateIndex < 3; candidateIndex++) {
    if (savedResponse->entry[candidateIndex].score != 0) {
      for (knownWordIndex = 0; knownWordIndex < distinctWordCount; knownWordIndex++) {
        if (*savedResponse->entry[candidateIndex].result == candidateWords[knownWordIndex]) {
          break;
        }
      }
      if (knownWordIndex == distinctWordCount) {
        candidateWords[distinctWordCount] = *savedResponse->entry[candidateIndex].result;
        candidateScores[distinctWordCount] = savedResponse->entry[candidateIndex].score;
        distinctWordCount++;
      }
    }
  }
  if (distinctWordCount <= 1) {
    return -1;
  }

  for (candidateIndex = 0; candidateIndex < distinctWordCount; candidateIndex++) {
    HuWinInsertMesSizeGet(messageBase + candidateWords[candidateIndex],0);
    HuWinMesMaxSizeGet(1,&windowSize,MIC_RECOGNIZED_WORD_CONFIRM_MESSAGE);
    confirmationWindow = HuWinExCreateFrame(lbl_802C1E98,lbl_802C1E9C,
                               (s16)windowSize.x,(s16)windowSize.y,-1,0);
    HuWinExOpen(confirmationWindow);
    HuWinMesSpeedSet(confirmationWindow,0);
    HuWinAttrSet(confirmationWindow,HUWIN_ATTR_ALIGN_CENTER);
    HuWinInsertMesSet(confirmationWindow,messageBase + candidateWords[candidateIndex],0);
    HuWinMesSet(confirmationWindow,MIC_RECOGNIZED_WORD_CONFIRM_MESSAGE);
    do {
      s32 yesNoContext = MCYesNoCtxId;
      HuMCSelWinContextSet(yesNoContext,NULL,0);
    } while (((confirmationChoice = HuMCSelWinChoiceGet()) != 0) && (confirmationChoice != 1));
    HuWinExClose(confirmationWindow);
    HuWinKill(confirmationWindow);
    if (confirmationChoice == 0) {
      break;
    }
  }
  if (candidateIndex == distinctWordCount) {
    return -1;
  }
  return candidateWords[candidateIndex];
}

/* During microphone context switching or shutdown, creates a baseline session if needed and
 * saves the previous player's engine session. Imports a saved selected-player session or, for
 * ID -2, the baseline; an unsaved player leaves the engine session unchanged. */
void HuMCSessionExportReset(void)

{
  int apiResult;
  s32 micMode;
  char *reportStrings;

  reportStrings = lbl_8023A988;
  micMode = GwCommon.mic;
  if ((micMode != 1) || (MCSessionCur == -1)) {
    return;
  }
  {
    if ((MCSessionP == 0) &&
       (apiResult = gsapi_EngineSessionDataExport(MC_gsapiEngine,&MCSessionP), apiResult != 0)) {
      OSReport(reportStrings + 710,apiResult);
    }
    if (MCSessionPrev >= 0) {
      gsapi_EngineSessionDataFree(PlayerSession[MCSessionPrev]);
      HeapNum = MIC_SESSION_ALLOC_TAG;
      apiResult = gsapi_EngineSessionDataExport(MC_gsapiEngine,PlayerSession + MCSessionPrev);
      if (apiResult != 0) {
        OSReport(reportStrings + 738,apiResult,PlayerSession[MCSessionPrev]);
      }
      HeapNum = MIC_ALLOC_TAG;
    }
    MCSessionPrev = MCSessionCur;
    if (MCSessionCur == -2) {
      apiResult = gsapi_EngineSessionDataImport(MC_gsapiEngine,MCSessionP,0);
      if (apiResult != 0) {
        OSReport(reportStrings + 763,apiResult,MCSessionP);
      }
    } else if ((PlayerSession[MCSessionCur] != 0) &&
               (apiResult =
                    gsapi_EngineSessionDataImport(MC_gsapiEngine, PlayerSession[MCSessionCur], 0),
                apiResult != 0)) {
        OSReport(reportStrings + 788, apiResult);
    }
    if (MCSessionCur >= 0) {
      *(u8 *)((int)&MCSessionTimer + (int)MCSessionCur) = 0;
    }
  }
  return;
}

/* Pauses gameplay and shows the device warning until an A-button retry mounts the microphone,
 * then resumes continuous recognition if needed. User and object pause are always cleared on exit;
 * process, model-motion, and sprite pause stay set if the game was already paused. */
static void MCDeviceMesExec(void)
{
  char wasPaused;
  u8 pauseWasEnabled;
  s32 padIndex;
  ANIMDATA *alertAnimation;
  void *alertFile;
  HUSPR_GROUPID alertGroup;
  HUSPRID alertSprite;
  HUWINID warningWindow;
  s32 mountStatus;

  pauseWasEnabled = omSysPauseEnableFlag;
  wasPaused = omPauseChk();

  omUPauseFlag = 1;
  omSysPauseEnable(0);
  omObjManPause(1);
  HuPrcAllPause(1);
  Hu3DPauseSet(1);
  HuSprPauseSet(1);
  HuPadRumbleAllStop();

  alertFile = HuAR_ARAMtoMRAMFileRead(MIC_DEVICE_ALERT_SPRITE_DATA_NUM,MIC_ALLOC_TAG,2);
  alertAnimation = HuSprAnimRead(alertFile);
  alertGroup = HuSprGrpCreate(1);
  alertSprite = HuSprCreate(alertAnimation,0,0);
  HuSprGrpMemberSet(alertGroup,0,alertSprite);
  HuSprTPLvlSet(alertGroup,0,lbl_802C1EA0);
  HuSprGrpPosSet(alertGroup,lbl_802C1EA4,lbl_802C1EA8);
  HuSprScaleSet(alertGroup,0,lbl_802C1EAC,lbl_802C1EAC);
  warningWindow = HuWinWarningCreate(lbl_802C1E98,lbl_802C1E98,412,120);
  HuWinPriSet(warningWindow,0);
  HuWinMesSpeedSet(warningWindow,0);
  HuWinWarningOpen(warningWindow);
  MCButton = MCButtonDown = 0;
  if (MCWrongDeviceF != 0) {
    mountStatus = -2;
  }
  else {
    mountStatus = -3;
  }

  while (mountStatus != 0) {
    if (mountStatus == -2) {
      HuWinMesSet(warningWindow,MIC_WRONG_DEVICE_MESSAGE);
    }
    else {
      HuWinMesSet(warningWindow,MIC_DEVICE_UNAVAILABLE_MESSAGE);
    }
    HuWinMesWait(warningWindow);
    for (;;) {
      for (padIndex = 0; padIndex < 4; padIndex++) {
        if (HuPadBtnDown[padIndex] & PAD_BUTTON_A) {
          break;
        }
      }
      if (padIndex != 4) {
        HuPrcVSleep();
        break;
      }
      HuPrcVSleep();
    }
    mountStatus = MCMountSub(1);
  }

  HuWinWarningClose(warningWindow);
  HuWinWarningKill(warningWindow);
  HuSprGrpKill(alertGroup);
  MICStart(1);
  if ((MCStat == 2) && (ContextCur >= 0)) {
    MCStat = 0;
    HuMCContextCallbackSet(ContextCur,MCContextCallback);
  }
  omUPauseFlag = 0;
  omSysPauseEnable(pauseWasEnabled);
  omObjManPause(0);
  if (!wasPaused) {
    HuPrcAllPause(0);
    Hu3DPauseSet(0);
    HuSprPauseSet(0);
  }
}

/* Runs the speech engine's start, audio-processing, and stop states from queued commands.
 * Suspends for pad-answer mode, disabled microphone mode, or engine errors. A stop error suspends
 * before waking stop waiters, leaving them asleep until this thread resumes. */
static void *MCThreadFunc(void *threadArgument)

{
  OSMessage threadCommand;
  s32 micMode;

  for (;;) {
    if (OSReceiveMessage(&MCMessageQueue,&threadCommand,0)) {
      MCSprStat = (s32)threadCommand;
    }
    if (HuMCSelWinCheck() || (micMode = GwCommon.mic, micMode != 1)) {
      OSSuspendThread(&MCThread);
      continue;
    }
    if ((MicOpenF != 0) && (MCSprStat != 2)) {
      if (MCSprStat == 1) {
        MCSprStat = 2;
      }
      else {
        MCSprStat = 3;
      }
    }
    switch (MCSprStat) {
      case 0:
        MC_gsapiEngineError = gsapi_EngineStart(MC_gsapiEngine);
        if (MC_gsapiEngineError < 0) {
          MCSprStat = 4;
          OSSuspendThread(&MCThread);
        }
        else {
          MCSprStat = 1;
        }
        continue;
      case 1:
        /* EngineRestart processes engine audio and pending restarts; its supplied argument is
         * unused. */
        gsapi_EngineRestart(0);
        OSSetThreadPriority(&MCThread,OS_PRIORITY_MAX);
        continue;
      case 2:
        MC_gsapiEngineError = gsapi_EngineStop(MC_gsapiEngine);
        if (MC_gsapiEngineError < 0) {
          MCSprStat = 4;
          OSSuspendThread(&MCThread);
        }
        else {
          MCSprStat = 3;
        }
        OSWakeupThread(&MCThreadQueue);
        continue;
      case 3:
        OSSetThreadPriority(&MCThread,OS_PRIORITY_MAX);
        continue;
    }
  }
}

/* The five-millisecond pad-update alarm restores the speech thread's low priority; its arguments
 * are unused. */
static void MCThreadWakeup(OSAlarm *alarm, OSContext *context)

{
  OSSetThreadPriority(&MCThread,OS_PRIORITY_MAX);
  return;
}

/* Pad updates briefly raise the initialized microphone speech thread's priority and schedule a
 * five-millisecond alarm to lower it again. */
void HuMCPeriodicProc(void)

{
  s32 micMode;

  if ((MCInitF == 0) || (MCResponseBuf == 0) ||
      (micMode = GwCommon.mic, micMode != 1)) {
    return;
  }
  OSCreateAlarm(&MCThreadAlarm);
  OSSetAlarm(&MCThreadAlarm,(OSTime)OSMillisecondsToTicks(5),MCThreadWakeup);
  OSSetThreadPriority(&MCThread,0);
  return;
}

/* Handles engine threshold, abnormal-signal and listening notifications, queuing failure responses
 * and tracking listening state. Repeated failures may discard the current player's saved session;
 * baseline-import and session-free results are ignored. */

static char lbl_8023ACB2[] = "OVERLOAD";
static char lbl_8023ACBB[] = "TOO QUIET";
static char lbl_8023ACC5[] = "NO SIGNAL";
static char lbl_8023ACCF[] = "GARBLED SOUND";
static char lbl_8023ACDD[] =
    "POOR MIC\0"
    "threshold %d not reached\n\0"
    "Abnormal condition #%d %s at sample %d\n\0"
    "SPEECH DETECTED\n\0"
    "LISTENING\n\0"
    "STOP LISTEN\n\0"
    "SILENCE DETECTED\n\0";
static char * const MCErrorTbl[] = {
    lbl_802BF9A2, lbl_8023ACB2, lbl_8023ACBB,
    lbl_8023ACC5, lbl_8023ACCF, lbl_8023ACDD
};

static void MicNotifyCallBack(u32 callbackContext, s32 event,
                              u32 sample, u32 value)

{
  short thresholdSession;
  short abnormalSession;
  s32 thresholdMicMode;
  s32 abnormalMicMode;
  char *reportStrings;

  reportStrings = lbl_8023A988;
  switch(event) {
  case 0:
    if (((value < 4000) && (MCSessionCur >= 0)) &&
       (MCSessionTimer[MCSessionCur]++ > 2)) {
      thresholdSession = MCSessionCur;
      thresholdMicMode = GwCommon.mic;
      if (((thresholdMicMode == 1) && (MC_gsapiEngine != 0)) &&
         (PlayerSession[thresholdSession] != 0)) {
        gsapi_EngineSessionDataImport(MC_gsapiEngine,MCSessionP,0);
        gsapi_EngineSessionDataFree(PlayerSession[thresholdSession]);
        PlayerSession[thresholdSession] = 0;
      }
      *(u8 *)((int)&MCSessionTimer + (int)MCSessionCur) = 0;
    }
  case 4:
    if (event == 0) {
      OSReport(reportStrings + 862,value);
      MicWriteResponse(MIC_THRESHOLD_MISSED_RESPONSE,0,0,0,MIC_RESPONSE_NO_RESULT);
    }
    else {
      if ((MCSessionCur >= 0) && (MCSessionTimer[MCSessionCur]++ > 2)) {
        abnormalSession = MCSessionCur;
        abnormalMicMode = GwCommon.mic;
        if ((abnormalMicMode == 1) &&
           ((MC_gsapiEngine != 0 && (PlayerSession[abnormalSession] != 0)))) {
          gsapi_EngineSessionDataImport(MC_gsapiEngine,MCSessionP,0);
          gsapi_EngineSessionDataFree(PlayerSession[abnormalSession]);
          PlayerSession[abnormalSession] = 0;
        }
        *(u8 *)((int)&MCSessionTimer + (int)MCSessionCur) = 0;
      }
      if (value == 1) {
        MicWriteResponse(MIC_ENGINE_CONDITION_RESPONSE,0,0,0,MIC_RESPONSE_NO_RESULT);
      }
      OSReport(reportStrings + 888,value,
               MCErrorTbl[value],sample);
    }
    break;
  case 7:
    OSReport(reportStrings + 928);
    break;
  case 2:
    OSReport(reportStrings + 945);
    MCListenF = 0;
    break;
  case 3:
    OSReport(reportStrings + 956);
    MCListenF = 1;
    break;
  case 8:
    OSReport(reportStrings + 969);
  }
}

/* Called by the speech engine when a recognition result is ready. */

void MicResultCallBack(u32 callbackContext, u32 callbackData, u32 resultNode)

{
  MicResultExec(callbackContext,callbackData,(MicResultNode_s *)resultNode);
  return;
}

/* Converts the engine's candidate list into the response queue for game modes. */

static char lbl_8023AD88[] =
    "%d:%d(%d),%s\n\0"
    "NBEST Error %x\n";

static void MicResultExec(u32 callbackContext, u32 callbackData,
                          MicResultNode_s *firstCandidate)

{
  MicResultNode_s *candidate;
  s16 *candidateWords;
  s16 candidateIndex;
  s16 wordIndex;
  char *wordName;
  MicResponseEntry_s candidates[10];

  candidate = firstCandidate;
  if (firstCandidate->count != 0) {
    for (candidate = firstCandidate, candidateIndex = 0; candidateIndex < 10; candidateIndex++) {
      if (!candidate || (candidate->count == 0)) {
        /* Missing candidates clear score and count only; their result pointer remains unset. */
        candidates[candidateIndex].score = 0;
        candidates[candidateIndex].count = 0;
        }
        else {
        if (((candidate->count + (-2)) + MCResultNum) >= MIC_RESULT_WORD_CAPACITY) {
          MCResultNum = 0;
        }
        candidateWords = &MCResultData[MCResultNum];
        /* Copy each candidate using the first candidate's word count, even when this candidate's
         * count differs; its reported count still comes from candidate. */
        for (wordIndex = 1; wordIndex < (firstCandidate->count - 1); wordIndex++) {
          MCResultData[MCResultNum++] = MicResultGet(candidate->result[wordIndex]);
          /* Ignore word-wordName lookup errors and log wordName even when the lookup leaves it
           * unassigned. */
          gsapi_EngineGetParam(MC_gsapiEngine,candidate->result[wordIndex],&wordName);
          OSReport(lbl_8023AD88, candidateIndex, MicResultGet(candidate->result[wordIndex]),
                   candidate->score, wordName);
        }
        candidates[candidateIndex].score = candidate->score;
        candidates[candidateIndex].count = candidate->count - 2;
        candidates[candidateIndex].result = candidateWords;
        candidate = candidate->next;
      }
    }
MicWriteResponseBuf(0,(u16 *)candidates);
    if (MCSessionCur >= 0) {
      MCSessionTimer[MCSessionCur] = 0;
    }
    ValidResultF = 1;
  }
  else {
    MicWriteResponse(-2,0,NULL,0,-1);
  }
}

/* While converting speech candidates, maps an engine word offset through the current context's
 * cumulative word-index table; returns -1 when no range contains it. */
static s32 MicResultGet(s32 wordOffset)

{
  s32 wordCount;
  s32 wordLimit;
  s32 wordIndex;
  u16 *wordSizes;

  wordSizes = MCContextP->binData;
  wordCount = *wordSizes++;
  wordIndex = wordLimit = 0;
  for (; wordIndex < wordCount; wordSizes++, wordIndex++) {
    wordLimit += *wordSizes;
    if (wordLimit > wordOffset) {
      return wordIndex;
    }
  }
  return -1;
}

/* Queues a single speech, pad or cancellation response using the supplied result pointer.
 * Clears the other candidate scores but preserves their counts and pointers. The queue position
 * caps at 255; subsequent writes overwrite slot 255 without advancing it. */
static int MicWriteResponse(s16 responseType, s32 validResult, void *responseData,
                            u16 resultScore, s32 resultWord)

{
  short entryIndex;
  MCResponse_s *response;

  response = (MCResponse_s *)(MCResponseBuf + MCResponseNo * 96);
  response->status = responseType;
  response->score = resultScore;
  response->count = validResult;
  response->result = responseData;
  response->word = resultWord;
  response->entry[0].score = resultScore;
  response->entry[0].count = validResult;
  response->entry[0].result = responseData;
  for (entryIndex = 1; entryIndex < 10; entryIndex++) {
    response->entry[entryIndex].score = 0;
  }
  MCResponseNo = MCResponseNo + 1;
  if (MCResponseNo >= MIC_RESPONSE_CAPACITY) {
    MCResponseNo = MIC_RESPONSE_CAPACITY - 1;
  }
  return MCResponseNo;
}

/* Queues ten speech candidates and derives the summary from the first, retaining their result
 * pointers. The queue position caps at 255; subsequent writes overwrite slot 255 without
 * advancing it. */
static s32 MicWriteResponseBuf(s16 responseType, u16 *entries)

{
  short entryIndex;
  MCResponse_s *response;

  response = (MCResponse_s *)(MCResponseBuf + MCResponseNo * 96);
  response->status = responseType;
  response->score = ((MicResponseEntry_s *)entries)->score;
  response->count = ((MCResponseEntry_s *)entries)->count;
  response->result = ((MCResponseEntry_s *)entries)->result;
  response->word = (int)*((MCResponseEntry_s *)entries)->result;
  for (entryIndex = 0; entryIndex < 10; entryIndex++) {
    response->entry[entryIndex] =
        ((MCResponseEntry_s *)entries)[entryIndex];
  }
  MCResponseNo = MCResponseNo + 1;
  if (MCResponseNo >= MIC_RESPONSE_CAPACITY) {
    MCResponseNo = MIC_RESPONSE_CAPACITY - 1;
  }
  return MCResponseNo;
}

/* Loads a recognition asset into storage tagged for microphone shutdown, then frees the DVD
 * read buffer. Flushes the newly allocated destination before copying the file into it. */
static void *MCDVDRead(const char *path)

{
  void *discBuffer;
  void *assetBuffer;

  discBuffer = (void *)HuDvdDataRead(path);
  assetBuffer = (void *)HuMemDirectMallocNum(0,DirDataSize,MIC_ALLOC_TAG);
  DCFlushRange(assetBuffer,DirDataSize + MIC_CACHE_LINE_LAST_BYTE & MIC_CACHE_LINE_ALIGN_MASK);
  memcpy(assetBuffer,discBuffer,DirDataSize);
  HuMemDirectFree(discBuffer);
  return assetBuffer;
}

/* While loading context files, copies the path up to its first dot and appends the requested
 * extension in a shared filename buffer; the next call overwrites that buffer. */
static char *MakeMCFilename(char *path, char *extension)

{
  char *sourceCursor;
  char *filenameCursor;

  sourceCursor = path;
  filenameCursor = MCFileName;
  while ((*sourceCursor != '\0') && (*sourceCursor != '.')) {
    *filenameCursor++ = *sourceCursor++;
  }
  sourceCursor = extension;
  while (*sourceCursor != '\0') {
    *filenameCursor++ = *sourceCursor++;
  }
  *filenameCursor = '\0';
  return MCFileName;
}

/* Prepares and activates a context for one-shot or continuous recognition, then saves and imports
 * player sessions. Parameter errors are logged without aborting; session processing also runs
 * when activation fails. Returns the activation result. */
static s32 ActivateContext(s16 contextId)

{
  int apiResult;
  s32 activationResult;
  s32 micMode;
  char *reportStrings;

  reportStrings = lbl_8023A988;
  activationResult = gsapi_ContextSetParam(MCContext[contextId].context,5,10);
  if (activationResult != 0) {
    OSReport(reportStrings + 1038,activationResult);
  }
  activationResult = gsapi_ContextActivate(MC_gsapiEngine,MCContext[contextId].context);
  micMode = GwCommon.mic;
  if ((micMode == 1) && (MCSessionCur != -1)) {
    if ((MCSessionP == 0) &&
       (apiResult = gsapi_EngineSessionDataExport(MC_gsapiEngine,&MCSessionP), apiResult != 0)) {
      OSReport(reportStrings + 710,apiResult);
    }
    if (MCSessionPrev >= 0) {
      gsapi_EngineSessionDataFree(PlayerSession[MCSessionPrev]);
      HeapNum = MIC_SESSION_ALLOC_TAG;
      apiResult = gsapi_EngineSessionDataExport(MC_gsapiEngine,PlayerSession + MCSessionPrev);
      if (apiResult != 0) {
        OSReport(reportStrings + 738,apiResult,PlayerSession[MCSessionPrev]);
      }
      HeapNum = MIC_ALLOC_TAG;
    }
    MCSessionPrev = MCSessionCur;
    if (MCSessionCur == -2) {
      apiResult = gsapi_EngineSessionDataImport(MC_gsapiEngine,MCSessionP,0);
      if (apiResult != 0) {
        OSReport(reportStrings + 763,apiResult,MCSessionP);
      }
    } else if ((PlayerSession[MCSessionCur] != 0) &&
               (apiResult =
                    gsapi_EngineSessionDataImport(MC_gsapiEngine, PlayerSession[MCSessionCur], 0),
                apiResult != 0)) {
        OSReport(reportStrings + 788, apiResult);
    }
    if (MCSessionCur >= 0) {
        *(u8 *)((int)&MCSessionTimer + (int)MCSessionCur) = 0;
    }
  }
  return activationResult;
}

/* Speech-engine initialization receives a zero heap handle and success; no separate heap is
 * created. */
u32 heap_Open(u32 *heapHandle)

{
  *heapHandle = 0;
  return 0;
}

/* Speech-engine shutdown releases every heap-0 block in the current speech memory group and
 * reports success. */
u32 heap_Close(void)

{
  HuMemDirectFreeNum(0,HeapNum);
  return 0;
}

/* Speech-engine memory requests use the tail of heap 0 with the current speech group number;
 * the supplied heap handle is ignored. */
void *heap_Alloc(void *heapHandle, u32 byteCount)

{
  void *allocatedBlock;
  allocatedBlock = HuMemDirectTailMallocNum(0,byteCount,HeapNum);
  return allocatedBlock;
}

/* The speech engine requests zeroed storage here: count times size bytes, or NULL if no block
 * fits. Uses the current speech group in heap 0's tail and ignores the supplied heap handle. */
void *heap_Calloc(void *heapHandle, u32 elementCount, u32 elementSize)

{
  void *allocatedBlock;
  void *heapBlock;
  void *requestedBlock;
  void *block;

  allocatedBlock = HuMemDirectTailMallocNum(0,elementCount * elementSize,HeapNum);
  heapBlock = allocatedBlock;
  requestedBlock = heapBlock;
  block = requestedBlock;
  if (!block) {
    return 0;
  }
  memset(block,0,elementCount * elementSize);
  return block;
}

/* The speech engine resizes storage here in heap 0, ignoring the supplied heap handle. A NULL
 * pointer gets tail storage in the current group; an existing block keeps its group when moved. */
void *heap_Realloc(void *heapHandle, void *existingBlock, u32 byteCount)

{
  void *allocatedBlock;
  void *heapBlock;
  void *requestedBlock;
  void *block;

  if (existingBlock == 0) {
    allocatedBlock = HuMemDirectTailMallocNum(0,byteCount,HeapNum);
    heapBlock = allocatedBlock;
    requestedBlock = heapBlock;
    block = requestedBlock;
  }
  else {
    block = HuMemDirectRealloc(0,existingBlock,byteCount);
  }
  return block;
}

/* The speech engine releases the supplied memory block; the heap argument is ignored. */
void heap_Free(void *heapHandle, void *block)

{
  HuMemDirectFree(block);
  return;
}

/* Before volume queries, allocates and clears a 44,100-sample PCM history buffer and resets its
 * write position and microphone-driver sample index. */
void HuMCVolSampleCreate(void)

{
  MCVolData.sample = (void *)HuMemDirectMallocNum(0,MIC_VOLUME_SAMPLE_BUFFER_SIZE,MIC_ALLOC_TAG);
  memset(MCVolData.sample,0,MIC_VOLUME_SAMPLE_BUFFER_SIZE);
  MCVolData.sampleNo = 0;
  MCVolData.index = 0;
  return;
}

/* Updates PCM history from microphone channel 1 and scales the energy of the latest 1,024 samples
 * into a volume value. Inactive capture clears the history and returns -1. The result is clamped
 * only below zero, so it can exceed scale. */
s32 HuMCVolGet(s32 minDb,u32 scale)

{
  BOOL interruptsWereEnabled;
  s16 *sampleBuffer;
  s32 sampleIndex;
  s32 previousSampleIndex;
  s32 samplesToEnd;
  s32 samplesRead;
  s32 volume;
  u32 driverSampleIndex;
  s32 energySampleIndex;
  s32 sampleValue;
  f32 sampleEnergy;
  f32 levelDb;

  interruptsWereEnabled = OSDisableInterrupts();
  if (!MICIsActive(1)) {
    memset(MCVolData.sample,0,MIC_VOLUME_SAMPLE_BUFFER_SIZE);
    MCVolData.sampleNo = 0;
    MCVolData.index = 0;
    OSRestoreInterrupts(interruptsWereEnabled);
    return -1;
  }
  sampleBuffer = MCVolData.sample;
  sampleIndex = MCVolData.sampleNo;
  driverSampleIndex = MCVolData.index;
  previousSampleIndex = sampleIndex;
  samplesToEnd = MIC_VOLUME_SAMPLE_COUNT - sampleIndex;
  samplesRead = MICGetSamples(1,sampleBuffer + sampleIndex,driverSampleIndex,samplesToEnd);
  sampleIndex += samplesRead;
  driverSampleIndex = MICUpdateIndex(1,driverSampleIndex,samplesRead);
  if (sampleIndex >= MIC_VOLUME_SAMPLE_COUNT) {
    samplesRead = MICGetSamples(1,sampleBuffer,driverSampleIndex,previousSampleIndex);
    sampleIndex = samplesRead;
    driverSampleIndex = MICUpdateIndex(1,driverSampleIndex,samplesRead);
  }
  MCVolData.sampleNo = sampleIndex;
  MCVolData.index = driverSampleIndex;
  OSRestoreInterrupts(interruptsWereEnabled);

  sampleIndex--;
  if (sampleIndex < 0) {
    sampleIndex += MIC_VOLUME_SAMPLE_COUNT;
  }
  sampleEnergy = lbl_802C1E48;
  for (energySampleIndex = 0; energySampleIndex < MIC_VOLUME_ENERGY_SAMPLE_COUNT;
       energySampleIndex++) {
      sampleValue = sampleBuffer[sampleIndex];
      sampleEnergy += (f32) (sampleValue * sampleValue);
      sampleIndex--;
      if (sampleIndex < 0) {
          sampleIndex += MIC_VOLUME_SAMPLE_COUNT;
      }
  }
  if (sampleEnergy > lbl_802C1E48) {
    levelDb = (f32)(lbl_802C1EB0 * log10((double)(sampleEnergy / lbl_802C1EB8)));
  }
  else {
    levelDb = lbl_802C1E4C;
  }
  levelDb = (levelDb * (f32)scale) / (f32)-minDb;
  volume = (s32)levelDb;
  volume += scale;
  if (volume < 0) {
    volume = 0;
  }
  return volume;
}
