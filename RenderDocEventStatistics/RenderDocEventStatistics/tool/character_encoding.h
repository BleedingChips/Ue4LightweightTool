#pragma once
#include <string>
namespace PO::Tool
{
	std::string utf32s_to_locals(const std::u32string& utf32s);
	std::u32string loacls_to_utf32s(const std::string& locals);
	std::string utf16s_to_loacls(const std::u16string& utf16);
	std::u16string locals_to_utf16s(const std::string& locals);

	size_t ascii_require_space(char da) noexcept;

	size_t utf8_require_space(char da);
	size_t utf16_require_space(char16_t da);

	size_t utf32_to_utf8_require_space(char32_t input);
	size_t utf32_to_utf16_require_space(char32_t input);
	
	size_t utf32_to_utf8(char32_t input, char* output_buffer, size_t output_size);
	size_t utf8_to_utf32(const char* input, size_t input_size, char32_t& output);
	
	size_t utf16_to_uft32(const char16_t* input, size_t input_size, char32_t& output);
	size_t utf32_to_utf16(char32_t input, char16_t* output, size_t output_size);

	std::pair<size_t, size_t> utf8_to_utf16(const char* input, size_t input_size, char16_t* output, size_t output_size);
	std::pair<size_t, size_t> utf16_to_utf8(const char16_t* input, size_t input_size, char* output, size_t avalible_buffer);
	std::pair<size_t, size_t> ascii_to_utf16(const char* input, size_t input_length, char16_t* output, size_t output_length) noexcept;
	std::pair<size_t, size_t> utf16_to_ascii(const char16_t* input, size_t input_length, char* output, size_t output_length) noexcept;

	std::pair<size_t, size_t> utf32s_to_utf8s(const char32_t* input, size_t input_size, char* output, size_t output_size) noexcept;
	std::pair<size_t, size_t> utf8s_to_utf32s(const char* input, size_t input_size, char32_t* output, size_t output_size) noexcept;
	std::pair<size_t, size_t> utf16s_to_utf8s(const char16_t* input, size_t input_size, char* output, size_t output_size) noexcept;
	std::pair<size_t, size_t> utf8s_to_utf16s(const char* input, size_t input_size, char16_t* output, size_t output_size) noexcept;
	std::pair<size_t, size_t> asciis_to_utf16s(const char* input, size_t input_length, char16_t* output, size_t output_length) noexcept;
	std::pair<size_t, size_t> utf16s_to_asciis(const char16_t* input, size_t input_length, char* output, size_t output_length) noexcept;

	std::string utf16s_to_utf8s(const std::u16string& utf16);
	std::u16string utf8s_to_utf16s(const std::string& utf16);
	std::u32string utf8s_to_utf32s(const std::string& input);
	std::string utf32s_to_utf8s(const std::u32string& input);
	
	//std::u16string wide_string_to_utf16s(const std::wstring& wide_string);
	//std::wstring utf16s_to_wide_string(const std::u16string& utf16s);
}