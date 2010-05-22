#ifndef _SERIALIZE_H
#define _SERIALIZE_H

#include "libs.h"
#include "Quaternion.h"
#include <vector>

class Frame;
class Body;
class StarSystem;
class SBody;

namespace Serializer {

	void IndexFrames();
	Frame *LookupFrame(size_t index);
	int LookupFrame(const Frame *f);

	void IndexBodies();
	Body *LookupBody(size_t index);
	int LookupBody(const Body *);

	void IndexSystemBodies(StarSystem *);
	SBody *LookupSystemBody(size_t index);
	int LookupSystemBody(const SBody*);

	class Writer {
	public:
		Writer() {}
		const std::string &GetData();
		void wr_byte(Uint8 x);
		void wr_bool(bool x);
		void wr_int16(Uint16 x);
		void wr_int32(Uint32 x);
		void wr_int64(Uint64 x);
		void wr_float(float f);
		void wr_double(double f);
		void wr_string(const char* s);
		void wr_string(const std::string &s);
		void wr_vector3d(vector3d vec);
		void wr_quaternionf(const Quaternionf &q);
	private:
		std::string m_str;
	};

	class Reader {
	public:
		Reader();
		Reader(const std::string &data);
		Reader(FILE *fptr);
		bool AtEnd();
		void Seek(int pos);
		Uint8 rd_byte();
		bool rd_bool();
		Uint16 rd_int16();
		Uint32 rd_int32();
		Uint64 rd_int64();
		float rd_float ();
		double rd_double ();
		std::string rd_string();
		char* rd_cstring();
		void rd_cstring2(char *buf, int len);
		vector3d rd_vector3d();
		Quaternionf rd_quaternionf();
	private:
		std::string m_data;
		int m_pos;
	};

	namespace Write {
		bool Game(const char *filename);
		void wr_bool(bool x);
		void wr_byte(unsigned char x);
		void wr_short(short x);
		void wr_int(int x);
		void wr_int64(Sint64 x);
		void wr_float (float f);
		void wr_double (double f);
		void wr_cstring(const char* s);
		void wr_string(const std::string &s);
		void wr_vector3d(vector3d vec);
		void wr_quaternionf(const Quaternionf &q);
		
		void wr_auto(Sint32 x);
		void wr_auto(Sint64 x);
		void wr_auto(float x);
		void wr_auto(double x);
	}

	namespace Read {
		bool Game(const char *filename);
		bool IsOlderThan (int version);
		bool rd_bool();
		unsigned char rd_byte ();
		short rd_short ();
		int rd_int ();
		Sint64 rd_int64 ();
		float rd_float ();
		double rd_double ();
		std::string rd_string();
		char* rd_cstring();
		void rd_cstring2(char *buf, int len);
		vector3d rd_vector3d();
		Quaternionf rd_quaternionf();
	
		void rd_auto(Sint32 *x);
		void rd_auto(Sint64 *x);
		void rd_auto(float *x);
		void rd_auto(double *x);
	}

}

#endif /* _SERIALIZE_H */
