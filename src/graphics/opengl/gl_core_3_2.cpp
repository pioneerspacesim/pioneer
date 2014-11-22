#include <algorithm>
#include <vector>
#include <string.h>
#include <stddef.h>
#include "gl_core_3_2.hpp"

#if defined(__APPLE__)
#include <mach-o/dyld.h>

static void* AppleGLGetProcAddress (const GLubyte *name)
{
  static const struct mach_header* image = NULL;
  NSSymbol symbol;
  char* symbolName;
  if (NULL == image)
  {
    image = NSAddImage("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", NSADDIMAGE_OPTION_RETURN_ON_ERROR);
  }
  /* prepend a '_' for the Unix C symbol mangling convention */
  symbolName = malloc(strlen((const char*)name) + 2);
  strcpy(symbolName+1, (const char*)name);
  symbolName[0] = '_';
  symbol = NULL;
  /* if (NSIsSymbolNameDefined(symbolName))
	 symbol = NSLookupAndBindSymbol(symbolName); */
  symbol = image ? NSLookupSymbolInImage(image, symbolName, NSLOOKUPSYMBOLINIMAGE_OPTION_BIND | NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR) : NULL;
  free(symbolName);
  return symbol ? NSAddressOfSymbol(symbol) : NULL;
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

namespace gl
{
	namespace exts
	{
		LoadTest var_EXT_texture_compression_s3tc;
		LoadTest var_EXT_texture_sRGB;
		LoadTest var_EXT_texture_filter_anisotropic;
		
	} //namespace exts
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
	typedef void (CODEGEN_FUNCPTR *PFNFINISH)();
	PFNFINISH Finish = 0;
	typedef void (CODEGEN_FUNCPTR *PFNFLUSH)();
	PFNFLUSH Flush = 0;
	typedef void (CODEGEN_FUNCPTR *PFNFRONTFACE)(GLenum);
	PFNFRONTFACE FrontFace = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETBOOLEANV)(GLenum, GLboolean *);
	PFNGETBOOLEANV GetBooleanv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETDOUBLEV)(GLenum, GLdouble *);
	PFNGETDOUBLEV GetDoublev = 0;
	typedef GLenum (CODEGEN_FUNCPTR *PFNGETERROR)();
	PFNGETERROR GetError = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETFLOATV)(GLenum, GLfloat *);
	PFNGETFLOATV GetFloatv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETINTEGERV)(GLenum, GLint *);
	PFNGETINTEGERV GetIntegerv = 0;
	typedef const GLubyte * (CODEGEN_FUNCPTR *PFNGETSTRING)(GLenum);
	PFNGETSTRING GetString = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETTEXIMAGE)(GLenum, GLint, GLenum, GLenum, GLvoid *);
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
	typedef void (CODEGEN_FUNCPTR *PFNREADPIXELS)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid *);
	PFNREADPIXELS ReadPixels = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSCISSOR)(GLint, GLint, GLsizei, GLsizei);
	PFNSCISSOR Scissor = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSTENCILFUNC)(GLenum, GLint, GLuint);
	PFNSTENCILFUNC StencilFunc = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSTENCILMASK)(GLuint);
	PFNSTENCILMASK StencilMask = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSTENCILOP)(GLenum, GLenum, GLenum);
	PFNSTENCILOP StencilOp = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXIMAGE1D)(GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
	PFNTEXIMAGE1D TexImage1D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXIMAGE2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
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
	typedef void (CODEGEN_FUNCPTR *PFNDRAWELEMENTS)(GLenum, GLsizei, GLenum, const GLvoid *);
	PFNDRAWELEMENTS DrawElements = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGENTEXTURES)(GLsizei, GLuint *);
	PFNGENTEXTURES GenTextures = 0;
	typedef GLboolean (CODEGEN_FUNCPTR *PFNISTEXTURE)(GLuint);
	PFNISTEXTURE IsTexture = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPOLYGONOFFSET)(GLfloat, GLfloat);
	PFNPOLYGONOFFSET PolygonOffset = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXSUBIMAGE1D)(GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const GLvoid *);
	PFNTEXSUBIMAGE1D TexSubImage1D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXSUBIMAGE2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
	PFNTEXSUBIMAGE2D TexSubImage2D = 0;
	
	typedef void (CODEGEN_FUNCPTR *PFNBLENDCOLOR)(GLfloat, GLfloat, GLfloat, GLfloat);
	PFNBLENDCOLOR BlendColor = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBLENDEQUATION)(GLenum);
	PFNBLENDEQUATION BlendEquation = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOPYTEXSUBIMAGE3D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
	PFNCOPYTEXSUBIMAGE3D CopyTexSubImage3D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDRAWRANGEELEMENTS)(GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *);
	PFNDRAWRANGEELEMENTS DrawRangeElements = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXIMAGE3D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
	PFNTEXIMAGE3D TexImage3D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXSUBIMAGE3D)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
	PFNTEXSUBIMAGE3D TexSubImage3D = 0;
	
	typedef void (CODEGEN_FUNCPTR *PFNACTIVETEXTURE)(GLenum);
	PFNACTIVETEXTURE ActiveTexture = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOMPRESSEDTEXIMAGE1D)(GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *);
	PFNCOMPRESSEDTEXIMAGE1D CompressedTexImage1D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOMPRESSEDTEXIMAGE2D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
	PFNCOMPRESSEDTEXIMAGE2D CompressedTexImage2D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOMPRESSEDTEXIMAGE3D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
	PFNCOMPRESSEDTEXIMAGE3D CompressedTexImage3D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOMPRESSEDTEXSUBIMAGE1D)(GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *);
	PFNCOMPRESSEDTEXSUBIMAGE1D CompressedTexSubImage1D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOMPRESSEDTEXSUBIMAGE2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
	PFNCOMPRESSEDTEXSUBIMAGE2D CompressedTexSubImage2D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNCOMPRESSEDTEXSUBIMAGE3D)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
	PFNCOMPRESSEDTEXSUBIMAGE3D CompressedTexSubImage3D = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETCOMPRESSEDTEXIMAGE)(GLenum, GLint, GLvoid *);
	PFNGETCOMPRESSEDTEXIMAGE GetCompressedTexImage = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSAMPLECOVERAGE)(GLfloat, GLboolean);
	PFNSAMPLECOVERAGE SampleCoverage = 0;
	
	typedef void (CODEGEN_FUNCPTR *PFNBLENDFUNCSEPARATE)(GLenum, GLenum, GLenum, GLenum);
	PFNBLENDFUNCSEPARATE BlendFuncSeparate = 0;
	typedef void (CODEGEN_FUNCPTR *PFNMULTIDRAWARRAYS)(GLenum, const GLint *, const GLsizei *, GLsizei);
	PFNMULTIDRAWARRAYS MultiDrawArrays = 0;
	typedef void (CODEGEN_FUNCPTR *PFNMULTIDRAWELEMENTS)(GLenum, const GLsizei *, GLenum, const GLvoid *const*, GLsizei);
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
	typedef void (CODEGEN_FUNCPTR *PFNBUFFERDATA)(GLenum, GLsizeiptr, const GLvoid *, GLenum);
	PFNBUFFERDATA BufferData = 0;
	typedef void (CODEGEN_FUNCPTR *PFNBUFFERSUBDATA)(GLenum, GLintptr, GLsizeiptr, const GLvoid *);
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
	typedef void (CODEGEN_FUNCPTR *PFNGETBUFFERPOINTERV)(GLenum, GLenum, GLvoid **);
	PFNGETBUFFERPOINTERV GetBufferPointerv = 0;
	typedef void (CODEGEN_FUNCPTR *PFNGETBUFFERSUBDATA)(GLenum, GLintptr, GLsizeiptr, GLvoid *);
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
	typedef GLuint (CODEGEN_FUNCPTR *PFNCREATEPROGRAM)();
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
	typedef void (CODEGEN_FUNCPTR *PFNGETVERTEXATTRIBPOINTERV)(GLuint, GLenum, GLvoid **);
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
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBPOINTER)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
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
	typedef void (CODEGEN_FUNCPTR *PFNENDCONDITIONALRENDER)();
	PFNENDCONDITIONALRENDER EndConditionalRender = 0;
	typedef void (CODEGEN_FUNCPTR *PFNENDTRANSFORMFEEDBACK)();
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
	typedef void (CODEGEN_FUNCPTR *PFNVERTEXATTRIBIPOINTER)(GLuint, GLint, GLenum, GLsizei, const GLvoid *);
	PFNVERTEXATTRIBIPOINTER VertexAttribIPointer = 0;
	
	typedef void (CODEGEN_FUNCPTR *PFNCOPYBUFFERSUBDATA)(GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr);
	PFNCOPYBUFFERSUBDATA CopyBufferSubData = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDRAWARRAYSINSTANCED)(GLenum, GLint, GLsizei, GLsizei);
	PFNDRAWARRAYSINSTANCED DrawArraysInstanced = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDRAWELEMENTSINSTANCED)(GLenum, GLsizei, GLenum, const GLvoid *, GLsizei);
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
	typedef void (CODEGEN_FUNCPTR *PFNDRAWELEMENTSBASEVERTEX)(GLenum, GLsizei, GLenum, const GLvoid *, GLint);
	PFNDRAWELEMENTSBASEVERTEX DrawElementsBaseVertex = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDRAWELEMENTSINSTANCEDBASEVERTEX)(GLenum, GLsizei, GLenum, const GLvoid *, GLsizei, GLint);
	PFNDRAWELEMENTSINSTANCEDBASEVERTEX DrawElementsInstancedBaseVertex = 0;
	typedef void (CODEGEN_FUNCPTR *PFNDRAWRANGEELEMENTSBASEVERTEX)(GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *, GLint);
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
	typedef void (CODEGEN_FUNCPTR *PFNMULTIDRAWELEMENTSBASEVERTEX)(GLenum, const GLsizei *, GLenum, const GLvoid *const*, GLsizei, const GLint *);
	PFNMULTIDRAWELEMENTSBASEVERTEX MultiDrawElementsBaseVertex = 0;
	typedef void (CODEGEN_FUNCPTR *PFNPROVOKINGVERTEX)(GLenum);
	PFNPROVOKINGVERTEX ProvokingVertex = 0;
	typedef void (CODEGEN_FUNCPTR *PFNSAMPLEMASKI)(GLuint, GLbitfield);
	PFNSAMPLEMASKI SampleMaski = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXIMAGE2DMULTISAMPLE)(GLenum, GLsizei, GLint, GLsizei, GLsizei, GLboolean);
	PFNTEXIMAGE2DMULTISAMPLE TexImage2DMultisample = 0;
	typedef void (CODEGEN_FUNCPTR *PFNTEXIMAGE3DMULTISAMPLE)(GLenum, GLsizei, GLint, GLsizei, GLsizei, GLsizei, GLboolean);
	PFNTEXIMAGE3DMULTISAMPLE TexImage3DMultisample = 0;
	typedef void (CODEGEN_FUNCPTR *PFNWAITSYNC)(GLsync, GLbitfield, GLuint64);
	PFNWAITSYNC WaitSync = 0;
	
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
		BlendColor = reinterpret_cast<PFNBLENDCOLOR>(IntGetProcAddress("glBlendColor"));
		if(!BlendColor) ++numFailed;
		BlendEquation = reinterpret_cast<PFNBLENDEQUATION>(IntGetProcAddress("glBlendEquation"));
		if(!BlendEquation) ++numFailed;
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
				table.reserve(3);
				table.push_back(MapEntry("GL_EXT_texture_compression_s3tc", &exts::var_EXT_texture_compression_s3tc));
				table.push_back(MapEntry("GL_EXT_texture_sRGB", &exts::var_EXT_texture_sRGB));
				table.push_back(MapEntry("GL_EXT_texture_filter_anisotropic", &exts::var_EXT_texture_filter_anisotropic));
			}
			
			void ClearExtensionVars()
			{
				exts::var_EXT_texture_compression_s3tc = exts::LoadTest();
				exts::var_EXT_texture_sRGB = exts::LoadTest();
				exts::var_EXT_texture_filter_anisotropic = exts::LoadTest();
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
				
			if(majorVersion > g_major_version) return true;
			if(majorVersion < g_major_version) return false;
			if(minorVersion >= g_minor_version) return true;
			return false;
		}
		
	} //namespace sys
} //namespace gl
