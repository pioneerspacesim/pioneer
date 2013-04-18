#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <new>

/*!
**
** Copyright (c) 2009 by John W. Ratcliff mailto:jratcliffscarab@gmail.com
**
** If you find this code useful or you are feeling particularily generous I would
** ask that you please go to http://www.amillionpixels.us and make a donation
** to Troy DeMolay.
**
** Skype ID: jratcliff63367
** Yahoo: jratcliff63367
** AOL: jratcliff1961
** email: jratcliffscarab@gmail.com
** Personal website: http://jratcliffscarab.blogspot.com
** Coding Website:   http://codesuppository.blogspot.com
** FundRaising Blog: http://amillionpixels.blogspot.com
** Fundraising site: http://www.amillionpixels.us
** New Temple Site:  http://newtemple.blogspot.com
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
#include "LockFreeQ.h"
#include "pool.h"

JOB_SWARM::JobSwarmContext *gJobSwarmContext=0;

namespace JOB_SWARM
{





class SwarmJob : public LOCK_FREE_Q::node_t
{
public:
    SwarmJob(void)
    {
        mNext      = 0;
        mPrevious  = 0;
        mInterface = 0;
        mCancelled = false;
        mUserData  = 0;
        mUserId    = 0;
    }

    SwarmJob(JobSwarmInterface *iface,void *userData,int userId)
    {
        mInterface = iface;
        mCancelled = false;
        mUserData  = userData;
        mUserId    = userId;
    }

    void notifyCompletion(void)
    {
        assert(mInterface);
        if ( mInterface )
        {
            if ( mCancelled )
                mInterface->job_onCancel(mUserData,mUserId);
            else
                mInterface->job_onFinish(mUserData,mUserId);
        }
        mInterface = 0;
    }


    void onExecute(void)
    {
        mInterface->job_process(mUserData,mUserId);
    }


    bool isCancelled(void) const {
        return mCancelled;
    };

    void cancel(void)
    {
        mCancelled = true;
    }

    SwarmJob *  GetNext(void) const              {
        return mNext;
    };
    SwarmJob *  GetPrevious(void) const          {
        return mPrevious;
    };
    void        SetNext(SwarmJob *next)         {
        mNext = next;
    };
    void        SetPrevious(SwarmJob *previous) {
        mPrevious = previous;
    };

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

#define MAX_COMPLETION 4096

class ThreadWorker : public THREAD_CONFIG::ThreadInterface
{
public:

    ThreadWorker(void)
    {
        mSwarmJob    = 0;
        mJobScheduler = 0;
        mExit         = false;
        mBusy         = 0;
        mThread     = 0;
        mRunning    = false;
        mFinished.init(MAX_COMPLETION);
    }

    bool isRunning(void) const {
        return mRunning;
    };

    bool hasJob(void) const
    {
        bool ret = false;

        if ( mSwarmJob != 0 ) ret = true;

        return ret;
    }

    ~ThreadWorker(void)
    {
        release();
    }

    void release(void)
    {
        THREAD_CONFIG::tc_releaseThreadEvent(mBusy);
        THREAD_CONFIG::tc_releaseThread(mThread);
        mBusy = 0;
        mThread = 0;
    }

    void setJobScheduler(JobScheduler *job);

    // occurs in another thread
    void threadMain(void);

    SwarmJob * getFinished(void)
    {
        SwarmJob *ret = 0;
        mFinished.pop(ret);
        return ret;
    }

    void setExit(void)
    {
        mExit = true;
        mBusy->setEvent(); // force a signal on the event!
    }

private:
    bool                  mRunning;
    bool                  mExit;
    THREAD_CONFIG::Thread               *mThread;
    THREAD_CONFIG::ThreadEvent          *mBusy;
    JobScheduler         *mJobScheduler;    // provides new jobs to perform
    SwarmJob            *mSwarmJob;             // current job being worked on
    LOCK_FREE_Q::CQueue< SwarmJob * > mFinished;     // jobs that have been completed and may be reported back to the application.
};

class JobScheduler : public JobSwarmContext
{
public:

    JobScheduler(int maxThreadCount)
    {
        mUseThreads = true;
        maxThreadCount = maxThreadCount;

        mPending = LOCK_FREE_Q::createLockFreeQ();
        mWaitFinish = false;

		// changed the default number of jobs to 1000 as the terrain can pump out several hundred
        mJobs.Set(1000,100,65536,"JobScheduler->mJobs",__FILE__,__LINE__);

        mMaxThreadCount = maxThreadCount;

        mThreads = MEMALLOC_NEW_ARRAY(ThreadWorker,mMaxThreadCount)[mMaxThreadCount]; // the number of worker threads....

        for (unsigned int i=0; i<mMaxThreadCount; i++)
        {
            mThreads[i].setJobScheduler(this);
        }
    }

    ~JobScheduler(void)
    {
        waitFinish();
        MEMALLOC_DELETE_ARRAY(ThreadWorker,mThreads);
        LOCK_FREE_Q::releaseLockFreeQ(mPending);
    }

    // Happens in main thread
    SwarmJob * createSwarmJob(JobSwarmInterface *tji,void *userData,int userId)
    {
        SwarmJob *vret[2] = { mJobs.GetFreeLink() , mJobs.GetFreeLink() };
        //
        SwarmJob *ret=0;
        if( vret[0]==mPending->getHead() )
        {
            ret=vret[1];
            mJobs.Release( vret[0] );
        }
        else
        {
            ret=vret[0];
            mJobs.Release( vret[1] );
        }
        //
        new ( ret ) SwarmJob(tji,userData,userId);
        mPending->enqueue(ret);
        //
        return ret;
    }

    // Empty the finished list.  This happens in the main application thread!
    unsigned int processSwarmJobs(void)
    {
        unsigned int ret = 0;

        bool completion = true;
        while ( completion )
        {
            completion = false;

            for (unsigned int i=0; i<mMaxThreadCount; i++)
            {
                SwarmJob *job = mThreads[i].getFinished();
                if ( job )
                {
                    completion = true;
                    job->notifyCompletion();
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
                    if ( !job->isCancelled() )
                    {
                        job->onExecute();
                    }
                    job->notifyCompletion();
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

        return ret;
    }

    // called from other thread to get a new job to perform
    SwarmJob * getJob(void)
    {
        SwarmJob *ret = 0;

        if ( !mWaitFinish && mUseThreads )
        {
            ret = static_cast< SwarmJob *>(mPending->dequeue());
        }
        //
        return ret;
    }

    // called from main thread..tags a job as canceled, callbacks will still occur!!
    void cancel(SwarmJob *job)
    {
        job->cancel();
    }


    void setUseThreads(bool state)
    {
        mUseThreads = state;
    }

    void waitFinish(void)
    {
        mWaitFinish = true;

        while ( mWaitFinish )
        {
            bool busy = false;

            for (unsigned int j=0; j<mMaxThreadCount; j++)
            {
                ThreadWorker *tw = &mThreads[j];
                tw->setExit();
            }

            for (unsigned int i=0; i<mMaxThreadCount; i++)
            {
                ThreadWorker *tw = &mThreads[i];
                if ( tw->isRunning() )
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
    }

private:
    bool                    mUseThreads;
    bool                    mWaitFinish;  // waiting for a finish completion event to occur...
    unsigned int            mMaxThreadCount;
    LOCK_FREE_Q::LockFreeQ *mPending;
    Pool< SwarmJob >       mJobs;
    ThreadWorker           *mThreads;

};


void ThreadWorker::setJobScheduler(JobScheduler *job)
{
    mRunning      = true;
    mJobScheduler = job;
    mBusy         = THREAD_CONFIG::tc_createThreadEvent();
    mThread       = THREAD_CONFIG::tc_createThread(this);
}

void ThreadWorker::threadMain(void)
{
    while ( !mExit )
    {
        if ( mSwarmJob ) // if I have a job to do...
        {
            if ( !mSwarmJob->isCancelled() )
            {
                mSwarmJob->onExecute();              // execute the job.
            }
            mFinished.push(mSwarmJob);
        }

        if ( !mExit )
        {

            SwarmJob *job = 0;

            if ( !mFinished.isFull() )
            {
                job = mJobScheduler->getJob(); // get a new job to perform.
            }

            if (job )
            {
                mSwarmJob = job;
            }
            else
            {
                mSwarmJob = 0;
                mBusy->resetEvent();
                mBusy->waitForSingleObject(10);
            }
        }
    }
    mRunning = false;
}



JobSwarmContext * createJobSwarmContext(unsigned int maxThreads)
{
    JobScheduler *tjf = MEMALLOC_NEW(JobScheduler)(maxThreads);
    JobSwarmContext *ret = static_cast< JobSwarmContext *>(tjf);
    return ret;
}

bool releaseJobSwarmContext(JobSwarmContext *tc)
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
