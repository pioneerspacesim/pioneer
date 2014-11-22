#ifndef POINTER_CPP_GENERATED_HEADEROPENGL_HPP
#define POINTER_CPP_GENERATED_HEADEROPENGL_HPP

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
		
		extern LoadTest var_EXT_texture_compression_s3tc;
		extern LoadTest var_EXT_texture_sRGB;
		extern LoadTest var_EXT_texture_filter_anisotropic;
		
	} //namespace exts
	enum
	{
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
		
		ALPHA                            = 0x1906,
		ALWAYS                           = 0x0207,
		AND                              = 0x1501,
		AND_INVERTED                     = 0x1504,
		AND_REVERSE                      = 0x1502,
		BACK                             = 0x0405,
		BACK_LEFT                        = 0x0402,
		BACK_RIGHT                       = 0x0403,
		BLEND                            = 0x0BE2,
		BLEND_DST                        = 0x0BE0,
		BLEND_SRC                        = 0x0BE1,
		BLUE                             = 0x1905,
		BYTE                             = 0x1400,
		CCW                              = 0x0901,
		CLEAR                            = 0x1500,
		COLOR                            = 0x1800,
		COLOR_BUFFER_BIT                 = 0x00004000,
		COLOR_CLEAR_VALUE                = 0x0C22,
		COLOR_LOGIC_OP                   = 0x0BF2,
		COLOR_WRITEMASK                  = 0x0C23,
		COPY                             = 0x1503,
		COPY_INVERTED                    = 0x150C,
		CULL_FACE                        = 0x0B44,
		CULL_FACE_MODE                   = 0x0B45,
		CW                               = 0x0900,
		DECR                             = 0x1E03,
		DEPTH                            = 0x1801,
		DEPTH_BUFFER_BIT                 = 0x00000100,
		DEPTH_CLEAR_VALUE                = 0x0B73,
		DEPTH_COMPONENT                  = 0x1902,
		DEPTH_FUNC                       = 0x0B74,
		DEPTH_RANGE                      = 0x0B70,
		DEPTH_TEST                       = 0x0B71,
		DEPTH_WRITEMASK                  = 0x0B72,
		DITHER                           = 0x0BD0,
		DONT_CARE                        = 0x1100,
		DOUBLE                           = 0x140A,
		DOUBLEBUFFER                     = 0x0C32,
		DRAW_BUFFER                      = 0x0C01,
		DST_ALPHA                        = 0x0304,
		DST_COLOR                        = 0x0306,
		EQUAL                            = 0x0202,
		EQUIV                            = 0x1509,
		EXTENSIONS                       = 0x1F03,
		FALSE_                           = 0,
		FASTEST                          = 0x1101,
		FILL                             = 0x1B02,
		FLOAT                            = 0x1406,
		FRONT                            = 0x0404,
		FRONT_AND_BACK                   = 0x0408,
		FRONT_FACE                       = 0x0B46,
		FRONT_LEFT                       = 0x0400,
		FRONT_RIGHT                      = 0x0401,
		GEQUAL                           = 0x0206,
		GREATER                          = 0x0204,
		GREEN                            = 0x1904,
		INCR                             = 0x1E02,
		INT                              = 0x1404,
		INVALID_ENUM                     = 0x0500,
		INVALID_OPERATION                = 0x0502,
		INVALID_VALUE                    = 0x0501,
		INVERT                           = 0x150A,
		KEEP                             = 0x1E00,
		LEFT                             = 0x0406,
		LEQUAL                           = 0x0203,
		LESS                             = 0x0201,
		LINE                             = 0x1B01,
		LINEAR                           = 0x2601,
		LINEAR_MIPMAP_LINEAR             = 0x2703,
		LINEAR_MIPMAP_NEAREST            = 0x2701,
		LINES                            = 0x0001,
		LINE_LOOP                        = 0x0002,
		LINE_SMOOTH                      = 0x0B20,
		LINE_SMOOTH_HINT                 = 0x0C52,
		LINE_STRIP                       = 0x0003,
		LINE_WIDTH                       = 0x0B21,
		LINE_WIDTH_GRANULARITY           = 0x0B23,
		LINE_WIDTH_RANGE                 = 0x0B22,
		LOGIC_OP_MODE                    = 0x0BF0,
		MAX_TEXTURE_SIZE                 = 0x0D33,
		MAX_VIEWPORT_DIMS                = 0x0D3A,
		NAND                             = 0x150E,
		NEAREST                          = 0x2600,
		NEAREST_MIPMAP_LINEAR            = 0x2702,
		NEAREST_MIPMAP_NEAREST           = 0x2700,
		NEVER                            = 0x0200,
		NICEST                           = 0x1102,
		NONE                             = 0,
		NOOP                             = 0x1505,
		NOR                              = 0x1508,
		NOTEQUAL                         = 0x0205,
		NO_ERROR_                        = 0,
		ONE                              = 1,
		ONE_MINUS_DST_ALPHA              = 0x0305,
		ONE_MINUS_DST_COLOR              = 0x0307,
		ONE_MINUS_SRC_ALPHA              = 0x0303,
		ONE_MINUS_SRC_COLOR              = 0x0301,
		OR                               = 0x1507,
		OR_INVERTED                      = 0x150D,
		OR_REVERSE                       = 0x150B,
		OUT_OF_MEMORY                    = 0x0505,
		PACK_ALIGNMENT                   = 0x0D05,
		PACK_LSB_FIRST                   = 0x0D01,
		PACK_ROW_LENGTH                  = 0x0D02,
		PACK_SKIP_PIXELS                 = 0x0D04,
		PACK_SKIP_ROWS                   = 0x0D03,
		PACK_SWAP_BYTES                  = 0x0D00,
		POINT                            = 0x1B00,
		POINTS                           = 0x0000,
		POINT_SIZE                       = 0x0B11,
		POINT_SIZE_GRANULARITY           = 0x0B13,
		POINT_SIZE_RANGE                 = 0x0B12,
		POLYGON_MODE                     = 0x0B40,
		POLYGON_OFFSET_FACTOR            = 0x8038,
		POLYGON_OFFSET_FILL              = 0x8037,
		POLYGON_OFFSET_LINE              = 0x2A02,
		POLYGON_OFFSET_POINT             = 0x2A01,
		POLYGON_OFFSET_UNITS             = 0x2A00,
		POLYGON_SMOOTH                   = 0x0B41,
		POLYGON_SMOOTH_HINT              = 0x0C53,
		PROXY_TEXTURE_1D                 = 0x8063,
		PROXY_TEXTURE_2D                 = 0x8064,
		QUADS                            = 0x0007,
		R3_G3_B2                         = 0x2A10,
		READ_BUFFER                      = 0x0C02,
		RED                              = 0x1903,
		RENDERER                         = 0x1F01,
		REPEAT                           = 0x2901,
		REPLACE                          = 0x1E01,
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
		RIGHT                            = 0x0407,
		SCISSOR_BOX                      = 0x0C10,
		SCISSOR_TEST                     = 0x0C11,
		SET                              = 0x150F,
		SHORT                            = 0x1402,
		SRC_ALPHA                        = 0x0302,
		SRC_ALPHA_SATURATE               = 0x0308,
		SRC_COLOR                        = 0x0300,
		STENCIL                          = 0x1802,
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
		TEXTURE                          = 0x1702,
		TEXTURE_1D                       = 0x0DE0,
		TEXTURE_2D                       = 0x0DE1,
		TEXTURE_ALPHA_SIZE               = 0x805F,
		TEXTURE_BINDING_1D               = 0x8068,
		TEXTURE_BINDING_2D               = 0x8069,
		TEXTURE_BLUE_SIZE                = 0x805E,
		TEXTURE_BORDER_COLOR             = 0x1004,
		TEXTURE_GREEN_SIZE               = 0x805D,
		TEXTURE_HEIGHT                   = 0x1001,
		TEXTURE_INTERNAL_FORMAT          = 0x1003,
		TEXTURE_MAG_FILTER               = 0x2800,
		TEXTURE_MIN_FILTER               = 0x2801,
		TEXTURE_RED_SIZE                 = 0x805C,
		TEXTURE_WIDTH                    = 0x1000,
		TEXTURE_WRAP_S                   = 0x2802,
		TEXTURE_WRAP_T                   = 0x2803,
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
		VENDOR                           = 0x1F00,
		VERSION                          = 0x1F02,
		VIEWPORT                         = 0x0BA2,
		XOR                              = 0x1506,
		ZERO                             = 0,
		
		ALIASED_LINE_WIDTH_RANGE         = 0x846E,
		BGR                              = 0x80E0,
		BGRA                             = 0x80E1,
		CLAMP_TO_EDGE                    = 0x812F,
		MAX_3D_TEXTURE_SIZE              = 0x8073,
		MAX_ELEMENTS_INDICES             = 0x80E9,
		MAX_ELEMENTS_VERTICES            = 0x80E8,
		PACK_IMAGE_HEIGHT                = 0x806C,
		PACK_SKIP_IMAGES                 = 0x806B,
		PROXY_TEXTURE_3D                 = 0x8070,
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
		CLAMP_TO_BORDER                  = 0x812D,
		COMPRESSED_RGB                   = 0x84ED,
		COMPRESSED_RGBA                  = 0x84EE,
		COMPRESSED_TEXTURE_FORMATS       = 0x86A3,
		MAX_CUBE_MAP_TEXTURE_SIZE        = 0x851C,
		MULTISAMPLE                      = 0x809D,
		NUM_COMPRESSED_TEXTURE_FORMATS   = 0x86A2,
		PROXY_TEXTURE_CUBE_MAP           = 0x851B,
		SAMPLES                          = 0x80A9,
		SAMPLE_ALPHA_TO_COVERAGE         = 0x809E,
		SAMPLE_ALPHA_TO_ONE              = 0x809F,
		SAMPLE_BUFFERS                   = 0x80A8,
		SAMPLE_COVERAGE                  = 0x80A0,
		SAMPLE_COVERAGE_INVERT           = 0x80AB,
		SAMPLE_COVERAGE_VALUE            = 0x80AA,
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
		
		BLEND_COLOR                      = 0x8005,
		BLEND_DST_ALPHA                  = 0x80CA,
		BLEND_DST_RGB                    = 0x80C8,
		BLEND_EQUATION                   = 0x8009,
		BLEND_SRC_ALPHA                  = 0x80CB,
		BLEND_SRC_RGB                    = 0x80C9,
		CONSTANT_ALPHA                   = 0x8003,
		CONSTANT_COLOR                   = 0x8001,
		DECR_WRAP                        = 0x8508,
		DEPTH_COMPONENT16                = 0x81A5,
		DEPTH_COMPONENT24                = 0x81A6,
		DEPTH_COMPONENT32                = 0x81A7,
		FUNC_ADD                         = 0x8006,
		FUNC_REVERSE_SUBTRACT            = 0x800B,
		FUNC_SUBTRACT                    = 0x800A,
		INCR_WRAP                        = 0x8507,
		MAX                              = 0x8008,
		MAX_TEXTURE_LOD_BIAS             = 0x84FD,
		MIN                              = 0x8007,
		MIRRORED_REPEAT                  = 0x8370,
		ONE_MINUS_CONSTANT_ALPHA         = 0x8004,
		ONE_MINUS_CONSTANT_COLOR         = 0x8002,
		POINT_FADE_THRESHOLD_SIZE        = 0x8128,
		TEXTURE_COMPARE_FUNC             = 0x884D,
		TEXTURE_COMPARE_MODE             = 0x884C,
		TEXTURE_DEPTH_SIZE               = 0x884A,
		TEXTURE_LOD_BIAS                 = 0x8501,
		
		ARRAY_BUFFER                     = 0x8892,
		ARRAY_BUFFER_BINDING             = 0x8894,
		BUFFER_ACCESS                    = 0x88BB,
		BUFFER_MAPPED                    = 0x88BC,
		BUFFER_MAP_POINTER               = 0x88BD,
		BUFFER_SIZE                      = 0x8764,
		BUFFER_USAGE                     = 0x8765,
		CURRENT_QUERY                    = 0x8865,
		DYNAMIC_COPY                     = 0x88EA,
		DYNAMIC_DRAW                     = 0x88E8,
		DYNAMIC_READ                     = 0x88E9,
		ELEMENT_ARRAY_BUFFER             = 0x8893,
		ELEMENT_ARRAY_BUFFER_BINDING     = 0x8895,
		QUERY_COUNTER_BITS               = 0x8864,
		QUERY_RESULT                     = 0x8866,
		QUERY_RESULT_AVAILABLE           = 0x8867,
		READ_ONLY                        = 0x88B8,
		READ_WRITE                       = 0x88BA,
		SAMPLES_PASSED                   = 0x8914,
		SRC1_ALPHA                       = 0x8589,
		STATIC_COPY                      = 0x88E6,
		STATIC_DRAW                      = 0x88E4,
		STATIC_READ                      = 0x88E5,
		STREAM_COPY                      = 0x88E2,
		STREAM_DRAW                      = 0x88E0,
		STREAM_READ                      = 0x88E1,
		VERTEX_ATTRIB_ARRAY_BUFFER_BINDING = 0x889F,
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
		MAX_TEXTURE_IMAGE_UNITS          = 0x8872,
		MAX_VARYING_FLOATS               = 0x8B4B,
		MAX_VERTEX_ATTRIBS               = 0x8869,
		MAX_VERTEX_TEXTURE_IMAGE_UNITS   = 0x8B4C,
		MAX_VERTEX_UNIFORM_COMPONENTS    = 0x8B4A,
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
		VERTEX_SHADER                    = 0x8B31,
		
		COMPRESSED_SRGB                  = 0x8C48,
		COMPRESSED_SRGB_ALPHA            = 0x8C49,
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
		SRGB                             = 0x8C40,
		SRGB8                            = 0x8C41,
		SRGB8_ALPHA8                     = 0x8C43,
		SRGB_ALPHA                       = 0x8C42,
		
		BGRA_INTEGER                     = 0x8D9B,
		BGR_INTEGER                      = 0x8D9A,
		BLUE_INTEGER                     = 0x8D96,
		BUFFER_ACCESS_FLAGS              = 0x911F,
		BUFFER_MAP_LENGTH                = 0x9120,
		BUFFER_MAP_OFFSET                = 0x9121,
		CLAMP_READ_COLOR                 = 0x891C,
		CLIP_DISTANCE0                   = 0x3000,
		CLIP_DISTANCE1                   = 0x3001,
		CLIP_DISTANCE2                   = 0x3002,
		CLIP_DISTANCE3                   = 0x3003,
		CLIP_DISTANCE4                   = 0x3004,
		CLIP_DISTANCE5                   = 0x3005,
		CLIP_DISTANCE6                   = 0x3006,
		CLIP_DISTANCE7                   = 0x3007,
		COLOR_ATTACHMENT0                = 0x8CE0,
		COLOR_ATTACHMENT1                = 0x8CE1,
		COLOR_ATTACHMENT10               = 0x8CEA,
		COLOR_ATTACHMENT11               = 0x8CEB,
		COLOR_ATTACHMENT12               = 0x8CEC,
		COLOR_ATTACHMENT13               = 0x8CED,
		COLOR_ATTACHMENT14               = 0x8CEE,
		COLOR_ATTACHMENT15               = 0x8CEF,
		COLOR_ATTACHMENT2                = 0x8CE2,
		COLOR_ATTACHMENT3                = 0x8CE3,
		COLOR_ATTACHMENT4                = 0x8CE4,
		COLOR_ATTACHMENT5                = 0x8CE5,
		COLOR_ATTACHMENT6                = 0x8CE6,
		COLOR_ATTACHMENT7                = 0x8CE7,
		COLOR_ATTACHMENT8                = 0x8CE8,
		COLOR_ATTACHMENT9                = 0x8CE9,
		COMPARE_REF_TO_TEXTURE           = 0x884E,
		COMPRESSED_RED                   = 0x8225,
		COMPRESSED_RED_RGTC1             = 0x8DBB,
		COMPRESSED_RG                    = 0x8226,
		COMPRESSED_RG_RGTC2              = 0x8DBD,
		COMPRESSED_SIGNED_RED_RGTC1      = 0x8DBC,
		COMPRESSED_SIGNED_RG_RGTC2       = 0x8DBE,
		CONTEXT_FLAGS                    = 0x821E,
		CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT = 0x00000001,
		DEPTH24_STENCIL8                 = 0x88F0,
		DEPTH32F_STENCIL8                = 0x8CAD,
		DEPTH_ATTACHMENT                 = 0x8D00,
		DEPTH_COMPONENT32F               = 0x8CAC,
		DEPTH_STENCIL                    = 0x84F9,
		DEPTH_STENCIL_ATTACHMENT         = 0x821A,
		DRAW_FRAMEBUFFER                 = 0x8CA9,
		DRAW_FRAMEBUFFER_BINDING         = 0x8CA6,
		FIXED_ONLY                       = 0x891D,
		FLOAT_32_UNSIGNED_INT_24_8_REV   = 0x8DAD,
		FRAMEBUFFER                      = 0x8D40,
		FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE = 0x8215,
		FRAMEBUFFER_ATTACHMENT_BLUE_SIZE = 0x8214,
		FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING = 0x8210,
		FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE = 0x8211,
		FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE = 0x8216,
		FRAMEBUFFER_ATTACHMENT_GREEN_SIZE = 0x8213,
		FRAMEBUFFER_ATTACHMENT_OBJECT_NAME = 0x8CD1,
		FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE = 0x8CD0,
		FRAMEBUFFER_ATTACHMENT_RED_SIZE  = 0x8212,
		FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE = 0x8217,
		FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE = 0x8CD3,
		FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER = 0x8CD4,
		FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL = 0x8CD2,
		FRAMEBUFFER_BINDING              = 0x8CA6,
		FRAMEBUFFER_COMPLETE             = 0x8CD5,
		FRAMEBUFFER_DEFAULT              = 0x8218,
		FRAMEBUFFER_INCOMPLETE_ATTACHMENT = 0x8CD6,
		FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER = 0x8CDB,
		FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT = 0x8CD7,
		FRAMEBUFFER_INCOMPLETE_MULTISAMPLE = 0x8D56,
		FRAMEBUFFER_INCOMPLETE_READ_BUFFER = 0x8CDC,
		FRAMEBUFFER_SRGB                 = 0x8DB9,
		FRAMEBUFFER_UNDEFINED            = 0x8219,
		FRAMEBUFFER_UNSUPPORTED          = 0x8CDD,
		GREEN_INTEGER                    = 0x8D95,
		HALF_FLOAT                       = 0x140B,
		INTERLEAVED_ATTRIBS              = 0x8C8C,
		INT_SAMPLER_1D                   = 0x8DC9,
		INT_SAMPLER_1D_ARRAY             = 0x8DCE,
		INT_SAMPLER_2D                   = 0x8DCA,
		INT_SAMPLER_2D_ARRAY             = 0x8DCF,
		INT_SAMPLER_3D                   = 0x8DCB,
		INT_SAMPLER_CUBE                 = 0x8DCC,
		INVALID_FRAMEBUFFER_OPERATION    = 0x0506,
		MAJOR_VERSION                    = 0x821B,
		MAP_FLUSH_EXPLICIT_BIT           = 0x0010,
		MAP_INVALIDATE_BUFFER_BIT        = 0x0008,
		MAP_INVALIDATE_RANGE_BIT         = 0x0004,
		MAP_READ_BIT                     = 0x0001,
		MAP_UNSYNCHRONIZED_BIT           = 0x0020,
		MAP_WRITE_BIT                    = 0x0002,
		MAX_ARRAY_TEXTURE_LAYERS         = 0x88FF,
		MAX_CLIP_DISTANCES               = 0x0D32,
		MAX_COLOR_ATTACHMENTS            = 0x8CDF,
		MAX_PROGRAM_TEXEL_OFFSET         = 0x8905,
		MAX_RENDERBUFFER_SIZE            = 0x84E8,
		MAX_SAMPLES                      = 0x8D57,
		MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS = 0x8C8A,
		MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS = 0x8C8B,
		MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS = 0x8C80,
		MAX_VARYING_COMPONENTS           = 0x8B4B,
		MINOR_VERSION                    = 0x821C,
		MIN_PROGRAM_TEXEL_OFFSET         = 0x8904,
		NUM_EXTENSIONS                   = 0x821D,
		PRIMITIVES_GENERATED             = 0x8C87,
		PROXY_TEXTURE_1D_ARRAY           = 0x8C19,
		PROXY_TEXTURE_2D_ARRAY           = 0x8C1B,
		QUERY_BY_REGION_NO_WAIT          = 0x8E16,
		QUERY_BY_REGION_WAIT             = 0x8E15,
		QUERY_NO_WAIT                    = 0x8E14,
		QUERY_WAIT                       = 0x8E13,
		R11F_G11F_B10F                   = 0x8C3A,
		R16                              = 0x822A,
		R16F                             = 0x822D,
		R16I                             = 0x8233,
		R16UI                            = 0x8234,
		R32F                             = 0x822E,
		R32I                             = 0x8235,
		R32UI                            = 0x8236,
		R8                               = 0x8229,
		R8I                              = 0x8231,
		R8UI                             = 0x8232,
		RASTERIZER_DISCARD               = 0x8C89,
		READ_FRAMEBUFFER                 = 0x8CA8,
		READ_FRAMEBUFFER_BINDING         = 0x8CAA,
		RED_INTEGER                      = 0x8D94,
		RENDERBUFFER                     = 0x8D41,
		RENDERBUFFER_ALPHA_SIZE          = 0x8D53,
		RENDERBUFFER_BINDING             = 0x8CA7,
		RENDERBUFFER_BLUE_SIZE           = 0x8D52,
		RENDERBUFFER_DEPTH_SIZE          = 0x8D54,
		RENDERBUFFER_GREEN_SIZE          = 0x8D51,
		RENDERBUFFER_HEIGHT              = 0x8D43,
		RENDERBUFFER_INTERNAL_FORMAT     = 0x8D44,
		RENDERBUFFER_RED_SIZE            = 0x8D50,
		RENDERBUFFER_SAMPLES             = 0x8CAB,
		RENDERBUFFER_STENCIL_SIZE        = 0x8D55,
		RENDERBUFFER_WIDTH               = 0x8D42,
		RG                               = 0x8227,
		RG16                             = 0x822C,
		RG16F                            = 0x822F,
		RG16I                            = 0x8239,
		RG16UI                           = 0x823A,
		RG32F                            = 0x8230,
		RG32I                            = 0x823B,
		RG32UI                           = 0x823C,
		RG8                              = 0x822B,
		RG8I                             = 0x8237,
		RG8UI                            = 0x8238,
		RGB16F                           = 0x881B,
		RGB16I                           = 0x8D89,
		RGB16UI                          = 0x8D77,
		RGB32F                           = 0x8815,
		RGB32I                           = 0x8D83,
		RGB32UI                          = 0x8D71,
		RGB8I                            = 0x8D8F,
		RGB8UI                           = 0x8D7D,
		RGB9_E5                          = 0x8C3D,
		RGBA16F                          = 0x881A,
		RGBA16I                          = 0x8D88,
		RGBA16UI                         = 0x8D76,
		RGBA32F                          = 0x8814,
		RGBA32I                          = 0x8D82,
		RGBA32UI                         = 0x8D70,
		RGBA8I                           = 0x8D8E,
		RGBA8UI                          = 0x8D7C,
		RGBA_INTEGER                     = 0x8D99,
		RGB_INTEGER                      = 0x8D98,
		RG_INTEGER                       = 0x8228,
		SAMPLER_1D_ARRAY                 = 0x8DC0,
		SAMPLER_1D_ARRAY_SHADOW          = 0x8DC3,
		SAMPLER_2D_ARRAY                 = 0x8DC1,
		SAMPLER_2D_ARRAY_SHADOW          = 0x8DC4,
		SAMPLER_CUBE_SHADOW              = 0x8DC5,
		SEPARATE_ATTRIBS                 = 0x8C8D,
		STENCIL_ATTACHMENT               = 0x8D20,
		STENCIL_INDEX1                   = 0x8D46,
		STENCIL_INDEX16                  = 0x8D49,
		STENCIL_INDEX4                   = 0x8D47,
		STENCIL_INDEX8                   = 0x8D48,
		TEXTURE_1D_ARRAY                 = 0x8C18,
		TEXTURE_2D_ARRAY                 = 0x8C1A,
		TEXTURE_ALPHA_TYPE               = 0x8C13,
		TEXTURE_BINDING_1D_ARRAY         = 0x8C1C,
		TEXTURE_BINDING_2D_ARRAY         = 0x8C1D,
		TEXTURE_BLUE_TYPE                = 0x8C12,
		TEXTURE_DEPTH_TYPE               = 0x8C16,
		TEXTURE_GREEN_TYPE               = 0x8C11,
		TEXTURE_RED_TYPE                 = 0x8C10,
		TEXTURE_SHARED_SIZE              = 0x8C3F,
		TEXTURE_STENCIL_SIZE             = 0x88F1,
		TRANSFORM_FEEDBACK_BUFFER        = 0x8C8E,
		TRANSFORM_FEEDBACK_BUFFER_BINDING = 0x8C8F,
		TRANSFORM_FEEDBACK_BUFFER_MODE   = 0x8C7F,
		TRANSFORM_FEEDBACK_BUFFER_SIZE   = 0x8C85,
		TRANSFORM_FEEDBACK_BUFFER_START  = 0x8C84,
		TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN = 0x8C88,
		TRANSFORM_FEEDBACK_VARYINGS      = 0x8C83,
		TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH = 0x8C76,
		UNSIGNED_INT_10F_11F_11F_REV     = 0x8C3B,
		UNSIGNED_INT_24_8                = 0x84FA,
		UNSIGNED_INT_5_9_9_9_REV         = 0x8C3E,
		UNSIGNED_INT_SAMPLER_1D          = 0x8DD1,
		UNSIGNED_INT_SAMPLER_1D_ARRAY    = 0x8DD6,
		UNSIGNED_INT_SAMPLER_2D          = 0x8DD2,
		UNSIGNED_INT_SAMPLER_2D_ARRAY    = 0x8DD7,
		UNSIGNED_INT_SAMPLER_3D          = 0x8DD3,
		UNSIGNED_INT_SAMPLER_CUBE        = 0x8DD4,
		UNSIGNED_INT_VEC2                = 0x8DC6,
		UNSIGNED_INT_VEC3                = 0x8DC7,
		UNSIGNED_INT_VEC4                = 0x8DC8,
		UNSIGNED_NORMALIZED              = 0x8C17,
		VERTEX_ARRAY_BINDING             = 0x85B5,
		VERTEX_ATTRIB_ARRAY_INTEGER      = 0x88FD,
		
		ACTIVE_UNIFORM_BLOCKS            = 0x8A36,
		ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH = 0x8A35,
		COPY_READ_BUFFER                 = 0x8F36,
		COPY_WRITE_BUFFER                = 0x8F37,
		INT_SAMPLER_2D_RECT              = 0x8DCD,
		INT_SAMPLER_BUFFER               = 0x8DD0,
		INVALID_INDEX                    = 0xFFFFFFFF,
		MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS = 0x8A33,
		MAX_COMBINED_UNIFORM_BLOCKS      = 0x8A2E,
		MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS = 0x8A31,
		MAX_FRAGMENT_UNIFORM_BLOCKS      = 0x8A2D,
		MAX_RECTANGLE_TEXTURE_SIZE       = 0x84F8,
		MAX_TEXTURE_BUFFER_SIZE          = 0x8C2B,
		MAX_UNIFORM_BLOCK_SIZE           = 0x8A30,
		MAX_UNIFORM_BUFFER_BINDINGS      = 0x8A2F,
		MAX_VERTEX_UNIFORM_BLOCKS        = 0x8A2B,
		PRIMITIVE_RESTART                = 0x8F9D,
		PRIMITIVE_RESTART_INDEX          = 0x8F9E,
		PROXY_TEXTURE_RECTANGLE          = 0x84F7,
		R16_SNORM                        = 0x8F98,
		R8_SNORM                         = 0x8F94,
		RG16_SNORM                       = 0x8F99,
		RG8_SNORM                        = 0x8F95,
		RGB16_SNORM                      = 0x8F9A,
		RGB8_SNORM                       = 0x8F96,
		RGBA16_SNORM                     = 0x8F9B,
		RGBA8_SNORM                      = 0x8F97,
		SAMPLER_2D_RECT                  = 0x8B63,
		SAMPLER_2D_RECT_SHADOW           = 0x8B64,
		SAMPLER_BUFFER                   = 0x8DC2,
		SIGNED_NORMALIZED                = 0x8F9C,
		TEXTURE_BINDING_BUFFER           = 0x8C2C,
		TEXTURE_BINDING_RECTANGLE        = 0x84F6,
		TEXTURE_BUFFER                   = 0x8C2A,
		TEXTURE_BUFFER_DATA_STORE_BINDING = 0x8C2D,
		TEXTURE_RECTANGLE                = 0x84F5,
		UNIFORM_ARRAY_STRIDE             = 0x8A3C,
		UNIFORM_BLOCK_ACTIVE_UNIFORMS    = 0x8A42,
		UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES = 0x8A43,
		UNIFORM_BLOCK_BINDING            = 0x8A3F,
		UNIFORM_BLOCK_DATA_SIZE          = 0x8A40,
		UNIFORM_BLOCK_INDEX              = 0x8A3A,
		UNIFORM_BLOCK_NAME_LENGTH        = 0x8A41,
		UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER = 0x8A46,
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
		UNSIGNED_INT_SAMPLER_2D_RECT     = 0x8DD5,
		UNSIGNED_INT_SAMPLER_BUFFER      = 0x8DD8,
		
		ALREADY_SIGNALED                 = 0x911A,
		CONDITION_SATISFIED              = 0x911C,
		CONTEXT_COMPATIBILITY_PROFILE_BIT = 0x00000002,
		CONTEXT_CORE_PROFILE_BIT         = 0x00000001,
		CONTEXT_PROFILE_MASK             = 0x9126,
		DEPTH_CLAMP                      = 0x864F,
		FIRST_VERTEX_CONVENTION          = 0x8E4D,
		FRAMEBUFFER_ATTACHMENT_LAYERED   = 0x8DA7,
		FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS = 0x8DA8,
		GEOMETRY_INPUT_TYPE              = 0x8917,
		GEOMETRY_OUTPUT_TYPE             = 0x8918,
		GEOMETRY_SHADER                  = 0x8DD9,
		GEOMETRY_VERTICES_OUT            = 0x8916,
		INT_SAMPLER_2D_MULTISAMPLE       = 0x9109,
		INT_SAMPLER_2D_MULTISAMPLE_ARRAY = 0x910C,
		LAST_VERTEX_CONVENTION           = 0x8E4E,
		LINES_ADJACENCY                  = 0x000A,
		LINE_STRIP_ADJACENCY             = 0x000B,
		MAX_COLOR_TEXTURE_SAMPLES        = 0x910E,
		MAX_DEPTH_TEXTURE_SAMPLES        = 0x910F,
		MAX_FRAGMENT_INPUT_COMPONENTS    = 0x9125,
		MAX_GEOMETRY_INPUT_COMPONENTS    = 0x9123,
		MAX_GEOMETRY_OUTPUT_COMPONENTS   = 0x9124,
		MAX_GEOMETRY_OUTPUT_VERTICES     = 0x8DE0,
		MAX_GEOMETRY_TEXTURE_IMAGE_UNITS = 0x8C29,
		MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS = 0x8DE1,
		MAX_GEOMETRY_UNIFORM_COMPONENTS  = 0x8DDF,
		MAX_INTEGER_SAMPLES              = 0x9110,
		MAX_SAMPLE_MASK_WORDS            = 0x8E59,
		MAX_SERVER_WAIT_TIMEOUT          = 0x9111,
		MAX_VERTEX_OUTPUT_COMPONENTS     = 0x9122,
		OBJECT_TYPE                      = 0x9112,
		PROGRAM_POINT_SIZE               = 0x8642,
		PROVOKING_VERTEX                 = 0x8E4F,
		PROXY_TEXTURE_2D_MULTISAMPLE     = 0x9101,
		PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY = 0x9103,
		QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION = 0x8E4C,
		SAMPLER_2D_MULTISAMPLE           = 0x9108,
		SAMPLER_2D_MULTISAMPLE_ARRAY     = 0x910B,
		SAMPLE_MASK                      = 0x8E51,
		SAMPLE_MASK_VALUE                = 0x8E52,
		SAMPLE_POSITION                  = 0x8E50,
		SIGNALED                         = 0x9119,
		SYNC_CONDITION                   = 0x9113,
		SYNC_FENCE                       = 0x9116,
		SYNC_FLAGS                       = 0x9115,
		SYNC_FLUSH_COMMANDS_BIT          = 0x00000001,
		SYNC_GPU_COMMANDS_COMPLETE       = 0x9117,
		SYNC_STATUS                      = 0x9114,
		TEXTURE_2D_MULTISAMPLE           = 0x9100,
		TEXTURE_2D_MULTISAMPLE_ARRAY     = 0x9102,
		TEXTURE_BINDING_2D_MULTISAMPLE   = 0x9104,
		TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY = 0x9105,
		TEXTURE_CUBE_MAP_SEAMLESS        = 0x884F,
		TEXTURE_FIXED_SAMPLE_LOCATIONS   = 0x9107,
		TEXTURE_SAMPLES                  = 0x9106,
		TIMEOUT_EXPIRED                  = 0x911B,
		TIMEOUT_IGNORED                  = 0xFFFFFFFF,
		TRIANGLES_ADJACENCY              = 0x000C,
		TRIANGLE_STRIP_ADJACENCY         = 0x000D,
		UNSIGNALED                       = 0x9118,
		UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE = 0x910A,
		UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY = 0x910D,
		WAIT_FAILED_                     = 0x911D,
		
	};
	
	
	
	extern void (CODEGEN_FUNCPTR *BlendFunc)(GLenum sfactor, GLenum dfactor);
	extern void (CODEGEN_FUNCPTR *Clear)(GLbitfield mask);
	extern void (CODEGEN_FUNCPTR *ClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	extern void (CODEGEN_FUNCPTR *ClearDepth)(GLdouble depth);
	extern void (CODEGEN_FUNCPTR *ClearStencil)(GLint s);
	extern void (CODEGEN_FUNCPTR *ColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
	extern void (CODEGEN_FUNCPTR *CullFace)(GLenum mode);
	extern void (CODEGEN_FUNCPTR *DepthFunc)(GLenum func);
	extern void (CODEGEN_FUNCPTR *DepthMask)(GLboolean flag);
	extern void (CODEGEN_FUNCPTR *DepthRange)(GLdouble ren_near, GLdouble ren_far);
	extern void (CODEGEN_FUNCPTR *Disable)(GLenum cap);
	extern void (CODEGEN_FUNCPTR *DrawBuffer)(GLenum mode);
	extern void (CODEGEN_FUNCPTR *Enable)(GLenum cap);
	extern void (CODEGEN_FUNCPTR *Finish)();
	extern void (CODEGEN_FUNCPTR *Flush)();
	extern void (CODEGEN_FUNCPTR *FrontFace)(GLenum mode);
	extern void (CODEGEN_FUNCPTR *GetBooleanv)(GLenum pname, GLboolean * params);
	extern void (CODEGEN_FUNCPTR *GetDoublev)(GLenum pname, GLdouble * params);
	extern GLenum (CODEGEN_FUNCPTR *GetError)();
	extern void (CODEGEN_FUNCPTR *GetFloatv)(GLenum pname, GLfloat * params);
	extern void (CODEGEN_FUNCPTR *GetIntegerv)(GLenum pname, GLint * params);
	extern const GLubyte * (CODEGEN_FUNCPTR *GetString)(GLenum name);
	extern void (CODEGEN_FUNCPTR *GetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid * pixels);
	extern void (CODEGEN_FUNCPTR *GetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat * params);
	extern void (CODEGEN_FUNCPTR *GetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint * params);
	extern void (CODEGEN_FUNCPTR *GetTexParameterfv)(GLenum target, GLenum pname, GLfloat * params);
	extern void (CODEGEN_FUNCPTR *GetTexParameteriv)(GLenum target, GLenum pname, GLint * params);
	extern void (CODEGEN_FUNCPTR *Hint)(GLenum target, GLenum mode);
	extern GLboolean (CODEGEN_FUNCPTR *IsEnabled)(GLenum cap);
	extern void (CODEGEN_FUNCPTR *LineWidth)(GLfloat width);
	extern void (CODEGEN_FUNCPTR *LogicOp)(GLenum opcode);
	extern void (CODEGEN_FUNCPTR *PixelStoref)(GLenum pname, GLfloat param);
	extern void (CODEGEN_FUNCPTR *PixelStorei)(GLenum pname, GLint param);
	extern void (CODEGEN_FUNCPTR *PointSize)(GLfloat size);
	extern void (CODEGEN_FUNCPTR *PolygonMode)(GLenum face, GLenum mode);
	extern void (CODEGEN_FUNCPTR *ReadBuffer)(GLenum mode);
	extern void (CODEGEN_FUNCPTR *ReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid * pixels);
	extern void (CODEGEN_FUNCPTR *Scissor)(GLint x, GLint y, GLsizei width, GLsizei height);
	extern void (CODEGEN_FUNCPTR *StencilFunc)(GLenum func, GLint ref, GLuint mask);
	extern void (CODEGEN_FUNCPTR *StencilMask)(GLuint mask);
	extern void (CODEGEN_FUNCPTR *StencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
	extern void (CODEGEN_FUNCPTR *TexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
	extern void (CODEGEN_FUNCPTR *TexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
	extern void (CODEGEN_FUNCPTR *TexParameterf)(GLenum target, GLenum pname, GLfloat param);
	extern void (CODEGEN_FUNCPTR *TexParameterfv)(GLenum target, GLenum pname, const GLfloat * params);
	extern void (CODEGEN_FUNCPTR *TexParameteri)(GLenum target, GLenum pname, GLint param);
	extern void (CODEGEN_FUNCPTR *TexParameteriv)(GLenum target, GLenum pname, const GLint * params);
	extern void (CODEGEN_FUNCPTR *Viewport)(GLint x, GLint y, GLsizei width, GLsizei height);
	
	extern void (CODEGEN_FUNCPTR *BindTexture)(GLenum target, GLuint texture);
	extern void (CODEGEN_FUNCPTR *CopyTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
	extern void (CODEGEN_FUNCPTR *CopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
	extern void (CODEGEN_FUNCPTR *CopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
	extern void (CODEGEN_FUNCPTR *CopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	extern void (CODEGEN_FUNCPTR *DeleteTextures)(GLsizei n, const GLuint * textures);
	extern void (CODEGEN_FUNCPTR *DrawArrays)(GLenum mode, GLint first, GLsizei count);
	extern void (CODEGEN_FUNCPTR *DrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);
	extern void (CODEGEN_FUNCPTR *GenTextures)(GLsizei n, GLuint * textures);
	extern GLboolean (CODEGEN_FUNCPTR *IsTexture)(GLuint texture);
	extern void (CODEGEN_FUNCPTR *PolygonOffset)(GLfloat factor, GLfloat units);
	extern void (CODEGEN_FUNCPTR *TexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid * pixels);
	extern void (CODEGEN_FUNCPTR *TexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels);
	
	extern void (CODEGEN_FUNCPTR *BlendColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	extern void (CODEGEN_FUNCPTR *BlendEquation)(GLenum mode);
	extern void (CODEGEN_FUNCPTR *CopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	extern void (CODEGEN_FUNCPTR *DrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices);
	extern void (CODEGEN_FUNCPTR *TexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
	extern void (CODEGEN_FUNCPTR *TexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * pixels);
	
	extern void (CODEGEN_FUNCPTR *ActiveTexture)(GLenum texture);
	extern void (CODEGEN_FUNCPTR *CompressedTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid * data);
	extern void (CODEGEN_FUNCPTR *CompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid * data);
	extern void (CODEGEN_FUNCPTR *CompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid * data);
	extern void (CODEGEN_FUNCPTR *CompressedTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid * data);
	extern void (CODEGEN_FUNCPTR *CompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid * data);
	extern void (CODEGEN_FUNCPTR *CompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data);
	extern void (CODEGEN_FUNCPTR *GetCompressedTexImage)(GLenum target, GLint level, GLvoid * img);
	extern void (CODEGEN_FUNCPTR *SampleCoverage)(GLfloat value, GLboolean invert);
	
	extern void (CODEGEN_FUNCPTR *BlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
	extern void (CODEGEN_FUNCPTR *MultiDrawArrays)(GLenum mode, const GLint * first, const GLsizei * count, GLsizei drawcount);
	extern void (CODEGEN_FUNCPTR *MultiDrawElements)(GLenum mode, const GLsizei * count, GLenum type, const GLvoid *const* indices, GLsizei drawcount);
	extern void (CODEGEN_FUNCPTR *PointParameterf)(GLenum pname, GLfloat param);
	extern void (CODEGEN_FUNCPTR *PointParameterfv)(GLenum pname, const GLfloat * params);
	extern void (CODEGEN_FUNCPTR *PointParameteri)(GLenum pname, GLint param);
	extern void (CODEGEN_FUNCPTR *PointParameteriv)(GLenum pname, const GLint * params);
	
	extern void (CODEGEN_FUNCPTR *BeginQuery)(GLenum target, GLuint id);
	extern void (CODEGEN_FUNCPTR *BindBuffer)(GLenum target, GLuint buffer);
	extern void (CODEGEN_FUNCPTR *BufferData)(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage);
	extern void (CODEGEN_FUNCPTR *BufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data);
	extern void (CODEGEN_FUNCPTR *DeleteBuffers)(GLsizei n, const GLuint * buffers);
	extern void (CODEGEN_FUNCPTR *DeleteQueries)(GLsizei n, const GLuint * ids);
	extern void (CODEGEN_FUNCPTR *EndQuery)(GLenum target);
	extern void (CODEGEN_FUNCPTR *GenBuffers)(GLsizei n, GLuint * buffers);
	extern void (CODEGEN_FUNCPTR *GenQueries)(GLsizei n, GLuint * ids);
	extern void (CODEGEN_FUNCPTR *GetBufferParameteriv)(GLenum target, GLenum pname, GLint * params);
	extern void (CODEGEN_FUNCPTR *GetBufferPointerv)(GLenum target, GLenum pname, GLvoid ** params);
	extern void (CODEGEN_FUNCPTR *GetBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid * data);
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
	extern GLuint (CODEGEN_FUNCPTR *CreateProgram)();
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
	extern void (CODEGEN_FUNCPTR *GetVertexAttribPointerv)(GLuint index, GLenum pname, GLvoid ** pointer);
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
	extern void (CODEGEN_FUNCPTR *VertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer);
	
	extern void (CODEGEN_FUNCPTR *UniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
	extern void (CODEGEN_FUNCPTR *UniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
	extern void (CODEGEN_FUNCPTR *UniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
	extern void (CODEGEN_FUNCPTR *UniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
	extern void (CODEGEN_FUNCPTR *UniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
	extern void (CODEGEN_FUNCPTR *UniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
	
	extern void (CODEGEN_FUNCPTR *BeginConditionalRender)(GLuint id, GLenum mode);
	extern void (CODEGEN_FUNCPTR *BeginTransformFeedback)(GLenum primitiveMode);
	extern void (CODEGEN_FUNCPTR *BindBufferBase)(GLenum target, GLuint index, GLuint buffer);
	extern void (CODEGEN_FUNCPTR *BindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
	extern void (CODEGEN_FUNCPTR *BindFragDataLocation)(GLuint program, GLuint color, const GLchar * name);
	extern void (CODEGEN_FUNCPTR *BindFramebuffer)(GLenum target, GLuint framebuffer);
	extern void (CODEGEN_FUNCPTR *BindRenderbuffer)(GLenum target, GLuint renderbuffer);
	extern void (CODEGEN_FUNCPTR *BindVertexArray)(GLuint ren_array);
	extern void (CODEGEN_FUNCPTR *BlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
	extern GLenum (CODEGEN_FUNCPTR *CheckFramebufferStatus)(GLenum target);
	extern void (CODEGEN_FUNCPTR *ClampColor)(GLenum target, GLenum clamp);
	extern void (CODEGEN_FUNCPTR *ClearBufferfi)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
	extern void (CODEGEN_FUNCPTR *ClearBufferfv)(GLenum buffer, GLint drawbuffer, const GLfloat * value);
	extern void (CODEGEN_FUNCPTR *ClearBufferiv)(GLenum buffer, GLint drawbuffer, const GLint * value);
	extern void (CODEGEN_FUNCPTR *ClearBufferuiv)(GLenum buffer, GLint drawbuffer, const GLuint * value);
	extern void (CODEGEN_FUNCPTR *ColorMaski)(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
	extern void (CODEGEN_FUNCPTR *DeleteFramebuffers)(GLsizei n, const GLuint * framebuffers);
	extern void (CODEGEN_FUNCPTR *DeleteRenderbuffers)(GLsizei n, const GLuint * renderbuffers);
	extern void (CODEGEN_FUNCPTR *DeleteVertexArrays)(GLsizei n, const GLuint * arrays);
	extern void (CODEGEN_FUNCPTR *Disablei)(GLenum target, GLuint index);
	extern void (CODEGEN_FUNCPTR *Enablei)(GLenum target, GLuint index);
	extern void (CODEGEN_FUNCPTR *EndConditionalRender)();
	extern void (CODEGEN_FUNCPTR *EndTransformFeedback)();
	extern void (CODEGEN_FUNCPTR *FlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length);
	extern void (CODEGEN_FUNCPTR *FramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
	extern void (CODEGEN_FUNCPTR *FramebufferTexture1D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	extern void (CODEGEN_FUNCPTR *FramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	extern void (CODEGEN_FUNCPTR *FramebufferTexture3D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
	extern void (CODEGEN_FUNCPTR *FramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
	extern void (CODEGEN_FUNCPTR *GenFramebuffers)(GLsizei n, GLuint * framebuffers);
	extern void (CODEGEN_FUNCPTR *GenRenderbuffers)(GLsizei n, GLuint * renderbuffers);
	extern void (CODEGEN_FUNCPTR *GenVertexArrays)(GLsizei n, GLuint * arrays);
	extern void (CODEGEN_FUNCPTR *GenerateMipmap)(GLenum target);
	extern void (CODEGEN_FUNCPTR *GetBooleani_v)(GLenum target, GLuint index, GLboolean * data);
	extern GLint (CODEGEN_FUNCPTR *GetFragDataLocation)(GLuint program, const GLchar * name);
	extern void (CODEGEN_FUNCPTR *GetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint * params);
	extern void (CODEGEN_FUNCPTR *GetIntegeri_v)(GLenum target, GLuint index, GLint * data);
	extern void (CODEGEN_FUNCPTR *GetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint * params);
	extern const GLubyte * (CODEGEN_FUNCPTR *GetStringi)(GLenum name, GLuint index);
	extern void (CODEGEN_FUNCPTR *GetTexParameterIiv)(GLenum target, GLenum pname, GLint * params);
	extern void (CODEGEN_FUNCPTR *GetTexParameterIuiv)(GLenum target, GLenum pname, GLuint * params);
	extern void (CODEGEN_FUNCPTR *GetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLsizei * size, GLenum * type, GLchar * name);
	extern void (CODEGEN_FUNCPTR *GetUniformuiv)(GLuint program, GLint location, GLuint * params);
	extern void (CODEGEN_FUNCPTR *GetVertexAttribIiv)(GLuint index, GLenum pname, GLint * params);
	extern void (CODEGEN_FUNCPTR *GetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint * params);
	extern GLboolean (CODEGEN_FUNCPTR *IsEnabledi)(GLenum target, GLuint index);
	extern GLboolean (CODEGEN_FUNCPTR *IsFramebuffer)(GLuint framebuffer);
	extern GLboolean (CODEGEN_FUNCPTR *IsRenderbuffer)(GLuint renderbuffer);
	extern GLboolean (CODEGEN_FUNCPTR *IsVertexArray)(GLuint ren_array);
	extern void * (CODEGEN_FUNCPTR *MapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
	extern void (CODEGEN_FUNCPTR *RenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
	extern void (CODEGEN_FUNCPTR *RenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
	extern void (CODEGEN_FUNCPTR *TexParameterIiv)(GLenum target, GLenum pname, const GLint * params);
	extern void (CODEGEN_FUNCPTR *TexParameterIuiv)(GLenum target, GLenum pname, const GLuint * params);
	extern void (CODEGEN_FUNCPTR *TransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar *const* varyings, GLenum bufferMode);
	extern void (CODEGEN_FUNCPTR *Uniform1ui)(GLint location, GLuint v0);
	extern void (CODEGEN_FUNCPTR *Uniform1uiv)(GLint location, GLsizei count, const GLuint * value);
	extern void (CODEGEN_FUNCPTR *Uniform2ui)(GLint location, GLuint v0, GLuint v1);
	extern void (CODEGEN_FUNCPTR *Uniform2uiv)(GLint location, GLsizei count, const GLuint * value);
	extern void (CODEGEN_FUNCPTR *Uniform3ui)(GLint location, GLuint v0, GLuint v1, GLuint v2);
	extern void (CODEGEN_FUNCPTR *Uniform3uiv)(GLint location, GLsizei count, const GLuint * value);
	extern void (CODEGEN_FUNCPTR *Uniform4ui)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
	extern void (CODEGEN_FUNCPTR *Uniform4uiv)(GLint location, GLsizei count, const GLuint * value);
	extern void (CODEGEN_FUNCPTR *VertexAttribI1i)(GLuint index, GLint x);
	extern void (CODEGEN_FUNCPTR *VertexAttribI1iv)(GLuint index, const GLint * v);
	extern void (CODEGEN_FUNCPTR *VertexAttribI1ui)(GLuint index, GLuint x);
	extern void (CODEGEN_FUNCPTR *VertexAttribI1uiv)(GLuint index, const GLuint * v);
	extern void (CODEGEN_FUNCPTR *VertexAttribI2i)(GLuint index, GLint x, GLint y);
	extern void (CODEGEN_FUNCPTR *VertexAttribI2iv)(GLuint index, const GLint * v);
	extern void (CODEGEN_FUNCPTR *VertexAttribI2ui)(GLuint index, GLuint x, GLuint y);
	extern void (CODEGEN_FUNCPTR *VertexAttribI2uiv)(GLuint index, const GLuint * v);
	extern void (CODEGEN_FUNCPTR *VertexAttribI3i)(GLuint index, GLint x, GLint y, GLint z);
	extern void (CODEGEN_FUNCPTR *VertexAttribI3iv)(GLuint index, const GLint * v);
	extern void (CODEGEN_FUNCPTR *VertexAttribI3ui)(GLuint index, GLuint x, GLuint y, GLuint z);
	extern void (CODEGEN_FUNCPTR *VertexAttribI3uiv)(GLuint index, const GLuint * v);
	extern void (CODEGEN_FUNCPTR *VertexAttribI4bv)(GLuint index, const GLbyte * v);
	extern void (CODEGEN_FUNCPTR *VertexAttribI4i)(GLuint index, GLint x, GLint y, GLint z, GLint w);
	extern void (CODEGEN_FUNCPTR *VertexAttribI4iv)(GLuint index, const GLint * v);
	extern void (CODEGEN_FUNCPTR *VertexAttribI4sv)(GLuint index, const GLshort * v);
	extern void (CODEGEN_FUNCPTR *VertexAttribI4ubv)(GLuint index, const GLubyte * v);
	extern void (CODEGEN_FUNCPTR *VertexAttribI4ui)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
	extern void (CODEGEN_FUNCPTR *VertexAttribI4uiv)(GLuint index, const GLuint * v);
	extern void (CODEGEN_FUNCPTR *VertexAttribI4usv)(GLuint index, const GLushort * v);
	extern void (CODEGEN_FUNCPTR *VertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid * pointer);
	
	extern void (CODEGEN_FUNCPTR *CopyBufferSubData)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
	extern void (CODEGEN_FUNCPTR *DrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
	extern void (CODEGEN_FUNCPTR *DrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei instancecount);
	extern void (CODEGEN_FUNCPTR *GetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformBlockName);
	extern void (CODEGEN_FUNCPTR *GetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint * params);
	extern void (CODEGEN_FUNCPTR *GetActiveUniformName)(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformName);
	extern void (CODEGEN_FUNCPTR *GetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint * uniformIndices, GLenum pname, GLint * params);
	extern GLuint (CODEGEN_FUNCPTR *GetUniformBlockIndex)(GLuint program, const GLchar * uniformBlockName);
	extern void (CODEGEN_FUNCPTR *GetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar *const* uniformNames, GLuint * uniformIndices);
	extern void (CODEGEN_FUNCPTR *PrimitiveRestartIndex)(GLuint index);
	extern void (CODEGEN_FUNCPTR *TexBuffer)(GLenum target, GLenum internalformat, GLuint buffer);
	extern void (CODEGEN_FUNCPTR *UniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
	
	extern GLenum (CODEGEN_FUNCPTR *ClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
	extern void (CODEGEN_FUNCPTR *DeleteSync)(GLsync sync);
	extern void (CODEGEN_FUNCPTR *DrawElementsBaseVertex)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLint basevertex);
	extern void (CODEGEN_FUNCPTR *DrawElementsInstancedBaseVertex)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei instancecount, GLint basevertex);
	extern void (CODEGEN_FUNCPTR *DrawRangeElementsBaseVertex)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices, GLint basevertex);
	extern GLsync (CODEGEN_FUNCPTR *FenceSync)(GLenum condition, GLbitfield flags);
	extern void (CODEGEN_FUNCPTR *FramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level);
	extern void (CODEGEN_FUNCPTR *GetBufferParameteri64v)(GLenum target, GLenum pname, GLint64 * params);
	extern void (CODEGEN_FUNCPTR *GetInteger64i_v)(GLenum target, GLuint index, GLint64 * data);
	extern void (CODEGEN_FUNCPTR *GetInteger64v)(GLenum pname, GLint64 * params);
	extern void (CODEGEN_FUNCPTR *GetMultisamplefv)(GLenum pname, GLuint index, GLfloat * val);
	extern void (CODEGEN_FUNCPTR *GetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * values);
	extern GLboolean (CODEGEN_FUNCPTR *IsSync)(GLsync sync);
	extern void (CODEGEN_FUNCPTR *MultiDrawElementsBaseVertex)(GLenum mode, const GLsizei * count, GLenum type, const GLvoid *const* indices, GLsizei drawcount, const GLint * basevertex);
	extern void (CODEGEN_FUNCPTR *ProvokingVertex)(GLenum mode);
	extern void (CODEGEN_FUNCPTR *SampleMaski)(GLuint index, GLbitfield mask);
	extern void (CODEGEN_FUNCPTR *TexImage2DMultisample)(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
	extern void (CODEGEN_FUNCPTR *TexImage3DMultisample)(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
	extern void (CODEGEN_FUNCPTR *WaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
	
	namespace sys
	{
		
		exts::LoadTest LoadFunctions();
		
		int GetMinorVersion();
		int GetMajorVersion();
		bool IsVersionGEQ(int majorVersion, int minorVersion);
		
	} //namespace sys
} //namespace gl
#endif //POINTER_CPP_GENERATED_HEADEROPENGL_HPP
