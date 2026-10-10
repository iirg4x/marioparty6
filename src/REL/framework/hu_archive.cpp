// Loads framework archive resources and resolves their public names.
static const char rcsid[] = "$Id: hu_archive.cpp,v 1.22 2004/09/09 12:24:38 saf Exp $";

#include "REL/framework/hu_archive.h"

// Reads a minigame archive into the model heap, then binds its tables during construction.
HuArchive::HuArchive(int dataNumber)
{
    fillArchiveRange((signed char *)&header, (signed char *)&allocation, 0);
    allocation = HuDataSelHeapReadNum(dataNumber, HU_MEMNUM_OVL, HEAP_MODEL);
    relocated = false;
    Load((ArchiveHeader *)allocation, 0);
}

// Archive destruction leaves the loaded model-heap allocation in place.
HuArchive::~HuArchive() {}

// Called by Load to replace resource-relative pointer words with addresses in the loaded image.
void HuArchive::Relocate()
{
    if (resources && relocations && !relocated) {
        char *firstAddress = *(char **)(resources + relocations[0]);
        // The first pointer is treated as an offset only if it lies before the resource block.
        if (firstAddress < resources) {
            for (unsigned long index = 0; index < header.relocationCount; ++index) {
                char **address = (char **)(resources + relocations[index]);
                *address += (unsigned long)resources;
            }
            relocated = true;
        }
    }
}

// Supplies the public entry count used by indexed resource access.
int HuArchive::GetPublicNum() const
{
    return header.publicCount;
}

// Called for indexed access and by FindPublic; the assertion accepts index equal to the count.
void *HuArchive::GetPublic(int index) const
{
    (index >= 0 && index <= GetPublicNum()) ? (void)0 :
        __msl_assertion_failed("idx >=0 && idx <= GetPublicNum()", "hu_archive.cpp", 97);
    return resources + publicEntries[index].resourceOffset;
}

// Called during construction to bind the image's tables and relocate resources; mode is ignored.
int HuArchive::Load(ArchiveHeader *loadedImage, int mode)
{
    fillArchiveRange((signed char *)&header, (signed char *)&allocation, 0);
    initialized = 1;
    header.imageSize = loadedImage->imageSize;
    header.resourceSize = loadedImage->resourceSize;
    header.relocationCount = loadedImage->relocationCount;
    header.publicCount = loadedImage->publicCount;
    header.privateCount = loadedImage->privateCount;
    header.format = loadedImage->format;
    header.metadata = loadedImage->metadata;
    unsigned long offset = 0;
    offset += sizeof(ArchiveHeader);
    // The image stores resources, relocation offsets, both entry tables, then entry names.
    if (header.resourceSize) {
        resources = (char *)loadedImage + offset;
        offset += header.resourceSize;
    }
    if (header.relocationCount) {
        relocations = (unsigned long *)((char *)loadedImage + offset);
        offset += header.relocationCount * sizeof(unsigned long);
    }
    if (header.publicCount) {
        publicEntries = (ArchiveEntry *)((char *)loadedImage + offset);
        offset += header.publicCount * sizeof(ArchiveEntry);
    }
    if (header.privateCount) {
        privateEntries = (ArchiveEntry *)((char *)loadedImage + offset);
        offset += header.privateCount * sizeof(ArchiveEntry);
    }
    if (offset < header.imageSize) names = (char *)loadedImage + offset;
    image = loadedImage;
    Relocate();
    return 0;
}

// Supplies a public entry's name for directory inspection, or null for an out-of-range index.
const char *HuArchive::GetPublicName(int index) const
{
    if (index < 0 || header.publicCount <= (unsigned int)index) {
        return 0;
    }
    return names + publicEntries[index].nameOffset;
}

// Called by Get to scan public entry names and return the resource, or null when absent.
void *HuArchive::FindPublic(const char *name) const
{
    ArchiveEntry *entries = publicEntries;
    char *nameTable = names;
    int entryCount = header.publicCount;
    for (int index = 0; index < entryCount; ++index) {
        if (strcmp(nameTable + entries[index].nameOffset, name) == 0) return GetPublic(index);
    }
    return 0;
}

// Provides named resource lookup to minigame callers using the Archive interface.
void *HuArchive::Get(const char *name)
{
    return FindPublic(name);
}
