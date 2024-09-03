// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "StringF.h"

#include <iostream>
#include <sstream>

#include <cassert>
#include <climits>
#include <cstring>

// ---------------------------------------------------------------------------
// ## FormatSpec

FormatSpec::FormatSpec() :
	format("")
{
	for (int i = 0; i <= MAX_PARAMS; ++i)
		params[i] = 0;
}

FormatSpec::FormatSpec(const char *format_) :
	format(format_)
{
	assert(format_);
	parseFormat(strlen(format));
}

FormatSpec::FormatSpec(const char *format_, int formatlen_) :
	format(format_)
{
	assert(format_ && (formatlen_ >= 0));
	parseFormat(formatlen_);
}

bool FormatSpec::empty() const
{
	return (params[MAX_PARAMS] == 0);
}

bool FormatSpec::specifierIs(const char *specifier) const
{
	int len = params[0];
	if (len && (format[len - 1] == ':'))
		--len;
	return (strncmp(specifier, format, len) == 0);
}

int FormatSpec::paramCount() const
{
	int i = 0;
	while ((i < MAX_PARAMS) && (params[i] < params[i + 1]))
		++i;
	return i;
}

std::string FormatSpec::param(int idx) const
{
	const char *beg, *end;
	paramPtr(idx, beg, end);

	std::string str;
	str.reserve(end - beg);

	const char *c = beg;
	while (c != end) {
		// scan to the next escape character or the end of the segment
		while ((c != end) && (*c != '\\'))
			++c;
		// append this run of unescaped characters
		str.append(beg, (c - beg));
		// if we're not at the end, we must have hit a backslash
		if (c != end) {
			++c; // skip the backslash
			if (c != end) {
				beg = c; // start the next run from the escaped character (whatever it is)
				++c; // skip the escaped character (important so we don't interpret a second '\\' as another escape char)
			} else {
				// XXX warning here? or even assert that the format spec doesn't have backslash as the last character?
				str += '\\';
			}
		}
	}

	return str;
}

void FormatSpec::paramPtr(int idx, const char *&begin, const char *&end) const
{
	assert(idx >= 0 && idx < MAX_PARAMS);
	begin = &format[params[idx]];
	end = &format[params[idx + 1]];
	// if it's not the last parameter, then we need to shift end back one,
	// to account for the '|' character that separates parameters
	// note: the last character of a parameter may legitimately be a '|', e.g.
	//  in   %foo{bar:qux\|}
	//  param 0 (before escape processing) is exactly ``qux\|''
	if ((idx + 1 < MAX_PARAMS) && (params[idx + 2] > params[idx + 1]))
		--end;
}

void FormatSpec::parseFormat(int length)
{
	// length limit due to the type used for the params[] array
	assert((length >= 0) && (length <= USHRT_MAX));
	assert(format); // format should already have been set

	const char *c = format;
	const char *end = &format[length];

	// scan the specifier
	while ((c != end) && isalpha(*c))
		++c;

	// skip the optional ':' separating the specifier and the first parameter
	if (*c == ':') ++c;

	for (int i = 0; i < MAX_PARAMS; ++i) {
		params[i] = (c - format);
		// scan to the beginning of the next parameter
		while (c != end) {
			if (*c == '\\') {
				++c;
				if (c != end) ++c;
			} else if (*c == '|') {
				++c;
				break;
			} else
				++c;
		}
	}
	params[MAX_PARAMS] = (c - format);

	assert((c == end) && "FormatSpec::MAX_PARAMS isn't big enough");
}

// ---------------------------------------------------------------------------
// ## to_string() implementations and helpers

struct PrintfSpec {
	enum Flags {
		AlignLeft = 1 << 0, // corresponds with '-' flag
		ForceSign = 1 << 1, // corresponds with '+' flag
		PadSign = 1 << 2, // corresponds with ' ' flag
		PadZero = 1 << 3, // corresponds with '0' flag
		ShowType = 1 << 4 // corresponds with '#' flag
	};

	int width;
	int precision;
	int flags;

	PrintfSpec() :
		width(-1),
		precision(-1),
		flags(0) {}
};

static const char *parse_printfspec(PrintfSpec &spec, const char *fmt, const char *end = 0)
{
	const char *c = fmt;
	//at_flags:
	while ((c != end) && *c) {
		// skip backslash
		if (*c == '\\') ++c;
		if (c == end) return c;
		int addflag = 0;
		switch (*c) {
		// printf flags
		case '-': addflag = PrintfSpec::AlignLeft; break;
		case '+': addflag = PrintfSpec::ForceSign; break;
		case ' ': addflag = PrintfSpec::PadSign; break;
		case '0': addflag = PrintfSpec::PadZero; break;
		case '#': addflag = PrintfSpec::ShowType; break;

		// valid characters for the width specifier
		case '*':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			goto at_width;

		// valid characters for the precision specifier
		case '.':
			goto at_precision;

		// unknown char
		default:
			return c;
		}
		if (addflag && !(spec.flags & addflag)) {
			spec.flags |= addflag;
		} else {
			// duplicate flag
			return c;
		}
		++c;
	}

at_width:
	// variable-width specifier not supported
	if ((c == end) || (*c == '*')) return c;
	if (isdigit(*c)) {
		int width = 0;
		while ((c != end) && isdigit(*c)) {
			width *= 10;
			width += (*c - '0');
			++c;
		}
		spec.width = width;
	}

at_precision:
	if ((c != end) && (*c == '.')) {
		++c;
		// variable-precision specifier not supported
		if ((c == end) || (*c == '*')) return c;
		int prec = 0;
		while ((c != end) && isdigit(*c)) {
			prec *= 10;
			prec += (*c - '0');
			++c;
		}
		spec.precision = prec;
	}
	return c;
}

void init_iosflags(std::ostream &ss, const PrintfSpec &spec)
{
	if (spec.width != -1)
		ss.width(spec.width);
	if (spec.precision != -1)
		ss.precision(spec.precision);

	if (spec.flags & PrintfSpec::AlignLeft)
		ss.setf(std::ios::left, std::ios::adjustfield);
	else {
		if (spec.flags & PrintfSpec::PadZero)
			ss.setf(std::ios::internal, std::ios::adjustfield);
		else
			ss.setf(std::ios::right, std::ios::adjustfield);
	}
	if (spec.flags & PrintfSpec::PadZero) ss.fill('0');
	if (spec.flags & PrintfSpec::ForceSign) ss.setf(std::ios::showpos);
	if (spec.flags & PrintfSpec::ShowType) {
		ss.setf(std::ios::showpoint);
		ss.setf(std::ios::showbase);
	}
}

/*
	if (spec.flags & PrintfSpec::AlignLeft) { ss << '-'; }
	if (spec.flags & PrintfSpec::PadZero)   { ss << '0'; }
	if (spec.flags & PrintfSpec::ForceSign) { ss << '+'; }
	if (spec.flags & PrintfSpec::PadSign)   { ss << ' '; }
	if (spec.flags & PrintfSpec::ShowType)  { ss << '#'; }
	if (spec.width != -1) { ss << spec.width; }
	if (spec.precision != -1) { ss << '.' << spec.precision; }
*/

std::string to_string(int64_t value, const FormatSpec &fmt)
{
	std::ostringstream ss;
	PrintfSpec spec;

	if (!fmt.empty()) {
		if (fmt.specifierIs("d") || fmt.specifierIs("i")) {
			ss.setf(std::ios::dec, std::ios::basefield);
		} else
			return std::string("%(err: bad format)");

		const char *fmtbegin, *fmtend;
		fmt.paramPtr(0, fmtbegin, fmtend);
		if (fmtend != fmtbegin)
			parse_printfspec(spec, fmtbegin, fmtend);
	}

	// sign padding is not supported
	if (spec.flags & PrintfSpec::PadSign)
		return std::string("%(err: bad format)");

	// precision is ignore for integer arguments
	if (spec.precision != -1)
		return std::string("%(err: bad format)");

	init_iosflags(ss, spec);

	ss << value;
	return ss.str();
}

std::string to_string(uint64_t value, const FormatSpec &fmt)
{
	std::ostringstream ss;
	PrintfSpec spec;

	if (!fmt.empty()) {
		if (fmt.specifierIs("u")) {
			ss.setf(std::ios::dec, std::ios::basefield);
		} else if (fmt.specifierIs("x")) {
			ss.setf(std::ios::hex, std::ios::basefield);
		} else if (fmt.specifierIs("X")) {
			ss.setf(std::ios::hex, std::ios::basefield);
			ss.setf(std::ios::uppercase);
		} else if (fmt.specifierIs("o")) {
			ss.setf(std::ios::oct, std::ios::basefield);
		} else
			return std::string("%(err: bad format)");

		const char *fmtbegin, *fmtend;
		fmt.paramPtr(0, fmtbegin, fmtend);
		if (fmtend != fmtbegin)
			parse_printfspec(spec, fmtbegin, fmtend);
	}

	// sign padding is not supported
	if (spec.flags & PrintfSpec::PadSign)
		return std::string("%(err: bad format)");

	// precision is ignore for integer arguments
	if (spec.precision != -1)
		return std::string("%(err: bad format)");

	init_iosflags(ss, spec);

	ss << value;
	return ss.str();
}

std::string to_string(double value, const FormatSpec &fmt)
{
	std::ostringstream ss;
	PrintfSpec spec;

	if (!fmt.empty()) {
		if (fmt.specifierIs("f")) {
			ss.setf(std::ios::fixed, std::ios::floatfield);
		} else if (fmt.specifierIs("g")) {
		} else if (fmt.specifierIs("G")) {
			ss.setf(std::ios::uppercase);
		} else if (fmt.specifierIs("e")) {
			ss.setf(std::ios::scientific, std::ios::floatfield);
		} else if (fmt.specifierIs("E")) {
			ss.setf(std::ios::scientific, std::ios::floatfield);
			ss.setf(std::ios::uppercase);
		} else
			return std::string("%(err: bad format)");

		const char *fmtbegin, *fmtend;
		fmt.paramPtr(0, fmtbegin, fmtend);
		if (fmtend != fmtbegin)
			parse_printfspec(spec, fmtbegin, fmtend);
	}

	// sign padding is not supported
	if (spec.flags & PrintfSpec::PadSign)
		return std::string("%(err: bad format)");

	init_iosflags(ss, spec);

	ss << value;
	return ss.str();
}

std::string to_string(const char *value, const FormatSpec &fmt)
{
	if (fmt.empty()) {
		return std::string(value);
	} else {
		return std::string("%(err: bad format)");
	}
}

std::string to_string(const std::string &value, const FormatSpec &fmt)
{
	if (fmt.empty()) {
		return value;
	} else {
		return std::string("%(err: bad format)");
	}
}

// ---------------------------------------------------------------------------
// ## string_format() and helpers

// static inline const char* scan_unformatted(const char* fmt) {
// 	const char *c = fmt;
// 	return c;
// }

static inline const char *scan_int(const char *fmt, int &value)
{
	value = 0;
	const char *c = fmt;
	while (*c >= 0 && isdigit(*c)) {
		value *= 10;
		value += (*c - '0');
		++c;
	}
	return c;
}

static inline const char *scan_ident(const char *fmt)
{
	const char *c = fmt;
	if (isalpha(*c) || (*c == '_')) {
		++c;
		while (isalnum(*c) || (*c == '_'))
			++c;
	}
	return c;
}

static inline const char *scan_bracetext(const char *fmt)
{
	const char *c = fmt;
	while (*c && (*c != '}')) {
		if (*c == '\\') {
			++c;
			if (*c) ++c;
		} else
			++c;
	}
	return c;
}

std::string string_format(const char *fmt, int numargs, FormatArg const *const args[])
{
	std::string out;
	const char *c = fmt;
	while (*c) {
		while (*c && *c != '%')
			++c;
		if (c != fmt) out.append(fmt, c - fmt);
		fmt = c;

		if (*c == '%') {
			++c;
			if (*c == '%') {
				fmt = c;
				++c;
			} else {
				int argid = -1;
				const char *identBegin = c;
				const char *identEnd = c;

				// parse and match reference
				if (isdigit(*c)) {
					c = identEnd = scan_int(identBegin = c, argid);
				} else {
					if (*c == '{') {
						identBegin = ++c;
						while (*c && (*c != '}'))
							++c;
						identEnd = c;
						if (*c == '}')
							++c;
						else {
							out.append("%(err: unfinished reference)");
							goto bad_reference;
						}
					} else if (isalpha(*c) || (*c == '_')) {
						c = identEnd = scan_ident(identBegin = c);
					} else {
						out.append("%(err: unfinished reference)");
						goto bad_reference;
					}

					if (identBegin == identEnd) {
						out.append("%(err: blank reference)");
						goto bad_reference;
					}

					for (int i = 0; i < numargs; ++i) {
						size_t identLen = (identEnd - identBegin);
						if (args[i]->name && (strncmp(args[i]->name, identBegin, identLen) == 0) && (args[i]->name[identLen] == '\0')) {
							argid = i;
							break;
						}
					}
				}

				if (argid >= 0 && argid < numargs) {
					const FormatArg &arg = *args[argid];
					const char *fmtBegin = 0;
					const char *fmtEnd = 0;
					// scan format specifier, if provided
					if (*c == '{') {
						++c;
						if (!*c) {
							out.append("%(err: unfinished format)");
							goto bad_reference;
						}

						c = fmtEnd = scan_bracetext(fmtBegin = c);

						if (fmtBegin == fmtEnd) {
							fmtBegin = fmtEnd = 0;
						}

						if (!*c) {
							out.append("%(err: unfinished format)");
							goto bad_reference;
						} else
							++c;
					}

					if (fmtBegin) {
						assert(fmtEnd > fmtBegin);
						FormatSpec fspec(fmtBegin, fmtEnd - fmtBegin);
						out.append(arg.format(fspec));
					} else if (arg.defaultformat) {
						FormatSpec fspec(arg.defaultformat);
						out.append(arg.format(fspec));
					} else
						out.append(arg.format(FormatSpec()));

				} else {
					out.append("%(err: unknown arg '");
					out.append(identBegin, identEnd - identBegin);
					out.append("')");
					goto bad_reference;
				}

			bad_reference:
				fmt = c;
			}
		}
	}
	// append the final range
	if (c != fmt)
		out.append(fmt, c - fmt);
	return out;
}
