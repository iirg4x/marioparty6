#ifndef _DOLPHIN_VI
#define _DOLPHIN_VI

#include <dolphin/gx/GXStruct.h>
#include <dolphin/vifuncs.h>

#ifdef __cplusplus
extern "C" {
#endif

void VIInit(void);
void VIConfigure(const GXRenderModeObj* rm);
void VIFlush(void);
u32 VIGetTvFormat(void);
void VISetNextFrameBuffer(void* fb);
void VIWaitForRetrace(void);
void VISetBlack(BOOL black);

#ifdef TARGET_PC
void VISetWindowTitle(const char* title);
void VISetWindowFullscreen(bool fullscreen);
bool VIGetWindowFullscreen();
#endif

void VIConfigurePan(u16 xOrg, u16 yOrg, u16 width, u16 height);
u32 VIGetRetraceCount(void);
u32 VIGetNextField(void);
u32 VIGetCurrentLine(void);
u32 VIGetDTVStatus(void);
VIRetraceCallback VISetPreRetraceCallback(VIRetraceCallback callback);
VIRetraceCallback VISetPostRetraceCallback(VIRetraceCallback callback);
#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_VI
