#ifndef __DAEMON_H
#define __DAEMON_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include "errors.h"
#include "statistics.h"
#include "traffic_usage.h"
#include "utils.h"
#include "config.h"


void run_daemon(const char *statistics_file_path, Interface interface);
void kill_daemon(void);

// ------------------------------------------------------------------------------------------------

pid_t get_daemon_pid(void);
void daemon_error_handler(TUError error);

#endif
