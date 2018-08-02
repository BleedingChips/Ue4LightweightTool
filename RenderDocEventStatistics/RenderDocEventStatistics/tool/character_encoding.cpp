#include "character_encoding.h"
#include <assert.h>
#ifdef _WIN32
#include <windows.h>
#include <vector>

#ifdef _UNICODE
#endif

namespace PO::Tool
{
	std::string utf16s_to_loacls(const std::u16string& data)
	{
		static thread_local std::vector<char>  translate_char_buffer;
		if (data.size() * 2 + 1 > translate_char_buffer.size())
			translate_char_buffer.resize(data.size() * 2 + 1);
		HRESULT re = WideCharToMultiByte(CP_ACP, 0, (wchar_t*)(data.c_str()), -1, &translate_char_buffer[0], static_cast<int>(translate_char_buffer.size()), NULL, NULL);
		if (!SUCCEEDED(re))
		{
			__debugbreak();
		}
		std::string tem(&translate_char_buffer[0]);
		return tem;
	}

	std::u16string locals_to_utf16s(const std::string& data)
	{
		static thread_local std::vector<wchar_t>  translate_char_buffer;
		if (data.size() + 1> translate_char_buffer.size())
			translate_char_buffer.resize(data.size() + 1);
		MultiByteToWideChar(CP_ACP, 0, data.c_str(), -1, &translate_char_buffer[0], static_cast<int>(translate_char_buffer.size()));
		return std::u16string(reinterpret_cast<char16_t*>(&translate_char_buffer[0]));
	}
}
#endif

namespace PO::Tool
{
	size_t ascii_require_space(char da) noexcept
	{
		return static_cast<uint8_t>(da) > 127 ? 2 : 1;
	}

	std::pair<size_t, size_t> ascii_to_utf16(const char* input, size_t input_length, char16_t* output, size_t output_length) noexcept
	{
		if (input_length != 0)
		{
			size_t require_space = ascii_require_space(input[0]);
			
			if (input_length >= require_space)
			{
				size_t result = MultiByteToWideChar(CP_ACP, 0, input, require_space, reinterpret_cast<wchar_t*>(output), output_length);
				return { require_space, result };
			}
		}
		return { 0, 0 };
	}

	std::pair<size_t, size_t> utf16_to_ascii(const char16_t* input, size_t input_length, char* output, size_t output_length) noexcept
	{
		if (input_length != 0)
		{
			size_t require_space = utf16_require_space(input[0]);
			BOOL unchangleble = true;
			if (input_length >= require_space)
			{
				size_t result = WideCharToMultiByte(CP_ACP, 0, reinterpret_cast<const wchar_t*>(input), require_space, output, output_length, "?", &unchangleble);
				//size_t result = MultiByteToWideChar(CP_ACP, 0, input, require_space, reinterpret_cast<wchar_t*>(output), output_length);
				return { require_space, result };
			}
		}
		return { 0, 0 };
	}

	size_t utf8_require_space(char da)
	{
		if ((da & 0xFE) == 0xFC)
			return 6;
		else if ((da & 0xFC) == 0xF8)
			return 5;
		else if ((da & 0xF8) == 0xF0)
			return 4;
		else if ((da & 0xF0) == 0xE0)
			return 3;
		else if ((da & 0xE0) == 0xC0)
			return 2;
		else if ((da & 0x80) == 0)
			return 1;
		else
			return 1;
	}

	size_t utf16_require_space(char16_t da)
	{
		if (da >= 0xD800 && da <= 0xDBFF)
			return 2;
		return 1;
	}

	size_t utf32_to_utf8_require_space(char32_t input)
	{
		if ((input & 0xFFFFFF80) == 0)
			return 1;
		else if ((input & 0xFFFF'F800) == 0)
			return 2;
		else if ((input & 0xFFFF'0000) == 0)
			return 3;
		else if ((input & 0xFFE0'0000) == 0)
			return 4;
		else if ((input & 0xFC00'0000) == 0)
			return 5;
		else if ((input & 0x8000'0000) == 0)
			return 6;
		else
			return 0;
	}

	size_t utf32_to_utf16_require_space(char32_t input)
	{
		if (input >= 0x10000 && input <= 0x10FFFF)
			return 2;
		else
			return 1;
	}

	size_t utf32_to_utf8(char32_t input, char* buffer, size_t output_size)
	{
		size_t require_count = utf32_to_utf8_require_space(input);
		if (output_size >= require_count)
		{
			switch (require_count)
			{
			case 1:
				*buffer = static_cast<char>(input & 0x0000007F);
				break;
			case 2:
				*buffer = 0xC0 | static_cast<char>((input & 0x07C0) >> 6);
				*(buffer + 1) = 0x80 | static_cast<char>((input & 0x3F));
				break;
			case 3:
				*buffer = 0xE0 | static_cast<char>((input & 0xF000) >> 12);
				*(buffer + 1) = 0x80 | static_cast<char>((input & 0xFC0) >> 6);
				*(buffer + 2) = 0x80 | static_cast<char>((input & 0x3F));
				break;
			case 4:
				*buffer = 0x1E | static_cast<char>((input & 0x1C0000) >> 18);
				*(buffer + 1) = 0x80 | static_cast<char>((input & 0x3F000) >> 12);
				*(buffer + 2) = 0x80 | static_cast<char>((input & 0xFC0) >> 6);
				*(buffer + 3) = 0x80 | static_cast<char>((input & 0x3F));
				break;
			case 5:
				*buffer = 0xF8 | static_cast<char>((input & 0x300'0000) >> 24);
				*(buffer + 1) = 0x80 | static_cast<char>((input & 0xFC'0000) >> 18);
				*(buffer + 2) = 0x80 | static_cast<char>((input & 0x3'F000) >> 12);
				*(buffer + 3) = 0x80 | static_cast<char>((input & 0xFC0) >> 6);
				*(buffer + 4) = 0x80 | static_cast<char>((input & 0x3F));
				break;
			case 6:
				*buffer = 0xFC | static_cast<char>((input & 0x4000'0000) >> 30);
				*(buffer + 1) = 0x80 | static_cast<char>((input & 0x3F00'0000) >> 24);
				*(buffer + 2) = 0x80 | static_cast<char>((input & 0xFC'0000) >> 18);
				*(buffer + 3) = 0x80 | static_cast<char>((input & 0x3'F000) >> 12);
				*(buffer + 4) = 0x80 | static_cast<char>((input & 0xFC0) >> 6);
				*(buffer + 5) = 0x80 | static_cast<char>((input & 0x3F));
				break;
			}
			return require_count;
		}
		return 0;
	}

	size_t utf8_to_utf32(const char* input, size_t input_size, char32_t& output)
	{
		if (input_size != 0)
		{
			size_t require_space = utf8_require_space(input[0]);
			if (require_space <= input_size)
			{
				switch (require_space)
				{
				case 1: output = input[0]; break;
				case 2: output = ((input[0] & 0x1F) << 6) | (input[1] & 0x3F); break;
				case 3: output = ((input[0] & 0x0F) << 12) | ((input[1] & 0x3F) << 6) | (input[2] & 0x3F); break;
				case 4: output = ((input[0] & 0x07) << 18) | ((input[1] & 0x3F) << 12) | ((input[2] & 0x3F) << 6) | (input[3] & 0x3F); break;
				case 5: output = ((input[0] & 0x03) << 24) | ((input[1] & 0x3F) << 18) | ((input[2] & 0x3F) << 12) | ((input[3] & 0x3F) << 6) | (input[4] & 0x3F); break;
				case 6: output = ((input[0] & 0x01) << 30) | ((input[1] & 0x3F) << 24) | ((input[2] & 0x3F) << 18) | ((input[3] & 0x3F) << 12) | ((input[4] & 0x3F) << 6) | (input[5] & 0x3F); break;
				}
				return require_space;
			}
		}
		return 0;
	}

	size_t utf16_to_uft32(const char16_t* input, size_t input_size, char32_t& output)
	{
		if (input_size != 0)
		{
			size_t require_space = utf16_require_space(input[0]);
			if (input_size >= require_space)
			{
				switch (require_space)
				{
				case 1: output = *input; break;
				case 2: output = (((input[0] & 0x3FF) << 10) | (input[1] & 0x3FF)) + 0x10000; break;
				}
				return require_space;
			}
			
		}
		return 0;
	}

	size_t utf32_to_utf16(char32_t input, char16_t* buffer, size_t output_size)
	{
		size_t require_space = utf32_to_utf16_require_space(input);
		if (output_size >= require_space)
		{
			switch (require_space)
			{
			case 1: buffer[0] = static_cast<char16_t>(input); break;
			case 2: 
				char32_t tem = input - 0x10000;
				buffer[0] = ((tem & 0xFFC00) >> 10) & 0xD800;
				buffer[0] = (tem & 0x3FF) & 0xDC00;
				break;
			}
			return require_space;
		}
		return 0;
	}

	std::pair<size_t, size_t> utf8_to_utf16(const char* input, size_t input_size, char16_t* output, size_t output_size)
	{
		size_t _1 = 0;
		size_t _2 = 0;
		if (input_size != 0)
		{
			char32_t tem;
			_1 = utf8_to_utf32(input, input_size, tem);
			if(_1 != 0)
				_2 = utf32_to_utf16(tem, output, output_size);
		}
		return { _1, _2 };
	}

	std::pair<size_t, size_t> utf16_to_utf8(const char16_t* input, size_t input_size, char* output, size_t output_size)
	{
		size_t _1 = 0;
		size_t _2 = 0;
		if (input != nullptr && input_size != 0)
		{
			char32_t tem;
			_1 = utf16_to_uft32(input, input_size, tem);
			if (_1 != 0)
				_2 = utf32_to_utf8(tem, output, output_size);
		}
		return { _1, _2 };
	}

	std::pair<size_t, size_t> utf16s_to_asciis(const char16_t* input, size_t input_length, char* output, size_t output_length) noexcept
	{
		size_t utf16_index = 0;
		size_t ascii_index = 0;
		while (utf16_index < input_length)
		{
			auto result = utf16_to_ascii(input + utf16_index, input_length - utf16_index, output + ascii_index, output_length - ascii_index);
			if (result.second != 0)
			{
				utf16_index += result.first;
				ascii_index += result.second;
			}
			else
				break;
		}
		return { utf16_index,  ascii_index };
	}
	std::pair<size_t, size_t> utf16s_to_asciis(const char16_t* input, size_t input_length, char* output, size_t output_length) noexcept;

	std::pair<size_t, size_t> utf32s_to_utf8s(const char32_t* input, size_t input_size, char* output, size_t output_size) noexcept
	{
		size_t index = 0;
		size_t utf8_used = 0;
		if (input_size >= 0)
		{
			for (size_t index = 0; index < input_size; ++index)
			{
				size_t used = utf32_to_utf8(input[index], output + utf8_used, output_size);
				if (used != 0)
				{
					output_size -= used;
					utf8_used += used;
				}
				else
					break;
			}
		}
		return { index, utf8_used };
	}

	std::pair<size_t, size_t> utf8s_to_utf32s(const char* input, size_t input_size, char32_t* output, size_t output_size) noexcept
	{
		size_t input_index = 0;
		size_t utf32_index = 0;
		if (input_size != 0)
		{
			for (; input_index < input_size && output_size > 0;)
			{
				size_t used = utf8_to_utf32(input + input_index, input_size, output[utf32_index]);
				if (used != 0)
				{
					input_index += used;
					++utf32_index;
				}
				else
					break;
			}
		}
		return { input_index, utf32_index };
	}

	std::pair<size_t, size_t> utf16s_to_utf8s(const char16_t* input, size_t input_size, char* output, size_t output_size) noexcept
	{
		size_t utf16_index = 0;
		size_t utf8_index = 0;
		if (input_size != 0)
		{
			for (; utf16_index < input_size;)
			{
				auto pair = utf16_to_utf8(input + utf16_index, input_size, output + utf8_index, output_size);
				if (pair.first != 0 && pair.second != 0)
				{
					utf16_index += pair.first;
					utf8_index += pair.second;
					output_size -= pair.second;
				}
				else
					break;
			}
		}
		return { utf16_index, utf8_index };
	}

	std::pair<size_t, size_t> utf8s_to_utf16s(const char* input, size_t input_size, char16_t* output, size_t output_size) noexcept
	{
		size_t utf16_index = 0;
		size_t utf8_index = 0;
		if (input_size != 0)
		{
			while (utf8_index < input_size)
			{
				auto pair = utf8_to_utf16(input + utf8_index, input_size, output + utf16_index, output_size);
				if (pair.first != 0 && pair.second != 0)
				{
					utf8_index += pair.first;
					utf16_index += pair.second;
					output_size -= pair.second;
				}
				else
					break;
			}
		}
		return { utf8_index, utf16_index };
	}

	
	std::u32string utf8s_to_utf32s(const std::string& input)
	{
		std::u32string temporary(input.size(), U'\0');
		auto pair = utf8s_to_utf32s(input.data(), input.size(), temporary.data(), temporary.size());
		temporary.resize(pair.second);
		return temporary;
	}

	std::string utf32s_to_utf8s(const std::u32string& input)
	{
		std::string temporary(input.size() * 6, u'\0');
		auto pair = utf32s_to_utf8s(input.data(), input.size(), temporary.data(), temporary.size());
		temporary.resize(pair.second);
		return temporary;
	}

	std::string utf16s_to_utf8s(const std::u16string& input)
	{
		std::string temporary(input.size() * 4, u'\0');
		auto pair = utf16s_to_utf8s(input.data(), input.size(), temporary.data(), temporary.size());
		temporary.resize(pair.second);
		return temporary;
	}

	std::u16string utf8s_to_utf16s(const std::string& input)
	{
		std::u16string temporary(input.size(), u'\0');
		auto pair = utf8s_to_utf16s(input.data(), input.size(), temporary.data(), temporary.size());
		temporary.resize(pair.second);
		return temporary;
	}
}