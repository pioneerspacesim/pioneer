// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "graphics/Texture.h"
#include "graphics/Types.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Graphics {

	namespace ShaderParser {

		struct Token {
			// very lightweight token info, just enough to parse a barebones
			// shaderdef file. Eventually look at being able to lex GLSL files for
			// inline 'combined' shader code.
			enum Type : uint8_t {
				Discard = 0,
				String = 1,
				Number = 2,
				Identifier = 3,
				EndOfFile = 26,

				LBrace = '{',
				RBrace = '}',
				Equals = '=',
				Hash = '#'
			};

			Type type;
			std::string_view range;

			std::string_view trim(int32_t start, int32_t end)
			{
				size_t pos = start < 0 ? range.size() - start : start;
				size_t len = (end < 0 ? range.size() + end : end) - start;
				return range.substr(pos, len);
			}
		};

		struct Tokenizer {
		public:
			Tokenizer(std::string_view data);

			// Returns the next token in the stream.
			// TODO: support for pushing the last token back on the stream (for look-ahead)
			Token advance();

			// Discard all tokens on the current line, such that a subsequent call to
			// advance() will return the first token on the next non-empty line.
			// Returns true if any tokens were discarded.
			bool discardLine();
			bool peekIsNextLine() const { return nextLine != currentLine; }

			const Token &peek() const { return next; }
			uint32_t getCurrentLine() const { return currentLine; }
			uint32_t getCurrentChar() const { return currentOffset; }

		private:
			std::string_view consume(size_t num);

			Token next;
			std::string_view remaining;
			uint32_t currentLine;
			uint32_t currentOffset;
			uint32_t nextLine;
			uint32_t nextOffset;
		};

		struct TextureInfo {
			Graphics::TextureType type;
			std::string bindName;
			std::string name;
			uint32_t binding;
		};

		struct BufferInfo {
			std::string name;
			uint32_t binding;
		};

		struct PushConstantInfo {
			Graphics::ConstantDataFormat type;
			std::string name;
			uint32_t binding;
		};

		struct ShaderInfo {
			std::string name;
			std::string vertexPath;
			std::string fragmentPath;

			std::vector<TextureInfo> textureBindings;
			std::vector<BufferInfo> bufferBindings;
			std::vector<PushConstantInfo> pushConstantBindings;
		};

		class Parser {
		public:
			ShaderInfo Parse(std::string filename, std::string_view fileData);

		protected:
			enum ParseResult {
				DidNotMatch,
				ParseFailure,
				Parsed
			};

			ParseResult parseTexture();
			ParseResult parseBuffer();
			ParseResult parsePushConstant();

			// advance a token and handle comments
			void advance();
			// advance a token if the next one is of the given type, ignoring comments
			bool advanceIfType(Token::Type type, bool shouldExpect = true);
			// advance a token if the next one is the given keyword, ignoring comments
			bool advanceIfKeyword(std::string_view keyword);
			bool advanceIfLineRemaining();

			bool expect(Token::Type type, const Token &tok);
			bool isKeyword(std::string_view keyword, const Token &tok);
			std::string makeLineInfo();

		private:
			std::string m_fileName;
			Token m_currentTok;
			ShaderInfo m_shaderData;
			uint32_t m_nextTextureBinding;
			uint32_t m_nextBufferBinding;
			uint32_t m_nextPushConstantBinding;
			std::unique_ptr<Tokenizer> m_tokenizer;
		};

	} // namespace ShaderParser

} // namespace Graphics
