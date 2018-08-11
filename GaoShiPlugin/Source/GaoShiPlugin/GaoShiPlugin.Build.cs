// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GaoShiPlugin : ModuleRules
{
    public GaoShiPlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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
				"GaoShiPlugin/Public"
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"GaoShiPlugin/Private",
                "Foliage/Public",
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
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
