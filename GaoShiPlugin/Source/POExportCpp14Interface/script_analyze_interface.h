#pragma once
#include <vector>
#include <tuple>
#include <regex>
namespace Lexical
{

	enum class ReplacementToken
	{
		ClassName,
		DisplayName,
		IDName,
		Normal
	};

	namespace Implement
	{
		struct ReplacementResult
		{
			const char* start;
			const char* end;
			ReplacementToken token;
		};

		bool handle_replacement_token_implement(const wchar_t* start, size_t count, size_t& end, ReplacementToken& Result);
	}

	template<typename CallableFunction>
	bool handle_replacement_token(const wchar_t* start, size_t end, CallableFunction&& CF)
	{
		uint64_t index = 0;
		if (end != 0)
		{
			while (true)
			{
				uint64_t used;
				ReplacementToken token;
				if (Implement::handle_replacement_token_implement(start + index, end, used, token))
				{
					CF(token, start + index, used);
					index += used;
				}else
					break;
			}
			return true;
		}
		return false;
	}

	size_t translate_escape_sequence(const wchar_t* input, size_t input_count, wchar_t* output, size_t output_count) noexcept;

}