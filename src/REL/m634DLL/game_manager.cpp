// The game manager: the shared sprites and stage models, and the objects it owns.
#include <algorithm>
#include <functional>
#include "REL/m634DLL/game_manager.h"
#include "REL/m634DLL/sequence_owner.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
extern "C" {
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/mg/actman.h"
}
#include "game/gamework.h"
#include "game/object.h"
#include "game/mgdata.h"
#include "game/charman.h"
#include "datadir_enum.h"
#include "REL/m634DLL/model_action.h"

struct CharacterModelParameterVector {
    Vec vector;
};

struct FurniturePlacementParameters {
    CharacterModelParameterVector prefix;
    f32 value;
};

struct CharacterModelParameterTable {
    FurniturePlacementParameters entries[16];
};

#include "REL/m634DLL/object_work.h"

extern "C" {
extern char *lbl_1_data_684[16];
}
void fn_1_CF58(std::vector<OwnedModel *> *models);

// Loads and locks a sprite animation.
static inline ANIMDATA *loadAnimation(int resourceNumber)
{
    ANIMDATA *animation = HuSprAnimRead(fn_80008DD0(resourceNumber, 0x10000000, 2));
    HuSprAnimLock(animation);
    return animation;
}

const s32 lbl_1_rodata_270[4] = { 0x5D0007, 0x5D0006, 0x5D0008, 0x5D0009 };

const s32 lbl_1_rodata_280[1] = { 0x5D0034 };

const s32 lbl_1_rodata_284[3] = { 0x5D005C, 0x5D005D, 0x5D005E };

const u32 lbl_1_rodata_290[2] = { 0x5D000D, 0x5D0035 };

const u32 lbl_1_rodata_298[2] = { 0x5D0013, 0x5D003B };

const u32 lbl_1_rodata_2A0[2] = { 0x5D0012, 0x5D003A };

// Loads the scene sprite animations and models.
GameManager::GameManager()
{
    int baseIndex;
    int nightIndex;
    int nightCount;
    int modelIndex;
    int resourceNumber;
    int modelCount;

    for (baseIndex = 0; baseIndex < 4; ++baseIndex) {
        animations.push_back(loadAnimation(lbl_1_rodata_270[baseIndex]));
    }
    nightCount = (GWMgNightFGet() == 0) ? 1 : 3;
    for (nightIndex = 0; nightIndex < nightCount; ++nightIndex) {
        resourceNumber
            = GWMgNightFGet() == 0 ? lbl_1_rodata_280[nightIndex] : lbl_1_rodata_284[nightIndex];
        animations.push_back(loadAnimation(resourceNumber));
    }
    modelCount = models.size();
    for (modelIndex = 0; modelIndex < modelCount; ++modelIndex) {
        models[modelIndex]->setAttribute(1);
    }
    registerChannel(1);
    registerChannel(2);
}

// Releases the scene sprite animations and models.
GameManager::~GameManager()
{
    std::for_each(animations.begin(), animations.end(), std::ptr_fun(HuSprAnimKill));
    fn_1_CF58(&models);
}

// Configures the scene models for the day or night setting.
void fn_1_C124(GameManager *manager)
{
    s32 nightFlag = GWMgNightFGet();
    OwnedModel *baseAnimation;
    OwnedModel *secondAnimation;
    OwnedModel *thirdAnimation;
    OwnedModel *fourthAnimation;

    s16 bankNumber;

    Hu3DBGColorSet(0, 0, 0);
    baseAnimation = new OwnedModel(DATANUM(0x5d0000, 0));
    secondAnimation = new OwnedModel(lbl_1_rodata_298[nightFlag]);
    thirdAnimation = new OwnedModel(DATANUM(0x5d0000, 12));
    fourthAnimation = new OwnedModel(lbl_1_rodata_290[nightFlag]);
    baseAnimation->setAttribute(1);
    secondAnimation->setLayer(0);
    secondAnimation->setCamera(2);
    thirdAnimation->setLayer(0);
    thirdAnimation->setCamera(2);
    fourthAnimation->setLightsDynamic(false);
    fourthAnimation->setLayer(0);
    fourthAnimation->setCamera(2);

    manager->models.push_back(baseAnimation);
    manager->models.push_back(secondAnimation);
    manager->models.push_back(thirdAnimation);
    manager->models.push_back(fourthAnimation);

    {
        Vec lightPosition = { 0.0f, 4500.0f, 4500.0f };
        Vec targetPosition = { 0.0f, 0.0f, 0.0f };
        Vec lightDirection;
        Vec shadowPosition;
        Vec lightUp;

        PSVECSubtract(&targetPosition, &lightPosition, &lightDirection);
        PSVECNormalize(&lightDirection, &lightDirection);
        fn_8002A3B0(30.0f, 1000.0f, 8000.0f, 2);
        PSVECScale(&lightDirection, &shadowPosition, -4000.0f);
        PSVECAdd(&shadowPosition, &targetPosition, &shadowPosition);
        lightUp.x = 0.0f;
        lightUp.y = 1.0f;
        lightUp.z = 0.0f;
        fn_8002A674(&shadowPosition, &lightUp, &targetPosition, 2);
        fn_8002AA3C(0, 0, 0x40, 2);
        fn_8002A8CC(0x80, 2);
        fn_8002A7D8(0.3f, 2);
    }

    bankNumber = *baseAnimation;
    fn_800804E8(&bankNumber, 1, 0x28);
    new ModelActionOwner(DATANUM(0x5d0000, 10), 4, 5);
    new ModelActionOwner(DATANUM(0x5d0000, 11), 6, 7);
    new ObjectWork(lbl_1_rodata_2A0[nightFlag]);
}

extern "C" {
char *lbl_1_data_684[16] = { "bed", "board", "clock", "flower", "fplace", "kitchen", "lbooks",
    "mirrer", "piano", "shelf", "sideb", "sofa", "table", "toy", "tv", "ward" };
}

const s32 lbl_1_rodata_2DC[16]
    = { 112, 128, 128, 128, 128, 128, 128, 112, 128, 128, 128, 128, 128, 112, 128, 112 };

const CharacterModelParameterTable lbl_1_rodata_31C
    = { { { { { 200.0f, 200.0f, 240.00002f } }, 0.0f }, { { { 190.0f, 200.0f, 110.0f } }, 45.0f },
        { { { 120.00001f, 200.0f, 100.0f } }, 0.0f }, { { { 100.0f, 200.0f, 130.0f } }, 0.0f },
        { { { 160.0f, 300.0f, 209.99998f } }, 0.0f }, { { { 300.0f, 200.0f, 200.0f } }, 45.0f },
        { { { 100.0f, 200.0f, 100.0f } }, 0.0f }, { { { 200.0f, 200.0f, 80.0f } }, 0.0f },
        { { { 240.00002f, 200.0f, 180.0f } }, 45.0f }, { { { 120.00001f, 200.0f, 110.0f } }, 0.0f },
        { { { 200.0f, 200.0f, 100.0f } }, 0.0f }, { { { 200.0f, 200.0f, 290.0f } }, 0.0f },
        { { { 200.0f, 200.0f, 250.0f } }, 45.0f }, { { { 190.0f, 200.0f, 190.0f } }, 0.0f },
        { { { 140.0f, 200.0f, 130.0f } }, 45.0f }, { { { 150.0f, 200.0f, 150.0f } }, 0.0f } } };

// Creates the furniture models from their placement parameters.
void fn_1_C534(GameManager *manager)
{
    CharacterModelParameterTable parameters;
    s32 index;

    parameters = lbl_1_rodata_31C;
    for (index = 0; index < 16; index++) {
        s32 firstId;
        s32 secondId;
        s16 firstNightFlag = lbl_802C0278;
        s16 secondNightFlag;

        firstId
            = (firstNightFlag == 0) ? index + DATANUM(0x5d0000, 20) : index + DATANUM(0x5d0000, 60);

        secondNightFlag = lbl_802C0278;
        secondId = (secondNightFlag == 0) ? index + DATANUM(0x5d0000, 36)
                                          : index + DATANUM(0x5d0000, 76);

        new SequenceOwner(firstId, secondId, lbl_1_data_684[index], lbl_1_rodata_2DC[index],
            parameters.entries[index].prefix.vector, parameters.entries[index].value);
    }
}
