#include "daemon.h"
#include "statistics.h"

void run_daemon(const char *statistics_file_path, Interface interface) {
    if (get_daemon_pid() != -1) {
        printf("traffic_usage daemon is already running!\n");
        return;
    }
    if (daemon(0, 0)) {
        perror("Daemon");
        return;
    }

    if (prctl(PR_SET_NAME, DAEMON_PROCESS_NAME, 0, 0, 0) == -1) {
        perror("Error setting daemon process name");
        return;
    }

    TUError error;
    TrafficStats stats = trafficstats_new();
    if (check_file(statistics_file_path).code == TU_FILE_DOESNT_EXIST) {
        error = trafficstats_write(stats, statistics_file_path);
    } else {
        error = trafficstats_read(&stats, statistics_file_path);
    }
    daemon_error_handler(error);

    TrafficUsage tu;
    TrafficUsage prev_tu = tu_new(interface_new());
    bool file_is_empty = true;
    if (stats.entry_count > 0) {
        prev_tu = stats.entries[stats.entry_count - 1];
        file_is_empty = false;
    }

    do {
        tu = interface_get_traffic_usage(interface);

        TrafficUsage diff = tu;
        if (!file_is_empty) {
            diff.tx = datasize_diff(tu.tx, prev_tu.tx);
            diff.rx = datasize_diff(tu.rx, prev_tu.rx);
        }

        trafficstats_add_entry(&stats, diff);
        prev_tu = tu;
        trafficstats_write(stats, statistics_file_path);
        file_is_empty = false;

        sleep(DAEMON_DELAY);
    } while (1);
}

void kill_daemon() {
    pid_t daemon_pid = get_daemon_pid();

    if (daemon_pid == -1) {
        printf("Daemon is not running!\n");
        return;
    }
    kill(daemon_pid, SIGKILL);
    printf("Daemon with PID %d has been successfuly killed!\n", daemon_pid);
}

// ------------------------------------------------------------------------------------------------

pid_t get_daemon_pid() {
    char buffer[8];
    char command[strlen("pgrep ") + strlen(DAEMON_PROCESS_NAME) + 1];
    pid_t pid = -1;

    sprintf(command, "pgrep %s", DAEMON_PROCESS_NAME);
    FILE *fp = popen(command, "r");

    if (fp == NULL) {
        printf("Error when getting daemon's PID");
        return -1;
    }
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        pid = atoi(buffer);
    }
    pclose(fp);
    return pid;
}

void daemon_error_handler(TUError error) {
    if (!has_error(error)) {
        return;
    }
    char buff[strlen(NOTIFY_CMD) + strlen(error.err_msg)];
    sprintf(buff, NOTIFY_CMD, error.err_msg);
    system(buff);
    exit(EXIT_FAILURE);
}
