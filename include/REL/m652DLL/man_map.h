// The map manager: the course's three scene layers, its model groups and menu layers, and the scene light.
#ifndef REL_M652DLL_MAN_MAP_H
#define REL_M652DLL_MAN_MAP_H
#include "REL/framework/list_object.h"
#include "REL/framework/model.h"
#include "REL/framework/singleton.h"
#include "REL/m652DLL/map_resource.h"

class MapManager : public RequestObject,
                    public MinigameSingleton<MapManager> {
public:
    OwnedModel layer0;
    OwnedModel layer1;
    OwnedModel layer2;
    MapResource groups[5][2];
    MapResource menuLayers[4];
    float previousValue;
    MapManager();
    virtual ~MapManager();
    virtual void flagEnabled(int);
    virtual void update();
};

#endif
