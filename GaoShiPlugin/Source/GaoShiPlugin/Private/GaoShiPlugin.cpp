// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "GaoShiPlugin.h"
#include "IConsoleManager.h"
#include "Engine.h"
#include "Classes/Animation/SkeletalMeshActor.h"
#include "InstancedFoliageActor.h"
#include "MeshLODDetector.h"
#include "SelectionFilter.h"
#include "InfoRecord.h"
#include <fstream>
#include <map>
#define LOCTEXT_NAMESPACE "FGaoShiPluginModule"

DECLARE_LOG_CATEGORY_EXTERN(GaoShiPlugin, Log, All);
DEFINE_LOG_CATEGORY(GaoShiPlugin);


FConsoleCommandWithWorldDelegate DetectObjectLODDelegate;
FConsoleCommandWithArgsDelegate FilterStaticMeshDelegate;
FConsoleCommandWithArgsDelegate FilterActorClassNameDelegate;
FConsoleCommandWithArgsDelegate FilterActorDisplayNameDelegate;
FConsoleCommandWithArgsDelegate FilterActorIDNameDelegate;
FConsoleCommandDelegate StoreSelectedActorDelegate;
FConsoleCommandDelegate RestoreSelectedActorDelegate;
FConsoleCommandWithWorldAndArgsDelegate DebugCommandLineOutputDelegate;
FConsoleCommandWithWorldAndArgsDelegate CopySeletionActorNameDelegate;
FConsoleCommandWithWorldDelegate CopyCurrentPositionDelegate;
FConsoleCommandWithArgsDelegate MoveToPositionDelegate;
//FConsoleCommandWithWorldAndArgsDelegate MoveToRecordedPostionDelegate;
//FConsoleCommandWithWorldAndArgsDelegate CopyRecordedPositionDelegate;

void FGaoShiPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	auto& ref = IConsoleManager::Get();

	DetectObjectLODDelegate.BindLambda([](UWorld* World) {
		DetectObjectLOD(World);
	});
	ref.RegisterConsoleCommand(L"GSDetectObjectLod", L"", DetectObjectLODDelegate);

#ifdef WITH_EDITORONLY_DATA
	FilterStaticMeshDelegate.BindLambda([](const TArray< FString >& para) {
		if (para.Num() == 1)
			FilterStaticMesh(para[0]);
		else
			UE_LOG(GaoShiPlugin, Log, L"GSFilterStaticMesh need one parameter for this command");
	});
	ref.RegisterConsoleCommand(L"GSFilterStaticMesh", L"", FilterStaticMeshDelegate);

	FilterActorClassNameDelegate.BindLambda([](const TArray< FString >& para) {
		if(para.Num() == 1)
			FilterActorClassName(para[0]);
		else {
			UE_LOG(GaoShiPlugin, Log, L"GSFilterActorClassName need one parameter for this command");
		}
	});
	ref.RegisterConsoleCommand(L"GSFilterActorClassName", L"", FilterActorClassNameDelegate);

	FilterActorDisplayNameDelegate.BindLambda([](const TArray< FString >& para) {
		if (para.Num() == 1)
			FilterActorDisplayName(para[0]);
		else {
			UE_LOG(GaoShiPlugin, Log, L"GSFilterActorDisplayName need one parameter for this command");
		}
	});
	ref.RegisterConsoleCommand(L"GSFilterActorDisplayName", L"", FilterActorDisplayNameDelegate);

	FilterActorIDNameDelegate.BindLambda([](const TArray< FString >& para) {
		if (para.Num() == 1)
			FilterActorIDName(para[0]);
		else {
			UE_LOG(GaoShiPlugin, Log, L"GSFilterActorIDName need one parameter for this command");
		}
	});
	ref.RegisterConsoleCommand(L"GSFilterActorIDName", L"", FilterActorIDNameDelegate);

	CopySeletionActorNameDelegate.BindLambda([](const TArray< FString >& para, UWorld* World) {
		FString total;
		for (size_t index = 0; index < para.Num(); ++index)
		{
			if (index == 0)
				total += para[index];
			else
				total += FString(L" ") + para[index];
		}
		CopySelectionActorToClioboard(total);
	});
	ref.RegisterConsoleCommand(L"GSCopySelectedActorName", L"", CopySeletionActorNameDelegate);

	MoveToPositionDelegate.BindLambda([](const TArray< FString >& para) {
		if (para.Num() < 1)
		{
			UE_LOG(GaoShiPlugin, Log, L"GSMoveToPosition need at last one parameter for this command");
		}
		else {
			FString total;
			for (size_t index = 0; index < para.Num(); ++index)
			{
				if (index == 0)
					total += para[index];
				else
					total += FString(L" ") + para[index];
			}
			MoveToPosition(total);
		}
	});
	ref.RegisterConsoleCommand(L"GSMoveToPosition", L"", MoveToPositionDelegate);

	StoreSelectedActorDelegate.BindLambda([]() {
		StoreSelectedActor();
	});
	ref.RegisterConsoleCommand(L"GSStoreSelectedActor", L"", StoreSelectedActorDelegate);

	RestoreSelectedActorDelegate.BindLambda([]() {
		RestoreSelectedActor();
	});
	ref.RegisterConsoleCommand(L"GSRestoreSelectedActor", L"", RestoreSelectedActorDelegate);

#endif

	CopyCurrentPositionDelegate.BindLambda([](UWorld* World) {
		CopyCurrentPosition(World);
	});
	ref.RegisterConsoleCommand(L"GSCopyCurrentPosition", L"", CopyCurrentPositionDelegate);

	DebugCommandLineOutputDelegate.BindLambda([](const TArray< FString >& para, UWorld* World)
	{
		FString Result;
		for (size_t i = 0; i < para.Num(); ++i)
			Result += FString{ L"<" } +para[i] + FString{ L">," };
		UE_LOG(GaoShiPlugin, Log, L"%s", *Result);
	});
	ref.RegisterConsoleCommand(L"GSDebugCommandLineOutput", L"", DebugCommandLineOutputDelegate);
	
}

void FGaoShiPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGaoShiPluginModule, GaoShiPlugin)