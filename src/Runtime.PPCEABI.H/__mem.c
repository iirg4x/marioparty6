#include "stddef.h"
#include <dolphin.h>

#define SECTION_INIT __declspec(section ".init")

SECTION_INIT void __fill_mem(void* dst, int val, size_t n);

SECTION_INIT void* memset(void* dst, int val, size_t n) {
    __fill_mem(dst, val, n);

    return dst;
}

SECTION_INIT void __fill_mem(void* dst, int val, size_t n) {
    unsigned long v = (unsigned char)val;
    unsigned long i;
    unsigned char* bytes = dst;

    --bytes;

    if (n >= 32) {
        i = (~(unsigned long)bytes) & 3;

        if (i) {
            n -= i;

            do {
                *++bytes = v;
            } while (--i);
        }

        if (v)
            v |= v << 24 | v << 16 | v << 8;

        (unsigned long*)bytes = (unsigned long*)(bytes + 1) - 1;

        i = n >> 5;

        if (i) {
            do {
                *++((unsigned long*)bytes) = v;
                *++((unsigned long*)bytes) = v;
                *++((unsigned long*)bytes) = v;
                *++((unsigned long*)bytes) = v;
                *++((unsigned long*)bytes) = v;
                *++((unsigned long*)bytes) = v;
                *++((unsigned long*)bytes) = v;
                *++((unsigned long*)bytes) = v;
            } while (--i);
        }

        i = (n & 31) >> 2;

        if (i) {
            do {
                *++((unsigned long*)bytes) = v;
            } while (--i);
        }

        bytes = (unsigned char*)((unsigned long*)bytes + 1) - 1;

        n &= 3;
    }

    if (n)
        do {
            *++bytes = v;
        } while (--n);

    return;
}

SECTION_INIT void* memcpy(void* dst, const void* src, size_t n) {
    const unsigned char* s;
    unsigned char* d;

    if ((unsigned long)src >= (unsigned long)dst) {
        s = src;
        d = dst;
        --s;
        --d;
        n++;
        while (--n != 0)
            *++d = *++s;
    } else {
        s = src;
        d = dst;
        s += n;
        d += n;
        n++;
        while (--n != 0)
            *--d = *--s;
    }
    return dst;
}
