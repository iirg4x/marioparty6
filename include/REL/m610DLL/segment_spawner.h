#ifndef REL_M610_SEGMENT_SPAWNER_H
#define REL_M610_SEGMENT_SPAWNER_H
#include "REL/framework/game_object.h"
#include "REL/framework/octtree_man.h"
#include "REL/framework/model.h"
#include "REL/m610DLL/collision_node.h"
/* Store the endpoint positions for a collision segment. */
struct SegmentContactPayload : CollisionNode
{
    Vec firstPosition, secondPosition;
    /* Register the two-endpoint collision payload. */
    SegmentContactPayload() : CollisionNode(3, this)
    {
    }
};
/* Own and replace a collision payload. */
template <class T> struct CollisionNodeOwner
{
    T *value;
    /* Take ownership of the supplied collision node. */
    CollisionNodeOwner(T *source = 0) : value(source)
    {
    }
    /* Release the owned collision node. */
    ~CollisionNodeOwner()
    {
        destroy(value);
    }
    /* Destroy and free the owned collision node. */
    static void destroy(T *value)
    {
        if (value)
        {
            value->~T();
            ::operator delete(value);
        }
    }
    /* Replace the owned node unless it is already selected. */
    CollisionNodeOwner &operator=(T *source)
    {
        if (value != source)
        {
            destroy(value);
            value = source;
        }
        return *this;
    }
    /* Return the owned collision node. */
    operator T *() const
    {
        return value;
    }
};
/* Updates the five named collision segments and their periodic spatial sounds. */
struct SegmentSpawner : GameObject
{
    std::auto_ptr<OwnedModel> resource;
    CollisionNodeOwner<CollisionNode> segments[5];
    explicit SegmentSpawner(OwnedModel *);
    virtual ~SegmentSpawner();
    virtual void update();
};

#endif
