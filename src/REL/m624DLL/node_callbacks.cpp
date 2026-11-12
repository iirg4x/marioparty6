#include "REL/m624DLL/callback_singleton.h"
#include "REL/m624DLL/handle_list_object.h"
#include "REL/framework/list_object.h"
#include "REL/m624DLL/tail_types.h"
#include "REL/m624DLL/model_commands.h"
#include "REL/framework/heap.h"

#include "dolphin/gx.h"

#include "math.h"

#include "game/main.h"

#include "game/object.h"

#include "game/charman.h"

#include "game/hu3d.h"

#include "game/gamework.h"

#include "game/wipe.h"

#include "game/mg/timer.h"

#include "REL/framework/linknode.h"

#include "REL/framework/model.h"

#include "dolphin/types.h"

extern "C" {
#include "game/hsfex.h"
}

extern "C" {
#include "dolphin/os.h"
}

#include "REL/framework/timer.h"

// The render objects.
extern ListRoot<HandleListObject> lbl_1_bss_7C;

#include "REL/framework/handle.h"
extern "C" { extern HandleAllocator *lbl_1_bss_84; }

RegisteredCallbackOwner::RegisteredCallbackOwner(HandleAllocator *allocator)
    : registeredNode(allocator)
{
    lbl_1_bss_84 = registeredNode;
}

void fn_1_5BC8();

RegisteredCallbackOwner::~RegisteredCallbackOwner()
{
    fn_1_5BC8();
}

void fn_1_5B00(void);

void fn_1_5B30(void);

void fn_1_5A54(void);

void fn_1_5A84(HandleListObject *);

void RegisteredCallbackOwner::flagEnabled(int bit)
{
    flags = flags & ~(1UL << bit);
    switch (bit) {
    case 0:
    case 2:
    case 4:
        break;
    case 1:
        fn_1_5B00();
        break;
    case 3:
        fn_1_5B30();
        flags = flags & ~(1UL << bit);
        break;
    }
}

void RegisteredCallbackOwner::update()
{
    fn_1_5A54();
}

void fn_1_5A54(void)
{
    lbl_1_bss_7C.visitMoved(fn_1_5A84);
}

void fn_1_5A84(HandleListObject *object)
{
    object->update();
}

void HandleListObject::update()
{
}

void HandleListObject::drawCallback(HU3D_MODEL *model, Mtx *matrix)
{
    static_cast<HandleListObject *>(model->hookData)->draw(matrix);
}

void fn_1_5B60(HandleListObject *);

void fn_1_5B00(void) {
    lbl_1_bss_7C.visit(fn_1_5B60);
}

void fn_1_5B94(HandleListObject *);

void fn_1_5B30(void) {
    lbl_1_bss_7C.visit(fn_1_5B94);
}

void fn_1_5B60(HandleListObject *object)
{
    object->start();
}

void fn_1_5B94(HandleListObject *object)
{
    object->finish();
}

void fn_1_5BC8() {
    while (!lbl_1_bss_7C.empty()) {
        delete lbl_1_bss_7C.back();
    }
}

HandleListObject::HandleListObject()
    : OwnedModel(drawCallback), registryId(lbl_1_bss_84->acquire(this))
{
    insert4(&lbl_1_bss_7C);
    setHookData(this);
}

HandleListObject::~HandleListObject()
{
    lbl_1_bss_84->release(registryId);
}

void *fn_1_5E1C(HandleListObject *object) {
    return (void *)object->registryId;
}

void *fn_1_5E24(void *handle)
{
    return lbl_1_bss_84->lookup((unsigned long)handle);
}

ListRoot<HandleListObject> lbl_1_bss_7C;
extern "C" {
HandleAllocator *lbl_1_bss_84;
}
