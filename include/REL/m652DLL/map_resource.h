#ifndef REL_M652DLL_MAP_RESOURCE_H
#define REL_M652DLL_MAP_RESOURCE_H
#include "REL/framework/model.h"

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

/* Deletes the owned model. */
inline MapResource::~MapResource() { delete value; }

/* Returns the owned model pointer. */
inline MapResource::operator OwnedModel *() const { return value; }
/* Returns the owned model reference. */
inline OwnedModel &MapResource::operator*() const { return *value; }
/* Deletes the previous model when the replacement differs. */
inline void MapResource::reset(OwnedModel *replacement)
{
    if (value != replacement) {
        delete value;
        value = replacement;
    }
}

/* Sets the model camera, layer, attributes and shadow map. */
inline void MapResource::configure(unsigned int visibility, unsigned long attributes)
{
    (**this).setCamera(visibility);
    (**this).setLayer(1);
    (**this).setAttribute(attributes);
    (**this).setShadowMap();
}

#endif
