// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GaoShiPlugin : ModuleRules
{

    private string ThirdPartProject
    {
        get { return ModuleDirectory + "\\..\\..\\ThirdPartProject\\"; }
    }

    public GaoShiPlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        //Type = ModuleType.External;
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        //PCHUsage = ModuleRules.PCHUsageMode.Default;
        //PrecompileForTargets = ModuleRules.PrecompileTargetsType.Any;
        if (Target.Type == TargetType.Editor)
        {
            PublicDependencyModuleNames.AddRange(
            new string[] { "UnrealED" });
            PublicIncludePaths.AddRange(
            new string[] {
                "UnrealED/Public"
			}
            );
        }


        PublicIncludePaths.AddRange(
			new string[] {
				"GaoShiPlugin/Public",
                 "Foliage/Public",
                ThirdPartProject + "ThirdPartProject",
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"GaoShiPlugin/Private",
                "Foliage/Private",
                ThirdPartProject + "ThirdPartProject",
				// ... add other private include paths required here ...
			}
			);

       

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "Foliage",
                "ApplicationCore",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);

        string ThirdPartPath = ThirdPartProject;
        if (Target.Platform == UnrealTargetPlatform.Win64)
            ThirdPartPath += "x64\\";
        /*
        if (Target.Configuration == UnrealTargetConfiguration.Debug
            || Target.Configuration == UnrealTargetConfiguration.DebugGame)
            ThirdPartPath += "debug\\";
        else
        */
        ThirdPartPath += "release\\";

        PublicAdditionalLibraries.AddRange(
            new string[]
            {
                ThirdPartPath + "ThirdPartProject.lib"
            }
            );

        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
