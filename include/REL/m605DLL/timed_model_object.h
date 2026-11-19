#ifndef REL_M605DLL_TIMED_MODEL_OBJECT_H
#define REL_M605DLL_TIMED_MODEL_OBJECT_H

#include "dolphin/mtx.h"
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/framework/sphere_type.h"

// Waits for its position on the song's timeline, plays once and removes itself.
class TimedModelObject : public GameObject {
public:
    Sphere triggerSphere;
    OwnedModel model;
    unsigned int resource;
    unsigned char unk28[4]; /* No code names this field. */
    bool started;

    TimedModelObject(unsigned int resource, Vec &position);
    virtual ~TimedModelObject();
    virtual void update();
};

bool fn_1_59C4(Sphere *comparison);

#endif
