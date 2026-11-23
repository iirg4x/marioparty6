/* Matrix storage and operations for feature processing. */
#ifndef GSSDK_MTX_H
#define GSSDK_MTX_H

#include "gssdk/tos.h"

/* Dense floating-point matrix with column-major storage. */
struct MTXTag {
    /* Number of rows. */
    u32 M;
    /* Number of columns. */
    u32 N;
    /* Total number of entries. */
    u32 MxN;
    /* Start of the column-major element array. */
    f32 *Entries;
    /* Address just past the last entry. */
    f32 *EndEntries;
    u8 Owner; /* Whether the matrix owns Entries. */
};

/* Dense integer matrix with column-major storage. */
struct INTMTXTag {
    /* Number of rows. */
    u32 M;
    /* Number of columns. */
    u32 N;
    /* Total number of entries. */
    u32 MxN;
    /* Start of the column-major element array. */
    int *Entries;
    /* Address just past the last entry. */
    int *EndEntries;
    u8 Owner; /* Whether the matrix owns Entries. */
};

/* Allocates a floating-point matrix record. */
struct MTXTag *mtxConstruct(struct _TOS *pTos);

/* Releases the floating-point matrix and any storage it owns. */
void mtxDestruct(struct MTXTag *pMtx, struct _TOS *pTos);

/* Allocates an integer matrix record. */
struct INTMTXTag *imtxConstruct(struct _TOS *pTos);

/* Releases the integer matrix and any storage it owns. */
void imtxDestruct(struct INTMTXTag *pMtx, struct _TOS *pTos);

/* Initializes a floating-point matrix with the given dimensions. */
struct MTXTag *mtxInit(
    struct MTXTag *pMtx, unsigned int M, unsigned int N, struct _TOS *pTos);

/* Initializes an integer matrix with the given dimensions. */
struct INTMTXTag *imtxInit(
    struct INTMTXTag *pMtx, unsigned int M, unsigned int N, struct _TOS *pTos);

/* Initializes a floating-point matrix from an existing matrix. */
struct MTXTag *mtxInitCopy(
    struct MTXTag *pMtxDst, const struct MTXTag *pMtxSrc, struct _TOS *pTos);

/* Results returned by matrix operations. */
enum MTXERRtAG {
    NoError = 0, /* The requested operation completed. */
    IOerror = 1, /* The matrix could not be read or written. */
    WrongMtxSize = 2, /* The matrix dimensions do not satisfy the operation. */
    Singular = 3, /* The matrix is singular. */
};

/* Removes a column from an integer matrix. */
void imtxDeleteCol(struct INTMTXTag *pMtx, unsigned int Col);

/* Associates a matrix with an existing array of entries. */
void mtxAssociateAll(
    struct MTXTag *pMtx, unsigned int m, unsigned int n, const f32 *x);

/* Returns the greatest entry in the matrix. */
f32 mtxMax(const struct MTXTag *pMtx);

/* Compresses entries above the supplied level. */
void mtxCompress(struct MTXTag *pMtx, f32 a);

/* Removes a column and updates the QR factors. */
enum MTXERRtAG QrDeleteCol(struct MTXTag *R, struct MTXTag *y, unsigned int k);

/* Applies the stored QR transformations to a vector. */
enum MTXERRtAG QrPreMult(
    struct MTXTag *QR, struct MTXTag *Beta, struct MTXTag *Y);

/* Copies entries into the destination matrix. */
void mtxFillCopy(struct MTXTag *PMTXDst, const struct MTXTag *PMTXSrc);

/* Sets all matrix entries to the supplied value. */
void mtxFillX(struct MTXTag *pMTX, f32 x);

#endif
