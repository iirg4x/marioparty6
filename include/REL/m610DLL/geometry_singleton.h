#ifndef REL_M610DLL_GEOMETRY_SINGLETON_H
#define REL_M610DLL_GEOMETRY_SINGLETON_H
#include <memory>
#include "REL/framework/singleton.h"
class GeometryOwner;

/* Access the scene debug-geometry owner through the shared singleton. */
struct GeometrySingleton : MinigameSingleton<GeometrySingleton>
{
    std::auto_ptr<GeometryOwner> object;
    GeometrySingleton();
    ~GeometrySingleton();
};
#endif
