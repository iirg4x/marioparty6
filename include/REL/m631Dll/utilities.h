/* Defines the callback job pool used for entrances, eliminations and effects. */
#ifndef REL_M631_UTILITIES_H
#define REL_M631_UTILITIES_H
#include "REL/m631Dll/m631_work.h"
typedef struct M631Job M631Job;
typedef struct M631Jobs {
    M631Job *freeHead; /* First available job, or NULL when the pool is full. */
    M631Job *activeHead; /* First scheduled job, or NULL when no jobs are active. */
    M631Job *storage; /* Beginning of the pool's job array. */
} M631Jobs;
struct M631Job {
    M631Jobs *owner; /* Pool to which this job returns. */
    M631Job *prev; /* Previous active job; NULL at the head or while free. */
    M631Job *next; /* Next active or free job, depending on its list. */
    s32 (*callback)(void *); /* Runs once per update; nonzero completes the job. */
    void *data; /* Actor or effect state passed to the callback. */
};
extern M631MotionPairNode *lbl_1_bss_10;
extern M631Job lbl_1_bss_14[256];
extern M631Jobs lbl_1_bss_1414;
#endif
