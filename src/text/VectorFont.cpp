#include "VectorFont.h"
#include "libs.h"
#include "FileSystem.h"

#define PARAGRAPH_SPACING 1.5f

#ifdef _WIN32
// to avoid including reams and reams of headers just for a couple of macros, they're duplicated here
#define APIENTRY __stdcall
#define CALLBACK __stdcall
typedef GLvoid (APIENTRY *_GLUfuncptr)();
#endif
#ifdef __APPLE__
// So much for 'standards'
typedef GLvoid (*_GLUfuncptr)();
#endif

namespace Text {

static GLUtesselator *tobj = 0;

static inline double fac(int n)
{
	double r = 1.0;
	for (double i=2; i<=n; i++) {
		r *= i;
	}
	return r;
}

static inline double binomial_coeff(int n, int m)
{
	return fac(n)/(fac(m)*(fac(n-m)));
}

static void eval_bezier (GLdouble *out, double t, int n_points, double *points)
{
	// it always is the case for truetype, isn't it? in which case we can
	// use a much simpler bezier evaluator
	assert(n_points == 3);
	std::vector<double> c(n_points);

	for (int i=0; i<n_points; i++) {
		c[i] = pow(1.0f-t, n_points-(i+1)) * pow(t,i) *
			binomial_coeff(n_points-1, i);
	}

	out[0] = out[1] = out[2] = 0;

	for (int i=0; i<n_points; i++) {
		out[0] += points[3*i] * c[i];
		out[1] += points[3*i+1] * c[i];
		out[2] += points[3*i+2] * c[i];
	}
}

#define DIV 2048.0f

bool GenContourPoints(int a_char, FT_Outline *a_outline, const int a_contour, int a_bezierIters, std::vector<double> *ao_points)
{
#define push_point(__p) { \
	ao_points->push_back((__p)[0]); \
	ao_points->push_back((__p)[1]); \
	ao_points->push_back((__p)[2]); \
	}

	int cont = (a_contour-1 < 0 ? 0 : 1+a_outline->contours[a_contour-1]);

	double point_buf[256][3];
	char point_type[256];
	int pos = 0;

	for (; cont<=a_outline->contours[a_contour]; cont++, pos++) {
		point_type[pos] = a_outline->tags[cont];
		point_buf[pos][0] = a_outline->points[cont].x/DIV;
		point_buf[pos][1] = a_outline->points[cont].y/DIV;
		point_buf[pos][2] = 0;
	}
	if (!point_type[pos-1]) {
		// need to duplicate first vertex if last
		// section is a bezier
		point_type[pos] = 1;
		point_buf[pos][0] = point_buf[0][0];
		point_buf[pos][1] = point_buf[0][1];
		point_buf[pos][2] = 0;
		pos++;
	}

	int start = -1;
	for (int k=0; k<pos; k++) {
		if (!(point_type[k] & 1)) continue;
			
		if (start == -1) { start = k; continue; }
		
		int len = 1+k-start;
		// trace segment
		if (len == 2) {
			// straight line
			push_point(point_buf[k-1]);
			push_point(point_buf[k]);
		} else {
			// bezier
			double b_in[3][3];
			double v[3];

			// truetype is all quadratic bezier,
			// using average points between
			// 'control points' as end points
			
			// first bezier
			b_in[0][0] = point_buf[start][0];
			b_in[0][1] = point_buf[start][1];
			b_in[0][2] = 0;

			b_in[1][0] = point_buf[start+1][0];
			b_in[1][1] = point_buf[start+1][1];
			b_in[1][2] = 0;

			if (len > 3) {
				b_in[2][0] = 0.5 * (point_buf[start+1][0] + point_buf[start+2][0]);
				b_in[2][1] = 0.5 * (point_buf[start+1][1] + point_buf[start+2][1]);
				b_in[2][2] = 0;
			} else {
				b_in[2][0] = point_buf[start+2][0];
				b_in[2][1] = point_buf[start+2][1];
				b_in[2][2] = 0;
			}

			for (int l=0; l<=a_bezierIters; l++) {
				double t = (1.0/a_bezierIters)*l;
				eval_bezier(v, t, 3, &b_in[0][0]);
				v[2] = 0.0;
				push_point(v);
			}

			// middle beziers
			
			if (len > 4) {
				for (int _p=1; _p < len-3; _p++) {
					b_in[0][0] = 0.5*(point_buf[start+_p][0] + point_buf[start+_p+1][0]);
					b_in[0][1] = 0.5*(point_buf[start+_p][1] + point_buf[start+_p+1][1]);
					b_in[0][2] = 0;

					b_in[1][0] = point_buf[start+_p+1][0];
					b_in[1][1] = point_buf[start+_p+1][1];
					b_in[1][2] = 0;

					b_in[2][0] = 0.5*(point_buf[start+_p+1][0] + point_buf[start+_p+2][0]);
					b_in[2][1] = 0.5*(point_buf[start+_p+1][1] + point_buf[start+_p+2][1]);
					b_in[2][2] = 0;
					
					for (int l=0; l<=a_bezierIters; l++) {
						double t = (1.0/a_bezierIters)*l;
						eval_bezier(v, t, 3, &b_in[0][0]);
						v[2] = 0.0;
						push_point(v);
					}
				}
			}

			// end
			if (len > 3) {
				const int _p = start+len-3;
				b_in[0][0] = 0.5 * (point_buf[_p][0] + point_buf[_p+1][0]);
				b_in[0][1] = 0.5 * (point_buf[_p][1] + point_buf[_p+1][1]);
				b_in[0][2] = 0;

				b_in[1][0] = point_buf[_p+1][0];
				b_in[1][1] = point_buf[_p+1][1];
				b_in[1][2] = 0;

				b_in[2][0] = point_buf[_p+2][0];
				b_in[2][1] = point_buf[_p+2][1];
				b_in[2][2] = 0;

				for (int l=0; l<=a_bezierIters; l++) {
					double t = (1.0/a_bezierIters)*l;
					eval_bezier(v, t, 3, &b_in[0][0]);
					v[2] = 0.0;
					push_point(v);
				}
			}
		}
		start = k;
	}
	return true;
}

#ifndef CALLBACK
# ifdef WIN32
#  define CALLBACK __attribute__ ((__stdcall__))
# else
#  define CALLBACK
# endif
#endif /* CALLBACK */



struct TessData
{
	std::vector<double> *pts;		// inputs, added by combine
	int numvtx;

	std::vector<Uint16> index;		// output index list
	GLenum lasttype;
	int state;		// 0, no vertices, 1, 1 vertex, 2, 2 or more
					// 0x4 => clockwise
	Uint16 vtx[2];
};

static Uint16 g_index[65536];

void CALLBACK beginCallback(GLenum which, GLvoid *poly_data)
{
	TessData *pData = static_cast<TessData *>(poly_data);
	pData->lasttype = which;
	pData->state = 0;
}

void CALLBACK errorCallback(GLenum errorCode)
{
	const GLubyte *estring;

	estring = gluErrorString(errorCode);
	fprintf(stderr, "Tessellation Error: %s\n", estring);
}

void CALLBACK endCallback(void)
{
}

void CALLBACK vertexCallback(GLvoid *vertex, GLvoid *poly_data)
{
	TessData *pData = static_cast<TessData *>(poly_data);
	Uint16 index = *static_cast<Uint16 *>(vertex);
	switch (pData->lasttype)
	{
		case GL_TRIANGLES:
		pData->index.push_back(index);
		break;
	
		case GL_TRIANGLE_STRIP:
		if ((pData->state & 3) < 2)
			pData->vtx[pData->state++] = index;
		else {
			pData->index.push_back(index);
			if (pData->state & 0x4) {
				pData->index.push_back(pData->vtx[1]);
				pData->index.push_back(pData->vtx[0]);
			} else {
				pData->index.push_back(pData->vtx[0]);
				pData->index.push_back(pData->vtx[1]);
			}
			pData->vtx[0] = pData->vtx[1];
			pData->vtx[1] = index;
			pData->state ^= 0x4;
		}
		break;

		case GL_TRIANGLE_FAN:
		if ((pData->state & 3) < 2)
			pData->vtx[pData->state++] = index;
		else {
			pData->index.push_back(index);
			pData->index.push_back(pData->vtx[0]);
			pData->index.push_back(pData->vtx[1]);
			pData->vtx[1] = index;
		}		
	}
}

void CALLBACK combineCallback(GLdouble coords[3], 
                     GLdouble *vertex_data[4],
                     GLfloat weight[4], void **dataOut, void *poly_data)
{
	TessData *pData = static_cast<TessData *>(poly_data);
	pData->pts->push_back(coords[0]);
	pData->pts->push_back(coords[1]);
	pData->pts->push_back(coords[2]);
	*dataOut = static_cast<void *>(&g_index[pData->numvtx++]);
}

#define BEZIER_STEPS 2

void VectorFont::RenderGlyph(int chr)
{
	glEnableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_NORMAL_ARRAY);

	glfglyph_t *glyph = &m_glyphs[chr];
	glVertexPointer (3, GL_FLOAT, 3*sizeof(float), glyph->varray);
	glDrawElements (GL_TRIANGLES, glyph->numidx, GL_UNSIGNED_SHORT, glyph->iarray);
}

void VectorFont::MeasureString(const char *str, float &w, float &h)
{
	w = 0;
	h = GetHeight();
	float line_width = 0;
	for (unsigned int i=0; i<strlen(str); i++) {
		if (str[i] == '\n') {
			if (line_width > w) w = line_width;
			line_width = 0;
			h += GetHeight() * PARAGRAPH_SPACING;
		} else {
			line_width += m_glyphs[str[i]].advx;
		}
	}
	if (line_width > w) w = line_width;
}

void VectorFont::RenderString(const char *str)
{
	glPushMatrix();
	for (unsigned int i=0; i<strlen(str); i++) {
		if (str[i] == '\n') {
			glPopMatrix();
			glTranslatef(0, -m_height*PARAGRAPH_SPACING, 0);
			glPushMatrix();
		} else {
			glfglyph_t *glyph = &m_glyphs[str[i]];
			if (glyph->numidx) RenderGlyph(str[i]);
			glTranslatef(glyph->advx,0,0);
		}
	}
	glPopMatrix();
}

void VectorFont::GetStringGeometry(const char *str,
		void (*index_callback)(int num, Uint16 *vals),
		void (*vertex_callback)(int num, float offsetX, float offsetY, float *vals))
{
	float offX, offY;
	offX = offY = 0;
	for (unsigned int i=0; i<strlen(str); i++) {
		if (str[i] == '\n') {
			offX = 0;
			offY -= m_height*PARAGRAPH_SPACING;
		} else {
			glfglyph_t *glyph = &m_glyphs[str[i]];
			if (glyph->numidx) {
				(*index_callback)(glyph->numidx, glyph->iarray);
				(*vertex_callback)(glyph->numvtx, offX, offY, glyph->varray);
			}
			offX += glyph->advx;
		}
	}
}

// 'Markup' indeed. #rgb hex is change color, no sensible escape
void VectorFont::RenderMarkup(const char *str)
{
	glPushMatrix();
	int len = strlen(str);
	for (int i=0; i<len; i++) {
		if (str[i] == '#') {
			unsigned int hexcol;
			if (sscanf(str+i, "#%3x", &hexcol)==1) {
				Uint8 col[3];
				col[0] = (hexcol&0xf00)>>4;
				col[1] = (hexcol&0xf0);
				col[2] = (hexcol&0xf)<<4;
				glColor3ubv(col);
				i+=3;
				continue;
			}
		}
		if (str[i] == '\n') {
			glPopMatrix();
			glTranslatef(0,-m_height*PARAGRAPH_SPACING,0);
			glPushMatrix();
		} else {
			glfglyph_t *glyph = &m_glyphs[str[i]];
			if (glyph->numidx) RenderGlyph(str[i]);
			glTranslatef(glyph->advx,0,0);
		}
	}
	glPopMatrix();
}

VectorFont::VectorFont(const FontDescriptor &descriptor) : Font(descriptor)
{
	// first time setup
	if (!tobj) {
		tobj = gluNewTess ();
		gluTessCallback(tobj, GLU_TESS_VERTEX_DATA, reinterpret_cast<_GLUfuncptr>(vertexCallback));
		gluTessCallback(tobj, GLU_TESS_BEGIN_DATA, reinterpret_cast<_GLUfuncptr>(beginCallback));
		gluTessCallback(tobj, GLU_TESS_END, reinterpret_cast<_GLUfuncptr>(endCallback));
		gluTessCallback(tobj, GLU_TESS_ERROR, reinterpret_cast<_GLUfuncptr>(errorCallback));
		gluTessCallback(tobj, GLU_TESS_COMBINE_DATA, reinterpret_cast<_GLUfuncptr>(combineCallback));

		for (Uint16 i=0; i<65535; i++) g_index[i] = i;
	}

	FT_F26Dot6 points = FT_F26Dot6(GetDescriptor().pointSize * 64.0);

	FT_Set_Char_Size(m_face, points, 0, 72, 0);
	for (int chr=32; chr<127; chr++) {
		if (0 != FT_Load_Char(m_face, chr, FT_LOAD_DEFAULT)) {
			printf("Couldn't load glyph\n");
			continue;
		}

		assert(m_face->glyph->format == FT_GLYPH_FORMAT_OUTLINE);
		FT_Outline *outline = &m_face->glyph->outline;

		std::vector<double> temppts;
		std::vector<Uint16> indices;
		std::vector<double> pts;
		int nv = 0;

		TessData tessdata;
		tessdata.pts = &pts;

		gluTessNormal (tobj, 0, 0, 1);
		gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
		gluTessBeginPolygon (tobj, &tessdata);
		for (int contour=0; contour < outline->n_contours; contour++)
		{
			gluTessBeginContour (tobj);
			temppts.clear();
			GenContourPoints(chr, outline, contour, BEZIER_STEPS, &temppts);
			for (size_t i=0; i<temppts.size(); i++) pts.push_back(temppts[i]);
			for (size_t i=0; i<temppts.size(); i+=3, nv++)
				gluTessVertex(tobj,&pts[nv*3],&g_index[nv]);
			gluTessEndContour(tobj);
		}
		tessdata.numvtx = nv;
		gluTessEndPolygon(tobj);

		glfglyph_t glyph;

		nv = tessdata.numvtx;
		glyph.numvtx = nv;
		glyph.varray = static_cast<float *>(malloc (nv*3*sizeof(float)));
		for (int i=0; i<nv*3; i++) glyph.varray[i] = float(pts[i]);

		glyph.numidx = int(tessdata.index.size());
		glyph.iarray = static_cast<Uint16 *>(malloc (glyph.numidx*sizeof(Uint16)));
		for (int i=0; i<glyph.numidx; i++) glyph.iarray[i] = tessdata.index[i];

		glyph.advx = float(m_face->glyph->linearHoriAdvance) / 65536.0f / 36.0f;
		glyph.advy = float(m_face->glyph->linearVertAdvance) / 65536.0f / 36.0f;

		m_glyphs[chr] = glyph;
	}

	m_height = m_glyphs['M'].advy;
	m_width = m_glyphs['M'].advx;
}

VectorFont::~VectorFont()
{
	std::map<int,glfglyph_t>::iterator i;
	for (i=m_glyphs.begin(); i!=m_glyphs.end(); ++i) {
		free ((*i).second.varray);
		free ((*i).second.iarray);
	}
}

}
