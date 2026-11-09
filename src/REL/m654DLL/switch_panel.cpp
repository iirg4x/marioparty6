#include "REL/m654DLL/framework_calls.h"
#include "REL/m654DLL/time_conversion.h"
#include "REL/m654DLL/unit_conversion.h"
// The floor switches: shuffles the panels into the rows, shows which were stepped on and plays each panel's effect.
#include <vector>
#include <algorithm>
#include <cmath>
#include "REL/framework/event_packet.h"
#include "REL/framework/game_object_man.h"
#include "REL/m654DLL/model_animation_owner.h"

extern "C" {
#include "REL/m654DLL/pad_control.h"
#include "game/audio.h"
extern const int lbl_1_rodata_3B8[3] = {0x710005, 0x710006, 0x710007};
extern const int lbl_1_rodata_3C4[3] = {0x710008, 0x710009, 0x71000B};
extern const unsigned int lbl_1_rodata_3D0[71] = {
    0x0082FCE1, 0x0119910D, 0x017F8444, 0x027CF502, 0x0323043B, 0x0330996C,
    0x0AE3A80E, 0x0B2F6F96, 0x0CCF181E, 0x0DD9B868, 0x0EEC8598, 0x100573FE,
    0x13A8A37A, 0x15B3E39C, 0x16E4EDA8, 0x17612174, 0x1AF3DF7E, 0x1C485B70,
    0x1EE3882F, 0x1F605CA0, 0x1F8CE492, 0x20B0EE59, 0x214F0C51, 0x2870D22A,
    0x28DCDE3C, 0x29315BFD, 0x29FEA8F0, 0x2BABEC0D, 0x2BE576B1, 0x2CBD7A9B,
    0x2FB522C3, 0x30003610, 0x31020380, 0x34153691, 0x342A8015, 0x34644A63,
    0x36107B91, 0x383696B2, 0x393C19C2, 0x3A6F8CBB, 0x3AC0DEC9, 0x3B896FDA,
    0x3FDE13AD, 0x423C5227, 0x4262440D, 0x43F667CA, 0x457F43CF, 0x4784ABCC,
    0x4793CDF9, 0x489F44B1, 0x49429F30, 0x4943AE90, 0x49ECA332, 0x52F05780,
    0x585FD426, 0x5E9E4222, 0x5FC639BD, 0x65EDD99E, 0x6854FF8F, 0x685AFB0E,
    0x6A1455AF, 0x6C791188, 0x6E262F78, 0x6FD122ED, 0x70022C70, 0x70F5CCD4,
    0x72835779, 0x76B82B8C, 0x78155BE7, 0x78274CA5, 0x79C4BA46,
};
char *lbl_1_data_498[2] = {"s1_null", 0};
char *lbl_1_data_4C0[5] = {"s2_null", "s3_null", "s4_null", "s5_null", 0};
char *lbl_1_data_4F4[5] = {"s6_null", "s7_null", "s8_null", "s9_null", 0};
char *lbl_1_data_52C[5] = {"s11_null", "s14_null", "s17_null", "s20null2", 0};
char *lbl_1_data_588[9] = {"s10_null", "s12_null", "s13_null", "s15_null", "s16_null", "s18_null", "s19_null", "s21_null", 0};
char **lbl_1_data_5AC[5] = {lbl_1_data_498, lbl_1_data_4C0, lbl_1_data_4F4, lbl_1_data_52C, lbl_1_data_588};
void __msl_assertion_failed(const char *expression, const char *file, int line);
int lbl_1_bss_C0 = fn_1_B80(1);
float lbl_1_bss_C4 = framesToSeconds(framesToSeconds(980.665));
std::vector<short> lbl_1_bss_D4;
}

// The panel kinds; the constructor's assert names NUMBER_OF_TYPE.
enum PanelType { NUMBER_OF_TYPE = 3 };

void RandomSeed(unsigned long seed);
unsigned long RandomUnsigned();
long RandomInteger(long range);

int fn_1_85D8(int range)
{
    return RandomInteger(range);
}

class SwitchPanel : public GameObject, public EventListener {
public:
    SwitchPanel(int type, int initialMode, Model *initialValue, char *setting);
    virtual ~SwitchPanel();
    virtual void update();
    virtual void receive(const EventPacket *event);
    void fn_1_8EF0();
    void fn_1_8F40();
    void fn_1_90FC(int value, Vec *position);
    void fn_1_94A4(Vec *position);

    Model *unknown14;
    char *unknown18;
    int type;
    unsigned char active : 1;
    unsigned char unknown20 : 7;
    unsigned char unknown21;
    OwnedModel firstModel;
    OwnedModel secondModel;
    unsigned char unknown26[2]; /* No code names this field. */
    Vec position;
    int soundHandle;
    int soundCountdown;
};

extern "C" {
void fn_1_8600(Model *setting);
int fn_1_8AD4(std::vector<Vec> *owner);
}

void fn_1_8600(Model *setting)
{
    char **configuration;
    unsigned int seed = lbl_1_rodata_3D0[(int)RandomUnsigned() % 71];
    RandomSeed(seed);
    std::vector<int> types(0x15);
    types.clear();
    types.insert(types.end(), 5, (PanelType)0);
    types.insert(types.end(), 4, (PanelType)1);
    types.insert(types.end(), 0xC, (PanelType)2);
    for (int shufflePass = 0; shufflePass < 0xD; shufflePass++) {
        std::random_shuffle(types.begin(), types.end(), fn_1_85D8);
    }
    for (int panelRow = 0; panelRow < 5; panelRow += 1) {
        configuration = lbl_1_data_5AC[panelRow];
        while (*configuration != 0) {
            new SwitchPanel(types.back(), panelRow, setting, *configuration++);
            types.pop_back();
        }
    }
    types.empty() ? (void)0 : (void)__msl_assertion_failed("types.empty()", "switch_panel.cpp", 0x5C);
    lbl_1_bss_D4.clear();
    for (int modelIndex = 0; modelIndex < 3; modelIndex++) {
        Model *model = new OwnedModel(lbl_1_rodata_3C4[modelIndex]);
        model->fn_1_33950((1 << 0));
        lbl_1_bss_D4.push_back(*model);
    }
}

int fn_1_8AD4(std::vector<Vec> *owner)
{
    std::vector<GameObject *> sequence;
    int completedCount;
    int index;
    collectGameObjects(1, &sequence);
    int count = sequence.size();
    owner->clear();
    completedCount = 0;
    index = 0;
    while (index < count) {
        SwitchPanel *entry = static_cast<SwitchPanel *>(sequence[index]);
        if ((unsigned char)entry->active == 0) {
            if (entry->type == 0) {
                owner->insert(owner->begin(), entry->position);
                completedCount++;
            } else {
                owner->push_back(entry->position);
            }
        }
        index++;
    }
    return completedCount;
}

SwitchPanel::SwitchPanel(int type, int initialMode, Model *initialValue, char *setting)
    : GameObject(1),
      unknown14(initialValue),
      unknown18(setting),
      type(type),
      firstModel(0x710004),
      secondModel(lbl_1_rodata_3B8[type]),
      soundHandle(-1),
      soundCountdown(0)
{
    int index;

    ((type >= 0) && (type < NUMBER_OF_TYPE)) ? (void)0
        : __msl_assertion_failed("(type >= 0) && (type < NUMBER_OF_TYPE)", "switch_panel.cpp", 0x87);

    firstModel.fn_1_33CB0(2);
    secondModel.fn_1_33CB0(2);
    firstModel.fn_1_34108();
    secondModel.fn_1_34108();
    fn_1_8EF0();
    registerChannel(initialMode + 5);
    for (index = 0; index < 4; index++) {
        registerChannel(index + 1);
    }
}

extern "C" void fn_800262D0(short modelId);

SwitchPanel::~SwitchPanel()
{
    while (!lbl_1_bss_D4.empty()) {
        fn_800262D0(lbl_1_bss_D4.back());
        lbl_1_bss_D4.pop_back();
    }
}

void SwitchPanel::fn_1_8EF0()
{
    firstModel.fn_1_33950(1);
    secondModel.fn_1_33988(1);
    active = 1;
}

void SwitchPanel::fn_1_8F40()
{
    firstModel.fn_1_33988(1);
    secondModel.fn_1_33950(1);
    active = 0;
}

int fn_1_AD90(s16 cameraMask, s32 effectId, Point3d *targetPoint);
extern "C" {

void fn_8002C95C(int modelId, short motionId);
void fn_800269E8(short modelId, unsigned long attribute);
void fn_80026790(short modelId, Vec *position);
int fn_80036EBC(short modelId);
void fn_80026B8C(short modelId, short layer);
void fn_80026A44(short modelId, unsigned long attribute);
void fn_8002D414(short modelId, float time);
float fn_8002D530(short modelId);
}

void SwitchPanel::update()
{
    float value;
    unsigned char stopSound;

    unknown14->fn_1_344A0(unknown18, &position);
    value = unknown14->fn_1_33BAC();
    firstModel.fn_1_33A3C(&position);
    firstModel.fn_1_33B5C(value);
    secondModel.fn_1_33A3C(&position);
    secondModel.fn_1_33B5C(value);
    if (soundHandle != -1) {
        stopSound = 0;
        if (soundCountdown > 0 && (soundCountdown -= 1) == 0) {
            stopSound = 1;
        }
        if (stopSound != 0) {
            HuAudFXStop(soundHandle);
            soundHandle = -1;
        }
    }
}

void SwitchPanel::receive(const EventPacket *event)
{
    switch (event->type) {
    case 0:
        fn_1_8F40();
        return;
    case 1:
        if (event->context != 0) {
            fn_1_90FC(event->channel, (Vec *)event->context);
        }
    }
}

static inline float Square90FC(const float &value)
{
    return value * value;
}

void SwitchPanel::fn_1_90FC(int value, Vec *target)
{
    float distance;
    if (!active) {
        distance = (float)std::sqrt(Square90FC(target->x - position.x) + Square90FC(target->z - position.z));
        if (!(distance > 75.0)) {
            fn_1_8EF0();
            if (type == 2) {
                fn_1_94A4(&position);
                return;
            }
            int modeData[2] = {2, 3};
            sendEvent(value, modeData[type], 0.0f);
            switch (type) {
            case 0:
                fn_1_94A4(&position);
                soundCountdown = 0;
                fn_1_AD90(1, 0x81B, &position);
                break;
            case 1: {
                Vec targetPosition = {0.0f, 0.0f, 0.0f};
                targetPosition.x = target->x;
                targetPosition.y = position.y;
                targetPosition.z = target->z;
                fn_1_94A4(&targetPosition);
                MinigameSingleton<ModelAnimationOwner>::getChecked().fn_1_226C();
                soundHandle = fn_1_AD90(1, 0x81A, &targetPosition);
                break;
            }
            default:
                break;
            }
        }
    }
}

void SwitchPanel::fn_1_94A4(Vec *position)
{
    int modelIndex = type;
    int duration;
    short &entry = lbl_1_bss_D4[modelIndex];
    int modelId = (short)entry;
    int linkedModelId;

    fn_8002C95C(modelId, -1);
    fn_800269E8(modelId, 0x40000002);
    fn_80026790(modelId, position);
    linkedModelId = fn_80036EBC(modelId);
    fn_80026B8C(linkedModelId, 4);
    fn_80026A44(linkedModelId, 1);
    fn_80026A44(linkedModelId, 0x40000003);
    fn_8002D414(linkedModelId, 0.0f);
    fn_80026790(linkedModelId, position);
    duration = (int)fn_8002D530(linkedModelId);
    soundCountdown = duration;
}
