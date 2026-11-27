/* Acoustic distances and online adaptation of the silence model. */
#include "types.h"

#include <PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/math.h>

#include "gssdk/blocks.h"
#include "gssdk/engine.h"
#include "gssdk/heap.h"
#include "gssdk/mathusage.h"
#include "gssdk/langdata.h"
#include "gssdk/tos.h"
#include "gssdk/mqueue.h"

#include "string.h"

/* Adaptation batches accumulated before normalizing the silence probabilities. */
#define SILENCE_NORMALIZATION_BATCHES 0x14

struct _WARPINFO;

typedef struct DistanceMatrix DistanceMatrix;

/* A gastone matrix and its dimensions. */
struct WMATRIX_ {
    unsigned short *Matrix; /* Gastone IDs indexed by state and codebook. */
    unsigned short Rows; /* Number of states in the matrix. */
    unsigned short Columns; /* Number of codebooks per state. */
};

typedef struct DISTSIL DISTSIL;

/* Storage for the saved acoustic model state. */
struct SAVEDMODELSTATE_ {
    DistanceMatrix *SavedActiveMatrices; /* The game's code never reads this field. */
    u16 SavedGarbPhenom; /* The game's code never reads this field. */
    u16 *pSavedPhenToStat; /* The game's code never reads this field. */
    u16 *lpSavedSilPhenoms; /* The game's code never reads this field. */
    u16 SavedNbrPhenoms; /* The game's code never reads this field. */
};

/* Distance calculation block and its silence model. */
struct DISTSIL {
    struct _BLOCK BaseBlock; /* Block lifecycle, queue, and control state. */
    u16 *ActiveDist; /* Phenome selection supplied with the current input. */
    u8 *ActiveBoolDist; /* Flags following the phenome selection in the input buffer. */
    u16 NbrCodeBooks; /* Number of codebooks in the language. */
    u8 *Ptrs[10]; /* Selected distance row for each codebook. */
    u16 *GasPtr; /* Gastone table indexed by phenome and codebook. */
    s32 sOffset; /* Baseline subtracted from a calculated distance. */
    struct _OUTPUTQUEUE *OutQueue; /* Queue for the calculated phenome distances. */
    /* Distance routine for the current probability encoding. */
    void (*DistCalc)(DISTSIL *, u16 *, int, u16);
    signed short *pCachedDistances; /* Cached distances returned to the recognizer. */
    DistanceMatrix *SDMatrices; /* Stored distance matrices released during block teardown. */
    DistanceMatrix *SIMatrices; /* Distance matrices loaded from the language codebooks. */
    struct WMATRIX_ SDGastoneMatrix; /* The game's code never reads this field. */
    struct WMATRIX_ SIGastoneMatrix; /* Gastone matrix loaded from the language. */
    DistanceMatrix *ActiveMatrices; /* Distance matrices used by the current model. */
    struct WMATRIX_ *ActiveGastoneMatrix; /* Gastone matrix used by the current model. */
    u16 NbrSDMatrices; /* Number of entries in SDMatrices. */
    u16 NbrSIMatrices; /* Number of entries in SIMatrices. */
    u16 NbrActiveMatrices; /* Number of distance matrices in the current model. */
    u16 AnySpeechGarbagePhenom; /* The game's code never reads this field. */
    s16 AnySpeechPenalty; /* Penalty used for the any-speech path. */
    s16 RejectionPenalty; /* Additional penalty used for the rejection path. */
    u16 NbrErgodicStates; /* Number of ergodic states in the language. */
    u16 NbrPhenoms; /* Number of phenomes in the language. */
    u16 NbrErgodicPhenoms; /* Number of ergodic phenomes in the language. */
    u16 NbrNonErgodicPhenoms; /* The game's code never reads this field. */
    s16 OffsetPhenToStat; /* Adjustment from a phenome ID to a state index. */
    u16 *pErgodicStates; /* Language table of state lists for the ergodic phenomes. */
    u16 *pErgodicPhenoms; /* Language table of ergodic phenome entries. */
    u16 *pErgodicDist; /* Calculated distances for ergodic states. */
    s16 *pLocalErgodicPenalty; /* Working penalty table adjusted for any-speech and rejection. */
    const s16 *pErgodicPenalty; /* Base ergodic penalties supplied by the language. */
    u16 NbrSilPhenoms; /* The game's code never reads this field. */
    u16 *pSilPhenoms; /* The game's code never reads this field. */
    u8 *ppAdaptSil[10]; /* Adapted silence distances, one array per codebook. */
    u16 AdaptSilState; /* State selected to initialize the silence model. */
    u16 NbrInTempDist; /* The game's code never reads this field. */
    u16 (*GetNbrOfStatesInPhenom)(DISTSIL *, u16); /* The game's code never reads this field. */
    u8 (*BuildReducedModel)(DISTSIL *, u16, u16 *); /* The game's code never reads this field. */
    u8 (*FreeReducedModel)(DISTSIL *); /* The game's code never reads this field. */
    struct SAVEDMODELSTATE_ SavedModelState; /* The game's code never reads this field. */
    DistanceMatrix *VpMatrices; /* The game's code never reads this field. */
    u16 NbrVpMatrices; /* The game's code never reads this field. */
    u32 Frame; /* Frame count within the current adaptation batch. */
    u32 Frame2; /* Adaptation batches elapsed since normalization. */
    unsigned char NeverAdapt; /* Profile flag that disables silence adaptation. */
    unsigned char SilAdaptOn; /* Current silence adaptation enable flag. */
    unsigned char Process; /* Whether to accept frames once a language is loaded. */
    float *ExpTable; /* Linear probabilities indexed by encoded silence distance. */
    float MaxProb; /* Upper bound for the silence log-probability range. */
    float MinProb; /* Lower bound for the silence log-probability range. */
    f32 Epsilon; /* Probability increment for each histogram observation. */
    u32 FramesToBulk; /* Number of frames per silence adaptation batch. */
    u8 **ppLookUp; /* Updated silence distances indexed by histogram count and old distance. */
};

/* One codebook probability matrix and its adaptation histogram. */
struct DistanceMatrix {
    u8 **Matrix; /* Probability rows indexed by the incoming codebook label. */
    u16 Rows; /* Number of labels in the codebook. */
    u16 Columns; /* Bytes per probability row. */
    u8 *pHisto; /* Observation count for each label during adaptation. */
};

/* Computes phenome distances using byte probabilities. */
static void DistCalc(DISTSIL *This, u16 *Distances, int NbrOfLabels, unsigned short AdaptSilDist);

/* Computes phenome distances using packed four-bit probabilities. */
static void DistCalc4Bit(
    DISTSIL *This, u16 *Distances, int NbrOfLabels, unsigned short AdaptSilDist);

/* Selects probability rows and prepares the cached ergodic distances. */
static void InitCalculateDistances(void *pThis, s16 *pDistances,
    u8 *pLabels, unsigned char Speech, int sOffset);

/* Returns the cached distance table owned by the block. */
static signed short *GetpCachedDistances(void *pThis);

/* Returns the distance baseline subtracted from each matrix result. */
static long GetDistanceOffset(void *pThis);

/* Sums byte distances across the codebooks for one phenome. */
static signed short CalculateDistance(void *pThis, unsigned int Phenom);

/* Sums four-bit distances across the codebooks for one phenome. */
static signed short CalculateDistance4Bit(void *pThis, unsigned int Phenom);

/* Releases prior language matrices and resizes their table when needed. */
static void *AllocMatrices(struct _OBJECT *This, DistanceMatrix **ppMatrix,
    u16 *pNbrMatrices, unsigned short NewNbrMatrices)
{
    struct _TOS *pTos = This->pTos;
    DistanceMatrix *pMatrix = *ppMatrix;
    s32 i = 0;

    while (i < *pNbrMatrices) {
        if (pMatrix->Matrix != NULL) {
            heap_Free(pTos->hHeap, pMatrix->Matrix);
            heap_Free(pTos->hHeap, pMatrix->pHisto);
            pMatrix->Matrix = NULL;
        }
        i++;
        pMatrix++;
    }
    if (*pNbrMatrices != NewNbrMatrices) {
        if (*pNbrMatrices != 0) {
            heap_Free(pTos->hHeap, *ppMatrix);
        }
        if (NewNbrMatrices != 0) {
            *ppMatrix = heap_Calloc(pTos->hHeap, NewNbrMatrices,
                sizeof(DistanceMatrix));
        }
        *pNbrMatrices = NewNbrMatrices;
    }
    return *ppMatrix;
}

/* Loads a matrix and its per-row transposition markers for one language. */
static u8 LoadMatrix(struct _OBJECT *This, DistanceMatrix *pMatrix, unsigned short Rows,
    unsigned short Columns, u8 *pData)
{
    struct _TOS *pTos = This->pTos;
    u8 *Ptr = pData;
    s32 i = 0;

    if (pMatrix->Matrix != NULL) {
        heap_Free(pTos->hHeap, pMatrix->Matrix);
        heap_Free(pTos->hHeap, pMatrix->pHisto);
    }
    pMatrix->Rows = Rows;
    pMatrix->Columns = Columns;
    pMatrix->Matrix = heap_Alloc(pTos->hHeap, Rows * sizeof(s32));
    pMatrix->pHisto = heap_Calloc(pTos->hHeap, Rows, sizeof(u8));
    if (pMatrix->Matrix == NULL || pMatrix->pHisto == NULL) {
        return _tosErrorLog(This, 2);
    }
    while (i < Rows) {
        pMatrix->Matrix[i] = Ptr;
        Ptr += Columns;
        i++;
    }
    return 0;
}

/* Initializes the silence distances from the selected gastone. */
static void CopySilProbs(DISTSIL *This)
{
    unsigned int i;
    unsigned int j;
    unsigned short AdaptSilGastone;
    unsigned char **Matrix;
    unsigned char mask;
    unsigned long shift;
    unsigned long factor;
    unsigned long SecCdbSize;

    for (i = 0; i < This->NbrCodeBooks; i++) {
        mask = 0xFF;
        shift = 0;
        factor = 1;
        AdaptSilGastone =
            (This->SIGastoneMatrix.Matrix + This->AdaptSilState * This->NbrCodeBooks)[i];
        Matrix = This->SIMatrices[i].Matrix;
        if (This->DistCalc == DistCalc4Bit) {
            if (AdaptSilGastone & 1) {
                mask = 0xF;
            } else {
                shift = 4;
            }
            AdaptSilGastone /= 2;
            factor = 0x11;
        }
        SecCdbSize = This->SIMatrices[i].Rows;
        for (j = 0; j < SecCdbSize; j++) {
            This->ppAdaptSil[i][j] = factor * ((Matrix[j][AdaptSilGastone] & mask) >> shift);
        }
    }
}

/* Restores the adaptation counters, distances, and histograms. */
static void DistCalcPutSession(DISTSIL *This, u8 *pSession)
{
    u8 *bufptr = pSession;
    s32 i = 0;

    This->Frame = *(s32 *)bufptr;
    bufptr += 4;
    This->Frame2 = *(s32 *)bufptr;
    bufptr += 4;
    while (i < This->NbrActiveMatrices) {
        s32 j = 0;

        while (j < This->ActiveMatrices[i].Rows) {
            This->ppAdaptSil[i][j] = bufptr[0];
            This->ActiveMatrices[i].pHisto[j] = bufptr[1];
            bufptr += 2;
            j++;
        }
        i++;
    }
}

/* Saves the adaptation counters, distances, and histograms. */
static void DistCalcGetSession(DISTSIL *This, u8 *pSession)
{
    u8 *bufptr = pSession;
    s32 i = 0;

    *(s32 *)bufptr = This->Frame;
    bufptr += 4;
    *(s32 *)bufptr = This->Frame2;
    bufptr += 4;
    while (i < This->NbrActiveMatrices) {
        s32 j = 0;

        while (j < This->ActiveMatrices[i].Rows) {
            bufptr[0] = This->ppAdaptSil[i][j];
            bufptr[1] = This->ActiveMatrices[i].pHisto[j];
            bufptr += 2;
            j++;
        }
        i++;
    }
}

/* Stores a language gastone matrix and its dimensions. */
static u8 LoadGastone(struct WMATRIX_ *pGastoneMatrix, unsigned short NbrOfStates,
    unsigned short NbrOfCodeBooks, unsigned short *pData)
{
    pGastoneMatrix->Rows = NbrOfStates;
    pGastoneMatrix->Columns = NbrOfCodeBooks;
    pGastoneMatrix->Matrix = pData;
    return 0;
}

/* Applies the rejection and any-speech penalties to the ergodic distances. */
static void UpdateRejectionAnySpeech(DISTSIL *This)
{
    const signed short *pErgodicPenalty = This->pErgodicPenalty;
    const signed short *pErgodicPenaltyEnd =
        pErgodicPenalty + This->pErgodicPhenoms[This->NbrErgodicPhenoms] - This->pErgodicPhenoms[0];
    signed short *pLocalErgodicPenalty = This->pLocalErgodicPenalty;
    signed short Penalty;
    long LocalPenalty;

    while (pErgodicPenalty < pErgodicPenaltyEnd) {
        Penalty = *pErgodicPenalty;
        LocalPenalty = Penalty >> 2;
        if (Penalty & 1) {
            LocalPenalty += This->RejectionPenalty;
        }
        if (Penalty & 2) {
            LocalPenalty += This->AnySpeechPenalty;
        }
        *pLocalErgodicPenalty = (signed short)LocalPenalty;
        pErgodicPenalty++;
        pLocalErgodicPenalty++;
    }
}

/* Resizes the connected queues for the new phoneme count. */
static unsigned char ResizeQueues(DISTSIL *This, unsigned int NbrPhenom)
{
    if (This->BaseBlock.NrInputs != 0 && This->OutQueue->pQ != NULL &&
        This->OutQueue->pQ->Size != NbrPhenom * 2) {
        This->OutQueue->Size = NbrPhenom * 2;
        if (qQueueReDimension(This->OutQueue->pQ, NbrPhenom * 2) == 0) {
            return 1;
        }
    }
    if (This->BaseBlock.NrInputs == 3) {
        if (This->BaseBlock.InputQueues[2].pQ != NULL &&
            This->BaseBlock.InputQueues[2].pQ->Size != NbrPhenom * 3) {
            This->BaseBlock.InputQueues[2].Size = NbrPhenom * 3;
            if (qQueueReDimension(This->BaseBlock.InputQueues[2].pQ, NbrPhenom * 3) == 0) {
                return 1;
            }
        }
        if (This->BaseBlock.InputQueues[2].pQ != NULL) {
            qDeQueueOne(This->BaseBlock.InputQueues[2].pQ, This->BaseBlock.InputQueues[2].ReadPos);
        }
    }
    return 0;
}

/* Loads the language matrices and initializes the silence model. */
static u8 LoadLanguage(DISTSIL *This, struct _LANGUAGE *pLang)
{
    unsigned int i;
    int FourBitModels;
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    void *pCdb;

    FourBitModels = pLang->langCheckBitField(pLang, 1);
    This->GasPtr = NULL;
    This->AdaptSilState = pLang->langGetAdaptSilState(pLang);
    This->NbrErgodicStates = pLang->langGetNbrErgodicStates(pLang);
    This->NbrPhenoms = pLang->langGetNbrPhenome(pLang);
    This->NbrNonErgodicPhenoms = pLang->langGetNbrNonErgodicPhenomes(pLang);
    This->NbrErgodicPhenoms = pLang->langGetNbrErgodicPhenomes(pLang);
    This->OffsetPhenToStat = -This->NbrErgodicPhenoms;
    This->pErgodicStates = pLang->langGetpErgodicStates(pLang);
    This->pErgodicPhenoms = pLang->langGetpErgodicPhenomes(pLang);
    This->pErgodicPenalty = pLang->langGetpErgodicPenalty(pLang);
    if (This->pErgodicDist != NULL) {
        heap_Free(pTos->hHeap, This->pErgodicDist);
        heap_Free(pTos->hHeap, This->pLocalErgodicPenalty);
    }
    This->pErgodicDist = heap_Alloc(pTos->hHeap, This->NbrErgodicStates * 2);
    This->pLocalErgodicPenalty = heap_Alloc(pTos->hHeap,
        (This->pErgodicPhenoms[This->NbrErgodicPhenoms] - This->pErgodicPhenoms[0]) * 2);
    UpdateRejectionAnySpeech(This);
    This->AnySpeechGarbagePhenom = pLang->langGetAnySpeechGarbagePhenome(pLang);
    LoadGastone(&This->SIGastoneMatrix, pLang->langGetNbrState(pLang),
        pLang->langGetNbrCodeBook(pLang), pLang->langGetpGastone(pLang));
    This->ActiveGastoneMatrix = &This->SIGastoneMatrix;
    This->NbrCodeBooks = pLang->langGetNbrCodeBook(pLang);
    if (AllocMatrices(&This->BaseBlock.Object, &This->SIMatrices,
                      &This->NbrSIMatrices, This->NbrCodeBooks) == NULL) {
        return 1;
    }
    for (i = 0; i < This->NbrCodeBooks; i++) {
        pCdb = pLang->langGetpCodeBook(pLang, i);
        if (LoadMatrix(&This->BaseBlock.Object, &This->SIMatrices[i],
                pLang->cdbGetSecondCdbSize(pCdb),
                FourBitModels ? (pLang->cdbGetNbrGastone(pCdb) + 1) >> 1
                              : pLang->cdbGetNbrGastone(pCdb),
                (u8 *)pLang->cdbGetpProbMatrix(pCdb))) {
            return 1;
        }
        if (This->ppAdaptSil[i] != NULL) {
            heap_Free(pTos->hHeap, This->ppAdaptSil[i]);
        }
        This->ppAdaptSil[i] = heap_Alloc(pTos->hHeap, pLang->cdbGetSecondCdbSize(pCdb));
    }
    This->ActiveMatrices = This->SIMatrices;
    This->NbrActiveMatrices = This->NbrSIMatrices;
    if (ResizeQueues(This, pLang->langGetNbrPhenome(pLang))) {
        return 1;
    }
    if (FourBitModels) {
        This->DistCalc = DistCalc4Bit;
    } else {
        This->DistCalc = DistCalc;
    }
    CopySilProbs(This);
    This->GasPtr =
        This->ActiveGastoneMatrix->Matrix + This->OffsetPhenToStat * This->NbrCodeBooks;
    if (This->pCachedDistances != NULL) {
        heap_Free(pTos->hHeap, This->pCachedDistances);
    }
    This->pCachedDistances = heap_Alloc(pTos->hHeap, This->NbrPhenoms * 2);
    if (This->pCachedDistances == NULL) {
        return _tosErrorLog(&This->BaseBlock.Object, 2);
    }
    return 0;
}

/* Computes phenome distances using byte probabilities. */
static void DistCalc(DISTSIL *This, u16 *Distances, int NbrOfLabels, unsigned short AdaptSilDist)
{
    int i;
    u16 *pErgodicPhenom;
    u16 *pErgodicPhenomEnd;
    u16 *pErgodicDist = This->pErgodicDist;
    u16 *GastoneMatrix = This->ActiveGastoneMatrix->Matrix;
    int Nbr = This->ActiveDist[0];
    int NbrErgodicPhenoms = This->NbrErgodicPhenoms;
    int OffsetPhenToStat = This->OffsetPhenToStat;
    u16 *pState;
    u16 *pStateEnd;
    s16 *pErgodicPenalty = This->pLocalErgodicPenalty;
    unsigned int MaxDist = NbrOfLabels * 0xFF;
    u8 **pStop = This->Ptrs + NbrOfLabels - 1;
    u8 **pStart = This->Ptrs;

    GastoneMatrix += OffsetPhenToStat * NbrOfLabels;
    if (Nbr == This->NbrPhenoms) {
        u16 *pDist = Distances + Nbr - 1;
        u16 *pStopDist = Distances + NbrErgodicPhenoms;
        u16 *GasPtr = GastoneMatrix + Nbr * NbrOfLabels - 1;

        while (pDist >= pStopDist) {
            u8 **pTmp = pStop;

            *pDist = (*pTmp--)[*GasPtr--];
            while (pTmp >= pStart) {
                *pDist += (*pTmp--)[*GasPtr--];
            }
            pDist--;
        }
        memset(This->ActiveBoolDist + This->NbrErgodicPhenoms, 0,
               Nbr - This->NbrErgodicPhenoms);
    } else {
        int j;

        for (j = NbrErgodicPhenoms + 1; j <= Nbr; j++) {
            int Phenom = This->ActiveDist[j];
            u16 *GasPtr = GastoneMatrix + Phenom * NbrOfLabels;
            u8 **pTmp = pStart;

            Distances[Phenom] = (*pTmp++)[*GasPtr++];
            while (pTmp <= pStop) {
                Distances[Phenom] += (*pTmp++)[*GasPtr++];
            }
            This->ActiveBoolDist[Phenom] = 0;
        }
    }
    GastoneMatrix -= OffsetPhenToStat * NbrOfLabels;
    pState = This->pErgodicStates + This->pErgodicStates[0];
    for (i = 0; i < This->NbrErgodicStates; i++) {
        unsigned int Dist = MaxDist;

        pStateEnd = This->pErgodicStates + This->pErgodicStates[i + 1];
        while (pState < pStateEnd) {
            unsigned int Dst;

            if (*pState != This->AdaptSilState) {
                u8 **pTmp = pStart;
                u16 *GasPtr = GastoneMatrix + *pState * NbrOfLabels;

                Dst = (*pTmp++)[*GasPtr++];
                while (pTmp <= pStop) {
                    Dst += (*pTmp++)[*GasPtr++];
                }
            } else {
                Dst = AdaptSilDist;
            }
            if (Dst < Dist) {
                Dist = Dst;
            }
            pState++;
        }
        pErgodicDist[i] = Dist;
    }
    pErgodicPhenom = This->pErgodicPhenoms + This->pErgodicPhenoms[0];
    for (i = 0; i < This->NbrErgodicPhenoms; i++) {
        int Dist;
        int Dst;

        pErgodicPhenomEnd = This->pErgodicPhenoms + This->pErgodicPhenoms[i + 1];
        Dist = pErgodicDist[*pErgodicPhenom++] + *pErgodicPenalty++;
        while (pErgodicPhenom < pErgodicPhenomEnd) {
            Dst = pErgodicDist[*pErgodicPhenom] + *pErgodicPenalty++;
            if (Dst < Dist) {
                Dist = Dst;
            }
            pErgodicPhenom++;
        }
        Distances[i] = Dist > 0 ? Dist : 0;
    }
}

/* Computes phenome distances using packed four-bit probabilities. */
static void DistCalc4Bit(
    DISTSIL *This, u16 *Distances, int NbrOfLabels, unsigned short AdaptSilDist)
{
    int i;
    u16 *pErgodicPhenom;
    u16 *pErgodicPhenomEnd;
    u16 *pErgodicDist = This->pErgodicDist;
    u16 *GastoneMatrix = This->ActiveGastoneMatrix->Matrix;
    int Nbr = This->ActiveDist[0];
    int NbrErgodicPhenoms = This->NbrErgodicPhenoms;
    signed short OffsetPhenToStat = This->OffsetPhenToStat;
    u16 *pState;
    u16 *pStateEnd;
    s16 *pErgodicPenalty = This->pLocalErgodicPenalty;
    unsigned int MaxDist = NbrOfLabels * 0xFF;
    u8 **pStop = This->Ptrs + NbrOfLabels - 1;
    u8 **pStart = This->Ptrs;

    GastoneMatrix += OffsetPhenToStat * NbrOfLabels;
    if (Nbr == This->NbrPhenoms) {
        u16 *pDist = Distances + Nbr - 1;
        u16 *pStopDist = Distances + NbrErgodicPhenoms;
        u16 *GasPtr = GastoneMatrix + Nbr * NbrOfLabels - 1;

        while (pDist >= pStopDist) {
            u8 **pTmp = pStop;

            {
                unsigned int Gastone = *GasPtr--;

                *pDist = ((*pTmp--)[Gastone >> 1] << ((Gastone & 1) << 2)) & 0xF0;
            }
            while (pTmp >= pStart) {
                {
                    unsigned int Gastone = *GasPtr--;

                    *pDist += ((*pTmp--)[Gastone >> 1] << ((Gastone & 1) << 2)) & 0xF0;
                }
            }
            pDist--;
        }
        memset(This->ActiveBoolDist + This->NbrErgodicPhenoms, 0,
               Nbr - This->NbrErgodicPhenoms);
    } else {
        for (i = NbrErgodicPhenoms + 1; i <= Nbr; i++) {
            unsigned int Phenom = This->ActiveDist[i];
            u16 *GasPtr = GastoneMatrix + Phenom * NbrOfLabels;
            u8 **pTmp = pStart;

            {
                unsigned int Gastone = *GasPtr++;

                Distances[Phenom] = ((*pTmp++)[Gastone >> 1] << ((Gastone & 1) << 2)) & 0xF0;
            }
            while (pTmp <= pStop) {
                {
                    unsigned int Gastone = *GasPtr++;

                    Distances[Phenom] += ((*pTmp++)[Gastone >> 1] << ((Gastone & 1) << 2)) & 0xF0;
                }
            }
            This->ActiveBoolDist[Phenom] = 0;
        }
    }
    GastoneMatrix -= OffsetPhenToStat * NbrOfLabels;
    pState = This->pErgodicStates + This->pErgodicStates[0];
    for (i = 0; i < This->NbrErgodicStates; i++) {
        unsigned int Dist = MaxDist;

        pStateEnd = This->pErgodicStates + This->pErgodicStates[i + 1];
        while (pState < pStateEnd) {
            unsigned int Dst;

            if (*pState != This->AdaptSilState) {
                u8 **pTmp = pStart;
                u16 *GasPtr = GastoneMatrix + *pState * NbrOfLabels;

                {
                    unsigned int Gastone = *GasPtr++;

                    Dst = ((*pTmp++)[Gastone >> 1] << ((Gastone & 1) << 2)) & 0xF0;
                }
                while (pTmp <= pStop) {
                    {
                        unsigned int Gastone = *GasPtr++;

                        Dst += ((*pTmp++)[Gastone >> 1] << ((Gastone & 1) << 2)) & 0xF0;
                    }
                }
            } else {
                Dst = AdaptSilDist;
            }
            if (Dst < Dist) {
                Dist = Dst;
            }
            pState++;
        }
        pErgodicDist[i] = Dist;
    }
    pErgodicPhenom = This->pErgodicPhenoms + This->pErgodicPhenoms[0];
    for (i = 0; i < This->NbrErgodicPhenoms; i++) {
        int Dist;
        int Dst;

        pErgodicPhenomEnd = This->pErgodicPhenoms + This->pErgodicPhenoms[i + 1];
        Dist = pErgodicDist[*pErgodicPhenom++] + *pErgodicPenalty++;
        while (pErgodicPhenom < pErgodicPhenomEnd) {
            Dst = pErgodicDist[*pErgodicPhenom] + *pErgodicPenalty++;
            if (Dst < Dist) {
                Dist = Dst;
            }
            pErgodicPhenom++;
        }
        Distances[i] = Dist > 0 ? Dist : 0;
    }
}

/* Idle callback for the distance calculation block. */
static void IdleDistCalcProcess(struct _BLOCK *block, void *inputs, int inputCount)
{
}

/* Releases the current language matrices and working distance buffers. */
static void FreeLanguage(DISTSIL *This)
{
    u32 i;
    struct _TOS *pTos = This->BaseBlock.Object.pTos;

    AllocMatrices(&This->BaseBlock.Object, &This->SDMatrices,
        &This->NbrSDMatrices, 0);
    AllocMatrices(&This->BaseBlock.Object, &This->SIMatrices,
        &This->NbrSIMatrices, 0);
    for (i = 0; i < 10; i++) {
        if (This->ppAdaptSil[i] != NULL) {
            heap_Free(pTos->hHeap, This->ppAdaptSil[i]);
            This->ppAdaptSil[i] = NULL;
        }
    }
    if (This->pErgodicDist != NULL) {
        heap_Free(pTos->hHeap, This->pErgodicDist);
        This->pErgodicDist = NULL;
        heap_Free(pTos->hHeap, This->pLocalErgodicPenalty);
        This->pLocalErgodicPenalty = NULL;
    }
    This->pErgodicStates = 0;
    This->pErgodicPhenoms = 0;
    This->pErgodicPenalty = 0;
    if (This->pCachedDistances != NULL) {
        heap_Free(pTos->hHeap, This->pCachedDistances);
        This->pCachedDistances = NULL;
    }
}

/* Releases the distance calculator and its adaptation tables. */
static u8 FreeDistCalc(DISTSIL *This)
{
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    s32 i;

    heap_Free(pTos->hHeap, This->ppLookUp[0]);
    heap_Free(pTos->hHeap, This->ppLookUp);
    heap_Free(pTos->hHeap, This->ExpTable);
    AllocMatrices(&This->BaseBlock.Object, &This->SDMatrices,
        &This->NbrSDMatrices, 0);
    AllocMatrices(&This->BaseBlock.Object, &This->SIMatrices,
        &This->NbrSIMatrices, 0);
    for (i = 0; i < 10; i++) {
        if (This->ppAdaptSil[i] != NULL) {
            heap_Free(pTos->hHeap, This->ppAdaptSil[i]);
        }
    }
    if (This->pErgodicDist != NULL) {
        heap_Free(pTos->hHeap, This->pErgodicDist);
        heap_Free(pTos->hHeap, This->pLocalErgodicPenalty);
    }
    if (This->pCachedDistances != NULL) {
        heap_Free(pTos->hHeap, This->pCachedDistances);
        This->pCachedDistances = NULL;
    }
    tosBaseBlockDestruct(&This->BaseBlock);
    return 0;
}

/* Initializes the distance queues and builds silence adaptation tables. */
static u8 InitDistSilProcess(struct _BLOCK *_This)
{
    DISTSIL *This = (DISTSIL *)_This;
    unsigned short NbrPhenoms;
    unsigned short Max_NbrCodeBooks;
    struct _TOS *pTos = This->BaseBlock.Object.pTos;
    unsigned int i;
    unsigned int j;
    unsigned char *Ptr;
    float Increment;
    float Temp2;
    float Temp;

    This->Process = 1;
    Max_NbrCodeBooks = _tosGetProfileU32(&This->BaseBlock.Object, 0x20, 4);
    NbrPhenoms = _tosGetProfileU32(&This->BaseBlock.Object, 0x21, 1);
    This->AnySpeechGarbagePhenom = 0xFFFF;
    This->AnySpeechPenalty = _tosGetProfileU32(&This->BaseBlock.Object, 0x1E, 0);
    This->RejectionPenalty = _tosGetProfileU32(&This->BaseBlock.Object, 0x1F, 0);
    if (This->BaseBlock.NrInputs != 0) {
        This->OutQueue = This->BaseBlock.OutputQueues;
        This->BaseBlock.InputQueues[1].Size = Max_NbrCodeBooks * sizeof(unsigned char);
        This->BaseBlock.InputQueues[0].Size = sizeof(unsigned char);
        This->OutQueue->Size = NbrPhenoms * sizeof(signed short);
        if (This->BaseBlock.NrInputs == 3) {
            This->BaseBlock.InputQueues[2].Size = NbrPhenoms * 3 * sizeof(unsigned char);
        }
    }
    This->FramesToBulk = (unsigned short)_tosGetProfileU32(&This->BaseBlock.Object, 0x13, 4);
    This->MaxProb = 0.0f;
    This->MinProb = -9.22306f;
    This->ExpTable = heap_Alloc(pTos->hHeap, 0x400);
    for (i = 0; i < 0x100; i++) {
        This->ExpTable[i] = expf(This->MaxProb - i * (This->MaxProb - This->MinProb) / 255.0);
    }
    This->Epsilon = _tosGetProfileFloat(&This->BaseBlock.Object, 0x12, 0.002f);
    This->ppLookUp = heap_Alloc(pTos->hHeap, (This->FramesToBulk + 1) * sizeof(unsigned char *));
    Ptr = heap_Alloc(pTos->hHeap, (This->FramesToBulk + 1) * 0x100);
    if (Ptr == NULL) {
        return 1;
    }
    for (i = 0; i <= This->FramesToBulk; i++) {
        This->ppLookUp[i] = Ptr;
        Ptr += 0x100;
    }
    Temp2 = logf_check(1.0 + This->FramesToBulk * This->Epsilon);
    for (j = 0; j <= This->FramesToBulk; j++) {
        Increment = j * This->Epsilon;
        for (i = 0; i < 0x100; i++) {
            Temp = This->MaxProb - i * (This->MaxProb - This->MinProb) / 255.0;
            Temp = expf(Temp);
            Temp += Increment;
            Temp = logf_check(Temp) - Temp2;
            Temp = 0.5 + -255.0 * (Temp - This->MaxProb) / (This->MaxProb - This->MinProb);
            if (Temp < 0.0f) {
                Temp = 0.0f;
            } else if (Temp > 255.0f) {
                Temp = 255.0f;
            }
            This->ppLookUp[j][i] = Temp;
        }
    }
    This->Frame = 0;
    This->Frame2 = 0;
    This->NeverAdapt = (signed short)_tosGetProfileU32(&This->BaseBlock.Object, 0x14, 0);
    This->SilAdaptOn = 1;
    This->pCachedDistances = NULL;
    return 0;
}

/* Normalizes the adapted silence distances using their summed probability. */
static void NormCorrection(DISTSIL *This, DistanceMatrix *ActMat, u8 *AdaptSilProb)
{
    float flSum = 0.0f;
    int i;
    float flCorrection;
    int dkCorrection;
    int Dist;

    for (i = 0; i < ActMat->Rows; i++) {
        flSum += This->ExpTable[AdaptSilProb[i]];
    }
    flCorrection = 255.0 * logf_check(flSum) / (This->MaxProb - This->MinProb);
    if (flCorrection < 0.0) {
        dkCorrection = flCorrection - 0.5;
    } else {
        dkCorrection = 0.5 + flCorrection;
    }
    for (i = 0; i < ActMat->Rows; i++) {
        Dist = dkCorrection + AdaptSilProb[i];
        if (Dist > 0xFF) {
            Dist = 0xFF;
        } else if (Dist < 0) {
            Dist = 0;
        }
        AdaptSilProb[i] = Dist;
    }
}

/* Updates the silence distances from the histogram and clears its counts. */
static void AdaptSilence(DistanceMatrix *ActMat, unsigned char *AdaptSilProb,
    const unsigned char *const *ppLocalLookUp)
{
    int i;
    unsigned char Dist;
    unsigned char n;

    for (i = 0; i < ActMat->Rows; i++) {
        Dist = ActMat->pHisto[i];
        n = AdaptSilProb[i];
        AdaptSilProb[i] = ppLocalLookUp[Dist][n];
        ActMat->pHisto[i] = 0;
    }
}

/* Updates the silence model and queues distances for the current frame. */
static void DistSilProcess(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    u8 Adapt;
    u8 AdaptSil;
    int UpdateHisto;
    DISTSIL *This = (DISTSIL *)_This;
    u8 *Labels = ((void **)pInputs)[1];

    if (Labels != NULL) {
        u8 *pLabels = Labels;

        if (pLabels != NULL) {
            s32 AdaptSilDist;
            DistanceMatrix *ActMat;
            s32 cdbid;

            This->ActiveDist = ((void **)pInputs)[2];
            Adapt = 0;
            This->ActiveBoolDist = (u8 *)(This->ActiveDist + This->NbrPhenoms);
            if (This->SilAdaptOn != 0) {
                Adapt = 1 - *(u8 *)((void **)pInputs)[0];
            }
            if (This->NeverAdapt != 0) {
                Adapt = 0;
            }
            UpdateHisto = Adapt;
            AdaptSil = 0;
            if (UpdateHisto != 0) {
                if (++This->Frame % This->FramesToBulk == 0) {
                    This->Frame = 0;
                    AdaptSil = 1;
                    This->Frame2++;
                }
            }
            ActMat = This->ActiveMatrices;
            AdaptSilDist = 0;
            cdbid = 0;
            while (cdbid < This->NbrActiveMatrices) {
                u8 *pAdaptSil = This->ppAdaptSil[cdbid];

                if (UpdateHisto != 0) {
                    ActMat->pHisto[*pLabels]++;
                }
                This->Ptrs[cdbid] = ActMat->Matrix[*pLabels];
                if (AdaptSil != 0) {
                    AdaptSilence(ActMat, pAdaptSil, This->ppLookUp);
                    if (This->Frame2 >= SILENCE_NORMALIZATION_BATCHES) {
                        NormCorrection(This, ActMat, pAdaptSil);
                    }
                }
                AdaptSilDist += pAdaptSil[*pLabels];
                pLabels++;
                ActMat++;
                cdbid++;
            }
            if (This->ActiveDist != 0) {
                u16 *Distances = qEnQueueOne(This->OutQueue->pQ);

                This->DistCalc(This, Distances, cdbid, (u16)AdaptSilDist);
            }
            if (This->Frame2 >= SILENCE_NORMALIZATION_BATCHES) {
                This->Frame2 = 0;
            }
        }
    }
}

/* Clears the frame counters and silence adaptation histograms. */
static void ResetSilenceAdaptation(DISTSIL *This)
{
    int i;
    int cdbid;
    DistanceMatrix *ActMat;

    This->Frame = 0;
    This->Frame2 = 0;
    ActMat = This->ActiveMatrices;
    for (cdbid = 0; cdbid < This->NbrActiveMatrices; cdbid++) {
        for (i = 0; i < ActMat->Rows; i++) {
            ActMat->pHisto[i] = 0;
        }
        ActMat++;
    }
}

/* Copies the current silence adaptation values into the caller buffer. */
static void DistCalcGetAdaptSilState(DISTSIL *This, void *pState)
{
    u8 *bufptr = pState;
    int i, j;

    for (i = 0; i < This->NbrActiveMatrices; i++) {
        for (j = 0; j < This->ActiveMatrices[i].Rows; j++) {
            *bufptr++ = This->ppAdaptSil[i][j];
        }
    }
}

/* Sets the adaptation flag and updates the control input queue. */
static u8 SilAdaptEnable(DISTSIL *This, u8 Enabled)
{
    This->SilAdaptOn = Enabled;
    return This->BaseBlock.NrInputs != 0 ? (u8)!qQueueControl(This->BaseBlock.InputQueues->pQ,
                                          Enabled ? 5 : 8, This->BaseBlock.InputQueues->ReadPos, 0)
                                    : (u8)0;
}

/* Handles language, session, and adaptation commands for the block. */
static u8 ControlDistSilProcess(struct _BLOCK *_This, unsigned char ControlId, u32 CParam1,
    u32 CParam2)
{
    DISTSIL *This = (DISTSIL *)_This;
    u8 RetVal = 1;

    switch ((s32)ControlId) {
    case 0x1C:
        _This->Process = IdleDistCalcProcess;
        This->Process = 0;
        break;
    case 0x1D:
        This->Process = 1;
        if (This->GasPtr != NULL) {
            _This->Process = DistSilProcess;
        }
        break;
    case 0x17:
        RetVal = qQueueControl(_This->InputQueues->pQ, 7, _This->InputQueues->ReadPos, CParam1);
        break;
    case 0x18:
        RetVal = qQueueControl(_This->InputQueues->pQ, 6, _This->InputQueues->ReadPos, 0);
        break;
    case 0x10:
        _This->Process = IdleDistCalcProcess;
        if (LoadLanguage(This, (struct _LANGUAGE *)CParam1) == 0) {
            ResetSilenceAdaptation(This);
            if (_This->NrInputs != 0 && This->Process != 0) {
                _This->Process = DistSilProcess;
            }
        }
        break;
    case 0x1F:
        RetVal = !ResizeQueues(
            This, ((struct _LANGUAGE *)CParam1)->langGetNbrPhenome((struct _LANGUAGE *)CParam1));
        break;
    case 0x1E:
        _This->Process = IdleDistCalcProcess;
        FreeLanguage(This);
        break;
    case 0xA:
        if (CParam1 != 0) {
            unsigned long *pIndex = (unsigned long *)CParam2;
            int i;

            DistCalcPutSession(This, (u8 *)CParam1 + *pIndex);
            *pIndex += 8;
            for (i = 0; i < This->NbrActiveMatrices; i++) {
                *pIndex += This->ActiveMatrices[i].Rows * 2;
            }
        } else {
            ResetSilenceAdaptation(This);
            CopySilProbs(This);
        }
        break;
    case 0xB:
    {
        unsigned long *pIndex = (unsigned long *)CParam2;
        int i;

        if (CParam1 != 0) {
            DistCalcGetSession(This, (u8 *)CParam1 + *pIndex);
        }
        *pIndex += 8;
        for (i = 0; i < This->NbrActiveMatrices; i++) {
            *pIndex += This->ActiveMatrices[i].Rows * 2;
        }
        break;
    }
    case 0x1A:
    {
        unsigned long *pIndex = (unsigned long *)CParam2;
        int i;

        if (CParam1 != 0) {
            DistCalcGetAdaptSilState(This, (u8 *)CParam1 + *pIndex);
        }
        *pIndex = 0;
        for (i = 0; i < This->NbrActiveMatrices; i++) {
            *pIndex += This->ActiveMatrices[i].Rows;
        }
        break;
    }
    case 0xF:
        This->RejectionPenalty = ((s16 *)CParam1)[1];
        This->AnySpeechPenalty = ((s16 *)CParam1)[0];
        if (This->pErgodicPenalty != NULL) {
            UpdateRejectionAnySpeech(This);
        }
        break;
    case 0xD:
        ((s16 *)CParam1)[1] = This->RejectionPenalty;
        ((s16 *)CParam1)[0] = This->AnySpeechPenalty;
        break;
    case 0x2:
        RetVal = !SilAdaptEnable(This, 1);
        break;
    case 0x3:
        RetVal = !SilAdaptEnable(This, 0);
        break;
    case 0x16:
        ResetSilenceAdaptation(This);
        break;
    case 0x19:
        while (_This->InputQueues[2].pQ != NULL
                   ? qDeQueueOne(_This->InputQueues[2].pQ, _This->InputQueues[2].ReadPos)
                   : 0) {
        }
        break;
    case 0x1B:
    {
        struct _DISTCALCFUNCS *pFuncs = (struct _DISTCALCFUNCS *)CParam1;

        pFuncs->GetpCachedDistances = GetpCachedDistances;
        pFuncs->GetDistanceOffset = GetDistanceOffset;
        pFuncs->InitCalculateDistances = InitCalculateDistances;
        if (This->DistCalc == DistCalc4Bit) {
            pFuncs->CalculateDistance = CalculateDistance4Bit;
        } else {
            pFuncs->CalculateDistance = CalculateDistance;
        }
        pFuncs->pThis = This;
        break;
    }
    case 0xFF:
        RetVal = !FreeDistCalc(This);
        break;
    default:
        RetVal = 0;
        break;
    }
    return RetVal;
}

/* Sums byte distances across the codebooks for one phenome. */
static signed short CalculateDistance(void *pThis, unsigned int Phenom)
{
    DISTSIL *This = pThis;
    s32 NbrCodeBooks = This->NbrCodeBooks;
    u16 *GasPtr = This->GasPtr + Phenom * NbrCodeBooks;
    u8 **Ptr = This->Ptrs;
    u8 **pStop = Ptr + NbrCodeBooks;
    s32 Dist = 0;

    do {
        u16 Gastone = *GasPtr++;

        Dist += (*Ptr++)[Gastone];
    } while (Ptr < pStop);
    return Dist - This->sOffset;
}

/* Sums four-bit distances across the codebooks for one phenome. */
static signed short CalculateDistance4Bit(void *pThis, unsigned int Phenom)
{
    DISTSIL *This = pThis;
    s32 NbrCodeBooks = This->NbrCodeBooks;
    u16 *GasPtr = This->GasPtr + Phenom * NbrCodeBooks;
    u8 **Ptr = This->Ptrs;
    u8 **pStop = Ptr + NbrCodeBooks;
    s32 Dist = 0;
    u8 DistPair;

    do {
        u32 Gastone = *GasPtr++;
        u8 *pCodebookRow = *Ptr++;

        DistPair = pCodebookRow[Gastone >> 1];
        Dist += (DistPair << ((Gastone * 4) & 4)) & 0xF0;
    } while (Ptr < pStop);
    return Dist - This->sOffset;
}

/* Creates the distance block with the queues selected by the profile. */
struct _BLOCK *ConstructDistCalc(struct _TOS *pTos, u8 BlockID)
{
    struct _OBJECT Object;
    u8 NrInputs;
    u8 NrOutputs;
    struct _BLOCK *pBlock;

    Object.pTos = pTos;
    Object.BlockID = BlockID;
    if ((u8)_tosGetProfileU32(&Object, 0x23, 0) != 0) {
        NrInputs = 0;
        NrOutputs = 0;
    } else if ((u8)_tosGetProfileU32(&Object, 0x22, 1) != 0) {
        NrOutputs = 1;
        NrInputs = 3;
    } else {
        NrOutputs = 1;
        NrInputs = 2;
    }
    pBlock = tosBaseBlockConstruct(pTos, BlockID, NrInputs, NrOutputs,
        IdleDistCalcProcess, InitDistSilProcess,
        ControlDistSilProcess, sizeof(DISTSIL));
    if (pBlock != NULL) {
        DISTSIL *This = (DISTSIL *)pBlock;

        This->SIMatrices = NULL;
        This->SDMatrices = NULL;
        This->ActiveMatrices = NULL;
        This->ActiveGastoneMatrix = 0;
    }
    return pBlock;
}

/* Returns the cached distance table owned by the block. */
static signed short *GetpCachedDistances(void *pThis)
{
    DISTSIL *This = pThis;

    return This->pCachedDistances;
}

/* Returns the distance baseline subtracted from each matrix result. */
static long GetDistanceOffset(void *pThis)
{
    DISTSIL *This = pThis;

    return This->sOffset;
}

/* Selects probability rows and prepares the cached ergodic distances. */
static void InitCalculateDistances(void *pThis, s16 *pDistances,
    u8 *pLabels, unsigned char Speech, int sOffset)
{
    DISTSIL *This = pThis;
    unsigned int i;
    unsigned int NbrOfLabels = This->NbrCodeBooks;
    int AdaptSilDist = 0;
    int UpdateHisto;
    int Adapt = 0;
    DistanceMatrix *pMat = This->SIMatrices;
    u16 *pErgodicPhenom;
    u16 *pErgodicPhenomEnd;
    u16 *pErgodicDist = This->pErgodicDist;
    u16 *GastoneMatrix = This->ActiveGastoneMatrix->Matrix;
    unsigned int NbrErgodicPhenoms = This->NbrErgodicPhenoms;
    u16 *pState;
    u16 *pStateEnd;
    u16 *GasPtr;
    s16 *pErgodicPenalty = This->pLocalErgodicPenalty;
    int MaxDist = NbrOfLabels * 0xFF;
    u8 *pLab = pLabels;
    u8 **pStop;
    u8 **pStart;
    unsigned int Shift;
    unsigned int Devide;
    int Mask;
    u8 *pAdaptSil;

    if (This->SilAdaptOn != 0 && This->NeverAdapt == 0) {
        UpdateHisto = 1 - Speech;
    } else {
        UpdateHisto = 0;
    }
    if (UpdateHisto != 0) {
        if (++This->Frame % This->FramesToBulk == 0) {
            This->Frame = 0;
            Adapt = 1;
            This->Frame2++;
        }
    }
    for (i = 0; i < NbrOfLabels; i++) {
        pAdaptSil = This->ppAdaptSil[i];
        if (UpdateHisto != 0) {
            pMat->pHisto[*pLab]++;
        }
        if (Adapt != 0) {
            AdaptSilence(pMat, pAdaptSil, This->ppLookUp);
            if (This->Frame2 >= SILENCE_NORMALIZATION_BATCHES) {
                NormCorrection(This, pMat, pAdaptSil);
            }
        }
        This->Ptrs[i] = pMat->Matrix[*pLab];
        AdaptSilDist += pAdaptSil[*pLab];
        pLab++;
        pMat++;
    }
    pStop = This->Ptrs + NbrOfLabels - 1;
    pStart = This->Ptrs;
    if (This->DistCalc == DistCalc4Bit) {
        Shift = 4;
        Mask = 0xF0;
        Devide = 2;
    } else {
        Shift = 0;
        Mask = 0xFF;
        Devide = 1;
    }
    pState = This->pErgodicStates + This->pErgodicStates[0];
    for (i = 0; i < This->NbrErgodicStates; i++) {
        int Dist = MaxDist;

        pStateEnd = This->pErgodicStates + This->pErgodicStates[i + 1];
        while (pState < pStateEnd) {
            int Dst;

            if (*pState != This->AdaptSilState) {
                u8 **pTmp = pStart;
                unsigned int Gastone;

                GasPtr = GastoneMatrix + *pState * NbrOfLabels;
                Gastone = *GasPtr++;
                Dst = ((*pTmp++)[Gastone / Devide] << ((Gastone & 1) * Shift)) & Mask;
                while (pTmp <= pStop) {
                    Gastone = *GasPtr++;
                    Dst += ((*pTmp++)[Gastone / Devide] << ((Gastone & 1) * Shift)) & Mask;
                }
            } else {
                Dst = AdaptSilDist;
            }
            if (Dst < Dist) {
                Dist = Dst;
            }
            pState++;
        }
        pErgodicDist[i] = Dist;
    }
    pErgodicPhenom = This->pErgodicPhenoms + This->pErgodicPhenoms[0];
    for (i = 0; i < NbrErgodicPhenoms; i++) {
        int Dist;

        pErgodicPhenomEnd = This->pErgodicPhenoms + This->pErgodicPhenoms[i + 1];
        Dist = *pErgodicPenalty++ + pErgodicDist[*pErgodicPhenom++];
        while (pErgodicPhenom < pErgodicPhenomEnd) {
            int Dst = *pErgodicPenalty++ + pErgodicDist[*pErgodicPhenom];

            if (Dst < Dist) {
                Dist = Dst;
            }
            pErgodicPhenom++;
        }
        pDistances[i] = Dist - sOffset;
    }
    if (This->NeverAdapt == 0 && This->Frame2 >= SILENCE_NORMALIZATION_BATCHES) {
        This->Frame2 = 0;
    }
    This->sOffset = sOffset;
}
