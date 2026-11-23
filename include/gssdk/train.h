#ifndef GSSDK_TRAIN_H
#define GSSDK_TRAIN_H

struct _ENGINE;

/* Allocates the count matrices and probability tables for user-word training. */
int TrainInit(struct _ENGINE *pEngine);

/* Releases the training count matrices and probability tables. */
void TrainFree(struct _ENGINE *pEngine);

/* Smooths label counts using the current codebook's smoothing coefficients. */
void TrainSmoothCounts(struct _ENGINE *pEngine);

/* Accumulates labels from an accepted utterance and transcription. */
void TrainCountsFromAcq(struct _ENGINE *pEngine, unsigned int UttCnt, unsigned int BestTransIdx);

/* Converts training counts to scaled negative log probabilities. */
void TrainProbsFromCounts(struct _ENGINE *pEngine, unsigned long);

/* Selects the phenoms with the smallest increase in entropy. */
void TrainSelectPhenom(struct _ENGINE *pEngine, unsigned long BestTransIdx);

/* Trains the models using the selected utterance transcription. */
void TrainModels(struct _ENGINE *pEngine, unsigned long BestTransIdx);

#endif
