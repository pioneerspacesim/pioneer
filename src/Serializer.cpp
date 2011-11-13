#include "StarSystem.h"
#include "Serializer.h"
#include "Pi.h"
#include "Frame.h"
#include "Space.h"
#include "Ship.h"
#include "HyperspaceCloud.h"

#define SAVEFILE_VERSION	42

namespace Serializer {

static std::vector<Frame*> g_frames;
static std::vector<Body*> g_bodies;
static std::vector<SBody*> g_sbodies;
static int stream_version_context = SAVEFILE_VERSION;

Frame *LookupFrame(uint32_t index) { return g_frames[index]; }
uint32_t LookupFrame(const Frame *f)
{
	for (unsigned int i=0; i<g_frames.size(); i++) {
		if (g_frames[i] == f) return i;
	}
	assert(0); return 0;
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
	g_frames.push_back(0);			// map zero to null
	AddFrame(Space::rootFrame);
}

SBody *LookupSystemBody(uint32_t index) { return g_sbodies[index]; }
uint32_t LookupSystemBody(const SBody *b)
{
	for (unsigned int i=0; i<g_sbodies.size(); i++) {
		if (g_sbodies[i] == b) return i;
	}
	assert(0); return 0;
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
	g_sbodies.push_back(0);			// map zero to null
	add_sbody(s->rootBody);
}


Body *LookupBody(uint32_t index) { return g_bodies[index]; }
uint32_t LookupBody(const Body *b)
{
	for (unsigned int i=0; i<g_bodies.size(); i++) {
		if (g_bodies[i] == b) return i;
	}
	assert(0); return 0;
}

void IndexBodies()
{
	g_bodies.clear();
	g_bodies.push_back(0);		// map zero to null
	for (Space::bodiesIter_t i = Space::bodies.begin(); i != Space::bodies.end(); ++i) {
		g_bodies.push_back(*i);
		// XXX hyperclouds can also own a reference to a body. we need to index that too
		// since lua modules can hold references to it
		if ((*i)->IsType(Object::HYPERSPACECLOUD)) {
			Ship *s = static_cast<HyperspaceCloud*>(*i)->GetShip();
			if (s) g_bodies.push_back(s);
		}
	}
}


const std::string &Writer::GetData() { return m_str; }
void Writer::Byte(Uint8 x) {
	m_str.push_back(char(x));
}
void Writer::Bool(bool x) {
	Byte(Uint8(x));
}
void Writer::Int16(Uint16 x) {
	m_str.push_back(char(x&0xff));
	m_str.push_back(char((x>>8)&0xff));
}
void Writer::Int32(Uint32 x) {
	m_str.push_back(char(x&0xff));
	m_str.push_back(char((x>>8)&0xff));
	m_str.push_back(char((x>>16)&0xff));
	m_str.push_back(char((x>>24)&0xff));
}
void Writer::Int64(Uint64 x) {
	m_str.push_back(char(x&0xff));
	m_str.push_back(char((x>>8)&0xff));
	m_str.push_back(char((x>>16)&0xff));
	m_str.push_back(char((x>>24)&0xff));
	m_str.push_back(char((x>>32)&0xff));
	m_str.push_back(char((x>>40)&0xff));
	m_str.push_back(char((x>>48)&0xff));
	m_str.push_back(char((x>>56)&0xff));
}
void Writer::Float(float f) {
	// not portable across architectures?
	unsigned int i;
	union {
		unsigned char c[sizeof (float)];
		float f;
	} p;
	p.f = f;

	for (i=0; i<sizeof (float); i++) {
		Byte (p.c[i]);
	}
}
void Writer::Double(double f) {
	// not portable across architectures
	unsigned int i;
	union {
		unsigned char c[sizeof (double)];
		double f;
	} p;
	p.f = f;

	for (i=0; i<sizeof (double); i++) {
		Byte (p.c[i]);
	}
}
/* First byte is string length, including null terminator */
void Writer::String(const char* s)
{
	/* We shouldn't fail on null strings */
	if (s == NULL) {
		Int32(0);
		return;
	}

	Int32(strlen(s)+1);

	while (*s) {
		Byte(*s);
		s++;
	}
	Byte(*s);
}

void Writer::String(const std::string &s)
{
	Int32(s.size()+1);

	for(size_t i=0; i<s.size(); i++) {
		Byte(s[i]);
	}
	Byte(0);
}

void Writer::Vector3d(vector3d vec)
{
	Double(vec.x);
	Double(vec.y);
	Double(vec.z);
}

void Writer::WrQuaternionf(const Quaternionf &q)
{
	Float(q.w);
	Float(q.x);
	Float(q.y);
	Float(q.z);
}


Reader::Reader(): m_data(""), m_pos(0) {
	m_streamVersion = stream_version_context;
}
Reader::Reader(const std::string &data):
	m_data(data),
	m_pos(0) {
	m_streamVersion = stream_version_context;
	
}
Reader::Reader(FILE *fptr): m_pos(0) {
	m_streamVersion = stream_version_context;
	m_data = "";
	while (!feof(fptr)) m_data.push_back(fgetc(fptr));
	printf("%lu characters in savefile\n", m_data.size());
}
bool Reader::AtEnd() { return m_pos >= m_data.size(); }
void Reader::Seek(int pos) { m_pos = pos; }
Uint8 Reader::Byte() {
#ifdef DEBUG
	assert(m_pos < m_data.size());
#endif /* DEBUG */
	return Uint8(m_data[m_pos++]);
}
bool Reader::Bool() {
	return Byte() != 0;
}
Uint16 Reader::Int16()
{
	int t1, t2;
	t2 = Byte();
	t1 = Byte();
	return ((t1 << 8) | t2);
}
Uint32 Reader::Int32(void)
{
	int t1, t2, t3, t4;
	t4 = Byte();
	t3 = Byte();
	t2 = Byte();
	t1 = Byte();
	return ((t1 << 24) | (t2 << 16) | (t3 << 8) | t4);
}
Uint64 Reader::Int64(void)
{
	Uint64 t1, t2, t3, t4, t5, t6, t7, t8;
	t8 = Byte();
	t7 = Byte();
	t6 = Byte();
	t5 = Byte();
	t4 = Byte();
	t3 = Byte();
	t2 = Byte();
	t1 = Byte();
	return ((t1<<56) | (t2<<48) | (t3<<40) | (t4<<32) | (t5 << 24) | (t6 << 16) | (t7 << 8) | t8);
}

float Reader::Float ()
{
	unsigned int i;
	union {
		unsigned char c[sizeof (float)];
		float f;
	} p;

	for (i=0; i<sizeof (float); i++) {
		p.c[i] = Byte ();
	}
	return p.f;
}

double Reader::Double ()
{
	unsigned int i;
	union {
		unsigned char c[sizeof (double)];
		double f;
	} p;

	for (i=0; i<sizeof (double); i++) {
		p.c[i] = Byte ();
	}
	return p.f;
}

std::string Reader::String()
{
	int size = Int32();
	if (size == 0) return "";

	std::string buf;
	buf.reserve(size-1);

	for (int i=0; i<size-1; i++) {
		buf.push_back(char(Byte()));
	}
	Byte();// discard null terminator
	return buf;
}

/* *Memory leaks included */
char* Reader::Cstring()
{
	char* buf;
	int i, size;

	/* Size is in first byte */
	size = Int32();

	/* A saved null string */
	if (size == 0) {
		return NULL;
	}

	buf = static_cast<char*>(malloc (sizeof(char)*size));

	for (i=0; i<size; i++) {
		buf[i] = Byte();
	}

	return buf;
}

void Reader::Cstring2(char *buf, int len)
{
	int i;
	int size;

	/* Size is in first byte */
	size = Int32();

	/* A saved null string */
	if (size == 0) {
		buf[0] = '\0';
		return;
	}

	assert (size < len);

	for (i=0; i<size; i++) {
		buf[i] = Byte();
	}
}

vector3d Reader::Vector3d()
{
	vector3d v;
	v.x = Double();
	v.y = Double();
	v.z = Double();
	return v;
}

Quaternionf Reader::RdQuaternionf()
{
	Quaternionf q;
	q.w = Float();
	q.x = Float();
	q.y = Float();
	q.z = Float();
	return q;
}

bool SaveGame(const char *filename)
{
	FILE *sfptr = fopen(filename, "wb");

	if (sfptr == NULL) {
		return false;
	}

	Writer wr;

	wr.Byte('P');
	wr.Byte('I');
	wr.Byte('O');
	wr.Byte('N');
	wr.Byte('E');
	wr.Byte('E');
	wr.Byte('R');
	wr.Byte('\0');

	/* Save file version */
	wr.Int32(SAVEFILE_VERSION);

	Pi::Serialize(wr);
	
	wr.Byte('E');
	wr.Byte('N');
	wr.Byte('D');
	wr.Byte(0);

	// actually write the shit
	const std::string &data = wr.GetData();
	for (size_t i=0; i<data.size(); i++) {
		putc(data[i], sfptr);
	}

	wr = Writer();

	fclose(sfptr);
	fprintf(stderr, "Game saved to '%s'\n", filename);

	return true;
}

void LoadGame(const char *filename)
{
	FILE *lfptr = fopen(filename, "rb");

	if (lfptr == NULL) throw CouldNotOpenFileException();

	Reader rd = Reader(lfptr);
	fclose(lfptr);

	if (rd.Byte() != 'P') throw SavedGameCorruptException();
	if (rd.Byte() != 'I') throw SavedGameCorruptException();
	if (rd.Byte() != 'O') throw SavedGameCorruptException();
	if (rd.Byte() != 'N') throw SavedGameCorruptException();
	if (rd.Byte() != 'E') throw SavedGameCorruptException();
	if (rd.Byte() != 'E') throw SavedGameCorruptException();
	if (rd.Byte() != 'R') throw SavedGameCorruptException();
	if (rd.Byte() != '\0') throw SavedGameCorruptException();
	
	/* savefile version */
	rd.SetStreamVersion(rd.Int32());
	fprintf(stderr, "Savefile version %d. ", rd.StreamVersion());

	if (rd.StreamVersion() != SAVEFILE_VERSION) {
		fprintf(stderr, "Can't load savefile, expected version %d\n", SAVEFILE_VERSION);
		throw SavedGameCorruptException();
	}

	stream_version_context = rd.StreamVersion();
	Pi::Unserialize(rd);
	stream_version_context = SAVEFILE_VERSION;

	if (rd.Byte() != 'E') throw SavedGameCorruptException();
	if (rd.Byte() != 'N') throw SavedGameCorruptException();
	if (rd.Byte() != 'D') throw SavedGameCorruptException();
	if (rd.Byte() != 0) throw SavedGameCorruptException();

	fprintf(stderr, "Loaded '%s'\n", filename);
}

} /* end namespace Serializer */
