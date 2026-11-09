#include "REL/m603DLL/callback_owner.h"

extern "C" unsigned long lbl_1_bss_AB4;

void fn_1_1A8D4(int bit)
{
    CallbackOwner *node = lbl_1_bss_AAC.front();
    Node *end = lbl_1_bss_AAC.end_node();
    lbl_1_bss_AB4 = bit;
    for (; node != end; node = node->next_node()) {
        node->state |= 1U << bit;
        node->update(bit);
    }
}

bool fn_1_1A96C()
{
    CallbackOwner *node = lbl_1_bss_AAC.front();
    Node *end = lbl_1_bss_AAC.end_node();
    for (; node != end; node = node->next_node()) {
        unsigned long bit = lbl_1_bss_AB4;
        if ((node->state & (1 << bit)) != 0) {
            return false;
        }
    }
    return true;
}

void fn_1_1A9F8()
{
    CallbackOwner *node = lbl_1_bss_AAC.front();
    Node *end = lbl_1_bss_AAC.end_node();
    for (; node != end; node = node->next_node()) {
        node->extra();
    }
}

void fn_1_1AA68()
{
    while (!lbl_1_bss_AAC.empty()) {
        delete lbl_1_bss_AAC.back();
    }
}

long fn_1_1AB30()
{
    return lbl_1_bss_AB4;
}

extern "C" {
CallbackOwnerList lbl_1_bss_AAC;
unsigned long lbl_1_bss_AB4;
}
