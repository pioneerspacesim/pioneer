#include "utils.h"
#include "libs.h"
#include "StringF.h"
#include "gui/Gui.h"
#include "Lang.h"
#include "FileSystem.h"
#include <sstream>

#define PNG_SKIP_SETJMP_CHECK
#include <png.h>

std::string format_money(Sint64 money)
{
	char buf[32];
	snprintf(buf, sizeof(buf), "$%.2f", 0.01*double(money));
	return std::string(buf);
}

class timedate {
public:
	timedate() : hour(0), minute(0), second(0), day(0), month(0), year(3200) {}
	timedate(int stamp) { *this = stamp; }
	timedate &operator=(int stamp);
	std::string fmt_time_date();
	std::string fmt_date();
private:
	int hour, minute, second, day, month, year;

	static const char months[37];
	static const unsigned char days[2][12];
};

// This string of months needs to be made translatable.
// It can always be an array of char with 37 elements,
// as all languages can use just the first three letters
// of the name of each month.
const char timedate::months[37] = "JanFebMarAprMayJunJulAugSepOctNovDec";
const unsigned char timedate::days[2][12] = {
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

timedate &timedate::operator=(int stamp)
{
	int i = int(stamp) % 86400;

	hour   = i / 3600; i %= 3600;
	minute = i /   60; i %=   60;
	second = i;

	i = int(stamp) / 86400 + 1168410; // days since "year 0"

	int n400 = i / 146097; i %= 146097;
	int n100 = i /  36524; i %=  36524;
	int n4   = i /   1461; i %=   1461;
	int n1   = i /    365;

	year = n1 + n4 * 4 + n100 * 100 + n400 * 400 + !(n100 == 4 || n1 == 4);
	day = i % 365 + (n100 == 4 || n1 == 4) * 365;
	int leap = (year % 4 == 0 && year % 100) || (year % 400 == 0);

	month = 0;
	while (day >= days[leap][month])
		day -= days[leap][month++];

	return *this;
}

std::string timedate::fmt_time_date()
{
	char buf[32];
	snprintf(buf, sizeof (buf), "%02d:%02d:%02d %d %.3s %d",
	         hour, minute, second, day + 1, months + month * 3, year);
	return buf;
}

std::string timedate::fmt_date()
{
	char buf[16];
	snprintf(buf, sizeof (buf), "%d %.3s %d",
	         day + 1, months + month * 3, year);
	return buf;
}


std::string format_date(double t)
{
	timedate stamp = int(t);
	return stamp.fmt_time_date();
}

std::string format_date_only(double t)
{
	timedate stamp = int(t);
	return stamp.fmt_date();
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
	fprintf(stderr, "Error: %s\n", buf);
	Gui::Screen::ShowBadError((std::string("Error: ") + buf).c_str());
	abort();
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
	std::string dir = FileSystem::GetUserDir("screenshots");
	FileSystem::rawFileSystem.MakeDirectory(dir);
	std::string fname = FileSystem::JoinPathBelow(dir, destFile);

	// pad rows to 4 bytes, which is the default row alignment for OpenGL
	const int stride = (3*width + 3) & ~3;

	std::vector<png_byte> pixel_data(stride * height);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 4); // never trust defaults
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, &pixel_data[0]);
	glFinish();

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr) {
		fprintf(stderr, "Couldn't create png_write_struct\n");
		return;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, 0);
		fprintf(stderr, "Couldn't create png_info_struct\n");
		return;
	}

	//http://www.libpng.org/pub/png/libpng-1.2.5-manual.html#section-3.1
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fprintf(stderr, "Couldn't set png jump buffer\n");
		return;
	}

	FILE *out = fopen(fname.c_str(), "wb");
	if (!out) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fprintf(stderr, "Couldn't open %s for writing\n", fname.c_str());
		return;
	}

	png_init_io(png_ptr, out);
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);

	png_bytepp rows = new png_bytep[height];

	for (int i = 0; i < height; ++i) {
		rows[i] = reinterpret_cast<png_bytep>(&pixel_data[(height-i-1) * stride]);
	}
	png_set_rows(png_ptr, info_ptr, rows);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, 0);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	delete[] rows;

	fclose(out);
	printf("Screenshot %s saved\n", fname.c_str());
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
					return NULL;

				if (TOLOWER(*rhaystack) != TOLOWER(*rneedle))
					break;
			}
		}
	}
}


#define HEXDUMP_CHUNK 16
void hexdump(const unsigned char *buf, int len)
{
	int count;

	for (int i = 0; i < len; i += HEXDUMP_CHUNK) {
		fprintf(stderr, "0x%06x  ", i);

		count = ((len-i) > HEXDUMP_CHUNK ? HEXDUMP_CHUNK : len-i);

		for (int j = 0; j < count; j++) {
			if (j == HEXDUMP_CHUNK/2) fputc(' ', stderr);
			fprintf(stderr, "%02x ", buf[i+j]);
		}

		for (int j = count; j < HEXDUMP_CHUNK; j++) {
			if (j == HEXDUMP_CHUNK/2) fputc(' ', stderr);
			fprintf(stderr, "   ");
		}

		fputc(' ', stderr);

		for (int j = 0; j < count; j++)
			fprintf(stderr, "%c", isprint(buf[i+j]) ? buf[i+j] : '.');

		fputc('\n', stderr);
	}
}
