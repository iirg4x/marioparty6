#ifndef MP6_PROJECT_ARITH_H
#define MP6_PROJECT_ARITH_H
typedef struct { int quot; int rem; } div_t;
#ifdef __cplusplus
extern "C" {
#endif
div_t div(int, int);
#ifdef __cplusplus
}
#endif
#endif
