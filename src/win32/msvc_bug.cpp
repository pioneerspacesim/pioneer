#include <stdio.h>
#include <vector>

class V {
public:
	double x,y,z;

	V() {};
	V(double _x, double _y, double _z): x(_x), y(_y), z(_z) {}
	V(const V &v): x(v.x), y(v.y), z(v.z) {}

	V operator+(const V a) const { return V(a.x+x, a.y+y, a.z+z); }
	friend V operator*(const V a, const double s) { return V(a.x*s, a.y*s, a.z*s); }
};

struct M {
	virtual void z(int *m) const {}
};

struct S: public M {
	void bug();
};

struct C {
	C(const S *s);
	std::vector<int> v;
};

volatile void *g_q;

void S::bug() {
	g_q = this;
	new C(this);
	if (this != g_q) {
		printf(":-(\n");
	} else {
		printf("zzz\n");
	}
}

C::C(const S *s) {
	int m;
	s->z(&m);

	V mx;
	mx*5 + mx*6;
}
/*
int main() {
	S *s = new S();
	s->bug();

	return 0;
}
*/
