#include "GeneratorCommand.h"
#include <sstream>
using namespace PO::Tool;


std::tuple<std::u16string, size_t> clear_name(const std::u16string& b)
{
	size_t index = 0;
	auto start = b.begin();
	for (; start != b.end(); ++start)
	{
		if (*start != u' ' && *start != u'\\' && *start != u'-')
			break;
		++index;
	}
	auto end = start;
	auto end_ite = end;
	for (; end_ite != b.end(); ++end_ite)
	{
		if (*end_ite != u' ')
			end = end_ite;
	}
	if (end != b.end())
		++end;
	return { { start, end }, index };
}


std::vector<draw_commend> GeneratorCommand(const std::filesystem::path& path)
{
	Lexical::regex_analyzer_wrapper_utf16 raw(
		{
			{ u"\\|", 1 },
		{ u"\\s*[0-9]+.?[0-9]*\\s*", 2 },
		//{ u"\\s*\\\\?\\-\\s([a-z]|[A-Z]|\\s|_|\\(|\\)|=|\\.|,|[0-9]|\\:|\\#)*", 3 },
		{ u"[^\\|]*", 3 },
		{ u"\\s+", 4 }
		}
	);

	Lexical::regex_analyzer_wrapper_utf16 raw2(
		{
			{ u"[0-9]+\\.?[0-9]*",2 },
		{ u".*",3 },
		{ u"", 4 },
		}
	);
	std::vector<draw_commend> all_command;
	Doc::loader_utf16 lu16(path);
	if (lu16.is_open())
	{
		Lexical::stream_analyzer_wrapper_utf16 saw(
			[&](char16_t* output, size_t avalible) -> size_t {
			size_t result = lu16.read_one(output, avalible);
			if (!lu16.end_of_file())
				return result;
			return 0;
		},
			Lexical::line_analyzer{}
		);

		size_t line_count = 0;
		
		draw_commend current_command;
		while (saw.generate_token())
		{
			if (saw.cast_token<Lexical::line_analyzer::Token>() == Lexical::line_analyzer::Token::Line)
			{
				++line_count;
				if (line_count >= 4)
				{
					raw.set_string(saw.string());
					size_t state = 0;
					while (raw.generate_token())
					{
						if (raw.token() == 1)
						{
							state += 1;
						}
						else {
							std::u16string clear{ raw.string().begin(), raw.string().end() };
							auto result = clear_name(clear);
							raw2.set_string(std::get<0>(result));
							if (raw2.generate_token())
							{
								if (state == 0)
								{
									if (raw2.token() == 1)
										current_command.EID = 0;
									else if (raw2.token() == 2)
									{
										std::wstring tem = reinterpret_cast<const wchar_t*>(std::get<0>(result).c_str());
										std::wstringstream wss;
										wss << tem;
										wss >> current_command.EID;
										//current_command.EID = (current_command.EID - 3) / 2;
									}
								}
								else if (state == 1)
								{
									current_command.name = std::get<0>(result);
									current_command.leval = (std::get<1>(result) - 3) / 2;
								}
								else if (state == 2)
								{
									std::wstring tem = reinterpret_cast<const wchar_t*>(std::get<0>(result).c_str());
									std::wstringstream wss;
									wss << tem;
									wss >> current_command.draw;
								}
								else if (state == 3)
								{
									if (raw2.token() == 2)
									{
										std::wstring tem = reinterpret_cast<const wchar_t*>(std::get<0>(result).c_str());
										std::wstringstream wss;
										wss << tem;
										wss >> current_command.duration;
									}

								}
							}
							else if (state == 0)
								current_command.EID = 0;
						}
					}
					if (state != 3)
					{
						struct da : std::exception
						{
							const char* what() const noexcept
							{
								return "Unable to recognize";
							}
						};
						throw da{};
					}
					all_command.push_back(current_command);
				}

			}
			//std::wcout << reinterpret_cast<const wchar_t*>(saw.string().c_str()) <<L"]" << std::endl;
			
		}
		
	}
	return all_command;
}