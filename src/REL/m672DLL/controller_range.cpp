#include "dolphin/types.h"

extern "C" {
#include <math.h>
}

extern "C" {
#include "dolphin/os.h"
}

extern "C" {
#include "game/mg/actman.h"
}

extern "C" {
#include "game/hu3d.h"
}

extern "C" {
#include "game/charman.h"
}

#include "REL/m672DLL/framework/range.h"

#include "REL/m672DLL/controllers_bf64.h"
#include "humath.h"
VirtualPad::VirtualPad(u16 cameraType,float initial)
 : type((u32)cameraType), bounds(-3.1415927f,3.1415927f,initial)
{
 horizontal=0.0f; vertical=0.0f; buttonsDown=0; buttons=0;
}
VirtualPad::~VirtualPad() {}

void VirtualPad::fn_1_12AB8(f32 value)
{
    bounds.fn_1_12F88(value);
    horizontal = 0.0f;
    vertical = 0.0f;
    buttonsDown = 0;
    buttons = 0;
}

void VirtualPad::fn_1_12B20(u32 flags) {
    buttons |= flags;
}

void VirtualPad::fn_1_12B30(u32 flags) {
    buttonsDown |= flags;
}

void VirtualPad::fn_1_12B40() {
    buttonsDown |= 0x200;
}

void VirtualPad::fn_1_12B50() {
    buttonsDown |= 0x100;
}

void VirtualPad::fn_1_12B60() {
    buttons |= 0x100;
}

extern "C" const Point3d lbl_1_rodata_774 = {0.0f, 0.0f, 0.0f};

void VirtualPad::fn_1_12B70(const Vec *movement, f32 amount)
{
    Point3d cameraMovement = lbl_1_rodata_774;
    Point3d cameraPosition;
    Point3d cameraUp;
    Point3d cameraTarget;
    Mtx cameraMatrix;
    f32 angle;
    f32 angleFactor;
    f32 radius;
    f32 adjustedAmount;
    f32 halfAngle;
    f32 absoluteAngle;
    f64 absoluteAngleDouble;
    f32 movementLength;

    cameraMovement.x = movement->x;
    cameraMovement.z = movement->z;
    fn_80027CD0(type, &cameraPosition, &cameraUp, &cameraTarget);
    cameraTarget.y = cameraPosition.y;
    C_MTXLookAt(cameraMatrix, &cameraPosition, &cameraUp, &cameraTarget);
    PSMTXMultVecSR(cameraMatrix, &cameraMovement, &cameraMovement);
    angle = atan2(cameraMovement.x, cameraMovement.z);
    halfAngle = bounds.fn_1_12FC4(angle) / 2.0f;
    absoluteAngle = __fabsf(halfAngle);
    absoluteAngleDouble = absoluteAngle;
    angleFactor = 1.0 - (absoluteAngleDouble / M_PI);
    movementLength = PSVECMag(&cameraMovement);
    radius = angleFactor * movementLength;
    if (amount < 0.1f) {
        adjustedAmount = 0.1f;
    } else if (amount > 1.0f) {
        adjustedAmount = 1.0f;
    } else {
        adjustedAmount = amount;
    }
    bounds.fn_1_13000(angle, adjustedAmount);
    horizontal = radius * sin(bounds.fn_1_13110());
    vertical = -radius * cos(bounds.fn_1_13110());
}

void VirtualPad::fn_1_12DA0(MGPLAYER *player) {
    float normalizedHorizontal;
    float horizontal;
    float normalizedVertical;
    float vertical;
    int padX;
    int padY;

    horizontal = this->horizontal / 10.0f;
    if (horizontal < -1.0f) {
        normalizedHorizontal = -1.0f;
    } else if (horizontal > 1.0f) {
        normalizedHorizontal = 1.0f;
    } else {
        normalizedHorizontal = horizontal;
    }
    padX = (int)(56.0f * normalizedHorizontal);

    vertical = this->vertical / 10.0f;
    if (vertical < -1.0f) {
        normalizedVertical = -1.0f;
    } else if (vertical > 1.0f) {
        normalizedVertical = 1.0f;
    } else {
        normalizedVertical = vertical;
    }
    padY = (int)(56.0f * normalizedVertical);

    fn_80080520(player, padX, padY, buttonsDown,
                   buttons | buttonsDown);
    fn_1_12F3C();
}


void VirtualPad::fn_1_12F3C() {
    horizontal *= 0.9;
    vertical *= 0.9;
    buttonsDown = 0;
    buttons = 0;
}

void Range::fn_1_12F88(float value) { current=fn_1_1370(value); }
float Range::fn_1_12FC4(float value) { return fn_1_1327C(current,value); }

void Range::fn_1_13000(float step, double rate)
{
    float progress = fn_1_1327C(current, step);
    current = fn_1_1370(current + rate * progress);
}

float Range::fn_1_1370(float value) {
    if (value < this->minimum) {
        return this->maximum - (f32) fmod((f64) (this->minimum - value), (f64) this->period);
    }
    if (value >= this->maximum) {
        return this->minimum + (f32) fmod((f64) (value - this->maximum), (f64) this->period);
    }
    return value;
}

float Range::fn_1_13110() { return current; }

static inline float rangeMinimum(float a, float b) { return a < b ? a : b; }

static inline float rangeMaximum(float a, float b) { return a > b ? a : b; }

Range::Range(float a, float b, float initial) {
    minimum = rangeMinimum(a, b);
    maximum = rangeMaximum(a, b);
    period = maximum - minimum;
    current = fn_1_1370(initial);
}

f32 Range::fn_1_1327C(f32 first, f32 second)
{
    f32 wrappedSecond;
    f32 wrappedFirst;
    f32 difference;
    f32 magnitude;
    f32 thresholdDistance;

    if (second < minimum) {
        wrappedSecond = maximum -
            (f32)fmod((f64)(minimum - second), (f64)period);
    } else if (second >= maximum) {
        wrappedSecond = minimum +
            (f32)fmod((f64)(second - maximum), (f64)period);
    } else {
        wrappedSecond = second;
    }

    if (first < minimum) {
        wrappedFirst = maximum -
            (f32)fmod((f64)(minimum - first), (f64)period);
    } else if (first >= maximum) {
        wrappedFirst = minimum +
            (f32)fmod((f64)(first - maximum), (f64)period);
    } else {
        wrappedFirst = first;
    }

    difference = wrappedSecond - wrappedFirst;
    if (difference < 0.0f) {
        thresholdDistance = -difference;
    } else {
        thresholdDistance = difference;
    }
    magnitude = thresholdDistance;
    return magnitude < period / 2.0f
        ? difference
        : (period - magnitude) *
          (f32)(difference < 0.0f ? 1 : -1);
}
