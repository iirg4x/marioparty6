#include "REL/framework/sound.h"
#include "REL/m605DLL/scene_controller.h"
#include "REL/framework/game_object_man.h"
#include "REL/framework/event.h"
#include "REL/framework/sphere.h"
#include "REL/framework/random.h"
#include "REL/m605DLL/module_globals.h"
extern "C" {
#include "dolphin/types.h"
#include "game/hu3d.h"
}

#include <vector>
#include "datadir_enum.h"
#include "REL/framework/singleton.h"
#include "REL/m605DLL/timed_model_object.h"

// Places models on the song's timeline and tests them for player contact.

// Turns the model to a random heading.
void fn_1_5554(Model *model)
{
    Vec offset;
    offset.x = 0.0f;
    offset.z = 0.0f;
    offset.y = RandomFloat(360.0f);
    model->setRotation(&offset);
}

// Places a course model and configures its trigger sphere and initial animation.
TimedModelObject::TimedModelObject(unsigned int resource, Vec &position)
    : GameObject(2), triggerSphere(position, 50.0f), model(resource),
      resource(resource), started(false)
{
    model.setPosition(&position);
    model.setAttribute(0x40000080);
    model.setAttribute(0x40000002);
    model.setShadowMap();

    switch (resource) {
    case DATANUM(0x400000, 2):
    case DATANUM(0x400000, 3):
    case DATANUM(0x400000, 4):
        fn_1_5554(&model);
        break;
    case DATANUM(0x400000, 11):
    case DATANUM(0x400000, 12):
    case DATANUM(0x400000, 13):
    case DATANUM(0x400000, 14):
    case DATANUM(0x400000, 15):
    case DATANUM(0x400000, 16):
    case DATANUM(0x400000, 17):
    case DATANUM(0x400000, 18):
        fn_80030644((s16)model);
        model.setMotionTime(0.0f);
        model.getObjectPosition("trig", &this->triggerSphere.center);
        this->triggerSphere.radius = 0.0f;
        break;
    default:
        fn_8002D86C((s16)model, 1.55f);
        this->triggerSphere.radius = 0.0f;
        break;
    }
}

// Activates a course model behind the camera and deletes it after its motion ends.
void TimedModelObject::update()
{
    SceneController *transition;

    switch (resource) {
    case DATANUM(0x400000, 5):
    case DATANUM(0x400000, 6):
    case DATANUM(0x400000, 7):
    case DATANUM(0x400000, 11):
    case DATANUM(0x400000, 12):
    case DATANUM(0x400000, 13):
    case DATANUM(0x400000, 14):
    case DATANUM(0x400000, 15):
    case DATANUM(0x400000, 16):
    case DATANUM(0x400000, 17):
    case DATANUM(0x400000, 18):
        if (triggerSphere.center.z > lbl_1_bss_34) {
            break;
        }
        if (!started) {
            sendEvent(0, 1, 0.0f);
            model.resetAttribute(0x40000080);
            model.resetAttribute(0x40000002);
            transition = &MinigameSingleton<SceneController>::getChecked();
            fn_1_23DC(transition, 20.0f, 0.0f);
            fn_1_23DC(transition, 0.0f, 1.5f);
            started = true;
            soundPlayAt(1, 0x640, &triggerSphere.center);
            break;
        }
        if (fn_8002D75C(model) != 0) {
            delete this;
            return;
        }
        break;
    default:
        break;
    }
}

// Releases the course model and unregister its trigger sphere.
TimedModelObject::~TimedModelObject() {}

// Reports whether the supplied sphere overlaps an active course-model sphere.
bool fn_1_59C4(Sphere *comparison)
{
    std::vector<GameObject *> sequence;
    collectGameObjects(2, &sequence);
    s32 count = sequence.size();
    s32 index = 0;
    for (; index < count; index++) {
        TimedModelObject *object = static_cast<TimedModelObject *>(sequence[index]);
        f32 triggerRadius = object->triggerSphere.radius;
        if (!(triggerRadius <= 0.0f) && spheresOverlap(&object->triggerSphere, comparison)) {
            return true;
        }
    }
    return false;
}
