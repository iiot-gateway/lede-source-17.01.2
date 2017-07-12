/*! *********************************************************************************
* \file hsdkLock.c
* This is a source file for the thread synchronization module.
*
* Copyright 2013-2016 Freescale Semiconductor, Inc.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* o Redistributions of source code must retain the above copyright notice, this list
*   of conditions and the following disclaimer.
*
* o Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
* o Neither the name of Freescale Semiconductor, Inc. nor the names of its
*   contributors may be used to endorse or promote products derived from this
*   software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************** */
#include "hsdkOSCommon.h"

#ifdef _WIN32

Lock HSDKCreateLock()
{
    Lock lock = (Lock)malloc(sizeof(CRITICAL_SECTION));
    InitializeCriticalSection(lock);
    return lock;
}

int HSDKAcquireLock(Lock l)
{
    EnterCriticalSection(l);
    return ERROR_SUCCESS;
}

int HSDKReleaseLock(Lock l)
{
    LeaveCriticalSection(l);
    return ERROR_SUCCESS;
}

int HSDKDestroyLock(Lock l)
{
    DeleteCriticalSection(l);
    return ERROR_SUCCESS;
}

#elif __linux__ || __APPLE__

Lock HSDKCreateLock()
{
    Lock lock = (Lock)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(lock, NULL);
    return lock;
}

int HSDKAcquireLock(Lock l)
{
    return pthread_mutex_lock(l);
}

int HSDKReleaseLock(Lock l)
{
    return pthread_mutex_unlock(l);
}

int HSDKDestroyLock(Lock l)
{
    return pthread_mutex_destroy(l);
}

ConditionVariable HSDKCreateCondition()
{
    ConditionVariable cond;
    pthread_cond_init(&cond, NULL);
    return cond;
}

int HSDKSignalCondition(ConditionVariable cond)
{
    return pthread_cond_signal(&cond);
}

int HSDKSignalConditionBroadcast(ConditionVariable cond)
{
    return pthread_cond_signal(&cond);
}

int HSDKDestroyCondition(ConditionVariable cond)
{
    return pthread_cond_destroy(&cond);
}

#endif
