/* Sprite drawing setup, texture upload, and layered 2D/3D sprite rendering. */
#define _MATH_H
#include "game/sprite.h"
#include "game/init.h"
#include "game/hu3d.h"
#include "game/disp.h"
#include "dolphin/mtx.h"
#include "dolphin/gx.h"
#include "dolphin/vi.h"

void HuSprTexLoad(ANIMDATA *anim, s16 bmpNo, s16 texMapId, GXTexWrapMode wrapS, GXTexWrapMode wrapT,
                  GXTexFilter filter);
;

typedef struct HuSprLayer_s {
    s16 drawNo; /* Sprite draw queue rendered by this 3D layer. */
    s16 layer; /* Hu3D layer that invokes the sprite draw callback. */
    s16 camera; /* Camera bit mask that enables this draw queue. */
} HUSPR_LAYER;

static void *bmpNoCC[8];
static HUSPR_LAYER HuSprLayer[HU3D_LAYER_HOOK_MAX];

static s16 bmpCCIdx;

void mtxTransCat(Mtx matrix, float x, float y, float z);

static void HuSprLayerHook(s16 layer);

/* The render pass calls this before 2D sprite queues to restore their GX state. */
void HuSprDispInit(void)
{
    Mtx44 proj;
    s16 textureSlot;
    for(textureSlot=0; textureSlot<8; textureSlot++) {
        bmpNoCC[textureSlot] = NULL;
    }
    bmpCCIdx = 0;
    GXInvalidateTexAll();
    MTXOrtho(proj, 0, HU_DISP_HEIGHT, 0, HU_DISP_WIDTH, 0, 10);
    GXSetProjection(proj, GX_ORTHOGRAPHIC);
    if(RenderMode->field_rendering) {
        GXSetViewportJitter(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, 0, 1, VIGetNextField());
    } else {
        GXSetViewport(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, 0, 1);
    }
    GXSetScissor(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP,
                  GX_AF_SPOT);
}

/* HuSprExec calls this for each visible sprite in the active draw queue. */
void HuSprDisp(HUSPRITE *sp)
{
    s16 layerIndex;
    ANIMDATA *anim = sp->data;
    ANIMPAT *pat = sp->patP;
    Vec axis = {0, 0, 1};
    Mtx modelview, rot;
    s16 chanSum;
    
    GXSetScissor(sp->scissorX, sp->scissorY, sp->scissorW, sp->scissorH);
    if(sp->attr & HUSPR_ATTR_FUNC) {
        if(sp->func) {
            HUSPR_FUNC func = sp->func;
            func(sp);
            HuSprDispInit();
        }
        
    } else {
        ANIMLAYER *layer;
        ANIMBMP *bgBmp;
        BOOL hasVtxColor;
        GXColor color;
        if(sp->attr & HUSPR_ATTR_VTXCOLOR) {
            hasVtxColor = TRUE;
            GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
            GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT0, GX_DF_CLAMP,
                          GX_AF_NONE);
            if(sp->attr & HUSPR_ATTR_VTXCOLOR_ADD) {
                GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
                GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                                GX_TEVPREV);
                GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
                GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                                GX_TEVPREV);
            } else {
                GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
            }
        } else {
            hasVtxColor = FALSE;
            GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP,
                          GX_AF_SPOT);
            GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
        }
        GXSetNumTexGens(1);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        color.r = color.g = color.b = color.a = 255;
        GXSetChanAmbColor(GX_COLOR0A0, color);
        GXSetChanMatColor(GX_COLOR0A0, color);
        color.r = sp->r;
        color.g = sp->g;
        color.b = sp->b;
        color.a = sp->a;
        chanSum = color.r+color.g+color.b+color.a;
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_C0, GX_CC_CPREV, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_A0, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetNumChans(1);
        if(sp->attr & HUSPR_ATTR_ADDCOL) {
            GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
        } else if(sp->attr & HUSPR_ATTR_INVCOL) {
            GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_INVDSTCLR, GX_LO_NOOP);
        } else {
            GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        }
        if(sp->bg) {
            ANIMPAT *bgPat;
            ANIMFRAME *bgFrame;
            bgFrame = sp->bg->bank[sp->bgBank].frame;
            bgPat = &sp->bg->pat[bgFrame->pat];
            layer = bgPat->layer;
            bgBmp = &sp->bg->bmp[layer->bmpNo];
            HuSprTexLoad(sp->bg, layer->bmpNo, 1, GX_CLAMP, GX_CLAMP, GX_NEAR);
            GXSetNumIndStages(1);
            GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_TRUE, bgBmp->sizeX*16, bgBmp->sizeY*16);
            GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP1);
            GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_16, GX_ITS_16);
            GXSetTevIndTile(GX_TEVSTAGE0, GX_INDTEXSTAGE0, 16, 16, 16, 16, GX_ITF_4, GX_ITM_0,
                            GX_ITB_NONE, GX_ITBA_OFF);
        }
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetZCompLoc(GX_FALSE);
        if(0 != sp->zRot) {
            MTXRotAxisDeg(rot, &axis, sp->zRot);
            MTXScale(modelview, sp->scale.x, sp->scale.y, 1.0f);
            MTXConcat(rot, modelview, modelview);
        } else {
            MTXScale(modelview, sp->scale.x, sp->scale.y, 1.0f);
        }
        mtxTransCat(modelview, sp->pos.x, sp->pos.y, 0);
        MTXConcat(*sp->groupMtx, modelview, modelview);
        GXLoadPosMtxImm(modelview, GX_PNMTX0);
        for(layerIndex=pat->layerNum-1; layerIndex>=0; layerIndex--) {
            HuVec2f pos[4];
            float uvX0, uvY0, uvX1, uvY1;
            ANIMBMP *bmp;
            layer = &pat->layer[layerIndex];
            bmp = &anim->bmp[layer->bmpNo];
            if(!bmp) {
                continue;
            }
            GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
            HuSprTexLoad(anim, layer->bmpNo, 0, sp->wrapS, sp->wrapT,
                         (sp->attr & HUSPR_ATTR_LINEAR) ? GX_LINEAR : GX_NEAR);
            if(layer->alpha != 255 || chanSum != 255*4) {
                color.a = (u16)(sp->a*layer->alpha) >> 8;
                GXSetTevColor(GX_TEVREG0, color);
                GXSetNumTevStages(2);
            } else {
                GXSetNumTevStages(1);
            }
            if(!sp->bg) {
                pos[0].x = layer->vtx[0]-pat->centerX;
                pos[0].y = layer->vtx[1]-pat->centerY;
                pos[1].x = layer->vtx[2]-pat->centerX;
                pos[1].y = layer->vtx[3]-pat->centerY;
                pos[2].x = layer->vtx[4]-pat->centerX;
                pos[2].y = layer->vtx[5]-pat->centerY;
                pos[3].x = layer->vtx[6]-pat->centerX;
                pos[3].y = layer->vtx[7]-pat->centerY;
                if(layer->flip & ANIM_LAYER_FLIPX) {
                    uvX1 = layer->startX/(float)bmp->sizeX;
                    uvX0 = (layer->startX+layer->sizeX)/(float)bmp->sizeX;
                } else {
                    uvX0 = layer->startX/(float)bmp->sizeX;
                    uvX1 = (layer->startX+layer->sizeX)/(float)bmp->sizeX;
                }
                if(layer->flip & ANIM_LAYER_FLIPY) {
                    uvY1 = layer->startY/(float)bmp->sizeY;
                    uvY0 = (layer->startY+layer->sizeY)/(float)bmp->sizeY;
                } else {
                    uvY0 = layer->startY/(float)bmp->sizeY;
                    uvY1 = (layer->startY+layer->sizeY)/(float)bmp->sizeY;
                }
            } else {
                pos[0].x = pos[3].x = -(bgBmp->sizeX*16)/2;
                pos[0].y = pos[1].y = -(bgBmp->sizeY*16)/2;
                pos[2].x = pos[1].x = pos[0].x+(bgBmp->sizeX*16);
                pos[2].y = pos[3].y = pos[0].y+(bgBmp->sizeY*16);
                uvX0 = uvY0 =  1.0/(bgBmp->sizeX*16);
                uvX1 = uvY1 = 1.0f;
            }
            if(sp->hook3D) {
                HUSPR_3DHOOK hook3D = sp->hook3D;
                HUSPR_RECT rectVtx;
                HUSPR_RECT rectST;
                rectVtx.x0 = pos[0].x;
                rectVtx.y0 = pos[0].y;
                rectVtx.x1 = pos[2].x;
                rectVtx.y1 = pos[2].y;
                rectST.x0 = uvX0*sp->uvScaleX;
                rectST.y0 = uvY0*sp->uvScaleY;
                rectST.x1 = uvX1*sp->uvScaleX;
                rectST.y1 = uvY1*sp->uvScaleY;
                hook3D(sp, &modelview, layerIndex, &rectVtx, &rectST);
            } else {
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                if(!hasVtxColor) {
                    GXPosition3f32(pos[0].x, pos[0].y, 0);
                    GXTexCoord2f32(uvX0*sp->uvScaleX, uvY0*sp->uvScaleY);
                    GXPosition3f32(pos[1].x, pos[1].y, 0);
                    GXTexCoord2f32(uvX1*sp->uvScaleX, uvY0*sp->uvScaleY);
                    GXPosition3f32(pos[2].x, pos[2].y, 0);
                    GXTexCoord2f32(uvX1*sp->uvScaleX, uvY1*sp->uvScaleY);
                    GXPosition3f32(pos[3].x, pos[3].y, 0);
                    GXTexCoord2f32(uvX0*sp->uvScaleX, uvY1*sp->uvScaleY);
                } else {
                    GXPosition3f32(pos[0].x, pos[0].y, 0);
                    GXColor4u8(sp->vtxColor[0].r, sp->vtxColor[0].g, sp->vtxColor[0].b,
                               sp->vtxColor[0].a);
                    GXTexCoord2f32(uvX0*sp->uvScaleX, uvY0*sp->uvScaleY);
                    GXPosition3f32(pos[1].x, pos[1].y, 0);
                    GXColor4u8(sp->vtxColor[1].r, sp->vtxColor[1].g, sp->vtxColor[1].b,
                               sp->vtxColor[1].a);
                    GXTexCoord2f32(uvX1*sp->uvScaleX, uvY0*sp->uvScaleY);
                    GXPosition3f32(pos[2].x, pos[2].y, 0);
                    GXColor4u8(sp->vtxColor[2].r, sp->vtxColor[2].g, sp->vtxColor[2].b,
                               sp->vtxColor[2].a);
                    GXTexCoord2f32(uvX1*sp->uvScaleX, uvY1*sp->uvScaleY);
                    GXPosition3f32(pos[3].x, pos[3].y, 0);
                    GXColor4u8(sp->vtxColor[3].r, sp->vtxColor[3].g, sp->vtxColor[3].b,
                               sp->vtxColor[3].a);
                    GXTexCoord2f32(uvX0*sp->uvScaleX, uvY1*sp->uvScaleY);
                }
                
                GXEnd();
            }
            
        }
        if(sp->bg) {
            GXSetNumIndStages(0);
            GXSetTevDirect(GX_TEVSTAGE0);
            GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_FALSE, 0, 0);
        }
        if(sp->hook3D || hasVtxColor) {
            HuSprDispInit();
        }
    }
}

/* Sprite drawing and other renderers call this to bind one animation bitmap. */
void HuSprTexLoad(ANIMDATA *anim, s16 bmpNo, s16 texMapId, GXTexWrapMode wrapS, GXTexWrapMode wrapT,
                  GXTexFilter filter)
{
    GXTexObj texObj;
    GXTlutObj tlutObj;
    ANIMBMP *bmp = &anim->bmp[bmpNo];
    s16 sizeX = bmp->sizeX;
    s16 sizeY = bmp->sizeY;
    switch(bmp->dataFmt & ANIM_BMP_FMTMASK) {
        case ANIM_BMP_RGBA8:
            GXInitTexObj(&texObj, bmp->data, sizeX, sizeY, GX_TF_RGBA8, wrapS, wrapT, GX_FALSE);
            break;
            
        case ANIM_BMP_RGB565:
        case ANIM_BMP_RGB5A3:
            GXInitTexObj(&texObj, bmp->data, sizeX, sizeY, GX_TF_RGB5A3, wrapS, wrapT, GX_FALSE);
            break;
            
        case ANIM_BMP_C8:
            /* Indexed bitmaps bind their palette before the texture object. */
            GXInitTlutObj(&tlutObj, bmp->palData, GX_TL_RGB5A3, bmp->palNum);
            GXLoadTlut(&tlutObj, texMapId);
            GXInitTexObjCI(&texObj, bmp->data, sizeX, sizeY, GX_TF_C8, wrapS, wrapT, GX_FALSE,
                           texMapId);
            break;
            
        case ANIM_BMP_C4:
            GXInitTlutObj(&tlutObj, bmp->palData, GX_TL_RGB5A3, bmp->palNum);
            GXLoadTlut(&tlutObj, texMapId);
            GXInitTexObjCI(&texObj, bmp->data, sizeX, sizeY, GX_TF_C4, wrapS, wrapT, GX_FALSE,
                           texMapId);
            break;
            
        case ANIM_BMP_IA8:
            GXInitTexObj(&texObj, bmp->data, sizeX, sizeY, GX_TF_IA8, wrapS, wrapT, GX_FALSE);
            break;
            
        case ANIM_BMP_IA4:
            GXInitTexObj(&texObj, bmp->data, sizeX, sizeY, GX_TF_IA4, wrapS, wrapT, GX_FALSE);
            break;
            
        case ANIM_BMP_I8:
            GXInitTexObj(&texObj, bmp->data, sizeX, sizeY, GX_TF_I8, wrapS, wrapT, GX_FALSE);
            break;
        
        case ANIM_BMP_I4:
            GXInitTexObj(&texObj, bmp->data, sizeX, sizeY, GX_TF_I4, wrapS, wrapT, GX_FALSE);
            break;
            
        case ANIM_BMP_A8:
            GXInitTexObj(&texObj, bmp->data, sizeX, sizeY, GX_CTF_A8, wrapS, wrapT, GX_FALSE);
            break;
            
        case ANIM_BMP_CMPR:
            GXInitTexObj(&texObj, bmp->data, sizeX, sizeY, GX_TF_CMPR, wrapS, wrapT, GX_FALSE);
            break;
            
        default:
            break;
    }
    GXInitTexObjLOD(&texObj, filter, filter, 0, 0, 0, GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&texObj, texMapId);
}

/* HuSprInit and HuSprClose clear the table of sprite queues on 3D layers. */
void HuSprExecLayerInit(void)
{
    s16 slotIndex;
    for(slotIndex=0; slotIndex<HU3D_LAYER_HOOK_MAX; slotIndex++) {
        HuSprLayer[slotIndex].layer = -1;
    }
}

/* Clients register a sprite queue for one camera mask and Hu3D layer. */
void HuSprExecLayerCameraSet(s16 drawNo, s16 camera, s16 layer)
{
    s16 slotIndex;
    
    for(slotIndex=0; slotIndex<HU3D_LAYER_HOOK_MAX; slotIndex++) {
        if(-1 == HuSprLayer[slotIndex].layer) {
            break;
        }
    }
    if(slotIndex == HU3D_LAYER_HOOK_MAX) {
        return;
    }
    HuSprLayer[slotIndex].layer = layer;
    HuSprLayer[slotIndex].camera = camera;
    HuSprLayer[slotIndex].drawNo = drawNo;
    Hu3DLayerHookSet(layer, HuSprLayerHook);
}

/* Clients register a sprite queue for a layer without a camera restriction. */
void HuSprExecLayerSet(s16 drawNo, s16 layer)
{
    s16 slotIndex;
    
    for(slotIndex=0; slotIndex<HU3D_LAYER_HOOK_MAX; slotIndex++) {
        if(-1 == HuSprLayer[slotIndex].layer) {
            break;
        }
    }
    if(slotIndex == HU3D_LAYER_HOOK_MAX) {
        return;
    }
    HuSprLayer[slotIndex].layer = layer;
    HuSprLayer[slotIndex].camera = -1;
    HuSprLayer[slotIndex].drawNo = drawNo;
    Hu3DLayerHookSet(layer, HuSprLayerHook);
}

/* Hu3D invokes this during the registered layer pass to draw its sprite queue for the active
 * camera. */
static void HuSprLayerHook(short layerNo)
{
    s16 slotIndex;
    for(slotIndex=0; slotIndex<HU3D_LAYER_HOOK_MAX; slotIndex++) {
        if(layerNo == HuSprLayer[slotIndex].layer) {
            break;
        }
    }
    if(slotIndex == HU3D_LAYER_HOOK_MAX) {
        return;
    }
    if((Hu3DCameraBit & HuSprLayer[slotIndex].camera) == 0) {
        return;
    }
    HuSprDispInit();
    HuSprExec(HuSprLayer[slotIndex].drawNo);
}

/* HuSpr3DSet installs this callback; HuSprDisp calls it for each sprite layer to draw its mesh. */
void HuSpr3DDisp(HUSPRITE *sp, Mtx *spriteModelView, s16 spriteLayerIndex, HUSPR_RECT *vertexRect,
                 HUSPR_RECT *texCoordRect)
{
    HUSPR_3DDATA *meshData;
    int vertexIndex;
    int columnCount;
    int columnIndex;
    int rowIndex;
    int rowCount;
    BOOL useVertexColors;
    
    float width;
    float height;
    float red;
    float green;
    float blue;
    float alpha;
    float projectionScale;
    float projectionDepth;
    float projectionCenterZ;
    float texCoordWidth;
    float texCoordHeight;
    float cellWidth;
    float cellHeight;
    float cellTexCoordWidth;
    float cellTexCoordHeight;
    float vertexDepth;
    
    Mtx44 projectionMatrix;
    Mtx rotationMatrix;
    
    meshData = sp->data3D;
    width = vertexRect->x1-vertexRect->x0;
    height = vertexRect->y1-vertexRect->y0;
    if(width > height) {
        /* Projection depth is three times the longer sprite dimension. */
        projectionDepth = 3*width;
    } else {
        projectionDepth = 3*height;
    }
    texCoordWidth = texCoordRect->x1-texCoordRect->x0;
    texCoordHeight = texCoordRect->y1-texCoordRect->y0;
    columnCount = meshData->col;
    rowCount = meshData->row;
    projectionCenterZ = -projectionDepth*0.5;
    
    MTXOrtho(projectionMatrix, 0, HU_DISP_HEIGHT, 0, HU_DISP_WIDTH, 0, 100);
    GXSetProjection(projectionMatrix, GX_ORTHOGRAPHIC);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetArray(GX_VA_POS, meshData->vtx, sizeof(HuVecF));
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VA_TEX0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetArray(GX_VA_TEX0, meshData->st, sizeof(HuVec2f));
    if(sp->attr & HUSPR_ATTR_VTXCOLOR) {
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        useVertexColors = TRUE;
    } else {
        useVertexColors = FALSE;
    }
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    cellWidth = width/columnCount;
    cellHeight = height/rowCount;
    cellTexCoordWidth = texCoordWidth/columnCount;
    cellTexCoordHeight = texCoordHeight/rowCount;
    for(rowIndex=0; rowIndex<=rowCount; rowIndex++) {
        for(columnIndex=0; columnIndex<=columnCount; columnIndex++) {
            vertexIndex = columnIndex+(rowIndex*(columnCount+1));
            meshData->vtx[vertexIndex].x = vertexRect->x0+(cellWidth*columnIndex);
            meshData->vtx[vertexIndex].y = vertexRect->y0+(cellHeight*rowIndex);
            meshData->vtx[vertexIndex].z = 0;
            meshData->st[vertexIndex].x = texCoordRect->x0+(cellTexCoordWidth*columnIndex);
            meshData->st[vertexIndex].y = texCoordRect->y0+(cellTexCoordHeight*rowIndex);
        }
    }
    DCFlushRangeNoSync(meshData->st, sizeof(HuVec2f)*(columnCount+1)*(rowCount+1));
    /* Rotate the grid, then project each vertex using its configured depth. */
    mtxRot(rotationMatrix, meshData->rot.x, meshData->rot.y, meshData->rot.z);
    MTXMultVecArray(rotationMatrix, meshData->vtx, meshData->vtx, (columnCount+1)*(rowCount+1));
    for(rowIndex=0; rowIndex<=rowCount; rowIndex++) {
        for(columnIndex=0; columnIndex<=columnCount; columnIndex++) {
            vertexIndex = columnIndex+(rowIndex*(columnCount+1));
            vertexDepth = -(projectionCenterZ+(meshData->depthScale*meshData->vtx[vertexIndex].z));
            projectionScale = -projectionCenterZ/vertexDepth;
            meshData->vtx[vertexIndex].x *= projectionScale;
            meshData->vtx[vertexIndex].y *= projectionScale;
            meshData->vtx[vertexIndex].z = -10;
        }
    }
    DCFlushRange(meshData->vtx, sizeof(HuVecF)*(columnCount+1)*(rowCount+1));
    GXBegin(GX_QUADS, GX_VTXFMT0, columnCount*rowCount*4);
    if(!useVertexColors) {
        for(rowIndex=0; rowIndex<rowCount; rowIndex++) {
            for(columnIndex=0; columnIndex<columnCount; columnIndex++) {
                vertexIndex = columnIndex+(rowIndex*(columnCount+1));
                GXPosition1x16(vertexIndex);
                GXTexCoord1x16(vertexIndex);
                GXPosition1x16(vertexIndex+1);
                GXTexCoord1x16(vertexIndex+1);
                GXPosition1x16(vertexIndex+columnCount+2);
                GXTexCoord1x16(vertexIndex+columnCount+2);
                GXPosition1x16(vertexIndex+columnCount+1);
                GXTexCoord1x16(vertexIndex+columnCount+1);
            }
        }
    } else {
        /* Interpolate corner RGBA values at each mesh vertex to shade the sprite. */
        for(rowIndex=0; rowIndex<rowCount; rowIndex++) {
            for(columnIndex=0; columnIndex<columnCount; columnIndex++) {
                vertexIndex = columnIndex+(rowIndex*(columnCount+1));
                GXPosition1x16(vertexIndex);
                width = (float)columnIndex/(float)columnCount;
                height = (float)rowIndex/(float)rowCount;
                red = sp->vtxColor[0].r*((1.0-width)*(1.0-height));
                green = sp->vtxColor[0].g*((1.0-width)*(1.0-height));
                blue = sp->vtxColor[0].b*((1.0-width)*(1.0-height));
                alpha = sp->vtxColor[0].a*((1.0-width)*(1.0-height));
                red += sp->vtxColor[1].r*(width*(1.0-height));
                green += sp->vtxColor[1].g*(width*(1.0-height));
                blue += sp->vtxColor[1].b*(width*(1.0-height));
                alpha += sp->vtxColor[1].a*(width*(1.0-height));
                red += sp->vtxColor[2].r*(width*height);
                green += sp->vtxColor[2].g*(width*height);
                blue += sp->vtxColor[2].b*(width*height);
                alpha += sp->vtxColor[2].a*(width*height);
                red += sp->vtxColor[3].r*((1.0-width)*height);
                green += sp->vtxColor[3].g*((1.0-width)*height);
                blue += sp->vtxColor[3].b*((1.0-width)*height);
                alpha += sp->vtxColor[3].a*((1.0-width)*height);
                GXColor4u8(red, green, blue, alpha);
                GXTexCoord1x16(vertexIndex);
                GXPosition1x16(vertexIndex+1);
                width = (float)(columnIndex+1)/(float)columnCount;
                height = (float)rowIndex/(float)rowCount;
                red = sp->vtxColor[0].r*((1.0-width)*(1.0-height));
                green = sp->vtxColor[0].g*((1.0-width)*(1.0-height));
                blue = sp->vtxColor[0].b*((1.0-width)*(1.0-height));
                alpha = sp->vtxColor[0].a*((1.0-width)*(1.0-height));
                red += sp->vtxColor[1].r*(width*(1.0-height));
                green += sp->vtxColor[1].g*(width*(1.0-height));
                blue += sp->vtxColor[1].b*(width*(1.0-height));
                alpha += sp->vtxColor[1].a*(width*(1.0-height));
                red += sp->vtxColor[2].r*(width*height);
                green += sp->vtxColor[2].g*(width*height);
                blue += sp->vtxColor[2].b*(width*height);
                alpha += sp->vtxColor[2].a*(width*height);
                red += sp->vtxColor[3].r*((1.0-width)*height);
                green += sp->vtxColor[3].g*((1.0-width)*height);
                blue += sp->vtxColor[3].b*((1.0-width)*height);
                alpha += sp->vtxColor[3].a*((1.0-width)*height);
                GXColor4u8(red, green, blue, alpha);
                GXTexCoord1x16(vertexIndex+1);
                GXPosition1x16(vertexIndex+columnCount+2);
                width = (float)(columnIndex+1)/(float)columnCount;
                height = (float)(rowIndex+1)/(float)rowCount;
                red = sp->vtxColor[0].r*((1.0-width)*(1.0-height));
                green = sp->vtxColor[0].g*((1.0-width)*(1.0-height));
                blue = sp->vtxColor[0].b*((1.0-width)*(1.0-height));
                alpha = sp->vtxColor[0].a*((1.0-width)*(1.0-height));
                red += sp->vtxColor[1].r*(width*(1.0-height));
                green += sp->vtxColor[1].g*(width*(1.0-height));
                blue += sp->vtxColor[1].b*(width*(1.0-height));
                alpha += sp->vtxColor[1].a*(width*(1.0-height));
                red += sp->vtxColor[2].r*(width*height);
                green += sp->vtxColor[2].g*(width*height);
                blue += sp->vtxColor[2].b*(width*height);
                alpha += sp->vtxColor[2].a*(width*height);
                red += sp->vtxColor[3].r*((1.0-width)*height);
                green += sp->vtxColor[3].g*((1.0-width)*height);
                blue += sp->vtxColor[3].b*((1.0-width)*height);
                alpha += sp->vtxColor[3].a*((1.0-width)*height);
                GXColor4u8(red, green, blue, alpha);
                GXTexCoord1x16(vertexIndex+columnCount+2);
                GXPosition1x16(vertexIndex+columnCount+1);
                width = (float)columnIndex/(float)columnCount;
                height = (float)(rowIndex+1)/(float)rowCount;
                red = sp->vtxColor[0].r*((1.0-width)*(1.0-height));
                green = sp->vtxColor[0].g*((1.0-width)*(1.0-height));
                blue = sp->vtxColor[0].b*((1.0-width)*(1.0-height));
                alpha = sp->vtxColor[0].a*((1.0-width)*(1.0-height));
                red += sp->vtxColor[1].r*(width*(1.0-height));
                green += sp->vtxColor[1].g*(width*(1.0-height));
                blue += sp->vtxColor[1].b*(width*(1.0-height));
                alpha += sp->vtxColor[1].a*(width*(1.0-height));
                red += sp->vtxColor[2].r*(width*height);
                green += sp->vtxColor[2].g*(width*height);
                blue += sp->vtxColor[2].b*(width*height);
                alpha += sp->vtxColor[2].a*(width*height);
                red += sp->vtxColor[3].r*((1.0-width)*height);
                green += sp->vtxColor[3].g*((1.0-width)*height);
                blue += sp->vtxColor[3].b*((1.0-width)*height);
                alpha += sp->vtxColor[3].a*((1.0-width)*height);
                GXColor4u8(red, green, blue, alpha);
                GXTexCoord1x16(vertexIndex+columnCount+1);
            }
        }
    }
}
