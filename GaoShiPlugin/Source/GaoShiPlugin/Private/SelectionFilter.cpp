#include "SelectionFilter.h"




#ifdef WITH_EDITORONLY_DATA
#include <Editor.h>

void FilteStaticMeshActor(const FString& name)
{
	USelection* selection = GEditor->GetSelectedActors();
	if (IsValid(selection))
	{
		TArray<TWeakObjectPtr<>> object_array;
		selection->GetSelectedObjects(object_array);
		GEditor->SelectNone(false, true, false);
		auto count = object_array.Num();
		for (size_t i = 0; i < count; ++i)
		{
			auto ptr = object_array[i];
			if (ptr.IsValid())
			{
				AStaticMeshActor* result = Cast<AStaticMeshActor>(ptr.Get());
				if (result != nullptr)
				{
					auto command = result->GetStaticMeshComponent();
					if (command != nullptr)
					{
						auto mesh = command->GetStaticMesh();
						if (mesh != nullptr)
							if (name == mesh->GetName())
								GEditor->SelectActor(result, true, true, true);;
					}
				}
			}
		}
	}
}


void RemoveFoliageInSelectedActor(UWorld* World)
{
	USelection* selection = GEditor->GetSelectedActors();
	if (IsValid(selection))
	{
		TArray<TWeakObjectPtr<>> object_array;
		selection->GetSelectedObjects(object_array);
		selection->BeginBatchSelectOperation();
		auto count = object_array.Num();
		for (size_t i = 0; i < count; ++i)
		{
			auto ptr = object_array[i];
			if (ptr.IsValid())
			{
				AInstancedFoliageActor* result = Cast<AInstancedFoliageActor>(ptr.Get());
				if (result != nullptr)
					selection->Deselect(ptr.Get());
			}
		}
		selection->EndBatchSelectOperation();
		//selection->
		//selection->MarkBatchDirty();
		//selection->MarkPackageDirty();
		//GEditor->Dirty
	}
}

void MoveStaticMeshActorToFoliage(UWorld* World, FString target_mesh_name)
{
	UE_LOG(LogTemp, Log, L"Try Handle mesh name: %s", (*target_mesh_name));
	TActorIterator<AActor> all_actor{ World };
	AInstancedFoliageActor* foliage = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(World, true);
	int32_t handle_count = 0;
	for (all_actor; all_actor; ++all_actor)
	{
		AStaticMeshActor* actor = Cast<AStaticMeshActor>(*all_actor);
		if (IsValid(actor))
		{
			UStaticMeshComponent* Component = actor->GetStaticMeshComponent();
			if (IsValid(Component))
			{
				UStaticMesh* Mesh = Component->GetStaticMesh();
				if (IsValid(Mesh))
				{
					FString Name = Mesh->GetName();
					if (Name == target_mesh_name)
					{

					}
				}
			}
		}
		//ExtractActor(*all_actor, static_mesh, dynamic_mesh, forest_mesh);
	}
}
#endif


#ifdef WITH_EDITORONLY_DATA

void SelecteActorFormSelectedStaticMeshActor(const FString& string) noexcept
{
	FRegexPattern pattern(string);
	USelection* selection = GEditor->GetSelectedActors();
	if (IsValid(selection))
	{
		TArray<TWeakObjectPtr<>> object_array;
		selection->GetSelectedObjects(object_array);
		GEditor->SelectNone(false, true, false);
		auto count = object_array.Num();
		for (size_t i = 0; i < count; ++i)
		{
			auto ptr = object_array[i];
			if (ptr.IsValid())
			{
				AStaticMeshActor* result = Cast<AStaticMeshActor>(ptr.Get());
				if (result != nullptr)
				{
					auto command = result->GetStaticMeshComponent();
					if (command != nullptr)
					{
						auto mesh = command->GetStaticMesh();
						if (mesh != nullptr)
						{
							FRegexMatcher regMatcher(pattern, mesh->GetName());
							regMatcher.SetLimits(0, mesh->GetName().Len());
							if(regMatcher.FindNext())
								GEditor->SelectActor(result, true, true, true);
						}
					}
				}
			}
		}
	}
}
void FilteActorClassNameAndActorName(UWorld* world, const FString& class_name, const FString& actor_name) noexcept
{
	FRegexPattern cn(class_name);
	FRegexPattern an(actor_name);
	USelection* selection = GEditor->GetSelectedActors();
	if (IsValid(selection))
	{
		TArray<TWeakObjectPtr<>> object_array;
		selection->GetSelectedObjects(object_array);
		GEditor->SelectNone(false, true, false);
		auto count = object_array.Num();
		for (size_t i = 0; i < count; ++i)
		{
			auto ptr = object_array[i];
			if (ptr.IsValid())
			{
				FName N = ptr->GetClass()->GetFName();
				FString ClassName = N.ToString();
				FRegexMatcher cn_match(cn, ClassName);
				if (cn_match.FindNext() && cn_match.GetMatchBeginning() == 0 && cn_match.GetMatchEnding() == ClassName.Len())
				{
					FString DisplayName = UKismetSystemLibrary::GetDisplayName(ptr.Get());
					FRegexMatcher dm_match(an, DisplayName);
					if (dm_match.FindNext() && dm_match.GetMatchBeginning() == 0 && cn_match.GetMatchEnding() == ClassName.Len())
					{
						AActor* ac = Cast<AActor>(ptr.Get());
						if (IsValid(ac))
							GEditor->SelectActor(ac, true, true, true);
					}
				}
			}
		}
	}
}
#endif