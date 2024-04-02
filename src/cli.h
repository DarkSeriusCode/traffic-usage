#ifndef __CLI_H
#define __CLI_H

#include <ncurses.h>
#include <time.h>
#include <stdbool.h>
#include "statistics.h"
#include "traffic_usage.h"

#define TABLE_HEADER "Day|Interface|%*s|%*s"
#define TABLE_HEADER_HEIGHT 2
#define TABLE_ENTRY_FORMAT "%3d|%*s|%*s|%*s"
#define TABLE_TX_RX_PADDING 2
#define TABLE_ENTRY_HEIGHT 2
#define TABLE_EMPTY_MSG "There's no statistics for this month ;-;"

#define USAGE_FOR_ALL_TIME_TITLE "Total usage (for all time)"
#define USAGE_FOR_MONTH_TITLE "Total usage (for current month)"

#define max(a, b) ((a > b) ? a : b)
#define min(a, b) ((a < b) ? a : b)

void date_inc_by_month(struct tm *date);
void date_dec_by_month(struct tm *date);

// ------------------------------------------------------------------------------------------------

typedef struct {
    size_t width, height;
} Geometry;

typedef struct {
    int x, y;
} Position;

// ------------------------------------------------------------------------------------------------

typedef struct {
    Geometry geometry;
    Position position;
    struct tm current_date;
    const struct tm UPPER_BOUND;
    const struct tm LOWER_BOUND;
} Header;

Header header_new(struct tm lower_bound, struct tm upper_bound);
void header_next_month(Header *header);
void header_previous_month(Header *header);
void header_draw(Header header);

// ------------------------------------------------------------------------------------------------

typedef struct {
    int day, interface, tx, rx;
} TableFieldSizes;

typedef struct {
    TrafficStats stats;
    int entries_offset;
    const size_t MAX_VISIBLE_ENTRIES;
    size_t visible_entries;
    Geometry geometry;
    Position position;
    TableFieldSizes field_sizes;
    WINDOW *wndw;
} TrafficUsageTable;

TrafficUsageTable trafftable_new(size_t max_height);
void trafftable_delete(TrafficUsageTable *table);
void trafftable_set_position(TrafficUsageTable *table, Position pos);
void trafftable_set_stats(TrafficUsageTable *table, TrafficStats stats);
void trafftable_draw(TrafficUsageTable table);
void trafftable_scrl(TrafficUsageTable *table, int scrl_value);

TableFieldSizes tablefieldsizes_calc_max(TrafficUsageTable table);

// ------------------------------------------------------------------------------------------------

typedef struct {
    TrafficUsage traffusage_for_all_time;
    TrafficUsage traffusage_for_month;
    Geometry geometry;
    Position position;
    WINDOW *wndw;
} TotalUsage;

TotalUsage totalusage_new(TrafficStats stats);
void totalusage_delete(TotalUsage *total_usage);
void totalusage_set_position(TotalUsage *total_usage, Position pos);
void totalusage_calc_for_current_month(TotalUsage *total_usage, TrafficStats stats_for_month);
void totalusage_draw(TotalUsage total_usage);

// ------------------------------------------------------------------------------------------------

void enter_tui_mode(TrafficStats stats);
void print_footer(TrafficStats stats);
void filter_trafficusage_by_date(TrafficStats *dest, TrafficStats src, struct tm date);

void print_empty_file_msg();

void format_datasize(char *buff, size_t len, DataSize ds);

// ------------------------------------------------------------------------------------------------

void sum(TrafficUsage *acc, TrafficUsage tu);
void wprint_line(WINDOW *wndw, char ch, int len);
const char *month_to_string(int month);

#endif
