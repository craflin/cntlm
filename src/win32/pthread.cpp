
#include <pthread.h>

#include <unordered_map>

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
                WaitForSingleObject(eventHandle, INFINITE);
                if (lockHandle == (void*)1)
                {
                    CloseHandle(eventHandle);
                    return 0;
                }

                ResetEvent(eventHandle);
                exchangeResult = InterlockedExchangePointer(&mutex->handle, eventHandle);
                SetEvent(lockHandle);
                lockHandle = exchangeResult;
            }
        }
        else if (exchangeResult == 0)
        {
            lockHandle = InterlockedCompareExchangePointer(&mutex->handle, (void*)1, 0);
            if (lockHandle == 0)
            {
                CloseHandle(eventHandle);
                return 0;
            }
        }
        else
            lockHandle = exchangeResult;
    }
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
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
    struct ThreadData
    {
        HANDLE hThread;
        void* (*start_routine)(void*);
        void* arg;
        void* result;
    };

    DWORD WINAPI pthread_proc(_In_ LPVOID lpParameter)
    {
        ThreadData* threadData = (ThreadData*)lpParameter;
        threadData->hThread = GetCurrentThread();
        threadData->result = threadData->start_routine(threadData->arg);
        return 0;
    }

    pthread_mutex_t _threadsMutex = PTHREAD_MUTEX_INITIALIZER;
    std::unordered_map<DWORD, ThreadData*> _threads;
}

pthread_t pthread_self(void)
{
    return GetCurrentThreadId();
}

int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg)
{
    ThreadData* threadData = new ThreadData;
    DWORD threadId;
    HANDLE hThread = CreateThread(NULL, 0, &pthread_proc, threadData, 0, &threadId);
    if (!hThread)
    {
        delete threadData;
        return -1;
    }
    threadData->hThread = hThread;
    {
        pthread_mutex_lock(&_threadsMutex);
        _threads.insert(std::make_pair(threadId, threadData));
        pthread_mutex_unlock(&_threadsMutex);
    }
    return 0;
}

int pthread_join(pthread_t thread, void** retval)
{
    ThreadData* threadData;
    {
        pthread_mutex_lock(&_threadsMutex);
        std::unordered_map<DWORD, ThreadData*>::iterator it = _threads.find(thread);
        threadData = it->second;
        _threads.erase(it);
        pthread_mutex_unlock(&_threadsMutex);
    }
    WaitForSingleObject(threadData->hThread, INFINITE);
    CloseHandle(threadData->hThread);
    if (retval)
        *retval = threadData->result;
    delete threadData;
    return 0;
}

}
