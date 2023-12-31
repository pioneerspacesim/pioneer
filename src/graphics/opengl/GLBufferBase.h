// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "OpenGLLibs.h"

namespace Graphics {

	namespace OGL {

		class GLBufferBase {
		public:
			GLBufferBase() :
				m_written(false) {}
			GLuint GetBuffer() const { return m_buffer; }

		protected:
			GLuint m_buffer;
			bool m_written; // to check for invalid data rendering
		};

	} // namespace OGL

} // namespace Graphics
