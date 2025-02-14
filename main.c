#include <stdio.h>
#include <argp.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "src/main.h"
#include "src/cli.h"
#include "src/daemon.h"
#include "src/statistics.h"
#include "src/errors.h"
#include "src/traffic_usage.h"

const char *statistics_file_path = NULL;
Interface interface;
Action action = NONE;

struct argp_option options[] = {
    { "file", 'f', "FILE_PATH", 0, "Sets statistics file path." },
    { "tui", 't', 0, 0, "Shows statistics in TUI." },
    { "daemon", 'd', "INTERFACE", 0,
        "Starts daemon on the background that collects traffic usage stats." },
    { "kill", 'k', 0, 0, "Kills a previously started daemon." },
    { "list", 'l', 0, 0, "Prints a list of all available interfaces." },
    { 0 }
};

int parse_opt(int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'f':
            if (arg) {
                statistics_file_path = arg;
            }
            break;
        case 'd':
            if (arg) {
                default_error_handler(check_interface_existence(arg));
                default_error_handler(interface_init(&interface, arg));
            }
            action = START_DAEMON;
            break;
        case 't': action = ENTER_TUI_MODE; break;
        case 'k': action = KILL_DAEMON; break;
        case 'l': action = PRINT_INTERFACE_LIST; break;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    char buff[64];
    if (!statistics_file_path) {
        struct passwd *pw = getpwuid(getuid());
        sprintf(buff, "%s/.traffic_usage_stats", pw->pw_dir);
        statistics_file_path = buff;
    }
    struct argp argp = { options, parse_opt };

    int exit_code = argp_parse(&argp, argc, argv, 0, 0, 0);
    if (exit_code) { return exit_code; }

    TrafficStats stats;
    TUError error;
    switch (action) {
        case NONE:
            // Unreachable
            break;
        case ENTER_TUI_MODE:
            error = trafficstats_read(&stats, statistics_file_path);
            if (has_error(error)) {
                exit_with_error(error);
            }
            enter_tui_mode(stats);
            break;
        case START_DAEMON:
            run_daemon(statistics_file_path, interface);
            break;
        case KILL_DAEMON:
            kill_daemon();
            break;
        case PRINT_INTERFACE_LIST:
            print_interface_list();
            break;
    }
    return 0;
}

// ------------------------------------------------------------------------------------------------

void default_error_handler(TUError error) {
    if (!has_error(error)) {
        return;
    }
    exit_with_error(error);
}

void print_interface_list() {
    DIR *dir = opendir(INTERFACE_PATH);
    struct dirent *entry;
    if (!dir) {
        printf("%s does not exist!\n", INTERFACE_PATH);
    }
    while ( (entry = readdir(dir)) != NULL ) {
        if (entry->d_name[0] == '.') {
            continue;
        }
        printf("* %s\n", entry->d_name);
    }
    closedir(dir);
}
