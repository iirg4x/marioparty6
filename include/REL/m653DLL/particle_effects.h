#ifndef M653_PARTICLE_EFFECTS_H
#define M653_PARTICLE_EFFECTS_H
#include "REL/framework/game_object.h"
#include "REL/framework/particle.h"
extern "C" {
#include "game/hu3d.h"
}
/* A ring of particles that bursts outward from a point and fades: ResourceOwner makes one where
 * an event on its channel says (event 0). The class name is descriptive: the game keeps none. */
class RingBurst : public GameObject {
public:
    Particle model;
    RingBurst(ANIMDATA *, Vec *);
    virtual ~RingBurst();
    virtual void update();
};
/* A spray of particles thrown up in a circle around a point, tinted (0xF0, 0xE6, 0x82), falling
 * back as they fade. Nothing in this module constructs it, but the REL holds its constructor,
 * destructor, update and vtable, so the class stays. The class name is descriptive: the game
 * keeps none. */
class DustSpray : public GameObject {
public:
    Particle model;
    DustSpray(ANIMDATA *, Vec *);
    virtual ~DustSpray();
    void fn_1_B88C();
    virtual void update();
};

// The draw callbacks of the ring burst and the dust spray.
void fn_1_B4B4(HU3D_MODEL *unused, HU3D_PARTICLE *work, Mtx unusedMatrix);
void fn_1_BB3C(HU3D_MODEL *owner, HU3D_PARTICLE *sequence, Mtx unusedMatrix);
#endif
