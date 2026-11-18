#ifndef REL_FRAMEWORK_UNREGISTER_VALUE_H
#define REL_FRAMEWORK_UNREGISTER_VALUE_H

// Unlinks a registered entry from the list headed by link.
template <class Entry>
inline void unlinkEntry(Entry *value, Entry **link)
{
    while (*link != value) {
        link = &(*link)->next;
    }
    *link = value->next;
}

#endif
