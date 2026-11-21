#ifndef REL_M623DLL_PLAYER_BODY_H
#define REL_M623DLL_PLAYER_BODY_H

#include "dolphin/mtx.h"
#include <memory>
#include "REL/framework/vector3.h"
#include "REL/framework/game_object.h"
#include "REL/m623DLL/scene_collision.h"

struct PlayerBody;
typedef void (PlayerBody::*BodyUpdate)(f32, f32);

// Holds the player's movement and ground-contact state.
struct PlayerBody : GameObject {
    explicit PlayerBody(bool primary);
    virtual ~PlayerBody();
    virtual void update();
    void fn_1_D064(f32 zInput, f32 xInput);
    void fn_1_D0D8(f32 zInput, f32 xInput);
    // Returns the stored horizontal stick input.
    f32 getStickInput()
    {
        return stickInput;
    }
    // Returns the displacement since the preceding update.
    Vec *getPositionDelta()
    {
        return &positionDelta;
    }
    // The movement callback for this player group.
    BodyUpdate updateBody;
    // Course collision query for the moving body.
    std::auto_ptr<TriangleCollision> collisionObject;
    // Current body position in the scene.
    Vector3 position;
    // Radius used when sweeping the body against the course.
    f32 radius;
    // Body position saved by the previous update.
    Vector3 previousPosition;
    // Body displacement applied per frame.
    Vec velocity;
    // Difference between the current and previous position.
    Vec positionDelta;
    // Freezes the eliminated body once it starts descending.
    u8 stopOnDescent;
    // The game's code never reads this field.
    bool unk55;
    // Latest horizontal stick input.
    f32 stickInput;
};

// Sets the moving body position.
void fn_1_CE5C(PlayerBody *body, const Vec *position);

// Stores the player input and dispatches the body callback.
void fn_1_D000(PlayerBody *body, f32 horizontal, f32 vertical);

// Separates two overlapping moving bodies.
void fn_1_D370(PlayerBody *first, PlayerBody *second);

// Kicks the body upward and enables the descent stop.
void fn_1_D4FC(PlayerBody *body);

#endif
