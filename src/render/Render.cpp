#include "Render.h"
#include "RenderTarget.h"
#include <stdexcept>
#include <sstream>
#include <iterator>

static GLuint boundArrayBufferObject = 0;
static GLuint boundElementArrayBufferObject = 0;

namespace Render {

static bool initted = false;

static bool hdrAvailable = false;
static bool hdrEnabled = false;

Shader *simpleShader;
Shader *planetRingsShader[4];

SHADER_CLASS_BEGIN(PostprocessShader)
	SHADER_UNIFORM_SAMPLER(fboTex)
SHADER_CLASS_END()

SHADER_CLASS_BEGIN(PostprocessComposeShader)
	SHADER_UNIFORM_SAMPLER(fboTex)
	SHADER_UNIFORM_SAMPLER(bloomTex)
	SHADER_UNIFORM_FLOAT(avgLum)
	SHADER_UNIFORM_FLOAT(middleGrey)
SHADER_CLASS_END()

SHADER_CLASS_BEGIN(PostprocessDownsampleShader)
	SHADER_UNIFORM_SAMPLER(fboTex)
	SHADER_UNIFORM_FLOAT(avgLum)
	SHADER_UNIFORM_FLOAT(middleGrey)
SHADER_CLASS_END()

PostprocessDownsampleShader *postprocessBloom1Downsample;
PostprocessShader *postprocessBloom2Downsample, *postprocessBloom3VBlur, *postprocessBloom4HBlur, *postprocessLuminance;
PostprocessComposeShader *postprocessCompose;

SHADER_CLASS_BEGIN(BillboardShader)
	SHADER_UNIFORM_SAMPLER(some_texture)
SHADER_CLASS_END()

BillboardShader *billboardShader;

int State::m_numLights = 1;
float State::m_znear = 10.0f;
float State::m_zfar = 1e6f;
float State::m_invLogZfarPlus1;
Shader *State::m_currentShader = 0;


// 2D rectangle texture for rtt
class RectangleTarget : public RenderTarget {
public:
	RectangleTarget(unsigned int w, unsigned int h) :
		RenderTarget(w, h, GL_TEXTURE_RECTANGLE, Texture::Format(GL_RGB16F_ARB, GL_RGB, GL_HALF_FLOAT_ARB))
	{ }
};


// 2D target with mipmaps for capturing scene luminance
class LuminanceTarget : public RenderTarget {
public:
	LuminanceTarget(unsigned int w, unsigned int h) :
		RenderTarget(w, h, GL_TEXTURE_2D, Texture::Format(GL_RGB16F, GL_RGB, GL_FLOAT), true)
	{ }

	void UpdateMipmaps() {
		glGenerateMipmapEXT(GL_TEXTURE_2D);
	}
};


// 2d rectangle target, used as base for fancier scene targets
class SceneTarget : public RectangleTarget {
protected:
	SceneTarget(unsigned int w, unsigned int h) :
		RectangleTarget(w, h)
	{ }
};


// 2d rectangle target with 24-bit depth buffer attachment
class StandardSceneTarget : public SceneTarget {
public:
	StandardSceneTarget(unsigned int w, unsigned int h) :
		SceneTarget(w, h)
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

		glGenRenderbuffersEXT(1, &m_depth);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depth);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, w, h);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_depth);

		CheckCompleteness();

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	~StandardSceneTarget() {
		glDeleteRenderbuffersEXT(1, &m_depth);
	}

private:
	GLuint m_depth;
};


// 2d rectangle target with multisampled colour and 24-bit depth buffer attachment
class MultiSampledSceneTarget : public SceneTarget {
public:
	MultiSampledSceneTarget(unsigned int w, int h, int samples) :
		SceneTarget(w, h)
	{
		const Texture::Format format = GetFormat();

		// multisampled fbo
		glGenFramebuffersEXT(1, &m_msFbo);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_msFbo);

		// ms color buffer
		glGenRenderbuffersEXT(1, &m_msColor);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_msColor);
		glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, samples, format.internalFormat, w, h);

		// ms depth buffer
		glGenRenderbuffersEXT(1, &m_msDepth);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_msDepth);
		glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, samples, GL_DEPTH_COMPONENT24, w, h);

		// attach
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, m_msColor);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,  GL_RENDERBUFFER_EXT, m_msDepth);

		CheckCompleteness();
	}

	~MultiSampledSceneTarget() {
		glDeleteRenderbuffersEXT(1, &m_msDepth);
		glDeleteRenderbuffersEXT(1, &m_msColor);
		glDeleteFramebuffersEXT(1, &m_msFbo);
	}

	virtual void BeginRTT() {
		//begin rendering to multisampled FBO
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_msFbo);
		glPushAttrib(GL_VIEWPORT_BIT);
		glViewport(0, 0, GetWidth(), GetHeight());
	}

	virtual void EndRTT() {
		//blit multisampled rendering to normal fbo
		glPopAttrib();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, m_msFbo);
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, m_fbo);

		//depth testing has already been done, so color is enough
		glBlitFramebufferEXT(0, 0, GetWidth(), GetHeight(), 0, 0, GetWidth(), GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
	}

private:
	GLuint m_msFbo;
	GLuint m_msColor;
	GLuint m_msDepth;
};


void BindArrayBuffer(GLuint bo)
{
	if (boundArrayBufferObject != bo) {
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, bo);
		boundArrayBufferObject = bo;
	}
}

bool IsArrayBufferBound(GLuint bo)
{
	return boundArrayBufferObject == bo;
}

void BindElementArrayBuffer(GLuint bo)
{
	if (boundElementArrayBufferObject != bo) {
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bo);
		boundElementArrayBufferObject = bo;
	}
}

bool IsElementArrayBufferBound(GLuint bo)
{
	return boundElementArrayBufferObject == bo;
}

void UnbindAllBuffers()
{
	BindElementArrayBuffer(0);
	BindArrayBuffer(0);
}

static struct postprocessBuffers_t {
	bool complete;
	RectangleTarget *halfSizeRT;
	LuminanceTarget *luminanceRT;
	RectangleTarget *bloom1RT;
	RectangleTarget *bloom2RT;
	SceneTarget     *sceneRT;
	int width, height;

	void CreateBuffers(int screen_width, int screen_height) {
		width = screen_width;
		height = screen_height;

		GLint msSamples = 0;
		glGetIntegerv(GL_SAMPLES, &msSamples);

		halfSizeRT  = new RectangleTarget(width>>1, height>>1);
		luminanceRT = new LuminanceTarget(128, 128);
		bloom1RT = new RectangleTarget(width>>2, height>>2);
		bloom2RT = new RectangleTarget(width>>2, height>>2);
		sceneRT = 0;
		if (msSamples > 1) {
			try {
				sceneRT = new MultiSampledSceneTarget(width, height, msSamples);
			} catch (RenderTarget::fbo_incomplete &ex) {
				if (ex.GetErrorCode() == GL_FRAMEBUFFER_UNSUPPORTED_EXT) {
					// try again without multisampling
					glDisable(GL_MULTISAMPLE);
					sceneRT = 0;
					msSamples = 1;
				} else {
					throw;
				}
			}
		}
		if (!sceneRT)
			sceneRT = new StandardSceneTarget(width, height);

		postprocessBloom1Downsample = new PostprocessDownsampleShader("postprocessBloom1Downsample", "#extension GL_ARB_texture_rectangle : enable\n");
		postprocessBloom2Downsample = new PostprocessShader("postprocessBloom2Downsample", "#extension GL_ARB_texture_rectangle : enable\n");
		postprocessBloom3VBlur = new PostprocessShader("postprocessBloom3VBlur");
		postprocessBloom4HBlur = new PostprocessShader("postprocessBloom4HBlur");
		postprocessCompose = new PostprocessComposeShader("postprocessCompose", "#extension GL_ARB_texture_rectangle : enable\n");
		postprocessLuminance = new PostprocessShader("postprocessLuminance", "#extension GL_ARB_texture_rectangle : enable\n");

		glError();
	}
	void DeleteBuffers() {
		delete halfSizeRT;
		delete luminanceRT;
		delete bloom1RT;
		delete bloom2RT;
		delete sceneRT;

		delete postprocessBloom1Downsample;
		delete postprocessBloom2Downsample;
		delete postprocessBloom3VBlur;
		delete postprocessBloom4HBlur;
		delete postprocessCompose;
		delete postprocessLuminance;
	}
	void DoPostprocess() {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0,1.0,0.0,1.0,-1.0,1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_LIGHTING);

		// So, to do proper tone mapping of HDR to LDR we need to know the average luminance
		// of the scene. We do this by rendering the scene's luminance to a smaller texture,
		// generating mipmaps for it, and grabbing the luminance at the smallest mipmap level
		luminanceRT->BeginRTT();
		sceneRT->BindTexture();
		State::UseProgram(postprocessLuminance);
		postprocessLuminance->set_fboTex(0);
		glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0, 0.0);
			glVertex2f(0.0, 0.0);
			glTexCoord2f(float(width), 0.0);
			glVertex2f(1.0, 0.0);
			glTexCoord2f(0.0,float(height));
			glVertex2f(0.0, 1.0);
			glTexCoord2f(float(width), float(height));
			glVertex2f(1.0, 1.0);
		glEnd();
		luminanceRT->EndRTT();
		sceneRT->UnbindTexture();

		luminanceRT->BindTexture();
		luminanceRT->UpdateMipmaps();
		float avgLum[4];
		glGetTexImage(GL_TEXTURE_2D, 7, GL_RGB, GL_FLOAT, avgLum);

		//printf("%f -> ", avgLum[0]);
		avgLum[0] = std::max(float(exp(avgLum[0])), 0.03f);
		//printf("%f\n", avgLum[0]);
		// see reinhard algo
		const float midGrey = 1.03f - 2.0f/(2.0f+log10(avgLum[0] + 1.0f));
		
		glDisable(GL_TEXTURE_2D);
		halfSizeRT->BeginRTT();
		sceneRT->BindTexture();
		State::UseProgram(postprocessBloom1Downsample);
		postprocessBloom1Downsample->set_avgLum(avgLum[0]);
		postprocessBloom1Downsample->set_middleGrey(midGrey);
		postprocessBloom1Downsample->set_fboTex(0);
		glBegin(GL_TRIANGLE_STRIP);
			glVertex2f(0.0, 0.0);
			glVertex2f(1.0, 0.0);
			glVertex2f(0.0, 1.0);
			glVertex2f(1.0, 1.0);
		glEnd();
		halfSizeRT->EndRTT();

		bloom1RT->BeginRTT();
		State::UseProgram(postprocessBloom2Downsample);
		halfSizeRT->BindTexture();
		glBegin(GL_TRIANGLE_STRIP);
			glVertex2f(0.0, 0.0);
			glVertex2f(1.0, 0.0);
			glVertex2f(0.0, 1.0);
			glVertex2f(1.0, 1.0);
		glEnd();
		halfSizeRT->UnbindTexture();
		bloom1RT->EndRTT();
		
		bloom2RT->BeginRTT();
		bloom1RT->BindTexture();
		State::UseProgram(postprocessBloom3VBlur);
		postprocessBloom3VBlur->set_fboTex(0);
		glBegin(GL_TRIANGLE_STRIP);
			glVertex2f(0.0, 0.0);
			glVertex2f(1.0, 0.0);
			glVertex2f(0.0, 1.0);
			glVertex2f(1.0, 1.0);
		glEnd();
		bloom2RT->EndRTT();

		bloom1RT->BeginRTT();
		bloom2RT->BindTexture();
		State::UseProgram(postprocessBloom4HBlur);
		postprocessBloom4HBlur->set_fboTex(0);
		glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0, 0.0);
			glVertex2f(0.0, 0.0);
			glTexCoord2f(1.0, 0.0);
			glVertex2f(1.0, 0.0);
			glTexCoord2f(0.0,1.0);
			glVertex2f(0.0, 1.0);
			glTexCoord2f(1.0, 1.0);
			glVertex2f(1.0, 1.0);
		glEnd();
		bloom1RT->EndRTT();
		
		glViewport(0,0,width,height);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		sceneRT->BindTexture();
		glActiveTexture(GL_TEXTURE1);
		bloom1RT->BindTexture();
		State::UseProgram(postprocessCompose);
		postprocessCompose->set_fboTex(0);
		postprocessCompose->set_bloomTex(1);
		postprocessCompose->set_avgLum(avgLum[0]);
		//printf("Mid grey %f\n", midGrey);
		postprocessCompose->set_middleGrey(midGrey);
		glBegin(GL_TRIANGLE_STRIP);
			glVertex2f(0.0, 0.0);
			glVertex2f(1.0, 0.0);
			glVertex2f(0.0, 1.0);
			glVertex2f(1.0, 1.0);
		glEnd();
		State::UseProgram(0);

		glEnable(GL_DEPTH_TEST);
		bloom1RT->UnbindTexture();
		glActiveTexture(GL_TEXTURE0);
		sceneRT->UnbindTexture();
		glError();
	}
} s_hdrBufs;

void Init(int screen_width, int screen_height)
{
	if (initted) return;

	PrintGLInfo();

	shadersAvailable = glewIsSupported("GL_VERSION_2_0");
	shadersEnabled = shadersAvailable;
	printf("GLSL shaders %s.\n", shadersEnabled ? "on" : "off");

	// Framebuffers for HDR
	hdrAvailable = glewIsSupported("GL_EXT_framebuffer_object GL_ARB_color_buffer_float GL_ARB_texture_rectangle");
	if (hdrAvailable) {
		try {
			s_hdrBufs.CreateBuffers(screen_width, screen_height);
		} catch (RenderTarget::fbo_incomplete &ex) {
			if (ex.GetErrorCode() == GL_FRAMEBUFFER_UNSUPPORTED_EXT) {
				fprintf(stderr, "HDR render targets unsupported: forcing HDR off\n");
			} else {
				fprintf(stderr, "HDR initialization error: %s\n", ex.what());
			}
			s_hdrBufs.DeleteBuffers();
			hdrAvailable = false;
			hdrEnabled = false;
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		}
	}
	
	initted = true;

	if (shadersEnabled) {
		simpleShader = new Shader("simple");
		billboardShader = new BillboardShader("billboard");
		planetRingsShader[0] = new Shader("planetrings", "#define NUM_LIGHTS 1\n");
		planetRingsShader[1] = new Shader("planetrings", "#define NUM_LIGHTS 2\n");
		planetRingsShader[2] = new Shader("planetrings", "#define NUM_LIGHTS 3\n");
		planetRingsShader[3] = new Shader("planetrings", "#define NUM_LIGHTS 4\n");
	}
}

void Uninit()
{
	s_hdrBufs.DeleteBuffers();
	delete simpleShader;
	delete billboardShader;
	delete planetRingsShader[0];
	delete planetRingsShader[1];
	delete planetRingsShader[2];
	delete planetRingsShader[3];
	FreeLibs();
}

bool IsHDREnabled() { return shadersEnabled && hdrEnabled; }
bool IsHDRAvailable() { return hdrAvailable; }

void PrepareFrame()
{
	if (IsHDRAvailable()) {
		if (IsHDREnabled())
			s_hdrBufs.sceneRT->BeginRTT();
		else
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
}

void PostProcess()
{
	if (IsHDREnabled()) {
		s_hdrBufs.sceneRT->EndRTT();
		s_hdrBufs.DoPostprocess();
	}
}

void SwapBuffers()
{
	SDL_GL_SwapBuffers();
}

bool AreShadersEnabled()
{
	return shadersEnabled;
}

void ToggleShaders()
{
	if (shadersAvailable) {
		shadersEnabled = (shadersEnabled ? false : true);
	}
	printf("GLSL shaders %s.\n", shadersEnabled ? "on" : "off");
}

void ToggleHDR()
{
	if (hdrAvailable)
		hdrEnabled = !hdrEnabled;
	printf("HDR lighting %s.\n", hdrEnabled ? "enabled" : "disabled");
}

void GetNearFarClipPlane(float &znear, float &zfar)
{
	if (shadersEnabled) {
		/* If vertex shaders are enabled then we have a lovely logarithmic
		 * z-buffer stretching out from 0.1mm to 10000km! */
		znear = 0.0001f;
		zfar = 10000000.0f;
	} else {
		/* Otherwise we have the usual hopelessly crap z-buffer */
		znear = 10.0f;
		zfar = 1000000.0f;
	}
}

/**
 * So if we are using the z-hack VPROG_POINTSPRITE then this still works.
 * Desired texture should already be bound on calling PutPointSprites()
 */
void PutPointSprites(int num, vector3f *v, float size, const float modulationCol[4], int stride)
{
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);

//	float quadratic[] =  { 0.0f, 0.0f, 0.00001f };
//	glPointParameterfv( GL_POINT_DISTANCE_ATTENUATION, quadratic );
//	glPointParameterf(GL_POINT_SIZE_MIN, 1.0 );
//	glPointParameterf(GL_POINT_SIZE_MAX, 10000.0 );
		
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glColor4fv(modulationCol);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);	

	// XXX point sprite thing needs some work. remember to enable point
	// sprite shader in LmrModel.cpp
	if (AreShadersEnabled()) {
		// this is a bit dumb since it doesn't care how many lights
		// the scene has, and this is a constant...
		State::UseProgram(billboardShader);
		billboardShader->set_some_texture(0);
	}

//	/*if (Shader::IsVtxProgActive())*/ glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
	if (0) {//GLEW_ARB_point_sprite) {
		glTexEnvf(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
		glEnable(GL_POINT_SPRITE_ARB);
		
		glPointSize(size);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, v);
		glDrawArrays(GL_POINTS, 0, num);
		glDisableClientState(GL_VERTEX_ARRAY);
		glPointSize(1);

		glDisable(GL_POINT_SPRITE_ARB);
		glDisable(GL_TEXTURE_2D);
	
	} else {
		// quad billboards
		matrix4x4f rot;
		glGetFloatv(GL_MODELVIEW_MATRIX, &rot[0]);
		rot.ClearToRotOnly();
		rot = rot.InverseOf();

		const float sz = 0.5f*size;
		const vector3f rotv1 = rot * vector3f(sz, sz, 0.0f);
		const vector3f rotv2 = rot * vector3f(sz, -sz, 0.0f);
		const vector3f rotv3 = rot * vector3f(-sz, -sz, 0.0f);
		const vector3f rotv4 = rot * vector3f(-sz, sz, 0.0f);

		glBegin(GL_QUADS);
		for (int i=0; i<num; i++) {
			vector3f pos(*v);
			vector3f vert;

			vert = pos+rotv4;
			glTexCoord2f(0.0f,0.0f);
			glVertex3f(vert.x, vert.y, vert.z);
			
			vert = pos+rotv3;
			glTexCoord2f(0.0f,1.0f);
			glVertex3f(vert.x, vert.y, vert.z);
			
			vert = pos+rotv2;
			glTexCoord2f(1.0f,1.0f);
			glVertex3f(vert.x, vert.y, vert.z);
			
			vert = pos+rotv1;
			glTexCoord2f(1.0f,0.0f);
			glVertex3f(vert.x, vert.y, vert.z);
			
			v = reinterpret_cast<vector3f*>(reinterpret_cast<char*>(v)+stride);
		}
		glEnd();
	}
//	/*if (Shader::IsVtxProgActive())*/ glDisable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);

//	quadratic[0] = 1; quadratic[1] = 0;
//	glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );

	glDisable(GL_TEXTURE_2D);
	glDepthMask(GL_TRUE);
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
}

bool State::UseProgram(Shader *shader)
{
	if (shadersEnabled) {
		if (shader) {
			if (m_currentShader != shader) {
				m_currentShader = shader;
				glUseProgram(shader->GetProgram());
				shader->set_invLogZfarPlus1(m_invLogZfarPlus1);
				return true;
			} else {
				return false;
			}
		} else {
			m_currentShader = 0;
			glUseProgram(0);
			return true;
		}
	} else {
		return false;
	}
}

void PrintGLInfo() {
	std::string fname = GetPiUserDir() + "opengl.txt";
	FILE *f = fopen(fname.c_str(), "w");
	if (!f) return;

	std::ostringstream ss;
	ss << "OpenGL version " << glGetString(GL_VERSION);
	ss << ", running on " << glGetString(GL_VENDOR);
	ss << " " << glGetString(GL_RENDERER) << std::endl;

	ss << "Available extensions:" << std::endl;
	GLint numext = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numext);
	if (glewIsSupported("GL_VERSION_3_0")) {
		for (int i = 0; i < numext; ++i) {
			ss << "  " << glGetStringi(GL_EXTENSIONS, i) << std::endl;
		}
	}
	else {
		ss << "  ";
		std::istringstream ext(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
		std::copy(
			std::istream_iterator<std::string>(ext),
			std::istream_iterator<std::string>(),
			std::ostream_iterator<std::string>(ss, "\n  "));
	}

	fprintf(f, "%s", ss.str().c_str());
	fclose(f);
	printf("OpenGL system information saved to %s\n", fname.c_str());
}

} /* namespace Render */
