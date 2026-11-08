#ifndef M648_LINKNODE_H
#define M648_LINKNODE_H

struct ModuleListNode {
    ModuleListNode *link0;
    ModuleListNode *link4;

    ModuleListNode();
    ~ModuleListNode();
    void unlink();
    void insert0(ModuleListNode *other);
    void insert4(ModuleListNode *other);
    void fn_1_31F9C(ModuleListNode *other);
};

#endif
