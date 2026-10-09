// Decode packed game resources into their destination buffers.
#include "game/data.h"
#include "game/memory.h"
#include "dolphin/os.h"
#include "zlib.h"

typedef struct Decode_s
{
    u8 *src;  // Current byte in the packed source data.
    u8 *dst;  // Next byte to write in the decoded destination.
    u32 size; // Output bytes remaining for size-driven decoders; HuDecodeZlib does not use this
              // field.
} DECODE;

static u8 textBuffer[1024];

#define LZ_CONTROL_BITS_RELOAD_MASK 0xFF00
#define LZ_BACKREF_LENGTH_MASK 0x3F
#define RLE_LITERAL_RUN_FLAG 0x80

static int HuDecodeZlib(DECODE *decode);

// Copy uncompressed bytes directly; called by HuDecodeData for the NONE format.
static void HuDecodeNone(DECODE *decode)
{
    while(decode->size) {
        *decode->dst++ = *decode->src++;
        decode->size--;
    }
}

// Expand LZ data: consume control bits least-significant first (1 = literal, 0 = back-reference);
// the 1024-byte history ring starts zeroed at position 958.
static void HuDecodeLz(DECODE *decode)
{
    u16 controlBits, windowPos;
    s32 windowReadPos, copyIndex, copyLen;
    controlBits = 0;
    windowPos = 958;

    for(windowReadPos=0; windowReadPos<1024; windowReadPos++) {
        textBuffer[windowReadPos] = 0;
    }
    while(decode->size) {
        controlBits >>= 1;
        if(!(controlBits & 0x100)) {
            controlBits = (*decode->src++)|LZ_CONTROL_BITS_RELOAD_MASK;
        }
        // A 1 control bit copies one source literal; a 0 reads a back-reference from the history
        // ring.
        if(controlBits & 0x1) {
            textBuffer[windowPos++] = *decode->dst++ = *decode->src++;
            windowPos = windowPos & 0x3FF;
            decode->size--;
        } else {
            windowReadPos = *decode->src++;
            copyLen = *decode->src++;
            // The second token byte's high two bits extend the first byte to a 10-bit history
            // position; its low six bits give the copy length minus three.
            windowReadPos |= ((copyLen & ~LZ_BACKREF_LENGTH_MASK) << 2);
            copyLen = (copyLen & LZ_BACKREF_LENGTH_MASK)+3;
            for(copyIndex=0; copyIndex<copyLen; copyIndex++) {
                textBuffer[windowPos++] = *decode->dst++ =
                    textBuffer[(windowReadPos + copyIndex) & 0x3FF];
                windowPos &= 0x3FF;
            }
            decode->size -= copyIndex;
        }
    }
}

// Read one 32-bit big-endian word from the packed stream and advance src by four bytes.
#define SlideReadUint(decode, dest) \
    (*(dest)) = (*(decode)->src++) << 24; \
    (*(dest)) += (*(decode)->src++) << 16; \
    (*(dest)) += (*(decode)->src++) << 8; \
    (*(dest)) += (*(decode)->src++) << 0

// HuDecodeData calls this for SLIDE resources; early references before output start become zeroes.
static void HuDecodeSlide(DECODE *decode)
{
    u8 *outputStart;
    u32 controlBitsRemaining, controlBits;
    u32 declaredOutputSize;
    SlideReadUint(decode, &declaredOutputSize);
    // The stream's declared size is read but this decoder uses decode->size instead.
    controlBitsRemaining = 0;
    controlBits = 0;
    outputStart = decode->dst;
    while(decode->size) {
        if(controlBitsRemaining == 0) {
            SlideReadUint(decode, &controlBits);
            controlBitsRemaining = 32;
        }
        // Consume control bits most-significant first: 1 is a literal byte; 0 is a back-reference.
        if(controlBits >> 31) {
            *decode->dst++ = (s32)*decode->src++;
            decode->size--;
        } else {
            u8 *backrefCursor;
            u32 distance, runLength;
            // The token's high nibble is length (zero adds a following byte plus 18; otherwise add
            // 2); its low 12 bits are the offset, and reads start at dst - offset - 1.
            distance = *decode->src++ << 8;
            distance = distance+(*decode->src++);
            runLength = (distance >> 12) & 0xF;
            distance &= 0xFFF;
            backrefCursor = decode->dst-distance;
            (void)distance;
            if(runLength == 0) {
                runLength = (*decode->src++)+18;
            } else {
                runLength += 2;
            }
            decode->size -= runLength;
            while(runLength) {
                if(backrefCursor-1 < outputStart) {
                    *decode->dst++ = 0;
                } else {
                    *decode->dst++ = backrefCursor[-1];
                }
                runLength--;
                backrefCursor++;
            }
        }

        controlBits <<= 1;
        controlBitsRemaining--;
    }
}

// Both FSLIDE types share this decoder; back-references copy directly from prior output without
// SLIDE's pre-start zero-fill guard.
static void HuDecodeFslide(DECODE *decode)
{
    u32 controlBitsRemaining, controlBits;
    u32 declaredOutputSize;
    SlideReadUint(decode, &declaredOutputSize);
    // The stream's declared size is read but this decoder uses decode->size instead.
    controlBitsRemaining = 0;
    controlBits = 0;
    while(decode->size) {
        if(controlBitsRemaining == 0) {
            SlideReadUint(decode, &controlBits);
            controlBitsRemaining = 32;
        }
        // Consume control bits most-significant first: 1 is a literal byte; 0 is a back-reference.
        if(controlBits >> 31) {
            *decode->dst++ = (s32)*decode->src++;
            decode->size--;
        } else {
            u8 *backrefCursor;
            u32 distance, runLength;
            // The token's high nibble is length (zero adds a following byte plus 18; otherwise add
            // 2); its low 12 bits are the offset, and reads start at dst - offset - 1.
            distance = *decode->src++ << 8;
            distance += *decode->src++;
            runLength = (distance >> 12) & 0xF;
            distance &= 0xFFF;
            backrefCursor = decode->dst-distance;
            (void)distance;
            if(runLength == 0) {
                runLength = (*decode->src++)+18;
            } else {
                runLength += 2;
            }
            decode->size -= runLength;
            while(runLength) {
                *decode->dst++ = backrefCursor[-1];
                runLength--;
                backrefCursor++;
            }
        }

        controlBits <<= 1;
        controlBitsRemaining--;
    }
}

// HuDecodeData calls this for RLE resources, expanding fill runs and literal byte runs.
static void HuDecodeRle(DECODE *decode)
{
    s32 runIndex;
    while(decode->size) {
        s32 runLength = *decode->src++;
        if(runLength < RLE_LITERAL_RUN_FLAG) {
            s32 repeatedByte = *decode->src++;
            for(runIndex=0; runIndex<runLength; runIndex++) {
                *decode->dst++ = repeatedByte;
            }
        } else {
            runLength -= RLE_LITERAL_RUN_FLAG;
            for(runIndex=0; runIndex<runLength; runIndex++) {
                *decode->dst++ = *decode->src++;
            }
        }
        decode->size -= runLength;
    }
}

// Resource loaders call this with packed source data and a destination buffer sized for the decoded
// resource.
void HuDecodeData(void *src, void *dst, u32 size, s32 decodeType)
{
    DECODE decode;
    DECODE *decodeState = &decode;
    decodeState->src = src;
    decodeState->dst = dst;
    decodeState->size = size;
    switch(decodeType) {
        case HU_DECODE_TYPE_NONE:
            HuDecodeNone(decodeState);
            break;

        case HU_DECODE_TYPE_LZ:
            HuDecodeLz(decodeState);
            break;

        case HU_DECODE_TYPE_SLIDE:
            HuDecodeSlide(decodeState);
            break;

        case HU_DECODE_TYPE_FSLIDE_ALT:
            HuDecodeFslide(decodeState);
            break;

        case HU_DECODE_TYPE_FSLIDE:
            HuDecodeFslide(decodeState);
            break;

        case HU_DECODE_TYPE_RLE:
            HuDecodeRle(decodeState);
            break;

        case HU_DECODE_TYPE_ZLIB:
            // Discard the zlib status, including errors; the requested destination range is still
            // flushed below.
            HuDecodeZlib(decodeState);
            break;

        default:
            OSReport("decode tyep unknown.(%x)\n", decodeType);
            break;
    }
    // Write back the requested cache range even when the type is unknown.
    DCFlushRange(dst, size);
}

// zlib calls this through stream.zalloc to obtain zero-filled storage while unpacking a resource.
static void *ZlibCalloc(voidpf opaque, uInt items, uInt size)
{
    s32 allocSize = items*size;
    void *allocation = HuMemDirectMallocNum(HEAP_MODEL, allocSize, HU_MEMNUM_OVL);
    // The heap result goes straight to memset, including NULL when no block is available.
    memset(allocation, 0, allocSize);
    return allocation;
}

// zlib calls this through stream.zfree to release temporary unpacking storage.
static void ZlibFree(voidpf opaque, voidpf allocation)
{
    HuMemDirectFree(allocation);
}

// HuDecodeData calls this for ZLIB resources; it feeds the payload after its two-word header to
// zlib.
static int HuDecodeZlib(DECODE *decode)
{
    z_stream stream;
    u32 outputLimitStorage;
    int zlibResult;
    u32 *compressedWords = (u32 *)decode->src;
    stream.avail_in = compressedWords[1];
    compressedWords += 2;
    stream.next_in = (Bytef *)compressedWords;
    stream.next_out = decode->dst;
    // The output limit is set from this local's address rather than decode->size.
    stream.avail_out = (uInt)(&outputLimitStorage);
    stream.zalloc = ZlibCalloc;
    stream.zfree = ZlibFree;
    zlibResult = inflateInit(&stream);
    if(zlibResult) {
        return zlibResult;
    }
    zlibResult = inflate(&stream, Z_FINISH);
    if(zlibResult != Z_STREAM_END) {
        inflateEnd(&stream);
        return (zlibResult == Z_OK) ? Z_BUF_ERROR : zlibResult;
    } else {
        zlibResult = inflateEnd(&stream);
        return zlibResult;
    }
}
