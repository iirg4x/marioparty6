// Resolves named resources in the framework's loaded archive image.
#ifndef REL_FRAMEWORK_HU_ARCHIVE_H
#define REL_FRAMEWORK_HU_ARCHIVE_H

extern "C" {
#include "game/data.h"
#include "string.h"
#include "REL/framework/assertion.h"
}

void operator delete(void *allocation);

// Common interface for minigame resource containers that look up data by name.
class Archive {
public:
    virtual ~Archive() {}
    virtual void *Get(const char *name) = 0;
};

// Clears the archive's header and table bindings during construction and Load.
template<class Iterator, class Value>
inline void fillArchiveRange(Iterator first, Iterator last, const Value& value)
{
    for (; first != last; ++first) *first = value;
}

// Four-byte format field copied from the loaded archive header.
struct ArchiveFormat {
    unsigned char bytes[4]; // Format bytes retained without interpretation here.
};

// Additional header information retained when an archive image is loaded.
struct ArchiveMetadata {
    unsigned long words[2]; // Two words copied without interpretation here.
};

// Describes the consecutive resource and directory sections of an archive image.
struct ArchiveHeader {
    unsigned long imageSize; // Total image length in bytes, including the header and names.
    unsigned long resourceSize; // Byte length of the resource block immediately after the header.
    unsigned long relocationCount; // Number of pointer offsets in the relocation table.
    unsigned long publicCount; // Number of entries exposed by indexed and named lookup.
    unsigned long privateCount; // Number of entries in the second directory table.
    ArchiveFormat format; // Format bytes retained from the image header.
    ArchiveMetadata metadata; // Additional header words retained from the image.
};

// Locates one resource and its name within the loaded archive's two byte blocks.
struct ArchiveEntry {
    unsigned long resourceOffset; // Byte offset from the resource block's start.
    unsigned long nameOffset; // Byte offset from the entry-name table's start.
};

// Loads model-heap archive images and exposes their public resources to minigames.
class HuArchive : public Archive {
public:
    HuArchive(int dataNumber);
    virtual ~HuArchive();
    void Relocate();
    int GetPublicNum() const;
    void *GetPublic(int index) const;
    int Load(ArchiveHeader *image, int mode);
    const char *GetPublicName(int index) const;
    void *FindPublic(const char *name) const;
    virtual void *Get(const char *name);

private:
    ArchiveHeader header; // Header fields copied from the current image.
    char *resources; // Resource block base; null when its byte length is zero.
    unsigned long *relocations; // Offsets of pointer words relative to the resource block.
    ArchiveEntry *publicEntries; // Public lookup table; null when the public count is zero.
    ArchiveEntry *privateEntries; // Second directory table; null when its entry count is zero.
    char *names; // Trailing entry-name strings; null when no bytes follow the directory tables.
    unsigned long directoryState[2]; // Two words cleared by Load and unused by these methods.
    int initialized; // Set to one after Load clears the header and table bindings.
    void *image; // Image whose resource and directory blocks are currently bound.
    void *allocation; // Model-heap buffer read during construction.
    bool relocated; // True after pointer fixups; Load does not reset it when binding another image.
};

#endif
