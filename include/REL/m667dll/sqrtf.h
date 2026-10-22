/* Entry-throw curve setup calls this inline helper for horizontal distances.
 * It returns an approximate square root for positive inputs and passes nonpositive inputs through.
 */
extern inline f32 fn_1_8E8C(f32 squaredDistance)
{
    const double halfScale = 0.5;
    const double threeTerm = 3.0;
    volatile f32 approximation;
    if (squaredDistance > 0.0f) {
        f64 reciprocalSqrtEstimate = __frsqrte((f64)squaredDistance);
        reciprocalSqrtEstimate =
            halfScale * reciprocalSqrtEstimate *
            (threeTerm - reciprocalSqrtEstimate * reciprocalSqrtEstimate * squaredDistance);
        reciprocalSqrtEstimate =
            halfScale * reciprocalSqrtEstimate *
            (threeTerm - reciprocalSqrtEstimate * reciprocalSqrtEstimate * squaredDistance);
        reciprocalSqrtEstimate =
            halfScale * reciprocalSqrtEstimate *
            (threeTerm - reciprocalSqrtEstimate * reciprocalSqrtEstimate * squaredDistance);
        approximation = (f32)(squaredDistance * reciprocalSqrtEstimate);
        return approximation;
    }
    return squaredDistance;
}
