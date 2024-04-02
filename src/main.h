#ifndef __MAIN_H
#define __MAIN_H

#include "errors.h"

typedef enum {
    NONE,
    ENTER_TUI_MODE,
    START_DAEMON,
    KILL_DAEMON,
    PRINT_INTERFACE_LIST,
} Action;

void default_error_handler(TUError error);
void print_interface_list(void);

#endif
