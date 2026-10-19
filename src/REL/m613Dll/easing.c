/* Interpolates a position between two endpoints with a cosine ease. */
#include "dolphin/math.h"
#include "game/main.h"

/* Writes a cosine-eased position using elapsedFrames divided by durationFrames. */
void fn_1_EE98(Point3d *result, Point3d *start, Point3d *end, s16 durationFrames,
               s16 elapsedFrames) {
    Point3d position;
    f32 amount;

    amount = (f32)cos((3.141592653589793 *
                       (f64)((180.0f / (f32)durationFrames) * (f32)elapsedFrames)) /
                      180.0);
    amount = 1.0f - (0.5f * (1.0f + amount));

    position.x = end->x - start->x;
    position.y = end->y - start->y;
    position.z = end->z - start->z;
    position.x *= amount;
    position.y *= amount;
    position.z *= amount;
    position.x += start->x;
    position.y += start->y;
    position.z += start->z;
    *result = position;
}
