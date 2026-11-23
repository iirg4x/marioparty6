/* Copies the first available input feature vector to the output queue. */
#include "gssdk/blocks.h"
#include "types.h"

#include <string.h>

#include "gssdk/mqueue.h"
#include "gssdk/tos.h"

/* Input count and vector width for the feature stream selector. */
typedef struct {
    struct _BLOCK BaseBlock; /* Scheduler-owned block header. */
    u32 NrInputs;           /* Number of feature inputs considered by the selector. */
    u16 IODimension;        /* Width of the feature vector written to the output queue. */
} GenderFilter;

/* Copies the first available input into the output queue. */
static void ProcessGenderFilter(struct _BLOCK *_This, void *pInputs, int NbrInputs) {
    GenderFilter *This = (GenderFilter *) _This;
    s32 i;
    u8 *pIn;
    u8 *pOut;

    pIn = *(void **) pInputs;
    i = 1;
    while (pIn == NULL && i < NbrInputs) {
        pIn = ((void **) pInputs)[i];
        i++;
    }
    if (pIn != NULL) {
        pOut = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
        if (pOut != NULL) {
            memcpy(pOut, pIn, This->IODimension);
        }
    }
}

/* Releases the gender filter when it receives the destruction command. */
static u8 ControlGenderFilter(struct _BLOCK *_This, u8 ControlId, u32 CParam, u32 DParam) {
    switch (ControlId) {
    case 0xFF:
        tosBaseBlockDestruct(_This);
        break;
    default:
        return 0;
    }
    return 1;
}

/* Initializes the input and output settings for the gender filter. */
static u8 InitGenderFilter(struct _BLOCK *_This) {
    GenderFilter *This = (GenderFilter *) _This;
    u32 i;

    This->NrInputs = (u16) _tosGetProfileU32(&This->BaseBlock.Object, 1, 3);
    This->IODimension = _tosGetProfileU32(&This->BaseBlock.Object, 2, 4);
    for (i = 0; i < This->NrInputs; i++) {
        _This->InputQueues[i].Size = This->IODimension;
    }
    _This->OutputQueues->Size = This->IODimension;
    return 0;
}

/* Constructs a TOS block that selects a stream by gender. */
struct _BLOCK *ConstructGenderFilter(struct _TOS *pTos, u8 BlockID) {
    struct _OBJECT Object;

    Object.pTos = pTos;
    Object.BlockID = BlockID;
    return tosBaseBlockConstruct(pTos, BlockID, (u16) _tosGetProfileU32(&Object, 1, 3), 1,
                                 ProcessGenderFilter, InitGenderFilter, ControlGenderFilter,
                                 sizeof(GenderFilter));
}
