#ifndef GL21_PIONEER_GENERATED_HEADEROPENGL_HPP
#define GL21_PIONEER_GENERATED_HEADEROPENGL_HPP

#if defined(__glew_h__) || defined(__GLEW_H__)
#error Attempt to include auto-generated header after including glew.h
#endif
#if defined(__gl_h_) || defined(__GL_H__)
#error Attempt to include auto-generated header after including gl.h
#endif
#if defined(__glext_h_) || defined(__GLEXT_H_)
#error Attempt to include auto-generated header after including glext.h
#endif
#if defined(__gltypes_h_)
#error Attempt to include auto-generated header after gltypes.h
#endif
#if defined(__gl_ATI_h_)
#error Attempt to include auto-generated header after including glATI.h
#endif

#define __glew_h__
#define __GLEW_H__
#define __gl_h_
#define __GL_H__
#define __glext_h_
#define __GLEXT_H_
#define __gltypes_h_
#define __gl_ATI_h_

#ifndef APIENTRY
	#if defined(__MINGW32__)
		#ifndef WIN32_LEAN_AND_MEAN
			#define WIN32_LEAN_AND_MEAN 1
		#endif
		#ifndef NOMINMAX
			#define NOMINMAX
		#endif
		#include <windows.h>
	#elif (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED) || defined(__BORLANDC__)
		#ifndef WIN32_LEAN_AND_MEAN
			#define WIN32_LEAN_AND_MEAN 1
		#endif
		#ifndef NOMINMAX
			#define NOMINMAX
		#endif
		#include <windows.h>
	#else
		#define APIENTRY
	#endif
#endif /*APIENTRY*/

#ifndef CODEGEN_FUNCPTR
	#define CODEGEN_REMOVE_FUNCPTR
	#if defined(_WIN32)
		#define CODEGEN_FUNCPTR APIENTRY
	#else
		#define CODEGEN_FUNCPTR
	#endif
#endif /*CODEGEN_FUNCPTR*/

#ifndef GLAPI
	#define GLAPI extern
#endif


#ifndef GL_LOAD_GEN_BASIC_OPENGL_TYPEDEFS
#define GL_LOAD_GEN_BASIC_OPENGL_TYPEDEFS


#endif /*GL_LOAD_GEN_BASIC_OPENGL_TYPEDEFS*/

#include <stddef.h>
#ifndef GLEXT_64_TYPES_DEFINED
/* This code block is duplicated in glxext.h, so must be protected */
#define GLEXT_64_TYPES_DEFINED
/* Define int32_t, int64_t, and uint64_t types for UST/MSC */
/* (as used in the GL_EXT_timer_query extension). */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <inttypes.h>
#elif defined(__sun__) || defined(__digital__)
#include <inttypes.h>
#if defined(__STDC__)
#if defined(__arch64__) || defined(_LP64)
typedef long int int64_t;
typedef unsigned long int uint64_t;
#else
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
#endif /* __arch64__ */
#endif /* __STDC__ */
#elif defined( __VMS ) || defined(__sgi)
#include <inttypes.h>
#elif defined(__SCO__) || defined(__USLC__)
#include <stdint.h>
#elif defined(__UNIXOS2__) || defined(__SOL64__)
typedef long int int32_t;
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
#elif defined(_WIN32) && defined(__GNUC__)
#include <stdint.h>
#elif defined(_WIN32)
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
/* Fallback if nothing above works */
#include <inttypes.h>
#endif
#endif
	typedef unsigned int GLenum;
	typedef unsigned char GLboolean;
	typedef unsigned int GLbitfield;
	typedef void GLvoid;
	typedef signed char GLbyte;
	typedef short GLshort;
	typedef int GLint;
	typedef unsigned char GLubyte;
	typedef unsigned short GLushort;
	typedef unsigned int GLuint;
	typedef int GLsizei;
	typedef float GLfloat;
	typedef float GLclampf;
	typedef double GLdouble;
	typedef double GLclampd;
	typedef char GLchar;
	typedef char GLcharARB;
	#ifdef __APPLE__
typedef void *GLhandleARB;
#else
typedef unsigned int GLhandleARB;
#endif
		typedef unsigned short GLhalfARB;
		typedef unsigned short GLhalf;
		typedef GLint GLfixed;
		typedef ptrdiff_t GLintptr;
		typedef ptrdiff_t GLsizeiptr;
		typedef int64_t GLint64;
		typedef uint64_t GLuint64;
		typedef ptrdiff_t GLintptrARB;
		typedef ptrdiff_t GLsizeiptrARB;
		typedef int64_t GLint64EXT;
		typedef uint64_t GLuint64EXT;
		typedef struct __GLsync *GLsync;
		struct _cl_context;
		struct _cl_event;
		typedef void (APIENTRY *GLDEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
		typedef void (APIENTRY *GLDEBUGPROCARB)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
		typedef void (APIENTRY *GLDEBUGPROCAMD)(GLuint id,GLenum category,GLenum severity,GLsizei length,const GLchar *message,void *userParam);
		typedef unsigned short GLhalfNV;
		typedef GLintptr GLvdpauSurfaceNV;

namespace gl21
{
	namespace gl
	{
		namespace exts
		{
			class LoadTest
			{
			private:
				//Safe bool idiom. Joy!
				typedef void (LoadTest::*bool_type)() const;
				void big_long_name_that_really_doesnt_matter() const {}
				
			public:
				operator bool_type() const
				{
					return m_isLoaded ? &LoadTest::big_long_name_that_really_doesnt_matter : 0;
				}
				
				int GetNumMissing() const {return m_numMissing;}
				
				LoadTest() : m_isLoaded(false), m_numMissing(0) {}
				LoadTest(bool isLoaded, int numMissing) : m_isLoaded(isLoaded), m_numMissing(numMissing) {}
			private:
				bool m_isLoaded;
				int m_numMissing;
			};
			
			extern LoadTest var_ARB_seamless_cube_map;
			extern LoadTest var_ARB_seamless_cubemap_per_texture;
			extern LoadTest var_ARB_draw_instanced;
			extern LoadTest var_ARB_uniform_buffer_object;
			extern LoadTest var_ARB_instanced_arrays;
			extern LoadTest var_ARB_vertex_array_object;
			extern LoadTest var_EXT_framebuffer_object;
			extern LoadTest var_EXT_texture_compression_s3tc;
			extern LoadTest var_EXT_texture_sRGB;
			extern LoadTest var_EXT_texture_filter_anisotropic;
			
		} //namespace exts
		enum
		{
			GL_TEXTURE_CUBE_MAP_SEAMLESS     = 0x884F,
			
			//TEXTURE_CUBE_MAP_SEAMLESS taken from ext: ARB_seamless_cube_map
			
			GL_ACTIVE_UNIFORM_BLOCKS         = 0x8A36,
			GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH = 0x8A35,
			GL_INVALID_INDEX                 = 0xFFFFFFFF,
			GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS = 0x8A33,
			GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS = 0x8A32,
			GL_MAX_COMBINED_UNIFORM_BLOCKS   = 0x8A2E,
			GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS = 0x8A31,
			GL_MAX_FRAGMENT_UNIFORM_BLOCKS   = 0x8A2D,
			GL_MAX_GEOMETRY_UNIFORM_BLOCKS   = 0x8A2C,
			GL_MAX_UNIFORM_BLOCK_SIZE        = 0x8A30,
			GL_MAX_UNIFORM_BUFFER_BINDINGS   = 0x8A2F,
			GL_MAX_VERTEX_UNIFORM_BLOCKS     = 0x8A2B,
			GL_UNIFORM_ARRAY_STRIDE          = 0x8A3C,
			GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS = 0x8A42,
			GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES = 0x8A43,
			GL_UNIFORM_BLOCK_BINDING         = 0x8A3F,
			GL_UNIFORM_BLOCK_DATA_SIZE       = 0x8A40,
			GL_UNIFORM_BLOCK_INDEX           = 0x8A3A,
			GL_UNIFORM_BLOCK_NAME_LENGTH     = 0x8A41,
			GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER = 0x8A46,
			GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER = 0x8A45,
			GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER = 0x8A44,
			GL_UNIFORM_BUFFER                = 0x8A11,
			GL_UNIFORM_BUFFER_BINDING        = 0x8A28,
			GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT = 0x8A34,
			GL_UNIFORM_BUFFER_SIZE           = 0x8A2A,
			GL_UNIFORM_BUFFER_START          = 0x8A29,
			GL_UNIFORM_IS_ROW_MAJOR          = 0x8A3E,
			GL_UNIFORM_MATRIX_STRIDE         = 0x8A3D,
			GL_UNIFORM_NAME_LENGTH           = 0x8A39,
			GL_UNIFORM_OFFSET                = 0x8A3B,
			GL_UNIFORM_SIZE                  = 0x8A38,
			GL_UNIFORM_TYPE                  = 0x8A37,
			
			GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ARB = 0x88FE,
			
			GL_VERTEX_ARRAY_BINDING          = 0x85B5,
			
			GL_COLOR_ATTACHMENT0_EXT         = 0x8CE0,
			GL_COLOR_ATTACHMENT10_EXT        = 0x8CEA,
			GL_COLOR_ATTACHMENT11_EXT        = 0x8CEB,
			GL_COLOR_ATTACHMENT12_EXT        = 0x8CEC,
			GL_COLOR_ATTACHMENT13_EXT        = 0x8CED,
			GL_COLOR_ATTACHMENT14_EXT        = 0x8CEE,
			GL_COLOR_ATTACHMENT15_EXT        = 0x8CEF,
			GL_COLOR_ATTACHMENT1_EXT         = 0x8CE1,
			GL_COLOR_ATTACHMENT2_EXT         = 0x8CE2,
			GL_COLOR_ATTACHMENT3_EXT         = 0x8CE3,
			GL_COLOR_ATTACHMENT4_EXT         = 0x8CE4,
			GL_COLOR_ATTACHMENT5_EXT         = 0x8CE5,
			GL_COLOR_ATTACHMENT6_EXT         = 0x8CE6,
			GL_COLOR_ATTACHMENT7_EXT         = 0x8CE7,
			GL_COLOR_ATTACHMENT8_EXT         = 0x8CE8,
			GL_COLOR_ATTACHMENT9_EXT         = 0x8CE9,
			GL_DEPTH_ATTACHMENT_EXT          = 0x8D00,
			GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT = 0x8CD1,
			GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT = 0x8CD0,
			GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT = 0x8CD4,
			GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT = 0x8CD3,
			GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT = 0x8CD2,
			GL_FRAMEBUFFER_BINDING_EXT       = 0x8CA6,
			GL_FRAMEBUFFER_COMPLETE_EXT      = 0x8CD5,
			GL_FRAMEBUFFER_EXT               = 0x8D40,
			GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT = 0x8CD6,
			GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT = 0x8CD9,
			GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT = 0x8CDB,
			GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT = 0x8CDA,
			GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT = 0x8CD7,
			GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT = 0x8CDC,
			GL_FRAMEBUFFER_UNSUPPORTED_EXT   = 0x8CDD,
			GL_INVALID_FRAMEBUFFER_OPERATION_EXT = 0x0506,
			GL_MAX_COLOR_ATTACHMENTS_EXT     = 0x8CDF,
			GL_MAX_RENDERBUFFER_SIZE_EXT     = 0x84E8,
			GL_RENDERBUFFER_ALPHA_SIZE_EXT   = 0x8D53,
			GL_RENDERBUFFER_BINDING_EXT      = 0x8CA7,
			GL_RENDERBUFFER_BLUE_SIZE_EXT    = 0x8D52,
			GL_RENDERBUFFER_DEPTH_SIZE_EXT   = 0x8D54,
			GL_RENDERBUFFER_EXT              = 0x8D41,
			GL_RENDERBUFFER_GREEN_SIZE_EXT   = 0x8D51,
			GL_RENDERBUFFER_HEIGHT_EXT       = 0x8D43,
			GL_RENDERBUFFER_INTERNAL_FORMAT_EXT = 0x8D44,
			GL_RENDERBUFFER_RED_SIZE_EXT     = 0x8D50,
			GL_RENDERBUFFER_STENCIL_SIZE_EXT = 0x8D55,
			GL_RENDERBUFFER_WIDTH_EXT        = 0x8D42,
			GL_STENCIL_ATTACHMENT_EXT        = 0x8D20,
			GL_STENCIL_INDEX16_EXT           = 0x8D49,
			GL_STENCIL_INDEX1_EXT            = 0x8D46,
			GL_STENCIL_INDEX4_EXT            = 0x8D47,
			GL_STENCIL_INDEX8_EXT            = 0x8D48,
			
			GL_COMPRESSED_RGBA_S3TC_DXT1_EXT = 0x83F1,
			GL_COMPRESSED_RGBA_S3TC_DXT3_EXT = 0x83F2,
			GL_COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3,
			GL_COMPRESSED_RGB_S3TC_DXT1_EXT  = 0x83F0,
			
			GL_COMPRESSED_SLUMINANCE_ALPHA_EXT = 0x8C4B,
			GL_COMPRESSED_SLUMINANCE_EXT     = 0x8C4A,
			GL_COMPRESSED_SRGB_ALPHA_EXT     = 0x8C49,
			GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT = 0x8C4D,
			GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT = 0x8C4E,
			GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT = 0x8C4F,
			GL_COMPRESSED_SRGB_EXT           = 0x8C48,
			GL_COMPRESSED_SRGB_S3TC_DXT1_EXT = 0x8C4C,
			GL_SLUMINANCE8_ALPHA8_EXT        = 0x8C45,
			GL_SLUMINANCE8_EXT               = 0x8C47,
			GL_SLUMINANCE_ALPHA_EXT          = 0x8C44,
			GL_SLUMINANCE_EXT                = 0x8C46,
			GL_SRGB8_ALPHA8_EXT              = 0x8C43,
			GL_SRGB8_EXT                     = 0x8C41,
			GL_SRGB_ALPHA_EXT                = 0x8C42,
			GL_SRGB_EXT                      = 0x8C40,
			
			GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT = 0x84FF,
			GL_TEXTURE_MAX_ANISOTROPY_EXT    = 0x84FE,
			
			GL_2D                            = 0x0600,
			GL_2_BYTES                       = 0x1407,
			GL_3D                            = 0x0601,
			GL_3D_COLOR                      = 0x0602,
			GL_3D_COLOR_TEXTURE              = 0x0603,
			GL_3_BYTES                       = 0x1408,
			GL_4D_COLOR_TEXTURE              = 0x0604,
			GL_4_BYTES                       = 0x1409,
			GL_ACCUM                         = 0x0100,
			GL_ACCUM_ALPHA_BITS              = 0x0D5B,
			GL_ACCUM_BLUE_BITS               = 0x0D5A,
			GL_ACCUM_BUFFER_BIT              = 0x00000200,
			GL_ACCUM_CLEAR_VALUE             = 0x0B80,
			GL_ACCUM_GREEN_BITS              = 0x0D59,
			GL_ACCUM_RED_BITS                = 0x0D58,
			GL_ADD                           = 0x0104,
			GL_ALL_ATTRIB_BITS               = 0xFFFFFFFF,
			GL_ALPHA                         = 0x1906,
			GL_ALPHA12                       = 0x803D,
			GL_ALPHA16                       = 0x803E,
			GL_ALPHA4                        = 0x803B,
			GL_ALPHA8                        = 0x803C,
			GL_ALPHA_BIAS                    = 0x0D1D,
			GL_ALPHA_BITS                    = 0x0D55,
			GL_ALPHA_SCALE                   = 0x0D1C,
			GL_ALPHA_TEST                    = 0x0BC0,
			GL_ALPHA_TEST_FUNC               = 0x0BC1,
			GL_ALPHA_TEST_REF                = 0x0BC2,
			GL_ALWAYS                        = 0x0207,
			GL_AMBIENT                       = 0x1200,
			GL_AMBIENT_AND_DIFFUSE           = 0x1602,
			GL_AND                           = 0x1501,
			GL_AND_INVERTED                  = 0x1504,
			GL_AND_REVERSE                   = 0x1502,
			GL_ATTRIB_STACK_DEPTH            = 0x0BB0,
			GL_AUTO_NORMAL                   = 0x0D80,
			GL_AUX0                          = 0x0409,
			GL_AUX1                          = 0x040A,
			GL_AUX2                          = 0x040B,
			GL_AUX3                          = 0x040C,
			GL_AUX_BUFFERS                   = 0x0C00,
			GL_BACK                          = 0x0405,
			GL_BACK_LEFT                     = 0x0402,
			GL_BACK_RIGHT                    = 0x0403,
			GL_BITMAP                        = 0x1A00,
			GL_BITMAP_TOKEN                  = 0x0704,
			GL_BLEND                         = 0x0BE2,
			GL_BLEND_DST                     = 0x0BE0,
			GL_BLEND_SRC                     = 0x0BE1,
			GL_BLUE                          = 0x1905,
			GL_BLUE_BIAS                     = 0x0D1B,
			GL_BLUE_BITS                     = 0x0D54,
			GL_BLUE_SCALE                    = 0x0D1A,
			GL_BYTE                          = 0x1400,
			GL_C3F_V3F                       = 0x2A24,
			GL_C4F_N3F_V3F                   = 0x2A26,
			GL_C4UB_V2F                      = 0x2A22,
			GL_C4UB_V3F                      = 0x2A23,
			GL_CCW                           = 0x0901,
			GL_CLAMP                         = 0x2900,
			GL_CLEAR                         = 0x1500,
			GL_CLIENT_ALL_ATTRIB_BITS        = 0xFFFFFFFF,
			GL_CLIENT_ATTRIB_STACK_DEPTH     = 0x0BB1,
			GL_CLIENT_PIXEL_STORE_BIT        = 0x00000001,
			GL_CLIENT_VERTEX_ARRAY_BIT       = 0x00000002,
			GL_CLIP_PLANE0                   = 0x3000,
			GL_CLIP_PLANE1                   = 0x3001,
			GL_CLIP_PLANE2                   = 0x3002,
			GL_CLIP_PLANE3                   = 0x3003,
			GL_CLIP_PLANE4                   = 0x3004,
			GL_CLIP_PLANE5                   = 0x3005,
			GL_COEFF                         = 0x0A00,
			GL_COLOR                         = 0x1800,
			GL_COLOR_ARRAY                   = 0x8076,
			GL_COLOR_ARRAY_POINTER           = 0x8090,
			GL_COLOR_ARRAY_SIZE              = 0x8081,
			GL_COLOR_ARRAY_STRIDE            = 0x8083,
			GL_COLOR_ARRAY_TYPE              = 0x8082,
			GL_COLOR_BUFFER_BIT              = 0x00004000,
			GL_COLOR_CLEAR_VALUE             = 0x0C22,
			GL_COLOR_INDEX                   = 0x1900,
			GL_COLOR_INDEXES                 = 0x1603,
			GL_COLOR_LOGIC_OP                = 0x0BF2,
			GL_COLOR_MATERIAL                = 0x0B57,
			GL_COLOR_MATERIAL_FACE           = 0x0B55,
			GL_COLOR_MATERIAL_PARAMETER      = 0x0B56,
			GL_COLOR_WRITEMASK               = 0x0C23,
			GL_COMPILE                       = 0x1300,
			GL_COMPILE_AND_EXECUTE           = 0x1301,
			GL_CONSTANT_ATTENUATION          = 0x1207,
			GL_COPY                          = 0x1503,
			GL_COPY_INVERTED                 = 0x150C,
			GL_COPY_PIXEL_TOKEN              = 0x0706,
			GL_CULL_FACE                     = 0x0B44,
			GL_CULL_FACE_MODE                = 0x0B45,
			GL_CURRENT_BIT                   = 0x00000001,
			GL_CURRENT_COLOR                 = 0x0B00,
			GL_CURRENT_INDEX                 = 0x0B01,
			GL_CURRENT_NORMAL                = 0x0B02,
			GL_CURRENT_RASTER_COLOR          = 0x0B04,
			GL_CURRENT_RASTER_DISTANCE       = 0x0B09,
			GL_CURRENT_RASTER_INDEX          = 0x0B05,
			GL_CURRENT_RASTER_POSITION       = 0x0B07,
			GL_CURRENT_RASTER_POSITION_VALID = 0x0B08,
			GL_CURRENT_RASTER_TEXTURE_COORDS = 0x0B06,
			GL_CURRENT_TEXTURE_COORDS        = 0x0B03,
			GL_CW                            = 0x0900,
			GL_DECAL                         = 0x2101,
			GL_DECR                          = 0x1E03,
			GL_DEPTH                         = 0x1801,
			GL_DEPTH_BIAS                    = 0x0D1F,
			GL_DEPTH_BITS                    = 0x0D56,
			GL_DEPTH_BUFFER_BIT              = 0x00000100,
			GL_DEPTH_CLEAR_VALUE             = 0x0B73,
			GL_DEPTH_COMPONENT               = 0x1902,
			GL_DEPTH_FUNC                    = 0x0B74,
			GL_DEPTH_RANGE                   = 0x0B70,
			GL_DEPTH_SCALE                   = 0x0D1E,
			GL_DEPTH_TEST                    = 0x0B71,
			GL_DEPTH_WRITEMASK               = 0x0B72,
			GL_DIFFUSE                       = 0x1201,
			GL_DITHER                        = 0x0BD0,
			GL_DOMAIN                        = 0x0A02,
			GL_DONT_CARE                     = 0x1100,
			GL_DOUBLE                        = 0x140A,
			GL_DOUBLEBUFFER                  = 0x0C32,
			GL_DRAW_BUFFER                   = 0x0C01,
			GL_DRAW_PIXEL_TOKEN              = 0x0705,
			GL_DST_ALPHA                     = 0x0304,
			GL_DST_COLOR                     = 0x0306,
			GL_EDGE_FLAG                     = 0x0B43,
			GL_EDGE_FLAG_ARRAY               = 0x8079,
			GL_EDGE_FLAG_ARRAY_POINTER       = 0x8093,
			GL_EDGE_FLAG_ARRAY_STRIDE        = 0x808C,
			GL_EMISSION                      = 0x1600,
			GL_ENABLE_BIT                    = 0x00002000,
			GL_EQUAL                         = 0x0202,
			GL_EQUIV                         = 0x1509,
			GL_EVAL_BIT                      = 0x00010000,
			GL_EXP                           = 0x0800,
			GL_EXP2                          = 0x0801,
			GL_EXTENSIONS                    = 0x1F03,
			GL_EYE_LINEAR                    = 0x2400,
			GL_EYE_PLANE                     = 0x2502,
			GL_FALSE                         = 0,
			GL_FASTEST                       = 0x1101,
			GL_FEEDBACK                      = 0x1C01,
			GL_FEEDBACK_BUFFER_POINTER       = 0x0DF0,
			GL_FEEDBACK_BUFFER_SIZE          = 0x0DF1,
			GL_FEEDBACK_BUFFER_TYPE          = 0x0DF2,
			GL_FILL                          = 0x1B02,
			GL_FLAT                          = 0x1D00,
			GL_FLOAT                         = 0x1406,
			GL_FOG                           = 0x0B60,
			GL_FOG_BIT                       = 0x00000080,
			GL_FOG_COLOR                     = 0x0B66,
			GL_FOG_DENSITY                   = 0x0B62,
			GL_FOG_END                       = 0x0B64,
			GL_FOG_HINT                      = 0x0C54,
			GL_FOG_INDEX                     = 0x0B61,
			GL_FOG_MODE                      = 0x0B65,
			GL_FOG_START                     = 0x0B63,
			GL_FRONT                         = 0x0404,
			GL_FRONT_AND_BACK                = 0x0408,
			GL_FRONT_FACE                    = 0x0B46,
			GL_FRONT_LEFT                    = 0x0400,
			GL_FRONT_RIGHT                   = 0x0401,
			GL_GEQUAL                        = 0x0206,
			GL_GREATER                       = 0x0204,
			GL_GREEN                         = 0x1904,
			GL_GREEN_BIAS                    = 0x0D19,
			GL_GREEN_BITS                    = 0x0D53,
			GL_GREEN_SCALE                   = 0x0D18,
			GL_HINT_BIT                      = 0x00008000,
			GL_INCR                          = 0x1E02,
			GL_INDEX_ARRAY                   = 0x8077,
			GL_INDEX_ARRAY_POINTER           = 0x8091,
			GL_INDEX_ARRAY_STRIDE            = 0x8086,
			GL_INDEX_ARRAY_TYPE              = 0x8085,
			GL_INDEX_BITS                    = 0x0D51,
			GL_INDEX_CLEAR_VALUE             = 0x0C20,
			GL_INDEX_LOGIC_OP                = 0x0BF1,
			GL_INDEX_MODE                    = 0x0C30,
			GL_INDEX_OFFSET                  = 0x0D13,
			GL_INDEX_SHIFT                   = 0x0D12,
			GL_INDEX_WRITEMASK               = 0x0C21,
			GL_INT                           = 0x1404,
			GL_INTENSITY                     = 0x8049,
			GL_INTENSITY12                   = 0x804C,
			GL_INTENSITY16                   = 0x804D,
			GL_INTENSITY4                    = 0x804A,
			GL_INTENSITY8                    = 0x804B,
			GL_INVALID_ENUM                  = 0x0500,
			GL_INVALID_OPERATION             = 0x0502,
			GL_INVALID_VALUE                 = 0x0501,
			GL_INVERT                        = 0x150A,
			GL_KEEP                          = 0x1E00,
			GL_LEFT                          = 0x0406,
			GL_LEQUAL                        = 0x0203,
			GL_LESS                          = 0x0201,
			GL_LIGHT0                        = 0x4000,
			GL_LIGHT1                        = 0x4001,
			GL_LIGHT2                        = 0x4002,
			GL_LIGHT3                        = 0x4003,
			GL_LIGHT4                        = 0x4004,
			GL_LIGHT5                        = 0x4005,
			GL_LIGHT6                        = 0x4006,
			GL_LIGHT7                        = 0x4007,
			GL_LIGHTING                      = 0x0B50,
			GL_LIGHTING_BIT                  = 0x00000040,
			GL_LIGHT_MODEL_AMBIENT           = 0x0B53,
			GL_LIGHT_MODEL_LOCAL_VIEWER      = 0x0B51,
			GL_LIGHT_MODEL_TWO_SIDE          = 0x0B52,
			GL_LINE                          = 0x1B01,
			GL_LINEAR                        = 0x2601,
			GL_LINEAR_ATTENUATION            = 0x1208,
			GL_LINEAR_MIPMAP_LINEAR          = 0x2703,
			GL_LINEAR_MIPMAP_NEAREST         = 0x2701,
			GL_LINES                         = 0x0001,
			GL_LINE_BIT                      = 0x00000004,
			GL_LINE_LOOP                     = 0x0002,
			GL_LINE_RESET_TOKEN              = 0x0707,
			GL_LINE_SMOOTH                   = 0x0B20,
			GL_LINE_SMOOTH_HINT              = 0x0C52,
			GL_LINE_STIPPLE                  = 0x0B24,
			GL_LINE_STIPPLE_PATTERN          = 0x0B25,
			GL_LINE_STIPPLE_REPEAT           = 0x0B26,
			GL_LINE_STRIP                    = 0x0003,
			GL_LINE_TOKEN                    = 0x0702,
			GL_LINE_WIDTH                    = 0x0B21,
			GL_LINE_WIDTH_GRANULARITY        = 0x0B23,
			GL_LINE_WIDTH_RANGE              = 0x0B22,
			GL_LIST_BASE                     = 0x0B32,
			GL_LIST_BIT                      = 0x00020000,
			GL_LIST_INDEX                    = 0x0B33,
			GL_LIST_MODE                     = 0x0B30,
			GL_LOAD                          = 0x0101,
			GL_LOGIC_OP                      = 0x0BF1,
			GL_LOGIC_OP_MODE                 = 0x0BF0,
			GL_LUMINANCE                     = 0x1909,
			GL_LUMINANCE12                   = 0x8041,
			GL_LUMINANCE12_ALPHA12           = 0x8047,
			GL_LUMINANCE12_ALPHA4            = 0x8046,
			GL_LUMINANCE16                   = 0x8042,
			GL_LUMINANCE16_ALPHA16           = 0x8048,
			GL_LUMINANCE4                    = 0x803F,
			GL_LUMINANCE4_ALPHA4             = 0x8043,
			GL_LUMINANCE6_ALPHA2             = 0x8044,
			GL_LUMINANCE8                    = 0x8040,
			GL_LUMINANCE8_ALPHA8             = 0x8045,
			GL_LUMINANCE_ALPHA               = 0x190A,
			GL_MAP1_COLOR_4                  = 0x0D90,
			GL_MAP1_GRID_DOMAIN              = 0x0DD0,
			GL_MAP1_GRID_SEGMENTS            = 0x0DD1,
			GL_MAP1_INDEX                    = 0x0D91,
			GL_MAP1_NORMAL                   = 0x0D92,
			GL_MAP1_TEXTURE_COORD_1          = 0x0D93,
			GL_MAP1_TEXTURE_COORD_2          = 0x0D94,
			GL_MAP1_TEXTURE_COORD_3          = 0x0D95,
			GL_MAP1_TEXTURE_COORD_4          = 0x0D96,
			GL_MAP1_VERTEX_3                 = 0x0D97,
			GL_MAP1_VERTEX_4                 = 0x0D98,
			GL_MAP2_COLOR_4                  = 0x0DB0,
			GL_MAP2_GRID_DOMAIN              = 0x0DD2,
			GL_MAP2_GRID_SEGMENTS            = 0x0DD3,
			GL_MAP2_INDEX                    = 0x0DB1,
			GL_MAP2_NORMAL                   = 0x0DB2,
			GL_MAP2_TEXTURE_COORD_1          = 0x0DB3,
			GL_MAP2_TEXTURE_COORD_2          = 0x0DB4,
			GL_MAP2_TEXTURE_COORD_3          = 0x0DB5,
			GL_MAP2_TEXTURE_COORD_4          = 0x0DB6,
			GL_MAP2_VERTEX_3                 = 0x0DB7,
			GL_MAP2_VERTEX_4                 = 0x0DB8,
			GL_MAP_COLOR                     = 0x0D10,
			GL_MAP_STENCIL                   = 0x0D11,
			GL_MATRIX_MODE                   = 0x0BA0,
			GL_MAX_ATTRIB_STACK_DEPTH        = 0x0D35,
			GL_MAX_CLIENT_ATTRIB_STACK_DEPTH = 0x0D3B,
			GL_MAX_CLIP_PLANES               = 0x0D32,
			GL_MAX_EVAL_ORDER                = 0x0D30,
			GL_MAX_LIGHTS                    = 0x0D31,
			GL_MAX_LIST_NESTING              = 0x0B31,
			GL_MAX_MODELVIEW_STACK_DEPTH     = 0x0D36,
			GL_MAX_NAME_STACK_DEPTH          = 0x0D37,
			GL_MAX_PIXEL_MAP_TABLE           = 0x0D34,
			GL_MAX_PROJECTION_STACK_DEPTH    = 0x0D38,
			GL_MAX_TEXTURE_SIZE              = 0x0D33,
			GL_MAX_TEXTURE_STACK_DEPTH       = 0x0D39,
			GL_MAX_VIEWPORT_DIMS             = 0x0D3A,
			GL_MODELVIEW                     = 0x1700,
			GL_MODELVIEW_MATRIX              = 0x0BA6,
			GL_MODELVIEW_STACK_DEPTH         = 0x0BA3,
			GL_MODULATE                      = 0x2100,
			GL_MULT                          = 0x0103,
			GL_N3F_V3F                       = 0x2A25,
			GL_NAME_STACK_DEPTH              = 0x0D70,
			GL_NAND                          = 0x150E,
			GL_NEAREST                       = 0x2600,
			GL_NEAREST_MIPMAP_LINEAR         = 0x2702,
			GL_NEAREST_MIPMAP_NEAREST        = 0x2700,
			GL_NEVER                         = 0x0200,
			GL_NICEST                        = 0x1102,
			GL_NONE                          = 0,
			GL_NOOP                          = 0x1505,
			GL_NOR                           = 0x1508,
			GL_NORMALIZE                     = 0x0BA1,
			GL_NORMAL_ARRAY                  = 0x8075,
			GL_NORMAL_ARRAY_POINTER          = 0x808F,
			GL_NORMAL_ARRAY_STRIDE           = 0x807F,
			GL_NORMAL_ARRAY_TYPE             = 0x807E,
			GL_NOTEQUAL                      = 0x0205,
			GL_NO_ERROR                      = 0,
			GL_OBJECT_LINEAR                 = 0x2401,
			GL_OBJECT_PLANE                  = 0x2501,
			GL_ONE                           = 1,
			GL_ONE_MINUS_DST_ALPHA           = 0x0305,
			GL_ONE_MINUS_DST_COLOR           = 0x0307,
			GL_ONE_MINUS_SRC_ALPHA           = 0x0303,
			GL_ONE_MINUS_SRC_COLOR           = 0x0301,
			GL_OR                            = 0x1507,
			GL_ORDER                         = 0x0A01,
			GL_OR_INVERTED                   = 0x150D,
			GL_OR_REVERSE                    = 0x150B,
			GL_OUT_OF_MEMORY                 = 0x0505,
			GL_PACK_ALIGNMENT                = 0x0D05,
			GL_PACK_LSB_FIRST                = 0x0D01,
			GL_PACK_ROW_LENGTH               = 0x0D02,
			GL_PACK_SKIP_PIXELS              = 0x0D04,
			GL_PACK_SKIP_ROWS                = 0x0D03,
			GL_PACK_SWAP_BYTES               = 0x0D00,
			GL_PASS_THROUGH_TOKEN            = 0x0700,
			GL_PERSPECTIVE_CORRECTION_HINT   = 0x0C50,
			GL_PIXEL_MAP_A_TO_A              = 0x0C79,
			GL_PIXEL_MAP_A_TO_A_SIZE         = 0x0CB9,
			GL_PIXEL_MAP_B_TO_B              = 0x0C78,
			GL_PIXEL_MAP_B_TO_B_SIZE         = 0x0CB8,
			GL_PIXEL_MAP_G_TO_G              = 0x0C77,
			GL_PIXEL_MAP_G_TO_G_SIZE         = 0x0CB7,
			GL_PIXEL_MAP_I_TO_A              = 0x0C75,
			GL_PIXEL_MAP_I_TO_A_SIZE         = 0x0CB5,
			GL_PIXEL_MAP_I_TO_B              = 0x0C74,
			GL_PIXEL_MAP_I_TO_B_SIZE         = 0x0CB4,
			GL_PIXEL_MAP_I_TO_G              = 0x0C73,
			GL_PIXEL_MAP_I_TO_G_SIZE         = 0x0CB3,
			GL_PIXEL_MAP_I_TO_I              = 0x0C70,
			GL_PIXEL_MAP_I_TO_I_SIZE         = 0x0CB0,
			GL_PIXEL_MAP_I_TO_R              = 0x0C72,
			GL_PIXEL_MAP_I_TO_R_SIZE         = 0x0CB2,
			GL_PIXEL_MAP_R_TO_R              = 0x0C76,
			GL_PIXEL_MAP_R_TO_R_SIZE         = 0x0CB6,
			GL_PIXEL_MAP_S_TO_S              = 0x0C71,
			GL_PIXEL_MAP_S_TO_S_SIZE         = 0x0CB1,
			GL_PIXEL_MODE_BIT                = 0x00000020,
			GL_POINT                         = 0x1B00,
			GL_POINTS                        = 0x0000,
			GL_POINT_BIT                     = 0x00000002,
			GL_POINT_SIZE                    = 0x0B11,
			GL_POINT_SIZE_GRANULARITY        = 0x0B13,
			GL_POINT_SIZE_RANGE              = 0x0B12,
			GL_POINT_SMOOTH                  = 0x0B10,
			GL_POINT_SMOOTH_HINT             = 0x0C51,
			GL_POINT_TOKEN                   = 0x0701,
			GL_POLYGON                       = 0x0009,
			GL_POLYGON_BIT                   = 0x00000008,
			GL_POLYGON_MODE                  = 0x0B40,
			GL_POLYGON_OFFSET_FACTOR         = 0x8038,
			GL_POLYGON_OFFSET_FILL           = 0x8037,
			GL_POLYGON_OFFSET_LINE           = 0x2A02,
			GL_POLYGON_OFFSET_POINT          = 0x2A01,
			GL_POLYGON_OFFSET_UNITS          = 0x2A00,
			GL_POLYGON_SMOOTH                = 0x0B41,
			GL_POLYGON_SMOOTH_HINT           = 0x0C53,
			GL_POLYGON_STIPPLE               = 0x0B42,
			GL_POLYGON_STIPPLE_BIT           = 0x00000010,
			GL_POLYGON_TOKEN                 = 0x0703,
			GL_POSITION                      = 0x1203,
			GL_PROJECTION                    = 0x1701,
			GL_PROJECTION_MATRIX             = 0x0BA7,
			GL_PROJECTION_STACK_DEPTH        = 0x0BA4,
			GL_PROXY_TEXTURE_1D              = 0x8063,
			GL_PROXY_TEXTURE_2D              = 0x8064,
			GL_Q                             = 0x2003,
			GL_QUADRATIC_ATTENUATION         = 0x1209,
			GL_QUADS                         = 0x0007,
			GL_QUAD_STRIP                    = 0x0008,
			GL_R                             = 0x2002,
			GL_R3_G3_B2                      = 0x2A10,
			GL_READ_BUFFER                   = 0x0C02,
			GL_RED                           = 0x1903,
			GL_RED_BIAS                      = 0x0D15,
			GL_RED_BITS                      = 0x0D52,
			GL_RED_SCALE                     = 0x0D14,
			GL_RENDER                        = 0x1C00,
			GL_RENDERER                      = 0x1F01,
			GL_RENDER_MODE                   = 0x0C40,
			GL_REPEAT                        = 0x2901,
			GL_REPLACE                       = 0x1E01,
			GL_RETURN                        = 0x0102,
			GL_RGB                           = 0x1907,
			GL_RGB10                         = 0x8052,
			GL_RGB10_A2                      = 0x8059,
			GL_RGB12                         = 0x8053,
			GL_RGB16                         = 0x8054,
			GL_RGB4                          = 0x804F,
			GL_RGB5                          = 0x8050,
			GL_RGB5_A1                       = 0x8057,
			GL_RGB8                          = 0x8051,
			GL_RGBA                          = 0x1908,
			GL_RGBA12                        = 0x805A,
			GL_RGBA16                        = 0x805B,
			GL_RGBA2                         = 0x8055,
			GL_RGBA4                         = 0x8056,
			GL_RGBA8                         = 0x8058,
			GL_RGBA_MODE                     = 0x0C31,
			GL_RIGHT                         = 0x0407,
			GL_S                             = 0x2000,
			GL_SCISSOR_BIT                   = 0x00080000,
			GL_SCISSOR_BOX                   = 0x0C10,
			GL_SCISSOR_TEST                  = 0x0C11,
			GL_SELECT                        = 0x1C02,
			GL_SELECTION_BUFFER_POINTER      = 0x0DF3,
			GL_SELECTION_BUFFER_SIZE         = 0x0DF4,
			GL_SET                           = 0x150F,
			GL_SHADE_MODEL                   = 0x0B54,
			GL_SHININESS                     = 0x1601,
			GL_SHORT                         = 0x1402,
			GL_SMOOTH                        = 0x1D01,
			GL_SPECULAR                      = 0x1202,
			GL_SPHERE_MAP                    = 0x2402,
			GL_SPOT_CUTOFF                   = 0x1206,
			GL_SPOT_DIRECTION                = 0x1204,
			GL_SPOT_EXPONENT                 = 0x1205,
			GL_SRC_ALPHA                     = 0x0302,
			GL_SRC_ALPHA_SATURATE            = 0x0308,
			GL_SRC_COLOR                     = 0x0300,
			GL_STACK_OVERFLOW                = 0x0503,
			GL_STACK_UNDERFLOW               = 0x0504,
			GL_STENCIL                       = 0x1802,
			GL_STENCIL_BITS                  = 0x0D57,
			GL_STENCIL_BUFFER_BIT            = 0x00000400,
			GL_STENCIL_CLEAR_VALUE           = 0x0B91,
			GL_STENCIL_FAIL                  = 0x0B94,
			GL_STENCIL_FUNC                  = 0x0B92,
			GL_STENCIL_INDEX                 = 0x1901,
			GL_STENCIL_PASS_DEPTH_FAIL       = 0x0B95,
			GL_STENCIL_PASS_DEPTH_PASS       = 0x0B96,
			GL_STENCIL_REF                   = 0x0B97,
			GL_STENCIL_TEST                  = 0x0B90,
			GL_STENCIL_VALUE_MASK            = 0x0B93,
			GL_STENCIL_WRITEMASK             = 0x0B98,
			GL_STEREO                        = 0x0C33,
			GL_SUBPIXEL_BITS                 = 0x0D50,
			GL_T                             = 0x2001,
			GL_T2F_C3F_V3F                   = 0x2A2A,
			GL_T2F_C4F_N3F_V3F               = 0x2A2C,
			GL_T2F_C4UB_V3F                  = 0x2A29,
			GL_T2F_N3F_V3F                   = 0x2A2B,
			GL_T2F_V3F                       = 0x2A27,
			GL_T4F_C4F_N3F_V4F               = 0x2A2D,
			GL_T4F_V4F                       = 0x2A28,
			GL_TEXTURE                       = 0x1702,
			GL_TEXTURE_1D                    = 0x0DE0,
			GL_TEXTURE_2D                    = 0x0DE1,
			GL_TEXTURE_ALPHA_SIZE            = 0x805F,
			GL_TEXTURE_BINDING_1D            = 0x8068,
			GL_TEXTURE_BINDING_2D            = 0x8069,
			GL_TEXTURE_BIT                   = 0x00040000,
			GL_TEXTURE_BLUE_SIZE             = 0x805E,
			GL_TEXTURE_BORDER                = 0x1005,
			GL_TEXTURE_BORDER_COLOR          = 0x1004,
			GL_TEXTURE_COMPONENTS            = 0x1003,
			GL_TEXTURE_COORD_ARRAY           = 0x8078,
			GL_TEXTURE_COORD_ARRAY_POINTER   = 0x8092,
			GL_TEXTURE_COORD_ARRAY_SIZE      = 0x8088,
			GL_TEXTURE_COORD_ARRAY_STRIDE    = 0x808A,
			GL_TEXTURE_COORD_ARRAY_TYPE      = 0x8089,
			GL_TEXTURE_ENV                   = 0x2300,
			GL_TEXTURE_ENV_COLOR             = 0x2201,
			GL_TEXTURE_ENV_MODE              = 0x2200,
			GL_TEXTURE_GEN_MODE              = 0x2500,
			GL_TEXTURE_GEN_Q                 = 0x0C63,
			GL_TEXTURE_GEN_R                 = 0x0C62,
			GL_TEXTURE_GEN_S                 = 0x0C60,
			GL_TEXTURE_GEN_T                 = 0x0C61,
			GL_TEXTURE_GREEN_SIZE            = 0x805D,
			GL_TEXTURE_HEIGHT                = 0x1001,
			GL_TEXTURE_INTENSITY_SIZE        = 0x8061,
			GL_TEXTURE_INTERNAL_FORMAT       = 0x1003,
			GL_TEXTURE_LUMINANCE_SIZE        = 0x8060,
			GL_TEXTURE_MAG_FILTER            = 0x2800,
			GL_TEXTURE_MATRIX                = 0x0BA8,
			GL_TEXTURE_MIN_FILTER            = 0x2801,
			GL_TEXTURE_PRIORITY              = 0x8066,
			GL_TEXTURE_RED_SIZE              = 0x805C,
			GL_TEXTURE_RESIDENT              = 0x8067,
			GL_TEXTURE_STACK_DEPTH           = 0x0BA5,
			GL_TEXTURE_WIDTH                 = 0x1000,
			GL_TEXTURE_WRAP_S                = 0x2802,
			GL_TEXTURE_WRAP_T                = 0x2803,
			GL_TRANSFORM_BIT                 = 0x00001000,
			GL_TRIANGLES                     = 0x0004,
			GL_TRIANGLE_FAN                  = 0x0006,
			GL_TRIANGLE_STRIP                = 0x0005,
			GL_TRUE                          = 1,
			GL_UNPACK_ALIGNMENT              = 0x0CF5,
			GL_UNPACK_LSB_FIRST              = 0x0CF1,
			GL_UNPACK_ROW_LENGTH             = 0x0CF2,
			GL_UNPACK_SKIP_PIXELS            = 0x0CF4,
			GL_UNPACK_SKIP_ROWS              = 0x0CF3,
			GL_UNPACK_SWAP_BYTES             = 0x0CF0,
			GL_UNSIGNED_BYTE                 = 0x1401,
			GL_UNSIGNED_INT                  = 0x1405,
			GL_UNSIGNED_SHORT                = 0x1403,
			GL_V2F                           = 0x2A20,
			GL_V3F                           = 0x2A21,
			GL_VENDOR                        = 0x1F00,
			GL_VERSION                       = 0x1F02,
			GL_VERTEX_ARRAY                  = 0x8074,
			GL_VERTEX_ARRAY_POINTER          = 0x808E,
			GL_VERTEX_ARRAY_SIZE             = 0x807A,
			GL_VERTEX_ARRAY_STRIDE           = 0x807C,
			GL_VERTEX_ARRAY_TYPE             = 0x807B,
			GL_VIEWPORT                      = 0x0BA2,
			GL_VIEWPORT_BIT                  = 0x00000800,
			GL_XOR                           = 0x1506,
			GL_ZERO                          = 0,
			GL_ZOOM_X                        = 0x0D16,
			GL_ZOOM_Y                        = 0x0D17,
			
			GL_ALIASED_LINE_WIDTH_RANGE      = 0x846E,
			GL_ALIASED_POINT_SIZE_RANGE      = 0x846D,
			GL_BGR                           = 0x80E0,
			GL_BGRA                          = 0x80E1,
			GL_CLAMP_TO_EDGE                 = 0x812F,
			GL_LIGHT_MODEL_COLOR_CONTROL     = 0x81F8,
			GL_MAX_3D_TEXTURE_SIZE           = 0x8073,
			GL_MAX_ELEMENTS_INDICES          = 0x80E9,
			GL_MAX_ELEMENTS_VERTICES         = 0x80E8,
			GL_PACK_IMAGE_HEIGHT             = 0x806C,
			GL_PACK_SKIP_IMAGES              = 0x806B,
			GL_PROXY_TEXTURE_3D              = 0x8070,
			GL_RESCALE_NORMAL                = 0x803A,
			GL_SEPARATE_SPECULAR_COLOR       = 0x81FA,
			GL_SINGLE_COLOR                  = 0x81F9,
			GL_SMOOTH_LINE_WIDTH_GRANULARITY = 0x0B23,
			GL_SMOOTH_LINE_WIDTH_RANGE       = 0x0B22,
			GL_SMOOTH_POINT_SIZE_GRANULARITY = 0x0B13,
			GL_SMOOTH_POINT_SIZE_RANGE       = 0x0B12,
			GL_TEXTURE_3D                    = 0x806F,
			GL_TEXTURE_BASE_LEVEL            = 0x813C,
			GL_TEXTURE_BINDING_3D            = 0x806A,
			GL_TEXTURE_DEPTH                 = 0x8071,
			GL_TEXTURE_MAX_LEVEL             = 0x813D,
			GL_TEXTURE_MAX_LOD               = 0x813B,
			GL_TEXTURE_MIN_LOD               = 0x813A,
			GL_TEXTURE_WRAP_R                = 0x8072,
			GL_UNPACK_IMAGE_HEIGHT           = 0x806E,
			GL_UNPACK_SKIP_IMAGES            = 0x806D,
			GL_UNSIGNED_BYTE_2_3_3_REV       = 0x8362,
			GL_UNSIGNED_BYTE_3_3_2           = 0x8032,
			GL_UNSIGNED_INT_10_10_10_2       = 0x8036,
			GL_UNSIGNED_INT_2_10_10_10_REV   = 0x8368,
			GL_UNSIGNED_INT_8_8_8_8          = 0x8035,
			GL_UNSIGNED_INT_8_8_8_8_REV      = 0x8367,
			GL_UNSIGNED_SHORT_1_5_5_5_REV    = 0x8366,
			GL_UNSIGNED_SHORT_4_4_4_4        = 0x8033,
			GL_UNSIGNED_SHORT_4_4_4_4_REV    = 0x8365,
			GL_UNSIGNED_SHORT_5_5_5_1        = 0x8034,
			GL_UNSIGNED_SHORT_5_6_5          = 0x8363,
			GL_UNSIGNED_SHORT_5_6_5_REV      = 0x8364,
			
			GL_ACTIVE_TEXTURE                = 0x84E0,
			GL_ADD_SIGNED                    = 0x8574,
			GL_CLAMP_TO_BORDER               = 0x812D,
			GL_CLIENT_ACTIVE_TEXTURE         = 0x84E1,
			GL_COMBINE                       = 0x8570,
			GL_COMBINE_ALPHA                 = 0x8572,
			GL_COMBINE_RGB                   = 0x8571,
			GL_COMPRESSED_ALPHA              = 0x84E9,
			GL_COMPRESSED_INTENSITY          = 0x84EC,
			GL_COMPRESSED_LUMINANCE          = 0x84EA,
			GL_COMPRESSED_LUMINANCE_ALPHA    = 0x84EB,
			GL_COMPRESSED_RGB                = 0x84ED,
			GL_COMPRESSED_RGBA               = 0x84EE,
			GL_COMPRESSED_TEXTURE_FORMATS    = 0x86A3,
			GL_CONSTANT                      = 0x8576,
			GL_DOT3_RGB                      = 0x86AE,
			GL_DOT3_RGBA                     = 0x86AF,
			GL_INTERPOLATE                   = 0x8575,
			GL_MAX_CUBE_MAP_TEXTURE_SIZE     = 0x851C,
			GL_MAX_TEXTURE_UNITS             = 0x84E2,
			GL_MULTISAMPLE                   = 0x809D,
			GL_MULTISAMPLE_BIT               = 0x20000000,
			GL_NORMAL_MAP                    = 0x8511,
			GL_NUM_COMPRESSED_TEXTURE_FORMATS = 0x86A2,
			GL_OPERAND0_ALPHA                = 0x8598,
			GL_OPERAND0_RGB                  = 0x8590,
			GL_OPERAND1_ALPHA                = 0x8599,
			GL_OPERAND1_RGB                  = 0x8591,
			GL_OPERAND2_ALPHA                = 0x859A,
			GL_OPERAND2_RGB                  = 0x8592,
			GL_PREVIOUS                      = 0x8578,
			GL_PRIMARY_COLOR                 = 0x8577,
			GL_PROXY_TEXTURE_CUBE_MAP        = 0x851B,
			GL_REFLECTION_MAP                = 0x8512,
			GL_RGB_SCALE                     = 0x8573,
			GL_SAMPLES                       = 0x80A9,
			GL_SAMPLE_ALPHA_TO_COVERAGE      = 0x809E,
			GL_SAMPLE_ALPHA_TO_ONE           = 0x809F,
			GL_SAMPLE_BUFFERS                = 0x80A8,
			GL_SAMPLE_COVERAGE               = 0x80A0,
			GL_SAMPLE_COVERAGE_INVERT        = 0x80AB,
			GL_SAMPLE_COVERAGE_VALUE         = 0x80AA,
			GL_SOURCE0_ALPHA                 = 0x8588,
			GL_SOURCE0_RGB                   = 0x8580,
			GL_SOURCE1_ALPHA                 = 0x8589,
			GL_SOURCE1_RGB                   = 0x8581,
			GL_SOURCE2_ALPHA                 = 0x858A,
			GL_SOURCE2_RGB                   = 0x8582,
			GL_SUBTRACT                      = 0x84E7,
			GL_TEXTURE0                      = 0x84C0,
			GL_TEXTURE1                      = 0x84C1,
			GL_TEXTURE10                     = 0x84CA,
			GL_TEXTURE11                     = 0x84CB,
			GL_TEXTURE12                     = 0x84CC,
			GL_TEXTURE13                     = 0x84CD,
			GL_TEXTURE14                     = 0x84CE,
			GL_TEXTURE15                     = 0x84CF,
			GL_TEXTURE16                     = 0x84D0,
			GL_TEXTURE17                     = 0x84D1,
			GL_TEXTURE18                     = 0x84D2,
			GL_TEXTURE19                     = 0x84D3,
			GL_TEXTURE2                      = 0x84C2,
			GL_TEXTURE20                     = 0x84D4,
			GL_TEXTURE21                     = 0x84D5,
			GL_TEXTURE22                     = 0x84D6,
			GL_TEXTURE23                     = 0x84D7,
			GL_TEXTURE24                     = 0x84D8,
			GL_TEXTURE25                     = 0x84D9,
			GL_TEXTURE26                     = 0x84DA,
			GL_TEXTURE27                     = 0x84DB,
			GL_TEXTURE28                     = 0x84DC,
			GL_TEXTURE29                     = 0x84DD,
			GL_TEXTURE3                      = 0x84C3,
			GL_TEXTURE30                     = 0x84DE,
			GL_TEXTURE31                     = 0x84DF,
			GL_TEXTURE4                      = 0x84C4,
			GL_TEXTURE5                      = 0x84C5,
			GL_TEXTURE6                      = 0x84C6,
			GL_TEXTURE7                      = 0x84C7,
			GL_TEXTURE8                      = 0x84C8,
			GL_TEXTURE9                      = 0x84C9,
			GL_TEXTURE_BINDING_CUBE_MAP      = 0x8514,
			GL_TEXTURE_COMPRESSED            = 0x86A1,
			GL_TEXTURE_COMPRESSED_IMAGE_SIZE = 0x86A0,
			GL_TEXTURE_COMPRESSION_HINT      = 0x84EF,
			GL_TEXTURE_CUBE_MAP              = 0x8513,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_X   = 0x8516,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Y   = 0x8518,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Z   = 0x851A,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X   = 0x8515,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Y   = 0x8517,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Z   = 0x8519,
			GL_TRANSPOSE_COLOR_MATRIX        = 0x84E6,
			GL_TRANSPOSE_MODELVIEW_MATRIX    = 0x84E3,
			GL_TRANSPOSE_PROJECTION_MATRIX   = 0x84E4,
			GL_TRANSPOSE_TEXTURE_MATRIX      = 0x84E5,
			
			GL_BLEND_COLOR                   = 0x8005,
			GL_BLEND_DST_ALPHA               = 0x80CA,
			GL_BLEND_DST_RGB                 = 0x80C8,
			GL_BLEND_SRC_ALPHA               = 0x80CB,
			GL_BLEND_SRC_RGB                 = 0x80C9,
			GL_COLOR_SUM                     = 0x8458,
			GL_COMPARE_R_TO_TEXTURE          = 0x884E,
			GL_CONSTANT_ALPHA                = 0x8003,
			GL_CONSTANT_COLOR                = 0x8001,
			GL_CURRENT_FOG_COORDINATE        = 0x8453,
			GL_CURRENT_SECONDARY_COLOR       = 0x8459,
			GL_DECR_WRAP                     = 0x8508,
			GL_DEPTH_COMPONENT16             = 0x81A5,
			GL_DEPTH_COMPONENT24             = 0x81A6,
			GL_DEPTH_COMPONENT32             = 0x81A7,
			GL_DEPTH_TEXTURE_MODE            = 0x884B,
			GL_FOG_COORDINATE                = 0x8451,
			GL_FOG_COORDINATE_ARRAY          = 0x8457,
			GL_FOG_COORDINATE_ARRAY_POINTER  = 0x8456,
			GL_FOG_COORDINATE_ARRAY_STRIDE   = 0x8455,
			GL_FOG_COORDINATE_ARRAY_TYPE     = 0x8454,
			GL_FOG_COORDINATE_SOURCE         = 0x8450,
			GL_FRAGMENT_DEPTH                = 0x8452,
			GL_FUNC_ADD                      = 0x8006,
			GL_FUNC_REVERSE_SUBTRACT         = 0x800B,
			GL_FUNC_SUBTRACT                 = 0x800A,
			GL_GENERATE_MIPMAP               = 0x8191,
			GL_GENERATE_MIPMAP_HINT          = 0x8192,
			GL_INCR_WRAP                     = 0x8507,
			GL_MAX                           = 0x8008,
			GL_MAX_TEXTURE_LOD_BIAS          = 0x84FD,
			GL_MIN                           = 0x8007,
			GL_MIRRORED_REPEAT               = 0x8370,
			GL_ONE_MINUS_CONSTANT_ALPHA      = 0x8004,
			GL_ONE_MINUS_CONSTANT_COLOR      = 0x8002,
			GL_POINT_DISTANCE_ATTENUATION    = 0x8129,
			GL_POINT_FADE_THRESHOLD_SIZE     = 0x8128,
			GL_POINT_SIZE_MAX                = 0x8127,
			GL_POINT_SIZE_MIN                = 0x8126,
			GL_SECONDARY_COLOR_ARRAY         = 0x845E,
			GL_SECONDARY_COLOR_ARRAY_POINTER = 0x845D,
			GL_SECONDARY_COLOR_ARRAY_SIZE    = 0x845A,
			GL_SECONDARY_COLOR_ARRAY_STRIDE  = 0x845C,
			GL_SECONDARY_COLOR_ARRAY_TYPE    = 0x845B,
			GL_TEXTURE_COMPARE_FUNC          = 0x884D,
			GL_TEXTURE_COMPARE_MODE          = 0x884C,
			GL_TEXTURE_DEPTH_SIZE            = 0x884A,
			GL_TEXTURE_FILTER_CONTROL        = 0x8500,
			GL_TEXTURE_LOD_BIAS              = 0x8501,
			
			GL_ARRAY_BUFFER                  = 0x8892,
			GL_ARRAY_BUFFER_BINDING          = 0x8894,
			GL_BUFFER_ACCESS                 = 0x88BB,
			GL_BUFFER_MAPPED                 = 0x88BC,
			GL_BUFFER_MAP_POINTER            = 0x88BD,
			GL_BUFFER_SIZE                   = 0x8764,
			GL_BUFFER_USAGE                  = 0x8765,
			GL_COLOR_ARRAY_BUFFER_BINDING    = 0x8898,
			GL_CURRENT_FOG_COORD             = 0x8453,
			GL_CURRENT_QUERY                 = 0x8865,
			GL_DYNAMIC_COPY                  = 0x88EA,
			GL_DYNAMIC_DRAW                  = 0x88E8,
			GL_DYNAMIC_READ                  = 0x88E9,
			GL_EDGE_FLAG_ARRAY_BUFFER_BINDING = 0x889B,
			GL_ELEMENT_ARRAY_BUFFER          = 0x8893,
			GL_ELEMENT_ARRAY_BUFFER_BINDING  = 0x8895,
			GL_FOG_COORD                     = 0x8451,
			GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING = 0x889D,
			GL_FOG_COORD_ARRAY               = 0x8457,
			GL_FOG_COORD_ARRAY_BUFFER_BINDING = 0x889D,
			GL_FOG_COORD_ARRAY_POINTER       = 0x8456,
			GL_FOG_COORD_ARRAY_STRIDE        = 0x8455,
			GL_FOG_COORD_ARRAY_TYPE          = 0x8454,
			GL_FOG_COORD_SRC                 = 0x8450,
			GL_INDEX_ARRAY_BUFFER_BINDING    = 0x8899,
			GL_NORMAL_ARRAY_BUFFER_BINDING   = 0x8897,
			GL_QUERY_COUNTER_BITS            = 0x8864,
			GL_QUERY_RESULT                  = 0x8866,
			GL_QUERY_RESULT_AVAILABLE        = 0x8867,
			GL_READ_ONLY                     = 0x88B8,
			GL_READ_WRITE                    = 0x88BA,
			GL_SAMPLES_PASSED                = 0x8914,
			GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING = 0x889C,
			GL_SRC0_ALPHA                    = 0x8588,
			GL_SRC0_RGB                      = 0x8580,
			GL_SRC1_ALPHA                    = 0x8589,
			GL_SRC1_RGB                      = 0x8581,
			GL_SRC2_ALPHA                    = 0x858A,
			GL_SRC2_RGB                      = 0x8582,
			GL_STATIC_COPY                   = 0x88E6,
			GL_STATIC_DRAW                   = 0x88E4,
			GL_STATIC_READ                   = 0x88E5,
			GL_STREAM_COPY                   = 0x88E2,
			GL_STREAM_DRAW                   = 0x88E0,
			GL_STREAM_READ                   = 0x88E1,
			GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING = 0x889A,
			GL_VERTEX_ARRAY_BUFFER_BINDING   = 0x8896,
			GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING = 0x889F,
			GL_WEIGHT_ARRAY_BUFFER_BINDING   = 0x889E,
			GL_WRITE_ONLY                    = 0x88B9,
			
			GL_ACTIVE_ATTRIBUTES             = 0x8B89,
			GL_ACTIVE_ATTRIBUTE_MAX_LENGTH   = 0x8B8A,
			GL_ACTIVE_UNIFORMS               = 0x8B86,
			GL_ACTIVE_UNIFORM_MAX_LENGTH     = 0x8B87,
			GL_ATTACHED_SHADERS              = 0x8B85,
			GL_BLEND_EQUATION_ALPHA          = 0x883D,
			GL_BLEND_EQUATION_RGB            = 0x8009,
			GL_BOOL                          = 0x8B56,
			GL_BOOL_VEC2                     = 0x8B57,
			GL_BOOL_VEC3                     = 0x8B58,
			GL_BOOL_VEC4                     = 0x8B59,
			GL_COMPILE_STATUS                = 0x8B81,
			GL_COORD_REPLACE                 = 0x8862,
			GL_CURRENT_PROGRAM               = 0x8B8D,
			GL_CURRENT_VERTEX_ATTRIB         = 0x8626,
			GL_DELETE_STATUS                 = 0x8B80,
			GL_DRAW_BUFFER0                  = 0x8825,
			GL_DRAW_BUFFER1                  = 0x8826,
			GL_DRAW_BUFFER10                 = 0x882F,
			GL_DRAW_BUFFER11                 = 0x8830,
			GL_DRAW_BUFFER12                 = 0x8831,
			GL_DRAW_BUFFER13                 = 0x8832,
			GL_DRAW_BUFFER14                 = 0x8833,
			GL_DRAW_BUFFER15                 = 0x8834,
			GL_DRAW_BUFFER2                  = 0x8827,
			GL_DRAW_BUFFER3                  = 0x8828,
			GL_DRAW_BUFFER4                  = 0x8829,
			GL_DRAW_BUFFER5                  = 0x882A,
			GL_DRAW_BUFFER6                  = 0x882B,
			GL_DRAW_BUFFER7                  = 0x882C,
			GL_DRAW_BUFFER8                  = 0x882D,
			GL_DRAW_BUFFER9                  = 0x882E,
			GL_FLOAT_MAT2                    = 0x8B5A,
			GL_FLOAT_MAT3                    = 0x8B5B,
			GL_FLOAT_MAT4                    = 0x8B5C,
			GL_FLOAT_VEC2                    = 0x8B50,
			GL_FLOAT_VEC3                    = 0x8B51,
			GL_FLOAT_VEC4                    = 0x8B52,
			GL_FRAGMENT_SHADER               = 0x8B30,
			GL_FRAGMENT_SHADER_DERIVATIVE_HINT = 0x8B8B,
			GL_INFO_LOG_LENGTH               = 0x8B84,
			GL_INT_VEC2                      = 0x8B53,
			GL_INT_VEC3                      = 0x8B54,
			GL_INT_VEC4                      = 0x8B55,
			GL_LINK_STATUS                   = 0x8B82,
			GL_LOWER_LEFT                    = 0x8CA1,
			GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS = 0x8B4D,
			GL_MAX_DRAW_BUFFERS              = 0x8824,
			GL_MAX_FRAGMENT_UNIFORM_COMPONENTS = 0x8B49,
			GL_MAX_TEXTURE_COORDS            = 0x8871,
			GL_MAX_TEXTURE_IMAGE_UNITS       = 0x8872,
			GL_MAX_VARYING_FLOATS            = 0x8B4B,
			GL_MAX_VERTEX_ATTRIBS            = 0x8869,
			GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS = 0x8B4C,
			GL_MAX_VERTEX_UNIFORM_COMPONENTS = 0x8B4A,
			GL_POINT_SPRITE                  = 0x8861,
			GL_POINT_SPRITE_COORD_ORIGIN     = 0x8CA0,
			GL_SAMPLER_1D                    = 0x8B5D,
			GL_SAMPLER_1D_SHADOW             = 0x8B61,
			GL_SAMPLER_2D                    = 0x8B5E,
			GL_SAMPLER_2D_SHADOW             = 0x8B62,
			GL_SAMPLER_3D                    = 0x8B5F,
			GL_SAMPLER_CUBE                  = 0x8B60,
			GL_SHADER_SOURCE_LENGTH          = 0x8B88,
			GL_SHADER_TYPE                   = 0x8B4F,
			GL_SHADING_LANGUAGE_VERSION      = 0x8B8C,
			GL_STENCIL_BACK_FAIL             = 0x8801,
			GL_STENCIL_BACK_FUNC             = 0x8800,
			GL_STENCIL_BACK_PASS_DEPTH_FAIL  = 0x8802,
			GL_STENCIL_BACK_PASS_DEPTH_PASS  = 0x8803,
			GL_STENCIL_BACK_REF              = 0x8CA3,
			GL_STENCIL_BACK_VALUE_MASK       = 0x8CA4,
			GL_STENCIL_BACK_WRITEMASK        = 0x8CA5,
			GL_UPPER_LEFT                    = 0x8CA2,
			GL_VALIDATE_STATUS               = 0x8B83,
			GL_VERTEX_ATTRIB_ARRAY_ENABLED   = 0x8622,
			GL_VERTEX_ATTRIB_ARRAY_NORMALIZED = 0x886A,
			GL_VERTEX_ATTRIB_ARRAY_POINTER   = 0x8645,
			GL_VERTEX_ATTRIB_ARRAY_SIZE      = 0x8623,
			GL_VERTEX_ATTRIB_ARRAY_STRIDE    = 0x8624,
			GL_VERTEX_ATTRIB_ARRAY_TYPE      = 0x8625,
			GL_VERTEX_PROGRAM_POINT_SIZE     = 0x8642,
			GL_VERTEX_PROGRAM_TWO_SIDE       = 0x8643,
			GL_VERTEX_SHADER                 = 0x8B31,
			
			GL_COMPRESSED_SLUMINANCE         = 0x8C4A,
			GL_COMPRESSED_SLUMINANCE_ALPHA   = 0x8C4B,
			GL_COMPRESSED_SRGB               = 0x8C48,
			GL_COMPRESSED_SRGB_ALPHA         = 0x8C49,
			GL_CURRENT_RASTER_SECONDARY_COLOR = 0x845F,
			GL_FLOAT_MAT2x3                  = 0x8B65,
			GL_FLOAT_MAT2x4                  = 0x8B66,
			GL_FLOAT_MAT3x2                  = 0x8B67,
			GL_FLOAT_MAT3x4                  = 0x8B68,
			GL_FLOAT_MAT4x2                  = 0x8B69,
			GL_FLOAT_MAT4x3                  = 0x8B6A,
			GL_PIXEL_PACK_BUFFER             = 0x88EB,
			GL_PIXEL_PACK_BUFFER_BINDING     = 0x88ED,
			GL_PIXEL_UNPACK_BUFFER           = 0x88EC,
			GL_PIXEL_UNPACK_BUFFER_BINDING   = 0x88EF,
			GL_SLUMINANCE                    = 0x8C46,
			GL_SLUMINANCE8                   = 0x8C47,
			GL_SLUMINANCE8_ALPHA8            = 0x8C45,
			GL_SLUMINANCE_ALPHA              = 0x8C44,
			GL_SRGB                          = 0x8C40,
			GL_SRGB8                         = 0x8C41,
			GL_SRGB8_ALPHA8                  = 0x8C43,
			GL_SRGB_ALPHA                    = 0x8C42,
			
		};
		
		
		extern void (CODEGEN_FUNCPTR *glDrawArraysInstancedARB)(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
		extern void (CODEGEN_FUNCPTR *glDrawElementsInstancedARB)(GLenum mode, GLsizei count, GLenum type, const void * indices, GLsizei primcount);
		
		extern void (CODEGEN_FUNCPTR *glBindBufferBase)(GLenum target, GLuint index, GLuint buffer);
		extern void (CODEGEN_FUNCPTR *glBindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
		extern void (CODEGEN_FUNCPTR *glGetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformBlockName);
		extern void (CODEGEN_FUNCPTR *glGetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetActiveUniformName)(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformName);
		extern void (CODEGEN_FUNCPTR *glGetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint * uniformIndices, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetIntegeri_v)(GLenum target, GLuint index, GLint * data);
		extern GLuint (CODEGEN_FUNCPTR *glGetUniformBlockIndex)(GLuint program, const GLchar * uniformBlockName);
		extern void (CODEGEN_FUNCPTR *glGetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar *const* uniformNames, GLuint * uniformIndices);
		extern void (CODEGEN_FUNCPTR *glUniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
		
		extern void (CODEGEN_FUNCPTR *glVertexAttribDivisorARB)(GLuint index, GLuint divisor);
		
		extern void (CODEGEN_FUNCPTR *glBindVertexArray)(GLuint ren_array);
		extern void (CODEGEN_FUNCPTR *glDeleteVertexArrays)(GLsizei n, const GLuint * arrays);
		extern void (CODEGEN_FUNCPTR *glGenVertexArrays)(GLsizei n, GLuint * arrays);
		extern GLboolean (CODEGEN_FUNCPTR *glIsVertexArray)(GLuint ren_array);
		
		extern void (CODEGEN_FUNCPTR *glBindFramebufferEXT)(GLenum target, GLuint framebuffer);
		extern void (CODEGEN_FUNCPTR *glBindRenderbufferEXT)(GLenum target, GLuint renderbuffer);
		extern GLenum (CODEGEN_FUNCPTR *glCheckFramebufferStatusEXT)(GLenum target);
		extern void (CODEGEN_FUNCPTR *glDeleteFramebuffersEXT)(GLsizei n, const GLuint * framebuffers);
		extern void (CODEGEN_FUNCPTR *glDeleteRenderbuffersEXT)(GLsizei n, const GLuint * renderbuffers);
		extern void (CODEGEN_FUNCPTR *glFramebufferRenderbufferEXT)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
		extern void (CODEGEN_FUNCPTR *glFramebufferTexture1DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
		extern void (CODEGEN_FUNCPTR *glFramebufferTexture2DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
		extern void (CODEGEN_FUNCPTR *glFramebufferTexture3DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
		extern void (CODEGEN_FUNCPTR *glGenFramebuffersEXT)(GLsizei n, GLuint * framebuffers);
		extern void (CODEGEN_FUNCPTR *glGenRenderbuffersEXT)(GLsizei n, GLuint * renderbuffers);
		extern void (CODEGEN_FUNCPTR *glGenerateMipmapEXT)(GLenum target);
		extern void (CODEGEN_FUNCPTR *glGetFramebufferAttachmentParameterivEXT)(GLenum target, GLenum attachment, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetRenderbufferParameterivEXT)(GLenum target, GLenum pname, GLint * params);
		extern GLboolean (CODEGEN_FUNCPTR *glIsFramebufferEXT)(GLuint framebuffer);
		extern GLboolean (CODEGEN_FUNCPTR *glIsRenderbufferEXT)(GLuint renderbuffer);
		extern void (CODEGEN_FUNCPTR *glRenderbufferStorageEXT)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
		
		
		
		
		extern void (CODEGEN_FUNCPTR *glAccum)(GLenum op, GLfloat value);
		extern void (CODEGEN_FUNCPTR *glAlphaFunc)(GLenum func, GLfloat ref);
		extern void (CODEGEN_FUNCPTR *glBegin)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *glBitmap)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte * bitmap);
		extern void (CODEGEN_FUNCPTR *glBlendFunc)(GLenum sfactor, GLenum dfactor);
		extern void (CODEGEN_FUNCPTR *glCallList)(GLuint list);
		extern void (CODEGEN_FUNCPTR *glCallLists)(GLsizei n, GLenum type, const void * lists);
		extern void (CODEGEN_FUNCPTR *glClear)(GLbitfield mask);
		extern void (CODEGEN_FUNCPTR *glClearAccum)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		extern void (CODEGEN_FUNCPTR *glClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		extern void (CODEGEN_FUNCPTR *glClearDepth)(GLdouble depth);
		extern void (CODEGEN_FUNCPTR *glClearIndex)(GLfloat c);
		extern void (CODEGEN_FUNCPTR *glClearStencil)(GLint s);
		extern void (CODEGEN_FUNCPTR *glClipPlane)(GLenum plane, const GLdouble * equation);
		extern void (CODEGEN_FUNCPTR *glColor3b)(GLbyte red, GLbyte green, GLbyte blue);
		extern void (CODEGEN_FUNCPTR *glColor3bv)(const GLbyte * v);
		extern void (CODEGEN_FUNCPTR *glColor3d)(GLdouble red, GLdouble green, GLdouble blue);
		extern void (CODEGEN_FUNCPTR *glColor3dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glColor3f)(GLfloat red, GLfloat green, GLfloat blue);
		extern void (CODEGEN_FUNCPTR *glColor3fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glColor3i)(GLint red, GLint green, GLint blue);
		extern void (CODEGEN_FUNCPTR *glColor3iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *glColor3s)(GLshort red, GLshort green, GLshort blue);
		extern void (CODEGEN_FUNCPTR *glColor3sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glColor3ub)(GLubyte red, GLubyte green, GLubyte blue);
		extern void (CODEGEN_FUNCPTR *glColor3ubv)(const GLubyte * v);
		extern void (CODEGEN_FUNCPTR *glColor3ui)(GLuint red, GLuint green, GLuint blue);
		extern void (CODEGEN_FUNCPTR *glColor3uiv)(const GLuint * v);
		extern void (CODEGEN_FUNCPTR *glColor3us)(GLushort red, GLushort green, GLushort blue);
		extern void (CODEGEN_FUNCPTR *glColor3usv)(const GLushort * v);
		extern void (CODEGEN_FUNCPTR *glColor4b)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
		extern void (CODEGEN_FUNCPTR *glColor4bv)(const GLbyte * v);
		extern void (CODEGEN_FUNCPTR *glColor4d)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
		extern void (CODEGEN_FUNCPTR *glColor4dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glColor4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		extern void (CODEGEN_FUNCPTR *glColor4fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glColor4i)(GLint red, GLint green, GLint blue, GLint alpha);
		extern void (CODEGEN_FUNCPTR *glColor4iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *glColor4s)(GLshort red, GLshort green, GLshort blue, GLshort alpha);
		extern void (CODEGEN_FUNCPTR *glColor4sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glColor4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
		extern void (CODEGEN_FUNCPTR *glColor4ubv)(const GLubyte * v);
		extern void (CODEGEN_FUNCPTR *glColor4ui)(GLuint red, GLuint green, GLuint blue, GLuint alpha);
		extern void (CODEGEN_FUNCPTR *glColor4uiv)(const GLuint * v);
		extern void (CODEGEN_FUNCPTR *glColor4us)(GLushort red, GLushort green, GLushort blue, GLushort alpha);
		extern void (CODEGEN_FUNCPTR *glColor4usv)(const GLushort * v);
		extern void (CODEGEN_FUNCPTR *glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
		extern void (CODEGEN_FUNCPTR *glColorMaterial)(GLenum face, GLenum mode);
		extern void (CODEGEN_FUNCPTR *glCopyPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
		extern void (CODEGEN_FUNCPTR *glCullFace)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *glDeleteLists)(GLuint list, GLsizei range);
		extern void (CODEGEN_FUNCPTR *glDepthFunc)(GLenum func);
		extern void (CODEGEN_FUNCPTR *glDepthMask)(GLboolean flag);
		extern void (CODEGEN_FUNCPTR *glDepthRange)(GLdouble ren_near, GLdouble ren_far);
		extern void (CODEGEN_FUNCPTR *glDisable)(GLenum cap);
		extern void (CODEGEN_FUNCPTR *glDrawBuffer)(GLenum buf);
		extern void (CODEGEN_FUNCPTR *glDrawPixels)(GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *glEdgeFlag)(GLboolean flag);
		extern void (CODEGEN_FUNCPTR *glEdgeFlagv)(const GLboolean * flag);
		extern void (CODEGEN_FUNCPTR *glEnable)(GLenum cap);
		extern void (CODEGEN_FUNCPTR *glEnd)(void);
		extern void (CODEGEN_FUNCPTR *glEndList)(void);
		extern void (CODEGEN_FUNCPTR *glEvalCoord1d)(GLdouble u);
		extern void (CODEGEN_FUNCPTR *glEvalCoord1dv)(const GLdouble * u);
		extern void (CODEGEN_FUNCPTR *glEvalCoord1f)(GLfloat u);
		extern void (CODEGEN_FUNCPTR *glEvalCoord1fv)(const GLfloat * u);
		extern void (CODEGEN_FUNCPTR *glEvalCoord2d)(GLdouble u, GLdouble v);
		extern void (CODEGEN_FUNCPTR *glEvalCoord2dv)(const GLdouble * u);
		extern void (CODEGEN_FUNCPTR *glEvalCoord2f)(GLfloat u, GLfloat v);
		extern void (CODEGEN_FUNCPTR *glEvalCoord2fv)(const GLfloat * u);
		extern void (CODEGEN_FUNCPTR *glEvalMesh1)(GLenum mode, GLint i1, GLint i2);
		extern void (CODEGEN_FUNCPTR *glEvalMesh2)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
		extern void (CODEGEN_FUNCPTR *glEvalPoint1)(GLint i);
		extern void (CODEGEN_FUNCPTR *glEvalPoint2)(GLint i, GLint j);
		extern void (CODEGEN_FUNCPTR *glFeedbackBuffer)(GLsizei size, GLenum type, GLfloat * buffer);
		extern void (CODEGEN_FUNCPTR *glFinish)(void);
		extern void (CODEGEN_FUNCPTR *glFlush)(void);
		extern void (CODEGEN_FUNCPTR *glFogf)(GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *glFogfv)(GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glFogi)(GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *glFogiv)(GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *glFrontFace)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *glFrustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
		extern GLuint (CODEGEN_FUNCPTR *glGenLists)(GLsizei range);
		extern void (CODEGEN_FUNCPTR *glGetBooleanv)(GLenum pname, GLboolean * data);
		extern void (CODEGEN_FUNCPTR *glGetClipPlane)(GLenum plane, GLdouble * equation);
		extern void (CODEGEN_FUNCPTR *glGetDoublev)(GLenum pname, GLdouble * data);
		extern GLenum (CODEGEN_FUNCPTR *glGetError)(void);
		extern void (CODEGEN_FUNCPTR *glGetFloatv)(GLenum pname, GLfloat * data);
		extern void (CODEGEN_FUNCPTR *glGetIntegerv)(GLenum pname, GLint * data);
		extern void (CODEGEN_FUNCPTR *glGetLightfv)(GLenum light, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glGetLightiv)(GLenum light, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetMapdv)(GLenum target, GLenum query, GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glGetMapfv)(GLenum target, GLenum query, GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glGetMapiv)(GLenum target, GLenum query, GLint * v);
		extern void (CODEGEN_FUNCPTR *glGetMaterialfv)(GLenum face, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glGetMaterialiv)(GLenum face, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetPixelMapfv)(GLenum map, GLfloat * values);
		extern void (CODEGEN_FUNCPTR *glGetPixelMapuiv)(GLenum map, GLuint * values);
		extern void (CODEGEN_FUNCPTR *glGetPixelMapusv)(GLenum map, GLushort * values);
		extern void (CODEGEN_FUNCPTR *glGetPolygonStipple)(GLubyte * mask);
		extern const GLubyte * (CODEGEN_FUNCPTR *glGetString)(GLenum name);
		extern void (CODEGEN_FUNCPTR *glGetTexEnvfv)(GLenum target, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glGetTexEnviv)(GLenum target, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetTexGendv)(GLenum coord, GLenum pname, GLdouble * params);
		extern void (CODEGEN_FUNCPTR *glGetTexGenfv)(GLenum coord, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glGetTexGeniv)(GLenum coord, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, void * pixels);
		extern void (CODEGEN_FUNCPTR *glGetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glGetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glGetTexParameteriv)(GLenum target, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glHint)(GLenum target, GLenum mode);
		extern void (CODEGEN_FUNCPTR *glIndexMask)(GLuint mask);
		extern void (CODEGEN_FUNCPTR *glIndexd)(GLdouble c);
		extern void (CODEGEN_FUNCPTR *glIndexdv)(const GLdouble * c);
		extern void (CODEGEN_FUNCPTR *glIndexf)(GLfloat c);
		extern void (CODEGEN_FUNCPTR *glIndexfv)(const GLfloat * c);
		extern void (CODEGEN_FUNCPTR *glIndexi)(GLint c);
		extern void (CODEGEN_FUNCPTR *glIndexiv)(const GLint * c);
		extern void (CODEGEN_FUNCPTR *glIndexs)(GLshort c);
		extern void (CODEGEN_FUNCPTR *glIndexsv)(const GLshort * c);
		extern void (CODEGEN_FUNCPTR *glInitNames)(void);
		extern GLboolean (CODEGEN_FUNCPTR *glIsEnabled)(GLenum cap);
		extern GLboolean (CODEGEN_FUNCPTR *glIsList)(GLuint list);
		extern void (CODEGEN_FUNCPTR *glLightModelf)(GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *glLightModelfv)(GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glLightModeli)(GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *glLightModeliv)(GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *glLightf)(GLenum light, GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *glLightfv)(GLenum light, GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glLighti)(GLenum light, GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *glLightiv)(GLenum light, GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *glLineStipple)(GLint factor, GLushort pattern);
		extern void (CODEGEN_FUNCPTR *glLineWidth)(GLfloat width);
		extern void (CODEGEN_FUNCPTR *glListBase)(GLuint base);
		extern void (CODEGEN_FUNCPTR *glLoadIdentity)(void);
		extern void (CODEGEN_FUNCPTR *glLoadMatrixd)(const GLdouble * m);
		extern void (CODEGEN_FUNCPTR *glLoadMatrixf)(const GLfloat * m);
		extern void (CODEGEN_FUNCPTR *glLoadName)(GLuint name);
		extern void (CODEGEN_FUNCPTR *glLogicOp)(GLenum opcode);
		extern void (CODEGEN_FUNCPTR *glMap1d)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble * points);
		extern void (CODEGEN_FUNCPTR *glMap1f)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat * points);
		extern void (CODEGEN_FUNCPTR *glMap2d)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble * points);
		extern void (CODEGEN_FUNCPTR *glMap2f)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat * points);
		extern void (CODEGEN_FUNCPTR *glMapGrid1d)(GLint un, GLdouble u1, GLdouble u2);
		extern void (CODEGEN_FUNCPTR *glMapGrid1f)(GLint un, GLfloat u1, GLfloat u2);
		extern void (CODEGEN_FUNCPTR *glMapGrid2d)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
		extern void (CODEGEN_FUNCPTR *glMapGrid2f)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
		extern void (CODEGEN_FUNCPTR *glMaterialf)(GLenum face, GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *glMaterialfv)(GLenum face, GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glMateriali)(GLenum face, GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *glMaterialiv)(GLenum face, GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *glMatrixMode)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *glMultMatrixd)(const GLdouble * m);
		extern void (CODEGEN_FUNCPTR *glMultMatrixf)(const GLfloat * m);
		extern void (CODEGEN_FUNCPTR *glNewList)(GLuint list, GLenum mode);
		extern void (CODEGEN_FUNCPTR *glNormal3b)(GLbyte nx, GLbyte ny, GLbyte nz);
		extern void (CODEGEN_FUNCPTR *glNormal3bv)(const GLbyte * v);
		extern void (CODEGEN_FUNCPTR *glNormal3d)(GLdouble nx, GLdouble ny, GLdouble nz);
		extern void (CODEGEN_FUNCPTR *glNormal3dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glNormal3f)(GLfloat nx, GLfloat ny, GLfloat nz);
		extern void (CODEGEN_FUNCPTR *glNormal3fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glNormal3i)(GLint nx, GLint ny, GLint nz);
		extern void (CODEGEN_FUNCPTR *glNormal3iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *glNormal3s)(GLshort nx, GLshort ny, GLshort nz);
		extern void (CODEGEN_FUNCPTR *glNormal3sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glOrtho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
		extern void (CODEGEN_FUNCPTR *glPassThrough)(GLfloat token);
		extern void (CODEGEN_FUNCPTR *glPixelMapfv)(GLenum map, GLsizei mapsize, const GLfloat * values);
		extern void (CODEGEN_FUNCPTR *glPixelMapuiv)(GLenum map, GLsizei mapsize, const GLuint * values);
		extern void (CODEGEN_FUNCPTR *glPixelMapusv)(GLenum map, GLsizei mapsize, const GLushort * values);
		extern void (CODEGEN_FUNCPTR *glPixelStoref)(GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *glPixelStorei)(GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *glPixelTransferf)(GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *glPixelTransferi)(GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *glPixelZoom)(GLfloat xfactor, GLfloat yfactor);
		extern void (CODEGEN_FUNCPTR *glPointSize)(GLfloat size);
		extern void (CODEGEN_FUNCPTR *glPolygonMode)(GLenum face, GLenum mode);
		extern void (CODEGEN_FUNCPTR *glPolygonStipple)(const GLubyte * mask);
		extern void (CODEGEN_FUNCPTR *glPopAttrib)(void);
		extern void (CODEGEN_FUNCPTR *glPopMatrix)(void);
		extern void (CODEGEN_FUNCPTR *glPopName)(void);
		extern void (CODEGEN_FUNCPTR *glPushAttrib)(GLbitfield mask);
		extern void (CODEGEN_FUNCPTR *glPushMatrix)(void);
		extern void (CODEGEN_FUNCPTR *glPushName)(GLuint name);
		extern void (CODEGEN_FUNCPTR *glRasterPos2d)(GLdouble x, GLdouble y);
		extern void (CODEGEN_FUNCPTR *glRasterPos2dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glRasterPos2f)(GLfloat x, GLfloat y);
		extern void (CODEGEN_FUNCPTR *glRasterPos2fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glRasterPos2i)(GLint x, GLint y);
		extern void (CODEGEN_FUNCPTR *glRasterPos2iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *glRasterPos2s)(GLshort x, GLshort y);
		extern void (CODEGEN_FUNCPTR *glRasterPos2sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glRasterPos3d)(GLdouble x, GLdouble y, GLdouble z);
		extern void (CODEGEN_FUNCPTR *glRasterPos3dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glRasterPos3f)(GLfloat x, GLfloat y, GLfloat z);
		extern void (CODEGEN_FUNCPTR *glRasterPos3fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glRasterPos3i)(GLint x, GLint y, GLint z);
		extern void (CODEGEN_FUNCPTR *glRasterPos3iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *glRasterPos3s)(GLshort x, GLshort y, GLshort z);
		extern void (CODEGEN_FUNCPTR *glRasterPos3sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glRasterPos4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
		extern void (CODEGEN_FUNCPTR *glRasterPos4dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glRasterPos4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
		extern void (CODEGEN_FUNCPTR *glRasterPos4fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glRasterPos4i)(GLint x, GLint y, GLint z, GLint w);
		extern void (CODEGEN_FUNCPTR *glRasterPos4iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *glRasterPos4s)(GLshort x, GLshort y, GLshort z, GLshort w);
		extern void (CODEGEN_FUNCPTR *glRasterPos4sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glReadBuffer)(GLenum src);
		extern void (CODEGEN_FUNCPTR *glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels);
		extern void (CODEGEN_FUNCPTR *glRectd)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
		extern void (CODEGEN_FUNCPTR *glRectdv)(const GLdouble * v1, const GLdouble * v2);
		extern void (CODEGEN_FUNCPTR *glRectf)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
		extern void (CODEGEN_FUNCPTR *glRectfv)(const GLfloat * v1, const GLfloat * v2);
		extern void (CODEGEN_FUNCPTR *glRecti)(GLint x1, GLint y1, GLint x2, GLint y2);
		extern void (CODEGEN_FUNCPTR *glRectiv)(const GLint * v1, const GLint * v2);
		extern void (CODEGEN_FUNCPTR *glRects)(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
		extern void (CODEGEN_FUNCPTR *glRectsv)(const GLshort * v1, const GLshort * v2);
		extern GLint (CODEGEN_FUNCPTR *glRenderMode)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *glRotated)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
		extern void (CODEGEN_FUNCPTR *glRotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
		extern void (CODEGEN_FUNCPTR *glScaled)(GLdouble x, GLdouble y, GLdouble z);
		extern void (CODEGEN_FUNCPTR *glScalef)(GLfloat x, GLfloat y, GLfloat z);
		extern void (CODEGEN_FUNCPTR *glScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *glSelectBuffer)(GLsizei size, GLuint * buffer);
		extern void (CODEGEN_FUNCPTR *glShadeModel)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *glStencilFunc)(GLenum func, GLint ref, GLuint mask);
		extern void (CODEGEN_FUNCPTR *glStencilMask)(GLuint mask);
		extern void (CODEGEN_FUNCPTR *glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
		extern void (CODEGEN_FUNCPTR *glTexCoord1d)(GLdouble s);
		extern void (CODEGEN_FUNCPTR *glTexCoord1dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glTexCoord1f)(GLfloat s);
		extern void (CODEGEN_FUNCPTR *glTexCoord1fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glTexCoord1i)(GLint s);
		extern void (CODEGEN_FUNCPTR *glTexCoord1iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *glTexCoord1s)(GLshort s);
		extern void (CODEGEN_FUNCPTR *glTexCoord1sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glTexCoord2d)(GLdouble s, GLdouble t);
		extern void (CODEGEN_FUNCPTR *glTexCoord2dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glTexCoord2f)(GLfloat s, GLfloat t);
		extern void (CODEGEN_FUNCPTR *glTexCoord2fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glTexCoord2i)(GLint s, GLint t);
		extern void (CODEGEN_FUNCPTR *glTexCoord2iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *glTexCoord2s)(GLshort s, GLshort t);
		extern void (CODEGEN_FUNCPTR *glTexCoord2sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glTexCoord3d)(GLdouble s, GLdouble t, GLdouble r);
		extern void (CODEGEN_FUNCPTR *glTexCoord3dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glTexCoord3f)(GLfloat s, GLfloat t, GLfloat r);
		extern void (CODEGEN_FUNCPTR *glTexCoord3fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glTexCoord3i)(GLint s, GLint t, GLint r);
		extern void (CODEGEN_FUNCPTR *glTexCoord3iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *glTexCoord3s)(GLshort s, GLshort t, GLshort r);
		extern void (CODEGEN_FUNCPTR *glTexCoord3sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glTexCoord4d)(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
		extern void (CODEGEN_FUNCPTR *glTexCoord4dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glTexCoord4f)(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
		extern void (CODEGEN_FUNCPTR *glTexCoord4fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glTexCoord4i)(GLint s, GLint t, GLint r, GLint q);
		extern void (CODEGEN_FUNCPTR *glTexCoord4iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *glTexCoord4s)(GLshort s, GLshort t, GLshort r, GLshort q);
		extern void (CODEGEN_FUNCPTR *glTexCoord4sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glTexEnvf)(GLenum target, GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *glTexEnvfv)(GLenum target, GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glTexEnvi)(GLenum target, GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *glTexEnviv)(GLenum target, GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *glTexGend)(GLenum coord, GLenum pname, GLdouble param);
		extern void (CODEGEN_FUNCPTR *glTexGendv)(GLenum coord, GLenum pname, const GLdouble * params);
		extern void (CODEGEN_FUNCPTR *glTexGenf)(GLenum coord, GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *glTexGenfv)(GLenum coord, GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glTexGeni)(GLenum coord, GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *glTexGeniv)(GLenum coord, GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *glTexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *glTexParameterf)(GLenum target, GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *glTexParameterfv)(GLenum target, GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glTexParameteri)(GLenum target, GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *glTexParameteriv)(GLenum target, GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *glTranslated)(GLdouble x, GLdouble y, GLdouble z);
		extern void (CODEGEN_FUNCPTR *glTranslatef)(GLfloat x, GLfloat y, GLfloat z);
		extern void (CODEGEN_FUNCPTR *glVertex2d)(GLdouble x, GLdouble y);
		extern void (CODEGEN_FUNCPTR *glVertex2dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glVertex2f)(GLfloat x, GLfloat y);
		extern void (CODEGEN_FUNCPTR *glVertex2fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glVertex2i)(GLint x, GLint y);
		extern void (CODEGEN_FUNCPTR *glVertex2iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *glVertex2s)(GLshort x, GLshort y);
		extern void (CODEGEN_FUNCPTR *glVertex2sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glVertex3d)(GLdouble x, GLdouble y, GLdouble z);
		extern void (CODEGEN_FUNCPTR *glVertex3dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glVertex3f)(GLfloat x, GLfloat y, GLfloat z);
		extern void (CODEGEN_FUNCPTR *glVertex3fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glVertex3i)(GLint x, GLint y, GLint z);
		extern void (CODEGEN_FUNCPTR *glVertex3iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *glVertex3s)(GLshort x, GLshort y, GLshort z);
		extern void (CODEGEN_FUNCPTR *glVertex3sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glVertex4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
		extern void (CODEGEN_FUNCPTR *glVertex4dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glVertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
		extern void (CODEGEN_FUNCPTR *glVertex4fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glVertex4i)(GLint x, GLint y, GLint z, GLint w);
		extern void (CODEGEN_FUNCPTR *glVertex4iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *glVertex4s)(GLshort x, GLshort y, GLshort z, GLshort w);
		extern void (CODEGEN_FUNCPTR *glVertex4sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
		
		extern GLboolean (CODEGEN_FUNCPTR *glAreTexturesResident)(GLsizei n, const GLuint * textures, GLboolean * residences);
		extern void (CODEGEN_FUNCPTR *glArrayElement)(GLint i);
		extern void (CODEGEN_FUNCPTR *glBindTexture)(GLenum target, GLuint texture);
		extern void (CODEGEN_FUNCPTR *glColorPointer)(GLint size, GLenum type, GLsizei stride, const void * pointer);
		extern void (CODEGEN_FUNCPTR *glCopyTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
		extern void (CODEGEN_FUNCPTR *glCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
		extern void (CODEGEN_FUNCPTR *glCopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
		extern void (CODEGEN_FUNCPTR *glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *glDeleteTextures)(GLsizei n, const GLuint * textures);
		extern void (CODEGEN_FUNCPTR *glDisableClientState)(GLenum ren_array);
		extern void (CODEGEN_FUNCPTR *glDrawArrays)(GLenum mode, GLint first, GLsizei count);
		extern void (CODEGEN_FUNCPTR *glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void * indices);
		extern void (CODEGEN_FUNCPTR *glEdgeFlagPointer)(GLsizei stride, const void * pointer);
		extern void (CODEGEN_FUNCPTR *glEnableClientState)(GLenum ren_array);
		extern void (CODEGEN_FUNCPTR *glGenTextures)(GLsizei n, GLuint * textures);
		extern void (CODEGEN_FUNCPTR *glGetPointerv)(GLenum pname, void ** params);
		extern void (CODEGEN_FUNCPTR *glIndexPointer)(GLenum type, GLsizei stride, const void * pointer);
		extern void (CODEGEN_FUNCPTR *glIndexub)(GLubyte c);
		extern void (CODEGEN_FUNCPTR *glIndexubv)(const GLubyte * c);
		extern void (CODEGEN_FUNCPTR *glInterleavedArrays)(GLenum format, GLsizei stride, const void * pointer);
		extern GLboolean (CODEGEN_FUNCPTR *glIsTexture)(GLuint texture);
		extern void (CODEGEN_FUNCPTR *glNormalPointer)(GLenum type, GLsizei stride, const void * pointer);
		extern void (CODEGEN_FUNCPTR *glPolygonOffset)(GLfloat factor, GLfloat units);
		extern void (CODEGEN_FUNCPTR *glPopClientAttrib)(void);
		extern void (CODEGEN_FUNCPTR *glPrioritizeTextures)(GLsizei n, const GLuint * textures, const GLfloat * priorities);
		extern void (CODEGEN_FUNCPTR *glPushClientAttrib)(GLbitfield mask);
		extern void (CODEGEN_FUNCPTR *glTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const void * pointer);
		extern void (CODEGEN_FUNCPTR *glTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *glVertexPointer)(GLint size, GLenum type, GLsizei stride, const void * pointer);
		
		extern void (CODEGEN_FUNCPTR *glCopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *glDrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void * indices);
		extern void (CODEGEN_FUNCPTR *glTexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *glTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void * pixels);
		
		extern void (CODEGEN_FUNCPTR *glActiveTexture)(GLenum texture);
		extern void (CODEGEN_FUNCPTR *glClientActiveTexture)(GLenum texture);
		extern void (CODEGEN_FUNCPTR *glCompressedTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *glCompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *glCompressedTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *glCompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *glGetCompressedTexImage)(GLenum target, GLint level, void * img);
		extern void (CODEGEN_FUNCPTR *glLoadTransposeMatrixd)(const GLdouble * m);
		extern void (CODEGEN_FUNCPTR *glLoadTransposeMatrixf)(const GLfloat * m);
		extern void (CODEGEN_FUNCPTR *glMultTransposeMatrixd)(const GLdouble * m);
		extern void (CODEGEN_FUNCPTR *glMultTransposeMatrixf)(const GLfloat * m);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord1d)(GLenum target, GLdouble s);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord1dv)(GLenum target, const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord1f)(GLenum target, GLfloat s);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord1fv)(GLenum target, const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord1i)(GLenum target, GLint s);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord1iv)(GLenum target, const GLint * v);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord1s)(GLenum target, GLshort s);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord1sv)(GLenum target, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord2d)(GLenum target, GLdouble s, GLdouble t);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord2dv)(GLenum target, const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord2f)(GLenum target, GLfloat s, GLfloat t);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord2fv)(GLenum target, const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord2i)(GLenum target, GLint s, GLint t);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord2iv)(GLenum target, const GLint * v);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord2s)(GLenum target, GLshort s, GLshort t);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord2sv)(GLenum target, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord3d)(GLenum target, GLdouble s, GLdouble t, GLdouble r);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord3dv)(GLenum target, const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord3f)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord3fv)(GLenum target, const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord3i)(GLenum target, GLint s, GLint t, GLint r);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord3iv)(GLenum target, const GLint * v);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord3s)(GLenum target, GLshort s, GLshort t, GLshort r);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord3sv)(GLenum target, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord4d)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord4dv)(GLenum target, const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord4f)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord4fv)(GLenum target, const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord4i)(GLenum target, GLint s, GLint t, GLint r, GLint q);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord4iv)(GLenum target, const GLint * v);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord4s)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
		extern void (CODEGEN_FUNCPTR *glMultiTexCoord4sv)(GLenum target, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glSampleCoverage)(GLfloat value, GLboolean invert);
		
		extern void (CODEGEN_FUNCPTR *glBlendColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		extern void (CODEGEN_FUNCPTR *glBlendEquation)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *glBlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
		extern void (CODEGEN_FUNCPTR *glFogCoordPointer)(GLenum type, GLsizei stride, const void * pointer);
		extern void (CODEGEN_FUNCPTR *glFogCoordd)(GLdouble coord);
		extern void (CODEGEN_FUNCPTR *glFogCoorddv)(const GLdouble * coord);
		extern void (CODEGEN_FUNCPTR *glFogCoordf)(GLfloat coord);
		extern void (CODEGEN_FUNCPTR *glFogCoordfv)(const GLfloat * coord);
		extern void (CODEGEN_FUNCPTR *glMultiDrawArrays)(GLenum mode, const GLint * first, const GLsizei * count, GLsizei drawcount);
		extern void (CODEGEN_FUNCPTR *glMultiDrawElements)(GLenum mode, const GLsizei * count, GLenum type, const void *const* indices, GLsizei drawcount);
		extern void (CODEGEN_FUNCPTR *glPointParameterf)(GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *glPointParameterfv)(GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glPointParameteri)(GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *glPointParameteriv)(GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *glSecondaryColor3b)(GLbyte red, GLbyte green, GLbyte blue);
		extern void (CODEGEN_FUNCPTR *glSecondaryColor3bv)(const GLbyte * v);
		extern void (CODEGEN_FUNCPTR *glSecondaryColor3d)(GLdouble red, GLdouble green, GLdouble blue);
		extern void (CODEGEN_FUNCPTR *glSecondaryColor3dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glSecondaryColor3f)(GLfloat red, GLfloat green, GLfloat blue);
		extern void (CODEGEN_FUNCPTR *glSecondaryColor3fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glSecondaryColor3i)(GLint red, GLint green, GLint blue);
		extern void (CODEGEN_FUNCPTR *glSecondaryColor3iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *glSecondaryColor3s)(GLshort red, GLshort green, GLshort blue);
		extern void (CODEGEN_FUNCPTR *glSecondaryColor3sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glSecondaryColor3ub)(GLubyte red, GLubyte green, GLubyte blue);
		extern void (CODEGEN_FUNCPTR *glSecondaryColor3ubv)(const GLubyte * v);
		extern void (CODEGEN_FUNCPTR *glSecondaryColor3ui)(GLuint red, GLuint green, GLuint blue);
		extern void (CODEGEN_FUNCPTR *glSecondaryColor3uiv)(const GLuint * v);
		extern void (CODEGEN_FUNCPTR *glSecondaryColor3us)(GLushort red, GLushort green, GLushort blue);
		extern void (CODEGEN_FUNCPTR *glSecondaryColor3usv)(const GLushort * v);
		extern void (CODEGEN_FUNCPTR *glSecondaryColorPointer)(GLint size, GLenum type, GLsizei stride, const void * pointer);
		extern void (CODEGEN_FUNCPTR *glWindowPos2d)(GLdouble x, GLdouble y);
		extern void (CODEGEN_FUNCPTR *glWindowPos2dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glWindowPos2f)(GLfloat x, GLfloat y);
		extern void (CODEGEN_FUNCPTR *glWindowPos2fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glWindowPos2i)(GLint x, GLint y);
		extern void (CODEGEN_FUNCPTR *glWindowPos2iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *glWindowPos2s)(GLshort x, GLshort y);
		extern void (CODEGEN_FUNCPTR *glWindowPos2sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glWindowPos3d)(GLdouble x, GLdouble y, GLdouble z);
		extern void (CODEGEN_FUNCPTR *glWindowPos3dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glWindowPos3f)(GLfloat x, GLfloat y, GLfloat z);
		extern void (CODEGEN_FUNCPTR *glWindowPos3fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glWindowPos3i)(GLint x, GLint y, GLint z);
		extern void (CODEGEN_FUNCPTR *glWindowPos3iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *glWindowPos3s)(GLshort x, GLshort y, GLshort z);
		extern void (CODEGEN_FUNCPTR *glWindowPos3sv)(const GLshort * v);
		
		extern void (CODEGEN_FUNCPTR *glBeginQuery)(GLenum target, GLuint id);
		extern void (CODEGEN_FUNCPTR *glBindBuffer)(GLenum target, GLuint buffer);
		extern void (CODEGEN_FUNCPTR *glBufferData)(GLenum target, GLsizeiptr size, const void * data, GLenum usage);
		extern void (CODEGEN_FUNCPTR *glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const void * data);
		extern void (CODEGEN_FUNCPTR *glDeleteBuffers)(GLsizei n, const GLuint * buffers);
		extern void (CODEGEN_FUNCPTR *glDeleteQueries)(GLsizei n, const GLuint * ids);
		extern void (CODEGEN_FUNCPTR *glEndQuery)(GLenum target);
		extern void (CODEGEN_FUNCPTR *glGenBuffers)(GLsizei n, GLuint * buffers);
		extern void (CODEGEN_FUNCPTR *glGenQueries)(GLsizei n, GLuint * ids);
		extern void (CODEGEN_FUNCPTR *glGetBufferParameteriv)(GLenum target, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetBufferPointerv)(GLenum target, GLenum pname, void ** params);
		extern void (CODEGEN_FUNCPTR *glGetBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, void * data);
		extern void (CODEGEN_FUNCPTR *glGetQueryObjectiv)(GLuint id, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetQueryObjectuiv)(GLuint id, GLenum pname, GLuint * params);
		extern void (CODEGEN_FUNCPTR *glGetQueryiv)(GLenum target, GLenum pname, GLint * params);
		extern GLboolean (CODEGEN_FUNCPTR *glIsBuffer)(GLuint buffer);
		extern GLboolean (CODEGEN_FUNCPTR *glIsQuery)(GLuint id);
		extern void * (CODEGEN_FUNCPTR *glMapBuffer)(GLenum target, GLenum access);
		extern GLboolean (CODEGEN_FUNCPTR *glUnmapBuffer)(GLenum target);
		
		extern void (CODEGEN_FUNCPTR *glAttachShader)(GLuint program, GLuint shader);
		extern void (CODEGEN_FUNCPTR *glBindAttribLocation)(GLuint program, GLuint index, const GLchar * name);
		extern void (CODEGEN_FUNCPTR *glBlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha);
		extern void (CODEGEN_FUNCPTR *glCompileShader)(GLuint shader);
		extern GLuint (CODEGEN_FUNCPTR *glCreateProgram)(void);
		extern GLuint (CODEGEN_FUNCPTR *glCreateShader)(GLenum type);
		extern void (CODEGEN_FUNCPTR *glDeleteProgram)(GLuint program);
		extern void (CODEGEN_FUNCPTR *glDeleteShader)(GLuint shader);
		extern void (CODEGEN_FUNCPTR *glDetachShader)(GLuint program, GLuint shader);
		extern void (CODEGEN_FUNCPTR *glDisableVertexAttribArray)(GLuint index);
		extern void (CODEGEN_FUNCPTR *glDrawBuffers)(GLsizei n, const GLenum * bufs);
		extern void (CODEGEN_FUNCPTR *glEnableVertexAttribArray)(GLuint index);
		extern void (CODEGEN_FUNCPTR *glGetActiveAttrib)(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name);
		extern void (CODEGEN_FUNCPTR *glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name);
		extern void (CODEGEN_FUNCPTR *glGetAttachedShaders)(GLuint program, GLsizei maxCount, GLsizei * count, GLuint * shaders);
		extern GLint (CODEGEN_FUNCPTR *glGetAttribLocation)(GLuint program, const GLchar * name);
		extern void (CODEGEN_FUNCPTR *glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei * length, GLchar * infoLog);
		extern void (CODEGEN_FUNCPTR *glGetProgramiv)(GLuint program, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * infoLog);
		extern void (CODEGEN_FUNCPTR *glGetShaderSource)(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * source);
		extern void (CODEGEN_FUNCPTR *glGetShaderiv)(GLuint shader, GLenum pname, GLint * params);
		extern GLint (CODEGEN_FUNCPTR *glGetUniformLocation)(GLuint program, const GLchar * name);
		extern void (CODEGEN_FUNCPTR *glGetUniformfv)(GLuint program, GLint location, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glGetUniformiv)(GLuint program, GLint location, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetVertexAttribPointerv)(GLuint index, GLenum pname, void ** pointer);
		extern void (CODEGEN_FUNCPTR *glGetVertexAttribdv)(GLuint index, GLenum pname, GLdouble * params);
		extern void (CODEGEN_FUNCPTR *glGetVertexAttribfv)(GLuint index, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glGetVertexAttribiv)(GLuint index, GLenum pname, GLint * params);
		extern GLboolean (CODEGEN_FUNCPTR *glIsProgram)(GLuint program);
		extern GLboolean (CODEGEN_FUNCPTR *glIsShader)(GLuint shader);
		extern void (CODEGEN_FUNCPTR *glLinkProgram)(GLuint program);
		extern void (CODEGEN_FUNCPTR *glShaderSource)(GLuint shader, GLsizei count, const GLchar *const* string, const GLint * length);
		extern void (CODEGEN_FUNCPTR *glStencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask);
		extern void (CODEGEN_FUNCPTR *glStencilMaskSeparate)(GLenum face, GLuint mask);
		extern void (CODEGEN_FUNCPTR *glStencilOpSeparate)(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
		extern void (CODEGEN_FUNCPTR *glUniform1f)(GLint location, GLfloat v0);
		extern void (CODEGEN_FUNCPTR *glUniform1fv)(GLint location, GLsizei count, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glUniform1i)(GLint location, GLint v0);
		extern void (CODEGEN_FUNCPTR *glUniform1iv)(GLint location, GLsizei count, const GLint * value);
		extern void (CODEGEN_FUNCPTR *glUniform2f)(GLint location, GLfloat v0, GLfloat v1);
		extern void (CODEGEN_FUNCPTR *glUniform2fv)(GLint location, GLsizei count, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glUniform2i)(GLint location, GLint v0, GLint v1);
		extern void (CODEGEN_FUNCPTR *glUniform2iv)(GLint location, GLsizei count, const GLint * value);
		extern void (CODEGEN_FUNCPTR *glUniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
		extern void (CODEGEN_FUNCPTR *glUniform3fv)(GLint location, GLsizei count, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glUniform3i)(GLint location, GLint v0, GLint v1, GLint v2);
		extern void (CODEGEN_FUNCPTR *glUniform3iv)(GLint location, GLsizei count, const GLint * value);
		extern void (CODEGEN_FUNCPTR *glUniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
		extern void (CODEGEN_FUNCPTR *glUniform4fv)(GLint location, GLsizei count, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glUniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
		extern void (CODEGEN_FUNCPTR *glUniform4iv)(GLint location, GLsizei count, const GLint * value);
		extern void (CODEGEN_FUNCPTR *glUniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glUseProgram)(GLuint program);
		extern void (CODEGEN_FUNCPTR *glValidateProgram)(GLuint program);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib1d)(GLuint index, GLdouble x);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib1dv)(GLuint index, const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib1f)(GLuint index, GLfloat x);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib1fv)(GLuint index, const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib1s)(GLuint index, GLshort x);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib1sv)(GLuint index, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib2d)(GLuint index, GLdouble x, GLdouble y);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib2dv)(GLuint index, const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib2f)(GLuint index, GLfloat x, GLfloat y);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib2fv)(GLuint index, const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib2s)(GLuint index, GLshort x, GLshort y);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib2sv)(GLuint index, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib3dv)(GLuint index, const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib3f)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib3fv)(GLuint index, const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib3s)(GLuint index, GLshort x, GLshort y, GLshort z);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib3sv)(GLuint index, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4Nbv)(GLuint index, const GLbyte * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4Niv)(GLuint index, const GLint * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4Nsv)(GLuint index, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4Nub)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4Nubv)(GLuint index, const GLubyte * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4Nuiv)(GLuint index, const GLuint * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4Nusv)(GLuint index, const GLushort * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4bv)(GLuint index, const GLbyte * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4dv)(GLuint index, const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4f)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4fv)(GLuint index, const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4iv)(GLuint index, const GLint * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4s)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4sv)(GLuint index, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4ubv)(GLuint index, const GLubyte * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4uiv)(GLuint index, const GLuint * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttrib4usv)(GLuint index, const GLushort * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer);
		
		extern void (CODEGEN_FUNCPTR *glUniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glUniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glUniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glUniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glUniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glUniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		
		namespace sys
		{
			
			exts::LoadTest LoadFunctions();
			
			int GetMinorVersion();
			int GetMajorVersion();
			bool IsVersionGEQ(int majorVersion, int minorVersion);
			
		} //namespace sys
	} //namespace gl
} //namespace gl21
#endif //GL21_PIONEER_GENERATED_HEADEROPENGL_HPP
