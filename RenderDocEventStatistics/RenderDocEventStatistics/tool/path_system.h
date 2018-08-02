#pragma once
//#include <string>
#include <filesystem>
#include <map>
#include <mutex>
//#include <fstream>


namespace PO::FileSystem
{
	namespace fs = std::experimental::filesystem;
	using path = fs::path;
	inline auto current_path() { return fs::current_path(); }

	class path_encoding
	{
		fs::path m_start_path;
		uint32_t m_directory_offset;
		uint32_t m_current_index;
		std::map<fs::path, uint64_t> m_path_map;
	public:
		path_encoding(fs::path start_path = fs::current_path());
		size_t size() const noexcept { return m_path_map.size(); }
		size_t update();
	};
}