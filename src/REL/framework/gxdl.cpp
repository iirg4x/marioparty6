// Records reusable GX display lists in a shared backing buffer.
static const char rcsid[] = "$Id: gxdl.cpp,v 1.6 2004/02/17 08:51:37 hanamasu Exp $";

#include "REL/framework/gxdl.h"

// When a cache is created, reserves and clears its command buffer before recording any draw.
DisplayListCache::DisplayListCache(long byteCount)
    : DisplayListBuffer(byteCount), next(DisplayListBuffer::begin()), remaining(byteCount)
{
    fill(next, next + byteCount, 0);
}

// When the cache is destroyed, reports unused bytes before its table and buffer are released.
DisplayListCache::~DisplayListCache()
{
    printf("remain = 0x%x\n", remaining);
}

// A draw caller replays a cached key, or gets 1 to record new commands and then call end().
int DisplayListCache::begin(unsigned long key)
{
    for (DisplayListEntry *entry = lists.begin(), *limit = lists.end(); entry != limit; ++entry) {
        if (entry->key == key) {
            entry->list.call();
            return 0;
        }
    }
    DisplayList newList(next, 0);
    DisplayListEntry newEntry(key, newList);
    lists.insert(lists.end(), 1, newEntry);
    // Discard CPU cache lines before GX records into the remaining buffer.
    DCInvalidateRange(next, remaining);
    GXBeginDisplayList(next, remaining);
    // GXBeginDisplayList resets the pipe too; this cache explicitly resets it again.
    GXResetWriteGatherPipe();
    return 1;
}

// A cache hit submits the recorded command range to GX without recording it again.
void DisplayList::call()
{
    GXCallDisplayList(data, size);
}

// After begin() returns 1 and the caller emits commands, finishes, stores and submits that list.
void DisplayListCache::end()
{
    unsigned long recordedBytes = GXEndDisplayList();
    // GX returns zero on buffer overflow; a zero-size result triggers the assertion.
    recordedBytes != 0 ? (void)0 : __msl_assertion_failed("size_of_used != 0", "gxdl.cpp", 84);
    DisplayList& list = lists.back().list;
    list.size = recordedBytes;
    next += recordedBytes;
    remaining -= recordedBytes;
    GXCallDisplayList(list.data, list.size);
}

// Cache misses append a keyed list; insertion also preserves entries following any position.
template<class Value>
void DisplayListVectorStorage<Value>::insert(Value *position, unsigned long insertedCount,
                                     const Value& value)
{
    if (insertedCount != 0) {
        unsigned long maximumEntries = maxSize();
        if (insertedCount > maximumEntries || count > maximumEntries - insertedCount) {
            fprintf(stderr, "vector::insert length error\n");
            abort();
        }
        if (count + insertedCount <= getCapacity()) {
            Value *last = end();
            unsigned long tailCount = last - position;
            const Value *sourceValue = &value;
            if (insertedCount > tailCount) {
                Value *destination = last;
                while (insertedCount > tailCount) {
                    construct(destination, value);
                    ++destination;
                    --insertedCount;
                    ++count;
                }
                for (Value *source = position; source < last; ) {
                    construct(destination, *source);
                    ++source;
                    ++destination;
                    ++count;
                }
            } else {
                Value *source = last - insertedCount;
                Value *destination = last;
                for (; source < last; ) {
                    construct(destination, *source);
                    ++source;
                    ++destination;
                    ++count;
                }
                // If the inserted value is in the moved tail, read its relocated copy.
                if (last - (tailCount - insertedCount) <= sourceValue && sourceValue < last) {
                    sourceValue += insertedCount;
                }
                copyBackward(position, position + (tailCount - insertedCount), last);
            }
            fillCount(position, insertedCount, *sourceValue);
        } else {
            DisplayListVectorStorage<Value> replacement(allocator());
            unsigned long requiredEntries = count + insertedCount;
            unsigned long newCapacity = capacityValue() != 0 ? capacityValue() : 1;
            // Grow geometrically, stopping at the allocator's entry limit.
            while (requiredEntries > newCapacity) {
                if (newCapacity < (maximumEntries >> 1)) newCapacity *= 2;
                else newCapacity = maximumEntries;
            }
            replacement.storagePointer() = replacement.allocator().allocate(newCapacity);
            replacement.capacityValue() = newCapacity;
            Value *destination = replacement.storage.entries;
            Value *source = begin();
            Value *last = end();
            while (source < position) {
                construct(destination, *source);
                ++source;
                ++destination;
                ++replacement.count;
            }
            while (insertedCount != 0) {
                construct(destination, value);
                ++destination;
                --insertedCount;
                ++replacement.count;
            }
            while (source < last) {
                construct(destination, *source);
                ++source;
                ++destination;
                ++replacement.count;
            }
            replacement.swapStorage(*this);
        }
    }
}

// On cache destruction or replacement after growth, destroys entries and frees their storage.
template<class Value>
inline DisplayListVectorStorage<Value>::~DisplayListVectorStorage()
{
    clear();
    if (storage.first()) allocator().deallocate(storage.first());
}

// Storage destruction uses this to destroy live entries from back to front without freeing
// capacity.
template<class Value>
void DisplayListVectorStorage<Value>::clear()
{
    Value *first = storage.first();
    Value *last = first + count;
    while (last > first) destroy(--last);
    count = 0;
}
