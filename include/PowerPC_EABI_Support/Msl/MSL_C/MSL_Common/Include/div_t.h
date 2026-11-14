#ifndef MP6_C_DIV_T_H
#define MP6_C_DIV_T_H
/* Division results retain both the quotient and remainder. */
typedef struct {
    int quot;
    int rem;
} div_t;
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
