#ifndef REL_M648_FIRST_LIST_OBJECT_H
#define REL_M648_FIRST_LIST_OBJECT_H
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"

class FirstListObject : public RequestObject, public MinigameSingleton<FirstListObject> {
public:
    FirstListObject();
    virtual ~FirstListObject();
    virtual void flagEnabled(int request);
    virtual void update();
};
#endif
