// The shapes the minigame tests against each other once a frame.
#ifndef REL_M634DLL_COLLISION_SHAPE_H
#define REL_M634DLL_COLLISION_SHAPE_H

#include "REL/framework/game_object2.h"
#include "REL/framework/vector3.h"

struct CollisionShape {
    Vector3 position;
    int kind; // 0 for a cross, 1 for a box.
    int collisionKind; // The other shape's shapeKind after a hit; -1 when clear.
    Mtx *matrix;
    int shapeKind;
    bool enabled;
    explicit CollisionShape(int value) : kind(value), collisionKind(-1), shapeKind(-1), enabled(0) {}
    virtual ~CollisionShape() {}
    int getKind() const { return kind; }
    int getCollisionKind() const { return collisionKind; }
    int getShapeKind() const { return shapeKind; }
    bool isEnabled() { return enabled; }
    Mtx &getMatrix() { return *matrix; }
};

struct CrossShape : CollisionShape, RenderObject {
    float radius;
    float getRadius() const { return radius; }
    explicit CrossShape(float value);
    virtual void update();
    virtual void draw(Mtx *);
    virtual ~CrossShape() {}
};

struct BoxShape : CollisionShape, RenderObject {
    Vec dimensions;
    float getWidth() const { return dimensions.x; }
    float getHeight() const { return dimensions.y; }
    float getDepth() const { return dimensions.z; }
    BoxShape(float width, float height, float depth);
    virtual void update();
    virtual void draw(Mtx *);
    virtual ~BoxShape() {}
};

#endif
