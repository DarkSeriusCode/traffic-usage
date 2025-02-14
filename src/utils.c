#include "utils.h"

TUError check_file(const char *fpath) {
    if (access(fpath, F_OK) != 0) {
        return make_error(TU_FILE_DOESNT_EXIST, fpath);
    } else if (access(fpath, R_OK) != 0) {
        return make_error(TU_NO_READ_PERMISSION, fpath);
    }
    return make_ok();
}

size_t count_digits(int num) {
    int count = 0;
    if(num < 0) {
        num *= -1;
    }
    while(num != 0) {
        num = num / 10;
        count++;
    }
    return count;
}
