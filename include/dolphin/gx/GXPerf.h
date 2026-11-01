#ifndef _DOLPHIN_GXPERF
#define _DOLPHIN_GXPERF

#include <dolphin/types.h>
#include <dolphin/gx/GXEnum.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void GXSetGPMetric(GXPerf0 perf0, GXPerf1 perf1);
extern void GXClearGPMetric();
extern void GXReadXfRasMetric(u32* xf_wait_in, u32* xf_wait_out, u32* ras_busy, u32* clocks);

// Unused/inlined in P2.
extern void GXReadGPMetric(u32* cnt0, u32* cnt1);
extern u32 GXReadGP0Metric();
extern u32 GXReadGP1Metric();
extern void GXReadMemMetric(u32* cp_req, u32* tc_req, u32* cpu_rd_req, u32* cpu_wr_req,
    u32* dsp_req, u32* io_req, u32* vi_req, u32* pe_req,
                            u32* rf_req, u32* fi_req);
extern void GXClearMemMetric();
extern void GXReadPixMetric(u32* top_pixels_in, u32* top_pixels_out, u32* bot_pixels_in,
    u32* bot_pixels_out, u32* clr_pixels_in, u32* copy_clks);
extern void GXClearPixMetric();
extern void GXSetVCacheMetric(GXVCachePerf attr);
extern void GXReadVCacheMetric(u32* check, u32* miss, u32* stall);
extern void GXClearVCacheMetric();
extern void GXInitXfRasMetric();
extern u32 GXReadClksPerVtx();

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_GXPERF
