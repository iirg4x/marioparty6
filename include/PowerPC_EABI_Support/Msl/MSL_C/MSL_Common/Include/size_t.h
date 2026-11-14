#ifndef MP6_C_SIZE_T_H
#define MP6_C_SIZE_T_H
/* Object extents use the unsigned 32-bit EABI size type. */
typedef unsigned long size_t;
#ifdef __cplusplus
namespace std { using ::size_t; }
#endif
#endif
