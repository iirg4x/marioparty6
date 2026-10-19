/* Builds and updates the writable glass surfaces and their pixel-reference grid. */
#include "REL/m613Dll/types.h"
#include "dolphin/os/OSCache.h"
#include <string.h>
#include "game/data.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/memory.h"

#define M613_MODEL_DATA_GROUP_FLAG 0x10000000

M613BitmapState lbl_1_bss_1C;
HSF_BITMAP *lbl_1_bss_18;
HSF_BITMAP *lbl_1_bss_14;
HSF_BITMAP *lbl_1_bss_10;
M613ValueKey *lbl_1_bss_C;
s32 lbl_1_bss_8;
extern s16 lbl_1_bss_1210[4];
extern void fn_1_D61C(void);
extern void fn_1_DD54(void);
extern void fn_1_DCF4(s16);
extern void fn_1_DE20(s16);
extern void HuAudFXPanning(s32, s16);
extern s32 fn_1_10C5C(Point3d *, Point3d *);
extern f64 fn_1_10E3C(f64, f64);

extern s32 fn_1_0(Point3d, f32, Point3d, f32);

extern void fn_1_9C7C(s16 playerIndex);
extern void fn_1_3FFC(f32 angleX, f32 angleY, f32 angleZ,
                     Vec source, Vec *destination);
extern s32 fn_1_70C(Point3d firstPosition, f32 angle, f32 firstRadius,
                    Point3d firstDirection, Point3d secondPosition,
                    f32 secondRadius, Point3d *secondDirection);
extern s32 fn_1_28C(Point3d firstPosition, Point3d *firstDirection,
                    f32 firstRadius, Point3d secondPosition,
                    Point3d *secondDirection, f32 secondRadius);
extern void fn_1_DBDC(s16 playerIndex);
extern f32 fn_1_99A4(Point3d movingPosition, Point3d displacement,
                     f32 radius, u8 side);
extern void omVibrate(s16 playerIndex, s16 duration, s16 strength, s16 count);
extern s32 HuAudFXPlay(s32 soundId);
extern void fn_1_B4A8(void);
extern s16 lbl_1_bss_0;

typedef struct {
    s16 dayLight;
    Vec dayPosition;
    Vec dayDirection;
    GXColor dayColor;
    s16 nightLight;
    Vec nightPosition;
    Vec nightDirection;
    GXColor nightColor;
} M613LightingData;

extern s32 fn_1_10C5C(Point3d *, Point3d *);

/* Walks and clips an integer pixel path; after the loop it clears drawPosition again, which repeats
 * the last sample or is unset if no step ran. */
#define M613_DRAW_SEGMENT(startX, startY, endX, endY)                       \
    do {                                                                    \
        drawCount = 0;                                                      \
        position.x = (f32)(s16)(startX);                                   \
        position.y = (f32)(s16)(startY);                                   \
        position.z = 0.0f;                                                 \
        direction.x = (f32)((s16)(endX) - (s16)(startX));                  \
        direction.y = (f32)((s16)(endY) - (s16)(startY));                  \
        direction.z = 0.0f;                                                \
        fn_1_10C5C(&direction, &direction);                                \
        while (((s32)((s16)(endX) - 1) > (s32)position.x) ||               \
               ((s32)position.x > (s32)((s16)(endX) + 1)) ||             \
               ((s32)((s16)(endY) - 1) > (s32)position.y) ||             \
               ((s32)position.y > (s32)((s16)(endY) + 1))) {             \
        drawPosition = position;                                      \
        if (drawPosition.x < 0.0f) drawPosition.x = 0.0f;              \
        if (drawPosition.x >= (f32)bitmapWidth)                        \
            drawPosition.x = (f32)(bitmapWidth - 1);                   \
        if (drawPosition.y < 0.0f) drawPosition.y = 0.0f;              \
        if (drawPosition.y >= (f32)bitmapHeight)                       \
            drawPosition.y = (f32)(bitmapHeight - 1);                  \
        fn_1_3998((s16)drawPosition.x, (s16)drawPosition.y);             \
        if (fn_1_3578(bitmap, (s16)drawPosition.x,                      \
                      (s16)drawPosition.y, pixelFormat) != 0)            \
                cleared += 1;                                               \
            drawCount += 1;                                                 \
            position.x = (f32)(s16)(startX) + direction.x * (f32)drawCount;\
            position.y = (f32)(s16)(startY) + direction.y * (f32)drawCount;\
            position.z = 0.0f;                                             \
        }                                                                   \
            fn_1_3998((s16)drawPosition.x, (s16)drawPosition.y);             \
            if (fn_1_3578(bitmap, (s16)drawPosition.x,                      \
                          (s16)drawPosition.y, pixelFormat) != 0)            \
                cleared += 1;                                               \
    } while (0)

void fn_1_11F8(void);
void fn_1_18FC(void);
void fn_1_1A7C(void);
void fn_1_1C24(s16 x, s16 y);
s16 fn_1_1D7C(void);
s16 fn_1_1D84(void);
s32 fn_1_1D8C(f32 coordinate);
s32 fn_1_1E64(f32 coordinate);
s16 fn_1_1EF4(s16 row, s16 column);
void fn_1_1F54(s16 row, s16 column, Point3d *position);
s32 fn_1_20D0(const Point3d *point0, const Point3d *point1,
              const Point3d *point2, const Point3d *point3);
s32 fn_1_2434(HSF_BITMAP *bitmap, s16 x0, s16 y0, s16 x1, s16 y1,
              s16 x2, s16 y2, s16 x3, s16 y3, s16 pixelFormat);
s32 fn_1_3578(HSF_BITMAP *bitmap, s16 x, s16 y, s16 pixelFormat);
s32 fn_1_3894(HSF_BITMAP *board, s16 pixelFormat);
void fn_1_3948(void);
void fn_1_3984(void);
void fn_1_3998(s16 value, s16 key);
s16 fn_1_3A10(void);
s16 fn_1_3A9C(void);
s32 fn_1_3B28(s16 key);
s16 fn_1_3BD4(s16 group);

M613LightingData lbl_1_data_0 = {
    0, {0.0f, 3000.0f, 3000.0f}, {0.0f, -1.0f, -1.0f}, {255,255,255,255},
    0, {0.0f, 3000.0f, 3000.0f}, {0.0f, -1.0f, -1.0f}, {64,64,64,64}
};

/* Called by the opening sequence to set lighting, board models, and glass bitmap state. */
void fn_1_11F8(void) {
    s32 initialReferenceCount;
    s16 lightMode = GwMgNightF;
    s16 nightMode = lightMode;
    HSF_DATA *modelData;
    s16 x;
    s16 y;

    if (nightMode == 0) {
        lbl_1_data_0.dayLight = Hu3DGLightCreateV(&lbl_1_data_0.dayPosition,
                                                  &lbl_1_data_0.dayDirection,
                                                  &lbl_1_data_0.dayColor);
        Hu3DGLightStaticSet(lbl_1_data_0.dayLight, 1);
        Hu3DGLightInfinitytSet(lbl_1_data_0.dayLight);
        lbl_1_data_0.nightLight = lbl_1_data_0.dayLight;
    } else {
        lbl_1_data_0.nightLight = Hu3DGLightCreateV(&lbl_1_data_0.nightPosition,
                                                    &lbl_1_data_0.nightDirection,
                                                    &lbl_1_data_0.nightColor);
        Hu3DGLightStaticSet(lbl_1_data_0.nightLight, 1);
        Hu3DGLightInfinitytSet(lbl_1_data_0.nightLight);
        lbl_1_data_0.dayLight = lbl_1_data_0.nightLight;
    }

    if (nightMode == 0) {
        lbl_1_bss_1C.modelIds[1] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m613, 0), M613_MODEL_DATA_GROUP_FLAG, HEAP_MODEL));
        lbl_1_bss_1C.modelIds[0] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m613, 2), M613_MODEL_DATA_GROUP_FLAG, HEAP_MODEL));
        lbl_1_bss_1C.modelIds[2] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m613, 4), M613_MODEL_DATA_GROUP_FLAG, HEAP_MODEL));
    } else {
        lbl_1_bss_1C.modelIds[1] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m613, 1), M613_MODEL_DATA_GROUP_FLAG, HEAP_MODEL));
        lbl_1_bss_1C.modelIds[0] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m613, 3), M613_MODEL_DATA_GROUP_FLAG, HEAP_MODEL));
        lbl_1_bss_1C.modelIds[2] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m613, 5), M613_MODEL_DATA_GROUP_FLAG, HEAP_MODEL));
    }

    Hu3DModelAttrSet(lbl_1_bss_1C.modelIds[0], HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(lbl_1_bss_1C.modelIds[1], HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(lbl_1_bss_1C.modelIds[2], HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet(lbl_1_bss_1C.modelIds[0], 1);
    Hu3DModelCameraSet(lbl_1_bss_1C.modelIds[1], 1);
    Hu3DModelCameraSet(lbl_1_bss_1C.modelIds[2], 1);
    Hu3DModelLayerSet(lbl_1_bss_1C.modelIds[0], 0);
    Hu3DModelLayerSet(lbl_1_bss_1C.modelIds[1], 2);
    Hu3DModelLayerSet(lbl_1_bss_1C.modelIds[2], 4);
    lbl_1_bss_1C.state = 0;
    Hu3DModelShadowMapSet(lbl_1_bss_1C.modelIds[0]);

    initialReferenceCount = 655;
    for (x = 0; x < 10; x++) {
        for (y = 0; y < 10; y++) {
            lbl_1_bss_1C.referenceCounts[x][y] = initialReferenceCount;
        }
    }

    lbl_1_bss_18 = NULL;
    lbl_1_bss_14 = NULL;
    lbl_1_bss_10 = NULL;
    modelData = Hu3DData[lbl_1_bss_1C.modelIds[1]].hsf;
    for (x = 0; x < modelData->attributeNum; x++) {
        HSF_BITMAP *bitmap = modelData->attribute[x].bitmap;
        if (nightMode == 0) {
            if ((strcmp(bitmap->name, "m613glass") == 0) && (lbl_1_bss_18 == 0)) {
                lbl_1_bss_18 = bitmap;
            } else if ((strcmp(bitmap->name, "m613glassG") == 0) && (lbl_1_bss_14 == 0)) {
                lbl_1_bss_14 = bitmap;
            } else if ((strcmp(bitmap->name, "m613wm") == 0) && (lbl_1_bss_10 == 0)) {
                lbl_1_bss_10 = bitmap;
            }
        } else if ((strcmp(bitmap->name, "m613Nglass") == 0) && (lbl_1_bss_18 == 0)) {
            lbl_1_bss_18 = bitmap;
        } else if ((strcmp(bitmap->name, "m613NglassG") == 0) && (lbl_1_bss_14 == 0)) {
            lbl_1_bss_14 = bitmap;
        } else if ((strcmp(bitmap->name, "m613wm") == 0) && (lbl_1_bss_10 == 0)) {
            lbl_1_bss_10 = bitmap;
        }
    }

    fn_1_3948();
    /* Border initialization uses format zero for all three bitmaps; later polygon updates use
     * format one for the bitmap named m613wm. */
    x = 0;
    y = 0;
    for (; x < 256; x++) {
        fn_1_3578(lbl_1_bss_18, x, y, 0);
        fn_1_3578(lbl_1_bss_14, x, y, 0);
        fn_1_3578(lbl_1_bss_10, x, y, 0);
    }
    x = 0;
    y = 255;
    for (; x < 256; x++) {
        fn_1_3578(lbl_1_bss_18, x, y, 0);
        fn_1_3578(lbl_1_bss_14, x, y, 0);
        fn_1_3578(lbl_1_bss_10, x, y, 0);
    }
    x = 0;
    for (y = 0; y < 256; y++) {
        fn_1_3578(lbl_1_bss_18, x, y, 0);
        fn_1_3578(lbl_1_bss_14, x, y, 0);
        fn_1_3578(lbl_1_bss_10, x, y, 0);
    }
    x = 255;
    for (y = 0; y < 256; y++) {
        fn_1_3578(lbl_1_bss_18, x, y, 0);
        fn_1_3578(lbl_1_bss_14, x, y, 0);
        fn_1_3578(lbl_1_bss_10, x, y, 0);
    }
}

/* Cycles through the three glass surfaces after updates; every fourth call flushes none. */
void fn_1_18FC(void) {
    if ((lbl_1_bss_1C.state & 3) == 1) {
        DCStoreRangeNoSync(lbl_1_bss_18->data,
                           lbl_1_bss_18->sizeX * lbl_1_bss_18->sizeY * lbl_1_bss_18->pixSize / 8);
    }
    if ((lbl_1_bss_1C.state & 3) == 2) {
        DCStoreRangeNoSync(lbl_1_bss_14->data,
                           lbl_1_bss_14->sizeX * lbl_1_bss_14->sizeY * lbl_1_bss_14->pixSize / 8);
    }
    if ((lbl_1_bss_1C.state & 3) == 3) {
        DCStoreRangeNoSync(lbl_1_bss_10->data,
                           (lbl_1_bss_10->sizeX * lbl_1_bss_10->sizeY * lbl_1_bss_10->pixSize / 4) /
                               2);
    }
    lbl_1_bss_1C.state += 1;
}

/* Called during result setup to hide the three board models and create the two day or night
* models used in their place. */
void fn_1_1A7C(void) {
    s16 selectedNight;
    s16 nightFlag;

    nightFlag = GwMgNightF;
    selectedNight = nightFlag;

    Hu3DModelAttrSet(lbl_1_bss_1C.modelIds[0], 1);
    Hu3DModelAttrSet(lbl_1_bss_1C.modelIds[2], 1);
    Hu3DModelAttrSet(lbl_1_bss_1C.modelIds[1], 1);
    if (selectedNight == 0) {
        lbl_1_bss_1C.modelIds[3] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m613, 6), M613_MODEL_DATA_GROUP_FLAG, HEAP_MODEL));
        lbl_1_bss_1C.modelIds[4] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m613, 8), M613_MODEL_DATA_GROUP_FLAG, HEAP_MODEL));
    } else {
        lbl_1_bss_1C.modelIds[3] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m613, 7), M613_MODEL_DATA_GROUP_FLAG, HEAP_MODEL));
        lbl_1_bss_1C.modelIds[4] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m613, 9), M613_MODEL_DATA_GROUP_FLAG, HEAP_MODEL));
    }
    Hu3DModelAttrSet(lbl_1_bss_1C.modelIds[3], HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(lbl_1_bss_1C.modelIds[4], HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet(lbl_1_bss_1C.modelIds[3], 1);
    Hu3DModelCameraSet(lbl_1_bss_1C.modelIds[4], 1);
    Hu3DModelLayerSet(lbl_1_bss_1C.modelIds[3], 0);
    Hu3DModelLayerSet(lbl_1_bss_1C.modelIds[4], 2);
    Hu3DModelShadowMapSet(lbl_1_bss_1C.modelIds[3]);
}

/* A cleared green-glass pixel uses mirrored coordinates to decrement its board cell, clamped at
 * zero. */
void fn_1_1C24(s16 x, s16 y)
{
    s16 row;
    s16 column;

    if (y < 0) {
        return;
    }
    if (y >= 256) {
        return;
    }
    if (x < 0) {
        return;
    }
    if (x >= 256) {
        return;
    }

    x = 255 - x;
    row = x / 25.6f;
    column = y / 25.6f;
    lbl_1_bss_1C.referenceCounts[row][column] -= 1;
    if (lbl_1_bss_1C.referenceCounts[row][column] < 0) {
        lbl_1_bss_1C.referenceCounts[row][column] = 0;
    }
}

/* Called by CPU target selection to report the number of board rows. */
s16 fn_1_1D7C(void) {
    return 10;
}

/* Called by CPU target selection to report the number of board columns. */
s16 fn_1_1D84(void) {
    return 10;
}

/* Clamps a board coordinate to -500..500 and converts it to a mirrored grid row. */
s32 fn_1_1D8C(f32 coordinate)
{
    f32 scale = 3.90625f;
    f32 limit = 500.0f;

    if (coordinate > limit) {
        coordinate = limit;
    }
    if (coordinate < -limit) {
        coordinate = -limit;
    }
    coordinate *= -1.0f;
    coordinate = 128.0f + (coordinate / scale);
    coordinate = (f32) (255 - (s16) coordinate);
    coordinate /= 25.6f;
    return (s32) coordinate;
}

/* Clamps a board coordinate to -500..500 and converts it to a grid column. */
s32 fn_1_1E64(f32 coordinate)
{
    f32 scale = 3.90625f;
    f32 limit = 500.0f;

    if (coordinate > limit) {
        coordinate = limit;
    }
    if (coordinate < -limit) {
        coordinate = -limit;
    }
    coordinate = 128.0f + (coordinate / scale);
    coordinate /= 25.6f;
    return (s32) coordinate;
}

/* Called by CPU target selection to read a cell's remaining green-glass pixel references. */
s16 fn_1_1EF4(s16 row, s16 column) {
    if ((column >= 0) && (column < 10) && (row >= 0) && (row < 10)) {
        return lbl_1_bss_1C.referenceCounts[row][column];
    }
    return 0;
}

/* Converts a cell to a CPU target position, adding 12.8 world units to X and Z after scaling. */
void fn_1_1F54(s16 row, s16 column, Point3d *position) {
    f32 xScale = 3.90625f;
    f32 zScale = 3.90625f;

    position->x = position->y = position->z = 0.0f;

    position->x = 25.6f * (f32) column;
    position->x -= 128.0f;
    position->x *= xScale;

    position->z = 25.6f * (f32) row;
    position->z = 255.0f - position->z;
    position->z -= 128.0f;
    position->z *= zScale;
    position->z *= -1.0f;

    position->x += 12.8f;
    position->z += 12.8f;
}

/* Called when a board shape is painted; the returned count is from the green glass only. */
s32 fn_1_20D0(const Point3d *point0, const Point3d *point1,
              const Point3d *point2, const Point3d *point3) {
    Point3d points[4];
    f32 coordinates[4][3];
    f32 scaleX;
    f32 scaleZ;
    s16 i;
    s32 result = 0;

    scaleX = 3.90625f;
    scaleZ = 3.90625f;

    points[0] = *point0;
    points[1] = *point1;
    points[2] = *point2;
    points[3] = *point3;

    for (i = 0; i < 4; i++) {
        points[i].z *= -1.0f;
        coordinates[i][0] = 128.0f + (points[i].x / scaleX);
        coordinates[i][2] = 128.0f + (points[i].z / scaleZ);
    }

    fn_1_2434(lbl_1_bss_18, (s16)coordinates[0][0], (s16)coordinates[0][2],
              (s16)coordinates[1][0], (s16)coordinates[1][2],
              (s16)coordinates[2][0], (s16)coordinates[2][2],
              (s16)coordinates[3][0], (s16)coordinates[3][2], 0);
    /* The caller uses only the green-glass count although all three surfaces are updated. */
    result = fn_1_2434(lbl_1_bss_14, (s16)coordinates[0][0], (s16)coordinates[0][2],
                       (s16)coordinates[1][0], (s16)coordinates[1][2],
                       (s16)coordinates[2][0], (s16)coordinates[2][2],
                       (s16)coordinates[3][0], (s16)coordinates[3][2], 0);
    fn_1_2434(lbl_1_bss_10, (s16)coordinates[0][0], (s16)coordinates[0][2],
              (s16)coordinates[1][0], (s16)coordinates[1][2],
              (s16)coordinates[2][0], (s16)coordinates[2][2],
              (s16)coordinates[3][0], (s16)coordinates[3][2], 1);
    return result;
}

/* Called by polygon painting to draw its outline and clear pixels enclosed by it. */
s32 fn_1_2434(HSF_BITMAP *bitmap, s16 x0, s16 y0, s16 x1, s16 y1,
              s16 x2, s16 y2, s16 x3, s16 y3, s16 pixelFormat) {
    s32 cleared = 0;
    s32 drawCount;
    s16 bitmapWidth = bitmap->sizeX;
    s16 bitmapHeight = bitmap->sizeY;
    Point3d direction;
    Point3d position;
    Point3d drawPosition;

    fn_1_3984();
    M613_DRAW_SEGMENT(x0, y0, x1, y1);
    M613_DRAW_SEGMENT(x1, y1, x2, y2);
    M613_DRAW_SEGMENT(x2, y2, x3, y3);
    M613_DRAW_SEGMENT(x3, y3, x0, y0);
    drawCount = fn_1_3894(bitmap, pixelFormat);
    cleared += drawCount;
    fn_1_3984();
    return cleared;
}

/* Clears an in-bounds pixel. Format zero reports whether its first byte was nonzero, while clearing
 * both bytes; format one tests and clears the selected nibble. */
s32 fn_1_3578(HSF_BITMAP *bitmap, s16 x, s16 y, s16 pixelFormat) {
    s32 width = bitmap->sizeX;
    s32 height = bitmap->sizeY;
    u8 *pixels = (u8 *)bitmap->data;
    s32 tileIndex;
    s32 pixelIndex;
    if ((x >= 0) && (x < width) && (y >= 0) && (y < height)) {
    if (pixelFormat == 0) {
        tileIndex = (x >> 2) + ((y >> 2) * (width >> 2));
        pixelIndex = ((y & 3) << 2) + (x & 3);

        pixelIndex = (tileIndex << 4) + pixelIndex;
        pixelIndex = pixelIndex << 1;
        if (pixels[pixelIndex] == 0) {
            pixels[pixelIndex] = 0;
            pixels[pixelIndex + 1] = 0;
        } else {
            pixels[pixelIndex] = 0;
            pixels[pixelIndex + 1] = 0;
            if (bitmap == lbl_1_bss_14) {
                fn_1_1C24(y, x);
            }
            return 1;
        }
    }

    if (pixelFormat == 1) {
        tileIndex = (x >> 3) + ((y >> 3) * (width >> 3));
        pixelIndex = ((y & 7) << 3) + (x & 7);
        pixelIndex = (tileIndex << 6) + pixelIndex;
        tileIndex = pixelIndex >> 1;
        if ((pixelIndex & 1) != 0) {
            if ((pixels[tileIndex] & 0xF) == 0) {
                pixels[tileIndex] = pixels[tileIndex] & 0xF0;
            } else {
                pixels[tileIndex] = pixels[tileIndex] & 0xF0;
                return 1;
            }
        } else {
            if ((pixels[tileIndex] & 0xF0) == 0) {
                pixels[tileIndex] = pixels[tileIndex] & 0xF;
            } else {
                pixels[tileIndex] = pixels[tileIndex] & 0xF;
                return 1;
            }
        }
    }
    }
    return 0;
}

/* Called after an outline to clear each recorded row from its smallest to largest outline
 * column. */
s32 fn_1_3894(HSF_BITMAP *board, s16 pixelFormat)
{
    s32 total = 0;
    s16 first_group = fn_1_3A10();
    s16 last_group = fn_1_3A9C();
    s32 group = (s16) first_group;

    while (group <= (s16) last_group) {
        s32 first_space = fn_1_3B28((s16) group);
        s16 last_space = fn_1_3BD4((s16) group);
        s32 space = (s16) first_space;

        while (space <= (s16) last_space) {
            if (fn_1_3578(board, (s16) space, (s16) group, pixelFormat) != 0) {
                total += 1;
            }
            space += 1;
        }
        group += 1;
    }
    return total;
}

/* Called during glass setup to allocate space for recorded outline pixels. */
void fn_1_3948(void) {
    lbl_1_bss_C = HuMemDirectMallocNum(0, 1200, HU_MEMNUM_OVL);
    fn_1_3984();
}

/* Called before and after each polygon fill to reset the recorded-pixel count. */
void fn_1_3984(void) {
    lbl_1_bss_8 = 0;
}

/* Called while drawing an outline to append its pixel coordinates to the fill list. */
void fn_1_3998(s16 value, s16 key) {
    if (lbl_1_bss_C != 0) {
        lbl_1_bss_C[lbl_1_bss_8].value = value;
        lbl_1_bss_C[lbl_1_bss_8].key = key;
        lbl_1_bss_8 += 1;
    }
}

/* Called by the fill pass to find the smallest recorded row. */
s16 fn_1_3A10(void) {
    s16 best = 2500;
    s16 i = 0;

    while (i < lbl_1_bss_8) {
        if (best > lbl_1_bss_C[i].key) {
            best = lbl_1_bss_C[i].key;
        }
        i += 1;
    }
    return best;
}

/* Called by the fill pass to find the largest recorded row. */
s16 fn_1_3A9C(void) {
    s16 best = -1;
    s16 i = 0;

    while (i < lbl_1_bss_8) {
        if (best < lbl_1_bss_C[i].key) {
            best = lbl_1_bss_C[i].key;
        }
        i += 1;
    }
    return best;
}

/* Called by the fill pass to find the smallest recorded column for one row. */
s32 fn_1_3B28(s16 key)
{
    s32 best = 2500;
    s16 i;

    i = 0;
    while (i < lbl_1_bss_8) {
        if (((s16) best > lbl_1_bss_C[i].value) &&
            (key == lbl_1_bss_C[i].key)) {
            best = lbl_1_bss_C[i].value;
        }
        i++;
    }
    return best;
}

/* Called by the fill pass to find the largest recorded column for one row. */
s16 fn_1_3BD4(s16 group) {
    s16 best = -1;
    s16 i = 0;
    while (i < lbl_1_bss_8) {
        if (best < lbl_1_bss_C[i].value && group == lbl_1_bss_C[i].key) {
            best = lbl_1_bss_C[i].value;
        }
        i++;
    }
    return best;
}

#undef M613_DRAW_SEGMENT
