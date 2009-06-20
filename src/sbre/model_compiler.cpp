
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <vector>
#include <string>
#include <map>
#include <float.h>
#include "sbre_int.h"
#include "sbre_models.h"
#include "sbre.h"

#define MAX_TOKEN_LEN 64

static const char * const anim_fns[] = {
	"gear", "gflap", "thrustpulse", "lin4sec", 0
};

// this nonsense all needs to be removed after compiling models....
#define IS_COMPLEX (0x4000)
int modelNum = SBRE_COMPILED_MODELS;
static std::map<std::string, Model*> models;
static std::map<std::string, int> models_idx;
static std::vector<Vector> vertices;
static std::map<std::string, int> vertex_identifiers;
static std::vector<Uint16> instrs[4];
static int lodSize[4]; // -1 = inactive
static bool lodActive[4]; // for this instruction
static int numLods;
int sbre_cache_size;
static std::vector<CompoundVertex> compound_vertices;
static std::vector<int> compound_vertex_fixups_in_instrs[4];
static std::vector<Thruster> thrusters[4];
struct obj_lod {
	int divs[4];
};
static std::map<std::string, obj_lod> obj_lods;

int sbreLookupModelByName(const char *name) throw (SbreModelNotFoundException)
{
	int idx;
	std::map<std::string, int>::iterator i = models_idx.find(name);
	if (i != models_idx.end()) return (*i).second;
	else if (sscanf(name, "%d", &idx) == 1) {
		return idx;
	} else {
		// hm. never thrown an exception before in c++. is this a
		// slippery slope to oblivion?
		throw SbreModelNotFoundException();
	}
}


static void fatal()
{
	printf("Internal compiler error (bug)\n");
	abort();
}

static int lexLine;

#ifndef __GNUC__
#define __attribute(x)
#endif /* __GNUC__ */
static void error(int line, const char *format, ...)
		__attribute((format(printf,2,3)));

static void error(int line, const char *format, ...)
{
	va_list argptr;
	printf("Error at line %d: ", line);
	va_start(argptr, format);
	vprintf(format, argptr);
	va_end(argptr);
	abort();
	printf("\n");
}

class Token {
public:
	enum token_type_t {
		END, OPENBRACKET, CLOSEBRACKET,
		OPENBRACE, CLOSEBRACE, ASSIGN, COMMA, COLON, FLOAT,
		INTEGER, IDENTIFIER
	} type;
	static std::string ToString(token_type_t type) {
		switch (type) {
			case Token::OPENBRACKET: return "(";
			case Token::CLOSEBRACKET: return ")";
			case Token::OPENBRACE: return "{";
			case Token::CLOSEBRACE: return "}";
			case Token::ASSIGN: return "=";
			case Token::COMMA: return ",";
			case Token::FLOAT: return "float";
			case Token::INTEGER: return "integer";
			case Token::IDENTIFIER: return "identifier";
			case Token::COLON: return ":";
			case Token::END: return "End of file";
			default:
					 return "??unknown??";
		}
	}
	int line;
	union value {
		int i;
		float f;
		char s[MAX_TOKEN_LEN];
	} val;
	Token() {
		memset(this, 0, sizeof(Token));
	}
	void Error(const char *format, ...) {
		va_list argptr;
		printf("Syntax error at line %d: ", line);
		va_start(argptr, format);
		vprintf(format, argptr);
		va_end(argptr);
		printf("\n");
		abort();
	}
	void Check(enum token_type_t type) {
		if (this->type != type) Error("Unexpected token %s, expected %s", ToString(this->type).c_str(), ToString(type).c_str());
	}
	float GetFloat() {
		if (type == INTEGER) {
			return (float)val.i;
		} else if (type == FLOAT) {
			return val.f;
		}
		Error("Expected float, got '%s'", ToString(type).c_str());
		return 0;
	}
	bool IsIdentifier(const char *str) {
		return (type == IDENTIFIER) && (strcmp(str, val.s)==0);
	}
	void MatchIdentifier(const char *str) {
		if (!IsIdentifier(str)) {
			if (type == IDENTIFIER) Error("Expected identifier '%s', got '%s'", str, val.s);
			else Error("Expected identifer '%s', got token '%s'", str, ToString(type).c_str());
		}
	}
};

void lex(const char *crud, std::vector<Token> &tokens)
{
	Token t;
	lexLine = 1;
	while (*crud != 0) {
		t = Token();
		t.line = lexLine;
		if (*crud == '(') {
			t.type = Token::OPENBRACKET;
			tokens.push_back(t);
			crud++;
		} else if (*crud == ')') {
			t.type = Token::CLOSEBRACKET;
			tokens.push_back(t);
			crud++;
		} else if (*crud == '{') {
			t.type = Token::OPENBRACE;
			tokens.push_back(t);
			crud++;
		} else if (*crud == '}') {
			t.type = Token::CLOSEBRACE;
			tokens.push_back(t);
			crud++;
		} else if (*crud == '=') {
			t.type = Token::ASSIGN;
			tokens.push_back(t);
			crud++;
		} else if (*crud == ':') {
			t.type = Token::COLON;
			tokens.push_back(t);
			crud++;
		} else if (*crud == ',') {
			t.type = Token::COMMA;
			tokens.push_back(t);
			crud++;
		} else if (isspace(*crud)) {
			if (*crud == '\n') lexLine++;
			crud++;
		} else if ((*crud == '.') || (*crud == '-') || (isdigit(*crud))) {
			int dots = 0;
			const char *start = crud;
			char buf[16];
			memset(buf, 0, 16);
			do {
				if (*crud == '.') dots++;
				crud++;
			} while (isdigit(*crud) || (*crud == '.'));
			if (dots == 0) {
				if (sscanf(start, "%d", &t.val.i) != 1)
					error(lexLine, "Malformed integer");
				t.type = Token::INTEGER;
			} else if (dots == 1) {
				if (sscanf(start, "%f", &t.val.f) != 1)
					error(lexLine, "Malformed float");
				t.type = Token::FLOAT;
			} else error(lexLine, "Malformed numeral");
			tokens.push_back(t);
		} else if (isupper(*crud) || islower(*crud) || (*crud == '_')) {
			const char *start = crud;
			int len = 0;
			do {
				len++;
				crud++;
			} while (islower(*crud) || isupper(*crud) || isdigit(*crud) || (*crud == '_'));
			if (len > MAX_TOKEN_LEN-1) error(lexLine, "Excessive identifier length");
			strncpy(t.val.s, start, len);
			t.type = Token::IDENTIFIER;
			tokens.push_back(t);
		} else if (*crud == '#') {
			do {
				crud++;
			} while ((*crud != '\n') && (*crud != '\0'));
		} else {
			error(lexLine, "Unknown token '%c' (%d)", *crud, *crud);
		}
	}
	t = Token();
	t.type = Token::END;
	tokens.push_back(t);
}

typedef std::vector<Token>::iterator tokenIter_t;

static void parseObjectLod(tokenIter_t &t, Uint16 objLod[4])
{
	if ((*t).type == Token::INTEGER) {
		for (int i=0; i<4; i++) objLod[i] = (*t).val.i;
		++t;
	} else if ((*t).type == Token::IDENTIFIER) {
		std::map<std::string, obj_lod>::iterator i = obj_lods.find((*t).val.s);
		if (i == obj_lods.end()) (*t).Error("Undefined obj_lod %s", (*t).val.s);
		else {
			for (int j=0; j<4; j++) {
				objLod[j] = (*i).second.divs[j];
			}
		}
		++t;
	} else {
		(*t).Error("Expected integer steps or obj_lod identifier");
	}
}

static bool VertexIdentifierExists(const char *vid)
{
	std::map<std::string, int>::iterator i = vertex_identifiers.find(vid);
	return (i != vertex_identifiers.end());
}

static int addPlainVtx(Vector v)
{
	if ((v.x==1) && (v.y==0) && (v.z==0)) return 0;
	if ((v.x==0) && (v.y==1) && (v.z==0)) return 1;
	if ((v.x==0) && (v.y==0) && (v.z==1)) return 2;
	if ((v.x==-1) && (v.y==0) && (v.z==0)) return 3;
	if ((v.x==0) && (v.y==-1) && (v.z==0)) return 4;
	if ((v.x==0) && (v.y==0) && (v.z==-1)) return 5;
	// first 6 are +ve and -ve axis unit vectors
	int idx = 6 + vertices.size();
	vertices.push_back(v);
	return idx;
}

static void instrsPutVtxRef(Uint16 ref, int *offset4 = 0)
{
	for (int i=0; i<4; i++) {
		if (!lodActive[i]) continue;
		if (offset4 == 0) {
			if (ref & IS_COMPLEX) {
				compound_vertex_fixups_in_instrs[i].push_back(instrs[i].size());
			}
			instrs[i].push_back(ref);
		} else {
			if (ref & IS_COMPLEX) {
				compound_vertex_fixups_in_instrs[i].push_back(offset4[i]);
			}
			instrs[i][offset4[i]] = ref;
		}
	}
}

/*
 * Pass array of offsets to write to specific location for
 * each lod instruction stream.
 */
static void instrsPutInstr(Uint16 val, int *offset4 = 0)
{
	for (int i=0; i<4; i++) {
		if (!lodActive[i]) continue;
		if (offset4) {
			instrs[i][offset4[i]] = val;
		} else {
			instrs[i].push_back(val);
		}
	}
}

static void instrsPutObjectLod(const Uint16 objLod[4])
{
	for (int i=0; i<4; i++) {
		if (!lodActive[i]) continue;
		instrs[i].push_back(objLod[i]);
	}
}

static void instrsPutCacheIdx()
{
	for (int i=0; i<4; i++) {
		if (!lodActive[i]) continue;
		instrs[i].push_back(sbre_cache_size++);
	}
}

static void getInstrsFixupPos(int offset4[4])
{
	for (int i=0; i<4; i++) {
		offset4[i] = instrs[i].size();
	}
}

static int addCompoundVtx(enum vtxtype type, Uint16 p0, Uint16 p1, Uint16 p2, Uint16 p3, Uint16 p4)
{
	int idx = compound_vertices.size() | IS_COMPLEX;
	CompoundVertex cv;
	cv.type = type;
	cv.pParam[0] = p0; 
	cv.pParam[1] = p1; 
	cv.pParam[2] = p2; 
	cv.pParam[3] = p3; 
	cv.pParam[4] = p4; 
	compound_vertices.push_back(cv);
	return idx;
}

/* return anim func index */
static int parseAnimFunc(tokenIter_t &t)
{
	(*t++).MatchIdentifier("animfn");
	(*t++).Check(Token::ASSIGN);

	for (int i=0; ; i++) {
		if (anim_fns[i] == 0) (*t).Error("Unknown anim function");
		if ((*t).IsIdentifier(anim_fns[i])) { ++t; return i; }
	}
	fatal();
	return -1;
}

static int parseVtxOrVtxRef(tokenIter_t &t);
/*
 * Returns index of plain vertex or 0x8000 if none found
 */
static int parseNewVertex(tokenIter_t &t)
{
	Vector v;
	bool normalize = false;
	if ((*t).IsIdentifier("v") ||
	    (*t).IsIdentifier("n")) {

		if ((*t).IsIdentifier("n")) {
			normalize = true;
		}

		++t;
		(*t++).Check(Token::OPENBRACKET);
		v.x = (*t++).GetFloat();
		(*t++).Check(Token::COMMA);
		v.y = (*t++).GetFloat();
		(*t++).Check(Token::COMMA);
		v.z = (*t++).GetFloat();
		(*t++).Check(Token::CLOSEBRACKET);
		if (normalize) {
			float len = 1.0f/sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
			v.x *= len;
			v.y *= len;
			v.z *= len;
		}
		return addPlainVtx(v);

	} else if ((*t).IsIdentifier("vnormal")) {

		++t;
		(*t++).Check(Token::OPENBRACKET);
		Uint16 v1 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		Uint16 v2 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		Uint16 v3 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::CLOSEBRACKET);

		return addCompoundVtx(VTYPE_NORM, v1, v2, v3, -1, -1);
	
	} else if ((*t).IsIdentifier("vcross")) {

		++t;
		(*t++).Check(Token::OPENBRACKET);
		Uint16 v1 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		Uint16 v2 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::CLOSEBRACKET);

		return addCompoundVtx(VTYPE_CROSS, v1, v2, -1, -1, -1);

	} else if ((*t).IsIdentifier("vlinear")) {

		++t;
		(*t++).Check(Token::OPENBRACKET);
		Uint16 v1 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		Uint16 v2 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		int animfn = parseAnimFunc(t);
		(*t++).Check(Token::CLOSEBRACKET);

		return addCompoundVtx(VTYPE_ANIMLIN, v1, v2, -1, -1, animfn);

	} else if ((*t).IsIdentifier("vcubic")) {

		++t;
		(*t++).Check(Token::OPENBRACKET);
		Uint16 v1 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		Uint16 v2 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		Uint16 v3 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		Uint16 v4 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		int animfn = parseAnimFunc(t);
		(*t++).Check(Token::CLOSEBRACKET);

		return addCompoundVtx(VTYPE_ANIMCUBIC, v1, v2, v3, v4, animfn);
	
	} else if ((*t).IsIdentifier("vhermite")) {

		++t;
		(*t++).Check(Token::OPENBRACKET);
		Uint16 v1 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		Uint16 v2 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		Uint16 n1 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		Uint16 n2 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		int animfn = parseAnimFunc(t);
		(*t++).Check(Token::CLOSEBRACKET);

		return addCompoundVtx(VTYPE_ANIMHERM, v1, v2, n1, n2, animfn);
	
	} else if ((*t).IsIdentifier("vrotate")) {

		++t;
		(*t++).Check(Token::OPENBRACKET);
		Uint16 v1 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		Uint16 v2 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		int animfn = parseAnimFunc(t);
		(*t++).Check(Token::CLOSEBRACKET);

		return addCompoundVtx(VTYPE_ANIMROTATE, v1, v2, -1, -1, animfn);

	} else {
		return 0x8000;
	}
}

/* return vertex identifier idx */
static int parseVtxOrVtxRef(tokenIter_t &t)
{
	Uint16 idx;
	//int numPVtx;				// number of plain vertices + 6
	(*t).Check(Token::IDENTIFIER);
	idx = parseNewVertex(t);
	// found new vertex. return idx,
	if (idx != 0x8000) return idx;
	// otherwise parse identifier reference turd
	else {
		std::map<std::string, int>::iterator i = vertex_identifiers.find((*t).val.s);
		if (i == vertex_identifiers.end()) {
			(*t).Error("Unknown vertex identifier %s", (*t).val.s);
		}
		++t;
		return (*i).second;
	}
}

/* 
 * returns first vertex idx
 */
static Uint16 parseCompObjectBits(tokenIter_t &t, bool smooth)
{
	int num = 0;
	Uint16 v1 = -1;
	for (;;) {
		if (smooth && (*t).IsIdentifier("hermite_norm")) {
			++t;
			(*t++).Check(Token::OPENBRACKET);
			v1 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 n0 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 n1 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::CLOSEBRACKET);
			instrsPutInstr(LTYPE_NORMS);
			instrsPutVtxRef(v1);
			instrsPutVtxRef(n0);
			instrsPutVtxRef(n1);
			num++;
		} else if ((*t).IsIdentifier("hermite")) {
			++t;
			(*t++).Check(Token::OPENBRACKET);
			v1 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 tan0 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 tan1 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::CLOSEBRACKET);
			instrsPutInstr(LTYPE_HERMITE);
			instrsPutVtxRef(v1);
			instrsPutVtxRef(tan0);
			instrsPutVtxRef(tan1);
			num++;
		} else if ((*t).IsIdentifier("line")) {
			++t;
			(*t++).Check(Token::OPENBRACKET);
			v1 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::CLOSEBRACKET);
			instrsPutInstr(LTYPE_LINE);
			instrsPutVtxRef(v1);
			num++;
		} else {
			(*t).Error("Expected a hermite or line segment");
		}
		if ((*t).type == Token::CLOSEBRACKET) {
			break;
		} else if ((*t).type == Token::COMMA) {
			t++;
		} else {
			break;
		}
	}
	if (v1== -1) fatal();
	if (num<2) (*t).Error("Expected at least 2 hermite or line segments");
	else if (smooth && (num>4)) (*t).Error("Expected at most 4 hermite or line segments for a smooth() turd");
	// first vertex in SBRE complex surfaces is the same as the last vtx
	return v1;
}

static int get_model_reference(tokenIter_t &t)
{
	(*t).Check(Token::IDENTIFIER);

	std::map<std::string, int>::iterator i = models_idx.find((*t).val.s);
	if (i == models_idx.end()) {
		(*t).Error("Unknown model '%s'", (*t).val.s);
		return -1;
	} else {
		int model = (*i).second;
		++t;
		return model;
	}
}

static Uint16 parseAnim(tokenIter_t &t)
{
	// don't want animfunc, want param->pAnim[idx] it uses
	int animfn = parseAnimFunc(t);
	return pAFunc[animfn].src;
}

static void parsePrimExtrusion(tokenIter_t &t)
{
	(*t++).MatchIdentifier("extrusion");
	(*t++).Check(Token::OPENBRACKET);
	Uint16 start = parseVtxOrVtxRef(t);
	(*t++).Check(Token::COMMA);
	Uint16 end = parseVtxOrVtxRef(t);
	(*t++).Check(Token::COMMA);
	Uint16 up = parseVtxOrVtxRef(t);
	(*t++).Check(Token::COMMA);
	(*t++).MatchIdentifier("radius");
	(*t++).Check(Token::ASSIGN);
	int rad = (int)((*t).GetFloat() * 100.0);
	if ((rad <= 0) || (rad > 65535))
		(*t).Error("Extrusion radius must be in range 0.01 to 655");
	++t;
	(*t++).Check(Token::COMMA);
	(*t++).Check(Token::OPENBRACE);
	int count = 1;
	Uint16 firstvtx = parseVtxOrVtxRef(t);
	while ((*t).type == Token::COMMA) {
		++t;
		Uint16 next = parseVtxOrVtxRef(t);
		if (next != firstvtx + count) {
			(*t).Error("Extrusion vertex list vertices must run in declaration order. ie {a,b,c} must have been declared in that order.");
		}
		count++;
	}
	if (count < 3) (*t).Error("Extrusion vertex list must contain at least 3 vertices");
	(*t++).Check(Token::CLOSEBRACE);
	(*t++).Check(Token::CLOSEBRACKET);

	instrsPutInstr(PTYPE_EXTRUSION);
	if ((start & IS_COMPLEX) ||
	    (end & IS_COMPLEX) ||
	    (up & IS_COMPLEX) ||
	    (firstvtx & IS_COMPLEX)) {
		instrsPutInstr(0x8000); // do not cache
	} else {
		instrsPutCacheIdx();
	}
	instrsPutInstr(count);
	instrsPutVtxRef(start);
	instrsPutVtxRef(end);
	instrsPutVtxRef(up);
	instrsPutInstr(rad);
	instrsPutVtxRef(firstvtx);
}

static void parseGeomInstructions(tokenIter_t &t)
{
	while ((*t).type != Token::CLOSEBRACE) {
		// parse model definition
		(*t).Check(Token::IDENTIFIER);
		lodActive[0] = lodActive[1] = lodActive[2] = lodActive[3] = false;

		bool flag_xref = false;
		bool flag_invisible = false;
		bool flag_thrust = false;
		bool flag_noang = false;
		// first the flags
		bool got_flag;
		bool got_lod_prefix = false;
		do {
			got_flag = false;

			if ((*t).IsIdentifier("xref")) {
				flag_xref = true;
				got_flag = true;
			}
			else if ((*t).IsIdentifier("thrust")) {
				flag_thrust = true;
				got_flag = true;
			}
			else if ((*t).IsIdentifier("notangular")) { // for thrusters
				flag_noang = true;
				got_flag = true;
			}
			else if ((*t).IsIdentifier("invisible")) {
				flag_invisible = true;
				got_flag = true;
			}
			else if ((*t).IsIdentifier("lod1")) {
				lodActive[0] = true;
				got_flag = true;
				got_lod_prefix = true;
			} else if ((*t).IsIdentifier("lod2")) {
				lodActive[1] = true;
				got_flag = true;
				got_lod_prefix = true;
			} else if ((*t).IsIdentifier("lod3")) {
				lodActive[2] = true;
				got_flag = true;
				got_lod_prefix = true;
			} else if ((*t).IsIdentifier("lod4")) {
				lodActive[3] = true;
				got_flag = true;
				got_lod_prefix = true;
			}
			if (got_flag) {
				++t;
				(*t++).Check(Token::COLON);
			}
		} while (got_flag);

		bool noneActive = true;
		for (int i=0; i<3; i++) {
			if (lodActive[i]) {
				noneActive = false;
				if (i>=numLods) (*t).Error("Lod prefix activates lod that does not exist");
			}
		}
		// no lod prefix = all active
		if (noneActive) for (int i=0; i<numLods; i++) lodActive[i] = true;

		/*static const int TFLAG_XCENTER = 0x8000;
static const int TFLAG_YCENTER = 0x4000;

static const int RFLAG_XREF = 0x8000;
static const int RFLAG_INVISIBLE = 0x4000;*/
	
		if ((*t).IsIdentifier("zbias")) {
			if (flag_noang || flag_invisible || flag_thrust || flag_xref) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			Uint16 pos = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 norm = parseVtxOrVtxRef(t);
			(*t++).Check(Token::CLOSEBRACKET);
			instrsPutInstr(PTYPE_ZBIAS);
			instrsPutVtxRef(pos);
			instrsPutVtxRef(norm);
			instrsPutInstr(0);

		} else if ((*t).IsIdentifier("nozbias")) {
			if (flag_noang || flag_invisible || flag_thrust || flag_xref) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			(*t++).Check(Token::CLOSEBRACKET);
			instrsPutInstr(PTYPE_ZBIAS);
			instrsPutInstr(0x8000);
			instrsPutInstr(0);
			instrsPutInstr(0);

		} else if ((*t).IsIdentifier("tri")) {
			if (flag_noang || flag_thrust) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			Uint16 v1 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 v2 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 v3 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::CLOSEBRACKET);
			instrsPutInstr(PTYPE_TRIFLAT |
					(flag_xref ? RFLAG_XREF : 0) | 
					(flag_invisible ? RFLAG_INVISIBLE : 0));
			instrsPutVtxRef(v1);
			instrsPutVtxRef(v2);
			instrsPutVtxRef(v3);
	
		} else if ((*t).IsIdentifier("quad")) {
			if (flag_noang || flag_thrust) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			Uint16 v1 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 v2 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 v3 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 v4 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::CLOSEBRACKET);
			instrsPutInstr(PTYPE_QUADFLAT |
					(flag_xref ? RFLAG_XREF : 0) | 
					(flag_invisible ? RFLAG_INVISIBLE : 0));
			instrsPutVtxRef(v1);
			instrsPutVtxRef(v2);
			instrsPutVtxRef(v3);
			instrsPutVtxRef(v4);

		} else if ((*t).IsIdentifier("circle")) {
			
			if (flag_noang || flag_invisible || flag_thrust) (*t).Error("Invalid flag");

			++t;
			(*t++).Check(Token::OPENBRACKET);
			Uint16 objLod[4];
			parseObjectLod(t, objLod);
			(*t++).Check(Token::COMMA);
			Uint16 vtx = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 norm = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 up = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			(*t++).MatchIdentifier("radius");
			(*t++).Check(Token::ASSIGN);
			int rad = (int)((*t).GetFloat() * 100.0);
			if ((rad <= 0) || (rad > 65535))
				(*t).Error("Circle radius must be in range 0.01 to 655");
			++t;
			(*t++).Check(Token::CLOSEBRACKET);
			instrsPutInstr(PTYPE_CIRCLE |
					(flag_xref ? RFLAG_XREF : 0));
			instrsPutInstr(0x8000); // not cached
			instrsPutObjectLod(objLod);
			instrsPutVtxRef(vtx);
			instrsPutVtxRef(norm);
			instrsPutVtxRef(up);
			instrsPutInstr(rad);

		} else if ((*t).IsIdentifier("cylinder")) {

			if (flag_noang || flag_invisible || flag_thrust) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			Uint16 objLod[4];
			parseObjectLod(t, objLod);
			(*t++).Check(Token::COMMA);
			Uint16 startvtx = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 endvtx = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 updir = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			(*t++).MatchIdentifier("radius");
			(*t++).Check(Token::ASSIGN);
			int rad = (int)((*t).GetFloat() * 100.0);
			if ((rad <= 0) || (rad > 65535))
				(*t).Error("Cylinder radius must be in range 0.01 to 655");
			++t;
			(*t++).Check(Token::CLOSEBRACKET);
			instrsPutInstr(PTYPE_CYLINDER |
					(flag_xref ? RFLAG_XREF : 0));
			if ((startvtx & IS_COMPLEX) ||
			    (endvtx & IS_COMPLEX) ||
			    (updir & IS_COMPLEX)) {
				instrsPutInstr(0x8000); // do not cache
			} else {
				instrsPutCacheIdx();
			}
			instrsPutObjectLod(objLod);
			instrsPutVtxRef(startvtx);
			instrsPutVtxRef(endvtx);
			instrsPutVtxRef(updir);
			instrsPutInstr(rad);

		} else if ((*t).IsIdentifier("tube")) {

			if (flag_noang || flag_invisible || flag_thrust) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			Uint16 objLod[4];
			parseObjectLod(t, objLod);
			(*t++).Check(Token::COMMA);
			Uint16 startvtx = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 endvtx = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 updir = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			(*t++).MatchIdentifier("innerrad");
			(*t++).Check(Token::ASSIGN);
			int innerrad = (int)((*t).GetFloat() * 100.0);
			if ((innerrad <= 0) || (innerrad > 65535))
				(*t).Error("Tube innerrad must be in range 0.01 to 655");
			++t;
			(*t++).Check(Token::COMMA);
			(*t++).MatchIdentifier("outerrad");
			(*t++).Check(Token::ASSIGN);
			int outerrad = (int)((*t).GetFloat() * 100.0);
			if ((outerrad <= 0) || (outerrad > 65535))
				(*t).Error("Tube innerrad must be in range 0.01 to 655");
			++t;
			(*t++).Check(Token::CLOSEBRACKET);
			instrsPutInstr(PTYPE_TUBE |
					(flag_xref ? RFLAG_XREF : 0));

			if ((startvtx & IS_COMPLEX) ||
			    (endvtx & IS_COMPLEX) ||
			    (updir & IS_COMPLEX)) {
				instrsPutInstr(0x8000); // do not cache
			} else {
				instrsPutCacheIdx();
			}
			instrsPutObjectLod(objLod);
			instrsPutVtxRef(startvtx);
			instrsPutVtxRef(endvtx);
			instrsPutVtxRef(updir);
			instrsPutInstr(outerrad);
			instrsPutInstr(innerrad);

		} else if ((*t).IsIdentifier("smooth")) {
			if (flag_noang || flag_invisible || flag_thrust) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			Uint16 objLod[4];
			parseObjectLod(t, objLod);
			instrsPutInstr(PTYPE_COMPOUND2S |
					(flag_xref ? RFLAG_XREF : 0) | 
					(flag_invisible ? RFLAG_INVISIBLE : 0));
			// XXX note always caching breaks animation if used
			instrsPutCacheIdx();
			instrsPutObjectLod(objLod);

			(*t++).Check(Token::COMMA);

			// get first vtx idx later, but make space for it
			int firstVtxFixup[4];
			getInstrsFixupPos(firstVtxFixup);
			instrsPutInstr(0);

			Uint16 firstVtx = parseCompObjectBits(t, true);
			(*t++).Check(Token::CLOSEBRACKET);
			instrsPutInstr(LTYPE_END);
			instrsPutVtxRef(firstVtx, firstVtxFixup);

		} else if ((*t).IsIdentifier("flat")) {
			if (flag_noang || flag_invisible || flag_thrust) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			Uint16 objLod[4];
			parseObjectLod(t, objLod);
			(*t++).Check(Token::COMMA);
			Uint16 norm = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			
			instrsPutInstr(PTYPE_COMPOUND2F |
					(flag_xref ? RFLAG_XREF : 0));
			// XXX note always caching breaks animation if used
			instrsPutCacheIdx();
			instrsPutObjectLod(objLod);
			int startVtxFixup[4];
			getInstrsFixupPos(startVtxFixup);
			instrsPutInstr(0); // must fixup with start vtx ref
			instrsPutVtxRef(norm);

			Uint16 startVtx = parseCompObjectBits(t, false);
			(*t++).Check(Token::CLOSEBRACKET);
			instrsPutInstr(LTYPE_END);
			// do fixup !!!!!!!!
			instrsPutVtxRef(startVtx, startVtxFixup);

		} else if ((*t).IsIdentifier("text")) {
			if (flag_noang || flag_invisible || flag_thrust || flag_xref) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			bool use_global_strings = false;
			if ((*t).IsIdentifier("global")) {
				use_global_strings = true;
			} else if (!(*t).IsIdentifier("params")) {
				(*t).Error("Text must index strings in either global: or params: arrays");
			}
			++t;
			(*t++).Check(Token::COLON);
			(*t).Check(Token::INTEGER);
			Uint16 string_idx = (*t++).val.i;
			(*t++).Check(Token::COMMA);
			Uint16 pos = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 norm = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 xaxis = parseVtxOrVtxRef(t);
			int xoff = 0;
			int yoff = 0;
			int scale = 100;
			int anim = 0x8000;
			while ((*t).type == Token::COMMA) {
				// get optional args
				++t;
				if ((*t).IsIdentifier("xoff")) {
					++t;
					(*t++).Check(Token::ASSIGN);
					xoff = (int)((*t++).GetFloat() * 100.0);
					if ((xoff < 0) || (xoff > 65535)) (*t).Error("Text xoff must be in range 0 to 655");
				} else if ((*t).IsIdentifier("yoff")) {
					++t;
					(*t++).Check(Token::ASSIGN);
					yoff = (int)((*t++).GetFloat() * 100.0);
					if ((yoff < 0) || (yoff > 65535)) (*t).Error("Text yoff must be in range 0 to 655");
				} else if ((*t).IsIdentifier("scale")) {
					++t;
					(*t++).Check(Token::ASSIGN);
					scale = (int)((*t++).GetFloat() * 100.0);
					if ((scale <= 0) || (scale > 65535)) (*t).Error("Text scale must be in range 0.01 to 655");
				} else if ((*t).IsIdentifier("animfn")) {
					anim = parseAnim(t);
				}
			}
			(*t++).Check(Token::CLOSEBRACKET);

			instrsPutInstr(PTYPE_TEXT | (use_global_strings ? TFLAG_STATIC : 0));
			instrsPutInstr(anim);
			instrsPutInstr(string_idx);
			instrsPutVtxRef(pos);
			instrsPutVtxRef(norm);
			instrsPutVtxRef(xaxis);
			instrsPutInstr(xoff);
			instrsPutInstr(yoff);
			instrsPutInstr(scale);

		} else if ((*t).IsIdentifier("material")) {
			if (flag_noang || flag_invisible || flag_thrust || flag_xref) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			int material_id = -1;
			if ((*t).IsIdentifier("animfn")) {
				// PFUNC_MATANIM
				int animfn = parseAnimFunc(t);
				int mat[20]; // 2 materials
				(*t++).Check(Token::COMMA);
				(*t++).MatchIdentifier("material");
				(*t++).Check(Token::OPENBRACKET);
				for (int i=0; i<10; i++) {
					mat[i] = (int)((*t++).GetFloat() * 100.0);
					if (i<9) (*t++).Check(Token::COMMA);
				}
				(*t++).Check(Token::CLOSEBRACKET);
				(*t++).Check(Token::COMMA);
				(*t++).MatchIdentifier("material");
				(*t++).Check(Token::OPENBRACKET);
				for (int i=10; i<20; i++) {
					mat[i] = (int)((*t++).GetFloat() * 100.0);
					if (i<19) (*t++).Check(Token::COMMA);
				}
				(*t++).Check(Token::CLOSEBRACKET);
				(*t++).Check(Token::CLOSEBRACKET);

				instrsPutInstr(PTYPE_MATANIM);
				instrsPutInstr(animfn);
				for (int i=0; i<20; i++) instrsPutInstr(mat[i]);
			} else {
				if ((*t).type == Token::INTEGER) material_id = (*t).val.i;
				int dr = (int)((*t++).GetFloat() * 100.0);
				if ((material_id != -1) && ((*t).type == Token::CLOSEBRACKET)) {
					// single argument MATVAR references params (0-2 materials)
					if ((material_id < 0) || (material_id > 2)) {
						(*t).Error("material id must be from 0 to 2");
					}
					(*t++).Check(Token::CLOSEBRACKET);

					instrsPutInstr(PTYPE_MATVAR);
					instrsPutInstr(material_id);
				} else {
					// MATFIXED
					(*t++).Check(Token::COMMA);
					int dg = (int)((*t++).GetFloat() * 100.0);
					(*t++).Check(Token::COMMA);
					int db = (int)((*t++).GetFloat() * 100.0);
					(*t++).Check(Token::COMMA);
					
					int sr = (int)((*t++).GetFloat() * 100.0);
					(*t++).Check(Token::COMMA);
					int sg = (int)((*t++).GetFloat() * 100.0);
					(*t++).Check(Token::COMMA);
					int sb = (int)((*t++).GetFloat() * 100.0);
					(*t++).Check(Token::COMMA);
					
					int shiny = (int)((*t++).GetFloat() * 100.0);
					(*t++).Check(Token::COMMA);
					int er = (int)((*t++).GetFloat() * 100.0);
					(*t++).Check(Token::COMMA);
					int eg = (int)((*t++).GetFloat() * 100.0);
					(*t++).Check(Token::COMMA);
					int eb = (int)((*t++).GetFloat() * 100.0);
					(*t++).Check(Token::CLOSEBRACKET);

					instrsPutInstr(PTYPE_MATFIXED);
					instrsPutInstr(dr);
					instrsPutInstr(dg);
					instrsPutInstr(db);
					instrsPutInstr(sr);
					instrsPutInstr(sg);
					instrsPutInstr(sb);
					instrsPutInstr(shiny);
					instrsPutInstr(er);
					instrsPutInstr(eg);
					instrsPutInstr(eb);
				}
			}

		} else if ((*t).IsIdentifier("extrusion")) {
			if (flag_noang || flag_invisible || flag_thrust || flag_xref) (*t).Error("Invalid flag");

			parsePrimExtrusion(t);

		} else if ((*t).IsIdentifier("subobject")) {
			
			if (flag_noang || flag_invisible || flag_xref) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			Uint16 model = get_model_reference(t);
			(*t++).Check(Token::COMMA);

			Uint16 offset = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 norm = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 zaxis = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			
			(*t++).MatchIdentifier("scale");
			(*t++).Check(Token::ASSIGN);
			int scale = (int)((*t).GetFloat() * 100.0);
			if ((scale <= 0) || (scale > 65535))
				(*t).Error("subobject scale must be in range 0.01 to 655");
			++t;
			Uint16 anim = 0x8000;
			if ((*t).type == Token::COMMA) {
				// get anim
				++t;
				anim = parseAnim(t);
			}
			(*t++).Check(Token::CLOSEBRACKET);

			instrsPutInstr(PTYPE_SUBOBJECT | (flag_thrust ? SUBOBJ_THRUST : 0));
			instrsPutInstr(anim);
			instrsPutInstr(model);
			instrsPutVtxRef(offset);
			instrsPutVtxRef(norm);
			instrsPutVtxRef(zaxis);
			instrsPutInstr(scale);

		} else if ((*t).IsIdentifier("geomflag")) {
			if (flag_noang || flag_invisible || flag_xref) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			(*t).Check(Token::INTEGER);
			Uint16 flag = (*t++).val.i;
			(*t++).Check(Token::CLOSEBRACKET);
			instrsPutInstr(PTYPE_SETCFLAG);
			instrsPutInstr(flag);

		} else if ((*t).IsIdentifier("thruster")) {
			if (flag_invisible || flag_thrust) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			Thruster thrust;
			thrust.detail = 0;
			if ((*t).IsIdentifier("fwd")) {
				thrust.dir = 2;
			} else if ((*t).IsIdentifier("rev")) {
				thrust.dir = 5;
			} else if ((*t).IsIdentifier("left")) {
				thrust.dir = 0;
			} else if ((*t).IsIdentifier("right")) {
				thrust.dir = 3;
			} else if ((*t).IsIdentifier("down")) {
				thrust.dir = 1;
			} else if ((*t).IsIdentifier("up")) {
				thrust.dir = 4;
			} else {
				(*t).Error("Expected thruster direction (fwd, rev, left, right, up, down)");
			}
			++t;
			(*t++).Check(Token::COMMA);
			thrust.pos = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			thrust.power = (*t++).GetFloat();
			(*t++).Check(Token::CLOSEBRACKET);

		       	thrust.dir |= (flag_xref ? THRUST_XREF : 0) | (flag_noang ? THRUST_NOANG : 0);
			for (int i=0; i<4; i++) {
				if (lodActive[i]) thrusters[i].push_back(thrust);
			}

		} else if ((*t).IsIdentifier("objlod")) {
			if (flag_noang || flag_invisible || flag_thrust || flag_xref || got_lod_prefix) (*t).Error("flags not valid with objlod assignment statement");
			++t;
			const char *name = (*t).val.s;
			(*t++).Check(Token::IDENTIFIER);
			(*t++).Check(Token::ASSIGN);
			(*t++).Check(Token::OPENBRACE);
			int idx = 0;
			obj_lod slod;
			for (idx=0; idx<numLods; idx++) {
				(*t).Check(Token::INTEGER);
				slod.divs[idx] = (*t++).val.i;
				if (idx < numLods-1) (*t++).Check(Token::COMMA);
			}
			(*t++).Check(Token::CLOSEBRACE);

			if (obj_lods.find(name) == obj_lods.end()) {
				obj_lods[name] = slod;
			} else {
				(*t).Error("Attempted redefinition of obj_lod %s", name);
			}

		} else {
			if (flag_noang || flag_invisible || flag_thrust || flag_xref || got_lod_prefix) (*t).Error("flags not valid with assignment statement");
			// assignment
			const char *identifier = (*t).val.s;
			++t;
			(*t++).Check(Token::ASSIGN);
			Uint16 vtx_id = parseNewVertex(t);
			if (vtx_id == 0x8000) {
				(*t).Error("Vertex expected");
			} else
			if (!VertexIdentifierExists(identifier)) {
				vertex_identifiers[identifier] = vtx_id;
			} else {
				(*t).Error("Vertex %s already defined", identifier);
			}
		}
	}
	// all streams must be terminated
	for (int i=0; i<4; i++) instrs[i].push_back(PTYPE_END);
}

void parseModel(tokenIter_t &t)
{
	const char *modelName;
	sbre_cache_size = 0;
	for (int i=0; i<4; i++) {
		compound_vertex_fixups_in_instrs[i].clear();
		instrs[i].clear();
		thrusters[i].clear();
		lodSize[i] = -1;
	}
	obj_lods.clear();
	compound_vertices.clear();
	vertices.clear();
	vertex_identifiers.clear();
	Model *m = new Model;
	memset(m, 0, sizeof(Model));
	m->scale = 1.0;
	m->radius = 1.0;

	// so start with a single level of detail, full:
	lodSize[0] = 0;

	// model(name,scale=234.0,radius=123.0)
	(*t++).Check(Token::OPENBRACKET);
	(*t).Check(Token::IDENTIFIER);
	printf("Model %s: ", (*t).val.s);
	modelName = (*t).val.s;
	t++;
	(*t++).Check(Token::COMMA);
	(*t++).MatchIdentifier("scale");
	(*t++).Check(Token::ASSIGN);
	m->scale = (*t++).GetFloat();
	(*t++).Check(Token::COMMA);
	(*t++).MatchIdentifier("radius");
	(*t++).Check(Token::ASSIGN);
	m->radius = (*t++).GetFloat();
	
	(*t++).Check(Token::CLOSEBRACKET);
	(*t++).Check(Token::OPENBRACE);

	numLods = 1;
	// see if lod definition exists
	if ((*t).IsIdentifier("lod")) {
		++t;
		(*t++).Check(Token::OPENBRACE);
		(*t++).IsIdentifier("full");
		float prevSize = FLT_MAX;
		while ((*t).type == Token::COMMA) {
			++t;
			if (numLods >= 4) (*t).Error("Too many LOD definitions (max 4)");
			lodSize[numLods] = (*t++).GetFloat();
			if (lodSize[numLods] > prevSize) {
				(*t).Error("LOD definitions must be ordered with decreasing pixel size");
			}
			(*t++).MatchIdentifier("px");
			prevSize = lodSize[numLods];
			numLods++;
		}
		(*t++).Check(Token::CLOSEBRACE);
	}
	
	parseGeomInstructions(t);

	// apply compound vertex index fixups
	int complex_fixup_offset = 6 + vertices.size() - IS_COMPLEX;
	for (int _lod=0; _lod<numLods; _lod++) {
		for (int i=0; i<(signed)compound_vertex_fixups_in_instrs[_lod].size(); i++) {
			int fixup_pos = compound_vertex_fixups_in_instrs[_lod][i];
			instrs[_lod][fixup_pos] += complex_fixup_offset;
		}
	}

	printf("%d plain vertices, %d compound vertices\n", vertices.size(), compound_vertices.size());
	m->numPVtx = 6+vertices.size();
	m->pPVtx = new PlainVertex[vertices.size()];
	for (int i=0; i<(signed)vertices.size(); i++) {
		m->pPVtx[i].type = VTYPE_PLAIN;
		m->pPVtx[i].pos.x = vertices[i].x;
		m->pPVtx[i].pos.y = vertices[i].y;
		m->pPVtx[i].pos.z = vertices[i].z;
	}
	m->numCache = sbre_cache_size;
	
	// for each lod copy instructions and give thrusters
	for (int _lod=0; _lod<numLods; _lod++) {
		// real lod idx on model
		int mlodIdx = numLods-_lod-1;
		// copy instructions to new turd
		Uint16 *data = new Uint16[instrs[_lod].size()];
		for (int i=0; i<(signed)instrs[_lod].size(); i++) {
			data[i] = instrs[_lod][i];
		}

		m->pLOD[mlodIdx].pixrad = lodSize[_lod];
		m->pLOD[mlodIdx].pData1 = data;
		m->pLOD[mlodIdx].pData2 = 0;
		m->pLOD[mlodIdx].numThrusters = thrusters[_lod].size();
		m->pLOD[mlodIdx].pThruster = new Thruster[m->pLOD[mlodIdx].numThrusters];
		for (int j=0; j<(signed)thrusters[_lod].size(); j++) {
			m->pLOD[mlodIdx].pThruster[j] = thrusters[_lod][j];
		}
	}

	// dummy complex vertex
	if (compound_vertices.size() == 0) {
		m->cvStart = m->numPVtx;
		m->numCVtx = 0;
		m->pCVtx = new CompoundVertex[1];
		m->pCVtx[0].type = VTYPE_CROSS;
		m->pCVtx[0].pParam[0] = 0;
		m->pCVtx[0].pParam[1] = 1;
		m->pCVtx[0].pParam[2] = 2;
		m->pCVtx[0].pParam[3] = -1;
		m->pCVtx[0].pParam[4] = -1;
	} else {
		m->cvStart = m->numPVtx;
		m->numCVtx = compound_vertices.size();
		m->pCVtx = new CompoundVertex[compound_vertices.size()];
		for (int i=0; i<(signed)compound_vertices.size(); i++) {
			m->pCVtx[i] = compound_vertices[i];
			/* fixup arg refs to complex vertices */
			for (int p=0; p<5; p++) {
				if (m->pCVtx[i].pParam[p] & IS_COMPLEX)
					m->pCVtx[i].pParam[p] += complex_fixup_offset;
			}
		}
	}
	
	ppTurdpiledModel[modelNum] = m;
	models_idx[modelName] = modelNum++;
	models[modelName] = m;
}

void parse(std::vector<Token> &tokens)
{
	std::vector<Token>::iterator i = tokens.begin();

	while (i != tokens.end()) {
		if ((*i).IsIdentifier("model")) {
			++i;
			parseModel(i);
			(*i++).Check(Token::CLOSEBRACE);
		} else if ((*i).type == Token::END) {
			break;
		} else {
			(*i).Error("Expected model");
		}
	}
}

void sbreCompilerLoadModels()
{
	FILE *f = fopen("data/models.def", "r");

	if (!f) {
		printf("Could not open data/models.def\n");
		exit(-1);
	}

	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = new char[size+1];
	buf[size] = 0;
	fread(buf, size, 1, f);

	std::vector<Token> tokens;
	lex(buf, tokens);
	parse(tokens);
	delete [] buf;
}
