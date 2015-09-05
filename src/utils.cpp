// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "utils.h"
#include "libs.h"
#include "gameconsts.h"
#include "StringF.h"
#include "gui/Gui.h"
#include "Lang.h"
#include "FileSystem.h"
#include "DateTime.h"
#include "PngWriter.h"
#include <sstream>
#include <cmath>
#include <cstdio>

std::string format_money(double cents, bool showCents){
	char *end;                                   // for  error checking
	size_t groupDigits = strtol(Lang::NUMBER_GROUP_NUM, &end, 10);
	assert(*end == 0);

	double money = showCents ? 0.01*cents : roundf(0.01*cents);

	const char *format = (money < 0) ? "-$%.2f" : "$%.2f";
	char buf[64];
	snprintf(buf, sizeof(buf), format, std::abs(money));
	std::string result(buf);

	size_t pos = result.find_first_of('.');      // pos to decimal point

	if(showCents)                                // replace decimal point
		result.replace(pos, 1, Lang::NUMBER_DECIMAL_POINT);
	else                                         // or just remove frac. part
		result.erase(result.begin() + pos, result.end());

	size_t groupMin = strtol(Lang::NUMBER_GROUP_MIN, &end, 10);
	assert(*end == 0);

	if(groupDigits != 0 && std::abs(money) >= groupMin){

		std::string groupSep = std::string(Lang::NUMBER_GROUP_SEP) == " " ?
			"\u00a0" : Lang::NUMBER_GROUP_SEP;     // space should be fixed space

		size_t skip = (money < 0) ? 2 : 1;        // compensate for "$" or "-$"
		while(pos - skip > groupDigits){          // insert thousand seperator
			pos = pos - groupDigits;
			result.insert(pos, groupSep);
		}
	}
	return result;
}

static const char * const MONTH_NAMES[] = {
	Lang::MONTH_JAN,
	Lang::MONTH_FEB,
	Lang::MONTH_MAR,
	Lang::MONTH_APR,
	Lang::MONTH_MAY,
	Lang::MONTH_JUN,
	Lang::MONTH_JUL,
	Lang::MONTH_AUG,
	Lang::MONTH_SEP,
	Lang::MONTH_OCT,
	Lang::MONTH_NOV,
	Lang::MONTH_DEC
};

std::string format_date(double t)
{
	const Time::DateTime dt(t);
	int year, month, day, hour, minute, second;
	dt.GetDateParts(&year, &month, &day);
	dt.GetTimeParts(&hour, &minute, &second);

	char buf[32];
	snprintf(buf, sizeof (buf), "%02d:%02d:%02d %d %s %d",
	         hour, minute, second, day, MONTH_NAMES[month - 1], year);
	return buf;
}

std::string format_date_only(double t)
{
	const Time::DateTime dt(t);
	int year, month, day;
	dt.GetDateParts(&year, &month, &day);

	char buf[16];
	snprintf(buf, sizeof (buf), "%d %s %d", day, MONTH_NAMES[month - 1], year);
	return buf;
}

std::string string_join(std::vector<std::string> &v, std::string sep)
{
	std::vector<std::string>::iterator i = v.begin();
	std::string out;

	while (i != v.end()) {
		out += *i;
		++i;
		if (i != v.end()) out += sep;
	}
	return out;
}

void Error(const char *format, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	Output("error: %s\n", buf);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Pioneer error", buf, 0);

	exit(1);
}

void Warning(const char *format, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	Output("warning: %s\n", buf);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Pioneer warning", buf, 0);
}

void Output(const char *format, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	fputs(buf, stderr);
}

void OpenGLDebugMsg(const char *format, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	fputs(buf, stderr);
}

std::string format_distance(double dist, int precision)
{
	std::ostringstream ss;
	ss.setf(std::ios::fixed, std::ios::floatfield);
	if (dist < 1000) {
		ss.precision(0);
		ss << dist << " m";
	} else {
		ss.precision(precision);
		if (dist < AU*0.1) {
			ss << (dist*0.001) << " km";
		} else {
			ss << (dist/AU) << " AU";
		}
	}
	return ss.str();
}

void write_screenshot(const Graphics::ScreendumpState &sd, const char* destFile)
{
	const std::string dir = "screenshots";
	FileSystem::userFiles.MakeDirectory(dir);
	const std::string fname = FileSystem::JoinPathBelow(dir, destFile);

	write_png(FileSystem::userFiles, fname, sd.pixels.get(), sd.width, sd.height, sd.stride, sd.bpp);

	Output("Screenshot %s saved\n", fname.c_str());
}

// strcasestr() adapted from gnulib
// (c) 2005 FSF. GPL2+

#define TOLOWER(c) (isupper(c) ? tolower(c) : (c))

const char *pi_strcasestr (const char *haystack, const char *needle)
{
	if (!*needle)
		return haystack;

	// cache the first character for speed
	char b = TOLOWER(*needle);

	needle++;
	for (;; haystack++) {
		if (!*haystack)
			return 0;

		if (TOLOWER(*haystack) == b) {
			const char *rhaystack = haystack + 1;
			const char *rneedle = needle;

			for (;; rhaystack++, rneedle++) {
				if (!*rneedle)
					return haystack;

				if (!*rhaystack)
					return 0;

				if (TOLOWER(*rhaystack) != TOLOWER(*rneedle))
					break;
			}
		}
	}
}

std::string SInt64ToStr(Sint64 val)
{
	char str[128];
	sprintf(str, "%" PRIu64, val);
	return str;
}

std::string UInt64ToStr(Uint64 val)
{
	char str[128];
	sprintf(str, "%" PRIu64, val);
	return str;
}

std::string FloatToStr(float val)
{
	// Can't get hexfloats to work.
	//char hex[128]; // Probably don't need such a large char array.
	//std::sprintf(hex, "%a", val);
	//return hex;

	// Lossy method storing as decimal and exponent.
	//char str[128]; // Probably don't need such a large char array.
	//std::sprintf(str, "%.7e", val);
	//return str;

	// Exact representation (but not human readable).
	static_assert(sizeof(float) == 4 || sizeof(float) == 8, "float isn't 4 or 8 bytes");
	if (sizeof(float) == 4)
	{
		uint32_t intVal;
		memcpy(&intVal, &val, 4);
		char str[64];
		sprintf(str, "%" PRIu32, intVal);
		return str;
	}
	else // sizeof(float) == 8
	{
		uint64_t intVal;
		memcpy(&intVal, &val, 8);
		char str[128];
		sprintf(str, "%" PRIu64, intVal);
		return str;
	}
}

std::string DoubleToStr(double val)
{
	// Can't get hexfloats to work.
	//char hex[128]; // Probably don't need such a large char array.
	//std::sprintf(hex, "%la", val);
	//return hex;

	// Lossy method storing as decimal and exponent.
	//char str[128]; // Probably don't need such a large char array.
	//std::sprintf(str, "%.15le", val);
	//return str;

	// Exact representation (but not human readable).
	static_assert(sizeof(double) == 4 || sizeof(double) == 8, "double isn't 4 or 8 bytes");
	if (sizeof(double) == 4)
	{
		uint32_t intVal;
		memcpy(&intVal, &val, 4);
		char str[64];
		sprintf(str, "%" PRIu32, intVal);
		return str;
	}
	else // sizeof(double) == 8
	{
		uint64_t intVal;
		memcpy(&intVal, &val, 8);
		char str[128];
		sprintf(str, "%" PRIu64, intVal);
		return str;
	}
}

std::string AutoToStr(Sint32 val)
{
	char str[64];
	//sprintf(str, "%I32d", val); // Windows
	sprintf(str, "%" PRId32, val);
	return str;
}

std::string AutoToStr(Sint64 val)
{
	char str[128];
	sprintf(str, "%" PRId64, val);
	return str;
}

std::string AutoToStr(float val)
{
	return FloatToStr(val);
}

std::string AutoToStr(double val)
{
	return DoubleToStr(val);
}

Sint64 StrToSInt64(const std::string &str)
{
	Sint64 val;
	sscanf(str.c_str(), "%" SCNd64, &val);
	return val;
}

Uint64 StrToUInt64(const std::string &str)
{
	Uint64 val;
	sscanf(str.c_str(), "%" SCNu64, &val);
	return val;
}

float StrToFloat(const std::string &str)
{
	// Can't get hexfloats to work.
	//return std::strtof(str.c_str(), 0);

	// Can't get hexfloats to work.
	//float val;
	//std::sscanf(str.c_str(), "%a", &val);
	//return val;

	// Lossy method storing as decimal and exponent.
	//float val;
	//std::sscanf(str.c_str(), "%e", &val);
	//return val;

	// Exact representation (but not human readable).
	static_assert(sizeof(float) == 4 || sizeof(float) == 8, "float isn't 4 or 8 bytes");
	if (sizeof(float) == 4)
	{
		uint32_t intVal;
		sscanf(str.c_str(), "%" SCNu32, &intVal);
		float val;
		memcpy(&val, &intVal, 4);
		return val;
	}
	else // sizeof(float) == 8
	{
		uint64_t intVal;
		sscanf(str.c_str(), "%" SCNu64, &intVal);
		float val;
		memcpy(&val, &intVal, 8);
		return val;
	}
}

double StrToDouble(const std::string &str)
{
	// Can't get hexfloats to work.
	//return std::strtod(str.c_str(), 0);

	// Can't get hexfloats to work.
	//double val;
	//std::sscanf(str.c_str(), "%la", &val);
	//return val;

	// Lossy method storing as decimal and exponent.
	//double val;
	//std::sscanf(str.c_str(), "%le", &val);
	//return val;

	// Exact representation (but not human readable).
	static_assert(sizeof(double) == 4 || sizeof(double) == 8, "double isn't 4 or 8 bytes");
	if (sizeof(double) == 4)
	{
		uint32_t intVal;
		sscanf(str.c_str(), "%" SCNu32, &intVal);
		double val;
		memcpy(&val, &intVal, 4);
		return val;
	}
	else // sizeof(double) == 8
	{
		uint64_t intVal;
		sscanf(str.c_str(), "%" SCNu64, &intVal);
		double val;
		memcpy(&val, &intVal, 8);
		return val;
	}
}

void StrToAuto(Sint32 *pVal, const std::string &str)
{
	sscanf(str.c_str(), "%" SCNd32, pVal);
}

void StrToAuto(Sint64 *pVal, const std::string &str)
{
	sscanf(str.c_str(), "%" SCNd64, pVal);
}

void StrToAuto(float *pVal, const std::string &str)
{
	*pVal = StrToFloat(str);
}

void StrToAuto(double *pVal, const std::string &str)
{
	*pVal = StrToDouble(str);
}

static const int HEXDUMP_CHUNK = 16;
void hexdump(const unsigned char *buf, int len)
{
	int count;

	for (int i = 0; i < len; i += HEXDUMP_CHUNK) {
		Output("0x%06x  ", i);

		count = ((len-i) > HEXDUMP_CHUNK ? HEXDUMP_CHUNK : len-i);

		for (int j = 0; j < count; j++) {
			if (j == HEXDUMP_CHUNK/2) Output(" ");
			Output("%02x ", buf[i+j]);
		}

		for (int j = count; j < HEXDUMP_CHUNK; j++) {
			if (j == HEXDUMP_CHUNK/2) Output(" ");
			Output("   ");
		}

		Output(" ");

		for (int j = 0; j < count; j++)
			Output("%c", isprint(buf[i+j]) ? buf[i+j] : '.');

		Output("\n");
	}
}
