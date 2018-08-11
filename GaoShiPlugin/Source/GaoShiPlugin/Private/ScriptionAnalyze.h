#pragma once
#include <vector>
#include <string>
#include <functional>
#include <array>
#include <regex>

namespace Lexical
{

	enum class ErrorState
	{
		OJBK,
		None = OJBK,
		UnknowToken,
		EmptyToken
	};

	template<typename Token, typename CharT, typename regex_traits = std::regex_traits<CharT>>
	struct regex_token
	{
		regex_token(std::initializer_list<std::tuple<const CharT*, Token>> list)
		{
			m_tokens.reserve(list.size());
			for (auto& ite : list)
				m_tokens.push_back({ std::basic_regex<CharT, regex_traits>{std::get<0>(ite), std::regex::optimize }, std::get<1>(ite) });
		}
	private:
		//Implement::
		std::vector<std::tuple< std::basic_regex<CharT, regex_traits>, Token>> m_tokens;
		template<typename T> friend struct regex_token_wrapper;
	};

	enum class EscapeSequence
	{
		SingleQuote,
		BackSlash,
		CarriageReturn,
		HorizontalTab,
		DoubleQuote,
		QuestionMark,
		AudibleBell,
		BackSpace,
		FormFeed,
		LineFeed,
		VerticalTab,
		ArbitraryOctalValue,
		ArbitraryHexadecimalValue,
		UniversalCharacterName,
		NormalString,
	};

	template<typename Token>
	struct UglyOptional
	{
		bool m_have;
		Token m_token;
		operator bool() const noexcept { return m_have; }
		operator Token () const noexcept { return m_token; }
		Token operator*() const noexcept { return m_token; }
		bool operator==(Token t) const noexcept { return m_token == t; }
		UglyOptional() : m_have(false) {}
		UglyOptional(Token t) : m_have(true), m_token(t) {}
	};

	const regex_token<EscapeSequence, wchar_t>& escape_sequence_cpp_wchar() noexcept;
	size_t translate(EscapeSequence, const wchar_t* input, size_t input_size, wchar_t* output, size_t output_size);

	template<typename CharT>
	struct regex_token_wrapper
	{
		regex_token_wrapper(const CharT* start, const CharT* end) noexcept : m_last_start(start), m_start(start), m_end(end) {}
		regex_token_wrapper(const CharT* start, size_t size) noexcept : m_last_start(start), m_start(start), m_end(start + size) {}
		template<typename T, typename K>
		regex_token_wrapper(const std::basic_string<CharT, T, K>& str) noexcept : regex_token_wrapper(str.data(), str.data() + str.size()) {}
		std::tuple<const CharT*, size_t> string() const noexcept { return { m_last_start, m_start - m_last_start }; }
		std::tuple<const CharT*, size_t> last() const noexcept { return { m_start, m_end - m_start }; }
		template<typename Token, typename regex_traits>
		UglyOptional<Token> generate_token(const regex_token<Token, CharT, regex_traits>& regex, ErrorState& Es) noexcept
		{
			if (m_start != m_end)
			{
				std::match_results<const CharT*> match;
				for (auto& ite : regex.m_tokens)
				{
					if (std::regex_search(m_start, m_end, match, std::get<0>(ite), std::regex_constants::match_flag_type::match_continuous))
					{
						m_last_start = m_start;
						m_start = match[0].second;
						if (match[0].second != match[0].first)
							Es = ErrorState::None;
						else
							Es = ErrorState::EmptyToken;
						return std::get<1>(ite);
					}
				}
				Es = ErrorState::UnknowToken;
				return {};
			}
			return {};
		}
	private:
		const CharT* m_last_start;
		const CharT* m_start;
		const CharT* m_end;
	};
}