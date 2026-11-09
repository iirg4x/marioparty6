#ifndef REL_M652DLL_MAP_RESOURCE_H
#define REL_M652DLL_MAP_RESOURCE_H
struct OwnedModel;
class MapResource {
public:
    OwnedModel *value;
    explicit MapResource(OwnedModel *value = 0);
    ~MapResource();
    operator OwnedModel *() const;
    OwnedModel &operator*() const;
    void reset(OwnedModel *replacement);
    void configure(unsigned int visibility, unsigned long attributes);
};
#endif
