#ifndef GL3X_PIONEER_GENERATED_HEADEROPENGL_HPP
#define GL3X_PIONEER_GENERATED_HEADEROPENGL_HPP

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

namespace gl3x
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
			
			extern LoadTest var_EXT_texture_compression_s3tc;
			extern LoadTest var_EXT_texture_sRGB;
			extern LoadTest var_EXT_texture_filter_anisotropic;
			extern LoadTest var_ARB_compressed_texture_pixel_storage;
			extern LoadTest var_ARB_conservative_depth;
			extern LoadTest var_ARB_ES2_compatibility;
			extern LoadTest var_ARB_get_program_binary;
			extern LoadTest var_ARB_explicit_uniform_location;
			extern LoadTest var_ARB_internalformat_query;
			extern LoadTest var_ARB_internalformat_query2;
			extern LoadTest var_ARB_map_buffer_alignment;
			extern LoadTest var_ARB_program_interface_query;
			extern LoadTest var_ARB_separate_shader_objects;
			extern LoadTest var_ARB_shading_language_420pack;
			extern LoadTest var_ARB_shading_language_packing;
			extern LoadTest var_ARB_texture_buffer_range;
			extern LoadTest var_ARB_texture_storage;
			extern LoadTest var_ARB_texture_view;
			extern LoadTest var_ARB_vertex_attrib_binding;
			extern LoadTest var_ARB_viewport_array;
			extern LoadTest var_ARB_arrays_of_arrays;
			extern LoadTest var_ARB_clear_buffer_object;
			extern LoadTest var_ARB_copy_image;
			extern LoadTest var_ARB_ES3_compatibility;
			extern LoadTest var_ARB_fragment_layer_viewport;
			extern LoadTest var_ARB_framebuffer_no_attachments;
			extern LoadTest var_ARB_invalidate_subdata;
			extern LoadTest var_ARB_robust_buffer_access_behavior;
			extern LoadTest var_ARB_stencil_texturing;
			extern LoadTest var_ARB_texture_query_levels;
			extern LoadTest var_ARB_texture_storage_multisample;
			extern LoadTest var_KHR_debug;
			extern LoadTest var_ARB_buffer_storage;
			extern LoadTest var_ARB_clear_texture;
			extern LoadTest var_ARB_enhanced_layouts;
			extern LoadTest var_ARB_multi_bind;
			extern LoadTest var_ARB_query_buffer_object;
			extern LoadTest var_ARB_texture_mirror_clamp_to_edge;
			extern LoadTest var_ARB_texture_stencil8;
			extern LoadTest var_ARB_vertex_type_10f_11f_11f_rev;
			extern LoadTest var_ARB_seamless_cubemap_per_texture;
			extern LoadTest var_ARB_clip_control;
			extern LoadTest var_ARB_conditional_render_inverted;
			extern LoadTest var_ARB_cull_distance;
			extern LoadTest var_ARB_derivative_control;
			extern LoadTest var_ARB_direct_state_access;
			extern LoadTest var_ARB_get_texture_sub_image;
			extern LoadTest var_ARB_shader_texture_image_samples;
			extern LoadTest var_ARB_texture_barrier;
			extern LoadTest var_KHR_context_flush_control;
			extern LoadTest var_KHR_robust_buffer_access_behavior;
			extern LoadTest var_KHR_robustness;
			
		} //namespace exts
		enum
		{
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
			
			GL_PACK_COMPRESSED_BLOCK_DEPTH   = 0x912D,
			GL_PACK_COMPRESSED_BLOCK_HEIGHT  = 0x912C,
			GL_PACK_COMPRESSED_BLOCK_SIZE    = 0x912E,
			GL_PACK_COMPRESSED_BLOCK_WIDTH   = 0x912B,
			GL_UNPACK_COMPRESSED_BLOCK_DEPTH = 0x9129,
			GL_UNPACK_COMPRESSED_BLOCK_HEIGHT = 0x9128,
			GL_UNPACK_COMPRESSED_BLOCK_SIZE  = 0x912A,
			GL_UNPACK_COMPRESSED_BLOCK_WIDTH = 0x9127,
			
			GL_FIXED                         = 0x140C,
			GL_HIGH_FLOAT                    = 0x8DF2,
			GL_HIGH_INT                      = 0x8DF5,
			GL_IMPLEMENTATION_COLOR_READ_FORMAT = 0x8B9B,
			GL_IMPLEMENTATION_COLOR_READ_TYPE = 0x8B9A,
			GL_LOW_FLOAT                     = 0x8DF0,
			GL_LOW_INT                       = 0x8DF3,
			GL_MAX_FRAGMENT_UNIFORM_VECTORS  = 0x8DFD,
			GL_MAX_VARYING_VECTORS           = 0x8DFC,
			GL_MAX_VERTEX_UNIFORM_VECTORS    = 0x8DFB,
			GL_MEDIUM_FLOAT                  = 0x8DF1,
			GL_MEDIUM_INT                    = 0x8DF4,
			GL_NUM_SHADER_BINARY_FORMATS     = 0x8DF9,
			GL_RGB565                        = 0x8D62,
			GL_SHADER_BINARY_FORMATS         = 0x8DF8,
			GL_SHADER_COMPILER               = 0x8DFA,
			
			GL_NUM_PROGRAM_BINARY_FORMATS    = 0x87FE,
			GL_PROGRAM_BINARY_FORMATS        = 0x87FF,
			GL_PROGRAM_BINARY_LENGTH         = 0x8741,
			GL_PROGRAM_BINARY_RETRIEVABLE_HINT = 0x8257,
			
			GL_MAX_UNIFORM_LOCATIONS         = 0x826E,
			
			GL_NUM_SAMPLE_COUNTS             = 0x9380,
			
			GL_AUTO_GENERATE_MIPMAP          = 0x8295,
			GL_CAVEAT_SUPPORT                = 0x82B8,
			GL_CLEAR_BUFFER                  = 0x82B4,
			GL_COLOR_COMPONENTS              = 0x8283,
			GL_COLOR_ENCODING                = 0x8296,
			GL_COLOR_RENDERABLE              = 0x8286,
			GL_COMPUTE_TEXTURE               = 0x82A0,
			GL_DEPTH_COMPONENTS              = 0x8284,
			GL_DEPTH_RENDERABLE              = 0x8287,
			GL_FILTER                        = 0x829A,
			GL_FRAGMENT_TEXTURE              = 0x829F,
			GL_FRAMEBUFFER_BLEND             = 0x828B,
			GL_FRAMEBUFFER_RENDERABLE        = 0x8289,
			GL_FRAMEBUFFER_RENDERABLE_LAYERED = 0x828A,
			GL_FULL_SUPPORT                  = 0x82B7,
			GL_GEOMETRY_TEXTURE              = 0x829E,
			GL_GET_TEXTURE_IMAGE_FORMAT      = 0x8291,
			GL_GET_TEXTURE_IMAGE_TYPE        = 0x8292,
			GL_IMAGE_CLASS_10_10_10_2        = 0x82C3,
			GL_IMAGE_CLASS_11_11_10          = 0x82C2,
			GL_IMAGE_CLASS_1_X_16            = 0x82BE,
			GL_IMAGE_CLASS_1_X_32            = 0x82BB,
			GL_IMAGE_CLASS_1_X_8             = 0x82C1,
			GL_IMAGE_CLASS_2_X_16            = 0x82BD,
			GL_IMAGE_CLASS_2_X_32            = 0x82BA,
			GL_IMAGE_CLASS_2_X_8             = 0x82C0,
			GL_IMAGE_CLASS_4_X_16            = 0x82BC,
			GL_IMAGE_CLASS_4_X_32            = 0x82B9,
			GL_IMAGE_CLASS_4_X_8             = 0x82BF,
			GL_IMAGE_COMPATIBILITY_CLASS     = 0x82A8,
			GL_IMAGE_FORMAT_COMPATIBILITY_TYPE = 0x90C7,
			GL_IMAGE_PIXEL_FORMAT            = 0x82A9,
			GL_IMAGE_PIXEL_TYPE              = 0x82AA,
			GL_IMAGE_TEXEL_SIZE              = 0x82A7,
			GL_INTERNALFORMAT_ALPHA_SIZE     = 0x8274,
			GL_INTERNALFORMAT_ALPHA_TYPE     = 0x827B,
			GL_INTERNALFORMAT_BLUE_SIZE      = 0x8273,
			GL_INTERNALFORMAT_BLUE_TYPE      = 0x827A,
			GL_INTERNALFORMAT_DEPTH_SIZE     = 0x8275,
			GL_INTERNALFORMAT_DEPTH_TYPE     = 0x827C,
			GL_INTERNALFORMAT_GREEN_SIZE     = 0x8272,
			GL_INTERNALFORMAT_GREEN_TYPE     = 0x8279,
			GL_INTERNALFORMAT_PREFERRED      = 0x8270,
			GL_INTERNALFORMAT_RED_SIZE       = 0x8271,
			GL_INTERNALFORMAT_RED_TYPE       = 0x8278,
			GL_INTERNALFORMAT_SHARED_SIZE    = 0x8277,
			GL_INTERNALFORMAT_STENCIL_SIZE   = 0x8276,
			GL_INTERNALFORMAT_STENCIL_TYPE   = 0x827D,
			GL_INTERNALFORMAT_SUPPORTED      = 0x826F,
			GL_MANUAL_GENERATE_MIPMAP        = 0x8294,
			GL_MAX_COMBINED_DIMENSIONS       = 0x8282,
			GL_MAX_DEPTH                     = 0x8280,
			GL_MAX_HEIGHT                    = 0x827F,
			GL_MAX_LAYERS                    = 0x8281,
			GL_MAX_WIDTH                     = 0x827E,
			GL_MIPMAP                        = 0x8293,
			//NUM_SAMPLE_COUNTS taken from ext: ARB_internalformat_query
			GL_READ_PIXELS                   = 0x828C,
			GL_READ_PIXELS_FORMAT            = 0x828D,
			GL_READ_PIXELS_TYPE              = 0x828E,
			GL_RENDERBUFFER                  = 0x8D41,
			GL_SAMPLES                       = 0x80A9,
			GL_SHADER_IMAGE_ATOMIC           = 0x82A6,
			GL_SHADER_IMAGE_LOAD             = 0x82A4,
			GL_SHADER_IMAGE_STORE            = 0x82A5,
			GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST = 0x82AC,
			GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE = 0x82AE,
			GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST = 0x82AD,
			GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE = 0x82AF,
			GL_SRGB_DECODE_ARB               = 0x8299,
			GL_SRGB_READ                     = 0x8297,
			GL_SRGB_WRITE                    = 0x8298,
			GL_STENCIL_COMPONENTS            = 0x8285,
			GL_STENCIL_RENDERABLE            = 0x8288,
			GL_TESS_CONTROL_TEXTURE          = 0x829C,
			GL_TESS_EVALUATION_TEXTURE       = 0x829D,
			GL_TEXTURE_1D                    = 0x0DE0,
			GL_TEXTURE_1D_ARRAY              = 0x8C18,
			GL_TEXTURE_2D                    = 0x0DE1,
			GL_TEXTURE_2D_ARRAY              = 0x8C1A,
			GL_TEXTURE_2D_MULTISAMPLE        = 0x9100,
			GL_TEXTURE_2D_MULTISAMPLE_ARRAY  = 0x9102,
			GL_TEXTURE_3D                    = 0x806F,
			GL_TEXTURE_BUFFER                = 0x8C2A,
			GL_TEXTURE_COMPRESSED            = 0x86A1,
			GL_TEXTURE_COMPRESSED_BLOCK_HEIGHT = 0x82B2,
			GL_TEXTURE_COMPRESSED_BLOCK_SIZE = 0x82B3,
			GL_TEXTURE_COMPRESSED_BLOCK_WIDTH = 0x82B1,
			GL_TEXTURE_CUBE_MAP              = 0x8513,
			GL_TEXTURE_CUBE_MAP_ARRAY        = 0x9009,
			GL_TEXTURE_GATHER                = 0x82A2,
			GL_TEXTURE_GATHER_SHADOW         = 0x82A3,
			GL_TEXTURE_IMAGE_FORMAT          = 0x828F,
			GL_TEXTURE_IMAGE_TYPE            = 0x8290,
			GL_TEXTURE_RECTANGLE             = 0x84F5,
			GL_TEXTURE_SHADOW                = 0x82A1,
			GL_TEXTURE_VIEW                  = 0x82B5,
			GL_VERTEX_TEXTURE                = 0x829B,
			GL_VIEW_CLASS_128_BITS           = 0x82C4,
			GL_VIEW_CLASS_16_BITS            = 0x82CA,
			GL_VIEW_CLASS_24_BITS            = 0x82C9,
			GL_VIEW_CLASS_32_BITS            = 0x82C8,
			GL_VIEW_CLASS_48_BITS            = 0x82C7,
			GL_VIEW_CLASS_64_BITS            = 0x82C6,
			GL_VIEW_CLASS_8_BITS             = 0x82CB,
			GL_VIEW_CLASS_96_BITS            = 0x82C5,
			GL_VIEW_CLASS_BPTC_FLOAT         = 0x82D3,
			GL_VIEW_CLASS_BPTC_UNORM         = 0x82D2,
			GL_VIEW_CLASS_RGTC1_RED          = 0x82D0,
			GL_VIEW_CLASS_RGTC2_RG           = 0x82D1,
			GL_VIEW_CLASS_S3TC_DXT1_RGB      = 0x82CC,
			GL_VIEW_CLASS_S3TC_DXT1_RGBA     = 0x82CD,
			GL_VIEW_CLASS_S3TC_DXT3_RGBA     = 0x82CE,
			GL_VIEW_CLASS_S3TC_DXT5_RGBA     = 0x82CF,
			GL_VIEW_COMPATIBILITY_CLASS      = 0x82B6,
			
			GL_MIN_MAP_BUFFER_ALIGNMENT      = 0x90BC,
			
			GL_ACTIVE_RESOURCES              = 0x92F5,
			GL_ACTIVE_VARIABLES              = 0x9305,
			GL_ARRAY_SIZE                    = 0x92FB,
			GL_ARRAY_STRIDE                  = 0x92FE,
			GL_ATOMIC_COUNTER_BUFFER         = 0x92C0,
			GL_ATOMIC_COUNTER_BUFFER_INDEX   = 0x9301,
			GL_BLOCK_INDEX                   = 0x92FD,
			GL_BUFFER_BINDING                = 0x9302,
			GL_BUFFER_DATA_SIZE              = 0x9303,
			GL_BUFFER_VARIABLE               = 0x92E5,
			GL_COMPATIBLE_SUBROUTINES        = 0x8E4B,
			GL_COMPUTE_SUBROUTINE            = 0x92ED,
			GL_COMPUTE_SUBROUTINE_UNIFORM    = 0x92F3,
			GL_FRAGMENT_SUBROUTINE           = 0x92EC,
			GL_FRAGMENT_SUBROUTINE_UNIFORM   = 0x92F2,
			GL_GEOMETRY_SUBROUTINE           = 0x92EB,
			GL_GEOMETRY_SUBROUTINE_UNIFORM   = 0x92F1,
			GL_IS_PER_PATCH                  = 0x92E7,
			GL_IS_ROW_MAJOR                  = 0x9300,
			GL_LOCATION                      = 0x930E,
			GL_LOCATION_INDEX                = 0x930F,
			GL_MATRIX_STRIDE                 = 0x92FF,
			GL_MAX_NAME_LENGTH               = 0x92F6,
			GL_MAX_NUM_ACTIVE_VARIABLES      = 0x92F7,
			GL_MAX_NUM_COMPATIBLE_SUBROUTINES = 0x92F8,
			GL_NAME_LENGTH                   = 0x92F9,
			GL_NUM_ACTIVE_VARIABLES          = 0x9304,
			GL_NUM_COMPATIBLE_SUBROUTINES    = 0x8E4A,
			GL_OFFSET                        = 0x92FC,
			GL_PROGRAM_INPUT                 = 0x92E3,
			GL_PROGRAM_OUTPUT                = 0x92E4,
			GL_REFERENCED_BY_COMPUTE_SHADER  = 0x930B,
			GL_REFERENCED_BY_FRAGMENT_SHADER = 0x930A,
			GL_REFERENCED_BY_GEOMETRY_SHADER = 0x9309,
			GL_REFERENCED_BY_TESS_CONTROL_SHADER = 0x9307,
			GL_REFERENCED_BY_TESS_EVALUATION_SHADER = 0x9308,
			GL_REFERENCED_BY_VERTEX_SHADER   = 0x9306,
			GL_SHADER_STORAGE_BLOCK          = 0x92E6,
			GL_TESS_CONTROL_SUBROUTINE       = 0x92E9,
			GL_TESS_CONTROL_SUBROUTINE_UNIFORM = 0x92EF,
			GL_TESS_EVALUATION_SUBROUTINE    = 0x92EA,
			GL_TESS_EVALUATION_SUBROUTINE_UNIFORM = 0x92F0,
			GL_TOP_LEVEL_ARRAY_SIZE          = 0x930C,
			GL_TOP_LEVEL_ARRAY_STRIDE        = 0x930D,
			GL_TRANSFORM_FEEDBACK_VARYING    = 0x92F4,
			GL_TYPE                          = 0x92FA,
			GL_UNIFORM                       = 0x92E1,
			GL_UNIFORM_BLOCK                 = 0x92E2,
			GL_VERTEX_SUBROUTINE             = 0x92E8,
			GL_VERTEX_SUBROUTINE_UNIFORM     = 0x92EE,
			
			GL_ACTIVE_PROGRAM                = 0x8259,
			GL_ALL_SHADER_BITS               = 0xFFFFFFFF,
			GL_FRAGMENT_SHADER_BIT           = 0x00000002,
			GL_GEOMETRY_SHADER_BIT           = 0x00000004,
			GL_PROGRAM_PIPELINE_BINDING      = 0x825A,
			GL_PROGRAM_SEPARABLE             = 0x8258,
			GL_TESS_CONTROL_SHADER_BIT       = 0x00000008,
			GL_TESS_EVALUATION_SHADER_BIT    = 0x00000010,
			GL_VERTEX_SHADER_BIT             = 0x00000001,
			
			GL_TEXTURE_BUFFER_OFFSET         = 0x919D,
			GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT = 0x919F,
			GL_TEXTURE_BUFFER_SIZE           = 0x919E,
			
			GL_TEXTURE_IMMUTABLE_FORMAT      = 0x912F,
			
			GL_TEXTURE_IMMUTABLE_LEVELS      = 0x82DF,
			GL_TEXTURE_VIEW_MIN_LAYER        = 0x82DD,
			GL_TEXTURE_VIEW_MIN_LEVEL        = 0x82DB,
			GL_TEXTURE_VIEW_NUM_LAYERS       = 0x82DE,
			GL_TEXTURE_VIEW_NUM_LEVELS       = 0x82DC,
			
			GL_MAX_VERTEX_ATTRIB_BINDINGS    = 0x82DA,
			GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET = 0x82D9,
			GL_VERTEX_ATTRIB_BINDING         = 0x82D4,
			GL_VERTEX_ATTRIB_RELATIVE_OFFSET = 0x82D5,
			GL_VERTEX_BINDING_DIVISOR        = 0x82D6,
			GL_VERTEX_BINDING_OFFSET         = 0x82D7,
			GL_VERTEX_BINDING_STRIDE         = 0x82D8,
			
			GL_DEPTH_RANGE                   = 0x0B70,
			GL_FIRST_VERTEX_CONVENTION       = 0x8E4D,
			GL_LAST_VERTEX_CONVENTION        = 0x8E4E,
			GL_LAYER_PROVOKING_VERTEX        = 0x825E,
			GL_MAX_VIEWPORTS                 = 0x825B,
			GL_PROVOKING_VERTEX              = 0x8E4F,
			GL_SCISSOR_BOX                   = 0x0C10,
			GL_SCISSOR_TEST                  = 0x0C11,
			GL_UNDEFINED_VERTEX              = 0x8260,
			GL_VIEWPORT                      = 0x0BA2,
			GL_VIEWPORT_BOUNDS_RANGE         = 0x825D,
			GL_VIEWPORT_INDEX_PROVOKING_VERTEX = 0x825F,
			GL_VIEWPORT_SUBPIXEL_BITS        = 0x825C,
			
			GL_ANY_SAMPLES_PASSED_CONSERVATIVE = 0x8D6A,
			GL_COMPRESSED_R11_EAC            = 0x9270,
			GL_COMPRESSED_RG11_EAC           = 0x9272,
			GL_COMPRESSED_RGB8_ETC2          = 0x9274,
			GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9276,
			GL_COMPRESSED_RGBA8_ETC2_EAC     = 0x9278,
			GL_COMPRESSED_SIGNED_R11_EAC     = 0x9271,
			GL_COMPRESSED_SIGNED_RG11_EAC    = 0x9273,
			GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC = 0x9279,
			GL_COMPRESSED_SRGB8_ETC2         = 0x9275,
			GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9277,
			GL_MAX_ELEMENT_INDEX             = 0x8D6B,
			GL_PRIMITIVE_RESTART_FIXED_INDEX = 0x8D69,
			
			GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS = 0x9314,
			GL_FRAMEBUFFER_DEFAULT_HEIGHT    = 0x9311,
			GL_FRAMEBUFFER_DEFAULT_LAYERS    = 0x9312,
			GL_FRAMEBUFFER_DEFAULT_SAMPLES   = 0x9313,
			GL_FRAMEBUFFER_DEFAULT_WIDTH     = 0x9310,
			GL_MAX_FRAMEBUFFER_HEIGHT        = 0x9316,
			GL_MAX_FRAMEBUFFER_LAYERS        = 0x9317,
			GL_MAX_FRAMEBUFFER_SAMPLES       = 0x9318,
			GL_MAX_FRAMEBUFFER_WIDTH         = 0x9315,
			
			GL_DEPTH_STENCIL_TEXTURE_MODE    = 0x90EA,
			
			GL_BUFFER                        = 0x82E0,
			GL_CONTEXT_FLAG_DEBUG_BIT        = 0x00000002,
			GL_DEBUG_CALLBACK_FUNCTION       = 0x8244,
			GL_DEBUG_CALLBACK_USER_PARAM     = 0x8245,
			GL_DEBUG_GROUP_STACK_DEPTH       = 0x826D,
			GL_DEBUG_LOGGED_MESSAGES         = 0x9145,
			GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH = 0x8243,
			GL_DEBUG_OUTPUT                  = 0x92E0,
			GL_DEBUG_OUTPUT_SYNCHRONOUS      = 0x8242,
			GL_DEBUG_SEVERITY_HIGH           = 0x9146,
			GL_DEBUG_SEVERITY_LOW            = 0x9148,
			GL_DEBUG_SEVERITY_MEDIUM         = 0x9147,
			GL_DEBUG_SEVERITY_NOTIFICATION   = 0x826B,
			GL_DEBUG_SOURCE_API              = 0x8246,
			GL_DEBUG_SOURCE_APPLICATION      = 0x824A,
			GL_DEBUG_SOURCE_OTHER            = 0x824B,
			GL_DEBUG_SOURCE_SHADER_COMPILER  = 0x8248,
			GL_DEBUG_SOURCE_THIRD_PARTY      = 0x8249,
			GL_DEBUG_SOURCE_WINDOW_SYSTEM    = 0x8247,
			GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR = 0x824D,
			GL_DEBUG_TYPE_ERROR              = 0x824C,
			GL_DEBUG_TYPE_MARKER             = 0x8268,
			GL_DEBUG_TYPE_OTHER              = 0x8251,
			GL_DEBUG_TYPE_PERFORMANCE        = 0x8250,
			GL_DEBUG_TYPE_POP_GROUP          = 0x826A,
			GL_DEBUG_TYPE_PORTABILITY        = 0x824F,
			GL_DEBUG_TYPE_PUSH_GROUP         = 0x8269,
			GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR = 0x824E,
			GL_DISPLAY_LIST                  = 0x82E7,
			GL_MAX_DEBUG_GROUP_STACK_DEPTH   = 0x826C,
			GL_MAX_DEBUG_LOGGED_MESSAGES     = 0x9144,
			GL_MAX_DEBUG_MESSAGE_LENGTH      = 0x9143,
			GL_MAX_LABEL_LENGTH              = 0x82E8,
			GL_PROGRAM                       = 0x82E2,
			GL_PROGRAM_PIPELINE              = 0x82E4,
			GL_QUERY                         = 0x82E3,
			GL_SAMPLER                       = 0x82E6,
			GL_SHADER                        = 0x82E1,
			GL_STACK_OVERFLOW                = 0x0503,
			GL_STACK_UNDERFLOW               = 0x0504,
			GL_VERTEX_ARRAY                  = 0x8074,
			
			GL_BUFFER_IMMUTABLE_STORAGE      = 0x821F,
			GL_BUFFER_STORAGE_FLAGS          = 0x8220,
			GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT = 0x00004000,
			GL_CLIENT_STORAGE_BIT            = 0x0200,
			GL_DYNAMIC_STORAGE_BIT           = 0x0100,
			GL_MAP_COHERENT_BIT              = 0x0080,
			GL_MAP_PERSISTENT_BIT            = 0x0040,
			GL_MAP_READ_BIT                  = 0x0001,
			GL_MAP_WRITE_BIT                 = 0x0002,
			
			GL_CLEAR_TEXTURE                 = 0x9365,
			
			GL_LOCATION_COMPONENT            = 0x934A,
			GL_TRANSFORM_FEEDBACK_BUFFER     = 0x8C8E,
			GL_TRANSFORM_FEEDBACK_BUFFER_INDEX = 0x934B,
			GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE = 0x934C,
			
			GL_QUERY_BUFFER                  = 0x9192,
			GL_QUERY_BUFFER_BARRIER_BIT      = 0x00008000,
			GL_QUERY_BUFFER_BINDING          = 0x9193,
			GL_QUERY_RESULT_NO_WAIT          = 0x9194,
			
			GL_MIRROR_CLAMP_TO_EDGE          = 0x8743,
			
			GL_STENCIL_INDEX                 = 0x1901,
			GL_STENCIL_INDEX8                = 0x8D48,
			
			GL_UNSIGNED_INT_10F_11F_11F_REV  = 0x8C3B,
			
			GL_TEXTURE_CUBE_MAP_SEAMLESS     = 0x884F,
			
			GL_CLIP_DEPTH_MODE               = 0x935D,
			GL_CLIP_ORIGIN                   = 0x935C,
			GL_LOWER_LEFT                    = 0x8CA1,
			GL_NEGATIVE_ONE_TO_ONE           = 0x935E,
			GL_UPPER_LEFT                    = 0x8CA2,
			GL_ZERO_TO_ONE                   = 0x935F,
			
			GL_QUERY_BY_REGION_NO_WAIT_INVERTED = 0x8E1A,
			GL_QUERY_BY_REGION_WAIT_INVERTED = 0x8E19,
			GL_QUERY_NO_WAIT_INVERTED        = 0x8E18,
			GL_QUERY_WAIT_INVERTED           = 0x8E17,
			
			GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES = 0x82FA,
			GL_MAX_CULL_DISTANCES            = 0x82F9,
			
			GL_QUERY_TARGET                  = 0x82EA,
			GL_TEXTURE_BINDING_1D            = 0x8068,
			GL_TEXTURE_BINDING_1D_ARRAY      = 0x8C1C,
			GL_TEXTURE_BINDING_2D            = 0x8069,
			GL_TEXTURE_BINDING_2D_ARRAY      = 0x8C1D,
			GL_TEXTURE_BINDING_2D_MULTISAMPLE = 0x9104,
			GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY = 0x9105,
			GL_TEXTURE_BINDING_3D            = 0x806A,
			GL_TEXTURE_BINDING_BUFFER        = 0x8C2C,
			GL_TEXTURE_BINDING_CUBE_MAP      = 0x8514,
			GL_TEXTURE_BINDING_CUBE_MAP_ARRAY = 0x900A,
			GL_TEXTURE_BINDING_RECTANGLE     = 0x84F6,
			GL_TEXTURE_TARGET                = 0x1006,
			
			GL_CONTEXT_RELEASE_BEHAVIOR      = 0x82FB,
			GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH = 0x82FC,
			GL_NONE                          = 0,
			
			GL_CONTEXT_LOST                  = 0x0507,
			GL_CONTEXT_ROBUST_ACCESS         = 0x90F3,
			GL_GUILTY_CONTEXT_RESET          = 0x8253,
			GL_INNOCENT_CONTEXT_RESET        = 0x8254,
			GL_LOSE_CONTEXT_ON_RESET         = 0x8252,
			GL_NO_ERROR                      = 0,
			GL_NO_RESET_NOTIFICATION         = 0x8261,
			GL_RESET_NOTIFICATION_STRATEGY   = 0x8256,
			GL_UNKNOWN_CONTEXT_RESET         = 0x8255,
			
			GL_ALPHA                         = 0x1906,
			GL_ALWAYS                        = 0x0207,
			GL_AND                           = 0x1501,
			GL_AND_INVERTED                  = 0x1504,
			GL_AND_REVERSE                   = 0x1502,
			GL_BACK                          = 0x0405,
			GL_BACK_LEFT                     = 0x0402,
			GL_BACK_RIGHT                    = 0x0403,
			GL_BLEND                         = 0x0BE2,
			GL_BLEND_DST                     = 0x0BE0,
			GL_BLEND_SRC                     = 0x0BE1,
			GL_BLUE                          = 0x1905,
			GL_BYTE                          = 0x1400,
			GL_CCW                           = 0x0901,
			GL_CLEAR                         = 0x1500,
			GL_COLOR                         = 0x1800,
			GL_COLOR_BUFFER_BIT              = 0x00004000,
			GL_COLOR_CLEAR_VALUE             = 0x0C22,
			GL_COLOR_LOGIC_OP                = 0x0BF2,
			GL_COLOR_WRITEMASK               = 0x0C23,
			GL_COPY                          = 0x1503,
			GL_COPY_INVERTED                 = 0x150C,
			GL_CULL_FACE                     = 0x0B44,
			GL_CULL_FACE_MODE                = 0x0B45,
			GL_CW                            = 0x0900,
			GL_DECR                          = 0x1E03,
			GL_DEPTH                         = 0x1801,
			GL_DEPTH_BUFFER_BIT              = 0x00000100,
			GL_DEPTH_CLEAR_VALUE             = 0x0B73,
			GL_DEPTH_COMPONENT               = 0x1902,
			GL_DEPTH_FUNC                    = 0x0B74,
			//DEPTH_RANGE taken from ext: ARB_viewport_array
			GL_DEPTH_TEST                    = 0x0B71,
			GL_DEPTH_WRITEMASK               = 0x0B72,
			GL_DITHER                        = 0x0BD0,
			GL_DONT_CARE                     = 0x1100,
			GL_DOUBLE                        = 0x140A,
			GL_DOUBLEBUFFER                  = 0x0C32,
			GL_DRAW_BUFFER                   = 0x0C01,
			GL_DST_ALPHA                     = 0x0304,
			GL_DST_COLOR                     = 0x0306,
			GL_EQUAL                         = 0x0202,
			GL_EQUIV                         = 0x1509,
			GL_EXTENSIONS                    = 0x1F03,
			GL_FALSE                         = 0,
			GL_FASTEST                       = 0x1101,
			GL_FILL                          = 0x1B02,
			GL_FLOAT                         = 0x1406,
			GL_FRONT                         = 0x0404,
			GL_FRONT_AND_BACK                = 0x0408,
			GL_FRONT_FACE                    = 0x0B46,
			GL_FRONT_LEFT                    = 0x0400,
			GL_FRONT_RIGHT                   = 0x0401,
			GL_GEQUAL                        = 0x0206,
			GL_GREATER                       = 0x0204,
			GL_GREEN                         = 0x1904,
			GL_INCR                          = 0x1E02,
			GL_INT                           = 0x1404,
			GL_INVALID_ENUM                  = 0x0500,
			GL_INVALID_OPERATION             = 0x0502,
			GL_INVALID_VALUE                 = 0x0501,
			GL_INVERT                        = 0x150A,
			GL_KEEP                          = 0x1E00,
			GL_LEFT                          = 0x0406,
			GL_LEQUAL                        = 0x0203,
			GL_LESS                          = 0x0201,
			GL_LINE                          = 0x1B01,
			GL_LINEAR                        = 0x2601,
			GL_LINEAR_MIPMAP_LINEAR          = 0x2703,
			GL_LINEAR_MIPMAP_NEAREST         = 0x2701,
			GL_LINES                         = 0x0001,
			GL_LINE_LOOP                     = 0x0002,
			GL_LINE_SMOOTH                   = 0x0B20,
			GL_LINE_SMOOTH_HINT              = 0x0C52,
			GL_LINE_STRIP                    = 0x0003,
			GL_LINE_WIDTH                    = 0x0B21,
			GL_LINE_WIDTH_GRANULARITY        = 0x0B23,
			GL_LINE_WIDTH_RANGE              = 0x0B22,
			GL_LOGIC_OP_MODE                 = 0x0BF0,
			GL_MAX_TEXTURE_SIZE              = 0x0D33,
			GL_MAX_VIEWPORT_DIMS             = 0x0D3A,
			GL_NAND                          = 0x150E,
			GL_NEAREST                       = 0x2600,
			GL_NEAREST_MIPMAP_LINEAR         = 0x2702,
			GL_NEAREST_MIPMAP_NEAREST        = 0x2700,
			GL_NEVER                         = 0x0200,
			GL_NICEST                        = 0x1102,
			//NONE taken from ext: KHR_context_flush_control
			GL_NOOP                          = 0x1505,
			GL_NOR                           = 0x1508,
			GL_NOTEQUAL                      = 0x0205,
			//NO_ERROR taken from ext: KHR_robustness
			GL_ONE                           = 1,
			GL_ONE_MINUS_DST_ALPHA           = 0x0305,
			GL_ONE_MINUS_DST_COLOR           = 0x0307,
			GL_ONE_MINUS_SRC_ALPHA           = 0x0303,
			GL_ONE_MINUS_SRC_COLOR           = 0x0301,
			GL_OR                            = 0x1507,
			GL_OR_INVERTED                   = 0x150D,
			GL_OR_REVERSE                    = 0x150B,
			GL_OUT_OF_MEMORY                 = 0x0505,
			GL_PACK_ALIGNMENT                = 0x0D05,
			GL_PACK_LSB_FIRST                = 0x0D01,
			GL_PACK_ROW_LENGTH               = 0x0D02,
			GL_PACK_SKIP_PIXELS              = 0x0D04,
			GL_PACK_SKIP_ROWS                = 0x0D03,
			GL_PACK_SWAP_BYTES               = 0x0D00,
			GL_POINT                         = 0x1B00,
			GL_POINTS                        = 0x0000,
			GL_POINT_SIZE                    = 0x0B11,
			GL_POINT_SIZE_GRANULARITY        = 0x0B13,
			GL_POINT_SIZE_RANGE              = 0x0B12,
			GL_POLYGON_MODE                  = 0x0B40,
			GL_POLYGON_OFFSET_FACTOR         = 0x8038,
			GL_POLYGON_OFFSET_FILL           = 0x8037,
			GL_POLYGON_OFFSET_LINE           = 0x2A02,
			GL_POLYGON_OFFSET_POINT          = 0x2A01,
			GL_POLYGON_OFFSET_UNITS          = 0x2A00,
			GL_POLYGON_SMOOTH                = 0x0B41,
			GL_POLYGON_SMOOTH_HINT           = 0x0C53,
			GL_PROXY_TEXTURE_1D              = 0x8063,
			GL_PROXY_TEXTURE_2D              = 0x8064,
			GL_R3_G3_B2                      = 0x2A10,
			GL_READ_BUFFER                   = 0x0C02,
			GL_RED                           = 0x1903,
			GL_RENDERER                      = 0x1F01,
			GL_REPEAT                        = 0x2901,
			GL_REPLACE                       = 0x1E01,
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
			GL_RIGHT                         = 0x0407,
			//SCISSOR_BOX taken from ext: ARB_viewport_array
			//SCISSOR_TEST taken from ext: ARB_viewport_array
			GL_SET                           = 0x150F,
			GL_SHORT                         = 0x1402,
			GL_SRC_ALPHA                     = 0x0302,
			GL_SRC_ALPHA_SATURATE            = 0x0308,
			GL_SRC_COLOR                     = 0x0300,
			GL_STENCIL                       = 0x1802,
			GL_STENCIL_BUFFER_BIT            = 0x00000400,
			GL_STENCIL_CLEAR_VALUE           = 0x0B91,
			GL_STENCIL_FAIL                  = 0x0B94,
			GL_STENCIL_FUNC                  = 0x0B92,
			//STENCIL_INDEX taken from ext: ARB_texture_stencil8
			GL_STENCIL_PASS_DEPTH_FAIL       = 0x0B95,
			GL_STENCIL_PASS_DEPTH_PASS       = 0x0B96,
			GL_STENCIL_REF                   = 0x0B97,
			GL_STENCIL_TEST                  = 0x0B90,
			GL_STENCIL_VALUE_MASK            = 0x0B93,
			GL_STENCIL_WRITEMASK             = 0x0B98,
			GL_STEREO                        = 0x0C33,
			GL_SUBPIXEL_BITS                 = 0x0D50,
			GL_TEXTURE                       = 0x1702,
			//TEXTURE_1D taken from ext: ARB_internalformat_query2
			//TEXTURE_2D taken from ext: ARB_internalformat_query2
			GL_TEXTURE_ALPHA_SIZE            = 0x805F,
			//TEXTURE_BINDING_1D taken from ext: ARB_direct_state_access
			//TEXTURE_BINDING_2D taken from ext: ARB_direct_state_access
			GL_TEXTURE_BLUE_SIZE             = 0x805E,
			GL_TEXTURE_BORDER_COLOR          = 0x1004,
			GL_TEXTURE_GREEN_SIZE            = 0x805D,
			GL_TEXTURE_HEIGHT                = 0x1001,
			GL_TEXTURE_INTERNAL_FORMAT       = 0x1003,
			GL_TEXTURE_MAG_FILTER            = 0x2800,
			GL_TEXTURE_MIN_FILTER            = 0x2801,
			GL_TEXTURE_RED_SIZE              = 0x805C,
			GL_TEXTURE_WIDTH                 = 0x1000,
			GL_TEXTURE_WRAP_S                = 0x2802,
			GL_TEXTURE_WRAP_T                = 0x2803,
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
			GL_VENDOR                        = 0x1F00,
			GL_VERSION                       = 0x1F02,
			//VIEWPORT taken from ext: ARB_viewport_array
			GL_XOR                           = 0x1506,
			GL_ZERO                          = 0,
			
			GL_ALIASED_LINE_WIDTH_RANGE      = 0x846E,
			GL_BGR                           = 0x80E0,
			GL_BGRA                          = 0x80E1,
			GL_CLAMP_TO_EDGE                 = 0x812F,
			GL_MAX_3D_TEXTURE_SIZE           = 0x8073,
			GL_MAX_ELEMENTS_INDICES          = 0x80E9,
			GL_MAX_ELEMENTS_VERTICES         = 0x80E8,
			GL_PACK_IMAGE_HEIGHT             = 0x806C,
			GL_PACK_SKIP_IMAGES              = 0x806B,
			GL_PROXY_TEXTURE_3D              = 0x8070,
			GL_SMOOTH_LINE_WIDTH_GRANULARITY = 0x0B23,
			GL_SMOOTH_LINE_WIDTH_RANGE       = 0x0B22,
			GL_SMOOTH_POINT_SIZE_GRANULARITY = 0x0B13,
			GL_SMOOTH_POINT_SIZE_RANGE       = 0x0B12,
			//TEXTURE_3D taken from ext: ARB_internalformat_query2
			GL_TEXTURE_BASE_LEVEL            = 0x813C,
			//TEXTURE_BINDING_3D taken from ext: ARB_direct_state_access
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
			GL_CLAMP_TO_BORDER               = 0x812D,
			GL_COMPRESSED_RGB                = 0x84ED,
			GL_COMPRESSED_RGBA               = 0x84EE,
			GL_COMPRESSED_TEXTURE_FORMATS    = 0x86A3,
			GL_MAX_CUBE_MAP_TEXTURE_SIZE     = 0x851C,
			GL_MULTISAMPLE                   = 0x809D,
			GL_NUM_COMPRESSED_TEXTURE_FORMATS = 0x86A2,
			GL_PROXY_TEXTURE_CUBE_MAP        = 0x851B,
			//SAMPLES taken from ext: ARB_internalformat_query2
			GL_SAMPLE_ALPHA_TO_COVERAGE      = 0x809E,
			GL_SAMPLE_ALPHA_TO_ONE           = 0x809F,
			GL_SAMPLE_BUFFERS                = 0x80A8,
			GL_SAMPLE_COVERAGE               = 0x80A0,
			GL_SAMPLE_COVERAGE_INVERT        = 0x80AB,
			GL_SAMPLE_COVERAGE_VALUE         = 0x80AA,
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
			//TEXTURE_BINDING_CUBE_MAP taken from ext: ARB_direct_state_access
			//TEXTURE_COMPRESSED taken from ext: ARB_internalformat_query2
			GL_TEXTURE_COMPRESSED_IMAGE_SIZE = 0x86A0,
			GL_TEXTURE_COMPRESSION_HINT      = 0x84EF,
			//TEXTURE_CUBE_MAP taken from ext: ARB_internalformat_query2
			GL_TEXTURE_CUBE_MAP_NEGATIVE_X   = 0x8516,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Y   = 0x8518,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Z   = 0x851A,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X   = 0x8515,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Y   = 0x8517,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Z   = 0x8519,
			
			GL_BLEND_COLOR                   = 0x8005,
			GL_BLEND_DST_ALPHA               = 0x80CA,
			GL_BLEND_DST_RGB                 = 0x80C8,
			GL_BLEND_SRC_ALPHA               = 0x80CB,
			GL_BLEND_SRC_RGB                 = 0x80C9,
			GL_CONSTANT_ALPHA                = 0x8003,
			GL_CONSTANT_COLOR                = 0x8001,
			GL_DECR_WRAP                     = 0x8508,
			GL_DEPTH_COMPONENT16             = 0x81A5,
			GL_DEPTH_COMPONENT24             = 0x81A6,
			GL_DEPTH_COMPONENT32             = 0x81A7,
			GL_FUNC_ADD                      = 0x8006,
			GL_FUNC_REVERSE_SUBTRACT         = 0x800B,
			GL_FUNC_SUBTRACT                 = 0x800A,
			GL_INCR_WRAP                     = 0x8507,
			GL_MAX                           = 0x8008,
			GL_MAX_TEXTURE_LOD_BIAS          = 0x84FD,
			GL_MIN                           = 0x8007,
			GL_MIRRORED_REPEAT               = 0x8370,
			GL_ONE_MINUS_CONSTANT_ALPHA      = 0x8004,
			GL_ONE_MINUS_CONSTANT_COLOR      = 0x8002,
			GL_POINT_FADE_THRESHOLD_SIZE     = 0x8128,
			GL_TEXTURE_COMPARE_FUNC          = 0x884D,
			GL_TEXTURE_COMPARE_MODE          = 0x884C,
			GL_TEXTURE_DEPTH_SIZE            = 0x884A,
			GL_TEXTURE_LOD_BIAS              = 0x8501,
			
			GL_ARRAY_BUFFER                  = 0x8892,
			GL_ARRAY_BUFFER_BINDING          = 0x8894,
			GL_BUFFER_ACCESS                 = 0x88BB,
			GL_BUFFER_MAPPED                 = 0x88BC,
			GL_BUFFER_MAP_POINTER            = 0x88BD,
			GL_BUFFER_SIZE                   = 0x8764,
			GL_BUFFER_USAGE                  = 0x8765,
			GL_CURRENT_QUERY                 = 0x8865,
			GL_DYNAMIC_COPY                  = 0x88EA,
			GL_DYNAMIC_DRAW                  = 0x88E8,
			GL_DYNAMIC_READ                  = 0x88E9,
			GL_ELEMENT_ARRAY_BUFFER          = 0x8893,
			GL_ELEMENT_ARRAY_BUFFER_BINDING  = 0x8895,
			GL_QUERY_COUNTER_BITS            = 0x8864,
			GL_QUERY_RESULT                  = 0x8866,
			GL_QUERY_RESULT_AVAILABLE        = 0x8867,
			GL_READ_ONLY                     = 0x88B8,
			GL_READ_WRITE                    = 0x88BA,
			GL_SAMPLES_PASSED                = 0x8914,
			GL_SRC1_ALPHA                    = 0x8589,
			GL_STATIC_COPY                   = 0x88E6,
			GL_STATIC_DRAW                   = 0x88E4,
			GL_STATIC_READ                   = 0x88E5,
			GL_STREAM_COPY                   = 0x88E2,
			GL_STREAM_DRAW                   = 0x88E0,
			GL_STREAM_READ                   = 0x88E1,
			GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING = 0x889F,
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
			//LOWER_LEFT taken from ext: ARB_clip_control
			GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS = 0x8B4D,
			GL_MAX_DRAW_BUFFERS              = 0x8824,
			GL_MAX_FRAGMENT_UNIFORM_COMPONENTS = 0x8B49,
			GL_MAX_TEXTURE_IMAGE_UNITS       = 0x8872,
			GL_MAX_VARYING_FLOATS            = 0x8B4B,
			GL_MAX_VERTEX_ATTRIBS            = 0x8869,
			GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS = 0x8B4C,
			GL_MAX_VERTEX_UNIFORM_COMPONENTS = 0x8B4A,
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
			//UPPER_LEFT taken from ext: ARB_clip_control
			GL_VALIDATE_STATUS               = 0x8B83,
			GL_VERTEX_ATTRIB_ARRAY_ENABLED   = 0x8622,
			GL_VERTEX_ATTRIB_ARRAY_NORMALIZED = 0x886A,
			GL_VERTEX_ATTRIB_ARRAY_POINTER   = 0x8645,
			GL_VERTEX_ATTRIB_ARRAY_SIZE      = 0x8623,
			GL_VERTEX_ATTRIB_ARRAY_STRIDE    = 0x8624,
			GL_VERTEX_ATTRIB_ARRAY_TYPE      = 0x8625,
			GL_VERTEX_PROGRAM_POINT_SIZE     = 0x8642,
			GL_VERTEX_SHADER                 = 0x8B31,
			
			GL_COMPRESSED_SRGB               = 0x8C48,
			GL_COMPRESSED_SRGB_ALPHA         = 0x8C49,
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
			GL_SRGB                          = 0x8C40,
			GL_SRGB8                         = 0x8C41,
			GL_SRGB8_ALPHA8                  = 0x8C43,
			GL_SRGB_ALPHA                    = 0x8C42,
			
			GL_BGRA_INTEGER                  = 0x8D9B,
			GL_BGR_INTEGER                   = 0x8D9A,
			GL_BLUE_INTEGER                  = 0x8D96,
			GL_BUFFER_ACCESS_FLAGS           = 0x911F,
			GL_BUFFER_MAP_LENGTH             = 0x9120,
			GL_BUFFER_MAP_OFFSET             = 0x9121,
			GL_CLAMP_READ_COLOR              = 0x891C,
			GL_CLIP_DISTANCE0                = 0x3000,
			GL_CLIP_DISTANCE1                = 0x3001,
			GL_CLIP_DISTANCE2                = 0x3002,
			GL_CLIP_DISTANCE3                = 0x3003,
			GL_CLIP_DISTANCE4                = 0x3004,
			GL_CLIP_DISTANCE5                = 0x3005,
			GL_CLIP_DISTANCE6                = 0x3006,
			GL_CLIP_DISTANCE7                = 0x3007,
			GL_COLOR_ATTACHMENT0             = 0x8CE0,
			GL_COLOR_ATTACHMENT1             = 0x8CE1,
			GL_COLOR_ATTACHMENT10            = 0x8CEA,
			GL_COLOR_ATTACHMENT11            = 0x8CEB,
			GL_COLOR_ATTACHMENT12            = 0x8CEC,
			GL_COLOR_ATTACHMENT13            = 0x8CED,
			GL_COLOR_ATTACHMENT14            = 0x8CEE,
			GL_COLOR_ATTACHMENT15            = 0x8CEF,
			GL_COLOR_ATTACHMENT16            = 0x8CF0,
			GL_COLOR_ATTACHMENT17            = 0x8CF1,
			GL_COLOR_ATTACHMENT18            = 0x8CF2,
			GL_COLOR_ATTACHMENT19            = 0x8CF3,
			GL_COLOR_ATTACHMENT2             = 0x8CE2,
			GL_COLOR_ATTACHMENT20            = 0x8CF4,
			GL_COLOR_ATTACHMENT21            = 0x8CF5,
			GL_COLOR_ATTACHMENT22            = 0x8CF6,
			GL_COLOR_ATTACHMENT23            = 0x8CF7,
			GL_COLOR_ATTACHMENT24            = 0x8CF8,
			GL_COLOR_ATTACHMENT25            = 0x8CF9,
			GL_COLOR_ATTACHMENT26            = 0x8CFA,
			GL_COLOR_ATTACHMENT27            = 0x8CFB,
			GL_COLOR_ATTACHMENT28            = 0x8CFC,
			GL_COLOR_ATTACHMENT29            = 0x8CFD,
			GL_COLOR_ATTACHMENT3             = 0x8CE3,
			GL_COLOR_ATTACHMENT30            = 0x8CFE,
			GL_COLOR_ATTACHMENT31            = 0x8CFF,
			GL_COLOR_ATTACHMENT4             = 0x8CE4,
			GL_COLOR_ATTACHMENT5             = 0x8CE5,
			GL_COLOR_ATTACHMENT6             = 0x8CE6,
			GL_COLOR_ATTACHMENT7             = 0x8CE7,
			GL_COLOR_ATTACHMENT8             = 0x8CE8,
			GL_COLOR_ATTACHMENT9             = 0x8CE9,
			GL_COMPARE_REF_TO_TEXTURE        = 0x884E,
			GL_COMPRESSED_RED                = 0x8225,
			GL_COMPRESSED_RED_RGTC1          = 0x8DBB,
			GL_COMPRESSED_RG                 = 0x8226,
			GL_COMPRESSED_RG_RGTC2           = 0x8DBD,
			GL_COMPRESSED_SIGNED_RED_RGTC1   = 0x8DBC,
			GL_COMPRESSED_SIGNED_RG_RGTC2    = 0x8DBE,
			GL_CONTEXT_FLAGS                 = 0x821E,
			GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT = 0x00000001,
			GL_DEPTH24_STENCIL8              = 0x88F0,
			GL_DEPTH32F_STENCIL8             = 0x8CAD,
			GL_DEPTH_ATTACHMENT              = 0x8D00,
			GL_DEPTH_COMPONENT32F            = 0x8CAC,
			GL_DEPTH_STENCIL                 = 0x84F9,
			GL_DEPTH_STENCIL_ATTACHMENT      = 0x821A,
			GL_DRAW_FRAMEBUFFER              = 0x8CA9,
			GL_DRAW_FRAMEBUFFER_BINDING      = 0x8CA6,
			GL_FIXED_ONLY                    = 0x891D,
			GL_FLOAT_32_UNSIGNED_INT_24_8_REV = 0x8DAD,
			GL_FRAMEBUFFER                   = 0x8D40,
			GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE = 0x8215,
			GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE = 0x8214,
			GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING = 0x8210,
			GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE = 0x8211,
			GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE = 0x8216,
			GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE = 0x8213,
			GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME = 0x8CD1,
			GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE = 0x8CD0,
			GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE = 0x8212,
			GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE = 0x8217,
			GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE = 0x8CD3,
			GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER = 0x8CD4,
			GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL = 0x8CD2,
			GL_FRAMEBUFFER_BINDING           = 0x8CA6,
			GL_FRAMEBUFFER_COMPLETE          = 0x8CD5,
			GL_FRAMEBUFFER_DEFAULT           = 0x8218,
			GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT = 0x8CD6,
			GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER = 0x8CDB,
			GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT = 0x8CD7,
			GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE = 0x8D56,
			GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER = 0x8CDC,
			GL_FRAMEBUFFER_SRGB              = 0x8DB9,
			GL_FRAMEBUFFER_UNDEFINED         = 0x8219,
			GL_FRAMEBUFFER_UNSUPPORTED       = 0x8CDD,
			GL_GREEN_INTEGER                 = 0x8D95,
			GL_HALF_FLOAT                    = 0x140B,
			GL_INTERLEAVED_ATTRIBS           = 0x8C8C,
			GL_INT_SAMPLER_1D                = 0x8DC9,
			GL_INT_SAMPLER_1D_ARRAY          = 0x8DCE,
			GL_INT_SAMPLER_2D                = 0x8DCA,
			GL_INT_SAMPLER_2D_ARRAY          = 0x8DCF,
			GL_INT_SAMPLER_3D                = 0x8DCB,
			GL_INT_SAMPLER_CUBE              = 0x8DCC,
			GL_INVALID_FRAMEBUFFER_OPERATION = 0x0506,
			GL_MAJOR_VERSION                 = 0x821B,
			GL_MAP_FLUSH_EXPLICIT_BIT        = 0x0010,
			GL_MAP_INVALIDATE_BUFFER_BIT     = 0x0008,
			GL_MAP_INVALIDATE_RANGE_BIT      = 0x0004,
			//MAP_READ_BIT taken from ext: ARB_buffer_storage
			GL_MAP_UNSYNCHRONIZED_BIT        = 0x0020,
			//MAP_WRITE_BIT taken from ext: ARB_buffer_storage
			GL_MAX_ARRAY_TEXTURE_LAYERS      = 0x88FF,
			GL_MAX_CLIP_DISTANCES            = 0x0D32,
			GL_MAX_COLOR_ATTACHMENTS         = 0x8CDF,
			GL_MAX_PROGRAM_TEXEL_OFFSET      = 0x8905,
			GL_MAX_RENDERBUFFER_SIZE         = 0x84E8,
			GL_MAX_SAMPLES                   = 0x8D57,
			GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS = 0x8C8A,
			GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS = 0x8C8B,
			GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS = 0x8C80,
			GL_MAX_VARYING_COMPONENTS        = 0x8B4B,
			GL_MINOR_VERSION                 = 0x821C,
			GL_MIN_PROGRAM_TEXEL_OFFSET      = 0x8904,
			GL_NUM_EXTENSIONS                = 0x821D,
			GL_PRIMITIVES_GENERATED          = 0x8C87,
			GL_PROXY_TEXTURE_1D_ARRAY        = 0x8C19,
			GL_PROXY_TEXTURE_2D_ARRAY        = 0x8C1B,
			GL_QUERY_BY_REGION_NO_WAIT       = 0x8E16,
			GL_QUERY_BY_REGION_WAIT          = 0x8E15,
			GL_QUERY_NO_WAIT                 = 0x8E14,
			GL_QUERY_WAIT                    = 0x8E13,
			GL_R11F_G11F_B10F                = 0x8C3A,
			GL_R16                           = 0x822A,
			GL_R16F                          = 0x822D,
			GL_R16I                          = 0x8233,
			GL_R16UI                         = 0x8234,
			GL_R32F                          = 0x822E,
			GL_R32I                          = 0x8235,
			GL_R32UI                         = 0x8236,
			GL_R8                            = 0x8229,
			GL_R8I                           = 0x8231,
			GL_R8UI                          = 0x8232,
			GL_RASTERIZER_DISCARD            = 0x8C89,
			GL_READ_FRAMEBUFFER              = 0x8CA8,
			GL_READ_FRAMEBUFFER_BINDING      = 0x8CAA,
			GL_RED_INTEGER                   = 0x8D94,
			//RENDERBUFFER taken from ext: ARB_internalformat_query2
			GL_RENDERBUFFER_ALPHA_SIZE       = 0x8D53,
			GL_RENDERBUFFER_BINDING          = 0x8CA7,
			GL_RENDERBUFFER_BLUE_SIZE        = 0x8D52,
			GL_RENDERBUFFER_DEPTH_SIZE       = 0x8D54,
			GL_RENDERBUFFER_GREEN_SIZE       = 0x8D51,
			GL_RENDERBUFFER_HEIGHT           = 0x8D43,
			GL_RENDERBUFFER_INTERNAL_FORMAT  = 0x8D44,
			GL_RENDERBUFFER_RED_SIZE         = 0x8D50,
			GL_RENDERBUFFER_SAMPLES          = 0x8CAB,
			GL_RENDERBUFFER_STENCIL_SIZE     = 0x8D55,
			GL_RENDERBUFFER_WIDTH            = 0x8D42,
			GL_RG                            = 0x8227,
			GL_RG16                          = 0x822C,
			GL_RG16F                         = 0x822F,
			GL_RG16I                         = 0x8239,
			GL_RG16UI                        = 0x823A,
			GL_RG32F                         = 0x8230,
			GL_RG32I                         = 0x823B,
			GL_RG32UI                        = 0x823C,
			GL_RG8                           = 0x822B,
			GL_RG8I                          = 0x8237,
			GL_RG8UI                         = 0x8238,
			GL_RGB16F                        = 0x881B,
			GL_RGB16I                        = 0x8D89,
			GL_RGB16UI                       = 0x8D77,
			GL_RGB32F                        = 0x8815,
			GL_RGB32I                        = 0x8D83,
			GL_RGB32UI                       = 0x8D71,
			GL_RGB8I                         = 0x8D8F,
			GL_RGB8UI                        = 0x8D7D,
			GL_RGB9_E5                       = 0x8C3D,
			GL_RGBA16F                       = 0x881A,
			GL_RGBA16I                       = 0x8D88,
			GL_RGBA16UI                      = 0x8D76,
			GL_RGBA32F                       = 0x8814,
			GL_RGBA32I                       = 0x8D82,
			GL_RGBA32UI                      = 0x8D70,
			GL_RGBA8I                        = 0x8D8E,
			GL_RGBA8UI                       = 0x8D7C,
			GL_RGBA_INTEGER                  = 0x8D99,
			GL_RGB_INTEGER                   = 0x8D98,
			GL_RG_INTEGER                    = 0x8228,
			GL_SAMPLER_1D_ARRAY              = 0x8DC0,
			GL_SAMPLER_1D_ARRAY_SHADOW       = 0x8DC3,
			GL_SAMPLER_2D_ARRAY              = 0x8DC1,
			GL_SAMPLER_2D_ARRAY_SHADOW       = 0x8DC4,
			GL_SAMPLER_CUBE_SHADOW           = 0x8DC5,
			GL_SEPARATE_ATTRIBS              = 0x8C8D,
			GL_STENCIL_ATTACHMENT            = 0x8D20,
			GL_STENCIL_INDEX1                = 0x8D46,
			GL_STENCIL_INDEX16               = 0x8D49,
			GL_STENCIL_INDEX4                = 0x8D47,
			//STENCIL_INDEX8 taken from ext: ARB_texture_stencil8
			//TEXTURE_1D_ARRAY taken from ext: ARB_internalformat_query2
			//TEXTURE_2D_ARRAY taken from ext: ARB_internalformat_query2
			GL_TEXTURE_ALPHA_TYPE            = 0x8C13,
			//TEXTURE_BINDING_1D_ARRAY taken from ext: ARB_direct_state_access
			//TEXTURE_BINDING_2D_ARRAY taken from ext: ARB_direct_state_access
			GL_TEXTURE_BLUE_TYPE             = 0x8C12,
			GL_TEXTURE_DEPTH_TYPE            = 0x8C16,
			GL_TEXTURE_GREEN_TYPE            = 0x8C11,
			GL_TEXTURE_RED_TYPE              = 0x8C10,
			GL_TEXTURE_SHARED_SIZE           = 0x8C3F,
			GL_TEXTURE_STENCIL_SIZE          = 0x88F1,
			//TRANSFORM_FEEDBACK_BUFFER taken from ext: ARB_enhanced_layouts
			GL_TRANSFORM_FEEDBACK_BUFFER_BINDING = 0x8C8F,
			GL_TRANSFORM_FEEDBACK_BUFFER_MODE = 0x8C7F,
			GL_TRANSFORM_FEEDBACK_BUFFER_SIZE = 0x8C85,
			GL_TRANSFORM_FEEDBACK_BUFFER_START = 0x8C84,
			GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN = 0x8C88,
			GL_TRANSFORM_FEEDBACK_VARYINGS   = 0x8C83,
			GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH = 0x8C76,
			//UNSIGNED_INT_10F_11F_11F_REV taken from ext: ARB_vertex_type_10f_11f_11f_rev
			GL_UNSIGNED_INT_24_8             = 0x84FA,
			GL_UNSIGNED_INT_5_9_9_9_REV      = 0x8C3E,
			GL_UNSIGNED_INT_SAMPLER_1D       = 0x8DD1,
			GL_UNSIGNED_INT_SAMPLER_1D_ARRAY = 0x8DD6,
			GL_UNSIGNED_INT_SAMPLER_2D       = 0x8DD2,
			GL_UNSIGNED_INT_SAMPLER_2D_ARRAY = 0x8DD7,
			GL_UNSIGNED_INT_SAMPLER_3D       = 0x8DD3,
			GL_UNSIGNED_INT_SAMPLER_CUBE     = 0x8DD4,
			GL_UNSIGNED_INT_VEC2             = 0x8DC6,
			GL_UNSIGNED_INT_VEC3             = 0x8DC7,
			GL_UNSIGNED_INT_VEC4             = 0x8DC8,
			GL_UNSIGNED_NORMALIZED           = 0x8C17,
			GL_VERTEX_ARRAY_BINDING          = 0x85B5,
			GL_VERTEX_ATTRIB_ARRAY_INTEGER   = 0x88FD,
			
			GL_ACTIVE_UNIFORM_BLOCKS         = 0x8A36,
			GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH = 0x8A35,
			GL_COPY_READ_BUFFER              = 0x8F36,
			GL_COPY_WRITE_BUFFER             = 0x8F37,
			GL_INT_SAMPLER_2D_RECT           = 0x8DCD,
			GL_INT_SAMPLER_BUFFER            = 0x8DD0,
			GL_INVALID_INDEX                 = 0xFFFFFFFF,
			GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS = 0x8A33,
			GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS = 0x8A32,
			GL_MAX_COMBINED_UNIFORM_BLOCKS   = 0x8A2E,
			GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS = 0x8A31,
			GL_MAX_FRAGMENT_UNIFORM_BLOCKS   = 0x8A2D,
			GL_MAX_GEOMETRY_UNIFORM_BLOCKS   = 0x8A2C,
			GL_MAX_RECTANGLE_TEXTURE_SIZE    = 0x84F8,
			GL_MAX_TEXTURE_BUFFER_SIZE       = 0x8C2B,
			GL_MAX_UNIFORM_BLOCK_SIZE        = 0x8A30,
			GL_MAX_UNIFORM_BUFFER_BINDINGS   = 0x8A2F,
			GL_MAX_VERTEX_UNIFORM_BLOCKS     = 0x8A2B,
			GL_PRIMITIVE_RESTART             = 0x8F9D,
			GL_PRIMITIVE_RESTART_INDEX       = 0x8F9E,
			GL_PROXY_TEXTURE_RECTANGLE       = 0x84F7,
			GL_R16_SNORM                     = 0x8F98,
			GL_R8_SNORM                      = 0x8F94,
			GL_RG16_SNORM                    = 0x8F99,
			GL_RG8_SNORM                     = 0x8F95,
			GL_RGB16_SNORM                   = 0x8F9A,
			GL_RGB8_SNORM                    = 0x8F96,
			GL_RGBA16_SNORM                  = 0x8F9B,
			GL_RGBA8_SNORM                   = 0x8F97,
			GL_SAMPLER_2D_RECT               = 0x8B63,
			GL_SAMPLER_2D_RECT_SHADOW        = 0x8B64,
			GL_SAMPLER_BUFFER                = 0x8DC2,
			GL_SIGNED_NORMALIZED             = 0x8F9C,
			//TEXTURE_BINDING_BUFFER taken from ext: ARB_direct_state_access
			//TEXTURE_BINDING_RECTANGLE taken from ext: ARB_direct_state_access
			//TEXTURE_BUFFER taken from ext: ARB_internalformat_query2
			GL_TEXTURE_BUFFER_DATA_STORE_BINDING = 0x8C2D,
			//TEXTURE_RECTANGLE taken from ext: ARB_internalformat_query2
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
			GL_UNSIGNED_INT_SAMPLER_2D_RECT  = 0x8DD5,
			GL_UNSIGNED_INT_SAMPLER_BUFFER   = 0x8DD8,
			
			GL_ALREADY_SIGNALED              = 0x911A,
			GL_CONDITION_SATISFIED           = 0x911C,
			GL_CONTEXT_COMPATIBILITY_PROFILE_BIT = 0x00000002,
			GL_CONTEXT_CORE_PROFILE_BIT      = 0x00000001,
			GL_CONTEXT_PROFILE_MASK          = 0x9126,
			GL_DEPTH_CLAMP                   = 0x864F,
			//FIRST_VERTEX_CONVENTION taken from ext: ARB_viewport_array
			GL_FRAMEBUFFER_ATTACHMENT_LAYERED = 0x8DA7,
			GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS = 0x8DA8,
			GL_GEOMETRY_INPUT_TYPE           = 0x8917,
			GL_GEOMETRY_OUTPUT_TYPE          = 0x8918,
			GL_GEOMETRY_SHADER               = 0x8DD9,
			GL_GEOMETRY_VERTICES_OUT         = 0x8916,
			GL_INT_SAMPLER_2D_MULTISAMPLE    = 0x9109,
			GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY = 0x910C,
			//LAST_VERTEX_CONVENTION taken from ext: ARB_viewport_array
			GL_LINES_ADJACENCY               = 0x000A,
			GL_LINE_STRIP_ADJACENCY          = 0x000B,
			GL_MAX_COLOR_TEXTURE_SAMPLES     = 0x910E,
			GL_MAX_DEPTH_TEXTURE_SAMPLES     = 0x910F,
			GL_MAX_FRAGMENT_INPUT_COMPONENTS = 0x9125,
			GL_MAX_GEOMETRY_INPUT_COMPONENTS = 0x9123,
			GL_MAX_GEOMETRY_OUTPUT_COMPONENTS = 0x9124,
			GL_MAX_GEOMETRY_OUTPUT_VERTICES  = 0x8DE0,
			GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS = 0x8C29,
			GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS = 0x8DE1,
			GL_MAX_GEOMETRY_UNIFORM_COMPONENTS = 0x8DDF,
			GL_MAX_INTEGER_SAMPLES           = 0x9110,
			GL_MAX_SAMPLE_MASK_WORDS         = 0x8E59,
			GL_MAX_SERVER_WAIT_TIMEOUT       = 0x9111,
			GL_MAX_VERTEX_OUTPUT_COMPONENTS  = 0x9122,
			GL_OBJECT_TYPE                   = 0x9112,
			GL_PROGRAM_POINT_SIZE            = 0x8642,
			//PROVOKING_VERTEX taken from ext: ARB_viewport_array
			GL_PROXY_TEXTURE_2D_MULTISAMPLE  = 0x9101,
			GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY = 0x9103,
			GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION = 0x8E4C,
			GL_SAMPLER_2D_MULTISAMPLE        = 0x9108,
			GL_SAMPLER_2D_MULTISAMPLE_ARRAY  = 0x910B,
			GL_SAMPLE_MASK                   = 0x8E51,
			GL_SAMPLE_MASK_VALUE             = 0x8E52,
			GL_SAMPLE_POSITION               = 0x8E50,
			GL_SIGNALED                      = 0x9119,
			GL_SYNC_CONDITION                = 0x9113,
			GL_SYNC_FENCE                    = 0x9116,
			GL_SYNC_FLAGS                    = 0x9115,
			GL_SYNC_FLUSH_COMMANDS_BIT       = 0x00000001,
			GL_SYNC_GPU_COMMANDS_COMPLETE    = 0x9117,
			GL_SYNC_STATUS                   = 0x9114,
			//TEXTURE_2D_MULTISAMPLE taken from ext: ARB_internalformat_query2
			//TEXTURE_2D_MULTISAMPLE_ARRAY taken from ext: ARB_internalformat_query2
			//TEXTURE_BINDING_2D_MULTISAMPLE taken from ext: ARB_direct_state_access
			//TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY taken from ext: ARB_direct_state_access
			//TEXTURE_CUBE_MAP_SEAMLESS taken from ext: ARB_seamless_cubemap_per_texture
			GL_TEXTURE_FIXED_SAMPLE_LOCATIONS = 0x9107,
			GL_TEXTURE_SAMPLES               = 0x9106,
			GL_TIMEOUT_EXPIRED               = 0x911B,
			GL_TIMEOUT_IGNORED               = 0xFFFFFFFFFFFFFFFF,
			GL_TRIANGLES_ADJACENCY           = 0x000C,
			GL_TRIANGLE_STRIP_ADJACENCY      = 0x000D,
			GL_UNSIGNALED                    = 0x9118,
			GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE = 0x910A,
			GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY = 0x910D,
			GL_WAIT_FAILED                   = 0x911D,
			
			GL_ANY_SAMPLES_PASSED            = 0x8C2F,
			GL_INT_2_10_10_10_REV            = 0x8D9F,
			GL_MAX_DUAL_SOURCE_DRAW_BUFFERS  = 0x88FC,
			GL_ONE_MINUS_SRC1_ALPHA          = 0x88FB,
			GL_ONE_MINUS_SRC1_COLOR          = 0x88FA,
			GL_RGB10_A2UI                    = 0x906F,
			GL_SAMPLER_BINDING               = 0x8919,
			GL_SRC1_COLOR                    = 0x88F9,
			GL_TEXTURE_SWIZZLE_A             = 0x8E45,
			GL_TEXTURE_SWIZZLE_B             = 0x8E44,
			GL_TEXTURE_SWIZZLE_G             = 0x8E43,
			GL_TEXTURE_SWIZZLE_R             = 0x8E42,
			GL_TEXTURE_SWIZZLE_RGBA          = 0x8E46,
			GL_TIMESTAMP                     = 0x8E28,
			GL_TIME_ELAPSED                  = 0x88BF,
			GL_VERTEX_ATTRIB_ARRAY_DIVISOR   = 0x88FE,
			
		};
		
		
		
		
		
		extern void (CODEGEN_FUNCPTR *glClearDepthf)(GLfloat d);
		extern void (CODEGEN_FUNCPTR *glDepthRangef)(GLfloat n, GLfloat f);
		extern void (CODEGEN_FUNCPTR *glGetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint * range, GLint * precision);
		extern void (CODEGEN_FUNCPTR *glReleaseShaderCompiler)(void);
		extern void (CODEGEN_FUNCPTR *glShaderBinary)(GLsizei count, const GLuint * shaders, GLenum binaryformat, const void * binary, GLsizei length);
		
		extern void (CODEGEN_FUNCPTR *glGetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei * length, GLenum * binaryFormat, void * binary);
		extern void (CODEGEN_FUNCPTR *glProgramBinary)(GLuint program, GLenum binaryFormat, const void * binary, GLsizei length);
		extern void (CODEGEN_FUNCPTR *glProgramParameteri)(GLuint program, GLenum pname, GLint value);
		
		
		extern void (CODEGEN_FUNCPTR *glGetInternalformativ)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint * params);
		
		extern void (CODEGEN_FUNCPTR *glGetInternalformati64v)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64 * params);
		
		
		extern void (CODEGEN_FUNCPTR *glGetProgramInterfaceiv)(GLuint program, GLenum programInterface, GLenum pname, GLint * params);
		extern GLuint (CODEGEN_FUNCPTR *glGetProgramResourceIndex)(GLuint program, GLenum programInterface, const GLchar * name);
		extern GLint (CODEGEN_FUNCPTR *glGetProgramResourceLocation)(GLuint program, GLenum programInterface, const GLchar * name);
		extern GLint (CODEGEN_FUNCPTR *glGetProgramResourceLocationIndex)(GLuint program, GLenum programInterface, const GLchar * name);
		extern void (CODEGEN_FUNCPTR *glGetProgramResourceName)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei * length, GLchar * name);
		extern void (CODEGEN_FUNCPTR *glGetProgramResourceiv)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum * props, GLsizei bufSize, GLsizei * length, GLint * params);
		
		extern void (CODEGEN_FUNCPTR *glActiveShaderProgram)(GLuint pipeline, GLuint program);
		extern void (CODEGEN_FUNCPTR *glBindProgramPipeline)(GLuint pipeline);
		extern GLuint (CODEGEN_FUNCPTR *glCreateShaderProgramv)(GLenum type, GLsizei count, const GLchar *const* strings);
		extern void (CODEGEN_FUNCPTR *glDeleteProgramPipelines)(GLsizei n, const GLuint * pipelines);
		extern void (CODEGEN_FUNCPTR *glGenProgramPipelines)(GLsizei n, GLuint * pipelines);
		extern void (CODEGEN_FUNCPTR *glGetProgramPipelineInfoLog)(GLuint pipeline, GLsizei bufSize, GLsizei * length, GLchar * infoLog);
		extern void (CODEGEN_FUNCPTR *glGetProgramPipelineiv)(GLuint pipeline, GLenum pname, GLint * params);
		extern GLboolean (CODEGEN_FUNCPTR *glIsProgramPipeline)(GLuint pipeline);
		extern void (CODEGEN_FUNCPTR *glProgramUniform1d)(GLuint program, GLint location, GLdouble v0);
		extern void (CODEGEN_FUNCPTR *glProgramUniform1dv)(GLuint program, GLint location, GLsizei count, const GLdouble * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniform1f)(GLuint program, GLint location, GLfloat v0);
		extern void (CODEGEN_FUNCPTR *glProgramUniform1fv)(GLuint program, GLint location, GLsizei count, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniform1i)(GLuint program, GLint location, GLint v0);
		extern void (CODEGEN_FUNCPTR *glProgramUniform1iv)(GLuint program, GLint location, GLsizei count, const GLint * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniform1ui)(GLuint program, GLint location, GLuint v0);
		extern void (CODEGEN_FUNCPTR *glProgramUniform1uiv)(GLuint program, GLint location, GLsizei count, const GLuint * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniform2d)(GLuint program, GLint location, GLdouble v0, GLdouble v1);
		extern void (CODEGEN_FUNCPTR *glProgramUniform2dv)(GLuint program, GLint location, GLsizei count, const GLdouble * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniform2f)(GLuint program, GLint location, GLfloat v0, GLfloat v1);
		extern void (CODEGEN_FUNCPTR *glProgramUniform2fv)(GLuint program, GLint location, GLsizei count, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniform2i)(GLuint program, GLint location, GLint v0, GLint v1);
		extern void (CODEGEN_FUNCPTR *glProgramUniform2iv)(GLuint program, GLint location, GLsizei count, const GLint * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniform2ui)(GLuint program, GLint location, GLuint v0, GLuint v1);
		extern void (CODEGEN_FUNCPTR *glProgramUniform2uiv)(GLuint program, GLint location, GLsizei count, const GLuint * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniform3d)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2);
		extern void (CODEGEN_FUNCPTR *glProgramUniform3dv)(GLuint program, GLint location, GLsizei count, const GLdouble * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniform3f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
		extern void (CODEGEN_FUNCPTR *glProgramUniform3fv)(GLuint program, GLint location, GLsizei count, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniform3i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
		extern void (CODEGEN_FUNCPTR *glProgramUniform3iv)(GLuint program, GLint location, GLsizei count, const GLint * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniform3ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
		extern void (CODEGEN_FUNCPTR *glProgramUniform3uiv)(GLuint program, GLint location, GLsizei count, const GLuint * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniform4d)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
		extern void (CODEGEN_FUNCPTR *glProgramUniform4dv)(GLuint program, GLint location, GLsizei count, const GLdouble * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniform4f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
		extern void (CODEGEN_FUNCPTR *glProgramUniform4fv)(GLuint program, GLint location, GLsizei count, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniform4i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
		extern void (CODEGEN_FUNCPTR *glProgramUniform4iv)(GLuint program, GLint location, GLsizei count, const GLint * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniform4ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
		extern void (CODEGEN_FUNCPTR *glProgramUniform4uiv)(GLuint program, GLint location, GLsizei count, const GLuint * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix2x3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix2x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix2x4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix2x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix3x2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix3x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix3x4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix3x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix4x2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix4x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix4x3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);
		extern void (CODEGEN_FUNCPTR *glProgramUniformMatrix4x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glUseProgramStages)(GLuint pipeline, GLbitfield stages, GLuint program);
		extern void (CODEGEN_FUNCPTR *glValidateProgramPipeline)(GLuint pipeline);
		
		
		
		extern void (CODEGEN_FUNCPTR *glTexBufferRange)(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
		
		extern void (CODEGEN_FUNCPTR *glTexStorage1D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
		extern void (CODEGEN_FUNCPTR *glTexStorage2D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *glTexStorage3D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
		
		extern void (CODEGEN_FUNCPTR *glTextureView)(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
		
		extern void (CODEGEN_FUNCPTR *glBindVertexBuffer)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
		extern void (CODEGEN_FUNCPTR *glVertexAttribBinding)(GLuint attribindex, GLuint bindingindex);
		extern void (CODEGEN_FUNCPTR *glVertexAttribFormat)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
		extern void (CODEGEN_FUNCPTR *glVertexAttribIFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
		extern void (CODEGEN_FUNCPTR *glVertexAttribLFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
		extern void (CODEGEN_FUNCPTR *glVertexBindingDivisor)(GLuint bindingindex, GLuint divisor);
		
		extern void (CODEGEN_FUNCPTR *glDepthRangeArrayv)(GLuint first, GLsizei count, const GLdouble * v);
		extern void (CODEGEN_FUNCPTR *glDepthRangeIndexed)(GLuint index, GLdouble n, GLdouble f);
		extern void (CODEGEN_FUNCPTR *glGetDoublei_v)(GLenum target, GLuint index, GLdouble * data);
		extern void (CODEGEN_FUNCPTR *glGetFloati_v)(GLenum target, GLuint index, GLfloat * data);
		extern void (CODEGEN_FUNCPTR *glScissorArrayv)(GLuint first, GLsizei count, const GLint * v);
		extern void (CODEGEN_FUNCPTR *glScissorIndexed)(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *glScissorIndexedv)(GLuint index, const GLint * v);
		extern void (CODEGEN_FUNCPTR *glViewportArrayv)(GLuint first, GLsizei count, const GLfloat * v);
		extern void (CODEGEN_FUNCPTR *glViewportIndexedf)(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
		extern void (CODEGEN_FUNCPTR *glViewportIndexedfv)(GLuint index, const GLfloat * v);
		
		
		extern void (CODEGEN_FUNCPTR *glClearBufferData)(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void * data);
		extern void (CODEGEN_FUNCPTR *glClearBufferSubData)(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void * data);
		
		extern void (CODEGEN_FUNCPTR *glCopyImageSubData)(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
		
		
		
		extern void (CODEGEN_FUNCPTR *glFramebufferParameteri)(GLenum target, GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *glGetFramebufferParameteriv)(GLenum target, GLenum pname, GLint * params);
		
		extern void (CODEGEN_FUNCPTR *glInvalidateBufferData)(GLuint buffer);
		extern void (CODEGEN_FUNCPTR *glInvalidateBufferSubData)(GLuint buffer, GLintptr offset, GLsizeiptr length);
		extern void (CODEGEN_FUNCPTR *glInvalidateFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum * attachments);
		extern void (CODEGEN_FUNCPTR *glInvalidateSubFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum * attachments, GLint x, GLint y, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *glInvalidateTexImage)(GLuint texture, GLint level);
		extern void (CODEGEN_FUNCPTR *glInvalidateTexSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth);
		
		
		
		
		extern void (CODEGEN_FUNCPTR *glTexStorage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
		extern void (CODEGEN_FUNCPTR *glTexStorage3DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
		
		extern void (CODEGEN_FUNCPTR *glDebugMessageCallback)(GLDEBUGPROC callback, const void * userParam);
		extern void (CODEGEN_FUNCPTR *glDebugMessageControl)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint * ids, GLboolean enabled);
		extern void (CODEGEN_FUNCPTR *glDebugMessageInsert)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * buf);
		extern GLuint (CODEGEN_FUNCPTR *glGetDebugMessageLog)(GLuint count, GLsizei bufSize, GLenum * sources, GLenum * types, GLuint * ids, GLenum * severities, GLsizei * lengths, GLchar * messageLog);
		extern void (CODEGEN_FUNCPTR *glGetObjectLabel)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei * length, GLchar * label);
		extern void (CODEGEN_FUNCPTR *glGetObjectPtrLabel)(const void * ptr, GLsizei bufSize, GLsizei * length, GLchar * label);
		extern void (CODEGEN_FUNCPTR *glGetPointerv)(GLenum pname, void ** params);
		extern void (CODEGEN_FUNCPTR *glObjectLabel)(GLenum identifier, GLuint name, GLsizei length, const GLchar * label);
		extern void (CODEGEN_FUNCPTR *glObjectPtrLabel)(const void * ptr, GLsizei length, const GLchar * label);
		extern void (CODEGEN_FUNCPTR *glPopDebugGroup)(void);
		extern void (CODEGEN_FUNCPTR *glPushDebugGroup)(GLenum source, GLuint id, GLsizei length, const GLchar * message);
		
		extern void (CODEGEN_FUNCPTR *glBufferStorage)(GLenum target, GLsizeiptr size, const void * data, GLbitfield flags);
		
		extern void (CODEGEN_FUNCPTR *glClearTexImage)(GLuint texture, GLint level, GLenum format, GLenum type, const void * data);
		extern void (CODEGEN_FUNCPTR *glClearTexSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void * data);
		
		
		extern void (CODEGEN_FUNCPTR *glBindBuffersBase)(GLenum target, GLuint first, GLsizei count, const GLuint * buffers);
		extern void (CODEGEN_FUNCPTR *glBindBuffersRange)(GLenum target, GLuint first, GLsizei count, const GLuint * buffers, const GLintptr * offsets, const GLsizeiptr * sizes);
		extern void (CODEGEN_FUNCPTR *glBindImageTextures)(GLuint first, GLsizei count, const GLuint * textures);
		extern void (CODEGEN_FUNCPTR *glBindSamplers)(GLuint first, GLsizei count, const GLuint * samplers);
		extern void (CODEGEN_FUNCPTR *glBindTextures)(GLuint first, GLsizei count, const GLuint * textures);
		extern void (CODEGEN_FUNCPTR *glBindVertexBuffers)(GLuint first, GLsizei count, const GLuint * buffers, const GLintptr * offsets, const GLsizei * strides);
		
		
		
		
		
		
		extern void (CODEGEN_FUNCPTR *glClipControl)(GLenum origin, GLenum depth);
		
		
		
		
		extern void (CODEGEN_FUNCPTR *glBindTextureUnit)(GLuint unit, GLuint texture);
		extern void (CODEGEN_FUNCPTR *glBlitNamedFramebuffer)(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
		extern GLenum (CODEGEN_FUNCPTR *glCheckNamedFramebufferStatus)(GLuint framebuffer, GLenum target);
		extern void (CODEGEN_FUNCPTR *glClearNamedBufferData)(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void * data);
		extern void (CODEGEN_FUNCPTR *glClearNamedBufferSubData)(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void * data);
		extern void (CODEGEN_FUNCPTR *glClearNamedFramebufferfi)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat depth, GLint stencil);
		extern void (CODEGEN_FUNCPTR *glClearNamedFramebufferfv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glClearNamedFramebufferiv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint * value);
		extern void (CODEGEN_FUNCPTR *glClearNamedFramebufferuiv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint * value);
		extern void (CODEGEN_FUNCPTR *glCompressedTextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *glCompressedTextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *glCompressedTextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *glCopyNamedBufferSubData)(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
		extern void (CODEGEN_FUNCPTR *glCopyTextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
		extern void (CODEGEN_FUNCPTR *glCopyTextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *glCopyTextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *glCreateBuffers)(GLsizei n, GLuint * buffers);
		extern void (CODEGEN_FUNCPTR *glCreateFramebuffers)(GLsizei n, GLuint * framebuffers);
		extern void (CODEGEN_FUNCPTR *glCreateProgramPipelines)(GLsizei n, GLuint * pipelines);
		extern void (CODEGEN_FUNCPTR *glCreateQueries)(GLenum target, GLsizei n, GLuint * ids);
		extern void (CODEGEN_FUNCPTR *glCreateRenderbuffers)(GLsizei n, GLuint * renderbuffers);
		extern void (CODEGEN_FUNCPTR *glCreateSamplers)(GLsizei n, GLuint * samplers);
		extern void (CODEGEN_FUNCPTR *glCreateTextures)(GLenum target, GLsizei n, GLuint * textures);
		extern void (CODEGEN_FUNCPTR *glCreateTransformFeedbacks)(GLsizei n, GLuint * ids);
		extern void (CODEGEN_FUNCPTR *glCreateVertexArrays)(GLsizei n, GLuint * arrays);
		extern void (CODEGEN_FUNCPTR *glDisableVertexArrayAttrib)(GLuint vaobj, GLuint index);
		extern void (CODEGEN_FUNCPTR *glEnableVertexArrayAttrib)(GLuint vaobj, GLuint index);
		extern void (CODEGEN_FUNCPTR *glFlushMappedNamedBufferRange)(GLuint buffer, GLintptr offset, GLsizeiptr length);
		extern void (CODEGEN_FUNCPTR *glGenerateTextureMipmap)(GLuint texture);
		extern void (CODEGEN_FUNCPTR *glGetCompressedTextureImage)(GLuint texture, GLint level, GLsizei bufSize, void * pixels);
		extern void (CODEGEN_FUNCPTR *glGetNamedBufferParameteri64v)(GLuint buffer, GLenum pname, GLint64 * params);
		extern void (CODEGEN_FUNCPTR *glGetNamedBufferParameteriv)(GLuint buffer, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetNamedBufferPointerv)(GLuint buffer, GLenum pname, void ** params);
		extern void (CODEGEN_FUNCPTR *glGetNamedBufferSubData)(GLuint buffer, GLintptr offset, GLsizeiptr size, void * data);
		extern void (CODEGEN_FUNCPTR *glGetNamedFramebufferAttachmentParameteriv)(GLuint framebuffer, GLenum attachment, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetNamedFramebufferParameteriv)(GLuint framebuffer, GLenum pname, GLint * param);
		extern void (CODEGEN_FUNCPTR *glGetNamedRenderbufferParameteriv)(GLuint renderbuffer, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetQueryBufferObjecti64v)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
		extern void (CODEGEN_FUNCPTR *glGetQueryBufferObjectiv)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
		extern void (CODEGEN_FUNCPTR *glGetQueryBufferObjectui64v)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
		extern void (CODEGEN_FUNCPTR *glGetQueryBufferObjectuiv)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
		extern void (CODEGEN_FUNCPTR *glGetTextureImage)(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void * pixels);
		extern void (CODEGEN_FUNCPTR *glGetTextureLevelParameterfv)(GLuint texture, GLint level, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glGetTextureLevelParameteriv)(GLuint texture, GLint level, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetTextureParameterIiv)(GLuint texture, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetTextureParameterIuiv)(GLuint texture, GLenum pname, GLuint * params);
		extern void (CODEGEN_FUNCPTR *glGetTextureParameterfv)(GLuint texture, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glGetTextureParameteriv)(GLuint texture, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetTransformFeedbacki64_v)(GLuint xfb, GLenum pname, GLuint index, GLint64 * param);
		extern void (CODEGEN_FUNCPTR *glGetTransformFeedbacki_v)(GLuint xfb, GLenum pname, GLuint index, GLint * param);
		extern void (CODEGEN_FUNCPTR *glGetTransformFeedbackiv)(GLuint xfb, GLenum pname, GLint * param);
		extern void (CODEGEN_FUNCPTR *glGetVertexArrayIndexed64iv)(GLuint vaobj, GLuint index, GLenum pname, GLint64 * param);
		extern void (CODEGEN_FUNCPTR *glGetVertexArrayIndexediv)(GLuint vaobj, GLuint index, GLenum pname, GLint * param);
		extern void (CODEGEN_FUNCPTR *glGetVertexArrayiv)(GLuint vaobj, GLenum pname, GLint * param);
		extern void (CODEGEN_FUNCPTR *glInvalidateNamedFramebufferData)(GLuint framebuffer, GLsizei numAttachments, const GLenum * attachments);
		extern void (CODEGEN_FUNCPTR *glInvalidateNamedFramebufferSubData)(GLuint framebuffer, GLsizei numAttachments, const GLenum * attachments, GLint x, GLint y, GLsizei width, GLsizei height);
		extern void * (CODEGEN_FUNCPTR *glMapNamedBuffer)(GLuint buffer, GLenum access);
		extern void * (CODEGEN_FUNCPTR *glMapNamedBufferRange)(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
		extern void (CODEGEN_FUNCPTR *glNamedBufferData)(GLuint buffer, GLsizeiptr size, const void * data, GLenum usage);
		extern void (CODEGEN_FUNCPTR *glNamedBufferStorage)(GLuint buffer, GLsizeiptr size, const void * data, GLbitfield flags);
		extern void (CODEGEN_FUNCPTR *glNamedBufferSubData)(GLuint buffer, GLintptr offset, GLsizeiptr size, const void * data);
		extern void (CODEGEN_FUNCPTR *glNamedFramebufferDrawBuffer)(GLuint framebuffer, GLenum buf);
		extern void (CODEGEN_FUNCPTR *glNamedFramebufferDrawBuffers)(GLuint framebuffer, GLsizei n, const GLenum * bufs);
		extern void (CODEGEN_FUNCPTR *glNamedFramebufferParameteri)(GLuint framebuffer, GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *glNamedFramebufferReadBuffer)(GLuint framebuffer, GLenum src);
		extern void (CODEGEN_FUNCPTR *glNamedFramebufferRenderbuffer)(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
		extern void (CODEGEN_FUNCPTR *glNamedFramebufferTexture)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
		extern void (CODEGEN_FUNCPTR *glNamedFramebufferTextureLayer)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
		extern void (CODEGEN_FUNCPTR *glNamedRenderbufferStorage)(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *glNamedRenderbufferStorageMultisample)(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *glTextureBuffer)(GLuint texture, GLenum internalformat, GLuint buffer);
		extern void (CODEGEN_FUNCPTR *glTextureBufferRange)(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
		extern void (CODEGEN_FUNCPTR *glTextureParameterIiv)(GLuint texture, GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *glTextureParameterIuiv)(GLuint texture, GLenum pname, const GLuint * params);
		extern void (CODEGEN_FUNCPTR *glTextureParameterf)(GLuint texture, GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *glTextureParameterfv)(GLuint texture, GLenum pname, const GLfloat * param);
		extern void (CODEGEN_FUNCPTR *glTextureParameteri)(GLuint texture, GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *glTextureParameteriv)(GLuint texture, GLenum pname, const GLint * param);
		extern void (CODEGEN_FUNCPTR *glTextureStorage1D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width);
		extern void (CODEGEN_FUNCPTR *glTextureStorage2D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *glTextureStorage2DMultisample)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
		extern void (CODEGEN_FUNCPTR *glTextureStorage3D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
		extern void (CODEGEN_FUNCPTR *glTextureStorage3DMultisample)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
		extern void (CODEGEN_FUNCPTR *glTextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *glTextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *glTextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *glTransformFeedbackBufferBase)(GLuint xfb, GLuint index, GLuint buffer);
		extern void (CODEGEN_FUNCPTR *glTransformFeedbackBufferRange)(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
		extern GLboolean (CODEGEN_FUNCPTR *glUnmapNamedBuffer)(GLuint buffer);
		extern void (CODEGEN_FUNCPTR *glVertexArrayAttribBinding)(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
		extern void (CODEGEN_FUNCPTR *glVertexArrayAttribFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
		extern void (CODEGEN_FUNCPTR *glVertexArrayAttribIFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
		extern void (CODEGEN_FUNCPTR *glVertexArrayAttribLFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
		extern void (CODEGEN_FUNCPTR *glVertexArrayBindingDivisor)(GLuint vaobj, GLuint bindingindex, GLuint divisor);
		extern void (CODEGEN_FUNCPTR *glVertexArrayElementBuffer)(GLuint vaobj, GLuint buffer);
		extern void (CODEGEN_FUNCPTR *glVertexArrayVertexBuffer)(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
		extern void (CODEGEN_FUNCPTR *glVertexArrayVertexBuffers)(GLuint vaobj, GLuint first, GLsizei count, const GLuint * buffers, const GLintptr * offsets, const GLsizei * strides);
		
		extern void (CODEGEN_FUNCPTR *glGetCompressedTextureSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void * pixels);
		extern void (CODEGEN_FUNCPTR *glGetTextureSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void * pixels);
		
		
		extern void (CODEGEN_FUNCPTR *glTextureBarrier)(void);
		
		
		
		extern GLenum (CODEGEN_FUNCPTR *glGetGraphicsResetStatus)(void);
		extern void (CODEGEN_FUNCPTR *glGetnUniformfv)(GLuint program, GLint location, GLsizei bufSize, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glGetnUniformiv)(GLuint program, GLint location, GLsizei bufSize, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetnUniformuiv)(GLuint program, GLint location, GLsizei bufSize, GLuint * params);
		extern void (CODEGEN_FUNCPTR *glReadnPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void * data);
		
		extern void (CODEGEN_FUNCPTR *glBlendFunc)(GLenum sfactor, GLenum dfactor);
		extern void (CODEGEN_FUNCPTR *glClear)(GLbitfield mask);
		extern void (CODEGEN_FUNCPTR *glClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		extern void (CODEGEN_FUNCPTR *glClearDepth)(GLdouble depth);
		extern void (CODEGEN_FUNCPTR *glClearStencil)(GLint s);
		extern void (CODEGEN_FUNCPTR *glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
		extern void (CODEGEN_FUNCPTR *glCullFace)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *glDepthFunc)(GLenum func);
		extern void (CODEGEN_FUNCPTR *glDepthMask)(GLboolean flag);
		extern void (CODEGEN_FUNCPTR *glDepthRange)(GLdouble ren_near, GLdouble ren_far);
		extern void (CODEGEN_FUNCPTR *glDisable)(GLenum cap);
		extern void (CODEGEN_FUNCPTR *glDrawBuffer)(GLenum buf);
		extern void (CODEGEN_FUNCPTR *glEnable)(GLenum cap);
		extern void (CODEGEN_FUNCPTR *glFinish)(void);
		extern void (CODEGEN_FUNCPTR *glFlush)(void);
		extern void (CODEGEN_FUNCPTR *glFrontFace)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *glGetBooleanv)(GLenum pname, GLboolean * data);
		extern void (CODEGEN_FUNCPTR *glGetDoublev)(GLenum pname, GLdouble * data);
		extern GLenum (CODEGEN_FUNCPTR *glGetError)(void);
		extern void (CODEGEN_FUNCPTR *glGetFloatv)(GLenum pname, GLfloat * data);
		extern void (CODEGEN_FUNCPTR *glGetIntegerv)(GLenum pname, GLint * data);
		extern const GLubyte * (CODEGEN_FUNCPTR *glGetString)(GLenum name);
		extern void (CODEGEN_FUNCPTR *glGetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, void * pixels);
		extern void (CODEGEN_FUNCPTR *glGetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glGetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glGetTexParameteriv)(GLenum target, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glHint)(GLenum target, GLenum mode);
		extern GLboolean (CODEGEN_FUNCPTR *glIsEnabled)(GLenum cap);
		extern void (CODEGEN_FUNCPTR *glLineWidth)(GLfloat width);
		extern void (CODEGEN_FUNCPTR *glLogicOp)(GLenum opcode);
		extern void (CODEGEN_FUNCPTR *glPixelStoref)(GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *glPixelStorei)(GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *glPointSize)(GLfloat size);
		extern void (CODEGEN_FUNCPTR *glPolygonMode)(GLenum face, GLenum mode);
		extern void (CODEGEN_FUNCPTR *glReadBuffer)(GLenum src);
		extern void (CODEGEN_FUNCPTR *glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels);
		extern void (CODEGEN_FUNCPTR *glScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *glStencilFunc)(GLenum func, GLint ref, GLuint mask);
		extern void (CODEGEN_FUNCPTR *glStencilMask)(GLuint mask);
		extern void (CODEGEN_FUNCPTR *glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
		extern void (CODEGEN_FUNCPTR *glTexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *glTexParameterf)(GLenum target, GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *glTexParameterfv)(GLenum target, GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glTexParameteri)(GLenum target, GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *glTexParameteriv)(GLenum target, GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
		
		extern void (CODEGEN_FUNCPTR *glBindTexture)(GLenum target, GLuint texture);
		extern void (CODEGEN_FUNCPTR *glCopyTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
		extern void (CODEGEN_FUNCPTR *glCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
		extern void (CODEGEN_FUNCPTR *glCopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
		extern void (CODEGEN_FUNCPTR *glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *glDeleteTextures)(GLsizei n, const GLuint * textures);
		extern void (CODEGEN_FUNCPTR *glDrawArrays)(GLenum mode, GLint first, GLsizei count);
		extern void (CODEGEN_FUNCPTR *glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void * indices);
		extern void (CODEGEN_FUNCPTR *glGenTextures)(GLsizei n, GLuint * textures);
		extern GLboolean (CODEGEN_FUNCPTR *glIsTexture)(GLuint texture);
		extern void (CODEGEN_FUNCPTR *glPolygonOffset)(GLfloat factor, GLfloat units);
		extern void (CODEGEN_FUNCPTR *glTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels);
		
		extern void (CODEGEN_FUNCPTR *glCopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *glDrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void * indices);
		extern void (CODEGEN_FUNCPTR *glTexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void * pixels);
		extern void (CODEGEN_FUNCPTR *glTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void * pixels);
		
		extern void (CODEGEN_FUNCPTR *glActiveTexture)(GLenum texture);
		extern void (CODEGEN_FUNCPTR *glCompressedTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *glCompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *glCompressedTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *glCompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void * data);
		extern void (CODEGEN_FUNCPTR *glGetCompressedTexImage)(GLenum target, GLint level, void * img);
		extern void (CODEGEN_FUNCPTR *glSampleCoverage)(GLfloat value, GLboolean invert);
		
		extern void (CODEGEN_FUNCPTR *glBlendColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
		extern void (CODEGEN_FUNCPTR *glBlendEquation)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *glBlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
		extern void (CODEGEN_FUNCPTR *glMultiDrawArrays)(GLenum mode, const GLint * first, const GLsizei * count, GLsizei drawcount);
		extern void (CODEGEN_FUNCPTR *glMultiDrawElements)(GLenum mode, const GLsizei * count, GLenum type, const void *const* indices, GLsizei drawcount);
		extern void (CODEGEN_FUNCPTR *glPointParameterf)(GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *glPointParameterfv)(GLenum pname, const GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glPointParameteri)(GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *glPointParameteriv)(GLenum pname, const GLint * params);
		
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
		
		extern void (CODEGEN_FUNCPTR *glBeginConditionalRender)(GLuint id, GLenum mode);
		extern void (CODEGEN_FUNCPTR *glBeginTransformFeedback)(GLenum primitiveMode);
		extern void (CODEGEN_FUNCPTR *glBindBufferBase)(GLenum target, GLuint index, GLuint buffer);
		extern void (CODEGEN_FUNCPTR *glBindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
		extern void (CODEGEN_FUNCPTR *glBindFragDataLocation)(GLuint program, GLuint color, const GLchar * name);
		extern void (CODEGEN_FUNCPTR *glBindFramebuffer)(GLenum target, GLuint framebuffer);
		extern void (CODEGEN_FUNCPTR *glBindRenderbuffer)(GLenum target, GLuint renderbuffer);
		extern void (CODEGEN_FUNCPTR *glBindVertexArray)(GLuint ren_array);
		extern void (CODEGEN_FUNCPTR *glBlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
		extern GLenum (CODEGEN_FUNCPTR *glCheckFramebufferStatus)(GLenum target);
		extern void (CODEGEN_FUNCPTR *glClampColor)(GLenum target, GLenum clamp);
		extern void (CODEGEN_FUNCPTR *glClearBufferfi)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
		extern void (CODEGEN_FUNCPTR *glClearBufferfv)(GLenum buffer, GLint drawbuffer, const GLfloat * value);
		extern void (CODEGEN_FUNCPTR *glClearBufferiv)(GLenum buffer, GLint drawbuffer, const GLint * value);
		extern void (CODEGEN_FUNCPTR *glClearBufferuiv)(GLenum buffer, GLint drawbuffer, const GLuint * value);
		extern void (CODEGEN_FUNCPTR *glColorMaski)(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
		extern void (CODEGEN_FUNCPTR *glDeleteFramebuffers)(GLsizei n, const GLuint * framebuffers);
		extern void (CODEGEN_FUNCPTR *glDeleteRenderbuffers)(GLsizei n, const GLuint * renderbuffers);
		extern void (CODEGEN_FUNCPTR *glDeleteVertexArrays)(GLsizei n, const GLuint * arrays);
		extern void (CODEGEN_FUNCPTR *glDisablei)(GLenum target, GLuint index);
		extern void (CODEGEN_FUNCPTR *glEnablei)(GLenum target, GLuint index);
		extern void (CODEGEN_FUNCPTR *glEndConditionalRender)(void);
		extern void (CODEGEN_FUNCPTR *glEndTransformFeedback)(void);
		extern void (CODEGEN_FUNCPTR *glFlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length);
		extern void (CODEGEN_FUNCPTR *glFramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
		extern void (CODEGEN_FUNCPTR *glFramebufferTexture1D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
		extern void (CODEGEN_FUNCPTR *glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
		extern void (CODEGEN_FUNCPTR *glFramebufferTexture3D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
		extern void (CODEGEN_FUNCPTR *glFramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
		extern void (CODEGEN_FUNCPTR *glGenFramebuffers)(GLsizei n, GLuint * framebuffers);
		extern void (CODEGEN_FUNCPTR *glGenRenderbuffers)(GLsizei n, GLuint * renderbuffers);
		extern void (CODEGEN_FUNCPTR *glGenVertexArrays)(GLsizei n, GLuint * arrays);
		extern void (CODEGEN_FUNCPTR *glGenerateMipmap)(GLenum target);
		extern void (CODEGEN_FUNCPTR *glGetBooleani_v)(GLenum target, GLuint index, GLboolean * data);
		extern GLint (CODEGEN_FUNCPTR *glGetFragDataLocation)(GLuint program, const GLchar * name);
		extern void (CODEGEN_FUNCPTR *glGetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetIntegeri_v)(GLenum target, GLuint index, GLint * data);
		extern void (CODEGEN_FUNCPTR *glGetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint * params);
		extern const GLubyte * (CODEGEN_FUNCPTR *glGetStringi)(GLenum name, GLuint index);
		extern void (CODEGEN_FUNCPTR *glGetTexParameterIiv)(GLenum target, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetTexParameterIuiv)(GLenum target, GLenum pname, GLuint * params);
		extern void (CODEGEN_FUNCPTR *glGetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLsizei * size, GLenum * type, GLchar * name);
		extern void (CODEGEN_FUNCPTR *glGetUniformuiv)(GLuint program, GLint location, GLuint * params);
		extern void (CODEGEN_FUNCPTR *glGetVertexAttribIiv)(GLuint index, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint * params);
		extern GLboolean (CODEGEN_FUNCPTR *glIsEnabledi)(GLenum target, GLuint index);
		extern GLboolean (CODEGEN_FUNCPTR *glIsFramebuffer)(GLuint framebuffer);
		extern GLboolean (CODEGEN_FUNCPTR *glIsRenderbuffer)(GLuint renderbuffer);
		extern GLboolean (CODEGEN_FUNCPTR *glIsVertexArray)(GLuint ren_array);
		extern void * (CODEGEN_FUNCPTR *glMapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
		extern void (CODEGEN_FUNCPTR *glRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *glRenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
		extern void (CODEGEN_FUNCPTR *glTexParameterIiv)(GLenum target, GLenum pname, const GLint * params);
		extern void (CODEGEN_FUNCPTR *glTexParameterIuiv)(GLenum target, GLenum pname, const GLuint * params);
		extern void (CODEGEN_FUNCPTR *glTransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar *const* varyings, GLenum bufferMode);
		extern void (CODEGEN_FUNCPTR *glUniform1ui)(GLint location, GLuint v0);
		extern void (CODEGEN_FUNCPTR *glUniform1uiv)(GLint location, GLsizei count, const GLuint * value);
		extern void (CODEGEN_FUNCPTR *glUniform2ui)(GLint location, GLuint v0, GLuint v1);
		extern void (CODEGEN_FUNCPTR *glUniform2uiv)(GLint location, GLsizei count, const GLuint * value);
		extern void (CODEGEN_FUNCPTR *glUniform3ui)(GLint location, GLuint v0, GLuint v1, GLuint v2);
		extern void (CODEGEN_FUNCPTR *glUniform3uiv)(GLint location, GLsizei count, const GLuint * value);
		extern void (CODEGEN_FUNCPTR *glUniform4ui)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
		extern void (CODEGEN_FUNCPTR *glUniform4uiv)(GLint location, GLsizei count, const GLuint * value);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI1i)(GLuint index, GLint x);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI1iv)(GLuint index, const GLint * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI1ui)(GLuint index, GLuint x);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI1uiv)(GLuint index, const GLuint * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI2i)(GLuint index, GLint x, GLint y);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI2iv)(GLuint index, const GLint * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI2ui)(GLuint index, GLuint x, GLuint y);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI2uiv)(GLuint index, const GLuint * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI3i)(GLuint index, GLint x, GLint y, GLint z);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI3iv)(GLuint index, const GLint * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI3ui)(GLuint index, GLuint x, GLuint y, GLuint z);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI3uiv)(GLuint index, const GLuint * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI4bv)(GLuint index, const GLbyte * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI4i)(GLuint index, GLint x, GLint y, GLint z, GLint w);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI4iv)(GLuint index, const GLint * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI4sv)(GLuint index, const GLshort * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI4ubv)(GLuint index, const GLubyte * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI4ui)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI4uiv)(GLuint index, const GLuint * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttribI4usv)(GLuint index, const GLushort * v);
		extern void (CODEGEN_FUNCPTR *glVertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const void * pointer);
		
		extern void (CODEGEN_FUNCPTR *glCopyBufferSubData)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
		extern void (CODEGEN_FUNCPTR *glDrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
		extern void (CODEGEN_FUNCPTR *glDrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const void * indices, GLsizei instancecount);
		extern void (CODEGEN_FUNCPTR *glGetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformBlockName);
		extern void (CODEGEN_FUNCPTR *glGetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetActiveUniformName)(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformName);
		extern void (CODEGEN_FUNCPTR *glGetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint * uniformIndices, GLenum pname, GLint * params);
		extern GLuint (CODEGEN_FUNCPTR *glGetUniformBlockIndex)(GLuint program, const GLchar * uniformBlockName);
		extern void (CODEGEN_FUNCPTR *glGetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar *const* uniformNames, GLuint * uniformIndices);
		extern void (CODEGEN_FUNCPTR *glPrimitiveRestartIndex)(GLuint index);
		extern void (CODEGEN_FUNCPTR *glTexBuffer)(GLenum target, GLenum internalformat, GLuint buffer);
		extern void (CODEGEN_FUNCPTR *glUniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
		
		extern GLenum (CODEGEN_FUNCPTR *glClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
		extern void (CODEGEN_FUNCPTR *glDeleteSync)(GLsync sync);
		extern void (CODEGEN_FUNCPTR *glDrawElementsBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void * indices, GLint basevertex);
		extern void (CODEGEN_FUNCPTR *glDrawElementsInstancedBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void * indices, GLsizei instancecount, GLint basevertex);
		extern void (CODEGEN_FUNCPTR *glDrawRangeElementsBaseVertex)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void * indices, GLint basevertex);
		extern GLsync (CODEGEN_FUNCPTR *glFenceSync)(GLenum condition, GLbitfield flags);
		extern void (CODEGEN_FUNCPTR *glFramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level);
		extern void (CODEGEN_FUNCPTR *glGetBufferParameteri64v)(GLenum target, GLenum pname, GLint64 * params);
		extern void (CODEGEN_FUNCPTR *glGetInteger64i_v)(GLenum target, GLuint index, GLint64 * data);
		extern void (CODEGEN_FUNCPTR *glGetInteger64v)(GLenum pname, GLint64 * data);
		extern void (CODEGEN_FUNCPTR *glGetMultisamplefv)(GLenum pname, GLuint index, GLfloat * val);
		extern void (CODEGEN_FUNCPTR *glGetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * values);
		extern GLboolean (CODEGEN_FUNCPTR *glIsSync)(GLsync sync);
		extern void (CODEGEN_FUNCPTR *glMultiDrawElementsBaseVertex)(GLenum mode, const GLsizei * count, GLenum type, const void *const* indices, GLsizei drawcount, const GLint * basevertex);
		extern void (CODEGEN_FUNCPTR *glProvokingVertex)(GLenum mode);
		extern void (CODEGEN_FUNCPTR *glSampleMaski)(GLuint maskNumber, GLbitfield mask);
		extern void (CODEGEN_FUNCPTR *glTexImage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
		extern void (CODEGEN_FUNCPTR *glTexImage3DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
		extern void (CODEGEN_FUNCPTR *glWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
		
		extern void (CODEGEN_FUNCPTR *glBindFragDataLocationIndexed)(GLuint program, GLuint colorNumber, GLuint index, const GLchar * name);
		extern void (CODEGEN_FUNCPTR *glBindSampler)(GLuint unit, GLuint sampler);
		extern void (CODEGEN_FUNCPTR *glDeleteSamplers)(GLsizei count, const GLuint * samplers);
		extern void (CODEGEN_FUNCPTR *glGenSamplers)(GLsizei count, GLuint * samplers);
		extern GLint (CODEGEN_FUNCPTR *glGetFragDataIndex)(GLuint program, const GLchar * name);
		extern void (CODEGEN_FUNCPTR *glGetQueryObjecti64v)(GLuint id, GLenum pname, GLint64 * params);
		extern void (CODEGEN_FUNCPTR *glGetQueryObjectui64v)(GLuint id, GLenum pname, GLuint64 * params);
		extern void (CODEGEN_FUNCPTR *glGetSamplerParameterIiv)(GLuint sampler, GLenum pname, GLint * params);
		extern void (CODEGEN_FUNCPTR *glGetSamplerParameterIuiv)(GLuint sampler, GLenum pname, GLuint * params);
		extern void (CODEGEN_FUNCPTR *glGetSamplerParameterfv)(GLuint sampler, GLenum pname, GLfloat * params);
		extern void (CODEGEN_FUNCPTR *glGetSamplerParameteriv)(GLuint sampler, GLenum pname, GLint * params);
		extern GLboolean (CODEGEN_FUNCPTR *glIsSampler)(GLuint sampler);
		extern void (CODEGEN_FUNCPTR *glQueryCounter)(GLuint id, GLenum target);
		extern void (CODEGEN_FUNCPTR *glSamplerParameterIiv)(GLuint sampler, GLenum pname, const GLint * param);
		extern void (CODEGEN_FUNCPTR *glSamplerParameterIuiv)(GLuint sampler, GLenum pname, const GLuint * param);
		extern void (CODEGEN_FUNCPTR *glSamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param);
		extern void (CODEGEN_FUNCPTR *glSamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat * param);
		extern void (CODEGEN_FUNCPTR *glSamplerParameteri)(GLuint sampler, GLenum pname, GLint param);
		extern void (CODEGEN_FUNCPTR *glSamplerParameteriv)(GLuint sampler, GLenum pname, const GLint * param);
		extern void (CODEGEN_FUNCPTR *glVertexAttribDivisor)(GLuint index, GLuint divisor);
		extern void (CODEGEN_FUNCPTR *glVertexAttribP1ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
		extern void (CODEGEN_FUNCPTR *glVertexAttribP1uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint * value);
		extern void (CODEGEN_FUNCPTR *glVertexAttribP2ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
		extern void (CODEGEN_FUNCPTR *glVertexAttribP2uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint * value);
		extern void (CODEGEN_FUNCPTR *glVertexAttribP3ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
		extern void (CODEGEN_FUNCPTR *glVertexAttribP3uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint * value);
		extern void (CODEGEN_FUNCPTR *glVertexAttribP4ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
		extern void (CODEGEN_FUNCPTR *glVertexAttribP4uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint * value);
		
		namespace sys
		{
			
			exts::LoadTest LoadFunctions();
			
			int GetMinorVersion();
			int GetMajorVersion();
			bool IsVersionGEQ(int majorVersion, int minorVersion);
			
		} //namespace sys
	} //namespace gl
} //namespace gl3x
#endif //GL3X_PIONEER_GENERATED_HEADEROPENGL_HPP
