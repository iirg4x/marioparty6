#ifndef MP6_C_CRITICAL_REGIONS_H
#define MP6_C_CRITICAL_REGIONS_H
/* Runtime critical-region operations. */
/* This single-threaded runtime does not enter or leave a locking primitive. */
#define __begin_critical_region(region)
#define __end_critical_region(region)
#endif
