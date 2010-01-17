
#include "StarSystem.h"
#include "Serializer.h"
#include "Pi.h"
#include "Frame.h"
#include "Space.h"

#define SAVEFILE_VERSION	11

namespace Serializer {

static std::vector<Frame*> g_frames;
static std::vector<Body*> g_bodies;
static std::vector<SBody*> g_sbodies;

Frame *LookupFrame(size_t index)
{
	if (index == -1) return 0;
	return g_frames[index];
}

int LookupFrame(const Frame *f)
{
	if (f == 0) return -1;
	for (unsigned int i=0; i<g_frames.size(); i++) {
		if (g_frames[i] == f) return i;
	}
	assert(0);
}

static void AddFrame(Frame *f)
{
	g_frames.push_back(f);
	for (std::list<Frame*>::iterator i = f->m_children.begin();
	     i != f->m_children.end(); ++i) {
		AddFrame(*i);
	}
}

void IndexFrames()
{
	g_frames.clear();
	AddFrame(Space::rootFrame);
}

SBody *LookupSystemBody(size_t index) { return (index == ~(size_t)0 ? 0 : g_sbodies[index]); }
int LookupSystemBody(const SBody *b)
{
	if (!b) return -1;
	for (unsigned int i=0; i<g_sbodies.size(); i++) {
		if (g_sbodies[i] == b) return i;
	}
	assert(0);
}
static void add_sbody(SBody *b)
{
	g_sbodies.push_back(b);
	for (unsigned int i=0; i<b->children.size(); i++) {
		add_sbody(b->children[i]);
	}
}
void IndexSystemBodies(StarSystem *s)
{
	g_sbodies.clear();
	add_sbody(s->rootBody);
}


Body *LookupBody(size_t index) { return (index == ~(size_t)0 ? 0 : g_bodies[index]); }
int LookupBody(const Body *b)
{
	if (!b) return -1;
	for (unsigned int i=0; i<g_bodies.size(); i++) {
		if (g_bodies[i] == b) return i;
	}
	assert(0);
}

void IndexBodies()
{
	g_bodies.clear();
	for (Space::bodiesIter_t i = Space::bodies.begin(); i != Space::bodies.end(); ++i) {
		g_bodies.push_back(*i);
	}
}

namespace Write {
	static int checksum;
	static FILE* sfptr;

	bool Game(const char *filename)
	{
		sfptr = fopen(filename, "wb");

		if (sfptr == NULL) {
			return false;
		}

		checksum = 0;

		wr_byte('P');
		wr_byte('I');
		wr_byte('O');
		wr_byte('N');
		wr_byte('E');
		wr_byte('E');
		wr_byte('R');
		wr_byte('\0');

		/* Save file version */
		wr_int (SAVEFILE_VERSION);

		Pi::Serialize();
		
		wr_int(checksum);
		fclose(sfptr);
		fprintf(stderr, "Game saved to '%s'\n", filename);

		return true;
	}

	void wr_byte(unsigned char x)
	{
		putc(x, sfptr);
		checksum += x;
	}

	void wr_bool(bool x)
	{
		wr_byte((unsigned char)x);
	}

	void wr_short(short x)
	{
		wr_byte((unsigned char)(x & 0xff));
		wr_byte((unsigned char)((x>>8) & 0xff));
	}

	void wr_int(int x)
	{
		wr_byte((unsigned char)(x & 0xff));
		wr_byte((unsigned char)((x>>8) & 0xff));
		wr_byte((unsigned char)((x>>16) & 0xff));
		wr_byte((unsigned char)((x>>24) & 0xff));
	}
	
	void wr_int64(Sint64 x)
	{
		wr_byte((unsigned char)(x & 0xff));
		wr_byte((unsigned char)((x>>8) & 0xff));
		wr_byte((unsigned char)((x>>16) & 0xff));
		wr_byte((unsigned char)((x>>24) & 0xff));
		wr_byte((unsigned char)((x>>32) & 0xff));
		wr_byte((unsigned char)((x>>40) & 0xff));
		wr_byte((unsigned char)((x>>48) & 0xff));
		wr_byte((unsigned char)((x>>56) & 0xff));
	}

	/* not portable */
	void wr_float (float f)
	{
		unsigned int i;
		unsigned char *p = (unsigned char*)&f;

		for (i=0; i<sizeof (float); i++, p++) {
			wr_byte (*p);
		}
	}

	void wr_double (double f)
	{
		unsigned int i;
		unsigned char *p = (unsigned char*)&f;

		for (i=0; i<sizeof (double); i++, p++) {
			wr_byte (*p);
		}
	}

	/* First byte is string length, including null terminator */
	void wr_string(const char* s)
	{
		/* We shouldn't fail on null strings */
		if (s == NULL) {
			wr_int(0);
			return;
		}

		wr_int(strlen(s)+1);

		while (*s) {
			wr_byte(*s);
			s++;
		}
		wr_byte(*s);
	}

	void wr_string(const std::string &s)
	{
		wr_string(s.c_str());
	}

	void wr_vector3d(vector3d vec)
	{
		wr_double(vec.x);
		wr_double(vec.y);
		wr_double(vec.z);
	}

	/* These are for templated type turds. it is best to use the explicit
	 * type ones to avoid the pain of strange type picking */
	void wr_auto(Sint32 x) { wr_int(x); }
	void wr_auto(Sint64 x) { wr_int64(x); }
	void wr_auto(float x) { wr_float(x); }
	void wr_auto(double x) { wr_double(x); }
}

namespace Read {
	static int loadsum;
	static FILE* lfptr;
	/* Version of savefile we are loading */
	static int version;

	bool Game(const char *filename)
	{
		loadsum = 0;
		
		lfptr = fopen(filename, "rb");

		if (lfptr == NULL) return false;

		if (rd_byte() != 'P') return false;
		if (rd_byte() != 'I') return false;
		if (rd_byte() != 'O') return false;
		if (rd_byte() != 'N') return false;
		if (rd_byte() != 'E') return false;
		if (rd_byte() != 'E') return false;
		if (rd_byte() != 'R') return false;
		if (rd_byte() != '\0') return false;
		
		/* savefile version */
		version = rd_int ();
		fprintf(stderr, "Savefile version %d. ", version);

		Pi::Unserialize();

		/* check the checksum */
		int i = loadsum;
		if (i != rd_int()) {
			fprintf (stderr, "Checksum error loading savefile.\n");
			return false;
		}

		fclose(lfptr);
		fprintf(stderr, "Loaded '%s'\n", filename);
		
		return true;
	}

	bool IsOlderThan(int ver)
	{
		return version < ver;
	}	

	unsigned char rd_byte(void)
	{
		int x = getc(lfptr) & 0xff;
		loadsum += x;
		return x;
	}

	bool rd_bool()
	{
		return (bool)rd_byte();
	}

	short rd_short(void)
	{
		int t1, t2;
		t2 = rd_byte();
		t1 = rd_byte();
		return ((t1 << 8) | t2);
	}

	int rd_int(void)
	{
		int t1, t2, t3, t4;
		t4 = rd_byte();
		t3 = rd_byte();
		t2 = rd_byte();
		t1 = rd_byte();
		return ((t1 << 24) | (t2 << 16) | (t3 << 8) | t4);
	}

	Sint64 rd_int64(void)
	{
		Sint64 t1, t2, t3, t4, t5, t6, t7, t8;
		t8 = rd_byte();
		t7 = rd_byte();
		t6 = rd_byte();
		t5 = rd_byte();
		t4 = rd_byte();
		t3 = rd_byte();
		t2 = rd_byte();
		t1 = rd_byte();
		return ((t1<<56) | (t2<<48) | (t3<<40) | (t4<<32) | (t5 << 24) | (t6 << 16) | (t7 << 8) | t8);
	}

	float rd_float ()
	{
		unsigned int i;
		float f;
		unsigned char *p = (unsigned char*)&f;

		for (i=0; i<sizeof (float); i++) {
			p[i] = rd_byte ();
		}
		return f;
	}

	double rd_double ()
	{
		unsigned int i;
		double f;
		unsigned char *p = (unsigned char*)&f;

		for (i=0; i<sizeof (double); i++) {
			p[i] = rd_byte ();
		}
		return f;
	}

	std::string rd_string()
	{
		char *s = rd_cstring();
		std::string str(s);
		free(s);
		return str;
	}

	/* *Memory leaks included */
	char* rd_cstring()
	{
		char* buf;
		int i;
		unsigned char size;

		/* Size is in first byte */
		size = rd_int();

		/* A saved null string */
		if (size == 0) {
			return NULL;
		}

		buf = (char*) malloc (sizeof(char)*size);

		for (i=0; i<size; i++) {
			buf[i] = rd_byte();
		}

		return buf;
	}

	void rd_cstring2(char *buf, int len)
	{
		int i;
		int size;

		/* Size is in first byte */
		size = rd_int();

		/* A saved null string */
		if (size == 0) {
			buf[0] = '\0';
			return;
		}

		assert (size < len);

		for (i=0; i<size; i++) {
			buf[i] = rd_byte();
		}
	}

	vector3d rd_vector3d()
	{
		vector3d v;
		v.x = rd_double();
		v.y = rd_double();
		v.z = rd_double();
		return v;
	}
	
	/* These are for templated type turds. it is best to use the explicit
	 * type ones to avoid the pain of strange type picking */
	void rd_auto(Sint32 *x) { *x = rd_int(); }
	void rd_auto(Sint64 *x) { *x = rd_int64(); }
	void rd_auto(float *x) { *x = rd_float(); }
	void rd_auto(double *x) { *x = rd_double(); }
}

} /* end namespace Serializer */
