#ifndef MP6_C_CRITICAL_REGIONS_H
#define MP6_C_CRITICAL_REGIONS_H
/* Runtime critical-region operations. */
/* This single-threaded runtime does not enter or leave a locking primitive. */
/* Entering a region requires no operation in this runtime. */
#define __begin_critical_region(region)
/* Leaving a region has no locking state to release. */
#define __end_critical_region(region)
#endif
