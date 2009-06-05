
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <vector>
#include <string>
#include <map>
#include "sbre_int.h"
#include "sbre_models.h"

extern Model * ppModel[];

#define MAX_TOKEN_LEN 64

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
		Error("Expected float");
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
			error(lexLine, "Unknown token");
		}
	}
	t = Token();
	t.type = Token::END;
	tokens.push_back(t);
}

typedef std::vector<Token>::iterator tokenIter_t;

#define IS_COMPLEX (0x4000)

int modelNum = SBRE_COMPILED_MODELS;
static std::map<std::string, Model*> models;
static std::map<std::string, int> models_idx;
static std::vector<Vector> vertices;
static std::map<std::string, int> vertex_identifiers;
static std::vector<Uint16> instrs;
int sbre_cache_size;
static std::vector<CompoundVertex> compound_vertices;
static std::vector<int> compound_vertex_fixups_in_instrs;

static bool VertexIdentifierExists(const char *vid)
{
	std::map<std::string, int>::iterator i = vertex_identifiers.find(vid);
	return (i != vertex_identifiers.end());
}

static int addPlainVtx(Vector v)
{
	// first 6 are +ve and -ve axis unit vectors
	int idx = 6 + vertices.size();
	vertices.push_back(v);
	return idx;
}

static void instrsPutVtxRef(Uint16 ref)
{
	if (ref & IS_COMPLEX) {
		compound_vertex_fixups_in_instrs.push_back(instrs.size());
	}
	instrs.push_back(ref);
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

	} else if ((*t).IsIdentifier("normal")) {

		++t;
		(*t++).Check(Token::OPENBRACKET);
		Uint16 v1 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		Uint16 v2 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		Uint16 v3 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::CLOSEBRACKET);
		if ((v1 & IS_COMPLEX) ||
		    (v2 & IS_COMPLEX) ||
		    (v3 & IS_COMPLEX)) (*t).Error("Cannot declare normal() using other complex vertices");

		return addCompoundVtx(VTYPE_NORM, v1, v2, v3, -1, -1);
	
	} else if ((*t).IsIdentifier("cross")) {

		++t;
		(*t++).Check(Token::OPENBRACKET);
		Uint16 v1 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		Uint16 v2 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::CLOSEBRACKET);
		if ((v1 & IS_COMPLEX) ||
		    (v2 & IS_COMPLEX)) (*t).Error("Cannot declare cross() using other complex vertices");

		return addCompoundVtx(VTYPE_CROSS, v1, v2, -1, -1, -1);

	} else if ((*t).IsIdentifier("animlin")) {

		++t;
		(*t++).Check(Token::OPENBRACKET);
		Uint16 v1 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::COMMA);
		Uint16 v2 = parseVtxOrVtxRef(t);
		(*t++).Check(Token::CLOSEBRACKET);
		if ((v1 & IS_COMPLEX) ||
		    (v2 & IS_COMPLEX)) (*t).Error("Cannot declare animlin() using other complex vertices");

		return addCompoundVtx(VTYPE_ANIMLIN, v1, v2, -1, -1, AFUNC_GEAR);

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

static void comp_get_line_bits(tokenIter_t &t)
{
	int num = 0;
	for (;;) {
		if ((*t).IsIdentifier("hermite")) {
			++t;
			(*t++).Check(Token::OPENBRACKET);
			Uint16 v1 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 v2 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 v3 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::CLOSEBRACKET);
			instrs.push_back(LTYPE_HERMITE);
			instrsPutVtxRef(v1);
			instrsPutVtxRef(v2);
			instrsPutVtxRef(v3);
			num++;
		} else if ((*t).IsIdentifier("line")) {
			++t;
			(*t++).Check(Token::OPENBRACKET);
			Uint16 v1 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::CLOSEBRACKET);
			instrs.push_back(LTYPE_LINE);
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
	if (num<2) (*t).Error("Expected at least 2 hermite or line segments");
	else if (num>4) (*t).Error("Expected at most 4 hermite or line segments");
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

static Uint16 get_anim(tokenIter_t &t)
{
	(*t++).MatchIdentifier("anim");
	(*t++).Check(Token::ASSIGN);
	if ((*t).type != Token::INTEGER) {
		(*t).Error("Integer anim expected");
		return 0;
	} else {
		Uint16 anim = (*t).val.i;
		if (anim > 9) {
			(*t).Error("Anim must be in range 0-9");
		}
		++t;
		return anim;
	}
}

void parseModel(tokenIter_t &t)
{
	const char *modelName;
	sbre_cache_size = 0;
	compound_vertex_fixups_in_instrs.clear();
	compound_vertices.clear();
	vertices.clear();
	vertex_identifiers.clear();
	instrs.clear();
	Model *m = new Model;
	memset(m, 0, sizeof(Model));
	(*t++).Check(Token::OPENBRACKET);
	(*t).Check(Token::IDENTIFIER);
	printf("Model %s:\n", (*t).val.s);
	modelName = (*t).val.s;
	t++;
	(*t++).Check(Token::CLOSEBRACKET);
	(*t++).Check(Token::OPENBRACE);
	
	while ((*t).type != Token::CLOSEBRACE) {
		// parse model definition
		(*t).Check(Token::IDENTIFIER);

		bool flag_xcenter = false;
		bool flag_ycenter = false;
		bool flag_xref = false;
		bool flag_invisible = false;
		bool flag_thrust = false;
		// first the flags
		bool got_flag;
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
			else if ((*t).IsIdentifier("ycenter")) {
				flag_ycenter = true;
				got_flag = true;
			}
			else if ((*t).IsIdentifier("xcenter")) {
				flag_xcenter = true;
				got_flag = true;
			}
			else if ((*t).IsIdentifier("invisible")) {
				flag_invisible = true;
				got_flag = true;
			}
			if (got_flag) {
				++t;
				(*t++).Check(Token::COLON);
			}
		} while (got_flag);
		/*static const int TFLAG_XCENTER = 0x8000;
static const int TFLAG_YCENTER = 0x4000;

static const int RFLAG_XREF = 0x8000;
static const int RFLAG_INVISIBLE = 0x4000;*/
	
		if ((*t).IsIdentifier("tri")) {
			if (flag_xcenter || flag_ycenter || flag_thrust) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			Uint16 v1 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 v2 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 v3 = parseVtxOrVtxRef(t);
			(*t++).Check(Token::CLOSEBRACKET);
			instrs.push_back(PTYPE_TRIFLAT |
					(flag_xref ? RFLAG_XREF : 0) | 
					(flag_invisible ? RFLAG_INVISIBLE : 0));
			instrsPutVtxRef(v1);
			instrsPutVtxRef(v2);
			instrsPutVtxRef(v3);
	
		} else if ((*t).IsIdentifier("quad")) {
			if (flag_xcenter || flag_ycenter || flag_thrust) (*t).Error("Invalid flag");
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
			instrs.push_back(PTYPE_QUADFLAT |
					(flag_xref ? RFLAG_XREF : 0) | 
					(flag_invisible ? RFLAG_INVISIBLE : 0));
			instrsPutVtxRef(v1);
			instrsPutVtxRef(v2);
			instrsPutVtxRef(v3);
			instrsPutVtxRef(v4);

		} else if ((*t).IsIdentifier("circle")) {
			
			if (flag_xcenter || flag_ycenter || flag_invisible || flag_thrust) (*t).Error("Invalid flag");

			++t;
			(*t++).Check(Token::OPENBRACKET);
			(*t).Check(Token::INTEGER);
			Uint16 steps = (*t++).val.i;
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
			instrs.push_back(PTYPE_CIRCLE |
					(flag_xref ? RFLAG_XREF : 0));
			instrs.push_back(0x8000); // not cached
			instrs.push_back(steps);
			instrsPutVtxRef(vtx);
			instrsPutVtxRef(norm);
			instrsPutVtxRef(up);
			instrs.push_back(rad);

		} else if ((*t).IsIdentifier("cylinder")) {

			if (flag_xcenter || flag_ycenter || flag_invisible || flag_thrust) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			(*t).Check(Token::INTEGER);
			Uint16 steps = (*t++).val.i;
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
			instrs.push_back(PTYPE_CYLINDER |
					(flag_xref ? RFLAG_XREF : 0));
			if ((startvtx & IS_COMPLEX) ||
			    (endvtx & IS_COMPLEX) ||
			    (updir & IS_COMPLEX)) {
				instrs.push_back(0x8000); // do not cache
			} else {
				instrs.push_back(sbre_cache_size++);
			}
			instrs.push_back(steps);
			instrsPutVtxRef(startvtx);
			instrsPutVtxRef(endvtx);
			instrsPutVtxRef(updir);
			instrs.push_back(rad);

		} else if ((*t).IsIdentifier("tube")) {

			if (flag_xcenter || flag_ycenter || flag_invisible || flag_thrust) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			(*t).Check(Token::INTEGER);
			Uint16 steps = (*t++).val.i;
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
			instrs.push_back(PTYPE_TUBE |
					(flag_xref ? RFLAG_XREF : 0));

			if ((startvtx & IS_COMPLEX) ||
			    (endvtx & IS_COMPLEX) ||
			    (updir & IS_COMPLEX)) {
				instrs.push_back(0x8000); // do not cache
			} else {
				instrs.push_back(sbre_cache_size++);
			}
			instrs.push_back(steps);
			instrsPutVtxRef(startvtx);
			instrsPutVtxRef(endvtx);
			instrsPutVtxRef(updir);
			instrs.push_back(outerrad);
			instrs.push_back(innerrad);

		} else if ((*t).IsIdentifier("smooth")) {
			if (flag_xcenter || flag_ycenter || flag_invisible || flag_thrust) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			(*t).Check(Token::INTEGER);
			int steps = (*t).val.i;
			if ((steps<1) || (steps>99)) (*t).Error("Too many steps for smooth(). Must be 1-99.");
			
			instrs.push_back(PTYPE_COMPSMOOTH2 |
					(flag_xref ? RFLAG_XREF : 0) | 
					(flag_invisible ? RFLAG_INVISIBLE : 0));
			// XXX note always caching breaks animation if used
			instrs.push_back(sbre_cache_size++);
			instrs.push_back(steps);

			++t;
			(*t++).Check(Token::COMMA);
			Uint16 v1 = parseVtxOrVtxRef(t);
			instrsPutVtxRef(v1);
			(*t++).Check(Token::COMMA);

			comp_get_line_bits(t);
			(*t++).Check(Token::CLOSEBRACKET);
			instrs.push_back(LTYPE_END);

	/*	} else if ((*t).IsIdentifier("cuboid")) {
			if (flag_xcenter || flag_ycenter || flag_thrust) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			Uint16 pos = parseVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
*/

		} else if ((*t).IsIdentifier("subobject")) {
			
			if (flag_xcenter || flag_ycenter || flag_invisible || flag_xref) (*t).Error("Invalid flag");
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
				anim = get_anim(t);
			}
			(*t++).Check(Token::CLOSEBRACKET);

			instrs.push_back(PTYPE_SUBOBJECT | (flag_thrust ? SUBOBJ_THRUST : 0));
			instrs.push_back(anim);
			instrs.push_back(model);
			instrsPutVtxRef(offset);
			instrsPutVtxRef(norm);
			instrsPutVtxRef(zaxis);
			instrs.push_back(scale);

		} else {
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
	instrs.push_back(PTYPE_END);
	
	// apply compound vertex index fixups
	int complex_fixup_offset = 6 + vertices.size() - IS_COMPLEX;
	for (int i=0; i<(signed)compound_vertex_fixups_in_instrs.size(); i++) {
		int fixup_pos = compound_vertex_fixups_in_instrs[i];
		instrs[fixup_pos] += complex_fixup_offset;
	}

	printf("%d plain vertices\n", vertices.size());
	printf("%d compound vertices\n", compound_vertices.size());
	m->numPVtx = 6+vertices.size();
	m->pPVtx = new PlainVertex[vertices.size()];
	for (int i=0; i<(signed)vertices.size(); i++) {
		m->pPVtx[i].type = VTYPE_PLAIN;
		m->pPVtx[i].pos.x = vertices[i].x;
		m->pPVtx[i].pos.y = vertices[i].y;
		m->pPVtx[i].pos.z = vertices[i].z;
	}
	m->scale = 1.0;
	m->radius = 1.0;

	m->numCache = sbre_cache_size;
	
	m->pLOD[0].pixrad = 0;
	m->pLOD[0].pData1 = new Uint16[instrs.size()];
	m->pLOD[0].pData2 = 0;
	m->pLOD[0].numThrusters = 0;
	m->pLOD[0].pThruster = 0;

	Uint16 *data = m->pLOD[0].pData1;
	for (int i=0; i<(signed)instrs.size(); i++) {
		printf("%d ", instrs[i]);
		*(data++) = instrs[i];
	}
	printf("\n");

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
		}
	}

	ppModel[modelNum] = m;
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

void model_compiler_test()
{
	FILE *f = fopen("data/models.def", "r");

	char *buf = new char[4096];
	memset(buf, 0, 4096);
	fread(buf, 4096, 1, f);

	std::vector<Token> tokens;
	lex(buf, tokens);
/*	for (std::vector<Token>::iterator i = tokens.begin(); i!= tokens.end(); ++i) {
		switch ((*i).type) {
			case Token::OPENBRACKET: printf("(\n"); break;
			case Token::CLOSEBRACKET: printf(")\n"); break;
			case Token::OPENBRACE: printf("{\n"); break;
			case Token::CLOSEBRACE: printf("}\n"); break;
			case Token::ASSIGN: printf("=\n"); break;
			case Token::COMMA: printf(", \n"); break;
			case Token::FLOAT: printf("%.1f\n", (*i).val.f); break;
			case Token::INTEGER: printf("%d\n", (*i).val.i); break;
			case Token::IDENTIFIER: printf("%s\n", (*i).val.s); break;
			case Token::END: printf("END\n"); break;
		}
	}*/
	parse(tokens);
	delete [] buf;
}
