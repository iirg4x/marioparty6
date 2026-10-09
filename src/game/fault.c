// Draws the system panic message directly into the active video framebuffers.
#include "game/fault.h"
#include "stdarg.h"
#include "stdio.h"

#define XFB_Y_BLACK 0
#define XFB_Y_WHITE 255
#define XFB_Y_LIGHT_GRAY 192
#define XFB_Y_GRAY 128
#define XFB_Y_DARK_GRAY 64
#define XFB_CHROMA_NEUTRAL 128
#define XFB_WIDTH_ALIGNMENT 0xFFFFFFF0

typedef struct Xfb_Color_s {
    u8 luma; // Brightness written for each glyph pixel.
    u8 chromaCr; // Neutral chroma keeps grayscale text colorless.
    u8 chromaCb; // Neutral chroma keeps grayscale text colorless.
} XFB_COLOR;

typedef struct Xfb_Geometry_s {
    void *framebuffers[4]; // Registered panic targets; zero xFBmode selects pairs by y parity.
    u16 width; // Framebuffer width in pixels, rounded to a 16-pixel boundary.
    u16 height; // Framebuffer height in pixels.
    u16 fieldMode; // VI XFB mode; zero selects XFB_putcInterlace, including for 480p.
} XFB_GEOMETRY;

static XFB_COLOR XFB_Colors[5] = {
    { XFB_Y_BLACK, XFB_CHROMA_NEUTRAL, XFB_CHROMA_NEUTRAL },
    { XFB_Y_WHITE, XFB_CHROMA_NEUTRAL, XFB_CHROMA_NEUTRAL },
    { XFB_Y_LIGHT_GRAY, XFB_CHROMA_NEUTRAL, XFB_CHROMA_NEUTRAL },
    { XFB_Y_GRAY, XFB_CHROMA_NEUTRAL, XFB_CHROMA_NEUTRAL },
    { XFB_Y_DARK_GRAY, XFB_CHROMA_NEUTRAL, XFB_CHROMA_NEUTRAL }
};

#include "Ascii8x8_1bpp.inc"

static XFB_GEOMETRY XFB_Geometry;

static s32 (*XFB_putc)(u8 character, s32 x, s32 y);

static XFB_COLOR Draw_Color;

static s32 x_start;
static s32 y_start;

static s32 XFB_putcProgressive(u8 character, s32 x, s32 y);
static s32 XFB_putcInterlace(u8 character, s32 x, s32 y);
static s32 XFB_puts(s8* message, s32 x, s32 y);
static s32 XFB_putcS(u8 character, s32 x, s32 y);
static void XFB_WriteBackCache(void);
static void XFB_CR(s32 lineCount, s32* x, s32* y);

// OSPanic draws the panic title, source location and formatted message before PPCHalt
// enters its non-returning spin loop.
void OSPanic(const char* file, int line, const char* msg, ...) {
    static char* titleMes = "OSPanic encounterd:";

    va_list list;
    s32 x;
    s32 y;
    char messageBuffer[1024];
    s32 lineAdvances;

    x = x_start = 16;
    y = y_start = 32;
    lineAdvances = XFB_puts((s8*)titleMes, x, y);
    XFB_CR(lineAdvances + 1, &x, &y);
    sprintf(messageBuffer, "%s:%d", file, line);
    lineAdvances = XFB_puts((s8*)messageBuffer, x, y);
    XFB_CR(lineAdvances, &x, &y);
    va_start(list, msg);
    vsnprintf(messageBuffer, 1024U, msg, list);
    lineAdvances = XFB_puts((s8*)messageBuffer, x, y);
    XFB_CR(lineAdvances, &x, &y);
    XFB_WriteBackCache();
    PPCHalt();
    va_end(list);
}

// Called during video setup to clear buffer addresses, set white text, and select the
// direct renderer by xFBmode; zero selects XFB_putcInterlace, including for 480p.
void HuFaultInitXfbDirectDraw(GXRenderModeObj *mode) {
    s32 i;

    for (i = 0; i < 4; i++) {
        XFB_Geometry.framebuffers[i] = 0;
    }

    XFB_Geometry.width = 0;
    XFB_Geometry.height = 0;
    XFB_Geometry.fieldMode = 0;

    XFB_putc = XFB_putcProgressive;
    Draw_Color = XFB_Colors[1];

    if (mode) {
        XFB_Geometry.width = ((u16)mode->fbWidth + 15) & XFB_WIDTH_ALIGNMENT;
        XFB_Geometry.height = mode->xfbHeight;
        XFB_Geometry.fieldMode = mode->xFBmode;

        if (XFB_Geometry.fieldMode == 0) {
            XFB_putc = XFB_putcInterlace;
        } else {
            XFB_putc = XFB_putcProgressive;
        }
    }
}

// Called by video setup to assign a buffer that the panic renderer can draw into.
void HuFaultSetXfbAddress(s16 id, void *addr) {
    if (id >= 0 && id < 4) {
        XFB_Geometry.framebuffers[id] = addr;
    }
}

// Called by OSPanic after drawing; DCStoreRange writes each buffer's cached pixels back.
static void XFB_WriteBackCache(void) {
    s32 i;
    void *framebuffer;
    u32 size;

    size = XFB_Geometry.width * 2 * XFB_Geometry.height;

    if (size != 0) {
        for (i = 0; i < 4; i += 1) {
            framebuffer = XFB_Geometry.framebuffers[i];

            if (framebuffer) {
                DCStoreRange(framebuffer, size);
            }
        }
    }
}

// Resets x to x_start and advances 18 pixels plus (lineCount & 7) extra rows.
static void XFB_CR(s32 lineCount, s32* xPosition, s32* yPosition) {
    s32 extraLines;
    s32 y;
    s32 x;

    x = *xPosition;
    y = *yPosition;
    x = x_start;
    y += 18;

    extraLines = lineCount & 7;
    if (extraLines != 0) {
        y += extraLines * 18;
    }

    *xPosition = x;
    *yPosition = y;
}

// Called by OSPanic to draw panic text, honoring newlines and glyph wrapping.
// The terminating zero also reaches XFB_putcS and can add a final wrap to the return count.
static s32 XFB_puts(s8* message, s32 x, s32 y) {
    s32 lineCount;
    s32 wrappedLines;
    s8 character;

    lineCount = 0;

    do {
        character = *message++;

        if (character == '\n') {
            XFB_CR(0, &x, &y);

            lineCount += 1;
        } else {
            wrappedLines = XFB_putcS(character, x, y);

            if (wrappedLines >= 0) {
                if (wrappedLines != 0) {
                    wrappedLines -= 1;

                    XFB_CR(wrappedLines, &x, &y);

                    lineCount += wrappedLines + 1;
                }
                x += 16;
            } else {
                break;
            }
        }
    } while(character != 0);

    return lineCount;
}

// Called by XFB_puts to outline a glyph and wrap once when its starting x is too wide.
// Renderer returns are ignored; only this helper's own wrap is reported to XFB_puts.
static s32 XFB_putcS(u8 character, s32 x, s32 y) {
    XFB_COLOR previousColor;
    s32 wrappedLines;

    wrappedLines = 0;
    previousColor = Draw_Color;

    if (x + 17 >= XFB_Geometry.width) {
        XFB_CR(0, &x, &y);
        wrappedLines++;
    }

    Draw_Color = XFB_Colors[0];
    XFB_putc(character, x, y - 2);
    XFB_putc(character, x, y + 2);
    XFB_putc(character, x - 1, y);
    XFB_putc(character, x + 1, y);

    Draw_Color = previousColor;
    XFB_putc(character, x, y);

    return wrappedLines;
}

// Called by XFB_putcS to draw 16-by-16 glyphs into every registered framebuffer.
// Selected for nonzero xFBmode, including double-field interlaced video.
static s32 XFB_putcProgressive(u8 c, s32 x, s32 y) {
    s32 result;
    s32 pitch;
    u8 colorLuma;
    u8 colorCb;
    u8 colorCr;

    s32 i;
    s32 j;
    s32 framebufferOffset;
    u8 *glyphRow;
    s32 k;
    s32 width;
    s32 maskBit;
    u32 pixelMask;
    u32 glyphBits;
    u8* framebuffer;

    result = 0;

    if (c == 0) {
        return -1;
    }

    if (x + 16 >= XFB_Geometry.width) {
        y += 18;
        x = x_start;
        result = 1;
    }

    if (y + 16 >= XFB_Geometry.height) {
        return -1;
    }

    colorLuma = Draw_Color.luma;
    colorCr = Draw_Color.chromaCr;
    colorCb = Draw_Color.chromaCb;

    pitch = XFB_Geometry.width * 2;
    framebufferOffset = (x & 0xFFFE) * 2 + y * pitch;
    glyphRow = Ascii8x8_1bpp + (c * 8);

    i = 8;

    while (i != 0) {
        j = 2;

        while (j != 0) {
            for (k = 0; k < 4; k ++) {
                framebuffer = XFB_Geometry.framebuffers[k];

                if (framebuffer != 0) {
                    framebuffer += framebufferOffset;

                    glyphBits = *glyphRow;
                    pixelMask = 0;
                    maskBit = 0;
                    while (maskBit < 16) {
                        // The comparison binds before &: test bit 0 and duplicate it into two
                        // horizontal pixels.
                        if (glyphBits & 0xF != 0) {
                            pixelMask |= 3 << maskBit;
                        }
                        maskBit += 2;
                        glyphBits >>= 1;
                    }
                    width = 8;
                    if ((s32) (x & 1) != 0) {
                        pixelMask *= 2;
                        width = 10;
                    }

                    while (width != 0) {
                        if (pixelMask & 3) {
                            framebuffer[1] = colorCb;
                            framebuffer[3] = colorCr;

                            if (pixelMask & 1) {
                                framebuffer[0] = colorLuma;
                            }
                            if (pixelMask & 2) {
                                framebuffer[2] = colorLuma;
                            }
                        }
                        width -= 1;
                        framebuffer += 4;
                        pixelMask = pixelMask >> 2;
                    }
                }
            }

            j -= 1;
            framebufferOffset += pitch;
        }
        i -= 1;
        glyphRow += 1;
    }

    return result;
}

// Called by XFB_putcS for zero xFBmode, including 480p, to draw 8-by-8 glyphs.
// Selects buffer pairs by y parity and advances a full buffer stride for every glyph row.
static s32 XFB_putcInterlace(u8 c, s32 x, s32 y) {
    u8 colorLuma;
    u8 colorCb;
    u8 colorCr;
    s32 pitch;
    s32 i;
    s32 framebufferOffset;
    u8* glyphRow;
    s32 j;
    s16 bits;
    s32 width;
    u8* framebuffer;

    if (c == 0) {
        return -1;
    }

    if (x + 8 >= XFB_Geometry.width || y + 8 >= XFB_Geometry.height) {
        return -1;
    }

    colorLuma = Draw_Color.luma;
    colorCr = Draw_Color.chromaCr;
    colorCb = Draw_Color.chromaCb;

    pitch = XFB_Geometry.width * 2;
    framebufferOffset = ((x & 0xFFFE) * 2) + ((y >> 1) * pitch);
    glyphRow = Ascii8x8_1bpp + c * 8;

    i = 8;

    while (i != 0) {
        for (j = 0; j < 4; j += 2) {
            width = j;

            if ((s32) (y & 1) != 0) {
                width += 1;
            }

            framebuffer = XFB_Geometry.framebuffers[width];

            if (framebuffer) {
                framebuffer = framebuffer + framebufferOffset;
                bits = *glyphRow;
                width = 4;

                if (x & 1) {
                    bits = (s16)bits * 2;
                    width = 5;
                }

                while (width) {
                    if (bits & 3) {
                        framebuffer[1] = colorCb;
                        framebuffer[3] = colorCr;

                        if (bits & 1) {
                            framebuffer[0] = colorLuma;
                        }
                        if (bits & 2) {
                            framebuffer[2] = colorLuma;
                        }
                    }

                    width -= 1;
                    framebuffer += 4;
                    bits >>= 2;
                }
            }
        }

        i -= 1;
        y += 1;
        glyphRow += 1;
        framebufferOffset += pitch;
    }

    return 0;
}
