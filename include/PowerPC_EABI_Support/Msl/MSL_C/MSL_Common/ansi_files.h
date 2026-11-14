#ifndef MP6_PROJECT_ANSI_FILES_H
#define MP6_PROJECT_ANSI_FILES_H
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/Include/file_struc.h"
/* Console callbacks and stream-list operations use the shared FILE record. */
#ifdef __cplusplus
extern "C" {
#endif
extern int __close_console(__file_handle file);
extern int __write_console(__file_handle file, unsigned char* buf, size_t* count,
    __idle_proc idle_fn);
extern int __read_console(__file_handle file, unsigned char* buf, size_t* count,
    __idle_proc idle_fn);

unsigned int __flush_all(void);
void __close_all(void);

#ifdef __cplusplus
}
#endif
#endif
