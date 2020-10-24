// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GLDEBUG_H
#define _GLDEBUG_H
/*
 * OpenGL debug helper using the GL_KHR_debug extension
 * which does not require a debug context.
 *
 * Something similar can be found at:
 * https://github.com/OpenGLInsights/ (Chapter 33)
 * which also includes stack printout
 */
#include "OpenGLLibs.h"
#include "utils.h"

#ifdef _WIN32
#define STDCALL __stdcall
#else
#define STDCALL
#endif

// some people build with an old version of GLEW that doesn't include KHR_debug
#if 1 //(GL_ARB_debug_output && GL_KHR_debug)

namespace Graphics {

	class GLDebug {
	private:
		static const char *type_to_string(GLenum type)
		{
			switch (type) {
			case GL_DEBUG_TYPE_ERROR:
				return ("Error");
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
				return ("Deprecated Behaviour");
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
				return ("Undefined Behaviour");
			case GL_DEBUG_TYPE_PORTABILITY:
				return ("Portability");
			case GL_DEBUG_TYPE_PERFORMANCE:
				return ("Performance");
			case GL_DEBUG_TYPE_OTHER:
				return ("Other");
			default:
				return ("");
			}
		}

		static const char *source_to_string(GLenum source)
		{
			switch (source) {
			case GL_DEBUG_SOURCE_API:
				return ("API");
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
				return ("Window System");
			case GL_DEBUG_SOURCE_SHADER_COMPILER:
				return ("Shader Compiler");
			case GL_DEBUG_SOURCE_THIRD_PARTY:
				return ("Third Party");
			case GL_DEBUG_SOURCE_APPLICATION:
				return ("Application");
			case GL_DEBUG_SOURCE_OTHER:
				return ("Other");
			default:
				return ("");
			}
		}

		static const char *severity_to_string(GLenum severity)
		{
			switch (severity) {
			case GL_DEBUG_SEVERITY_HIGH:
				return ("High");
			case GL_DEBUG_SEVERITY_MEDIUM:
				return ("Medium");
			case GL_DEBUG_SEVERITY_LOW:
				return ("Low");
			default:
				return ("");
			}
		}

		// put a breakpoint in this function
		static void STDCALL PrintMessage(GLenum source, GLenum type,
			GLuint id, GLenum severity, GLsizei length,
			const GLchar *message, const void *userParam)
		{
			// filter out Type=Other informational messages
			if (type > GL_DEBUG_TYPE_PERFORMANCE)
				return;
			Output("Type: %s, Source: %s, ID: %u, Severity: %s, Message: %s\n",
				type_to_string(type), source_to_string(source), id,
				severity_to_string(severity), message);
		}

	public:
		//register the callback function, if the extension is available
		static void Enable()
		{
			if (!glewIsSupported("GL_KHR_debug")) {
				Output("GL_KHR_debug is not supported; GLDebug will not work\n");
				return;
			}

			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(PrintMessage, 0);

			//Using the default message type and severity parameters.
			//If you want to be drowned in performance warnings, use:
			//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, 0, true);
		}

		static void Disable()
		{
			if (glewIsSupported("GL_KHR_debug")) {
				glDisable(GL_DEBUG_OUTPUT);
			}
		}
	};

} // namespace Graphics

#else

namespace Graphics {

	class GLDebug {
	public:
		static void Enable()
		{
			Output("GL Debug support was excluded from this build because the glLoadGen headers did not include support for it.\n");
		}

		static void Disable() {}
	};

} // namespace Graphics

#endif

#endif
