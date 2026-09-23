#include "dolphin/mtx/GeoTypes.h"

double sin(double);
double cos(double);
extern const float lbl_1_rodata_14;
extern const float lbl_1_rodata_18;

void fn_1_150C(Mtx out, double angle)
{
    float sine = (float)sin(angle);
    float cosine = (float)cos(angle);

    out[0][0] = cosine;
    out[0][1] = lbl_1_rodata_18;
    out[0][2] = -sine;
    out[1][0] = lbl_1_rodata_18;
    out[1][1] = lbl_1_rodata_14;
    out[1][2] = lbl_1_rodata_18;
    out[2][0] = sine;
    out[2][1] = lbl_1_rodata_18;
    out[2][2] = cosine;
}

void fn_1_15E4(Mtx dst, Mtx src, float scale)
{
    int column;
    int row;
    for (row = 0; row < 3; row++) {
        for (column = 0; column < 3; column++) {
            dst[row][column] = scale * src[row][column];
        }
    }
}
