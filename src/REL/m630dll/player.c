#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/pad.h"
#include "game/charman.h"
#include "game/frand.h"
#include "math.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"
#include "REL/m630/game.h"

void fn_1_7C34(Mtx matrix, int cameraId);
int fn_1_7EF4(HuVecF *src, HuVecF *dst);

void fn_1_78A4(M630Player *player)
{
    float angle;
    float magnitude;
    float magnitudeSquared;
    Mtx matrix;
    HuVecF move;
    HuVecF direction;

    move.x = HuPadStkX[player->padNo] / 5.6f;
    move.y = 0.0f;
    move.z = HuPadStkY[player->padNo] / 5.6f;
    fn_1_7C34(matrix, 1);
    PSMTXMultVec(matrix, &move, &move);
    player->pos.x += move.x;
    player->pos.y += move.y;
    player->pos.z += move.z;
    magnitudeSquared = move.z * move.z + (move.x * move.x + move.y * move.y);
    if (magnitudeSquared >= 0.001) {
        fn_1_7EF4(&move, &direction);
        if (direction.z > 1.0f) {
            direction.z = 1.0f;
        } else if (direction.z < -1.0f) {
            direction.z = -1.0f;
        }
        angle = acosf(direction.z);
        if (direction.x < 0.0f) {
            angle = -angle;
        }
        player->rotationY = (180.0f * angle) / 3.141592653589793;
    }
    magnitude = move.x * move.x + move.z * move.z;
    if (magnitude > 25.0f) {
        if (player->currentMotion != 2) {
            CharMotionShiftSet(player->charNo, player->motionId[2], 0.0f, 5.0f, 1073741825U);
            player->currentMotion = 2;
        }
    } else if (magnitude > 0.001) {
        if (player->currentMotion != 1) {
            CharMotionShiftSet(player->charNo, player->motionId[1], 0.0f, 5.0f, 1073741825U);
            player->currentMotion = 1;
        }
    } else if (player->currentMotion != 0) {
        CharMotionShiftSet(player->charNo, player->motionId[0], 0.0f, 5.0f, 1073741825U);
        player->currentMotion = 0;
    }
}

void fn_1_7C34(Mtx matrix, int cameraId)
{
    HuVecF pos;
    HuVecF target;
    HuVecF up;
    HuVecF x;
    HuVecF y;
    HuVecF z;
    HuVecF horizontal;
    HuVecF direction;
    HuVecF cameraPos;
    HuVecF cameraTarget;
    HuVecF cameraUp;

    Hu3DCameraPosGet(cameraId, &cameraPos, &cameraUp, &cameraTarget);
    pos.x = cameraPos.x;
    pos.y = cameraPos.y;
    pos.z = cameraPos.z;
    target.x = cameraTarget.x;
    target.y = cameraTarget.y;
    target.z = cameraTarget.z;
    up.x = cameraUp.x;
    up.y = cameraUp.y;
    up.z = cameraUp.z;
    x.x = 1.0f;
    x.y = 0.0f;
    x.z = 0.0f;
    y.x = 0.0f;
    y.y = 1.0f;
    y.z = 0.0f;
    z.x = 0.0f;
    z.y = 0.0f;
    z.z = 1.0f;
    PSVECSubtract(&target, &pos, &direction);
    horizontal = direction;
    horizontal.y = 0.0f;
    if (PSVECSquareMag(&horizontal) < 0.001) {
        horizontal = direction;
        direction = up;
        PSVECScale(&horizontal, &up, -1.0f);
    }
    direction.y = 0.0f;
    fn_1_7EF4(&direction, &direction);
    if (PSVECDotProduct(&y, &up) < 0.0f) {
        y.y *= -1.0f;
        z.z *= -1.0f;
    }
    PSVECCrossProduct(&direction, &y, &x);
    z = direction;
    matrix[0][0] = x.x;
    matrix[0][1] = x.y;
    matrix[0][2] = x.z;
    matrix[0][3] = 0.0f;
    matrix[1][0] = y.x;
    matrix[1][1] = y.y;
    matrix[1][2] = y.z;
    matrix[1][3] = 0.0f;
    matrix[2][0] = z.x;
    matrix[2][1] = z.y;
    matrix[2][2] = z.z;
    matrix[2][3] = 0.0f;
}

int fn_1_7EF4(HuVecF *src, HuVecF *dst)
{
    int result;
    float y;

    if (PSVECSquareMag(src) < 0.000001) {
        dst->x = 0.01f * ((float)(u32)frandmod(20) - 10.0f);
        dst->z = 0.01f * ((float)(u32)frandmod(20) - 10.0f);
        if ((u32)frandmod(1) != 0U) {
            y = 0.01f;
        } else {
            y = -0.01f;
        }
        dst->y = y;
        PSVECNormalize(dst, dst);
        result = 0;
    } else {
        PSVECNormalize(src, dst);
        result = 1;
    }
    return result;
}
