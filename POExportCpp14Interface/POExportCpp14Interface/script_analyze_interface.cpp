#include "script_analyze_interface.h"
#include "po/tool/script_analyze.h"
namespace Lexical
{
	size_t translate_escape_sequence(const wchar_t* input, size_t input_count, wchar_t* output, size_t output_count) noexcept
	{
		size_t output_index = 0;
		if (PO::Lexical::regex_token_wrapper::generate(input, input_count, PO::Lexical::escape_sequence_cpp_wchar(),
			[&](PO::Lexical::EscapeSequence token, const wchar_t* s, size_t s_count)-> bool {
			auto count = PO::Lexical::translate(token, s, s_count, output + output_index, output_count - output_index);
			if (count == 0 || count + output_index >= output_count)
				return false;
			else {
				output_index += count;
				return true;
			}
		}, [](auto ...) ->bool {return true; }))
		{
			return output_index;
		}
		else
			return 0;
	}

	PO::Lexical::regex_token<ReplacementToken, wchar_t> replace_mark_implement
	{
	{L"%CN", ReplacementToken::ClassName },
	{L"%DN", ReplacementToken::DisplayName },
	{L"%IN", ReplacementToken::IDName},
	{L"%.+", ReplacementToken::Normal},
	{L"[^%]+", ReplacementToken::Normal}
	};
	namespace Implement
	{
		bool handle_replacement_token_implement(const wchar_t* start, size_t count, size_t& end, ReplacementToken& Result)
		{
			return PO::Lexical::regex_token_wrapper::generate(start, count, replace_mark_implement, 
				[&](ReplacementToken token, const wchar_t* input, size_t input_count) {
				end = input_count;
				Result = token;
				return false;
			}, [](auto...) {return true; });
		}
	}
	
}