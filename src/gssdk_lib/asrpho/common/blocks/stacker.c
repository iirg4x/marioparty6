/* Stacks consecutive input frames for downstream processing. */
#include "gssdk/blocks.h"
#include "gssdk/mqueue.h"
#include "gssdk/heap.h"
#include "types.h"

#include <string.h>

#include "gssdk/tos.h"

/* Combines selected input queue entries into one output entry. */
typedef struct {
    struct _BLOCK BaseBlock; /* Scheduler-owned block header. */
    f32 **pIn;              /* The game's code never reads this field. */
    u16 NbrOfInputs;         /* Number of configured input slots. */
    u16 *InputDimensions;    /* Number of bytes reserved for each input slot. */
    u8 *EnableStackerInput;  /* Whether each input contributes bytes to the output. */
} Stacker;

/* Packs the enabled inputs at their configured byte offsets. */
static void ProcessStacker(struct _BLOCK *_This, void *pInputs, int NbrInputs)
{
    Stacker *This = (Stacker *) _This;
    u16 i;
    u8 *pOut;

    for (i = 0; i < NbrInputs; i++) {
        if (((void **)pInputs)[i] != NULL) {
            break;
        }
    }
    if (i == NbrInputs) {
        return;
    }

    pOut = qEnQueueOne(This->BaseBlock.OutputQueues->pQ);
    for (i = 0; i < NbrInputs; i++) {
        if (This->EnableStackerInput[i] != 0) {
            memcpy(pOut, ((void **)pInputs)[i], This->InputDimensions[i]);
        }
        pOut += This->InputDimensions[i];
    }
}

/* Enables or disables an input, or destroys the block. */
static u8 ControlStacker(struct _BLOCK *_This, u8 ControlId, u32 CParam, u32 DParam)
{
    Stacker *This = (Stacker *) _This;

    switch (ControlId) {
    case 0x64:
        This->EnableStackerInput[0] = 0;
        break;
    case 0x65:
        This->EnableStackerInput[1] = 0;
        break;
    case 0x96:
        This->EnableStackerInput[0] = 1;
        break;
    case 0x97:
        This->EnableStackerInput[1] = 1;
        break;
    case 0xFF:
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->InputDimensions);
        heap_Free(This->BaseBlock.Object.pTos->hHeap, This->EnableStackerInput);
        tosBaseBlockDestruct(&This->BaseBlock);
        break;
    default:
        return 0;
    }
    return 1;
}

/* Reads the input widths and sets the total output width. */
static unsigned char InitStacker(struct _BLOCK *_This)
{
    Stacker *This = (Stacker *) _This;
    struct _TOS *pTos = _This->Object.pTos;
    u16 i;

    This->NbrOfInputs = _tosGetProfileU32(&This->BaseBlock.Object, 0x03, 2);
    This->InputDimensions = heap_Calloc(pTos->hHeap, This->NbrOfInputs, sizeof(u16));
    This->EnableStackerInput = heap_Calloc(pTos->hHeap, This->NbrOfInputs, sizeof(u8));
    if (This->InputDimensions == NULL || This->EnableStackerInput == NULL) {
        _tosErrorLog(&This->BaseBlock.Object, 0x02);
        return 1;
    }

    for (i = 0; i < This->NbrOfInputs; i++) {
        switch (i) {
        case 0:
            This->InputDimensions[0] = _tosGetProfileU32(&This->BaseBlock.Object, 0x01, 152);
            break;
        case 1:
            This->InputDimensions[1] = _tosGetProfileU32(&This->BaseBlock.Object, 0x02, 20);
            break;
        default:
            _tosErrorLog(&This->BaseBlock.Object, 0xC8);
            return 1;
        }
        This->EnableStackerInput[i] = 1;
    }

    for (i = 0; i < This->NbrOfInputs; i++) {
        This->BaseBlock.InputQueues[i].Size = This->InputDimensions[i];
        This->BaseBlock.OutputQueues->Size =
            This->BaseBlock.OutputQueues->Size + This->BaseBlock.InputQueues[i].Size;
    }
    return 0;
}

/* Allocates a block that stacks two input feature streams. */
struct _BLOCK *ConstructStacker(struct _TOS *pTos, u8 BlockID)
{
    return tosBaseBlockConstruct(pTos, BlockID, 2, 1, ProcessStacker, InitStacker,
                                 ControlStacker,
                                 sizeof(Stacker));
}
