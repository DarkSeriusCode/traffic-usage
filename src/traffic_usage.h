#ifndef __TRAFFIC_USAGE_H
#define __TRAFFIC_USAGE_H

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>
#include <assert.h>
#include <stdio.h>
#include "utils.h"
#include "errors.h"

#define INTERFACE_PATH "/sys/class/net/"
#define NET_STAT_PATH_FORMAT INTERFACE_PATH "%s/statistics/%s"
#define NET_STAT_PATH_BUF_SIZE 64
#define INTERFACE_NAME_LEN 16

// ------------------------------------------------------------------------------------------------

// Represents the information size.
typedef struct {
    size_t GiB;
    size_t MiB;
    size_t KiB;
    size_t bytes;
} DataSize;

// Creates new empty DataSize
DataSize datasize_new(void);
// Normalizes fields of DataSize. For example: converts 1030bytes to 1KiB and 6bytes
void datasize_normalize(DataSize *data_size);
// Adds two DataSize objects
DataSize datasize_add(DataSize ds1, DataSize ds2);
// Difference between ds1 and ds2
DataSize datasize_diff(DataSize ds1, DataSize ds2);
// Compares two DataSizes, returns
// 0 if they're equal,
// 1 if ds1 > ds2,
// -1 if ds1 < ds2
short datasize_cmp(DataSize ds1, DataSize ds2);
// Makes DataSize from bytes
DataSize datasize_from_bytes(size_t bytes);
// Convert DataSize to bytes
size_t datasize_to_bytes(DataSize ds);
// Returns len of the DataSize string representation
size_t datasize_string_len(DataSize ds);

// ------------------------------------------------------------------------------------------------

// Represents net interface
typedef struct {
    char name[INTERFACE_NAME_LEN];
    char tx_file_path[NET_STAT_PATH_BUF_SIZE];
    char rx_file_path[NET_STAT_PATH_BUF_SIZE];
} Interface;

// Represents spent net traffic using some interface for some day
typedef struct {
    DataSize tx;
    DataSize rx;
    char interface_name[INTERFACE_NAME_LEN];
    struct tm date;
} TrafficUsage;

// Returns empty Interface
Interface interface_new(void);
// Initializates Interface by name
TUError interface_init(Interface *interface, const char *name);
// Returns information about traffic usage using `interface`
TrafficUsage interface_get_traffic_usage(Interface interface);
// Checks interface existence
TUError check_interface_existence(const char *interface_name);

// Returns empty TrafficUsage for `interface`
TrafficUsage tu_new(Interface interface);

#endif
