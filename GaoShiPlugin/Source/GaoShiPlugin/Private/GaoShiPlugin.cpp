// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "GaoShiPlugin.h"
#include "IConsoleManager.h"
#include "Engine.h"
#include "Classes/Animation/SkeletalMeshActor.h"
#include "InstancedFoliageActor.h"
#include "MeshLODDetector.h"
#include <fstream>
#include <map>
#define LOCTEXT_NAMESPACE "FGaoShiPluginModule"


FConsoleCommandWithWorldAndArgsDelegate DetectObjectLODDelegate;

FConsoleCommandWithWorldAndArgsDelegate RemoveFoliageInSelectedActorDelegarte;

void FGaoShiPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	auto& ref = IConsoleManager::Get();

	DetectObjectLODDelegate.BindLambda([](const TArray< FString >&, UWorld* World) {
		DetectObjectLOD(World);
	});
	ref.RegisterConsoleCommand(L"GaoShiDetectObjectLod", L"", DetectObjectLODDelegate);

#ifdef WITH_EDITORONLY_DATA
	RemoveFoliageInSelectedActorDelegarte.BindLambda([](const TArray< FString >& para, UWorld* World) {
		if(para.Num() == 1)
			FilteStaticMeshActor(para[0]);
	});
	ref.RegisterConsoleCommand(L"GaoShiFilteStaticMechActor", L"", RemoveFoliageInSelectedActorDelegarte);
#endif


	
	
}

void FGaoShiPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGaoShiPluginModule, GaoShiPlugin)