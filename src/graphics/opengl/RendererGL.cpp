// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "RendererGL.h"
#include "MathUtil.h"
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

#include "CommandBufferGL.h"
#include "GLDebug.h"
#include "MaterialGL.h"
#include "Program.h"
#include "RenderStateCache.h"
#include "RenderTargetGL.h"
#include "Shader.h"
#include "TextureGL.h"
#include "UniformBuffer.h"
#include "VertexBufferGL.h"

#include "core/Log.h"

#include <SDL.h>

#include <cstddef> //for offsetof
#include <iterator>
#include <ostream>
#include <sstream>

using RenderPassCmd = Graphics::OGL::CommandList::RenderPassCmd;

namespace Graphics {

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

		// HACK (sturnclaw): request RGBA backbuffer specifically for the purpose of using
		// it as an intermediate multisample resolve target with RGBA textures.
		// See ResolveRenderTarget() for more details
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

		winFlags |= (vs.hidden ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN);
		if (!vs.hidden && vs.fullscreen) // TODO: support for borderless fullscreen and changing window size
			winFlags |= SDL_WINDOW_FULLSCREEN;

		if (vs.canBeResized)
			winFlags |= SDL_WINDOW_RESIZABLE;

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

		SDL_GL_SetSwapInterval((vs.vsync != 0) ? -1 : 0);

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
		m_glContext(glContext)
	{
		PROFILE_SCOPED()
		glewExperimental = true;
		GLenum glew_err;
		if ((glew_err = glewInit()) != GLEW_OK)
			Error("GLEW initialisation failed: %s", glstr_to_str(glewGetErrorString(glew_err)));

		// pump this once as glewExperimental is necessary but spews a single error
		glGetError();

		if (vs.enableDebugMessages)
			GLDebug::Enable();

		if (!glewIsSupported("GL_VERSION_3_2")) {
			Error(
				"Pioneer can not run on your graphics card as it does not appear to support OpenGL 3.2\n"
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

		if (!glewIsSupported("GL_ARB_vertex_attrib_binding")) {
			Error("OpenGL extension GL_ARB_vertex_attrib_binding not supported.\n"
				  "Pioneer can not run on your graphics card as it does not support vertex attribute bindings.\n"
				  "Please check to see if your GPU driver vendor has an updated driver - or that drivers are installed correctly.");
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
		glDepthFunc(GL_GEQUAL);
		// clear to 0.0 for use with reverse-Z
		glClearDepth(0.0);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glEnable(GL_PROGRAM_POINT_SIZE);

		glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);
		glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);

		CHECKERRORS();

		// create the state cache immediately after establishing baseline state.
		m_renderStateCache.reset(new OGL::RenderStateCache());

		// check enum PrimitiveType matches OpenGL values
		static_assert(POINTS == GL_POINTS);
		static_assert(LINE_SINGLE == GL_LINES);
		static_assert(LINE_LOOP == GL_LINE_LOOP);
		static_assert(LINE_STRIP == GL_LINE_STRIP);
		static_assert(TRIANGLES == GL_TRIANGLES);
		static_assert(TRIANGLE_STRIP == GL_TRIANGLE_STRIP);
		static_assert(TRIANGLE_FAN == GL_TRIANGLE_FAN);

		m_drawCommandList.reset(new OGL::CommandList(this));

		m_viewport = ViewportExtents(0, 0, m_width, m_height);
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

	void RendererOGL::SetVSyncEnabled(bool enabled)
	{
		SDL_GL_SetSwapInterval(enabled ? -1 : 0);
	}

	void RendererOGL::OnWindowResized()
	{
		SDL_GL_GetDrawableSize(m_window, &m_width, &m_height);
	}

	bool RendererOGL::BeginFrame()
	{
		PROFILE_SCOPED()
		// clear the cached program state (program loading may have trashed it)
		m_renderStateCache->SetProgram(nullptr);

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

		for (auto &buffer : s_DynamicDrawBufferMap) {
			buffer.vtxBuffer->Reset();
		}

		stat.SetStatCount(Stats::STAT_DYNAMIC_DRAW_BUFFER_INUSE, s_DynamicDrawBufferMap.size());
		stat.SetStatCount(Stats::STAT_DRAW_UNIFORM_BUFFER_INUSE, uint32_t(m_drawUniformBuffers.size()));
		stat.SetStatCount(Stats::STAT_DRAW_UNIFORM_BUFFER_ALLOCS, numAllocs);

		uint32_t numShaderPrograms = 0;
		for (auto &pair : m_shaders)
			numShaderPrograms += pair.second->GetNumVariants();

		stat.SetStatCount(Stats::STAT_NUM_RENDER_STATES, m_renderStateCache->m_stateDescCache.size());
		stat.SetStatCount(Stats::STAT_NUM_SHADER_PROGRAMS, numShaderPrograms);

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
				Log::Info("{}", ss.str());
		}
	}

	bool RendererOGL::SwapBuffers()
	{
		PROFILE_SCOPED()

		// Reset to a "known good" render state (disable scissor etc.)
		m_renderStateCache->ApplyRenderState(RenderStateDesc{});

		// TODO(sturnclaw): handle upscaling to higher-resolution screens
		// we'll need an intermediate target to resolve to; resolve and rescale are mutually exclusive
		ViewportExtents ext = { 0, 0, m_width, m_height };
		bool isMSAA = m_activeRenderTarget->GetDesc().numSamples > 0;

		m_drawCommandList->AddBlitRenderTargetCmd(m_activeRenderTarget, nullptr, ext, ext, isMSAA);

		FlushCommandBuffers();
		CheckRenderErrors(__FUNCTION__, __LINE__);

		SDL_GL_SwapWindow(m_window);
		m_activeRenderTarget = nullptr;
		m_renderStateCache->ResetFrame();
		m_stats.NextFrame();
		return true;
	}

	RenderTarget *RendererOGL::GetRenderTarget()
	{
		return m_activeRenderTarget;
	}

	bool RendererOGL::SetRenderTarget(RenderTarget *rt)
	{
		PROFILE_SCOPED()
		FlushCommandBuffers();

		m_activeRenderTarget = static_cast<OGL::RenderTarget *>(rt);
		m_drawCommandList->AddRenderPassCmd(m_activeRenderTarget, m_viewport);
		CheckRenderErrors(__FUNCTION__, __LINE__);

		return true;
	}

	void RendererOGL::CopyRenderTarget(RenderTarget *src, RenderTarget *dst, ViewportExtents srcExtents, ViewportExtents dstExtents, bool linearFilter)
	{
		m_drawCommandList->AddBlitRenderTargetCmd(src, dst, srcExtents, dstExtents, false, false, linearFilter);
		m_drawCommandList->AddRenderPassCmd(m_activeRenderTarget, m_viewport);
	}

	void RendererOGL::ResolveRenderTarget(RenderTarget *src, RenderTarget *dst, ViewportExtents extents)
	{
		bool hasDepthTexture = src->GetDepthTexture() && dst->GetDepthTexture();

		// HACK (sturnclaw): work around NVidia undocumented behavior of using a higher-quality filtering
		// kernel when resolving to window backbuffer instead of offscreen FBO.
		// Otherwise there's a distinct visual quality loss when performing MSAA resolve to offscreen FBO.
		// Ideally this should be replaced by using a custom MSAA resolve shader; however builtin resolve
		// usually has better performance (ref: https://therealmjp.github.io/posts/msaa-resolve-filters/)
		// NOTE: this behavior appears to be independent of setting GL_MULTISAMPLE_FILTER_HINT_NV on Linux
		if (!hasDepthTexture && extents.w <= m_width && extents.h <= m_height) {
			ViewportExtents tmpExtents = { 0, 0, extents.w, extents.h };

			m_drawCommandList->AddBlitRenderTargetCmd(src, nullptr, extents, tmpExtents, true);
			m_drawCommandList->AddBlitRenderTargetCmd(nullptr, dst, tmpExtents, extents, true);
		} else {
			m_drawCommandList->AddBlitRenderTargetCmd(src, dst, extents, extents, true, hasDepthTexture);
		}

		m_drawCommandList->AddRenderPassCmd(m_activeRenderTarget, m_viewport);
	}

	bool RendererOGL::SetScissor(ViewportExtents extents)
	{
		m_drawCommandList->AddScissorCmd(extents);
		return true;
	}

	bool RendererOGL::ClearScreen(const Color &clearColor, bool depth)
	{
		m_drawCommandList->AddClearCmd(true, depth, clearColor);
		return true;
	}

	bool RendererOGL::ClearDepthBuffer()
	{
		m_drawCommandList->AddClearCmd(false, true, Color());
		return true;
	}

	bool RendererOGL::SetWireFrameMode(bool enabled)
	{
		FlushCommandBuffers();
		glPolygonMode(GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
		return true;
	}

	bool RendererOGL::SetViewport(ViewportExtents v)
	{
		m_viewport = v;
		m_drawCommandList->AddRenderPassCmd(m_activeRenderTarget, m_viewport);
		return true;
	}

	bool RendererOGL::SetTransform(const matrix4x4f &m)
	{
		m_modelViewMat = m;
		return true;
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
		PROFILE_SCOPED()

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

	// 1MB vertex draw buffer should be enough for most cases, right?
	static constexpr uint32_t DYNAMIC_DRAW_BUFFER_SIZE = 1 << 20;
	bool RendererOGL::DrawBuffer(const VertexArray *v, Material *m)
	{
		PROFILE_SCOPED()

		if (v->IsEmpty()) return false;

		const AttributeSet attrs = v->GetAttributeSet();

		// Find a buffer matching our attributes with enough free space
		auto iter = std::find_if(s_DynamicDrawBufferMap.begin(), s_DynamicDrawBufferMap.end(), [&](DynamicBufferData &a) {
			uint32_t freeSize = a.vtxBuffer->GetCapacity() - a.vtxBuffer->GetSize();
			return a.attrs == attrs && freeSize >= v->GetNumVerts();
		});

		// If we don't have one, make one
		if (iter == s_DynamicDrawBufferMap.end()) {
			auto desc = VertexBufferDesc::FromAttribSet(v->GetAttributeSet());
			desc.numVertices = DYNAMIC_DRAW_BUFFER_SIZE / desc.stride;
			desc.usage = BUFFER_USAGE_DYNAMIC;

			size_t stateHash = m_renderStateCache->CacheVertexDesc(desc);
			OGL::CachedVertexBuffer *vb = new OGL::CachedVertexBuffer(desc, stateHash);
			MeshObject *meshObject = CreateMeshObject(vb, nullptr);
			s_DynamicDrawBufferMap.push_back(DynamicBufferData{ attrs, vb, RefCountedPtr<MeshObject>(meshObject) });

			GetStats().AddToStatCount(Stats::STAT_CREATE_BUFFER, 1);
			GetStats().AddToStatCount(Stats::STAT_DYNAMIC_DRAW_BUFFER_CREATED, 1);
			iter = s_DynamicDrawBufferMap.end() - 1;
		}

		// Write our data into the buffer
		uint32_t offset = iter->vtxBuffer->GetOffset();
		iter->vtxBuffer->Populate(*v);
		CheckRenderErrors(__FUNCTION__, __LINE__);

		// Append a command to the command list
		m_drawCommandList->AddDynamicDrawCmd({ iter->mesh->GetVertexBuffer(), offset, v->GetNumVerts() }, {}, m);

		return true;
	}

	bool RendererOGL::DrawBufferDynamic(VertexBuffer *v, uint32_t vtxOffset, IndexBuffer *i, uint32_t idxOffset, uint32_t numElems, Material *mat)
	{
		if (!numElems)
			return false;

		uint32_t indexSize = i && i->GetElementSize() == INDEX_BUFFER_16BIT ? sizeof(uint16_t) : sizeof(uint32_t);
		m_drawCommandList->AddDynamicDrawCmd(
			{ v, vtxOffset * v->GetDesc().stride, numElems },
			{ i, i == nullptr ? 0 : idxOffset * indexSize, numElems },
			mat);

		return true;
	}

	bool RendererOGL::DrawMesh(MeshObject *mesh, Material *material)
	{
		m_drawCommandList->AddDrawCmd(mesh, material, nullptr);
		return true;
	}

	bool RendererOGL::DrawMeshInstanced(MeshObject *mesh, Material *material, InstanceBuffer *inst)
	{
		m_drawCommandList->AddDrawCmd(mesh, material, inst);
		return true;
	}

	bool RendererOGL::FlushCommandBuffers()
	{
		PROFILE_SCOPED()
		if (!m_drawCommandList || m_drawCommandList->IsEmpty())
			return false;

		for (auto &buffer : m_drawUniformBuffers)
			buffer->Flush();

		for (auto &buffer : s_DynamicDrawBufferMap)
			buffer.vtxBuffer->Flush();

		m_drawCommandList->m_executing = true;

		for (const auto &cmd : m_drawCommandList->GetDrawCmds()) {
			if (auto *drawCmd = std::get_if<OGL::CommandList::DrawCmd>(&cmd))
				m_drawCommandList->ExecuteDrawCmd(*drawCmd);
			else if (auto *dynDrawCmd = std::get_if<OGL::CommandList::DynamicDrawCmd>(&cmd))
				m_drawCommandList->ExecuteDynamicDrawCmd(*dynDrawCmd);
			else if (auto *renderPassCmd = std::get_if<OGL::CommandList::RenderPassCmd>(&cmd))
				m_drawCommandList->ExecuteRenderPassCmd(*renderPassCmd);
			else if (auto *blitRenderTargetCmd = std::get_if<OGL::CommandList::BlitRenderTargetCmd>(&cmd))
				m_drawCommandList->ExecuteBlitRenderTargetCmd(*blitRenderTargetCmd);
		}

		// we don't manually reset the active vertex array after each drawcall for performance,
		// so we need to reset it here or risk stomping on state.
		glBindVertexArray(0);

		m_drawCommandList->m_executing = false;
		m_drawCommandList->Reset();

		m_stats.AddToStatCount(Stats::STAT_NUM_CMDLIST_FLUSHES, 1);
		return true;
	}

	static void stat_primitives(Stats &stats, PrimitiveType type, uint32_t count)
	{
		switch (type) {
		case POINTS:
			stats.AddToStatCount(Stats::STAT_NUM_POINTS, count);
			return;
		case LINE_STRIP:
			count -= 1; // fall-through
		case LINE_LOOP:
			stats.AddToStatCount(Stats::STAT_NUM_LINES, count);
			return;
		case LINE_SINGLE:
			stats.AddToStatCount(Stats::STAT_NUM_LINES, count / 2);
			return;
		case TRIANGLE_FAN:
		case TRIANGLE_STRIP:
			stats.AddToStatCount(Stats::STAT_NUM_TRIS, count - 2);
			return;
		case TRIANGLES:
			stats.AddToStatCount(Stats::STAT_NUM_TRIS, count / 3);
			return;
		}
	}

	static GLuint get_element_size(OGL::IndexBuffer *idx)
	{
		return idx->GetElementSize() == INDEX_BUFFER_16BIT ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
	}

	bool RendererOGL::DrawMeshInternal(OGL::MeshObject *mesh, PrimitiveType type)
	{
		PROFILE_SCOPED()

		glBindVertexArray(mesh->GetVertexArrayObject());
		uint32_t numElems = mesh->m_idxBuffer.Valid() ? mesh->m_idxBuffer->GetIndexCount() : mesh->m_vtxBuffer->GetSize();

		if (mesh->m_idxBuffer.Valid()) {
			// FIXME: terrain segfaults without this BindBuffer call
			// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_idxBuffer->GetBuffer());
			glDrawElements(type, numElems, get_element_size(mesh->m_idxBuffer.Get()), nullptr);
		} else
			glDrawArrays(type, 0, numElems);

		CheckRenderErrors(__FUNCTION__, __LINE__);
		m_stats.AddToStatCount(Stats::STAT_DRAWCALL, 1);
		stat_primitives(m_stats, type, numElems);
		return true;
	}

	bool RendererOGL::DrawMeshInstancedInternal(OGL::MeshObject *mesh, OGL::InstanceBuffer *inst, PrimitiveType type)
	{
		PROFILE_SCOPED()

		glBindVertexArray(mesh->GetVertexArrayObject());
		uint32_t numElems = mesh->m_idxBuffer.Valid() ? mesh->m_idxBuffer->GetIndexCount() : mesh->m_vtxBuffer->GetSize();
		inst->Bind();

		if (mesh->m_idxBuffer.Valid()) {
			glDrawElementsInstanced(type, numElems, get_element_size(mesh->m_idxBuffer.Get()), nullptr, inst->GetInstanceCount());
		} else {
			glDrawArraysInstanced(type, 0, numElems, inst->GetInstanceCount());
		}

		inst->Release();
		CheckRenderErrors(__FUNCTION__, __LINE__);
		m_stats.AddToStatCount(Stats::STAT_DRAWCALL, 1);
		stat_primitives(m_stats, type, numElems);
		return true;
	}

	bool RendererOGL::DrawMeshDynamicInternal(BufferBinding<OGL::VertexBuffer> vtxBind, BufferBinding<OGL::IndexBuffer> idxBind, PrimitiveType type)
	{
		PROFILE_SCOPED()

		glBindVertexArray(m_renderStateCache->GetVertexArrayObject(vtxBind.buffer->GetVertexFormatHash()));
		glBindVertexBuffer(0, vtxBind.buffer->GetBuffer(), vtxBind.offset, vtxBind.buffer->GetDesc().stride);
		if (idxBind.buffer) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxBind.buffer->GetBuffer());
			glDrawElements(type, idxBind.size, get_element_size(idxBind.buffer), (void *)(uintptr_t)(idxBind.offset));
		} else {
			glDrawArrays(type, 0, vtxBind.size);
		}

		CheckRenderErrors(__FUNCTION__, __LINE__);
		m_stats.AddToStatCount(Stats::STAT_DRAWCALL, 1);
		stat_primitives(m_stats, type, idxBind.buffer ? idxBind.size : vtxBind.size);
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
		mat->m_renderStateHash = m_renderStateCache->InternRenderState(stateDescriptor);

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
		newMat->m_renderStateHash = m_renderStateCache->InternRenderState(stateDescriptor);

		const OGL::Material *material = static_cast<const OGL::Material *>(old);
		newMat->SetShader(material->m_shader);
		material->Copy(newMat);

		CheckRenderErrors(__FUNCTION__, __LINE__);
		return newMat;
	}

	bool RendererOGL::ReloadShaders()
	{
		m_renderStateCache->SetProgram(nullptr);
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

	RenderTarget *RendererOGL::CreateRenderTarget(const RenderTargetDesc &desc)
	{
		PROFILE_SCOPED()
		OGL::RenderTarget *rt = new OGL::RenderTarget(this, desc);
		CheckRenderErrors(__FUNCTION__, __LINE__);
		m_renderStateCache->SetRenderTarget(rt);

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
			CHECKERRORS();
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
				OGL::TextureGL *depthTex = new OGL::TextureGL(ddesc, false, false, desc.numSamples);
				rt->SetDepthTexture(depthTex);
				CHECKERRORS();
			} else {
				rt->CreateDepthRenderbuffer();
			}
		}

		CheckRenderErrors(__FUNCTION__, __LINE__);

		if (desc.colorFormat != TEXTURE_NONE && desc.depthFormat != TEXTURE_NONE && !rt->CheckStatus()) {
			GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			Log::Error("Unable to create complete render target. (Error: {})\n"
					   "Does your graphics driver support multisample anti-aliasing?\n"
					   "If this issue persists, try setting AntiAliasingMode=0 in your config file.\n",
				gl_framebuffer_error_to_string(status));
		}

		// Rebind the active render target
		m_renderStateCache->SetRenderTarget(m_activeRenderTarget);

		return rt;
	}

	VertexBuffer *RendererOGL::CreateVertexBuffer(const VertexBufferDesc &desc)
	{
		m_stats.AddToStatCount(Stats::STAT_CREATE_BUFFER, 1);
		size_t stateHash = m_renderStateCache->CacheVertexDesc(desc);
		return new OGL::VertexBuffer(desc, stateHash);
	}

	IndexBuffer *RendererOGL::CreateIndexBuffer(Uint32 size, BufferUsage usage, IndexBufferSize el)
	{
		m_stats.AddToStatCount(Stats::STAT_CREATE_BUFFER, 1);
		return new OGL::IndexBuffer(size, usage, el);
	}

	InstanceBuffer *RendererOGL::CreateInstanceBuffer(Uint32 size, BufferUsage usage)
	{
		m_stats.AddToStatCount(Stats::STAT_CREATE_BUFFER, 1);
		return new OGL::InstanceBuffer(size, usage);
	}

	UniformBuffer *RendererOGL::CreateUniformBuffer(Uint32 size, BufferUsage usage)
	{
		m_stats.AddToStatCount(Stats::STAT_CREATE_BUFFER, 1);
		return new OGL::UniformBuffer(size, usage);
	}

	MeshObject *RendererOGL::CreateMeshObject(VertexBuffer *v, IndexBuffer *i)
	{
		m_stats.AddToStatCount(Stats::STAT_CREATE_BUFFER, 1);
		return new OGL::MeshObject(v, i);
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
		for (auto &buffer : m_drawUniformBuffers)
			if (buffer->FreeSize() >= size)
				return buffer.get();

		auto *buffer = new OGL::UniformLinearBuffer(1 << 20);
		m_drawUniformBuffers.emplace_back(buffer);
		return buffer;
	}

	const RenderStateDesc &RendererOGL::GetMaterialRenderState(const Graphics::Material *m)
	{
		return m_renderStateCache->GetRenderState(m->m_renderStateHash);
	}

	bool RendererOGL::Screendump(ScreendumpState &sd)
	{
		int w, h;
		SDL_GetWindowSize(m_window, &w, &h);
		sd.width = w;
		sd.height = h;
		sd.bpp = 3;

		sd.stride = (sd.bpp * sd.width);

		sd.pixels.reset(new Uint8[sd.stride * sd.height]);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadBuffer(GL_FRONT);
		glReadPixels(0, 0, sd.width, sd.height, GL_RGB, GL_UNSIGNED_BYTE, sd.pixels.get());
		glFinish();

		// this might harmlessly error if we're in a single buffered mode,
		// however in double buffered mode it makes the window in window screens
		// such as ones that show the ship within a menu (F3) work correctly
		// as GL_BACK is the default.
		glReadBuffer(GL_BACK);
		return true;
	}

} // namespace Graphics
