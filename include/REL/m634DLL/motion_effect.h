#ifndef REL_M634DLL_MOTION_EFFECT_H
#define REL_M634DLL_MOTION_EFFECT_H
#include <vector>
#include "REL/framework/game_object2.h"
extern "C" {
#include "game/sprite.h"
}
#include "dolphin/mtx.h"

struct MotionRecord {
 int duration, state, frameCount;
 Vec position, destination, scale;
 double initialValue, minimumValue, maximumValue;
 double initialProgress, finalProgress, lowerBound, upperBound;
 GXColor color;
};
#include "REL/m634DLL/mode_render_object.h"
struct MotionObject : ModeRenderObject {
 unsigned char unobserved14[4];
 Vec position;
 unsigned char unobserved24[12];
 unsigned char active;
 unsigned char unobserved31[3];
 float phase;
 int state, unobserved;
 ANIMDATA *resource;
 explicit MotionObject(ANIMDATA *resourceId);
 virtual ~MotionObject();
 virtual void update()=0;
 virtual void draw(Mtx*)=0;
 virtual void mode(int)=0;
 virtual void action0()=0;
 virtual void action1()=0;
 virtual void action2()=0;
 virtual void action3()=0;
};
struct MotionEffect:MotionObject {
 int motionCount;
 std::vector<MotionRecord> motions;
 void (MotionEffect::*callback)();
 MotionEffect(ANIMDATA *resourceId,int iterations,int selector);
 virtual ~MotionEffect();
 void initializeMotion(MotionRecord *,int);
 void restartMotion(MotionRecord *);
 void reposition(const Vec *);
 virtual void update();
 virtual void draw(Mtx*);
 virtual void mode(int);
 virtual void action0() {}
 virtual void action1() {}
 virtual void action2() {}
 virtual void action3() {}
};

#endif
