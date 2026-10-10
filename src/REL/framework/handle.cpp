// Registers object pointers with reusable handles and keeps free and allocated slot lists.
static const char rcsid[] = "$Id: handle.cpp,v 1.6 2004/02/17 09:22:00 hanamasu Exp $";

extern "C" {
#include "REL/framework/assertion.h"
}

void *operator new[](unsigned long);
void operator delete[](void *);

enum { MAXCOUNT = 4096 };

// A handle combines a slot number with a generation that changes when the slot is reused.
union Handle {
    unsigned long encodedValue; // Packed handle returned to the caller; zero means no handle.
    struct {
        unsigned long index : 20; // Slot number; slot zero is reserved for the free-list sentinel.
        unsigned long generation : 12; // Reuse counter, wrapping after 4096 acquisitions of this
                                       // slot.
    } fields; // The slot and generation used to validate a packed handle.
};

// Links one slot into the free or allocated list and holds its current object and handle.
struct HandleEntry {
    HandleEntry *next; // Next entry in the slot's current circular list.
    HandleEntry *previous; // Previous entry in that list.
    void *object; // Registered pointer, cleared when the slot is released.
    Handle handle; // Slot index and reuse generation.
    unsigned long count() const;
};

// Registers object pointers in reusable slots without owning the objects themselves.
class HandleAllocator {
    HandleEntry *entries; // Array whose first entry is the free-list sentinel.
    HandleEntry *allocated; // Sentinel after the handle slots, for the allocated list.
    unsigned long capacity; // Exclusive slot-index limit; usable indices are 1 through capacity -
                            // 1.
public:
    HandleAllocator(unsigned long slotLimit);
    ~HandleAllocator();
    unsigned long acquire(void *object);
    void *release(unsigned long handleValue);
    void *lookup(unsigned long handleValue);
    unsigned long allocatedCount() const;
    unsigned long freeCount() const;
};

// Creates empty handle lists when the allocator is constructed, before objects register.
HandleAllocator::HandleAllocator(unsigned long slotLimit)
{
    // Only the upper limit is checked; list setup assumes a limit of at least two.
    slotLimit <= MAXCOUNT ? (void)0 : __msl_assertion_failed("n <= MAXCOUNT", "handle.cpp", 67);
    capacity = slotLimit;
    entries = new HandleEntry[slotLimit + 1];
    // The first and last entries are list sentinels rather than usable handle slots.
    allocated = &entries[slotLimit];
    allocated->next = allocated;
    allocated->previous = allocated;
    for (unsigned long index = 1; index < slotLimit; ++index) {
        entries[index].next = &entries[index] + 1;
        entries[index].previous = &entries[index] - 1;
        entries[index].object = 0;
        entries[index].handle.fields.index = index;
        entries[index].handle.fields.generation = 0;
    }
    entries[1].previous = entries;
    entries[slotLimit - 1].next = entries;
    entries->next = &entries[1];
    entries->previous = &entries[slotLimit] - 1;
}

// Frees the slot array on destruction; registered objects are not destroyed here.
HandleAllocator::~HandleAllocator()
{
    delete[] entries;
}

// Registers a caller's pointer in the first free slot, returning zero when all slots are in use.
unsigned long HandleAllocator::acquire(void *object)
{
    HandleEntry *entry = entries->next;
    if (entry == entries) {
        return 0;
    }
    entry->next->previous = entry->previous;
    entry->previous->next = entry->next;
    entry->next = allocated->next;
    entry->previous = allocated;
    allocated->next->previous = entry;
    allocated->next = entry;
    entry->object = object;
    // Reusing this slot invalidates its previous generation until the counter wraps.
    ++entry->handle.fields.generation;
    return entry->handle.encodedValue;
}

// Unregisters a caller's handle, returning its pointer for the caller to dispose of if needed.
void *HandleAllocator::release(unsigned long handleValue)
{
    Handle handle;
    handle.encodedValue = handleValue;
    if (handleValue == 0) return 0;
    if (handle.fields.index >= capacity) return 0;
    HandleEntry *entry = &entries[handle.fields.index];
    if (entry->handle.encodedValue != handleValue) return 0;
    void *object = entry->object;
    // Keep the generation until the next acquisition; lookups now return a null pointer.
    entry->object = 0;
    entry->next->previous = entry->previous;
    entry->previous->next = entry->next;
    entry->next = entries;
    entry->previous = entries->previous;
    entries->previous->next = entry;
    entries->previous = entry;
    return object;
}

// Resolves a caller's handle to its pointer, returning null for a rejected or released handle.
void *HandleAllocator::lookup(unsigned long handleValue)
{
    Handle handle;
    handle.encodedValue = handleValue;
    if (handleValue == 0) return 0;
    if (handle.fields.index >= capacity) return 0;
    HandleEntry *entry = &entries[handle.fields.index];
    if (entry->handle.encodedValue != handleValue) return 0;
    return entry->object;
}

// Counts a list's slots for the allocator's freeCount and allocatedCount queries.
unsigned long HandleEntry::count() const
{
    unsigned long total = 0;
    const HandleEntry *entry = next;
    while (entry != this) {
        entry = entry->next;
        ++total;
    }
    return total;
}

// Reports how many slots are currently registered.
unsigned long HandleAllocator::allocatedCount() const
{
    return allocated->count();
}

// Reports how many slots remain available for registration.
unsigned long HandleAllocator::freeCount() const
{
    return entries->count();
}
