#ifndef M652_MODEL_CHILD_H
#define M652_MODEL_CHILD_H
#include "humath.h"
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/framework/event_packet.h"
#include "REL/framework/category_sphere.h"
struct Racer;
class PlayerItem : public GameObject, public OwnedModel, public EventListener {
public:
    PlayerItem(Racer *, int);
    virtual ~PlayerItem();
    virtual void update();
    virtual void receive(const EventPacket *);
    void (PlayerItem::*callback)();
    Racer *playerWork;
    OwnedModel model;
    RegisteredValue registered;
    Point3d position;
    Point3d start;
    Point3d destination;
    int durationFrames;
    void fn_1_2660();
    void fn_1_2664();
    void fn_1_248C();
    void fn_1_2C78();
    void fn_1_2CF8();
    void fn_1_2DB8();
    typedef void (PlayerItem::*Callback)();
    void setDurationFrames(int frames) { durationFrames = frames; }
    void set_callback(Callback next) { callback = next; }
    bool update_countdown()
    {
        bool expired = false;
        if (durationFrames > 0 && --durationFrames == 0) expired = true;
        return expired;
    }
};


#endif
