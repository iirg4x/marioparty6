/* Cosine-eased interpolation between two Mole-it! movement positions. */
#include "math.h"
#include "game/main.h"

/* Returns the cosine-eased point between start and end; no function in this module calls it. */
void fn_1_10BC(Point3d *result, const Point3d *start, const Point3d *end, s16 stepCount,
               s16 stepIndex) {
    Point3d position;
    f32 interpolation;

    interpolation = (f32)cos((3.141592653589793 *
                              (f64)((180.0f / (f32)stepCount) *
                                    (f32)stepIndex)) /
                             180.0);
    interpolation = 1.0f - (0.5f * (1.0f + interpolation));
    position.x = end->x - start->x;
    position.y = end->y - start->y;
    position.z = end->z - start->z;
    position.x *= interpolation;
    position.y *= interpolation;
    position.z *= interpolation;
    position.x += start->x;
    position.y += start->y;
    position.z += start->z;
    *result = position;
}
