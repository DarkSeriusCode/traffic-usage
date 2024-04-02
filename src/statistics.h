#ifndef __STATISTICS_H
#define __STATISTICS_H

#include <stdlib.h>
#include <errno.h>
#include "traffic_usage.h"
#include "errors.h"
#include "config.h"

#define FILE_FORMAT_VER_LEN 4

// Filters items in stats
#define trafficstats_filter(stats, cond)                   \
    TrafficStats _temp = trafficstats_new();               \
    for (size_t i = 0; i < (stats).entry_count; i++) {     \
        TrafficUsage _entry = *((stats).entries + i);      \
        if ((cond)) {                                      \
            trafficstats_add_entry_unsafe(&_temp, _entry); \
        }                                                  \
    }                                                      \
    trafficstats_copy(_temp, &(stats));                    \
    trafficstats_delete(&_temp);


// Statistics of net traffic usage
typedef struct {
    size_t entry_count;
    TrafficUsage *entries;
} TrafficStats;

// Function type used in `trafficstats_reduce` func
typedef void (*StatsReduceFn)(TrafficUsage *acc, TrafficUsage tu);

// Returns empty TrafficStats
TrafficStats trafficstats_new(void);
// Free memory used by `trafficstats`
void trafficstats_delete(TrafficStats *trafficstats);
// Copy TrafficStats
void trafficstats_copy(TrafficStats src, TrafficStats *dest);

// Adds new entry without any chekcs. See also `trafficstats_add_entry`
void trafficstats_add_entry_unsafe(TrafficStats *trafficstats, TrafficUsage tu);
// Adds new entry with chekcs (next: `last` - the last entry in `trafficstats`):
// * If `tu` made in the same day that `last`, then tu overwrites `last`
// * If day difference between `tu` and `last` equal or more than 1 and
//   tx or rx in `tu` are greater then tx or rx in `last`, then adds the difference between
//   `tu` and `last`
void trafficstats_add_entry(TrafficStats *trafficstats, TrafficUsage tu);

// Writes `trafficstats` into a file
TUError trafficstats_write(TrafficStats trafficstats, const char *fname);
// Reads TrafficStats from a file
TUError trafficstats_read(TrafficStats *stats, const char *fname);

// Reduces entries in `stats` using `reduce_func`
TrafficUsage trafficstats_reduce(TrafficStats stats, StatsReduceFn reduce_func);

#endif
