#pragma once
#include <vector>
#include "tool/file_asset.h"
#include "tool/script_analyze.h"
struct draw_commend
{
	uint32_t EID;
	uint32_t leval;
	uint32_t draw;
	std::u16string name;
	float duration;
};

std::vector<draw_commend> GeneratorCommand(const std::filesystem::path&);