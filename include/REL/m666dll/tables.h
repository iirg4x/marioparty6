/* Motion resources and CPU steering-choice tables used by Shoot Yer Mouth Off. */
#ifndef REL_M666DLL_TABLES_H
#define REL_M666DLL_TABLES_H

typedef struct M666MotionData {
    u32 fileId;
    u32 flags;
} M666MotionData;

extern M666MotionData lbl_1_data_680[6];

typedef struct M666FourChoiceTable {
    u16 values[4];
} M666FourChoiceTable;

/* Four avoidance angles in degrees, one for each steering direction. */
extern const M666FourChoiceTable lbl_1_rodata_148;
/* Per-difficulty CPU steering probabilities, in percent. */
extern const M666FourChoiceTable lbl_1_rodata_158;

#endif
