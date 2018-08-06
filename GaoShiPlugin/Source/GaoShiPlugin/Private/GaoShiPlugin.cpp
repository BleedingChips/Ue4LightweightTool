// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "GaoShiPlugin.h"
#include "IConsoleManager.h"
#include "Engine.h"
#include "Classes/Animation/SkeletalMeshActor.h"
#include "InstancedFoliageActor.h"
#include "MeshLODDetector.h"
#include "SelectionFilter.h"
#include <fstream>
#include <map>
#define LOCTEXT_NAMESPACE "FGaoShiPluginModule"

DECLARE_LOG_CATEGORY_EXTERN(GaoShiPlugin, Log, All);
DEFINE_LOG_CATEGORY(GaoShiPlugin);
FConsoleCommandWithWorldAndArgsDelegate DetectObjectLODDelegate;

FConsoleCommandWithWorldAndArgsDelegate FilteStaticMeshActorDelegate;

FConsoleCommandWithWorldAndArgsDelegate FilteClassNameAndNameDelegate;

void FGaoShiPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	auto& ref = IConsoleManager::Get();

	DetectObjectLODDelegate.BindLambda([](const TArray< FString >&, UWorld* World) {
		DetectObjectLOD(World);
	});
	ref.RegisterConsoleCommand(L"GaoShiDetectObjectLod", L"", DetectObjectLODDelegate);

#ifdef WITH_EDITORONLY_DATA
	FilteStaticMeshActorDelegate.BindLambda([](const TArray< FString >& para, UWorld* World) {
		if (para.Num() == 1)
			SelecteActorFormSelectedStaticMeshActor(para[0]);
		else
			UE_LOG(GaoShiPlugin, Log, L"Gaoshi Plugin Need one parameter for this command");
	});
	ref.RegisterConsoleCommand(L"GaoShiFilteStaticMeshActor", L"", FilteStaticMeshActorDelegate);

	FilteClassNameAndNameDelegate.BindLambda([](const TArray< FString >& para, UWorld* World) {
		if(para.Num() == 2)
			FilteActorClassNameAndActorName(World, para[0], para[1]);
		else if(para.Num() == 1)
			FilteActorClassNameAndActorName(World, para[0], L".+");
		else {
			UE_LOG(GaoShiPlugin, Log, L"Gaoshi Plugin Need one or tow parameter for this command");
		}
	});
	ref.RegisterConsoleCommand(L"GaoShiFilteClassNameAndName", L"", FilteClassNameAndNameDelegate);
#endif


	
	
}

void FGaoShiPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGaoShiPluginModule, GaoShiPlugin)