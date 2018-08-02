#pragma once
#include <vector>
#include <filesystem>
#include <string>
#include <functional>
#include <array>
#include <optional>
#include <regex>
namespace PO::Tool
{
	namespace Lexical
	{
		inline bool is_number(char16_t input) { return input >= u'0' && input <= u'9'; }
		inline bool is_lowercase(char16_t input) { return input >= u'a' && input <= u'z'; }
		inline bool is_uppercase(char16_t input) { return input >= u'A' && input <= u'Z'; }
		inline bool is_character(char16_t input) { return is_lowercase(input) || is_uppercase(input); }
	}

	enum class SpliterOpe
	{
		// push string, move cursor
		StorageAndSkip = 0,

		// ignore string, move cursor
		IgnoreAndSkip,

		// push token, move Cursor
		PushAndStop,

		// push token, move Cursor
		PushAndSkip,

		// push Storage And Skip
		PushStorageAndSkip,

		// error
		Error
	};

	struct lexical_analyzer
	{

		struct error : ::std::exception
		{
			std::u32string m_buffer;
			error(const ::std::u32string& bu) noexcept : m_buffer(bu) {}
			const char* what() const noexcept override
			{
				return "lexical meet error";
			}
		};

		uint64_t last_token() const noexcept { return m_token; }
		template<typename Enum_t>
		Enum_t cast_token() const noexcept { return static_cast<Enum_t>(m_token); }
		operator bool() const noexcept { return m_avalible; }
		bool generator_token();
		lexical_analyzer(::std::function<char32_t(bool&)> generator, ::std::function<SpliterOpe(uint64_t& inside_flag, uint64_t& output_flag, char32_t& input)> lexical_spliter);
		const ::std::u32string& buffer() const noexcept { return temporary_buffer; }

	private:

		::std::function<char32_t(bool&)> m_generator;
		::std::function<SpliterOpe(uint64_t& inside_flag, uint64_t& output_flag, char32_t& input)> m_lexical_spliter;
		uint64_t m_inside_flag;
		bool m_avalible;
		bool m_double_check;
		char32_t m_result;
		uint64_t m_token;
		::std::u32string temporary_buffer;
	};

	struct stream_wrapper
	{
		enum class State
		{
			Error,
			Finsh,
			Inprocess
		};

		enum class Cursor
		{
			Stop,
			IgnoreNext,
			Next,
		};

		struct result
		{
			State m_state;
			Cursor m_cursor;
			uint64_t m_token;
		};

		inline static result inprocess(Cursor cursor = Cursor::Next) { return result{State::Inprocess, cursor, 0}; }
		inline static result error() { return result{ State::Error, Cursor::Next, 0 }; }
		inline static result finish(uint64_t token, Cursor cursor = Cursor::Stop) {
			return result{ State::Finsh, cursor, token };
		}
		template<typename T> static result finish(T type, Cursor cursor = Cursor::Stop) {
			return finish(static_cast<uint64_t>(type), cursor);
		}

		struct unknow_lecical : std::exception
		{
			const char* what() const noexcept override { return "unknow lecical"; }
		};

		struct end_symbol_unhandled : std::exception
		{
			const char* what() const noexcept override { return "end symbol unhandled"; }
		};

		bool generator_token();

		stream_wrapper(
			std::function<char32_t(bool&)> generator,
			std::function<result(char32_t)> analyzer
		) noexcept;

		const std::u32string& buffer() const noexcept { return m_buffer; }
		uint64_t token() const noexcept { return m_token; }
		template<typename T> T cast_token() const noexcept { return static_cast<T>(m_token); }

	private:
		char32_t m_last_input;
		std::u32string m_buffer;
		uint64_t m_token;
		Cursor m_last_ope;
		std::function<char32_t(bool&)> m_generator;
		std::function<result(char32_t)> m_analyzer;
	};

	struct line_analyzer
	{
		enum class Token
		{
			Error,
			Line,
			LineBreakCR,
			LineBreakCRLF,
			LineBreakLF
		};
		stream_wrapper::result operator()(char32_t input) noexcept;
	private:
		uint64_t m_state = 0;
	};

	struct simple_lexical_spliter
	{
		enum class Token : uint64_t
		{
			Error = 0,
			Integer,
			Float,
			Ope,
			Id,
			Control,
			String,
			Note,
			Char
		};
		SpliterOpe operator()(uint64_t& inside_flag, uint64_t& outside_flag, char32_t& input);
	};


	enum class DefaultLexicalToken : uint64_t
	{
		Integer,
		Float,
		Ope,
		Id,
		Control,
		String,
		Note,
		Char
	};

	const char* to_utf8(DefaultLexicalToken input) noexcept;
	const char16_t* to_utf16(DefaultLexicalToken input) noexcept;

	SpliterOpe default_lexical_spliter(uint64_t& inside_flag, uint64_t& outside_flag, char32_t& input);
}

namespace PO::Tool::Lexical
{
	enum class State
	{
		Error,
		Finsh,
		Inprocess
	};

	enum class Cursor
	{
		Stop,
		IgnoreNext,
		Next,
	};

	struct result
	{
		State m_state;
		Cursor m_cursor;
		uint64_t m_token;
	};

	struct unknow_lecical : std::exception
	{
		const char* what() const noexcept override { return "unknow lecical"; }
	};

	struct end_symbol_unhandled : std::exception
	{
		const char* what() const noexcept override { return "end symbol unhandled"; }
	};

	inline result inprocess(Cursor cursor = Cursor::Next) { return result{ State::Inprocess, cursor, 0 }; }
	inline result error() { return result{ State::Error, Cursor::Next, 0 }; }
	inline result finish(uint64_t token, Cursor cursor = Cursor::Stop) {
		return result{ State::Finsh, cursor, token };
	}
	template<typename T> result finish(T type, Cursor cursor = Cursor::Stop) {
		return finish(static_cast<uint64_t>(type), cursor);
	}

	struct stream_analyzer_wrapper_utf16
	{
		bool generate_token();

		stream_analyzer_wrapper_utf16(
			std::function<size_t(char16_t*, size_t avalible)> generator,
			std::function<result(const char16_t*, size_t)> analyzer
		) noexcept;

		std::u16string string() const noexcept { return m_string; }
		uint64_t token() const noexcept { return m_token; }
		template<typename T> T cast_token() const noexcept { return static_cast<T>(m_token); }
		operator bool() const noexcept { return m_generator && m_analyzer; }
	private:
		bool m_avalible;
		char16_t m_last_input[2];
		size_t m_last_input_length;
		std::u16string m_string;
		uint64_t m_token;
		Cursor m_last_ope;
		std::function<size_t(char16_t*, size_t avalible)> m_generator;
		std::function<result(const char16_t*, size_t)> m_analyzer;
	};

	struct line_analyzer
	{
		enum class Token
		{
			Error,
			Empty,
			Line,
			LineBreakCR,
			LineBreakCRLF,
			LineBreakLF
		};
		result operator()(char32_t input) noexcept;
		result operator()(const char16_t* input, size_t length) noexcept;
	private:
		uint64_t m_state = 0;
	};

}
/*
namespace std
{
	template<> struct regex_traits<char16_t>
	{
		using char_type = char16_t;
		using string_type = std::u16string;
		using locale_type = std::locale;
		//using char_class_type = 
		int value(char16_t _Ch, int _Base) const {
			if ((_Base != 8 && u'0' <= _Ch && _Ch <= u'9')
				|| (_Base == 8 && u'0' <= _Ch && _Ch <= u'7'))
				return (_Ch - u'0');
			else if (_Base != 16)
				;
			else if (u'a' <= _Ch && _Ch <= u'f')
				return (_Ch - u'a' + 10);
			else if (u'A' <= _Ch && _Ch <= u'F')
				return (_Ch - u'A' + 10);
			return (-1);
		}
		static size_t length(const char16_t* input) { return std::char_traits<char16_t>::length(input); }
		char16_t translate(char16_t input) const { return input; }
		char16_t translate_nocase(char16_t input) const { 
			return std::char_traits<char16_t>::
		}
		template< class ForwardIt >
		string_type transform(ForwardIt first, ForwardIt last) const;
	private:
		regex_traits<wchar_t> m_implement;
	};
}
*/

namespace PO::Tool::Lexical
{
	// only avalible in windows
	struct regex_analyzer_wrapper_utf16
	{
		regex_analyzer_wrapper_utf16(
			std::initializer_list<std::tuple<const char16_t*, uint64_t>>
		);
		void set_string(const std::u16string& string);
		bool generate_token();
		std::u16string_view string() const noexcept { return m_view; }
		uint64_t token() const noexcept { return m_token; }
		template<typename T> T cast_token() const noexcept { return static_cast<T>(m_token); }
	private:
		std::u16string_view m_view;
		std::wstring m_string;
		std::wstring::const_iterator m_cursor;
		std::vector <std::tuple<std::wregex, uint64_t>> m_regex;
		uint64_t m_token;
		std::wsmatch m_match;
	};
}