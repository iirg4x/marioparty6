#ifndef REL_M653DLL_MODEL_IMAGE_H
#define REL_M653DLL_MODEL_IMAGE_H
#include "REL/m653DLL/car.h"
#include "REL/framework/p_chara.h"
#include "REL/framework/category_sphere.h"
// Owns the winner's character image and deletes it with the image object.
struct SelectionHolder {
    PlayerConfigSelection *value;
    SelectionHolder(PlayerConfigSelection *initial) : value(initial) {}
    ~SelectionHolder();
};
struct ModelImageObject : GameObject {
    Mtx matrix;
    ModelHolder source;
    ModelHolder resources[6];
    SelectionHolder image;
    RegisteredValue registeredValue;
    ModelImageObject(OwnedModel *, OwnedModel *, PlayerConfigSelection *);
    virtual ~ModelImageObject();
    virtual void update();
    PlayerConfigSelection *imageObject() const { return image.value; }
};

// Places the image object's matrix, its character model and its sphere around a stage point.
void fn_1_6594(ModelImageObject *owner, const Vec *position);
#endif
