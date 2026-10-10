// Stores each character's requested motions and supplies motion-list allocation and lookup helpers.
#ifndef REL_FRAMEWORK_CHARA_MOTION_H
#define REL_FRAMEWORK_CHARA_MOTION_H

extern "C" {
#include <stdio.h>
#include "dolphin/math.h"
#include "game/charman.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/stdio.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/abort_exit.h"
}

// MotionAllocator constructs records in slots it has already allocated.
inline void *operator new(unsigned long, void *storage) { return storage; }

// Associates a requested character motion resource with the engine handle returned for it.
struct MotionRecord {
    unsigned int dataNum; // Resource number supplied to CharMotionCreate, also used for lookup.
    HU3D_MOTIONID motionId; // Engine motion handle; HU3D_MOTIONID_NONE means no motion handle was
                            // created.
    template<class MotionId>
    MotionRecord(const unsigned int &dataNum, const MotionId &motionId)
        : dataNum(dataNum), motionId(motionId) {}
};

// MotionAllocator uses this element count limit to avoid overflowing the allocation byte size.
template<class T>
inline unsigned long motionAllocationLimit()
{
    return (unsigned long)-1 / sizeof(T);
}

// Allocates motion-list storage and constructs or destroys its individual records.
template<class T> struct MotionAllocator {
    MotionAllocator() {}
    MotionAllocator(const MotionAllocator &) {}
    void destroy(T *record) { record->~T(); }
    void deallocate(T *allocation) { ::operator delete(allocation); }
    void construct(T *position, const T &record) { new ((void *)position) T(record); }
    unsigned long maximumCount() const { return motionAllocationLimit<T>(); }
    // insertMotionRecords requests a new allocation here; allocation failure ends the program.
    T *allocate(unsigned long count) {
        T *allocation = (T *)::operator new(count * sizeof(T));
        if (!allocation) {
            fprintf(stderr, "Memory allocation failure");
            abort();
        }
        return allocation;
    }
};

template<class T>
inline void exchangeMotionValue(T &, T &);

// Owns the motion-list allocation, its live records and the unused slots available for insertion.
template<class T> struct SequenceStorage : MotionAllocator<T> {
    unsigned long capacity; // Allocated record slots, including unused space at the end.
    unsigned long count; // Constructed records at the beginning of the allocation.
    T *storage; // First allocated slot, or null before the list has an allocation.
    SequenceStorage() : capacity(0), count(0), storage(0) {}
    SequenceStorage(const MotionAllocator<T> &allocator)
        : MotionAllocator<T>(allocator), capacity(0), count(0), storage(0) {}
    const MotionAllocator<T> &allocator() const {
        return static_cast<const MotionAllocator<T> &>(*this);
    }
    MotionAllocator<T> &allocatorStorage() { return *this; }
    MotionAllocator<T> &allocator() { return allocatorStorage(); }
    unsigned long &capacityStorage() { return capacity; }
    unsigned long &capacityValue() { return capacityStorage(); }
    const unsigned long &capacityValue() const { return capacity; }
    T *&storagePointer() { return storage; }
    T *begin() { return storagePointer(); }
    T *end() { return storagePointer() + count; }
    T &back() { return *(storagePointer() + count - 1); }
    unsigned long availableCapacity() const { return capacityValue(); }
    unsigned long maximumCount() const { return allocator().maximumCount(); }
    // exchangeMotionStorageValues swaps the reserved slot count during allocation replacement.
    void exchangeCapacity(SequenceStorage &other) {
        exchangeMotionValue(capacity, other.capacity);
    }
    void clear();
    ~SequenceStorage();
};

// Supplies the owning storage base for the character motion sequence.
template<class T> struct SequenceBase : SequenceStorage<T> {
    ~SequenceBase() {}
};

// Character motion lists use this sequence of resource-number and motion-handle records.
template<class T> struct Sequence : SequenceBase<T> {
    ~Sequence() {}
};

typedef SequenceStorage<MotionRecord> MotionSequenceStorage;
typedef SequenceBase<MotionRecord> MotionSequenceBase;
typedef Sequence<MotionRecord> MotionSequence;

// Tracks one character's cached motions and the resource most recently selected for playback.
struct MotionOwner {
    s32 charNo; // Character number used by the engine's character motion functions.
    MotionSequence motions; // Requested resources and returned handles, including failed loads.
    unsigned int currentDataNum; // Selected resource number; unsigned -1 means none selected yet.
    MotionOwner(unsigned int characterNo);
    ~MotionOwner();
};

typedef MotionOwner CharacterMotion;

template<class T>
void insertMotionRecords(Sequence<T> *, T *, unsigned int, const T *);

// Motion loading packages its resource number and returned handle for insertion into the list.
template<class MotionId>
inline MotionRecord makeMotionRecord(unsigned int dataNum, MotionId motionId)
{
    return MotionRecord(dataNum, motionId);
}

// Character motion loading appends one new resource-to-handle record.
inline void appendMotionRecord(MotionSequence &sequence, const MotionRecord &record)
{
    insertMotionRecords(&sequence, sequence.end(), 1, &record);
}

// Describes the two input types and result used by motion cleanup and lookup callbacks.
template<class FirstArgument, class SecondArgument, class Result> struct MotionBinaryFunction {
    typedef FirstArgument first_argument_type;
    typedef SecondArgument second_argument_type;
    typedef Result result_type;
};

// Wraps a callback that acts on a motion handle belonging to a specified character.
struct MotionFunction : MotionBinaryFunction<s16, unsigned int, void> {
    void (*callback)(s16, unsigned int); // Character-and-motion operation used during cleanup.
    MotionFunction(void (*callback)(s16, unsigned int)) : callback(callback) {}
    // MotionCleanup supplies the bound character and each stored motion handle during destruction.
    void operator()(s16 characterNo, unsigned int motionId) const
    {
        callback(characterNo, motionId);
    }
};

// Describes the single input and result of a character-bound cleanup or resource-bound matcher.
template<class Argument, class Result> struct MotionUnaryFunction {
    MotionUnaryFunction() {}
    typedef Argument argument_type;
    typedef Result result_type;
};

// Binds the character number so list cleanup only needs each record's motion handle.
struct MotionCleanup : MotionUnaryFunction<unsigned int, void> {
    MotionFunction operation; // Callback applied to each handle by applyMotionCleanup.
    s16 character; // Character number passed to the cleanup callback.
    MotionCleanup(const MotionFunction &function, const s16 &character)
        : operation(function), character(character) {}
    // applyMotionCleanup invokes this for each cached motion when the owner is destroyed.
    void operator()(const unsigned int &motionId) const
    {
        operation(character, motionId);
    }
};

// MotionOwner destruction wraps CharMotionKill before binding its character number.
inline MotionFunction makeMotionFunction(void (*function)(s16, unsigned int))
{
    return MotionFunction(function);
}

// MotionOwner destruction binds its character number to the motion cleanup callback.
inline MotionCleanup bindMotionCleanup(const MotionFunction &function, const s32 &character)
{
    s16 characterNo(character);
    return MotionCleanup(function, characterNo);
}

// Wraps the predicate comparing a stored motion record with a requested resource number.
struct MotionMatchFunction : MotionBinaryFunction<const MotionRecord &, unsigned int, bool> {
    bool (*callback)(const MotionRecord &, unsigned int); // Resource comparison used during lookup.
    MotionMatchFunction(bool (*callback)(const MotionRecord &, unsigned int))
        : callback(callback) {}
    // MotionMatcher calls the record predicate with the resource number bound for this lookup.
    bool operator()(const MotionRecord &record, const unsigned int &dataNum) const
    {
        const unsigned int requestedDataNum = dataNum;
        return callback(record, requestedDataNum);
    }
};

// Binds the requested resource number for a linear search through the cached motion records.
struct MotionMatcher : MotionUnaryFunction<MotionRecord, bool> {
    MotionMatchFunction operation; // Predicate used to compare each record with the requested
                                   // number.
    unsigned int dataNum; // Requested resource number, unchanged throughout one lookup.
    MotionMatcher(const MotionMatchFunction &operation, const unsigned int &dataNum)
        : operation(operation), dataNum(dataNum) {}
    // findMatchingMotion tests each record through this bound predicate.
    bool operator()(const MotionRecord &record) const
    {
        return operation(record, dataNum);
    }
};

// getCharacterMotion wraps its record comparison before binding the requested resource number.
inline MotionMatchFunction makeMotionMatchFunction(
    bool (*function)(const MotionRecord &, unsigned int))
{
    return MotionMatchFunction(function);
}

// getCharacterMotion fixes the requested resource number for its cached-motion search.
inline MotionMatcher bindMotionMatcher(const MotionMatchFunction &operation,
                                       const unsigned int &dataNum)
{
    unsigned int requestedDataNum(dataNum);
    return MotionMatcher(operation, requestedDataNum);
}

// getCharacterMotion finds the first matching cached record, or returns last if none matches.
inline MotionRecord *findMatchingMotion(MotionRecord *first, MotionRecord *last,
                                       MotionMatcher matcher)
{
    while (first != last && !matcher(*first)) ++first;
    return first;
}

// Names indexed by the low 16-bit motion resource number, used for a missed-preload warning.
extern const char *characterMotionNames[];

template<class T>
inline T *copyMotionRecordsBackward(T *, T *, T *);

// insertMotionRecords assigns the insertion value into already constructed record slots.
template<class T>
inline void fillMotionRecords(T *destination, unsigned int count, const T &source)
{
    while (count != 0) {
        *destination = source;
        ++destination;
        --count;
    }
}

// Swaps the reserved capacity, allocation pointer and live-record count between two lists.
template<class T>
inline void exchangeMotionStorageValues(SequenceStorage<T> &left, SequenceStorage<T> &right)
{
    if (&left != &right) {
        left.exchangeCapacity(right);
        exchangeMotionValue(left.storage, right.storage);
        exchangeMotionValue(left.count, right.count);
    }
}

// insertMotionRecords transfers the replacement allocation to the character's motion sequence.
template<class T>
inline void exchangeMotionStorage(SequenceStorage<T> &left, SequenceStorage<T> &right)
{
    exchangeMotionStorageValues(left, right);
}

#endif
