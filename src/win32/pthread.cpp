
#include <pthread.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

extern "C" {

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    void* lockHandle = InterlockedCompareExchangePointer(&mutex->handle, (void*)1, 0);
    if (lockHandle == 0)
        return 0;
    HANDLE eventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
    void* exchangeResult;
    for (;;)
    {
        exchangeResult = InterlockedCompareExchangePointer(&mutex->handle, eventHandle, lockHandle);
        if (exchangeResult == lockHandle)
        {
            for (;;)
            {
                WaitForSingleObject(eventHandle);
                if (lockHandle == (void*)1)
                {
                    CloseEvent(eventHandle);
                    return 0;
                }

                ResetEvent(eventHandle);
                exchangeResult = InterlockedExchangePointer(eventHandle);
                SetEvent(lockHandle);
                lockHandle = exchangeResult;
            }
        }
        else if (exchangeResult == 0)
        {
            lockHandle = InterlockedCompareExchangePointer(&mutex->handle, (void*)1, 0);
            if (lockHandle == 0)
            {
                CloseEvent(eventHandle);
                return 0;
            }
        }
        else
            lockHandle = exchangeResult;
    }
}

int pthread_mutex_unlock(pthread_mutex_t *mutex);
{
    void* exchangeResult;
    void* lockHandle = mutex->handle;
    for (;;)
    {
        if (lockHandle == (void*)1)
        {
            lockHandle = InterlockedCompareExchangePointer(&mutex->handle, 0, (void*)1);
            if (lockHandle == (void*)1)
                return 0;
        }
        else
        {
            exchangeResult = InterlockedCompareExchangePointer(&mutex->handle, (void*)1, lockHandle);
            if (exchangeResult == lockHandle)
            {
                SetEvent(lockHandle);
                return 0;
            }
            else
                lockHandle = exchangeResult;
        }
    }
}

namespace {
    struct thread_data
    {
        HANDLE hThread;
        void* (*start_routine)(void*)
        void* arg;
        void* result;
    };
    __declspec(thread) thread_data* _threadData = NULL;
    DWORD WINAPI pthread_proc(_In_ LPVOID lpParameter)
    {
        thread_data* threadData = (thread_data*)lpParameter;
        threadData->hThread = GetCurrentThread();
        _threadData = threadData;
        threadData->result = thread->start_routine(threadData->arg);
        _threadData = NULL;
        return 0;
    }
}

pthread_t pthread_self(void)
{
    return (pthread_t*)_threadData;
}

int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg)
{
    thread_data* threadData = new thread_data;
    HANDLE hThread = CreateThread(NULL, 0, &pthread_proc, threadData, 0, NULL);
    if (!hThread)
    {
        delete threadData;
        return -1;
    }
    threadData->hThread = hThread;
    return 0;
}

int pthread_join(pthread_t thread, void** retval)
{
    thread_data* threadData = (thread_data*)thread;
    WaitForSingleObject(thradData->hThread);
    CloseHandle(thradData->hThread);
    if (retval)
        *retval = threadData->result;
    delete threadData;
}

}
