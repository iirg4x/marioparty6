#ifndef _DOLPHIN_GXTEXTURE
#define _DOLPHIN_GXTEXTURE

#include <dolphin/gx/GXEnum.h>
#include <dolphin/gx/GXStruct.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef GXTexRegion* (*GXTexRegionCallback)(const GXTexObj* obj, GXTexMapID id);
typedef GXTlutRegion* (*GXTlutRegionCallback)(u32 idx);

void GXInitTexObj(GXTexObj* obj, void* image_ptr, u16 width, u16 height, GXTexFmt format,
                  GXTexWrapMode wrap_s, GXTexWrapMode wrap_t, u8 mipmap);
void GXInitTexObjCI(GXTexObj* obj, void* image_ptr, u16 width, u16 height, GXCITexFmt format,
                    GXTexWrapMode wrap_s, GXTexWrapMode wrap_t, u8 mipmap, u32 tlut_name);
void GXInitTexObjData(GXTexObj* obj, void* data);
void GXInitTexObjLOD(GXTexObj* obj, GXTexFilter min_filt, GXTexFilter mag_filt, f32 min_lod,
                     f32 max_lod, f32 lod_bias, GXBool bias_clamp, GXBool do_edge_lod,
                     GXAnisotropy max_aniso);
void GXLoadTexObj(const GXTexObj* obj, GXTexMapID id);
void GXLoadTexObjPreLoaded(const GXTexObj* obj, const GXTexRegion* region, GXTexMapID id);
u32 GXGetTexBufferSize(u16 width, u16 height, u32 format, GXBool mipmap, u8 max_lod);
void GXInvalidateTexAll(void);
void GXInitTexObjWrapMode(GXTexObj* obj, GXTexWrapMode s, GXTexWrapMode t);
void GXInitTlutObj(GXTlutObj* tlut_obj, void* lut, GXTlutFmt fmt, u16 n_entries);
void GXLoadTlut(const GXTlutObj* tlut_obj, u32 tlut_name);
void GXSetTexCoordScaleManually(GXTexCoordID coord, GXBool enable, u16 ss, u16 ts);
void GXInitTexCacheRegion(GXTexRegion* region, GXBool is_32b_mipmap, u32 tmem_even,
                          GXTexCacheSize size_even, u32 tmem_odd, GXTexCacheSize size_odd);
GXTexRegionCallback GXSetTexRegionCallback(GXTexRegionCallback f);
void GXInitTlutRegion(GXTlutRegion* region, u32 tmem_addr, GXTlutSize tlut_size);
GXTlutRegionCallback GXSetTlutRegionCallback(GXTlutRegionCallback f);
void GXInvalidateTexRegion(GXTexRegion* region);

GXBool GXGetTexObjMipMap(const GXTexObj* to);
GXTexFmt GXGetTexObjFmt(const GXTexObj* to);
void __GXSetSUTexRegs(void);
void __GXSetTmemConfig(u32 config);
void __GetImageTileCount(GXTexFmt fmt, u16 wd, u16 ht, u32 *rowTiles, u32 *colTiles, u32 *cmpTiles);
#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_GXTEXTURE
