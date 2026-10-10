#ifndef FRAMEWORK_ASSERTION_H
#define FRAMEWORK_ASSERTION_H

// The C runtime reports a failed assertion with its expression and source location.
void __msl_assertion_failed(const char *expression, const char *file, int line);

#endif
