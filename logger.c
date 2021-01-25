#include "frcc.h"

#define LOG_FILE     "./log/log.txt"

FILE* log_file;

void open_logger() {
    if((log_file = fopen(LOG_FILE, "a+")) == NULL) {
        exit(1);
    }
}

void log_print(char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    vfprintf(log_file, fmt, ap);
    return;
}

void close_logger() {
    fclose(log_file);
}
