// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GL2_RENDERTARGET_H
#define _GL2_RENDERTARGET_H
/*
 * Framebuffer object with switchable target textures.
 * In theory you should use one texture format and size per FBO
 * 2013-May-05 left out stencil buffer because we don't need it now
 */
#include "graphics/RenderTarget.h"

namespace Graphics {

class RendererGL2;

namespace GL2 {

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
	virtual void SetColorTexture(Texture*);
	virtual void SetDepthTexture(Texture*);

protected:
	friend class Graphics::RendererGL2;
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
