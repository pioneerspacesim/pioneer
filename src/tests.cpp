// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

void test_frames();
void test_stringf();
void test_filesystem();
void test_random();

int main(int argc, char *argv[])
{
	test_frames();
	test_stringf();
	test_filesystem();
	test_random();
	return 0;
}
