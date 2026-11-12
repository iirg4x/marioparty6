#ifndef REL_FRAMEWORK_HANDLE_H
#define REL_FRAMEWORK_HANDLE_H

struct HandleEntry;
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

#endif
