
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <vector>
#include <string>
#include <map>
#include "sbre/sbre_int.h"
#include "vector3.h"

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
		if (this->type != type) Error("Unexpected token type %d", type);
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
	bool MatchIdentifier(const char *str) {
		return (type == IDENTIFIER) && (strcmp(str, val.s)==0);
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

static std::map<std::string, Model*> models;
static std::vector<vector3f> vertices;
static std::map<std::string, int> vertex_identifiers;
static Model *cur_model;

static bool VertexIdentifierExists(const char *vid)
{
	std::map<std::string, int>::iterator i = vertex_identifiers.find(vid);
	return (i != vertex_identifiers.end());
}

/*
 * Returns index of plain vertex
 */
static int parsePlainVtx(tokenIter_t &t)
{
	vector3f v;
	bool normalize = false;
	if ((*t).MatchIdentifier("v")) {

	} else if ((*t).MatchIdentifier("n")) {
		normalize = true;
	} else {
		(*t).Error("Expected vertex");
	}

	++t;
	(*t++).Check(Token::OPENBRACKET);
	v.x = (*t++).GetFloat();
	(*t++).Check(Token::COMMA);
	v.y = (*t++).GetFloat();
	(*t++).Check(Token::COMMA);
	v.z = (*t++).GetFloat();
	(*t++).Check(Token::CLOSEBRACKET);
	if (normalize) v = v.Normalized();
	printf("%f,%f,%f\n", v.x, v.y, v.z);
	// first 6 are +ve and -ve axis unit vectors
	int idx = 6 + vertices.size();
	vertices.push_back(v);
	return idx;
}

/* return vertex identifier idx */
static int parsePlainVtxOrVtxRef(tokenIter_t &t)
{
	//int numPVtx;				// number of plain vertices + 6
	(*t).Check(Token::IDENTIFIER);
	if ((*t).MatchIdentifier("v") ||
	    (*t).MatchIdentifier("n")) {
		return parsePlainVtx(t);
	} else {
		printf("Turd\n");
		std::map<std::string, int>::iterator i = vertex_identifiers.find((*t).val.s);
		if (i == vertex_identifiers.end()) {
			(*t).Error("Unknown vertex identifier %s", (*t).val.s);
		}
		++t;
		return (*i).second;
	}
}

void parseModel(tokenIter_t &t)
{
	std::vector<Uint16> instrs;
	Model *m = new Model;
	memset(m, 0, sizeof(Model));

	(*t++).Check(Token::OPENBRACKET);
	(*t).Check(Token::IDENTIFIER);
	printf("Model %s:\n", (*t).val.s);
	models[(*t).val.s] = m;
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
		// first the flags
		bool got_flag;
		do {
			got_flag = false;

			if ((*t).MatchIdentifier("xref")) {
				flag_xref = true;
				got_flag = true;
			}
			else if ((*t).MatchIdentifier("ycenter")) {
				flag_ycenter = true;
				got_flag = true;
			}
			else if ((*t).MatchIdentifier("xcenter")) {
				flag_xcenter = true;
				got_flag = true;
			}
			else if ((*t).MatchIdentifier("invisible")) {
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
	
		if ((*t).MatchIdentifier("tri")) {
			if (flag_xcenter || flag_ycenter) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			Uint16 v1 = parsePlainVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 v2 = parsePlainVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 v3 = parsePlainVtxOrVtxRef(t);
			(*t++).Check(Token::CLOSEBRACKET);
			instrs.push_back(PTYPE_TRIFLAT |
					(flag_xref ? RFLAG_XREF : 0) | 
					(flag_invisible ? RFLAG_INVISIBLE : 0));
			instrs.push_back(v1);
			instrs.push_back(v2);
			instrs.push_back(v3);
	
		} else if ((*t).MatchIdentifier("quad")) {
			if (flag_xcenter || flag_ycenter) (*t).Error("Invalid flag");
			++t;
			(*t++).Check(Token::OPENBRACKET);
			Uint16 v1 = parsePlainVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 v2 = parsePlainVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 v3 = parsePlainVtxOrVtxRef(t);
			(*t++).Check(Token::COMMA);
			Uint16 v4 = parsePlainVtxOrVtxRef(t);
			(*t++).Check(Token::CLOSEBRACKET);
			instrs.push_back(PTYPE_QUADFLAT |
					(flag_xref ? RFLAG_XREF : 0) | 
					(flag_invisible ? RFLAG_INVISIBLE : 0));
			instrs.push_back(v1);
			instrs.push_back(v2);
			instrs.push_back(v3);
			instrs.push_back(v4);
		} else {
			printf("assignment\n");
			// assignment
			const char *identifier = (*t).val.s;
			++t;
			(*t++).Check(Token::ASSIGN);
			Uint16 vtx_id = parsePlainVtx(t);
			if (!VertexIdentifierExists(identifier)) {
				vertex_identifiers[identifier] = vtx_id;
			} else {
				(*t).Error("Vertex %s already defined", identifier);
			}
		}
	}
	instrs.push_back(PTYPE_END);

	printf("%d plain vertices\n", vertices.size());
	m->numPVtx = 6+vertices.size();
	m->pPVtx = new PlainVertex[vertices.size()];
	for (int i=0; i<vertices.size(); i++) {
		m->pPVtx[i].type = VTYPE_PLAIN;
		m->pPVtx[i].pos.x = vertices[i].x;
		m->pPVtx[i].pos.y = vertices[i].y;
		m->pPVtx[i].pos.z = vertices[i].z;
	}
	m->scale = 1.0;
	m->radius = 1.0;

	m->numCache = 0;
	
	m->pLOD[0].pixrad = 0;
	m->pLOD[0].pData1 = new Uint16[instrs.size()];
	m->pLOD[0].pData2 = 0;
	m->pLOD[0].numThrusters = 0;
	m->pLOD[0].pThruster = 0;

	Uint16 *data = m->pLOD[0].pData1;
	for (int i=0; i<instrs.size(); i++) {
		*(data++) = instrs[i];
	}

	// dummy complex vertex
	m->cvStart = m->numPVtx;
	m->pCVtx = new CompoundVertex[1];
	m->pCVtx[0].type = VTYPE_CROSS;
	m->pCVtx[0].pParam[0] = 0;
	m->pCVtx[0].pParam[1] = 1;
	m->pCVtx[0].pParam[2] = 2;
	m->pCVtx[0].pParam[3] = -1;
	m->pCVtx[0].pParam[4] = -1;

	ppModel[0] = m;
}

void parse(std::vector<Token> &tokens)
{
	std::vector<Token>::iterator i = tokens.begin();

	while (i != tokens.end()) {
		if ((*i).MatchIdentifier("model")) {
			++i;
			parseModel(i);
			return;
		} else if ((*i).type == Token::END) {
			break;
		} else {
			(*i).Error("Expected model");
		}
	}
}

void model_compiler_test()
{
	FILE *f = fopen("sbre2.obj", "r");

	char *buf = new char[4096];
	memset(buf, 0, 4096);
	fread(buf, 4096, 1, f);

	std::vector<Token> tokens;
	lex(buf, tokens);
	printf("%d tokens\n", tokens.size());
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
}
