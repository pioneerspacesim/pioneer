//********************************************************************************
// Demonstrates how to use the JobSwarm system.
//********************************************************************************

#include "JobManager.h"
#include "CoreCount.h"

#include "UserMemAlloc.h"
#include "ThreadConfig.h"
#include "JobSwarm.h"

#define MAX_THREADS 64
#define MIN_THREADS 1
#define DEF_THREADS 8

JobManager::JobManager(const int iNumThreads) 
	: mpContext(NULL), mPrevTasksRemaining(0), mTasksRemaining(0), mCurrentTaskID(0), mMaxNumTasks(0), mNumTasksSoFar(0), mIncomingMutex(0)
{
	// check and limit number of threads
	const int32_t numCoresToUse = std::min( std::max( (getNumCores()-1), MIN_THREADS ), MAX_THREADS );
	const int actualNumThreads = std::min( std::max( (iNumThreads<=(0) ? numCoresToUse : iNumThreads), MIN_THREADS ), MAX_THREADS );
	mpContext = JOB_SWARM::CreateJobSwarmContext( actualNumThreads );
	mNumThreadsUsed = actualNumThreads;
	mIncomingMutex = SDL_CreateMutex();
}

JobManager::~JobManager()
{
	// ok..now we MUST wait until the task remaining counter is zero.
    while ( mpContext && mTasksRemaining != 0 )
    {
        mpContext->ProcessSwarmJobs();
    }

	SDL_DestroyMutex(mIncomingMutex);

	JOB_SWARM::ReleaseJobSwarmContext(mpContext);
}

JOB_SWARM::SwarmJob* JobManager::AddJob(PureJob* pNewJob, unsigned char *userData)
{
	unsigned int handleID = mCurrentTaskID;
	++mCurrentTaskID;
	if( INVALID_JOB_HANDLE<=mCurrentTaskID ) {
		mCurrentTaskID = 0;
	}

	// no mutex, no nothing, just do it immediately
	return AddIncomingJob(pNewJob, userData);
}

JOB_SWARM::SwarmJob* JobManager::AddIncomingJob(PureJob* pNewJob, unsigned char *userData)
{
	++mTasksRemaining;
	pNewJob->Init(&mTasksRemaining);
	return mpContext->CreateSwarmJob(pNewJob,userData,0);
}

void JobManager::Update()
{
	if (mIncomingJobs.size() > 0) {
		SDL_mutexP(mIncomingMutex);
		std::deque<TIncomingJobData>::const_iterator iter = mIncomingJobs.begin();
		std::deque<TIncomingJobData>::const_iterator itEnd = mIncomingJobs.end();
		while(itEnd != iter) {
			AddIncomingJob((*iter).first, (*iter).second);
			++iter;
		}
		mNumTasksSoFar += mIncomingJobs.size();
		mIncomingJobs.clear();
		SDL_mutexV(mIncomingMutex);
	}
	// ok..now we wait until the task remaining counter is zero.
    if ( (mTasksRemaining != mPrevTasksRemaining) || (mPrevTasksRemaining != 0) )
    {
        mpContext->ProcessSwarmJobs();
    }
	mPrevTasksRemaining = mTasksRemaining;
	mMaxNumTasks = std::max( mMaxNumTasks, mPrevTasksRemaining );
}

bool JobManager::JobsRemaining() const { return (mTasksRemaining>0); }

void JobManager::CancelAllPendingJobs()
{
	SDL_mutexP(mIncomingMutex);
	mIncomingJobs.clear();
	SDL_mutexV(mIncomingMutex);
}

void JobManager::Cancel(JOB_SWARM::SwarmJob* pJob)
{
	
	mpContext->Cancel(pJob);
}
