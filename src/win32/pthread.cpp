
#include <pthread.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

extern "C" {

pthread_t pthread_self(void)
{
    return GetCurrentThread();
}

}
