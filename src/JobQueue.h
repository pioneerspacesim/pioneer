// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef JOBQUEUE_H
#define JOBQUEUE_H

#include <deque>
#include <vector>
#include <string>
#include "SDL_thread.h"

static const Uint32 MAX_THREADS = 64;

class JobQueue;
class JobRunner;

// represents a single unit of work that you want done
// subclass and implement:
//
// OnRun: called from worker thread, and does the actual stuff you want done.
//        store all your data in the object itself.
//
// OnFinish: called from the main thread once the worker completes the job.
//           this is where you deliver the results from the worker
//
// OnCancel: optional. called from the main thread to tell the job that its
//           results are not wanted. it should arrange for OnRun to return
//           as quickly as possible. OnFinish will not be called for the job
class Job {
public:
	Job() : cancelled(false) {}
	virtual ~Job() {}

	virtual void OnRun() = 0;
	virtual void OnFinish() = 0;
	virtual void OnCancel() {}

private:
	friend class JobQueue;
	bool cancelled;
};


// a runner wraps a single thread, and calls into the queue when its ready for
// a new job. no user-servicable parts inside!
class JobRunner {
public:
	JobRunner(JobQueue *jq, const uint8_t idx);
	~JobRunner();
	SDL_mutex *GetQueueDestroyingLock();
	void SetQueueDestroyed();

private:
	static int Trampoline(void *);
	void Main();

	JobQueue *m_jobQueue;

	Job *m_job;
	SDL_mutex *m_jobLock;
	SDL_mutex *m_queueDestroyingLock;

	SDL_Thread *m_threadId;

	uint8_t m_threadIdx;
	std::string m_threadName;
	bool m_queueDestroyed;
};


// the queue management class. create one from the main thread, and feed your
// jobs do it. it will take care of the rest
class JobQueue {
public:
	// numRunners is the number of jobs to run in parallel. right now its the
	// same as the number of threads, but there's no reason that it has to be
	JobQueue(Uint32 numRunners);
	~JobQueue();

	// call from the main thread to add a job to the queue. the job should be
	// allocated with new. the queue will delete it once its its completed
	void Queue(Job *job);

	// call from the main thread to cancel a job. one of three things will happen
	//
	// - the job hasn't run yet. it will never be run, and neither OnFinished nor
	//   OnCancel will be called. the job will be deleted on the next call to
	//   FinishJobs
	//
	// - the job has finished. neither onFinished not onCancel will be called.
	//   the job will be deleted on the next call to FinishJobs
	//
	// - the job is running. OnCancel will be called
	void Cancel(Job *job);

	// call from the main loop. this will call OnFinish for any finished jobs,
	// and then delete all finished and cancelled jobs. returns the number of
	// finished jobs (not cancelled)
	Uint32 FinishJobs();

private:
	friend class JobRunner;
	Job *GetJob();
	void Finish(Job *job, const uint8_t threadIdx);

	std::deque<Job*> m_queue;
	SDL_mutex *m_queueLock;
	SDL_cond *m_queueWaitCond;

	std::deque<Job*> m_finished[MAX_THREADS];
	SDL_mutex *m_finishedLock[MAX_THREADS];

	std::vector<JobRunner*> m_runners;

	bool m_shutdown;
};

#endif
