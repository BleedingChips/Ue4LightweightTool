#include "InfoRecord.h"
#include "HAL/PlatformApplicationMisc.h"
#include "LevelEditorViewport.h"
#include "POExportCpp14Interface/script_analyze_interface.h"
#include <sstream>
#include <map>
#ifdef WITH_EDITORONLY_DATA

using namespace Lexical;

void CopySelectionActorToClioboard(const FString& replace)
{
	std::vector<wchar_t> result;
	result.resize(replace.Len());
	auto count = translate_escape_sequence(*replace, replace.Len(), result.data(), result.size());
	result.resize(count);
	std::vector<std::tuple<Lexical::ReplacementToken, FString>> pattern;
	
	if (Lexical::handle_replacement_token(result.data(), result.size(), 
		[&](Lexical::ReplacementToken token, const wchar_t* input, size_t input_count) {
		pattern.push_back({ token,FString{static_cast<int32_t>(input_count), input} });
	}))
	{
		FString TotalString;
		USelection* selection = GEditor->GetSelectedActors();
		if (IsValid(selection))
		{
			TArray<TWeakObjectPtr<>> object_array;
			selection->GetSelectedObjects(object_array);
			auto count = object_array.Num();
			for (size_t i = 0; i < count; ++i)
			{
				auto ptr = object_array[i];
				if (ptr.IsValid())
				{
					FName N = ptr->GetClass()->GetFName();
					FString ClassName = N.ToString();
					FString IDName = ptr->GetName();
					FString DisplayName = UKismetSystemLibrary::GetDisplayName(ptr.Get());
					for (auto& ite : pattern)
					{
						switch (std::get<0>(ite))
						{
						case Lexical::ReplacementToken::ClassName:
							TotalString += ClassName;
							break;
						case Lexical::ReplacementToken::DisplayName:
							TotalString += DisplayName;
							break;
						case Lexical::ReplacementToken::IDName:
							TotalString += IDName;
							break;
						case Lexical::ReplacementToken::Normal:
							TotalString += std::get<1>(ite);
							break;
						}
					}
				}
			}
			FPlatformApplicationMisc::ClipboardCopy(*TotalString);
		}
	}
}

FString PositionToString(const FVector& FV, const FRotator& FR)
{
	return FString{ L"{" } +FV.ToString() + FString{ L" " } +FR.ToString() + FString{ L"}" };
}

void CopyCurrentPosition(UWorld* world)
{
	FVector CameraLocation;
	FRotator CameraRotation;
	if (FApp::IsGame())
	{
		auto* pc = world->GetFirstPlayerController();
		auto* Cm = pc->PlayerCameraManager;
		CameraLocation = Cm->GetCameraLocation();
		CameraRotation = Cm->GetCameraRotation();
	}
	else {
#ifdef WITH_EDITORONLY_DATA
		for (auto LevelVC : GEditor->LevelViewportClients)
		{
			if (LevelVC && LevelVC->IsPerspective())
			{
				CameraLocation = LevelVC->GetViewLocation();
				CameraRotation = LevelVC->GetViewRotation();
				
				break;
			}
		}
#endif
	}

	FPlatformApplicationMisc::ClipboardCopy(*PositionToString(CameraLocation, CameraRotation));
}




const std::wregex& PositionRegex()
{
	static std::wregex regex{ LR"(X\s*=\s*(\+|-)?([0-9]+)(.[0-9]+)?(\s|,)+Y\s*=\s*(\+|-)?([0-9]+)(.[0-9]+)?(\s|,)+Z\s*=\s*(\+|-)?([0-9]+)(.[0-9]+)?)", std::regex::optimize };
	return regex;
}

const std::wregex& RotateRegex()
{
	static std::wregex regex{ LR"(\P\s*=\s*(\+|-)?([0-9]+)(.[0-9]+)?(\s|,)+\Y\s*=\s*(\+|-)?([0-9]+)(.[0-9]+)?(\s|,)+\R\s*=\s*(\+|-)?([0-9]+)(.[0-9]+)?)", std::regex::optimize };
	return regex;
}

const std::wregex& NumberRegex()
{
	static std::wregex regex{ LR"((\+|-)?([0-9]+)(.[0-9]+))", std::regex::optimize };
	return regex;
}

float stof(const wchar_t* input, size_t count)
{
	float tem = 0.0f;
	std::wstringstream wss;
	wss.write(input, count);
	wss >> tem;
	return tem;
}

void StringToTransort(const FString& replace, FVector& CameraLocation, FRotator& CameraRotation)
{
	std::wcmatch LocationMatch;
	if (std::regex_search(*replace, *replace + replace.Len(), LocationMatch, PositionRegex()))
	{
		std::pair<const wchar_t*, const wchar_t*> rengae = { LocationMatch[0].first , LocationMatch[0].second };
		std::wcmatch NumberMatch;
		uint32_t state = 0;
		while (state <= 2 && std::regex_search(rengae.first, rengae.second, NumberMatch, NumberRegex()))
		{
			switch (state)
			{
			case 0:
				CameraLocation.X = stof(&*(NumberMatch[0].first), NumberMatch[0].length());
				break;
			case 1:
				CameraLocation.Y = stof(&*(NumberMatch[0].first), NumberMatch[0].length());
				break;
			case 2:
				CameraLocation.Z = stof(&*(NumberMatch[0].first), NumberMatch[0].length());
				break;
			}
			++state;
			rengae = { NumberMatch.suffix().first, NumberMatch.suffix().second };
		}
	}
	std::wcmatch RotateMatch;
	if (std::regex_search(*replace, *replace + replace.Len(), RotateMatch, RotateRegex()))
	{
		std::pair<const wchar_t*, const wchar_t*> rengae = { RotateMatch[0].first , RotateMatch[0].second };
		std::wcmatch NumberMatch;
		uint32_t state = 0;
		while (state <= 2 && std::regex_search(rengae.first, rengae.second, NumberMatch, NumberRegex()))
		{
			switch (state)
			{
			case 0:
				CameraRotation.Pitch = stof(&*(NumberMatch[0].first), NumberMatch[0].length());
				break;
			case 1:
				CameraRotation.Yaw = stof(&*(NumberMatch[0].first), NumberMatch[0].length());
				break;
			case 2:
				CameraRotation.Roll = stof(&*(NumberMatch[0].first), NumberMatch[0].length());
				break;
			}
			++state;
			rengae = { NumberMatch.suffix().first, NumberMatch.suffix().second };
		}
	}
}

void MoveToPosition(const FString& replace)
{
	if (!FApp::IsGame())
	{
		FLevelEditorViewportClient* View = nullptr;
		for (auto LevelVC : GEditor->LevelViewportClients)
		{
			if (LevelVC && LevelVC->IsPerspective())
			{
				View = LevelVC;
				break;
			}
		}
		if (View != nullptr)
		{
			FVector CameraLocation = View->GetViewLocation();
			FRotator CameraRotation = View->GetViewRotation();
			StringToTransort(replace, CameraLocation, CameraRotation);
			View->SetViewLocation(CameraLocation);
			View->SetViewRotation(CameraRotation);
		}
	}
}

#endif