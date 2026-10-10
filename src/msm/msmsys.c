/* Coordinates MSM sound-file loading, group stacks, effects, and output mode. */
#include "dolphin/math.h"
#include "msm/msmsys.h"
#include "msm/msmfio.h"
#include "msm/msmmem.h"
#include "msm/msmmus.h"
#include "msm/msmse.h"
#include "msm/msmstream.h"

static MSM_SYS sys;

/* Each AI DMA completion updates MSM audio on the first installed call and every third call after
 * that, then chains the prior callback. */
static void msmSysServer(void)
{
    if (sndIsInstalled() == 1) {
        if (--sys.timer == 0) {
            sys.timer = 3;
            msmMusPeriodicProc();
            msmSePeriodicProc();
            msmStreamPeriodicProc();
        }
    }
    sys.oldAIDCallback();
}

/* Initialization and msmSysSetAux prepare effect callbacks here. Negative requests retain the
* current choice and slots marked MSM_AUXNO_NULL stay disabled. If both stored choices are negative,
* no callbacks are installed. Reverb preparation clears each copied tempDisableFX setting. */
static s32 msmSysSetAuxParam(s32 requestedAuxA, s32 requestedAuxB)
{
    s32 reservedStackWordsA[2];
    SND_AUX_CALLBACK auxCallbacks[2];
    s32 reservedStackWordsB[2];
    MSM_AUXPARAM *auxParameter;
    MSM_AUX *auxState;
    u32 prepareResult;
    s32 auxIndex;

    if (sys.auxParamNo[0] != MSM_AUXNO_NULL && requestedAuxA >= 0) {
        sys.auxParamNo[0] = requestedAuxA;
    }
    if (sys.auxParamNo[1] != MSM_AUXNO_NULL && requestedAuxB >= 0) {
        sys.auxParamNo[1] = requestedAuxB;
    }
    if (sys.auxParamNo[0] < 0 && sys.auxParamNo[1] < 0) {
        return 0;
    }
    for (auxIndex = 0; auxIndex < 2; auxIndex++) {
        if (sys.auxParamNo[auxIndex] < 0) {
            auxCallbacks[auxIndex] = NULL;
            continue;
        }
        auxParameter = &sys.auxParam[sys.auxParamNo[auxIndex]];
        auxState = &sys.aux[auxIndex];
        switch (auxParameter->type) {
            case MSM_AUX_REVERBHI:
                auxCallbacks[auxIndex] = sndAuxCallbackReverbHI;
                auxState->revHi.tempDisableFX = auxParameter->revHi.tempDisableFX;
                auxState->revHi.coloration = auxParameter->revHi.coloration;
                auxState->revHi.mix = auxParameter->revHi.mix;
                auxState->revHi.time = auxParameter->revHi.time;
                auxState->revHi.damping = auxParameter->revHi.damping;
                auxState->revHi.preDelay = auxParameter->revHi.preDelay;
                auxState->revHi.crosstalk = auxParameter->revHi.crosstalk;
                prepareResult = sndAuxCallbackPrepareReverbHI(&auxState->revHi);
                break;

            case MSM_AUX_REVERBSTD:
                auxCallbacks[auxIndex] = sndAuxCallbackReverbSTD;
                auxState->revStd.tempDisableFX = auxParameter->revStd.tempDisableFX;
                auxState->revStd.coloration = auxParameter->revStd.coloration;
                auxState->revStd.mix = auxParameter->revStd.mix;
                auxState->revStd.time = auxParameter->revStd.time;
                auxState->revStd.damping = auxParameter->revStd.damping;
                auxState->revStd.preDelay = auxParameter->revStd.preDelay;
                prepareResult = sndAuxCallbackPrepareReverbSTD(&auxState->revStd);
                break;

            case MSM_AUX_CHORUS:
                auxCallbacks[auxIndex] = sndAuxCallbackChorus;
                auxState->chorus.baseDelay = auxParameter->chorus.baseDelay;
                auxState->chorus.variation = auxParameter->chorus.variation;
                auxState->chorus.period = auxParameter->chorus.period;
                prepareResult = sndAuxCallbackPrepareChorus(&auxState->chorus);
                break;

            case MSM_AUX_DELAY:
                auxCallbacks[auxIndex] = sndAuxCallbackDelay;
                auxState->delay.delay[0] = auxParameter->delay.delay[0];
                auxState->delay.feedback[0] = auxParameter->delay.feedback[0];
                auxState->delay.output[0] = auxParameter->delay.output[0];
                auxState->delay.delay[1] = auxParameter->delay.delay[1];
                auxState->delay.feedback[1] = auxParameter->delay.feedback[1];
                auxState->delay.output[1] = auxParameter->delay.output[1];
                auxState->delay.delay[2] = auxParameter->delay.delay[2];
                auxState->delay.feedback[2] = auxParameter->delay.feedback[2];
                auxState->delay.output[2] = auxParameter->delay.output[2];
                prepareResult = sndAuxCallbackPrepareDelay(&auxState->delay);
                break;
        }
        if (prepareResult == FALSE) {
            /* The public setter treats this nonzero result as an invalid effect setting. */
            return TRUE;
        }
    }
    sndSetAuxProcessingCallbacks(0, auxCallbacks[0], &sys.aux[0], SND_MIDI_NONE, 0,
        auxCallbacks[1], &sys.aux[1], SND_MIDI_NONE, 0);
    return FALSE;
}

/* msmSysLoadGroup calls this for group index zero to load and register the full base set. */
static s32 msmSysLoadBaseGroup(void *sampleBuffer)
{
    DVDFileInfo file;
    s32 baseGroupIndex;
    MSM_GRP_HEAD *groupData;
    MSM_GRP_INFO *groupInfo;

    if (msmFioOpen(sys.msmEntryNum, &file) != TRUE) {
        return MSM_ERR_OPENFAIL;
    }
    for (baseGroupIndex = 0; baseGroupIndex < sys.baseGrpNum; baseGroupIndex++) {
        groupData = sys.grpData[baseGroupIndex];
        groupInfo = &sys.grpInfo[sys.info->baseGrp[baseGroupIndex]];
        if (msmFioRead(&file, groupData, groupInfo->dataSize,
            groupInfo->dataOfs + sys.header->grpDataOfs) < 0) {
            msmFioClose(&file);
            return MSM_ERR_READFAIL;
        }
        if (msmFioRead(&file, sampleBuffer, groupInfo->sampSize,
            groupInfo->sampOfs + sys.header->sampOfs) < 0) {
            msmFioClose(&file);
            return MSM_ERR_READFAIL;
        }
        if (!sndPushGroup((void*) (groupData->projOfs + (u32) groupData), groupInfo->gid,
            sampleBuffer, (void*) (groupData->sdirOfs + (u32) groupData),
            (void*) (groupData->poolOfs + (u32) groupData)))
        {
            msmFioClose(&file);
            return MSM_ERR_GRP_FAILPUSH;
        }
        sys.aramP += groupInfo->sampSize;
    }
    msmFioClose(&file);
    return 0;
}

/* Group loaders choose the last empty slot or encode the newest replaceable slot as -(index + 1).
* If no eligible slot is empty or replaceable, the returned replacement value is uninitialized. */
s32 msmSysSearchGroupStack(s32 groupIndex, s32 excludedSlot)
{
    MSM_GRP_STACK *groupStack;
    u32 loadSequence;
    s32 stackIndex;
    s32 newestSlotResult;
    s32 emptySlotIndex;
    s32 newestSequence;
    s32 stackCapacity;

    emptySlotIndex = -1;
    newestSequence = 0;
    if (sys.grpInfo[groupIndex].stackNo == 0) {
        groupStack = sys.grpStackA;
        stackCapacity = sys.grpStackAMax;
    } else {
        groupStack = sys.grpStackB;
        stackCapacity = sys.grpStackBMax;
    }
    for (stackIndex = 0; stackIndex < stackCapacity; groupStack++, stackIndex++) {
        if (stackIndex == excludedSlot) {
            continue;
        }
        if ((loadSequence = groupStack->num) != 0) {
            if (groupStack->baseGrpF == 0 && loadSequence > newestSequence) {
                newestSequence = loadSequence;
                newestSlotResult = -(stackIndex + 1);
            }
        } else {
            emptySlotIndex = stackIndex;
        }
    }
    return (emptySlotIndex < 0) ? newestSlotResult : emptySlotIndex;
}

/* msmSysInit calls this to read group metadata and allocate base and stack buffers. */
s32 msmSysGroupInit(DVDFileInfo *file)
{
    s32 groupIndex;
    MSM_GRP_STACK *groupStack;
    MSM_GRP_INFO *groupInfo;

    sys.grpMax = sys.info->grpMax;
    sys.grpLoadMode = MSM_GROUP_LOAD_MANUAL;
    sys.grpNum = 1;
    sys.baseGrpNum = sys.info->baseGrpNum;
    sys.grpStackAMax = sys.info->stackDepthA;
    sys.grpStackADepth = 0;
    sys.grpStackAOfs = 0;
    sys.grpStackBMax = sys.info->stackDepthB;
    sys.grpStackBDepth = 0;
    sys.grpStackBOfs = 0;
    sys.grpLoadNum = 0;
    if ((sys.grpInfo = msmMemAlloc(sys.header->grpInfoSize)) == NULL) {
        return MSM_ERR_OUTOFMEM;
    }
    if (msmFioRead(file, sys.grpInfo, sys.header->grpInfoSize, sys.header->grpInfoOfs) < 0) {
        return MSM_ERR_READFAIL;
    }
    if ((sys.grpBufA = msmMemAlloc(sys.info->grpBufSizeA * sys.grpStackAMax)) == NULL) {
        return MSM_ERR_OUTOFMEM;
    }
    if ((sys.grpBufB = msmMemAlloc(sys.info->grpBufSizeB * sys.grpStackBMax)) == NULL) {
        return MSM_ERR_OUTOFMEM;
    }
    if (sys.header->grpSetSize) {
        if ((sys.grpSet = msmMemAlloc(sys.header->grpSetSize)) == NULL) {
            return MSM_ERR_OUTOFMEM;
        }
        if (msmFioRead(file, sys.grpSet, sys.header->grpSetSize, sys.header->grpSetOfs) < 0) {
            return MSM_ERR_READFAIL;
        }
    } else {
        sys.grpSet = NULL;
    }
    for (groupIndex = 0; groupIndex < sys.grpStackAMax; groupIndex++) {
        groupStack = &sys.grpStackA[groupIndex];
        groupStack->grpId = groupStack->baseGrpF = 0;
        groupStack->num = 0;
        groupStack->buf = (void*) ((u32) sys.grpBufA + sys.info->grpBufSizeA * groupIndex);
    }
    for (groupIndex = 0; groupIndex < sys.grpStackBMax; groupIndex++) {
        groupStack = &sys.grpStackB[groupIndex];
        groupStack->grpId = groupStack->baseGrpF = 0;
        groupStack->num = 0;
        groupStack->buf = (void*) ((u32) sys.grpBufB + sys.info->grpBufSizeB * groupIndex);
    }
    sys.sampSize = 0;
    for (groupIndex = 0; groupIndex < sys.baseGrpNum; groupIndex++) {
        groupInfo = &sys.grpInfo[sys.info->baseGrp[groupIndex]];
        if ((sys.grpData[groupIndex] = msmMemAlloc(groupInfo->dataSize)) == NULL) {
            return MSM_ERR_OUTOFMEM;
        }
        if (sys.sampSize < groupInfo->sampSize) {
            sys.sampSize = groupInfo->sampSize;
        }
        /* A negative size marks base-group entries for the pass below. */
        groupInfo->sampSize *= -1;
    }
    sys.sampSizeBase = 0;
    for (groupIndex = 1; groupIndex < sys.grpMax; groupIndex++) {
        groupInfo = &sys.grpInfo[groupIndex];
        if (groupInfo->sampSize < 0) {
            groupInfo->sampSize *= -1;
        } else if (sys.sampSizeBase < groupInfo->sampSize) {
            sys.sampSizeBase = groupInfo->sampSize;
        }
    }
    return 0;
}

/* Stream operations use this for nested critical sections; only the first entry disables IRQs. */
void msmSysIrqDisable(void)
{
    if (sys.irqDepth++ == 0) {
        sys.irqState = OSDisableInterrupts();
    }
}

/* Stream operations use this to restore IRQs when the final nested critical section exits. */
void msmSysIrqEnable(void)
{
    if (sys.irqDepth != 0) {
        if (--sys.irqDepth == 0) {
            OSRestoreInterrupts(sys.irqState);
        }
    }
}

/* msmSysLoadGroupBase uses this to avoid adding a table index already in the base list. */
static inline BOOL msmSysCheckBaseGroupNo(s32 groupIndex)
{
    s32 baseGroupIndex;

    for (baseGroupIndex = 0; baseGroupIndex < sys.baseGrpNum + sys.grpStackAOfs +
        sys.grpStackBOfs; baseGroupIndex++) {
        if (sys.info->baseGrp[baseGroupIndex] == groupIndex) {
            return TRUE;
        }
    }
    return FALSE;
}

/* Music and sound-effect updates use this to keep active base-group sounds classified. */
BOOL msmSysCheckBaseGroup(s32 soundGroupId)
{
    s32 baseGroupIndex;

    for (baseGroupIndex = 0; baseGroupIndex < sys.baseGrpNum + sys.grpStackAOfs +
        sys.grpStackBOfs; baseGroupIndex++) {
        if (sys.grpInfo[sys.info->baseGrp[baseGroupIndex]].gid == soundGroupId) {
            return TRUE;
        }
    }
    return FALSE;
}

/* Music playback uses this to find the loaded data buffer for a song's group-table index. */
void *msmSysGetGroupDataPtr(s32 requestedGroupIndex)
{
    MSM_GRP_STACK *groupStack;
    s32 groupIndex;

    for (groupIndex = 0; groupIndex < sys.baseGrpNum; groupIndex++) {
        if (sys.info->baseGrp[groupIndex] == requestedGroupIndex) {
            return sys.grpData[groupIndex];
        }
    }
    for (groupIndex = 0; groupIndex < sys.grpStackAMax; groupIndex++) {
        groupStack = &sys.grpStackA[groupIndex];
        if (groupStack->num != 0 && groupStack->grpId == requestedGroupIndex) {
            return groupStack->buf;
        }
    }
    for (groupIndex = 0; groupIndex < sys.grpStackBMax; groupIndex++) {
        groupStack = &sys.grpStackB[groupIndex];
        if (groupStack->num != 0 && groupStack->grpId == requestedGroupIndex) {
            return groupStack->buf;
        }
    }
    return NULL;
}

/* Music playback and group loading use this to check whether a sound library ID is loaded. */
BOOL msmSysCheckLoadGroupID(s32 soundGroupId)
{
    MSM_GRP_STACK *groupStack;
    s32 groupIndex;

    for (groupIndex = 0; groupIndex < sys.baseGrpNum + sys.grpStackAOfs +
        sys.grpStackBOfs; groupIndex++) {
        if (sys.grpInfo[sys.info->baseGrp[groupIndex]].gid == soundGroupId) {
            return TRUE;
        }
    }
    for (groupIndex = 0; groupIndex < sys.grpStackAMax; groupIndex++) {
        groupStack = &sys.grpStackA[groupIndex];
        if (groupStack->num != 0 && sys.grpInfo[groupStack->grpId].gid == soundGroupId) {
            return TRUE;
        }
    }
    for (groupIndex = 0; groupIndex < sys.grpStackBMax; groupIndex++) {
        groupStack = &sys.grpStackB[groupIndex];
        if (groupStack->num != 0 && sys.grpInfo[groupStack->grpId].gid == soundGroupId) {
            return TRUE;
        }
    }
    return FALSE;
}

void msmSysRegularProc(void)
{
}

s32 msmSysGetOutputMode(void)
{
    return sys.outputMode;
}

/* Audio setup and file-select apply modes here; unsupported surround stores stereo and returns 1.
* Other unrecognized modes select stereo in MusyX but pass the stored input mode to streaming and
* return 0. */
BOOL msmSysSetOutputMode(SND_OUTPUTMODE mode)
{
    SND_OUTPUTMODE outputMode;
    BOOL modeUnavailable;

    modeUnavailable = 0;
    sys.outputMode = mode;
    switch (mode) {
        case SND_OUTPUTMODE_MONO:
            outputMode = SND_OUTPUTMODE_MONO;
            break;
        case SND_OUTPUTMODE_SURROUND:
            if (sys.info->surroundF != 0) {
                outputMode = SND_OUTPUTMODE_SURROUND;
            } else {
                sys.outputMode = SND_OUTPUTMODE_STEREO;
                outputMode = SND_OUTPUTMODE_STEREO;
                modeUnavailable = 1;
            }
            break;
        case SND_OUTPUTMODE_STEREO:
        default:
            outputMode = SND_OUTPUTMODE_STEREO;
            break;
    }
    sndOutputMode(outputMode);
    msmStreamSetOutputMode(sys.outputMode);
    OSSetSoundMode((mode != SND_OUTPUTMODE_MONO) ? 1 : 0);
    return modeUnavailable;
}

/* The game audio manager calls this after choosing effects; it clears old callbacks and applies new
 * ones. */
s32 msmSysSetAux(s32 requestedAuxA, s32 requestedAuxB)
{
    s32 auxIndex;

    sndSetAuxProcessingCallbacks(0, NULL, NULL, 0, 0, NULL, NULL, 0, 0);
    for (auxIndex = 1; auxIndex >= 0; auxIndex--) {
        if (sys.auxParamNo[auxIndex] < 0) {
            continue;
        }
        switch (sys.auxParam[sys.auxParamNo[auxIndex]].type) {
            case MSM_AUX_REVERBHI:
                sndAuxCallbackShutdownReverbHI(&sys.aux[auxIndex].revHi);
                break;
            case MSM_AUX_REVERBSTD:
                sndAuxCallbackShutdownReverbSTD(&sys.aux[auxIndex].revStd);
                break;
            case MSM_AUX_CHORUS:
                sndAuxCallbackShutdownChorus(&sys.aux[auxIndex].chorus);
                break;
            case MSM_AUX_DELAY:
                sndAuxCallbackShutdownDelay(&sys.aux[auxIndex].delay);
                break;
            }
    }
    if (msmSysSetAuxParam(requestedAuxA, requestedAuxB) != 0) {
        return MSM_ERR_INVALID_AUXPARAM;
    }
    return 0;
}

/* The audio manager passes a group index or zero: nonzero gets one-group space, zero the base
 * set. */
s32 msmSysGetSampSize(BOOL singleGroupRequest)
{
    if (singleGroupRequest != 0) {
        return sys.sampSizeBase;
    }
    return sys.sampSize;
}

/* Group-set changes call this to unload replaceable groups while preserving pinned groups. */
s32 msmSysDelGroupAll(void)
{
    MSM_GRP_STACK *groupStack;
    s32 stackIndex;

    for (stackIndex = 0; stackIndex < sys.grpStackBMax; stackIndex++) {
        groupStack = &sys.grpStackB[stackIndex];
        if (groupStack->num != 0 && groupStack->baseGrpF == 0) {
            groupStack->num = 0;
            sndPopGroup();
            sys.aramP -= sys.grpInfo[groupStack->grpId].sampSize;
            sys.grpLoadNum--;
            sys.grpStackBDepth--;
        }
    }
    for (stackIndex = 0; stackIndex < sys.grpStackAMax; stackIndex++) {
        groupStack = &sys.grpStackA[stackIndex];
        if (groupStack->num != 0 && groupStack->baseGrpF == 0) {
            groupStack->num = 0;
            sndPopGroup();
            sys.aramP -= sys.grpInfo[groupStack->grpId].sampSize;
            sys.grpLoadNum--;
            sys.grpStackADepth--;
        }
    }
    return 0;
}

/* Common-group changes use this to remove pinned groups. With none pinned it returns; otherwise a
 * nonzero count below the total first unloads replaceable groups, while zero or at least the total
 * clears both stacks. */
s32 msmSysDelGroupBase(s32 groupsToDelete)
{
    s32 stackIndex;
    s32 deletionIndex;
    s32 groupIndex;
    MSM_GRP_STACK *groupStack;

    if (sys.grpStackAOfs + sys.grpStackBOfs == 0) {
        return 0;
    }
    if (groupsToDelete >= sys.grpStackAOfs + sys.grpStackBOfs) {
        groupsToDelete = 0;
    }
    if (groupsToDelete != 0) {
        msmSysDelGroupAll();
        for (deletionIndex = 0; deletionIndex < groupsToDelete; deletionIndex++) {
            if (sys.grpLoadNum == 0) {
                break;
            }
            groupIndex = sys.grpLoadId[sys.grpLoadNum - 1];
            if (sys.grpInfo[groupIndex].stackNo == 0) {
                for (stackIndex = 0; stackIndex < sys.grpStackAMax; stackIndex++) {
                    MSM_GRP_STACK *groupSlot = &sys.grpStackA[stackIndex];
                    if (groupSlot->num != 0 && groupSlot->grpId == groupIndex) {
                        sndPopGroup();
                        sys.aramP -= sys.grpInfo[groupSlot->grpId].sampSize;
                        sys.grpLoadNum--;
                        groupSlot->num = 0;
                        groupSlot->baseGrpF = 0;
                        sys.grpStackADepth--;
                        sys.grpStackAOfs--;
                        break;
                    }
                }
            } else {
                for (stackIndex = 0; stackIndex < sys.grpStackBMax; stackIndex++) {
                    MSM_GRP_STACK *groupSlot = &sys.grpStackB[stackIndex];
                    if (groupSlot->num != 0 && groupSlot->grpId == groupIndex) {
                        sndPopGroup();
                        sys.aramP -= sys.grpInfo[groupSlot->grpId].sampSize;
                        sys.grpLoadNum--;
                        groupSlot->num = 0;
                        groupSlot->baseGrpF = 0;
                        sys.grpStackBDepth--;
                        sys.grpStackBOfs--;
                        break;
                    }
                }
            }
        }
    } else {
        for (stackIndex = 0; stackIndex < sys.grpStackAMax; stackIndex++) {
            groupStack = &sys.grpStackA[stackIndex];
            if (groupStack->num != 0) {
                sndPopGroup();
                sys.aramP -= sys.grpInfo[groupStack->grpId].sampSize;
                sys.grpLoadNum--;
                groupStack->baseGrpF = 0;
                groupStack->num = 0;
            }
        }
        for (stackIndex = 0; stackIndex < sys.grpStackBMax; stackIndex++) {
            groupStack = &sys.grpStackB[stackIndex];
            if (groupStack->num != 0) {
                sndPopGroup();
                sys.aramP -= sys.grpInfo[groupStack->grpId].sampSize;
                sys.grpLoadNum--;
                groupStack->baseGrpF = 0;
                groupStack->num = 0;
            }
        }
        sys.grpStackBOfs = 0;
        sys.grpStackBDepth = 0;
        sys.grpStackAOfs = 0;
        sys.grpStackADepth = 0;
    }
    return 0;
}

/* msmSysPushGroup calls this after reading a group to register its project and samples with
 * MusyX. */
static inline s32 msmSysAddGroup(void *sampleBuffer, MSM_GRP_STACK *groupSlot,
    MSM_GRP_INFO *groupInfo)
{
    s32 pushResult;
    MSM_GRP_HEAD *groupData;

    groupData = groupSlot->buf;
    if (!sndPushGroup((void*) (groupData->projOfs + (u32) groupData), groupInfo->gid,
        sampleBuffer, (void*) (groupData->sdirOfs + (u32) groupData),
        (void*) (groupData->poolOfs + (u32) groupData)))
    {
        pushResult = MSM_ERR_GRP_FAILPUSH;
    } else {
        pushResult = 0;
        sys.aramP += groupInfo->sampSize;
        sys.grpLoadId[sys.grpLoadNum++] = groupSlot->grpId;
    }
    return pushResult;
}

/* Group-load helpers call this to read one group's data and samples, then register it with
 * MusyX. */
static inline s32 msmSysPushGroup(DVDFileInfo *file, void *sampleBuffer,
    MSM_GRP_STACK *groupSlot, s32 groupIndex)
{
    s32 pushResult;
    MSM_GRP_INFO *groupInfo;

    groupInfo = &sys.grpInfo[groupIndex];
    if (msmFioRead(file, groupSlot->buf, groupInfo->dataSize,
        groupInfo->dataOfs + sys.header->grpDataOfs) < 0) {
        return MSM_ERR_READFAIL;
    }
    if (msmFioRead(file, sampleBuffer, groupInfo->sampSize,
        groupInfo->sampOfs + sys.header->sampOfs) < 0) {
        return MSM_ERR_READFAIL;
    }
    groupSlot->grpId = groupIndex;
    pushResult = msmSysAddGroup(sampleBuffer, groupSlot, groupInfo);
    if (pushResult != 0) {
        return pushResult;
    }
    groupSlot->num = sys.grpNum++;
    return 0;
}

/* The audio manager calls this when a common sound group must remain loaded as a base group. */
s32 msmSysLoadGroupBase(s32 groupIndex, void *sampleBuffer)
{
    s32 baseGroupSlot;
    s32 stackBank;
    s32 loadResult;
    s32 stackSlotIndex;
    MSM_GRP_STACK *groupSlot;
    u8 reservedGroupWorkspace[8];
    DVDFileInfo file;

    if (groupIndex < 1 || groupIndex >= sys.grpMax) {
        return MSM_ERR_64;
    }
    /* Ordinary groups are cleared before a group is added to the pinned list. */
    loadResult = msmSysDelGroupAll();
    if (loadResult != 0) {
        return loadResult;
    }
    baseGroupSlot = sys.baseGrpNum + sys.grpStackAOfs + sys.grpStackBOfs;
    if (msmSysCheckBaseGroupNo(groupIndex)) {
        return 0;
    }
    if (baseGroupSlot >= 15) {
        return MSM_ERR_STACK_OVERFLOW;
    }
    stackSlotIndex = msmSysSearchGroupStack(groupIndex, -1);
    if (stackSlotIndex < 0) {
        return MSM_ERR_STACK_OVERFLOW;
    }
    stackBank = sys.grpInfo[groupIndex].stackNo;
    if (!stackBank) {
        groupSlot = &sys.grpStackA[stackSlotIndex];
    } else {
        groupSlot = &sys.grpStackB[stackSlotIndex];
    }
    if (msmFioOpen(sys.msmEntryNum, &file) != 1) {
        return MSM_ERR_OPENFAIL;
    }
    loadResult = msmSysPushGroup(&file, sampleBuffer, groupSlot, groupIndex);
    if (loadResult != 0) {
        msmFioClose(&file);
        return loadResult;
    }
    msmFioClose(&file);
    sys.info->baseGrp[baseGroupSlot] = groupIndex;
    groupSlot->baseGrpF = 1;
    if (stackBank == 0) {
        sys.grpStackAOfs++;
        sys.grpStackADepth++;
    } else {
        sys.grpStackBOfs++;
        sys.grpStackBDepth++;
    }
    return 0;
}

/* Manual loads reload a dependency unless it is in the base list, then evict and replace stack
* entries as needed. Success returns the last displaced group-table index, or zero if none was
* displaced. */
static s32 msmSysLoadGroupSub(DVDFileInfo *file, s32 groupIndex, void *sampleBuffer)
{
    s32 replacedGroupIndex;
    s32 dependencyCheck;
    s32 stackSlotIndex;
    s32 loadResult;
    u8 *stackDepth;
    MSM_GRP_STACK *groupStack;
    MSM_GRP_INFO *groupInfo;
    replacedGroupIndex = 0;
    groupInfo = &sys.grpInfo[groupIndex];
    if (groupInfo->stackNo == 0)
    {
        groupStack = sys.grpStackA;
        stackDepth = &sys.grpStackADepth;
    }
    else
    {
        groupStack = sys.grpStackB;
        stackDepth = &sys.grpStackBDepth;
    }
    if (groupInfo->subGrpId != 0)
    {
        if (!msmSysCheckBaseGroup(sys.grpInfo[groupInfo->subGrpId].gid))
        {
            stackSlotIndex = -1;
            for (dependencyCheck = 0; dependencyCheck < 2; dependencyCheck++)
            {
                stackSlotIndex = msmSysSearchGroupStack(groupInfo->subGrpId, stackSlotIndex);
                if (0 > stackSlotIndex)
                {
                    stackSlotIndex = -(1 + stackSlotIndex);
                    (* stackDepth)--;
                    sndPopGroup();
                    sys.aramP -= sys.grpInfo[groupStack[stackSlotIndex].grpId].sampSize;
                    sys.grpLoadNum--;
                    replacedGroupIndex = groupStack[stackSlotIndex].grpId;
                    groupStack[stackSlotIndex].num = 0;
                }
            }
            loadResult = msmSysPushGroup(file, sampleBuffer, &groupStack[stackSlotIndex],
                groupInfo->subGrpId);
            if (loadResult != 0)
            {
                return loadResult;
            }
            (* stackDepth)++;
        }
    }
    stackSlotIndex = msmSysSearchGroupStack(groupIndex, -1);
    if (0 > stackSlotIndex)
    {
        stackSlotIndex = -(stackSlotIndex + 1);
        (* stackDepth)--;
        sndPopGroup();
        sys.aramP -= sys.grpInfo[groupStack[stackSlotIndex].grpId].sampSize;
        sys.grpLoadNum--;
        replacedGroupIndex = groupStack[stackSlotIndex].grpId;
    }
    loadResult = msmSysPushGroup(file, sampleBuffer, &groupStack[stackSlotIndex], groupIndex);
    if (loadResult == 0)
    {
        loadResult = replacedGroupIndex;
    }
    /* The stack depth advances even if the requested group's push failed. */
    (* stackDepth)++;
    return loadResult;
}

/* A-stack reloads call this before rebuilding B-stack entries in the MusyX group stack. */
static inline void msmSysPopGroup(s32 stackIndex)
{
    MSM_GRP_STACK *groupStack;

    groupStack = &sys.grpStackB[stackIndex];
    if (groupStack->num != 0 && groupStack->baseGrpF == 0) {
        sndPopGroup();
        sys.aramP -= sys.grpInfo[groupStack->grpId].sampSize;
        sys.grpLoadNum--;
    }
}

/* Audio requests call this to load a group on its stack; a NULL sample buffer returns success
 * without loading, and the legacy flag argument is ignored. */
s32 msmSysLoadGroup(s32 groupIndex, void *sampleBuffer, BOOL callerFlag)
{
    MSM_GRP_STACK *groupStack;
    MSM_GRP_INFO *groupInfo;
    s32 pushResult;
    s32 stackIndex;
    s32 manualLoadResult;
    DVDFileInfo file;

    if (sampleBuffer == NULL) {
        return 0;
    }
    if (groupIndex == 0) {
        return msmSysLoadBaseGroup(sampleBuffer);
    }
    groupInfo = &sys.grpInfo[groupIndex];
    if (msmSysCheckLoadGroupID(groupInfo->gid)) {
        return 0;
    }
    if (msmFioOpen(sys.msmEntryNum, &file) != TRUE) {
        return MSM_ERR_OPENFAIL;
    }
    if (sys.grpLoadMode != MSM_GROUP_LOAD_MANUAL) {
        MSM_GRP_STACK *groupStackA;
        s32 loadResult;

        loadResult = MSM_ERR_STACK_OVERFLOW;
        if (groupInfo->stackNo == 0) {
            for (stackIndex = 0; stackIndex < sys.grpStackAMax; stackIndex++) {
                groupStackA = &sys.grpStackA[stackIndex];
                if (groupStackA->num == 0) {
                    pushResult = msmSysPushGroup(&file, sampleBuffer, groupStackA, groupIndex);
                    loadResult = pushResult;
                    if (pushResult == 0) {
                        sys.grpStackADepth++;
                    }
                    break;
                }
            }
        } else {
            for (stackIndex = 0; stackIndex < sys.grpStackBMax; stackIndex++) {
                groupStack = &sys.grpStackB[stackIndex];
                if (groupStack->num == 0) {
                    pushResult = msmSysPushGroup(&file, sampleBuffer, groupStack, groupIndex);
                    loadResult = pushResult;
                    if (pushResult == 0) {
                        sys.grpStackBDepth++;
                    }
                    break;
                }
            }
        }
        msmFioClose(&file);
        return loadResult;
    }
    if (groupInfo->stackNo == 0) {
        /* Rebuild B-stack entries after changing A so sound-library pop order stays valid. */
        for (stackIndex = 0; stackIndex < sys.grpStackBMax; stackIndex++) {
            msmSysPopGroup(stackIndex);
        }
        manualLoadResult = msmSysLoadGroupSub(&file, groupIndex, sampleBuffer);
        for (stackIndex = 0; stackIndex < sys.grpStackBMax; stackIndex++) {
            groupStack = &sys.grpStackB[stackIndex];
            if (groupStack->num != 0 && groupStack->baseGrpF == 0) {
                pushResult = msmSysPushGroup(&file, sampleBuffer, groupStack, groupStack->grpId);
                if (pushResult != 0) {
                    msmFioClose(&file);
                    return pushResult;
                }
            }
        }
    } else {
        manualLoadResult = msmSysLoadGroupSub(&file, groupIndex, sampleBuffer);
    }
    msmFioClose(&file);
    return manualLoadResult;
}

/* Boot selects automatic free-slot loads or the manual stack-eviction path here. */
void msmSysSetGroupLoadMode(s32 loadMode)
{
    sys.grpLoadMode = loadMode;
}

/* Reset handling calls this to invoke sndIsInstalled; this wrapper discards its result. */
void msmSysCheckInit(void)
{
    sndIsInstalled();
}

/* Game audio startup loads MSM metadata, initializes sound systems, and installs the AI
 * callback. */
s32 msmSysInit(MSM_INIT *initParams, MSM_ARAM *aramConfig)
{
    s32 initResult;
    void *minimumHeapBlock;

    SND_HOOKS sndHooks = { msmMemAlloc, msmMemFree };
    DVDFileInfo file;
    if (sndIsInstalled() == 1) {
        return MSM_ERR_INSTALLED;
    }
    initResult = 0;
    sys.irqDepth = 0;
    msmMemInit(initParams->heap, initParams->heapSize);
    msmFioInit(initParams->open, initParams->read, initParams->close);
    sys.msmEntryNum = DVDConvertPathToEntrynum(initParams->msmPath);
    if (sys.msmEntryNum < 0) {
        return MSM_ERR_OPENFAIL;
    }
    if (msmFioOpen(sys.msmEntryNum, &file) != 1) {
        return MSM_ERR_OPENFAIL;
    }
    if ((sys.header = msmMemAlloc(96)) == NULL) {
        msmFioClose(&file);
        return MSM_ERR_OUTOFMEM;
    }
    if (msmFioRead(&file, sys.header, 96, 0) < 0) {
        msmFioClose(&file);
        return MSM_ERR_READFAIL;
    }
    if (sys.header->version != MSM_FILE_VERSION) {
        msmFioClose(&file);
        return MSM_ERR_INVALIDFILE;
    }
    if ((sys.info = msmMemAlloc(sys.header->infoSize)) == NULL) {
        msmFioClose(&file);
        return MSM_ERR_OUTOFMEM;
    }
    if (msmFioRead(&file, sys.info, sys.header->infoSize, sys.header->infoOfs) < 0) {
        msmFioClose(&file);
        return MSM_ERR_READFAIL;
    }
    if (aramConfig != NULL) {
        /* Reserve ARAM from the supplied stack settings, or check the caller's limit. The
         * skipARInit path still calls ARInit(NULL, 0) and ARQInit after that check. */
        if (aramConfig->skipARInit == 0) {
            ARInit(aramConfig->stackIndex, aramConfig->aramEnd);
            ARQInit();
            aramConfig = (MSM_ARAM *)ARAlloc(sys.info->aramSize);
            if ((u32)aramConfig != ARGetBaseAddress()) {
                msmFioClose(&file);
                return MSM_ERR_OUTOFAMEM;
            }
            sys.arInitF = FALSE;
        }
        else {
            if ((sys.info->aramSize + ARGetBaseAddress()) > aramConfig->aramEnd) {
                msmFioClose(&file);
                return MSM_ERR_OUTOFAMEM;
            }
            ARInit(NULL, 0);
            ARQInit();
            sys.arInitF = TRUE;
        }
    }
    initResult = msmSysGroupInit(&file);
    if (initResult != 0) {
        msmFioClose(&file);
        return initResult;
    }
    initResult = msmMusInit(&sys, &file);
    if (initResult != 0) {
        msmFioClose(&file);
        return initResult;
    }
    initResult = msmSeInit(&sys, &file);
    if (initResult != 0) {
        msmFioClose(&file);
        return initResult;
    }
    sys.auxParamNo[0] = sys.info->auxParamA == MSM_AUXNO_NULL ? MSM_AUXNO_NULL : MSM_AUXNO_UNSET;
    sys.auxParamNo[1] = sys.info->auxParamB == MSM_AUXNO_NULL ? MSM_AUXNO_NULL : MSM_AUXNO_UNSET;
    if ((s32)sys.header->auxParamSize == 0) {
        initResult = 0;
    }
    else {
        if ((sys.auxParam = msmMemAlloc(sys.header->auxParamSize)) == NULL) {
            initResult = MSM_ERR_OUTOFMEM;
        }
        else {
            if (msmFioRead(&file, sys.auxParam, sys.header->auxParamSize, sys.header->auxParamOfs) <
                0) {
                initResult = MSM_ERR_READFAIL;
            } else {
                initResult = 0;
            }
        }
    }
    if (initResult != 0) {
        msmFioClose(&file);
        return initResult;
    }
    msmFioClose(&file);
    initResult = msmStreamInit(initParams->pdtPath);
    if (initResult < 0) {
        return initResult;
    }
    AIInit(NULL);
    if (sys.info->surroundF == 2) {
        /* Only the file's value 2 enables MusyX's surround flag. */
        initResult = TRUE;
    } else {
        initResult = FALSE;
    }
    sndSetHooks(&sndHooks);
    if (sndInit(sys.info->voices, sys.info->music, sys.info->sfx, 1, initResult,
        sys.info->aramSize) != 0) {
        return MSM_ERR_INITFAIL;
    }
    /* msmSysServer chains this callback after its periodic audio updates. */
    sys.oldAIDCallback = AIRegisterDMACallback(msmSysServer);
    sys.timer = 1;
    initResult = msmStreamAmemAlloc();
    if (initResult < 0) {
        sndQuit();
        return initResult;
    }
    /* The ARAM cursor starts 1280 bytes past the stream buffer size. */
    sys.aramP = initResult + 1280;
    if ((int)sys.info->minMem != 0) {
        /* When minMem is nonzero, startup allocates minMem + 256 bytes to check heap availability,
         * then frees the block. */
        minimumHeapBlock = msmMemAlloc(sys.info->minMem + 256);
        if (minimumHeapBlock == NULL) {
            msmStreamAmemFree();
            sndQuit();
            return MSM_ERR_OUTOFMEM;
        }
        msmMemFree(minimumHeapBlock);
    }
    if (msmSysSetAuxParam(sys.info->auxParamA, sys.info->auxParamB) != 0) {
        msmStreamAmemFree();
        sndQuit();
        return MSM_ERR_INVALID_AUXPARAM;
    }
    msmSysSetOutputMode(OSGetSoundMode() == 0 ? SND_OUTPUTMODE_MONO : SND_OUTPUTMODE_STEREO);
    /* Start standard music and effect volume groups at full volume. */
    sndVolume(127, 0, SND_ALL_VOLGROUPS);
    return 0;
}
