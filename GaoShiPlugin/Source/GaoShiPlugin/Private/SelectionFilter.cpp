#include "SelectionFilter.h"
#include <regex>


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

void FilterStaticMesh(const FString& string) noexcept
{
	std::wregex pattern(*string, std::regex::optimize);
	//FRegexPattern pattern(string);
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
							FString Name = mesh->GetName();
							if (std::regex_match(*Name, *Name + Name.Len(), pattern))
								GEditor->SelectActor(result, true, false);
						}
					}
				}
			}
		}
		GEditor->SelectActor(nullptr, true, true, true, true);
	}
}

void FilterActorClassName(const FString& string) noexcept
{
	std::wregex pattern(*string, std::regex::optimize);
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
			AActor* ac = Cast<AActor>(ptr.Get());
			if (IsValid(ac))
			{
				FName N = ptr->GetClass()->GetFName();
				FString ClassName = N.ToString();
				if (std::regex_match(*ClassName, *ClassName + ClassName.Len(), pattern))
					GEditor->SelectActor(ac, true, false);
			}
		}
		GEditor->SelectActor(nullptr, true, true, true, true);
	}
}

void FilterActorDisplayName(const FString& string) noexcept
{
	std::wregex pattern(*string, std::regex::optimize);
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
			AActor* ac = Cast<AActor>(ptr.Get());
			if (IsValid(ac))
			{
				FString DisplayName = UKismetSystemLibrary::GetDisplayName(ptr.Get());
				if (std::regex_match(*DisplayName, *DisplayName + DisplayName.Len(), pattern))
					GEditor->SelectActor(ac, true, false);
			}
		}
		GEditor->SelectActor(nullptr, true, true, true, true);
	}
}

void FilterActorIDName(const FString& string) noexcept
{
	std::wregex pattern(*string, std::regex::optimize);
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
			AActor* ac = Cast<AActor>(ptr.Get());
			if (IsValid(ac))
			{
				FString IDName = ptr->GetName();
				if (std::regex_match(*IDName, *IDName + IDName.Len(), pattern))
					GEditor->SelectActor(ac, true, false);
			}
		}
		GEditor->SelectActor(nullptr, true, true, true, true);
	}
}

TArray<TWeakObjectPtr<>> storage_object;
void StoreSelectedActor() noexcept
{
	USelection* selection = GEditor->GetSelectedActors();
	if (IsValid(selection))
	{
		TArray<TWeakObjectPtr<>> object_array;
		selection->GetSelectedObjects(object_array);
		storage_object = std::move(object_array);
	}
}

void RestoreSelectedActor() noexcept
{
	GEditor->SelectNone(false, true, false);
	for (auto& ite : storage_object)
	{
		if (ite.IsValid())
		{
			AActor* act = Cast<AActor>(ite.Get());
			if (act != nullptr)
				GEditor->SelectActor(act, true, false);
		}
	}
	GEditor->SelectActor(nullptr, true, true, true, true);
}
#endif