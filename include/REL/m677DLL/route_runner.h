// Moves event models along queued waypoints and turns them toward their heading.

#ifndef REL_SEER_TERROR_ROUTE_RUNNER_H
#define REL_SEER_TERROR_ROUTE_RUNNER_H
#include "REL/framework/game_object.h"
#include "REL/framework/model_motion.h"
#include "REL/framework/range.h"
#include <list>
#include <memory>
// Advances a model through a queue of waypoints.
class RouteRunner : public GameObject {
public:
    // Update function selected by the runner state.
    typedef void (RouteRunner::*State)();

    State state;
    std::auto_ptr<ModelMotion> model;
    Vec position;
    // Position of the active waypoint.
    Vec target;
    // Direction used for the movement step.
    Vec velocity;
    std::list<Vec> path;
    // Current model heading, wrapped in degrees.
    Range<float> heading;
    // Heading the model turns toward, in degrees.
    float direction;
    // The game's code never reads this field.
    float unk64;
    // Distance advanced during a movement update.
    float movementRate;
    // Rate passed to queueWaypoint when moving beside the player. Also the distance
    // threshold for updating direction in move.
    float speed;
    // Angle used to place waypoints around the player, in degrees.
    float waypointAngle;
    // Whether the runner has finished its queued path.
    unsigned char idle;
    // Whether the model has reached its target heading.
    bool turned;
    unsigned char unk76[2]; /* No code names this field. */
    // Frames remaining until the next movement sound.
    int soundCountdown;

    RouteRunner();
    virtual ~RouteRunner();
    virtual void update();
    void waitIdle();
    void turnTowardPlayer();
    void moveBesidePlayer();
    void queueWaypoint(Vec value, float amount);
    // Returns the runner's current position by value.
    Vec getPosition() const { return position; }
    // Selects the next state handler.
    void setState(State next) { state = next; }
    // Sets the frames before the next movement sound.
    void setSoundCountdown(int frames) { soundCountdown = frames; }
    // Advances the movement sound countdown and reports when it expires.
    bool soundDue()
    {
        return soundCountdown > 0 && --soundCountdown == 0;
    }
    double distance(Vec from, Vec to);
    unsigned char move();
    bool turn();
};

bool matchesWrappedAngle(Range<float> *range, float value);
void aimAtWaypoint(RouteRunner *runner);
void selectWaypoint(RouteRunner *runner);

#endif
