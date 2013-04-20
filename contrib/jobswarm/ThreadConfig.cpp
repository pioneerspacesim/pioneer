#include <cassert>
#include "ThreadConfig.h"

/*!
**
** Copyright (c) 20011 by John W. Ratcliff mailto:jratcliffscarab@gmail.com
**
**
** The MIT license:
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is furnished
** to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <SDL.h>

#if defined(WIN32)
#define _WIN32_WINNT 0x400
#include <windows.h>
#pragma comment(lib,"winmm.lib")
#endif

#if defined(__linux__)
//#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#define __stdcall
#endif

#ifdef	NDEBUG
#define VERIFY( x ) (x)
#else
#define VERIFY( x ) assert((x))
#endif

namespace THREAD_CONFIG
{

unsigned int tc_timeGetTime()
{
#if defined(__linux__)
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#else
    return timeGetTime();
#endif
}

void   tc_sleep(unsigned int ms)
{
#if defined(__linux__)
    usleep(ms * 1000);
#else
    Sleep(ms);
#endif
}

//void tc_spinloop()
//{
//#ifdef __linux__
//    asm ( "pause" );
//#else
//    __asm { pause };
//#endif
//}

static int _ThreadWorkerFunc(void *);

class MyThread : public Thread
{
public:
    MyThread(ThreadInterface *iface)
    {
        mInterface = iface;
		mThread = SDL_CreateThread(_ThreadWorkerFunc, this);
    }

    ~MyThread()
    {
    }

    void onJobExecute()
    {
        mInterface->ThreadMain();
    }

	virtual void Suspend(void)
	{
#ifdef HACD_WINDOWS
		SuspendThread(mThread);
#endif
	}

	virtual void Resume(void) 
	{
#ifdef HACD_WINDOWS
		ResumeThread(mThread);
#endif
	}

private:
    ThreadInterface *mInterface;
	SDL_Thread *mThread;
};


Thread      * tc_createThread(ThreadInterface *tinterface)
{
    MyThread *m = new MyThread(tinterface);
    return static_cast< Thread *>(m);
}

void          tc_releaseThread(Thread *t)
{
    MyThread *m = static_cast<MyThread *>(t);
    delete m;
}

static int _ThreadWorkerFunc(void *arg)
{
    MyThread *worker = (MyThread *)arg;
    worker->onJobExecute();
    return 0;
}



ThreadEvent::ThreadEvent()
{
	mpEventMutex = SDL_CreateMutex();
	mpEvent = SDL_CreateCond();
}

ThreadEvent::~ThreadEvent()
{
	SDL_DestroyCond(mpEvent);
	SDL_DestroyMutex(mpEventMutex);
}

void ThreadEvent::setEvent()  // signal the event
{
	VERIFY( SDL_mutexP(mpEventMutex) == 0 );	// lock
	VERIFY( SDL_CondSignal(mpEvent) == 0 );	 	//Waits on a condition variable
	VERIFY( SDL_mutexV(mpEventMutex) == 0 );	// unlock
}

void ThreadEvent::waitForSingleObject(unsigned int ms)
{
	SDL_mutexP(mpEventMutex);	// lock
	if (ms == 0xffffffff)
    {
		VERIFY( SDL_CondWait(mpEvent, mpEventMutex) == 0 );	 	//Waits on a condition variable
    }
    else
    {
		VERIFY( SDL_CondWaitTimeout(mpEvent, mpEventMutex, ms) == 0 );	 	//Waits on a condition variable, with timeout Time
    }
	SDL_mutexV(mpEventMutex);	// unlock
}

int tc_atomicAdd(int *addend,int amount)
{
#if defined(WIN32)
	return InterlockedExchangeAdd((long*) addend, long (amount));
#endif

#if defined(__linux__)
	return __sync_fetch_and_add ((int32_t*)addend, amount );
#endif

#if defined(__APPLE__)
	int count = OSAtomicAdd32 (amount, (int32_t*)addend);
	return count - *addend;
#endif

}

ThreadEvent * tc_createThreadEvent()
{
    ThreadEvent *m = new ThreadEvent;
    return m;
}

void tc_releaseThreadEvent(ThreadEvent *t)
{
    delete t;
}

}; // end of namespace
