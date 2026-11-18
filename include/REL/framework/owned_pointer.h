#ifndef REL_FRAMEWORK_OWNED_POINTER_H
#define REL_FRAMEWORK_OWNED_POINTER_H
void operator delete(void *);

// Owns one object and deletes it on replacement or destruction.
template <class Value> struct OwnedPointer {
    Value *pointer; // Owned object, or null before it is assigned.
    // Start with no owned object.
    OwnedPointer() : pointer(0) {}
    // Take ownership of the supplied object.
    OwnedPointer(Value *value) : pointer(value) {}
    // Delete the owned object.
    ~OwnedPointer() { delete pointer; }
    Value *get() const { return pointer; }
    Value *operator->() const { return pointer; }
    // Return the owned object by reference.
    Value &operator*() const { return *get(); }
    // Test whether the owner is empty.
    bool operator!() const { return get() == 0; }
    operator Value *() const { return pointer; }
    // Replace the owned object unless it is already the supplied object.
    // m605 calls reset; written as an assignment there, its code compiles differently.
    void reset(Value *value) {
        if (pointer != value) {
            delete pointer;
            pointer = value;
        }
    }
    // Delete a supplied object.
    static void destroy(Value *value) {
        delete value;
    }
    // Assignment deletes the previous object only when the incoming pointer differs.
    OwnedPointer &operator=(Value *value) {
        if (pointer != value) {
            destroy(pointer);
            pointer = value;
        }
        return *this;
    }
};

#endif
