#include "gssdk/gsapi_internal.h"
#include "gssdk/heap.h"

#include <stddef.h>
#include <string.h>

#define WRD_INVALID_ARGUMENT 0x80CC0080
#define WRD_OUT_OF_MEMORY 0x80CC0086

long WrdCreateWordList(void *hHeap, struct tag_WRD_WORDLIST *pWordList, void *pWrd)
{
    u32 i;
    char *word;

    if (pWordList == NULL) {
        return WRD_INVALID_ARGUMENT;
    }

    pWordList->u32NbrWords = ((const unsigned long *)pWrd)[1];
    pWordList->pWordStringList = heap_Calloc(hHeap, pWordList->u32NbrWords, sizeof(char *));
    if (pWordList->pWordStringList == NULL) {
        return WRD_OUT_OF_MEMORY;
    }

    word = (char *)pWrd + 0x8;
    for (i = 0; i < pWordList->u32NbrWords; i++) {
        pWordList->pWordStringList[i] = word;
        word += strlen(word) + 1;
    }
    return 0;
}

long WrdDestroyWordList(void *hHeap, struct tag_WRD_WORDLIST *pWordList)
{
    if (pWordList == NULL) {
        return 0;
    }
    if (pWordList->pWordStringList != NULL) {
        heap_Free(hHeap, pWordList->pWordStringList);
    }
    pWordList->pWordStringList = NULL;
    pWordList->u32NbrWords = 0;
    return 0;
}
