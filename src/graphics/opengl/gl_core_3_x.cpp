#include <algorithm>
#include <vector>
#include <string.h>
#include <stddef.h>
#include "gl_core_3_x.hpp"

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
#pragma warning(disable: 4996)
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

namespace gl3x
{
	namespace gl
	{
		namespace exts
		{
			LoadTest var_EXT_texture_compression_s3tc;
			LoadTest var_EXT_texture_sRGB;
			LoadTest var_EXT_texture_filter_anisotropic;
			LoadTest var_ARB_compressed_texture_pixel_storage;
			LoadTest var_ARB_conservative_depth;
			LoadTest var_ARB_ES2_compatibility;
			LoadTest var_ARB_get_program_binary;
			LoadTest var_ARB_explicit_uniform_location;
			LoadTest var_ARB_internalformat_query;
			LoadTest var_ARB_internalformat_query2;
			LoadTest var_ARB_map_buffer_alignment;
			LoadTest var_ARB_program_interface_query;
			LoadTest var_ARB_separate_shader_objects;
			LoadTest var_ARB_shading_language_420pack;
			LoadTest var_ARB_shading_language_packing;
			LoadTest var_ARB_texture_buffer_range;
			LoadTest var_ARB_texture_storage;
			LoadTest var_ARB_texture_view;
			LoadTest var_ARB_vertex_attrib_binding;
			LoadTest var_ARB_viewport_array;
			LoadTest var_ARB_arrays_of_arrays;
			LoadTest var_ARB_clear_buffer_object;
			LoadTest var_ARB_copy_image;
			LoadTest var_ARB_ES3_compatibility;
			LoadTest var_ARB_fragment_layer_viewport;
			LoadTest var_ARB_framebuffer_no_attachments;
			LoadTest var_ARB_invalidate_subdata;
			LoadTest var_ARB_robust_buffer_access_behavior;
			LoadTest var_ARB_stencil_texturing;
			LoadTest var_ARB_texture_query_levels;
			LoadTest var_ARB_texture_storage_multisample;
			LoadTest var_KHR_debug;
			LoadTest var_ARB_buffer_storage;
			LoadTest var_ARB_clear_texture;
			LoadTest var_ARB_enhanced_layouts;
			LoadTest var_ARB_multi_bind;
			LoadTest var_ARB_query_buffer_object;
			LoadTest var_ARB_texture_mirror_clamp_to_edge;
			LoadTest var_ARB_texture_stencil8;
			LoadTest var_ARB_vertex_type_10f_11f_11f_rev;
			LoadTest var_ARB_seamless_cubemap_per_texture;
			LoadTest var_ARB_clip_control;
			LoadTest var_ARB_conditional_render_inverted;
			LoadTest var_ARB_cull_distance;
			LoadTest var_ARB_derivative_control;
			LoadTest var_ARB_direct_state_access;
			LoadTest var_ARB_get_texture_sub_image;
			LoadTest var_ARB_shader_texture_image_samples;
			LoadTest var_ARB_texture_barrier;
			LoadTest var_KHR_context_flush_control;
			LoadTest var_KHR_robust_buffer_access_behavior;
			LoadTest var_KHR_robustness;
			
		} //namespace exts
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARDEPTHF)(GLfloat);
		PFNGLCLEARDEPTHF glClearDepthf = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDEPTHRANGEF)(GLfloat, GLfloat);
		PFNGLDEPTHRANGEF glDepthRangef = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETSHADERPRECISIONFORMAT)(GLenum, GLenum, GLint *, GLint *);
		PFNGLGETSHADERPRECISIONFORMAT glGetShaderPrecisionFormat = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLRELEASESHADERCOMPILER)(void);
		PFNGLRELEASESHADERCOMPILER glReleaseShaderCompiler = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSHADERBINARY)(GLsizei, const GLuint *, GLenum, const void *, GLsizei);
		PFNGLSHADERBINARY glShaderBinary = 0;
		
		static int Load_ARB_ES2_compatibility()
		{
			int numFailed = 0;
			glClearDepthf = reinterpret_cast<PFNGLCLEARDEPTHF>(IntGetProcAddress("glClearDepthf"));
			if(!glClearDepthf) ++numFailed;
			glDepthRangef = reinterpret_cast<PFNGLDEPTHRANGEF>(IntGetProcAddress("glDepthRangef"));
			if(!glDepthRangef) ++numFailed;
			glGetShaderPrecisionFormat = reinterpret_cast<PFNGLGETSHADERPRECISIONFORMAT>(IntGetProcAddress("glGetShaderPrecisionFormat"));
			if(!glGetShaderPrecisionFormat) ++numFailed;
			glReleaseShaderCompiler = reinterpret_cast<PFNGLRELEASESHADERCOMPILER>(IntGetProcAddress("glReleaseShaderCompiler"));
			if(!glReleaseShaderCompiler) ++numFailed;
			glShaderBinary = reinterpret_cast<PFNGLSHADERBINARY>(IntGetProcAddress("glShaderBinary"));
			if(!glShaderBinary) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLGETPROGRAMBINARY)(GLuint, GLsizei, GLsizei *, GLenum *, void *);
		PFNGLGETPROGRAMBINARY glGetProgramBinary = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMBINARY)(GLuint, GLenum, const void *, GLsizei);
		PFNGLPROGRAMBINARY glProgramBinary = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMPARAMETERI)(GLuint, GLenum, GLint);
		PFNGLPROGRAMPARAMETERI glProgramParameteri = 0;
		
		static int Load_ARB_get_program_binary()
		{
			int numFailed = 0;
			glGetProgramBinary = reinterpret_cast<PFNGLGETPROGRAMBINARY>(IntGetProcAddress("glGetProgramBinary"));
			if(!glGetProgramBinary) ++numFailed;
			glProgramBinary = reinterpret_cast<PFNGLPROGRAMBINARY>(IntGetProcAddress("glProgramBinary"));
			if(!glProgramBinary) ++numFailed;
			glProgramParameteri = reinterpret_cast<PFNGLPROGRAMPARAMETERI>(IntGetProcAddress("glProgramParameteri"));
			if(!glProgramParameteri) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLGETINTERNALFORMATIV)(GLenum, GLenum, GLenum, GLsizei, GLint *);
		PFNGLGETINTERNALFORMATIV glGetInternalformativ = 0;
		
		static int Load_ARB_internalformat_query()
		{
			int numFailed = 0;
			glGetInternalformativ = reinterpret_cast<PFNGLGETINTERNALFORMATIV>(IntGetProcAddress("glGetInternalformativ"));
			if(!glGetInternalformativ) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLGETINTERNALFORMATI64V)(GLenum, GLenum, GLenum, GLsizei, GLint64 *);
		PFNGLGETINTERNALFORMATI64V glGetInternalformati64v = 0;
		
		static int Load_ARB_internalformat_query2()
		{
			int numFailed = 0;
			glGetInternalformati64v = reinterpret_cast<PFNGLGETINTERNALFORMATI64V>(IntGetProcAddress("glGetInternalformati64v"));
			if(!glGetInternalformati64v) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLGETPROGRAMINTERFACEIV)(GLuint, GLenum, GLenum, GLint *);
		PFNGLGETPROGRAMINTERFACEIV glGetProgramInterfaceiv = 0;
		typedef GLuint (CODEGEN_FUNCPTR *PFNGLGETPROGRAMRESOURCEINDEX)(GLuint, GLenum, const GLchar *);
		PFNGLGETPROGRAMRESOURCEINDEX glGetProgramResourceIndex = 0;
		typedef GLint (CODEGEN_FUNCPTR *PFNGLGETPROGRAMRESOURCELOCATION)(GLuint, GLenum, const GLchar *);
		PFNGLGETPROGRAMRESOURCELOCATION glGetProgramResourceLocation = 0;
		typedef GLint (CODEGEN_FUNCPTR *PFNGLGETPROGRAMRESOURCELOCATIONINDEX)(GLuint, GLenum, const GLchar *);
		PFNGLGETPROGRAMRESOURCELOCATIONINDEX glGetProgramResourceLocationIndex = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETPROGRAMRESOURCENAME)(GLuint, GLenum, GLuint, GLsizei, GLsizei *, GLchar *);
		PFNGLGETPROGRAMRESOURCENAME glGetProgramResourceName = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETPROGRAMRESOURCEIV)(GLuint, GLenum, GLuint, GLsizei, const GLenum *, GLsizei, GLsizei *, GLint *);
		PFNGLGETPROGRAMRESOURCEIV glGetProgramResourceiv = 0;
		
		static int Load_ARB_program_interface_query()
		{
			int numFailed = 0;
			glGetProgramInterfaceiv = reinterpret_cast<PFNGLGETPROGRAMINTERFACEIV>(IntGetProcAddress("glGetProgramInterfaceiv"));
			if(!glGetProgramInterfaceiv) ++numFailed;
			glGetProgramResourceIndex = reinterpret_cast<PFNGLGETPROGRAMRESOURCEINDEX>(IntGetProcAddress("glGetProgramResourceIndex"));
			if(!glGetProgramResourceIndex) ++numFailed;
			glGetProgramResourceLocation = reinterpret_cast<PFNGLGETPROGRAMRESOURCELOCATION>(IntGetProcAddress("glGetProgramResourceLocation"));
			if(!glGetProgramResourceLocation) ++numFailed;
			glGetProgramResourceLocationIndex = reinterpret_cast<PFNGLGETPROGRAMRESOURCELOCATIONINDEX>(IntGetProcAddress("glGetProgramResourceLocationIndex"));
			if(!glGetProgramResourceLocationIndex) ++numFailed;
			glGetProgramResourceName = reinterpret_cast<PFNGLGETPROGRAMRESOURCENAME>(IntGetProcAddress("glGetProgramResourceName"));
			if(!glGetProgramResourceName) ++numFailed;
			glGetProgramResourceiv = reinterpret_cast<PFNGLGETPROGRAMRESOURCEIV>(IntGetProcAddress("glGetProgramResourceiv"));
			if(!glGetProgramResourceiv) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLACTIVESHADERPROGRAM)(GLuint, GLuint);
		PFNGLACTIVESHADERPROGRAM glActiveShaderProgram = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDPROGRAMPIPELINE)(GLuint);
		PFNGLBINDPROGRAMPIPELINE glBindProgramPipeline = 0;
		typedef GLuint (CODEGEN_FUNCPTR *PFNGLCREATESHADERPROGRAMV)(GLenum, GLsizei, const GLchar *const*);
		PFNGLCREATESHADERPROGRAMV glCreateShaderProgramv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDELETEPROGRAMPIPELINES)(GLsizei, const GLuint *);
		PFNGLDELETEPROGRAMPIPELINES glDeleteProgramPipelines = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGENPROGRAMPIPELINES)(GLsizei, GLuint *);
		PFNGLGENPROGRAMPIPELINES glGenProgramPipelines = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETPROGRAMPIPELINEINFOLOG)(GLuint, GLsizei, GLsizei *, GLchar *);
		PFNGLGETPROGRAMPIPELINEINFOLOG glGetProgramPipelineInfoLog = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETPROGRAMPIPELINEIV)(GLuint, GLenum, GLint *);
		PFNGLGETPROGRAMPIPELINEIV glGetProgramPipelineiv = 0;
		typedef GLboolean (CODEGEN_FUNCPTR *PFNGLISPROGRAMPIPELINE)(GLuint);
		PFNGLISPROGRAMPIPELINE glIsProgramPipeline = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM1D)(GLuint, GLint, GLdouble);
		PFNGLPROGRAMUNIFORM1D glProgramUniform1d = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM1DV)(GLuint, GLint, GLsizei, const GLdouble *);
		PFNGLPROGRAMUNIFORM1DV glProgramUniform1dv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM1F)(GLuint, GLint, GLfloat);
		PFNGLPROGRAMUNIFORM1F glProgramUniform1f = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM1FV)(GLuint, GLint, GLsizei, const GLfloat *);
		PFNGLPROGRAMUNIFORM1FV glProgramUniform1fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM1I)(GLuint, GLint, GLint);
		PFNGLPROGRAMUNIFORM1I glProgramUniform1i = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM1IV)(GLuint, GLint, GLsizei, const GLint *);
		PFNGLPROGRAMUNIFORM1IV glProgramUniform1iv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM1UI)(GLuint, GLint, GLuint);
		PFNGLPROGRAMUNIFORM1UI glProgramUniform1ui = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM1UIV)(GLuint, GLint, GLsizei, const GLuint *);
		PFNGLPROGRAMUNIFORM1UIV glProgramUniform1uiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM2D)(GLuint, GLint, GLdouble, GLdouble);
		PFNGLPROGRAMUNIFORM2D glProgramUniform2d = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM2DV)(GLuint, GLint, GLsizei, const GLdouble *);
		PFNGLPROGRAMUNIFORM2DV glProgramUniform2dv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM2F)(GLuint, GLint, GLfloat, GLfloat);
		PFNGLPROGRAMUNIFORM2F glProgramUniform2f = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM2FV)(GLuint, GLint, GLsizei, const GLfloat *);
		PFNGLPROGRAMUNIFORM2FV glProgramUniform2fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM2I)(GLuint, GLint, GLint, GLint);
		PFNGLPROGRAMUNIFORM2I glProgramUniform2i = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM2IV)(GLuint, GLint, GLsizei, const GLint *);
		PFNGLPROGRAMUNIFORM2IV glProgramUniform2iv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM2UI)(GLuint, GLint, GLuint, GLuint);
		PFNGLPROGRAMUNIFORM2UI glProgramUniform2ui = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM2UIV)(GLuint, GLint, GLsizei, const GLuint *);
		PFNGLPROGRAMUNIFORM2UIV glProgramUniform2uiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM3D)(GLuint, GLint, GLdouble, GLdouble, GLdouble);
		PFNGLPROGRAMUNIFORM3D glProgramUniform3d = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM3DV)(GLuint, GLint, GLsizei, const GLdouble *);
		PFNGLPROGRAMUNIFORM3DV glProgramUniform3dv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM3F)(GLuint, GLint, GLfloat, GLfloat, GLfloat);
		PFNGLPROGRAMUNIFORM3F glProgramUniform3f = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM3FV)(GLuint, GLint, GLsizei, const GLfloat *);
		PFNGLPROGRAMUNIFORM3FV glProgramUniform3fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM3I)(GLuint, GLint, GLint, GLint, GLint);
		PFNGLPROGRAMUNIFORM3I glProgramUniform3i = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM3IV)(GLuint, GLint, GLsizei, const GLint *);
		PFNGLPROGRAMUNIFORM3IV glProgramUniform3iv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM3UI)(GLuint, GLint, GLuint, GLuint, GLuint);
		PFNGLPROGRAMUNIFORM3UI glProgramUniform3ui = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM3UIV)(GLuint, GLint, GLsizei, const GLuint *);
		PFNGLPROGRAMUNIFORM3UIV glProgramUniform3uiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM4D)(GLuint, GLint, GLdouble, GLdouble, GLdouble, GLdouble);
		PFNGLPROGRAMUNIFORM4D glProgramUniform4d = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM4DV)(GLuint, GLint, GLsizei, const GLdouble *);
		PFNGLPROGRAMUNIFORM4DV glProgramUniform4dv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM4F)(GLuint, GLint, GLfloat, GLfloat, GLfloat, GLfloat);
		PFNGLPROGRAMUNIFORM4F glProgramUniform4f = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM4FV)(GLuint, GLint, GLsizei, const GLfloat *);
		PFNGLPROGRAMUNIFORM4FV glProgramUniform4fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM4I)(GLuint, GLint, GLint, GLint, GLint, GLint);
		PFNGLPROGRAMUNIFORM4I glProgramUniform4i = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM4IV)(GLuint, GLint, GLsizei, const GLint *);
		PFNGLPROGRAMUNIFORM4IV glProgramUniform4iv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM4UI)(GLuint, GLint, GLuint, GLuint, GLuint, GLuint);
		PFNGLPROGRAMUNIFORM4UI glProgramUniform4ui = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORM4UIV)(GLuint, GLint, GLsizei, const GLuint *);
		PFNGLPROGRAMUNIFORM4UIV glProgramUniform4uiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX2DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
		PFNGLPROGRAMUNIFORMMATRIX2DV glProgramUniformMatrix2dv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX2FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLPROGRAMUNIFORMMATRIX2FV glProgramUniformMatrix2fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX2X3DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
		PFNGLPROGRAMUNIFORMMATRIX2X3DV glProgramUniformMatrix2x3dv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX2X3FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLPROGRAMUNIFORMMATRIX2X3FV glProgramUniformMatrix2x3fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX2X4DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
		PFNGLPROGRAMUNIFORMMATRIX2X4DV glProgramUniformMatrix2x4dv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX2X4FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLPROGRAMUNIFORMMATRIX2X4FV glProgramUniformMatrix2x4fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX3DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
		PFNGLPROGRAMUNIFORMMATRIX3DV glProgramUniformMatrix3dv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX3FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLPROGRAMUNIFORMMATRIX3FV glProgramUniformMatrix3fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX3X2DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
		PFNGLPROGRAMUNIFORMMATRIX3X2DV glProgramUniformMatrix3x2dv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX3X2FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLPROGRAMUNIFORMMATRIX3X2FV glProgramUniformMatrix3x2fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX3X4DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
		PFNGLPROGRAMUNIFORMMATRIX3X4DV glProgramUniformMatrix3x4dv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX3X4FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLPROGRAMUNIFORMMATRIX3X4FV glProgramUniformMatrix3x4fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX4DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
		PFNGLPROGRAMUNIFORMMATRIX4DV glProgramUniformMatrix4dv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX4FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLPROGRAMUNIFORMMATRIX4FV glProgramUniformMatrix4fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX4X2DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
		PFNGLPROGRAMUNIFORMMATRIX4X2DV glProgramUniformMatrix4x2dv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX4X2FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLPROGRAMUNIFORMMATRIX4X2FV glProgramUniformMatrix4x2fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX4X3DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
		PFNGLPROGRAMUNIFORMMATRIX4X3DV glProgramUniformMatrix4x3dv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROGRAMUNIFORMMATRIX4X3FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLPROGRAMUNIFORMMATRIX4X3FV glProgramUniformMatrix4x3fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUSEPROGRAMSTAGES)(GLuint, GLbitfield, GLuint);
		PFNGLUSEPROGRAMSTAGES glUseProgramStages = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVALIDATEPROGRAMPIPELINE)(GLuint);
		PFNGLVALIDATEPROGRAMPIPELINE glValidateProgramPipeline = 0;
		
		static int Load_ARB_separate_shader_objects()
		{
			int numFailed = 0;
			glActiveShaderProgram = reinterpret_cast<PFNGLACTIVESHADERPROGRAM>(IntGetProcAddress("glActiveShaderProgram"));
			if(!glActiveShaderProgram) ++numFailed;
			glBindProgramPipeline = reinterpret_cast<PFNGLBINDPROGRAMPIPELINE>(IntGetProcAddress("glBindProgramPipeline"));
			if(!glBindProgramPipeline) ++numFailed;
			glCreateShaderProgramv = reinterpret_cast<PFNGLCREATESHADERPROGRAMV>(IntGetProcAddress("glCreateShaderProgramv"));
			if(!glCreateShaderProgramv) ++numFailed;
			glDeleteProgramPipelines = reinterpret_cast<PFNGLDELETEPROGRAMPIPELINES>(IntGetProcAddress("glDeleteProgramPipelines"));
			if(!glDeleteProgramPipelines) ++numFailed;
			glGenProgramPipelines = reinterpret_cast<PFNGLGENPROGRAMPIPELINES>(IntGetProcAddress("glGenProgramPipelines"));
			if(!glGenProgramPipelines) ++numFailed;
			glGetProgramPipelineInfoLog = reinterpret_cast<PFNGLGETPROGRAMPIPELINEINFOLOG>(IntGetProcAddress("glGetProgramPipelineInfoLog"));
			if(!glGetProgramPipelineInfoLog) ++numFailed;
			glGetProgramPipelineiv = reinterpret_cast<PFNGLGETPROGRAMPIPELINEIV>(IntGetProcAddress("glGetProgramPipelineiv"));
			if(!glGetProgramPipelineiv) ++numFailed;
			glIsProgramPipeline = reinterpret_cast<PFNGLISPROGRAMPIPELINE>(IntGetProcAddress("glIsProgramPipeline"));
			if(!glIsProgramPipeline) ++numFailed;
			glProgramUniform1d = reinterpret_cast<PFNGLPROGRAMUNIFORM1D>(IntGetProcAddress("glProgramUniform1d"));
			if(!glProgramUniform1d) ++numFailed;
			glProgramUniform1dv = reinterpret_cast<PFNGLPROGRAMUNIFORM1DV>(IntGetProcAddress("glProgramUniform1dv"));
			if(!glProgramUniform1dv) ++numFailed;
			glProgramUniform1f = reinterpret_cast<PFNGLPROGRAMUNIFORM1F>(IntGetProcAddress("glProgramUniform1f"));
			if(!glProgramUniform1f) ++numFailed;
			glProgramUniform1fv = reinterpret_cast<PFNGLPROGRAMUNIFORM1FV>(IntGetProcAddress("glProgramUniform1fv"));
			if(!glProgramUniform1fv) ++numFailed;
			glProgramUniform1i = reinterpret_cast<PFNGLPROGRAMUNIFORM1I>(IntGetProcAddress("glProgramUniform1i"));
			if(!glProgramUniform1i) ++numFailed;
			glProgramUniform1iv = reinterpret_cast<PFNGLPROGRAMUNIFORM1IV>(IntGetProcAddress("glProgramUniform1iv"));
			if(!glProgramUniform1iv) ++numFailed;
			glProgramUniform1ui = reinterpret_cast<PFNGLPROGRAMUNIFORM1UI>(IntGetProcAddress("glProgramUniform1ui"));
			if(!glProgramUniform1ui) ++numFailed;
			glProgramUniform1uiv = reinterpret_cast<PFNGLPROGRAMUNIFORM1UIV>(IntGetProcAddress("glProgramUniform1uiv"));
			if(!glProgramUniform1uiv) ++numFailed;
			glProgramUniform2d = reinterpret_cast<PFNGLPROGRAMUNIFORM2D>(IntGetProcAddress("glProgramUniform2d"));
			if(!glProgramUniform2d) ++numFailed;
			glProgramUniform2dv = reinterpret_cast<PFNGLPROGRAMUNIFORM2DV>(IntGetProcAddress("glProgramUniform2dv"));
			if(!glProgramUniform2dv) ++numFailed;
			glProgramUniform2f = reinterpret_cast<PFNGLPROGRAMUNIFORM2F>(IntGetProcAddress("glProgramUniform2f"));
			if(!glProgramUniform2f) ++numFailed;
			glProgramUniform2fv = reinterpret_cast<PFNGLPROGRAMUNIFORM2FV>(IntGetProcAddress("glProgramUniform2fv"));
			if(!glProgramUniform2fv) ++numFailed;
			glProgramUniform2i = reinterpret_cast<PFNGLPROGRAMUNIFORM2I>(IntGetProcAddress("glProgramUniform2i"));
			if(!glProgramUniform2i) ++numFailed;
			glProgramUniform2iv = reinterpret_cast<PFNGLPROGRAMUNIFORM2IV>(IntGetProcAddress("glProgramUniform2iv"));
			if(!glProgramUniform2iv) ++numFailed;
			glProgramUniform2ui = reinterpret_cast<PFNGLPROGRAMUNIFORM2UI>(IntGetProcAddress("glProgramUniform2ui"));
			if(!glProgramUniform2ui) ++numFailed;
			glProgramUniform2uiv = reinterpret_cast<PFNGLPROGRAMUNIFORM2UIV>(IntGetProcAddress("glProgramUniform2uiv"));
			if(!glProgramUniform2uiv) ++numFailed;
			glProgramUniform3d = reinterpret_cast<PFNGLPROGRAMUNIFORM3D>(IntGetProcAddress("glProgramUniform3d"));
			if(!glProgramUniform3d) ++numFailed;
			glProgramUniform3dv = reinterpret_cast<PFNGLPROGRAMUNIFORM3DV>(IntGetProcAddress("glProgramUniform3dv"));
			if(!glProgramUniform3dv) ++numFailed;
			glProgramUniform3f = reinterpret_cast<PFNGLPROGRAMUNIFORM3F>(IntGetProcAddress("glProgramUniform3f"));
			if(!glProgramUniform3f) ++numFailed;
			glProgramUniform3fv = reinterpret_cast<PFNGLPROGRAMUNIFORM3FV>(IntGetProcAddress("glProgramUniform3fv"));
			if(!glProgramUniform3fv) ++numFailed;
			glProgramUniform3i = reinterpret_cast<PFNGLPROGRAMUNIFORM3I>(IntGetProcAddress("glProgramUniform3i"));
			if(!glProgramUniform3i) ++numFailed;
			glProgramUniform3iv = reinterpret_cast<PFNGLPROGRAMUNIFORM3IV>(IntGetProcAddress("glProgramUniform3iv"));
			if(!glProgramUniform3iv) ++numFailed;
			glProgramUniform3ui = reinterpret_cast<PFNGLPROGRAMUNIFORM3UI>(IntGetProcAddress("glProgramUniform3ui"));
			if(!glProgramUniform3ui) ++numFailed;
			glProgramUniform3uiv = reinterpret_cast<PFNGLPROGRAMUNIFORM3UIV>(IntGetProcAddress("glProgramUniform3uiv"));
			if(!glProgramUniform3uiv) ++numFailed;
			glProgramUniform4d = reinterpret_cast<PFNGLPROGRAMUNIFORM4D>(IntGetProcAddress("glProgramUniform4d"));
			if(!glProgramUniform4d) ++numFailed;
			glProgramUniform4dv = reinterpret_cast<PFNGLPROGRAMUNIFORM4DV>(IntGetProcAddress("glProgramUniform4dv"));
			if(!glProgramUniform4dv) ++numFailed;
			glProgramUniform4f = reinterpret_cast<PFNGLPROGRAMUNIFORM4F>(IntGetProcAddress("glProgramUniform4f"));
			if(!glProgramUniform4f) ++numFailed;
			glProgramUniform4fv = reinterpret_cast<PFNGLPROGRAMUNIFORM4FV>(IntGetProcAddress("glProgramUniform4fv"));
			if(!glProgramUniform4fv) ++numFailed;
			glProgramUniform4i = reinterpret_cast<PFNGLPROGRAMUNIFORM4I>(IntGetProcAddress("glProgramUniform4i"));
			if(!glProgramUniform4i) ++numFailed;
			glProgramUniform4iv = reinterpret_cast<PFNGLPROGRAMUNIFORM4IV>(IntGetProcAddress("glProgramUniform4iv"));
			if(!glProgramUniform4iv) ++numFailed;
			glProgramUniform4ui = reinterpret_cast<PFNGLPROGRAMUNIFORM4UI>(IntGetProcAddress("glProgramUniform4ui"));
			if(!glProgramUniform4ui) ++numFailed;
			glProgramUniform4uiv = reinterpret_cast<PFNGLPROGRAMUNIFORM4UIV>(IntGetProcAddress("glProgramUniform4uiv"));
			if(!glProgramUniform4uiv) ++numFailed;
			glProgramUniformMatrix2dv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX2DV>(IntGetProcAddress("glProgramUniformMatrix2dv"));
			if(!glProgramUniformMatrix2dv) ++numFailed;
			glProgramUniformMatrix2fv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX2FV>(IntGetProcAddress("glProgramUniformMatrix2fv"));
			if(!glProgramUniformMatrix2fv) ++numFailed;
			glProgramUniformMatrix2x3dv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX2X3DV>(IntGetProcAddress("glProgramUniformMatrix2x3dv"));
			if(!glProgramUniformMatrix2x3dv) ++numFailed;
			glProgramUniformMatrix2x3fv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX2X3FV>(IntGetProcAddress("glProgramUniformMatrix2x3fv"));
			if(!glProgramUniformMatrix2x3fv) ++numFailed;
			glProgramUniformMatrix2x4dv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX2X4DV>(IntGetProcAddress("glProgramUniformMatrix2x4dv"));
			if(!glProgramUniformMatrix2x4dv) ++numFailed;
			glProgramUniformMatrix2x4fv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX2X4FV>(IntGetProcAddress("glProgramUniformMatrix2x4fv"));
			if(!glProgramUniformMatrix2x4fv) ++numFailed;
			glProgramUniformMatrix3dv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX3DV>(IntGetProcAddress("glProgramUniformMatrix3dv"));
			if(!glProgramUniformMatrix3dv) ++numFailed;
			glProgramUniformMatrix3fv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX3FV>(IntGetProcAddress("glProgramUniformMatrix3fv"));
			if(!glProgramUniformMatrix3fv) ++numFailed;
			glProgramUniformMatrix3x2dv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX3X2DV>(IntGetProcAddress("glProgramUniformMatrix3x2dv"));
			if(!glProgramUniformMatrix3x2dv) ++numFailed;
			glProgramUniformMatrix3x2fv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX3X2FV>(IntGetProcAddress("glProgramUniformMatrix3x2fv"));
			if(!glProgramUniformMatrix3x2fv) ++numFailed;
			glProgramUniformMatrix3x4dv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX3X4DV>(IntGetProcAddress("glProgramUniformMatrix3x4dv"));
			if(!glProgramUniformMatrix3x4dv) ++numFailed;
			glProgramUniformMatrix3x4fv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX3X4FV>(IntGetProcAddress("glProgramUniformMatrix3x4fv"));
			if(!glProgramUniformMatrix3x4fv) ++numFailed;
			glProgramUniformMatrix4dv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX4DV>(IntGetProcAddress("glProgramUniformMatrix4dv"));
			if(!glProgramUniformMatrix4dv) ++numFailed;
			glProgramUniformMatrix4fv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX4FV>(IntGetProcAddress("glProgramUniformMatrix4fv"));
			if(!glProgramUniformMatrix4fv) ++numFailed;
			glProgramUniformMatrix4x2dv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX4X2DV>(IntGetProcAddress("glProgramUniformMatrix4x2dv"));
			if(!glProgramUniformMatrix4x2dv) ++numFailed;
			glProgramUniformMatrix4x2fv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX4X2FV>(IntGetProcAddress("glProgramUniformMatrix4x2fv"));
			if(!glProgramUniformMatrix4x2fv) ++numFailed;
			glProgramUniformMatrix4x3dv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX4X3DV>(IntGetProcAddress("glProgramUniformMatrix4x3dv"));
			if(!glProgramUniformMatrix4x3dv) ++numFailed;
			glProgramUniformMatrix4x3fv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX4X3FV>(IntGetProcAddress("glProgramUniformMatrix4x3fv"));
			if(!glProgramUniformMatrix4x3fv) ++numFailed;
			glUseProgramStages = reinterpret_cast<PFNGLUSEPROGRAMSTAGES>(IntGetProcAddress("glUseProgramStages"));
			if(!glUseProgramStages) ++numFailed;
			glValidateProgramPipeline = reinterpret_cast<PFNGLVALIDATEPROGRAMPIPELINE>(IntGetProcAddress("glValidateProgramPipeline"));
			if(!glValidateProgramPipeline) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXBUFFERRANGE)(GLenum, GLenum, GLuint, GLintptr, GLsizeiptr);
		PFNGLTEXBUFFERRANGE glTexBufferRange = 0;
		
		static int Load_ARB_texture_buffer_range()
		{
			int numFailed = 0;
			glTexBufferRange = reinterpret_cast<PFNGLTEXBUFFERRANGE>(IntGetProcAddress("glTexBufferRange"));
			if(!glTexBufferRange) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXSTORAGE1D)(GLenum, GLsizei, GLenum, GLsizei);
		PFNGLTEXSTORAGE1D glTexStorage1D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXSTORAGE2D)(GLenum, GLsizei, GLenum, GLsizei, GLsizei);
		PFNGLTEXSTORAGE2D glTexStorage2D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXSTORAGE3D)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei);
		PFNGLTEXSTORAGE3D glTexStorage3D = 0;
		
		static int Load_ARB_texture_storage()
		{
			int numFailed = 0;
			glTexStorage1D = reinterpret_cast<PFNGLTEXSTORAGE1D>(IntGetProcAddress("glTexStorage1D"));
			if(!glTexStorage1D) ++numFailed;
			glTexStorage2D = reinterpret_cast<PFNGLTEXSTORAGE2D>(IntGetProcAddress("glTexStorage2D"));
			if(!glTexStorage2D) ++numFailed;
			glTexStorage3D = reinterpret_cast<PFNGLTEXSTORAGE3D>(IntGetProcAddress("glTexStorage3D"));
			if(!glTexStorage3D) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTUREVIEW)(GLuint, GLenum, GLuint, GLenum, GLuint, GLuint, GLuint, GLuint);
		PFNGLTEXTUREVIEW glTextureView = 0;
		
		static int Load_ARB_texture_view()
		{
			int numFailed = 0;
			glTextureView = reinterpret_cast<PFNGLTEXTUREVIEW>(IntGetProcAddress("glTextureView"));
			if(!glTextureView) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDVERTEXBUFFER)(GLuint, GLuint, GLintptr, GLsizei);
		PFNGLBINDVERTEXBUFFER glBindVertexBuffer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBBINDING)(GLuint, GLuint);
		PFNGLVERTEXATTRIBBINDING glVertexAttribBinding = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBFORMAT)(GLuint, GLint, GLenum, GLboolean, GLuint);
		PFNGLVERTEXATTRIBFORMAT glVertexAttribFormat = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBIFORMAT)(GLuint, GLint, GLenum, GLuint);
		PFNGLVERTEXATTRIBIFORMAT glVertexAttribIFormat = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBLFORMAT)(GLuint, GLint, GLenum, GLuint);
		PFNGLVERTEXATTRIBLFORMAT glVertexAttribLFormat = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXBINDINGDIVISOR)(GLuint, GLuint);
		PFNGLVERTEXBINDINGDIVISOR glVertexBindingDivisor = 0;
		
		static int Load_ARB_vertex_attrib_binding()
		{
			int numFailed = 0;
			glBindVertexBuffer = reinterpret_cast<PFNGLBINDVERTEXBUFFER>(IntGetProcAddress("glBindVertexBuffer"));
			if(!glBindVertexBuffer) ++numFailed;
			glVertexAttribBinding = reinterpret_cast<PFNGLVERTEXATTRIBBINDING>(IntGetProcAddress("glVertexAttribBinding"));
			if(!glVertexAttribBinding) ++numFailed;
			glVertexAttribFormat = reinterpret_cast<PFNGLVERTEXATTRIBFORMAT>(IntGetProcAddress("glVertexAttribFormat"));
			if(!glVertexAttribFormat) ++numFailed;
			glVertexAttribIFormat = reinterpret_cast<PFNGLVERTEXATTRIBIFORMAT>(IntGetProcAddress("glVertexAttribIFormat"));
			if(!glVertexAttribIFormat) ++numFailed;
			glVertexAttribLFormat = reinterpret_cast<PFNGLVERTEXATTRIBLFORMAT>(IntGetProcAddress("glVertexAttribLFormat"));
			if(!glVertexAttribLFormat) ++numFailed;
			glVertexBindingDivisor = reinterpret_cast<PFNGLVERTEXBINDINGDIVISOR>(IntGetProcAddress("glVertexBindingDivisor"));
			if(!glVertexBindingDivisor) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLDEPTHRANGEARRAYV)(GLuint, GLsizei, const GLdouble *);
		PFNGLDEPTHRANGEARRAYV glDepthRangeArrayv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDEPTHRANGEINDEXED)(GLuint, GLdouble, GLdouble);
		PFNGLDEPTHRANGEINDEXED glDepthRangeIndexed = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETDOUBLEI_V)(GLenum, GLuint, GLdouble *);
		PFNGLGETDOUBLEI_V glGetDoublei_v = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETFLOATI_V)(GLenum, GLuint, GLfloat *);
		PFNGLGETFLOATI_V glGetFloati_v = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSCISSORARRAYV)(GLuint, GLsizei, const GLint *);
		PFNGLSCISSORARRAYV glScissorArrayv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSCISSORINDEXED)(GLuint, GLint, GLint, GLsizei, GLsizei);
		PFNGLSCISSORINDEXED glScissorIndexed = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSCISSORINDEXEDV)(GLuint, const GLint *);
		PFNGLSCISSORINDEXEDV glScissorIndexedv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVIEWPORTARRAYV)(GLuint, GLsizei, const GLfloat *);
		PFNGLVIEWPORTARRAYV glViewportArrayv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVIEWPORTINDEXEDF)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
		PFNGLVIEWPORTINDEXEDF glViewportIndexedf = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVIEWPORTINDEXEDFV)(GLuint, const GLfloat *);
		PFNGLVIEWPORTINDEXEDFV glViewportIndexedfv = 0;
		
		static int Load_ARB_viewport_array()
		{
			int numFailed = 0;
			glDepthRangeArrayv = reinterpret_cast<PFNGLDEPTHRANGEARRAYV>(IntGetProcAddress("glDepthRangeArrayv"));
			if(!glDepthRangeArrayv) ++numFailed;
			glDepthRangeIndexed = reinterpret_cast<PFNGLDEPTHRANGEINDEXED>(IntGetProcAddress("glDepthRangeIndexed"));
			if(!glDepthRangeIndexed) ++numFailed;
			glGetDoublei_v = reinterpret_cast<PFNGLGETDOUBLEI_V>(IntGetProcAddress("glGetDoublei_v"));
			if(!glGetDoublei_v) ++numFailed;
			glGetFloati_v = reinterpret_cast<PFNGLGETFLOATI_V>(IntGetProcAddress("glGetFloati_v"));
			if(!glGetFloati_v) ++numFailed;
			glScissorArrayv = reinterpret_cast<PFNGLSCISSORARRAYV>(IntGetProcAddress("glScissorArrayv"));
			if(!glScissorArrayv) ++numFailed;
			glScissorIndexed = reinterpret_cast<PFNGLSCISSORINDEXED>(IntGetProcAddress("glScissorIndexed"));
			if(!glScissorIndexed) ++numFailed;
			glScissorIndexedv = reinterpret_cast<PFNGLSCISSORINDEXEDV>(IntGetProcAddress("glScissorIndexedv"));
			if(!glScissorIndexedv) ++numFailed;
			glViewportArrayv = reinterpret_cast<PFNGLVIEWPORTARRAYV>(IntGetProcAddress("glViewportArrayv"));
			if(!glViewportArrayv) ++numFailed;
			glViewportIndexedf = reinterpret_cast<PFNGLVIEWPORTINDEXEDF>(IntGetProcAddress("glViewportIndexedf"));
			if(!glViewportIndexedf) ++numFailed;
			glViewportIndexedfv = reinterpret_cast<PFNGLVIEWPORTINDEXEDFV>(IntGetProcAddress("glViewportIndexedfv"));
			if(!glViewportIndexedfv) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARBUFFERDATA)(GLenum, GLenum, GLenum, GLenum, const void *);
		PFNGLCLEARBUFFERDATA glClearBufferData = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARBUFFERSUBDATA)(GLenum, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void *);
		PFNGLCLEARBUFFERSUBDATA glClearBufferSubData = 0;
		
		static int Load_ARB_clear_buffer_object()
		{
			int numFailed = 0;
			glClearBufferData = reinterpret_cast<PFNGLCLEARBUFFERDATA>(IntGetProcAddress("glClearBufferData"));
			if(!glClearBufferData) ++numFailed;
			glClearBufferSubData = reinterpret_cast<PFNGLCLEARBUFFERSUBDATA>(IntGetProcAddress("glClearBufferSubData"));
			if(!glClearBufferSubData) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLCOPYIMAGESUBDATA)(GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei);
		PFNGLCOPYIMAGESUBDATA glCopyImageSubData = 0;
		
		static int Load_ARB_copy_image()
		{
			int numFailed = 0;
			glCopyImageSubData = reinterpret_cast<PFNGLCOPYIMAGESUBDATA>(IntGetProcAddress("glCopyImageSubData"));
			if(!glCopyImageSubData) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLFRAMEBUFFERPARAMETERI)(GLenum, GLenum, GLint);
		PFNGLFRAMEBUFFERPARAMETERI glFramebufferParameteri = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETFRAMEBUFFERPARAMETERIV)(GLenum, GLenum, GLint *);
		PFNGLGETFRAMEBUFFERPARAMETERIV glGetFramebufferParameteriv = 0;
		
		static int Load_ARB_framebuffer_no_attachments()
		{
			int numFailed = 0;
			glFramebufferParameteri = reinterpret_cast<PFNGLFRAMEBUFFERPARAMETERI>(IntGetProcAddress("glFramebufferParameteri"));
			if(!glFramebufferParameteri) ++numFailed;
			glGetFramebufferParameteriv = reinterpret_cast<PFNGLGETFRAMEBUFFERPARAMETERIV>(IntGetProcAddress("glGetFramebufferParameteriv"));
			if(!glGetFramebufferParameteriv) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLINVALIDATEBUFFERDATA)(GLuint);
		PFNGLINVALIDATEBUFFERDATA glInvalidateBufferData = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLINVALIDATEBUFFERSUBDATA)(GLuint, GLintptr, GLsizeiptr);
		PFNGLINVALIDATEBUFFERSUBDATA glInvalidateBufferSubData = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLINVALIDATEFRAMEBUFFER)(GLenum, GLsizei, const GLenum *);
		PFNGLINVALIDATEFRAMEBUFFER glInvalidateFramebuffer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLINVALIDATESUBFRAMEBUFFER)(GLenum, GLsizei, const GLenum *, GLint, GLint, GLsizei, GLsizei);
		PFNGLINVALIDATESUBFRAMEBUFFER glInvalidateSubFramebuffer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLINVALIDATETEXIMAGE)(GLuint, GLint);
		PFNGLINVALIDATETEXIMAGE glInvalidateTexImage = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLINVALIDATETEXSUBIMAGE)(GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei);
		PFNGLINVALIDATETEXSUBIMAGE glInvalidateTexSubImage = 0;
		
		static int Load_ARB_invalidate_subdata()
		{
			int numFailed = 0;
			glInvalidateBufferData = reinterpret_cast<PFNGLINVALIDATEBUFFERDATA>(IntGetProcAddress("glInvalidateBufferData"));
			if(!glInvalidateBufferData) ++numFailed;
			glInvalidateBufferSubData = reinterpret_cast<PFNGLINVALIDATEBUFFERSUBDATA>(IntGetProcAddress("glInvalidateBufferSubData"));
			if(!glInvalidateBufferSubData) ++numFailed;
			glInvalidateFramebuffer = reinterpret_cast<PFNGLINVALIDATEFRAMEBUFFER>(IntGetProcAddress("glInvalidateFramebuffer"));
			if(!glInvalidateFramebuffer) ++numFailed;
			glInvalidateSubFramebuffer = reinterpret_cast<PFNGLINVALIDATESUBFRAMEBUFFER>(IntGetProcAddress("glInvalidateSubFramebuffer"));
			if(!glInvalidateSubFramebuffer) ++numFailed;
			glInvalidateTexImage = reinterpret_cast<PFNGLINVALIDATETEXIMAGE>(IntGetProcAddress("glInvalidateTexImage"));
			if(!glInvalidateTexImage) ++numFailed;
			glInvalidateTexSubImage = reinterpret_cast<PFNGLINVALIDATETEXSUBIMAGE>(IntGetProcAddress("glInvalidateTexSubImage"));
			if(!glInvalidateTexSubImage) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXSTORAGE2DMULTISAMPLE)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean);
		PFNGLTEXSTORAGE2DMULTISAMPLE glTexStorage2DMultisample = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXSTORAGE3DMULTISAMPLE)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean);
		PFNGLTEXSTORAGE3DMULTISAMPLE glTexStorage3DMultisample = 0;
		
		static int Load_ARB_texture_storage_multisample()
		{
			int numFailed = 0;
			glTexStorage2DMultisample = reinterpret_cast<PFNGLTEXSTORAGE2DMULTISAMPLE>(IntGetProcAddress("glTexStorage2DMultisample"));
			if(!glTexStorage2DMultisample) ++numFailed;
			glTexStorage3DMultisample = reinterpret_cast<PFNGLTEXSTORAGE3DMULTISAMPLE>(IntGetProcAddress("glTexStorage3DMultisample"));
			if(!glTexStorage3DMultisample) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLDEBUGMESSAGECALLBACK)(GLDEBUGPROC, const void *);
		PFNGLDEBUGMESSAGECALLBACK glDebugMessageCallback = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDEBUGMESSAGECONTROL)(GLenum, GLenum, GLenum, GLsizei, const GLuint *, GLboolean);
		PFNGLDEBUGMESSAGECONTROL glDebugMessageControl = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDEBUGMESSAGEINSERT)(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *);
		PFNGLDEBUGMESSAGEINSERT glDebugMessageInsert = 0;
		typedef GLuint (CODEGEN_FUNCPTR *PFNGLGETDEBUGMESSAGELOG)(GLuint, GLsizei, GLenum *, GLenum *, GLuint *, GLenum *, GLsizei *, GLchar *);
		PFNGLGETDEBUGMESSAGELOG glGetDebugMessageLog = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETOBJECTLABEL)(GLenum, GLuint, GLsizei, GLsizei *, GLchar *);
		PFNGLGETOBJECTLABEL glGetObjectLabel = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETOBJECTPTRLABEL)(const void *, GLsizei, GLsizei *, GLchar *);
		PFNGLGETOBJECTPTRLABEL glGetObjectPtrLabel = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETPOINTERV)(GLenum, void **);
		PFNGLGETPOINTERV glGetPointerv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLOBJECTLABEL)(GLenum, GLuint, GLsizei, const GLchar *);
		PFNGLOBJECTLABEL glObjectLabel = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLOBJECTPTRLABEL)(const void *, GLsizei, const GLchar *);
		PFNGLOBJECTPTRLABEL glObjectPtrLabel = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPOPDEBUGGROUP)(void);
		PFNGLPOPDEBUGGROUP glPopDebugGroup = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPUSHDEBUGGROUP)(GLenum, GLuint, GLsizei, const GLchar *);
		PFNGLPUSHDEBUGGROUP glPushDebugGroup = 0;
		
		static int Load_KHR_debug()
		{
			int numFailed = 0;
			glDebugMessageCallback = reinterpret_cast<PFNGLDEBUGMESSAGECALLBACK>(IntGetProcAddress("glDebugMessageCallback"));
			if(!glDebugMessageCallback) ++numFailed;
			glDebugMessageControl = reinterpret_cast<PFNGLDEBUGMESSAGECONTROL>(IntGetProcAddress("glDebugMessageControl"));
			if(!glDebugMessageControl) ++numFailed;
			glDebugMessageInsert = reinterpret_cast<PFNGLDEBUGMESSAGEINSERT>(IntGetProcAddress("glDebugMessageInsert"));
			if(!glDebugMessageInsert) ++numFailed;
			glGetDebugMessageLog = reinterpret_cast<PFNGLGETDEBUGMESSAGELOG>(IntGetProcAddress("glGetDebugMessageLog"));
			if(!glGetDebugMessageLog) ++numFailed;
			glGetObjectLabel = reinterpret_cast<PFNGLGETOBJECTLABEL>(IntGetProcAddress("glGetObjectLabel"));
			if(!glGetObjectLabel) ++numFailed;
			glGetObjectPtrLabel = reinterpret_cast<PFNGLGETOBJECTPTRLABEL>(IntGetProcAddress("glGetObjectPtrLabel"));
			if(!glGetObjectPtrLabel) ++numFailed;
			glGetPointerv = reinterpret_cast<PFNGLGETPOINTERV>(IntGetProcAddress("glGetPointerv"));
			if(!glGetPointerv) ++numFailed;
			glObjectLabel = reinterpret_cast<PFNGLOBJECTLABEL>(IntGetProcAddress("glObjectLabel"));
			if(!glObjectLabel) ++numFailed;
			glObjectPtrLabel = reinterpret_cast<PFNGLOBJECTPTRLABEL>(IntGetProcAddress("glObjectPtrLabel"));
			if(!glObjectPtrLabel) ++numFailed;
			glPopDebugGroup = reinterpret_cast<PFNGLPOPDEBUGGROUP>(IntGetProcAddress("glPopDebugGroup"));
			if(!glPopDebugGroup) ++numFailed;
			glPushDebugGroup = reinterpret_cast<PFNGLPUSHDEBUGGROUP>(IntGetProcAddress("glPushDebugGroup"));
			if(!glPushDebugGroup) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLBUFFERSTORAGE)(GLenum, GLsizeiptr, const void *, GLbitfield);
		PFNGLBUFFERSTORAGE glBufferStorage = 0;
		
		static int Load_ARB_buffer_storage()
		{
			int numFailed = 0;
			glBufferStorage = reinterpret_cast<PFNGLBUFFERSTORAGE>(IntGetProcAddress("glBufferStorage"));
			if(!glBufferStorage) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARTEXIMAGE)(GLuint, GLint, GLenum, GLenum, const void *);
		PFNGLCLEARTEXIMAGE glClearTexImage = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARTEXSUBIMAGE)(GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void *);
		PFNGLCLEARTEXSUBIMAGE glClearTexSubImage = 0;
		
		static int Load_ARB_clear_texture()
		{
			int numFailed = 0;
			glClearTexImage = reinterpret_cast<PFNGLCLEARTEXIMAGE>(IntGetProcAddress("glClearTexImage"));
			if(!glClearTexImage) ++numFailed;
			glClearTexSubImage = reinterpret_cast<PFNGLCLEARTEXSUBIMAGE>(IntGetProcAddress("glClearTexSubImage"));
			if(!glClearTexSubImage) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDBUFFERSBASE)(GLenum, GLuint, GLsizei, const GLuint *);
		PFNGLBINDBUFFERSBASE glBindBuffersBase = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDBUFFERSRANGE)(GLenum, GLuint, GLsizei, const GLuint *, const GLintptr *, const GLsizeiptr *);
		PFNGLBINDBUFFERSRANGE glBindBuffersRange = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDIMAGETEXTURES)(GLuint, GLsizei, const GLuint *);
		PFNGLBINDIMAGETEXTURES glBindImageTextures = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDSAMPLERS)(GLuint, GLsizei, const GLuint *);
		PFNGLBINDSAMPLERS glBindSamplers = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDTEXTURES)(GLuint, GLsizei, const GLuint *);
		PFNGLBINDTEXTURES glBindTextures = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDVERTEXBUFFERS)(GLuint, GLsizei, const GLuint *, const GLintptr *, const GLsizei *);
		PFNGLBINDVERTEXBUFFERS glBindVertexBuffers = 0;
		
		static int Load_ARB_multi_bind()
		{
			int numFailed = 0;
			glBindBuffersBase = reinterpret_cast<PFNGLBINDBUFFERSBASE>(IntGetProcAddress("glBindBuffersBase"));
			if(!glBindBuffersBase) ++numFailed;
			glBindBuffersRange = reinterpret_cast<PFNGLBINDBUFFERSRANGE>(IntGetProcAddress("glBindBuffersRange"));
			if(!glBindBuffersRange) ++numFailed;
			glBindImageTextures = reinterpret_cast<PFNGLBINDIMAGETEXTURES>(IntGetProcAddress("glBindImageTextures"));
			if(!glBindImageTextures) ++numFailed;
			glBindSamplers = reinterpret_cast<PFNGLBINDSAMPLERS>(IntGetProcAddress("glBindSamplers"));
			if(!glBindSamplers) ++numFailed;
			glBindTextures = reinterpret_cast<PFNGLBINDTEXTURES>(IntGetProcAddress("glBindTextures"));
			if(!glBindTextures) ++numFailed;
			glBindVertexBuffers = reinterpret_cast<PFNGLBINDVERTEXBUFFERS>(IntGetProcAddress("glBindVertexBuffers"));
			if(!glBindVertexBuffers) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLCLIPCONTROL)(GLenum, GLenum);
		PFNGLCLIPCONTROL glClipControl = 0;
		
		static int Load_ARB_clip_control()
		{
			int numFailed = 0;
			glClipControl = reinterpret_cast<PFNGLCLIPCONTROL>(IntGetProcAddress("glClipControl"));
			if(!glClipControl) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDTEXTUREUNIT)(GLuint, GLuint);
		PFNGLBINDTEXTUREUNIT glBindTextureUnit = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBLITNAMEDFRAMEBUFFER)(GLuint, GLuint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
		PFNGLBLITNAMEDFRAMEBUFFER glBlitNamedFramebuffer = 0;
		typedef GLenum (CODEGEN_FUNCPTR *PFNGLCHECKNAMEDFRAMEBUFFERSTATUS)(GLuint, GLenum);
		PFNGLCHECKNAMEDFRAMEBUFFERSTATUS glCheckNamedFramebufferStatus = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARNAMEDBUFFERDATA)(GLuint, GLenum, GLenum, GLenum, const void *);
		PFNGLCLEARNAMEDBUFFERDATA glClearNamedBufferData = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARNAMEDBUFFERSUBDATA)(GLuint, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void *);
		PFNGLCLEARNAMEDBUFFERSUBDATA glClearNamedBufferSubData = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARNAMEDFRAMEBUFFERFI)(GLuint, GLenum, GLint, const GLfloat, GLint);
		PFNGLCLEARNAMEDFRAMEBUFFERFI glClearNamedFramebufferfi = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARNAMEDFRAMEBUFFERFV)(GLuint, GLenum, GLint, const GLfloat *);
		PFNGLCLEARNAMEDFRAMEBUFFERFV glClearNamedFramebufferfv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARNAMEDFRAMEBUFFERIV)(GLuint, GLenum, GLint, const GLint *);
		PFNGLCLEARNAMEDFRAMEBUFFERIV glClearNamedFramebufferiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARNAMEDFRAMEBUFFERUIV)(GLuint, GLenum, GLint, const GLuint *);
		PFNGLCLEARNAMEDFRAMEBUFFERUIV glClearNamedFramebufferuiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOMPRESSEDTEXTURESUBIMAGE1D)(GLuint, GLint, GLint, GLsizei, GLenum, GLsizei, const void *);
		PFNGLCOMPRESSEDTEXTURESUBIMAGE1D glCompressedTextureSubImage1D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOMPRESSEDTEXTURESUBIMAGE2D)(GLuint, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void *);
		PFNGLCOMPRESSEDTEXTURESUBIMAGE2D glCompressedTextureSubImage2D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOMPRESSEDTEXTURESUBIMAGE3D)(GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void *);
		PFNGLCOMPRESSEDTEXTURESUBIMAGE3D glCompressedTextureSubImage3D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOPYNAMEDBUFFERSUBDATA)(GLuint, GLuint, GLintptr, GLintptr, GLsizeiptr);
		PFNGLCOPYNAMEDBUFFERSUBDATA glCopyNamedBufferSubData = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOPYTEXTURESUBIMAGE1D)(GLuint, GLint, GLint, GLint, GLint, GLsizei);
		PFNGLCOPYTEXTURESUBIMAGE1D glCopyTextureSubImage1D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOPYTEXTURESUBIMAGE2D)(GLuint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
		PFNGLCOPYTEXTURESUBIMAGE2D glCopyTextureSubImage2D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOPYTEXTURESUBIMAGE3D)(GLuint, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
		PFNGLCOPYTEXTURESUBIMAGE3D glCopyTextureSubImage3D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCREATEBUFFERS)(GLsizei, GLuint *);
		PFNGLCREATEBUFFERS glCreateBuffers = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCREATEFRAMEBUFFERS)(GLsizei, GLuint *);
		PFNGLCREATEFRAMEBUFFERS glCreateFramebuffers = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCREATEPROGRAMPIPELINES)(GLsizei, GLuint *);
		PFNGLCREATEPROGRAMPIPELINES glCreateProgramPipelines = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCREATEQUERIES)(GLenum, GLsizei, GLuint *);
		PFNGLCREATEQUERIES glCreateQueries = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCREATERENDERBUFFERS)(GLsizei, GLuint *);
		PFNGLCREATERENDERBUFFERS glCreateRenderbuffers = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCREATESAMPLERS)(GLsizei, GLuint *);
		PFNGLCREATESAMPLERS glCreateSamplers = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCREATETEXTURES)(GLenum, GLsizei, GLuint *);
		PFNGLCREATETEXTURES glCreateTextures = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCREATETRANSFORMFEEDBACKS)(GLsizei, GLuint *);
		PFNGLCREATETRANSFORMFEEDBACKS glCreateTransformFeedbacks = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCREATEVERTEXARRAYS)(GLsizei, GLuint *);
		PFNGLCREATEVERTEXARRAYS glCreateVertexArrays = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDISABLEVERTEXARRAYATTRIB)(GLuint, GLuint);
		PFNGLDISABLEVERTEXARRAYATTRIB glDisableVertexArrayAttrib = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLENABLEVERTEXARRAYATTRIB)(GLuint, GLuint);
		PFNGLENABLEVERTEXARRAYATTRIB glEnableVertexArrayAttrib = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLFLUSHMAPPEDNAMEDBUFFERRANGE)(GLuint, GLintptr, GLsizeiptr);
		PFNGLFLUSHMAPPEDNAMEDBUFFERRANGE glFlushMappedNamedBufferRange = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGENERATETEXTUREMIPMAP)(GLuint);
		PFNGLGENERATETEXTUREMIPMAP glGenerateTextureMipmap = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETCOMPRESSEDTEXTUREIMAGE)(GLuint, GLint, GLsizei, void *);
		PFNGLGETCOMPRESSEDTEXTUREIMAGE glGetCompressedTextureImage = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETNAMEDBUFFERPARAMETERI64V)(GLuint, GLenum, GLint64 *);
		PFNGLGETNAMEDBUFFERPARAMETERI64V glGetNamedBufferParameteri64v = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETNAMEDBUFFERPARAMETERIV)(GLuint, GLenum, GLint *);
		PFNGLGETNAMEDBUFFERPARAMETERIV glGetNamedBufferParameteriv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETNAMEDBUFFERPOINTERV)(GLuint, GLenum, void **);
		PFNGLGETNAMEDBUFFERPOINTERV glGetNamedBufferPointerv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETNAMEDBUFFERSUBDATA)(GLuint, GLintptr, GLsizeiptr, void *);
		PFNGLGETNAMEDBUFFERSUBDATA glGetNamedBufferSubData = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIV)(GLuint, GLenum, GLenum, GLint *);
		PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIV glGetNamedFramebufferAttachmentParameteriv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETNAMEDFRAMEBUFFERPARAMETERIV)(GLuint, GLenum, GLint *);
		PFNGLGETNAMEDFRAMEBUFFERPARAMETERIV glGetNamedFramebufferParameteriv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETNAMEDRENDERBUFFERPARAMETERIV)(GLuint, GLenum, GLint *);
		PFNGLGETNAMEDRENDERBUFFERPARAMETERIV glGetNamedRenderbufferParameteriv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETQUERYBUFFEROBJECTI64V)(GLuint, GLuint, GLenum, GLintptr);
		PFNGLGETQUERYBUFFEROBJECTI64V glGetQueryBufferObjecti64v = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETQUERYBUFFEROBJECTIV)(GLuint, GLuint, GLenum, GLintptr);
		PFNGLGETQUERYBUFFEROBJECTIV glGetQueryBufferObjectiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETQUERYBUFFEROBJECTUI64V)(GLuint, GLuint, GLenum, GLintptr);
		PFNGLGETQUERYBUFFEROBJECTUI64V glGetQueryBufferObjectui64v = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETQUERYBUFFEROBJECTUIV)(GLuint, GLuint, GLenum, GLintptr);
		PFNGLGETQUERYBUFFEROBJECTUIV glGetQueryBufferObjectuiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTEXTUREIMAGE)(GLuint, GLint, GLenum, GLenum, GLsizei, void *);
		PFNGLGETTEXTUREIMAGE glGetTextureImage = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTEXTURELEVELPARAMETERFV)(GLuint, GLint, GLenum, GLfloat *);
		PFNGLGETTEXTURELEVELPARAMETERFV glGetTextureLevelParameterfv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTEXTURELEVELPARAMETERIV)(GLuint, GLint, GLenum, GLint *);
		PFNGLGETTEXTURELEVELPARAMETERIV glGetTextureLevelParameteriv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTEXTUREPARAMETERIIV)(GLuint, GLenum, GLint *);
		PFNGLGETTEXTUREPARAMETERIIV glGetTextureParameterIiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTEXTUREPARAMETERIUIV)(GLuint, GLenum, GLuint *);
		PFNGLGETTEXTUREPARAMETERIUIV glGetTextureParameterIuiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTEXTUREPARAMETERFV)(GLuint, GLenum, GLfloat *);
		PFNGLGETTEXTUREPARAMETERFV glGetTextureParameterfv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTEXTUREPARAMETERIV)(GLuint, GLenum, GLint *);
		PFNGLGETTEXTUREPARAMETERIV glGetTextureParameteriv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTRANSFORMFEEDBACKI64_V)(GLuint, GLenum, GLuint, GLint64 *);
		PFNGLGETTRANSFORMFEEDBACKI64_V glGetTransformFeedbacki64_v = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTRANSFORMFEEDBACKI_V)(GLuint, GLenum, GLuint, GLint *);
		PFNGLGETTRANSFORMFEEDBACKI_V glGetTransformFeedbacki_v = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTRANSFORMFEEDBACKIV)(GLuint, GLenum, GLint *);
		PFNGLGETTRANSFORMFEEDBACKIV glGetTransformFeedbackiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETVERTEXARRAYINDEXED64IV)(GLuint, GLuint, GLenum, GLint64 *);
		PFNGLGETVERTEXARRAYINDEXED64IV glGetVertexArrayIndexed64iv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETVERTEXARRAYINDEXEDIV)(GLuint, GLuint, GLenum, GLint *);
		PFNGLGETVERTEXARRAYINDEXEDIV glGetVertexArrayIndexediv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETVERTEXARRAYIV)(GLuint, GLenum, GLint *);
		PFNGLGETVERTEXARRAYIV glGetVertexArrayiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLINVALIDATENAMEDFRAMEBUFFERDATA)(GLuint, GLsizei, const GLenum *);
		PFNGLINVALIDATENAMEDFRAMEBUFFERDATA glInvalidateNamedFramebufferData = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATA)(GLuint, GLsizei, const GLenum *, GLint, GLint, GLsizei, GLsizei);
		PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATA glInvalidateNamedFramebufferSubData = 0;
		typedef void * (CODEGEN_FUNCPTR *PFNGLMAPNAMEDBUFFER)(GLuint, GLenum);
		PFNGLMAPNAMEDBUFFER glMapNamedBuffer = 0;
		typedef void * (CODEGEN_FUNCPTR *PFNGLMAPNAMEDBUFFERRANGE)(GLuint, GLintptr, GLsizeiptr, GLbitfield);
		PFNGLMAPNAMEDBUFFERRANGE glMapNamedBufferRange = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLNAMEDBUFFERDATA)(GLuint, GLsizeiptr, const void *, GLenum);
		PFNGLNAMEDBUFFERDATA glNamedBufferData = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLNAMEDBUFFERSTORAGE)(GLuint, GLsizeiptr, const void *, GLbitfield);
		PFNGLNAMEDBUFFERSTORAGE glNamedBufferStorage = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLNAMEDBUFFERSUBDATA)(GLuint, GLintptr, GLsizeiptr, const void *);
		PFNGLNAMEDBUFFERSUBDATA glNamedBufferSubData = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLNAMEDFRAMEBUFFERDRAWBUFFER)(GLuint, GLenum);
		PFNGLNAMEDFRAMEBUFFERDRAWBUFFER glNamedFramebufferDrawBuffer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLNAMEDFRAMEBUFFERDRAWBUFFERS)(GLuint, GLsizei, const GLenum *);
		PFNGLNAMEDFRAMEBUFFERDRAWBUFFERS glNamedFramebufferDrawBuffers = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLNAMEDFRAMEBUFFERPARAMETERI)(GLuint, GLenum, GLint);
		PFNGLNAMEDFRAMEBUFFERPARAMETERI glNamedFramebufferParameteri = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLNAMEDFRAMEBUFFERREADBUFFER)(GLuint, GLenum);
		PFNGLNAMEDFRAMEBUFFERREADBUFFER glNamedFramebufferReadBuffer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLNAMEDFRAMEBUFFERRENDERBUFFER)(GLuint, GLenum, GLenum, GLuint);
		PFNGLNAMEDFRAMEBUFFERRENDERBUFFER glNamedFramebufferRenderbuffer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLNAMEDFRAMEBUFFERTEXTURE)(GLuint, GLenum, GLuint, GLint);
		PFNGLNAMEDFRAMEBUFFERTEXTURE glNamedFramebufferTexture = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLNAMEDFRAMEBUFFERTEXTURELAYER)(GLuint, GLenum, GLuint, GLint, GLint);
		PFNGLNAMEDFRAMEBUFFERTEXTURELAYER glNamedFramebufferTextureLayer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLNAMEDRENDERBUFFERSTORAGE)(GLuint, GLenum, GLsizei, GLsizei);
		PFNGLNAMEDRENDERBUFFERSTORAGE glNamedRenderbufferStorage = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLE)(GLuint, GLsizei, GLenum, GLsizei, GLsizei);
		PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLE glNamedRenderbufferStorageMultisample = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTUREBUFFER)(GLuint, GLenum, GLuint);
		PFNGLTEXTUREBUFFER glTextureBuffer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTUREBUFFERRANGE)(GLuint, GLenum, GLuint, GLintptr, GLsizeiptr);
		PFNGLTEXTUREBUFFERRANGE glTextureBufferRange = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTUREPARAMETERIIV)(GLuint, GLenum, const GLint *);
		PFNGLTEXTUREPARAMETERIIV glTextureParameterIiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTUREPARAMETERIUIV)(GLuint, GLenum, const GLuint *);
		PFNGLTEXTUREPARAMETERIUIV glTextureParameterIuiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTUREPARAMETERF)(GLuint, GLenum, GLfloat);
		PFNGLTEXTUREPARAMETERF glTextureParameterf = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTUREPARAMETERFV)(GLuint, GLenum, const GLfloat *);
		PFNGLTEXTUREPARAMETERFV glTextureParameterfv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTUREPARAMETERI)(GLuint, GLenum, GLint);
		PFNGLTEXTUREPARAMETERI glTextureParameteri = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTUREPARAMETERIV)(GLuint, GLenum, const GLint *);
		PFNGLTEXTUREPARAMETERIV glTextureParameteriv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTURESTORAGE1D)(GLuint, GLsizei, GLenum, GLsizei);
		PFNGLTEXTURESTORAGE1D glTextureStorage1D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTURESTORAGE2D)(GLuint, GLsizei, GLenum, GLsizei, GLsizei);
		PFNGLTEXTURESTORAGE2D glTextureStorage2D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTURESTORAGE2DMULTISAMPLE)(GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLboolean);
		PFNGLTEXTURESTORAGE2DMULTISAMPLE glTextureStorage2DMultisample = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTURESTORAGE3D)(GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLsizei);
		PFNGLTEXTURESTORAGE3D glTextureStorage3D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTURESTORAGE3DMULTISAMPLE)(GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean);
		PFNGLTEXTURESTORAGE3DMULTISAMPLE glTextureStorage3DMultisample = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTURESUBIMAGE1D)(GLuint, GLint, GLint, GLsizei, GLenum, GLenum, const void *);
		PFNGLTEXTURESUBIMAGE1D glTextureSubImage1D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTURESUBIMAGE2D)(GLuint, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void *);
		PFNGLTEXTURESUBIMAGE2D glTextureSubImage2D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTURESUBIMAGE3D)(GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void *);
		PFNGLTEXTURESUBIMAGE3D glTextureSubImage3D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTRANSFORMFEEDBACKBUFFERBASE)(GLuint, GLuint, GLuint);
		PFNGLTRANSFORMFEEDBACKBUFFERBASE glTransformFeedbackBufferBase = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTRANSFORMFEEDBACKBUFFERRANGE)(GLuint, GLuint, GLuint, GLintptr, GLsizeiptr);
		PFNGLTRANSFORMFEEDBACKBUFFERRANGE glTransformFeedbackBufferRange = 0;
		typedef GLboolean (CODEGEN_FUNCPTR *PFNGLUNMAPNAMEDBUFFER)(GLuint);
		PFNGLUNMAPNAMEDBUFFER glUnmapNamedBuffer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXARRAYATTRIBBINDING)(GLuint, GLuint, GLuint);
		PFNGLVERTEXARRAYATTRIBBINDING glVertexArrayAttribBinding = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXARRAYATTRIBFORMAT)(GLuint, GLuint, GLint, GLenum, GLboolean, GLuint);
		PFNGLVERTEXARRAYATTRIBFORMAT glVertexArrayAttribFormat = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXARRAYATTRIBIFORMAT)(GLuint, GLuint, GLint, GLenum, GLuint);
		PFNGLVERTEXARRAYATTRIBIFORMAT glVertexArrayAttribIFormat = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXARRAYATTRIBLFORMAT)(GLuint, GLuint, GLint, GLenum, GLuint);
		PFNGLVERTEXARRAYATTRIBLFORMAT glVertexArrayAttribLFormat = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXARRAYBINDINGDIVISOR)(GLuint, GLuint, GLuint);
		PFNGLVERTEXARRAYBINDINGDIVISOR glVertexArrayBindingDivisor = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXARRAYELEMENTBUFFER)(GLuint, GLuint);
		PFNGLVERTEXARRAYELEMENTBUFFER glVertexArrayElementBuffer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXARRAYVERTEXBUFFER)(GLuint, GLuint, GLuint, GLintptr, GLsizei);
		PFNGLVERTEXARRAYVERTEXBUFFER glVertexArrayVertexBuffer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXARRAYVERTEXBUFFERS)(GLuint, GLuint, GLsizei, const GLuint *, const GLintptr *, const GLsizei *);
		PFNGLVERTEXARRAYVERTEXBUFFERS glVertexArrayVertexBuffers = 0;
		
		static int Load_ARB_direct_state_access()
		{
			int numFailed = 0;
			glBindTextureUnit = reinterpret_cast<PFNGLBINDTEXTUREUNIT>(IntGetProcAddress("glBindTextureUnit"));
			if(!glBindTextureUnit) ++numFailed;
			glBlitNamedFramebuffer = reinterpret_cast<PFNGLBLITNAMEDFRAMEBUFFER>(IntGetProcAddress("glBlitNamedFramebuffer"));
			if(!glBlitNamedFramebuffer) ++numFailed;
			glCheckNamedFramebufferStatus = reinterpret_cast<PFNGLCHECKNAMEDFRAMEBUFFERSTATUS>(IntGetProcAddress("glCheckNamedFramebufferStatus"));
			if(!glCheckNamedFramebufferStatus) ++numFailed;
			glClearNamedBufferData = reinterpret_cast<PFNGLCLEARNAMEDBUFFERDATA>(IntGetProcAddress("glClearNamedBufferData"));
			if(!glClearNamedBufferData) ++numFailed;
			glClearNamedBufferSubData = reinterpret_cast<PFNGLCLEARNAMEDBUFFERSUBDATA>(IntGetProcAddress("glClearNamedBufferSubData"));
			if(!glClearNamedBufferSubData) ++numFailed;
			glClearNamedFramebufferfi = reinterpret_cast<PFNGLCLEARNAMEDFRAMEBUFFERFI>(IntGetProcAddress("glClearNamedFramebufferfi"));
			if(!glClearNamedFramebufferfi) ++numFailed;
			glClearNamedFramebufferfv = reinterpret_cast<PFNGLCLEARNAMEDFRAMEBUFFERFV>(IntGetProcAddress("glClearNamedFramebufferfv"));
			if(!glClearNamedFramebufferfv) ++numFailed;
			glClearNamedFramebufferiv = reinterpret_cast<PFNGLCLEARNAMEDFRAMEBUFFERIV>(IntGetProcAddress("glClearNamedFramebufferiv"));
			if(!glClearNamedFramebufferiv) ++numFailed;
			glClearNamedFramebufferuiv = reinterpret_cast<PFNGLCLEARNAMEDFRAMEBUFFERUIV>(IntGetProcAddress("glClearNamedFramebufferuiv"));
			if(!glClearNamedFramebufferuiv) ++numFailed;
			glCompressedTextureSubImage1D = reinterpret_cast<PFNGLCOMPRESSEDTEXTURESUBIMAGE1D>(IntGetProcAddress("glCompressedTextureSubImage1D"));
			if(!glCompressedTextureSubImage1D) ++numFailed;
			glCompressedTextureSubImage2D = reinterpret_cast<PFNGLCOMPRESSEDTEXTURESUBIMAGE2D>(IntGetProcAddress("glCompressedTextureSubImage2D"));
			if(!glCompressedTextureSubImage2D) ++numFailed;
			glCompressedTextureSubImage3D = reinterpret_cast<PFNGLCOMPRESSEDTEXTURESUBIMAGE3D>(IntGetProcAddress("glCompressedTextureSubImage3D"));
			if(!glCompressedTextureSubImage3D) ++numFailed;
			glCopyNamedBufferSubData = reinterpret_cast<PFNGLCOPYNAMEDBUFFERSUBDATA>(IntGetProcAddress("glCopyNamedBufferSubData"));
			if(!glCopyNamedBufferSubData) ++numFailed;
			glCopyTextureSubImage1D = reinterpret_cast<PFNGLCOPYTEXTURESUBIMAGE1D>(IntGetProcAddress("glCopyTextureSubImage1D"));
			if(!glCopyTextureSubImage1D) ++numFailed;
			glCopyTextureSubImage2D = reinterpret_cast<PFNGLCOPYTEXTURESUBIMAGE2D>(IntGetProcAddress("glCopyTextureSubImage2D"));
			if(!glCopyTextureSubImage2D) ++numFailed;
			glCopyTextureSubImage3D = reinterpret_cast<PFNGLCOPYTEXTURESUBIMAGE3D>(IntGetProcAddress("glCopyTextureSubImage3D"));
			if(!glCopyTextureSubImage3D) ++numFailed;
			glCreateBuffers = reinterpret_cast<PFNGLCREATEBUFFERS>(IntGetProcAddress("glCreateBuffers"));
			if(!glCreateBuffers) ++numFailed;
			glCreateFramebuffers = reinterpret_cast<PFNGLCREATEFRAMEBUFFERS>(IntGetProcAddress("glCreateFramebuffers"));
			if(!glCreateFramebuffers) ++numFailed;
			glCreateProgramPipelines = reinterpret_cast<PFNGLCREATEPROGRAMPIPELINES>(IntGetProcAddress("glCreateProgramPipelines"));
			if(!glCreateProgramPipelines) ++numFailed;
			glCreateQueries = reinterpret_cast<PFNGLCREATEQUERIES>(IntGetProcAddress("glCreateQueries"));
			if(!glCreateQueries) ++numFailed;
			glCreateRenderbuffers = reinterpret_cast<PFNGLCREATERENDERBUFFERS>(IntGetProcAddress("glCreateRenderbuffers"));
			if(!glCreateRenderbuffers) ++numFailed;
			glCreateSamplers = reinterpret_cast<PFNGLCREATESAMPLERS>(IntGetProcAddress("glCreateSamplers"));
			if(!glCreateSamplers) ++numFailed;
			glCreateTextures = reinterpret_cast<PFNGLCREATETEXTURES>(IntGetProcAddress("glCreateTextures"));
			if(!glCreateTextures) ++numFailed;
			glCreateTransformFeedbacks = reinterpret_cast<PFNGLCREATETRANSFORMFEEDBACKS>(IntGetProcAddress("glCreateTransformFeedbacks"));
			if(!glCreateTransformFeedbacks) ++numFailed;
			glCreateVertexArrays = reinterpret_cast<PFNGLCREATEVERTEXARRAYS>(IntGetProcAddress("glCreateVertexArrays"));
			if(!glCreateVertexArrays) ++numFailed;
			glDisableVertexArrayAttrib = reinterpret_cast<PFNGLDISABLEVERTEXARRAYATTRIB>(IntGetProcAddress("glDisableVertexArrayAttrib"));
			if(!glDisableVertexArrayAttrib) ++numFailed;
			glEnableVertexArrayAttrib = reinterpret_cast<PFNGLENABLEVERTEXARRAYATTRIB>(IntGetProcAddress("glEnableVertexArrayAttrib"));
			if(!glEnableVertexArrayAttrib) ++numFailed;
			glFlushMappedNamedBufferRange = reinterpret_cast<PFNGLFLUSHMAPPEDNAMEDBUFFERRANGE>(IntGetProcAddress("glFlushMappedNamedBufferRange"));
			if(!glFlushMappedNamedBufferRange) ++numFailed;
			glGenerateTextureMipmap = reinterpret_cast<PFNGLGENERATETEXTUREMIPMAP>(IntGetProcAddress("glGenerateTextureMipmap"));
			if(!glGenerateTextureMipmap) ++numFailed;
			glGetCompressedTextureImage = reinterpret_cast<PFNGLGETCOMPRESSEDTEXTUREIMAGE>(IntGetProcAddress("glGetCompressedTextureImage"));
			if(!glGetCompressedTextureImage) ++numFailed;
			glGetNamedBufferParameteri64v = reinterpret_cast<PFNGLGETNAMEDBUFFERPARAMETERI64V>(IntGetProcAddress("glGetNamedBufferParameteri64v"));
			if(!glGetNamedBufferParameteri64v) ++numFailed;
			glGetNamedBufferParameteriv = reinterpret_cast<PFNGLGETNAMEDBUFFERPARAMETERIV>(IntGetProcAddress("glGetNamedBufferParameteriv"));
			if(!glGetNamedBufferParameteriv) ++numFailed;
			glGetNamedBufferPointerv = reinterpret_cast<PFNGLGETNAMEDBUFFERPOINTERV>(IntGetProcAddress("glGetNamedBufferPointerv"));
			if(!glGetNamedBufferPointerv) ++numFailed;
			glGetNamedBufferSubData = reinterpret_cast<PFNGLGETNAMEDBUFFERSUBDATA>(IntGetProcAddress("glGetNamedBufferSubData"));
			if(!glGetNamedBufferSubData) ++numFailed;
			glGetNamedFramebufferAttachmentParameteriv = reinterpret_cast<PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIV>(IntGetProcAddress("glGetNamedFramebufferAttachmentParameteriv"));
			if(!glGetNamedFramebufferAttachmentParameteriv) ++numFailed;
			glGetNamedFramebufferParameteriv = reinterpret_cast<PFNGLGETNAMEDFRAMEBUFFERPARAMETERIV>(IntGetProcAddress("glGetNamedFramebufferParameteriv"));
			if(!glGetNamedFramebufferParameteriv) ++numFailed;
			glGetNamedRenderbufferParameteriv = reinterpret_cast<PFNGLGETNAMEDRENDERBUFFERPARAMETERIV>(IntGetProcAddress("glGetNamedRenderbufferParameteriv"));
			if(!glGetNamedRenderbufferParameteriv) ++numFailed;
			glGetQueryBufferObjecti64v = reinterpret_cast<PFNGLGETQUERYBUFFEROBJECTI64V>(IntGetProcAddress("glGetQueryBufferObjecti64v"));
			if(!glGetQueryBufferObjecti64v) ++numFailed;
			glGetQueryBufferObjectiv = reinterpret_cast<PFNGLGETQUERYBUFFEROBJECTIV>(IntGetProcAddress("glGetQueryBufferObjectiv"));
			if(!glGetQueryBufferObjectiv) ++numFailed;
			glGetQueryBufferObjectui64v = reinterpret_cast<PFNGLGETQUERYBUFFEROBJECTUI64V>(IntGetProcAddress("glGetQueryBufferObjectui64v"));
			if(!glGetQueryBufferObjectui64v) ++numFailed;
			glGetQueryBufferObjectuiv = reinterpret_cast<PFNGLGETQUERYBUFFEROBJECTUIV>(IntGetProcAddress("glGetQueryBufferObjectuiv"));
			if(!glGetQueryBufferObjectuiv) ++numFailed;
			glGetTextureImage = reinterpret_cast<PFNGLGETTEXTUREIMAGE>(IntGetProcAddress("glGetTextureImage"));
			if(!glGetTextureImage) ++numFailed;
			glGetTextureLevelParameterfv = reinterpret_cast<PFNGLGETTEXTURELEVELPARAMETERFV>(IntGetProcAddress("glGetTextureLevelParameterfv"));
			if(!glGetTextureLevelParameterfv) ++numFailed;
			glGetTextureLevelParameteriv = reinterpret_cast<PFNGLGETTEXTURELEVELPARAMETERIV>(IntGetProcAddress("glGetTextureLevelParameteriv"));
			if(!glGetTextureLevelParameteriv) ++numFailed;
			glGetTextureParameterIiv = reinterpret_cast<PFNGLGETTEXTUREPARAMETERIIV>(IntGetProcAddress("glGetTextureParameterIiv"));
			if(!glGetTextureParameterIiv) ++numFailed;
			glGetTextureParameterIuiv = reinterpret_cast<PFNGLGETTEXTUREPARAMETERIUIV>(IntGetProcAddress("glGetTextureParameterIuiv"));
			if(!glGetTextureParameterIuiv) ++numFailed;
			glGetTextureParameterfv = reinterpret_cast<PFNGLGETTEXTUREPARAMETERFV>(IntGetProcAddress("glGetTextureParameterfv"));
			if(!glGetTextureParameterfv) ++numFailed;
			glGetTextureParameteriv = reinterpret_cast<PFNGLGETTEXTUREPARAMETERIV>(IntGetProcAddress("glGetTextureParameteriv"));
			if(!glGetTextureParameteriv) ++numFailed;
			glGetTransformFeedbacki64_v = reinterpret_cast<PFNGLGETTRANSFORMFEEDBACKI64_V>(IntGetProcAddress("glGetTransformFeedbacki64_v"));
			if(!glGetTransformFeedbacki64_v) ++numFailed;
			glGetTransformFeedbacki_v = reinterpret_cast<PFNGLGETTRANSFORMFEEDBACKI_V>(IntGetProcAddress("glGetTransformFeedbacki_v"));
			if(!glGetTransformFeedbacki_v) ++numFailed;
			glGetTransformFeedbackiv = reinterpret_cast<PFNGLGETTRANSFORMFEEDBACKIV>(IntGetProcAddress("glGetTransformFeedbackiv"));
			if(!glGetTransformFeedbackiv) ++numFailed;
			glGetVertexArrayIndexed64iv = reinterpret_cast<PFNGLGETVERTEXARRAYINDEXED64IV>(IntGetProcAddress("glGetVertexArrayIndexed64iv"));
			if(!glGetVertexArrayIndexed64iv) ++numFailed;
			glGetVertexArrayIndexediv = reinterpret_cast<PFNGLGETVERTEXARRAYINDEXEDIV>(IntGetProcAddress("glGetVertexArrayIndexediv"));
			if(!glGetVertexArrayIndexediv) ++numFailed;
			glGetVertexArrayiv = reinterpret_cast<PFNGLGETVERTEXARRAYIV>(IntGetProcAddress("glGetVertexArrayiv"));
			if(!glGetVertexArrayiv) ++numFailed;
			glInvalidateNamedFramebufferData = reinterpret_cast<PFNGLINVALIDATENAMEDFRAMEBUFFERDATA>(IntGetProcAddress("glInvalidateNamedFramebufferData"));
			if(!glInvalidateNamedFramebufferData) ++numFailed;
			glInvalidateNamedFramebufferSubData = reinterpret_cast<PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATA>(IntGetProcAddress("glInvalidateNamedFramebufferSubData"));
			if(!glInvalidateNamedFramebufferSubData) ++numFailed;
			glMapNamedBuffer = reinterpret_cast<PFNGLMAPNAMEDBUFFER>(IntGetProcAddress("glMapNamedBuffer"));
			if(!glMapNamedBuffer) ++numFailed;
			glMapNamedBufferRange = reinterpret_cast<PFNGLMAPNAMEDBUFFERRANGE>(IntGetProcAddress("glMapNamedBufferRange"));
			if(!glMapNamedBufferRange) ++numFailed;
			glNamedBufferData = reinterpret_cast<PFNGLNAMEDBUFFERDATA>(IntGetProcAddress("glNamedBufferData"));
			if(!glNamedBufferData) ++numFailed;
			glNamedBufferStorage = reinterpret_cast<PFNGLNAMEDBUFFERSTORAGE>(IntGetProcAddress("glNamedBufferStorage"));
			if(!glNamedBufferStorage) ++numFailed;
			glNamedBufferSubData = reinterpret_cast<PFNGLNAMEDBUFFERSUBDATA>(IntGetProcAddress("glNamedBufferSubData"));
			if(!glNamedBufferSubData) ++numFailed;
			glNamedFramebufferDrawBuffer = reinterpret_cast<PFNGLNAMEDFRAMEBUFFERDRAWBUFFER>(IntGetProcAddress("glNamedFramebufferDrawBuffer"));
			if(!glNamedFramebufferDrawBuffer) ++numFailed;
			glNamedFramebufferDrawBuffers = reinterpret_cast<PFNGLNAMEDFRAMEBUFFERDRAWBUFFERS>(IntGetProcAddress("glNamedFramebufferDrawBuffers"));
			if(!glNamedFramebufferDrawBuffers) ++numFailed;
			glNamedFramebufferParameteri = reinterpret_cast<PFNGLNAMEDFRAMEBUFFERPARAMETERI>(IntGetProcAddress("glNamedFramebufferParameteri"));
			if(!glNamedFramebufferParameteri) ++numFailed;
			glNamedFramebufferReadBuffer = reinterpret_cast<PFNGLNAMEDFRAMEBUFFERREADBUFFER>(IntGetProcAddress("glNamedFramebufferReadBuffer"));
			if(!glNamedFramebufferReadBuffer) ++numFailed;
			glNamedFramebufferRenderbuffer = reinterpret_cast<PFNGLNAMEDFRAMEBUFFERRENDERBUFFER>(IntGetProcAddress("glNamedFramebufferRenderbuffer"));
			if(!glNamedFramebufferRenderbuffer) ++numFailed;
			glNamedFramebufferTexture = reinterpret_cast<PFNGLNAMEDFRAMEBUFFERTEXTURE>(IntGetProcAddress("glNamedFramebufferTexture"));
			if(!glNamedFramebufferTexture) ++numFailed;
			glNamedFramebufferTextureLayer = reinterpret_cast<PFNGLNAMEDFRAMEBUFFERTEXTURELAYER>(IntGetProcAddress("glNamedFramebufferTextureLayer"));
			if(!glNamedFramebufferTextureLayer) ++numFailed;
			glNamedRenderbufferStorage = reinterpret_cast<PFNGLNAMEDRENDERBUFFERSTORAGE>(IntGetProcAddress("glNamedRenderbufferStorage"));
			if(!glNamedRenderbufferStorage) ++numFailed;
			glNamedRenderbufferStorageMultisample = reinterpret_cast<PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLE>(IntGetProcAddress("glNamedRenderbufferStorageMultisample"));
			if(!glNamedRenderbufferStorageMultisample) ++numFailed;
			glTextureBuffer = reinterpret_cast<PFNGLTEXTUREBUFFER>(IntGetProcAddress("glTextureBuffer"));
			if(!glTextureBuffer) ++numFailed;
			glTextureBufferRange = reinterpret_cast<PFNGLTEXTUREBUFFERRANGE>(IntGetProcAddress("glTextureBufferRange"));
			if(!glTextureBufferRange) ++numFailed;
			glTextureParameterIiv = reinterpret_cast<PFNGLTEXTUREPARAMETERIIV>(IntGetProcAddress("glTextureParameterIiv"));
			if(!glTextureParameterIiv) ++numFailed;
			glTextureParameterIuiv = reinterpret_cast<PFNGLTEXTUREPARAMETERIUIV>(IntGetProcAddress("glTextureParameterIuiv"));
			if(!glTextureParameterIuiv) ++numFailed;
			glTextureParameterf = reinterpret_cast<PFNGLTEXTUREPARAMETERF>(IntGetProcAddress("glTextureParameterf"));
			if(!glTextureParameterf) ++numFailed;
			glTextureParameterfv = reinterpret_cast<PFNGLTEXTUREPARAMETERFV>(IntGetProcAddress("glTextureParameterfv"));
			if(!glTextureParameterfv) ++numFailed;
			glTextureParameteri = reinterpret_cast<PFNGLTEXTUREPARAMETERI>(IntGetProcAddress("glTextureParameteri"));
			if(!glTextureParameteri) ++numFailed;
			glTextureParameteriv = reinterpret_cast<PFNGLTEXTUREPARAMETERIV>(IntGetProcAddress("glTextureParameteriv"));
			if(!glTextureParameteriv) ++numFailed;
			glTextureStorage1D = reinterpret_cast<PFNGLTEXTURESTORAGE1D>(IntGetProcAddress("glTextureStorage1D"));
			if(!glTextureStorage1D) ++numFailed;
			glTextureStorage2D = reinterpret_cast<PFNGLTEXTURESTORAGE2D>(IntGetProcAddress("glTextureStorage2D"));
			if(!glTextureStorage2D) ++numFailed;
			glTextureStorage2DMultisample = reinterpret_cast<PFNGLTEXTURESTORAGE2DMULTISAMPLE>(IntGetProcAddress("glTextureStorage2DMultisample"));
			if(!glTextureStorage2DMultisample) ++numFailed;
			glTextureStorage3D = reinterpret_cast<PFNGLTEXTURESTORAGE3D>(IntGetProcAddress("glTextureStorage3D"));
			if(!glTextureStorage3D) ++numFailed;
			glTextureStorage3DMultisample = reinterpret_cast<PFNGLTEXTURESTORAGE3DMULTISAMPLE>(IntGetProcAddress("glTextureStorage3DMultisample"));
			if(!glTextureStorage3DMultisample) ++numFailed;
			glTextureSubImage1D = reinterpret_cast<PFNGLTEXTURESUBIMAGE1D>(IntGetProcAddress("glTextureSubImage1D"));
			if(!glTextureSubImage1D) ++numFailed;
			glTextureSubImage2D = reinterpret_cast<PFNGLTEXTURESUBIMAGE2D>(IntGetProcAddress("glTextureSubImage2D"));
			if(!glTextureSubImage2D) ++numFailed;
			glTextureSubImage3D = reinterpret_cast<PFNGLTEXTURESUBIMAGE3D>(IntGetProcAddress("glTextureSubImage3D"));
			if(!glTextureSubImage3D) ++numFailed;
			glTransformFeedbackBufferBase = reinterpret_cast<PFNGLTRANSFORMFEEDBACKBUFFERBASE>(IntGetProcAddress("glTransformFeedbackBufferBase"));
			if(!glTransformFeedbackBufferBase) ++numFailed;
			glTransformFeedbackBufferRange = reinterpret_cast<PFNGLTRANSFORMFEEDBACKBUFFERRANGE>(IntGetProcAddress("glTransformFeedbackBufferRange"));
			if(!glTransformFeedbackBufferRange) ++numFailed;
			glUnmapNamedBuffer = reinterpret_cast<PFNGLUNMAPNAMEDBUFFER>(IntGetProcAddress("glUnmapNamedBuffer"));
			if(!glUnmapNamedBuffer) ++numFailed;
			glVertexArrayAttribBinding = reinterpret_cast<PFNGLVERTEXARRAYATTRIBBINDING>(IntGetProcAddress("glVertexArrayAttribBinding"));
			if(!glVertexArrayAttribBinding) ++numFailed;
			glVertexArrayAttribFormat = reinterpret_cast<PFNGLVERTEXARRAYATTRIBFORMAT>(IntGetProcAddress("glVertexArrayAttribFormat"));
			if(!glVertexArrayAttribFormat) ++numFailed;
			glVertexArrayAttribIFormat = reinterpret_cast<PFNGLVERTEXARRAYATTRIBIFORMAT>(IntGetProcAddress("glVertexArrayAttribIFormat"));
			if(!glVertexArrayAttribIFormat) ++numFailed;
			glVertexArrayAttribLFormat = reinterpret_cast<PFNGLVERTEXARRAYATTRIBLFORMAT>(IntGetProcAddress("glVertexArrayAttribLFormat"));
			if(!glVertexArrayAttribLFormat) ++numFailed;
			glVertexArrayBindingDivisor = reinterpret_cast<PFNGLVERTEXARRAYBINDINGDIVISOR>(IntGetProcAddress("glVertexArrayBindingDivisor"));
			if(!glVertexArrayBindingDivisor) ++numFailed;
			glVertexArrayElementBuffer = reinterpret_cast<PFNGLVERTEXARRAYELEMENTBUFFER>(IntGetProcAddress("glVertexArrayElementBuffer"));
			if(!glVertexArrayElementBuffer) ++numFailed;
			glVertexArrayVertexBuffer = reinterpret_cast<PFNGLVERTEXARRAYVERTEXBUFFER>(IntGetProcAddress("glVertexArrayVertexBuffer"));
			if(!glVertexArrayVertexBuffer) ++numFailed;
			glVertexArrayVertexBuffers = reinterpret_cast<PFNGLVERTEXARRAYVERTEXBUFFERS>(IntGetProcAddress("glVertexArrayVertexBuffers"));
			if(!glVertexArrayVertexBuffers) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLGETCOMPRESSEDTEXTURESUBIMAGE)(GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLsizei, void *);
		PFNGLGETCOMPRESSEDTEXTURESUBIMAGE glGetCompressedTextureSubImage = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTEXTURESUBIMAGE)(GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, GLsizei, void *);
		PFNGLGETTEXTURESUBIMAGE glGetTextureSubImage = 0;
		
		static int Load_ARB_get_texture_sub_image()
		{
			int numFailed = 0;
			glGetCompressedTextureSubImage = reinterpret_cast<PFNGLGETCOMPRESSEDTEXTURESUBIMAGE>(IntGetProcAddress("glGetCompressedTextureSubImage"));
			if(!glGetCompressedTextureSubImage) ++numFailed;
			glGetTextureSubImage = reinterpret_cast<PFNGLGETTEXTURESUBIMAGE>(IntGetProcAddress("glGetTextureSubImage"));
			if(!glGetTextureSubImage) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXTUREBARRIER)(void);
		PFNGLTEXTUREBARRIER glTextureBarrier = 0;
		
		static int Load_ARB_texture_barrier()
		{
			int numFailed = 0;
			glTextureBarrier = reinterpret_cast<PFNGLTEXTUREBARRIER>(IntGetProcAddress("glTextureBarrier"));
			if(!glTextureBarrier) ++numFailed;
			return numFailed;
		}
		
		typedef GLenum (CODEGEN_FUNCPTR *PFNGLGETGRAPHICSRESETSTATUS)(void);
		PFNGLGETGRAPHICSRESETSTATUS glGetGraphicsResetStatus = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETNUNIFORMFV)(GLuint, GLint, GLsizei, GLfloat *);
		PFNGLGETNUNIFORMFV glGetnUniformfv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETNUNIFORMIV)(GLuint, GLint, GLsizei, GLint *);
		PFNGLGETNUNIFORMIV glGetnUniformiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETNUNIFORMUIV)(GLuint, GLint, GLsizei, GLuint *);
		PFNGLGETNUNIFORMUIV glGetnUniformuiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLREADNPIXELS)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLsizei, void *);
		PFNGLREADNPIXELS glReadnPixels = 0;
		
		static int Load_KHR_robustness()
		{
			int numFailed = 0;
			glGetGraphicsResetStatus = reinterpret_cast<PFNGLGETGRAPHICSRESETSTATUS>(IntGetProcAddress("glGetGraphicsResetStatus"));
			if(!glGetGraphicsResetStatus) ++numFailed;
			glGetnUniformfv = reinterpret_cast<PFNGLGETNUNIFORMFV>(IntGetProcAddress("glGetnUniformfv"));
			if(!glGetnUniformfv) ++numFailed;
			glGetnUniformiv = reinterpret_cast<PFNGLGETNUNIFORMIV>(IntGetProcAddress("glGetnUniformiv"));
			if(!glGetnUniformiv) ++numFailed;
			glGetnUniformuiv = reinterpret_cast<PFNGLGETNUNIFORMUIV>(IntGetProcAddress("glGetnUniformuiv"));
			if(!glGetnUniformuiv) ++numFailed;
			glReadnPixels = reinterpret_cast<PFNGLREADNPIXELS>(IntGetProcAddress("glReadnPixels"));
			if(!glReadnPixels) ++numFailed;
			return numFailed;
		}
		
		typedef void (CODEGEN_FUNCPTR *PFNGLBLENDFUNC)(GLenum, GLenum);
		PFNGLBLENDFUNC glBlendFunc = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEAR)(GLbitfield);
		PFNGLCLEAR glClear = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARCOLOR)(GLfloat, GLfloat, GLfloat, GLfloat);
		PFNGLCLEARCOLOR glClearColor = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARDEPTH)(GLdouble);
		PFNGLCLEARDEPTH glClearDepth = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARSTENCIL)(GLint);
		PFNGLCLEARSTENCIL glClearStencil = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOLORMASK)(GLboolean, GLboolean, GLboolean, GLboolean);
		PFNGLCOLORMASK glColorMask = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCULLFACE)(GLenum);
		PFNGLCULLFACE glCullFace = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDEPTHFUNC)(GLenum);
		PFNGLDEPTHFUNC glDepthFunc = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDEPTHMASK)(GLboolean);
		PFNGLDEPTHMASK glDepthMask = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDEPTHRANGE)(GLdouble, GLdouble);
		PFNGLDEPTHRANGE glDepthRange = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDISABLE)(GLenum);
		PFNGLDISABLE glDisable = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDRAWBUFFER)(GLenum);
		PFNGLDRAWBUFFER glDrawBuffer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLENABLE)(GLenum);
		PFNGLENABLE glEnable = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLFINISH)(void);
		PFNGLFINISH glFinish = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLFLUSH)(void);
		PFNGLFLUSH glFlush = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLFRONTFACE)(GLenum);
		PFNGLFRONTFACE glFrontFace = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETBOOLEANV)(GLenum, GLboolean *);
		PFNGLGETBOOLEANV glGetBooleanv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETDOUBLEV)(GLenum, GLdouble *);
		PFNGLGETDOUBLEV glGetDoublev = 0;
		typedef GLenum (CODEGEN_FUNCPTR *PFNGLGETERROR)(void);
		PFNGLGETERROR glGetError = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETFLOATV)(GLenum, GLfloat *);
		PFNGLGETFLOATV glGetFloatv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETINTEGERV)(GLenum, GLint *);
		PFNGLGETINTEGERV glGetIntegerv = 0;
		typedef const GLubyte * (CODEGEN_FUNCPTR *PFNGLGETSTRING)(GLenum);
		PFNGLGETSTRING glGetString = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTEXIMAGE)(GLenum, GLint, GLenum, GLenum, void *);
		PFNGLGETTEXIMAGE glGetTexImage = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTEXLEVELPARAMETERFV)(GLenum, GLint, GLenum, GLfloat *);
		PFNGLGETTEXLEVELPARAMETERFV glGetTexLevelParameterfv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTEXLEVELPARAMETERIV)(GLenum, GLint, GLenum, GLint *);
		PFNGLGETTEXLEVELPARAMETERIV glGetTexLevelParameteriv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTEXPARAMETERFV)(GLenum, GLenum, GLfloat *);
		PFNGLGETTEXPARAMETERFV glGetTexParameterfv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTEXPARAMETERIV)(GLenum, GLenum, GLint *);
		PFNGLGETTEXPARAMETERIV glGetTexParameteriv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLHINT)(GLenum, GLenum);
		PFNGLHINT glHint = 0;
		typedef GLboolean (CODEGEN_FUNCPTR *PFNGLISENABLED)(GLenum);
		PFNGLISENABLED glIsEnabled = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLLINEWIDTH)(GLfloat);
		PFNGLLINEWIDTH glLineWidth = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLLOGICOP)(GLenum);
		PFNGLLOGICOP glLogicOp = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPIXELSTOREF)(GLenum, GLfloat);
		PFNGLPIXELSTOREF glPixelStoref = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPIXELSTOREI)(GLenum, GLint);
		PFNGLPIXELSTOREI glPixelStorei = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPOINTSIZE)(GLfloat);
		PFNGLPOINTSIZE glPointSize = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPOLYGONMODE)(GLenum, GLenum);
		PFNGLPOLYGONMODE glPolygonMode = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLREADBUFFER)(GLenum);
		PFNGLREADBUFFER glReadBuffer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLREADPIXELS)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void *);
		PFNGLREADPIXELS glReadPixels = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSCISSOR)(GLint, GLint, GLsizei, GLsizei);
		PFNGLSCISSOR glScissor = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSTENCILFUNC)(GLenum, GLint, GLuint);
		PFNGLSTENCILFUNC glStencilFunc = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSTENCILMASK)(GLuint);
		PFNGLSTENCILMASK glStencilMask = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSTENCILOP)(GLenum, GLenum, GLenum);
		PFNGLSTENCILOP glStencilOp = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXIMAGE1D)(GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const void *);
		PFNGLTEXIMAGE1D glTexImage1D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXIMAGE2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *);
		PFNGLTEXIMAGE2D glTexImage2D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXPARAMETERF)(GLenum, GLenum, GLfloat);
		PFNGLTEXPARAMETERF glTexParameterf = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXPARAMETERFV)(GLenum, GLenum, const GLfloat *);
		PFNGLTEXPARAMETERFV glTexParameterfv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXPARAMETERI)(GLenum, GLenum, GLint);
		PFNGLTEXPARAMETERI glTexParameteri = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXPARAMETERIV)(GLenum, GLenum, const GLint *);
		PFNGLTEXPARAMETERIV glTexParameteriv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVIEWPORT)(GLint, GLint, GLsizei, GLsizei);
		PFNGLVIEWPORT glViewport = 0;
		
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDTEXTURE)(GLenum, GLuint);
		PFNGLBINDTEXTURE glBindTexture = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOPYTEXIMAGE1D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint);
		PFNGLCOPYTEXIMAGE1D glCopyTexImage1D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOPYTEXIMAGE2D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
		PFNGLCOPYTEXIMAGE2D glCopyTexImage2D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOPYTEXSUBIMAGE1D)(GLenum, GLint, GLint, GLint, GLint, GLsizei);
		PFNGLCOPYTEXSUBIMAGE1D glCopyTexSubImage1D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOPYTEXSUBIMAGE2D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
		PFNGLCOPYTEXSUBIMAGE2D glCopyTexSubImage2D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDELETETEXTURES)(GLsizei, const GLuint *);
		PFNGLDELETETEXTURES glDeleteTextures = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDRAWARRAYS)(GLenum, GLint, GLsizei);
		PFNGLDRAWARRAYS glDrawArrays = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDRAWELEMENTS)(GLenum, GLsizei, GLenum, const void *);
		PFNGLDRAWELEMENTS glDrawElements = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGENTEXTURES)(GLsizei, GLuint *);
		PFNGLGENTEXTURES glGenTextures = 0;
		typedef GLboolean (CODEGEN_FUNCPTR *PFNGLISTEXTURE)(GLuint);
		PFNGLISTEXTURE glIsTexture = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPOLYGONOFFSET)(GLfloat, GLfloat);
		PFNGLPOLYGONOFFSET glPolygonOffset = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXSUBIMAGE1D)(GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const void *);
		PFNGLTEXSUBIMAGE1D glTexSubImage1D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXSUBIMAGE2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void *);
		PFNGLTEXSUBIMAGE2D glTexSubImage2D = 0;
		
		typedef void (CODEGEN_FUNCPTR *PFNGLCOPYTEXSUBIMAGE3D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
		PFNGLCOPYTEXSUBIMAGE3D glCopyTexSubImage3D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDRAWRANGEELEMENTS)(GLenum, GLuint, GLuint, GLsizei, GLenum, const void *);
		PFNGLDRAWRANGEELEMENTS glDrawRangeElements = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXIMAGE3D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *);
		PFNGLTEXIMAGE3D glTexImage3D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXSUBIMAGE3D)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void *);
		PFNGLTEXSUBIMAGE3D glTexSubImage3D = 0;
		
		typedef void (CODEGEN_FUNCPTR *PFNGLACTIVETEXTURE)(GLenum);
		PFNGLACTIVETEXTURE glActiveTexture = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOMPRESSEDTEXIMAGE1D)(GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const void *);
		PFNGLCOMPRESSEDTEXIMAGE1D glCompressedTexImage1D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOMPRESSEDTEXIMAGE2D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void *);
		PFNGLCOMPRESSEDTEXIMAGE2D glCompressedTexImage2D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOMPRESSEDTEXIMAGE3D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const void *);
		PFNGLCOMPRESSEDTEXIMAGE3D glCompressedTexImage3D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOMPRESSEDTEXSUBIMAGE1D)(GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const void *);
		PFNGLCOMPRESSEDTEXSUBIMAGE1D glCompressedTexSubImage1D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOMPRESSEDTEXSUBIMAGE2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void *);
		PFNGLCOMPRESSEDTEXSUBIMAGE2D glCompressedTexSubImage2D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOMPRESSEDTEXSUBIMAGE3D)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void *);
		PFNGLCOMPRESSEDTEXSUBIMAGE3D glCompressedTexSubImage3D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETCOMPRESSEDTEXIMAGE)(GLenum, GLint, void *);
		PFNGLGETCOMPRESSEDTEXIMAGE glGetCompressedTexImage = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSAMPLECOVERAGE)(GLfloat, GLboolean);
		PFNGLSAMPLECOVERAGE glSampleCoverage = 0;
		
		typedef void (CODEGEN_FUNCPTR *PFNGLBLENDCOLOR)(GLfloat, GLfloat, GLfloat, GLfloat);
		PFNGLBLENDCOLOR glBlendColor = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBLENDEQUATION)(GLenum);
		PFNGLBLENDEQUATION glBlendEquation = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBLENDFUNCSEPARATE)(GLenum, GLenum, GLenum, GLenum);
		PFNGLBLENDFUNCSEPARATE glBlendFuncSeparate = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLMULTIDRAWARRAYS)(GLenum, const GLint *, const GLsizei *, GLsizei);
		PFNGLMULTIDRAWARRAYS glMultiDrawArrays = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLMULTIDRAWELEMENTS)(GLenum, const GLsizei *, GLenum, const void *const*, GLsizei);
		PFNGLMULTIDRAWELEMENTS glMultiDrawElements = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPOINTPARAMETERF)(GLenum, GLfloat);
		PFNGLPOINTPARAMETERF glPointParameterf = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPOINTPARAMETERFV)(GLenum, const GLfloat *);
		PFNGLPOINTPARAMETERFV glPointParameterfv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPOINTPARAMETERI)(GLenum, GLint);
		PFNGLPOINTPARAMETERI glPointParameteri = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPOINTPARAMETERIV)(GLenum, const GLint *);
		PFNGLPOINTPARAMETERIV glPointParameteriv = 0;
		
		typedef void (CODEGEN_FUNCPTR *PFNGLBEGINQUERY)(GLenum, GLuint);
		PFNGLBEGINQUERY glBeginQuery = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDBUFFER)(GLenum, GLuint);
		PFNGLBINDBUFFER glBindBuffer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBUFFERDATA)(GLenum, GLsizeiptr, const void *, GLenum);
		PFNGLBUFFERDATA glBufferData = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBUFFERSUBDATA)(GLenum, GLintptr, GLsizeiptr, const void *);
		PFNGLBUFFERSUBDATA glBufferSubData = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDELETEBUFFERS)(GLsizei, const GLuint *);
		PFNGLDELETEBUFFERS glDeleteBuffers = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDELETEQUERIES)(GLsizei, const GLuint *);
		PFNGLDELETEQUERIES glDeleteQueries = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLENDQUERY)(GLenum);
		PFNGLENDQUERY glEndQuery = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGENBUFFERS)(GLsizei, GLuint *);
		PFNGLGENBUFFERS glGenBuffers = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGENQUERIES)(GLsizei, GLuint *);
		PFNGLGENQUERIES glGenQueries = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETBUFFERPARAMETERIV)(GLenum, GLenum, GLint *);
		PFNGLGETBUFFERPARAMETERIV glGetBufferParameteriv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETBUFFERPOINTERV)(GLenum, GLenum, void **);
		PFNGLGETBUFFERPOINTERV glGetBufferPointerv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETBUFFERSUBDATA)(GLenum, GLintptr, GLsizeiptr, void *);
		PFNGLGETBUFFERSUBDATA glGetBufferSubData = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETQUERYOBJECTIV)(GLuint, GLenum, GLint *);
		PFNGLGETQUERYOBJECTIV glGetQueryObjectiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETQUERYOBJECTUIV)(GLuint, GLenum, GLuint *);
		PFNGLGETQUERYOBJECTUIV glGetQueryObjectuiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETQUERYIV)(GLenum, GLenum, GLint *);
		PFNGLGETQUERYIV glGetQueryiv = 0;
		typedef GLboolean (CODEGEN_FUNCPTR *PFNGLISBUFFER)(GLuint);
		PFNGLISBUFFER glIsBuffer = 0;
		typedef GLboolean (CODEGEN_FUNCPTR *PFNGLISQUERY)(GLuint);
		PFNGLISQUERY glIsQuery = 0;
		typedef void * (CODEGEN_FUNCPTR *PFNGLMAPBUFFER)(GLenum, GLenum);
		PFNGLMAPBUFFER glMapBuffer = 0;
		typedef GLboolean (CODEGEN_FUNCPTR *PFNGLUNMAPBUFFER)(GLenum);
		PFNGLUNMAPBUFFER glUnmapBuffer = 0;
		
		typedef void (CODEGEN_FUNCPTR *PFNGLATTACHSHADER)(GLuint, GLuint);
		PFNGLATTACHSHADER glAttachShader = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDATTRIBLOCATION)(GLuint, GLuint, const GLchar *);
		PFNGLBINDATTRIBLOCATION glBindAttribLocation = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBLENDEQUATIONSEPARATE)(GLenum, GLenum);
		PFNGLBLENDEQUATIONSEPARATE glBlendEquationSeparate = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOMPILESHADER)(GLuint);
		PFNGLCOMPILESHADER glCompileShader = 0;
		typedef GLuint (CODEGEN_FUNCPTR *PFNGLCREATEPROGRAM)(void);
		PFNGLCREATEPROGRAM glCreateProgram = 0;
		typedef GLuint (CODEGEN_FUNCPTR *PFNGLCREATESHADER)(GLenum);
		PFNGLCREATESHADER glCreateShader = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDELETEPROGRAM)(GLuint);
		PFNGLDELETEPROGRAM glDeleteProgram = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDELETESHADER)(GLuint);
		PFNGLDELETESHADER glDeleteShader = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDETACHSHADER)(GLuint, GLuint);
		PFNGLDETACHSHADER glDetachShader = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDISABLEVERTEXATTRIBARRAY)(GLuint);
		PFNGLDISABLEVERTEXATTRIBARRAY glDisableVertexAttribArray = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDRAWBUFFERS)(GLsizei, const GLenum *);
		PFNGLDRAWBUFFERS glDrawBuffers = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLENABLEVERTEXATTRIBARRAY)(GLuint);
		PFNGLENABLEVERTEXATTRIBARRAY glEnableVertexAttribArray = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETACTIVEATTRIB)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
		PFNGLGETACTIVEATTRIB glGetActiveAttrib = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETACTIVEUNIFORM)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
		PFNGLGETACTIVEUNIFORM glGetActiveUniform = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETATTACHEDSHADERS)(GLuint, GLsizei, GLsizei *, GLuint *);
		PFNGLGETATTACHEDSHADERS glGetAttachedShaders = 0;
		typedef GLint (CODEGEN_FUNCPTR *PFNGLGETATTRIBLOCATION)(GLuint, const GLchar *);
		PFNGLGETATTRIBLOCATION glGetAttribLocation = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETPROGRAMINFOLOG)(GLuint, GLsizei, GLsizei *, GLchar *);
		PFNGLGETPROGRAMINFOLOG glGetProgramInfoLog = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETPROGRAMIV)(GLuint, GLenum, GLint *);
		PFNGLGETPROGRAMIV glGetProgramiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETSHADERINFOLOG)(GLuint, GLsizei, GLsizei *, GLchar *);
		PFNGLGETSHADERINFOLOG glGetShaderInfoLog = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETSHADERSOURCE)(GLuint, GLsizei, GLsizei *, GLchar *);
		PFNGLGETSHADERSOURCE glGetShaderSource = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETSHADERIV)(GLuint, GLenum, GLint *);
		PFNGLGETSHADERIV glGetShaderiv = 0;
		typedef GLint (CODEGEN_FUNCPTR *PFNGLGETUNIFORMLOCATION)(GLuint, const GLchar *);
		PFNGLGETUNIFORMLOCATION glGetUniformLocation = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETUNIFORMFV)(GLuint, GLint, GLfloat *);
		PFNGLGETUNIFORMFV glGetUniformfv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETUNIFORMIV)(GLuint, GLint, GLint *);
		PFNGLGETUNIFORMIV glGetUniformiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETVERTEXATTRIBPOINTERV)(GLuint, GLenum, void **);
		PFNGLGETVERTEXATTRIBPOINTERV glGetVertexAttribPointerv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETVERTEXATTRIBDV)(GLuint, GLenum, GLdouble *);
		PFNGLGETVERTEXATTRIBDV glGetVertexAttribdv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETVERTEXATTRIBFV)(GLuint, GLenum, GLfloat *);
		PFNGLGETVERTEXATTRIBFV glGetVertexAttribfv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETVERTEXATTRIBIV)(GLuint, GLenum, GLint *);
		PFNGLGETVERTEXATTRIBIV glGetVertexAttribiv = 0;
		typedef GLboolean (CODEGEN_FUNCPTR *PFNGLISPROGRAM)(GLuint);
		PFNGLISPROGRAM glIsProgram = 0;
		typedef GLboolean (CODEGEN_FUNCPTR *PFNGLISSHADER)(GLuint);
		PFNGLISSHADER glIsShader = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLLINKPROGRAM)(GLuint);
		PFNGLLINKPROGRAM glLinkProgram = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSHADERSOURCE)(GLuint, GLsizei, const GLchar *const*, const GLint *);
		PFNGLSHADERSOURCE glShaderSource = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSTENCILFUNCSEPARATE)(GLenum, GLenum, GLint, GLuint);
		PFNGLSTENCILFUNCSEPARATE glStencilFuncSeparate = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSTENCILMASKSEPARATE)(GLenum, GLuint);
		PFNGLSTENCILMASKSEPARATE glStencilMaskSeparate = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSTENCILOPSEPARATE)(GLenum, GLenum, GLenum, GLenum);
		PFNGLSTENCILOPSEPARATE glStencilOpSeparate = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM1F)(GLint, GLfloat);
		PFNGLUNIFORM1F glUniform1f = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM1FV)(GLint, GLsizei, const GLfloat *);
		PFNGLUNIFORM1FV glUniform1fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM1I)(GLint, GLint);
		PFNGLUNIFORM1I glUniform1i = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM1IV)(GLint, GLsizei, const GLint *);
		PFNGLUNIFORM1IV glUniform1iv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM2F)(GLint, GLfloat, GLfloat);
		PFNGLUNIFORM2F glUniform2f = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM2FV)(GLint, GLsizei, const GLfloat *);
		PFNGLUNIFORM2FV glUniform2fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM2I)(GLint, GLint, GLint);
		PFNGLUNIFORM2I glUniform2i = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM2IV)(GLint, GLsizei, const GLint *);
		PFNGLUNIFORM2IV glUniform2iv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM3F)(GLint, GLfloat, GLfloat, GLfloat);
		PFNGLUNIFORM3F glUniform3f = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM3FV)(GLint, GLsizei, const GLfloat *);
		PFNGLUNIFORM3FV glUniform3fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM3I)(GLint, GLint, GLint, GLint);
		PFNGLUNIFORM3I glUniform3i = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM3IV)(GLint, GLsizei, const GLint *);
		PFNGLUNIFORM3IV glUniform3iv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM4F)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
		PFNGLUNIFORM4F glUniform4f = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM4FV)(GLint, GLsizei, const GLfloat *);
		PFNGLUNIFORM4FV glUniform4fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM4I)(GLint, GLint, GLint, GLint, GLint);
		PFNGLUNIFORM4I glUniform4i = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM4IV)(GLint, GLsizei, const GLint *);
		PFNGLUNIFORM4IV glUniform4iv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORMMATRIX2FV)(GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLUNIFORMMATRIX2FV glUniformMatrix2fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORMMATRIX3FV)(GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLUNIFORMMATRIX3FV glUniformMatrix3fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORMMATRIX4FV)(GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLUNIFORMMATRIX4FV glUniformMatrix4fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUSEPROGRAM)(GLuint);
		PFNGLUSEPROGRAM glUseProgram = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVALIDATEPROGRAM)(GLuint);
		PFNGLVALIDATEPROGRAM glValidateProgram = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB1D)(GLuint, GLdouble);
		PFNGLVERTEXATTRIB1D glVertexAttrib1d = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB1DV)(GLuint, const GLdouble *);
		PFNGLVERTEXATTRIB1DV glVertexAttrib1dv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB1F)(GLuint, GLfloat);
		PFNGLVERTEXATTRIB1F glVertexAttrib1f = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB1FV)(GLuint, const GLfloat *);
		PFNGLVERTEXATTRIB1FV glVertexAttrib1fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB1S)(GLuint, GLshort);
		PFNGLVERTEXATTRIB1S glVertexAttrib1s = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB1SV)(GLuint, const GLshort *);
		PFNGLVERTEXATTRIB1SV glVertexAttrib1sv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB2D)(GLuint, GLdouble, GLdouble);
		PFNGLVERTEXATTRIB2D glVertexAttrib2d = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB2DV)(GLuint, const GLdouble *);
		PFNGLVERTEXATTRIB2DV glVertexAttrib2dv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB2F)(GLuint, GLfloat, GLfloat);
		PFNGLVERTEXATTRIB2F glVertexAttrib2f = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB2FV)(GLuint, const GLfloat *);
		PFNGLVERTEXATTRIB2FV glVertexAttrib2fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB2S)(GLuint, GLshort, GLshort);
		PFNGLVERTEXATTRIB2S glVertexAttrib2s = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB2SV)(GLuint, const GLshort *);
		PFNGLVERTEXATTRIB2SV glVertexAttrib2sv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB3D)(GLuint, GLdouble, GLdouble, GLdouble);
		PFNGLVERTEXATTRIB3D glVertexAttrib3d = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB3DV)(GLuint, const GLdouble *);
		PFNGLVERTEXATTRIB3DV glVertexAttrib3dv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB3F)(GLuint, GLfloat, GLfloat, GLfloat);
		PFNGLVERTEXATTRIB3F glVertexAttrib3f = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB3FV)(GLuint, const GLfloat *);
		PFNGLVERTEXATTRIB3FV glVertexAttrib3fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB3S)(GLuint, GLshort, GLshort, GLshort);
		PFNGLVERTEXATTRIB3S glVertexAttrib3s = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB3SV)(GLuint, const GLshort *);
		PFNGLVERTEXATTRIB3SV glVertexAttrib3sv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4NBV)(GLuint, const GLbyte *);
		PFNGLVERTEXATTRIB4NBV glVertexAttrib4Nbv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4NIV)(GLuint, const GLint *);
		PFNGLVERTEXATTRIB4NIV glVertexAttrib4Niv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4NSV)(GLuint, const GLshort *);
		PFNGLVERTEXATTRIB4NSV glVertexAttrib4Nsv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4NUB)(GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
		PFNGLVERTEXATTRIB4NUB glVertexAttrib4Nub = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4NUBV)(GLuint, const GLubyte *);
		PFNGLVERTEXATTRIB4NUBV glVertexAttrib4Nubv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4NUIV)(GLuint, const GLuint *);
		PFNGLVERTEXATTRIB4NUIV glVertexAttrib4Nuiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4NUSV)(GLuint, const GLushort *);
		PFNGLVERTEXATTRIB4NUSV glVertexAttrib4Nusv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4BV)(GLuint, const GLbyte *);
		PFNGLVERTEXATTRIB4BV glVertexAttrib4bv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4D)(GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
		PFNGLVERTEXATTRIB4D glVertexAttrib4d = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4DV)(GLuint, const GLdouble *);
		PFNGLVERTEXATTRIB4DV glVertexAttrib4dv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4F)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
		PFNGLVERTEXATTRIB4F glVertexAttrib4f = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4FV)(GLuint, const GLfloat *);
		PFNGLVERTEXATTRIB4FV glVertexAttrib4fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4IV)(GLuint, const GLint *);
		PFNGLVERTEXATTRIB4IV glVertexAttrib4iv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4S)(GLuint, GLshort, GLshort, GLshort, GLshort);
		PFNGLVERTEXATTRIB4S glVertexAttrib4s = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4SV)(GLuint, const GLshort *);
		PFNGLVERTEXATTRIB4SV glVertexAttrib4sv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4UBV)(GLuint, const GLubyte *);
		PFNGLVERTEXATTRIB4UBV glVertexAttrib4ubv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4UIV)(GLuint, const GLuint *);
		PFNGLVERTEXATTRIB4UIV glVertexAttrib4uiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIB4USV)(GLuint, const GLushort *);
		PFNGLVERTEXATTRIB4USV glVertexAttrib4usv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBPOINTER)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void *);
		PFNGLVERTEXATTRIBPOINTER glVertexAttribPointer = 0;
		
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORMMATRIX2X3FV)(GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLUNIFORMMATRIX2X3FV glUniformMatrix2x3fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORMMATRIX2X4FV)(GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLUNIFORMMATRIX2X4FV glUniformMatrix2x4fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORMMATRIX3X2FV)(GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLUNIFORMMATRIX3X2FV glUniformMatrix3x2fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORMMATRIX3X4FV)(GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLUNIFORMMATRIX3X4FV glUniformMatrix3x4fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORMMATRIX4X2FV)(GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLUNIFORMMATRIX4X2FV glUniformMatrix4x2fv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORMMATRIX4X3FV)(GLint, GLsizei, GLboolean, const GLfloat *);
		PFNGLUNIFORMMATRIX4X3FV glUniformMatrix4x3fv = 0;
		
		typedef void (CODEGEN_FUNCPTR *PFNGLBEGINCONDITIONALRENDER)(GLuint, GLenum);
		PFNGLBEGINCONDITIONALRENDER glBeginConditionalRender = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBEGINTRANSFORMFEEDBACK)(GLenum);
		PFNGLBEGINTRANSFORMFEEDBACK glBeginTransformFeedback = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDBUFFERBASE)(GLenum, GLuint, GLuint);
		PFNGLBINDBUFFERBASE glBindBufferBase = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDBUFFERRANGE)(GLenum, GLuint, GLuint, GLintptr, GLsizeiptr);
		PFNGLBINDBUFFERRANGE glBindBufferRange = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDFRAGDATALOCATION)(GLuint, GLuint, const GLchar *);
		PFNGLBINDFRAGDATALOCATION glBindFragDataLocation = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDFRAMEBUFFER)(GLenum, GLuint);
		PFNGLBINDFRAMEBUFFER glBindFramebuffer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDRENDERBUFFER)(GLenum, GLuint);
		PFNGLBINDRENDERBUFFER glBindRenderbuffer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDVERTEXARRAY)(GLuint);
		PFNGLBINDVERTEXARRAY glBindVertexArray = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBLITFRAMEBUFFER)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
		PFNGLBLITFRAMEBUFFER glBlitFramebuffer = 0;
		typedef GLenum (CODEGEN_FUNCPTR *PFNGLCHECKFRAMEBUFFERSTATUS)(GLenum);
		PFNGLCHECKFRAMEBUFFERSTATUS glCheckFramebufferStatus = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCLAMPCOLOR)(GLenum, GLenum);
		PFNGLCLAMPCOLOR glClampColor = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARBUFFERFI)(GLenum, GLint, GLfloat, GLint);
		PFNGLCLEARBUFFERFI glClearBufferfi = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARBUFFERFV)(GLenum, GLint, const GLfloat *);
		PFNGLCLEARBUFFERFV glClearBufferfv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARBUFFERIV)(GLenum, GLint, const GLint *);
		PFNGLCLEARBUFFERIV glClearBufferiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCLEARBUFFERUIV)(GLenum, GLint, const GLuint *);
		PFNGLCLEARBUFFERUIV glClearBufferuiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLCOLORMASKI)(GLuint, GLboolean, GLboolean, GLboolean, GLboolean);
		PFNGLCOLORMASKI glColorMaski = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDELETEFRAMEBUFFERS)(GLsizei, const GLuint *);
		PFNGLDELETEFRAMEBUFFERS glDeleteFramebuffers = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDELETERENDERBUFFERS)(GLsizei, const GLuint *);
		PFNGLDELETERENDERBUFFERS glDeleteRenderbuffers = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDELETEVERTEXARRAYS)(GLsizei, const GLuint *);
		PFNGLDELETEVERTEXARRAYS glDeleteVertexArrays = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDISABLEI)(GLenum, GLuint);
		PFNGLDISABLEI glDisablei = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLENABLEI)(GLenum, GLuint);
		PFNGLENABLEI glEnablei = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLENDCONDITIONALRENDER)(void);
		PFNGLENDCONDITIONALRENDER glEndConditionalRender = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLENDTRANSFORMFEEDBACK)(void);
		PFNGLENDTRANSFORMFEEDBACK glEndTransformFeedback = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLFLUSHMAPPEDBUFFERRANGE)(GLenum, GLintptr, GLsizeiptr);
		PFNGLFLUSHMAPPEDBUFFERRANGE glFlushMappedBufferRange = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLFRAMEBUFFERRENDERBUFFER)(GLenum, GLenum, GLenum, GLuint);
		PFNGLFRAMEBUFFERRENDERBUFFER glFramebufferRenderbuffer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLFRAMEBUFFERTEXTURE1D)(GLenum, GLenum, GLenum, GLuint, GLint);
		PFNGLFRAMEBUFFERTEXTURE1D glFramebufferTexture1D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLFRAMEBUFFERTEXTURE2D)(GLenum, GLenum, GLenum, GLuint, GLint);
		PFNGLFRAMEBUFFERTEXTURE2D glFramebufferTexture2D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLFRAMEBUFFERTEXTURE3D)(GLenum, GLenum, GLenum, GLuint, GLint, GLint);
		PFNGLFRAMEBUFFERTEXTURE3D glFramebufferTexture3D = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLFRAMEBUFFERTEXTURELAYER)(GLenum, GLenum, GLuint, GLint, GLint);
		PFNGLFRAMEBUFFERTEXTURELAYER glFramebufferTextureLayer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGENFRAMEBUFFERS)(GLsizei, GLuint *);
		PFNGLGENFRAMEBUFFERS glGenFramebuffers = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGENRENDERBUFFERS)(GLsizei, GLuint *);
		PFNGLGENRENDERBUFFERS glGenRenderbuffers = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGENVERTEXARRAYS)(GLsizei, GLuint *);
		PFNGLGENVERTEXARRAYS glGenVertexArrays = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGENERATEMIPMAP)(GLenum);
		PFNGLGENERATEMIPMAP glGenerateMipmap = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETBOOLEANI_V)(GLenum, GLuint, GLboolean *);
		PFNGLGETBOOLEANI_V glGetBooleani_v = 0;
		typedef GLint (CODEGEN_FUNCPTR *PFNGLGETFRAGDATALOCATION)(GLuint, const GLchar *);
		PFNGLGETFRAGDATALOCATION glGetFragDataLocation = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIV)(GLenum, GLenum, GLenum, GLint *);
		PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIV glGetFramebufferAttachmentParameteriv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETINTEGERI_V)(GLenum, GLuint, GLint *);
		PFNGLGETINTEGERI_V glGetIntegeri_v = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETRENDERBUFFERPARAMETERIV)(GLenum, GLenum, GLint *);
		PFNGLGETRENDERBUFFERPARAMETERIV glGetRenderbufferParameteriv = 0;
		typedef const GLubyte * (CODEGEN_FUNCPTR *PFNGLGETSTRINGI)(GLenum, GLuint);
		PFNGLGETSTRINGI glGetStringi = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTEXPARAMETERIIV)(GLenum, GLenum, GLint *);
		PFNGLGETTEXPARAMETERIIV glGetTexParameterIiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTEXPARAMETERIUIV)(GLenum, GLenum, GLuint *);
		PFNGLGETTEXPARAMETERIUIV glGetTexParameterIuiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETTRANSFORMFEEDBACKVARYING)(GLuint, GLuint, GLsizei, GLsizei *, GLsizei *, GLenum *, GLchar *);
		PFNGLGETTRANSFORMFEEDBACKVARYING glGetTransformFeedbackVarying = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETUNIFORMUIV)(GLuint, GLint, GLuint *);
		PFNGLGETUNIFORMUIV glGetUniformuiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETVERTEXATTRIBIIV)(GLuint, GLenum, GLint *);
		PFNGLGETVERTEXATTRIBIIV glGetVertexAttribIiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETVERTEXATTRIBIUIV)(GLuint, GLenum, GLuint *);
		PFNGLGETVERTEXATTRIBIUIV glGetVertexAttribIuiv = 0;
		typedef GLboolean (CODEGEN_FUNCPTR *PFNGLISENABLEDI)(GLenum, GLuint);
		PFNGLISENABLEDI glIsEnabledi = 0;
		typedef GLboolean (CODEGEN_FUNCPTR *PFNGLISFRAMEBUFFER)(GLuint);
		PFNGLISFRAMEBUFFER glIsFramebuffer = 0;
		typedef GLboolean (CODEGEN_FUNCPTR *PFNGLISRENDERBUFFER)(GLuint);
		PFNGLISRENDERBUFFER glIsRenderbuffer = 0;
		typedef GLboolean (CODEGEN_FUNCPTR *PFNGLISVERTEXARRAY)(GLuint);
		PFNGLISVERTEXARRAY glIsVertexArray = 0;
		typedef void * (CODEGEN_FUNCPTR *PFNGLMAPBUFFERRANGE)(GLenum, GLintptr, GLsizeiptr, GLbitfield);
		PFNGLMAPBUFFERRANGE glMapBufferRange = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLRENDERBUFFERSTORAGE)(GLenum, GLenum, GLsizei, GLsizei);
		PFNGLRENDERBUFFERSTORAGE glRenderbufferStorage = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLRENDERBUFFERSTORAGEMULTISAMPLE)(GLenum, GLsizei, GLenum, GLsizei, GLsizei);
		PFNGLRENDERBUFFERSTORAGEMULTISAMPLE glRenderbufferStorageMultisample = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXPARAMETERIIV)(GLenum, GLenum, const GLint *);
		PFNGLTEXPARAMETERIIV glTexParameterIiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXPARAMETERIUIV)(GLenum, GLenum, const GLuint *);
		PFNGLTEXPARAMETERIUIV glTexParameterIuiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTRANSFORMFEEDBACKVARYINGS)(GLuint, GLsizei, const GLchar *const*, GLenum);
		PFNGLTRANSFORMFEEDBACKVARYINGS glTransformFeedbackVaryings = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM1UI)(GLint, GLuint);
		PFNGLUNIFORM1UI glUniform1ui = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM1UIV)(GLint, GLsizei, const GLuint *);
		PFNGLUNIFORM1UIV glUniform1uiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM2UI)(GLint, GLuint, GLuint);
		PFNGLUNIFORM2UI glUniform2ui = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM2UIV)(GLint, GLsizei, const GLuint *);
		PFNGLUNIFORM2UIV glUniform2uiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM3UI)(GLint, GLuint, GLuint, GLuint);
		PFNGLUNIFORM3UI glUniform3ui = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM3UIV)(GLint, GLsizei, const GLuint *);
		PFNGLUNIFORM3UIV glUniform3uiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM4UI)(GLint, GLuint, GLuint, GLuint, GLuint);
		PFNGLUNIFORM4UI glUniform4ui = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORM4UIV)(GLint, GLsizei, const GLuint *);
		PFNGLUNIFORM4UIV glUniform4uiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI1I)(GLuint, GLint);
		PFNGLVERTEXATTRIBI1I glVertexAttribI1i = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI1IV)(GLuint, const GLint *);
		PFNGLVERTEXATTRIBI1IV glVertexAttribI1iv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI1UI)(GLuint, GLuint);
		PFNGLVERTEXATTRIBI1UI glVertexAttribI1ui = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI1UIV)(GLuint, const GLuint *);
		PFNGLVERTEXATTRIBI1UIV glVertexAttribI1uiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI2I)(GLuint, GLint, GLint);
		PFNGLVERTEXATTRIBI2I glVertexAttribI2i = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI2IV)(GLuint, const GLint *);
		PFNGLVERTEXATTRIBI2IV glVertexAttribI2iv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI2UI)(GLuint, GLuint, GLuint);
		PFNGLVERTEXATTRIBI2UI glVertexAttribI2ui = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI2UIV)(GLuint, const GLuint *);
		PFNGLVERTEXATTRIBI2UIV glVertexAttribI2uiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI3I)(GLuint, GLint, GLint, GLint);
		PFNGLVERTEXATTRIBI3I glVertexAttribI3i = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI3IV)(GLuint, const GLint *);
		PFNGLVERTEXATTRIBI3IV glVertexAttribI3iv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI3UI)(GLuint, GLuint, GLuint, GLuint);
		PFNGLVERTEXATTRIBI3UI glVertexAttribI3ui = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI3UIV)(GLuint, const GLuint *);
		PFNGLVERTEXATTRIBI3UIV glVertexAttribI3uiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI4BV)(GLuint, const GLbyte *);
		PFNGLVERTEXATTRIBI4BV glVertexAttribI4bv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI4I)(GLuint, GLint, GLint, GLint, GLint);
		PFNGLVERTEXATTRIBI4I glVertexAttribI4i = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI4IV)(GLuint, const GLint *);
		PFNGLVERTEXATTRIBI4IV glVertexAttribI4iv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI4SV)(GLuint, const GLshort *);
		PFNGLVERTEXATTRIBI4SV glVertexAttribI4sv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI4UBV)(GLuint, const GLubyte *);
		PFNGLVERTEXATTRIBI4UBV glVertexAttribI4ubv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI4UI)(GLuint, GLuint, GLuint, GLuint, GLuint);
		PFNGLVERTEXATTRIBI4UI glVertexAttribI4ui = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI4UIV)(GLuint, const GLuint *);
		PFNGLVERTEXATTRIBI4UIV glVertexAttribI4uiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBI4USV)(GLuint, const GLushort *);
		PFNGLVERTEXATTRIBI4USV glVertexAttribI4usv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBIPOINTER)(GLuint, GLint, GLenum, GLsizei, const void *);
		PFNGLVERTEXATTRIBIPOINTER glVertexAttribIPointer = 0;
		
		typedef void (CODEGEN_FUNCPTR *PFNGLCOPYBUFFERSUBDATA)(GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr);
		PFNGLCOPYBUFFERSUBDATA glCopyBufferSubData = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDRAWARRAYSINSTANCED)(GLenum, GLint, GLsizei, GLsizei);
		PFNGLDRAWARRAYSINSTANCED glDrawArraysInstanced = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDRAWELEMENTSINSTANCED)(GLenum, GLsizei, GLenum, const void *, GLsizei);
		PFNGLDRAWELEMENTSINSTANCED glDrawElementsInstanced = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETACTIVEUNIFORMBLOCKNAME)(GLuint, GLuint, GLsizei, GLsizei *, GLchar *);
		PFNGLGETACTIVEUNIFORMBLOCKNAME glGetActiveUniformBlockName = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETACTIVEUNIFORMBLOCKIV)(GLuint, GLuint, GLenum, GLint *);
		PFNGLGETACTIVEUNIFORMBLOCKIV glGetActiveUniformBlockiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETACTIVEUNIFORMNAME)(GLuint, GLuint, GLsizei, GLsizei *, GLchar *);
		PFNGLGETACTIVEUNIFORMNAME glGetActiveUniformName = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETACTIVEUNIFORMSIV)(GLuint, GLsizei, const GLuint *, GLenum, GLint *);
		PFNGLGETACTIVEUNIFORMSIV glGetActiveUniformsiv = 0;
		typedef GLuint (CODEGEN_FUNCPTR *PFNGLGETUNIFORMBLOCKINDEX)(GLuint, const GLchar *);
		PFNGLGETUNIFORMBLOCKINDEX glGetUniformBlockIndex = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETUNIFORMINDICES)(GLuint, GLsizei, const GLchar *const*, GLuint *);
		PFNGLGETUNIFORMINDICES glGetUniformIndices = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPRIMITIVERESTARTINDEX)(GLuint);
		PFNGLPRIMITIVERESTARTINDEX glPrimitiveRestartIndex = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXBUFFER)(GLenum, GLenum, GLuint);
		PFNGLTEXBUFFER glTexBuffer = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLUNIFORMBLOCKBINDING)(GLuint, GLuint, GLuint);
		PFNGLUNIFORMBLOCKBINDING glUniformBlockBinding = 0;
		
		typedef GLenum (CODEGEN_FUNCPTR *PFNGLCLIENTWAITSYNC)(GLsync, GLbitfield, GLuint64);
		PFNGLCLIENTWAITSYNC glClientWaitSync = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDELETESYNC)(GLsync);
		PFNGLDELETESYNC glDeleteSync = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDRAWELEMENTSBASEVERTEX)(GLenum, GLsizei, GLenum, const void *, GLint);
		PFNGLDRAWELEMENTSBASEVERTEX glDrawElementsBaseVertex = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDRAWELEMENTSINSTANCEDBASEVERTEX)(GLenum, GLsizei, GLenum, const void *, GLsizei, GLint);
		PFNGLDRAWELEMENTSINSTANCEDBASEVERTEX glDrawElementsInstancedBaseVertex = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDRAWRANGEELEMENTSBASEVERTEX)(GLenum, GLuint, GLuint, GLsizei, GLenum, const void *, GLint);
		PFNGLDRAWRANGEELEMENTSBASEVERTEX glDrawRangeElementsBaseVertex = 0;
		typedef GLsync (CODEGEN_FUNCPTR *PFNGLFENCESYNC)(GLenum, GLbitfield);
		PFNGLFENCESYNC glFenceSync = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLFRAMEBUFFERTEXTURE)(GLenum, GLenum, GLuint, GLint);
		PFNGLFRAMEBUFFERTEXTURE glFramebufferTexture = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETBUFFERPARAMETERI64V)(GLenum, GLenum, GLint64 *);
		PFNGLGETBUFFERPARAMETERI64V glGetBufferParameteri64v = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETINTEGER64I_V)(GLenum, GLuint, GLint64 *);
		PFNGLGETINTEGER64I_V glGetInteger64i_v = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETINTEGER64V)(GLenum, GLint64 *);
		PFNGLGETINTEGER64V glGetInteger64v = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETMULTISAMPLEFV)(GLenum, GLuint, GLfloat *);
		PFNGLGETMULTISAMPLEFV glGetMultisamplefv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETSYNCIV)(GLsync, GLenum, GLsizei, GLsizei *, GLint *);
		PFNGLGETSYNCIV glGetSynciv = 0;
		typedef GLboolean (CODEGEN_FUNCPTR *PFNGLISSYNC)(GLsync);
		PFNGLISSYNC glIsSync = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLMULTIDRAWELEMENTSBASEVERTEX)(GLenum, const GLsizei *, GLenum, const void *const*, GLsizei, const GLint *);
		PFNGLMULTIDRAWELEMENTSBASEVERTEX glMultiDrawElementsBaseVertex = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLPROVOKINGVERTEX)(GLenum);
		PFNGLPROVOKINGVERTEX glProvokingVertex = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSAMPLEMASKI)(GLuint, GLbitfield);
		PFNGLSAMPLEMASKI glSampleMaski = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXIMAGE2DMULTISAMPLE)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean);
		PFNGLTEXIMAGE2DMULTISAMPLE glTexImage2DMultisample = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLTEXIMAGE3DMULTISAMPLE)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean);
		PFNGLTEXIMAGE3DMULTISAMPLE glTexImage3DMultisample = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLWAITSYNC)(GLsync, GLbitfield, GLuint64);
		PFNGLWAITSYNC glWaitSync = 0;
		
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDFRAGDATALOCATIONINDEXED)(GLuint, GLuint, GLuint, const GLchar *);
		PFNGLBINDFRAGDATALOCATIONINDEXED glBindFragDataLocationIndexed = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLBINDSAMPLER)(GLuint, GLuint);
		PFNGLBINDSAMPLER glBindSampler = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLDELETESAMPLERS)(GLsizei, const GLuint *);
		PFNGLDELETESAMPLERS glDeleteSamplers = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGENSAMPLERS)(GLsizei, GLuint *);
		PFNGLGENSAMPLERS glGenSamplers = 0;
		typedef GLint (CODEGEN_FUNCPTR *PFNGLGETFRAGDATAINDEX)(GLuint, const GLchar *);
		PFNGLGETFRAGDATAINDEX glGetFragDataIndex = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETQUERYOBJECTI64V)(GLuint, GLenum, GLint64 *);
		PFNGLGETQUERYOBJECTI64V glGetQueryObjecti64v = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETQUERYOBJECTUI64V)(GLuint, GLenum, GLuint64 *);
		PFNGLGETQUERYOBJECTUI64V glGetQueryObjectui64v = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETSAMPLERPARAMETERIIV)(GLuint, GLenum, GLint *);
		PFNGLGETSAMPLERPARAMETERIIV glGetSamplerParameterIiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETSAMPLERPARAMETERIUIV)(GLuint, GLenum, GLuint *);
		PFNGLGETSAMPLERPARAMETERIUIV glGetSamplerParameterIuiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETSAMPLERPARAMETERFV)(GLuint, GLenum, GLfloat *);
		PFNGLGETSAMPLERPARAMETERFV glGetSamplerParameterfv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLGETSAMPLERPARAMETERIV)(GLuint, GLenum, GLint *);
		PFNGLGETSAMPLERPARAMETERIV glGetSamplerParameteriv = 0;
		typedef GLboolean (CODEGEN_FUNCPTR *PFNGLISSAMPLER)(GLuint);
		PFNGLISSAMPLER glIsSampler = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLQUERYCOUNTER)(GLuint, GLenum);
		PFNGLQUERYCOUNTER glQueryCounter = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSAMPLERPARAMETERIIV)(GLuint, GLenum, const GLint *);
		PFNGLSAMPLERPARAMETERIIV glSamplerParameterIiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSAMPLERPARAMETERIUIV)(GLuint, GLenum, const GLuint *);
		PFNGLSAMPLERPARAMETERIUIV glSamplerParameterIuiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSAMPLERPARAMETERF)(GLuint, GLenum, GLfloat);
		PFNGLSAMPLERPARAMETERF glSamplerParameterf = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSAMPLERPARAMETERFV)(GLuint, GLenum, const GLfloat *);
		PFNGLSAMPLERPARAMETERFV glSamplerParameterfv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSAMPLERPARAMETERI)(GLuint, GLenum, GLint);
		PFNGLSAMPLERPARAMETERI glSamplerParameteri = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLSAMPLERPARAMETERIV)(GLuint, GLenum, const GLint *);
		PFNGLSAMPLERPARAMETERIV glSamplerParameteriv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBDIVISOR)(GLuint, GLuint);
		PFNGLVERTEXATTRIBDIVISOR glVertexAttribDivisor = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBP1UI)(GLuint, GLenum, GLboolean, GLuint);
		PFNGLVERTEXATTRIBP1UI glVertexAttribP1ui = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBP1UIV)(GLuint, GLenum, GLboolean, const GLuint *);
		PFNGLVERTEXATTRIBP1UIV glVertexAttribP1uiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBP2UI)(GLuint, GLenum, GLboolean, GLuint);
		PFNGLVERTEXATTRIBP2UI glVertexAttribP2ui = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBP2UIV)(GLuint, GLenum, GLboolean, const GLuint *);
		PFNGLVERTEXATTRIBP2UIV glVertexAttribP2uiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBP3UI)(GLuint, GLenum, GLboolean, GLuint);
		PFNGLVERTEXATTRIBP3UI glVertexAttribP3ui = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBP3UIV)(GLuint, GLenum, GLboolean, const GLuint *);
		PFNGLVERTEXATTRIBP3UIV glVertexAttribP3uiv = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBP4UI)(GLuint, GLenum, GLboolean, GLuint);
		PFNGLVERTEXATTRIBP4UI glVertexAttribP4ui = 0;
		typedef void (CODEGEN_FUNCPTR *PFNGLVERTEXATTRIBP4UIV)(GLuint, GLenum, GLboolean, const GLuint *);
		PFNGLVERTEXATTRIBP4UIV glVertexAttribP4uiv = 0;
		
		static int LoadCoreFunctions()
		{
			int numFailed = 0;
			glBlendFunc = reinterpret_cast<PFNGLBLENDFUNC>(IntGetProcAddress("glBlendFunc"));
			if(!glBlendFunc) ++numFailed;
			glClear = reinterpret_cast<PFNGLCLEAR>(IntGetProcAddress("glClear"));
			if(!glClear) ++numFailed;
			glClearColor = reinterpret_cast<PFNGLCLEARCOLOR>(IntGetProcAddress("glClearColor"));
			if(!glClearColor) ++numFailed;
			glClearDepth = reinterpret_cast<PFNGLCLEARDEPTH>(IntGetProcAddress("glClearDepth"));
			if(!glClearDepth) ++numFailed;
			glClearStencil = reinterpret_cast<PFNGLCLEARSTENCIL>(IntGetProcAddress("glClearStencil"));
			if(!glClearStencil) ++numFailed;
			glColorMask = reinterpret_cast<PFNGLCOLORMASK>(IntGetProcAddress("glColorMask"));
			if(!glColorMask) ++numFailed;
			glCullFace = reinterpret_cast<PFNGLCULLFACE>(IntGetProcAddress("glCullFace"));
			if(!glCullFace) ++numFailed;
			glDepthFunc = reinterpret_cast<PFNGLDEPTHFUNC>(IntGetProcAddress("glDepthFunc"));
			if(!glDepthFunc) ++numFailed;
			glDepthMask = reinterpret_cast<PFNGLDEPTHMASK>(IntGetProcAddress("glDepthMask"));
			if(!glDepthMask) ++numFailed;
			glDepthRange = reinterpret_cast<PFNGLDEPTHRANGE>(IntGetProcAddress("glDepthRange"));
			if(!glDepthRange) ++numFailed;
			glDisable = reinterpret_cast<PFNGLDISABLE>(IntGetProcAddress("glDisable"));
			if(!glDisable) ++numFailed;
			glDrawBuffer = reinterpret_cast<PFNGLDRAWBUFFER>(IntGetProcAddress("glDrawBuffer"));
			if(!glDrawBuffer) ++numFailed;
			glEnable = reinterpret_cast<PFNGLENABLE>(IntGetProcAddress("glEnable"));
			if(!glEnable) ++numFailed;
			glFinish = reinterpret_cast<PFNGLFINISH>(IntGetProcAddress("glFinish"));
			if(!glFinish) ++numFailed;
			glFlush = reinterpret_cast<PFNGLFLUSH>(IntGetProcAddress("glFlush"));
			if(!glFlush) ++numFailed;
			glFrontFace = reinterpret_cast<PFNGLFRONTFACE>(IntGetProcAddress("glFrontFace"));
			if(!glFrontFace) ++numFailed;
			glGetBooleanv = reinterpret_cast<PFNGLGETBOOLEANV>(IntGetProcAddress("glGetBooleanv"));
			if(!glGetBooleanv) ++numFailed;
			glGetDoublev = reinterpret_cast<PFNGLGETDOUBLEV>(IntGetProcAddress("glGetDoublev"));
			if(!glGetDoublev) ++numFailed;
			glGetError = reinterpret_cast<PFNGLGETERROR>(IntGetProcAddress("glGetError"));
			if(!glGetError) ++numFailed;
			glGetFloatv = reinterpret_cast<PFNGLGETFLOATV>(IntGetProcAddress("glGetFloatv"));
			if(!glGetFloatv) ++numFailed;
			glGetIntegerv = reinterpret_cast<PFNGLGETINTEGERV>(IntGetProcAddress("glGetIntegerv"));
			if(!glGetIntegerv) ++numFailed;
			glGetString = reinterpret_cast<PFNGLGETSTRING>(IntGetProcAddress("glGetString"));
			if(!glGetString) ++numFailed;
			glGetTexImage = reinterpret_cast<PFNGLGETTEXIMAGE>(IntGetProcAddress("glGetTexImage"));
			if(!glGetTexImage) ++numFailed;
			glGetTexLevelParameterfv = reinterpret_cast<PFNGLGETTEXLEVELPARAMETERFV>(IntGetProcAddress("glGetTexLevelParameterfv"));
			if(!glGetTexLevelParameterfv) ++numFailed;
			glGetTexLevelParameteriv = reinterpret_cast<PFNGLGETTEXLEVELPARAMETERIV>(IntGetProcAddress("glGetTexLevelParameteriv"));
			if(!glGetTexLevelParameteriv) ++numFailed;
			glGetTexParameterfv = reinterpret_cast<PFNGLGETTEXPARAMETERFV>(IntGetProcAddress("glGetTexParameterfv"));
			if(!glGetTexParameterfv) ++numFailed;
			glGetTexParameteriv = reinterpret_cast<PFNGLGETTEXPARAMETERIV>(IntGetProcAddress("glGetTexParameteriv"));
			if(!glGetTexParameteriv) ++numFailed;
			glHint = reinterpret_cast<PFNGLHINT>(IntGetProcAddress("glHint"));
			if(!glHint) ++numFailed;
			glIsEnabled = reinterpret_cast<PFNGLISENABLED>(IntGetProcAddress("glIsEnabled"));
			if(!glIsEnabled) ++numFailed;
			glLineWidth = reinterpret_cast<PFNGLLINEWIDTH>(IntGetProcAddress("glLineWidth"));
			if(!glLineWidth) ++numFailed;
			glLogicOp = reinterpret_cast<PFNGLLOGICOP>(IntGetProcAddress("glLogicOp"));
			if(!glLogicOp) ++numFailed;
			glPixelStoref = reinterpret_cast<PFNGLPIXELSTOREF>(IntGetProcAddress("glPixelStoref"));
			if(!glPixelStoref) ++numFailed;
			glPixelStorei = reinterpret_cast<PFNGLPIXELSTOREI>(IntGetProcAddress("glPixelStorei"));
			if(!glPixelStorei) ++numFailed;
			glPointSize = reinterpret_cast<PFNGLPOINTSIZE>(IntGetProcAddress("glPointSize"));
			if(!glPointSize) ++numFailed;
			glPolygonMode = reinterpret_cast<PFNGLPOLYGONMODE>(IntGetProcAddress("glPolygonMode"));
			if(!glPolygonMode) ++numFailed;
			glReadBuffer = reinterpret_cast<PFNGLREADBUFFER>(IntGetProcAddress("glReadBuffer"));
			if(!glReadBuffer) ++numFailed;
			glReadPixels = reinterpret_cast<PFNGLREADPIXELS>(IntGetProcAddress("glReadPixels"));
			if(!glReadPixels) ++numFailed;
			glScissor = reinterpret_cast<PFNGLSCISSOR>(IntGetProcAddress("glScissor"));
			if(!glScissor) ++numFailed;
			glStencilFunc = reinterpret_cast<PFNGLSTENCILFUNC>(IntGetProcAddress("glStencilFunc"));
			if(!glStencilFunc) ++numFailed;
			glStencilMask = reinterpret_cast<PFNGLSTENCILMASK>(IntGetProcAddress("glStencilMask"));
			if(!glStencilMask) ++numFailed;
			glStencilOp = reinterpret_cast<PFNGLSTENCILOP>(IntGetProcAddress("glStencilOp"));
			if(!glStencilOp) ++numFailed;
			glTexImage1D = reinterpret_cast<PFNGLTEXIMAGE1D>(IntGetProcAddress("glTexImage1D"));
			if(!glTexImage1D) ++numFailed;
			glTexImage2D = reinterpret_cast<PFNGLTEXIMAGE2D>(IntGetProcAddress("glTexImage2D"));
			if(!glTexImage2D) ++numFailed;
			glTexParameterf = reinterpret_cast<PFNGLTEXPARAMETERF>(IntGetProcAddress("glTexParameterf"));
			if(!glTexParameterf) ++numFailed;
			glTexParameterfv = reinterpret_cast<PFNGLTEXPARAMETERFV>(IntGetProcAddress("glTexParameterfv"));
			if(!glTexParameterfv) ++numFailed;
			glTexParameteri = reinterpret_cast<PFNGLTEXPARAMETERI>(IntGetProcAddress("glTexParameteri"));
			if(!glTexParameteri) ++numFailed;
			glTexParameteriv = reinterpret_cast<PFNGLTEXPARAMETERIV>(IntGetProcAddress("glTexParameteriv"));
			if(!glTexParameteriv) ++numFailed;
			glViewport = reinterpret_cast<PFNGLVIEWPORT>(IntGetProcAddress("glViewport"));
			if(!glViewport) ++numFailed;
			glBindTexture = reinterpret_cast<PFNGLBINDTEXTURE>(IntGetProcAddress("glBindTexture"));
			if(!glBindTexture) ++numFailed;
			glCopyTexImage1D = reinterpret_cast<PFNGLCOPYTEXIMAGE1D>(IntGetProcAddress("glCopyTexImage1D"));
			if(!glCopyTexImage1D) ++numFailed;
			glCopyTexImage2D = reinterpret_cast<PFNGLCOPYTEXIMAGE2D>(IntGetProcAddress("glCopyTexImage2D"));
			if(!glCopyTexImage2D) ++numFailed;
			glCopyTexSubImage1D = reinterpret_cast<PFNGLCOPYTEXSUBIMAGE1D>(IntGetProcAddress("glCopyTexSubImage1D"));
			if(!glCopyTexSubImage1D) ++numFailed;
			glCopyTexSubImage2D = reinterpret_cast<PFNGLCOPYTEXSUBIMAGE2D>(IntGetProcAddress("glCopyTexSubImage2D"));
			if(!glCopyTexSubImage2D) ++numFailed;
			glDeleteTextures = reinterpret_cast<PFNGLDELETETEXTURES>(IntGetProcAddress("glDeleteTextures"));
			if(!glDeleteTextures) ++numFailed;
			glDrawArrays = reinterpret_cast<PFNGLDRAWARRAYS>(IntGetProcAddress("glDrawArrays"));
			if(!glDrawArrays) ++numFailed;
			glDrawElements = reinterpret_cast<PFNGLDRAWELEMENTS>(IntGetProcAddress("glDrawElements"));
			if(!glDrawElements) ++numFailed;
			glGenTextures = reinterpret_cast<PFNGLGENTEXTURES>(IntGetProcAddress("glGenTextures"));
			if(!glGenTextures) ++numFailed;
			glIsTexture = reinterpret_cast<PFNGLISTEXTURE>(IntGetProcAddress("glIsTexture"));
			if(!glIsTexture) ++numFailed;
			glPolygonOffset = reinterpret_cast<PFNGLPOLYGONOFFSET>(IntGetProcAddress("glPolygonOffset"));
			if(!glPolygonOffset) ++numFailed;
			glTexSubImage1D = reinterpret_cast<PFNGLTEXSUBIMAGE1D>(IntGetProcAddress("glTexSubImage1D"));
			if(!glTexSubImage1D) ++numFailed;
			glTexSubImage2D = reinterpret_cast<PFNGLTEXSUBIMAGE2D>(IntGetProcAddress("glTexSubImage2D"));
			if(!glTexSubImage2D) ++numFailed;
			glCopyTexSubImage3D = reinterpret_cast<PFNGLCOPYTEXSUBIMAGE3D>(IntGetProcAddress("glCopyTexSubImage3D"));
			if(!glCopyTexSubImage3D) ++numFailed;
			glDrawRangeElements = reinterpret_cast<PFNGLDRAWRANGEELEMENTS>(IntGetProcAddress("glDrawRangeElements"));
			if(!glDrawRangeElements) ++numFailed;
			glTexImage3D = reinterpret_cast<PFNGLTEXIMAGE3D>(IntGetProcAddress("glTexImage3D"));
			if(!glTexImage3D) ++numFailed;
			glTexSubImage3D = reinterpret_cast<PFNGLTEXSUBIMAGE3D>(IntGetProcAddress("glTexSubImage3D"));
			if(!glTexSubImage3D) ++numFailed;
			glActiveTexture = reinterpret_cast<PFNGLACTIVETEXTURE>(IntGetProcAddress("glActiveTexture"));
			if(!glActiveTexture) ++numFailed;
			glCompressedTexImage1D = reinterpret_cast<PFNGLCOMPRESSEDTEXIMAGE1D>(IntGetProcAddress("glCompressedTexImage1D"));
			if(!glCompressedTexImage1D) ++numFailed;
			glCompressedTexImage2D = reinterpret_cast<PFNGLCOMPRESSEDTEXIMAGE2D>(IntGetProcAddress("glCompressedTexImage2D"));
			if(!glCompressedTexImage2D) ++numFailed;
			glCompressedTexImage3D = reinterpret_cast<PFNGLCOMPRESSEDTEXIMAGE3D>(IntGetProcAddress("glCompressedTexImage3D"));
			if(!glCompressedTexImage3D) ++numFailed;
			glCompressedTexSubImage1D = reinterpret_cast<PFNGLCOMPRESSEDTEXSUBIMAGE1D>(IntGetProcAddress("glCompressedTexSubImage1D"));
			if(!glCompressedTexSubImage1D) ++numFailed;
			glCompressedTexSubImage2D = reinterpret_cast<PFNGLCOMPRESSEDTEXSUBIMAGE2D>(IntGetProcAddress("glCompressedTexSubImage2D"));
			if(!glCompressedTexSubImage2D) ++numFailed;
			glCompressedTexSubImage3D = reinterpret_cast<PFNGLCOMPRESSEDTEXSUBIMAGE3D>(IntGetProcAddress("glCompressedTexSubImage3D"));
			if(!glCompressedTexSubImage3D) ++numFailed;
			glGetCompressedTexImage = reinterpret_cast<PFNGLGETCOMPRESSEDTEXIMAGE>(IntGetProcAddress("glGetCompressedTexImage"));
			if(!glGetCompressedTexImage) ++numFailed;
			glSampleCoverage = reinterpret_cast<PFNGLSAMPLECOVERAGE>(IntGetProcAddress("glSampleCoverage"));
			if(!glSampleCoverage) ++numFailed;
			glBlendColor = reinterpret_cast<PFNGLBLENDCOLOR>(IntGetProcAddress("glBlendColor"));
			if(!glBlendColor) ++numFailed;
			glBlendEquation = reinterpret_cast<PFNGLBLENDEQUATION>(IntGetProcAddress("glBlendEquation"));
			if(!glBlendEquation) ++numFailed;
			glBlendFuncSeparate = reinterpret_cast<PFNGLBLENDFUNCSEPARATE>(IntGetProcAddress("glBlendFuncSeparate"));
			if(!glBlendFuncSeparate) ++numFailed;
			glMultiDrawArrays = reinterpret_cast<PFNGLMULTIDRAWARRAYS>(IntGetProcAddress("glMultiDrawArrays"));
			if(!glMultiDrawArrays) ++numFailed;
			glMultiDrawElements = reinterpret_cast<PFNGLMULTIDRAWELEMENTS>(IntGetProcAddress("glMultiDrawElements"));
			if(!glMultiDrawElements) ++numFailed;
			glPointParameterf = reinterpret_cast<PFNGLPOINTPARAMETERF>(IntGetProcAddress("glPointParameterf"));
			if(!glPointParameterf) ++numFailed;
			glPointParameterfv = reinterpret_cast<PFNGLPOINTPARAMETERFV>(IntGetProcAddress("glPointParameterfv"));
			if(!glPointParameterfv) ++numFailed;
			glPointParameteri = reinterpret_cast<PFNGLPOINTPARAMETERI>(IntGetProcAddress("glPointParameteri"));
			if(!glPointParameteri) ++numFailed;
			glPointParameteriv = reinterpret_cast<PFNGLPOINTPARAMETERIV>(IntGetProcAddress("glPointParameteriv"));
			if(!glPointParameteriv) ++numFailed;
			glBeginQuery = reinterpret_cast<PFNGLBEGINQUERY>(IntGetProcAddress("glBeginQuery"));
			if(!glBeginQuery) ++numFailed;
			glBindBuffer = reinterpret_cast<PFNGLBINDBUFFER>(IntGetProcAddress("glBindBuffer"));
			if(!glBindBuffer) ++numFailed;
			glBufferData = reinterpret_cast<PFNGLBUFFERDATA>(IntGetProcAddress("glBufferData"));
			if(!glBufferData) ++numFailed;
			glBufferSubData = reinterpret_cast<PFNGLBUFFERSUBDATA>(IntGetProcAddress("glBufferSubData"));
			if(!glBufferSubData) ++numFailed;
			glDeleteBuffers = reinterpret_cast<PFNGLDELETEBUFFERS>(IntGetProcAddress("glDeleteBuffers"));
			if(!glDeleteBuffers) ++numFailed;
			glDeleteQueries = reinterpret_cast<PFNGLDELETEQUERIES>(IntGetProcAddress("glDeleteQueries"));
			if(!glDeleteQueries) ++numFailed;
			glEndQuery = reinterpret_cast<PFNGLENDQUERY>(IntGetProcAddress("glEndQuery"));
			if(!glEndQuery) ++numFailed;
			glGenBuffers = reinterpret_cast<PFNGLGENBUFFERS>(IntGetProcAddress("glGenBuffers"));
			if(!glGenBuffers) ++numFailed;
			glGenQueries = reinterpret_cast<PFNGLGENQUERIES>(IntGetProcAddress("glGenQueries"));
			if(!glGenQueries) ++numFailed;
			glGetBufferParameteriv = reinterpret_cast<PFNGLGETBUFFERPARAMETERIV>(IntGetProcAddress("glGetBufferParameteriv"));
			if(!glGetBufferParameteriv) ++numFailed;
			glGetBufferPointerv = reinterpret_cast<PFNGLGETBUFFERPOINTERV>(IntGetProcAddress("glGetBufferPointerv"));
			if(!glGetBufferPointerv) ++numFailed;
			glGetBufferSubData = reinterpret_cast<PFNGLGETBUFFERSUBDATA>(IntGetProcAddress("glGetBufferSubData"));
			if(!glGetBufferSubData) ++numFailed;
			glGetQueryObjectiv = reinterpret_cast<PFNGLGETQUERYOBJECTIV>(IntGetProcAddress("glGetQueryObjectiv"));
			if(!glGetQueryObjectiv) ++numFailed;
			glGetQueryObjectuiv = reinterpret_cast<PFNGLGETQUERYOBJECTUIV>(IntGetProcAddress("glGetQueryObjectuiv"));
			if(!glGetQueryObjectuiv) ++numFailed;
			glGetQueryiv = reinterpret_cast<PFNGLGETQUERYIV>(IntGetProcAddress("glGetQueryiv"));
			if(!glGetQueryiv) ++numFailed;
			glIsBuffer = reinterpret_cast<PFNGLISBUFFER>(IntGetProcAddress("glIsBuffer"));
			if(!glIsBuffer) ++numFailed;
			glIsQuery = reinterpret_cast<PFNGLISQUERY>(IntGetProcAddress("glIsQuery"));
			if(!glIsQuery) ++numFailed;
			glMapBuffer = reinterpret_cast<PFNGLMAPBUFFER>(IntGetProcAddress("glMapBuffer"));
			if(!glMapBuffer) ++numFailed;
			glUnmapBuffer = reinterpret_cast<PFNGLUNMAPBUFFER>(IntGetProcAddress("glUnmapBuffer"));
			if(!glUnmapBuffer) ++numFailed;
			glAttachShader = reinterpret_cast<PFNGLATTACHSHADER>(IntGetProcAddress("glAttachShader"));
			if(!glAttachShader) ++numFailed;
			glBindAttribLocation = reinterpret_cast<PFNGLBINDATTRIBLOCATION>(IntGetProcAddress("glBindAttribLocation"));
			if(!glBindAttribLocation) ++numFailed;
			glBlendEquationSeparate = reinterpret_cast<PFNGLBLENDEQUATIONSEPARATE>(IntGetProcAddress("glBlendEquationSeparate"));
			if(!glBlendEquationSeparate) ++numFailed;
			glCompileShader = reinterpret_cast<PFNGLCOMPILESHADER>(IntGetProcAddress("glCompileShader"));
			if(!glCompileShader) ++numFailed;
			glCreateProgram = reinterpret_cast<PFNGLCREATEPROGRAM>(IntGetProcAddress("glCreateProgram"));
			if(!glCreateProgram) ++numFailed;
			glCreateShader = reinterpret_cast<PFNGLCREATESHADER>(IntGetProcAddress("glCreateShader"));
			if(!glCreateShader) ++numFailed;
			glDeleteProgram = reinterpret_cast<PFNGLDELETEPROGRAM>(IntGetProcAddress("glDeleteProgram"));
			if(!glDeleteProgram) ++numFailed;
			glDeleteShader = reinterpret_cast<PFNGLDELETESHADER>(IntGetProcAddress("glDeleteShader"));
			if(!glDeleteShader) ++numFailed;
			glDetachShader = reinterpret_cast<PFNGLDETACHSHADER>(IntGetProcAddress("glDetachShader"));
			if(!glDetachShader) ++numFailed;
			glDisableVertexAttribArray = reinterpret_cast<PFNGLDISABLEVERTEXATTRIBARRAY>(IntGetProcAddress("glDisableVertexAttribArray"));
			if(!glDisableVertexAttribArray) ++numFailed;
			glDrawBuffers = reinterpret_cast<PFNGLDRAWBUFFERS>(IntGetProcAddress("glDrawBuffers"));
			if(!glDrawBuffers) ++numFailed;
			glEnableVertexAttribArray = reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAY>(IntGetProcAddress("glEnableVertexAttribArray"));
			if(!glEnableVertexAttribArray) ++numFailed;
			glGetActiveAttrib = reinterpret_cast<PFNGLGETACTIVEATTRIB>(IntGetProcAddress("glGetActiveAttrib"));
			if(!glGetActiveAttrib) ++numFailed;
			glGetActiveUniform = reinterpret_cast<PFNGLGETACTIVEUNIFORM>(IntGetProcAddress("glGetActiveUniform"));
			if(!glGetActiveUniform) ++numFailed;
			glGetAttachedShaders = reinterpret_cast<PFNGLGETATTACHEDSHADERS>(IntGetProcAddress("glGetAttachedShaders"));
			if(!glGetAttachedShaders) ++numFailed;
			glGetAttribLocation = reinterpret_cast<PFNGLGETATTRIBLOCATION>(IntGetProcAddress("glGetAttribLocation"));
			if(!glGetAttribLocation) ++numFailed;
			glGetProgramInfoLog = reinterpret_cast<PFNGLGETPROGRAMINFOLOG>(IntGetProcAddress("glGetProgramInfoLog"));
			if(!glGetProgramInfoLog) ++numFailed;
			glGetProgramiv = reinterpret_cast<PFNGLGETPROGRAMIV>(IntGetProcAddress("glGetProgramiv"));
			if(!glGetProgramiv) ++numFailed;
			glGetShaderInfoLog = reinterpret_cast<PFNGLGETSHADERINFOLOG>(IntGetProcAddress("glGetShaderInfoLog"));
			if(!glGetShaderInfoLog) ++numFailed;
			glGetShaderSource = reinterpret_cast<PFNGLGETSHADERSOURCE>(IntGetProcAddress("glGetShaderSource"));
			if(!glGetShaderSource) ++numFailed;
			glGetShaderiv = reinterpret_cast<PFNGLGETSHADERIV>(IntGetProcAddress("glGetShaderiv"));
			if(!glGetShaderiv) ++numFailed;
			glGetUniformLocation = reinterpret_cast<PFNGLGETUNIFORMLOCATION>(IntGetProcAddress("glGetUniformLocation"));
			if(!glGetUniformLocation) ++numFailed;
			glGetUniformfv = reinterpret_cast<PFNGLGETUNIFORMFV>(IntGetProcAddress("glGetUniformfv"));
			if(!glGetUniformfv) ++numFailed;
			glGetUniformiv = reinterpret_cast<PFNGLGETUNIFORMIV>(IntGetProcAddress("glGetUniformiv"));
			if(!glGetUniformiv) ++numFailed;
			glGetVertexAttribPointerv = reinterpret_cast<PFNGLGETVERTEXATTRIBPOINTERV>(IntGetProcAddress("glGetVertexAttribPointerv"));
			if(!glGetVertexAttribPointerv) ++numFailed;
			glGetVertexAttribdv = reinterpret_cast<PFNGLGETVERTEXATTRIBDV>(IntGetProcAddress("glGetVertexAttribdv"));
			if(!glGetVertexAttribdv) ++numFailed;
			glGetVertexAttribfv = reinterpret_cast<PFNGLGETVERTEXATTRIBFV>(IntGetProcAddress("glGetVertexAttribfv"));
			if(!glGetVertexAttribfv) ++numFailed;
			glGetVertexAttribiv = reinterpret_cast<PFNGLGETVERTEXATTRIBIV>(IntGetProcAddress("glGetVertexAttribiv"));
			if(!glGetVertexAttribiv) ++numFailed;
			glIsProgram = reinterpret_cast<PFNGLISPROGRAM>(IntGetProcAddress("glIsProgram"));
			if(!glIsProgram) ++numFailed;
			glIsShader = reinterpret_cast<PFNGLISSHADER>(IntGetProcAddress("glIsShader"));
			if(!glIsShader) ++numFailed;
			glLinkProgram = reinterpret_cast<PFNGLLINKPROGRAM>(IntGetProcAddress("glLinkProgram"));
			if(!glLinkProgram) ++numFailed;
			glShaderSource = reinterpret_cast<PFNGLSHADERSOURCE>(IntGetProcAddress("glShaderSource"));
			if(!glShaderSource) ++numFailed;
			glStencilFuncSeparate = reinterpret_cast<PFNGLSTENCILFUNCSEPARATE>(IntGetProcAddress("glStencilFuncSeparate"));
			if(!glStencilFuncSeparate) ++numFailed;
			glStencilMaskSeparate = reinterpret_cast<PFNGLSTENCILMASKSEPARATE>(IntGetProcAddress("glStencilMaskSeparate"));
			if(!glStencilMaskSeparate) ++numFailed;
			glStencilOpSeparate = reinterpret_cast<PFNGLSTENCILOPSEPARATE>(IntGetProcAddress("glStencilOpSeparate"));
			if(!glStencilOpSeparate) ++numFailed;
			glUniform1f = reinterpret_cast<PFNGLUNIFORM1F>(IntGetProcAddress("glUniform1f"));
			if(!glUniform1f) ++numFailed;
			glUniform1fv = reinterpret_cast<PFNGLUNIFORM1FV>(IntGetProcAddress("glUniform1fv"));
			if(!glUniform1fv) ++numFailed;
			glUniform1i = reinterpret_cast<PFNGLUNIFORM1I>(IntGetProcAddress("glUniform1i"));
			if(!glUniform1i) ++numFailed;
			glUniform1iv = reinterpret_cast<PFNGLUNIFORM1IV>(IntGetProcAddress("glUniform1iv"));
			if(!glUniform1iv) ++numFailed;
			glUniform2f = reinterpret_cast<PFNGLUNIFORM2F>(IntGetProcAddress("glUniform2f"));
			if(!glUniform2f) ++numFailed;
			glUniform2fv = reinterpret_cast<PFNGLUNIFORM2FV>(IntGetProcAddress("glUniform2fv"));
			if(!glUniform2fv) ++numFailed;
			glUniform2i = reinterpret_cast<PFNGLUNIFORM2I>(IntGetProcAddress("glUniform2i"));
			if(!glUniform2i) ++numFailed;
			glUniform2iv = reinterpret_cast<PFNGLUNIFORM2IV>(IntGetProcAddress("glUniform2iv"));
			if(!glUniform2iv) ++numFailed;
			glUniform3f = reinterpret_cast<PFNGLUNIFORM3F>(IntGetProcAddress("glUniform3f"));
			if(!glUniform3f) ++numFailed;
			glUniform3fv = reinterpret_cast<PFNGLUNIFORM3FV>(IntGetProcAddress("glUniform3fv"));
			if(!glUniform3fv) ++numFailed;
			glUniform3i = reinterpret_cast<PFNGLUNIFORM3I>(IntGetProcAddress("glUniform3i"));
			if(!glUniform3i) ++numFailed;
			glUniform3iv = reinterpret_cast<PFNGLUNIFORM3IV>(IntGetProcAddress("glUniform3iv"));
			if(!glUniform3iv) ++numFailed;
			glUniform4f = reinterpret_cast<PFNGLUNIFORM4F>(IntGetProcAddress("glUniform4f"));
			if(!glUniform4f) ++numFailed;
			glUniform4fv = reinterpret_cast<PFNGLUNIFORM4FV>(IntGetProcAddress("glUniform4fv"));
			if(!glUniform4fv) ++numFailed;
			glUniform4i = reinterpret_cast<PFNGLUNIFORM4I>(IntGetProcAddress("glUniform4i"));
			if(!glUniform4i) ++numFailed;
			glUniform4iv = reinterpret_cast<PFNGLUNIFORM4IV>(IntGetProcAddress("glUniform4iv"));
			if(!glUniform4iv) ++numFailed;
			glUniformMatrix2fv = reinterpret_cast<PFNGLUNIFORMMATRIX2FV>(IntGetProcAddress("glUniformMatrix2fv"));
			if(!glUniformMatrix2fv) ++numFailed;
			glUniformMatrix3fv = reinterpret_cast<PFNGLUNIFORMMATRIX3FV>(IntGetProcAddress("glUniformMatrix3fv"));
			if(!glUniformMatrix3fv) ++numFailed;
			glUniformMatrix4fv = reinterpret_cast<PFNGLUNIFORMMATRIX4FV>(IntGetProcAddress("glUniformMatrix4fv"));
			if(!glUniformMatrix4fv) ++numFailed;
			glUseProgram = reinterpret_cast<PFNGLUSEPROGRAM>(IntGetProcAddress("glUseProgram"));
			if(!glUseProgram) ++numFailed;
			glValidateProgram = reinterpret_cast<PFNGLVALIDATEPROGRAM>(IntGetProcAddress("glValidateProgram"));
			if(!glValidateProgram) ++numFailed;
			glVertexAttrib1d = reinterpret_cast<PFNGLVERTEXATTRIB1D>(IntGetProcAddress("glVertexAttrib1d"));
			if(!glVertexAttrib1d) ++numFailed;
			glVertexAttrib1dv = reinterpret_cast<PFNGLVERTEXATTRIB1DV>(IntGetProcAddress("glVertexAttrib1dv"));
			if(!glVertexAttrib1dv) ++numFailed;
			glVertexAttrib1f = reinterpret_cast<PFNGLVERTEXATTRIB1F>(IntGetProcAddress("glVertexAttrib1f"));
			if(!glVertexAttrib1f) ++numFailed;
			glVertexAttrib1fv = reinterpret_cast<PFNGLVERTEXATTRIB1FV>(IntGetProcAddress("glVertexAttrib1fv"));
			if(!glVertexAttrib1fv) ++numFailed;
			glVertexAttrib1s = reinterpret_cast<PFNGLVERTEXATTRIB1S>(IntGetProcAddress("glVertexAttrib1s"));
			if(!glVertexAttrib1s) ++numFailed;
			glVertexAttrib1sv = reinterpret_cast<PFNGLVERTEXATTRIB1SV>(IntGetProcAddress("glVertexAttrib1sv"));
			if(!glVertexAttrib1sv) ++numFailed;
			glVertexAttrib2d = reinterpret_cast<PFNGLVERTEXATTRIB2D>(IntGetProcAddress("glVertexAttrib2d"));
			if(!glVertexAttrib2d) ++numFailed;
			glVertexAttrib2dv = reinterpret_cast<PFNGLVERTEXATTRIB2DV>(IntGetProcAddress("glVertexAttrib2dv"));
			if(!glVertexAttrib2dv) ++numFailed;
			glVertexAttrib2f = reinterpret_cast<PFNGLVERTEXATTRIB2F>(IntGetProcAddress("glVertexAttrib2f"));
			if(!glVertexAttrib2f) ++numFailed;
			glVertexAttrib2fv = reinterpret_cast<PFNGLVERTEXATTRIB2FV>(IntGetProcAddress("glVertexAttrib2fv"));
			if(!glVertexAttrib2fv) ++numFailed;
			glVertexAttrib2s = reinterpret_cast<PFNGLVERTEXATTRIB2S>(IntGetProcAddress("glVertexAttrib2s"));
			if(!glVertexAttrib2s) ++numFailed;
			glVertexAttrib2sv = reinterpret_cast<PFNGLVERTEXATTRIB2SV>(IntGetProcAddress("glVertexAttrib2sv"));
			if(!glVertexAttrib2sv) ++numFailed;
			glVertexAttrib3d = reinterpret_cast<PFNGLVERTEXATTRIB3D>(IntGetProcAddress("glVertexAttrib3d"));
			if(!glVertexAttrib3d) ++numFailed;
			glVertexAttrib3dv = reinterpret_cast<PFNGLVERTEXATTRIB3DV>(IntGetProcAddress("glVertexAttrib3dv"));
			if(!glVertexAttrib3dv) ++numFailed;
			glVertexAttrib3f = reinterpret_cast<PFNGLVERTEXATTRIB3F>(IntGetProcAddress("glVertexAttrib3f"));
			if(!glVertexAttrib3f) ++numFailed;
			glVertexAttrib3fv = reinterpret_cast<PFNGLVERTEXATTRIB3FV>(IntGetProcAddress("glVertexAttrib3fv"));
			if(!glVertexAttrib3fv) ++numFailed;
			glVertexAttrib3s = reinterpret_cast<PFNGLVERTEXATTRIB3S>(IntGetProcAddress("glVertexAttrib3s"));
			if(!glVertexAttrib3s) ++numFailed;
			glVertexAttrib3sv = reinterpret_cast<PFNGLVERTEXATTRIB3SV>(IntGetProcAddress("glVertexAttrib3sv"));
			if(!glVertexAttrib3sv) ++numFailed;
			glVertexAttrib4Nbv = reinterpret_cast<PFNGLVERTEXATTRIB4NBV>(IntGetProcAddress("glVertexAttrib4Nbv"));
			if(!glVertexAttrib4Nbv) ++numFailed;
			glVertexAttrib4Niv = reinterpret_cast<PFNGLVERTEXATTRIB4NIV>(IntGetProcAddress("glVertexAttrib4Niv"));
			if(!glVertexAttrib4Niv) ++numFailed;
			glVertexAttrib4Nsv = reinterpret_cast<PFNGLVERTEXATTRIB4NSV>(IntGetProcAddress("glVertexAttrib4Nsv"));
			if(!glVertexAttrib4Nsv) ++numFailed;
			glVertexAttrib4Nub = reinterpret_cast<PFNGLVERTEXATTRIB4NUB>(IntGetProcAddress("glVertexAttrib4Nub"));
			if(!glVertexAttrib4Nub) ++numFailed;
			glVertexAttrib4Nubv = reinterpret_cast<PFNGLVERTEXATTRIB4NUBV>(IntGetProcAddress("glVertexAttrib4Nubv"));
			if(!glVertexAttrib4Nubv) ++numFailed;
			glVertexAttrib4Nuiv = reinterpret_cast<PFNGLVERTEXATTRIB4NUIV>(IntGetProcAddress("glVertexAttrib4Nuiv"));
			if(!glVertexAttrib4Nuiv) ++numFailed;
			glVertexAttrib4Nusv = reinterpret_cast<PFNGLVERTEXATTRIB4NUSV>(IntGetProcAddress("glVertexAttrib4Nusv"));
			if(!glVertexAttrib4Nusv) ++numFailed;
			glVertexAttrib4bv = reinterpret_cast<PFNGLVERTEXATTRIB4BV>(IntGetProcAddress("glVertexAttrib4bv"));
			if(!glVertexAttrib4bv) ++numFailed;
			glVertexAttrib4d = reinterpret_cast<PFNGLVERTEXATTRIB4D>(IntGetProcAddress("glVertexAttrib4d"));
			if(!glVertexAttrib4d) ++numFailed;
			glVertexAttrib4dv = reinterpret_cast<PFNGLVERTEXATTRIB4DV>(IntGetProcAddress("glVertexAttrib4dv"));
			if(!glVertexAttrib4dv) ++numFailed;
			glVertexAttrib4f = reinterpret_cast<PFNGLVERTEXATTRIB4F>(IntGetProcAddress("glVertexAttrib4f"));
			if(!glVertexAttrib4f) ++numFailed;
			glVertexAttrib4fv = reinterpret_cast<PFNGLVERTEXATTRIB4FV>(IntGetProcAddress("glVertexAttrib4fv"));
			if(!glVertexAttrib4fv) ++numFailed;
			glVertexAttrib4iv = reinterpret_cast<PFNGLVERTEXATTRIB4IV>(IntGetProcAddress("glVertexAttrib4iv"));
			if(!glVertexAttrib4iv) ++numFailed;
			glVertexAttrib4s = reinterpret_cast<PFNGLVERTEXATTRIB4S>(IntGetProcAddress("glVertexAttrib4s"));
			if(!glVertexAttrib4s) ++numFailed;
			glVertexAttrib4sv = reinterpret_cast<PFNGLVERTEXATTRIB4SV>(IntGetProcAddress("glVertexAttrib4sv"));
			if(!glVertexAttrib4sv) ++numFailed;
			glVertexAttrib4ubv = reinterpret_cast<PFNGLVERTEXATTRIB4UBV>(IntGetProcAddress("glVertexAttrib4ubv"));
			if(!glVertexAttrib4ubv) ++numFailed;
			glVertexAttrib4uiv = reinterpret_cast<PFNGLVERTEXATTRIB4UIV>(IntGetProcAddress("glVertexAttrib4uiv"));
			if(!glVertexAttrib4uiv) ++numFailed;
			glVertexAttrib4usv = reinterpret_cast<PFNGLVERTEXATTRIB4USV>(IntGetProcAddress("glVertexAttrib4usv"));
			if(!glVertexAttrib4usv) ++numFailed;
			glVertexAttribPointer = reinterpret_cast<PFNGLVERTEXATTRIBPOINTER>(IntGetProcAddress("glVertexAttribPointer"));
			if(!glVertexAttribPointer) ++numFailed;
			glUniformMatrix2x3fv = reinterpret_cast<PFNGLUNIFORMMATRIX2X3FV>(IntGetProcAddress("glUniformMatrix2x3fv"));
			if(!glUniformMatrix2x3fv) ++numFailed;
			glUniformMatrix2x4fv = reinterpret_cast<PFNGLUNIFORMMATRIX2X4FV>(IntGetProcAddress("glUniformMatrix2x4fv"));
			if(!glUniformMatrix2x4fv) ++numFailed;
			glUniformMatrix3x2fv = reinterpret_cast<PFNGLUNIFORMMATRIX3X2FV>(IntGetProcAddress("glUniformMatrix3x2fv"));
			if(!glUniformMatrix3x2fv) ++numFailed;
			glUniformMatrix3x4fv = reinterpret_cast<PFNGLUNIFORMMATRIX3X4FV>(IntGetProcAddress("glUniformMatrix3x4fv"));
			if(!glUniformMatrix3x4fv) ++numFailed;
			glUniformMatrix4x2fv = reinterpret_cast<PFNGLUNIFORMMATRIX4X2FV>(IntGetProcAddress("glUniformMatrix4x2fv"));
			if(!glUniformMatrix4x2fv) ++numFailed;
			glUniformMatrix4x3fv = reinterpret_cast<PFNGLUNIFORMMATRIX4X3FV>(IntGetProcAddress("glUniformMatrix4x3fv"));
			if(!glUniformMatrix4x3fv) ++numFailed;
			glBeginConditionalRender = reinterpret_cast<PFNGLBEGINCONDITIONALRENDER>(IntGetProcAddress("glBeginConditionalRender"));
			if(!glBeginConditionalRender) ++numFailed;
			glBeginTransformFeedback = reinterpret_cast<PFNGLBEGINTRANSFORMFEEDBACK>(IntGetProcAddress("glBeginTransformFeedback"));
			if(!glBeginTransformFeedback) ++numFailed;
			glBindBufferBase = reinterpret_cast<PFNGLBINDBUFFERBASE>(IntGetProcAddress("glBindBufferBase"));
			if(!glBindBufferBase) ++numFailed;
			glBindBufferRange = reinterpret_cast<PFNGLBINDBUFFERRANGE>(IntGetProcAddress("glBindBufferRange"));
			if(!glBindBufferRange) ++numFailed;
			glBindFragDataLocation = reinterpret_cast<PFNGLBINDFRAGDATALOCATION>(IntGetProcAddress("glBindFragDataLocation"));
			if(!glBindFragDataLocation) ++numFailed;
			glBindFramebuffer = reinterpret_cast<PFNGLBINDFRAMEBUFFER>(IntGetProcAddress("glBindFramebuffer"));
			if(!glBindFramebuffer) ++numFailed;
			glBindRenderbuffer = reinterpret_cast<PFNGLBINDRENDERBUFFER>(IntGetProcAddress("glBindRenderbuffer"));
			if(!glBindRenderbuffer) ++numFailed;
			glBindVertexArray = reinterpret_cast<PFNGLBINDVERTEXARRAY>(IntGetProcAddress("glBindVertexArray"));
			if(!glBindVertexArray) ++numFailed;
			glBlitFramebuffer = reinterpret_cast<PFNGLBLITFRAMEBUFFER>(IntGetProcAddress("glBlitFramebuffer"));
			if(!glBlitFramebuffer) ++numFailed;
			glCheckFramebufferStatus = reinterpret_cast<PFNGLCHECKFRAMEBUFFERSTATUS>(IntGetProcAddress("glCheckFramebufferStatus"));
			if(!glCheckFramebufferStatus) ++numFailed;
			glClampColor = reinterpret_cast<PFNGLCLAMPCOLOR>(IntGetProcAddress("glClampColor"));
			if(!glClampColor) ++numFailed;
			glClearBufferfi = reinterpret_cast<PFNGLCLEARBUFFERFI>(IntGetProcAddress("glClearBufferfi"));
			if(!glClearBufferfi) ++numFailed;
			glClearBufferfv = reinterpret_cast<PFNGLCLEARBUFFERFV>(IntGetProcAddress("glClearBufferfv"));
			if(!glClearBufferfv) ++numFailed;
			glClearBufferiv = reinterpret_cast<PFNGLCLEARBUFFERIV>(IntGetProcAddress("glClearBufferiv"));
			if(!glClearBufferiv) ++numFailed;
			glClearBufferuiv = reinterpret_cast<PFNGLCLEARBUFFERUIV>(IntGetProcAddress("glClearBufferuiv"));
			if(!glClearBufferuiv) ++numFailed;
			glColorMaski = reinterpret_cast<PFNGLCOLORMASKI>(IntGetProcAddress("glColorMaski"));
			if(!glColorMaski) ++numFailed;
			glDeleteFramebuffers = reinterpret_cast<PFNGLDELETEFRAMEBUFFERS>(IntGetProcAddress("glDeleteFramebuffers"));
			if(!glDeleteFramebuffers) ++numFailed;
			glDeleteRenderbuffers = reinterpret_cast<PFNGLDELETERENDERBUFFERS>(IntGetProcAddress("glDeleteRenderbuffers"));
			if(!glDeleteRenderbuffers) ++numFailed;
			glDeleteVertexArrays = reinterpret_cast<PFNGLDELETEVERTEXARRAYS>(IntGetProcAddress("glDeleteVertexArrays"));
			if(!glDeleteVertexArrays) ++numFailed;
			glDisablei = reinterpret_cast<PFNGLDISABLEI>(IntGetProcAddress("glDisablei"));
			if(!glDisablei) ++numFailed;
			glEnablei = reinterpret_cast<PFNGLENABLEI>(IntGetProcAddress("glEnablei"));
			if(!glEnablei) ++numFailed;
			glEndConditionalRender = reinterpret_cast<PFNGLENDCONDITIONALRENDER>(IntGetProcAddress("glEndConditionalRender"));
			if(!glEndConditionalRender) ++numFailed;
			glEndTransformFeedback = reinterpret_cast<PFNGLENDTRANSFORMFEEDBACK>(IntGetProcAddress("glEndTransformFeedback"));
			if(!glEndTransformFeedback) ++numFailed;
			glFlushMappedBufferRange = reinterpret_cast<PFNGLFLUSHMAPPEDBUFFERRANGE>(IntGetProcAddress("glFlushMappedBufferRange"));
			if(!glFlushMappedBufferRange) ++numFailed;
			glFramebufferRenderbuffer = reinterpret_cast<PFNGLFRAMEBUFFERRENDERBUFFER>(IntGetProcAddress("glFramebufferRenderbuffer"));
			if(!glFramebufferRenderbuffer) ++numFailed;
			glFramebufferTexture1D = reinterpret_cast<PFNGLFRAMEBUFFERTEXTURE1D>(IntGetProcAddress("glFramebufferTexture1D"));
			if(!glFramebufferTexture1D) ++numFailed;
			glFramebufferTexture2D = reinterpret_cast<PFNGLFRAMEBUFFERTEXTURE2D>(IntGetProcAddress("glFramebufferTexture2D"));
			if(!glFramebufferTexture2D) ++numFailed;
			glFramebufferTexture3D = reinterpret_cast<PFNGLFRAMEBUFFERTEXTURE3D>(IntGetProcAddress("glFramebufferTexture3D"));
			if(!glFramebufferTexture3D) ++numFailed;
			glFramebufferTextureLayer = reinterpret_cast<PFNGLFRAMEBUFFERTEXTURELAYER>(IntGetProcAddress("glFramebufferTextureLayer"));
			if(!glFramebufferTextureLayer) ++numFailed;
			glGenFramebuffers = reinterpret_cast<PFNGLGENFRAMEBUFFERS>(IntGetProcAddress("glGenFramebuffers"));
			if(!glGenFramebuffers) ++numFailed;
			glGenRenderbuffers = reinterpret_cast<PFNGLGENRENDERBUFFERS>(IntGetProcAddress("glGenRenderbuffers"));
			if(!glGenRenderbuffers) ++numFailed;
			glGenVertexArrays = reinterpret_cast<PFNGLGENVERTEXARRAYS>(IntGetProcAddress("glGenVertexArrays"));
			if(!glGenVertexArrays) ++numFailed;
			glGenerateMipmap = reinterpret_cast<PFNGLGENERATEMIPMAP>(IntGetProcAddress("glGenerateMipmap"));
			if(!glGenerateMipmap) ++numFailed;
			glGetBooleani_v = reinterpret_cast<PFNGLGETBOOLEANI_V>(IntGetProcAddress("glGetBooleani_v"));
			if(!glGetBooleani_v) ++numFailed;
			glGetFragDataLocation = reinterpret_cast<PFNGLGETFRAGDATALOCATION>(IntGetProcAddress("glGetFragDataLocation"));
			if(!glGetFragDataLocation) ++numFailed;
			glGetFramebufferAttachmentParameteriv = reinterpret_cast<PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIV>(IntGetProcAddress("glGetFramebufferAttachmentParameteriv"));
			if(!glGetFramebufferAttachmentParameteriv) ++numFailed;
			glGetIntegeri_v = reinterpret_cast<PFNGLGETINTEGERI_V>(IntGetProcAddress("glGetIntegeri_v"));
			if(!glGetIntegeri_v) ++numFailed;
			glGetRenderbufferParameteriv = reinterpret_cast<PFNGLGETRENDERBUFFERPARAMETERIV>(IntGetProcAddress("glGetRenderbufferParameteriv"));
			if(!glGetRenderbufferParameteriv) ++numFailed;
			glGetStringi = reinterpret_cast<PFNGLGETSTRINGI>(IntGetProcAddress("glGetStringi"));
			if(!glGetStringi) ++numFailed;
			glGetTexParameterIiv = reinterpret_cast<PFNGLGETTEXPARAMETERIIV>(IntGetProcAddress("glGetTexParameterIiv"));
			if(!glGetTexParameterIiv) ++numFailed;
			glGetTexParameterIuiv = reinterpret_cast<PFNGLGETTEXPARAMETERIUIV>(IntGetProcAddress("glGetTexParameterIuiv"));
			if(!glGetTexParameterIuiv) ++numFailed;
			glGetTransformFeedbackVarying = reinterpret_cast<PFNGLGETTRANSFORMFEEDBACKVARYING>(IntGetProcAddress("glGetTransformFeedbackVarying"));
			if(!glGetTransformFeedbackVarying) ++numFailed;
			glGetUniformuiv = reinterpret_cast<PFNGLGETUNIFORMUIV>(IntGetProcAddress("glGetUniformuiv"));
			if(!glGetUniformuiv) ++numFailed;
			glGetVertexAttribIiv = reinterpret_cast<PFNGLGETVERTEXATTRIBIIV>(IntGetProcAddress("glGetVertexAttribIiv"));
			if(!glGetVertexAttribIiv) ++numFailed;
			glGetVertexAttribIuiv = reinterpret_cast<PFNGLGETVERTEXATTRIBIUIV>(IntGetProcAddress("glGetVertexAttribIuiv"));
			if(!glGetVertexAttribIuiv) ++numFailed;
			glIsEnabledi = reinterpret_cast<PFNGLISENABLEDI>(IntGetProcAddress("glIsEnabledi"));
			if(!glIsEnabledi) ++numFailed;
			glIsFramebuffer = reinterpret_cast<PFNGLISFRAMEBUFFER>(IntGetProcAddress("glIsFramebuffer"));
			if(!glIsFramebuffer) ++numFailed;
			glIsRenderbuffer = reinterpret_cast<PFNGLISRENDERBUFFER>(IntGetProcAddress("glIsRenderbuffer"));
			if(!glIsRenderbuffer) ++numFailed;
			glIsVertexArray = reinterpret_cast<PFNGLISVERTEXARRAY>(IntGetProcAddress("glIsVertexArray"));
			if(!glIsVertexArray) ++numFailed;
			glMapBufferRange = reinterpret_cast<PFNGLMAPBUFFERRANGE>(IntGetProcAddress("glMapBufferRange"));
			if(!glMapBufferRange) ++numFailed;
			glRenderbufferStorage = reinterpret_cast<PFNGLRENDERBUFFERSTORAGE>(IntGetProcAddress("glRenderbufferStorage"));
			if(!glRenderbufferStorage) ++numFailed;
			glRenderbufferStorageMultisample = reinterpret_cast<PFNGLRENDERBUFFERSTORAGEMULTISAMPLE>(IntGetProcAddress("glRenderbufferStorageMultisample"));
			if(!glRenderbufferStorageMultisample) ++numFailed;
			glTexParameterIiv = reinterpret_cast<PFNGLTEXPARAMETERIIV>(IntGetProcAddress("glTexParameterIiv"));
			if(!glTexParameterIiv) ++numFailed;
			glTexParameterIuiv = reinterpret_cast<PFNGLTEXPARAMETERIUIV>(IntGetProcAddress("glTexParameterIuiv"));
			if(!glTexParameterIuiv) ++numFailed;
			glTransformFeedbackVaryings = reinterpret_cast<PFNGLTRANSFORMFEEDBACKVARYINGS>(IntGetProcAddress("glTransformFeedbackVaryings"));
			if(!glTransformFeedbackVaryings) ++numFailed;
			glUniform1ui = reinterpret_cast<PFNGLUNIFORM1UI>(IntGetProcAddress("glUniform1ui"));
			if(!glUniform1ui) ++numFailed;
			glUniform1uiv = reinterpret_cast<PFNGLUNIFORM1UIV>(IntGetProcAddress("glUniform1uiv"));
			if(!glUniform1uiv) ++numFailed;
			glUniform2ui = reinterpret_cast<PFNGLUNIFORM2UI>(IntGetProcAddress("glUniform2ui"));
			if(!glUniform2ui) ++numFailed;
			glUniform2uiv = reinterpret_cast<PFNGLUNIFORM2UIV>(IntGetProcAddress("glUniform2uiv"));
			if(!glUniform2uiv) ++numFailed;
			glUniform3ui = reinterpret_cast<PFNGLUNIFORM3UI>(IntGetProcAddress("glUniform3ui"));
			if(!glUniform3ui) ++numFailed;
			glUniform3uiv = reinterpret_cast<PFNGLUNIFORM3UIV>(IntGetProcAddress("glUniform3uiv"));
			if(!glUniform3uiv) ++numFailed;
			glUniform4ui = reinterpret_cast<PFNGLUNIFORM4UI>(IntGetProcAddress("glUniform4ui"));
			if(!glUniform4ui) ++numFailed;
			glUniform4uiv = reinterpret_cast<PFNGLUNIFORM4UIV>(IntGetProcAddress("glUniform4uiv"));
			if(!glUniform4uiv) ++numFailed;
			glVertexAttribI1i = reinterpret_cast<PFNGLVERTEXATTRIBI1I>(IntGetProcAddress("glVertexAttribI1i"));
			if(!glVertexAttribI1i) ++numFailed;
			glVertexAttribI1iv = reinterpret_cast<PFNGLVERTEXATTRIBI1IV>(IntGetProcAddress("glVertexAttribI1iv"));
			if(!glVertexAttribI1iv) ++numFailed;
			glVertexAttribI1ui = reinterpret_cast<PFNGLVERTEXATTRIBI1UI>(IntGetProcAddress("glVertexAttribI1ui"));
			if(!glVertexAttribI1ui) ++numFailed;
			glVertexAttribI1uiv = reinterpret_cast<PFNGLVERTEXATTRIBI1UIV>(IntGetProcAddress("glVertexAttribI1uiv"));
			if(!glVertexAttribI1uiv) ++numFailed;
			glVertexAttribI2i = reinterpret_cast<PFNGLVERTEXATTRIBI2I>(IntGetProcAddress("glVertexAttribI2i"));
			if(!glVertexAttribI2i) ++numFailed;
			glVertexAttribI2iv = reinterpret_cast<PFNGLVERTEXATTRIBI2IV>(IntGetProcAddress("glVertexAttribI2iv"));
			if(!glVertexAttribI2iv) ++numFailed;
			glVertexAttribI2ui = reinterpret_cast<PFNGLVERTEXATTRIBI2UI>(IntGetProcAddress("glVertexAttribI2ui"));
			if(!glVertexAttribI2ui) ++numFailed;
			glVertexAttribI2uiv = reinterpret_cast<PFNGLVERTEXATTRIBI2UIV>(IntGetProcAddress("glVertexAttribI2uiv"));
			if(!glVertexAttribI2uiv) ++numFailed;
			glVertexAttribI3i = reinterpret_cast<PFNGLVERTEXATTRIBI3I>(IntGetProcAddress("glVertexAttribI3i"));
			if(!glVertexAttribI3i) ++numFailed;
			glVertexAttribI3iv = reinterpret_cast<PFNGLVERTEXATTRIBI3IV>(IntGetProcAddress("glVertexAttribI3iv"));
			if(!glVertexAttribI3iv) ++numFailed;
			glVertexAttribI3ui = reinterpret_cast<PFNGLVERTEXATTRIBI3UI>(IntGetProcAddress("glVertexAttribI3ui"));
			if(!glVertexAttribI3ui) ++numFailed;
			glVertexAttribI3uiv = reinterpret_cast<PFNGLVERTEXATTRIBI3UIV>(IntGetProcAddress("glVertexAttribI3uiv"));
			if(!glVertexAttribI3uiv) ++numFailed;
			glVertexAttribI4bv = reinterpret_cast<PFNGLVERTEXATTRIBI4BV>(IntGetProcAddress("glVertexAttribI4bv"));
			if(!glVertexAttribI4bv) ++numFailed;
			glVertexAttribI4i = reinterpret_cast<PFNGLVERTEXATTRIBI4I>(IntGetProcAddress("glVertexAttribI4i"));
			if(!glVertexAttribI4i) ++numFailed;
			glVertexAttribI4iv = reinterpret_cast<PFNGLVERTEXATTRIBI4IV>(IntGetProcAddress("glVertexAttribI4iv"));
			if(!glVertexAttribI4iv) ++numFailed;
			glVertexAttribI4sv = reinterpret_cast<PFNGLVERTEXATTRIBI4SV>(IntGetProcAddress("glVertexAttribI4sv"));
			if(!glVertexAttribI4sv) ++numFailed;
			glVertexAttribI4ubv = reinterpret_cast<PFNGLVERTEXATTRIBI4UBV>(IntGetProcAddress("glVertexAttribI4ubv"));
			if(!glVertexAttribI4ubv) ++numFailed;
			glVertexAttribI4ui = reinterpret_cast<PFNGLVERTEXATTRIBI4UI>(IntGetProcAddress("glVertexAttribI4ui"));
			if(!glVertexAttribI4ui) ++numFailed;
			glVertexAttribI4uiv = reinterpret_cast<PFNGLVERTEXATTRIBI4UIV>(IntGetProcAddress("glVertexAttribI4uiv"));
			if(!glVertexAttribI4uiv) ++numFailed;
			glVertexAttribI4usv = reinterpret_cast<PFNGLVERTEXATTRIBI4USV>(IntGetProcAddress("glVertexAttribI4usv"));
			if(!glVertexAttribI4usv) ++numFailed;
			glVertexAttribIPointer = reinterpret_cast<PFNGLVERTEXATTRIBIPOINTER>(IntGetProcAddress("glVertexAttribIPointer"));
			if(!glVertexAttribIPointer) ++numFailed;
			glCopyBufferSubData = reinterpret_cast<PFNGLCOPYBUFFERSUBDATA>(IntGetProcAddress("glCopyBufferSubData"));
			if(!glCopyBufferSubData) ++numFailed;
			glDrawArraysInstanced = reinterpret_cast<PFNGLDRAWARRAYSINSTANCED>(IntGetProcAddress("glDrawArraysInstanced"));
			if(!glDrawArraysInstanced) ++numFailed;
			glDrawElementsInstanced = reinterpret_cast<PFNGLDRAWELEMENTSINSTANCED>(IntGetProcAddress("glDrawElementsInstanced"));
			if(!glDrawElementsInstanced) ++numFailed;
			glGetActiveUniformBlockName = reinterpret_cast<PFNGLGETACTIVEUNIFORMBLOCKNAME>(IntGetProcAddress("glGetActiveUniformBlockName"));
			if(!glGetActiveUniformBlockName) ++numFailed;
			glGetActiveUniformBlockiv = reinterpret_cast<PFNGLGETACTIVEUNIFORMBLOCKIV>(IntGetProcAddress("glGetActiveUniformBlockiv"));
			if(!glGetActiveUniformBlockiv) ++numFailed;
			glGetActiveUniformName = reinterpret_cast<PFNGLGETACTIVEUNIFORMNAME>(IntGetProcAddress("glGetActiveUniformName"));
			if(!glGetActiveUniformName) ++numFailed;
			glGetActiveUniformsiv = reinterpret_cast<PFNGLGETACTIVEUNIFORMSIV>(IntGetProcAddress("glGetActiveUniformsiv"));
			if(!glGetActiveUniformsiv) ++numFailed;
			glGetUniformBlockIndex = reinterpret_cast<PFNGLGETUNIFORMBLOCKINDEX>(IntGetProcAddress("glGetUniformBlockIndex"));
			if(!glGetUniformBlockIndex) ++numFailed;
			glGetUniformIndices = reinterpret_cast<PFNGLGETUNIFORMINDICES>(IntGetProcAddress("glGetUniformIndices"));
			if(!glGetUniformIndices) ++numFailed;
			glPrimitiveRestartIndex = reinterpret_cast<PFNGLPRIMITIVERESTARTINDEX>(IntGetProcAddress("glPrimitiveRestartIndex"));
			if(!glPrimitiveRestartIndex) ++numFailed;
			glTexBuffer = reinterpret_cast<PFNGLTEXBUFFER>(IntGetProcAddress("glTexBuffer"));
			if(!glTexBuffer) ++numFailed;
			glUniformBlockBinding = reinterpret_cast<PFNGLUNIFORMBLOCKBINDING>(IntGetProcAddress("glUniformBlockBinding"));
			if(!glUniformBlockBinding) ++numFailed;
			glClientWaitSync = reinterpret_cast<PFNGLCLIENTWAITSYNC>(IntGetProcAddress("glClientWaitSync"));
			if(!glClientWaitSync) ++numFailed;
			glDeleteSync = reinterpret_cast<PFNGLDELETESYNC>(IntGetProcAddress("glDeleteSync"));
			if(!glDeleteSync) ++numFailed;
			glDrawElementsBaseVertex = reinterpret_cast<PFNGLDRAWELEMENTSBASEVERTEX>(IntGetProcAddress("glDrawElementsBaseVertex"));
			if(!glDrawElementsBaseVertex) ++numFailed;
			glDrawElementsInstancedBaseVertex = reinterpret_cast<PFNGLDRAWELEMENTSINSTANCEDBASEVERTEX>(IntGetProcAddress("glDrawElementsInstancedBaseVertex"));
			if(!glDrawElementsInstancedBaseVertex) ++numFailed;
			glDrawRangeElementsBaseVertex = reinterpret_cast<PFNGLDRAWRANGEELEMENTSBASEVERTEX>(IntGetProcAddress("glDrawRangeElementsBaseVertex"));
			if(!glDrawRangeElementsBaseVertex) ++numFailed;
			glFenceSync = reinterpret_cast<PFNGLFENCESYNC>(IntGetProcAddress("glFenceSync"));
			if(!glFenceSync) ++numFailed;
			glFramebufferTexture = reinterpret_cast<PFNGLFRAMEBUFFERTEXTURE>(IntGetProcAddress("glFramebufferTexture"));
			if(!glFramebufferTexture) ++numFailed;
			glGetBufferParameteri64v = reinterpret_cast<PFNGLGETBUFFERPARAMETERI64V>(IntGetProcAddress("glGetBufferParameteri64v"));
			if(!glGetBufferParameteri64v) ++numFailed;
			glGetInteger64i_v = reinterpret_cast<PFNGLGETINTEGER64I_V>(IntGetProcAddress("glGetInteger64i_v"));
			if(!glGetInteger64i_v) ++numFailed;
			glGetInteger64v = reinterpret_cast<PFNGLGETINTEGER64V>(IntGetProcAddress("glGetInteger64v"));
			if(!glGetInteger64v) ++numFailed;
			glGetMultisamplefv = reinterpret_cast<PFNGLGETMULTISAMPLEFV>(IntGetProcAddress("glGetMultisamplefv"));
			if(!glGetMultisamplefv) ++numFailed;
			glGetSynciv = reinterpret_cast<PFNGLGETSYNCIV>(IntGetProcAddress("glGetSynciv"));
			if(!glGetSynciv) ++numFailed;
			glIsSync = reinterpret_cast<PFNGLISSYNC>(IntGetProcAddress("glIsSync"));
			if(!glIsSync) ++numFailed;
			glMultiDrawElementsBaseVertex = reinterpret_cast<PFNGLMULTIDRAWELEMENTSBASEVERTEX>(IntGetProcAddress("glMultiDrawElementsBaseVertex"));
			if(!glMultiDrawElementsBaseVertex) ++numFailed;
			glProvokingVertex = reinterpret_cast<PFNGLPROVOKINGVERTEX>(IntGetProcAddress("glProvokingVertex"));
			if(!glProvokingVertex) ++numFailed;
			glSampleMaski = reinterpret_cast<PFNGLSAMPLEMASKI>(IntGetProcAddress("glSampleMaski"));
			if(!glSampleMaski) ++numFailed;
			glTexImage2DMultisample = reinterpret_cast<PFNGLTEXIMAGE2DMULTISAMPLE>(IntGetProcAddress("glTexImage2DMultisample"));
			if(!glTexImage2DMultisample) ++numFailed;
			glTexImage3DMultisample = reinterpret_cast<PFNGLTEXIMAGE3DMULTISAMPLE>(IntGetProcAddress("glTexImage3DMultisample"));
			if(!glTexImage3DMultisample) ++numFailed;
			glWaitSync = reinterpret_cast<PFNGLWAITSYNC>(IntGetProcAddress("glWaitSync"));
			if(!glWaitSync) ++numFailed;
			glBindFragDataLocationIndexed = reinterpret_cast<PFNGLBINDFRAGDATALOCATIONINDEXED>(IntGetProcAddress("glBindFragDataLocationIndexed"));
			if(!glBindFragDataLocationIndexed) ++numFailed;
			glBindSampler = reinterpret_cast<PFNGLBINDSAMPLER>(IntGetProcAddress("glBindSampler"));
			if(!glBindSampler) ++numFailed;
			glDeleteSamplers = reinterpret_cast<PFNGLDELETESAMPLERS>(IntGetProcAddress("glDeleteSamplers"));
			if(!glDeleteSamplers) ++numFailed;
			glGenSamplers = reinterpret_cast<PFNGLGENSAMPLERS>(IntGetProcAddress("glGenSamplers"));
			if(!glGenSamplers) ++numFailed;
			glGetFragDataIndex = reinterpret_cast<PFNGLGETFRAGDATAINDEX>(IntGetProcAddress("glGetFragDataIndex"));
			if(!glGetFragDataIndex) ++numFailed;
			glGetQueryObjecti64v = reinterpret_cast<PFNGLGETQUERYOBJECTI64V>(IntGetProcAddress("glGetQueryObjecti64v"));
			if(!glGetQueryObjecti64v) ++numFailed;
			glGetQueryObjectui64v = reinterpret_cast<PFNGLGETQUERYOBJECTUI64V>(IntGetProcAddress("glGetQueryObjectui64v"));
			if(!glGetQueryObjectui64v) ++numFailed;
			glGetSamplerParameterIiv = reinterpret_cast<PFNGLGETSAMPLERPARAMETERIIV>(IntGetProcAddress("glGetSamplerParameterIiv"));
			if(!glGetSamplerParameterIiv) ++numFailed;
			glGetSamplerParameterIuiv = reinterpret_cast<PFNGLGETSAMPLERPARAMETERIUIV>(IntGetProcAddress("glGetSamplerParameterIuiv"));
			if(!glGetSamplerParameterIuiv) ++numFailed;
			glGetSamplerParameterfv = reinterpret_cast<PFNGLGETSAMPLERPARAMETERFV>(IntGetProcAddress("glGetSamplerParameterfv"));
			if(!glGetSamplerParameterfv) ++numFailed;
			glGetSamplerParameteriv = reinterpret_cast<PFNGLGETSAMPLERPARAMETERIV>(IntGetProcAddress("glGetSamplerParameteriv"));
			if(!glGetSamplerParameteriv) ++numFailed;
			glIsSampler = reinterpret_cast<PFNGLISSAMPLER>(IntGetProcAddress("glIsSampler"));
			if(!glIsSampler) ++numFailed;
			glQueryCounter = reinterpret_cast<PFNGLQUERYCOUNTER>(IntGetProcAddress("glQueryCounter"));
			if(!glQueryCounter) ++numFailed;
			glSamplerParameterIiv = reinterpret_cast<PFNGLSAMPLERPARAMETERIIV>(IntGetProcAddress("glSamplerParameterIiv"));
			if(!glSamplerParameterIiv) ++numFailed;
			glSamplerParameterIuiv = reinterpret_cast<PFNGLSAMPLERPARAMETERIUIV>(IntGetProcAddress("glSamplerParameterIuiv"));
			if(!glSamplerParameterIuiv) ++numFailed;
			glSamplerParameterf = reinterpret_cast<PFNGLSAMPLERPARAMETERF>(IntGetProcAddress("glSamplerParameterf"));
			if(!glSamplerParameterf) ++numFailed;
			glSamplerParameterfv = reinterpret_cast<PFNGLSAMPLERPARAMETERFV>(IntGetProcAddress("glSamplerParameterfv"));
			if(!glSamplerParameterfv) ++numFailed;
			glSamplerParameteri = reinterpret_cast<PFNGLSAMPLERPARAMETERI>(IntGetProcAddress("glSamplerParameteri"));
			if(!glSamplerParameteri) ++numFailed;
			glSamplerParameteriv = reinterpret_cast<PFNGLSAMPLERPARAMETERIV>(IntGetProcAddress("glSamplerParameteriv"));
			if(!glSamplerParameteriv) ++numFailed;
			glVertexAttribDivisor = reinterpret_cast<PFNGLVERTEXATTRIBDIVISOR>(IntGetProcAddress("glVertexAttribDivisor"));
			if(!glVertexAttribDivisor) ++numFailed;
			glVertexAttribP1ui = reinterpret_cast<PFNGLVERTEXATTRIBP1UI>(IntGetProcAddress("glVertexAttribP1ui"));
			if(!glVertexAttribP1ui) ++numFailed;
			glVertexAttribP1uiv = reinterpret_cast<PFNGLVERTEXATTRIBP1UIV>(IntGetProcAddress("glVertexAttribP1uiv"));
			if(!glVertexAttribP1uiv) ++numFailed;
			glVertexAttribP2ui = reinterpret_cast<PFNGLVERTEXATTRIBP2UI>(IntGetProcAddress("glVertexAttribP2ui"));
			if(!glVertexAttribP2ui) ++numFailed;
			glVertexAttribP2uiv = reinterpret_cast<PFNGLVERTEXATTRIBP2UIV>(IntGetProcAddress("glVertexAttribP2uiv"));
			if(!glVertexAttribP2uiv) ++numFailed;
			glVertexAttribP3ui = reinterpret_cast<PFNGLVERTEXATTRIBP3UI>(IntGetProcAddress("glVertexAttribP3ui"));
			if(!glVertexAttribP3ui) ++numFailed;
			glVertexAttribP3uiv = reinterpret_cast<PFNGLVERTEXATTRIBP3UIV>(IntGetProcAddress("glVertexAttribP3uiv"));
			if(!glVertexAttribP3uiv) ++numFailed;
			glVertexAttribP4ui = reinterpret_cast<PFNGLVERTEXATTRIBP4UI>(IntGetProcAddress("glVertexAttribP4ui"));
			if(!glVertexAttribP4ui) ++numFailed;
			glVertexAttribP4uiv = reinterpret_cast<PFNGLVERTEXATTRIBP4UIV>(IntGetProcAddress("glVertexAttribP4uiv"));
			if(!glVertexAttribP4uiv) ++numFailed;
			return numFailed;
		}
		
		namespace sys
		{
			namespace 
			{
				typedef int (*PFN_LOADEXTENSION)();
				struct MapEntry
				{
					MapEntry(const char *_extName, exts::LoadTest *_extVariable)
						: extName(_extName)
						, extVariable(_extVariable)
						, loaderFunc(0)
						{}
						
					MapEntry(const char *_extName, exts::LoadTest *_extVariable, PFN_LOADEXTENSION _loaderFunc)
						: extName(_extName)
						, extVariable(_extVariable)
						, loaderFunc(_loaderFunc)
						{}
					
					const char *extName;
					exts::LoadTest *extVariable;
					PFN_LOADEXTENSION loaderFunc;
				};
				
				struct MapCompare
				{
					MapCompare(const char *test_) : test(test_) {}
					bool operator()(const MapEntry &other) { return strcmp(test, other.extName) == 0; }
					const char *test;
				};
				
				void InitializeMappingTable(std::vector<MapEntry> &table)
				{
					table.reserve(52);
					table.push_back(MapEntry("GL_EXT_texture_compression_s3tc", &exts::var_EXT_texture_compression_s3tc));
					table.push_back(MapEntry("GL_EXT_texture_sRGB", &exts::var_EXT_texture_sRGB));
					table.push_back(MapEntry("GL_EXT_texture_filter_anisotropic", &exts::var_EXT_texture_filter_anisotropic));
					table.push_back(MapEntry("GL_ARB_compressed_texture_pixel_storage", &exts::var_ARB_compressed_texture_pixel_storage));
					table.push_back(MapEntry("GL_ARB_conservative_depth", &exts::var_ARB_conservative_depth));
					table.push_back(MapEntry("GL_ARB_ES2_compatibility", &exts::var_ARB_ES2_compatibility, Load_ARB_ES2_compatibility));
					table.push_back(MapEntry("GL_ARB_get_program_binary", &exts::var_ARB_get_program_binary, Load_ARB_get_program_binary));
					table.push_back(MapEntry("GL_ARB_explicit_uniform_location", &exts::var_ARB_explicit_uniform_location));
					table.push_back(MapEntry("GL_ARB_internalformat_query", &exts::var_ARB_internalformat_query, Load_ARB_internalformat_query));
					table.push_back(MapEntry("GL_ARB_internalformat_query2", &exts::var_ARB_internalformat_query2, Load_ARB_internalformat_query2));
					table.push_back(MapEntry("GL_ARB_map_buffer_alignment", &exts::var_ARB_map_buffer_alignment));
					table.push_back(MapEntry("GL_ARB_program_interface_query", &exts::var_ARB_program_interface_query, Load_ARB_program_interface_query));
					table.push_back(MapEntry("GL_ARB_separate_shader_objects", &exts::var_ARB_separate_shader_objects, Load_ARB_separate_shader_objects));
					table.push_back(MapEntry("GL_ARB_shading_language_420pack", &exts::var_ARB_shading_language_420pack));
					table.push_back(MapEntry("GL_ARB_shading_language_packing", &exts::var_ARB_shading_language_packing));
					table.push_back(MapEntry("GL_ARB_texture_buffer_range", &exts::var_ARB_texture_buffer_range, Load_ARB_texture_buffer_range));
					table.push_back(MapEntry("GL_ARB_texture_storage", &exts::var_ARB_texture_storage, Load_ARB_texture_storage));
					table.push_back(MapEntry("GL_ARB_texture_view", &exts::var_ARB_texture_view, Load_ARB_texture_view));
					table.push_back(MapEntry("GL_ARB_vertex_attrib_binding", &exts::var_ARB_vertex_attrib_binding, Load_ARB_vertex_attrib_binding));
					table.push_back(MapEntry("GL_ARB_viewport_array", &exts::var_ARB_viewport_array, Load_ARB_viewport_array));
					table.push_back(MapEntry("GL_ARB_arrays_of_arrays", &exts::var_ARB_arrays_of_arrays));
					table.push_back(MapEntry("GL_ARB_clear_buffer_object", &exts::var_ARB_clear_buffer_object, Load_ARB_clear_buffer_object));
					table.push_back(MapEntry("GL_ARB_copy_image", &exts::var_ARB_copy_image, Load_ARB_copy_image));
					table.push_back(MapEntry("GL_ARB_ES3_compatibility", &exts::var_ARB_ES3_compatibility));
					table.push_back(MapEntry("GL_ARB_fragment_layer_viewport", &exts::var_ARB_fragment_layer_viewport));
					table.push_back(MapEntry("GL_ARB_framebuffer_no_attachments", &exts::var_ARB_framebuffer_no_attachments, Load_ARB_framebuffer_no_attachments));
					table.push_back(MapEntry("GL_ARB_invalidate_subdata", &exts::var_ARB_invalidate_subdata, Load_ARB_invalidate_subdata));
					table.push_back(MapEntry("GL_ARB_robust_buffer_access_behavior", &exts::var_ARB_robust_buffer_access_behavior));
					table.push_back(MapEntry("GL_ARB_stencil_texturing", &exts::var_ARB_stencil_texturing));
					table.push_back(MapEntry("GL_ARB_texture_query_levels", &exts::var_ARB_texture_query_levels));
					table.push_back(MapEntry("GL_ARB_texture_storage_multisample", &exts::var_ARB_texture_storage_multisample, Load_ARB_texture_storage_multisample));
					table.push_back(MapEntry("GL_KHR_debug", &exts::var_KHR_debug, Load_KHR_debug));
					table.push_back(MapEntry("GL_ARB_buffer_storage", &exts::var_ARB_buffer_storage, Load_ARB_buffer_storage));
					table.push_back(MapEntry("GL_ARB_clear_texture", &exts::var_ARB_clear_texture, Load_ARB_clear_texture));
					table.push_back(MapEntry("GL_ARB_enhanced_layouts", &exts::var_ARB_enhanced_layouts));
					table.push_back(MapEntry("GL_ARB_multi_bind", &exts::var_ARB_multi_bind, Load_ARB_multi_bind));
					table.push_back(MapEntry("GL_ARB_query_buffer_object", &exts::var_ARB_query_buffer_object));
					table.push_back(MapEntry("GL_ARB_texture_mirror_clamp_to_edge", &exts::var_ARB_texture_mirror_clamp_to_edge));
					table.push_back(MapEntry("GL_ARB_texture_stencil8", &exts::var_ARB_texture_stencil8));
					table.push_back(MapEntry("GL_ARB_vertex_type_10f_11f_11f_rev", &exts::var_ARB_vertex_type_10f_11f_11f_rev));
					table.push_back(MapEntry("GL_ARB_seamless_cubemap_per_texture", &exts::var_ARB_seamless_cubemap_per_texture));
					table.push_back(MapEntry("GL_ARB_clip_control", &exts::var_ARB_clip_control, Load_ARB_clip_control));
					table.push_back(MapEntry("GL_ARB_conditional_render_inverted", &exts::var_ARB_conditional_render_inverted));
					table.push_back(MapEntry("GL_ARB_cull_distance", &exts::var_ARB_cull_distance));
					table.push_back(MapEntry("GL_ARB_derivative_control", &exts::var_ARB_derivative_control));
					table.push_back(MapEntry("GL_ARB_direct_state_access", &exts::var_ARB_direct_state_access, Load_ARB_direct_state_access));
					table.push_back(MapEntry("GL_ARB_get_texture_sub_image", &exts::var_ARB_get_texture_sub_image, Load_ARB_get_texture_sub_image));
					table.push_back(MapEntry("GL_ARB_shader_texture_image_samples", &exts::var_ARB_shader_texture_image_samples));
					table.push_back(MapEntry("GL_ARB_texture_barrier", &exts::var_ARB_texture_barrier, Load_ARB_texture_barrier));
					table.push_back(MapEntry("GL_KHR_context_flush_control", &exts::var_KHR_context_flush_control));
					table.push_back(MapEntry("GL_KHR_robust_buffer_access_behavior", &exts::var_KHR_robust_buffer_access_behavior));
					table.push_back(MapEntry("GL_KHR_robustness", &exts::var_KHR_robustness, Load_KHR_robustness));
				}
				
				void ClearExtensionVars()
				{
					exts::var_EXT_texture_compression_s3tc = exts::LoadTest();
					exts::var_EXT_texture_sRGB = exts::LoadTest();
					exts::var_EXT_texture_filter_anisotropic = exts::LoadTest();
					exts::var_ARB_compressed_texture_pixel_storage = exts::LoadTest();
					exts::var_ARB_conservative_depth = exts::LoadTest();
					exts::var_ARB_ES2_compatibility = exts::LoadTest();
					exts::var_ARB_get_program_binary = exts::LoadTest();
					exts::var_ARB_explicit_uniform_location = exts::LoadTest();
					exts::var_ARB_internalformat_query = exts::LoadTest();
					exts::var_ARB_internalformat_query2 = exts::LoadTest();
					exts::var_ARB_map_buffer_alignment = exts::LoadTest();
					exts::var_ARB_program_interface_query = exts::LoadTest();
					exts::var_ARB_separate_shader_objects = exts::LoadTest();
					exts::var_ARB_shading_language_420pack = exts::LoadTest();
					exts::var_ARB_shading_language_packing = exts::LoadTest();
					exts::var_ARB_texture_buffer_range = exts::LoadTest();
					exts::var_ARB_texture_storage = exts::LoadTest();
					exts::var_ARB_texture_view = exts::LoadTest();
					exts::var_ARB_vertex_attrib_binding = exts::LoadTest();
					exts::var_ARB_viewport_array = exts::LoadTest();
					exts::var_ARB_arrays_of_arrays = exts::LoadTest();
					exts::var_ARB_clear_buffer_object = exts::LoadTest();
					exts::var_ARB_copy_image = exts::LoadTest();
					exts::var_ARB_ES3_compatibility = exts::LoadTest();
					exts::var_ARB_fragment_layer_viewport = exts::LoadTest();
					exts::var_ARB_framebuffer_no_attachments = exts::LoadTest();
					exts::var_ARB_invalidate_subdata = exts::LoadTest();
					exts::var_ARB_robust_buffer_access_behavior = exts::LoadTest();
					exts::var_ARB_stencil_texturing = exts::LoadTest();
					exts::var_ARB_texture_query_levels = exts::LoadTest();
					exts::var_ARB_texture_storage_multisample = exts::LoadTest();
					exts::var_KHR_debug = exts::LoadTest();
					exts::var_ARB_buffer_storage = exts::LoadTest();
					exts::var_ARB_clear_texture = exts::LoadTest();
					exts::var_ARB_enhanced_layouts = exts::LoadTest();
					exts::var_ARB_multi_bind = exts::LoadTest();
					exts::var_ARB_query_buffer_object = exts::LoadTest();
					exts::var_ARB_texture_mirror_clamp_to_edge = exts::LoadTest();
					exts::var_ARB_texture_stencil8 = exts::LoadTest();
					exts::var_ARB_vertex_type_10f_11f_11f_rev = exts::LoadTest();
					exts::var_ARB_seamless_cubemap_per_texture = exts::LoadTest();
					exts::var_ARB_clip_control = exts::LoadTest();
					exts::var_ARB_conditional_render_inverted = exts::LoadTest();
					exts::var_ARB_cull_distance = exts::LoadTest();
					exts::var_ARB_derivative_control = exts::LoadTest();
					exts::var_ARB_direct_state_access = exts::LoadTest();
					exts::var_ARB_get_texture_sub_image = exts::LoadTest();
					exts::var_ARB_shader_texture_image_samples = exts::LoadTest();
					exts::var_ARB_texture_barrier = exts::LoadTest();
					exts::var_KHR_context_flush_control = exts::LoadTest();
					exts::var_KHR_robust_buffer_access_behavior = exts::LoadTest();
					exts::var_KHR_robustness = exts::LoadTest();
				}
				
				void LoadExtByName(std::vector<MapEntry> &table, const char *extensionName)
				{
					std::vector<MapEntry>::iterator entry = std::find_if(table.begin(), table.end(), MapCompare(extensionName));
					
					if(entry != table.end())
					{
						if(entry->loaderFunc)
							(*entry->extVariable) = exts::LoadTest(true, entry->loaderFunc());
						else
							(*entry->extVariable) = exts::LoadTest(true, 0);
					}
				}
			} //namespace 
			
			
			namespace 
			{
				static void ProcExtsFromExtList(std::vector<MapEntry> &table)
				{
					GLint iLoop;
					GLint iNumExtensions = 0;
					gl3x::gl::glGetIntegerv(gl3x::gl::GL_NUM_EXTENSIONS, &iNumExtensions);
				
					for(iLoop = 0; iLoop < iNumExtensions; iLoop++)
					{
						const char *strExtensionName = (const char *)gl3x::gl::glGetStringi(gl3x::gl::GL_EXTENSIONS, iLoop);
						LoadExtByName(table, strExtensionName);
					}
				}
				
			} //namespace 
			
			exts::LoadTest LoadFunctions()
			{
				ClearExtensionVars();
				std::vector<MapEntry> table;
				InitializeMappingTable(table);
				
				glGetIntegerv = reinterpret_cast<PFNGLGETINTEGERV>(IntGetProcAddress("glGetIntegerv"));
				if(!glGetIntegerv) return exts::LoadTest();
				glGetStringi = reinterpret_cast<PFNGLGETSTRINGI>(IntGetProcAddress("glGetStringi"));
				if(!glGetStringi) return exts::LoadTest();
				
				ProcExtsFromExtList(table);
				
				int numFailed = LoadCoreFunctions();
				return exts::LoadTest(true, numFailed);
			}
			
			static int g_major_version = 0;
			static int g_minor_version = 0;
			
			static void GetGLVersion()
			{
				glGetIntegerv(GL_MAJOR_VERSION, &g_major_version);
				glGetIntegerv(GL_MINOR_VERSION, &g_minor_version);
			}
			
			int GetMajorVersion()
			{
				if(g_major_version == 0)
					GetGLVersion();
				return g_major_version;
			}
			
			int GetMinorVersion()
			{
				if(g_major_version == 0) //Yes, check the major version to get the minor one.
					GetGLVersion();
				return g_minor_version;
			}
			
			bool IsVersionGEQ(int majorVersion, int minorVersion)
			{
				if(g_major_version == 0)
					GetGLVersion();
				
				if(majorVersion < g_major_version) return true;
				if(majorVersion > g_major_version) return false;
				if(minorVersion <= g_minor_version) return true;
				return false;
			}
			
		} //namespace sys
	} //namespace gl
} //namespace gl3x
