#pragma once
#include "Encoding.h"
#include "Engine.h"
#include <map>
#include <vector>
struct LODState
{
	int32 triangle;
	int32 section;
	float screne_rate;
};

struct MeshState
{
	size_t total_count = 0;
	size_t cast_shadow_count = 0;
	size_t movable_count = 0;
	size_t stationary_count = 0;
	std::vector<LODState> lod_state;
};

std::string translate(const FString& fs);

void ComponentToStateMap(UStaticMeshComponent* component, std::map<std::string, MeshState>& output, size_t count = 1);
void ComponentToStateMap(USkeletalMeshComponent* component, std::map<std::string, MeshState>& output);
void ExtractActor(AActor* act, std::map<std::string, MeshState>& static_mesh, std::map<std::string, MeshState>& dymamin_mesh, std::map<std::string, MeshState>& forest);
void DetectObjectLOD(UWorld* world);