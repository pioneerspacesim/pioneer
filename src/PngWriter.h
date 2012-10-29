#ifndef PNGWRITER_H
#define PNGWRITER_H

#include <SDL_stdinc.h>
#include <string>

namespace FileSystem {
	class FileSourceFS;
}

// stride is in bytes (bytes per row)
void write_png(FileSystem::FileSourceFS &fs, const std::string &path, const Uint8 *bytes, int width, int height, int stride, int bytes_per_pixel);

#endif
