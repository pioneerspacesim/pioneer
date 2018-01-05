// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OGL_RENDERTARGET_H
#define _OGL_RENDERTARGET_H
/*
 * Framebuffer object with switchable target textures.
 * In theory you should use one texture format and size per FBO
 * 2013-May-05 left out stencil buffer because we don't need it now
 */
#include "OpenGLLibs.h"
#include "graphics/RenderTarget.h"

namespace Graphics {

class RendererOGL;

namespace OGL {

class RenderTarget;

class RenderBuffer : public RefCounted {
public:
	~RenderBuffer();
	void Bind();
	void Unbind();
	void Attach(GLenum attachment);

protected:
	friend class RenderTarget;
	RenderBuffer();
	GLuint buffer;
};

class RenderTarget : public Graphics::RenderTarget {
public:
	~RenderTarget();
	virtual Texture *GetColorTexture() const;
	virtual Texture *GetDepthTexture() const;
	virtual void SetCubeFaceTexture(const Uint32 face, Texture* t) final;
	virtual void SetColorTexture(Texture*) final;
	virtual void SetDepthTexture(Texture*) final;

protected:
	friend class Graphics::RendererOGL;
	RenderTarget(const RenderTargetDesc &);
	void Bind();
	void Unbind();
	void CreateDepthRenderbuffer();
	bool CheckStatus();

	bool m_active;
	GLuint m_fbo;

	RefCountedPtr<RenderBuffer> m_depthRenderBuffer;
	RefCountedPtr<Texture> m_colorTexture;
	RefCountedPtr<Texture> m_depthTexture;
};

}

}

#endif
