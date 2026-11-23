/* User word training counts, probabilities and phoneme selection. */
#include "gssdk/mathusage.h"
#include "gssdk/engine.h"
#include "gssdk/heap.h"
#include "gssdk/train.h"

#include <string.h>

/* Lookup table for the logarithm of averaged probabilities. */
static const signed short log_p_average_[511] = {
    -0xEC, -0xEB, -0xEA, -0xE9, -0xE8, -0xE7, -0xE6, -0xE5,
    -0xE4, -0xE3, -0xE2, -0xE1, -0xE0, -0xDF, -0xDE, -0xDD,
    -0xDC, -0xDB, -0xDA, -0xD9, -0xD8, -0xD7, -0xD6, -0xD5,
    -0xD4, -0xD3, -0xD2, -0xD1, -0xD0, -0xCF, -0xCE, -0xCD,
    -0xCC, -0xCB, -0xCA, -0xC9, -0xC8, -0xC7, -0xC6, -0xC5,
    -0xC4, -0xC3, -0xC2, -0xC1, -0xC0, -0xBF, -0xBE, -0xBD,
    -0xBC, -0xBB, -0xBA, -0xB9, -0xB8, -0xB7, -0xB6, -0xB5,
    -0xB4, -0xB3, -0xB2, -0xB1, -0xB0, -0xAF, -0xAE, -0xAD,
    -0xAC, -0xAB, -0xAA, -0xA9, -0xA8, -0xA7, -0xA6, -0xA5,
    -0xA4, -0xA3, -0xA2, -0xA1, -0xA0, -0x9F, -0x9E, -0x9D,
    -0x9C, -0x9B, -0x9A, -0x99, -0x98, -0x97, -0x96, -0x95,
    -0x94, -0x93, -0x92, -0x91, -0x90, -0x8F, -0x8E, -0x8D,
    -0x8C, -0x8B, -0x8A, -0x89, -0x88, -0x87, -0x86, -0x85,
    -0x84, -0x83, -0x82, -0x81, -0x80, -0x7F, -0x7E, -0x7D,
    -0x7C, -0x7B, -0x7A, -0x79, -0x78, -0x77, -0x76, -0x75,
    -0x74, -0x73, -0x72, -0x71, -0x70, -0x6F, -0x6E, -0x6D,
    -0x6C, -0x6B, -0x6A, -0x69, -0x68, -0x67, -0x66, -0x65,
    -0x64, -0x63, -0x62, -0x61, -0x60, -0x5F, -0x5E, -0x5D,
    -0x5C, -0x5B, -0x5A, -0x59, -0x58, -0x57, -0x56, -0x55,
    -0x54, -0x54, -0x53, -0x52, -0x51, -0x50, -0x4F, -0x4E,
    -0x4D, -0x4C, -0x4B, -0x4A, -0x49, -0x48, -0x47, -0x46,
    -0x45, -0x44, -0x43, -0x42, -0x41, -0x40, -0x3F, -0x3E,
    -0x3D, -0x3C, -0x3B, -0x3B, -0x3A, -0x39, -0x38, -0x37,
    -0x36, -0x35, -0x34, -0x33, -0x32, -0x31, -0x30, -0x2F,
    -0x2F, -0x2E, -0x2D, -0x2C, -0x2B, -0x2A, -0x29, -0x28,
    -0x27, -0x27, -0x26, -0x25, -0x24, -0x23, -0x22, -0x21,
    -0x20, -0x20, -0x1F, -0x1E, -0x1D, -0x1C, -0x1B, -0x1B,
    -0x1A, -0x19, -0x18, -0x17, -0x17, -0x16, -0x15, -0x14,
    -0x14, -0x13, -0x12, -0x11, -0x11, -0x10, -0xF, -0xF,
    -0xE, -0xD, -0xC, -0xC, -0xB, -0xA, -0xA, -0x9,
    -0x9, -0x8, -0x7, -0x7, -0x6, -0x5, -0x5, -0x4,
    -0x4, -0x3, -0x3, -0x2, -0x2, -0x1, -0x1, 0x0,
    0x0, 0x1, 0x1, 0x2, 0x2, 0x3, 0x3, 0x4,
    0x4, 0x5, 0x5, 0x5, 0x6, 0x6, 0x6, 0x7,
    0x7, 0x8, 0x8, 0x8, 0x9, 0x9, 0x9, 0x9,
    0xA, 0xA, 0xA, 0xB, 0xB, 0xB, 0xB, 0xC,
    0xC, 0xC, 0xC, 0xD, 0xD, 0xD, 0xD, 0xD,
    0xE, 0xE, 0xE, 0xE, 0xE, 0xE, 0xF, 0xF,
    0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x12, 0x12, 0x12, 0x12,
    0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
    0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
    0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
    0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
};
/* Origin of the table indexed by signed probability differences. */
static const signed short *const global_log_p_average = log_p_average_ + 255;
/* Entropy contributions indexed by quantized probability. */
static const unsigned long p_Log_p[256] = {
    0x0, 0x8EE29, 0x1139E7, 0x18EBE0, 0x200C54, 0x26A31C, 0x2CB7B4, 0x32513A,
    0x377676, 0x3C2DDB, 0x407D8E, 0x446B67, 0x47FCF5, 0x4B3781, 0x4E2014, 0x50BB74,
    0x530E2E, 0x551C92, 0x56EABD, 0x587C93, 0x59D5C9, 0x5AF9E3, 0x5BEC37, 0x5CAFF1,
    0x5D4811, 0x5DB771, 0x5E00C6, 0x5E269E, 0x5E2B69, 0x5E1173, 0x5DDAE9, 0x5D89DC,
    0x5D203F, 0x5C9FED, 0x5C0AA2, 0x5B6207, 0x5AA7AC, 0x59DD08, 0x590381, 0x581C67,
    0x5728F7, 0x562A5C, 0x5521B0, 0x540FFC, 0x52F63A, 0x51D554, 0x50AE29, 0x4F8186,
    0x4E502F, 0x4D1ADB, 0x4BE234, 0x4AA6DC, 0x496967, 0x482A63, 0x46EA52, 0x45A9AE,
    0x4468E9, 0x43286E, 0x41E89E, 0x40A9D5, 0x3F6C67, 0x3E30A3, 0x3CF6D2, 0x3BBF35,
    0x3A8A0A, 0x395789, 0x3827E5, 0x36FB4E, 0x35D1EE, 0x34ABEC, 0x33896B, 0x326A89,
    0x314F63, 0x303811, 0x2F24AA, 0x2E1540, 0x2D09E4, 0x2C02A4, 0x2AFF8B, 0x2A00A2,
    0x2905F3, 0x280F81, 0x271D50, 0x262F64, 0x2545BC, 0x246059, 0x237F36, 0x22A253,
    0x21C9A9, 0x20F534, 0x2024EC, 0x1F58CC, 0x1E90C9, 0x1DCCDC, 0x1D0CFB, 0x1C511B,
    0x1B9932, 0x1AE534, 0x1A3515, 0x1988C9, 0x18E044, 0x183B78, 0x179A58, 0x16FCD7,
    0x1662E6, 0x15CC78, 0x15397F, 0x14A9EC, 0x141DB2, 0x1394C0, 0x130F0A, 0x128C81,
    0x120D16, 0x1190BA, 0x11175F, 0x10A0F7, 0x102D73, 0xFBCC4, 0xF4EDD, 0xEE3B0,
    0xE7B2E, 0xE1549, 0xDB1F5, 0xD5122, 0xCF2C3, 0xC96CC, 0xC3D2E, 0xBE5DD,
    0xB90CC, 0xB3DEF, 0xAED38, 0xA9E9B, 0xA520D, 0xA0781, 0x9BEEB, 0x97840,
    0x93374, 0x8F07C, 0x8AF4E, 0x86FDD, 0x83220, 0x7F60D, 0x7BB98, 0x782B8,
    0x74B63, 0x7158F, 0x6E133, 0x6AE46, 0x67CBE, 0x64C92, 0x61DBA, 0x5F02D,
    0x5C3E2, 0x598D2, 0x56EF5, 0x54642, 0x51EB2, 0x4F83D, 0x4D2DC, 0x4AE88,
    0x48B3A, 0x468EB, 0x44794, 0x4272F, 0x407B5, 0x3E921, 0x3CB6B, 0x3AE8E,
    0x39284, 0x37747, 0x35CD3, 0x34321, 0x32A2C, 0x311F0, 0x2FA67, 0x2E38B,
    0x2CD59, 0x2B7CC, 0x2A2E0, 0x28E8E, 0x27AD5, 0x267AF, 0x25518, 0x2430C,
    0x23187, 0x22087, 0x21005, 0x20001, 0x1F075, 0x1E15E, 0x1D2B9, 0x1C484,
    0x1B6BA, 0x1A958, 0x19C5C, 0x18FC3, 0x1838A, 0x177AE, 0x16C2C, 0x16103,
    0x1562F, 0x14BAD, 0x1417D, 0x1379A, 0x12E03, 0x124B6, 0x11BB1, 0x112F1,
    0x10A75, 0x1023A, 0xFA3F, 0xF282, 0xEB00, 0xE3B9, 0xDCAA, 0xD5D2,
    0xCF2F, 0xC8C0, 0xC282, 0xBC76, 0xB699, 0xB0E9, 0xAB66, 0xA60E,
    0xA0E0, 0x9BDA, 0x96FC, 0x9244, 0x8DB1, 0x8942, 0x84F5, 0x80CB,
    0x7CC2, 0x78D8, 0x750D, 0x7160, 0x6DD0, 0x6A5C, 0x6704, 0x63C6,
    0x60A2, 0x5D96, 0x5AA3, 0x57C8, 0x5502, 0x5253, 0x4FBA, 0x4D35,
    0x4AC4, 0x4866, 0x461C, 0x43E3, 0x41BD, 0x3FA7, 0x3DA3, 0x3BAE,
};

/* Smooths each model state using its configured count filter. */
void TrainSmoothCounts(struct _ENGINE *pEngine)
{
    struct _COUNTMATRIX *Counts;
    unsigned int NbrStates = pEngine->NbrStates;
    unsigned int NbrLabel;
    unsigned int St;
    int CdbNbr = 0;
    const float *pSmoother;
    float *pSmoothedState;
    unsigned short *pTmpCounts;
    unsigned long SumCnt;
    unsigned int i;
    unsigned int j;

    for (Counts = pEngine->Counts; Counts != 0; Counts = Counts->Next, CdbNbr++) {
        NbrLabel = Counts->CountsSize / NbrStates;
        for (St = 0; St < NbrStates; St++) {
            pSmoother = pEngine->ppSmoother[CdbNbr];
            pSmoothedState = pEngine->pSmoothedState;
            SumCnt = 0;
            for (i = 0; i < NbrLabel; i++) {
                pTmpCounts = Counts->Counts + St;
                *pSmoothedState = 0;
                for (j = 0; j < NbrLabel; j++) {
                    *pSmoothedState += *pSmoother++ * *pTmpCounts;
                    pTmpCounts += NbrStates;
                }
                pSmoothedState++;
            }
            pSmoothedState = pEngine->pSmoothedState;
            pTmpCounts = Counts->Counts + St;
            for (i = 0; i < NbrLabel; i++) {
                *pTmpCounts = 0.5 + *pSmoothedState++;
                SumCnt += *pTmpCounts;
                pTmpCounts += NbrStates;
            }
            Counts->SumCounts[St] = SumCnt;
        }
    }
}

/* Converts observation counts to negative log probabilities scaled by 255 over MaxDist. */
void TrainProbsFromCounts(struct _ENGINE *pEngine, unsigned long)
{
    unsigned int NbrTrainStates = pEngine->NbrStates;
    struct _COUNTMATRIX *Counts = pEngine->Counts;
    float Max = pEngine->MaxDist;
    float Eps = pEngine->Epsilon;

    while (Counts != 0) {
        unsigned int NbrLabel = Counts->CountsSize / NbrTrainStates;
        float NxEps_1 = 1.0f + (Eps * (float)NbrLabel);
        unsigned int St;

        for (St = 0; St < NbrTrainStates; St++) {
            unsigned short *pCnt = Counts->Counts + St;
            unsigned int i;
            float SumCnt = (float)Counts->SumCounts[St];
            float Sum = 0.0f;
            float *lpProb = pEngine->TmpProbs;

            for (i = 0; i < NbrLabel; i++) {
                float Pcal = (Eps + ((float)*pCnt / SumCnt)) /
                    NxEps_1;

                *lpProb++ = Pcal;
                Sum += Pcal;
                pCnt += NbrTrainStates;
            }
            pCnt = Counts->Counts + St;
            lpProb = pEngine->TmpProbs;
            for (i = 0; i < NbrLabel; i++) {
                *pCnt = (short)(0.5 +
                    ((255.0 * -logf_check(*lpProb++ / Sum)) /
                        (double)Max));
                pCnt += NbrTrainStates;
            }
        }
        Counts = Counts->Next;
    }
}

/* Accumulates segmented utterance labels into model counts. */
void TrainCountsFromAcq(struct _ENGINE *pEngine, unsigned int UttCnt, unsigned int BestTransIdx)
{
    struct _COUNTMATRIX *Counts;
    unsigned long NbrStates;
    unsigned long St;
    unsigned short Inc;
    unsigned short LabelOffset;
    unsigned short *pSegmentation;
    long Time;
    unsigned char *pLabel;
    unsigned short *CtBase;
    unsigned short *SumCtBase;
    unsigned short *WrCount;

    LabelOffset = 0;
    Inc = pEngine->LabelOutQueue.pQ->Size;
    Counts = pEngine->Counts;
    NbrStates = pEngine->NbrStates;
    pSegmentation = pEngine->ppUserwordSegmentation[UttCnt + BestTransIdx * 3];
    for (; Counts != 0; Counts = Counts->Next, LabelOffset++) {
        Time = pSegmentation[0] - 1;
        pLabel = pEngine->UtteranceLabels[UttCnt] + Time * Inc + LabelOffset;
        CtBase = Counts->Counts;
        SumCtBase = Counts->SumCounts;
        for (St = 0; St < NbrStates; St++) {
            WrCount = CtBase + St;
            while (Time >= pSegmentation[St + 1]) {
                unsigned char Label = *pLabel;
                pLabel -= Inc;
                Time--;
                WrCount[Label * NbrStates] += 50;
                SumCtBase[St] += 50;
            }
        }
    }
}

/* Allocates the training count matrices and temporary probability tables. */
int TrainInit(struct _ENGINE *pEngine)
{
    struct _LANGUAGE *pLang = pEngine->pLang;
    struct _COUNTMATRIX **NextCount = &pEngine->Counts;
    unsigned int NbrCdbks;
    unsigned int cdbk;

    *NextCount = 0;
    pEngine->MaxCodebookSize = 0;
    NbrCdbks = pLang->langGetNbrCodeBook(pLang);
    for (cdbk = 0; cdbk < NbrCdbks; cdbk++) {
        unsigned int NbrRows;
        struct _COUNTMATRIX *Current;
        void *pCdbk;
        int Size;

        pCdbk = pLang->langGetpCodeBook(pLang, cdbk);
        NbrRows = pLang->cdbGetSecondCdbSize(pCdbk);
        if (NbrRows > pEngine->MaxCodebookSize) {
            pEngine->MaxCodebookSize = NbrRows;
        }
        Current = heap_Alloc(pEngine->hHeap, sizeof(*Current));
        if (Current == 0) {
            return 1;
        }
        Current->Counts = 0;
        Current->SumCounts = 0;
        *NextCount = Current;
        Size = NbrRows * pEngine->NbrStates;
        Current->Counts = heap_Calloc(
            pEngine->hHeap, Size * sizeof(*Current->Counts), 1);
        if (Current->Counts == 0) {
            return 1;
        }
        Current->CountsSize = Size;
        Current->SumCounts = heap_Calloc(pEngine->hHeap,
            pEngine->NbrStates * sizeof(*Current->SumCounts), 1);
        if (Current->SumCounts == 0) {
            return 1;
        }
        Current->SumCountsSize = pEngine->NbrStates;
        Current->Next = 0;
        NextCount = &Current->Next;
    }
    {
        float Max = 0.0f;
        for (cdbk = 0; cdbk < NbrCdbks; cdbk++) {
            unsigned int N;
            float Tmp;
            void *pCdbk;

            pCdbk = pLang->langGetpCodeBook(pLang, cdbk);
            N = pLang->cdbGetSecondCdbSize(pCdbk);
            Tmp = -logf_check(pEngine->Epsilon /
                (1.0f + ((float)N * pEngine->Epsilon)));
            if (Tmp > Max) {
                Max = Tmp;
            }
        }
        pEngine->MaxDist = Max;
    }
    pEngine->TmpProbs =
        heap_Alloc(pEngine->hHeap, pEngine->MaxCodebookSize * sizeof(*pEngine->TmpProbs));
    pEngine->pSmoothedState =
        heap_Calloc(pEngine->hHeap, pEngine->MaxCodebookSize, sizeof(*pEngine->pSmoothedState));
    if (pEngine->TmpProbs == 0 || pEngine->pSmoothedState == 0) {
        return 1;
    }
    return 0;
}

/* Sums the entropy contributions of packed four-bit probabilities. */
unsigned long CalcEntropy4bit(unsigned char *pProb, int Shift, unsigned long Nbr,
                              unsigned long Increment)
{
    unsigned long i;
    unsigned long Entropy = 0;

    Increment = (Increment + 1) / 2;
    if (Shift != 0) {
        for (i = 0; i < Nbr; i++) {
            int Prob = (*pProb << 4) & 0xF0;
            Entropy += p_Log_p[Prob];
            pProb += Increment;
        }
    } else {
        for (i = 0; i < Nbr; i++) {
            int Prob = *pProb & 0xF0;
            Entropy += p_Log_p[Prob];
            pProb += Increment;
        }
    }
    return Entropy;
}

/* Accumulates per-state entropy after merging the old and training probabilities. */
void CalcEntropyMerged(unsigned long *pEntropy, unsigned char *pOldProb, unsigned long Increment,
                       unsigned short *pNewProbs, unsigned long NbrTrainStates,
                       unsigned long NbrLabel)
{
    unsigned long i;
    unsigned long j;
    const short *log_p_average = global_log_p_average;
    unsigned long *pCurrent;

    for (i = 0; i < NbrLabel; i++) {
        int Prob1 = *pOldProb;
        pOldProb += Increment;
        log_p_average -= Prob1;
        pCurrent = pEntropy;
        for (j = 0; j < NbrTrainStates; j++) {
            *pCurrent++ += p_Log_p[Prob1 + log_p_average[*pNewProbs++]];
        }
        log_p_average += Prob1;
    }
}

/* Accumulates merged per-state entropy from packed old probabilities. */
void CalcEntropyMerged4bit(unsigned long *pEntropy, unsigned char *pOldProb,
                           int Shift, unsigned long Increment, unsigned short *pNewProbs,
                           unsigned long NbrTrainStates, unsigned long NbrLabel)
{
    unsigned long i;
    unsigned long j;
    const short *log_p_average = global_log_p_average;

    Increment = (Increment + 1) / 2;
    if (Shift != 0) {
        for (i = 0; i < NbrLabel; i++) {
            unsigned long *pCurrent = pEntropy;
            int Prob1 = (*pOldProb & 0xF) << 4;

            pOldProb += Increment;
            log_p_average -= Prob1;
            for (j = 0; j < NbrTrainStates; j++) {
                *pCurrent++ += p_Log_p[Prob1 + log_p_average[*pNewProbs++]];
            }
            log_p_average += Prob1;
        }
    } else {
        for (i = 0; i < NbrLabel; i++) {
            unsigned long *pCurrent = pEntropy;
            int Prob1 = *pOldProb & 0xF0;

            pOldProb += Increment;
            log_p_average -= Prob1;
            for (j = 0; j < NbrTrainStates; j++) {
                *pCurrent++ += p_Log_p[Prob1 + log_p_average[*pNewProbs++]];
            }
            log_p_average += Prob1;
        }
    }
}

/* Sums the entropy table entries for strided probabilities. */
unsigned long CalcEntropy(unsigned char *pProb, unsigned long Nbr, unsigned long Increment)
{
    unsigned long i;
    unsigned long Entropy = 0;

    for (i = 0; i < Nbr; i++) {
        Entropy += p_Log_p[*pProb];
        pProb += Increment;
    }
    return Entropy;
}

/* Selects the lowest entropy language states for the trained word. */
void TrainSelectPhenom(struct _ENGINE *pEngine, unsigned long BestTransIdx)
{
    struct _LANGUAGE *pLang = pEngine->pLang;
    unsigned long *pEntropyMergedStates;
    unsigned int NbrCodeBooks = pLang->langGetNbrCodeBook(pLang);
    unsigned short *pGastone = pLang->langGetpGastone(pLang);
    unsigned int NbrStates = pLang->langGetNbrState(pLang);
    unsigned long UWSilPhenome = pEngine->UserWordSilPhenom;
    int FourBitModels = pLang->langCheckBitField(pLang, 1);
    unsigned long PhenToStatOffset = pLang->langGetNbrErgodicPhenomes(pLang);
    unsigned long PhenomID;
    unsigned long NbrTrainedStates = pEngine->NbrStates;
    unsigned short *pBestMatchingPhenoms = pEngine->UserWordUtt[BestTransIdx].pPhenoms;

    pEntropyMergedStates =
        heap_Calloc(pEngine->hHeap, NbrTrainedStates * 2, sizeof(*pEntropyMergedStates));

    if (pEntropyMergedStates != 0) {
        unsigned int trainedState;
        unsigned int state;
        unsigned long *pEntropySmallestIncrease = pEntropyMergedStates + NbrTrainedStates;

        memset(pEntropySmallestIncrease, 0xFF,
               NbrTrainedStates * sizeof(*pEntropySmallestIncrease));
        PhenomID = PhenToStatOffset;
        for (state = 0; state < NbrStates; PhenomID++, state++) {
            struct _COUNTMATRIX *Counts = pEngine->Counts;
            unsigned short *pBestPhenom;
            unsigned int Entropy = 0;
            unsigned int codebook;

            for (codebook = 0; codebook < NbrCodeBooks; codebook++, pGastone++) {
                void *pCdbk = pLang->langGetpCodeBook(pLang, codebook);
                unsigned char *pProbMatrix = (unsigned char *)pLang->cdbGetpProbMatrix(pCdbk);
                int NbrGastones = pLang->cdbGetNbrGastone(pCdbk);
                unsigned int NbrLabel = pLang->cdbGetSecondCdbSize(pCdbk);

                if (FourBitModels != 0) {
                    unsigned char *pProb = pProbMatrix + *pGastone / 2;
                    int Shift = *pGastone & 1;

                    Entropy += CalcEntropy4bit(
                        pProb, Shift, NbrLabel, NbrGastones);
                    CalcEntropyMerged4bit(pEntropyMergedStates, pProb, Shift,
                                          NbrGastones, Counts->Counts, NbrTrainedStates, NbrLabel);
                } else {
                    unsigned char *pProb = pProbMatrix + *pGastone;
                    Entropy += CalcEntropy(pProb, NbrLabel, NbrGastones);
                    CalcEntropyMerged(pEntropyMergedStates, pProb, NbrGastones,
                                      Counts->Counts, NbrTrainedStates, NbrLabel);
                }
                Counts = Counts->Next;
            }

            Entropy >>= 1;
            pBestPhenom = pBestMatchingPhenoms + NbrTrainedStates - 1;
            for (trainedState = 0; trainedState < NbrTrainedStates; trainedState++, pBestPhenom--) {
                if (*pBestPhenom != UWSilPhenome) {
                    unsigned int Tmp = pEntropyMergedStates[trainedState] - Entropy;
                    if (Tmp < pEntropySmallestIncrease[trainedState]) {
                        pEntropySmallestIncrease[trainedState] = Tmp;
                        *pBestPhenom = PhenomID;
                    }
                }
                pEntropyMergedStates[trainedState] = 0;
            }
        }
        heap_Free(pEngine->hHeap, pEntropyMergedStates);
    }
}

/* Builds trained models and releases their temporary count storage. */
void TrainModels(struct _ENGINE *pEngine, unsigned long BestTransIdx)
{
    int transcriptionIndex = BestTransIdx;
    unsigned int UttCnt;
    struct _COUNTMATRIX *entry;
    struct _COUNTMATRIX *next;

    for (UttCnt = 0; UttCnt < pEngine->NbrUttForTrain; UttCnt++) {
        TrainCountsFromAcq(pEngine, UttCnt, transcriptionIndex);
    }
    if (pEngine->ppSmoother[0] != 0) {
        TrainSmoothCounts(pEngine);
    }
    TrainProbsFromCounts(pEngine, transcriptionIndex);
    TrainSelectPhenom(pEngine, transcriptionIndex);
    entry = pEngine->Counts;
    while (entry != 0) {
        next = entry->Next;
        heap_Free(pEngine->hHeap, entry->Counts);
        heap_Free(pEngine->hHeap, entry->SumCounts);
        heap_Free(pEngine->hHeap, entry);
        entry = next;
    }
    heap_Free(pEngine->hHeap, pEngine->TmpProbs);
    heap_Free(pEngine->hHeap, pEngine->pSmoothedState);
}
