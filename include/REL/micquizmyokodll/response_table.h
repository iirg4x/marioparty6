#ifndef REL_MICQUIZMYOKODLL_RESPONSE_TABLE_H
#define REL_MICQUIZMYOKODLL_RESPONSE_TABLE_H

typedef struct MicquizResponseEntry_s {
    int key;
    int flag;
    unsigned char unk_08[4]; /* No code names this field. */
    int state;
} MicquizResponseEntry;

#ifdef __cplusplus
extern "C" {
#endif

extern MicquizResponseEntry lbl_1_bss_4F0[8];

#ifdef __cplusplus
}
#endif

#endif
