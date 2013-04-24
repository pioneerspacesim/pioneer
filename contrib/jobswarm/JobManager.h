#ifndef __JOBMANAGER_H__
#define __JOBMANAGER_H__

#include <SDL.h>
#include <deque>

#include "ThreadConfig.h"
#include "JobSwarm.h"

#include <cstdlib>
#include <climits>

//********************************************************************************
// A small class to handle each individual 'job'
//********************************************************************************
class PureJob : public JOB_SWARM::JobSwarmInterface
{
public:
	enum eJobStatus {
		EJB_WORKING=0,
		EJB_COMPLETE,
		EJB_CANCELLED,
		EJB_INVALID
	};

    PureJob() : mCounter(NULL), mJobStatus(EJB_INVALID)
    {
    }

	virtual ~PureJob()
	{
		mCounter = NULL;
		mJobStatus = EJB_INVALID;
	}

    virtual void Init(unsigned int *counter)
    {
        mCounter = counter;
		mJobStatus = EJB_WORKING;
    }

    virtual void Process(void * userData,int /* userId */) = 0;    // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!

    virtual void OnFinish(void * /* userData */,int /* userId */)  // runs in primary thread of the context
    {
        (*mCounter)--;
		mJobStatus = EJB_COMPLETE;
    }

    virtual void OnCancel(void * /* userData */,int /* userId */)   // runs in primary thread of the context
    {
        (*mCounter)--;
		mJobStatus = EJB_CANCELLED;
    }

	virtual bool IsJobRunning() const 
	{ 
		return (EJB_WORKING==mJobStatus); 
	}

private:
    unsigned int	*mCounter;
	eJobStatus		mJobStatus;
};

class JobManager
{
public:
	// terrain always adds jobs in blocks of 4 so we'll actually go over this value sometimes then settle back down to it.
	// this is not a flaw or a problem it's just a way of limiting the number of jobs which was occassionally... insane.
	static const unsigned int MAX_NUMBER_JOBS = 1024; 
	static const unsigned int INVALID_JOB_HANDLE = UINT_MAX;
	typedef unsigned int JobHandle;

	// ctor and dtor
	JobManager(const int iNumThreads=(-1));
	~JobManager();

	// adds the job to the list, takes ownership of pointer
	bool					CanAddJob() const { return (mTasksRemaining<MAX_NUMBER_JOBS); }
	JOB_SWARM::SwarmJob*	AddJob(PureJob* pNewJob, unsigned char *userData);

	void Update();
	bool JobsRemaining() const;
	void CancelAllPendingJobs();
	void Cancel(JOB_SWARM::SwarmJob* pJob);

	unsigned int NumOfThreadsUsed() const { return mNumThreadsUsed; }

private:
	JOB_SWARM::SwarmJob*		AddIncomingJob(PureJob* pNewJob, unsigned char *userData);
	JOB_SWARM::JobSwarmContext	*mpContext;

	JobHandle mPrevTasksRemaining;
    JobHandle mTasksRemaining;
	JobHandle mCurrentTaskID;
	JobHandle mMaxNumTasks;
	JobHandle mNumTasksSoFar;
	JobHandle mNumThreadsUsed;

	typedef std::pair<PureJob*, unsigned char*> TIncomingJobData;
	std::deque<TIncomingJobData> mIncomingJobs;
	SDL_mutex* mIncomingMutex;
};

#endif
