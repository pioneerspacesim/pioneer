// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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

void Screendump(const char* destFile, const int width, const int height)
{
	const std::string dir = "screenshots";
	FileSystem::userFiles.MakeDirectory(dir);
	const std::string fname = FileSystem::JoinPathBelow(dir, destFile);

	// pad rows to 4 bytes, which is the default row alignment for OpenGL
	const int stride = (3*width + 3) & ~3;

	std::vector<Uint8> pixel_data(stride * height);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 4); // never trust defaults
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, &pixel_data[0]);
	glFinish();

	write_png(FileSystem::userFiles, fname, &pixel_data[0], width, height, stride, 3);

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
