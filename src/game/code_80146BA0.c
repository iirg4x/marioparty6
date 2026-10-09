// Encodes and decodes the four-character codes used by the game.
#include "dolphin.h"

#define CODE_VALUE_BIAS 6383
#define CODE_CHARACTER_BASE 32
#define CODE_CHARACTER_MAP_SIZE 224
#define CODE_CHARACTER_MAP_INVALID 255
#define CODE_CHARACTER_VALUE_COUNT 64

void fn_80146DB8(void);

// Character bytes used for each six-bit part of an encoded code.
u8 lbl_80245FC8[64] = {
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x4B, 0x4C, 0x4D, 0x4E, 0x50, 0x51, 0x52, 0x53,
    0x54, 0x55, 0x56, 0x57, 0x59, 0x5A, 0x91, 0x92,
    0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A,
    0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0xA0, 0xA1, 0xA2,
    0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA,
    0xAB, 0xAC, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3,
    0xB4, 0xB5, 0xB6, 0xB7, 0xB9, 0xBA, 0xBC, 0xBD
};

// Bit destinations used to interleave the encoded code's 24 bits.
s32 lbl_80246008[24] = {
    9, 12, 19, 14, 16, 21, 13, 2, 18, 6, 15, 7,
    17, 0, 1, 3, 10, 4, 20, 5, 11, 22, 8, 23
};

// Reverse map from character byte minus 0x20 to its six-bit value.
u8 lbl_802B6A20[CODE_CHARACTER_MAP_SIZE];

// The kernel jump table calls this to encode a signed code value as four characters.
void fn_80146BA0(s32 codeValue, u8 *encodedText)
{
    s32 biasedValue;
    u32 encodedBits;
    s32 checkValue;
    u32 packedCode;
    s32 bitIndex;

    biasedValue = codeValue + CODE_VALUE_BIAS;
    packedCode = biasedValue;
    checkValue = 0;
    // The first check is the sum of the biased value's seven two-bit groups, modulo four.
    for (bitIndex = 0; bitIndex < 7; bitIndex++) {
        checkValue += packedCode & 3;
        packedCode >>= 2;
    }
    checkValue &= 3;
    // Store the two-bit check value above the biased 14-bit code.
    packedCode = biasedValue + (checkValue << 14);
    // Store the sum of the low two bytes, modulo 256, in bits 16-23 as a second check.
    checkValue = ((packedCode & 0xFF) + ((packedCode >> 8) & 0xFF)) & 0xFF;
    packedCode += checkValue << 16;
    encodedBits = 0;
    for (bitIndex = 0; bitIndex < 24; bitIndex++) {
        encodedBits |= (packedCode & 1) << lbl_80246008[bitIndex];
        packedCode >>= 1;
    }
    for (bitIndex = 0; bitIndex < 4; bitIndex++) {
        encodedText[bitIndex] = lbl_80245FC8[encodedBits & 0x3F];
        encodedBits >>= 6;
    }
}

// The kernel jump table calls this to validate four characters and recover their code value.
s32 fn_80146C88(u8 *encodedText)
{
    s32 bitIndex;
    u32 decodedCode;
    u32 encodedBits;
    u32 checkValue;
    u32 characterValue;

    // Rebuild the reverse character map before decoding this code.
    fn_80146DB8();
    encodedBits = 0;
    for (bitIndex = 0; bitIndex < 4; bitIndex++) {
        encodedBits <<= 6;
        characterValue = lbl_802B6A20[*(encodedText + 3 - bitIndex) - CODE_CHARACTER_BASE];
        if (characterValue > CODE_CHARACTER_VALUE_COUNT) {
            return -1;
        }
        encodedBits |= characterValue;
    }
    decodedCode = 0;
    for (bitIndex = 23; bitIndex >= 0; bitIndex--) {
        decodedCode <<= 1;
        decodedCode |= (encodedBits >> lbl_80246008[bitIndex]) & 1;
    }
    checkValue = ((decodedCode & 0xFF) + ((decodedCode >> 8) & 0xFF)) & 0xFF;
    if (checkValue != ((decodedCode >> 16) & 0xFF)) {
        return -1;
    }
    decodedCode &= 0xFFFF;
    encodedBits = decodedCode;
    checkValue = 0;
    for (bitIndex = 0; bitIndex < 7; bitIndex++) {
        checkValue += decodedCode & 3;
        decodedCode >>= 2;
    }
    checkValue &= 3;
    if (checkValue != ((encodedBits >> 14) & 3)) {
        return -1;
    }
    return (encodedBits & 0x3FFF) - CODE_VALUE_BIAS;
}

// Builds the reverse map called by fn_80146C88 before it checks entered characters.
void fn_80146DB8(void)
{
    s32 characterIndex;

    for (characterIndex = 0; characterIndex < CODE_CHARACTER_MAP_SIZE; characterIndex++) {
        lbl_802B6A20[characterIndex] = CODE_CHARACTER_MAP_INVALID;
    }
    for (characterIndex = 0; characterIndex < CODE_CHARACTER_VALUE_COUNT; characterIndex++) {
        lbl_802B6A20[lbl_80245FC8[characterIndex] - CODE_CHARACTER_BASE] = characterIndex;
    }
}
