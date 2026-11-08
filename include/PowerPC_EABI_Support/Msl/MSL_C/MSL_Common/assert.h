/* Standard assertion interface for the MSL C runtime. */
#ifndef MSL_ASSERT_H_
#define MSL_ASSERT_H_

#ifdef __cplusplus
extern "C" {
#endif
void __msl_assertion_failed(const char *expression, const char *file, int line);
#ifdef __cplusplus
}
#endif

#ifdef NDEBUG
#define assert(expression) ((void)0)
#else
#define assert(expression) \
    ((expression) ? (void)0 : __msl_assertion_failed(#expression, __FILE__, __LINE__))
#endif

#endif
