#ifndef REL_M653DLL_MODEL_IMAGE_H
#define REL_M653DLL_MODEL_IMAGE_H
#include "REL/m653DLL/car.h"
#include "REL/framework/p_chara.h"
#include "REL/framework/category_sphere.h"
struct Owner47C { PlayerConfigSelection *value; Owner47C(PlayerConfigSelection *initial) : value(initial) {} ~Owner47C(); };
struct ModelImageObject : GameObject {
    Mtx matrix;
    ModelHolder source;
    ModelHolder resources[6];
    Owner47C image;
    RegisteredValue registeredValue;
    ModelImageObject(OwnedReleasedModel *, OwnedReleasedModel *, PlayerConfigSelection *);
    virtual ~ModelImageObject();
    virtual void update();
    PlayerConfigSelection *imageObject() const { return image.value; }
};

// Places the image object's matrix, its character model and its sphere around a stage point.
void fn_1_6594(ModelImageObject *owner, const Vec *position);
#endif
