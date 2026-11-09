#include "dolphin/types.h"
extern "C" {
#include "math.h"
#include "game/hu3d.h"
#include "game/charman.h"
#include "game/data.h"
}
#include "REL/m681DLL/character_sequence.h"

extern "C" void __msl_assertion_failed(const char *, const char *, int);


/* Reconstructed consumed transform view; prefix meaning is unknown. */
struct Transform {
    unsigned char unknown_00[24]; /* No code names this field. */
    Vec pos, rot, scale;
};

struct HsfObject_s;

/* Consumed model-handle prefix. The original class name and complete layout
 * are unknown; these methods only access the signed model ID at offset zero. */
struct Model {
    short modelId;
    operator short() const { return modelId; }

    short Replace(short value);
    void fn_1_33950(unsigned long flags);
    void fn_1_33988(unsigned long flags);
    void fn_1_33C78(float level);
    void fn_1_33CB0(short layer);
    void fn_1_33CE8(unsigned short camera);
    bool fn_1_33D20(unsigned short camera);
    void fn_1_344F0(float time);
    void fn_1_34528(float time);
    void fn_1_339C0(Transform *transform);
    void fn_1_33A3C(Vec *pos);
    Vec *fn_1_33A84(Vec *pos);
    void fn_1_33ACC(Vec *rot);
    Vec *fn_1_33B14(Vec *rot);
    void fn_1_33B5C(float angle);
    float fn_1_33BAC();
    void fn_1_33BE8(Vec *scale);
    void fn_1_33C30(float scale);
    void fn_1_33D68(char *name, Vec *pos, Vec *dir, GXColor *color);
    void fn_1_33DF8(bool moving);
    void fn_1_33E3C(Vec *color);
    void setHookData(void *hook);
    float *fn_1_33EA0();
    void fn_1_33F98();
    void fn_1_340D8();
    void fn_1_34108();
    float fn_1_345E4();
    float fn_1_34618();
    float fn_1_3464C();
    void fn_1_34680(float speed);
    int fn_1_346CC();
    void fn_1_34734();
    void fn_1_34764();
    void fn_1_34794(char *name, unsigned long attr);
    void fn_1_347D4(char *name, short hook);
    void fn_1_34814(char *name, short other);
    HsfObject_s *fn_1_33F28(char *name);
    void fn_1_34138(char *name);
    void fn_1_34170();
    Vec *fn_1_344A0(char *name, Vec *pos);
    float fn_1_34560();
};


#include "REL/m681DLL/framework/linknode.h"
typedef bool (*Predicate)(Node *, Node *);
NodeLinks *fn_1_176EC(Predicate, NodeLinks *);
NodeLinks *fn_1_17D94(NodeLinks *);
Node *fn_1_17DE8(Predicate, Node *, Node *);

void fn_1_17634(Node *self, Predicate predicate) {
    bool isEmpty = false;
    if (self->link0 == self) {
        if (self->link0 == self->link4) {
            isEmpty = true;
        }
    }

    if (!isEmpty) {
        self->link4->link0 = 0;
        Node *first = static_cast<Node *>(fn_1_176EC(predicate, self->link0));
        self->link0 = first;

        Node *last = self;
        while (first != 0) {
            first->link4 = last;
            last = first;
            first = first->link0;
        }

        self->link4 = last;
        last->link0 = self;
    }
}

NodeLinks *fn_1_176EC(Predicate predicate, NodeLinks *list)
{
    if (list == 0) return 0;
    if (static_cast<Node *>(list)->link0 == 0) return list;
    NodeLinks *first = list;
    NodeLinks *second = fn_1_17D94(first);
    first = fn_1_176EC(predicate, first);
    second = fn_1_176EC(predicate, second);
    return fn_1_17DE8(predicate, static_cast<Node *>(first), static_cast<Node *>(second));
}

NodeLinks *fn_1_17D94(NodeLinks *node) {
    NodeLinks *fast=node->link0; if(fast) fast=fast->link0;
    while(fast) { node=node->link0; fast=fast->link0; if(fast) fast=fast->link0; }
    fast=node->link0; node->link0=0; return fast;
}

Node *fn_1_17DE8(Predicate predicate, Node *first, Node *second) {
    NodeLinks head;
    Node **tail = &head.link0;
    while (first != 0 && second != 0) {
        if (predicate(first, second)) {
            *tail = first;
            tail = &first->link0;
            first = first->link0;
        } else {
            *tail = second;
            tail = &second->link0;
            second = second->link0;
        }
    }
    if (first == 0)
        *tail = second;
    else
        *tail = first;
    return head.link0;
}
