
#include <fnmatch.h>

extern "C" {

int fnmatch(const char* pat, const char* str, int)
{
    const char* s, * p;
    bool star = false;
loopStart:
    for (s = str, p = pat; *s; ++s, ++p)
    {
        switch (*p)
        {
        case '?':
            break;
        case '*':
            star = true;
            str = s, pat = p;
            do { ++pat; } while (*pat == '*');
            if (!*pat) return 0;
            goto loopStart;
        default:
            if (*s != *p) goto starCheck;
            break;
        }
    }
    while (*p == '*') ++p;
    return !*p ? 1 : 0;
starCheck:
    if (!star) return 1;
    str++;
    goto loopStart;
}

}
