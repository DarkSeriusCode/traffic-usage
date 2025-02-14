#ifndef __UTILS_H
#define __UTILS_H

#include <unistd.h>
#include "errors.h"

#define max(a, b) ((a > b) ? a : b)
#define min(a, b) ((a < b) ? a : b)

// Checks the existence of a file and the permissions to access it
TUError check_file(const char *fpath);
// Counts digits in a number
size_t count_digits(int num);

#endif
