// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "DateTime.h"
#include <fmt/core.h>
#include <fmt/printf.h>
#include <sigc++/signal.h>
#include <string_view>

namespace Log {
	enum class Severity : int8_t {
		Fatal = -3,
		Error = -2,
		Warning = -1,
		Info = 0,
		Debug = 1,
		Verbose = 2
	};

	struct Logger {
		~Logger();

		// Handle formatting, indentation, etc.
		// Prefer the Verbose, Info, Warning, etc. functions instead of directly using this one
		void LogLevel(Severity sv, std::string &message);
		void LogLevel(Severity sv, std::string_view message);
		void LogLevel(Severity sv, const char *message);

		bool SetLogFile(std::string filename);
		FILE *GetLogFileHandle() { return file; }

		// Return the severity cutoff at which log messages will be printed to stderr.
		Severity GetSeverity() { return m_maxSeverity; }
		void SetSeverity(Severity sv) { m_maxSeverity = sv; }

		Severity GetFileSeverity() { return m_maxFileSeverity; }
		void SetFileSeverity(Severity sv) { m_maxFileSeverity = sv; }

		Severity GetMsgSeverity() { return m_maxMsgSeverity; }
		void SetMsgSeverity(Severity sv) { m_maxMsgSeverity = sv; }

		void IncreaseIndent() { current_indent += 1; }
		void DecreaseIndent()
		{
			if (current_indent) current_indent -= 1;
		}

		sigc::signal<void, Time::DateTime, Severity, std::string_view> printCallback;

	private:
		void WriteLog(Time::DateTime t, Severity sv, std::string_view msg);

		FILE *file;
		Severity m_maxSeverity = Severity::Info;
		Severity m_maxFileSeverity = Severity::Debug;
		Severity m_maxMsgSeverity = Severity::Error;
		uint8_t current_indent;
		std::string m_logName;
	};

	void SetLog(Logger &log);
	Logger *GetLog();

	inline Severity GetLogLevel() { return GetLog()->GetSeverity(); }
	inline void SetLogLevel(Severity sv) { GetLog()->SetSeverity(sv); }

	void LogOld(Severity sv, const char *message, fmt::printf_args args);
	[[noreturn]] void LogFatalOld(const char *message, fmt::printf_args args);

	void LogInternal(Severity sv, const char *message, fmt::format_args args);
	[[noreturn]] void LogFatalInternal(const char *message, fmt::format_args args);

	void IncreaseIndent();
	void DecreaseIndent();

	template <typename... Args>
	inline void Verbose(const char *message, Args... args)
	{
		LogInternal(Severity::Verbose, message, fmt::make_format_args(args...));
	}

	template <typename... Args>
	inline void Info(const char *message, Args... args)
	{
		LogInternal(Severity::Info, message, fmt::make_format_args(args...));
	}

	template <typename... Args>
	inline void Debug(const char *message, Args... args)
	{
		LogInternal(Severity::Debug, message, fmt::make_format_args(args...));
	}

	template <typename... Args>
	inline void Warning(const char *message, Args... args)
	{
		LogInternal(Severity::Warning, message, fmt::make_format_args(args...));
	}

	template <typename... Args>
	inline void Error(const char *message, Args... args)
	{
		LogInternal(Severity::Error, message, fmt::make_format_args(args...));
	}

	template <typename... Args>
	[[noreturn]] inline void Fatal(const char *message, Args... args)
	{
		LogFatalInternal(message, fmt::make_format_args(args...));
	}

} // namespace Log

// Compatibility functions for old printf-style formatting
template <typename... Args>
inline void Output(const char *message, Args... args)
{
	Log::LogOld(Log::Severity::Info, message, fmt::make_printf_args(args...));
}

template <typename... Args>
inline void Warning(const char *message, Args... args)
{
	Log::LogOld(Log::Severity::Warning, message, fmt::make_printf_args(args...));
}

template <typename... Args>
[[noreturn]] inline void Error(const char *message, Args... args)
{
	Log::LogFatalOld(message, fmt::make_printf_args(args...));
}

template <typename... Args>
inline void DebugMsg(const char *message, Args... args)
{
	Log::LogOld(Log::Severity::Debug, message, fmt::make_printf_args(args...));
}
