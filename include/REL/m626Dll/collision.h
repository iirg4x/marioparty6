/* Collision entries, constraint bounds, and transformed mesh edges. */
#ifndef REL_M626DLL_COLLISION_H
#define REL_M626DLL_COLLISION_H
#include "dolphin/mtx.h"
#include "game/object.h"
#include "game/hsfex.h"

typedef struct M626Entry {
    u32 flags;
    u32 mask;
    u32 type;
    u32 unknown_0C;
    s32 state;
    void *primaryRelation;
    void *secondaryRelation;
    void (*invoke)(s32 argument);
    void (*collisionCallback)(void *first, void *second);
    Vec *position;
    Vec *direction;
    Vec worldStart;
    Vec worldEnd;
    Mtx *matrix;
    Vec segmentStart;
    Vec segmentEnd;
    f32 radius;
    f32 collisionMode;
} M626Entry;

typedef struct M626RangeBounds {
    u32 flags;
    u32 mask;
    u32 type;
    u32 unknown_0C;
    s32 state;
    void *primaryRelation;
    void *secondaryRelation;
    void (*invoke)(s32 argument);
    Vec origin;
    Vec extent;
} M626RangeBounds;

typedef struct M626EllipseBounds {
    u32 flags;
    u32 mask;
    u32 type;
    u32 unknown_0C;
    s32 state;
    void *primaryRelation;
    void *secondaryRelation;
    void (*invoke)(s32 argument);
    Vec origin;
    f32 radiusX;
    f32 radiusZ;
    s32 direction;
} M626EllipseBounds;

typedef struct M626MeshEdge {
    Vec first;
    Vec second;
    Mtx *matrix;
} M626MeshEdge;

typedef struct M626WorldEdge {
    Vec first;
    Vec second;
    Vec normal;
    f32 planeConstant;
} M626WorldEdge;

typedef struct M626SurfaceCollider {
    u32 flags;
    u32 mask;
    u32 type;
    u32 unknown_0C;
    s32 state;
    void *primaryRelation;
    void *secondaryRelation;
    void (*invoke)(s32 argument);
    void (*collisionCallback)(void *first, void *second);
    s32 edgeCount;
    M626WorldEdge *current;
    M626WorldEdge *alternate;
    M626MeshEdge *original;
    s32 transformCount;
    HSF_TRANSFORM **transforms;
    Mtx *matrices;
    M626WorldEdge *hitEdge;
} M626SurfaceCollider;

typedef struct M626EntryWork {
    void *entries[38];
    void *secondaryEntries[10];
} M626EntryWork;

typedef struct M626ProjectionRatios {
    f32 start;
    f32 end;
} M626ProjectionRatios;

extern OMOBJ *lbl_1_bss_6D0;
extern s32 lbl_1_data_908[4];
f32 fn_1_3FC8(f32 start, f32 end);
void fn_1_E268(OMOBJMAN *objMan);
void fn_1_E2F0(void);
void fn_1_E2F4(OMOBJ *obj);
void fn_1_E304(OMOBJ *obj);
s32 fn_1_E56C(M626Entry *first, M626Entry *second);
s32 fn_1_E994(M626Entry *motion, M626EllipseBounds *settings);
s32 fn_1_EC80(M626Entry *owner, M626RangeBounds *limits);
s32 fn_1_ED80(M626Entry *body, M626SurfaceCollider *manager);
void fn_1_F190(M626Entry *bounds);
s32 fn_1_F264(Vec *firstStart, Vec *firstEnd, Vec *secondStart,
              Vec *secondEnd, Vec *firstClosest, Vec *secondClosest);
static inline void fn_1_F598(Vec *origin, Vec *direction, Vec *point, s32 clampResult,
                             Vec *result, f32 *ratio);
void fn_1_F73C(Vec *firstStart, Vec *firstEnd, Vec *firstDirection,
               Vec *secondStart, Vec *secondEnd, Vec *secondDirection,
               Vec *firstClosest, Vec *secondClosest);
void fn_1_FC4C(Vec *firstStart, Vec *firstDirection, Vec *secondStart,
               Vec *secondDirection, f32 firstRatio, f32 secondRatio,
               Vec *firstClosest, Vec *secondClosest);
int fn_1_10598(void *value);
s32 fn_1_10610(void *value);
void fn_1_10688(s32 index);
void *fn_1_106EC(s32 index);
void fn_1_1075C(M626SurfaceCollider *record, HSF_OBJECT *objects, s32 count);
void fn_1_10A1C(M626SurfaceCollider *collider, s16 modelNo);
void fn_1_10A74(M626SurfaceCollider *collider, HSF_OBJECT *object);
void fn_1_10AA8(M626SurfaceCollider *record);
s32 fn_1_10EA0(M626SurfaceCollider *work, Vec *position);
#endif
