#include "dolphin/types.h"

#include "dolphin/mtx.h"
#include "datadir_enum.h"

#include "REL/framework/chara_motion.h"

#include "REL/m610DLL/module_types.h"

#include "REL/framework/model.h"

#include "REL/m610DLL/model_group.h"

#include "REL/framework/p_chara.h"

#include "REL/framework/random.h"

#include "REL/framework/ground_sprite.h"

extern "C"
{
#include "game/hu3d.h"
}

#include "REL/framework/game_object.h"

#include <new>

/* Own the six linked models and start the selected character motion. */
ResultModelOwner::ResultModelOwner(OwnedModel *model, OwnedModel *source,
                                   PlayerConfigSelection *playerConfig)
    : GameObject(0), origin(0), model(model), playerConfig(playerConfig)
{
    models[0].reset(source);
    models[1].reset(new OwnedModel(*source));
    models[2].reset(new OwnedModel(*source));
    models[3].reset(new OwnedModel(*source));
    models[4].reset(new OwnedModel(*source));
    models[5].reset(new OwnedModel(*source));
    for (int index = 0; index < 6; ++index)
    {
        models[index].get()->setShadow();
        models[index].get()->setLayer(5);
    }
    PSMTXIdentity(transform);
    createCharacterMotion(&static_cast<MotionOwner &>(*playerConfiguration()),
                          DATANUM(0x930000, 6));
    shiftCharacterMotion(&static_cast<MotionOwner &>(*playerConfiguration()),
                         DATANUM(0x930000, 6), 10.0f, 0.0f, 0);
}

/* Position the model group and its character around the supplied point. */
void ResultModelOwner::fn_1_C358(const Vec *position)
{
    Mtx rotation;
    Vec transformedPosition = *position;
    Vec childPosition = *position;
    f32 scale;
    f32 translationX;
    f32 translationY;
    f32 translationZ;

    transformedPosition.x -= 80.0f;
    transformedPosition.z -= (100.0f * 1.2f);
    childPosition.z += (100.0f * 1.2f);
    PSMTXRotRad(rotation, 'Y', MTXDegToRad(-60.0f));
    translationZ = transformedPosition.z;
    translationY = transformedPosition.y;
    translationX = transformedPosition.x;
    PSMTXTransApply(rotation, this->transform, translationX, translationY, translationZ);
    this->playerConfig.get()->setPosition(&childPosition);
    this->origin = *position;
    scale = 400.0f;
    this->origin.radius = scale;
}

/* Local positions of the six child models. */
const Vec lbl_1_rodata_400[6] = { { 84.0f, 40.0f, 98.0f },  { 84.0f, 40.0f, 6.0f },
                                         { 84.0f, 40.0f, -82.0f }, { -84.0f, 40.0f, 98.0f },
                                         { -84.0f, 40.0f, 6.0f },  { -84.0f, 40.0f, -82.0f } };

/* Apply the group transform to the six models and the main model. */
void ResultModelOwner::update()
{
    Mtx matrix;
    s32 rotationIndex = 0;
    s32 halfTurn;
    f32 translationX;
    f32 translationY;
    f32 translationZ;

    while (rotationIndex < 6)
    {
        if (rotationIndex >= 3)
        {
            halfTurn = 180;
        }
        else
        {
            halfTurn = 0;
        }
        PSMTXRotRad(matrix, 'Y', MTXDegToRad((f32) halfTurn));
        translationZ = lbl_1_rodata_400[rotationIndex].z;
        translationY = lbl_1_rodata_400[rotationIndex].y;
        translationX = lbl_1_rodata_400[rotationIndex].x;
        PSMTXTransApply(matrix, matrix, translationX, translationY, translationZ);
        PSMTXConcat(transform, matrix, matrix);
        models[rotationIndex]->copy_matrix(matrix);
        rotationIndex += 1;
    }
    model->copy_matrix(transform);
}

/* Release the group models and character owner. */
ResultModelOwner::~ResultModelOwner()
{
}
