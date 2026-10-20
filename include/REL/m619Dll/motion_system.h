/* Declares the shared collision list and callback-job pool. */
#ifndef REL_M619_MOTION_SYSTEM_H
#define REL_M619_MOTION_SYSTEM_H

#include "dolphin/types.h"
#include "game/hu3d.h"

typedef struct M619MotionNode M619MotionNode;
struct M619MotionNode {
    HuVecF *position; /* Current world position used for collision checks. */
    HuVecF previousPosition; /* Position captured before the frame update. */
    f32 deltaX; /* X displacement accumulated by the update callbacks. */
    f32 deltaY; /* Y displacement; the collision pass updates only X and Z. */
    f32 deltaZ; /* Z displacement accumulated by the update callbacks. */
    u8 unidentifiedBytes[12]; /* Opaque bytes in the node; no behavior is established here. */
    f32 radius; /* Collision radius in world units. */
    s32 kind; /* Participant category used by the pair-resolution callback. */
    void *owner; /* Minigame state passed to the update callbacks. */
    void (*beforeUpdate)(void *); /* Updates owner state before pair checks. */
    s32 (*afterUpdate)(void *); /* Returns nonzero when the node should be unlinked. */
    void (*resolvePair)(M619MotionNode *, M619MotionNode *, f32); /* Resolves an overlap. */
    M619MotionNode *previous; /* Previous node in the shared collision list. */
    M619MotionNode *next; /* Next node in the shared collision list. */
};

typedef struct M619Job M619Job;
typedef struct M619Jobs {
    M619Job *freeHead; /* First reusable callback job. */
    M619Job *activeHead; /* First scheduled callback job. */
    M619Job *storage; /* Base of the fixed job array. */
} M619Jobs;

struct M619Job {
    M619Jobs *owner; /* Pool that owns this job. */
    M619Job *previous; /* Previous job in its active or free list. */
    M619Job *next; /* Next job in its active or free list. */
    s32 (*callback)(void *); /* Returns nonzero when the scheduled work is complete. */
    void *data; /* Argument passed to the callback. */
};

extern M619MotionNode *lbl_1_bss_10;
extern M619Job lbl_1_bss_14[256];
extern M619Jobs lbl_1_bss_1414;

void fn_1_E05C(void);
void fn_1_E070(void);
void fn_1_E43C(M619MotionNode *node, void *owner,
               void (*beforeUpdate)(void *), s32 (*afterUpdate)(void *),
               void (*resolvePair)(M619MotionNode *, M619MotionNode *, f32),
               HuVecF *position);
void fn_1_E4A4(M619MotionNode *node);
void fn_1_E508(M619MotionNode *node, f32 radius, s32 kind);
void fn_1_E514(void);
s32 fn_1_E5B8(s32 (*callback)(void *), void *data);
void fn_1_E620(M619Job **job);
void fn_1_E6AC(void);

#endif
