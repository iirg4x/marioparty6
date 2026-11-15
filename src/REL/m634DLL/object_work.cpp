#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/framework/sound.h"

#include "REL/m634DLL/object_work.h"

// Configures the object model and its timed callback.
ObjectWork::ObjectWork(s32 resource)
    : GameObject(0x20)
    , OwnedModel((unsigned int)resource)
{
    setCamera(2);
    setLayer(0);
    frame = 0;
    Callback next = &ObjectWork::fn_1_1161C;
    callback = next;
}

// Releases the object and its member-owned resources.
ObjectWork::~ObjectWork() { }

// Runs the current object callback.
void ObjectWork::update()
{
    if (callback) {
        (this->*callback)();
    }
}

// Does nothing.
void ObjectWork::fn_1_11618() { }

// Plays the scheduled positional sounds and updates the object motion.
void ObjectWork::fn_1_1161C()
{
    Vec positions[2] = { { -610.0f, 0.0f, 375.0f }, { -447.0f, 0.0f, 147.0f } };
    if (frame == 144) {
        soundPlayAt(2, 0x73E, &positions[0]);
    }
    if (frame == 182) {
        soundPlayAt(2, 0x73D, &positions[0]);
    }
    if (frame == 183) {
        soundPlayAt(2, 0x73D, &positions[1]);
    }
    if (frame > 185) {
        setAttribute(0x40000002);
        void (ObjectWork::*next)() = &ObjectWork::fn_1_11618;
        callback = next;
    }
    frame = frame + 1;
}
