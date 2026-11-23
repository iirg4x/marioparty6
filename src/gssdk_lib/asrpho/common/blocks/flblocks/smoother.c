/* Fits a reduced spline basis to smooth a feature vector. */

#include "gssdk/heap.h"
#include "gssdk/smoother.h"

/* Allocates the smoother and associates its matrices with the supplied spline coefficients. */
struct BandPassSmootherTag *smtConstruct(struct _TOS *pTos, unsigned int nLogPower, f32 SegWdth,
                                        const struct _SMOOTHERINFO *pSmInfo, f32 Compression)
{
    struct BandPassSmootherTag *lpSmoother;
    u32 NbSegs;

    lpSmoother = heap_Calloc(pTos->hHeap, 1, sizeof(struct BandPassSmootherTag));
    if (lpSmoother == NULL) {
        return NULL;
    }

    /* These failure branches release the smoother record alone. */
    lpSmoother->BaseSplineMtx = mtxConstruct(pTos);
    if (lpSmoother->BaseSplineMtx == NULL) {
        heap_Free(pTos->hHeap, lpSmoother);
        return NULL;
    }
    mtxAssociateAll(lpSmoother->BaseSplineMtx, pSmInfo->SPLINEM, pSmInfo->SPLINEN,
                    pSmInfo->aSpline);

    lpSmoother->QrBaseSplineMtx = mtxConstruct(pTos);
    if (lpSmoother->QrBaseSplineMtx == NULL) {
        heap_Free(pTos->hHeap, lpSmoother);
        return NULL;
    }
    mtxAssociateAll(lpSmoother->QrBaseSplineMtx, pSmInfo->QRSPLINEM, pSmInfo->QRSPLINEN,
                    pSmInfo->aQRSpline);

    lpSmoother->Beta = mtxConstruct(pTos);
    if (lpSmoother->Beta == NULL) {
        heap_Free(pTos->hHeap, lpSmoother);
        return NULL;
    }
    mtxAssociateAll(lpSmoother->Beta, pSmInfo->BETAM, pSmInfo->BETAN, pSmInfo->aBeta);

    NbSegs = (u32) (0.5 + (f32) (nLogPower - 1) / SegWdth);

    lpSmoother->Spec = mtxConstruct(pTos);
    if (lpSmoother->Spec == NULL) {
        heap_Free(pTos->hHeap, lpSmoother);
        return NULL;
    }
    mtxAssociateAll(lpSmoother->Spec, nLogPower, 1, NULL);

    lpSmoother->Param = mtxConstruct(pTos);
    if (lpSmoother->Param == NULL) {
        heap_Free(pTos->hHeap, lpSmoother);
        return NULL;
    }
    if (mtxInit(lpSmoother->Param, NbSegs + 2, 1, pTos) == NULL) {
        heap_Free(pTos->hHeap, lpSmoother);
        return NULL;
    }

    lpSmoother->RetainedCols = imtxConstruct(pTos);
    if (lpSmoother->RetainedCols == NULL) {
        heap_Free(pTos->hHeap, lpSmoother);
        return NULL;
    }
    if (imtxInit(lpSmoother->RetainedCols, 1, NbSegs + 2, pTos) == NULL) {
        heap_Free(pTos->hHeap, lpSmoother);
        return NULL;
    }

    lpSmoother->QrSplineMtx = mtxConstruct(pTos);
    if (lpSmoother->QrSplineMtx == NULL) {
        heap_Free(pTos->hHeap, lpSmoother);
        return NULL;
    }
    if (mtxInitCopy(lpSmoother->QrSplineMtx, lpSmoother->QrBaseSplineMtx, pTos) == NULL) {
        heap_Free(pTos->hHeap, lpSmoother);
        return NULL;
    }

    lpSmoother->Compression = Compression;
    return lpSmoother;
}

/* Releases the smoother's spline matrices and working memory. */
void smtDestruct(struct BandPassSmootherTag *pSmoother, struct _TOS *pTos)
{
    mtxDestruct(pSmoother->BaseSplineMtx, pTos);
    mtxDestruct(pSmoother->QrBaseSplineMtx, pTos);
    mtxDestruct(pSmoother->Beta, pTos);
    mtxDestruct(pSmoother->Spec, pTos);
    mtxDestruct(pSmoother->Param, pTos);
    imtxDestruct(pSmoother->RetainedCols, pTos);
    mtxDestruct(pSmoother->QrSplineMtx, pTos);
    heap_Free(pTos->hHeap, pSmoother);
}

/* Fits spline coefficients and replaces the input vector with the smoothed values. */
unsigned char Smoothing(struct BandPassSmootherTag *lpSmoother, float *Vector)
{
    unsigned int k;
    int *Col;
    float *p;
    float *pEnd;
    float Max;
    unsigned int TotalCols;
    struct MTXTag *BaseSplineMtx;
    struct MTXTag *QrBaseSplineMtx;
    struct MTXTag *QrSplineMtx;
    struct MTXTag *Beta;
    struct MTXTag *Spec;
    struct MTXTag *Param;
    struct INTMTXTag *RetainedCols;
    struct MTXTag *QR;
    struct MTXTag *y;
    struct MTXTag *x;
    float *px;
    float *py;
    float *xScan;
    float *pr;
    float *qrEnd;
    float rkk;
    float *tmp;
    float *c;
    unsigned int i;

    BaseSplineMtx = lpSmoother->BaseSplineMtx;
    QrBaseSplineMtx = lpSmoother->QrBaseSplineMtx;
    QrSplineMtx = lpSmoother->QrSplineMtx;
    Beta = lpSmoother->Beta;
    Spec = lpSmoother->Spec;
    Param = lpSmoother->Param;
    RetainedCols = lpSmoother->RetainedCols;
    mtxFillCopy(QrSplineMtx, QrBaseSplineMtx);
    Spec->Entries = Vector;
    Spec->EndEntries = Vector + Spec->MxN;
    Max = mtxMax(Spec);
    mtxCompress(Spec, Max - lpSmoother->Compression);
    if (QrPreMult(QrSplineMtx, Beta, Spec) != NoError) {
        return 1;
    }

    for (k = RetainedCols->N; k != 0;) {
        k--;
        RetainedCols->Entries[k] = k;
    }

    k = 3;
    while (k > 2) {
        QR = QrSplineMtx;
        y = Spec;
        x = Param;
        px = x->Entries + x->M;
        py = y->Entries + y->M;
        qrEnd = QR->Entries + QR->M * QR->N;
        py -= y->M - QR->N;
        for (k = QR->N; k != 0; k--) {
            pr = QR->Entries + (k - 1) * QR->M + k;
            rkk = *--pr;
            px--;
            py--;
            *px = *py;
            xScan = px;
            pr += QR->M;
            while (pr < qrEnd) {
                *px -= *pr * *++xScan;
                pr += QR->M;
            }
            *px /= rkk;
            if (*px > 0.0f && k > 2) {
                break;
            }
        }
        if (k != 0) {
            if (QrDeleteCol(QrSplineMtx, Spec, k - 1) != NoError) {
                return 1;
            }
            Param->M--;
            imtxDeleteCol(RetainedCols, k - 1);
        }
    }

    mtxFillX(Spec, 0.0f);
    Col = RetainedCols->Entries;
    c = Param->Entries;
    pEnd = c + Param->M;
    while (c < pEnd) {
        tmp = BaseSplineMtx->Entries + BaseSplineMtx->M * *Col++;
        p = Spec->Entries;
        for (i = 0; i < Spec->MxN; i++) {
            *p++ += *tmp++ * *c;
        }
        c++;
    }
    TotalCols = BaseSplineMtx->N;
    QrSplineMtx->N = TotalCols;
    Param->M = TotalCols;
    RetainedCols->N = TotalCols;
    return 0;
}
