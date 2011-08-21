#include "StringF.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <cassert>

const FormatSpec FormatSpec::DEFAULT_FORMAT_SPEC;

void init_iosflags(const FormatSpec& fmt, std::ostream& ss) {
	ss.width(fmt.minwidth);
	ss.precision(fmt.precision);

	if (fmt.flags & FormatSpec::ShowType) {
		ss.setf(std::ios::showbase);
		ss.setf(std::ios::showpoint);
	}
	if (fmt.flags & FormatSpec::AlignLeft)
		ss.setf(std::ios::left, std::ios::adjustfield);
	if (fmt.flags & FormatSpec::ForceSign)
		ss.setf(std::ios::showpos);

	if (fmt.flags & FormatSpec::PadLeftZero)
		ss.fill('0');
	else
		ss.fill(' ');

	if (fmt.form == FormatSpec::HexUpper) {
		ss.setf(std::ios::hex, std::ios::basefield);
		ss.setf(std::ios::uppercase);
	} else if (fmt.form == FormatSpec::HexLower) {
		ss.setf(std::ios::hex, std::ios::basefield);
		ss.unsetf(std::ios::uppercase);
	} else if (fmt.form == FormatSpec::Oct)
		ss.setf(std::ios::oct, std::ios::basefield);
	else 
		ss.setf(std::ios::dec, std::ios::basefield);

	if (fmt.form == FormatSpec::Sci)
		ss.setf(std::ios::scientific, std::ios::floatfield);
	else if (fmt.form == FormatSpec::Fix)
		ss.setf(std::ios::fixed, std::ios::floatfield);
	else
		ss.unsetf(std::ios::floatfield);
}

void to_string(std::string& buf, int64_t value, const FormatSpec& fmt) {
	(void)(fmt);
	std::ostringstream ss;
	init_iosflags(fmt, ss);
	ss << value;
	buf.append(ss.str());
}

void to_string(std::string& buf, uint64_t value, const FormatSpec& fmt) {
	(void)(fmt);
	std::ostringstream ss;
	init_iosflags(fmt, ss);
	ss << value;
	buf.append(ss.str());
}

void to_string(std::string& buf, double value, const FormatSpec& fmt) {
	(void)(fmt);
	std::ostringstream ss;
	init_iosflags(fmt, ss);
	ss << value;
	buf.append(ss.str());
}

void to_string(std::string& buf, const char* value, const FormatSpec& fmt) {
	(void)(fmt);
	size_t len = strlen(value);
	bool pad = (len < size_t(fmt.minwidth));
	if (pad && ((fmt.flags & FormatSpec::AlignLeft) == 0))
		buf.append(fmt.minwidth - len, (fmt.flags & FormatSpec::PadLeftZero) ? '0' : ' ');
	buf.append(value, len);
	if (pad && ((fmt.flags & FormatSpec::AlignLeft) != 0))
		buf.append(fmt.minwidth - len, ' ');
}

void to_string(std::string& buf, const std::string& value, const FormatSpec& fmt) {
	(void)(fmt);
	size_t len = value.size();
	bool pad = (len < size_t(fmt.minwidth));
	if (pad && ((fmt.flags & FormatSpec::AlignLeft) == 0))
		buf.append(fmt.minwidth - len, (fmt.flags & FormatSpec::PadLeftZero) ? '0' : ' ');
	buf.append(value);
	if (pad && ((fmt.flags & FormatSpec::AlignLeft) != 0))
		buf.append(fmt.minwidth - len, ' ');
}

/*
 *   template = ( plain ( percentfmt | bracefmt )* )+
 *   plain = [^%{]*
 *
 *   percentfmt = '%' flags width? precision? ( 'h' | 'l' | 'll' | 'L' )? [cdieEfgGosuxXpn]
 *   flags = [-+ #0]+ // but no repeats
 *   width = number | '*'
 *   precision = '.' ( number | '*' )
 *
 *   bracefmt = '{' argid ( ':' fmtspec )? '}'
 *   argid = number | id
 *
 *   id = ( alpha | '_' ) ( alpha | decimal | '_' )*
 *
 *   fmtspec = ( flags? number? ( '.' number )? ) | fmtfunc
 *   fmtfunc = ( id ( ':' [^|}]+ ( '|' [^|}]+ )* )? )
 *
 *   number = decimal+
 *
 *   decimal = [0123456789]
 *   alpha = // characters for which isalpha() returns true
 */

const char* parse_fmtspec(FormatSpec& spec, const char* fmt) {
	const char* c = fmt;
//at_flags:
	while (*c) {
		switch (*c) {
			case '-':
				assert(((spec.flags & FormatSpec::AlignLeft) == 0) &&
					"Bad format spec (duplicate flag)");
				spec.flags |= FormatSpec::AlignLeft;
				break;
			case '+':
				assert(((spec.flags & FormatSpec::ForceSign) == 0) &&
					"Bad format spec (duplicate flag)");
				spec.flags |= FormatSpec::ForceSign;
				break;
			case ' ':
				assert(((spec.flags & FormatSpec::PadSign) == 0) &&
					"Bad format spec (duplicate flag)");
				spec.flags |= FormatSpec::PadSign;
				break;
			case '0':
				assert(((spec.flags & FormatSpec::PadLeftZero) == 0) &&
					"Bad format spec (duplicate flag)");
				spec.flags |= FormatSpec::PadLeftZero;
				break;
			case '#':
				assert(((spec.flags & FormatSpec::ShowType) == 0) &&
					"Bad format spec (duplicate flag)");
				spec.flags |= FormatSpec::ShowType;
				break;
			case '*':
			case '1': case '2': case '3': case '4': case '5':
			case '6': case '7': case '8': case '9':
				goto at_width;
			case '.':
				goto at_precision;
			default:
				return c;
		}
		++c;
	}
at_width:
	if ((*c == '*') || isdigit(*c)) {
		assert((*c != '*') && "Variable-width precision specifier not supported.");
		int width = 0;
		while (*c) {
			switch (*c) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				width *= 10;
				width += (*c - '0');
				break;
			case '.':
				spec.minwidth = width;
				goto at_precision;
			default:
				spec.minwidth = width;
				return c;
			}
			++c;
		}
		spec.minwidth = width;
	}
at_precision:
	if (*c == '.') {
		++c;
		assert((*c != '*') && "Variable-width precision specifier not supported.");
		assert(isdigit(*c) && "Bad precision specifier");
		int prec = 0;
		while (*c) {
			if (isdigit(*c)) {
				prec *= 10;
				prec += (*c - '0');
			} else {
				spec.precision = prec;
				return c;
			}
			++c;
		}
	}
	return c;
}

const char* parse_printfspec(FormatSpec& spec, const char* fmt) {
	const char* c = parse_fmtspec(spec, fmt);
	switch (*c) {
		case 'h':
			++c;
			break;
		case 'l':
			++c;
			if (*c == 'l') ++c;
			break;
		case 'L':
			++c;
			if (*c == 'L') ++c;
			break;
		default: break;
	}
	switch (*c) {
	case 's': case 'c': case 'd': case 'i': case 'u': case 'g': case 'G':
		++c;
		spec.form = FormatSpec::Gen;
		break;
	case 'o':
		++c;
		spec.form = FormatSpec::Oct;
		break;
	case 'x':
		++c;
		spec.form = FormatSpec::HexLower;
		break;
	case 'X':
		++c;
		spec.form = FormatSpec::HexUpper;
		break;
	case 'e': case 'E':
		++c;
		spec.form = FormatSpec::Sci;
		break;
	case 'f':
		++c;
		spec.form = FormatSpec::Fix;
		break;
	case 'p': case 'n':
		++c;
		assert(0 && "Pointer and character-count specifiers are not supported.");
		break;
	default:
		assert(0 && "Bad format specifier.");
		break;
	}
//at_end:
	return c;
}

const char* parse_bracespec(FormatSpec& spec, const char* fmt) {
	const char* c = parse_fmtspec(spec, fmt);
	assert((*c == '}') && "Bad brace format specifier");
	return c;
}

std::string string_format(const char* fmt, int numargs, FormatArg const * const args[]) {
	std::string out;

	const char *mark = fmt, *c = fmt;
	int nextarg = 0;
	while (*c) {
		if (*c == '%') {
			out.append(mark, size_t(c - mark));
			++c;
			if (*c == '%') {
				++c;
				out += '%';
			} else {
				int arg = nextarg++;
				assert((arg >= 0) && (arg < numargs) && "Invalid format string: reference to out-of-bounds argument.");
				FormatSpec spec;
				c = parse_printfspec(spec, c);
				out.append(args[arg]->format(spec));
			}
			mark = c;
		} else if (*c == '{') {
			out.append(mark, size_t(c - mark));
			++c;
			if (*c == '{') {
				++c;
				out += '{';
			} else {
				const char* namebegin = c;
				while (*c && (isalnum(*c) || ((*c == '-') || (*c == '_') || (*c == '.'))))
					++c;
				const char* nameend = c;

				int arg = -1;
				if (isdigit(*namebegin)) {
					arg = 0;
					const char* cc = namebegin;
					while ((arg < numargs) && (*cc >= '0' && *cc <= '9')) {
						arg *= 10;
						arg += (*cc - '0');
						++cc;
					}
					assert((cc == nameend) && "Invalid format string: bad argument index.");
				} else {
					for (arg = 0; arg < numargs; ++arg) {
						if (args[arg]->name && (strncmp(args[arg]->name, namebegin, size_t(nameend - namebegin)) == 0))
							break;
					}
				}
				assert((arg >= 0) && (arg < numargs) && "Invalid format string: reference to unknown named argument.");

				FormatSpec spec;
				if (*c == ':') {
					++c;
					c = parse_bracespec(spec, c);
				}
				assert((*c == '}') && "Invalid format string: un-finished argument reference");
				++c;

				out.append(args[arg]->format(spec));
			}
			mark = c;
		} else
			++c;
	}
	out.append(mark, size_t(c - mark));
	return out;
}
