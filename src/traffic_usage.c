#include "traffic_usage.h"

// Reads and returns written in a file number
static size_t read_number_from_file(const char *fpath) {
    assert(!has_error(check_file(fpath)));

    size_t bytes = 0;
    FILE *fp = fopen(fpath, "r");
    if (!fp) {
        return 0;
    }
    fscanf(fp, "%li", &bytes);
    fclose(fp);
    return bytes;
}

// ------------------------------------------------------------------------------------------------

DataSize datasize_new(void) {
    return (DataSize){0, 0, 0, 0};
}

void datasize_normalize(DataSize *data_size) {
    data_size->KiB += data_size->bytes / 1024;
    data_size->bytes %= 1024;

    data_size->MiB += data_size->KiB / 1024;
    data_size->KiB %= 1024;

    data_size->GiB += data_size->MiB / 1024;
    data_size->MiB %= 1024;
}

DataSize datasize_add(DataSize ds1, DataSize ds2) {
     return datasize_from_bytes(datasize_to_bytes(ds1) + datasize_to_bytes(ds2));
}

DataSize datasize_diff(DataSize ds1, DataSize ds2) {
     return datasize_from_bytes(datasize_to_bytes(ds1) - datasize_to_bytes(ds2));
}

DataSize datasize_from_bytes(size_t bytes) {
    DataSize ds = datasize_new();
    ds.bytes = bytes;
    datasize_normalize(&ds);
    return ds;
}

size_t datasize_to_bytes(DataSize ds) {
    ds.MiB += ds.GiB * 1024;
    ds.KiB += ds.MiB * 1024;
    ds.bytes += ds.KiB * 1024;
    return ds.bytes;
}

size_t datasize_string_len(DataSize ds) {
    size_t len = 0;
    int need_space = 0;

    len += count_digits(ds.bytes) + count_digits(ds.KiB)
           + count_digits(ds.MiB) + count_digits(ds.GiB);

    if (ds.bytes) {
        len += need_space++;
        len += strlen("B");
    }
    if (ds.KiB) {
        len += strlen("KiB") + need_space;
    }
    if (ds.MiB) {
        len += strlen("MiB") + need_space;
    }
    if (ds.GiB) {
        len += strlen("GiB") + need_space;
    }

    return len;
}

// ------------------------------------------------------------------------------------------------

Interface interface_new(void) {
    return (Interface){ "", "", "" };
}

TUError interface_init(Interface *interface, const char *name) {
    TUError error;

    strcpy(interface->name, name);

    sprintf(interface->tx_file_path, NET_STAT_PATH_FORMAT, name, "tx_bytes");
    error = check_file(interface->tx_file_path);
    if (has_error(error)) {
        return error;
    }

    sprintf(interface->rx_file_path, NET_STAT_PATH_FORMAT, name, "rx_bytes");
    error = check_file(interface->rx_file_path);
    if (has_error(error)) {
        return error;
    }

    return make_ok();
}

TrafficUsage interface_get_traffic_usage(Interface interface) {
    TrafficUsage tf_usage = tu_new(interface);

    // tx_bytes
    tf_usage.tx = datasize_from_bytes(read_number_from_file(interface.tx_file_path));
    // rx_bytes
    tf_usage.rx = datasize_from_bytes(read_number_from_file(interface.rx_file_path));
    // time
    time_t raw_time = time(NULL);
    tf_usage.date = *localtime(&raw_time);

    return tf_usage;
}

TUError check_interface_existence(const char *interface_name) {
    DIR *dir = opendir(INTERFACE_PATH);
    struct dirent *entry;
    if (!dir) {
        return make_error(TU_DIR_DOESNT_EXIST, INTERFACE_PATH);
    }

    while ( (entry = readdir(dir)) != NULL ) {
        if (strcmp(entry->d_name, interface_name) == 0) {
            closedir(dir);
            return make_ok();
        }
    }
    closedir(dir);
    return make_error(TU_INTERFACE_DOESNT_EXIST, interface_name);
}

TrafficUsage tu_new(Interface interface) {
    TrafficUsage tu;
    struct tm date;
    memset(&date, 0, sizeof(struct tm));
    tu.date = date;
    strcpy(tu.interface_name, interface.name);
    tu.tx = tu.rx = datasize_new();
    return tu;
}
