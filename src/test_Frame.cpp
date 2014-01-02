// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Pi.h"
#include "Frame.h"
#include "Space.h"

#if 0
#define CRAP	0.00001
static void test(vector3d a, vector3d b)
{
	vector3d c = a-b;
	c.x = fabs(c.x);
	c.y = fabs(c.y);
	c.z = fabs(c.z);
	if ((c.x < CRAP) && (c.y < CRAP) && (c.z < CRAP)) {
		printf("PASSED\n");
	} else {
		printf("\nFAILED-------------------\n");
	}
}

static void dump_frames(Frame *f, int depth)
{
	for(int i=0; i<depth; i++) putchar('\t');
	printf("%s: Vel %f\n", f->GetLabel(), f->GetVelocity().Length());
	std::list<Frame*> m_children;
	for (std::list<Frame*>::iterator i = f->m_children.begin(); i!=f->m_children.end(); ++i) {
		dump_frames(*i, depth+1);
	}
}

void test_frames()
{
	printf("Testing FOR\n");
	matrix4x4d m;
	vector3d a,b;
	Frame *root = new Frame(0, "Root");
	Space::rootFrame = root;

	Frame *fa1 = new Frame(root, "Fa1");
	fa1->SetVelocity(vector3d(0,0,10));

	Frame *fa2 = new Frame(fa1, "Fa2");
	fa2->SetVelocity(vector3d(0,0,1));

	Frame *fb1 = new Frame(root, "Fb1");
	fb1->SetVelocity(vector3d(0,0,10));

	Frame *fb2 = new Frame(fb1, "Fb2");
	fb2->SetVelocity(vector3d(0,0,5));

	a = Frame::GetFrameRelativeVelocity(fb1, root);
	b = Frame::GetFrameRelativeVelocity(root, fb1);
	test(a, vector3d(0,0,-10));
	test(b, vector3d(0,0,10));
	a = Frame::GetFrameRelativeVelocity(fb2, root);
	b = Frame::GetFrameRelativeVelocity(root, fb2);
	test(a, vector3d(0,0,-15));
	test(b, vector3d(0,0,15));

	a = Frame::GetFrameRelativeVelocity(fb2, fa2);
	b = Frame::GetFrameRelativeVelocity(fa2, fb2);
	test(a, vector3d(0, 0, -4));
	test(b, vector3d(0, 0, 4));


	// ----------- make some frames rotated --------
	m = matrix4x4d::RotateYMatrix(M_PI/2);
	fb2->SetOrientation(m);
	m = matrix4x4d::RotateYMatrix(M_PI);
	fb1->SetOrientation(m);

	a = Frame::GetFrameRelativeVelocity(fb2, root);
	b = Frame::GetFrameRelativeVelocity(root, fb2);
	test(a, vector3d(-10, 0, -5));
	test(b, vector3d(5, 0, -10));

	a = Frame::GetFrameRelativeVelocity(fb2, fa2);
	b = Frame::GetFrameRelativeVelocity(fa2, fb2);
	test(a, vector3d(-21, 0, -5));
	test(b, vector3d(5, 0, -21));
}
#endif

void test_frames() {
}
