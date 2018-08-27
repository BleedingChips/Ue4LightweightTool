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

	namespace Implement
	{

		template<typename CharT> struct line_traits_implement;
		template<>
		struct line_traits_implement<char>
		{
			static bool is_carriage_return(const char* input, size_t count) { return input[0] == '\r'; }
			static bool is_line_feed(const char* input, size_t count) { return input[0] == '\n'; }
			static bool is_back_slash(const char* input, size_t count) { return input[0] == '\\'; }
			static std::pair<const char*, size_t> back_slash() noexcept { return { "\\", 1 }; }
		};
		template<>
		struct line_traits_implement<char16_t>
		{
			static bool is_carriage_return(const char16_t* input, size_t count) { return input[0] == u'\r'; }
			static bool is_line_feed(const char16_t* input, size_t count) { return input[0] == u'\n'; }
			static bool is_back_slash(const char16_t* input, size_t count) { return input[0] == u'\\'; }
			static std::pair<const char16_t*, size_t> back_slash() noexcept { return { u"\\", 1 }; }
		};
		template<>
		struct line_traits_implement<char32_t>
		{
			static bool is_carriage_return(const char32_t* input, size_t count) { return input[0] == U'\r'; }
			static bool is_line_feed(const char32_t* input, size_t count) { return input[0] == U'\n'; }
			static bool is_back_slash(const char32_t* input, size_t count) { return input[0] == U'\\'; }
			static std::pair<const char32_t*, size_t> back_slash() noexcept { return { U"\\", 1 }; }
		};
		template<>
		struct line_traits_implement<wchar_t>
		{
			static bool is_carriage_return(const wchar_t* input, size_t count) { return input[0] == L'\r'; }
			static bool is_line_feed(const wchar_t* input, size_t count) { return input[0] == L'\n'; }
			static bool is_back_slash(const wchar_t* input, size_t count) { return input[0] == L'\\'; }
			static std::pair<const wchar_t*, size_t> back_slash() noexcept { return { L"\\", 1 }; }
		};

		template<typename CharT> struct one_charactor_storage;
		template<> struct one_charactor_storage<char>
		{
			operator char*() noexcept { return m_storage; }
			operator const char*() const noexcept { return m_storage; }
			size_t size() const noexcept { return 6; }
		private:
			char m_storage[6];
		};
		template<> struct one_charactor_storage<char16_t>
		{
			operator char16_t*() noexcept { return m_storage; }
			operator const char16_t*() const noexcept { return m_storage; }
			size_t size() const noexcept { return 2; }
		private:
			char16_t m_storage[2];
		};
		template<> struct one_charactor_storage<char32_t>
		{
			operator char32_t*() noexcept { return &m_storage; }
			operator const char32_t*() const noexcept { return &m_storage; }
			size_t size() const noexcept { return 1; }
		private:
			char32_t m_storage;
		};
		template<> struct one_charactor_storage<wchar_t>
		{
			operator wchar_t*() noexcept { return m_storage; }
			operator const wchar_t*() const noexcept { return m_storage; }
#ifdef _WIN32
			size_t size() const noexcept { return 2; }
		private:
			wchar_t m_storage[2];
#else
			size_t size() const noexcept { return 1; }
		private:
			wchar_t m_storage;
#endif
		};

		template<typename CharT>
		struct basic_string_wrapper
		{
			template<typename T, typename A>
			static void append(std::basic_string<CharT, T, A>& string, const CharT* input, size_t input_count)
			{
				string.append(input, input_count);
			}
			template<typename T, typename A> static void clear(std::basic_string<CharT, T, A>& string)
			{
				string.clear();
			}
		};

		template<typename CharT> struct line_spliter_implement
		{
			template<typename StringWrapper, typename Wrapper, typename StringT> std::tuple<std::optional<LineToken>, bool> generate_implement(
				StringT& sting, const CharT* input, size_t input_count)
			{
				switch (m_state)
				{
				case 0:
					if (input_count != 0)
					{
						bool is_carriage_return = Wrapper::is_carriage_return(input, input_count);
						bool is_line_feed = Wrapper::is_line_feed(input, input_count);
						if (!is_carriage_return && !is_line_feed)
							StringWrapper::append(sting, input, input_count);
						else if (is_line_feed)
							return { LineToken::LineBreakLF, true };
						else
							m_state = 1;
					}
					else
					{
						return { LineToken::Line, true };
					}
					break;
				case 1:
					m_state = 0;
					if (input_count != 0)
					{
						if (!Wrapper::is_line_feed(input, input_count))
						{
							return { LineToken::LineBreakCR, false };
						}else
							return { LineToken::LineBreakCRLF, true };
					}
					return { LineToken::LineBreakCR, true };
					break;
				}
				return { {}, true };
			}
		private:
			uint32_t m_state = 0;
		};

		template<typename CharT> struct escape_line_spliter_implement
		{
		private:
			enum class TropLineSpliterState
			{
				Empty = 0,
				CRStart,
				TropeStart,
				TropeStartCR,
			};
			TropLineSpliterState m_state = TropLineSpliterState::Empty;
		public:
			template<typename StringWrapper, typename Wrapper, typename StringT> std::tuple<std::optional<LineToken>, bool> generate_implement(
				StringT& sting, const CharT* input, size_t input_count)
			{
				switch (m_state)
				{
				case TropLineSpliterState::Empty:
					if (input_count != 0)
					{
						bool is_carriage_return = Wrapper::is_carriage_return(input, input_count);
						bool is_line_feed = Wrapper::is_line_feed(input, input_count);
						bool is_back_slash = Wrapper::is_back_slash(input, input_count);
						if (!is_carriage_return && !is_line_feed && !is_back_slash)
							StringWrapper::append(sting, input, input_count);
						else if (is_line_feed)
							return { LineToken::LineBreakLF, true };
						else if (is_carriage_return)
							m_state = TropLineSpliterState::CRStart;
						else
							m_state = TropLineSpliterState::TropeStart;
					}
					else
					{
						return { LineToken::Line, true };
					}
					break;
				case TropLineSpliterState::CRStart:
					m_state = TropLineSpliterState::Empty;
					if (input_count != 0)
					{
						if (!Wrapper::is_line_feed(input, input_count))
						{
							return { LineToken::LineBreakCR, false };
						}
						else
							return { LineToken::LineBreakCRLF, true };
					}
					return { LineToken::LineBreakCR, true };
				case TropLineSpliterState::TropeStart:
					if (input_count != 0)
					{
						bool is_carriage_return = Wrapper::is_carriage_return(input, input_count);
						bool is_line_feed = Wrapper::is_line_feed(input, input_count);
						if (!is_carriage_return && !is_line_feed)
						{
							auto back_slash = Wrapper::back_slash();
							StringWrapper::append(sting, back_slash.first, back_slash.second);
							m_state = TropLineSpliterState::Empty;
							return { {}, false };
						}
						else if (is_carriage_return)
							m_state = TropLineSpliterState::TropeStartCR;
						else if (is_line_feed)
							m_state = TropLineSpliterState::Empty;
						break;
					}
					else {
						auto back_slash = Wrapper::back_slash();
						StringWrapper::append(sting, back_slash.first, back_slash.second);
						m_state = TropLineSpliterState::Empty;
						return { LineToken::Line, false };
					}
					break;
				case TropLineSpliterState::TropeStartCR:
					m_state = TropLineSpliterState::Empty;
					if (input_count != 0)
					{
						if (!Wrapper::is_line_feed(input, input_count))
							return { {}, false };
					}
					else {
						return { LineToken::LineBreakCR, true };
					}
					break;
				}
				return { {}, true };
			}
		};
		
	}

	template<typename CharT, typename LineTraits = Implement::line_traits_implement<CharT>, typename StorageT = Implement::one_charactor_storage<CharT>,
		typename StringT = std::basic_string<CharT>, typename StringWrapper = Implement::basic_string_wrapper<CharT>, 
		typename ImplementT = Implement::line_spliter_implement<CharT>>
		struct line_spliter
	{
		// Generator -> size_t(CharT*,size_t) , Handler -> void(LineToken, StringT&);
		template<typename Generator, typename Handler> bool generate(Generator&& G, Handler&& H)
		{
			StringT string;
			StorageT storage;
			size_t m_input_count = G(static_cast<CharT*>(storage), storage.size());
			ImplementT imp;
			if (m_input_count != 0)
			{
				while (true)
				{
					auto result = imp.generate_implement<StringWrapper, LineTraits>(string, static_cast<const CharT*>(storage), m_input_count);
					if (std::get<0>(result))
					{
						H(*std::get<0>(result), string);
						StringWrapper::clear(string);
					}
					if (m_input_count != 0)
					{
						if(std::get<1>(result))
							m_input_count = G(static_cast<CharT*>(storage), storage.size());
					}
					else
						break;
				}
				return true;
			}
			return false;
		}
	};

	template<typename CharT, typename LineTraits = Implement::line_traits_implement<CharT>, typename StorageT = Implement::one_charactor_storage<CharT>,
		typename StringT = std::basic_string<CharT>, typename StringWrapper = Implement::basic_string_wrapper<CharT>>
	using escape_line_spliter = line_spliter<CharT, LineTraits, StorageT, StringT, StringWrapper, Implement::escape_line_spliter_implement<CharT>>;\

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
		friend struct regex_token_wrapper;
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
		friend struct regex_token_wrapper;
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

	struct regex_token_wrapper
	{
		// Handler -> bool(Token, const CharT*, size_t) , UnTokenHandler -> bool(const Char*, size_t);
		template<typename CharT, typename Token, typename Traits, typename Handler, typename UnTokenHandler>
		static bool generate(const CharT* input, size_t input_count, const regex_token<Token, CharT, Traits>& regex, Handler&& H, UnTokenHandler&& UH) noexcept
		{
			if (input_count > 0)
			{
				while (true)
				{
					std::match_results<const CharT*> match;
					bool is_match = false;
					for (auto& ite : regex.m_tokens)
					{
						if (std::regex_search(input, input + input_count, match, std::get<0>(ite), std::regex_constants::match_flag_type::match_continuous))
						{
							size_t count = static_cast<size_t>(match[0].second - match[0].first);
							bool need_continue = false;
							if (H(std::get<1>(ite), match[0].first, count) && count < input_count)
							{
								input += count;
								input_count -= count;
								is_match = true;
								break;
							}
							else
								return true;
						}
					}
					if (is_match || UH(input, input_count));
					else
						break;
				}
				return true;
			}
			return false;
		}

		// Handler -> bool(Token, Ite, Ite) , UnTokenHandler -> bool(Ite, Ite);
		template<typename CharT, typename Ite, typename Token, typename Traits, typename Handler, typename UnTokenHandler>
		bool generate(Ite begin, Ite end, const regex_token<Token, CharT, Traits>& regex, Handler&& H, UnTokenHandler&& UH) noexcept
		{
			if (begin != end)
			{
				while (true)
				{
					std::match_results<Ite> match;
					bool is_match = false;
					for (auto& ite : regex.m_tokens)
					{
						if (std::regex_search(begin, end, match, std::get<0>(ite), std::regex_constants::match_flag_type::match_continuous))
						{
							auto b = match[0].first;
							auto e = match[0].second;
							if (H(std::get<1>(ite), b, e) && e != end)
							{
								begin = e;
								is_match = true;
								break;
							}
							else
								return false;
						}
					}
					if (is_match || UH(begin, end));
					else
						break;
				}
				return true;
			}
			return false;
		}

	};

}

namespace PO::Syntax 
{
	//template<typename SyntaxToken, typename LexicalToSyntaxTokenWrapper, >
	//LR1
}