// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifdef ENABLE_SERVER_AGENT
#ifndef SERVERAGENT_H
#define SERVERAGENT_H

#include "libs.h"
#include <curl/curl.h>
#include <json/json.h>
#include <map>
#include <queue>

class ServerAgent {
public:
	virtual ~ServerAgent() {}

	typedef sigc::slot<void, const Json &, void *> SuccessCallback;
	typedef sigc::slot<void, const std::string &, void *> FailCallback;

	virtual void Call(const std::string &method, const Json &data, SuccessCallback onSuccess, FailCallback onFail, void *userdata) = 0;

	virtual void ProcessResponses() = 0;

protected:
	static void IgnoreSuccessCallback(const Json &data) {}
	static void IgnoreFailCallback(const std::string &error) {}
};

class NullServerAgent : public ServerAgent {
public:
	virtual void Call(const std::string &method, const Json &data, ServerAgent::SuccessCallback onSuccess = sigc::ptr_fun(&ServerAgent::IgnoreSuccessCallback), ServerAgent::FailCallback onFail = sigc::ptr_fun(&ServerAgent::IgnoreFailCallback), void *userdata = 0);

	virtual void ProcessResponses();

private:
	struct Response {
		Response(FailCallback _onFail, void *_userdata) :
			onFail(_onFail),
			userdata(_userdata)
		{}

		FailCallback onFail;
		void *userdata;
	};

	std::queue<Response> m_queue;
};

class HTTPServerAgent : public ServerAgent {
public:
	HTTPServerAgent(const std::string &endpoint);
	virtual ~HTTPServerAgent();

	virtual void Call(const std::string &method, const Json &data, SuccessCallback onSuccess = sigc::ptr_fun(&ServerAgent::IgnoreSuccessCallback), FailCallback onFail = sigc::ptr_fun(&ServerAgent::IgnoreFailCallback), void *userdata = 0);

	virtual void ProcessResponses();

private:
	struct Request {
		Request(const std::string &_method, const Json &_data, SuccessCallback _onSuccess, FailCallback _onFail, void *_userdata) :
			method(_method),
			data(_data),
			onSuccess(_onSuccess),
			onFail(_onFail),
			userdata(_userdata) {}

		const std::string method;
		const Json data;

		std::string buffer;

		SuccessCallback onSuccess;
		FailCallback onFail;

		void *userdata;
	};

	struct Response {
		Response(SuccessCallback _onSuccess, FailCallback _onFail, void *_userdata) :
			success(false),
			onSuccess(_onSuccess),
			onFail(_onFail),
			userdata(_userdata) {}

		bool success;

		std::string buffer;

		SuccessCallback onSuccess;
		Json data;

		FailCallback onFail;

		void *userdata;
	};

	static int ThreadEntry(void *data);
	void ThreadMain();

	static const std::string &UserAgent();

	static size_t FillRequestBuffer(char *ptr, size_t size, size_t nmemb, void *userdata);
	static size_t FillResponseBuffer(char *ptr, size_t size, size_t nmemb, void *userdata);

	static bool s_initialised;

	const std::string m_endpoint;

	SDL_Thread *m_thread;

	CURL *m_curl;
	curl_slist *m_curlHeaders;

	std::queue<Request> m_requestQueue;
	SDL_mutex *m_requestQueueLock;
	SDL_cond *m_requestQueueCond;

	std::queue<Response> m_responseQueue;
	SDL_mutex *m_responseQueueLock;
};

#endif
#endif // ENABLE_SERVER_AGENT
