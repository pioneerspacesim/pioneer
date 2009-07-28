#include "libs.h"
#include "Shader.h"
#include "Pi.h"
#include "sbre/sbre_int.h"

namespace Shader {

GLuint vtxprog[VPROG_MAX];
bool isEnabled = false;

void ToggleState()
{
	if (GLEW_ARB_vertex_program && !isEnabled) isEnabled = true;
	else isEnabled = false;
	printf("Vertex shaders %s.\n", isEnabled ? "on" : "off");
}

void DisableVertexProgram()
{
	if (!isEnabled) return;
	glDisable(GL_VERTEX_PROGRAM_ARB);
}

void EnableVertexProgram(VertexProgram p)
{
	if (!isEnabled) return;
	glEnable(GL_VERTEX_PROGRAM_ARB);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vtxprog[(int)p]);
}

static void CompileProgram(VertexProgram p, const char *code)
{
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vtxprog[(int)p]);
	glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(code), code);
	
	if (GL_INVALID_OPERATION == glGetError()) {
		GLint errPos;
		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errPos);
		const GLubyte *errString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
		printf("Error at position %d\n%s\n", errPos, errString);
		printf("%s\n", &code[errPos]);
		Pi::Quit();
	}
}

static const char *geosphere_prog = "!!ARBvp1.0\n"
	"ATTRIB pos = vertex.position;\n"
	"ATTRIB norm = vertex.normal;\n"
	
	"OUTPUT oColor = result.color;\n"
	"PARAM mv[4] = { state.matrix.modelview };\n"
	"PARAM mvp[4] = { state.matrix.mvp };\n"
	"PARAM mvinv[4] = { state.matrix.modelview.invtrans };\n"
	"PARAM specExp = state.material.shininess;\n"
	"TEMP eyeNormal, temp, dots, lightcoefs;\n"
	
	// transform vertex to clip coords, linearizing z
	"DP4 temp.x, mvp[0], pos;\n"
	"DP4 temp.y, mvp[1], pos;\n"
	"DP4 temp.z, mv[2], pos;\n"
	"DP4 temp.w, mvp[3], pos;\n"
	"MOV result.position.xyw, temp;\n"
	"RSQ temp.z, temp.z;\n"
	"MAD temp.z, temp.z, program.env[0].x, program.env[0].y;\n" // (1.0/sqrt(temp.z))*zmul + zmod
	"MUL result.position.z, temp.z, temp.w;\n"
	
	// transform normal into eye space
	"DP3 eyeNormal.x, mvinv[0], norm;\n"
	"DP3 eyeNormal.y, mvinv[1], norm;\n"
	"DP3 eyeNormal.z, mvinv[2], norm;\n"
	// geosphere needs normals normalized
	"DP3 temp.w, eyeNormal, eyeNormal;\n"
	"RSQ temp.w, temp.w;\n"
	"MUL eyeNormal.xyz, temp.w, eyeNormal;\n"


	"TEMP colacc;\n"
	// accumulate color contributions
	"TEMP diffuse;\n"
	
	// light 0
	"DP3 dots.x, eyeNormal, state.light[0].position;\n"
	"DP3 dots.y, eyeNormal, state.light[0].half;\n"
	"MOV dots.w, specExp.x;\n"
	"LIT lightcoefs, dots;\n"
	"MUL diffuse, state.light[0].diffuse, vertex.color;\n"
	"MUL colacc.xyz, lightcoefs.y, diffuse;\n"
	// light 1
	"DP3 dots.x, eyeNormal, state.light[1].position;\n"
	"DP3 dots.y, eyeNormal, state.light[1].half;\n"
	"MOV dots.w, specExp.x;\n"
	"LIT lightcoefs, dots;\n"
	"MUL diffuse, state.light[1].diffuse, vertex.color;\n"
	"MAD colacc.xyz, lightcoefs.y, diffuse, colacc;\n"
	// light 2
	"DP3 dots.x, eyeNormal, state.light[2].position;\n"
	"DP3 dots.y, eyeNormal, state.light[2].half;\n"
	"MOV dots.w, specExp.x;\n"
	"LIT lightcoefs, dots;\n"
	"MUL diffuse, state.light[2].diffuse, vertex.color;\n"
	"MAD colacc.xyz, lightcoefs.y, diffuse, colacc;\n"
	// light 3
	"DP3 dots.x, eyeNormal, state.light[3].position;\n"
	"DP3 dots.y, eyeNormal, state.light[3].half;\n"
	"MOV dots.w, specExp.x;\n"
	"LIT lightcoefs, dots;\n"
	"MUL diffuse, state.light[3].diffuse, vertex.color;\n"
	"MAD colacc.xyz, lightcoefs.y, diffuse, colacc;\n"
	
	// ambient
	"MAD oColor.xyz, vertex.color, {.1,.1,.1,1.0}, colacc;\n"
	"MOV oColor.w, vertex.color.w;\n"
	
	"END\n"
	;

static const char *sbre_prog = "!!ARBvp1.0\n"
	"ATTRIB pos = vertex.position;\n"
	"ATTRIB norm = vertex.normal;\n"
	
	"OUTPUT oColor = result.color;\n"
	"PARAM mv[4] = { state.matrix.modelview };\n"
	"PARAM mvp[4] = { state.matrix.mvp };\n"
	"PARAM mvinv[4] = { state.matrix.modelview.invtrans };\n"
	"PARAM shininess = state.material.shininess;\n"
	"PARAM diffuseCol = state.lightprod[0].diffuse;\n"
	"PARAM emissionCol = state.material.emission;\n"
	"PARAM sbre_amb = program.env[1];\n"
	"TEMP eyeNormal, temp, dots, lightcoefs;\n"
	
	// transform vertex to clip coords, linearizing z
	"DP4 temp.x, mvp[0], pos;\n"
	"DP4 temp.y, mvp[1], pos;\n"
	"DP4 temp.z, mv[2], pos;\n"
	"DP4 temp.w, mvp[3], pos;\n"
	"MOV result.position.xyw, temp;\n"
	"RSQ temp.z, temp.z;\n"
	"MAD temp.z, temp.z, program.env[0].x, program.env[0].y;\n" // (1.0/sqrt(temp.z))*zmul + zmod
	"MUL result.position.z, temp.z, temp.w;\n"
	
	// transform normal into eye space
	"DP3 eyeNormal.x, mvinv[0], norm;\n"
	"DP3 eyeNormal.y, mvinv[1], norm;\n"
	"DP3 eyeNormal.z, mvinv[2], norm;\n"
	// geosphere needs normals normalized
	"DP3 temp.w, eyeNormal, eyeNormal;\n"
	"RSQ temp.w, temp.w;\n"
	"MUL eyeNormal.xyz, temp.w, eyeNormal;\n"

	// transform normal into eye space
	"DP3 eyeNormal.x, mvinv[0], norm;\n"
	"DP3 eyeNormal.y, mvinv[1], norm;\n"
	"DP3 eyeNormal.z, mvinv[2], norm;\n"
	
	// geosphere needs normals normalized
	"DP3 temp.w, eyeNormal, eyeNormal;\n"
	"RSQ temp.w, temp.w;\n"
	"MUL eyeNormal.xyz, temp.w, eyeNormal;\n"

	"TEMP colacc;\n"
	
	// light 0
	"DP3 dots.x, eyeNormal, state.light[0].position;\n"
	"DP3 dots.y, eyeNormal, state.light[0].half;\n"
	"MOV dots.w, shininess.x;\n"
	"LIT lightcoefs, dots;\n"
	"MAD temp, lightcoefs.y, state.lightprod[0].diffuse, state.lightprod[0].ambient;\n"
	"MAD colacc.xyz, lightcoefs.z, state.lightprod[0].specular, temp;\n"
	// light 1
	"DP3 dots.x, eyeNormal, state.light[1].position;\n"
	"DP3 dots.y, eyeNormal, state.light[1].half;\n"
	"MOV dots.w, shininess.x;\n"
	"LIT lightcoefs, dots;\n"
	"ADD colacc.xyz, colacc, state.lightprod[1].ambient;\n"
	"MAD temp, lightcoefs.y, state.lightprod[1].diffuse, colacc;\n"
	"MAD colacc.xyz, lightcoefs.z, state.lightprod[1].specular, temp;\n"
	// light 2
	"DP3 dots.x, eyeNormal, state.light[2].position;\n"
	"DP3 dots.y, eyeNormal, state.light[2].half;\n"
	"MOV dots.w, shininess.x;\n"
	"LIT lightcoefs, dots;\n"
	"ADD colacc.xyz, colacc, state.lightprod[2].ambient;\n"
	"MAD temp, lightcoefs.y, state.lightprod[2].diffuse, colacc;\n"
	"MAD colacc.xyz, lightcoefs.z, state.lightprod[2].specular, temp;\n"
	// light 3
	"DP3 dots.x, eyeNormal, state.light[3].position;\n"
	"DP3 dots.y, eyeNormal, state.light[3].half;\n"
	"MOV dots.w, shininess.x;\n"
	"LIT lightcoefs, dots;\n"
	"ADD colacc.xyz, colacc, state.lightprod[3].ambient;\n"
	"MAD temp, lightcoefs.y, state.lightprod[3].diffuse, colacc;\n"
	"MAD colacc.xyz, lightcoefs.z, state.lightprod[3].specular, temp;\n"
	// SBRE_AMB
	"MAD colacc, state.material.ambient, sbre_amb, colacc;\n"
	
	"ADD oColor.xyz, colacc, emissionCol;\n"
	"MOV oColor.w, diffuseCol.w;\n"
	
	"END\n"
	;

void Init()
{
	isEnabled = GLEW_ARB_vertex_program;
	fprintf(stderr, "GL_ARB_vertex_program: %s\n", isEnabled ? "Yes" : "No");
	if (!isEnabled) return;
	
	glEnable(GL_VERTEX_PROGRAM_ARB);
	glGenProgramsARB(VPROG_MAX, vtxprog);
	
	{
		const float zmul = 1.0 / (1.0/sqrtf(WORLDVIEW_ZFAR) - 1.0/sqrtf(WORLDVIEW_ZNEAR));
		const float zmod = -zmul / sqrtf(WORLDVIEW_ZNEAR);
		glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0, zmul, zmod, 0.0, 0.0);
		glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, 1, SBRE_AMB, SBRE_AMB, SBRE_AMB, 1.0);
	}
	CompileProgram(VPROG_GEOSPHERE, geosphere_prog);
	CompileProgram(VPROG_SBRE, sbre_prog);
	
	DisableVertexProgram();
}

} /* namespace Shader */ 

