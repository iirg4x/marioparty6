// Implements PowerPC exception lookup, stack unwinding, and transfer to a matching catch block.
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "PowerPC_EABI_Support/Runtime/Gecko_ExceptionPPC.h"
#include "PowerPC_EABI_Support/Runtime/NMWException.h"
#include "PowerPC_EABI_Support/Runtime/__ppc_eabi_linker.h"

#define RETURN_ADDRESS 4

union MWE_GeckoVector64 {
	f64 doubleValue; // The saved floating-point register value.
	f32 singlePrecisionLanes[2]; // The paired single-precision values saved by vector instructions.
};

typedef union MWE_GeckoVector64 MWE_GeckoVector64;

struct GeckoFPRContext {
	f64 doubleValue; // Saved scalar floating-point register value.
	MWE_GeckoVector64 vectorValue; // Saved paired values when the function uses vector registers.
};

typedef struct GeckoFPRContext GeckoFPRContext;

typedef struct ThrowContext {
	GeckoFPRContext floatingPointRegisters[32]; // Floating-point state restored when control
                                                // resumes.
	s32 generalPurposeRegisters[32]; // General-purpose state restored for the selected handler.
	s32 conditionRegister; // Condition-register state captured at the throw site.
	char* stackPointer; // Stack pointer for the frame currently being unwound.
	char* framePointer; // Frame pointer, or stack pointer for a frame without one.
	char* throwStackPointer; // Throw-site stack pointer used to restore the original frame.
	char* returnAddress; // Return address whose function exception record is being searched.
	char* exceptionType; // Encoded type information for the thrown object.
	void* objectLocation; // Address of the thrown object or subobject.
	void* destructor; // Destructor for the thrown object, or retained from the active catch during
                      // rethrow.
	CatchInfo* catchInfo; // Catch metadata selected during handler search.
} ThrowContext;

typedef ThrowContext* ThrowContextPtr;

typedef struct MWExceptionInfo {
	ExceptionTableSmall* exceptionRecord; // Exception table for the current function.
	char* currentFunction; // Start address of the function owning the table.
	char* actionPointer; // Current action record in the function's exception table.
	char* codeSection; // Base address used to resolve function offsets.
	char* dataSection; // Base address used for exception tables stored in data.
	char* toc; // Table of contents used when control jumps into the handler.
} MWExceptionInfo;

typedef struct FragmentInfo {
	ExceptionTableIndex* exceptionStart; // First function index in this fragment.
	ExceptionTableIndex* exceptionEnd; // One-past-last function index in this fragment.
	char* codeStart; // Code base for a statically linked fragment; null for registered fragments.
	char* codeEnd; // End of the code range when supplied by the fragment lookup.
	char* dataStart; // Data base for exception tables; null for registered fragments.
	char* dataEnd; // End of the data range when supplied by the fragment lookup.
	char* toc; // Table of contents associated with the fragment.
	int isActive; // Whether this fragment is registered and available to lookup.
} FragmentInfo;

typedef struct ProcessInfo {
	__eti_init_info* exceptionInfo; // Exception index supplied by the runtime linker.
	char* toc; // Table of contents for code in this registered fragment.
	int isActive; // Whether the single registration slot is occupied.
} ProcessInfo;

typedef struct ActionIterator {
	MWExceptionInfo exceptionInfo; // Function table and action currently being searched.
	char* stackPointer; // Stack pointer for the current frame in the search.
	char* framePointer; // Frame pointer used to resolve local and catch records.
	s32 savedR31; // Saved R31 value used when the frame has a frame pointer.
} ActionIterator;

#define MAXFRAGMENTS 1
static ProcessInfo fragmentinfo[MAXFRAGMENTS];

typedef void (*DeleteFunc)(void*);

// Called by the runtime linker when a dynamic code fragment is loaded; records its exception index
// and TOC.
int __register_fragment(struct __eti_init_info* exceptionInfo, char* tableOfContents)
{

	ProcessInfo* process = fragmentinfo;
	int fragmentIndex;

	for (fragmentIndex = 0; fragmentIndex < MAXFRAGMENTS; fragmentIndex++, process++) {
		if (process->isActive == 0) {
			process->exceptionInfo = exceptionInfo;
			process->toc           = tableOfContents;
			process->isActive      = 1;
			return fragmentIndex;
		}
	}

	return -1;
}

// Called by the runtime linker when a dynamic code fragment is unloaded; clears its stored
// exception-index and TOC references.
void __unregister_fragment(int fragmentId)
{
	ProcessInfo* process;

	if (fragmentId >= 0 && fragmentId < MAXFRAGMENTS) {
		process                 = &fragmentinfo[fragmentId];
		process->exceptionInfo = 0;
		process->toc           = 0;
		process->isActive      = 0;
	}
}

// Called during exception lookup; finds the registered fragment containing the return address.
static inline int ExPPC_FindExceptionFragment(char* returnAddress, FragmentInfo* fragment)
{
	ProcessInfo* process;
	int fragmentIndex;
	__eti_init_info* exceptionInfo;

        for (fragmentIndex = 0, process = fragmentinfo; fragmentIndex < MAXFRAGMENTS;
             ++fragmentIndex, ++process) {
            if (process->isActive) {
                exceptionInfo = process->exceptionInfo;
                while (1) {
                    if (exceptionInfo->code_size == 0)
                        break;
                    if (returnAddress >= exceptionInfo->code_start &&
                        returnAddress <
                            (char *) exceptionInfo->code_start + exceptionInfo->code_size) {
                        fragment->exceptionStart = (ExceptionTableIndex *) exceptionInfo->eti_start;
                        fragment->exceptionEnd = (ExceptionTableIndex *) exceptionInfo->eti_end;
                        fragment->codeStart = 0;
                        fragment->codeEnd = 0;
                        fragment->dataStart = 0;
                        fragment->dataEnd = 0;
                        fragment->toc = process->toc;
                        fragment->isActive = process->isActive;
                        return 1;
                    }
                    exceptionInfo++;
                }
            }
        }

        return 0;
}

// Called during handler search and unwinding; resolves a return address to its exception action.
static void ExPPC_FindExceptionRecord(char* returnAddress, MWExceptionInfo* info)
{
	FragmentInfo* fragment;
	FragmentInfo foundFragment;
	ExceptionTableIndex *exceptionIndex, *tableEntry;
	u32 returnOffset;
	s32 low, middle, high;

	info->exceptionRecord = 0;
	info->actionPointer   = 0;

	if ((ExPPC_FindExceptionFragment(returnAddress, &foundFragment)) == 0)
		return;
	fragment = &foundFragment;

	info->codeSection = fragment->codeStart;
	info->dataSection = fragment->dataStart;
	info->toc         = fragment->toc;

	returnOffset   = returnAddress - fragment->codeStart;
	exceptionIndex = fragment->exceptionStart;
	for (low = 0, high = fragment->exceptionEnd - fragment->exceptionStart;;) {
		if (low > high)
			return;
		tableEntry = &exceptionIndex[middle = (low + high) / 2];

		if (returnOffset < tableEntry->functionoffset) {
			high = middle - 1;
                } else if (returnOffset > tableEntry->functionoffset +
                                              ETI_GetFunctionSize(tableEntry->eti_field)) {
                    low = middle + 1;
                } else
                    break;
	}
	info->currentFunction = fragment->codeStart + tableEntry->functionoffset;
        info->exceptionRecord =
            ETI_GetDirectStore(tableEntry->eti_field)
                ? (ExceptionTableSmall *) (&tableEntry->exceptionoffset)
                : (ExceptionTableSmall *) (fragment->dataStart + tableEntry->exceptionoffset);

        returnOffset -= tableEntry->functionoffset;

	if (ET_IsLargeTable(info->exceptionRecord->et_field)) {
		ExceptionTableLarge* largeTable = (ExceptionTableLarge*)info->exceptionRecord;
		ExceptionRangeLarge* range;

		for (range = largeTable->ranges; range->start != 0; range++) {
			u32 rangeEnd = range->start + (range->size * 4);

			if (range->start <= returnOffset && rangeEnd >= returnOffset) {
				info->actionPointer = (char*)largeTable + range->action;
				break;
			}
		}
	} else {
		ExceptionTableSmall* smallTable = (ExceptionTableSmall*)info->exceptionRecord;
		ExceptionRangeSmall* range;

		for (range = smallTable->ranges; range->start != 0; range++) {
			if (range->start <= returnOffset && range->end >= returnOffset) {
				info->actionPointer = (char*)smallTable + range->action;
				break;
			}
		}
	}
}

// Called by ExPPC_NextAction when a frame saves R31; reads that value for frame-pointer recovery.
static inline s32 ExPPC_PopR31(char* stackPointer, MWExceptionInfo* info)
{
	f64* floatingPointSaveArea;
	s32* generalPurposeSaveArea;
	int savedGeneralPurposeRegisters, savedFloatingPointRegisters;

	savedFloatingPointRegisters = ET_GetSavedFPRs(info->exceptionRecord->et_field);
	floatingPointSaveArea       = (f64*)(stackPointer - savedFloatingPointRegisters * 8);
	savedGeneralPurposeRegisters = ET_GetSavedGPRs(info->exceptionRecord->et_field);
	generalPurposeSaveArea       = (s32*)floatingPointSaveArea;

	return generalPurposeSaveArea[-1];
}

// Called by ExPPC_ThrowHandler before each search step; returns the current action kind.
static inline exaction_type ExPPC_CurrentAction(const ActionIterator* iter)
{
	if (iter->exceptionInfo.actionPointer == 0) {
		return EXACTION_ENDOFLIST;
	}

	return ((ex_destroylocal*)iter->exceptionInfo.actionPointer)->action & EXACTION_MASK;
}

// Called by ExPPC_ThrowHandler after each action; advances records and moves to caller frames.
static exaction_type ExPPC_NextAction(ActionIterator* iter)
{
	exaction_type action;

	for (;;) {
            if (iter->exceptionInfo.actionPointer == 0 ||
                ((action = ((ex_destroylocal *) iter->exceptionInfo.actionPointer)->action) &
                 EXACTION_ENDBIT) != 0) {
                char *returnAddress, *callersStackPointer;

                callersStackPointer = *(char **) iter->stackPointer;

                if (ET_GetSavedGPRs(iter->exceptionInfo.exceptionRecord->et_field)) {
                    iter->savedR31 = ExPPC_PopR31(callersStackPointer, &iter->exceptionInfo);
                }

                returnAddress = *(char **) (callersStackPointer + RETURN_ADDRESS);

                ExPPC_FindExceptionRecord(returnAddress, &iter->exceptionInfo);

                if (iter->exceptionInfo.exceptionRecord == 0) {
                    terminate();
                }

                iter->stackPointer = callersStackPointer;
                iter->framePointer =
                    (ET_GetHasFramePtr(iter->exceptionInfo.exceptionRecord->et_field))
                        ? (char *) iter->savedR31
                        : iter->stackPointer;

                if (iter->exceptionInfo.actionPointer == 0)
                    continue;
            } else {
                switch (action) {
                case EXACTION_DESTROYLOCAL:
                    iter->exceptionInfo.actionPointer += sizeof(ex_destroylocal);
                    break;
                case EXACTION_DESTROYLOCALCOND:
                    iter->exceptionInfo.actionPointer += sizeof(ex_destroylocalcond);
                    break;
                case EXACTION_DESTROYLOCALPOINTER:
                    iter->exceptionInfo.actionPointer += sizeof(ex_destroylocalpointer);
                    break;
                case EXACTION_DESTROYLOCALARRAY:
                    iter->exceptionInfo.actionPointer += sizeof(ex_destroylocalarray);
                    break;
                case EXACTION_DESTROYBASE:
                case EXACTION_DESTROYMEMBER:
                    iter->exceptionInfo.actionPointer += sizeof(ex_destroymember);
                    break;
                case EXACTION_DESTROYMEMBERCOND:
                    iter->exceptionInfo.actionPointer += sizeof(ex_destroymembercond);
                    break;
                case EXACTION_DESTROYMEMBERARRAY:
                    iter->exceptionInfo.actionPointer += sizeof(ex_destroymemberarray);
                    break;
                case EXACTION_DELETEPOINTER:
                    iter->exceptionInfo.actionPointer += sizeof(ex_deletepointer);
                    break;
                case EXACTION_DELETEPOINTERCOND:
                    iter->exceptionInfo.actionPointer += sizeof(ex_deletepointercond);
                    break;
                case EXACTION_CATCHBLOCK:
                    iter->exceptionInfo.actionPointer += sizeof(ex_catchblock);
                    break;
                case EXACTION_CATCHBLOCK_32:
                    iter->exceptionInfo.actionPointer += sizeof(ex_catchblock_32);
                    break;
                case EXACTION_ACTIVECATCHBLOCK:
                    iter->exceptionInfo.actionPointer += sizeof(ex_activecatchblock);
                    break;
                case EXACTION_SPECIFICATION:
                    iter->exceptionInfo.actionPointer +=
                        sizeof(ex_specification) +
                        ((ex_specification *) iter->exceptionInfo.actionPointer)->specs *
                            sizeof(void *);
                    break;
                default:
                    terminate();
                }
            }

            action =
                ((ex_destroylocal *) iter->exceptionInfo.actionPointer)->action & EXACTION_MASK;

            if (action == EXACTION_BRANCH) {
                iter->exceptionInfo.actionPointer =
                    ((char *) iter->exceptionInfo.exceptionRecord) +
                    ((ex_branch *) iter->exceptionInfo.actionPointer)->target;
                action =
                    ((ex_destroylocal *) iter->exceptionInfo.actionPointer)->action & EXACTION_MASK;
                }
		return action;
	}
}

// Called by ExPPC_UnwindStack; copies saved registers into the throw context and returns the caller
// return address.
static char* ExPPC_PopStackFrame(ThrowContext* context, MWExceptionInfo* info)
{
	char *stackPointer, *callersStackPointer;
	f64* floatingPointSaveArea;
	s32* generalPurposeSaveArea;
	int savedGeneralPurposeRegisters, savedFloatingPointRegisters;
	GeckoFPRContext* vectorSaveArea;
	int registerIndex, saveIndex;

	stackPointer = context->stackPointer;
	callersStackPointer = *(char**)stackPointer;
	savedFloatingPointRegisters = ET_GetSavedFPRs(info->exceptionRecord->et_field);

	if (ET_HasElfVector(info->exceptionRecord->et_field)) {
		vectorSaveArea = (GeckoFPRContext*)(callersStackPointer - savedFloatingPointRegisters * 16);
		floatingPointSaveArea = (f64*)vectorSaveArea;
	} else {
		floatingPointSaveArea = (f64*)(callersStackPointer - savedFloatingPointRegisters * 8);
	}

	if (ET_HasElfVector(info->exceptionRecord->et_field)) {
            for (registerIndex = 32 - savedFloatingPointRegisters, saveIndex = 0;
                 registerIndex < 32; ++registerIndex, ++saveIndex) {
                context->floatingPointRegisters[registerIndex].vectorValue.singlePrecisionLanes[0] =
                    vectorSaveArea[saveIndex].vectorValue.singlePrecisionLanes[0];
                context->floatingPointRegisters[registerIndex].vectorValue.singlePrecisionLanes[1] =
                    vectorSaveArea[saveIndex].vectorValue.singlePrecisionLanes[1];
                context->floatingPointRegisters[registerIndex].doubleValue =
                    vectorSaveArea[saveIndex].doubleValue;
            }
        } else {
            for (registerIndex = 32 - savedFloatingPointRegisters, saveIndex = 0;
                 registerIndex < 32; ++registerIndex, ++saveIndex) {
                context->floatingPointRegisters[registerIndex].doubleValue =
                    floatingPointSaveArea[saveIndex];
            }
        }

	savedGeneralPurposeRegisters = ET_GetSavedGPRs(info->exceptionRecord->et_field);
	generalPurposeSaveArea = (s32*)floatingPointSaveArea;
	generalPurposeSaveArea -= savedGeneralPurposeRegisters;

        for (registerIndex = 32 - savedGeneralPurposeRegisters, saveIndex = 0; registerIndex < 32;
             ++registerIndex, ++saveIndex) {
            context->generalPurposeRegisters[registerIndex] = generalPurposeSaveArea[saveIndex];
        }

        context->stackPointer = callersStackPointer;
	return *(char**)(callersStackPointer + RETURN_ADDRESS);
}

// Called by ExPPC_UnwindStack for a local-destruction record; runs the object's destructor.
static inline void ExPPC_DestroyLocal(ThrowContext *context, const ex_destroylocal *ex)
{
    DTORCALL_COMPLETE(ex->dtor, context->framePointer + ex->local);
}

// Called by ExPPC_UnwindStack for a conditional local record; destroys it only if constructed.
static inline void ExPPC_DestroyLocalCond(ThrowContext* context, const ex_destroylocalcond* ex)
{
    int constructionCondition = ex_destroylocalcond_GetRegCond(ex->dlc_field)
                                    ? (local_cond_type) context->generalPurposeRegisters[ex->cond]
                                    : *(local_cond_type *) (context->framePointer + ex->cond);

    if (constructionCondition) {
        DTORCALL_COMPLETE(ex->dtor, context->framePointer + ex->local);
	}
}

// Called by ExPPC_UnwindStack for a pointer record; destroys the object at its recorded address.
static inline void ExPPC_DestroyLocalPointer(ThrowContext *context,
                                             const ex_destroylocalpointer *ex)
{
    void *objectPointer = ex_destroylocalpointer_GetRegPointer(ex->dlp_field)
                              ? (void *) context->generalPurposeRegisters[ex->pointer]
                              : *(void **) (context->framePointer + ex->pointer);

    DTORCALL_COMPLETE(ex->dtor, objectPointer);
}

// Called by ExPPC_UnwindStack for a local-array record; destroys elements in reverse order.
static inline void ExPPC_DestroyLocalArray(ThrowContext* context, const ex_destroylocalarray* ex)
{
	char* elementAddress = context->framePointer + ex->localarray;
	s32 remainingElements = ex->elements;
	s32 elementSize = ex->element_size;

        for (elementAddress = elementAddress + elementSize * remainingElements;
             remainingElements > 0; remainingElements--) {
            elementAddress -= elementSize;
            DTORCALL_COMPLETE(ex->dtor, elementAddress);
        }
}

// Called by ExPPC_UnwindStack for a member record; runs the member complete destructor.
static inline void ExPPC_DestroyMember(ThrowContext* context, const ex_destroymember* ex)
{
    char *objectPointer = ex_destroymember_GetRegPointer(ex->dm_field)
                              ? (char *) context->generalPurposeRegisters[ex->objectptr]
                              : *(char **) (context->framePointer + ex->objectptr);

    DTORCALL_COMPLETE(ex->dtor, objectPointer + ex->offset);
}

// Called by ExPPC_UnwindStack for a base record; runs the base destructor during construction
// failure.
static inline void ExPPC_DestroyBase(ThrowContext* context, const ex_destroymember* ex)
{
    char *objectPointer = ex_destroymember_GetRegPointer(ex->dm_field)
                              ? (char *) context->generalPurposeRegisters[ex->objectptr]
                              : *(char **) (context->framePointer + ex->objectptr);

    DTORCALL_PARTIAL(ex->dtor, objectPointer + ex->offset);
}

// Called by ExPPC_UnwindStack; runs the member's partial destructor only when its construction flag
// is set.
static inline void ExPPC_DestroyMemberCond(ThrowContext* context, const ex_destroymembercond* ex)
{
    char *objectPointer = ex_destroymembercond_GetRegPointer(ex->dmc_field)
                              ? (char *) context->generalPurposeRegisters[ex->objectptr]
                              : *(char **) (context->framePointer + ex->objectptr);
    int constructionCondition =
        ex_destroymembercond_GetRegCond(ex->dmc_field)
            ? (vbase_ctor_arg_type) context->generalPurposeRegisters[ex->cond]
            : *(vbase_ctor_arg_type *) (context->framePointer + ex->cond);

    if (constructionCondition) {
        DTORCALL_PARTIAL(ex->dtor, objectPointer + ex->offset);
	}
}

// Called by ExPPC_UnwindStack for a member-array record; destroys elements in reverse order.
static inline void ExPPC_DestroyMemberArray(ThrowContext* context, const ex_destroymemberarray* ex)
{
    char *elementAddress = ex_destroymemberarray_GetRegPointer(ex->dma_field)
                               ? (char *) context->generalPurposeRegisters[ex->objectptr]
                               : *(char **) (context->framePointer + ex->objectptr);
    s32 remainingElements = ex->elements;
    s32 elementSize = ex->element_size;

    elementAddress += ex->offset;

    for (elementAddress = elementAddress + elementSize * remainingElements; remainingElements > 0;
         remainingElements--) {
        elementAddress -= elementSize;
        DTORCALL_COMPLETE(ex->dtor, elementAddress);
    }
}

// Called by ExPPC_UnwindStack for a delete record; calls the recorded delete function.
static inline void ExPPC_DeletePointer(ThrowContext* context, const ex_deletepointer* ex)
{
    char *objectPointer = ex_deletepointer_GetRegPointer(ex->dp_field)
                              ? (char *) context->generalPurposeRegisters[ex->objectptr]
                              : *(char **) (context->framePointer + ex->objectptr);

    ((DeleteFunc) ex->deletefunc)(objectPointer);
}

// Called by ExPPC_UnwindStack for a conditional delete record; deletes only when its condition is
// true.
static inline void ExPPC_DeletePointerCond(ThrowContext* context, const ex_deletepointercond* ex)
{
    char *objectPointer = ex_deletepointercond_GetRegPointer(ex->dpc_field)
                              ? (char *) context->generalPurposeRegisters[ex->objectptr]
                              : *(char **) (context->framePointer + ex->objectptr);
    int constructionCondition = ex_deletepointercond_GetRegCond(ex->dpc_field)
                                    ? (local_cond_type) context->generalPurposeRegisters[ex->cond]
                                    : *(local_cond_type *) (context->framePointer + ex->cond);

    if (constructionCondition) {
        ((DeleteFunc) ex->deletefunc)(objectPointer);
	}
}

// Called by ExPPC_ThrowHandler and ExPPC_HandleUnexpected; unwinds cleanup actions up to a catch or
// exception-specification record.
static void ExPPC_UnwindStack(ThrowContext* context, MWExceptionInfo* info, void* stopAction)
{
	exaction_type action;

#pragma exception_terminate

	for (;;) {
		if (info->actionPointer == 0) {
			char* returnAddress;

			returnAddress = ExPPC_PopStackFrame(context, info);
			ExPPC_FindExceptionRecord(returnAddress, info);

			if (info->exceptionRecord == 0) {
				terminate();
			}

                        context->framePointer = (ET_GetHasFramePtr(info->exceptionRecord->et_field))
                                                    ? (char *) context->generalPurposeRegisters[31]
                                                    : context->stackPointer;
                        continue;
		}

		action = ((ex_destroylocal*)info->actionPointer)->action;

		switch (action & EXACTION_MASK) {
		case EXACTION_BRANCH:
                    info->actionPointer = ((char *) info->exceptionRecord) +
                                          ((ex_branch *) info->actionPointer)->target;
                    break;
		case EXACTION_DESTROYLOCAL:
			ExPPC_DestroyLocal(context, (ex_destroylocal*)info->actionPointer);
			info->actionPointer += sizeof(ex_destroylocal);
			break;
		case EXACTION_DESTROYLOCALCOND:
			ExPPC_DestroyLocalCond(context, (ex_destroylocalcond*)info->actionPointer);
			info->actionPointer += sizeof(ex_destroylocalcond);
			break;
		case EXACTION_DESTROYLOCALPOINTER:
			ExPPC_DestroyLocalPointer(context, (ex_destroylocalpointer*)info->actionPointer);
			info->actionPointer += sizeof(ex_destroylocalpointer);
			break;
		case EXACTION_DESTROYLOCALARRAY:
			ExPPC_DestroyLocalArray(context, (ex_destroylocalarray*)info->actionPointer);
			info->actionPointer += sizeof(ex_destroylocalarray);
			break;
		case EXACTION_DESTROYBASE:
			ExPPC_DestroyBase(context, (ex_destroymember*)info->actionPointer);
			info->actionPointer += sizeof(ex_destroymember);
			break;
		case EXACTION_DESTROYMEMBER:
			ExPPC_DestroyMember(context, (ex_destroymember*)info->actionPointer);
			info->actionPointer += sizeof(ex_destroymember);
			break;
		case EXACTION_DESTROYMEMBERCOND:
			ExPPC_DestroyMemberCond(context, (ex_destroymembercond*)info->actionPointer);
			info->actionPointer += sizeof(ex_destroymembercond);
			break;
		case EXACTION_DESTROYMEMBERARRAY:
			ExPPC_DestroyMemberArray(context, (ex_destroymemberarray*)info->actionPointer);
			info->actionPointer += sizeof(ex_destroymemberarray);
			break;
		case EXACTION_DELETEPOINTER:
			ExPPC_DeletePointer(context, (ex_deletepointer*)info->actionPointer);
			info->actionPointer += sizeof(ex_deletepointer);
			break;
		case EXACTION_DELETEPOINTERCOND:
			ExPPC_DeletePointerCond(context, (ex_deletepointercond*)info->actionPointer);
			info->actionPointer += sizeof(ex_deletepointercond);
			break;
		case EXACTION_CATCHBLOCK:
			if (stopAction == (void*)info->actionPointer)
				return;
			info->actionPointer += sizeof(ex_catchblock);
			break;
		case EXACTION_CATCHBLOCK_32:
			if (stopAction == (void*)info->actionPointer)
				return;
			info->actionPointer += sizeof(ex_catchblock_32);
			break;
		case EXACTION_ACTIVECATCHBLOCK: {
			CatchInfo* catchInfo;

                        catchInfo = (CatchInfo *) (context->framePointer +
                                                   ((ex_activecatchblock *) info->actionPointer)
                                                       ->cinfo_ref);

                        // While unwinding an active catch, retain its destructor for the same
                        // object; destroy it when unwinding a different object.
                        if (catchInfo->dtor) {
				if (context->objectLocation == catchInfo->location) {
					context->destructor = catchInfo->dtor;
				} else {
					DTORCALL_COMPLETE(catchInfo->dtor, catchInfo->location);
				}
			}
			info->actionPointer += sizeof(ex_activecatchblock);
		} break;
		case EXACTION_SPECIFICATION:
			if (stopAction == (void*)info->actionPointer)
				return;
                        info->actionPointer +=
                            sizeof(ex_specification) +
                            ((ex_specification *) info->actionPointer)->specs * sizeof(void *);
                        break;
		default:
			terminate();
		}

		if (action & EXACTION_ENDBIT)
			info->actionPointer = 0;
	}
}

// Called by __unexpected and ExPPC_ThrowHandler; checks whether a specification permits the
// exception type.
static inline int ExPPC_IsInSpecification(char* exceptionType, ex_specification* specification)
{
	s32 typeIndex, inheritanceOffset;

	for (typeIndex = 0; typeIndex < specification->specs; typeIndex++) {
            if (__throw_catch_compare(exceptionType, specification->spec[typeIndex],
                                      &inheritanceOffset))
                return 1;
	}

	return 0;
}

// Called by the C++ runtime for a violated exception specification; invokes unexpected handling.
extern void __unexpected(CatchInfo* catchInfo)
{
	ex_specification* specification = (ex_specification*)catchInfo->stacktop;

#pragma exception_magic // allow access to __exception_magic in try/catch blocks

	try {
		unexpected();
	} catch (...) {
            if (ExPPC_IsInSpecification((char *) ((CatchInfo *) &__exception_magic)->typeinfo,
                                        specification)) {
                throw;
            }
                if (ExPPC_IsInSpecification("!bad_exception!!", specification)) {
			throw bad_exception();
		}
		if (ExPPC_IsInSpecification("!std::bad_exception!!", specification)) {
			throw bad_exception();
		}
	}
	terminate();
}

// Called by the throw handler after unwinding; restores processor state and jumps to the landing
// pad.
asm static void ExPPC_LongJump(register ThrowContext *context, register void *newRTOC,
                               register void *newPC)
{
#ifdef __MWERKS__ // clang-format off
	nofralloc

	mr r8, newPC
	mr RTOC, newRTOC
	lwz r0, context->conditionRegister
	mtcrf 255, r0

	lmw r13, context->generalPurposeRegisters[13]

	la r7, context->floatingPointRegisters[14].vectorValue
	psq_lx fp14, 0, r7, 0, 0
	lfd fp14, context->floatingPointRegisters[14].doubleValue

	la r7, context->floatingPointRegisters[15].vectorValue
	psq_lx fp15, 0, r7, 0, 0
	lfd fp15, context->floatingPointRegisters[15].doubleValue

	la r7, context->floatingPointRegisters[16].vectorValue
	psq_lx fp16, 0, r7, 0, 0
	lfd fp16, context->floatingPointRegisters[16].doubleValue

	la r7, context->floatingPointRegisters[17].vectorValue
	psq_lx fp17, 0, r7, 0, 0
	lfd fp17, context->floatingPointRegisters[17].doubleValue

	la r7, context->floatingPointRegisters[18].vectorValue
	psq_lx fp18, 0, r7, 0, 0
	lfd fp18, context->floatingPointRegisters[18].doubleValue

	la r7, context->floatingPointRegisters[19].vectorValue
	psq_lx fp19, 0, r7, 0, 0
	lfd fp19, context->floatingPointRegisters[19].doubleValue

	la r7, context->floatingPointRegisters[20].vectorValue
	psq_lx fp20, 0, r7, 0, 0
	lfd fp20, context->floatingPointRegisters[20].doubleValue

	la r7, context->floatingPointRegisters[21].vectorValue
	psq_lx fp21, 0, r7, 0, 0
	lfd fp21, context->floatingPointRegisters[21].doubleValue

	la r7, context->floatingPointRegisters[22].vectorValue
	psq_lx fp22, 0, r7, 0, 0
	lfd fp22, context->floatingPointRegisters[22].doubleValue

	la r7, context->floatingPointRegisters[23].vectorValue
	psq_lx fp23, 0, r7, 0, 0
	lfd fp23, context->floatingPointRegisters[23].doubleValue

	la r7, context->floatingPointRegisters[24].vectorValue
	psq_lx fp24, 0, r7, 0, 0
	lfd fp24, context->floatingPointRegisters[24].doubleValue

	la r7, context->floatingPointRegisters[25].vectorValue
	psq_lx fp25, 0, r7, 0, 0
	lfd fp25, context->floatingPointRegisters[25].doubleValue

	la r7, context->floatingPointRegisters[26].vectorValue
	psq_lx fp26, 0, r7, 0, 0
	lfd fp26, context->floatingPointRegisters[26].doubleValue

	la r7, context->floatingPointRegisters[27].vectorValue
	psq_lx fp27, 0, r7, 0, 0
	lfd fp27, context->floatingPointRegisters[27].doubleValue

	la r7, context->floatingPointRegisters[28].vectorValue
	psq_lx fp28, 0, r7, 0, 0
	lfd fp28, context->floatingPointRegisters[28].doubleValue

	la r7, context->floatingPointRegisters[29].vectorValue
	psq_lx fp29, 0, r7, 0, 0
	lfd fp29, context->floatingPointRegisters[29].doubleValue

	la r7, context->floatingPointRegisters[30].vectorValue
	psq_lx fp30, 0, r7, 0, 0
	lfd fp30, context->floatingPointRegisters[30].doubleValue

	la r7, context->floatingPointRegisters[31].vectorValue
	psq_lx fp31, 0, r7, 0, 0
	lfd fp31, context->floatingPointRegisters[31].doubleValue

	mtlr r8

	lwz SP, context->throwStackPointer
	lwz r3, context->stackPointer
	lwz r3, 0(r3)
	stw r3, 0(SP)
	blr
#endif // clang-format on
}

// Called by ExPPC_ThrowHandler for a disallowed exception; unwinds to the unexpected landing pad.
static inline void ExPPC_HandleUnexpected(ThrowContext *context, MWExceptionInfo *info,
                                          ex_specification *specification)
{
	CatchInfo* catchInfo;

#pragma exception_terminate

	ExPPC_UnwindStack(context, info, specification);

	catchInfo           = (CatchInfo*)(context->framePointer + specification->cinfo_ref);
	catchInfo->location = context->objectLocation;
	catchInfo->typeinfo = context->exceptionType;
	catchInfo->dtor     = context->destructor;
	catchInfo->stacktop = specification;

	ExPPC_LongJump(context, info->toc, info->currentFunction + specification->pcoffset);
}

// Called by __throw after processor state is saved; searches for a handler and unwinds to it.
static void ExPPC_ThrowHandler(ThrowContext* context)
{
	ActionIterator iter;
	MWExceptionInfo info;
	exaction_type action;
	CatchInfo* catchInfo;
	s32 catchOffset;

	ExPPC_FindExceptionRecord(context->returnAddress, &info);

	if (info.exceptionRecord == 0) {
		terminate();
	}

        context->framePointer = (ET_GetHasFramePtr(info.exceptionRecord->et_field))
                                    ? (char *) context->generalPurposeRegisters[31]
                                    : context->stackPointer;

        // A null exception type enters the rethrow path: recover the type and object location from
        // the active catch record.
        if (context->exceptionType == 0) {
		iter.exceptionInfo        = info;
		iter.stackPointer  = context->stackPointer;
		iter.framePointer  = context->framePointer;
		iter.savedR31 = context->generalPurposeRegisters[31];

		for (action = ExPPC_CurrentAction(&iter);; action = ExPPC_NextAction(&iter)) {
			switch (action) {
			case EXACTION_ACTIVECATCHBLOCK:
				break;
			case EXACTION_ENDOFLIST:
			case EXACTION_DESTROYLOCAL:
			case EXACTION_DESTROYLOCALCOND:
			case EXACTION_DESTROYLOCALPOINTER:
			case EXACTION_DESTROYLOCALARRAY:
			case EXACTION_DESTROYBASE:
			case EXACTION_DESTROYMEMBER:
			case EXACTION_DESTROYMEMBERCOND:
			case EXACTION_DESTROYMEMBERARRAY:
			case EXACTION_DELETEPOINTER:
			case EXACTION_DELETEPOINTERCOND:
			case EXACTION_CATCHBLOCK:
			case EXACTION_CATCHBLOCK_32:
			case EXACTION_SPECIFICATION:
				continue;
			case EXACTION_TERMINATE:
			default:
				terminate();
			}
			break;
		}

                catchInfo =
                    (CatchInfo *) (iter.framePointer +
                                   ((ex_activecatchblock *) iter.exceptionInfo.actionPointer)
                                       ->cinfo_ref);
                context->exceptionType = (char*)catchInfo->typeinfo;
		context->objectLocation  = catchInfo->location;
		context->destructor      = 0;
		context->catchInfo = catchInfo;
	} else {
		context->catchInfo = 0L;
	}

	iter.exceptionInfo        = info;
	iter.stackPointer  = context->stackPointer;
	iter.framePointer  = context->framePointer;
	iter.savedR31 = context->generalPurposeRegisters[31];

	for (action = ExPPC_CurrentAction(&iter);; action = ExPPC_NextAction(&iter)) {
		switch (action) {
		case EXACTION_CATCHBLOCK_32:
                    if (__throw_catch_compare(
                            context->exceptionType,
                            ((ex_catchblock_32 *) iter.exceptionInfo.actionPointer)->catch_type,
                            &catchOffset)) {
                        break;
                    }
                        continue;
		case EXACTION_CATCHBLOCK:
                    if (__throw_catch_compare(
                            context->exceptionType,
                            ((ex_catchblock *) iter.exceptionInfo.actionPointer)->catch_type,
                            &catchOffset)) {
                        break;
                    }
                        continue;
		case EXACTION_SPECIFICATION:
                    if (!ExPPC_IsInSpecification(
                            context->exceptionType,
                            (ex_specification *) iter.exceptionInfo.actionPointer)) {
                        ExPPC_HandleUnexpected(
                            context, &info, (ex_specification *) iter.exceptionInfo.actionPointer);
                    }
                        continue;
		case EXACTION_ENDOFLIST:
		case EXACTION_DESTROYLOCAL:
		case EXACTION_DESTROYLOCALCOND:
		case EXACTION_DESTROYLOCALPOINTER:
		case EXACTION_DESTROYLOCALARRAY:
		case EXACTION_DESTROYBASE:
		case EXACTION_DESTROYMEMBER:
		case EXACTION_DESTROYMEMBERCOND:
		case EXACTION_DESTROYMEMBERARRAY:
		case EXACTION_DELETEPOINTER:
		case EXACTION_DELETEPOINTERCOND:
		case EXACTION_ACTIVECATCHBLOCK:
			continue;
		case EXACTION_TERMINATE:
		default:
			terminate();
		}
		break;
	}

	if (action == EXACTION_CATCHBLOCK_32) {
		ex_catchblock_32* catchBlock32;
		catchBlock32 = (ex_catchblock_32*)iter.exceptionInfo.actionPointer;

		ExPPC_UnwindStack(context, &info, catchBlock32);

		catchInfo           = (CatchInfo*)(context->framePointer + catchBlock32->cinfo_ref);
		catchInfo->location = context->objectLocation;
		catchInfo->typeinfo = context->exceptionType;
		catchInfo->dtor     = context->destructor;

		// Pointer catches store the adjusted pointer in CatchInfo and point sublocation at that
		// copy.
		if (*context->exceptionType == '*') {
			catchInfo->sublocation = &catchInfo->pointercopy;
			catchInfo->pointercopy = *(s32*)context->objectLocation + catchOffset;
		} else {
			catchInfo->sublocation = (char*)context->objectLocation + catchOffset;
		}

		ExPPC_LongJump(context, info.toc, info.currentFunction + catchBlock32->catch_pcoffset);
	} else {
		ex_catchblock* catchblock;

		catchblock = (ex_catchblock*)iter.exceptionInfo.actionPointer;
		ExPPC_UnwindStack(context, &info, catchblock);

		catchInfo           = (CatchInfo*)(context->framePointer + catchblock->cinfo_ref);
		catchInfo->location = context->objectLocation;
		catchInfo->typeinfo = context->exceptionType;
		catchInfo->dtor     = context->destructor;

		// Pointer catches store the adjusted pointer in CatchInfo and point sublocation at that
		// copy.
		if (*context->exceptionType == '*') {
			catchInfo->sublocation = &catchInfo->pointercopy;
			catchInfo->pointercopy = *(s32*)context->objectLocation + catchOffset;
		} else {
			catchInfo->sublocation = (char*)context->objectLocation + catchOffset;
		}

		ExPPC_LongJump(context, info.toc, info.currentFunction + catchblock->catch_pcoffset);
	}
}

// Called by generated catch-block cleanup code; destroys the caught object when needed.
void __end__catch(CatchInfo* catchInfo)
{
	if (catchInfo->location && catchInfo->dtor) {
		DTORCALL_COMPLETE(catchInfo->dtor, catchInfo->location);
	}
}

// Called by generated throw code; saves processor state and starts handler search and unwinding.
asm void __throw(char* exceptionType, void* objectLocation, void* destructor)
{
#ifdef __MWERKS__ // clang-format off
	ThrowContext throwcontext;

	fralloc

	stmw r13, throwcontext.generalPurposeRegisters[13]

	stfd fp14, throwcontext.floatingPointRegisters[14].doubleValue
	la r3, throwcontext.floatingPointRegisters[14].vectorValue
	psq_stx fp14, 0, r3,0,0

	stfd fp15, throwcontext.floatingPointRegisters[15].doubleValue
	la r3, throwcontext.floatingPointRegisters[15].vectorValue
	psq_stx fp15, 0, r3, 0, 0

	stfd fp16, throwcontext.floatingPointRegisters[16].doubleValue
	la r3, throwcontext.floatingPointRegisters[16].vectorValue
	psq_stx fp16, 0, r3, 0, 0

	stfd fp17, throwcontext.floatingPointRegisters[17].doubleValue
	la r3, throwcontext.floatingPointRegisters[17].vectorValue
	psq_stx fp17, 0, r3, 0, 0

	stfd fp18, throwcontext.floatingPointRegisters[18].doubleValue
	la r3, throwcontext.floatingPointRegisters[18].vectorValue
	psq_stx fp18, 0, r3, 0, 0

	stfd fp19, throwcontext.floatingPointRegisters[19].doubleValue
	la r3, throwcontext.floatingPointRegisters[19].vectorValue
	psq_stx fp19, 0, r3, 0, 0

	stfd fp20, throwcontext.floatingPointRegisters[20].doubleValue
	la r3, throwcontext.floatingPointRegisters[20].vectorValue
	psq_stx fp20, 0, r3, 0, 0

	stfd fp21, throwcontext.floatingPointRegisters[21].doubleValue
	la r3, throwcontext.floatingPointRegisters[21].vectorValue
	psq_stx fp21, 0, r3, 0, 0

	stfd fp22, throwcontext.floatingPointRegisters[22].doubleValue
	la r3, throwcontext.floatingPointRegisters[22].vectorValue
	psq_stx fp22, 0, r3, 0, 0

	stfd fp23, throwcontext.floatingPointRegisters[23].doubleValue
	la r3, throwcontext.floatingPointRegisters[23].vectorValue
	psq_stx fp23, 0, r3, 0, 0

	stfd fp24, throwcontext.floatingPointRegisters[24].doubleValue
	la r3, throwcontext.floatingPointRegisters[24].vectorValue
	psq_stx fp24, 0, r3, 0, 0

	stfd fp25, throwcontext.floatingPointRegisters[25].doubleValue
	la r3, throwcontext.floatingPointRegisters[25].vectorValue
	psq_stx fp25, 0, r3, 0, 0

	stfd fp26, throwcontext.floatingPointRegisters[26].doubleValue
	la r3, throwcontext.floatingPointRegisters[26].vectorValue
	psq_stx fp26, 0, r3, 0, 0

	stfd fp27, throwcontext.floatingPointRegisters[27].doubleValue
	la r3, throwcontext.floatingPointRegisters[27].vectorValue
	psq_stx fp27, 0, r3, 0, 0

	stfd fp28, throwcontext.floatingPointRegisters[28].doubleValue
	la r3, throwcontext.floatingPointRegisters[28].vectorValue
	psq_stx fp28, 0, r3, 0, 0

	stfd fp29, throwcontext.floatingPointRegisters[29].doubleValue
	la r3, throwcontext.floatingPointRegisters[29].vectorValue
	psq_stx fp29, 0, r3, 0, 0

	stfd fp30, throwcontext.floatingPointRegisters[30].doubleValue
	la r3, throwcontext.floatingPointRegisters[30].vectorValue
	psq_stx fp30, 0, r3, 0, 0

	stfd fp31, throwcontext.floatingPointRegisters[31].doubleValue
	la r3, throwcontext.floatingPointRegisters[31].vectorValue
	psq_stx fp31, 0, r3, 0, 0

	mfcr r3
	stw	r3, throwcontext.conditionRegister;

	lwz r3, 0(sp)
	lwz r4, RETURN_ADDRESS(r3)
	stw r3, throwcontext.stackPointer;
	stw r3, throwcontext.throwStackPointer;
	stw r4, throwcontext.returnAddress;

	lwz r3,exceptionType
	stw r3, throwcontext.exceptionType
	lwz r3,objectLocation
	stw r3, throwcontext.objectLocation
	lwz r3,destructor
	stw r3, throwcontext.destructor
	la r3, throwcontext
	bl ExPPC_ThrowHandler
	nop
	frfree
	blr
#endif // clang-format on
}
