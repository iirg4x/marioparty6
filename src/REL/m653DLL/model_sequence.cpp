#include "dolphin/mtx.h"

#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"

#include "REL/framework/model.h"

#include "REL/m653DLL/resource_handles.h"

#include "datadir_enum.h"

#include "game/hu3d.h"

#include "REL/m653DLL/car.h"

#include "REL/framework/p_chara.h"

#include "REL/m653DLL/boundary_plane.h"

#include "REL/framework/game_object_man.h"

#include "REL/m653DLL/motion_work.h"

#include "REL/framework/category_sphere.h"

#include "REL/m653DLL.h"

#include "REL/m653DLL/model_image.h"
#include "REL/m653DLL/translate_matrix.h"

ModelImageObject::ModelImageObject(OwnedModel *source,
                                      OwnedModel *resource, PlayerConfigSelection *image)
    : GameObject(0), source(source), image(image), registeredValue(0)
{
    this->resources[0].reset(resource);
    this->resources[1].copy(resource);
    this->resources[2].copy(resource);
    this->resources[3].copy(resource);
    this->resources[4].copy(resource);
    this->resources[5].copy(resource);
    for (int index = 0; index < 6; ++index) {
        this->resources[index].get()->setShadow();
        this->resources[index].get()->setLayer(2);
    }
    PSMTXIdentity(this->matrix);
    createCharacterMotion(&static_cast<MotionOwner &>(*this->imageObject()), DATANUM(0x930000, 6));
    shiftCharacterMotion(&static_cast<MotionOwner &>(*this->imageObject()), DATANUM(0x930000, 6),
               10.0f, 0.0f, 0);

}

SelectionHolder::~SelectionHolder() { delete value; }

/* Rotates the input point into the object's matrix and updates its child target and scale. */
void fn_1_6594(ModelImageObject *owner, const Vec *position)
{
    Mtx rotation;
    Vec transformedPosition = *position;
    Vec childPosition = *position;
    f32 radius;
    f32 translationX;
    f32 translationY;
    f32 translationZ;

    transformedPosition.x -= 80.0f;
    transformedPosition.z -= 120.00001f;
    childPosition.z += 120.00001f;
    PSMTXRotRad(rotation, 'Y', -1.0471976f);
    translationZ = transformedPosition.z;
    translationY = transformedPosition.y;
    translationX = transformedPosition.x;
    PSMTXTransApply(rotation, owner->matrix,
                    translationX, translationY, translationZ);
    owner->imageObject()->setPosition(&childPosition);
    owner->registeredValue = *position;
    radius = 400.0f;
    owner->registeredValue.radius = radius;
}

extern "C" {
/* Applies six translated rotations to the node resources and refreshes the root matrix. */
/* The six fixed positions used by this model group. */
const Vec lbl_1_rodata_288[6] = {
    {84.0f, 40.0f, 98.0f},
    {84.0f, 40.0f, 6.0f},
    {84.0f, 40.0f, -82.0f},
    {-84.0f, 40.0f, 98.0f},
    {-84.0f, 40.0f, 6.0f},
    {-84.0f, 40.0f, -82.0f},
};
}

void ModelImageObject::update()
{
    Mtx matrix;
    s32 rotationIndex = 0;
    s32 halfTurn;
    Model *wheel;
    Model *body;

    while (rotationIndex < 6) {
        if (rotationIndex >= 3) {
            halfTurn = 0xB4;
        } else {
            halfTurn = 0;
        }
        PSMTXRotRad(matrix, 'Y', MTXDegToRad((f32)halfTurn));
        translateMatrix(matrix, matrix, lbl_1_rodata_288[rotationIndex].x,
                        lbl_1_rodata_288[rotationIndex].y, lbl_1_rodata_288[rotationIndex].z);
        PSMTXConcat(this->matrix, matrix, matrix);
        wheel = this->resources[rotationIndex].value;
        wheel->copy_matrix(matrix);
        rotationIndex += 1;
    }
    body = this->source.value;
    body->copy_matrix(this->matrix);
}

ModelImageObject::~ModelImageObject()
{
}
