#include "errors.h"

TUError make_ok() {
    return (TUError){ TU_OK, "" };
}

TUError make_error(TUErrorCode code, ...) {
    TUError error;
    error.code = code;

    const char *fmt = NULL;
    char buff[DEFAULT_ERR_BUF_SIZE];

    va_list args;
    va_start(args, code);

    switch (code) {
        case TU_OK:
            fmt = "Everything is good";
            break;
        case TU_FILE_DOESNT_EXIST:
            fmt = "File \"%s\" doesn't exist";
            break;
        case TU_DIR_DOESNT_EXIST:
            fmt = "Directory \"%s\" doesn't exist";
            break;
        case TU_NO_READ_PERMISSION:
            fmt = "File \"%s\" cannot be opened for reading";
            break;
        case TU_INTERFACE_DOESNT_EXIST:
            fmt = "Interface \"%s\" doesn't exist";
            break;
        case TU_FROM_ERRNO:
            vsprintf(buff, "When %s: ", args);
            strcat(buff, strerror(errno));
            break;
        default:
            fmt = "Unknown error";
    }
    vsprintf(error.err_msg, (fmt) ? fmt : buff, args);
    va_end(args);
    return error;
}

bool has_error(TUError err) {
    if (err.code != 0)
        return true;
    return false;
}

void exit_with_error(TUError err) {
    fprintf(stderr, "%s\n", err.err_msg);
    exit(err.code);
}
