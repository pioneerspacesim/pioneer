#include "Render.h"

static GLuint boundArrayBufferObject = 0;
static GLuint boundElementArrayBufferObject = 0;

namespace Render {

static bool initted = false;
static bool shadersEnabled;
static bool shadersAvailable;
static bool isHDREnabled = false;
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
	GLuint fb, halfsizeFb, bloomFb1, bloomFb2, tex, halfsizeTex, bloomTex1, bloomTex2, depthbuffer, luminanceFb, luminanceTex;
	int width, height;
	postprocessBuffers_t() {
		memset(this, 0, sizeof(postprocessBuffers_t));
	}
	bool CheckFBO()
	{
		GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			fprintf(stderr, "Hm. Framebuffer status 0x%x. No HDR for you pal.\n", int(status));
			return false;
		} else {
			return true;
		}
	}
	void CreateBuffers(int screen_width, int screen_height) {
		width = screen_width;
		height = screen_height;
		glGenFramebuffersEXT(1, &halfsizeFb);
		glGenTextures(1, &halfsizeTex);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, halfsizeFb);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, halfsizeTex);
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB16F_ARB, width>>1, height>>1, 0, GL_RGB, GL_HALF_FLOAT_ARB, NULL);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, halfsizeTex, 0);
		if (!CheckFBO()) {
			DeleteBuffers();
			return;
		}
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		glGenFramebuffersEXT(1, &luminanceFb);
		glGenTextures(1, &luminanceTex);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, luminanceFb);
		glBindTexture(GL_TEXTURE_2D, luminanceTex);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F_ARB, 128, 128, 0, GL_RGB, GL_FLOAT, NULL);
		glGenerateMipmapEXT(GL_TEXTURE_2D);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, luminanceTex, 0);
		if (!CheckFBO()) {
			DeleteBuffers();
			return;
		}
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glError();

		glGenFramebuffersEXT(1, &bloomFb1);
		glGenTextures(1, &bloomTex1);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, bloomFb1);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, bloomTex1);
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB16F_ARB, width>>2, height>>2, 0, GL_RGB, GL_HALF_FLOAT_ARB, NULL);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, bloomTex1, 0);
		if (!CheckFBO()) {
			DeleteBuffers();
			return;
		}
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		glGenFramebuffersEXT(1, &bloomFb2);
		glGenTextures(1, &bloomTex2);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, bloomFb2);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, bloomTex2);
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB16F_ARB, width>>2, height>>2, 0, GL_RGB, GL_HALF_FLOAT_ARB, NULL);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, bloomTex2, 0);
		if (!CheckFBO()) {
			DeleteBuffers();
			return;
		}
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		glGenFramebuffersEXT(1, &fb);
		glGenTextures(1, &tex);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex);
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB16F_ARB, width, height, 0, GL_RGB, GL_HALF_FLOAT_ARB, NULL);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, tex, 0);
		glError();
		
		glGenRenderbuffersEXT(1, &depthbuffer);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthbuffer);
		glError();
		if (!CheckFBO()) {
			DeleteBuffers();
			return;
		}

		postprocessBloom1Downsample = new PostprocessDownsampleShader("postprocessBloom1Downsample");
		postprocessBloom2Downsample = new PostprocessShader("postprocessBloom2Downsample");
		postprocessBloom3VBlur = new PostprocessShader("postprocessBloom3VBlur");
		postprocessBloom4HBlur = new PostprocessShader("postprocessBloom4HBlur");
		postprocessCompose = new PostprocessComposeShader("postprocessCompose");
		postprocessLuminance = new PostprocessShader("postprocessLuminance");

		glError();
	}
	void DeleteBuffers() {
		if (fb) glDeleteFramebuffersEXT(1, &fb);
		if (halfsizeFb) glDeleteFramebuffersEXT(1, &halfsizeFb);
		if (bloomFb1) glDeleteFramebuffersEXT(1, &bloomFb1);
		if (bloomFb2) glDeleteFramebuffersEXT(1, &bloomFb2);
		if (luminanceFb) glDeleteFramebuffersEXT(1, &luminanceFb);
		fb = halfsizeFb = bloomFb1 = bloomFb2 = luminanceFb = 0;
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
		glViewport(0,0,128,128);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, luminanceFb);
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex);
		State::UseProgram(postprocessLuminance);
		postprocessLuminance->set_fboTex(0);
		glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0, 0.0);
			glVertex2f(0.0, 0.0);
			glTexCoord2f(width, 0.0);
			glVertex2f(1.0, 0.0);
			glTexCoord2f(0.0,height);
			glVertex2f(0.0, 1.0);
			glTexCoord2f(width, height);
			glVertex2f(1.0, 1.0);
		glEnd();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glDisable(GL_TEXTURE_RECTANGLE_ARB);
		glBindTexture(GL_TEXTURE_2D, luminanceTex);
		glEnable(GL_TEXTURE_2D);
		glGenerateMipmapEXT(GL_TEXTURE_2D);
		float avgLum[4];
		glGetTexImage(GL_TEXTURE_2D, 7, GL_RGB, GL_FLOAT, avgLum);

		//printf("%f -> ", avgLum[0]);
		avgLum[0] = std::max(float(exp(avgLum[0])), 0.03f);
		//printf("%f\n", avgLum[0]);
		// see reinhard algo
		const float midGrey = 1.03f - 2.0f/(2.0f+log10(avgLum[0] + 1.0f));
		
		glDisable(GL_TEXTURE_2D);
		glViewport(0,0,width>>1,height>>1);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, halfsizeFb);
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex);
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

		glViewport(0,0,width>>2,height>>2);
		State::UseProgram(postprocessBloom2Downsample);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, bloomFb1);
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, halfsizeTex);
		glBegin(GL_TRIANGLE_STRIP);
			glVertex2f(0.0, 0.0);
			glVertex2f(1.0, 0.0);
			glVertex2f(0.0, 1.0);
			glVertex2f(1.0, 1.0);
		glEnd();
		
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, bloomFb2);
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, bloomTex1);
		State::UseProgram(postprocessBloom3VBlur);
		postprocessBloom3VBlur->set_fboTex(0);
		glBegin(GL_TRIANGLE_STRIP);
			glVertex2f(0.0, 0.0);
			glVertex2f(1.0, 0.0);
			glVertex2f(0.0, 1.0);
			glVertex2f(1.0, 1.0);
		glEnd();

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, bloomFb1);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, bloomTex2);
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
		
		glViewport(0,0,width,height);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex);
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, bloomTex1);
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
#if 0
		glViewport(0,0,width,height);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_RECTANGLE_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
		glColor3f(1.0,1.0,1.0);
		glBindTexture(GL_TEXTURE_2D, luminanceTex);
		State::UseProgram(0);
		glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0, 0.0);
			glVertex2f(0.0, 0.0);
			glTexCoord2f(1.0, 0.0);
			glVertex2f(1.0, 0.0);
			glTexCoord2f(0.0, 1.0);
			glVertex2f(0.0, 1.0);
			glTexCoord2f(1.0, 1.0);
			glVertex2f(1.0, 1.0);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
		glDisable(GL_TEXTURE_2D);
#endif
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_RECTANGLE_ARB);
		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_TEXTURE_RECTANGLE_ARB);
		glError();
	}
	bool Initted() { return fb ? true : false; }
} s_hdrBufs;

void Init(int screen_width, int screen_height)
{
	if (initted) return;
	shadersAvailable = (GLEW_VERSION_2_0 ? true : false);
	shadersEnabled = shadersAvailable;
	printf("GLSL shaders %s.\n", shadersEnabled ? "on" : "off");
	// Framebuffers for HDR
	if (GLEW_EXT_framebuffer_object && GLEW_ARB_color_buffer_float && GLEW_ARB_texture_rectangle) { // && GLEW_ARB_depth_buffer_float) {
		s_hdrBufs.CreateBuffers(screen_width, screen_height);
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

bool IsHDREnabled() { return (s_hdrBufs.Initted() && isHDREnabled && shadersEnabled) ? 1 : 0; }

bool IsHDRAvailable() { return s_hdrBufs.Initted() ? true : false; }

void PrepareFrame()
{
	if (IsHDREnabled()) {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, s_hdrBufs.fb);
	} else {
		if (GLEW_EXT_framebuffer_object) glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
}

void PostProcess()
{
	if (IsHDREnabled()) {
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
	isHDREnabled = !isHDREnabled;
	printf("HDR lighting %s.\n", isHDREnabled ? "enabled" : "disabled");
}

/*
 * So if we are using the z-hack VPROG_POINTSPRITE then this still works.
 */
void PutPointSprites(int num, vector3f *v, float size, const float modulationCol[4], GLuint tex, int stride)
{
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);

//	float quadratic[] =  { 0.0f, 0.0f, 0.00001f };
//	glPointParameterfv( GL_POINT_DISTANCE_ATTENUATION, quadratic );
//	glPointParameterf(GL_POINT_SIZE_MIN, 1.0 );
//	glPointParameterf(GL_POINT_SIZE_MAX, 10000.0 );
		
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
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

// -------------- class Shader ----------------


static __attribute((malloc)) char *load_file(const char *filename)
{
	FILE *f = fopen(filename, "rb");
	if (!f) {
		//printf("Could not open %s.\n", filename);
		return 0;
	}
	fseek(f, 0, SEEK_END);
	size_t len = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = static_cast<char*>(malloc(sizeof(char) * (len+1)));
	fread(buf, 1, len, f);
	fclose(f);
	buf[len] = 0;
	return buf;
}

static void PrintGLSLCompileError(const char *filename, GLuint obj)
{
	int infologLength = 0;
	char infoLog[1024];

	if (glIsShader(obj))
		glGetShaderInfoLog(obj, 1024, &infologLength, infoLog);
	else
		glGetProgramInfoLog(obj, 1024, &infologLength, infoLog);

	if (infologLength > 0) {
		Warning("Error compiling shader: %s: %s\nOpenGL vendor: %s\nOpenGL renderer string: %s\n\nPioneer will run with shaders disabled.",
				filename, infoLog, glGetString(GL_VENDOR), glGetString(GL_RENDERER));
		shadersAvailable = false;
		shadersEnabled = false;
	}
}
	
bool Shader::Compile(const char *shader_name, const char *additional_defines)
{
	GLuint vs, ps = 0;
	std::vector<const char*> shader_src;

	if (!shadersAvailable) {
		m_program = 0;
		return false;
	}
	static char *lib_fs = 0;
	static char *lib_vs = 0;
	static char *lib_all = 0;
	if (!lib_fs) lib_fs = load_file(PIONEER_DATA_DIR"/shaders/_library.frag.glsl");
	if (!lib_vs) lib_vs = load_file(PIONEER_DATA_DIR"/shaders/_library.vert.glsl");
	if (!lib_all) lib_all = load_file(PIONEER_DATA_DIR"/shaders/_library.all.glsl");

	const std::string name = std::string(PIONEER_DATA_DIR"/shaders/") + shader_name;
	char *vscode = load_file((name + ".vert.glsl").c_str());
	char *pscode = load_file((name + ".frag.glsl").c_str());
	char *allcode = load_file((name + ".all.glsl").c_str());
	
	if (vscode == 0) {
		Warning("Could not find shader %s.", (name + ".vert.glsl").c_str());
		goto fail;
	}
		
	vs = glCreateShader(GL_VERTEX_SHADER);

	if (additional_defines) shader_src.push_back(additional_defines);
	shader_src.push_back("#define ZHACK 1\n");
	shader_src.push_back(lib_all);
	shader_src.push_back(lib_vs);
	if (allcode) shader_src.push_back(allcode);
	shader_src.push_back(vscode);

	glShaderSource(vs, shader_src.size(), &shader_src[0], 0);
	glCompileShader(vs);
	GLint status;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	if (!status) {
		PrintGLSLCompileError((name + ".vert.glsl").c_str(), vs);
		goto fail;
	}

	if (pscode) {
		shader_src.clear();
		if (additional_defines) shader_src.push_back(additional_defines);
		shader_src.push_back("#define ZHACK 1\n");
		shader_src.push_back(lib_all);
		shader_src.push_back(lib_fs);
		if (allcode) shader_src.push_back(allcode);
		shader_src.push_back(pscode);
		
		ps = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(ps, shader_src.size(), &shader_src[0], 0);
		glCompileShader(ps);
		glGetShaderiv(ps, GL_COMPILE_STATUS, &status);
		if (!status) {
			PrintGLSLCompileError((name + ".frag.glsl").c_str(), ps);
			goto fail;
		}
	}

	m_program = glCreateProgram();
	glAttachShader(m_program, vs);
	if (pscode) glAttachShader(m_program, ps);
	glLinkProgram(m_program);
	glGetProgramiv(m_program, GL_LINK_STATUS, &status);
	if (!status) {
		PrintGLSLCompileError(name.c_str(), m_program);
		goto fail;
	}

	free(vscode);
	free(pscode);
	free(allcode);
	
	return true;

fail:
	free(vscode);
	free(pscode);
	free(allcode);

	m_program = 0;
	return false;
}

// --------------- class Shader ------------------

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

} /* namespace Render */
