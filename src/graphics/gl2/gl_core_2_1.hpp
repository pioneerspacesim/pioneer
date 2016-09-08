#ifndef GL21_POINTER_CPP_GENERATED_HEADEROPENGL_HPP
#define GL21_POINTER_CPP_GENERATED_HEADEROPENGL_HPP

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
			TEXTURE_CUBE_MAP_SEAMLESS        = 0x884F,
			
			//TEXTURE_CUBE_MAP_SEAMLESS taken from ext: ARB_seamless_cube_map
			
			ACTIVE_UNIFORM_BLOCKS            = 0x8A36,
			ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH = 0x8A35,
			INVALID_INDEX                    = 0xFFFFFFFF,
			MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS = 0x8A33,
			MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS = 0x8A32,
			MAX_COMBINED_UNIFORM_BLOCKS      = 0x8A2E,
			MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS = 0x8A31,
			MAX_FRAGMENT_UNIFORM_BLOCKS      = 0x8A2D,
			MAX_GEOMETRY_UNIFORM_BLOCKS      = 0x8A2C,
			MAX_UNIFORM_BLOCK_SIZE           = 0x8A30,
			MAX_UNIFORM_BUFFER_BINDINGS      = 0x8A2F,
			MAX_VERTEX_UNIFORM_BLOCKS        = 0x8A2B,
			UNIFORM_ARRAY_STRIDE             = 0x8A3C,
			UNIFORM_BLOCK_ACTIVE_UNIFORMS    = 0x8A42,
			UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES = 0x8A43,
			UNIFORM_BLOCK_BINDING            = 0x8A3F,
			UNIFORM_BLOCK_DATA_SIZE          = 0x8A40,
			UNIFORM_BLOCK_INDEX              = 0x8A3A,
			UNIFORM_BLOCK_NAME_LENGTH        = 0x8A41,
			UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER = 0x8A46,
			UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER = 0x8A45,
			UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER = 0x8A44,
			UNIFORM_BUFFER                   = 0x8A11,
			UNIFORM_BUFFER_BINDING           = 0x8A28,
			UNIFORM_BUFFER_OFFSET_ALIGNMENT  = 0x8A34,
			UNIFORM_BUFFER_SIZE              = 0x8A2A,
			UNIFORM_BUFFER_START             = 0x8A29,
			UNIFORM_IS_ROW_MAJOR             = 0x8A3E,
			UNIFORM_MATRIX_STRIDE            = 0x8A3D,
			UNIFORM_NAME_LENGTH              = 0x8A39,
			UNIFORM_OFFSET                   = 0x8A3B,
			UNIFORM_SIZE                     = 0x8A38,
			UNIFORM_TYPE                     = 0x8A37,
			
			VERTEX_ATTRIB_ARRAY_DIVISOR_ARB  = 0x88FE,
			
			VERTEX_ARRAY_BINDING             = 0x85B5,
			
			COLOR_ATTACHMENT0_EXT            = 0x8CE0,
			COLOR_ATTACHMENT10_EXT           = 0x8CEA,
			COLOR_ATTACHMENT11_EXT           = 0x8CEB,
			COLOR_ATTACHMENT12_EXT           = 0x8CEC,
			COLOR_ATTACHMENT13_EXT           = 0x8CED,
			COLOR_ATTACHMENT14_EXT           = 0x8CEE,
			COLOR_ATTACHMENT15_EXT           = 0x8CEF,
			COLOR_ATTACHMENT1_EXT            = 0x8CE1,
			COLOR_ATTACHMENT2_EXT            = 0x8CE2,
			COLOR_ATTACHMENT3_EXT            = 0x8CE3,
			COLOR_ATTACHMENT4_EXT            = 0x8CE4,
			COLOR_ATTACHMENT5_EXT            = 0x8CE5,
			COLOR_ATTACHMENT6_EXT            = 0x8CE6,
			COLOR_ATTACHMENT7_EXT            = 0x8CE7,
			COLOR_ATTACHMENT8_EXT            = 0x8CE8,
			COLOR_ATTACHMENT9_EXT            = 0x8CE9,
			DEPTH_ATTACHMENT_EXT             = 0x8D00,
			FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT = 0x8CD1,
			FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT = 0x8CD0,
			FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT = 0x8CD4,
			FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT = 0x8CD3,
			FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT = 0x8CD2,
			FRAMEBUFFER_BINDING_EXT          = 0x8CA6,
			FRAMEBUFFER_COMPLETE_EXT         = 0x8CD5,
			FRAMEBUFFER_EXT                  = 0x8D40,
			FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT = 0x8CD6,
			FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT = 0x8CD9,
			FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT = 0x8CDB,
			FRAMEBUFFER_INCOMPLETE_FORMATS_EXT = 0x8CDA,
			FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT = 0x8CD7,
			FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT = 0x8CDC,
			FRAMEBUFFER_UNSUPPORTED_EXT      = 0x8CDD,
			INVALID_FRAMEBUFFER_OPERATION_EXT = 0x0506,
			MAX_COLOR_ATTACHMENTS_EXT        = 0x8CDF,
			MAX_RENDERBUFFER_SIZE_EXT        = 0x84E8,
			RENDERBUFFER_ALPHA_SIZE_EXT      = 0x8D53,
			RENDERBUFFER_BINDING_EXT         = 0x8CA7,
			RENDERBUFFER_BLUE_SIZE_EXT       = 0x8D52,
			RENDERBUFFER_DEPTH_SIZE_EXT      = 0x8D54,
			RENDERBUFFER_EXT                 = 0x8D41,
			RENDERBUFFER_GREEN_SIZE_EXT      = 0x8D51,
			RENDERBUFFER_HEIGHT_EXT          = 0x8D43,
			RENDERBUFFER_INTERNAL_FORMAT_EXT = 0x8D44,
			RENDERBUFFER_RED_SIZE_EXT        = 0x8D50,
			RENDERBUFFER_STENCIL_SIZE_EXT    = 0x8D55,
			RENDERBUFFER_WIDTH_EXT           = 0x8D42,
			STENCIL_ATTACHMENT_EXT           = 0x8D20,
			STENCIL_INDEX16_EXT              = 0x8D49,
			STENCIL_INDEX1_EXT               = 0x8D46,
			STENCIL_INDEX4_EXT               = 0x8D47,
			STENCIL_INDEX8_EXT               = 0x8D48,
			
			COMPRESSED_RGBA_S3TC_DXT1_EXT    = 0x83F1,
			COMPRESSED_RGBA_S3TC_DXT3_EXT    = 0x83F2,
			COMPRESSED_RGBA_S3TC_DXT5_EXT    = 0x83F3,
			COMPRESSED_RGB_S3TC_DXT1_EXT     = 0x83F0,
			
			COMPRESSED_SLUMINANCE_ALPHA_EXT  = 0x8C4B,
			COMPRESSED_SLUMINANCE_EXT        = 0x8C4A,
			COMPRESSED_SRGB_ALPHA_EXT        = 0x8C49,
			COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT = 0x8C4D,
			COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT = 0x8C4E,
			COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT = 0x8C4F,
			COMPRESSED_SRGB_EXT              = 0x8C48,
			COMPRESSED_SRGB_S3TC_DXT1_EXT    = 0x8C4C,
			SLUMINANCE8_ALPHA8_EXT           = 0x8C45,
			SLUMINANCE8_EXT                  = 0x8C47,
			SLUMINANCE_ALPHA_EXT             = 0x8C44,
			SLUMINANCE_EXT                   = 0x8C46,
			SRGB8_ALPHA8_EXT                 = 0x8C43,
			SRGB8_EXT                        = 0x8C41,
			SRGB_ALPHA_EXT                   = 0x8C42,
			SRGB_EXT                         = 0x8C40,
			
			MAX_TEXTURE_MAX_ANISOTROPY_EXT   = 0x84FF,
			TEXTURE_MAX_ANISOTROPY_EXT       = 0x84FE,
			
			_2D                              = 0x0600,
			_2_BYTES                         = 0x1407,
			_3D                              = 0x0601,
			_3D_COLOR                        = 0x0602,
			_3D_COLOR_TEXTURE                = 0x0603,
			_3_BYTES                         = 0x1408,
			_4D_COLOR_TEXTURE                = 0x0604,
			_4_BYTES                         = 0x1409,
			ACCUM                            = 0x0100,
			ACCUM_ALPHA_BITS                 = 0x0D5B,
			ACCUM_BLUE_BITS                  = 0x0D5A,
			ACCUM_BUFFER_BIT                 = 0x00000200,
			ACCUM_CLEAR_VALUE                = 0x0B80,
			ACCUM_GREEN_BITS                 = 0x0D59,
			ACCUM_RED_BITS                   = 0x0D58,
			ADD                              = 0x0104,
			ALL_ATTRIB_BITS                  = 0xFFFFFFFF,
			ALPHA                            = 0x1906,
			ALPHA12                          = 0x803D,
			ALPHA16                          = 0x803E,
			ALPHA4                           = 0x803B,
			ALPHA8                           = 0x803C,
			ALPHA_BIAS                       = 0x0D1D,
			ALPHA_BITS                       = 0x0D55,
			ALPHA_SCALE                      = 0x0D1C,
			ALPHA_TEST                       = 0x0BC0,
			ALPHA_TEST_FUNC                  = 0x0BC1,
			ALPHA_TEST_REF                   = 0x0BC2,
			ALWAYS                           = 0x0207,
			AMBIENT                          = 0x1200,
			AMBIENT_AND_DIFFUSE              = 0x1602,
			AND                              = 0x1501,
			AND_INVERTED                     = 0x1504,
			AND_REVERSE                      = 0x1502,
			ATTRIB_STACK_DEPTH               = 0x0BB0,
			AUTO_NORMAL                      = 0x0D80,
			AUX0                             = 0x0409,
			AUX1                             = 0x040A,
			AUX2                             = 0x040B,
			AUX3                             = 0x040C,
			AUX_BUFFERS                      = 0x0C00,
			BACK                             = 0x0405,
			BACK_LEFT                        = 0x0402,
			BACK_RIGHT                       = 0x0403,
			BITMAP                           = 0x1A00,
			BITMAP_TOKEN                     = 0x0704,
			BLEND                            = 0x0BE2,
			BLEND_DST                        = 0x0BE0,
			BLEND_SRC                        = 0x0BE1,
			BLUE                             = 0x1905,
			BLUE_BIAS                        = 0x0D1B,
			BLUE_BITS                        = 0x0D54,
			BLUE_SCALE                       = 0x0D1A,
			BYTE                             = 0x1400,
			C3F_V3F                          = 0x2A24,
			C4F_N3F_V3F                      = 0x2A26,
			C4UB_V2F                         = 0x2A22,
			C4UB_V3F                         = 0x2A23,
			CCW                              = 0x0901,
			CLAMP                            = 0x2900,
			CLEAR                            = 0x1500,
			CLIENT_ALL_ATTRIB_BITS           = 0xFFFFFFFF,
			CLIENT_ATTRIB_STACK_DEPTH        = 0x0BB1,
			CLIENT_PIXEL_STORE_BIT           = 0x00000001,
			CLIENT_VERTEX_ARRAY_BIT          = 0x00000002,
			CLIP_PLANE0                      = 0x3000,
			CLIP_PLANE1                      = 0x3001,
			CLIP_PLANE2                      = 0x3002,
			CLIP_PLANE3                      = 0x3003,
			CLIP_PLANE4                      = 0x3004,
			CLIP_PLANE5                      = 0x3005,
			COEFF                            = 0x0A00,
			COLOR                            = 0x1800,
			COLOR_ARRAY                      = 0x8076,
			COLOR_ARRAY_POINTER              = 0x8090,
			COLOR_ARRAY_SIZE                 = 0x8081,
			COLOR_ARRAY_STRIDE               = 0x8083,
			COLOR_ARRAY_TYPE                 = 0x8082,
			COLOR_BUFFER_BIT                 = 0x00004000,
			COLOR_CLEAR_VALUE                = 0x0C22,
			COLOR_INDEX                      = 0x1900,
			COLOR_INDEXES                    = 0x1603,
			COLOR_LOGIC_OP                   = 0x0BF2,
			COLOR_MATERIAL                   = 0x0B57,
			COLOR_MATERIAL_FACE              = 0x0B55,
			COLOR_MATERIAL_PARAMETER         = 0x0B56,
			COLOR_WRITEMASK                  = 0x0C23,
			COMPILE                          = 0x1300,
			COMPILE_AND_EXECUTE              = 0x1301,
			CONSTANT_ATTENUATION             = 0x1207,
			COPY                             = 0x1503,
			COPY_INVERTED                    = 0x150C,
			COPY_PIXEL_TOKEN                 = 0x0706,
			CULL_FACE                        = 0x0B44,
			CULL_FACE_MODE                   = 0x0B45,
			CURRENT_BIT                      = 0x00000001,
			CURRENT_COLOR                    = 0x0B00,
			CURRENT_INDEX                    = 0x0B01,
			CURRENT_NORMAL                   = 0x0B02,
			CURRENT_RASTER_COLOR             = 0x0B04,
			CURRENT_RASTER_DISTANCE          = 0x0B09,
			CURRENT_RASTER_INDEX             = 0x0B05,
			CURRENT_RASTER_POSITION          = 0x0B07,
			CURRENT_RASTER_POSITION_VALID    = 0x0B08,
			CURRENT_RASTER_TEXTURE_COORDS    = 0x0B06,
			CURRENT_TEXTURE_COORDS           = 0x0B03,
			CW                               = 0x0900,
			DECAL                            = 0x2101,
			DECR                             = 0x1E03,
			DEPTH                            = 0x1801,
			DEPTH_BIAS                       = 0x0D1F,
			DEPTH_BITS                       = 0x0D56,
			DEPTH_BUFFER_BIT                 = 0x00000100,
			DEPTH_CLEAR_VALUE                = 0x0B73,
			DEPTH_COMPONENT                  = 0x1902,
			DEPTH_FUNC                       = 0x0B74,
			DEPTH_RANGE                      = 0x0B70,
			DEPTH_SCALE                      = 0x0D1E,
			DEPTH_TEST                       = 0x0B71,
			DEPTH_WRITEMASK                  = 0x0B72,
			DIFFUSE                          = 0x1201,
			DITHER                           = 0x0BD0,
			DOMAIN_                          = 0x0A02,
			DONT_CARE                        = 0x1100,
			DOUBLE                           = 0x140A,
			DOUBLEBUFFER                     = 0x0C32,
			DRAW_BUFFER                      = 0x0C01,
			DRAW_PIXEL_TOKEN                 = 0x0705,
			DST_ALPHA                        = 0x0304,
			DST_COLOR                        = 0x0306,
			EDGE_FLAG                        = 0x0B43,
			EDGE_FLAG_ARRAY                  = 0x8079,
			EDGE_FLAG_ARRAY_POINTER          = 0x8093,
			EDGE_FLAG_ARRAY_STRIDE           = 0x808C,
			EMISSION                         = 0x1600,
			ENABLE_BIT                       = 0x00002000,
			EQUAL                            = 0x0202,
			EQUIV                            = 0x1509,
			EVAL_BIT                         = 0x00010000,
			EXP                              = 0x0800,
			EXP2                             = 0x0801,
			EXTENSIONS                       = 0x1F03,
			EYE_LINEAR                       = 0x2400,
			EYE_PLANE                        = 0x2502,
			FALSE_                           = 0,
			FASTEST                          = 0x1101,
			FEEDBACK                         = 0x1C01,
			FEEDBACK_BUFFER_POINTER          = 0x0DF0,
			FEEDBACK_BUFFER_SIZE             = 0x0DF1,
			FEEDBACK_BUFFER_TYPE             = 0x0DF2,
			FILL                             = 0x1B02,
			FLAT                             = 0x1D00,
			FLOAT                            = 0x1406,
			FOG                              = 0x0B60,
			FOG_BIT                          = 0x00000080,
			FOG_COLOR                        = 0x0B66,
			FOG_DENSITY                      = 0x0B62,
			FOG_END                          = 0x0B64,
			FOG_HINT                         = 0x0C54,
			FOG_INDEX                        = 0x0B61,
			FOG_MODE                         = 0x0B65,
			FOG_START                        = 0x0B63,
			FRONT                            = 0x0404,
			FRONT_AND_BACK                   = 0x0408,
			FRONT_FACE                       = 0x0B46,
			FRONT_LEFT                       = 0x0400,
			FRONT_RIGHT                      = 0x0401,
			GEQUAL                           = 0x0206,
			GREATER                          = 0x0204,
			GREEN                            = 0x1904,
			GREEN_BIAS                       = 0x0D19,
			GREEN_BITS                       = 0x0D53,
			GREEN_SCALE                      = 0x0D18,
			HINT_BIT                         = 0x00008000,
			INCR                             = 0x1E02,
			INDEX_ARRAY                      = 0x8077,
			INDEX_ARRAY_POINTER              = 0x8091,
			INDEX_ARRAY_STRIDE               = 0x8086,
			INDEX_ARRAY_TYPE                 = 0x8085,
			INDEX_BITS                       = 0x0D51,
			INDEX_CLEAR_VALUE                = 0x0C20,
			INDEX_LOGIC_OP                   = 0x0BF1,
			INDEX_MODE                       = 0x0C30,
			INDEX_OFFSET                     = 0x0D13,
			INDEX_SHIFT                      = 0x0D12,
			INDEX_WRITEMASK                  = 0x0C21,
			INT                              = 0x1404,
			INTENSITY                        = 0x8049,
			INTENSITY12                      = 0x804C,
			INTENSITY16                      = 0x804D,
			INTENSITY4                       = 0x804A,
			INTENSITY8                       = 0x804B,
			INVALID_ENUM                     = 0x0500,
			INVALID_OPERATION                = 0x0502,
			INVALID_VALUE                    = 0x0501,
			INVERT                           = 0x150A,
			KEEP                             = 0x1E00,
			LEFT                             = 0x0406,
			LEQUAL                           = 0x0203,
			LESS                             = 0x0201,
			LIGHT0                           = 0x4000,
			LIGHT1                           = 0x4001,
			LIGHT2                           = 0x4002,
			LIGHT3                           = 0x4003,
			LIGHT4                           = 0x4004,
			LIGHT5                           = 0x4005,
			LIGHT6                           = 0x4006,
			LIGHT7                           = 0x4007,
			LIGHTING                         = 0x0B50,
			LIGHTING_BIT                     = 0x00000040,
			LIGHT_MODEL_AMBIENT              = 0x0B53,
			LIGHT_MODEL_LOCAL_VIEWER         = 0x0B51,
			LIGHT_MODEL_TWO_SIDE             = 0x0B52,
			LINE                             = 0x1B01,
			LINEAR                           = 0x2601,
			LINEAR_ATTENUATION               = 0x1208,
			LINEAR_MIPMAP_LINEAR             = 0x2703,
			LINEAR_MIPMAP_NEAREST            = 0x2701,
			LINES                            = 0x0001,
			LINE_BIT                         = 0x00000004,
			LINE_LOOP                        = 0x0002,
			LINE_RESET_TOKEN                 = 0x0707,
			LINE_SMOOTH                      = 0x0B20,
			LINE_SMOOTH_HINT                 = 0x0C52,
			LINE_STIPPLE                     = 0x0B24,
			LINE_STIPPLE_PATTERN             = 0x0B25,
			LINE_STIPPLE_REPEAT              = 0x0B26,
			LINE_STRIP                       = 0x0003,
			LINE_TOKEN                       = 0x0702,
			LINE_WIDTH                       = 0x0B21,
			LINE_WIDTH_GRANULARITY           = 0x0B23,
			LINE_WIDTH_RANGE                 = 0x0B22,
			LIST_BASE                        = 0x0B32,
			LIST_BIT                         = 0x00020000,
			LIST_INDEX                       = 0x0B33,
			LIST_MODE                        = 0x0B30,
			LOAD                             = 0x0101,
			LOGIC_OP                         = 0x0BF1,
			LOGIC_OP_MODE                    = 0x0BF0,
			LUMINANCE                        = 0x1909,
			LUMINANCE12                      = 0x8041,
			LUMINANCE12_ALPHA12              = 0x8047,
			LUMINANCE12_ALPHA4               = 0x8046,
			LUMINANCE16                      = 0x8042,
			LUMINANCE16_ALPHA16              = 0x8048,
			LUMINANCE4                       = 0x803F,
			LUMINANCE4_ALPHA4                = 0x8043,
			LUMINANCE6_ALPHA2                = 0x8044,
			LUMINANCE8                       = 0x8040,
			LUMINANCE8_ALPHA8                = 0x8045,
			LUMINANCE_ALPHA                  = 0x190A,
			MAP1_COLOR_4                     = 0x0D90,
			MAP1_GRID_DOMAIN                 = 0x0DD0,
			MAP1_GRID_SEGMENTS               = 0x0DD1,
			MAP1_INDEX                       = 0x0D91,
			MAP1_NORMAL                      = 0x0D92,
			MAP1_TEXTURE_COORD_1             = 0x0D93,
			MAP1_TEXTURE_COORD_2             = 0x0D94,
			MAP1_TEXTURE_COORD_3             = 0x0D95,
			MAP1_TEXTURE_COORD_4             = 0x0D96,
			MAP1_VERTEX_3                    = 0x0D97,
			MAP1_VERTEX_4                    = 0x0D98,
			MAP2_COLOR_4                     = 0x0DB0,
			MAP2_GRID_DOMAIN                 = 0x0DD2,
			MAP2_GRID_SEGMENTS               = 0x0DD3,
			MAP2_INDEX                       = 0x0DB1,
			MAP2_NORMAL                      = 0x0DB2,
			MAP2_TEXTURE_COORD_1             = 0x0DB3,
			MAP2_TEXTURE_COORD_2             = 0x0DB4,
			MAP2_TEXTURE_COORD_3             = 0x0DB5,
			MAP2_TEXTURE_COORD_4             = 0x0DB6,
			MAP2_VERTEX_3                    = 0x0DB7,
			MAP2_VERTEX_4                    = 0x0DB8,
			MAP_COLOR                        = 0x0D10,
			MAP_STENCIL                      = 0x0D11,
			MATRIX_MODE                      = 0x0BA0,
			MAX_ATTRIB_STACK_DEPTH           = 0x0D35,
			MAX_CLIENT_ATTRIB_STACK_DEPTH    = 0x0D3B,
			MAX_CLIP_PLANES                  = 0x0D32,
			MAX_EVAL_ORDER                   = 0x0D30,
			MAX_LIGHTS                       = 0x0D31,
			MAX_LIST_NESTING                 = 0x0B31,
			MAX_MODELVIEW_STACK_DEPTH        = 0x0D36,
			MAX_NAME_STACK_DEPTH             = 0x0D37,
			MAX_PIXEL_MAP_TABLE              = 0x0D34,
			MAX_PROJECTION_STACK_DEPTH       = 0x0D38,
			MAX_TEXTURE_SIZE                 = 0x0D33,
			MAX_TEXTURE_STACK_DEPTH          = 0x0D39,
			MAX_VIEWPORT_DIMS                = 0x0D3A,
			MODELVIEW                        = 0x1700,
			MODELVIEW_MATRIX                 = 0x0BA6,
			MODELVIEW_STACK_DEPTH            = 0x0BA3,
			MODULATE                         = 0x2100,
			MULT                             = 0x0103,
			N3F_V3F                          = 0x2A25,
			NAME_STACK_DEPTH                 = 0x0D70,
			NAND                             = 0x150E,
			NEAREST                          = 0x2600,
			NEAREST_MIPMAP_LINEAR            = 0x2702,
			NEAREST_MIPMAP_NEAREST           = 0x2700,
			NEVER                            = 0x0200,
			NICEST                           = 0x1102,
			NONE                             = 0,
			NOOP                             = 0x1505,
			NOR                              = 0x1508,
			NORMALIZE                        = 0x0BA1,
			NORMAL_ARRAY                     = 0x8075,
			NORMAL_ARRAY_POINTER             = 0x808F,
			NORMAL_ARRAY_STRIDE              = 0x807F,
			NORMAL_ARRAY_TYPE                = 0x807E,
			NOTEQUAL                         = 0x0205,
			NO_ERROR_                        = 0,
			OBJECT_LINEAR                    = 0x2401,
			OBJECT_PLANE                     = 0x2501,
			ONE                              = 1,
			ONE_MINUS_DST_ALPHA              = 0x0305,
			ONE_MINUS_DST_COLOR              = 0x0307,
			ONE_MINUS_SRC_ALPHA              = 0x0303,
			ONE_MINUS_SRC_COLOR              = 0x0301,
			OR                               = 0x1507,
			ORDER                            = 0x0A01,
			OR_INVERTED                      = 0x150D,
			OR_REVERSE                       = 0x150B,
			OUT_OF_MEMORY                    = 0x0505,
			PACK_ALIGNMENT                   = 0x0D05,
			PACK_LSB_FIRST                   = 0x0D01,
			PACK_ROW_LENGTH                  = 0x0D02,
			PACK_SKIP_PIXELS                 = 0x0D04,
			PACK_SKIP_ROWS                   = 0x0D03,
			PACK_SWAP_BYTES                  = 0x0D00,
			PASS_THROUGH_TOKEN               = 0x0700,
			PERSPECTIVE_CORRECTION_HINT      = 0x0C50,
			PIXEL_MAP_A_TO_A                 = 0x0C79,
			PIXEL_MAP_A_TO_A_SIZE            = 0x0CB9,
			PIXEL_MAP_B_TO_B                 = 0x0C78,
			PIXEL_MAP_B_TO_B_SIZE            = 0x0CB8,
			PIXEL_MAP_G_TO_G                 = 0x0C77,
			PIXEL_MAP_G_TO_G_SIZE            = 0x0CB7,
			PIXEL_MAP_I_TO_A                 = 0x0C75,
			PIXEL_MAP_I_TO_A_SIZE            = 0x0CB5,
			PIXEL_MAP_I_TO_B                 = 0x0C74,
			PIXEL_MAP_I_TO_B_SIZE            = 0x0CB4,
			PIXEL_MAP_I_TO_G                 = 0x0C73,
			PIXEL_MAP_I_TO_G_SIZE            = 0x0CB3,
			PIXEL_MAP_I_TO_I                 = 0x0C70,
			PIXEL_MAP_I_TO_I_SIZE            = 0x0CB0,
			PIXEL_MAP_I_TO_R                 = 0x0C72,
			PIXEL_MAP_I_TO_R_SIZE            = 0x0CB2,
			PIXEL_MAP_R_TO_R                 = 0x0C76,
			PIXEL_MAP_R_TO_R_SIZE            = 0x0CB6,
			PIXEL_MAP_S_TO_S                 = 0x0C71,
			PIXEL_MAP_S_TO_S_SIZE            = 0x0CB1,
			PIXEL_MODE_BIT                   = 0x00000020,
			POINT                            = 0x1B00,
			POINTS                           = 0x0000,
			POINT_BIT                        = 0x00000002,
			POINT_SIZE                       = 0x0B11,
			POINT_SIZE_GRANULARITY           = 0x0B13,
			POINT_SIZE_RANGE                 = 0x0B12,
			POINT_SMOOTH                     = 0x0B10,
			POINT_SMOOTH_HINT                = 0x0C51,
			POINT_TOKEN                      = 0x0701,
			POLYGON                          = 0x0009,
			POLYGON_BIT                      = 0x00000008,
			POLYGON_MODE                     = 0x0B40,
			POLYGON_OFFSET_FACTOR            = 0x8038,
			POLYGON_OFFSET_FILL              = 0x8037,
			POLYGON_OFFSET_LINE              = 0x2A02,
			POLYGON_OFFSET_POINT             = 0x2A01,
			POLYGON_OFFSET_UNITS             = 0x2A00,
			POLYGON_SMOOTH                   = 0x0B41,
			POLYGON_SMOOTH_HINT              = 0x0C53,
			POLYGON_STIPPLE                  = 0x0B42,
			POLYGON_STIPPLE_BIT              = 0x00000010,
			POLYGON_TOKEN                    = 0x0703,
			POSITION                         = 0x1203,
			PROJECTION                       = 0x1701,
			PROJECTION_MATRIX                = 0x0BA7,
			PROJECTION_STACK_DEPTH           = 0x0BA4,
			PROXY_TEXTURE_1D                 = 0x8063,
			PROXY_TEXTURE_2D                 = 0x8064,
			Q                                = 0x2003,
			QUADRATIC_ATTENUATION            = 0x1209,
			QUADS                            = 0x0007,
			QUAD_STRIP                       = 0x0008,
			R                                = 0x2002,
			R3_G3_B2                         = 0x2A10,
			READ_BUFFER                      = 0x0C02,
			RED                              = 0x1903,
			RED_BIAS                         = 0x0D15,
			RED_BITS                         = 0x0D52,
			RED_SCALE                        = 0x0D14,
			RENDER                           = 0x1C00,
			RENDERER                         = 0x1F01,
			RENDER_MODE                      = 0x0C40,
			REPEAT                           = 0x2901,
			REPLACE                          = 0x1E01,
			RETURN                           = 0x0102,
			RGB                              = 0x1907,
			RGB10                            = 0x8052,
			RGB10_A2                         = 0x8059,
			RGB12                            = 0x8053,
			RGB16                            = 0x8054,
			RGB4                             = 0x804F,
			RGB5                             = 0x8050,
			RGB5_A1                          = 0x8057,
			RGB8                             = 0x8051,
			RGBA                             = 0x1908,
			RGBA12                           = 0x805A,
			RGBA16                           = 0x805B,
			RGBA2                            = 0x8055,
			RGBA4                            = 0x8056,
			RGBA8                            = 0x8058,
			RGBA_MODE                        = 0x0C31,
			RIGHT                            = 0x0407,
			S                                = 0x2000,
			SCISSOR_BIT                      = 0x00080000,
			SCISSOR_BOX                      = 0x0C10,
			SCISSOR_TEST                     = 0x0C11,
			SELECT                           = 0x1C02,
			SELECTION_BUFFER_POINTER         = 0x0DF3,
			SELECTION_BUFFER_SIZE            = 0x0DF4,
			SET                              = 0x150F,
			SHADE_MODEL                      = 0x0B54,
			SHININESS                        = 0x1601,
			SHORT                            = 0x1402,
			SMOOTH                           = 0x1D01,
			SPECULAR                         = 0x1202,
			SPHERE_MAP                       = 0x2402,
			SPOT_CUTOFF                      = 0x1206,
			SPOT_DIRECTION                   = 0x1204,
			SPOT_EXPONENT                    = 0x1205,
			SRC_ALPHA                        = 0x0302,
			SRC_ALPHA_SATURATE               = 0x0308,
			SRC_COLOR                        = 0x0300,
			STACK_OVERFLOW                   = 0x0503,
			STACK_UNDERFLOW                  = 0x0504,
			STENCIL                          = 0x1802,
			STENCIL_BITS                     = 0x0D57,
			STENCIL_BUFFER_BIT               = 0x00000400,
			STENCIL_CLEAR_VALUE              = 0x0B91,
			STENCIL_FAIL                     = 0x0B94,
			STENCIL_FUNC                     = 0x0B92,
			STENCIL_INDEX                    = 0x1901,
			STENCIL_PASS_DEPTH_FAIL          = 0x0B95,
			STENCIL_PASS_DEPTH_PASS          = 0x0B96,
			STENCIL_REF                      = 0x0B97,
			STENCIL_TEST                     = 0x0B90,
			STENCIL_VALUE_MASK               = 0x0B93,
			STENCIL_WRITEMASK                = 0x0B98,
			STEREO                           = 0x0C33,
			SUBPIXEL_BITS                    = 0x0D50,
			T                                = 0x2001,
			T2F_C3F_V3F                      = 0x2A2A,
			T2F_C4F_N3F_V3F                  = 0x2A2C,
			T2F_C4UB_V3F                     = 0x2A29,
			T2F_N3F_V3F                      = 0x2A2B,
			T2F_V3F                          = 0x2A27,
			T4F_C4F_N3F_V4F                  = 0x2A2D,
			T4F_V4F                          = 0x2A28,
			TEXTURE                          = 0x1702,
			TEXTURE_1D                       = 0x0DE0,
			TEXTURE_2D                       = 0x0DE1,
			TEXTURE_ALPHA_SIZE               = 0x805F,
			TEXTURE_BINDING_1D               = 0x8068,
			TEXTURE_BINDING_2D               = 0x8069,
			TEXTURE_BIT                      = 0x00040000,
			TEXTURE_BLUE_SIZE                = 0x805E,
			TEXTURE_BORDER                   = 0x1005,
			TEXTURE_BORDER_COLOR             = 0x1004,
			TEXTURE_COMPONENTS               = 0x1003,
			TEXTURE_COORD_ARRAY              = 0x8078,
			TEXTURE_COORD_ARRAY_POINTER      = 0x8092,
			TEXTURE_COORD_ARRAY_SIZE         = 0x8088,
			TEXTURE_COORD_ARRAY_STRIDE       = 0x808A,
			TEXTURE_COORD_ARRAY_TYPE         = 0x8089,
			TEXTURE_ENV                      = 0x2300,
			TEXTURE_ENV_COLOR                = 0x2201,
			TEXTURE_ENV_MODE                 = 0x2200,
			TEXTURE_GEN_MODE                 = 0x2500,
			TEXTURE_GEN_Q                    = 0x0C63,
			TEXTURE_GEN_R                    = 0x0C62,
			TEXTURE_GEN_S                    = 0x0C60,
			TEXTURE_GEN_T                    = 0x0C61,
			TEXTURE_GREEN_SIZE               = 0x805D,
			TEXTURE_HEIGHT                   = 0x1001,
			TEXTURE_INTENSITY_SIZE           = 0x8061,
			TEXTURE_INTERNAL_FORMAT          = 0x1003,
			TEXTURE_LUMINANCE_SIZE           = 0x8060,
			TEXTURE_MAG_FILTER               = 0x2800,
			TEXTURE_MATRIX                   = 0x0BA8,
			TEXTURE_MIN_FILTER               = 0x2801,
			TEXTURE_PRIORITY                 = 0x8066,
			TEXTURE_RED_SIZE                 = 0x805C,
			TEXTURE_RESIDENT                 = 0x8067,
			TEXTURE_STACK_DEPTH              = 0x0BA5,
			TEXTURE_WIDTH                    = 0x1000,
			TEXTURE_WRAP_S                   = 0x2802,
			TEXTURE_WRAP_T                   = 0x2803,
			TRANSFORM_BIT                    = 0x00001000,
			TRIANGLES                        = 0x0004,
			TRIANGLE_FAN                     = 0x0006,
			TRIANGLE_STRIP                   = 0x0005,
			TRUE_                            = 1,
			UNPACK_ALIGNMENT                 = 0x0CF5,
			UNPACK_LSB_FIRST                 = 0x0CF1,
			UNPACK_ROW_LENGTH                = 0x0CF2,
			UNPACK_SKIP_PIXELS               = 0x0CF4,
			UNPACK_SKIP_ROWS                 = 0x0CF3,
			UNPACK_SWAP_BYTES                = 0x0CF0,
			UNSIGNED_BYTE                    = 0x1401,
			UNSIGNED_INT                     = 0x1405,
			UNSIGNED_SHORT                   = 0x1403,
			V2F                              = 0x2A20,
			V3F                              = 0x2A21,
			VENDOR                           = 0x1F00,
			VERSION                          = 0x1F02,
			VERTEX_ARRAY                     = 0x8074,
			VERTEX_ARRAY_POINTER             = 0x808E,
			VERTEX_ARRAY_SIZE                = 0x807A,
			VERTEX_ARRAY_STRIDE              = 0x807C,
			VERTEX_ARRAY_TYPE                = 0x807B,
			VIEWPORT                         = 0x0BA2,
			VIEWPORT_BIT                     = 0x00000800,
			XOR                              = 0x1506,
			ZERO                             = 0,
			ZOOM_X                           = 0x0D16,
			ZOOM_Y                           = 0x0D17,
			
			ALIASED_LINE_WIDTH_RANGE         = 0x846E,
			ALIASED_POINT_SIZE_RANGE         = 0x846D,
			BGR                              = 0x80E0,
			BGRA                             = 0x80E1,
			CLAMP_TO_EDGE                    = 0x812F,
			LIGHT_MODEL_COLOR_CONTROL        = 0x81F8,
			MAX_3D_TEXTURE_SIZE              = 0x8073,
			MAX_ELEMENTS_INDICES             = 0x80E9,
			MAX_ELEMENTS_VERTICES            = 0x80E8,
			PACK_IMAGE_HEIGHT                = 0x806C,
			PACK_SKIP_IMAGES                 = 0x806B,
			PROXY_TEXTURE_3D                 = 0x8070,
			RESCALE_NORMAL                   = 0x803A,
			SEPARATE_SPECULAR_COLOR          = 0x81FA,
			SINGLE_COLOR                     = 0x81F9,
			SMOOTH_LINE_WIDTH_GRANULARITY    = 0x0B23,
			SMOOTH_LINE_WIDTH_RANGE          = 0x0B22,
			SMOOTH_POINT_SIZE_GRANULARITY    = 0x0B13,
			SMOOTH_POINT_SIZE_RANGE          = 0x0B12,
			TEXTURE_3D                       = 0x806F,
			TEXTURE_BASE_LEVEL               = 0x813C,
			TEXTURE_BINDING_3D               = 0x806A,
			TEXTURE_DEPTH                    = 0x8071,
			TEXTURE_MAX_LEVEL                = 0x813D,
			TEXTURE_MAX_LOD                  = 0x813B,
			TEXTURE_MIN_LOD                  = 0x813A,
			TEXTURE_WRAP_R                   = 0x8072,
			UNPACK_IMAGE_HEIGHT              = 0x806E,
			UNPACK_SKIP_IMAGES               = 0x806D,
			UNSIGNED_BYTE_2_3_3_REV          = 0x8362,
			UNSIGNED_BYTE_3_3_2              = 0x8032,
			UNSIGNED_INT_10_10_10_2          = 0x8036,
			UNSIGNED_INT_2_10_10_10_REV      = 0x8368,
			UNSIGNED_INT_8_8_8_8             = 0x8035,
			UNSIGNED_INT_8_8_8_8_REV         = 0x8367,
			UNSIGNED_SHORT_1_5_5_5_REV       = 0x8366,
			UNSIGNED_SHORT_4_4_4_4           = 0x8033,
			UNSIGNED_SHORT_4_4_4_4_REV       = 0x8365,
			UNSIGNED_SHORT_5_5_5_1           = 0x8034,
			UNSIGNED_SHORT_5_6_5             = 0x8363,
			UNSIGNED_SHORT_5_6_5_REV         = 0x8364,
			
			ACTIVE_TEXTURE                   = 0x84E0,
			ADD_SIGNED                       = 0x8574,
			CLAMP_TO_BORDER                  = 0x812D,
			CLIENT_ACTIVE_TEXTURE            = 0x84E1,
			COMBINE                          = 0x8570,
			COMBINE_ALPHA                    = 0x8572,
			COMBINE_RGB                      = 0x8571,
			COMPRESSED_ALPHA                 = 0x84E9,
			COMPRESSED_INTENSITY             = 0x84EC,
			COMPRESSED_LUMINANCE             = 0x84EA,
			COMPRESSED_LUMINANCE_ALPHA       = 0x84EB,
			COMPRESSED_RGB                   = 0x84ED,
			COMPRESSED_RGBA                  = 0x84EE,
			COMPRESSED_TEXTURE_FORMATS       = 0x86A3,
			CONSTANT                         = 0x8576,
			DOT3_RGB                         = 0x86AE,
			DOT3_RGBA                        = 0x86AF,
			INTERPOLATE                      = 0x8575,
			MAX_CUBE_MAP_TEXTURE_SIZE        = 0x851C,
			MAX_TEXTURE_UNITS                = 0x84E2,
			MULTISAMPLE                      = 0x809D,
			MULTISAMPLE_BIT                  = 0x20000000,
			NORMAL_MAP                       = 0x8511,
			NUM_COMPRESSED_TEXTURE_FORMATS   = 0x86A2,
			OPERAND0_ALPHA                   = 0x8598,
			OPERAND0_RGB                     = 0x8590,
			OPERAND1_ALPHA                   = 0x8599,
			OPERAND1_RGB                     = 0x8591,
			OPERAND2_ALPHA                   = 0x859A,
			OPERAND2_RGB                     = 0x8592,
			PREVIOUS                         = 0x8578,
			PRIMARY_COLOR                    = 0x8577,
			PROXY_TEXTURE_CUBE_MAP           = 0x851B,
			REFLECTION_MAP                   = 0x8512,
			RGB_SCALE                        = 0x8573,
			SAMPLES                          = 0x80A9,
			SAMPLE_ALPHA_TO_COVERAGE         = 0x809E,
			SAMPLE_ALPHA_TO_ONE              = 0x809F,
			SAMPLE_BUFFERS                   = 0x80A8,
			SAMPLE_COVERAGE                  = 0x80A0,
			SAMPLE_COVERAGE_INVERT           = 0x80AB,
			SAMPLE_COVERAGE_VALUE            = 0x80AA,
			SOURCE0_ALPHA                    = 0x8588,
			SOURCE0_RGB                      = 0x8580,
			SOURCE1_ALPHA                    = 0x8589,
			SOURCE1_RGB                      = 0x8581,
			SOURCE2_ALPHA                    = 0x858A,
			SOURCE2_RGB                      = 0x8582,
			SUBTRACT                         = 0x84E7,
			TEXTURE0                         = 0x84C0,
			TEXTURE1                         = 0x84C1,
			TEXTURE10                        = 0x84CA,
			TEXTURE11                        = 0x84CB,
			TEXTURE12                        = 0x84CC,
			TEXTURE13                        = 0x84CD,
			TEXTURE14                        = 0x84CE,
			TEXTURE15                        = 0x84CF,
			TEXTURE16                        = 0x84D0,
			TEXTURE17                        = 0x84D1,
			TEXTURE18                        = 0x84D2,
			TEXTURE19                        = 0x84D3,
			TEXTURE2                         = 0x84C2,
			TEXTURE20                        = 0x84D4,
			TEXTURE21                        = 0x84D5,
			TEXTURE22                        = 0x84D6,
			TEXTURE23                        = 0x84D7,
			TEXTURE24                        = 0x84D8,
			TEXTURE25                        = 0x84D9,
			TEXTURE26                        = 0x84DA,
			TEXTURE27                        = 0x84DB,
			TEXTURE28                        = 0x84DC,
			TEXTURE29                        = 0x84DD,
			TEXTURE3                         = 0x84C3,
			TEXTURE30                        = 0x84DE,
			TEXTURE31                        = 0x84DF,
			TEXTURE4                         = 0x84C4,
			TEXTURE5                         = 0x84C5,
			TEXTURE6                         = 0x84C6,
			TEXTURE7                         = 0x84C7,
			TEXTURE8                         = 0x84C8,
			TEXTURE9                         = 0x84C9,
			TEXTURE_BINDING_CUBE_MAP         = 0x8514,
			TEXTURE_COMPRESSED               = 0x86A1,
			TEXTURE_COMPRESSED_IMAGE_SIZE    = 0x86A0,
			TEXTURE_COMPRESSION_HINT         = 0x84EF,
			TEXTURE_CUBE_MAP                 = 0x8513,
			TEXTURE_CUBE_MAP_NEGATIVE_X      = 0x8516,
			TEXTURE_CUBE_MAP_NEGATIVE_Y      = 0x8518,
			TEXTURE_CUBE_MAP_NEGATIVE_Z      = 0x851A,
			TEXTURE_CUBE_MAP_POSITIVE_X      = 0x8515,
			TEXTURE_CUBE_MAP_POSITIVE_Y      = 0x8517,
			TEXTURE_CUBE_MAP_POSITIVE_Z      = 0x8519,
			TRANSPOSE_COLOR_MATRIX           = 0x84E6,
			TRANSPOSE_MODELVIEW_MATRIX       = 0x84E3,
			TRANSPOSE_PROJECTION_MATRIX      = 0x84E4,
			TRANSPOSE_TEXTURE_MATRIX         = 0x84E5,
			
			BLEND_COLOR                      = 0x8005,
			BLEND_DST_ALPHA                  = 0x80CA,
			BLEND_DST_RGB                    = 0x80C8,
			BLEND_SRC_ALPHA                  = 0x80CB,
			BLEND_SRC_RGB                    = 0x80C9,
			COLOR_SUM                        = 0x8458,
			COMPARE_R_TO_TEXTURE             = 0x884E,
			CONSTANT_ALPHA                   = 0x8003,
			CONSTANT_COLOR                   = 0x8001,
			CURRENT_FOG_COORDINATE           = 0x8453,
			CURRENT_SECONDARY_COLOR          = 0x8459,
			DECR_WRAP                        = 0x8508,
			DEPTH_COMPONENT16                = 0x81A5,
			DEPTH_COMPONENT24                = 0x81A6,
			DEPTH_COMPONENT32                = 0x81A7,
			DEPTH_TEXTURE_MODE               = 0x884B,
			FOG_COORDINATE                   = 0x8451,
			FOG_COORDINATE_ARRAY             = 0x8457,
			FOG_COORDINATE_ARRAY_POINTER     = 0x8456,
			FOG_COORDINATE_ARRAY_STRIDE      = 0x8455,
			FOG_COORDINATE_ARRAY_TYPE        = 0x8454,
			FOG_COORDINATE_SOURCE            = 0x8450,
			FRAGMENT_DEPTH                   = 0x8452,
			FUNC_ADD                         = 0x8006,
			FUNC_REVERSE_SUBTRACT            = 0x800B,
			FUNC_SUBTRACT                    = 0x800A,
			GENERATE_MIPMAP                  = 0x8191,
			GENERATE_MIPMAP_HINT             = 0x8192,
			INCR_WRAP                        = 0x8507,
			MAX                              = 0x8008,
			MAX_TEXTURE_LOD_BIAS             = 0x84FD,
			MIN                              = 0x8007,
			MIRRORED_REPEAT                  = 0x8370,
			ONE_MINUS_CONSTANT_ALPHA         = 0x8004,
			ONE_MINUS_CONSTANT_COLOR         = 0x8002,
			POINT_DISTANCE_ATTENUATION       = 0x8129,
			POINT_FADE_THRESHOLD_SIZE        = 0x8128,
			POINT_SIZE_MAX                   = 0x8127,
			POINT_SIZE_MIN                   = 0x8126,
			SECONDARY_COLOR_ARRAY            = 0x845E,
			SECONDARY_COLOR_ARRAY_POINTER    = 0x845D,
			SECONDARY_COLOR_ARRAY_SIZE       = 0x845A,
			SECONDARY_COLOR_ARRAY_STRIDE     = 0x845C,
			SECONDARY_COLOR_ARRAY_TYPE       = 0x845B,
			TEXTURE_COMPARE_FUNC             = 0x884D,
			TEXTURE_COMPARE_MODE             = 0x884C,
			TEXTURE_DEPTH_SIZE               = 0x884A,
			TEXTURE_FILTER_CONTROL           = 0x8500,
			TEXTURE_LOD_BIAS                 = 0x8501,
			
			ARRAY_BUFFER                     = 0x8892,
			ARRAY_BUFFER_BINDING             = 0x8894,
			BUFFER_ACCESS                    = 0x88BB,
			BUFFER_MAPPED                    = 0x88BC,
			BUFFER_MAP_POINTER               = 0x88BD,
			BUFFER_SIZE                      = 0x8764,
			BUFFER_USAGE                     = 0x8765,
			COLOR_ARRAY_BUFFER_BINDING       = 0x8898,
			CURRENT_FOG_COORD                = 0x8453,
			CURRENT_QUERY                    = 0x8865,
			DYNAMIC_COPY                     = 0x88EA,
			DYNAMIC_DRAW                     = 0x88E8,
			DYNAMIC_READ                     = 0x88E9,
			EDGE_FLAG_ARRAY_BUFFER_BINDING   = 0x889B,
			ELEMENT_ARRAY_BUFFER             = 0x8893,
			ELEMENT_ARRAY_BUFFER_BINDING     = 0x8895,
			FOG_COORD                        = 0x8451,
			FOG_COORDINATE_ARRAY_BUFFER_BINDING = 0x889D,
			FOG_COORD_ARRAY                  = 0x8457,
			FOG_COORD_ARRAY_BUFFER_BINDING   = 0x889D,
			FOG_COORD_ARRAY_POINTER          = 0x8456,
			FOG_COORD_ARRAY_STRIDE           = 0x8455,
			FOG_COORD_ARRAY_TYPE             = 0x8454,
			FOG_COORD_SRC                    = 0x8450,
			INDEX_ARRAY_BUFFER_BINDING       = 0x8899,
			NORMAL_ARRAY_BUFFER_BINDING      = 0x8897,
			QUERY_COUNTER_BITS               = 0x8864,
			QUERY_RESULT                     = 0x8866,
			QUERY_RESULT_AVAILABLE           = 0x8867,
			READ_ONLY                        = 0x88B8,
			READ_WRITE                       = 0x88BA,
			SAMPLES_PASSED                   = 0x8914,
			SECONDARY_COLOR_ARRAY_BUFFER_BINDING = 0x889C,
			SRC0_ALPHA                       = 0x8588,
			SRC0_RGB                         = 0x8580,
			SRC1_ALPHA                       = 0x8589,
			SRC1_RGB                         = 0x8581,
			SRC2_ALPHA                       = 0x858A,
			SRC2_RGB                         = 0x8582,
			STATIC_COPY                      = 0x88E6,
			STATIC_DRAW                      = 0x88E4,
			STATIC_READ                      = 0x88E5,
			STREAM_COPY                      = 0x88E2,
			STREAM_DRAW                      = 0x88E0,
			STREAM_READ                      = 0x88E1,
			TEXTURE_COORD_ARRAY_BUFFER_BINDING = 0x889A,
			VERTEX_ARRAY_BUFFER_BINDING      = 0x8896,
			VERTEX_ATTRIB_ARRAY_BUFFER_BINDING = 0x889F,
			WEIGHT_ARRAY_BUFFER_BINDING      = 0x889E,
			WRITE_ONLY                       = 0x88B9,
			
			ACTIVE_ATTRIBUTES                = 0x8B89,
			ACTIVE_ATTRIBUTE_MAX_LENGTH      = 0x8B8A,
			ACTIVE_UNIFORMS                  = 0x8B86,
			ACTIVE_UNIFORM_MAX_LENGTH        = 0x8B87,
			ATTACHED_SHADERS                 = 0x8B85,
			BLEND_EQUATION_ALPHA             = 0x883D,
			BLEND_EQUATION_RGB               = 0x8009,
			BOOL                             = 0x8B56,
			BOOL_VEC2                        = 0x8B57,
			BOOL_VEC3                        = 0x8B58,
			BOOL_VEC4                        = 0x8B59,
			COMPILE_STATUS                   = 0x8B81,
			COORD_REPLACE                    = 0x8862,
			CURRENT_PROGRAM                  = 0x8B8D,
			CURRENT_VERTEX_ATTRIB            = 0x8626,
			DELETE_STATUS                    = 0x8B80,
			DRAW_BUFFER0                     = 0x8825,
			DRAW_BUFFER1                     = 0x8826,
			DRAW_BUFFER10                    = 0x882F,
			DRAW_BUFFER11                    = 0x8830,
			DRAW_BUFFER12                    = 0x8831,
			DRAW_BUFFER13                    = 0x8832,
			DRAW_BUFFER14                    = 0x8833,
			DRAW_BUFFER15                    = 0x8834,
			DRAW_BUFFER2                     = 0x8827,
			DRAW_BUFFER3                     = 0x8828,
			DRAW_BUFFER4                     = 0x8829,
			DRAW_BUFFER5                     = 0x882A,
			DRAW_BUFFER6                     = 0x882B,
			DRAW_BUFFER7                     = 0x882C,
			DRAW_BUFFER8                     = 0x882D,
			DRAW_BUFFER9                     = 0x882E,
			FLOAT_MAT2                       = 0x8B5A,
			FLOAT_MAT3                       = 0x8B5B,
			FLOAT_MAT4                       = 0x8B5C,
			FLOAT_VEC2                       = 0x8B50,
			FLOAT_VEC3                       = 0x8B51,
			FLOAT_VEC4                       = 0x8B52,
			FRAGMENT_SHADER                  = 0x8B30,
			FRAGMENT_SHADER_DERIVATIVE_HINT  = 0x8B8B,
			INFO_LOG_LENGTH                  = 0x8B84,
			INT_VEC2                         = 0x8B53,
			INT_VEC3                         = 0x8B54,
			INT_VEC4                         = 0x8B55,
			LINK_STATUS                      = 0x8B82,
			LOWER_LEFT                       = 0x8CA1,
			MAX_COMBINED_TEXTURE_IMAGE_UNITS = 0x8B4D,
			MAX_DRAW_BUFFERS                 = 0x8824,
			MAX_FRAGMENT_UNIFORM_COMPONENTS  = 0x8B49,
			MAX_TEXTURE_COORDS               = 0x8871,
			MAX_TEXTURE_IMAGE_UNITS          = 0x8872,
			MAX_VARYING_FLOATS               = 0x8B4B,
			MAX_VERTEX_ATTRIBS               = 0x8869,
			MAX_VERTEX_TEXTURE_IMAGE_UNITS   = 0x8B4C,
			MAX_VERTEX_UNIFORM_COMPONENTS    = 0x8B4A,
			POINT_SPRITE                     = 0x8861,
			POINT_SPRITE_COORD_ORIGIN        = 0x8CA0,
			SAMPLER_1D                       = 0x8B5D,
			SAMPLER_1D_SHADOW                = 0x8B61,
			SAMPLER_2D                       = 0x8B5E,
			SAMPLER_2D_SHADOW                = 0x8B62,
			SAMPLER_3D                       = 0x8B5F,
			SAMPLER_CUBE                     = 0x8B60,
			SHADER_SOURCE_LENGTH             = 0x8B88,
			SHADER_TYPE                      = 0x8B4F,
			SHADING_LANGUAGE_VERSION         = 0x8B8C,
			STENCIL_BACK_FAIL                = 0x8801,
			STENCIL_BACK_FUNC                = 0x8800,
			STENCIL_BACK_PASS_DEPTH_FAIL     = 0x8802,
			STENCIL_BACK_PASS_DEPTH_PASS     = 0x8803,
			STENCIL_BACK_REF                 = 0x8CA3,
			STENCIL_BACK_VALUE_MASK          = 0x8CA4,
			STENCIL_BACK_WRITEMASK           = 0x8CA5,
			UPPER_LEFT                       = 0x8CA2,
			VALIDATE_STATUS                  = 0x8B83,
			VERTEX_ATTRIB_ARRAY_ENABLED      = 0x8622,
			VERTEX_ATTRIB_ARRAY_NORMALIZED   = 0x886A,
			VERTEX_ATTRIB_ARRAY_POINTER      = 0x8645,
			VERTEX_ATTRIB_ARRAY_SIZE         = 0x8623,
			VERTEX_ATTRIB_ARRAY_STRIDE       = 0x8624,
			VERTEX_ATTRIB_ARRAY_TYPE         = 0x8625,
			VERTEX_PROGRAM_POINT_SIZE        = 0x8642,
			VERTEX_PROGRAM_TWO_SIDE          = 0x8643,
			VERTEX_SHADER                    = 0x8B31,
			
			COMPRESSED_SLUMINANCE            = 0x8C4A,
			COMPRESSED_SLUMINANCE_ALPHA      = 0x8C4B,
			COMPRESSED_SRGB                  = 0x8C48,
			COMPRESSED_SRGB_ALPHA            = 0x8C49,
			CURRENT_RASTER_SECONDARY_COLOR   = 0x845F,
			FLOAT_MAT2x3                     = 0x8B65,
			FLOAT_MAT2x4                     = 0x8B66,
			FLOAT_MAT3x2                     = 0x8B67,
			FLOAT_MAT3x4                     = 0x8B68,
			FLOAT_MAT4x2                     = 0x8B69,
			FLOAT_MAT4x3                     = 0x8B6A,
			PIXEL_PACK_BUFFER                = 0x88EB,
			PIXEL_PACK_BUFFER_BINDING        = 0x88ED,
			PIXEL_UNPACK_BUFFER              = 0x88EC,
			PIXEL_UNPACK_BUFFER_BINDING      = 0x88EF,
			SLUMINANCE                       = 0x8C46,
			SLUMINANCE8                      = 0x8C47,
			SLUMINANCE8_ALPHA8               = 0x8C45,
			SLUMINANCE_ALPHA                 = 0x8C44,
			SRGB                             = 0x8C40,
			SRGB8                            = 0x8C41,
			SRGB8_ALPHA8                     = 0x8C43,
			SRGB_ALPHA                       = 0x8C42,
			
		};
		
		
		extern void (CODEGEN_FUNCPTR *DrawArraysInstancedARB)(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
		extern void (CODEGEN_FUNCPTR *DrawElementsInstancedARB)(GLenum mode, GLsizei count, GLenum type, const void * indices, GLsizei primcount);
		
		extern void (CODEGEN_FUNCPTR *BindBufferBase)(GLenum target, GLuint index, GLuint buffer);
		extern void (CODEGEN_FUNCPTR *BindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
		extern void (CODEGEN_FUNCPTR *GetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformBlockName);
		extern void (CODEGEN_FUNCPTR *GetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *GetActiveUniformName)(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformName);
		extern void (CODEGEN_FUNCPTR *GetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint * uniformIndices, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *GetIntegeri_v)(GLenum target, GLuint index, GLint * data);
		extern GLuint (CODEGEN_FUNCPTR *GetUniformBlockIndex)(GLuint program, const GLchar * uniformBlockName);
		extern void (CODEGEN_FUNCPTR *GetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar *const* uniformNames, GLuint * uniformIndices);
		extern void (CODEGEN_FUNCPTR *UniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
		
		extern void (CODEGEN_FUNCPTR *VertexAttribDivisorARB)(GLuint index, GLuint divisor);
		
		extern void (CODEGEN_FUNCPTR *BindVertexArray)(GLuint ren_array);
		extern void (CODEGEN_FUNCPTR *DeleteVertexArrays)(GLsizei n, const GLuint * arrays);
		extern void (CODEGEN_FUNCPTR *GenVertexArrays)(GLsizei n, GLuint * arrays);
		extern GLboolean (CODEGEN_FUNCPTR *IsVertexArray)(GLuint ren_array);
		
		extern void (CODEGEN_FUNCPTR *BindFramebufferEXT)(GLenum target, GLuint framebuffer);
		extern void (CODEGEN_FUNCPTR *BindRenderbufferEXT)(GLenum target, GLuint renderbuffer);
		extern GLenum (CODEGEN_FUNCPTR *CheckFramebufferStatusEXT)(GLenum target);
		extern void (CODEGEN_FUNCPTR *DeleteFramebuffersEXT)(GLsizei n, const GLuint * framebuffers);
		extern void (CODEGEN_FUNCPTR *DeleteRenderbuffersEXT)(GLsizei n, const GLuint * renderbuffers);
		extern void (CODEGEN_FUNCPTR *FramebufferRenderbufferEXT)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
		extern void (CODEGEN_FUNCPTR *FramebufferTexture1DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
		extern void (CODEGEN_FUNCPTR *FramebufferTexture2DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
		extern void (CODEGEN_FUNCPTR *FramebufferTexture3DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
		extern void (CODEGEN_FUNCPTR *GenFramebuffersEXT)(GLsizei n, GLuint * framebuffers);
		extern void (CODEGEN_FUNCPTR *GenRenderbuffersEXT)(GLsizei n, GLuint * renderbuffers);
		extern void (CODEGEN_FUNCPTR *GenerateMipmapEXT)(GLenum target);
		extern void (CODEGEN_FUNCPTR *GetFramebufferAttachmentParameterivEXT)(GLenum target, GLenum attachment, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *GetRenderbufferParameterivEXT)(GLenum target, GLenum pname, GLint * params);
		extern GLboolean (CODEGEN_FUNCPTR *IsFramebufferEXT)(GLuint framebuffer);
		extern GLboolean (CODEGEN_FUNCPTR *IsRenderbufferEXT)(GLuint renderbuffer);
		extern void (CODEGEN_FUNCPTR *RenderbufferStorageEXT)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
		
		
		
		
		extern void (CODEGEN_FUNCPTR *Accum)(GLenum op, GLfloat value);
		extern void (CODEGEN_FUNCPTR *AlphaFunc)(GLenum func, GLfloat ref);
		extern void (CODEGEN_FUNCPTR *Begin)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *Bitmap)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte * bitmap);
		extern void (CODEGEN_FUNCPTR *BlendFunc)(GLenum sfactor, GLenum dfactor);
		extern void (CODEGEN_FUNCPTR *CallList)(GLuint list);
		extern void (CODEGEN_FUNCPTR *CallLists)(GLsizei n, GLenum type, const void * lists);
		extern void (CODEGEN_FUNCPTR *Clear)(GLbitfield mask);
		extern void (CODEGEN_FUNCPTR *ClearAccum)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		extern void (CODEGEN_FUNCPTR *ClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		extern void (CODEGEN_FUNCPTR *ClearDepth)(GLdouble depth);
		extern void (CODEGEN_FUNCPTR *ClearIndex)(GLfloat c);
		extern void (CODEGEN_FUNCPTR *ClearStencil)(GLint s);
		extern void (CODEGEN_FUNCPTR *ClipPlane)(GLenum plane, const GLdouble * equation);
		extern void (CODEGEN_FUNCPTR *Color3b)(GLbyte red, GLbyte green, GLbyte blue);
		extern void (CODEGEN_FUNCPTR *Color3bv)(const GLbyte * v);
		extern void (CODEGEN_FUNCPTR *Color3d)(GLdouble red, GLdouble green, GLdouble blue);
		extern void (CODEGEN_FUNCPTR *Color3dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *Color3f)(GLfloat red, GLfloat green, GLfloat blue);
		extern void (CODEGEN_FUNCPTR *Color3fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *Color3i)(GLint red, GLint green, GLint blue);
		extern void (CODEGEN_FUNCPTR *Color3iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *Color3s)(GLshort red, GLshort green, GLshort blue);
		extern void (CODEGEN_FUNCPTR *Color3sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *Color3ub)(GLubyte red, GLubyte green, GLubyte blue);
		extern void (CODEGEN_FUNCPTR *Color3ubv)(const GLubyte * v);
		extern void (CODEGEN_FUNCPTR *Color3ui)(GLuint red, GLuint green, GLuint blue);
		extern void (CODEGEN_FUNCPTR *Color3uiv)(const GLuint * v);
		extern void (CODEGEN_FUNCPTR *Color3us)(GLushort red, GLushort green, GLushort blue);
		extern void (CODEGEN_FUNCPTR *Color3usv)(const GLushort * v);
		extern void (CODEGEN_FUNCPTR *Color4b)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
		extern void (CODEGEN_FUNCPTR *Color4bv)(const GLbyte * v);
		extern void (CODEGEN_FUNCPTR *Color4d)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
		extern void (CODEGEN_FUNCPTR *Color4dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *Color4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		extern void (CODEGEN_FUNCPTR *Color4fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *Color4i)(GLint red, GLint green, GLint blue, GLint alpha);
		extern void (CODEGEN_FUNCPTR *Color4iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *Color4s)(GLshort red, GLshort green, GLshort blue, GLshort alpha);
		extern void (CODEGEN_FUNCPTR *Color4sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *Color4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
		extern void (CODEGEN_FUNCPTR *Color4ubv)(const GLubyte * v);
		extern void (CODEGEN_FUNCPTR *Color4ui)(GLuint red, GLuint green, GLuint blue, GLuint alpha);
		extern void (CODEGEN_FUNCPTR *Color4uiv)(const GLuint * v);
		extern void (CODEGEN_FUNCPTR *Color4us)(GLushort red, GLushort green, GLushort blue, GLushort alpha);
		extern void (CODEGEN_FUNCPTR *Color4usv)(const GLushort * v);
		extern void (CODEGEN_FUNCPTR *ColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
		extern void (CODEGEN_FUNCPTR *ColorMaterial)(GLenum face, GLenum mode);
		extern void (CODEGEN_FUNCPTR *CopyPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
		extern void (CODEGEN_FUNCPTR *CullFace)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *DeleteLists)(GLuint list, GLsizei range);
		extern void (CODEGEN_FUNCPTR *DepthFunc)(GLenum func);
		extern void (CODEGEN_FUNCPTR *DepthMask)(GLboolean flag);
		extern void (CODEGEN_FUNCPTR *DepthRange)(GLdouble ren_near, GLdouble ren_far);
		extern void (CODEGEN_FUNCPTR *Disable)(GLenum cap);
		extern void (CODEGEN_FUNCPTR *DrawBuffer)(GLenum buf);
		extern void (CODEGEN_FUNCPTR *DrawPixels)(GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *EdgeFlag)(GLboolean flag);
		extern void (CODEGEN_FUNCPTR *EdgeFlagv)(const GLboolean * flag);
		extern void (CODEGEN_FUNCPTR *Enable)(GLenum cap);
		extern void (CODEGEN_FUNCPTR *End)(void);
		extern void (CODEGEN_FUNCPTR *EndList)(void);
		extern void (CODEGEN_FUNCPTR *EvalCoord1d)(GLdouble u);
		extern void (CODEGEN_FUNCPTR *EvalCoord1dv)(const GLdouble * u);
		extern void (CODEGEN_FUNCPTR *EvalCoord1f)(GLfloat u);
		extern void (CODEGEN_FUNCPTR *EvalCoord1fv)(const GLfloat * u);
		extern void (CODEGEN_FUNCPTR *EvalCoord2d)(GLdouble u, GLdouble v);
		extern void (CODEGEN_FUNCPTR *EvalCoord2dv)(const GLdouble * u);
		extern void (CODEGEN_FUNCPTR *EvalCoord2f)(GLfloat u, GLfloat v);
		extern void (CODEGEN_FUNCPTR *EvalCoord2fv)(const GLfloat * u);
		extern void (CODEGEN_FUNCPTR *EvalMesh1)(GLenum mode, GLint i1, GLint i2);
		extern void (CODEGEN_FUNCPTR *EvalMesh2)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
		extern void (CODEGEN_FUNCPTR *EvalPoint1)(GLint i);
		extern void (CODEGEN_FUNCPTR *EvalPoint2)(GLint i, GLint j);
		extern void (CODEGEN_FUNCPTR *FeedbackBuffer)(GLsizei size, GLenum type, GLfloat * buffer);
		extern void (CODEGEN_FUNCPTR *Finish)(void);
		extern void (CODEGEN_FUNCPTR *Flush)(void);
		extern void (CODEGEN_FUNCPTR *Fogf)(GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *Fogfv)(GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *Fogi)(GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *Fogiv)(GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *FrontFace)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *Frustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
		extern GLuint (CODEGEN_FUNCPTR *GenLists)(GLsizei range);
		extern void (CODEGEN_FUNCPTR *GetBooleanv)(GLenum pname, GLboolean * data);
		extern void (CODEGEN_FUNCPTR *GetClipPlane)(GLenum plane, GLdouble * equation);
		extern void (CODEGEN_FUNCPTR *GetDoublev)(GLenum pname, GLdouble * data);
		extern GLenum (CODEGEN_FUNCPTR *GetError)(void);
		extern void (CODEGEN_FUNCPTR *GetFloatv)(GLenum pname, GLfloat * data);
		extern void (CODEGEN_FUNCPTR *GetIntegerv)(GLenum pname, GLint * data);
		extern void (CODEGEN_FUNCPTR *GetLightfv)(GLenum light, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *GetLightiv)(GLenum light, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *GetMapdv)(GLenum target, GLenum query, GLdouble * v);
		extern void (CODEGEN_FUNCPTR *GetMapfv)(GLenum target, GLenum query, GLfloat * v);
		extern void (CODEGEN_FUNCPTR *GetMapiv)(GLenum target, GLenum query, GLint * v);
		extern void (CODEGEN_FUNCPTR *GetMaterialfv)(GLenum face, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *GetMaterialiv)(GLenum face, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *GetPixelMapfv)(GLenum map, GLfloat * values);
		extern void (CODEGEN_FUNCPTR *GetPixelMapuiv)(GLenum map, GLuint * values);
		extern void (CODEGEN_FUNCPTR *GetPixelMapusv)(GLenum map, GLushort * values);
		extern void (CODEGEN_FUNCPTR *GetPolygonStipple)(GLubyte * mask);
		extern const GLubyte * (CODEGEN_FUNCPTR *GetString)(GLenum name);
		extern void (CODEGEN_FUNCPTR *GetTexEnvfv)(GLenum target, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *GetTexEnviv)(GLenum target, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *GetTexGendv)(GLenum coord, GLenum pname, GLdouble * params);
		extern void (CODEGEN_FUNCPTR *GetTexGenfv)(GLenum coord, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *GetTexGeniv)(GLenum coord, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *GetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, void * pixels);
		extern void (CODEGEN_FUNCPTR *GetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *GetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *GetTexParameterfv)(GLenum target, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *GetTexParameteriv)(GLenum target, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *Hint)(GLenum target, GLenum mode);
		extern void (CODEGEN_FUNCPTR *IndexMask)(GLuint mask);
		extern void (CODEGEN_FUNCPTR *Indexd)(GLdouble c);
		extern void (CODEGEN_FUNCPTR *Indexdv)(const GLdouble * c);
		extern void (CODEGEN_FUNCPTR *Indexf)(GLfloat c);
		extern void (CODEGEN_FUNCPTR *Indexfv)(const GLfloat * c);
		extern void (CODEGEN_FUNCPTR *Indexi)(GLint c);
		extern void (CODEGEN_FUNCPTR *Indexiv)(const GLint * c);
		extern void (CODEGEN_FUNCPTR *Indexs)(GLshort c);
		extern void (CODEGEN_FUNCPTR *Indexsv)(const GLshort * c);
		extern void (CODEGEN_FUNCPTR *InitNames)(void);
		extern GLboolean (CODEGEN_FUNCPTR *IsEnabled)(GLenum cap);
		extern GLboolean (CODEGEN_FUNCPTR *IsList)(GLuint list);
		extern void (CODEGEN_FUNCPTR *LightModelf)(GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *LightModelfv)(GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *LightModeli)(GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *LightModeliv)(GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *Lightf)(GLenum light, GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *Lightfv)(GLenum light, GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *Lighti)(GLenum light, GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *Lightiv)(GLenum light, GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *LineStipple)(GLint factor, GLushort pattern);
		extern void (CODEGEN_FUNCPTR *LineWidth)(GLfloat width);
		extern void (CODEGEN_FUNCPTR *ListBase)(GLuint base);
		extern void (CODEGEN_FUNCPTR *LoadIdentity)(void);
		extern void (CODEGEN_FUNCPTR *LoadMatrixd)(const GLdouble * m);
		extern void (CODEGEN_FUNCPTR *LoadMatrixf)(const GLfloat * m);
		extern void (CODEGEN_FUNCPTR *LoadName)(GLuint name);
		extern void (CODEGEN_FUNCPTR *LogicOp)(GLenum opcode);
		extern void (CODEGEN_FUNCPTR *Map1d)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble * points);
		extern void (CODEGEN_FUNCPTR *Map1f)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat * points);
		extern void (CODEGEN_FUNCPTR *Map2d)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble * points);
		extern void (CODEGEN_FUNCPTR *Map2f)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat * points);
		extern void (CODEGEN_FUNCPTR *MapGrid1d)(GLint un, GLdouble u1, GLdouble u2);
		extern void (CODEGEN_FUNCPTR *MapGrid1f)(GLint un, GLfloat u1, GLfloat u2);
		extern void (CODEGEN_FUNCPTR *MapGrid2d)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
		extern void (CODEGEN_FUNCPTR *MapGrid2f)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
		extern void (CODEGEN_FUNCPTR *Materialf)(GLenum face, GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *Materialfv)(GLenum face, GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *Materiali)(GLenum face, GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *Materialiv)(GLenum face, GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *MatrixMode)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *MultMatrixd)(const GLdouble * m);
		extern void (CODEGEN_FUNCPTR *MultMatrixf)(const GLfloat * m);
		extern void (CODEGEN_FUNCPTR *NewList)(GLuint list, GLenum mode);
		extern void (CODEGEN_FUNCPTR *Normal3b)(GLbyte nx, GLbyte ny, GLbyte nz);
		extern void (CODEGEN_FUNCPTR *Normal3bv)(const GLbyte * v);
		extern void (CODEGEN_FUNCPTR *Normal3d)(GLdouble nx, GLdouble ny, GLdouble nz);
		extern void (CODEGEN_FUNCPTR *Normal3dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *Normal3f)(GLfloat nx, GLfloat ny, GLfloat nz);
		extern void (CODEGEN_FUNCPTR *Normal3fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *Normal3i)(GLint nx, GLint ny, GLint nz);
		extern void (CODEGEN_FUNCPTR *Normal3iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *Normal3s)(GLshort nx, GLshort ny, GLshort nz);
		extern void (CODEGEN_FUNCPTR *Normal3sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *Ortho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
		extern void (CODEGEN_FUNCPTR *PassThrough)(GLfloat token);
		extern void (CODEGEN_FUNCPTR *PixelMapfv)(GLenum map, GLsizei mapsize, const GLfloat * values);
		extern void (CODEGEN_FUNCPTR *PixelMapuiv)(GLenum map, GLsizei mapsize, const GLuint * values);
		extern void (CODEGEN_FUNCPTR *PixelMapusv)(GLenum map, GLsizei mapsize, const GLushort * values);
		extern void (CODEGEN_FUNCPTR *PixelStoref)(GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *PixelStorei)(GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *PixelTransferf)(GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *PixelTransferi)(GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *PixelZoom)(GLfloat xfactor, GLfloat yfactor);
		extern void (CODEGEN_FUNCPTR *PointSize)(GLfloat size);
		extern void (CODEGEN_FUNCPTR *PolygonMode)(GLenum face, GLenum mode);
		extern void (CODEGEN_FUNCPTR *PolygonStipple)(const GLubyte * mask);
		extern void (CODEGEN_FUNCPTR *PopAttrib)(void);
		extern void (CODEGEN_FUNCPTR *PopMatrix)(void);
		extern void (CODEGEN_FUNCPTR *PopName)(void);
		extern void (CODEGEN_FUNCPTR *PushAttrib)(GLbitfield mask);
		extern void (CODEGEN_FUNCPTR *PushMatrix)(void);
		extern void (CODEGEN_FUNCPTR *PushName)(GLuint name);
		extern void (CODEGEN_FUNCPTR *RasterPos2d)(GLdouble x, GLdouble y);
		extern void (CODEGEN_FUNCPTR *RasterPos2dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *RasterPos2f)(GLfloat x, GLfloat y);
		extern void (CODEGEN_FUNCPTR *RasterPos2fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *RasterPos2i)(GLint x, GLint y);
		extern void (CODEGEN_FUNCPTR *RasterPos2iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *RasterPos2s)(GLshort x, GLshort y);
		extern void (CODEGEN_FUNCPTR *RasterPos2sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *RasterPos3d)(GLdouble x, GLdouble y, GLdouble z);
		extern void (CODEGEN_FUNCPTR *RasterPos3dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *RasterPos3f)(GLfloat x, GLfloat y, GLfloat z);
		extern void (CODEGEN_FUNCPTR *RasterPos3fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *RasterPos3i)(GLint x, GLint y, GLint z);
		extern void (CODEGEN_FUNCPTR *RasterPos3iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *RasterPos3s)(GLshort x, GLshort y, GLshort z);
		extern void (CODEGEN_FUNCPTR *RasterPos3sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *RasterPos4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
		extern void (CODEGEN_FUNCPTR *RasterPos4dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *RasterPos4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
		extern void (CODEGEN_FUNCPTR *RasterPos4fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *RasterPos4i)(GLint x, GLint y, GLint z, GLint w);
		extern void (CODEGEN_FUNCPTR *RasterPos4iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *RasterPos4s)(GLshort x, GLshort y, GLshort z, GLshort w);
		extern void (CODEGEN_FUNCPTR *RasterPos4sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *ReadBuffer)(GLenum src);
		extern void (CODEGEN_FUNCPTR *ReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels);
		extern void (CODEGEN_FUNCPTR *Rectd)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
		extern void (CODEGEN_FUNCPTR *Rectdv)(const GLdouble * v1, const GLdouble * v2);
		extern void (CODEGEN_FUNCPTR *Rectf)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
		extern void (CODEGEN_FUNCPTR *Rectfv)(const GLfloat * v1, const GLfloat * v2);
		extern void (CODEGEN_FUNCPTR *Recti)(GLint x1, GLint y1, GLint x2, GLint y2);
		extern void (CODEGEN_FUNCPTR *Rectiv)(const GLint * v1, const GLint * v2);
		extern void (CODEGEN_FUNCPTR *Rects)(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
		extern void (CODEGEN_FUNCPTR *Rectsv)(const GLshort * v1, const GLshort * v2);
		extern GLint (CODEGEN_FUNCPTR *RenderMode)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *Rotated)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
		extern void (CODEGEN_FUNCPTR *Rotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
		extern void (CODEGEN_FUNCPTR *Scaled)(GLdouble x, GLdouble y, GLdouble z);
		extern void (CODEGEN_FUNCPTR *Scalef)(GLfloat x, GLfloat y, GLfloat z);
		extern void (CODEGEN_FUNCPTR *Scissor)(GLint x, GLint y, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *SelectBuffer)(GLsizei size, GLuint * buffer);
		extern void (CODEGEN_FUNCPTR *ShadeModel)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *StencilFunc)(GLenum func, GLint ref, GLuint mask);
		extern void (CODEGEN_FUNCPTR *StencilMask)(GLuint mask);
		extern void (CODEGEN_FUNCPTR *StencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
		extern void (CODEGEN_FUNCPTR *TexCoord1d)(GLdouble s);
		extern void (CODEGEN_FUNCPTR *TexCoord1dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *TexCoord1f)(GLfloat s);
		extern void (CODEGEN_FUNCPTR *TexCoord1fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *TexCoord1i)(GLint s);
		extern void (CODEGEN_FUNCPTR *TexCoord1iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *TexCoord1s)(GLshort s);
		extern void (CODEGEN_FUNCPTR *TexCoord1sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *TexCoord2d)(GLdouble s, GLdouble t);
		extern void (CODEGEN_FUNCPTR *TexCoord2dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *TexCoord2f)(GLfloat s, GLfloat t);
		extern void (CODEGEN_FUNCPTR *TexCoord2fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *TexCoord2i)(GLint s, GLint t);
		extern void (CODEGEN_FUNCPTR *TexCoord2iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *TexCoord2s)(GLshort s, GLshort t);
		extern void (CODEGEN_FUNCPTR *TexCoord2sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *TexCoord3d)(GLdouble s, GLdouble t, GLdouble r);
		extern void (CODEGEN_FUNCPTR *TexCoord3dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *TexCoord3f)(GLfloat s, GLfloat t, GLfloat r);
		extern void (CODEGEN_FUNCPTR *TexCoord3fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *TexCoord3i)(GLint s, GLint t, GLint r);
		extern void (CODEGEN_FUNCPTR *TexCoord3iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *TexCoord3s)(GLshort s, GLshort t, GLshort r);
		extern void (CODEGEN_FUNCPTR *TexCoord3sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *TexCoord4d)(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
		extern void (CODEGEN_FUNCPTR *TexCoord4dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *TexCoord4f)(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
		extern void (CODEGEN_FUNCPTR *TexCoord4fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *TexCoord4i)(GLint s, GLint t, GLint r, GLint q);
		extern void (CODEGEN_FUNCPTR *TexCoord4iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *TexCoord4s)(GLshort s, GLshort t, GLshort r, GLshort q);
		extern void (CODEGEN_FUNCPTR *TexCoord4sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *TexEnvf)(GLenum target, GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *TexEnvfv)(GLenum target, GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *TexEnvi)(GLenum target, GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *TexEnviv)(GLenum target, GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *TexGend)(GLenum coord, GLenum pname, GLdouble param);
		extern void (CODEGEN_FUNCPTR *TexGendv)(GLenum coord, GLenum pname, const GLdouble * params);
		extern void (CODEGEN_FUNCPTR *TexGenf)(GLenum coord, GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *TexGenfv)(GLenum coord, GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *TexGeni)(GLenum coord, GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *TexGeniv)(GLenum coord, GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *TexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *TexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *TexParameterf)(GLenum target, GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *TexParameterfv)(GLenum target, GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *TexParameteri)(GLenum target, GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *TexParameteriv)(GLenum target, GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *Translated)(GLdouble x, GLdouble y, GLdouble z);
		extern void (CODEGEN_FUNCPTR *Translatef)(GLfloat x, GLfloat y, GLfloat z);
		extern void (CODEGEN_FUNCPTR *Vertex2d)(GLdouble x, GLdouble y);
		extern void (CODEGEN_FUNCPTR *Vertex2dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *Vertex2f)(GLfloat x, GLfloat y);
		extern void (CODEGEN_FUNCPTR *Vertex2fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *Vertex2i)(GLint x, GLint y);
		extern void (CODEGEN_FUNCPTR *Vertex2iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *Vertex2s)(GLshort x, GLshort y);
		extern void (CODEGEN_FUNCPTR *Vertex2sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *Vertex3d)(GLdouble x, GLdouble y, GLdouble z);
		extern void (CODEGEN_FUNCPTR *Vertex3dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *Vertex3f)(GLfloat x, GLfloat y, GLfloat z);
		extern void (CODEGEN_FUNCPTR *Vertex3fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *Vertex3i)(GLint x, GLint y, GLint z);
		extern void (CODEGEN_FUNCPTR *Vertex3iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *Vertex3s)(GLshort x, GLshort y, GLshort z);
		extern void (CODEGEN_FUNCPTR *Vertex3sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *Vertex4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
		extern void (CODEGEN_FUNCPTR *Vertex4dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *Vertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
		extern void (CODEGEN_FUNCPTR *Vertex4fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *Vertex4i)(GLint x, GLint y, GLint z, GLint w);
		extern void (CODEGEN_FUNCPTR *Vertex4iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *Vertex4s)(GLshort x, GLshort y, GLshort z, GLshort w);
		extern void (CODEGEN_FUNCPTR *Vertex4sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *Viewport)(GLint x, GLint y, GLsizei width, GLsizei height);
		
		extern GLboolean (CODEGEN_FUNCPTR *AreTexturesResident)(GLsizei n, const GLuint * textures, GLboolean * residences);
		extern void (CODEGEN_FUNCPTR *ArrayElement)(GLint i);
		extern void (CODEGEN_FUNCPTR *BindTexture)(GLenum target, GLuint texture);
		extern void (CODEGEN_FUNCPTR *ColorPointer)(GLint size, GLenum type, GLsizei stride, const void * pointer);
		extern void (CODEGEN_FUNCPTR *CopyTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
		extern void (CODEGEN_FUNCPTR *CopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
		extern void (CODEGEN_FUNCPTR *CopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
		extern void (CODEGEN_FUNCPTR *CopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *DeleteTextures)(GLsizei n, const GLuint * textures);
		extern void (CODEGEN_FUNCPTR *DisableClientState)(GLenum ren_array);
		extern void (CODEGEN_FUNCPTR *DrawArrays)(GLenum mode, GLint first, GLsizei count);
		extern void (CODEGEN_FUNCPTR *DrawElements)(GLenum mode, GLsizei count, GLenum type, const void * indices);
		extern void (CODEGEN_FUNCPTR *EdgeFlagPointer)(GLsizei stride, const void * pointer);
		extern void (CODEGEN_FUNCPTR *EnableClientState)(GLenum ren_array);
		extern void (CODEGEN_FUNCPTR *GenTextures)(GLsizei n, GLuint * textures);
		extern void (CODEGEN_FUNCPTR *GetPointerv)(GLenum pname, void ** params);
		extern void (CODEGEN_FUNCPTR *IndexPointer)(GLenum type, GLsizei stride, const void * pointer);
		extern void (CODEGEN_FUNCPTR *Indexub)(GLubyte c);
		extern void (CODEGEN_FUNCPTR *Indexubv)(const GLubyte * c);
		extern void (CODEGEN_FUNCPTR *InterleavedArrays)(GLenum format, GLsizei stride, const void * pointer);
		extern GLboolean (CODEGEN_FUNCPTR *IsTexture)(GLuint texture);
		extern void (CODEGEN_FUNCPTR *NormalPointer)(GLenum type, GLsizei stride, const void * pointer);
		extern void (CODEGEN_FUNCPTR *PolygonOffset)(GLfloat factor, GLfloat units);
		extern void (CODEGEN_FUNCPTR *PopClientAttrib)(void);
		extern void (CODEGEN_FUNCPTR *PrioritizeTextures)(GLsizei n, const GLuint * textures, const GLfloat * priorities);
		extern void (CODEGEN_FUNCPTR *PushClientAttrib)(GLbitfield mask);
		extern void (CODEGEN_FUNCPTR *TexCoordPointer)(GLint size, GLenum type, GLsizei stride, const void * pointer);
		extern void (CODEGEN_FUNCPTR *TexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *TexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *VertexPointer)(GLint size, GLenum type, GLsizei stride, const void * pointer);
		
		extern void (CODEGEN_FUNCPTR *CopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *DrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void * indices);
		extern void (CODEGEN_FUNCPTR *TexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *TexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void * pixels);
		
		extern void (CODEGEN_FUNCPTR *ActiveTexture)(GLenum texture);
		extern void (CODEGEN_FUNCPTR *ClientActiveTexture)(GLenum texture);
		extern void (CODEGEN_FUNCPTR *CompressedTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *CompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *CompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *CompressedTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *CompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *CompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *GetCompressedTexImage)(GLenum target, GLint level, void * img);
		extern void (CODEGEN_FUNCPTR *LoadTransposeMatrixd)(const GLdouble * m);
		extern void (CODEGEN_FUNCPTR *LoadTransposeMatrixf)(const GLfloat * m);
		extern void (CODEGEN_FUNCPTR *MultTransposeMatrixd)(const GLdouble * m);
		extern void (CODEGEN_FUNCPTR *MultTransposeMatrixf)(const GLfloat * m);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord1d)(GLenum target, GLdouble s);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord1dv)(GLenum target, const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord1f)(GLenum target, GLfloat s);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord1fv)(GLenum target, const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord1i)(GLenum target, GLint s);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord1iv)(GLenum target, const GLint * v);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord1s)(GLenum target, GLshort s);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord1sv)(GLenum target, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord2d)(GLenum target, GLdouble s, GLdouble t);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord2dv)(GLenum target, const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord2f)(GLenum target, GLfloat s, GLfloat t);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord2fv)(GLenum target, const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord2i)(GLenum target, GLint s, GLint t);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord2iv)(GLenum target, const GLint * v);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord2s)(GLenum target, GLshort s, GLshort t);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord2sv)(GLenum target, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord3d)(GLenum target, GLdouble s, GLdouble t, GLdouble r);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord3dv)(GLenum target, const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord3f)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord3fv)(GLenum target, const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord3i)(GLenum target, GLint s, GLint t, GLint r);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord3iv)(GLenum target, const GLint * v);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord3s)(GLenum target, GLshort s, GLshort t, GLshort r);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord3sv)(GLenum target, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord4d)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord4dv)(GLenum target, const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord4f)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord4fv)(GLenum target, const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord4i)(GLenum target, GLint s, GLint t, GLint r, GLint q);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord4iv)(GLenum target, const GLint * v);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord4s)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
		extern void (CODEGEN_FUNCPTR *MultiTexCoord4sv)(GLenum target, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *SampleCoverage)(GLfloat value, GLboolean invert);
		
		extern void (CODEGEN_FUNCPTR *BlendColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		extern void (CODEGEN_FUNCPTR *BlendEquation)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *BlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
		extern void (CODEGEN_FUNCPTR *FogCoordPointer)(GLenum type, GLsizei stride, const void * pointer);
		extern void (CODEGEN_FUNCPTR *FogCoordd)(GLdouble coord);
		extern void (CODEGEN_FUNCPTR *FogCoorddv)(const GLdouble * coord);
		extern void (CODEGEN_FUNCPTR *FogCoordf)(GLfloat coord);
		extern void (CODEGEN_FUNCPTR *FogCoordfv)(const GLfloat * coord);
		extern void (CODEGEN_FUNCPTR *MultiDrawArrays)(GLenum mode, const GLint * first, const GLsizei * count, GLsizei drawcount);
		extern void (CODEGEN_FUNCPTR *MultiDrawElements)(GLenum mode, const GLsizei * count, GLenum type, const void *const* indices, GLsizei drawcount);
		extern void (CODEGEN_FUNCPTR *PointParameterf)(GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *PointParameterfv)(GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *PointParameteri)(GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *PointParameteriv)(GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *SecondaryColor3b)(GLbyte red, GLbyte green, GLbyte blue);
		extern void (CODEGEN_FUNCPTR *SecondaryColor3bv)(const GLbyte * v);
		extern void (CODEGEN_FUNCPTR *SecondaryColor3d)(GLdouble red, GLdouble green, GLdouble blue);
		extern void (CODEGEN_FUNCPTR *SecondaryColor3dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *SecondaryColor3f)(GLfloat red, GLfloat green, GLfloat blue);
		extern void (CODEGEN_FUNCPTR *SecondaryColor3fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *SecondaryColor3i)(GLint red, GLint green, GLint blue);
		extern void (CODEGEN_FUNCPTR *SecondaryColor3iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *SecondaryColor3s)(GLshort red, GLshort green, GLshort blue);
		extern void (CODEGEN_FUNCPTR *SecondaryColor3sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *SecondaryColor3ub)(GLubyte red, GLubyte green, GLubyte blue);
		extern void (CODEGEN_FUNCPTR *SecondaryColor3ubv)(const GLubyte * v);
		extern void (CODEGEN_FUNCPTR *SecondaryColor3ui)(GLuint red, GLuint green, GLuint blue);
		extern void (CODEGEN_FUNCPTR *SecondaryColor3uiv)(const GLuint * v);
		extern void (CODEGEN_FUNCPTR *SecondaryColor3us)(GLushort red, GLushort green, GLushort blue);
		extern void (CODEGEN_FUNCPTR *SecondaryColor3usv)(const GLushort * v);
		extern void (CODEGEN_FUNCPTR *SecondaryColorPointer)(GLint size, GLenum type, GLsizei stride, const void * pointer);
		extern void (CODEGEN_FUNCPTR *WindowPos2d)(GLdouble x, GLdouble y);
		extern void (CODEGEN_FUNCPTR *WindowPos2dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *WindowPos2f)(GLfloat x, GLfloat y);
		extern void (CODEGEN_FUNCPTR *WindowPos2fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *WindowPos2i)(GLint x, GLint y);
		extern void (CODEGEN_FUNCPTR *WindowPos2iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *WindowPos2s)(GLshort x, GLshort y);
		extern void (CODEGEN_FUNCPTR *WindowPos2sv)(const GLshort * v);
		extern void (CODEGEN_FUNCPTR *WindowPos3d)(GLdouble x, GLdouble y, GLdouble z);
		extern void (CODEGEN_FUNCPTR *WindowPos3dv)(const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *WindowPos3f)(GLfloat x, GLfloat y, GLfloat z);
		extern void (CODEGEN_FUNCPTR *WindowPos3fv)(const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *WindowPos3i)(GLint x, GLint y, GLint z);
		extern void (CODEGEN_FUNCPTR *WindowPos3iv)(const GLint * v);
		extern void (CODEGEN_FUNCPTR *WindowPos3s)(GLshort x, GLshort y, GLshort z);
		extern void (CODEGEN_FUNCPTR *WindowPos3sv)(const GLshort * v);
		
		extern void (CODEGEN_FUNCPTR *BeginQuery)(GLenum target, GLuint id);
		extern void (CODEGEN_FUNCPTR *BindBuffer)(GLenum target, GLuint buffer);
		extern void (CODEGEN_FUNCPTR *BufferData)(GLenum target, GLsizeiptr size, const void * data, GLenum usage);
		extern void (CODEGEN_FUNCPTR *BufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const void * data);
		extern void (CODEGEN_FUNCPTR *DeleteBuffers)(GLsizei n, const GLuint * buffers);
		extern void (CODEGEN_FUNCPTR *DeleteQueries)(GLsizei n, const GLuint * ids);
		extern void (CODEGEN_FUNCPTR *EndQuery)(GLenum target);
		extern void (CODEGEN_FUNCPTR *GenBuffers)(GLsizei n, GLuint * buffers);
		extern void (CODEGEN_FUNCPTR *GenQueries)(GLsizei n, GLuint * ids);
		extern void (CODEGEN_FUNCPTR *GetBufferParameteriv)(GLenum target, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *GetBufferPointerv)(GLenum target, GLenum pname, void ** params);
		extern void (CODEGEN_FUNCPTR *GetBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, void * data);
		extern void (CODEGEN_FUNCPTR *GetQueryObjectiv)(GLuint id, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *GetQueryObjectuiv)(GLuint id, GLenum pname, GLuint * params);
		extern void (CODEGEN_FUNCPTR *GetQueryiv)(GLenum target, GLenum pname, GLint * params);
		extern GLboolean (CODEGEN_FUNCPTR *IsBuffer)(GLuint buffer);
		extern GLboolean (CODEGEN_FUNCPTR *IsQuery)(GLuint id);
		extern void * (CODEGEN_FUNCPTR *MapBuffer)(GLenum target, GLenum access);
		extern GLboolean (CODEGEN_FUNCPTR *UnmapBuffer)(GLenum target);
		
		extern void (CODEGEN_FUNCPTR *AttachShader)(GLuint program, GLuint shader);
		extern void (CODEGEN_FUNCPTR *BindAttribLocation)(GLuint program, GLuint index, const GLchar * name);
		extern void (CODEGEN_FUNCPTR *BlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha);
		extern void (CODEGEN_FUNCPTR *CompileShader)(GLuint shader);
		extern GLuint (CODEGEN_FUNCPTR *CreateProgram)(void);
		extern GLuint (CODEGEN_FUNCPTR *CreateShader)(GLenum type);
		extern void (CODEGEN_FUNCPTR *DeleteProgram)(GLuint program);
		extern void (CODEGEN_FUNCPTR *DeleteShader)(GLuint shader);
		extern void (CODEGEN_FUNCPTR *DetachShader)(GLuint program, GLuint shader);
		extern void (CODEGEN_FUNCPTR *DisableVertexAttribArray)(GLuint index);
		extern void (CODEGEN_FUNCPTR *DrawBuffers)(GLsizei n, const GLenum * bufs);
		extern void (CODEGEN_FUNCPTR *EnableVertexAttribArray)(GLuint index);
		extern void (CODEGEN_FUNCPTR *GetActiveAttrib)(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name);
		extern void (CODEGEN_FUNCPTR *GetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name);
		extern void (CODEGEN_FUNCPTR *GetAttachedShaders)(GLuint program, GLsizei maxCount, GLsizei * count, GLuint * shaders);
		extern GLint (CODEGEN_FUNCPTR *GetAttribLocation)(GLuint program, const GLchar * name);
		extern void (CODEGEN_FUNCPTR *GetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei * length, GLchar * infoLog);
		extern void (CODEGEN_FUNCPTR *GetProgramiv)(GLuint program, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *GetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * infoLog);
		extern void (CODEGEN_FUNCPTR *GetShaderSource)(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * source);
		extern void (CODEGEN_FUNCPTR *GetShaderiv)(GLuint shader, GLenum pname, GLint * params);
		extern GLint (CODEGEN_FUNCPTR *GetUniformLocation)(GLuint program, const GLchar * name);
		extern void (CODEGEN_FUNCPTR *GetUniformfv)(GLuint program, GLint location, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *GetUniformiv)(GLuint program, GLint location, GLint * params);
		extern void (CODEGEN_FUNCPTR *GetVertexAttribPointerv)(GLuint index, GLenum pname, void ** pointer);
		extern void (CODEGEN_FUNCPTR *GetVertexAttribdv)(GLuint index, GLenum pname, GLdouble * params);
		extern void (CODEGEN_FUNCPTR *GetVertexAttribfv)(GLuint index, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *GetVertexAttribiv)(GLuint index, GLenum pname, GLint * params);
		extern GLboolean (CODEGEN_FUNCPTR *IsProgram)(GLuint program);
		extern GLboolean (CODEGEN_FUNCPTR *IsShader)(GLuint shader);
		extern void (CODEGEN_FUNCPTR *LinkProgram)(GLuint program);
		extern void (CODEGEN_FUNCPTR *ShaderSource)(GLuint shader, GLsizei count, const GLchar *const* string, const GLint * length);
		extern void (CODEGEN_FUNCPTR *StencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask);
		extern void (CODEGEN_FUNCPTR *StencilMaskSeparate)(GLenum face, GLuint mask);
		extern void (CODEGEN_FUNCPTR *StencilOpSeparate)(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
		extern void (CODEGEN_FUNCPTR *Uniform1f)(GLint location, GLfloat v0);
		extern void (CODEGEN_FUNCPTR *Uniform1fv)(GLint location, GLsizei count, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *Uniform1i)(GLint location, GLint v0);
		extern void (CODEGEN_FUNCPTR *Uniform1iv)(GLint location, GLsizei count, const GLint * value);
		extern void (CODEGEN_FUNCPTR *Uniform2f)(GLint location, GLfloat v0, GLfloat v1);
		extern void (CODEGEN_FUNCPTR *Uniform2fv)(GLint location, GLsizei count, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *Uniform2i)(GLint location, GLint v0, GLint v1);
		extern void (CODEGEN_FUNCPTR *Uniform2iv)(GLint location, GLsizei count, const GLint * value);
		extern void (CODEGEN_FUNCPTR *Uniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
		extern void (CODEGEN_FUNCPTR *Uniform3fv)(GLint location, GLsizei count, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *Uniform3i)(GLint location, GLint v0, GLint v1, GLint v2);
		extern void (CODEGEN_FUNCPTR *Uniform3iv)(GLint location, GLsizei count, const GLint * value);
		extern void (CODEGEN_FUNCPTR *Uniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
		extern void (CODEGEN_FUNCPTR *Uniform4fv)(GLint location, GLsizei count, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *Uniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
		extern void (CODEGEN_FUNCPTR *Uniform4iv)(GLint location, GLsizei count, const GLint * value);
		extern void (CODEGEN_FUNCPTR *UniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *UniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *UniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *UseProgram)(GLuint program);
		extern void (CODEGEN_FUNCPTR *ValidateProgram)(GLuint program);
		extern void (CODEGEN_FUNCPTR *VertexAttrib1d)(GLuint index, GLdouble x);
		extern void (CODEGEN_FUNCPTR *VertexAttrib1dv)(GLuint index, const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib1f)(GLuint index, GLfloat x);
		extern void (CODEGEN_FUNCPTR *VertexAttrib1fv)(GLuint index, const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib1s)(GLuint index, GLshort x);
		extern void (CODEGEN_FUNCPTR *VertexAttrib1sv)(GLuint index, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib2d)(GLuint index, GLdouble x, GLdouble y);
		extern void (CODEGEN_FUNCPTR *VertexAttrib2dv)(GLuint index, const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib2f)(GLuint index, GLfloat x, GLfloat y);
		extern void (CODEGEN_FUNCPTR *VertexAttrib2fv)(GLuint index, const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib2s)(GLuint index, GLshort x, GLshort y);
		extern void (CODEGEN_FUNCPTR *VertexAttrib2sv)(GLuint index, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
		extern void (CODEGEN_FUNCPTR *VertexAttrib3dv)(GLuint index, const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib3f)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
		extern void (CODEGEN_FUNCPTR *VertexAttrib3fv)(GLuint index, const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib3s)(GLuint index, GLshort x, GLshort y, GLshort z);
		extern void (CODEGEN_FUNCPTR *VertexAttrib3sv)(GLuint index, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4Nbv)(GLuint index, const GLbyte * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4Niv)(GLuint index, const GLint * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4Nsv)(GLuint index, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4Nub)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4Nubv)(GLuint index, const GLubyte * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4Nuiv)(GLuint index, const GLuint * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4Nusv)(GLuint index, const GLushort * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4bv)(GLuint index, const GLbyte * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4dv)(GLuint index, const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4f)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4fv)(GLuint index, const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4iv)(GLuint index, const GLint * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4s)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4sv)(GLuint index, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4ubv)(GLuint index, const GLubyte * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4uiv)(GLuint index, const GLuint * v);
		extern void (CODEGEN_FUNCPTR *VertexAttrib4usv)(GLuint index, const GLushort * v);
		extern void (CODEGEN_FUNCPTR *VertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer);
		
		extern void (CODEGEN_FUNCPTR *UniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *UniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *UniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *UniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *UniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *UniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		
		namespace sys
		{
			
			exts::LoadTest LoadFunctions();
			
			int GetMinorVersion();
			int GetMajorVersion();
			bool IsVersionGEQ(int majorVersion, int minorVersion);
			
		} //namespace sys
	} //namespace gl
} //namespace gl21
#endif //GL21_POINTER_CPP_GENERATED_HEADEROPENGL_HPP
