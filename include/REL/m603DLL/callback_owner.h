#ifndef M603_CALLBACK_OWNER_H
#define M603_CALLBACK_OWNER_H
#include "dolphin/types.h"
#include "REL/m603DLL/linknode.h"

struct CallbackOwner;

struct CallbackOwnerList : Node {
    ~CallbackOwnerList();
    Node *first_node() { return link0; }
    Node *last_node() { return link4; }
    Node *end_node() { return this; }
    CallbackOwner *front();
    CallbackOwner *back();
    bool empty() const { return link0 == this && link0 == link4; }
};

extern "C" CallbackOwnerList lbl_1_bss_AAC;

struct CallbackElementNodeED8 : Node {};
struct CallbackOwner : CallbackElementNodeED8 {
    virtual ~CallbackOwner();
    virtual void update(unsigned long) = 0;
    virtual void extra();
    s32 state;
    CallbackOwner() { state = 0; this->insert4(&lbl_1_bss_AAC); }
    CallbackOwner *next_node() { return static_cast<CallbackOwner *>(link0); }
};

inline CallbackOwner::~CallbackOwner() {}
inline CallbackOwner *CallbackOwnerList::front()
{
    return static_cast<CallbackOwner *>(first_node());
}
inline CallbackOwner *CallbackOwnerList::back()
{
    return static_cast<CallbackOwner *>(last_node());
}
void fn_1_1A8D4(int bit);
void fn_1_1A9F8(void);
void fn_1_1AA68(void);
long fn_1_1AB30(void);
#endif
