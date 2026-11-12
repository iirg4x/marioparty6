#ifndef M624_ATTACKER_H
#define M624_ATTACKER_H
#include "REL/m624DLL/handle_list_object.h"
#include "REL/framework/frame_interpolator.h"

// One attacker: it waits, shakes, jumps and lands back on its start point, lit from a fixed
// point.
class Attacker : public HandleListObject {
public:
    typedef void (Attacker::*State)();

    Attacker();
    virtual ~Attacker();
    virtual void update();
    virtual void start();
    void setState(State next) { state = next; }
    void setFrame(long frames) { frame = frames; }
    long getFrame() const { return frame; }
    // Counts the frame timer down; true on the frame it reaches zero.
    bool countDown()
    {
        bool expired = false;
        if (frame > 0) {
            if (--frame == 0) {
                expired = true;
            }
        }
        return expired;
    }
    void none() {}
    void wait();
    void shake();
    void lean();
    void hold();
    void recover();
    void updatePose();
    void jump();
    void fall();
    void move();

    State state;
    OwnedModel model;
    short m_light_id;
    Vec position;
    float speed;
    FrameInterpolator<float> transition;
    long frame;
    long selection;
};

#endif
