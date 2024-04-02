#include "cli.h"

void date_inc_by_month(struct tm *date) {
    date->tm_mon++;
    if (date->tm_mon > 11) {
        date->tm_mon = 0;
        date->tm_year++;
    }
}

void date_dec_by_month(struct tm *date) {
    date->tm_mon--;
    if (date->tm_mon < 0) {
        date->tm_mon = 11;
        date->tm_year--;
    }
}

// ------------------------------------------------------------------------------------------------

Header header_new(struct tm lower_bound, struct tm upper_bound) {
    Header header = {
        .geometry = { getmaxx(stdscr), 3 },
        .position = { 0, 0 },
        .current_date = lower_bound,
        .UPPER_BOUND = upper_bound,
        .LOWER_BOUND = lower_bound,
    };
    return header;
}

void header_next_month(Header *header) {
    date_inc_by_month(&header->current_date);
    if (header->current_date.tm_year > header->UPPER_BOUND.tm_year
        || header->current_date.tm_mon > header->UPPER_BOUND.tm_mon)
    {
        header->current_date = header->LOWER_BOUND;
    }
}

void header_previous_month(Header *header) {
    date_dec_by_month(&header->current_date);
    if (header->current_date.tm_year < header->LOWER_BOUND.tm_year
        || header->current_date.tm_mon < header->LOWER_BOUND.tm_mon)
    {
        header->current_date = header->UPPER_BOUND;
    }
}

void header_draw(Header header) {
    for (int i = 0; i < header.geometry.width; i++) {
        mvprintw(header.position.y, i, "-");
        mvprintw(header.position.y + 2, i, "-");
    }
    attron(A_BOLD);
    const char *month = month_to_string(header.current_date.tm_mon + 1);
    const int X_POS = (header.geometry.width - strlen(month) - 5) / 2;
    const int CENTER_Y_POS = header.position.y + 1;
    mvprintw(CENTER_Y_POS, X_POS, "%s %d", month, header.current_date.tm_year + 1900);
    attroff(A_BOLD);
    mvprintw(CENTER_Y_POS, header.position.x, "<--");
    mvprintw(CENTER_Y_POS, header.geometry.width - 3, "-->");
}

// ------------------------------------------------------------------------------------------------

TableFieldSizes tablefieldsizes_calc_max(TrafficUsageTable table) {
    TableFieldSizes sizes = {
        .day       = strlen("Day"),
        .interface = strlen("Interface"),
        .tx        = 0,
        .rx        = 0,
    };
    int rx_field_len = 0, tx_field_len = 0;
    for (size_t i = 0; i < table.stats.entry_count && i < table.visible_entries; i++) {
        TrafficUsage entry = table.stats.entries[i];
        rx_field_len = max(rx_field_len, datasize_string_len(entry.rx));
        tx_field_len = max(tx_field_len, datasize_string_len(entry.tx));
    }
    sizes.tx = tx_field_len + TABLE_TX_RX_PADDING;
    sizes.rx = rx_field_len + TABLE_TX_RX_PADDING;
    return sizes;
}

static void print_table_line(char line_ch, char separator, TrafficUsageTable table) {
    WINDOW *wndw = table.wndw;
    wprint_line(wndw, line_ch, table.field_sizes.day);
    wprintw(wndw, "%c", separator); wprint_line(wndw, line_ch, table.field_sizes.interface);
    wprintw(wndw, "%c", separator); wprint_line(wndw, line_ch, table.field_sizes.tx);
    wprintw(wndw, "%c", separator); wprint_line(wndw, line_ch, table.field_sizes.rx);
}

static WINDOW *recreate_window(WINDOW *wndw, Position pos, Geometry geom) {
    if (wndw != NULL) {
        wclear(wndw);
        wrefresh(wndw);
        delwin(wndw);
    }
    return newwin(geom.height, geom.width, pos.y, pos.x);
}

TrafficUsageTable trafftable_new(size_t max_height) {
    TrafficUsageTable table = {
        .stats               = trafficstats_new(),
        .entries_offset      = 0,
        .MAX_VISIBLE_ENTRIES = (max_height - 2 - TABLE_HEADER_HEIGHT) / TABLE_ENTRY_HEIGHT,
        .visible_entries     = 0,
        .geometry            = { 0, 0 },
        .position            = { 0, 0 },
        .field_sizes         = { 0, 0, 0, 0 },
        .wndw                = NULL,
    };
    table.wndw = recreate_window(table.wndw, table.position, table.geometry);

    return table;
}

void trafftable_delete(TrafficUsageTable *table) {
    trafficstats_delete(&table->stats);
    if (table->wndw == NULL) {
        return;
    }
    wclear(table->wndw);
    wrefresh(table->wndw);
    delwin(table->wndw);
}

void trafftable_set_position(TrafficUsageTable *table, Position pos) {
    table->position = pos;
    table->wndw = recreate_window(table->wndw, table->position, table->geometry);
}

void trafftable_set_stats(TrafficUsageTable *table, TrafficStats stats) {
    trafficstats_copy(stats, &table->stats);

    if (stats.entry_count == 0) {
        table->geometry = (Geometry){ strlen(TABLE_EMPTY_MSG), 1 };
        table->wndw = recreate_window(table->wndw, table->position, table->geometry);
        memset(&table->field_sizes, 0, sizeof(table->field_sizes));
        return;
    }
    table->visible_entries = min(table->MAX_VISIBLE_ENTRIES, stats.entry_count);
    table->field_sizes = tablefieldsizes_calc_max(*table);
    // 5 - '|' 3 times and 2 border char
    table->geometry.width = table->field_sizes.day + table->field_sizes.interface
                           + table->field_sizes.tx + table->field_sizes.rx + 5;
    // 2 - border; -1 because for n entries only n - 1 line separators are needed
    table->geometry.height = TABLE_HEADER_HEIGHT + 2 - 1 + TABLE_ENTRY_HEIGHT
                             * table->visible_entries;
    table->wndw = recreate_window(table->wndw, table->position, table->geometry);
}

void trafftable_draw(TrafficUsageTable table) {
    if (table.wndw == NULL) {
        return;
    }
    if (table.stats.entry_count == 0) {
        mvwprintw(table.wndw, 0, 0, TABLE_EMPTY_MSG);
        wrefresh(table.wndw);
        return;
    }
    // Printing
    int x_pos, y_pos;
    x_pos = y_pos = 1;

    mvwprintw(table.wndw, y_pos, x_pos, TABLE_HEADER, table.field_sizes.tx, "tx",
              table.field_sizes.rx, "rx");
    wmove(table.wndw, ++y_pos, x_pos);
    print_table_line('=', '|', table);

    char tx_buf[table.field_sizes.tx + 1];
    char rx_buf[table.field_sizes.rx + 1];
    for (size_t i = 0; i < table.visible_entries; i++) {
        TrafficUsage entry = table.stats.entries[i + table.entries_offset];
        format_datasize(tx_buf, sizeof(tx_buf), entry.tx);
        format_datasize(rx_buf, sizeof(rx_buf), entry.rx);

        mvwprintw(table.wndw, ++y_pos, x_pos, TABLE_ENTRY_FORMAT, entry.date.tm_mday,
                 table.field_sizes.interface, entry.interface_name,
                 table.field_sizes.tx, tx_buf, table.field_sizes.rx, rx_buf);
        wmove(table.wndw, ++y_pos, x_pos);
        print_table_line('-', '+', table);
    }
    box(table.wndw, 0, 0);
    wrefresh(table.wndw);
}

void trafftable_scrl(TrafficUsageTable *table, int scrl_value) {
    table->entries_offset += scrl_value;
    if (table->entries_offset < 0) {
        table->entries_offset = 0;
    }
    if (table->stats.entry_count - table->visible_entries < table->entries_offset) {
        table->entries_offset = table->stats.entry_count - table->visible_entries;
    }
}

// ------------------------------------------------------------------------------------------------

static void wprint_traffic_usage(WINDOW *wndw, const char *title, TrafficUsage tu) {
    const int WIDTH = getmaxx(wndw);
    int y_pos = getcury(wndw);
    int title_pos = (WIDTH - strlen(title)) / 2;
    char tx_rx_buff[WIDTH];

    mvwprintw(wndw, y_pos++, title_pos, "%s", title);
    wmove(wndw, y_pos++, 1);
    wprint_line(wndw, '-', WIDTH);

    memset(tx_rx_buff, 0, sizeof(tx_rx_buff));
    format_datasize(tx_rx_buff, sizeof(tx_rx_buff), tu.tx);
    int tx_x_pos = (WIDTH - strlen("tx: ") - datasize_string_len(tu.tx)) / 2;
    mvwprintw(wndw, y_pos++, tx_x_pos, "tx: %s", tx_rx_buff);

    memset(tx_rx_buff, 0, sizeof(tx_rx_buff));
    format_datasize(tx_rx_buff, sizeof(tx_rx_buff), tu.rx);
    int rx_x_pos = (WIDTH - strlen("rx: ") - datasize_string_len(tu.rx)) / 2;
    mvwprintw(wndw, y_pos++, rx_x_pos, "rx: %s", tx_rx_buff);
}

TotalUsage totalusage_new(const TrafficStats stats) {
    TotalUsage total_usage = {
        .traffusage_for_all_time = trafficstats_reduce(stats, &sum),
        .traffusage_for_month    = tu_new(interface_new()),
        .position                = { 0, 0 },
        .geometry                = { 0, 12 },
        .wndw                    = NULL,
    };
    size_t tx_width = datasize_string_len(total_usage.traffusage_for_all_time.tx);
    size_t rx_width = datasize_string_len(total_usage.traffusage_for_all_time.rx);
    size_t tx_rx_max_width = max(tx_width, rx_width);
    size_t title_max_width = max(strlen(USAGE_FOR_ALL_TIME_TITLE), strlen(USAGE_FOR_MONTH_TITLE));
    // 2 - border; +4 because of "tx: "
    total_usage.geometry.width = max(tx_rx_max_width, title_max_width) + 2 + 4;
    total_usage.wndw = recreate_window(NULL, total_usage.position, total_usage.geometry);
    return total_usage;
}

void totalusage_delete(TotalUsage *total_usage) {
    if (total_usage->wndw == NULL) {
        return;
    }
    wclear(total_usage->wndw);
    wrefresh(total_usage->wndw);
    delwin(total_usage->wndw);
}

void totalusage_set_position(TotalUsage *total_usage, Position pos) {
    total_usage->position = pos;
    total_usage->wndw = recreate_window(total_usage->wndw, total_usage->position,
                                                           total_usage->geometry);
}

void totalusage_calc_for_current_month(TotalUsage *total_usage, TrafficStats stats_for_month) {
    total_usage->traffusage_for_month = trafficstats_reduce(stats_for_month, &sum);
}

void totalusage_draw(TotalUsage total_usage) {
    if (total_usage.wndw == NULL) {
        return;
    }
    wclear(total_usage.wndw);

    // Total usage for all time
    wmove(total_usage.wndw, 1, 1);
    wprint_traffic_usage(total_usage.wndw, USAGE_FOR_ALL_TIME_TITLE,
                         total_usage.traffusage_for_all_time);

    // Total usage for month
    wmove(total_usage.wndw, 6, 1);
    wprint_traffic_usage(total_usage.wndw, USAGE_FOR_MONTH_TITLE,
                         total_usage.traffusage_for_month);

    box(total_usage.wndw, 0, 0);
    wrefresh(total_usage.wndw);
}

// ------------------------------------------------------------------------------------------------

void enter_tui_mode(TrafficStats stats) {
    initscr();
    noecho();
    curs_set(0);
    const int WIDTH = getmaxx(stdscr);
    const int HEIGHT = getmaxy(stdscr);
    if (stats.entry_count == 0) {
        print_empty_file_msg();
        endwin();
        return;
    }

    Header header = header_new(stats.entries[0].date, stats.entries[stats.entry_count - 1].date);
    TrafficStats filtered = trafficstats_new();
    filter_trafficusage_by_date(&filtered, stats, header.current_date);

    TotalUsage total_usage = totalusage_new(stats);
    totalusage_calc_for_current_month(&total_usage, filtered);
    bool is_total_usage_showing = false;
    const int TOTAL_USAGE_X_POS = (WIDTH - total_usage.geometry.width) / 2;
    const int TOTAL_USAGE_Y_POS = (HEIGHT - total_usage.geometry.height) / 2;
    totalusage_set_position(&total_usage, (Position){TOTAL_USAGE_X_POS, TOTAL_USAGE_Y_POS});

    // 1 - footer height;
    const int BODY_HEIGHT = HEIGHT - header.geometry.height - 1;
    TrafficUsageTable table = trafftable_new(BODY_HEIGHT);
    trafftable_set_stats(&table, filtered);
    int table_x_pos = (WIDTH - table.geometry.width) / 2;
    trafftable_set_position(&table, (Position){table_x_pos, header.geometry.height});

    while (1) {
        clear();
        refresh();
        header_draw(header);

        if (is_total_usage_showing) {
            totalusage_draw(total_usage);
        } else {
            trafftable_draw(table);
        }

        move(HEIGHT - 5, 0);
        print_footer(stats);

        int action = getch();
        switch (action) {
            case 'q':
                trafftable_delete(&table);
                totalusage_delete(&total_usage);
                endwin();
                return;
                break;
            case 'a':
                is_total_usage_showing = !is_total_usage_showing;
                break;
            case 'k':
                trafftable_scrl(&table, 1);
                break;
            case 'j':
                trafftable_scrl(&table, -1);
                break;
            case 'h':
                header_next_month(&header);
                break;
            case 'l':
                header_previous_month(&header);
                break;
        }
        if (action == 'h' || action == 'l') {
            filter_trafficusage_by_date(&filtered, stats, header.current_date);
            totalusage_calc_for_current_month(&total_usage, filtered);
            trafftable_set_stats(&table, filtered);
            int table_x_pos = (WIDTH - table.geometry.width) / 2;
            trafftable_set_position(&table, (Position){table_x_pos, header.geometry.height});
        }
    }
}

void print_footer(TrafficStats stats) {
    const int HEIGHT = getmaxy(stdscr);
    mvprintw(HEIGHT - 1, 0, "Press Q to exit    Press A to show/hide total traffic usage");
}

void filter_trafficusage_by_date(TrafficStats *dest, TrafficStats src, struct tm date) {
    trafficstats_copy(src, dest);
    trafficstats_filter(*dest, _entry.date.tm_mon == date.tm_mon &&
                                  _entry.date.tm_year == date.tm_year);
}

void print_empty_file_msg() {
    const char *msg = "This file is empty ;-;";
    const int X_POS = (getmaxx(stdscr) - strlen(msg)) / 2;
    const int Y_POS = (getmaxy(stdscr) - 1) / 2;
    mvprintw(Y_POS, X_POS, "%s", msg);
    refresh();
    getch();
}

// ------------------------------------------------------------------------------------------------

void format_datasize(char *buff, size_t len, DataSize ds) {
    char temp[8];
    bool need_space = false;
    memset(buff, 0, len);
    if (ds.GiB) {
        sprintf(temp, "%zdGiB", ds.GiB);
        strcat(buff, temp);
        need_space = true;
    }
    if (ds.MiB) {
        if (need_space)
            strcat(buff, " ");
        sprintf(temp, "%zdMiB", ds.MiB);
        strcat(buff, temp);
        need_space = true;
    }
    if (ds.KiB) {
        if (need_space)
            strcat(buff, " ");
        sprintf(temp, "%zdKiB", ds.KiB);
        strcat(buff, temp);
        need_space = true;
    }
    if (ds.bytes) {
        if (need_space)
            strcat(buff, " ");
        sprintf(temp, "%zdB", ds.bytes);
        strcat(buff, temp);
    }
}

// ------------------------------------------------------------------------------------------------

void sum(TrafficUsage *acc, TrafficUsage tu) {
    acc->rx = datasize_add(acc->rx, tu.rx);
    acc->tx = datasize_add(acc->tx, tu.tx);
}

void wprint_line(WINDOW *wndw, char ch, int len) {
    for (int i = 0; i < len; i++) {
        wprintw(wndw, "%c", ch);
    }
}

const char *month_to_string(int month) {
    switch (month) {
        case 1:  return "Jan";
        case 2:  return "Feb";
        case 3:  return "Mar";
        case 4:  return "Apr";
        case 5:  return "May";
        case 6:  return "Jun";
        case 7:  return "Jul";
        case 8:  return "Aug";
        case 9:  return "Sep";
        case 10: return "Oct";
        case 11: return "Nov";
        case 12: return "Dec";
    }
    return "Unknown";
}
