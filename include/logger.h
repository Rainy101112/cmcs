#ifndef INCLUDE_LOGGER_H_
#define INCLUDE_LOGGER_H_

#define LOG_INFO    "[INFO]\t"
#define LOG_WARN    "[WARN]\t"
#define LOG_ERROR   "[ERROR]\t"

void plog_init(const char *path);
void plog_close(void);
void plog(const char *format, ...);
void plog_noprefix(const char *format, ...);

#endif // INCLUDE_LOGGER_H_
