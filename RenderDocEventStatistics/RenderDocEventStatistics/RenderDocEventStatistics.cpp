#include <iostream>
#include <sstream>
#include "tool/file_asset.h"
#include "tool/script_analyze.h"
#include "GeneratorCommand.h"
using namespace PO::Tool;






/*
Lexical::regex_analyzer_wrapper_utf16 statement(
	{
		{u"DrawIndexed\\([0-9]*\\)", 5},
		{ u"DrawIndexedInstanced\\([0-9]*\\,[0-9]*\\)", 5 },
		{u"([a-z]|[A-Z]|[0-9]|_)*\\s([a-z]|[A-Z]|[0-9]|_)*", 5},
		{u"[0-9]*\\.?[0-9]*", 6},
	}
);
*/

std::string tos(float f)
{
	std::stringstream ss;
	ss << f;
	std::string result;
	ss >> result;
	return result;
}

std::string tos(uint32_t f)
{
	std::stringstream ss;
	ss << f;
	std::string result;
	ss >> result;
	return result;
}



int main(int args_num, const char** arss)
{
	/*
	if (args_num == 1)
	{
		args_num += 1;
		arss[1] = "result2.txt";
	}
	*/
	
	
	

	if (args_num == 1)
	{
		std::wcout << L"move RenderDoc event output file to this exe." << std::endl;
	}
	else if(args_num ==2){
		std::cout << "Start load :" << arss[1] << std::endl;
		std::wcout << L"Inprocess..." << std::endl;
		std::vector<draw_commend> result;
		try {
			result = GeneratorCommand(arss[1]);
		}
		catch (const std::exception& exp)
		{
			std::cout << exp.what() << std::endl;
			std::wcout << L"Please make sure the the target file is correct." << std::endl;
			system("pause");
			return -1;
		}
		catch (...)
		{
			std::wcout << L"Please make sure the the target file is correct." << std::endl;
			system("pause");
			return -1;
		}
		std::wcout << L"Complete! Total count :" << result.size() << std::endl;
		std::cout << "Generator result file ..." << std::endl;
		Lexical::regex_analyzer_wrapper_utf16 raw{
			{u"DrawIndexed\\([0-9]+\\)", 2},
			{u"DrawIndexedInstanced\\([0-9]+\\,\\s*[0-9]+\\)", 3},
			{u".+", 1}
		};

		/*
		Lexical::regex_analyzer_wrapper_utf16 raw2{
			{ u"WorldGridMaterial\\s\\S+", 3 },{ u"M\\S+\\s\\S+", 6 },
			{ u".+", 1 }
		};
		*/

		struct DrawCommandState
		{
			uint32_t count;
			float total_time;
		};

		struct ElementState
		{
			float total_time = 0;
			std::map<std::u16string, DrawCommandState> command_state;
			void insert(const std::u16string& name, DrawCommandState state)
			{
				auto ite = command_state.find(name);
				if (ite != command_state.end())
				{
					ite->second.count += state.count;
					ite->second.total_time += state.total_time;
				}
				else
					command_state.insert({ name, state });
				total_time += state.total_time;
			}
			void link(const ElementState& ES)
			{
				total_time += ES.total_time;
				for (auto& ite : ES.command_state)
				{
					auto ite2 = command_state.find(ite.first);
					if (ite2 != command_state.end())
					{
						ite2->second.count += ite.second.count;
						ite2->second.total_time += ite.second.total_time;
					}
					else {
						command_state.insert(ite);
					}
				}
			}
		};

		std::map<std::u16string, ElementState> all_data;
		std::vector<std::u16string> levels;

		std::vector < std::tuple< std::u16string, std::map<std::u16string, ElementState> >> all_levels;

		//PO::Tool::utf_file_writer ufw(path);
		//std::map<std::u16string, std::map<std::u16string, std::tuple<uint32_t, float>>> total;
		//std::map<std::u16string, std::map<std::u16string, std::tuple<uint32_t, float>>> all_data;
		
		for (auto& ite : result)
		{
			if (ite.EID == 0)
			{
				if (levels.size() > ite.leval + 1)
				{
					if (!all_data.empty())
					{

						std::u16string level_name;
						for (size_t i = 1; i < levels.size(); ++i)
							level_name += levels[i - 1] + u',';
						all_levels.push_back({ std::move(level_name), std::move(all_data) });
					}
					all_data.clear();
				}
				levels.resize(ite.leval + 1);
				levels[ite.leval] = ite.name;
			}
			else {
				raw.set_string(ite.name);
				raw.generate_token();
				if (raw.token() != 1)
				{
					auto& name = *levels.rbegin();
					//raw2.set_string(name);
					//raw2.generate_token();
					//if (raw2.token() != 1)
					//{
						auto ite2 = all_data.find(name);
						if (ite2 == all_data.end())
							ite2 = all_data.insert({ name, ElementState{} }).first;
						ite2->second.insert(ite.name, { 1, ite.duration });
						/*
						auto ite3 = ite2->second.find(ite.name);
						if (ite3 != ite2->second.end())
						{
							std::get<0>(ite3->second) += 1;
							std::get<1>(ite3->second) += ite.duration;
						}
						else {
							ite2->second.insert({ ite.name, {1, ite.duration } });
						}
						*/
					//}
				}
			}
		}

		std::map<std::u16string, ElementState> total;

		for (auto& ite122 : all_levels)
		{
			for (auto& ite2 : std::get<1>(ite122))
			{
				auto ite33 = total.find(ite2.first);
				if (ite33 != total.end())
					ite33->second.link(ite2.second);
				else
					total.insert(ite2);
			}
		}

		std::experimental::filesystem::path path = arss[1];
		auto ptt = path.filename();
		auto go = path.parent_path();
		path.remove_filename();
		//path.replace_extension();
		path = path.u16string() + (u"\\StatisticsResult_") + ptt.u16string();
		//((path += u".result") += u".txt");
		utf_file_writer ufw(path);

		auto outputfunction = [&](const std::u16string& name, const std::map<std::u16string, ElementState>& mapping)
		{
			ufw.write(U"\n\n********************************");
			ufw.write(utf16s_to_utf8s(name + u"\n"));

			std::vector<std::tuple<float, std::pair<std::u16string, ElementState>>> m_sort;
			for (auto& ite : mapping)
				m_sort.push_back({ ite.second.total_time, ite });
			std::sort(m_sort.begin(), m_sort.end(), [](auto& ite1, auto& ite2) { return std::get<0>(ite1) > std::get<0>(ite2); });

			for (auto& ite333 : m_sort)
			{
				auto& ref = std::get<1>(ite333);
				ufw.write(utf16s_to_utf8s(ref.first));
				ufw.write(U"\t-\t");
				ufw.write(tos(std::get<0>(ite333)));
				ufw.write(U"\n");
				for (auto& ite666 : ref.second.command_state)
				{
					ufw.write(U"\t\t");
					ufw.write(utf16s_to_utf8s(ite666.first));
					ufw.write(U"\tx");
					ufw.write(tos(ite666.second.count));
					ufw.write(U"\t - \t");
					ufw.write(tos(ite666.second.total_time));
					ufw.write(U"\n");
				}
			}
		};

		outputfunction(u"total", total);
		for (auto& ite : all_levels)
		{
			outputfunction(std::get<0>(ite), std::get<1>(ite));
		}


		auto utf16s = path.u16string();
		std::string re(utf16s.size() * 2, '0');
		auto p = utf16s_to_asciis(utf16s.data(), utf16s.size(), re.data(), re.size());
		re.resize(p.second);
		std::cout << "Down! Save to:" << re << std::endl;
	}
	else {
		std::wcout << L"RenderDocEventStatistics can only receive a path with RenderDoc event output file." << std::endl;
	}
	system("pause");
	return 0;
}