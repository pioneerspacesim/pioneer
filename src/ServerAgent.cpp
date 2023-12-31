// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifdef ENABLE_SERVER_AGENT

#if defined(_MSC_VER) && !defined(NOMINMAX)
#define NOMINMAX
#endif

#ifdef WIN32
#pragma comment(lib, "libcurl.lib")
#endif

#include "ServerAgent.h"
#include "StringF.h"
#include <curl/curl.h>

void NullServerAgent::Call(const std::string &method, const Json &data, SuccessCallback onSuccess, FailCallback onFail, void *userdata)
{
	m_queue.push(Response(onFail, userdata));
}

void NullServerAgent::ProcessResponses()
{
	while (!m_queue.empty()) {
		Response &resp(m_queue.front());
		resp.onFail("ServerAgent not available", resp.userdata);
		m_queue.pop();
	}
}

bool HTTPServerAgent::s_initialised = false;

HTTPServerAgent::HTTPServerAgent(const std::string &endpoint) :
	m_endpoint(endpoint)
{
	if (!s_initialised)
		curl_global_init(CURL_GLOBAL_ALL);

	m_curl = curl_easy_init();
	//curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1);

	curl_easy_setopt(m_curl, CURLOPT_POST, 1);

	curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, HTTPServerAgent::FillRequestBuffer);
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, HTTPServerAgent::FillResponseBuffer);

	m_curlHeaders = 0;
	m_curlHeaders = curl_slist_append(m_curlHeaders, ("User-agent: " + UserAgent()).c_str());
	m_curlHeaders = curl_slist_append(m_curlHeaders, "Content-type: application/json");
	curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_curlHeaders);

	m_requestQueueLock = SDL_CreateMutex();
	m_requestQueueCond = SDL_CreateCond();

	m_responseQueueLock = SDL_CreateMutex();

	m_thread = SDL_CreateThread(&HTTPServerAgent::ThreadEntry, "HTTPServerAgent", this);
}

HTTPServerAgent::~HTTPServerAgent()
{
	// flush the queue
	SDL_LockMutex(m_requestQueueLock);
	while (m_requestQueue.size() > 0)
		m_requestQueue.pop();
	SDL_UnlockMutex(m_requestQueueLock);

	// signal the thread. empty queue will cause it to exit
	SDL_CondBroadcast(m_requestQueueCond);

	// first thing the queue does is check the queue, so we must
	// not shut down until its done that. it will release the queue
	// lock before it dies, so try to take it
	SDL_LockMutex(m_requestQueueLock);

	// we have the lock, so we know the thread won't doing anything
	// else. we can clean up now
	SDL_DestroyMutex(m_responseQueueLock);
	SDL_DestroyMutex(m_requestQueueLock);
	SDL_DestroyCond(m_requestQueueCond);

	curl_slist_free_all(m_curlHeaders);

	curl_easy_cleanup(m_curl);
}

void HTTPServerAgent::Call(const std::string &method, const Json &data, SuccessCallback onSuccess, FailCallback onFail, void *userdata)
{
	SDL_LockMutex(m_requestQueueLock);
	m_requestQueue.push(Request(method, data, onSuccess, onFail, userdata));
	SDL_UnlockMutex(m_requestQueueLock);

	SDL_CondBroadcast(m_requestQueueCond);
}

void HTTPServerAgent::ProcessResponses()
{
	std::queue<Response> responseQueue;

	// make a copy of the response queue so we can process
	// the response at our leisure
	SDL_LockMutex(m_responseQueueLock);
	responseQueue = m_responseQueue;
	while (!m_responseQueue.empty())
		m_responseQueue.pop();
	SDL_UnlockMutex(m_responseQueueLock);

	while (!responseQueue.empty()) {
		Response &resp = responseQueue.front();

		if (resp.success)
			resp.onSuccess(resp.data, resp.userdata);
		else
			resp.onFail(resp.buffer, resp.userdata);

		responseQueue.pop();
	}
}

int HTTPServerAgent::ThreadEntry(void *data)
{
	reinterpret_cast<HTTPServerAgent *>(data)->ThreadMain();
	return 0;
}

void HTTPServerAgent::ThreadMain()
{
	while (1) {

		// look for requests
		SDL_LockMutex(m_requestQueueLock);

		// if there's no requests, wait until the main thread wakes us
		if (m_requestQueue.empty())
			SDL_CondWait(m_requestQueueCond, m_requestQueueLock);

		// woken up but nothing on the queue means we're being destroyed
		if (m_requestQueue.empty()) {
			// main thread is waiting for this lock, and will start
			// cleanup as soon as it has it
			SDL_UnlockMutex(m_requestQueueLock);

			// might be cleaned up already, so don't do anything else,
			// just get out of here
			return;
		}

		// grab a request
		Request req = m_requestQueue.front();
		m_requestQueue.pop();

		// done with the queue
		SDL_UnlockMutex(m_requestQueueLock);

		Json::FastWriter writer;
		req.buffer = writer.write(req.data);

		Response resp(req.onSuccess, req.onFail, req.userdata);

		curl_easy_setopt(m_curl, CURLOPT_URL, std::string(m_endpoint + "/" + req.method).c_str());
		curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, req.buffer.size());
		curl_easy_setopt(m_curl, CURLOPT_READDATA, &req);
		curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &resp);

		CURLcode rc = curl_easy_perform(m_curl);
		resp.success = rc == CURLE_OK;
		if (!resp.success)
			resp.buffer = std::string("call failed: " + std::string(curl_easy_strerror(rc)));

		if (resp.success) {
			uint32_t code;
			curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &code);
			if (code != 200) {
				resp.success = false;
				resp.buffer = stringf("call returned HTTP status: %0{d}", code);
			}
		}

		if (resp.success) {
			Json::Reader reader;
			resp.success = reader.parse(resp.buffer, resp.data, false);
			if (!resp.success)
				resp.buffer = std::string("JSON parse error: " + reader.getFormattedErrorMessages());
		}

		SDL_LockMutex(m_responseQueueLock);
		m_responseQueue.push(resp);
		SDL_UnlockMutex(m_responseQueueLock);
	}
}

size_t HTTPServerAgent::FillRequestBuffer(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	HTTPServerAgent::Request *req = reinterpret_cast<HTTPServerAgent::Request *>(userdata);
	size_t amount = std::max(size * nmemb, req->buffer.size());
	memcpy(ptr, req->buffer.data(), amount);
	req->buffer.erase(0, amount);
	return amount;
}

size_t HTTPServerAgent::FillResponseBuffer(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	HTTPServerAgent::Response *resp = reinterpret_cast<HTTPServerAgent::Response *>(userdata);
	resp->buffer.append(ptr, size * nmemb);
	return size * nmemb;
}

const std::string &HTTPServerAgent::UserAgent()
{
	static std::string userAgent;
	if (userAgent.size() > 0) return userAgent;

	userAgent = "PioneerServerAgent/" PIONEER_VERSION;

	size_t pos = 0;
	while ((pos = userAgent.find(' ', pos)) != std::string::npos)
		userAgent[pos] = '.';

	if (strlen(PIONEER_EXTRAVERSION))
		userAgent += "." PIONEER_EXTRAVERSION;

	return userAgent;
}

#endif //ENABLE_SERVER_AGENT
