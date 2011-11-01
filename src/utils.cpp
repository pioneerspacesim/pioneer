#include "libs.h"
#include "StringF.h"
#include "gui/Gui.h"

#define PNG_SKIP_SETJMP_CHECK
#include <png.h>

// MinGW targets NT4 by default. We need to set some higher versions to ensure
// that functions we need are available. Specifically, SHCreateDirectoryExA
// requires Windows 2000 and IE5. We include w32api.h to get the symbolic
// constants for these things.
#ifdef __MINGW32__
#	include <w32api.h>
#	ifdef WINVER
#		undef WINVER
#	endif
#	define WINVER Windows2000
#	define _WIN32_IE IE5
#endif

#ifdef _WIN32
// GetPiUserDir() needs these
#include <shlobj.h>
#include <shlwapi.h>
#endif

std::string GetPiUserDir(const std::string &subdir)
{

#if defined(_WIN32)

	char appdata_path[MAX_PATH];
	if (SHGetFolderPathA(0, CSIDL_PERSONAL, 0, SHGFP_TYPE_CURRENT, appdata_path) != S_OK) {
		fprintf(stderr, "Couldn't get user documents folder path\n");
		exit(-1);
	}

	std::string path(appdata_path);
	path += "/Pioneer";

	if (!PathFileExistsA(path.c_str())) {
		if (SHCreateDirectoryExA(0, path.c_str(), 0) != ERROR_SUCCESS) {
			fprintf(stderr, "Couldn't create user game folder '%s'", path.c_str());
			exit(-1);
		}
	}

	if (subdir.length() > 0) {
		path += "/" + subdir;
		if (!PathFileExistsA(path.c_str())) {
			if (SHCreateDirectoryExA(0, path.c_str(), 0) != ERROR_SUCCESS) {
				fprintf(stderr, "Couldn't create user game folder '%s'", path.c_str());
				exit(-1);
			}
		}
	}

	return path + "/";

#else

	std::string path = getenv("HOME");

#ifdef __APPLE__
	path += "/Library/Application Support/Pioneer";
#else
	path += "/.pioneer";
#endif

	struct stat st;
	if (stat(path.c_str(), &st) < 0 && mkdir(path.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) < 0) {
		fprintf(stderr, "Couldn't create user dir '%s': %s\n", path.c_str(), strerror(errno));
		exit(-1);
	}

	if (subdir.length() > 0) {
		path += "/" + subdir;
		if (stat(path.c_str(), &st) < 0 && mkdir(path.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) < 0) {
			fprintf(stderr, "Couldn't create user dir '%s': %s\n", path.c_str(), strerror(errno));
			exit(-1);
		}
	}

	return path + "/";

#endif

}

std::string PiGetDataDir()
{
	return PIONEER_DATA_DIR + std::string("/");
}

FILE *fopen_or_die(const char *filename, const char *mode)
{
	FILE *f = fopen(filename, mode);
	if (!f) {
		fprintf(stderr, "Error: could not open file '%s'\n", filename);
		abort();
	}
	return f;
}

size_t fread_or_die(void* ptr, size_t size, size_t nmemb, FILE* stream, bool allow_truncated)
{
	size_t read_count = fread(ptr, size, nmemb, stream);
	if ((read_count < nmemb) && (!allow_truncated || ferror(stream))) {
		fprintf(stderr, "Error: failed to read file (%s)\n", (feof(stream) ? "truncated" : "read error"));
		abort();
	}
	return read_count;
}

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

std::string join_path(const char *firstbit, ...)
{
	const char *bit;
	va_list ap;
	std::string out = firstbit;
	va_start(ap, firstbit);
	while ((bit = va_arg(ap, const char *))) {
		out = out + "/" + std::string(bit);
	}
	va_end(ap);
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

void Warning(const char *format, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
	fprintf(stderr, "%s\n", buf);
	Gui::Screen::ShowBadError(buf);
}

void SilentWarning(const char *format, ...)
{
	fputs("Warning: ", stderr);
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	fputs("\n", stderr);
}

std::string format_distance(double dist)
{
	if (dist < 1000) {
		return stringf("%0{f.0} m", dist);
	} else if (dist < AU*0.1) {
		return stringf("%0{f.2} km", dist*0.001);
	} else {
		return stringf("%0{f.2} AU", dist/AU);
	}
}

bool is_file(const std::string &filename)
{
	struct stat info;
	if (!stat(filename.c_str(), &info)) {
		if (S_ISREG(info.st_mode)) {
			return true;
		}
	}
	return false;
}

bool is_dir(const std::string &filename)
{
	struct stat info;
	if (!stat(filename.c_str(), &info)) {
		if (S_ISDIR(info.st_mode)) {
			return true;
		}
	}
	return false;
}

void foreach_file_in(const std::string &directory, void (*callback)(const std::string &, const std::string &))
{
	DIR *dir;
	struct dirent *entry;

	if ((dir = opendir(directory.c_str()))==NULL) {
		Error("Could not open directory %s", directory.c_str());
	} 
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] != '.') {
			std::string filename = directory + std::string("/") + entry->d_name;
			(*callback)(entry->d_name, filename);
		}
	}
	closedir(dir);
}

Uint32 ceil_pow2(Uint32 v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

void Screendump(const char* destFile, const int width, const int height)
{
	std::string fname = join_path(GetPiUserDir("screenshots").c_str(), destFile, 0);

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

// returns num bytes consumed, or 0 for end/bogus
int conv_mb_to_wc(Uint32 *chr, const char *src)
{
	unsigned int c = *(reinterpret_cast<const unsigned char*>(src));
	if (!c) { *chr = c; return 0; }
	if (!(c & 0x80)) { *chr = c; return 1; }
	else if (c >= 0xf0) {
		if (!src[1] || !src[2] || !src[3]) return 0;
		c = (c & 0x7) << 18;
		c |= (src[1] & 0x3f) << 12;
		c |= (src[2] & 0x3f) << 6;
		c |= src[3] & 0x3f;
		*chr = c; return 4;
	}
	else if (c >= 0xe0) {
		if (!src[1] || !src[2]) return 0;
		c = (c & 0xf) << 12;
		c |= (src[1] & 0x3f) << 6;
		c |= src[2] & 0x3f;
		*chr = c; return 3;
	}
	else {
		if (!src[1]) return 0;
		c = (c & 0x1f) << 6;
		c |= src[1] & 0x3f;
		*chr = c; return 2;
	}
}

// encode one Unicode code-point as UTF-8
//  chr: the Unicode code-point
//  buf: a character buffer, which must have space for at least 4 bytes
//       (i.e., assigning to buf[3] must be a valid operation)
//  returns: number of bytes in the encoded character
int conv_wc_to_mb(Uint32 chr, char buf[4])
{
	unsigned char *ubuf = reinterpret_cast<unsigned char*>(buf);
	if (chr <= 0x7f) {
		ubuf[0] = chr;
		return 1;
	} else if (chr <= 0x7ff) {
		ubuf[0] = 0xc0 | (chr >> 6);
		ubuf[1] = 0x80 | (chr & 0x3f);
		return 2;
	} else if (chr <= 0xffff) {
		ubuf[0] = 0xe0 | (chr >> 12);
		ubuf[1] = 0x80 | ((chr >> 6) & 0x3f);
		ubuf[2] = 0x80 | (chr & 0x3f);
		return 3;
	} else if (chr <= 0x10fff) {
		ubuf[0] = 0xf0 | (chr >> 18);
		ubuf[1] = 0x80 | ((chr >> 12) & 0x3f);
		ubuf[2] = 0x80 | ((chr >> 6) & 0x3f);
		ubuf[3] = 0x80 | (chr & 0x3f);
		return 4;
	} else {
		assert(0 && "Invalid Unicode code-point.");
		return 0;
	}
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
