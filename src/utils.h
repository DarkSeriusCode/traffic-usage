#ifndef __UTILS_H
#define __UTILS_H

#include <unistd.h>
#include "errors.h"

// Checks the existence of a file and the permissions to access it
TUError check_file(const char *fpath);
// Counts digits in a number
size_t count_digits(int num);

#endif
