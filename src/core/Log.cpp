// Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Log.h"
#include "DateTime.h"
#include "FileSystem.h"
#include "SDL_messagebox.h"
#include <SDL.h>
#include <chrono>
#include <cstdio>
#include <map>
#include <string_view>

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>

namespace Log {
	Logger s_defaultLog;

	std::map<Severity, std::string> s_severityNames = {
		{ Severity::Fatal, "Fatal" },
		{ Severity::Error, "Error" },
		{ Severity::Warning, "Warning" },
		{ Severity::Info, "Info" },
		{ Severity::Debug, "Debug" },
		{ Severity::Verbose, "Verbose" }
	};

} // namespace Log

Log::Logger::~Logger()
{
	if (file)
		fclose(file);
}

bool Log::Logger::SetLogFile(std::string filename)
{
	FILE *stream = FileSystem::userFiles.OpenWriteStream(filename, FileSystem::FileSourceFS::WRITE_TEXT);
	if (!stream) {
		LogLevel(Severity::Error, fmt::format("Couldn't open log file {} for writing", filename).c_str());
		return false;
	}

	if (file)
		fclose(file);

	file = stream;
	return true;
}

void Log::Logger::LogLevel(Severity sv, const char *message)
{
	LogLevel(sv, std::string_view(message, strlen(message)));
}

void Log::Logger::LogLevel(Severity sv, std::string &message)
{
	LogLevel(sv, std::string_view(message));
}

inline bool is_space(char c)
{
	return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

void Log::Logger::LogLevel(Severity sv, std::string_view message)
{
	// Convert std::chrono::system_clock (epoch functionally guaranteed to be 1970/1/1-00:00:00)
	// to our DateTime class (epoch defined as 2001/1/1-00:00:00)
	auto t = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now()).time_since_epoch();
	Time::DateTime epoch(1970, 1, 1);
	Time::DateTime time = epoch + Time::TimeDelta(t.count(), Time::Microsecond);

	// FIXME: make StringRange derive from string_view so we can use its overloads *and* format it easily
	while (!message.empty() && is_space(message[0])) {
		message.remove_prefix(1);
	}

	WriteLog(time, sv, message);
}

void Log::Logger::WriteLog(Time::DateTime time, Severity sv, std::string_view msg)
{
	std::string &svName = s_severityNames.at(sv);

	/* Don't output to the console on Windows
	   Builds on /subsystem:WINDOWS will not usually have a console
	   and fmt::print will throw an exception in this case */
#ifndef WIN32
	if (sv <= Severity::Warning) {
		fmt::print(stderr, "{}: {}", svName, msg);
	} else if (sv <= m_maxSeverity) {
		fmt::print(stdout, "{}", msg);
		// flush stdout because it might have a different cache size than stderr
		fflush(stdout);
	}
#endif

	if (!printCallback.empty()) {
		printCallback(time, sv, msg);
	}

	if (file && sv <= m_maxFileSeverity) {
		fmt::print(file, "[{0}]{1:>8}: {2}", time.ToTimeString(), svName, msg);
	}

	if (sv <= m_maxMsgSeverity) {
		uint32_t flags;
		if (sv <= Severity::Error)
			flags = SDL_MESSAGEBOX_ERROR;
		else if (sv <= Severity::Warning)
			flags = SDL_MESSAGEBOX_WARNING;
		else
			flags = SDL_MESSAGEBOX_INFORMATION;

		// convert to std::string since we're going to be pausing the program here for the message box anyways
		SDL_ShowSimpleMessageBox(flags, "Pioneer Warning", std::string(msg).c_str(), 0);
	}
}

Log::Logger *Log::GetLog()
{
	return &s_defaultLog;
}

void Log::SetLog(Logger &log)
{
	s_defaultLog = std::move(log);
}

void Log::IncreaseIndent()
{
	GetLog()->IncreaseIndent();
}

void Log::DecreaseIndent()
{
	GetLog()->DecreaseIndent();
}

void Log::LogInternal(Severity sv, const char *message, fmt::format_args args)
{
	std::string buffer = fmt::vformat(message, args);
	GetLog()->LogLevel(sv, buffer);
}

void Log::LogFatalInternal(const char *message, fmt::format_args args)
{
	std::string buffer = fmt::vformat(message, args);
	GetLog()->LogLevel(Severity::Fatal, buffer);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", buffer.c_str(), 0);
	exit(1);
}

void Log::LogOld(Severity sv, std::string message)
{
	GetLog()->LogLevel(sv, message);
	if (sv == Severity::Warning) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Pioneer warning", message.c_str(), 0);
	}
}

[[noreturn]] void Log::LogFatalOld(std::string message)
{
	GetLog()->LogLevel(Severity::Fatal, message);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Pioneer error", message.c_str(), 0);

	exit(1);
}
