#ifndef MICQUIZISHIDLL_RECORD_VIEWS_V1_H
#define MICQUIZISHIDLL_RECORD_VIEWS_V1_H

#include "REL/micquizishidll/module_base.h"

typedef struct MicQuizRecord {
    s32 word00;
    s32 word04;
    s32 word08;
} MicQuizRecord;

typedef struct MicQuizRecordGroup {
    MicQuizRecord entries[4];
} MicQuizRecordGroup;

extern MicQuizRecordGroup lbl_1_bss_6E0[];

#endif
