#ifndef REL_M652DLL_PLAYER_WORK_H
#define REL_M652DLL_PLAYER_WORK_H
#include <memory>
#include "humath.h"
#include "REL/framework/chara_motion.h"
#include "REL/framework/game_object.h"
#include "REL/framework/p_info.h"
#include "REL/framework/category_sphere.h"
extern "C" {
#include "game/mg/actman.h"
}
struct Racer;
class RacerControl;
typedef void (Racer::*ItemUpdate5B3C)();
struct MotionFlags {
    MotionFlags() : stateFlag(0), attributeEnabled(0), hasWon(0), collision4(0) {}
    unsigned char stateFlag : 1;
    unsigned char attributeEnabled : 1;
    unsigned char hasWon : 1; /* Set when this player is registered as a winner. */
    unsigned char collision4 : 1;
    unsigned char unknown_3 : 1;
    unsigned char unknown_2 : 1;
    unsigned char unknown_1 : 1;
    unsigned char unknown_0 : 1;
};

struct Racer : PlayerInfo, MotionOwner, GameObject {
    void fn_1_6134();
    void fn_1_61C0();
    unsigned char fn_1_6258();
    unsigned int fn_1_62AC();
    void fn_1_5B28();
    void fn_1_74B0();
    void fn_1_6300();
    unsigned char fn_1_63E8();
    void fn_1_64F0();
    void fn_1_65FC();
    void fn_1_66F4();
    void fn_1_68B4();
    void fn_1_68F8();
    void fn_1_6978();
    void fn_1_6A38();
    void fn_1_6AA4();
    void fn_1_6C30();
    static int fn_1_5B2C();
    static void fn_1_5B3C(ItemUpdate5B3C update);
    Racer(int character, int index, Point3d *position);
    virtual ~Racer();
    virtual void update();
    void (Racer::*callback)();
    int index;
    unsigned short playerMask;
    MGPLAYER *player;
    MotionFlags flags;
    Point3d position;
    Vec motion;
    float rotation;
    RegisteredValue firstRegistered;
    RegisteredValue secondRegistered;
    std::auto_ptr<RacerControl> child;
};

#endif
