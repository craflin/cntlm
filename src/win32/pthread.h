
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
    void* volatile handle;
} pthread_mutex_t;

#define PTHREAD_MUTEX_INITIALIZER {0}

typedef void* pthread_t;
typedef int pthread_attr_t;

int pthread_mutex_lock(pthread_mutex_t* mutex);
int pthread_mutex_unlock(pthread_mutex_t* mutex);

pthread_t pthread_self(void);
int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg);
int pthread_join(pthread_t thread, void** retval);

#ifdef __cplusplus
}
#endif
