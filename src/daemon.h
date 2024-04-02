#ifndef __DAEMON_H
#define __DAEMON_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include "errors.h"
#include "statistics.h"
#include "traffic_usage.h"
#include "utils.h"

#define DAEMON_PROCESS_NAME "traffic_usaged"
#define NOFITY_CMD "notify-send -u critical -a traffic-usage \"Error: %s\""
#define DAEMON_DELAY 10

void run_daemon(const char *statistics_file_path, Interface interface);
void kill_daemon(void);

// ------------------------------------------------------------------------------------------------

pid_t get_daemon_pid(void);
void daemon_error_handler(TUError error);

#endif
