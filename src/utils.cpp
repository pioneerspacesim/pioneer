#ifdef __MINGW32__
#define WINVER 0x0500
#include <w32api.h>
#define _WIN32_IE IE5
#endif

#include <stdlib.h>
#include <math.h>
#include "libs.h"
#include "utils.h"
#include "Gui.h"
#include <string>
#include <map>

#ifdef _WIN32

#ifdef __MINGW32__
#include <dirent.h>
#include <sys/stat.h>
#include <stdexcept>
#define WINSHLWAPI
#else /* !__MINGW32__ */
#include "win32-dirent.h"
#endif

#include <shlobj.h>
#include <shlwapi.h>

#else /* !_WIN32 */
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

std::string GetPiUserDir(const std::string &subdir)
{
#if defined(__MINGW32__)

	/* XXX limiting this implementation to mingw32 for now, because the normal
	 * win32 seems to work fine under msvc and I'm unable to test thoroughly */
	std::string path = getenv("appdata");
	path += "\\Pioneer";

	struct stat st;
	if (stat(path.c_str(), &st) < 0 && mkdir(path.c_str()) < 0) {
		fprintf(stderr, "Couldn't create user dir '%s': %s\n", path.c_str(), strerror(errno));
		exit(-1);
	}

	if (subdir.length() > 0) {
		path += "\\" + subdir;
		if (stat(path.c_str(), &st) < 0 && mkdir(path.c_str()) < 0) {
			fprintf(stderr, "Couldn't create user dir '%s': %s\n", path.c_str(), strerror(errno));
			exit(-1);
		}
	}

	return path + "\\";

#elif defined(_WIN32)
	try {
		TCHAR path[MAX_PATH];
		if(S_OK != SHGetFolderPath(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path))
			throw std::runtime_error("SHGetFolderPath");

		TCHAR temp[MAX_PATH];
		MultiByteToWideChar(CP_ACP, 0, "Pioneer", strlen("Pioneer")+1, (WCHAR*)temp, MAX_PATH);
		if(!PathAppend(path, temp))
			throw std::runtime_error("PathAppend");

		if (subdir != "") {
			MultiByteToWideChar(CP_ACP, 0, subdir.c_str(), subdir.size()+1, (WCHAR*)temp, MAX_PATH);
			if(!PathAppend(path, temp))
				throw std::runtime_error("PathAppend");
		}

		if(!PathFileExists(path) && ERROR_SUCCESS != SHCreateDirectoryEx(0, path, 0))
			throw std::runtime_error("SHCreateDirectoryEx");

		char temp2[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, (const WCHAR*)path, wcslen((const wchar_t*)path)+1, temp2, MAX_PATH, 0, 0);
		return std::string(temp2)+"/";
	}
	catch(const std::exception&) {
		Gui::Screen::ShowBadError("Can't get path to save directory");
		return "";
	}

#else
	const char *homedir = getenv("HOME");
	std::string path = join_path(homedir, ".pioneer", 0);
	DIR *dir = opendir(path.c_str());
	if (!dir) {
		if (mkdir(path.c_str(), 0770) == -1) {
			Gui::Screen::ShowBadError(stringf(128, "Error: Could not create or open '%s'.", path.c_str()).c_str());
		}
	} else {
		closedir(dir);
	}
	if (subdir != "") {
		path = join_path(homedir, ".pioneer", subdir.c_str(), 0);
		dir = opendir(path.c_str());
		if (!dir) {
			if (mkdir(path.c_str(), 0770) == -1) {
				Gui::Screen::ShowBadError(stringf(128, "Error: Could not create or open '%s'.", path.c_str()).c_str());
			}
		} else {
            closedir(dir);
        }
	}
	return path+"/";
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
		printf("Error: could not open file '%s'\n", filename);
		throw MissingFileException();
	}
	return f;
}

std::string format_money(Sint64 money)
{
	char buf[32];
	snprintf(buf, sizeof(buf), "$%.1f", 0.01*(double)money);
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

void strip_cr_lf(char *string)
{
	char *s = string;
	while (*s) {
		if ((*s == '\r') || (*s == '\n')) {
			*s = 0;
			break;
		}
		s++;
	}
}

#define AU		149598000000.0
std::string format_distance(double dist)
{
	if (dist < 1000) {
		return stringf(128, "%.0f m", dist);
	} else if (dist < AU*0.1) {
		return stringf(128, "%.2f km", dist*0.001);
	} else {
		return stringf(128, "%.2f AU", dist/AU);
	}
}

void GetFrustum(Plane planes[6])
{
	GLdouble modelMatrix[16];
	GLdouble projMatrix[16];

	glGetDoublev (GL_MODELVIEW_MATRIX, modelMatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projMatrix);

	matrix4x4d m = matrix4x4d(projMatrix) * matrix4x4d(modelMatrix); 

	// Left clipping plane
	planes[0].a = m[3] + m[0];
	planes[0].b = m[7] + m[4];
	planes[0].c = m[11] + m[8];
	planes[0].d = m[15] + m[12];
	// Right clipping plane
	planes[1].a = m[3] - m[0];
	planes[1].b = m[7] - m[4];
	planes[1].c = m[11] - m[8];
	planes[1].d = m[15] - m[12];
	// Top clipping plane
	planes[2].a = m[3] - m[1];
	planes[2].b = m[7] - m[5];
	planes[2].c = m[11] - m[9];
	planes[2].d = m[15] - m[13];
	// Bottom clipping plane
	planes[3].a = m[3] + m[1];
	planes[3].b = m[7] + m[5];
	planes[3].c = m[11] + m[9];
	planes[3].d = m[15] + m[13];
	// Near clipping plane
	planes[4].a = m[3] + m[2];
	planes[4].b = m[7] + m[6];
	planes[4].c = m[11] + m[10];
	planes[4].d = m[15] + m[14];
	// Far clipping plane
	planes[5].a = m[3] + m[2];
	planes[5].b = m[7] + m[6];
	planes[5].c = m[11] + m[10];
	planes[5].d = m[15] + m[14];

	// Normalize the fuckers
	for (int i=0; i<6; i++) {
		double invlen;
		invlen = 1.0 / sqrt(planes[i].a*planes[i].a + planes[i].b*planes[i].b + planes[i].c*planes[i].c);
		planes[i].a *= invlen;
		planes[i].b *= invlen;
		planes[i].c *= invlen;
		planes[i].d *= invlen;
	}
}

/*
 * So (if you will excuse the C99 compound array literal):
 * string_subst("Hello %1, you smell of %0. Yep, definitely %0.", 2, (std::string[]){"shit","Tom"});
 * will return the string "Hello Tom, you smell of shit. Yep, definitely shit."
 */
std::string string_subst(const char *format, const unsigned int num_args, std::string args[])
{
	std::string out;
	const char *pos = format;

	while (*pos) {
		int i = 0;
		// look for control symbol
		while (pos[i] && (pos[i]!='%')) i++;
		out.append(pos, i);
		if (pos[i]=='%') {
			unsigned int argnum;
			if (pos[++i]=='%') {
				out.push_back('%');
				i++;
			}
			else if (1 == sscanf(&pos[i], "%d", &argnum)) {
				if (argnum >= num_args) out.append("(INVALID ARG)");
				else {
					out.append(args[argnum]);
					while (isdigit(pos[i])) i++;
				}
			} else {
				out.append("(INVALID %code)");
			}
		}
		pos += i;
	}
	return out;
}

static std::map<std::string, GLuint> s_textures;

GLuint util_load_tex_rgba(const char *filename)
{
	GLuint tex = -1;
	std::map<std::string, GLuint>::iterator t = s_textures.find(filename);

	if (t != s_textures.end()) return (*t).second;

	SDL_Surface *s = IMG_Load(filename);

	if (s)
	{
		glGenTextures (1, &tex);
		glBindTexture (GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		switch ( s->format->BitsPerPixel )
		{
		case 32:
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, s->w, s->h, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);
			break;
		case 24:
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, s->w, s->h, GL_RGB, GL_UNSIGNED_BYTE, s->pixels);
			break;
		default:
			printf("Texture '%s' needs to be 24 or 32 bit.\n", filename);
			exit(0);
		}
	
		SDL_FreeSurface(s);

		s_textures[filename] = tex;
	} else {
		Error("IMG_Load: %s\n", IMG_GetError());
	}

	return tex;
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
}
