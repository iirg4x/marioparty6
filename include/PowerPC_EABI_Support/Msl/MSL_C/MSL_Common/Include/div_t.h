#ifndef MP6_C_DIV_T_H
#define MP6_C_DIV_T_H
/* Division results retain both the quotient and remainder. */
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/arith.h"
typedef struct {
    long quot;
    long rem;
} ldiv_t;
#ifdef __cplusplus
namespace std {
    using ::div_t;
    using ::ldiv_t;
}
#endif
#endif
