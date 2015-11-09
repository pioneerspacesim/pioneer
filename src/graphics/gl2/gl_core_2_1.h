#ifndef POINTER_C_GENERATED_HEADER_OPENGL_H
#define POINTER_C_GENERATED_HEADER_OPENGL_H

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

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

extern int ogl_ext_EXT_texture_compression_s3tc;
extern int ogl_ext_EXT_texture_sRGB;
extern int ogl_ext_EXT_texture_filter_anisotropic;

#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0

#define GL_COMPRESSED_SLUMINANCE_ALPHA_EXT 0x8C4B
#define GL_COMPRESSED_SLUMINANCE_EXT 0x8C4A
#define GL_COMPRESSED_SRGB_ALPHA_EXT 0x8C49
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
#define GL_COMPRESSED_SRGB_EXT 0x8C48
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT 0x8C4C
#define GL_SLUMINANCE8_ALPHA8_EXT 0x8C45
#define GL_SLUMINANCE8_EXT 0x8C47
#define GL_SLUMINANCE_ALPHA_EXT 0x8C44
#define GL_SLUMINANCE_EXT 0x8C46
#define GL_SRGB8_ALPHA8_EXT 0x8C43
#define GL_SRGB8_EXT 0x8C41
#define GL_SRGB_ALPHA_EXT 0x8C42
#define GL_SRGB_EXT 0x8C40

#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE

#define GL_2D 0x0600
#define GL_2_BYTES 0x1407
#define GL_3D 0x0601
#define GL_3D_COLOR 0x0602
#define GL_3D_COLOR_TEXTURE 0x0603
#define GL_3_BYTES 0x1408
#define GL_4D_COLOR_TEXTURE 0x0604
#define GL_4_BYTES 0x1409
#define GL_ACCUM 0x0100
#define GL_ACCUM_ALPHA_BITS 0x0D5B
#define GL_ACCUM_BLUE_BITS 0x0D5A
#define GL_ACCUM_BUFFER_BIT 0x00000200
#define GL_ACCUM_CLEAR_VALUE 0x0B80
#define GL_ACCUM_GREEN_BITS 0x0D59
#define GL_ACCUM_RED_BITS 0x0D58
#define GL_ADD 0x0104
#define GL_ALL_ATTRIB_BITS 0xFFFFFFFF
#define GL_ALPHA 0x1906
#define GL_ALPHA12 0x803D
#define GL_ALPHA16 0x803E
#define GL_ALPHA4 0x803B
#define GL_ALPHA8 0x803C
#define GL_ALPHA_BIAS 0x0D1D
#define GL_ALPHA_BITS 0x0D55
#define GL_ALPHA_SCALE 0x0D1C
#define GL_ALPHA_TEST 0x0BC0
#define GL_ALPHA_TEST_FUNC 0x0BC1
#define GL_ALPHA_TEST_REF 0x0BC2
#define GL_ALWAYS 0x0207
#define GL_AMBIENT 0x1200
#define GL_AMBIENT_AND_DIFFUSE 0x1602
#define GL_AND 0x1501
#define GL_AND_INVERTED 0x1504
#define GL_AND_REVERSE 0x1502
#define GL_ATTRIB_STACK_DEPTH 0x0BB0
#define GL_AUTO_NORMAL 0x0D80
#define GL_AUX0 0x0409
#define GL_AUX1 0x040A
#define GL_AUX2 0x040B
#define GL_AUX3 0x040C
#define GL_AUX_BUFFERS 0x0C00
#define GL_BACK 0x0405
#define GL_BACK_LEFT 0x0402
#define GL_BACK_RIGHT 0x0403
#define GL_BITMAP 0x1A00
#define GL_BITMAP_TOKEN 0x0704
#define GL_BLEND 0x0BE2
#define GL_BLEND_DST 0x0BE0
#define GL_BLEND_SRC 0x0BE1
#define GL_BLUE 0x1905
#define GL_BLUE_BIAS 0x0D1B
#define GL_BLUE_BITS 0x0D54
#define GL_BLUE_SCALE 0x0D1A
#define GL_BYTE 0x1400
#define GL_C3F_V3F 0x2A24
#define GL_C4F_N3F_V3F 0x2A26
#define GL_C4UB_V2F 0x2A22
#define GL_C4UB_V3F 0x2A23
#define GL_CCW 0x0901
#define GL_CLAMP 0x2900
#define GL_CLEAR 0x1500
#define GL_CLIENT_ALL_ATTRIB_BITS 0xFFFFFFFF
#define GL_CLIENT_ATTRIB_STACK_DEPTH 0x0BB1
#define GL_CLIENT_PIXEL_STORE_BIT 0x00000001
#define GL_CLIENT_VERTEX_ARRAY_BIT 0x00000002
#define GL_CLIP_PLANE0 0x3000
#define GL_CLIP_PLANE1 0x3001
#define GL_CLIP_PLANE2 0x3002
#define GL_CLIP_PLANE3 0x3003
#define GL_CLIP_PLANE4 0x3004
#define GL_CLIP_PLANE5 0x3005
#define GL_COEFF 0x0A00
#define GL_COLOR 0x1800
#define GL_COLOR_ARRAY 0x8076
#define GL_COLOR_ARRAY_POINTER 0x8090
#define GL_COLOR_ARRAY_SIZE 0x8081
#define GL_COLOR_ARRAY_STRIDE 0x8083
#define GL_COLOR_ARRAY_TYPE 0x8082
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_COLOR_CLEAR_VALUE 0x0C22
#define GL_COLOR_INDEX 0x1900
#define GL_COLOR_INDEXES 0x1603
#define GL_COLOR_LOGIC_OP 0x0BF2
#define GL_COLOR_MATERIAL 0x0B57
#define GL_COLOR_MATERIAL_FACE 0x0B55
#define GL_COLOR_MATERIAL_PARAMETER 0x0B56
#define GL_COLOR_WRITEMASK 0x0C23
#define GL_COMPILE 0x1300
#define GL_COMPILE_AND_EXECUTE 0x1301
#define GL_CONSTANT_ATTENUATION 0x1207
#define GL_COPY 0x1503
#define GL_COPY_INVERTED 0x150C
#define GL_COPY_PIXEL_TOKEN 0x0706
#define GL_CULL_FACE 0x0B44
#define GL_CULL_FACE_MODE 0x0B45
#define GL_CURRENT_BIT 0x00000001
#define GL_CURRENT_COLOR 0x0B00
#define GL_CURRENT_INDEX 0x0B01
#define GL_CURRENT_NORMAL 0x0B02
#define GL_CURRENT_RASTER_COLOR 0x0B04
#define GL_CURRENT_RASTER_DISTANCE 0x0B09
#define GL_CURRENT_RASTER_INDEX 0x0B05
#define GL_CURRENT_RASTER_POSITION 0x0B07
#define GL_CURRENT_RASTER_POSITION_VALID 0x0B08
#define GL_CURRENT_RASTER_TEXTURE_COORDS 0x0B06
#define GL_CURRENT_TEXTURE_COORDS 0x0B03
#define GL_CW 0x0900
#define GL_DECAL 0x2101
#define GL_DECR 0x1E03
#define GL_DEPTH 0x1801
#define GL_DEPTH_BIAS 0x0D1F
#define GL_DEPTH_BITS 0x0D56
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_DEPTH_CLEAR_VALUE 0x0B73
#define GL_DEPTH_COMPONENT 0x1902
#define GL_DEPTH_FUNC 0x0B74
#define GL_DEPTH_RANGE 0x0B70
#define GL_DEPTH_SCALE 0x0D1E
#define GL_DEPTH_TEST 0x0B71
#define GL_DEPTH_WRITEMASK 0x0B72
#define GL_DIFFUSE 0x1201
#define GL_DITHER 0x0BD0
#define GL_DOMAIN 0x0A02
#define GL_DONT_CARE 0x1100
#define GL_DOUBLE 0x140A
#define GL_DOUBLEBUFFER 0x0C32
#define GL_DRAW_BUFFER 0x0C01
#define GL_DRAW_PIXEL_TOKEN 0x0705
#define GL_DST_ALPHA 0x0304
#define GL_DST_COLOR 0x0306
#define GL_EDGE_FLAG 0x0B43
#define GL_EDGE_FLAG_ARRAY 0x8079
#define GL_EDGE_FLAG_ARRAY_POINTER 0x8093
#define GL_EDGE_FLAG_ARRAY_STRIDE 0x808C
#define GL_EMISSION 0x1600
#define GL_ENABLE_BIT 0x00002000
#define GL_EQUAL 0x0202
#define GL_EQUIV 0x1509
#define GL_EVAL_BIT 0x00010000
#define GL_EXP 0x0800
#define GL_EXP2 0x0801
#define GL_EXTENSIONS 0x1F03
#define GL_EYE_LINEAR 0x2400
#define GL_EYE_PLANE 0x2502
#define GL_FALSE 0
#define GL_FASTEST 0x1101
#define GL_FEEDBACK 0x1C01
#define GL_FEEDBACK_BUFFER_POINTER 0x0DF0
#define GL_FEEDBACK_BUFFER_SIZE 0x0DF1
#define GL_FEEDBACK_BUFFER_TYPE 0x0DF2
#define GL_FILL 0x1B02
#define GL_FLAT 0x1D00
#define GL_FLOAT 0x1406
#define GL_FOG 0x0B60
#define GL_FOG_BIT 0x00000080
#define GL_FOG_COLOR 0x0B66
#define GL_FOG_DENSITY 0x0B62
#define GL_FOG_END 0x0B64
#define GL_FOG_HINT 0x0C54
#define GL_FOG_INDEX 0x0B61
#define GL_FOG_MODE 0x0B65
#define GL_FOG_START 0x0B63
#define GL_FRONT 0x0404
#define GL_FRONT_AND_BACK 0x0408
#define GL_FRONT_FACE 0x0B46
#define GL_FRONT_LEFT 0x0400
#define GL_FRONT_RIGHT 0x0401
#define GL_GEQUAL 0x0206
#define GL_GREATER 0x0204
#define GL_GREEN 0x1904
#define GL_GREEN_BIAS 0x0D19
#define GL_GREEN_BITS 0x0D53
#define GL_GREEN_SCALE 0x0D18
#define GL_HINT_BIT 0x00008000
#define GL_INCR 0x1E02
#define GL_INDEX_ARRAY 0x8077
#define GL_INDEX_ARRAY_POINTER 0x8091
#define GL_INDEX_ARRAY_STRIDE 0x8086
#define GL_INDEX_ARRAY_TYPE 0x8085
#define GL_INDEX_BITS 0x0D51
#define GL_INDEX_CLEAR_VALUE 0x0C20
#define GL_INDEX_LOGIC_OP 0x0BF1
#define GL_INDEX_MODE 0x0C30
#define GL_INDEX_OFFSET 0x0D13
#define GL_INDEX_SHIFT 0x0D12
#define GL_INDEX_WRITEMASK 0x0C21
#define GL_INT 0x1404
#define GL_INTENSITY 0x8049
#define GL_INTENSITY12 0x804C
#define GL_INTENSITY16 0x804D
#define GL_INTENSITY4 0x804A
#define GL_INTENSITY8 0x804B
#define GL_INVALID_ENUM 0x0500
#define GL_INVALID_OPERATION 0x0502
#define GL_INVALID_VALUE 0x0501
#define GL_INVERT 0x150A
#define GL_KEEP 0x1E00
#define GL_LEFT 0x0406
#define GL_LEQUAL 0x0203
#define GL_LESS 0x0201
#define GL_LIGHT0 0x4000
#define GL_LIGHT1 0x4001
#define GL_LIGHT2 0x4002
#define GL_LIGHT3 0x4003
#define GL_LIGHT4 0x4004
#define GL_LIGHT5 0x4005
#define GL_LIGHT6 0x4006
#define GL_LIGHT7 0x4007
#define GL_LIGHTING 0x0B50
#define GL_LIGHTING_BIT 0x00000040
#define GL_LIGHT_MODEL_AMBIENT 0x0B53
#define GL_LIGHT_MODEL_LOCAL_VIEWER 0x0B51
#define GL_LIGHT_MODEL_TWO_SIDE 0x0B52
#define GL_LINE 0x1B01
#define GL_LINEAR 0x2601
#define GL_LINEAR_ATTENUATION 0x1208
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_LINES 0x0001
#define GL_LINE_BIT 0x00000004
#define GL_LINE_LOOP 0x0002
#define GL_LINE_RESET_TOKEN 0x0707
#define GL_LINE_SMOOTH 0x0B20
#define GL_LINE_SMOOTH_HINT 0x0C52
#define GL_LINE_STIPPLE 0x0B24
#define GL_LINE_STIPPLE_PATTERN 0x0B25
#define GL_LINE_STIPPLE_REPEAT 0x0B26
#define GL_LINE_STRIP 0x0003
#define GL_LINE_TOKEN 0x0702
#define GL_LINE_WIDTH 0x0B21
#define GL_LINE_WIDTH_GRANULARITY 0x0B23
#define GL_LINE_WIDTH_RANGE 0x0B22
#define GL_LIST_BASE 0x0B32
#define GL_LIST_BIT 0x00020000
#define GL_LIST_INDEX 0x0B33
#define GL_LIST_MODE 0x0B30
#define GL_LOAD 0x0101
#define GL_LOGIC_OP 0x0BF1
#define GL_LOGIC_OP_MODE 0x0BF0
#define GL_LUMINANCE 0x1909
#define GL_LUMINANCE12 0x8041
#define GL_LUMINANCE12_ALPHA12 0x8047
#define GL_LUMINANCE12_ALPHA4 0x8046
#define GL_LUMINANCE16 0x8042
#define GL_LUMINANCE16_ALPHA16 0x8048
#define GL_LUMINANCE4 0x803F
#define GL_LUMINANCE4_ALPHA4 0x8043
#define GL_LUMINANCE6_ALPHA2 0x8044
#define GL_LUMINANCE8 0x8040
#define GL_LUMINANCE8_ALPHA8 0x8045
#define GL_LUMINANCE_ALPHA 0x190A
#define GL_MAP1_COLOR_4 0x0D90
#define GL_MAP1_GRID_DOMAIN 0x0DD0
#define GL_MAP1_GRID_SEGMENTS 0x0DD1
#define GL_MAP1_INDEX 0x0D91
#define GL_MAP1_NORMAL 0x0D92
#define GL_MAP1_TEXTURE_COORD_1 0x0D93
#define GL_MAP1_TEXTURE_COORD_2 0x0D94
#define GL_MAP1_TEXTURE_COORD_3 0x0D95
#define GL_MAP1_TEXTURE_COORD_4 0x0D96
#define GL_MAP1_VERTEX_3 0x0D97
#define GL_MAP1_VERTEX_4 0x0D98
#define GL_MAP2_COLOR_4 0x0DB0
#define GL_MAP2_GRID_DOMAIN 0x0DD2
#define GL_MAP2_GRID_SEGMENTS 0x0DD3
#define GL_MAP2_INDEX 0x0DB1
#define GL_MAP2_NORMAL 0x0DB2
#define GL_MAP2_TEXTURE_COORD_1 0x0DB3
#define GL_MAP2_TEXTURE_COORD_2 0x0DB4
#define GL_MAP2_TEXTURE_COORD_3 0x0DB5
#define GL_MAP2_TEXTURE_COORD_4 0x0DB6
#define GL_MAP2_VERTEX_3 0x0DB7
#define GL_MAP2_VERTEX_4 0x0DB8
#define GL_MAP_COLOR 0x0D10
#define GL_MAP_STENCIL 0x0D11
#define GL_MATRIX_MODE 0x0BA0
#define GL_MAX_ATTRIB_STACK_DEPTH 0x0D35
#define GL_MAX_CLIENT_ATTRIB_STACK_DEPTH 0x0D3B
#define GL_MAX_CLIP_PLANES 0x0D32
#define GL_MAX_EVAL_ORDER 0x0D30
#define GL_MAX_LIGHTS 0x0D31
#define GL_MAX_LIST_NESTING 0x0B31
#define GL_MAX_MODELVIEW_STACK_DEPTH 0x0D36
#define GL_MAX_NAME_STACK_DEPTH 0x0D37
#define GL_MAX_PIXEL_MAP_TABLE 0x0D34
#define GL_MAX_PROJECTION_STACK_DEPTH 0x0D38
#define GL_MAX_TEXTURE_SIZE 0x0D33
#define GL_MAX_TEXTURE_STACK_DEPTH 0x0D39
#define GL_MAX_VIEWPORT_DIMS 0x0D3A
#define GL_MODELVIEW 0x1700
#define GL_MODELVIEW_MATRIX 0x0BA6
#define GL_MODELVIEW_STACK_DEPTH 0x0BA3
#define GL_MODULATE 0x2100
#define GL_MULT 0x0103
#define GL_N3F_V3F 0x2A25
#define GL_NAME_STACK_DEPTH 0x0D70
#define GL_NAND 0x150E
#define GL_NEAREST 0x2600
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_NEVER 0x0200
#define GL_NICEST 0x1102
#define GL_NONE 0
#define GL_NOOP 0x1505
#define GL_NOR 0x1508
#define GL_NORMALIZE 0x0BA1
#define GL_NORMAL_ARRAY 0x8075
#define GL_NORMAL_ARRAY_POINTER 0x808F
#define GL_NORMAL_ARRAY_STRIDE 0x807F
#define GL_NORMAL_ARRAY_TYPE 0x807E
#define GL_NOTEQUAL 0x0205
#define GL_NO_ERROR 0
#define GL_OBJECT_LINEAR 0x2401
#define GL_OBJECT_PLANE 0x2501
#define GL_ONE 1
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_OR 0x1507
#define GL_ORDER 0x0A01
#define GL_OR_INVERTED 0x150D
#define GL_OR_REVERSE 0x150B
#define GL_OUT_OF_MEMORY 0x0505
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_PACK_LSB_FIRST 0x0D01
#define GL_PACK_ROW_LENGTH 0x0D02
#define GL_PACK_SKIP_PIXELS 0x0D04
#define GL_PACK_SKIP_ROWS 0x0D03
#define GL_PACK_SWAP_BYTES 0x0D00
#define GL_PASS_THROUGH_TOKEN 0x0700
#define GL_PERSPECTIVE_CORRECTION_HINT 0x0C50
#define GL_PIXEL_MAP_A_TO_A 0x0C79
#define GL_PIXEL_MAP_A_TO_A_SIZE 0x0CB9
#define GL_PIXEL_MAP_B_TO_B 0x0C78
#define GL_PIXEL_MAP_B_TO_B_SIZE 0x0CB8
#define GL_PIXEL_MAP_G_TO_G 0x0C77
#define GL_PIXEL_MAP_G_TO_G_SIZE 0x0CB7
#define GL_PIXEL_MAP_I_TO_A 0x0C75
#define GL_PIXEL_MAP_I_TO_A_SIZE 0x0CB5
#define GL_PIXEL_MAP_I_TO_B 0x0C74
#define GL_PIXEL_MAP_I_TO_B_SIZE 0x0CB4
#define GL_PIXEL_MAP_I_TO_G 0x0C73
#define GL_PIXEL_MAP_I_TO_G_SIZE 0x0CB3
#define GL_PIXEL_MAP_I_TO_I 0x0C70
#define GL_PIXEL_MAP_I_TO_I_SIZE 0x0CB0
#define GL_PIXEL_MAP_I_TO_R 0x0C72
#define GL_PIXEL_MAP_I_TO_R_SIZE 0x0CB2
#define GL_PIXEL_MAP_R_TO_R 0x0C76
#define GL_PIXEL_MAP_R_TO_R_SIZE 0x0CB6
#define GL_PIXEL_MAP_S_TO_S 0x0C71
#define GL_PIXEL_MAP_S_TO_S_SIZE 0x0CB1
#define GL_PIXEL_MODE_BIT 0x00000020
#define GL_POINT 0x1B00
#define GL_POINTS 0x0000
#define GL_POINT_BIT 0x00000002
#define GL_POINT_SIZE 0x0B11
#define GL_POINT_SIZE_GRANULARITY 0x0B13
#define GL_POINT_SIZE_RANGE 0x0B12
#define GL_POINT_SMOOTH 0x0B10
#define GL_POINT_SMOOTH_HINT 0x0C51
#define GL_POINT_TOKEN 0x0701
#define GL_POLYGON 0x0009
#define GL_POLYGON_BIT 0x00000008
#define GL_POLYGON_MODE 0x0B40
#define GL_POLYGON_OFFSET_FACTOR 0x8038
#define GL_POLYGON_OFFSET_FILL 0x8037
#define GL_POLYGON_OFFSET_LINE 0x2A02
#define GL_POLYGON_OFFSET_POINT 0x2A01
#define GL_POLYGON_OFFSET_UNITS 0x2A00
#define GL_POLYGON_SMOOTH 0x0B41
#define GL_POLYGON_SMOOTH_HINT 0x0C53
#define GL_POLYGON_STIPPLE 0x0B42
#define GL_POLYGON_STIPPLE_BIT 0x00000010
#define GL_POLYGON_TOKEN 0x0703
#define GL_POSITION 0x1203
#define GL_PROJECTION 0x1701
#define GL_PROJECTION_MATRIX 0x0BA7
#define GL_PROJECTION_STACK_DEPTH 0x0BA4
#define GL_PROXY_TEXTURE_1D 0x8063
#define GL_PROXY_TEXTURE_2D 0x8064
#define GL_Q 0x2003
#define GL_QUADRATIC_ATTENUATION 0x1209
#define GL_QUADS 0x0007
#define GL_QUAD_STRIP 0x0008
#define GL_R 0x2002
#define GL_R3_G3_B2 0x2A10
#define GL_READ_BUFFER 0x0C02
#define GL_RED 0x1903
#define GL_RED_BIAS 0x0D15
#define GL_RED_BITS 0x0D52
#define GL_RED_SCALE 0x0D14
#define GL_RENDER 0x1C00
#define GL_RENDERER 0x1F01
#define GL_RENDER_MODE 0x0C40
#define GL_REPEAT 0x2901
#define GL_REPLACE 0x1E01
#define GL_RETURN 0x0102
#define GL_RGB 0x1907
#define GL_RGB10 0x8052
#define GL_RGB10_A2 0x8059
#define GL_RGB12 0x8053
#define GL_RGB16 0x8054
#define GL_RGB4 0x804F
#define GL_RGB5 0x8050
#define GL_RGB5_A1 0x8057
#define GL_RGB8 0x8051
#define GL_RGBA 0x1908
#define GL_RGBA12 0x805A
#define GL_RGBA16 0x805B
#define GL_RGBA2 0x8055
#define GL_RGBA4 0x8056
#define GL_RGBA8 0x8058
#define GL_RGBA_MODE 0x0C31
#define GL_RIGHT 0x0407
#define GL_S 0x2000
#define GL_SCISSOR_BIT 0x00080000
#define GL_SCISSOR_BOX 0x0C10
#define GL_SCISSOR_TEST 0x0C11
#define GL_SELECT 0x1C02
#define GL_SELECTION_BUFFER_POINTER 0x0DF3
#define GL_SELECTION_BUFFER_SIZE 0x0DF4
#define GL_SET 0x150F
#define GL_SHADE_MODEL 0x0B54
#define GL_SHININESS 0x1601
#define GL_SHORT 0x1402
#define GL_SMOOTH 0x1D01
#define GL_SPECULAR 0x1202
#define GL_SPHERE_MAP 0x2402
#define GL_SPOT_CUTOFF 0x1206
#define GL_SPOT_DIRECTION 0x1204
#define GL_SPOT_EXPONENT 0x1205
#define GL_SRC_ALPHA 0x0302
#define GL_SRC_ALPHA_SATURATE 0x0308
#define GL_SRC_COLOR 0x0300
#define GL_STACK_OVERFLOW 0x0503
#define GL_STACK_UNDERFLOW 0x0504
#define GL_STENCIL 0x1802
#define GL_STENCIL_BITS 0x0D57
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_STENCIL_CLEAR_VALUE 0x0B91
#define GL_STENCIL_FAIL 0x0B94
#define GL_STENCIL_FUNC 0x0B92
#define GL_STENCIL_INDEX 0x1901
#define GL_STENCIL_PASS_DEPTH_FAIL 0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS 0x0B96
#define GL_STENCIL_REF 0x0B97
#define GL_STENCIL_TEST 0x0B90
#define GL_STENCIL_VALUE_MASK 0x0B93
#define GL_STENCIL_WRITEMASK 0x0B98
#define GL_STEREO 0x0C33
#define GL_SUBPIXEL_BITS 0x0D50
#define GL_T 0x2001
#define GL_T2F_C3F_V3F 0x2A2A
#define GL_T2F_C4F_N3F_V3F 0x2A2C
#define GL_T2F_C4UB_V3F 0x2A29
#define GL_T2F_N3F_V3F 0x2A2B
#define GL_T2F_V3F 0x2A27
#define GL_T4F_C4F_N3F_V4F 0x2A2D
#define GL_T4F_V4F 0x2A28
#define GL_TEXTURE 0x1702
#define GL_TEXTURE_1D 0x0DE0
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_ALPHA_SIZE 0x805F
#define GL_TEXTURE_BINDING_1D 0x8068
#define GL_TEXTURE_BINDING_2D 0x8069
#define GL_TEXTURE_BIT 0x00040000
#define GL_TEXTURE_BLUE_SIZE 0x805E
#define GL_TEXTURE_BORDER 0x1005
#define GL_TEXTURE_BORDER_COLOR 0x1004
#define GL_TEXTURE_COMPONENTS 0x1003
#define GL_TEXTURE_COORD_ARRAY 0x8078
#define GL_TEXTURE_COORD_ARRAY_POINTER 0x8092
#define GL_TEXTURE_COORD_ARRAY_SIZE 0x8088
#define GL_TEXTURE_COORD_ARRAY_STRIDE 0x808A
#define GL_TEXTURE_COORD_ARRAY_TYPE 0x8089
#define GL_TEXTURE_ENV 0x2300
#define GL_TEXTURE_ENV_COLOR 0x2201
#define GL_TEXTURE_ENV_MODE 0x2200
#define GL_TEXTURE_GEN_MODE 0x2500
#define GL_TEXTURE_GEN_Q 0x0C63
#define GL_TEXTURE_GEN_R 0x0C62
#define GL_TEXTURE_GEN_S 0x0C60
#define GL_TEXTURE_GEN_T 0x0C61
#define GL_TEXTURE_GREEN_SIZE 0x805D
#define GL_TEXTURE_HEIGHT 0x1001
#define GL_TEXTURE_INTENSITY_SIZE 0x8061
#define GL_TEXTURE_INTERNAL_FORMAT 0x1003
#define GL_TEXTURE_LUMINANCE_SIZE 0x8060
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MATRIX 0x0BA8
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_PRIORITY 0x8066
#define GL_TEXTURE_RED_SIZE 0x805C
#define GL_TEXTURE_RESIDENT 0x8067
#define GL_TEXTURE_STACK_DEPTH 0x0BA5
#define GL_TEXTURE_WIDTH 0x1000
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_TRANSFORM_BIT 0x00001000
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_FAN 0x0006
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRUE 1
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_UNPACK_LSB_FIRST 0x0CF1
#define GL_UNPACK_ROW_LENGTH 0x0CF2
#define GL_UNPACK_SKIP_PIXELS 0x0CF4
#define GL_UNPACK_SKIP_ROWS 0x0CF3
#define GL_UNPACK_SWAP_BYTES 0x0CF0
#define GL_UNSIGNED_BYTE 0x1401
#define GL_UNSIGNED_INT 0x1405
#define GL_UNSIGNED_SHORT 0x1403
#define GL_V2F 0x2A20
#define GL_V3F 0x2A21
#define GL_VENDOR 0x1F00
#define GL_VERSION 0x1F02
#define GL_VERTEX_ARRAY 0x8074
#define GL_VERTEX_ARRAY_POINTER 0x808E
#define GL_VERTEX_ARRAY_SIZE 0x807A
#define GL_VERTEX_ARRAY_STRIDE 0x807C
#define GL_VERTEX_ARRAY_TYPE 0x807B
#define GL_VIEWPORT 0x0BA2
#define GL_VIEWPORT_BIT 0x00000800
#define GL_XOR 0x1506
#define GL_ZERO 0
#define GL_ZOOM_X 0x0D16
#define GL_ZOOM_Y 0x0D17

#define GL_ALIASED_LINE_WIDTH_RANGE 0x846E
#define GL_ALIASED_POINT_SIZE_RANGE 0x846D
#define GL_BGR 0x80E0
#define GL_BGRA 0x80E1
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_LIGHT_MODEL_COLOR_CONTROL 0x81F8
#define GL_MAX_3D_TEXTURE_SIZE 0x8073
#define GL_MAX_ELEMENTS_INDICES 0x80E9
#define GL_MAX_ELEMENTS_VERTICES 0x80E8
#define GL_PACK_IMAGE_HEIGHT 0x806C
#define GL_PACK_SKIP_IMAGES 0x806B
#define GL_PROXY_TEXTURE_3D 0x8070
#define GL_RESCALE_NORMAL 0x803A
#define GL_SEPARATE_SPECULAR_COLOR 0x81FA
#define GL_SINGLE_COLOR 0x81F9
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY 0x0B23
#define GL_SMOOTH_LINE_WIDTH_RANGE 0x0B22
#define GL_SMOOTH_POINT_SIZE_GRANULARITY 0x0B13
#define GL_SMOOTH_POINT_SIZE_RANGE 0x0B12
#define GL_TEXTURE_3D 0x806F
#define GL_TEXTURE_BASE_LEVEL 0x813C
#define GL_TEXTURE_BINDING_3D 0x806A
#define GL_TEXTURE_DEPTH 0x8071
#define GL_TEXTURE_MAX_LEVEL 0x813D
#define GL_TEXTURE_MAX_LOD 0x813B
#define GL_TEXTURE_MIN_LOD 0x813A
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_UNPACK_IMAGE_HEIGHT 0x806E
#define GL_UNPACK_SKIP_IMAGES 0x806D
#define GL_UNSIGNED_BYTE_2_3_3_REV 0x8362
#define GL_UNSIGNED_BYTE_3_3_2 0x8032
#define GL_UNSIGNED_INT_10_10_10_2 0x8036
#define GL_UNSIGNED_INT_2_10_10_10_REV 0x8368
#define GL_UNSIGNED_INT_8_8_8_8 0x8035
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366
#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#define GL_UNSIGNED_SHORT_4_4_4_4_REV 0x8365
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV 0x8364

#define GL_ACTIVE_TEXTURE 0x84E0
#define GL_ADD_SIGNED 0x8574
#define GL_CLAMP_TO_BORDER 0x812D
#define GL_CLIENT_ACTIVE_TEXTURE 0x84E1
#define GL_COMBINE 0x8570
#define GL_COMBINE_ALPHA 0x8572
#define GL_COMBINE_RGB 0x8571
#define GL_COMPRESSED_ALPHA 0x84E9
#define GL_COMPRESSED_INTENSITY 0x84EC
#define GL_COMPRESSED_LUMINANCE 0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA 0x84EB
#define GL_COMPRESSED_RGB 0x84ED
#define GL_COMPRESSED_RGBA 0x84EE
#define GL_COMPRESSED_TEXTURE_FORMATS 0x86A3
#define GL_CONSTANT 0x8576
#define GL_DOT3_RGB 0x86AE
#define GL_DOT3_RGBA 0x86AF
#define GL_INTERPOLATE 0x8575
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE 0x851C
#define GL_MAX_TEXTURE_UNITS 0x84E2
#define GL_MULTISAMPLE 0x809D
#define GL_MULTISAMPLE_BIT 0x20000000
#define GL_NORMAL_MAP 0x8511
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#define GL_OPERAND0_ALPHA 0x8598
#define GL_OPERAND0_RGB 0x8590
#define GL_OPERAND1_ALPHA 0x8599
#define GL_OPERAND1_RGB 0x8591
#define GL_OPERAND2_ALPHA 0x859A
#define GL_OPERAND2_RGB 0x8592
#define GL_PREVIOUS 0x8578
#define GL_PRIMARY_COLOR 0x8577
#define GL_PROXY_TEXTURE_CUBE_MAP 0x851B
#define GL_REFLECTION_MAP 0x8512
#define GL_RGB_SCALE 0x8573
#define GL_SAMPLES 0x80A9
#define GL_SAMPLE_ALPHA_TO_COVERAGE 0x809E
#define GL_SAMPLE_ALPHA_TO_ONE 0x809F
#define GL_SAMPLE_BUFFERS 0x80A8
#define GL_SAMPLE_COVERAGE 0x80A0
#define GL_SAMPLE_COVERAGE_INVERT 0x80AB
#define GL_SAMPLE_COVERAGE_VALUE 0x80AA
#define GL_SOURCE0_ALPHA 0x8588
#define GL_SOURCE0_RGB 0x8580
#define GL_SOURCE1_ALPHA 0x8589
#define GL_SOURCE1_RGB 0x8581
#define GL_SOURCE2_ALPHA 0x858A
#define GL_SOURCE2_RGB 0x8582
#define GL_SUBTRACT 0x84E7
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE10 0x84CA
#define GL_TEXTURE11 0x84CB
#define GL_TEXTURE12 0x84CC
#define GL_TEXTURE13 0x84CD
#define GL_TEXTURE14 0x84CE
#define GL_TEXTURE15 0x84CF
#define GL_TEXTURE16 0x84D0
#define GL_TEXTURE17 0x84D1
#define GL_TEXTURE18 0x84D2
#define GL_TEXTURE19 0x84D3
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE20 0x84D4
#define GL_TEXTURE21 0x84D5
#define GL_TEXTURE22 0x84D6
#define GL_TEXTURE23 0x84D7
#define GL_TEXTURE24 0x84D8
#define GL_TEXTURE25 0x84D9
#define GL_TEXTURE26 0x84DA
#define GL_TEXTURE27 0x84DB
#define GL_TEXTURE28 0x84DC
#define GL_TEXTURE29 0x84DD
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE30 0x84DE
#define GL_TEXTURE31 0x84DF
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_TEXTURE6 0x84C6
#define GL_TEXTURE7 0x84C7
#define GL_TEXTURE8 0x84C8
#define GL_TEXTURE9 0x84C9
#define GL_TEXTURE_BINDING_CUBE_MAP 0x8514
#define GL_TEXTURE_COMPRESSED 0x86A1
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE 0x86A0
#define GL_TEXTURE_COMPRESSION_HINT 0x84EF
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#define GL_TRANSPOSE_COLOR_MATRIX 0x84E6
#define GL_TRANSPOSE_MODELVIEW_MATRIX 0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX 0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX 0x84E5

#define GL_BLEND_DST_ALPHA 0x80CA
#define GL_BLEND_DST_RGB 0x80C8
#define GL_BLEND_SRC_ALPHA 0x80CB
#define GL_BLEND_SRC_RGB 0x80C9
#define GL_COLOR_SUM 0x8458
#define GL_COMPARE_R_TO_TEXTURE 0x884E
#define GL_CURRENT_FOG_COORDINATE 0x8453
#define GL_CURRENT_SECONDARY_COLOR 0x8459
#define GL_DECR_WRAP 0x8508
#define GL_DEPTH_COMPONENT16 0x81A5
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_DEPTH_COMPONENT32 0x81A7
#define GL_DEPTH_TEXTURE_MODE 0x884B
#define GL_FOG_COORDINATE 0x8451
#define GL_FOG_COORDINATE_ARRAY 0x8457
#define GL_FOG_COORDINATE_ARRAY_POINTER 0x8456
#define GL_FOG_COORDINATE_ARRAY_STRIDE 0x8455
#define GL_FOG_COORDINATE_ARRAY_TYPE 0x8454
#define GL_FOG_COORDINATE_SOURCE 0x8450
#define GL_FRAGMENT_DEPTH 0x8452
#define GL_GENERATE_MIPMAP 0x8191
#define GL_GENERATE_MIPMAP_HINT 0x8192
#define GL_INCR_WRAP 0x8507
#define GL_MAX_TEXTURE_LOD_BIAS 0x84FD
#define GL_MIRRORED_REPEAT 0x8370
#define GL_POINT_DISTANCE_ATTENUATION 0x8129
#define GL_POINT_FADE_THRESHOLD_SIZE 0x8128
#define GL_POINT_SIZE_MAX 0x8127
#define GL_POINT_SIZE_MIN 0x8126
#define GL_SECONDARY_COLOR_ARRAY 0x845E
#define GL_SECONDARY_COLOR_ARRAY_POINTER 0x845D
#define GL_SECONDARY_COLOR_ARRAY_SIZE 0x845A
#define GL_SECONDARY_COLOR_ARRAY_STRIDE 0x845C
#define GL_SECONDARY_COLOR_ARRAY_TYPE 0x845B
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#define GL_TEXTURE_COMPARE_MODE 0x884C
#define GL_TEXTURE_DEPTH_SIZE 0x884A
#define GL_TEXTURE_FILTER_CONTROL 0x8500
#define GL_TEXTURE_LOD_BIAS 0x8501

#define GL_ARRAY_BUFFER 0x8892
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_BUFFER_ACCESS 0x88BB
#define GL_BUFFER_MAPPED 0x88BC
#define GL_BUFFER_MAP_POINTER 0x88BD
#define GL_BUFFER_SIZE 0x8764
#define GL_BUFFER_USAGE 0x8765
#define GL_COLOR_ARRAY_BUFFER_BINDING 0x8898
#define GL_CURRENT_FOG_COORD 0x8453
#define GL_CURRENT_QUERY 0x8865
#define GL_DYNAMIC_COPY 0x88EA
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING 0x889B
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#define GL_FOG_COORD 0x8451
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING 0x889D
#define GL_FOG_COORD_ARRAY 0x8457
#define GL_FOG_COORD_ARRAY_BUFFER_BINDING 0x889D
#define GL_FOG_COORD_ARRAY_POINTER 0x8456
#define GL_FOG_COORD_ARRAY_STRIDE 0x8455
#define GL_FOG_COORD_ARRAY_TYPE 0x8454
#define GL_FOG_COORD_SRC 0x8450
#define GL_INDEX_ARRAY_BUFFER_BINDING 0x8899
#define GL_NORMAL_ARRAY_BUFFER_BINDING 0x8897
#define GL_QUERY_COUNTER_BITS 0x8864
#define GL_QUERY_RESULT 0x8866
#define GL_QUERY_RESULT_AVAILABLE 0x8867
#define GL_READ_ONLY 0x88B8
#define GL_READ_WRITE 0x88BA
#define GL_SAMPLES_PASSED 0x8914
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING 0x889C
#define GL_SRC0_ALPHA 0x8588
#define GL_SRC0_RGB 0x8580
#define GL_SRC1_ALPHA 0x8589
#define GL_SRC1_RGB 0x8581
#define GL_SRC2_ALPHA 0x858A
#define GL_SRC2_RGB 0x8582
#define GL_STATIC_COPY 0x88E6
#define GL_STATIC_DRAW 0x88E4
#define GL_STATIC_READ 0x88E5
#define GL_STREAM_COPY 0x88E2
#define GL_STREAM_DRAW 0x88E0
#define GL_STREAM_READ 0x88E1
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING 0x889A
#define GL_VERTEX_ARRAY_BUFFER_BINDING 0x8896
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_WEIGHT_ARRAY_BUFFER_BINDING 0x889E
#define GL_WRITE_ONLY 0x88B9

#define GL_ACTIVE_ATTRIBUTES 0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH 0x8B8A
#define GL_ACTIVE_UNIFORMS 0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH 0x8B87
#define GL_ATTACHED_SHADERS 0x8B85
#define GL_BLEND_EQUATION_ALPHA 0x883D
#define GL_BLEND_EQUATION_RGB 0x8009
#define GL_BOOL 0x8B56
#define GL_BOOL_VEC2 0x8B57
#define GL_BOOL_VEC3 0x8B58
#define GL_BOOL_VEC4 0x8B59
#define GL_COMPILE_STATUS 0x8B81
#define GL_COORD_REPLACE 0x8862
#define GL_CURRENT_PROGRAM 0x8B8D
#define GL_CURRENT_VERTEX_ATTRIB 0x8626
#define GL_DELETE_STATUS 0x8B80
#define GL_DRAW_BUFFER0 0x8825
#define GL_DRAW_BUFFER1 0x8826
#define GL_DRAW_BUFFER10 0x882F
#define GL_DRAW_BUFFER11 0x8830
#define GL_DRAW_BUFFER12 0x8831
#define GL_DRAW_BUFFER13 0x8832
#define GL_DRAW_BUFFER14 0x8833
#define GL_DRAW_BUFFER15 0x8834
#define GL_DRAW_BUFFER2 0x8827
#define GL_DRAW_BUFFER3 0x8828
#define GL_DRAW_BUFFER4 0x8829
#define GL_DRAW_BUFFER5 0x882A
#define GL_DRAW_BUFFER6 0x882B
#define GL_DRAW_BUFFER7 0x882C
#define GL_DRAW_BUFFER8 0x882D
#define GL_DRAW_BUFFER9 0x882E
#define GL_FLOAT_MAT2 0x8B5A
#define GL_FLOAT_MAT3 0x8B5B
#define GL_FLOAT_MAT4 0x8B5C
#define GL_FLOAT_VEC2 0x8B50
#define GL_FLOAT_VEC3 0x8B51
#define GL_FLOAT_VEC4 0x8B52
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_INT_VEC2 0x8B53
#define GL_INT_VEC3 0x8B54
#define GL_INT_VEC4 0x8B55
#define GL_LINK_STATUS 0x8B82
#define GL_LOWER_LEFT 0x8CA1
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_MAX_DRAW_BUFFERS 0x8824
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_MAX_TEXTURE_COORDS 0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS 0x8872
#define GL_MAX_VARYING_FLOATS 0x8B4B
#define GL_MAX_VERTEX_ATTRIBS 0x8869
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS 0x8B4A
#define GL_POINT_SPRITE 0x8861
#define GL_POINT_SPRITE_COORD_ORIGIN 0x8CA0
#define GL_SAMPLER_1D 0x8B5D
#define GL_SAMPLER_1D_SHADOW 0x8B61
#define GL_SAMPLER_2D 0x8B5E
#define GL_SAMPLER_2D_SHADOW 0x8B62
#define GL_SAMPLER_3D 0x8B5F
#define GL_SAMPLER_CUBE 0x8B60
#define GL_SHADER_SOURCE_LENGTH 0x8B88
#define GL_SHADER_TYPE 0x8B4F
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_STENCIL_BACK_FAIL 0x8801
#define GL_STENCIL_BACK_FUNC 0x8800
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL 0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS 0x8803
#define GL_STENCIL_BACK_REF 0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK 0x8CA4
#define GL_STENCIL_BACK_WRITEMASK 0x8CA5
#define GL_UPPER_LEFT 0x8CA2
#define GL_VALIDATE_STATUS 0x8B83
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED 0x8622
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_VERTEX_ATTRIB_ARRAY_POINTER 0x8645
#define GL_VERTEX_ATTRIB_ARRAY_SIZE 0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE 0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE 0x8625
#define GL_VERTEX_PROGRAM_POINT_SIZE 0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE 0x8643
#define GL_VERTEX_SHADER 0x8B31

#define GL_COMPRESSED_SLUMINANCE 0x8C4A
#define GL_COMPRESSED_SLUMINANCE_ALPHA 0x8C4B
#define GL_COMPRESSED_SRGB 0x8C48
#define GL_COMPRESSED_SRGB_ALPHA 0x8C49
#define GL_CURRENT_RASTER_SECONDARY_COLOR 0x845F
#define GL_FLOAT_MAT2x3 0x8B65
#define GL_FLOAT_MAT2x4 0x8B66
#define GL_FLOAT_MAT3x2 0x8B67
#define GL_FLOAT_MAT3x4 0x8B68
#define GL_FLOAT_MAT4x2 0x8B69
#define GL_FLOAT_MAT4x3 0x8B6A
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_PIXEL_PACK_BUFFER_BINDING 0x88ED
#define GL_PIXEL_UNPACK_BUFFER 0x88EC
#define GL_PIXEL_UNPACK_BUFFER_BINDING 0x88EF
#define GL_SLUMINANCE 0x8C46
#define GL_SLUMINANCE8 0x8C47
#define GL_SLUMINANCE8_ALPHA8 0x8C45
#define GL_SLUMINANCE_ALPHA 0x8C44
#define GL_SRGB 0x8C40
#define GL_SRGB8 0x8C41
#define GL_SRGB8_ALPHA8 0x8C43
#define GL_SRGB_ALPHA 0x8C42




extern void (CODEGEN_FUNCPTR *_ptrc_glAccum)(GLenum, GLfloat);
#define glAccum _ptrc_glAccum
extern void (CODEGEN_FUNCPTR *_ptrc_glAlphaFunc)(GLenum, GLfloat);
#define glAlphaFunc _ptrc_glAlphaFunc
extern void (CODEGEN_FUNCPTR *_ptrc_glBegin)(GLenum);
#define glBegin _ptrc_glBegin
extern void (CODEGEN_FUNCPTR *_ptrc_glBitmap)(GLsizei, GLsizei, GLfloat, GLfloat, GLfloat, GLfloat, const GLubyte *);
#define glBitmap _ptrc_glBitmap
extern void (CODEGEN_FUNCPTR *_ptrc_glBlendFunc)(GLenum, GLenum);
#define glBlendFunc _ptrc_glBlendFunc
extern void (CODEGEN_FUNCPTR *_ptrc_glCallList)(GLuint);
#define glCallList _ptrc_glCallList
extern void (CODEGEN_FUNCPTR *_ptrc_glCallLists)(GLsizei, GLenum, const GLvoid *);
#define glCallLists _ptrc_glCallLists
extern void (CODEGEN_FUNCPTR *_ptrc_glClear)(GLbitfield);
#define glClear _ptrc_glClear
extern void (CODEGEN_FUNCPTR *_ptrc_glClearAccum)(GLfloat, GLfloat, GLfloat, GLfloat);
#define glClearAccum _ptrc_glClearAccum
extern void (CODEGEN_FUNCPTR *_ptrc_glClearColor)(GLfloat, GLfloat, GLfloat, GLfloat);
#define glClearColor _ptrc_glClearColor
extern void (CODEGEN_FUNCPTR *_ptrc_glClearDepth)(GLdouble);
#define glClearDepth _ptrc_glClearDepth
extern void (CODEGEN_FUNCPTR *_ptrc_glClearIndex)(GLfloat);
#define glClearIndex _ptrc_glClearIndex
extern void (CODEGEN_FUNCPTR *_ptrc_glClearStencil)(GLint);
#define glClearStencil _ptrc_glClearStencil
extern void (CODEGEN_FUNCPTR *_ptrc_glClipPlane)(GLenum, const GLdouble *);
#define glClipPlane _ptrc_glClipPlane
extern void (CODEGEN_FUNCPTR *_ptrc_glColor3b)(GLbyte, GLbyte, GLbyte);
#define glColor3b _ptrc_glColor3b
extern void (CODEGEN_FUNCPTR *_ptrc_glColor3bv)(const GLbyte *);
#define glColor3bv _ptrc_glColor3bv
extern void (CODEGEN_FUNCPTR *_ptrc_glColor3d)(GLdouble, GLdouble, GLdouble);
#define glColor3d _ptrc_glColor3d
extern void (CODEGEN_FUNCPTR *_ptrc_glColor3dv)(const GLdouble *);
#define glColor3dv _ptrc_glColor3dv
extern void (CODEGEN_FUNCPTR *_ptrc_glColor3f)(GLfloat, GLfloat, GLfloat);
#define glColor3f _ptrc_glColor3f
extern void (CODEGEN_FUNCPTR *_ptrc_glColor3fv)(const GLfloat *);
#define glColor3fv _ptrc_glColor3fv
extern void (CODEGEN_FUNCPTR *_ptrc_glColor3i)(GLint, GLint, GLint);
#define glColor3i _ptrc_glColor3i
extern void (CODEGEN_FUNCPTR *_ptrc_glColor3iv)(const GLint *);
#define glColor3iv _ptrc_glColor3iv
extern void (CODEGEN_FUNCPTR *_ptrc_glColor3s)(GLshort, GLshort, GLshort);
#define glColor3s _ptrc_glColor3s
extern void (CODEGEN_FUNCPTR *_ptrc_glColor3sv)(const GLshort *);
#define glColor3sv _ptrc_glColor3sv
extern void (CODEGEN_FUNCPTR *_ptrc_glColor3ub)(GLubyte, GLubyte, GLubyte);
#define glColor3ub _ptrc_glColor3ub
extern void (CODEGEN_FUNCPTR *_ptrc_glColor3ubv)(const GLubyte *);
#define glColor3ubv _ptrc_glColor3ubv
extern void (CODEGEN_FUNCPTR *_ptrc_glColor3ui)(GLuint, GLuint, GLuint);
#define glColor3ui _ptrc_glColor3ui
extern void (CODEGEN_FUNCPTR *_ptrc_glColor3uiv)(const GLuint *);
#define glColor3uiv _ptrc_glColor3uiv
extern void (CODEGEN_FUNCPTR *_ptrc_glColor3us)(GLushort, GLushort, GLushort);
#define glColor3us _ptrc_glColor3us
extern void (CODEGEN_FUNCPTR *_ptrc_glColor3usv)(const GLushort *);
#define glColor3usv _ptrc_glColor3usv
extern void (CODEGEN_FUNCPTR *_ptrc_glColor4b)(GLbyte, GLbyte, GLbyte, GLbyte);
#define glColor4b _ptrc_glColor4b
extern void (CODEGEN_FUNCPTR *_ptrc_glColor4bv)(const GLbyte *);
#define glColor4bv _ptrc_glColor4bv
extern void (CODEGEN_FUNCPTR *_ptrc_glColor4d)(GLdouble, GLdouble, GLdouble, GLdouble);
#define glColor4d _ptrc_glColor4d
extern void (CODEGEN_FUNCPTR *_ptrc_glColor4dv)(const GLdouble *);
#define glColor4dv _ptrc_glColor4dv
extern void (CODEGEN_FUNCPTR *_ptrc_glColor4f)(GLfloat, GLfloat, GLfloat, GLfloat);
#define glColor4f _ptrc_glColor4f
extern void (CODEGEN_FUNCPTR *_ptrc_glColor4fv)(const GLfloat *);
#define glColor4fv _ptrc_glColor4fv
extern void (CODEGEN_FUNCPTR *_ptrc_glColor4i)(GLint, GLint, GLint, GLint);
#define glColor4i _ptrc_glColor4i
extern void (CODEGEN_FUNCPTR *_ptrc_glColor4iv)(const GLint *);
#define glColor4iv _ptrc_glColor4iv
extern void (CODEGEN_FUNCPTR *_ptrc_glColor4s)(GLshort, GLshort, GLshort, GLshort);
#define glColor4s _ptrc_glColor4s
extern void (CODEGEN_FUNCPTR *_ptrc_glColor4sv)(const GLshort *);
#define glColor4sv _ptrc_glColor4sv
extern void (CODEGEN_FUNCPTR *_ptrc_glColor4ub)(GLubyte, GLubyte, GLubyte, GLubyte);
#define glColor4ub _ptrc_glColor4ub
extern void (CODEGEN_FUNCPTR *_ptrc_glColor4ubv)(const GLubyte *);
#define glColor4ubv _ptrc_glColor4ubv
extern void (CODEGEN_FUNCPTR *_ptrc_glColor4ui)(GLuint, GLuint, GLuint, GLuint);
#define glColor4ui _ptrc_glColor4ui
extern void (CODEGEN_FUNCPTR *_ptrc_glColor4uiv)(const GLuint *);
#define glColor4uiv _ptrc_glColor4uiv
extern void (CODEGEN_FUNCPTR *_ptrc_glColor4us)(GLushort, GLushort, GLushort, GLushort);
#define glColor4us _ptrc_glColor4us
extern void (CODEGEN_FUNCPTR *_ptrc_glColor4usv)(const GLushort *);
#define glColor4usv _ptrc_glColor4usv
extern void (CODEGEN_FUNCPTR *_ptrc_glColorMask)(GLboolean, GLboolean, GLboolean, GLboolean);
#define glColorMask _ptrc_glColorMask
extern void (CODEGEN_FUNCPTR *_ptrc_glColorMaterial)(GLenum, GLenum);
#define glColorMaterial _ptrc_glColorMaterial
extern void (CODEGEN_FUNCPTR *_ptrc_glCopyPixels)(GLint, GLint, GLsizei, GLsizei, GLenum);
#define glCopyPixels _ptrc_glCopyPixels
extern void (CODEGEN_FUNCPTR *_ptrc_glCullFace)(GLenum);
#define glCullFace _ptrc_glCullFace
extern void (CODEGEN_FUNCPTR *_ptrc_glDeleteLists)(GLuint, GLsizei);
#define glDeleteLists _ptrc_glDeleteLists
extern void (CODEGEN_FUNCPTR *_ptrc_glDepthFunc)(GLenum);
#define glDepthFunc _ptrc_glDepthFunc
extern void (CODEGEN_FUNCPTR *_ptrc_glDepthMask)(GLboolean);
#define glDepthMask _ptrc_glDepthMask
extern void (CODEGEN_FUNCPTR *_ptrc_glDepthRange)(GLdouble, GLdouble);
#define glDepthRange _ptrc_glDepthRange
extern void (CODEGEN_FUNCPTR *_ptrc_glDisable)(GLenum);
#define glDisable _ptrc_glDisable
extern void (CODEGEN_FUNCPTR *_ptrc_glDrawBuffer)(GLenum);
#define glDrawBuffer _ptrc_glDrawBuffer
extern void (CODEGEN_FUNCPTR *_ptrc_glDrawPixels)(GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
#define glDrawPixels _ptrc_glDrawPixels
extern void (CODEGEN_FUNCPTR *_ptrc_glEdgeFlag)(GLboolean);
#define glEdgeFlag _ptrc_glEdgeFlag
extern void (CODEGEN_FUNCPTR *_ptrc_glEdgeFlagv)(const GLboolean *);
#define glEdgeFlagv _ptrc_glEdgeFlagv
extern void (CODEGEN_FUNCPTR *_ptrc_glEnable)(GLenum);
#define glEnable _ptrc_glEnable
extern void (CODEGEN_FUNCPTR *_ptrc_glEnd)();
#define glEnd _ptrc_glEnd
extern void (CODEGEN_FUNCPTR *_ptrc_glEndList)();
#define glEndList _ptrc_glEndList
extern void (CODEGEN_FUNCPTR *_ptrc_glEvalCoord1d)(GLdouble);
#define glEvalCoord1d _ptrc_glEvalCoord1d
extern void (CODEGEN_FUNCPTR *_ptrc_glEvalCoord1dv)(const GLdouble *);
#define glEvalCoord1dv _ptrc_glEvalCoord1dv
extern void (CODEGEN_FUNCPTR *_ptrc_glEvalCoord1f)(GLfloat);
#define glEvalCoord1f _ptrc_glEvalCoord1f
extern void (CODEGEN_FUNCPTR *_ptrc_glEvalCoord1fv)(const GLfloat *);
#define glEvalCoord1fv _ptrc_glEvalCoord1fv
extern void (CODEGEN_FUNCPTR *_ptrc_glEvalCoord2d)(GLdouble, GLdouble);
#define glEvalCoord2d _ptrc_glEvalCoord2d
extern void (CODEGEN_FUNCPTR *_ptrc_glEvalCoord2dv)(const GLdouble *);
#define glEvalCoord2dv _ptrc_glEvalCoord2dv
extern void (CODEGEN_FUNCPTR *_ptrc_glEvalCoord2f)(GLfloat, GLfloat);
#define glEvalCoord2f _ptrc_glEvalCoord2f
extern void (CODEGEN_FUNCPTR *_ptrc_glEvalCoord2fv)(const GLfloat *);
#define glEvalCoord2fv _ptrc_glEvalCoord2fv
extern void (CODEGEN_FUNCPTR *_ptrc_glEvalMesh1)(GLenum, GLint, GLint);
#define glEvalMesh1 _ptrc_glEvalMesh1
extern void (CODEGEN_FUNCPTR *_ptrc_glEvalMesh2)(GLenum, GLint, GLint, GLint, GLint);
#define glEvalMesh2 _ptrc_glEvalMesh2
extern void (CODEGEN_FUNCPTR *_ptrc_glEvalPoint1)(GLint);
#define glEvalPoint1 _ptrc_glEvalPoint1
extern void (CODEGEN_FUNCPTR *_ptrc_glEvalPoint2)(GLint, GLint);
#define glEvalPoint2 _ptrc_glEvalPoint2
extern void (CODEGEN_FUNCPTR *_ptrc_glFeedbackBuffer)(GLsizei, GLenum, GLfloat *);
#define glFeedbackBuffer _ptrc_glFeedbackBuffer
extern void (CODEGEN_FUNCPTR *_ptrc_glFinish)();
#define glFinish _ptrc_glFinish
extern void (CODEGEN_FUNCPTR *_ptrc_glFlush)();
#define glFlush _ptrc_glFlush
extern void (CODEGEN_FUNCPTR *_ptrc_glFogf)(GLenum, GLfloat);
#define glFogf _ptrc_glFogf
extern void (CODEGEN_FUNCPTR *_ptrc_glFogfv)(GLenum, const GLfloat *);
#define glFogfv _ptrc_glFogfv
extern void (CODEGEN_FUNCPTR *_ptrc_glFogi)(GLenum, GLint);
#define glFogi _ptrc_glFogi
extern void (CODEGEN_FUNCPTR *_ptrc_glFogiv)(GLenum, const GLint *);
#define glFogiv _ptrc_glFogiv
extern void (CODEGEN_FUNCPTR *_ptrc_glFrontFace)(GLenum);
#define glFrontFace _ptrc_glFrontFace
extern void (CODEGEN_FUNCPTR *_ptrc_glFrustum)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
#define glFrustum _ptrc_glFrustum
extern GLuint (CODEGEN_FUNCPTR *_ptrc_glGenLists)(GLsizei);
#define glGenLists _ptrc_glGenLists
extern void (CODEGEN_FUNCPTR *_ptrc_glGetBooleanv)(GLenum, GLboolean *);
#define glGetBooleanv _ptrc_glGetBooleanv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetClipPlane)(GLenum, GLdouble *);
#define glGetClipPlane _ptrc_glGetClipPlane
extern void (CODEGEN_FUNCPTR *_ptrc_glGetDoublev)(GLenum, GLdouble *);
#define glGetDoublev _ptrc_glGetDoublev
extern GLenum (CODEGEN_FUNCPTR *_ptrc_glGetError)();
#define glGetError _ptrc_glGetError
extern void (CODEGEN_FUNCPTR *_ptrc_glGetFloatv)(GLenum, GLfloat *);
#define glGetFloatv _ptrc_glGetFloatv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetIntegerv)(GLenum, GLint *);
#define glGetIntegerv _ptrc_glGetIntegerv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetLightfv)(GLenum, GLenum, GLfloat *);
#define glGetLightfv _ptrc_glGetLightfv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetLightiv)(GLenum, GLenum, GLint *);
#define glGetLightiv _ptrc_glGetLightiv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetMapdv)(GLenum, GLenum, GLdouble *);
#define glGetMapdv _ptrc_glGetMapdv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetMapfv)(GLenum, GLenum, GLfloat *);
#define glGetMapfv _ptrc_glGetMapfv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetMapiv)(GLenum, GLenum, GLint *);
#define glGetMapiv _ptrc_glGetMapiv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetMaterialfv)(GLenum, GLenum, GLfloat *);
#define glGetMaterialfv _ptrc_glGetMaterialfv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetMaterialiv)(GLenum, GLenum, GLint *);
#define glGetMaterialiv _ptrc_glGetMaterialiv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetPixelMapfv)(GLenum, GLfloat *);
#define glGetPixelMapfv _ptrc_glGetPixelMapfv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetPixelMapuiv)(GLenum, GLuint *);
#define glGetPixelMapuiv _ptrc_glGetPixelMapuiv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetPixelMapusv)(GLenum, GLushort *);
#define glGetPixelMapusv _ptrc_glGetPixelMapusv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetPolygonStipple)(GLubyte *);
#define glGetPolygonStipple _ptrc_glGetPolygonStipple
extern const GLubyte * (CODEGEN_FUNCPTR *_ptrc_glGetString)(GLenum);
#define glGetString _ptrc_glGetString
extern void (CODEGEN_FUNCPTR *_ptrc_glGetTexEnvfv)(GLenum, GLenum, GLfloat *);
#define glGetTexEnvfv _ptrc_glGetTexEnvfv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetTexEnviv)(GLenum, GLenum, GLint *);
#define glGetTexEnviv _ptrc_glGetTexEnviv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetTexGendv)(GLenum, GLenum, GLdouble *);
#define glGetTexGendv _ptrc_glGetTexGendv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetTexGenfv)(GLenum, GLenum, GLfloat *);
#define glGetTexGenfv _ptrc_glGetTexGenfv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetTexGeniv)(GLenum, GLenum, GLint *);
#define glGetTexGeniv _ptrc_glGetTexGeniv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetTexImage)(GLenum, GLint, GLenum, GLenum, GLvoid *);
#define glGetTexImage _ptrc_glGetTexImage
extern void (CODEGEN_FUNCPTR *_ptrc_glGetTexLevelParameterfv)(GLenum, GLint, GLenum, GLfloat *);
#define glGetTexLevelParameterfv _ptrc_glGetTexLevelParameterfv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetTexLevelParameteriv)(GLenum, GLint, GLenum, GLint *);
#define glGetTexLevelParameteriv _ptrc_glGetTexLevelParameteriv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetTexParameterfv)(GLenum, GLenum, GLfloat *);
#define glGetTexParameterfv _ptrc_glGetTexParameterfv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetTexParameteriv)(GLenum, GLenum, GLint *);
#define glGetTexParameteriv _ptrc_glGetTexParameteriv
extern void (CODEGEN_FUNCPTR *_ptrc_glHint)(GLenum, GLenum);
#define glHint _ptrc_glHint
extern void (CODEGEN_FUNCPTR *_ptrc_glIndexMask)(GLuint);
#define glIndexMask _ptrc_glIndexMask
extern void (CODEGEN_FUNCPTR *_ptrc_glIndexd)(GLdouble);
#define glIndexd _ptrc_glIndexd
extern void (CODEGEN_FUNCPTR *_ptrc_glIndexdv)(const GLdouble *);
#define glIndexdv _ptrc_glIndexdv
extern void (CODEGEN_FUNCPTR *_ptrc_glIndexf)(GLfloat);
#define glIndexf _ptrc_glIndexf
extern void (CODEGEN_FUNCPTR *_ptrc_glIndexfv)(const GLfloat *);
#define glIndexfv _ptrc_glIndexfv
extern void (CODEGEN_FUNCPTR *_ptrc_glIndexi)(GLint);
#define glIndexi _ptrc_glIndexi
extern void (CODEGEN_FUNCPTR *_ptrc_glIndexiv)(const GLint *);
#define glIndexiv _ptrc_glIndexiv
extern void (CODEGEN_FUNCPTR *_ptrc_glIndexs)(GLshort);
#define glIndexs _ptrc_glIndexs
extern void (CODEGEN_FUNCPTR *_ptrc_glIndexsv)(const GLshort *);
#define glIndexsv _ptrc_glIndexsv
extern void (CODEGEN_FUNCPTR *_ptrc_glInitNames)();
#define glInitNames _ptrc_glInitNames
extern GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsEnabled)(GLenum);
#define glIsEnabled _ptrc_glIsEnabled
extern GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsList)(GLuint);
#define glIsList _ptrc_glIsList
extern void (CODEGEN_FUNCPTR *_ptrc_glLightModelf)(GLenum, GLfloat);
#define glLightModelf _ptrc_glLightModelf
extern void (CODEGEN_FUNCPTR *_ptrc_glLightModelfv)(GLenum, const GLfloat *);
#define glLightModelfv _ptrc_glLightModelfv
extern void (CODEGEN_FUNCPTR *_ptrc_glLightModeli)(GLenum, GLint);
#define glLightModeli _ptrc_glLightModeli
extern void (CODEGEN_FUNCPTR *_ptrc_glLightModeliv)(GLenum, const GLint *);
#define glLightModeliv _ptrc_glLightModeliv
extern void (CODEGEN_FUNCPTR *_ptrc_glLightf)(GLenum, GLenum, GLfloat);
#define glLightf _ptrc_glLightf
extern void (CODEGEN_FUNCPTR *_ptrc_glLightfv)(GLenum, GLenum, const GLfloat *);
#define glLightfv _ptrc_glLightfv
extern void (CODEGEN_FUNCPTR *_ptrc_glLighti)(GLenum, GLenum, GLint);
#define glLighti _ptrc_glLighti
extern void (CODEGEN_FUNCPTR *_ptrc_glLightiv)(GLenum, GLenum, const GLint *);
#define glLightiv _ptrc_glLightiv
extern void (CODEGEN_FUNCPTR *_ptrc_glLineStipple)(GLint, GLushort);
#define glLineStipple _ptrc_glLineStipple
extern void (CODEGEN_FUNCPTR *_ptrc_glLineWidth)(GLfloat);
#define glLineWidth _ptrc_glLineWidth
extern void (CODEGEN_FUNCPTR *_ptrc_glListBase)(GLuint);
#define glListBase _ptrc_glListBase
extern void (CODEGEN_FUNCPTR *_ptrc_glLoadIdentity)();
#define glLoadIdentity _ptrc_glLoadIdentity
extern void (CODEGEN_FUNCPTR *_ptrc_glLoadMatrixd)(const GLdouble *);
#define glLoadMatrixd _ptrc_glLoadMatrixd
extern void (CODEGEN_FUNCPTR *_ptrc_glLoadMatrixf)(const GLfloat *);
#define glLoadMatrixf _ptrc_glLoadMatrixf
extern void (CODEGEN_FUNCPTR *_ptrc_glLoadName)(GLuint);
#define glLoadName _ptrc_glLoadName
extern void (CODEGEN_FUNCPTR *_ptrc_glLogicOp)(GLenum);
#define glLogicOp _ptrc_glLogicOp
extern void (CODEGEN_FUNCPTR *_ptrc_glMap1d)(GLenum, GLdouble, GLdouble, GLint, GLint, const GLdouble *);
#define glMap1d _ptrc_glMap1d
extern void (CODEGEN_FUNCPTR *_ptrc_glMap1f)(GLenum, GLfloat, GLfloat, GLint, GLint, const GLfloat *);
#define glMap1f _ptrc_glMap1f
extern void (CODEGEN_FUNCPTR *_ptrc_glMap2d)(GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, const GLdouble *);
#define glMap2d _ptrc_glMap2d
extern void (CODEGEN_FUNCPTR *_ptrc_glMap2f)(GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, const GLfloat *);
#define glMap2f _ptrc_glMap2f
extern void (CODEGEN_FUNCPTR *_ptrc_glMapGrid1d)(GLint, GLdouble, GLdouble);
#define glMapGrid1d _ptrc_glMapGrid1d
extern void (CODEGEN_FUNCPTR *_ptrc_glMapGrid1f)(GLint, GLfloat, GLfloat);
#define glMapGrid1f _ptrc_glMapGrid1f
extern void (CODEGEN_FUNCPTR *_ptrc_glMapGrid2d)(GLint, GLdouble, GLdouble, GLint, GLdouble, GLdouble);
#define glMapGrid2d _ptrc_glMapGrid2d
extern void (CODEGEN_FUNCPTR *_ptrc_glMapGrid2f)(GLint, GLfloat, GLfloat, GLint, GLfloat, GLfloat);
#define glMapGrid2f _ptrc_glMapGrid2f
extern void (CODEGEN_FUNCPTR *_ptrc_glMaterialf)(GLenum, GLenum, GLfloat);
#define glMaterialf _ptrc_glMaterialf
extern void (CODEGEN_FUNCPTR *_ptrc_glMaterialfv)(GLenum, GLenum, const GLfloat *);
#define glMaterialfv _ptrc_glMaterialfv
extern void (CODEGEN_FUNCPTR *_ptrc_glMateriali)(GLenum, GLenum, GLint);
#define glMateriali _ptrc_glMateriali
extern void (CODEGEN_FUNCPTR *_ptrc_glMaterialiv)(GLenum, GLenum, const GLint *);
#define glMaterialiv _ptrc_glMaterialiv
extern void (CODEGEN_FUNCPTR *_ptrc_glMatrixMode)(GLenum);
#define glMatrixMode _ptrc_glMatrixMode
extern void (CODEGEN_FUNCPTR *_ptrc_glMultMatrixd)(const GLdouble *);
#define glMultMatrixd _ptrc_glMultMatrixd
extern void (CODEGEN_FUNCPTR *_ptrc_glMultMatrixf)(const GLfloat *);
#define glMultMatrixf _ptrc_glMultMatrixf
extern void (CODEGEN_FUNCPTR *_ptrc_glNewList)(GLuint, GLenum);
#define glNewList _ptrc_glNewList
extern void (CODEGEN_FUNCPTR *_ptrc_glNormal3b)(GLbyte, GLbyte, GLbyte);
#define glNormal3b _ptrc_glNormal3b
extern void (CODEGEN_FUNCPTR *_ptrc_glNormal3bv)(const GLbyte *);
#define glNormal3bv _ptrc_glNormal3bv
extern void (CODEGEN_FUNCPTR *_ptrc_glNormal3d)(GLdouble, GLdouble, GLdouble);
#define glNormal3d _ptrc_glNormal3d
extern void (CODEGEN_FUNCPTR *_ptrc_glNormal3dv)(const GLdouble *);
#define glNormal3dv _ptrc_glNormal3dv
extern void (CODEGEN_FUNCPTR *_ptrc_glNormal3f)(GLfloat, GLfloat, GLfloat);
#define glNormal3f _ptrc_glNormal3f
extern void (CODEGEN_FUNCPTR *_ptrc_glNormal3fv)(const GLfloat *);
#define glNormal3fv _ptrc_glNormal3fv
extern void (CODEGEN_FUNCPTR *_ptrc_glNormal3i)(GLint, GLint, GLint);
#define glNormal3i _ptrc_glNormal3i
extern void (CODEGEN_FUNCPTR *_ptrc_glNormal3iv)(const GLint *);
#define glNormal3iv _ptrc_glNormal3iv
extern void (CODEGEN_FUNCPTR *_ptrc_glNormal3s)(GLshort, GLshort, GLshort);
#define glNormal3s _ptrc_glNormal3s
extern void (CODEGEN_FUNCPTR *_ptrc_glNormal3sv)(const GLshort *);
#define glNormal3sv _ptrc_glNormal3sv
extern void (CODEGEN_FUNCPTR *_ptrc_glOrtho)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
#define glOrtho _ptrc_glOrtho
extern void (CODEGEN_FUNCPTR *_ptrc_glPassThrough)(GLfloat);
#define glPassThrough _ptrc_glPassThrough
extern void (CODEGEN_FUNCPTR *_ptrc_glPixelMapfv)(GLenum, GLsizei, const GLfloat *);
#define glPixelMapfv _ptrc_glPixelMapfv
extern void (CODEGEN_FUNCPTR *_ptrc_glPixelMapuiv)(GLenum, GLsizei, const GLuint *);
#define glPixelMapuiv _ptrc_glPixelMapuiv
extern void (CODEGEN_FUNCPTR *_ptrc_glPixelMapusv)(GLenum, GLsizei, const GLushort *);
#define glPixelMapusv _ptrc_glPixelMapusv
extern void (CODEGEN_FUNCPTR *_ptrc_glPixelStoref)(GLenum, GLfloat);
#define glPixelStoref _ptrc_glPixelStoref
extern void (CODEGEN_FUNCPTR *_ptrc_glPixelStorei)(GLenum, GLint);
#define glPixelStorei _ptrc_glPixelStorei
extern void (CODEGEN_FUNCPTR *_ptrc_glPixelTransferf)(GLenum, GLfloat);
#define glPixelTransferf _ptrc_glPixelTransferf
extern void (CODEGEN_FUNCPTR *_ptrc_glPixelTransferi)(GLenum, GLint);
#define glPixelTransferi _ptrc_glPixelTransferi
extern void (CODEGEN_FUNCPTR *_ptrc_glPixelZoom)(GLfloat, GLfloat);
#define glPixelZoom _ptrc_glPixelZoom
extern void (CODEGEN_FUNCPTR *_ptrc_glPointSize)(GLfloat);
#define glPointSize _ptrc_glPointSize
extern void (CODEGEN_FUNCPTR *_ptrc_glPolygonMode)(GLenum, GLenum);
#define glPolygonMode _ptrc_glPolygonMode
extern void (CODEGEN_FUNCPTR *_ptrc_glPolygonStipple)(const GLubyte *);
#define glPolygonStipple _ptrc_glPolygonStipple
extern void (CODEGEN_FUNCPTR *_ptrc_glPopAttrib)();
#define glPopAttrib _ptrc_glPopAttrib
extern void (CODEGEN_FUNCPTR *_ptrc_glPopMatrix)();
#define glPopMatrix _ptrc_glPopMatrix
extern void (CODEGEN_FUNCPTR *_ptrc_glPopName)();
#define glPopName _ptrc_glPopName
extern void (CODEGEN_FUNCPTR *_ptrc_glPushAttrib)(GLbitfield);
#define glPushAttrib _ptrc_glPushAttrib
extern void (CODEGEN_FUNCPTR *_ptrc_glPushMatrix)();
#define glPushMatrix _ptrc_glPushMatrix
extern void (CODEGEN_FUNCPTR *_ptrc_glPushName)(GLuint);
#define glPushName _ptrc_glPushName
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos2d)(GLdouble, GLdouble);
#define glRasterPos2d _ptrc_glRasterPos2d
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos2dv)(const GLdouble *);
#define glRasterPos2dv _ptrc_glRasterPos2dv
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos2f)(GLfloat, GLfloat);
#define glRasterPos2f _ptrc_glRasterPos2f
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos2fv)(const GLfloat *);
#define glRasterPos2fv _ptrc_glRasterPos2fv
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos2i)(GLint, GLint);
#define glRasterPos2i _ptrc_glRasterPos2i
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos2iv)(const GLint *);
#define glRasterPos2iv _ptrc_glRasterPos2iv
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos2s)(GLshort, GLshort);
#define glRasterPos2s _ptrc_glRasterPos2s
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos2sv)(const GLshort *);
#define glRasterPos2sv _ptrc_glRasterPos2sv
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos3d)(GLdouble, GLdouble, GLdouble);
#define glRasterPos3d _ptrc_glRasterPos3d
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos3dv)(const GLdouble *);
#define glRasterPos3dv _ptrc_glRasterPos3dv
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos3f)(GLfloat, GLfloat, GLfloat);
#define glRasterPos3f _ptrc_glRasterPos3f
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos3fv)(const GLfloat *);
#define glRasterPos3fv _ptrc_glRasterPos3fv
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos3i)(GLint, GLint, GLint);
#define glRasterPos3i _ptrc_glRasterPos3i
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos3iv)(const GLint *);
#define glRasterPos3iv _ptrc_glRasterPos3iv
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos3s)(GLshort, GLshort, GLshort);
#define glRasterPos3s _ptrc_glRasterPos3s
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos3sv)(const GLshort *);
#define glRasterPos3sv _ptrc_glRasterPos3sv
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos4d)(GLdouble, GLdouble, GLdouble, GLdouble);
#define glRasterPos4d _ptrc_glRasterPos4d
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos4dv)(const GLdouble *);
#define glRasterPos4dv _ptrc_glRasterPos4dv
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos4f)(GLfloat, GLfloat, GLfloat, GLfloat);
#define glRasterPos4f _ptrc_glRasterPos4f
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos4fv)(const GLfloat *);
#define glRasterPos4fv _ptrc_glRasterPos4fv
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos4i)(GLint, GLint, GLint, GLint);
#define glRasterPos4i _ptrc_glRasterPos4i
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos4iv)(const GLint *);
#define glRasterPos4iv _ptrc_glRasterPos4iv
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos4s)(GLshort, GLshort, GLshort, GLshort);
#define glRasterPos4s _ptrc_glRasterPos4s
extern void (CODEGEN_FUNCPTR *_ptrc_glRasterPos4sv)(const GLshort *);
#define glRasterPos4sv _ptrc_glRasterPos4sv
extern void (CODEGEN_FUNCPTR *_ptrc_glReadBuffer)(GLenum);
#define glReadBuffer _ptrc_glReadBuffer
extern void (CODEGEN_FUNCPTR *_ptrc_glReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid *);
#define glReadPixels _ptrc_glReadPixels
extern void (CODEGEN_FUNCPTR *_ptrc_glRectd)(GLdouble, GLdouble, GLdouble, GLdouble);
#define glRectd _ptrc_glRectd
extern void (CODEGEN_FUNCPTR *_ptrc_glRectdv)(const GLdouble *, const GLdouble *);
#define glRectdv _ptrc_glRectdv
extern void (CODEGEN_FUNCPTR *_ptrc_glRectf)(GLfloat, GLfloat, GLfloat, GLfloat);
#define glRectf _ptrc_glRectf
extern void (CODEGEN_FUNCPTR *_ptrc_glRectfv)(const GLfloat *, const GLfloat *);
#define glRectfv _ptrc_glRectfv
extern void (CODEGEN_FUNCPTR *_ptrc_glRecti)(GLint, GLint, GLint, GLint);
#define glRecti _ptrc_glRecti
extern void (CODEGEN_FUNCPTR *_ptrc_glRectiv)(const GLint *, const GLint *);
#define glRectiv _ptrc_glRectiv
extern void (CODEGEN_FUNCPTR *_ptrc_glRects)(GLshort, GLshort, GLshort, GLshort);
#define glRects _ptrc_glRects
extern void (CODEGEN_FUNCPTR *_ptrc_glRectsv)(const GLshort *, const GLshort *);
#define glRectsv _ptrc_glRectsv
extern GLint (CODEGEN_FUNCPTR *_ptrc_glRenderMode)(GLenum);
#define glRenderMode _ptrc_glRenderMode
extern void (CODEGEN_FUNCPTR *_ptrc_glRotated)(GLdouble, GLdouble, GLdouble, GLdouble);
#define glRotated _ptrc_glRotated
extern void (CODEGEN_FUNCPTR *_ptrc_glRotatef)(GLfloat, GLfloat, GLfloat, GLfloat);
#define glRotatef _ptrc_glRotatef
extern void (CODEGEN_FUNCPTR *_ptrc_glScaled)(GLdouble, GLdouble, GLdouble);
#define glScaled _ptrc_glScaled
extern void (CODEGEN_FUNCPTR *_ptrc_glScalef)(GLfloat, GLfloat, GLfloat);
#define glScalef _ptrc_glScalef
extern void (CODEGEN_FUNCPTR *_ptrc_glScissor)(GLint, GLint, GLsizei, GLsizei);
#define glScissor _ptrc_glScissor
extern void (CODEGEN_FUNCPTR *_ptrc_glSelectBuffer)(GLsizei, GLuint *);
#define glSelectBuffer _ptrc_glSelectBuffer
extern void (CODEGEN_FUNCPTR *_ptrc_glShadeModel)(GLenum);
#define glShadeModel _ptrc_glShadeModel
extern void (CODEGEN_FUNCPTR *_ptrc_glStencilFunc)(GLenum, GLint, GLuint);
#define glStencilFunc _ptrc_glStencilFunc
extern void (CODEGEN_FUNCPTR *_ptrc_glStencilMask)(GLuint);
#define glStencilMask _ptrc_glStencilMask
extern void (CODEGEN_FUNCPTR *_ptrc_glStencilOp)(GLenum, GLenum, GLenum);
#define glStencilOp _ptrc_glStencilOp
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord1d)(GLdouble);
#define glTexCoord1d _ptrc_glTexCoord1d
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord1dv)(const GLdouble *);
#define glTexCoord1dv _ptrc_glTexCoord1dv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord1f)(GLfloat);
#define glTexCoord1f _ptrc_glTexCoord1f
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord1fv)(const GLfloat *);
#define glTexCoord1fv _ptrc_glTexCoord1fv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord1i)(GLint);
#define glTexCoord1i _ptrc_glTexCoord1i
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord1iv)(const GLint *);
#define glTexCoord1iv _ptrc_glTexCoord1iv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord1s)(GLshort);
#define glTexCoord1s _ptrc_glTexCoord1s
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord1sv)(const GLshort *);
#define glTexCoord1sv _ptrc_glTexCoord1sv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord2d)(GLdouble, GLdouble);
#define glTexCoord2d _ptrc_glTexCoord2d
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord2dv)(const GLdouble *);
#define glTexCoord2dv _ptrc_glTexCoord2dv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord2f)(GLfloat, GLfloat);
#define glTexCoord2f _ptrc_glTexCoord2f
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord2fv)(const GLfloat *);
#define glTexCoord2fv _ptrc_glTexCoord2fv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord2i)(GLint, GLint);
#define glTexCoord2i _ptrc_glTexCoord2i
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord2iv)(const GLint *);
#define glTexCoord2iv _ptrc_glTexCoord2iv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord2s)(GLshort, GLshort);
#define glTexCoord2s _ptrc_glTexCoord2s
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord2sv)(const GLshort *);
#define glTexCoord2sv _ptrc_glTexCoord2sv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord3d)(GLdouble, GLdouble, GLdouble);
#define glTexCoord3d _ptrc_glTexCoord3d
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord3dv)(const GLdouble *);
#define glTexCoord3dv _ptrc_glTexCoord3dv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord3f)(GLfloat, GLfloat, GLfloat);
#define glTexCoord3f _ptrc_glTexCoord3f
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord3fv)(const GLfloat *);
#define glTexCoord3fv _ptrc_glTexCoord3fv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord3i)(GLint, GLint, GLint);
#define glTexCoord3i _ptrc_glTexCoord3i
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord3iv)(const GLint *);
#define glTexCoord3iv _ptrc_glTexCoord3iv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord3s)(GLshort, GLshort, GLshort);
#define glTexCoord3s _ptrc_glTexCoord3s
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord3sv)(const GLshort *);
#define glTexCoord3sv _ptrc_glTexCoord3sv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord4d)(GLdouble, GLdouble, GLdouble, GLdouble);
#define glTexCoord4d _ptrc_glTexCoord4d
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord4dv)(const GLdouble *);
#define glTexCoord4dv _ptrc_glTexCoord4dv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord4f)(GLfloat, GLfloat, GLfloat, GLfloat);
#define glTexCoord4f _ptrc_glTexCoord4f
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord4fv)(const GLfloat *);
#define glTexCoord4fv _ptrc_glTexCoord4fv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord4i)(GLint, GLint, GLint, GLint);
#define glTexCoord4i _ptrc_glTexCoord4i
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord4iv)(const GLint *);
#define glTexCoord4iv _ptrc_glTexCoord4iv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord4s)(GLshort, GLshort, GLshort, GLshort);
#define glTexCoord4s _ptrc_glTexCoord4s
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoord4sv)(const GLshort *);
#define glTexCoord4sv _ptrc_glTexCoord4sv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexEnvf)(GLenum, GLenum, GLfloat);
#define glTexEnvf _ptrc_glTexEnvf
extern void (CODEGEN_FUNCPTR *_ptrc_glTexEnvfv)(GLenum, GLenum, const GLfloat *);
#define glTexEnvfv _ptrc_glTexEnvfv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexEnvi)(GLenum, GLenum, GLint);
#define glTexEnvi _ptrc_glTexEnvi
extern void (CODEGEN_FUNCPTR *_ptrc_glTexEnviv)(GLenum, GLenum, const GLint *);
#define glTexEnviv _ptrc_glTexEnviv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexGend)(GLenum, GLenum, GLdouble);
#define glTexGend _ptrc_glTexGend
extern void (CODEGEN_FUNCPTR *_ptrc_glTexGendv)(GLenum, GLenum, const GLdouble *);
#define glTexGendv _ptrc_glTexGendv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexGenf)(GLenum, GLenum, GLfloat);
#define glTexGenf _ptrc_glTexGenf
extern void (CODEGEN_FUNCPTR *_ptrc_glTexGenfv)(GLenum, GLenum, const GLfloat *);
#define glTexGenfv _ptrc_glTexGenfv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexGeni)(GLenum, GLenum, GLint);
#define glTexGeni _ptrc_glTexGeni
extern void (CODEGEN_FUNCPTR *_ptrc_glTexGeniv)(GLenum, GLenum, const GLint *);
#define glTexGeniv _ptrc_glTexGeniv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexImage1D)(GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
#define glTexImage1D _ptrc_glTexImage1D
extern void (CODEGEN_FUNCPTR *_ptrc_glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
#define glTexImage2D _ptrc_glTexImage2D
extern void (CODEGEN_FUNCPTR *_ptrc_glTexParameterf)(GLenum, GLenum, GLfloat);
#define glTexParameterf _ptrc_glTexParameterf
extern void (CODEGEN_FUNCPTR *_ptrc_glTexParameterfv)(GLenum, GLenum, const GLfloat *);
#define glTexParameterfv _ptrc_glTexParameterfv
extern void (CODEGEN_FUNCPTR *_ptrc_glTexParameteri)(GLenum, GLenum, GLint);
#define glTexParameteri _ptrc_glTexParameteri
extern void (CODEGEN_FUNCPTR *_ptrc_glTexParameteriv)(GLenum, GLenum, const GLint *);
#define glTexParameteriv _ptrc_glTexParameteriv
extern void (CODEGEN_FUNCPTR *_ptrc_glTranslated)(GLdouble, GLdouble, GLdouble);
#define glTranslated _ptrc_glTranslated
extern void (CODEGEN_FUNCPTR *_ptrc_glTranslatef)(GLfloat, GLfloat, GLfloat);
#define glTranslatef _ptrc_glTranslatef
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex2d)(GLdouble, GLdouble);
#define glVertex2d _ptrc_glVertex2d
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex2dv)(const GLdouble *);
#define glVertex2dv _ptrc_glVertex2dv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex2f)(GLfloat, GLfloat);
#define glVertex2f _ptrc_glVertex2f
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex2fv)(const GLfloat *);
#define glVertex2fv _ptrc_glVertex2fv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex2i)(GLint, GLint);
#define glVertex2i _ptrc_glVertex2i
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex2iv)(const GLint *);
#define glVertex2iv _ptrc_glVertex2iv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex2s)(GLshort, GLshort);
#define glVertex2s _ptrc_glVertex2s
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex2sv)(const GLshort *);
#define glVertex2sv _ptrc_glVertex2sv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex3d)(GLdouble, GLdouble, GLdouble);
#define glVertex3d _ptrc_glVertex3d
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex3dv)(const GLdouble *);
#define glVertex3dv _ptrc_glVertex3dv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex3f)(GLfloat, GLfloat, GLfloat);
#define glVertex3f _ptrc_glVertex3f
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex3fv)(const GLfloat *);
#define glVertex3fv _ptrc_glVertex3fv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex3i)(GLint, GLint, GLint);
#define glVertex3i _ptrc_glVertex3i
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex3iv)(const GLint *);
#define glVertex3iv _ptrc_glVertex3iv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex3s)(GLshort, GLshort, GLshort);
#define glVertex3s _ptrc_glVertex3s
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex3sv)(const GLshort *);
#define glVertex3sv _ptrc_glVertex3sv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex4d)(GLdouble, GLdouble, GLdouble, GLdouble);
#define glVertex4d _ptrc_glVertex4d
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex4dv)(const GLdouble *);
#define glVertex4dv _ptrc_glVertex4dv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex4f)(GLfloat, GLfloat, GLfloat, GLfloat);
#define glVertex4f _ptrc_glVertex4f
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex4fv)(const GLfloat *);
#define glVertex4fv _ptrc_glVertex4fv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex4i)(GLint, GLint, GLint, GLint);
#define glVertex4i _ptrc_glVertex4i
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex4iv)(const GLint *);
#define glVertex4iv _ptrc_glVertex4iv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex4s)(GLshort, GLshort, GLshort, GLshort);
#define glVertex4s _ptrc_glVertex4s
extern void (CODEGEN_FUNCPTR *_ptrc_glVertex4sv)(const GLshort *);
#define glVertex4sv _ptrc_glVertex4sv
extern void (CODEGEN_FUNCPTR *_ptrc_glViewport)(GLint, GLint, GLsizei, GLsizei);
#define glViewport _ptrc_glViewport

extern GLboolean (CODEGEN_FUNCPTR *_ptrc_glAreTexturesResident)(GLsizei, const GLuint *, GLboolean *);
#define glAreTexturesResident _ptrc_glAreTexturesResident
extern void (CODEGEN_FUNCPTR *_ptrc_glArrayElement)(GLint);
#define glArrayElement _ptrc_glArrayElement
extern void (CODEGEN_FUNCPTR *_ptrc_glBindTexture)(GLenum, GLuint);
#define glBindTexture _ptrc_glBindTexture
extern void (CODEGEN_FUNCPTR *_ptrc_glColorPointer)(GLint, GLenum, GLsizei, const GLvoid *);
#define glColorPointer _ptrc_glColorPointer
extern void (CODEGEN_FUNCPTR *_ptrc_glCopyTexImage1D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint);
#define glCopyTexImage1D _ptrc_glCopyTexImage1D
extern void (CODEGEN_FUNCPTR *_ptrc_glCopyTexImage2D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
#define glCopyTexImage2D _ptrc_glCopyTexImage2D
extern void (CODEGEN_FUNCPTR *_ptrc_glCopyTexSubImage1D)(GLenum, GLint, GLint, GLint, GLint, GLsizei);
#define glCopyTexSubImage1D _ptrc_glCopyTexSubImage1D
extern void (CODEGEN_FUNCPTR *_ptrc_glCopyTexSubImage2D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
#define glCopyTexSubImage2D _ptrc_glCopyTexSubImage2D
extern void (CODEGEN_FUNCPTR *_ptrc_glDeleteTextures)(GLsizei, const GLuint *);
#define glDeleteTextures _ptrc_glDeleteTextures
extern void (CODEGEN_FUNCPTR *_ptrc_glDisableClientState)(GLenum);
#define glDisableClientState _ptrc_glDisableClientState
extern void (CODEGEN_FUNCPTR *_ptrc_glDrawArrays)(GLenum, GLint, GLsizei);
#define glDrawArrays _ptrc_glDrawArrays
extern void (CODEGEN_FUNCPTR *_ptrc_glDrawElements)(GLenum, GLsizei, GLenum, const GLvoid *);
#define glDrawElements _ptrc_glDrawElements
extern void (CODEGEN_FUNCPTR *_ptrc_glEdgeFlagPointer)(GLsizei, const GLvoid *);
#define glEdgeFlagPointer _ptrc_glEdgeFlagPointer
extern void (CODEGEN_FUNCPTR *_ptrc_glEnableClientState)(GLenum);
#define glEnableClientState _ptrc_glEnableClientState
extern void (CODEGEN_FUNCPTR *_ptrc_glGenTextures)(GLsizei, GLuint *);
#define glGenTextures _ptrc_glGenTextures
extern void (CODEGEN_FUNCPTR *_ptrc_glGetPointerv)(GLenum, GLvoid **);
#define glGetPointerv _ptrc_glGetPointerv
extern void (CODEGEN_FUNCPTR *_ptrc_glIndexPointer)(GLenum, GLsizei, const GLvoid *);
#define glIndexPointer _ptrc_glIndexPointer
extern void (CODEGEN_FUNCPTR *_ptrc_glIndexub)(GLubyte);
#define glIndexub _ptrc_glIndexub
extern void (CODEGEN_FUNCPTR *_ptrc_glIndexubv)(const GLubyte *);
#define glIndexubv _ptrc_glIndexubv
extern void (CODEGEN_FUNCPTR *_ptrc_glInterleavedArrays)(GLenum, GLsizei, const GLvoid *);
#define glInterleavedArrays _ptrc_glInterleavedArrays
extern GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsTexture)(GLuint);
#define glIsTexture _ptrc_glIsTexture
extern void (CODEGEN_FUNCPTR *_ptrc_glNormalPointer)(GLenum, GLsizei, const GLvoid *);
#define glNormalPointer _ptrc_glNormalPointer
extern void (CODEGEN_FUNCPTR *_ptrc_glPolygonOffset)(GLfloat, GLfloat);
#define glPolygonOffset _ptrc_glPolygonOffset
extern void (CODEGEN_FUNCPTR *_ptrc_glPopClientAttrib)();
#define glPopClientAttrib _ptrc_glPopClientAttrib
extern void (CODEGEN_FUNCPTR *_ptrc_glPrioritizeTextures)(GLsizei, const GLuint *, const GLfloat *);
#define glPrioritizeTextures _ptrc_glPrioritizeTextures
extern void (CODEGEN_FUNCPTR *_ptrc_glPushClientAttrib)(GLbitfield);
#define glPushClientAttrib _ptrc_glPushClientAttrib
extern void (CODEGEN_FUNCPTR *_ptrc_glTexCoordPointer)(GLint, GLenum, GLsizei, const GLvoid *);
#define glTexCoordPointer _ptrc_glTexCoordPointer
extern void (CODEGEN_FUNCPTR *_ptrc_glTexSubImage1D)(GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const GLvoid *);
#define glTexSubImage1D _ptrc_glTexSubImage1D
extern void (CODEGEN_FUNCPTR *_ptrc_glTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
#define glTexSubImage2D _ptrc_glTexSubImage2D
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexPointer)(GLint, GLenum, GLsizei, const GLvoid *);
#define glVertexPointer _ptrc_glVertexPointer

extern void (CODEGEN_FUNCPTR *_ptrc_glBlendColor)(GLfloat, GLfloat, GLfloat, GLfloat);
#define glBlendColor _ptrc_glBlendColor
extern void (CODEGEN_FUNCPTR *_ptrc_glBlendEquation)(GLenum);
#define glBlendEquation _ptrc_glBlendEquation
extern void (CODEGEN_FUNCPTR *_ptrc_glCopyTexSubImage3D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
#define glCopyTexSubImage3D _ptrc_glCopyTexSubImage3D
extern void (CODEGEN_FUNCPTR *_ptrc_glDrawRangeElements)(GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *);
#define glDrawRangeElements _ptrc_glDrawRangeElements
extern void (CODEGEN_FUNCPTR *_ptrc_glTexImage3D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
#define glTexImage3D _ptrc_glTexImage3D
extern void (CODEGEN_FUNCPTR *_ptrc_glTexSubImage3D)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
#define glTexSubImage3D _ptrc_glTexSubImage3D

extern void (CODEGEN_FUNCPTR *_ptrc_glActiveTexture)(GLenum);
#define glActiveTexture _ptrc_glActiveTexture
extern void (CODEGEN_FUNCPTR *_ptrc_glClientActiveTexture)(GLenum);
#define glClientActiveTexture _ptrc_glClientActiveTexture
extern void (CODEGEN_FUNCPTR *_ptrc_glCompressedTexImage1D)(GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *);
#define glCompressedTexImage1D _ptrc_glCompressedTexImage1D
extern void (CODEGEN_FUNCPTR *_ptrc_glCompressedTexImage2D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
#define glCompressedTexImage2D _ptrc_glCompressedTexImage2D
extern void (CODEGEN_FUNCPTR *_ptrc_glCompressedTexImage3D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
#define glCompressedTexImage3D _ptrc_glCompressedTexImage3D
extern void (CODEGEN_FUNCPTR *_ptrc_glCompressedTexSubImage1D)(GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *);
#define glCompressedTexSubImage1D _ptrc_glCompressedTexSubImage1D
extern void (CODEGEN_FUNCPTR *_ptrc_glCompressedTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
#define glCompressedTexSubImage2D _ptrc_glCompressedTexSubImage2D
extern void (CODEGEN_FUNCPTR *_ptrc_glCompressedTexSubImage3D)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
#define glCompressedTexSubImage3D _ptrc_glCompressedTexSubImage3D
extern void (CODEGEN_FUNCPTR *_ptrc_glGetCompressedTexImage)(GLenum, GLint, GLvoid *);
#define glGetCompressedTexImage _ptrc_glGetCompressedTexImage
extern void (CODEGEN_FUNCPTR *_ptrc_glLoadTransposeMatrixd)(const GLdouble *);
#define glLoadTransposeMatrixd _ptrc_glLoadTransposeMatrixd
extern void (CODEGEN_FUNCPTR *_ptrc_glLoadTransposeMatrixf)(const GLfloat *);
#define glLoadTransposeMatrixf _ptrc_glLoadTransposeMatrixf
extern void (CODEGEN_FUNCPTR *_ptrc_glMultTransposeMatrixd)(const GLdouble *);
#define glMultTransposeMatrixd _ptrc_glMultTransposeMatrixd
extern void (CODEGEN_FUNCPTR *_ptrc_glMultTransposeMatrixf)(const GLfloat *);
#define glMultTransposeMatrixf _ptrc_glMultTransposeMatrixf
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord1d)(GLenum, GLdouble);
#define glMultiTexCoord1d _ptrc_glMultiTexCoord1d
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord1dv)(GLenum, const GLdouble *);
#define glMultiTexCoord1dv _ptrc_glMultiTexCoord1dv
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord1f)(GLenum, GLfloat);
#define glMultiTexCoord1f _ptrc_glMultiTexCoord1f
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord1fv)(GLenum, const GLfloat *);
#define glMultiTexCoord1fv _ptrc_glMultiTexCoord1fv
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord1i)(GLenum, GLint);
#define glMultiTexCoord1i _ptrc_glMultiTexCoord1i
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord1iv)(GLenum, const GLint *);
#define glMultiTexCoord1iv _ptrc_glMultiTexCoord1iv
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord1s)(GLenum, GLshort);
#define glMultiTexCoord1s _ptrc_glMultiTexCoord1s
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord1sv)(GLenum, const GLshort *);
#define glMultiTexCoord1sv _ptrc_glMultiTexCoord1sv
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord2d)(GLenum, GLdouble, GLdouble);
#define glMultiTexCoord2d _ptrc_glMultiTexCoord2d
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord2dv)(GLenum, const GLdouble *);
#define glMultiTexCoord2dv _ptrc_glMultiTexCoord2dv
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord2f)(GLenum, GLfloat, GLfloat);
#define glMultiTexCoord2f _ptrc_glMultiTexCoord2f
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord2fv)(GLenum, const GLfloat *);
#define glMultiTexCoord2fv _ptrc_glMultiTexCoord2fv
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord2i)(GLenum, GLint, GLint);
#define glMultiTexCoord2i _ptrc_glMultiTexCoord2i
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord2iv)(GLenum, const GLint *);
#define glMultiTexCoord2iv _ptrc_glMultiTexCoord2iv
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord2s)(GLenum, GLshort, GLshort);
#define glMultiTexCoord2s _ptrc_glMultiTexCoord2s
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord2sv)(GLenum, const GLshort *);
#define glMultiTexCoord2sv _ptrc_glMultiTexCoord2sv
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord3d)(GLenum, GLdouble, GLdouble, GLdouble);
#define glMultiTexCoord3d _ptrc_glMultiTexCoord3d
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord3dv)(GLenum, const GLdouble *);
#define glMultiTexCoord3dv _ptrc_glMultiTexCoord3dv
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord3f)(GLenum, GLfloat, GLfloat, GLfloat);
#define glMultiTexCoord3f _ptrc_glMultiTexCoord3f
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord3fv)(GLenum, const GLfloat *);
#define glMultiTexCoord3fv _ptrc_glMultiTexCoord3fv
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord3i)(GLenum, GLint, GLint, GLint);
#define glMultiTexCoord3i _ptrc_glMultiTexCoord3i
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord3iv)(GLenum, const GLint *);
#define glMultiTexCoord3iv _ptrc_glMultiTexCoord3iv
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord3s)(GLenum, GLshort, GLshort, GLshort);
#define glMultiTexCoord3s _ptrc_glMultiTexCoord3s
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord3sv)(GLenum, const GLshort *);
#define glMultiTexCoord3sv _ptrc_glMultiTexCoord3sv
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord4d)(GLenum, GLdouble, GLdouble, GLdouble, GLdouble);
#define glMultiTexCoord4d _ptrc_glMultiTexCoord4d
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord4dv)(GLenum, const GLdouble *);
#define glMultiTexCoord4dv _ptrc_glMultiTexCoord4dv
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord4f)(GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
#define glMultiTexCoord4f _ptrc_glMultiTexCoord4f
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord4fv)(GLenum, const GLfloat *);
#define glMultiTexCoord4fv _ptrc_glMultiTexCoord4fv
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord4i)(GLenum, GLint, GLint, GLint, GLint);
#define glMultiTexCoord4i _ptrc_glMultiTexCoord4i
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord4iv)(GLenum, const GLint *);
#define glMultiTexCoord4iv _ptrc_glMultiTexCoord4iv
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord4s)(GLenum, GLshort, GLshort, GLshort, GLshort);
#define glMultiTexCoord4s _ptrc_glMultiTexCoord4s
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiTexCoord4sv)(GLenum, const GLshort *);
#define glMultiTexCoord4sv _ptrc_glMultiTexCoord4sv
extern void (CODEGEN_FUNCPTR *_ptrc_glSampleCoverage)(GLfloat, GLboolean);
#define glSampleCoverage _ptrc_glSampleCoverage

extern void (CODEGEN_FUNCPTR *_ptrc_glBlendFuncSeparate)(GLenum, GLenum, GLenum, GLenum);
#define glBlendFuncSeparate _ptrc_glBlendFuncSeparate
extern void (CODEGEN_FUNCPTR *_ptrc_glFogCoordPointer)(GLenum, GLsizei, const GLvoid *);
#define glFogCoordPointer _ptrc_glFogCoordPointer
extern void (CODEGEN_FUNCPTR *_ptrc_glFogCoordd)(GLdouble);
#define glFogCoordd _ptrc_glFogCoordd
extern void (CODEGEN_FUNCPTR *_ptrc_glFogCoorddv)(const GLdouble *);
#define glFogCoorddv _ptrc_glFogCoorddv
extern void (CODEGEN_FUNCPTR *_ptrc_glFogCoordf)(GLfloat);
#define glFogCoordf _ptrc_glFogCoordf
extern void (CODEGEN_FUNCPTR *_ptrc_glFogCoordfv)(const GLfloat *);
#define glFogCoordfv _ptrc_glFogCoordfv
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiDrawArrays)(GLenum, const GLint *, const GLsizei *, GLsizei);
#define glMultiDrawArrays _ptrc_glMultiDrawArrays
extern void (CODEGEN_FUNCPTR *_ptrc_glMultiDrawElements)(GLenum, const GLsizei *, GLenum, const GLvoid *const*, GLsizei);
#define glMultiDrawElements _ptrc_glMultiDrawElements
extern void (CODEGEN_FUNCPTR *_ptrc_glPointParameterf)(GLenum, GLfloat);
#define glPointParameterf _ptrc_glPointParameterf
extern void (CODEGEN_FUNCPTR *_ptrc_glPointParameterfv)(GLenum, const GLfloat *);
#define glPointParameterfv _ptrc_glPointParameterfv
extern void (CODEGEN_FUNCPTR *_ptrc_glPointParameteri)(GLenum, GLint);
#define glPointParameteri _ptrc_glPointParameteri
extern void (CODEGEN_FUNCPTR *_ptrc_glPointParameteriv)(GLenum, const GLint *);
#define glPointParameteriv _ptrc_glPointParameteriv
extern void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColor3b)(GLbyte, GLbyte, GLbyte);
#define glSecondaryColor3b _ptrc_glSecondaryColor3b
extern void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColor3bv)(const GLbyte *);
#define glSecondaryColor3bv _ptrc_glSecondaryColor3bv
extern void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColor3d)(GLdouble, GLdouble, GLdouble);
#define glSecondaryColor3d _ptrc_glSecondaryColor3d
extern void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColor3dv)(const GLdouble *);
#define glSecondaryColor3dv _ptrc_glSecondaryColor3dv
extern void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColor3f)(GLfloat, GLfloat, GLfloat);
#define glSecondaryColor3f _ptrc_glSecondaryColor3f
extern void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColor3fv)(const GLfloat *);
#define glSecondaryColor3fv _ptrc_glSecondaryColor3fv
extern void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColor3i)(GLint, GLint, GLint);
#define glSecondaryColor3i _ptrc_glSecondaryColor3i
extern void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColor3iv)(const GLint *);
#define glSecondaryColor3iv _ptrc_glSecondaryColor3iv
extern void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColor3s)(GLshort, GLshort, GLshort);
#define glSecondaryColor3s _ptrc_glSecondaryColor3s
extern void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColor3sv)(const GLshort *);
#define glSecondaryColor3sv _ptrc_glSecondaryColor3sv
extern void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColor3ub)(GLubyte, GLubyte, GLubyte);
#define glSecondaryColor3ub _ptrc_glSecondaryColor3ub
extern void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColor3ubv)(const GLubyte *);
#define glSecondaryColor3ubv _ptrc_glSecondaryColor3ubv
extern void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColor3ui)(GLuint, GLuint, GLuint);
#define glSecondaryColor3ui _ptrc_glSecondaryColor3ui
extern void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColor3uiv)(const GLuint *);
#define glSecondaryColor3uiv _ptrc_glSecondaryColor3uiv
extern void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColor3us)(GLushort, GLushort, GLushort);
#define glSecondaryColor3us _ptrc_glSecondaryColor3us
extern void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColor3usv)(const GLushort *);
#define glSecondaryColor3usv _ptrc_glSecondaryColor3usv
extern void (CODEGEN_FUNCPTR *_ptrc_glSecondaryColorPointer)(GLint, GLenum, GLsizei, const GLvoid *);
#define glSecondaryColorPointer _ptrc_glSecondaryColorPointer
extern void (CODEGEN_FUNCPTR *_ptrc_glWindowPos2d)(GLdouble, GLdouble);
#define glWindowPos2d _ptrc_glWindowPos2d
extern void (CODEGEN_FUNCPTR *_ptrc_glWindowPos2dv)(const GLdouble *);
#define glWindowPos2dv _ptrc_glWindowPos2dv
extern void (CODEGEN_FUNCPTR *_ptrc_glWindowPos2f)(GLfloat, GLfloat);
#define glWindowPos2f _ptrc_glWindowPos2f
extern void (CODEGEN_FUNCPTR *_ptrc_glWindowPos2fv)(const GLfloat *);
#define glWindowPos2fv _ptrc_glWindowPos2fv
extern void (CODEGEN_FUNCPTR *_ptrc_glWindowPos2i)(GLint, GLint);
#define glWindowPos2i _ptrc_glWindowPos2i
extern void (CODEGEN_FUNCPTR *_ptrc_glWindowPos2iv)(const GLint *);
#define glWindowPos2iv _ptrc_glWindowPos2iv
extern void (CODEGEN_FUNCPTR *_ptrc_glWindowPos2s)(GLshort, GLshort);
#define glWindowPos2s _ptrc_glWindowPos2s
extern void (CODEGEN_FUNCPTR *_ptrc_glWindowPos2sv)(const GLshort *);
#define glWindowPos2sv _ptrc_glWindowPos2sv
extern void (CODEGEN_FUNCPTR *_ptrc_glWindowPos3d)(GLdouble, GLdouble, GLdouble);
#define glWindowPos3d _ptrc_glWindowPos3d
extern void (CODEGEN_FUNCPTR *_ptrc_glWindowPos3dv)(const GLdouble *);
#define glWindowPos3dv _ptrc_glWindowPos3dv
extern void (CODEGEN_FUNCPTR *_ptrc_glWindowPos3f)(GLfloat, GLfloat, GLfloat);
#define glWindowPos3f _ptrc_glWindowPos3f
extern void (CODEGEN_FUNCPTR *_ptrc_glWindowPos3fv)(const GLfloat *);
#define glWindowPos3fv _ptrc_glWindowPos3fv
extern void (CODEGEN_FUNCPTR *_ptrc_glWindowPos3i)(GLint, GLint, GLint);
#define glWindowPos3i _ptrc_glWindowPos3i
extern void (CODEGEN_FUNCPTR *_ptrc_glWindowPos3iv)(const GLint *);
#define glWindowPos3iv _ptrc_glWindowPos3iv
extern void (CODEGEN_FUNCPTR *_ptrc_glWindowPos3s)(GLshort, GLshort, GLshort);
#define glWindowPos3s _ptrc_glWindowPos3s
extern void (CODEGEN_FUNCPTR *_ptrc_glWindowPos3sv)(const GLshort *);
#define glWindowPos3sv _ptrc_glWindowPos3sv

extern void (CODEGEN_FUNCPTR *_ptrc_glBeginQuery)(GLenum, GLuint);
#define glBeginQuery _ptrc_glBeginQuery
extern void (CODEGEN_FUNCPTR *_ptrc_glBindBuffer)(GLenum, GLuint);
#define glBindBuffer _ptrc_glBindBuffer
extern void (CODEGEN_FUNCPTR *_ptrc_glBufferData)(GLenum, GLsizeiptr, const GLvoid *, GLenum);
#define glBufferData _ptrc_glBufferData
extern void (CODEGEN_FUNCPTR *_ptrc_glBufferSubData)(GLenum, GLintptr, GLsizeiptr, const GLvoid *);
#define glBufferSubData _ptrc_glBufferSubData
extern void (CODEGEN_FUNCPTR *_ptrc_glDeleteBuffers)(GLsizei, const GLuint *);
#define glDeleteBuffers _ptrc_glDeleteBuffers
extern void (CODEGEN_FUNCPTR *_ptrc_glDeleteQueries)(GLsizei, const GLuint *);
#define glDeleteQueries _ptrc_glDeleteQueries
extern void (CODEGEN_FUNCPTR *_ptrc_glEndQuery)(GLenum);
#define glEndQuery _ptrc_glEndQuery
extern void (CODEGEN_FUNCPTR *_ptrc_glGenBuffers)(GLsizei, GLuint *);
#define glGenBuffers _ptrc_glGenBuffers
extern void (CODEGEN_FUNCPTR *_ptrc_glGenQueries)(GLsizei, GLuint *);
#define glGenQueries _ptrc_glGenQueries
extern void (CODEGEN_FUNCPTR *_ptrc_glGetBufferParameteriv)(GLenum, GLenum, GLint *);
#define glGetBufferParameteriv _ptrc_glGetBufferParameteriv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetBufferPointerv)(GLenum, GLenum, GLvoid **);
#define glGetBufferPointerv _ptrc_glGetBufferPointerv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetBufferSubData)(GLenum, GLintptr, GLsizeiptr, GLvoid *);
#define glGetBufferSubData _ptrc_glGetBufferSubData
extern void (CODEGEN_FUNCPTR *_ptrc_glGetQueryObjectiv)(GLuint, GLenum, GLint *);
#define glGetQueryObjectiv _ptrc_glGetQueryObjectiv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetQueryObjectuiv)(GLuint, GLenum, GLuint *);
#define glGetQueryObjectuiv _ptrc_glGetQueryObjectuiv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetQueryiv)(GLenum, GLenum, GLint *);
#define glGetQueryiv _ptrc_glGetQueryiv
extern GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsBuffer)(GLuint);
#define glIsBuffer _ptrc_glIsBuffer
extern GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsQuery)(GLuint);
#define glIsQuery _ptrc_glIsQuery
extern void * (CODEGEN_FUNCPTR *_ptrc_glMapBuffer)(GLenum, GLenum);
#define glMapBuffer _ptrc_glMapBuffer
extern GLboolean (CODEGEN_FUNCPTR *_ptrc_glUnmapBuffer)(GLenum);
#define glUnmapBuffer _ptrc_glUnmapBuffer

extern void (CODEGEN_FUNCPTR *_ptrc_glAttachShader)(GLuint, GLuint);
#define glAttachShader _ptrc_glAttachShader
extern void (CODEGEN_FUNCPTR *_ptrc_glBindAttribLocation)(GLuint, GLuint, const GLchar *);
#define glBindAttribLocation _ptrc_glBindAttribLocation
extern void (CODEGEN_FUNCPTR *_ptrc_glBlendEquationSeparate)(GLenum, GLenum);
#define glBlendEquationSeparate _ptrc_glBlendEquationSeparate
extern void (CODEGEN_FUNCPTR *_ptrc_glCompileShader)(GLuint);
#define glCompileShader _ptrc_glCompileShader
extern GLuint (CODEGEN_FUNCPTR *_ptrc_glCreateProgram)();
#define glCreateProgram _ptrc_glCreateProgram
extern GLuint (CODEGEN_FUNCPTR *_ptrc_glCreateShader)(GLenum);
#define glCreateShader _ptrc_glCreateShader
extern void (CODEGEN_FUNCPTR *_ptrc_glDeleteProgram)(GLuint);
#define glDeleteProgram _ptrc_glDeleteProgram
extern void (CODEGEN_FUNCPTR *_ptrc_glDeleteShader)(GLuint);
#define glDeleteShader _ptrc_glDeleteShader
extern void (CODEGEN_FUNCPTR *_ptrc_glDetachShader)(GLuint, GLuint);
#define glDetachShader _ptrc_glDetachShader
extern void (CODEGEN_FUNCPTR *_ptrc_glDisableVertexAttribArray)(GLuint);
#define glDisableVertexAttribArray _ptrc_glDisableVertexAttribArray
extern void (CODEGEN_FUNCPTR *_ptrc_glDrawBuffers)(GLsizei, const GLenum *);
#define glDrawBuffers _ptrc_glDrawBuffers
extern void (CODEGEN_FUNCPTR *_ptrc_glEnableVertexAttribArray)(GLuint);
#define glEnableVertexAttribArray _ptrc_glEnableVertexAttribArray
extern void (CODEGEN_FUNCPTR *_ptrc_glGetActiveAttrib)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
#define glGetActiveAttrib _ptrc_glGetActiveAttrib
extern void (CODEGEN_FUNCPTR *_ptrc_glGetActiveUniform)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *);
#define glGetActiveUniform _ptrc_glGetActiveUniform
extern void (CODEGEN_FUNCPTR *_ptrc_glGetAttachedShaders)(GLuint, GLsizei, GLsizei *, GLuint *);
#define glGetAttachedShaders _ptrc_glGetAttachedShaders
extern GLint (CODEGEN_FUNCPTR *_ptrc_glGetAttribLocation)(GLuint, const GLchar *);
#define glGetAttribLocation _ptrc_glGetAttribLocation
extern void (CODEGEN_FUNCPTR *_ptrc_glGetProgramInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *);
#define glGetProgramInfoLog _ptrc_glGetProgramInfoLog
extern void (CODEGEN_FUNCPTR *_ptrc_glGetProgramiv)(GLuint, GLenum, GLint *);
#define glGetProgramiv _ptrc_glGetProgramiv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetShaderInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *);
#define glGetShaderInfoLog _ptrc_glGetShaderInfoLog
extern void (CODEGEN_FUNCPTR *_ptrc_glGetShaderSource)(GLuint, GLsizei, GLsizei *, GLchar *);
#define glGetShaderSource _ptrc_glGetShaderSource
extern void (CODEGEN_FUNCPTR *_ptrc_glGetShaderiv)(GLuint, GLenum, GLint *);
#define glGetShaderiv _ptrc_glGetShaderiv
extern GLint (CODEGEN_FUNCPTR *_ptrc_glGetUniformLocation)(GLuint, const GLchar *);
#define glGetUniformLocation _ptrc_glGetUniformLocation
extern void (CODEGEN_FUNCPTR *_ptrc_glGetUniformfv)(GLuint, GLint, GLfloat *);
#define glGetUniformfv _ptrc_glGetUniformfv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetUniformiv)(GLuint, GLint, GLint *);
#define glGetUniformiv _ptrc_glGetUniformiv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetVertexAttribPointerv)(GLuint, GLenum, GLvoid **);
#define glGetVertexAttribPointerv _ptrc_glGetVertexAttribPointerv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetVertexAttribdv)(GLuint, GLenum, GLdouble *);
#define glGetVertexAttribdv _ptrc_glGetVertexAttribdv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetVertexAttribfv)(GLuint, GLenum, GLfloat *);
#define glGetVertexAttribfv _ptrc_glGetVertexAttribfv
extern void (CODEGEN_FUNCPTR *_ptrc_glGetVertexAttribiv)(GLuint, GLenum, GLint *);
#define glGetVertexAttribiv _ptrc_glGetVertexAttribiv
extern GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsProgram)(GLuint);
#define glIsProgram _ptrc_glIsProgram
extern GLboolean (CODEGEN_FUNCPTR *_ptrc_glIsShader)(GLuint);
#define glIsShader _ptrc_glIsShader
extern void (CODEGEN_FUNCPTR *_ptrc_glLinkProgram)(GLuint);
#define glLinkProgram _ptrc_glLinkProgram
extern void (CODEGEN_FUNCPTR *_ptrc_glShaderSource)(GLuint, GLsizei, const GLchar *const*, const GLint *);
#define glShaderSource _ptrc_glShaderSource
extern void (CODEGEN_FUNCPTR *_ptrc_glStencilFuncSeparate)(GLenum, GLenum, GLint, GLuint);
#define glStencilFuncSeparate _ptrc_glStencilFuncSeparate
extern void (CODEGEN_FUNCPTR *_ptrc_glStencilMaskSeparate)(GLenum, GLuint);
#define glStencilMaskSeparate _ptrc_glStencilMaskSeparate
extern void (CODEGEN_FUNCPTR *_ptrc_glStencilOpSeparate)(GLenum, GLenum, GLenum, GLenum);
#define glStencilOpSeparate _ptrc_glStencilOpSeparate
extern void (CODEGEN_FUNCPTR *_ptrc_glUniform1f)(GLint, GLfloat);
#define glUniform1f _ptrc_glUniform1f
extern void (CODEGEN_FUNCPTR *_ptrc_glUniform1fv)(GLint, GLsizei, const GLfloat *);
#define glUniform1fv _ptrc_glUniform1fv
extern void (CODEGEN_FUNCPTR *_ptrc_glUniform1i)(GLint, GLint);
#define glUniform1i _ptrc_glUniform1i
extern void (CODEGEN_FUNCPTR *_ptrc_glUniform1iv)(GLint, GLsizei, const GLint *);
#define glUniform1iv _ptrc_glUniform1iv
extern void (CODEGEN_FUNCPTR *_ptrc_glUniform2f)(GLint, GLfloat, GLfloat);
#define glUniform2f _ptrc_glUniform2f
extern void (CODEGEN_FUNCPTR *_ptrc_glUniform2fv)(GLint, GLsizei, const GLfloat *);
#define glUniform2fv _ptrc_glUniform2fv
extern void (CODEGEN_FUNCPTR *_ptrc_glUniform2i)(GLint, GLint, GLint);
#define glUniform2i _ptrc_glUniform2i
extern void (CODEGEN_FUNCPTR *_ptrc_glUniform2iv)(GLint, GLsizei, const GLint *);
#define glUniform2iv _ptrc_glUniform2iv
extern void (CODEGEN_FUNCPTR *_ptrc_glUniform3f)(GLint, GLfloat, GLfloat, GLfloat);
#define glUniform3f _ptrc_glUniform3f
extern void (CODEGEN_FUNCPTR *_ptrc_glUniform3fv)(GLint, GLsizei, const GLfloat *);
#define glUniform3fv _ptrc_glUniform3fv
extern void (CODEGEN_FUNCPTR *_ptrc_glUniform3i)(GLint, GLint, GLint, GLint);
#define glUniform3i _ptrc_glUniform3i
extern void (CODEGEN_FUNCPTR *_ptrc_glUniform3iv)(GLint, GLsizei, const GLint *);
#define glUniform3iv _ptrc_glUniform3iv
extern void (CODEGEN_FUNCPTR *_ptrc_glUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
#define glUniform4f _ptrc_glUniform4f
extern void (CODEGEN_FUNCPTR *_ptrc_glUniform4fv)(GLint, GLsizei, const GLfloat *);
#define glUniform4fv _ptrc_glUniform4fv
extern void (CODEGEN_FUNCPTR *_ptrc_glUniform4i)(GLint, GLint, GLint, GLint, GLint);
#define glUniform4i _ptrc_glUniform4i
extern void (CODEGEN_FUNCPTR *_ptrc_glUniform4iv)(GLint, GLsizei, const GLint *);
#define glUniform4iv _ptrc_glUniform4iv
extern void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix2fv)(GLint, GLsizei, GLboolean, const GLfloat *);
#define glUniformMatrix2fv _ptrc_glUniformMatrix2fv
extern void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix3fv)(GLint, GLsizei, GLboolean, const GLfloat *);
#define glUniformMatrix3fv _ptrc_glUniformMatrix3fv
extern void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat *);
#define glUniformMatrix4fv _ptrc_glUniformMatrix4fv
extern void (CODEGEN_FUNCPTR *_ptrc_glUseProgram)(GLuint);
#define glUseProgram _ptrc_glUseProgram
extern void (CODEGEN_FUNCPTR *_ptrc_glValidateProgram)(GLuint);
#define glValidateProgram _ptrc_glValidateProgram
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib1d)(GLuint, GLdouble);
#define glVertexAttrib1d _ptrc_glVertexAttrib1d
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib1dv)(GLuint, const GLdouble *);
#define glVertexAttrib1dv _ptrc_glVertexAttrib1dv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib1f)(GLuint, GLfloat);
#define glVertexAttrib1f _ptrc_glVertexAttrib1f
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib1fv)(GLuint, const GLfloat *);
#define glVertexAttrib1fv _ptrc_glVertexAttrib1fv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib1s)(GLuint, GLshort);
#define glVertexAttrib1s _ptrc_glVertexAttrib1s
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib1sv)(GLuint, const GLshort *);
#define glVertexAttrib1sv _ptrc_glVertexAttrib1sv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib2d)(GLuint, GLdouble, GLdouble);
#define glVertexAttrib2d _ptrc_glVertexAttrib2d
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib2dv)(GLuint, const GLdouble *);
#define glVertexAttrib2dv _ptrc_glVertexAttrib2dv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib2f)(GLuint, GLfloat, GLfloat);
#define glVertexAttrib2f _ptrc_glVertexAttrib2f
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib2fv)(GLuint, const GLfloat *);
#define glVertexAttrib2fv _ptrc_glVertexAttrib2fv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib2s)(GLuint, GLshort, GLshort);
#define glVertexAttrib2s _ptrc_glVertexAttrib2s
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib2sv)(GLuint, const GLshort *);
#define glVertexAttrib2sv _ptrc_glVertexAttrib2sv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib3d)(GLuint, GLdouble, GLdouble, GLdouble);
#define glVertexAttrib3d _ptrc_glVertexAttrib3d
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib3dv)(GLuint, const GLdouble *);
#define glVertexAttrib3dv _ptrc_glVertexAttrib3dv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib3f)(GLuint, GLfloat, GLfloat, GLfloat);
#define glVertexAttrib3f _ptrc_glVertexAttrib3f
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib3fv)(GLuint, const GLfloat *);
#define glVertexAttrib3fv _ptrc_glVertexAttrib3fv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib3s)(GLuint, GLshort, GLshort, GLshort);
#define glVertexAttrib3s _ptrc_glVertexAttrib3s
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib3sv)(GLuint, const GLshort *);
#define glVertexAttrib3sv _ptrc_glVertexAttrib3sv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4Nbv)(GLuint, const GLbyte *);
#define glVertexAttrib4Nbv _ptrc_glVertexAttrib4Nbv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4Niv)(GLuint, const GLint *);
#define glVertexAttrib4Niv _ptrc_glVertexAttrib4Niv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4Nsv)(GLuint, const GLshort *);
#define glVertexAttrib4Nsv _ptrc_glVertexAttrib4Nsv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4Nub)(GLuint, GLubyte, GLubyte, GLubyte, GLubyte);
#define glVertexAttrib4Nub _ptrc_glVertexAttrib4Nub
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4Nubv)(GLuint, const GLubyte *);
#define glVertexAttrib4Nubv _ptrc_glVertexAttrib4Nubv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4Nuiv)(GLuint, const GLuint *);
#define glVertexAttrib4Nuiv _ptrc_glVertexAttrib4Nuiv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4Nusv)(GLuint, const GLushort *);
#define glVertexAttrib4Nusv _ptrc_glVertexAttrib4Nusv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4bv)(GLuint, const GLbyte *);
#define glVertexAttrib4bv _ptrc_glVertexAttrib4bv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4d)(GLuint, GLdouble, GLdouble, GLdouble, GLdouble);
#define glVertexAttrib4d _ptrc_glVertexAttrib4d
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4dv)(GLuint, const GLdouble *);
#define glVertexAttrib4dv _ptrc_glVertexAttrib4dv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4f)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
#define glVertexAttrib4f _ptrc_glVertexAttrib4f
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4fv)(GLuint, const GLfloat *);
#define glVertexAttrib4fv _ptrc_glVertexAttrib4fv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4iv)(GLuint, const GLint *);
#define glVertexAttrib4iv _ptrc_glVertexAttrib4iv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4s)(GLuint, GLshort, GLshort, GLshort, GLshort);
#define glVertexAttrib4s _ptrc_glVertexAttrib4s
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4sv)(GLuint, const GLshort *);
#define glVertexAttrib4sv _ptrc_glVertexAttrib4sv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4ubv)(GLuint, const GLubyte *);
#define glVertexAttrib4ubv _ptrc_glVertexAttrib4ubv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4uiv)(GLuint, const GLuint *);
#define glVertexAttrib4uiv _ptrc_glVertexAttrib4uiv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttrib4usv)(GLuint, const GLushort *);
#define glVertexAttrib4usv _ptrc_glVertexAttrib4usv
extern void (CODEGEN_FUNCPTR *_ptrc_glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
#define glVertexAttribPointer _ptrc_glVertexAttribPointer

extern void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix2x3fv)(GLint, GLsizei, GLboolean, const GLfloat *);
#define glUniformMatrix2x3fv _ptrc_glUniformMatrix2x3fv
extern void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix2x4fv)(GLint, GLsizei, GLboolean, const GLfloat *);
#define glUniformMatrix2x4fv _ptrc_glUniformMatrix2x4fv
extern void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix3x2fv)(GLint, GLsizei, GLboolean, const GLfloat *);
#define glUniformMatrix3x2fv _ptrc_glUniformMatrix3x2fv
extern void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix3x4fv)(GLint, GLsizei, GLboolean, const GLfloat *);
#define glUniformMatrix3x4fv _ptrc_glUniformMatrix3x4fv
extern void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix4x2fv)(GLint, GLsizei, GLboolean, const GLfloat *);
#define glUniformMatrix4x2fv _ptrc_glUniformMatrix4x2fv
extern void (CODEGEN_FUNCPTR *_ptrc_glUniformMatrix4x3fv)(GLint, GLsizei, GLboolean, const GLfloat *);
#define glUniformMatrix4x3fv _ptrc_glUniformMatrix4x3fv

enum ogl_LoadStatus
{
	ogl_LOAD_FAILED = 0,
	ogl_LOAD_SUCCEEDED = 1,
};

int ogl_LoadFunctions();

int ogl_GetMinorVersion();
int ogl_GetMajorVersion();
int ogl_IsVersionGEQ(int majorVersion, int minorVersion);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif //POINTER_C_GENERATED_HEADER_OPENGL_H
