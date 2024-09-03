// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShaderParser.h"
#include "core/Log.h"
#include "utils.h"

#include <cctype>
#include <string_view>

using namespace Graphics::ShaderParser;

std::string to_string(Token::Type type)
{
	switch (type) {
	case Token::Discard: return "Discard";
	case Token::String: return "String";
	case Token::Number: return "Number";
	case Token::Identifier: return "Identifier";
	case Token::EndOfFile: return "EOF";
	default:
		return "<unknown>";
	}
}

// ============================================================================
// Tokenizer
//

Tokenizer::Tokenizer(std::string_view data) :
	remaining(data),
	nextLine(1),
	nextOffset(1)
{
	advance();
}

// Use the current character to return the best-guess type of
// the next token
Token::Type guess_next_token(std::string_view current)
{
	if (current.empty())
		return Token::EndOfFile;

	if (current[0] == ' ' || current[0] == '\t' || current[0] == '\r' || current[0] == '\n')
		return Token::Discard;

	if (current[0] == '"')
		return Token::String;

	if (isalpha(current[0]) || current[0] == '_')
		return Token::Identifier;

	if (isdigit(current[0]))
		return Token::Number;

	return Token::Type(current[0]);
}

// Pop the given number of characters from the remaining buffer and
// update line-offset information.
// Returns a string_view of the consumed characters.
std::string_view Tokenizer::consume(size_t num)
{
	num = std::min(num, remaining.size());
	std::string_view ret(remaining.data(), num);
	nextOffset += num;
	remaining.remove_prefix(num);
	return ret;
}

// Where the magic happens. Returns the cached next token and adds the
// subsequent token to the lookahead cache.
Token Tokenizer::advance()
{
	Token returnTok = next;
	currentLine = nextLine;
	currentOffset = nextOffset - next.range.size();

	Token::Type nextType = guess_next_token(remaining);

	// eat as much whitespace as possible
	while (nextType == Token::Discard) {
		// if this is a newline, eat it and reset line tracking
		if (remaining[0] == '\n') {
			nextLine += 1;
			nextOffset = 1;
			remaining.remove_prefix(1);
		}

		// eat all contiguous whitespace characters and skip ahead to
		// the next newline or valid token
		consume(remaining.find_first_not_of(" \t\r"));
		nextType = guess_next_token(remaining);
	}

	size_t length = 1;

	// if this is an identifier, we want to grab all consecutive alphanumberic
	// characters that make up the identifer
	if (nextType == Token::Identifier) {
		while (isalnum(remaining[length]) || remaining[length] == '_')
			length++;
	}

	// otherwise if it's a digit we assume it's a plain, unadorned integer
	if (nextType == Token::Number) {
		while (isdigit(remaining[length]))
			length++;
	}

	// if it's a string, continue until we find a string terminator character
	if (nextType == Token::String) {
		for (; length < remaining.size(); length++) {
			if (remaining[length] == '"') {
				length++;
				break;
			}

			// skip escaped string terminators
			if (remaining[length] == '\'' && length + 1 < remaining.size())
				length++;
		}
	}

	next.type = nextType;
	next.range = consume(length);

	return returnTok;
}

bool Tokenizer::discardLine()
{
	// if the nextLine value is different than the current token's line,
	// the current token was the last one on this line, making this a no-op
	if (nextLine != currentLine)
		return false;

	// consume all (potentially-tokenizable) data until the next newline character,
	// then throw away the old cached token and cache the first token on the next line.
	consume(remaining.find_first_of('\n'));
	advance();
	return true;
}

// ============================================================================
// Parser
//

ShaderInfo Parser::Parse(std::string fileName, std::string_view fileData)
{
	m_currentTok = Token{};
	m_fileName = fileName;
	m_tokenizer.reset(new Tokenizer(fileData));

	m_shaderData = ShaderInfo{};
	m_nextTextureBinding = 0;
	m_nextPushConstantBinding = 0;
	m_nextBufferBinding = 0;

	// loop through the file until we hit the end of it,
	// complaining if we find something we don't know how to parse.
	while (m_tokenizer->peek().type != Token::EndOfFile) {
		advance();

		if (!expect(Token::Identifier, m_currentTok)) {
			m_tokenizer->discardLine();
			continue;
		}

		// complain if the Shader <name> line isn't at the top of the document
		if (m_shaderData.name.empty() && !isKeyword("Shader", m_currentTok)) {
			Log::Warning("{} Expected Shader name declaration as first statement in file.\n", makeLineInfo());
			m_shaderData.name = "<unknown>";
		}

		// Parse "Shader <shaderName>"
		if (isKeyword("Shader", m_currentTok)) {
			if (advanceIfType(Token::Identifier))
				m_shaderData.name = std::string(m_currentTok.range);

			continue;
		}

		if (parseTexture() != ParseResult::DidNotMatch)
			continue;

		if (parseBuffer() != ParseResult::DidNotMatch)
			continue;

		if (parsePushConstant() != ParseResult::DidNotMatch)
			continue;

		// Parse "Vertex <path>"
		if (isKeyword("Vertex", m_currentTok)) {
			if (advanceIfType(Token::String))
				m_shaderData.vertexPath = std::string(m_currentTok.trim(1, -1));

			continue;
		}

		// Parse "Fragment <path>"
		if (isKeyword("Fragment", m_currentTok)) {
			if (advanceIfType(Token::String))
				m_shaderData.fragmentPath = std::string(m_currentTok.trim(1, -1));

			continue;
		}

		// If we get an identifier we don't know what to do with, complain about it and skip the line.
		Log::Warning("{} Expected one of 'Shader', 'Texture', 'Buffer', 'PushConstant', 'Fragment', 'Vertex'; got '{}'\n",
			makeLineInfo(), m_currentTok.range);
		m_tokenizer->discardLine();
	}

	return m_shaderData;
}

// Parses "Texture <samplerType> <textureName> [name=diffuse] [binding=0]"
Parser::ParseResult Parser::parseTexture()
{
	if (!isKeyword("Texture", m_currentTok))
		return ParseResult::DidNotMatch;

	if (!advanceIfType(Token::Identifier))
		return ParseResult::ParseFailure;

	TextureInfo texInfo{};
	if (compare_ci(m_currentTok.range, "sampler2d")) {
		texInfo.type = Graphics::TextureType::TEXTURE_2D;
	} else if (compare_ci(m_currentTok.range, "sampler2dArray")) {
		texInfo.type = Graphics::TextureType::TEXTURE_2D_ARRAY;
	} else if (compare_ci(m_currentTok.range, "samplerCube")) {
		texInfo.type = Graphics::TextureType::TEXTURE_CUBE_MAP;
	} else {
		Log::Warning("{} Expected one of 'texture2D, texture2DArray, samplerCube', got '{}'\n",
			makeLineInfo(), m_currentTok.range);
		return ParseResult::ParseFailure;
	}

	if (!advanceIfType(Token::Identifier))
		return ParseResult::ParseFailure;

	texInfo.bindName = std::string(m_currentTok.range);

	if (advanceIfKeyword("name") && advanceIfType(Token::Equals) && advanceIfType(Token::Identifier)) {
		texInfo.name = std::string(m_currentTok.range);
	}

	if (advanceIfKeyword("binding") && advanceIfType(Token::Equals) && advanceIfType(Token::Number)) {
		texInfo.binding = std::stoi(std::string(m_currentTok.range));
		m_nextTextureBinding = texInfo.binding + 1;
	} else {
		texInfo.binding = m_nextTextureBinding++;
	}

	m_shaderData.textureBindings.emplace_back(std::move(texInfo));
	advanceIfLineRemaining();
	return ParseResult::Parsed;
}

// Parses "Buffer <bufferName> [binding=0]"
Parser::ParseResult Parser::parseBuffer()
{
	if (!isKeyword("Buffer", m_currentTok))
		return ParseResult::DidNotMatch;

	if (!advanceIfType(Token::Identifier))
		return ParseResult::ParseFailure;

	BufferInfo bufInfo{};
	bufInfo.name = std::string(m_currentTok.range);

	if (advanceIfKeyword("binding") && advanceIfType(Token::Equals) && advanceIfType(Token::Number)) {
		bufInfo.binding = std::stoi(std::string(m_currentTok.range));
		m_nextBufferBinding = bufInfo.binding + 1;
	} else {
		bufInfo.binding = m_nextBufferBinding++;
	}

	m_shaderData.bufferBindings.emplace_back(std::move(bufInfo));
	advanceIfLineRemaining();
	return ParseResult::Parsed;
}

// Parses "PushConstant <type> <constantName> [binding=0]"
Parser::ParseResult Parser::parsePushConstant()
{
	if (!isKeyword("PushConstant", m_currentTok))
		return ParseResult::DidNotMatch;

	if (!advanceIfType(Token::Identifier))
		return ParseResult::ParseFailure;

	PushConstantInfo constInfo{};
	if (compare_ci(m_currentTok.range, "int")) {
		constInfo.type = Graphics::ConstantDataFormat::DATA_FORMAT_INT;
	} else if (compare_ci(m_currentTok.range, "float")) {
		constInfo.type = Graphics::ConstantDataFormat::DATA_FORMAT_FLOAT;
	} else if (compare_ci(m_currentTok.range, "vec3")) {
		constInfo.type = Graphics::ConstantDataFormat::DATA_FORMAT_FLOAT3;
	} else if (compare_ci(m_currentTok.range, "vec4")) {
		constInfo.type = Graphics::ConstantDataFormat::DATA_FORMAT_FLOAT4;
	} else if (compare_ci(m_currentTok.range, "mat3")) {
		constInfo.type = Graphics::ConstantDataFormat::DATA_FORMAT_MAT3;
	} else if (compare_ci(m_currentTok.range, "mat4")) {
		constInfo.type = Graphics::ConstantDataFormat::DATA_FORMAT_MAT4;
	} else {
		Log::Warning("{} Expected one of 'int', 'float', 'vec3', 'vec4', 'mat3', 'mat4'; got '{}'\n",
			makeLineInfo(), m_currentTok.range);
		advanceIfLineRemaining();
		return ParseResult::ParseFailure;
	}

	if (!advanceIfType(Token::Identifier))
		return ParseResult::ParseFailure;

	constInfo.name = std::string(m_currentTok.range);

	if (advanceIfKeyword("binding") && advanceIfType(Token::Equals) && advanceIfType(Token::Number)) {
		constInfo.binding = std::stoi(std::string(m_currentTok.range));

		if (constInfo.binding < m_nextPushConstantBinding) {
			Log::Warning("{} PushConstant binding '{}' index {} is before next-free index {}. Assigning that index.\n",
				makeLineInfo(), constInfo.name, constInfo.binding, m_nextPushConstantBinding);
			constInfo.binding = m_nextPushConstantBinding;
		}

		for (const auto &info : m_shaderData.pushConstantBindings) {
			if (info.binding == constInfo.binding) {
				Log::Warning("{} PushConstant binding '{}' index {} is already taken by constant '{}'. Assigning index {}.\n",
					makeLineInfo(), constInfo.name, constInfo.binding, info.binding, m_nextPushConstantBinding);
				constInfo.binding = m_nextPushConstantBinding;
			}
		}

		m_nextPushConstantBinding = constInfo.binding + 1;
	} else {
		constInfo.binding = m_nextPushConstantBinding++;
	}

	m_shaderData.pushConstantBindings.emplace_back(std::move(constInfo));
	advanceIfLineRemaining();
	return ParseResult::Parsed;
}

// ============================================================================
// Parser Helper Functions
//

// Format the current position in the source data
std::string Parser::makeLineInfo()
{
	return fmt::format("{}:{}:{}", m_fileName,
		m_tokenizer->getCurrentLine(), m_tokenizer->getCurrentChar());
}

// Returns true if the passed token is of the given type. Otherwise, prints an error message.
bool Parser::expect(Token::Type type, const Token &tok)
{
	if (tok.type == type) return true;

	if (type <= Token::EndOfFile)
		Log::Warning("{} Unknown token '{}', expected a {}\n", makeLineInfo(), tok.range, to_string(type));
	else
		Log::Warning("{} Unknown token '{}', expected '{}'\n", makeLineInfo(), tok.range, char(type));

	return false;
}

// Returns true if the passed token is the given keyword
bool Parser::isKeyword(std::string_view keyword, const Token &tok)
{
	return tok.type == Token::Identifier && tok.range.compare(keyword) == 0;
}

// Make the next token the current token, discarding comments.
void Parser::advance()
{
	// Discard comments.
	while (m_tokenizer->peek().type == Token::Hash) {
		m_tokenizer->advance();
		m_tokenizer->discardLine();
	}

	m_currentTok = m_tokenizer->advance();
}

// If the next token is of the given type, make it the current token
bool Parser::advanceIfType(Token::Type type, bool shouldExpect)
{
	// Discard comments.
	while (m_tokenizer->peek().type == Token::Hash) {
		m_tokenizer->advance();
		m_tokenizer->discardLine();
	}

	if (shouldExpect)
		expect(type, m_tokenizer->peek());

	if (m_tokenizer->peek().type == type) {
		m_currentTok = m_tokenizer->advance();
		return true;
	}

	return false;
}

// If the next token is the given keyword, make it the current token
bool Parser::advanceIfKeyword(std::string_view keyword)
{
	// Discard comments.
	while (m_tokenizer->peek().type == Token::Hash) {
		m_tokenizer->advance();
		m_tokenizer->discardLine();
	}

	if (isKeyword(keyword, m_tokenizer->peek())) {
		m_currentTok = m_tokenizer->advance();
		return true;
	}

	return false;
}

// Warns about and discards anything left on the current line.
bool Parser::advanceIfLineRemaining()
{
	bool didAdvance = false;
	if (m_tokenizer->peek().type != Token::Hash && !m_tokenizer->peekIsNextLine()) {
		m_currentTok = m_tokenizer->advance();
		didAdvance = true;
		Log::Warning("{} Unexpected token '{}'; discarding remainder of line\n",
			makeLineInfo(), m_currentTok.range);
	}

	m_tokenizer->discardLine();

	return didAdvance;
}
