#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "gl_core_3_x.h"

#if defined(__APPLE__)
#include <dlfcn.h>

static void* AppleGLGetProcAddress (const char *name)
{
    static void* image = NULL;
    if (NULL == image)
        image = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);

    return (image ? dlsym(image, name) : NULL);
}
#endif /* __APPLE__ */

#if defined(__sgi) || defined (__sun)
#include <dlfcn.h>
#include <stdio.h>

static void* SunGetProcAddress (const GLubyte* name)
{
  static void* h = NULL;
  static void* gpa;

  if (h == NULL)
  {
    if ((h = dlopen(NULL, RTLD_LAZY | RTLD_LOCAL)) == NULL) return NULL;
    gpa = dlsym(h, "glXGetProcAddress");
  }

  if (gpa != NULL)
    return ((void*(*)(const GLubyte*))gpa)(name);
  else
    return dlsym(h, (const char*)name);
}
#endif /* __sgi || __sun */

#if defined(_WIN32)

#ifdef _MSC_VER
#pragma warning(disable: 4055)
#pragma warning(disable: 4054)
#endif

static int TestPointer(const PROC pTest)
{
	ptrdiff_t iTest;
	if(!pTest) return 0;
	iTest = (ptrdiff_t)pTest;
	
	if(iTest == 1 || iTest == 2 || iTest == 3 || iTest == -1) return 0;
	
	return 1;
}

static PROC WinGetProcAddress(const char *name)
{
	HMODULE glMod = NULL;
	PROC pFunc = wglGetProcAddress((LPCSTR)name);
	if(TestPointer(pFunc))
	{
		return pFunc;
	}
	glMod = GetModuleHandleA("OpenGL32.dll");
	return (PROC)GetProcAddress(glMod, (LPCSTR)name);
}
	
#define IntGetProcAddress(name) WinGetProcAddress(name)
#else
	#if defined(__APPLE__)
		#define IntGetProcAddress(name) AppleGLGetProcAddress(name)
	#else
		#if defined(__sgi) || defined(__sun)
			#define IntGetProcAddress(name) SunGetProcAddress(name)
		#else /* GLX */
		    #include <GL/glx.h>

			#define IntGetProcAddress(name) (*glXGetProcAddressARB)((const GLubyte*)name)
		#endif
	#endif
#endif

int ogl_ext_ARB_seamless_cube_map = ogl_LOAD_FAILED;
int ogl_ext_EXT_texture_compression_s3tc = ogl_LOAD_FAILED;
int ogl_ext_EXT_texture_sRGB = ogl_LOAD_FAILED;
int ogl_ext_EXT_texture_filter_anisotropic = ogl_LOAD_FAILED;
int ogl_ext_ARB_compressed_texture_pixel_storage = ogl_LOAD_FAILED;
int ogl_ext_ARB_conservative_depth = ogl_LOAD_FAILED;
int ogl_ext_ARB_ES2_compatibility = ogl_LOAD_FAILED;
int ogl_ext_ARB_get_program_binary = ogl_LOAD_FAILED;
int ogl_ext_ARB_explicit_uniform_location = ogl_LOAD_FAILED;
int ogl_ext_ARB_internalformat_query = ogl_LOAD_FAILED;
int ogl_ext_ARB_internalformat_query2 = ogl_LOAD_FAILED;
int ogl_ext_ARB_map_buffer_alignment = ogl_LOAD_FAILED;
int ogl_ext_ARB_program_interface_query = ogl_LOAD_FAILED;
int ogl_ext_ARB_separate_shader_objects = ogl_LOAD_FAILED;
int ogl_ext_ARB_shading_language_420pack = ogl_LOAD_FAILED;
int ogl_ext_ARB_shading_language_packing = ogl_LOAD_FAILED;
int ogl_ext_ARB_texture_buffer_range = ogl_LOAD_FAILED;
int ogl_ext_ARB_texture_storage = ogl_LOAD_FAILED;
int ogl_ext_ARB_texture_view = ogl_LOAD_FAILED;
int ogl_ext_ARB_vertex_attrib_binding = ogl_LOAD_FAILED;
int ogl_ext_ARB_viewport_array = ogl_LOAD_FAILED;
int ogl_ext_ARB_arrays_of_arrays = ogl_LOAD_FAILED;
int ogl_ext_ARB_clear_buffer_object = ogl_LOAD_FAILED;
int ogl_ext_ARB_copy_image = ogl_LOAD_FAILED;
int ogl_ext_ARB_ES3_compatibility = ogl_LOAD_FAILED;
int ogl_ext_ARB_fragment_layer_viewport = ogl_LOAD_FAILED;
int ogl_ext_ARB_framebuffer_no_attachments = ogl_LOAD_FAILED;
int ogl_ext_ARB_invalidate_subdata = ogl_LOAD_FAILED;
int ogl_ext_ARB_robust_buffer_access_behavior = ogl_LOAD_FAILED;
int ogl_ext_ARB_stencil_texturing = ogl_LOAD_FAILED;
int ogl_ext_ARB_texture_query_levels = ogl_LOAD_FAILED;
int ogl_ext_ARB_texture_storage_multisample = ogl_LOAD_FAILED;
int ogl_ext_KHR_debug = ogl_LOAD_FAILED;

void (CODEGEN_FUNCPTR *_ptrc_glClearDepthf)(GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDepthRangef)(GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetShaderPrecisionFormat)(GLenum, GLenum, GLint *, GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glReleaseShaderCompiler)() = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glShaderBinary)(GLsizei, const GLuint *, GLenum, const GLvoid *, GLsizei) = NULL;

static int Load_ARB_ES2_compatibility()
{
	int numFailed = 0;
	_ptrc_glClearDepthf = (void (CODEGEN_FUNCPTR *)(GLfloat))IntGetProcAddress("glClearDepthf");
	if(!_ptrc_glClearDepthf) numFailed++;
	_ptrc_glDepthRangef = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat))IntGetProcAddress("glDepthRangef");
	if(!_ptrc_glDepthRangef) numFailed++;
	_ptrc_glGetShaderPrecisionFormat = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *, GLint *))IntGetProcAddress("glGetShaderPrecisionFormat");
	if(!_ptrc_glGetShaderPrecisionFormat) numFailed++;
	_ptrc_glReleaseShaderCompiler = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glReleaseShaderCompiler");
	if(!_ptrc_glReleaseShaderCompiler) numFailed++;
	_ptrc_glShaderBinary = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *, GLenum, const GLvoid *, GLsizei))IntGetProcAddress("glShaderBinary");
	if(!_ptrc_glShaderBinary) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glGetProgramBinary)(GLuint, GLsizei, GLsizei *, GLenum *, GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramBinary)(GLuint, GLenum, const GLvoid *, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramParameteri)(GLuint, GLenum, GLint) = NULL;

static int Load_ARB_get_program_binary()
{
	int numFailed = 0;
	_ptrc_glGetProgramBinary = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLsizei *, GLenum *, GLvoid *))IntGetProcAddress("glGetProgramBinary");
	if(!_ptrc_glGetProgramBinary) numFailed++;
	_ptrc_glProgramBinary = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLvoid *, GLsizei))IntGetProcAddress("glProgramBinary");
	if(!_ptrc_glProgramBinary) numFailed++;
	_ptrc_glProgramParameteri = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint))IntGetProcAddress("glProgramParameteri");
	if(!_ptrc_glProgramParameteri) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glGetInternalformativ)(GLenum, GLenum, GLenum, GLsizei, GLint *) = NULL;

static int Load_ARB_internalformat_query()
{
	int numFailed = 0;
	_ptrc_glGetInternalformativ = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLsizei, GLint *))IntGetProcAddress("glGetInternalformativ");
	if(!_ptrc_glGetInternalformativ) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glGetInternalformati64v)(GLenum, GLenum, GLenum, GLsizei, GLint64 *) = NULL;

static int Load_ARB_internalformat_query2()
{
	int numFailed = 0;
	_ptrc_glGetInternalformati64v = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLsizei, GLint64 *))IntGetProcAddress("glGetInternalformati64v");
	if(!_ptrc_glGetInternalformati64v) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glGetProgramInterfaceiv)(GLuint, GLenum, GLenum, GLint *) = NULL;
GLuint (CODEGEN_FUNCPTR *_ptrc_glGetProgramResourceIndex)(GLuint, GLenum, const GLchar *) = NULL;
GLint (CODEGEN_FUNCPTR *_ptrc_glGetProgramResourceLocation)(GLuint, GLenum, const GLchar *) = NULL;
GLint (CODEGEN_FUNCPTR *_ptrc_glGetProgramResourceLocationIndex)(GLuint, GLenum, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetProgramResourceName)(GLuint, GLenum, GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetProgramResourceiv)(GLuint, GLenum, GLuint, GLsizei, const GLenum *, GLsizei, GLsizei *, GLint *) = NULL;

static int Load_ARB_program_interface_query()
{
	int numFailed = 0;
	_ptrc_glGetProgramInterfaceiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLenum, GLint *))IntGetProcAddress("glGetProgramInterfaceiv");
	if(!_ptrc_glGetProgramInterfaceiv) numFailed++;
	_ptrc_glGetProgramResourceIndex = (GLuint (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLchar *))IntGetProcAddress("glGetProgramResourceIndex");
	if(!_ptrc_glGetProgramResourceIndex) numFailed++;
	_ptrc_glGetProgramResourceLocation = (GLint (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLchar *))IntGetProcAddress("glGetProgramResourceLocation");
	if(!_ptrc_glGetProgramResourceLocation) numFailed++;
	_ptrc_glGetProgramResourceLocationIndex = (GLint (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLchar *))IntGetProcAddress("glGetProgramResourceLocationIndex");
	if(!_ptrc_glGetProgramResourceLocationIndex) numFailed++;
	_ptrc_glGetProgramResourceName = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetProgramResourceName");
	if(!_ptrc_glGetProgramResourceName) numFailed++;
	_ptrc_glGetProgramResourceiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLuint, GLsizei, const GLenum *, GLsizei, GLsizei *, GLint *))IntGetProcAddress("glGetProgramResourceiv");
	if(!_ptrc_glGetProgramResourceiv) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glActiveShaderProgram)(GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glBindProgramPipeline)(GLuint) = NULL;
GLuint (CODEGEN_FUNCPTR *_ptrc_glCreateShaderProgramv)(GLenum, GLsizei, const GLchar *const*) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDeleteProgramPipelines)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGenProgramPipelines)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetProgramPipelineInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetProgramPipelineiv)(GLuint, GLenum, GLint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsProgramPipeline)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform1d)(GLuint, GLint, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform1dv)(GLuint, GLint, GLsizei, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform1f)(GLuint, GLint, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform1fv)(GLuint, GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform1i)(GLuint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform1iv)(GLuint, GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform1ui)(GLuint, GLint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform1uiv)(GLuint, GLint, GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform2d)(GLuint, GLint, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform2dv)(GLuint, GLint, GLsizei, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform2f)(GLuint, GLint, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform2fv)(GLuint, GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform2i)(GLuint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform2iv)(GLuint, GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform2ui)(GLuint, GLint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform2uiv)(GLuint, GLint, GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform3d)(GLuint, GLint, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform3dv)(GLuint, GLint, GLsizei, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform3f)(GLuint, GLint, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform3fv)(GLuint, GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform3i)(GLuint, GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform3iv)(GLuint, GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform3ui)(GLuint, GLint, GLuint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform3uiv)(GLuint, GLint, GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform4d)(GLuint, GLint, GLdouble, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform4dv)(GLuint, GLint, GLsizei, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform4f)(GLuint, GLint, GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform4fv)(GLuint, GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform4i)(GLuint, GLint, GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform4iv)(GLuint, GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform4ui)(GLuint, GLint, GLuint, GLuint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniform4uiv)(GLuint, GLint, GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix2dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix2fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix2x3dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix2x3fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix2x4dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix2x4fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix3dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix3fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix3x2dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix3x2fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix3x4dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix3x4fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix4dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix4fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix4x2dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix4x2fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix4x3dv)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProgramUniformMatrix4x3fv)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUseProgramStages)(GLuint, GLbitfield, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glValidateProgramPipeline)(GLuint) = NULL;

static int Load_ARB_separate_shader_objects()
{
	int numFailed = 0;
	_ptrc_glActiveShaderProgram = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glActiveShaderProgram");
	if(!_ptrc_glActiveShaderProgram) numFailed++;
	_ptrc_glBindProgramPipeline = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glBindProgramPipeline");
	if(!_ptrc_glBindProgramPipeline) numFailed++;
	_ptrc_glCreateShaderProgramv = (GLuint (CODEGEN_FUNCPTR *)(GLenum, GLsizei, const GLchar *const*))IntGetProcAddress("glCreateShaderProgramv");
	if(!_ptrc_glCreateShaderProgramv) numFailed++;
	_ptrc_glDeleteProgramPipelines = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteProgramPipelines");
	if(!_ptrc_glDeleteProgramPipelines) numFailed++;
	_ptrc_glGenProgramPipelines = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenProgramPipelines");
	if(!_ptrc_glGenProgramPipelines) numFailed++;
	_ptrc_glGetProgramPipelineInfoLog = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetProgramPipelineInfoLog");
	if(!_ptrc_glGetProgramPipelineInfoLog) numFailed++;
	_ptrc_glGetProgramPipelineiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetProgramPipelineiv");
	if(!_ptrc_glGetProgramPipelineiv) numFailed++;
	_ptrc_glIsProgramPipeline = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsProgramPipeline");
	if(!_ptrc_glIsProgramPipeline) numFailed++;
	_ptrc_glProgramUniform1d = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLdouble))IntGetProcAddress("glProgramUniform1d");
	if(!_ptrc_glProgramUniform1d) numFailed++;
	_ptrc_glProgramUniform1dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLdouble *))IntGetProcAddress("glProgramUniform1dv");
	if(!_ptrc_glProgramUniform1dv) numFailed++;
	_ptrc_glProgramUniform1f = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLfloat))IntGetProcAddress("glProgramUniform1f");
	if(!_ptrc_glProgramUniform1f) numFailed++;
	_ptrc_glProgramUniform1fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLfloat *))IntGetProcAddress("glProgramUniform1fv");
	if(!_ptrc_glProgramUniform1fv) numFailed++;
	_ptrc_glProgramUniform1i = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint))IntGetProcAddress("glProgramUniform1i");
	if(!_ptrc_glProgramUniform1i) numFailed++;
	_ptrc_glProgramUniform1iv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLint *))IntGetProcAddress("glProgramUniform1iv");
	if(!_ptrc_glProgramUniform1iv) numFailed++;
	_ptrc_glProgramUniform1ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLuint))IntGetProcAddress("glProgramUniform1ui");
	if(!_ptrc_glProgramUniform1ui) numFailed++;
	_ptrc_glProgramUniform1uiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLuint *))IntGetProcAddress("glProgramUniform1uiv");
	if(!_ptrc_glProgramUniform1uiv) numFailed++;
	_ptrc_glProgramUniform2d = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLdouble, GLdouble))IntGetProcAddress("glProgramUniform2d");
	if(!_ptrc_glProgramUniform2d) numFailed++;
	_ptrc_glProgramUniform2dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLdouble *))IntGetProcAddress("glProgramUniform2dv");
	if(!_ptrc_glProgramUniform2dv) numFailed++;
	_ptrc_glProgramUniform2f = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLfloat, GLfloat))IntGetProcAddress("glProgramUniform2f");
	if(!_ptrc_glProgramUniform2f) numFailed++;
	_ptrc_glProgramUniform2fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLfloat *))IntGetProcAddress("glProgramUniform2fv");
	if(!_ptrc_glProgramUniform2fv) numFailed++;
	_ptrc_glProgramUniform2i = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint, GLint))IntGetProcAddress("glProgramUniform2i");
	if(!_ptrc_glProgramUniform2i) numFailed++;
	_ptrc_glProgramUniform2iv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLint *))IntGetProcAddress("glProgramUniform2iv");
	if(!_ptrc_glProgramUniform2iv) numFailed++;
	_ptrc_glProgramUniform2ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLuint, GLuint))IntGetProcAddress("glProgramUniform2ui");
	if(!_ptrc_glProgramUniform2ui) numFailed++;
	_ptrc_glProgramUniform2uiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLuint *))IntGetProcAddress("glProgramUniform2uiv");
	if(!_ptrc_glProgramUniform2uiv) numFailed++;
	_ptrc_glProgramUniform3d = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glProgramUniform3d");
	if(!_ptrc_glProgramUniform3d) numFailed++;
	_ptrc_glProgramUniform3dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLdouble *))IntGetProcAddress("glProgramUniform3dv");
	if(!_ptrc_glProgramUniform3dv) numFailed++;
	_ptrc_glProgramUniform3f = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glProgramUniform3f");
	if(!_ptrc_glProgramUniform3f) numFailed++;
	_ptrc_glProgramUniform3fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLfloat *))IntGetProcAddress("glProgramUniform3fv");
	if(!_ptrc_glProgramUniform3fv) numFailed++;
	_ptrc_glProgramUniform3i = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint, GLint, GLint))IntGetProcAddress("glProgramUniform3i");
	if(!_ptrc_glProgramUniform3i) numFailed++;
	_ptrc_glProgramUniform3iv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLint *))IntGetProcAddress("glProgramUniform3iv");
	if(!_ptrc_glProgramUniform3iv) numFailed++;
	_ptrc_glProgramUniform3ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLuint, GLuint, GLuint))IntGetProcAddress("glProgramUniform3ui");
	if(!_ptrc_glProgramUniform3ui) numFailed++;
	_ptrc_glProgramUniform3uiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLuint *))IntGetProcAddress("glProgramUniform3uiv");
	if(!_ptrc_glProgramUniform3uiv) numFailed++;
	_ptrc_glProgramUniform4d = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLdouble, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glProgramUniform4d");
	if(!_ptrc_glProgramUniform4d) numFailed++;
	_ptrc_glProgramUniform4dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLdouble *))IntGetProcAddress("glProgramUniform4dv");
	if(!_ptrc_glProgramUniform4dv) numFailed++;
	_ptrc_glProgramUniform4f = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glProgramUniform4f");
	if(!_ptrc_glProgramUniform4f) numFailed++;
	_ptrc_glProgramUniform4fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLfloat *))IntGetProcAddress("glProgramUniform4fv");
	if(!_ptrc_glProgramUniform4fv) numFailed++;
	_ptrc_glProgramUniform4i = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint, GLint, GLint, GLint))IntGetProcAddress("glProgramUniform4i");
	if(!_ptrc_glProgramUniform4i) numFailed++;
	_ptrc_glProgramUniform4iv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLint *))IntGetProcAddress("glProgramUniform4iv");
	if(!_ptrc_glProgramUniform4iv) numFailed++;
	_ptrc_glProgramUniform4ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLuint, GLuint, GLuint, GLuint))IntGetProcAddress("glProgramUniform4ui");
	if(!_ptrc_glProgramUniform4ui) numFailed++;
	_ptrc_glProgramUniform4uiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, const GLuint *))IntGetProcAddress("glProgramUniform4uiv");
	if(!_ptrc_glProgramUniform4uiv) numFailed++;
	_ptrc_glProgramUniformMatrix2dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix2dv");
	if(!_ptrc_glProgramUniformMatrix2dv) numFailed++;
	_ptrc_glProgramUniformMatrix2fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix2fv");
	if(!_ptrc_glProgramUniformMatrix2fv) numFailed++;
	_ptrc_glProgramUniformMatrix2x3dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix2x3dv");
	if(!_ptrc_glProgramUniformMatrix2x3dv) numFailed++;
	_ptrc_glProgramUniformMatrix2x3fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix2x3fv");
	if(!_ptrc_glProgramUniformMatrix2x3fv) numFailed++;
	_ptrc_glProgramUniformMatrix2x4dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix2x4dv");
	if(!_ptrc_glProgramUniformMatrix2x4dv) numFailed++;
	_ptrc_glProgramUniformMatrix2x4fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix2x4fv");
	if(!_ptrc_glProgramUniformMatrix2x4fv) numFailed++;
	_ptrc_glProgramUniformMatrix3dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix3dv");
	if(!_ptrc_glProgramUniformMatrix3dv) numFailed++;
	_ptrc_glProgramUniformMatrix3fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix3fv");
	if(!_ptrc_glProgramUniformMatrix3fv) numFailed++;
	_ptrc_glProgramUniformMatrix3x2dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix3x2dv");
	if(!_ptrc_glProgramUniformMatrix3x2dv) numFailed++;
	_ptrc_glProgramUniformMatrix3x2fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix3x2fv");
	if(!_ptrc_glProgramUniformMatrix3x2fv) numFailed++;
	_ptrc_glProgramUniformMatrix3x4dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix3x4dv");
	if(!_ptrc_glProgramUniformMatrix3x4dv) numFailed++;
	_ptrc_glProgramUniformMatrix3x4fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix3x4fv");
	if(!_ptrc_glProgramUniformMatrix3x4fv) numFailed++;
	_ptrc_glProgramUniformMatrix4dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix4dv");
	if(!_ptrc_glProgramUniformMatrix4dv) numFailed++;
	_ptrc_glProgramUniformMatrix4fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix4fv");
	if(!_ptrc_glProgramUniformMatrix4fv) numFailed++;
	_ptrc_glProgramUniformMatrix4x2dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix4x2dv");
	if(!_ptrc_glProgramUniformMatrix4x2dv) numFailed++;
	_ptrc_glProgramUniformMatrix4x2fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix4x2fv");
	if(!_ptrc_glProgramUniformMatrix4x2fv) numFailed++;
	_ptrc_glProgramUniformMatrix4x3dv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *))IntGetProcAddress("glProgramUniformMatrix4x3dv");
	if(!_ptrc_glProgramUniformMatrix4x3dv) numFailed++;
	_ptrc_glProgramUniformMatrix4x3fv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glProgramUniformMatrix4x3fv");
	if(!_ptrc_glProgramUniformMatrix4x3fv) numFailed++;
	_ptrc_glUseProgramStages = (void (CODEGEN_FUNCPTR *)(GLuint, GLbitfield, GLuint))IntGetProcAddress("glUseProgramStages");
	if(!_ptrc_glUseProgramStages) numFailed++;
	_ptrc_glValidateProgramPipeline = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glValidateProgramPipeline");
	if(!_ptrc_glValidateProgramPipeline) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glTexBufferRange)(GLenum, GLenum, GLuint, GLintptr, GLsizeiptr) = NULL;

static int Load_ARB_texture_buffer_range()
{
	int numFailed = 0;
	_ptrc_glTexBufferRange = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint, GLintptr, GLsizeiptr))IntGetProcAddress("glTexBufferRange");
	if(!_ptrc_glTexBufferRange) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glTexStorage1D)(GLenum, GLsizei, GLenum, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexStorage2D)(GLenum, GLsizei, GLenum, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexStorage3D)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei) = NULL;

static int Load_ARB_texture_storage()
{
	int numFailed = 0;
	_ptrc_glTexStorage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei))IntGetProcAddress("glTexStorage1D");
	if(!_ptrc_glTexStorage1D) numFailed++;
	_ptrc_glTexStorage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei))IntGetProcAddress("glTexStorage2D");
	if(!_ptrc_glTexStorage2D) numFailed++;
	_ptrc_glTexStorage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei))IntGetProcAddress("glTexStorage3D");
	if(!_ptrc_glTexStorage3D) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glTextureView)(GLuint, GLenum, GLuint, GLenum, GLuint, GLuint, GLuint, GLuint) = NULL;

static int Load_ARB_texture_view()
{
	int numFailed = 0;
	_ptrc_glTextureView = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLuint, GLenum, GLuint, GLuint, GLuint, GLuint))IntGetProcAddress("glTextureView");
	if(!_ptrc_glTextureView) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glBindVertexBuffer)(GLuint, GLuint, GLintptr, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribBinding)(GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribFormat)(GLuint, GLint, GLenum, GLboolean, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribIFormat)(GLuint, GLint, GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribLFormat)(GLuint, GLint, GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexBindingDivisor)(GLuint, GLuint) = NULL;

static int Load_ARB_vertex_attrib_binding()
{
	int numFailed = 0;
	_ptrc_glBindVertexBuffer = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLintptr, GLsizei))IntGetProcAddress("glBindVertexBuffer");
	if(!_ptrc_glBindVertexBuffer) numFailed++;
	_ptrc_glVertexAttribBinding = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glVertexAttribBinding");
	if(!_ptrc_glVertexAttribBinding) numFailed++;
	_ptrc_glVertexAttribFormat = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLenum, GLboolean, GLuint))IntGetProcAddress("glVertexAttribFormat");
	if(!_ptrc_glVertexAttribFormat) numFailed++;
	_ptrc_glVertexAttribIFormat = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLenum, GLuint))IntGetProcAddress("glVertexAttribIFormat");
	if(!_ptrc_glVertexAttribIFormat) numFailed++;
	_ptrc_glVertexAttribLFormat = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLenum, GLuint))IntGetProcAddress("glVertexAttribLFormat");
	if(!_ptrc_glVertexAttribLFormat) numFailed++;
	_ptrc_glVertexBindingDivisor = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glVertexBindingDivisor");
	if(!_ptrc_glVertexBindingDivisor) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glDepthRangeArrayv)(GLuint, GLsizei, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDepthRangeIndexed)(GLuint, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetDoublei_v)(GLenum, GLuint, GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetFloati_v)(GLenum, GLuint, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glScissorArrayv)(GLuint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glScissorIndexed)(GLuint, GLint, GLint, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glScissorIndexedv)(GLuint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glViewportArrayv)(GLuint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glViewportIndexedf)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glViewportIndexedfv)(GLuint, const GLfloat *) = NULL;

static int Load_ARB_viewport_array()
{
	int numFailed = 0;
	_ptrc_glDepthRangeArrayv = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, const GLdouble *))IntGetProcAddress("glDepthRangeArrayv");
	if(!_ptrc_glDepthRangeArrayv) numFailed++;
	_ptrc_glDepthRangeIndexed = (void (CODEGEN_FUNCPTR *)(GLuint, GLdouble, GLdouble))IntGetProcAddress("glDepthRangeIndexed");
	if(!_ptrc_glDepthRangeIndexed) numFailed++;
	_ptrc_glGetDoublei_v = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLdouble *))IntGetProcAddress("glGetDoublei_v");
	if(!_ptrc_glGetDoublei_v) numFailed++;
	_ptrc_glGetFloati_v = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLfloat *))IntGetProcAddress("glGetFloati_v");
	if(!_ptrc_glGetFloati_v) numFailed++;
	_ptrc_glScissorArrayv = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, const GLint *))IntGetProcAddress("glScissorArrayv");
	if(!_ptrc_glScissorArrayv) numFailed++;
	_ptrc_glScissorIndexed = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint, GLsizei, GLsizei))IntGetProcAddress("glScissorIndexed");
	if(!_ptrc_glScissorIndexed) numFailed++;
	_ptrc_glScissorIndexedv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLint *))IntGetProcAddress("glScissorIndexedv");
	if(!_ptrc_glScissorIndexedv) numFailed++;
	_ptrc_glViewportArrayv = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, const GLfloat *))IntGetProcAddress("glViewportArrayv");
	if(!_ptrc_glViewportArrayv) numFailed++;
	_ptrc_glViewportIndexedf = (void (CODEGEN_FUNCPTR *)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glViewportIndexedf");
	if(!_ptrc_glViewportIndexedf) numFailed++;
	_ptrc_glViewportIndexedfv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLfloat *))IntGetProcAddress("glViewportIndexedfv");
	if(!_ptrc_glViewportIndexedfv) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glClearBufferData)(GLenum, GLenum, GLenum, GLenum, const void *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glClearBufferSubData)(GLenum, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void *) = NULL;

static int Load_ARB_clear_buffer_object()
{
	int numFailed = 0;
	_ptrc_glClearBufferData = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLenum, const void *))IntGetProcAddress("glClearBufferData");
	if(!_ptrc_glClearBufferData) numFailed++;
	_ptrc_glClearBufferSubData = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void *))IntGetProcAddress("glClearBufferSubData");
	if(!_ptrc_glClearBufferSubData) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glCopyImageSubData)(GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei) = NULL;

static int Load_ARB_copy_image()
{
	int numFailed = 0;
	_ptrc_glCopyImageSubData = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei))IntGetProcAddress("glCopyImageSubData");
	if(!_ptrc_glCopyImageSubData) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glFramebufferParameteri)(GLenum, GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetFramebufferParameteriv)(GLenum, GLenum, GLint *) = NULL;

static int Load_ARB_framebuffer_no_attachments()
{
	int numFailed = 0;
	_ptrc_glFramebufferParameteri = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint))IntGetProcAddress("glFramebufferParameteri");
	if(!_ptrc_glFramebufferParameteri) numFailed++;
	_ptrc_glGetFramebufferParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetFramebufferParameteriv");
	if(!_ptrc_glGetFramebufferParameteriv) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glInvalidateBufferData)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glInvalidateBufferSubData)(GLuint, GLintptr, GLsizeiptr) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glInvalidateFramebuffer)(GLenum, GLsizei, const GLenum *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glInvalidateSubFramebuffer)(GLenum, GLsizei, const GLenum *, GLint, GLint, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glInvalidateTexImage)(GLuint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glInvalidateTexSubImage)(GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei) = NULL;

static int Load_ARB_invalidate_subdata()
{
	int numFailed = 0;
	_ptrc_glInvalidateBufferData = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glInvalidateBufferData");
	if(!_ptrc_glInvalidateBufferData) numFailed++;
	_ptrc_glInvalidateBufferSubData = (void (CODEGEN_FUNCPTR *)(GLuint, GLintptr, GLsizeiptr))IntGetProcAddress("glInvalidateBufferSubData");
	if(!_ptrc_glInvalidateBufferSubData) numFailed++;
	_ptrc_glInvalidateFramebuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, const GLenum *))IntGetProcAddress("glInvalidateFramebuffer");
	if(!_ptrc_glInvalidateFramebuffer) numFailed++;
	_ptrc_glInvalidateSubFramebuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, const GLenum *, GLint, GLint, GLsizei, GLsizei))IntGetProcAddress("glInvalidateSubFramebuffer");
	if(!_ptrc_glInvalidateSubFramebuffer) numFailed++;
	_ptrc_glInvalidateTexImage = (void (CODEGEN_FUNCPTR *)(GLuint, GLint))IntGetProcAddress("glInvalidateTexImage");
	if(!_ptrc_glInvalidateTexImage) numFailed++;
	_ptrc_glInvalidateTexSubImage = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei))IntGetProcAddress("glInvalidateTexSubImage");
	if(!_ptrc_glInvalidateTexSubImage) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glTexStorage2DMultisample)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexStorage3DMultisample)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean) = NULL;

static int Load_ARB_texture_storage_multisample()
{
	int numFailed = 0;
	_ptrc_glTexStorage2DMultisample = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean))IntGetProcAddress("glTexStorage2DMultisample");
	if(!_ptrc_glTexStorage2DMultisample) numFailed++;
	_ptrc_glTexStorage3DMultisample = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean))IntGetProcAddress("glTexStorage3DMultisample");
	if(!_ptrc_glTexStorage3DMultisample) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glDebugMessageCallback)(GLDEBUGPROC, const void *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDebugMessageControl)(GLenum, GLenum, GLenum, GLsizei, const GLuint *, GLboolean) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDebugMessageInsert)(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *) = NULL;
GLuint (CODEGEN_FUNCPTR *_ptrc_glGetDebugMessageLog)(GLuint, GLsizei, GLenum *, GLenum *, GLuint *, GLenum *, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetObjectLabel)(GLenum, GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetObjectPtrLabel)(const void *, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetPointerv)(GLenum, GLvoid **) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glObjectLabel)(GLenum, GLuint, GLsizei, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glObjectPtrLabel)(const void *, GLsizei, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glPopDebugGroup)() = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glPushDebugGroup)(GLenum, GLuint, GLsizei, const GLchar *) = NULL;

static int Load_KHR_debug()
{
	int numFailed = 0;
	_ptrc_glDebugMessageCallback = (void (CODEGEN_FUNCPTR *)(GLDEBUGPROC, const void *))IntGetProcAddress("glDebugMessageCallback");
	if(!_ptrc_glDebugMessageCallback) numFailed++;
	_ptrc_glDebugMessageControl = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLsizei, const GLuint *, GLboolean))IntGetProcAddress("glDebugMessageControl");
	if(!_ptrc_glDebugMessageControl) numFailed++;
	_ptrc_glDebugMessageInsert = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *))IntGetProcAddress("glDebugMessageInsert");
	if(!_ptrc_glDebugMessageInsert) numFailed++;
	_ptrc_glGetDebugMessageLog = (GLuint (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLenum *, GLenum *, GLuint *, GLenum *, GLsizei *, GLchar *))IntGetProcAddress("glGetDebugMessageLog");
	if(!_ptrc_glGetDebugMessageLog) numFailed++;
	_ptrc_glGetObjectLabel = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetObjectLabel");
	if(!_ptrc_glGetObjectLabel) numFailed++;
	_ptrc_glGetObjectPtrLabel = (void (CODEGEN_FUNCPTR *)(const void *, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetObjectPtrLabel");
	if(!_ptrc_glGetObjectPtrLabel) numFailed++;
	_ptrc_glGetPointerv = (void (CODEGEN_FUNCPTR *)(GLenum, GLvoid **))IntGetProcAddress("glGetPointerv");
	if(!_ptrc_glGetPointerv) numFailed++;
	_ptrc_glObjectLabel = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLsizei, const GLchar *))IntGetProcAddress("glObjectLabel");
	if(!_ptrc_glObjectLabel) numFailed++;
	_ptrc_glObjectPtrLabel = (void (CODEGEN_FUNCPTR *)(const void *, GLsizei, const GLchar *))IntGetProcAddress("glObjectPtrLabel");
	if(!_ptrc_glObjectPtrLabel) numFailed++;
	_ptrc_glPopDebugGroup = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glPopDebugGroup");
	if(!_ptrc_glPopDebugGroup) numFailed++;
	_ptrc_glPushDebugGroup = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLsizei, const GLchar *))IntGetProcAddress("glPushDebugGroup");
	if(!_ptrc_glPushDebugGroup) numFailed++;
	return numFailed;
}

void (CODEGEN_FUNCPTR *_ptrc_glBlendFunc)(GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glClear)(GLbitfield) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glClearColor)(GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glClearDepth)(GLdouble) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glClearStencil)(GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glColorMask)(GLboolean, GLboolean, GLboolean, GLboolean) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glCullFace)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDepthFunc)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDepthMask)(GLboolean) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDepthRange)(GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDisable)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDrawBuffer)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glEnable)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glFinish)() = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glFlush)() = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glFrontFace)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetBooleanv)(GLenum, GLboolean *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetDoublev)(GLenum, GLdouble *) = NULL;
GLenum (CODEGEN_FUNCPTR *_ptrc_glGetError)() = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetFloatv)(GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetIntegerv)(GLenum, GLint *) = NULL;
const GLubyte * (CODEGEN_FUNCPTR *_ptrc_glGetString)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetTexImage)(GLenum, GLint, GLenum, GLenum, GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetTexLevelParameterfv)(GLenum, GLint, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetTexLevelParameteriv)(GLenum, GLint, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetTexParameterfv)(GLenum, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetTexParameteriv)(GLenum, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glHint)(GLenum, GLenum) = NULL;
GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsEnabled)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glLineWidth)(GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glLogicOp)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glPixelStoref)(GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glPixelStorei)(GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glPointSize)(GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glPolygonMode)(GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glReadBuffer)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glScissor)(GLint, GLint, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glStencilFunc)(GLenum, GLint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glStencilMask)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glStencilOp)(GLenum, GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexImage1D)(GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexParameterf)(GLenum, GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexParameterfv)(GLenum, GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexParameteri)(GLenum, GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexParameteriv)(GLenum, GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glViewport)(GLint, GLint, GLsizei, GLsizei) = NULL;

void (CODEGEN_FUNCPTR *_ptrc_glBindTexture)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glCopyTexImage1D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glCopyTexImage2D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glCopyTexSubImage1D)(GLenum, GLint, GLint, GLint, GLint, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glCopyTexSubImage2D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDeleteTextures)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDrawArrays)(GLenum, GLint, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDrawElements)(GLenum, GLsizei, GLenum, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGenTextures)(GLsizei, GLuint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsTexture)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glPolygonOffset)(GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexSubImage1D)(GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *) = NULL;

void (CODEGEN_FUNCPTR *_ptrc_glBlendColor)(GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glBlendEquation)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glCopyTexSubImage3D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDrawRangeElements)(GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexImage3D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexSubImage3D)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *) = NULL;

void (CODEGEN_FUNCPTR *_ptrc_glActiveTexture)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glCompressedTexImage1D)(GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glCompressedTexImage2D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glCompressedTexImage3D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glCompressedTexSubImage1D)(GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glCompressedTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glCompressedTexSubImage3D)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetCompressedTexImage)(GLenum, GLint, GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glSampleCoverage)(GLfloat, GLboolean) = NULL;

void (CODEGEN_FUNCPTR *_ptrc_glBlendFuncSeparate)(GLenum, GLenum, GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glMultiDrawArrays)(GLenum, const GLint *, const GLsizei *, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glMultiDrawElements)(GLenum, const GLsizei *, GLenum, const GLvoid *const*, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glPointParameterf)(GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glPointParameterfv)(GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glPointParameteri)(GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glPointParameteriv)(GLenum, const GLint *) = NULL;

void (CODEGEN_FUNCPTR *_ptrc_glBeginQuery)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glBindBuffer)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glBufferData)(GLenum, GLsizeiptr, const GLvoid *, GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glBufferSubData)(GLenum, GLintptr, GLsizeiptr, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDeleteBuffers)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDeleteQueries)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glEndQuery)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGenBuffers)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGenQueries)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetBufferParameteriv)(GLenum, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetBufferPointerv)(GLenum, GLenum, GLvoid **) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetBufferSubData)(GLenum, GLintptr, GLsizeiptr, GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetQueryObjectiv)(GLuint, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetQueryObjectuiv)(GLuint, GLenum, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetQueryiv)(GLenum, GLenum, GLint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsBuffer)(GLuint) = NULL;
GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsQuery)(GLuint) = NULL;
void * (CODEGEN_FUNCPTR *_ptrc_glMapBuffer)(GLenum, GLenum) = NULL;
GLboolean (CODEGEN_FUNCPTR *_ptrc_glUnmapBuffer)(GLenum) = NULL;

void (CODEGEN_FUNCPTR *_ptrc_glAttachShader)(GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glBindAttribLocation)(GLuint, GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glBlendEquationSeparate)(GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glCompileShader)(GLuint) = NULL;
GLuint (CODEGEN_FUNCPTR *_ptrc_glCreateProgram)() = NULL;
GLuint (CODEGEN_FUNCPTR *_ptrc_glCreateShader)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDeleteProgram)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDeleteShader)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDetachShader)(GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDisableVertexAttribArray)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDrawBuffers)(GLsizei, const GLenum *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glEnableVertexAttribArray)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetActiveAttrib)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetActiveUniform)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetAttachedShaders)(GLuint, GLsizei, GLsizei *, GLuint *) = NULL;
GLint (CODEGEN_FUNCPTR *_ptrc_glGetAttribLocation)(GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetProgramInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetProgramiv)(GLuint, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetShaderInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetShaderSource)(GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetShaderiv)(GLuint, GLenum, GLint *) = NULL;
GLint (CODEGEN_FUNCPTR *_ptrc_glGetUniformLocation)(GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetUniformfv)(GLuint, GLint, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetUniformiv)(GLuint, GLint, GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetVertexAttribPointerv)(GLuint, GLenum, GLvoid **) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetVertexAttribdv)(GLuint, GLenum, GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetVertexAttribfv)(GLuint, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetVertexAttribiv)(GLuint, GLenum, GLint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsProgram)(GLuint) = NULL;
GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsShader)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glLinkProgram)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glShaderSource)(GLuint, GLsizei, const GLchar *const*, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glStencilFuncSeparate)(GLenum, GLenum, GLint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glStencilMaskSeparate)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glStencilOpSeparate)(GLenum, GLenum, GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform1f)(GLint, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform1fv)(GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform1i)(GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform1iv)(GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform2f)(GLint, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform2fv)(GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform2i)(GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform2iv)(GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform3f)(GLint, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform3fv)(GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform3i)(GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform3iv)(GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform4fv)(GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform4i)(GLint, GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform4iv)(GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix2fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix3fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUseProgram)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glValidateProgram)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib1d)(GLuint, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib1dv)(GLuint, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib1f)(GLuint, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib1fv)(GLuint, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib1s)(GLuint, GLshort) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib1sv)(GLuint, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib2d)(GLuint, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib2dv)(GLuint, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib2f)(GLuint, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib2fv)(GLuint, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib2s)(GLuint, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib2sv)(GLuint, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib3d)(GLuint, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib3dv)(GLuint, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib3f)(GLuint, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib3fv)(GLuint, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib3s)(GLuint, GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib3sv)(GLuint, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4Nbv)(GLuint, const GLbyte *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4Niv)(GLuint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4Nsv)(GLuint, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4Nub)(GLuint, GLubyte, GLubyte, GLubyte, GLubyte) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4Nubv)(GLuint, const GLubyte *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4Nuiv)(GLuint, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4Nusv)(GLuint, const GLushort *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4bv)(GLuint, const GLbyte *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4d)(GLuint, GLdouble, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4dv)(GLuint, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4f)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4fv)(GLuint, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4iv)(GLuint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4s)(GLuint, GLshort, GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4sv)(GLuint, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4ubv)(GLuint, const GLubyte *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4uiv)(GLuint, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4usv)(GLuint, const GLushort *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *) = NULL;

void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix2x3fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix2x4fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix3x2fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix3x4fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix4x2fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix4x3fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;

void (CODEGEN_FUNCPTR *_ptrc_glBeginConditionalRender)(GLuint, GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glBeginTransformFeedback)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glBindBufferBase)(GLenum, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glBindBufferRange)(GLenum, GLuint, GLuint, GLintptr, GLsizeiptr) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glBindFragDataLocation)(GLuint, GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glBindFramebuffer)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glBindRenderbuffer)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glBindVertexArray)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glBlitFramebuffer)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum) = NULL;
GLenum (CODEGEN_FUNCPTR *_ptrc_glCheckFramebufferStatus)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glClampColor)(GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glClearBufferfi)(GLenum, GLint, GLfloat, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glClearBufferfv)(GLenum, GLint, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glClearBufferiv)(GLenum, GLint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glClearBufferuiv)(GLenum, GLint, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glColorMaski)(GLuint, GLboolean, GLboolean, GLboolean, GLboolean) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDeleteFramebuffers)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDeleteRenderbuffers)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDeleteVertexArrays)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDisablei)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glEnablei)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glEndConditionalRender)() = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glEndTransformFeedback)() = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glFlushMappedBufferRange)(GLenum, GLintptr, GLsizeiptr) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glFramebufferRenderbuffer)(GLenum, GLenum, GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glFramebufferTexture1D)(GLenum, GLenum, GLenum, GLuint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glFramebufferTexture2D)(GLenum, GLenum, GLenum, GLuint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glFramebufferTexture3D)(GLenum, GLenum, GLenum, GLuint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glFramebufferTextureLayer)(GLenum, GLenum, GLuint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGenFramebuffers)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGenRenderbuffers)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGenVertexArrays)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGenerateMipmap)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetBooleani_v)(GLenum, GLuint, GLboolean *) = NULL;
GLint (CODEGEN_FUNCPTR *_ptrc_glGetFragDataLocation)(GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetFramebufferAttachmentParameteriv)(GLenum, GLenum, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetIntegeri_v)(GLenum, GLuint, GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetRenderbufferParameteriv)(GLenum, GLenum, GLint *) = NULL;
const GLubyte * (CODEGEN_FUNCPTR *_ptrc_glGetStringi)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetTexParameterIiv)(GLenum, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetTexParameterIuiv)(GLenum, GLenum, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetTransformFeedbackVarying)(GLuint, GLuint, GLsizei, GLsizei *, GLsizei *, GLenum *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetUniformuiv)(GLuint, GLint, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetVertexAttribIiv)(GLuint, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetVertexAttribIuiv)(GLuint, GLenum, GLuint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsEnabledi)(GLenum, GLuint) = NULL;
GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsFramebuffer)(GLuint) = NULL;
GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsRenderbuffer)(GLuint) = NULL;
GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsVertexArray)(GLuint) = NULL;
void * (CODEGEN_FUNCPTR *_ptrc_glMapBufferRange)(GLenum, GLintptr, GLsizeiptr, GLbitfield) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glRenderbufferStorage)(GLenum, GLenum, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glRenderbufferStorageMultisample)(GLenum, GLsizei, GLenum, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexParameterIiv)(GLenum, GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexParameterIuiv)(GLenum, GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTransformFeedbackVaryings)(GLuint, GLsizei, const GLchar *const*, GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform1ui)(GLint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform1uiv)(GLint, GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform2ui)(GLint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform2uiv)(GLint, GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform3ui)(GLint, GLuint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform3uiv)(GLint, GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform4ui)(GLint, GLuint, GLuint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniform4uiv)(GLint, GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI1i)(GLuint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI1iv)(GLuint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI1ui)(GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI1uiv)(GLuint, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI2i)(GLuint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI2iv)(GLuint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI2ui)(GLuint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI2uiv)(GLuint, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI3i)(GLuint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI3iv)(GLuint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI3ui)(GLuint, GLuint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI3uiv)(GLuint, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI4bv)(GLuint, const GLbyte *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI4i)(GLuint, GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI4iv)(GLuint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI4sv)(GLuint, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI4ubv)(GLuint, const GLubyte *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI4ui)(GLuint, GLuint, GLuint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI4uiv)(GLuint, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribI4usv)(GLuint, const GLushort *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribIPointer)(GLuint, GLint, GLenum, GLsizei, const GLvoid *) = NULL;

void (CODEGEN_FUNCPTR *_ptrc_glCopyBufferSubData)(GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDrawArraysInstanced)(GLenum, GLint, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDrawElementsInstanced)(GLenum, GLsizei, GLenum, const GLvoid *, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetActiveUniformBlockName)(GLuint, GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetActiveUniformBlockiv)(GLuint, GLuint, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetActiveUniformName)(GLuint, GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetActiveUniformsiv)(GLuint, GLsizei, const GLuint *, GLenum, GLint *) = NULL;
GLuint (CODEGEN_FUNCPTR *_ptrc_glGetUniformBlockIndex)(GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetUniformIndices)(GLuint, GLsizei, const GLchar *const*, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glPrimitiveRestartIndex)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexBuffer)(GLenum, GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glUniformBlockBinding)(GLuint, GLuint, GLuint) = NULL;

GLenum (CODEGEN_FUNCPTR *_ptrc_glClientWaitSync)(GLsync, GLbitfield, GLuint64) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDeleteSync)(GLsync) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDrawElementsBaseVertex)(GLenum, GLsizei, GLenum, const GLvoid *, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDrawElementsInstancedBaseVertex)(GLenum, GLsizei, GLenum, const GLvoid *, GLsizei, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDrawRangeElementsBaseVertex)(GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *, GLint) = NULL;
GLsync (CODEGEN_FUNCPTR *_ptrc_glFenceSync)(GLenum, GLbitfield) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glFramebufferTexture)(GLenum, GLenum, GLuint, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetBufferParameteri64v)(GLenum, GLenum, GLint64 *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetInteger64i_v)(GLenum, GLuint, GLint64 *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetInteger64v)(GLenum, GLint64 *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetMultisamplefv)(GLenum, GLuint, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetSynciv)(GLsync, GLenum, GLsizei, GLsizei *, GLint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsSync)(GLsync) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glMultiDrawElementsBaseVertex)(GLenum, const GLsizei *, GLenum, const GLvoid *const*, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glProvokingVertex)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glSampleMaski)(GLuint, GLbitfield) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexImage2DMultisample)(GLenum, GLsizei, GLint, GLsizei, GLsizei, GLboolean) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexImage3DMultisample)(GLenum, GLsizei, GLint, GLsizei, GLsizei, GLsizei, GLboolean) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glWaitSync)(GLsync, GLbitfield, GLuint64) = NULL;

void (CODEGEN_FUNCPTR *_ptrc_glBindFragDataLocationIndexed)(GLuint, GLuint, GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glBindSampler)(GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glColorP3ui)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glColorP3uiv)(GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glColorP4ui)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glColorP4uiv)(GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glDeleteSamplers)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGenSamplers)(GLsizei, GLuint *) = NULL;
GLint (CODEGEN_FUNCPTR *_ptrc_glGetFragDataIndex)(GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetQueryObjecti64v)(GLuint, GLenum, GLint64 *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetQueryObjectui64v)(GLuint, GLenum, GLuint64 *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetSamplerParameterIiv)(GLuint, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetSamplerParameterIuiv)(GLuint, GLenum, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetSamplerParameterfv)(GLuint, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glGetSamplerParameteriv)(GLuint, GLenum, GLint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsSampler)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoordP1ui)(GLenum, GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoordP1uiv)(GLenum, GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoordP2ui)(GLenum, GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoordP2uiv)(GLenum, GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoordP3ui)(GLenum, GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoordP3uiv)(GLenum, GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoordP4ui)(GLenum, GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoordP4uiv)(GLenum, GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glNormalP3ui)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glNormalP3uiv)(GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glQueryCounter)(GLuint, GLenum) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glSamplerParameterIiv)(GLuint, GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glSamplerParameterIuiv)(GLuint, GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glSamplerParameterf)(GLuint, GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glSamplerParameterfv)(GLuint, GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glSamplerParameteri)(GLuint, GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glSamplerParameteriv)(GLuint, GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColorP3ui)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColorP3uiv)(GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexCoordP1ui)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexCoordP1uiv)(GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexCoordP2ui)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexCoordP2uiv)(GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexCoordP3ui)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexCoordP3uiv)(GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexCoordP4ui)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glTexCoordP4uiv)(GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribDivisor)(GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribP1ui)(GLuint, GLenum, GLboolean, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribP1uiv)(GLuint, GLenum, GLboolean, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribP2ui)(GLuint, GLenum, GLboolean, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribP2uiv)(GLuint, GLenum, GLboolean, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribP3ui)(GLuint, GLenum, GLboolean, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribP3uiv)(GLuint, GLenum, GLboolean, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribP4ui)(GLuint, GLenum, GLboolean, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribP4uiv)(GLuint, GLenum, GLboolean, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexP2ui)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexP2uiv)(GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexP3ui)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexP3uiv)(GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexP4ui)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *_ptrc_glVertexP4uiv)(GLenum, const GLuint *) = NULL;

static int Load_Version_3_3()
{
	int numFailed = 0;
	_ptrc_glBlendFunc = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glBlendFunc");
	if(!_ptrc_glBlendFunc) numFailed++;
	_ptrc_glClear = (void (CODEGEN_FUNCPTR *)(GLbitfield))IntGetProcAddress("glClear");
	if(!_ptrc_glClear) numFailed++;
	_ptrc_glClearColor = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glClearColor");
	if(!_ptrc_glClearColor) numFailed++;
	_ptrc_glClearDepth = (void (CODEGEN_FUNCPTR *)(GLdouble))IntGetProcAddress("glClearDepth");
	if(!_ptrc_glClearDepth) numFailed++;
	_ptrc_glClearStencil = (void (CODEGEN_FUNCPTR *)(GLint))IntGetProcAddress("glClearStencil");
	if(!_ptrc_glClearStencil) numFailed++;
	_ptrc_glColorMask = (void (CODEGEN_FUNCPTR *)(GLboolean, GLboolean, GLboolean, GLboolean))IntGetProcAddress("glColorMask");
	if(!_ptrc_glColorMask) numFailed++;
	_ptrc_glCullFace = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glCullFace");
	if(!_ptrc_glCullFace) numFailed++;
	_ptrc_glDepthFunc = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glDepthFunc");
	if(!_ptrc_glDepthFunc) numFailed++;
	_ptrc_glDepthMask = (void (CODEGEN_FUNCPTR *)(GLboolean))IntGetProcAddress("glDepthMask");
	if(!_ptrc_glDepthMask) numFailed++;
	_ptrc_glDepthRange = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble))IntGetProcAddress("glDepthRange");
	if(!_ptrc_glDepthRange) numFailed++;
	_ptrc_glDisable = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glDisable");
	if(!_ptrc_glDisable) numFailed++;
	_ptrc_glDrawBuffer = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glDrawBuffer");
	if(!_ptrc_glDrawBuffer) numFailed++;
	_ptrc_glEnable = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glEnable");
	if(!_ptrc_glEnable) numFailed++;
	_ptrc_glFinish = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glFinish");
	if(!_ptrc_glFinish) numFailed++;
	_ptrc_glFlush = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glFlush");
	if(!_ptrc_glFlush) numFailed++;
	_ptrc_glFrontFace = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glFrontFace");
	if(!_ptrc_glFrontFace) numFailed++;
	_ptrc_glGetBooleanv = (void (CODEGEN_FUNCPTR *)(GLenum, GLboolean *))IntGetProcAddress("glGetBooleanv");
	if(!_ptrc_glGetBooleanv) numFailed++;
	_ptrc_glGetDoublev = (void (CODEGEN_FUNCPTR *)(GLenum, GLdouble *))IntGetProcAddress("glGetDoublev");
	if(!_ptrc_glGetDoublev) numFailed++;
	_ptrc_glGetError = (GLenum (CODEGEN_FUNCPTR *)())IntGetProcAddress("glGetError");
	if(!_ptrc_glGetError) numFailed++;
	_ptrc_glGetFloatv = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat *))IntGetProcAddress("glGetFloatv");
	if(!_ptrc_glGetFloatv) numFailed++;
	_ptrc_glGetIntegerv = (void (CODEGEN_FUNCPTR *)(GLenum, GLint *))IntGetProcAddress("glGetIntegerv");
	if(!_ptrc_glGetIntegerv) numFailed++;
	_ptrc_glGetString = (const GLubyte * (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glGetString");
	if(!_ptrc_glGetString) numFailed++;
	_ptrc_glGetTexImage = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLenum, GLvoid *))IntGetProcAddress("glGetTexImage");
	if(!_ptrc_glGetTexImage) numFailed++;
	_ptrc_glGetTexLevelParameterfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLfloat *))IntGetProcAddress("glGetTexLevelParameterfv");
	if(!_ptrc_glGetTexLevelParameterfv) numFailed++;
	_ptrc_glGetTexLevelParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLint *))IntGetProcAddress("glGetTexLevelParameteriv");
	if(!_ptrc_glGetTexLevelParameteriv) numFailed++;
	_ptrc_glGetTexParameterfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLfloat *))IntGetProcAddress("glGetTexParameterfv");
	if(!_ptrc_glGetTexParameterfv) numFailed++;
	_ptrc_glGetTexParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetTexParameteriv");
	if(!_ptrc_glGetTexParameteriv) numFailed++;
	_ptrc_glHint = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glHint");
	if(!_ptrc_glHint) numFailed++;
	_ptrc_glIsEnabled = (GLboolean (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glIsEnabled");
	if(!_ptrc_glIsEnabled) numFailed++;
	_ptrc_glLineWidth = (void (CODEGEN_FUNCPTR *)(GLfloat))IntGetProcAddress("glLineWidth");
	if(!_ptrc_glLineWidth) numFailed++;
	_ptrc_glLogicOp = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glLogicOp");
	if(!_ptrc_glLogicOp) numFailed++;
	_ptrc_glPixelStoref = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat))IntGetProcAddress("glPixelStoref");
	if(!_ptrc_glPixelStoref) numFailed++;
	_ptrc_glPixelStorei = (void (CODEGEN_FUNCPTR *)(GLenum, GLint))IntGetProcAddress("glPixelStorei");
	if(!_ptrc_glPixelStorei) numFailed++;
	_ptrc_glPointSize = (void (CODEGEN_FUNCPTR *)(GLfloat))IntGetProcAddress("glPointSize");
	if(!_ptrc_glPointSize) numFailed++;
	_ptrc_glPolygonMode = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glPolygonMode");
	if(!_ptrc_glPolygonMode) numFailed++;
	_ptrc_glReadBuffer = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glReadBuffer");
	if(!_ptrc_glReadBuffer) numFailed++;
	_ptrc_glReadPixels = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid *))IntGetProcAddress("glReadPixels");
	if(!_ptrc_glReadPixels) numFailed++;
	_ptrc_glScissor = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLsizei, GLsizei))IntGetProcAddress("glScissor");
	if(!_ptrc_glScissor) numFailed++;
	_ptrc_glStencilFunc = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLuint))IntGetProcAddress("glStencilFunc");
	if(!_ptrc_glStencilFunc) numFailed++;
	_ptrc_glStencilMask = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glStencilMask");
	if(!_ptrc_glStencilMask) numFailed++;
	_ptrc_glStencilOp = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum))IntGetProcAddress("glStencilOp");
	if(!_ptrc_glStencilOp) numFailed++;
	_ptrc_glTexImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid *))IntGetProcAddress("glTexImage1D");
	if(!_ptrc_glTexImage1D) numFailed++;
	_ptrc_glTexImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *))IntGetProcAddress("glTexImage2D");
	if(!_ptrc_glTexImage2D) numFailed++;
	_ptrc_glTexParameterf = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLfloat))IntGetProcAddress("glTexParameterf");
	if(!_ptrc_glTexParameterf) numFailed++;
	_ptrc_glTexParameterfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLfloat *))IntGetProcAddress("glTexParameterfv");
	if(!_ptrc_glTexParameterfv) numFailed++;
	_ptrc_glTexParameteri = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint))IntGetProcAddress("glTexParameteri");
	if(!_ptrc_glTexParameteri) numFailed++;
	_ptrc_glTexParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLint *))IntGetProcAddress("glTexParameteriv");
	if(!_ptrc_glTexParameteriv) numFailed++;
	_ptrc_glViewport = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLsizei, GLsizei))IntGetProcAddress("glViewport");
	if(!_ptrc_glViewport) numFailed++;
	_ptrc_glBindTexture = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBindTexture");
	if(!_ptrc_glBindTexture) numFailed++;
	_ptrc_glCopyTexImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint))IntGetProcAddress("glCopyTexImage1D");
	if(!_ptrc_glCopyTexImage1D) numFailed++;
	_ptrc_glCopyTexImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint))IntGetProcAddress("glCopyTexImage2D");
	if(!_ptrc_glCopyTexImage2D) numFailed++;
	_ptrc_glCopyTexSubImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLint, GLsizei))IntGetProcAddress("glCopyTexSubImage1D");
	if(!_ptrc_glCopyTexSubImage1D) numFailed++;
	_ptrc_glCopyTexSubImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei))IntGetProcAddress("glCopyTexSubImage2D");
	if(!_ptrc_glCopyTexSubImage2D) numFailed++;
	_ptrc_glDeleteTextures = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteTextures");
	if(!_ptrc_glDeleteTextures) numFailed++;
	_ptrc_glDrawArrays = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLsizei))IntGetProcAddress("glDrawArrays");
	if(!_ptrc_glDrawArrays) numFailed++;
	_ptrc_glDrawElements = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, const GLvoid *))IntGetProcAddress("glDrawElements");
	if(!_ptrc_glDrawElements) numFailed++;
	_ptrc_glGenTextures = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenTextures");
	if(!_ptrc_glGenTextures) numFailed++;
	_ptrc_glIsTexture = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsTexture");
	if(!_ptrc_glIsTexture) numFailed++;
	_ptrc_glPolygonOffset = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat))IntGetProcAddress("glPolygonOffset");
	if(!_ptrc_glPolygonOffset) numFailed++;
	_ptrc_glTexSubImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const GLvoid *))IntGetProcAddress("glTexSubImage1D");
	if(!_ptrc_glTexSubImage1D) numFailed++;
	_ptrc_glTexSubImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *))IntGetProcAddress("glTexSubImage2D");
	if(!_ptrc_glTexSubImage2D) numFailed++;
	_ptrc_glBlendColor = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glBlendColor");
	if(!_ptrc_glBlendColor) numFailed++;
	_ptrc_glBlendEquation = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glBlendEquation");
	if(!_ptrc_glBlendEquation) numFailed++;
	_ptrc_glCopyTexSubImage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei))IntGetProcAddress("glCopyTexSubImage3D");
	if(!_ptrc_glCopyTexSubImage3D) numFailed++;
	_ptrc_glDrawRangeElements = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *))IntGetProcAddress("glDrawRangeElements");
	if(!_ptrc_glDrawRangeElements) numFailed++;
	_ptrc_glTexImage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *))IntGetProcAddress("glTexImage3D");
	if(!_ptrc_glTexImage3D) numFailed++;
	_ptrc_glTexSubImage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *))IntGetProcAddress("glTexSubImage3D");
	if(!_ptrc_glTexSubImage3D) numFailed++;
	_ptrc_glActiveTexture = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glActiveTexture");
	if(!_ptrc_glActiveTexture) numFailed++;
	_ptrc_glCompressedTexImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *))IntGetProcAddress("glCompressedTexImage1D");
	if(!_ptrc_glCompressedTexImage1D) numFailed++;
	_ptrc_glCompressedTexImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *))IntGetProcAddress("glCompressedTexImage2D");
	if(!_ptrc_glCompressedTexImage2D) numFailed++;
	_ptrc_glCompressedTexImage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *))IntGetProcAddress("glCompressedTexImage3D");
	if(!_ptrc_glCompressedTexImage3D) numFailed++;
	_ptrc_glCompressedTexSubImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *))IntGetProcAddress("glCompressedTexSubImage1D");
	if(!_ptrc_glCompressedTexSubImage1D) numFailed++;
	_ptrc_glCompressedTexSubImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *))IntGetProcAddress("glCompressedTexSubImage2D");
	if(!_ptrc_glCompressedTexSubImage2D) numFailed++;
	_ptrc_glCompressedTexSubImage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *))IntGetProcAddress("glCompressedTexSubImage3D");
	if(!_ptrc_glCompressedTexSubImage3D) numFailed++;
	_ptrc_glGetCompressedTexImage = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLvoid *))IntGetProcAddress("glGetCompressedTexImage");
	if(!_ptrc_glGetCompressedTexImage) numFailed++;
	_ptrc_glSampleCoverage = (void (CODEGEN_FUNCPTR *)(GLfloat, GLboolean))IntGetProcAddress("glSampleCoverage");
	if(!_ptrc_glSampleCoverage) numFailed++;
	_ptrc_glBlendFuncSeparate = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLenum))IntGetProcAddress("glBlendFuncSeparate");
	if(!_ptrc_glBlendFuncSeparate) numFailed++;
	_ptrc_glMultiDrawArrays = (void (CODEGEN_FUNCPTR *)(GLenum, const GLint *, const GLsizei *, GLsizei))IntGetProcAddress("glMultiDrawArrays");
	if(!_ptrc_glMultiDrawArrays) numFailed++;
	_ptrc_glMultiDrawElements = (void (CODEGEN_FUNCPTR *)(GLenum, const GLsizei *, GLenum, const GLvoid *const*, GLsizei))IntGetProcAddress("glMultiDrawElements");
	if(!_ptrc_glMultiDrawElements) numFailed++;
	_ptrc_glPointParameterf = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat))IntGetProcAddress("glPointParameterf");
	if(!_ptrc_glPointParameterf) numFailed++;
	_ptrc_glPointParameterfv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLfloat *))IntGetProcAddress("glPointParameterfv");
	if(!_ptrc_glPointParameterfv) numFailed++;
	_ptrc_glPointParameteri = (void (CODEGEN_FUNCPTR *)(GLenum, GLint))IntGetProcAddress("glPointParameteri");
	if(!_ptrc_glPointParameteri) numFailed++;
	_ptrc_glPointParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLint *))IntGetProcAddress("glPointParameteriv");
	if(!_ptrc_glPointParameteriv) numFailed++;
	_ptrc_glBeginQuery = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBeginQuery");
	if(!_ptrc_glBeginQuery) numFailed++;
	_ptrc_glBindBuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBindBuffer");
	if(!_ptrc_glBindBuffer) numFailed++;
	_ptrc_glBufferData = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizeiptr, const GLvoid *, GLenum))IntGetProcAddress("glBufferData");
	if(!_ptrc_glBufferData) numFailed++;
	_ptrc_glBufferSubData = (void (CODEGEN_FUNCPTR *)(GLenum, GLintptr, GLsizeiptr, const GLvoid *))IntGetProcAddress("glBufferSubData");
	if(!_ptrc_glBufferSubData) numFailed++;
	_ptrc_glDeleteBuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteBuffers");
	if(!_ptrc_glDeleteBuffers) numFailed++;
	_ptrc_glDeleteQueries = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteQueries");
	if(!_ptrc_glDeleteQueries) numFailed++;
	_ptrc_glEndQuery = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glEndQuery");
	if(!_ptrc_glEndQuery) numFailed++;
	_ptrc_glGenBuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenBuffers");
	if(!_ptrc_glGenBuffers) numFailed++;
	_ptrc_glGenQueries = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenQueries");
	if(!_ptrc_glGenQueries) numFailed++;
	_ptrc_glGetBufferParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetBufferParameteriv");
	if(!_ptrc_glGetBufferParameteriv) numFailed++;
	_ptrc_glGetBufferPointerv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLvoid **))IntGetProcAddress("glGetBufferPointerv");
	if(!_ptrc_glGetBufferPointerv) numFailed++;
	_ptrc_glGetBufferSubData = (void (CODEGEN_FUNCPTR *)(GLenum, GLintptr, GLsizeiptr, GLvoid *))IntGetProcAddress("glGetBufferSubData");
	if(!_ptrc_glGetBufferSubData) numFailed++;
	_ptrc_glGetQueryObjectiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetQueryObjectiv");
	if(!_ptrc_glGetQueryObjectiv) numFailed++;
	_ptrc_glGetQueryObjectuiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLuint *))IntGetProcAddress("glGetQueryObjectuiv");
	if(!_ptrc_glGetQueryObjectuiv) numFailed++;
	_ptrc_glGetQueryiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetQueryiv");
	if(!_ptrc_glGetQueryiv) numFailed++;
	_ptrc_glIsBuffer = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsBuffer");
	if(!_ptrc_glIsBuffer) numFailed++;
	_ptrc_glIsQuery = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsQuery");
	if(!_ptrc_glIsQuery) numFailed++;
	_ptrc_glMapBuffer = (void * (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glMapBuffer");
	if(!_ptrc_glMapBuffer) numFailed++;
	_ptrc_glUnmapBuffer = (GLboolean (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glUnmapBuffer");
	if(!_ptrc_glUnmapBuffer) numFailed++;
	_ptrc_glAttachShader = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glAttachShader");
	if(!_ptrc_glAttachShader) numFailed++;
	_ptrc_glBindAttribLocation = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, const GLchar *))IntGetProcAddress("glBindAttribLocation");
	if(!_ptrc_glBindAttribLocation) numFailed++;
	_ptrc_glBlendEquationSeparate = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glBlendEquationSeparate");
	if(!_ptrc_glBlendEquationSeparate) numFailed++;
	_ptrc_glCompileShader = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glCompileShader");
	if(!_ptrc_glCompileShader) numFailed++;
	_ptrc_glCreateProgram = (GLuint (CODEGEN_FUNCPTR *)())IntGetProcAddress("glCreateProgram");
	if(!_ptrc_glCreateProgram) numFailed++;
	_ptrc_glCreateShader = (GLuint (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glCreateShader");
	if(!_ptrc_glCreateShader) numFailed++;
	_ptrc_glDeleteProgram = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glDeleteProgram");
	if(!_ptrc_glDeleteProgram) numFailed++;
	_ptrc_glDeleteShader = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glDeleteShader");
	if(!_ptrc_glDeleteShader) numFailed++;
	_ptrc_glDetachShader = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glDetachShader");
	if(!_ptrc_glDetachShader) numFailed++;
	_ptrc_glDisableVertexAttribArray = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glDisableVertexAttribArray");
	if(!_ptrc_glDisableVertexAttribArray) numFailed++;
	_ptrc_glDrawBuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLenum *))IntGetProcAddress("glDrawBuffers");
	if(!_ptrc_glDrawBuffers) numFailed++;
	_ptrc_glEnableVertexAttribArray = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glEnableVertexAttribArray");
	if(!_ptrc_glEnableVertexAttribArray) numFailed++;
	_ptrc_glGetActiveAttrib = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *))IntGetProcAddress("glGetActiveAttrib");
	if(!_ptrc_glGetActiveAttrib) numFailed++;
	_ptrc_glGetActiveUniform = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *))IntGetProcAddress("glGetActiveUniform");
	if(!_ptrc_glGetActiveUniform) numFailed++;
	_ptrc_glGetAttachedShaders = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLsizei *, GLuint *))IntGetProcAddress("glGetAttachedShaders");
	if(!_ptrc_glGetAttachedShaders) numFailed++;
	_ptrc_glGetAttribLocation = (GLint (CODEGEN_FUNCPTR *)(GLuint, const GLchar *))IntGetProcAddress("glGetAttribLocation");
	if(!_ptrc_glGetAttribLocation) numFailed++;
	_ptrc_glGetProgramInfoLog = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetProgramInfoLog");
	if(!_ptrc_glGetProgramInfoLog) numFailed++;
	_ptrc_glGetProgramiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetProgramiv");
	if(!_ptrc_glGetProgramiv) numFailed++;
	_ptrc_glGetShaderInfoLog = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetShaderInfoLog");
	if(!_ptrc_glGetShaderInfoLog) numFailed++;
	_ptrc_glGetShaderSource = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetShaderSource");
	if(!_ptrc_glGetShaderSource) numFailed++;
	_ptrc_glGetShaderiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetShaderiv");
	if(!_ptrc_glGetShaderiv) numFailed++;
	_ptrc_glGetUniformLocation = (GLint (CODEGEN_FUNCPTR *)(GLuint, const GLchar *))IntGetProcAddress("glGetUniformLocation");
	if(!_ptrc_glGetUniformLocation) numFailed++;
	_ptrc_glGetUniformfv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLfloat *))IntGetProcAddress("glGetUniformfv");
	if(!_ptrc_glGetUniformfv) numFailed++;
	_ptrc_glGetUniformiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint *))IntGetProcAddress("glGetUniformiv");
	if(!_ptrc_glGetUniformiv) numFailed++;
	_ptrc_glGetVertexAttribPointerv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLvoid **))IntGetProcAddress("glGetVertexAttribPointerv");
	if(!_ptrc_glGetVertexAttribPointerv) numFailed++;
	_ptrc_glGetVertexAttribdv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLdouble *))IntGetProcAddress("glGetVertexAttribdv");
	if(!_ptrc_glGetVertexAttribdv) numFailed++;
	_ptrc_glGetVertexAttribfv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLfloat *))IntGetProcAddress("glGetVertexAttribfv");
	if(!_ptrc_glGetVertexAttribfv) numFailed++;
	_ptrc_glGetVertexAttribiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetVertexAttribiv");
	if(!_ptrc_glGetVertexAttribiv) numFailed++;
	_ptrc_glIsProgram = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsProgram");
	if(!_ptrc_glIsProgram) numFailed++;
	_ptrc_glIsShader = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsShader");
	if(!_ptrc_glIsShader) numFailed++;
	_ptrc_glLinkProgram = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glLinkProgram");
	if(!_ptrc_glLinkProgram) numFailed++;
	_ptrc_glShaderSource = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, const GLchar *const*, const GLint *))IntGetProcAddress("glShaderSource");
	if(!_ptrc_glShaderSource) numFailed++;
	_ptrc_glStencilFuncSeparate = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint, GLuint))IntGetProcAddress("glStencilFuncSeparate");
	if(!_ptrc_glStencilFuncSeparate) numFailed++;
	_ptrc_glStencilMaskSeparate = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glStencilMaskSeparate");
	if(!_ptrc_glStencilMaskSeparate) numFailed++;
	_ptrc_glStencilOpSeparate = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLenum))IntGetProcAddress("glStencilOpSeparate");
	if(!_ptrc_glStencilOpSeparate) numFailed++;
	_ptrc_glUniform1f = (void (CODEGEN_FUNCPTR *)(GLint, GLfloat))IntGetProcAddress("glUniform1f");
	if(!_ptrc_glUniform1f) numFailed++;
	_ptrc_glUniform1fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLfloat *))IntGetProcAddress("glUniform1fv");
	if(!_ptrc_glUniform1fv) numFailed++;
	_ptrc_glUniform1i = (void (CODEGEN_FUNCPTR *)(GLint, GLint))IntGetProcAddress("glUniform1i");
	if(!_ptrc_glUniform1i) numFailed++;
	_ptrc_glUniform1iv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLint *))IntGetProcAddress("glUniform1iv");
	if(!_ptrc_glUniform1iv) numFailed++;
	_ptrc_glUniform2f = (void (CODEGEN_FUNCPTR *)(GLint, GLfloat, GLfloat))IntGetProcAddress("glUniform2f");
	if(!_ptrc_glUniform2f) numFailed++;
	_ptrc_glUniform2fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLfloat *))IntGetProcAddress("glUniform2fv");
	if(!_ptrc_glUniform2fv) numFailed++;
	_ptrc_glUniform2i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint))IntGetProcAddress("glUniform2i");
	if(!_ptrc_glUniform2i) numFailed++;
	_ptrc_glUniform2iv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLint *))IntGetProcAddress("glUniform2iv");
	if(!_ptrc_glUniform2iv) numFailed++;
	_ptrc_glUniform3f = (void (CODEGEN_FUNCPTR *)(GLint, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glUniform3f");
	if(!_ptrc_glUniform3f) numFailed++;
	_ptrc_glUniform3fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLfloat *))IntGetProcAddress("glUniform3fv");
	if(!_ptrc_glUniform3fv) numFailed++;
	_ptrc_glUniform3i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint, GLint))IntGetProcAddress("glUniform3i");
	if(!_ptrc_glUniform3i) numFailed++;
	_ptrc_glUniform3iv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLint *))IntGetProcAddress("glUniform3iv");
	if(!_ptrc_glUniform3iv) numFailed++;
	_ptrc_glUniform4f = (void (CODEGEN_FUNCPTR *)(GLint, GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glUniform4f");
	if(!_ptrc_glUniform4f) numFailed++;
	_ptrc_glUniform4fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLfloat *))IntGetProcAddress("glUniform4fv");
	if(!_ptrc_glUniform4fv) numFailed++;
	_ptrc_glUniform4i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint, GLint, GLint))IntGetProcAddress("glUniform4i");
	if(!_ptrc_glUniform4i) numFailed++;
	_ptrc_glUniform4iv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLint *))IntGetProcAddress("glUniform4iv");
	if(!_ptrc_glUniform4iv) numFailed++;
	_ptrc_glUniformMatrix2fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix2fv");
	if(!_ptrc_glUniformMatrix2fv) numFailed++;
	_ptrc_glUniformMatrix3fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix3fv");
	if(!_ptrc_glUniformMatrix3fv) numFailed++;
	_ptrc_glUniformMatrix4fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix4fv");
	if(!_ptrc_glUniformMatrix4fv) numFailed++;
	_ptrc_glUseProgram = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glUseProgram");
	if(!_ptrc_glUseProgram) numFailed++;
	_ptrc_glValidateProgram = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glValidateProgram");
	if(!_ptrc_glValidateProgram) numFailed++;
	_ptrc_glVertexAttrib1d = (void (CODEGEN_FUNCPTR *)(GLuint, GLdouble))IntGetProcAddress("glVertexAttrib1d");
	if(!_ptrc_glVertexAttrib1d) numFailed++;
	_ptrc_glVertexAttrib1dv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLdouble *))IntGetProcAddress("glVertexAttrib1dv");
	if(!_ptrc_glVertexAttrib1dv) numFailed++;
	_ptrc_glVertexAttrib1f = (void (CODEGEN_FUNCPTR *)(GLuint, GLfloat))IntGetProcAddress("glVertexAttrib1f");
	if(!_ptrc_glVertexAttrib1f) numFailed++;
	_ptrc_glVertexAttrib1fv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLfloat *))IntGetProcAddress("glVertexAttrib1fv");
	if(!_ptrc_glVertexAttrib1fv) numFailed++;
	_ptrc_glVertexAttrib1s = (void (CODEGEN_FUNCPTR *)(GLuint, GLshort))IntGetProcAddress("glVertexAttrib1s");
	if(!_ptrc_glVertexAttrib1s) numFailed++;
	_ptrc_glVertexAttrib1sv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLshort *))IntGetProcAddress("glVertexAttrib1sv");
	if(!_ptrc_glVertexAttrib1sv) numFailed++;
	_ptrc_glVertexAttrib2d = (void (CODEGEN_FUNCPTR *)(GLuint, GLdouble, GLdouble))IntGetProcAddress("glVertexAttrib2d");
	if(!_ptrc_glVertexAttrib2d) numFailed++;
	_ptrc_glVertexAttrib2dv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLdouble *))IntGetProcAddress("glVertexAttrib2dv");
	if(!_ptrc_glVertexAttrib2dv) numFailed++;
	_ptrc_glVertexAttrib2f = (void (CODEGEN_FUNCPTR *)(GLuint, GLfloat, GLfloat))IntGetProcAddress("glVertexAttrib2f");
	if(!_ptrc_glVertexAttrib2f) numFailed++;
	_ptrc_glVertexAttrib2fv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLfloat *))IntGetProcAddress("glVertexAttrib2fv");
	if(!_ptrc_glVertexAttrib2fv) numFailed++;
	_ptrc_glVertexAttrib2s = (void (CODEGEN_FUNCPTR *)(GLuint, GLshort, GLshort))IntGetProcAddress("glVertexAttrib2s");
	if(!_ptrc_glVertexAttrib2s) numFailed++;
	_ptrc_glVertexAttrib2sv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLshort *))IntGetProcAddress("glVertexAttrib2sv");
	if(!_ptrc_glVertexAttrib2sv) numFailed++;
	_ptrc_glVertexAttrib3d = (void (CODEGEN_FUNCPTR *)(GLuint, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glVertexAttrib3d");
	if(!_ptrc_glVertexAttrib3d) numFailed++;
	_ptrc_glVertexAttrib3dv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLdouble *))IntGetProcAddress("glVertexAttrib3dv");
	if(!_ptrc_glVertexAttrib3dv) numFailed++;
	_ptrc_glVertexAttrib3f = (void (CODEGEN_FUNCPTR *)(GLuint, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glVertexAttrib3f");
	if(!_ptrc_glVertexAttrib3f) numFailed++;
	_ptrc_glVertexAttrib3fv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLfloat *))IntGetProcAddress("glVertexAttrib3fv");
	if(!_ptrc_glVertexAttrib3fv) numFailed++;
	_ptrc_glVertexAttrib3s = (void (CODEGEN_FUNCPTR *)(GLuint, GLshort, GLshort, GLshort))IntGetProcAddress("glVertexAttrib3s");
	if(!_ptrc_glVertexAttrib3s) numFailed++;
	_ptrc_glVertexAttrib3sv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLshort *))IntGetProcAddress("glVertexAttrib3sv");
	if(!_ptrc_glVertexAttrib3sv) numFailed++;
	_ptrc_glVertexAttrib4Nbv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLbyte *))IntGetProcAddress("glVertexAttrib4Nbv");
	if(!_ptrc_glVertexAttrib4Nbv) numFailed++;
	_ptrc_glVertexAttrib4Niv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLint *))IntGetProcAddress("glVertexAttrib4Niv");
	if(!_ptrc_glVertexAttrib4Niv) numFailed++;
	_ptrc_glVertexAttrib4Nsv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLshort *))IntGetProcAddress("glVertexAttrib4Nsv");
	if(!_ptrc_glVertexAttrib4Nsv) numFailed++;
	_ptrc_glVertexAttrib4Nub = (void (CODEGEN_FUNCPTR *)(GLuint, GLubyte, GLubyte, GLubyte, GLubyte))IntGetProcAddress("glVertexAttrib4Nub");
	if(!_ptrc_glVertexAttrib4Nub) numFailed++;
	_ptrc_glVertexAttrib4Nubv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLubyte *))IntGetProcAddress("glVertexAttrib4Nubv");
	if(!_ptrc_glVertexAttrib4Nubv) numFailed++;
	_ptrc_glVertexAttrib4Nuiv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLuint *))IntGetProcAddress("glVertexAttrib4Nuiv");
	if(!_ptrc_glVertexAttrib4Nuiv) numFailed++;
	_ptrc_glVertexAttrib4Nusv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLushort *))IntGetProcAddress("glVertexAttrib4Nusv");
	if(!_ptrc_glVertexAttrib4Nusv) numFailed++;
	_ptrc_glVertexAttrib4bv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLbyte *))IntGetProcAddress("glVertexAttrib4bv");
	if(!_ptrc_glVertexAttrib4bv) numFailed++;
	_ptrc_glVertexAttrib4d = (void (CODEGEN_FUNCPTR *)(GLuint, GLdouble, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glVertexAttrib4d");
	if(!_ptrc_glVertexAttrib4d) numFailed++;
	_ptrc_glVertexAttrib4dv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLdouble *))IntGetProcAddress("glVertexAttrib4dv");
	if(!_ptrc_glVertexAttrib4dv) numFailed++;
	_ptrc_glVertexAttrib4f = (void (CODEGEN_FUNCPTR *)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glVertexAttrib4f");
	if(!_ptrc_glVertexAttrib4f) numFailed++;
	_ptrc_glVertexAttrib4fv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLfloat *))IntGetProcAddress("glVertexAttrib4fv");
	if(!_ptrc_glVertexAttrib4fv) numFailed++;
	_ptrc_glVertexAttrib4iv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLint *))IntGetProcAddress("glVertexAttrib4iv");
	if(!_ptrc_glVertexAttrib4iv) numFailed++;
	_ptrc_glVertexAttrib4s = (void (CODEGEN_FUNCPTR *)(GLuint, GLshort, GLshort, GLshort, GLshort))IntGetProcAddress("glVertexAttrib4s");
	if(!_ptrc_glVertexAttrib4s) numFailed++;
	_ptrc_glVertexAttrib4sv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLshort *))IntGetProcAddress("glVertexAttrib4sv");
	if(!_ptrc_glVertexAttrib4sv) numFailed++;
	_ptrc_glVertexAttrib4ubv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLubyte *))IntGetProcAddress("glVertexAttrib4ubv");
	if(!_ptrc_glVertexAttrib4ubv) numFailed++;
	_ptrc_glVertexAttrib4uiv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLuint *))IntGetProcAddress("glVertexAttrib4uiv");
	if(!_ptrc_glVertexAttrib4uiv) numFailed++;
	_ptrc_glVertexAttrib4usv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLushort *))IntGetProcAddress("glVertexAttrib4usv");
	if(!_ptrc_glVertexAttrib4usv) numFailed++;
	_ptrc_glVertexAttribPointer = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *))IntGetProcAddress("glVertexAttribPointer");
	if(!_ptrc_glVertexAttribPointer) numFailed++;
	_ptrc_glUniformMatrix2x3fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix2x3fv");
	if(!_ptrc_glUniformMatrix2x3fv) numFailed++;
	_ptrc_glUniformMatrix2x4fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix2x4fv");
	if(!_ptrc_glUniformMatrix2x4fv) numFailed++;
	_ptrc_glUniformMatrix3x2fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix3x2fv");
	if(!_ptrc_glUniformMatrix3x2fv) numFailed++;
	_ptrc_glUniformMatrix3x4fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix3x4fv");
	if(!_ptrc_glUniformMatrix3x4fv) numFailed++;
	_ptrc_glUniformMatrix4x2fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix4x2fv");
	if(!_ptrc_glUniformMatrix4x2fv) numFailed++;
	_ptrc_glUniformMatrix4x3fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix4x3fv");
	if(!_ptrc_glUniformMatrix4x3fv) numFailed++;
	_ptrc_glBeginConditionalRender = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum))IntGetProcAddress("glBeginConditionalRender");
	if(!_ptrc_glBeginConditionalRender) numFailed++;
	_ptrc_glBeginTransformFeedback = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glBeginTransformFeedback");
	if(!_ptrc_glBeginTransformFeedback) numFailed++;
	_ptrc_glBindBufferBase = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLuint))IntGetProcAddress("glBindBufferBase");
	if(!_ptrc_glBindBufferBase) numFailed++;
	_ptrc_glBindBufferRange = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLuint, GLintptr, GLsizeiptr))IntGetProcAddress("glBindBufferRange");
	if(!_ptrc_glBindBufferRange) numFailed++;
	_ptrc_glBindFragDataLocation = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, const GLchar *))IntGetProcAddress("glBindFragDataLocation");
	if(!_ptrc_glBindFragDataLocation) numFailed++;
	_ptrc_glBindFramebuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBindFramebuffer");
	if(!_ptrc_glBindFramebuffer) numFailed++;
	_ptrc_glBindRenderbuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBindRenderbuffer");
	if(!_ptrc_glBindRenderbuffer) numFailed++;
	_ptrc_glBindVertexArray = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glBindVertexArray");
	if(!_ptrc_glBindVertexArray) numFailed++;
	_ptrc_glBlitFramebuffer = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum))IntGetProcAddress("glBlitFramebuffer");
	if(!_ptrc_glBlitFramebuffer) numFailed++;
	_ptrc_glCheckFramebufferStatus = (GLenum (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glCheckFramebufferStatus");
	if(!_ptrc_glCheckFramebufferStatus) numFailed++;
	_ptrc_glClampColor = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glClampColor");
	if(!_ptrc_glClampColor) numFailed++;
	_ptrc_glClearBufferfi = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLfloat, GLint))IntGetProcAddress("glClearBufferfi");
	if(!_ptrc_glClearBufferfi) numFailed++;
	_ptrc_glClearBufferfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, const GLfloat *))IntGetProcAddress("glClearBufferfv");
	if(!_ptrc_glClearBufferfv) numFailed++;
	_ptrc_glClearBufferiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, const GLint *))IntGetProcAddress("glClearBufferiv");
	if(!_ptrc_glClearBufferiv) numFailed++;
	_ptrc_glClearBufferuiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, const GLuint *))IntGetProcAddress("glClearBufferuiv");
	if(!_ptrc_glClearBufferuiv) numFailed++;
	_ptrc_glColorMaski = (void (CODEGEN_FUNCPTR *)(GLuint, GLboolean, GLboolean, GLboolean, GLboolean))IntGetProcAddress("glColorMaski");
	if(!_ptrc_glColorMaski) numFailed++;
	_ptrc_glDeleteFramebuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteFramebuffers");
	if(!_ptrc_glDeleteFramebuffers) numFailed++;
	_ptrc_glDeleteRenderbuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteRenderbuffers");
	if(!_ptrc_glDeleteRenderbuffers) numFailed++;
	_ptrc_glDeleteVertexArrays = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteVertexArrays");
	if(!_ptrc_glDeleteVertexArrays) numFailed++;
	_ptrc_glDisablei = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glDisablei");
	if(!_ptrc_glDisablei) numFailed++;
	_ptrc_glEnablei = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glEnablei");
	if(!_ptrc_glEnablei) numFailed++;
	_ptrc_glEndConditionalRender = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glEndConditionalRender");
	if(!_ptrc_glEndConditionalRender) numFailed++;
	_ptrc_glEndTransformFeedback = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glEndTransformFeedback");
	if(!_ptrc_glEndTransformFeedback) numFailed++;
	_ptrc_glFlushMappedBufferRange = (void (CODEGEN_FUNCPTR *)(GLenum, GLintptr, GLsizeiptr))IntGetProcAddress("glFlushMappedBufferRange");
	if(!_ptrc_glFlushMappedBufferRange) numFailed++;
	_ptrc_glFramebufferRenderbuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLuint))IntGetProcAddress("glFramebufferRenderbuffer");
	if(!_ptrc_glFramebufferRenderbuffer) numFailed++;
	_ptrc_glFramebufferTexture1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLuint, GLint))IntGetProcAddress("glFramebufferTexture1D");
	if(!_ptrc_glFramebufferTexture1D) numFailed++;
	_ptrc_glFramebufferTexture2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLuint, GLint))IntGetProcAddress("glFramebufferTexture2D");
	if(!_ptrc_glFramebufferTexture2D) numFailed++;
	_ptrc_glFramebufferTexture3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLuint, GLint, GLint))IntGetProcAddress("glFramebufferTexture3D");
	if(!_ptrc_glFramebufferTexture3D) numFailed++;
	_ptrc_glFramebufferTextureLayer = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint, GLint, GLint))IntGetProcAddress("glFramebufferTextureLayer");
	if(!_ptrc_glFramebufferTextureLayer) numFailed++;
	_ptrc_glGenFramebuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenFramebuffers");
	if(!_ptrc_glGenFramebuffers) numFailed++;
	_ptrc_glGenRenderbuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenRenderbuffers");
	if(!_ptrc_glGenRenderbuffers) numFailed++;
	_ptrc_glGenVertexArrays = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenVertexArrays");
	if(!_ptrc_glGenVertexArrays) numFailed++;
	_ptrc_glGenerateMipmap = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glGenerateMipmap");
	if(!_ptrc_glGenerateMipmap) numFailed++;
	_ptrc_glGetBooleani_v = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLboolean *))IntGetProcAddress("glGetBooleani_v");
	if(!_ptrc_glGetBooleani_v) numFailed++;
	_ptrc_glGetFragDataLocation = (GLint (CODEGEN_FUNCPTR *)(GLuint, const GLchar *))IntGetProcAddress("glGetFragDataLocation");
	if(!_ptrc_glGetFragDataLocation) numFailed++;
	_ptrc_glGetFramebufferAttachmentParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLint *))IntGetProcAddress("glGetFramebufferAttachmentParameteriv");
	if(!_ptrc_glGetFramebufferAttachmentParameteriv) numFailed++;
	_ptrc_glGetIntegeri_v = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLint *))IntGetProcAddress("glGetIntegeri_v");
	if(!_ptrc_glGetIntegeri_v) numFailed++;
	_ptrc_glGetRenderbufferParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetRenderbufferParameteriv");
	if(!_ptrc_glGetRenderbufferParameteriv) numFailed++;
	_ptrc_glGetStringi = (const GLubyte * (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glGetStringi");
	if(!_ptrc_glGetStringi) numFailed++;
	_ptrc_glGetTexParameterIiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetTexParameterIiv");
	if(!_ptrc_glGetTexParameterIiv) numFailed++;
	_ptrc_glGetTexParameterIuiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint *))IntGetProcAddress("glGetTexParameterIuiv");
	if(!_ptrc_glGetTexParameterIuiv) numFailed++;
	_ptrc_glGetTransformFeedbackVarying = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLsizei, GLsizei *, GLsizei *, GLenum *, GLchar *))IntGetProcAddress("glGetTransformFeedbackVarying");
	if(!_ptrc_glGetTransformFeedbackVarying) numFailed++;
	_ptrc_glGetUniformuiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLuint *))IntGetProcAddress("glGetUniformuiv");
	if(!_ptrc_glGetUniformuiv) numFailed++;
	_ptrc_glGetVertexAttribIiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetVertexAttribIiv");
	if(!_ptrc_glGetVertexAttribIiv) numFailed++;
	_ptrc_glGetVertexAttribIuiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLuint *))IntGetProcAddress("glGetVertexAttribIuiv");
	if(!_ptrc_glGetVertexAttribIuiv) numFailed++;
	_ptrc_glIsEnabledi = (GLboolean (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glIsEnabledi");
	if(!_ptrc_glIsEnabledi) numFailed++;
	_ptrc_glIsFramebuffer = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsFramebuffer");
	if(!_ptrc_glIsFramebuffer) numFailed++;
	_ptrc_glIsRenderbuffer = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsRenderbuffer");
	if(!_ptrc_glIsRenderbuffer) numFailed++;
	_ptrc_glIsVertexArray = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsVertexArray");
	if(!_ptrc_glIsVertexArray) numFailed++;
	_ptrc_glMapBufferRange = (void * (CODEGEN_FUNCPTR *)(GLenum, GLintptr, GLsizeiptr, GLbitfield))IntGetProcAddress("glMapBufferRange");
	if(!_ptrc_glMapBufferRange) numFailed++;
	_ptrc_glRenderbufferStorage = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLsizei, GLsizei))IntGetProcAddress("glRenderbufferStorage");
	if(!_ptrc_glRenderbufferStorage) numFailed++;
	_ptrc_glRenderbufferStorageMultisample = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei))IntGetProcAddress("glRenderbufferStorageMultisample");
	if(!_ptrc_glRenderbufferStorageMultisample) numFailed++;
	_ptrc_glTexParameterIiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLint *))IntGetProcAddress("glTexParameterIiv");
	if(!_ptrc_glTexParameterIiv) numFailed++;
	_ptrc_glTexParameterIuiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLuint *))IntGetProcAddress("glTexParameterIuiv");
	if(!_ptrc_glTexParameterIuiv) numFailed++;
	_ptrc_glTransformFeedbackVaryings = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, const GLchar *const*, GLenum))IntGetProcAddress("glTransformFeedbackVaryings");
	if(!_ptrc_glTransformFeedbackVaryings) numFailed++;
	_ptrc_glUniform1ui = (void (CODEGEN_FUNCPTR *)(GLint, GLuint))IntGetProcAddress("glUniform1ui");
	if(!_ptrc_glUniform1ui) numFailed++;
	_ptrc_glUniform1uiv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLuint *))IntGetProcAddress("glUniform1uiv");
	if(!_ptrc_glUniform1uiv) numFailed++;
	_ptrc_glUniform2ui = (void (CODEGEN_FUNCPTR *)(GLint, GLuint, GLuint))IntGetProcAddress("glUniform2ui");
	if(!_ptrc_glUniform2ui) numFailed++;
	_ptrc_glUniform2uiv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLuint *))IntGetProcAddress("glUniform2uiv");
	if(!_ptrc_glUniform2uiv) numFailed++;
	_ptrc_glUniform3ui = (void (CODEGEN_FUNCPTR *)(GLint, GLuint, GLuint, GLuint))IntGetProcAddress("glUniform3ui");
	if(!_ptrc_glUniform3ui) numFailed++;
	_ptrc_glUniform3uiv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLuint *))IntGetProcAddress("glUniform3uiv");
	if(!_ptrc_glUniform3uiv) numFailed++;
	_ptrc_glUniform4ui = (void (CODEGEN_FUNCPTR *)(GLint, GLuint, GLuint, GLuint, GLuint))IntGetProcAddress("glUniform4ui");
	if(!_ptrc_glUniform4ui) numFailed++;
	_ptrc_glUniform4uiv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLuint *))IntGetProcAddress("glUniform4uiv");
	if(!_ptrc_glUniform4uiv) numFailed++;
	_ptrc_glVertexAttribI1i = (void (CODEGEN_FUNCPTR *)(GLuint, GLint))IntGetProcAddress("glVertexAttribI1i");
	if(!_ptrc_glVertexAttribI1i) numFailed++;
	_ptrc_glVertexAttribI1iv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLint *))IntGetProcAddress("glVertexAttribI1iv");
	if(!_ptrc_glVertexAttribI1iv) numFailed++;
	_ptrc_glVertexAttribI1ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glVertexAttribI1ui");
	if(!_ptrc_glVertexAttribI1ui) numFailed++;
	_ptrc_glVertexAttribI1uiv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLuint *))IntGetProcAddress("glVertexAttribI1uiv");
	if(!_ptrc_glVertexAttribI1uiv) numFailed++;
	_ptrc_glVertexAttribI2i = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint))IntGetProcAddress("glVertexAttribI2i");
	if(!_ptrc_glVertexAttribI2i) numFailed++;
	_ptrc_glVertexAttribI2iv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLint *))IntGetProcAddress("glVertexAttribI2iv");
	if(!_ptrc_glVertexAttribI2iv) numFailed++;
	_ptrc_glVertexAttribI2ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLuint))IntGetProcAddress("glVertexAttribI2ui");
	if(!_ptrc_glVertexAttribI2ui) numFailed++;
	_ptrc_glVertexAttribI2uiv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLuint *))IntGetProcAddress("glVertexAttribI2uiv");
	if(!_ptrc_glVertexAttribI2uiv) numFailed++;
	_ptrc_glVertexAttribI3i = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint, GLint))IntGetProcAddress("glVertexAttribI3i");
	if(!_ptrc_glVertexAttribI3i) numFailed++;
	_ptrc_glVertexAttribI3iv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLint *))IntGetProcAddress("glVertexAttribI3iv");
	if(!_ptrc_glVertexAttribI3iv) numFailed++;
	_ptrc_glVertexAttribI3ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLuint, GLuint))IntGetProcAddress("glVertexAttribI3ui");
	if(!_ptrc_glVertexAttribI3ui) numFailed++;
	_ptrc_glVertexAttribI3uiv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLuint *))IntGetProcAddress("glVertexAttribI3uiv");
	if(!_ptrc_glVertexAttribI3uiv) numFailed++;
	_ptrc_glVertexAttribI4bv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLbyte *))IntGetProcAddress("glVertexAttribI4bv");
	if(!_ptrc_glVertexAttribI4bv) numFailed++;
	_ptrc_glVertexAttribI4i = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint, GLint, GLint))IntGetProcAddress("glVertexAttribI4i");
	if(!_ptrc_glVertexAttribI4i) numFailed++;
	_ptrc_glVertexAttribI4iv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLint *))IntGetProcAddress("glVertexAttribI4iv");
	if(!_ptrc_glVertexAttribI4iv) numFailed++;
	_ptrc_glVertexAttribI4sv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLshort *))IntGetProcAddress("glVertexAttribI4sv");
	if(!_ptrc_glVertexAttribI4sv) numFailed++;
	_ptrc_glVertexAttribI4ubv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLubyte *))IntGetProcAddress("glVertexAttribI4ubv");
	if(!_ptrc_glVertexAttribI4ubv) numFailed++;
	_ptrc_glVertexAttribI4ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLuint, GLuint, GLuint))IntGetProcAddress("glVertexAttribI4ui");
	if(!_ptrc_glVertexAttribI4ui) numFailed++;
	_ptrc_glVertexAttribI4uiv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLuint *))IntGetProcAddress("glVertexAttribI4uiv");
	if(!_ptrc_glVertexAttribI4uiv) numFailed++;
	_ptrc_glVertexAttribI4usv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLushort *))IntGetProcAddress("glVertexAttribI4usv");
	if(!_ptrc_glVertexAttribI4usv) numFailed++;
	_ptrc_glVertexAttribIPointer = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLenum, GLsizei, const GLvoid *))IntGetProcAddress("glVertexAttribIPointer");
	if(!_ptrc_glVertexAttribIPointer) numFailed++;
	_ptrc_glCopyBufferSubData = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr))IntGetProcAddress("glCopyBufferSubData");
	if(!_ptrc_glCopyBufferSubData) numFailed++;
	_ptrc_glDrawArraysInstanced = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLsizei, GLsizei))IntGetProcAddress("glDrawArraysInstanced");
	if(!_ptrc_glDrawArraysInstanced) numFailed++;
	_ptrc_glDrawElementsInstanced = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, const GLvoid *, GLsizei))IntGetProcAddress("glDrawElementsInstanced");
	if(!_ptrc_glDrawElementsInstanced) numFailed++;
	_ptrc_glGetActiveUniformBlockName = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetActiveUniformBlockName");
	if(!_ptrc_glGetActiveUniformBlockName) numFailed++;
	_ptrc_glGetActiveUniformBlockiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLenum, GLint *))IntGetProcAddress("glGetActiveUniformBlockiv");
	if(!_ptrc_glGetActiveUniformBlockiv) numFailed++;
	_ptrc_glGetActiveUniformName = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetActiveUniformName");
	if(!_ptrc_glGetActiveUniformName) numFailed++;
	_ptrc_glGetActiveUniformsiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, const GLuint *, GLenum, GLint *))IntGetProcAddress("glGetActiveUniformsiv");
	if(!_ptrc_glGetActiveUniformsiv) numFailed++;
	_ptrc_glGetUniformBlockIndex = (GLuint (CODEGEN_FUNCPTR *)(GLuint, const GLchar *))IntGetProcAddress("glGetUniformBlockIndex");
	if(!_ptrc_glGetUniformBlockIndex) numFailed++;
	_ptrc_glGetUniformIndices = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, const GLchar *const*, GLuint *))IntGetProcAddress("glGetUniformIndices");
	if(!_ptrc_glGetUniformIndices) numFailed++;
	_ptrc_glPrimitiveRestartIndex = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glPrimitiveRestartIndex");
	if(!_ptrc_glPrimitiveRestartIndex) numFailed++;
	_ptrc_glTexBuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint))IntGetProcAddress("glTexBuffer");
	if(!_ptrc_glTexBuffer) numFailed++;
	_ptrc_glUniformBlockBinding = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLuint))IntGetProcAddress("glUniformBlockBinding");
	if(!_ptrc_glUniformBlockBinding) numFailed++;
	_ptrc_glClientWaitSync = (GLenum (CODEGEN_FUNCPTR *)(GLsync, GLbitfield, GLuint64))IntGetProcAddress("glClientWaitSync");
	if(!_ptrc_glClientWaitSync) numFailed++;
	_ptrc_glDeleteSync = (void (CODEGEN_FUNCPTR *)(GLsync))IntGetProcAddress("glDeleteSync");
	if(!_ptrc_glDeleteSync) numFailed++;
	_ptrc_glDrawElementsBaseVertex = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, const GLvoid *, GLint))IntGetProcAddress("glDrawElementsBaseVertex");
	if(!_ptrc_glDrawElementsBaseVertex) numFailed++;
	_ptrc_glDrawElementsInstancedBaseVertex = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, const GLvoid *, GLsizei, GLint))IntGetProcAddress("glDrawElementsInstancedBaseVertex");
	if(!_ptrc_glDrawElementsInstancedBaseVertex) numFailed++;
	_ptrc_glDrawRangeElementsBaseVertex = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *, GLint))IntGetProcAddress("glDrawRangeElementsBaseVertex");
	if(!_ptrc_glDrawRangeElementsBaseVertex) numFailed++;
	_ptrc_glFenceSync = (GLsync (CODEGEN_FUNCPTR *)(GLenum, GLbitfield))IntGetProcAddress("glFenceSync");
	if(!_ptrc_glFenceSync) numFailed++;
	_ptrc_glFramebufferTexture = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint, GLint))IntGetProcAddress("glFramebufferTexture");
	if(!_ptrc_glFramebufferTexture) numFailed++;
	_ptrc_glGetBufferParameteri64v = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint64 *))IntGetProcAddress("glGetBufferParameteri64v");
	if(!_ptrc_glGetBufferParameteri64v) numFailed++;
	_ptrc_glGetInteger64i_v = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLint64 *))IntGetProcAddress("glGetInteger64i_v");
	if(!_ptrc_glGetInteger64i_v) numFailed++;
	_ptrc_glGetInteger64v = (void (CODEGEN_FUNCPTR *)(GLenum, GLint64 *))IntGetProcAddress("glGetInteger64v");
	if(!_ptrc_glGetInteger64v) numFailed++;
	_ptrc_glGetMultisamplefv = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLfloat *))IntGetProcAddress("glGetMultisamplefv");
	if(!_ptrc_glGetMultisamplefv) numFailed++;
	_ptrc_glGetSynciv = (void (CODEGEN_FUNCPTR *)(GLsync, GLenum, GLsizei, GLsizei *, GLint *))IntGetProcAddress("glGetSynciv");
	if(!_ptrc_glGetSynciv) numFailed++;
	_ptrc_glIsSync = (GLboolean (CODEGEN_FUNCPTR *)(GLsync))IntGetProcAddress("glIsSync");
	if(!_ptrc_glIsSync) numFailed++;
	_ptrc_glMultiDrawElementsBaseVertex = (void (CODEGEN_FUNCPTR *)(GLenum, const GLsizei *, GLenum, const GLvoid *const*, GLsizei, const GLint *))IntGetProcAddress("glMultiDrawElementsBaseVertex");
	if(!_ptrc_glMultiDrawElementsBaseVertex) numFailed++;
	_ptrc_glProvokingVertex = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glProvokingVertex");
	if(!_ptrc_glProvokingVertex) numFailed++;
	_ptrc_glSampleMaski = (void (CODEGEN_FUNCPTR *)(GLuint, GLbitfield))IntGetProcAddress("glSampleMaski");
	if(!_ptrc_glSampleMaski) numFailed++;
	_ptrc_glTexImage2DMultisample = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLint, GLsizei, GLsizei, GLboolean))IntGetProcAddress("glTexImage2DMultisample");
	if(!_ptrc_glTexImage2DMultisample) numFailed++;
	_ptrc_glTexImage3DMultisample = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLint, GLsizei, GLsizei, GLsizei, GLboolean))IntGetProcAddress("glTexImage3DMultisample");
	if(!_ptrc_glTexImage3DMultisample) numFailed++;
	_ptrc_glWaitSync = (void (CODEGEN_FUNCPTR *)(GLsync, GLbitfield, GLuint64))IntGetProcAddress("glWaitSync");
	if(!_ptrc_glWaitSync) numFailed++;
	_ptrc_glBindFragDataLocationIndexed = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLuint, const GLchar *))IntGetProcAddress("glBindFragDataLocationIndexed");
	if(!_ptrc_glBindFragDataLocationIndexed) numFailed++;
	_ptrc_glBindSampler = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glBindSampler");
	if(!_ptrc_glBindSampler) numFailed++;
	_ptrc_glColorP3ui = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glColorP3ui");
	if(!_ptrc_glColorP3ui) numFailed++;
	_ptrc_glColorP3uiv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLuint *))IntGetProcAddress("glColorP3uiv");
	if(!_ptrc_glColorP3uiv) numFailed++;
	_ptrc_glColorP4ui = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glColorP4ui");
	if(!_ptrc_glColorP4ui) numFailed++;
	_ptrc_glColorP4uiv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLuint *))IntGetProcAddress("glColorP4uiv");
	if(!_ptrc_glColorP4uiv) numFailed++;
	_ptrc_glDeleteSamplers = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteSamplers");
	if(!_ptrc_glDeleteSamplers) numFailed++;
	_ptrc_glGenSamplers = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenSamplers");
	if(!_ptrc_glGenSamplers) numFailed++;
	_ptrc_glGetFragDataIndex = (GLint (CODEGEN_FUNCPTR *)(GLuint, const GLchar *))IntGetProcAddress("glGetFragDataIndex");
	if(!_ptrc_glGetFragDataIndex) numFailed++;
	_ptrc_glGetQueryObjecti64v = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint64 *))IntGetProcAddress("glGetQueryObjecti64v");
	if(!_ptrc_glGetQueryObjecti64v) numFailed++;
	_ptrc_glGetQueryObjectui64v = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLuint64 *))IntGetProcAddress("glGetQueryObjectui64v");
	if(!_ptrc_glGetQueryObjectui64v) numFailed++;
	_ptrc_glGetSamplerParameterIiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetSamplerParameterIiv");
	if(!_ptrc_glGetSamplerParameterIiv) numFailed++;
	_ptrc_glGetSamplerParameterIuiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLuint *))IntGetProcAddress("glGetSamplerParameterIuiv");
	if(!_ptrc_glGetSamplerParameterIuiv) numFailed++;
	_ptrc_glGetSamplerParameterfv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLfloat *))IntGetProcAddress("glGetSamplerParameterfv");
	if(!_ptrc_glGetSamplerParameterfv) numFailed++;
	_ptrc_glGetSamplerParameteriv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetSamplerParameteriv");
	if(!_ptrc_glGetSamplerParameteriv) numFailed++;
	_ptrc_glIsSampler = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsSampler");
	if(!_ptrc_glIsSampler) numFailed++;
	_ptrc_glMultiTexCoordP1ui = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint))IntGetProcAddress("glMultiTexCoordP1ui");
	if(!_ptrc_glMultiTexCoordP1ui) numFailed++;
	_ptrc_glMultiTexCoordP1uiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLuint *))IntGetProcAddress("glMultiTexCoordP1uiv");
	if(!_ptrc_glMultiTexCoordP1uiv) numFailed++;
	_ptrc_glMultiTexCoordP2ui = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint))IntGetProcAddress("glMultiTexCoordP2ui");
	if(!_ptrc_glMultiTexCoordP2ui) numFailed++;
	_ptrc_glMultiTexCoordP2uiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLuint *))IntGetProcAddress("glMultiTexCoordP2uiv");
	if(!_ptrc_glMultiTexCoordP2uiv) numFailed++;
	_ptrc_glMultiTexCoordP3ui = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint))IntGetProcAddress("glMultiTexCoordP3ui");
	if(!_ptrc_glMultiTexCoordP3ui) numFailed++;
	_ptrc_glMultiTexCoordP3uiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLuint *))IntGetProcAddress("glMultiTexCoordP3uiv");
	if(!_ptrc_glMultiTexCoordP3uiv) numFailed++;
	_ptrc_glMultiTexCoordP4ui = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint))IntGetProcAddress("glMultiTexCoordP4ui");
	if(!_ptrc_glMultiTexCoordP4ui) numFailed++;
	_ptrc_glMultiTexCoordP4uiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLuint *))IntGetProcAddress("glMultiTexCoordP4uiv");
	if(!_ptrc_glMultiTexCoordP4uiv) numFailed++;
	_ptrc_glNormalP3ui = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glNormalP3ui");
	if(!_ptrc_glNormalP3ui) numFailed++;
	_ptrc_glNormalP3uiv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLuint *))IntGetProcAddress("glNormalP3uiv");
	if(!_ptrc_glNormalP3uiv) numFailed++;
	_ptrc_glQueryCounter = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum))IntGetProcAddress("glQueryCounter");
	if(!_ptrc_glQueryCounter) numFailed++;
	_ptrc_glSamplerParameterIiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLint *))IntGetProcAddress("glSamplerParameterIiv");
	if(!_ptrc_glSamplerParameterIiv) numFailed++;
	_ptrc_glSamplerParameterIuiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLuint *))IntGetProcAddress("glSamplerParameterIuiv");
	if(!_ptrc_glSamplerParameterIuiv) numFailed++;
	_ptrc_glSamplerParameterf = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLfloat))IntGetProcAddress("glSamplerParameterf");
	if(!_ptrc_glSamplerParameterf) numFailed++;
	_ptrc_glSamplerParameterfv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLfloat *))IntGetProcAddress("glSamplerParameterfv");
	if(!_ptrc_glSamplerParameterfv) numFailed++;
	_ptrc_glSamplerParameteri = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint))IntGetProcAddress("glSamplerParameteri");
	if(!_ptrc_glSamplerParameteri) numFailed++;
	_ptrc_glSamplerParameteriv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLint *))IntGetProcAddress("glSamplerParameteriv");
	if(!_ptrc_glSamplerParameteriv) numFailed++;
	_ptrc_glSecondaryColorP3ui = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glSecondaryColorP3ui");
	if(!_ptrc_glSecondaryColorP3ui) numFailed++;
	_ptrc_glSecondaryColorP3uiv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLuint *))IntGetProcAddress("glSecondaryColorP3uiv");
	if(!_ptrc_glSecondaryColorP3uiv) numFailed++;
	_ptrc_glTexCoordP1ui = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glTexCoordP1ui");
	if(!_ptrc_glTexCoordP1ui) numFailed++;
	_ptrc_glTexCoordP1uiv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLuint *))IntGetProcAddress("glTexCoordP1uiv");
	if(!_ptrc_glTexCoordP1uiv) numFailed++;
	_ptrc_glTexCoordP2ui = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glTexCoordP2ui");
	if(!_ptrc_glTexCoordP2ui) numFailed++;
	_ptrc_glTexCoordP2uiv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLuint *))IntGetProcAddress("glTexCoordP2uiv");
	if(!_ptrc_glTexCoordP2uiv) numFailed++;
	_ptrc_glTexCoordP3ui = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glTexCoordP3ui");
	if(!_ptrc_glTexCoordP3ui) numFailed++;
	_ptrc_glTexCoordP3uiv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLuint *))IntGetProcAddress("glTexCoordP3uiv");
	if(!_ptrc_glTexCoordP3uiv) numFailed++;
	_ptrc_glTexCoordP4ui = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glTexCoordP4ui");
	if(!_ptrc_glTexCoordP4ui) numFailed++;
	_ptrc_glTexCoordP4uiv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLuint *))IntGetProcAddress("glTexCoordP4uiv");
	if(!_ptrc_glTexCoordP4uiv) numFailed++;
	_ptrc_glVertexAttribDivisor = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glVertexAttribDivisor");
	if(!_ptrc_glVertexAttribDivisor) numFailed++;
	_ptrc_glVertexAttribP1ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLboolean, GLuint))IntGetProcAddress("glVertexAttribP1ui");
	if(!_ptrc_glVertexAttribP1ui) numFailed++;
	_ptrc_glVertexAttribP1uiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLboolean, const GLuint *))IntGetProcAddress("glVertexAttribP1uiv");
	if(!_ptrc_glVertexAttribP1uiv) numFailed++;
	_ptrc_glVertexAttribP2ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLboolean, GLuint))IntGetProcAddress("glVertexAttribP2ui");
	if(!_ptrc_glVertexAttribP2ui) numFailed++;
	_ptrc_glVertexAttribP2uiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLboolean, const GLuint *))IntGetProcAddress("glVertexAttribP2uiv");
	if(!_ptrc_glVertexAttribP2uiv) numFailed++;
	_ptrc_glVertexAttribP3ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLboolean, GLuint))IntGetProcAddress("glVertexAttribP3ui");
	if(!_ptrc_glVertexAttribP3ui) numFailed++;
	_ptrc_glVertexAttribP3uiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLboolean, const GLuint *))IntGetProcAddress("glVertexAttribP3uiv");
	if(!_ptrc_glVertexAttribP3uiv) numFailed++;
	_ptrc_glVertexAttribP4ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLboolean, GLuint))IntGetProcAddress("glVertexAttribP4ui");
	if(!_ptrc_glVertexAttribP4ui) numFailed++;
	_ptrc_glVertexAttribP4uiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLboolean, const GLuint *))IntGetProcAddress("glVertexAttribP4uiv");
	if(!_ptrc_glVertexAttribP4uiv) numFailed++;
	_ptrc_glVertexP2ui = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glVertexP2ui");
	if(!_ptrc_glVertexP2ui) numFailed++;
	_ptrc_glVertexP2uiv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLuint *))IntGetProcAddress("glVertexP2uiv");
	if(!_ptrc_glVertexP2uiv) numFailed++;
	_ptrc_glVertexP3ui = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glVertexP3ui");
	if(!_ptrc_glVertexP3ui) numFailed++;
	_ptrc_glVertexP3uiv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLuint *))IntGetProcAddress("glVertexP3uiv");
	if(!_ptrc_glVertexP3uiv) numFailed++;
	_ptrc_glVertexP4ui = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glVertexP4ui");
	if(!_ptrc_glVertexP4ui) numFailed++;
	_ptrc_glVertexP4uiv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLuint *))IntGetProcAddress("glVertexP4uiv");
	if(!_ptrc_glVertexP4uiv) numFailed++;
	return numFailed;
}

typedef int (*PFN_LOADFUNCPOINTERS)();
typedef struct ogl_StrToExtMap_s
{
	char *extensionName;
	int *extensionVariable;
	PFN_LOADFUNCPOINTERS LoadExtension;
} ogl_StrToExtMap;

static ogl_StrToExtMap ExtensionMap[33] = {
	{"GL_ARB_seamless_cube_map", &ogl_ext_ARB_seamless_cube_map, NULL},
	{"GL_EXT_texture_compression_s3tc", &ogl_ext_EXT_texture_compression_s3tc, NULL},
	{"GL_EXT_texture_sRGB", &ogl_ext_EXT_texture_sRGB, NULL},
	{"GL_EXT_texture_filter_anisotropic", &ogl_ext_EXT_texture_filter_anisotropic, NULL},
	{"GL_ARB_compressed_texture_pixel_storage", &ogl_ext_ARB_compressed_texture_pixel_storage, NULL},
	{"GL_ARB_conservative_depth", &ogl_ext_ARB_conservative_depth, NULL},
	{"GL_ARB_ES2_compatibility", &ogl_ext_ARB_ES2_compatibility, Load_ARB_ES2_compatibility},
	{"GL_ARB_get_program_binary", &ogl_ext_ARB_get_program_binary, Load_ARB_get_program_binary},
	{"GL_ARB_explicit_uniform_location", &ogl_ext_ARB_explicit_uniform_location, NULL},
	{"GL_ARB_internalformat_query", &ogl_ext_ARB_internalformat_query, Load_ARB_internalformat_query},
	{"GL_ARB_internalformat_query2", &ogl_ext_ARB_internalformat_query2, Load_ARB_internalformat_query2},
	{"GL_ARB_map_buffer_alignment", &ogl_ext_ARB_map_buffer_alignment, NULL},
	{"GL_ARB_program_interface_query", &ogl_ext_ARB_program_interface_query, Load_ARB_program_interface_query},
	{"GL_ARB_separate_shader_objects", &ogl_ext_ARB_separate_shader_objects, Load_ARB_separate_shader_objects},
	{"GL_ARB_shading_language_420pack", &ogl_ext_ARB_shading_language_420pack, NULL},
	{"GL_ARB_shading_language_packing", &ogl_ext_ARB_shading_language_packing, NULL},
	{"GL_ARB_texture_buffer_range", &ogl_ext_ARB_texture_buffer_range, Load_ARB_texture_buffer_range},
	{"GL_ARB_texture_storage", &ogl_ext_ARB_texture_storage, Load_ARB_texture_storage},
	{"GL_ARB_texture_view", &ogl_ext_ARB_texture_view, Load_ARB_texture_view},
	{"GL_ARB_vertex_attrib_binding", &ogl_ext_ARB_vertex_attrib_binding, Load_ARB_vertex_attrib_binding},
	{"GL_ARB_viewport_array", &ogl_ext_ARB_viewport_array, Load_ARB_viewport_array},
	{"GL_ARB_arrays_of_arrays", &ogl_ext_ARB_arrays_of_arrays, NULL},
	{"GL_ARB_clear_buffer_object", &ogl_ext_ARB_clear_buffer_object, Load_ARB_clear_buffer_object},
	{"GL_ARB_copy_image", &ogl_ext_ARB_copy_image, Load_ARB_copy_image},
	{"GL_ARB_ES3_compatibility", &ogl_ext_ARB_ES3_compatibility, NULL},
	{"GL_ARB_fragment_layer_viewport", &ogl_ext_ARB_fragment_layer_viewport, NULL},
	{"GL_ARB_framebuffer_no_attachments", &ogl_ext_ARB_framebuffer_no_attachments, Load_ARB_framebuffer_no_attachments},
	{"GL_ARB_invalidate_subdata", &ogl_ext_ARB_invalidate_subdata, Load_ARB_invalidate_subdata},
	{"GL_ARB_robust_buffer_access_behavior", &ogl_ext_ARB_robust_buffer_access_behavior, NULL},
	{"GL_ARB_stencil_texturing", &ogl_ext_ARB_stencil_texturing, NULL},
	{"GL_ARB_texture_query_levels", &ogl_ext_ARB_texture_query_levels, NULL},
	{"GL_ARB_texture_storage_multisample", &ogl_ext_ARB_texture_storage_multisample, Load_ARB_texture_storage_multisample},
	{"GL_KHR_debug", &ogl_ext_KHR_debug, Load_KHR_debug},
};

static int g_extensionMapSize = 33;

static ogl_StrToExtMap *FindExtEntry(const char *extensionName)
{
	int loop;
	ogl_StrToExtMap *currLoc = ExtensionMap;
	for(loop = 0; loop < g_extensionMapSize; ++loop, ++currLoc)
	{
		if(strcmp(extensionName, currLoc->extensionName) == 0)
			return currLoc;
	}
	
	return NULL;
}

static void ClearExtensionVars()
{
	ogl_ext_ARB_seamless_cube_map = ogl_LOAD_FAILED;
	ogl_ext_EXT_texture_compression_s3tc = ogl_LOAD_FAILED;
	ogl_ext_EXT_texture_sRGB = ogl_LOAD_FAILED;
	ogl_ext_EXT_texture_filter_anisotropic = ogl_LOAD_FAILED;
	ogl_ext_ARB_compressed_texture_pixel_storage = ogl_LOAD_FAILED;
	ogl_ext_ARB_conservative_depth = ogl_LOAD_FAILED;
	ogl_ext_ARB_ES2_compatibility = ogl_LOAD_FAILED;
	ogl_ext_ARB_get_program_binary = ogl_LOAD_FAILED;
	ogl_ext_ARB_explicit_uniform_location = ogl_LOAD_FAILED;
	ogl_ext_ARB_internalformat_query = ogl_LOAD_FAILED;
	ogl_ext_ARB_internalformat_query2 = ogl_LOAD_FAILED;
	ogl_ext_ARB_map_buffer_alignment = ogl_LOAD_FAILED;
	ogl_ext_ARB_program_interface_query = ogl_LOAD_FAILED;
	ogl_ext_ARB_separate_shader_objects = ogl_LOAD_FAILED;
	ogl_ext_ARB_shading_language_420pack = ogl_LOAD_FAILED;
	ogl_ext_ARB_shading_language_packing = ogl_LOAD_FAILED;
	ogl_ext_ARB_texture_buffer_range = ogl_LOAD_FAILED;
	ogl_ext_ARB_texture_storage = ogl_LOAD_FAILED;
	ogl_ext_ARB_texture_view = ogl_LOAD_FAILED;
	ogl_ext_ARB_vertex_attrib_binding = ogl_LOAD_FAILED;
	ogl_ext_ARB_viewport_array = ogl_LOAD_FAILED;
	ogl_ext_ARB_arrays_of_arrays = ogl_LOAD_FAILED;
	ogl_ext_ARB_clear_buffer_object = ogl_LOAD_FAILED;
	ogl_ext_ARB_copy_image = ogl_LOAD_FAILED;
	ogl_ext_ARB_ES3_compatibility = ogl_LOAD_FAILED;
	ogl_ext_ARB_fragment_layer_viewport = ogl_LOAD_FAILED;
	ogl_ext_ARB_framebuffer_no_attachments = ogl_LOAD_FAILED;
	ogl_ext_ARB_invalidate_subdata = ogl_LOAD_FAILED;
	ogl_ext_ARB_robust_buffer_access_behavior = ogl_LOAD_FAILED;
	ogl_ext_ARB_stencil_texturing = ogl_LOAD_FAILED;
	ogl_ext_ARB_texture_query_levels = ogl_LOAD_FAILED;
	ogl_ext_ARB_texture_storage_multisample = ogl_LOAD_FAILED;
	ogl_ext_KHR_debug = ogl_LOAD_FAILED;
}


static void LoadExtByName(const char *extensionName)
{
	ogl_StrToExtMap *entry = NULL;
	entry = FindExtEntry(extensionName);
	if(entry)
	{
		if(entry->LoadExtension)
		{
			int numFailed = entry->LoadExtension();
			if(numFailed == 0)
			{
				*(entry->extensionVariable) = ogl_LOAD_SUCCEEDED;
			}
			else
			{
				*(entry->extensionVariable) = ogl_LOAD_SUCCEEDED + numFailed;
			}
		}
		else
		{
			*(entry->extensionVariable) = ogl_LOAD_SUCCEEDED;
		}
	}
}


static void ProcExtsFromExtList()
{
	GLint iLoop;
	GLint iNumExtensions = 0;
	_ptrc_glGetIntegerv(GL_NUM_EXTENSIONS, &iNumExtensions);

	for(iLoop = 0; iLoop < iNumExtensions; iLoop++)
	{
		const char *strExtensionName = (const char *)_ptrc_glGetStringi(GL_EXTENSIONS, iLoop);
		LoadExtByName(strExtensionName);
	}
}

int ogl_LoadFunctions()
{
	int numFailed = 0;
	ClearExtensionVars();
	
	_ptrc_glGetIntegerv = (void (CODEGEN_FUNCPTR *)(GLenum, GLint *))IntGetProcAddress("glGetIntegerv");
	if(!_ptrc_glGetIntegerv) return ogl_LOAD_FAILED;
	_ptrc_glGetStringi = (const GLubyte * (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glGetStringi");
	if(!_ptrc_glGetStringi) return ogl_LOAD_FAILED;
	
	ProcExtsFromExtList();
	numFailed = Load_Version_3_3();
	
	if(numFailed == 0)
		return ogl_LOAD_SUCCEEDED;
	else
		return ogl_LOAD_SUCCEEDED + numFailed;
}

static int g_major_version = 0;
static int g_minor_version = 0;

static void GetGLVersion()
{
	glGetIntegerv(GL_MAJOR_VERSION, &g_major_version);
	glGetIntegerv(GL_MINOR_VERSION, &g_minor_version);
}

int ogl_GetMajorVersion()
{
	if(g_major_version == 0)
		GetGLVersion();
	return g_major_version;
}

int ogl_GetMinorVersion()
{
	if(g_major_version == 0) //Yes, check the major version to get the minor one.
		GetGLVersion();
	return g_minor_version;
}

int ogl_IsVersionGEQ(int majorVersion, int minorVersion)
{
	if(g_major_version == 0)
		GetGLVersion();
		
	if(majorVersion > g_major_version) return 1;
	if(majorVersion < g_major_version) return 0;
	if(minorVersion >= g_minor_version) return 1;
	return 0;
}

