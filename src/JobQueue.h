// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef JOBQUEUE_H
#define JOBQUEUE_H

#include "SDL_thread.h"
#include "core/TaskGraph.h"
#include <atomic>
#include <cassert>
#include <deque>
#include <set>
#include <string>
#include <vector>

static const uint32_t MAX_THREADS = 64;

class JobClient;
class JobQueue;

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
	// This is the RAII handle for a queued Job. A job is cancelled when the
	// Job::Handle is destroyed. There is at most one Job::Handle for each Job
	// (non-queued Jobs have no handle). Job::Handle is not copyable only
	// moveable.
	class Handle {
	public:
		Handle() :
			m_id(++s_nextId),
			m_job(nullptr),
			m_queue(nullptr),
			m_client(nullptr) {}
		Handle(Handle &&other);
		Handle &operator=(Handle &&other);
		~Handle();

		Handle(const Handle &) = delete;
		Handle &operator=(const Handle &) = delete;

		bool HasJob() const { return m_job != nullptr; }
		Job *GetJob() const { return m_job; }

		bool operator<(const Handle &other) const { return m_id < other.m_id; }

	private:
		friend class Job;
		friend class AsyncJobQueue;
		friend class SyncJobQueue;
		friend class TaskGraphJobQueueImpl;

		Handle(Job *job, JobQueue *queue, JobClient *client);
		void Unlink();

		static Uint64 s_nextId;

		Uint64 m_id;
		Job *m_job;
		JobQueue *m_queue;
		JobClient *m_client;
	};

public:
	Job() :
		cancelled(false),
		m_handle(nullptr) {}
	virtual ~Job();

	Job(const Job &) = delete;
	Job &operator=(const Job &) = delete;

	virtual void OnRun() = 0;
	virtual void OnFinish() = 0;
	virtual void OnCancel() {}

private:
	friend class AsyncJobQueue;
	friend class SyncJobQueue;
	friend class JobRunner;

	// TaskGraph stuff
	friend class TaskGraph;
	friend class TaskGraphJobQueueImpl;

	void UnlinkHandle();
	const Handle *GetHandle() const { return m_handle; }
	void SetHandle(Handle *handle) { m_handle.store(handle, std::memory_order_release); }
	void ClearHandle() { m_handle = nullptr; }

	std::atomic<bool> cancelled;
	std::atomic<Handle *> m_handle;
};

// the queue management class. create one from the main thread, and feed your
// jobs do it. it will take care of the rest
class JobQueue {
public:
	JobQueue() = default;
	JobQueue(const JobQueue &) = delete;
	JobQueue &operator=(const JobQueue &) = delete;
	virtual ~JobQueue() {}

	// call from the main thread to add a job to the queue. the job should be
	// allocated with new. the queue will delete it once its its completed
	virtual Job::Handle Queue(Job *job, JobClient *client = nullptr) = 0;

	// Call from the main thread to cancel a job.
	// The job will not be run if it is not already executing, and OnFinished
	// will not be called for the job. OnCancel will be called for the job
	// when it is eventually processed in a FinishJobs() call
	virtual void Cancel(Job *job) = 0;

	// call from the main loop. this will call OnFinish for any finished jobs,
	// and then delete all finished and cancelled jobs. returns the number of
	// finished jobs (not cancelled)
	virtual Uint32 FinishJobs() = 0;
};

class SyncJobQueue : public JobQueue {
public:
	SyncJobQueue() = default;
	virtual ~SyncJobQueue();

	virtual Job::Handle Queue(Job *job, JobClient *client = nullptr) override;
	virtual void Cancel(Job *job) override;
	virtual Uint32 FinishJobs() override;

	Uint32 RunJobs(Uint32 count = 1);

private:
	std::deque<Job *> m_queue;
	std::deque<Job *> m_finished;
};

// JobClient is an abstraction to allow transparent management of job handles
class JobClient {
public:
	virtual void Order(Job *job) = 0;
	virtual void RemoveJob(Job::Handle *handle) = 0;
	virtual ~JobClient() {}
};

// JobSet provides an interface for "fire and forget" jobs - call Order with your job,
// and JobSet will keep the handle alive until the job has finished.
class JobSet : public JobClient {
public:
	JobSet(JobQueue *queue) :
		m_queue(queue) {}
	JobSet(JobSet &&other) :
		m_queue(other.m_queue),
		m_jobs(std::move(other.m_jobs)) { other.m_queue = nullptr; }
	JobSet &operator=(JobSet &&other)
	{
		m_queue = other.m_queue;
		m_jobs = std::move(other.m_jobs);
		other.m_queue = nullptr;
		return *this;
	}

	JobSet(const JobSet &) = delete;
	JobSet &operator=(const JobSet &other) = delete;

	virtual void Order(Job *job)
	{
		auto x = m_jobs.insert(m_queue->Queue(job, this));
		(void)x; // suppress unused variable warning
		assert(x.second);
	}
	virtual void RemoveJob(Job::Handle *handle) { m_jobs.erase(*handle); }

	bool IsEmpty() const { return m_jobs.empty(); }

private:
	JobQueue *m_queue;
	std::set<Job::Handle> m_jobs;
};

#endif
