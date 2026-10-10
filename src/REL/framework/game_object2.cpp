// Game objects that draw through an owned Hu3D model hook.
static const char rcsid[] = "$Id: game_object2.cpp,v 1.21 2004/09/09 12:24:38 saf Exp $";

#include "REL/framework/game_object2.h"

// During derived-object construction, registers its category and creates its drawing hook model.
RenderObject::RenderObject(int category)
    : GameObject(category), OwnedModel(drawCallback)
{
    // The engine passes the model to the hook, so keep the owning object in its hook data.
    setHookData(this);
}

// On derived-object destruction, the bases release the model, then unlink the collection node.
RenderObject::~RenderObject()
{
}

// Hu3D's drawing pipeline calls this hook to draw the owning object with the model's draw matrix.
void RenderObject::drawCallback(HU3D_MODEL *model, Mtx *matrix)
{
    static_cast<RenderObject *>(model->hookData)->draw(matrix);
}
