#include "script_analyze.h"
#include <assert.h>
#include <string>
#include <iostream>

namespace PO::Tool
{
	/*
	lexical_analyzer::error::error(const std::u32string& u) noexcept
	{
		size_t buffer_size = u.size();
		if (buffer_size > 20)
		{
			std::memcpy(buffer.data(), u.c_str() + (buffer_size - 20) , 20);
			buffer[20] = U'\0';
		}
		else {
			std::memcpy(buffer.data(), u.c_str(), buffer_size);
			buffer[buffer_size] = U'\0';
		}
	}
	*/

	lexical_analyzer::lexical_analyzer(std::function<char32_t(bool&)> generator, 
		std::function<SpliterOpe(uint64_t& inside_flag, uint64_t& output_flag, char32_t& input)> lexical_spliter)
		: m_generator(std::move(generator)), m_lexical_spliter(std::move(lexical_spliter)), m_inside_flag(0), m_avalible(true), m_double_check(false), m_token(0)
	{
		assert(m_generator);
		assert(m_lexical_spliter);
	}


	bool lexical_analyzer::generator_token()
	{
		temporary_buffer.clear();
		while (m_avalible)
		{
			if (!m_double_check)
				m_result = m_generator(m_avalible);
			else
				m_double_check = false;
			auto spliter_result = m_lexical_spliter(m_inside_flag, m_token, m_result);
			switch (spliter_result)
			{
			case PO::Tool::SpliterOpe::StorageAndSkip:
				temporary_buffer += m_result;
				break;
			case PO::Tool::SpliterOpe::PushAndStop:
				m_double_check = true;
				return true;
			case PO::Tool::SpliterOpe::PushAndSkip:
				return true;
			case PO::Tool::SpliterOpe::IgnoreAndSkip:
				break;
			case PO::Tool::SpliterOpe::PushStorageAndSkip:
				temporary_buffer += m_result;
				return true;
			default:
				temporary_buffer += m_result;
				throw error{ temporary_buffer };
			}
		}
		return false;
	}

	stream_wrapper::stream_wrapper(
		std::function<char32_t(bool&)> generator,
		std::function<result(char32_t)> analyzer
	) noexcept : 
		m_generator(std::move(generator)), m_analyzer(std::move(m_analyzer)), m_last_ope(Cursor::Next), m_token(0)
	{}

	bool stream_wrapper::generator_token()
	{
		bool avalible = true;
		do {
			if (m_last_ope != Cursor::Stop)
			{
				m_last_input = m_generator(avalible);
				if (!avalible)
					m_last_input = 0;
			}
			result re = m_analyzer(m_last_input);
			m_last_ope = re.m_cursor;
			if (re.m_cursor == Cursor::Next)
				m_buffer += m_last_input;
			switch (re.m_state)
			{
			case State::Error:
				throw unknow_lecical{};
				break;
			case State::Inprocess:
				if (!avalible)
					throw end_symbol_unhandled{};
				break;
			case State::Finsh:
			{
				m_token = re.m_token;
				return true;
			}
			default:
				break;
			}
		} while (avalible);
		return true;
	}

	stream_wrapper::result line_analyzer::operator()(char32_t input) noexcept
	{
		enum State : uint64_t
		{
			Ready = 0,
			Line,
			LineBreakCR,
		};

		switch (m_state)
		{
		case State::Ready:
			if (input == U'\r') {
				m_state = State::LineBreakCR; 
				return stream_wrapper::inprocess();
			}
			else if (input == U'\n') {
				m_state = State::Ready; 
				return stream_wrapper::finish(Token::LineBreakLF, stream_wrapper::Cursor::Next);
			}
			else {
				m_state = State::Line; 
				return stream_wrapper::inprocess();
			}
			break;
		case State::LineBreakCR:
			if (input == U'\n'){
				m_state = State::Ready;
				return stream_wrapper::finish(Token::LineBreakCRLF, stream_wrapper::Cursor::Next);
			}
			else {
				return stream_wrapper::error();
			}
			break;
		case State::Line:
			if (input != U'\r' || input != U'\n') {
				return stream_wrapper::inprocess();
			}
			else {
				return stream_wrapper::finish(Token::Line, stream_wrapper::Cursor::Stop);
			}
		default:
			return stream_wrapper::error();
			break;
		}
	}

#define case_to_string(str) case DefaultLexicalToken::str: return #str;
	const char* to_utf8(DefaultLexicalToken input) noexcept
	{
		switch (input)
		{
			case_to_string(Integer);
			case_to_string(Float);
			case_to_string(Ope);
			case_to_string(Id);
			case_to_string(Control);
			case_to_string(String);
			case_to_string(Note);
			case_to_string(Char);
		default:
			return "unknow";
		}
	}
#undef case_to_string

#define case_to_string(str) case DefaultLexicalToken::str: return U## #str;
	const char32_t* to_utf32(DefaultLexicalToken input) noexcept
	{
		switch (input)
		{
			case_to_string(Integer);
			case_to_string(Float);
			case_to_string(Ope);
			case_to_string(Id);
			case_to_string(Control);
			case_to_string(String);
			case_to_string(Note);
			case_to_string(Char);
		default:
			return U"unknow";
		}
	}
#undef case_to_string

}










bool in_range(char32_t input, char32_t start, char32_t end)
{
	return input >= start && input <= end;
}



enum class DefaultLexialInsideFlag : uint64_t
{
	Empty = 0,
	Integer,
	Float,
	String,
	StringTranslate,
	Char,
	CharTranslate,
	CharEnd,
	BackSlant,
	NoteLine,
	NoteBlock,
	NoteBlockWait,
	Slant,
	Id,
	Ope,
	MulityOpe,
	Undecimal,
	Binary,
	Hexadecimal,
};

PO::Tool::SpliterOpe default_lexical_spliter_implement(DefaultLexialInsideFlag& inside_flag, PO::Tool::DefaultLexicalToken& outside_flag, char32_t& input)
{
	using namespace PO::Tool;
	switch (inside_flag)
	{
	case DefaultLexialInsideFlag::Empty:
		switch (input)
		{
		case U' ':
		case U'\0':
			return SpliterOpe::IgnoreAndSkip;
		case U'(': case U')': case U'{': case U'}': case U'[': case U']':case U';':case U'.':case U',':
			outside_flag = DefaultLexicalToken::Ope; return SpliterOpe::PushStorageAndSkip;
		case U'\t': case U'\n': case U'\r':
			outside_flag = DefaultLexicalToken::Control; return SpliterOpe::PushStorageAndSkip;
		case U'\'':
			inside_flag = DefaultLexialInsideFlag::Char; return SpliterOpe::IgnoreAndSkip;
		case U'\"':
			inside_flag = DefaultLexialInsideFlag::String; return SpliterOpe::IgnoreAndSkip;
		case U'/':
			inside_flag = DefaultLexialInsideFlag::BackSlant; return SpliterOpe::StorageAndSkip;
		default:
			if (input == U'0')
			{
				inside_flag = DefaultLexialInsideFlag::Undecimal;
				return SpliterOpe::StorageAndSkip;
			}else if (in_range(input, U'1', U'9')) { inside_flag = DefaultLexialInsideFlag::Integer; return SpliterOpe::StorageAndSkip; }
			else if (input == U'!' || in_range(input, U'#', U'&') || in_range(input, U'*', U'-') ||
				in_range(input, U':', U'@') || input == U'\\' || input == U'^' || input == U'~')
			{
				inside_flag = DefaultLexialInsideFlag::Ope; return SpliterOpe::StorageAndSkip;
			}
			else {
				inside_flag = DefaultLexialInsideFlag::Id; return SpliterOpe::StorageAndSkip;
			}
		}
	case DefaultLexialInsideFlag::Ope:
		if (input == U'!' || in_range(input, U'#', U'&') || in_range(input, U'*', U'-') ||
			in_range(input, U':', U'@') || input == U'\\' || input == U'^' || input == U'~')
		{
			return SpliterOpe::StorageAndSkip;
		}
		else {
			inside_flag = DefaultLexialInsideFlag::Empty;
			outside_flag = DefaultLexicalToken::Ope;
			return SpliterOpe::PushAndStop;
		}
	case DefaultLexialInsideFlag::BackSlant:
		switch (input)
		{
		case U'*':
			inside_flag = DefaultLexialInsideFlag::NoteBlock; return SpliterOpe::StorageAndSkip;
		case U'/':
			inside_flag = DefaultLexialInsideFlag::NoteLine; return SpliterOpe::StorageAndSkip;
		default:
			if (input == U'!' || in_range(input, U'#', U'&') || in_range(input, U'*', U'-') ||
				in_range(input, U':', U'@') || input == U'\\' || input == U'^' || input == U'~')
			{
				inside_flag = DefaultLexialInsideFlag::MulityOpe; return SpliterOpe::StorageAndSkip;
			}
			else {
				inside_flag = DefaultLexialInsideFlag::Empty; outside_flag = DefaultLexicalToken::Ope; return SpliterOpe::PushAndStop;
			}
		}
	case DefaultLexialInsideFlag::NoteLine:
		if (input != U'\r' && input != U'\n')
		{
			return SpliterOpe::StorageAndSkip;
		}
		inside_flag = DefaultLexialInsideFlag::Empty;
		outside_flag = DefaultLexicalToken::Note;
		return SpliterOpe::PushAndSkip;
	case DefaultLexialInsideFlag::NoteBlock:
		if (input != U'*')
			return SpliterOpe::StorageAndSkip;
		inside_flag = DefaultLexialInsideFlag::NoteBlockWait;
		return SpliterOpe::StorageAndSkip;
	case DefaultLexialInsideFlag::NoteBlockWait:
		if (input != U'/')
		{
			inside_flag = DefaultLexialInsideFlag::NoteBlock;
			return SpliterOpe::StorageAndSkip;
		}
		else {
			inside_flag = DefaultLexialInsideFlag::Empty;
			outside_flag = DefaultLexicalToken::Note;
			return SpliterOpe::PushStorageAndSkip;
		}
	case DefaultLexialInsideFlag::Char:
		if (input != U'\'')
		{
			if (input == U'\\')
				inside_flag = DefaultLexialInsideFlag::CharTranslate;
			else
				inside_flag = DefaultLexialInsideFlag::CharEnd;
			return SpliterOpe::StorageAndSkip;
		}
	case DefaultLexialInsideFlag::CharTranslate:
		inside_flag = DefaultLexialInsideFlag::CharEnd;
		return SpliterOpe::StorageAndSkip;
	case DefaultLexialInsideFlag::CharEnd:
		if (input == U'\'')
		{
			inside_flag = DefaultLexialInsideFlag::Empty;
			outside_flag = DefaultLexicalToken::Char;
			return SpliterOpe::IgnoreAndSkip;
		}
	case DefaultLexialInsideFlag::String:
		if (input != U'\"')
		{
			if (input == U'\\')
				inside_flag = DefaultLexialInsideFlag::StringTranslate;
			return SpliterOpe::StorageAndSkip;
		}
		else {
			inside_flag = DefaultLexialInsideFlag::Empty;
			outside_flag = DefaultLexicalToken::String;
			return SpliterOpe::PushAndSkip;
		}
	case DefaultLexialInsideFlag::StringTranslate:
		inside_flag = DefaultLexialInsideFlag::String;
		return SpliterOpe::StorageAndSkip;
	case DefaultLexialInsideFlag::Integer:
		if (in_range(input, U'0', U'9'))
			return SpliterOpe::StorageAndSkip;
		else if (input == U'.')
		{
			inside_flag = DefaultLexialInsideFlag::Float;
			return SpliterOpe::StorageAndSkip;
		}
		else if (in_range(input, U'a', U'z') || in_range(input, U'A', U'Z') || input == '_' || input > 0xFF)
		{
			return SpliterOpe::Error;
		}
		else {
			inside_flag = DefaultLexialInsideFlag::Empty;
			outside_flag = DefaultLexicalToken::Integer;
			return SpliterOpe::PushAndStop;
		}
	case DefaultLexialInsideFlag::Float:
		if (in_range(input, U'0', U'9'))
			return SpliterOpe::StorageAndSkip;
		else if (input == U'.' || in_range(input, U'a', U'z') || in_range(input, U'A', U'Z') || input == U'_' || input > 0xFF)
			return SpliterOpe::Error;
		else {
			inside_flag = DefaultLexialInsideFlag::Empty;
			outside_flag = DefaultLexicalToken::Float;
			return SpliterOpe::PushAndSkip;
		}
	case DefaultLexialInsideFlag::Id:
		if (in_range(input, U'0', U'9') || in_range(input, U'a', U'z') || in_range(input, U'A', U'Z') || input == U'_' || input > 0xFF)
			return SpliterOpe::StorageAndSkip;
		else {
			inside_flag = DefaultLexialInsideFlag::Empty;
			outside_flag = DefaultLexicalToken::Id;
			return SpliterOpe::PushAndStop;
		}
	case DefaultLexialInsideFlag::Undecimal:
		if (input == U'b' || input == U'B')
		{
			inside_flag = DefaultLexialInsideFlag::Binary;
			return SpliterOpe::StorageAndSkip;
		}
		else if (input == U'x' || input == U'X')
		{
			inside_flag = DefaultLexialInsideFlag::Hexadecimal;
			return SpliterOpe::StorageAndSkip;
		}
		else if (input == U'.')
		{
			inside_flag = DefaultLexialInsideFlag::Float;
			return SpliterOpe::StorageAndSkip;
		}
		else if (in_range(input, U'0', U'9'))
		{
			inside_flag = DefaultLexialInsideFlag::Integer;
			return SpliterOpe::StorageAndSkip;
		}
		else if (in_range(input, U'a', U'z') || in_range(input, U'A', U'Z') || input == U'_' || input > 0xFF)
		{
			return SpliterOpe::Error;
		}
		else {
			inside_flag = DefaultLexialInsideFlag::Empty;
			outside_flag = DefaultLexicalToken::Integer;
			return SpliterOpe::PushAndStop;
		}
	case DefaultLexialInsideFlag::Binary:
		if (input == U'0' || input == U'1')
			return SpliterOpe::StorageAndSkip;
		else if (input == U'.' || in_range(input, U'a', U'z') || in_range(input, U'A', U'Z') || input == U'_' || input > 0xFF || in_range(input, U'2', U'9'))
			return SpliterOpe::Error;
		else {
			inside_flag = DefaultLexialInsideFlag::Empty;
			outside_flag = DefaultLexicalToken::Integer;
			return SpliterOpe::PushAndStop;
		}
	case DefaultLexialInsideFlag::Hexadecimal:
		if (in_range(input, U'a', U'f') || in_range(input, U'A', U'F') || in_range(input, U'0', U'9'))
			return SpliterOpe::StorageAndSkip;
		else if (input == U'.' || input == U'_' || input > 0xFF)
			return SpliterOpe::Error;
		else {
			inside_flag = DefaultLexialInsideFlag::Empty;
			outside_flag = DefaultLexicalToken::Integer;
			return SpliterOpe::PushAndStop;
		}
	}
	return SpliterOpe::Error;
}

namespace PO::Tool
{
	SpliterOpe default_lexical_spliter(uint64_t& inside_flag, uint64_t& outside_flag, char32_t& input)
	{
		DefaultLexialInsideFlag inside = static_cast<DefaultLexialInsideFlag>(inside_flag);
		DefaultLexicalToken outside = static_cast<DefaultLexicalToken>(outside_flag);
		auto result = default_lexical_spliter_implement(inside, outside, input);
		inside_flag = static_cast<uint64_t>(inside);
		outside_flag = static_cast<uint64_t>(outside);
		return result;
	}
}



namespace PO::Tool::Lexical
{
	stream_analyzer_wrapper_utf16::stream_analyzer_wrapper_utf16(
		std::function<size_t(char16_t*, size_t)> generator,
		std::function<result(const char16_t*, size_t)> analyzer
	) noexcept :
		m_generator(std::move(generator)), m_analyzer(std::move(analyzer)), m_last_ope(Cursor::Next), 
		m_token(0), m_last_input_length(0), m_avalible(true)
	{}

	bool stream_analyzer_wrapper_utf16::generate_token()
	{
		assert(m_generator);
		assert(m_analyzer);
		m_string.clear();
		while(m_avalible)
		{
			if (m_last_ope != Cursor::Stop)
			{
				m_last_input_length = m_generator(m_last_input, 2);
				if (m_last_input_length != 0);
				else {
					m_avalible = false;
					m_last_input_length = 1;
					m_last_input[0] = 0;
				}
			}
			result re = m_analyzer(m_last_input, m_last_input_length);
			m_last_ope = re.m_cursor;
			if (re.m_cursor == Cursor::Next)
				m_string.append(m_last_input, m_last_input_length);
			switch (re.m_state)
			{
			case State::Error:
				throw unknow_lecical{};
				break;
			case State::Inprocess:
				if (!m_avalible)
					throw end_symbol_unhandled{};
				break;
			case State::Finsh:
			{
				m_token = re.m_token;
				return true;
			}
			default:
				break;
			}
		};
		return false;
	}

	result line_analyzer::operator()(char32_t input) noexcept
	{
		enum State : uint64_t
		{
			Ready = 0,
			Line,
			LineBreakCR,
		};

		switch (m_state)
		{
		case State::Ready:
			if (input == U'\r') {
				m_state = State::LineBreakCR;
				return inprocess();
			}
			else if (input == U'\n') {
				m_state = State::Ready;
				return finish(Token::LineBreakLF, Cursor::Next);
			}
			else {
				m_state = State::Line;
				return inprocess();
			}
			break;
		case State::LineBreakCR:
			if (input == U'\n') {
				m_state = State::Ready;
				return finish(Token::LineBreakCRLF, Cursor::Next);
			}
			else {
				return error();
			}
			break;
		case State::Line:
			if (input != U'\r' || input != U'\n') {
				return inprocess();
			}
			else {
				return finish(Token::Line, Cursor::Stop);
			}
		default:
			return error();
			break;
		}
	}

	result line_analyzer::operator()(const char16_t* input, size_t length) noexcept
	{
		enum State : uint64_t
		{
			Ready = 0,
			Line,
			LineBreakCR,
		};

		switch (m_state)
		{
		case State::Ready:
			if (input[0] == u'\r') {
				m_state = State::LineBreakCR;
				return inprocess();
			}
			else if (input[0] == u'\n') {
				m_state = State::Ready;
				return finish(Token::LineBreakLF, Cursor::Next);
			}
			else if (input[0] == 0){
				return finish(Token::Empty, Cursor::Next);
			}
			else {
				m_state = State::Line;
				return inprocess();
			}
			break;
		case State::LineBreakCR:
			if (input[0] == u'\n') {
				m_state = State::Ready;
				return finish(Token::LineBreakCRLF, Cursor::Next);
			}
			else {
				return error();
			}
			break;
		case State::Line:
			if (input[0] == 0)
			{
				m_state = State::Ready;
				return finish(Token::Line, Cursor::Next);
			}else if (input[0] != u'\r' && input[0] != u'\n') {
				return inprocess();
			}
			else {
				m_state = State::Ready;
				return finish(Token::Line, Cursor::Stop);
			}
		default:
			return error();
			break;
		}
	}

	regex_analyzer_wrapper_utf16::regex_analyzer_wrapper_utf16(
		std::initializer_list<std::tuple<const char16_t*, uint64_t>> il
	) : m_token(0), m_cursor(m_string.cend())
	{
		m_regex.reserve(il.size());
		for (auto& ite : il)
			m_regex.push_back({ std::wregex(reinterpret_cast<const wchar_t*>(std::get<0>(ite)), std::regex::optimize), std::get<1>(ite) });
	}

	void regex_analyzer_wrapper_utf16::set_string(const std::u16string& string)
	{
		m_string = std::wstring(reinterpret_cast<const wchar_t*>(string.data()), string.size());
		m_cursor = m_string.cbegin();
	}

	bool regex_analyzer_wrapper_utf16::generate_token()
	{
		assert(!m_regex.empty());
		if (m_cursor != m_string.cend())
		{
			for (auto& ite : m_regex)
			{
				std::wsmatch match;
				if (std::regex_search(m_cursor, m_string.cend(), match, std::get<0>(ite), std::regex_constants::match_flag_type::match_continuous))
				{
					if (match[0].second == match[0].first)
						throw empty_lexical{};
					m_cursor = match[0].second;
					m_view = std::u16string_view(reinterpret_cast<const char16_t*>(&(*match[0].first)), match[0].second - match[0].first);
					m_token = std::get<1>(ite);
					return true;
				}
			}
			throw unknow_lecical{};
		}
		return false;
	}

}