#ifndef M622_ROOT_LIST_H
#define M622_ROOT_LIST_H

#include "REL/m622DLL/linknode.h"

class VirtualNode;

/* Circular list root used to visit the module's registered objects. */
class LinkRoot : public Node {
public:
    LinkRoot() {}
    ~LinkRoot() {}
    Node *first_node() { return link0; }
    Node *end_node() { return this; }
    VirtualNode *front();
    bool empty() const { return link0 == this && link0 == link4; }
    Node *last_node() { return link4; }
    VirtualNode *back();
};

typedef LinkRoot LinkRoot1AB40;

void fn_1_E348(int bit);
bool fn_1_E3E0();
void fn_1_E46C();
void fn_1_E4DC();
int fn_1_E5A4();

#endif
