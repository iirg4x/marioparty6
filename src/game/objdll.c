// Loads, starts, and releases the object modules used as game overlays.
#define _MATH_H
#include "game/objdll.h"
#include "game/dvd.h"
#include "game/memory.h"

typedef BOOL (*OMDLLPROLOG)(void);
typedef void (*OMDLLEPILOG)(void);

OMDLLINFO *omDLLinfoTbl[OMDLLINFO_MAX];

static OVLTBL *omDLLFileList;

void omDLLDBGOut(void)
{
	OSReport("DLL DBG OUT\n");
}

// Called by omMasterInit to clear the loaded-module slots and save the overlay table.
void omDLLInit(OVLTBL *overlayTable)
{
	s32 slot;
	OSReport("DLL DBG OUT\n");
	for(slot=0; slot<OMDLLINFO_MAX; slot++) {
		omDLLinfoTbl[slot] = NULL;
	}
	omDLLFileList = overlayTable;
}

// Called by the overlay manager when an overlay is selected; links a new module or restarts a
// loaded one.
s32 omDLLStart(s16 overlayNumber, s16 reloadFlag)
{
	s32 slot;
	OSReport("DLLStart %d %d\n", overlayNumber, reloadFlag);
	slot = omDLLSearch(overlayNumber);
	if(slot >= 0 && !reloadFlag) {
		OMDLLINFO *moduleInfo = omDLLinfoTbl[slot];
                OSReport("objdll>Already Loaded %s(%08x %08x)\n", moduleInfo->name,
                         moduleInfo->module, moduleInfo->bss);

                omDLLInfoDump(&moduleInfo->module->info);
		omDLLHeaderDump(moduleInfo->module);
		// Clear the module's BSS before rerunning its prolog.
		memset(moduleInfo->bss, 0, moduleInfo->module->bssSize);
		HuMemDCFlushAll();
		moduleInfo->ret = ((OMDLLPROLOG)moduleInfo->module->prolog)();
		OSReport("objdll> %s prolog end\n", moduleInfo->name);
		return slot;
	} else {
		for(slot=0; slot<OMDLLINFO_MAX; slot++) {
			if(omDLLinfoTbl[slot] == NULL) {
				break;
			}
		}
		if(slot == OMDLLINFO_MAX) {
			return -1;
		}
		omDLLLink(&omDLLinfoTbl[slot], overlayNumber, TRUE);
		return slot;
	}
}

// Called by omOvlKill to end the overlay identified by its table number.
void omDLLNumEnd(s16 overlayNumber, s16 reloadFlag)
{
	s16 slot;
	if(overlayNumber < 0) {
		OSReport("objdll>omDLLNumEnd Invalid dllno %d\n", overlayNumber);
		return;
	}
	OSReport("objdll>omDLLNumEnd %d %d\n", overlayNumber, reloadFlag);
	slot = omDLLSearch(overlayNumber);
	if(slot < 0) {
		OSReport("objdll>omDLLNumEnd not found DLL No%d\n", overlayNumber);
		return;
	}
	omDLLEnd(slot, reloadFlag);
}

// Ends a loaded overlay; omDLLNumEnd resolves the table number before calling this.
void omDLLEnd(s16 slot, s16 reloadFlag)
{
	OSReport("objdll>omDLLEnd %d %d\n", slot, reloadFlag);
	if(reloadFlag == TRUE) {
		OSReport("objdll>End DLL:%s\n", omDLLinfoTbl[slot]->name);
		omDLLUnlink(omDLLinfoTbl[slot], 1);
		omDLLinfoTbl[slot] = NULL;
	} else {
		OMDLLINFO *moduleInfo = omDLLinfoTbl[slot];

		OSReport("objdll>Call Epilog\n");
		((OMDLLEPILOG)moduleInfo->module->epilog)();
		OSReport("objdll>End DLL stayed:%s\n", omDLLinfoTbl[slot]->name);
	}
	OSReport("objdll>End DLL finish\n");
}

// Reads and links the overlay module, then optionally runs its prolog during startup.
OMDLLINFO *omDLLLink(OMDLLINFO **moduleInfoOut, s16 overlayNumber, s16 callProlog)
{
	OMDLLINFO *moduleInfo;
	OVLTBL *overlayEntry = &omDLLFileList[overlayNumber];
    static u8 ATTRIBUTE_ALIGN(32) strTable[1024]; // Reserved to preserve BSS alignment in this
                                                  // module.

	OSReport("objdll>Link DLL:%s\n", overlayEntry->name);
	moduleInfo = HuMemDirectMalloc(HEAP_HEAP, sizeof(OMDLLINFO));
	*moduleInfoOut = moduleInfo;
	moduleInfo->name = overlayEntry->name;
	moduleInfo->module = HuDvdDataReadDirect(overlayEntry->name, HEAP_HEAP);
	moduleInfo->bss = HuMemDirectMalloc(HEAP_HEAP, moduleInfo->module->bssSize);
	if(OSLink(&moduleInfo->module->info, moduleInfo->bss) != TRUE) {
		OSReport("objdll>++++++++++++++++ DLL Link Failed\n");
		// The failed link is logged, then this path continues to report and optionally start the
		// module.
	}
	omDLLInfoDump(&moduleInfo->module->info);
	omDLLHeaderDump(moduleInfo->module);
	OSReport("objdll>LinkOK %08x %08x\n", moduleInfo->module, moduleInfo->bss);
	if(callProlog == TRUE) {
		OSReport("objdll> %s prolog start\n", overlayEntry->name);
		moduleInfo->ret = ((OMDLLPROLOG)moduleInfo->module->prolog)();
		OSReport("objdll> %s prolog end\n", overlayEntry->name);
	}
	return moduleInfo;
}

// Runs the epilog when requested, attempts to unlink the module, logs failure, then frees its BSS,
// module image, and info block regardless.
void omDLLUnlink(OMDLLINFO *moduleInfo, s16 callEpilog)
{
	OSReport("odjdll>Unlink DLL:%s\n", moduleInfo->name);
	if(callEpilog == TRUE) {
		OSReport("objdll>Unlink DLL epilog\n");
		((OMDLLEPILOG)moduleInfo->module->epilog)();
		OSReport("objdll>Unlink DLL epilog finish\n");
	}
	if(OSUnlink(&moduleInfo->module->info) != TRUE) {
		OSReport("objdll>+++++++++++++++++ DLL Unlink Failed\n");
	}
	HuMemDirectFree(moduleInfo->bss);
	HuMemDirectFree(moduleInfo->module);
	HuMemDirectFree(moduleInfo);
}

// Finds the loaded slot whose module name matches the requested overlay table entry.
s32 omDLLSearch(s16 overlayNumber)
{
	s32 slot;
	OVLTBL *overlayEntry = &omDLLFileList[overlayNumber];
	OSReport("Search:%s\n", overlayEntry->name);
	for(slot=0; slot<OMDLLINFO_MAX; slot++) {
		OMDLLINFO *loadedModule = omDLLinfoTbl[slot];
		if(loadedModule != NULL && strcmp(loadedModule->name, overlayEntry->name) == 0) {
			OSReport("+++++++++++ Find%d: %s\n", slot, loadedModule->name);
			return slot;
		}
	}
	return -1;
}

// Prints the OS linker metadata when a module is loaded or restarted.
void omDLLInfoDump(OSModuleInfo *moduleInfo)
{
	OSReport("===== DLL Module Info dump ====\n");
	OSReport("                   ID:0x%08x\n", moduleInfo->id);
	OSReport("             LinkPrev:0x%08x\n", moduleInfo->link.prev);
	OSReport("             LinkNext:0x%08x\n", moduleInfo->link.next);
	OSReport("          Section num:%d\n", moduleInfo->numSections);
	OSReport("Section info tbl ofst:0x%08x\n", moduleInfo->sectionInfoOffset);
	OSReport("           nameOffset:0x%08x\n", moduleInfo->nameOffset);
	OSReport("             nameSize:%d\n", moduleInfo->nameSize);
	OSReport("              version:0x%08x\n", moduleInfo->version);
	OSReport("===============================\n");
}

// Prints the module header's BSS, relocation, and entry-point fields during loading.
void omDLLHeaderDump(OSModuleHeader *moduleHeader)
{
	OSReport("==== DLL Module Header dump ====\n");
	OSReport("          bss Size:0x%08x\n", moduleHeader->bssSize);
	OSReport("        rel Offset:0x%08x\n", moduleHeader->relOffset);
	OSReport("        imp Offset:0x%08x\n", moduleHeader->impOffset);
	OSReport("    prolog Section:%d\n", moduleHeader->prologSection);
	OSReport("    epilog Section:%d\n", moduleHeader->epilogSection);
	OSReport("unresolved Section:%d\n", moduleHeader->unresolvedSection);
	OSReport("       prolog func:0x%08x\n", moduleHeader->prolog);
	OSReport("       epilog func:0x%08x\n", moduleHeader->epilog);
	OSReport("   unresolved func:0x%08x\n", moduleHeader->unresolved);
	OSReport("================================\n");
}
