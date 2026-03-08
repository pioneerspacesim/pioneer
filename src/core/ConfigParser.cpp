// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ConfigParser.h"
#include "core/Log.h"

#include <string_view>
#include <fast_float/fast_float.h>

using namespace Config;

std::string to_string(Token::Type type)
{
	switch (type) {
	case Token::Discard: return "Discard";
	case Token::String: return "String";
	case Token::Number: return "Number";
	case Token::Identifier: return "Identifier";
	case Token::Symbol: return "Symbol";
	case Token::EndOfFile: return "EOF";
	default:
		return "<unknown>";
	}
}

// ============================================================================
// Tokenizer
//

// Use the current character to return the best-guess type of
// the next token
static Token::Type guess_next_token(const std::string_view &current)
{
	if (current.empty())
		return Token::EndOfFile;

	if (current[0] == ' ' || current[0] == '\t' || current[0] == '\r' || current[0] == '\n')
		return Token::Discard;

	if (current[0] == '"' || current[0] == '\'')
		return Token::String;

	if (isalpha(current[0]) || current[0] == '_')
		return Token::Identifier;

	if (isdigit(current[0]))
		return Token::Number;

	return Token::Symbol;
}

size_t Tokenizer::default_classifier(Tokenizer *t, std::string_view remaining, Token &out)
{
	out.type = guess_next_token(remaining);

	if (out.type == Token::EndOfFile)
		return 0;

	// eat as much whitespace as possible
	if (out.type == Token::Discard) {
		// eat all contiguous whitespace characters and skip ahead to
		// the next valid token
		size_t next = remaining.find_first_not_of(" \t\r\n");

		return next == std::string_view::npos ? remaining.size() : next;
	}

	size_t length = 1;

	// if this is an identifier, we want to grab all consecutive alphanumberic
	// characters that make up the identifer
	if (out.type == Token::Identifier) {
		while (isalnum(remaining[length]) || remaining[length] == '_')
			length++;
	}

	// potential leading minus-sign for a number
	if (remaining[0] == '-') {
		if (isdigit(remaining[length]))
			out.type = Token::Number;
	}

	// a Number token type means there's an optional '-' and at least one leading digit;
	// parse its value as an optionally-floating point number
	if (out.type == Token::Number) {
		const char *end = fast_float::from_chars(remaining.data(), remaining.data() + remaining.size(), out.value).ptr;
		length = std::distance<>(remaining.data(), end);
	}

	// if it's a string, continue until we find a string terminator character
	if (out.type == Token::String) {
		// store the string symbol in the id field
		out.id = remaining[0];

		for (; length < remaining.size(); length++) {
			if (remaining[length] == out.id) {
				length++;
				break;
			}

			// skip escaped string terminators
			if (remaining[length] == '\\' && length + 1 < remaining.size())
				length++;
		}
	}

	if (out.type == Token::Symbol) {
		// store the symbol character in the id field
		out.id = remaining[0];
	}

	out.range = remaining.substr(0, length);
	return length;
}

Tokenizer::Tokenizer(std::string_view data, Classifier cls) :
	classifier(cls ? cls : default_classifier),
	remaining(data),
	nextLine(1),
	nextOffset(1),
	nextLen(0)
{
	advance();
}


// Pop the given number of characters from the remaining buffer and
// update line-offset information.
// Returns a string_view of the consumed characters.
std::string_view Tokenizer::consume(size_t num)
{
	num = std::min(num, remaining.size());
	std::string_view ret(remaining.data(), num);
	remaining.remove_prefix(num);

	return ret;
}

// Count all newlines encountered over the given span, and determine our offset from the start of the line
void Tokenizer::update(std::string_view span)
{
	size_t offset = span.find('\n');

	while (offset != std::string_view::npos) {
		span.remove_prefix(offset + 1);
		offset = span.find('\n');

		nextLine += 1;
		nextOffset = 1;
	}

	nextOffset += span.size();
}

// Where the magic happens. Returns the cached next token and adds the
// subsequent token to the lookahead cache.
Token Tokenizer::advance()
{
	Token returnTok = next;
	next = Token {};

	currentLine = nextLine;
	currentOffset = nextOffset - next.range.size();

	acquireNextToken();
	return returnTok;
}

void Tokenizer::acquireNextToken()
{
	// Skip the cached next token, advancing the tokenizer state forwards.
	size_t discarded = nextLen;
	nextLen = classifier(this, remaining.substr(discarded), next);

	// Advance the tokenizer state until we find a non-discard token.
	while (next.type == Token::Discard) {
		discarded += nextLen;
		nextLen = classifier(this, remaining.substr(discarded), next);
	}

	// Pop this token (and any discarded characters) off the remaining string.
	// Update line information for the start of this next token.
	update(consume(discarded));
}

bool Tokenizer::discardLine()
{
	// if the nextLine value is different than the current token's line,
	// the current token was the last one on this line, making this a no-op
	if (nextLine != currentLine)
		return false;

	// treat all (potentially-tokenizable) data until the next newline character
	// as part of the old cached token, then advance past it and cache the first
	// token after that (on the next line).
	nextLen = remaining.find_first_of('\n');
	if (nextLen == std::string_view::npos)
		nextLen = remaining.size();

	advance();
	return true;
}

bool Tokenizer::discardCachedLine()
{
	// If the next token is EOF, we can't do anything.
	if (next.type == Token::EndOfFile)
		return false;

	// Find the next newline after the cached next token and discard up to it.
	nextLen = remaining.find_first_of('\n', nextLen);
	if (nextLen == std::string_view::npos)
		nextLen = remaining.size();

	// Set the cached next token to be the first token after the newline.
	acquireNextToken();
	return true;
}

// ============================================================================

Parser::Result Parser::Parse()
{
	while (state.peek().type != Token::EndOfFile && !m_contexts.empty()) {
		// Context list can change due to PushContext()
		size_t activeCtx = m_contexts.size() - 1;
		Context &ctx = *m_contexts[activeCtx].get();

		Token tok = Advance();
		Result res = ctx(this, tok);

		if (res == Result::ParseFailure) {
			return res;
		}

		if (res == Result::DidNotMatch) {
			// Unknown token type; ignore the rest of the line
			// TODO: fallback parser function or immediate parse failure?
			state.discardLine();
		}

		if (ctx.finished) {
			m_contexts.erase(m_contexts.begin() + activeCtx);
		}
	}

	// Partially parsed but tokens still remaining; report the failure.
	if (state.peek().type != Token::EndOfFile)
		return Result::DidNotMatch;

	return Result::Parsed;
}

// Returns true and consumes the next token if it is of the specified type
bool Parser::Acquire(Token::Type type, Token *outTok)
{
	if (!Expect(type, 0, Peek()))
		return false;

	if (outTok != nullptr) {
		*outTok = Advance();
	} else {
		Advance();
	}

	return true;
}

// Check the next token to see if it is a symbol, and advance the parser state if valid.
bool Parser::Symbol(uint8_t id, Token *outTok)
{
	if (!Expect(Token::Symbol, id, Peek()))
		return false;

	if (outTok != nullptr) {
		*outTok = Advance();
	} else {
		Advance();
	}

	return true;
}

// Check the next token to see if it is the given keyword, and advance the parser state if valid.
bool Parser::Keyword(std::string_view name)
{
	const Token &tok = Peek();

	if (!tok.isKeyword(name)) {
		Log::Warning("{} Unknown token '{}', expected '{}'\n", MakeLineInfo(), tok.range, name);
		return false;
	}

	Advance();
	return true;
}

// Returns true if the passed token is of the given type. Otherwise, prints an error message.
bool Parser::Expect(Token::Type type, uint16_t id, const Token &tok) const
{
	if (tok.type == type) return true;
	if (id && tok.id == id) return true;

	if (id != 0)
		Log::Warning("{} Unknown token '{}', expected '{}'\n", MakeLineInfo(), tok.range, char(id));
	else
		Log::Warning("{} Unknown token '{}', expected a {}\n", MakeLineInfo(), tok.range, to_string(type));

	return false;
}

// Advance a token, discarding line comments
// TODO: update classifier to create Comment tokens and discard those instead
Token Parser::Advance()
{
	while (state.peek().isSymbol(lineComment)) {
		state.discardCachedLine();
	}

	return state.advance();
}

const Token &Parser::Peek()
{
	while (state.peek().isSymbol(lineComment)) {
		state.discardCachedLine();
	}

	return state.peek();
}

bool Parser::IsLineRemaining() const
{
	return !state.peek().isSymbol(lineComment) && !state.peekIsNextLine();
}

// Format the current position in the source data
std::string Parser::MakeLineInfo() const
{
	return fmt::format("{}:{}:{}", fileName, state.getCurrentLine(), state.getCurrentChar());
}

// Push a new context onto the stack
Parser::Result Parser::PushContext(Context *ctx)
{
	m_contexts.emplace_back(ctx);
	return Result::Parsed;
}

// Doesn't actually pop the context from the stack as that would delete it while it's still evaluating
Parser::Result Parser::PopContext(Context *ctx)
{
	ctx->finished = true;
	return Result::Parsed;
}
