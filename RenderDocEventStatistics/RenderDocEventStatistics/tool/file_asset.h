#pragma once
#include "character_encoding.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <map>
namespace PO::Tool
{

	enum class FileCharFormat
	{
		UNKNOW,
		UTF8,
		UTF16LE,
		UTF16BE,
		UTF32BE,
		UTF32LE,
	};

	struct utf_file_reader
	{
		utf_file_reader(const std::experimental::filesystem::path& p);
		utf_file_reader(utf_file_reader&& uf);
		utf_file_reader() {}
		utf_file_reader& operator=(utf_file_reader&& uf);
		
		bool open(const std::experimental::filesystem::path& p);
		void close();

		//uint64_t last_utf8_space() const noexcept;
		bool is_open() const noexcept{return file.is_open();}
		bool eof() const noexcept{return file.eof();}
		void set_cursor_to_start() noexcept;
		char32_t read_one_utf32() noexcept { char32_t tem; read_all_utf32(&tem, 1); return tem;  }
		//std::string read_one_u8() noexcept { char32_t tem; read_all_u32(&tem, 1); return tem; }
		size_t read_all_utf32(char32_t* output, size_t avalible_count) noexcept;
		std::u32string read_all_utf32();
		size_t read_all_utf8(char* output, size_t avalible_count) noexcept;
		std::string read_all_utf8();
		FileCharFormat character_encoding() const noexcept { return format; }
		size_t estimation_utf8_count() noexcept;
		size_t estimation_utf32_count() noexcept;
		//size_t read_utf16_once(char* output, size_t avalible_count) noexcept;
	private:
		std::ifstream file;
		FileCharFormat format = FileCharFormat::UNKNOW;
	};

	struct utf_file_writer
	{
		utf_file_writer(const std::experimental::filesystem::path& p, FileCharFormat format = FileCharFormat::UTF8, bool append = false);
		utf_file_writer(utf_file_writer&& uf);
		utf_file_writer() {}
		utf_file_writer& operator=(utf_file_writer&& uf);
		bool open(const std::experimental::filesystem::path& p, FileCharFormat format = FileCharFormat::UTF8, bool append = false);
		void close();
		bool is_open() const noexcept{return file.is_open();}
		FileCharFormat character_encoding() const noexcept { return format; }
		void write(const char32_t*, size_t count) noexcept;
		void write(const std::u32string& input) noexcept { write(input.data(), input.size()); }
		void write(const char*, size_t count) noexcept;
		void write(const std::string& input) noexcept { write(input.data(), input.size()); }
	private:
		std::ofstream file;
		FileCharFormat format = FileCharFormat::UNKNOW;
	};

	struct binary_file_reader 
	{
		binary_file_reader(const std::experimental::filesystem::path& p);
		binary_file_reader(binary_file_reader&& uf);
		binary_file_reader() {}
		binary_file_reader& operator=(binary_file_reader&& uf);

		bool open(const std::experimental::filesystem::path& p);
		bool is_open() const noexcept { return m_file.is_open(); }
		void close();

		//uint64_t last_utf8_space() const noexcept;
		uint64_t total_space() const noexcept { return m_total_space; }
		uint64_t last_space() const noexcept { return m_last_space; }
		size_t read(void* buffer, size_t buffer_length) noexcept;
	private:
		std::ifstream m_file;
		uint64_t m_total_space = 0;
		uint64_t m_last_space = 0;
	};

	struct binary_file_writer 
	{
		binary_file_writer(const std::experimental::filesystem::path& p, bool append = false) { open(p, append); }
		binary_file_writer(binary_file_writer&& uf) = default;
		binary_file_writer() {}
		binary_file_writer& operator=(binary_file_writer&& uf) { m_file.operator=(std::move(uf.m_file)); return *this; }

		bool open(const std::experimental::filesystem::path& p, bool append = false) { 
			m_file.open(p, std::ios::binary | (append ? std::ios::app : 0));
			return m_file.is_open();
		}
		void close() { m_file.close(); }
		bool is_open() const noexcept { return m_file.is_open(); }

		//uint64_t last_utf8_space() const noexcept;
		void write(const void* buffer, size_t buffer_length) noexcept { m_file.write(reinterpret_cast<const char*>(buffer), buffer_length); }
	private:
		std::ofstream m_file;
	};


	template<typename input, typename = std::enable_if_t<std::is_pod_v<input>>>
	PO::Tool::binary_file_writer& operator<<(PO::Tool::binary_file_writer& o, const input& in)
	{
		o.write(&in, sizeof(in));
		return o;
	}

	template<typename output, typename = std::enable_if_t<std::is_pod_v<output>>>
	PO::Tool::binary_file_reader& operator>>(PO::Tool::binary_file_reader& o, output& out)
	{
		o.read(&out, sizeof(out));
		return o;
	}

	PO::Tool::binary_file_writer& operator<<(PO::Tool::binary_file_writer& o, const std::string& input);
	PO::Tool::binary_file_reader& operator>>(PO::Tool::binary_file_reader& o, std::string& input);
	PO::Tool::binary_file_writer& operator<<(PO::Tool::binary_file_writer& o, const std::u32string& input);
	PO::Tool::binary_file_reader& operator>>(PO::Tool::binary_file_reader& o, std::u32string& input);
	
	inline PO::Tool::binary_file_writer& operator<<(PO::Tool::binary_file_writer& o, const std::experimental::filesystem::path& input)
	{
		return o << input.u8string();
	}

	inline PO::Tool::binary_file_reader& operator>>(PO::Tool::binary_file_reader& o, std::experimental::filesystem::path& input)
	{
		std::string tem;
		o>>tem;
		input = tem;
		return o;
	}

	template<typename type, typename allocate>
	PO::Tool::binary_file_writer& operator<<(PO::Tool::binary_file_writer& o, const std::vector<type, allocate>& input)
	{
		uint64_t size = input.size();
		o.write(&size, sizeof(size));
		for (auto& ite : input)
			o << ite;
		return o;
	}
	

	template<typename type, typename allocate>
	PO::Tool::binary_file_reader& operator>>(PO::Tool::binary_file_reader& o, std::vector<type, allocate>& input)
	{
		uint64_t size;
		o.read(&size, sizeof(size));
		input.reserve(size);
		for (uint64_t i = 0; i < size; ++i)
		{
			type tem;
			o >> tem;
			input.push_back(std::move(tem));
		}
		return o;
	}

	template<typename type, typename type2, typename allocate>
	PO::Tool::binary_file_writer& operator<<(PO::Tool::binary_file_writer& o, const std::map<type, type2, allocate>& input)
	{
		uint64_t size = input.size();
		o.write(&size, sizeof(size));
		for (auto& ite : input)
			o << ite.first << ite.second;
		return o;
	}

	template<typename type, typename type2, typename allocate>
	PO::Tool::binary_file_reader& operator>>(PO::Tool::binary_file_reader& o, std::map<type, type2, allocate>& input)
	{
		uint64_t size;
		o >> size;
		for (uint64_t i = 0; i < size; ++i)
		{
			type _1;
			type2 _2;
			o >> _1 >> _2;
			input.insert({std::move(_1), std::move(_2)});
		}
		return o;
	}




}

namespace PO::Tool::Doc
{
	enum format
	{
		UNKNOW,
		UTF8,
		UTF16LE,
		UTF16BE,
		UTF32BE,
		UTF32LE,
	};

	struct loader_utf16
	{
		bool is_open() const noexcept { return m_file.is_open(); }
		bool end_of_file() const noexcept { return m_file.eof(); }
		void close() noexcept { m_file.close(); }
		size_t read_one(char16_t* output, size_t output_length) noexcept;
		loader_utf16(const std::filesystem::path& path) noexcept;
	private:
		size_t (*execute_function)(std::ifstream& file, char16_t*, size_t output_length) noexcept = nullptr;
		std::ifstream m_file;
		format m_format = format::UNKNOW;
	};
}

