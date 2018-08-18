#pragma once
#include <string>
namespace Doc
{
	enum class Format
	{
		UTF8,
		UTF8_WITH_BOM,
		NOT_UNICIDE,
	};

	struct writer
	{
		writer(const wchar_t*, Format format = Format::UTF8);
		~writer();
		void write(const wchar_t*, size_t count);
		void write(const wchar_t*);
		template<typename T, typename K>
		void write(const std::basic_string<wchar_t, T, K>& string) { write(string.data(), string.size()); }
		bool is_open() const noexcept;
	public:
		void* implement;
	};
}