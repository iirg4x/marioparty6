#ifndef M639_PARTICIPANT_OWNER_H
#define M639_PARTICIPANT_OWNER_H
// The two-sided gauge display.
#include "REL/framework/frame_interpolator.h"
#include <memory>
#include <vector>
#include "REL/framework/singleton.h"
#include "REL/framework/game_object.h"
#include "REL/framework/list_object.h"
#include "REL/m639DLL/resource_parameters.h"

class ParticipantSetup;

class ParticipantOwner : public RequestObject, public MinigameSingleton<ParticipantOwner> {
public:
    unsigned char active;
    FrameInterpolator<float> transition;
    std::vector<ANIMDATA *> gaugeAnimations;
    std::auto_ptr<ParticipantSetup> participants[2];
    ParticipantOwner();
    virtual ~ParticipantOwner();
    virtual void flagEnabled(int event);
    virtual void update();
    void fn_1_40EC();
};

class ParticipantSetup {
public:
    int participantIndex, firstPlayerIndex, secondPlayerIndex;
    ResourceParameters primary;
    ResourceParameters first;
    ResourceParameters second;
    std::auto_ptr<ResourceParameters> sprites[2][15];
    std::auto_ptr<ResourceParameters> markers[2];
    ParticipantSetup(int index);
    void fn_1_4A2C();
    void fn_1_4F44();
    void draw();
};

/* The frames of the gauge's fill transition. */
extern "C" int lbl_1_bss_84;

#endif
