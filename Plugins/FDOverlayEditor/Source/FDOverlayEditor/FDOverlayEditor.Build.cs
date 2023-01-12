// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FDOverlayEditor : ModuleRules
{
	public FDOverlayEditor(ReadOnlyTargetRules Target) : base(Target)
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
				// ... add other public dependencies that you statically link with here ...
				"UnrealEd",
                "InputCore",
                "InteractiveToolsFramework",
                "MeshModelingToolsExp",
                "GeometryProcessingInterfaces",
				"FDShaders",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Projects",
				"InputCore",
				"EditorFramework",
				"EditorStyle",
				"UnrealEd",
				"LevelEditor",
				"InteractiveToolsFramework",
				"EditorInteractiveToolsFramework",
				// ... add private dependencies that you statically link with here ...	
                "EditorSubsystem",
                "WorkspaceMenuStructure",
                "UVEditorTools",
                "DynamicMesh",
				"GeometryCore",
				"AdvancedPreviewScene",
                "ModelingComponentsEditorOnly", // Static/skeletal mesh tool targets
				"ModelingToolsEditorMode",
                "ModelingComponents",
				"FDShaders",
				"AssetTools",
				"AssetRegistry",
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
