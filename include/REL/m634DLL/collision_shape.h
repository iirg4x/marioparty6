// The shapes the minigame tests against each other once a frame.
#ifndef REL_M634DLL_COLLISION_SHAPE_H
#define REL_M634DLL_COLLISION_SHAPE_H

#include "REL/framework/game_object2.h"
#include "REL/framework/vector3.h"
#include <vector>

struct CollisionShape {
    Vector3 position;
    int kind; // 0 for a cross, 1 for a box.
    int collisionKind; // The other shape's shapeKind after a hit; -1 when clear.
    Mtx *matrix;
    int shapeKind;
    bool enabled;
    // Initializes the collision model and shape kind.
    explicit CollisionShape(int value)
        : kind(value)
        , collisionKind(-1)
        , shapeKind(-1)
        , enabled(0)
    {
    }
    // Releases the object and its member-owned resources.
    virtual ~CollisionShape() { }
    // Returns the collision shape kind.
    int getKind() const
    {
        return kind;
    }
    // Returns the recorded collision kind.
    int getCollisionKind() const
    {
        return collisionKind;
    }
    // Returns the shape identifier.
    int getShapeKind() const
    {
        return shapeKind;
    }
    // Returns the shape enabled flag.
    bool isEnabled()
    {
        return enabled;
    }
    // Returns the collision model matrix.
    Mtx &getMatrix()
    {
        return *matrix;
    }
};

struct CrossShape : CollisionShape, RenderObject {
    float radius;
    // Returns the cross shape radius.
    float getRadius() const
    {
        return radius;
    }
    explicit CrossShape(float value);
    virtual void update();
    virtual void draw(Mtx *);
    // Releases the object and its member-owned resources.
    virtual ~CrossShape() { }
};

struct BoxShape : CollisionShape, RenderObject {
    Vec dimensions;
    // Returns the box width.
    float getWidth() const
    {
        return dimensions.x;
    }
    // Returns the box height.
    float getHeight() const
    {
        return dimensions.y;
    }
    // Returns the box depth.
    float getDepth() const
    {
        return dimensions.z;
    }
    BoxShape(float width, float height, float depth);
    virtual void update();
    virtual void draw(Mtx *);
    // Releases the object and its member-owned resources.
    virtual ~BoxShape() { }
};

extern std::vector<CollisionShape *> lbl_1_bss_34;

#endif
