#ifndef REL_M623DLL_CIRCLE_RENDERER_H
#define REL_M623DLL_CIRCLE_RENDERER_H
#include "REL/framework/singleton.h"
#include "REL/framework/model.h"
#include <list>

struct CircleEntry;

// Draws the soft circles registered for each of the two views.
class CircleRenderer : public MinigameSingleton<CircleRenderer> {
  public:
    CircleRenderer();
    ~CircleRenderer();

    // The model hook for view index 0.
    OwnedModel cameraOneModel;
    // The model hook for view index 1.
    OwnedModel cameraTwoModel;
    // Circles registered for view index 0.
    std::list<CircleEntry *> cameraOneRecords;
    // Circles registered for view index 1.
    std::list<CircleEntry *> cameraTwoRecords;
};
#endif
