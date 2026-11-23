/* State and entry points for smoothing a sequence of feature vectors. */
#ifndef GSSDK_SMOOTHER_H
#define GSSDK_SMOOTHER_H

#include "gssdk/mtx.h"

/* Spline, QR decomposition and beta coefficients for one sample rate. */
struct _SMOOTHERINFO {
    /* Spline coefficient table. */
    const f32 *aSpline;
    /* Number of rows in the spline table. */
    u16 SPLINEM;
    /* Number of columns in the spline table. */
    u16 SPLINEN;
    /* QR coefficients for the spline basis. */
    const f32 *aQRSpline;
    /* Number of rows in the QR table. */
    u16 QRSPLINEM;
    /* Number of columns in the QR table. */
    u16 QRSPLINEN;
    /* Beta coefficient table. */
    const f32 *aBeta;
    /* Number of rows in the beta table. */
    u16 BETAM;
    /* Number of columns in the beta table. */
    u16 BETAN;
};

/* Working matrices and compression threshold for the band-pass smoother. */
struct BandPassSmootherTag {
    /* Full spline basis before selecting columns. */
    struct MTXTag *BaseSplineMtx;
    /* QR factors for the full spline basis. */
    struct MTXTag *QrBaseSplineMtx;
    /* Working QR factors for the retained columns. */
    struct MTXTag *QrSplineMtx;
    /* Matrix of beta coefficients. */
    struct MTXTag *Beta;
    /* Spectrum being smoothed. */
    struct MTXTag *Spec;
    /* Fitted spline parameters. */
    struct MTXTag *Param;
    /* Indices of the columns retained in the fit. */
    struct INTMTXTag *RetainedCols;
    /* Compression parameter applied during smoothing. */
    f32 Compression;
};

/* Spline, QR and beta coefficients for 8 kHz audio. */
extern const struct _SMOOTHERINFO Smoother1600Info8kHz;
/* Spline, QR and beta coefficients for 11 kHz audio. */
extern const struct _SMOOTHERINFO Smoother1600Info11kHz;

/* Allocates a smoother for the supplied spline coefficients. */
struct BandPassSmootherTag *smtConstruct(struct _TOS *pTos, unsigned int nLogPower, f32 SegWdth,
                                        const struct _SMOOTHERINFO *pSmInfo, f32 Compression);

/* Releases the smoother and its matrices. */
void smtDestruct(struct BandPassSmootherTag *pSmoother, struct _TOS *pTos);

/* Fits the retained spline columns to the input vector. */
unsigned char Smoothing(struct BandPassSmootherTag *lpSmoother, float *Vector);

#endif
