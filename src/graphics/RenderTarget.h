#ifndef _RENDERTARGET_H
#define _RENDERTARGET_H

#include "Texture.h"
#include <exception>

namespace Graphics {

	/*
	 * FBO rendertarget, only color
	 */
	class RenderTarget : public Texture {
	public:
		struct fbo_incomplete : public std::exception {
			public:
				fbo_incomplete(GLenum errcode): m_errcode(errcode) {}
				const char *what() const throw();
				GLenum GetErrorCode() const throw() { return m_errcode; }
			private:
				GLenum m_errcode;
		};

		RenderTarget(unsigned int width, unsigned int height, GLenum target, const Texture::Format &format, bool wantMipmaps = false);
		virtual ~RenderTarget();

		virtual void BeginRTT();
		virtual void EndRTT();

		//more readable this way with rendertargets
		inline void BindTexture() { Bind(); }
		inline void UnbindTexture() { Unbind(); }

	protected:
		/*
		 * Throws an exception if FBO is incomplete.
		 * Bind before checking.
		 */
		virtual void CheckCompleteness() const;
		GLuint m_fbo;
	};

}

#endif
