#include "file_asset.h"
#include "character_encoding.h"
#include <assert.h>

constexpr uint32_t utf8_filter = 0x00FF'FFFF;
constexpr uint32_t utf8_bom = 0x00BF'BBEF;
constexpr uint32_t utf32_be_bom = 0xFEFF'0000;
constexpr uint32_t utf32_le_bom = 0x0000'FFFE;
constexpr uint32_t utf16_filter = 0x0000'FFFF;
constexpr uint32_t utf16_be_bom = 0x0000'FFFE;
constexpr uint32_t utf16_le_bom = 0x0000'FEFF;

PO::Tool::FileCharFormat translate_bom(uint32_t bom)
{
	using namespace PO::Tool;
	if ((bom & utf8_filter) == utf8_bom)
		return FileCharFormat::UTF8;
	else if (bom == utf32_be_bom)
		return FileCharFormat::UTF32BE;
	else if (bom == utf32_le_bom)
		return FileCharFormat::UTF32LE;
	else if ((bom & utf16_filter) == utf16_le_bom)
		return FileCharFormat::UTF16LE;
	else if ((bom & utf16_filter) == utf16_be_bom)
		return FileCharFormat::UTF16BE;
	else
		return FileCharFormat::UNKNOW;
}

void write_bom_binary(std::ofstream& output, PO::Tool::FileCharFormat format)
{
	using namespace PO::Tool;
	switch (format)
	{
	case FileCharFormat::UTF8:
	{
		unsigned char bom[] = { 0xef, 0xbb, 0xbf};
		output.write(reinterpret_cast<char*>(&bom), 3);
		break;
	}
	case FileCharFormat::UTF16LE:
	{
		unsigned char bom[] = { 0xfe, 0xff };
		output.write(reinterpret_cast<char*>(&bom), 2);
		break;
	}
	case FileCharFormat::UTF16BE:
	{
		unsigned char bom[] = { 0xff, 0xfe };
		output.write(reinterpret_cast<char*>(&bom), 2);
		break;
	}	
	case FileCharFormat::UTF32LE:
	{
		unsigned char bom[] = { 0x00, 0x00, 0xfe, 0xff };
		output.write(reinterpret_cast<char*>(&bom), 4);
		break;
	}
	case FileCharFormat::UTF32BE:
	{
		unsigned char bom[] = { 0xfe, 0xff, 0x00, 0x00 };
		output.write(reinterpret_cast<char*>(&bom), 4);
		break;
	}
	}
}

size_t calculate_bom_space(PO::Tool::FileCharFormat format)
{
	switch (format)
	{
	case PO::Tool::FileCharFormat::UTF8:
		return 3;
	case PO::Tool::FileCharFormat::UTF16LE:
	case PO::Tool::FileCharFormat::UTF16BE:
		return 2;
	case PO::Tool::FileCharFormat::UTF32BE:
	case PO::Tool::FileCharFormat::UTF32LE:
		return 4;
	default:
		return 0;
	}
}

size_t load_utf32LE_to_utf32_implement(std::ifstream& file, char32_t& output) noexcept
{
	file.read(reinterpret_cast<char*>(output), sizeof(char32_t));
	return file.eof() ? 0 : 1;
}

size_t load_utf32BE_to_utf32_implement(std::ifstream& file, char32_t& output) noexcept
{
	char buffer[4];
	file.read(buffer, sizeof(char32_t));
	std::swap(buffer[0], buffer[3]);
	std::swap(buffer[1], buffer[2]);
	output = *reinterpret_cast<char32_t*>(buffer);
	return file.eof() ? 0 : 1;
}

size_t load_utf16LE_to_utf16_implement(std::ifstream& file, char16_t* output, size_t avalible) noexcept
{
	if (avalible != 0)
	{
		file.read(reinterpret_cast<char*>(output), sizeof(char16_t));
		if (!file.eof())
		{
			size_t required = PO::Tool::utf16_require_space(output[0]);
			if (required <= avalible)
			{
				if (required == 2)
					file.read(reinterpret_cast<char*>(output + 1), sizeof(char16_t));
				return required;
			}
			else {
				file.seekg(-static_cast<int64_t>(sizeof(char16_t)), std::ios::cur);
			}
		}
		
	}
	return 0;
}

size_t load_utf16BE_to_utf16_implement(std::ifstream& file, char16_t* output, size_t avalible) noexcept
{
	if (avalible != 0)
	{
		auto ptr = reinterpret_cast<char*>(output);
		file.read(ptr, sizeof(char16_t));
		if (!file.eof())
		{
			std::swap(ptr[0], ptr[1]);
			size_t require_space = PO::Tool::utf16_require_space(output[0]);
			if (require_space == 2)
			{
				file.read(ptr + 2, sizeof(char16_t));
				std::swap(ptr[2], ptr[3]);
			}
			return require_space;
		}
	}
	return 0;
}

size_t load_utf8_to_utf8_implement(std::ifstream& file, char* output, size_t avalible) noexcept
{
	if (avalible != 0)
	{
		file.read(output, 1);
		if (!file.eof())
		{
			size_t require_space = PO::Tool::utf8_require_space(output[0]);
			if (require_space <= avalible)
			{
				file.read(output + 1, require_space - 1);
				return require_space;
			}
			else {
				file.seekg(-static_cast<int64_t>(sizeof(char)), std::ios::cur);
			}
		}
	}
	return 0;
}

bool load_utf8_to_utf32(std::ifstream& file, char32_t& output) noexcept
{
	char buffer[6];
	if (load_utf8_to_utf8_implement(file, buffer, 6) != 0)
	{
		PO::Tool::utf8_to_utf32(buffer, 6, output);
		return true;
	}
	return false;
}
bool load_utf32LE_to_utf32(std::ifstream& file, char32_t& output) noexcept
{
	return (load_utf32LE_to_utf32_implement(file, output) == 0) ? false : true;
}
bool load_utf32BE_to_utf32(std::ifstream& file, char32_t& output) noexcept
{
	return (load_utf32BE_to_utf32_implement(file, output) == 0) ? false : true;
}
bool load_utf16LE_to_utf32(std::ifstream& file, char32_t& output) noexcept
{
	char16_t buffer[2];
	if (load_utf16LE_to_utf16_implement(file, buffer, 2) != 0)
	{
		PO::Tool::utf16_to_uft32(buffer, 2, output);
		return true;
	}
	return false;
}
bool load_utf16BE_to_utf32(std::ifstream& file, char32_t& output) noexcept
{
	char16_t buffer[2];
	if (load_utf16BE_to_utf16_implement(file, buffer, 2) != 0)
	{
		PO::Tool::utf16_to_uft32(buffer, 2, output);
		return true;
	}
	return false;
}

const std::map<PO::Tool::FileCharFormat, bool(*)(std::ifstream& file, char32_t& output) noexcept> to_utf32_map = 
{
{ PO::Tool::FileCharFormat::UTF16LE, load_utf16LE_to_utf32 },
{ PO::Tool::FileCharFormat::UTF16BE, load_utf16BE_to_utf32 },
{ PO::Tool::FileCharFormat::UTF32LE, load_utf32LE_to_utf32 },
{ PO::Tool::FileCharFormat::UTF32BE, load_utf32BE_to_utf32 },
{ PO::Tool::FileCharFormat::UTF8, load_utf8_to_utf32 },
{ PO::Tool::FileCharFormat::UNKNOW, load_utf8_to_utf32 },
};

size_t load_utf8_to_utf8(std::ifstream& file, char* output, size_t avalible) noexcept
{
	return load_utf8_to_utf8_implement(file, output, avalible);
}
size_t load_utf32LE_to_utf8(std::ifstream& file, char* output, size_t avalible) noexcept
{
	if (avalible != 0)
	{
		char32_t tem;
		if (load_utf32LE_to_utf32(file, tem) != 0)
		{
			size_t used = PO::Tool::utf32_to_utf8(tem, output, avalible);
			if (used == 0)
				file.seekg(-static_cast<int64_t>(sizeof(char32_t)), std::ios::cur);
			return used;
		}
	}
	return 0;
}
size_t load_utf32BE_to_utf8(std::ifstream& file, char* output, size_t avalible) noexcept
{
	if (avalible != 0)
	{
		char32_t tem;
		if (load_utf32BE_to_utf32(file, tem) != 0)
		{
			size_t used = PO::Tool::utf32_to_utf8(tem, output, avalible);
			if (used == 0)
				file.seekg(-static_cast<int64_t>(sizeof(char32_t)), std::ios::cur);
			return used;
		}
	}
	return 0;
}
size_t load_utf16LE_to_utf8(std::ifstream& file, char* output, size_t avalible) noexcept
{
	if (avalible != 0)
	{
		char16_t buffer[2];
		size_t used_16 = load_utf16LE_to_utf16_implement(file, buffer, 2);
		if (used_16 != 0)
		{
			size_t used_8;
			std::tie(std::ignore, used_8) = PO::Tool::utf16_to_utf8(buffer, 2, output, avalible);
			if (used_8 == 0)
				file.seekg(used_16  * -static_cast<int64_t>(sizeof(char16_t)), std::ios::cur);
			return used_8;
		}
	}
	return 0;
}
size_t load_utf16BE_to_utf8(std::ifstream& file, char* output, size_t avalible) noexcept
{
	if(avalible != 0)
	{
		char16_t buffer[2];
		size_t used_16 = load_utf16BE_to_utf16_implement(file, buffer, 2);
		if (used_16 != 0)
		{
			size_t used_8;
			std::tie(std::ignore, used_8) = PO::Tool::utf16_to_utf8(buffer,2, output, avalible);
			if (used_8 == 0)
				file.seekg(used_16  * -static_cast<int64_t>(sizeof(char16_t)), std::ios::cur);
			return used_8;
		}
	}
	return 0;
}

const std::map<PO::Tool::FileCharFormat, size_t (*)(std::ifstream& file, char* output, size_t avalible) noexcept> to_utf8_map =
{
{ PO::Tool::FileCharFormat::UTF16LE, load_utf16LE_to_utf8 },
{ PO::Tool::FileCharFormat::UTF16BE, load_utf16BE_to_utf8 },
{ PO::Tool::FileCharFormat::UTF32LE, load_utf32LE_to_utf8 },
{ PO::Tool::FileCharFormat::UTF32BE, load_utf32BE_to_utf8 },
{ PO::Tool::FileCharFormat::UTF8, load_utf8_to_utf8 },
{ PO::Tool::FileCharFormat::UNKNOW, load_utf8_to_utf8 },
};

size_t calculate_binary_space(std::ifstream& file) noexcept
{
	auto poi = file.tellg();
	file.seekg(0, std::ios::end);
	auto poi2 = file.tellg();
	file.seekg(poi);
	return static_cast<size_t>(poi2 - poi);
}

size_t estimation_count_utf16_to_utf32(std::ifstream& file) noexcept
{
	return calculate_binary_space(file) / 2;
}

size_t estimation_count_utf32_to_utf32(std::ifstream& file) noexcept
{
	return calculate_binary_space(file) / 4;
}

const std::map<PO::Tool::FileCharFormat, size_t(*)(std::ifstream& file) noexcept> estimation_count_to_utf32 =
{
{ PO::Tool::FileCharFormat::UTF16LE, estimation_count_utf16_to_utf32 },
{ PO::Tool::FileCharFormat::UTF16BE, estimation_count_utf16_to_utf32 },
{ PO::Tool::FileCharFormat::UTF32LE, estimation_count_utf32_to_utf32 },
{ PO::Tool::FileCharFormat::UTF32BE, estimation_count_utf32_to_utf32 },
{ PO::Tool::FileCharFormat::UTF8, calculate_binary_space },
{ PO::Tool::FileCharFormat::UNKNOW, calculate_binary_space },
};

size_t estimation_count_utf32_to_utf8(std::ifstream& file) noexcept
{
	return calculate_binary_space(file) / 2 * 3;
}

size_t estimation_count_utf16_to_utf8(std::ifstream& file) noexcept
{
	return calculate_binary_space(file) * 2;
}

const std::map<PO::Tool::FileCharFormat, size_t(*)(std::ifstream& file) noexcept> estimation_count_to_utf8 =
{
{ PO::Tool::FileCharFormat::UTF16LE, estimation_count_utf16_to_utf8 },
{ PO::Tool::FileCharFormat::UTF16BE, estimation_count_utf16_to_utf8 },
{ PO::Tool::FileCharFormat::UTF32LE, estimation_count_utf32_to_utf8 },
{ PO::Tool::FileCharFormat::UTF32BE, estimation_count_utf32_to_utf8 },
{ PO::Tool::FileCharFormat::UTF8, calculate_binary_space },
{ PO::Tool::FileCharFormat::UNKNOW, calculate_binary_space },
};

namespace PO::Tool
{
	utf_file_reader::utf_file_reader(const std::experimental::filesystem::path& p) { open(p); }
	utf_file_reader::utf_file_reader(utf_file_reader&& uf) :file(std::move(uf.file)), format(uf.format)
	{
		uf.format = FileCharFormat::UNKNOW;
	}

	utf_file_reader& utf_file_reader::operator=(utf_file_reader&& uf)
	{
		utf_file_reader tem(std::move(uf));
		file = std::move(tem.file);
		format = tem.format;
		return *this;
	}

	bool utf_file_reader::open(const std::experimental::filesystem::path& p)
	{
		file.open(p, std::ios::binary);
		if (file.is_open())
		{
			uint32_t bom;
			file.read(reinterpret_cast<char*>(&bom), 4);
			format = translate_bom(bom);
			set_cursor_to_start();
			return true;
		}
		return false;
	}

	void utf_file_reader::set_cursor_to_start() noexcept
	{
		file.seekg(calculate_bom_space(format), std::ios::beg);
	}

	void utf_file_reader::close()
	{
		file.close();
		format = FileCharFormat::UNKNOW;
	}

	size_t utf_file_reader::estimation_utf8_count() noexcept
	{
		auto ite = estimation_count_to_utf8.find(format);
		if (ite != estimation_count_to_utf8.end())
			return ite->second(file);
		else
			return 0;
	}
	size_t utf_file_reader::estimation_utf32_count() noexcept
	{
		auto ite = estimation_count_to_utf32.find(format);
		if (ite != estimation_count_to_utf32.end())
			return ite->second(file);
		else
			return 0;
	}

	size_t utf_file_reader::read_all_utf32(char32_t* output, size_t avalible_count) noexcept
	{
		if (file.good())
		{
			auto ite = to_utf32_map.find(format);
			if (ite != to_utf32_map.end())
			{
				size_t index = 0;
				while (index < avalible_count && ite->second(file, output[index]))
					++index;
				return index;
			}
		}
		return 0;
	}
	/*
	size_t utf_file_reader::read_utf16_once(char* output, size_t avalible_count) noexcept
	{
		if (!eof())
		{
			
		}
	}
	*/

	size_t utf_file_reader::read_all_utf8(char* output, size_t avalible_count) noexcept
	{
		if (file.good())
		{
			auto ite = to_utf8_map.find(format);
			if (ite != to_utf8_map.end())
			{
				size_t index = 0;
				while (index < avalible_count)
				{
					size_t used = ite->second(file, output + index, avalible_count - index);
					if (used != 0)
						index += used;
					else
						break;
				}
				return index;
			}
		}
		return {};
	}
	std::string utf_file_reader::read_all_utf8()
	{
		if (file.good())
		{
			auto ite = estimation_count_to_utf8.find(format);
			if (ite != estimation_count_to_utf8.end())
			{
				size_t element_count = ite->second(file);
				std::string buffer(element_count, '\0');
				auto size = read_all_utf8(buffer.data(), element_count);
				buffer.resize(size);
				return buffer;
			}
		}
		return {};
	}

	std::u32string utf_file_reader::read_all_utf32()
	{
		if (file.good())
		{
			auto ite = estimation_count_to_utf32.find(format);
			if (ite != estimation_count_to_utf32.end())
			{
				size_t element_count = ite->second(file);
				std::u32string buffer(element_count, U'\0');
				auto size = read_all_utf32(buffer.data(), element_count);
				buffer.resize(size);
				return buffer;
			}
		}
		return {};
	}

	utf_file_writer::utf_file_writer(const std::experimental::filesystem::path& p, FileCharFormat format, bool append)
	{
		open(p, format, append);
	}

	utf_file_writer::utf_file_writer(utf_file_writer&& ufw) : file(std::move(ufw.file)), format(ufw.format) {}

	utf_file_writer& utf_file_writer::operator=(utf_file_writer&& uf)
	{
		utf_file_writer tem(std::move(uf));
		file = std::move(tem.file);
		format = tem.format;
		tem.format = FileCharFormat::UNKNOW;
		return *this;
	}

	bool utf_file_writer::open(const std::experimental::filesystem::path& p, FileCharFormat m_format, bool append)
	{
		if (append)
		{
			utf_file_reader read{ p };
			if (read.is_open())
				format = read.character_encoding();
		}
		else {
			format = m_format;
		}
		file.open(p, std::ios::binary | (append ? std::ios::app : 0));
		if (file.is_open())
		{
			write_bom_binary(file, format);
			return true;
		}
		return false;
	}

	void utf_file_writer::close()
	{
		file.close();
		format = FileCharFormat::UNKNOW;
	}

	void utf_file_writer::write(const char32_t* input, size_t count) noexcept
	{
		switch (format)
		{
		case FileCharFormat::UTF16BE:
		{
			size_t index = 0;
			while (index < count)
			{
				char buffer[4];
				size_t space = utf32_to_utf16(input[index++], reinterpret_cast<char16_t*>(buffer), 2);
				std::swap(buffer[0], buffer[1]);
				std::swap(buffer[2], buffer[3]);
				file.write(buffer, space * 2);
			}
			break;
		}
		case FileCharFormat::UTF16LE:
		{
			size_t index = 0;
			while (index < count)
			{
				char buffer[4];
				size_t space = utf32_to_utf16(input[index++], reinterpret_cast<char16_t*>(buffer), 2);
				file.write(buffer, space * sizeof(char16_t));
			}
			break;
		}
		case FileCharFormat::UTF32LE:
		{
			file.write(reinterpret_cast<const char*>(input), count * sizeof(char32_t));
			break;
		}
		case FileCharFormat::UTF32BE:
		{
			size_t index = 0;
			while (index < count)
			{
				char buffer[4];
				*reinterpret_cast<char32_t*>(buffer) = input[index++];
				std::swap(buffer[0], buffer[3]);
				std::swap(buffer[1], buffer[2]);
				file.write(buffer, 4);
			}
			break;
		}
		case FileCharFormat::UTF8:
		default:
		{
			size_t index = 0;
			while (index < count)
			{
				char buffer[6];
				size_t size = utf32_to_utf8(input[index++], buffer, 6);
				file.write(buffer, size);
			}
			break;
		}
		}
	}

	void utf_file_writer::write(const char* input, size_t count) noexcept
	{
		switch (format)
		{
		case FileCharFormat::UTF16BE:
		{
			size_t index = 0;
			while (index < count)
			{
				char buffer[4];
				size_t utf8, utf16;
				std::tie(utf8, utf16) = utf8_to_utf16(input, count, reinterpret_cast<char16_t*>(buffer), 2);
				std::swap(buffer[0], buffer[1]);
				std::swap(buffer[2], buffer[3]);
				file.write(buffer, utf16 * sizeof(char16_t));
				index += utf8;
			}
			break;
		}
		case FileCharFormat::UTF16LE:
		{
			size_t index = 0;
			while (index < count)
			{
				char buffer[4];
				size_t utf8, utf16;
				std::tie(utf8, utf16) = utf8_to_utf16(input, count, reinterpret_cast<char16_t*>(buffer), 2);
				file.write(buffer, utf16 * sizeof(char16_t));
				index += utf8;
			}
			break;
		}
		case FileCharFormat::UTF32LE:
		{
			char32_t tem;
			size_t index = 0;
			while (index < count)
			{
				size_t used = Tool::utf8_to_utf32(input, count, tem);
				file.write(reinterpret_cast<char*>(&tem), 1);
				index += used;
			}
			break;
		}
		case FileCharFormat::UTF32BE:
		{
			char tem[sizeof(char32_t)];
			size_t index = 0;
			while (index < count)
			{
				size_t used = Tool::utf8_to_utf32(input, count, *reinterpret_cast<char32_t*>(tem));
				std::swap(tem[0], tem[3]);
				std::swap(tem[1], tem[2]);
				file.write(reinterpret_cast<char*>(&tem), sizeof(char32_t));
				index += used;
			}
			break;
		}
		case FileCharFormat::UTF8:
		default:
		{
			file.write(input, count);
		}
		}
	}

	binary_file_reader::binary_file_reader(const std::experimental::filesystem::path& p)
	{
		open(p);
	}

	binary_file_reader::binary_file_reader(binary_file_reader&& uf) : m_file(std::move(uf.m_file)), m_total_space(uf.m_total_space), m_last_space(uf.m_last_space)
	{
		uf.m_total_space = 0;
		uf.m_last_space = 0;
	}

	binary_file_reader& binary_file_reader::operator=(binary_file_reader&& uf)
	{
		binary_file_reader tem(std::move(uf));
		m_file.operator=(std::move(tem.m_file));
		m_total_space = tem.m_total_space;
		m_last_space = tem.m_last_space;
		return *this;
	}

	void binary_file_reader::close()
	{
		m_file.close();
		m_total_space = 0;
		m_last_space = 0;
	}

	size_t binary_file_reader::read(void* buffer, size_t buffer_length) noexcept
	{
		uint64_t min_space = buffer_length >= m_last_space ? m_last_space : buffer_length;
		m_file.read(reinterpret_cast<char*>(buffer), min_space);
		m_last_space -= min_space;
		return min_space;
	}

	bool binary_file_reader::open(const std::experimental::filesystem::path& p)
	{
		m_total_space = 0;
		m_last_space = 0;
		m_file.open(p, std::ios::binary);
		if (m_file.is_open())
		{
			auto poi = m_file.tellg();
			m_file.seekg(0, std::ios::end);
			auto poi2 = m_file.tellg();
			m_file.seekg(0, std::ios::beg);
			m_total_space = uint64_t(poi2) - uint64_t(poi);
			m_last_space = m_total_space;
		}
		return false;
	}
	PO::Tool::binary_file_writer& operator<<(PO::Tool::binary_file_writer& o, const std::string& input)
	{
		uint64_t size = input.size();
		o << size;
		o.write(input.data(), size);
		return o;
	}
	PO::Tool::binary_file_reader& operator>>(PO::Tool::binary_file_reader& o, std::string& input)
	{
		uint64_t size;
		o >> size;
		input.resize(size);
		o.read(input.data(), size);
		return o;
	}

	PO::Tool::binary_file_writer& operator<<(PO::Tool::binary_file_writer& o, const std::u32string& input)
	{
		uint64_t size = input.size();
		o << size;
		o.write(reinterpret_cast<const char*>(input.data()), size * sizeof(char32_t));
		return o;
	}
	PO::Tool::binary_file_reader& operator>>(PO::Tool::binary_file_reader& o, std::u32string& input)
	{
		uint64_t size;
		o >> size;
		input.resize(size);
		o.read(reinterpret_cast<char*>(input.data()), size * sizeof(char32_t));
		return o;
	}
}


namespace PO::Tool::Doc
{

	namespace Implement
	{
		size_t utf8_to_utf16(std::ifstream& file, char16_t* output, size_t output_length) noexcept
		{
			assert(file.is_open());
			char utf8_buffer[6];
			file.read(utf8_buffer, 1);
			size_t size = utf8_require_space(utf8_buffer[0]);
			file.read(utf8_buffer + 1, size - 1);
			return Tool::utf8s_to_utf16s(utf8_buffer, size, output, output_length).second;
		}

		size_t ascii_to_utf16(std::ifstream& file, char16_t* output, size_t output_length) noexcept
		{
			assert(file.is_open());
			char ascii_buffer[2];
			file.read(ascii_buffer, 1);
			size_t size = ascii_require_space(ascii_buffer[0]);
			file.read(ascii_buffer + 1, size - 1);
			return Tool::ascii_to_utf16(ascii_buffer, size, output, output_length).second;
		}
	}

	loader_utf16::loader_utf16(const std::filesystem::path& path) noexcept 
		: m_file(path, std::ios::binary)
	{
		if (is_open())
		{
			m_format = format::UTF8;
			execute_function = Implement::ascii_to_utf16;
			/*
			uint32_t bom_buffer;
			m_file.read(reinterpret_cast<char*>(&bom_buffer), sizeof(uint32_t));
			if (bom_buffer & ::utf8_filter == ::utf8_bom)
			{
				m_format = format::UTF8;
				execute_function = Implement::utf8_to_utf16;
			}
			else
				assert(false);
				*/
		}
	}

	size_t loader_utf16::read_one(char16_t* output, size_t output_length) noexcept
	{
		assert(execute_function != nullptr);
		return execute_function(m_file, output, output_length);
	}
}