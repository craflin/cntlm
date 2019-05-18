
#include <syslog.h>

extern "C" {

void openlog(const char *ident, int option, int facility)
{
}

void syslog(int priority, const char *format, ...)
{
}

void closelog(void)
{
}

}
