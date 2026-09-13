/* Minimal SDL Windows thread create/join for nxdk. */
#include "SDL_internal.h"

#ifdef SDL_THREAD_WINDOWS

#include "../SDL_thread_c.h"
#include "../SDL_systhread.h"
#include "SDL_systhread_c.h"

#ifndef STACK_SIZE_PARAM_IS_A_RESERVATION
#define STACK_SIZE_PARAM_IS_A_RESERVATION 0x00010000
#endif

static DWORD WINAPI RunThreadViaCreateThread(LPVOID data)
{
    SDL_RunThread((SDL_Thread *)data);
    return 0;
}

bool SDL_SYS_CreateThread(SDL_Thread *thread,
                          SDL_FunctionPointer vpfnBeginThread,
                          SDL_FunctionPointer vpfnEndThread)
{
    DWORD threadid = 0;
    const DWORD flags = thread->stacksize ? STACK_SIZE_PARAM_IS_A_RESERVATION : 0;

    (void)vpfnBeginThread;
    (void)vpfnEndThread;
    thread->endfunc = NULL;

    thread->handle = CreateThread(NULL, thread->stacksize,
                                  RunThreadViaCreateThread,
                                  thread, flags, &threadid);
    thread->threadid = (SDL_ThreadID)threadid;
    if (!thread->handle) {
        return SDL_SetError("Not enough resources to create thread");
    }
    return true;
}

void SDL_SYS_SetupThread(const char *name)
{
    (void)name;
}

SDL_ThreadID SDL_GetCurrentThreadID(void)
{
    return (SDL_ThreadID)GetCurrentThreadId();
}

bool SDL_SYS_SetThreadPriority(SDL_ThreadPriority priority)
{
    int value;

    if (priority == SDL_THREAD_PRIORITY_LOW) {
        value = THREAD_PRIORITY_LOWEST;
    } else if (priority == SDL_THREAD_PRIORITY_HIGH) {
        value = THREAD_PRIORITY_HIGHEST;
    } else if (priority == SDL_THREAD_PRIORITY_TIME_CRITICAL) {
        value = THREAD_PRIORITY_TIME_CRITICAL;
    } else {
        value = THREAD_PRIORITY_NORMAL;
    }
    if (!SetThreadPriority(GetCurrentThread(), value)) {
        return WIN_SetError("SetThreadPriority()");
    }
    return true;
}

void SDL_SYS_WaitThread(SDL_Thread *thread)
{
    WaitForSingleObjectEx(thread->handle, INFINITE, FALSE);
    CloseHandle(thread->handle);
}

void SDL_SYS_DetachThread(SDL_Thread *thread)
{
    CloseHandle(thread->handle);
}

#endif /* SDL_THREAD_WINDOWS */
