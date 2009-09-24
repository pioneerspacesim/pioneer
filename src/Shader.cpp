#include "libs.h"
#include "Shader.h"
#include "Pi.h"
#include "sbre/sbre_int.h"
#include "WorldView.h"

namespace Shader {

// 4 vertex programs per enum VertexProgram, for 1,2,3,4 lights
GLuint vtxprog[VPROG_MAX*4];
bool isEnabled = false;
bool isVtxProgActive = false;

bool IsEnabled() { return isEnabled; }
bool IsVtxProgActive() { return isVtxProgActive; }

void ToggleState()
{
	if (GLEW_ARB_vertex_program && !isEnabled) isEnabled = true;
	else isEnabled = false;
	printf("Vertex shaders %s.\n", isEnabled ? "on" : "off");
}

void DisableVertexProgram()
{
	if (!isEnabled) return;
	isVtxProgActive = false;
	glDisable(GL_VERTEX_PROGRAM_ARB);
}

void EnableVertexProgram(VertexProgram p)
{
	if (!isEnabled) return;
	isVtxProgActive = true;
	glEnable(GL_VERTEX_PROGRAM_ARB);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vtxprog[4*(int)p + Pi::worldView->GetNumLights() - 1]);
}

static void CompileProgram(VertexProgram p, const char *code[4])
{
	for (int i=0; i<4; i++) {
		if (!code[i]) {
			vtxprog[4*(int)p + i] = vtxprog[4*(int)p];
			continue;
		}
		glGenProgramsARB(1, &vtxprog[4*(int)p + i]);
		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vtxprog[4*(int)p + i]);
		glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(code[i]), code[i]);
		
		if (GL_INVALID_OPERATION == glGetError()) {
			GLint errPos;
			glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errPos);
			const GLubyte *errString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			printf("Error at position %d\n%s\n", errPos, errString);
			printf("%s\n", &code[i][errPos]);
			Pi::Quit();
		}
	}
}

// makes depth value = log(C*z + 1) / log(C*zfar + 1)
// args.x = 1.0/log(C*zfar + 1.0)
// args.y = 1.0
#define FIXZ_N_TRANSFORM_2_CLIPCOORDS() \
	"PARAM args = program.env[0];\n" \
	"DP4 temp.x, mvp[0], pos;\n" \
	"DP4 temp.y, mvp[1], pos;\n" \
	"DP4 temp.z, mvp[2], pos;\n" \
	"DP4 temp.w, mvp[3], pos;\n" \
	"MOV result.position.xyw, temp;\n" \
	"ADD temp.z, temp.z, args.y;\n" \
	"LG2 temp.z, temp.z;\n" \
	"MUL temp.z, temp.z, args.x;\n" \
	/* multiply by w to subvert fixed-function mandatory div by w */ \
	"MUL result.position.z, temp.z, temp.w;\n" 

#define MAKE_EYENORMAL() \
	/* transform normal into eye space */ \
	"DP3 eyeNormal.x, mvinv[0], norm;\n" \
	"DP3 eyeNormal.y, mvinv[1], norm;\n" \
	"DP3 eyeNormal.z, mvinv[2], norm;\n" \
	/* normalize */ \
	"DP3 temp.w, eyeNormal, eyeNormal;\n" \
	"RSQ temp.w, temp.w;\n" \
	"MUL eyeNormal.xyz, temp.w, eyeNormal;\n"
	
#define MAKE_LIGHT_COEFFS(lightNum) \
	"DP3 dots.x, eyeNormal, state.light[" lightNum "].position;\n" \
	"DP3 dots.y, eyeNormal, state.light[" lightNum "].half;\n" \
	"MOV dots.w, shininess.x;\n" \
	"LIT lightcoefs, dots;\n"

static const char *simple_prog[4] = {
	"!!ARBvp1.0\n"
	"ATTRIB pos = vertex.position;\n"
	"OUTPUT oColor = result.color;\n"
	"PARAM mv[4] = { state.matrix.modelview };\n"
	"PARAM mvp[4] = { state.matrix.mvp };\n"
	"TEMP temp;\n"
	FIXZ_N_TRANSFORM_2_CLIPCOORDS()
	"MOV oColor, vertex.color;\n"
	"END"
	,0,0,0
};

static const char *pointsprite_prog[4] = {
	"!!ARBvp1.0\n"
	"ATTRIB pos = vertex.position;\n"
	"ATTRIB tex = vertex.texcoord;\n"
	"OUTPUT oColor = result.color;\n"
	"OUTPUT oPointSize = result.pointsize;\n"
	"OUTPUT oTexCoord1 = result.texcoord;\n"
	"PARAM mv[4] = { state.matrix.modelview };\n"
	"PARAM mvp[4] = { state.matrix.mvp };\n"
	"TEMP temp;\n"
	FIXZ_N_TRANSFORM_2_CLIPCOORDS()
	"MOV oColor, vertex.color;\n"
	"MOV oTexCoord1, tex;\n"

	// attenuate points using only the linear attenuation coefficient
	"DP4 temp.z, mv[2], pos;\n" 
	"MUL temp.x, state.point.attenuation.y, temp.z;\n" 
	"RSQ temp.x, temp.x;\n" 
	"MUL oPointSize.x, temp.x, state.point.size.x;\n"

	"END"
	,0,0,0
};



#define GEOSPHERE_PROG_START() \
	"!!ARBvp1.0\n"\
	"ATTRIB pos = vertex.position;\n"\
	"ATTRIB norm = vertex.normal;\n"\
	"OUTPUT oColor = result.color;\n"\
	"PARAM mv[4] = { state.matrix.modelview };\n"\
	"PARAM mvp[4] = { state.matrix.mvp };\n"\
	"PARAM mvinv[4] = { state.matrix.modelview.invtrans };\n"\
	"PARAM shininess = state.material.shininess;\n"\
	"TEMP eyeNormal, temp, dots, lightcoefs;\n"\
	FIXZ_N_TRANSFORM_2_CLIPCOORDS()\
	MAKE_EYENORMAL()\
	"TEMP colacc;\n"\
	"TEMP diffuse;\n"\
	"MOV colacc.xyz, {0,0,0};\n"
	
#define GEOSPHERE_ACCUMULATE_LIGHT(light_n) \
	MAKE_LIGHT_COEFFS( light_n )\
	"MAD colacc.xyz, state.light[" light_n "].ambient, vertex.color, colacc;\n" \
	"MUL diffuse, state.light[" light_n "].diffuse, vertex.color;\n"\
	"MAD colacc.xyz, lightcoefs.y, diffuse, colacc;\n"
	
#define GEOSPHERE_PROG_END() \
	"MOV oColor.xyz, colacc;\n"\
	"MOV oColor.w, vertex.color.w;\n"\
	"END\n"

static const char *geosphere_prog[4] = {
	GEOSPHERE_PROG_START()
	GEOSPHERE_ACCUMULATE_LIGHT("0")
	GEOSPHERE_PROG_END(),

	GEOSPHERE_PROG_START()
	GEOSPHERE_ACCUMULATE_LIGHT("0")
	GEOSPHERE_ACCUMULATE_LIGHT("1")
	GEOSPHERE_PROG_END(),

	GEOSPHERE_PROG_START()
	GEOSPHERE_ACCUMULATE_LIGHT("0")
	GEOSPHERE_ACCUMULATE_LIGHT("1")
	GEOSPHERE_ACCUMULATE_LIGHT("2")
	GEOSPHERE_PROG_END(),

	GEOSPHERE_PROG_START()
	GEOSPHERE_ACCUMULATE_LIGHT("0")
	GEOSPHERE_ACCUMULATE_LIGHT("1")
	GEOSPHERE_ACCUMULATE_LIGHT("2")
	GEOSPHERE_ACCUMULATE_LIGHT("3")
	GEOSPHERE_PROG_END()
};

#define SBRE_PROG_START() \
	"!!ARBvp1.0\n" \
	"ATTRIB pos = vertex.position;\n" \
	"ATTRIB norm = vertex.normal;\n" \
	\
	"OUTPUT oColor = result.color;\n" \
	"PARAM mv[4] = { state.matrix.modelview };\n" \
	"PARAM mvp[4] = { state.matrix.mvp };\n" \
	"PARAM mvinv[4] = { state.matrix.modelview.invtrans };\n" \
	"PARAM shininess = state.material.shininess;\n" \
	"PARAM diffuseCol = state.lightprod[0].diffuse;\n" \
	"PARAM emissionCol = state.material.emission;\n" \
	"PARAM sbre_amb = program.env[1];\n" \
	"TEMP eyeNormal, colacc, temp, dots, lightcoefs;\n" \
	FIXZ_N_TRANSFORM_2_CLIPCOORDS() \
	MAKE_EYENORMAL() \
	"MAD colacc.xyz, state.material.ambient, sbre_amb, emissionCol;\n"

#define SBRE_ACCUMULATE_LIGHT(light_n) \
	MAKE_LIGHT_COEFFS( light_n ) \
	"ADD colacc.xyz, colacc, state.lightprod[" light_n "].ambient;\n" \
	"MAD temp, lightcoefs.y, state.lightprod[" light_n "].diffuse, colacc;\n"\
	"MAD colacc.xyz, lightcoefs.z, state.lightprod[" light_n "].specular, temp;\n"

#define SBRE_PROG_END() \
	"MOV oColor.xyz, colacc;\n"\
	"MOV oColor.w, diffuseCol.w;\n"\
	"END\n"

/*
 * Shaders come in one, two, three and four light versions.
 */
static const char *sbre_prog[4] = {
	SBRE_PROG_START()
	SBRE_ACCUMULATE_LIGHT("0")
	SBRE_PROG_END(),

	SBRE_PROG_START()
	SBRE_ACCUMULATE_LIGHT("0")
	SBRE_ACCUMULATE_LIGHT("1")
	SBRE_PROG_END(),

	SBRE_PROG_START()
	SBRE_ACCUMULATE_LIGHT("0")
	SBRE_ACCUMULATE_LIGHT("1")
	SBRE_ACCUMULATE_LIGHT("2")
	SBRE_PROG_END(),

	SBRE_PROG_START()
	SBRE_ACCUMULATE_LIGHT("0")
	SBRE_ACCUMULATE_LIGHT("1")
	SBRE_ACCUMULATE_LIGHT("2")
	SBRE_ACCUMULATE_LIGHT("3")
	SBRE_PROG_END()
};

void Init()
{
	isEnabled = GLEW_ARB_vertex_program;
	fprintf(stderr, "GL_ARB_vertex_program: %s\n", isEnabled ? "Yes" : "No");
	if (!isEnabled) return;
	
	glEnable(GL_VERTEX_PROGRAM_ARB);
	
	{
		/* zbuffer values are (where z = z*modelview*projection):
		 * depthvalue = log(C*z + 1) / log(C*zfar + 1)
		 * using C = 1.0 */
		float znear, zfar;
		Pi::worldView->GetNearFarClipPlane(&znear, &zfar);
		const float invDenominator = 1.0/log2(zfar + 1.0);
		glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0, invDenominator, 1.0, 0.0, 0.0);
		glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 1, SBRE_AMB, SBRE_AMB, SBRE_AMB, 1.0);
	}
	CompileProgram(VPROG_GEOSPHERE, geosphere_prog);
	CompileProgram(VPROG_SBRE, sbre_prog);
	CompileProgram(VPROG_SIMPLE, simple_prog);
	CompileProgram(VPROG_POINTSPRITE, pointsprite_prog);
	
	DisableVertexProgram();
}

} /* namespace Shader */ 

