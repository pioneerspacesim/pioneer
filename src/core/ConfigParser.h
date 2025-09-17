// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Config {

	struct Token {
		// Very lightweight tokenization.
		enum Type : uint8_t {
			Discard = 0, // not a real token
			EndOfFile = 26, // not a real token

			// Token kinds are used as an interface for the token classifier.
			// The token ID is freely available for the token classifier to set.
			String = 1,
			Number = 2,
			Identifier = 3,
			Comment = 4,
			Symbol = 5,
		};

		Type type;
		uint16_t id;
		float value;
		std::string_view range;

		std::string_view trim(int32_t start, int32_t end) const
		{
			size_t pos = start < 0 ? range.size() - start : start;
			size_t len = (end < 0 ? range.size() + end : end) - start;
			return range.substr(pos, len);
		}

		// Return the contents of the token; for strings this is the string data without quotes;
		// for all other tokens it is the textual representation of the token.
		std::string_view contents() const { return type == Token::String ? trim(1, -1) : range; }

		// Convert the contents of the token to a string.
		std::string toString() const { return std::string{ contents() }; }

		bool isKeyword(std::string_view keyword) const
		{
			return type == Type::Identifier && range.compare(keyword) == 0;
		}

		bool isSymbol(uint16_t id) const
		{
			return type == Type::Symbol && this->id == id;
		}
	};

	struct Tokenizer {
	public:
		using Classifier = size_t (*)(Tokenizer *t, std::string_view data, Token &out);

		// Basic classifier suitable for configuration files and the like. Does not recognize comment characters or multi-character symbols.
		static size_t default_classifier(Tokenizer *t, std::string_view data, Token &out);

		Tokenizer() = default;
		Tokenizer(std::string_view data, Classifier classifier = nullptr);

		// Returns the next token in the stream.
		// TODO: support for pushing the last token back on the stream (for look-ahead)
		Token advance();

		// Advance the cached next token, skipping the currently cached token.
		void acquireNextToken();

		// Discard all tokens on the current line, such that a subsequent call to
		// advance() will return the first token on the next non-empty line.
		// Returns true if any tokens were discarded.
		bool discardLine();

		// Discard the cached next token and the remainder of the line it occurs on.
		// Updates the next token to be the first valid token on the following line.
		bool discardCachedLine();

		bool peekIsNextLine() const { return nextLine != currentLine; }

		const Token &peek() const { return next; }
		uint32_t getCurrentLine() const { return currentLine; }
		uint32_t getCurrentChar() const { return currentOffset; }

	private:
		// Consume the given number of characters from the remaining data.
		std::string_view consume(size_t num);
		// Update the next line / next offset information for the next token.
		void update(std::string_view span);

		Classifier classifier;

		Token next;
		std::string_view remaining;

		uint32_t currentLine;
		uint32_t currentOffset;
		uint32_t nextLine;
		uint32_t nextOffset;
		size_t nextLen;
	};

	struct Parser {
	public:
		enum class Result {
			DidNotMatch,
			ParseFailure,
			Parsed
		};

		struct Context {
			using Result = Parser::Result;
			virtual ~Context() = default;

			// Parse the given token (and any required following tokens).
			// Returns DidNotMatch if the token is unknown, ParseFailure if a syntax error occurred,
			// or Parsed if the token(s) were successfully parsed.
			virtual Result operator()(Parser *parser, const Token &tok) = 0;
			bool finished = false;
		};

		Parser() = default;
		Parser(Context *initialCtx, uint8_t lineComment = Token::Discard) :
			lineComment(lineComment)
		{
			PushContext(initialCtx);
		}

		// Initialize the parser to read from the given file contents
		void Init(std::string_view path, std::string_view data)
		{
			fileName = path;
			state = Tokenizer(data);
		}

		// Destroy all contexts and clear tokenizer state and filename data.
		void Reset()
		{
			fileName = "";
			state = Tokenizer();
			m_contexts.clear();
		}

		// Process the file data through the current context.
		Result Parse();

		// Check the next token to see if it is a symbol, and advance the parser state if valid.
		bool Symbol(uint8_t id, Token *outTok = nullptr);

		// Check the next token type and advance the parser state if valid, storing the parsed token in outTok if not nullptr.
		// Logs an error message if the token is not of a valid type.
		bool Acquire(Token::Type type, Token *outTok = nullptr);

		// Checks the next token to be a keyword and advances the parser state if valid.
		bool Keyword(std::string_view name);

		// Returns true if the passed token is of the given type (optionally with the given id). Otherwise, logs an error message.
		// IDs must be non-zero.
		bool Expect(Token::Type type, const Token &tok) const { return Expect(type, 0, tok); }
		bool Expect(Token::Type type, uint16_t id, const Token &tok) const;

		// Return the next valid token, discarding line comments of the configured token type. Advances the tokenizer state.
		Token Advance();

		// Peek into the next valid token, discarding line comments of the configured token type.
		const Token &Peek();

		// Return true if there are non-comment tokens left on the current line.
		bool IsLineRemaining() const;

		// Format the current position in the source data
		std::string MakeLineInfo() const;

		// Returns a dummy Result::Parsed
		Result PushContext(Context *);
		// Returns a dummy Result::Parsed
		Result PopContext(Context *);

		Tokenizer state;
		std::string fileName;
		uint8_t lineComment = Token::Discard;
	private:
		std::vector<std::unique_ptr<Context>> m_contexts;
	};

} // namespace Config
