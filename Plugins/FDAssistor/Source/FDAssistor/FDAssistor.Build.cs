// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FDAssistor : ModuleRules
{
	public FDAssistor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",

                "UnrealEd",
                "InputCore",
                "InteractiveToolsFramework",
                "MeshModelingToolsExp",
                "GeometryProcessingInterfaces" // For supporting launching the UVEditor directly from Modeling Tools or elsewhere
			}
            );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"EditorFramework",
				"EditorStyle",
				"UnrealEd",
				"LevelEditor",
				"InteractiveToolsFramework",
				"EditorInteractiveToolsFramework",
				"EditorSubsystem",
				"WorkspaceMenuStructure",
				"UVEditorTools",
				"DynamicMesh",
				"AdvancedPreviewScene",
				"ModelingComponentsEditorOnly", // Static/skeletal mesh tool targets
				"ModelingToolsEditorMode",
				"ModelingComponents",
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
