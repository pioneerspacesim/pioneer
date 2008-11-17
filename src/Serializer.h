#ifndef _SERIALIZE_H
#define _SERIALIZE_H

#include "libs.h"
#include "StarSystem.h"
#include <vector>
#define SAVEFILE_VERSION	1

class Frame;
class Body;
class StarSystem;

namespace Serializer {

	void IndexFrames();
	Frame *LookupFrame(size_t index);
	int LookupFrame(Frame *f);

	void IndexBodies();
	Body *LookupBody(size_t index);
	int LookupBody(Body *);

	void IndexSystemBodies(StarSystem *);
	StarSystem::SBody *LookupSystemBody(size_t index);
	int LookupSystemBody(StarSystem::SBody*);

	namespace Write {
		bool Game(const char *filename);
		void wr_bool(bool x);
		void wr_byte(unsigned char x);
		void wr_short(short x);
		void wr_int(int x);
		void wr_float (float f);
		void wr_double (double f);
		void wr_cstring(const char* s);
		void wr_string(std::string &s);
		void wr_vector3d(vector3d vec);
	}

	namespace Read {
		bool Game(const char *filename);
		bool is_olderthan (int version);
		bool rd_bool();
		unsigned char rd_byte ();
		short rd_short ();
		int rd_int ();
		float rd_float ();
		double rd_double ();
		std::string rd_string();
		char* rd_cstring();
		void rd_cstring2(char *buf, int len);
		vector3d rd_vector3d();
	}

}

#endif /* _SERIALIZE_H */
