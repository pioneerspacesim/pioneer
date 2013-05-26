// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "JobQueue.h"

JobRunner::JobRunner(JobQueue *jq, const uint8_t idx) :
	m_jobQueue(jq),
	m_job(0),
	m_threadIdx(idx)
{
	m_jobLock = SDL_CreateMutex();
	m_threadId = SDL_CreateThread(&JobRunner::Trampoline, this);
}

JobRunner::~JobRunner()
{
	// if we have a job running, cancel it. the worker will return it to the
	// finish queue, where it will be deleted later, so we don't need to do that
	SDL_LockMutex(m_jobLock);
	if (m_job)
		m_job->OnCancel();
	SDL_UnlockMutex(m_jobLock);

	// XXX - AndyC(FluffyFreak) - I'd like to find a better answer for this but I can't see what purpose it serves.
	//		- this is only called on shutting down the program, so there's no case now where we'd be restarting anything.
	// kill the thread. this will block until the thread goes away
	//SDL_KillThread(m_threadId);
}

// entry point for SDL thread. we simply get back onto a method. convenience mostly
int JobRunner::Trampoline(void *data)
{
	JobRunner *jr = static_cast<JobRunner*>(data);
	jr->Main();
	return 0;
}

void JobRunner::Main()
{
	Job *job = m_jobQueue->GetJob();
	while (job) {
		// record the job so we can cancel it in case of premature shutdown
		SDL_LockMutex(m_jobLock);
		m_job = job;
		SDL_UnlockMutex(m_jobLock);

		// run the thing
		job->OnRun();
		m_jobQueue->Finish(job, m_threadIdx);

		SDL_LockMutex(m_jobLock);
		m_job = 0;
		SDL_UnlockMutex(m_jobLock);

		// get a new job. this will block normally, or return null during
		// shutdown
		job = m_jobQueue->GetJob();
	}
}


JobQueue::JobQueue(Uint32 numRunners) :
	m_shutdown(false)
{
	// Want to limit this for now to the maximum number of threads defined in the class
	numRunners = std::min( numRunners, MAX_THREADS );

	m_queueLock = SDL_CreateMutex();
	m_queueWaitCond = SDL_CreateCond();

	for (Uint32 i = 0; i < numRunners; i++) {
		m_finishedLock[i] = SDL_CreateMutex();
		m_runners.push_back(new JobRunner(this, i));
	}
}

JobQueue::~JobQueue()
{
	// flag shutdown. protected by the queue lock for convenience in GetJob
	SDL_LockMutex(m_queueLock);
	m_shutdown = true;
	SDL_UnlockMutex(m_queueLock);

	// broadcast to any waiting runners that they should try (and fail) to get
	// a new job right now
	SDL_CondBroadcast(m_queueWaitCond);

	const uint32_t numThreads = m_runners.size();
	// delete the runners. this will tear down their underlying threads
	for (std::vector<JobRunner*>::iterator i = m_runners.begin(); i != m_runners.end(); ++i)
		delete (*i);

	// delete any remaining jobs
	for (std::deque<Job*>::iterator i = m_queue.begin(); i != m_queue.end(); ++i)
		delete (*i);
	for (uint32_t threadIdx=0; threadIdx<numThreads; threadIdx++) {
		for (std::deque<Job*>::iterator i = m_finished[threadIdx].begin(); i != m_finished[threadIdx].end(); ++i) {
			delete (*i);
		}
	}

	// only us left now, we can clean up and get out of here
	for (uint32_t threadIdx=0; threadIdx<numThreads; threadIdx++) {
		SDL_DestroyMutex(m_finishedLock[threadIdx]);
	}
	SDL_DestroyCond(m_queueWaitCond);
	SDL_DestroyMutex(m_queueLock);
}

void JobQueue::Queue(Job *job)
{
	// push the job onto the queue
	SDL_LockMutex(m_queueLock);
	m_queue.push_back(job);
	SDL_UnlockMutex(m_queueLock);

	// and tell a waiting runner that there's one available
	SDL_CondSignal(m_queueWaitCond);
}

// called by the runner to get a new job
Job *JobQueue::GetJob()
{
	SDL_LockMutex(m_queueLock);

	// loop until a new job is available
	Job *job = 0;
	while (!job) {
		// we're shutting down, so just get out of here
		if (m_shutdown) {
			SDL_UnlockMutex(m_queueLock);
			return 0;
		}

		if (!m_queue.size())
			// no jobs, go to sleep until one arrives
			SDL_CondWait(m_queueWaitCond, m_queueLock);

		else {
			// got one, pop it and return it
			job = m_queue.front();
			m_queue.pop_front();
		}

	}

	SDL_UnlockMutex(m_queueLock);

	return job;
}

// called by the runner when a job completes
void JobQueue::Finish(Job *job, const uint8_t threadIdx)
{
	SDL_LockMutex(m_finishedLock[threadIdx]);
	m_finished[threadIdx].push_back(job);
	SDL_UnlockMutex(m_finishedLock[threadIdx]);
}

// call OnFinish methods for completed jobs, and clean up
Uint32 JobQueue::FinishJobs()
{
	Uint32 finished = 0;

	const uint32_t numRunners = m_runners.size();
	for( uint32_t i=0; i<numRunners ; ++i) {
		SDL_LockMutex(m_finishedLock[i]);
		if( m_finished[i].empty() ) {
			SDL_UnlockMutex(m_finishedLock[i]);
			continue;
		}
		Job *job = m_finished[i].front();
		m_finished[i].pop_front();
		SDL_UnlockMutex(m_finishedLock[i]);

		// if its already been cancelled then its taken care of, so we just forget about it
		if (!job->cancelled) {
			job->OnFinish();
			finished++;
		}

		delete job;
	}

	return finished;
}

void JobQueue::Cancel(Job *job) {
	// lock both queues, so we know that all jobs will stay put
	SDL_LockMutex(m_queueLock);
	const uint32_t numRunners = m_runners.size();
	for( uint32_t i=0; i<numRunners ; ++i) {
		SDL_LockMutex(m_finishedLock[i]);
	}

	// check the waiting list. if its there then it hasn't run yet. just forget about it
	for (std::deque<Job*>::iterator i = m_queue.begin(); i != m_queue.end(); ++i) {
		if (*i == job) {
			i = m_queue.erase(i);
			delete job;
			goto unlock;
		}
	}

	// check the finshed list. if its there then it can't be cancelled, because
	// its alread finished! we remove it because the caller is saying "I don't care"
	for( uint32_t iRunner=0; iRunner<numRunners ; ++iRunner) {
		for (std::deque<Job*>::iterator i = m_queue.begin(); i != m_queue.end(); ++i) {
			if (*i == job) {
				i = m_finished[iRunner].erase(i);
				delete job;
				goto unlock;
			}
		}
	}

	// its running, so we have to tell it to cancel
	job->cancelled = true;
	job->OnCancel();

unlock:
	for( uint32_t i=0; i<numRunners ; ++i) {
		SDL_UnlockMutex(m_finishedLock[i]);
	}
	SDL_UnlockMutex(m_queueLock);
}
