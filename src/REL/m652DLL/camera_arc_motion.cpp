#include "REL/m652DLL/range_operations.h"
#include "REL/m652DLL/tail_types.h"
#include "REL/m652DLL/camera_arc_motion.h"
extern "C" {
#include "math.h"
}
#include "humath.h"

extern "C" void fn_80027CD0(s32 cameraBit, Vec *position, Vec *up, Vec *target);
extern "C" void fn_80080520(void *player, s32 padX, s32 padY,
                                s32 buttonState, s32 buttons);

CameraArcMotion::CameraArcMotion(u16 cameraBit, f32 angle)
    : cameraBit((u16)cameraBit), angle(-3.1415927f, 3.1415927f, angle),
      horizontalMotion(0.0f), forwardMotion(0.0f), flags_primary(0), flags_secondary(0) {}

CameraArcMotion::~CameraArcMotion() {}

void fn_1_ABA8(CameraArcMotion *control, f32 value)
{
    control->angle.SetCurrent(value);
    control->horizontalMotion = 0.0f;
    control->forwardMotion = 0.0f;
    control->flags_primary = 0;
    control->flags_secondary = 0;
}

void CameraArcMotion::AddSecondaryFlags(u32 flags) {
    flags_secondary |= flags;
}

void CameraArcMotion::AddPrimaryFlags(u32 flags) {
    flags_primary |= flags;
}

void CameraArcMotion::SetPrimaryFlag200() {
    flags_primary |= 0x200;
}

void CameraArcMotion::fn_1_AC40() {
    flags_primary |= 0x100;
}

void CameraArcMotion::fn_1_AC50() {
    flags_secondary |= 0x100;
}

void CameraArcMotion::fn_1_AC60(const Vec *movement, f32 amount)
{
    Point3d cameraMovement = {0.0f, 0.0f, 0.0f};
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
    fn_80027CD0(cameraBit, &cameraPosition, &cameraUp, &cameraTarget);
    cameraTarget.y = cameraPosition.y;
    C_MTXLookAt(cameraMatrix, &cameraPosition, &cameraUp, &cameraTarget);
    PSMTXMultVecSR(cameraMatrix, &cameraMovement, &cameraMovement);
    angle = atan2(cameraMovement.x, cameraMovement.z);
    halfAngle = fn_1_75EC(&this->angle, angle) / 2.0f;
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
    fn_1_7628(&this->angle, angle, adjustedAmount);
    horizontalMotion = radius * sin(this->angle.GetCurrent());
    forwardMotion = -radius * cos(this->angle.GetCurrent());
}

void CameraArcMotion::fn_1_AE90(void *player)
{
    f32 normalizedHorizontal;
    f32 horizontal;
    f32 normalizedVertical;
    f32 vertical;
    s32 padX;
    s32 padY;

    horizontal = horizontalMotion / 10.0f;
    if (horizontal < -1.0f) {
        normalizedHorizontal = -1.0f;
    } else if (horizontal > 1.0f) {
        normalizedHorizontal = 1.0f;
    } else {
        normalizedHorizontal = horizontal;
    }
    padX = (s32)(56.0f * normalizedHorizontal);

    vertical = forwardMotion / 10.0f;
    if (vertical < -1.0f) {
        normalizedVertical = -1.0f;
    } else if (vertical > 1.0f) {
        normalizedVertical = 1.0f;
    } else {
        normalizedVertical = vertical;
    }
    padY = (s32)(56.0f * normalizedVertical);

    fn_80080520(player, padX, padY,
                   flags_primary, flags_secondary | flags_primary);
    fn_1_B02C();
}

void CameraArcMotion::fn_1_B02C() {
    horizontalMotion *= 0.9;
    forwardMotion *= 0.9;
    flags_primary = 0;
    flags_secondary = 0;
}

template<class T> void RangeT<T>::SetCurrent(T value) { current = fn_1_1370(value); }

template<class T> T RangeT<T>::GetCurrent() { return current; }

template<> inline float fn_1_75EC<float>(Range *self, float value)
{
    return self->fn_1_7CE0(self->current, value);
}
template<> inline float fn_1_7628<float>(Range *range, float step, double rate)
{
    float progress = range->fn_1_7CE0(range->current, step);
    return range->current = range->fn_1_1370(range->current + rate * progress);
}

template<class T> T RangeT<T>::fn_1_7CE0(T first, T second) const
{
    T wrappedSecond;
    T wrappedFirst;
    T difference;
    T magnitude;

    if (second < this->minimum) {
        wrappedSecond = this->maximum -
            (T)std::fmod((double)(this->minimum - second), (double)this->period);
    } else if (second >= this->maximum) {
        wrappedSecond = this->minimum +
            (T)std::fmod((double)(second - this->maximum), (double)this->period);
    } else {
        wrappedSecond = second;
    }

    if (first < this->minimum) {
        wrappedFirst = this->maximum -
            (T)std::fmod((double)(this->minimum - first), (double)this->period);
    } else if (first >= this->maximum) {
        wrappedFirst = this->minimum +
            (T)std::fmod((double)(first - this->maximum), (double)this->period);
    } else {
        wrappedFirst = first;
    }

    difference = wrappedSecond - wrappedFirst;
    magnitude = Abs7CE0(difference);
    return magnitude < this->period / 2.0f
        ? difference
        : (this->period - magnitude) *
          (T)(difference < lbl_1_rodata_330[0] ? 1 : -1);
}
