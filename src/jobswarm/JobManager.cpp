#include "libs.h"

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
	int32_t numCoresToUse = std::min( std::max( getNumCores()-2, MIN_THREADS ), MAX_THREADS );
	const int actualNumThreads = std::min( std::max( (iNumThreads<=(0) ? numCoresToUse/*DEF_THREADS*/ : iNumThreads), MIN_THREADS ), MAX_THREADS );
	mpContext = JOB_SWARM::createJobSwarmContext( actualNumThreads );
	mIncomingMutex = THREAD_CONFIG::tc_createThreadMutex();
}

JobManager::~JobManager()
{
	// ok..now we MUST wait until the task remaining counter is zero.
    while ( mpContext && mTasksRemaining != 0 )
    {
        mpContext->processSwarmJobs();
    }

	printf("mMaxNumTasks == (%u)\n", mMaxNumTasks);

	JOB_SWARM::releaseJobSwarmContext(mpContext);
}

unsigned int JobManager::addJobFromThread(PureJob* pNewJob, unsigned char *userData)
{
	unsigned int handleID = mCurrentTaskID;
	++mCurrentTaskID;
	if( INVALID_JOB_HANDLE<=mCurrentTaskID ) {
		mCurrentTaskID = 0;
	}
	// add the data
	mIncomingMutex->lock();
	mIncomingJobs.push_back( TIncomingJobData(pNewJob,userData) );
	mIncomingMutex->unlock();

	return handleID;
}

JOB_SWARM::SwarmJob* JobManager::addJobMainThread(PureJob* pNewJob, unsigned char *userData)
{
	unsigned int handleID = mCurrentTaskID;
	++mCurrentTaskID;
	if( INVALID_JOB_HANDLE<=mCurrentTaskID ) {
		mCurrentTaskID = 0;
	}

	// no mutex, no nothing, just do it immediately
	return addIncomingJob(pNewJob, userData);
}

JOB_SWARM::SwarmJob* JobManager::addIncomingJob(PureJob* pNewJob, unsigned char *userData)
{
	++mTasksRemaining;
	pNewJob->init(&mTasksRemaining);
	return mpContext->createSwarmJob(pNewJob,userData,0);
}

void JobManager::update()
{
	if (mIncomingJobs.size() > 0) {
		mIncomingMutex->lock();
		std::deque<TIncomingJobData>::const_iterator iter = mIncomingJobs.begin();
		std::deque<TIncomingJobData>::const_iterator itEnd = mIncomingJobs.end();
		while(itEnd != iter) {
			addIncomingJob((*iter).first, (*iter).second);
			++iter;
		}
		mNumTasksSoFar += mIncomingJobs.size();
		mIncomingJobs.clear();
		mIncomingMutex->unlock();
	}
	// ok..now we wait until the task remaining counter is zero.
    if ( (mTasksRemaining != mPrevTasksRemaining) || (mPrevTasksRemaining != 0) )
    {
        mpContext->processSwarmJobs();
    }
	mPrevTasksRemaining = mTasksRemaining;
	mMaxNumTasks = std::max( mMaxNumTasks, mPrevTasksRemaining );
}

bool JobManager::jobsRemaining() const { return (mTasksRemaining>0); }

void JobManager::cancel(JOB_SWARM::SwarmJob* pJob)
{
	mpContext->cancel(pJob);
}