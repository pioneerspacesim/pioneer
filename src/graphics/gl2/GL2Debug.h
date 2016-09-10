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
#include "libs.h"
#ifdef _WIN32
#define STDCALL __stdcall
#else
#define STDCALL
#endif

// some people build with an old version of GLEW that doesn't include KHR_debug
#if (GL_ARB_debug_output && GL_KHR_debug)

namespace Graphics {

	class GLDebug {
	private:
		static const char *type_to_string(GLenum type) {
			switch(type) {
				case GL_DEBUG_TYPE_ERROR_ARB:
					return("Error");
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
					return("Deprecated Behaviour");
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
					return("Undefined Behaviour");
				case GL_DEBUG_TYPE_PORTABILITY_ARB:
					return("Portability");
				case GL_DEBUG_TYPE_PERFORMANCE_ARB:
					return("Performance");
				case GL_DEBUG_TYPE_OTHER_ARB:
					return("Other");
				default:
					return("");
			}
		}

		static const char *source_to_string(GLenum source) {
			switch(source) {
				case GL_DEBUG_SOURCE_API_ARB:
					return("API");
				case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
					return("Window System");
				case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
					return("Shader Compiler");
				case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
					return("Third Party");
				case GL_DEBUG_SOURCE_APPLICATION_ARB:
					return("Application");
				case GL_DEBUG_SOURCE_OTHER_ARB:
					return("Other");
				default:
					return("");
			}
		}

		static const char *severity_to_string(GLenum severity) {
			switch(severity) {
				case GL_DEBUG_SEVERITY_HIGH_ARB:
					return("High");
				case GL_DEBUG_SEVERITY_MEDIUM_ARB:
					return("Medium");
				case GL_DEBUG_SEVERITY_LOW_ARB:
					return("Low");
				default:
					return("");
			}
		}

		// put a breakpoint in this function
		static void STDCALL PrintMessage(GLenum source, GLenum type,
			GLuint id, GLenum severity, GLsizei length,
			const GLchar* message, void* userParam)
		{
			Output("Type: %s, Source: %s, ID: %u, Severity: %s, Message: %s\n",
				type_to_string(type), source_to_string(source), id,
				severity_to_string(severity), message);
		}

	public:
		//register the callback function, if the extension is available
		static void Enable() {
			if (!glewIsSupported("GL_KHR_debug")) {
				Output("GL_KHR_debug is not supported; GLDebug will not work\n");
				return;
			}

			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
			glDebugMessageCallbackARB(PrintMessage, 0);

			//Using the default message type and severity parameters.
			//If you want to be drowned in performance warnings, use:
			//glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW_ARB, 0, 0, true);
		}

		static void Disable() {
			if (glewIsSupported("GL_KHR_debug")) {
				glDisable(GL_DEBUG_OUTPUT);
			}
		}

	};

}

#else

namespace Graphics {

	class GLDebug {
	public:
		static void Enable() {
			Output("GL Debug support was excluded from this build because the GLEW headers were not recent enough\n");
		}

		static void Disable() {}

	};

}

#endif

#endif
