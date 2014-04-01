#ifndef DT_LOG_H
#define DT_LOG_H

typedef enum
{
    DT_LOG_INVALID = -1,
    DT_LOG_DEBUG,
    DT_LOG_WARNING,
    DT_LOG_INFO,
    DT_LOG_ERROR,
    DT_LOG_MAX
} DT_LOG_LEVEL;

void dt_log (const char *tag, DT_LOG_LEVEL level, const char *fmt, ...);
void dt_error (const char *tag, const char *fmt, ...); // default error level
void dt_debug (const char *tag, const char *fmt, ...); // default debug level
void dt_warning (const char *tag, const char *fmt, ...); // default warning level
void dt_info (const char *tag, const char *fmt, ...); // default info level

void dt_set_log_level (int level);
void dt_get_log_level (int level);

#endif
