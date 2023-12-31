// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "JobQueue.h"
#include "StringF.h"
#include "profiler/Profiler.h"

void Job::UnlinkHandle()
{
	Handle *handle = m_handle.load(std::memory_order_acquire);
	if (handle)
		handle->Unlink();
}

//virtual
Job::~Job()
{
	UnlinkHandle();
}

//static
Uint64 Job::Handle::s_nextId(0);

Job::Handle::Handle(Job *job, JobQueue *queue, JobClient *client) :
	m_id(++s_nextId),
	m_job(job),
	m_queue(queue),
	m_client(client)
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
	JobClient *client = m_client; // This Job::Handle may be deleted by the client, so clear it before
	m_job = nullptr;
	m_queue = nullptr;
	m_client = nullptr;
	if (client)
		client->RemoveJob(this); // This might delete this Job::Handle, so the object must be cleared before
}

Job::Handle::Handle(Handle &&other) :
	m_id(other.m_id),
	m_job(other.m_job),
	m_queue(other.m_queue),
	m_client(other.m_client)
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

Job::Handle &Job::Handle::operator=(Handle &&other)
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

SyncJobQueue::~SyncJobQueue()
{
	// delete any remaining jobs
	for (Job *j : m_queue)
		delete j;
	for (Job *j : m_finished)
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
		if (!job->cancelled) {
			job->UnlinkHandle();
			job->OnFinish();
			finished++;
		}

		delete job;
	}
	return finished;
}

void SyncJobQueue::Cancel(Job *job)
{
	// Check the waiting list. If it's there then it hasn't run yet. Just forget about it.
	for (std::deque<Job *>::iterator i = m_queue.begin(); i != m_queue.end(); ++i) {
		if (*i == job) {
			i = m_queue.erase(i);
			delete job;
			return;
		}
	}

	// Check the finshed list. If it's there then it can't be cancelled, because
	// it's already finished! We remove it because the caller is saying "I don't care".
	for (std::deque<Job *>::iterator i = m_finished.begin(); i != m_finished.end(); ++i) {
		if (*i == job) {
			i = m_finished.erase(i);
			delete job;
			return;
		}
	}

	// it's running, so we have to tell it to cancel
	job->cancelled = true;
	job->UnlinkHandle();
	job->OnCancel();
}

Uint32 SyncJobQueue::RunJobs(Uint32 count)
{
	PROFILE_SCOPED()
	Uint32 executed = 0;
	assert(count >= 1);
	for (Uint32 i = 0; i < count; ++i) {
		if (m_queue.empty())
			break;

		Job *job = m_queue.front();
		m_queue.pop_front();
		job->OnRun();
		executed++;
		m_finished.push_back(job);
	}
	return executed;
}
