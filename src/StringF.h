#ifndef _STRINGF_H
#define _STRINGF_H

#include "libs.h"
#include <string>
#include <SDL_stdinc.h>

//   provides (for integer types, floating point types, const char* and std::string):
//
// Basic value -> string functions:
//   std::string to_string(T value);
//   std::string to_string(T value, const FormatSpec& format);
//
// You may extend the system by providing your own overloads for
// std::string to_string(T value, const FormatSpec& format)
//
// String formatter:
//   std::string stringf(const char* fmt, ...);
//
// This should work for up to 7 arguments, where each argument is:
// - An object of type FormatArg or FormatArgT<T> for some T
// - Or, the result of a call to formatarg(name, value)
// - Or, a value of a type that can be converted to a string with to_string
//
// formatarg() allows you to give a name and optionally a default format
// for an argument to stringf()
// e.g., formatarg("distance", 42.5, "f.2")
//
// That argument can then be referenced in the format template as %distance,
// and will be formatted as a fixed-point number with 2 decimal places.
//
// stringf(), along with FormatArg and formatarg() is a wrapper around
// string_format(const char* fmt, int numargs, const FormatArg * const args[])
//
// Syntax for argument references:
//   ref = '%' ( ident | int | '{' [^}]+ '}' ) ( '{' formatspec '}' )?
//   int = [0-9]+
//   ident = [a-zA-Z_] [a-zA-Z0-9_]*
//   alpha = [a-zA-Z]
//   formatspec = alpha+ ( ':'? fmtparam ( '|' fmtparam)* )
//   fmtparam = ( [^}\] | '\' any )*
//
// To insert a literal % character, use %% (as in printf)
//
// References are either an integer argument index (0-based),
// or a text string, which can follow C identifier rules, or be any string
// enclosed in braces.
//
// The format specifier, if provided, consists of a style name, followed by
// a series of parameters. Style names are alphabetic only (no underscore,
// digits or puncutation). Parameters may immediately follow the style name,
// or be separated from the style name by a colon.
// Parameters are separated from each other by a pipe character.
// Backslash may be used within format parameters to escape '|' and '}',
// more generally, backslash within a format parameter causes the next
// character to be taken as a literal, regardless of what that character is
//
// Examples of references:
//  stringf("Hello, %0.", "Jameson") -> "Hello, Jameson."
//  stringf("Hello, %0.", formatarg("name", "Jameson")) -> "Hello, Jameson."
//  stringf("Hello, %name.", formatarg("name", "Jameson")) -> "Hello, Jameson."
//  stringf("That's %{mood}tastic!", formatarg("mood", "funky")) -> "That's funkytastic!"
//
//  stringf("I've already wasted %count %{trip(s)} on this fooling endeavour!",
//      formatarg("count", 3), formatarg("trip(s)", "trips"))
//      -> "I've already wasted 3 trips on this fooling endeavour!"
//
//  stringf("I've already wasted %count %{trip(s)} on this fooling endeavour!",
//      formatarg("count", 1), formatarg("trip(s)", "trip"))
//      -> "I've already wasted 1 trip on this fooling endeavour!"
//
//  stringf("That'll be %0 credits, Mr. %1.", 50, "Jameson")
//      -> "That'll be 50 credits, Mr. Jameson."
//  stringf("Excellent choice, Mr. %1! That'll be %0 credits, please.", 50, "Jameson")
//      -> "Excellent choice, Mr. Jameson! That'll be 50 credits, please."
//
// Currently implemented format styles are designed to mostly match printf()
// specifiers, except to follow the general syntax described above, the
// specifier itself comes first, then any flags as a parameter. Only numeric
// types currently interpret these format specifiers. So:
//
//   printf("%s", "Hello") =~= stringf("%0", "Hello")
//   printf("%f", 42.125) =~= stringf("%0{f}", 42.125)
//   printf("%.2f", 42.125) =~= stringf("%0{f.2}", 42.125)
//   printf("%+2.3f", 42.125) =~= stringf("%0{f+2.3}", 42.125)
//   printf("%08d", 42) =~= stringf("%0{d08}", 42)
//

class FormatSpec {
public:
	FormatSpec();
	FormatSpec(const char* format);
	FormatSpec(const char* format, int formatlen);

	bool empty() const;

	// access to components of the formatspec
	bool specifierIs(const char* specifier) const;
	int paramCount() const;
	std::string param(int idx) const;
	void paramPtr(int idx, const char*& begin, const char*& end) const;

private:
	static const int MAX_PARAMS = 3;

	void parseFormat(int length);

	const char * const format;
	// each entry in the params array specifies the index within format[]
	// of the first byte in the parameter
	uint16_t params[MAX_PARAMS+1];
};

std::string to_string(int32_t value, const FormatSpec& fmt);
std::string to_string(int64_t value, const FormatSpec& fmt);
std::string to_string(uint32_t value, const FormatSpec& fmt);
std::string to_string(uint64_t value, const FormatSpec& fmt);
std::string to_string(float value, const FormatSpec& fmt);
std::string to_string(double value, const FormatSpec& fmt);
std::string to_string(fixed value, const FormatSpec& fmt);
std::string to_string(const char* value, const FormatSpec& fmt);
std::string to_string(const std::string& value, const FormatSpec& fmt);

inline std::string to_string(int32_t value, const FormatSpec& fmt) {
	return to_string(int64_t(value), fmt);
}

inline std::string to_string(uint32_t value, const FormatSpec& fmt) {
	return to_string(uint64_t(value), fmt);
}

inline std::string to_string(float value, const FormatSpec& fmt) {
	return to_string(double(value), fmt);
}

inline std::string to_string(fixed value, const FormatSpec& fmt) {
	return to_string(value.ToDouble(), fmt);
}

template <typename T>
inline std::string to_string(const T& value) {
	return to_string(value, FormatSpec());
}

class FormatArg {
public:
	explicit FormatArg(const char* name_ = 0, const char* defaultformat_ = 0):
		name(name_), defaultformat(defaultformat_) {}

	char const * const name;
	char const * const defaultformat;

	virtual std::string format(const FormatSpec& spec) const = 0;
};

template <typename T>
class FormatArgT : public FormatArg {
public:
	FormatArgT(const char* name_, const T& value_, const char* defaultformat_):
		FormatArg(name_, defaultformat_), value(value_) {}

	virtual std::string format(const FormatSpec& spec) const {
		return to_string(value, spec);
	}

private:
	const T value;
};

// ---------------------------------------------------------------------------

template <typename T> struct FormatArgWrapper;

template <typename T> struct FormatArgWrapper {
	typedef FormatArgT<T> type;
	static type wrap(const T& arg, const char* name = 0, const char* defaultformat = 0)
	{ return FormatArgT<T>(name, arg, defaultformat); }
};
template <int N> struct FormatArgWrapper<char[N]> {
	typedef FormatArgT<const char*> type;
	static type wrap(const char (&arg)[N], const char* name = 0, const char* defaultformat = 0)
	{ return FormatArgT<const char*>(name, arg, defaultformat); }
};
template <> struct FormatArgWrapper<char[]> {
	typedef FormatArgT<const char*> type;
	static type wrap(const char *arg, const char* name = 0, const char* defaultformat = 0)
	{ return FormatArgT<const char*>(name, arg, defaultformat); }
};
template <> struct FormatArgWrapper<FormatArg> {
	typedef FormatArg type;
	static const type& wrap(const FormatArg& arg) { return arg; }
};
template <typename T> struct FormatArgWrapper< FormatArgT<T> > {
	typedef FormatArgT<T> type;
	static const type& wrap(const FormatArgT<T>& arg) { return arg; }
};

// ---------------------------------------------------------------------------

// this version is safer (doesn't rely on the value out-living the FormatArgT object)
// but performs a string copy
/*
FormatArgT<std::string> formatarg(const char* name, const char* value) {
	return FormatArgT<std::string>(name, std::string(value));
}
*/

template <typename T>
inline typename FormatArgWrapper<T>::type
formatarg(const char* name, const T& value, const char* defaultformat = 0) {
	return FormatArgWrapper<T>::wrap(value, name, defaultformat);
}

// underlying formatting function

std::string string_format(const char* fmt, int numargs, FormatArg const * const args[]);

// ---------------------------------------------------------------------------

// ---- stringf(format, args...) for 0 to 7 arguments ----

inline std::string stringf(const char* fmt) {
	return string_format(fmt, 0, 0);
}

template <typename T0>
inline std::string stringf(const char* fmt, const T0& p0) {
	const typename FormatArgWrapper<T0>::type& arg0 = FormatArgWrapper<T0>::wrap(p0);
	FormatArg const * const args[] = { &arg0 };
	return string_format(fmt, COUNTOF(args), args);
}

template <typename T0, typename T1>
inline std::string stringf(const char* fmt, const T0& p0, const T1& p1) {
	const typename FormatArgWrapper<T0>::type& arg0 = FormatArgWrapper<T0>::wrap(p0);
	const typename FormatArgWrapper<T1>::type& arg1 = FormatArgWrapper<T1>::wrap(p1);
	FormatArg const * const args[] = { &arg0, &arg1 };
	return string_format(fmt, COUNTOF(args), args);
}

template <typename T0, typename T1, typename T2>
inline std::string stringf(const char* fmt, const T0& p0, const T1& p1, const T2& p2) {
	const typename FormatArgWrapper<T0>::type& arg0 = FormatArgWrapper<T0>::wrap(p0);
	const typename FormatArgWrapper<T1>::type& arg1 = FormatArgWrapper<T1>::wrap(p1);
	const typename FormatArgWrapper<T2>::type& arg2 = FormatArgWrapper<T2>::wrap(p2);
	FormatArg const * const args[] = { &arg0, &arg1, &arg2 };
	return string_format(fmt, COUNTOF(args), args);
}

template <typename T0, typename T1, typename T2, typename T3>
inline std::string stringf(const char* fmt, const T0& p0, const T1& p1, const T2& p2, const T3& p3) {
	const typename FormatArgWrapper<T0>::type& arg0 = FormatArgWrapper<T0>::wrap(p0);
	const typename FormatArgWrapper<T1>::type& arg1 = FormatArgWrapper<T1>::wrap(p1);
	const typename FormatArgWrapper<T2>::type& arg2 = FormatArgWrapper<T2>::wrap(p2);
	const typename FormatArgWrapper<T3>::type& arg3 = FormatArgWrapper<T3>::wrap(p3);
	FormatArg const * const args[] = { &arg0, &arg1, &arg2, &arg3 };
	return string_format(fmt, COUNTOF(args), args);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline std::string stringf(const char* fmt, const T0& p0, const T1& p1, const T2& p2, const T3& p3, const T4& p4) {
	const typename FormatArgWrapper<T0>::type& arg0 = FormatArgWrapper<T0>::wrap(p0);
	const typename FormatArgWrapper<T1>::type& arg1 = FormatArgWrapper<T1>::wrap(p1);
	const typename FormatArgWrapper<T2>::type& arg2 = FormatArgWrapper<T2>::wrap(p2);
	const typename FormatArgWrapper<T3>::type& arg3 = FormatArgWrapper<T3>::wrap(p3);
	const typename FormatArgWrapper<T4>::type& arg4 = FormatArgWrapper<T4>::wrap(p4);
	FormatArg const * const args[] = { &arg0, &arg1, &arg2, &arg3, &arg4 };
	return string_format(fmt, COUNTOF(args), args);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline std::string stringf(const char* fmt, const T0& p0, const T1& p1, const T2& p2, const T3& p3, const T4& p4, const T5& p5) {
	const typename FormatArgWrapper<T0>::type& arg0 = FormatArgWrapper<T0>::wrap(p0);
	const typename FormatArgWrapper<T1>::type& arg1 = FormatArgWrapper<T1>::wrap(p1);
	const typename FormatArgWrapper<T2>::type& arg2 = FormatArgWrapper<T2>::wrap(p2);
	const typename FormatArgWrapper<T3>::type& arg3 = FormatArgWrapper<T3>::wrap(p3);
	const typename FormatArgWrapper<T4>::type& arg4 = FormatArgWrapper<T4>::wrap(p4);
	const typename FormatArgWrapper<T5>::type& arg5 = FormatArgWrapper<T5>::wrap(p5);
	FormatArg const * const args[] = { &arg0, &arg1, &arg2, &arg3, &arg4, &arg5 };
	return string_format(fmt, COUNTOF(args), args);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline std::string stringf(const char* fmt, const T0& p0, const T1& p1, const T2& p2, const T3& p3, const T4& p4, const T5& p5, const T6& p6) {
	const typename FormatArgWrapper<T0>::type& arg0 = FormatArgWrapper<T0>::wrap(p0);
	const typename FormatArgWrapper<T1>::type& arg1 = FormatArgWrapper<T1>::wrap(p1);
	const typename FormatArgWrapper<T2>::type& arg2 = FormatArgWrapper<T2>::wrap(p2);
	const typename FormatArgWrapper<T3>::type& arg3 = FormatArgWrapper<T3>::wrap(p3);
	const typename FormatArgWrapper<T4>::type& arg4 = FormatArgWrapper<T4>::wrap(p4);
	const typename FormatArgWrapper<T5>::type& arg5 = FormatArgWrapper<T5>::wrap(p5);
	const typename FormatArgWrapper<T6>::type& arg6 = FormatArgWrapper<T6>::wrap(p6);
	FormatArg const * const args[] = { &arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6 };
	return string_format(fmt, COUNTOF(args), args);
}

#endif
