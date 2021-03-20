// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "RendererGL.h"
#include "RefCounted.h"
#include "SDL_video.h"
#include "StringF.h"

#include "graphics/Graphics.h"
#include "graphics/Light.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Texture.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Types.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"

#include "GLDebug.h"
#include "MaterialGL.h"
#include "Program.h"
#include "RenderTargetGL.h"
#include "Shader.h"
#include "TextureGL.h"
#include "UniformBuffer.h"
#include "VertexBufferGL.h"

#include "core/Log.h"

#include <cstddef> //for offsetof
#include <iterator>
#include <ostream>
#include <sstream>

namespace Graphics {
	static uint32_t HashRenderStateDesc(const RenderStateDesc &desc);

	const char *gl_framebuffer_error_to_string(GLuint st);

	static bool CreateWindowAndContext(const char *name, const Graphics::Settings &vs, SDL_Window *&window, SDL_GLContext &context)
	{
		PROFILE_SCOPED()
		Uint32 winFlags = 0;

		winFlags |= SDL_WINDOW_OPENGL;
		// We'd like a context that implements OpenGL 3.2 to allow creation of multisampled textures
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
		// Request core profile as we're uninterested in old fixed-function API
		// also cannot initialise 3.x context on OSX with anything but CORE profile
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		// OSX doesn't care about forward-compatible flag, but it's good practice.
		if (vs.gl3ForwardCompatible) SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
		if (vs.enableDebugMessages) SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

		// Don't request a depth/stencil/multisample buffer.
		// We'll render to an offscreen buffer supporting these features and blit to the OS window from there.
		// This is for multiple reasons:
		// - we need a 32-bit float depth buffer
		// - we'd like to be able to render at e.g. 1600x900 and display on a 3200x1800 screen for laptops
		// - changing graphics settings like antialiasing or resolution doesn't require restarting the game (or recreating the window)
		// - we need to MSAA resolve before running post-processing
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		// TODO: verify and enable sRGB-correct rendering through the entire pipeline
		// SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);

		// need at least 24-bit color
		// alpha channel will be present on main render target instead of window surface
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);

		winFlags |= (vs.hidden ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN);
		if (!vs.hidden && vs.fullscreen) // TODO: support for borderless fullscreen and changing window size
			winFlags |= SDL_WINDOW_FULLSCREEN;

		window = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, vs.width, vs.height, winFlags);
		if (!window)
			return false;

		context = SDL_GL_CreateContext(window);
		if (!context) {
			SDL_DestroyWindow(window);
			window = nullptr;
			return false;
		}

		return true;
	}

	static Renderer *CreateRenderer(const Settings &vs)
	{
		PROFILE_SCOPED()
		assert(vs.rendererType == Graphics::RendererType::RENDERER_OPENGL_3x);

		const std::string name("Pioneer");
		SDL_Window *window = nullptr;
		SDL_GLContext glContext = nullptr;

		bool ok = CreateWindowAndContext(name.c_str(), vs, window, glContext);
		if (!ok) {
			Error("Failed to set video mode: %s", SDL_GetError());
			return nullptr;
		}

		SDLSurfacePtr surface = LoadSurfaceFromFile(vs.iconFile);
		if (surface)
			SDL_SetWindowIcon(window, surface.Get());

		SDL_SetWindowTitle(window, vs.title);
		SDL_ShowCursor(0);

		SDL_GL_SetSwapInterval((vs.vsync != 0) ? 1 : 0);

		return new RendererOGL(window, vs, glContext);
	}

	struct LightData {
		Color4f diffuse;
		Color4f specular;
		vector3f position;
		float w;
	};
	static_assert(sizeof(LightData) == 48, "LightData glsl struct has incorrect size/alignment in C++");

	// static method instantiations
	void RendererOGL::RegisterRenderer()
	{
		Graphics::RegisterRenderer(Graphics::RENDERER_OPENGL_3x, CreateRenderer);
	}

	// static member instantiations
	bool RendererOGL::initted = false;
	RendererOGL::DynamicBufferMap RendererOGL::s_DynamicDrawBufferMap;

	// typedefs
	typedef std::vector<std::pair<MaterialDescriptor, OGL::Program *>>::const_iterator ProgramIterator;

	// ----------------------------------------------------------------------------
	RendererOGL::RendererOGL(SDL_Window *window, const Graphics::Settings &vs, SDL_GLContext &glContext) :
		Renderer(window, vs.width, vs.height),
		m_frameNum(0),
		m_numLights(0),
		m_numDirLights(0)
		//the range is very large due to a "logarithmic z-buffer" trick used
		//http://outerra.blogspot.com/2009/08/logarithmic-z-buffer.html
		//http://www.gamedev.net/blog/73/entry-2006307-tip-of-the-day-logarithmic-zbuffer-artifacts-fix/
		,
		m_minZNear(0.001f),
		m_maxZFar(100000000.0f),
		m_useCompressedTextures(false),
		m_activeRenderTarget(0),
		m_activeRenderStateHash(0),
		m_glContext(glContext)
	{
		PROFILE_SCOPED()
		glewExperimental = true;
		GLenum glew_err;
		if ((glew_err = glewInit()) != GLEW_OK)
			Error("GLEW initialisation failed: %s", glewGetErrorString(glew_err));

		// pump this once as glewExperimental is necessary but spews a single error
		glGetError();

		if (vs.enableDebugMessages)
			GLDebug::Enable();

		if (!glewIsSupported("GL_VERSION_3_1")) {
			Error(
				"Pioneer can not run on your graphics card as it does not appear to support OpenGL 3.1\n"
				"Please check to see if your GPU driver vendor has an updated driver - or that drivers are installed correctly.");
		}

		if (!glewIsSupported("GL_EXT_texture_compression_s3tc")) {
			if (glewIsSupported("GL_ARB_texture_compression")) {
				GLint intv[4];
				glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &intv[0]);
				if (intv[0] == 0) {
					Error("GL_NUM_COMPRESSED_TEXTURE_FORMATS is zero.\nPioneer can not run on your graphics card as it does not support compressed (DXTn/S3TC) format textures.");
				}
			} else {
				Error(
					"OpenGL extension GL_EXT_texture_compression_s3tc not supported.\n"
					"Pioneer can not run on your graphics card as it does not support compressed (DXTn/S3TC) format textures.");
			}
		}

		// use floating-point reverse-Z depth buffer to remove the need for depth buffer hacks
		m_useNVDepthRanged = false;
		if (glewIsSupported("GL_ARB_clip_control")) {
			glDepthRange(0.0, 1.0);
			glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
		} else if (glewIsSupported("GL_NV_depth_buffer_float")) {
			m_useNVDepthRanged = true;
			glDepthRangedNV(-1, 1);
		} else {
			Error(
				"Pioneer requires the GL_ARB_clip_control or GL_NV_depth_buffer_float OpenGL extensions.\n"
				"These extensions are not supported by your graphics card or graphics driver version.\n"
				"Please check to see if your GPU driver vendor has an updated driver - or that drivers are installed correctly.");
		}

		const char *ver = reinterpret_cast<const char *>(glGetString(GL_VERSION));
		if (vs.gl3ForwardCompatible && strstr(ver, "9.17.10.4229")) {
			Warning("Driver needs GL3ForwardCompatible=0 in config.ini to display billboards (stars, navlights etc.)");
		}

		TextureBuilder::Init();

		const bool useDXTnTextures = vs.useTextureCompression;
		m_useCompressedTextures = useDXTnTextures;

		const bool useAnisotropicFiltering = vs.useAnisotropicFiltering;
		m_useAnisotropicFiltering = useAnisotropicFiltering;

		//XXX bunch of fixed function states here!
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		// use floating-point reverse-Z depth buffer to remove the need for depth buffer hacks
		glDepthFunc(GL_GREATER);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glEnable(GL_PROGRAM_POINT_SIZE);
		if (!vs.gl3ForwardCompatible) glEnable(GL_POINT_SPRITE); // GL_POINT_SPRITE hack for compatibility contexts

		glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);
		glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);

		glClearDepth(0.0); // clear to 0.0 for use with reverse-Z
		SetClearColor(Color4f(0.f, 0.f, 0.f, 0.f));
		SetViewport(Viewport(0, 0, m_width, m_height));

		// check enum PrimitiveType matches OpenGL values
		assert(POINTS == GL_POINTS);
		assert(LINE_SINGLE == GL_LINES);
		assert(LINE_LOOP == GL_LINE_LOOP);
		assert(LINE_STRIP == GL_LINE_STRIP);
		assert(TRIANGLES == GL_TRIANGLES);
		assert(TRIANGLE_STRIP == GL_TRIANGLE_STRIP);
		assert(TRIANGLE_FAN == GL_TRIANGLE_FAN);

		RenderTargetDesc windowTargetDesc(
			m_width, m_height,
			// TODO: sRGB format for render target?
			TextureFormat::TEXTURE_RGBA_8888,
			TextureFormat::TEXTURE_DEPTH,
			false, vs.requestedSamples);
		m_windowRenderTarget = static_cast<OGL::RenderTarget *>(CreateRenderTarget(windowTargetDesc));

		m_windowRenderTarget->Bind();
		if (!m_windowRenderTarget->CheckStatus()) {
			GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			Log::Fatal("Pioneer window render target is invalid. (Error: {})\n"
				"Does your graphics driver support multisample anti-aliasing?\n"
				"If this issue persists, try setting AntiAliasingMode=0 in your config file.\n",
				gl_framebuffer_error_to_string(status));
		}

		SetRenderTarget(nullptr);

		m_drawUniformBuffers.reserve(8);
		GetDrawUniformBuffer(0);

		m_lightUniformBuffer.Reset(new OGL::UniformBuffer(sizeof(LightData) * TOTAL_NUM_LIGHTS, BUFFER_USAGE_DYNAMIC));
	}

	RendererOGL::~RendererOGL()
	{
		for (auto &buffer : m_drawUniformBuffers) {
			buffer.reset();
		}

		m_lightUniformBuffer.Reset();

		s_DynamicDrawBufferMap.clear();

		// HACK ANDYC - this crashes when shutting down? They'll be released anyway right?
		while (!m_shaders.empty()) {
			delete m_shaders.back().second;
			m_shaders.pop_back();
		}

		if (m_windowRenderTarget->m_active)
			m_windowRenderTarget->Unbind();
		delete m_windowRenderTarget;

		SDL_GL_DeleteContext(m_glContext);
	}

	static const char *gl_error_to_string(GLenum err)
	{
		switch (err) {
		case GL_NO_ERROR: return "(no error)";
		case GL_INVALID_ENUM: return "invalid enum";
		case GL_INVALID_VALUE: return "invalid value";
		case GL_INVALID_OPERATION: return "invalid operation";
		case GL_INVALID_FRAMEBUFFER_OPERATION: return "invalid framebuffer operation";
		case GL_OUT_OF_MEMORY: return "out of memory";
		default: return "(unknown error)";
		}
	}

	const char *gl_framebuffer_error_to_string(GLuint st)
	{
		switch (st) {
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			return "INCOMPLETE_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			return "INCOMPLETE_MISSING_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			return "INCOMPLETE_DRAW_BUFFER";
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			return "INCOMPLETE_READ_BUFFER";
		case GL_FRAMEBUFFER_UNSUPPORTED:
			return "FRAMEBUFFER_UNSUPPORTED";
		default:
			return "Unknown reason";
		}
	}

	static void dump_and_clear_opengl_errors(std::ostream &out, GLenum first_error = GL_NO_ERROR)
	{
		GLenum err = ((first_error == GL_NO_ERROR) ? glGetError() : first_error);
		if (err != GL_NO_ERROR) {
			out << "errors: ";
			do {
				out << gl_error_to_string(err) << " ";
				err = glGetError();
			} while (err != GL_NO_ERROR);
			out << std::endl;
		}
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
				if (err == GL_INVALID_ENUM) {
					out << name << " -- not supported\n";
				} else {
					out << name << " -- unexpected error (" << err << ") retrieving value\n";
				}
				err = glGetError();
			}
		}
	}

	void RendererOGL::WriteRendererInfo(std::ostream &out) const
	{
		out << "OpenGL version " << glGetString(GL_VERSION);
		out << ", running on " << glGetString(GL_VENDOR);
		out << " " << glGetString(GL_RENDERER) << "\n";

		out << "Available extensions:"
			<< "\n";
		if (glewIsSupported("GL_VERSION_3_1")) {
			out << "Shading language version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
			GLint numext = 0;
			glGetIntegerv(GL_NUM_EXTENSIONS, &numext);
			for (int i = 0; i < numext; ++i) {
				out << "  " << glGetStringi(GL_EXTENSIONS, i) << "\n";
			}
		} else {
			out << "  ";
			std::istringstream ext(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
			std::copy(
				std::istream_iterator<std::string>(ext),
				std::istream_iterator<std::string>(),
				std::ostream_iterator<std::string>(out, "\n  "));
		}

		out << "\nImplementation Limits:\n";

		// first, clear all OpenGL error flags
		dump_and_clear_opengl_errors(out);

#define DUMP_GL_VALUE(name) dump_opengl_value(out, #name, name, 1)
#define DUMP_GL_VALUE2(name) dump_opengl_value(out, #name, name, 2)

		DUMP_GL_VALUE(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
		DUMP_GL_VALUE(GL_MAX_CUBE_MAP_TEXTURE_SIZE);
		DUMP_GL_VALUE(GL_MAX_DRAW_BUFFERS);
		DUMP_GL_VALUE(GL_MAX_ELEMENTS_INDICES);
		DUMP_GL_VALUE(GL_MAX_ELEMENTS_VERTICES);
		DUMP_GL_VALUE(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS);
		DUMP_GL_VALUE(GL_MAX_TEXTURE_IMAGE_UNITS);
		DUMP_GL_VALUE(GL_MAX_TEXTURE_LOD_BIAS);
		DUMP_GL_VALUE(GL_MAX_TEXTURE_SIZE);
		DUMP_GL_VALUE(GL_MAX_VERTEX_ATTRIBS);
		DUMP_GL_VALUE(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
		DUMP_GL_VALUE(GL_MAX_VERTEX_UNIFORM_COMPONENTS);
		DUMP_GL_VALUE(GL_NUM_COMPRESSED_TEXTURE_FORMATS);
		DUMP_GL_VALUE(GL_SAMPLE_BUFFERS);
		DUMP_GL_VALUE(GL_SAMPLES);
		DUMP_GL_VALUE2(GL_ALIASED_LINE_WIDTH_RANGE);
		DUMP_GL_VALUE2(GL_MAX_VIEWPORT_DIMS);
		DUMP_GL_VALUE2(GL_SMOOTH_LINE_WIDTH_RANGE);
		DUMP_GL_VALUE2(GL_SMOOTH_POINT_SIZE_RANGE);

#undef DUMP_GL_VALUE
#undef DUMP_GL_VALUE2

		// enumerate compressed texture formats
		{
			dump_and_clear_opengl_errors(out);
			out << "\nCompressed texture formats:\n";

			GLint nformats;
			GLint formats[128]; // XXX 128 should be enough, right?

			glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &nformats);
			GLenum err = glGetError();
			if (err != GL_NO_ERROR) {
				out << "Get NUM_COMPRESSED_TEXTURE_FORMATS failed\n";
				dump_and_clear_opengl_errors(out, err);
			} else {
				assert(nformats >= 0 && nformats < int(COUNTOF(formats)));
				glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, formats);
				err = glGetError();
				if (err != GL_NO_ERROR) {
					out << "Get COMPRESSED_TEXTURE_FORMATS failed\n";
					dump_and_clear_opengl_errors(out, err);
				} else {
					for (int i = 0; i < nformats; ++i) {
						out << stringf("  %0{x#}\n", unsigned(formats[i]));
					}
				}
			}
		}
		// one last time
		dump_and_clear_opengl_errors(out);
	}

	int RendererOGL::GetMaximumNumberAASamples() const
	{
		GLint value = 0;
		glGetIntegerv(GL_MAX_SAMPLES, &value);
		return value;
	}

	bool RendererOGL::GetNearFarRange(float &near_, float &far_) const
	{
		near_ = m_minZNear;
		far_ = m_maxZFar;
		return true;
	}

	bool RendererOGL::BeginFrame()
	{
		PROFILE_SCOPED()
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_frameNum++;
		return true;
	}

	bool RendererOGL::EndFrame()
	{
		PROFILE_SCOPED()
		uint32_t used_tex2d = 0;
		uint32_t used_texCube = 0;
		uint32_t used_texArray2d = 0;
		uint32_t num_tex2d = 0;
		uint32_t num_texCube = 0;
		uint32_t num_texArray2d = 0;

		for (const auto &pair : GetTextureCache()) {
			auto *texture = pair.second->Get();

			uint32_t size = texture->GetTextureMemSize();
			switch (texture->GetDescriptor().type) {
			default:
				assert(0);
			case TextureType::TEXTURE_2D:
				used_tex2d += size;
				num_tex2d++;
				break;
			case TextureType::TEXTURE_CUBE_MAP:
				used_texCube += size;
				num_texCube++;
				break;
			case TextureType::TEXTURE_2D_ARRAY:
				used_texArray2d += size;
				num_texArray2d++;
				break;
			}
		}

		auto &stat = GetStats();
		stat.SetStatCount(Stats::STAT_NUM_TEXTURE2D, num_tex2d);
		stat.SetStatCount(Stats::STAT_MEM_TEXTURE2D, used_tex2d);
		stat.SetStatCount(Stats::STAT_NUM_TEXTUREARRAY2D, num_texArray2d);
		stat.SetStatCount(Stats::STAT_MEM_TEXTUREARRAY2D, used_texArray2d);
		stat.SetStatCount(Stats::STAT_NUM_TEXTURECUBE, num_texCube);
		stat.SetStatCount(Stats::STAT_MEM_TEXTURECUBE, used_texCube);

		uint32_t numAllocs = 0;
		for (auto &buffer : m_drawUniformBuffers) {
			numAllocs += buffer->NumAllocs();
			buffer->Reset();
		}

		stat.SetStatCount(Stats::STAT_DRAW_UNIFORM_BUFFER_INUSE, uint32_t(m_drawUniformBuffers.size()));
		stat.SetStatCount(Stats::STAT_DRAW_UNIFORM_BUFFER_ALLOCS, numAllocs);

		// evict temporary vertex buffers if they haven't been used in at least 30 frames
		for (auto iter = s_DynamicDrawBufferMap.begin(); iter != s_DynamicDrawBufferMap.end();) {
			if (iter->lastFrameUsed < (std::max(m_frameNum, 30UL) - 30))
				iter = s_DynamicDrawBufferMap.erase(iter);
			else
				iter++;
		}

		stat.SetStatCount(Stats::STAT_DYNAMIC_DRAW_BUFFER_INUSE, s_DynamicDrawBufferMap.size());

		return true;
	}

	static std::string glerr_to_string(GLenum err)
	{
		switch (err) {
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";
		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";
		default:
			return stringf("Unknown error 0x0%0{x}", err);
		}
	}

	void RendererOGL::CheckErrors(const char *func, const int line)
	{
#ifndef PIONEER_PROFILER
		GLenum err = glGetError();
		if (err) {
			// static-cache current err that sparked this
			static GLenum s_prevErr = GL_NO_ERROR;
			const bool showWarning = (s_prevErr != err);
			s_prevErr = err;
			// now build info string
			std::stringstream ss;
			assert(func != nullptr && line >= 0);
			ss << "OpenGL error(s) during frame:\n";
			ss << "In function " << std::string(func) << "\nOn line " << std::to_string(line) << "\n";
			while (err != GL_NO_ERROR) {
				ss << glerr_to_string(err) << '\n';
				err = glGetError();
				if (err == GL_OUT_OF_MEMORY) {
					ss << "Out-of-memory on graphics card." << std::endl
					   << "Recommend enabling \"Compress Textures\" in game options." << std::endl
					   << "Also try reducing City and Planet detail settings." << std::endl;
				}
#ifdef _WIN32
				else if (err == GL_INVALID_OPERATION) {
					ss << "Invalid operations can occur if you are using overlay software." << std::endl
					   << "Such as FRAPS, RivaTuner, MSI Afterburner etc." << std::endl
					   << "Please try disabling this kind of software and testing again, thankyou." << std::endl;
				}
#endif
			}
			// show warning dialog or just log to output
			if (showWarning)
				Log::Warning("{}", ss.str());
			else
				Log::Output("{}", ss.str());
		}
#endif
	}

	bool RendererOGL::SwapBuffers()
	{
		PROFILE_SCOPED()
		CheckRenderErrors(__FUNCTION__, __LINE__);

		// Make sure we set the active FBO to our "default" window target
		SetRenderTarget(nullptr);

		// TODO(sturnclaw): handle upscaling to higher-resolution screens
		// we'll need an intermediate target to resolve to, resolve and rescale are mutually exclusive
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_windowRenderTarget->m_fbo);

		SDL_GL_SwapWindow(m_window);
		m_stats.NextFrame();
		return true;
	}

	void ApplyRenderState(const RenderStateDesc &rsd)
	{
		switch (rsd.blendMode) {
		case BLEND_SOLID:
			glDisable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ZERO);
			break;
		case BLEND_ADDITIVE:
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			break;
		case BLEND_ALPHA:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case BLEND_ALPHA_ONE:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
		case BLEND_ALPHA_PREMULT:
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case BLEND_SET_ALPHA:
			glEnable(GL_BLEND);
			glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ZERO);
			break;
		case BLEND_DEST_ALPHA:
			glEnable(GL_BLEND);
			glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
		default:
			break;
		}

		if (rsd.cullMode == CULL_BACK) {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		} else if (rsd.cullMode == CULL_FRONT) {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
		} else {
			glDisable(GL_CULL_FACE);
		}

		if (rsd.depthTest)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

		if (rsd.depthWrite)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);
	}

	void RendererOGL::SetRenderState(const RenderStateDesc &rsd)
	{
		uint32_t hash = HashRenderStateDesc(rsd);
		if (hash != m_activeRenderStateHash) {
			m_activeRenderStateHash = hash;
			ApplyRenderState(rsd); // TODO: should we track state more granularly?
		}
		CheckRenderErrors(__FUNCTION__, __LINE__);
	}

	bool RendererOGL::SetRenderTarget(RenderTarget *rt)
	{
		PROFILE_SCOPED()
		if (rt) {
			if (m_activeRenderTarget)
				m_activeRenderTarget->Unbind();
			else
				m_windowRenderTarget->Unbind();

			static_cast<OGL::RenderTarget *>(rt)->Bind();
		} else {
			if (m_activeRenderTarget)
				m_activeRenderTarget->Unbind();

			m_windowRenderTarget->Bind();
		}

		m_activeRenderTarget = static_cast<OGL::RenderTarget *>(rt);
		CheckRenderErrors(__FUNCTION__, __LINE__);

		return true;
	}

	bool RendererOGL::ClearScreen()
	{
		m_activeRenderStateHash = 0;
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		CheckRenderErrors(__FUNCTION__, __LINE__);

		return true;
	}

	bool RendererOGL::ClearDepthBuffer()
	{
		m_activeRenderStateHash = 0;
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glClear(GL_DEPTH_BUFFER_BIT);
		CheckRenderErrors(__FUNCTION__, __LINE__);

		return true;
	}

	bool RendererOGL::SetClearColor(const Color &c)
	{
		glClearColor(c.r, c.g, c.b, c.a);
		return true;
	}

	bool RendererOGL::SetViewport(Viewport v)
	{
		m_viewport = v;
		glViewport(v.x, v.y, v.w, v.h);
		return true;
	}

	Viewport RendererOGL::GetViewport() const
	{
		return m_viewport;
	}

	bool RendererOGL::SetTransform(const matrix4x4f &m)
	{
		m_modelViewMat = m;
		return true;
	}

	matrix4x4f RendererOGL::GetTransform() const
	{
		return m_modelViewMat;
	}

	bool RendererOGL::SetPerspectiveProjection(float fov, float aspect, float near_, float far_)
	{
		PROFILE_SCOPED()

		Graphics::SetFov(fov);
		SetProjection(matrix4x4f::PerspectiveMatrix(DEG2RAD(fov), aspect, near_, far_));
		return true;
	}

	bool RendererOGL::SetOrthographicProjection(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax)
	{
		PROFILE_SCOPED()
		const matrix4x4f orthoMat = matrix4x4f::OrthoFrustum(xmin, xmax, ymin, ymax, zmin, zmax);
		SetProjection(orthoMat);
		return true;
	}

	bool RendererOGL::SetProjection(const matrix4x4f &m)
	{
		m_projectionMat = m;
		return true;
	}

	matrix4x4f RendererOGL::GetProjection() const
	{
		return m_projectionMat;
	}

	bool RendererOGL::SetWireFrameMode(bool enabled)
	{
		glPolygonMode(GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
		return true;
	}

	bool RendererOGL::SetLightIntensity(Uint32 numlights, const float *intensity)
	{
		numlights = std::min(numlights, m_numLights);
		for (Uint32 i = 0; i < numlights; i++) {
			m_lights[i].SetIntensity(intensity[i]);
		}

		return true;
	}

	bool RendererOGL::SetLights(Uint32 numlights, const Light *lights)
	{
		numlights = std::min(numlights, TOTAL_NUM_LIGHTS);
		if (numlights < 1) {
			m_numLights = 0;
			m_numDirLights = 0;
			return false;
		}

		m_numLights = numlights;
		m_numDirLights = 0;
		// ScopedMap will be released at the end of the function
		auto lightData = m_lightUniformBuffer->Map<LightData>(BufferMapMode::BUFFER_MAP_WRITE);
		assert(lightData.isValid());

		for (Uint32 i = 0; i < numlights; i++) {
			const Light &l = lights[i];
			m_lights[i].SetPosition(l.GetPosition());
			m_lights[i].SetDiffuse(l.GetDiffuse());
			m_lights[i].SetSpecular(l.GetSpecular());

			if (l.GetType() == Light::LIGHT_DIRECTIONAL)
				m_numDirLights++;

			assert(m_numDirLights <= TOTAL_NUM_LIGHTS);

			// Update the GPU-side light data buffer
			LightData &gpuLight = lightData.data()[i];
			gpuLight.diffuse = l.GetDiffuse().ToColor4f();
			gpuLight.specular = l.GetSpecular().ToColor4f();
			gpuLight.position = l.GetPosition();
			gpuLight.w = l.GetType() == Light::LIGHT_DIRECTIONAL ? 0.0f : 1.0f;
		}

		return true;
	}

	bool RendererOGL::SetAmbientColor(const Color &c)
	{
		m_ambient = c;
		return true;
	}

	bool RendererOGL::SetScissor(bool enabled, const vector2f &pos, const vector2f &size)
	{
		if (enabled) {
			glScissor(pos.x, pos.y, size.x, size.y);
			glEnable(GL_SCISSOR_TEST);
		} else {
			glDisable(GL_SCISSOR_TEST);
		}
		return true;
	}

	bool RendererOGL::DrawBuffer(const VertexArray *v, Material *m)
	{
		PROFILE_SCOPED()
		constexpr double BUFFER_WASTE_PROPORTION = 2.0;

		if (v->IsEmpty()) return false;

		const AttributeSet attrs = v->GetAttributeSet();
		MeshObject *meshObject;

		auto iter = std::find_if(s_DynamicDrawBufferMap.begin(), s_DynamicDrawBufferMap.end(), [&](const DynamicBufferData &a) {
			return a.attrs == attrs && a.vertexCount >= v->GetNumVerts() && a.vertexCount <= v->GetNumVerts() * BUFFER_WASTE_PROPORTION;
		});

		if (iter == s_DynamicDrawBufferMap.end()) {
			auto desc = VertexBufferDesc::FromAttribSet(v->GetAttributeSet());
			desc.numVertices = v->GetNumVerts() * BUFFER_WASTE_PROPORTION;
			desc.usage = BUFFER_USAGE_DYNAMIC;
			meshObject = CreateMeshObject(CreateVertexBuffer(desc), nullptr);
			s_DynamicDrawBufferMap.push_back(DynamicBufferData{ attrs, RefCountedPtr<MeshObject>(meshObject), m_frameNum, desc.numVertices });
			GetStats().AddToStatCount(Stats::STAT_DYNAMIC_DRAW_BUFFER_CREATED, 1);
		} else {
			meshObject = iter->mesh.Get();
			iter->lastFrameUsed = m_frameNum;
		}

		meshObject->GetVertexBuffer()->Populate(*v);
		const bool res = DrawMesh(meshObject, m);
		CheckRenderErrors(__FUNCTION__, __LINE__);

		return res;
	}

	bool RendererOGL::DrawMesh(MeshObject *mesh, Material *mat)
	{
		PROFILE_SCOPED()
		SetRenderState(mat->GetStateDescriptor());
		mat->Apply();
		CheckRenderErrors(__FUNCTION__, __LINE__);

		int numElems = mesh->GetIndexBuffer() ? mesh->GetIndexBuffer()->GetIndexCount() : mesh->GetVertexBuffer()->GetSize();
		PrimitiveType pt = mat->GetStateDescriptor().primitiveType;

		mesh->Bind();
		if (mesh->GetIndexBuffer())
			glDrawElements(pt, numElems, GL_UNSIGNED_INT, nullptr);
		else
			glDrawArrays(pt, 0, numElems);

		mesh->Release();
		CheckRenderErrors(__FUNCTION__, __LINE__);

		m_stats.AddToStatCount(Stats::STAT_DRAWCALL, 1);
		return true;
	}

	bool RendererOGL::DrawMeshInstanced(MeshObject *mesh, Material *mat, InstanceBuffer *inst)
	{
		PROFILE_SCOPED()
		SetRenderState(mat->GetStateDescriptor());
		mat->Apply();
		CheckRenderErrors(__FUNCTION__, __LINE__);

		int numElems = mesh->GetIndexBuffer() ? mesh->GetIndexBuffer()->GetIndexCount() : mesh->GetVertexBuffer()->GetSize();
		PrimitiveType pt = mat->GetStateDescriptor().primitiveType;

		mesh->Bind();
		inst->Bind();
		if (mesh->GetIndexBuffer())
			glDrawElementsInstanced(pt, numElems, GL_UNSIGNED_INT, nullptr, inst->GetInstanceCount());
		else
			glDrawArraysInstanced(pt, 0, numElems, inst->GetInstanceCount());
		inst->Release();
		mesh->Release();
		CheckRenderErrors(__FUNCTION__, __LINE__);

		m_stats.AddToStatCount(Stats::STAT_DRAWCALL, 1);
		return true;
	}

	Material *RendererOGL::CreateMaterial(const std::string &shader, const MaterialDescriptor &d, const RenderStateDesc &stateDescriptor)
	{
		PROFILE_SCOPED()
		MaterialDescriptor desc = d;

		OGL::Material *mat = new OGL::Material;

		if (desc.lighting) {
			desc.dirLights = m_numDirLights;
		}

		mat->m_renderer = this;
		mat->m_descriptor = desc;
		mat->m_stateDescriptor = stateDescriptor;

		OGL::Shader *s = nullptr;
		for (auto &pair : m_shaders) {
			if (pair.first == shader)
				s = pair.second;
		}

		if (!s) {
			s = new OGL::Shader(shader);
			Log::Info("Created shader {} (address={})\n", shader, (void *)s);
			CheckRenderErrors(__FUNCTION__, __LINE__);

			m_shaders.push_back({ shader, s });
		}

		mat->SetShader(s);
		CheckRenderErrors(__FUNCTION__, __LINE__);
		return mat;
	}

	Material *RendererOGL::CloneMaterial(const Material *old, const MaterialDescriptor &descriptor, const RenderStateDesc &stateDescriptor)
	{
		OGL::Material *newMat = new OGL::Material();
		newMat->m_renderer = this;
		newMat->m_descriptor = descriptor;
		newMat->m_stateDescriptor = stateDescriptor;

		const OGL::Material *material = static_cast<const OGL::Material *>(old);
		newMat->SetShader(material->m_shader);
		material->Copy(newMat);

		CheckRenderErrors(__FUNCTION__, __LINE__);
		return newMat;
	}

	bool RendererOGL::ReloadShaders()
	{
		Log::Info("Reloading {} shaders...\n", m_shaders.size());
		Log::Info("Note: runtime shader reloading does not reload uniform assignments. Restart the program when making major changes.\n");
		for (auto &pair : m_shaders) {
			pair.second->Reload();
		}
		Log::Info("Done.\n");

		return true;
	}

	Texture *RendererOGL::CreateTexture(const TextureDescriptor &descriptor)
	{
		PROFILE_SCOPED()
		return new OGL::TextureGL(descriptor, m_useCompressedTextures, m_useAnisotropicFiltering);
	}

	static uint32_t HashRenderStateDesc(const RenderStateDesc &desc)
	{
		// Can't directly pass RenderStateDesc* to lookup3_hashlittle, because
		// it (most likely) has padding bytes, and those bytes are uninitialized,
		// thereby arbitrarily affecting the hash output.
		// (We used to do this and valgrind complained).
		uint32_t words[5] = {
			desc.blendMode,
			desc.cullMode,
			desc.primitiveType,
			desc.depthTest,
			desc.depthWrite,
		};
		return lookup3_hashword(words, 5, 0);
	}

	RenderTarget *RendererOGL::CreateRenderTarget(const RenderTargetDesc &desc)
	{
		PROFILE_SCOPED()
		OGL::RenderTarget *rt = new OGL::RenderTarget(desc);
		CheckRenderErrors(__FUNCTION__, __LINE__);
		rt->Bind();
		if (desc.colorFormat != TEXTURE_NONE) {
			Graphics::TextureDescriptor cdesc(
				desc.colorFormat,
				vector3f(desc.width, desc.height, 0.0f),
				vector2f(desc.width, desc.height),
				LINEAR_CLAMP,
				false,
				false,
				false,
				0, Graphics::TEXTURE_2D);
			OGL::TextureGL *colorTex = new OGL::TextureGL(cdesc, false, false, desc.numSamples);
			rt->SetColorTexture(colorTex);
		}
		if (desc.depthFormat != TEXTURE_NONE) {
			if (desc.allowDepthTexture) {
				Graphics::TextureDescriptor ddesc(
					TEXTURE_DEPTH,
					vector3f(desc.width, desc.height, 0.0f),
					vector2f(desc.width, desc.height),
					LINEAR_CLAMP,
					false,
					false,
					false,
					0, Graphics::TEXTURE_2D);
				OGL::TextureGL *depthTex = new OGL::TextureGL(ddesc, false, false);
				rt->SetDepthTexture(depthTex);
			} else {
				rt->CreateDepthRenderbuffer();
			}
		}

		rt->Unbind();
		CheckRenderErrors(__FUNCTION__, __LINE__);

		// Rebind the active render target
		if (m_activeRenderTarget)
			m_activeRenderTarget->Bind();
		// we can't assume the window render target exists yet because we might be creating it
		else if (m_windowRenderTarget)
			m_windowRenderTarget->Bind();

		return rt;
	}

	VertexBuffer *RendererOGL::CreateVertexBuffer(const VertexBufferDesc &desc)
	{
		m_stats.AddToStatCount(Stats::STAT_CREATE_BUFFER, 1);
		return new OGL::VertexBuffer(desc);
	}

	IndexBuffer *RendererOGL::CreateIndexBuffer(Uint32 size, BufferUsage usage)
	{
		m_stats.AddToStatCount(Stats::STAT_CREATE_BUFFER, 1);
		return new OGL::IndexBuffer(size, usage);
	}

	InstanceBuffer *RendererOGL::CreateInstanceBuffer(Uint32 size, BufferUsage usage)
	{
		m_stats.AddToStatCount(Stats::STAT_CREATE_BUFFER, 1);
		return new OGL::InstanceBuffer(size, usage);
	}

	OGL::UniformBuffer *RendererOGL::CreateUniformBuffer(Uint32 size, BufferUsage usage)
	{
		m_stats.AddToStatCount(Stats::STAT_CREATE_BUFFER, 1);
		return new OGL::UniformBuffer(size, usage);
	}

	MeshObject *RendererOGL::CreateMeshObject(VertexBuffer *v, IndexBuffer *i)
	{
		m_stats.AddToStatCount(Stats::STAT_CREATE_BUFFER, 1);
		return new OGL::MeshObject(
			RefCountedPtr<OGL::VertexBuffer>(static_cast<OGL::VertexBuffer *>(v)),
			RefCountedPtr<OGL::IndexBuffer>(static_cast<OGL::IndexBuffer *>(i)));
	}

	MeshObject *RendererOGL::CreateMeshObjectFromArray(const VertexArray *vertexArray, IndexBuffer *indexBuffer, BufferUsage usage)
	{
		VertexBufferDesc desc = VertexBufferDesc::FromAttribSet(vertexArray->GetAttributeSet());
		desc.numVertices = vertexArray->GetNumVerts();
		desc.usage = usage;

		Graphics::VertexBuffer *vertexBuffer = CreateVertexBuffer(desc);
		vertexBuffer->Populate(*vertexArray);

		return CreateMeshObject(vertexBuffer, indexBuffer);
	}

	OGL::UniformBuffer *RendererOGL::GetLightUniformBuffer()
	{
		return m_lightUniformBuffer.Get();
	}

	OGL::UniformLinearBuffer *RendererOGL::GetDrawUniformBuffer(Uint32 size)
	{
		if (m_drawUniformBuffers.empty() || m_drawUniformBuffers.back()->FreeSize() < size) {
			// Create a 1MiB buffer for draw uniform data
			auto *buffer = new OGL::UniformLinearBuffer(1 << 20);
			buffer->IncRefCount();
			m_drawUniformBuffers.emplace_back(buffer);
		}

		return m_drawUniformBuffers.back().get();
	}

	// XXX very heavy. in the future when all GL calls are made through the
	// renderer, we can probably do better by trackingn current state and
	// only restoring the things that have changed
	void RendererOGL::PushState()
	{
		// empty since viewport handling is now external, evaluate if renderer will need to save any custom state
	}

	void RendererOGL::PopState()
	{
		// empty since viewport handling is now external, evaluate if renderer will need to save any custom state
	}

	bool RendererOGL::Screendump(ScreendumpState &sd)
	{
		int w, h;
		SDL_GetWindowSize(m_window, &w, &h);
		sd.width = w;
		sd.height = h;
		sd.bpp = 4; // XXX get from window

		// pad rows to 4 bytes, which is the default row alignment for OpenGL
		sd.stride = ((sd.bpp * sd.width) + 3) & ~3;

		sd.pixels.reset(new Uint8[sd.stride * sd.height]);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glPixelStorei(GL_PACK_ALIGNMENT, 4); // never trust defaults
		glReadBuffer(GL_FRONT);
		glReadPixels(0, 0, sd.width, sd.height, GL_RGBA, GL_UNSIGNED_BYTE, sd.pixels.get());
		glFinish();

		return true;
	}

	bool RendererOGL::FrameGrab(ScreendumpState &sd)
	{
		int w, h;
		SDL_GetWindowSize(m_window, &w, &h);
		sd.width = w;
		sd.height = h;
		sd.bpp = 4; // XXX get from window

		sd.stride = (4 * sd.width);

		sd.pixels.reset(new Uint8[sd.stride * sd.height]);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glPixelStorei(GL_PACK_ALIGNMENT, 4); // never trust defaults
		glReadBuffer(GL_FRONT);
		glReadPixels(0, 0, sd.width, sd.height, GL_RGBA, GL_UNSIGNED_BYTE, sd.pixels.get());
		glFinish();

		return true;
	}

} // namespace Graphics
