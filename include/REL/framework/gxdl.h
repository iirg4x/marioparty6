// Keyed GX command lists share one owned buffer for repeated minigame draws.
#ifndef REL_FRAMEWORK_GXDL_H
#define REL_FRAMEWORK_GXDL_H

extern "C" {
#include "dolphin/gx.h"
#include "dolphin/os.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/stdio.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/stdlib.h"
#include "REL/framework/assertion.h"
}
void *operator new(unsigned long size);
void *operator new[](unsigned long size);
void operator delete(void *allocation);
inline void *operator new(unsigned long, void *address) { return address; }

// Storage swaps exchange values when a grown entry table replaces the old one.
template<class Value> inline void swapValues(Value& left, Value& right)
{
    Value temporary = left;
    left = right;
    right = temporary;
}

template<class Value> inline void swap(Value& left, Value& right)
{
    swapValues(left, right);
}

// Cache construction fills its command buffer over the half-open iterator range.
template<class Iterator, class Value>
inline void fill(Iterator first, Iterator last, const Value& value)
{
    for (; first != last; ++first) *first = value;
}

// A recorded draw holds a command range in the cache's shared buffer; it does not own that range.
struct DisplayList {
    unsigned char *data; // Beginning of the recorded GX commands.
    unsigned long size; // Number of command bytes passed to GX.
    DisplayList(unsigned char *const& data, const unsigned long& size) : data(data), size(size) {}
    void call();
};

// One cache record associates the caller's draw key with its recorded commands.
struct DisplayListEntry {
    unsigned long key; // Identifies the draw operation whose commands are cached.
    DisplayList list; // Command range belonging to this key.
    DisplayListEntry(const unsigned long& key, const DisplayList& list) : key(key), list(list) {}
};

// Clearing a table destroys an entry while retaining the slot's allocation.
template<class Value> inline void destroy(Value *entry) { entry->~Value(); }

// Insertion constructs an entry in a table slot that has no live value yet.
template<class Value> inline void construct(Value *entry, const Value& value)
{
    new (entry) Value(value);
}

// Insertion moves a tail toward higher slots from back to front so overlapping ranges survive.
template <class InputIterator, class OutputIterator>
inline OutputIterator copyBackwardRange(InputIterator first, InputIterator last,
                                        OutputIterator result)
{
    while (last > first) *--result = *--last;
    return result;
}

template<class InputIterator, class OutputIterator>
inline OutputIterator copyBackward(InputIterator first, InputIterator last, OutputIterator result)
{
    return copyBackwardRange(first, last, result);
}

// Insertion fills the newly opened table slots with copies of the supplied entry.
template<class Iterator, class Size, class Value>
inline void fillCount(Iterator first, Size count, const Value& value)
{
    for (; count != 0; ++first, --count) *first = value;
}

template<class Value> inline unsigned long allocationLimit()
{
    return (unsigned long)-1 / sizeof(Value);
}

// Allocates raw storage for cache entries through the framework's object allocator.
template<class Value> struct EntryAllocator {
    EntryAllocator() {}
    EntryAllocator(const EntryAllocator&) {}
    unsigned long maxSize() const { return allocationLimit<Value>(); }
    // Table growth requests room for this many entries and aborts if allocation fails.
    Value *allocate(unsigned long count) {
        void *allocation = operator new(count * sizeof(Value));
        if (allocation == 0) {
            fprintf(stderr, "Memory allocation failure");
            abort();
        }
        return (Value *)allocation;
    }
    void deallocate(Value *allocation) { operator delete(allocation); }
};

// Holds the beginning of an entry table without tracking its live entries or capacity.
template<class Value> struct EntryStorage {
    Value *entries; // First entry slot, or null before the first allocation.
    Value*& first() { return entries; }
};

// Stores an entry table's capacity together with the allocator that releases it.
template<class Value> struct CapacityAllocator : EntryAllocator<Value> {
    unsigned long capacity; // Allocated entry slots; zero before the first allocation.
    unsigned long& first() { return capacity; }
    const unsigned long& first() const { return capacity; }
    EntryAllocator<Value>& second() { return *this; }
    const EntryAllocator<Value>& second() const { return *this; }
    CapacityAllocator() {}
    CapacityAllocator(const EntryAllocator<Value>& allocator)
        : EntryAllocator<Value>(allocator) {}
    void swap(CapacityAllocator& other) { ::swap(capacity, other.capacity); }
};

// Owns the ordered cache-entry table and expands it when new draw keys are recorded.
template<class Value> struct DisplayListVectorStorage {
    CapacityAllocator<Value> allocation; // Allocated entry slots and their allocator.
    unsigned long count; // Number of constructed entries, never greater than capacity.
    EntryStorage<Value> storage; // Entry allocation; null while capacity is zero.
    // A new cache starts with no entries and no table allocation.
    DisplayListVectorStorage() {
        allocation.capacity = 0;
        count = 0;
        storage.entries = 0;
    }
    // Insertion creates an empty replacement table using the current table's allocator.
    DisplayListVectorStorage(const EntryAllocator<Value>& allocator)
        : allocation(allocator) {
        allocation.capacity = 0;
        count = 0;
        storage.entries = 0;
    }
    ~DisplayListVectorStorage();
    void clear();
    Value *begin() { return storage.first(); }
    Value *end() { return storage.first() + count; }
    Value& back() { return *(storage.first() + count - 1); }
    const EntryAllocator<Value>& allocator() const { return allocation.second(); }
    EntryAllocator<Value>& allocator() { return allocation.second(); }
    unsigned long maxSize() const { return allocator().maxSize(); }
    unsigned long& capacityValue() { return allocation.first(); }
    const unsigned long& capacityValue() const { return allocation.first(); }
    unsigned long getCapacity() const { return capacityValue(); }
    Value*& storagePointer() { return storage.entries; }
    // Growth exchanges table ownership so the replacement destroys the old entries.
    void swapStorage(DisplayListVectorStorage& other) {
        if (this != &other) {
            allocation.swap(other.allocation);
            swap(storage.entries, other.storage.entries);
            swap(count, other.count);
        }
    }
    void insert(Value *position, unsigned long insertedCount, const Value& value);
};

typedef DisplayListVectorStorage<DisplayListEntry> DisplayListStorage;

// Supplies the keyed-entry storage owned by a display-list vector.
struct DisplayListVectorBase : DisplayListStorage {
    ~DisplayListVectorBase() {}
};

// The cache's ordered collection of keyed display lists.
struct DisplayListVector : DisplayListVectorBase {
    ~DisplayListVector() {}
};

// Owns the command bytes shared by every list in a cache.
struct DisplayListBuffer {
    unsigned char *data; // Command allocation, released when the cache is destroyed.
    // Cache construction allocates command bytes through the framework's array allocator.
    DisplayListBuffer(long byteCount) {
        unsigned char *allocation = new unsigned char[byteCount];
        data = allocation;
    }
    // Cache destruction releases the array through the framework's object delete entry point.
    ~DisplayListBuffer() { operator delete(data); }
    unsigned char *begin() { return data; }
};

// Records each caller-selected draw key once and submits its command list on later draws.
class DisplayListCache : DisplayListBuffer {
public:
    DisplayListVector lists; // Cached keys in recording order, including the current new list.
    unsigned char *next; // Start of the unused part of the command buffer.
    unsigned long remaining; // Free command-buffer bytes.
    DisplayListCache(long byteCount);
    ~DisplayListCache();
    int begin(unsigned long key);
    void end();
};

#endif
