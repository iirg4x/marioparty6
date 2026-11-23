/* Constructors for the voice SDK processing blocks. */
#ifndef GSSDK_BLOCKS_H
#define GSSDK_BLOCKS_H

#include "gssdk/tos.h"

/* Selects all distributions or only the active distributions. */
enum _ACTDISTSTATE {
    ACTDISTSTATE_MakeAllDist = 0,
    ACTDISTSTATE_MakeActiveDist = 1
};

/* Creates a block that applies the analysis window to input frames. */
struct _BLOCK *ConstructWindow(struct _TOS *pTos, u8 BlockID);

/* Creates a block that computes Fourier transforms of the input frames. */
struct _BLOCK *ConstructFFTMod(struct _TOS *pTos, u8 BlockID);

/* Creates a block that computes mel-scale spectral coefficients. */
struct _BLOCK *ConstructMel(struct _TOS *pTos, u8 BlockID);

/* Creates a block that detects voice activity in the input stream. */
struct _BLOCK *ConstructVad(struct _TOS *pTos, u8 BlockID);

/* Creates a block that delays features for look-ahead analysis. */
struct _BLOCK *ConstructLookAhead(struct _TOS *pTos, u8 BlockID);

/* Creates a block that suppresses noise with spectral subtraction. */
struct _BLOCK *ConstructSpecSub(struct _TOS *pTos, u8 BlockID);

/* Creates a block that tracks the stationarity of incoming features. */
struct _BLOCK *ConstructStationarity(struct _TOS *pTos, u8 BlockID);

/* Creates a block that normalizes acoustic features. */
struct _BLOCK *ConstructAcne(struct _TOS *pTos, u8 BlockID);

/* Creates a block that computes cosine coefficients and applies liftering. */
struct _BLOCK *ConstructDCTLift(struct _TOS *pTos, u8 BlockID);

/* Creates a block that scores features against the configured gender models. */
struct _BLOCK *ConstructGender(struct _TOS *pTos, u8 BlockID);

/* Creates a block that combines selected features from its input vector. */
struct _BLOCK *ConstructCombiner(struct _TOS *pTos, u8 BlockID);

/* Creates a block that reduces the input sample rate. */
struct _BLOCK *ConstructUndersampler(struct _TOS *pTos, u8 BlockID);

/* Creates a block that delays one stream by the configured number of frames. */
struct _BLOCK *ConstructDelayBlock(struct _TOS *pTos, u8 BlockID);

/* Creates a block that prepares input windows for pitch analysis. */
struct _BLOCK *ConstructPitchWindow(struct _TOS *pTos, u8 BlockID);

/* Creates a block that estimates pitch and detects voicing. */
struct _BLOCK *ConstructSHS_VUV(struct _TOS *pTos, u8 BlockID);

/* Creates a block that tracks pitch candidates across frames. */
struct _BLOCK *ConstructPitchDP(struct _TOS *pTos, u8 BlockID);

/* Creates a block that selects the stream corresponding to the chosen gender. */
struct _BLOCK *ConstructGenderFilter(struct _TOS *pTos, u8 BlockID);

/* Creates a block that combines pitch features with the input vectors. */
struct _BLOCK *ConstructPitchCombiner(struct _TOS *pTos, u8 BlockID);

/* Creates a block that groups consecutive input frames into one output vector. */
struct _BLOCK *ConstructStacker(struct _TOS *pTos, u8 BlockID);

/* Creates a block that quantizes features against the language codebook. */
struct _BLOCK *ConstructVq(struct _TOS *pTos, u8 BlockID);

/* Creates a block that computes acoustic distances for recognition. */
struct _BLOCK *ConstructDistCalc(struct _TOS *pTos, u8 BlockID);

/* Creates a block that tracks and ranks candidate recognition paths. */
struct _BLOCK *ConstructNBestDynProg(struct _TOS *pTos, u8 BlockID);

/* Creates a block that generates user-word paths from distribution scores. */
struct _BLOCK *ConstructDpGenUw(struct _TOS *pTos, u8 BlockID);

/* Creates a block that scores the paths for a user word. */
struct _BLOCK *ConstructDpScoreUw(struct _TOS *pTos, u8 BlockID);

/* Creates a block that tracks isolated-word recognition paths. */
struct _BLOCK *ConstructIsoWordTokenDp(struct _TOS *pTos, u8 BlockID);

/* Creates a block that scores extra recognition events. */
struct _BLOCK *ConstructExtraEventDp(struct _TOS *pTos, u8 BlockID);

/* Creates a block that computes a sliding median of the input values. */
struct _BLOCK *ConstructMedian(struct _TOS *pTos, u8 BlockID);

/* Creates a block that detects triggers using the likelihood-ratio and voicing measures. */
struct _BLOCK *ConstructTriggerLR(struct _TOS *pTos, u8 BlockID);

#endif
