/* Microphone capture and spoken-response support used by game modes. */
#define _MATH_H
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
#define s_threshold__d_not_reached_8023ace6 (lbl_8023A988 + 0x35e)
#define s_Abnormal_condition___d__s_at_sam_8023ad00 (lbl_8023A988 + 0x378)
#define s_SPEECH_DETECTED_8023ad28 (lbl_8023A988 + 0x3a0)
#define s_LISTENING_8023ad39 (lbl_8023A988 + 0x3b1)
#define s_STOP_LISTEN_8023ad44 (lbl_8023A988 + 0x3bc)
#define s_SILENCE_DETECTED_8023ad51 (lbl_8023A988 + 0x3c9)

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

static u8 lbl_80287554[0x108];
static MCVolData_s MCVolData;
static char MCFileName[0x40];
static OSAlarm MCThreadAlarm;
static u8 MCUnkResponseData[0x60];
static MCSelWinWork_s MCSelWinWork;
static u8 MCCurResponse[0x60];
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
static u32 pad_10_802C04D4_sbss;

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
  ContextCur = 0xffff;
  HeapNum = MIC_ALLOC_TAG;
  InitMCSelWin();
  /* Disable floating-point exception reporting before initializing speech support. */
  OSSetErrorHandler(0x10,0);
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
  s16 i;
  s32 result;
  s32 rawLanguage;
  s32 heapCheck;

  languageIndex;
  (void)languageIndex;
  (void)rawLanguage;
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
  for (i = 0; i < 8; i++) {
    MCContext[i].context = 0;
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
  if ((MicBuffer = HuMemDirectMallocNum(0,12288,MIC_ALLOC_TAG)) == 0) {
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
  rawLanguage = GwCommon.languageNo;
  languageIndex = rawLanguage;
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
  heapCheck = OSCheckHeap(currentHeapHandle);
  OSReport("OSAlloc Size Left %dkb(%x)\n",OSCheckHeap(currentHeapHandle) / 1024,heapCheck);
  MCAnswerProc = HuPrcCreate(MCAnswerMain,65000,16384,0);
  HuPrcSetStat(MCAnswerProc,0xc);
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
  char *temp;
  char *scan;
  char *filename;
  MCContextData_s *context;
  s16 index;
  s32 error;

  if (HuMCMicSaveGet() != 1) {
    return -1;
  }
  temp = HuMemDirectMalloc(0,0x100);
  strcpy(temp,path);
  /* All non-Japanese languages append US to the full path; the dot scan does not alter it. */
  if (LanguageNo != 0) {
    scan = temp;
    while ((*scan != '\0') && (*scan != '.')) {
      scan++;
    }
    switch (LanguageNo) {
      case 1:
        strcat(temp,lbl_802BF984);
        break;
      default:
        strcat(temp,lbl_802BF984);
        break;
    }
  }
  /* Compare the original path in occupied slots before the first free slot; stored paths may end in
   * US. */
  for (index = 0; index < 8; index++) {
    if (MCContext[index].context == 0) {
      break;
    }
    if (strcmp(path,MCContext[index].path) == 0) {
      HuMemDirectFree(temp);
      return index;
    }
  }
  if (index == 8) {
    HuMemDirectFree(temp);
    return -1;
  }
  context = &MCContext[index];
  strcpy(context->path,temp);
  filename = MakeMCFilename(temp,lbl_802BF987);
  context->ctxData = MCDVDRead(filename);
  filename = MakeMCFilename(temp,lbl_802BF98C);
  context->gcdData = MCDVDRead(filename);
  filename = MakeMCFilename(temp,lbl_802BF991);
  context->wrdData = MCDVDRead(filename);
  filename = MakeMCFilename(temp,lbl_802BF996);
  context->binData = MCDVDRead(filename);
  /* Context-data errors are reported without aborting; parameter results are ignored, and the
   * slot index is still returned. */
  error = gsapi_ContextSetCtxData(context->ctxData,&context->context);
  if (error != 0) {
    OSReport("Error CTX %x\n",error);
  }
  error = gsapi_ContextSetGcdData(context->context,context->gcdData);
  if (error != 0) {
    OSReport("Error GCD %x\n",error);
  }
  error = gsapi_ContextSetWrdData(context->context,context->wrdData);
  if (error != 0) {
    OSReport("Error WRD %x\n",error);
  }
  gsapi_ContextSetParam(context->context,9,MCThreshold);
  if ((omcurovl == 0x4a) || (omcurovl == 0x4b) || (omcurovl == 0x46) ||
      (omcurovl == 0x66) || (omcurovl == 0x67) || (omcurovl == 0x69) ||
      (omcurovl == 0x68)) {
    gsapi_ContextSetParam(context->context,10,200);
  }
  else {
    gsapi_ContextSetParam(context->context,10,100);
  }
  HuMemDirectFree(temp);
  return index;
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
            *(MCResponseData_s *)(MCResponseBuf + (MCResponseLastNo * 0x60));
        MCResponseLastNo++;
        if (MCResponseNo <= MCResponseLastNo) {
            MCResponseLastNo = MCResponseNo;
        }
    } else {
        *(s16 *)&MCCurResponse[0] = -1;
        *(u16 *)&MCCurResponse[2] = 0xFFFF;
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

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

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

/* 80092aa0 HuMCMount */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

static char lbl_8023ABA0[] = "MIC Error: M2SSetActiveChannel() %d\n";
static char lbl_8023ABC5[] = "M2SSetShifts Error\n";

/* Mounts chan for speech setup, accepting an already-mounted result. After device and channel
* checks, logs shift rejection but continues. Gain and channel-1 start results are ignored,
* so their failures do not change the returned success status. */
inline s32 HuMCMount(s32 chan)
{
  s32 result;
  s32 idResult;
  s32 activeResult;
  s32 probeResult2;
  OSTick start;
  s32 probeResult;
  s32 deviceId;

  M2SClose();
  MicOpenF = 1;
  MCWrongDeviceF = 0;
  start = OSGetTick();
  while (OSTicksToMilliseconds(OSGetTick() - start) < 500) {
    if ((probeResult = MICProbeEx(chan)) != -1) {
      break;
    }
  }
  probeResult2 = probeResult;
  if ((result = probeResult2) != 0) {
    return result;
  }
  if ((result = MICMount(chan, MicBuffer, 0x3000, MCExtHandler)) != 0) {
    if (result != -4) {
      return result;
    }
    result = 0;
  }
  idResult = MICGetDeviceID(chan, &deviceId);
  if (idResult != 0) {
    return idResult;
  }
  if (deviceId != 0) {
    MCWrongDeviceF = 1;
    return -2;
  }
  activeResult = M2SSetActiveChannel(chan);
  if (activeResult == 0) {
    OSReport(lbl_8023ABA0, activeResult);
    return -0x80;
  }
  MICSetGain(chan, 0);
  if (M2SSetShifts(M2SShift) == 0) {
    OSReport(lbl_8023ABC5);
  }
  MicOpenF = 0;
  MICStart(1);
  M2SOpen();
  return result;
}

/* Polls a channel for up to half a second while a lost microphone is retried. */
static inline s32 MCProbeSub(s32 chan)
{
  s32 result;
  OSTick start;

  start = OSGetTick();
  while (OSTicksToMilliseconds(OSGetTick() - start) < 500) {
    if ((result = MICProbeEx(chan)) != -1) {
      break;
    }
  }
  return result;
}

/* Retries setup while the answer process or warning waits for a reconnect, accepting an
* already-mounted result. After device and channel checks, logs shift rejection but continues.
* Gain and channel-1 start results are ignored, so their failures do not change success. */
static inline s32 MCMountSub(s32 chan)
{
  s32 result;
  s32 idResult;
  s32 deviceId;
  s32 activeResult;

  M2SClose();
  MicOpenF = 1;
  MCWrongDeviceF = 0;
  if ((result = MCProbeSub(chan)) != 0) {
    return result;
  }
  if ((result = MICMount(chan, MicBuffer, 0x3000, MCExtHandler)) != 0) {
    if (result != -4) {
      return result;
    }
    result = 0;
  }
  idResult = MICGetDeviceID(chan, &deviceId);
  if (idResult != 0) {
    return idResult;
  }
  if (deviceId != 0) {
    MCWrongDeviceF = 1;
    return -2;
  }
  if ((activeResult = M2SSetActiveChannel(chan)) == 0) {
    OSReport(lbl_8023ABA0, activeResult);
    return -0x80;
  }
  MICSetGain(chan, 0);
  if (M2SSetShifts(M2SShift) == 0) {
    OSReport(lbl_8023ABC5);
  }
  MicOpenF = 0;
  MICStart(1);
  M2SOpen();
  return result;
}

/* 80092c34 MCExtHandler */

/* The microphone driver's detach callback marks capture unavailable and stops the speech stream. */
void MCExtHandler(void)

{
  MicOpenF = 1;
  M2SStop();
  OSReport(lbl_802BF99B);
  return;
}

/* 80092c68 HuMCShiftsSet */

/* Saves the requested PCM bit shift and attempts to apply it immediately;
   rejection is ignored and the saved value is retried during mounting. */
void HuMCShiftsSet(u32 shifts)

{
  M2SShift = shifts;
  M2SSetShifts(shifts);
  return;
}

/* 80092c9c HuMCMicSprCreate */

/* Creates an initially hidden microphone-status sprite at x and y;
   the answer process shows it while recognition is running. */
void HuMCMicSprCreate(f32 x,f32 y)

{
  void *data;
  ANIMDATA *anim;
  HUSPRID sprId;

  if (HuMCMicSaveGet() == 1) {
    data = HuAR_ARAMtoMRAMFileRead(MIC_STATUS_SPRITE_DATA_NUM,MIC_ALLOC_TAG,2);
    anim = HuSprAnimRead(data);
    MCSprGrpId = HuSprGrpCreate(1);
    sprId = HuSprCreate(anim,1,0);
    HuSprGrpMemberSet(MCSprGrpId,0,sprId);
    HuSprGrpPosSet(MCSprGrpId,lbl_802C1E48,lbl_802C1E48);
    HuSprPosSet(MCSprGrpId,0,x,y);
    HuSprAttrSet(MCSprGrpId,0,4);
  }
  return;
}

/* 80092d6c HuMCMicSprKill */

/* Microphone shutdown removes the status sprite group and clears its stored group ID. */
void HuMCMicSprKill(void)

{
  if (MCSprGrpId != -1) {
    HuSprGrpKill(MCSprGrpId);
    MCSprGrpId = -1;
  }
  return;
}

/* 80092da4 HuMCMicGet */

/* Returns a pending microphone choice when one exists, otherwise the saved game microphone mode. */
s32 HuMCMicGet(void)

{
  if (MCMicValue != -1) {
    return MCMicValue;
  }
  return GwCommon.mic;
}

/* 80092dcc HuMCMicSaveGet */

inline s32 HuMCMicSaveGet(void)

{
  return GwCommon.mic;
}

/* 80092de0 HuMCMicSet */

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

/* 80092e10 InitMCSelWin */

/* Game startup clears the pad-answer window ID and its update-process pointer. */
void InitMCSelWin(void)

{
  MCSelWinWork.winId = 0xffff;
  MCSelWinWork.proc = 0;
  return;
}

/* 80092e34 HuMCSelWinCreate */

/* In pad-answer mode, creates choice-window storage and its update process,
   saving x and y for later option layout. */
void HuMCSelWinCreate(f32 x,f32 y)

{
  s32 mic;
  MCSelWinWork.winId = 0xffff;
  mic = GwCommon.mic;
  if (mic == 2) {
    MCSelWinWork.winId = HuWinCreate((double)lbl_802C1E4C,(double)lbl_802C1E4C,0x20,0x20,0);
    MCSelWinWork.x = x;
    MCSelWinWork.y = y;
    MCSelWinWork.item = (u8 *)HuMemDirectMallocNum(0,256,MIC_ALLOC_TAG);
    *MCSelWinWork.item = 0;
    MCSelWinWork.order = HuMemDirectMallocNum(0,64,MIC_ALLOC_TAG);
    MCSelWinWork.choice = 0;
    MCSelWinWork.proc = HuPrcCreate(MCSelWinFunc,65000,0x2000,0);
    HuPrcSetStat(MCSelWinWork.proc,0xc);
  }
  return;
}

/* 80092f64 HuMCSelWinKill */

/* Shutdown removes the pad-answer window and its process,
   then frees the encoded choices and option-order buffers. */
void HuMCSelWinKill(void)

{
  if (MCSelWinWork.winId != -1) {
    HuWinKill(MCSelWinWork.winId);
  }
  MCSelWinWork.winId = 0xffff;
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

/* 80093050 HuMCSelWinItemRandSet */

/* WARNING: Removing unreachable block (ram,0x800935ec) */
/* WARNING: Removing unreachable block (ram,0x800935f4) */

static char lbl_8023ABD9[] = "Error: HuMCSelWinItemRandSet() %d<%d\n";

/* In pad-answer mode, prepares a hidden window for padNo using shownCount sorted choices,
   including fixedItem, from the shuffled message range. */
s32 HuMCSelWinItemRandSet(s32 messageBase,s16 itemCount,s16 fixedItem,
                          s16 shownCount,s16 padNo)
{
  s32 temp;
  s16 randomItem;
  s16 pass;
  u8 *item;
  s16 i;
  HuVec2f size;
  f32 x;
  f32 y;

  item = MCSelWinWork.item;
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

  for (i = 0; i < itemCount; i++) {
    MCSelWinWork.order[i] = (u8)i;
  }
  temp = MCSelWinWork.order[0];
  MCSelWinWork.order[0] = MCSelWinWork.order[fixedItem];
  MCSelWinWork.order[fixedItem] = temp;
  for (pass = 0; pass < 10; pass++) {
    for (i = 1; i < itemCount; i++) {
      randomItem = frandmod(itemCount - 1) + 1;
      temp = MCSelWinWork.order[i];
      MCSelWinWork.order[i] = MCSelWinWork.order[randomItem];
      MCSelWinWork.order[randomItem] = temp;
    }
  }
  for (pass = 0; pass < shownCount - 1; pass++) {
    for (i = 0; i < shownCount - pass - 1; i++) {
      if (MCSelWinWork.order[i] > MCSelWinWork.order[i + 1]) {
        temp = MCSelWinWork.order[i];
        MCSelWinWork.order[i] = MCSelWinWork.order[i + 1];
        MCSelWinWork.order[i + 1] = temp;
      }
    }
  }

  *item++ = 0xb;
  for (i = 0; i < shownCount; i++) {
    *item++ = 0x10;
    *item++ = 0xf;
    *item++ = 0x1f;
    *item++ = (u8)(i + 1);
    if (i + 1 < shownCount) {
      *item++ = 10;
    }
    HuWinInsertMesSizeGet(messageBase + MCSelWinWork.order[i],(s16)i);
  }
  *item++ = 0;
  HuWinMesMaxSizeGet(1,&size,MCSelWinWork.item);

  if (lbl_802C1E50 == MCSelWinWork.x) {
    x = lbl_802C1E58;
  }
  else if (lbl_802C1E60 == MCSelWinWork.x) {
    x = lbl_802C1E68 - lbl_802C1E6C * size.x;
  }
  else {
    x = MCSelWinWork.x;
  }
  if (lbl_802C1E70 == MCSelWinWork.y) {
    y = lbl_802C1E78;
  }
  else if (lbl_802C1E80 == MCSelWinWork.y) {
    y = lbl_802C1E88 - lbl_802C1E6C * size.y;
  }
  else {
    y = MCSelWinWork.y;
  }

  MCSelWinWork.winId = HuWinCreate(x,y,(s16)size.x,(s16)size.y,0);
  HuWinScaleSet(MCSelWinWork.winId,lbl_802C1E6C,lbl_802C1E6C);
  for (i = 0; i < itemCount; i++) {
    HuWinInsertMesSet(MCSelWinWork.winId,messageBase + MCSelWinWork.order[i],(s16)i);
  }
  HuWinAttrSet(MCSelWinWork.winId,0x10);
  HuWinAttrSet(MCSelWinWork.winId,0x4000);
  winData[MCSelWinWork.winId].padMask = 1 << padNo;
  HuWinDispOff(MCSelWinWork.winId);
  if (MCSelWinWork.proc != 0) {
    HuPrcKill(MCSelWinWork.proc);
  }
  MCSelWinWork.proc = HuPrcCreate(MCSelWinFunc,65000,0x2000,0);
  HuPrcSetStat(MCSelWinWork.proc,0xc);
  return MCSelWinWork.winId;
}

/* 80093614 HuMCSelWinItemSet */

/* Prepares all message choices for the given pad; the window stays hidden until recognition is
 * requested. */
void HuMCSelWinItemSet(s32 messageBase,s16 itemCount,s16 padNo)

{
  HuMCSelWinItemRandSet(messageBase,itemCount,0,itemCount,padNo);
  return;
}

/* 8009365c HuMCSelWinCheck */

/* Tests whether a pad-answer window ID exists; it does not test whether the window is visible. */
inline BOOL HuMCSelWinCheck(void)

{
  if (MCSelWinWork.winId == -1) {
    return FALSE;
  }
  return TRUE;
}

/* 80093680 HuMCSelModeGet */

/* Returns the pad-answer window's requested state, or -1 when no window exists. */
int HuMCSelModeGet(void)

{
  if (HuMCSelWinCheck() == FALSE) {
    return -1;
  }
  return MCSelWinWork.choice;
}

/* 800936d0 HuMCSelModeSet */

/* Requests a pad-answer window state; hiding an active choice also cancels its selection. */
void HuMCSelModeSet(short mode)

{
  HUWIN *win;

  if (MCSelWinWork.winId != -1) {
    if (mode == 2) {
      win = &winData[MCSelWinWork.winId];
      if (win->stat == 3) {
        win->stat = 0;
        win->choice = -1;
      }
    }
    MCSelWinWork.choice = mode;
  }
}

/* 80093748 MCSelWinFunc */

/* Runs for a pad-answer window: shows requested choices, queues the selected answer or
 * cancellation,
 * invokes the response callback, and hides the window. Pauses temporarily hide its
 * sprites. */
static void MCSelWinFunc(void)
{
  HUWIN *win;
  MCResponseCallback callback;
  s16 choice;
  s16 timer;
  s16 response;

  for (;;) {
    switch (MCSelWinWork.choice) {
      case 1:
        if (*MCSelWinWork.item == 0) {
          MCSelWinWork.choice = 0;
          break;
        }
        HuWinDispOn(MCSelWinWork.winId);
        HuWinMesSet(MCSelWinWork.winId,(u32)MCSelWinWork.item);
        win = &winData[MCSelWinWork.winId];
        while (win->stat != 0) {
          if (omUPauseFlag + omPauseChk() != 0) {
            HuWinDispOff(MCSelWinWork.winId);
          } else {
            HuWinDispOn(MCSelWinWork.winId);
          }
          HuPrcVSleep();
        }
        HuWinChoiceSet(MCSelWinWork.winId,0);
        while (win->stat != 0) {
          if (omUPauseFlag + omPauseChk() != 0) {
            HuWinDispOff(MCSelWinWork.winId);
          } else {
            HuWinDispOn(MCSelWinWork.winId);
          }
          HuPrcVSleep();
        }
        choice = win->choice;
        if (choice >= 0) {
          response = MCSelWinWork.order[choice];
          for (timer = 0; timer < 10; timer++) {
            if (omUPauseFlag + omPauseChk() != 0) {
              HuWinDispOff(MCSelWinWork.winId);
            } else {
              HuWinDispOn(MCSelWinWork.winId);
            }
            HuPrcVSleep();
          }
          /* The queued result points to this process's local response; the summary word is
           * one-based. */
          MicWriteResponse(0,1,&response,10000,response + 1);
        } else if (choice == -1) {
          MicWriteResponse(-2,0,NULL,0,-1);
        }
        if (MCContextCallback) {
          callback = MCContextCallback;
          callback(HuMCCurResponseGet());
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

/* 80093aa0 HuMCSelWinContextSet */

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
  MCListenerProc = HuPrcCreate(MCSelWinContextProc,65000,0x2000,0);
  MCListenerProc->property = (void *)(((u32)padNo << 16) | (u16)contextId);
  MCSelWinMaxTime = 0;
  return;
}

/* 80093b44 HuMCSelWinContextKill */

/* Cancels the one-shot answer process by queuing a no-result response, hiding pad choices or
 * stopping active one-shot speech recognition, then removing its listener process. */
void HuMCSelWinContextKill(void)
{
  s16 response;

  MicWriteResponse(-1,0,NULL,0,-1);
  if (HuMCSelWinCheck()) {
    HuMCSelModeSet(2);
    (void)(MCResponseBuf + MCResponseLastNo * 0x60 + 4);
  }
  else {
    if ((HuMCMicSaveGet() == 1) && (MCStat == 1) && (MicOpenF == 0)) {
      OSSendMessage(&MCMessageQueue,(OSMessage)2,1);
      OSSleepThread(&MCThreadQueue);
      MCStat = 0;
      if (MCResponseNo != 0) {
        response = ((MCResponse_s *)MCResponseBuf)[MCResponseLastNo].status;
        if ((response == 0) && (MCResponseNo != MCResponseLastNo)) {
          (void)(MCResponseBuf + MCResponseLastNo * 0x60 + 4);
        }
      }
    }
  }
  if (MCListenerProc) {
    HuPrcKill(MCListenerProc);
  }
  MCListenerProc = 0;
}

/* 80093ce8 HuMCSelWinMaxTimeSet */

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

/* 80093d0c HuMCSelWinMaxTimeGet */

/* Returns the remaining answer timeout in frames, represented as a float. */
f32 HuMCSelWinMaxTimeGet(void)

{
  return (f32)MCSelWinMaxTime;
}

/* 80093d38 HuMCSelWinChoiceGet */

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

/* 80093dec MCSelWinContextProc */

/* Waits for the microphone talk button or the selected pad's R button, runs one-shot recognition
 *
 * or pad choices, and ends after a response or timeout. Speech retries recover stopped listening.
 */
static void MCSelWinContextProc(void)
{
  HUPROCESS *proc;
  u32 property;
  u8 padNo;
  s32 context;
  s32 button;
  s16 responseNo;
  s16 timer;
  s16 *response;

  proc = HuPrcCurrentGet();
  property = (u32)proc->property;
  padNo = property >> 16;
  context = (u16)property;
  button = HuMCButtonGet();
  for (;;) {
    if (((HuMCMicSaveGet() == 1) && (HuMCButtonDownGet() || button)) ||
        (HuMCSelWinCheck() && (HuPadBtnDown[padNo] & 0x20))) {
      button = 0;
      HuMCContextSet(context);
      MCContextCallback = MCCallback;
      if (HuMCMicSaveGet() == 1) {
        responseNo = MCResponseNo;
        while (HuMCButtonGet()) {
          if (responseNo != MCResponseNo) {
            response = (s16 *)(MCResponseBuf + (MCResponseNo - 1) * 0x60);
            if ((*response == 0) || (*response == -2) || (*response == -4)) {
              break;
            }
            responseNo = MCResponseNo;
            HuMCResponseGet2();
            HuMCContextSet(context);
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
        if (responseNo == MCResponseNo) {
          timer = 0;
          while (timer < 60) {
            if (responseNo != MCResponseNo) {
              break;
            }
            HuPrcVSleep();
            timer++;
          }
          if (timer == 60) {
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

/* 8009482c HuMCListenerCreate */

/* When microphone support is enabled, replaces the current listener process and stores its context,
 * response callback, and pad number for continuous button-driven recognition. */
void HuMCListenerCreate(s16 contextId, MCResponseCallback responseCallback,
                        u8 padNo)

{
  s32 mic;

  mic = GwCommon.mic;
  if (mic != 0) {
    MCCallback = responseCallback;
    if (MCListenerProc != 0) {
      HuPrcKill(MCListenerProc);
    }
    MCListenerProc = HuPrcCreate(MCListenerFunc,65000,0x2000,0);
    MCListenerProc->property = (void *)(((u32)padNo << 16) | (u16)contextId);
  }
  return;
}

/* 800948cc HuMCListenerKill */

/* Queues a cancellation response, hides or removes pad choices, stops active continuous speech
 * recognition, and removes the listener process. */
void HuMCListenerKill(void)

{
  s16 response;
  s32 mic;

  MicWriteResponse(-1,0,0,0,-1);
  if (MCSelWinWork.winId == -1 ? FALSE : TRUE) {
    HuMCSelModeSet(2);
    (void)(MCResponseBuf + MCResponseLastNo * 0x60 + 4);
  }
  else {
    mic = GwCommon.mic;
    if (((mic == 1) && (MCStat == 2)) && (MicOpenF == 0)) {
      MCContextCallback = 0;
      OSSendMessage(&MCMessageQueue,(OSMessage)2,1);
      OSSleepThread(&MCThreadQueue);
      response = ((MCResponse_s *)MCResponseBuf)[MCResponseLastNo].status;
      MCStat = 0;
      if (response == 0) {
        (void)(MCResponseNo == MCResponseLastNo);
      }
    }
  }
  if (HuMCSelWinCheck()) {
    HuMCSelWinKill();
  }
  if (MCListenerProc != 0) {
    HuPrcKill(MCListenerProc);
  }
  MCListenerProc = 0;
  return;
}

/* 80094b48 MCListenerFunc */

/* Repeatedly listens while the microphone talk button is held. If no response is queued on
* release, waits up to 60 frames for one and then sleeps one additional frame before stopping.
* A pad R press starts choices but requests hiding on the following frame. Pending valid
* responses are forwarded to the callback. */
static void MCListenerFunc(void)
{
  HUPROCESS *process;
  u32 property;
  u8 padNo;
  u16 context;
  s16 responseNo;
  s16 timer;

  process = HuPrcCurrentGet();
  property = (u32)process->property;
  padNo = property >> 16;
  context = property;
  responseNo = 100;
  for (;;) {
    if (((HuMCMicSaveGet() == 1) && HuMCButtonGet()) ||
        (HuMCSelWinCheck() && (HuPadBtnDown[padNo] & 0x20))) {
      MCResponseCallback contextCallback;

      contextCallback = MCCallback;
      HuMCContextCallbackSet(context,contextCallback);
      responseNo = MCResponseNo;
      if (HuMCMicSaveGet() == 1) {
        while (HuMCButtonGet()) {
          HuPrcVSleep();
        }
        if (responseNo == MCResponseNo) {
          for (timer = 0; timer < 60; timer++) {
            if (responseNo != MCResponseNo) {
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

/* 80095034 MCAnswerMain */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

/* Runs while speech support is open: retries a lost microphone, reads talk-button transitions,
 * dispatches queued responses, and shows the status sprite only while recognition is running. */
static void MCAnswerMain(void)
{
  /* oldButton and oldResponseNo have no initial values before their first comparisons. */
  u32 oldButton;
  s16 oldResponseNo;
  MCResponseCallback callback;

  (void)oldResponseNo;
  ValidResultF = 0;
  for (;;) {
    do {
      HuPrcVSleep();
    } while (HuMCSelWinCheck() || (HuMCMicSaveGet() != 1));

    if ((MicOpenF != 0) && (HuMCMicSaveGet() == 1)) {
      if ((omcurovl != 0x5d) && (omcurovl != 0x72) &&
          (omcurovl != 0x73) && (omcurovl != 0x74)) {
        s32 mountResult;

        mountResult = -3;
        while ((WipeCheckIn() != 0) || (Hu3DPauseF != 0)) {
          mountResult = MCMountSub(1);
          if (mountResult == 0) {
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
    MCButtonDown = MCButton & (oldButton ^ MCButton);
    oldButton = MCButton;
    if (MCContextCallback && (oldResponseNo != MCResponseNo)) {
      if (oldResponseNo < MCResponseNo) {
        callback = MCContextCallback;
        (*callback)(HuMCCurResponseGet());
        if (ValidResultF != 0) {
          ValidResultF = 0;
        }
      }
      oldResponseNo = MCResponseNo;
    }
    if (MCSprStat == 4) {
      OSReport(s_MIC_Error__Engine__x___8023ac23,MC_gsapiEngineError);
    }
    if (MCSprGrpId != -1) {
      if (MCSprStat == 1) {
        HuSprAttrReset(MCSprGrpId,0,4);
      } else {
        HuSprAttrSet(MCSprGrpId,0,4);
      }
    }
  }
}

/* 800955e0 HuMCSessionSet */

/* Selects the session ID used by later context activation in microphone mode; IDs below four,
 * including -1 to skip session switching and -2 to restore the baseline session, are accepted. */
void HuMCSessionSet(short playerIndex)

{
  s32 mic;

  mic = GwCommon.mic;
  if ((mic == 1) && (playerIndex < 4)) {
    MCSessionCur = playerIndex;
  }
  return;
}

/* 8009561c HuMCSessionClose */

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

/* 800956e4 HuMCSessionKill */

/* In microphone mode, restores the baseline engine session before freeing this player's saved
 * session; both engine results are ignored. */
void HuMCSessionKill(short playerIndex)

{
  s32 mic;

  mic = GwCommon.mic;
  if ((mic != 1) || (MC_gsapiEngine == 0)) {
    return;
  }
  if (PlayerSession[playerIndex] != 0) {
    gsapi_EngineSessionDataImport(MC_gsapiEngine,MCSessionP,0);
    gsapi_EngineSessionDataFree(PlayerSession[playerIndex]);
    PlayerSession[playerIndex] = 0;
  }
  return;
}

/* 800957a4 HuMCUnkResponseCheck */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

/* Searches backward for the latest successful response. Saves it for confirmation when
* at least two distinct first words occur among its first three candidate slots with nonzero
* scores and the first distinct word's signed score minus the second's is at most 1000.
* The score comparison does not use an absolute difference. */
s32 HuMCUnkResponseCheck(void)
{
  MCResponse_s *response;
  s32 i;
  s16 word[3];
  s16 score[3];
  s16 count;
  s32 j;

  ((MCResponse_s *)MCUnkResponseData)->entry[0].score =
      ((MCResponse_s *)MCUnkResponseData)->entry[1].score =
      ((MCResponse_s *)MCUnkResponseData)->entry[2].score = 0;
  for (i = MCResponseNo - 1; i >= 0; i--) {
    response = (MCResponse_s *)(MCResponseBuf + i * MIC_RESPONSE_RECORD_SIZE);
    if (response->status == 0) {
      break;
    }
  }
  if (i == -1) {
    return 0;
  }
  if (response->entry[0].score == 0) {
    return 0;
  }

  i = count = 0;
  for (; i < 3; i++) {
    if (response->entry[i].score != 0) {
      for (j = 0; j < count; j++) {
        if (*response->entry[i].result == word[j]) {
          break;
        }
      }
      if (j == count) {
        word[count] = *response->entry[i].result;
        score[count] = response->entry[i].score;
        count++;
      }
    }
  }
  if (count <= 1) {
    return 0;
  }
  if ((score[0] - score[1]) <= 1000) {
    *(MCResponse_s *)MCUnkResponseData = *response;
    return 1;
  }
  return 0;
}

/* 80095968 HuMCNewResponseGet */

static char lbl_8023AC3B[] =
    "/mic/ctx/mic_yesno\0"
    "Reset Session Export >>>%x\n\0"
    "Session Export >>>%x:%x\n\0"
    "Session Import >>>%x:%x\n\0"
    "Session Import >>>%x\n";

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

/* Confirms up to three distinct words from the saved ambiguous response with successive yes/no
 *
 * prompts. If the yes/no context cannot be created, returns the first candidate without
 * confirmation. */
s32 HuMCNewResponseGet(s32 messageBase)
{
  MCResponse_s *response;
  HuVec2f size;
  s16 word[3];
  s16 score[3];
  s16 count;
  s32 i;
  s32 j;
  s32 choice;
  s32 window;

  response = (MCResponse_s *)MCUnkResponseData;
  if (response->entry[0].score == 0) {
    return -1;
  }
  if (MCYesNoCtxId == -1) {
    MCYesNoCtxId = HuMCContextCreate(lbl_8023AC3B);
    if (MCYesNoCtxId == -1) {
      return *response->entry[0].result;
    }
  }

  i = count = 0;
  for (; i < 3; i++) {
    if (response->entry[i].score != 0) {
      for (j = 0; j < count; j++) {
        if (*response->entry[i].result == word[j]) {
          break;
        }
      }
      if (j == count) {
        word[count] = *response->entry[i].result;
        score[count] = response->entry[i].score;
        count++;
      }
    }
  }
  if (count <= 1) {
    return -1;
  }

  for (i = 0; i < count; i++) {
    HuWinInsertMesSizeGet(messageBase + word[i],0);
    HuWinMesMaxSizeGet(1,&size,MIC_RECOGNIZED_WORD_CONFIRM_MESSAGE);
    window = HuWinExCreateFrame(lbl_802C1E98,lbl_802C1E9C,
                               (s16)size.x,(s16)size.y,-1,0);
    HuWinExOpen(window);
    HuWinMesSpeedSet(window,0);
    HuWinAttrSet(window,0x800);
    HuWinInsertMesSet(window,messageBase + word[i],0);
    HuWinMesSet(window,MIC_RECOGNIZED_WORD_CONFIRM_MESSAGE);
    do {
      s32 context = MCYesNoCtxId;
      HuMCSelWinContextSet(context,NULL,0);
    } while (((choice = HuMCSelWinChoiceGet()) != 0) && (choice != 1));
    HuWinExClose(window);
    HuWinKill(window);
    if (choice == 0) {
      break;
    }
  }
  if (i == count) {
    return -1;
  }
  return word[i];
}

/* 80095e20 HuMCSessionExportReset */

/* During microphone context switching or shutdown, creates a baseline session if needed and
 * saves the previous player's engine session. Imports a saved selected-player session or, for
 * ID -2, the baseline; an unsaved player leaves the engine session unchanged. */
void HuMCSessionExportReset(void)

{
  int apiResult;
  s32 mic;
  char *strings;

  strings = lbl_8023A988;
  mic = GwCommon.mic;
  if ((mic != 1) || (MCSessionCur == -1)) {
    return;
  }
  {
    if ((MCSessionP == 0) &&
       (apiResult = gsapi_EngineSessionDataExport(MC_gsapiEngine,&MCSessionP), apiResult != 0)) {
      OSReport(strings + 710,apiResult);
    }
    if (MCSessionPrev >= 0) {
      gsapi_EngineSessionDataFree(PlayerSession[MCSessionPrev]);
      HeapNum = MIC_SESSION_ALLOC_TAG;
      apiResult = gsapi_EngineSessionDataExport(MC_gsapiEngine,PlayerSession + MCSessionPrev);
      if (apiResult != 0) {
        OSReport(strings + 738,apiResult,PlayerSession[MCSessionPrev]);
      }
      HeapNum = MIC_ALLOC_TAG;
    }
    MCSessionPrev = MCSessionCur;
    if (MCSessionCur == -2) {
      apiResult = gsapi_EngineSessionDataImport(MC_gsapiEngine,MCSessionP,0);
      if (apiResult != 0) {
        OSReport(strings + 763,apiResult,MCSessionP);
      }
    } else if ((PlayerSession[MCSessionCur] != 0) &&
               (apiResult =
                    gsapi_EngineSessionDataImport(MC_gsapiEngine, PlayerSession[MCSessionCur], 0),
                apiResult != 0)) {
        OSReport(strings + 788, apiResult);
    }
    if (MCSessionCur >= 0) {
      *(u8 *)((int)&MCSessionTimer + (int)MCSessionCur) = 0;
    }
  }
  return;
}

/* 80096008 MCDeviceMesExec */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

/* Pauses gameplay and shows the device warning until an A-button retry mounts the microphone,
 * then resumes continuous recognition if needed. User and object pause are always cleared on exit;
 * process, model-motion, and sprite pause stay set if the game was already paused. */
static void MCDeviceMesExec(void)
{
  char paused;
  u8 pauseEnable;
  s32 pad;
  ANIMDATA *anim;
  void *fileData;
  HUSPR_GROUPID group;
  HUSPRID sprite;
  HUWINID window;
  s32 result;

  pauseEnable = omSysPauseEnableFlag;
  paused = omPauseChk();

  omUPauseFlag = 1;
  omSysPauseEnable(0);
  omObjManPause(1);
  HuPrcAllPause(1);
  Hu3DPauseSet(1);
  HuSprPauseSet(1);
  HuPadRumbleAllStop();

  fileData = HuAR_ARAMtoMRAMFileRead(MIC_DEVICE_ALERT_SPRITE_DATA_NUM,MIC_ALLOC_TAG,2);
  anim = HuSprAnimRead(fileData);
  group = HuSprGrpCreate(1);
  sprite = HuSprCreate(anim,0,0);
  HuSprGrpMemberSet(group,0,sprite);
  HuSprTPLvlSet(group,0,lbl_802C1EA0);
  HuSprGrpPosSet(group,lbl_802C1EA4,lbl_802C1EA8);
  HuSprScaleSet(group,0,lbl_802C1EAC,lbl_802C1EAC);
  window = HuWinWarningCreate(lbl_802C1E98,lbl_802C1E98,0x19c,0x78);
  HuWinPriSet(window,0);
  HuWinMesSpeedSet(window,0);
  HuWinWarningOpen(window);
  MCButton = MCButtonDown = 0;
  if (MCWrongDeviceF != 0) {
    result = -2;
  }
  else {
    result = -3;
  }

  while (result != 0) {
    if (result == -2) {
      HuWinMesSet(window,MIC_WRONG_DEVICE_MESSAGE);
    }
    else {
      HuWinMesSet(window,MIC_DEVICE_UNAVAILABLE_MESSAGE);
    }
    HuWinMesWait(window);
    for (;;) {
      for (pad = 0; pad < 4; pad++) {
        if (HuPadBtnDown[pad] & 0x100) {
          break;
        }
      }
      if (pad != 4) {
        HuPrcVSleep();
        break;
      }
      HuPrcVSleep();
    }
    result = MCMountSub(1);
  }

  HuWinWarningClose(window);
  HuWinWarningKill(window);
  HuSprGrpKill(group);
  MICStart(1);
  if ((MCStat == 2) && (ContextCur >= 0)) {
    MCStat = 0;
    HuMCContextCallbackSet(ContextCur,MCContextCallback);
  }
  omUPauseFlag = 0;
  omSysPauseEnable(pauseEnable);
  omObjManPause(0);
  if (!paused) {
    HuPrcAllPause(0);
    Hu3DPauseSet(0);
    HuSprPauseSet(0);
  }
}

/* 80096534 MCThreadFunc */

/* Runs the speech engine's start, audio-processing, and stop states from queued commands.
 * Suspends for pad-answer mode, disabled microphone mode, or engine errors. A stop error suspends
 * before waking stop waiters, leaving them asleep until this thread resumes. */
static void *MCThreadFunc(void *arg)

{
  OSMessage message;
  s32 mic;

  for (;;) {
    if (OSReceiveMessage(&MCMessageQueue,&message,0)) {
      MCSprStat = (s32)message;
    }
    if (HuMCSelWinCheck() || (mic = GwCommon.mic, mic != 1)) {
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
        OSSetThreadPriority(&MCThread,0x1f);
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
        OSSetThreadPriority(&MCThread,0x1f);
        continue;
    }
  }
}

/* 800966bc MCThreadWakeup */

/* The five-millisecond pad-update alarm restores the speech thread's low priority; its arguments
 * are unused. */
static void MCThreadWakeup(OSAlarm *alarm, OSContext *context)

{
  OSSetThreadPriority(&MCThread,0x1f);
  return;
}

/* 800966e8 HuMCPeriodicProc */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

/* Pad updates briefly raise the initialized microphone speech thread's priority and schedule a
 * five-millisecond alarm to lower it again. */
void HuMCPeriodicProc(void)

{
  s32 mic;

  if ((MCInitF == 0) || (MCResponseBuf == 0) ||
      (mic = GwCommon.mic, mic != 1)) {
    return;
  }
  OSCreateAlarm(&MCThreadAlarm);
  OSSetAlarm(&MCThreadAlarm,(OSTime)OSMillisecondsToTicks(5),MCThreadWakeup);
  OSSetThreadPriority(&MCThread,0);
  return;
}

/* 80096794 MicNotifyCallBack */

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
  short session1;
  short session2;
  s32 mic1;
  s32 mic2;
  char *strings;

  strings = lbl_8023A988;
  switch(event) {
  case 0:
    if (((value < 4000) && (MCSessionCur >= 0)) &&
       (MCSessionTimer[MCSessionCur]++ > 2)) {
      session1 = MCSessionCur;
      mic1 = GwCommon.mic;
      if (((mic1 == 1) && (MC_gsapiEngine != 0)) &&
         (PlayerSession[session1] != 0)) {
        gsapi_EngineSessionDataImport(MC_gsapiEngine,MCSessionP,0);
        gsapi_EngineSessionDataFree(PlayerSession[session1]);
        PlayerSession[session1] = 0;
      }
      *(u8 *)((int)&MCSessionTimer + (int)MCSessionCur) = 0;
    }
  case 4:
    if (event == 0) {
      OSReport(strings + 0x35e,value);
      MicWriteResponse(MIC_THRESHOLD_MISSED_RESPONSE,0,0,0,MIC_RESPONSE_NO_RESULT);
    }
    else {
      if ((MCSessionCur >= 0) && (MCSessionTimer[MCSessionCur]++ > 2)) {
        session2 = MCSessionCur;
        mic2 = GwCommon.mic;
        if ((mic2 == 1) &&
           ((MC_gsapiEngine != 0 && (PlayerSession[session2] != 0)))) {
          gsapi_EngineSessionDataImport(MC_gsapiEngine,MCSessionP,0);
          gsapi_EngineSessionDataFree(PlayerSession[session2]);
          PlayerSession[session2] = 0;
        }
        *(u8 *)((int)&MCSessionTimer + (int)MCSessionCur) = 0;
      }
      if (value == 1) {
        MicWriteResponse(MIC_ENGINE_CONDITION_RESPONSE,0,0,0,MIC_RESPONSE_NO_RESULT);
      }
      OSReport(strings + 0x378,value,
               MCErrorTbl[value],sample);
    }
    break;
  case 7:
    OSReport(strings + 0x3a0);
    break;
  case 2:
    OSReport(strings + 0x3b1);
    MCListenF = 0;
    break;
  case 3:
    OSReport(strings + 0x3bc);
    MCListenF = 1;
    break;
  case 8:
    OSReport(strings + 0x3c9);
  }
}

/* 80096a60 MicResultCallBack */

/* Called by the speech engine when a recognition result is ready. */

void MicResultCallBack(u32 callbackContext, u32 callbackData, u32 resultNode)

{
  MicResultExec(callbackContext,callbackData,(MicResultNode_s *)resultNode);
  return;
}

/* 80096a98 MicResultExec */

/* Converts the engine's candidate list into the response queue for game modes. */

static char lbl_8023AD88[] =
    "%d:%d(%d),%s\n\0"
    "NBEST Error %x\n\0\0";

static void MicResultExec(u32 callbackContext, u32 callbackData,
                          MicResultNode_s *root)

{
  MicResultNode_s *node;
  s16 *resultP;
  s16 i;
  s16 j;
  char *name;
  MicResponseEntry_s entry[10];

  node = root;
  if (root->count != 0) {
    for (node = root, i = 0; i < 10; i++) {
      if (!node || (node->count == 0)) {
        /* Missing candidates clear score and count only; their result pointer remains unset. */
        entry[i].score = 0;
        entry[i].count = 0;
        }
        else {
        if (((node->count + (-2)) + MCResultNum) >= 0x40) {
          MCResultNum = 0;
        }
        resultP = &MCResultData[MCResultNum];
        /* Copy each candidate using the first candidate's word count, even when this candidate's
         * count differs; its reported count still comes from node. */
        for (j = 1; j < (root->count - 1); j++) {
          MCResultData[MCResultNum++] = MicResultGet(node->result[j]);
          /* Ignore word-name lookup errors and log name even when the lookup leaves it
           * unassigned. */
          gsapi_EngineGetParam(MC_gsapiEngine,node->result[j],&name);
          OSReport(lbl_8023AD88,i,MicResultGet(node->result[j]),node->score,name);
        }
        entry[i].score = node->score;
        entry[i].count = node->count - 2;
        entry[i].result = resultP;
        node = node->next;
      }
    }
MicWriteResponseBuf(0,(u16 *)entry);
    if (MCSessionCur >= 0) {
      MCSessionTimer[MCSessionCur] = 0;
    }
    ValidResultF = 1;
  }
  else {
    MicWriteResponse(-2,0,NULL,0,-1);
  }
}

/* 80096ca4 MicResultGet */

/* While converting speech candidates, maps an engine word offset through the current context's
 * cumulative word-index table; returns -1 when no range contains it. */
static s32 MicResultGet(s32 wordOffset)

{
  s32 count;
  s32 total;
  s32 i;
  u16 *ptr;

  ptr = MCContextP->binData;
  count = *ptr++;
  i = total = 0;
  for (; i < count; ptr++, i++) {
    total += *ptr;
    if (total > wordOffset) {
      return i;
    }
  }
  return -1;
}

/* 80096d1c MicWriteResponse */

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
  if (MCResponseNo >= 0x100) {
    MCResponseNo = 0xff;
  }
  return MCResponseNo;
}

/* 80096dc0 MicWriteResponseBuf */

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
  if (MCResponseNo >= 0x100) {
    MCResponseNo = 0xff;
  }
  return MCResponseNo;
}

/* 80096e7c MCDVDRead */

/* Loads a recognition asset into storage tagged for microphone shutdown, then frees the DVD
 * read buffer. Flushes the newly allocated destination before copying the file into it. */
static void *MCDVDRead(const char *path)

{
  void *__src;
  void *__dest;

  __src = (void *)HuDvdDataRead(path);
  __dest = (void *)HuMemDirectMallocNum(0,DirDataSize,MIC_ALLOC_TAG);
  DCFlushRange(__dest,DirDataSize + MIC_CACHE_LINE_LAST_BYTE & MIC_CACHE_LINE_ALIGN_MASK);
  memcpy(__dest,__src,DirDataSize);
  HuMemDirectFree(__src);
  return __dest;
}

/* 80096efc MakeMCFilename */

/* While loading context files, copies the path up to its first dot and appends the requested
 * extension in a shared filename buffer; the next call overwrites that buffer. */
static char *MakeMCFilename(char *path, char *extension)

{
  char *src;
  char *dst;

  src = path;
  dst = MCFileName;
  while ((*src != '\0') && (*src != '.')) {
    *dst++ = *src++;
  }
  src = extension;
  while (*src != '\0') {
    *dst++ = *src++;
  }
  *dst = '\0';
  return MCFileName;
}

/* 80096f84 ActivateContext */

/* Prepares and activates a context for one-shot or continuous recognition, then saves and imports
 * player sessions. Parameter errors are logged without aborting; session processing also runs
 * when activation fails. Returns the activation result. */
static s32 ActivateContext(s16 context)

{
  int apiResult;
  s32 result;
  s32 mic;
  char *strings;

  strings = lbl_8023A988;
  result = gsapi_ContextSetParam(MCContext[context].context,5,10);
  if (result != 0) {
    OSReport(strings + 0x40e,result);
  }
  result = gsapi_ContextActivate(MC_gsapiEngine,MCContext[context].context);
  mic = GwCommon.mic;
  if ((mic == 1) && (MCSessionCur != -1)) {
    if ((MCSessionP == 0) &&
       (apiResult = gsapi_EngineSessionDataExport(MC_gsapiEngine,&MCSessionP), apiResult != 0)) {
      OSReport(strings + 710,apiResult);
    }
    if (MCSessionPrev >= 0) {
      gsapi_EngineSessionDataFree(PlayerSession[MCSessionPrev]);
      HeapNum = MIC_SESSION_ALLOC_TAG;
      apiResult = gsapi_EngineSessionDataExport(MC_gsapiEngine,PlayerSession + MCSessionPrev);
      if (apiResult != 0) {
        OSReport(strings + 738,apiResult,PlayerSession[MCSessionPrev]);
      }
      HeapNum = MIC_ALLOC_TAG;
    }
    MCSessionPrev = MCSessionCur;
    if (MCSessionCur == -2) {
      apiResult = gsapi_EngineSessionDataImport(MC_gsapiEngine,MCSessionP,0);
      if (apiResult != 0) {
        OSReport(strings + 763,apiResult,MCSessionP);
      }
    } else if ((PlayerSession[MCSessionCur] != 0) &&
               (apiResult =
                    gsapi_EngineSessionDataImport(MC_gsapiEngine, PlayerSession[MCSessionCur], 0),
                apiResult != 0)) {
        OSReport(strings + 788, apiResult);
    }
    if (MCSessionCur >= 0) {
        *(u8 *)((int)&MCSessionTimer + (int)MCSessionCur) = 0;
    }
  }
  return result;
}

/* 800971c4 heap_Open */

/* Speech-engine initialization receives a zero heap handle and success; no separate heap is
 * created. */
u32 heap_Open(u32 *heapHandle)

{
  *heapHandle = 0;
  return 0;
}

/* 800971d4 heap_Close */

/* Speech-engine shutdown releases every heap-0 block in the current speech memory group and
 * reports success. */
u32 heap_Close(void)

{
  HuMemDirectFreeNum(0,HeapNum);
  return 0;
}

/* 80097200 heap_Alloc */

/* Speech-engine memory requests use the tail of heap 0 with the current speech group number;
 * the supplied heap handle is ignored. */
void *heap_Alloc(void *heap, u32 size)

{
  void *result;
  result = HuMemDirectTailMallocNum(0,size,HeapNum);
  return result;
}

/* 80097240 heap_Calloc */

/* The speech engine requests zeroed storage here: count times size bytes, or NULL if no block
 * fits. Uses the current speech group in heap 0's tail and ignores the supplied heap handle. */
void *heap_Calloc(void *heap, u32 count, u32 size)

{
  void *alloc;
  void *result1;
  void *result2;
  void *result;

  alloc = HuMemDirectTailMallocNum(0,count * size,HeapNum);
  result1 = alloc;
  result2 = result1;
  result = result2;
  if (!result) {
    return 0;
  }
  memset(result,0,count * size);
  return result;
}

/* 800972b8 heap_Realloc */

/* The speech engine resizes storage here in heap 0, ignoring the supplied heap handle. A NULL
 * pointer gets tail storage in the current group; an existing block keeps its group when moved. */
void *heap_Realloc(void *heap, void *ptr, u32 size)

{
  void *alloc;
  void *result1;
  void *result2;
  void *result;

  if (ptr == 0) {
    alloc = HuMemDirectTailMallocNum(0,size,HeapNum);
    result1 = alloc;
    result2 = result1;
    result = result2;
  }
  else {
    result = HuMemDirectRealloc(0,ptr,size);
  }
  return result;
}

/* 80097330 heap_Free */

/* The speech engine releases the supplied memory block; the heap argument is ignored. */
void heap_Free(void *heap, void *ptr)

{
  HuMemDirectFree(ptr);
  return;
}

/* 80097358 HuMCVolSampleCreate */

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

/* 800973d0 HuMCVolGet */

/* WARNING: Removing unreachable block (ram,0x80097624) */
/* WARNING: Removing unreachable block (ram,0x8009762c) */

/* Updates PCM history from microphone channel 1 and scales the energy of the latest 1,024 samples
 * into a volume value. Inactive capture clears the history and returns -1. The result is clamped
 * only below zero, so it can exceed scale. */
s32 HuMCVolGet(s32 minDb,u32 scale)

{
  BOOL interrupts;
  s16 *sample;
  s32 sampleNo;
  s32 oldSampleNo;
  s32 remaining;
  s32 got;
  s32 result;
  u32 index;
  s32 i;
  s32 value;
  f32 power;
  f32 db;

  interrupts = OSDisableInterrupts();
  if (!MICIsActive(1)) {
    memset(MCVolData.sample,0,MIC_VOLUME_SAMPLE_BUFFER_SIZE);
    MCVolData.sampleNo = 0;
    MCVolData.index = 0;
    OSRestoreInterrupts(interrupts);
    return -1;
  }
  sample = MCVolData.sample;
  sampleNo = MCVolData.sampleNo;
  index = MCVolData.index;
  oldSampleNo = sampleNo;
  remaining = 0xac44 - sampleNo;
  got = MICGetSamples(1,sample + sampleNo,index,remaining);
  sampleNo += got;
  index = MICUpdateIndex(1,index,got);
  if (sampleNo >= 0xac44) {
    got = MICGetSamples(1,sample,index,oldSampleNo);
    sampleNo = got;
    index = MICUpdateIndex(1,index,got);
  }
  MCVolData.sampleNo = sampleNo;
  MCVolData.index = index;
  OSRestoreInterrupts(interrupts);

  sampleNo--;
  if (sampleNo < 0) {
    sampleNo += 0xac44;
  }
  power = lbl_802C1E48;
  for (i = 0; i < 0x400; i++) {
    value = sample[sampleNo];
    power += (f32)(value * value);
    sampleNo--;
    if (sampleNo < 0) {
      sampleNo += 0xac44;
    }
  }
  if (power > lbl_802C1E48) {
    db = (f32)(lbl_802C1EB0 * log10((double)(power / lbl_802C1EB8)));
  }
  else {
    db = lbl_802C1E4C;
  }
  db = (db * (f32)scale) / (f32)-minDb;
  result = (s32)db;
  result += scale;
  if (result < 0) {
    result = 0;
  }
  return result;
}
