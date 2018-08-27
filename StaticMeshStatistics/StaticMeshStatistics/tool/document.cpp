#include "document.h"
#include "character_encoding.h"
#include <assert.h>


namespace 
{
	using namespace PO::Encoding;
	constexpr uint32_t utf8_filter = 0x00FF'FFFF;
	constexpr uint32_t utf8_bom = 0x00BF'BBEF;
	constexpr uint32_t utf32_be_bom = 0xFEFF'0000;
	constexpr uint32_t utf32_le_bom = 0x0000'FFFE;
	constexpr uint32_t utf16_filter = 0x0000'FFFF;
	constexpr uint32_t utf16_be_bom = 0x0000'FFFE;
	constexpr uint32_t utf16_le_bom = 0x0000'FEFF;

	enum class BomType
	{
		None,
		UTF8,
		UTF16LE,
		UTF16BE,
		UTF32LE,
		UTF32BE
	};


	BomType translate_bom(uint32_t bom)
	{
		if ((bom & utf8_filter) == utf8_bom)
			return BomType::UTF8;
		else if (bom == utf32_be_bom)
			return BomType::UTF32BE;
		else if (bom == utf32_le_bom)
			return BomType::UTF32LE;
		else if ((bom & utf16_filter) == utf16_le_bom)
			return BomType::UTF16LE;
		else if ((bom & utf16_filter) == utf16_be_bom)
			return BomType::UTF16BE;
		else
			return BomType::None;
	}

	size_t write_bom_binary(std::ofstream& output, BomType format)
	{
		switch (format)
		{
		case BomType::UTF8:
		{
			unsigned char bom[] = { 0xef, 0xbb, 0xbf };
			output.write(reinterpret_cast<char*>(&bom), 3);
			return 3;
		}
		case BomType::UTF16LE:
		{
			unsigned char bom[] = { 0xfe, 0xff };
			output.write(reinterpret_cast<char*>(&bom), 2);
			return 2;
		}
		case BomType::UTF16BE:
		{
			unsigned char bom[] = { 0xff, 0xfe };
			output.write(reinterpret_cast<char*>(&bom), 2);
			return 2;
		}
		case BomType::UTF32LE:
		{
			unsigned char bom[] = { 0x00, 0x00, 0xfe, 0xff };
			output.write(reinterpret_cast<char*>(&bom), 4);
			return 4;
		}
		case BomType::UTF32BE:
		{
			unsigned char bom[] = { 0xfe, 0xff, 0x00, 0x00 };
			output.write(reinterpret_cast<char*>(&bom), 4);
			return 4;
		}
		default:
			return 0;
		}
	}

	size_t calculate_bom_space(BomType format)
	{
		switch (format)
		{
		case BomType::UTF8:
			return 3;
		case BomType::UTF16LE:
		case BomType::UTF16BE:
			return 2;
		case BomType::UTF32BE:
		case BomType::UTF32LE:
			return 4;
		default:
			return 0;
		}
	}

	// for read_utf16
	size_t read_utf8_to_utf16(std::ifstream& file, char16_t* output, size_t output_length) noexcept
	{
		assert(file.is_open());
		char utf8_buffer[6] = { 0, 0, 0, 0, 0, 0 };
		file.read(utf8_buffer, 1);
		size_t size = utf8_require_space(utf8_buffer[0]);
		if (size != 0)
		{
			file.read(utf8_buffer + 1, size - 1);
			if (utf8_check_string(utf8_buffer, 6, size))
				return utf8_to_utf16(utf8_buffer, size, output, output_length).second;
			else
				return 0;
		}

		return size;
	}

	size_t read_ascii_to_utf16(std::ifstream& file, char16_t* output, size_t output_length) noexcept
	{
		assert(file.is_open());
		char ascii_buffer[2] = { 0, 0 };
		file.read(ascii_buffer, 1);
		size_t size = ansi_require_space(ascii_buffer[0]);
		file.read(ascii_buffer + 1, size - 1);
		auto re = ansi_to_utf16(ascii_buffer, size, output, output_length).second;
		return re;
	}

	size_t read_utf16_le_to_utf16(std::ifstream& file, char16_t* output, size_t output_length) noexcept
	{
		assert(file.is_open());
		char16_t buffer[2] = { 0, 0 };
		file.read(reinterpret_cast<char*>(buffer), sizeof(char16_t));
		size_t size = utf16_require_space(buffer[0]);
		if (size == 1)
		{
			if (output_length >= 1)
			{
				std::memcpy(output, buffer, sizeof(char16_t));
				return 1;
			}
		}
		else {
			assert(size == 2);
			if (output_length >= 2)
			{
				std::memcpy(output, buffer, sizeof(char16_t) * 2);
				return 2;
			}
		}
		return 0;
	}

	// for read_utf32
	size_t read_ascii_to_utf32(std::ifstream& file, char32_t* output) noexcept
	{
		char16_t utf16_buffer[2];
		size_t size = read_ascii_to_utf16(file, utf16_buffer, 2);
		if (size != 0)
		{
			utf16_to_uft32(utf16_buffer, size, output[0]);
			return 1;
		}
		return 0;
	}

	size_t read_utf8_to_utf32(std::ifstream& file, char32_t* output) noexcept
	{
		char16_t utf16_buffer[2];
		size_t size = read_utf8_to_utf16(file, utf16_buffer, 2);
		if (size != 0)
		{
			utf16_to_uft32(utf16_buffer, size, output[0]);
			return 1;
		}
		return 0;
	}

	// for write_utf16
	void write_utf16_to_utf8(std::ofstream& o, const char16_t* input, size_t length)
	{
		size_t index = 0;
		char buffer[6];
		while (true)
		{
			auto p = utf16s_to_utf8s(input + index, length, buffer, 6);
			if (p.first != 0)
			{
				assert(p.second != 0);
				o.write(buffer, p.second);
				length -= p.first;
				index += p.first;
			}
			else
				break;
		}
	}

	// for write_utf32
	void write_utf32_to_utf8(std::ofstream& o, const char32_t* input, size_t length)
	{
		size_t index = 0;
		char buffer[6];
		while (true)
		{
			auto p = utf32s_to_utf8s(input + index, length, buffer, 6);
			if (p.first != 0)
			{
				assert(p.second != 0);
				o.write(buffer, p.second);
				length -= p.first;
				index += p.first;
			}
			else
				break;
		}
	}

}


namespace PO :: Doc
{
	BomType format_to_bom(Format format)
	{
		switch (format)
		{
		case Format::UTF8_WITH_BOM:
			return BomType::UTF8;
		case Format::UTF16BE:
			return BomType::UTF16BE;
		case Format::UTF16LE:
			return BomType::UTF16LE;
		case Format::UTF32LE:
			return BomType::UTF32LE;
		case Format::UTF32BE:
			return BomType::UTF32BE;
		case Format::UTF8:
		case Format::NOT_UNICODE:
		default:
			return BomType::None;
		}
	}

	Format bom_to_format(BomType bt, Format default_format)
	{
		switch (bt)
		{
		case BomType::None:
			return default_format;
		case BomType::UTF8:
			return Format::UTF8;
		case BomType::UTF16LE:
			return Format::UTF16LE;
		case BomType::UTF16BE:
			return Format::UTF16BE;
		case BomType::UTF32LE:
			return Format::UTF32LE;
		case BomType::UTF32BE:
			return Format::UTF32BE;
		default:
			return default_format;
		}
	}
	namespace Implement
	{

		loader_base::loader_base() noexcept : m_format(Format::UTF8) {}
		loader_base::loader_base(loader_base&& lb) noexcept :m_file(std::move(lb.m_file)), m_format(lb.m_format)
		{
			lb.m_format = Format::UTF8;
		}
		loader_base::loader_base(const std::filesystem::path& path, Format default_format) noexcept
			: m_file(path, std::ios::binary), m_format(default_format)
		{
			if (m_file.is_open())
			{
				uint32_t bom_buffer = 0;
				m_file.read(reinterpret_cast<char*>(&bom_buffer), sizeof(uint32_t));
				BomType bom_type = translate_bom(bom_buffer);
				m_format = bom_to_format(bom_type, default_format);
				m_file.seekg(calculate_bom_space(bom_type));
			}
			else
				m_format = Format::UTF8;
		}

		loader_base& loader_base::operator=(loader_base&& lb) noexcept
		{
			loader_base lbt(std::move(lb));
			m_file = std::move(lbt.m_file);
			m_format = lbt.m_format;
			return *this;
		}

		void loader_base::seek_begin() noexcept
		{
			m_file.seekg(calculate_bom_space(format_to_bom(m_format)));
		}

		void loader_base::close() noexcept
		{
			m_file.close();
		}

		loader_utf16::loader_utf16(const std::filesystem::path& path, Format default_format) noexcept
			: loader_base(path, default_format)
		{
			if (is_open())
				reset_format(format());
		}

		loader_utf16& loader_utf16::operator=(loader_utf16&& lu) noexcept
		{
			loader_utf16 lut(std::move(lu));
			loader_base::operator=(std::move(lu));
			execute_function = lut.execute_function;
			return *this;
		}

		void loader_utf16::reset_format(Format f) noexcept
		{
			m_format = f;
			switch (f)
			{
			case Format::UTF8:
			case Format::UTF8_WITH_BOM:
				execute_function = ::read_utf8_to_utf16;
				break;
			case Format::UTF16LE:
				execute_function = ::read_utf16_le_to_utf16;
				break;
			case Format::NOT_UNICODE:
				execute_function = ::read_ascii_to_utf16;
				break;
			default:
				assert(false);
			}
		}

		result loader_utf16::read_one(char16_t* output, size_t output_length) noexcept
		{
			assert(is_open());
			size_t size = (*execute_function)(m_file, output, output_length);
			if (!end_of_file() && size != 0)
				return { size, ErrorState::OK };
			else if (end_of_file())
				return { 0, ErrorState::OK };
			else {
				if (output_length >= 1)
					output[0] = u'?';
				return { 1, ErrorState::WrongFormat };
			}
		}

		loader_utf32::loader_utf32(const std::filesystem::path& path, Format default_format) noexcept
			: loader_base(path, default_format)
		{
			if (is_open())
				reset_format(format());
		}

		void loader_utf32::reset_format(Format f) noexcept
		{
			m_format = f;
			switch (f)
			{
			case Format::UTF8:
			case Format::UTF8_WITH_BOM:
				execute_function = ::read_utf8_to_utf32;
				break;
			case Format::NOT_UNICODE:
				execute_function = ::read_ascii_to_utf32;
				break;
			default:
				assert(false);
			}
		}

		result loader_utf32::read_one(char32_t* output, size_t output_length) noexcept
		{
			if (output_length >= 1)
			{
				assert(is_open());
				size_t size = (*execute_function)(m_file, output);
				if (!end_of_file() && size != 0)
					return { size, ErrorState::OK };
				else if (end_of_file())
					return { 0, ErrorState::OK };
				else {
					if (output_length >= 1)
						output[0] = u'?';
					return { 1, ErrorState::WrongFormat };
				}
			}
			return { 0, ErrorState::OK };
		}

		loader_utf32& loader_utf32::operator=(loader_utf32&& lu) noexcept
		{
			loader_utf32 lut(std::move(lu));
			loader_base::operator=(std::move(lu));
			execute_function = lut.execute_function;
			return *this;
		}

	}

	namespace Implement
	{
		writer_base::writer_base(const std::filesystem::path& path, Format format) noexcept
			: m_file(path, std::ios::binary), m_format(format)
		{
			if (is_open())
				write_bom_binary(m_file, format_to_bom(m_format));
		}

		writer_base::writer_base() noexcept : m_format(Format::UTF8) {}

		writer_base::writer_base(writer_base&& eb) noexcept : m_file(std::move(eb.m_file)), m_format(eb.m_format)
		{
			eb.m_format = Format::UTF8;
		}

		writer_base& writer_base::operator=(writer_base&& wb) noexcept
		{
			writer_base ebt(std::move(wb));
			m_file = std::move(ebt.m_file);
			m_format = ebt.m_format;
			return *this;
		}

		writer_utf16::writer_utf16(const std::filesystem::path& path, Format format) noexcept
			: writer_base(path, format)
		{
			if (is_open())
			{
				switch (m_format)
				{
				case Format::UTF8:
				case Format::UTF8_WITH_BOM:
					execute_function = ::write_utf16_to_utf8;
					break;
				default:
					assert(false);
					break;
				}
			}
		}

		writer_utf16& writer_utf16::operator=(writer_utf16&& wu) noexcept
		{
			writer_utf16 lut(std::move(wu));
			writer_base::operator=(std::move(wu));
			execute_function = lut.execute_function;
			return *this;
		}

		void writer_utf16::write(const char16_t* input, size_t input_length)
		{
			assert(is_open());
			(*execute_function)(m_file, input, input_length);
		}

		writer_utf32::writer_utf32(const std::filesystem::path& path, Format format) noexcept
			: writer_base(path, format)
		{
			if (is_open())
			{
				switch (m_format)
				{
				case Format::UTF8:
				case Format::UTF8_WITH_BOM:
					execute_function = ::write_utf32_to_utf8;
					break;
				default:
					assert(false);
					break;
				}
			}
		}

		void writer_utf32::write(const char32_t* input, size_t input_length)
		{
			assert(is_open());
			(*execute_function)(m_file, input, input_length);
		}
	}
}