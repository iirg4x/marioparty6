/* Queues and draws on-screen text and filled panels over the rendered frame. */
#include "dolphin.h"
#include "game/printfunc.h"
#include "game/init.h"
#include "game/disp.h"

#include "stdio.h"
#include "stdarg.h"

extern u8 ank8x8_4b[];

#define STRLINE_MAX 256
#define STRLINE_CHARMAX 80
/* Inline text controls consume the following byte as a scale or palette value. */
#define PF_CONTROL_SCALE 255
#define PF_CONTROL_COLOR 254
#define PF_CONTROL_SHADOW 253
/* A negative shadow palette value selects the regular glyph-only draw path. */
#define PF_SHADOW_NONE -1

typedef struct strline_s {
    u16 entryType; /* 0: text, 1: filled window. */
    u16 textColor; /* Index in the 16-entry font color table. */
    s16 screenX; /* Left edge in framebuffer pixels. */
    s16 screenY; /* Top edge in framebuffer pixels. */
    s16 windowWidth; /* Filled window width in framebuffer pixels. */
    s16 windowHeight; /* Filled window height in framebuffer pixels. */
    s16 nextFreeEntry; /* Next sequential slot; the last slot points one past the array. */
    float textScale; /* Base glyph size multiplier. */
    char text[STRLINE_CHARMAX]; /* Formatted text with inline color, shadow, and scale controls. */
    GXColor windowColor; /* RGBA fill color for a window entry. */
} STRLINE;

/* Per-frame queue shared by the print helpers and the main render pass. */
static STRLINE strline[STRLINE_MAX];
static char pfStrBuf[STRLINE_MAX];

/* Current font palette entry selected by callers before queuing text. */
int fontcolor;
/* Index of the next queue slot and number of entries queued this frame. */
u16 empstrline;
u16 strlinecnt;
/* Enables the red safe-area guide drawn with the debug overlays. */
BOOL saftyFrameF;

static void WireDraw(void);

static GXColor ATTRIBUTE_ALIGN(32) fcoltbl[16] = {
    { 0, 0, 0, 255 },
    { 0, 0, 128, 255 },
    { 128, 0, 0, 255 },
    { 128, 0, 128, 255 },
    { 0, 128, 0, 255 },
    { 0, 128, 128, 255 },
    { 128, 128, 0, 255 }, 
    { 128, 128, 128, 255 },
    { 128, 128, 128, 128 },
    { 0, 0, 255, 255 },
    { 255, 0, 0, 255 },
    { 255, 0, 255, 255 },
    { 0, 255, 0, 255 },
    { 0, 255, 255, 255 },
    { 255, 255, 0, 255 }, 
    { 255, 255, 255, 255 }
};

/* Resets the debug font state once during game startup. */
void pfInit(void)
{
    int lineIndex;
    fontcolor = FONT_COLOR_WHITE;
    empstrline = 0;
    
    for (lineIndex = 0; lineIndex < STRLINE_MAX; lineIndex++) {
        strline[lineIndex].text[0] = 0;
    }
    pfClsScr();
}

/* Clears queued overlays before the main-loop update and render pass. */
void pfClsScr(void)
{
    int lineIndex;
    empstrline = 0;
    strlinecnt = 0;
    for (lineIndex = 0; lineIndex < STRLINE_MAX; lineIndex++) {
        strline[lineIndex].nextFreeEntry = lineIndex+1;
        strline[lineIndex].entryType = 0;
        if (strline[lineIndex].text[0] != 0) {
            strline[lineIndex].text[0] = 0;
        }
    }
}

/* Queues formatted text using the current font color; returns -1 if all 256 frame entries are
 * queued. */
s16 print8(s16 screenX, s16 screenY, float textScale, char *format, ...)
{
    STRLINE *lineEntry;
    char *formattedText = pfStrBuf;
    char *lineText;
    s16 entryIndex;
    va_list arguments;
    lineEntry = &strline[empstrline];
    if(strlinecnt >= STRLINE_MAX) {
        return -1;
    }
    va_start(arguments, format);
    vsprintf(pfStrBuf, format, arguments);
    strlinecnt++;
    entryIndex = empstrline;
    empstrline = lineEntry->nextFreeEntry;
    lineEntry->entryType = 0;
    lineEntry->textColor = fontcolor;
    lineEntry->screenX = screenX;
    lineEntry->screenY = screenY;
    lineEntry->textScale = textScale;
    lineText = lineEntry->text;
    while(*formattedText) {
        *lineText++ = *formattedText++;
    }
    *lineText = 0;
    va_end(arguments);
    return entryIndex;
}

/* Queues a filled panel by copying its RGBA color; returns -1 if all 256 frame entries are
 * queued. */
s16 printWin(s16 screenX, s16 screenY, s16 windowWidth, s16 windowHeight, GXColor *fillColor)
{
    STRLINE *lineEntry;
    s16 entryIndex;
    char *formattedTextBuffer = pfStrBuf;
    lineEntry = &strline[empstrline];
    if(strlinecnt >= STRLINE_MAX) {
        return -1;
    }
    strlinecnt++;
    entryIndex = empstrline;
    empstrline = lineEntry->nextFreeEntry;
    lineEntry->entryType = 1;
    lineEntry->windowColor.r = fillColor->r;
    lineEntry->windowColor.g = fillColor->g;
    lineEntry->windowColor.b = fillColor->b;
    lineEntry->windowColor.a = fillColor->a;
    lineEntry->screenX = screenX;
    lineEntry->screenY = screenY;
    lineEntry->windowWidth = windowWidth;
    lineEntry->windowHeight = windowHeight;
    return entryIndex;
}

/* Called by main after game rendering to draw queued overlays before presenting the frame. */
void pfDrawFonts(void)
{
    GXTexObj fontTexture;
    Mtx44 projection;
    Mtx modelView;
    int entryIndex;
    s16 screenX, screenY, windowWidth, windowHeight;
    
    u16 queuedEntryCount = strlinecnt;
    if(saftyFrameF) {
        WireDraw();
    }
    MTXOrtho(projection, 0, HU_FB_HEIGHT, 0, HU_FB_WIDTH, 0, 10);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    MTXIdentity(modelView);
    GXLoadPosMtxImm(modelView, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
    GXSetViewport(0, 0, RenderMode->fbWidth, RenderMode->efbHeight, 0, 1);
    GXSetScissor(0, 0, RenderMode->fbWidth, RenderMode->efbHeight);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetArray(GX_VA_CLR0, fcoltbl, sizeof(GXColor));
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXInvalidateTexAll();
    GXInitTexObj(&fontTexture, ank8x8_4b, 128, 128, GX_TF_I4, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GXInitTexObjLOD(&fontTexture, GX_NEAR, GX_NEAR, 0, 0, 0, GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&fontTexture, GX_TEXMAP0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT0, GX_DF_CLAMP,
                  GX_AF_SPOT);
    GXSetZCompLoc(GX_FALSE);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetAlphaUpdate(GX_TRUE);
    for(entryIndex=0; entryIndex<STRLINE_MAX; entryIndex++) {
        screenX = strline[entryIndex].screenX;
        screenY = strline[entryIndex].screenY;
        if(strline[entryIndex].entryType == 1) {
            windowWidth = strline[entryIndex].windowWidth;
            windowHeight = strline[entryIndex].windowHeight;
            GXClearVtxDesc();
            GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_S16, 0);
            GXSetTevColor(GX_TEVREG0, strline[entryIndex].windowColor);
            GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
            GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                            GX_TEVPREV);
            GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
            GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                            GX_TEVPREV);
            GXSetNumTevStages(1);
            GXSetNumTexGens(0);
            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            GXPosition2s16(screenX, screenY);
            GXPosition2s16(screenX+windowWidth, screenY);
            GXPosition2s16(screenX+windowWidth, screenY+windowHeight);
            GXPosition2s16(screenX, screenY+windowHeight);
            GXEnd();
            GXClearVtxDesc();
            GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
            GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
            GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
            GXSetArray(GX_VA_CLR0, fcoltbl, sizeof(GXColor));
            GXSetNumTevStages(1);
            GXSetNumTexGens(1);
            GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
            GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
            GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
        } else {
            if(strline[entryIndex].text[0] != '\0') {
                float shadowOffsetX, shadowOffsetY;
                float glyphWidth;
                float glyphHeight;
                float glyphTexU, glyphTexV;
                char *textCursor;
                u16 glyphColor;
                s16 shadowColorIndex;
                float glyphScale;
                glyphWidth = glyphHeight = 8.0f*strline[entryIndex].textScale;
                textCursor = strline[entryIndex].text;
                glyphColor = strline[entryIndex].textColor;
                shadowColorIndex = PF_SHADOW_NONE;
                glyphScale = 1.0f;
                while(*textCursor) {
                    char glyphCode = *textCursor++;
                    switch(glyphCode) {
                        case PF_CONTROL_SCALE:
                            glyphCode = *textCursor++;
                            glyphScale = glyphCode/16.0f;
                            glyphWidth = 8.0f*strline[entryIndex].textScale*glyphScale;
                            glyphHeight = 8.0f*strline[entryIndex].textScale*glyphScale;
                            break;
                            
                        case PF_CONTROL_COLOR:
                            glyphColor = (*textCursor++)-1;
                            break;
                            
                        case PF_CONTROL_SHADOW:
                            shadowColorIndex = (*textCursor++)-1;
                            shadowOffsetX = 1.3333333f*strline[entryIndex].textScale*glyphScale;
                            shadowOffsetY = 1.3333333f*strline[entryIndex].textScale*glyphScale;
                            break;
                            
                        default:
                            glyphTexU = (glyphCode%16)/16.0f;
                            glyphTexV = ((glyphCode/16)/16.0f)+(1/128.0f);
                            if(shadowColorIndex < 0) {
                                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                                GXPosition3s16(screenX, screenY, 0);
                                GXColor1x8(glyphColor);
                                GXTexCoord2f32(glyphTexU, glyphTexV);
                                GXPosition3s16(screenX+glyphWidth, screenY, 0);
                                GXColor1x8(glyphColor);
                                GXTexCoord2f32(glyphTexU+(1/16.0f), glyphTexV);
                                GXPosition3s16(screenX+glyphWidth, screenY+glyphHeight, 0);
                                GXColor1x8(glyphColor);
                                GXTexCoord2f32(glyphTexU+(1/16.0f), glyphTexV+(1/16.0f));
                                GXPosition3s16(screenX, screenY+glyphHeight, 0);
                                GXColor1x8(glyphColor);
                                GXTexCoord2f32(glyphTexU, glyphTexV+(1/16.0f));
                                GXEnd();
                            } else {
                                GXBegin(GX_QUADS, GX_VTXFMT0, 8);
                                GXPosition3s16(screenX+shadowOffsetX, screenY+shadowOffsetY, 0);
                                GXColor1x8(shadowColorIndex);
                                GXTexCoord2f32(glyphTexU, glyphTexV);
                                GXPosition3s16(screenX + glyphWidth + shadowOffsetX,
                                               screenY + shadowOffsetY, 0);
                                GXColor1x8(shadowColorIndex);
                                GXTexCoord2f32(glyphTexU+(1/16.0f), glyphTexV);
                                GXPosition3s16(screenX + glyphWidth + shadowOffsetX,
                                               screenY + glyphHeight + shadowOffsetY, 0);
                                GXColor1x8(shadowColorIndex);
                                GXTexCoord2f32(glyphTexU+(1/16.0f), glyphTexV+(1/16.0f));
                                GXPosition3s16(screenX + shadowOffsetX,
                                               screenY + glyphHeight + shadowOffsetY, 0);
                                GXColor1x8(shadowColorIndex);
                                GXTexCoord2f32(glyphTexU, glyphTexV+(1/16.0f));
                                GXPosition3s16(screenX, screenY, 0);
                                GXColor1x8(glyphColor);
                                GXTexCoord2f32(glyphTexU, glyphTexV);
                                GXPosition3s16(screenX+glyphWidth, screenY, 0);
                                GXColor1x8(glyphColor);
                                GXTexCoord2f32(glyphTexU+(1/16.0f), glyphTexV);
                                GXPosition3s16(screenX+glyphWidth, screenY+glyphHeight, 0);
                                GXColor1x8(glyphColor);
                                GXTexCoord2f32(glyphTexU+(1/16.0f), glyphTexV+(1/16.0f));
                                GXPosition3s16(screenX, screenY+glyphHeight, 0);
                                GXColor1x8(glyphColor);
                                GXTexCoord2f32(glyphTexU, glyphTexV+(1/16.0f));
                                GXEnd();
                            }
                            /* Text wraps to the left edge after crossing the framebuffer width. */
                            screenX += glyphWidth;
                            if(screenX > HU_FB_WIDTH) {
                                screenX = 0;
                                screenY += glyphHeight;
                            }
                            break;
                    }
                }
            }
        }
    }
}

#define SAFETY_W 16
#define SAFETY_H 40

/* Called by pfDrawFonts to draw the red safe-area guide when its debug flag is enabled. */
static void WireDraw(void)
{
    Mtx44 projection;
    Mtx modelView;
    MTXOrtho(projection, 0, HU_DISP_HEIGHT, 0, HU_DISP_WIDTH, 0, 10);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    if(RenderMode->field_rendering) {
        GXSetViewportJitter(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, 0, 1, VIGetNextField());
    } else {
        GXSetViewport(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, 0, 1);
    }
    GXSetScissor(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGB8, 0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_RASC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP,
                  GX_AF_NONE);
    MTXIdentity(modelView);
    GXLoadPosMtxImm(modelView, GX_PNMTX0);
    GXBegin(GX_LINES, 0, 8);
    GXPosition2f32(SAFETY_W, SAFETY_H);
    GXColor3u8(255, 0, 0);
    GXPosition2f32(SAFETY_W, HU_DISP_HEIGHT-SAFETY_H);
    GXColor3u8(255, 0, 0);
    GXPosition2f32(SAFETY_W, SAFETY_H);
    GXColor3u8(255, 0, 0);
    GXPosition2f32(HU_DISP_WIDTH-SAFETY_W, SAFETY_H);
    GXColor3u8(255, 0, 0);
    GXPosition2f32(HU_DISP_WIDTH-SAFETY_W, HU_DISP_HEIGHT-SAFETY_H);
    GXColor3u8(255, 0, 0);
    GXPosition2f32(HU_DISP_WIDTH-SAFETY_W, SAFETY_H);
    GXColor3u8(255, 0, 0);
    GXPosition2f32(HU_DISP_WIDTH-SAFETY_W, HU_DISP_HEIGHT-SAFETY_H);
    GXColor3u8(255, 0, 0);
    GXPosition2f32(SAFETY_W, HU_DISP_HEIGHT-SAFETY_H);
    GXColor3u8(255, 0, 0);
    GXEnd();
}
