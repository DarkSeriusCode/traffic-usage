#ifndef __ERRORS_H
#define __ERRORS_H

#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define DEFAULT_ERR_BUF_SIZE 128

// All available error codes. Use `make_error` to format error description.
typedef enum {
    TU_OK,
    // File "%s" doesn't exist
    TU_FILE_DOESNT_EXIST,
    // Directory "%s" doesn't exist
    TU_DIR_DOESNT_EXIST,
    // File "%s" cannot be opened for reading
    TU_NO_READ_PERMISSION,
    // Interface "%s" doesn't exist
    TU_INTERFACE_DOESNT_EXIST,
    // When %s: <errno error string>
    TU_FROM_ERRNO,
} TUErrorCode;

// Error type. Contains error code and its description.
typedef struct {
    TUErrorCode code;
    char err_msg[DEFAULT_ERR_BUF_SIZE];
} TUError;

// Makes TUError with OK code
TUError make_ok(void);
// Makes TUError with given error code and formats error description.
TUError make_error(TUErrorCode code, ...);

// Return true if err.code != OK, otherwise false
bool has_error(TUError err);

// Prints error description to stderr and exit the program
void exit_with_error(TUError err);

#endif
