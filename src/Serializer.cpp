
#include "Serializer.h"
#include "Pi.h"
#include "Frame.h"
#include "Space.h"

namespace Serializer {

static std::vector<Frame*> g_frames;

Frame *LookupFrame(int index)
{
	return g_frames[index];
}

int LookupFrame(Frame *f)
{
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
	printf("%d frames indexed\n", g_frames.size());
}

namespace Write {
	static int checksum;
	static FILE* sfptr;

	bool Game(const char *filename)
	{
		struct Object *o;
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

	void wr_string(std::string &s)
	{
		wr_string(s.c_str());
	}

	void wr_vector3d(vector3d &vec)
	{
		wr_double(vec.x);
		wr_double(vec.y);
		wr_double(vec.z);
	}
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

	bool is_olderthan(int ver)
	{
		return ver < version;
	}	

	unsigned char rd_byte(void)
	{
		int x = getc(lfptr) & 0xff;
		loadsum += x;
		return x;
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
}

} /* end namespace Serializer */
