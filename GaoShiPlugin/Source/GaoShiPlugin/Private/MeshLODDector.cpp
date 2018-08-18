#include "MeshLODDetector.h"
#include "Classes/Animation/SkeletalMeshActor.h"
#include "document_interface.h"
#include <set>
#include <sstream>
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
	for (auto& ite : SM->StaticMaterials)
	{
		auto name = ite.MaterialInterface->GetName();
		if (m_material_name.find(name) != m_material_name.end())
			return { std::move(tem), true };
		else
			m_material_name.insert(name);
	}
	return { std::move(tem), false };
}

struct MeshState
{
	uint32_t m_actor_count = 0;
	uint32_t m_foliage_count = 0;
	bool m_repeat_material = false;
	std::vector<LODState> m_all_state;
};


template<typename T>
std::wstring NumberToString(T value)
{
	static std::wstringstream wss;
	wss.clear();
	std::wstring buffer;
	wss << value;
	wss >> buffer;
	return buffer;
}

void CheckStaticMesh(UWorld* World)
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
		std::wstringstream wss;
		for (auto& ite : m_all_mesh)
		{
			output.write(*ite.first);
			output.write(L",");
			output.write(NumberToString(ite.second.m_actor_count));
			output.write(L",");
			output.write(NumberToString(ite.second.m_foliage_count));
			output.write(L",");
			if(ite.second.m_repeat_material)
				output.write(L"是");
			else
				output.write(L"否");
			output.write(L",");
			for (auto& ite : ite.second.m_all_state)
			{
				output.write(NumberToString(ite.triangle));
				output.write(L",");
				output.write(NumberToString(ite.section));
				output.write(L",");
				output.write(NumberToString(ite.screne_rate));
				output.write(L",");
			}
			output.write(L"\r\n");
		}
	}
	else {
		UE_LOG(LogTemp, Log, L"File OpenFile %s", (*ThePath));
	}
	UE_LOG(LogTemp, Log, L"%s", (*World->GetMapName()));
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

