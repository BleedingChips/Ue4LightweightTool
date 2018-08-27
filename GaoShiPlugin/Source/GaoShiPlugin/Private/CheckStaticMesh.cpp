#include "CheckStaticMesh.h"
#include "Classes/Animation/SkeletalMeshActor.h"
#include "POExportCpp14Interface/document_interface.h"
#include "InstancedFoliage.h"
#include "FoliageType.h"
#include "InstancedFoliageActor.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Classes/Engine/AssetManager.h"
#include <mutex>
#include <thread>
#include <atomic>
#include <set>
#include <sstream>
#include <algorithm>
#include <assert.h>
struct LODState
{
	int32 triangle;
	int32 section;
	float screne_rate;
};

std::pair<std::vector<LODState>, bool> LoadMeshLoad(UStaticMesh* SM)
{
	std::vector<LODState> tem;
	size_t mesh_count = SM->GetNumLODs();
	for (size_t i = 0; i < mesh_count; ++i)
	{
		const FStaticMeshLODResources& res = SM->GetLODForExport(i);
		tem.push_back({ LODState{ res.GetNumTriangles(), res.Sections.Num(), SM->SourceModels[i].ScreenSize } });
	}
	std::set<FString> m_material_name;
	for (auto ite : SM->StaticMaterials)
	{
		if (ite.MaterialInterface)
		{
			auto name = ite.MaterialInterface->GetName();
			if (m_material_name.find(name) != m_material_name.end())
				return { std::move(tem), true };
			else
				m_material_name.insert(name);
		}
	}
	return { std::move(tem), false };
}

template<typename T>
std::wstring NumberToString(T value)
{
	std::wstringstream wss;
	wss.clear();
	std::wstring buffer;
	wss << value;
	wss >> buffer;
	return buffer;
}

std::wstring MeshLODToString(const std::vector<LODState>& input)
{
	std::wstring temporary;
	for (auto& ite : input)
		temporary += NumberToString(ite.triangle) + L"," + NumberToString(ite.section) + L"," + NumberToString(ite.screne_rate);
	return temporary;
}

struct MeshState
{
	uint32_t m_actor_count = 0;
	uint32_t m_foliage_count = 0;
	bool m_repeat_material = false;
	std::vector<LODState> m_all_state;
};

std::wstring MeshStateToString(const wchar_t* name,  const MeshState& MS)
{
	std::wstring temporary;
	temporary += std::wstring(name) + L"," + NumberToString(MS.m_actor_count) + L"," + NumberToString(MS.m_foliage_count) + L",";
	if (MS.m_repeat_material)
		temporary += L"是,";
	else
		temporary += L"否,";
	temporary += MeshLODToString(MS.m_all_state) + L"\r\n";
	return temporary;
}

void CheckCurrentLevelStaticMesh(UWorld* World)
{
	std::map<FString, MeshState> m_all_mesh;
	TActorIterator<AActor> all_actor{ World };
	for (; all_actor; ++all_actor)
	{
		auto static_mesh_actor = Cast<AStaticMeshActor>(*all_actor);
		if (IsValid(static_mesh_actor) && IsValid(static_mesh_actor->GetStaticMeshComponent()) )
		{
			auto component = static_mesh_actor->GetStaticMeshComponent();
			if (IsValid(component))
			{
				auto mesh = component->GetStaticMesh();
				if (mesh != nullptr)
				{
					auto name = mesh->GetName();
					auto fine = m_all_mesh.find(name);
					if (fine == m_all_mesh.end())
					{
						fine = m_all_mesh.insert({ name, MeshState {} }).first;
						auto result = LoadMeshLoad(mesh);
						fine->second.m_repeat_material = result.second;
						fine->second.m_all_state = std::move(result.first);
					}
					fine->second.m_actor_count += 1;
				}
			}
		}
		else {
			AInstancedFoliageActor* actoraif = Cast<AInstancedFoliageActor>(*all_actor);
			if (actoraif != nullptr)
			{
				TMap<UFoliageType*, FFoliageMeshInfo*> map = actoraif->GetAllInstancesFoliageType();
				for (auto& ite : map)
				{
					auto index = ite.Value->Instances.Num();
					auto component = ite.Value->Component;
					if (IsValid(component))
					{
						auto mesh = component->GetStaticMesh();
						if (mesh != nullptr)
						{
							auto name = mesh->GetName();
							auto fine = m_all_mesh.find(name);
							if (fine == m_all_mesh.end())
							{
								fine = m_all_mesh.insert({ name, MeshState {} }).first;
								auto result = LoadMeshLoad(mesh);
								fine->second.m_repeat_material = result.second;
								fine->second.m_all_state = std::move(result.first);
							}
							fine->second.m_foliage_count += index;
						}
					}
				}
			}
		}
	}

	FString ThePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectLogDir());
	FString CurrentDir = World->GetMapName();
	ThePath += CurrentDir + L".csv";
	Doc::writer output(*ThePath, Doc::Format::UTF8_WITH_BOM);
	
	if (output.is_open())
	{
		UE_LOG(LogTemp, Log, L"Success OpenFile %s", (*ThePath));
		output.write(*World->GetMapName());
		output.write(L"\r\n");
		output.write(L"模型名字,Actor数,植物数,是否含有相同的Matrial,LOD面数,LODSection数,LOD屏幕距离\r\n");
		for (auto& ite : m_all_mesh)
			output.write(MeshStateToString(*ite.first, ite.second));
	}
	else {
		UE_LOG(LogTemp, Log, L"File OpenFile %s", (*ThePath));
	}
	UE_LOG(LogTemp, Log, L"%s", (*World->GetMapName()));
}

struct ExecuteBlock
{
	FStreamableManager Manager;
	std::vector<std::tuple<FString, FAssetData>> target;
	decltype(target)::const_iterator start, end;
	std::vector<std::tuple<FString, uint64_t, bool, FString>> result;
	Doc::writer output;
	TSharedPtr<FStreamableHandle> current_handle;
	ExecuteBlock(Doc::writer&& w) : output(std::move(w)) {}
};

void execute_function(std::shared_ptr<ExecuteBlock> control)
{
	uint64_t current = control->end - control->target.begin();
	uint64_t total = control->target.size();
	UE_LOG(LogTemp, Log, L"Handling : %d - %d", current, total);
	for (control->start; control->start != control->end; ++(control->start))
	{
		UObject* UO = std::get<1>(*control->start).GetAsset();
		assert(UO);
		UStaticMesh* USM = Cast<UStaticMesh>(UO);
		if (IsValid(USM))
		{
			auto re = LoadMeshLoad(USM);
			auto Path = std::get<1>(*control->start).ObjectPath.ToString();
			control->result.push_back({ std::move(std::get<0>(*control->start)), std::get<0>(re)[0].triangle, std::get<1>(re), std::move(Path) });
		}
	}
	
	if (control->end != control->target.end())
	{
		control->start = control->end;
		control->end = control->end + 20;
		if (control->end > control->target.end())
			control->end = control->target.end();
		TArray<FSoftObjectPath> m_tar;
		for (auto ite = control->start; ite != control->end; ++ite)
			m_tar.Add(std::get<1>(*ite).ToSoftObjectPath());
		control->Manager.RequestAsyncLoad(m_tar, std::bind(&execute_function, control));
	}
	else {

		std::sort(control->result.begin(), control->result.end(), [](const decltype(control->result)::value_type& p, const decltype(control->result)::value_type& k) -> bool {
			auto& ref = std::get<1>(p);
			auto& ref2 = std::get<1>(k);
			return ref > ref2;
		});

		control->output.write(L"名称,最大面数,重复材质,地址,\r\n");
		for (auto& ite : control->result)
		{
			control->output.write(*std::get<0>(ite));
			control->output.write(L",");
			control->output.write(NumberToString(std::get<1>(ite)) + L",");
			if (std::get<2>(ite))
				control->output.write(L"是,");
			else
				control->output.write(L"否,");
			control->output.write(*std::get<3>(ite));
			control->output.write(L",\r\n");
		}
		UE_LOG(LogTemp, Log, L"Down");
	}
}

void CheckTotalStaticMesh()
{
	FString ThePath = FPaths::ProjectLogDir();
	ThePath += L"total_mesh.csv";
	Doc::writer output(*ThePath, Doc::Format::UTF8_WITH_BOM);
	if (output.is_open())
	{
		std::shared_ptr<ExecuteBlock> control = std::make_shared<ExecuteBlock>(std::move(output));
		IPlatformFile& fileManager = FPlatformFileManager::Get().GetPlatformFile();
		UAssetManager& UAM = UAssetManager::Get();
		TArray<FString> path;
		fileManager.FindFilesRecursively(path, *FPaths::ProjectContentDir(), L".uasset");
		std::vector<FAssetData> request;
		for (auto ite = path.CreateConstIterator(); ite; ++ite)
		{
			FString tem{ ite->Len() - 7, **ite };
			FString Name = FPaths::GetCleanFilename(tem);
			FPaths::MakePathRelativeTo(tem, *FPaths::ProjectContentDir());
			tem = L"/Game/" + tem + L"." + Name;
			FAssetData FAD;
			FName StaticMeshName = FName(L"StaticMesh");
			if (UAM.GetAssetDataForPath(tem, FAD))
			{
				if(FAD.AssetClass == StaticMeshName)
					control->target.push_back({ Name, FAD });
			}
		}
		control->start = control->target.begin();
		control->end = control->start;
		execute_function(control);
	}
	else {
		UE_LOG(LogTemp, Log, L"Unable to open output file :%s", *ThePath);
	}


	/*

	bool avalible;
	{
		std::lock_guard<std::mutex> lg(Control.m_control_mutex);
		avalible = Control.m_avalible;
	}
	if (avalible)
	{
		Control.m_avalible = false;
		Control.clean();
		
		TArray<FStringAssetReference> TF;
		//ptr->RequestAsyncLoad(TF, std::bind([=]() {}, std::move(FSM), std::move()));

		std::vector<std::tuple<FString, uint64_t, bool, FString>> result;
		auto ite = 






		Control.m_target_count = path.Num();
		Control.m_finish_count = 0;
		for (size_t i = 0; i < 10; ++i)
			Control.m_all_thread.push_back(std::thread(&FunctionExecute));
	}
	*/
	/*
	FString ThePath = FPaths::ProjectLogDir();
	ThePath += L"total_mesh.csv";
	Doc::writer output(*ThePath, Doc::Format::UTF8_WITH_BOM);
	if (output.is_open())
	{
		std::map<FString, MeshState> m_all_mesh;

		IPlatformFile& fileManager = FPlatformFileManager::Get().GetPlatformFile();
		TArray<FString> path;
		fileManager.FindFilesRecursively(path, *FPaths::ProjectContentDir(), L".uasset");
		std::vector<std::tuple<FString, uint64_t, bool, FString>> all_mesh;
		for (auto ite = path.CreateIterator(); ite; ++ite)
		{
			FString tem{ ite->Len() - 7, **ite };
			FString Name = FPaths::GetCleanFilename(tem);
			FPaths::MakePathRelativeTo(tem, *FPaths::ProjectContentDir());
			tem = L"/Game/" + tem;
			UStaticMesh* Object = LoadObject<UStaticMesh>(nullptr, *tem);
			if (IsValid(Object))
			{
				//UObject* Object = LoadObject<UObject>(nullptr, *tem);
				UStaticMesh* Mesh = Cast<UStaticMesh>(Object);
				if (IsValid(Mesh))
				{
					auto result = LoadMeshLoad(Mesh);
					all_mesh.push_back({ std::move(Name),std::get<0>(result)[0].triangle, std::move(std::get<1>(result)), tem });
				}
			}
		}
		std::sort(all_mesh.begin(), all_mesh.end(), [](const decltype(all_mesh)::value_type& p, const decltype(all_mesh)::value_type& k) -> bool {
			auto& ref = std::get<1>(p);
			auto& ref2 = std::get<1>(k);
			return ref > ref2;
		});


		output.write(L"名称,最大面数,重复材质,地址,\r\n");
		for (auto& ite : all_mesh)
		{
			output.write(*std::get<0>(ite));
			output.write(L",");
			output.write(NumberToString(std::get<1>(ite)) + L",");
			if (std::get<2>(ite))
				output.write(L"是,");
			else
				output.write(L"否,");
			output.write(*std::get<3>(ite));
			output.write(L",\r\n");
		}
	}
	else
		UE_LOG(LogTemp, Log, L"Can not open file %s", *ThePath);
		*/
	

	//TArray<UObject*> all_object;
	//EngineUtils::FindOrLoadAssetsByPath(FPaths::ProjectContentDir(), )
	/*
		soft_path.Add(FSoftObjectPath{ *ite });
	auto result = inst.LoadAssetList(soft_path);
	if (result->IsLoadingInProgress())
	{
		result->BindUpdateDelegate(FStreamableUpdateDelegate::CreateLambda([](TSharedRef<struct FStreamableHandle> Handle) {
			TArray<UObject*> all_object;
			Handle->GetLoadedAssets(all_object);
			volatile int i = 0;
		}));
	}
	*/
	/*
	for (auto ite = path.CreateIterator(); ite; ++ite)
	{
		FSoftObjectPath Path{ *ite };
		FAssetData FAD;
		if (inst.GetAssetDataForPath(Path, FAD))
		{
			UStaticMesh* SM = Cast<UStaticMesh>(FAD.GetAsset());
			if (IsValid(SM))
			{
				auto name = SM->GetName();
				volatile int  i = 0;
			}
				
		}
		//Path.

	}
	*/


	/*
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FAssetPickerConfig Config;
	Config.SelectionMode = decltype(Config.SelectionMode)::Multi;
	Config.Filter.ClassNames.Append({ UStaticMesh::StaticClass()->GetFName() });
	auto ptr = ContentBrowserModule.Get().CreateAssetPicker(Config);
	*/
	//Config
}


/*
void ListSameSectionOfStaticMesh(UWorld* World)
{
	TActorIterator<AActor> all_actor{ World };
	std::map<FString, std::set<FString>> static_mesh_matrial;

	for (all_actor; all_actor; ++all_actor)
	{
		AActor* A = *all_actor;
		
		AStaticMeshActor* SMA = Cast<AStaticMeshActor>(A);
		if (IsValid(SMA))
		{
			UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent();
			UMaterialInterface* UMI2 = SMC->GetMaterial(0);
			FString string2 = UMI2->GetName();
			UStaticMesh* FSM = SMC->GetStaticMesh();
			UMaterialInterface* UMI = FSM->GetMaterial(0);
			FString string = UMI->GetName();
		}
	}
}
*/

