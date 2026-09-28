#define _MATH_H
#include "game/main.h"

/* Target rodata is an authentic 0x48-byte table. This consumed 12x3 s16 view
 * defines no storage; the original declaration and name are unknown. */
typedef struct {
    s16 rows[12][3];
} MDMinNeighborTable;

extern const MDMinNeighborTable lbl_1_rodata_1CC;
struct _struct_lbl_1_data_89A_0x12;
extern s16 fn_1_9348(s16, s16, s16, s16,
                     struct _struct_lbl_1_data_89A_0x12 (*)[], s16);

s16 fn_1_9730(s16 arg0, struct _struct_lbl_1_data_89A_0x12 (*arg1)[],
              s16 arg2, s16 arg3)
{
    MDMinNeighborTable neighbors = lbl_1_rodata_1CC;
    return fn_1_9348(arg0, neighbors.rows[arg2][0],
                     neighbors.rows[arg2][1], neighbors.rows[arg2][2],
                     arg1, arg3);
}
