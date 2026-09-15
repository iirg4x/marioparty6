#include "game/object.h"
#include "game/memory.h"
#include "string.h"
#include "REL/m659/collision.h"

OMOBJ *lbl_1_bss_54;
int lbl_1_bss_50;

/* Both array extents are bounded by the corresponding insertion and lookup
 * functions. The second set's element semantics are not yet reconstructed. */
void fn_1_4280(OMOBJ *obj);
void fn_1_4290(OMOBJ *obj);
#include "REL/m659/collision.h"
/* Classification values follow the MSL float interface. */
enum {
    M659_FP_SNAN = 0, M659_FP_QNAN = 1, M659_FP_INFINITE = 2,
    M659_FP_ZERO = 3, M659_FP_NORMAL = 4, M659_FP_SUBNORMAL = 5
};

/* Recovered float classifier; each test reads the signed representation word.
 * This implementation is not claimed to reproduce an original vendor header. */
#include "REL/m659/collision.h"
#include "math.h"
#include "game/object.h"
#include "math.h"

/* The caller copies both operands and the magnitude argument. These helper
 * signatures explain those aggregate copies without synthetic local homes. */

void fn_1_41F4(OMOBJMAN *manager)
{
    OMOBJ *obj;
    M659CollisionTable *work;

    obj = omAddObjEx(manager, 70, 0, 0, -1, fn_1_4280);
    lbl_1_bss_54 = obj;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M659CollisionTable), 268435456);
    obj->data = work;
    memset(work, 0, sizeof(M659CollisionTable));
}

void fn_1_427C(void)
{
}

void fn_1_4280(OMOBJ *obj)
{
    obj->objFunc = fn_1_4290;
}

static inline int classifyFloat(float value)
{
    switch (*(int *)&value & 2139095040) {
    case 2139095040:
        if ((*(int *)&value & 8388607) != 0) {
            return M659_FP_QNAN;
        }
        return M659_FP_INFINITE;
    case 0:
        if ((*(int *)&value & 8388607) != 0) {
            return M659_FP_SUBNORMAL;
        }
        return M659_FP_ZERO;
    }
    return M659_FP_NORMAL;
}

static inline void recoverPosition(void *entry)
{
    M659Collision *a = entry;
    if (classifyFloat(a->pos->x) == M659_FP_QNAN || classifyFloat(a->pos->z) == M659_FP_QNAN) {
        *a->pos = a->previous;
    }
}

void fn_1_4290(OMOBJ *obj)
{
    int i;
    M659Collision *a;
    M659CollisionTable *work = obj->data;
    int j;
    M659Collision *b;
    int changed = 0;
    M659Collision *aParams;

    for (i = 0; i < 200; i++) {
        a = work->entries[i];
        if (a != NULL) {
            aParams = a;
            if ((aParams->flags & 1) && aParams->unk_10 == 0) {
                for (j = i + 1; j < 200; j++) {
                    b = work->entries[j];
                    if (b != NULL) {
                        M659Collision *bParams = b;
                        if ((bParams->flags & 1) && bParams->unk_10 == 0
                            && (aParams->mask & bParams->mask) && fn_1_450C(a, b) == 1) {
                            changed = 1;
                        }
                    }
                }
            }
        }
    }
    if (changed == 1) {
        lbl_1_bss_50 = 1;
    }
    for (i = 0; i < 200; i++) {
        a = work->entries[i];
        if (a != NULL) {
            aParams = a;
            if (aParams->unk_10 == 0) {
                recoverPosition(a);
            }
            if (aParams->update != NULL) {
                aParams->update(aParams->userData);
            }
        }
    }
}

int fn_1_450C(M659Collision *a, M659Collision *b)
{
    float dx, dz;
    float distance;
    HuVecF middle;
    HuVecF *posA = a->pos;
    HuVecF *posB = b->pos;
    float rate, radiusSum;

    dx = posA->x - posB->x;
    dz = posA->z - posB->z;
    distance = sqrtf(dx * dx + dz * dz);
    radiusSum = a->radius + b->radius;
    if (radiusSum > distance) {
        if (distance == 0.0) {
            dx = 0.0f;
            dz = -1.0f;
        } else {
            dx /= distance;
            dz /= distance;
        }
        middle.x = posA->x + 0.5 * (posB->x - posA->x);
        middle.z = posA->z + 0.5 * (posB->z - posA->z);
        distance = 0.5 * radiusSum;
        posA->x = middle.x + dx * distance;
        posA->z = middle.z + dz * distance;
        posB->x = middle.x - dx * distance;
        posB->z = middle.z - dz * distance;
        if ((a->unk_08 != 0 || b->unk_08 != 0)
            && (b->unk_08 != 0 || a->unk_08 != 0)) {
            rate = 0.1f;
            a->velocity->x += a->factor * (rate * (dx * distance));
            a->velocity->z += a->factor * (rate * (dz * distance));
            rate = 0.1f;
            b->velocity->x -= b->factor * (rate * (dx * distance));
            b->velocity->z -= b->factor * (rate * (dz * distance));
        }
        if (lbl_1_bss_50 == 0) {
            if (a->collide != NULL) {
                a->collide(a, b);
            }
            if (b->collide != NULL) {
                b->collide(b, a);
            }
        }
        return 1;
    }
    return 0;
}

void fn_1_4918(HuVecF a, HuVecF b, HuVecF *delta)
{
    delta->x = a.x - b.x;
    delta->z = a.z - b.z;
}

double fn_1_493C(HuVecF delta)
{
    return sqrt(delta.x * delta.x + delta.z * delta.z);
}

double fn_1_4ABC(HuVecF a, HuVecF b)
{
    HuVecF delta;
    fn_1_4918(a, b, &delta);
    return fn_1_493C(delta);
}

int fn_1_4CA4(M659Collision *value)
{
    M659Collision **entry;
    int index;
    OMOBJ *obj = lbl_1_bss_54;
    M659CollisionTable *work = obj->data;

    entry = work->entries;
    for (index = 0; index < 200; index++, entry++) {
        if (*entry == NULL) {
            *entry = value;
            return index;
        }
    }
    return -1;
}

int fn_1_4D1C(void *value)
{
    void **entry;
    int index;
    OMOBJ *obj = lbl_1_bss_54;
    M659CollisionTable *work = obj->data;

    entry = work->entries2;
    for (index = 0; index < 130; index++, entry++) {
        if (*entry == NULL) {
            *entry = value;
            return index + 200;
        }
    }
    return -1;
}

void fn_1_4D94(int index)
{
    M659CollisionTable *work;
    OMOBJ *obj = lbl_1_bss_54;
    work = obj->data;
    if (index >= 0) {
        if (index < 200) {
            work->entries[index] = NULL;
        }
        index -= 200;
        if (index < 130) {
            work->entries2[index] = NULL;
        }
    }
}

void *fn_1_4DF8(int index)
{
    M659CollisionTable *work;
    OMOBJ *obj = lbl_1_bss_54;
    work = obj->data;
    if (index < 0) {
        return NULL;
    }
    if (index < 200) {
        return work->entries[index];
    }
    index -= 200;
    if (index < 130) {
        return work->entries2[index];
    }
    return NULL;
}
