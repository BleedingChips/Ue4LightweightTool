#pragma once
#include <vector>
#include <tuple>
#include <regex>
namespace Lexical
{
	template<typename Token>
	struct regex_token
	{
		regex_token(std::initializer_list<std::tuple<const wchar_t*, Token>> list)
		{
			m_tokens.reserve(list.size());
			for (auto& ite : list)
				m_tokens.push_back({ std::basic_regex<wchar_t>{std::get<0>(ite), std::regex::optimize }, std::get<1>(ite) });
		}
	private:
		//Implement::
		std::vector<std::tuple< std::basic_regex<wchar_t>, Token>> m_tokens;
		friend struct regex_token_wrapper;
	};

	struct string_view
	{
		const wchar_t* m_string;
		size_t m_count;
	};

	template<typename Token>
	struct token_result
	{
		token_result() : m_avalible(false) {}
		token_result(Token token) : m_token(token) {}
		token_result(const token_result&) = default;

		operator bool() const noexcept { return m_avalible; }
		Token operator*() const noexcept { return m_token; }
	private:
		bool m_avalible;
		Token m_token;
	};

	enum class ErrorState
	{
		None,
		OJBK = None,
		UnknowToken,
		TokenWithEmptyString,
	};

	struct regex_token_wrapper
	{
		regex_token_wrapper(const wchar_t* start, const wchar_t* end) noexcept : m_last_start(start), m_start(start), m_end(end) {}
		regex_token_wrapper(const wchar_t* start, size_t size) noexcept : m_last_start(start), m_start(start), m_end(start + size) {}
		template<typename T, typename K>
		regex_token_wrapper(const std::basic_string<wchar_t, T, K>& str) noexcept : regex_token_wrapper(str.data(), str.data() + str.size()) {}
		string_view string() const noexcept { return { m_last_start, static_cast<size_t>(m_start - m_last_start) }; }
		string_view last() const noexcept { return { m_start, static_cast<size_t>(m_end - m_start) }; }
		template<typename Token>
		token_result<Token> generate_token(const regex_token<Token>& regex)
		{
			if (m_start != m_end)
			{
				std::match_results<const wchar_t*> match;
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
		const wchar_t* m_last_start;
		const wchar_t* m_start;
		const wchar_t* m_end;
	};

	size_t translate_escape_sequence(const wchar_t* input, size_t input_count, wchar_t* output, size_t output_count) noexcept;
}