#include "Render.h"

namespace Render {

static bool initted = false;
static bool shadersEnabled;
static State *currentState;
Shader *simpleShader;
Shader *planetRingsShader;

void Init()
{
	if (initted) return;
	shadersEnabled = (GLEW_VERSION_2_0 ? true : false);
	printf("GLSL shaders %s.\n", shadersEnabled ? "on" : "off");
	if (shadersEnabled) {
		simpleShader = new Shader("simple");
		planetRingsShader = new Shader("planetrings");
	}
	initted = true;
}

bool AreShadersEnabled()
{
	return shadersEnabled;
}

void SetCurrentState(State *state)
{
	currentState = state;
}

State *GetCurrentState()
{
	return currentState;
}

GLuint UseProgram(const Shader *shader)
{
	assert(currentState != 0);
	return currentState->UseProgram(shader);
}

void ToggleShaders()
{
	if (CanDoShaders()) {
		shadersEnabled = (shadersEnabled ? false : true);
	}
	printf("GLSL shaders %s.\n", shadersEnabled ? "on" : "off");
}

/*
 * So if we are using the z-hack VPROG_POINTSPRITE then this still works.
 */
void PutPointSprites(int num, vector3f v[], float size, const float modulationCol[4], GLuint tex)
{
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);

	float quadratic[] =  { 0.0f, 0.0f, 0.00001f };
	glPointParameterfv( GL_POINT_DISTANCE_ATTENUATION, quadratic );
	glPointParameterf(GL_POINT_SIZE_MIN, 1.0 );
	glPointParameterf(GL_POINT_SIZE_MAX, 10000.0 );
		
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glColor4fv(modulationCol);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);	

	// XXX point sprite thing needs some work. remember to enable point
	// sprite shader in LmrModel.cpp
//	/*if (Shader::IsVtxProgActive())*/ glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
	if (0) {//GLEW_ARB_point_sprite) {
		glTexEnvf(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
		glEnable(GL_POINT_SPRITE_ARB);
		
		glPointSize(size);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, &v[0].x);
		glDrawArrays(GL_POINTS, 0, num);
		glDisableClientState(GL_VERTEX_ARRAY);
		glPointSize(1);

		glDisable(GL_POINT_SPRITE_ARB);
		glDisable(GL_TEXTURE_2D);
	
	} else {
		// quad billboards
		const float sz = 0.5f*size;
		vector3f v1(sz, sz, 0.0f);
		vector3f v2(sz, -sz, 0.0f);
		vector3f v3(-sz, -sz, 0.0f);
		vector3f v4(-sz, sz, 0.0f);
		
		matrix4x4f rot;
		glGetFloatv(GL_MODELVIEW_MATRIX, &rot[0]);
		rot.ClearToRotOnly();
		rot = rot.InverseOf();

		glBegin(GL_QUADS);
		for (int i=0; i<num; i++) {
			vector3f pos(&v[i].x);
			glTexCoord2f(0.0f,0.0f);
			glVertex3fv(reinterpret_cast<const GLfloat*>(&(pos+rot*v4)));
			glTexCoord2f(0.0f,1.0f);
			glVertex3fv(reinterpret_cast<const GLfloat*>(&(pos+rot*v3)));
			glTexCoord2f(1.0f,1.0f);
			glVertex3fv(reinterpret_cast<const GLfloat*>(&(pos+rot*v2)));
			glTexCoord2f(1.0f,0.0f);
			glVertex3fv(reinterpret_cast<const GLfloat*>(&(pos+rot*v1)));
		}
		glEnd();
	}
//	/*if (Shader::IsVtxProgActive())*/ glDisable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);

	quadratic[0] = 1; quadratic[1] = 0;
	glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );

	glDisable(GL_TEXTURE_2D);
	glDepthMask(GL_TRUE);
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
}

// -------------- class Shader ----------------


static char *load_file(const char *filename)
{
	FILE *f = fopen(filename, "r");
	if (!f) {
		//printf("Could not open %s.\n", filename);
		return 0;
	}
	fseek(f, 0, SEEK_END);
	size_t len = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = (char*)malloc(sizeof(char) * (len+1));
	fread(buf, 1, len, f);
	fclose(f);
	buf[len] = 0;
	return buf;
}

static void printLog(const char *filename, GLuint obj)
{
	int infologLength = 0;
	char infoLog[1024];

	if (glIsShader(obj))
		glGetShaderInfoLog(obj, 1024, &infologLength, infoLog);
	else
		glGetProgramInfoLog(obj, 1024, &infologLength, infoLog);

	if (infologLength > 0)
		printf("%s: %s", filename, infoLog);
}
	
#if 0
	{
		/* zbuffer values are (where z = z*modelview*projection):
		 * depthvalue = log(C*z + 1) / log(C*zfar + 1)
		 * using C = 1.0 */
		glUseProgram(prog);
		GLint invLogZfarPlus1Loc = glGetUniformLocation(prog, "invLogZfarPlus1");
//			assert(invLogZfarPlus1Loc);
		float znear, zfar;
		Pi::worldView->GetNearFarClipPlane(&znear, &zfar);
		const float invDenominator = 1.0/(log(zfar + 1.0)/log(2.0));

		glUniform1f(invLogZfarPlus1Loc, invDenominator);
		glUseProgram(0);
	}
#endif

GLuint Shader::CompileProgram(const char *shader_name, int num_lights)
{
	static char *lib_vs = 0;
	static char *lib_all = 0;
	if (!lib_vs) lib_vs = load_file("data/shaders/_library.vert.glsl");
	if (!lib_all) lib_all = load_file("data/shaders/_library.all.glsl");

	const std::string name = std::string("data/shaders/") + shader_name;
	char *vscode = load_file((name + ".vert.glsl").c_str());
	char *pscode = load_file((name + ".frag.glsl").c_str());
	char *allcode = load_file((name + ".all.glsl").c_str());
	
	if (vscode == 0) {
		printf("Error loading shader %s.\n", (name + ".vert.glsl").c_str());
		abort();
	}
		
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	std::vector<const char*> shader_src;

	char lightDef[128];
	snprintf(lightDef, sizeof(lightDef), "#define NUM_LIGHTS %d\n", num_lights);
	std::string vendor;
	if (strcmp((const char*)glGetString(GL_VENDOR), "X.Org R300 Project") == 0) {
		vendor = "#define VENDOR_R300_GALLIUM\n";
	}
	shader_src.push_back(vendor.c_str());
	shader_src.push_back(lightDef);
	shader_src.push_back(lib_all);
	shader_src.push_back(lib_vs);
	if (allcode) shader_src.push_back(allcode);
	shader_src.push_back(vscode);

	glShaderSource(vs, shader_src.size(), &shader_src[0], 0);
	glCompileShader(vs);
	GLint status;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	if (!status) {
		printLog((name + ".vert.glsl").c_str(), vs);
		abort();
	}

	GLuint ps = 0;
	if (pscode) {
		shader_src.clear();
		shader_src.push_back(vendor.c_str());
		shader_src.push_back(lightDef);
		shader_src.push_back(lib_all);
		if (allcode) shader_src.push_back(allcode);
		shader_src.push_back(pscode);
		
		ps = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(ps, shader_src.size(), &shader_src[0], 0);
		glCompileShader(ps);
		GLint status;
		glGetShaderiv(ps, GL_COMPILE_STATUS, &status);
		if (!status) {
			printLog((name + ".frag.glsl").c_str(), ps);
			abort();
		}
	}

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	if (pscode) glAttachShader(prog, ps);
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (!status) {
		printLog(name.c_str(), prog);
		abort();
	}

	free(vscode);
	if (pscode) free(pscode);
	if (allcode) free(allcode);
	
	return prog;
}

Shader::Shader(const char *name)
{
	for (int i=0; i<4; i++) {
		m_program[i] = CompileProgram(name, i+1);
	}
}

// --------------- class Shader ------------------

GLuint State::UseProgram(const Shader *shader)
{
	if (shadersEnabled) {
		if (shader) {
			GLuint prog = shader->m_program[m_numLights-1];
			glUseProgram(prog);
			GLint invLogZfarPlus1Loc = glGetUniformLocation(prog, "invLogZfarPlus1");
			glUniform1f(invLogZfarPlus1Loc, m_invLogZfarPlus1);
			return prog;
		} else {
			glUseProgram(0);
		}
	}
	return 0;
}

}; /* namespace Render */
