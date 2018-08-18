#pragma once
#include <vector>
#include <string>
#include <functional>
#include <array>
#include <optional>
#include <regex>

namespace PO::Lexical
{

	enum class LineToken
	{
		Line,
		LineBreakCR,
		LineBreakCRLF,
		LineBreakLF
	};

	enum class ErrorState
	{
		None,
		OJBK = None,
		UnknowToken,
		TokenWithEmptyString,
	};

	template<typename CharT> struct line_spliter; 
	template<> struct line_spliter<char16_t>
	{
		template<typename Generator>
		std::optional<LineToken> genrate_token(Generator&& G)
		{
			m_string.clear();
			while (m_avalible)
			{
				if (!m_no_need_input)
					m_last_input_size = G(m_input, 2);
				else
					m_no_need_input = false;
				if (m_last_input_size != 0);
				else
					m_avalible = false;
				auto tok = this->generate_implement();
				if (tok)
					return *tok;
			}
			return {};
		}
		std::u16string_view string() const noexcept { return std::u16string_view{ m_string.data(), m_string.size() }; }
		void clear() noexcept;
	private:
		bool m_avalible = true;
		bool m_no_need_input = false;
		std::optional<LineToken> generate_implement() noexcept;
		uint32_t m_state = 0;
		std::u16string m_string;
		char16_t m_input[2];
		size_t m_last_input_size = 0;
	};

	template<typename CharT> struct trope_line_spliter;
	template<> struct trope_line_spliter<char16_t>
	{
		template<typename Generator>
		std::optional<LineToken> genrate_token(Generator&& G)
		{
			m_string.clear();
			while (m_avalible)
			{
				if (!m_no_need_input)
					m_last_input_size = G(m_input, 2);
				else
					m_no_need_input = false;
				if (m_last_input_size != 0);
				else
					m_avalible = false;
				std::optional<LineToken> tok = this->generate_implement();
				if (tok)
					return *tok;
			}
			return {};
		}
		std::u16string_view string() const noexcept { return std::u16string_view{ m_string.data(), m_string.size() }; }
		void clear() noexcept;
	private:
		bool m_avalible = true;
		bool m_no_need_input = false;
		std::optional<LineToken> generate_implement() noexcept;
		uint32_t m_state = 0;
		std::u16string m_string;
		char16_t m_input[2];
		size_t m_last_input_size = 0;
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

#ifdef _WIN32
	template<typename Token> 
	struct regex_token<Token, char16_t, std::regex_traits<char16_t>>
	{
		regex_token(std::initializer_list<std::tuple<const char16_t*, Token>> list)
		{
			m_tokens.reserve(list.size());
			for (auto& ite : list)
				m_tokens.push_back({ std::wregex{ reinterpret_cast<const wchar_t*>(std::get<0>(ite)), std::regex::optimize }, std::get<1>(ite) });
		}
	private:
		//Implement::
		std::vector<std::tuple<std::wregex, Token>> m_tokens;
		template<typename T> friend struct regex_token_wrapper;
	};
#endif

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

	const regex_token<EscapeSequence, wchar_t>& escape_sequence_cpp_wchar() noexcept;
	size_t translate(EscapeSequence, const wchar_t* input, size_t input_size, wchar_t* output, size_t output_size) noexcept;

	template<typename CharT>
	struct regex_token_wrapper
	{
		regex_token_wrapper(const CharT* start, const CharT* end) noexcept : m_last_start(start), m_start(start), m_end(end) {}
		regex_token_wrapper(const CharT* start, size_t size) noexcept : m_last_start(start), m_start(start), m_end(start + size) {}
		template<typename T, typename K>
		regex_token_wrapper(const std::basic_string<CharT, T, K>& str) noexcept : regex_token_wrapper(str.data(), str.data() + str.size()) {}
		std::basic_string_view<CharT> string() const noexcept { return { m_last_start, static_cast<typename std::basic_string_view<CharT>::size_type>(m_start - m_last_start) }; }
		std::basic_string_view<CharT> last() const noexcept { return { m_start, static_cast<typename std::basic_string_view<CharT>::size_type>(m_end - m_start) }; }
		template<typename Token, typename regex_traits>
		std::optional<Token> generate_token(const regex_token<Token, CharT, regex_traits>& regex)
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
							return std::get<1>(ite);
						else
							throw ErrorState::TokenWithEmptyString;
					}
				}
				throw ErrorState::UnknowToken;
			}
			return {};
		}
	private:
		const CharT* m_last_start;
		const CharT* m_start;
		const CharT* m_end;
	};

#ifdef _WIN32
	template<>
	struct regex_token_wrapper<char16_t>
	{
		regex_token_wrapper(const char16_t* start, const char16_t* end) noexcept : m_last_start(start), m_start(start), m_end(end) {}
		regex_token_wrapper(const char16_t* start, size_t  count) noexcept : m_last_start(start), m_start(start), m_end(start + count) {}
		template<typename T>
		regex_token_wrapper(const std::basic_string<char16_t, T>& str) noexcept : regex_token_wrapper(str.data(), str.data() + str.size()){}
		std::u16string_view string() const noexcept { return {m_last_start, static_cast<std::basic_string_view<char16_t,std::char_traits<char16_t>>::size_type>(m_start - m_last_start) }; }
		std::u16string_view last() const noexcept { return { m_start, static_cast<std::basic_string_view<char16_t,std::char_traits<char16_t>>::size_type>(m_end - m_start) }; }
		template<typename Token, typename regex_traits>
		std::optional<Token> generate_token(const regex_token<Token, char16_t, regex_traits>& regex)
		{
			if (m_start != m_end)
			{
				std::wcmatch match;
				for (auto& ite : regex.m_tokens)
				{
					if (std::regex_search(reinterpret_cast<const wchar_t*>(m_start), reinterpret_cast<const wchar_t*>(m_end), match, std::get<0>(ite), std::regex_constants::match_flag_type::match_continuous))
					{
						m_last_start = m_start;
						m_start = match[0].second;
						if (match[0].second != match[0].first)
							return std::get<1>(ite);
						else
							throw ErrorState::EmptyToken;
					}
				}
				throw ErrorState::UnknowToken;
			}
			return {};
		}
	private:
		const char16_t* m_last_start;
		const char16_t* m_start;
		const char16_t* m_end;
	};
#endif

}