
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void openlog(const char *ident, int option, int facility);
void syslog(int priority, const char *format, ...);
void closelog(void); 

#define LOG_ERR 3
#define LOG_WARNING 4
#define LOG_INFO 6
#define LOG_DEBUG 7

#define LOG_CONS 0
#define LOG_DAEMON 0
#define LOG_PID 0

#ifdef __cplusplus
}
#endif
