// Keyed GX command lists share one owned buffer for repeated minigame draws.
#ifndef REL_FRAMEWORK_GXDL_H
#define REL_FRAMEWORK_GXDL_H

#include <memory>
#include <utility>
#include <vector>

// A recorded draw: where its commands start in the cache's shared buffer, and how many bytes they
// fill. It does not own that range.
typedef std::pair<unsigned char *, unsigned long> DisplayList;

// One cache record: the caller's draw key and the commands recorded for it.
typedef std::pair<unsigned long, DisplayList> DisplayListEntry;

// Records each caller-selected draw key once and submits its command list on later draws.
class DisplayListCache {
public:
    DisplayListCache(long byteCount);
    ~DisplayListCache();
    bool begin(unsigned long key);
    void end();

private:
    std::auto_ptr<unsigned char> buffer; // Command bytes, released when the cache is destroyed.
    std::vector<DisplayListEntry> lists; // Cached keys in recording order, the newest list last.
    unsigned char *next; // Start of the unused part of the command buffer.
    unsigned long remain; // Free command-buffer bytes.
};

#endif
