#include "MeshLODDetector.h"
#include "Classes/Animation/SkeletalMeshActor.h"
#include <fstream>
std::string translate(const FString& fs)
{
	std::string temporary(fs.Len() * 4, '\0');
	auto pair = PO::Tool::utf16s_to_utf8s(reinterpret_cast<const char16_t*>(*fs), fs.Len(), &temporary[0], fs.Len() * 4);
	temporary.resize(pair.second);
	return temporary;
}

void AddMeshCount(EComponentMobility::Type type, bool case_shadow, MeshState& state, size_t count = 1)
{
	state.total_count += count;
	switch (type)
	{
	case EComponentMobility::Type::Movable:
		state.movable_count += count;
		break;
	case EComponentMobility::Type::Stationary:
		state.stationary_count += count;
		break;
	}
	if (case_shadow)
		state.cast_shadow_count += count;
}

void ComponentToStateMap(UStaticMeshComponent* component, std::map<std::string, MeshState>& output, size_t count)
{
	if (component != nullptr)
	{
		const UStaticMesh* mesh = component->GetStaticMesh();
		EComponentMobility::Type mobility = component->Mobility;
		bool cast_shadow = component->CastShadow;
		if (mesh != nullptr)
		{
			std::string name = translate(mesh->GetName());
			auto ite = output.find(name);
			if (ite == output.end())
			{
				ite = output.insert({ name, MeshState{ } }).first;
				size_t mesh_count = mesh->GetNumLODs();
				for (size_t i = 0; i < mesh_count; ++i)
				{
					const FStaticMeshLODResources& res = mesh->GetLODForExport(i);
					ite->second.lod_state.push_back({ LODState{ res.GetNumTriangles(), res.Sections.Num(), mesh->SourceModels[i].ScreenSize } });
				}
			}
			AddMeshCount(component->Mobility, component->CastShadow, ite->second, count);
		}
	}
}

void ComponentToStateMap(USkeletalMeshComponent* component, std::map<std::string, MeshState>& output)
{
	if (component != nullptr)
	{
		const USkeletalMesh* mesh = component->SkeletalMesh;
		if (mesh != nullptr)
		{
			std::string name = translate(mesh->GetName());
			auto ite = output.find(name);
			if(ite == output.end())
			{
				ite = output.insert({ std::move(name), MeshState {} }).first;
				const FSkeletalMeshResource* res = component->GetSkeletalMeshResource();
				auto& lod = mesh->LODInfo;
				for (size_t i = 0; i < res->LODModels.Num(); ++i)
				{
					const FStaticLODModel& mode = res->LODModels[i];
					ite->second.lod_state.push_back(LODState{ mode.GetTotalFaces(), mode.Sections.Num(), lod[i].ScreenSize });
				}
			}
			AddMeshCount(component->Mobility, component->CastShadow, ite->second);
		}
	}
}

void ExtractActor(AActor* act, std::map<std::string, MeshState>& static_mesh, std::map<std::string, MeshState>& dymamin_mesh, std::map<std::string, MeshState>& forest)
{
	TArray<AActor*> child_actor;
	act->GetAllChildActors(child_actor);
	if (child_actor.Num() != 0)
	{
		for (size_t i = 0; i < child_actor.Num(); ++i)
			ExtractActor(child_actor[i], static_mesh, dymamin_mesh, forest);
	}
	auto static_mesh_actor = Cast<AStaticMeshActor>(act);
	if (static_mesh_actor != nullptr)
	{
		if (static_mesh_actor->GetStaticMeshComponent() != nullptr)
			ComponentToStateMap(static_mesh_actor->GetStaticMeshComponent(), static_mesh);
	}
	else {
		const ASkeletalMeshActor* actorsk = Cast<ASkeletalMeshActor>(act);
		if (actorsk != nullptr)
		{
			if (actorsk->GetSkeletalMeshComponent() != nullptr)
				ComponentToStateMap(actorsk->GetSkeletalMeshComponent(), dymamin_mesh);
		}
		else {
			AInstancedFoliageActor* actoraif = Cast<AInstancedFoliageActor>(act);
			if (actoraif != nullptr)
			{
				TMap<UFoliageType*, FFoliageMeshInfo*> map = actoraif->GetAllInstancesFoliageType();
				for (auto& ite : map)
				{
					auto index = ite.Value->Instances.Num();
					ComponentToStateMap(ite.Value->Component, forest, index);
				}
			}
		}
	}
}

void DetectObjectLOD(UWorld* World)
{
	std::map<std::string, MeshState> static_mesh;
	std::map<std::string, MeshState> dynamic_mesh;
	std::map<std::string, MeshState> forest_mesh;
	TActorIterator<AActor> all_actor{ World };
	for (all_actor; all_actor; ++all_actor)
	{
		ExtractActor(*all_actor, static_mesh, dynamic_mesh, forest_mesh);
	}

	FString ThePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectLogDir());
	FString CurrentDir = World->GetMapName();
	ThePath += CurrentDir + L".csv";

	std::ofstream file(*ThePath, std::ios::binary | std::ios::out);
	if (file)
	{
		unsigned char bom[] = { 0xef, 0xbb, 0xbf };
		UE_LOG(LogTemp, Log, L"Success OpenFile %s", (*ThePath));
		file.write(reinterpret_cast<const char*>(bom), 3);
		file << translate(World->GetMapName()) << std::endl;
		file << "静态模型" << std::endl;
		file << "模型名字,模型使用数,动态数,固定数,产生阴影的个数,LOD面数,LODSection数,LOD屏幕距离" << std::endl;
		for (auto& ite : static_mesh)
		{
			file << ite.first << "," << ite.second.total_count << "," << ite.second.movable_count << "," <<
				ite.second.stationary_count << "," << ite.second.cast_shadow_count << ",";
			for (auto& ite2 : ite.second.lod_state)
				file << ite2.triangle << "," << ite2.section << "," << ite2.screne_rate << ",";
			file << std::endl;
		}
		file << std::endl;

		file << "动态模型" << std::endl;
		file << "模型名字,模型使用数,动态数,固定数,产生阴影的个数,LOD面数,LODSection数,LOD屏幕距离" << std::endl;
		for (auto& ite : dynamic_mesh)
		{
			file << ite.first << "," << ite.second.total_count << "," << ite.second.movable_count << "," <<
				ite.second.stationary_count << "," << ite.second.cast_shadow_count << ",";
			for (auto& ite2 : ite.second.lod_state)
				file << ite2.triangle << "," << ite2.section << "," << ite2.screne_rate << ",";
			file << std::endl;
		}
		file << std::endl;

		file << "植被模型" << std::endl;
		file << "模型名字,模型使用数,动态数,固定数,产生阴影的个数,LOD面数,LODSection数,LOD屏幕距离" << std::endl;
		for (auto& ite : forest_mesh)
		{
			file << ite.first << "," << ite.second.total_count << "," << ite.second.movable_count << "," <<
				ite.second.stationary_count << "," << ite.second.cast_shadow_count << ",";
			for (auto& ite2 : ite.second.lod_state)
				file << ite2.triangle << "," << ite2.section << "," << ite2.screne_rate << ",";
			file << std::endl;
		}
		file << std::endl;

		for (auto& ite : forest_mesh)
		{
			auto ite2 = static_mesh.find(ite.first);
			if (ite2 != static_mesh.end())
			{
				ite2->second.total_count += ite.second.total_count;
				ite2->second.movable_count += ite.second.movable_count;
				ite2->second.stationary_count += ite.second.stationary_count;
				ite2->second.cast_shadow_count += ite.second.cast_shadow_count;
			}
			else {
				static_mesh.insert(ite);
			}
		}
		
		file << "总静态模型" << std::endl;
		file << "模型名字,模型使用数,动态数,固定数,产生阴影的个数,LOD面数,LODSection数,LOD屏幕距离" << std::endl;
		for (auto& ite : static_mesh)
		{
			file << ite.first << "," << ite.second.total_count << "," << ite.second.movable_count << "," <<
				ite.second.stationary_count << "," << ite.second.cast_shadow_count << ",";
			for (auto& ite2 : ite.second.lod_state)
				file << ite2.triangle << "," << ite2.section << "," << ite2.screne_rate << ",";
			file << std::endl;
		}
		file << std::endl;
	}
	else {
		UE_LOG(LogTemp, Log, L"File OpenFile %s", (*ThePath));
	}


	UE_LOG(LogTemp, Log, L"%s", (*World->GetMapName()));
}


