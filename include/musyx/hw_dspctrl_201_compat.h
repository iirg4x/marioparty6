#ifndef MUSYX_HW_DSPCTRL_201_COMPAT_H
#define MUSYX_HW_DSPCTRL_201_COMPAT_H

/*
 * Translation-unit-only MP4 MusyX 2.0.1+ visibility for hw_dspctrl.c.
 *
 * The canonical MP6 headers in this workspace predate the LPF/compressor
 * fields used by the authenticated MP4 whole-TU donor.  Keep those headers
 * untouched for every other owner: this overlay supplies only the exact
 * donor layouts and prototypes needed by hw_dspctrl.c, then marks the stale
 * public guards so the donor's following includes remain no-ops.
 */
#include "musyx/musyx.h"

/* These are the public headers included by the donor after this overlay. */
#define DSPVOICE_H
#define _MUSYX_HARDWARE
#define SAL_H
#define _MUSYX_STREAM
#define _MUSYX_VOICE_H_
#define _MUSYX_SYNTH
#define ADSR_H
#define SYNTHDATA_H
#define _MUSYX_MACROS

typedef struct SNDADPCMinfo {
  // total size: 0x28
  u16 numCoef;       // offset 0x0, size 0x2
  u8 initialPS;      // offset 0x2, size 0x1
  u8 loopPS;         // offset 0x3, size 0x1
  s16 loopY0;        // offset 0x4, size 0x2
  s16 loopY1;        // offset 0x6, size 0x2
  s16 coefTab[8][2]; // offset 0x8, size 0x20
} SNDADPCMinfo;

typedef struct DSPADPCMblock {
  // total size: 0x6
  signed short Y0;        // offset 0x0, size 0x2
  signed short Y1;        // offset 0x2, size 0x2
  unsigned char PS;       // offset 0x4, size 0x1
  unsigned char reserved; // offset 0x5, size 0x1
} DSPADPCMblock;

typedef struct DSPADPCMplusInfo {
  // total size: 0x2E
  unsigned short numCoef;     // offset 0x0, size 0x2
  unsigned char initialPS;    // offset 0x2, size 0x1
  unsigned char loopPS;       // offset 0x3, size 0x1
  signed short loopY0;        // offset 0x4, size 0x2
  signed short loopY1;        // offset 0x6, size 0x2
  signed short coefTab[8][2]; // offset 0x8, size 0x20
  DSPADPCMblock blk[1];       // offset 0x28, size 0x6
} DSPADPCMplusInfo;

typedef struct SAMPLE_INFO {
  // total size: 0x20
  u32 info;        // offset 0x0, size 0x4
  void* addr;      // offset 0x4, size 0x4
  void* extraData; // offset 0x8, size 0x4
  u32 offset;      // offset 0xC, size 0x4
  u32 length;      // offset 0x10, size 0x4
  u32 loop;        // offset 0x14, size 0x4
  u32 loopLength;  // offset 0x18, size 0x4
  u8 compType;     // offset 0x1C, size 0x1
} SAMPLE_INFO;

typedef struct ADSR_INFO {
  // total size: 0x14
  union ai_data {
    struct {
      // total size: 0x14
      s32 atime;  // offset 0x0, size 0x4
      s32 dtime;  // offset 0x4, size 0x4
      u16 slevel; // offset 0x8, size 0x2
      u16 rtime;  // offset 0xA, size 0x2
      s32 ascale; // offset 0xC, size 0x4
      s32 dscale; // offset 0x10, size 0x4
    } dls;
    struct {
      // total size: 0x8
      u16 atime;  // offset 0x0, size 0x2
      u16 dtime;  // offset 0x2, size 0x2
      u16 slevel; // offset 0x4, size 0x2
      u16 rtime;  // offset 0x6, size 0x2
    } linear;
  } data; // offset 0x0, size 0x14
} ADSR_INFO;

typedef struct ADSR_VARS {
  u8 mode;
  u8 state;
  u32 cnt;
  s32 currentVolume;
  s32 currentIndex;
  s32 currentDelta;

  union data {
    struct _dls {
      u32 aTime;
      u32 dTime;
      u16 sLevel;
      u32 rTime;
      u16 cutOff;
      u8 aMode;
    } dls;

    struct _linear {
      u32 aTime;
      u32 dTime;
      u16 sLevel;
      u32 rTime;
    } linear;
  } data;
} ADSR_VARS;

u32 adsrStartRelease(ADSR_VARS* adsr, u32 rtime);
u32 adsrHandle(ADSR_VARS* adsr, u16* adsr_start, u16* adsr_delta);
u32 adsrHandleLowPrecision(ADSR_VARS* adsr, u16* adsr_start, u16* adsr_delta);
bool adsrRelease(ADSR_VARS* adsr);
u32 adsrConvertTimeCents(s32 tc);
u32 adsrSetup(ADSR_VARS* adsr);

typedef struct _PBMIX {
  // total size: 0x24
  u16 vL;          // offset 0x0, size 0x2
  u16 vDeltaL;     // offset 0x2, size 0x2
  u16 vR;          // offset 0x4, size 0x2
  u16 vDeltaR;     // offset 0x6, size 0x2
  u16 vAuxAL;      // offset 0x8, size 0x2
  u16 vDeltaAuxAL; // offset 0xA, size 0x2
  u16 vAuxAR;      // offset 0xC, size 0x2
  u16 vDeltaAuxAR; // offset 0xE, size 0x2
  u16 vAuxBL;      // offset 0x10, size 0x2
  u16 vDeltaAuxBL; // offset 0x12, size 0x2
  u16 vAuxBR;      // offset 0x14, size 0x2
  u16 vDeltaAuxBR; // offset 0x16, size 0x2
  u16 vAuxBS;      // offset 0x18, size 0x2
  u16 vDeltaAuxBS; // offset 0x1A, size 0x2
  u16 vS;          // offset 0x1C, size 0x2
  u16 vDeltaS;     // offset 0x1E, size 0x2
  u16 vAuxAS;      // offset 0x20, size 0x2
  u16 vDeltaAuxAS; // offset 0x22, size 0x2
} _PBMIX;

typedef struct _PBITD {
  // total size: 0xE
  u16 flag;         // offset 0x0, size 0x2
  u16 bufferHi;     // offset 0x2, size 0x2
  u16 bufferLo;     // offset 0x4, size 0x2
  u16 shiftL;       // offset 0x6, size 0x2
  u16 shiftR;       // offset 0x8, size 0x2
  u16 targetShiftL; // offset 0xA, size 0x2
  u16 targetShiftR; // offset 0xC, size 0x2
} _PBITD;

typedef struct _PBUPDATE {
  // total size: 0xE
  u16 updNum[5]; // offset 0x0, size 0xA
  u16 dataHi;    // offset 0xA, size 0x2
  u16 dataLo;    // offset 0xC, size 0x2
} _PBUPDATE;

typedef struct _PBDPOP {
  // total size: 0x12
  u16 aL;     // offset 0x0, size 0x2
  u16 aAuxAL; // offset 0x2, size 0x2
  u16 aAuxBL; // offset 0x4, size 0x2
  u16 aR;     // offset 0x6, size 0x2
  u16 aAuxAR; // offset 0x8, size 0x2
  u16 aAuxBR; // offset 0xA, size 0x2
  u16 aS;     // offset 0xC, size 0x2
  u16 aAuxAS; // offset 0xE, size 0x2
  u16 aAuxBS; // offset 0x10, size 0x2
} _PBDPOP;

typedef struct _PBVE {
  // total size: 0x4
  u16 currentVolume; // offset 0x0, size 0x2
  u16 currentDelta;  // offset 0x2, size 0x2
} _PBVE;

typedef struct _PBFIR {
  // total size: 0x6
  u16 numCoefs; // offset 0x0, size 0x2
  u16 coefsHi;  // offset 0x2, size 0x2
  u16 coefsLo;  // offset 0x4, size 0x2
} _PBFIR;

typedef struct _PBADDR {
  // total size: 0x10
  u16 loopFlag;         // offset 0x0, size 0x2
  u16 format;           // offset 0x2, size 0x2
  u16 loopAddressHi;    // offset 0x4, size 0x2
  u16 loopAddressLo;    // offset 0x6, size 0x2
  u16 endAddressHi;     // offset 0x8, size 0x2
  u16 endAddressLo;     // offset 0xA, size 0x2
  u16 currentAddressHi; // offset 0xC, size 0x2
  u16 currentAddressLo; // offset 0xE, size 0x2
} _PBADDR;

typedef struct _PBADPCM {
  // total size: 0x28
  u16 a[8][2];    // offset 0x0, size 0x20
  u16 gain;       // offset 0x20, size 0x2
  u16 pred_scale; // offset 0x22, size 0x2
  u16 yn1;        // offset 0x24, size 0x2
  u16 yn2;        // offset 0x26, size 0x2
} _PBADPCM;

typedef struct _PBSRC {
  // total size: 0xE
  u16 ratioHi;            // offset 0x0, size 0x2
  u16 ratioLo;            // offset 0x2, size 0x2
  u16 currentAddressFrac; // offset 0x4, size 0x2
  u16 last_samples[4];    // offset 0x6, size 0x8
} _PBSRC;

typedef struct _PBADPCMLOOP {
  // total size: 0x6
  u16 loop_pred_scale; // offset 0x0, size 0x2
  u16 loop_yn1;        // offset 0x2, size 0x2
  u16 loop_yn2;        // offset 0x4, size 0x2
} _PBADPCMLOOP;

#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 1)
typedef struct _PBLPF {
  // total size: 0x8
  unsigned short flag; // offset 0x0, size 0x2
  unsigned short yn1;  // offset 0x2, size 0x2
  unsigned short a0;   // offset 0x4, size 0x2
  unsigned short b0;   // offset 0x6, size 0x2
} _PBLPF;
#endif

typedef struct _PB {
  // total size: 0xBC
  u16 nextHi;             // offset 0x0, size 0x2
  u16 nextLo;             // offset 0x2, size 0x2
  u16 currHi;             // offset 0x4, size 0x2
  u16 currLo;             // offset 0x6, size 0x2
  u16 srcSelect;          // offset 0x8, size 0x2
  u16 coefSelect;         // offset 0xA, size 0x2
  u16 mixerCtrl;          // offset 0xC, size 0x2
  u16 state;              // offset 0xE, size 0x2
  u16 loopType;           // offset 0x10, size 0x2
  _PBMIX mix;             // offset 0x12, size 0x24
  _PBITD itd;             // offset 0x36, size 0xE
  _PBUPDATE update;       // offset 0x44, size 0xE
  _PBDPOP dpop;              // offset 0x52, size 0x12
  _PBVE ve;                  // offset 0x64, size 0x4
  _PBFIR fir;                // offset 0x68, size 0x6
  _PBADDR addr;              // offset 0x6E, size 0x10
  _PBADPCM adpcm;            // offset 0x7E, size 0x28
  _PBSRC src;                // offset 0xA6, size 0xE
  _PBADPCMLOOP adpcmLoop;    // offset 0xB4, size 0x6
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 1)
  _PBLPF lpf; // offset 0xBA, size 0x8
#endif
  u16 streamLoopCnt; // offset 0xBA, size 0x2
} _PB;

typedef struct VSampleInfo {
  // total size: 0xC
  void* loopBufferAddr; // offset 0x0, size 0x4
  u32 loopBufferLength; // offset 0x4, size 0x4
  u8 inLoopBuffer;      // offset 0x8, size 0x1
} VSampleInfo;

typedef struct _SPB {
  // total size: 0x36
  u16 dpopLHi;     // offset 0x0, size 0x2
  u16 dpopLLo;     // offset 0x2, size 0x2
  u16 dpopLDelta;  // offset 0x4, size 0x2
  u16 dpopRHi;     // offset 0x6, size 0x2
  u16 dpopRLo;     // offset 0x8, size 0x2
  u16 dpopRDelta;  // offset 0xA, size 0x2
  u16 dpopSHi;     // offset 0xC, size 0x2
  u16 dpopSLo;     // offset 0xE, size 0x2
  u16 dpopSDelta;  // offset 0x10, size 0x2
  u16 dpopALHi;    // offset 0x12, size 0x2
  u16 dpopALLo;    // offset 0x14, size 0x2
  u16 dpopALDelta; // offset 0x16, size 0x2
  u16 dpopARHi;    // offset 0x18, size 0x2
  u16 dpopARLo;    // offset 0x1A, size 0x2
  u16 dpopARDelta; // offset 0x1C, size 0x2
  u16 dpopASHi;    // offset 0x1E, size 0x2
  u16 dpopASLo;    // offset 0x20, size 0x2
  u16 dpopASDelta; // offset 0x22, size 0x2
  u16 dpopBLHi;    // offset 0x24, size 0x2
  u16 dpopBLLo;    // offset 0x26, size 0x2
  u16 dpopBLDelta; // offset 0x28, size 0x2
  u16 dpopBRHi;    // offset 0x2A, size 0x2
  u16 dpopBRLo;    // offset 0x2C, size 0x2
  u16 dpopBRDelta; // offset 0x2E, size 0x2
  u16 dpopBSHi;    // offset 0x30, size 0x2
  u16 dpopBSLo;    // offset 0x32, size 0x2
  u16 dpopBSDelta; // offset 0x34, size 0x2
} _SPB;

typedef struct FILTERInfo {
  // total size: 0x6
  u8 on;     // offset 0x0, size 0x1
  u16 coefA; // offset 0x2, size 0x2
  u16 coefB; // offset 0x4, size 0x2
} FILTERInfo;

typedef struct DSPvoice {
  _PB* pb;
  void* patchData;
  void* itdBuffer;
  struct DSPvoice* next;
  struct DSPvoice* prev;
  struct DSPvoice* nextAlien;
  u32 mesgCallBackUserValue;
  u32 prio;
  u32 currentAddr;
  u32 changed[5];
  u32 pitch[5];
  u16 volL;
  u16 volR;
  u16 volS;
  u16 volLa;
  u16 volRa;
  u16 volSa;
  u16 volLb;
  u16 volRb;
  u16 volSb;
  u16 lastVolL;
  u16 lastVolR;
  u16 lastVolS;
  u16 lastVolLa;
  u16 lastVolRa;
  u16 lastVolSa;
  u16 lastVolLb;
  u16 lastVolRb;
  u16 lastVolSb;
  u16 smp_id;
  SAMPLE_INFO smp_info;
  VSampleInfo vSampleInfo;
  u8 streamLoopPS;
  ADSR_VARS adsr;
  u16 srcTypeSelect;
  u16 srcCoefSelect;
  u16 itdShiftL;
  u16 itdShiftR;
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 1)
  FILTERInfo filter;
#endif
  u8 singleOffset;
  struct {
    u32 posHi;
    u32 posLo;
    u32 pitch;
  } playInfo;
  struct {
    u8 pitch;
    u8 vol;
    u8 volA;
    u8 volB;
  } lastUpdate;
  u32 virtualSampleID;
  u8 state;
  u8 postBreak;
  u8 startupBreak;
  u8 studio;
  u32 flags;
} DSPvoice;

typedef struct DSPhostDPop {
  s32 l;
  s32 r;
  s32 s;
  s32 lA;
  s32 rA;
  s32 sA;
  s32 lB;
  s32 rB;
  s32 sB;
} DSPhostDPop;

typedef struct DSPinput {
  u8 studio;
  u16 vol;
  u16 volA;
  u16 volB;
  SND_STUDIO_INPUT* desc;
} DSPinput;

typedef struct DSPstudioinfo {
  _SPB* spb;
  DSPhostDPop hostDPopSum;
  s32* main[2];
  s32* auxA[3];
  s32* auxB[3];
  DSPvoice* voiceRoot;
  DSPvoice* alienVoiceRoot;
  u8 state;
  u8 isMaster;
  u8 numInputs;
  SND_STUDIO_TYPE type;
  DSPinput in[7];
  SND_AUX_CALLBACK auxAHandler;
  SND_AUX_CALLBACK auxBHandler;
  void* auxAUser;
  void* auxBUser;
} DSPstudioinfo;

typedef u32 (*SND_MESSAGE_CALLBACK)(u32, u32);

#define SAL_MAX_STUDIONUM 8
extern u8 salMaxStudioNum;
extern u8 salNumVoices;
extern u16* dspCmdList;
void* salMalloc(size_t len);
void* salMallocPhysical(size_t len);
void salFree(void* addr);
bool salInitDspCtrl(u8 numVoices, u8 numStudios, u32 defaultStudioDPL2);
void salInitHRTFBuffer();
bool salExitDspCtrl();
void salActivateStudio(u8 studio, u32 isMaster, SND_STUDIO_TYPE type);
void salDeactivateStudio(u8 studio);
void salActivateVoice(DSPvoice* dsp_vptr, u8 studio);
void salDeactivateVoice(DSPvoice* dsp_vptr);
void salReconnectVoice(DSPvoice* dsp_vptr, u8 studio);
bool salAddStudioInput(DSPstudioinfo* stp, SND_STUDIO_INPUT* desc);
bool salRemoveStudioInput(DSPstudioinfo* stp, SND_STUDIO_INPUT* desc);
u32 salSynthSendMessage(DSPvoice* dsp_vptr, u32 mesg);
void salHandleAuxProcessing();
void salBuildCommandList(s16* dest, u32 nsDelay);

unsigned long aramGetZeroBuffer();
void hwEnableCompressor();
void hwDisableCompressor();

#endif /* MUSYX_HW_DSPCTRL_201_COMPAT_H */
