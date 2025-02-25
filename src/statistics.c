#include "statistics.h"

TrafficStats trafficstats_new(void) {
    TrafficStats stats = {
        .entry_count = 0,
        .entries     = NULL,
        .cache       = { datasize_new(), datasize_new() },
    };
    return stats;
}

void trafficstats_delete(TrafficStats *trafficstats) {
    if (trafficstats->entries != NULL) {
        free(trafficstats->entries);
        trafficstats->entries = NULL;
    }
}

void trafficstats_copy(TrafficStats src, TrafficStats *dest) {
    dest->entry_count = src.entry_count;
    dest->entries = malloc(dest->entry_count * sizeof(TrafficUsage));
    memset(dest->entries, 0, dest->entry_count * sizeof(TrafficUsage));
    memcpy(dest->entries, src.entries, dest->entry_count * sizeof(TrafficUsage));
}

void trafficstats_add_entry_unsafe(TrafficStats *trafficstats, TrafficUsage tu) {
    if (trafficstats->entry_count == 0) {
        trafficstats->entries = malloc(sizeof(TrafficUsage));
        *trafficstats->entries = tu;
        trafficstats->entry_count++;
        return;
    }
    trafficstats->entries = realloc(trafficstats->entries,
                                    ++trafficstats->entry_count * sizeof(TrafficUsage));
    *(trafficstats->entries + (trafficstats->entry_count - 1)) = tu;
}

void trafficstats_add_entry(TrafficStats *trafficstats, TrafficUsage tu) {
    if (trafficstats->entry_count == 0) {
        trafficstats_add_entry_unsafe(trafficstats, tu);
        return;
    }

    TrafficUsage *last_entry = &trafficstats->entries[trafficstats->entry_count - 1];
    if (last_entry->date.tm_yday == tu.date.tm_yday
        && strcmp(last_entry->interface_name, tu.interface_name) == 0)
    {
        last_entry->tx = datasize_add(last_entry->tx, tu.tx);
        last_entry->rx = datasize_add(last_entry->rx, tu.rx);
        return;
    }
    trafficstats_add_entry_unsafe(trafficstats, tu);
}

TUError trafficstats_write(TrafficStats trafficstats, const char *fname) {
    // TODO: Создавать необходимые директории в fname рекурсивно
    errno = 0;
    FILE *fp = fopen(fname, "w");
    if (errno) {
        char buf[32 + strlen(fname)];
        sprintf(buf, "opening %s", fname);
        return make_error(TU_FROM_ERRNO, buf);
    }
    char file_format[FILE_FORMAT_VER_LEN];
    sprintf(file_format, "TU%d", FILE_FORMAT_VERSION);

    fwrite(file_format, sizeof(char), sizeof(file_format), fp);
    fwrite(&trafficstats.entry_count, sizeof(size_t), 1, fp);

    for (size_t i = 0; i < trafficstats.entry_count; i++) {
        TrafficUsage *entry = trafficstats.entries + i;
        time_t raw_time = mktime(&entry->date);
        unsigned short interface_name_len = strlen(entry->interface_name);

        fwrite(&entry->tx, sizeof(DataSize), 2, fp);
        fwrite(&interface_name_len, sizeof(short), 1, fp);
        fwrite(entry->interface_name, sizeof(char), strlen(entry->interface_name), fp);
        fwrite(&raw_time, sizeof(time_t), 1, fp);
    }
    fwrite(&trafficstats.cache, sizeof(DataSize), 2, fp);

    fclose(fp);
    return make_ok();
}

TUError trafficstats_read(TrafficStats *stats, const char *fname) {
    FILE *fp = fopen(fname, "rb");
    if (errno) {
        char buf[32 + strlen(fname)];
        sprintf(buf, "opening %s", fname);
        return make_error(TU_FROM_ERRNO, buf);
    }

    // Maybe will be used later, when I'll update file format
    char file_format[FILE_FORMAT_VER_LEN];
    fread(file_format, sizeof(char), FILE_FORMAT_VER_LEN, fp);

    fread(&stats->entry_count, sizeof(size_t), 1, fp);
    stats->entries = malloc(sizeof(TrafficUsage) * stats->entry_count);
    for (size_t i = 0; i < stats->entry_count; i++) {
        TrafficUsage tu;
        memset(&tu, 0, sizeof(TrafficUsage));
        time_t raw_time = 0;
        unsigned short interface_name_len = 0;

        fread(&tu, sizeof(DataSize), 2, fp);
        fread(&interface_name_len, sizeof(short), 1, fp);
        fread(tu.interface_name, sizeof(char), interface_name_len, fp);
        fread(&raw_time, sizeof(time_t), 1, fp);
        tu.date = *localtime(&raw_time);
        *(stats->entries + i) = tu;
    }
    fread(&stats->cache, sizeof(DataSize), 2, fp);

    fclose(fp);
    return make_ok();
}

TrafficUsage trafficstats_reduce(TrafficStats stats, StatsReduceFn reduce_func) {
    TrafficUsage acc = tu_new(interface_new());
    for (size_t i = 0; i < stats.entry_count; i++) {
        reduce_func(&acc, *(stats.entries + i));
    }
    return acc;
}

void trafficstats_concat_dup(TrafficStats *stats) {
    for (size_t i = 0; i < stats->entry_count; i++) {
        TrafficUsage *current_entry = &stats->entries[i];

        for (size_t j = i + 1; j < stats->entry_count; j++){
            TrafficUsage *entry = &stats->entries[j];
            if (current_entry->date.tm_mon != entry->date.tm_mon) break;
            if (current_entry->date.tm_mday == entry->date.tm_mday) {
                current_entry->rx = datasize_add(current_entry->rx, entry->rx);
                current_entry->tx = datasize_add(current_entry->tx, entry->tx);
                memcpy(entry, entry + 1, sizeof(TrafficUsage) * (stats->entry_count - 1 - j));
                stats->entry_count -= 1;
            }
        }
    }
}
