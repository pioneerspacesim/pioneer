#ifndef _STRINGF_H
#define _STRINGF_H

#include <string>
#include <stdint.h>

//   provides (for integer types, floating point types, const char* and std::string):
//
// Basic value -> string functions:
//   std::string to_string(T value);
//   std::string to_string(T value, const FormatSpec& format);
//   void to_string(std::string& buf, T value);
//   void to_string(std::string& buf, T value, const FormatSpec& format);
//
// You may extend the system by providing your own overloads for
// to_string(std::string& buf, T value, const FormatSpec& format)
//
// String formatter:
//   std::string stringf(const char* fmt, ...);
//
// This should work for up to 7 arguments, where each argument is:
// - An object of type FormatArg or FormatArgT<T> for some T
// - Or, the result of a call to formatarg(name, value)
// - Or, a value of a type that can be converted to a string with to_string
//
// stringf(), along with FormatArg and formatarg() is a wrapper around
// format_string(const char* fmt, int numargs, const FormatArg * const args[])
//
// which can be used if for some reason you need to pass more than 7 arguments
// (but if you need that then you're probably doing something wrong somewhere)

struct FormatSpec {
	FormatSpec(): minwidth(0), precision(0), flags(0), form(Gen) {}
	static const FormatSpec DEFAULT_FORMAT_SPEC;

	enum Form {
		Gen, // general

		Dec, // decimal
		Oct, // octal
		HexLower, // lower-case hexadecimal
		HexUpper, // upper-case hexadecimal

		Sci, // scientific notation (e.g., 1.8e-2)
		Fix  // decimal with floating point
	};

	enum Flags {
		ShowType     = 1 << 0, // prefix octal with '0', HexLower with '0x', HexUpper with '0X',
		                       // and always include a decimal point for floating point numbers
		PadLeftZero  = 1 << 1, // pad with zeros instead of spaces
		AlignLeft    = 1 << 2, // align output to the left of its width block
		ForceSign    = 1 << 3, // prefix positive values with a +
		PadSign      = 1 << 4  // prefix positive values with a space
	};

	int minwidth;
	int precision;
	int flags;
	Form form;
};

void to_string(std::string& buf, int32_t value, const FormatSpec& fmt);
void to_string(std::string& buf, int64_t value, const FormatSpec& fmt);
void to_string(std::string& buf, uint32_t value, const FormatSpec& fmt);
void to_string(std::string& buf, uint64_t value, const FormatSpec& fmt);
void to_string(std::string& buf, float value, const FormatSpec& fmt);
void to_string(std::string& buf, double value, const FormatSpec& fmt);
//void to_string(std::string& buf, fixed value, const FormatSpec& fmt);
void to_string(std::string& buf, const char* value, const FormatSpec& fmt);
void to_string(std::string& buf, const std::string& value, const FormatSpec& fmt);

inline void to_string(std::string& buf, int32_t value, const FormatSpec& fmt) {
	to_string(buf, int64_t(value), fmt);
}

inline void to_string(std::string& buf, uint32_t value, const FormatSpec& fmt) {
	to_string(buf, uint64_t(value), fmt);
}

inline void to_string(std::string& buf, float value, const FormatSpec& fmt) {
	to_string(buf, double(value), fmt);
}

/*
inline void to_string(std::string& buf, fixed value, const FormatSpec& fmt) {
	to_string(buf, value.ToDouble(), fmt);
}
*/

template <typename T>
inline void to_string(std::string& buf, const T& value) {
	to_string(buf, value, FormatSpec::DEFAULT_FORMAT_SPEC);
}

template <typename T>
inline std::string to_string(const T& value) {
	std::string s;
	to_string(s, value);
	return s;
}

template <typename T>
inline std::string to_string(const T& value, const FormatSpec& fmt) {
	std::string s;
	to_string(s, value, fmt);
	return s;
}

class FormatArg {
public:
	FormatArg(): name(0) {}
	explicit FormatArg(const char* name_): name(name_) {}

	char const * const name;
	virtual void format(std::string& buf, const FormatSpec& spec) const = 0;
	virtual std::string format(const FormatSpec& spec) const {
		std::string s;
		format(s, spec);
		return s;
	}
};

template <typename T>
class FormatArgT : public FormatArg {
public:
	FormatArgT(const T& value_): value(value_) {}
	FormatArgT(const char* name_, const T& value_): FormatArg(name_), value(value_) {}

	virtual void format(std::string& buf, const FormatSpec& spec) const {
		to_string(buf, value, spec);
	}

private:
	const T value;
};

// this version is safer (doesn't rely on the value out-living the FormatArgT object)
// but performs a string copy
/*
FormatArgT<std::string> formatarg(const char* name, const char* value) {
	return FormatArgT<std::string>(name, std::string(value));
}
*/

FormatArgT<const char*> formatarg(const char* name, const char* value) {
	return FormatArgT<const char*>(name, value);
}

template <typename T>
FormatArgT<T> formatarg(const char* name, const T& value) {
	return FormatArgT<T>(name, value);
}

// underlying formatting function

std::string string_format(const char* fmt, int numargs, FormatArg const * const args[]);

// ---------------------------------------------------------------------------

template <typename T> struct FormatArgWrapper;

template <typename T> struct FormatArgWrapper {
	typedef FormatArgT<T> type;
	static type wrap(const T& arg) { return FormatArgT<T>(arg); }
};
template <int N> struct FormatArgWrapper<char[N]> {
	typedef FormatArgT<const char*> type;
	static type wrap(const char (&arg)[N]) { return FormatArgT<const char*>(arg); }
};
template <> struct FormatArgWrapper<FormatArg> {
	typedef FormatArg type;
	static const type& wrap(const FormatArg& arg) { return arg; }
};
template <typename T> struct FormatArgWrapper< FormatArgT<T> > {
	typedef FormatArgT<T> type;
	static const type& wrap(const FormatArgT<T>& arg) { return arg; }
};

// ---- stringf(format, args...) for 0 to 7 arguments ----

inline std::string stringf(const char* fmt) {
	return string_format(fmt, 0, 0);
}

template <typename T0>
inline std::string stringf(const char* fmt, const T0& p0) {
	const typename FormatArgWrapper<T0>::type& arg0 = FormatArgWrapper<T0>::wrap(p0);
	FormatArg const * const args[] = { &arg0 };
	return string_format(fmt, 1, args);
}

template <typename T0, typename T1>
inline std::string stringf(const char* fmt, const T0& p0, const T1& p1) {
	const typename FormatArgWrapper<T0>::type& arg0 = FormatArgWrapper<T0>::wrap(p0);
	const typename FormatArgWrapper<T1>::type& arg1 = FormatArgWrapper<T1>::wrap(p1);
	FormatArg const * const args[] = { &arg0, &arg1 };
	return string_format(fmt, 2, args);
}

template <typename T0, typename T1, typename T2>
inline std::string stringf(const char* fmt, const T0& p0, const T1& p1, const T2& p2) {
	const typename FormatArgWrapper<T0>::type& arg0 = FormatArgWrapper<T0>::wrap(p0);
	const typename FormatArgWrapper<T1>::type& arg1 = FormatArgWrapper<T1>::wrap(p1);
	const typename FormatArgWrapper<T2>::type& arg2 = FormatArgWrapper<T2>::wrap(p2);
	FormatArg const * const args[] = { &arg0, &arg1, &arg2 };
	return string_format(fmt, 3, args);
}

template <typename T0, typename T1, typename T2, typename T3>
inline std::string stringf(const char* fmt, const T0& p0, const T1& p1, const T2& p2, const T3& p3) {
	const typename FormatArgWrapper<T0>::type& arg0 = FormatArgWrapper<T0>::wrap(p0);
	const typename FormatArgWrapper<T1>::type& arg1 = FormatArgWrapper<T1>::wrap(p1);
	const typename FormatArgWrapper<T2>::type& arg2 = FormatArgWrapper<T2>::wrap(p2);
	const typename FormatArgWrapper<T3>::type& arg3 = FormatArgWrapper<T3>::wrap(p3);
	FormatArg const * const args[] = { &arg0, &arg1, &arg2, &arg3 };
	return string_format(fmt, 4, args);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline std::string stringf(const char* fmt, const T0& p0, const T1& p1, const T2& p2, const T3& p3, const T4& p4) {
	const typename FormatArgWrapper<T0>::type& arg0 = FormatArgWrapper<T0>::wrap(p0);
	const typename FormatArgWrapper<T1>::type& arg1 = FormatArgWrapper<T1>::wrap(p1);
	const typename FormatArgWrapper<T2>::type& arg2 = FormatArgWrapper<T2>::wrap(p2);
	const typename FormatArgWrapper<T3>::type& arg3 = FormatArgWrapper<T3>::wrap(p3);
	const typename FormatArgWrapper<T4>::type& arg4 = FormatArgWrapper<T4>::wrap(p4);
	FormatArg const * const args[] = { &arg0, &arg1, &arg2, &arg3, &arg4 };
	return string_format(fmt, 5, args);
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
	return string_format(fmt, 6, args);
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
	return string_format(fmt, 7, args);
}

#endif