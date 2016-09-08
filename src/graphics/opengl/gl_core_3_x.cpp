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
	typedef void (CODEGEN_FUNCPTR *PFNCLEARDEPTHF)(GLfloat);
	PFNCLEARDEPTHF ClearDepthf = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDEPTHRANGEF)(GLfloat, GLfloat);
	PFNDEPTHRANGEF DepthRangef = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETSHADERPRECISIONFORMAT)(GLenum, GLenum, GLint *, GLint *);
	PFNGETSHADERPRECISIONFORMAT GetShaderPrecisionFormat = 0;
	typedef void (CODEGEN_FUNCPTR *PFNRELEASESHADERCOMPILER)(void);
	PFNRELEASESHADERCOMPILER ReleaseShaderCompiler = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSHADERBINARY)(GLsizei, const GLuint *, GLenum, const void *, GLsizei);
	PFNSHADERBINARY ShaderBinary = 0;
	
	static int Load_ARB_ES2_compatibility()
	{
		int numFailed = 0;
		ClearDepthf = reinterpret_cast<PFNCLEARDEPTHF>(IntGetProcAddress("glClearDepthf"));
		if(!ClearDepthf) ++numFailed;
		DepthRangef = reinterpret_cast<PFNDEPTHRANGEF>(IntGetProcAddress("glDepthRangef"));
		if(!DepthRangef) ++numFailed;
		GetShaderPrecisionFormat = reinterpret_cast<PFNGETSHADERPRECISIONFORMAT>(IntGetProcAddress("glGetShaderPrecisionFormat"));
		if(!GetShaderPrecisionFormat) ++numFailed;
		ReleaseShaderCompiler = reinterpret_cast<PFNRELEASESHADERCOMPILER>(IntGetProcAddress("glReleaseShaderCompiler"));
		if(!ReleaseShaderCompiler) ++numFailed;
		ShaderBinary = reinterpret_cast<PFNSHADERBINARY>(IntGetProcAddress("glShaderBinary"));
		if(!ShaderBinary) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNGETPROGRAMBINARY)(GLuint, GLsizei, GLsizei *, GLenum *, void *);
	PFNGETPROGRAMBINARY GetProgramBinary = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMBINARY)(GLuint, GLenum, const void *, GLsizei);
	PFNPROGRAMBINARY ProgramBinary = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMPARAMETERI)(GLuint, GLenum, GLint);
	PFNPROGRAMPARAMETERI ProgramParameteri = 0;
	
	static int Load_ARB_get_program_binary()
	{
		int numFailed = 0;
		GetProgramBinary = reinterpret_cast<PFNGETPROGRAMBINARY>(IntGetProcAddress("glGetProgramBinary"));
		if(!GetProgramBinary) ++numFailed;
		ProgramBinary = reinterpret_cast<PFNPROGRAMBINARY>(IntGetProcAddress("glProgramBinary"));
		if(!ProgramBinary) ++numFailed;
		ProgramParameteri = reinterpret_cast<PFNPROGRAMPARAMETERI>(IntGetProcAddress("glProgramParameteri"));
		if(!ProgramParameteri) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNGETINTERNALFORMATIV)(GLenum, GLenum, GLenum, GLsizei, GLint *);
	PFNGETINTERNALFORMATIV GetInternalformativ = 0;
	
	static int Load_ARB_internalformat_query()
	{
		int numFailed = 0;
		GetInternalformativ = reinterpret_cast<PFNGETINTERNALFORMATIV>(IntGetProcAddress("glGetInternalformativ"));
		if(!GetInternalformativ) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNGETINTERNALFORMATI64V)(GLenum, GLenum, GLenum, GLsizei, GLint64 *);
	PFNGETINTERNALFORMATI64V GetInternalformati64v = 0;
	
	static int Load_ARB_internalformat_query2()
	{
		int numFailed = 0;
		GetInternalformati64v = reinterpret_cast<PFNGETINTERNALFORMATI64V>(IntGetProcAddress("glGetInternalformati64v"));
		if(!GetInternalformati64v) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNGETPROGRAMINTERFACEIV)(GLuint, GLenum, GLenum, GLint *);
	PFNGETPROGRAMINTERFACEIV GetProgramInterfaceiv = 0;
	typedef GLuint (CODEGEN_FUNCPTR *PFNGETPROGRAMRESOURCEINDEX)(GLuint, GLenum, const GLchar *);
	PFNGETPROGRAMRESOURCEINDEX GetProgramResourceIndex = 0;
	typedef GLint (CODEGEN_FUNCPTR *PFNGETPROGRAMRESOURCELOCATION)(GLuint, GLenum, const GLchar *);
	PFNGETPROGRAMRESOURCELOCATION GetProgramResourceLocation = 0;
	typedef GLint (CODEGEN_FUNCPTR *PFNGETPROGRAMRESOURCELOCATIONINDEX)(GLuint, GLenum, const GLchar *);
	PFNGETPROGRAMRESOURCELOCATIONINDEX GetProgramResourceLocationIndex = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETPROGRAMRESOURCENAME)(GLuint, GLenum, GLuint, GLsizei, GLsizei *, GLchar *);
	PFNGETPROGRAMRESOURCENAME GetProgramResourceName = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETPROGRAMRESOURCEIV)(GLuint, GLenum, GLuint, GLsizei, const GLenum *, GLsizei, GLsizei *, GLint *);
	PFNGETPROGRAMRESOURCEIV GetProgramResourceiv = 0;
	
	static int Load_ARB_program_interface_query()
	{
		int numFailed = 0;
		GetProgramInterfaceiv = reinterpret_cast<PFNGETPROGRAMINTERFACEIV>(IntGetProcAddress("glGetProgramInterfaceiv"));
		if(!GetProgramInterfaceiv) ++numFailed;
		GetProgramResourceIndex = reinterpret_cast<PFNGETPROGRAMRESOURCEINDEX>(IntGetProcAddress("glGetProgramResourceIndex"));
		if(!GetProgramResourceIndex) ++numFailed;
		GetProgramResourceLocation = reinterpret_cast<PFNGETPROGRAMRESOURCELOCATION>(IntGetProcAddress("glGetProgramResourceLocation"));
		if(!GetProgramResourceLocation) ++numFailed;
		GetProgramResourceLocationIndex = reinterpret_cast<PFNGETPROGRAMRESOURCELOCATIONINDEX>(IntGetProcAddress("glGetProgramResourceLocationIndex"));
		if(!GetProgramResourceLocationIndex) ++numFailed;
		GetProgramResourceName = reinterpret_cast<PFNGETPROGRAMRESOURCENAME>(IntGetProcAddress("glGetProgramResourceName"));
		if(!GetProgramResourceName) ++numFailed;
		GetProgramResourceiv = reinterpret_cast<PFNGETPROGRAMRESOURCEIV>(IntGetProcAddress("glGetProgramResourceiv"));
		if(!GetProgramResourceiv) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNACTIVESHADERPROGRAM)(GLuint, GLuint);
	PFNACTIVESHADERPROGRAM ActiveShaderProgram = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBINDPROGRAMPIPELINE)(GLuint);
	PFNBINDPROGRAMPIPELINE BindProgramPipeline = 0;
	typedef GLuint (CODEGEN_FUNCPTR *PFNCREATESHADERPROGRAMV)(GLenum, GLsizei, const GLchar *const*);
	PFNCREATESHADERPROGRAMV CreateShaderProgramv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDELETEPROGRAMPIPELINES)(GLsizei, const GLuint *);
	PFNDELETEPROGRAMPIPELINES DeleteProgramPipelines = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGENPROGRAMPIPELINES)(GLsizei, GLuint *);
	PFNGENPROGRAMPIPELINES GenProgramPipelines = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETPROGRAMPIPELINEINFOLOG)(GLuint, GLsizei, GLsizei *, GLchar *);
	PFNGETPROGRAMPIPELINEINFOLOG GetProgramPipelineInfoLog = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETPROGRAMPIPELINEIV)(GLuint, GLenum, GLint *);
	PFNGETPROGRAMPIPELINEIV GetProgramPipelineiv = 0;
	typedef GLboolean (CODEGEN_FUNCPTR *PFNISPROGRAMPIPELINE)(GLuint);
	PFNISPROGRAMPIPELINE IsProgramPipeline = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM1D)(GLuint, GLint, GLdouble);
	PFNPROGRAMUNIFORM1D ProgramUniform1d = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM1DV)(GLuint, GLint, GLsizei, const GLdouble *);
	PFNPROGRAMUNIFORM1DV ProgramUniform1dv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM1F)(GLuint, GLint, GLfloat);
	PFNPROGRAMUNIFORM1F ProgramUniform1f = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM1FV)(GLuint, GLint, GLsizei, const GLfloat *);
	PFNPROGRAMUNIFORM1FV ProgramUniform1fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM1I)(GLuint, GLint, GLint);
	PFNPROGRAMUNIFORM1I ProgramUniform1i = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM1IV)(GLuint, GLint, GLsizei, const GLint *);
	PFNPROGRAMUNIFORM1IV ProgramUniform1iv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM1UI)(GLuint, GLint, GLuint);
	PFNPROGRAMUNIFORM1UI ProgramUniform1ui = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM1UIV)(GLuint, GLint, GLsizei, const GLuint *);
	PFNPROGRAMUNIFORM1UIV ProgramUniform1uiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM2D)(GLuint, GLint, GLdouble, GLdouble);
	PFNPROGRAMUNIFORM2D ProgramUniform2d = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM2DV)(GLuint, GLint, GLsizei, const GLdouble *);
	PFNPROGRAMUNIFORM2DV ProgramUniform2dv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM2F)(GLuint, GLint, GLfloat, GLfloat);
	PFNPROGRAMUNIFORM2F ProgramUniform2f = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM2FV)(GLuint, GLint, GLsizei, const GLfloat *);
	PFNPROGRAMUNIFORM2FV ProgramUniform2fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM2I)(GLuint, GLint, GLint, GLint);
	PFNPROGRAMUNIFORM2I ProgramUniform2i = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM2IV)(GLuint, GLint, GLsizei, const GLint *);
	PFNPROGRAMUNIFORM2IV ProgramUniform2iv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM2UI)(GLuint, GLint, GLuint, GLuint);
	PFNPROGRAMUNIFORM2UI ProgramUniform2ui = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM2UIV)(GLuint, GLint, GLsizei, const GLuint *);
	PFNPROGRAMUNIFORM2UIV ProgramUniform2uiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM3D)(GLuint, GLint, GLdouble, GLdouble, GLdouble);
	PFNPROGRAMUNIFORM3D ProgramUniform3d = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM3DV)(GLuint, GLint, GLsizei, const GLdouble *);
	PFNPROGRAMUNIFORM3DV ProgramUniform3dv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM3F)(GLuint, GLint, GLfloat, GLfloat, GLfloat);
	PFNPROGRAMUNIFORM3F ProgramUniform3f = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM3FV)(GLuint, GLint, GLsizei, const GLfloat *);
	PFNPROGRAMUNIFORM3FV ProgramUniform3fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM3I)(GLuint, GLint, GLint, GLint, GLint);
	PFNPROGRAMUNIFORM3I ProgramUniform3i = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM3IV)(GLuint, GLint, GLsizei, const GLint *);
	PFNPROGRAMUNIFORM3IV ProgramUniform3iv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM3UI)(GLuint, GLint, GLuint, GLuint, GLuint);
	PFNPROGRAMUNIFORM3UI ProgramUniform3ui = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM3UIV)(GLuint, GLint, GLsizei, const GLuint *);
	PFNPROGRAMUNIFORM3UIV ProgramUniform3uiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM4D)(GLuint, GLint, GLdouble, GLdouble, GLdouble, GLdouble);
	PFNPROGRAMUNIFORM4D ProgramUniform4d = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM4DV)(GLuint, GLint, GLsizei, const GLdouble *);
	PFNPROGRAMUNIFORM4DV ProgramUniform4dv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM4F)(GLuint, GLint, GLfloat, GLfloat, GLfloat, GLfloat);
	PFNPROGRAMUNIFORM4F ProgramUniform4f = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM4FV)(GLuint, GLint, GLsizei, const GLfloat *);
	PFNPROGRAMUNIFORM4FV ProgramUniform4fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM4I)(GLuint, GLint, GLint, GLint, GLint, GLint);
	PFNPROGRAMUNIFORM4I ProgramUniform4i = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM4IV)(GLuint, GLint, GLsizei, const GLint *);
	PFNPROGRAMUNIFORM4IV ProgramUniform4iv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM4UI)(GLuint, GLint, GLuint, GLuint, GLuint, GLuint);
	PFNPROGRAMUNIFORM4UI ProgramUniform4ui = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORM4UIV)(GLuint, GLint, GLsizei, const GLuint *);
	PFNPROGRAMUNIFORM4UIV ProgramUniform4uiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX2DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
	PFNPROGRAMUNIFORMMATRIX2DV ProgramUniformMatrix2dv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX2FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
	PFNPROGRAMUNIFORMMATRIX2FV ProgramUniformMatrix2fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX2X3DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
	PFNPROGRAMUNIFORMMATRIX2X3DV ProgramUniformMatrix2x3dv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX2X3FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
	PFNPROGRAMUNIFORMMATRIX2X3FV ProgramUniformMatrix2x3fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX2X4DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
	PFNPROGRAMUNIFORMMATRIX2X4DV ProgramUniformMatrix2x4dv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX2X4FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
	PFNPROGRAMUNIFORMMATRIX2X4FV ProgramUniformMatrix2x4fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX3DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
	PFNPROGRAMUNIFORMMATRIX3DV ProgramUniformMatrix3dv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX3FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
	PFNPROGRAMUNIFORMMATRIX3FV ProgramUniformMatrix3fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX3X2DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
	PFNPROGRAMUNIFORMMATRIX3X2DV ProgramUniformMatrix3x2dv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX3X2FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
	PFNPROGRAMUNIFORMMATRIX3X2FV ProgramUniformMatrix3x2fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX3X4DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
	PFNPROGRAMUNIFORMMATRIX3X4DV ProgramUniformMatrix3x4dv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX3X4FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
	PFNPROGRAMUNIFORMMATRIX3X4FV ProgramUniformMatrix3x4fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX4DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
	PFNPROGRAMUNIFORMMATRIX4DV ProgramUniformMatrix4dv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX4FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
	PFNPROGRAMUNIFORMMATRIX4FV ProgramUniformMatrix4fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX4X2DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
	PFNPROGRAMUNIFORMMATRIX4X2DV ProgramUniformMatrix4x2dv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX4X2FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
	PFNPROGRAMUNIFORMMATRIX4X2FV ProgramUniformMatrix4x2fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX4X3DV)(GLuint, GLint, GLsizei, GLboolean, const GLdouble *);
	PFNPROGRAMUNIFORMMATRIX4X3DV ProgramUniformMatrix4x3dv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROGRAMUNIFORMMATRIX4X3FV)(GLuint, GLint, GLsizei, GLboolean, const GLfloat *);
	PFNPROGRAMUNIFORMMATRIX4X3FV ProgramUniformMatrix4x3fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUSEPROGRAMSTAGES)(GLuint, GLbitfield, GLuint);
	PFNUSEPROGRAMSTAGES UseProgramStages = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVALIDATEPROGRAMPIPELINE)(GLuint);
	PFNVALIDATEPROGRAMPIPELINE ValidateProgramPipeline = 0;
	
	static int Load_ARB_separate_shader_objects()
	{
		int numFailed = 0;
		ActiveShaderProgram = reinterpret_cast<PFNACTIVESHADERPROGRAM>(IntGetProcAddress("glActiveShaderProgram"));
		if(!ActiveShaderProgram) ++numFailed;
		BindProgramPipeline = reinterpret_cast<PFNBINDPROGRAMPIPELINE>(IntGetProcAddress("glBindProgramPipeline"));
		if(!BindProgramPipeline) ++numFailed;
		CreateShaderProgramv = reinterpret_cast<PFNCREATESHADERPROGRAMV>(IntGetProcAddress("glCreateShaderProgramv"));
		if(!CreateShaderProgramv) ++numFailed;
		DeleteProgramPipelines = reinterpret_cast<PFNDELETEPROGRAMPIPELINES>(IntGetProcAddress("glDeleteProgramPipelines"));
		if(!DeleteProgramPipelines) ++numFailed;
		GenProgramPipelines = reinterpret_cast<PFNGENPROGRAMPIPELINES>(IntGetProcAddress("glGenProgramPipelines"));
		if(!GenProgramPipelines) ++numFailed;
		GetProgramPipelineInfoLog = reinterpret_cast<PFNGETPROGRAMPIPELINEINFOLOG>(IntGetProcAddress("glGetProgramPipelineInfoLog"));
		if(!GetProgramPipelineInfoLog) ++numFailed;
		GetProgramPipelineiv = reinterpret_cast<PFNGETPROGRAMPIPELINEIV>(IntGetProcAddress("glGetProgramPipelineiv"));
		if(!GetProgramPipelineiv) ++numFailed;
		IsProgramPipeline = reinterpret_cast<PFNISPROGRAMPIPELINE>(IntGetProcAddress("glIsProgramPipeline"));
		if(!IsProgramPipeline) ++numFailed;
		ProgramUniform1d = reinterpret_cast<PFNPROGRAMUNIFORM1D>(IntGetProcAddress("glProgramUniform1d"));
		if(!ProgramUniform1d) ++numFailed;
		ProgramUniform1dv = reinterpret_cast<PFNPROGRAMUNIFORM1DV>(IntGetProcAddress("glProgramUniform1dv"));
		if(!ProgramUniform1dv) ++numFailed;
		ProgramUniform1f = reinterpret_cast<PFNPROGRAMUNIFORM1F>(IntGetProcAddress("glProgramUniform1f"));
		if(!ProgramUniform1f) ++numFailed;
		ProgramUniform1fv = reinterpret_cast<PFNPROGRAMUNIFORM1FV>(IntGetProcAddress("glProgramUniform1fv"));
		if(!ProgramUniform1fv) ++numFailed;
		ProgramUniform1i = reinterpret_cast<PFNPROGRAMUNIFORM1I>(IntGetProcAddress("glProgramUniform1i"));
		if(!ProgramUniform1i) ++numFailed;
		ProgramUniform1iv = reinterpret_cast<PFNPROGRAMUNIFORM1IV>(IntGetProcAddress("glProgramUniform1iv"));
		if(!ProgramUniform1iv) ++numFailed;
		ProgramUniform1ui = reinterpret_cast<PFNPROGRAMUNIFORM1UI>(IntGetProcAddress("glProgramUniform1ui"));
		if(!ProgramUniform1ui) ++numFailed;
		ProgramUniform1uiv = reinterpret_cast<PFNPROGRAMUNIFORM1UIV>(IntGetProcAddress("glProgramUniform1uiv"));
		if(!ProgramUniform1uiv) ++numFailed;
		ProgramUniform2d = reinterpret_cast<PFNPROGRAMUNIFORM2D>(IntGetProcAddress("glProgramUniform2d"));
		if(!ProgramUniform2d) ++numFailed;
		ProgramUniform2dv = reinterpret_cast<PFNPROGRAMUNIFORM2DV>(IntGetProcAddress("glProgramUniform2dv"));
		if(!ProgramUniform2dv) ++numFailed;
		ProgramUniform2f = reinterpret_cast<PFNPROGRAMUNIFORM2F>(IntGetProcAddress("glProgramUniform2f"));
		if(!ProgramUniform2f) ++numFailed;
		ProgramUniform2fv = reinterpret_cast<PFNPROGRAMUNIFORM2FV>(IntGetProcAddress("glProgramUniform2fv"));
		if(!ProgramUniform2fv) ++numFailed;
		ProgramUniform2i = reinterpret_cast<PFNPROGRAMUNIFORM2I>(IntGetProcAddress("glProgramUniform2i"));
		if(!ProgramUniform2i) ++numFailed;
		ProgramUniform2iv = reinterpret_cast<PFNPROGRAMUNIFORM2IV>(IntGetProcAddress("glProgramUniform2iv"));
		if(!ProgramUniform2iv) ++numFailed;
		ProgramUniform2ui = reinterpret_cast<PFNPROGRAMUNIFORM2UI>(IntGetProcAddress("glProgramUniform2ui"));
		if(!ProgramUniform2ui) ++numFailed;
		ProgramUniform2uiv = reinterpret_cast<PFNPROGRAMUNIFORM2UIV>(IntGetProcAddress("glProgramUniform2uiv"));
		if(!ProgramUniform2uiv) ++numFailed;
		ProgramUniform3d = reinterpret_cast<PFNPROGRAMUNIFORM3D>(IntGetProcAddress("glProgramUniform3d"));
		if(!ProgramUniform3d) ++numFailed;
		ProgramUniform3dv = reinterpret_cast<PFNPROGRAMUNIFORM3DV>(IntGetProcAddress("glProgramUniform3dv"));
		if(!ProgramUniform3dv) ++numFailed;
		ProgramUniform3f = reinterpret_cast<PFNPROGRAMUNIFORM3F>(IntGetProcAddress("glProgramUniform3f"));
		if(!ProgramUniform3f) ++numFailed;
		ProgramUniform3fv = reinterpret_cast<PFNPROGRAMUNIFORM3FV>(IntGetProcAddress("glProgramUniform3fv"));
		if(!ProgramUniform3fv) ++numFailed;
		ProgramUniform3i = reinterpret_cast<PFNPROGRAMUNIFORM3I>(IntGetProcAddress("glProgramUniform3i"));
		if(!ProgramUniform3i) ++numFailed;
		ProgramUniform3iv = reinterpret_cast<PFNPROGRAMUNIFORM3IV>(IntGetProcAddress("glProgramUniform3iv"));
		if(!ProgramUniform3iv) ++numFailed;
		ProgramUniform3ui = reinterpret_cast<PFNPROGRAMUNIFORM3UI>(IntGetProcAddress("glProgramUniform3ui"));
		if(!ProgramUniform3ui) ++numFailed;
		ProgramUniform3uiv = reinterpret_cast<PFNPROGRAMUNIFORM3UIV>(IntGetProcAddress("glProgramUniform3uiv"));
		if(!ProgramUniform3uiv) ++numFailed;
		ProgramUniform4d = reinterpret_cast<PFNPROGRAMUNIFORM4D>(IntGetProcAddress("glProgramUniform4d"));
		if(!ProgramUniform4d) ++numFailed;
		ProgramUniform4dv = reinterpret_cast<PFNPROGRAMUNIFORM4DV>(IntGetProcAddress("glProgramUniform4dv"));
		if(!ProgramUniform4dv) ++numFailed;
		ProgramUniform4f = reinterpret_cast<PFNPROGRAMUNIFORM4F>(IntGetProcAddress("glProgramUniform4f"));
		if(!ProgramUniform4f) ++numFailed;
		ProgramUniform4fv = reinterpret_cast<PFNPROGRAMUNIFORM4FV>(IntGetProcAddress("glProgramUniform4fv"));
		if(!ProgramUniform4fv) ++numFailed;
		ProgramUniform4i = reinterpret_cast<PFNPROGRAMUNIFORM4I>(IntGetProcAddress("glProgramUniform4i"));
		if(!ProgramUniform4i) ++numFailed;
		ProgramUniform4iv = reinterpret_cast<PFNPROGRAMUNIFORM4IV>(IntGetProcAddress("glProgramUniform4iv"));
		if(!ProgramUniform4iv) ++numFailed;
		ProgramUniform4ui = reinterpret_cast<PFNPROGRAMUNIFORM4UI>(IntGetProcAddress("glProgramUniform4ui"));
		if(!ProgramUniform4ui) ++numFailed;
		ProgramUniform4uiv = reinterpret_cast<PFNPROGRAMUNIFORM4UIV>(IntGetProcAddress("glProgramUniform4uiv"));
		if(!ProgramUniform4uiv) ++numFailed;
		ProgramUniformMatrix2dv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX2DV>(IntGetProcAddress("glProgramUniformMatrix2dv"));
		if(!ProgramUniformMatrix2dv) ++numFailed;
		ProgramUniformMatrix2fv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX2FV>(IntGetProcAddress("glProgramUniformMatrix2fv"));
		if(!ProgramUniformMatrix2fv) ++numFailed;
		ProgramUniformMatrix2x3dv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX2X3DV>(IntGetProcAddress("glProgramUniformMatrix2x3dv"));
		if(!ProgramUniformMatrix2x3dv) ++numFailed;
		ProgramUniformMatrix2x3fv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX2X3FV>(IntGetProcAddress("glProgramUniformMatrix2x3fv"));
		if(!ProgramUniformMatrix2x3fv) ++numFailed;
		ProgramUniformMatrix2x4dv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX2X4DV>(IntGetProcAddress("glProgramUniformMatrix2x4dv"));
		if(!ProgramUniformMatrix2x4dv) ++numFailed;
		ProgramUniformMatrix2x4fv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX2X4FV>(IntGetProcAddress("glProgramUniformMatrix2x4fv"));
		if(!ProgramUniformMatrix2x4fv) ++numFailed;
		ProgramUniformMatrix3dv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX3DV>(IntGetProcAddress("glProgramUniformMatrix3dv"));
		if(!ProgramUniformMatrix3dv) ++numFailed;
		ProgramUniformMatrix3fv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX3FV>(IntGetProcAddress("glProgramUniformMatrix3fv"));
		if(!ProgramUniformMatrix3fv) ++numFailed;
		ProgramUniformMatrix3x2dv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX3X2DV>(IntGetProcAddress("glProgramUniformMatrix3x2dv"));
		if(!ProgramUniformMatrix3x2dv) ++numFailed;
		ProgramUniformMatrix3x2fv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX3X2FV>(IntGetProcAddress("glProgramUniformMatrix3x2fv"));
		if(!ProgramUniformMatrix3x2fv) ++numFailed;
		ProgramUniformMatrix3x4dv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX3X4DV>(IntGetProcAddress("glProgramUniformMatrix3x4dv"));
		if(!ProgramUniformMatrix3x4dv) ++numFailed;
		ProgramUniformMatrix3x4fv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX3X4FV>(IntGetProcAddress("glProgramUniformMatrix3x4fv"));
		if(!ProgramUniformMatrix3x4fv) ++numFailed;
		ProgramUniformMatrix4dv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX4DV>(IntGetProcAddress("glProgramUniformMatrix4dv"));
		if(!ProgramUniformMatrix4dv) ++numFailed;
		ProgramUniformMatrix4fv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX4FV>(IntGetProcAddress("glProgramUniformMatrix4fv"));
		if(!ProgramUniformMatrix4fv) ++numFailed;
		ProgramUniformMatrix4x2dv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX4X2DV>(IntGetProcAddress("glProgramUniformMatrix4x2dv"));
		if(!ProgramUniformMatrix4x2dv) ++numFailed;
		ProgramUniformMatrix4x2fv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX4X2FV>(IntGetProcAddress("glProgramUniformMatrix4x2fv"));
		if(!ProgramUniformMatrix4x2fv) ++numFailed;
		ProgramUniformMatrix4x3dv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX4X3DV>(IntGetProcAddress("glProgramUniformMatrix4x3dv"));
		if(!ProgramUniformMatrix4x3dv) ++numFailed;
		ProgramUniformMatrix4x3fv = reinterpret_cast<PFNPROGRAMUNIFORMMATRIX4X3FV>(IntGetProcAddress("glProgramUniformMatrix4x3fv"));
		if(!ProgramUniformMatrix4x3fv) ++numFailed;
		UseProgramStages = reinterpret_cast<PFNUSEPROGRAMSTAGES>(IntGetProcAddress("glUseProgramStages"));
		if(!UseProgramStages) ++numFailed;
		ValidateProgramPipeline = reinterpret_cast<PFNVALIDATEPROGRAMPIPELINE>(IntGetProcAddress("glValidateProgramPipeline"));
		if(!ValidateProgramPipeline) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNTEXBUFFERRANGE)(GLenum, GLenum, GLuint, GLintptr, GLsizeiptr);
	PFNTEXBUFFERRANGE TexBufferRange = 0;
	
	static int Load_ARB_texture_buffer_range()
	{
		int numFailed = 0;
		TexBufferRange = reinterpret_cast<PFNTEXBUFFERRANGE>(IntGetProcAddress("glTexBufferRange"));
		if(!TexBufferRange) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNTEXSTORAGE1D)(GLenum, GLsizei, GLenum, GLsizei);
	PFNTEXSTORAGE1D TexStorage1D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXSTORAGE2D)(GLenum, GLsizei, GLenum, GLsizei, GLsizei);
	PFNTEXSTORAGE2D TexStorage2D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXSTORAGE3D)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei);
	PFNTEXSTORAGE3D TexStorage3D = 0;
	
	static int Load_ARB_texture_storage()
	{
		int numFailed = 0;
		TexStorage1D = reinterpret_cast<PFNTEXSTORAGE1D>(IntGetProcAddress("glTexStorage1D"));
		if(!TexStorage1D) ++numFailed;
		TexStorage2D = reinterpret_cast<PFNTEXSTORAGE2D>(IntGetProcAddress("glTexStorage2D"));
		if(!TexStorage2D) ++numFailed;
		TexStorage3D = reinterpret_cast<PFNTEXSTORAGE3D>(IntGetProcAddress("glTexStorage3D"));
		if(!TexStorage3D) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNTEXTUREVIEW)(GLuint, GLenum, GLuint, GLenum, GLuint, GLuint, GLuint, GLuint);
	PFNTEXTUREVIEW TextureView = 0;
	
	static int Load_ARB_texture_view()
	{
		int numFailed = 0;
		TextureView = reinterpret_cast<PFNTEXTUREVIEW>(IntGetProcAddress("glTextureView"));
		if(!TextureView) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNBINDVERTEXBUFFER)(GLuint, GLuint, GLintptr, GLsizei);
	PFNBINDVERTEXBUFFER BindVertexBuffer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBBINDING)(GLuint, GLuint);
	PFNVERTEXATTRIBBINDING VertexAttribBinding = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBFORMAT)(GLuint, GLint, GLenum, GLboolean, GLuint);
	PFNVERTEXATTRIBFORMAT VertexAttribFormat = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBIFORMAT)(GLuint, GLint, GLenum, GLuint);
	PFNVERTEXATTRIBIFORMAT VertexAttribIFormat = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBLFORMAT)(GLuint, GLint, GLenum, GLuint);
	PFNVERTEXATTRIBLFORMAT VertexAttribLFormat = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXBINDINGDIVISOR)(GLuint, GLuint);
	PFNVERTEXBINDINGDIVISOR VertexBindingDivisor = 0;
	
	static int Load_ARB_vertex_attrib_binding()
	{
		int numFailed = 0;
		BindVertexBuffer = reinterpret_cast<PFNBINDVERTEXBUFFER>(IntGetProcAddress("glBindVertexBuffer"));
		if(!BindVertexBuffer) ++numFailed;
		VertexAttribBinding = reinterpret_cast<PFNVERTEXATTRIBBINDING>(IntGetProcAddress("glVertexAttribBinding"));
		if(!VertexAttribBinding) ++numFailed;
		VertexAttribFormat = reinterpret_cast<PFNVERTEXATTRIBFORMAT>(IntGetProcAddress("glVertexAttribFormat"));
		if(!VertexAttribFormat) ++numFailed;
		VertexAttribIFormat = reinterpret_cast<PFNVERTEXATTRIBIFORMAT>(IntGetProcAddress("glVertexAttribIFormat"));
		if(!VertexAttribIFormat) ++numFailed;
		VertexAttribLFormat = reinterpret_cast<PFNVERTEXATTRIBLFORMAT>(IntGetProcAddress("glVertexAttribLFormat"));
		if(!VertexAttribLFormat) ++numFailed;
		VertexBindingDivisor = reinterpret_cast<PFNVERTEXBINDINGDIVISOR>(IntGetProcAddress("glVertexBindingDivisor"));
		if(!VertexBindingDivisor) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNDEPTHRANGEARRAYV)(GLuint, GLsizei, const GLdouble *);
	PFNDEPTHRANGEARRAYV DepthRangeArrayv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDEPTHRANGEINDEXED)(GLuint, GLdouble, GLdouble);
	PFNDEPTHRANGEINDEXED DepthRangeIndexed = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETDOUBLEI_V)(GLenum, GLuint, GLdouble *);
	PFNGETDOUBLEI_V GetDoublei_v = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETFLOATI_V)(GLenum, GLuint, GLfloat *);
	PFNGETFLOATI_V GetFloati_v = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSCISSORARRAYV)(GLuint, GLsizei, const GLint *);
	PFNSCISSORARRAYV ScissorArrayv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSCISSORINDEXED)(GLuint, GLint, GLint, GLsizei, GLsizei);
	PFNSCISSORINDEXED ScissorIndexed = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSCISSORINDEXEDV)(GLuint, const GLint *);
	PFNSCISSORINDEXEDV ScissorIndexedv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVIEWPORTARRAYV)(GLuint, GLsizei, const GLfloat *);
	PFNVIEWPORTARRAYV ViewportArrayv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVIEWPORTINDEXEDF)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
	PFNVIEWPORTINDEXEDF ViewportIndexedf = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVIEWPORTINDEXEDFV)(GLuint, const GLfloat *);
	PFNVIEWPORTINDEXEDFV ViewportIndexedfv = 0;
	
	static int Load_ARB_viewport_array()
	{
		int numFailed = 0;
		DepthRangeArrayv = reinterpret_cast<PFNDEPTHRANGEARRAYV>(IntGetProcAddress("glDepthRangeArrayv"));
		if(!DepthRangeArrayv) ++numFailed;
		DepthRangeIndexed = reinterpret_cast<PFNDEPTHRANGEINDEXED>(IntGetProcAddress("glDepthRangeIndexed"));
		if(!DepthRangeIndexed) ++numFailed;
		GetDoublei_v = reinterpret_cast<PFNGETDOUBLEI_V>(IntGetProcAddress("glGetDoublei_v"));
		if(!GetDoublei_v) ++numFailed;
		GetFloati_v = reinterpret_cast<PFNGETFLOATI_V>(IntGetProcAddress("glGetFloati_v"));
		if(!GetFloati_v) ++numFailed;
		ScissorArrayv = reinterpret_cast<PFNSCISSORARRAYV>(IntGetProcAddress("glScissorArrayv"));
		if(!ScissorArrayv) ++numFailed;
		ScissorIndexed = reinterpret_cast<PFNSCISSORINDEXED>(IntGetProcAddress("glScissorIndexed"));
		if(!ScissorIndexed) ++numFailed;
		ScissorIndexedv = reinterpret_cast<PFNSCISSORINDEXEDV>(IntGetProcAddress("glScissorIndexedv"));
		if(!ScissorIndexedv) ++numFailed;
		ViewportArrayv = reinterpret_cast<PFNVIEWPORTARRAYV>(IntGetProcAddress("glViewportArrayv"));
		if(!ViewportArrayv) ++numFailed;
		ViewportIndexedf = reinterpret_cast<PFNVIEWPORTINDEXEDF>(IntGetProcAddress("glViewportIndexedf"));
		if(!ViewportIndexedf) ++numFailed;
		ViewportIndexedfv = reinterpret_cast<PFNVIEWPORTINDEXEDFV>(IntGetProcAddress("glViewportIndexedfv"));
		if(!ViewportIndexedfv) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNCLEARBUFFERDATA)(GLenum, GLenum, GLenum, GLenum, const void *);
	PFNCLEARBUFFERDATA ClearBufferData = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCLEARBUFFERSUBDATA)(GLenum, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void *);
	PFNCLEARBUFFERSUBDATA ClearBufferSubData = 0;
	
	static int Load_ARB_clear_buffer_object()
	{
		int numFailed = 0;
		ClearBufferData = reinterpret_cast<PFNCLEARBUFFERDATA>(IntGetProcAddress("glClearBufferData"));
		if(!ClearBufferData) ++numFailed;
		ClearBufferSubData = reinterpret_cast<PFNCLEARBUFFERSUBDATA>(IntGetProcAddress("glClearBufferSubData"));
		if(!ClearBufferSubData) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNCOPYIMAGESUBDATA)(GLuint, GLenum, GLint, GLint, GLint, GLint, GLuint, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei);
	PFNCOPYIMAGESUBDATA CopyImageSubData = 0;
	
	static int Load_ARB_copy_image()
	{
		int numFailed = 0;
		CopyImageSubData = reinterpret_cast<PFNCOPYIMAGESUBDATA>(IntGetProcAddress("glCopyImageSubData"));
		if(!CopyImageSubData) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNFRAMEBUFFERPARAMETERI)(GLenum, GLenum, GLint);
	PFNFRAMEBUFFERPARAMETERI FramebufferParameteri = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETFRAMEBUFFERPARAMETERIV)(GLenum, GLenum, GLint *);
	PFNGETFRAMEBUFFERPARAMETERIV GetFramebufferParameteriv = 0;
	
	static int Load_ARB_framebuffer_no_attachments()
	{
		int numFailed = 0;
		FramebufferParameteri = reinterpret_cast<PFNFRAMEBUFFERPARAMETERI>(IntGetProcAddress("glFramebufferParameteri"));
		if(!FramebufferParameteri) ++numFailed;
		GetFramebufferParameteriv = reinterpret_cast<PFNGETFRAMEBUFFERPARAMETERIV>(IntGetProcAddress("glGetFramebufferParameteriv"));
		if(!GetFramebufferParameteriv) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNINVALIDATEBUFFERDATA)(GLuint);
	PFNINVALIDATEBUFFERDATA InvalidateBufferData = 0;
	typedef void (CODEGEN_FUNCPTR *PFNINVALIDATEBUFFERSUBDATA)(GLuint, GLintptr, GLsizeiptr);
	PFNINVALIDATEBUFFERSUBDATA InvalidateBufferSubData = 0;
	typedef void (CODEGEN_FUNCPTR *PFNINVALIDATEFRAMEBUFFER)(GLenum, GLsizei, const GLenum *);
	PFNINVALIDATEFRAMEBUFFER InvalidateFramebuffer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNINVALIDATESUBFRAMEBUFFER)(GLenum, GLsizei, const GLenum *, GLint, GLint, GLsizei, GLsizei);
	PFNINVALIDATESUBFRAMEBUFFER InvalidateSubFramebuffer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNINVALIDATETEXIMAGE)(GLuint, GLint);
	PFNINVALIDATETEXIMAGE InvalidateTexImage = 0;
	typedef void (CODEGEN_FUNCPTR *PFNINVALIDATETEXSUBIMAGE)(GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei);
	PFNINVALIDATETEXSUBIMAGE InvalidateTexSubImage = 0;
	
	static int Load_ARB_invalidate_subdata()
	{
		int numFailed = 0;
		InvalidateBufferData = reinterpret_cast<PFNINVALIDATEBUFFERDATA>(IntGetProcAddress("glInvalidateBufferData"));
		if(!InvalidateBufferData) ++numFailed;
		InvalidateBufferSubData = reinterpret_cast<PFNINVALIDATEBUFFERSUBDATA>(IntGetProcAddress("glInvalidateBufferSubData"));
		if(!InvalidateBufferSubData) ++numFailed;
		InvalidateFramebuffer = reinterpret_cast<PFNINVALIDATEFRAMEBUFFER>(IntGetProcAddress("glInvalidateFramebuffer"));
		if(!InvalidateFramebuffer) ++numFailed;
		InvalidateSubFramebuffer = reinterpret_cast<PFNINVALIDATESUBFRAMEBUFFER>(IntGetProcAddress("glInvalidateSubFramebuffer"));
		if(!InvalidateSubFramebuffer) ++numFailed;
		InvalidateTexImage = reinterpret_cast<PFNINVALIDATETEXIMAGE>(IntGetProcAddress("glInvalidateTexImage"));
		if(!InvalidateTexImage) ++numFailed;
		InvalidateTexSubImage = reinterpret_cast<PFNINVALIDATETEXSUBIMAGE>(IntGetProcAddress("glInvalidateTexSubImage"));
		if(!InvalidateTexSubImage) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNTEXSTORAGE2DMULTISAMPLE)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean);
	PFNTEXSTORAGE2DMULTISAMPLE TexStorage2DMultisample = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXSTORAGE3DMULTISAMPLE)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean);
	PFNTEXSTORAGE3DMULTISAMPLE TexStorage3DMultisample = 0;
	
	static int Load_ARB_texture_storage_multisample()
	{
		int numFailed = 0;
		TexStorage2DMultisample = reinterpret_cast<PFNTEXSTORAGE2DMULTISAMPLE>(IntGetProcAddress("glTexStorage2DMultisample"));
		if(!TexStorage2DMultisample) ++numFailed;
		TexStorage3DMultisample = reinterpret_cast<PFNTEXSTORAGE3DMULTISAMPLE>(IntGetProcAddress("glTexStorage3DMultisample"));
		if(!TexStorage3DMultisample) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNDEBUGMESSAGECALLBACK)(GLDEBUGPROC, const void *);
	PFNDEBUGMESSAGECALLBACK DebugMessageCallback = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDEBUGMESSAGECONTROL)(GLenum, GLenum, GLenum, GLsizei, const GLuint *, GLboolean);
	PFNDEBUGMESSAGECONTROL DebugMessageControl = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDEBUGMESSAGEINSERT)(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *);
	PFNDEBUGMESSAGEINSERT DebugMessageInsert = 0;
	typedef GLuint (CODEGEN_FUNCPTR *PFNGETDEBUGMESSAGELOG)(GLuint, GLsizei, GLenum *, GLenum *, GLuint *, GLenum *, GLsizei *, GLchar *);
	PFNGETDEBUGMESSAGELOG GetDebugMessageLog = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETOBJECTLABEL)(GLenum, GLuint, GLsizei, GLsizei *, GLchar *);
	PFNGETOBJECTLABEL GetObjectLabel = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETOBJECTPTRLABEL)(const void *, GLsizei, GLsizei *, GLchar *);
	PFNGETOBJECTPTRLABEL GetObjectPtrLabel = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETPOINTERV)(GLenum, void **);
	PFNGETPOINTERV GetPointerv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNOBJECTLABEL)(GLenum, GLuint, GLsizei, const GLchar *);
	PFNOBJECTLABEL ObjectLabel = 0;
	typedef void (CODEGEN_FUNCPTR *PFNOBJECTPTRLABEL)(const void *, GLsizei, const GLchar *);
	PFNOBJECTPTRLABEL ObjectPtrLabel = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPOPDEBUGGROUP)(void);
	PFNPOPDEBUGGROUP PopDebugGroup = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPUSHDEBUGGROUP)(GLenum, GLuint, GLsizei, const GLchar *);
	PFNPUSHDEBUGGROUP PushDebugGroup = 0;
	
	static int Load_KHR_debug()
	{
		int numFailed = 0;
		DebugMessageCallback = reinterpret_cast<PFNDEBUGMESSAGECALLBACK>(IntGetProcAddress("glDebugMessageCallback"));
		if(!DebugMessageCallback) ++numFailed;
		DebugMessageControl = reinterpret_cast<PFNDEBUGMESSAGECONTROL>(IntGetProcAddress("glDebugMessageControl"));
		if(!DebugMessageControl) ++numFailed;
		DebugMessageInsert = reinterpret_cast<PFNDEBUGMESSAGEINSERT>(IntGetProcAddress("glDebugMessageInsert"));
		if(!DebugMessageInsert) ++numFailed;
		GetDebugMessageLog = reinterpret_cast<PFNGETDEBUGMESSAGELOG>(IntGetProcAddress("glGetDebugMessageLog"));
		if(!GetDebugMessageLog) ++numFailed;
		GetObjectLabel = reinterpret_cast<PFNGETOBJECTLABEL>(IntGetProcAddress("glGetObjectLabel"));
		if(!GetObjectLabel) ++numFailed;
		GetObjectPtrLabel = reinterpret_cast<PFNGETOBJECTPTRLABEL>(IntGetProcAddress("glGetObjectPtrLabel"));
		if(!GetObjectPtrLabel) ++numFailed;
		GetPointerv = reinterpret_cast<PFNGETPOINTERV>(IntGetProcAddress("glGetPointerv"));
		if(!GetPointerv) ++numFailed;
		ObjectLabel = reinterpret_cast<PFNOBJECTLABEL>(IntGetProcAddress("glObjectLabel"));
		if(!ObjectLabel) ++numFailed;
		ObjectPtrLabel = reinterpret_cast<PFNOBJECTPTRLABEL>(IntGetProcAddress("glObjectPtrLabel"));
		if(!ObjectPtrLabel) ++numFailed;
		PopDebugGroup = reinterpret_cast<PFNPOPDEBUGGROUP>(IntGetProcAddress("glPopDebugGroup"));
		if(!PopDebugGroup) ++numFailed;
		PushDebugGroup = reinterpret_cast<PFNPUSHDEBUGGROUP>(IntGetProcAddress("glPushDebugGroup"));
		if(!PushDebugGroup) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNBUFFERSTORAGE)(GLenum, GLsizeiptr, const void *, GLbitfield);
	PFNBUFFERSTORAGE BufferStorage = 0;
	
	static int Load_ARB_buffer_storage()
	{
		int numFailed = 0;
		BufferStorage = reinterpret_cast<PFNBUFFERSTORAGE>(IntGetProcAddress("glBufferStorage"));
		if(!BufferStorage) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNCLEARTEXIMAGE)(GLuint, GLint, GLenum, GLenum, const void *);
	PFNCLEARTEXIMAGE ClearTexImage = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCLEARTEXSUBIMAGE)(GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void *);
	PFNCLEARTEXSUBIMAGE ClearTexSubImage = 0;
	
	static int Load_ARB_clear_texture()
	{
		int numFailed = 0;
		ClearTexImage = reinterpret_cast<PFNCLEARTEXIMAGE>(IntGetProcAddress("glClearTexImage"));
		if(!ClearTexImage) ++numFailed;
		ClearTexSubImage = reinterpret_cast<PFNCLEARTEXSUBIMAGE>(IntGetProcAddress("glClearTexSubImage"));
		if(!ClearTexSubImage) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNBINDBUFFERSBASE)(GLenum, GLuint, GLsizei, const GLuint *);
	PFNBINDBUFFERSBASE BindBuffersBase = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBINDBUFFERSRANGE)(GLenum, GLuint, GLsizei, const GLuint *, const GLintptr *, const GLsizeiptr *);
	PFNBINDBUFFERSRANGE BindBuffersRange = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBINDIMAGETEXTURES)(GLuint, GLsizei, const GLuint *);
	PFNBINDIMAGETEXTURES BindImageTextures = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBINDSAMPLERS)(GLuint, GLsizei, const GLuint *);
	PFNBINDSAMPLERS BindSamplers = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBINDTEXTURES)(GLuint, GLsizei, const GLuint *);
	PFNBINDTEXTURES BindTextures = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBINDVERTEXBUFFERS)(GLuint, GLsizei, const GLuint *, const GLintptr *, const GLsizei *);
	PFNBINDVERTEXBUFFERS BindVertexBuffers = 0;
	
	static int Load_ARB_multi_bind()
	{
		int numFailed = 0;
		BindBuffersBase = reinterpret_cast<PFNBINDBUFFERSBASE>(IntGetProcAddress("glBindBuffersBase"));
		if(!BindBuffersBase) ++numFailed;
		BindBuffersRange = reinterpret_cast<PFNBINDBUFFERSRANGE>(IntGetProcAddress("glBindBuffersRange"));
		if(!BindBuffersRange) ++numFailed;
		BindImageTextures = reinterpret_cast<PFNBINDIMAGETEXTURES>(IntGetProcAddress("glBindImageTextures"));
		if(!BindImageTextures) ++numFailed;
		BindSamplers = reinterpret_cast<PFNBINDSAMPLERS>(IntGetProcAddress("glBindSamplers"));
		if(!BindSamplers) ++numFailed;
		BindTextures = reinterpret_cast<PFNBINDTEXTURES>(IntGetProcAddress("glBindTextures"));
		if(!BindTextures) ++numFailed;
		BindVertexBuffers = reinterpret_cast<PFNBINDVERTEXBUFFERS>(IntGetProcAddress("glBindVertexBuffers"));
		if(!BindVertexBuffers) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNCLIPCONTROL)(GLenum, GLenum);
	PFNCLIPCONTROL ClipControl = 0;
	
	static int Load_ARB_clip_control()
	{
		int numFailed = 0;
		ClipControl = reinterpret_cast<PFNCLIPCONTROL>(IntGetProcAddress("glClipControl"));
		if(!ClipControl) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNBINDTEXTUREUNIT)(GLuint, GLuint);
	PFNBINDTEXTUREUNIT BindTextureUnit = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBLITNAMEDFRAMEBUFFER)(GLuint, GLuint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
	PFNBLITNAMEDFRAMEBUFFER BlitNamedFramebuffer = 0;
	typedef GLenum (CODEGEN_FUNCPTR *PFNCHECKNAMEDFRAMEBUFFERSTATUS)(GLuint, GLenum);
	PFNCHECKNAMEDFRAMEBUFFERSTATUS CheckNamedFramebufferStatus = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCLEARNAMEDBUFFERDATA)(GLuint, GLenum, GLenum, GLenum, const void *);
	PFNCLEARNAMEDBUFFERDATA ClearNamedBufferData = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCLEARNAMEDBUFFERSUBDATA)(GLuint, GLenum, GLintptr, GLsizeiptr, GLenum, GLenum, const void *);
	PFNCLEARNAMEDBUFFERSUBDATA ClearNamedBufferSubData = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCLEARNAMEDFRAMEBUFFERFI)(GLuint, GLenum, GLint, const GLfloat, GLint);
	PFNCLEARNAMEDFRAMEBUFFERFI ClearNamedFramebufferfi = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCLEARNAMEDFRAMEBUFFERFV)(GLuint, GLenum, GLint, const GLfloat *);
	PFNCLEARNAMEDFRAMEBUFFERFV ClearNamedFramebufferfv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCLEARNAMEDFRAMEBUFFERIV)(GLuint, GLenum, GLint, const GLint *);
	PFNCLEARNAMEDFRAMEBUFFERIV ClearNamedFramebufferiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCLEARNAMEDFRAMEBUFFERUIV)(GLuint, GLenum, GLint, const GLuint *);
	PFNCLEARNAMEDFRAMEBUFFERUIV ClearNamedFramebufferuiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOMPRESSEDTEXTURESUBIMAGE1D)(GLuint, GLint, GLint, GLsizei, GLenum, GLsizei, const void *);
	PFNCOMPRESSEDTEXTURESUBIMAGE1D CompressedTextureSubImage1D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOMPRESSEDTEXTURESUBIMAGE2D)(GLuint, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void *);
	PFNCOMPRESSEDTEXTURESUBIMAGE2D CompressedTextureSubImage2D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOMPRESSEDTEXTURESUBIMAGE3D)(GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void *);
	PFNCOMPRESSEDTEXTURESUBIMAGE3D CompressedTextureSubImage3D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOPYNAMEDBUFFERSUBDATA)(GLuint, GLuint, GLintptr, GLintptr, GLsizeiptr);
	PFNCOPYNAMEDBUFFERSUBDATA CopyNamedBufferSubData = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOPYTEXTURESUBIMAGE1D)(GLuint, GLint, GLint, GLint, GLint, GLsizei);
	PFNCOPYTEXTURESUBIMAGE1D CopyTextureSubImage1D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOPYTEXTURESUBIMAGE2D)(GLuint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
	PFNCOPYTEXTURESUBIMAGE2D CopyTextureSubImage2D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOPYTEXTURESUBIMAGE3D)(GLuint, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
	PFNCOPYTEXTURESUBIMAGE3D CopyTextureSubImage3D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCREATEBUFFERS)(GLsizei, GLuint *);
	PFNCREATEBUFFERS CreateBuffers = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCREATEFRAMEBUFFERS)(GLsizei, GLuint *);
	PFNCREATEFRAMEBUFFERS CreateFramebuffers = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCREATEPROGRAMPIPELINES)(GLsizei, GLuint *);
	PFNCREATEPROGRAMPIPELINES CreateProgramPipelines = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCREATEQUERIES)(GLenum, GLsizei, GLuint *);
	PFNCREATEQUERIES CreateQueries = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCREATERENDERBUFFERS)(GLsizei, GLuint *);
	PFNCREATERENDERBUFFERS CreateRenderbuffers = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCREATESAMPLERS)(GLsizei, GLuint *);
	PFNCREATESAMPLERS CreateSamplers = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCREATETEXTURES)(GLenum, GLsizei, GLuint *);
	PFNCREATETEXTURES CreateTextures = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCREATETRANSFORMFEEDBACKS)(GLsizei, GLuint *);
	PFNCREATETRANSFORMFEEDBACKS CreateTransformFeedbacks = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCREATEVERTEXARRAYS)(GLsizei, GLuint *);
	PFNCREATEVERTEXARRAYS CreateVertexArrays = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDISABLEVERTEXARRAYATTRIB)(GLuint, GLuint);
	PFNDISABLEVERTEXARRAYATTRIB DisableVertexArrayAttrib = 0;
	typedef void (CODEGEN_FUNCPTR *PFNENABLEVERTEXARRAYATTRIB)(GLuint, GLuint);
	PFNENABLEVERTEXARRAYATTRIB EnableVertexArrayAttrib = 0;
	typedef void (CODEGEN_FUNCPTR *PFNFLUSHMAPPEDNAMEDBUFFERRANGE)(GLuint, GLintptr, GLsizeiptr);
	PFNFLUSHMAPPEDNAMEDBUFFERRANGE FlushMappedNamedBufferRange = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGENERATETEXTUREMIPMAP)(GLuint);
	PFNGENERATETEXTUREMIPMAP GenerateTextureMipmap = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETCOMPRESSEDTEXTUREIMAGE)(GLuint, GLint, GLsizei, void *);
	PFNGETCOMPRESSEDTEXTUREIMAGE GetCompressedTextureImage = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETNAMEDBUFFERPARAMETERI64V)(GLuint, GLenum, GLint64 *);
	PFNGETNAMEDBUFFERPARAMETERI64V GetNamedBufferParameteri64v = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETNAMEDBUFFERPARAMETERIV)(GLuint, GLenum, GLint *);
	PFNGETNAMEDBUFFERPARAMETERIV GetNamedBufferParameteriv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETNAMEDBUFFERPOINTERV)(GLuint, GLenum, void **);
	PFNGETNAMEDBUFFERPOINTERV GetNamedBufferPointerv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETNAMEDBUFFERSUBDATA)(GLuint, GLintptr, GLsizeiptr, void *);
	PFNGETNAMEDBUFFERSUBDATA GetNamedBufferSubData = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIV)(GLuint, GLenum, GLenum, GLint *);
	PFNGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIV GetNamedFramebufferAttachmentParameteriv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETNAMEDFRAMEBUFFERPARAMETERIV)(GLuint, GLenum, GLint *);
	PFNGETNAMEDFRAMEBUFFERPARAMETERIV GetNamedFramebufferParameteriv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETNAMEDRENDERBUFFERPARAMETERIV)(GLuint, GLenum, GLint *);
	PFNGETNAMEDRENDERBUFFERPARAMETERIV GetNamedRenderbufferParameteriv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETQUERYBUFFEROBJECTI64V)(GLuint, GLuint, GLenum, GLintptr);
	PFNGETQUERYBUFFEROBJECTI64V GetQueryBufferObjecti64v = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETQUERYBUFFEROBJECTIV)(GLuint, GLuint, GLenum, GLintptr);
	PFNGETQUERYBUFFEROBJECTIV GetQueryBufferObjectiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETQUERYBUFFEROBJECTUI64V)(GLuint, GLuint, GLenum, GLintptr);
	PFNGETQUERYBUFFEROBJECTUI64V GetQueryBufferObjectui64v = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETQUERYBUFFEROBJECTUIV)(GLuint, GLuint, GLenum, GLintptr);
	PFNGETQUERYBUFFEROBJECTUIV GetQueryBufferObjectuiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTEXTUREIMAGE)(GLuint, GLint, GLenum, GLenum, GLsizei, void *);
	PFNGETTEXTUREIMAGE GetTextureImage = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTEXTURELEVELPARAMETERFV)(GLuint, GLint, GLenum, GLfloat *);
	PFNGETTEXTURELEVELPARAMETERFV GetTextureLevelParameterfv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTEXTURELEVELPARAMETERIV)(GLuint, GLint, GLenum, GLint *);
	PFNGETTEXTURELEVELPARAMETERIV GetTextureLevelParameteriv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTEXTUREPARAMETERIIV)(GLuint, GLenum, GLint *);
	PFNGETTEXTUREPARAMETERIIV GetTextureParameterIiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTEXTUREPARAMETERIUIV)(GLuint, GLenum, GLuint *);
	PFNGETTEXTUREPARAMETERIUIV GetTextureParameterIuiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTEXTUREPARAMETERFV)(GLuint, GLenum, GLfloat *);
	PFNGETTEXTUREPARAMETERFV GetTextureParameterfv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTEXTUREPARAMETERIV)(GLuint, GLenum, GLint *);
	PFNGETTEXTUREPARAMETERIV GetTextureParameteriv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTRANSFORMFEEDBACKI64_V)(GLuint, GLenum, GLuint, GLint64 *);
	PFNGETTRANSFORMFEEDBACKI64_V GetTransformFeedbacki64_v = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTRANSFORMFEEDBACKI_V)(GLuint, GLenum, GLuint, GLint *);
	PFNGETTRANSFORMFEEDBACKI_V GetTransformFeedbacki_v = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTRANSFORMFEEDBACKIV)(GLuint, GLenum, GLint *);
	PFNGETTRANSFORMFEEDBACKIV GetTransformFeedbackiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETVERTEXARRAYINDEXED64IV)(GLuint, GLuint, GLenum, GLint64 *);
	PFNGETVERTEXARRAYINDEXED64IV GetVertexArrayIndexed64iv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETVERTEXARRAYINDEXEDIV)(GLuint, GLuint, GLenum, GLint *);
	PFNGETVERTEXARRAYINDEXEDIV GetVertexArrayIndexediv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETVERTEXARRAYIV)(GLuint, GLenum, GLint *);
	PFNGETVERTEXARRAYIV GetVertexArrayiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNINVALIDATENAMEDFRAMEBUFFERDATA)(GLuint, GLsizei, const GLenum *);
	PFNINVALIDATENAMEDFRAMEBUFFERDATA InvalidateNamedFramebufferData = 0;
	typedef void (CODEGEN_FUNCPTR *PFNINVALIDATENAMEDFRAMEBUFFERSUBDATA)(GLuint, GLsizei, const GLenum *, GLint, GLint, GLsizei, GLsizei);
	PFNINVALIDATENAMEDFRAMEBUFFERSUBDATA InvalidateNamedFramebufferSubData = 0;
	typedef void * (CODEGEN_FUNCPTR *PFNMAPNAMEDBUFFER)(GLuint, GLenum);
	PFNMAPNAMEDBUFFER MapNamedBuffer = 0;
	typedef void * (CODEGEN_FUNCPTR *PFNMAPNAMEDBUFFERRANGE)(GLuint, GLintptr, GLsizeiptr, GLbitfield);
	PFNMAPNAMEDBUFFERRANGE MapNamedBufferRange = 0;
	typedef void (CODEGEN_FUNCPTR *PFNNAMEDBUFFERDATA)(GLuint, GLsizeiptr, const void *, GLenum);
	PFNNAMEDBUFFERDATA NamedBufferData = 0;
	typedef void (CODEGEN_FUNCPTR *PFNNAMEDBUFFERSTORAGE)(GLuint, GLsizeiptr, const void *, GLbitfield);
	PFNNAMEDBUFFERSTORAGE NamedBufferStorage = 0;
	typedef void (CODEGEN_FUNCPTR *PFNNAMEDBUFFERSUBDATA)(GLuint, GLintptr, GLsizeiptr, const void *);
	PFNNAMEDBUFFERSUBDATA NamedBufferSubData = 0;
	typedef void (CODEGEN_FUNCPTR *PFNNAMEDFRAMEBUFFERDRAWBUFFER)(GLuint, GLenum);
	PFNNAMEDFRAMEBUFFERDRAWBUFFER NamedFramebufferDrawBuffer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNNAMEDFRAMEBUFFERDRAWBUFFERS)(GLuint, GLsizei, const GLenum *);
	PFNNAMEDFRAMEBUFFERDRAWBUFFERS NamedFramebufferDrawBuffers = 0;
	typedef void (CODEGEN_FUNCPTR *PFNNAMEDFRAMEBUFFERPARAMETERI)(GLuint, GLenum, GLint);
	PFNNAMEDFRAMEBUFFERPARAMETERI NamedFramebufferParameteri = 0;
	typedef void (CODEGEN_FUNCPTR *PFNNAMEDFRAMEBUFFERREADBUFFER)(GLuint, GLenum);
	PFNNAMEDFRAMEBUFFERREADBUFFER NamedFramebufferReadBuffer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNNAMEDFRAMEBUFFERRENDERBUFFER)(GLuint, GLenum, GLenum, GLuint);
	PFNNAMEDFRAMEBUFFERRENDERBUFFER NamedFramebufferRenderbuffer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNNAMEDFRAMEBUFFERTEXTURE)(GLuint, GLenum, GLuint, GLint);
	PFNNAMEDFRAMEBUFFERTEXTURE NamedFramebufferTexture = 0;
	typedef void (CODEGEN_FUNCPTR *PFNNAMEDFRAMEBUFFERTEXTURELAYER)(GLuint, GLenum, GLuint, GLint, GLint);
	PFNNAMEDFRAMEBUFFERTEXTURELAYER NamedFramebufferTextureLayer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNNAMEDRENDERBUFFERSTORAGE)(GLuint, GLenum, GLsizei, GLsizei);
	PFNNAMEDRENDERBUFFERSTORAGE NamedRenderbufferStorage = 0;
	typedef void (CODEGEN_FUNCPTR *PFNNAMEDRENDERBUFFERSTORAGEMULTISAMPLE)(GLuint, GLsizei, GLenum, GLsizei, GLsizei);
	PFNNAMEDRENDERBUFFERSTORAGEMULTISAMPLE NamedRenderbufferStorageMultisample = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXTUREBUFFER)(GLuint, GLenum, GLuint);
	PFNTEXTUREBUFFER TextureBuffer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXTUREBUFFERRANGE)(GLuint, GLenum, GLuint, GLintptr, GLsizeiptr);
	PFNTEXTUREBUFFERRANGE TextureBufferRange = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXTUREPARAMETERIIV)(GLuint, GLenum, const GLint *);
	PFNTEXTUREPARAMETERIIV TextureParameterIiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXTUREPARAMETERIUIV)(GLuint, GLenum, const GLuint *);
	PFNTEXTUREPARAMETERIUIV TextureParameterIuiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXTUREPARAMETERF)(GLuint, GLenum, GLfloat);
	PFNTEXTUREPARAMETERF TextureParameterf = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXTUREPARAMETERFV)(GLuint, GLenum, const GLfloat *);
	PFNTEXTUREPARAMETERFV TextureParameterfv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXTUREPARAMETERI)(GLuint, GLenum, GLint);
	PFNTEXTUREPARAMETERI TextureParameteri = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXTUREPARAMETERIV)(GLuint, GLenum, const GLint *);
	PFNTEXTUREPARAMETERIV TextureParameteriv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXTURESTORAGE1D)(GLuint, GLsizei, GLenum, GLsizei);
	PFNTEXTURESTORAGE1D TextureStorage1D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXTURESTORAGE2D)(GLuint, GLsizei, GLenum, GLsizei, GLsizei);
	PFNTEXTURESTORAGE2D TextureStorage2D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXTURESTORAGE2DMULTISAMPLE)(GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLboolean);
	PFNTEXTURESTORAGE2DMULTISAMPLE TextureStorage2DMultisample = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXTURESTORAGE3D)(GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLsizei);
	PFNTEXTURESTORAGE3D TextureStorage3D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXTURESTORAGE3DMULTISAMPLE)(GLuint, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean);
	PFNTEXTURESTORAGE3DMULTISAMPLE TextureStorage3DMultisample = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXTURESUBIMAGE1D)(GLuint, GLint, GLint, GLsizei, GLenum, GLenum, const void *);
	PFNTEXTURESUBIMAGE1D TextureSubImage1D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXTURESUBIMAGE2D)(GLuint, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void *);
	PFNTEXTURESUBIMAGE2D TextureSubImage2D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXTURESUBIMAGE3D)(GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void *);
	PFNTEXTURESUBIMAGE3D TextureSubImage3D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTRANSFORMFEEDBACKBUFFERBASE)(GLuint, GLuint, GLuint);
	PFNTRANSFORMFEEDBACKBUFFERBASE TransformFeedbackBufferBase = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTRANSFORMFEEDBACKBUFFERRANGE)(GLuint, GLuint, GLuint, GLintptr, GLsizeiptr);
	PFNTRANSFORMFEEDBACKBUFFERRANGE TransformFeedbackBufferRange = 0;
	typedef GLboolean (CODEGEN_FUNCPTR *PFNUNMAPNAMEDBUFFER)(GLuint);
	PFNUNMAPNAMEDBUFFER UnmapNamedBuffer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXARRAYATTRIBBINDING)(GLuint, GLuint, GLuint);
	PFNVERTEXARRAYATTRIBBINDING VertexArrayAttribBinding = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXARRAYATTRIBFORMAT)(GLuint, GLuint, GLint, GLenum, GLboolean, GLuint);
	PFNVERTEXARRAYATTRIBFORMAT VertexArrayAttribFormat = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXARRAYATTRIBIFORMAT)(GLuint, GLuint, GLint, GLenum, GLuint);
	PFNVERTEXARRAYATTRIBIFORMAT VertexArrayAttribIFormat = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXARRAYATTRIBLFORMAT)(GLuint, GLuint, GLint, GLenum, GLuint);
	PFNVERTEXARRAYATTRIBLFORMAT VertexArrayAttribLFormat = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXARRAYBINDINGDIVISOR)(GLuint, GLuint, GLuint);
	PFNVERTEXARRAYBINDINGDIVISOR VertexArrayBindingDivisor = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXARRAYELEMENTBUFFER)(GLuint, GLuint);
	PFNVERTEXARRAYELEMENTBUFFER VertexArrayElementBuffer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXARRAYVERTEXBUFFER)(GLuint, GLuint, GLuint, GLintptr, GLsizei);
	PFNVERTEXARRAYVERTEXBUFFER VertexArrayVertexBuffer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXARRAYVERTEXBUFFERS)(GLuint, GLuint, GLsizei, const GLuint *, const GLintptr *, const GLsizei *);
	PFNVERTEXARRAYVERTEXBUFFERS VertexArrayVertexBuffers = 0;
	
	static int Load_ARB_direct_state_access()
	{
		int numFailed = 0;
		BindTextureUnit = reinterpret_cast<PFNBINDTEXTUREUNIT>(IntGetProcAddress("glBindTextureUnit"));
		if(!BindTextureUnit) ++numFailed;
		BlitNamedFramebuffer = reinterpret_cast<PFNBLITNAMEDFRAMEBUFFER>(IntGetProcAddress("glBlitNamedFramebuffer"));
		if(!BlitNamedFramebuffer) ++numFailed;
		CheckNamedFramebufferStatus = reinterpret_cast<PFNCHECKNAMEDFRAMEBUFFERSTATUS>(IntGetProcAddress("glCheckNamedFramebufferStatus"));
		if(!CheckNamedFramebufferStatus) ++numFailed;
		ClearNamedBufferData = reinterpret_cast<PFNCLEARNAMEDBUFFERDATA>(IntGetProcAddress("glClearNamedBufferData"));
		if(!ClearNamedBufferData) ++numFailed;
		ClearNamedBufferSubData = reinterpret_cast<PFNCLEARNAMEDBUFFERSUBDATA>(IntGetProcAddress("glClearNamedBufferSubData"));
		if(!ClearNamedBufferSubData) ++numFailed;
		ClearNamedFramebufferfi = reinterpret_cast<PFNCLEARNAMEDFRAMEBUFFERFI>(IntGetProcAddress("glClearNamedFramebufferfi"));
		if(!ClearNamedFramebufferfi) ++numFailed;
		ClearNamedFramebufferfv = reinterpret_cast<PFNCLEARNAMEDFRAMEBUFFERFV>(IntGetProcAddress("glClearNamedFramebufferfv"));
		if(!ClearNamedFramebufferfv) ++numFailed;
		ClearNamedFramebufferiv = reinterpret_cast<PFNCLEARNAMEDFRAMEBUFFERIV>(IntGetProcAddress("glClearNamedFramebufferiv"));
		if(!ClearNamedFramebufferiv) ++numFailed;
		ClearNamedFramebufferuiv = reinterpret_cast<PFNCLEARNAMEDFRAMEBUFFERUIV>(IntGetProcAddress("glClearNamedFramebufferuiv"));
		if(!ClearNamedFramebufferuiv) ++numFailed;
		CompressedTextureSubImage1D = reinterpret_cast<PFNCOMPRESSEDTEXTURESUBIMAGE1D>(IntGetProcAddress("glCompressedTextureSubImage1D"));
		if(!CompressedTextureSubImage1D) ++numFailed;
		CompressedTextureSubImage2D = reinterpret_cast<PFNCOMPRESSEDTEXTURESUBIMAGE2D>(IntGetProcAddress("glCompressedTextureSubImage2D"));
		if(!CompressedTextureSubImage2D) ++numFailed;
		CompressedTextureSubImage3D = reinterpret_cast<PFNCOMPRESSEDTEXTURESUBIMAGE3D>(IntGetProcAddress("glCompressedTextureSubImage3D"));
		if(!CompressedTextureSubImage3D) ++numFailed;
		CopyNamedBufferSubData = reinterpret_cast<PFNCOPYNAMEDBUFFERSUBDATA>(IntGetProcAddress("glCopyNamedBufferSubData"));
		if(!CopyNamedBufferSubData) ++numFailed;
		CopyTextureSubImage1D = reinterpret_cast<PFNCOPYTEXTURESUBIMAGE1D>(IntGetProcAddress("glCopyTextureSubImage1D"));
		if(!CopyTextureSubImage1D) ++numFailed;
		CopyTextureSubImage2D = reinterpret_cast<PFNCOPYTEXTURESUBIMAGE2D>(IntGetProcAddress("glCopyTextureSubImage2D"));
		if(!CopyTextureSubImage2D) ++numFailed;
		CopyTextureSubImage3D = reinterpret_cast<PFNCOPYTEXTURESUBIMAGE3D>(IntGetProcAddress("glCopyTextureSubImage3D"));
		if(!CopyTextureSubImage3D) ++numFailed;
		CreateBuffers = reinterpret_cast<PFNCREATEBUFFERS>(IntGetProcAddress("glCreateBuffers"));
		if(!CreateBuffers) ++numFailed;
		CreateFramebuffers = reinterpret_cast<PFNCREATEFRAMEBUFFERS>(IntGetProcAddress("glCreateFramebuffers"));
		if(!CreateFramebuffers) ++numFailed;
		CreateProgramPipelines = reinterpret_cast<PFNCREATEPROGRAMPIPELINES>(IntGetProcAddress("glCreateProgramPipelines"));
		if(!CreateProgramPipelines) ++numFailed;
		CreateQueries = reinterpret_cast<PFNCREATEQUERIES>(IntGetProcAddress("glCreateQueries"));
		if(!CreateQueries) ++numFailed;
		CreateRenderbuffers = reinterpret_cast<PFNCREATERENDERBUFFERS>(IntGetProcAddress("glCreateRenderbuffers"));
		if(!CreateRenderbuffers) ++numFailed;
		CreateSamplers = reinterpret_cast<PFNCREATESAMPLERS>(IntGetProcAddress("glCreateSamplers"));
		if(!CreateSamplers) ++numFailed;
		CreateTextures = reinterpret_cast<PFNCREATETEXTURES>(IntGetProcAddress("glCreateTextures"));
		if(!CreateTextures) ++numFailed;
		CreateTransformFeedbacks = reinterpret_cast<PFNCREATETRANSFORMFEEDBACKS>(IntGetProcAddress("glCreateTransformFeedbacks"));
		if(!CreateTransformFeedbacks) ++numFailed;
		CreateVertexArrays = reinterpret_cast<PFNCREATEVERTEXARRAYS>(IntGetProcAddress("glCreateVertexArrays"));
		if(!CreateVertexArrays) ++numFailed;
		DisableVertexArrayAttrib = reinterpret_cast<PFNDISABLEVERTEXARRAYATTRIB>(IntGetProcAddress("glDisableVertexArrayAttrib"));
		if(!DisableVertexArrayAttrib) ++numFailed;
		EnableVertexArrayAttrib = reinterpret_cast<PFNENABLEVERTEXARRAYATTRIB>(IntGetProcAddress("glEnableVertexArrayAttrib"));
		if(!EnableVertexArrayAttrib) ++numFailed;
		FlushMappedNamedBufferRange = reinterpret_cast<PFNFLUSHMAPPEDNAMEDBUFFERRANGE>(IntGetProcAddress("glFlushMappedNamedBufferRange"));
		if(!FlushMappedNamedBufferRange) ++numFailed;
		GenerateTextureMipmap = reinterpret_cast<PFNGENERATETEXTUREMIPMAP>(IntGetProcAddress("glGenerateTextureMipmap"));
		if(!GenerateTextureMipmap) ++numFailed;
		GetCompressedTextureImage = reinterpret_cast<PFNGETCOMPRESSEDTEXTUREIMAGE>(IntGetProcAddress("glGetCompressedTextureImage"));
		if(!GetCompressedTextureImage) ++numFailed;
		GetNamedBufferParameteri64v = reinterpret_cast<PFNGETNAMEDBUFFERPARAMETERI64V>(IntGetProcAddress("glGetNamedBufferParameteri64v"));
		if(!GetNamedBufferParameteri64v) ++numFailed;
		GetNamedBufferParameteriv = reinterpret_cast<PFNGETNAMEDBUFFERPARAMETERIV>(IntGetProcAddress("glGetNamedBufferParameteriv"));
		if(!GetNamedBufferParameteriv) ++numFailed;
		GetNamedBufferPointerv = reinterpret_cast<PFNGETNAMEDBUFFERPOINTERV>(IntGetProcAddress("glGetNamedBufferPointerv"));
		if(!GetNamedBufferPointerv) ++numFailed;
		GetNamedBufferSubData = reinterpret_cast<PFNGETNAMEDBUFFERSUBDATA>(IntGetProcAddress("glGetNamedBufferSubData"));
		if(!GetNamedBufferSubData) ++numFailed;
		GetNamedFramebufferAttachmentParameteriv = reinterpret_cast<PFNGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIV>(IntGetProcAddress("glGetNamedFramebufferAttachmentParameteriv"));
		if(!GetNamedFramebufferAttachmentParameteriv) ++numFailed;
		GetNamedFramebufferParameteriv = reinterpret_cast<PFNGETNAMEDFRAMEBUFFERPARAMETERIV>(IntGetProcAddress("glGetNamedFramebufferParameteriv"));
		if(!GetNamedFramebufferParameteriv) ++numFailed;
		GetNamedRenderbufferParameteriv = reinterpret_cast<PFNGETNAMEDRENDERBUFFERPARAMETERIV>(IntGetProcAddress("glGetNamedRenderbufferParameteriv"));
		if(!GetNamedRenderbufferParameteriv) ++numFailed;
		GetQueryBufferObjecti64v = reinterpret_cast<PFNGETQUERYBUFFEROBJECTI64V>(IntGetProcAddress("glGetQueryBufferObjecti64v"));
		if(!GetQueryBufferObjecti64v) ++numFailed;
		GetQueryBufferObjectiv = reinterpret_cast<PFNGETQUERYBUFFEROBJECTIV>(IntGetProcAddress("glGetQueryBufferObjectiv"));
		if(!GetQueryBufferObjectiv) ++numFailed;
		GetQueryBufferObjectui64v = reinterpret_cast<PFNGETQUERYBUFFEROBJECTUI64V>(IntGetProcAddress("glGetQueryBufferObjectui64v"));
		if(!GetQueryBufferObjectui64v) ++numFailed;
		GetQueryBufferObjectuiv = reinterpret_cast<PFNGETQUERYBUFFEROBJECTUIV>(IntGetProcAddress("glGetQueryBufferObjectuiv"));
		if(!GetQueryBufferObjectuiv) ++numFailed;
		GetTextureImage = reinterpret_cast<PFNGETTEXTUREIMAGE>(IntGetProcAddress("glGetTextureImage"));
		if(!GetTextureImage) ++numFailed;
		GetTextureLevelParameterfv = reinterpret_cast<PFNGETTEXTURELEVELPARAMETERFV>(IntGetProcAddress("glGetTextureLevelParameterfv"));
		if(!GetTextureLevelParameterfv) ++numFailed;
		GetTextureLevelParameteriv = reinterpret_cast<PFNGETTEXTURELEVELPARAMETERIV>(IntGetProcAddress("glGetTextureLevelParameteriv"));
		if(!GetTextureLevelParameteriv) ++numFailed;
		GetTextureParameterIiv = reinterpret_cast<PFNGETTEXTUREPARAMETERIIV>(IntGetProcAddress("glGetTextureParameterIiv"));
		if(!GetTextureParameterIiv) ++numFailed;
		GetTextureParameterIuiv = reinterpret_cast<PFNGETTEXTUREPARAMETERIUIV>(IntGetProcAddress("glGetTextureParameterIuiv"));
		if(!GetTextureParameterIuiv) ++numFailed;
		GetTextureParameterfv = reinterpret_cast<PFNGETTEXTUREPARAMETERFV>(IntGetProcAddress("glGetTextureParameterfv"));
		if(!GetTextureParameterfv) ++numFailed;
		GetTextureParameteriv = reinterpret_cast<PFNGETTEXTUREPARAMETERIV>(IntGetProcAddress("glGetTextureParameteriv"));
		if(!GetTextureParameteriv) ++numFailed;
		GetTransformFeedbacki64_v = reinterpret_cast<PFNGETTRANSFORMFEEDBACKI64_V>(IntGetProcAddress("glGetTransformFeedbacki64_v"));
		if(!GetTransformFeedbacki64_v) ++numFailed;
		GetTransformFeedbacki_v = reinterpret_cast<PFNGETTRANSFORMFEEDBACKI_V>(IntGetProcAddress("glGetTransformFeedbacki_v"));
		if(!GetTransformFeedbacki_v) ++numFailed;
		GetTransformFeedbackiv = reinterpret_cast<PFNGETTRANSFORMFEEDBACKIV>(IntGetProcAddress("glGetTransformFeedbackiv"));
		if(!GetTransformFeedbackiv) ++numFailed;
		GetVertexArrayIndexed64iv = reinterpret_cast<PFNGETVERTEXARRAYINDEXED64IV>(IntGetProcAddress("glGetVertexArrayIndexed64iv"));
		if(!GetVertexArrayIndexed64iv) ++numFailed;
		GetVertexArrayIndexediv = reinterpret_cast<PFNGETVERTEXARRAYINDEXEDIV>(IntGetProcAddress("glGetVertexArrayIndexediv"));
		if(!GetVertexArrayIndexediv) ++numFailed;
		GetVertexArrayiv = reinterpret_cast<PFNGETVERTEXARRAYIV>(IntGetProcAddress("glGetVertexArrayiv"));
		if(!GetVertexArrayiv) ++numFailed;
		InvalidateNamedFramebufferData = reinterpret_cast<PFNINVALIDATENAMEDFRAMEBUFFERDATA>(IntGetProcAddress("glInvalidateNamedFramebufferData"));
		if(!InvalidateNamedFramebufferData) ++numFailed;
		InvalidateNamedFramebufferSubData = reinterpret_cast<PFNINVALIDATENAMEDFRAMEBUFFERSUBDATA>(IntGetProcAddress("glInvalidateNamedFramebufferSubData"));
		if(!InvalidateNamedFramebufferSubData) ++numFailed;
		MapNamedBuffer = reinterpret_cast<PFNMAPNAMEDBUFFER>(IntGetProcAddress("glMapNamedBuffer"));
		if(!MapNamedBuffer) ++numFailed;
		MapNamedBufferRange = reinterpret_cast<PFNMAPNAMEDBUFFERRANGE>(IntGetProcAddress("glMapNamedBufferRange"));
		if(!MapNamedBufferRange) ++numFailed;
		NamedBufferData = reinterpret_cast<PFNNAMEDBUFFERDATA>(IntGetProcAddress("glNamedBufferData"));
		if(!NamedBufferData) ++numFailed;
		NamedBufferStorage = reinterpret_cast<PFNNAMEDBUFFERSTORAGE>(IntGetProcAddress("glNamedBufferStorage"));
		if(!NamedBufferStorage) ++numFailed;
		NamedBufferSubData = reinterpret_cast<PFNNAMEDBUFFERSUBDATA>(IntGetProcAddress("glNamedBufferSubData"));
		if(!NamedBufferSubData) ++numFailed;
		NamedFramebufferDrawBuffer = reinterpret_cast<PFNNAMEDFRAMEBUFFERDRAWBUFFER>(IntGetProcAddress("glNamedFramebufferDrawBuffer"));
		if(!NamedFramebufferDrawBuffer) ++numFailed;
		NamedFramebufferDrawBuffers = reinterpret_cast<PFNNAMEDFRAMEBUFFERDRAWBUFFERS>(IntGetProcAddress("glNamedFramebufferDrawBuffers"));
		if(!NamedFramebufferDrawBuffers) ++numFailed;
		NamedFramebufferParameteri = reinterpret_cast<PFNNAMEDFRAMEBUFFERPARAMETERI>(IntGetProcAddress("glNamedFramebufferParameteri"));
		if(!NamedFramebufferParameteri) ++numFailed;
		NamedFramebufferReadBuffer = reinterpret_cast<PFNNAMEDFRAMEBUFFERREADBUFFER>(IntGetProcAddress("glNamedFramebufferReadBuffer"));
		if(!NamedFramebufferReadBuffer) ++numFailed;
		NamedFramebufferRenderbuffer = reinterpret_cast<PFNNAMEDFRAMEBUFFERRENDERBUFFER>(IntGetProcAddress("glNamedFramebufferRenderbuffer"));
		if(!NamedFramebufferRenderbuffer) ++numFailed;
		NamedFramebufferTexture = reinterpret_cast<PFNNAMEDFRAMEBUFFERTEXTURE>(IntGetProcAddress("glNamedFramebufferTexture"));
		if(!NamedFramebufferTexture) ++numFailed;
		NamedFramebufferTextureLayer = reinterpret_cast<PFNNAMEDFRAMEBUFFERTEXTURELAYER>(IntGetProcAddress("glNamedFramebufferTextureLayer"));
		if(!NamedFramebufferTextureLayer) ++numFailed;
		NamedRenderbufferStorage = reinterpret_cast<PFNNAMEDRENDERBUFFERSTORAGE>(IntGetProcAddress("glNamedRenderbufferStorage"));
		if(!NamedRenderbufferStorage) ++numFailed;
		NamedRenderbufferStorageMultisample = reinterpret_cast<PFNNAMEDRENDERBUFFERSTORAGEMULTISAMPLE>(IntGetProcAddress("glNamedRenderbufferStorageMultisample"));
		if(!NamedRenderbufferStorageMultisample) ++numFailed;
		TextureBuffer = reinterpret_cast<PFNTEXTUREBUFFER>(IntGetProcAddress("glTextureBuffer"));
		if(!TextureBuffer) ++numFailed;
		TextureBufferRange = reinterpret_cast<PFNTEXTUREBUFFERRANGE>(IntGetProcAddress("glTextureBufferRange"));
		if(!TextureBufferRange) ++numFailed;
		TextureParameterIiv = reinterpret_cast<PFNTEXTUREPARAMETERIIV>(IntGetProcAddress("glTextureParameterIiv"));
		if(!TextureParameterIiv) ++numFailed;
		TextureParameterIuiv = reinterpret_cast<PFNTEXTUREPARAMETERIUIV>(IntGetProcAddress("glTextureParameterIuiv"));
		if(!TextureParameterIuiv) ++numFailed;
		TextureParameterf = reinterpret_cast<PFNTEXTUREPARAMETERF>(IntGetProcAddress("glTextureParameterf"));
		if(!TextureParameterf) ++numFailed;
		TextureParameterfv = reinterpret_cast<PFNTEXTUREPARAMETERFV>(IntGetProcAddress("glTextureParameterfv"));
		if(!TextureParameterfv) ++numFailed;
		TextureParameteri = reinterpret_cast<PFNTEXTUREPARAMETERI>(IntGetProcAddress("glTextureParameteri"));
		if(!TextureParameteri) ++numFailed;
		TextureParameteriv = reinterpret_cast<PFNTEXTUREPARAMETERIV>(IntGetProcAddress("glTextureParameteriv"));
		if(!TextureParameteriv) ++numFailed;
		TextureStorage1D = reinterpret_cast<PFNTEXTURESTORAGE1D>(IntGetProcAddress("glTextureStorage1D"));
		if(!TextureStorage1D) ++numFailed;
		TextureStorage2D = reinterpret_cast<PFNTEXTURESTORAGE2D>(IntGetProcAddress("glTextureStorage2D"));
		if(!TextureStorage2D) ++numFailed;
		TextureStorage2DMultisample = reinterpret_cast<PFNTEXTURESTORAGE2DMULTISAMPLE>(IntGetProcAddress("glTextureStorage2DMultisample"));
		if(!TextureStorage2DMultisample) ++numFailed;
		TextureStorage3D = reinterpret_cast<PFNTEXTURESTORAGE3D>(IntGetProcAddress("glTextureStorage3D"));
		if(!TextureStorage3D) ++numFailed;
		TextureStorage3DMultisample = reinterpret_cast<PFNTEXTURESTORAGE3DMULTISAMPLE>(IntGetProcAddress("glTextureStorage3DMultisample"));
		if(!TextureStorage3DMultisample) ++numFailed;
		TextureSubImage1D = reinterpret_cast<PFNTEXTURESUBIMAGE1D>(IntGetProcAddress("glTextureSubImage1D"));
		if(!TextureSubImage1D) ++numFailed;
		TextureSubImage2D = reinterpret_cast<PFNTEXTURESUBIMAGE2D>(IntGetProcAddress("glTextureSubImage2D"));
		if(!TextureSubImage2D) ++numFailed;
		TextureSubImage3D = reinterpret_cast<PFNTEXTURESUBIMAGE3D>(IntGetProcAddress("glTextureSubImage3D"));
		if(!TextureSubImage3D) ++numFailed;
		TransformFeedbackBufferBase = reinterpret_cast<PFNTRANSFORMFEEDBACKBUFFERBASE>(IntGetProcAddress("glTransformFeedbackBufferBase"));
		if(!TransformFeedbackBufferBase) ++numFailed;
		TransformFeedbackBufferRange = reinterpret_cast<PFNTRANSFORMFEEDBACKBUFFERRANGE>(IntGetProcAddress("glTransformFeedbackBufferRange"));
		if(!TransformFeedbackBufferRange) ++numFailed;
		UnmapNamedBuffer = reinterpret_cast<PFNUNMAPNAMEDBUFFER>(IntGetProcAddress("glUnmapNamedBuffer"));
		if(!UnmapNamedBuffer) ++numFailed;
		VertexArrayAttribBinding = reinterpret_cast<PFNVERTEXARRAYATTRIBBINDING>(IntGetProcAddress("glVertexArrayAttribBinding"));
		if(!VertexArrayAttribBinding) ++numFailed;
		VertexArrayAttribFormat = reinterpret_cast<PFNVERTEXARRAYATTRIBFORMAT>(IntGetProcAddress("glVertexArrayAttribFormat"));
		if(!VertexArrayAttribFormat) ++numFailed;
		VertexArrayAttribIFormat = reinterpret_cast<PFNVERTEXARRAYATTRIBIFORMAT>(IntGetProcAddress("glVertexArrayAttribIFormat"));
		if(!VertexArrayAttribIFormat) ++numFailed;
		VertexArrayAttribLFormat = reinterpret_cast<PFNVERTEXARRAYATTRIBLFORMAT>(IntGetProcAddress("glVertexArrayAttribLFormat"));
		if(!VertexArrayAttribLFormat) ++numFailed;
		VertexArrayBindingDivisor = reinterpret_cast<PFNVERTEXARRAYBINDINGDIVISOR>(IntGetProcAddress("glVertexArrayBindingDivisor"));
		if(!VertexArrayBindingDivisor) ++numFailed;
		VertexArrayElementBuffer = reinterpret_cast<PFNVERTEXARRAYELEMENTBUFFER>(IntGetProcAddress("glVertexArrayElementBuffer"));
		if(!VertexArrayElementBuffer) ++numFailed;
		VertexArrayVertexBuffer = reinterpret_cast<PFNVERTEXARRAYVERTEXBUFFER>(IntGetProcAddress("glVertexArrayVertexBuffer"));
		if(!VertexArrayVertexBuffer) ++numFailed;
		VertexArrayVertexBuffers = reinterpret_cast<PFNVERTEXARRAYVERTEXBUFFERS>(IntGetProcAddress("glVertexArrayVertexBuffers"));
		if(!VertexArrayVertexBuffers) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNGETCOMPRESSEDTEXTURESUBIMAGE)(GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLsizei, void *);
	PFNGETCOMPRESSEDTEXTURESUBIMAGE GetCompressedTextureSubImage = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTEXTURESUBIMAGE)(GLuint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, GLsizei, void *);
	PFNGETTEXTURESUBIMAGE GetTextureSubImage = 0;
	
	static int Load_ARB_get_texture_sub_image()
	{
		int numFailed = 0;
		GetCompressedTextureSubImage = reinterpret_cast<PFNGETCOMPRESSEDTEXTURESUBIMAGE>(IntGetProcAddress("glGetCompressedTextureSubImage"));
		if(!GetCompressedTextureSubImage) ++numFailed;
		GetTextureSubImage = reinterpret_cast<PFNGETTEXTURESUBIMAGE>(IntGetProcAddress("glGetTextureSubImage"));
		if(!GetTextureSubImage) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNTEXTUREBARRIER)(void);
	PFNTEXTUREBARRIER TextureBarrier = 0;
	
	static int Load_ARB_texture_barrier()
	{
		int numFailed = 0;
		TextureBarrier = reinterpret_cast<PFNTEXTUREBARRIER>(IntGetProcAddress("glTextureBarrier"));
		if(!TextureBarrier) ++numFailed;
		return numFailed;
	}
	
	typedef GLenum (CODEGEN_FUNCPTR *PFNGETGRAPHICSRESETSTATUS)(void);
	PFNGETGRAPHICSRESETSTATUS GetGraphicsResetStatus = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETNUNIFORMFV)(GLuint, GLint, GLsizei, GLfloat *);
	PFNGETNUNIFORMFV GetnUniformfv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETNUNIFORMIV)(GLuint, GLint, GLsizei, GLint *);
	PFNGETNUNIFORMIV GetnUniformiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETNUNIFORMUIV)(GLuint, GLint, GLsizei, GLuint *);
	PFNGETNUNIFORMUIV GetnUniformuiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNREADNPIXELS)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLsizei, void *);
	PFNREADNPIXELS ReadnPixels = 0;
	
	static int Load_KHR_robustness()
	{
		int numFailed = 0;
		GetGraphicsResetStatus = reinterpret_cast<PFNGETGRAPHICSRESETSTATUS>(IntGetProcAddress("glGetGraphicsResetStatus"));
		if(!GetGraphicsResetStatus) ++numFailed;
		GetnUniformfv = reinterpret_cast<PFNGETNUNIFORMFV>(IntGetProcAddress("glGetnUniformfv"));
		if(!GetnUniformfv) ++numFailed;
		GetnUniformiv = reinterpret_cast<PFNGETNUNIFORMIV>(IntGetProcAddress("glGetnUniformiv"));
		if(!GetnUniformiv) ++numFailed;
		GetnUniformuiv = reinterpret_cast<PFNGETNUNIFORMUIV>(IntGetProcAddress("glGetnUniformuiv"));
		if(!GetnUniformuiv) ++numFailed;
		ReadnPixels = reinterpret_cast<PFNREADNPIXELS>(IntGetProcAddress("glReadnPixels"));
		if(!ReadnPixels) ++numFailed;
		return numFailed;
	}
	
	typedef void (CODEGEN_FUNCPTR *PFNBLENDFUNC)(GLenum, GLenum);
	PFNBLENDFUNC BlendFunc = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCLEAR)(GLbitfield);
	PFNCLEAR Clear = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCLEARCOLOR)(GLfloat, GLfloat, GLfloat, GLfloat);
	PFNCLEARCOLOR ClearColor = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCLEARDEPTH)(GLdouble);
	PFNCLEARDEPTH ClearDepth = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCLEARSTENCIL)(GLint);
	PFNCLEARSTENCIL ClearStencil = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOLORMASK)(GLboolean, GLboolean, GLboolean, GLboolean);
	PFNCOLORMASK ColorMask = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCULLFACE)(GLenum);
	PFNCULLFACE CullFace = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDEPTHFUNC)(GLenum);
	PFNDEPTHFUNC DepthFunc = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDEPTHMASK)(GLboolean);
	PFNDEPTHMASK DepthMask = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDEPTHRANGE)(GLdouble, GLdouble);
	PFNDEPTHRANGE DepthRange = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDISABLE)(GLenum);
	PFNDISABLE Disable = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDRAWBUFFER)(GLenum);
	PFNDRAWBUFFER DrawBuffer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNENABLE)(GLenum);
	PFNENABLE Enable = 0;
	typedef void (CODEGEN_FUNCPTR *PFNFINISH)(void);
	PFNFINISH Finish = 0;
	typedef void (CODEGEN_FUNCPTR *PFNFLUSH)(void);
	PFNFLUSH Flush = 0;
	typedef void (CODEGEN_FUNCPTR *PFNFRONTFACE)(GLenum);
	PFNFRONTFACE FrontFace = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETBOOLEANV)(GLenum, GLboolean *);
	PFNGETBOOLEANV GetBooleanv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETDOUBLEV)(GLenum, GLdouble *);
	PFNGETDOUBLEV GetDoublev = 0;
	typedef GLenum (CODEGEN_FUNCPTR *PFNGETERROR)(void);
	PFNGETERROR GetError = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETFLOATV)(GLenum, GLfloat *);
	PFNGETFLOATV GetFloatv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETINTEGERV)(GLenum, GLint *);
	PFNGETINTEGERV GetIntegerv = 0;
	typedef const GLubyte * (CODEGEN_FUNCPTR *PFNGETSTRING)(GLenum);
	PFNGETSTRING GetString = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTEXIMAGE)(GLenum, GLint, GLenum, GLenum, void *);
	PFNGETTEXIMAGE GetTexImage = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTEXLEVELPARAMETERFV)(GLenum, GLint, GLenum, GLfloat *);
	PFNGETTEXLEVELPARAMETERFV GetTexLevelParameterfv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTEXLEVELPARAMETERIV)(GLenum, GLint, GLenum, GLint *);
	PFNGETTEXLEVELPARAMETERIV GetTexLevelParameteriv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTEXPARAMETERFV)(GLenum, GLenum, GLfloat *);
	PFNGETTEXPARAMETERFV GetTexParameterfv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTEXPARAMETERIV)(GLenum, GLenum, GLint *);
	PFNGETTEXPARAMETERIV GetTexParameteriv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNHINT)(GLenum, GLenum);
	PFNHINT Hint = 0;
	typedef GLboolean (CODEGEN_FUNCPTR *PFNISENABLED)(GLenum);
	PFNISENABLED IsEnabled = 0;
	typedef void (CODEGEN_FUNCPTR *PFNLINEWIDTH)(GLfloat);
	PFNLINEWIDTH LineWidth = 0;
	typedef void (CODEGEN_FUNCPTR *PFNLOGICOP)(GLenum);
	PFNLOGICOP LogicOp = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPIXELSTOREF)(GLenum, GLfloat);
	PFNPIXELSTOREF PixelStoref = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPIXELSTOREI)(GLenum, GLint);
	PFNPIXELSTOREI PixelStorei = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPOINTSIZE)(GLfloat);
	PFNPOINTSIZE PointSize = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPOLYGONMODE)(GLenum, GLenum);
	PFNPOLYGONMODE PolygonMode = 0;
	typedef void (CODEGEN_FUNCPTR *PFNREADBUFFER)(GLenum);
	PFNREADBUFFER ReadBuffer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNREADPIXELS)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void *);
	PFNREADPIXELS ReadPixels = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSCISSOR)(GLint, GLint, GLsizei, GLsizei);
	PFNSCISSOR Scissor = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSTENCILFUNC)(GLenum, GLint, GLuint);
	PFNSTENCILFUNC StencilFunc = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSTENCILMASK)(GLuint);
	PFNSTENCILMASK StencilMask = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSTENCILOP)(GLenum, GLenum, GLenum);
	PFNSTENCILOP StencilOp = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXIMAGE1D)(GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const void *);
	PFNTEXIMAGE1D TexImage1D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXIMAGE2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *);
	PFNTEXIMAGE2D TexImage2D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXPARAMETERF)(GLenum, GLenum, GLfloat);
	PFNTEXPARAMETERF TexParameterf = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXPARAMETERFV)(GLenum, GLenum, const GLfloat *);
	PFNTEXPARAMETERFV TexParameterfv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXPARAMETERI)(GLenum, GLenum, GLint);
	PFNTEXPARAMETERI TexParameteri = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXPARAMETERIV)(GLenum, GLenum, const GLint *);
	PFNTEXPARAMETERIV TexParameteriv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVIEWPORT)(GLint, GLint, GLsizei, GLsizei);
	PFNVIEWPORT Viewport = 0;
	
	typedef void (CODEGEN_FUNCPTR *PFNBINDTEXTURE)(GLenum, GLuint);
	PFNBINDTEXTURE BindTexture = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOPYTEXIMAGE1D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint);
	PFNCOPYTEXIMAGE1D CopyTexImage1D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOPYTEXIMAGE2D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
	PFNCOPYTEXIMAGE2D CopyTexImage2D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOPYTEXSUBIMAGE1D)(GLenum, GLint, GLint, GLint, GLint, GLsizei);
	PFNCOPYTEXSUBIMAGE1D CopyTexSubImage1D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOPYTEXSUBIMAGE2D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
	PFNCOPYTEXSUBIMAGE2D CopyTexSubImage2D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDELETETEXTURES)(GLsizei, const GLuint *);
	PFNDELETETEXTURES DeleteTextures = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDRAWARRAYS)(GLenum, GLint, GLsizei);
	PFNDRAWARRAYS DrawArrays = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDRAWELEMENTS)(GLenum, GLsizei, GLenum, const void *);
	PFNDRAWELEMENTS DrawElements = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGENTEXTURES)(GLsizei, GLuint *);
	PFNGENTEXTURES GenTextures = 0;
	typedef GLboolean (CODEGEN_FUNCPTR *PFNISTEXTURE)(GLuint);
	PFNISTEXTURE IsTexture = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPOLYGONOFFSET)(GLfloat, GLfloat);
	PFNPOLYGONOFFSET PolygonOffset = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXSUBIMAGE1D)(GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const void *);
	PFNTEXSUBIMAGE1D TexSubImage1D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXSUBIMAGE2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void *);
	PFNTEXSUBIMAGE2D TexSubImage2D = 0;
	
	typedef void (CODEGEN_FUNCPTR *PFNCOPYTEXSUBIMAGE3D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
	PFNCOPYTEXSUBIMAGE3D CopyTexSubImage3D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDRAWRANGEELEMENTS)(GLenum, GLuint, GLuint, GLsizei, GLenum, const void *);
	PFNDRAWRANGEELEMENTS DrawRangeElements = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXIMAGE3D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *);
	PFNTEXIMAGE3D TexImage3D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXSUBIMAGE3D)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void *);
	PFNTEXSUBIMAGE3D TexSubImage3D = 0;
	
	typedef void (CODEGEN_FUNCPTR *PFNACTIVETEXTURE)(GLenum);
	PFNACTIVETEXTURE ActiveTexture = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOMPRESSEDTEXIMAGE1D)(GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const void *);
	PFNCOMPRESSEDTEXIMAGE1D CompressedTexImage1D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOMPRESSEDTEXIMAGE2D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const void *);
	PFNCOMPRESSEDTEXIMAGE2D CompressedTexImage2D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOMPRESSEDTEXIMAGE3D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const void *);
	PFNCOMPRESSEDTEXIMAGE3D CompressedTexImage3D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOMPRESSEDTEXSUBIMAGE1D)(GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const void *);
	PFNCOMPRESSEDTEXSUBIMAGE1D CompressedTexSubImage1D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOMPRESSEDTEXSUBIMAGE2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const void *);
	PFNCOMPRESSEDTEXSUBIMAGE2D CompressedTexSubImage2D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOMPRESSEDTEXSUBIMAGE3D)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void *);
	PFNCOMPRESSEDTEXSUBIMAGE3D CompressedTexSubImage3D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETCOMPRESSEDTEXIMAGE)(GLenum, GLint, void *);
	PFNGETCOMPRESSEDTEXIMAGE GetCompressedTexImage = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSAMPLECOVERAGE)(GLfloat, GLboolean);
	PFNSAMPLECOVERAGE SampleCoverage = 0;
	
	typedef void (CODEGEN_FUNCPTR *PFNBLENDCOLOR)(GLfloat, GLfloat, GLfloat, GLfloat);
	PFNBLENDCOLOR BlendColor = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBLENDEQUATION)(GLenum);
	PFNBLENDEQUATION BlendEquation = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBLENDFUNCSEPARATE)(GLenum, GLenum, GLenum, GLenum);
	PFNBLENDFUNCSEPARATE BlendFuncSeparate = 0;
	typedef void (CODEGEN_FUNCPTR *PFNMULTIDRAWARRAYS)(GLenum, const GLint *, const GLsizei *, GLsizei);
	PFNMULTIDRAWARRAYS MultiDrawArrays = 0;
	typedef void (CODEGEN_FUNCPTR *PFNMULTIDRAWELEMENTS)(GLenum, const GLsizei *, GLenum, const void *const*, GLsizei);
	PFNMULTIDRAWELEMENTS MultiDrawElements = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPOINTPARAMETERF)(GLenum, GLfloat);
	PFNPOINTPARAMETERF PointParameterf = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPOINTPARAMETERFV)(GLenum, const GLfloat *);
	PFNPOINTPARAMETERFV PointParameterfv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPOINTPARAMETERI)(GLenum, GLint);
	PFNPOINTPARAMETERI PointParameteri = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPOINTPARAMETERIV)(GLenum, const GLint *);
	PFNPOINTPARAMETERIV PointParameteriv = 0;
	
	typedef void (CODEGEN_FUNCPTR *PFNBEGINQUERY)(GLenum, GLuint);
	PFNBEGINQUERY BeginQuery = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBINDBUFFER)(GLenum, GLuint);
	PFNBINDBUFFER BindBuffer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBUFFERDATA)(GLenum, GLsizeiptr, const void *, GLenum);
	PFNBUFFERDATA BufferData = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBUFFERSUBDATA)(GLenum, GLintptr, GLsizeiptr, const void *);
	PFNBUFFERSUBDATA BufferSubData = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDELETEBUFFERS)(GLsizei, const GLuint *);
	PFNDELETEBUFFERS DeleteBuffers = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDELETEQUERIES)(GLsizei, const GLuint *);
	PFNDELETEQUERIES DeleteQueries = 0;
	typedef void (CODEGEN_FUNCPTR *PFNENDQUERY)(GLenum);
	PFNENDQUERY EndQuery = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGENBUFFERS)(GLsizei, GLuint *);
	PFNGENBUFFERS GenBuffers = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGENQUERIES)(GLsizei, GLuint *);
	PFNGENQUERIES GenQueries = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETBUFFERPARAMETERIV)(GLenum, GLenum, GLint *);
	PFNGETBUFFERPARAMETERIV GetBufferParameteriv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETBUFFERPOINTERV)(GLenum, GLenum, void **);
	PFNGETBUFFERPOINTERV GetBufferPointerv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETBUFFERSUBDATA)(GLenum, GLintptr, GLsizeiptr, void *);
	PFNGETBUFFERSUBDATA GetBufferSubData = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETQUERYOBJECTIV)(GLuint, GLenum, GLint *);
	PFNGETQUERYOBJECTIV GetQueryObjectiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETQUERYOBJECTUIV)(GLuint, GLenum, GLuint *);
	PFNGETQUERYOBJECTUIV GetQueryObjectuiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETQUERYIV)(GLenum, GLenum, GLint *);
	PFNGETQUERYIV GetQueryiv = 0;
	typedef GLboolean (CODEGEN_FUNCPTR *PFNISBUFFER)(GLuint);
	PFNISBUFFER IsBuffer = 0;
	typedef GLboolean (CODEGEN_FUNCPTR *PFNISQUERY)(GLuint);
	PFNISQUERY IsQuery = 0;
	typedef void * (CODEGEN_FUNCPTR *PFNMAPBUFFER)(GLenum, GLenum);
	PFNMAPBUFFER MapBuffer = 0;
	typedef GLboolean (CODEGEN_FUNCPTR *PFNUNMAPBUFFER)(GLenum);
	PFNUNMAPBUFFER UnmapBuffer = 0;
	
	typedef void (CODEGEN_FUNCPTR *PFNATTACHSHADER)(GLuint, GLuint);
	PFNATTACHSHADER AttachShader = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBINDATTRIBLOCATION)(GLuint, GLuint, const GLchar *);
	PFNBINDATTRIBLOCATION BindAttribLocation = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBLENDEQUATIONSEPARATE)(GLenum, GLenum);
	PFNBLENDEQUATIONSEPARATE BlendEquationSeparate = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOMPILESHADER)(GLuint);
	PFNCOMPILESHADER CompileShader = 0;
	typedef GLuint (CODEGEN_FUNCPTR *PFNCREATEPROGRAM)(void);
	PFNCREATEPROGRAM CreateProgram = 0;
	typedef GLuint (CODEGEN_FUNCPTR *PFNCREATESHADER)(GLenum);
	PFNCREATESHADER CreateShader = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDELETEPROGRAM)(GLuint);
	PFNDELETEPROGRAM DeleteProgram = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDELETESHADER)(GLuint);
	PFNDELETESHADER DeleteShader = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDETACHSHADER)(GLuint, GLuint);
	PFNDETACHSHADER DetachShader = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDISABLEVERTEXATTRIBARRAY)(GLuint);
	PFNDISABLEVERTEXATTRIBARRAY DisableVertexAttribArray = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDRAWBUFFERS)(GLsizei, const GLenum *);
	PFNDRAWBUFFERS DrawBuffers = 0;
	typedef void (CODEGEN_FUNCPTR *PFNENABLEVERTEXATTRIBARRAY)(GLuint);
	PFNENABLEVERTEXATTRIBARRAY EnableVertexAttribArray = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETACTIVEATTRIB)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
	PFNGETACTIVEATTRIB GetActiveAttrib = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETACTIVEUNIFORM)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
	PFNGETACTIVEUNIFORM GetActiveUniform = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETATTACHEDSHADERS)(GLuint, GLsizei, GLsizei *, GLuint *);
	PFNGETATTACHEDSHADERS GetAttachedShaders = 0;
	typedef GLint (CODEGEN_FUNCPTR *PFNGETATTRIBLOCATION)(GLuint, const GLchar *);
	PFNGETATTRIBLOCATION GetAttribLocation = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETPROGRAMINFOLOG)(GLuint, GLsizei, GLsizei *, GLchar *);
	PFNGETPROGRAMINFOLOG GetProgramInfoLog = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETPROGRAMIV)(GLuint, GLenum, GLint *);
	PFNGETPROGRAMIV GetProgramiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETSHADERINFOLOG)(GLuint, GLsizei, GLsizei *, GLchar *);
	PFNGETSHADERINFOLOG GetShaderInfoLog = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETSHADERSOURCE)(GLuint, GLsizei, GLsizei *, GLchar *);
	PFNGETSHADERSOURCE GetShaderSource = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETSHADERIV)(GLuint, GLenum, GLint *);
	PFNGETSHADERIV GetShaderiv = 0;
	typedef GLint (CODEGEN_FUNCPTR *PFNGETUNIFORMLOCATION)(GLuint, const GLchar *);
	PFNGETUNIFORMLOCATION GetUniformLocation = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETUNIFORMFV)(GLuint, GLint, GLfloat *);
	PFNGETUNIFORMFV GetUniformfv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETUNIFORMIV)(GLuint, GLint, GLint *);
	PFNGETUNIFORMIV GetUniformiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETVERTEXATTRIBPOINTERV)(GLuint, GLenum, void **);
	PFNGETVERTEXATTRIBPOINTERV GetVertexAttribPointerv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETVERTEXATTRIBDV)(GLuint, GLenum, GLdouble *);
	PFNGETVERTEXATTRIBDV GetVertexAttribdv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETVERTEXATTRIBFV)(GLuint, GLenum, GLfloat *);
	PFNGETVERTEXATTRIBFV GetVertexAttribfv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETVERTEXATTRIBIV)(GLuint, GLenum, GLint *);
	PFNGETVERTEXATTRIBIV GetVertexAttribiv = 0;
	typedef GLboolean (CODEGEN_FUNCPTR *PFNISPROGRAM)(GLuint);
	PFNISPROGRAM IsProgram = 0;
	typedef GLboolean (CODEGEN_FUNCPTR *PFNISSHADER)(GLuint);
	PFNISSHADER IsShader = 0;
	typedef void (CODEGEN_FUNCPTR *PFNLINKPROGRAM)(GLuint);
	PFNLINKPROGRAM LinkProgram = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSHADERSOURCE)(GLuint, GLsizei, const GLchar *const*, const GLint *);
	PFNSHADERSOURCE ShaderSource = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSTENCILFUNCSEPARATE)(GLenum, GLenum, GLint, GLuint);
	PFNSTENCILFUNCSEPARATE StencilFuncSeparate = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSTENCILMASKSEPARATE)(GLenum, GLuint);
	PFNSTENCILMASKSEPARATE StencilMaskSeparate = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSTENCILOPSEPARATE)(GLenum, GLenum, GLenum, GLenum);
	PFNSTENCILOPSEPARATE StencilOpSeparate = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM1F)(GLint, GLfloat);
	PFNUNIFORM1F Uniform1f = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM1FV)(GLint, GLsizei, const GLfloat *);
	PFNUNIFORM1FV Uniform1fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM1I)(GLint, GLint);
	PFNUNIFORM1I Uniform1i = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM1IV)(GLint, GLsizei, const GLint *);
	PFNUNIFORM1IV Uniform1iv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM2F)(GLint, GLfloat, GLfloat);
	PFNUNIFORM2F Uniform2f = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM2FV)(GLint, GLsizei, const GLfloat *);
	PFNUNIFORM2FV Uniform2fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM2I)(GLint, GLint, GLint);
	PFNUNIFORM2I Uniform2i = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM2IV)(GLint, GLsizei, const GLint *);
	PFNUNIFORM2IV Uniform2iv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM3F)(GLint, GLfloat, GLfloat, GLfloat);
	PFNUNIFORM3F Uniform3f = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM3FV)(GLint, GLsizei, const GLfloat *);
	PFNUNIFORM3FV Uniform3fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM3I)(GLint, GLint, GLint, GLint);
	PFNUNIFORM3I Uniform3i = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM3IV)(GLint, GLsizei, const GLint *);
	PFNUNIFORM3IV Uniform3iv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM4F)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
	PFNUNIFORM4F Uniform4f = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM4FV)(GLint, GLsizei, const GLfloat *);
	PFNUNIFORM4FV Uniform4fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM4I)(GLint, GLint, GLint, GLint, GLint);
	PFNUNIFORM4I Uniform4i = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM4IV)(GLint, GLsizei, const GLint *);
	PFNUNIFORM4IV Uniform4iv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORMMATRIX2FV)(GLint, GLsizei, GLboolean, const GLfloat *);
	PFNUNIFORMMATRIX2FV UniformMatrix2fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORMMATRIX3FV)(GLint, GLsizei, GLboolean, const GLfloat *);
	PFNUNIFORMMATRIX3FV UniformMatrix3fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORMMATRIX4FV)(GLint, GLsizei, GLboolean, const GLfloat *);
	PFNUNIFORMMATRIX4FV UniformMatrix4fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUSEPROGRAM)(GLuint);
	PFNUSEPROGRAM UseProgram = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVALIDATEPROGRAM)(GLuint);
	PFNVALIDATEPROGRAM ValidateProgram = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB1D)(GLuint, GLdouble);
	PFNVERTEXATTRIB1D VertexAttrib1d = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB1DV)(GLuint, const GLdouble *);
	PFNVERTEXATTRIB1DV VertexAttrib1dv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB1F)(GLuint, GLfloat);
	PFNVERTEXATTRIB1F VertexAttrib1f = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB1FV)(GLuint, const GLfloat *);
	PFNVERTEXATTRIB1FV VertexAttrib1fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB1S)(GLuint, GLshort);
	PFNVERTEXATTRIB1S VertexAttrib1s = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB1SV)(GLuint, const GLshort *);
	PFNVERTEXATTRIB1SV VertexAttrib1sv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB2D)(GLuint, GLdouble, GLdouble);
	PFNVERTEXATTRIB2D VertexAttrib2d = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB2DV)(GLuint, const GLdouble *);
	PFNVERTEXATTRIB2DV VertexAttrib2dv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB2F)(GLuint, GLfloat, GLfloat);
	PFNVERTEXATTRIB2F VertexAttrib2f = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB2FV)(GLuint, const GLfloat *);
	PFNVERTEXATTRIB2FV VertexAttrib2fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB2S)(GLuint, GLshort, GLshort);
	PFNVERTEXATTRIB2S VertexAttrib2s = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB2SV)(GLuint, const GLshort *);
	PFNVERTEXATTRIB2SV VertexAttrib2sv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB3D)(GLuint, GLdouble, GLdouble, GLdouble);
	PFNVERTEXATTRIB3D VertexAttrib3d = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB3DV)(GLuint, const GLdouble *);
	PFNVERTEXATTRIB3DV VertexAttrib3dv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB3F)(GLuint, GLfloat, GLfloat, GLfloat);
	PFNVERTEXATTRIB3F VertexAttrib3f = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB3FV)(GLuint, const GLfloat *);
	PFNVERTEXATTRIB3FV VertexAttrib3fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB3S)(GLuint, GLshort, GLshort, GLshort);
	PFNVERTEXATTRIB3S VertexAttrib3s = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB3SV)(GLuint, const GLshort *);
	PFNVERTEXATTRIB3SV VertexAttrib3sv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4NBV)(GLuint, const GLbyte *);
	PFNVERTEXATTRIB4NBV VertexAttrib4Nbv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4NIV)(GLuint, const GLint *);
	PFNVERTEXATTRIB4NIV VertexAttrib4Niv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4NSV)(GLuint, const GLshort *);
	PFNVERTEXATTRIB4NSV VertexAttrib4Nsv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4NUB)(GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
	PFNVERTEXATTRIB4NUB VertexAttrib4Nub = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4NUBV)(GLuint, const GLubyte *);
	PFNVERTEXATTRIB4NUBV VertexAttrib4Nubv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4NUIV)(GLuint, const GLuint *);
	PFNVERTEXATTRIB4NUIV VertexAttrib4Nuiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4NUSV)(GLuint, const GLushort *);
	PFNVERTEXATTRIB4NUSV VertexAttrib4Nusv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4BV)(GLuint, const GLbyte *);
	PFNVERTEXATTRIB4BV VertexAttrib4bv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4D)(GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
	PFNVERTEXATTRIB4D VertexAttrib4d = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4DV)(GLuint, const GLdouble *);
	PFNVERTEXATTRIB4DV VertexAttrib4dv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4F)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
	PFNVERTEXATTRIB4F VertexAttrib4f = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4FV)(GLuint, const GLfloat *);
	PFNVERTEXATTRIB4FV VertexAttrib4fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4IV)(GLuint, const GLint *);
	PFNVERTEXATTRIB4IV VertexAttrib4iv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4S)(GLuint, GLshort, GLshort, GLshort, GLshort);
	PFNVERTEXATTRIB4S VertexAttrib4s = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4SV)(GLuint, const GLshort *);
	PFNVERTEXATTRIB4SV VertexAttrib4sv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4UBV)(GLuint, const GLubyte *);
	PFNVERTEXATTRIB4UBV VertexAttrib4ubv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4UIV)(GLuint, const GLuint *);
	PFNVERTEXATTRIB4UIV VertexAttrib4uiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIB4USV)(GLuint, const GLushort *);
	PFNVERTEXATTRIB4USV VertexAttrib4usv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBPOINTER)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void *);
	PFNVERTEXATTRIBPOINTER VertexAttribPointer = 0;
	
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORMMATRIX2X3FV)(GLint, GLsizei, GLboolean, const GLfloat *);
	PFNUNIFORMMATRIX2X3FV UniformMatrix2x3fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORMMATRIX2X4FV)(GLint, GLsizei, GLboolean, const GLfloat *);
	PFNUNIFORMMATRIX2X4FV UniformMatrix2x4fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORMMATRIX3X2FV)(GLint, GLsizei, GLboolean, const GLfloat *);
	PFNUNIFORMMATRIX3X2FV UniformMatrix3x2fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORMMATRIX3X4FV)(GLint, GLsizei, GLboolean, const GLfloat *);
	PFNUNIFORMMATRIX3X4FV UniformMatrix3x4fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORMMATRIX4X2FV)(GLint, GLsizei, GLboolean, const GLfloat *);
	PFNUNIFORMMATRIX4X2FV UniformMatrix4x2fv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORMMATRIX4X3FV)(GLint, GLsizei, GLboolean, const GLfloat *);
	PFNUNIFORMMATRIX4X3FV UniformMatrix4x3fv = 0;
	
	typedef void (CODEGEN_FUNCPTR *PFNBEGINCONDITIONALRENDER)(GLuint, GLenum);
	PFNBEGINCONDITIONALRENDER BeginConditionalRender = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBEGINTRANSFORMFEEDBACK)(GLenum);
	PFNBEGINTRANSFORMFEEDBACK BeginTransformFeedback = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBINDBUFFERBASE)(GLenum, GLuint, GLuint);
	PFNBINDBUFFERBASE BindBufferBase = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBINDBUFFERRANGE)(GLenum, GLuint, GLuint, GLintptr, GLsizeiptr);
	PFNBINDBUFFERRANGE BindBufferRange = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBINDFRAGDATALOCATION)(GLuint, GLuint, const GLchar *);
	PFNBINDFRAGDATALOCATION BindFragDataLocation = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBINDFRAMEBUFFER)(GLenum, GLuint);
	PFNBINDFRAMEBUFFER BindFramebuffer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBINDRENDERBUFFER)(GLenum, GLuint);
	PFNBINDRENDERBUFFER BindRenderbuffer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBINDVERTEXARRAY)(GLuint);
	PFNBINDVERTEXARRAY BindVertexArray = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBLITFRAMEBUFFER)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
	PFNBLITFRAMEBUFFER BlitFramebuffer = 0;
	typedef GLenum (CODEGEN_FUNCPTR *PFNCHECKFRAMEBUFFERSTATUS)(GLenum);
	PFNCHECKFRAMEBUFFERSTATUS CheckFramebufferStatus = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCLAMPCOLOR)(GLenum, GLenum);
	PFNCLAMPCOLOR ClampColor = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCLEARBUFFERFI)(GLenum, GLint, GLfloat, GLint);
	PFNCLEARBUFFERFI ClearBufferfi = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCLEARBUFFERFV)(GLenum, GLint, const GLfloat *);
	PFNCLEARBUFFERFV ClearBufferfv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCLEARBUFFERIV)(GLenum, GLint, const GLint *);
	PFNCLEARBUFFERIV ClearBufferiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCLEARBUFFERUIV)(GLenum, GLint, const GLuint *);
	PFNCLEARBUFFERUIV ClearBufferuiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOLORMASKI)(GLuint, GLboolean, GLboolean, GLboolean, GLboolean);
	PFNCOLORMASKI ColorMaski = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDELETEFRAMEBUFFERS)(GLsizei, const GLuint *);
	PFNDELETEFRAMEBUFFERS DeleteFramebuffers = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDELETERENDERBUFFERS)(GLsizei, const GLuint *);
	PFNDELETERENDERBUFFERS DeleteRenderbuffers = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDELETEVERTEXARRAYS)(GLsizei, const GLuint *);
	PFNDELETEVERTEXARRAYS DeleteVertexArrays = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDISABLEI)(GLenum, GLuint);
	PFNDISABLEI Disablei = 0;
	typedef void (CODEGEN_FUNCPTR *PFNENABLEI)(GLenum, GLuint);
	PFNENABLEI Enablei = 0;
	typedef void (CODEGEN_FUNCPTR *PFNENDCONDITIONALRENDER)(void);
	PFNENDCONDITIONALRENDER EndConditionalRender = 0;
	typedef void (CODEGEN_FUNCPTR *PFNENDTRANSFORMFEEDBACK)(void);
	PFNENDTRANSFORMFEEDBACK EndTransformFeedback = 0;
	typedef void (CODEGEN_FUNCPTR *PFNFLUSHMAPPEDBUFFERRANGE)(GLenum, GLintptr, GLsizeiptr);
	PFNFLUSHMAPPEDBUFFERRANGE FlushMappedBufferRange = 0;
	typedef void (CODEGEN_FUNCPTR *PFNFRAMEBUFFERRENDERBUFFER)(GLenum, GLenum, GLenum, GLuint);
	PFNFRAMEBUFFERRENDERBUFFER FramebufferRenderbuffer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNFRAMEBUFFERTEXTURE1D)(GLenum, GLenum, GLenum, GLuint, GLint);
	PFNFRAMEBUFFERTEXTURE1D FramebufferTexture1D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNFRAMEBUFFERTEXTURE2D)(GLenum, GLenum, GLenum, GLuint, GLint);
	PFNFRAMEBUFFERTEXTURE2D FramebufferTexture2D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNFRAMEBUFFERTEXTURE3D)(GLenum, GLenum, GLenum, GLuint, GLint, GLint);
	PFNFRAMEBUFFERTEXTURE3D FramebufferTexture3D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNFRAMEBUFFERTEXTURELAYER)(GLenum, GLenum, GLuint, GLint, GLint);
	PFNFRAMEBUFFERTEXTURELAYER FramebufferTextureLayer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGENFRAMEBUFFERS)(GLsizei, GLuint *);
	PFNGENFRAMEBUFFERS GenFramebuffers = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGENRENDERBUFFERS)(GLsizei, GLuint *);
	PFNGENRENDERBUFFERS GenRenderbuffers = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGENVERTEXARRAYS)(GLsizei, GLuint *);
	PFNGENVERTEXARRAYS GenVertexArrays = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGENERATEMIPMAP)(GLenum);
	PFNGENERATEMIPMAP GenerateMipmap = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETBOOLEANI_V)(GLenum, GLuint, GLboolean *);
	PFNGETBOOLEANI_V GetBooleani_v = 0;
	typedef GLint (CODEGEN_FUNCPTR *PFNGETFRAGDATALOCATION)(GLuint, const GLchar *);
	PFNGETFRAGDATALOCATION GetFragDataLocation = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETFRAMEBUFFERATTACHMENTPARAMETERIV)(GLenum, GLenum, GLenum, GLint *);
	PFNGETFRAMEBUFFERATTACHMENTPARAMETERIV GetFramebufferAttachmentParameteriv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETINTEGERI_V)(GLenum, GLuint, GLint *);
	PFNGETINTEGERI_V GetIntegeri_v = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETRENDERBUFFERPARAMETERIV)(GLenum, GLenum, GLint *);
	PFNGETRENDERBUFFERPARAMETERIV GetRenderbufferParameteriv = 0;
	typedef const GLubyte * (CODEGEN_FUNCPTR *PFNGETSTRINGI)(GLenum, GLuint);
	PFNGETSTRINGI GetStringi = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTEXPARAMETERIIV)(GLenum, GLenum, GLint *);
	PFNGETTEXPARAMETERIIV GetTexParameterIiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTEXPARAMETERIUIV)(GLenum, GLenum, GLuint *);
	PFNGETTEXPARAMETERIUIV GetTexParameterIuiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTRANSFORMFEEDBACKVARYING)(GLuint, GLuint, GLsizei, GLsizei *, GLsizei *, GLenum *, GLchar *);
	PFNGETTRANSFORMFEEDBACKVARYING GetTransformFeedbackVarying = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETUNIFORMUIV)(GLuint, GLint, GLuint *);
	PFNGETUNIFORMUIV GetUniformuiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETVERTEXATTRIBIIV)(GLuint, GLenum, GLint *);
	PFNGETVERTEXATTRIBIIV GetVertexAttribIiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETVERTEXATTRIBIUIV)(GLuint, GLenum, GLuint *);
	PFNGETVERTEXATTRIBIUIV GetVertexAttribIuiv = 0;
	typedef GLboolean (CODEGEN_FUNCPTR *PFNISENABLEDI)(GLenum, GLuint);
	PFNISENABLEDI IsEnabledi = 0;
	typedef GLboolean (CODEGEN_FUNCPTR *PFNISFRAMEBUFFER)(GLuint);
	PFNISFRAMEBUFFER IsFramebuffer = 0;
	typedef GLboolean (CODEGEN_FUNCPTR *PFNISRENDERBUFFER)(GLuint);
	PFNISRENDERBUFFER IsRenderbuffer = 0;
	typedef GLboolean (CODEGEN_FUNCPTR *PFNISVERTEXARRAY)(GLuint);
	PFNISVERTEXARRAY IsVertexArray = 0;
	typedef void * (CODEGEN_FUNCPTR *PFNMAPBUFFERRANGE)(GLenum, GLintptr, GLsizeiptr, GLbitfield);
	PFNMAPBUFFERRANGE MapBufferRange = 0;
	typedef void (CODEGEN_FUNCPTR *PFNRENDERBUFFERSTORAGE)(GLenum, GLenum, GLsizei, GLsizei);
	PFNRENDERBUFFERSTORAGE RenderbufferStorage = 0;
	typedef void (CODEGEN_FUNCPTR *PFNRENDERBUFFERSTORAGEMULTISAMPLE)(GLenum, GLsizei, GLenum, GLsizei, GLsizei);
	PFNRENDERBUFFERSTORAGEMULTISAMPLE RenderbufferStorageMultisample = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXPARAMETERIIV)(GLenum, GLenum, const GLint *);
	PFNTEXPARAMETERIIV TexParameterIiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXPARAMETERIUIV)(GLenum, GLenum, const GLuint *);
	PFNTEXPARAMETERIUIV TexParameterIuiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTRANSFORMFEEDBACKVARYINGS)(GLuint, GLsizei, const GLchar *const*, GLenum);
	PFNTRANSFORMFEEDBACKVARYINGS TransformFeedbackVaryings = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM1UI)(GLint, GLuint);
	PFNUNIFORM1UI Uniform1ui = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM1UIV)(GLint, GLsizei, const GLuint *);
	PFNUNIFORM1UIV Uniform1uiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM2UI)(GLint, GLuint, GLuint);
	PFNUNIFORM2UI Uniform2ui = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM2UIV)(GLint, GLsizei, const GLuint *);
	PFNUNIFORM2UIV Uniform2uiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM3UI)(GLint, GLuint, GLuint, GLuint);
	PFNUNIFORM3UI Uniform3ui = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM3UIV)(GLint, GLsizei, const GLuint *);
	PFNUNIFORM3UIV Uniform3uiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM4UI)(GLint, GLuint, GLuint, GLuint, GLuint);
	PFNUNIFORM4UI Uniform4ui = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORM4UIV)(GLint, GLsizei, const GLuint *);
	PFNUNIFORM4UIV Uniform4uiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI1I)(GLuint, GLint);
	PFNVERTEXATTRIBI1I VertexAttribI1i = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI1IV)(GLuint, const GLint *);
	PFNVERTEXATTRIBI1IV VertexAttribI1iv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI1UI)(GLuint, GLuint);
	PFNVERTEXATTRIBI1UI VertexAttribI1ui = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI1UIV)(GLuint, const GLuint *);
	PFNVERTEXATTRIBI1UIV VertexAttribI1uiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI2I)(GLuint, GLint, GLint);
	PFNVERTEXATTRIBI2I VertexAttribI2i = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI2IV)(GLuint, const GLint *);
	PFNVERTEXATTRIBI2IV VertexAttribI2iv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI2UI)(GLuint, GLuint, GLuint);
	PFNVERTEXATTRIBI2UI VertexAttribI2ui = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI2UIV)(GLuint, const GLuint *);
	PFNVERTEXATTRIBI2UIV VertexAttribI2uiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI3I)(GLuint, GLint, GLint, GLint);
	PFNVERTEXATTRIBI3I VertexAttribI3i = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI3IV)(GLuint, const GLint *);
	PFNVERTEXATTRIBI3IV VertexAttribI3iv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI3UI)(GLuint, GLuint, GLuint, GLuint);
	PFNVERTEXATTRIBI3UI VertexAttribI3ui = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI3UIV)(GLuint, const GLuint *);
	PFNVERTEXATTRIBI3UIV VertexAttribI3uiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI4BV)(GLuint, const GLbyte *);
	PFNVERTEXATTRIBI4BV VertexAttribI4bv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI4I)(GLuint, GLint, GLint, GLint, GLint);
	PFNVERTEXATTRIBI4I VertexAttribI4i = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI4IV)(GLuint, const GLint *);
	PFNVERTEXATTRIBI4IV VertexAttribI4iv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI4SV)(GLuint, const GLshort *);
	PFNVERTEXATTRIBI4SV VertexAttribI4sv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI4UBV)(GLuint, const GLubyte *);
	PFNVERTEXATTRIBI4UBV VertexAttribI4ubv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI4UI)(GLuint, GLuint, GLuint, GLuint, GLuint);
	PFNVERTEXATTRIBI4UI VertexAttribI4ui = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI4UIV)(GLuint, const GLuint *);
	PFNVERTEXATTRIBI4UIV VertexAttribI4uiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBI4USV)(GLuint, const GLushort *);
	PFNVERTEXATTRIBI4USV VertexAttribI4usv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBIPOINTER)(GLuint, GLint, GLenum, GLsizei, const void *);
	PFNVERTEXATTRIBIPOINTER VertexAttribIPointer = 0;
	
	typedef void (CODEGEN_FUNCPTR *PFNCOPYBUFFERSUBDATA)(GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr);
	PFNCOPYBUFFERSUBDATA CopyBufferSubData = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDRAWARRAYSINSTANCED)(GLenum, GLint, GLsizei, GLsizei);
	PFNDRAWARRAYSINSTANCED DrawArraysInstanced = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDRAWELEMENTSINSTANCED)(GLenum, GLsizei, GLenum, const void *, GLsizei);
	PFNDRAWELEMENTSINSTANCED DrawElementsInstanced = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETACTIVEUNIFORMBLOCKNAME)(GLuint, GLuint, GLsizei, GLsizei *, GLchar *);
	PFNGETACTIVEUNIFORMBLOCKNAME GetActiveUniformBlockName = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETACTIVEUNIFORMBLOCKIV)(GLuint, GLuint, GLenum, GLint *);
	PFNGETACTIVEUNIFORMBLOCKIV GetActiveUniformBlockiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETACTIVEUNIFORMNAME)(GLuint, GLuint, GLsizei, GLsizei *, GLchar *);
	PFNGETACTIVEUNIFORMNAME GetActiveUniformName = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETACTIVEUNIFORMSIV)(GLuint, GLsizei, const GLuint *, GLenum, GLint *);
	PFNGETACTIVEUNIFORMSIV GetActiveUniformsiv = 0;
	typedef GLuint (CODEGEN_FUNCPTR *PFNGETUNIFORMBLOCKINDEX)(GLuint, const GLchar *);
	PFNGETUNIFORMBLOCKINDEX GetUniformBlockIndex = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETUNIFORMINDICES)(GLuint, GLsizei, const GLchar *const*, GLuint *);
	PFNGETUNIFORMINDICES GetUniformIndices = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPRIMITIVERESTARTINDEX)(GLuint);
	PFNPRIMITIVERESTARTINDEX PrimitiveRestartIndex = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXBUFFER)(GLenum, GLenum, GLuint);
	PFNTEXBUFFER TexBuffer = 0;
	typedef void (CODEGEN_FUNCPTR *PFNUNIFORMBLOCKBINDING)(GLuint, GLuint, GLuint);
	PFNUNIFORMBLOCKBINDING UniformBlockBinding = 0;
	
	typedef GLenum (CODEGEN_FUNCPTR *PFNCLIENTWAITSYNC)(GLsync, GLbitfield, GLuint64);
	PFNCLIENTWAITSYNC ClientWaitSync = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDELETESYNC)(GLsync);
	PFNDELETESYNC DeleteSync = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDRAWELEMENTSBASEVERTEX)(GLenum, GLsizei, GLenum, const void *, GLint);
	PFNDRAWELEMENTSBASEVERTEX DrawElementsBaseVertex = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDRAWELEMENTSINSTANCEDBASEVERTEX)(GLenum, GLsizei, GLenum, const void *, GLsizei, GLint);
	PFNDRAWELEMENTSINSTANCEDBASEVERTEX DrawElementsInstancedBaseVertex = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDRAWRANGEELEMENTSBASEVERTEX)(GLenum, GLuint, GLuint, GLsizei, GLenum, const void *, GLint);
	PFNDRAWRANGEELEMENTSBASEVERTEX DrawRangeElementsBaseVertex = 0;
	typedef GLsync (CODEGEN_FUNCPTR *PFNFENCESYNC)(GLenum, GLbitfield);
	PFNFENCESYNC FenceSync = 0;
	typedef void (CODEGEN_FUNCPTR *PFNFRAMEBUFFERTEXTURE)(GLenum, GLenum, GLuint, GLint);
	PFNFRAMEBUFFERTEXTURE FramebufferTexture = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETBUFFERPARAMETERI64V)(GLenum, GLenum, GLint64 *);
	PFNGETBUFFERPARAMETERI64V GetBufferParameteri64v = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETINTEGER64I_V)(GLenum, GLuint, GLint64 *);
	PFNGETINTEGER64I_V GetInteger64i_v = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETINTEGER64V)(GLenum, GLint64 *);
	PFNGETINTEGER64V GetInteger64v = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETMULTISAMPLEFV)(GLenum, GLuint, GLfloat *);
	PFNGETMULTISAMPLEFV GetMultisamplefv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETSYNCIV)(GLsync, GLenum, GLsizei, GLsizei *, GLint *);
	PFNGETSYNCIV GetSynciv = 0;
	typedef GLboolean (CODEGEN_FUNCPTR *PFNISSYNC)(GLsync);
	PFNISSYNC IsSync = 0;
	typedef void (CODEGEN_FUNCPTR *PFNMULTIDRAWELEMENTSBASEVERTEX)(GLenum, const GLsizei *, GLenum, const void *const*, GLsizei, const GLint *);
	PFNMULTIDRAWELEMENTSBASEVERTEX MultiDrawElementsBaseVertex = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROVOKINGVERTEX)(GLenum);
	PFNPROVOKINGVERTEX ProvokingVertex = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSAMPLEMASKI)(GLuint, GLbitfield);
	PFNSAMPLEMASKI SampleMaski = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXIMAGE2DMULTISAMPLE)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean);
	PFNTEXIMAGE2DMULTISAMPLE TexImage2DMultisample = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXIMAGE3DMULTISAMPLE)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei, GLboolean);
	PFNTEXIMAGE3DMULTISAMPLE TexImage3DMultisample = 0;
	typedef void (CODEGEN_FUNCPTR *PFNWAITSYNC)(GLsync, GLbitfield, GLuint64);
	PFNWAITSYNC WaitSync = 0;
	
	typedef void (CODEGEN_FUNCPTR *PFNBINDFRAGDATALOCATIONINDEXED)(GLuint, GLuint, GLuint, const GLchar *);
	PFNBINDFRAGDATALOCATIONINDEXED BindFragDataLocationIndexed = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBINDSAMPLER)(GLuint, GLuint);
	PFNBINDSAMPLER BindSampler = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDELETESAMPLERS)(GLsizei, const GLuint *);
	PFNDELETESAMPLERS DeleteSamplers = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGENSAMPLERS)(GLsizei, GLuint *);
	PFNGENSAMPLERS GenSamplers = 0;
	typedef GLint (CODEGEN_FUNCPTR *PFNGETFRAGDATAINDEX)(GLuint, const GLchar *);
	PFNGETFRAGDATAINDEX GetFragDataIndex = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETQUERYOBJECTI64V)(GLuint, GLenum, GLint64 *);
	PFNGETQUERYOBJECTI64V GetQueryObjecti64v = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETQUERYOBJECTUI64V)(GLuint, GLenum, GLuint64 *);
	PFNGETQUERYOBJECTUI64V GetQueryObjectui64v = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETSAMPLERPARAMETERIIV)(GLuint, GLenum, GLint *);
	PFNGETSAMPLERPARAMETERIIV GetSamplerParameterIiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETSAMPLERPARAMETERIUIV)(GLuint, GLenum, GLuint *);
	PFNGETSAMPLERPARAMETERIUIV GetSamplerParameterIuiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETSAMPLERPARAMETERFV)(GLuint, GLenum, GLfloat *);
	PFNGETSAMPLERPARAMETERFV GetSamplerParameterfv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETSAMPLERPARAMETERIV)(GLuint, GLenum, GLint *);
	PFNGETSAMPLERPARAMETERIV GetSamplerParameteriv = 0;
	typedef GLboolean (CODEGEN_FUNCPTR *PFNISSAMPLER)(GLuint);
	PFNISSAMPLER IsSampler = 0;
	typedef void (CODEGEN_FUNCPTR *PFNQUERYCOUNTER)(GLuint, GLenum);
	PFNQUERYCOUNTER QueryCounter = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSAMPLERPARAMETERIIV)(GLuint, GLenum, const GLint *);
	PFNSAMPLERPARAMETERIIV SamplerParameterIiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSAMPLERPARAMETERIUIV)(GLuint, GLenum, const GLuint *);
	PFNSAMPLERPARAMETERIUIV SamplerParameterIuiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSAMPLERPARAMETERF)(GLuint, GLenum, GLfloat);
	PFNSAMPLERPARAMETERF SamplerParameterf = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSAMPLERPARAMETERFV)(GLuint, GLenum, const GLfloat *);
	PFNSAMPLERPARAMETERFV SamplerParameterfv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSAMPLERPARAMETERI)(GLuint, GLenum, GLint);
	PFNSAMPLERPARAMETERI SamplerParameteri = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSAMPLERPARAMETERIV)(GLuint, GLenum, const GLint *);
	PFNSAMPLERPARAMETERIV SamplerParameteriv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBDIVISOR)(GLuint, GLuint);
	PFNVERTEXATTRIBDIVISOR VertexAttribDivisor = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBP1UI)(GLuint, GLenum, GLboolean, GLuint);
	PFNVERTEXATTRIBP1UI VertexAttribP1ui = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBP1UIV)(GLuint, GLenum, GLboolean, const GLuint *);
	PFNVERTEXATTRIBP1UIV VertexAttribP1uiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBP2UI)(GLuint, GLenum, GLboolean, GLuint);
	PFNVERTEXATTRIBP2UI VertexAttribP2ui = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBP2UIV)(GLuint, GLenum, GLboolean, const GLuint *);
	PFNVERTEXATTRIBP2UIV VertexAttribP2uiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBP3UI)(GLuint, GLenum, GLboolean, GLuint);
	PFNVERTEXATTRIBP3UI VertexAttribP3ui = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBP3UIV)(GLuint, GLenum, GLboolean, const GLuint *);
	PFNVERTEXATTRIBP3UIV VertexAttribP3uiv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBP4UI)(GLuint, GLenum, GLboolean, GLuint);
	PFNVERTEXATTRIBP4UI VertexAttribP4ui = 0;
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBP4UIV)(GLuint, GLenum, GLboolean, const GLuint *);
	PFNVERTEXATTRIBP4UIV VertexAttribP4uiv = 0;
	
	static int LoadCoreFunctions()
	{
		int numFailed = 0;
		BlendFunc = reinterpret_cast<PFNBLENDFUNC>(IntGetProcAddress("glBlendFunc"));
		if(!BlendFunc) ++numFailed;
		Clear = reinterpret_cast<PFNCLEAR>(IntGetProcAddress("glClear"));
		if(!Clear) ++numFailed;
		ClearColor = reinterpret_cast<PFNCLEARCOLOR>(IntGetProcAddress("glClearColor"));
		if(!ClearColor) ++numFailed;
		ClearDepth = reinterpret_cast<PFNCLEARDEPTH>(IntGetProcAddress("glClearDepth"));
		if(!ClearDepth) ++numFailed;
		ClearStencil = reinterpret_cast<PFNCLEARSTENCIL>(IntGetProcAddress("glClearStencil"));
		if(!ClearStencil) ++numFailed;
		ColorMask = reinterpret_cast<PFNCOLORMASK>(IntGetProcAddress("glColorMask"));
		if(!ColorMask) ++numFailed;
		CullFace = reinterpret_cast<PFNCULLFACE>(IntGetProcAddress("glCullFace"));
		if(!CullFace) ++numFailed;
		DepthFunc = reinterpret_cast<PFNDEPTHFUNC>(IntGetProcAddress("glDepthFunc"));
		if(!DepthFunc) ++numFailed;
		DepthMask = reinterpret_cast<PFNDEPTHMASK>(IntGetProcAddress("glDepthMask"));
		if(!DepthMask) ++numFailed;
		DepthRange = reinterpret_cast<PFNDEPTHRANGE>(IntGetProcAddress("glDepthRange"));
		if(!DepthRange) ++numFailed;
		Disable = reinterpret_cast<PFNDISABLE>(IntGetProcAddress("glDisable"));
		if(!Disable) ++numFailed;
		DrawBuffer = reinterpret_cast<PFNDRAWBUFFER>(IntGetProcAddress("glDrawBuffer"));
		if(!DrawBuffer) ++numFailed;
		Enable = reinterpret_cast<PFNENABLE>(IntGetProcAddress("glEnable"));
		if(!Enable) ++numFailed;
		Finish = reinterpret_cast<PFNFINISH>(IntGetProcAddress("glFinish"));
		if(!Finish) ++numFailed;
		Flush = reinterpret_cast<PFNFLUSH>(IntGetProcAddress("glFlush"));
		if(!Flush) ++numFailed;
		FrontFace = reinterpret_cast<PFNFRONTFACE>(IntGetProcAddress("glFrontFace"));
		if(!FrontFace) ++numFailed;
		GetBooleanv = reinterpret_cast<PFNGETBOOLEANV>(IntGetProcAddress("glGetBooleanv"));
		if(!GetBooleanv) ++numFailed;
		GetDoublev = reinterpret_cast<PFNGETDOUBLEV>(IntGetProcAddress("glGetDoublev"));
		if(!GetDoublev) ++numFailed;
		GetError = reinterpret_cast<PFNGETERROR>(IntGetProcAddress("glGetError"));
		if(!GetError) ++numFailed;
		GetFloatv = reinterpret_cast<PFNGETFLOATV>(IntGetProcAddress("glGetFloatv"));
		if(!GetFloatv) ++numFailed;
		GetIntegerv = reinterpret_cast<PFNGETINTEGERV>(IntGetProcAddress("glGetIntegerv"));
		if(!GetIntegerv) ++numFailed;
		GetString = reinterpret_cast<PFNGETSTRING>(IntGetProcAddress("glGetString"));
		if(!GetString) ++numFailed;
		GetTexImage = reinterpret_cast<PFNGETTEXIMAGE>(IntGetProcAddress("glGetTexImage"));
		if(!GetTexImage) ++numFailed;
		GetTexLevelParameterfv = reinterpret_cast<PFNGETTEXLEVELPARAMETERFV>(IntGetProcAddress("glGetTexLevelParameterfv"));
		if(!GetTexLevelParameterfv) ++numFailed;
		GetTexLevelParameteriv = reinterpret_cast<PFNGETTEXLEVELPARAMETERIV>(IntGetProcAddress("glGetTexLevelParameteriv"));
		if(!GetTexLevelParameteriv) ++numFailed;
		GetTexParameterfv = reinterpret_cast<PFNGETTEXPARAMETERFV>(IntGetProcAddress("glGetTexParameterfv"));
		if(!GetTexParameterfv) ++numFailed;
		GetTexParameteriv = reinterpret_cast<PFNGETTEXPARAMETERIV>(IntGetProcAddress("glGetTexParameteriv"));
		if(!GetTexParameteriv) ++numFailed;
		Hint = reinterpret_cast<PFNHINT>(IntGetProcAddress("glHint"));
		if(!Hint) ++numFailed;
		IsEnabled = reinterpret_cast<PFNISENABLED>(IntGetProcAddress("glIsEnabled"));
		if(!IsEnabled) ++numFailed;
		LineWidth = reinterpret_cast<PFNLINEWIDTH>(IntGetProcAddress("glLineWidth"));
		if(!LineWidth) ++numFailed;
		LogicOp = reinterpret_cast<PFNLOGICOP>(IntGetProcAddress("glLogicOp"));
		if(!LogicOp) ++numFailed;
		PixelStoref = reinterpret_cast<PFNPIXELSTOREF>(IntGetProcAddress("glPixelStoref"));
		if(!PixelStoref) ++numFailed;
		PixelStorei = reinterpret_cast<PFNPIXELSTOREI>(IntGetProcAddress("glPixelStorei"));
		if(!PixelStorei) ++numFailed;
		PointSize = reinterpret_cast<PFNPOINTSIZE>(IntGetProcAddress("glPointSize"));
		if(!PointSize) ++numFailed;
		PolygonMode = reinterpret_cast<PFNPOLYGONMODE>(IntGetProcAddress("glPolygonMode"));
		if(!PolygonMode) ++numFailed;
		ReadBuffer = reinterpret_cast<PFNREADBUFFER>(IntGetProcAddress("glReadBuffer"));
		if(!ReadBuffer) ++numFailed;
		ReadPixels = reinterpret_cast<PFNREADPIXELS>(IntGetProcAddress("glReadPixels"));
		if(!ReadPixels) ++numFailed;
		Scissor = reinterpret_cast<PFNSCISSOR>(IntGetProcAddress("glScissor"));
		if(!Scissor) ++numFailed;
		StencilFunc = reinterpret_cast<PFNSTENCILFUNC>(IntGetProcAddress("glStencilFunc"));
		if(!StencilFunc) ++numFailed;
		StencilMask = reinterpret_cast<PFNSTENCILMASK>(IntGetProcAddress("glStencilMask"));
		if(!StencilMask) ++numFailed;
		StencilOp = reinterpret_cast<PFNSTENCILOP>(IntGetProcAddress("glStencilOp"));
		if(!StencilOp) ++numFailed;
		TexImage1D = reinterpret_cast<PFNTEXIMAGE1D>(IntGetProcAddress("glTexImage1D"));
		if(!TexImage1D) ++numFailed;
		TexImage2D = reinterpret_cast<PFNTEXIMAGE2D>(IntGetProcAddress("glTexImage2D"));
		if(!TexImage2D) ++numFailed;
		TexParameterf = reinterpret_cast<PFNTEXPARAMETERF>(IntGetProcAddress("glTexParameterf"));
		if(!TexParameterf) ++numFailed;
		TexParameterfv = reinterpret_cast<PFNTEXPARAMETERFV>(IntGetProcAddress("glTexParameterfv"));
		if(!TexParameterfv) ++numFailed;
		TexParameteri = reinterpret_cast<PFNTEXPARAMETERI>(IntGetProcAddress("glTexParameteri"));
		if(!TexParameteri) ++numFailed;
		TexParameteriv = reinterpret_cast<PFNTEXPARAMETERIV>(IntGetProcAddress("glTexParameteriv"));
		if(!TexParameteriv) ++numFailed;
		Viewport = reinterpret_cast<PFNVIEWPORT>(IntGetProcAddress("glViewport"));
		if(!Viewport) ++numFailed;
		BindTexture = reinterpret_cast<PFNBINDTEXTURE>(IntGetProcAddress("glBindTexture"));
		if(!BindTexture) ++numFailed;
		CopyTexImage1D = reinterpret_cast<PFNCOPYTEXIMAGE1D>(IntGetProcAddress("glCopyTexImage1D"));
		if(!CopyTexImage1D) ++numFailed;
		CopyTexImage2D = reinterpret_cast<PFNCOPYTEXIMAGE2D>(IntGetProcAddress("glCopyTexImage2D"));
		if(!CopyTexImage2D) ++numFailed;
		CopyTexSubImage1D = reinterpret_cast<PFNCOPYTEXSUBIMAGE1D>(IntGetProcAddress("glCopyTexSubImage1D"));
		if(!CopyTexSubImage1D) ++numFailed;
		CopyTexSubImage2D = reinterpret_cast<PFNCOPYTEXSUBIMAGE2D>(IntGetProcAddress("glCopyTexSubImage2D"));
		if(!CopyTexSubImage2D) ++numFailed;
		DeleteTextures = reinterpret_cast<PFNDELETETEXTURES>(IntGetProcAddress("glDeleteTextures"));
		if(!DeleteTextures) ++numFailed;
		DrawArrays = reinterpret_cast<PFNDRAWARRAYS>(IntGetProcAddress("glDrawArrays"));
		if(!DrawArrays) ++numFailed;
		DrawElements = reinterpret_cast<PFNDRAWELEMENTS>(IntGetProcAddress("glDrawElements"));
		if(!DrawElements) ++numFailed;
		GenTextures = reinterpret_cast<PFNGENTEXTURES>(IntGetProcAddress("glGenTextures"));
		if(!GenTextures) ++numFailed;
		IsTexture = reinterpret_cast<PFNISTEXTURE>(IntGetProcAddress("glIsTexture"));
		if(!IsTexture) ++numFailed;
		PolygonOffset = reinterpret_cast<PFNPOLYGONOFFSET>(IntGetProcAddress("glPolygonOffset"));
		if(!PolygonOffset) ++numFailed;
		TexSubImage1D = reinterpret_cast<PFNTEXSUBIMAGE1D>(IntGetProcAddress("glTexSubImage1D"));
		if(!TexSubImage1D) ++numFailed;
		TexSubImage2D = reinterpret_cast<PFNTEXSUBIMAGE2D>(IntGetProcAddress("glTexSubImage2D"));
		if(!TexSubImage2D) ++numFailed;
		CopyTexSubImage3D = reinterpret_cast<PFNCOPYTEXSUBIMAGE3D>(IntGetProcAddress("glCopyTexSubImage3D"));
		if(!CopyTexSubImage3D) ++numFailed;
		DrawRangeElements = reinterpret_cast<PFNDRAWRANGEELEMENTS>(IntGetProcAddress("glDrawRangeElements"));
		if(!DrawRangeElements) ++numFailed;
		TexImage3D = reinterpret_cast<PFNTEXIMAGE3D>(IntGetProcAddress("glTexImage3D"));
		if(!TexImage3D) ++numFailed;
		TexSubImage3D = reinterpret_cast<PFNTEXSUBIMAGE3D>(IntGetProcAddress("glTexSubImage3D"));
		if(!TexSubImage3D) ++numFailed;
		ActiveTexture = reinterpret_cast<PFNACTIVETEXTURE>(IntGetProcAddress("glActiveTexture"));
		if(!ActiveTexture) ++numFailed;
		CompressedTexImage1D = reinterpret_cast<PFNCOMPRESSEDTEXIMAGE1D>(IntGetProcAddress("glCompressedTexImage1D"));
		if(!CompressedTexImage1D) ++numFailed;
		CompressedTexImage2D = reinterpret_cast<PFNCOMPRESSEDTEXIMAGE2D>(IntGetProcAddress("glCompressedTexImage2D"));
		if(!CompressedTexImage2D) ++numFailed;
		CompressedTexImage3D = reinterpret_cast<PFNCOMPRESSEDTEXIMAGE3D>(IntGetProcAddress("glCompressedTexImage3D"));
		if(!CompressedTexImage3D) ++numFailed;
		CompressedTexSubImage1D = reinterpret_cast<PFNCOMPRESSEDTEXSUBIMAGE1D>(IntGetProcAddress("glCompressedTexSubImage1D"));
		if(!CompressedTexSubImage1D) ++numFailed;
		CompressedTexSubImage2D = reinterpret_cast<PFNCOMPRESSEDTEXSUBIMAGE2D>(IntGetProcAddress("glCompressedTexSubImage2D"));
		if(!CompressedTexSubImage2D) ++numFailed;
		CompressedTexSubImage3D = reinterpret_cast<PFNCOMPRESSEDTEXSUBIMAGE3D>(IntGetProcAddress("glCompressedTexSubImage3D"));
		if(!CompressedTexSubImage3D) ++numFailed;
		GetCompressedTexImage = reinterpret_cast<PFNGETCOMPRESSEDTEXIMAGE>(IntGetProcAddress("glGetCompressedTexImage"));
		if(!GetCompressedTexImage) ++numFailed;
		SampleCoverage = reinterpret_cast<PFNSAMPLECOVERAGE>(IntGetProcAddress("glSampleCoverage"));
		if(!SampleCoverage) ++numFailed;
		BlendColor = reinterpret_cast<PFNBLENDCOLOR>(IntGetProcAddress("glBlendColor"));
		if(!BlendColor) ++numFailed;
		BlendEquation = reinterpret_cast<PFNBLENDEQUATION>(IntGetProcAddress("glBlendEquation"));
		if(!BlendEquation) ++numFailed;
		BlendFuncSeparate = reinterpret_cast<PFNBLENDFUNCSEPARATE>(IntGetProcAddress("glBlendFuncSeparate"));
		if(!BlendFuncSeparate) ++numFailed;
		MultiDrawArrays = reinterpret_cast<PFNMULTIDRAWARRAYS>(IntGetProcAddress("glMultiDrawArrays"));
		if(!MultiDrawArrays) ++numFailed;
		MultiDrawElements = reinterpret_cast<PFNMULTIDRAWELEMENTS>(IntGetProcAddress("glMultiDrawElements"));
		if(!MultiDrawElements) ++numFailed;
		PointParameterf = reinterpret_cast<PFNPOINTPARAMETERF>(IntGetProcAddress("glPointParameterf"));
		if(!PointParameterf) ++numFailed;
		PointParameterfv = reinterpret_cast<PFNPOINTPARAMETERFV>(IntGetProcAddress("glPointParameterfv"));
		if(!PointParameterfv) ++numFailed;
		PointParameteri = reinterpret_cast<PFNPOINTPARAMETERI>(IntGetProcAddress("glPointParameteri"));
		if(!PointParameteri) ++numFailed;
		PointParameteriv = reinterpret_cast<PFNPOINTPARAMETERIV>(IntGetProcAddress("glPointParameteriv"));
		if(!PointParameteriv) ++numFailed;
		BeginQuery = reinterpret_cast<PFNBEGINQUERY>(IntGetProcAddress("glBeginQuery"));
		if(!BeginQuery) ++numFailed;
		BindBuffer = reinterpret_cast<PFNBINDBUFFER>(IntGetProcAddress("glBindBuffer"));
		if(!BindBuffer) ++numFailed;
		BufferData = reinterpret_cast<PFNBUFFERDATA>(IntGetProcAddress("glBufferData"));
		if(!BufferData) ++numFailed;
		BufferSubData = reinterpret_cast<PFNBUFFERSUBDATA>(IntGetProcAddress("glBufferSubData"));
		if(!BufferSubData) ++numFailed;
		DeleteBuffers = reinterpret_cast<PFNDELETEBUFFERS>(IntGetProcAddress("glDeleteBuffers"));
		if(!DeleteBuffers) ++numFailed;
		DeleteQueries = reinterpret_cast<PFNDELETEQUERIES>(IntGetProcAddress("glDeleteQueries"));
		if(!DeleteQueries) ++numFailed;
		EndQuery = reinterpret_cast<PFNENDQUERY>(IntGetProcAddress("glEndQuery"));
		if(!EndQuery) ++numFailed;
		GenBuffers = reinterpret_cast<PFNGENBUFFERS>(IntGetProcAddress("glGenBuffers"));
		if(!GenBuffers) ++numFailed;
		GenQueries = reinterpret_cast<PFNGENQUERIES>(IntGetProcAddress("glGenQueries"));
		if(!GenQueries) ++numFailed;
		GetBufferParameteriv = reinterpret_cast<PFNGETBUFFERPARAMETERIV>(IntGetProcAddress("glGetBufferParameteriv"));
		if(!GetBufferParameteriv) ++numFailed;
		GetBufferPointerv = reinterpret_cast<PFNGETBUFFERPOINTERV>(IntGetProcAddress("glGetBufferPointerv"));
		if(!GetBufferPointerv) ++numFailed;
		GetBufferSubData = reinterpret_cast<PFNGETBUFFERSUBDATA>(IntGetProcAddress("glGetBufferSubData"));
		if(!GetBufferSubData) ++numFailed;
		GetQueryObjectiv = reinterpret_cast<PFNGETQUERYOBJECTIV>(IntGetProcAddress("glGetQueryObjectiv"));
		if(!GetQueryObjectiv) ++numFailed;
		GetQueryObjectuiv = reinterpret_cast<PFNGETQUERYOBJECTUIV>(IntGetProcAddress("glGetQueryObjectuiv"));
		if(!GetQueryObjectuiv) ++numFailed;
		GetQueryiv = reinterpret_cast<PFNGETQUERYIV>(IntGetProcAddress("glGetQueryiv"));
		if(!GetQueryiv) ++numFailed;
		IsBuffer = reinterpret_cast<PFNISBUFFER>(IntGetProcAddress("glIsBuffer"));
		if(!IsBuffer) ++numFailed;
		IsQuery = reinterpret_cast<PFNISQUERY>(IntGetProcAddress("glIsQuery"));
		if(!IsQuery) ++numFailed;
		MapBuffer = reinterpret_cast<PFNMAPBUFFER>(IntGetProcAddress("glMapBuffer"));
		if(!MapBuffer) ++numFailed;
		UnmapBuffer = reinterpret_cast<PFNUNMAPBUFFER>(IntGetProcAddress("glUnmapBuffer"));
		if(!UnmapBuffer) ++numFailed;
		AttachShader = reinterpret_cast<PFNATTACHSHADER>(IntGetProcAddress("glAttachShader"));
		if(!AttachShader) ++numFailed;
		BindAttribLocation = reinterpret_cast<PFNBINDATTRIBLOCATION>(IntGetProcAddress("glBindAttribLocation"));
		if(!BindAttribLocation) ++numFailed;
		BlendEquationSeparate = reinterpret_cast<PFNBLENDEQUATIONSEPARATE>(IntGetProcAddress("glBlendEquationSeparate"));
		if(!BlendEquationSeparate) ++numFailed;
		CompileShader = reinterpret_cast<PFNCOMPILESHADER>(IntGetProcAddress("glCompileShader"));
		if(!CompileShader) ++numFailed;
		CreateProgram = reinterpret_cast<PFNCREATEPROGRAM>(IntGetProcAddress("glCreateProgram"));
		if(!CreateProgram) ++numFailed;
		CreateShader = reinterpret_cast<PFNCREATESHADER>(IntGetProcAddress("glCreateShader"));
		if(!CreateShader) ++numFailed;
		DeleteProgram = reinterpret_cast<PFNDELETEPROGRAM>(IntGetProcAddress("glDeleteProgram"));
		if(!DeleteProgram) ++numFailed;
		DeleteShader = reinterpret_cast<PFNDELETESHADER>(IntGetProcAddress("glDeleteShader"));
		if(!DeleteShader) ++numFailed;
		DetachShader = reinterpret_cast<PFNDETACHSHADER>(IntGetProcAddress("glDetachShader"));
		if(!DetachShader) ++numFailed;
		DisableVertexAttribArray = reinterpret_cast<PFNDISABLEVERTEXATTRIBARRAY>(IntGetProcAddress("glDisableVertexAttribArray"));
		if(!DisableVertexAttribArray) ++numFailed;
		DrawBuffers = reinterpret_cast<PFNDRAWBUFFERS>(IntGetProcAddress("glDrawBuffers"));
		if(!DrawBuffers) ++numFailed;
		EnableVertexAttribArray = reinterpret_cast<PFNENABLEVERTEXATTRIBARRAY>(IntGetProcAddress("glEnableVertexAttribArray"));
		if(!EnableVertexAttribArray) ++numFailed;
		GetActiveAttrib = reinterpret_cast<PFNGETACTIVEATTRIB>(IntGetProcAddress("glGetActiveAttrib"));
		if(!GetActiveAttrib) ++numFailed;
		GetActiveUniform = reinterpret_cast<PFNGETACTIVEUNIFORM>(IntGetProcAddress("glGetActiveUniform"));
		if(!GetActiveUniform) ++numFailed;
		GetAttachedShaders = reinterpret_cast<PFNGETATTACHEDSHADERS>(IntGetProcAddress("glGetAttachedShaders"));
		if(!GetAttachedShaders) ++numFailed;
		GetAttribLocation = reinterpret_cast<PFNGETATTRIBLOCATION>(IntGetProcAddress("glGetAttribLocation"));
		if(!GetAttribLocation) ++numFailed;
		GetProgramInfoLog = reinterpret_cast<PFNGETPROGRAMINFOLOG>(IntGetProcAddress("glGetProgramInfoLog"));
		if(!GetProgramInfoLog) ++numFailed;
		GetProgramiv = reinterpret_cast<PFNGETPROGRAMIV>(IntGetProcAddress("glGetProgramiv"));
		if(!GetProgramiv) ++numFailed;
		GetShaderInfoLog = reinterpret_cast<PFNGETSHADERINFOLOG>(IntGetProcAddress("glGetShaderInfoLog"));
		if(!GetShaderInfoLog) ++numFailed;
		GetShaderSource = reinterpret_cast<PFNGETSHADERSOURCE>(IntGetProcAddress("glGetShaderSource"));
		if(!GetShaderSource) ++numFailed;
		GetShaderiv = reinterpret_cast<PFNGETSHADERIV>(IntGetProcAddress("glGetShaderiv"));
		if(!GetShaderiv) ++numFailed;
		GetUniformLocation = reinterpret_cast<PFNGETUNIFORMLOCATION>(IntGetProcAddress("glGetUniformLocation"));
		if(!GetUniformLocation) ++numFailed;
		GetUniformfv = reinterpret_cast<PFNGETUNIFORMFV>(IntGetProcAddress("glGetUniformfv"));
		if(!GetUniformfv) ++numFailed;
		GetUniformiv = reinterpret_cast<PFNGETUNIFORMIV>(IntGetProcAddress("glGetUniformiv"));
		if(!GetUniformiv) ++numFailed;
		GetVertexAttribPointerv = reinterpret_cast<PFNGETVERTEXATTRIBPOINTERV>(IntGetProcAddress("glGetVertexAttribPointerv"));
		if(!GetVertexAttribPointerv) ++numFailed;
		GetVertexAttribdv = reinterpret_cast<PFNGETVERTEXATTRIBDV>(IntGetProcAddress("glGetVertexAttribdv"));
		if(!GetVertexAttribdv) ++numFailed;
		GetVertexAttribfv = reinterpret_cast<PFNGETVERTEXATTRIBFV>(IntGetProcAddress("glGetVertexAttribfv"));
		if(!GetVertexAttribfv) ++numFailed;
		GetVertexAttribiv = reinterpret_cast<PFNGETVERTEXATTRIBIV>(IntGetProcAddress("glGetVertexAttribiv"));
		if(!GetVertexAttribiv) ++numFailed;
		IsProgram = reinterpret_cast<PFNISPROGRAM>(IntGetProcAddress("glIsProgram"));
		if(!IsProgram) ++numFailed;
		IsShader = reinterpret_cast<PFNISSHADER>(IntGetProcAddress("glIsShader"));
		if(!IsShader) ++numFailed;
		LinkProgram = reinterpret_cast<PFNLINKPROGRAM>(IntGetProcAddress("glLinkProgram"));
		if(!LinkProgram) ++numFailed;
		ShaderSource = reinterpret_cast<PFNSHADERSOURCE>(IntGetProcAddress("glShaderSource"));
		if(!ShaderSource) ++numFailed;
		StencilFuncSeparate = reinterpret_cast<PFNSTENCILFUNCSEPARATE>(IntGetProcAddress("glStencilFuncSeparate"));
		if(!StencilFuncSeparate) ++numFailed;
		StencilMaskSeparate = reinterpret_cast<PFNSTENCILMASKSEPARATE>(IntGetProcAddress("glStencilMaskSeparate"));
		if(!StencilMaskSeparate) ++numFailed;
		StencilOpSeparate = reinterpret_cast<PFNSTENCILOPSEPARATE>(IntGetProcAddress("glStencilOpSeparate"));
		if(!StencilOpSeparate) ++numFailed;
		Uniform1f = reinterpret_cast<PFNUNIFORM1F>(IntGetProcAddress("glUniform1f"));
		if(!Uniform1f) ++numFailed;
		Uniform1fv = reinterpret_cast<PFNUNIFORM1FV>(IntGetProcAddress("glUniform1fv"));
		if(!Uniform1fv) ++numFailed;
		Uniform1i = reinterpret_cast<PFNUNIFORM1I>(IntGetProcAddress("glUniform1i"));
		if(!Uniform1i) ++numFailed;
		Uniform1iv = reinterpret_cast<PFNUNIFORM1IV>(IntGetProcAddress("glUniform1iv"));
		if(!Uniform1iv) ++numFailed;
		Uniform2f = reinterpret_cast<PFNUNIFORM2F>(IntGetProcAddress("glUniform2f"));
		if(!Uniform2f) ++numFailed;
		Uniform2fv = reinterpret_cast<PFNUNIFORM2FV>(IntGetProcAddress("glUniform2fv"));
		if(!Uniform2fv) ++numFailed;
		Uniform2i = reinterpret_cast<PFNUNIFORM2I>(IntGetProcAddress("glUniform2i"));
		if(!Uniform2i) ++numFailed;
		Uniform2iv = reinterpret_cast<PFNUNIFORM2IV>(IntGetProcAddress("glUniform2iv"));
		if(!Uniform2iv) ++numFailed;
		Uniform3f = reinterpret_cast<PFNUNIFORM3F>(IntGetProcAddress("glUniform3f"));
		if(!Uniform3f) ++numFailed;
		Uniform3fv = reinterpret_cast<PFNUNIFORM3FV>(IntGetProcAddress("glUniform3fv"));
		if(!Uniform3fv) ++numFailed;
		Uniform3i = reinterpret_cast<PFNUNIFORM3I>(IntGetProcAddress("glUniform3i"));
		if(!Uniform3i) ++numFailed;
		Uniform3iv = reinterpret_cast<PFNUNIFORM3IV>(IntGetProcAddress("glUniform3iv"));
		if(!Uniform3iv) ++numFailed;
		Uniform4f = reinterpret_cast<PFNUNIFORM4F>(IntGetProcAddress("glUniform4f"));
		if(!Uniform4f) ++numFailed;
		Uniform4fv = reinterpret_cast<PFNUNIFORM4FV>(IntGetProcAddress("glUniform4fv"));
		if(!Uniform4fv) ++numFailed;
		Uniform4i = reinterpret_cast<PFNUNIFORM4I>(IntGetProcAddress("glUniform4i"));
		if(!Uniform4i) ++numFailed;
		Uniform4iv = reinterpret_cast<PFNUNIFORM4IV>(IntGetProcAddress("glUniform4iv"));
		if(!Uniform4iv) ++numFailed;
		UniformMatrix2fv = reinterpret_cast<PFNUNIFORMMATRIX2FV>(IntGetProcAddress("glUniformMatrix2fv"));
		if(!UniformMatrix2fv) ++numFailed;
		UniformMatrix3fv = reinterpret_cast<PFNUNIFORMMATRIX3FV>(IntGetProcAddress("glUniformMatrix3fv"));
		if(!UniformMatrix3fv) ++numFailed;
		UniformMatrix4fv = reinterpret_cast<PFNUNIFORMMATRIX4FV>(IntGetProcAddress("glUniformMatrix4fv"));
		if(!UniformMatrix4fv) ++numFailed;
		UseProgram = reinterpret_cast<PFNUSEPROGRAM>(IntGetProcAddress("glUseProgram"));
		if(!UseProgram) ++numFailed;
		ValidateProgram = reinterpret_cast<PFNVALIDATEPROGRAM>(IntGetProcAddress("glValidateProgram"));
		if(!ValidateProgram) ++numFailed;
		VertexAttrib1d = reinterpret_cast<PFNVERTEXATTRIB1D>(IntGetProcAddress("glVertexAttrib1d"));
		if(!VertexAttrib1d) ++numFailed;
		VertexAttrib1dv = reinterpret_cast<PFNVERTEXATTRIB1DV>(IntGetProcAddress("glVertexAttrib1dv"));
		if(!VertexAttrib1dv) ++numFailed;
		VertexAttrib1f = reinterpret_cast<PFNVERTEXATTRIB1F>(IntGetProcAddress("glVertexAttrib1f"));
		if(!VertexAttrib1f) ++numFailed;
		VertexAttrib1fv = reinterpret_cast<PFNVERTEXATTRIB1FV>(IntGetProcAddress("glVertexAttrib1fv"));
		if(!VertexAttrib1fv) ++numFailed;
		VertexAttrib1s = reinterpret_cast<PFNVERTEXATTRIB1S>(IntGetProcAddress("glVertexAttrib1s"));
		if(!VertexAttrib1s) ++numFailed;
		VertexAttrib1sv = reinterpret_cast<PFNVERTEXATTRIB1SV>(IntGetProcAddress("glVertexAttrib1sv"));
		if(!VertexAttrib1sv) ++numFailed;
		VertexAttrib2d = reinterpret_cast<PFNVERTEXATTRIB2D>(IntGetProcAddress("glVertexAttrib2d"));
		if(!VertexAttrib2d) ++numFailed;
		VertexAttrib2dv = reinterpret_cast<PFNVERTEXATTRIB2DV>(IntGetProcAddress("glVertexAttrib2dv"));
		if(!VertexAttrib2dv) ++numFailed;
		VertexAttrib2f = reinterpret_cast<PFNVERTEXATTRIB2F>(IntGetProcAddress("glVertexAttrib2f"));
		if(!VertexAttrib2f) ++numFailed;
		VertexAttrib2fv = reinterpret_cast<PFNVERTEXATTRIB2FV>(IntGetProcAddress("glVertexAttrib2fv"));
		if(!VertexAttrib2fv) ++numFailed;
		VertexAttrib2s = reinterpret_cast<PFNVERTEXATTRIB2S>(IntGetProcAddress("glVertexAttrib2s"));
		if(!VertexAttrib2s) ++numFailed;
		VertexAttrib2sv = reinterpret_cast<PFNVERTEXATTRIB2SV>(IntGetProcAddress("glVertexAttrib2sv"));
		if(!VertexAttrib2sv) ++numFailed;
		VertexAttrib3d = reinterpret_cast<PFNVERTEXATTRIB3D>(IntGetProcAddress("glVertexAttrib3d"));
		if(!VertexAttrib3d) ++numFailed;
		VertexAttrib3dv = reinterpret_cast<PFNVERTEXATTRIB3DV>(IntGetProcAddress("glVertexAttrib3dv"));
		if(!VertexAttrib3dv) ++numFailed;
		VertexAttrib3f = reinterpret_cast<PFNVERTEXATTRIB3F>(IntGetProcAddress("glVertexAttrib3f"));
		if(!VertexAttrib3f) ++numFailed;
		VertexAttrib3fv = reinterpret_cast<PFNVERTEXATTRIB3FV>(IntGetProcAddress("glVertexAttrib3fv"));
		if(!VertexAttrib3fv) ++numFailed;
		VertexAttrib3s = reinterpret_cast<PFNVERTEXATTRIB3S>(IntGetProcAddress("glVertexAttrib3s"));
		if(!VertexAttrib3s) ++numFailed;
		VertexAttrib3sv = reinterpret_cast<PFNVERTEXATTRIB3SV>(IntGetProcAddress("glVertexAttrib3sv"));
		if(!VertexAttrib3sv) ++numFailed;
		VertexAttrib4Nbv = reinterpret_cast<PFNVERTEXATTRIB4NBV>(IntGetProcAddress("glVertexAttrib4Nbv"));
		if(!VertexAttrib4Nbv) ++numFailed;
		VertexAttrib4Niv = reinterpret_cast<PFNVERTEXATTRIB4NIV>(IntGetProcAddress("glVertexAttrib4Niv"));
		if(!VertexAttrib4Niv) ++numFailed;
		VertexAttrib4Nsv = reinterpret_cast<PFNVERTEXATTRIB4NSV>(IntGetProcAddress("glVertexAttrib4Nsv"));
		if(!VertexAttrib4Nsv) ++numFailed;
		VertexAttrib4Nub = reinterpret_cast<PFNVERTEXATTRIB4NUB>(IntGetProcAddress("glVertexAttrib4Nub"));
		if(!VertexAttrib4Nub) ++numFailed;
		VertexAttrib4Nubv = reinterpret_cast<PFNVERTEXATTRIB4NUBV>(IntGetProcAddress("glVertexAttrib4Nubv"));
		if(!VertexAttrib4Nubv) ++numFailed;
		VertexAttrib4Nuiv = reinterpret_cast<PFNVERTEXATTRIB4NUIV>(IntGetProcAddress("glVertexAttrib4Nuiv"));
		if(!VertexAttrib4Nuiv) ++numFailed;
		VertexAttrib4Nusv = reinterpret_cast<PFNVERTEXATTRIB4NUSV>(IntGetProcAddress("glVertexAttrib4Nusv"));
		if(!VertexAttrib4Nusv) ++numFailed;
		VertexAttrib4bv = reinterpret_cast<PFNVERTEXATTRIB4BV>(IntGetProcAddress("glVertexAttrib4bv"));
		if(!VertexAttrib4bv) ++numFailed;
		VertexAttrib4d = reinterpret_cast<PFNVERTEXATTRIB4D>(IntGetProcAddress("glVertexAttrib4d"));
		if(!VertexAttrib4d) ++numFailed;
		VertexAttrib4dv = reinterpret_cast<PFNVERTEXATTRIB4DV>(IntGetProcAddress("glVertexAttrib4dv"));
		if(!VertexAttrib4dv) ++numFailed;
		VertexAttrib4f = reinterpret_cast<PFNVERTEXATTRIB4F>(IntGetProcAddress("glVertexAttrib4f"));
		if(!VertexAttrib4f) ++numFailed;
		VertexAttrib4fv = reinterpret_cast<PFNVERTEXATTRIB4FV>(IntGetProcAddress("glVertexAttrib4fv"));
		if(!VertexAttrib4fv) ++numFailed;
		VertexAttrib4iv = reinterpret_cast<PFNVERTEXATTRIB4IV>(IntGetProcAddress("glVertexAttrib4iv"));
		if(!VertexAttrib4iv) ++numFailed;
		VertexAttrib4s = reinterpret_cast<PFNVERTEXATTRIB4S>(IntGetProcAddress("glVertexAttrib4s"));
		if(!VertexAttrib4s) ++numFailed;
		VertexAttrib4sv = reinterpret_cast<PFNVERTEXATTRIB4SV>(IntGetProcAddress("glVertexAttrib4sv"));
		if(!VertexAttrib4sv) ++numFailed;
		VertexAttrib4ubv = reinterpret_cast<PFNVERTEXATTRIB4UBV>(IntGetProcAddress("glVertexAttrib4ubv"));
		if(!VertexAttrib4ubv) ++numFailed;
		VertexAttrib4uiv = reinterpret_cast<PFNVERTEXATTRIB4UIV>(IntGetProcAddress("glVertexAttrib4uiv"));
		if(!VertexAttrib4uiv) ++numFailed;
		VertexAttrib4usv = reinterpret_cast<PFNVERTEXATTRIB4USV>(IntGetProcAddress("glVertexAttrib4usv"));
		if(!VertexAttrib4usv) ++numFailed;
		VertexAttribPointer = reinterpret_cast<PFNVERTEXATTRIBPOINTER>(IntGetProcAddress("glVertexAttribPointer"));
		if(!VertexAttribPointer) ++numFailed;
		UniformMatrix2x3fv = reinterpret_cast<PFNUNIFORMMATRIX2X3FV>(IntGetProcAddress("glUniformMatrix2x3fv"));
		if(!UniformMatrix2x3fv) ++numFailed;
		UniformMatrix2x4fv = reinterpret_cast<PFNUNIFORMMATRIX2X4FV>(IntGetProcAddress("glUniformMatrix2x4fv"));
		if(!UniformMatrix2x4fv) ++numFailed;
		UniformMatrix3x2fv = reinterpret_cast<PFNUNIFORMMATRIX3X2FV>(IntGetProcAddress("glUniformMatrix3x2fv"));
		if(!UniformMatrix3x2fv) ++numFailed;
		UniformMatrix3x4fv = reinterpret_cast<PFNUNIFORMMATRIX3X4FV>(IntGetProcAddress("glUniformMatrix3x4fv"));
		if(!UniformMatrix3x4fv) ++numFailed;
		UniformMatrix4x2fv = reinterpret_cast<PFNUNIFORMMATRIX4X2FV>(IntGetProcAddress("glUniformMatrix4x2fv"));
		if(!UniformMatrix4x2fv) ++numFailed;
		UniformMatrix4x3fv = reinterpret_cast<PFNUNIFORMMATRIX4X3FV>(IntGetProcAddress("glUniformMatrix4x3fv"));
		if(!UniformMatrix4x3fv) ++numFailed;
		BeginConditionalRender = reinterpret_cast<PFNBEGINCONDITIONALRENDER>(IntGetProcAddress("glBeginConditionalRender"));
		if(!BeginConditionalRender) ++numFailed;
		BeginTransformFeedback = reinterpret_cast<PFNBEGINTRANSFORMFEEDBACK>(IntGetProcAddress("glBeginTransformFeedback"));
		if(!BeginTransformFeedback) ++numFailed;
		BindBufferBase = reinterpret_cast<PFNBINDBUFFERBASE>(IntGetProcAddress("glBindBufferBase"));
		if(!BindBufferBase) ++numFailed;
		BindBufferRange = reinterpret_cast<PFNBINDBUFFERRANGE>(IntGetProcAddress("glBindBufferRange"));
		if(!BindBufferRange) ++numFailed;
		BindFragDataLocation = reinterpret_cast<PFNBINDFRAGDATALOCATION>(IntGetProcAddress("glBindFragDataLocation"));
		if(!BindFragDataLocation) ++numFailed;
		BindFramebuffer = reinterpret_cast<PFNBINDFRAMEBUFFER>(IntGetProcAddress("glBindFramebuffer"));
		if(!BindFramebuffer) ++numFailed;
		BindRenderbuffer = reinterpret_cast<PFNBINDRENDERBUFFER>(IntGetProcAddress("glBindRenderbuffer"));
		if(!BindRenderbuffer) ++numFailed;
		BindVertexArray = reinterpret_cast<PFNBINDVERTEXARRAY>(IntGetProcAddress("glBindVertexArray"));
		if(!BindVertexArray) ++numFailed;
		BlitFramebuffer = reinterpret_cast<PFNBLITFRAMEBUFFER>(IntGetProcAddress("glBlitFramebuffer"));
		if(!BlitFramebuffer) ++numFailed;
		CheckFramebufferStatus = reinterpret_cast<PFNCHECKFRAMEBUFFERSTATUS>(IntGetProcAddress("glCheckFramebufferStatus"));
		if(!CheckFramebufferStatus) ++numFailed;
		ClampColor = reinterpret_cast<PFNCLAMPCOLOR>(IntGetProcAddress("glClampColor"));
		if(!ClampColor) ++numFailed;
		ClearBufferfi = reinterpret_cast<PFNCLEARBUFFERFI>(IntGetProcAddress("glClearBufferfi"));
		if(!ClearBufferfi) ++numFailed;
		ClearBufferfv = reinterpret_cast<PFNCLEARBUFFERFV>(IntGetProcAddress("glClearBufferfv"));
		if(!ClearBufferfv) ++numFailed;
		ClearBufferiv = reinterpret_cast<PFNCLEARBUFFERIV>(IntGetProcAddress("glClearBufferiv"));
		if(!ClearBufferiv) ++numFailed;
		ClearBufferuiv = reinterpret_cast<PFNCLEARBUFFERUIV>(IntGetProcAddress("glClearBufferuiv"));
		if(!ClearBufferuiv) ++numFailed;
		ColorMaski = reinterpret_cast<PFNCOLORMASKI>(IntGetProcAddress("glColorMaski"));
		if(!ColorMaski) ++numFailed;
		DeleteFramebuffers = reinterpret_cast<PFNDELETEFRAMEBUFFERS>(IntGetProcAddress("glDeleteFramebuffers"));
		if(!DeleteFramebuffers) ++numFailed;
		DeleteRenderbuffers = reinterpret_cast<PFNDELETERENDERBUFFERS>(IntGetProcAddress("glDeleteRenderbuffers"));
		if(!DeleteRenderbuffers) ++numFailed;
		DeleteVertexArrays = reinterpret_cast<PFNDELETEVERTEXARRAYS>(IntGetProcAddress("glDeleteVertexArrays"));
		if(!DeleteVertexArrays) ++numFailed;
		Disablei = reinterpret_cast<PFNDISABLEI>(IntGetProcAddress("glDisablei"));
		if(!Disablei) ++numFailed;
		Enablei = reinterpret_cast<PFNENABLEI>(IntGetProcAddress("glEnablei"));
		if(!Enablei) ++numFailed;
		EndConditionalRender = reinterpret_cast<PFNENDCONDITIONALRENDER>(IntGetProcAddress("glEndConditionalRender"));
		if(!EndConditionalRender) ++numFailed;
		EndTransformFeedback = reinterpret_cast<PFNENDTRANSFORMFEEDBACK>(IntGetProcAddress("glEndTransformFeedback"));
		if(!EndTransformFeedback) ++numFailed;
		FlushMappedBufferRange = reinterpret_cast<PFNFLUSHMAPPEDBUFFERRANGE>(IntGetProcAddress("glFlushMappedBufferRange"));
		if(!FlushMappedBufferRange) ++numFailed;
		FramebufferRenderbuffer = reinterpret_cast<PFNFRAMEBUFFERRENDERBUFFER>(IntGetProcAddress("glFramebufferRenderbuffer"));
		if(!FramebufferRenderbuffer) ++numFailed;
		FramebufferTexture1D = reinterpret_cast<PFNFRAMEBUFFERTEXTURE1D>(IntGetProcAddress("glFramebufferTexture1D"));
		if(!FramebufferTexture1D) ++numFailed;
		FramebufferTexture2D = reinterpret_cast<PFNFRAMEBUFFERTEXTURE2D>(IntGetProcAddress("glFramebufferTexture2D"));
		if(!FramebufferTexture2D) ++numFailed;
		FramebufferTexture3D = reinterpret_cast<PFNFRAMEBUFFERTEXTURE3D>(IntGetProcAddress("glFramebufferTexture3D"));
		if(!FramebufferTexture3D) ++numFailed;
		FramebufferTextureLayer = reinterpret_cast<PFNFRAMEBUFFERTEXTURELAYER>(IntGetProcAddress("glFramebufferTextureLayer"));
		if(!FramebufferTextureLayer) ++numFailed;
		GenFramebuffers = reinterpret_cast<PFNGENFRAMEBUFFERS>(IntGetProcAddress("glGenFramebuffers"));
		if(!GenFramebuffers) ++numFailed;
		GenRenderbuffers = reinterpret_cast<PFNGENRENDERBUFFERS>(IntGetProcAddress("glGenRenderbuffers"));
		if(!GenRenderbuffers) ++numFailed;
		GenVertexArrays = reinterpret_cast<PFNGENVERTEXARRAYS>(IntGetProcAddress("glGenVertexArrays"));
		if(!GenVertexArrays) ++numFailed;
		GenerateMipmap = reinterpret_cast<PFNGENERATEMIPMAP>(IntGetProcAddress("glGenerateMipmap"));
		if(!GenerateMipmap) ++numFailed;
		GetBooleani_v = reinterpret_cast<PFNGETBOOLEANI_V>(IntGetProcAddress("glGetBooleani_v"));
		if(!GetBooleani_v) ++numFailed;
		GetFragDataLocation = reinterpret_cast<PFNGETFRAGDATALOCATION>(IntGetProcAddress("glGetFragDataLocation"));
		if(!GetFragDataLocation) ++numFailed;
		GetFramebufferAttachmentParameteriv = reinterpret_cast<PFNGETFRAMEBUFFERATTACHMENTPARAMETERIV>(IntGetProcAddress("glGetFramebufferAttachmentParameteriv"));
		if(!GetFramebufferAttachmentParameteriv) ++numFailed;
		GetIntegeri_v = reinterpret_cast<PFNGETINTEGERI_V>(IntGetProcAddress("glGetIntegeri_v"));
		if(!GetIntegeri_v) ++numFailed;
		GetRenderbufferParameteriv = reinterpret_cast<PFNGETRENDERBUFFERPARAMETERIV>(IntGetProcAddress("glGetRenderbufferParameteriv"));
		if(!GetRenderbufferParameteriv) ++numFailed;
		GetStringi = reinterpret_cast<PFNGETSTRINGI>(IntGetProcAddress("glGetStringi"));
		if(!GetStringi) ++numFailed;
		GetTexParameterIiv = reinterpret_cast<PFNGETTEXPARAMETERIIV>(IntGetProcAddress("glGetTexParameterIiv"));
		if(!GetTexParameterIiv) ++numFailed;
		GetTexParameterIuiv = reinterpret_cast<PFNGETTEXPARAMETERIUIV>(IntGetProcAddress("glGetTexParameterIuiv"));
		if(!GetTexParameterIuiv) ++numFailed;
		GetTransformFeedbackVarying = reinterpret_cast<PFNGETTRANSFORMFEEDBACKVARYING>(IntGetProcAddress("glGetTransformFeedbackVarying"));
		if(!GetTransformFeedbackVarying) ++numFailed;
		GetUniformuiv = reinterpret_cast<PFNGETUNIFORMUIV>(IntGetProcAddress("glGetUniformuiv"));
		if(!GetUniformuiv) ++numFailed;
		GetVertexAttribIiv = reinterpret_cast<PFNGETVERTEXATTRIBIIV>(IntGetProcAddress("glGetVertexAttribIiv"));
		if(!GetVertexAttribIiv) ++numFailed;
		GetVertexAttribIuiv = reinterpret_cast<PFNGETVERTEXATTRIBIUIV>(IntGetProcAddress("glGetVertexAttribIuiv"));
		if(!GetVertexAttribIuiv) ++numFailed;
		IsEnabledi = reinterpret_cast<PFNISENABLEDI>(IntGetProcAddress("glIsEnabledi"));
		if(!IsEnabledi) ++numFailed;
		IsFramebuffer = reinterpret_cast<PFNISFRAMEBUFFER>(IntGetProcAddress("glIsFramebuffer"));
		if(!IsFramebuffer) ++numFailed;
		IsRenderbuffer = reinterpret_cast<PFNISRENDERBUFFER>(IntGetProcAddress("glIsRenderbuffer"));
		if(!IsRenderbuffer) ++numFailed;
		IsVertexArray = reinterpret_cast<PFNISVERTEXARRAY>(IntGetProcAddress("glIsVertexArray"));
		if(!IsVertexArray) ++numFailed;
		MapBufferRange = reinterpret_cast<PFNMAPBUFFERRANGE>(IntGetProcAddress("glMapBufferRange"));
		if(!MapBufferRange) ++numFailed;
		RenderbufferStorage = reinterpret_cast<PFNRENDERBUFFERSTORAGE>(IntGetProcAddress("glRenderbufferStorage"));
		if(!RenderbufferStorage) ++numFailed;
		RenderbufferStorageMultisample = reinterpret_cast<PFNRENDERBUFFERSTORAGEMULTISAMPLE>(IntGetProcAddress("glRenderbufferStorageMultisample"));
		if(!RenderbufferStorageMultisample) ++numFailed;
		TexParameterIiv = reinterpret_cast<PFNTEXPARAMETERIIV>(IntGetProcAddress("glTexParameterIiv"));
		if(!TexParameterIiv) ++numFailed;
		TexParameterIuiv = reinterpret_cast<PFNTEXPARAMETERIUIV>(IntGetProcAddress("glTexParameterIuiv"));
		if(!TexParameterIuiv) ++numFailed;
		TransformFeedbackVaryings = reinterpret_cast<PFNTRANSFORMFEEDBACKVARYINGS>(IntGetProcAddress("glTransformFeedbackVaryings"));
		if(!TransformFeedbackVaryings) ++numFailed;
		Uniform1ui = reinterpret_cast<PFNUNIFORM1UI>(IntGetProcAddress("glUniform1ui"));
		if(!Uniform1ui) ++numFailed;
		Uniform1uiv = reinterpret_cast<PFNUNIFORM1UIV>(IntGetProcAddress("glUniform1uiv"));
		if(!Uniform1uiv) ++numFailed;
		Uniform2ui = reinterpret_cast<PFNUNIFORM2UI>(IntGetProcAddress("glUniform2ui"));
		if(!Uniform2ui) ++numFailed;
		Uniform2uiv = reinterpret_cast<PFNUNIFORM2UIV>(IntGetProcAddress("glUniform2uiv"));
		if(!Uniform2uiv) ++numFailed;
		Uniform3ui = reinterpret_cast<PFNUNIFORM3UI>(IntGetProcAddress("glUniform3ui"));
		if(!Uniform3ui) ++numFailed;
		Uniform3uiv = reinterpret_cast<PFNUNIFORM3UIV>(IntGetProcAddress("glUniform3uiv"));
		if(!Uniform3uiv) ++numFailed;
		Uniform4ui = reinterpret_cast<PFNUNIFORM4UI>(IntGetProcAddress("glUniform4ui"));
		if(!Uniform4ui) ++numFailed;
		Uniform4uiv = reinterpret_cast<PFNUNIFORM4UIV>(IntGetProcAddress("glUniform4uiv"));
		if(!Uniform4uiv) ++numFailed;
		VertexAttribI1i = reinterpret_cast<PFNVERTEXATTRIBI1I>(IntGetProcAddress("glVertexAttribI1i"));
		if(!VertexAttribI1i) ++numFailed;
		VertexAttribI1iv = reinterpret_cast<PFNVERTEXATTRIBI1IV>(IntGetProcAddress("glVertexAttribI1iv"));
		if(!VertexAttribI1iv) ++numFailed;
		VertexAttribI1ui = reinterpret_cast<PFNVERTEXATTRIBI1UI>(IntGetProcAddress("glVertexAttribI1ui"));
		if(!VertexAttribI1ui) ++numFailed;
		VertexAttribI1uiv = reinterpret_cast<PFNVERTEXATTRIBI1UIV>(IntGetProcAddress("glVertexAttribI1uiv"));
		if(!VertexAttribI1uiv) ++numFailed;
		VertexAttribI2i = reinterpret_cast<PFNVERTEXATTRIBI2I>(IntGetProcAddress("glVertexAttribI2i"));
		if(!VertexAttribI2i) ++numFailed;
		VertexAttribI2iv = reinterpret_cast<PFNVERTEXATTRIBI2IV>(IntGetProcAddress("glVertexAttribI2iv"));
		if(!VertexAttribI2iv) ++numFailed;
		VertexAttribI2ui = reinterpret_cast<PFNVERTEXATTRIBI2UI>(IntGetProcAddress("glVertexAttribI2ui"));
		if(!VertexAttribI2ui) ++numFailed;
		VertexAttribI2uiv = reinterpret_cast<PFNVERTEXATTRIBI2UIV>(IntGetProcAddress("glVertexAttribI2uiv"));
		if(!VertexAttribI2uiv) ++numFailed;
		VertexAttribI3i = reinterpret_cast<PFNVERTEXATTRIBI3I>(IntGetProcAddress("glVertexAttribI3i"));
		if(!VertexAttribI3i) ++numFailed;
		VertexAttribI3iv = reinterpret_cast<PFNVERTEXATTRIBI3IV>(IntGetProcAddress("glVertexAttribI3iv"));
		if(!VertexAttribI3iv) ++numFailed;
		VertexAttribI3ui = reinterpret_cast<PFNVERTEXATTRIBI3UI>(IntGetProcAddress("glVertexAttribI3ui"));
		if(!VertexAttribI3ui) ++numFailed;
		VertexAttribI3uiv = reinterpret_cast<PFNVERTEXATTRIBI3UIV>(IntGetProcAddress("glVertexAttribI3uiv"));
		if(!VertexAttribI3uiv) ++numFailed;
		VertexAttribI4bv = reinterpret_cast<PFNVERTEXATTRIBI4BV>(IntGetProcAddress("glVertexAttribI4bv"));
		if(!VertexAttribI4bv) ++numFailed;
		VertexAttribI4i = reinterpret_cast<PFNVERTEXATTRIBI4I>(IntGetProcAddress("glVertexAttribI4i"));
		if(!VertexAttribI4i) ++numFailed;
		VertexAttribI4iv = reinterpret_cast<PFNVERTEXATTRIBI4IV>(IntGetProcAddress("glVertexAttribI4iv"));
		if(!VertexAttribI4iv) ++numFailed;
		VertexAttribI4sv = reinterpret_cast<PFNVERTEXATTRIBI4SV>(IntGetProcAddress("glVertexAttribI4sv"));
		if(!VertexAttribI4sv) ++numFailed;
		VertexAttribI4ubv = reinterpret_cast<PFNVERTEXATTRIBI4UBV>(IntGetProcAddress("glVertexAttribI4ubv"));
		if(!VertexAttribI4ubv) ++numFailed;
		VertexAttribI4ui = reinterpret_cast<PFNVERTEXATTRIBI4UI>(IntGetProcAddress("glVertexAttribI4ui"));
		if(!VertexAttribI4ui) ++numFailed;
		VertexAttribI4uiv = reinterpret_cast<PFNVERTEXATTRIBI4UIV>(IntGetProcAddress("glVertexAttribI4uiv"));
		if(!VertexAttribI4uiv) ++numFailed;
		VertexAttribI4usv = reinterpret_cast<PFNVERTEXATTRIBI4USV>(IntGetProcAddress("glVertexAttribI4usv"));
		if(!VertexAttribI4usv) ++numFailed;
		VertexAttribIPointer = reinterpret_cast<PFNVERTEXATTRIBIPOINTER>(IntGetProcAddress("glVertexAttribIPointer"));
		if(!VertexAttribIPointer) ++numFailed;
		CopyBufferSubData = reinterpret_cast<PFNCOPYBUFFERSUBDATA>(IntGetProcAddress("glCopyBufferSubData"));
		if(!CopyBufferSubData) ++numFailed;
		DrawArraysInstanced = reinterpret_cast<PFNDRAWARRAYSINSTANCED>(IntGetProcAddress("glDrawArraysInstanced"));
		if(!DrawArraysInstanced) ++numFailed;
		DrawElementsInstanced = reinterpret_cast<PFNDRAWELEMENTSINSTANCED>(IntGetProcAddress("glDrawElementsInstanced"));
		if(!DrawElementsInstanced) ++numFailed;
		GetActiveUniformBlockName = reinterpret_cast<PFNGETACTIVEUNIFORMBLOCKNAME>(IntGetProcAddress("glGetActiveUniformBlockName"));
		if(!GetActiveUniformBlockName) ++numFailed;
		GetActiveUniformBlockiv = reinterpret_cast<PFNGETACTIVEUNIFORMBLOCKIV>(IntGetProcAddress("glGetActiveUniformBlockiv"));
		if(!GetActiveUniformBlockiv) ++numFailed;
		GetActiveUniformName = reinterpret_cast<PFNGETACTIVEUNIFORMNAME>(IntGetProcAddress("glGetActiveUniformName"));
		if(!GetActiveUniformName) ++numFailed;
		GetActiveUniformsiv = reinterpret_cast<PFNGETACTIVEUNIFORMSIV>(IntGetProcAddress("glGetActiveUniformsiv"));
		if(!GetActiveUniformsiv) ++numFailed;
		GetUniformBlockIndex = reinterpret_cast<PFNGETUNIFORMBLOCKINDEX>(IntGetProcAddress("glGetUniformBlockIndex"));
		if(!GetUniformBlockIndex) ++numFailed;
		GetUniformIndices = reinterpret_cast<PFNGETUNIFORMINDICES>(IntGetProcAddress("glGetUniformIndices"));
		if(!GetUniformIndices) ++numFailed;
		PrimitiveRestartIndex = reinterpret_cast<PFNPRIMITIVERESTARTINDEX>(IntGetProcAddress("glPrimitiveRestartIndex"));
		if(!PrimitiveRestartIndex) ++numFailed;
		TexBuffer = reinterpret_cast<PFNTEXBUFFER>(IntGetProcAddress("glTexBuffer"));
		if(!TexBuffer) ++numFailed;
		UniformBlockBinding = reinterpret_cast<PFNUNIFORMBLOCKBINDING>(IntGetProcAddress("glUniformBlockBinding"));
		if(!UniformBlockBinding) ++numFailed;
		ClientWaitSync = reinterpret_cast<PFNCLIENTWAITSYNC>(IntGetProcAddress("glClientWaitSync"));
		if(!ClientWaitSync) ++numFailed;
		DeleteSync = reinterpret_cast<PFNDELETESYNC>(IntGetProcAddress("glDeleteSync"));
		if(!DeleteSync) ++numFailed;
		DrawElementsBaseVertex = reinterpret_cast<PFNDRAWELEMENTSBASEVERTEX>(IntGetProcAddress("glDrawElementsBaseVertex"));
		if(!DrawElementsBaseVertex) ++numFailed;
		DrawElementsInstancedBaseVertex = reinterpret_cast<PFNDRAWELEMENTSINSTANCEDBASEVERTEX>(IntGetProcAddress("glDrawElementsInstancedBaseVertex"));
		if(!DrawElementsInstancedBaseVertex) ++numFailed;
		DrawRangeElementsBaseVertex = reinterpret_cast<PFNDRAWRANGEELEMENTSBASEVERTEX>(IntGetProcAddress("glDrawRangeElementsBaseVertex"));
		if(!DrawRangeElementsBaseVertex) ++numFailed;
		FenceSync = reinterpret_cast<PFNFENCESYNC>(IntGetProcAddress("glFenceSync"));
		if(!FenceSync) ++numFailed;
		FramebufferTexture = reinterpret_cast<PFNFRAMEBUFFERTEXTURE>(IntGetProcAddress("glFramebufferTexture"));
		if(!FramebufferTexture) ++numFailed;
		GetBufferParameteri64v = reinterpret_cast<PFNGETBUFFERPARAMETERI64V>(IntGetProcAddress("glGetBufferParameteri64v"));
		if(!GetBufferParameteri64v) ++numFailed;
		GetInteger64i_v = reinterpret_cast<PFNGETINTEGER64I_V>(IntGetProcAddress("glGetInteger64i_v"));
		if(!GetInteger64i_v) ++numFailed;
		GetInteger64v = reinterpret_cast<PFNGETINTEGER64V>(IntGetProcAddress("glGetInteger64v"));
		if(!GetInteger64v) ++numFailed;
		GetMultisamplefv = reinterpret_cast<PFNGETMULTISAMPLEFV>(IntGetProcAddress("glGetMultisamplefv"));
		if(!GetMultisamplefv) ++numFailed;
		GetSynciv = reinterpret_cast<PFNGETSYNCIV>(IntGetProcAddress("glGetSynciv"));
		if(!GetSynciv) ++numFailed;
		IsSync = reinterpret_cast<PFNISSYNC>(IntGetProcAddress("glIsSync"));
		if(!IsSync) ++numFailed;
		MultiDrawElementsBaseVertex = reinterpret_cast<PFNMULTIDRAWELEMENTSBASEVERTEX>(IntGetProcAddress("glMultiDrawElementsBaseVertex"));
		if(!MultiDrawElementsBaseVertex) ++numFailed;
		ProvokingVertex = reinterpret_cast<PFNPROVOKINGVERTEX>(IntGetProcAddress("glProvokingVertex"));
		if(!ProvokingVertex) ++numFailed;
		SampleMaski = reinterpret_cast<PFNSAMPLEMASKI>(IntGetProcAddress("glSampleMaski"));
		if(!SampleMaski) ++numFailed;
		TexImage2DMultisample = reinterpret_cast<PFNTEXIMAGE2DMULTISAMPLE>(IntGetProcAddress("glTexImage2DMultisample"));
		if(!TexImage2DMultisample) ++numFailed;
		TexImage3DMultisample = reinterpret_cast<PFNTEXIMAGE3DMULTISAMPLE>(IntGetProcAddress("glTexImage3DMultisample"));
		if(!TexImage3DMultisample) ++numFailed;
		WaitSync = reinterpret_cast<PFNWAITSYNC>(IntGetProcAddress("glWaitSync"));
		if(!WaitSync) ++numFailed;
		BindFragDataLocationIndexed = reinterpret_cast<PFNBINDFRAGDATALOCATIONINDEXED>(IntGetProcAddress("glBindFragDataLocationIndexed"));
		if(!BindFragDataLocationIndexed) ++numFailed;
		BindSampler = reinterpret_cast<PFNBINDSAMPLER>(IntGetProcAddress("glBindSampler"));
		if(!BindSampler) ++numFailed;
		DeleteSamplers = reinterpret_cast<PFNDELETESAMPLERS>(IntGetProcAddress("glDeleteSamplers"));
		if(!DeleteSamplers) ++numFailed;
		GenSamplers = reinterpret_cast<PFNGENSAMPLERS>(IntGetProcAddress("glGenSamplers"));
		if(!GenSamplers) ++numFailed;
		GetFragDataIndex = reinterpret_cast<PFNGETFRAGDATAINDEX>(IntGetProcAddress("glGetFragDataIndex"));
		if(!GetFragDataIndex) ++numFailed;
		GetQueryObjecti64v = reinterpret_cast<PFNGETQUERYOBJECTI64V>(IntGetProcAddress("glGetQueryObjecti64v"));
		if(!GetQueryObjecti64v) ++numFailed;
		GetQueryObjectui64v = reinterpret_cast<PFNGETQUERYOBJECTUI64V>(IntGetProcAddress("glGetQueryObjectui64v"));
		if(!GetQueryObjectui64v) ++numFailed;
		GetSamplerParameterIiv = reinterpret_cast<PFNGETSAMPLERPARAMETERIIV>(IntGetProcAddress("glGetSamplerParameterIiv"));
		if(!GetSamplerParameterIiv) ++numFailed;
		GetSamplerParameterIuiv = reinterpret_cast<PFNGETSAMPLERPARAMETERIUIV>(IntGetProcAddress("glGetSamplerParameterIuiv"));
		if(!GetSamplerParameterIuiv) ++numFailed;
		GetSamplerParameterfv = reinterpret_cast<PFNGETSAMPLERPARAMETERFV>(IntGetProcAddress("glGetSamplerParameterfv"));
		if(!GetSamplerParameterfv) ++numFailed;
		GetSamplerParameteriv = reinterpret_cast<PFNGETSAMPLERPARAMETERIV>(IntGetProcAddress("glGetSamplerParameteriv"));
		if(!GetSamplerParameteriv) ++numFailed;
		IsSampler = reinterpret_cast<PFNISSAMPLER>(IntGetProcAddress("glIsSampler"));
		if(!IsSampler) ++numFailed;
		QueryCounter = reinterpret_cast<PFNQUERYCOUNTER>(IntGetProcAddress("glQueryCounter"));
		if(!QueryCounter) ++numFailed;
		SamplerParameterIiv = reinterpret_cast<PFNSAMPLERPARAMETERIIV>(IntGetProcAddress("glSamplerParameterIiv"));
		if(!SamplerParameterIiv) ++numFailed;
		SamplerParameterIuiv = reinterpret_cast<PFNSAMPLERPARAMETERIUIV>(IntGetProcAddress("glSamplerParameterIuiv"));
		if(!SamplerParameterIuiv) ++numFailed;
		SamplerParameterf = reinterpret_cast<PFNSAMPLERPARAMETERF>(IntGetProcAddress("glSamplerParameterf"));
		if(!SamplerParameterf) ++numFailed;
		SamplerParameterfv = reinterpret_cast<PFNSAMPLERPARAMETERFV>(IntGetProcAddress("glSamplerParameterfv"));
		if(!SamplerParameterfv) ++numFailed;
		SamplerParameteri = reinterpret_cast<PFNSAMPLERPARAMETERI>(IntGetProcAddress("glSamplerParameteri"));
		if(!SamplerParameteri) ++numFailed;
		SamplerParameteriv = reinterpret_cast<PFNSAMPLERPARAMETERIV>(IntGetProcAddress("glSamplerParameteriv"));
		if(!SamplerParameteriv) ++numFailed;
		VertexAttribDivisor = reinterpret_cast<PFNVERTEXATTRIBDIVISOR>(IntGetProcAddress("glVertexAttribDivisor"));
		if(!VertexAttribDivisor) ++numFailed;
		VertexAttribP1ui = reinterpret_cast<PFNVERTEXATTRIBP1UI>(IntGetProcAddress("glVertexAttribP1ui"));
		if(!VertexAttribP1ui) ++numFailed;
		VertexAttribP1uiv = reinterpret_cast<PFNVERTEXATTRIBP1UIV>(IntGetProcAddress("glVertexAttribP1uiv"));
		if(!VertexAttribP1uiv) ++numFailed;
		VertexAttribP2ui = reinterpret_cast<PFNVERTEXATTRIBP2UI>(IntGetProcAddress("glVertexAttribP2ui"));
		if(!VertexAttribP2ui) ++numFailed;
		VertexAttribP2uiv = reinterpret_cast<PFNVERTEXATTRIBP2UIV>(IntGetProcAddress("glVertexAttribP2uiv"));
		if(!VertexAttribP2uiv) ++numFailed;
		VertexAttribP3ui = reinterpret_cast<PFNVERTEXATTRIBP3UI>(IntGetProcAddress("glVertexAttribP3ui"));
		if(!VertexAttribP3ui) ++numFailed;
		VertexAttribP3uiv = reinterpret_cast<PFNVERTEXATTRIBP3UIV>(IntGetProcAddress("glVertexAttribP3uiv"));
		if(!VertexAttribP3uiv) ++numFailed;
		VertexAttribP4ui = reinterpret_cast<PFNVERTEXATTRIBP4UI>(IntGetProcAddress("glVertexAttribP4ui"));
		if(!VertexAttribP4ui) ++numFailed;
		VertexAttribP4uiv = reinterpret_cast<PFNVERTEXATTRIBP4UIV>(IntGetProcAddress("glVertexAttribP4uiv"));
		if(!VertexAttribP4uiv) ++numFailed;
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
				gl::GetIntegerv(gl::NUM_EXTENSIONS, &iNumExtensions);
			
				for(iLoop = 0; iLoop < iNumExtensions; iLoop++)
				{
					const char *strExtensionName = (const char *)gl::GetStringi(gl::EXTENSIONS, iLoop);
					LoadExtByName(table, strExtensionName);
				}
			}
			
		} //namespace 
		
		exts::LoadTest LoadFunctions()
		{
			ClearExtensionVars();
			std::vector<MapEntry> table;
			InitializeMappingTable(table);
			
			GetIntegerv = reinterpret_cast<PFNGETINTEGERV>(IntGetProcAddress("glGetIntegerv"));
			if(!GetIntegerv) return exts::LoadTest();
			GetStringi = reinterpret_cast<PFNGETSTRINGI>(IntGetProcAddress("glGetStringi"));
			if(!GetStringi) return exts::LoadTest();
			
			ProcExtsFromExtList(table);
			
			int numFailed = LoadCoreFunctions();
			return exts::LoadTest(true, numFailed);
		}
		
		static int g_major_version = 0;
		static int g_minor_version = 0;
		
		static void GetGLVersion()
		{
			GetIntegerv(MAJOR_VERSION, &g_major_version);
			GetIntegerv(MINOR_VERSION, &g_minor_version);
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
