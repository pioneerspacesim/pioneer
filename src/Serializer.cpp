#include "StarSystem.h"
#include "Serializer.h"
#include "Pi.h"
#include "Frame.h"
#include "Space.h"
#include "Ship.h"
#include "HyperspaceCloud.h"

namespace Serializer {

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
}
Reader::Reader(const std::string &data):
	m_data(data),
	m_pos(0) {
	
}
Reader::Reader(FILE *fptr): m_pos(0) {
	m_data = "";
	while (!feof(fptr)) m_data.push_back(fgetc(fptr));
	printf(SIZET_FMT " characters in savefile\n", m_data.size());
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

} /* end namespace Serializer */
