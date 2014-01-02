// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "StringF.h"
#include <string>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstring>

static int verbose = 2;

static void check(int line, const char* format, const char* expected, const std::string& result) {
	if (result == std::string(expected)) {
		if (verbose >= 2)
			printf("[line %5d] OK ('%s' -> '%s')\n", line, format, result.c_str());
		else if (verbose >= 1)
			printf("[line %5d]", line);
	} else {
		printf("[line %5d] FAIL:\n  '%s'\n  '%s'\n  '%s'\n", line, format, expected, result.c_str());
	}
}

static char tmpbuf[512];

#define TEST0(format, expected) \
	check(__LINE__, (format), (expected), stringf((format)))
#define TEST1(format, arg0, expected) \
	check(__LINE__, (format), (expected), stringf((format), (arg0)))
#define TEST2(format, arg0, arg1, expected) \
	check(__LINE__, (format), (expected), stringf((format), (arg0), (arg1)))
#define TEST3(format, arg0, arg1, arg2, expected) \
	check(__LINE__, (format), (expected), stringf((format), (arg0), (arg1), (arg2)))
#define TEST4(format, arg0, arg1, arg2, arg3, expected) \
	check(__LINE__, (format), (expected), stringf((format), (arg0), (arg1), (arg2), (arg3)))

#define TESTPF1(format, arg0, pfformat) \
	do { \
		snprintf(tmpbuf, sizeof(tmpbuf), (pfformat), (arg0)); \
		check(__LINE__, (format), tmpbuf, stringf((format), (arg0))); \
	} while (0)

void test_stringf() {
	TEST0("", "");
	TEST0("Hello.", "Hello.");

	TEST0("backslash: \\", "backslash: \\");
	TEST0("2-backslash: \\\\", "2-backslash: \\\\");

	TEST0("%%", "%");
	TEST0("Hello: %%", "Hello: %");
	TEST0("%%: Hello", "%: Hello");
	TEST0("foo %% bar", "foo % bar");

	TEST0("%", "%(err: unfinished reference)");
	TEST0("Hello: %", "Hello: %(err: unfinished reference)");
	TEST0("% xyz", "%(err: unfinished reference) xyz");
	TEST0("%0", "%(err: unknown arg '0')");
	TEST0("%1", "%(err: unknown arg '1')");
	TEST0("%10", "%(err: unknown arg '10')");
	TEST0("%1.", "%(err: unknown arg '1').");
	TEST0("Hello: %1.", "Hello: %(err: unknown arg '1').");
	TEST0("Hello: %xyz", "Hello: %(err: unknown arg 'xyz')");
	TEST0("Hello: %xyz-hello", "Hello: %(err: unknown arg 'xyz')-hello");
	TEST0("Hello: %{xyz-he}llo", "Hello: %(err: unknown arg 'xyz-he')llo");

	TEST0("Hello: %{} end", "Hello: %(err: blank reference) end");
	TEST0("Hello: %_hello end", "Hello: %(err: unknown arg '_hello') end");

	TEST1("Hello: %xyz end", formatarg("x", 42), "Hello: %(err: unknown arg 'xyz') end");
	TEST1("Hello: %x end", formatarg("xyz", 42), "Hello: %(err: unknown arg 'x') end");
	TEST1("Hello: %1 end", formatarg("x", 42), "Hello: %(err: unknown arg '1') end");

	TEST1("Hello: %0 end", formatarg("x", 42), "Hello: 42 end");
	TEST1("Hello: %xyz end", formatarg("xyz", 42), "Hello: 42 end");
	TEST1("Hello: %{xyz} end", formatarg("xyz", 42), "Hello: 42 end");

	TEST1("Hello: %xyz%xyz end", formatarg("xyz", 42), "Hello: 4242 end");
	TEST1("Hello: %xyz %xyz end", formatarg("xyz", 42), "Hello: 42 42 end");

	TEST3("[%x, %y, %z]", formatarg("x", "X"), formatarg("y", "Y"), formatarg("z", "Z"), "[X, Y, Z]");
	TEST3("[%z, %y, %x]", formatarg("x", "X"), formatarg("y", "Y"), formatarg("z", "Z"), "[Z, Y, X]");
	TEST3("[%z, %y, %z]", formatarg("x", "X"), formatarg("y", "Y"), formatarg("z", "Z"), "[Z, Y, Z]");
	TEST3("[%0x, %1y, %2z]", formatarg("x", "X"), formatarg("y", "Y"), formatarg("z", "Z"), "[Xx, Yy, Zz]");

	TEST3("[%0x, %1y, %2z]", formatarg("x", "X"), formatarg("y", "Y"), formatarg("z", "Z"), "[Xx, Yy, Zz]");

	TESTPF1("%0{f}", 42.125, "%f");
	TESTPF1("%0{g}", 42.125, "%g");
	TESTPF1("%0{G}", 42.125, "%G");
	TESTPF1("%0{g}", 42.125e10, "%g");
	TESTPF1("%0{G}", 42.125e10, "%G");
	TESTPF1("%0{e}", 42.125, "%e");
	TESTPF1("%0{E}", 42.125, "%E");

	TESTPF1("%0{f.5}", 42.125, "%.5f");
	TESTPF1("%0{f.0}", 42.125, "%.0f");
	TESTPF1("%0{f8}", 42.125, "%8f");
	TESTPF1("%0{f0}", 42.125, "%0f");
	TESTPF1("%0{f#}", 42.125, "%#f");
	TESTPF1("%0{f+}", 42.125, "%+f");
	TESTPF1("%0{f-8.1}", 42.125, "%-8.1f");
	TESTPF1("%0{f08}", 42.125, "%08f");
	TESTPF1("%0{f+08}", 42.125, "%+08f");
	TESTPF1("%0{f+08.2}", 42.125, "%+08.2f");
	TESTPF1("%0{f08.2}", 42.125, "%08.2f");

	TESTPF1("%0{f.5}", 42.0, "%.5f");
	TESTPF1("%0{f.0}", 42.0, "%.0f");
	TESTPF1("%0{f8}", 42.0, "%8f");
	TESTPF1("%0{f0}", 42.0, "%0f");
	TESTPF1("%0{f#}", 42.0, "%#f");
	TESTPF1("%0{f+}", 42.0, "%+f");
	TESTPF1("%0{f-8.1}", 42.0, "%-8.1f");
	TESTPF1("%0{f08}", 42.0, "%08f");
	TESTPF1("%0{f+08}", 42.0, "%+08f");
	TESTPF1("%0{f+08.2}", 42.0, "%+08.2f");
	TESTPF1("%0{f08.2}", 42.0, "%08.2f");
	TEST1("%0{f }", 42.0, "%(err: bad format)");

	TESTPF1("%0{i8}", 42, "%8i");
	TESTPF1("%0{i0}", 42, "%0i");
	TESTPF1("%0{i+}", 42, "%+i");
	TESTPF1("%0{i-8}", 42, "%-8i");
	TESTPF1("%0{i08}", 42, "%08i");
	TESTPF1("%0{i+08}", 42, "%+08i");
	TESTPF1("%0{i+8}", 42, "%+8i");

	TESTPF1("%0{x02}", 93u, "%02x");

	TEST1("%0{i.0}", 42, "%(err: bad format)");
	TEST1("%0{i }", 42, "%(err: bad format)");
}
