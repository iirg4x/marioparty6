/* Route points and direction vectors used by Boo'd Off the Stage. */
#ifndef REL_M660_ROUTES_H
#define REL_M660_ROUTES_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"

typedef struct M660RoutePath {
    s16 direction;
    s16 count;
    Vec directions[2];
    Vec points[2];
} M660RoutePath;

typedef struct M660Route {
    s32 unobserved00;
    M660RoutePath paths[4];
} M660Route;

extern M660Route lbl_1_bss_36B8[14];
#endif
