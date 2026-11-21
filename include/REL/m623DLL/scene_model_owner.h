#ifndef REL_M623DLL_SCENE_MODEL_OWNER_H
#define REL_M623DLL_SCENE_MODEL_OWNER_H
#include "REL/framework/model.h"
#include "REL/framework/category_sphere.h"
#include "REL/framework/singleton.h"
#include "REL/framework/list_object.h"
#include "dolphin/mtx.h"

// Owns the day and night models and draws the scene's animated path.
class SceneModel : public RequestObject, public MinigameSingleton<SceneModel>, public OwnedModel {
  public:
    SceneModel(unsigned char dayMode);
    virtual ~SceneModel();
    virtual void flagEnabled(int request);
    virtual void update();
    void drawMotion(Mtx modelMatrix);
    // Day or night scenery models.
    OwnedModel unlitModel;
    OwnedModel secondModel;
    OwnedModel thirdModel;
    OwnedModel fourthModel;
    OwnedModel fifthModel;
    OwnedModel shadowReceiver;
    OwnedModel seventhModel;
    OwnedModel eighthModel;
    OwnedModel ninthModel;
    OwnedModel revealModel;
    // The model whose hook draws the animated path.
    OwnedModel pathModel;
    OwnedModel *stageModel;
    OwnedModel *shadowModel;
    RegisteredValue interactionSphere;
    unsigned char enabled;
    int phase;
    // Positions and orientations sampled along the animated path.
    Point3d positions[1001];
    unsigned char visible[1000];
    Quaternion rotations[1000];
    unsigned char dayMode;
};

// Clears attributes 0x40000002 on the scenery model revealed during play.
void fn_1_30A8();

// Initializes the scene models for the active day or night setting.
Model *fn_1_2C50(void);

#endif
