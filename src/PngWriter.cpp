// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PngWriter.h"
#include "FileSystem.h"
#include "utils.h"

#define PNG_SKIP_SETJMP_CHECK
#include <png.h>

void write_png(FileSystem::FileSourceFS &fs, const std::string &path, const Uint8 *bytes, int width, int height, int stride, int bytes_per_pixel) {
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr) {
		Output("Couldn't create png_write_struct\n");
		return;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, 0);
		Output("Couldn't create png_info_struct\n");
		return;
	}

	//http://www.libpng.org/pub/png/libpng-1.2.5-manual.html#section-3.1
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		Output("Couldn't set png jump buffer\n");
		return;
	}

	FILE *out = fs.OpenWriteStream(path);
	if (!out) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		Output("Couldn't open '%s/%s' for writing\n", fs.GetRoot().c_str(), path.c_str());
		return;
	}

	int colour_type;
	switch (bytes_per_pixel) {
		case 1: colour_type = PNG_COLOR_TYPE_GRAY; break;
		case 2: colour_type = PNG_COLOR_TYPE_GRAY_ALPHA; break;
		case 3: colour_type = PNG_COLOR_TYPE_RGB; break;
		case 4: colour_type = PNG_COLOR_TYPE_RGB_ALPHA; break;
		default: assert(0); return;
	}

	png_init_io(png_ptr, out);
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, colour_type,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);

	png_bytepp rows = new png_bytep[height];

	for (int i = 0; i < height; ++i) {
		const Uint8 *row = bytes + ((height-i-1) * stride);
		rows[i] = const_cast<Uint8*>(row);
	}
	png_set_rows(png_ptr, info_ptr, rows);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, 0);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	delete[] rows;

	fclose(out);
}
