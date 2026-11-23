/* Initial block, queue and profile configuration for the recognizer. */
#include "types.h"

#include "gssdk/blocks.h"

/* Frame window integer profile. */
static const struct _KEYVAL_U32 FrameWin_KeyVals[] = {
    { 0x02, 0x100 },
    { 0x00, 0 },
};

/* FFT magnitude integer profile. */
static const struct _KEYVAL_U32 FFTMod_KeyVals_U32[] = {
    { 0x06, 0x100 },
    { 0x04, 0x100 },
    { 0x05, 0x9C40 },
    { 0x00, 0 },
};

/* FFT magnitude floating-point profile. */
static const struct _KEYVAL_FLT FFTMod_KeyVals_FLT[] = {
    { 0x01, 400.0f },
    { 0x02, 4500.0f },
    { 0x05, 40000.0f },
    { 0x00, 0.0f },
};

/* Mel filter bank integer profile. */
static const struct _KEYVAL_U32 Mel_KeyVals_U32[] = {
    { 0x03, 0x81 },
    { 0x04, 0x15 },
    { 0x00, 0 },
};

/* Mel filter bank floating-point profile. */
static const struct _KEYVAL_FLT Mel_KeyVals_FLT[] = {
    { 0x02, 5500.0f },
    { 0x00, 0.0f },
};

/* Voice activity detection floating-point profile. */
static const struct _KEYVAL_FLT Vad_KeyVals_FLT[] = {
    { 0x24, 15.0f },
    { 0x00, 0.0f },
};

/* Lookahead integer profile. */
static const struct _KEYVAL_U32 LookAhead_KeyVals_U32[] = {
    { 0x01, 5 },
    { 0x00, 0 },
};

/* Spectral subtraction floating-point profile. */
static const struct _KEYVAL_FLT SpecSub_KeyVals_FLT[] = {
    { 0x02, 0.02f },
    { 0x07, 0.84f },
    { 0x03, 1.5f },
    { 0x00, 0.0f },
};

/* Spectral subtraction integer profile. */
static const struct _KEYVAL_U32 SpecSub_KeyVals_U32[] = {
    { 0x06, 3 },
    { 0x1E, 3 },
    { 0x1F, 10 },
    { 0x20, 16 },
    { 0x21, 21 },
    { 0x00, 0 },
};

/* Second voice activity detector floating-point profile. */
static const struct _KEYVAL_FLT Vad2_KeyVals_FLT[] = {
    { 0x24, 15.0f },
    { 0x2B, 26.525f },
    { 0x00, 0.0f },
};

/* Second voice activity detector integer profile. */
static const struct _KEYVAL_U32 Vad2_KeyVals_U32[] = {
    { 0x27, 1 },
    { 0x28, 0 },
    { 0x2A, 1 },
    { 0x25, 1 },
    { 0x0D, 2000 },
    { 0x2D, 3 },
    { 0x00, 0 },
};

/* Stationarity estimation floating-point profile. */
static const struct _KEYVAL_FLT Stationarity_KeyVals_FLT[] = {
    { 0x02, 2.0f },
    { 0x00, 0.0f },
};

/* Stationarity estimation integer profile. */
static const struct _KEYVAL_U32 Stationarity_KeyVals_U32[] = {
    { 0x01, 2 },
    { 0x00, 0 },
};

/* ACNE floating-point profile. */
static const struct _KEYVAL_FLT Acne_KeyVals_FLT[] = {
    { 0x03, 6.0f },
    { 0x04, -15.0f },
    { 0x05, 50.0f },
    { 0x00, 0.0f },
};

/* ACNE integer profile. */
static const struct _KEYVAL_U32 Acne_KeyVals_U32[] = {
    { 0x01, 0 },
    { 0x06, 15 },
    { 0x02, 50 },
    { 0x05, 50 },
    { 0x00, 0 },
};

/* DCT lifter integer profile. */
static const struct _KEYVAL_U32 DCTLift_KeyVals_U32[] = {
    { 0x01, 21 },
    { 0x04, 1 },
    { 0x02, 13 },
    { 0x00, 0 },
};

/* Gender classification integer profile. */
static const struct _KEYVAL_U32 Gender_KeyVals_U32[] = {
    { 0x06, 14 },
    { 0x05, 2 },
    { 0x01, 50 },
    { 0x00, 0 },
};

/* Undersampling integer profile. */
static const struct _KEYVAL_U32 Undersampler_KeyVals_U32[] = {
    { 0x01, 110 },
    { 0x02, 11000 },
    { 0x03, 2000 },
    { 0x00, 0 },
};

/* Undersampler delay integer profile. */
static const struct _KEYVAL_U32 UndersamplerDelayBlock_KeyVals_U32[] = {
    { 0x01, 6 },
    { 0x02, 88 },
    { 0x00, 0 },
};

/* Pitch window floating-point profile. */
static const struct _KEYVAL_FLT PitchWindow_KeyVals_FLT[] = {
    { 0x02, 0.04f },
    { 0x00, 0.0f },
};

/* Pitch window integer profile. */
static const struct _KEYVAL_U32 PitchWindow_KeyVals_U32[] = {
    { 0x01, 2200 },
    { 0x03, 110 },
    { 0x04, 11000 },
    { 0x00, 0 },
};

/* Pitch FFT integer profile. */
static const struct _KEYVAL_U32 PitchFFT_KeyVals_U32[] = {
    { 0x07, 0 },
    { 0x04, 128 },
    { 0x06, 88 },
    { 0x00, 0 },
};

/* Voiced and unvoiced detection integer profile. */
static const struct _KEYVAL_U32 SHS_VUV_KeyVals_U32[] = {
    { 0x02, 2200 },
    { 0x03, 65 },
    { 0x00, 0 },
};

/* Pitch dynamic programming floating-point profile. */
static const struct _KEYVAL_FLT PitchDP_KeyVals_FLT[] = {
    { 0x09, 0.01f },
    { 0x00, 0.0f },
};

/* Pitch dynamic programming integer profile. */
static const struct _KEYVAL_U32 PitchDP_KeyVals_U32[] = {
    { 0x02, 500 },
    { 0x03, 65 },
    { 0x04, 15 },
    { 0x00, 0 },
};

/* Gender filter for normalized Mel data integer profile. */
static const struct _KEYVAL_U32 MelNormGenderFilter_KeyVals_U32[] = {
    { 0x02, 88 },
    { 0x00, 0 },
};

/* Frame stacking integer profile. */
static const struct _KEYVAL_U32 Stacker_Keyvals_U32[] = {
    { 0x03, 2 },
    { 0x01, 152 },
    { 0x02, 20 },
    { 0x00, 0 },
};

/* Vector quantization integer profile. */
static const struct _KEYVAL_U32 Vq_KeyVals_U32[] = {
    { 0x02, 5 },
    { 0x01, 43 },
    { 0x00, 0 },
};

/* Distance calculation integer profile. */
static const struct _KEYVAL_U32 DistCalc_KeyVals_U32[] = {
    { 0x20, 5 },
    { 0x1E, 0 },
    { 0x1F, 0 },
    { 0x13, 32 },
    { 0x14, 0 },
    { 0x00, 0 },
};

/* N-best dynamic programming floating-point profile. */
static const struct _KEYVAL_FLT NBestDynProg_KeyVals_FLT[] = {
    { 0x0A, 0.2f },
    { 0x00, 0.0f },
};

/* N-best dynamic programming integer profile. */
static const struct _KEYVAL_U32 NBestDynProg_KeyVals_U32[] = {
    { 0x06, 60 },
    { 0x0B, 5000 },
    { 0x09, 300 },
    { 0x0E, 512 },
    { 0x07, 10 },
    { 0x0F, 5 },
    { 0x00, 0 },
};

/* User-word ID generation integer profile. */
static const struct _KEYVAL_U32 DpGenUwID_KeyVals_U32[] = {
    { 0x13, 1 },
    { 0x14, 120 },
    { 0x23, 900 },
    { 0x00, 0 },
};

/* User-word scoring integer profile. */
static const struct _KEYVAL_U32 DpScoreUw_KeyVals_U32[] = {
    { 0x13, 1 },
    { 0x00, 0 },
};

/* Isolated-word token search integer profile. */
static const struct _KEYVAL_U32 IsoWordTokenDp_KeyVals_U32[] = {
    { 0x0F, 5 },
    { 0x00, 0 },
};

/* Extra event search integer profile. */
static const struct _KEYVAL_U32 ExtraEventDp_KeyVals_U32[] = {
    { 0x0F, 5 },
    { 0x00, 0 },
};

/* Log-energy median integer profile. */
static const struct _KEYVAL_U32 LogEMedian_KeyVals_U32[] = {
    { 0x01, 9 },
    { 0x00, 0 },
};

/* Trigger integer profile. */
static const struct _KEYVAL_U32 Trigger_KeyVals_U32[] = {
    { 0x11, 30 },
    { 0x03, 10 },
    { 0x04, 45 },
    { 0x0E, 100 },
    { 0x06, 10 },
    { 0x00, 0 },
};

/* Trigger floating-point profile. */
static const struct _KEYVAL_FLT Trigger_KeyVals_FLT[] = {
    { 0x09, 3.0f },
    { 0x08, 1.0f },
    { 0x0C, 10.5f },
    { 0x0D, 31.0f },
    { 0x00, 0.0f },
};

/* Spectral subtraction Mel data queue integer profile. */
static const struct _KEYVAL_U32 MelSpecSubQueue_KeyVals[] = {
    { 0x04, 45 },
    { 0x00, 0 },
};

/* Noise spectrum queue integer profile. */
static const struct _KEYVAL_U32 NoiseSpecQueue_KeyVals[] = {
    { 0x04, 45 },
    { 0x00, 0 },
};

/* Silence detection queue integer profile. */
static const struct _KEYVAL_U32 SilDetectQueue_KeyVals[] = {
    { 0x04, 45 },
    { 0x00, 0 },
};

/* Second voice activity detector queue integer profile. */
static const struct _KEYVAL_U32 Vad2Queue_KeyVals[] = {
    { 0x04, 45 },
    { 0x00, 0 },
};

/* Signal levels queue integer profile. */
static const struct _KEYVAL_U32 LevelsQueue_KeyVals[] = {
    { 0x04, 45 },
    { 0x00, 0 },
};

/* Label queue integer profile. */
static const struct _KEYVAL_U32 LabelQueue_KeyVals[] = {
    { 0x02, 5 },
    { 0x00, 0 },
};

/* Undersampler delay queue integer profile. */
static const struct _KEYVAL_U32 UndersamplerDelayBlockQueue_KeyVals[] = {
    { 0x04, 45 },
    { 0x00, 0 },
};

/* Profile entries indexed by block and queue ID. */
const struct _KEYVALS IniFile[87] = {
    { NULL, NULL },                                         /* 0x00 */
    { NULL, FrameWin_KeyVals },                             /* 0x01 */
    { FFTMod_KeyVals_FLT, FFTMod_KeyVals_U32 },             /* 0x02 */
    { Mel_KeyVals_FLT, Mel_KeyVals_U32 },                   /* 0x03 */
    { Mel_KeyVals_FLT, Mel_KeyVals_U32 },                   /* 0x04 */
    { Mel_KeyVals_FLT, Mel_KeyVals_U32 },                   /* 0x05 */
    { Vad_KeyVals_FLT, NULL },                              /* 0x06 */
    { NULL, LookAhead_KeyVals_U32 },                        /* 0x07 */
    { SpecSub_KeyVals_FLT, SpecSub_KeyVals_U32 },           /* 0x08 */
    { SpecSub_KeyVals_FLT, SpecSub_KeyVals_U32 },           /* 0x09 */
    { SpecSub_KeyVals_FLT, SpecSub_KeyVals_U32 },           /* 0x0A */
    { Vad2_KeyVals_FLT, Vad2_KeyVals_U32 },                 /* 0x0B */
    { Stationarity_KeyVals_FLT, Stationarity_KeyVals_U32 }, /* 0x0C */
    { Acne_KeyVals_FLT, Acne_KeyVals_U32 },                 /* 0x0D */
    { Acne_KeyVals_FLT, Acne_KeyVals_U32 },                 /* 0x0E */
    { Acne_KeyVals_FLT, Acne_KeyVals_U32 },                 /* 0x0F */
    { NULL, DCTLift_KeyVals_U32 },                          /* 0x10 */
    { NULL, DCTLift_KeyVals_U32 },                          /* 0x11 */
    { NULL, DCTLift_KeyVals_U32 },                          /* 0x12 */
    { NULL, Gender_KeyVals_U32 },                           /* 0x13 */
    { NULL, NULL },                                         /* 0x14 */
    { NULL, Undersampler_KeyVals_U32 },                     /* 0x15 */
    { NULL, UndersamplerDelayBlock_KeyVals_U32 },           /* 0x16 */
    { PitchWindow_KeyVals_FLT, PitchWindow_KeyVals_U32 },   /* 0x17 */
    { NULL, PitchFFT_KeyVals_U32 },                         /* 0x18 */
    { NULL, SHS_VUV_KeyVals_U32 },                          /* 0x19 */
    { PitchDP_KeyVals_FLT, PitchDP_KeyVals_U32 },           /* 0x1A */
    { NULL, MelNormGenderFilter_KeyVals_U32 },              /* 0x1B */
    { NULL, NULL },                                         /* 0x1C */
    { NULL, Stacker_Keyvals_U32 },                          /* 0x1D */
    { NULL, Vq_KeyVals_U32 },                               /* 0x1E */
    { NULL, DistCalc_KeyVals_U32 },                         /* 0x1F */
    { NBestDynProg_KeyVals_FLT, NBestDynProg_KeyVals_U32 }, /* 0x20 */
    { NULL, DpGenUwID_KeyVals_U32 },                        /* 0x21 */
    { NULL, DpScoreUw_KeyVals_U32 },                        /* 0x22 */
    { NULL, IsoWordTokenDp_KeyVals_U32 },                   /* 0x23 */
    { NULL, ExtraEventDp_KeyVals_U32 },                     /* 0x24 */
    { NULL, NULL },                                         /* 0x25 */
    { NULL, LogEMedian_KeyVals_U32 },                       /* 0x26 */
    { Trigger_KeyVals_FLT, Trigger_KeyVals_U32 },           /* 0x27 */
    { NULL, NULL },                                         /* 0x28 */
    { NULL, NULL },                                         /* 0x29 */
    { NULL, NULL },                                         /* 0x2A */
    { NULL, NULL },                                         /* 0x2B */
    { NULL, NULL },                                         /* 0x2C */
    { NULL, NULL },                                         /* 0x2D */
    { NULL, NULL },                                         /* 0x2E */
    { NULL, NULL },                                         /* 0x2F */
    { NULL, SilDetectQueue_KeyVals },                       /* 0x30 */
    { NULL, NULL },                                         /* 0x31 */
    { NULL, NULL },                                         /* 0x32 */
    { NULL, NULL },                                         /* 0x33 */
    { NULL, MelSpecSubQueue_KeyVals },                      /* 0x34 */
    { NULL, MelSpecSubQueue_KeyVals },                      /* 0x35 */
    { NULL, MelSpecSubQueue_KeyVals },                      /* 0x36 */
    { NULL, NoiseSpecQueue_KeyVals },                       /* 0x37 */
    { NULL, NoiseSpecQueue_KeyVals },                       /* 0x38 */
    { NULL, NoiseSpecQueue_KeyVals },                       /* 0x39 */
    { NULL, Vad2Queue_KeyVals },                            /* 0x3A */
    { NULL, LevelsQueue_KeyVals },                          /* 0x3B */
    { NULL, NULL },                                         /* 0x3C */
    { NULL, NULL },                                         /* 0x3D */
    { NULL, NULL },                                         /* 0x3E */
    { NULL, NULL },                                         /* 0x3F */
    { NULL, NULL },                                         /* 0x40 */
    { NULL, NULL },                                         /* 0x41 */
    { NULL, NULL },                                         /* 0x42 */
    { NULL, NULL },                                         /* 0x43 */
    { NULL, NULL },                                         /* 0x44 */
    { NULL, LabelQueue_KeyVals },                           /* 0x45 */
    { NULL, NULL },                                         /* 0x46 */
    { NULL, NULL },                                         /* 0x47 */
    { NULL, NULL },                                         /* 0x48 */
    { NULL, UndersamplerDelayBlockQueue_KeyVals },          /* 0x49 */
    { NULL, NULL },                                         /* 0x4A */
    { NULL, NULL },                                         /* 0x4B */
    { NULL, NULL },                                         /* 0x4C */
    { NULL, NULL },                                         /* 0x4D */
    { NULL, NULL },                                         /* 0x4E */
    { NULL, NULL },                                         /* 0x4F */
    { NULL, NULL },                                         /* 0x50 */
    { NULL, NULL },                                         /* 0x51 */
    { NULL, NULL },                                         /* 0x52 */
    { NULL, NULL },                                         /* 0x53 */
    { NULL, NULL },                                         /* 0x54 */
    { NULL, NULL },                                         /* 0x55 */
    { NULL, NULL },                                         /* 0x56 */
};

/* Processing block IDs, startup enable flags and constructors. */
const struct _PROCDESCRIP InitialProcs[] = {
    { 0x00, 1, NULL },
    { 0x01, 1, ConstructWindow },
    { 0x02, 1, ConstructFFTMod },
    { 0x03, 0, ConstructMel },
    { 0x04, 0, ConstructMel },
    { 0x05, 0, ConstructMel },
    { 0x06, 1, ConstructVad },
    { 0x07, 1, ConstructLookAhead },
    { 0x08, 0, ConstructSpecSub },
    { 0x09, 0, ConstructSpecSub },
    { 0x0A, 0, ConstructSpecSub },
    { 0x0B, 1, ConstructVad },
    { 0x0C, 1, ConstructStationarity },
    { 0x0D, 0, ConstructAcne },
    { 0x0E, 0, ConstructAcne },
    { 0x0F, 0, ConstructAcne },
    { 0x10, 0, ConstructDCTLift },
    { 0x11, 0, ConstructDCTLift },
    { 0x12, 0, ConstructDCTLift },
    { 0x13, 1, ConstructGender },
    { 0x14, 1, ConstructCombiner },
    { 0x15, 0, ConstructUndersampler },
    { 0x16, 0, ConstructDelayBlock },
    { 0x17, 0, ConstructPitchWindow },
    { 0x18, 0, ConstructFFTMod },
    { 0x19, 0, ConstructSHS_VUV },
    { 0x1A, 0, ConstructPitchDP },
    { 0x1B, 0, ConstructGenderFilter },
    { 0x1C, 0, ConstructPitchCombiner },
    { 0x1D, 1, ConstructStacker },
    { 0x1E, 1, ConstructVq },
    { 0x1F, 1, ConstructDistCalc },
    { 0x20, 1, ConstructNBestDynProg },
    { 0x21, 0, ConstructDpGenUw },
    { 0x22, 0, ConstructDpScoreUw },
    { 0x23, 1, ConstructIsoWordTokenDp },
    { 0x24, 1, ConstructExtraEventDp },
    { 0x25, 1, ConstructGenderFilter },
    { 0x26, 1, ConstructMedian },
    { 0x27, 1, ConstructTriggerLR },
    { 0xFF, 1, NULL },
};

/* Queue IDs, startup enable flags and source blocks. */
const struct _QDESCRIP QueueWriters[] = {
    { 0x28, 1, 0x00 },
    { 0x29, 1, 0x01 },
    { 0x2A, 1, 0x02 },
    { 0x2B, 1, 0x02 },
    { 0x2C, 0, 0x03 },
    { 0x2D, 0, 0x04 },
    { 0x2E, 0, 0x05 },
    { 0x2F, 1, 0x06 },
    { 0x30, 1, 0x07 },
    { 0x30, 1, 0x00 },
    { 0x31, 0, 0x08 },
    { 0x32, 0, 0x09 },
    { 0x33, 0, 0x0A },
    { 0x34, 0, 0x08 },
    { 0x35, 0, 0x09 },
    { 0x36, 0, 0x0A },
    { 0x37, 0, 0x08 },
    { 0x38, 0, 0x09 },
    { 0x39, 0, 0x0A },
    { 0x3A, 1, 0x0B },
    { 0x3B, 1, 0x0B },
    { 0x3C, 1, 0x0C },
    { 0x3D, 0, 0x0D },
    { 0x3E, 0, 0x0E },
    { 0x3F, 0, 0x0F },
    { 0x40, 1, 0x10 },
    { 0x41, 1, 0x11 },
    { 0x42, 1, 0x12 },
    { 0x43, 1, 0x13 },
    { 0x43, 1, 0x00 },
    { 0x44, 1, 0x14 },
    { 0x45, 1, 0x1E },
    { 0x45, 1, 0x00 },
    { 0x46, 0, 0x1F },
    { 0x47, 0, 0x21 },
    { 0x47, 0, 0x22 },
    { 0x48, 0, 0x15 },
    { 0x49, 0, 0x16 },
    { 0x4A, 0, 0x17 },
    { 0x4B, 0, 0x18 },
    { 0x4C, 0, 0x19 },
    { 0x4D, 0, 0x1A },
    { 0x4E, 0, 0x19 },
    { 0x4F, 0, 0x1C },
    { 0x50, 1, 0x1D },
    { 0x51, 0, 0x08 },
    { 0x52, 0, 0x09 },
    { 0x53, 0, 0x0A },
    { 0x54, 1, 0x25 },
    { 0x55, 1, 0x26 },
    { 0x56, 0, 0x1B },
    { 0xFF, 1, 0xFF },
};

/* Input blocks registered to read from each queue. */
const struct _QTARGETDESCRIP QueueReaders[] = {
    { 0x28, 0x01 },
    { 0x28, 0x15 },
    { 0x28, 0x27 },
    { 0x29, 0x02 },
    { 0x2A, 0x06 },
    { 0x2B, 0x03 },
    { 0x2B, 0x04 },
    { 0x2B, 0x05 },
    { 0x2C, 0x08 },
    { 0x2D, 0x09 },
    { 0x2E, 0x0A },
    { 0x2F, 0x07 },
    { 0x30, 0x08 },
    { 0x30, 0x09 },
    { 0x30, 0x0A },
    { 0x30, 0x1F },
    { 0x30, 0x20 },
    { 0x30, 0x23 },
    { 0x31, 0x0B },
    { 0x32, 0x0B },
    { 0x33, 0x0B },
    { 0x34, 0x0D },
    { 0x35, 0x0E },
    { 0x36, 0x0F },
    { 0x37, 0x0D },
    { 0x38, 0x0E },
    { 0x39, 0x0F },
    { 0x3B, 0x0C },
    { 0x3A, 0x0C },
    { 0x3C, 0x0D },
    { 0x3C, 0x0E },
    { 0x3C, 0x0F },
    { 0x3B, 0x0D },
    { 0x3B, 0x0E },
    { 0x3B, 0x0F },
    { 0x3B, 0x19 },
    { 0x3B, 0x27 },
    { 0x3B, 0x26 },
    { 0x3D, 0x10 },
    { 0x3D, 0x1B },
    { 0x3E, 0x11 },
    { 0x3E, 0x1B },
    { 0x3F, 0x12 },
    { 0x3F, 0x1B },
    { 0x3C, 0x13 },
    { 0x40, 0x13 },
    { 0x41, 0x13 },
    { 0x42, 0x13 },
    { 0x43, 0x14 },
    { 0x44, 0x1D },
    { 0x48, 0x16 },
    { 0x49, 0x17 },
    { 0x4A, 0x18 },
    { 0x4B, 0x19 },
    { 0x4C, 0x1A },
    { 0x4D, 0x1C },
    { 0x4E, 0x1C },
    { 0x4F, 0x1D },
    { 0x50, 0x1E },
    { 0x45, 0x20 },
    { 0x45, 0x1F },
    { 0x45, 0x00 },
    { 0x45, 0x23 },
    { 0x45, 0x24 },
    { 0x46, 0x21 },
    { 0x46, 0x22 },
    { 0x47, 0x1F },
    { 0x51, 0x25 },
    { 0x52, 0x25 },
    { 0x53, 0x25 },
    { 0x54, 0x27 },
    { 0x55, 0x27 },
    { 0x56, 0x1C },
    { 0xFF, 0xFF },
};
