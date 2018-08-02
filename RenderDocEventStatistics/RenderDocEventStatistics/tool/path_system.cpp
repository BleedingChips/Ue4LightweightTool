#include "path_system.h"
#include <iostream>
#include <assert.h>
namespace PO::FileSystem
{
	path_encoding::path_encoding(fs::path start_path) : m_start_path(std::move(start_path)), m_current_index(1) { update(); }
	size_t path_encoding::update()
	{
		std::cout << fs::absolute(fs::path{u".\\"}) << std::endl;
		for (const auto& ite : fs::recursive_directory_iterator{ fs::path{ u".\\" } })
		{
			//auto current = ite - m_start_path;
			if (fs::is_directory(ite))
			{
				std::cout << "find directory:" << ite << std::endl;
			}
			else {
				auto current_path = fs::path{ ite };
				auto pa_ite = current_path.begin();
				auto cu_ite = m_start_path.begin();
				fs::path result{u".\\"};
				while (pa_ite != current_path.end() && cu_ite != m_start_path.end())
				{
					if (*pa_ite != *cu_ite)
					{
						for (;pa_ite != current_path.end(); ++pa_ite)
							result /= *pa_ite;
						std::cout << "result : " << result << std::endl;
						break;
					}
					else {
						++pa_ite;
						++cu_ite;
					}
				}
				//auto ite2 = m_start_path.begin();
				std::cout << "find file: [" << ite << "] with id :" << m_current_index << std::endl;
				m_path_map.insert({ std::move(result) , m_current_index++ });
			}
		}
		return m_path_map.size();
	}





	/*
	uint64_t path_encoding::update(fs::path start_path)
	{
		m_start_path = std::move(start_path);
		m_path_map.clear();
		uint64_t index = 1;
		std::vector<fs::directory_iterator> directory_stack{ fs::directory_iterator{ m_start_path } };
		while (!directory_stack.empty())
		{
			auto current = std::move(*directory_stack.rbegin());
			directory_stack.pop_back();
			for (const auto& ite : current)
			{
				//auto current = ite - m_start_path;
				if (fs::is_directory(ite))
				{
					directory_stack.push_back(fs::directory_iterator{ ite });
					std::cout << "find directory:" << ite << std::endl;
				}
				else {
					std::cout << "find file:" << ite << "with id :" << index << std::endl;
					m_path_map.insert({ std::move(ite) , index++ });
				}
			}
		}
		return index;
	}
	*/
}