#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <new>

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

#include "JobSwarm.h"
#include "ThreadConfig.h"
#include "ThreadSafeQueue.h"
#include "pool.h"

#define MAX_THREADS 64

namespace JOB_SWARM
{

class SwarmJob : public THREAD_SAFE_QUEUE::node_t
{
public:
    SwarmJob()
    {
        mNext      = NULL;
        mPrevious  = NULL;
        mInterface = NULL;
        mCancelled = false;
        mUserData  = NULL;
        mUserId    = 0;
    }

    SwarmJob(JobSwarmInterface *iface,void *userData,int userId)
    {
        mInterface = iface;
        mCancelled = false;
        mUserData  = userData;
        mUserId    = userId;
    }

    void NotifyCompletion()
    {
        assert(mInterface);
        if ( mInterface )
        {
            if ( mCancelled )
                mInterface->OnCancel(mUserData,mUserId);
            else
                mInterface->OnFinish(mUserData,mUserId);
        }
        mInterface = NULL;
    }


    void OnExecute()
    {
        mInterface->Process(mUserData,mUserId);
    }


    bool IsCancelled() const {
        return mCancelled;
    };

    void Cancel()
    {
        mCancelled = true;
    }

	SwarmJob *  GetNext(void) const              { return mNext; };
	SwarmJob *  GetPrevious(void) const          { return mPrevious; };
	void        SetNext(SwarmJob *next)         { mNext = next; };
	void        SetPrevious(SwarmJob *previous) { mPrevious = previous; };

private:
    SwarmJob          *mNext;
    SwarmJob          *mPrevious;
    JobSwarmInterface *mInterface;
    bool                mCancelled;
    void               *mUserData;
    int                 mUserId;
};

class JobScheduler;

// Each thread worker can keep track of 4 jobs.
// There can be as many thread workers as there application desiers.

#define MAX_COMPLETION 4096 // maximum number of completed jobs we will queue up before they get despooled by the main thread.

class ThreadWorker : public THREAD_CONFIG::ThreadInterface
{
public:

    ThreadWorker()
    {
        mJobScheduler = NULL;
        mExit         = false;
        mThread     = NULL;
        mWaiting = true;
		mWaitPending = false;
		mExitComplete = false;
		mThreadFlag = 0;
        mFinished.init(MAX_COMPLETION);
    }

    ~ThreadWorker()
    {
        Release();
    }

    void Release()
    {
		SetExit();
		while ( !mExitComplete )
		{
			THREAD_CONFIG::tc_sleep(0);
		}
        THREAD_CONFIG::tc_releaseThread(mThread);
        
        mThread = NULL;
		mWaiting = true;
		mWaitPending = false;
    }

    void SetJobScheduler(JobScheduler *job, unsigned int threadFlag);

    // occurs in another thread
    void ThreadMain();

    SwarmJob * GetFinished()
    {
        SwarmJob *ret = NULL;
        mFinished.pop(ret);
        return ret;
    }

    void SetExit()
    {
        mExit = true;
		WakeUp();
	}

	bool WakeUp(void)
	{
		bool ret = mWaiting;

		if ( mWaiting )
		{
			mWaiting = false; // no longer waiting, we have woken up the thread
			mThread->Resume();
		}

		return ret;
	}

	bool ProcessWaitPending(void);
	
private:
	void Wait(void); // wait until we are signalled again.

	unsigned int				mThreadFlag;	// bit flag identifying this thread
	bool						mWaitPending;
	bool						mWaiting;
	bool						mExit;					// exit condition
	bool						mExitComplete;
    THREAD_CONFIG::Thread		*mThread;
    JobScheduler         		*mJobScheduler;    // provides new jobs to perform
    THREAD_SAFE_QUEUE::CQueue< SwarmJob * > mFinished;     // jobs that have been completed and may be reported back to the application.
};

class JobScheduler : public JobSwarmContext
{
public:

    JobScheduler(const int maxThreadCount) : mUseThreads(true), mMaxThreadCount(maxThreadCount)
    {
        mPending = THREAD_SAFE_QUEUE::createThreadSafeQueue();

		// changed the default number of jobs to 1000 as the terrain can pump out several hundred
        mJobs.Set(1000,100,65536,"JobScheduler->mJobs",__FILE__,__LINE__);

        mThreads = MEMALLOC_NEW_ARRAY(ThreadWorker,mMaxThreadCount)[mMaxThreadCount]; // the number of worker threads....
		mPendingSleepCount = 0;
		mThreadAwakeCount = 0;
        for (unsigned int i=0; i<mMaxThreadCount; i++) {
			mThreadAsleep[i] = 1;
			mPendingSleep[i] = 0;
            mThreads[i].SetJobScheduler(this,i);
        }
    }

    ~JobScheduler()
    {
        delete []mThreads;
        THREAD_SAFE_QUEUE::releaseThreadSafeQueue(mPending);
    }

    // Happens in main thread
    SwarmJob * CreateSwarmJob(JobSwarmInterface *tji,void *userData,int userId)
    {
        SwarmJob *ret = mJobs.GetFreeLink();
		//
		new ( ret ) SwarmJob(tji,userData,userId);
		mPending->enqueue(ret);
		THREAD_CONFIG::tc_atomicAdd(&mPendingCount,1);
		wakeUpThreads(); // if the worker threads are aslpeep; wake them up to process this job
		return ret;
    }

    // Empty the finished list.  This happens in the main application thread!
    unsigned int ProcessSwarmJobs()
    {
        unsigned int ret = 0;

        bool completion = true;
        THREAD_CONFIG::tc_sleep(0); // give up timeslice to threads
		applySleep(); // if there are any threads waiting to go to sleep; let's put them to sleep
		wakeUpThreads(); // if there is work to be done, then wake up the threads
		// de-queue all completed jobs in the main thread.
		while ( completion )
        {
            completion = false;

            for (unsigned int i=0; i<mMaxThreadCount; i++)
            {
                SwarmJob *job = mThreads[i].GetFinished();
                if ( job )
                {
                    completion = true;
                    job->NotifyCompletion();
                    mJobs.Release(job);
                }
            }
        }

        if ( !mUseThreads )
        {
            unsigned int stime = THREAD_CONFIG::tc_timeGetTime();
            bool working = true;
            while ( working )
            {
                SwarmJob *job = static_cast< SwarmJob *>(mPending->dequeue());
                if ( job )
                {
                    if ( !job->IsCancelled() )
                    {
                        job->OnExecute();
                    }
                    job->NotifyCompletion();
                    mJobs.Release(job);
                }
                else
                {
                    working = false;
                }
                unsigned int etime = THREAD_CONFIG::tc_timeGetTime();
                unsigned int dtime = etime - stime;
                if ( dtime > 30 ) break;
            }

        }

        return mJobs.GetUsedCount();
    }

    // called from other thread to get a new job to perform
    SwarmJob * GetJob()
    {
        SwarmJob *ret = NULL;

        if ( mUseThreads )
        {
            ret = static_cast< SwarmJob *>(mPending->dequeue());
			if ( ret )
			{
				THREAD_CONFIG::tc_atomicAdd(&mPendingCount,-1);
			}
        }
        return ret;
    }

    // called from main thread..tags a job as canceled, callbacks will still occur!!
    void Cancel(SwarmJob *job)
    {
        job->Cancel();
    }


    void SetUseThreads(bool state)
    {
        mUseThreads = state;
    }
	
	void wakeUpThreads(void)
	{
		unsigned int jobsWaiting = mPendingCount;

		if ( jobsWaiting ) // if there are any threads not currently running jobs...
		{
			for (unsigned int i=0; i<mMaxThreadCount; i++)
			{
				if ( mThreadAsleep[i] )
				{
					if ( mThreads[i].WakeUp() ) // enable the signal on this thread to wake it up to process jobs
					{
						mThreadAwakeCount++;
						mThreadAsleep[i] = 0;
						jobsWaiting--;
						if ( jobsWaiting == 0 )  // if we have woken up enough threads to consume the pending jobs
							break;
					}
					else
					{
						assert(false); // should always be able to wake it up!
					}
				}
			}
		}
	}

	void putToSleep(unsigned int threadIndex)
	{
		assert( mPendingSleep[threadIndex] == false );
		THREAD_CONFIG::tc_atomicAdd(&mPendingSleep[threadIndex],1);
		THREAD_CONFIG::tc_atomicAdd(&mPendingSleepCount,1);
	}

	// always called from the main thread.  puts to sleep any treads that had no work to do.
	void applySleep(void)
	{
		if ( mPendingSleepCount ) // if there are any threads pending to be put to sleep...
		{
			for (unsigned int i=0; i<mMaxThreadCount; i++)
			{
				if ( mPendingSleep[i] )
				{
					mThreadAwakeCount--;
					mThreadAsleep[i] = 1; // mark it as actually being asleep now.
					THREAD_CONFIG::tc_atomicAdd(&mPendingSleep[i],-1);
					THREAD_CONFIG::tc_atomicAdd(&mPendingSleepCount,-1);
					mThreads[i].ProcessWaitPending();
				}
			}
//			assert(mPendingSleepCount==0);
		}
	}

    /*void WaitFinish()
    {
        mWaitFinish = true;

        while ( mWaitFinish )
        {
            bool busy = false;

            for (unsigned int j=0; j<mMaxThreadCount; j++)
            {
                ThreadWorker *tw = &mThreads[j];
                tw->SetExit();
            }

            for (unsigned int i=0; i<mMaxThreadCount; i++)
            {
                ThreadWorker *tw = &mThreads[i];
                if ( tw->IsRunning() )
                {
                    busy = true;
                    break;
                }
            }

            if ( !busy )
            {
                mWaitFinish = false;
            }
            else
            {
                THREAD_CONFIG::tc_sleep(0);
            }
        }
    }*/

private:
    bool                    mUseThreads;
    unsigned int            mMaxThreadCount;
    THREAD_SAFE_QUEUE::ThreadSafeQueue *mPending;
    Pool< SwarmJob >       mJobs;
    ThreadWorker           *mThreads;
	int				mPendingCount;
	unsigned int	mWaitPending;
	int				mPendingSleepCount;
	int				mThreadAwakeCount;
	int				mPendingSleep[MAX_THREADS];
	int				mThreadAsleep[MAX_THREADS];

};


void ThreadWorker::SetJobScheduler(JobScheduler *job, unsigned int threadFlag)
{
    mThreadFlag = threadFlag;
    mJobScheduler = job;
    mWaiting	= true; // threads begin in a suspended state!
    mThread       = THREAD_CONFIG::tc_createThread(this);
}

void ThreadWorker::ThreadMain()
{
    while ( !mExit )
    {
        if ( mWaitPending || mWaiting )
		{
			// already asleep, or waiting to be put to sleep by the main thread...
			THREAD_CONFIG::tc_sleep(0);
		}
		else
		{
			if ( !mFinished.isFull() )
			{
				SwarmJob *job = mJobScheduler->GetJob(); // get a new job to perform.
				while ( job )
				{
					job->OnExecute();              // execute the job.
					mFinished.push(job);
					job = mJobScheduler->GetJob(); // get a new job to perform.
				}
				Wait();
			}
			else
			{
				Wait();
			}
        }
    }
	mExitComplete = true;
}

void ThreadWorker::Wait(void)
{
	if ( !mExit )
		return;

	assert(!mWaitPending);
	assert(!mWaiting);
	mWaitPending = true;
	mJobScheduler->putToSleep(mThreadFlag);
}

bool ThreadWorker::ProcessWaitPending(void)
{
	bool ret = mWaitPending;
	assert(mWaiting==false);
	assert(mWaitPending);
	if ( mWaitPending )
	{
		if ( mWaiting == false )
		{
			mThread->Suspend(); // suspend thread execution
			mWaiting = true; // indicate that we are currently waiting to be freshly signalled
		}
		mWaitPending = false;
	}
	return ret;
}

JobSwarmContext * CreateJobSwarmContext(unsigned int maxThreads)
{
    JobScheduler *tjf = MEMALLOC_NEW(JobScheduler)(maxThreads);
    JobSwarmContext *ret = static_cast< JobSwarmContext *>(tjf);
    return ret;
}

bool ReleaseJobSwarmContext(JobSwarmContext *tc)
{
    bool ret = false;
    if ( tc )
    {
        JobScheduler *tjf = static_cast< JobScheduler *>(tc);
        MEMALLOC_DELETE(JobScheduler,tjf);
        ret = true;
    }
    return ret;
}

}; // END OF NAMESPACE
