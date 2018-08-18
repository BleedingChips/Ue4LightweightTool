#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include <map>

namespace PO {
	namespace Doc
	{
		enum class Format : uint32_t
		{
			UTF8 = 0,
			UTF8_WITH_BOM,
			UTF16LE,
			UTF16BE,
			UTF32BE,
			UTF32LE,
			NOT_UNICODE,
		};

		enum class ErrorState
		{
			OK = 0,
			WrongFormat,
		};

		struct result
		{
			operator size_t() const noexcept { return size; }
			operator bool() const noexcept { return state == ErrorState::OK; }
			size_t size;
			ErrorState state;
		};

		namespace Implement
		{
			struct loader_base
			{
				loader_base() noexcept;
				loader_base(loader_base&& lb) noexcept;
				loader_base(const std::filesystem::path& path, Format default_format = Format::UTF8) noexcept;
				loader_base& operator=(loader_base&&) noexcept;

				bool is_open() const noexcept { return m_file.is_open(); }
				bool end_of_file() const noexcept { return m_file.eof(); }
				Format format() const noexcept { return m_format; }
				void seek_begin() noexcept;
				void close() noexcept;

			protected:
				std::ifstream m_file;
				Format m_format;
			};

			struct loader_utf16 : loader_base
			{
				result read_one(char16_t* output, size_t output_length) noexcept;
				loader_utf16() noexcept = default;
				loader_utf16(const std::filesystem::path& path, Format default_format = Format::UTF8) noexcept;
				loader_utf16(loader_utf16&& lu) noexcept : loader_base(std::move(lu)), execute_function(lu.execute_function) {};
				loader_utf16& operator=(loader_utf16&&) noexcept;
				void reset_format(Format f) noexcept;
			protected:
				size_t(*execute_function)(std::ifstream& file, char16_t*, size_t output_length) noexcept = nullptr;
			};

			struct loader_utf32 : loader_base
			{
				result read_one(char32_t* output, size_t output_length = 1) noexcept;
				//loader_utf16(const std::filesystem::path& path) noexcept;
				loader_utf32(const std::filesystem::path& path, Format default_format = Format::UTF8) noexcept;
				void reset_format(Format f) noexcept;
				loader_utf32& operator=(loader_utf32&&) noexcept;
				loader_utf32(loader_utf32&& lu) noexcept : loader_base(std::move(lu)), execute_function(lu.execute_function) {}
			private:
				size_t(*execute_function)(std::ifstream& file, char32_t*) noexcept = nullptr;
			};


			struct loader_wchar :
#ifdef _WIN32
				protected loader_utf16
			{
			private:
				using upper = loader_utf16;
			public:
				result read_one(wchar_t* output, size_t output_length) noexcept { return loader_utf16::read_one(reinterpret_cast<char16_t*>(output), output_length); }
#else
				protected loader_utf32
			{
			private:
				using upper = loader_utf32;
			public:
				result read_one(wchar_t* output, size_t output_length = 1) noexcept { return loader_utf32::read_one(reinterpret_cast<char32_t*>(output), output_length); }
	#endif
				bool is_open() const noexcept { return upper::is_open(); }
				bool end_of_file() const noexcept { return upper::end_of_file(); }
				Format format() const noexcept { return upper::format(); }
				void seek_begin() noexcept { upper::seek_begin(); }
				loader_wchar(const std::filesystem::path& path, Format default_format = Format::UTF8) noexcept : upper(path, default_format) {}
				void close() noexcept { upper::close(); }
				loader_wchar() noexcept = default;
				loader_wchar(loader_wchar&& lw) noexcept = default;
				loader_wchar& operator=(loader_wchar&&) noexcept = default;
			};

			template<typename T> struct loader_picker;
			template<> struct loader_picker<wchar_t> { using type = loader_wchar; };
			template<> struct loader_picker<char16_t> { using type = loader_utf16; };
			template<> struct loader_picker<char32_t> { using type = loader_utf32; };
		}

		template<typename T> using loader = typename Implement::loader_picker<T>::type;

		namespace Implement
		{
			struct writer_base
			{
				writer_base(const std::filesystem::path&, Format format = Format::UTF8) noexcept;
				writer_base() noexcept;
				writer_base(writer_base&& eb) noexcept;
				writer_base& operator=(writer_base&& wb) noexcept;
				void close() noexcept { m_file.close(); }
				bool is_open() const noexcept { return m_file.is_open(); }
				Format format() const noexcept { return m_format; }

			protected:

				Format m_format;
				std::ofstream m_file;
			};

			struct writer_utf16 : writer_base
			{
				writer_utf16(const std::filesystem::path& path, Format format = Format::UTF8) noexcept;
				writer_utf16() noexcept = default;
				writer_utf16(writer_utf16&& wu) noexcept : writer_base(std::move(wu)), execute_function(wu.execute_function) {}
				writer_utf16& operator=(writer_utf16&& wu) noexcept;
				void write(const char16_t*, size_t);
				void write(const char16_t* input) { write(input, std::char_traits<char16_t>::length(input)); }
				template<typename T, typename P> void write(const std::basic_string<char16_t, T, P>& p) { write(p.data(), p.size()); }
			protected:
				void(*execute_function)(std::ofstream& o, const char16_t* input, size_t length) = nullptr;
			};

			struct writer_utf32 : writer_base
			{
				writer_utf32(const std::filesystem::path& path, Format format = Format::UTF8) noexcept;
				writer_utf32() noexcept = default;
				writer_utf32(writer_utf32&& wu) noexcept : writer_base(std::move(wu)), execute_function(wu.execute_function) {}
				void write(const char32_t* input, size_t input_length);
				void write(const char32_t* input) { write(input, std::char_traits<char32_t>::length(input)); }
			protected:
				void(*execute_function)(std::ofstream& o, const char32_t* input, size_t length) = nullptr;
			};

			struct writer_wchar :
#ifdef _WIN32
				protected writer_utf16
			{
			private:
				using upper = writer_utf16;
			public:
				void write(const wchar_t* input, size_t input_length) { writer_utf16::write(reinterpret_cast<const char16_t*>(input), input_length); }
#else
				protected writer_utf32
			{
			private:
				using upper = writer_utf32;
			public:
				void write(const wchar_t* input, size_t input_length) { writer_utf32::write(reinterpret_cast<const char32_t*>(input), input_length); }
	#endif
				void write(const wchar_t* input) { write(input, std::char_traits<wchar_t>::length(input)); }
				bool is_open() const noexcept { return upper::is_open(); }
				Format format() const noexcept { return upper::format(); }
				void close() noexcept { upper::close(); }
				writer_wchar(const std::filesystem::path& path, Format default_format = Format::UTF8) noexcept : upper(path, default_format) {}
				writer_wchar() noexcept = default;
				writer_wchar(writer_wchar&& lw) noexcept = default;
				writer_wchar& operator=(writer_wchar&&) noexcept = default;
			};

			template<typename T> struct writer_picker;
			template<> struct writer_picker<wchar_t> { using type = writer_wchar; };
			template<> struct writer_picker<char16_t> { using type = writer_utf16; };
			template<> struct writer_picker<char32_t> { using type = writer_utf32; };
		}

		template<typename T> using writer = typename Implement::writer_picker<T>::type;

	}
}

