/* Matrix operations for QR-based smoothing. */
#include "gssdk/mtx.h"

#include <string.h>

/* Applies the stored QR multipliers before solving the matrix system. */
enum MTXERRtAG QrPreMult(struct MTXTag *QR, struct MTXTag *Beta, struct MTXTag *Y)
{
    f32 *beta;
    f32 *xr;
    f32 *xScan;
    f32 *y;
    f32 *yScan;
    u32 i;
    u32 j;

    j = 0;
    y = Y->Entries;
    beta = Beta->Entries;
    xr = QR->Entries;

    while (j++ < QR->N) {
        f32 tmp;

        xr += j;
        tmp = *y++;
        xScan = xr;
        yScan = y;
        for (i = j; i < QR->M; i++) {
            tmp += *yScan++ * *xScan++;
        }

        tmp *= *beta++;
        y[-1] -= tmp;
        yScan = y;
        for (i = j; i < QR->M; i++) {
            *yScan++ -= tmp * *xr++;
        }
    }

    return NoError;
}

/* Copies the source entries into an allocated destination. */
void mtxFillCopy(struct MTXTag *PMTXDst, const struct MTXTag *PMTXSrc)
{
    memcpy(
        PMTXDst->Entries, PMTXSrc->Entries,
        PMTXDst->MxN * sizeof(f32));
}

/* Fills the matrix entries with one value. */
void mtxFillX(struct MTXTag *pMTX, f32 x)
{
    u32 aEnd = pMTX->MxN;
    f32 *a = pMTX->Entries;
    u32 i;

    for (i = 0; i < aEnd; i++) {
        *a++ = x;
    }
}
