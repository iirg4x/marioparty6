#include "REL/m652DLL/map_singleton.h"
#include "REL/m652DLL/map_resource.h"
#include "dolphin/types.h"
#include "REL/m652DLL/node_owner.h"
#include "REL/framework/minigame_singleton.h"
extern "C" {
#include "game/hu3d.h"
#include "game/mg/actman.h"
}

extern "C" {
#include "dolphin.h"
}
class Model {
public:
    short modelId; // Index in Hu3DData, or -1 when no model is owned.

    inline ~Model() { Replace(-1); }
    operator short() const { return modelId; }
    short Replace(short replacement);
    void setHookData(void *owner);
    void fn_1_33CB0(short layer);
    int fn_1_33DF8(bool enabled);
    void fn_1_33D68(char *, Vec *, Vec *, GXColor *);
    void fn_1_33950(unsigned long attributes);
    void fn_1_34108();
    void fn_1_34528(float);
    float fn_1_3464C();
    void fn_1_33CE8(unsigned short);
    void fn_1_33988(unsigned long);
    void fn_1_33A3C(Vec *);
    void fn_1_33ACC(Vec *);
    void fn_1_33C30(float);
    void fn_1_33F98();
    void fn_1_340D8();
    void fn_1_344F0(float);
    Vec *fn_1_344A0(char *, Vec *);
    void fn_1_34734();
    float fn_1_34560();
};

// Holds a model created with a custom Hu3D drawing callback.
struct OwnedModel : Model {
    OwnedModel(const OwnedModel&);
    void created(short value);
    explicit OwnedModel(void*);
    OwnedModel(int character,int variant);
    void apply_defaults();
    void load(unsigned int resource);
    explicit OwnedModel(HU3D_MODEL_HOOK hook);
    explicit OwnedModel(unsigned int resource);
    ~OwnedModel();
};

inline OwnedModel::~OwnedModel() {}

#include "REL/framework/linknode.h"
#include "REL/framework/game_object.h"
#include "REL/framework/scene_shadow.h"
class MapManager;
void fn_1_50C8(const MapManager *);

extern "C" {
const unsigned int lbl_1_rodata_2A0[3][2] = {{0x6F0004, 0x6F0005}, {0x6F0006, 0x6F0007}, {0x6F0008, 0x6F0009}};
}

void fn_1_488C(short)
{
    GXColor color = {0, 0, 0, 0};
    float nearPlane;
    float farPlane;
    float fov;
    fn_80027D8C(1, &fov, &nearPlane, &farPlane);
    GXSetFog(GX_FOG_EXP, 2000.0f, 4200.0f, nearPlane, farPlane, color);
}

void fn_1_493C(short)
{
    GXColor color = {0, 0, 0, 0};
    GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, color);
}

inline MapResource::~MapResource() { delete value; }

inline MapResource::operator OwnedModel *() const { return value; }
inline OwnedModel &MapResource::operator*() const { return *value; }
inline void MapResource::reset(OwnedModel *replacement)
{
    if (value != replacement) {
        delete value;
        value = replacement;
    }
}

class MapManager : public FlagNode,
                    public MinigameSingleton<MapManager> {
public:
    OwnedModel layer0;
    OwnedModel layer1;
    OwnedModel layer2;
    MapResource groups[5][2];
    MapResource menuLayers[4];
    float previousValue;
    MapManager();
    virtual ~MapManager();
    virtual void flagEnabled(int);
    virtual void update();
};

extern "C" {
extern void __msl_assertion_failed(const char *, const char *, int);
extern void *fn_1_17544(void *);
extern void *fn_1_17608(void *, const void *);
extern void *__ct__10OwnedModelFUi(void *, unsigned int);
extern void *fn_1_192C0(void *, int);
extern void *fn_1_1D638(void *, short, Vec *, float, int);
extern void *fn_1_A900(unsigned int);
extern void *fn_1_AA00(void *);
extern FlagNodeList lbl_1_bss_EC;
}

inline void MapResource::configure(unsigned int visibility, unsigned long attributes)
{
    (**this).fn_1_33CE8(visibility);
    (**this).fn_1_33CB0(1);
    (**this).fn_1_33950(attributes);
    (**this).fn_1_34108();
}

MapManager::MapManager()
    : layer0(0x6F0000u), layer1(0x6F0001u), layer2(0x6F0003u)
{
    int group;
    int member;
    int menuIndex;
    int lightLayer;
    int lights;
    Vec lightDirection;
    Vec lightPosition;
    GXColor lightColor;
    const unsigned int *rowResources;
    SceneShadow * light;
    previousValue = 0.0f;
    layer0.fn_1_33CB0(1);
    layer1.fn_1_33CB0(1);
    layer1.fn_1_34108();
    for (group = 0; group < 3; group++)
    {
        rowResources = lbl_1_rodata_2A0[group == 2 ? 2 : (group & 1)];
        unsigned short visibility = (unsigned short)(group == 2 ? 3 : (1 << group));
        for (member = 0; member < 2; member++)
        {
            MapResource * slot = & groups[group][member];
            slot->reset(new OwnedModel(rowResources[member]));
            slot->configure(visibility, ((1 << 30) | (1 << 0)));
        }
    }
    for (menuIndex = 0; menuIndex < 2; menuIndex++)
    {
        unsigned short visibility = (unsigned short)(1 << menuIndex);
        MapResource * slot = & menuLayers[menuIndex];
        slot->reset(new OwnedModel(0x6F0002));
        slot->configure(visibility, ((1 << 30) | (1 << 1)));
    }
    lights = layer0.fn_1_33DF8(0);
    (lights > 0) ? (void) 0 : __msl_assertion_failed("lights > 0", "man_map.cpp", 0x5B);
    layer0.fn_1_33D68("inf_light1", &lightPosition, &lightDirection, &lightColor);
    light = new SceneShadow(3, &lightDirection, 5.0f, 0x64);
    for (lightLayer = 0; lightLayer < 2; lightLayer++)
    {
        light->setCategory(lightLayer + 2, (short)(1 << lightLayer));
    }
    light->setHalfResolution(true);
    fn_80025918(1, fn_1_493C);
    fn_80025918(2, fn_1_488C);
    fn_80025918(7, fn_1_493C);
    fn_1_50C8(this);
}

MapManager::~MapManager() {}

extern "C" {
char lbl_1_data_2B8[] = "pass000";
char lbl_1_data_2C0[] = "pass001";
char lbl_1_data_2C8[] = "pass002";
char lbl_1_data_2D0[] = "pass003";
char lbl_1_data_2D8[] = "pass004";
char lbl_1_data_2E0[] = "pass005";
char lbl_1_data_2E8[] = "pass006";
char lbl_1_data_2F0[] = "pass007";
char lbl_1_data_2F8[] = "pass008";
char lbl_1_data_300[] = "pass009";
char lbl_1_data_308[] = "pass010";
char *lbl_1_data_310[11] = {lbl_1_data_2B8, lbl_1_data_2C0, lbl_1_data_2C8, lbl_1_data_2D0, lbl_1_data_2D8, lbl_1_data_2E0, lbl_1_data_2E8, lbl_1_data_2F0, lbl_1_data_2F8, lbl_1_data_300, lbl_1_data_308};
char lbl_1_data_33C[] = "pass100";
char lbl_1_data_344[] = "pass101";
char lbl_1_data_34C[] = "pass102";
char lbl_1_data_354[] = "pass103";
char lbl_1_data_35C[] = "pass104";
char lbl_1_data_364[] = "pass105";
char lbl_1_data_36C[] = "pass106";
char lbl_1_data_374[] = "pass107";
char lbl_1_data_37C[] = "pass108";
char lbl_1_data_384[] = "pass109";
char lbl_1_data_38C[] = "pass110";
char *lbl_1_data_394[11] = {lbl_1_data_33C, lbl_1_data_344, lbl_1_data_34C, lbl_1_data_354, lbl_1_data_35C, lbl_1_data_364, lbl_1_data_36C, lbl_1_data_374, lbl_1_data_37C, lbl_1_data_384, lbl_1_data_38C};
char **lbl_1_data_3C0[2] = {lbl_1_data_310, lbl_1_data_394};

}

void fn_1_50C8(const MapManager *work)
{
    int index;
    short attributes[3] = { 0, 0, 0 };

    attributes[0] = work->layer2;
    attributes[1] = *work->groups[2][0];
    attributes[2] = *work->groups[2][1];
    fn_800804E8(attributes, 3, 8);
    for (index = 0; index < 3; index++) {
        fn_800269E8((int)(short)attributes[index], 1);
    }
}

extern "C" const int lbl_1_rodata_2D8[2] = {11, 11};

unsigned char fn_1_5190(MapManager *work, int modelIndex, int itemIndex, Vec *position)
{
    char *modelItem;
    Model *model;

    if (modelIndex < 0 || modelIndex >= 2) {
        return 0;
    }
    if (itemIndex < 0 || itemIndex >= lbl_1_rodata_2D8[modelIndex]) {
        return 0;
    }
    modelItem = lbl_1_data_3C0[modelIndex][itemIndex];
    model = work->groups[2][modelIndex].value;
    model->fn_1_344A0(modelItem, position);
    return 1;
}

void fn_1_9308(s16 cameraMask, s32 effectId, Vec *targetPoint);

extern "C" {
const Vec lbl_1_rodata_2E0 = {0, 0.0f, 0.0f};
int fn_1_33988__5ModelFUl(void *model, int attributes);

}

void fn_1_524C(MapManager *work, int modelIndex)
{
    short flags;
    int setupMode;
    void *model;
    int mode;

    model = work->menuLayers[modelIndex].value;
    fn_1_33988__5ModelFUl(model, ((1 << 30) | (1 << 1)));
    flags = 1 << modelIndex;
    if (modelIndex == 0) {
        mode = 0x811;
    } else {
        mode = 0x812;
    }
    setupMode = mode;
    Vec setup = lbl_1_rodata_2E0;
    setup.y = 360.0f;
    setup.z = -2700.0f;
    fn_1_9308((short)flags, setupMode, &setup);
}

void MapManager::flagEnabled(int flagIndex)
{
    COL_ATTRPARAM parameters;
    OwnedModel **actorSlot;
    void *actor;
    int outerIndex;
    int innerIndex;

    if (flagIndex == 3) {
        parameters.type = 1;
        parameters.speed = 0.0f;
        parameters.yDeviate = 1.0f;
        parameters.maxDot = 1.0f;
        parameters.attr = 0;
        fn_800870A4(&parameters, 1);
        for (outerIndex = 0; outerIndex < 3; outerIndex++) {
            for (innerIndex = 0; innerIndex < 2; innerIndex++) {
                actorSlot = &this->groups[outerIndex][innerIndex].value;
                actor = *actorSlot;
                fn_1_33988__5ModelFUl(actor, ((1 << 30) | (1 << 0)));
            }
        }
    }
    this->flags &= ~(1u << flagIndex);
}

void fn_1_5434(MapManager *work);
void fn_1_54B4(MapManager *work);

void MapManager::update()
{
    fn_1_5434(this);
    fn_1_54B4(this);
}

void fn_1_5434(MapManager *work)
{
    Model *firstModel;
    Model *secondModel;
    Model *thirdModel;
    OwnedModel **modelEntry;
    int index;

    index = 0;
    while (index < 2) {
        modelEntry = &work->groups[2][index].value;
        firstModel = *modelEntry;
        firstModel->fn_1_33988(1);
        secondModel = *modelEntry;
        secondModel->fn_1_34734();
        thirdModel = *modelEntry;
        thirdModel->fn_1_33950(1);
        index += 1;
    }
}

void fn_1_54B4(MapManager *work)
{
    float value;
    unsigned char shouldUpdate;
    int thresholdIndex;
    int *threshold;
    int modelIndex;
    short flags;
    int model;
    int setupMode;
    Model *modelObject;

    modelObject = work->groups[2][0].value;
    value = modelObject->fn_1_34560();
    int thresholds[] = {180, 380};
    shouldUpdate = 0;
    for (thresholdIndex = 0; thresholdIndex < 2; thresholdIndex += 1) {
        threshold = &thresholds[thresholdIndex];
        if (work->previousValue < (float)*threshold) {
            if (value >= (float)*threshold) {
                shouldUpdate = 1;
            }
        }
    }
    if (shouldUpdate) {
        Vec setup = {0, 0.0f, 0.0f};
        setup.y = 250.0f;
        setup.z = -2700.0f;
        for (modelIndex = 0; modelIndex < 2; modelIndex += 1) {
            flags = 1 << modelIndex;
            if (modelIndex == 0) {
                model = 0x80D;
            } else {
                model = 0x80E;
            }
            setupMode = model;
            fn_1_9308((short)flags, setupMode, &setup);
        }
    }
    work->previousValue = value;
}
