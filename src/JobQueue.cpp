// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "JobQueue.h"
#include "StringF.h"

void Job::UnlinkHandle()
{
	if (m_handle)
		m_handle->Unlink();
}

//virtual
Job::~Job()
{
	UnlinkHandle();
}

//static
unsigned long long Job::Handle::s_nextId(0);

Job::Handle::Handle(Job* job, JobQueue* queue, JobClient* client) : m_id(++s_nextId), m_job(job), m_queue(queue), m_client(client)
{
	assert(!m_job->GetHandle());
	m_job->SetHandle(this);
}

void Job::Handle::Unlink()
{
	if (m_job) {
		assert(m_job->GetHandle() == this);
		m_job->ClearHandle();
	}
	JobClient* client = m_client; // This Job::Handle may be deleted by the client, so clear it before
	m_job = nullptr;
	m_queue = nullptr;
	m_client = nullptr;
	if (client)
		client->RemoveJob(this); // This might delete this Job::Handle, so the object must be cleared before
}

Job::Handle::Handle(Handle&& other) : m_id(other.m_id), m_job(other.m_job), m_queue(other.m_queue), m_client(other.m_client)
{
	if (m_job) {
		assert(m_job->GetHandle() == &other);
		m_job->SetHandle(this);
	}
	other.m_id = 0;
	other.m_job = nullptr;
	other.m_queue = nullptr;
	other.m_client = nullptr;
}

Job::Handle& Job::Handle::operator=(Handle&& other)
{
	if (m_job && m_queue)
		m_queue->Cancel(m_job);
	m_id = other.m_id;
	m_job = other.m_job;
	m_queue = other.m_queue;
	m_client = other.m_client;
	if (m_job) {
		assert(m_job->GetHandle() == &other);
		m_job->SetHandle(this);
	}
	other.m_id = 0;
	other.m_job = nullptr;
	other.m_queue = nullptr;
	other.m_client = nullptr;
	return *this;
}

Job::Handle::~Handle()
{
	if (m_job && m_queue) {
		m_client = nullptr; // Must not tell client to remove the handle, if it's just being destroyed obviously
		m_queue->Cancel(m_job);
	} else {
		m_client = nullptr; // Must not tell client to remove the handle, if it's just being destroyed obviously
		Unlink();
	}
}


AsyncJobQueue::AsyncJobQueue(Uint32 numRunners) :
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

AsyncJobQueue::~AsyncJobQueue()
{
	// flag shutdown. protected by the queue lock for convenience in GetJob
	SDL_LockMutex(m_queueLock);
	m_shutdown = true;
	SDL_UnlockMutex(m_queueLock);

	// broadcast to any waiting runners that they should try (and fail) to get
	// a new job right now
	SDL_CondBroadcast(m_queueWaitCond);

	// Flag each job runner that we're being destroyed (with lock so no one
	// else is running one of our functions). Both the flag and the mutex
	// must be owned by the runner, because we may not exist when it's
	// checked.
	for (std::vector<JobRunner*>::iterator i = m_runners.begin(); i != m_runners.end(); ++i) {
		SDL_LockMutex((*i)->GetQueueDestroyingLock());
		(*i)->SetQueueDestroyed();
		SDL_UnlockMutex((*i)->GetQueueDestroyingLock());
	}

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

Job::Handle AsyncJobQueue::Queue(Job *job, JobClient *client)
{
	Job::Handle handle(job, this, client);

	// push the job onto the queue
	SDL_LockMutex(m_queueLock);
	m_queue.push_back(job);
	SDL_UnlockMutex(m_queueLock);

	// and tell a waiting runner that there's one available
	SDL_CondSignal(m_queueWaitCond);
	return handle;
}

// called by the runner to get a new job
Job *AsyncJobQueue::GetJob()
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
void AsyncJobQueue::Finish(Job *job, const uint8_t threadIdx)
{
	SDL_LockMutex(m_finishedLock[threadIdx]);
	m_finished[threadIdx].push_back(job);
	SDL_UnlockMutex(m_finishedLock[threadIdx]);
}

// call OnFinish methods for completed jobs, and clean up
Uint32 AsyncJobQueue::FinishJobs()
{
	PROFILE_SCOPED()
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

		assert(job);

		// if its already been cancelled then its taken care of, so we just forget about it
		if(!job->cancelled) {
			job->UnlinkHandle();
			job->OnFinish();
			finished++;
		}

		delete job;
	}

	return finished;
}

void AsyncJobQueue::Cancel(Job *job) {
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
		for (std::deque<Job*>::iterator i = m_finished[iRunner].begin(); i != m_finished[iRunner].end(); ++i) {
			if (*i == job) {
				i = m_finished[iRunner].erase(i);
				delete job;
				goto unlock;
			}
		}
	}

	// its running, so we have to tell it to cancel
	job->cancelled = true;
	job->UnlinkHandle();
	job->OnCancel();

unlock:
	for( uint32_t i=0; i<numRunners ; ++i) {
		SDL_UnlockMutex(m_finishedLock[i]);
	}
	SDL_UnlockMutex(m_queueLock);
}

AsyncJobQueue::JobRunner::JobRunner(AsyncJobQueue *jq, const uint8_t idx) :
	m_jobQueue(jq),
	m_job(0),
	m_threadIdx(idx),
	m_queueDestroyed(false)
{
	m_threadName = stringf("Thread %0{d}", m_threadIdx);
	m_jobLock = SDL_CreateMutex();
	m_queueDestroyingLock = SDL_CreateMutex();
	m_threadId = SDL_CreateThread(&JobRunner::Trampoline, m_threadName.c_str(), this);
}

AsyncJobQueue::JobRunner::~JobRunner()
{
	// if we have a job running, cancel it. the worker will return it to the
	// finish queue, where it will be deleted later, so we don't need to do that
	SDL_LockMutex(m_jobLock);
	if (m_job) {
		m_job->UnlinkHandle();
		m_job->OnCancel();
	}
	SDL_UnlockMutex(m_jobLock);

	// XXX - AndyC(FluffyFreak) - I'd like to find a better answer for this but I can't see what purpose it serves.
	//		- this is only called on shutting down the program, so there's no case now where we'd be restarting anything.
	// kill the thread. this will block until the thread goes away
	//SDL_KillThread(m_threadId);
}

// entry point for SDL thread. we simply get back onto a method. convenience mostly
int AsyncJobQueue::JobRunner::Trampoline(void *data)
{
	JobRunner *jr = static_cast<JobRunner*>(data);
	jr->Main();
	return 0;
}

void AsyncJobQueue::JobRunner::Main()
{
	Job *job;

	// Lock to prevent destruction of the queue while calling GetJob.
	SDL_LockMutex(m_queueDestroyingLock);
	if (m_queueDestroyed) {
		SDL_UnlockMutex(m_queueDestroyingLock);
		return;
	}
	job = m_jobQueue->GetJob();
	SDL_UnlockMutex(m_queueDestroyingLock);

	while (job) {
		// record the job so we can cancel it in case of premature shutdown
		SDL_LockMutex(m_jobLock);
		m_job = job;
		SDL_UnlockMutex(m_jobLock);

		// run the thing
		job->OnRun();

		// Lock to prevent destruction of the queue while calling Finish
		SDL_LockMutex(m_queueDestroyingLock);
		if (m_queueDestroyed) {
			SDL_UnlockMutex(m_queueDestroyingLock);
			return;
		}
		m_jobQueue->Finish(job, m_threadIdx);
		SDL_UnlockMutex(m_queueDestroyingLock);

		SDL_LockMutex(m_jobLock);
		m_job = 0;
		SDL_UnlockMutex(m_jobLock);

		// get a new job. this will block normally, or return null during
		// shutdown (Lock to protect against the queue being destroyed
		// during GetJob)
		SDL_LockMutex(m_queueDestroyingLock);
		if (m_queueDestroyed) {
			SDL_UnlockMutex(m_queueDestroyingLock);
			return;
		}
		job = m_jobQueue->GetJob();
		SDL_UnlockMutex(m_queueDestroyingLock);
	}
}

SDL_mutex *AsyncJobQueue::JobRunner::GetQueueDestroyingLock()
{
	return m_queueDestroyingLock;
}

void AsyncJobQueue::JobRunner::SetQueueDestroyed()
{
	m_queueDestroyed = true;
}


SyncJobQueue::~SyncJobQueue()
{
	// delete any remaining jobs
	for (Job* j : m_queue)
		delete j;
	for (Job* j : m_finished)
		delete j;
}

Job::Handle SyncJobQueue::Queue(Job *job, JobClient *client)
{
	Job::Handle handle(job, this, client);
	m_queue.push_back(job);
	return handle;
}

// call OnFinish methods for completed jobs, and clean up
Uint32 SyncJobQueue::FinishJobs()
{
	PROFILE_SCOPED()
	Uint32 finished = 0;

	while (!m_finished.empty()) {
		Job *job = m_finished.front();
		m_finished.pop_front();

		// if its already been cancelled then its taken care of, so we just forget about it
		if(!job->cancelled) {
			job->UnlinkHandle();
			job->OnFinish();
			finished++;
		}

		delete job;
	}
	return finished;
}

void SyncJobQueue::Cancel(Job *job) {
	// check the waiting list. if its there then it hasn't run yet. just forget about it
	for (std::deque<Job*>::iterator i = m_queue.begin(); i != m_queue.end(); ++i) {
		if (*i == job) {
			i = m_queue.erase(i);
			delete job;
			return;
		}
	}

	// check the finshed list. if its there then it can't be cancelled, because
	// its alread finished! we remove it because the caller is saying "I don't care"
	for (std::deque<Job*>::iterator i = m_finished.begin(); i != m_finished.end(); ++i) {
		if (*i == job) {
			i = m_finished.erase(i);
			delete job;
			return;
		}
	}

	// its running, so we have to tell it to cancel
	job->cancelled = true;
	job->UnlinkHandle();
	job->OnCancel();
}

Uint32 SyncJobQueue::RunJobs(Uint32 count)
{
	Uint32 executed = 0;
	assert(count >= 1);
	for (Uint32 i = 0; i < count; ++i) {
		if (m_queue.empty())
			break;

		Job* job = m_queue.front();
		m_queue.pop_front();
		job->OnRun();
		executed++;
		m_finished.push_back(job);
	}
	return executed;
}
