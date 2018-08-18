#include "script_analyze_interface.h"
#include "tool/script_analyze.h"
namespace Lexical
{
	size_t translate_escape_sequence(const wchar_t* input, size_t input_count, wchar_t* output, size_t output_count) noexcept
	{
		size_t output_index = 0;
		PO::Lexical::regex_token_wrapper<wchar_t> wrapper{input, input + input_count};
		while (auto result = wrapper.generate_token(PO::Lexical::escape_sequence_cpp_wchar()))
		{
			auto string = wrapper.string();
			auto count = PO::Lexical::translate(*result, string.data(), string.size(), output + output_index, output_count - output_index);
			if (count == 0 || count + output_index >= output_count)
				break;
		}
		return output_index;
	}
}