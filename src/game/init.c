// Set up the console video, memory heaps, audio, and per-frame display flow.
#include "game/init.h"
#include "game/fault.h"
#include "dolphin/os.h"
#include "dolphin/gx.h"
#include "dolphin/dvd.h"
#include "dolphin/vi.h"
#include "dolphin/pad.h"

#define FIFO_SIZE 524288
#define FRAMEBUFFER_FILL_BIT_PATTERN 0x800080
#define DEVHW1_PHYSICAL_MEM_SIZE_4MB 4194304
#define DEVHW1_SIMULATED_MEM_SIZE_24MB 25165824

struct memory_info {
    void *rangeStart; // First address in a memory range reserved away from the heap.
    void *rangeEnd;   // Address just past the reserved range, as consumed by OSAllocFixed.
};

extern void HuAudInit();
extern void HuCardInit();
extern void HuARInit();
extern u32 frand();

static GXRenderModeObj rmodeobj;

static BOOL FirstFrame = TRUE;
GXRenderModeObj *RenderMode;
OSHeapHandle currentHeapHandle;
static void *DefaultFifo;
static GXFifoObj *DefaultFifoObj;
void *DemoFrameBuffer1;
void *DemoFrameBuffer2;
void *DemoCurrentBuffer;
u32 minimumVcount;
float minimumVcountf;
u32 worstVcount;
static BOOL DemoStatEnable;

static void InitRenderMode(GXRenderModeObj *renderMode);
static void InitMem();
static void InitGX();
static void InitVI();
static void SwapBuffers();
static void LoadMemInfo();

// Called once by main before game systems start; initializes video, heaps, and services. On
// non-MPAL systems, a reset with progressive mode and DTV active overrides the requested mode with
// 480p.
void HuSysInit(GXRenderModeObj *renderMode)
{
    u32 ignoredRandomValue;
    OSInit();
    DVDInit();
    VIInit();
    PADInit();
    if (VIGetTvFormat() != VI_MPAL && OSGetResetCode() != 0 && OSGetProgressiveMode() == 1 &&
        VIGetDTVStatus() == 1) {
        renderMode = &GXNtsc480Prog;
    }
    InitRenderMode(renderMode);
    InitMem();
    VIConfigure(RenderMode);
    VIConfigurePan(0, 0, 640, 480);
    DefaultFifo = OSAlloc(FIFO_SIZE);
    DefaultFifoObj = GXInit(DefaultFifo, FIFO_SIZE);
    InitGX();
    InitVI();
    HuFaultInitXfbDirectDraw(RenderMode);
    HuFaultSetXfbAddress(0, DemoFrameBuffer1);
    HuFaultSetXfbAddress(1, DemoFrameBuffer2);
    HuDvdErrDispInit(RenderMode, DemoFrameBuffer1, DemoFrameBuffer2);
    // Advance the random sequence during startup; this returned value is not used.
    ignoredRandomValue = frand();
    HuMemInitAll();
    HuAudInit();
    HuARInit();
    minimumVcount = minimumVcountf = 1.0f;
    worstVcount = 0;
    OSInitFastCast();
    HuCardInit();
}

// Called by HuSysInit to select the requested display mode or derive and overscan-adjust one.
static void InitRenderMode(GXRenderModeObj *renderMode)
{
    if(renderMode != NULL) {
        RenderMode = renderMode;
        return;
    }
    switch(VIGetTvFormat()) {
        case VI_NTSC:
            RenderMode = &GXNtsc480IntDf;
            break;
            
        case VI_PAL:
            RenderMode = &GXPal528IntDf;
            break;
            
        case VI_MPAL:
            RenderMode = &GXMpal480IntDf;
            break;
            
        default:
            OSPanic("init.c", 191, "DEMOInit: invalid TV format\n");
            break;
    }
    GXAdjustForOverscan(RenderMode, &rmodeobj, 0, 16);
    RenderMode = &rmodeobj;
}

// Called by HuSysInit to configure GX copy, viewport, and pixel settings for the selected mode.
static void InitGX()
{
    GXSetViewport(0, 0, RenderMode->fbWidth, RenderMode->xfbHeight, 0, 1);
    GXSetScissor(0, 0, RenderMode->fbWidth, RenderMode->efbHeight);
    GXSetDispCopySrc(0, 0, RenderMode->fbWidth, RenderMode->efbHeight);
    GXSetDispCopyDst(RenderMode->fbWidth, RenderMode->xfbHeight);
    GXSetDispCopyYScale(GXGetYScaleFactor(RenderMode->efbHeight, RenderMode->xfbHeight));
    GXSetCopyFilter(RenderMode->aa, RenderMode->sample_pattern, GX_TRUE, RenderMode->vfilter);
    if(RenderMode->aa) {
        GXSetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    } else {
        GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    }
    GXCopyDisp(DemoCurrentBuffer, GX_TRUE);
    GXSetDispCopyGamma(GX_GM_1_0);
}

// Called by HuSysInit to initialize both display buffers with FRAMEBUFFER_FILL_BIT_PATTERN, write
// them back, advance the arena past them, and initialize the heap.
static void InitMem()
{
    void *arenaStart = OSGetArenaLo();
    void *arenaEnd = OSGetArenaHi();
    u32 frameBufferBytes = (u16)(((u16)RenderMode->fbWidth+15) & ~15)*RenderMode->xfbHeight*2;
    u32 *firstBufferWord;
    u32 *secondBufferWord;
    u32 wordIndex;
    DemoFrameBuffer1 = (void *)OSRoundUp32B((u32)arenaStart);
    DemoFrameBuffer2 = (void *)OSRoundUp32B((u32)DemoFrameBuffer1+frameBufferBytes);
    DemoCurrentBuffer = DemoFrameBuffer2;
    firstBufferWord = DemoFrameBuffer1;
    secondBufferWord = DemoFrameBuffer2;
    for (wordIndex = 0; wordIndex < frameBufferBytes / 4;
         wordIndex++, firstBufferWord++, secondBufferWord++) {
        *firstBufferWord = *secondBufferWord = FRAMEBUFFER_FILL_BIT_PATTERN;
    }
    DCStoreRangeNoSync(DemoFrameBuffer1, frameBufferBytes);
    DCStoreRangeNoSync(DemoFrameBuffer2, frameBufferBytes);
    arenaStart = (void *)OSRoundUp32B((u32)DemoFrameBuffer2+frameBufferBytes);
    OSSetArenaLo(arenaStart);
    if (OSGetConsoleType() == OS_CONSOLE_DEVHW1 &&
        OSGetPhysicalMemSize() != DEVHW1_PHYSICAL_MEM_SIZE_4MB &&
        OSGetConsoleSimulatedMemSize() < DEVHW1_SIMULATED_MEM_SIZE_24MB) {
        LoadMemInfo();
    } else {
        arenaStart = OSGetArenaLo();
        arenaEnd = OSGetArenaHi();
        arenaStart = OSInitAlloc(arenaStart, arenaEnd, 1);
        OSSetArenaLo(arenaStart);
        arenaStart = (void *)OSRoundUp32B((u32)arenaStart);
        arenaEnd = (void *)OSRoundDown32B((u32)arenaEnd);
        OSSetCurrentHeap(currentHeapHandle = OSCreateHeap(arenaStart, arenaEnd));
        arenaStart = arenaEnd;
        OSSetArenaLo(arenaStart);
    }
}

// Called by HuSysInit to queue the first framebuffer, wait for retrace, and wait for a second
// retrace when viTVmode's low bit is set.
static void InitVI()
{
    u32 waitSecondRetrace;
    VISetNextFrameBuffer(DemoFrameBuffer1);
    DemoCurrentBuffer = DemoFrameBuffer2;
    VIFlush();
    VIWaitForRetrace();
    waitSecondRetrace = RenderMode->viTVmode & 0x1;
    if(waitSecondRetrace) {
        VIWaitForRetrace();
    }
}

// Main calls this before drawing on each render pass; it selects a field-aware GX viewport and
// invalidates the vertex and texture caches.
void HuSysBeforeRender()
{
    if(RenderMode->field_rendering) {
        GXSetViewportJitter(0, 0, RenderMode->fbWidth, RenderMode->xfbHeight, 0, 1,
                            VIGetNextField());
    } else {
        GXSetViewport(0, 0, RenderMode->fbWidth, RenderMode->xfbHeight, 0, 1);
    }
    GXInvalidateVtxCache();
    GXInvalidateTexAll();
}

// Main calls this after each render pass; it copies the frame, conditionally enforces the minimum
// retrace distance, then swaps buffers.
void HuSysDoneRender(s32 retraceCount)
{
    s32 retraceDistance;
    if(DemoStatEnable) {
        GXDrawDone();
        DEMOUpdateStats(1);
        DEMOPrintStats();
        GXDrawDone();
        DEMOUpdateStats(0);
    }
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetColorUpdate(GX_TRUE);
    GXDrawDone();
    GXCopyDisp(DemoCurrentBuffer, GX_TRUE);
    if(minimumVcount != 0) {
        retraceDistance = VIGetRetraceCount()-retraceCount;
        if(worstVcount < retraceDistance) {
            worstVcount = retraceDistance;
        }
        while(VIGetRetraceCount()-retraceCount < minimumVcount-1) {
            VIWaitForRetrace();
        }
    }
    SwapBuffers();
}

// Called by HuSysDoneRender to queue the rendered buffer, reveal the display on the first swap,
// wait one retrace, then select the other framebuffer.
static void SwapBuffers()
{
    VISetNextFrameBuffer(DemoCurrentBuffer);
    if(FirstFrame) {
        VISetBlack(GX_FALSE);
        FirstFrame = FALSE;
    }
    VIFlush();
    VIWaitForRetrace();
    if(DemoCurrentBuffer == DemoFrameBuffer1) {
        DemoCurrentBuffer = DemoFrameBuffer2;
    } else {
        DemoCurrentBuffer = DemoFrameBuffer1;
    }
}

// On limited-memory DEVHW1 setups, reads /meminfo.bin to reserve its listed ranges; if the file is
// absent, initializes a heap from the available arena.
static void LoadMemInfo()
{
    u32 remainingBytes;
    u32 chunkBytes;
    u32 fileOffset;
    u32 rangeCount;
    u32 rangeIndex;
    void *arenaStart;
    void *arenaEnd;
    void *fixedRangeStart;
    void *fixedRangeEnd;
    
    struct memory_info *rangeEntries;
    DVDFileInfo fileInfo;
    char chunkBuffer[240];
    
    OSReport("\nNow, try to find memory info file...\n\n");
    if(!DVDOpen("/meminfo.bin", &fileInfo)) {
        OSReport("\nCan't find memory info file. Use /XXX toolname/ to maximize available\n");
        OSReport("memory space. For now, we only use the first %dMB.\n",
                 (OSGetConsoleSimulatedMemSize() / 1024) / 1024);
        arenaStart = OSGetArenaLo();
        arenaEnd = OSGetArenaHi();
        arenaStart = OSInitAlloc(arenaStart, arenaEnd, 1);
        OSSetArenaLo(arenaStart);
        arenaStart = (void *)OSRoundUp32B((u32)arenaStart);
        arenaEnd = (void *)OSRoundDown32B((u32)arenaEnd);
        OSSetCurrentHeap(OSCreateHeap(arenaStart, arenaEnd));
        arenaStart = arenaEnd;
        OSSetArenaLo(arenaStart);
    } else {
        rangeEntries = (struct memory_info *)OSRoundUp32B((u32)chunkBuffer);
        fixedRangeStart = OSGetArenaHi();
        fixedRangeEnd = (void *)(OSGetConsoleSimulatedMemSize()+OS_BASE_CACHED);
        OSSetArenaHi((void *)(OSGetPhysicalMemSize()+OS_BASE_CACHED));
        arenaStart = OSGetArenaLo();
        arenaEnd = OSGetArenaHi();
        arenaStart = OSInitAlloc(arenaStart, arenaEnd, 1);
        OSSetArenaLo(arenaStart);
        arenaStart = (void *)OSRoundUp32B((u32)arenaStart);
        arenaEnd = (void *)OSRoundDown32B((u32)arenaEnd);
        OSSetCurrentHeap(OSCreateHeap(arenaStart, arenaEnd));
        arenaStart = arenaEnd;
        OSSetArenaLo(arenaStart);
        // Remove the unused simulated-memory region from the new heap before applying file ranges.
        OSAllocFixed(&fixedRangeStart, &fixedRangeEnd);
        remainingBytes = fileInfo.length;
        fileOffset = 0;
        while(remainingBytes) {
            OSReport("loop\n");
            chunkBytes = (remainingBytes < 32) ? remainingBytes : 32;
            if(DVDRead(&fileInfo, rangeEntries, OSRoundUp32B(chunkBytes), fileOffset) < 0) {
                OSPanic("init.c", 617, "An error occurred when issuing read to /meminfo.bin\n");
            }
            rangeCount = chunkBytes/sizeof(struct memory_info);
            for(rangeIndex=0; rangeIndex<rangeCount; rangeIndex++) {
                OSReport("start: 0x%08x, end: 0x%08x\n", rangeEntries[rangeIndex].rangeStart,
                         rangeEntries[rangeIndex].rangeEnd);
                OSAllocFixed(&rangeEntries[rangeIndex].rangeStart,
                             &rangeEntries[rangeIndex].rangeEnd);
                OSReport("Removed 0x%08x - 0x%08x from the current heap\n",
                         rangeEntries[rangeIndex].rangeStart,
                         (u32) rangeEntries[rangeIndex].rangeEnd - 1);
            }
            remainingBytes -= chunkBytes;
            fileOffset += chunkBytes;
        }
        DVDClose(&fileInfo);
        OSDumpHeap(__OSCurrHeap);
    }
}
