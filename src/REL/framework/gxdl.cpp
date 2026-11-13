// Records reusable GX display lists in a shared backing buffer.
#include <algorithm>
#include <cstdio>

extern "C" {
#include "dolphin/gx.h"
#include "dolphin/os.h"
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
}

#include "REL/framework/gxdl.h"

static const char rcsid[] = "$Id: gxdl.cpp,v 1.6 2004/02/17 08:51:37 hanamasu Exp $";

void callDisplayList(const DisplayList& list);

// When a cache is created, reserves and clears its command buffer
// before recording any draw.
DisplayListCache::DisplayListCache(long byteCount)
    : buffer(new unsigned char[byteCount]), next(buffer.get()), remain(byteCount)
{
    std::fill(next,
        next + byteCount,
        0);
}

// When the cache is destroyed, reports unused bytes before its table
// and buffer are released.
DisplayListCache::~DisplayListCache()
{
    std::printf("remain = 0x%x\n",
        remain);
}

// A draw caller replays a cached key and gets false, or gets true to
// record new commands and then
// call end().
bool DisplayListCache::begin(unsigned long key)
{
    std::vector<DisplayListEntry>::iterator entry = lists.begin();

    std::vector<DisplayListEntry>::iterator limit = lists.end();

    for (; entry != limit; ++entry)
    {
        if (entry->first == key)
        {
            callDisplayList(entry->second);

            return false;
        }
    }

    lists.push_back(DisplayListEntry(key, DisplayList(next, 0)));
    // Discard CPU cache lines before GX records into the remaining
    // buffer.
    DCInvalidateRange(next,
        remain);

    GXBeginDisplayList(next,
        remain);
    // GXBeginDisplayList resets the pipe too; this cache explicitly
    // resets it again.
    GXResetWriteGatherPipe();

    return true;
}

// Submits a recorded command range to GX without recording it again.
void callDisplayList(const DisplayList& list)
{
    GXCallDisplayList(list.first,
        list.second);
}

// After begin() returns true and the caller emits commands, finishes,
// stores and submits that list.
void DisplayListCache::end()
{
    unsigned long size_of_used = GXEndDisplayList();
    // GX returns zero on buffer overflow; a zero-size result triggers
    // the assertion.
    assert(size_of_used != 0);
    DisplayList& list = lists.back().second;
    list.second = size_of_used;
    next += size_of_used;
    remain -= size_of_used;
    callDisplayList(list);
}
