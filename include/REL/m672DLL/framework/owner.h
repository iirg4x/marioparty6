#ifndef REL_M672DLL_FRAMEWORK_OWNER_H
#define REL_M672DLL_FRAMEWORK_OWNER_H
#include <stddef.h>

template<class T> struct ScenePointer672 {
    T *pointer;
    explicit ScenePointer672(T *value) : pointer(value) {}
    ~ScenePointer672() { delete pointer; }
    T *operator->() { return pointer; }
    operator T *() const { return pointer; }
    void clear() { if (pointer != NULL) { delete pointer; pointer = 0; } }
    ScenePointer672 &operator=(T *value) {
        if (pointer != value) {
            delete pointer;
            pointer = value;
        }
        return *this;
    }
    ScenePointer672 &operator=(const ScenePointer672 &value) {
        if (pointer != value.pointer) {
            delete pointer;
            pointer = value.pointer;
        }
        return *this;
    }
};
#endif
