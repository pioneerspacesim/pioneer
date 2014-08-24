// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Graphics.h"
#include "FileSystem.h"
#include "Material.h"
#include "RendererGL2.h"
#include "OS.h"
#include <sstream>
#include <iterator>

namespace Graphics {

static bool initted = false;
Material *vtxColorMaterial;
static int width, height;
static float g_fov = 85.f;
static float g_fovFactor = 1.f;

int GetScreenWidth()
{
	return width;
}

int GetScreenHeight()
{
	return height;
}

float GetFov()
{
	return g_fov;
}

void SetFov(float fov)
{
	g_fov = fov;
	g_fovFactor = 2 * tan(DEG2RAD(g_fov) / 2.f);
}

float GetFovFactor()
{
	return g_fovFactor;
}

static void dump_opengl_value(std::ostream &out, const char *name, GLenum id, int num_elems)
{
	assert(num_elems > 0 && num_elems <= 4);
	assert(name);

	GLdouble e[4];
	glGetDoublev(id, e);

	GLenum err = glGetError();
	if (err == GL_NO_ERROR) {
		out << name << " = " << e[0];
		for (int i = 1; i < num_elems; ++i)
			out << ", " << e[i];
		out << "\n";
	} else {
		while (err != GL_NO_ERROR) {
			if (err == GL_INVALID_ENUM) { out << name << " -- not supported\n"; }
			else { out << name << " -- unexpected error (" << err << ") retrieving value\n"; }
			err = glGetError();
		}
	}
}

static void write_opengl_info(std::ostream &out)
{
	out << "OpenGL version " << glGetString(GL_VERSION);
	out << ", running on " << glGetString(GL_VENDOR);
	out << " " << glGetString(GL_RENDERER) << "\n";

	out << "GLEW version " << glewGetString(GLEW_VERSION) << "\n";

	if (glewIsSupported("GL_VERSION_2_0"))
		out << "Shading language version: " <<  glGetString(GL_SHADING_LANGUAGE_VERSION_ARB) << "\n";

	out << "Available extensions:" << "\n";
	if (glewIsSupported("GL_VERSION_3_0")) {
		GLint numext = 0;
		glGetIntegerv(GL_NUM_EXTENSIONS, &numext);
		for (int i = 0; i < numext; ++i) {
			out << "  " << glGetStringi(GL_EXTENSIONS, i) << "\n";
		}
	}
	else {
		out << "  ";
		std::istringstream ext(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
		std::copy(
			std::istream_iterator<std::string>(ext),
			std::istream_iterator<std::string>(),
			std::ostream_iterator<std::string>(out, "\n  "));
	}

	out << "\nImplementation Limits:\n";

	// first, clear all OpenGL error flags
	while (glGetError() != GL_NO_ERROR) {}

#define DUMP_GL_VALUE(name) dump_opengl_value(out, #name, name, 1)
#define DUMP_GL_VALUE2(name) dump_opengl_value(out, #name, name, 2)

	DUMP_GL_VALUE(GL_MAX_3D_TEXTURE_SIZE);
	DUMP_GL_VALUE(GL_MAX_ATTRIB_STACK_DEPTH);
	DUMP_GL_VALUE(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH);
	DUMP_GL_VALUE(GL_MAX_CLIP_PLANES);
	DUMP_GL_VALUE(GL_MAX_COLOR_ATTACHMENTS_EXT);
	DUMP_GL_VALUE(GL_MAX_COLOR_MATRIX_STACK_DEPTH);
	DUMP_GL_VALUE(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
	DUMP_GL_VALUE(GL_MAX_CUBE_MAP_TEXTURE_SIZE);
	DUMP_GL_VALUE(GL_MAX_DRAW_BUFFERS);
	DUMP_GL_VALUE(GL_MAX_ELEMENTS_INDICES);
	DUMP_GL_VALUE(GL_MAX_ELEMENTS_VERTICES);
	DUMP_GL_VALUE(GL_MAX_EVAL_ORDER);
	DUMP_GL_VALUE(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS);
	DUMP_GL_VALUE(GL_MAX_LIGHTS);
	DUMP_GL_VALUE(GL_MAX_LIST_NESTING);
	DUMP_GL_VALUE(GL_MAX_MODELVIEW_STACK_DEPTH);
	DUMP_GL_VALUE(GL_MAX_NAME_STACK_DEPTH);
	DUMP_GL_VALUE(GL_MAX_PIXEL_MAP_TABLE);
	DUMP_GL_VALUE(GL_MAX_PROJECTION_STACK_DEPTH);
	DUMP_GL_VALUE(GL_MAX_RENDERBUFFER_SIZE_EXT);
	DUMP_GL_VALUE(GL_MAX_SAMPLES_EXT);
	DUMP_GL_VALUE(GL_MAX_TEXTURE_COORDS);
	DUMP_GL_VALUE(GL_MAX_TEXTURE_IMAGE_UNITS);
	DUMP_GL_VALUE(GL_MAX_TEXTURE_LOD_BIAS);
	DUMP_GL_VALUE(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT);
	DUMP_GL_VALUE(GL_MAX_TEXTURE_SIZE);
	DUMP_GL_VALUE(GL_MAX_TEXTURE_STACK_DEPTH);
	DUMP_GL_VALUE(GL_MAX_TEXTURE_UNITS);
	DUMP_GL_VALUE(GL_MAX_VARYING_FLOATS);
	DUMP_GL_VALUE(GL_MAX_VERTEX_ATTRIBS);
	DUMP_GL_VALUE(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
	DUMP_GL_VALUE(GL_MAX_VERTEX_UNIFORM_COMPONENTS);
	DUMP_GL_VALUE(GL_NUM_COMPRESSED_TEXTURE_FORMATS);
	DUMP_GL_VALUE(GL_SAMPLE_BUFFERS);
	DUMP_GL_VALUE(GL_SAMPLES);
	DUMP_GL_VALUE2(GL_ALIASED_LINE_WIDTH_RANGE);
	DUMP_GL_VALUE2(GL_ALIASED_POINT_SIZE_RANGE);
	DUMP_GL_VALUE2(GL_MAX_VIEWPORT_DIMS);
	DUMP_GL_VALUE2(GL_SMOOTH_LINE_WIDTH_RANGE);
	DUMP_GL_VALUE2(GL_SMOOTH_POINT_SIZE_RANGE);

#undef DUMP_GL_VALUE
#undef DUMP_GL_VALUE2
}

Renderer* Init(Settings vs)
{
	assert(!initted);
	if (initted) return 0;

	// no mode set, find an ok one
	if ((vs.width <= 0) || (vs.height <= 0)) {
		const std::vector<VideoMode> modes = GetAvailableVideoModes();
		assert(!modes.empty());

		vs.width = modes.front().width;
		vs.height = modes.front().height;
	}

	WindowSDL *window = new WindowSDL(vs, "Pioneer");
	width = window->GetWidth();
	height = window->GetHeight();

	glewInit();

	{
		std::ostringstream buf;
		write_opengl_info(buf);

		FILE *f = FileSystem::userFiles.OpenWriteStream("opengl.txt", FileSystem::FileSourceFS::WRITE_TEXT);
		if (!f)
			Output("Could not open 'opengl.txt'\n");
		const std::string &s = buf.str();
		fwrite(s.c_str(), 1, s.size(), f);
		fclose(f);
	}

	if (!glewIsSupported("GL_VERSION_2_0") )
		Error("OpenGL Version 2.0 is not supported. Pioneer cannot run on your graphics card.");

	if (!glewIsSupported("GL_ARB_vertex_buffer_object"))
		Error("OpenGL extension ARB_vertex_buffer_object not supported. Pioneer can not run on your graphics card.");

	if (!glewIsSupported("GL_EXT_texture_compression_s3tc"))
		Error("OpenGL extension GL_EXT_texture_compression_s3tc not supported.\nPioneer can not run on your graphics card as it does not support compressed (DXTn/S3TC) format textures.");

	if (glewIsSupported("GL_ARB_texture_compression")) {
		GLint intv[4];
		glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &intv[0]);
		if( intv[0] == 0 )
			Error("GL_NUM_COMPRESSED_TEXTURE_FORMATS is zero.\nPioneer can not run on your graphics card as it does not support compressed (DXTn/S3TC) format textures.");
	}

	Renderer *renderer = new RendererGL2(window, vs);

	Output("Initialized %s\n", renderer->GetName());

	initted = true;

	MaterialDescriptor desc;
	desc.vertexColors = true;
	vtxColorMaterial = renderer->CreateMaterial(desc);
	vtxColorMaterial->IncRefCount();

	return renderer;
}

void Uninit()
{
	delete vtxColorMaterial;
}

static bool operator==(const VideoMode &a, const VideoMode &b) {
	return a.width==b.width && a.height==b.height;
}

std::vector<VideoMode> GetAvailableVideoModes()
{
	std::vector<VideoMode> modes;

	const int num_displays = SDL_GetNumVideoDisplays();
	for(int display_index = 0; display_index < num_displays; display_index++)
	{
		const int num_modes = SDL_GetNumDisplayModes(display_index);

		SDL_Rect display_bounds;
		SDL_GetDisplayBounds(display_index, &display_bounds);

		for (int display_mode = 0; display_mode < num_modes; display_mode++)
		{
			SDL_DisplayMode mode;
			SDL_GetDisplayMode(display_index, display_mode, &mode);
			// insert only if unique resolution
			if( modes.end()==std::find(modes.begin(), modes.end(), VideoMode(mode.w, mode.h)) ) {
				modes.push_back(VideoMode(mode.w, mode.h));
			}
		}
	}
	if( num_displays==0 ) {
		modes.push_back(VideoMode(800, 600));
	}
	return modes;
}

}
