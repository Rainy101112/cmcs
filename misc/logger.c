#include "logger.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static FILE *log_file = NULL;

void plog_init(const char *path) {
    if (path == NULL) path = "server.log";
    log_file = fopen(path, "a");
    if (log_file == NULL) {
        fprintf(stderr, "[LOGGER] Failed to open log file: %s\n", path);
    }
}

void plog_close(void) {
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}

void plog(const char *format, ...) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timebuf[20];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);

    fprintf(stdout, "[%s] ", timebuf);

    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);

    fprintf(stdout, "\n");
    fflush(stdout);

    if (log_file != NULL) {
        fprintf(log_file, "[%s] ", timebuf);
        va_list args2;
        va_start(args2, format);
        vfprintf(log_file, format, args2);
        va_end(args2);
        fprintf(log_file, "\n");
        fflush(log_file);
    }
}

void plog_noprefix(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    fprintf(stdout, "\n");
    fflush(stdout);

    if (log_file != NULL) {
        va_list args2;
        va_start(args2, format);
        vfprintf(log_file, format, args2);
        va_end(args2);
        fprintf(log_file, "\n");
        fflush(log_file);
    }
}
